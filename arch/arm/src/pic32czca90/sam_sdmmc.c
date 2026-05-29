/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sdmmc.c
 *
 * PIC32CZ CA90 SDMMC1 driver.
 *
 * The SDMMC IP in CA90 (variant sdmmc_44002) is SD Host Controller Spec 2.0
 * compatible. Adapted from SAMA5 sam_sdmmc.c (same IP, identical register
 * offsets) with three CA90-specific changes:
 *
 *  1. Clock enable: GCLK4 (100 MHz main) + GCLK5 (12 MHz slow) + MCLK AHB/APB.
 *
 *  2. Clock divider: reads CA0R.BASECLKF + CA1R.CLKMULT at runtime; uses
 *     programmable mode (CLKGSEL=1) when CLKMULT>0 (Harmony CA90 path),
 *     divided mode (CLKGSEL=0) as fallback — replaces SAMA5 prescaler+divisor.
 *
 *  3. DMA: ADMA2 (descriptor at SAM_SDMMC_ASAR_OFFSET @0x58) when
 *     CONFIG_PIC32CZCA90_SDMMC1_DMA=y. PIO (interrupt-driven) is the
 *     fallback path — toggle via Kconfig for DMA debugging.
 *
 * Combined 32-bit interrupt register access:
 *   NISTR (0x30) and EISTR (0x32) are 16-bit registers at adjacent addresses.
 *   A 32-bit read/write at SAM_SDMMC_NISTR_OFFSET returns bits[15:0]=NISTR
 *   and bits[31:16]=EISTR. The SDMMC_INT_* local defines use this combined
 *   view (error bits shifted left by 16). Same applies to NISTER/EISTER (0x34)
 *   and NISIER/EISIER (0x38).
 *
 * Pin mux: SDMMC1 uses mux I (function code 8).
 *   PC30=CLK, PG03=CMD, PC31=DAT0, PG00=DAT1, PG01=DAT2, PG02=DAT3, PC28=CD
 *
 * IMPORTANT: SDMMC1 shares these pins with SQI1 (mux H = 7). Only one
 * peripheral may own the pins at a time. See init.c for the mux strategy.
 *
 * Reference: Harmony core_apps_pic32cz_ca8x_ca9x sdmmc_fat/plib_sdmmc1.c
 *            platforms/nuttx/NuttX/nuttx/arch/arm/src/sama5/sam_sdmmc.c
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_SDMMC1

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/wdog.h>
#include <nuttx/clock.h>
#include <nuttx/arch.h>
#include <nuttx/sdio.h>
#include <nuttx/wqueue.h>
#include <nuttx/semaphore.h>
#include <nuttx/mmcsd.h>
#include <nuttx/kmalloc.h>
#include <nuttx/irq.h>
#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "sam_port.h"
#include "sam_gclk.h"
#include "sam_sdmmc.h"
#include "hardware/sam_sdmmc.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_gclk.h"
#include "hardware/sam_pinmap.h"

/****************************************************************************
 * Combined 32-bit interrupt status/enable/signal bit positions.
 *
 * 32-bit read at SAM_SDMMC_NISTR_OFFSET (0x30):
 *   bits[15:0]  = NISTR (normal interrupt status)
 *   bits[31:16] = EISTR (error interrupt status)
 *
 * Normal interrupt bits map directly to their NISTR position.
 * Error interrupt bits are shifted left 16 from their EISTR position.
 * Same combined layout applies at NISTER (0x34) and NISIER (0x38).
 ****************************************************************************/

/* Normal interrupt bits (NISTR bit positions = combined bit positions) */
#define SDMMC_INT_CC       SDMMC_NISTR_CMDC      /* bit 0  — Command Complete */
#define SDMMC_INT_TC       SDMMC_NISTR_TRFC      /* bit 1  — Transfer Complete */
#define SDMMC_INT_BGE      SDMMC_NISTR_BLKGE     /* bit 2  — Block Gap Event */
#define SDMMC_INT_DINT     SDMMC_NISTR_DMAINT    /* bit 3  — DMA Interrupt */
#define SDMMC_INT_BWR      SDMMC_NISTR_BWRRDY    /* bit 4  — Buffer Write Ready */
#define SDMMC_INT_BRR      SDMMC_NISTR_BRDRDY    /* bit 5  — Buffer Read Ready */
#define SDMMC_INT_CINS     SDMMC_NISTR_CINS      /* bit 6  — Card Insertion */
#define SDMMC_INT_CRM      SDMMC_NISTR_CREM      /* bit 7  — Card Removal */
#define SDMMC_INT_CINT     SDMMC_NISTR_CINT      /* bit 8  — Card Interrupt */

/* Error interrupt bits (EISTR bit N → combined bit N+16) */
#define SDMMC_INT_CTOE     (SDMMC_EISTR_CMDTEO << 16)   /* bit 16 — Cmd Timeout */
#define SDMMC_INT_CCE      (SDMMC_EISTR_CMDCRC << 16)   /* bit 17 — Cmd CRC */
#define SDMMC_INT_CEBE     (SDMMC_EISTR_CMDEND << 16)   /* bit 18 — Cmd End Bit */
#define SDMMC_INT_CIE      (SDMMC_EISTR_CMDIDX << 16)   /* bit 19 — Cmd Index */
#define SDMMC_INT_DTOE     (SDMMC_EISTR_DATTEO << 16)   /* bit 20 — Data Timeout */
#define SDMMC_INT_DCE      (SDMMC_EISTR_DATCRC << 16)   /* bit 21 — Data CRC */
#define SDMMC_INT_DEBE     (SDMMC_EISTR_DATEND << 16)   /* bit 22 — Data End Bit */
#define SDMMC_INT_CURLIM   (SDMMC_EISTR_CURLIM << 16)   /* bit 23 — Current Limit */
#define SDMMC_INT_AC12E    (SDMMC_EISTR_ACMD   << 16)   /* bit 24 — Auto CMD12 */
#define SDMMC_INT_ADMAE    (SDMMC_EISTR_ADMA   << 16)   /* bit 25 — ADMA Error */

/* Combined mask: all normal + all error bits */
#define SDMMC_INT_ALL  (0x000001ffu | ((uint32_t)SDMMC_EISTR_ALL << 16))

/****************************************************************************
 * HC1R (Host Control 1) bit aliases in 32-bit view at HC1R_OFFSET (0x28).
 * 32-bit RMW at HC1R_OFFSET covers HC1R[7:0]+PCR[15:8]+BGCR+WCR safely.
 ****************************************************************************/

#define SDMMC_HC1_DTW_MASK    (3u << 1)
#define SDMMC_HC1_DTW_1BIT    SDMMC_HC1R_DW_1BIT    /* (0<<1) */
#define SDMMC_HC1_DTW_4BIT    SDMMC_HC1R_DW_4BIT    /* (1<<1) */
#define SDMMC_HC1_HSEN        SDMMC_HC1R_HSEN        /* (1<<2) */
#define SDMMC_HC1_DMAS_MASK   (3u << 3)
#define SDMMC_HC1_DMAS_ADMA   SDMMC_HC1R_DMASEL_ADMA2  /* (2<<3) */

/****************************************************************************
 * CCR (Clock Control) bit aliases (16-bit register).
 * SRR.SWRSTALL at byte offset 0x2F = bit 24 in 32-bit word at CCR_OFFSET.
 ****************************************************************************/

#define SDMMC_CCR_RSTA        (1u << 24)  /* SRR.SWRSTALL via 32-bit view */
#define SDMMC_CCR_RSTC        (1u << 25)  /* SRR.SWRSTCMD */
#define SDMMC_CCR_RSTD        (1u << 26)  /* SRR.SWRSTDAT */

/****************************************************************************
 * SRR (Software Reset) aliases for 8-bit access.
 ****************************************************************************/

#define SDMMC_RESET_ALL     SDMMC_SRR_SWRSTALL   /* 0x01 */
#define SDMMC_RESET_CMD     SDMMC_SRR_SWRSTCMD   /* 0x02 */
#define SDMMC_RESET_DATA    SDMMC_SRR_SWRSTDAT   /* 0x04 */

/****************************************************************************
 * PCR (Power Control) combined bit values.
 ****************************************************************************/

#define SDMMC_POWER_ON      SDMMC_PCR_SDBPWR_ON
#define SDMMC_POWER_330     (SDMMC_PCR_SDBVSEL_3V3 | SDMMC_PCR_SDBPWR_ON)
#define SDMMC_POWER_300     (SDMMC_PCR_SDBVSEL_3V0 | SDMMC_PCR_SDBPWR_ON)
#define SDMMC_POWER_180     (SDMMC_PCR_SDBVSEL_1V8 | SDMMC_PCR_SDBPWR_ON)

/****************************************************************************
 * CA0R (Capabilities 0) voltage bits.
 ****************************************************************************/

#define SDMMC_CA0_VS33      (1u << 24)
#define SDMMC_CA0_VS30      (1u << 25)
#define SDMMC_CA0_VS18      (1u << 26)
#define SDMMC_CA0_DDR50     (1u << 2)
#define SDMMC_CA0_SDR50     (1u << 0)
#define SDMMC_CA0_SDR104    (1u << 1)

/****************************************************************************
 * HC2R (Host Control 2) UHS mode bits.
 ****************************************************************************/

#define SDMMC_HC2_UHS_MASK    (7u << 0)
#define SDMMC_HC2_UHS_SDR12   (0u << 0)
#define SDMMC_HC2_UHS_SDR25   (1u << 0)
#define SDMMC_HC2_UHS_SDR50   (2u << 0)
#define SDMMC_HC2_UHS_SDR104  (3u << 0)
#define SDMMC_HC2_UHS_DDR50   (4u << 0)

