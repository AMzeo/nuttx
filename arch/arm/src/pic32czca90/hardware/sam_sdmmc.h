/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_sdmmc.h
 *
 * PIC32CZ CA90 SDMMC (SD/MMC Host Controller) register definitions.
 *
 * SDMMC0 is used for the SD card (PC08-PC15, mux I=8, EXT1/EXT2 headers).
 * SDMMC1 pins (PC30/PG00-03) are dedicated to SQI1 flash.
 *
 * Transfer mode: ADMA2 (HC1R_DMASEL=2); no system DMA needed.
 * Base clock freq for CCR divider: 100 MHz from GCLK4 (PLL0/3).
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SDMMC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SDMMC_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * SDMMC0 Instance Parameters (DFP-verified: instance/sdmmc0.h)
 * =========================================================================
 */

#define SAM_SDMMC0_BASE          0x458A0000u
#define SAM_SDMMC0_GCLK_ID       58          /* GCLK4 → 100 MHz main clock */
#define SAM_SDMMC0_GCLK_ID_SLOW  59          /* GCLK5 → 12 MHz slow clock */
#define SAM_SDMMC0_MCLK_ID_AHB   69          /* MCLK AHB enable */
#define SAM_SDMMC0_MCLK_ID_APB   70          /* MCLK APB enable */

/* =========================================================================
 * SDMMC1 Instance Parameters (retained for reference; not used)
 * =========================================================================
 */

#define SAM_SDMMC1_BASE          0x460A0000u
#define SAM_SDMMC1_GCLK_ID       60
#define SAM_SDMMC1_GCLK_ID_SLOW  61
#define SAM_SDMMC1_MCLK_ID_AHB   71
#define SAM_SDMMC1_MCLK_ID_APB   72

/* Base clock frequency fed to SDMMC from GCLK4 (100 MHz).
 * Used by CCR divider calculation: CCR_SDCLKFSEL = base_freq / (2 * sdclk) */
#define SDMMC0_BASE_CLOCK_FREQUENCY  100000000u  /* 100 MHz */
#define SDMMC_CLOCK_FREQ_400_KHZ     400000u     /* identification phase */
#define SDMMC_CLOCK_FREQ_5_MHZ       5000000u    /* jumper-wire safe transfer speed */
#define SDMMC_CLOCK_FREQ_25_MHZ      25000000u   /* normal speed (PCB trace only) */

/* =========================================================================
 * Register Offsets (from SDMMC base address)
 * =========================================================================
 */

