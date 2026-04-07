/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_clockconfig.c
 *
 * PIC32CZ CA90 clock configuration
 *
 * Clock tree for CA90 Curiosity Ultra (Harmony-verified, current implementation):
 *
 *   DFLL48M (48 MHz, open-loop, already running at reset — NOT configured in SW)
 *     |
 *   sam_pll0_init(): PLL0 REFSEL=2 (DFLL), REFDIV=12, FBDIV=225, POSTDIV0=3
 *     = 48 / 12 = 4 MHz ref  ×  225 = 900 MHz VCO  /  3 = 300 MHz
 *     |
 *   GCLK0 (SRC=6=PLL0_1, DIV=1) → 300 MHz
 *     |
 *   MCLK.CLKDIV[1] = 2  (written before GCLK0 switch, NEVER restored — Harmony does
 *                         the same)  → CPU effective = 300 / 2 = 150 MHz
 *     BOARD_CPU_FREQUENCY = 150 MHz = DPLL0_FREQUENCY / 2  (SysTick uses this)
 *
 *   GCLK1 (SRC=6=PLL0_1, DIV=2) → 150 MHz → SERCOM1 core clock
 *     └─ SERCOM1 BAUD=64730 → 115200 baud  (Harmony usart_echo_blocking verified)
 *
 *   GCLK3 (SRC=3=OSCULP32K, DIV=1) → 32.768 kHz → SERCOM slow clock, WDT
 *
 *   XOSC0 (12 MHz MEMS, Y300=DSC6011JI2B-012.0000) is present on the board but
 *   NOT used: BOARD_HAVE_XOSC0=0, GCLK5 disabled.  PLL0 references DFLL directly.
 *   All PLL0 bring-up is done by sam_pll0_init() which mirrors Harmony's
 *   PLL0_Initialize().
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

#if BOARD_HAVE_XOSC32K != 0
static void sam_xosc32k_configure(const struct sam_xosc32_config_s *config)
{
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
}
#endif /* BOARD_HAVE_XOSC32K */

#if BOARD_HAVE_XOSC0 != 0
static void sam_xosc_configure(const struct sam_xosc_config_s *config)
{
  uint32_t regval;
  uintptr_t regaddr;

  if (!config->enable)
    {
      return;
    }

  regaddr = SAM_OSCCTRL_XOSCCTRL(0);

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

  while ((getreg32(SAM_OSCCTRL_STATUS) &
          OSCCTRL_STATUS_XOSCRDY0) == 0)
    {
    }
}
#endif /* BOARD_HAVE_XOSC0 */

/****************************************************************************
 * Name: sam_pll0_init
 *
 * Description:
 *   Initialize PLL0 to 300 MHz using DFLL48M as reference.
 *   Sequence verified from Harmony plib_clock.c for CA80/CA90.
 *
 *   Clock math:
 *     DFLL48M (48 MHz) / REFDIV(12) = 4 MHz ref
 *     4 MHz * FBDIV(225) = 900 MHz VCO
 *     900 MHz / POSTDIV0(3) = 300 MHz output
 *
 *   MUST be called before GCLK set2 (after SUPC voltage regulator is ready).
 *
 ****************************************************************************/

