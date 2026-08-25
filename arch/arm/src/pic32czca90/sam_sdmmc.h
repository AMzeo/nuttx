/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sdmmc.h
 *
 * Public interface for the PIC32CZ CA90 SDMMC0 driver.
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_SDMMC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_SDMMC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/sdio.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: sam_sdmmc0_initialize
 *
 * Description:
 *   Initialize SDMMC0 for SD card access.  Enables GCLK4 (100 MHz main
 *   clock), GCLK5 (12 MHz slow clock), and MCLK AHB/APB gates.  Configures
 *   the card-detect GPIO (PC15, active LOW, tied to GND = always inserted).
 *
 * Returned Value:
 *   Pointer to the SDIO device handle, or NULL on failure.
 *
 ****************************************************************************/

EXTERN struct sdio_dev_s *sam_sdmmc0_initialize(void);

/****************************************************************************
 * Name: sam_sdmmc0_slotinitialize
 *
 * Description:
 *   Combined SDMMC0 hardware init + mmcsd_slotinitialize().  Call this from
 *   board_app_initialize() instead of sam_sdmmc0_initialize()+mmcsd_slotinitialize()
 *   separately.
 *
 * Returned Value:
 *   OK on success; negative errno on failure.
 *
 ****************************************************************************/

EXTERN int sam_sdmmc0_slotinitialize(int minor);

/****************************************************************************
 * Name: sdmmc0_clk_enable
 *
 * Description:
 *   Enable GCLK4 / GCLK5 and MCLK AHB / APB gates for SDMMC0.
 *   Called from sam_sdmmc0_initialize(); may also be called after a
 *   deep-sleep resume to re-enable clocks before the first transfer.
 *
 ****************************************************************************/

EXTERN void sdmmc0_clk_enable(void);

/****************************************************************************
 * Name: sam_sdmmc_set_sdio_card_isr
 *
 * Description:
 *   Register a callback for SDIO card interrupts (SDIO device, not card
 *   detect).  Pass func=NULL to disable.
 *
 ****************************************************************************/

EXTERN void sam_sdmmc_set_sdio_card_isr(struct sdio_dev_s *dev,
                                        int (*func)(void *), void *arg);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_SDMMC_H */