#define SAM_SDMMC_SSAR_OFFSET    0x0000  /* SDMA System Address / Argument 2 (R/W 32) */
#define SAM_SDMMC_BSR_OFFSET     0x0004  /* Block Size (R/W 16) */
#define SAM_SDMMC_BCR_OFFSET     0x0006  /* Block Count (R/W 16) */
#define SAM_SDMMC_ARG1R_OFFSET   0x0008  /* Argument 1 (R/W 32) */
#define SAM_SDMMC_TMR_OFFSET     0x000C  /* Transfer Mode (R/W 16) */
#define SAM_SDMMC_CR_OFFSET      0x000E  /* Command (R/W 16) */
#define SAM_SDMMC_RR0_OFFSET     0x0010  /* Response 0 (R 32) */
#define SAM_SDMMC_RR1_OFFSET     0x0014  /* Response 1 (R 32) */
#define SAM_SDMMC_RR2_OFFSET     0x0018  /* Response 2 (R 32) */
#define SAM_SDMMC_RR3_OFFSET     0x001C  /* Response 3 (R 32) */
#define SAM_SDMMC_BDPR_OFFSET    0x0020  /* Buffer Data Port (R/W 32) */
#define SAM_SDMMC_PSR_OFFSET     0x0024  /* Present State (R 32) */
#define SAM_SDMMC_HC1R_OFFSET    0x0028  /* Host Control 1 (R/W 8) */
#define SAM_SDMMC_PCR_OFFSET     0x0029  /* Power Control (R/W 8) */
#define SAM_SDMMC_BGCR_OFFSET    0x002A  /* Block Gap Control (R/W 8) */
#define SAM_SDMMC_WCR_OFFSET     0x002B  /* Wakeup Control (R/W 8) */
#define SAM_SDMMC_CCR_OFFSET     0x002C  /* Clock Control (R/W 16) */
#define SAM_SDMMC_TCR_OFFSET     0x002E  /* Timeout Control (R/W 8) */
#define SAM_SDMMC_SRR_OFFSET     0x002F  /* Software Reset (R/W 8) */
#define SAM_SDMMC_NISTR_OFFSET   0x0030  /* Normal Interrupt Status (R/W 16) */
#define SAM_SDMMC_EISTR_OFFSET   0x0032  /* Error Interrupt Status (R/W 16) */
#define SAM_SDMMC_NISTER_OFFSET  0x0034  /* Normal Interrupt Status Enable (R/W 16) */
#define SAM_SDMMC_EISTER_OFFSET  0x0036  /* Error Interrupt Status Enable (R/W 16) */
#define SAM_SDMMC_NISIER_OFFSET  0x0038  /* Normal Interrupt Signal Enable (R/W 16) */
#define SAM_SDMMC_EISIER_OFFSET  0x003A  /* Error Interrupt Signal Enable (R/W 16) */
#define SAM_SDMMC_ACER_OFFSET    0x003C  /* Auto CMD Error Status (R 16) */
#define SAM_SDMMC_HC2R_OFFSET    0x003E  /* Host Control 2 (R/W 16) */
#define SAM_SDMMC_CA0R_OFFSET    0x0040  /* Capabilities 0 (R 32) */
#define SAM_SDMMC_CA1R_OFFSET    0x0044  /* Capabilities 1 (R 32) */
#define SAM_SDMMC_MCCAR_OFFSET   0x0048  /* Max Current Capabilities (R 32) */
#define SAM_SDMMC_FERACES_OFFSET 0x0050  /* Force Event For Auto CMD Error Status (W 16) */
#define SAM_SDMMC_FERESR_OFFSET  0x0052  /* Force Event For Error Interrupt Status (W 16) */
#define SAM_SDMMC_AESR_OFFSET    0x0054  /* ADMA Error Status (R 8) */
#define SAM_SDMMC_ASAR_OFFSET    0x0058  /* ADMA System Address (R/W 32) */
#define SAM_SDMMC_SISR_OFFSET    0x00FC  /* Slot Interrupt Status (R 16) */
#define SAM_SDMMC_HCVR_OFFSET    0x00FE  /* Host Controller Version (R 16) */
#define SAM_SDMMC_DBGR_OFFSET    0x0234  /* Debug Register (R/W 32) — DFP DBGR_REG_OFST=0x234 */
#define SAM_SDMMC_DEBR_OFFSET    0x0207  /* Debounce Register (R/W 8) — CA90 extension */
#define SAM_SDMMC_CC2R_OFFSET    0x020C  /* Clock Control 2 Register (R/W 32) — CA90 extension */

/* =========================================================================
 * SDMMC1 Register Addresses
 * =========================================================================
 */

