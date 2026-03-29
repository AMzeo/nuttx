/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_gclk.h
 *
 * PIC32CZ CA90 Generic Clock Controller (GCLK) Register Definitions
 *
 * 12 generators, 48 peripheral channels. Same IP block as SAMD5E5.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "hardware/sam_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* GCLK register offsets */

#define SAM_GCLK_CTRLA_OFFSET      0x0000  /* Control */
#define SAM_GCLK_SYNCBUSY_OFFSET   0x0004  /* Synchronization Busy */
#define SAM_GCLK_GENCTRL_OFFSET(n) (0x0020 + ((n) * 4))  /* Generator Control n */
#define SAM_GCLK_PCHCTRL_OFFSET(n) (0x0080 + ((n) * 4))  /* Peripheral Channel Control n */

/* GCLK register addresses */

#define SAM_GCLK_CTRLA             (SAM_GCLK_BASE + SAM_GCLK_CTRLA_OFFSET)
#define SAM_GCLK_SYNCBUSY          (SAM_GCLK_BASE + SAM_GCLK_SYNCBUSY_OFFSET)
#define SAM_GCLK_GENCTRL(n)        (SAM_GCLK_BASE + SAM_GCLK_GENCTRL_OFFSET(n))
#define SAM_GCLK_PCHCTRL(n)        (SAM_GCLK_BASE + SAM_GCLK_PCHCTRL_OFFSET(n))

/* GCLK CTRLA register bit definitions */

#define GCLK_CTRLA_SWRST           (1 << 0)  /* Bit 0: Software Reset */

/* GCLK SYNCBUSY register bit definitions */

#define GCLK_SYNCBUSY_SWRST        (1 << 0)  /* Bit 0: SWRST Synchronization Busy */
#define GCLK_SYNCBUSY_GENCTRL(n)   (1 << ((n) + 2))  /* GENCTRL n Synchronization Busy */

/* GCLK Generator Control register bit definitions */

#define GCLK_GENCTRL_SRC_SHIFT     (0)       /* Bits 0-3: Generator Source Selection */
#define GCLK_GENCTRL_SRC_MASK      (0x0f << GCLK_GENCTRL_SRC_SHIFT)
#  define GCLK_GENCTRL_SRC_XOSC0   (0 << GCLK_GENCTRL_SRC_SHIFT)  /* XOSC0 */
#  define GCLK_GENCTRL_SRC_XOSC1   (1 << GCLK_GENCTRL_SRC_SHIFT)  /* XOSC1 */
#  define GCLK_GENCTRL_SRC_GCLKIN   (2 << GCLK_GENCTRL_SRC_SHIFT)  /* GCLK_IN */
#  define GCLK_GENCTRL_SRC_GCLKGEN1 (3 << GCLK_GENCTRL_SRC_SHIFT)  /* GCLK_GEN1 */
#  define GCLK_GENCTRL_SRC_OSCULP32K (4 << GCLK_GENCTRL_SRC_SHIFT) /* OSCULP32K */
#  define GCLK_GENCTRL_SRC_XOSC32K  (5 << GCLK_GENCTRL_SRC_SHIFT) /* XOSC32K */
#  define GCLK_GENCTRL_SRC_DFLL     (6 << GCLK_GENCTRL_SRC_SHIFT)  /* DFLL */
#  define GCLK_GENCTRL_SRC_DPLL0    (7 << GCLK_GENCTRL_SRC_SHIFT)  /* DPLL0 */
#  define GCLK_GENCTRL_SRC_DPLL1    (8 << GCLK_GENCTRL_SRC_SHIFT)  /* DPLL1 */
#define GCLK_GENCTRL_GENEN         (1 << 8)  /* Bit 8: Generator Enable */
#define GCLK_GENCTRL_IDC           (1 << 9)  /* Bit 9: Improve Duty Cycle */
#define GCLK_GENCTRL_OOV           (1 << 10) /* Bit 10: Output Off Value */
#define GCLK_GENCTRL_OE            (1 << 11) /* Bit 11: Output Enable */
#define GCLK_GENCTRL_DIVSEL        (1 << 12) /* Bit 12: Divide Selection */
#define GCLK_GENCTRL_RUNSTDBY      (1 << 13) /* Bit 13: Run in Standby */
#define GCLK_GENCTRL_DIV_SHIFT     (16)      /* Bits 16-31: Division Factor */
#define GCLK_GENCTRL_DIV_MASK      (0xffff << GCLK_GENCTRL_DIV_SHIFT)

