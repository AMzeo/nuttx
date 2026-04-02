/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_gclk.h
 *
 * PIC32CZ CA90 Generic Clock Controller (GCLK) – DS60001749K Section 20
 * Base: 0x44050000
 *
 * Register offsets and peripheral channel IDs verified from
 * PIC32CZ8110CA80208_DFP/component/gclk.h and instance files.
 *
 * CRITICAL FIXES vs previous version:
 *   1. SRC values corrected for CA90 (SAMD5x values were wrong):
 *      OSCULP32K = 3  (was 4)
 *      DFLL48M   = 5  (was 6)
 *      PLL0_1    = 6  (was DPLL0=7, which does not exist on CA90)
 *      PLL0_2    = 7
 *   2. SERCOM4 core clock channel = 25 (was 34 – SAMD5x value)
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets (Section 20.7)
 * =========================================================================
 */

#define SAM_GCLK_CTRLA_OFFSET       0x0000
#define SAM_GCLK_SYNCBUSY_OFFSET    0x0004
#define SAM_GCLK_GENCTRL_OFFSET(n)  (0x0020 + ((n) * 4))
#define SAM_GCLK_PCHCTRL_OFFSET(n)  (0x0080 + ((n) * 4))

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_GCLK_CTRLA              (SAM_GCLK_BASE + SAM_GCLK_CTRLA_OFFSET)
#define SAM_GCLK_SYNCBUSY           (SAM_GCLK_BASE + SAM_GCLK_SYNCBUSY_OFFSET)
#define SAM_GCLK_GENCTRL(n)         (SAM_GCLK_BASE + SAM_GCLK_GENCTRL_OFFSET(n))
#define SAM_GCLK_PCHCTRL(n)         (SAM_GCLK_BASE + SAM_GCLK_PCHCTRL_OFFSET(n))

/* =========================================================================
 * CTRLA Bits
 * =========================================================================
 */

#define GCLK_CTRLA_SWRST            (1 << 0)

/* =========================================================================
 * SYNCBUSY Bits
 * =========================================================================
 */

#define GCLK_SYNCBUSY_SWRST         (1 << 0)
#define GCLK_SYNCBUSY_GENCTRL(n)    (1 << ((n) + 2))

/* =========================================================================
 * GENCTRL Bits
 *
 * SRC field values – CA90 DFP verified (DIFFERENT from SAMD5x):
 *   0 = XOSC       (external oscillator)
 *   1 = GCLKIN
 *   2 = GCLKGEN1
 *   3 = OSCULP32K  (NOT 4 like SAMD5x)
 *   4 = XOSC32K
 *   5 = DFLL48M    (NOT 6 like SAMD5x)
 *   6 = PLL0_1     300 MHz output (GCLK0 = CPU, GCLK1 source)
 *   7 = PLL0_2     150 MHz output
 *   8 = PLL0_3, 9 = PLL0_4
 * =========================================================================
 */

#define GCLK_GENCTRL_SRC_SHIFT      0
#define GCLK_GENCTRL_SRC_MASK       (0x0f << GCLK_GENCTRL_SRC_SHIFT)
#  define GCLK_GENCTRL_SRC_XOSC     (0 << GCLK_GENCTRL_SRC_SHIFT) /* ext osc  */
#  define GCLK_GENCTRL_SRC_XOSC0    (0 << GCLK_GENCTRL_SRC_SHIFT) /* alias    */
#  define GCLK_GENCTRL_SRC_GCLKIN   (1 << GCLK_GENCTRL_SRC_SHIFT)
#  define GCLK_GENCTRL_SRC_GCLKGEN1 (2 << GCLK_GENCTRL_SRC_SHIFT)
#  define GCLK_GENCTRL_SRC_OSCULP32K (3 << GCLK_GENCTRL_SRC_SHIFT) /* 3 not 4 */
#  define GCLK_GENCTRL_SRC_XOSC32K  (4 << GCLK_GENCTRL_SRC_SHIFT)
#  define GCLK_GENCTRL_SRC_DFLL     (5 << GCLK_GENCTRL_SRC_SHIFT) /* 5 not 6 */
#  define GCLK_GENCTRL_SRC_PLL0_1   (6 << GCLK_GENCTRL_SRC_SHIFT) /* 300 MHz */
#  define GCLK_GENCTRL_SRC_PLL0_2   (7 << GCLK_GENCTRL_SRC_SHIFT) /* 150 MHz */
#  define GCLK_GENCTRL_SRC_DPLL0    (6 << GCLK_GENCTRL_SRC_SHIFT) /* = PLL0_1 */
#  define GCLK_GENCTRL_SRC_DPLL1    (7 << GCLK_GENCTRL_SRC_SHIFT) /* = PLL0_2 */
#define GCLK_GENCTRL_GENEN          (1 << 8)
#define GCLK_GENCTRL_IDC            (1 << 9)
#define GCLK_GENCTRL_OOV            (1 << 10)
#define GCLK_GENCTRL_OE             (1 << 11)
#define GCLK_GENCTRL_DIVSEL         (1 << 12)
#define GCLK_GENCTRL_RUNSTDBY       (1 << 13)
#define GCLK_GENCTRL_DIV_SHIFT      16
#define GCLK_GENCTRL_DIV_MASK       (0xffff << GCLK_GENCTRL_DIV_SHIFT)

