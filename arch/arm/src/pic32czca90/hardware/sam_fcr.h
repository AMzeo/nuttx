/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_fcr.h
 *
 * PIC32CZ CA90 Flash Read Controller (FCR)
 * APB base: 0x44004000  (DFP pic32cz8110ca90208.h FCR_BASE_ADDRESS)
 *
 * Register offsets and bit fields verified from:
 *   PIC32CZ-CA90_DFP/1.7.168/CA90/include/component/fcr.h
 *   PIC32CZ-CA90_DFP/1.7.168/CA90/include/instance/fcr.h
 *
 * FCR controls flash read timing (wait states) and ECC.
 * Flash writes use FCW (sam_fcw.h), not FCR.
 *
 * Key action required at boot (before cache enable):
 *   Set FCR_CTRLA.AUTOWS=1 to let hardware manage wait states, OR
 *   set FCR_CTRLA.FWS to the correct value for the CPU clock:
 *     At 300 MHz, PFM access time = 25 ns → ceil(25 / 3.33) = 8 wait states.
 *   Harmony example uses AUTOWS=1 (field AUTOWS at bit 15).
 *
 * MCLK IDs (DFP instance/fcr.h):
 *   FCR_MCLK_ID_AHB = 4   → CLKMSK[0] bit 4
 *   FCR_MCLK_ID_APB = 5   → CLKMSK[0] bit 5
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCR_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCR_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets — DFP component/fcr.h verified
 * =========================================================================
 */

#define SAM_FCR_CTRLA_OFFSET        0x0000  /* Control A — wait states, arbitration (R/W 32) */
#define SAM_FCR_CTRLB_OFFSET        0x0004  /* Control B — power reduction mode (R/W 32) */
#define SAM_FCR_INTENCLR_OFFSET     0x0008  /* Interrupt Enable Clear (R/W 32) */
#define SAM_FCR_INTENSET_OFFSET     0x000C  /* Interrupt Enable Set (R/W 32) */
#define SAM_FCR_INTFLAG_OFFSET      0x0010  /* Interrupt Flag Register (R/W 32) */
#define SAM_FCR_STATUS_OFFSET       0x0014  /* NVM Status Register (R/ 32) */

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_FCR_CTRLA               (SAM_FCR_BASE + SAM_FCR_CTRLA_OFFSET)
#define SAM_FCR_CTRLB               (SAM_FCR_BASE + SAM_FCR_CTRLB_OFFSET)
#define SAM_FCR_INTENCLR            (SAM_FCR_BASE + SAM_FCR_INTENCLR_OFFSET)
#define SAM_FCR_INTENSET            (SAM_FCR_BASE + SAM_FCR_INTENSET_OFFSET)
#define SAM_FCR_INTFLAG             (SAM_FCR_BASE + SAM_FCR_INTFLAG_OFFSET)
#define SAM_FCR_STATUS              (SAM_FCR_BASE + SAM_FCR_STATUS_OFFSET)

/* =========================================================================
 * FCR_CTRLA — Control A Register (offset 0x00)
 *
 * FWS[11:8]:   Flash wait states (manual, ignored when AUTOWS=1)
 * AUTOWS[15]:  Automatic wait state management — set this at boot.
 *              Hardware adjusts wait states based on CPU clock.
 *              Recommended for all CA90 use cases.
 * RDBUFWS[19:16]: Read buffer wait states (matches FWS when AUTOWS=1)
 * =========================================================================
 */

#define FCR_CTRLA_FWS_SHIFT         8
#define FCR_CTRLA_FWS_MASK          (0xfu << FCR_CTRLA_FWS_SHIFT)
#define FCR_CTRLA_FWS(n)            ((uint32_t)(n) << FCR_CTRLA_FWS_SHIFT)
#define FCR_CTRLA_ADRWS             (1u << 14)  /* Address wait state enable */
#define FCR_CTRLA_AUTOWS            (1u << 15)  /* Automatic wait state (set at boot) */

/* =========================================================================
 * FCR_CTRLB — Control B Register (offset 0x04)
 * =========================================================================
 */

#define FCR_CTRLB_PRM               (1u << 0)  /* Power reduction mode */
#define FCR_CTRLB_TEMP              (1u << 1)  /* High-temperature read mode */

/* =========================================================================
 * FCR_INTFLAG — Interrupt Flag Register (offset 0x10)
 * ECC error flags — useful for detecting flash data corruption.
 * =========================================================================
 */

#define FCR_INTFLAG_SERR            (1u << 0)   /* ECC single-bit error (corrected) */
#define FCR_INTFLAG_DERR            (1u << 1)   /* ECC double-bit error (uncorrectable) */
#define FCR_INTFLAG_CRCDONE         (1u << 8)   /* CRC calculation complete */
#define FCR_INTFLAG_CRCERR          (1u << 9)   /* CRC mismatch */

/* =========================================================================
 * FCR_STATUS — NVM Status Register (offset 0x14, read-only)
 * =========================================================================
 */

#define FCR_STATUS_PRM              (1u << 0)  /* Power reduction mode active */
#define FCR_STATUS_TEMP             (1u << 1)  /* High-temperature mode active */

/* =========================================================================
 * MCLK IDs — enable clocks before any FCR register access
 * (DFP instance/fcr.h FCR_MCLK_ID_AHB / FCR_MCLK_ID_APB)
 * Formula: CLKMSK[id/32] |= (1 << (id % 32))
 * =========================================================================
 */

#define MCLK_ID_AHB_FCR             4u   /* CLKMSK[0] bit 4 */
#define MCLK_ID_APB_FCR             5u   /* CLKMSK[0] bit 5 */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCR_H */
