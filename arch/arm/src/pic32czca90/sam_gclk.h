/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_gclk.h
 *
 * PIC32CZ CA90 Generic Clock Controller interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_GCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_GCLK_H

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include "arm_internal.h"
#include "sam_config.h"
#include "hardware/sam_gclk.h"

#ifndef __ASSEMBLY__

/* This structure describes the configuration of one GCLK */

struct sam_gclk_config_s
{
  uint8_t enable     : 1;    /* True: Enable GCLK */
  uint8_t oov        : 1;    /* True: Clock output selection */
  uint8_t oe         : 1;    /* True: Output enable */
  uint8_t runstdby   : 1;    /* True: Run in standby */
  uint8_t source;            /* GCLK source (see GCLK_GENCTRL_SRC_*) */
  uint16_t div;              /* Division factor */
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

void sam_gclk_configure(int gclk,
                        const struct sam_gclk_config_s *config);
void sam_gclk_chan_enable(uint8_t channel, uint8_t srcgen, bool wrlock);
void sam_gclk_chan_disable(uint8_t channel);

static inline bool sam_gclk_chan_locked(uint8_t channel)
{
  uint32_t regaddr = SAM_GCLK_PCHCTRL(channel);
  uint32_t regval  = getreg32(regaddr);
  return (regval & GCLK_PCHCTRL_WRTLOCK) != 0;
}

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_GCLK_H */
