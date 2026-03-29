/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_appinit.c
 *
 * Application initialization logic
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/board.h>

#include "pic32czca90-curiosity.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform architecture-specific initialization. This function is invoked
 *   early in the initialization sequence by board_initialize().
 *
 ****************************************************************************/

#ifdef CONFIG_BOARDCTL
int board_app_initialize(uintptr_t arg)
{
  return pic32czca90_bringup();
}
#endif
