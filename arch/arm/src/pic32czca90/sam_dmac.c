/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_dmac.c
 *
 * PIC32CZ CA90 System DMA Controller Driver
 *
 * Register-mode only (no linked-list descriptors). Sufficient for I2C/SPI
 * transfers up to 65535 bytes.
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/semaphore.h>

#include "arm_internal.h"
#include "barriers.h"

#include "hardware/sam_dma.h"
#include "hardware/sam_mclk.h"
#include "sam_dmac.h"

#include <arch/pic32czca90/pic32czca90_irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DMA_NCHANNELS   PIC32CZCA90_DMA_NCHANNELS

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_dmach_s
{
  uint8_t        chan;
  bool           inuse;
  bool           rx;
  volatile bool  busy;
  uint32_t       flags;
  uint32_t       base;
  dma_callback_t callback;
  void          *arg;
  uint32_t       rxaddr;
  size_t         rxsize;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sam_dmach_s g_dmach[DMA_NCHANNELS];
static sem_t g_chsem;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t dma_getreg(uint32_t addr)
{
  return getreg32(addr);
}

static inline void dma_putreg(uint32_t val, uint32_t addr)
{
  putreg32(val, addr);
}

static inline uint32_t dmach_getreg(struct sam_dmach_s *dmach, uint32_t offset)
{
  return getreg32(dmach->base + offset);
}

static inline void dmach_putreg(struct sam_dmach_s *dmach, uint32_t offset,
                                uint32_t val)
{
  putreg32(val, dmach->base + offset);
}

/****************************************************************************
 * Name: sam_dma_terminate
 *
 * Description:
 *   Terminate a DMA transfer and invoke callback.
 *   Called from ISR or from sam_dmastop().
 *
 ****************************************************************************/

static void sam_dma_terminate(struct sam_dmach_s *dmach, int result)
{
  dma_callback_t callback;
  void *arg;

  /* Disable all channel interrupts */

  dmach_putreg(dmach, SAM_DMACH_INTENCLR_OFFSET, DMA_CHINTF_ALL);

  /* Disable channel and wait for hardware to confirm */

  uint32_t ctrla = dmach_getreg(dmach, SAM_DMACH_CTRLA_OFFSET);
  ctrla &= ~DMA_CHCTRLA_ENABLE;
  dmach_putreg(dmach, SAM_DMACH_CTRLA_OFFSET, ctrla);

  while (dmach_getreg(dmach, SAM_DMACH_CTRLA_OFFSET) & DMA_CHCTRLA_ENABLE)
    {
    }

  dmach->busy = false;

  /* Invoke callback */

  callback = dmach->callback;
  arg = dmach->arg;
  dmach->callback = NULL;
  dmach->arg = NULL;

  if (callback)
    {
      callback((DMA_HANDLE)dmach, arg, result);
    }
}

/****************************************************************************
 * Name: sam_dma_interrupt
 *
 * Description:
 *   DMA interrupt handler — single handler for all 4 priority IRQ vectors.
 *
 *   Priority mapping (naming is confusing):
 *     CHCTRLB.PRI=0 (PRI_1) → INTSTAT1 → NVIC "DMA_PRI0" (EXTINT+42)
 *     CHCTRLB.PRI=1 (PRI_2) → INTSTAT2 → NVIC "DMA_PRI1" (EXTINT+41)
 *     CHCTRLB.PRI=2 (PRI_3) → INTSTAT3 → NVIC "DMA_PRI2" (EXTINT+40)
 *     CHCTRLB.PRI=3 (PRI_4) → INTSTAT4 → NVIC "DMA_PRI3" (EXTINT+39)
 *
 ****************************************************************************/

static int sam_dma_interrupt(int irq, void *context, void *arg)
{
  uint32_t intstat_addr;
  uint32_t intstat;
  int ch;

  /* Map IRQ to INTSTATn register */

  switch (irq)
    {
      case SAM_IRQ_DMA_PRI0:
        intstat_addr = SAM_DMA_INTSTAT1;
        break;

      case SAM_IRQ_DMA_PRI1:
        intstat_addr = SAM_DMA_INTSTAT2;
        break;

      case SAM_IRQ_DMA_PRI2:
        intstat_addr = SAM_DMA_INTSTAT3;
        break;

      case SAM_IRQ_DMA_PRI3:
        intstat_addr = SAM_DMA_INTSTAT4;
        break;

      default:
        return OK;
    }

  intstat = dma_getreg(intstat_addr);

  /* Process each active channel */

  for (ch = 0; ch < DMA_NCHANNELS && intstat != 0; ch++)
    {
      if (!(intstat & (1u << ch)))
        {
          continue;
        }

      intstat &= ~(1u << ch);

      struct sam_dmach_s *dmach = &g_dmach[ch];

      if (!dmach->busy)
        {
          /* Spurious — clear flags and move on */

          dmach_putreg(dmach, SAM_DMACH_INTF_OFFSET, DMA_CHINTF_ALL);
          continue;
        }

      uint32_t chintf = dmach_getreg(dmach, SAM_DMACH_INTF_OFFSET);

      /* Clear all flags (write-to-clear) */

      dmach_putreg(dmach, SAM_DMACH_INTF_OFFSET, chintf);

      if (chintf & DMA_CHINTF_ERRORS)
        {
          sam_dma_terminate(dmach, -EIO);
        }
      else if (chintf & DMA_CHINTF_BC)
        {
          /* Block complete — invalidate cache for RX buffers */

          if (dmach->rx && dmach->rxsize > 0)
            {
              up_invalidate_dcache(dmach->rxaddr,
                                   dmach->rxaddr + dmach->rxsize);
            }

          sam_dma_terminate(dmach, OK);
        }
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_dma_initialize
 *
 * Description:
 *   NuttX weak function called early in boot to initialize DMA.
 *
 ****************************************************************************/

void weak_function arm_dma_initialize(void)
{
  int ch;

  /* Enable MCLK clocks for DMA controller.
   * CA90 clock enable: CLKMSK[id/32] |= (1 << (id%32))
   */

  uint32_t regval;

  regval = getreg32(SAM_MCLK_CLKMSK(MCLK_ID_AXI_DMA / 32u));
  regval |= (1u << (MCLK_ID_AXI_DMA % 32u));
  putreg32(regval, SAM_MCLK_CLKMSK(MCLK_ID_AXI_DMA / 32u));

  regval = getreg32(SAM_MCLK_CLKMSK(MCLK_ID_APB_DMA / 32u));
  regval |= (1u << (MCLK_ID_APB_DMA % 32u));
  putreg32(regval, SAM_MCLK_CLKMSK(MCLK_ID_APB_DMA / 32u));

  /* Enable DMA controller */

  dma_putreg(DMA_CTRLA_ENABLE, SAM_DMA_CTRLA);

  /* Initialize channel allocation semaphore (mutex) */

  nxsem_init(&g_chsem, 0, 1);

  /* Initialize channel state */

  for (ch = 0; ch < DMA_NCHANNELS; ch++)
    {
      g_dmach[ch].chan  = ch;
      g_dmach[ch].inuse = false;
      g_dmach[ch].busy  = false;
      g_dmach[ch].base  = SAM_DMA_CHAN_BASE(ch);

      /* Ensure channel is disabled and interrupts cleared */

      dmach_putreg(&g_dmach[ch], SAM_DMACH_CTRLA_OFFSET, 0);
      dmach_putreg(&g_dmach[ch], SAM_DMACH_INTENCLR_OFFSET, DMA_CHINTF_ALL);
      dmach_putreg(&g_dmach[ch], SAM_DMACH_INTF_OFFSET, DMA_CHINTF_ALL);
    }

  /* Attach single ISR to all 4 DMA priority IRQ vectors */

  irq_attach(SAM_IRQ_DMA_PRI0, sam_dma_interrupt, NULL);
  irq_attach(SAM_IRQ_DMA_PRI1, sam_dma_interrupt, NULL);
  irq_attach(SAM_IRQ_DMA_PRI2, sam_dma_interrupt, NULL);
  irq_attach(SAM_IRQ_DMA_PRI3, sam_dma_interrupt, NULL);

  up_enable_irq(SAM_IRQ_DMA_PRI0);
  up_enable_irq(SAM_IRQ_DMA_PRI1);
  up_enable_irq(SAM_IRQ_DMA_PRI2);
  up_enable_irq(SAM_IRQ_DMA_PRI3);
}

/****************************************************************************
 * Name: sam_dmachannel
 ****************************************************************************/

DMA_HANDLE sam_dmachannel(uint32_t chflags)
{
  struct sam_dmach_s *dmach = NULL;
  int ch;

  nxsem_wait_uninterruptible(&g_chsem);

  for (ch = 0; ch < DMA_NCHANNELS; ch++)
    {
      if (!g_dmach[ch].inuse)
        {
          dmach = &g_dmach[ch];
          dmach->inuse    = true;
          dmach->flags    = chflags;
          dmach->busy     = false;
          dmach->callback = NULL;
          dmach->arg      = NULL;

          /* Clear any pending flags */

          dmach_putreg(dmach, SAM_DMACH_INTF_OFFSET, DMA_CHINTF_ALL);
          break;
        }
    }

  nxsem_post(&g_chsem);
  return (DMA_HANDLE)dmach;
}

/****************************************************************************
 * Name: sam_dmaconfig
 ****************************************************************************/

void sam_dmaconfig(DMA_HANDLE handle, uint32_t chflags)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;
  DEBUGASSERT(dmach && dmach->inuse && !dmach->busy);
  dmach->flags = chflags;
}

/****************************************************************************
 * Name: sam_dmafree
 ****************************************************************************/

void sam_dmafree(DMA_HANDLE handle)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;

  DEBUGASSERT(dmach && dmach->inuse);

  if (dmach->busy)
    {
      sam_dmastop(handle);
    }

  nxsem_wait_uninterruptible(&g_chsem);
  dmach->inuse = false;
  nxsem_post(&g_chsem);
}

/****************************************************************************
 * Name: sam_dmarxsetup
 ****************************************************************************/

int sam_dmarxsetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                   size_t nbytes)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;

  DEBUGASSERT(dmach && dmach->inuse && !dmach->busy);
  DEBUGASSERT(nbytes > 0 && nbytes <= 65535);

  uint32_t trig = (dmach->flags & DMACH_FLAG_PERIPHPID_MASK) >>
                   DMACH_FLAG_PERIPHPID_SHIFT;
  uint32_t pri  = (dmach->flags & DMACH_FLAG_PRIORITY_MASK) >>
                   DMACH_FLAG_PRIORITY_SHIFT;

  dmach->rx     = true;
  dmach->rxaddr = maddr;
  dmach->rxsize = nbytes;

  /* Source = peripheral (fixed byte), Dest = memory (byte increment) */

  dmach_putreg(dmach, SAM_DMACH_SSA_OFFSET, paddr);
  dmach_putreg(dmach, SAM_DMACH_DSA_OFFSET, maddr);

  /* Cell size = 1 byte per trigger, Block size = total bytes */

  dmach_putreg(dmach, SAM_DMACH_XSIZ_OFFSET,
               DMA_CHXSIZ_CSZ(1) | DMA_CHXSIZ_BLKSZ(nbytes));

  /* CHCTRLB: RAS=FIXED_BYTE (read from peripheral),
   *          WAS depends on NOINC flag (normal=BYTE_INCR, NULL-buf=FIXED),
   *          TRIG=trigger_id, PRI=priority, CASTEN=0
   */

  uint32_t was = (dmach->flags & DMACH_FLAG_NOINC) ?
                  DMA_WAS_FIXED_BYTE : DMA_WAS_BYTE_INCR;

  uint32_t ctrlb = DMA_CHCTRLB_RAS(DMA_RAS_FIXED_BYTE) |
                   DMA_CHCTRLB_WAS(was) |
                   DMA_CHCTRLB_TRIG(trig) |
                   DMA_CHCTRLB_PRI(pri);

  dmach_putreg(dmach, SAM_DMACH_CTRLB_OFFSET, ctrlb);

  /* Invalidate D-cache for RX buffer before DMA writes to it */

  up_invalidate_dcache(maddr, maddr + nbytes);

  return OK;
}

