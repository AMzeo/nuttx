/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/sam_dmac.h
 *
 * PIC32CZ CA90 System DMA — Public API
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_DMAC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_DMAC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stddef.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Channel flags — encode peripheral trigger ID and DMA priority in a
 * 32-bit word, following the SAMV7 DMACH_FLAG_* convention.
 */

/* Peripheral trigger ID [7:0] — maps to CHCTRLB.TRIG */

#define DMACH_FLAG_PERIPHPID_SHIFT  0
#define DMACH_FLAG_PERIPHPID_MASK   (0xFFu << DMACH_FLAG_PERIPHPID_SHIFT)
#define DMACH_FLAG_PERIPHPID(n)     ((uint32_t)(n) << DMACH_FLAG_PERIPHPID_SHIFT)

/* DMA priority level [9:8] — maps to CHCTRLB.PRI */

#define DMACH_FLAG_PRIORITY_SHIFT   8
#define DMACH_FLAG_PRIORITY_MASK    (0x3u << DMACH_FLAG_PRIORITY_SHIFT)
#define DMACH_FLAG_PRIORITY(n)      ((uint32_t)(n) << DMACH_FLAG_PRIORITY_SHIFT)

/* Memory address increment control [10] — when set, memory side is FIXED
 * (no address increment).  Used for SPI NULL-buffer DMA where a single
 * dummy byte is read/written repeatedly.  Default (0) = memory increments.
 */

#define DMACH_FLAG_NOINC            (1u << 10)

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef void *DMA_HANDLE;
typedef void (*dma_callback_t)(DMA_HANDLE handle, void *arg, int result);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_PIC32CZCA90_DMAC

/****************************************************************************
 * Name: sam_dmachannel
 *
 * Description:
 *   Allocate a DMA channel. Returns NULL if no channel available.
 *
 * Input Parameters:
 *   chflags - DMACH_FLAG_PERIPHPID(trigger) | DMACH_FLAG_PRIORITY(pri)
 *
 ****************************************************************************/

DMA_HANDLE sam_dmachannel(uint32_t chflags);

/****************************************************************************
 * Name: sam_dmaconfig
 *
 * Description:
 *   Reconfigure channel flags between transfers (two-model pattern).
 *
 ****************************************************************************/

void sam_dmaconfig(DMA_HANDLE handle, uint32_t chflags);

/****************************************************************************
 * Name: sam_dmafree
 *
 * Description:
 *   Release a previously allocated DMA channel.
 *
 ****************************************************************************/

void sam_dmafree(DMA_HANDLE handle);

/****************************************************************************
 * Name: sam_dmarxsetup
 *
 * Description:
 *   Configure a peripheral-to-memory (RX) DMA transfer.
 *   Invalidates D-cache for the destination buffer.
 *
 * Input Parameters:
 *   handle - DMA channel handle
 *   paddr  - Peripheral register address (fixed, e.g., SERCOM DATA)
 *   maddr  - Memory buffer address (incrementing)
 *   nbytes - Transfer size (max 65535)
 *
 ****************************************************************************/

int sam_dmarxsetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                   size_t nbytes);

/****************************************************************************
 * Name: sam_dmatxsetup
 *
 * Description:
 *   Configure a memory-to-peripheral (TX) DMA transfer.
 *   Cleans D-cache for the source buffer.
 *
 ****************************************************************************/

int sam_dmatxsetup(DMA_HANDLE handle, uint32_t paddr, uint32_t maddr,
                   size_t nbytes);

/****************************************************************************
 * Name: sam_dmastart
 *
 * Description:
 *   Start a previously configured DMA transfer. The callback will be
 *   invoked from ISR context with result=OK on success, -EIO on error.
 *
 ****************************************************************************/

int sam_dmastart(DMA_HANDLE handle, dma_callback_t callback, void *arg);

/****************************************************************************
 * Name: sam_dmastop
 *
 * Description:
 *   Stop an in-progress DMA transfer. Callback is invoked with -EINTR.
 *
 ****************************************************************************/

void sam_dmastop(DMA_HANDLE handle);

/****************************************************************************
 * Name: sam_dmaresidual
 *
 * Description:
 *   Return the number of bytes remaining in the current block transfer.
 *   Reads CHXSIZ.BLKSZ (hardware decrements during transfer).
 *
 ****************************************************************************/

size_t sam_dmaresidual(DMA_HANDLE handle);

#endif /* CONFIG_PIC32CZCA90_DMAC */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_DMAC_H */
