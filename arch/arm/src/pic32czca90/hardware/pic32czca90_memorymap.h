/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/pic32czca90_memorymap.h
 *
 * PIC32CZ CA90 Memory Map Definitions
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_MEMORYMAP_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_MEMORYMAP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* System Memory Map for PIC32CZ CA90 */

#define SAM_CODE_BASE       0x00000000  /* Code address space */
#  define SAM_ITCM_BASE     0x00000000  /* ITCM (64 KB) */
#  define SAM_FLASH_BASE    0x0c000000  /* Internal FLASH (8 MB) */
#  define SAM_QSPI_BASE     0x04000000  /* QSPI memory-mapped */
#define SAM_SRAM_BASE       0x20000000  /* SRAM address space */
#  define SAM_DTCM_BASE     0x20000000  /* DTCM (128 KB) */
#  define SAM_FLEXRAM_BASE  0x20020000  /* FlexRAM */
#  define SAM_SYSRAM_BASE   0x20028000  /* System SRAM */
#define SAM_PERIPH_BASE     0x40000000  /* Peripherals memory space */
#  define SAM_AHBA_BASE     0x40000000  /* AHB-APB Bridge A */
#  define SAM_AHBB_BASE     0x41000000  /* AHB-APB Bridge B */
#  define SAM_AHBC_BASE     0x42000000  /* AHB-APB Bridge C */
#  define SAM_AHBD_BASE     0x43000000  /* AHB-APB Bridge D */
#  define SAM_AHBE_BASE     0x44000000  /* AHB-APB Bridge E (CA90-specific) */
#define SAM_SYSTEM_BASE     0xe0000000  /* System address space */
#  define SAM_SCS_BASE      0xe000e000  /* System Control Space */
#  define SAM_ROMTAB_BASE   0xe000ff00  /* ROM table */

/* NVM area */

#define SAM_NVM_CALIBAREA   0x00800080  /* NVM software calibration area */
#define SAM_NVM_USERPAGE    0x00804000  /* NVM user page */

/* AHB-APB Bridge A - APBA peripherals */

#define SAM_PAC_BASE        0x40000000  /* Peripheral Access Controller */
#define SAM_PM_BASE         0x40000400  /* Power Manager */
#define SAM_MCLK_BASE       0x40000800  /* Main Clock */
#define SAM_RSTC_BASE       0x40000c00  /* Reset Controller */
#define SAM_OSCCTRL_BASE    0x40001000  /* Oscillator Control */
#define SAM_OSC32KCTRL_BASE 0x40001400  /* 32K Oscillator Control */
#define SAM_SUPC_BASE       0x40001800  /* Supply Controller */
#define SAM_GCLK_BASE       0x40001c00  /* Generic Clock Controller */
#define SAM_WDT_BASE        0x40002000  /* Watchdog Timer */
#define SAM_RTC_BASE        0x40002400  /* Real-Time Counter */
#define SAM_EIC_BASE        0x40002800  /* External Interrupt Controller */
#define SAM_FREQM_BASE      0x40002c00  /* Frequency Meter */
#define SAM_SERCOM0_BASE    0x40003000  /* SERCOM0 */
#define SAM_SERCOM1_BASE    0x40003400  /* SERCOM1 */
#define SAM_TC0_BASE        0x40003800  /* Timer/Counter 0 */
#define SAM_TC1_BASE        0x40003c00  /* Timer/Counter 1 */

/* AHB-APB Bridge B - APBB peripherals */

#define SAM_USB_BASE        0x41000000  /* USB */
#define SAM_DSU_BASE        0x41002000  /* Device Service Unit */
#define SAM_NVMCTRL_BASE    0x41004000  /* NVM Controller */
#define SAM_PORT_BASE       0x41008000  /* Ports (GPIO) */
#define SAM_DMAC_BASE       0x4100a000  /* DMA Controller */
#define SAM_EVSYS_BASE      0x4100e000  /* Event System */
#define SAM_SERCOM2_BASE    0x41012000  /* SERCOM2 */
#define SAM_SERCOM3_BASE    0x41014000  /* SERCOM3 */
#define SAM_TCC0_BASE       0x41016000  /* TCC0 */
#define SAM_TCC1_BASE       0x41018000  /* TCC1 */
#define SAM_TC2_BASE        0x4101a000  /* Timer/Counter 2 */
#define SAM_TC3_BASE        0x4101c000  /* Timer/Counter 3 */
#define SAM_RAMECC_BASE     0x41020000  /* RAM ECC */

/* AHB-APB Bridge C - APBC peripherals */

#define SAM_CAN0_BASE       0x42000000  /* CAN0 (MCAN) */
#define SAM_CAN1_BASE       0x42000400  /* CAN1 (MCAN) */
#define SAM_GMAC_BASE       0x42000800  /* Gigabit MAC (Ethernet) */
#define SAM_TCC2_BASE       0x42000c00  /* TCC2 */
#define SAM_TCC3_BASE       0x42001000  /* TCC3 */
#define SAM_TC4_BASE        0x42001400  /* Timer/Counter 4 */
#define SAM_TC5_BASE        0x42001800  /* Timer/Counter 5 */
#define SAM_PDEC_BASE       0x42001c00  /* Position Decoder */
#define SAM_AC_BASE         0x42002000  /* Analog Comparator */
#define SAM_AES_BASE        0x42002400  /* AES */
#define SAM_TRNG_BASE       0x42002800  /* True RNG */
#define SAM_ICM_BASE        0x42002c00  /* Integrity Check Monitor */
#define SAM_PUKCC_BASE      0x42003000  /* PUKCC */
#define SAM_QSPIC_BASE      0x42003400  /* QSPI Controller */
#define SAM_CCL_BASE        0x42003800  /* CCL */

/* AHB-APB Bridge D - APBD peripherals */

#define SAM_SERCOM5_BASE    0x43000000  /* SERCOM5 */
#define SAM_SERCOM6_BASE    0x43000400  /* SERCOM6 */
#define SAM_SERCOM7_BASE    0x43000800  /* SERCOM7 */
#define SAM_TCC4_BASE       0x43001000  /* TCC4 */
#define SAM_TC6_BASE        0x43001400  /* Timer/Counter 6 */
#define SAM_TC7_BASE        0x43001800  /* Timer/Counter 7 */
#define SAM_ADC0_BASE       0x43001c00  /* ADC0 */
#define SAM_ADC1_BASE       0x43002000  /* ADC1 */
#define SAM_DAC_BASE        0x43002400  /* DAC */
#define SAM_I2S_BASE        0x43002800  /* I2S */
#define SAM_PCC_BASE        0x43002c00  /* PCC */

/* AHB-APB Bridge E - APBE peripherals (CA90-specific) */

#define SAM_SERCOM4_BASE    0x44000000  /* SERCOM4 (on APB E, NOT D!) */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_PIC32CZCA90_MEMORYMAP_H */