#define SAM_SDMMC1_SSAR    (SAM_SDMMC1_BASE + SAM_SDMMC_SSAR_OFFSET)
#define SAM_SDMMC1_BSR     (SAM_SDMMC1_BASE + SAM_SDMMC_BSR_OFFSET)
#define SAM_SDMMC1_BCR     (SAM_SDMMC1_BASE + SAM_SDMMC_BCR_OFFSET)
#define SAM_SDMMC1_ARG1R   (SAM_SDMMC1_BASE + SAM_SDMMC_ARG1R_OFFSET)
#define SAM_SDMMC1_TMR     (SAM_SDMMC1_BASE + SAM_SDMMC_TMR_OFFSET)
#define SAM_SDMMC1_CR      (SAM_SDMMC1_BASE + SAM_SDMMC_CR_OFFSET)
#define SAM_SDMMC1_RR(n)   (SAM_SDMMC1_BASE + SAM_SDMMC_RR0_OFFSET + (n) * 4)
#define SAM_SDMMC1_BDPR    (SAM_SDMMC1_BASE + SAM_SDMMC_BDPR_OFFSET)
#define SAM_SDMMC1_PSR     (SAM_SDMMC1_BASE + SAM_SDMMC_PSR_OFFSET)
#define SAM_SDMMC1_HC1R    (SAM_SDMMC1_BASE + SAM_SDMMC_HC1R_OFFSET)
#define SAM_SDMMC1_PCR     (SAM_SDMMC1_BASE + SAM_SDMMC_PCR_OFFSET)
#define SAM_SDMMC1_BGCR    (SAM_SDMMC1_BASE + SAM_SDMMC_BGCR_OFFSET)
#define SAM_SDMMC1_WCR     (SAM_SDMMC1_BASE + SAM_SDMMC_WCR_OFFSET)
#define SAM_SDMMC1_CCR     (SAM_SDMMC1_BASE + SAM_SDMMC_CCR_OFFSET)
#define SAM_SDMMC1_TCR     (SAM_SDMMC1_BASE + SAM_SDMMC_TCR_OFFSET)
#define SAM_SDMMC1_SRR     (SAM_SDMMC1_BASE + SAM_SDMMC_SRR_OFFSET)
#define SAM_SDMMC1_NISTR   (SAM_SDMMC1_BASE + SAM_SDMMC_NISTR_OFFSET)
#define SAM_SDMMC1_EISTR   (SAM_SDMMC1_BASE + SAM_SDMMC_EISTR_OFFSET)
#define SAM_SDMMC1_NISTER  (SAM_SDMMC1_BASE + SAM_SDMMC_NISTER_OFFSET)
#define SAM_SDMMC1_EISTER  (SAM_SDMMC1_BASE + SAM_SDMMC_EISTER_OFFSET)
#define SAM_SDMMC1_NISIER  (SAM_SDMMC1_BASE + SAM_SDMMC_NISIER_OFFSET)
#define SAM_SDMMC1_EISIER  (SAM_SDMMC1_BASE + SAM_SDMMC_EISIER_OFFSET)
#define SAM_SDMMC1_ACER    (SAM_SDMMC1_BASE + SAM_SDMMC_ACER_OFFSET)
#define SAM_SDMMC1_HC2R    (SAM_SDMMC1_BASE + SAM_SDMMC_HC2R_OFFSET)
#define SAM_SDMMC1_CA0R    (SAM_SDMMC1_BASE + SAM_SDMMC_CA0R_OFFSET)
#define SAM_SDMMC1_CA1R    (SAM_SDMMC1_BASE + SAM_SDMMC_CA1R_OFFSET)
#define SAM_SDMMC1_AESR    (SAM_SDMMC1_BASE + SAM_SDMMC_AESR_OFFSET)
#define SAM_SDMMC1_ASAR    (SAM_SDMMC1_BASE + SAM_SDMMC_ASAR_OFFSET)
#define SAM_SDMMC1_SISR    (SAM_SDMMC1_BASE + SAM_SDMMC_SISR_OFFSET)
#define SAM_SDMMC1_HCVR    (SAM_SDMMC1_BASE + SAM_SDMMC_HCVR_OFFSET)
#define SAM_SDMMC1_DBGR    (SAM_SDMMC1_BASE + SAM_SDMMC_DBGR_OFFSET)
#define SAM_SDMMC1_DEBR    (SAM_SDMMC1_BASE + SAM_SDMMC_DEBR_OFFSET)
#define SAM_SDMMC1_CC2R    (SAM_SDMMC1_BASE + SAM_SDMMC_CC2R_OFFSET)

/* =========================================================================
 * SDMMC_TMR (Transfer Mode, offset 0x0C, R/W 16) Bits
 * =========================================================================
 */

