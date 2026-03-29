/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/include/pic32czca90/pic32czca90_irq.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/* PIC32CZ CA90 Peripheral Interrupt Definitions
 *
 * This file should never be included directly but, rather,
 * only indirectly through nuttx/irq.h
 */

#ifndef __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H
#define __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

/* External interrupts (vectors >= 16)
 *
 * PIC32CZ CA90 interrupt vector table.
 * Similar to SAME54 but with CA90-specific vector assignments.
 */

/* Clock / Power / Reset */

#define SAM_IRQ_PM             (SAM_IRQ_EXTINT + 0)    /* Power Manager */
#define SAM_IRQ_MCLK           (SAM_IRQ_EXTINT + 1)    /* Main Clock */
#define SAM_IRQ_XOSC0          (SAM_IRQ_EXTINT + 2)    /* XOSC0 */
#define SAM_IRQ_XOSC1          (SAM_IRQ_EXTINT + 3)    /* XOSC1 */
#define SAM_IRQ_DFLL           (SAM_IRQ_EXTINT + 4)    /* DFLL */
#define SAM_IRQ_DPLL0          (SAM_IRQ_EXTINT + 5)    /* DPLL0 */
#define SAM_IRQ_DPLL1          (SAM_IRQ_EXTINT + 6)    /* DPLL1 */
#define SAM_IRQ_OSC32K         (SAM_IRQ_EXTINT + 7)    /* OSC32KCTRL */
#define SAM_IRQ_SUPCRDY        (SAM_IRQ_EXTINT + 8)    /* SUPC Ready */
#define SAM_IRQ_SUPCDET        (SAM_IRQ_EXTINT + 9)    /* SUPC Detect */
#define SAM_IRQ_WDT            (SAM_IRQ_EXTINT + 10)   /* WDT */
#define SAM_IRQ_RTC            (SAM_IRQ_EXTINT + 11)   /* RTC */

/* External Interrupt Controller (EIC) */

#define SAM_IRQ_EXTINT0        (SAM_IRQ_EXTINT + 12)   /* EIC EXTINT0 */
#define SAM_IRQ_EXTINT1        (SAM_IRQ_EXTINT + 13)   /* EIC EXTINT1 */
#define SAM_IRQ_EXTINT2        (SAM_IRQ_EXTINT + 14)   /* EIC EXTINT2 */
#define SAM_IRQ_EXTINT3        (SAM_IRQ_EXTINT + 15)   /* EIC EXTINT3 */
#define SAM_IRQ_EXTINT4        (SAM_IRQ_EXTINT + 16)   /* EIC EXTINT4 */
#define SAM_IRQ_EXTINT5        (SAM_IRQ_EXTINT + 17)   /* EIC EXTINT5 */
#define SAM_IRQ_EXTINT6        (SAM_IRQ_EXTINT + 18)   /* EIC EXTINT6 */
#define SAM_IRQ_EXTINT7        (SAM_IRQ_EXTINT + 19)   /* EIC EXTINT7 */
#define SAM_IRQ_EXTINT8        (SAM_IRQ_EXTINT + 20)   /* EIC EXTINT8 */
#define SAM_IRQ_EXTINT9        (SAM_IRQ_EXTINT + 21)   /* EIC EXTINT9 */
#define SAM_IRQ_EXTINT10       (SAM_IRQ_EXTINT + 22)   /* EIC EXTINT10 */
#define SAM_IRQ_EXTINT11       (SAM_IRQ_EXTINT + 23)   /* EIC EXTINT11 */
#define SAM_IRQ_EXTINT12       (SAM_IRQ_EXTINT + 24)   /* EIC EXTINT12 */
#define SAM_IRQ_EXTINT13       (SAM_IRQ_EXTINT + 25)   /* EIC EXTINT13 */
#define SAM_IRQ_EXTINT14       (SAM_IRQ_EXTINT + 26)   /* EIC EXTINT14 */
#define SAM_IRQ_EXTINT15       (SAM_IRQ_EXTINT + 27)   /* EIC EXTINT15 */

/* System peripherals */

#define SAM_IRQ_FREQM          (SAM_IRQ_EXTINT + 28)   /* FREQM */
#define SAM_IRQ_NVMCTRL0       (SAM_IRQ_EXTINT + 29)   /* NVMCTRL 0 */
#define SAM_IRQ_NVMCTRL1       (SAM_IRQ_EXTINT + 30)   /* NVMCTRL 1 */

/* DMA Controller */

