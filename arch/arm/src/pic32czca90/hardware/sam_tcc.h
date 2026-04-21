/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_tcc.h
 *
 * PIC32CZ CA90 TCC (Timer/Counter for Control) register definitions.
 *
 * All values verified against PIC32CZ8110CA80208_DFP component/tcc.h and
 * instance/tcc0.h (DFP file date: 2024-07-31).
 *
 * Key instance parameters for TCC0:
 *   TCC0_GCLK_ID      = 31   (GCLK peripheral channel)
 *   TCC0_MCLK_ID_APB  = 41   (MCLK APB clock enable ID → CLKMSK[1] bit 9)
 *   TCC0_CC_NUM        = 8   (8 compare/capture channels)
 *   TCC0_SIZE          = 32  (32-bit counter)
 *   SAM_IRQ_TCC0MC0    = SAM_IRQ_EXTINT+127
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_TCC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_TCC_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets (from TCC base address)
 * All verified from DFP component/tcc.h
 * =========================================================================
 */

#define SAM_TCC_CTRLA_OFFSET       0x0000  /* Control A (R/W 32) */
#define SAM_TCC_CTRLBCLR_OFFSET    0x0004  /* Control B Clear (R/W 8) */
#define SAM_TCC_CTRLBSET_OFFSET    0x0005  /* Control B Set (R/W 8) */
#define SAM_TCC_SYNCBUSY_OFFSET    0x0008  /* Synchronization Busy (R 32) */
#define SAM_TCC_FCTRLA_OFFSET      0x000C  /* Fault Control A (R/W 32) */
#define SAM_TCC_FCTRLB_OFFSET      0x0010  /* Fault Control B (R/W 32) */
#define SAM_TCC_INTENCLR_OFFSET    0x0024  /* Interrupt Enable Clear (R/W 32) */
#define SAM_TCC_INTENSET_OFFSET    0x0028  /* Interrupt Enable Set (R/W 32) */
#define SAM_TCC_INTFLAG_OFFSET     0x002C  /* Interrupt Flag (R/W 32) */
#define SAM_TCC_STATUS_OFFSET      0x0030  /* Status (R/W 32) */
#define SAM_TCC_COUNT_OFFSET       0x0034  /* Count (R/W 32) */
#define SAM_TCC_WAVE_OFFSET        0x003C  /* Waveform Control (R/W 32) */
#define SAM_TCC_PER_OFFSET         0x0040  /* Period (R/W 32) */
#define SAM_TCC_CC_OFFSET(n)       (0x0044 + (n) * 4)  /* Compare/Capture n (R/W 32) */
#define SAM_TCC_PERBUF_OFFSET      0x0064  /* Period Buffer (R/W 32) */
#define SAM_TCC_CCBUF_OFFSET(n)    (0x0068 + (n) * 4)  /* CC Buffer n (R/W 32) */

/* =========================================================================
 * TCC0 Register Addresses
 * =========================================================================
 */

#define SAM_TCC0_CTRLA       (SAM_TCC0_BASE + SAM_TCC_CTRLA_OFFSET)
#define SAM_TCC0_CTRLBCLR    (SAM_TCC0_BASE + SAM_TCC_CTRLBCLR_OFFSET)
#define SAM_TCC0_CTRLBSET    (SAM_TCC0_BASE + SAM_TCC_CTRLBSET_OFFSET)
#define SAM_TCC0_SYNCBUSY    (SAM_TCC0_BASE + SAM_TCC_SYNCBUSY_OFFSET)
#define SAM_TCC0_INTENCLR    (SAM_TCC0_BASE + SAM_TCC_INTENCLR_OFFSET)
#define SAM_TCC0_INTENSET    (SAM_TCC0_BASE + SAM_TCC_INTENSET_OFFSET)
#define SAM_TCC0_INTFLAG     (SAM_TCC0_BASE + SAM_TCC_INTFLAG_OFFSET)
#define SAM_TCC0_STATUS      (SAM_TCC0_BASE + SAM_TCC_STATUS_OFFSET)
#define SAM_TCC0_COUNT       (SAM_TCC0_BASE + SAM_TCC_COUNT_OFFSET)
#define SAM_TCC0_WAVE        (SAM_TCC0_BASE + SAM_TCC_WAVE_OFFSET)
#define SAM_TCC0_PER         (SAM_TCC0_BASE + SAM_TCC_PER_OFFSET)
#define SAM_TCC0_CC(n)       (SAM_TCC0_BASE + SAM_TCC_CC_OFFSET(n))
#define SAM_TCC0_PERBUF      (SAM_TCC0_BASE + SAM_TCC_PERBUF_OFFSET)
#define SAM_TCC0_CCBUF(n)    (SAM_TCC0_BASE + SAM_TCC_CCBUF_OFFSET(n))

