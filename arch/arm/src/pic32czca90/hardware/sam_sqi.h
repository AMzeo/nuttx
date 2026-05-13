/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_sqi.h
 *
 * PIC32CZ CA90 SQI (Serial Quad Interface) register definitions.
 *
 * All values verified against PIC32CZ8110CA80208_DFP component/sqi.h and
 * instance/sqi1.h (DFP v1.7.168, file date 2024-07-31).
 *
 * Key instance parameters for SQI1:
 *   SAM_SQI1_BASE        = 0x4F009000  (peripheral base address)
 *   SAM_SQI1_GCLK_ID     = 57          (GCLK peripheral channel → GCLK2)
 *   SAM_SQI1_MCLK_ID_AHB = 67          (MCLK AHB clock enable)
 *   SAM_SQI1_XIP_BASE    = 0x90000000  (XIP memory-mapped window)
 *   No APB clock — SQI uses AHB bus only.
 *
 * BD descriptor structs must be placed in the nocache MPU region
 * (linker-reserved at 0x200F0000).
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SQI_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SQI_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * SQI1 Instance Parameters
 * =========================================================================
 */

/* SAM_SQI1_BASE is defined in hardware/pic32czca90_memorymap.h */
#define SAM_SQI1_GCLK_ID     57           /* Route to GCLK2 (100 MHz) */
#define SAM_SQI1_MCLK_ID_AHB 67          /* MCLK AHB clock enable */
#define SAM_SQI1_XIP_BASE    0x90000000u  /* XIP window base */

/* =========================================================================
 * Register Offsets (from SQI base address)
 * All verified from DFP component/sqi.h
 * =========================================================================
 */

#define SAM_SQI_CTRLA_OFFSET     0x0000  /* Control A (R/W 8) */
#define SAM_SQI_INTENCLR_OFFSET  0x0010  /* Interrupt Enable Clear (R/W 8) */
#define SAM_SQI_INTENSET_OFFSET  0x0014  /* Interrupt Enable Set (R/W 8) */
#define SAM_SQI_INTFLAG_OFFSET   0x0018  /* Interrupt Flag (R/W 8) */
#define SAM_SQI_SYNCBUSY_OFFSET  0x0020  /* Synchronization Busy (R 32) */
#define SAM_SQI_XCON1_OFFSET     0x0100  /* XIP Control 1 (R/W 32) */
#define SAM_SQI_XCON2_OFFSET     0x0104  /* XIP Control 2 (R/W 32) */
#define SAM_SQI_CFG_OFFSET       0x0108  /* Configuration (R/W 32) */
#define SAM_SQI_CON_OFFSET       0x010C  /* Control (R/W 32) */
#define SAM_SQI_CLKCON_OFFSET    0x0110  /* Clock Control (R/W 32) */
#define SAM_SQI_CMDTHR_OFFSET    0x0114  /* Command Threshold (R/W 32) */
#define SAM_SQI_INTTHR_OFFSET    0x0118  /* Interrupt Threshold (R/W 32) */
#define SAM_SQI_INTEN_OFFSET     0x011C  /* Interrupt Enable (R/W 32) */
#define SAM_SQI_INTSTAT_OFFSET   0x0120  /* Interrupt Status (R/W 32) */
#define SAM_SQI_TXDATA_OFFSET    0x0124  /* TX Data FIFO (W 32) */
#define SAM_SQI_RXDATA_OFFSET    0x0128  /* RX Data FIFO (R 32) */
#define SAM_SQI_STAT1_OFFSET     0x012C  /* Status 1 (R 32) */
#define SAM_SQI_STAT2_OFFSET     0x0130  /* Status 2 (R 32) */
#define SAM_SQI_BDCON_OFFSET     0x0134  /* BD DMA Control (R/W 32) */
#define SAM_SQI_BDCURADD_OFFSET  0x0138  /* BD Current Address (R 32) */
#define SAM_SQI_BDBASEADD_OFFSET 0x0140  /* BD Base Address (R/W 32) */
#define SAM_SQI_BDSTAT_OFFSET    0x0144  /* BD DMA Status (R 32) */
#define SAM_SQI_BDPOLLCON_OFFSET 0x0148  /* BD Poll Control (R/W 32) */
#define SAM_SQI_BDTXDSTAT_OFFSET 0x014C  /* BD TX Data Status (R 32) */
#define SAM_SQI_BDRXDSTAT_OFFSET 0x0150  /* BD RX Data Status (R 32) */
#define SAM_SQI_THR_OFFSET       0x0154  /* Threshold (R/W 32) */
#define SAM_SQI_INTSIGEN_OFFSET  0x0158  /* Interrupt Signal Enable (R/W 32) */
#define SAM_SQI_TAPCON_OFFSET    0x015C  /* TAP Control (R/W 32) */
#define SAM_SQI_MEMSTAT_OFFSET   0x0160  /* Memory Status (R/W 32) */

