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

/* PIC32CZ CA80/CA90 Peripheral Interrupt Definitions
 *
 * Source of truth: Harmony device_vectors.h (PIC32CZ8110CA80208 DFP).
 * CA80 and CA90 have identical interrupt tables; CA90 adds HSM only.
 *
 * 222 peripheral IRQs total (SAM_IRQ_EXTINT+0 ... SAM_IRQ_EXTINT+221).
 * NOTE: this is NOT the SAMD5x/SAME54 table — the CA90 has a completely
 * different peripheral and ordering, including 10 SERCOMs (not 8), 10 TCCs
 * (not 5), 6 CANs, no TC basic timers, and different DMA/EVSYS grouping.
 *
 * SERCOM IRQ layout per instance (7 vectors each, base = EXTINT+55+n*7):
 *   +0: SERCOMn_6  Error
 *   +1: SERCOMn_5  Receive Break
 *   +2: SERCOMn_0  DRE (Data Register Empty / TX)
 *   +3: SERCOMn_1  TXC (Transmit Complete)
 *   +4: SERCOMn_2  RXC (Receive Complete / RX)
 *   +5: SERCOMn_3  Receive Start
 *   +6: SERCOMn_4  CTS Input Change
 *
 * This file should never be included directly but only indirectly
 * through nuttx/irq.h.
 ****************************************************************************/

#ifndef __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H
#define __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Flash controllers */

#define SAM_IRQ_FCW            (SAM_IRQ_EXTINT + 0)    /* Flash Write Controller */
#define SAM_IRQ_FCR_ECCERR     (SAM_IRQ_EXTINT + 1)    /* Flash Read ECC Error */
#define SAM_IRQ_FCR_CRC_FAULT  (SAM_IRQ_EXTINT + 2)    /* Flash Read CRC Fault */

/* Power / Clock / Reset */

#define SAM_IRQ_PM             (SAM_IRQ_EXTINT + 3)    /* Power Manager */
#define SAM_IRQ_SUPC           (SAM_IRQ_EXTINT + 4)    /* Supply Controller */
#define SAM_IRQ_OSCCTRL_FAIL   (SAM_IRQ_EXTINT + 5)    /* Oscillator Fail */
#define SAM_IRQ_OSCCTRL_XOSCRDY (SAM_IRQ_EXTINT + 6)  /* XOSC Ready */
#define SAM_IRQ_OSCCTRL_DFLLRDY (SAM_IRQ_EXTINT + 7)  /* DFLL Ready */
#define SAM_IRQ_OSCCTRL_OTHER  (SAM_IRQ_EXTINT + 8)    /* DFLL Other */
#define SAM_IRQ_OSCCTRL_LOCK   (SAM_IRQ_EXTINT + 9)    /* PLL Lock */
#define SAM_IRQ_OSC32KCTRL_FAIL (SAM_IRQ_EXTINT + 10) /* 32K Oscillator Fail */
#define SAM_IRQ_OSC32KCTRL_RDY (SAM_IRQ_EXTINT + 11)  /* 32K Oscillator Ready */
#define SAM_IRQ_MCLK           (SAM_IRQ_EXTINT + 12)   /* Main Clock */
#define SAM_IRQ_FREQM          (SAM_IRQ_EXTINT + 13)   /* Frequency Meter */
#define SAM_IRQ_WDT            (SAM_IRQ_EXTINT + 14)   /* Watchdog Timer */

/* RTC */

#define SAM_IRQ_RTC_TAMPER     (SAM_IRQ_EXTINT + 15)   /* RTC Tamper */
#define SAM_IRQ_RTC_OVF        (SAM_IRQ_EXTINT + 16)   /* RTC Overflow */
#define SAM_IRQ_RTC_PERIOD     (SAM_IRQ_EXTINT + 17)   /* RTC Period */
#define SAM_IRQ_RTC_COMPARE    (SAM_IRQ_EXTINT + 18)   /* RTC Compare */

/* External Interrupt Controller (EIC) */