/* =========================================================================
 * TCC_CTRLA Bits (offset 0x00, R/W 32)
 * DFP: TCC_CTRLA_*
 * =========================================================================
 */

#define TCC_CTRLA_SWRST              (1u << 0)   /* Software Reset */
#define TCC_CTRLA_ENABLE             (1u << 1)   /* Enable */

/* RESOLUTION [6:5] — dithering; 0 = disabled (use for HRT) */
#define TCC_CTRLA_RESOLUTION_SHIFT   5
#define TCC_CTRLA_RESOLUTION_NONE    (0u << 5)

/* PRESCALER [10:8] — DFP-verified positions */
#define TCC_CTRLA_PRESCALER_SHIFT    8
#define TCC_CTRLA_PRESCALER_MSK      (7u << 8)
#define TCC_CTRLA_PRESCALER_DIV1     (0u << 8)   /* No division (for HRT) */
#define TCC_CTRLA_PRESCALER_DIV2     (1u << 8)
#define TCC_CTRLA_PRESCALER_DIV4     (2u << 8)
#define TCC_CTRLA_PRESCALER_DIV8     (3u << 8)
#define TCC_CTRLA_PRESCALER_DIV16    (4u << 8)
#define TCC_CTRLA_PRESCALER_DIV64    (5u << 8)
#define TCC_CTRLA_PRESCALER_DIV256   (6u << 8)
#define TCC_CTRLA_PRESCALER_DIV1024  (7u << 8)

/* PRESCSYNC [13:12] */
#define TCC_CTRLA_PRESCSYNC_GCLK     (0u << 12)
#define TCC_CTRLA_PRESCSYNC_PRESC    (1u << 12)
#define TCC_CTRLA_PRESCSYNC_RESYNC   (2u << 12)

/* =========================================================================
 * TCC_CTRLBSET / TCC_CTRLBCLR Bits (offset 0x05/0x04, R/W 8)
 * DFP: TCC_CTRLBSET_*
 * =========================================================================
 */

#define TCC_CTRLBSET_DIR             (1u << 0)   /* Counter Direction (1=DOWN) */
#define TCC_CTRLBSET_LUPD            (1u << 1)   /* Lock Update */
#define TCC_CTRLBSET_ONESHOT         (1u << 2)   /* One-Shot */

/* CMD [7:5] — command field */
#define TCC_CTRLBSET_CMD_SHIFT       5
#define TCC_CTRLBSET_CMD_MSK         (7u << 5)
#define TCC_CTRLBSET_CMD_NONE        (0u << 5)
#define TCC_CTRLBSET_CMD_RETRIGGER   (1u << 5)
#define TCC_CTRLBSET_CMD_STOP        (2u << 5)
#define TCC_CTRLBSET_CMD_UPDATE      (3u << 5)
#define TCC_CTRLBSET_CMD_READSYNC    (4u << 5)   /* Force COUNT read sync */

/* =========================================================================
 * TCC_SYNCBUSY Bits (offset 0x08, R 32)
 * DFP: TCC_SYNCBUSY_*
 * =========================================================================
 */

#define TCC_SYNCBUSY_SWRST           (1u << 0)
#define TCC_SYNCBUSY_ENABLE          (1u << 1)
#define TCC_SYNCBUSY_CTRLB           (1u << 2)
#define TCC_SYNCBUSY_STATUS          (1u << 3)
#define TCC_SYNCBUSY_COUNT           (1u << 4)
#define TCC_SYNCBUSY_PATT            (1u << 5)
#define TCC_SYNCBUSY_WAVE            (1u << 6)
#define TCC_SYNCBUSY_PER             (1u << 7)
#define TCC_SYNCBUSY_CC0             (1u << 8)
#define TCC_SYNCBUSY_CC1             (1u << 9)
#define TCC_SYNCBUSY_CC2             (1u << 10)
#define TCC_SYNCBUSY_CC3             (1u << 11)

/* =========================================================================
 * TCC_INTENCLR / TCC_INTENSET Bits (offsets 0x24/0x28, R/W 32)
 * DFP: TCC_INTENSET_* / TCC_INTENCLR_*
 * =========================================================================
 */

