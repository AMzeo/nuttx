/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_gclk.c
 *
 * PIC32CZ CA90 Generic Clock Controller
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <assert.h>
#include <nuttx/irq.h>
#include "arm_internal.h"
#include "sam_gclk.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void sam_gclk_waitsyncbusy(uint8_t gclk)
{
  uint32_t gclkbit = GCLK_SYNCBUSY_GENCTRL(gclk);
  while ((getreg32(SAM_GCLK_SYNCBUSY) & gclkbit) != 0)
    {
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gclk_configure
 *
 * Description:
 *   Configure a single GCLK based on settings in the config structure.
 *
 ****************************************************************************/

void sam_gclk_configure(int gclk, const struct sam_gclk_config_s *config)
{
  irqstate_t flags;
  uintptr_t regaddr;
  uint32_t regval;

  regval  = 0;
  regaddr = SAM_GCLK_GENCTRL(gclk);

  if (config->enable)
    {
      /* Select the requested source clock for the generator */

      regval = (uint32_t)(config->source) << GCLK_GENCTRL_SRC_SHIFT;

      if (config->oov)
        {
          regval |= GCLK_GENCTRL_OOV;
        }

      if (config->oe)
        {
          regval |= GCLK_GENCTRL_OE;
        }

      if (config->runstdby)
        {
          regval |= GCLK_GENCTRL_RUNSTDBY;
        }

      /* Set the prescaler division factor */

      if (config->div > 1)
        {
          if (((config->div & (config->div - 1)) == 0))
            {
              /* Power of 2 division */

              uint32_t count = 0;
              uint32_t mask;

              for (mask = 2; mask < (uint32_t)config->div; mask <<= 1)
                {
                  count++;
                }

              regval |= (count << GCLK_GENCTRL_DIV_SHIFT);
              regval |= GCLK_GENCTRL_DIVSEL;
            }
          else
            {
              /* Integer division */

              regval |= ((uint32_t)config->div << GCLK_GENCTRL_DIV_SHIFT);
              regval |= GCLK_GENCTRL_IDC;
            }
        }

      /* Don't disable GCLK0 */

      if (gclk == 0)
        {
          regval |= GCLK_GENCTRL_GENEN;
        }
    }

  /* Configure the generator */

  flags = enter_critical_section();
  putreg32(regval, regaddr);
  sam_gclk_waitsyncbusy(gclk);
  leave_critical_section(flags);
  sam_gclk_waitsyncbusy(gclk);

  if (config->enable)
    {
      /* Enable the clock generator */

      flags    = enter_critical_section();
      regval  |= GCLK_GENCTRL_GENEN;
      putreg32(regval, regaddr);
      sam_gclk_waitsyncbusy(gclk);
      leave_critical_section(flags);
    }
}

/****************************************************************************
 * Name: sam_gclk_chan_enable
 *
 * Description:
 *   Configure and enable a GCLK peripheral channel.
 *
 ****************************************************************************/

void sam_gclk_chan_enable(uint8_t channel, uint8_t srcgen, bool wrlock)
{
  irqstate_t flags;
  uint32_t regaddr;
  uint32_t regval;

  regaddr = SAM_GCLK_PCHCTRL(channel);

  flags = enter_critical_section();
  sam_gclk_chan_disable(channel);

  regval = GCLK_PCHCTRL_GEN(srcgen);
  putreg32(regval, regaddr);

  regval |= GCLK_PCHCTRL_CHEN;

  if (wrlock)
    {
      regval |= GCLK_PCHCTRL_WRTLOCK;
    }

  putreg32(regval, regaddr);

  /* Wait for clock synchronization */

  while ((getreg32(regaddr) & GCLK_PCHCTRL_CHEN) == 0)
    {
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: sam_gclk_chan_disable
 *
 * Description:
 *   Disable a GCLK peripheral channel.
 *
 ****************************************************************************/

void sam_gclk_chan_disable(uint8_t channel)
{
  irqstate_t flags;
  uint32_t regaddr;
  uint32_t regval;

  regaddr = SAM_GCLK_PCHCTRL(channel);

  flags   = enter_critical_section();
  regval  = getreg32(regaddr);
  regval &= ~GCLK_PCHCTRL_CHEN;
  putreg32(regval, regaddr);

  while ((getreg32(regaddr) & GCLK_PCHCTRL_CHEN) != 0)
    {
    }

  leave_critical_section(flags);
}
