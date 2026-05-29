/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/pic32czca90_pinmap.h
 *
 * PIC32CZ CA90 Curiosity Ultra (EV16W43A) pin assignments — DS70005522C
 *
 * Console UART: SERCOM1, PC04 (PAD0 TX) / PC07 (PAD3 RX), function D
 *   → PKoB4 VCP (J700).  Confirmed from Harmony usart_echo_blocking.
 *   PC21/PC22 are SERCOM4 EXT2 expansion pins — NOT the console.
 *
 * LEDs:  LED0=PB21 (active LOW), LED1=PB22 (active LOW) — DS70005522C §2-11
 * Buttons: SW0=PB24, SW1=PC23 (active LOW, pullup) — DS70005522C §2-11
 * Oscillator: Y300 = DSC6011JI2B-012.0000 = 12 MHz MEMS
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H

/* =========================================================================
 * Pin encoding for port_pinset_t (uint32_t)
 *
 *   Bits 31-28: Port (0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G)
 *   Bits 27-24: Peripheral function (A=0 .. N=13)
 *   Bits 20-16: Pin number (0-31)
 *   Bits 7-0:   Configuration flags
 * =========================================================================
 */

#define PORT_PORTA          (0u << 28)
#define PORT_PORTB          (1u << 28)
#define PORT_PORTC          (2u << 28)
#define PORT_PORTD          (3u << 28)
#define PORT_PORTE          (4u << 28)
#define PORT_PORTF          (5u << 28)
#define PORT_PORTG          (6u << 28)

#define PORT_FUNC_SHIFT     24
#define PORT_FUNC(n)        ((uint32_t)(n) << PORT_FUNC_SHIFT)

#define PORT_PIN_SHIFT      16
#define PORT_PIN(n)         ((uint32_t)(n) << PORT_PIN_SHIFT)

/* Configuration flags */
#define PORT_FLAG_PMUXEN       (1 << 0)
#define PORT_FLAG_INEN         (1 << 1)
#define PORT_FLAG_PULLEN       (1 << 2)
#define PORT_FLAG_OUTPUT       (1 << 3)
#define PORT_FLAG_DRVSTR       (1 << 4)
#define PORT_FLAG_OUTVAL_HIGH  (1 << 5)  /* preload OUT=1 before enabling output */

/* =========================================================================
 * SERCOM1 – Console UART (PKoB4 VCP on J700)
 *
 * GROUND TRUTH: Harmony usart_echo_blocking example (confirmed working on
 * CA90 board) uses SERCOM1 with PC04 (TX, PAD0) and PC07 (RX, PAD3),
 * peripheral function D (value 3). TXPO=0, RXPO=3.
 *
 * DS70005522C signal names:
 *   PC04 → SERCOM1 PAD0 → PKoB4 APP_VCP_TX (board TX to host RX)
 *   PC07 → SERCOM1 PAD3 → PKoB4 APP_VCP_RX (board RX from host TX)
 * =========================================================================
 */

#define PORT_SERCOM1_PAD0   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(4) | \
                             PORT_FLAG_PMUXEN)                /* PC04 TX */
#define PORT_SERCOM1_PAD3   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(7) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN) /* PC07 RX */

/* =========================================================================
 * SERCOM4 – EXT2 expansion connector (NOT PKOB4 VCP)
 *
 * PC21 (PAD0) and PC22 (PAD1) are EXT2 header pins. The PKOB4 VCP is
 * on SERCOM1 (PC04/PC07) — see above.
 * =========================================================================
 */

#define PORT_SERCOM4_PAD0   (PORT_PORTC | PORT_FUNC(4) | PORT_PIN(21) | \
                             PORT_FLAG_PMUXEN)                /* PC21 EXT2 */
#define PORT_SERCOM4_PAD1   (PORT_PORTC | PORT_FUNC(4) | PORT_PIN(22) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN) /* PC22 EXT2 */

/* =========================================================================
 * SERCOM0 – PA04(TX)/PA05(RX), function D
 * =========================================================================
 */

#define PORT_SERCOM0_PAD0   (PORT_PORTA | PORT_FUNC(3) | PORT_PIN(4) | \
                             PORT_FLAG_PMUXEN)
#define PORT_SERCOM0_PAD1   (PORT_PORTA | PORT_FUNC(3) | PORT_PIN(5) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* =========================================================================
 * SERCOM2 – PC08(TX)/PC09(RX), function E (EXT1 SPI pins)
 * DS70005522C Table 2-4 EXT1: SPI MOSI=PC08, SCK=PC09
 * =========================================================================
 */

#define PORT_SERCOM2_PAD0   (PORT_PORTC | PORT_FUNC(4) | PORT_PIN(8) | \
                             PORT_FLAG_PMUXEN)
#define PORT_SERCOM2_PAD1   (PORT_PORTC | PORT_FUNC(4) | PORT_PIN(9) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* =========================================================================
 * SERCOM3 – PC12(TX)/PC13(RX), function E (EXT2 / MikroBUS SPI)
 * DS70005522C Table 2-3 MikroBUS: MOSI=PC12, SCK=PC13
 * =========================================================================
 */

