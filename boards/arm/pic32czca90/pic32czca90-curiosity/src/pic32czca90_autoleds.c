/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_autoleds.c
 *
 * Automatic LED control for NuttX events
 *
 * Board: PIC32CZ CA90 Curiosity Ultra (EV16W43A)
 * LED0: PB21, active LOW — DS70005522C Table 2-11
 * LED1: PB22, active LOW — DS70005522C Table 2-11
 *
 * State machine (matches LED_* values in board.h):
 *   0 (STARTED/HEAPALLOCATE/IRQSENABLED) → no change
 *   1 (STACKCREATED)                     → LED0 on  (system booted)
 *   2 (INIRQ/SIGNAL/ASSERTION)           → no change (LED1 is heartbeat)
 *   3 (PANIC)                            → LED0+LED1 blink (fault)
 *
 * LED1 heartbeat: pic32czca90_bringup() starts an LPWORK job that toggles
 * LED1 at 1 Hz.  If LED1 stops blinking, the scheduler has stalled.
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

void board_autoled_initialize(void)
{
  sam_portconfig(PORT_LED0);
  sam_portconfig(PORT_LED1);
}

void board_autoled_on(int led)
{
  switch (led)
    {
      case LED_STACKCREATED:            /* 1: system is running */
        sam_portwrite(PORT_LED0, false);  /* active LOW: drive LOW = on */
        break;

      case LED_INIRQ:                   /* 2: LED1 is heartbeat — no change */
        break;

      case LED_PANIC:                   /* 3: fault — both LEDs blink */
        sam_portwrite(PORT_LED0, false);
        sam_portwrite(PORT_LED1, false);
        break;

      default:
        break;
    }
}

void board_autoled_off(int led)
{
  switch (led)
    {
      case LED_INIRQ:                   /* LED1 is heartbeat — no change */
        break;

      case LED_PANIC:
        sam_portwrite(PORT_LED0, true);
        sam_portwrite(PORT_LED1, true);
        break;

      default:
        break;
    }
}

#endif /* CONFIG_ARCH_LEDS */
