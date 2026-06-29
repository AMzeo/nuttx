/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_dma.h
 *
 * PIC32CZ CA90 System DMA Controller
 * Base: 0x44850000, 16 channels, stride 0x50 per channel
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_DMA_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_DMA_H

#include "hardware/pic32czca90_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define PIC32CZCA90_DMA_NCHANNELS   16
#define SAM_DMA_CHAN_STRIDE         0x50

/* =========================================================================
 * Global Register Offsets (from DMA base)
 * ========================================================================= */

#define SAM_DMA_CTRLA_OFFSET        0x00
#define SAM_DMA_DBGCTRL_OFFSET      0x08
#define SAM_DMA_CRCPOLYA_OFFSET     0x0C
#define SAM_DMA_CRCPOLYB_OFFSET     0x10
#define SAM_DMA_INTSTAT4_OFFSET     0x14
#define SAM_DMA_INTSTAT3_OFFSET     0x18
#define SAM_DMA_INTSTAT2_OFFSET     0x1C
#define SAM_DMA_INTSTAT1_OFFSET     0x20

/* =========================================================================
 * Channel Register Offsets (relative to channel base)
 * Channel n base = SAM_DMAC_BASE + 0x50 + n * 0x50
 * ========================================================================= */

#define SAM_DMACH_CTRLA_OFFSET      0x00
#define SAM_DMACH_CTRLB_OFFSET      0x04
#define SAM_DMACH_EVCTRL_OFFSET     0x08
#define SAM_DMACH_INTENCLR_OFFSET   0x0C
#define SAM_DMACH_INTENSET_OFFSET   0x10
#define SAM_DMACH_INTF_OFFSET       0x14
#define SAM_DMACH_SSA_OFFSET        0x18
#define SAM_DMACH_DSA_OFFSET        0x1C
#define SAM_DMACH_SSTRD_OFFSET      0x20
#define SAM_DMACH_DSTRD_OFFSET      0x24
#define SAM_DMACH_XSIZ_OFFSET       0x28
#define SAM_DMACH_PDAT_OFFSET       0x2C
#define SAM_DMACH_CTRLCRC_OFFSET    0x30
#define SAM_DMACH_CRCDAT_OFFSET     0x34
#define SAM_DMACH_NXT_OFFSET        0x38
#define SAM_DMACH_LLCFGSTAT_OFFSET  0x3C
#define SAM_DMACH_STATBC_OFFSET     0x40
#define SAM_DMACH_STATCC_OFFSET     0x44
#define SAM_DMACH_STAT_OFFSET       0x48

/* =========================================================================
 * Register Addresses
 * ========================================================================= */

#define SAM_DMA_CTRLA               (SAM_DMAC_BASE + SAM_DMA_CTRLA_OFFSET)
#define SAM_DMA_INTSTAT4            (SAM_DMAC_BASE + SAM_DMA_INTSTAT4_OFFSET)
#define SAM_DMA_INTSTAT3            (SAM_DMAC_BASE + SAM_DMA_INTSTAT3_OFFSET)
#define SAM_DMA_INTSTAT2            (SAM_DMAC_BASE + SAM_DMA_INTSTAT2_OFFSET)
#define SAM_DMA_INTSTAT1            (SAM_DMAC_BASE + SAM_DMA_INTSTAT1_OFFSET)

#define SAM_DMA_CHAN_BASE(ch)       (SAM_DMAC_BASE + 0x50 + (uint32_t)(ch) * SAM_DMA_CHAN_STRIDE)

#define SAM_DMACH_CTRLA(ch)         (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_CTRLA_OFFSET)
#define SAM_DMACH_CTRLB(ch)         (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_CTRLB_OFFSET)
#define SAM_DMACH_INTENCLR(ch)      (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_INTENCLR_OFFSET)
#define SAM_DMACH_INTENSET(ch)      (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_INTENSET_OFFSET)
#define SAM_DMACH_INTF(ch)          (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_INTF_OFFSET)
#define SAM_DMACH_SSA(ch)           (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_SSA_OFFSET)
#define SAM_DMACH_DSA(ch)           (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_DSA_OFFSET)
#define SAM_DMACH_XSIZ(ch)         (SAM_DMA_CHAN_BASE(ch) + SAM_DMACH_XSIZ_OFFSET)

