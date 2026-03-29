/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_pm.h
 *
 * PIC32CZ CA90 Power Manager (PM) Register Definitions
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PM_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "hardware/sam_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* PM register offsets */

#define SAM_PM_CTRLA_OFFSET     0x0000
#define SAM_PM_SLEEPCFG_OFFSET  0x0001
#define SAM_PM_INTENCLR_OFFSET  0x0004
#define SAM_PM_INTENSET_OFFSET  0x0005
#define SAM_PM_INTFLAG_OFFSET   0x0006
#define SAM_PM_STDBYCFG_OFFSET  0x0008
#define SAM_PM_HIBCFG_OFFSET    0x0009
#define SAM_PM_BKUPCFG_OFFSET   0x000a
#define SAM_PM_PWSAKDLY_OFFSET  0x0012

/* PM register addresses */

#define SAM_PM_CTRLA            (SAM_PM_BASE + SAM_PM_CTRLA_OFFSET)
#define SAM_PM_SLEEPCFG         (SAM_PM_BASE + SAM_PM_SLEEPCFG_OFFSET)
#define SAM_PM_INTENCLR         (SAM_PM_BASE + SAM_PM_INTENCLR_OFFSET)
#define SAM_PM_INTENSET         (SAM_PM_BASE + SAM_PM_INTENSET_OFFSET)
#define SAM_PM_INTFLAG          (SAM_PM_BASE + SAM_PM_INTFLAG_OFFSET)
#define SAM_PM_STDBYCFG         (SAM_PM_BASE + SAM_PM_STDBYCFG_OFFSET)

/* SLEEPCFG register */

#define PM_SLEEPCFG_SLEEPMODE_SHIFT 0
#define PM_SLEEPCFG_SLEEPMODE_MASK  (0x7 << PM_SLEEPCFG_SLEEPMODE_SHIFT)
#  define PM_SLEEPCFG_IDLE          (0x2 << PM_SLEEPCFG_SLEEPMODE_SHIFT)
#  define PM_SLEEPCFG_STANDBY       (0x4 << PM_SLEEPCFG_SLEEPMODE_SHIFT)
#  define PM_SLEEPCFG_HIBERNATE     (0x5 << PM_SLEEPCFG_SLEEPMODE_SHIFT)
#  define PM_SLEEPCFG_BACKUP        (0x6 << PM_SLEEPCFG_SLEEPMODE_SHIFT)
#  define PM_SLEEPCFG_OFF           (0x7 << PM_SLEEPCFG_SLEEPMODE_SHIFT)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PM_H */
