/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_irq.c
 *
 * PIC32CZ CA90 IRQ initialization
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <assert.h>
#include <errno.h>
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

#define NVIC_ENA_OFFSET    (0)
#define NVIC_CLRENA_OFFSET (NVIC_IRQ0_31_CLEAR - NVIC_IRQ0_31_ENABLE)

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* g_current_regs[] holds a reference to the current interrupt level
 * register storage structure.  It is non-NULL only during interrupt
 * processing.  Access to g_current_regs[] must be through the macro
 * CURRENT_REGS for portability.
 */

volatile uint32_t *g_current_regs[1];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int sam_irqinfo(int irq, uintptr_t *regaddr, uint32_t *bit,
                       int offset)
{
  int n;

  DEBUGASSERT(regaddr != NULL && bit != NULL);

  if (irq >= SAM_IRQ_EXTINT && irq < SAM_IRQ_EXTINT + SAM_IRQ_NEXTINT)
    {
      n        = irq - SAM_IRQ_EXTINT;
      *regaddr = NVIC_IRQ_ENABLE(n) + offset;
      *bit     = 1 << (n & 0x1f);
    }
  else if (irq == SAM_IRQ_MEMFAULT)
    {
      *regaddr = NVIC_SYSHCON;
      *bit     = NVIC_SYSHCON_MEMFAULTENA;
    }
  else if (irq == SAM_IRQ_BUSFAULT)
    {
      *regaddr = NVIC_SYSHCON;
      *bit     = NVIC_SYSHCON_BUSFAULTENA;
    }
  else if (irq == SAM_IRQ_USAGEFAULT)
    {
      *regaddr = NVIC_SYSHCON;
      *bit     = NVIC_SYSHCON_USGFAULTENA;
    }
  else if (irq == SAM_IRQ_SYSTICK)
    {
      *regaddr = NVIC_SYSTICK_CTRL;
      *bit     = NVIC_SYSTICK_CTRL_ENABLE;
    }
  else
    {
      return -EINVAL;
    }

  return OK;
}

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

  putreg32(DEFPRIORITY32, NVIC_SYSH4_7_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH8_11_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH12_15_PRIORITY);

  /* currents_regs is non-NULL only while processing an interrupt */

  CURRENT_REGS = NULL;

  /* Attach the SVCall and Hard Fault exception handlers */

  irq_attach(SAM_IRQ_SVCALL, arm_svcall, NULL);
  irq_attach(SAM_IRQ_HARDFAULT, arm_hardfault, NULL);

#ifdef CONFIG_ARM_MPU
  irq_attach(SAM_IRQ_MEMFAULT, arm_memfault, NULL);
  up_enable_irq(SAM_IRQ_MEMFAULT);
#endif

#ifndef CONFIG_SUPPRESS_INTERRUPTS

  /* And finally, enable interrupts */

  up_irq_enable();
#endif
}

/****************************************************************************
 * Name: up_disable_irq
 ****************************************************************************/

void up_disable_irq(int irq)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (sam_irqinfo(irq, &regaddr, &bit, NVIC_CLRENA_OFFSET) == 0)
    {
      if (irq >= SAM_IRQ_EXTINT)
        {
          putreg32(bit, regaddr);
        }
      else
        {
          regval  = getreg32(regaddr);
          regval &= ~bit;
          putreg32(regval, regaddr);
        }
    }
}

/****************************************************************************
 * Name: up_enable_irq
 ****************************************************************************/

void up_enable_irq(int irq)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (sam_irqinfo(irq, &regaddr, &bit, NVIC_ENA_OFFSET) == 0)
    {
      if (irq >= SAM_IRQ_EXTINT)
        {
          putreg32(bit, regaddr);
        }
      else
        {
          regval  = getreg32(regaddr);
          regval |= bit;
          putreg32(regval, regaddr);
        }
    }
}

/****************************************************************************
 * Name: arm_ack_irq
 ****************************************************************************/

void arm_ack_irq(int irq)
{
}