#define SAM_IRQ_EXTINT0        (SAM_IRQ_EXTINT + 19)   /* EIC EXTINT0 */
#define SAM_IRQ_EXTINT1        (SAM_IRQ_EXTINT + 20)   /* EIC EXTINT1 */
#define SAM_IRQ_EXTINT2        (SAM_IRQ_EXTINT + 21)   /* EIC EXTINT2 */
#define SAM_IRQ_EXTINT3        (SAM_IRQ_EXTINT + 22)   /* EIC EXTINT3 */
#define SAM_IRQ_EXTINT4        (SAM_IRQ_EXTINT + 23)   /* EIC EXTINT4 */
#define SAM_IRQ_EXTINT5        (SAM_IRQ_EXTINT + 24)   /* EIC EXTINT5 */
#define SAM_IRQ_EXTINT6        (SAM_IRQ_EXTINT + 25)   /* EIC EXTINT6 */
#define SAM_IRQ_EXTINT7        (SAM_IRQ_EXTINT + 26)   /* EIC EXTINT7 */
#define SAM_IRQ_EXTINT8        (SAM_IRQ_EXTINT + 27)   /* EIC EXTINT8 */
#define SAM_IRQ_EXTINT9        (SAM_IRQ_EXTINT + 28)   /* EIC EXTINT9 */
#define SAM_IRQ_EXTINT10       (SAM_IRQ_EXTINT + 29)   /* EIC EXTINT10 */
#define SAM_IRQ_EXTINT11       (SAM_IRQ_EXTINT + 30)   /* EIC EXTINT11 */
#define SAM_IRQ_EXTINT12       (SAM_IRQ_EXTINT + 31)   /* EIC EXTINT12 */
#define SAM_IRQ_EXTINT13       (SAM_IRQ_EXTINT + 32)   /* EIC EXTINT13 */
#define SAM_IRQ_EXTINT14       (SAM_IRQ_EXTINT + 33)   /* EIC EXTINT14 */
#define SAM_IRQ_EXTINT15       (SAM_IRQ_EXTINT + 34)   /* EIC EXTINT15 */

/* System peripherals */

#define SAM_IRQ_PAC            (SAM_IRQ_EXTINT + 35)   /* Peripheral Access Controller */
#define SAM_IRQ_DRMTCM         (SAM_IRQ_EXTINT + 36)   /* TCM RAM ECC */
#define SAM_IRQ_MCRAMC         (SAM_IRQ_EXTINT + 37)   /* Multi-Channel RAM Controller */
#define SAM_IRQ_TRAM           (SAM_IRQ_EXTINT + 38)   /* Trust RAM */

/* DMA Controller (priority-grouped) */

#define SAM_IRQ_DMA_PRI3       (SAM_IRQ_EXTINT + 39)   /* DMA Priority 3 */
#define SAM_IRQ_DMA_PRI2       (SAM_IRQ_EXTINT + 40)   /* DMA Priority 2 */
#define SAM_IRQ_DMA_PRI1       (SAM_IRQ_EXTINT + 41)   /* DMA Priority 1 */
#define SAM_IRQ_DMA_PRI0       (SAM_IRQ_EXTINT + 42)   /* DMA Priority 0 */

/* Event System (12 channels, one IRQ each) */

#define SAM_IRQ_EVSYS0         (SAM_IRQ_EXTINT + 43)   /* EVSYS Channel 0 */
#define SAM_IRQ_EVSYS1         (SAM_IRQ_EXTINT + 44)   /* EVSYS Channel 1 */
#define SAM_IRQ_EVSYS2         (SAM_IRQ_EXTINT + 45)   /* EVSYS Channel 2 */
#define SAM_IRQ_EVSYS3         (SAM_IRQ_EXTINT + 46)   /* EVSYS Channel 3 */
#define SAM_IRQ_EVSYS4         (SAM_IRQ_EXTINT + 47)   /* EVSYS Channel 4 */
#define SAM_IRQ_EVSYS5         (SAM_IRQ_EXTINT + 48)   /* EVSYS Channel 5 */
#define SAM_IRQ_EVSYS6         (SAM_IRQ_EXTINT + 49)   /* EVSYS Channel 6 */
#define SAM_IRQ_EVSYS7         (SAM_IRQ_EXTINT + 50)   /* EVSYS Channel 7 */
#define SAM_IRQ_EVSYS8         (SAM_IRQ_EXTINT + 51)   /* EVSYS Channel 8 */
#define SAM_IRQ_EVSYS9         (SAM_IRQ_EXTINT + 52)   /* EVSYS Channel 9 */
#define SAM_IRQ_EVSYS10        (SAM_IRQ_EXTINT + 53)   /* EVSYS Channel 10 */
#define SAM_IRQ_EVSYS11        (SAM_IRQ_EXTINT + 54)   /* EVSYS Channel 11 */

