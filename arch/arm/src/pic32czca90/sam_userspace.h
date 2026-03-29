/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_userspace.h
 *
 * PIC32CZ CA90 userspace/MPU initialization prototypes
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_USERSPACE_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_USERSPACE_H

#include <nuttx/config.h>

#ifdef CONFIG_BUILD_PROTECTED
void sam_userspace(void);
void sam_mpu_initialize(void);
#else
#  define sam_userspace()
#  define sam_mpu_initialize()
#endif

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_USERSPACE_H */
