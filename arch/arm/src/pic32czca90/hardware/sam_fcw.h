/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_fcw.h
 *
 * PIC32CZ CA90 Flash Write Controller (FCW)
 * APB base: 0x44002000
 *
 * NOTE: CA90 uses FCW for flash writes, NOT NVMCTRL.
 *
 * Flash geometry:
 *   PFM bus address:  0x0C000000  (8 MB, 2 panels of 4 MB)
 *   BFM bus address:  0x08000000  (128 KB)
 *   Page size:        4096 bytes  (erase unit)
 *   Row size:         1024 bytes  (ROW_PROGRAM write unit)
 *   Write at < row:   64-bit or 256-bit doubleword writes
 *
 * MCLK IDs:
 *   FCW_MCLK_ID_AHB = 2   → CLKMSK[0] bit 2
 *   FCW_MCLK_ID_APB = 3   → CLKMSK[0] bit 3
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCW_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCW_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets
 * =========================================================================
 */

#define SAM_FCW_CTRLA_OFFSET        0x0000  /* NVM Write Control Register (R/W 32) */
#define SAM_FCW_CTRLB_OFFSET        0x0004  /* NVM Control B Register (R/W 32) */
#define SAM_FCW_MUTEX_OFFSET        0x0008  /* NVM MUTEX Register (R/W 32) */
#define SAM_FCW_INTENCLR_OFFSET     0x000C  /* Interrupt Enable Clear (R/W 32) */
#define SAM_FCW_INTENSET_OFFSET     0x0010  /* Interrupt Enable Set (R/W 32) */
#define SAM_FCW_INTFLAG_OFFSET      0x0014  /* Interrupt Flag Register (R/W 32) */
#define SAM_FCW_STATUS_OFFSET       0x0018  /* NVM Status Register (R/ 32) */
#define SAM_FCW_KEY_OFFSET          0x001C  /* SFR Unlock Register (R/W 32) */
#define SAM_FCW_ADDR_OFFSET         0x0020  /* Flash Address Register (R/W 32) */
#define SAM_FCW_SRCADDR_OFFSET      0x0024  /* Source Data Address Register (R/W 32) */
#define SAM_FCW_DATA_OFFSET(n)      (0x0028 + ((n) * 4))  /* Flash Write Data[0..7] */
#define SAM_FCW_SWAP_OFFSET         0x0048  /* NVM Panel Swap Register (R/W 32) */
#define SAM_FCW_PWP_OFFSET(n)       (0x004C + ((n) * 4))  /* PFM Write Protect Region[0..7] */
#define SAM_FCW_LBWP_OFFSET         0x006C  /* Lower BFM Write Protect (R/W 32) */
#define SAM_FCW_UBWP_OFFSET         0x0070  /* Upper BFM Write Protect (R/W 32) */

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_FCW_CTRLA               (SAM_FCW_BASE + SAM_FCW_CTRLA_OFFSET)
#define SAM_FCW_CTRLB               (SAM_FCW_BASE + SAM_FCW_CTRLB_OFFSET)
#define SAM_FCW_MUTEX               (SAM_FCW_BASE + SAM_FCW_MUTEX_OFFSET)
#define SAM_FCW_INTENCLR            (SAM_FCW_BASE + SAM_FCW_INTENCLR_OFFSET)
#define SAM_FCW_INTENSET            (SAM_FCW_BASE + SAM_FCW_INTENSET_OFFSET)
#define SAM_FCW_INTFLAG             (SAM_FCW_BASE + SAM_FCW_INTFLAG_OFFSET)
#define SAM_FCW_STATUS              (SAM_FCW_BASE + SAM_FCW_STATUS_OFFSET)
#define SAM_FCW_KEY                 (SAM_FCW_BASE + SAM_FCW_KEY_OFFSET)
#define SAM_FCW_ADDR                (SAM_FCW_BASE + SAM_FCW_ADDR_OFFSET)
#define SAM_FCW_SRCADDR             (SAM_FCW_BASE + SAM_FCW_SRCADDR_OFFSET)
#define SAM_FCW_DATA(n)             (SAM_FCW_BASE + SAM_FCW_DATA_OFFSET(n))
#define SAM_FCW_SWAP                (SAM_FCW_BASE + SAM_FCW_SWAP_OFFSET)
#define SAM_FCW_PWP(n)              (SAM_FCW_BASE + SAM_FCW_PWP_OFFSET(n))
#define SAM_FCW_LBWP                (SAM_FCW_BASE + SAM_FCW_LBWP_OFFSET)
#define SAM_FCW_UBWP                (SAM_FCW_BASE + SAM_FCW_UBWP_OFFSET)

/* =========================================================================
 * FCW_CTRLA — NVM Write Control Register (offset 0x00)
 *
 * NVMOP[3:0] selects the flash operation.
 * PREPG[7]   must be set to trigger the operation (set along with NVMOP).
 * Write KEY register with FCW_WRKEY immediately before writing CTRLA.
 * =========================================================================
 */

