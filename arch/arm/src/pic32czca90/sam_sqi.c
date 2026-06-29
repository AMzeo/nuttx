/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sqi.c
 *
 * NuttX SPI lower-half driver wrapping the PIC32CZ CA90 SQI1 peripheral.
 *
 * BD-DMA mode — production driver uses BD-DMA for all flash operations.
 *
 * Transfer model — half-duplex BDs:
 *   SQI DMA is half-duplex at the BD level.  TX BD (DIR=0) drives IO0
 *   (MOSI) and discards IO1 (MISO).  RX BD (DIR=1) clocks dummy bytes on
 *   IO0 and captures IO1 (MISO) bytes into bd_bufaddr.
 *
 *   txbuffer != NULL → TX BD: sends txbuffer bytes, rxbuffer ignored.
 *   txbuffer == NULL → RX BD: captures chunk bytes into rxbuffer.
 *
 * Flash JEDEC probe sequence (CS asserted across both):
 *   sqi_exchange(dev, cmd, NULL, 1)   — TX BD, sends 0x9F
 *   sqi_exchange(dev, NULL, id, 3)    — RX BD, captures BF 26 42
 *
 * Sequence per chunk (TX BD):
 *   1. Copy TX bytes to g_sqi_tx_buf.
 *   2. CFG |= RXBUFRST (flush RXFIFO, flush stale data).
 *   3. Fill g_sqi_tx_desc: DESC_EN|CS_ASSERT|LAST_BD|CBD_INT_EN|BUFLEN(n).
 *   4. BDBASEADD = &g_sqi_tx_desc; BDCON = START|DMAEN.
 *   5. Poll INTSTAT for BDDONE; BDCON = 0; clear INTSTAT.
 *
 * Sequence per chunk (RX BD):
 *   1. CFG |= RXBUFRST (flush RXFIFO, flush stale data).
 *   2. Fill g_sqi_rx_desc: DESC_EN|CS_ASSERT|LAST_BD|CBD_INT_EN|DIR|BUFLEN(n).
 *      bd_bufaddr = g_sqi_rx_buf (nocache).
 *   3. BDBASEADD = &g_sqi_rx_desc; BDCON = START|DMAEN.
 *   4. Poll INTSTAT for BDDONE; BDCON = 0; clear INTSTAT.
 *   5. Copy g_sqi_rx_buf[0..chunk-1] to rxbuffer.
 *
 * CS management:
 *   sqi_select(true)  — BDCON=0; drain stale RXFIFO bytes.
 *   sqi_select(false) — toggle SQIEN to force CS high.
 *   LIFM=0 in bd_ctrl — CS stays asserted between sqi_exchange() calls.
 *
 * Reference: PIC32CZ8110CA90208 component/sqi.h, instance/sqi1.h
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_SQI1

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>
#include <syslog.h>
#include <nuttx/cache.h>

#include <nuttx/arch.h>
#include <nuttx/mutex.h>
#include <nuttx/spi/spi.h>

#include "arm_internal.h"
#include "sam_gclk.h"
#include "sam_port.h"
#include "hardware/sam_gclk.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_port.h"
#include "hardware/sam_sqi.h"
#include "hardware/pic32czca90_pinmap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SQI1_GCLK_GEN     2u
#define SQI1_GCLK_HZ      100000000u
#define SQI1_CLKDIV_50MHZ 1u

/* Max bytes per DMA chunk — BUFLEN field is 9 bits (max 511; use 256 to
 * keep well within RXFIFO depth and avoid overflow) */

#define SQI_DMA_BUF_SIZE  512u

