/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_autoleds.c
 *
 * Automatic LED control for NuttX events
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "sam_port.h"
#include "hardware/sam_pinmap.h"

#ifdef CONFIG_ARCH_LEDS

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_autoled_initialize
 ****************************************************************************/

void board_autoled_initialize(void)
{
  sam_portconfig(PORT_LED0);
}

/****************************************************************************
 * Name: board_autoled_on
 ****************************************************************************/

void board_autoled_on(int led)
{
  switch (led)
    {
      case LED_STACKCREATED:
        sam_portwrite(PORT_LED0, true);
        break;

      case LED_PANIC:
        sam_portwrite(PORT_LED0, true);
        break;

      default:
        break;
    }
}

/****************************************************************************
 * Name: board_autoled_off
 ****************************************************************************/

void board_autoled_off(int led)
{
  switch (led)
    {
      case LED_PANIC:
        sam_portwrite(PORT_LED0, false);
        break;

      default:
        break;
    }
}

#endif /* CONFIG_ARCH_LEDS */