/****************************************************************************
 * Name: sam_dmatxsetup
 ****************************************************************************/

int sam_dmatxsetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                   size_t nbytes)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;

  DEBUGASSERT(dmach && dmach->inuse && !dmach->busy);
  DEBUGASSERT(nbytes > 0 && nbytes <= 65535);

  uint32_t trig = (dmach->flags & DMACH_FLAG_PERIPHPID_MASK) >>
                   DMACH_FLAG_PERIPHPID_SHIFT;
  uint32_t pri  = (dmach->flags & DMACH_FLAG_PRIORITY_MASK) >>
                   DMACH_FLAG_PRIORITY_SHIFT;

  dmach->rx     = false;
  dmach->rxaddr = 0;
  dmach->rxsize = 0;

  /* Source = memory (byte increment), Dest = peripheral (fixed byte) */

  dmach_putreg(dmach, SAM_DMACH_SSA_OFFSET, maddr);
  dmach_putreg(dmach, SAM_DMACH_DSA_OFFSET, paddr);

  /* Cell size = 1 byte per trigger, Block size = total bytes */

  dmach_putreg(dmach, SAM_DMACH_XSIZ_OFFSET,
               DMA_CHXSIZ_CSZ(1) | DMA_CHXSIZ_BLKSZ(nbytes));

  /* CHCTRLB: RAS depends on NOINC flag (normal=BYTE_INCR, NULL-buf=FIXED),
   *          WAS=FIXED_BYTE (write to peripheral),
   *          TRIG=trigger_id, PRI=priority, CASTEN=0
   */

  uint32_t ras = (dmach->flags & DMACH_FLAG_NOINC) ?
                  DMA_RAS_FIXED_BYTE : DMA_RAS_BYTE_INCR;

  uint32_t ctrlb = DMA_CHCTRLB_RAS(ras) |
                   DMA_CHCTRLB_WAS(DMA_WAS_FIXED_BYTE) |
                   DMA_CHCTRLB_TRIG(trig) |
                   DMA_CHCTRLB_PRI(pri);

  dmach_putreg(dmach, SAM_DMACH_CTRLB_OFFSET, ctrlb);

  /* Clean D-cache so DMA reads current data from RAM */

  up_clean_dcache(maddr, maddr + nbytes);

  return OK;
}