#define SDMMC_TMR_DMAEN          (1u << 0)   /* DMA Enable */
#define SDMMC_TMR_BCEN           (1u << 1)   /* Block Count Enable */

/* ACMDEN [3:2] — Auto Command Enable */
#define SDMMC_TMR_ACMDEN_SHIFT   2
#define SDMMC_TMR_ACMDEN_MASK    (0x3u << 2)
#define SDMMC_TMR_ACMDEN_DISABLED  (0u << 2)
#define SDMMC_TMR_ACMDEN_CMD12     (1u << 2)
#define SDMMC_TMR_ACMDEN_CMD23     (2u << 2)

#define SDMMC_TMR_DTDSEL         (1u << 4)   /* Data Transfer Direction: 1=read (card→host) */
#define SDMMC_TMR_MSBSEL         (1u << 5)   /* Multi/Single Block: 1=multi-block */

/* =========================================================================
 * SDMMC_CR (Command, offset 0x0E, R/W 16) Bits
 * =========================================================================
 */

/* RESPTYP [1:0] — Response Type */
#define SDMMC_CR_RESPTYP_SHIFT   0
#define SDMMC_CR_RESPTYP_NONE    (0u << 0)
#define SDMMC_CR_RESPTYP_136     (1u << 0)   /* 136-bit response */
#define SDMMC_CR_RESPTYP_48      (2u << 0)   /* 48-bit response */
#define SDMMC_CR_RESPTYP_48BUSY  (3u << 0)   /* 48-bit response with busy */

#define SDMMC_CR_CMDCCEN         (1u << 3)   /* Command CRC Check Enable */
#define SDMMC_CR_CMDICEN         (1u << 4)   /* Command Index Check Enable */
#define SDMMC_CR_DPSEL           (1u << 5)   /* Data Present Select */

/* CMDTYP [7:6] */
#define SDMMC_CR_CMDTYP_NORMAL   (0u << 6)
#define SDMMC_CR_CMDTYP_ABORT    (3u << 6)

/* CMDIDX [13:8] — Command Index */
#define SDMMC_CR_CMDIDX_SHIFT    8
#define SDMMC_CR_CMDIDX(idx)     (((idx) & 0x3Fu) << 8)

/* =========================================================================
 * SDMMC_PSR (Present State, offset 0x24, R 32) Bits
 * =========================================================================
 */

#define SDMMC_PSR_CMDINHC        (1u << 0)   /* Command Inhibit (CMD line busy) */
#define SDMMC_PSR_CMDINHD        (1u << 1)   /* Command Inhibit (DAT line busy) */
#define SDMMC_PSR_DLACT          (1u << 2)   /* DAT Line Active */
#define SDMMC_PSR_WTACT          (1u << 8)   /* Write Transfer Active */
#define SDMMC_PSR_RTACT          (1u << 9)   /* Read Transfer Active */
#define SDMMC_PSR_BUFWREN        (1u << 10)  /* Buffer Write Enable */
#define SDMMC_PSR_BUFRDEN        (1u << 11)  /* Buffer Read Enable */
#define SDMMC_PSR_CARDINS        (1u << 16)  /* Card Inserted */
#define SDMMC_PSR_CARDSS         (1u << 17)  /* Card State Stable */
#define SDMMC_PSR_CARDDPL        (1u << 18)  /* Card Detect Pin Level (1=card present) */

/* =========================================================================
 * SDMMC_HC1R (Host Control 1, offset 0x28, R/W 8) Bits
 * =========================================================================
 */

#define SDMMC_HC1R_DW_1BIT       (0u << 1)   /* 1-bit data bus */
#define SDMMC_HC1R_DW_4BIT       (1u << 1)   /* 4-bit data bus */
#define SDMMC_HC1R_HSEN          (1u << 2)   /* High Speed Enable */

/* DMASEL [4:3] — DMA Select */
#define SDMMC_HC1R_DMASEL_SHIFT  3
#define SDMMC_HC1R_DMASEL_SDMA   (0u << 3)   /* SDMA */
#define SDMMC_HC1R_DMASEL_ADMA2  (2u << 3)   /* 32-bit ADMA2 */