/****************************************************************************
 * TMR+CR combined 32-bit write at SAM_SDMMC_TMR_OFFSET (0x0C).
 * TMR bits at [15:0]; CR bits at [31:16] (CR is at 0x0E = TMR_OFFSET + 2).
 ****************************************************************************/

#define SDMMC_CMD_DMAEN          SDMMC_TMR_DMAEN          /* TMR bit 0 */
#define SDMMC_CMD_BCEN           SDMMC_TMR_BCEN           /* TMR bit 1 */
#define SDMMC_CMD_AC12EN         SDMMC_TMR_ACMDEN_CMD12   /* TMR bit 2 */
#define SDMMC_CMD_DTDSEL         SDMMC_TMR_DTDSEL         /* TMR bit 4 */
#define SDMMC_CMD_MSBSEL         SDMMC_TMR_MSBSEL         /* TMR bit 5 */
/* CR bits shifted to their 32-bit combined positions (CR at +2 → +16 bits) */
#define SDMMC_CMD_RSPTYP_NONE    (0u << 16)
#define SDMMC_CMD_RSPTYP_136     (1u << 16)
#define SDMMC_CMD_RSPTYP_48      (2u << 16)
#define SDMMC_CMD_RSPTYP_48BUSY  (3u << 16)
#define SDMMC_CMD_CCCEN          (SDMMC_CR_CMDCCEN << 16) /* CR bit 3 → combined bit 19 */
#define SDMMC_CMD_CICEN          (SDMMC_CR_CMDICEN << 16) /* CR bit 4 → combined bit 20 */
#define SDMMC_CMD_DPSEL          (SDMMC_CR_DPSEL   << 16) /* CR bit 5 → combined bit 21 */
#define SDMMC_CMD_ABORT          (SDMMC_CR_CMDTYP_ABORT << 16) /* CR [7:6] → [23:22] */
#define SDMMC_CMD_IDXSHIFT       (SDMMC_CR_CMDIDX_SHIFT + 16)  /* CR [13:8] → [29:24] */

/****************************************************************************
 * Bus speed constants.
 ****************************************************************************/

#define SDMMC1_BUS_HIGH_SPEED_THRESHOLD   26000000u

/****************************************************************************
 * Timeout and TCR constants.
 ****************************************************************************/

#define SDMMC_CMDTIMEOUT        MSEC2TICK(200)
#define SDMMC_LONGTIMEOUT       MSEC2TICK(2000)
#define SDMMC_DTOCV_MAXTIMEOUT  SDMMC_TCR_DTCVAL_MAX  /* 0x0E */

/****************************************************************************
 * Interrupt mask groupings.
 ****************************************************************************/

#define SDMMC_RESPERR_INTS   (SDMMC_INT_CCE | SDMMC_INT_CTOE | \
                               SDMMC_INT_CEBE | SDMMC_INT_CIE)
#define SDMMC_RESPDONE_INTS  (SDMMC_RESPERR_INTS | SDMMC_INT_CC)

#define SDMMC_XFRERR_INTS    (SDMMC_INT_DCE | SDMMC_INT_DTOE | SDMMC_INT_DEBE)
#define SDMMC_RCVDONE_INTS   (SDMMC_XFRERR_INTS | SDMMC_INT_BRR | SDMMC_INT_TC)
#define SDMMC_SNDDONE_INTS   (SDMMC_XFRERR_INTS | SDMMC_INT_BWR | SDMMC_INT_TC)
#define SDMMC_XFRDONE_INTS   (SDMMC_XFRERR_INTS | SDMMC_INT_BRR | \
                               SDMMC_INT_BWR | SDMMC_INT_TC)
#define SDMMC_DMAERR_INTS    (SDMMC_XFRERR_INTS | SDMMC_INT_ADMAE)
#define SDMMC_DMADONE_INTS   (SDMMC_DMAERR_INTS | SDMMC_INT_TC)
#define SDMMC_WAITALL_INTS   (SDMMC_RESPDONE_INTS | SDMMC_XFRDONE_INTS | \
                               SDMMC_DMADONE_INTS)

#define SDMMC_INT_CMD_MASK   (SDMMC_INT_CC | SDMMC_INT_CTOE | \
                               SDMMC_INT_CCE | SDMMC_INT_CEBE | SDMMC_INT_CIE)
#define SDMMC_INT_DATA_MASK  (SDMMC_INT_TC | SDMMC_INT_DINT | \
                               SDMMC_INT_BRR | SDMMC_INT_BWR | \
                               SDMMC_INT_CTOE | SDMMC_INT_CCE | \
                               SDMMC_INT_DEBE | SDMMC_INT_ADMAE)

/* bus_mode enum (SD Host Controller Spec 2.0) */
enum bus_mode
{
  MMC_LEGACY,
  MMC_HS,
  SD_HS,
  MMC_HS_52,
  MMC_DDR_52,
  UHS_SDR12,
  UHS_SDR25,
  UHS_SDR50,
  UHS_DDR50,
  UHS_SDR104,
  MMC_HS_200,
  MMC_HS_400,
  MMC_HS_400_ES,
  MMC_MODES_END
};

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_dev_s
{
  struct sdio_dev_s dev;

  uint32_t base;
  sem_t waitsem;
  sdio_eventset_t waitevents;
  uint32_t waitints;
  volatile sdio_eventset_t wkupevent;
  struct wdog_s waitwdog;

  sdio_statset_t cdstatus;
  sdio_eventset_t cbevents;
  worker_t callback;
  void *cbarg;
  struct work_s cbwork;

  uint32_t *buffer;
  size_t remaining;
  uint32_t xfrints;

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  volatile uint8_t xfrflags;
  uint32_t *bufferend;
#endif

  uint32_t cintints;
  int (*do_sdio_card)(void *);
  void *do_sdio_arg;

  uint32_t addr;
  uint32_t sw_cd_gpio;
  uint32_t cd_invert;

  bool cmd_error;  /* true if sam_waitresponse detected a command error */
};


/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void sam_takesem(struct sam_dev_s *priv);
#define     sam_givesem(priv) (nxsem_post(&priv->waitsem))
static void sam_configwaitints(struct sam_dev_s *priv, uint32_t waitints,
              sdio_eventset_t waitevents, sdio_eventset_t wkupevents);
static void sam_configxfrints(struct sam_dev_s *priv, uint32_t xfrints);

static void sam_dataconfig(struct sam_dev_s *priv, bool bwrite,
              unsigned int datalen, unsigned int timeout);

#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
static void sam_transmit(struct sam_dev_s *priv);
static void sam_receive(struct sam_dev_s *priv);
#endif

static void sam_eventtimeout(wdparm_t arg);
static void sam_endwait(struct sam_dev_s *priv, sdio_eventset_t wkupevent);
static void sam_endtransfer(struct sam_dev_s *priv,
              sdio_eventset_t wkupevent);

static int  sam_interrupt(int irq, void *context, void *arg);

static void sam_reset(struct sdio_dev_s *dev);
static sdio_capset_t sam_capabilities(struct sdio_dev_s *dev);
static sdio_statset_t sam_status(struct sdio_dev_s *dev);
static void sam_widebus(struct sdio_dev_s *dev, bool enable);
static void sam_clock(struct sdio_dev_s *dev, enum sdio_clock_e rate);
static void sam_power(struct sam_dev_s *priv);
static int  sam_attach(struct sdio_dev_s *dev);
static int  sam_sendcmd(struct sdio_dev_s *dev, uint32_t cmd, uint32_t arg);
#ifdef CONFIG_SDIO_BLOCKSETUP
static void sam_blocksetup(struct sdio_dev_s *dev,
              unsigned int blocklen, unsigned int nblocks);
#endif
#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
static int  sam_recvsetup(struct sdio_dev_s *dev, uint8_t *buffer,
              size_t nbytes);
static int  sam_sendsetup(struct sdio_dev_s *dev,
              const uint8_t *buffer, size_t nbytes);
#endif
static int  sam_cancel(struct sdio_dev_s *dev);
static int  sam_waitresponse(struct sdio_dev_s *dev, uint32_t cmd);
static int  sam_recvshortcrc(struct sdio_dev_s *dev, uint32_t cmd,
              uint32_t *rshort);
static int  sam_recvlong(struct sdio_dev_s *dev, uint32_t cmd,
              uint32_t rlong[4]);
static int  sam_recvshort(struct sdio_dev_s *dev, uint32_t cmd,
              uint32_t *rshort);
static void sam_waitenable(struct sdio_dev_s *dev,
              sdio_eventset_t eventset, uint32_t timeout);
static sdio_eventset_t sam_eventwait(struct sdio_dev_s *dev);
static void sam_callbackenable(struct sdio_dev_s *dev,
              sdio_eventset_t eventset);
static int  sam_registercallback(struct sdio_dev_s *dev,
              worker_t callback, void *arg);
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
static int  sam_dmarecvsetup(struct sdio_dev_s *dev,
              uint8_t *buffer, size_t buflen);
static int  sam_dmasendsetup(struct sdio_dev_s *dev,
              const uint8_t *buffer, size_t buflen);
#endif
static void sam_callback(void *arg);
static void sam_set_uhs_timing(struct sam_dev_s *priv,
              enum bus_mode selected_mode);
static int  sam_set_clock(struct sam_dev_s *priv, uint32_t clock);
static int  sam_set_interrupts(struct sam_dev_s *priv);

