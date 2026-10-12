/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_progmem.c
 *
 * PIC32CZ CA90 program flash MTD lower-half — NuttX up_progmem_* API.
 *
 * Hardware: FCW (Flash Control Write) controller at 0x44002000.
 * Flash geometry (DFP-verified, sam_fcw.h):
 *   PFM base: 0x0C000000, 8 MB
 *   Erase unit (page):  4096 bytes  — FCW_NVMOP_PGERA
 *   Write unit (row):   1024 bytes  — FCW_NVMOP_ROWP (from SRCADDR → RAM buf)
 *
 * The PROGMEM region occupies CONFIG_PIC32CZCA90_PROGMEM_NPAGES pages at the
 * TOP of PFM, leaving the lower flash untouched for code.
 *
 * FCW key discipline:
 *   Write FCW_KEY_WRKEY to SAM_FCW_KEY IMMEDIATELY before writing SAM_FCW_CTRLA.
 *   No other register access may appear between the two writes.
 *   enter_critical_section() + leave_critical_section() wraps just this pair
 *   to prevent an IRQ from consuming the key.
 *
 * D-cache discipline:
 *   After every FCW erase or write, call up_invalidate_dcache() for the
 *   affected range to evict stale XIP cache lines.  Failure to do this is the
 *   class of bug that destroyed SQI params (see Key Bug History in CLAUDE.md).
 *
 * Reference: arch/arm/src/samv7/sam_progmem.c (structure only — EEFC≠FCW).
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_PROGMEM

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/progmem.h>

#include "arm_internal.h"
#include "hardware/sam_fcw.h"
#include "hardware/sam_mclk.h"
#include "sam_progmem.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PROGMEM_PAGE_SIZE   SAM_FCW_PAGE_SIZE   /* 4096 — FCW erase unit */
#define PROGMEM_ROW_SIZE    SAM_FCW_ROW_SIZE    /* 1024 — FCW write unit */
#define PROGMEM_NPAGES      CONFIG_PIC32CZCA90_PROGMEM_NPAGES
#define PROGMEM_PFM_END     (SAM_FCW_PFM_BASE + SAM_FCW_FLASH_SIZE)
#define PROGMEM_START       (PROGMEM_PFM_END - ((uintptr_t)PROGMEM_NPAGES * PROGMEM_PAGE_SIZE))
#define PROGMEM_SIZE        ((size_t)PROGMEM_NPAGES * PROGMEM_PAGE_SIZE)

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Row-program source buffer — must live in SRAM.
 * FCW reads via AHB; placing this in PFM would deadlock the flash bus. */

static uint8_t g_row_buf[PROGMEM_ROW_SIZE] __attribute__((aligned(4)));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void fcw_wait_ready(void)
{
  while (getreg32(SAM_FCW_STATUS) & FCW_STATUS_BUSY)
    {
    }
}

static int fcw_check_errors(void)
{
  uint32_t flags = getreg32(SAM_FCW_INTFLAG);

  if (flags & FCW_INTFLAG_ERRMASK)
    {
      putreg32(FCW_INTFLAG_ERRMASK, SAM_FCW_INTFLAG);   /* W1C */
      return -EIO;
    }

  return OK;
}

/****************************************************************************
 * Public Functions — sam_progmem_initialize
 ****************************************************************************/

