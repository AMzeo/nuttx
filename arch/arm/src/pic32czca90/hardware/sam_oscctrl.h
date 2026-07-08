/* SPDX-License-Identifier: Apache-2.0 */

/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_oscctrl.h
 *
 * PIC32CZ CA90 Oscillator Controller (OSCCTRL)
 * Base: 0x44040000
 *
 * Clock strategy:
 *   DFLL48M (48 MHz, free-running from reset) → PLL0 reference (REFSEL=2)
 *   PLL0: REFDIV=12, FBDIV=225 → 900 MHz VCO, POSTDIV0=3 → 300 MHz
 *   DFLLSYNC does NOT exist on CA90. sam_dfll_configure() is never called.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_OSCCTRL_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_OSCCTRL_H

#include "hardware/sam_memorymap.h"

/* =========================================================================
 * Register Offsets
 * =========================================================================
 */

#define SAM_OSCCTRL_EVCTRL_OFFSET           0x0000  /* Event Control (8-bit) */
#define SAM_OSCCTRL_INTENCLR_OFFSET         0x0004  /* Interrupt Enable Clr  */
#define SAM_OSCCTRL_INTENSET_OFFSET         0x0008  /* Interrupt Enable Set  */
#define SAM_OSCCTRL_INTFLAG_OFFSET          0x000C  /* Interrupt Flag        */
#define SAM_OSCCTRL_STATUS_OFFSET           0x0010  /* Status (32-bit)       */
#define SAM_OSCCTRL_XOSCCTRLA_OFFSET        0x0014  /* XOSC Control A        */
#define SAM_OSCCTRL_DFLLCTRLA_OFFSET        0x002C  /* DFLL48M Control A     */
#define SAM_OSCCTRL_DFLLCTRLB_OFFSET        0x0030  /* DFLL48M Control B     */
#define SAM_OSCCTRL_DFLLVAL_OFFSET          0x0034  /* DFLL48M Value         */
#define SAM_OSCCTRL_DFLLMUL_OFFSET          0x003C  /* DFLL48M Multiplier    */
#define SAM_OSCCTRL_PLL0CTRL_OFFSET         0x0040  /* PLL0 Control          */
#define SAM_OSCCTRL_PLL0FBDIV_OFFSET        0x0044  /* PLL0 Feedback Divider */
#define SAM_OSCCTRL_PLL0REFDIV_OFFSET       0x0048  /* PLL0 Reference Divider*/
#define SAM_OSCCTRL_PLL0POSTDIVA_OFFSET     0x004C  /* PLL0 Post-Divider A   */
#define SAM_OSCCTRL_PLL1CTRL_OFFSET         0x0054  /* PLL1 Control          */
#define SAM_OSCCTRL_FRACDIV0_OFFSET         0x006C  /* Fractional Divider 0  */
#define SAM_OSCCTRL_SYNCBUSY_OFFSET         0x0078  /* Sync Busy (32-bit)    */

/* =========================================================================
 * Register Addresses
 * =========================================================================
 */

#define SAM_OSCCTRL_EVCTRL       (SAM_OSCCTRL_BASE + SAM_OSCCTRL_EVCTRL_OFFSET)
#define SAM_OSCCTRL_INTENCLR     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_INTENCLR_OFFSET)
#define SAM_OSCCTRL_INTENSET     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_INTENSET_OFFSET)
#define SAM_OSCCTRL_INTFLAG      (SAM_OSCCTRL_BASE + SAM_OSCCTRL_INTFLAG_OFFSET)
#define SAM_OSCCTRL_STATUS       (SAM_OSCCTRL_BASE + SAM_OSCCTRL_STATUS_OFFSET)
#define SAM_OSCCTRL_XOSCCTRLA    (SAM_OSCCTRL_BASE + SAM_OSCCTRL_XOSCCTRLA_OFFSET)
#define SAM_OSCCTRL_DFLLCTRLA    (SAM_OSCCTRL_BASE + SAM_OSCCTRL_DFLLCTRLA_OFFSET)
#define SAM_OSCCTRL_DFLLCTRLB    (SAM_OSCCTRL_BASE + SAM_OSCCTRL_DFLLCTRLB_OFFSET)
#define SAM_OSCCTRL_DFLLVAL      (SAM_OSCCTRL_BASE + SAM_OSCCTRL_DFLLVAL_OFFSET)
#define SAM_OSCCTRL_DFLLMUL      (SAM_OSCCTRL_BASE + SAM_OSCCTRL_DFLLMUL_OFFSET)
#define SAM_OSCCTRL_PLL0CTRL     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_PLL0CTRL_OFFSET)
#define SAM_OSCCTRL_PLL0FBDIV    (SAM_OSCCTRL_BASE + SAM_OSCCTRL_PLL0FBDIV_OFFSET)
#define SAM_OSCCTRL_PLL0REFDIV   (SAM_OSCCTRL_BASE + SAM_OSCCTRL_PLL0REFDIV_OFFSET)
#define SAM_OSCCTRL_PLL0POSTDIVA (SAM_OSCCTRL_BASE + SAM_OSCCTRL_PLL0POSTDIVA_OFFSET)
#define SAM_OSCCTRL_PLL1CTRL     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_PLL1CTRL_OFFSET)
#define SAM_OSCCTRL_FRACDIV0     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_FRACDIV0_OFFSET)
#define SAM_OSCCTRL_SYNCBUSY     (SAM_OSCCTRL_BASE + SAM_OSCCTRL_SYNCBUSY_OFFSET)