void sdio_mediachange(struct sdio_dev_s *dev, bool cardinslot);

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct sam_dev_s g_sdmmcdev =
{
  .addr = SAM_SDMMC1_BASE,
  .dev  =
  {
    .reset            = sam_reset,
    .capabilities     = sam_capabilities,
    .status           = sam_status,
    .widebus          = sam_widebus,
    .clock            = sam_clock,
    .attach           = sam_attach,
    .sendcmd          = sam_sendcmd,
#ifdef CONFIG_SDIO_BLOCKSETUP
    .blocksetup       = sam_blocksetup,
#endif
#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
    .recvsetup        = sam_recvsetup,
    .sendsetup        = sam_sendsetup,
#else
    .recvsetup        = sam_dmarecvsetup,
    .sendsetup        = sam_dmasendsetup,
#endif
    .cancel           = sam_cancel,
    .waitresponse     = sam_waitresponse,
    .recv_r1          = sam_recvshortcrc,
    .recv_r2          = sam_recvlong,
    .recv_r3          = sam_recvshort,
    .recv_r4          = sam_recvshort,
    .recv_r5          = sam_recvshortcrc,
    .recv_r6          = sam_recvshortcrc,
    .recv_r7          = sam_recvshort,
    .waitenable       = sam_waitenable,
    .eventwait        = sam_eventwait,
    .callbackenable   = sam_callbackenable,
    .registercallback = sam_registercallback,
#ifdef CONFIG_SDIO_DMA
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
    .dmarecvsetup     = sam_dmarecvsetup,
    .dmasendsetup     = sam_dmasendsetup,
#else
    .dmarecvsetup     = sam_recvsetup,
    .dmasendsetup     = sam_sendsetup,
#endif
#endif
  },
};

/* ADMA2 descriptor in nocache region (one descriptor handles ≤65535 bytes) */

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
static sdmmc_adma_desc_t g_adma_desc
  __attribute__((section(".nocache"), aligned(4)));
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint32_t sam_getreg8(struct sam_dev_s *priv,
                                    unsigned int offset)
{
  return getreg8(priv->base + offset);
}

static inline uint32_t sam_getreg16(struct sam_dev_s *priv,
                                     unsigned int offset)
{
  return getreg16(priv->base + offset);
}

static inline uint32_t sam_getreg32(struct sam_dev_s *priv,
                                     unsigned int offset)
{
  return getreg32(priv->base + offset);
}

static inline uint32_t sam_getreg(struct sam_dev_s *priv,
                                   unsigned int offset)
{
  return sam_getreg32(priv, offset);
}

static inline void sam_putreg8(struct sam_dev_s *priv, uint32_t value,
                                unsigned int offset)
{
  putreg8((uint8_t)value, priv->base + offset);
}

static inline void sam_putreg16(struct sam_dev_s *priv, uint32_t value,
                                 unsigned int offset)
{
  putreg16((uint16_t)value, priv->base + offset);
}

static inline void sam_putreg32(struct sam_dev_s *priv, uint32_t value,
                                 unsigned int offset)
{
  putreg32(value, priv->base + offset);
}

static inline void sam_putreg(struct sam_dev_s *priv, uint32_t value,
                               unsigned int offset)
{
  sam_putreg32(priv, value, offset);
}

static void sam_takesem(struct sam_dev_s *priv)
{
  nxsem_wait_uninterruptible(&priv->waitsem);
}

static void sam_configwaitints(struct sam_dev_s *priv, uint32_t waitints,
                                sdio_eventset_t waitevents,
                                sdio_eventset_t wkupevent)
{
  irqstate_t flags;

  flags            = enter_critical_section();
  priv->waitevents = waitevents;
  priv->wkupevent  = wkupevent;
  priv->waitints   = waitints;

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  priv->xfrflags   = 0;
#endif

  {
    uint32_t  v       = priv->xfrints | priv->waitints | priv->cintints |
                        SDMMC_INT_DINT;
    uint16_t  nisier  = (uint16_t)(v & 0xffffu);
    uint16_t  eisier  = (uint16_t)(v >> 16);

    /* NISIER bit 15 (ERRINT) gates all EISIER-signalled errors to the NVIC.
     * Without it, ADMA/data errors set EISTR but never fire an interrupt. */

    if (eisier != 0)
      {
        nisier |= SDMMC_NISTR_ERRINT;
      }

    sam_putreg16(priv, nisier, SAM_SDMMC_NISIER_OFFSET);
    sam_putreg16(priv, eisier, SAM_SDMMC_EISIER_OFFSET);
  }
  leave_critical_section(flags);
}

static void sam_configxfrints(struct sam_dev_s *priv, uint32_t xfrints)
{
  irqstate_t flags;

  flags = enter_critical_section();
  priv->xfrints = xfrints;
  {
    uint32_t  v       = priv->xfrints | priv->waitints | priv->cintints;
    uint16_t  nisier  = (uint16_t)(v & 0xffffu);
    uint16_t  eisier  = (uint16_t)(v >> 16);

    if (eisier != 0)
      {
        nisier |= SDMMC_NISTR_ERRINT;
      }

    sam_putreg16(priv, nisier, SAM_SDMMC_NISIER_OFFSET);
    sam_putreg16(priv, eisier, SAM_SDMMC_EISIER_OFFSET);
  }
  leave_critical_section(flags);
}

static inline void sam_dataconfig(struct sam_dev_s *priv, bool bwrite,
                                   unsigned int datalen, unsigned int timeout)
{
  sam_putreg8(priv, timeout & 0x0fu, SAM_SDMMC_TCR_OFFSET);
}

/****************************************************************************
 * PIO (non-DMA) data transfer helpers
 ****************************************************************************/

#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
static void sam_transmit(struct sam_dev_s *priv)
{
  union { uint32_t w; uint8_t b[4]; } data;

  while (priv->remaining > 0 &&
         (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
          SDMMC_PSR_BUFWREN) != 0)
    {
      if (priv->remaining >= sizeof(uint32_t))
        {
          data.w           = *priv->buffer++;
          priv->remaining -= sizeof(uint32_t);
        }
      else
        {
          uint8_t *ptr = (uint8_t *)priv->buffer;
          int i;
          data.w = 0;
          for (i = 0; i < (int)priv->remaining; i++)
            {
              data.b[i] = *ptr++;
            }
          priv->remaining = 0;
        }

      sam_putreg(priv, data.w, SAM_SDMMC_BDPR_OFFSET);
    }

  sam_putreg(priv, SDMMC_INT_BWR, SAM_SDMMC_NISTR_OFFSET);
}

static void sam_receive(struct sam_dev_s *priv)
{
  union { uint32_t w; uint8_t b[4]; } data;

  sam_putreg(priv, SDMMC_INT_BRR, SAM_SDMMC_NISTR_OFFSET);

  while (priv->remaining > 0 &&
         (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
          SDMMC_PSR_BUFRDEN) != 0)
    {
      data.w = sam_getreg(priv, SAM_SDMMC_BDPR_OFFSET);

      if (priv->remaining >= sizeof(uint32_t))
        {
          *priv->buffer++  = data.w;
          priv->remaining -= sizeof(uint32_t);
        }
      else
        {
          uint8_t *ptr = (uint8_t *)priv->buffer;
          int i;
          for (i = 0; i < (int)priv->remaining; i++)
            {
              *ptr++ = data.b[i];
            }
          priv->remaining = 0;
        }
    }

}
#endif /* !CONFIG_PIC32CZCA90_SDMMC1_DMA */

/****************************************************************************
 * Event helpers
 ****************************************************************************/

static void sam_eventtimeout(wdparm_t arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)arg;

  DEBUGASSERT(priv != NULL);
  DEBUGASSERT((priv->waitevents & SDIOWAIT_TIMEOUT) != 0 ||
              priv->wkupevent != 0);

  mcerr("WDOG: waitevents=%02x wkupevent=%02x"
        " NISTR=%08" PRIx32 " NISIER=%08" PRIx32
        " PSR=%08" PRIx32 " AESR=%02" PRIx32 "\n",
        priv->waitevents, priv->wkupevent,
        sam_getreg32(priv, SAM_SDMMC_NISTR_OFFSET),
        sam_getreg(priv, SAM_SDMMC_NISIER_OFFSET),
        sam_getreg(priv, SAM_SDMMC_PSR_OFFSET),
        (uint32_t)sam_getreg8(priv, SAM_SDMMC_AESR_OFFSET));

  if ((priv->waitevents & SDIOWAIT_TIMEOUT) != 0)
    {
      sam_endwait(priv, SDIOWAIT_TIMEOUT);
      mcerr("ERROR: Timeout\n");
    }
}

static void sam_endwait(struct sam_dev_s *priv, sdio_eventset_t wkupevent)
{
  wd_cancel(&priv->waitwdog);
  sam_configwaitints(priv, 0, 0, wkupevent);
  sam_givesem(priv);
}

static void sam_endtransfer(struct sam_dev_s *priv,
                              sdio_eventset_t wkupevent)
{
  sam_configxfrints(priv, 0);
  sam_putreg(priv, SDMMC_XFRDONE_INTS | SDMMC_DMADONE_INTS,
             SAM_SDMMC_NISTR_OFFSET);

  priv->remaining = 0;

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  up_invalidate_dcache((uintptr_t)priv->buffer,
                       (uintptr_t)priv->bufferend);
#endif

  if ((priv->waitevents & wkupevent) != 0)
    {
      sam_endwait(priv, wkupevent);
    }
}

/****************************************************************************
 * Interrupt handler
 ****************************************************************************/