void sam_progmem_initialize(void)
{
  uint32_t r;

  /* Enable FCW AHB clock (MCLK_ID_AHB_FCW = 2 → CLKMSK[0] bit 2) */

  r  = getreg32(SAM_MCLK_CLKMSK_ADDR(MCLK_ID_AHB_FCW));
  r |= SAM_MCLK_CLKMSK_BIT(MCLK_ID_AHB_FCW);
  putreg32(r, SAM_MCLK_CLKMSK_ADDR(MCLK_ID_AHB_FCW));

  /* Enable FCW APB clock (MCLK_ID_APB_FCW = 3 → CLKMSK[0] bit 3) */

  r  = getreg32(SAM_MCLK_CLKMSK_ADDR(MCLK_ID_APB_FCW));
  r |= SAM_MCLK_CLKMSK_BIT(MCLK_ID_APB_FCW);
  putreg32(r, SAM_MCLK_CLKMSK_ADDR(MCLK_ID_APB_FCW));

  /* Detect and recover a corrupted PROGMEM region.
   *
   * After a crash loop (many writes without erase), FCW ROWP ANDs each new
   * byte with existing flash.  All bits eventually get driven to 0.  The
   * progmem_dump_probe() called from board_hardfault_init() cannot recover:
   * it tries to write clean headers via up_progmem_write() but FCW ROWP can
   * only clear bits, not restore them to 1.  Result: every file header has
   * fileno=0 and len=0.  progmem_dump_savepanic() then checks
   * progmem_check_erase(&pf->lastwrite) — lastwrite is 0x00 (not 0xFF) —
   * and skips writing crash data.  The dump is silently lost.
   *
   * Detection: progmemfh_s.len is at offset 4 in the first header.  It is:
   *   0xFFFFFFFF   on a freshly erased page (safe — probe will init it)
   *   >0           on a validly initialised file (file data size e.g. 384)
   *   0            ONLY when all bits have been driven to 0 (corrupted)
   *
   * Recovery: erase all PROGMEM pages; probe will write correct headers.
   */

  /* Invalidate D-cache for the ENTIRE PROGMEM region before reading.
   * A chip-erase via MPLAB IPE clears flash to 0xFF but does not touch the
   * D-cache.  Without this invalidate, stale cache lines from a previous
   * crash-loop session remain and mask the actual flash state. */

  up_invalidate_dcache(PROGMEM_START, PROGMEM_START + PROGMEM_SIZE);

  {
    /* File layout (computed from PROGMEM_FILE_SIZES + PROGMEM_DUMP_ALIGNMENT):
     *   File 0 header: PROGMEM_START + 0      (alloc 2048)
     *   File 1 header: PROGMEM_START + 2048   (alloc 2048)
     *   File 2 header: PROGMEM_START + 4096   (alloc 2048)
     *   File 3 header: PROGMEM_START + 6144   (alloc 6144 with NPAGES=3, crash dump)
     *
     * Valid states for each header's fileno field:
     *   0xFFFFFFFF  fresh erase — probe will write correct values
     *   == i        correctly initialised by probe on a prior boot
     *   anything else: corrupted by ROWP AND-writes without prior erase
     *
     * We check all four headers; any corruption triggers a full erase so
     * probe can write clean headers on this boot.
     */

    static const uint32_t offsets[4] = { 0u, 2048u, 4096u, 6144u };
    bool need_erase = false;
    size_t i;

    for (i = 0; i < 4u; i++)
      {
        uint32_t fn  = getreg32(PROGMEM_START + offsets[i] + 0u);
        uint32_t len = getreg32(PROGMEM_START + offsets[i] + 4u);

        bool fresh = (fn == 0xFFFFFFFFu) && (len == 0xFFFFFFFFu);
        bool valid = (fn == (uint32_t)i) &&
                     (len > 0u) && (len < (uint32_t)PROGMEM_SIZE);

        if (!fresh && !valid)
          {
            need_erase = true;
          }
      }

    if (need_erase)
      {
        size_t b;

        for (b = 0; b < PROGMEM_NPAGES; b++)
          {
            up_progmem_eraseblock(b);
          }
      }
  }
}

/****************************************************************************
 * up_progmem API — geometry queries
 ****************************************************************************/

size_t up_progmem_neraseblocks(void)
{
  return PROGMEM_NPAGES;
}

bool up_progmem_isuniform(void)
{
  return true;
}

size_t up_progmem_pagesize(size_t page)
{
  (void)page;
  return PROGMEM_PAGE_SIZE;
}

size_t up_progmem_erasesize(size_t block)
{
  (void)block;
  return PROGMEM_PAGE_SIZE;
}

ssize_t up_progmem_getpage(size_t addr)
{
  /* Accept both absolute and region-relative addresses */

  if (addr >= PROGMEM_START)
    {
      addr -= PROGMEM_START;
    }

  if (addr >= PROGMEM_SIZE)
    {
      return -EFAULT;
    }

  return (ssize_t)(addr / PROGMEM_PAGE_SIZE);
}

size_t up_progmem_getaddress(size_t page)
{
  if (page >= (size_t)PROGMEM_NPAGES)
    {
      return SIZE_MAX;
    }

  return PROGMEM_START + page * PROGMEM_PAGE_SIZE;
}

uint8_t up_progmem_erasestate(void)
{
  return 0xFF;
}

/****************************************************************************
 * up_progmem_ispageerased
 ****************************************************************************/

ssize_t up_progmem_ispageerased(size_t page)
{
  uintptr_t base;
  size_t    count;
  size_t    i;

  if (page >= (size_t)PROGMEM_NPAGES)
    {
      return -EFAULT;
    }

  base = up_progmem_getaddress(page);

  /* Evict stale XIP cache lines before reading back flash contents */

  up_invalidate_dcache(base, base + PROGMEM_PAGE_SIZE);

  count = 0;
  for (i = 0; i < PROGMEM_PAGE_SIZE; i++)
    {
      if (getreg8(base + i) != 0xFF)
        {
          count++;
        }
    }

  return (ssize_t)count;
}