/* SERCOM0 (base = EXTINT+55, 7 vectors: _6,_5,_0,_1,_2,_3,_4) */

#define SAM_IRQ_SERCOM0_6      (SAM_IRQ_EXTINT + 55)   /* SERCOM0 Error */
#define SAM_IRQ_SERCOM0_5      (SAM_IRQ_EXTINT + 56)   /* SERCOM0 Receive Break */
#define SAM_IRQ_SERCOM0_0      (SAM_IRQ_EXTINT + 57)   /* SERCOM0 DRE (TX) */
#define SAM_IRQ_SERCOM0_1      (SAM_IRQ_EXTINT + 58)   /* SERCOM0 TXC */
#define SAM_IRQ_SERCOM0_2      (SAM_IRQ_EXTINT + 59)   /* SERCOM0 RXC (RX) */
#define SAM_IRQ_SERCOM0_3      (SAM_IRQ_EXTINT + 60)   /* SERCOM0 Receive Start */
#define SAM_IRQ_SERCOM0_4      (SAM_IRQ_EXTINT + 61)   /* SERCOM0 CTS Change */

/* SERCOM1 (base = EXTINT+62) — console UART (PC04 TX / PC07 RX) */

#define SAM_IRQ_SERCOM1_6      (SAM_IRQ_EXTINT + 62)   /* SERCOM1 Error */
#define SAM_IRQ_SERCOM1_5      (SAM_IRQ_EXTINT + 63)   /* SERCOM1 Receive Break */
#define SAM_IRQ_SERCOM1_0      (SAM_IRQ_EXTINT + 64)   /* SERCOM1 DRE (TX) */
#define SAM_IRQ_SERCOM1_1      (SAM_IRQ_EXTINT + 65)   /* SERCOM1 TXC */
#define SAM_IRQ_SERCOM1_2      (SAM_IRQ_EXTINT + 66)   /* SERCOM1 RXC (RX) */
#define SAM_IRQ_SERCOM1_3      (SAM_IRQ_EXTINT + 67)   /* SERCOM1 Receive Start */
#define SAM_IRQ_SERCOM1_4      (SAM_IRQ_EXTINT + 68)   /* SERCOM1 CTS Change */

/* SERCOM2 (base = EXTINT+69) */

#define SAM_IRQ_SERCOM2_6      (SAM_IRQ_EXTINT + 69)   /* SERCOM2 Error */
#define SAM_IRQ_SERCOM2_5      (SAM_IRQ_EXTINT + 70)   /* SERCOM2 Receive Break */
#define SAM_IRQ_SERCOM2_0      (SAM_IRQ_EXTINT + 71)   /* SERCOM2 DRE (TX) */
#define SAM_IRQ_SERCOM2_1      (SAM_IRQ_EXTINT + 72)   /* SERCOM2 TXC */
#define SAM_IRQ_SERCOM2_2      (SAM_IRQ_EXTINT + 73)   /* SERCOM2 RXC (RX) */
#define SAM_IRQ_SERCOM2_3      (SAM_IRQ_EXTINT + 74)   /* SERCOM2 Receive Start */
#define SAM_IRQ_SERCOM2_4      (SAM_IRQ_EXTINT + 75)   /* SERCOM2 CTS Change */

/* SERCOM3 (base = EXTINT+76) */