static int sam_interrupt(int irq, void *context, void *arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)arg;
  uint32_t enabled;
  uint32_t pending;
  uint32_t nisier;
  uint32_t nistr;

  /* 32-bit read at NISIER_OFFSET covers NISIER[15:0] + EISIER[31:16] */

  nisier  = sam_getreg(priv, SAM_SDMMC_NISIER_OFFSET);

  /* 32-bit read at NISTR_OFFSET covers NISTR[15:0] + EISTR[31:16] */

  nistr   = sam_getreg32(priv, SAM_SDMMC_NISTR_OFFSET);
  enabled = nistr & nisier;
  pending = enabled & priv->xfrints;

  mcinfo("ISR: NISTR=%04" PRIx32 " EISTR=%04" PRIx32
         " NISIER=%04" PRIx32 " pending=%08" PRIx32 "\n",
         nistr & 0xffffu, nistr >> 16, nisier & 0xffffu, pending);

  if (pending != 0)
    {
#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
      if ((pending & SDMMC_INT_BRR) != 0)
        {
          sam_receive(priv);
        }
      else if ((pending & SDMMC_INT_BWR) != 0)
        {
          sam_transmit(priv);
        }
#else
      /* ADMA2: DINT means a descriptor with INT bit was processed.
       * The engine advances automatically — just clear the flag. */

      if (((pending & SDMMC_INT_DINT) != 0) &&
          ((pending & SDMMC_INT_TC)   == 0))
        {
          sam_putreg(priv, SDMMC_INT_DINT, SAM_SDMMC_NISTR_OFFSET);
        }
#endif

      if ((pending & SDMMC_INT_TC) != 0)
        {
          sam_endtransfer(priv, SDIOWAIT_TRANSFERDONE);
        }
      else if ((pending & (SDMMC_INT_DCE | SDMMC_INT_DTOE |
                           SDMMC_INT_ADMAE)) != 0)
        {
          if ((pending & SDMMC_INT_ADMAE) != 0)
            {
              mcerr("SDMMC: ADMA error (AESR=0x%02" PRIx32 ")\n",
                    sam_getreg8(priv, SAM_SDMMC_AESR_OFFSET));
            }
          else if ((pending & SDMMC_INT_DCE) != 0)
            {
              mcerr("SDMMC: data CRC error\n");
            }
          else
            {
              mcerr("SDMMC: data timeout\n");
            }

          /* SW reset DAT lane — clears DAT inhibit so next transfer can start.
           * Harmony ErrorReset(SDMMC_RESET_DAT) pattern. */

          sam_putreg8(priv, SDMMC_RESET_DATA, SAM_SDMMC_SRR_OFFSET);
          while (sam_getreg8(priv, SAM_SDMMC_SRR_OFFSET) & SDMMC_RESET_DATA)
            ;

          sam_endtransfer(priv, SDIOWAIT_TRANSFERDONE | SDIOWAIT_ERROR);
        }
    }

  /* Command-phase events */

  pending = enabled & priv->waitints;

  if (pending != 0)
    {
      if ((pending & SDMMC_RESPERR_INTS) != 0)
        {
          sam_endwait(priv, SDIOWAIT_CMDDONE | SDIOWAIT_ERROR);
        }
      else if ((pending & SDMMC_INT_CC) != 0)
        {
          sam_putreg(priv, SDMMC_INT_CC, SAM_SDMMC_NISTR_OFFSET);
          sam_endwait(priv, SDIOWAIT_CMDDONE);
        }
    }

  /* Card insertion / removal events */

  if ((enabled & (SDMMC_INT_CINS | SDMMC_INT_CRM)) != 0)
    {
      bool inserted = (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
                       SDMMC_PSR_CARDINS) != 0;
      sam_putreg(priv, SDMMC_INT_CINS | SDMMC_INT_CRM,
                 SAM_SDMMC_NISTR_OFFSET);
      sdio_mediachange(&priv->dev, inserted ^ (bool)priv->cd_invert);
    }

  /* SDIO card interrupt */

  if ((enabled & SDMMC_INT_CINT) != 0)
    {
      sam_putreg(priv, SDMMC_INT_CINT, SAM_SDMMC_NISTR_OFFSET);
      if (priv->do_sdio_card)
        {
          priv->do_sdio_card(priv->do_sdio_arg);
        }
    }

  /* Clear all status bits captured at entry (Harmony pattern: write back at
   * ISR end so no set bit survives to re-trigger the level-sensitive NVIC). */

  sam_putreg(priv, nistr, SAM_SDMMC_NISTR_OFFSET);

  return OK;
}

/****************************************************************************
 * SDIO interface methods
 ****************************************************************************/

static void sam_reset(struct sdio_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  unsigned long timeout_ms;
  uint8_t hc1;

  sdmmc1_clk_enable();

  /* Disable all interrupt signals before reset */

  sam_putreg16(priv, 0, SAM_SDMMC_NISIER_OFFSET);
  sam_putreg16(priv, 0, SAM_SDMMC_EISIER_OFFSET);

  /* Software reset all (Harmony: SRR |= SWRSTALL, wait) */

  sam_putreg8(priv, SDMMC_RESET_ALL, SAM_SDMMC_SRR_OFFSET);
  timeout_ms = 1000;
  while (sam_getreg8(priv, SAM_SDMMC_SRR_OFFSET) & SDMMC_RESET_ALL)
    {
      if (timeout_ms == 0)
        {
          mcerr("Reset never completed\n");
          return;
        }

      timeout_ms--;
      usleep(100);
    }

  /* Clear all status registers (W1C) — Harmony: EISTR=Msk, NISTR=Msk */

  sam_putreg16(priv, SDMMC_EISTR_ALL, SAM_SDMMC_EISTR_OFFSET);
  sam_putreg16(priv, 0x01FFu,         SAM_SDMMC_NISTR_OFFSET);

  /* Enable all normal and error status bits — Harmony: NISTER=Msk, EISTER=Msk.
   * Combined 32-bit write: [15:0]=NISTER, [31:16]=EISTER. */

  sam_putreg(priv, SDMMC_INT_ALL, SAM_SDMMC_NISTER_OFFSET);

  /* Maximum data timeout — Harmony: TCR = DTCVAL(0xE) */

  sam_putreg8(priv, SDMMC_TCR_DTCVAL_MAX, SAM_SDMMC_TCR_OFFSET);

  /* DMA select: ADMA2 in DMA mode, clear (SDMA=0) in PIO mode.
   * HC1R.DMASEL is spec'd to matter only when TMR.DMAEN=1, but the
   * sdmmc_44002 IP on CA90 suppresses BRR/BDPR when DMASEL=ADMA2 even
   * with DMAEN=0 — PIO receive never sees data. Clear in PIO mode. */

  hc1  = (uint8_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET);
  hc1 &= (uint8_t)~SDMMC_HC1_DMAS_MASK;
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  hc1 |= (uint8_t)SDMMC_HC1_DMAS_ADMA;
#endif
  sam_putreg8(priv, hc1, SAM_SDMMC_HC1R_OFFSET);

  /* SD Bus Voltage = 3.3V, Power On — Harmony: PCR = SDBVSEL_3V3 | SDBPWR_ON.
   * SWRSTALL resets PCR to 0 (power off).  Without this write the card has
   * no VDD and cannot respond to any command (CMDTOE+CMDCRC on every xfer).
   * Must be set here so every SWRSTALL — including the one triggered by the
   * NuttX mmcsd layer via sdio_reset() — restores card power. */

  sam_putreg8(priv, (uint8_t)SDMMC_POWER_330, SAM_SDMMC_PCR_OFFSET);

  /* Enable 400 kHz clock — Harmony ModuleInit step 7: ClockSet(400kHz).
   *
   * SWRSTALL clears CCR to 0 (INTCLKEN=0).  Without INTCLKEN=1 the
   * SDMMC hardware debounce timer has no clock source: PSR.CARDSS stays
   * 0 and the controller refuses to execute any command (fires immediate
   * CMDTOE+CMDCRC even for CMD0 which expects no response).
   *
   * Harmony sets the 400 kHz clock BEFORE clearing HC1R and BEFORE
   * enabling NISIER.  We follow the same order. */

  sam_set_clock(priv, SDMMC_CLOCK_FREQ_400_KHZ);

  /* Clear high-speed and bus-width bits — Harmony: HC1R &= ~(HSEN | DW) */

  hc1  = (uint8_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET);
  hc1 &= (uint8_t)~((uint8_t)SDMMC_HC1_HSEN | (uint8_t)SDMMC_HC1_DTW_MASK);
  sam_putreg8(priv, hc1, SAM_SDMMC_HC1R_OFFSET);

  /* Clear driver state */

  priv->waitevents = 0;
  priv->waitints   = 0;
  priv->wkupevent  = 0;
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  priv->xfrflags   = 0;
#endif

  wd_cancel(&priv->waitwdog);

  priv->buffer    = 0;
  priv->remaining = 0;
  priv->xfrints   = 0;
}

static sdio_capset_t sam_capabilities(struct sdio_dev_s *dev)
{
  sdio_capset_t caps = 0;

#ifdef CONFIG_PIC32CZCA90_SDMMC1_WIDTH_D1_D4
  caps |= SDIO_CAPS_4BIT;
#endif

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  caps |= SDIO_CAPS_DMASUPPORTED;
#endif
  caps |= SDIO_CAPS_DMABEFOREWRITE;

  return caps;
}

static sdio_statset_t sam_status(struct sdio_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  bool present;

  if (priv->sw_cd_gpio != 0)
    {
      present = (bool)priv->cd_invert ^
                !(sam_portread(priv->sw_cd_gpio));
    }
  else
    {
      present = ((sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
                  SDMMC_PSR_CARDINS) != 0) ^ (bool)priv->cd_invert;
    }

  if (present)
    {
      priv->cdstatus |= SDIO_STATUS_PRESENT;
    }
  else
    {
      priv->cdstatus &= ~SDIO_STATUS_PRESENT;
    }

  return priv->cdstatus;
}

static void sam_widebus(struct sdio_dev_s *dev, bool wide)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  uint32_t regval;

  regval  = sam_getreg(priv, SAM_SDMMC_HC1R_OFFSET);
  regval &= ~SDMMC_HC1_DTW_MASK;
  regval |= wide ? SDMMC_HC1_DTW_4BIT : SDMMC_HC1_DTW_1BIT;
  sam_putreg(priv, regval, SAM_SDMMC_HC1R_OFFSET);
}

