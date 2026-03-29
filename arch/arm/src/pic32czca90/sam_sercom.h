/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_sercom.h
 *
 * PIC32CZ CA90 SERCOM interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_SERCOM_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_SERCOM_H

#include <nuttx/config.h>
#include <stdbool.h>
#include "arm_internal.h"
#include "sam_config.h"
#include "sam_periphclks.h"

#ifndef __ASSEMBLY__
#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

void sercom_enable(int sercom);
void sercom_coreclk_configure(int sercom, int gclkgen, bool wrlock);
void sercom_slowclk_configure(int sercom, int gclkgen);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_SERCOM_H */