#define FCW_CTRLA_NVMOP_SHIFT       0
#define FCW_CTRLA_NVMOP_MASK        (0xfu << FCW_CTRLA_NVMOP_SHIFT)
#  define FCW_CTRLA_NVMOP_NOP       (0x0u << FCW_CTRLA_NVMOP_SHIFT)  /* No operation */
#  define FCW_CTRLA_NVMOP_SDWP      (0x1u << FCW_CTRLA_NVMOP_SHIFT)  /* Single double-word (64-bit) program */
#  define FCW_CTRLA_NVMOP_QDWP      (0x2u << FCW_CTRLA_NVMOP_SHIFT)  /* Quad double-word (256-bit) program */
#  define FCW_CTRLA_NVMOP_ROWP      (0x3u << FCW_CTRLA_NVMOP_SHIFT)  /* Row program (1024 B from SRCADDR) */
#  define FCW_CTRLA_NVMOP_PGERA     (0x4u << FCW_CTRLA_NVMOP_SHIFT)  /* Page erase (4096 B at ADDR) */
#  define FCW_CTRLA_NVMOP_BULKERA   (0x7u << FCW_CTRLA_NVMOP_SHIFT)  /* Program/bulk erase (full panel) */
#define FCW_CTRLA_PREPG             (1u << 7)  /* Pre-program / trigger bit — set with NVMOP */

/* =========================================================================
 * FCW_MUTEX — Multi-CPU Access Mutex (offset 0x08)
 * Only needed on multi-core CA90 variants. Single-CPU use: read before write,
 * check LOCK=0, then write 1 to acquire.
 * =========================================================================
 */

#define FCW_MUTEX_LOCK              (1u << 0)  /* FCW locked by this owner */
#define FCW_MUTEX_OWNER_SHIFT       1
#define FCW_MUTEX_OWNER_MASK        (0x3u << FCW_MUTEX_OWNER_SHIFT)  /* Owner CPU ID */

/* =========================================================================
 * FCW_INTFLAG — Interrupt Flag Register (offset 0x14)
 * Write 1 to clear a flag. DONE is the normal completion flag.
 * All others are error flags — check after operation.
 * =========================================================================
 */

#define FCW_INTFLAG_DONE            (1u << 0)   /* Operation complete */
#define FCW_INTFLAG_KEYERR          (1u << 1)   /* Wrong unlock key written */
#define FCW_INTFLAG_CFGERR          (1u << 2)   /* Configuration error (PAC, DAL) */
#define FCW_INTFLAG_FIFOERR         (1u << 3)   /* FIFO underrun during row write */
#define FCW_INTFLAG_BUSERR          (1u << 4)   /* AHB bus error during row write */
#define FCW_INTFLAG_WPERR           (1u << 5)   /* Write protection violation */
#define FCW_INTFLAG_OPERR           (1u << 6)   /* Invalid NVMOP for current state */
#define FCW_INTFLAG_SECERR          (1u << 7)   /* Security violation */
#define FCW_INTFLAG_HTDPGM          (1u << 8)   /* High temperature during program */
#define FCW_INTFLAG_BORERR          (1u << 12)  /* Brown-out during program */
#define FCW_INTFLAG_WRERR           (1u << 13)  /* Write error (verify failed) */

/* Mask of all error flags — check these after any write/erase */
#define FCW_INTFLAG_ERRMASK         (FCW_INTFLAG_KEYERR  | FCW_INTFLAG_CFGERR  | \
                                     FCW_INTFLAG_FIFOERR | FCW_INTFLAG_BUSERR  | \
                                     FCW_INTFLAG_WPERR   | FCW_INTFLAG_OPERR   | \
                                     FCW_INTFLAG_SECERR  | FCW_INTFLAG_HTDPGM  | \
                                     FCW_INTFLAG_BORERR  | FCW_INTFLAG_WRERR)

/* =========================================================================
 * FCW_STATUS — NVM Status Register (offset 0x18, read-only)
 * =========================================================================
 */

#define FCW_STATUS_BUSY             (1u << 0)   /* Flash operation in progress */

/* =========================================================================
 * FCW_KEY — SFR Unlock Register (offset 0x1C)
 *
 * Write the appropriate key immediately before every FCW_CTRLA write.
 * The key is consumed on the next register write — do NOT insert any
 * other register access between KEY write and CTRLA write.
 *
 * Key values:
 * =========================================================================
 */

#define FCW_KEY_WRKEY               0x91C32C01u  /* Unlock for erase/program ops */
#define FCW_KEY_SWAPKEY             0x91C32C02u  /* Unlock for panel swap (FCW_SWAP) */
#define FCW_KEY_CFGKEY              0x91C32C04u  /* Unlock for write-protect config */

/* =========================================================================
 * FCW_SWAP — NVM Panel Swap Register (offset 0x48)
 * =========================================================================
 */

#define FCW_SWAP_PFSWAP             (1u << 0)   /* 0=Panel 1 active, 1=Panel 2 active */

/* =========================================================================
 * Flash Geometry Constants
 * =========================================================================
 */

#define SAM_FCW_PFM_BASE            0x0C000000u  /* PFM bus start address */
#define SAM_FCW_BFM_BASE            0x08000000u  /* BFM bus start address */
#define SAM_FCW_FLASH_SIZE          0x00800000u  /* 8 MB total PFM */
#define SAM_FCW_PAGE_SIZE           4096u        /* Bytes per page (erase unit) */
#define SAM_FCW_ROW_SIZE            1024u        /* Bytes per row (ROW_PROGRAM unit) */
#define SAM_FCW_NDATA_REGS          8u           /* DATA[0..7] for quad-DW writes */

/* =========================================================================
 * MCLK IDs — enable clocks before any FCW register access
 * Formula: CLKMSK[id/32] |= (1 << (id % 32))
 * =========================================================================
 */

#define MCLK_ID_AHB_FCW             2u   /* CLKMSK[0] bit 2 */
#define MCLK_ID_APB_FCW             3u   /* CLKMSK[0] bit 3 */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_FCW_H */
