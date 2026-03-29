/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_periphclks.h
 *
 * PIC32CZ CA90 peripheral clock enable/disable macros via MCLK APB masks
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_PERIPHCLKS_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_PERIPHCLKS_H

#include <nuttx/config.h>
#include "hardware/sam_mclk.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Helper macros to enable/disable peripheral clocks via MCLK APB masks.
 *
 * PIC32CZ CA90 has APB buses A through E:
 *   APBA: PAC, PM, MCLK, RSTC, OSCCTRL, OSC32KCTRL, SUPC, GCLK, WDT,
 *         RTC, EIC, FREQM, SERCOM0, SERCOM1, TC0, TC1
 *   APBB: USB, DSU, NVMCTRL, PORT, DMAC, EVSYS, SERCOM2, SERCOM3,
 *         TCC0, TCC1, TC2, TC3, RAMECC
 *   APBC: GMAC, TCC2, TCC3, TC4, TC5, PDEC, AC, AES, TRNG, ICM,
 *         QSPI, CCL
 *   APBD: SERCOM4, SERCOM5, SERCOM6, SERCOM7, TCC4, TC6, TC7,
 *         ADC0, ADC1, DAC, I2S, PCC
 *   APBE: SERCOM4_ALT (CA90 has SERCOM4 accessible on APBE as well)
 *
 * Note: On the CA90, SERCOM4 is on APB bus E, not D.
 * This is a key difference from SAMD5E5.
 */

/* AHB clock enable/disable */

#define sam_ahb_enableperiph(mask) \
  modifyreg32(SAM_MCLK_AHBMASK, 0, (mask))
#define sam_ahb_disableperiph(mask) \
  modifyreg32(SAM_MCLK_AHBMASK, (mask), 0)

/* APBA clock enable/disable */

#define sam_apba_enableperiph(mask) \
  modifyreg32(SAM_MCLK_APBAMASK, 0, (mask))
#define sam_apba_disableperiph(mask) \
  modifyreg32(SAM_MCLK_APBAMASK, (mask), 0)

/* APBB clock enable/disable */

#define sam_apbb_enableperiph(mask) \
  modifyreg32(SAM_MCLK_APBBMASK, 0, (mask))
#define sam_apbb_disableperiph(mask) \
  modifyreg32(SAM_MCLK_APBBMASK, (mask), 0)

/* APBC clock enable/disable */

#define sam_apbc_enableperiph(mask) \
  modifyreg32(SAM_MCLK_APBCMASK, 0, (mask))
#define sam_apbc_disableperiph(mask) \
  modifyreg32(SAM_MCLK_APBCMASK, (mask), 0)

/* APBD clock enable/disable */

#define sam_apbd_enableperiph(mask) \
  modifyreg32(SAM_MCLK_APBDMASK, 0, (mask))
#define sam_apbd_disableperiph(mask) \
  modifyreg32(SAM_MCLK_APBDMASK, (mask), 0)

/* APBE clock enable/disable - CA90 specific (SERCOM4 lives here) */

#define sam_apbe_enableperiph(mask) \
  modifyreg32(SAM_MCLK_APBEMASK, 0, (mask))
#define sam_apbe_disableperiph(mask) \
  modifyreg32(SAM_MCLK_APBEMASK, (mask), 0)

/* Convenience macros for specific peripherals */

/* SERCOM clock enables */

#define sam_sercom0_enableperiph()   sam_apba_enableperiph(MCLK_APBAMASK_SERCOM0)
#define sam_sercom1_enableperiph()   sam_apba_enableperiph(MCLK_APBAMASK_SERCOM1)
#define sam_sercom2_enableperiph()   sam_apbb_enableperiph(MCLK_APBBMASK_SERCOM2)
#define sam_sercom3_enableperiph()   sam_apbb_enableperiph(MCLK_APBBMASK_SERCOM3)
#define sam_sercom4_enableperiph()   sam_apbe_enableperiph(MCLK_APBEMASK_SERCOM4)
#define sam_sercom5_enableperiph()   sam_apbd_enableperiph(MCLK_APBDMASK_SERCOM5)
#define sam_sercom6_enableperiph()   sam_apbd_enableperiph(MCLK_APBDMASK_SERCOM6)
#define sam_sercom7_enableperiph()   sam_apbd_enableperiph(MCLK_APBDMASK_SERCOM7)

#define sam_sercom0_disableperiph()  sam_apba_disableperiph(MCLK_APBAMASK_SERCOM0)
#define sam_sercom1_disableperiph()  sam_apba_disableperiph(MCLK_APBAMASK_SERCOM1)
#define sam_sercom2_disableperiph()  sam_apbb_disableperiph(MCLK_APBBMASK_SERCOM2)
#define sam_sercom3_disableperiph()  sam_apbb_disableperiph(MCLK_APBBMASK_SERCOM3)
#define sam_sercom4_disableperiph()  sam_apbe_disableperiph(MCLK_APBEMASK_SERCOM4)
#define sam_sercom5_disableperiph()  sam_apbd_disableperiph(MCLK_APBDMASK_SERCOM5)
#define sam_sercom6_disableperiph()  sam_apbd_disableperiph(MCLK_APBDMASK_SERCOM6)
#define sam_sercom7_disableperiph()  sam_apbd_disableperiph(MCLK_APBDMASK_SERCOM7)

/* Port (GPIO) clock enable */

#define sam_port_enableperiph()      sam_apbb_enableperiph(MCLK_APBBMASK_PORT)

/* USB clock enable */

#define sam_usb_enableperiph()       sam_apbb_enableperiph(MCLK_APBBMASK_USB)
#define sam_usb_disableperiph()      sam_apbb_disableperiph(MCLK_APBBMASK_USB)

/* GMAC (Ethernet) clock enable */

#define sam_gmac_enableperiph()      sam_apbc_enableperiph(MCLK_APBCMASK_GMAC)
#define sam_gmac_disableperiph()     sam_apbc_disableperiph(MCLK_APBCMASK_GMAC)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_PERIPHCLKS_H */
