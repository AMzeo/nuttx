/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_port.h
 *
 * PIC32CZ CA90 PORT (GPIO) register definitions
 * Same PORT IP block as SAMD5E5
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PORT_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PORT_H

#include "hardware/sam_memorymap.h"

/* PORT group register offsets (each group = 0x80 bytes) */

#define SAM_PORT_DIR_OFFSET         0x0000
#define SAM_PORT_DIRCLR_OFFSET      0x0004
#define SAM_PORT_DIRSET_OFFSET      0x0008
#define SAM_PORT_DIRTGL_OFFSET      0x000c
#define SAM_PORT_OUT_OFFSET         0x0010
#define SAM_PORT_OUTCLR_OFFSET      0x0014
#define SAM_PORT_OUTSET_OFFSET      0x0018
#define SAM_PORT_OUTTGL_OFFSET      0x001c
#define SAM_PORT_IN_OFFSET          0x0020
#define SAM_PORT_CTRL_OFFSET        0x0024
#define SAM_PORT_WRCONFIG_OFFSET    0x0028
#define SAM_PORT_EVCTRL_OFFSET      0x002c
#define SAM_PORT_PMUX_OFFSET(n)     (0x0030 + (n))
#define SAM_PORT_PINCFG_OFFSET(n)   (0x0040 + (n))

/* Port group base addresses */

#define SAM_PORTA_BASE              (SAM_PORT_BASE + 0x0000)
#define SAM_PORTB_BASE              (SAM_PORT_BASE + 0x0080)
#define SAM_PORTC_BASE              (SAM_PORT_BASE + 0x0100)
#define SAM_PORTD_BASE              (SAM_PORT_BASE + 0x0180)

/* PINCFG register */

#define PORT_PINCFG_PMUXEN          (1 << 0)
#define PORT_PINCFG_INEN            (1 << 1)
#define PORT_PINCFG_PULLEN          (1 << 2)
#define PORT_PINCFG_DRVSTR          (1 << 6)

/* PMUX register */

#define PORT_PMUX_PMUXE_SHIFT       0
#define PORT_PMUX_PMUXE_MASK        (0xf << PORT_PMUX_PMUXE_SHIFT)
#define PORT_PMUX_PMUXO_SHIFT       4
#define PORT_PMUX_PMUXO_MASK        (0xf << PORT_PMUX_PMUXO_SHIFT)

/* Peripheral function selections A-N */

#define PORT_PMUX_FUNC_A            0
#define PORT_PMUX_FUNC_B            1
#define PORT_PMUX_FUNC_C            2
#define PORT_PMUX_FUNC_D            3
#define PORT_PMUX_FUNC_E            4
#define PORT_PMUX_FUNC_F            5
#define PORT_PMUX_FUNC_G            6
#define PORT_PMUX_FUNC_H            7
#define PORT_PMUX_FUNC_I            8
#define PORT_PMUX_FUNC_J            9
#define PORT_PMUX_FUNC_K            10
#define PORT_PMUX_FUNC_L            11
#define PORT_PMUX_FUNC_M            12
#define PORT_PMUX_FUNC_N            13

/* WRCONFIG register */

#define PORT_WRCONFIG_PINMASK_SHIFT 0
#define PORT_WRCONFIG_PINMASK_MASK  (0xffff << PORT_WRCONFIG_PINMASK_SHIFT)
#define PORT_WRCONFIG_PMUXEN        (1 << 16)
#define PORT_WRCONFIG_INEN          (1 << 17)
#define PORT_WRCONFIG_PULLEN        (1 << 18)
#define PORT_WRCONFIG_DRVSTR        (1 << 22)
#define PORT_WRCONFIG_PMUX_SHIFT    24
#define PORT_WRCONFIG_PMUX_MASK     (0xf << PORT_WRCONFIG_PMUX_SHIFT)
#define PORT_WRCONFIG_WRPMUX        (1 << 28)
#define PORT_WRCONFIG_WRPINCFG      (1 << 30)
#define PORT_WRCONFIG_HWSEL         (1 << 31)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_PORT_H */