/* CARDDTL [6] — Card Detect Test Level (only when CARDDSEL=1) */
#define SDMMC_HC1R_CARDDTL       (1u << 6)   /* 1=card present in test mode */

/* CARDDSEL [7] — Card Detect Signal Selection */
#define SDMMC_HC1R_CARDDSEL      (1u << 7)   /* 0=use SDCD# pin, 1=use CARDDTL test bit */

/* =========================================================================
 * SDMMC_PCR (Power Control, offset 0x29, R/W 8) Bits
 * =========================================================================
 */

#define SDMMC_PCR_SDBPWR_ON      (1u << 0)   /* SD Bus Power On */
#define SDMMC_PCR_SDBPWR_OFF     (0u << 0)   /* SD Bus Power Off */

/* SDBVSEL [3:1] — SD Bus Voltage Select */
#define SDMMC_PCR_SDBVSEL_3V3    (7u << 1)   /* 3.3V */
#define SDMMC_PCR_SDBVSEL_3V0    (6u << 1)   /* 3.0V */
#define SDMMC_PCR_SDBVSEL_1V8    (5u << 1)   /* 1.8V */

/* =========================================================================
 * SDMMC_CA0R (Capabilities 0, offset 0x40, R 32) — Selected Fields
 * =========================================================================
 */

/* BASECLKF [15:8] — Base Clock Frequency for SD clock, in MHz.
 * Read by sam_set_clock() to compute CCR divider dynamically.
 * If 0 (BASECLKF_OTHER), fall back to SDMMC0_BASE_CLOCK_FREQUENCY (100 MHz). */
#define SDMMC_CA0R_BASECLKF_Pos    8u
#define SDMMC_CA0R_BASECLKF_Msk    (0xFFu << SDMMC_CA0R_BASECLKF_Pos)

/* =========================================================================
 * SDMMC_CA1R (Capabilities 1, offset 0x44, R 32) — Selected Fields
 * =========================================================================
 */

/* CLKMULT [23:16] — Clock Multiplier for programmable clock mode.
 * If CLKMULT > 0: F_MULTCLK = BASECLK × (CLKMULT + 1);
 *   div = F_MULTCLK / target_clk − 1; set CLKGSEL=1.
 * If CLKMULT == 0: use divided mode (CLKGSEL=0). */
#define SDMMC_CA1R_CLKMULT_Pos     16u
#define SDMMC_CA1R_CLKMULT_Msk     (0xFFu << SDMMC_CA1R_CLKMULT_Pos)

/* =========================================================================
 * SDMMC_CCR (Clock Control, offset 0x2C, R/W 16) Bits
 * =========================================================================
 */

#define SDMMC_CCR_INTCLKEN       (1u << 0)   /* Internal Clock Enable */
#define SDMMC_CCR_INTCLKS        (1u << 1)   /* Internal Clock Stable (RO) */
#define SDMMC_CCR_SDCLKEN        (1u << 2)   /* SD Clock Enable */
#define SDMMC_CCR_CLKGSEL        (1u << 5)   /* 1=Programmable clock generator */

/* USDCLKFSEL [7:6] — Upper 2 bits of SDCLK divider */
#define SDMMC_CCR_USDCLKFSEL_SHIFT  6
/* SDCLKFSEL [15:8] — Lower 8 bits of SDCLK divider (total 10-bit divider) */
#define SDMMC_CCR_SDCLKFSEL_SHIFT   8

/* 10-bit divider field encoding: lower 8 bits at [15:8], upper 2 bits at [7:6].
 * Same encoding for both divided mode (CLKGSEL=0) and programmable mode (CLKGSEL=1). */
#define SDMMC_CCR_SDCLKFSEL_DIV(div) \
  ((((div) & 0xFFu) << 8) | (((div) >> 8) & 0x3u) << 6)

/* =========================================================================
 * SDMMC_TCR (Timeout Control, offset 0x2E, R/W 8) Bits
 * =========================================================================
 */

