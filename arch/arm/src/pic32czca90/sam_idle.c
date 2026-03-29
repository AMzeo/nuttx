/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_idle.c
 *
 * PIC32CZ CA90 IDLE thread
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/board.h>

#include "arm_internal.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_idle
 *
 * Description:
 *   up_idle() is the logic that will be executed when there is no other
 *   ready-to-run task.  This is processor idle time and will continue until
 *   some interrupt occurs to cause a context switch from the idle task.
 *
 *   Processing in this state may be processor-specific. e.g., this is where
 *   power management operations might be performed.
 *
 ****************************************************************************/

void up_idle(void)
{
#if defined(CONFIG_SUPPRESS_INTERRUPTS) || defined(CONFIG_SUPPRESS_TIMER_INTS)
  /* If the system is idle and there are no timer interrupts, then process
   * "fake" timer interrupts. Necessary for system operation when timer
   * interrupts are disabled.
   */

  nxsched_process_timer();
#else

  /* Sleep until an interrupt occurs to save power */

#ifdef CONFIG_PM
  /* Check power management state and switch to reduced power mode if
   * appropriate.
   */

  board_autoled_off(LED_IDLE);
#endif

  asm("WFI");

#ifdef CONFIG_PM
  board_autoled_on(LED_IDLE);
#endif

#endif
}