/****************************************************************************
 * up_progmem_eraseblock — FCW PGERA (4096-byte page erase)
 ****************************************************************************/

ssize_t up_progmem_eraseblock(size_t block)
{
  uintptr_t  addr;
  irqstate_t flags;
  int        ret;

  if (block >= (size_t)PROGMEM_NPAGES)
    {
      return -EFAULT;
    }

  addr = up_progmem_getaddress(block);

  fcw_wait_ready();

  /* Write target address, then KEY→CTRLA with no intervening access */

  putreg32((uint32_t)addr, SAM_FCW_ADDR);

  flags = enter_critical_section();
  putreg32(FCW_KEY_WRKEY, SAM_FCW_KEY);
  putreg32(FCW_CTRLA_NVMOP_PGERA | FCW_CTRLA_PREPG, SAM_FCW_CTRLA);
  leave_critical_section(flags);

  fcw_wait_ready();

  ret = fcw_check_errors();
  if (ret < 0)
    {
      return ret;
    }

  /* Evict stale XIP lines covering the erased page */

  up_invalidate_dcache(addr, addr + PROGMEM_PAGE_SIZE);

  /* Verify erase */

  if (up_progmem_ispageerased(block) != 0)
    {
      return -EIO;
    }

  return (ssize_t)PROGMEM_PAGE_SIZE;
}

/****************************************************************************
 * up_progmem_write — FCW ROWP (1024-byte row program)
 *
 * Both addr and count must be PROGMEM_ROW_SIZE (1024) aligned.
 * The crash-dump layer (progmem_dump.c) guarantees this when
 * PROGMEM_DUMP_ALIGNMENT == SAM_FCW_ROW_SIZE.
 ****************************************************************************/

ssize_t up_progmem_write(size_t addr, FAR const void *buffer, size_t count)
{
  FAR const uint8_t *src = (FAR const uint8_t *)buffer;
  irqstate_t         flags;
  ssize_t            total = 0;

  /* Accept both absolute and region-relative addresses */

  if (addr < PROGMEM_START && addr < PROGMEM_SIZE)
    {
      addr += PROGMEM_START;
    }

  if (addr < PROGMEM_START || (addr + count) > PROGMEM_PFM_END)
    {
      return -EFAULT;
    }

  while (count > 0)
    {
      /* Determine the 1024-byte row containing addr */

      size_t row_base   = addr & ~((size_t)PROGMEM_ROW_SIZE - 1u);
      size_t row_offset = addr - row_base;
      size_t chunk      = PROGMEM_ROW_SIZE - row_offset;

      if (chunk > count)
        {
          chunk = count;
        }

      /* Read-modify: load current row content from XIP.
       * Invalidate D-cache first so we read the actual flash state, not a
       * stale cache line from a previous write cycle.                     */

      up_invalidate_dcache(row_base, row_base + PROGMEM_ROW_SIZE);
      memcpy(g_row_buf, (FAR const void *)row_base, PROGMEM_ROW_SIZE);

      /* Overlay the new bytes.  FCW ROWP ANDs g_row_buf with existing
       * flash, so bits can only be cleared (0 → 0, 1 → src bit).
       * Copying the new bytes into their position produces the correct
       * AND result for the changed region; the rest of the row is
       * already the current flash content, so it ANDs with itself → no
       * change.                                                            */

      memcpy(g_row_buf + row_offset, src, chunk);

      fcw_wait_ready();

      /* Set up source (RAM) and destination (flash) addresses, then
       * write KEY→CTRLA with absolutely no access between them.           */

      putreg32((uint32_t)g_row_buf, SAM_FCW_SRCADDR);
      putreg32((uint32_t)row_base,  SAM_FCW_ADDR);

      flags = enter_critical_section();
      putreg32(FCW_KEY_WRKEY, SAM_FCW_KEY);
      putreg32(FCW_CTRLA_NVMOP_ROWP | FCW_CTRLA_PREPG, SAM_FCW_CTRLA);
      leave_critical_section(flags);

      fcw_wait_ready();

      if (fcw_check_errors() < 0)
        {
          return -EIO;
        }

      /* Evict stale XIP lines for the written row */

      up_invalidate_dcache(row_base, row_base + PROGMEM_ROW_SIZE);

      addr  += chunk;
      src   += chunk;
      count -= chunk;
      total += (ssize_t)chunk;
    }

  return total;
}

#endif /* CONFIG_PIC32CZCA90_PROGMEM */