/* DTCVAL [3:0] — timeout counter value; timeout = 2^(DTCVAL+13) / base_clk
 * DTCVAL=0xE → 2^27 cycles → ~1.34 s at 100 MHz */
#define SDMMC_TCR_DTCVAL_SHIFT  0
#define SDMMC_TCR_DTCVAL(v)     ((v) & 0xFu)
#define SDMMC_TCR_DTCVAL_MAX    0x0Eu

/* =========================================================================
 * SDMMC_SRR (Software Reset, offset 0x2F, R/W 8) Bits
 * =========================================================================
 */

#define SDMMC_SRR_SWRSTALL      (1u << 0)  /* Reset all */
#define SDMMC_SRR_SWRSTCMD      (1u << 1)  /* Reset CMD line */
#define SDMMC_SRR_SWRSTDAT      (1u << 2)  /* Reset DAT line */

/* =========================================================================
 * SDMMC_NISTR (Normal Interrupt Status, offset 0x30, R/W 16) Bits
 * SDMMC_NISTER (enable) / SDMMC_NISIER (signal) use the same bit positions.
 * =========================================================================
 */

#define SDMMC_NISTR_CMDC        (1u << 0)   /* Command Complete */
#define SDMMC_NISTR_TRFC        (1u << 1)   /* Transfer Complete */
#define SDMMC_NISTR_BLKGE       (1u << 2)   /* Block Gap Event */
#define SDMMC_NISTR_DMAINT      (1u << 3)   /* DMA Interrupt */
#define SDMMC_NISTR_BWRRDY      (1u << 4)   /* Buffer Write Ready */
#define SDMMC_NISTR_BRDRDY      (1u << 5)   /* Buffer Read Ready */
#define SDMMC_NISTR_CINS        (1u << 6)   /* Card Insertion */
#define SDMMC_NISTR_CREM        (1u << 7)   /* Card Removal */
#define SDMMC_NISTR_CINT        (1u << 8)   /* Card Interrupt */
#define SDMMC_NISTR_ERRINT      (1u << 15)  /* Error Interrupt (RO, reflects EISTR) */

/* =========================================================================
 * SDMMC_EISTR (Error Interrupt Status, offset 0x32, R/W 16) Bits
 * SDMMC_EISTER (enable) / SDMMC_EISIER (signal) use the same bit positions.
 * =========================================================================
 */

#define SDMMC_EISTR_CMDTEO      (1u << 0)   /* Command Timeout Error */
#define SDMMC_EISTR_CMDCRC      (1u << 1)   /* Command CRC Error */
#define SDMMC_EISTR_CMDEND      (1u << 2)   /* Command End Bit Error */
#define SDMMC_EISTR_CMDIDX      (1u << 3)   /* Command Index Error */
#define SDMMC_EISTR_DATTEO      (1u << 4)   /* Data Timeout Error */
#define SDMMC_EISTR_DATCRC      (1u << 5)   /* Data CRC Error */
#define SDMMC_EISTR_DATEND      (1u << 6)   /* Data End Bit Error */
#define SDMMC_EISTR_CURLIM      (1u << 7)   /* Current Limit Error */
#define SDMMC_EISTR_ACMD        (1u << 8)   /* Auto CMD Error */
#define SDMMC_EISTR_ADMA        (1u << 9)   /* ADMA Error */

/* All error bits — write to clear all */
#define SDMMC_EISTR_ALL         (0x03FFu)

/* =========================================================================
 * Enable masks used during init
 * =========================================================================
 */

/* Normal status enables: transfer complete, command complete, card ins/rem */
#define SDMMC_NISTER_INIT   (SDMMC_NISTR_CMDC | SDMMC_NISTR_TRFC | \
                             SDMMC_NISTR_DMAINT | SDMMC_NISTR_BWRRDY | \
                             SDMMC_NISTR_BRDRDY | SDMMC_NISTR_CINS | \
                             SDMMC_NISTR_CREM)
