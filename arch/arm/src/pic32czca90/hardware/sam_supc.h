/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_supc.h
 *
 * PIC32CZ CA90 Supply Controller (SUPC)
 * Base: SAM_SUPC_BASE
 *
 * VREGCTRL is at offset 0x1C (not 0x18 — that is LVD).
 * VREGCTRL.AVREGEN[18:16] must be set to 4 before enabling PLL0.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets
 * =========================================================================
 */

#define SAM_SUPC_INTENCLR_OFFSET    0x0000
#define SAM_SUPC_INTENSET_OFFSET    0x0004
#define SAM_SUPC_INTFLAG_OFFSET     0x0008
#define SAM_SUPC_STATUS_OFFSET      0x000C
#define SAM_SUPC_BOD33_OFFSET       0x0010
/* 0x0014: reserved */
#define SAM_SUPC_LVD_OFFSET         0x0018  /* Low-Voltage Detect            */
#define SAM_SUPC_VREGCTRL_OFFSET    0x001C  /* Voltage Regulator Control     */

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_SUPC_INTENCLR           (SAM_SUPC_BASE + SAM_SUPC_INTENCLR_OFFSET)
#define SAM_SUPC_INTENSET           (SAM_SUPC_BASE + SAM_SUPC_INTENSET_OFFSET)
#define SAM_SUPC_INTFLAG            (SAM_SUPC_BASE + SAM_SUPC_INTFLAG_OFFSET)
#define SAM_SUPC_STATUS             (SAM_SUPC_BASE + SAM_SUPC_STATUS_OFFSET)
#define SAM_SUPC_BOD33              (SAM_SUPC_BASE + SAM_SUPC_BOD33_OFFSET)
#define SAM_SUPC_LVD                (SAM_SUPC_BASE + SAM_SUPC_LVD_OFFSET)
#define SAM_SUPC_VREGCTRL           (SAM_SUPC_BASE + SAM_SUPC_VREGCTRL_OFFSET)

/* =========================================================================
 * STATUS Register Bits (offset 0x000C, 32-bit)
 *
 * ADDVREGRDY[10:8]: indicates which additional voltage regulator is ready.
 * Wait for bit 10 when AVREGEN=4 is requested.
 * =========================================================================
 */

#define SUPC_STATUS_BOD33RDY        (1u << 0)
#define SUPC_STATUS_BOD33DET        (1u << 1)
#define SUPC_STATUS_ADDVREGRDY0     (1u << 8)   /* Additional reg 0 ready    */
#define SUPC_STATUS_ADDVREGRDY1     (1u << 9)   /* Additional reg 1 ready    */
#define SUPC_STATUS_ADDVREGRDY2     (1u << 10)  /* Additional reg 2 ready    */
#define SUPC_STATUS_ADDVREGRDY_ALL  (7u << 8)   /* All 3 regulators ready    */

/* =========================================================================
 * VREGCTRL Register Bits (offset 0x001C, 32-bit)
 *
 * AVREGEN[18:16]: Additional Voltage Regulator Enable
 *   Set AVREGEN=4 before enabling PLL0.
 *   This selects/enables the additional voltage regulator needed for PLL.
 * =========================================================================
 */

#define SUPC_VREGCTRL_AVREGEN_SHIFT  16
#define SUPC_VREGCTRL_AVREGEN_MASK   (0x7u << SUPC_VREGCTRL_AVREGEN_SHIFT)
#  define SUPC_VREGCTRL_AVREGEN(v)   ((uint32_t)(v) << SUPC_VREGCTRL_AVREGEN_SHIFT)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H */