#define TCC_INTSET_OVF               (1u << 0)   /* Overflow */
#define TCC_INTSET_TRG               (1u << 1)   /* Retrigger */
#define TCC_INTSET_CNT               (1u << 2)   /* Counter */
#define TCC_INTSET_ERR               (1u << 3)   /* Error */
#define TCC_INTSET_MC0               (1u << 16)  /* Match/Capture Ch 0 (HRT deadline) */
#define TCC_INTSET_MC1               (1u << 17)
#define TCC_INTSET_MC2               (1u << 18)
#define TCC_INTSET_MC3               (1u << 19)

/* =========================================================================
 * TCC_INTFLAG Bits (offset 0x2C, R/W 32) — write 1 to clear
 * DFP: TCC_INTFLAG_*
 * =========================================================================
 */

#define TCC_INTFLAG_OVF              (1u << 0)
#define TCC_INTFLAG_TRG              (1u << 1)
#define TCC_INTFLAG_CNT              (1u << 2)
#define TCC_INTFLAG_ERR              (1u << 3)
#define TCC_INTFLAG_MC0              (1u << 16)  /* Match/Capture Ch 0 */
#define TCC_INTFLAG_MC1              (1u << 17)
#define TCC_INTFLAG_MC2              (1u << 18)
#define TCC_INTFLAG_MC3              (1u << 19)
#define TCC_INTFLAG_ALL              0x00FF0F0Fu  /* All flags */

/* =========================================================================
 * TCC_WAVE Bits (offset 0x3C, R/W 32)
 * DFP: TCC_WAVE_*
 * =========================================================================
 */

#define TCC_WAVE_WAVEGEN_SHIFT       0
#define TCC_WAVE_WAVEGEN_MSK         (7u << 0)
#define TCC_WAVE_WAVEGEN_NFRQ        (0u << 0)   /* Normal frequency (free-run) */
#define TCC_WAVE_WAVEGEN_MFRQ        (1u << 0)   /* Match frequency */
#define TCC_WAVE_WAVEGEN_NPWM        (2u << 0)   /* Normal PWM */

/* =========================================================================
 * GCLK and MCLK IDs for TCC0-9
 * Source: PIC32CZ8110CA80208_DFP instance/tcc*.h (verified 2024-07-31)
 *
 * Note: Each TCC has its OWN GCLK channel (unlike SAMD5x which shared).
 *       CAN channels are 46-51 (NOT adjacent to TCC).
 * =========================================================================
 */

/* GCLK peripheral channel IDs (for GCLK_PCHCTRL[n]) */

#define TCC0_GCLK_ID     31u   /* TCC0 — GCLK_PCHCTRL[31] */
#define TCC1_GCLK_ID     32u   /* TCC1 */
#define TCC2_GCLK_ID     33u   /* TCC2 */
#define TCC3_GCLK_ID     34u   /* TCC3 */
#define TCC4_GCLK_ID     35u   /* TCC4 */
#define TCC5_GCLK_ID     36u   /* TCC5 */
#define TCC6_GCLK_ID     37u   /* TCC6 */
#define TCC7_GCLK_ID     38u   /* TCC7 */
#define TCC8_GCLK_ID     39u   /* TCC8 */
#define TCC9_GCLK_ID     40u   /* TCC9 */

/* MCLK APB clock IDs (for MCLK_CLKMSK[id/32] bit [id%32]) */

#define MCLK_ID_APB_TCC0  41u  /* CLKMSK[1] bit  9 */
#define MCLK_ID_APB_TCC1  42u  /* CLKMSK[1] bit 10 */
#define MCLK_ID_APB_TCC2  43u  /* CLKMSK[1] bit 11 */
#define MCLK_ID_APB_TCC3  44u  /* CLKMSK[1] bit 12 */
#define MCLK_ID_APB_TCC4  45u  /* CLKMSK[1] bit 13 */
#define MCLK_ID_APB_TCC5  46u  /* CLKMSK[1] bit 14 */
#define MCLK_ID_APB_TCC6  47u  /* CLKMSK[1] bit 15 */
#define MCLK_ID_APB_TCC7  48u  /* CLKMSK[1] bit 16 */
#define MCLK_ID_APB_TCC8  49u  /* CLKMSK[1] bit 17 */
#define MCLK_ID_APB_TCC9  50u  /* CLKMSK[1] bit 18 */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_TCC_H */