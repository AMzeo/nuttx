/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_irq.c
 *
 * PIC32CZ CA90 IRQ initialization
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <inttypes.h>
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

/* Address of the vector table (from linker script) */

extern uint32_t _vectors[];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_nmi, sam_busfault, sam_usagefault, sam_pendsv, sam_dbgmonitor,
 *       sam_reserved
 *
 * Description:
 *   Handlers for system exceptions.  These output a diagnostic message and
 *   call PANIC().  They are registered under CONFIG_DEBUG_FEATURES only.
 ****************************************************************************/

#ifdef CONFIG_DEBUG_FEATURES
static int sam_nmi(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! NMI received\n");
  PANIC();
  return 0;
}

static int sam_busfault(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! Bus fault received: %08" PRIx32 "\n", getreg32(NVIC_CFAULTS));
  PANIC();
  return 0;
}

static int sam_usagefault(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! Usage fault received: %08" PRIx32 "\n", getreg32(NVIC_CFAULTS));
  PANIC();
  return 0;
}

static int sam_pendsv(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! PendSV received\n");
  PANIC();
  return 0;
}

static int sam_dbgmonitor(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! Debug Monitor received\n");
  PANIC();
  return 0;
}

static int sam_reserved(int irq, void *context, void *arg)
{
  up_irq_save();
  _err("PANIC!!! Reserved interrupt\n");
  PANIC();
  return 0;
}
#endif /* CONFIG_DEBUG_FEATURES */

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
  uintptr_t regaddr;
  int nintlines;
  int i;

  /* The NVIC ICTR register (bits 0-4) holds the number of interrupt lines
   * that the NVIC supports in groups of 32.
   * nintlines = (INTLINESNUM + 1) = number of 32-IRQ groups.
   * For CA90 with 222 IRQs: INTLINESNUM=6, nintlines=7.
   */

  nintlines = (getreg32(NVIC_ICTR) & NVIC_ICTR_INTLINESNUM_MASK) + 1;

  /* Disable all interrupts.  There are nintlines interrupt clear registers. */

  for (i = nintlines, regaddr = NVIC_IRQ0_31_CLEAR;
       i > 0;
       i--, regaddr += 4)
    {
      putreg32(0xffffffff, regaddr);
    }

  /* Affirm the vector table address — BootROM sets this to 0x08000000 before
   * calling __start(), and __start() writes it again as its very first
   * instruction.  Writing it a third time here (following SAMV7 practice)
   * ensures it is correct by the time any exception can be dispatched.
   */

  putreg32((uint32_t)_vectors, NVIC_VECTAB);

  /* Set all system handler exceptions to the default priority */

  putreg32(DEFPRIORITY32, NVIC_SYSH4_7_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH8_11_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH12_15_PRIORITY);

  /* Set all peripheral IRQ priorities.  There are nintlines * 8 priority
   * registers (each register covers 4 IRQs × 8-bit priority fields).
   */

  for (i = (nintlines << 3), regaddr = NVIC_IRQ0_3_PRIORITY;
       i > 0;
       i--, regaddr += 4)
    {
      putreg32(DEFPRIORITY32, regaddr);
    }

  /* currents_regs is non-NULL only while processing an interrupt */

  CURRENT_REGS = NULL;

  /* Attach the SVCall and Hard Fault exception handlers */

  irq_attach(SAM_IRQ_SVCALL, arm_svcall, NULL);
  irq_attach(SAM_IRQ_HARDFAULT, arm_hardfault, NULL);

#ifdef CONFIG_ARM_MPU
  irq_attach(SAM_IRQ_MEMFAULT, arm_memfault, NULL);
  up_enable_irq(SAM_IRQ_MEMFAULT);
#endif

  /* Attach diagnostic handlers for all other system exceptions.
   * These call PANIC() with an error message rather than silently looping,
   * which makes root-cause analysis possible when an unexpected exception
   * fires.  Enabled only under CONFIG_DEBUG_FEATURES to avoid overhead in
   * release builds.
   */

#ifdef CONFIG_DEBUG_FEATURES
  irq_attach(SAM_IRQ_NMI, sam_nmi, NULL);
#  ifndef CONFIG_ARM_MPU
  irq_attach(SAM_IRQ_MEMFAULT, arm_memfault, NULL);
#  endif
  irq_attach(SAM_IRQ_BUSFAULT, sam_busfault, NULL);
  irq_attach(SAM_IRQ_USAGEFAULT, sam_usagefault, NULL);
  irq_attach(SAM_IRQ_PENDSV, sam_pendsv, NULL);
  irq_attach(SAM_IRQ_DBGMONITOR, sam_dbgmonitor, NULL);
  irq_attach(SAM_IRQ_RESERVED, sam_reserved, NULL);
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

/****************************************************************************
 * Name: up_prioritize_irq
 *
 * Description:
 *   Set the priority of an IRQ.
 *
 *   Since this API is not supported on all architectures, it should be
 *   avoided in common implementations where possible.
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_IRQPRIO
int up_prioritize_irq(int irq, int priority)
{
  uint32_t regaddr;
  uint32_t regval;
  int shift;

  DEBUGASSERT(irq >= SAM_IRQ_MEMFAULT && irq < NR_IRQS &&
              (unsigned)priority <= NVIC_SYSH_PRIORITY_MIN);

  if (irq < SAM_IRQ_EXTINT)
    {
      regaddr = NVIC_SYSH_PRIORITY(irq);
      irq    -= 4;
    }
  else
    {
      irq    -= SAM_IRQ_EXTINT;
      regaddr = NVIC_IRQ_PRIORITY(irq);
    }

  regval      = getreg32(regaddr);
  shift       = ((irq & 3) << 3);
  regval     &= ~(0xff << shift);
  regval     |= (priority << shift);
  putreg32(regval, regaddr);

  return OK;
}
#endif