static void sam_clock(struct sdio_dev_s *dev, enum sdio_clock_e rate)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  switch (rate)
    {
    default:
    case CLOCK_SDIO_DISABLED:
      sam_set_clock(priv, 0);
      break;

    case CLOCK_IDMODE:
      sam_set_clock(priv, SDMMC_CLOCK_FREQ_400_KHZ);

      /* Poll PSR.CARDSS: debounce complete (up to 500 ms). */

      {
        int cardss_ms = 0;
        while (cardss_ms < 500 &&
               (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
                SDMMC_PSR_CARDSS) == 0)
          {
            usleep(10000);
            cardss_ms += 10;
          }

        if ((sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
             SDMMC_PSR_CARDSS) == 0)
          {
            mcerr("SDMMC: CARDSS timeout PSR=%08" PRIx32 "\n",
                  sam_getreg(priv, SAM_SDMMC_PSR_OFFSET));
          }
      }
      break;

    case CLOCK_MMC_TRANSFER:
    case CLOCK_SD_TRANSFER_1BIT:
    case CLOCK_SD_TRANSFER_4BIT:
      sam_set_clock(priv, SDMMC_CLOCK_FREQ_25_MHZ);
      break;
    }
}

static int sam_attach(struct sdio_dev_s *dev)
{
  int ret;
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  ret = irq_attach(SAM_IRQ_SDMMC1, sam_interrupt, priv);

  if (ret == OK)
    {
      sam_putreg16(priv, 0, SAM_SDMMC_NISIER_OFFSET);
      sam_putreg16(priv, 0, SAM_SDMMC_EISIER_OFFSET);
      sam_putreg(priv, SDMMC_INT_ALL, SAM_SDMMC_NISTR_OFFSET);
      up_enable_irq(SAM_IRQ_SDMMC1);
    }

  return ret;
}

static int sam_sendcmd(struct sdio_dev_s *dev, uint32_t cmd, uint32_t arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  clock_t timeout;
  clock_t start;
  clock_t elapsed;
  uint32_t regval;
  uint32_t cmdidx;

  priv->cmd_error = false;

  cmdidx = (cmd & MMCSD_CMDIDX_MASK) >> MMCSD_CMDIDX_SHIFT;
  regval = cmdidx << SDMMC_CMD_IDXSHIFT;

  if (cmd & MMCSD_WRXFR)
    {
      regval |= SDMMC_CMD_DPSEL;
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
      regval |= SDMMC_CMD_DMAEN;
#endif
      if (cmd & MMCSD_MULTIBLOCK)
        {
          regval |= SDMMC_CMD_MSBSEL | SDMMC_CMD_BCEN;
          if (cmd & MMCSD_STOPXFR)
            {
              regval |= SDMMC_CMD_AC12EN;
            }
        }
    }
  else if (cmd & MMCSD_DATAXFR)
    {
      regval |= SDMMC_CMD_DPSEL | SDMMC_CMD_DTDSEL;
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
      regval |= SDMMC_CMD_DMAEN;
#endif
      if (cmd & MMCSD_MULTIBLOCK)
        {
          regval |= SDMMC_CMD_MSBSEL | SDMMC_CMD_BCEN;
          if (cmd & MMCSD_STOPXFR)
            {
              regval |= SDMMC_CMD_AC12EN;
            }
        }
    }

  switch (cmd & MMCSD_RESPONSE_MASK)
    {
    case MMCSD_NO_RESPONSE:
      break;

    case MMCSD_R1_RESPONSE:
      regval |= (SDMMC_CMD_RSPTYP_48 | SDMMC_CMD_CCCEN | SDMMC_CMD_CICEN);
      break;

    case MMCSD_R1B_RESPONSE:
      regval |= (SDMMC_CMD_RSPTYP_48BUSY | SDMMC_CMD_CCCEN |
                 SDMMC_CMD_CICEN);
      break;

    case MMCSD_R2_RESPONSE:
      regval |= (SDMMC_CMD_RSPTYP_136 | SDMMC_CMD_CCCEN);
      break;

    case MMCSD_R3_RESPONSE:
    case MMCSD_R4_RESPONSE:
      regval |= SDMMC_CMD_RSPTYP_48;
      break;

    case MMCSD_R5_RESPONSE:
      regval |= (SDMMC_CMD_RSPTYP_48 | SDMMC_CMD_CCCEN | SDMMC_CMD_CICEN);
      break;

    case MMCSD_R6_RESPONSE:
    case MMCSD_R7_RESPONSE:
      regval |= (SDMMC_CMD_RSPTYP_48 | SDMMC_CMD_CCCEN | SDMMC_CMD_CICEN);
      break;

    default:
      break;
    }

  /* Wait for CMD inhibit (CMDINHC) and DATA inhibit (CMDINHD) to clear.
   * CMDINHD must be clear before any command that uses the DAT line
   * (writes, R1b responses).  Without this, a CMD23/CMD24 sent while the
   * card is still programming the previous block causes CMDTEO. */

  timeout = SDMMC_LONGTIMEOUT;
  start   = clock_systime_ticks();

  while ((sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
          (SDMMC_PSR_CMDINHC | SDMMC_PSR_CMDINHD)) != 0)
    {
      elapsed = clock_systime_ticks() - start;
      if (elapsed >= timeout)
        {
          mcerr("Timeout waiting for CMD/DAT inhibit clear\n");
          return -EBUSY;
        }
    }


  /* Harmony pattern: clear pending status, write NISIER/EISIER, then
   * trigger the command — all in one critical section.  This closes the
   * race where a stale TC/DINT bit in NISTR fires a spurious ISR between
   * configxfrints returning and the command actually starting.
   *
   * EISIER = nisier_v >> 16 (selective, not 0xFFFF): only enable error signals
   * for bits that are currently expected.  NISIER.ERRINT (bit 15) is set
   * whenever EISIER != 0 — without it, EISIER-signalled errors never reach
   * the NVIC even if EISTR bits are set. */
  {
    irqstate_t isrflags;
    uint32_t   nisier_v;
    uint16_t   nisier_reg;
    uint16_t   eisier_reg;

    nisier_v  = priv->xfrints | priv->waitints | priv->cintints |
                SDMMC_INT_DINT;
    nisier_reg = (uint16_t)(nisier_v & 0xffffu);
    eisier_reg = (uint16_t)(nisier_v >> 16);

    if (eisier_reg != 0)
      {
        nisier_reg |= SDMMC_NISTR_ERRINT;
      }

    isrflags  = enter_critical_section();

    /* Clear all pending W1C status bits (Harmony: NISTR=0x01FF, EISTR=0x03FF) */

    sam_putreg16(priv, 0x01ffu,         SAM_SDMMC_NISTR_OFFSET);
    sam_putreg16(priv, SDMMC_EISTR_ALL, SAM_SDMMC_EISTR_OFFSET);

    /* Combine accumulated bits from dmarecvsetup + waitenable (if called first) */

    sam_putreg16(priv, nisier_reg, SAM_SDMMC_NISIER_OFFSET);
    sam_putreg16(priv, eisier_reg, SAM_SDMMC_EISIER_OFFSET);

    sam_putreg(priv, arg,    SAM_SDMMC_ARG1R_OFFSET);
    sam_putreg(priv, regval, SAM_SDMMC_TMR_OFFSET);

    leave_critical_section(isrflags);
  }

  return OK;
}

#ifdef CONFIG_SDIO_BLOCKSETUP
static void sam_blocksetup(struct sdio_dev_s *dev,
                            unsigned int blocklen, unsigned int nblocks)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  sam_putreg(priv, (nblocks << 16) | (blocklen & 0xffffu),
             SAM_SDMMC_BSR_OFFSET);
}
#endif

#ifndef CONFIG_PIC32CZCA90_SDMMC1_DMA
static int sam_recvsetup(struct sdio_dev_s *dev, uint8_t *buffer,
                          size_t nbytes)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  DEBUGASSERT(priv != NULL && buffer != NULL && nbytes > 0);
  DEBUGASSERT(((uint32_t)(uintptr_t)buffer & 3) == 0);

  priv->buffer    = (uint32_t *)buffer;
  priv->remaining = nbytes;

  sam_dataconfig(priv, false, nbytes, SDMMC_DTOCV_MAXTIMEOUT);
  sam_configxfrints(priv, SDMMC_RCVDONE_INTS);

  return OK;
}

static int sam_sendsetup(struct sdio_dev_s *dev,
                          const uint8_t *buffer, size_t nbytes)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  DEBUGASSERT(priv != NULL && buffer != NULL && nbytes > 0);
  DEBUGASSERT(((uint32_t)(uintptr_t)buffer & 3) == 0);

  priv->buffer    = (uint32_t *)buffer;
  priv->remaining = nbytes;

  sam_dataconfig(priv, true, nbytes, SDMMC_DTOCV_MAXTIMEOUT);
  sam_configxfrints(priv, SDMMC_SNDDONE_INTS);

  return OK;
}
#endif /* !CONFIG_PIC32CZCA90_SDMMC1_DMA */

static int sam_cancel(struct sdio_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  sam_configxfrints(priv, 0);
  sam_configwaitints(priv, 0, 0, 0);

  priv->waitevents = 0;
  priv->wkupevent  = 0;
  priv->waitints   = 0;
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
  priv->xfrflags   = 0;
#endif

  wd_cancel(&priv->waitwdog);

  priv->buffer    = 0;
  priv->remaining = 0;
  priv->xfrints   = 0;

  return OK;
}

