/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_start.c
 *
 * PIC32CZ CA90 startup code for NuttX
 *
 * Based on arch/arm/src/samd5e5/sam_start.c
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/init.h>

#include "arm_internal.h"
#include "nvic.h"

#include <arch/board/board.h>
#include "sam_clockconfig.h"
#include "sam_lowputc.h"
#include "sam_userspace.h"
#include "sam_start.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IDLE_STACK ((uintptr_t)&_ebss + CONFIG_IDLETHREAD_STACKSIZE)
#define HEAP_BASE  ((uintptr_t)&_ebss + CONFIG_IDLETHREAD_STACKSIZE)

/****************************************************************************
 * Public Data
 ****************************************************************************/

const uintptr_t g_idle_topstack = HEAP_BASE;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_ARMV7M_STACKCHECK
static void sam_stack_color(FAR void *stackbase, size_t nbytes)
{
  uint32_t *stkptr = (uint32_t *)stackbase;
  uintptr_t stkend = (uintptr_t)stackbase + nbytes;
  while ((uintptr_t)stkptr < stkend)
    {
      *stkptr++ = STACK_COLOR;
    }
}
#endif

/****************************************************************************
 * Name: showprogress
 *
 * Description:
 *   Print a character on the UART to show boot status.
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_FEATURES
#  define showprogress(c) arm_lowputc(c)
#else
#  define showprogress(c)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point.
 ****************************************************************************/

void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

  /* Explicitly set VTOR to our flash vector table.
   *
   * CA90 boot sequence: BootROM at 0x04000000 is aliased to 0x00000000 at
   * reset (VTOR=0x00000000).  BootROM reads SP/PC from [0x0C000000]/[4],
   * sets VTOR=0x0C000000, then jumps here.  We re-affirm VTOR here so that:
   *   (a) any early exception (HardFault during clock init) is handled by
   *       our own flash handlers, not the BootROM's handlers at 0x00000000;
   *   (b) this code is correct even if a future BootROM version does not
   *       update VTOR before jumping.
   * Mirrors Harmony startup_xc32.c: SCB->VTOR = (uint32_t)&__svectors.
   *
   * Note: ITCM (128KB, 0x00000000) and DTCM (128KB, 0x20000000) are present
   * on this Cortex-M7.  ITCM is disabled at reset and kept disabled.  DTCM
   * may have been enabled and partially initialized by Boot ROM (DS §7.6.2:
   * "Boot ROM initializes a portion of DTCM to facilitate function") — we
   * never access the DTCM region since all data is in SRAM (0x20020000).
   * With ITCM disabled, VTOR must not point to 0x00000000 — hence this write.
   */

  extern const uint32_t _vectors[];
  putreg32((uint32_t)_vectors, NVIC_VECTAB);

#ifdef CONFIG_ARMV7M_STACKCHECK
  /* Set the stack limit before we attempt to call any functions */

  __asm__ volatile("sub r10, sp, %0" : :
                   "r"(CONFIG_IDLETHREAD_STACKSIZE - 64) :);
#endif

  /* Clear .bss.
   *
   * NOTE: _sbss/_ebss are linker symbols declared as "extern uint32_t".
   * Their ADDRESSES are the BSS region boundaries, not their values.
   * Use &_sbss / &_ebss (address-of) NOT (uint32_t *)_sbss (value-of).
   * See NuttX arm_internal.h comment: "We can recover the linker value
   * then by simply taking the address of _data. like: &_sdata".
   * Reference: arch/arm/src/samd5e5/sam_start.c (identical chip family).
   */

  for (dest = &_sbss; dest < &_ebss; )
    {
      *dest++ = 0;
    }

  /* Move the initialized data section from FLASH to SRAM.
   * The CA90 always boots from flash so this is unconditional.
   */

  for (src = &_eronly, dest = &_sdata; dest < &_edata; )
    {
      *dest++ = *src++;
    }

#ifdef CONFIG_ARCH_RAMFUNCS
  /* Copy any necessary code sections from FLASH to RAM */

  for (src = (const uint32_t *)_framfuncs,
       dest = (uint32_t *)_sramfuncs; dest < (uint32_t *)_eramfuncs; )
    {
      *dest++ = *src++;
    }
#endif

  /* Initialize clocking and the FPU.  Configure the console UART so that
   * we can get debug output as soon as possible.
   */

  sam_clock_initialize();
  arm_fpuconfig();
  sam_lowsetup();
  showprogress('A');

  /* Perform early serial initialization */

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif
  showprogress('B');

  /* For the case of the separate user-/kernel-space build, perform whatever
   * platform specific initialization of the user memory is required.
   * Normally this just means initializing the user space .data and .bss
   * segments.
   */

#ifdef CONFIG_BUILD_PROTECTED
  sam_userspace();
  showprogress('C');
#endif

  /* Initialize on-board resources */

  sam_board_initialize();
  showprogress('D');

#ifdef CONFIG_ARMV7M_ICACHE
  /* Enable I-Cache */

  up_enable_icache();
#endif

#ifdef CONFIG_ARMV7M_DCACHE
  /* Enable D-Cache */

  up_enable_dcache();
#endif

#ifdef CONFIG_ARMV7M_STACKCHECK
  sam_stack_color((FAR void *)((uintptr_t)&_ebss),
                  CONFIG_IDLETHREAD_STACKSIZE);
#endif

  showprogress('E');
  showprogress('\n');

  /* Then start NuttX */

  nx_start();

  /* Shouldn't get here */

  for (; ; );
}
