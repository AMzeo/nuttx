/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_irq.c
 *
 * PIC32CZ CA90 IRQ initialization
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <arch/irq.h>

#include "nvic.h"
#include "ram_vectors.h"
#include "arm_internal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Get a 32-bit version of the default priority */

#define DEFPRIORITY32 \
  (NVIC_SYSH_PRIORITY_DEFAULT << 24 | \
   NVIC_SYSH_PRIORITY_DEFAULT << 16 | \
   NVIC_SYSH_PRIORITY_DEFAULT << 8  | \
   NVIC_SYSH_PRIORITY_DEFAULT)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_irqinitialize
 ****************************************************************************/

void up_irqinitialize(void)
{
  uint32_t regaddr;
  int num_priority_registers;
  int i;

  /* Disable all interrupts */

  for (i = 0; i < SAM_IRQ_NEXTINT; i += 32)
    {
      putreg32(0xffffffff, NVIC_IRQ_CLEAR(i));
    }

  /* Set all interrupts (and exceptions) to the default priority */

  num_priority_registers = (SAM_IRQ_NEXTINT + 3) / 4;
  for (i = 0; i < num_priority_registers; i++)
    {
      regaddr = NVIC_IRQ_PRIORITY(i);
      putreg32(DEFPRIORITY32, regaddr);
    }

  /* Set the priority of the SVCall, PendSV, and SysTick exceptions */

  putreg32(NVIC_SYSH_PRIORITY_DEFAULT, NVIC_SYSH4_7_PRIORITY);
  putreg32(NVIC_SYSH_PRIORITY_DEFAULT, NVIC_SYSH8_11_PRIORITY);
  putreg32(NVIC_SYSH_PRIORITY_DEFAULT, NVIC_SYSH12_15_PRIORITY);

  /* currents_regs is non-NULL only while processing an interrupt */

  CURRENT_REGS = NULL;

  /* Attach the SVCall and Hard Fault exception handlers */

  irq_attach(SAM_IRQ_SVCALL, arm_svcall, NULL);
  irq_attach(SAM_IRQ_HARDFAULT, arm_hardfault, NULL);

#ifndef CONFIG_SUPPRESS_INTERRUPTS

  /* And finally, enable interrupts */

  up_irq_enable();
#endif
}
