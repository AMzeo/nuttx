/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/chip.h
 *
 * PIC32CZ CA90 chip-specific definitions
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_CHIP_H
#define __ARCH_ARM_SRC_PIC32CZCA90_CHIP_H

#include <nuttx/config.h>
#include <arch/irq.h>
#include <arch/pic32czca90/chip.h>

/* Cortex-M7: number of peripheral interrupts = SAM_IRQ_NEXTINT */

#define ARMV7M_PERIPHERAL_INTERRUPTS  SAM_IRQ_NEXTINT

/* Cortex-M7 D-Cache line size: 32 bytes (8 words) */

#define ARMV7M_DCACHE_LINESIZE  32

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_CHIP_H */
