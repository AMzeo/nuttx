/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_mclk.h
 *
 * PIC32CZ CA90 Main Clock (MCLK) Register Definitions
 *
 * Includes APB buses A through E. Note: APB E is CA90-specific and
 * hosts SERCOM4.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "hardware/sam_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* MCLK register offsets */

#define SAM_MCLK_HSDIV_OFFSET    0x0000  /* HS Clock Division */
#define SAM_MCLK_CPUDIV_OFFSET   0x0001  /* CPU Clock Division */
#define SAM_MCLK_AHBMASK_OFFSET  0x0010  /* AHB Mask */
#define SAM_MCLK_APBAMASK_OFFSET 0x0014  /* APBA Mask */
#define SAM_MCLK_APBBMASK_OFFSET 0x0018  /* APBB Mask */
#define SAM_MCLK_APBCMASK_OFFSET 0x001c  /* APBC Mask */
#define SAM_MCLK_APBDMASK_OFFSET 0x0020  /* APBD Mask */
#define SAM_MCLK_APBEMASK_OFFSET 0x0024  /* APBE Mask (CA90-specific) */

/* MCLK register addresses */

#define SAM_MCLK_HSDIV            (SAM_MCLK_BASE + SAM_MCLK_HSDIV_OFFSET)
#define SAM_MCLK_CPUDIV           (SAM_MCLK_BASE + SAM_MCLK_CPUDIV_OFFSET)
#define SAM_MCLK_AHBMASK          (SAM_MCLK_BASE + SAM_MCLK_AHBMASK_OFFSET)
#define SAM_MCLK_APBAMASK         (SAM_MCLK_BASE + SAM_MCLK_APBAMASK_OFFSET)
#define SAM_MCLK_APBBMASK         (SAM_MCLK_BASE + SAM_MCLK_APBBMASK_OFFSET)
#define SAM_MCLK_APBCMASK         (SAM_MCLK_BASE + SAM_MCLK_APBCMASK_OFFSET)
#define SAM_MCLK_APBDMASK         (SAM_MCLK_BASE + SAM_MCLK_APBDMASK_OFFSET)
#define SAM_MCLK_APBEMASK         (SAM_MCLK_BASE + SAM_MCLK_APBEMASK_OFFSET)

/* CPUDIV register values */

#define MCLK_CPUDIV_DIV1          0x01
#define MCLK_CPUDIV_DIV2          0x02
#define MCLK_CPUDIV_DIV4          0x04
#define MCLK_CPUDIV_DIV8          0x08
#define MCLK_CPUDIV_DIV16         0x10
#define MCLK_CPUDIV_DIV32         0x20
#define MCLK_CPUDIV_DIV64         0x40
#define MCLK_CPUDIV_DIV128        0x80

/* AHBMASK register bit definitions */

#define MCLK_AHBMASK_HPB0         (1 << 0)   /* HPB0 */
#define MCLK_AHBMASK_HPB1         (1 << 1)   /* HPB1 */
#define MCLK_AHBMASK_HPB2         (1 << 2)   /* HPB2 */
#define MCLK_AHBMASK_HPB3         (1 << 3)   /* HPB3 */
#define MCLK_AHBMASK_DSU          (1 << 4)   /* DSU */
#define MCLK_AHBMASK_NVMCTRL      (1 << 6)   /* NVMCTRL */
#define MCLK_AHBMASK_CMCC         (1 << 8)   /* CMCC */
#define MCLK_AHBMASK_DMAC         (1 << 9)   /* DMAC */
#define MCLK_AHBMASK_USB          (1 << 10)  /* USB */
#define MCLK_AHBMASK_PAC          (1 << 12)  /* PAC */
#define MCLK_AHBMASK_QSPI         (1 << 13)  /* QSPI */
#define MCLK_AHBMASK_GMAC         (1 << 14)  /* GMAC */
#define MCLK_AHBMASK_SDHC0        (1 << 15)  /* SDHC0 */
#define MCLK_AHBMASK_SDHC1        (1 << 16)  /* SDHC1 */
#define MCLK_AHBMASK_CAN0         (1 << 17)  /* CAN0 */
#define MCLK_AHBMASK_CAN1         (1 << 18)  /* CAN1 */
#define MCLK_AHBMASK_ICM          (1 << 19)  /* ICM */
#define MCLK_AHBMASK_PUKCC        (1 << 20)  /* PUKCC */
#define MCLK_AHBMASK_QSPI2X       (1 << 21)  /* QSPI 2X */
#define MCLK_AHBMASK_NVMCTRL_SMEEPROM (1 << 22) /* NVMCTRL SmartEEPROM */
#define MCLK_AHBMASK_NVMCTRL_CACHE (1 << 23) /* NVMCTRL Cache */
#define MCLK_AHBMASK_HPB4         (1 << 24)  /* HPB4 (for APBE) */