static void sam_pll0_init(void)
{
  uint32_t regval;

  /* 1. Enable additional voltage regulator (SUPC.VREGCTRL.AVREGEN=4).
   *    Required for PLL0 operation on CA90.
   */

  regval  = getreg32(SAM_SUPC_VREGCTRL);
  regval &= ~SUPC_VREGCTRL_AVREGEN_MASK;
  regval |= SUPC_VREGCTRL_AVREGEN(4);
  putreg32(regval, SAM_SUPC_VREGCTRL);

  while ((getreg32(SAM_SUPC_STATUS) & SUPC_STATUS_ADDVREGRDY2) == 0)
    {
    }

  /* 2. Disable PLL0 before configuration */

  putreg32(0, SAM_OSCCTRL_PLL0CTRL);

  /* 3. Set reference divider: 48 MHz / 12 = 4 MHz */

  putreg32(12, SAM_OSCCTRL_PLL0REFDIV);

  /* 4. Set feedback divider: 4 MHz * 225 = 900 MHz VCO */

  putreg32(225, SAM_OSCCTRL_PLL0FBDIV);

  /* 5. Clear fractional divider and wait for sync */

  putreg32(0, SAM_OSCCTRL_FRACDIV0);
  while ((getreg32(SAM_OSCCTRL_SYNCBUSY) &
          OSCCTRL_SYNCBUSY_FRACDIV0) != 0)
    {
    }

  /* 6. Set post-divider: 900 MHz / 3 = 300 MHz, enable output */

  putreg32(OSCCTRL_PLL0POSTDIVA_OUTEN0 |
           OSCCTRL_PLL0POSTDIVA_POSTDIV0(3),
           SAM_OSCCTRL_PLL0POSTDIVA);

  /* 7. Enable PLL0: REFSEL=2 (DFLL48M), BWSEL=1, ENABLE */

  putreg32(OSCCTRL_PLL0CTRL_ENABLE |
           OSCCTRL_PLL0CTRL_REFSEL_DFLL |
           OSCCTRL_PLL0CTRL_BWSEL(1),
           SAM_OSCCTRL_PLL0CTRL);

  /* 8. Wait for PLL0 lock */

  while ((getreg32(SAM_OSCCTRL_STATUS) &
          OSCCTRL_STATUS_PLL0LOCK) == 0)
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

  /* 2. Configure XOSC0 if enabled (12 MHz MEMS oscillator, currently unused) */

#if BOARD_HAVE_XOSC0 != 0
  sam_xosc_configure(&config->xosc0);
#endif

  /* 4. Configure GCLK set 1 (generators that must be up before PLL0 starts). */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset1 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }

  /* 5. DFLL48M is already running from reset (DFLLCTRLA RESETVALUE=0x82:
   *    ENABLE=1, ONDEMAND=1). Harmony plib_clock.c never calls DFLL_Initialize()
   *    — it relies on the reset default. Do NOT call sam_dfll_configure() here:
   *    CA90 has no DFLLSYNC register; disabling then re-enabling DFLL before
   *    PLL0 init risks a gap in the reference clock.
   */

  /* 6. Initialize PLL0: DFLL48M → REFSEL=2, REFDIV=12, FBDIV=225, POSTDIV0=3
   *    → 300 MHz.  Harmony-verified sequence (plib_clock.c PLL0_Initialize()).
   */

  sam_pll0_init();

  /* 7. Write MCLK.CLKDIV[1]=BOARD_MCLK_CPUDIV (=2) as Harmony does,
   *    BEFORE switching GCLK0 source to PLL0.
   */

  putreg32(config->cpudiv, SAM_MCLK_CPUDIV);

  /* Wait for clock domain divider to settle before switching GCLK0 to PLL0.
   * Harmony always polls MCLK_INTFLAG.CKRDY here; omitting it risks a brief
   * over-speed fetch when GCLK0 source switches.
   */

  while ((getreg32(SAM_MCLK_INTFLAG) & MCLK_INTFLAG_CKRDY) == 0)
    {
    }

  /* 8. Configure remaining GCLKs (set 2):
   *    GCLK0: SRC=6(PLL0_1), DIV=1 -> 300 MHz CPU
   *    GCLK1: SRC=6(PLL0_1), DIV=2 -> 150 MHz SERCOM1 core clock
   *    GCLK3: SRC=3(OSCULP32K), DIV=1 -> 32.768 kHz SERCOM slow / WDT
   */

  for (i = 0; i < SAM_GCLK_NGEN; i++)
    {
      if ((config->gclkset2 & (1 << i)) != 0)
        {
          sam_gclk_configure(i, &config->gclk[i]);
        }
    }
}

void sam_clock_initialize(void)
{
  sam_clock_configure(&g_initial_clocking);
}
