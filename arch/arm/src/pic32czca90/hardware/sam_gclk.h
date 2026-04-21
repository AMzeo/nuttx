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
#define SAM_GCLK_NCH                62   /* 62 peripheral channels (0-61)  */

/* =========================================================================
 * GCLK Peripheral Channel IDs for PIC32CZ CA80/CA90
 * Source: PIC32CZ8110CA80208_DFP and PIC32CZ-CA90_DFP instance files.
 * Both DFPs are identical for all channel assignments below.
 *
 * NOTE: Channels 3/4/5/6/18 differ from SAMD5x layout — see values below.
 * =========================================================================
 */

#define GCLK_CHAN_DFLL48M_REF       0    /* DFLL48M reference clock        */
#define GCLK_CHAN_DPLL0_REF         1    /* PLL0 GCLK reference input      */
#define GCLK_CHAN_DPLL1_REF         2    /* PLL1 GCLK reference input      */
#define GCLK_CHAN_FREQM_MSR         3    /* FREQM measure clock            */
#define GCLK_CHAN_FREQM_REF         4    /* FREQM reference clock          */
#define GCLK_CHAN_EIC               5    /* External Interrupt Controller  */

/* SERCOM slow clock — shared by all SERCOM instances (SERCOM*_GCLK_ID_SLOW)
 * Channels 6-17 are unassigned / reserved. */

#define GCLK_CHAN_SERCOM_SLOW       18   /* SERCOM slow clock (all SERCOM) */

/* SERCOM core clocks – DFP verified (SERCOM*_GCLK_ID_CORE) */

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

/* Timer/Counter for Control (TCC) — each instance has its OWN GCLK channel.
 * Source: DFP instance/tcc*.h (TCC*_GCLK_ID, verified).
 * NOTE: SAMD5x shared TCC channels — CA90 does NOT share. */

#define GCLK_CHAN_TCC0              31   /* TCC0 — same value as TCC0_GCLK_ID in sam_tcc.h */
#define GCLK_CHAN_TCC1              32
#define GCLK_CHAN_TCC2              33
#define GCLK_CHAN_TCC3              34
#define GCLK_CHAN_TCC4              35
#define GCLK_CHAN_TCC5              36
#define GCLK_CHAN_TCC6              37
#define GCLK_CHAN_TCC7              38
#define GCLK_CHAN_TCC8              39
#define GCLK_CHAN_TCC9              40

/* ADC / AC / PTC — DFP instance/adc.h, ac.h, ptc.h verified */

#define GCLK_CHAN_ADC               41   /* ADC0-3 shared GCLK channel     */
#define GCLK_CHAN_AC                42   /* Analog Comparator              */
#define GCLK_CHAN_PTC               43   /* Peripheral Touch Controller    */

/* CAN-FD (MCAN) — DFP instance/can*.h CAN*_GCLK_ID verified */

#define GCLK_CHAN_CAN0              46
#define GCLK_CHAN_CAN1              47
#define GCLK_CHAN_CAN2              48
#define GCLK_CHAN_CAN3              49
#define GCLK_CHAN_CAN4              50
#define GCLK_CHAN_CAN5              51

/* SDMMC — DFP instance/sdmmc*.h verified (channels 58-61)
 * Each instance has a main clock and a slow clock channel. */

#define GCLK_CHAN_SDMMC0            58   /* SDMMC0 main clock              */
#define GCLK_CHAN_SDMMC0_SLOW       59   /* SDMMC0 slow clock              */
#define GCLK_CHAN_SDMMC1            60   /* SDMMC1 main clock              */
#define GCLK_CHAN_SDMMC1_SLOW       61   /* SDMMC1 slow clock              */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H */