#define SAM_IRQ_SERCOM3_6      (SAM_IRQ_EXTINT + 76)   /* SERCOM3 Error */
#define SAM_IRQ_SERCOM3_5      (SAM_IRQ_EXTINT + 77)   /* SERCOM3 Receive Break */
#define SAM_IRQ_SERCOM3_0      (SAM_IRQ_EXTINT + 78)   /* SERCOM3 DRE (TX) */
#define SAM_IRQ_SERCOM3_1      (SAM_IRQ_EXTINT + 79)   /* SERCOM3 TXC */
#define SAM_IRQ_SERCOM3_2      (SAM_IRQ_EXTINT + 80)   /* SERCOM3 RXC (RX) */
#define SAM_IRQ_SERCOM3_3      (SAM_IRQ_EXTINT + 81)   /* SERCOM3 Receive Start */
#define SAM_IRQ_SERCOM3_4      (SAM_IRQ_EXTINT + 82)   /* SERCOM3 CTS Change */

/* SERCOM4 (base = EXTINT+83) */

#define SAM_IRQ_SERCOM4_6      (SAM_IRQ_EXTINT + 83)   /* SERCOM4 Error */
#define SAM_IRQ_SERCOM4_5      (SAM_IRQ_EXTINT + 84)   /* SERCOM4 Receive Break */
#define SAM_IRQ_SERCOM4_0      (SAM_IRQ_EXTINT + 85)   /* SERCOM4 DRE (TX) */
#define SAM_IRQ_SERCOM4_1      (SAM_IRQ_EXTINT + 86)   /* SERCOM4 TXC */
#define SAM_IRQ_SERCOM4_2      (SAM_IRQ_EXTINT + 87)   /* SERCOM4 RXC (RX) */
#define SAM_IRQ_SERCOM4_3      (SAM_IRQ_EXTINT + 88)   /* SERCOM4 Receive Start */
#define SAM_IRQ_SERCOM4_4      (SAM_IRQ_EXTINT + 89)   /* SERCOM4 CTS Change */

/* SERCOM5 (base = EXTINT+90) */

#define SAM_IRQ_SERCOM5_6      (SAM_IRQ_EXTINT + 90)   /* SERCOM5 Error */
#define SAM_IRQ_SERCOM5_5      (SAM_IRQ_EXTINT + 91)   /* SERCOM5 Receive Break */
#define SAM_IRQ_SERCOM5_0      (SAM_IRQ_EXTINT + 92)   /* SERCOM5 DRE (TX) */
#define SAM_IRQ_SERCOM5_1      (SAM_IRQ_EXTINT + 93)   /* SERCOM5 TXC */
#define SAM_IRQ_SERCOM5_2      (SAM_IRQ_EXTINT + 94)   /* SERCOM5 RXC (RX) */
#define SAM_IRQ_SERCOM5_3      (SAM_IRQ_EXTINT + 95)   /* SERCOM5 Receive Start */
#define SAM_IRQ_SERCOM5_4      (SAM_IRQ_EXTINT + 96)   /* SERCOM5 CTS Change */

/* SERCOM6 (base = EXTINT+97) */

#define SAM_IRQ_SERCOM6_6      (SAM_IRQ_EXTINT + 97)   /* SERCOM6 Error */
#define SAM_IRQ_SERCOM6_5      (SAM_IRQ_EXTINT + 98)   /* SERCOM6 Receive Break */
#define SAM_IRQ_SERCOM6_0      (SAM_IRQ_EXTINT + 99)   /* SERCOM6 DRE (TX) */
#define SAM_IRQ_SERCOM6_1      (SAM_IRQ_EXTINT + 100)  /* SERCOM6 TXC */
#define SAM_IRQ_SERCOM6_2      (SAM_IRQ_EXTINT + 101)  /* SERCOM6 RXC (RX) */
#define SAM_IRQ_SERCOM6_3      (SAM_IRQ_EXTINT + 102)  /* SERCOM6 Receive Start */
#define SAM_IRQ_SERCOM6_4      (SAM_IRQ_EXTINT + 103)  /* SERCOM6 CTS Change */

/* SERCOM7 (base = EXTINT+104) */

