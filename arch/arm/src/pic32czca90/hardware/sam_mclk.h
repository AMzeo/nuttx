/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_mclk.h
 *
 * PIC32CZ CA90 Main Clock (MCLK)
 * Base: 0x44052000
 *
 * Register layout verified from PIC32CZ8110CA80208_DFP/component/mclk.h
 *
 * CRITICAL: CA90 uses CLKMSK[0..8] registers for peripheral APB clock
 * enables – NOT APBxMASK registers as on SAMD5x. The previous version
 * had APBxMASK registers (offsets 0x10-0x28) which do NOT exist on CA90,
 * causing SERCOM4 to never receive an APB clock.
 *
 *   Peripheral enable: CLKMSK[id/32] |= (1 << (id%32))
 *   SERCOM4 MCLK_ID_APB = 35 → CLKMSK[1] bit 3
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets – DFP verified
 * =========================================================================
 */

#define SAM_MCLK_INTENCLR_OFFSET    0x0000  /* Interrupt Enable Clear (8-bit) */
#define SAM_MCLK_INTENSET_OFFSET    0x0004  /* Interrupt Enable Set (8-bit)   */
#define SAM_MCLK_INTFLAG_OFFSET     0x0008  /* Interrupt Flag (8-bit)         */
/* 0x000B: reserved */
#define SAM_MCLK_CLKDIV_OFFSET(n)  (0x000C + (n) * 4)  /* Clock Divider n (32-bit) */
/* CLKDIV[0]=0x000C, CLKDIV[1]=0x0010 */
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

/* Convenience aliases */

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
 * MCLK_ID_APB values from PIC32CZ8110CA80208 DFP instance files.
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

/* Helper macros to compute the CLKMSK register address and bit for an ID */

#define SAM_MCLK_CLKMSK_ADDR(id)    SAM_MCLK_CLKMSK((uint32_t)(id) / 32u)
#define SAM_MCLK_CLKMSK_BIT(id)     (1u << ((uint32_t)(id) % 32u))

/* CPUDIV alias – CA90 has no dedicated CPUDIV register.
 * Harmony writes CLKDIV[1]=2 before switching GCLK0 to PLL0.
 * Map SAM_MCLK_CPUDIV to CLKDIV[1] so that
 *   putreg32(BOARD_MCLK_CPUDIV, SAM_MCLK_CPUDIV)
 * writes the correct value (2).
 */

#define SAM_MCLK_CPUDIV             SAM_MCLK_CLKDIV(1)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_MCLK_H */