/* GCLK Peripheral Channel Control register bit definitions */

#define GCLK_PCHCTRL_GEN_SHIFT     (0)       /* Bits 0-3: Generator Selection */
#define GCLK_PCHCTRL_GEN_MASK      (0x0f << GCLK_PCHCTRL_GEN_SHIFT)
#  define GCLK_PCHCTRL_GEN(n)      ((n) << GCLK_PCHCTRL_GEN_SHIFT)
#define GCLK_PCHCTRL_CHEN          (1 << 6)  /* Bit 6: Channel Enable */
#define GCLK_PCHCTRL_WRTLOCK       (1 << 7)  /* Bit 7: Write Lock */

/* Number of GCLK generators and peripheral channels */

#define SAM_GCLK_NGEN              12
#define SAM_GCLK_NCH               48

/* GCLK peripheral channel assignments for PIC32CZ CA90 */

#define GCLK_CHAN_DFLL48M           0   /* DFLL48M reference */
#define GCLK_CHAN_DPLL0             1   /* DPLL0 reference */
#define GCLK_CHAN_DPLL1             2   /* DPLL1 reference */
#define GCLK_CHAN_DPLL0_32K         3   /* DPLL0/1 32K reference */
#define GCLK_CHAN_EIC               4   /* EIC */
#define GCLK_CHAN_FREQM_MSR         5   /* FREQM measure */
#define GCLK_CHAN_FREQM_REF         6   /* FREQM reference */
#define GCLK_CHAN_SERCOM0_CORE      7   /* SERCOM0 core */
#define GCLK_CHAN_SERCOM1_CORE      8   /* SERCOM1 core */
#define GCLK_CHAN_TC0_TC1           9   /* TC0/TC1 */
#define GCLK_CHAN_USB               10  /* USB */
#define GCLK_CHAN_EVSYS0            11  /* EVSYS channel 0 */
#define GCLK_CHAN_EVSYS1            12  /* EVSYS channel 1 */
#define GCLK_CHAN_EVSYS2            13  /* EVSYS channel 2 */
#define GCLK_CHAN_EVSYS3            14  /* EVSYS channel 3 */
#define GCLK_CHAN_EVSYS4            15  /* EVSYS channels 4-11 */
#define GCLK_CHAN_SERCOM2_CORE      23  /* SERCOM2 core */
#define GCLK_CHAN_SERCOM3_CORE      24  /* SERCOM3 core */
#define GCLK_CHAN_TCC0_TCC1         25  /* TCC0/TCC1 */
#define GCLK_CHAN_TC2_TC3           26  /* TC2/TC3 */
#define GCLK_CHAN_CAN0              27  /* CAN0 */
#define GCLK_CHAN_CAN1              28  /* CAN1 */
#define GCLK_CHAN_TCC2_TCC3         29  /* TCC2/TCC3 */
#define GCLK_CHAN_TC4_TC5           30  /* TC4/TC5 */
#define GCLK_CHAN_PDEC              31  /* PDEC */
#define GCLK_CHAN_AC                32  /* AC */
#define GCLK_CHAN_CCL               33  /* CCL */
#define GCLK_CHAN_SERCOM4_CORE      34  /* SERCOM4 core */
#define GCLK_CHAN_SERCOM5_CORE      35  /* SERCOM5 core */
#define GCLK_CHAN_SERCOM6_CORE      36  /* SERCOM6 core */
#define GCLK_CHAN_SERCOM7_CORE      37  /* SERCOM7 core */
#define GCLK_CHAN_TCC4              38  /* TCC4 */
#define GCLK_CHAN_TC6_TC7           39  /* TC6/TC7 */
#define GCLK_CHAN_ADC0              40  /* ADC0 */
#define GCLK_CHAN_ADC1              41  /* ADC1 */
#define GCLK_CHAN_DAC               42  /* DAC */
#define GCLK_CHAN_I2S0              43  /* I2S 0 */
#define GCLK_CHAN_I2S1              44  /* I2S 1 */
#define GCLK_CHAN_SDHC0             45  /* SDHC0 */
#define GCLK_CHAN_SDHC1             46  /* SDHC1 */
#define GCLK_CHAN_CM4_TRACE         47  /* CM7 Trace (actually CM7 on CA90) */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_GCLK_H */