#define SAM_IRQ_SERCOM7_6      (SAM_IRQ_EXTINT + 104)  /* SERCOM7 Error */
#define SAM_IRQ_SERCOM7_5      (SAM_IRQ_EXTINT + 105)  /* SERCOM7 Receive Break */
#define SAM_IRQ_SERCOM7_0      (SAM_IRQ_EXTINT + 106)  /* SERCOM7 DRE (TX) */
#define SAM_IRQ_SERCOM7_1      (SAM_IRQ_EXTINT + 107)  /* SERCOM7 TXC */
#define SAM_IRQ_SERCOM7_2      (SAM_IRQ_EXTINT + 108)  /* SERCOM7 RXC (RX) */
#define SAM_IRQ_SERCOM7_3      (SAM_IRQ_EXTINT + 109)  /* SERCOM7 Receive Start */
#define SAM_IRQ_SERCOM7_4      (SAM_IRQ_EXTINT + 110)  /* SERCOM7 CTS Change */

/* SERCOM8 (base = EXTINT+111) */

#define SAM_IRQ_SERCOM8_6      (SAM_IRQ_EXTINT + 111)  /* SERCOM8 Error */
#define SAM_IRQ_SERCOM8_5      (SAM_IRQ_EXTINT + 112)  /* SERCOM8 Receive Break */
#define SAM_IRQ_SERCOM8_0      (SAM_IRQ_EXTINT + 113)  /* SERCOM8 DRE (TX) */
#define SAM_IRQ_SERCOM8_1      (SAM_IRQ_EXTINT + 114)  /* SERCOM8 TXC */
#define SAM_IRQ_SERCOM8_2      (SAM_IRQ_EXTINT + 115)  /* SERCOM8 RXC (RX) */
#define SAM_IRQ_SERCOM8_3      (SAM_IRQ_EXTINT + 116)  /* SERCOM8 Receive Start */
#define SAM_IRQ_SERCOM8_4      (SAM_IRQ_EXTINT + 117)  /* SERCOM8 CTS Change */

/* SERCOM9 (base = EXTINT+118) */

#define SAM_IRQ_SERCOM9_6      (SAM_IRQ_EXTINT + 118)  /* SERCOM9 Error */
#define SAM_IRQ_SERCOM9_5      (SAM_IRQ_EXTINT + 119)  /* SERCOM9 Receive Break */
#define SAM_IRQ_SERCOM9_0      (SAM_IRQ_EXTINT + 120)  /* SERCOM9 DRE (TX) */
#define SAM_IRQ_SERCOM9_1      (SAM_IRQ_EXTINT + 121)  /* SERCOM9 TXC */
#define SAM_IRQ_SERCOM9_2      (SAM_IRQ_EXTINT + 122)  /* SERCOM9 RXC (RX) */
#define SAM_IRQ_SERCOM9_3      (SAM_IRQ_EXTINT + 123)  /* SERCOM9 Receive Start */
#define SAM_IRQ_SERCOM9_4      (SAM_IRQ_EXTINT + 124)  /* SERCOM9 CTS Change */

/* TCC0 (10 vectors: OTHER, CNT_TRIG, MC0-7) */

#define SAM_IRQ_TCC0           (SAM_IRQ_EXTINT + 125)  /* TCC0 Other */
#define SAM_IRQ_TCC0_CNT       (SAM_IRQ_EXTINT + 126)  /* TCC0 Counter/Retrigger */
#define SAM_IRQ_TCC0MC0        (SAM_IRQ_EXTINT + 127)  /* TCC0 MC0 */
#define SAM_IRQ_TCC0MC1        (SAM_IRQ_EXTINT + 128)  /* TCC0 MC1 */
#define SAM_IRQ_TCC0MC2        (SAM_IRQ_EXTINT + 129)  /* TCC0 MC2 */
#define SAM_IRQ_TCC0MC3        (SAM_IRQ_EXTINT + 130)  /* TCC0 MC3 */
#define SAM_IRQ_TCC0MC4        (SAM_IRQ_EXTINT + 131)  /* TCC0 MC4 */
#define SAM_IRQ_TCC0MC5        (SAM_IRQ_EXTINT + 132)  /* TCC0 MC5 */
#define SAM_IRQ_TCC0MC6        (SAM_IRQ_EXTINT + 133)  /* TCC0 MC6 */
#define SAM_IRQ_TCC0MC7        (SAM_IRQ_EXTINT + 134)  /* TCC0 MC7 */

