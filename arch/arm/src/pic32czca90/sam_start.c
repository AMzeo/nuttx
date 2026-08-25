/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_start.c
 *
 * PIC32CZ CA90 startup code
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
#include "mpu.h"

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
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point.
 ****************************************************************************/

/* Linker-defined symbols for the DMA nocache region */

extern uint8_t _s_nocache[];
extern uint8_t _e_nocache[];

void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

  /* Set VTOR to the BFM vector table (0x08000000).  The CA90 BootROM reads
   * the reset vector from BFM and sets VTOR before jumping here; we
   * re-affirm it so early exceptions are handled by our own vectors.
   */

  extern const uint32_t _vectors[];
  putreg32((uint32_t)_vectors, NVIC_VECTAB);

#ifdef CONFIG_ARM_MPU_EARLY_RESET
  /* Clear any bootloader MPU configuration before we set up ours. */
  mpu_early_reset();
#endif

#ifdef CONFIG_ARMV7M_STACKCHECK
  /* Set the stack limit before we attempt to call any functions */

  __asm__ volatile("sub r10, sp, %0" : :
                   "r"(CONFIG_IDLETHREAD_STACKSIZE - 64) :);
#endif

  /* Clear .bss */

  for (dest = &_sbss; dest < &_ebss; )
    {
      *dest++ = 0;
    }

  /* Copy .data from flash to SRAM */

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

  sam_clock_initialize();
  arm_fpuconfig();
  sam_lowsetup();

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif

#ifdef CONFIG_BUILD_PROTECTED
  sam_userspace();
#endif

  sam_board_initialize();

#ifdef CONFIG_ARM_MPU
  /* Mark the DMA nocache region (0x200F0000, 64 KB) as Normal,
   * Non-cacheable so the Cortex-M7 D-cache never caches DMA descriptors
   * or buffers placed there by sam_sqi.c, sam_sdmmc.c, sam_dmac.c.
   * TEX=1 C=0 B=0 S=0 = Outer/Inner Non-cacheable Normal memory.
   * PRIVDEFENA=true keeps the default memory map active for all other
   * regions, so flash/SRAM/peripherals are unaffected.
   */
  mpu_configure_region((uintptr_t)_s_nocache,
                       (size_t)(_e_nocache - _s_nocache),
                       MPU_RASR_TEX_NOR |
                       MPU_RASR_AP_RWRW |
                       MPU_RASR_XN);
  mpu_control(true, false, true);
#endif

#ifdef CONFIG_ARMV7M_ICACHE
  up_enable_icache();
#endif

#ifdef CONFIG_ARMV7M_DCACHE
  up_enable_dcache();
#endif

#ifdef CONFIG_ARMV7M_STACKCHECK
  sam_stack_color((FAR void *)((uintptr_t)&_ebss),
                  CONFIG_IDLETHREAD_STACKSIZE);
#endif

  nx_start();

  /* Shouldn't get here */

  for (; ; );
}