#define SQI_DMA_TIMEOUT   500000u

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_sqi_dev_s
{
  struct spi_dev_s spi;
  mutex_t          lock;
  bool             cs_active;
  bool             xip_configured;  /* true after sam_sqi_xip_enable() */
  uint32_t         frequency;
  uint8_t          nbits;
  uint8_t          mode;
  size_t           pending_tx_len;  /* TX bytes staged in g_sqi_tx_buf awaiting RX or deselect */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int      sqi_lock(FAR struct spi_dev_s *dev, bool lock);
static int      sqi_dma_run(sqi_dma_desc_t *desc, size_t chunk, size_t offset);
static void     sqi_select(FAR struct spi_dev_s *dev, uint32_t devid,
                            bool selected);
static uint32_t sqi_setfrequency(FAR struct spi_dev_s *dev, uint32_t freq);
static void     sqi_setmode(FAR struct spi_dev_s *dev, enum spi_mode_e mode);
static void     sqi_setbits(FAR struct spi_dev_s *dev, int nbits);
static uint32_t sqi_send(FAR struct spi_dev_s *dev, uint32_t wd);
static void     sqi_exchange(FAR struct spi_dev_s *dev,
                              FAR const void *txbuffer,
                              FAR void *rxbuffer, size_t nwords);
static void     sqi_full_reset(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct spi_ops_s g_sqi_ops =
{
  .lock          = sqi_lock,
  .select        = sqi_select,
  .setfrequency  = sqi_setfrequency,
  .setmode       = sqi_setmode,
  .setbits       = sqi_setbits,
  .exchange      = sqi_exchange,
  .send          = sqi_send,
};

static struct sam_sqi_dev_s g_sqi1_dev =
{
  .spi       = { .ops = &g_sqi_ops },
  .lock      = NXMUTEX_INITIALIZER,
  .frequency = 0,
  .nbits     = 8,
  .mode      = SPIDEV_MODE0,
  .cs_active = false,
};

/* TX DMA descriptor and buffer — nocache for DMA coherency */

static sqi_dma_desc_t g_sqi_tx_desc
  __attribute__((section(".nocache"), aligned(32)));

static uint8_t g_sqi_tx_buf[SQI_DMA_BUF_SIZE]
  __attribute__((section(".nocache")));

/* RX DMA descriptor and buffer — nocache for DMA coherency */

static sqi_dma_desc_t g_sqi_rx_desc
  __attribute__((section(".nocache"), aligned(32)));

static uint8_t g_sqi_rx_buf[SQI_DMA_BUF_SIZE]
  __attribute__((section(".nocache")));

/****************************************************************************
 * Private Helpers
 ****************************************************************************/

static inline uint32_t sqi_getreg(uint32_t offset)
{
  return getreg32(SAM_SQI1_BASE + offset);
}

static inline void sqi_putreg(uint32_t offset, uint32_t val)
{
  putreg32(val, SAM_SQI1_BASE + offset);
}

static inline void sqi_modreg(uint32_t offset, uint32_t clr, uint32_t set)
{
  uint32_t v = getreg32(SAM_SQI1_BASE + offset);
  v = (v & ~clr) | set;
  putreg32(v, SAM_SQI1_BASE + offset);
}

static void sqi_mclk_enable(void)
{
  uint32_t id  = SAM_SQI1_MCLK_ID_AHB;
  uint32_t reg = SAM_MCLK_CLKMSK_ADDR(id);
  uint32_t bit = SAM_MCLK_CLKMSK_BIT(id);
  putreg32(getreg32(reg) | bit, reg);
}

/****************************************************************************
 * SPI Operations
 ****************************************************************************/

static int sqi_lock(FAR struct spi_dev_s *dev, bool lock)
{
  struct sam_sqi_dev_s *priv = (struct sam_sqi_dev_s *)dev;

  if (lock)
    return nxmutex_lock(&priv->lock);
  else
    return nxmutex_unlock(&priv->lock);
}

static void sqi_select(FAR struct spi_dev_s *dev, uint32_t devid,
                        bool selected)
{
  struct sam_sqi_dev_s *priv = (struct sam_sqi_dev_s *)dev;
  uint32_t cfg;

  priv->cs_active = selected;

  if (selected)
    {
      /* Ensure DMA mode for SPI transactions.  If XIP was active, disable
       * SQIEN first to cleanly exit XIP, then switch mode, then re-enable. */

      if (priv->xip_configured &&
          (sqi_getreg(SAM_SQI_CFG_OFFSET) & SQI_CFG_MODE_MASK) == SQI_CFG_MODE_XIP)
        {
          sqi_modreg(SAM_SQI_CFG_OFFSET, SQI_CFG_SQIEN, 0);
          sqi_modreg(SAM_SQI_CFG_OFFSET, SQI_CFG_MODE_MASK, SQI_CFG_MODE_DMA);
          sqi_modreg(SAM_SQI_CFG_OFFSET, 0, SQI_CFG_SQIEN);
        }

      /* Stop any in-progress DMA */

      sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
      priv->pending_tx_len = 0;

      /* Drain stale RXFIFO bytes left from previous transaction. */

      while (sqi_getreg(SAM_SQI_STAT1_OFFSET) & SQI_STAT1_RXBUFCNT_MASK)
        {
          (void)sqi_getreg(SAM_SQI_RXDATA_OFFSET);
        }
    }
  else
    {
      /* Flush any pending TX-only command (e.g. WREN, RST) that was
       * never followed by an RX phase.  Use LIFM to deassert CS. */

      if (priv->pending_tx_len > 0)
        {
          g_sqi_tx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                                      SQI_BDCTRL_LAST_BD    |
                                      SQI_BDCTRL_LIFM       |
                                      SQI_BDCTRL_CBD_INT_EN |
                                      SQI_BDCTRL_BUFLEN(priv->pending_tx_len);
          g_sqi_tx_desc.bd_stat    = 0;
          g_sqi_tx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_tx_buf;
          g_sqi_tx_desc.bd_nxtptr  = NULL;

          sqi_dma_run(&g_sqi_tx_desc, priv->pending_tx_len, 0);
          priv->pending_tx_len = 0;
        }

      /* Deassert CS0: briefly disable then re-enable SQIEN */

      cfg = sqi_getreg(SAM_SQI_CFG_OFFSET);
      sqi_putreg(SAM_SQI_CFG_OFFSET, cfg & ~SQI_CFG_SQIEN);
      sqi_putreg(SAM_SQI_CFG_OFFSET, cfg);

      /* Stay in DMA mode — don't switch back to XIP here.
       * XIP is re-entered only by qspi_xip_read/bread. */
    }
}

static uint32_t sqi_setfrequency(FAR struct spi_dev_s *dev, uint32_t freq)
{
  uint32_t divider;
  uint32_t clkdiv;
  uint32_t actual;

  if (freq == 0)
    {
      freq = 1;
    }

  divider = (SQI1_GCLK_HZ + 2u * freq - 1u) / (2u * freq);
  if (divider < 1u)
    {
      divider = 1u;
    }

  clkdiv = 1u;
  while (clkdiv < divider)
    {
      clkdiv <<= 1;
    }

  if (clkdiv > 0x400u)
    {
      clkdiv = 0x400u;
    }

  sqi_modreg(SAM_SQI_CLKCON_OFFSET,
             SQI_CLKCON_CLKDIV_MASK,
             SQI_CLKCON_CLKDIV(clkdiv));
  while ((sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE) == 0)
    {
    }

  actual = SQI1_GCLK_HZ / (2u * clkdiv);
  ((struct sam_sqi_dev_s *)dev)->frequency = actual;
  return actual;
}

static void sqi_setmode(FAR struct spi_dev_s *dev, enum spi_mode_e mode)
{
  uint32_t cfg = sqi_getreg(SAM_SQI_CFG_OFFSET) & ~(SQI_CFG_CPHA | SQI_CFG_CPOL);

  switch (mode)
    {
      case SPIDEV_MODE0:                                      break;
      case SPIDEV_MODE1: cfg |= SQI_CFG_CPHA;                break;
      case SPIDEV_MODE2: cfg |= SQI_CFG_CPOL;                break;
      case SPIDEV_MODE3: cfg |= SQI_CFG_CPHA | SQI_CFG_CPOL; break;
      default:                                                break;
    }

  sqi_putreg(SAM_SQI_CFG_OFFSET, cfg);
  ((struct sam_sqi_dev_s *)dev)->mode = (uint8_t)mode;
}

static void sqi_setbits(FAR struct spi_dev_s *dev, int nbits)
{
  ((struct sam_sqi_dev_s *)dev)->nbits = (uint8_t)nbits;
}

/****************************************************************************
 * sqi_dma_run — submit one BD chain and poll for BDDONE.
 * Returns 0 on success, -1 on timeout.
 ****************************************************************************/

static int sqi_dma_run(sqi_dma_desc_t *desc, size_t chunk, size_t offset)
{
  uint32_t timeout;

  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
  sqi_putreg(SAM_SQI_BDBASEADD_OFFSET, (uint32_t)(uintptr_t)desc);
  sqi_putreg(SAM_SQI_BDCON_OFFSET, SQI_BDCON_START | SQI_BDCON_DMAEN);

  timeout = SQI_DMA_TIMEOUT;
  while (!(sqi_getreg(SAM_SQI_INTSTAT_OFFSET) &
           (SQI_INT_BDDONE | SQI_INT_PKTCOMP)))
    {
      if (--timeout == 0)
        {
          syslog(LOG_ERR,
                 "[sqi] DMA timeout chunk=%zu offset=%zu "
                 "INTSTAT=%08lx BDSTAT=%08lx CFG=%08lx "
                 "BDCON=%08lx STAT1=%08lx BDCURADD=%08lx\n",
                 chunk, offset,
                 (unsigned long)sqi_getreg(SAM_SQI_INTSTAT_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDSTAT_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_CFG_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDCON_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_STAT1_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDCURADD_OFFSET));
          sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
          return -1;
        }
    }

  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, sqi_getreg(SAM_SQI_INTSTAT_OFFSET));
  return 0;
}

/****************************************************************************
 * sqi_exchange — SPI exchange using linked BD chains.
 *
 * CS cannot stay LOW between separate DMA runs (hardware deasserts CS when
 * idle). To hold CS across command+response, TX bytes are staged in
 * g_sqi_tx_buf. When RX is requested (txbuffer==NULL), a linked TX→RX BD
 * chain is submitted atomically (same pattern as sam_sqi_flash_cmd_read).
 *
 * Patterns:
 *   exchange(cmd, NULL, 1)    — stage TX (no DMA yet)
 *   exchange(NULL, data, n)   — execute linked TX→RX chain, CS held
 *   select(false)             — flush any pending TX-only (e.g. WREN)
 *   exchange(tx, rx, 1)       — spi_send: single TX BD + drain RXFIFO
 ****************************************************************************/

static void sqi_exchange(FAR struct spi_dev_s *dev,
                          FAR const void *txbuffer,
                          FAR void *rxbuffer, size_t nwords)
{
  struct sam_sqi_dev_s *priv = (struct sam_sqi_dev_s *)dev;
  const uint8_t *tx  = (const uint8_t *)txbuffer;
  uint8_t       *rx  = (uint8_t *)rxbuffer;
  size_t         i;

  if (tx && rx)
    {
      /* Full-duplex (SPI_SEND path).
       *
       * SAFETY: Replay is only safe for READ commands (RDID, RDSR).
       * For WRITE commands (erase, program), intermediate replays with
       * WEL set could accidentally execute at wrong addresses.
       *
       * Strategy:
       *   Read commands (0x9F, 0x05, 0x35): replay TX(1:cmd)→RX(N)
       *   Write/other: just stage bytes, return 0xFF (caller discards) */

      for (i = 0; i < nwords && priv->pending_tx_len < SQI_DMA_BUF_SIZE; i++)
        {
          g_sqi_tx_buf[priv->pending_tx_len++] = tx[i];
        }

      uint8_t cmd = g_sqi_tx_buf[0];
      bool is_read_cmd = (cmd == 0x9Fu || cmd == 0x05u || cmd == 0x35u);

      if (!is_read_cmd || priv->pending_tx_len <= 1)
        {
          /* Write command or first byte: stage only, return 0xFF. */

          rx[0] = 0xFF;
        }
      else
        {
          /* Read-command replay: TX(1:cmd) → RX(N-1).
           * Returns the last captured byte (current response position). */

          size_t rx_count = priv->pending_tx_len - 1;
          uint32_t timeout;

          g_sqi_tx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN |
                                      SQI_BDCTRL_BUFLEN(1);
          g_sqi_tx_desc.bd_stat    = 0;
          g_sqi_tx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_tx_buf;
          g_sqi_tx_desc.bd_nxtptr  = &g_sqi_rx_desc;

          g_sqi_rx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                                      SQI_BDCTRL_LAST_BD    |
                                      SQI_BDCTRL_LIFM       |
                                      SQI_BDCTRL_DIR        |
                                      SQI_BDCTRL_PKT_INT_EN |
                                      SQI_BDCTRL_BUFLEN(rx_count);
          g_sqi_rx_desc.bd_stat    = 0;
          g_sqi_rx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_rx_buf;
          g_sqi_rx_desc.bd_nxtptr  = NULL;

          sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
          sqi_putreg(SAM_SQI_BDBASEADD_OFFSET,
                     (uint32_t)(uintptr_t)&g_sqi_tx_desc);
          sqi_putreg(SAM_SQI_BDCON_OFFSET,
                     SQI_BDCON_START | SQI_BDCON_DMAEN);

          timeout = SQI_DMA_TIMEOUT;
          while (!(sqi_getreg(SAM_SQI_INTSTAT_OFFSET) & SQI_INT_PKTCOMP))
            {
              if (--timeout == 0)
                {
                  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
                  rx[0] = 0xFF;
                  return;
                }
            }

          sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
          sqi_putreg(SAM_SQI_INTSTAT_OFFSET,
                     sqi_getreg(SAM_SQI_INTSTAT_OFFSET));

          rx[0] = g_sqi_rx_buf[rx_count - 1];
        }
    }
  else if (tx)
    {
      /* TX-only: stage bytes for later linked-chain execution.
       * Append to pending buffer (supports multi-byte commands). */

      for (i = 0; i < nwords && priv->pending_tx_len < SQI_DMA_BUF_SIZE; i++)
        {
          g_sqi_tx_buf[priv->pending_tx_len++] = tx[i];
        }
    }
  else if (rx)
    {
      /* RX requested: execute linked TX→RX chain (CS held throughout).
       * If no pending TX, just do standalone RX. */

      size_t rxlen = nwords;
      if (rxlen > SQI_DMA_BUF_SIZE)
        {
          rxlen = SQI_DMA_BUF_SIZE;
        }

      if (priv->pending_tx_len > 0)
        {
          /* Linked chain: TX BD (staged command) → RX BD (response) */

          g_sqi_tx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN |
                                      SQI_BDCTRL_BUFLEN(priv->pending_tx_len);
          g_sqi_tx_desc.bd_stat    = 0;
          g_sqi_tx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_tx_buf;
          g_sqi_tx_desc.bd_nxtptr  = &g_sqi_rx_desc;

          g_sqi_rx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                                      SQI_BDCTRL_LAST_BD    |
                                      SQI_BDCTRL_LIFM       |
                                      SQI_BDCTRL_DIR        |
                                      SQI_BDCTRL_PKT_INT_EN |
                                      SQI_BDCTRL_BUFLEN(rxlen);
          g_sqi_rx_desc.bd_stat    = 0;
          g_sqi_rx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_rx_buf;
          g_sqi_rx_desc.bd_nxtptr  = NULL;

          /* Submit and poll PKTCOMP */

          uint32_t timeout;
          sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
          sqi_putreg(SAM_SQI_BDBASEADD_OFFSET,
                     (uint32_t)(uintptr_t)&g_sqi_tx_desc);
          sqi_putreg(SAM_SQI_BDCON_OFFSET,
                     SQI_BDCON_START | SQI_BDCON_DMAEN);

          timeout = SQI_DMA_TIMEOUT;
          while (!(sqi_getreg(SAM_SQI_INTSTAT_OFFSET) & SQI_INT_PKTCOMP))
            {
              if (--timeout == 0)
                {
                  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
                  priv->pending_tx_len = 0;
                  return;
                }
            }

          sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
          sqi_putreg(SAM_SQI_INTSTAT_OFFSET,
                     sqi_getreg(SAM_SQI_INTSTAT_OFFSET));
          priv->pending_tx_len = 0;

          for (i = 0; i < rxlen; i++)
            {
              rx[i] = g_sqi_rx_buf[i];
            }
        }
      else
        {
          /* No pending TX — standalone RX BD (e.g. bulk read after cmd) */

          g_sqi_rx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                                      SQI_BDCTRL_LAST_BD    |
                                      SQI_BDCTRL_CBD_INT_EN |
                                      SQI_BDCTRL_DIR        |
                                      SQI_BDCTRL_BUFLEN(rxlen);
          g_sqi_rx_desc.bd_stat    = 0;
          g_sqi_rx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_rx_buf;
          g_sqi_rx_desc.bd_nxtptr  = NULL;

          if (sqi_dma_run(&g_sqi_rx_desc, rxlen, 0) < 0)
            {
              return;
            }

          for (i = 0; i < rxlen; i++)
            {
              rx[i] = g_sqi_rx_buf[i];
            }
        }
    }
}

static uint32_t sqi_send(FAR struct spi_dev_s *dev, uint32_t wd)
{
  uint8_t tx = (uint8_t)wd;
  uint8_t rx = 0;
  sqi_exchange(dev, &tx, &rx, 1);
  return rx;
}

/****************************************************************************
 * Public Function: sam_sqi_flash_cmd_read
 *
 * Submit a linked TX BD → RX BD chain in a single DMA operation so CS
 * stays asserted across command and response phases.
 *
 * TX BD: no CBD_INT_EN, no LAST_BD, bd_nxtptr → g_sqi_rx_desc.
 * RX BD: CBD_INT_EN + LAST_BD + LIFM.
 *
 * Polls PKTCOMP (fires at LIFM = end of SPI transaction), NOT BDDONE.
 * BDDONE may fire per-BD regardless of CBD_INT_EN (hardware-dependent);
 * PKTCOMP only fires once at the LIFM-marked BD — safe for multi-BD chains.
 ****************************************************************************/

int sam_sqi_flash_cmd_read(FAR struct spi_dev_s *dev,
                            FAR const uint8_t *cmd, size_t cmdlen,
                            FAR uint8_t *data, size_t datalen)
{
  uint32_t timeout;
  uint32_t intstat;
  size_t   i;

  if (cmdlen == 0 || cmdlen > SQI_DMA_BUF_SIZE ||
      datalen == 0 || datalen > SQI_DMA_BUF_SIZE)
    {
      return -EINVAL;
    }

  /* SWRST + full reinit — clears BD processor cache (Michigan Ax) */

  sqi_full_reset();

  for (i = 0; i < cmdlen; i++)
    {
      g_sqi_tx_buf[i] = cmd[i];
    }

  /* Flush D-Cache for TX buffer + descriptors (no MPU nocache region yet) */

  {
    uintptr_t addr;
    for (addr = (uintptr_t)g_sqi_tx_buf & ~31u;
         addr < (uintptr_t)g_sqi_tx_buf + SQI_DMA_BUF_SIZE;
         addr += 32)
      putreg32(addr, 0xE000EF68u);  /* DCCMVAC */
    for (addr = (uintptr_t)&g_sqi_tx_desc & ~31u;
         addr < (uintptr_t)&g_sqi_tx_desc + 32;
         addr += 32)
      putreg32(addr, 0xE000EF68u);
    for (addr = (uintptr_t)&g_sqi_rx_desc & ~31u;
         addr < (uintptr_t)&g_sqi_rx_desc + 32;
         addr += 32)
      putreg32(addr, 0xE000EF68u);
    __asm__ volatile ("dsb sy" ::: "memory");
  }

  /* TX BD — chains to RX BD */

  g_sqi_tx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                              SQI_BDCTRL_BUFLEN(cmdlen);
  g_sqi_tx_desc.bd_stat    = 0;
  g_sqi_tx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_tx_buf;
  g_sqi_tx_desc.bd_nxtptr  = &g_sqi_rx_desc;

  /* RX BD — CBD_INT_EN + LAST_BD + LIFM */

  /* PKT_INT_EN triggers PKTCOMP in INTSTAT when this LIFM BD completes.
   * On Michigan Ax, BDCON.DMAEN does not auto-clear — PKTCOMP is the only
   * reliable completion signal for linked BD chains. */

  g_sqi_rx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                              SQI_BDCTRL_LAST_BD    |
                              SQI_BDCTRL_LIFM       |
                              SQI_BDCTRL_DIR        |
                              SQI_BDCTRL_PKT_INT_EN |
                              SQI_BDCTRL_BUFLEN(datalen);
  g_sqi_rx_desc.bd_stat    = 0;
  g_sqi_rx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_rx_buf;
  g_sqi_rx_desc.bd_nxtptr  = NULL;


  /* Force CS HIGH between commands (t_CSH >= 50 ns for SST26) */

  {
    uint32_t c = sqi_getreg(SAM_SQI_CFG_OFFSET);
    volatile int _dly;
    sqi_putreg(SAM_SQI_CFG_OFFSET, c & ~SQI_CFG_SQIEN);
    for (_dly = 0; _dly < 50; _dly++) { __asm__ volatile ("nop"); }
    sqi_putreg(SAM_SQI_CFG_OFFSET, c);
  }

  /* Drain stale RXFIFO bytes */

  while (sqi_getreg(SAM_SQI_STAT1_OFFSET) & SQI_STAT1_RXBUFCNT_MASK)
    {
      (void)sqi_getreg(SAM_SQI_RXDATA_OFFSET);
    }

  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
  sqi_modreg(SAM_SQI_CFG_OFFSET, 0, SQI_CFG_RXBUFRST);
  sqi_putreg(SAM_SQI_BDBASEADD_OFFSET, (uint32_t)(uintptr_t)&g_sqi_tx_desc);
  sqi_putreg(SAM_SQI_BDCON_OFFSET, SQI_BDCON_START | SQI_BDCON_DMAEN);

  timeout = SQI_DMA_TIMEOUT;
  while (!(sqi_getreg(SAM_SQI_INTSTAT_OFFSET) & SQI_INT_PKTCOMP))
    {
      if (--timeout == 0)
        {
          intstat = sqi_getreg(SAM_SQI_INTSTAT_OFFSET);
          syslog(LOG_ERR,
                 "[sqi] cmd_read timeout INTSTAT=%08lx BDSTAT=%08lx"
                 " BDRXDSTAT=%08lx BDTXDSTAT=%08lx\n"
                 "[sqi]   BDCON=%08lx BDCURADD=%08lx STAT1=%08lx STAT2=%08lx\n"
                 "[sqi]   tx_bd_ctrl=%08lx rx_bd_ctrl=%08lx"
                 " rxbuf=%02x %02x %02x\n",
                 (unsigned long)intstat,
                 (unsigned long)sqi_getreg(SAM_SQI_BDSTAT_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDRXDSTAT_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDTXDSTAT_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDCON_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_BDCURADD_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_STAT1_OFFSET),
                 (unsigned long)sqi_getreg(SAM_SQI_STAT2_OFFSET),
                 (unsigned long)g_sqi_tx_desc.bd_ctrl,
                 (unsigned long)g_sqi_rx_desc.bd_ctrl,
                 (unsigned)g_sqi_rx_buf[0],
                 (unsigned)g_sqi_rx_buf[1],
                 (unsigned)g_sqi_rx_buf[2]);
          sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
          return -EIO;
        }
    }

  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, sqi_getreg(SAM_SQI_INTSTAT_OFFSET));

  /* Invalidate D-Cache for RX buffer (no MPU nocache region yet) */

  {
    uintptr_t addr;
    for (addr = (uintptr_t)g_sqi_rx_buf & ~31u;
         addr < (uintptr_t)g_sqi_rx_buf + datalen;
         addr += 32)
      putreg32(addr, 0xE000EF5Cu);  /* DCIMVAC */
    __asm__ volatile ("dsb sy" ::: "memory");
  }

  for (i = 0; i < datalen; i++)
    {
      data[i] = g_sqi_rx_buf[i];
    }

  return 0;
}

