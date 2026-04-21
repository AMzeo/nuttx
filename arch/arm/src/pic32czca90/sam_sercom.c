/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sercom.c
 *
 * PIC32CZ CA90 SERCOM peripheral clock enable.
 *
 * CA90 uses MCLK.CLKMSK[n] registers for peripheral APB clock enables.
 * There are NO APBxMASK registers on CA90 (those are SAMD5x-only).
 *
 * Enable formula: CLKMSK[id/32] |= (1 << (id%32))
 * SERCOM MCLK_ID_APB values from PIC32CZ8110CA80208 DFP:
 *   SERCOM0=31, SERCOM1=32, SERCOM2=33, SERCOM3=34, SERCOM4=35,
 *   SERCOM5=36, SERCOM6=37, SERCOM7=38, SERCOM8=39, SERCOM9=40
 *
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
  GCLK_CHAN_SERCOM4_CORE,   /* SERCOM4 - console */
  GCLK_CHAN_SERCOM5_CORE,   /* SERCOM5 */
  GCLK_CHAN_SERCOM6_CORE,   /* SERCOM6 */
  GCLK_CHAN_SERCOM7_CORE,   /* SERCOM7 */
}; /* PIC32CZCA90_NSERCOM=8: SERCOM0-7 only */

/* MCLK_ID_APB for each SERCOM - used to compute CLKMSK register and bit */

static const uint8_t g_sercom_mclk_id[PIC32CZCA90_NSERCOM] =
{
  MCLK_ID_APB_SERCOM0,   /* 31 -> CLKMSK[0] bit 31 */
  MCLK_ID_APB_SERCOM1,   /* 32 -> CLKMSK[1] bit  0 */
  MCLK_ID_APB_SERCOM2,   /* 33 -> CLKMSK[1] bit  1 */
  MCLK_ID_APB_SERCOM3,   /* 34 -> CLKMSK[1] bit  2 */
  MCLK_ID_APB_SERCOM4,   /* 35 -> CLKMSK[1] bit  3 */
  MCLK_ID_APB_SERCOM5,   /* 36 -> CLKMSK[1] bit  4 */
  MCLK_ID_APB_SERCOM6,   /* 37 -> CLKMSK[1] bit  5 */
  MCLK_ID_APB_SERCOM7,   /* 38 -> CLKMSK[1] bit  6 */
}; /* PIC32CZCA90_NSERCOM=8: SERCOM0-7 only */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sercom_enable
 *
 * Description:
 *   Enable the APB clock to a SERCOM module via MCLK.CLKMSK[].
 *   CA90 has NO APBxMASK registers; CLKMSK[id/32] bit[id%32] is used.
 *
 ****************************************************************************/

void sercom_enable(int sercom)
{
  irqstate_t flags;
  uint8_t    id;
  uint32_t   regval;

  DEBUGASSERT((unsigned)sercom < PIC32CZCA90_NSERCOM);

  id = g_sercom_mclk_id[sercom];

  flags  = enter_critical_section();
  regval = getreg32(SAM_MCLK_CLKMSK(id / 32u));
  regval |= (1u << (id % 32u));
  putreg32(regval, SAM_MCLK_CLKMSK(id / 32u));
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
  /* All SERCOMs share GCLK channel 18 for the slow clock (DFP: SERCOM*_GCLK_ID_SLOW=18) */

  sam_gclk_chan_enable(GCLK_CHAN_SERCOM_SLOW, (uint8_t)gclkgen, false);
}