#define PORT_SERCOM3_PAD0   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(12) | \
                             PORT_FLAG_PMUXEN)                          /* PC12 MOSI (func D=3, DFP-verified) */
#define PORT_SERCOM3_PAD1   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(13) | \
                             PORT_FLAG_PMUXEN)                          /* PC13 SCK (func D=3, DFP-verified) */
#define PORT_SERCOM3_PAD3   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(15) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)        /* PC15 MISO (func D=3, DFP-verified) */

/* SERCOM5 I2C (EXT2 header) — PC25=SDA(PAD0), PC26=SCL(PAD1), mux D=3
 * Harmony plib_port.c: PINCFG=0x01 (PMUXEN only, NO INEN). */
#define PORT_SERCOM5_PAD0   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(25) | \
                             PORT_FLAG_PMUXEN)                         /* PC25 SDA */
#define PORT_SERCOM5_PAD1   (PORT_PORTC | PORT_FUNC(3) | PORT_PIN(26) | \
                             PORT_FLAG_PMUXEN)                         /* PC26 SCL */

/* =========================================================================
 * SERCOM8 SPI — PD24(MOSI/PAD0), PD25(SCK/PAD1), PD26(SS/PAD2), PD27(MISO/PAD3)
 * All mux D=3, DFP-verified (PIC32CZ-CA90_DFP/1.7.168 pio/pic32cz8110ca90208.h)
 * APB C bridge (base 0x45002000)
 * =========================================================================
 */

#define PORT_SERCOM8_PAD0   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(24) | \
                             PORT_FLAG_PMUXEN)                          /* PD24 MOSI */
#define PORT_SERCOM8_PAD1   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(25) | \
                             PORT_FLAG_PMUXEN)                          /* PD25 SCK */
#define PORT_SERCOM8_PAD2   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(26) | \
                             PORT_FLAG_PMUXEN)                          /* PD26 SS */
#define PORT_SERCOM8_PAD3   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(27) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)        /* PD27 MISO */

/* =========================================================================
 * SERCOM9 SPI — PD28(MOSI/PAD0), PD29(SCK/PAD1), PE04(SS/PAD2), PE05(MISO/PAD3)
 * All mux D=3, DFP-verified
 * APB C bridge (base 0x45004000)
 * =========================================================================
 */

#define PORT_SERCOM9_PAD0   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(28) | \
                             PORT_FLAG_PMUXEN)                          /* PD28 MOSI */
#define PORT_SERCOM9_PAD1   (PORT_PORTD | PORT_FUNC(3) | PORT_PIN(29) | \
                             PORT_FLAG_PMUXEN)                          /* PD29 SCK */
#define PORT_SERCOM9_PAD2   (PORT_PORTE | PORT_FUNC(3) | PORT_PIN(4) | \
                             PORT_FLAG_PMUXEN)                          /* PE04 SS */
#define PORT_SERCOM9_PAD3   (PORT_PORTE | PORT_FUNC(3) | PORT_PIN(5) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)        /* PE05 MISO */

/* =========================================================================
 * On-board LEDs – DS70005522C Table 2-11
 * LED0: PB21 (active LOW, yellow) — initial state HIGH = LED off (Harmony: OUTSET then DIRSET)
 * LED1: PB22 (active LOW, yellow) — initial state HIGH = LED off
 * =========================================================================
 */

#define PORT_LED0           (PORT_PORTB | PORT_PIN(21) | PORT_FLAG_OUTPUT | PORT_FLAG_OUTVAL_HIGH)
#define PORT_LED1           (PORT_PORTB | PORT_PIN(22) | PORT_FLAG_OUTPUT | PORT_FLAG_OUTVAL_HIGH)

/* =========================================================================
 * On-board Buttons – DS70005522C Table 2-11
 * SW0: PB24 (active LOW, input with pullup)
 * SW1: PC23 (active LOW, input with pullup)
 * =========================================================================
 */

#define PORT_SW0            (PORT_PORTB | PORT_PIN(24) | \
                             PORT_FLAG_INEN | PORT_FLAG_PULLEN)
#define PORT_SW1            (PORT_PORTC | PORT_PIN(23) | \
                             PORT_FLAG_INEN | PORT_FLAG_PULLEN)

/* =========================================================================
 * SQI1 — SST26VF032BAT flash, CS0=PG03 (hardware-managed)
 *
 * Peripheral function H (index 7). Same physical pins as SDMMC1 (func I=8).
 * Mux for SQI1 first in board_app_initialize, then remux to SDMMC1 after
 * flash partitions are mounted.
 *   PC30 = SQI1_SCK   (output only, no INEN)
 *   PG03 = SQI1_CS0   (hardware-managed CS, INEN for status read)
 *   PC31 = SQI1_IO0   (MOSI in SPI mode, bidirectional)
 *   PG00 = SQI1_IO1   (MISO in SPI mode, bidirectional)
 *   PG01 = SQI1_IO2   (WP# in QSPI mode, bidirectional)
 *   PG02 = SQI1_IO3   (HOLD# in QSPI mode, bidirectional)
 *
 * IO2 (WP#) and IO3 (HOLD#) need pullups: in single-lane mode (DATAEN=0)
 * the SQI peripheral does not drive these pins (HOLD=0, WP=0 in CFG).
 * Without pullups IO3/HOLD# floats low — SST26 enters hold state and
 * ignores all SCK cycles.  PORT pullup (PULLEN + OUT=1) keeps them high
 * when the SQI peripheral is not actively driving.
 * =========================================================================
 */