#define SAM_IRQ_DMACH0         (SAM_IRQ_EXTINT + 31)   /* DMA Channel 0 */
#define SAM_IRQ_DMACH1         (SAM_IRQ_EXTINT + 32)   /* DMA Channel 1 */
#define SAM_IRQ_DMACH2         (SAM_IRQ_EXTINT + 33)   /* DMA Channel 2 */
#define SAM_IRQ_DMACH3         (SAM_IRQ_EXTINT + 34)   /* DMA Channel 3 */
#define SAM_IRQ_DMACH4_31      (SAM_IRQ_EXTINT + 35)   /* DMA Channels 4-31 */

/* Event System */

#define SAM_IRQ_EVSYS0         (SAM_IRQ_EXTINT + 36)   /* EVSYS Channel 0 */
#define SAM_IRQ_EVSYS1         (SAM_IRQ_EXTINT + 37)   /* EVSYS Channel 1 */
#define SAM_IRQ_EVSYS2         (SAM_IRQ_EXTINT + 38)   /* EVSYS Channel 2 */
#define SAM_IRQ_EVSYS3         (SAM_IRQ_EXTINT + 39)   /* EVSYS Channel 3 */
#define SAM_IRQ_EVSYS4_11      (SAM_IRQ_EXTINT + 40)   /* EVSYS Channels 4-11 */

/* Peripheral Access Controller */

#define SAM_IRQ_PAC            (SAM_IRQ_EXTINT + 41)   /* PAC */

/* Vectors 42-44 reserved */

/* RAM ECC */

#define SAM_IRQ_RAMECC         (SAM_IRQ_EXTINT + 45)   /* RAM ECC */

/* SERCOM0 */

#define SAM_IRQ_SERCOM0_0      (SAM_IRQ_EXTINT + 46)   /* SERCOM0 INT0 */
#define SAM_IRQ_SERCOM0_1      (SAM_IRQ_EXTINT + 47)   /* SERCOM0 INT1 */
#define SAM_IRQ_SERCOM0_2      (SAM_IRQ_EXTINT + 48)   /* SERCOM0 INT2 */
#define SAM_IRQ_SERCOM0_46     (SAM_IRQ_EXTINT + 49)   /* SERCOM0 INT3-6 */

/* SERCOM1 */

#define SAM_IRQ_SERCOM1_0      (SAM_IRQ_EXTINT + 50)   /* SERCOM1 INT0 */
#define SAM_IRQ_SERCOM1_1      (SAM_IRQ_EXTINT + 51)   /* SERCOM1 INT1 */
#define SAM_IRQ_SERCOM1_2      (SAM_IRQ_EXTINT + 52)   /* SERCOM1 INT2 */
#define SAM_IRQ_SERCOM1_46     (SAM_IRQ_EXTINT + 53)   /* SERCOM1 INT3-6 */

/* SERCOM2 */

#define SAM_IRQ_SERCOM2_0      (SAM_IRQ_EXTINT + 54)   /* SERCOM2 INT0 */
#define SAM_IRQ_SERCOM2_1      (SAM_IRQ_EXTINT + 55)   /* SERCOM2 INT1 */
#define SAM_IRQ_SERCOM2_2      (SAM_IRQ_EXTINT + 56)   /* SERCOM2 INT2 */
#define SAM_IRQ_SERCOM2_46     (SAM_IRQ_EXTINT + 57)   /* SERCOM2 INT3-6 */

/* SERCOM3 */

#define SAM_IRQ_SERCOM3_0      (SAM_IRQ_EXTINT + 58)   /* SERCOM3 INT0 */
#define SAM_IRQ_SERCOM3_1      (SAM_IRQ_EXTINT + 59)   /* SERCOM3 INT1 */
#define SAM_IRQ_SERCOM3_2      (SAM_IRQ_EXTINT + 60)   /* SERCOM3 INT2 */
#define SAM_IRQ_SERCOM3_46     (SAM_IRQ_EXTINT + 61)   /* SERCOM3 INT3-6 */

/* SERCOM4 */

#define SAM_IRQ_SERCOM4_0      (SAM_IRQ_EXTINT + 62)   /* SERCOM4 INT0 */
#define SAM_IRQ_SERCOM4_1      (SAM_IRQ_EXTINT + 63)   /* SERCOM4 INT1 */
#define SAM_IRQ_SERCOM4_2      (SAM_IRQ_EXTINT + 64)   /* SERCOM4 INT2 */
#define SAM_IRQ_SERCOM4_46     (SAM_IRQ_EXTINT + 65)   /* SERCOM4 INT3-6 */

/* SERCOM5 */

