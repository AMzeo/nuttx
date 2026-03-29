/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_boot.c
 *
 * Board initialization logic
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/board.h>

#include "sam_port.h"
#include "hardware/sam_pinmap.h"
#include "pic32czca90-curiosity.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_board_initialize
 *
 * Description:
 *   Called from __start() in sam_start.c after clocking and memory are
 *   configured but before devices are initialized. Configure LEDs and
 *   basic GPIOs here.
 *
 ****************************************************************************/

void sam_board_initialize(void)
{
  /* Configure LED0 (PC21) as output */

  sam_portconfig(PORT_LED0);

#ifdef CONFIG_ARCH_LEDS
  board_autoled_initialize();
#endif
}

/****************************************************************************
 * Name: board_late_initialize
 *
 * Description:
 *   If CONFIG_BOARD_LATE_INITIALIZE is selected, then an additional
 *   initialization call will be performed in the boot-up sequence to a
 *   function called board_late_initialize().
 *
 ****************************************************************************/

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
#ifdef CONFIG_NSH_ARCHINIT
  /* Perform board-specific initialization */

  pic32czca90_bringup();
#endif
}
#endif
