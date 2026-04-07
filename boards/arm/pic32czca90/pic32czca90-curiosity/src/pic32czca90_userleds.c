/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/src/pic32czca90_userleds.c
 *
 * User LED control
 *
 * Board: PIC32CZ CA90 Curiosity Ultra (EV16W43A)
 * LED0 (BOARD_LED0): PB21, active LOW — DS70005522C Table 2-11
 * LED1 (BOARD_LED1): PB22, active LOW — DS70005522C Table 2-11
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "sam_port.h"
#include "hardware/sam_pinmap.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint32_t g_ledpins[BOARD_NLEDS] =
{
  PORT_LED0,  /* BOARD_LED0 = 0, PB21 */
  PORT_LED1,  /* BOARD_LED1 = 1, PB22 */
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

uint32_t board_userled_initialize(void)
{
  int i;

  for (i = 0; i < BOARD_NLEDS; i++)
    {
      sam_portconfig(g_ledpins[i]);
    }

  return BOARD_NLEDS;
}

void board_userled(int led, bool ledon)
{
  if ((unsigned)led < BOARD_NLEDS)
    {
      sam_portwrite(g_ledpins[led], !ledon);  /* active LOW: invert */
    }
}

void board_userled_all(uint32_t ledset)
{
  int i;

  for (i = 0; i < BOARD_NLEDS; i++)
    {
      board_userled(i, (ledset & (1 << i)) != 0);
    }
}