static int sam_waitresponse(struct sdio_dev_s *dev, uint32_t cmd)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  clock_t timeout;
  clock_t start;
  clock_t elapsed;
  uint32_t nistr;
  int ret = OK;

  switch (cmd & MMCSD_RESPONSE_MASK)
    {
    case MMCSD_R1_RESPONSE:
    case MMCSD_R1B_RESPONSE:
    case MMCSD_R2_RESPONSE:
    case MMCSD_R6_RESPONSE:
      timeout = SDMMC_LONGTIMEOUT;
      break;

    case MMCSD_R4_RESPONSE:
    case MMCSD_R5_RESPONSE:
      return -ENOSYS;

    case MMCSD_NO_RESPONSE:
    case MMCSD_R3_RESPONSE:
    case MMCSD_R7_RESPONSE:
      timeout = SDMMC_CMDTIMEOUT;
      break;

    default:
      return -EINVAL;
    }

  start = clock_systime_ticks();

  do
    {
      nistr = sam_getreg(priv, SAM_SDMMC_NISTR_OFFSET);
      if ((nistr & SDMMC_RESPERR_INTS) != 0)
        {
          uint32_t psr = sam_getreg(priv, SAM_SDMMC_PSR_OFFSET);
          uint16_t ccr = sam_getreg16(priv, SAM_SDMMC_CCR_OFFSET);
          uint8_t  pcr = (uint8_t)sam_getreg8(priv, SAM_SDMMC_PCR_OFFSET);
          mcerr("CMD%d error NISTR=%08" PRIx32 " PSR=%08" PRIx32
                " CCR=%04" PRIx32 " PCR=%02" PRIx32 "\n",
                (int)((cmd & MMCSD_CMDIDX_MASK) >> MMCSD_CMDIDX_SHIFT),
                nistr, psr, (uint32_t)ccr, (uint32_t)pcr);
          ret = -EIO;
          break;
        }

      if ((nistr & SDMMC_INT_CC) != 0)
        {
          break;
        }

      elapsed = clock_systime_ticks() - start;
      if (elapsed >= timeout)
        {
          mcerr("Timeout waiting for response\n");
          ret = -ETIMEDOUT;
          break;
        }
    }
  while (1);

  sam_putreg(priv, SDMMC_RESPDONE_INTS, SAM_SDMMC_NISTR_OFFSET);

  if (ret != OK)
    {
      /* After any command error PSR.CMDINHC stays set until SWRSTCMD.
       * If DAT line is also stuck (CMDINHD=1), SWRSTDAT is required too —
       * otherwise every subsequent command will timeout forever (field failure
       * scenario: card internal error leaves DAT0 busy permanently).
       * Harmony ErrorReset pattern: SWRSTCMD + SWRSTDAT together. */

      uint8_t rst_bits = SDMMC_SRR_SWRSTCMD;

      if (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) & SDMMC_PSR_CMDINHD)
        {
          rst_bits |= SDMMC_RESET_DATA;
        }

      uint16_t rst_timeout = 200;
      sam_putreg8(priv, rst_bits, SAM_SDMMC_SRR_OFFSET);

      while (sam_getreg8(priv, SAM_SDMMC_SRR_OFFSET) & rst_bits)
        {
          if (rst_timeout-- == 0)
            {
              mcerr("CMD/DAT reset never completed\n");
              break;
            }

          usleep(10);
        }

      /* Remember the error so sam_recvshort/sam_recvshortcrc can report it.
       * NISTR and EISTR are already cleared by the W1C write and software
       * reset above, so the recv functions cannot check them directly. */

      priv->cmd_error = true;
    }
  else
    {
      priv->cmd_error = false;
    }

  return ret;
}

static int sam_recvshortcrc(struct sdio_dev_s *dev, uint32_t cmd,
                             uint32_t *rshort)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  /* sam_waitresponse already cleared NISTR via W1C — check cmd_error flag
   * set by waitresponse instead (same pattern as sam_recvshort). */

  if (priv->cmd_error)
    {
      if (rshort)
        {
          *rshort = 0;
        }

      return -ETIMEDOUT;
    }

  if (rshort)
    {
      *rshort = sam_getreg(priv, SAM_SDMMC_RR0_OFFSET);
    }

  return OK;
}

static int sam_recvlong(struct sdio_dev_s *dev, uint32_t cmd,
                         uint32_t rlong[4])
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  /* sam_waitresponse already cleared NISTR via W1C — check cmd_error flag
   * set by waitresponse instead. */

  if (priv->cmd_error)
    {
      return -ETIMEDOUT;
    }

  if (rlong)
    {
      /* The SDMMC hardware stores the 136-bit R2 response right-justified:
       * RR3[23:0] = CSD[127:104], RR2 = CSD[103:72], RR1 = CSD[71:40],
       * RR0 = CSD[39:8].  CSD[7:0] (internal CRC) is not stored.
       * Reassemble into the 4-word layout NuttX expects (csd[0]=CSD[127:96])
       * by shifting the 128-bit register value left by 8 bits. */

      uint32_t rsp3 = sam_getreg(priv, SAM_SDMMC_RR3_OFFSET);
      uint32_t rsp2 = sam_getreg(priv, SAM_SDMMC_RR2_OFFSET);
      uint32_t rsp1 = sam_getreg(priv, SAM_SDMMC_RR1_OFFSET);
      uint32_t rsp0 = sam_getreg(priv, SAM_SDMMC_RR0_OFFSET);

      rlong[0] = rsp3 << 8 | rsp2 >> 24;
      rlong[1] = rsp2 << 8 | rsp1 >> 24;
      rlong[2] = rsp1 << 8 | rsp0 >> 24;
      rlong[3] = rsp0 << 8;
    }

  return OK;
}

static int sam_recvshort(struct sdio_dev_s *dev, uint32_t cmd,
                          uint32_t *rshort)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  uint32_t regval;
  int ret = OK;

  /* If the preceding sam_waitresponse detected a command error (CMDTOE,
   * CMDCRC, etc.), NISTR/EISTR were already cleared by the W1C write and
   * SWRSTCMD.  Propagate the error here so the mmcsd upper layer sees a
   * failed response rather than a zero OCR (which it would misinterpret as
   * "MMC card not ready, keep polling").  This is critical for the
   * CMD1-vs-CMD8 identification path: CMD1 timeout must look like a failed
   * response, not an OCR=0 response, or mmcsd never tries CMD8. */

  if (priv->cmd_error)
    {
      if (rshort)
        {
          *rshort = 0;
        }

      return -ETIMEDOUT;
    }

  regval = sam_getreg(priv, SAM_SDMMC_NISTR_OFFSET);

  if (regval & SDMMC_INT_CTOE)
    {
      mcerr("Command timeout: %08" PRIx32 "\n", regval);
      ret = -ETIMEDOUT;
    }

  if (rshort)
    {
      *rshort = sam_getreg(priv, SAM_SDMMC_RR0_OFFSET);
    }

  return ret;
}

static void sam_waitenable(struct sdio_dev_s *dev,
                            sdio_eventset_t eventset, uint32_t timeout)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  uint32_t waitints;

  DEBUGASSERT(priv != NULL);

  waitints = 0;
  if ((eventset & SDIOWAIT_CMDDONE) != 0)
    {
      waitints |= SDMMC_RESPDONE_INTS;
    }

  if ((eventset & SDIOWAIT_TRANSFERDONE) != 0)
    {
#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA
      /* DMA mode: ADMA2 owns the buffer; BRR/BWR have no ISR handler here
       * and would storm if left enabled.  Only TC + data error bits needed. */

      waitints |= SDMMC_DMADONE_INTS;
#else
      waitints |= SDMMC_XFRDONE_INTS;
#endif
    }

  sam_configwaitints(priv, waitints, eventset, 0);

  if ((priv->waitevents & SDIOWAIT_TIMEOUT) != 0)
    {
      if (!timeout)
        {
          priv->wkupevent = SDIOWAIT_TIMEOUT;
          return;
        }

      wd_start(&priv->waitwdog, MSEC2TICK(timeout),
               sam_eventtimeout, (wdparm_t)priv);
    }
}

static sdio_eventset_t sam_eventwait(struct sdio_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  sdio_eventset_t wkupevent = 0;

  DEBUGASSERT(priv->waitevents != 0 || priv->wkupevent != 0);

  sam_takesem(priv);
  wkupevent = priv->wkupevent;

  priv->waitevents = 0;
  priv->wkupevent  = 0;
  priv->waitints   = 0;
  sam_configwaitints(priv, 0, 0, 0);

  return wkupevent;
}

static void sam_callbackenable(struct sdio_dev_s *dev,
                                sdio_eventset_t eventset)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  mcinfo("eventset: %02x\n", eventset);
  DEBUGASSERT(priv != NULL);

  priv->cbevents = eventset;
  sam_callback(priv);
}

static int sam_registercallback(struct sdio_dev_s *dev,
                                 worker_t callback, void *arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;

  priv->cbevents = 0;
  priv->cbarg    = arg;
  priv->callback = callback;

  return OK;
}

/****************************************************************************
 * ADMA2 DMA setup (when CONFIG_PIC32CZCA90_SDMMC1_DMA=y)
 *
 * Harmony reference: plib_sdmmc1.c SDMMC_DMASetup()
 * Single descriptor covers the full transfer buffer (max 65535 bytes).
 * D-cache must be coherent: invalidate RX before DMA, clean TX before DMA,
 * clean descriptor table before writing its address to ASAR.
 ****************************************************************************/

#ifdef CONFIG_PIC32CZCA90_SDMMC1_DMA