/* CA90 has a single XOSC (XOSCCTRLA). Aliases for code using XOSC0 name. */

#define SAM_OSCCTRL_XOSCCTRL0       SAM_OSCCTRL_XOSCCTRLA
#define SAM_OSCCTRL_XOSCCTRL(n)     SAM_OSCCTRL_XOSCCTRLA

/* =========================================================================
 * STATUS Register Bits (offset 0x0010, 32-bit)
 * =========================================================================
 */

#define OSCCTRL_STATUS_XOSCRDY0          (1u << 0)  /* XOSC0 ready (CA90 has one XOSC) */
#define OSCCTRL_STATUS_XOSCFAIL0         (1u << 2)  /* XOSC failure          */
#define OSCCTRL_STATUS_DFLLRDY           (1u << 8)  /* DFLL48M ready         */
#define OSCCTRL_STATUS_DFLLOOB           (1u << 9)
#define OSCCTRL_STATUS_DFLLLCKF          (1u << 10)
#define OSCCTRL_STATUS_DFLLLCKC          (1u << 11)
#define OSCCTRL_STATUS_PLL0LOCK          (1u << 24) /* PLL0 locked           */
#define OSCCTRL_STATUS_PLL1LOCK          (1u << 25) /* PLL1 locked           */

/* =========================================================================
 * SYNCBUSY Register Bits (offset 0x0078, 32-bit)
 * =========================================================================
 */

#define OSCCTRL_SYNCBUSY_DFLLCTRLA       (1u << 0)
#define OSCCTRL_SYNCBUSY_DFLLCTRLB       (1u << 1)
#define OSCCTRL_SYNCBUSY_DFLLVAL         (1u << 2)
#define OSCCTRL_SYNCBUSY_DFLLMUL         (1u << 3)
#define OSCCTRL_SYNCBUSY_PLL0CTRL        (1u << 4)
#define OSCCTRL_SYNCBUSY_FRACDIV0        (1u << 6)

/* =========================================================================
 * XOSCCTRLA Register Bits (offset 0x0014, 32-bit)
 * DFP: PIC32CZ8110CA80208, mask = 0x830F0FBE
 * =========================================================================
 */

#define OSCCTRL_XOSCCTRL_ENABLE          (1u << 1)
#define OSCCTRL_XOSCCTRL_AGC             (1u << 2)
#define OSCCTRL_XOSCCTRL_XTALEN          (1u << 3)  /* 0=ext clock, 1=crystal */
#define OSCCTRL_XOSCCTRL_CFDEN           (1u << 4)
#define OSCCTRL_XOSCCTRL_SWBEN           (1u << 5)
#define OSCCTRL_XOSCCTRL_ONDEMAND        (1u << 7)
#define OSCCTRL_XOSCCTRL_STARTUP_SHIFT   8
#define OSCCTRL_XOSCCTRL_STARTUP_MASK    (0xfu << OSCCTRL_XOSCCTRL_STARTUP_SHIFT)
#define OSCCTRL_XOSCCTRL_CFDPRESC_SHIFT  16
#define OSCCTRL_XOSCCTRL_CFDPRESC_MASK   (0xfu << OSCCTRL_XOSCCTRL_CFDPRESC_SHIFT)
#define OSCCTRL_XOSCCTRL_USBHSDIV_SHIFT  24
#define OSCCTRL_XOSCCTRL_USBHSDIV_MASK   (0x3u << OSCCTRL_XOSCCTRL_USBHSDIV_SHIFT)
#define OSCCTRL_XOSCCTRL_USBHSDIV(v)     (((uint32_t)(v) << OSCCTRL_XOSCCTRL_USBHSDIV_SHIFT) & OSCCTRL_XOSCCTRL_USBHSDIV_MASK)
#define OSCCTRL_XOSCCTRL_USBHSDIV_DIS    OSCCTRL_XOSCCTRL_USBHSDIV(0)
#define OSCCTRL_XOSCCTRL_USBHSDIV_DIV1   OSCCTRL_XOSCCTRL_USBHSDIV(1)
#define OSCCTRL_XOSCCTRL_USBHSDIV_DIV2   OSCCTRL_XOSCCTRL_USBHSDIV(2)
#define OSCCTRL_XOSCCTRL_USBHSDIV_DIV4   OSCCTRL_XOSCCTRL_USBHSDIV(3)
#define OSCCTRL_XOSCCTRL_LOWBUFGAIN      (1u << 31)

