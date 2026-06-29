/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_eic.h
 *
 * PIC32CZ CA90 External Interrupt Controller (EIC) registers.
 * External Interrupt Controller register definitions.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_EIC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_EIC_H

/****************************************************************************
 * EIC Base Address, GCLK, MCLK
 ****************************************************************************/

#ifndef SAM_EIC_BASE
#define SAM_EIC_BASE              0x44800000u
#endif
#define EIC_GCLK_ID              5
#define EIC_MCLK_ID_APB          16
#define EIC_NEXTINT              16

/****************************************************************************
 * Register Offsets
 ****************************************************************************/

#define SAM_EIC_CTRLA_OFFSET      0x00   /* Control A (8-bit) */
#define SAM_EIC_NMICTRL_OFFSET    0x01   /* NMI Control (8-bit) */
#define SAM_EIC_NMIFLAG_OFFSET    0x02   /* NMI Flag (16-bit) */
#define SAM_EIC_SYNCBUSY_OFFSET   0x04   /* Sync Busy (32-bit, RO) */
#define SAM_EIC_EVCTRL_OFFSET     0x08   /* Event Control (32-bit) */
#define SAM_EIC_INTENCLR_OFFSET   0x0C   /* Interrupt Enable Clear (32-bit) */
#define SAM_EIC_INTENSET_OFFSET   0x10   /* Interrupt Enable Set (32-bit) */
#define SAM_EIC_INTFLAG_OFFSET    0x14   /* Interrupt Flag (32-bit, W1C) */
#define SAM_EIC_ASYNCH_OFFSET     0x18   /* Async Mode (32-bit) */
#define SAM_EIC_CONFIG0_OFFSET    0x1C   /* Sense Config EXTINT0-7 (32-bit) */
#define SAM_EIC_CONFIG1_OFFSET    0x20   /* Sense Config EXTINT8-15 (32-bit) */
#define SAM_EIC_DEBOUNCEN_OFFSET  0x30   /* Debouncer Enable (32-bit) */
#define SAM_EIC_DPRESCALER_OFFSET 0x34   /* Debouncer Prescaler (32-bit) */
#define SAM_EIC_PINSTATE_OFFSET   0x38   /* Pin State (32-bit, RO) */

/****************************************************************************
 * Register Addresses
 ****************************************************************************/

#define SAM_EIC_CTRLA             (SAM_EIC_BASE + SAM_EIC_CTRLA_OFFSET)
#define SAM_EIC_SYNCBUSY          (SAM_EIC_BASE + SAM_EIC_SYNCBUSY_OFFSET)
#define SAM_EIC_INTENCLR          (SAM_EIC_BASE + SAM_EIC_INTENCLR_OFFSET)
#define SAM_EIC_INTENSET          (SAM_EIC_BASE + SAM_EIC_INTENSET_OFFSET)
#define SAM_EIC_INTFLAG           (SAM_EIC_BASE + SAM_EIC_INTFLAG_OFFSET)
#define SAM_EIC_ASYNCH            (SAM_EIC_BASE + SAM_EIC_ASYNCH_OFFSET)
#define SAM_EIC_CONFIG0           (SAM_EIC_BASE + SAM_EIC_CONFIG0_OFFSET)
#define SAM_EIC_CONFIG1           (SAM_EIC_BASE + SAM_EIC_CONFIG1_OFFSET)
#define SAM_EIC_DEBOUNCEN         (SAM_EIC_BASE + SAM_EIC_DEBOUNCEN_OFFSET)
#define SAM_EIC_PINSTATE          (SAM_EIC_BASE + SAM_EIC_PINSTATE_OFFSET)

/****************************************************************************
 * CTRLA Bits (offset 0x00, 8-bit)
 ****************************************************************************/

#define EIC_CTRLA_SWRST           (1u << 0)
#define EIC_CTRLA_ENABLE          (1u << 1)
#define EIC_CTRLA_CKSEL           (1u << 4)   /* 0=GCLK, 1=CLK_ULP32K */

/****************************************************************************
 * SYNCBUSY Bits (offset 0x04, 32-bit, RO)
 ****************************************************************************/

#define EIC_SYNCBUSY_SWRST        (1u << 0)
#define EIC_SYNCBUSY_ENABLE       (1u << 1)

/****************************************************************************
 * CONFIG0/CONFIG1 — Sense Configuration
 *
 * Each EXTINT line uses 4 bits: [3:1]=SENSE, [0]=FILTEN (relative to group)
 * Actually: bits [2:0]=SENSE, bit [3]=FILTEN per 4-bit field
 * CONFIG0: EXTINT0-7 (bits [31:0], 4 bits each)
 * CONFIG1: EXTINT8-15 (bits [31:0], 4 bits each)
 *
 * For EXTINTn in CONFIGx: shift = (n % 8) * 4
 ****************************************************************************/

#define EIC_SENSE_NONE            0x0u
#define EIC_SENSE_RISE            0x1u
#define EIC_SENSE_FALL            0x2u
#define EIC_SENSE_BOTH            0x3u
#define EIC_SENSE_HIGH            0x4u
#define EIC_SENSE_LOW             0x5u

#define EIC_CONFIG_SENSE_SHIFT(n) (((n) % 8u) * 4u)
#define EIC_CONFIG_SENSE_MASK(n)  (0x7u << EIC_CONFIG_SENSE_SHIFT(n))
#define EIC_CONFIG_SENSE(n, s)    (((s) & 0x7u) << EIC_CONFIG_SENSE_SHIFT(n))
#define EIC_CONFIG_FILTEN(n)      (1u << (EIC_CONFIG_SENSE_SHIFT(n) + 3u))

/****************************************************************************
 * INTFLAG / INTENSET / INTENCLR — per-EXTINT bit
 ****************************************************************************/

#define EIC_EXTINT(n)             (1u << (n))

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_EIC_H */