/* TCC1 (10 vectors) */

#define SAM_IRQ_TCC1           (SAM_IRQ_EXTINT + 135)  /* TCC1 Other */
#define SAM_IRQ_TCC1_CNT       (SAM_IRQ_EXTINT + 136)  /* TCC1 Counter/Retrigger */
#define SAM_IRQ_TCC1MC0        (SAM_IRQ_EXTINT + 137)  /* TCC1 MC0 */
#define SAM_IRQ_TCC1MC1        (SAM_IRQ_EXTINT + 138)  /* TCC1 MC1 */
#define SAM_IRQ_TCC1MC2        (SAM_IRQ_EXTINT + 139)  /* TCC1 MC2 */
#define SAM_IRQ_TCC1MC3        (SAM_IRQ_EXTINT + 140)  /* TCC1 MC3 */
#define SAM_IRQ_TCC1MC4        (SAM_IRQ_EXTINT + 141)  /* TCC1 MC4 */
#define SAM_IRQ_TCC1MC5        (SAM_IRQ_EXTINT + 142)  /* TCC1 MC5 */
#define SAM_IRQ_TCC1MC6        (SAM_IRQ_EXTINT + 143)  /* TCC1 MC6 */
#define SAM_IRQ_TCC1MC7        (SAM_IRQ_EXTINT + 144)  /* TCC1 MC7 */

/* TCC2 (8 vectors: OTHER, CNT, MC0-5) */

#define SAM_IRQ_TCC2           (SAM_IRQ_EXTINT + 145)  /* TCC2 Other */
#define SAM_IRQ_TCC2_CNT       (SAM_IRQ_EXTINT + 146)  /* TCC2 Counter/Retrigger */
#define SAM_IRQ_TCC2MC0        (SAM_IRQ_EXTINT + 147)  /* TCC2 MC0 */
#define SAM_IRQ_TCC2MC1        (SAM_IRQ_EXTINT + 148)  /* TCC2 MC1 */
#define SAM_IRQ_TCC2MC2        (SAM_IRQ_EXTINT + 149)  /* TCC2 MC2 */
#define SAM_IRQ_TCC2MC3        (SAM_IRQ_EXTINT + 150)  /* TCC2 MC3 */
#define SAM_IRQ_TCC2MC4        (SAM_IRQ_EXTINT + 151)  /* TCC2 MC4 */
#define SAM_IRQ_TCC2MC5        (SAM_IRQ_EXTINT + 152)  /* TCC2 MC5 */

/* TCC3 (4 vectors: OTHER, CNT, MC0-1) */

#define SAM_IRQ_TCC3           (SAM_IRQ_EXTINT + 153)  /* TCC3 Other */
#define SAM_IRQ_TCC3_CNT       (SAM_IRQ_EXTINT + 154)  /* TCC3 Counter/Retrigger */
#define SAM_IRQ_TCC3MC0        (SAM_IRQ_EXTINT + 155)  /* TCC3 MC0 */
#define SAM_IRQ_TCC3MC1        (SAM_IRQ_EXTINT + 156)  /* TCC3 MC1 */

/* TCC4 (4 vectors) */

#define SAM_IRQ_TCC4           (SAM_IRQ_EXTINT + 157)  /* TCC4 Other */
#define SAM_IRQ_TCC4_CNT       (SAM_IRQ_EXTINT + 158)  /* TCC4 Counter/Retrigger */
#define SAM_IRQ_TCC4MC0        (SAM_IRQ_EXTINT + 159)  /* TCC4 MC0 */
#define SAM_IRQ_TCC4MC1        (SAM_IRQ_EXTINT + 160)  /* TCC4 MC1 */

/* TCC5 (4 vectors) */