/* APBA mask register bit definitions */

#define MCLK_APBAMASK_PAC         (1 << 0)
#define MCLK_APBAMASK_PM          (1 << 1)
#define MCLK_APBAMASK_MCLK        (1 << 2)
#define MCLK_APBAMASK_RSTC        (1 << 3)
#define MCLK_APBAMASK_OSCCTRL     (1 << 4)
#define MCLK_APBAMASK_OSC32KCTRL  (1 << 5)
#define MCLK_APBAMASK_SUPC        (1 << 6)
#define MCLK_APBAMASK_GCLK        (1 << 7)
#define MCLK_APBAMASK_WDT         (1 << 8)
#define MCLK_APBAMASK_RTC         (1 << 9)
#define MCLK_APBAMASK_EIC         (1 << 10)
#define MCLK_APBAMASK_FREQM       (1 << 11)
#define MCLK_APBAMASK_SERCOM0     (1 << 12)
#define MCLK_APBAMASK_SERCOM1     (1 << 13)
#define MCLK_APBAMASK_TC0         (1 << 14)
#define MCLK_APBAMASK_TC1         (1 << 15)

/* APBB mask register bit definitions */

#define MCLK_APBBMASK_USB         (1 << 0)
#define MCLK_APBBMASK_DSU         (1 << 1)
#define MCLK_APBBMASK_NVMCTRL     (1 << 2)
#define MCLK_APBBMASK_PORT        (1 << 4)
#define MCLK_APBBMASK_EVSYS       (1 << 7)
#define MCLK_APBBMASK_SERCOM2     (1 << 9)
#define MCLK_APBBMASK_SERCOM3     (1 << 10)
#define MCLK_APBBMASK_TCC0        (1 << 11)
#define MCLK_APBBMASK_TCC1        (1 << 12)
#define MCLK_APBBMASK_TC2         (1 << 13)
#define MCLK_APBBMASK_TC3         (1 << 14)
#define MCLK_APBBMASK_RAMECC      (1 << 16)

/* APBC mask register bit definitions */

#define MCLK_APBCMASK_GMAC        (1 << 2)
#define MCLK_APBCMASK_TCC2        (1 << 3)
#define MCLK_APBCMASK_TCC3        (1 << 4)
#define MCLK_APBCMASK_TC4         (1 << 5)
#define MCLK_APBCMASK_TC5         (1 << 6)
#define MCLK_APBCMASK_PDEC        (1 << 7)
#define MCLK_APBCMASK_AC          (1 << 8)
#define MCLK_APBCMASK_AES         (1 << 9)
#define MCLK_APBCMASK_TRNG        (1 << 10)
#define MCLK_APBCMASK_ICM         (1 << 11)
#define MCLK_APBCMASK_QSPI        (1 << 13)
#define MCLK_APBCMASK_CCL         (1 << 14)

/* APBD mask register bit definitions */

#define MCLK_APBDMASK_SERCOM5     (1 << 0)
#define MCLK_APBDMASK_SERCOM6     (1 << 1)
#define MCLK_APBDMASK_SERCOM7     (1 << 2)
#define MCLK_APBDMASK_TCC4        (1 << 4)
#define MCLK_APBDMASK_TC6         (1 << 5)
#define MCLK_APBDMASK_TC7         (1 << 6)
#define MCLK_APBDMASK_ADC0        (1 << 7)
#define MCLK_APBDMASK_ADC1        (1 << 8)
#define MCLK_APBDMASK_DAC         (1 << 9)
#define MCLK_APBDMASK_I2S         (1 << 10)
#define MCLK_APBDMASK_PCC         (1 << 11)

/* APBE mask register bit definitions (CA90-specific) */

#define MCLK_APBEMASK_SERCOM4     (1 << 0)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H */