/* =========================================================================
 * CTRLA — Global DMA Enable (offset 0x00)
 * ========================================================================= */

#define DMA_CTRLA_ENABLE            (1u << 1)

/* =========================================================================
 * CHCTRLA — Channel Control A (offset 0x00 within channel)
 * ========================================================================= */

#define DMA_CHCTRLA_ENABLE          (1u << 0)
#define DMA_CHCTRLA_LLEN            (1u << 8)
#define DMA_CHCTRLA_SWFRC           (1u << 16)
#define DMA_CHCTRLA_RUNSTDBY        (1u << 24)

/* =========================================================================
 * CHCTRLB — Channel Control B (offset 0x04 within channel)
 *
 * Write-protected when CHCTRLA.ENABLE=1.
 * ========================================================================= */

/* WAS — Write Address Sequence [2:0] */

#define DMA_CHCTRLB_WAS_SHIFT       0
#define DMA_CHCTRLB_WAS_MASK        (0x7u << DMA_CHCTRLB_WAS_SHIFT)
#define DMA_CHCTRLB_WAS(v)          (((uint32_t)(v) << DMA_CHCTRLB_WAS_SHIFT) & DMA_CHCTRLB_WAS_MASK)

#define DMA_WAS_BYTE_INCR           0u
#define DMA_WAS_HWORD_INCR          1u
#define DMA_WAS_AUTO_INCR            2u
#define DMA_WAS_FIXED_BYTE          3u
#define DMA_WAS_FIXED_HWORD         4u
#define DMA_WAS_FIXED_WORD          5u

/* RAS — Read Address Sequence [6:4] */

#define DMA_CHCTRLB_RAS_SHIFT       4
#define DMA_CHCTRLB_RAS_MASK        (0x7u << DMA_CHCTRLB_RAS_SHIFT)
#define DMA_CHCTRLB_RAS(v)          (((uint32_t)(v) << DMA_CHCTRLB_RAS_SHIFT) & DMA_CHCTRLB_RAS_MASK)

#define DMA_RAS_BYTE_INCR           0u
#define DMA_RAS_HWORD_INCR          1u
#define DMA_RAS_AUTO_INCR            2u
#define DMA_RAS_FIXED_BYTE          3u
#define DMA_RAS_FIXED_HWORD         4u
#define DMA_RAS_FIXED_WORD          5u

/* PRI — Priority [9:8] */

#define DMA_CHCTRLB_PRI_SHIFT       8
#define DMA_CHCTRLB_PRI_MASK        (0x3u << DMA_CHCTRLB_PRI_SHIFT)
#define DMA_CHCTRLB_PRI(v)          (((uint32_t)(v) << DMA_CHCTRLB_PRI_SHIFT) & DMA_CHCTRLB_PRI_MASK)

#define DMA_PRI_1                   0u  /* Priority 1 (lowest) → fires INTSTAT1 → DMA_PRI0 IRQ */
#define DMA_PRI_2                   1u  /* Priority 2 → fires INTSTAT2 → DMA_PRI1 IRQ */
#define DMA_PRI_3                   2u  /* Priority 3 → fires INTSTAT3 → DMA_PRI2 IRQ */
#define DMA_PRI_4                   3u  /* Priority 4 (highest) → fires INTSTAT4 → DMA_PRI3 IRQ */

/* TRIG — Hardware Trigger [23:16] */

#define DMA_CHCTRLB_TRIG_SHIFT      16
#define DMA_CHCTRLB_TRIG_MASK       (0xFFu << DMA_CHCTRLB_TRIG_SHIFT)
#define DMA_CHCTRLB_TRIG(v)         (((uint32_t)(v) << DMA_CHCTRLB_TRIG_SHIFT) & DMA_CHCTRLB_TRIG_MASK)

/* CASTEN — Cell Auto Start Enable [29] */

#define DMA_CHCTRLB_CASTEN          (1u << 29)

/* =========================================================================
 * CHINTF — Channel Interrupt Flags (offset 0x14 within channel)
 * CHINTENSET/CHINTENCLR use the same bit positions.
 * Write-to-clear for CHINTF; write-to-set/clear for INTENSET/INTENCLR.
 * ========================================================================= */

