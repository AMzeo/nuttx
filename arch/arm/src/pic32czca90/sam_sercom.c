/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sercom.c
 *
 * PIC32CZ CA90 SERCOM peripheral clock enable
 *
 * CRITICAL: SERCOM4 is on APB bus E (APBEMASK), NOT bus D.
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <assert.h>
#include <nuttx/irq.h>
#include "arm_internal.h"
#include "sam_gclk.h"
#include "sam_sercom.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_gclk.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* GCLK peripheral channel index for each SERCOM core clock */

static const uint8_t g_sercom_gclk_chan[PIC32CZCA90_NSERCOM] =
{
  GCLK_CHAN_SERCOM0_CORE,   /* SERCOM0 */
  GCLK_CHAN_SERCOM1_CORE,   /* SERCOM1 */
  GCLK_CHAN_SERCOM2_CORE,   /* SERCOM2 */
  GCLK_CHAN_SERCOM3_CORE,   /* SERCOM3 */
  GCLK_CHAN_SERCOM4_CORE,   /* SERCOM4 */
  GCLK_CHAN_SERCOM5_CORE,   /* SERCOM5 */
  GCLK_CHAN_SERCOM6_CORE,   /* SERCOM6 */
  GCLK_CHAN_SERCOM7_CORE,   /* SERCOM7 */
  GCLK_CHAN_SERCOM8_CORE,   /* SERCOM8 */
  GCLK_CHAN_SERCOM9_CORE,   /* SERCOM9 */
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sercom_enable
 *
 * Description:
 *   Enable clocking to a SERCOM module via MCLK APB masks.
 *   IMPORTANT: SERCOM4 is on APB bus E (not D) on PIC32CZ CA90.
 *
 ****************************************************************************/

void sercom_enable(int sercom)
{
  irqstate_t flags;
  uint32_t regval;

  flags = enter_critical_section();

  switch (sercom)
    {
      case 0: /* SERCOM0 - APB A */
        regval  = getreg32(SAM_MCLK_APBEMASK);
        regval |= MCLK_APBEMASK_SERCOM0;
        putreg32(regval, SAM_MCLK_APBEMASK);
        break;

      case 1: /* SERCOM1 - APB A */
        regval  = getreg32(SAM_MCLK_APBEMASK);
        regval |= MCLK_APBEMASK_SERCOM1;
        putreg32(regval, SAM_MCLK_APBEMASK);
        break;

      case 2: /* SERCOM2 - APB B */
        regval  = getreg32(SAM_MCLK_APBDMASK);
        regval |= MCLK_APBDMASK_SERCOM2;
        putreg32(regval, SAM_MCLK_APBDMASK);
        break;

      case 3: /* SERCOM3 - APB B */
        regval  = getreg32(SAM_MCLK_APBDMASK);
        regval |= MCLK_APBDMASK_SERCOM3;
        putreg32(regval, SAM_MCLK_APBDMASK);
        break;

      case 4: /* SERCOM4 - APB **E** (CA90-specific!) */
        regval  = getreg32(SAM_MCLK_APBEMASK);
        regval |= MCLK_APBEMASK_SERCOM4;
        putreg32(regval, SAM_MCLK_APBEMASK);
        break;

      case 5: /* SERCOM5 - APB D */
        regval  = getreg32(SAM_MCLK_APBDMASK);
        regval |= MCLK_APBDMASK_SERCOM5;
        putreg32(regval, SAM_MCLK_APBDMASK);
        break;

      case 6: /* SERCOM6 - APB D */
        regval  = getreg32(SAM_MCLK_APBDMASK);
        regval |= MCLK_APBDMASK_SERCOM6;
        putreg32(regval, SAM_MCLK_APBDMASK);
        break;

      case 7: /* SERCOM7 - APB C */
        regval  = getreg32(SAM_MCLK_APBCMASK);
        regval |= MCLK_APBCMASK_SERCOM7;
        putreg32(regval, SAM_MCLK_APBCMASK);
        break;

      case 8: /* SERCOM8 - APB C */
        regval  = getreg32(SAM_MCLK_APBCMASK);
        regval |= MCLK_APBCMASK_SERCOM8;
        putreg32(regval, SAM_MCLK_APBCMASK);
        break;

      case 9: /* SERCOM9 - APB C */
        regval  = getreg32(SAM_MCLK_APBCMASK);
        regval |= MCLK_APBCMASK_SERCOM9;
        putreg32(regval, SAM_MCLK_APBCMASK);
        break;

      default:
        break;
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: sercom_coreclk_configure
 *
 * Description:
 *   Route a GCLK generator to the SERCOM core clock channel.
 *
 ****************************************************************************/

void sercom_coreclk_configure(int sercom, int gclkgen, bool wrlock)
{
  DEBUGASSERT((unsigned)sercom < PIC32CZCA90_NSERCOM);
  sam_gclk_chan_enable(g_sercom_gclk_chan[sercom],
                       (uint8_t)gclkgen, wrlock);
}

/****************************************************************************
 * Name: sercom_slowclk_configure
 *
 * Description:
 *   Route a GCLK generator to the SERCOM slow clock (shared channel 3).
 *
 ****************************************************************************/

void sercom_slowclk_configure(int sercom, int gclkgen)
{
  /* All SERCOMs share GCLK channel 3 for the slow clock */

  sam_gclk_chan_enable(3, (uint8_t)gclkgen, false);
}