/* =========================================================================
 * DFLL48M Register Bits
 * =========================================================================
 */

#define OSCCTRL_DFLLCTRLA_ENABLE         (1u << 1)
#define OSCCTRL_DFLLCTRLA_RUNSTDBY       (1u << 6)
#define OSCCTRL_DFLLCTRLA_ONDEMAND       (1u << 7)

#define OSCCTRL_DFLLCTRLB_MODE           (1u << 0) /* 0=open loop */
#define OSCCTRL_DFLLCTRLB_STABLE         (1u << 1)
#define OSCCTRL_DFLLCTRLB_LLAW           (1u << 2)
#define OSCCTRL_DFLLCTRLB_USBCRM         (1u << 3)
#define OSCCTRL_DFLLCTRLB_CCDIS          (1u << 4)
#define OSCCTRL_DFLLCTRLB_QLDIS          (1u << 5)
#define OSCCTRL_DFLLCTRLB_BPLCKC         (1u << 6)
#define OSCCTRL_DFLLCTRLB_WAITLOCK       (1u << 7)

#define OSCCTRL_DFLLMUL_MUL_SHIFT        0
#define OSCCTRL_DFLLMUL_MUL_MASK         (0xffffu << OSCCTRL_DFLLMUL_MUL_SHIFT)
#define OSCCTRL_DFLLMUL_FSTEP_SHIFT      16
#define OSCCTRL_DFLLMUL_FSTEP_MASK       (0xffu << OSCCTRL_DFLLMUL_FSTEP_SHIFT)
#define OSCCTRL_DFLLMUL_CSTEP_SHIFT      26
#define OSCCTRL_DFLLMUL_CSTEP_MASK       (0x3fu << OSCCTRL_DFLLMUL_CSTEP_SHIFT)

/* =========================================================================
 * PLL0CTRL Register Bits (offset 0x0040, 32-bit)
 * =========================================================================
 */

#define OSCCTRL_PLL0CTRL_ENABLE          (1u << 1)
#define OSCCTRL_PLL0CTRL_REFSEL_SHIFT    8
#define OSCCTRL_PLL0CTRL_REFSEL_MASK     (0x7u << OSCCTRL_PLL0CTRL_REFSEL_SHIFT)
#  define OSCCTRL_PLL0CTRL_REFSEL_GCLK   (0u << OSCCTRL_PLL0CTRL_REFSEL_SHIFT)
#  define OSCCTRL_PLL0CTRL_REFSEL_XOSC   (1u << OSCCTRL_PLL0CTRL_REFSEL_SHIFT)
#  define OSCCTRL_PLL0CTRL_REFSEL_DFLL   (2u << OSCCTRL_PLL0CTRL_REFSEL_SHIFT)
#define OSCCTRL_PLL0CTRL_BWSEL_SHIFT     11
#define OSCCTRL_PLL0CTRL_BWSEL_MASK      (0x7u << OSCCTRL_PLL0CTRL_BWSEL_SHIFT)
#  define OSCCTRL_PLL0CTRL_BWSEL(v)      ((uint32_t)(v) << OSCCTRL_PLL0CTRL_BWSEL_SHIFT)

/* =========================================================================
 * PLL0FBDIV Register (offset 0x0044, 32-bit)
 * FBDIV=225: VCO = 4 MHz × 225 = 900 MHz
 * =========================================================================
 */

#define OSCCTRL_PLL0FBDIV_MASK           0x00000fffu
#  define OSCCTRL_PLL0FBDIV(v)           ((uint32_t)(v) & OSCCTRL_PLL0FBDIV_MASK)

/* =========================================================================
 * PLL0REFDIV Register (offset 0x0048, 32-bit)
 * REFDIV=12: 48 MHz / 12 = 4 MHz reference
 * =========================================================================
 */

#define OSCCTRL_PLL0REFDIV_MASK          0x000003ffu
#  define OSCCTRL_PLL0REFDIV(v)          ((uint32_t)(v) & OSCCTRL_PLL0REFDIV_MASK)