/* =========================================================================
 * SQI1 Register Addresses
 * =========================================================================
 */

#define SAM_SQI1_CTRLA      (SAM_SQI1_BASE + SAM_SQI_CTRLA_OFFSET)
#define SAM_SQI1_INTENCLR   (SAM_SQI1_BASE + SAM_SQI_INTENCLR_OFFSET)
#define SAM_SQI1_INTENSET   (SAM_SQI1_BASE + SAM_SQI_INTENSET_OFFSET)
#define SAM_SQI1_INTFLAG    (SAM_SQI1_BASE + SAM_SQI_INTFLAG_OFFSET)
#define SAM_SQI1_SYNCBUSY   (SAM_SQI1_BASE + SAM_SQI_SYNCBUSY_OFFSET)
#define SAM_SQI1_XCON1      (SAM_SQI1_BASE + SAM_SQI_XCON1_OFFSET)
#define SAM_SQI1_XCON2      (SAM_SQI1_BASE + SAM_SQI_XCON2_OFFSET)
#define SAM_SQI1_CFG        (SAM_SQI1_BASE + SAM_SQI_CFG_OFFSET)
#define SAM_SQI1_CON        (SAM_SQI1_BASE + SAM_SQI_CON_OFFSET)
#define SAM_SQI1_CLKCON     (SAM_SQI1_BASE + SAM_SQI_CLKCON_OFFSET)
#define SAM_SQI1_CMDTHR     (SAM_SQI1_BASE + SAM_SQI_CMDTHR_OFFSET)
#define SAM_SQI1_INTTHR     (SAM_SQI1_BASE + SAM_SQI_INTTHR_OFFSET)
#define SAM_SQI1_INTEN      (SAM_SQI1_BASE + SAM_SQI_INTEN_OFFSET)
#define SAM_SQI1_INTSTAT    (SAM_SQI1_BASE + SAM_SQI_INTSTAT_OFFSET)
#define SAM_SQI1_TXDATA     (SAM_SQI1_BASE + SAM_SQI_TXDATA_OFFSET)
#define SAM_SQI1_RXDATA     (SAM_SQI1_BASE + SAM_SQI_RXDATA_OFFSET)
#define SAM_SQI1_STAT1      (SAM_SQI1_BASE + SAM_SQI_STAT1_OFFSET)
#define SAM_SQI1_STAT2      (SAM_SQI1_BASE + SAM_SQI_STAT2_OFFSET)
#define SAM_SQI1_BDCON      (SAM_SQI1_BASE + SAM_SQI_BDCON_OFFSET)
#define SAM_SQI1_BDCURADD   (SAM_SQI1_BASE + SAM_SQI_BDCURADD_OFFSET)
#define SAM_SQI1_BDBASEADD  (SAM_SQI1_BASE + SAM_SQI_BDBASEADD_OFFSET)
#define SAM_SQI1_BDSTAT     (SAM_SQI1_BASE + SAM_SQI_BDSTAT_OFFSET)
#define SAM_SQI1_BDPOLLCON  (SAM_SQI1_BASE + SAM_SQI_BDPOLLCON_OFFSET)
#define SAM_SQI1_BDTXDSTAT  (SAM_SQI1_BASE + SAM_SQI_BDTXDSTAT_OFFSET)
#define SAM_SQI1_BDRXDSTAT  (SAM_SQI1_BASE + SAM_SQI_BDRXDSTAT_OFFSET)
#define SAM_SQI1_THR        (SAM_SQI1_BASE + SAM_SQI_THR_OFFSET)
#define SAM_SQI1_INTSIGEN   (SAM_SQI1_BASE + SAM_SQI_INTSIGEN_OFFSET)
#define SAM_SQI1_TAPCON     (SAM_SQI1_BASE + SAM_SQI_TAPCON_OFFSET)
#define SAM_SQI1_MEMSTAT    (SAM_SQI1_BASE + SAM_SQI_MEMSTAT_OFFSET)