#define DMA_CHINTF_SD               (1u << 0)   /* Start Detected */
#define DMA_CHINTF_TA               (1u << 1)   /* Transfer Abort */
#define DMA_CHINTF_CC               (1u << 2)   /* Cell Transfer Complete */
#define DMA_CHINTF_BC               (1u << 3)   /* Block Transfer Complete */
#define DMA_CHINTF_BH               (1u << 4)   /* Block Half Complete */
#define DMA_CHINTF_LL               (1u << 5)   /* Linked List Done */
#define DMA_CHINTF_WRE              (1u << 17)  /* Write Error */
#define DMA_CHINTF_RDE              (1u << 18)  /* Read Error */

#define DMA_CHINTF_ERRORS           (DMA_CHINTF_WRE | DMA_CHINTF_RDE)
#define DMA_CHINTF_ALL              (0x0006003Fu)

/* =========================================================================
 * CHXSIZ — Channel Transfer Size (offset 0x28 within channel)
 * ========================================================================= */

#define DMA_CHXSIZ_CSZ_SHIFT        0
#define DMA_CHXSIZ_CSZ_MASK         (0x3FFu << DMA_CHXSIZ_CSZ_SHIFT)
#define DMA_CHXSIZ_CSZ(v)           (((uint32_t)(v) << DMA_CHXSIZ_CSZ_SHIFT) & DMA_CHXSIZ_CSZ_MASK)

#define DMA_CHXSIZ_BLKSZ_SHIFT     16
#define DMA_CHXSIZ_BLKSZ_MASK      (0xFFFFu << DMA_CHXSIZ_BLKSZ_SHIFT)
#define DMA_CHXSIZ_BLKSZ(v)        (((uint32_t)(v) << DMA_CHXSIZ_BLKSZ_SHIFT) & DMA_CHXSIZ_BLKSZ_MASK)

/* =========================================================================
 * CHSTATBC — bytes transferred in block (offset 0x40, read-only)
 * ========================================================================= */

#define DMA_CHSTATBC_BBTC_MASK      0x0001FFFFu

/* =========================================================================
 * DMA Hardware Trigger IDs
 *
 * SERCOM formula: RX = 5 + sercom*2, TX = 6 + sercom*2
 * ========================================================================= */

#define DMAC_TRIG_SERCOM0_RX        5u
#define DMAC_TRIG_SERCOM0_TX        6u
#define DMAC_TRIG_SERCOM1_RX        7u
#define DMAC_TRIG_SERCOM1_TX        8u
#define DMAC_TRIG_SERCOM2_RX        9u
#define DMAC_TRIG_SERCOM2_TX        10u
#define DMAC_TRIG_SERCOM3_RX        11u
#define DMAC_TRIG_SERCOM3_TX        12u
#define DMAC_TRIG_SERCOM4_RX        13u
#define DMAC_TRIG_SERCOM4_TX        14u
#define DMAC_TRIG_SERCOM5_RX        15u
#define DMAC_TRIG_SERCOM5_TX        16u
#define DMAC_TRIG_SERCOM6_RX        17u
#define DMAC_TRIG_SERCOM6_TX        18u
#define DMAC_TRIG_SERCOM7_RX        19u
#define DMAC_TRIG_SERCOM7_TX        20u
#define DMAC_TRIG_SERCOM8_RX        21u
#define DMAC_TRIG_SERCOM8_TX        22u
#define DMAC_TRIG_SERCOM9_RX        23u
#define DMAC_TRIG_SERCOM9_TX        24u

/* Computed trigger from SERCOM number (portable across board configs) */

#define DMAC_TRIG_SERCOM_RX(n)      (DMAC_TRIG_SERCOM0_RX + (n) * 2u)
#define DMAC_TRIG_SERCOM_TX(n)      (DMAC_TRIG_SERCOM0_TX + (n) * 2u)

/* =========================================================================
 * MCLK Clock IDs for DMA controller
 * ========================================================================= */

#define MCLK_ID_AXI_DMA            24u
#define MCLK_ID_APB_DMA            25u

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_DMA_H */