#define SAM_IRQ_TCC5           (SAM_IRQ_EXTINT + 161)  /* TCC5 Other */
#define SAM_IRQ_TCC5_CNT       (SAM_IRQ_EXTINT + 162)  /* TCC5 Counter/Retrigger */
#define SAM_IRQ_TCC5MC0        (SAM_IRQ_EXTINT + 163)  /* TCC5 MC0 */
#define SAM_IRQ_TCC5MC1        (SAM_IRQ_EXTINT + 164)  /* TCC5 MC1 */

/* TCC6 (4 vectors) */

#define SAM_IRQ_TCC6           (SAM_IRQ_EXTINT + 165)  /* TCC6 Other */
#define SAM_IRQ_TCC6_CNT       (SAM_IRQ_EXTINT + 166)  /* TCC6 Counter/Retrigger */
#define SAM_IRQ_TCC6MC0        (SAM_IRQ_EXTINT + 167)  /* TCC6 MC0 */
#define SAM_IRQ_TCC6MC1        (SAM_IRQ_EXTINT + 168)  /* TCC6 MC1 */

/* TCC7 (4 vectors) */

#define SAM_IRQ_TCC7           (SAM_IRQ_EXTINT + 169)  /* TCC7 Other */
#define SAM_IRQ_TCC7_CNT       (SAM_IRQ_EXTINT + 170)  /* TCC7 Counter/Retrigger */
#define SAM_IRQ_TCC7MC0        (SAM_IRQ_EXTINT + 171)  /* TCC7 MC0 */
#define SAM_IRQ_TCC7MC1        (SAM_IRQ_EXTINT + 172)  /* TCC7 MC1 */

/* TCC8 (4 vectors) */

#define SAM_IRQ_TCC8           (SAM_IRQ_EXTINT + 173)  /* TCC8 Other */
#define SAM_IRQ_TCC8_CNT       (SAM_IRQ_EXTINT + 174)  /* TCC8 Counter/Retrigger */
#define SAM_IRQ_TCC8MC0        (SAM_IRQ_EXTINT + 175)  /* TCC8 MC0 */
#define SAM_IRQ_TCC8MC1        (SAM_IRQ_EXTINT + 176)  /* TCC8 MC1 */

/* TCC9 (8 vectors: OTHER, CNT, MC0-5) */

#define SAM_IRQ_TCC9           (SAM_IRQ_EXTINT + 177)  /* TCC9 Other */
#define SAM_IRQ_TCC9_CNT       (SAM_IRQ_EXTINT + 178)  /* TCC9 Counter/Retrigger */
#define SAM_IRQ_TCC9MC0        (SAM_IRQ_EXTINT + 179)  /* TCC9 MC0 */
#define SAM_IRQ_TCC9MC1        (SAM_IRQ_EXTINT + 180)  /* TCC9 MC1 */
#define SAM_IRQ_TCC9MC2        (SAM_IRQ_EXTINT + 181)  /* TCC9 MC2 */
#define SAM_IRQ_TCC9MC3        (SAM_IRQ_EXTINT + 182)  /* TCC9 MC3 */
#define SAM_IRQ_TCC9MC4        (SAM_IRQ_EXTINT + 183)  /* TCC9 MC4 */
#define SAM_IRQ_TCC9MC5        (SAM_IRQ_EXTINT + 184)  /* TCC9 MC5 */

/* ADC (global + 4 cores) */

#define SAM_IRQ_ADC_GLOBAL     (SAM_IRQ_EXTINT + 185)  /* ADC Global */
#define SAM_IRQ_ADC_CORE1      (SAM_IRQ_EXTINT + 186)  /* ADC Core 1 */
#define SAM_IRQ_ADC_CORE2      (SAM_IRQ_EXTINT + 187)  /* ADC Core 2 */
#define SAM_IRQ_ADC_CORE3      (SAM_IRQ_EXTINT + 188)  /* ADC Core 3 */
#define SAM_IRQ_ADC_CORE4      (SAM_IRQ_EXTINT + 189)  /* ADC Core 4 */

/* Analog Comparator / Touch / Audio */