/****************************************************************************
 * Public Function: sam_sqibus_initialize
 *
 * Init sequence:
 *   GCLK/MCLK → pins → SWRST → CFG(DMA,BURSTEN,DATAEN=0,CSEN0) →
 *   CLKCON(EN→STABLE→CLKDIV) → SQIEN → CMDTHR(32,32) → INTTHR(1,1) →
 *   THR(1) → INTEN(PKTCOMP|BDDONE) → INTSIGEN(same) → BDCON=0
 *
 * CFG value: MODE=DMA(2), BURSTEN(bit11), DATAEN=0(single-lane SPI), CSEN0.
 * NOT DATAEN=2 (quad) — SST26 starts in SPI mode; quad needs QPIEN first.
 ****************************************************************************/

FAR struct spi_dev_s *sam_sqibus_initialize(int bus)
{
  struct sam_sqi_dev_s *priv = &g_sqi1_dev;

  if (bus != 1)
    {
      return NULL;
    }


  /* 1. Mux SQI1 pins — IO2/IO3 PULLEN+OUTVAL_HIGH keeps WP#/HOLD# high */

  sam_portconfig(PORT_SQI1_CLK);
  sam_portconfig(PORT_SQI1_CS0);
  sam_portconfig(PORT_SQI1_IO0);
  sam_portconfig(PORT_SQI1_IO1);
  sam_portconfig(PORT_SQI1_IO2);
  sam_portconfig(PORT_SQI1_IO3);

  /* Read back PORT registers to confirm PMUXEN=1 and func=7 (FUNC_H) on
   * every SQI1 pin.  Expected:
   *   PORTG PINCFG[0]=03 PINCFG[3]=03 PMUX[0]=77 PMUX[1]=77
   *   PORTC PINCFG[30]=01 PINCFG[31]=03 PMUX[15]=77
   * PMUXEN=bit0 of PINCFG; PMUX nibble=7 means FUNC_H=SQI1.
   * If any PINCFG shows bit0=0, that pin is still GPIO (not muxed). */



  /* 2. Enable GCLK2 → SQI1 (GCLK_PCHCTRL[57]) */

  sam_gclk_chan_enable(SAM_SQI1_GCLK_ID, SQI1_GCLK_GEN, false);

  /* 3. Enable MCLK AHB clock for SQI1 */

  sqi_mclk_enable();

  /* 4. Software reset */

  putreg8(SQI_CTRLA_SWRST, SAM_SQI1_CTRLA);
  while (getreg8(SAM_SQI1_SYNCBUSY) & SQI_SYNCBUSY_SWRST)
    {
    }

  /* 5. Configure CFG: DMA mode, BURSTEN(bit11), 1-lane SPI, CS0
   *    DATAEN=0: IO0=MOSI output, IO1=MISO input (single-lane SPI).
   *    WP=0, HOLD=0: keep IO2/IO3 high via PORT pullup. */

  sqi_putreg(SAM_SQI_CFG_OFFSET,
             SQI_CFG_MODE_DMA  |
             SQI_CFG_BURSTEN   |
             SQI_CFG_DATAEN(0) |   /* single-lane SPI */
             SQI_CFG_CSEN0);

  /* 6. Enable clock and wait for stable */

  sqi_putreg(SAM_SQI_CLKCON_OFFSET, SQI_CLKCON_EN);
  while ((sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE) == 0)
    {
    }

  /* 7. Set clock divider: CLKDIV=1 → 100 MHz / 2 = 50 MHz SCK */

  sqi_modreg(SAM_SQI_CLKCON_OFFSET,
             SQI_CLKCON_CLKDIV_MASK,
             SQI_CLKCON_CLKDIV(SQI1_CLKDIV_50MHZ));
  while ((sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE) == 0)
    {
    }


  /* 8. Enable SQI block */

  sqi_modreg(SAM_SQI_CFG_OFFSET, 0, SQI_CFG_SQIEN);

  /* 9. CMDTHR: RXCMDTHR=1 so DMA can consume RXFIFO with as few as 1 byte.
   *    Higher values stall on short transfers (e.g. 3-byte JEDEC). */

  sqi_putreg(SAM_SQI_CMDTHR_OFFSET,
             SQI_CMDTHR_RXCMDTHR(0x01u) | SQI_CMDTHR_TXCMDTHR(0x01u));

  /* 10. INTTHR = 1 for both RX and TX */

  sqi_putreg(SAM_SQI_INTTHR_OFFSET,
             SQI_INTTHR_RXINTTHR(0x01u) | SQI_INTTHR_TXINTTHR(0x01u));

  /* 11. THR = 1 */

  sqi_putreg(SAM_SQI_THR_OFFSET, SQI_THR_THRES(1u));

  /* 12. Enable internal events so INTSTAT reflects BDDONE/PKTCOMP (polled) */

  sqi_putreg(SAM_SQI_INTEN_OFFSET,
             SQI_INTEN_BDDONEIE | SQI_INTEN_PKTCOMPIE);
  sqi_putreg(SAM_SQI_INTSIGEN_OFFSET,
             SQI_INTSIGEN_BDDONEISE | SQI_INTSIGEN_PKTCOMPISE);

  /* 13. Clear BDCON and any stale interrupt flags */

  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);

  priv->frequency = 50000000u;
  return &priv->spi;
}