/* =========================================================================
 * PLL0POSTDIVA Register (offset 0x004C, 32-bit)
 * POSTDIV0=3: 900 MHz / 3 = 300 MHz output
 * OUTEN0=1: enable output
 * =========================================================================
 */

#define OSCCTRL_PLL0POSTDIVA_POSTDIV0_MASK  0x0000003fu
#  define OSCCTRL_PLL0POSTDIVA_POSTDIV0(v)  \
     ((uint32_t)(v) & OSCCTRL_PLL0POSTDIVA_POSTDIV0_MASK)
#define OSCCTRL_PLL0POSTDIVA_OUTEN0         (1u << 7)

/* =========================================================================
 * FRACDIV0 Register (offset 0x006C, 32-bit)
 * Set to 0 (integer-only, no fractional division)
 * =========================================================================
 */

#define OSCCTRL_FRACDIV0_REMDIV_SHIFT    0
#define OSCCTRL_FRACDIV0_REMDIV_MASK     (0xfffu << OSCCTRL_FRACDIV0_REMDIV_SHIFT)
#  define OSCCTRL_FRACDIV0_REMDIV(v)     ((uint32_t)(v) & OSCCTRL_FRACDIV0_REMDIV_MASK)
#define OSCCTRL_FRACDIV0_INTDIV_SHIFT    16
#define OSCCTRL_FRACDIV0_INTDIV_MASK     (0xffu << OSCCTRL_FRACDIV0_INTDIV_SHIFT)
#  define OSCCTRL_FRACDIV0_INTDIV(v)     ((uint32_t)(v) << OSCCTRL_FRACDIV0_INTDIV_SHIFT)

/* =========================================================================
 * Legacy DPLL compatibility aliases
 *
 * CA90 has NO DPLL0/DPLL1 registers. The "DPLL" name in older NuttX code
 * refers to a different IP block that does not exist on CA90.
 *
 * These aliases are provided ONLY to prevent compilation errors. The actual
 * DPLL-configure path is disabled via BOARD_DPLL0_ENABLE=FALSE in board.h,
 * so sam_dpll_configure() returns immediately without touching hardware.
 * =========================================================================
 */

#define SAM_OSCCTRL_DPLLCTRLA(n)         SAM_OSCCTRL_PLL0CTRL
#define SAM_OSCCTRL_DPLLRATIO(n)         SAM_OSCCTRL_PLL0FBDIV
#define SAM_OSCCTRL_DPLLCTRLB(n)         SAM_OSCCTRL_PLL0REFDIV
#define SAM_OSCCTRL_DPLLSYNCBUSY(n)      SAM_OSCCTRL_SYNCBUSY
#define SAM_OSCCTRL_DPLLSTATUS(n)        SAM_OSCCTRL_STATUS

#define OSCCTRL_DPLLCTRLA_ENABLE         OSCCTRL_PLL0CTRL_ENABLE
#define OSCCTRL_DPLLCTRLA_RUNSTDBY       0u
#define OSCCTRL_DPLLCTRLA_ONDEMAND       0u
#define OSCCTRL_DPLLSYNCBUSY_ENABLE      OSCCTRL_SYNCBUSY_PLL0CTRL
#define OSCCTRL_DPLLSYNCBUSY_DPLLRATIO   0u
#define OSCCTRL_DPLLRATIO_LDR_SHIFT      0
#define OSCCTRL_DPLLRATIO_LDR_MASK       0u
#define OSCCTRL_DPLLRATIO_LDRFRAC_SHIFT  16
#define OSCCTRL_DPLLRATIO_LDRFRAC_MASK   0u
#define OSCCTRL_DPLLCTRLB_FILTER_SHIFT   0
#define OSCCTRL_DPLLCTRLB_REFCLK_SHIFT   5
#define OSCCTRL_DPLLCTRLB_LTIME_SHIFT    8
#define OSCCTRL_DPLLCTRLB_DCOFILTER_SHIFT 12
#define OSCCTRL_DPLLCTRLB_DIV_SHIFT      16
#define OSCCTRL_DPLLCTRLB_DCOEN          0u
#define OSCCTRL_DPLLCTRLB_LBYPASS        0u
#define OSCCTRL_DPLLCTRLB_WUF            0u
#define OSCCTRL_DPLLSTATUS_LOCK          OSCCTRL_STATUS_PLL0LOCK
#define OSCCTRL_DPLLSTATUS_CLKRDY        OSCCTRL_STATUS_PLL0LOCK

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_OSCCTRL_H */
