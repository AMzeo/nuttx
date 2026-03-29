/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/pic32czca90_pinmap.h
 *
 * PIC32CZ CA90 pin multiplexing definitions
 * Pin assignments for Curiosity Ultra board (EV16W43A)
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H

/* Pin encoding for port_pinset_t (uint32_t):
 *
 *   Bits 31-28: Port (0=A, 1=B, 2=C, 3=D)
 *   Bits 27-24: Peripheral function (A=0..N=13)
 *   Bits 20-16: Pin number (0-31)
 *   Bits 7-0:   Configuration flags
 */

#define PORT_PORTA                  (0u << 28)
#define PORT_PORTB                  (1u << 28)
#define PORT_PORTC                  (2u << 28)
#define PORT_PORTD                  (3u << 28)

#define PORT_FUNC_SHIFT             24
#define PORT_FUNC(n)                ((uint32_t)(n) << PORT_FUNC_SHIFT)

#define PORT_PIN_SHIFT              16
#define PORT_PIN(n)                 ((uint32_t)(n) << PORT_PIN_SHIFT)

/* Configuration flags */

#define PORT_FLAG_PMUXEN            (1 << 0)
#define PORT_FLAG_INEN              (1 << 1)
#define PORT_FLAG_PULLEN            (1 << 2)
#define PORT_FLAG_OUTPUT            (1 << 3)
#define PORT_FLAG_DRVSTR            (1 << 4)

/* SERCOM4 PAD0 (TX) = PB08, function D
 * SERCOM4 PAD1 (RX) = PB09, function D
 * These are the debug UART pins on CA90 Curiosity Ultra
 */

#define PORT_SERCOM4_PAD0  (PORT_PORTB | PORT_FUNC(3) | PORT_PIN(8) | \
                            PORT_FLAG_PMUXEN)
#define PORT_SERCOM4_PAD1  (PORT_PORTB | PORT_FUNC(3) | PORT_PIN(9) | \
                            PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* SERCOM0 on PA04/PA05 function D */

#define PORT_SERCOM0_PAD0  (PORT_PORTA | PORT_FUNC(3) | PORT_PIN(4) | \
                            PORT_FLAG_PMUXEN)
#define PORT_SERCOM0_PAD1  (PORT_PORTA | PORT_FUNC(3) | PORT_PIN(5) | \
                            PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* SERCOM1 on PA16/PA17 function C */

#define PORT_SERCOM1_PAD0  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(16) | \
                            PORT_FLAG_PMUXEN)
#define PORT_SERCOM1_PAD1  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(17) | \
                            PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* SERCOM2 on PA12/PA13 function C */

#define PORT_SERCOM2_PAD0  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(12) | \
                            PORT_FLAG_PMUXEN)
#define PORT_SERCOM2_PAD1  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(13) | \
                            PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* SERCOM3 on PA22/PA23 function C */

#define PORT_SERCOM3_PAD0  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(22) | \
                            PORT_FLAG_PMUXEN)
#define PORT_SERCOM3_PAD1  (PORT_PORTA | PORT_FUNC(2) | PORT_PIN(23) | \
                            PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* LED on PC21 */

#define PORT_LED0          (PORT_PORTC | PORT_PIN(21) | PORT_FLAG_OUTPUT)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H */