/****************************************************************************
 * Public Function: sam_sqi_xip_enable
 *
 * Configure XIP mode for SST26 Regular Read (0x03): single-lane,
 * 3-byte address, no dummy.  After this call, flash data is accessible
 * at SAM_SQI1_XIP_BASE (0x90000000) + offset.
 ****************************************************************************/

void sam_sqi_xip_enable(void)
{
  struct sam_sqi_dev_s *priv = &g_sqi1_dev;

  sqi_putreg(SAM_SQI_XCON1_OFFSET, SQI_XCON1_SST26_READ);
  sqi_putreg(SAM_SQI_XCON2_OFFSET, SQI_XCON2_SST26_CS0);
  sqi_modreg(SAM_SQI_CFG_OFFSET, SQI_CFG_MODE_MASK, SQI_CFG_MODE_XIP);
  priv->xip_configured = true;
}

void sam_sqi_enter_xip(void)
{
  uint32_t cfg = sqi_getreg(SAM_SQI_CFG_OFFSET);
  irqstate_t flags = enter_critical_section();

  /* 1. Stop DMA, clear any pending state */

  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);

  /* 2. Disable SQIEN (SQI goes inactive — no bus activity) */

  cfg = sqi_getreg(SAM_SQI_CFG_OFFSET);
  sqi_putreg(SAM_SQI_CFG_OFFSET, cfg & ~SQI_CFG_SQIEN);

  /* 3. Re-configure XIP registers (SWRST in cmd_write/cmd_read clears these) */

  sqi_putreg(SAM_SQI_XCON1_OFFSET, SQI_XCON1_SST26_READ);
  sqi_putreg(SAM_SQI_XCON2_OFFSET, SQI_XCON2_SST26_CS0);

  /* 4. Restore CLKCON (XIP mode clears CLKDIV per hardware behavior doc 6b)
   *    Timeout prevents infinite hang if peripheral is in bad state. */

  sqi_putreg(SAM_SQI_CLKCON_OFFSET, SQI_CLKCON_EN);
  {
    volatile uint32_t tout = 100000u;
    while (!(sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE))
      {
        if (--tout == 0)
          {
            /* Clock stuck — force SWRST and retry */
            leave_critical_section(flags);
            sqi_full_reset();
            flags = enter_critical_section();
            sqi_putreg(SAM_SQI_XCON1_OFFSET, SQI_XCON1_SST26_READ);
            sqi_putreg(SAM_SQI_XCON2_OFFSET, SQI_XCON2_SST26_CS0);
            sqi_putreg(SAM_SQI_CLKCON_OFFSET, SQI_CLKCON_EN);
            tout = 100000u;
          }
      }
  }
  sqi_putreg(SAM_SQI_CLKCON_OFFSET,
             SQI_CLKCON_EN | SQI_CLKCON_CLKDIV(SQI1_CLKDIV_50MHZ));
  {
    volatile uint32_t tout = 100000u;
    while (!(sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE))
      {
        if (--tout == 0) break;
      }
  }

  __asm__ volatile ("dsb sy" ::: "memory");

  /* 5. Switch MODE to XIP + enable SQIEN */

  sqi_putreg(SAM_SQI_CFG_OFFSET,
             SQI_CFG_MODE_XIP | SQI_CFG_BURSTEN |
             SQI_CFG_DATAEN(0) | SQI_CFG_CSEN0 | SQI_CFG_SQIEN);

  __asm__ volatile ("dsb sy\n isb sy" ::: "memory");

  leave_critical_section(flags);
}