#define PORT_SQI1_CLK   (PORT_PORTC | PORT_FUNC(7) | PORT_PIN(30) | \
                          PORT_FLAG_PMUXEN)
#define PORT_SQI1_CS0   (PORT_PORTG | PORT_FUNC(7) | PORT_PIN(3)  | \
                          PORT_FLAG_PMUXEN | PORT_FLAG_INEN | \
                          PORT_FLAG_PULLEN | PORT_FLAG_OUTVAL_HIGH)
#define PORT_SQI1_IO0   (PORT_PORTC | PORT_FUNC(7) | PORT_PIN(31) | \
                          PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SQI1_IO1   (PORT_PORTG | PORT_FUNC(7) | PORT_PIN(0)  | \
                          PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SQI1_IO2   (PORT_PORTG | PORT_FUNC(7) | PORT_PIN(1)  | \
                          PORT_FLAG_PMUXEN | PORT_FLAG_INEN | \
                          PORT_FLAG_PULLEN | PORT_FLAG_OUTVAL_HIGH)
#define PORT_SQI1_IO3   (PORT_PORTG | PORT_FUNC(7) | PORT_PIN(2)  | \
                          PORT_FLAG_PMUXEN | PORT_FLAG_INEN | \
                          PORT_FLAG_PULLEN | PORT_FLAG_OUTVAL_HIGH)

/* =========================================================================
 * SDMMC1 — micro-SD socket on Curiosity Ultra (EV16W43A)
 *
 * Peripheral function I (index 8) for SDMMC; function H (index 7) for SQI.
 * SDMMC1 and SQI1 share these physical pins — only one active at a time.
 *
 * DS70005522C schematic / CA90 DFP instance/sdmmc1.h:
 *   PC30 = SDMMC1_CLK  (output only, no INEN)
 *   PG03 = SDMMC1_CMD  (bidirectional)
 *   PC31 = SDMMC1_DAT0 (bidirectional)
 *   PG00 = SDMMC1_DAT1 (bidirectional)
 *   PG01 = SDMMC1_DAT2 (bidirectional)
 *   PG02 = SDMMC1_DAT3 (bidirectional)
 *   PC28 = SDMMC1_CD   (GPIO input, active LOW, pullup)
 * =========================================================================
 */

#define PORT_SDMMC1_CLK   (PORT_PORTC | PORT_FUNC(8) | PORT_PIN(30) | \
                           PORT_FLAG_PMUXEN)
#define PORT_SDMMC1_CMD   (PORT_PORTG | PORT_FUNC(8) | PORT_PIN(3)  | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SDMMC1_DAT0  (PORT_PORTC | PORT_FUNC(8) | PORT_PIN(31) | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SDMMC1_DAT1  (PORT_PORTG | PORT_FUNC(8) | PORT_PIN(0)  | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SDMMC1_DAT2  (PORT_PORTG | PORT_FUNC(8) | PORT_PIN(1)  | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SDMMC1_DAT3  (PORT_PORTG | PORT_FUNC(8) | PORT_PIN(2)  | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN)
#define PORT_SDMMC1_CD    (PORT_PORTC | PORT_FUNC(8) | PORT_PIN(28) | \
                           PORT_FLAG_PMUXEN | PORT_FLAG_INEN | PORT_FLAG_PULLEN | \
                           PORT_FLAG_OUTVAL_HIGH)
#define PIN_SDMMC1_CD     PORT_SDMMC1_CD  /* GPIO alias — used by sam_sdmmc.c */

/* =========================================================================
 * CAN3 – DS70005522C schematic, ATA6561 transceiver J701
 * PD13 = CAN3_TX (PMUX G = function 6)
 * PC29 = CAN3_RX (PMUX G = function 6)
 * =========================================================================
 */

#define PORT_CAN3_TX        (PORT_PORTD | PORT_FUNC(6) | PORT_PIN(13) | \
                             PORT_FLAG_PMUXEN)
#define PORT_CAN3_RX        (PORT_PORTC | PORT_FUNC(6) | PORT_PIN(29) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

/* =========================================================================
 * CAN4 – DS70005522C schematic, ATA6561 transceiver J702
 * PA31 = CAN4_TX (PMUX G = function 6)
 * PA30 = CAN4_RX (PMUX G = function 6)
 * =========================================================================
 */

#define PORT_CAN4_TX        (PORT_PORTA | PORT_FUNC(6) | PORT_PIN(31) | \
                             PORT_FLAG_PMUXEN)
#define PORT_CAN4_RX        (PORT_PORTA | PORT_FUNC(6) | PORT_PIN(30) | \
                             PORT_FLAG_PMUXEN | PORT_FLAG_INEN)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_PINMAP_H */
