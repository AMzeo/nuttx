/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_clockconfig.h
 *
 * PIC32CZ CA90 clock configuration interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_CLOCKCONFIG_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_CLOCKCONFIG_H

#include <nuttx/config.h>
#include "sam_gclk.h"

/* XOSC32K configuration */

struct sam_xosc32_config_s
{
  uint8_t enable     : 1;
  uint8_t highspeed  : 1;
  uint8_t extalen    : 1;
  uint8_t en32k      : 1;
  uint8_t en1k       : 1;
  uint8_t runstdby   : 1;
  uint8_t ondemand   : 1;
  uint8_t cfden      : 1;
  uint8_t cfdeo      : 1;
  uint8_t caliben    : 1;
  uint8_t startup;
  uint8_t calib;
  uint8_t rtcsel;
};

/* XOSC0 configuration (CA90 has one external oscillator: XOSCCTRLA) */

struct sam_xosc_config_s
{
  uint8_t enable     : 1;
  uint8_t extalen    : 1;   /* 0=ext clock, 1=crystal */
  uint8_t runstdby   : 1;
  uint8_t ondemand   : 1;
  uint8_t lowgain    : 1;
  uint8_t enalc      : 1;
  uint8_t cfden      : 1;
  uint8_t swben      : 1;
  uint8_t startup;
  uint32_t xosc_frequency;
};

/* DFLL configuration */

struct sam_dfll_config_s
{
  uint8_t enable     : 1;
  uint8_t runstdby   : 1;
  uint8_t ondemand   : 1;
  uint8_t mode       : 1;   /* 0=open-loop, 1=closed-loop */
  uint8_t stable     : 1;
  uint8_t llaw       : 1;
  uint8_t usbcrm     : 1;
  uint8_t ccdis      : 1;
  uint8_t qldis      : 1;
  uint8_t bplckc     : 1;
  uint8_t waitlock   : 1;
  uint8_t caliben    : 1;
  uint8_t gclklock   : 1;
  uint8_t fcalib;
  uint8_t ccalib;
  uint8_t fstep;
  uint8_t cstep;
  uint8_t gclk;
  uint16_t mul;
};

/* DPLL0/1 configuration */

struct sam_dpll_config_s
{
  uint8_t enable     : 1;
  uint8_t dcoen      : 1;
  uint8_t lbypass    : 1;
  uint8_t wuf        : 1;
  uint8_t runstdby   : 1;
  uint8_t ondemand   : 1;
  uint8_t reflock    : 1;
  uint8_t refclk;            /* 0=GCLK, 1=XOSC32K, 2=XOSC0 */
  uint8_t ltime;
  uint8_t filter;
  uint8_t dcofilter;
  uint8_t gclk;              /* GCLK source (if refclk==0) */
  uint8_t ldrfrac;           /* Loop divider fractional part */
  uint16_t ldrint;           /* Loop divider integer (LDR value) */
  uint16_t div;              /* Clock divider */
};

/* Complete clock configuration */

struct sam_clockconfig_s
{
  uint8_t waitstates;        /* NVM read wait states */
  uint8_t cpudiv;            /* MCLK divider to get CPU frequency */
  uint16_t gclkset1;         /* GCLKs to init before DPLL */
  uint16_t gclkset2;         /* GCLKs to init after DPLL */
  uint32_t cpu_frequency;    /* Resulting CPU frequency */
#if BOARD_HAVE_XOSC32K != 0
  struct sam_xosc32_config_s xosc32k;
#endif
#if BOARD_HAVE_XOSC0 != 0
  struct sam_xosc_config_s xosc0;
#endif
  struct sam_dfll_config_s dfll;
  struct sam_dpll_config_s dpll[2];
  struct sam_gclk_config_s gclk[SAM_GCLK_NGEN];
};

#ifndef __ASSEMBLY__
#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

void sam_clock_configure(const struct sam_clockconfig_s *config);
void sam_clock_initialize(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_CLOCKCONFIG_H */
