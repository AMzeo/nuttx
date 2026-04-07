/****************************************************************************
 * arch/arm/src/armv7-m/arm_vectors.c
 *
 *   Copyright (C) 2012 Michael Smith. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"
#include "arm_internal.h"

void my_hardfault(void)
{
  volatile int x = 0;

  while (1)
  {
    x++;
  }
}

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IDLE_STACK      ((unsigned)&_ebss+CONFIG_IDLETHREAD_STACKSIZE)

#ifndef ARMV7M_PERIPHERAL_INTERRUPTS
#  error ARMV7M_PERIPHERAL_INTERRUPTS must be defined to the number of I/O interrupts to be supported
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/* Chip-specific entrypoint */

extern void __start(void);

/* Common exception entrypoint */

extern void exception_common(void);

/****************************************************************************
 * Public data
 ****************************************************************************/

/* The v7m vector table consists of an array of function pointers, with the
 * first slot (vector zero) used to hold the initial stack pointer.
 *
 * As all exceptions (interrupts) are routed via exception_common, we just
 * need to fill this array with pointers to it.
 *
 * Note that the [ ... ] designated initializer is a GCC extension.
 */

unsigned _vectors[] locate_data(".vectors") =
{
  IDLE_STACK,
  (unsigned)&__start,

  /* NMI: keep as infinite loop (should never fire in normal operation) */
  [2] = (unsigned)&my_hardfault,

  /* Fault exceptions: route through exception_common so NuttX's registered
   * handlers run (arm_hardfault/arm_memfault/arm_busfault/arm_usagefault).
   * These are attached via irq_attach() in sam_irq.c and output full
   * diagnostics via PANIC() when CONFIG_DEBUG_HARDFAULT_ALERT=y.
   * Routing to my_hardfault silently loops with no output — unusable for debug.
   */
  [3]  = (unsigned)&exception_common,   /* HardFault */
  [4]  = (unsigned)&exception_common,   /* MemManage */
  [5]  = (unsigned)&exception_common,   /* BusFault */
  [6]  = (unsigned)&exception_common,   /* UsageFault */

  /* Reserved (7-10): silent loop */
  [7 ... 10] = (unsigned)&my_hardfault,

  /* SVC (11): system call, DebugMon (12): reserved/loop,
   * PendSV (14): context switch, SysTick (15): timer tick */
  [11] = (unsigned)&exception_common,   /* SVC — system call */
  [12 ... 13] = (unsigned)&my_hardfault,
  [14] = (unsigned)&exception_common,   /* PendSV — context switch */
  [15] = (unsigned)&exception_common,   /* SysTick — timer tick */

  /* Peripheral IRQs */
  [16 ... (15 + ARMV7M_PERIPHERAL_INTERRUPTS)] = (unsigned)&exception_common
};