#define SAM_IRQ_SERCOM5_0      (SAM_IRQ_EXTINT + 66)   /* SERCOM5 INT0 */
#define SAM_IRQ_SERCOM5_1      (SAM_IRQ_EXTINT + 67)   /* SERCOM5 INT1 */
#define SAM_IRQ_SERCOM5_2      (SAM_IRQ_EXTINT + 68)   /* SERCOM5 INT2 */
#define SAM_IRQ_SERCOM5_46     (SAM_IRQ_EXTINT + 69)   /* SERCOM5 INT3-6 */

/* SERCOM6 */

#define SAM_IRQ_SERCOM6_0      (SAM_IRQ_EXTINT + 70)   /* SERCOM6 INT0 */
#define SAM_IRQ_SERCOM6_1      (SAM_IRQ_EXTINT + 71)   /* SERCOM6 INT1 */
#define SAM_IRQ_SERCOM6_2      (SAM_IRQ_EXTINT + 72)   /* SERCOM6 INT2 */
#define SAM_IRQ_SERCOM6_46     (SAM_IRQ_EXTINT + 73)   /* SERCOM6 INT3-6 */

/* SERCOM7 */

#define SAM_IRQ_SERCOM7_0      (SAM_IRQ_EXTINT + 74)   /* SERCOM7 INT0 */
#define SAM_IRQ_SERCOM7_1      (SAM_IRQ_EXTINT + 75)   /* SERCOM7 INT1 */
#define SAM_IRQ_SERCOM7_2      (SAM_IRQ_EXTINT + 76)   /* SERCOM7 INT2 */
#define SAM_IRQ_SERCOM7_46     (SAM_IRQ_EXTINT + 77)   /* SERCOM7 INT3-6 */

/* CAN-FD */

#define SAM_IRQ_CAN0           (SAM_IRQ_EXTINT + 78)   /* CAN0 */
#define SAM_IRQ_CAN1           (SAM_IRQ_EXTINT + 79)   /* CAN1 */

/* USB High-Speed */

#define SAM_IRQ_USB            (SAM_IRQ_EXTINT + 80)   /* USB */
#define SAM_IRQ_USBSOF         (SAM_IRQ_EXTINT + 81)   /* USB SOF */
#define SAM_IRQ_USBTRCPT0      (SAM_IRQ_EXTINT + 82)   /* USB TRCPT0 */
#define SAM_IRQ_USBTRCPT1      (SAM_IRQ_EXTINT + 83)   /* USB TRCPT1 */

/* Ethernet MAC */

#define SAM_IRQ_GMAC           (SAM_IRQ_EXTINT + 84)   /* GMAC */

/* Timer/Counter for Control (TCC0) - 6 match/capture channels */

#define SAM_IRQ_TCC0           (SAM_IRQ_EXTINT + 85)   /* TCC0 */
#define SAM_IRQ_TCC0MC0        (SAM_IRQ_EXTINT + 86)   /* TCC0 MC0 */
#define SAM_IRQ_TCC0MC1        (SAM_IRQ_EXTINT + 87)   /* TCC0 MC1 */
#define SAM_IRQ_TCC0MC2        (SAM_IRQ_EXTINT + 88)   /* TCC0 MC2 */
#define SAM_IRQ_TCC0MC3        (SAM_IRQ_EXTINT + 89)   /* TCC0 MC3 */
#define SAM_IRQ_TCC0MC4        (SAM_IRQ_EXTINT + 90)   /* TCC0 MC4 */
#define SAM_IRQ_TCC0MC5        (SAM_IRQ_EXTINT + 91)   /* TCC0 MC5 */

/* Timer/Counter for Control (TCC1) - 4 match/capture channels */

#define SAM_IRQ_TCC1           (SAM_IRQ_EXTINT + 92)   /* TCC1 */
#define SAM_IRQ_TCC1MC0        (SAM_IRQ_EXTINT + 93)   /* TCC1 MC0 */
#define SAM_IRQ_TCC1MC1        (SAM_IRQ_EXTINT + 94)   /* TCC1 MC1 */
#define SAM_IRQ_TCC1MC2        (SAM_IRQ_EXTINT + 95)   /* TCC1 MC2 */
#define SAM_IRQ_TCC1MC3        (SAM_IRQ_EXTINT + 96)   /* TCC1 MC3 */

/* Timer/Counter for Control (TCC2) - 3 match/capture channels */

#define SAM_IRQ_TCC2           (SAM_IRQ_EXTINT + 97)   /* TCC2 */
#define SAM_IRQ_TCC2MC0        (SAM_IRQ_EXTINT + 98)   /* TCC2 MC0 */
#define SAM_IRQ_TCC2MC1        (SAM_IRQ_EXTINT + 99)   /* TCC2 MC1 */
#define SAM_IRQ_TCC2MC2        (SAM_IRQ_EXTINT + 100)  /* TCC2 MC2 */