/* Error status enables: all errors */
#define SDMMC_EISTER_INIT   SDMMC_EISTR_ALL

/* Normal signal enables: transfer complete, command complete for ISR */
#define SDMMC_NISIER_INIT   (SDMMC_NISTR_CMDC | SDMMC_NISTR_TRFC | \
                             SDMMC_NISTR_DMAINT | SDMMC_NISTR_CINS | \
                             SDMMC_NISTR_CREM)
/* Error signal enables: all errors raise NVIC */
#define SDMMC_EISIER_INIT   SDMMC_EISTR_ALL

/* =========================================================================
 * ADMA2 Descriptor Structure (SD Host Controller Spec 2.0, §1.13)
 *
 * Single 8-byte descriptor covers one contiguous buffer (max 65535 bytes).
 * The descriptor table must be 4-byte aligned and D-cache clean before
 * writing ASAR.
 *   DCACHE_CLEAN_BY_ADDR(desc_table, sizeof(SDMMC_ADMA_DESCR))
 *
 * Attribute bits:
 *   bit 0 = VALID   — descriptor is valid
 *   bit 1 = END     — last descriptor in table
 *   bit 2 = INT     — interrupt after processing this descriptor
 *   bit 4-5 = ACT  — action: 00=NOP, 01=RSRV, 10=TRAN, 11=LINK
 *   TRAN=0x20 | VALID=0x01 | END=0x02 | INT=0x04 = 0x27
 * =========================================================================
 */

#define SDMMC_ADMA_ATTR_VALID   (1u << 0)
#define SDMMC_ADMA_ATTR_END     (1u << 1)
#define SDMMC_ADMA_ATTR_INT     (1u << 2)
#define SDMMC_ADMA_ATTR_ACT_NOP (0u << 4)
#define SDMMC_ADMA_ATTR_ACT_TRAN (2u << 4)   /* Transfer data to/from buffer */
#define SDMMC_ADMA_ATTR_ACT_LINK (3u << 4)   /* Link to another descriptor */

/* Combined attribute for a single-buffer transfer */
#define SDMMC_ADMA_XFER_LAST  (SDMMC_ADMA_ATTR_ACT_TRAN | SDMMC_ADMA_ATTR_VALID | \
                                SDMMC_ADMA_ATTR_INT | SDMMC_ADMA_ATTR_END)

/* =========================================================================
 * SDMMC_DEBR (Debounce Register, offset 0x207, R/W 8) — CA90 extension
 * Card detect debounce period counted in SLOW CLOCK cycles.
 * CDDVAL=3 (reset) = 328 slow clock cycles; at 12 MHz slow clock ≈ 27 µs.
 * If slow clock not running, debounce counter frozen → CARDSS stays 0.
 * =========================================================================
 */
#define SDMMC_DEBR_CDDVAL_SHIFT  0
#define SDMMC_DEBR_CDDVAL_MASK   (0x3u)
#define SDMMC_DEBR_CDDVAL_1      (0u)   /* 1 slow clock cycle */
#define SDMMC_DEBR_CDDVAL_8      (1u)   /* 8 slow clock cycles */
#define SDMMC_DEBR_CDDVAL_33     (2u)   /* 33 slow clock cycles */
#define SDMMC_DEBR_CDDVAL_328    (3u)   /* 328 slow clock cycles (reset) */

/* =========================================================================
 * SDMMC_CC2R (Clock Control 2, offset 0x20C, R/W 32) — CA90 extension
 * FSDCLKD: force SDCLK disabled even when SDCLKEN=1.
 * =========================================================================
 */
#define SDMMC_CC2R_FSDCLKD       (1u << 0)   /* Force SDCLK Disabled */

typedef struct
{
  uint16_t  attribute; /* Descriptor attributes (SDMMC_ADMA_ATTR_*) */
  uint16_t  length;    /* Buffer length in bytes (0 = 65536) */
  uint32_t  address;   /* Physical address of data buffer */
} sdmmc_adma_desc_t;

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SDMMC_H */