/* =========================================================================
 * PCHCTRL Bits
 * =========================================================================
 */

#define GCLK_PCHCTRL_GEN_SHIFT      0
#define GCLK_PCHCTRL_GEN_MASK       (0x0f << GCLK_PCHCTRL_GEN_SHIFT)
#  define GCLK_PCHCTRL_GEN(n)       ((n) << GCLK_PCHCTRL_GEN_SHIFT)
#define GCLK_PCHCTRL_CHEN           (1 << 6)
#define GCLK_PCHCTRL_WRTLOCK        (1 << 7)

/* =========================================================================
 * Number of generators and channels
 * =========================================================================
 */

#define SAM_GCLK_NGEN               12   /* 12 generators                  */
#define SAM_GCLK_NCH                56   /* 56 peripheral channels         */

/* =========================================================================
 * GCLK Peripheral Channel IDs for PIC32CZ CA90
 * Source: PIC32CZ8110CA80208_DFP instance files (verified)
 * =========================================================================
 */

#define GCLK_CHAN_DFLL48M_REF       0    /* DFLL48M reference clock        */
#define GCLK_CHAN_DPLL0_REF         1    /* PLL0 GCLK reference input      */
#define GCLK_CHAN_DPLL1_REF         2    /* PLL1 GCLK reference input      */
#define GCLK_CHAN_SLOW              3    /* Slow clock (SERCOM slow, WDT)  */
#define GCLK_CHAN_EIC               4    /* External Interrupt Controller  */
#define GCLK_CHAN_FREQM_MSR         5    /* FREQM measure clock            */
#define GCLK_CHAN_FREQM_REF         6    /* FREQM reference clock          */

/* SERCOM core clocks – DFP verified (channel = SERCOM_GCLK_ID_CORE) */

#define GCLK_CHAN_SERCOM0_CORE      21   /* SERCOM0 core                   */
#define GCLK_CHAN_SERCOM1_CORE      22   /* SERCOM1 core                   */
#define GCLK_CHAN_SERCOM2_CORE      23   /* SERCOM2 core                   */
#define GCLK_CHAN_SERCOM3_CORE      24   /* SERCOM3 core                   */
#define GCLK_CHAN_SERCOM4_CORE      25   /* SERCOM4 core – console UART    */
#define GCLK_CHAN_SERCOM5_CORE      26   /* SERCOM5 core                   */
#define GCLK_CHAN_SERCOM6_CORE      27   /* SERCOM6 core                   */
#define GCLK_CHAN_SERCOM7_CORE      28   /* SERCOM7 core                   */
#define GCLK_CHAN_SERCOM8_CORE      29   /* SERCOM8 core                   */
#define GCLK_CHAN_SERCOM9_CORE      30   /* SERCOM9 core                   */

/* CAN */

#define GCLK_CHAN_CAN0              31
#define GCLK_CHAN_CAN1              32
#define GCLK_CHAN_CAN2              33
#define GCLK_CHAN_CAN3              34
#define GCLK_CHAN_CAN4              35
#define GCLK_CHAN_CAN5              36

/* Timer/Counter */

#define GCLK_CHAN_TCC0_TCC1         37
#define GCLK_CHAN_TCC2              38
#define GCLK_CHAN_TCC3_TCC4         39
#define GCLK_CHAN_TCC5_TCC6         40
#define GCLK_CHAN_TCC7_TCC8_TCC9    41

/* ADC / AC / PTC */

#define GCLK_CHAN_ADC               42
#define GCLK_CHAN_AC                43
#define GCLK_CHAN_PTC               44

/* Storage / Audio */

#define GCLK_CHAN_SDHC0             45
#define GCLK_CHAN_SDHC1             46
#define GCLK_CHAN_I2S0              47
#define GCLK_CHAN_I2S1              48

/* Ethernet */

#define GCLK_CHAN_GMAC              51

/* Aliases */

#define GCLK_CHAN_SERCOM_SLOW       GCLK_CHAN_SLOW

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H */