/****************************************************************************
 * Public Function: sam_sqi_flash_cmd_write
 *
 * Send a flash command as a single standalone TX BD with LIFM.
 * All bytes in txbuf are shifted out on IO0, then CS deasserts (LIFM).
 * Used for: WREN (1B), Sector Erase (4B), Page Program (4+256B).
 *
 * Ensures DMA mode, submits BD, polls PKTCOMP, returns to XIP.
 * Returns 0 on success, -EIO on timeout.
 ****************************************************************************/

int sam_sqi_flash_cmd_write(FAR const uint8_t *txbuf, size_t txlen)
{
  uint32_t timeout;
  size_t i;

  if (txlen == 0 || txlen > SQI_DMA_BUF_SIZE)
    {
      return -EINVAL;
    }

  /* SWRST + full reinit — clears BD processor cache (Michigan Ax) */

  sqi_full_reset();

  /* Force CS HIGH between commands (t_CSH >= 50 ns for SST26) */

  {
    uint32_t c = sqi_getreg(SAM_SQI_CFG_OFFSET);
    volatile int _dly;
    sqi_putreg(SAM_SQI_CFG_OFFSET, c & ~SQI_CFG_SQIEN);
    for (_dly = 0; _dly < 50; _dly++) { __asm__ volatile ("nop"); }
    sqi_putreg(SAM_SQI_CFG_OFFSET, c);
  }

  /* Copy TX data */

  for (i = 0; i < txlen; i++)
    {
      g_sqi_tx_buf[i] = txbuf[i];
    }

  /* Set up BD descriptor */

  g_sqi_tx_desc.bd_ctrl    = SQI_BDCTRL_DESC_EN   |
                              SQI_BDCTRL_LAST_BD    |
                              SQI_BDCTRL_LIFM       |
                              SQI_BDCTRL_PKT_INT_EN |
                              SQI_BDCTRL_BUFLEN(txlen);
  g_sqi_tx_desc.bd_stat    = 0;
  g_sqi_tx_desc.bd_bufaddr = (uint32_t)(uintptr_t)g_sqi_tx_buf;
  g_sqi_tx_desc.bd_nxtptr  = NULL;

  /* Flush D-Cache for TX buffer + descriptor (no MPU nocache region yet) */

  {
    uintptr_t addr;
    for (addr = (uintptr_t)g_sqi_tx_buf & ~31u;
         addr < (uintptr_t)g_sqi_tx_buf + txlen;
         addr += 32)
      putreg32(addr, 0xE000EF68u);  /* DCCMVAC */
    for (addr = (uintptr_t)&g_sqi_tx_desc & ~31u;
         addr < (uintptr_t)&g_sqi_tx_desc + 32;
         addr += 32)
      putreg32(addr, 0xE000EF68u);
    __asm__ volatile ("dsb sy" ::: "memory");
  }


  /* Submit BD */

  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
  sqi_putreg(SAM_SQI_BDBASEADD_OFFSET, (uint32_t)(uintptr_t)&g_sqi_tx_desc);
  sqi_putreg(SAM_SQI_BDCON_OFFSET, SQI_BDCON_START | SQI_BDCON_DMAEN);

  timeout = SQI_DMA_TIMEOUT;
  while (!(sqi_getreg(SAM_SQI_INTSTAT_OFFSET) &
           (SQI_INT_BDDONE | SQI_INT_PKTCOMP)))
    {
      if (--timeout == 0)
        {
          sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
          return -EIO;
        }
    }

  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, sqi_getreg(SAM_SQI_INTSTAT_OFFSET));
  return 0;
}

