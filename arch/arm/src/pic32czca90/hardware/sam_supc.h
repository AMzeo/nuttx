/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_supc.h
 *
 * PIC32CZ CA90 Supply Controller (SUPC) Register Definitions
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "hardware/sam_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SUPC register offsets */

#define SAM_SUPC_INTENCLR_OFFSET  0x0000
#define SAM_SUPC_INTENSET_OFFSET  0x0004
#define SAM_SUPC_INTFLAG_OFFSET   0x0008
#define SAM_SUPC_STATUS_OFFSET    0x000c
#define SAM_SUPC_BOD33_OFFSET     0x0010
#define SAM_SUPC_VREG_OFFSET      0x0018
#define SAM_SUPC_VREF_OFFSET      0x001c

/* SUPC register addresses */

#define SAM_SUPC_INTENCLR         (SAM_SUPC_BASE + SAM_SUPC_INTENCLR_OFFSET)
#define SAM_SUPC_INTENSET         (SAM_SUPC_BASE + SAM_SUPC_INTENSET_OFFSET)
#define SAM_SUPC_INTFLAG          (SAM_SUPC_BASE + SAM_SUPC_INTFLAG_OFFSET)
#define SAM_SUPC_STATUS           (SAM_SUPC_BASE + SAM_SUPC_STATUS_OFFSET)
#define SAM_SUPC_BOD33            (SAM_SUPC_BASE + SAM_SUPC_BOD33_OFFSET)
#define SAM_SUPC_VREG             (SAM_SUPC_BASE + SAM_SUPC_VREG_OFFSET)
#define SAM_SUPC_VREF             (SAM_SUPC_BASE + SAM_SUPC_VREF_OFFSET)

/* VREG register */

#define SUPC_VREG_ENABLE          (1 << 1)
#define SUPC_VREG_SEL             (1 << 2)  /* 0=LDO, 1=BUCK */

/* STATUS register */

#define SUPC_STATUS_BOD33RDY      (1 << 0)
#define SUPC_STATUS_BOD33DET      (1 << 1)
#define SUPC_STATUS_VREGRDY       (1 << 8)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SUPC_H */
