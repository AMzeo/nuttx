/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_progmem.h
 *
 * Public interface for the PIC32CZ CA90 program flash MTD driver.
 * Implements the NuttX up_progmem_* API using the FCW (Flash Control Write)
 * controller.  Provides the crash-log (P9) storage partition.
 *
 * Call sam_progmem_initialize() once from board_app_initialize() before
 * any up_progmem_* or board_hardfault_init() call.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_PROGMEM_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_PROGMEM_H

#ifdef CONFIG_PIC32CZCA90_PROGMEM

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Enable FCW AHB + APB clocks.  Must be called before up_progmem_* or
 * board_hardfault_init().
 */

void sam_progmem_initialize(void);

#endif /* CONFIG_PIC32CZCA90_PROGMEM */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_PROGMEM_H */
