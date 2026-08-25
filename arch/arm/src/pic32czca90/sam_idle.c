/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_idle.c
 *
 * PIC32CZ CA90 IDLE thread
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/board.h>

#include "arm_internal.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void up_idle(void)
{
#if defined(CONFIG_SUPPRESS_INTERRUPTS) || defined(CONFIG_SUPPRESS_TIMER_INTS)
  nxsched_process_timer();
#else

#ifdef CONFIG_PM
  board_autoled_off(LED_IDLE);
#endif

  asm("WFI");

#ifdef CONFIG_PM
  board_autoled_on(LED_IDLE);
#endif

#endif
}