/* Timer/Counter for Control (TCC3) - 2 match/capture channels */

#define SAM_IRQ_TCC3           (SAM_IRQ_EXTINT + 101)  /* TCC3 */
#define SAM_IRQ_TCC3MC0        (SAM_IRQ_EXTINT + 102)  /* TCC3 MC0 */
#define SAM_IRQ_TCC3MC1        (SAM_IRQ_EXTINT + 103)  /* TCC3 MC1 */

/* Timer/Counter for Control (TCC4) - 2 match/capture channels */

#define SAM_IRQ_TCC4           (SAM_IRQ_EXTINT + 104)  /* TCC4 */
#define SAM_IRQ_TCC4MC0        (SAM_IRQ_EXTINT + 105)  /* TCC4 MC0 */
#define SAM_IRQ_TCC4MC1        (SAM_IRQ_EXTINT + 106)  /* TCC4 MC1 */

/* Basic Timer/Counters (TC0-TC7) */

#define SAM_IRQ_TC0            (SAM_IRQ_EXTINT + 107)  /* TC0 */
#define SAM_IRQ_TC1            (SAM_IRQ_EXTINT + 108)  /* TC1 */
#define SAM_IRQ_TC2            (SAM_IRQ_EXTINT + 109)  /* TC2 */
#define SAM_IRQ_TC3            (SAM_IRQ_EXTINT + 110)  /* TC3 */
#define SAM_IRQ_TC4            (SAM_IRQ_EXTINT + 111)  /* TC4 */
#define SAM_IRQ_TC5            (SAM_IRQ_EXTINT + 112)  /* TC5 */
#define SAM_IRQ_TC6            (SAM_IRQ_EXTINT + 113)  /* TC6 */
#define SAM_IRQ_TC7            (SAM_IRQ_EXTINT + 114)  /* TC7 */

/* Position Decoder (PDEC) */

#define SAM_IRQ_PDEC           (SAM_IRQ_EXTINT + 115)  /* PDEC */
#define SAM_IRQ_PDEC_MC0       (SAM_IRQ_EXTINT + 116)  /* PDEC MC0 */
#define SAM_IRQ_PDEC_MC1       (SAM_IRQ_EXTINT + 117)  /* PDEC MC1 */

/* Analog-to-Digital Converters */

#define SAM_IRQ_ADC0           (SAM_IRQ_EXTINT + 118)  /* ADC0 */
#define SAM_IRQ_ADC0_1         (SAM_IRQ_EXTINT + 119)  /* ADC0 Result */
#define SAM_IRQ_ADC1           (SAM_IRQ_EXTINT + 120)  /* ADC1 */
#define SAM_IRQ_ADC1_1         (SAM_IRQ_EXTINT + 121)  /* ADC1 Result */

/* Analog Comparator */

#define SAM_IRQ_AC             (SAM_IRQ_EXTINT + 122)  /* AC */

/* Digital-to-Analog Converter */

#define SAM_IRQ_DAC0           (SAM_IRQ_EXTINT + 123)  /* DAC Channel 0 */
#define SAM_IRQ_DAC1           (SAM_IRQ_EXTINT + 124)  /* DAC Channel 1 */
#define SAM_IRQ_DAC2           (SAM_IRQ_EXTINT + 125)  /* DAC Empty 0 */
#define SAM_IRQ_DAC3           (SAM_IRQ_EXTINT + 126)  /* DAC Empty 1 */
#define SAM_IRQ_DAC4           (SAM_IRQ_EXTINT + 127)  /* DAC Result */

/* Audio / Camera / Crypto */

#define SAM_IRQ_I2S            (SAM_IRQ_EXTINT + 128)  /* I2S */
#define SAM_IRQ_PCC            (SAM_IRQ_EXTINT + 129)  /* PCC */
#define SAM_IRQ_AES            (SAM_IRQ_EXTINT + 130)  /* AES */
#define SAM_IRQ_TRNG           (SAM_IRQ_EXTINT + 131)  /* TRNG */
#define SAM_IRQ_ICM            (SAM_IRQ_EXTINT + 132)  /* ICM */
#define SAM_IRQ_PUKCC          (SAM_IRQ_EXTINT + 133)  /* PUKCC */

/* QSPI / SDHC */

#define SAM_IRQ_QSPI           (SAM_IRQ_EXTINT + 134)  /* QSPI */
#define SAM_IRQ_SDHC0          (SAM_IRQ_EXTINT + 135)  /* SDHC0 */
#define SAM_IRQ_SDHC1          (SAM_IRQ_EXTINT + 136)  /* SDHC1 */

/* Total number of peripheral interrupt sources */

#define SAM_IRQ_NEXTINT        137

#endif /* __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H */
