/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_nvmctrl.h
 *
 * PIC32CZ CA90 NVM Controller (NVMCTRL) Register Definitions
 *
 * The CA90 has 8 MB dual-panel flash at base address 0x0C000000.
 * At 300 MHz, typically 7+ wait states are needed.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_NVMCTRL_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_NVMCTRL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "hardware/sam_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* NVM Controller register offsets */

#define SAM_NVMCTRL_CTRLA_OFFSET   0x0000
#define SAM_NVMCTRL_CTRLB_OFFSET   0x0004
#define SAM_NVMCTRL_PARAM_OFFSET   0x0008
#define SAM_NVMCTRL_INTENCLR_OFFSET 0x000c
#define SAM_NVMCTRL_INTENSET_OFFSET 0x0010
#define SAM_NVMCTRL_INTFLAG_OFFSET  0x0014
#define SAM_NVMCTRL_STATUS_OFFSET   0x0018
#define SAM_NVMCTRL_ADDR_OFFSET     0x001c
#define SAM_NVMCTRL_RUNLOCK_OFFSET  0x0020
#define SAM_NVMCTRL_PBLDATA0_OFFSET 0x0028
#define SAM_NVMCTRL_PBLDATA1_OFFSET 0x002c

/* Register addresses */

#define SAM_NVMCTRL_CTRLA          (SAM_NVMCTRL_BASE + SAM_NVMCTRL_CTRLA_OFFSET)
#define SAM_NVMCTRL_CTRLB          (SAM_NVMCTRL_BASE + SAM_NVMCTRL_CTRLB_OFFSET)
#define SAM_NVMCTRL_PARAM          (SAM_NVMCTRL_BASE + SAM_NVMCTRL_PARAM_OFFSET)
#define SAM_NVMCTRL_INTFLAG        (SAM_NVMCTRL_BASE + SAM_NVMCTRL_INTFLAG_OFFSET)
#define SAM_NVMCTRL_STATUS         (SAM_NVMCTRL_BASE + SAM_NVMCTRL_STATUS_OFFSET)

/* CTRLA wait state bits */

#define NVMCTRL_CTRLA_RWS_SHIFT    8
#define NVMCTRL_CTRLA_RWS_MASK     (0xf << NVMCTRL_CTRLA_RWS_SHIFT)
#define NVMCTRL_CTRLA_RWS(n)       ((n) << NVMCTRL_CTRLA_RWS_SHIFT)
#define NVMCTRL_CTRLA_AUTOWS       (1 << 2)  /* Automatic wait state */

/* For CA90 at 300 MHz: typically need 7+ wait states */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_NVMCTRL_H */
