/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_clockconfig.c
 *
 * PIC32CZ CA90 clock configuration
 *
 * Clock tree for CA90 Curiosity Ultra:
 *   MEMS oscillator Y300 (DSC6011JI2B-012.0000) -> XOSC0 (12 MHz, XTALEN=0)
 *   XOSC0 -> GCLK5 (div 2 = 6 MHz) -> DPLL0 ref (via GCLK_PCHCTRL[1])
 *   DPLL0: LDR=49 -> 6 MHz * 50 = 300 MHz
 *   DPLL0 -> GCLK0 -> CPU (300 MHz)
 *   DFLL48M (open loop) -> GCLK1 (48 MHz) -> USB, peripherals
 *   GCLK5 (6 MHz) -> SERCOM4 core (console UART, stable clock for bringup)
 *
 * FIX 1 (CRITICAL): Added sam_gclk_chan_enable(GCLK_CHAN_DPLL0_REF, ...) 
 *   before sam_dpll_configure(0, ...). Without this the DPLL0 reference
 *   GCLK peripheral channel (PCHCTRL[1]) is never connected to GCLK5.
 *   DPLL0 therefore had no reference and the DPLLSTATUS lock-wait loop
 *   in sam_dpll_configure() blocked forever, halting all boot.
 *
 * FIX 2 (COMMENT): Stale header comment said "XOSC0 (24 MHz)" —
 *   the actual oscillator is 12 MHz (DSC6011JI2B-012.0000). The
 *   BOARD_XOSC0_FREQUENCY define was already correct; only the comment
 *   was wrong. Corrected here.
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

  regval |= OSC32KCTRL_XOSC32K_ENABLE;
  putreg16(regval, SAM_OSC32KCTRL_XOSC32K);

  while ((getreg32(SAM_OSC32KCTRL_STATUS) &
          OSC32KCTRL_STATUS_XOSC32KRDY) == 0)
    {
    }

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

  regval |= OSCCTRL_XOSCCTRL_ENABLE;
  putreg32(regval, regaddr);

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

  putreg8(0, SAM_OSCCTRL_DFLLCTRLA);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_ENABLE) != 0)
    {
    }

  regval = (config->mul << OSCCTRL_DFLLMUL_MUL_SHIFT) |
           (config->fstep << OSCCTRL_DFLLMUL_FSTEP_SHIFT) |
           (config->cstep << OSCCTRL_DFLLMUL_CSTEP_SHIFT);
  putreg32(regval, SAM_OSCCTRL_DFLLMUL);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_DFLLMUL) != 0)
    {
    }

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

  /* NOTE: Do NOT set WAITLOCK in open-loop mode (BOARD_DFLL_MODE=FALSE).
   * In open loop, WAITLOCK=1 can prevent the DFLL from outputting a clock
   * until a (possibly non-occurring) stable condition is met. Keep it 0
   * unless using closed-loop mode where USB lock is guaranteed.
   */

  if (config->waitlock && config->mode)
    {
      regval |= OSCCTRL_DFLLCTRLB_WAITLOCK;
    }

  putreg8(regval, SAM_OSCCTRL_DFLLCTRLB);
  while ((getreg8(SAM_OSCCTRL_DFLLSYNC) & OSCCTRL_DFLLSYNC_DFLLCTRLB) != 0)
    {
    }

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

  /* Only wait for DFLL lock in closed-loop mode */

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

  putreg8(0, SAM_OSCCTRL_DPLLCTRLA(dpll));
  while ((getreg32(SAM_OSCCTRL_DPLLSYNCBUSY(dpll)) &
          OSCCTRL_DPLLSYNCBUSY_ENABLE) != 0)
    {
    }

  regval = ((uint32_t)config->ldrint << OSCCTRL_DPLLRATIO_LDR_SHIFT) |
           ((uint32_t)config->ldrfrac << OSCCTRL_DPLLRATIO_LDRFRAC_SHIFT);
  putreg32(regval, SAM_OSCCTRL_DPLLRATIO(dpll));
  while ((getreg32(SAM_OSCCTRL_DPLLSYNCBUSY(dpll)) &
          OSCCTRL_DPLLSYNCBUSY_DPLLRATIO) != 0)
    {
    }

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

  /* PIC32CZ CA90: No NVMCTRL – FCR manages flash wait states automatically.
   * Do NOT write to SAM_NVMCTRL_CTRLA (that peripheral does not exist here).
   */

  /* 1. Configure XOSC32K if needed */

#if BOARD_HAVE_XOSC32K != 0
  sam_xosc32k_configure(&config->xosc32k);
#endif

  /* 2. Configure XOSC0 (12 MHz MEMS oscillator on CA90 Curiosity Ultra) */

#if BOARD_HAVE_XOSC0 != 0
  sam_xosc_configure(0, &config->xosc0);
#endif

  /* 3. Configure XOSC1 if needed */

#if BOARD_HAVE_XOSC1 != 0
  sam_xosc_configure(1, &config->xosc1);
#endif

  /* 4. Configure GCLK set 1 (runs before DPLLs).
   *    e.g. GCLK5 = XOSC0/2 = 6 MHz, used as DPLL0 reference.
   */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset1 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }

  /* 5. Configure DFLL48M (open-loop for USB CRM) */

  sam_dfll_configure(&config->dfll);

  /* 6. FIX: Route GCLK5 to DPLL0's GCLK reference peripheral channel
   *    (GCLK_PCHCTRL[1] = DPLL0 reference) BEFORE enabling DPLL0.
   *
   *    Without this call, DPLL0 CTRLB.REFCLK=0 (GCLK source selected)
   *    but GCLK_PCHCTRL[1] is never connected, so DPLL0 has no reference
   *    clock and the DPLLSTATUS.LOCK wait loop below hangs forever.
   *
   *    Harmony plib_clk.c equivalent:
   *      GCLK_REGS->GCLK_PCHCTRL[1] = GCLK_PCHCTRL_GEN(5) |
   *                                    GCLK_PCHCTRL_CHEN(1);
   */

  if (config->dpll[0].enable &&
      config->dpll[0].refclk == 0 /* GCLK reference */)
    {
      sam_gclk_chan_enable(GCLK_CHAN_DPLL0_REF,
                           config->dpll[0].gclk,
                           false);
    }

  if (config->dpll[1].enable &&
      config->dpll[1].refclk == 0)
    {
      sam_gclk_chan_enable(GCLK_CHAN_DPLL1_REF,
                           config->dpll[1].gclk,
                           false);
    }

  /* 7. Configure DPLL0 and DPLL1 (reference now connected above) */

  sam_dpll_configure(0, &config->dpll[0]);
  sam_dpll_configure(1, &config->dpll[1]);

  /* 8. Configure remaining GCLKs (set 2: GCLK0 from DPLL0, GCLK1 from
   *    DFLL, GCLK3 from OSCULP32K, etc.)
   */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset2 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }

  /* 9. Set CPU clock divider (MCLK.CPUDIV = 1 → no division) */

  putreg8(config->cpudiv, SAM_MCLK_CPUDIV);
}

void sam_clock_initialize(void)
{
  sam_clock_configure(&g_initial_clocking);
}