static void sqi_full_reset(void)
{
  volatile uint32_t t;

  putreg8(SQI_CTRLA_SWRST, SAM_SQI1_CTRLA);
  t = 100000u;
  while (getreg8(SAM_SQI1_SYNCBUSY) & SQI_SYNCBUSY_SWRST)
    { if (--t == 0) break; }

  /* Clear XIP registers — they persist through SWRST and can interfere
   * with DMA mode if sam_sqi_enter_xip() was previously called. */

  sqi_putreg(SAM_SQI_XCON1_OFFSET, 0);
  sqi_putreg(SAM_SQI_XCON2_OFFSET, 0);

  /* RXBUFRST + configure DMA mode */

  sqi_putreg(SAM_SQI_CFG_OFFSET,
             SQI_CFG_MODE_DMA | SQI_CFG_BURSTEN |
             SQI_CFG_DATAEN(0) | SQI_CFG_CSEN0 | SQI_CFG_RXBUFRST);
  sqi_putreg(SAM_SQI_CLKCON_OFFSET, SQI_CLKCON_EN);
  {
    volatile uint32_t t = 100000u;
    while (!(sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE))
      { if (--t == 0) break; }
  }
  sqi_putreg(SAM_SQI_CLKCON_OFFSET,
             SQI_CLKCON_EN | SQI_CLKCON_CLKDIV(SQI1_CLKDIV_50MHZ));
  {
    volatile uint32_t t = 100000u;
    while (!(sqi_getreg(SAM_SQI_CLKCON_OFFSET) & SQI_CLKCON_STABLE))
      { if (--t == 0) break; }
  }
  sqi_modreg(SAM_SQI_CFG_OFFSET, 0, SQI_CFG_SQIEN);
  sqi_putreg(SAM_SQI_CMDTHR_OFFSET,
             SQI_CMDTHR_RXCMDTHR(0x01u) | SQI_CMDTHR_TXCMDTHR(0x01u));
  sqi_putreg(SAM_SQI_INTTHR_OFFSET,
             SQI_INTTHR_RXINTTHR(0x01u) | SQI_INTTHR_TXINTTHR(0x01u));
  sqi_putreg(SAM_SQI_THR_OFFSET, SQI_THR_THRES(1u));
  sqi_putreg(SAM_SQI_INTEN_OFFSET,
             SQI_INTEN_BDDONEIE | SQI_INTEN_PKTCOMPIE);
  sqi_putreg(SAM_SQI_INTSIGEN_OFFSET,
             SQI_INTSIGEN_BDDONEISE | SQI_INTSIGEN_PKTCOMPISE);
  sqi_putreg(SAM_SQI_BDCON_OFFSET, 0);
  sqi_putreg(SAM_SQI_INTSTAT_OFFSET, 0xFFFFFFFFu);
}

