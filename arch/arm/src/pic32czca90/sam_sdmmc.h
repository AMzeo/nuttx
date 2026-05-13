/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sdmmc.h
 *
 * Public interface for the PIC32CZ CA90 SDMMC1 driver.
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
 * Name: sam_sdmmc1_initialize
 *
 * Description:
 *   Initialize SDMMC1 for SD card access.  Enables GCLK4 (100 MHz main
 *   clock), GCLK5 (12 MHz slow clock), and MCLK AHB/APB gates.  Configures
 *   the card-detect GPIO (PC28, active LOW).
 *
 * Returned Value:
 *   Pointer to the SDIO device handle, or NULL on failure.
 *
 ****************************************************************************/

EXTERN struct sdio_dev_s *sam_sdmmc1_initialize(void);

/****************************************************************************
 * Name: sam_sdmmc1_slotinitialize
 *
 * Description:
 *   Combined SDMMC1 hardware init + mmcsd_slotinitialize().  Call this from
 *   board_app_initialize() instead of sam_sdmmc1_initialize()+mmcsd_slotinitialize()
 *   separately.  Placing both calls in the chip layer (libarch.a) ensures
 *   mmcsd_sdio.o is pulled into the --start-group linker block where up_udelay
 *   is available (SAMV7 chip-layer pattern).
 *
 * Returned Value:
 *   OK on success; negative errno on failure.
 *
 ****************************************************************************/

EXTERN int sam_sdmmc1_slotinitialize(int minor);

/****************************************************************************
 * Name: sdmmc1_clk_enable
 *
 * Description:
 *   Enable GCLK4 / GCLK5 and MCLK AHB / APB gates for SDMMC1.
 *   Called from sam_sdmmc1_initialize(); may also be called after a
 *   deep-sleep resume to re-enable clocks before the first transfer.
 *
 ****************************************************************************/

EXTERN void sdmmc1_clk_enable(void);

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
