/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_start.h
 *
 * PIC32CZ CA90 start-up function prototypes
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_START_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_START_H

#include <nuttx/config.h>

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_board_initialize
 *
 * Description:
 *   All PIC32CZ CA90 architectures must provide the following entry point.
 *   This entry point is called early in the initialization -- after
 *   clocking and memory have been configured but before caches have been
 *   enabled and before any devices have been initialized.
 ****************************************************************************/

void sam_board_initialize(void);

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_START_H */