/****************************************************************************
 * Public Function: sam_sqi_flash_wren_cmd
 *
 * Send WREN (0x06) followed by a command, with ONE SWRST at the start.
 * NO SWRST between WREN and the command — preserves WEL.
 * Uses flush stale data: RXBUFRST + BDBASEADD + START per BD submission.
 * Returns 0 on success, -EIO on timeout.
 ****************************************************************************/

int sam_sqi_flash_wren_cmd(FAR const uint8_t *cmd, size_t cmdlen)
{
  int ret;
  uint8_t wren = 0x06u;

  ret = sam_sqi_flash_cmd_write(&wren, 1);
  if (ret < 0) return ret;

  /* RDSR between WREN and the write command is REQUIRED.
   * The cmd_read (TX→RX linked chain) resets BD processor state so the
   * subsequent standalone TX BD produces bus activity. Without this,
   * SE/PP commands silently fail (confirmed by logic analyzer). */

  sam_sqi_flash_rdsr();

  return sam_sqi_flash_cmd_write(cmd, cmdlen);
}

/****************************************************************************
 * Public Function: sam_sqi_flash_rdsr
 *
 * Read the SST26 Status Register via TX(1:0x05)→RX(1) linked chain.
 * Returns the status byte, or -1 on timeout.
 ****************************************************************************/

int sam_sqi_flash_rdsr(void)
{
  uint8_t cmd = 0x05u;
  uint8_t status;
  int ret;

  ret = sam_sqi_flash_cmd_read(NULL, &cmd, 1, &status, 1);

  return (ret < 0) ? -1 : (int)status;
}

#endif /* CONFIG_PIC32CZCA90_SQI1 */