/* =========================================================================
 * SQI_CTRLA Bits (offset 0x00, R/W 8)
 * =========================================================================
 */

#define SQI_CTRLA_SWRST      (1u << 0)  /* Software Reset */
#define SQI_CTRLA_RUNSTDBY   (1u << 6)  /* Run in Standby */

/* =========================================================================
 * SQI_INTENCLR / SQI_INTENSET / SQI_INTFLAG Bits (offsets 0x10/0x14/0x18, R/W 8)
 * Single SQI interrupt flag — set to enable/clear/flag.
 * =========================================================================
 */

#define SQI_INT_SQI          (1u << 0)  /* SQI interrupt */

/* =========================================================================
 * SQI_SYNCBUSY Bits (offset 0x20, R 32)
 * =========================================================================
 */

#define SQI_SYNCBUSY_SWRST   (1u << 0)  /* Software Reset synchronizing */

/* =========================================================================
 * SQI_XCON1 Bits (offset 0x100, R/W 32) — XIP Control 1
 * =========================================================================
 */

#define SQI_XCON1_TYPECMD_SHIFT     0
#define SQI_XCON1_TYPECMD(v)        (((v) & 0x3u) << 0)
#define SQI_XCON1_TYPEADDR_SHIFT    2
#define SQI_XCON1_TYPEADDR(v)       (((v) & 0x3u) << 2)
#define SQI_XCON1_TYPEMODE_SHIFT    4
#define SQI_XCON1_TYPEMODE(v)       (((v) & 0x3u) << 4)
#define SQI_XCON1_TYPEDUMMY_SHIFT   6
#define SQI_XCON1_TYPEDUMMY(v)      (((v) & 0x3u) << 6)
#define SQI_XCON1_TYPEDATA_SHIFT    8
#define SQI_XCON1_TYPEDATA(v)       (((v) & 0x3u) << 8)
#define SQI_XCON1_READOPCODE_SHIFT  10
#define SQI_XCON1_READOPCODE(v)     (((v) & 0xFFu) << 10)
#define SQI_XCON1_ADDRBYTES_SHIFT   18
#define SQI_XCON1_ADDRBYTES(v)      (((v) & 0x7u) << 18)
#define SQI_XCON1_DUMMYBYTES_SHIFT  21
#define SQI_XCON1_DUMMYBYTES(v)     (((v) & 0x7u) << 21)

/* SST26VF032BAT: Regular Read (0x03), 3-byte addr, no dummy, single lane.
 * ADDRBYTES field: value = number of address bytes (DFP-verified).
 * Must match the flash's Read command address phase length. */
#define SQI_XCON1_SST26_READ       (SQI_XCON1_READOPCODE(0x03) | \
                                    SQI_XCON1_ADDRBYTES(3))