static int sam_dmarecvsetup(struct sdio_dev_s *dev,
                             uint8_t *buffer, size_t buflen)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  uint8_t hc1;

  DEBUGASSERT(priv != NULL && buffer != NULL && buflen > 0);
  DEBUGASSERT(((uint32_t)(uintptr_t)buffer & 3) == 0);

  priv->buffer    = (uint32_t *)buffer;
  priv->remaining = buflen;
  priv->bufferend = (uint32_t *)(buffer + buflen);

  /* Invalidate RX buffer before ADMA2 writes into it */

  up_invalidate_dcache((uintptr_t)buffer, (uintptr_t)buffer + buflen);

  /* Build ADMA2 descriptor */

  g_adma_desc.attribute = SDMMC_ADMA_XFER_LAST;
  g_adma_desc.length    = (uint16_t)(buflen & 0xffffu);  /* 0 = 65536 */
  g_adma_desc.address   = (uint32_t)(uintptr_t)buffer;

  /* Clean descriptor to main memory before writing address to ASAR */

  up_clean_dcache((uintptr_t)&g_adma_desc,
                   (uintptr_t)&g_adma_desc + sizeof(sdmmc_adma_desc_t));

  sam_dataconfig(priv, false, buflen, SDMMC_DTOCV_MAXTIMEOUT);

  /* HC1R: select ADMA2 (8-bit access to avoid disturbing PCR/BGCR/WCR) */

  hc1  = (uint8_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET);
  hc1 &= (uint8_t)~SDMMC_HC1_DMAS_MASK;
  hc1 |= (uint8_t)SDMMC_HC1_DMAS_ADMA;
  sam_putreg8(priv, hc1, SAM_SDMMC_HC1R_OFFSET);

  /* Write descriptor table base address to ASAR */

  sam_putreg(priv, (uint32_t)(uintptr_t)&g_adma_desc,
             SAM_SDMMC_ASAR_OFFSET);

  sam_configxfrints(priv, SDMMC_DMADONE_INTS | SDMMC_INT_DINT);

  mcinfo("dmarecv: buf=%p len=%zu desc@%p attr=%04x ASAR=%08" PRIx32
         " HC1R=%02" PRIx32 " NISIER=%08" PRIx32 "\n",
         buffer, buflen, &g_adma_desc,
         g_adma_desc.attribute,
         sam_getreg(priv, SAM_SDMMC_ASAR_OFFSET),
         (uint32_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET),
         sam_getreg(priv, SAM_SDMMC_NISIER_OFFSET));
  return OK;
}

static int sam_dmasendsetup(struct sdio_dev_s *dev,
                             const uint8_t *buffer, size_t buflen)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  uint8_t hc1;

  DEBUGASSERT(priv != NULL && buffer != NULL && buflen > 0);
  DEBUGASSERT(((uint32_t)(uintptr_t)buffer & 3) == 0);

  priv->buffer    = (uint32_t *)buffer;
  priv->remaining = buflen;
  priv->bufferend = (uint32_t *)(buffer + buflen);

  /* Clean TX buffer to main memory before ADMA2 reads it */

  up_clean_dcache((uintptr_t)buffer, (uintptr_t)buffer + buflen);

  /* Build ADMA2 descriptor */

  g_adma_desc.attribute = SDMMC_ADMA_XFER_LAST;
  g_adma_desc.length    = (uint16_t)(buflen & 0xffffu);
  g_adma_desc.address   = (uint32_t)(uintptr_t)buffer;

  up_clean_dcache((uintptr_t)&g_adma_desc,
                   (uintptr_t)&g_adma_desc + sizeof(sdmmc_adma_desc_t));

  sam_dataconfig(priv, true, buflen, SDMMC_DTOCV_MAXTIMEOUT);

  hc1  = (uint8_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET);
  hc1 &= (uint8_t)~SDMMC_HC1_DMAS_MASK;
  hc1 |= (uint8_t)SDMMC_HC1_DMAS_ADMA;
  sam_putreg8(priv, hc1, SAM_SDMMC_HC1R_OFFSET);

  sam_putreg(priv, (uint32_t)(uintptr_t)&g_adma_desc,
             SAM_SDMMC_ASAR_OFFSET);

  sam_configxfrints(priv, SDMMC_DMADONE_INTS | SDMMC_INT_DINT);
  return OK;
}

#endif /* CONFIG_PIC32CZCA90_SDMMC1_DMA */

/****************************************************************************
 * Callback
 ****************************************************************************/

static void sam_callback(void *arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)arg;

  DEBUGASSERT(priv != NULL);

  if (priv->callback)
    {
      if ((priv->cdstatus & SDIO_STATUS_PRESENT) != 0)
        {
          if ((priv->cbevents & SDIOMEDIA_INSERTED) == 0)
            {
              return;
            }
        }
      else
        {
          if ((priv->cbevents & SDIOMEDIA_EJECTED) == 0)
            {
              return;
            }
        }

      priv->cbevents = 0;

      if (up_interrupt_context())
        {
          work_queue(HPWORK, &priv->cbwork, priv->callback, priv->cbarg, 0);
        }
      else
        {
          priv->callback(priv->cbarg);
        }
    }
}

/****************************************************************************
 * UHS mode timing (HC2R)
 ****************************************************************************/

static void sam_set_uhs_timing(struct sam_dev_s *priv,
                                enum bus_mode selected_mode)
{
  uint16_t reg;

  reg = (uint16_t)sam_getreg16(priv, SAM_SDMMC_HC2R_OFFSET);
  reg &= (uint16_t)~SDMMC_HC2_UHS_MASK;

  switch (selected_mode)
    {
      case UHS_SDR50:
      case MMC_HS_52:
        reg |= SDMMC_HC2_UHS_SDR50;
        break;
      case UHS_DDR50:
      case MMC_DDR_52:
        reg |= SDMMC_HC2_UHS_DDR50;
        break;
      case UHS_SDR104:
      case MMC_HS_200:
        reg |= SDMMC_HC2_UHS_SDR104;
        break;
      default:
        reg |= SDMMC_HC2_UHS_SDR12;
        break;
    }

  sam_putreg16(priv, reg, SAM_SDMMC_HC2R_OFFSET);
}

/****************************************************************************
 * Clock setup — matches Harmony plib_sdmmc1.c SDMMC1_ClockSet() exactly.
 *
 * Reads CA0R.BASECLKF and CA1R.CLKMULT from hardware at runtime.
 * Programmable mode (CLKGSEL=1, CLKMULT>0):
 *   F_MULTCLK = baseclk × (CLKMULT + 1)
 *   divider   = F_MULTCLK / target − 1
 * Divided mode fallback (CLKGSEL=0, CLKMULT==0):
 *   divider   = ceil(baseclk / (2 × target))
 ****************************************************************************/

static int sam_set_clock(struct sam_dev_s *priv, uint32_t clock)
{
  uint16_t ccr;
  uint32_t div;
  uint16_t timeout;
  uint8_t  hc1;
  uint32_t baseclk;
  uint32_t clkmul;

  /* Wait for CMD/DAT inhibit */

  timeout = 200;
  while (sam_getreg(priv, SAM_SDMMC_PSR_OFFSET) &
         (SDMMC_PSR_CMDINHC | SDMMC_PSR_CMDINHD))
    {
      if (timeout == 0)
        {
          mcerr("Timeout waiting for inhibit to clear\n");
          return -EBUSY;
        }

      timeout--;
      usleep(100);
    }

  /* Disable SD clock */

  sam_putreg16(priv, 0, SAM_SDMMC_CCR_OFFSET);

  if (clock == 0)
    {
      return OK;
    }

  /* Read base clock from CA0R.BASECLKF (in MHz); fall back if 0 */

  baseclk = (sam_getreg(priv, SAM_SDMMC_CA0R_OFFSET) & SDMMC_CA0R_BASECLKF_Msk)
             >> SDMMC_CA0R_BASECLKF_Pos;
  if (baseclk == 0u)
    {
      baseclk = SDMMC1_BASE_CLOCK_FREQUENCY / 2u;
    }
  else
    {
      baseclk *= 1000000u;
    }

  /* Read programmable clock multiplier from CA1R.CLKMULT */

  clkmul = (sam_getreg(priv, SAM_SDMMC_CA1R_OFFSET) & SDMMC_CA1R_CLKMULT_Msk)
            >> SDMMC_CA1R_CLKMULT_Pos;

  /* High-speed mode: set/clear HC1R.HSEN before programming divider.
   * Harmony does this inside the CLKMULT>0 branch with the IP-limitation
   * guard (if HSEN and div==0, force div=1). */

  hc1 = (uint8_t)sam_getreg8(priv, SAM_SDMMC_HC1R_OFFSET);
  if (clock > SDMMC1_BUS_HIGH_SPEED_THRESHOLD)
    {
      hc1 |= (uint8_t)SDMMC_HC1_HSEN;
    }
  else
    {
      hc1 &= (uint8_t)~SDMMC_HC1_HSEN;
    }

  sam_putreg8(priv, hc1, SAM_SDMMC_HC1R_OFFSET);

  if (clkmul > 0u)
    {
      /* Programmable mode (Harmony CA80/CA90 path):
       * F_SDCLK = (baseclk × (clkmul+1)) / (divider+1) */

      div = (baseclk * (clkmul + 1u)) / clock;
      if (div > 0u)
        {
          div--;
        }

      /* IP limitation: HSEN set requires divider ≥ 1 */

      if ((hc1 & SDMMC_HC1_HSEN) != 0u && div == 0u)
        {
          div = 1u;
        }

      ccr = (uint16_t)(SDMMC_CCR_CLKGSEL | SDMMC_CCR_INTCLKEN |
                       SDMMC_CCR_SDCLKFSEL_DIV(div));
    }
  else
    {
      /* Divided mode fallback (CLKGSEL=0):
       * F_SDCLK = baseclk / (2 × divider) */

      div = (baseclk + 2u * clock - 1u) / (2u * clock);
      if (div > 1023u) div = 1023u;
      if (div == 0u)   div = 1u;

      ccr = (uint16_t)(SDMMC_CCR_INTCLKEN | SDMMC_CCR_SDCLKFSEL_DIV(div));
    }

  sam_putreg16(priv, ccr, SAM_SDMMC_CCR_OFFSET);

  /* Wait for internal clock stable */

  timeout = 200;
  while (!(sam_getreg16(priv, SAM_SDMMC_CCR_OFFSET) & SDMMC_CCR_INTCLKS))
    {
      if (timeout == 0)
        {
          mcerr("Internal clock never stabilised\n");
          return -EBUSY;
        }

      timeout--;
      usleep(100);
    }

  /* Enable SD clock */

  ccr |= SDMMC_CCR_SDCLKEN;
  sam_putreg16(priv, ccr, SAM_SDMMC_CCR_OFFSET);

  return OK;
}