#define SAM_IRQ_AC             (SAM_IRQ_EXTINT + 190)  /* Analog Comparator */
#define SAM_IRQ_PTC            (SAM_IRQ_EXTINT + 191)  /* Peripheral Touch Controller */
#define SAM_IRQ_SPI_IXS0       (SAM_IRQ_EXTINT + 192)  /* Audio SPI 0 */
#define SAM_IRQ_SPI_IXS1       (SAM_IRQ_EXTINT + 193)  /* Audio SPI 1 */

/* CAN-FD (6 controllers) */

#define SAM_IRQ_CAN0           (SAM_IRQ_EXTINT + 194)  /* CAN0 */
#define SAM_IRQ_CAN1           (SAM_IRQ_EXTINT + 195)  /* CAN1 */
#define SAM_IRQ_CAN2           (SAM_IRQ_EXTINT + 196)  /* CAN2 */
#define SAM_IRQ_CAN3           (SAM_IRQ_EXTINT + 197)  /* CAN3 */
#define SAM_IRQ_CAN4           (SAM_IRQ_EXTINT + 198)  /* CAN4 */
#define SAM_IRQ_CAN5           (SAM_IRQ_EXTINT + 199)  /* CAN5 */

/* Reserved 200-201 */

/* Ethernet MAC (6 priority queues) */

#define SAM_IRQ_ETH_PRI_Q_0    (SAM_IRQ_EXTINT + 202)  /* Ethernet MAC Queue 0 */
#define SAM_IRQ_ETH_PRI_Q_1    (SAM_IRQ_EXTINT + 203)  /* Ethernet MAC Queue 1 */
#define SAM_IRQ_ETH_PRI_Q_2    (SAM_IRQ_EXTINT + 204)  /* Ethernet MAC Queue 2 */
#define SAM_IRQ_ETH_PRI_Q_3    (SAM_IRQ_EXTINT + 205)  /* Ethernet MAC Queue 3 */
#define SAM_IRQ_ETH_PRI_Q_4    (SAM_IRQ_EXTINT + 206)  /* Ethernet MAC Queue 4 */
#define SAM_IRQ_ETH_PRI_Q_5    (SAM_IRQ_EXTINT + 207)  /* Ethernet MAC Queue 5 */

/* QSPI / SD-MMC / USB / Misc */

#define SAM_IRQ_SQI0           (SAM_IRQ_EXTINT + 208)  /* Quad SPI 0 */
#define SAM_IRQ_SQI1           (SAM_IRQ_EXTINT + 209)  /* Quad SPI 1 */
#define SAM_IRQ_TRNG           (SAM_IRQ_EXTINT + 210)  /* True Random Number Generator */
#define SAM_IRQ_SDMMC0         (SAM_IRQ_EXTINT + 211)  /* SD/MMC 0 */
#define SAM_IRQ_SDMMC1         (SAM_IRQ_EXTINT + 212)  /* SD/MMC 1 */
#define SAM_IRQ_USBHS0         (SAM_IRQ_EXTINT + 213)  /* USB High-Speed 0 */
#define SAM_IRQ_USBHS1         (SAM_IRQ_EXTINT + 214)  /* USB High-Speed 1 */

/* Reserved 215-217 */

/* Miscellaneous */

#define SAM_IRQ_MLB_GENERAL    (SAM_IRQ_EXTINT + 218)  /* Media Local Bus General */
#define SAM_IRQ_MLB_BUSREQ     (SAM_IRQ_EXTINT + 219)  /* Media Local Bus Request */
#define SAM_IRQ_CM7H_CTIIRQ_0  (SAM_IRQ_EXTINT + 220)  /* CTI IRQ 0 */
#define SAM_IRQ_CM7H_CTIIRQ_1  (SAM_IRQ_EXTINT + 221)  /* CTI IRQ 1 */

/* Total peripheral interrupt count (verified from device_vectors.h) */

#define SAM_IRQ_NEXTINT        222

#endif /* __ARCH_ARM_INCLUDE_PIC32CZCA90_PIC32CZCA90_IRQ_H */