/* =========================================================================
 * SQI_XCON2 Bits (offset 0x104, R/W 32) — XIP Control 2
 * =========================================================================
 */

#define SQI_XCON2_MODECODE_SHIFT    0
#define SQI_XCON2_MODECODE(v)       (((v) & 0xFFu) << 0)
#define SQI_XCON2_MODEBYTES_SHIFT   8
#define SQI_XCON2_MODEBYTES(v)      (((v) & 0x3u) << 8)
#define SQI_XCON2_DEVSEL_SHIFT      10
#define SQI_XCON2_DEVSEL(v)         (((v) & 0x7u) << 10)

#define SQI_XCON2_SST26_CS0        (SQI_XCON2_DEVSEL(0))

/* =========================================================================
 * SQI_CFG Bits (offset 0x108, R/W 32)
 * =========================================================================
 */

/* MODE [2:0] — Operating mode */
#define SQI_CFG_MODE_SHIFT   0
#define SQI_CFG_MODE_MASK    (0x7u << 0)
#define SQI_CFG_MODE(v)      (((v) & 0x7u) << 0)
#define SQI_CFG_MODE_BOOT    SQI_CFG_MODE(0)  /* Boot mode */
#define SQI_CFG_MODE_PIO     SQI_CFG_MODE(1)  /* PIO / FIFO mode */
#define SQI_CFG_MODE_DMA     SQI_CFG_MODE(2)  /* BD-DMA mode */
#define SQI_CFG_MODE_XIP     SQI_CFG_MODE(3)  /* XIP mode */

#define SQI_CFG_CPHA         (1u << 3)   /* SPI Clock Phase (0=sample leading) */
#define SQI_CFG_CPOL         (1u << 4)   /* SPI Clock Polarity (0=idle low) */
#define SQI_CFG_LSBF         (1u << 5)   /* LSB First */
#define SQI_CFG_WP           (1u << 9)   /* WP=1 drives IO2/spiout2 LOW (asserts SST26 WP# = write-protected); keep 0 */
#define SQI_CFG_HOLD         (1u << 10)  /* HOLD=1 drives IO3/spiout3 LOW (asserts SST26 HOLD# = SCK paused); keep 0 */
#define SQI_CFG_BURSTEN            (1u << 11)  /* AHB burst enable — bit 11 (DFP-verified; DS is wrong) */
#define SQI_CFG_AHB_BURST_INCR4_EN (1u << 12)  /* AHB burst INCR4 enable */
#define SQI_CFG_AHB_BURST_INCR8_EN (1u << 13)  /* AHB burst INCR8 enable */
#define SQI_CFG_AHB_BURST_INCR16_EN (1u << 14) /* AHB burst INCR16 enable */
#define SQI_CFG_RESET        (1u << 16)  /* Reset (HC — cleared by hardware) */
/* Michigan Ax: TXBUFRST/RXBUFRST/CONBUFRST are Reserved — NEVER write these bits */
#define SQI_CFG_TXBUFRST     (1u << 17)  /* TX Buffer Reset — NOT for Michigan Ax */
#define SQI_CFG_RXBUFRST     (1u << 18)  /* RX Buffer Reset — NOT for Michigan Ax */
#define SQI_CFG_CONBUFRST    (1u << 19)  /* Control Buffer Reset — NOT for Michigan Ax */

/* DATAEN [21:20] — Data lane count */
#define SQI_CFG_DATAEN_SHIFT 20
#define SQI_CFG_DATAEN_MASK  (0x3u << 20)
#define SQI_CFG_DATAEN(v)    (((v) & 0x3u) << 20)
#define SQI_CFG_DATAEN_1     SQI_CFG_DATAEN(0)  /* SPI (1 lane) */
#define SQI_CFG_DATAEN_2     SQI_CFG_DATAEN(1)  /* DSPI (2 lanes) */
#define SQI_CFG_DATAEN_4     SQI_CFG_DATAEN(2)  /* QSPI (4 lanes) */

