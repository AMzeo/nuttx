/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_clockconfig.c
 *
 * PIC32CZ CA90 clock configuration
 *
 * Clock tree for CA90 Curiosity Ultra:
 *   MEMS oscillator -> XOSC0 (24 MHz, XTALEN=0)
 *   XOSC0 -> GCLK5 (div 4 = 6 MHz) -> DPLL0 ref
 *   DPLL0: LDR=49 -> 6 MHz * 50 = 300 MHz
 *   DPLL0 -> GCLK0 -> CPU (300 MHz)
 *   DFLL48M -> GCLK1 (48 MHz) -> USB, peripherals
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "arm_internal.h"
#include "hardware/sam_pm.h"
#include "hardware/sam_supc.h"
#include "hardware/sam_oscctrl.h"
#include "hardware/sam_osc32kctrl.h"
#include "hardware/sam_gclk.h"
#include "hardware/sam_nvmctrl.h"
#include "hardware/sam_mclk.h"
#include "sam_gclk.h"
#include "sam_periphclks.h"

#include <arch/board/board.h>
#include "sam_clockconfig.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct sam_clockconfig_s g_initial_clocking =
{
  .waitstates        = BOARD_FLASH_WAITSTATES,
  .cpudiv            = BOARD_MCLK_CPUDIV,
  .gclkset1          = BOARD_GCLK_SET1,
  .gclkset2          = BOARD_GCLK_SET2,
  .cpu_frequency     = BOARD_CPU_FREQUENCY,
#if BOARD_HAVE_XOSC32K != 0
  .xosc32k           =
    {
      .enable         = BOARD_XOSC32K_ENABLE,
      .highspeed      = BOARD_XOSC32K_HIGHSPEED,
      .extalen        = BOARD_XOSC32K_XTALEN,
      .en32k          = BOARD_XOSC32K_EN32K,
      .en1k           = BOARD_XOSC32K_EN1K,
      .runstdby       = BOARD_XOSC32K_RUNSTDBY,
      .ondemand       = BOARD_XOSC32K_ONDEMAND,
      .cfden          = BOARD_XOSC32K_CFDEN,
      .cfdeo          = BOARD_XOSC32K_CFDEO,
      .caliben        = BOARD_XOSC32K_CALIBEN,
      .startup        = BOARD_XOSC32K_STARTUP,
      .calib          = BOARD_XOSC32K_CALIB,
      .rtcsel         = BOARD_XOSC32K_RTCSEL,
    },
#endif
#if BOARD_HAVE_XOSC0 != 0
  .xosc0             =
    {
      .enable         = BOARD_XOSC0_ENABLE,
      .extalen        = BOARD_XOSC0_XTALEN,
      .runstdby       = BOARD_XOSC0_RUNSTDBY,
      .ondemand       = BOARD_XOSC0_ONDEMAND,
      .lowgain        = BOARD_XOSC0_LOWGAIN,
      .enalc          = BOARD_XOSC0_ENALC,
      .cfden          = BOARD_XOSC0_CFDEN,
      .startup        = BOARD_XOSC0_STARTUP,
      .xosc_frequency = BOARD_XOSC0_FREQUENCY,
    },
#endif
#if BOARD_HAVE_XOSC1 != 0
  .xosc1             =
    {
      .enable         = BOARD_XOSC1_ENABLE,
      .extalen        = BOARD_XOSC1_XTALEN,
      .runstdby       = BOARD_XOSC1_RUNSTDBY,
      .ondemand       = BOARD_XOSC1_ONDEMAND,
      .lowgain        = BOARD_XOSC1_LOWGAIN,
      .enalc          = BOARD_XOSC1_ENALC,
      .cfden          = BOARD_XOSC1_CFDEN,
      .startup        = BOARD_XOSC1_STARTUP,
      .xosc_frequency = BOARD_XOSC1_FREQUENCY,
    },
#endif
  .dfll              =
    {
      .enable         = BOARD_DFLL_ENABLE,
      .runstdby       = BOARD_DFLL_RUNSTDBY,
      .ondemand       = BOARD_DFLL_ONDEMAND,
      .mode           = BOARD_DFLL_MODE,
      .stable         = BOARD_DFLL_STABLE,
      .llaw           = BOARD_DFLL_LLAW,
      .usbcrm         = BOARD_DFLL_USBCRM,
      .ccdis          = BOARD_DFLL_CCDIS,
      .qldis          = BOARD_DFLL_QLDIS,
      .bplckc         = BOARD_DFLL_BPLCKC,
      .waitlock       = BOARD_DFLL_WAITLOCK,
      .caliben        = BOARD_DFLL_CALIBEN,
      .gclklock       = BOARD_DFLL_GCLKLOCK,
      .fcalib         = BOARD_DFLL_FCALIB,
      .ccalib         = BOARD_DFLL_CCALIB,
      .fstep          = BOARD_DFLL_FSTEP,
      .cstep          = BOARD_DFLL_CSTEP,
      .gclk           = BOARD_DFLL_GCLK,
      .mul            = BOARD_DFLL_MUL,
    },
  .dpll              =
    {
      {
        .enable       = BOARD_DPLL0_ENABLE,
        .dcoen        = BOARD_DPLL0_DCOEN,
        .lbypass      = BOARD_DPLL0_LBYPASS,
        .wuf          = BOARD_DPLL0_WUF,
        .runstdby     = BOARD_DPLL0_RUNSTDBY,
        .ondemand     = BOARD_DPLL0_ONDEMAND,
        .reflock      = BOARD_DPLL0_REFLOCK,
        .refclk       = BOARD_DPLL0_REFCLK,
        .ltime        = BOARD_DPLL0_LTIME,
        .filter       = BOARD_DPLL0_FILTER,
        .dcofilter    = BOARD_DPLL0_DCOFILTER,
        .gclk         = BOARD_DPLL0_GCLK,
        .ldrfrac      = BOARD_DPLL0_LDRFRAC,
        .ldrint       = BOARD_DPLL0_LDRINT,
        .div          = BOARD_DPLL0_DIV,
      },
      {
        .enable       = BOARD_DPLL1_ENABLE,
        .dcoen        = BOARD_DPLL1_DCOEN,
        .lbypass      = BOARD_DPLL1_LBYPASS,
        .wuf          = BOARD_DPLL1_WUF,
        .runstdby     = BOARD_DPLL1_RUNSTDBY,
        .ondemand     = BOARD_DPLL1_ONDEMAND,
        .reflock      = BOARD_DPLL1_REFLOCK,
        .refclk       = BOARD_DPLL1_REFCLK,
        .ltime        = BOARD_DPLL1_LTIME,
        .filter       = BOARD_DPLL1_FILTER,
        .dcofilter    = BOARD_DPLL1_DCOFILTER,
        .gclk         = BOARD_DPLL1_GCLK,
        .ldrfrac      = BOARD_DPLL1_LDRFRAC,
        .ldrint       = BOARD_DPLL1_LDRINT,
        .div          = BOARD_DPLL1_DIV,
      },
    },
  .gclk              =
    {
      { BOARD_GCLK0_ENABLE,  BOARD_GCLK0_OOV,  BOARD_GCLK0_OE,
        BOARD_GCLK0_RUNSTDBY,  BOARD_GCLK0_SOURCE,  BOARD_GCLK0_DIV },
      { BOARD_GCLK1_ENABLE,  BOARD_GCLK1_OOV,  BOARD_GCLK1_OE,
        BOARD_GCLK1_RUNSTDBY,  BOARD_GCLK1_SOURCE,  BOARD_GCLK1_DIV },
      { BOARD_GCLK2_ENABLE,  BOARD_GCLK2_OOV,  BOARD_GCLK2_OE,
        BOARD_GCLK2_RUNSTDBY,  BOARD_GCLK2_SOURCE,  BOARD_GCLK2_DIV },
      { BOARD_GCLK3_ENABLE,  BOARD_GCLK3_OOV,  BOARD_GCLK3_OE,
        BOARD_GCLK3_RUNSTDBY,  BOARD_GCLK3_SOURCE,  BOARD_GCLK3_DIV },
      { BOARD_GCLK4_ENABLE,  BOARD_GCLK4_OOV,  BOARD_GCLK4_OE,
        BOARD_GCLK4_RUNSTDBY,  BOARD_GCLK4_SOURCE,  BOARD_GCLK4_DIV },
      { BOARD_GCLK5_ENABLE,  BOARD_GCLK5_OOV,  BOARD_GCLK5_OE,
        BOARD_GCLK5_RUNSTDBY,  BOARD_GCLK5_SOURCE,  BOARD_GCLK5_DIV },
      { BOARD_GCLK6_ENABLE,  BOARD_GCLK6_OOV,  BOARD_GCLK6_OE,
        BOARD_GCLK6_RUNSTDBY,  BOARD_GCLK6_SOURCE,  BOARD_GCLK6_DIV },
      { BOARD_GCLK7_ENABLE,  BOARD_GCLK7_OOV,  BOARD_GCLK7_OE,
        BOARD_GCLK7_RUNSTDBY,  BOARD_GCLK7_SOURCE,  BOARD_GCLK7_DIV },
      { BOARD_GCLK8_ENABLE,  BOARD_GCLK8_OOV,  BOARD_GCLK8_OE,
        BOARD_GCLK8_RUNSTDBY,  BOARD_GCLK8_SOURCE,  BOARD_GCLK8_DIV },
      { BOARD_GCLK9_ENABLE,  BOARD_GCLK9_OOV,  BOARD_GCLK9_OE,
        BOARD_GCLK9_RUNSTDBY,  BOARD_GCLK9_SOURCE,  BOARD_GCLK9_DIV },
      { BOARD_GCLK10_ENABLE, BOARD_GCLK10_OOV, BOARD_GCLK10_OE,
        BOARD_GCLK10_RUNSTDBY, BOARD_GCLK10_SOURCE, BOARD_GCLK10_DIV },
      { BOARD_GCLK11_ENABLE, BOARD_GCLK11_OOV, BOARD_GCLK11_OE,
        BOARD_GCLK11_RUNSTDBY, BOARD_GCLK11_SOURCE, BOARD_GCLK11_DIV },
    },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void sam_xosc32k_configure(const struct sam_xosc32_config_s *config)
{
#if BOARD_HAVE_XOSC32K != 0
  uint16_t regval;

  if (!config->enable)
    {
      return;
    }

  /* Configure XOSC32K */

  regval = 0;

  if (config->extalen)
    {
      regval |= OSC32KCTRL_XOSC32K_XTALEN;
    }

  if (config->en32k)
    {
      regval |= OSC32KCTRL_XOSC32K_EN32K;
    }

  if (config->en1k)
    {
      regval |= OSC32KCTRL_XOSC32K_EN1K;
    }

  if (config->runstdby)
    {
      regval |= OSC32KCTRL_XOSC32K_RUNSTDBY;
    }

  if (config->ondemand)
    {
      regval |= OSC32KCTRL_XOSC32K_ONDEMAND;
    }

  regval |= (config->startup << OSC32KCTRL_XOSC32K_STARTUP_SHIFT);

  /* Enable */

  regval |= OSC32KCTRL_XOSC32K_ENABLE;
  putreg16(regval, SAM_OSC32KCTRL_XOSC32K);

  /* Wait for ready */

  while ((getreg32(SAM_OSC32KCTRL_STATUS) &
          OSC32KCTRL_STATUS_XOSC32KRDY) == 0)
    {
    }

  /* Set RTC clock source */

  putreg8(config->rtcsel, SAM_OSC32KCTRL_RTCCTRL);
#endif
}

static void sam_xosc_configure(int xosc,
                               const struct sam_xosc_config_s *config)
{
  uint32_t regval;
  uintptr_t regaddr;

  if (!config->enable)
    {
      return;
    }

  regaddr = SAM_OSCCTRL_XOSCCTRL(xosc);

  regval = 0;

  if (config->extalen)
    {
      regval |= OSCCTRL_XOSCCTRL_XTALEN;
    }

  if (config->runstdby)
    {
      regval |= OSCCTRL_XOSCCTRL_RUNSTDBY;
    }

  if (config->ondemand)
    {
      regval |= OSCCTRL_XOSCCTRL_ONDEMAND;
    }

  if (config->lowgain)
    {
      regval |= OSCCTRL_XOSCCTRL_LOWBUFGAIN;
    }

  if (config->enalc)
    {
      regval |= OSCCTRL_XOSCCTRL_ENALC;
    }

  if (config->cfden)
    {
      regval |= OSCCTRL_XOSCCTRL_CFDEN;
    }

  regval |= (config->startup << OSCCTRL_XOSCCTRL_STARTUP_SHIFT);

  /* Set current multiplier and reference current based on frequency */

  if (config->xosc_frequency <= 8000000)
    {
      regval |= (3 << OSCCTRL_XOSCCTRL_IMULT_SHIFT);
      regval |= (2 << OSCCTRL_XOSCCTRL_IPTAT_SHIFT);
    }
  else if (config->xosc_frequency <= 16000000)
    {
      regval |= (4 << OSCCTRL_XOSCCTRL_IMULT_SHIFT);
      regval |= (3 << OSCCTRL_XOSCCTRL_IPTAT_SHIFT);
    }
  else if (config->xosc_frequency <= 24000000)
    {
      regval |= (5 << OSCCTRL_XOSCCTRL_IMULT_SHIFT);
      regval |= (3 << OSCCTRL_XOSCCTRL_IPTAT_SHIFT);
    }
  else
    {
      regval |= (6 << OSCCTRL_XOSCCTRL_IMULT_SHIFT);
      regval |= (3 << OSCCTRL_XOSCCTRL_IPTAT_SHIFT);
    }

  /* Enable */

  regval |= OSCCTRL_XOSCCTRL_ENABLE;
  putreg32(regval, regaddr);

  /* Wait for XOSC ready */

  if (xosc == 0)
    {
      while ((getreg32(SAM_OSCCTRL_STATUS) &
              OSCCTRL_STATUS_XOSCRDY0) == 0)
        {
        }
    }
  else
    {
      while ((getreg32(SAM_OSCCTRL_STATUS) &
              OSCCTRL_STATUS_XOSCRDY1) == 0)
        {
        }
    }
}

static void sam_dfll_configure(const struct sam_dfll_config_s *config)
{
  uint32_t regval;

  if (!config->enable)
    {
      return;
    }

  /* Disable DFLL before configuring */

  putreg8(0, SAM_OSCCTRL_DFLLCTRLA);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_ENABLE) != 0)
    {
    }

  /* Set multiplier */

  regval = (config->mul << OSCCTRL_DFLLMUL_MUL_SHIFT) |
           (config->fstep << OSCCTRL_DFLLMUL_FSTEP_SHIFT) |
           (config->cstep << OSCCTRL_DFLLMUL_CSTEP_SHIFT);
  putreg32(regval, SAM_OSCCTRL_DFLLMUL);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_DFLLMUL) != 0)
    {
    }

  /* Configure CTRLB */

  regval = 0;
  if (config->mode)
    {
      regval |= OSCCTRL_DFLLCTRLB_MODE;
    }

  if (config->usbcrm)
    {
      regval |= OSCCTRL_DFLLCTRLB_USBCRM;
    }

  if (config->qldis)
    {
      regval |= OSCCTRL_DFLLCTRLB_QLDIS;
    }

  if (config->ccdis)
    {
      regval |= OSCCTRL_DFLLCTRLB_CCDIS;
    }

  if (config->waitlock)
    {
      regval |= OSCCTRL_DFLLCTRLB_WAITLOCK;
    }

  putreg8(regval, SAM_OSCCTRL_DFLLCTRLB);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_DFLLCTRLB) != 0)
    {
    }

  /* Enable DFLL */

  regval = OSCCTRL_DFLLCTRLA_ENABLE;
  if (config->runstdby)
    {
      regval |= OSCCTRL_DFLLCTRLA_RUNSTDBY;
    }

  if (config->ondemand)
    {
      regval |= OSCCTRL_DFLLCTRLA_ONDEMAND;
    }

  putreg8(regval, SAM_OSCCTRL_DFLLCTRLA);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_ENABLE) != 0)
    {
    }

  /* Wait for DFLL lock if in closed-loop mode */

  if (config->mode)
    {
      while ((getreg32(SAM_OSCCTRL_STATUS) &
              OSCCTRL_STATUS_DFLLRDY) == 0)
        {
        }
    }
}

