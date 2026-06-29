/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_eic.h
 *
 * PIC32CZ CA90 External Interrupt Controller (EIC) interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_EIC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_EIC_H

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include "hardware/sam_eic.h"

#ifdef CONFIG_PIC32CZCA90_EIC

#ifndef __ASSEMBLY__
#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: sam_eic_initialize
 *
 * Description:
 *   Initialize the EIC. Called once during system bring-up.
 *   Enables APB clock, GCLK channel, performs software reset, and enables
 *   the EIC module.
 *
 ****************************************************************************/

int sam_eic_initialize(void);

/****************************************************************************
 * Name: sam_eic_configure
 *
 * Description:
 *   Configure edge sensitivity for one EXTINT channel.
 *   The EIC interrupt is enabled at the EIC level (INTENSET) but NOT at NVIC.
 *
 * Input Parameters:
 *   eirq   - EXTINT channel (0..15)
 *   sense  - Edge sense (EIC_SENSE_RISE, EIC_SENSE_FALL, EIC_SENSE_BOTH, etc.)
 *   filter - Enable digital filter (requires GCLK)
 *
 ****************************************************************************/

int sam_eic_configure(uint8_t eirq, uint8_t sense, bool filter);

/****************************************************************************
 * Name: sam_eic_disable
 *
 * Description:
 *   Disable one EXTINT channel at the EIC level.
 *
 ****************************************************************************/

int sam_eic_disable(uint8_t eirq);

/****************************************************************************
 * Name: sam_eic_irq_ack
 *
 * Description:
 *   Acknowledge (clear) an EIC interrupt flag.
 *
 * Input Parameters:
 *   irq - NuttX IRQ number (SAM_IRQ_EXTINT0 .. SAM_IRQ_EXTINT15)
 *
 ****************************************************************************/

int sam_eic_irq_ack(int irq);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* CONFIG_PIC32CZCA90_EIC */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_EIC_H */