#define SQI_CFG_SQIEN        (1u << 23)  /* SQI Enable */

/* CSEN [25:24] — Chip Select Enable (DS: 2-bit field, CS1/CS0) */
#define SQI_CFG_CSEN_SHIFT   24
#define SQI_CFG_CSEN_MASK    (0x3u << 24)
#define SQI_CFG_CSEN(v)      (((v) & 0x3u) << 24)
#define SQI_CFG_CSEN0        (1u << 24)  /* CS0 enable */
#define SQI_CFG_CSEN1        (1u << 25)  /* CS1 enable */

/* =========================================================================
 * SQI_CON Bits (offset 0x10C, R/W 32)
 * PIO mode transfer length and control.
 * =========================================================================
 */

/* TXRXCOUNT [15:0] — number of bytes to transfer in PIO mode */
#define SQI_CON_TXRXCOUNT_SHIFT  0
#define SQI_CON_TXRXCOUNT_MASK   (0xFFFFu << 0)
#define SQI_CON_TXRXCOUNT(v)     (((v) & 0xFFFFu) << 0)

/* CMDINIT [17:16] — command init lanes */
#define SQI_CON_CMDINIT_SHIFT    16
#define SQI_CON_CMDINIT_MASK     (0x3u << 16)
#define SQI_CON_CMDINIT(v)       (((v) & 0x3u) << 16)

/* LANEMODE [19:18] — data lane mode */
#define SQI_CON_LANEMODE_SHIFT   18
#define SQI_CON_LANEMODE_MASK    (0x3u << 18)
#define SQI_CON_LANEMODE(v)      (((v) & 0x3u) << 18)

/* DEVSEL [21:20] — device select */
#define SQI_CON_DEVSEL_SHIFT     20
#define SQI_CON_DEVSEL_MASK      (0x3u << 20)
#define SQI_CON_DEVSEL(v)        (((v) & 0x3u) << 20)

#define SQI_CON_DASSERT          (1u << 22)  /* Deassert CS after transfer */

/* =========================================================================
 * SQI_CLKCON Bits (offset 0x110, R/W 32)
 * =========================================================================
 */

#define SQI_CLKCON_EN            (1u << 0)   /* Clock Enable */
#define SQI_CLKCON_STABLE        (1u << 1)   /* Clock Stable (RO, poll after EN) */

/* CLKDIV [18:8] — 11-bit clock divider (power-of-2 pre-divider of GCLK input)
 * Value 0x001 = /2, 0x002 = /4, 0x004 = /8 … 0x400 = /2048
 * Harmony uses CLKDIV(1) → 100 MHz GCLK2 / 2 = 50 MHz SCK */
#define SQI_CLKCON_CLKDIV_SHIFT  8
#define SQI_CLKCON_CLKDIV_MASK   (0x7FFu << 8)
#define SQI_CLKCON_CLKDIV(v)     (((v) & 0x7FFu) << 8)

/* =========================================================================
 * SQI_CMDTHR Bits (offset 0x114, R/W 32)
 * =========================================================================
 */

#define SQI_CMDTHR_RXCMDTHR_SHIFT  0
#define SQI_CMDTHR_RXCMDTHR_MASK   (0xFFu << 0)
#define SQI_CMDTHR_RXCMDTHR(v)     (((v) & 0xFFu) << 0)
#define SQI_CMDTHR_TXCMDTHR_SHIFT  8
#define SQI_CMDTHR_TXCMDTHR_MASK   (0xFFu << 8)
#define SQI_CMDTHR_TXCMDTHR(v)     (((v) & 0xFFu) << 8)

/* =========================================================================
 * SQI_INTTHR Bits (offset 0x118, R/W 32)
 * =========================================================================
 */