static void sam_dpll_configure(int dpll,
                               const struct sam_dpll_config_s *config)
{
  uint32_t regval;

  if (!config->enable)
    {
      return;
    }

  /* Disable DPLL before configuring */

  putreg8(0, SAM_OSCCTRL_DPLLCTRLA(dpll));
  while ((getreg32(SAM_OSCCTRL_DPLLSYNCBUSY(dpll)) &
          OSCCTRL_DPLLSYNCBUSY_ENABLE) != 0)
    {
    }

  /* Set DPLL ratio: LDR and LDRFRAC */

  regval = ((uint32_t)config->ldrint << OSCCTRL_DPLLRATIO_LDR_SHIFT) |
           ((uint32_t)config->ldrfrac << OSCCTRL_DPLLRATIO_LDRFRAC_SHIFT);
  putreg32(regval, SAM_OSCCTRL_DPLLRATIO(dpll));
  while ((getreg32(SAM_OSCCTRL_DPLLSYNCBUSY(dpll)) &
          OSCCTRL_DPLLSYNCBUSY_DPLLRATIO) != 0)
    {
    }

  /* Configure CTRLB: reference clock, filter, lock time, divider */

  regval = (config->filter << OSCCTRL_DPLLCTRLB_FILTER_SHIFT) |
           (config->refclk << OSCCTRL_DPLLCTRLB_REFCLK_SHIFT) |
           (config->ltime << OSCCTRL_DPLLCTRLB_LTIME_SHIFT) |
           (config->dcofilter << OSCCTRL_DPLLCTRLB_DCOFILTER_SHIFT) |
           (config->div << OSCCTRL_DPLLCTRLB_DIV_SHIFT);

  if (config->dcoen)
    {
      regval |= OSCCTRL_DPLLCTRLB_DCOEN;
    }

  if (config->lbypass)
    {
      regval |= OSCCTRL_DPLLCTRLB_LBYPASS;
    }

  if (config->wuf)
    {
      regval |= OSCCTRL_DPLLCTRLB_WUF;
    }

  putreg32(regval, SAM_OSCCTRL_DPLLCTRLB(dpll));

  /* Enable DPLL */

  regval = OSCCTRL_DPLLCTRLA_ENABLE;
  if (config->runstdby)
    {
      regval |= OSCCTRL_DPLLCTRLA_RUNSTDBY;
    }

  if (config->ondemand)
    {
      regval |= OSCCTRL_DPLLCTRLA_ONDEMAND;
    }

  putreg8(regval, SAM_OSCCTRL_DPLLCTRLA(dpll));
  while ((getreg32(SAM_OSCCTRL_DPLLSYNCBUSY(dpll)) &
          OSCCTRL_DPLLSYNCBUSY_ENABLE) != 0)
    {
    }

  /* Wait for DPLL lock and clock ready */

  while ((getreg32(SAM_OSCCTRL_DPLLSTATUS(dpll)) &
          (OSCCTRL_DPLLSTATUS_LOCK | OSCCTRL_DPLLSTATUS_CLKRDY)) !=
         (OSCCTRL_DPLLSTATUS_LOCK | OSCCTRL_DPLLSTATUS_CLKRDY))
    {
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void sam_clock_configure(const struct sam_clockconfig_s *config)
{
  int i;

  /* Set flash wait states for target frequency */
  /* CZCA90: No NVMCTRL. FCR manages flash wait states automatically.
 * At 300 MHz with VDDREG=1.8V, FCR sets appropriate wait states. */
  // putreg32(NVMCTRL_CTRLA_RWS(config->waitstates) | NVMCTRL_CTRLA_AUTOWS,
  //          SAM_NVMCTRL_CTRLA);

  /* Configure XOSC32K if needed */

#if BOARD_HAVE_XOSC32K != 0
  sam_xosc32k_configure(&config->xosc32k);
#endif

  /* Configure XOSC0 (MEMS oscillator on CA90 Curiosity Ultra) */

#if BOARD_HAVE_XOSC0 != 0
  sam_xosc_configure(0, &config->xosc0);
#endif

  /* Configure XOSC1 if needed */

#if BOARD_HAVE_XOSC1 != 0
  sam_xosc_configure(1, &config->xosc1);
#endif

  /* Configure GCLK set 1 (needed before DPLL, e.g., GCLK5 feeds DPLL0) */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset1 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }

  /* Configure DFLL48M */

  sam_dfll_configure(&config->dfll);

  /* Configure DPLL0 and DPLL1 */

  sam_dpll_configure(0, &config->dpll[0]);
  sam_dpll_configure(1, &config->dpll[1]);

  /* Configure remaining GCLKs (set 2) */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset2 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }

  /* Set CPU clock divider */

  putreg8(config->cpudiv, SAM_MCLK_CPUDIV);
}

void sam_clock_initialize(void)
{
  sam_clock_configure(&g_initial_clocking);
}
