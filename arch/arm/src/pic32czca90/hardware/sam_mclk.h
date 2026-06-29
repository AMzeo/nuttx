/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_mclk.h
 *
 * PIC32CZ CA90 Main Clock (MCLK)
 * Base: 0x44052000
 *
 * Peripheral APB clock enable: CLKMSK[id/32] |= (1 << (id%32))
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets
 * =========================================================================
 */

#define SAM_MCLK_INTENCLR_OFFSET    0x0000  /* Interrupt Enable Clear (8-bit) */
#define SAM_MCLK_INTENSET_OFFSET    0x0004  /* Interrupt Enable Set (8-bit)   */
#define SAM_MCLK_INTFLAG_OFFSET     0x0008  /* Interrupt Flag (8-bit)         */
/* 0x000B: reserved */
#define SAM_MCLK_CLKDIV_OFFSET(n)  (0x000C + (n) * 4)  /* Clock Divider n (32-bit) */
/* CLKDIV[0]=0x000C, CLKDIV[1]=0x0010 — uint32_t[2] stride 4 bytes */
/* 0x0014-0x003B: reserved */
#define SAM_MCLK_CLKMSK_OFFSET(n)  (0x003C + (n) * 4)  /* Clock Mask n (32-bit) */
/* CLKMSK[0]=0x003C .. CLKMSK[8]=0x005C */

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_MCLK_INTENCLR           (SAM_MCLK_BASE + SAM_MCLK_INTENCLR_OFFSET)
#define SAM_MCLK_INTENSET           (SAM_MCLK_BASE + SAM_MCLK_INTENSET_OFFSET)
#define SAM_MCLK_INTFLAG            (SAM_MCLK_BASE + SAM_MCLK_INTFLAG_OFFSET)
#define SAM_MCLK_CLKDIV(n)          (SAM_MCLK_BASE + SAM_MCLK_CLKDIV_OFFSET(n))
#define SAM_MCLK_CLKMSK(n)          (SAM_MCLK_BASE + SAM_MCLK_CLKMSK_OFFSET(n))

/* Convenience aliases
 *
 *   MCLK_CLKDIV[0]  offset 0x0C  CPU Clock Division Factor (PAC-protected)
 *   MCLK_CLKDIV[1]  offset 0x10  Second clock domain divider
 */

#define SAM_MCLK_CLKDIV0            SAM_MCLK_CLKDIV(0)
#define SAM_MCLK_CLKDIV1            SAM_MCLK_CLKDIV(1)

/* =========================================================================
 * INTFLAG Bits (offset 0x0008, 8-bit)
 * =========================================================================
 */

#define MCLK_INTFLAG_CKRDY          (1u << 0)  /* Clock Ready */

/* =========================================================================
 * CLKDIV field
 * =========================================================================
 */

#define MCLK_CLKDIV_DIV(v)          ((uint32_t)(v))

/* =========================================================================
 * Peripheral APB clock enable via CLKMSK[]
 *
 * Formula: enable = CLKMSK[id/32] |= (1 << (id%32))
 * =========================================================================
 */

#define MCLK_ID_APB_SERCOM0         31u  /* CLKMSK[0] bit 31 */
#define MCLK_ID_APB_SERCOM1         32u  /* CLKMSK[1] bit  0 */
#define MCLK_ID_APB_SERCOM2         33u  /* CLKMSK[1] bit  1 */
#define MCLK_ID_APB_SERCOM3         34u  /* CLKMSK[1] bit  2 */
#define MCLK_ID_APB_SERCOM4         35u  /* CLKMSK[1] bit  3 – console UART */
#define MCLK_ID_APB_SERCOM5         36u  /* CLKMSK[1] bit  4 */
#define MCLK_ID_APB_SERCOM6         37u  /* CLKMSK[1] bit  5 */
#define MCLK_ID_APB_SERCOM7         38u  /* CLKMSK[1] bit  6 */
#define MCLK_ID_APB_SERCOM8         39u  /* CLKMSK[1] bit  7 */
#define MCLK_ID_APB_SERCOM9         40u  /* CLKMSK[1] bit  8 */

/* TCC APB clock IDs */

#define MCLK_ID_APB_TCC0            41u  /* CLKMSK[1] bit  9 */
#define MCLK_ID_APB_TCC1            42u  /* CLKMSK[1] bit 10 */
#define MCLK_ID_APB_TCC2            43u  /* CLKMSK[1] bit 11 */
#define MCLK_ID_APB_TCC3            44u  /* CLKMSK[1] bit 12 */
#define MCLK_ID_APB_TCC4            45u  /* CLKMSK[1] bit 13 */
#define MCLK_ID_APB_TCC5            46u  /* CLKMSK[1] bit 14 */
#define MCLK_ID_APB_TCC6            47u  /* CLKMSK[1] bit 15 */
#define MCLK_ID_APB_TCC7            48u  /* CLKMSK[1] bit 16 */
#define MCLK_ID_APB_TCC8            49u  /* CLKMSK[1] bit 17 */
#define MCLK_ID_APB_TCC9            50u  /* CLKMSK[1] bit 18 */

/* Low-numbered APB/AHB clock IDs (CLKMSK[0])
 *
 * DSU:  APB=1, AHB=0  — enable before accessing DSU_DID (MCU serial number)
 * FCW:  AHB=2, APB=3  — enable before any flash write/erase
 * FCR:  AHB=4, APB=5  — enable before flash read controller config
 */

#define MCLK_ID_AHB_DSU             0u   /* CLKMSK[0] bit  0 */
#define MCLK_ID_APB_DSU             1u   /* CLKMSK[0] bit  1 */
#define MCLK_ID_AHB_FCW             2u   /* CLKMSK[0] bit  2 */
#define MCLK_ID_APB_FCW             3u   /* CLKMSK[0] bit  3 */
#define MCLK_ID_AHB_FCR             4u   /* CLKMSK[0] bit  4 */
#define MCLK_ID_APB_FCR             5u   /* CLKMSK[0] bit  5 */

/* Helper macros to compute the CLKMSK register address and bit for an ID */

#define SAM_MCLK_CLKMSK_ADDR(id)    SAM_MCLK_CLKMSK((uint32_t)(id) / 32u)
#define SAM_MCLK_CLKMSK_BIT(id)     (1u << ((uint32_t)(id) % 32u))

/* MCLK CLKDIV register map:
 *
 *   0x0C  CLKDIV[0]  CPU divider. Reads 1 (no division). PAC-protected — write = BusFault.
 *   0x10  CLKDIV[1]  Domain divider. Writable. Set to 2 then poll CKRDY before GCLK0 switch.
 *   0x14  (not accessible on CA90 — bus stall on read. DS §21.6 map is erroneous here.)
 *
 * SAM_MCLK_CPUDIV is intentionally NOT defined — CLKDIV[0] must never be written. */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H */