#define SQI_INTTHR_RXINTTHR_SHIFT  0
#define SQI_INTTHR_RXINTTHR_MASK   (0xFFu << 0)
#define SQI_INTTHR_RXINTTHR(v)     (((v) & 0xFFu) << 0)
#define SQI_INTTHR_TXINTTHR_SHIFT  8
#define SQI_INTTHR_TXINTTHR_MASK   (0xFFu << 8)
#define SQI_INTTHR_TXINTTHR(v)     (((v) & 0xFFu) << 8)

/* =========================================================================
 * SQI_INTEN / SQI_INTSTAT Bits (offsets 0x11C / 0x120, R/W 32)
 * SQI_INTEN enables interrupt sources; SQI_INTSTAT reflects current flags.
 * =========================================================================
 */

#define SQI_INT_TXEMPTY      (1u << 0)   /* TX FIFO empty */
#define SQI_INT_TXFULL       (1u << 1)   /* TX FIFO full */
#define SQI_INT_TXTHR        (1u << 2)   /* TX FIFO at/below threshold */
#define SQI_INT_RXEMPTY      (1u << 3)   /* RX FIFO empty */
#define SQI_INT_RXFULL       (1u << 4)   /* RX FIFO full */
#define SQI_INT_RXTHR        (1u << 5)   /* RX FIFO at/above threshold */
#define SQI_INT_CONFULL      (1u << 6)   /* Control buffer full */
#define SQI_INT_CONEMPTY     (1u << 7)   /* Control buffer empty */
#define SQI_INT_CONTHR       (1u << 8)   /* Control buffer threshold */
#define SQI_INT_BDDONE       (1u << 9)   /* BD transfer done */
#define SQI_INT_PKTCOMP      (1u << 10)  /* Packet complete */
#define SQI_INT_DMAE         (1u << 11)  /* DMA error */

/* Aliases matching Harmony plib naming */
#define SQI_INTEN_BDDONEIE    SQI_INT_BDDONE
#define SQI_INTEN_PKTCOMPIE   SQI_INT_PKTCOMP
#define SQI_INTSTAT_BDDONEIF  SQI_INT_BDDONE
#define SQI_INTSTAT_PKTCOMPIF SQI_INT_PKTCOMP

/* =========================================================================
 * SQI_STAT1 Bits (offset 0x12C, R 32)
 * =========================================================================
 */

/* RXBUFCNT [15:0] — bytes available in RX FIFO */
#define SQI_STAT1_RXBUFCNT_SHIFT  0
#define SQI_STAT1_RXBUFCNT_MASK   (0xFFFFu << 0)

/* TXBUFFREE [31:16] — free bytes in TX FIFO */
#define SQI_STAT1_TXBUFFREE_SHIFT 16
#define SQI_STAT1_TXBUFFREE_MASK  (0xFFFFu << 16)

/* =========================================================================
 * SQI_STAT2 Bits (offset 0x130, R 32)
 * =========================================================================
 */

#define SQI_STAT2_TXOV           (1u << 0)  /* TX overflow */
#define SQI_STAT2_RXUN           (1u << 1)  /* RX underflow */
#define SQI_STAT2_SQID0          (1u << 3)  /* SQI data line 0 state */
#define SQI_STAT2_SQID1          (1u << 4)  /* SQI data line 1 state */
#define SQI_STAT2_SQID2          (1u << 5)  /* SQI data line 2 state */
#define SQI_STAT2_SQID3          (1u << 6)  /* SQI data line 3 state */

/* CMDSTAT [17:16] */
#define SQI_STAT2_CMDSTAT_SHIFT  16
#define SQI_STAT2_CMDSTAT_MASK   (0x3u << 16)

/* =========================================================================
 * SQI_BDCON Bits (offset 0x134, R/W 32)
 * =========================================================================
 */