/****************************************************************************
 * Name: sam_dmastart
 ****************************************************************************/

int sam_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;
  irqstate_t flags;

  DEBUGASSERT(dmach && dmach->inuse && !dmach->busy);

  flags = enter_critical_section();

  dmach->callback = callback;
  dmach->arg      = arg;
  dmach->busy     = true;

  /* Clear all pending interrupt flags */

  dmach_putreg(dmach, SAM_DMACH_INTF_OFFSET, DMA_CHINTF_ALL);

  /* Enable Block Complete + error interrupts */

  dmach_putreg(dmach, SAM_DMACH_INTENSET_OFFSET,
               DMA_CHINTF_BC | DMA_CHINTF_WRE | DMA_CHINTF_RDE);

  /* Memory barriers before enabling channel */

  ARM_DSB();
  ARM_DMB();

  /* Enable channel — starts transfer on next hardware trigger */

  uint32_t ctrla = dmach_getreg(dmach, SAM_DMACH_CTRLA_OFFSET);
  ctrla |= DMA_CHCTRLA_ENABLE;
  dmach_putreg(dmach, SAM_DMACH_CTRLA_OFFSET, ctrla);

  leave_critical_section(flags);
  return OK;
}

/****************************************************************************
 * Name: sam_dmastop
 ****************************************************************************/

void sam_dmastop(DMA_HANDLE handle)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;
  irqstate_t flags;

  DEBUGASSERT(dmach && dmach->inuse);

  flags = enter_critical_section();

  if (dmach->busy)
    {
      sam_dma_terminate(dmach, -EINTR);
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: sam_dmaresidual
 ****************************************************************************/

size_t sam_dmaresidual(DMA_HANDLE handle)
{
  struct sam_dmach_s *dmach = (struct sam_dmach_s *)handle;

  DEBUGASSERT(dmach && dmach->inuse);

  uint32_t xsiz = dmach_getreg(dmach, SAM_DMACH_XSIZ_OFFSET);
  return (size_t)((xsiz & DMA_CHXSIZ_BLKSZ_MASK) >> DMA_CHXSIZ_BLKSZ_SHIFT);
}
