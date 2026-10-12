/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_wdt.h
 *
 * Public interface for the PIC32CZ CA90 Watchdog Timer driver.
 *
 * Call sam_wdt_initialize() once from board_app_initialize() to configure
 * the hardware and register the device as CONFIG_WATCHDOG_DEVPATH
 * ("/dev/watchdog0").  A separate task must then open that device and
 * drive WDIOC_START + periodic WDIOC_KEEPALIVE to keep the WDT fed.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_WDT_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_WDT_H

#ifdef CONFIG_PIC32CZCA90_WDT

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Configure WDT hardware (APB clock, CONFIG, EWCTRL, EW ISR) and register
 * the NuttX watchdog lower-half as CONFIG_WATCHDOG_DEVPATH.
 * Does NOT start the WDT — caller must open the device and send WDIOC_START.
 */

int sam_wdt_initialize(void);

#endif /* CONFIG_PIC32CZCA90_WDT */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_WDT_H */