#define SQI_BDCON_DMAEN      (1u << 0)  /* DMA Enable */
#define SQI_BDCON_POLLEN     (1u << 1)  /* BD Poll Enable */
#define SQI_BDCON_START      (1u << 2)  /* Start DMA transfer */

/* =========================================================================
 * SQI_BDSTAT Bits (offset 0x144, R 32)
 * =========================================================================
 */

#define SQI_BDSTAT_DMAACTV   (1u << 16)  /* DMA Active */
#define SQI_BDSTAT_DMASTART  (1u << 17)  /* DMA Started */

/* BDSTATE [21:18] */
#define SQI_BDSTAT_BDSTATE_SHIFT  18
#define SQI_BDSTAT_BDSTATE_MASK   (0xFu << 18)

/* =========================================================================
 * SQI_INTSIGEN Bits (offset 0x158, R/W 32)
 * =========================================================================
 */

#define SQI_INTSIGEN_TXEMPTYISE  (1u << 0)
#define SQI_INTSIGEN_TXFULLISE   (1u << 1)
#define SQI_INTSIGEN_TXTHRISE    (1u << 2)
#define SQI_INTSIGEN_RXEMPTYISE  (1u << 3)
#define SQI_INTSIGEN_RXFULLISE   (1u << 4)
#define SQI_INTSIGEN_RXTHRISE    (1u << 5)
#define SQI_INTSIGEN_CONFULLISE  (1u << 6)
#define SQI_INTSIGEN_CONEMPTYISE (1u << 7)
#define SQI_INTSIGEN_CONTHRISE   (1u << 8)
#define SQI_INTSIGEN_BDDONEISE   (1u << 9)
#define SQI_INTSIGEN_PKTCOMPISE  (1u << 10)
#define SQI_INTSIGEN_DMAEISE     (1u << 11)

/* =========================================================================
 * SQI_MEMSTAT Bits (offset 0x160, R/W 32)
 * Status register check configuration (used by Harmony for SST26 auto-poll).
 * =========================================================================
 */

/* STATCMD [15:0] — command byte(s) to issue for status check */
#define SQI_MEMSTAT_STATCMD_SHIFT   0
#define SQI_MEMSTAT_STATCMD_MASK    (0xFFFFu << 0)
#define SQI_MEMSTAT_STATCMD(v)      (((v) & 0xFFFFu) << 0)

/* STATBYTES [17:16] — number of status bytes to read */
#define SQI_MEMSTAT_STATBYTES_SHIFT 16
#define SQI_MEMSTAT_STATBYTES_MASK  (0x3u << 16)
#define SQI_MEMSTAT_STATBYTES(v)    (((v) & 0x3u) << 16)

/* TYPESTAT [19:18] — status type */
#define SQI_MEMSTAT_TYPESTAT_SHIFT  18
#define SQI_MEMSTAT_TYPESTAT_MASK   (0x3u << 18)
#define SQI_MEMSTAT_TYPESTAT(v)     (((v) & 0x3u) << 18)

#define SQI_MEMSTAT_STATPOS         (1u << 20)  /* Status bit position */

/* =========================================================================
 * SQI THR (offset 0x154, R/W 32)
 * =========================================================================
 */

#define SQI_THR_THRES_SHIFT  0
#define SQI_THR_THRES_MASK   (0x7u << 0)
#define SQI_THR_THRES(v)     (((v) & 0x7u) << 0)

/* =========================================================================
 * BD Descriptor Control Word (bd_ctrl field in sqi_dma_desc_t)
 *
 * These bits reside in the bd_ctrl word of the in-memory BD descriptor —
 * NOT in a peripheral register. Verified from DFP component/sqi.h BDCTRL
 * bit-field comments.
 *
 * Typical TX last BD:  DESC_EN | CS_ASSERT | LAST_BD | LIFM | BUFLEN(n)
 * Typical RX last BD:  DESC_EN | CS_ASSERT | LAST_BD | LIFM | DIR | BUFLEN(n)
 * =========================================================================
 */

