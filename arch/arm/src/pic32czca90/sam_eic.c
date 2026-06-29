/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_eic.c
 *
 * PIC32CZ CA90 External Interrupt Controller (EIC) driver.
 *
 * 16 EXTINT channels, each with a dedicated NVIC vector (no software
 * demux needed). Async mode is used — edge detection does not require
 * GCLK, giving lowest ISR latency. GCLK3 (32 kHz) is still connected
 * for debounce/filter functionality when requested.
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_EIC

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <debug.h>

#include "arm_internal.h"
#include "hardware/sam_eic.h"
#include "hardware/sam_mclk.h"
#include "sam_gclk.h"
#include "sam_eic.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void sam_eic_sync_wait(uint32_t mask)
{
  while ((getreg32(SAM_EIC_SYNCBUSY) & mask) != 0)
    {
    }
}

static void eic_module_disable(void)
{
  uint8_t ctrla = getreg8(SAM_EIC_CTRLA);
  ctrla &= ~EIC_CTRLA_ENABLE;
  putreg8(ctrla, SAM_EIC_CTRLA);
  sam_eic_sync_wait(EIC_SYNCBUSY_ENABLE);
}

static void eic_module_enable(void)
{
  uint8_t ctrla = getreg8(SAM_EIC_CTRLA);
  ctrla |= EIC_CTRLA_ENABLE;
  putreg8(ctrla, SAM_EIC_CTRLA);
  sam_eic_sync_wait(EIC_SYNCBUSY_ENABLE);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_eic_initialize
 *
 * Description:
 *   Enable APB clock, connect GCLK, software-reset, and enable the EIC.
 ****************************************************************************/

int sam_eic_initialize(void)
{
  uint32_t regval;

  /* 1. Enable APB clock for EIC (MCLK_ID_APB = 16) */

  regval = getreg32(SAM_MCLK_CLKMSK_ADDR(EIC_MCLK_ID_APB));
  regval |= SAM_MCLK_CLKMSK_BIT(EIC_MCLK_ID_APB);
  putreg32(regval, SAM_MCLK_CLKMSK_ADDR(EIC_MCLK_ID_APB));

  /* 2. Connect GCLK3 (32.768 kHz) to EIC peripheral channel (GCLK_ID=5).
   *    This clock is only needed when filter/debounce is enabled on a
   *    channel; async-mode edges don't use it. Connect it anyway so
   *    filters work if requested later.
   */

  sam_gclk_chan_enable(EIC_GCLK_ID, 3, false);

  /* 3. Software reset — clears all CONFIG/INTENSET/ASYNCH state */

  putreg8(EIC_CTRLA_SWRST, SAM_EIC_CTRLA);
  sam_eic_sync_wait(EIC_SYNCBUSY_SWRST);

  /* 4. Enable the EIC module */

  eic_module_enable();

  return OK;
}

/****************************************************************************
 * Name: sam_eic_configure
 *
 * Description:
 *   Configure one EXTINT channel with edge sense and optional filter.
 *   Sets async mode (no GCLK latency) unless filter is requested.
 *   Enables the interrupt at EIC level (INTENSET) but NOT at NVIC.
 ****************************************************************************/

int sam_eic_configure(uint8_t eirq, uint8_t sense, bool filter)
{
  uint32_t regval;
  uintptr_t config_reg;

  if (eirq >= EIC_NEXTINT)
    {
      return -EINVAL;
    }

  /* Must disable EIC before writing CONFIG registers */

  eic_module_disable();

  /* Set async mode for this channel (unless filter needed — filter
   * requires synchronous GCLK-driven sampling).
   */

  regval = getreg32(SAM_EIC_ASYNCH);
  if (filter)
    {
      regval &= ~EIC_EXTINT(eirq);
    }
  else
    {
      regval |= EIC_EXTINT(eirq);
    }
  putreg32(regval, SAM_EIC_ASYNCH);

  /* Write CONFIG register (CONFIG0 for EXTINT0-7, CONFIG1 for EXTINT8-15) */

  config_reg = (eirq < 8) ? SAM_EIC_CONFIG0 : SAM_EIC_CONFIG1;
  regval = getreg32(config_reg);

  /* Clear existing SENSE + FILTEN for this channel */

  regval &= ~(EIC_CONFIG_SENSE_MASK(eirq) | EIC_CONFIG_FILTEN(eirq));

  /* Set new SENSE value */

  regval |= EIC_CONFIG_SENSE(eirq, sense);

  /* Set FILTEN if requested */

  if (filter)
    {
      regval |= EIC_CONFIG_FILTEN(eirq);
    }

  putreg32(regval, config_reg);

  /* Enable interrupt for this channel at EIC level */

  putreg32(EIC_EXTINT(eirq), SAM_EIC_INTENSET);

  /* Re-enable EIC */

  eic_module_enable();

  return OK;
}

/****************************************************************************
 * Name: sam_eic_disable
 *
 * Description:
 *   Disable one EXTINT channel interrupt and clear its sense config.
 ****************************************************************************/

int sam_eic_disable(uint8_t eirq)
{
  uint32_t regval;
  uintptr_t config_reg;

  if (eirq >= EIC_NEXTINT)
    {
      return -EINVAL;
    }

  /* Disable interrupt at EIC level */

  putreg32(EIC_EXTINT(eirq), SAM_EIC_INTENCLR);

  /* Disable EIC to modify CONFIG */

  eic_module_disable();

  /* Clear SENSE to NONE */

  config_reg = (eirq < 8) ? SAM_EIC_CONFIG0 : SAM_EIC_CONFIG1;
  regval = getreg32(config_reg);
  regval &= ~(EIC_CONFIG_SENSE_MASK(eirq) | EIC_CONFIG_FILTEN(eirq));
  putreg32(regval, config_reg);

  /* Clear async mode bit */

  regval = getreg32(SAM_EIC_ASYNCH);
  regval &= ~EIC_EXTINT(eirq);
  putreg32(regval, SAM_EIC_ASYNCH);

  /* Re-enable EIC (other channels may still be active) */

  eic_module_enable();

  return OK;
}

/****************************************************************************
 * Name: sam_eic_irq_ack
 *
 * Description:
 *   Clear the INTFLAG for the EXTINT that generated the given NuttX IRQ.
 ****************************************************************************/

int sam_eic_irq_ack(int irq)
{
  int eirq = irq - SAM_IRQ_EXTINT0;

  if (eirq < 0 || eirq >= EIC_NEXTINT)
    {
      return -EINVAL;
    }

  /* Write-1-to-clear the flag */

  putreg32(EIC_EXTINT(eirq), SAM_EIC_INTFLAG);

  return OK;
}

#endif /* CONFIG_PIC32CZCA90_EIC */