/****************************************************************************
 * Power setup — query CA0R capabilities and apply bus voltage.
 *
 * Harmony reference: plib_sdmmc1.c SDMMC_InitCard() power/voltage sequence.
 ****************************************************************************/

static void sam_power(struct sam_dev_s *priv)
{
  uint8_t  card_power = 0;
  uint32_t caps0      = sam_getreg(priv, SAM_SDMMC_CA0R_OFFSET);

  if (caps0 & SDMMC_CA0_VS33)
    {
      card_power = SDMMC_POWER_330;
    }
  else if (caps0 & SDMMC_CA0_VS30)
    {
      card_power = SDMMC_POWER_300;
    }
  else if (caps0 & SDMMC_CA0_VS18)
    {
      card_power = SDMMC_POWER_180;
    }

  if (card_power == 0)
    {
      sam_putreg8(priv, 0, SAM_SDMMC_PCR_OFFSET);
      return;
    }

  sam_putreg8(priv, card_power, SAM_SDMMC_PCR_OFFSET);

  if (caps0 & SDMMC_CA0_DDR50)
    {
      sam_set_uhs_timing(priv, UHS_DDR50);
    }
  else if (caps0 & SDMMC_CA0_SDR50)
    {
      sam_set_uhs_timing(priv, UHS_SDR50);
    }
}

static int sam_set_interrupts(struct sam_dev_s *priv)
{
  /* Enable all normal + error status bits — matches Harmony NISTER=Msk, EISTER=Msk.
   * Combined 32-bit write at NISTER_OFFSET: [15:0]=NISTER, [31:16]=EISTER. */

  sam_putreg(priv, SDMMC_INT_ALL, SAM_SDMMC_NISTER_OFFSET);

  /* Signal only card insertion/removal via NVIC at init — Harmony: NISIER=CINS|CREM.
   * Per-transfer interrupt signals are programmed by sam_configwaitints(). */

  sam_putreg16(priv, (uint16_t)(SDMMC_INT_CINS | SDMMC_INT_CRM),
               SAM_SDMMC_NISIER_OFFSET);
  sam_putreg16(priv, 0, SAM_SDMMC_EISIER_OFFSET);
  return OK;
}

/****************************************************************************
 * CA90 clock enable helper.
 *
 * Enables GCLK4 (100 MHz, channel 60) and GCLK5 (12 MHz, channel 61) for
 * SDMMC1, plus MCLK AHB (ID=71) and APB (ID=72) gates.
 * Mirrors Harmony plib_clock.c generator + channel + peripheral clock setup.
 ****************************************************************************/

void sdmmc1_clk_enable(void)
{
  uint32_t id;
  uint32_t reg;
  uint32_t bit;

  /* Configure GCLK4 generator: PLL0 / 3 = 300 MHz / 3 = 100 MHz (SDMMC main).
   * Must be done before enabling the channel — generator reset default is
   * GENEN=0, which leaves the channel with no clock. */

  static const struct sam_gclk_config_s gclk4_cfg =
    {
      .enable   = 1,
      .source   = 6u,  /* GCLK_GENCTRL_SRC_PLL0_1 = 300 MHz */
      .div      = 3u,  /* 300 / 3 = 100 MHz */
    };

  sam_gclk_configure(4, &gclk4_cfg);

  /* Configure GCLK5 generator: PLL0 / 25 = 300 MHz / 25 = 12 MHz (SDMMC slow). */

  static const struct sam_gclk_config_s gclk5_cfg =
    {
      .enable   = 1,
      .source   = 6u,  /* GCLK_GENCTRL_SRC_PLL0_1 = 300 MHz */
      .div      = 25u, /* 300 / 25 = 12 MHz */
    };

  sam_gclk_configure(5, &gclk5_cfg);

  /* GCLK4 → SDMMC1 main clock (GCLK_PCHCTRL[60]) */

  sam_gclk_chan_enable(SAM_SDMMC1_GCLK_ID, 4u, false);

  /* GCLK5 → SDMMC1 slow clock (GCLK_PCHCTRL[61]) */

  sam_gclk_chan_enable(SAM_SDMMC1_GCLK_ID_SLOW, 5u, false);

  /* MCLK AHB */

  id  = SAM_SDMMC1_MCLK_ID_AHB;
  reg = SAM_MCLK_CLKMSK_ADDR(id);
  bit = SAM_MCLK_CLKMSK_BIT(id);
  putreg32(getreg32(reg) | bit, reg);

  /* MCLK APB */

  id  = SAM_SDMMC1_MCLK_ID_APB;
  reg = SAM_MCLK_CLKMSK_ADDR(id);
  bit = SAM_MCLK_CLKMSK_BIT(id);
  putreg32(getreg32(reg) | bit, reg);

}

/****************************************************************************
 * Public Function: sam_sdmmc1_initialize
 *
 * Called from board_app_initialize() after shared pins are muxed to SDMMC1
 * function (mux I = 8). Returns the sdio_dev_s pointer to pass to
 * mmcsd_slotinitialize().
 ****************************************************************************/

struct sdio_dev_s *sam_sdmmc1_initialize(void)
{
  struct sam_dev_s *priv = &g_sdmmcdev;

  nxsem_init(&priv->waitsem, 0, 0);
  nxsem_set_protocol(&priv->waitsem, SEM_PRIO_NONE);

  priv->base = SAM_SDMMC1_BASE;

  /* Configure CD pin (PC28, GPIO input, pull-up, active LOW) */

#if defined(PIN_SDMMC1_CD)
  sam_portconfig(PIN_SDMMC1_CD);
  priv->sw_cd_gpio  = PIN_SDMMC1_CD;
  priv->cd_invert   = 0;
#endif

  sdmmc1_clk_enable();

  /* Wait for GCLK4/GCLK5/MCLK gates to propagate before accessing SDMMC
   * registers.  Also forces arm_udelay.o to be co-linked from libarch.a:
   * libdrivers_board.a lands outside --start-group (CMake INTERFACE dep
   * expansion), so the only libarch.a occurrence that follows libdrivers.a
   * is the one that pulls sam_sdmmc.o.  Referencing up_udelay here causes
   * the linker to rescan that same occurrence and co-pull arm_udelay.o
   * before mmcsd_sdio.o is resolved. */
  up_udelay(1);

  sam_reset(&priv->dev);
  sam_clock(&priv->dev, CLOCK_SDIO_DISABLED);
  sam_power(priv);
  sam_set_interrupts(priv);

  return &priv->dev;
}

/****************************************************************************
 * Public APIs: sdio_mediachange, sdio_wrprotect, sam_sdmmc_set_sdio_card_isr
 ****************************************************************************/

void sdio_mediachange(struct sdio_dev_s *dev, bool cardinslot)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  sdio_statset_t cdstatus;
  irqstate_t flags;

  flags    = enter_critical_section();
  cdstatus = priv->cdstatus;

  if (cardinslot)
    {
      priv->cdstatus |= SDIO_STATUS_PRESENT;
    }
  else
    {
      priv->cdstatus &= ~SDIO_STATUS_PRESENT;
    }

  if (cdstatus != priv->cdstatus)
    {
      sam_callback(priv);
    }

  leave_critical_section(flags);
}

void sdio_wrprotect(struct sdio_dev_s *dev, bool wrprotect)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  irqstate_t flags;

  flags = enter_critical_section();
  if (wrprotect)
    {
      priv->cdstatus |= SDIO_STATUS_WRPROTECTED;
    }
  else
    {
      priv->cdstatus &= ~SDIO_STATUS_WRPROTECTED;
    }

  leave_critical_section(flags);
}

void sam_sdmmc_set_sdio_card_isr(struct sdio_dev_s *dev,
                                  int (*func)(void *), void *arg)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev;
  irqstate_t flags;
  uint32_t regval;

  priv->do_sdio_card = func;
  priv->do_sdio_arg  = arg;

  priv->cintints = (func != NULL) ? SDMMC_INT_CINT : 0;

#if defined(CONFIG_MMCSD_HAVE_CARDDETECT)
  if (priv->sw_cd_gpio == 0)
    {
      priv->cintints |= SDMMC_INT_CINS | SDMMC_INT_CRM;
    }
#endif

  flags  = enter_critical_section();
  regval = sam_getreg16(priv, SAM_SDMMC_NISIER_OFFSET);
  regval = (regval & ~(SDMMC_INT_CINT & 0xffffu)) |
           (priv->cintints & 0xffffu);
  sam_putreg16(priv, (uint16_t)regval, SAM_SDMMC_NISIER_OFFSET);
  leave_critical_section(flags);
}

/****************************************************************************
 * Public Function: sam_sdmmc1_slotinitialize
 *
 * Combined init + mmcsd_slotinitialize wrapper following SAMV7 chip-layer
 * pattern (arch/arm/src/samv7/sam_hsmci.c).
 *
 * Placing this call here — inside libarch.a — ensures that mmcsd_sdio.o
 * gets pulled from libdrivers.a inside the --start-group block, where
 * up_udelay from arm_udelay.o (also libarch.a) is available.  Calling
 * mmcsd_slotinitialize only from board init.c (outside the group) causes
 * a linker "undefined reference to up_udelay" because arm_udelay.o is not
 * re-scanned after the group closes.
 ****************************************************************************/

int sam_sdmmc1_slotinitialize(int minor)
{
  struct sdio_dev_s *sdio = sam_sdmmc1_initialize();
  if (sdio == NULL)
    {
      syslog(LOG_ERR, "sam_sdmmc1_slotinitialize: hw init failed\n");
      return -ENODEV;
    }

  return mmcsd_slotinitialize(minor, sdio);
}

#endif /* CONFIG_PIC32CZCA90_SDMMC1 */