/* BUFLEN [8:0] — buffer length in bytes (max 512) */
#define SQI_BDCTRL_BUFLEN_SHIFT   0
#define SQI_BDCTRL_BUFLEN_MASK    (0x1FFu << 0)
#define SQI_BDCTRL_BUFLEN(n)      (((n) & 0x1FFu) << 0)

#define SQI_BDCTRL_CBD_INT_EN     (1u << 16)  /* BD-done interrupt enable */
#define SQI_BDCTRL_PKT_INT_EN     (1u << 17)  /* Packet-complete interrupt enable */
#define SQI_BDCTRL_LIFM           (1u << 18)  /* Last in frame — deassert CS at end */
#define SQI_BDCTRL_LAST_BD        (1u << 19)  /* Last BD in chain */
#define SQI_BDCTRL_DIR            (1u << 20)  /* Direction: 0=TX, 1=RX */
#define SQI_BDCTRL_SDR_DDR        (1u << 21)  /* DDR mode (0=SDR) */

/* MODE [23:22] — transfer lane mode for this BD */
#define SQI_BDCTRL_MODE_SHIFT     22
#define SQI_BDCTRL_MODE_MASK      (0x3u << 22)
#define SQI_BDCTRL_MODE(v)        (((v) & 0x3u) << 22)

#define SQI_BDCTRL_SPI_DEV_SEL2   (1u << 24)  /* Device select bit 2 */
#define SQI_BDCTRL_LSBF           (1u << 25)  /* LSB first for this BD */
#define SQI_BDCTRL_STAT_CHECK     (1u << 27)  /* Status check after this BD */

/* SPI_DEV_SEL [29:28] — device select bits [1:0] */
#define SQI_BDCTRL_SPI_DEV_SEL_SHIFT 28
#define SQI_BDCTRL_SPI_DEV_SEL_MASK  (0x3u << 28)
#define SQI_BDCTRL_SPI_DEV_SEL(v)    (((v) & 0x3u) << 28)

#define SQI_BDCTRL_CS_ASSERT      (1u << 30)  /* DEASSERT CS (drive HIGH) for this BD.
                                                 * DFP name is misleading — 1=CS HIGH, 0=CS LOW.
                                                 * Confirmed by PORT IN readback during active DMA. */
#define SQI_BDCTRL_DESC_EN        (1u << 31)  /* Descriptor enabled */

/* =========================================================================
 * BD Descriptor Structure
 *
 * Must be placed in the nocache MPU region (0x200F0000) to avoid D-cache
 * coherency issues. Total size: 32 bytes (includes 16-byte alignment pad).
 *
 * Cache rules:
 *  - TX: DCACHE_CLEAN_BY_ADDR(desc) + DCACHE_CLEAN_BY_ADDR(buf) before start
 *  - RX: DCACHE_INVALIDATE_BY_ADDR(buf) after ISR completion
 * =========================================================================
 */

struct sqi_dma_desc_s
{
  uint32_t              bd_ctrl;     /* BD control word (SQI_BDCTRL_* bits) */
  uint32_t              bd_stat;     /* BD status (written by hardware) */
  uint32_t              bd_bufaddr;  /* Physical address of data buffer (uint32_t per Harmony) */
  struct sqi_dma_desc_s *bd_nxtptr;  /* Pointer to next BD (NULL = end) */
  uint8_t               _pad[16];   /* Pad to 32 bytes for cache alignment */
};

typedef struct sqi_dma_desc_s sqi_dma_desc_t;

/* =========================================================================
 * Convenience: Harmony-compatible mode values passed to sam_sqibus_initialize
 * =========================================================================
 */

#define SQI_BUS_MODE_SPI   1   /* Single SPI (SST26 command phase) */
#define SQI_BUS_MODE_QUAD  2   /* Quad SPI (SST26 data phase after QPIEN) */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SQI_H */
