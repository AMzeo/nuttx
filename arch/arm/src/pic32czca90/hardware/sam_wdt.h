/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_wdt.h
 *
 * PIC32CZ CA90 Watchdog Timer (WDT) register definitions.
 * DFP-verified: component/wdt.h + instance/wdt.h (PIC32CZ-CA90_DFP 1.7.168)
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_WDT_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_WDT_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* MCLK APB clock ID — DFP instance/wdt.h: WDT_MCLK_ID_APB = 14
 * Maps to CLKMSK[0] bit 14 via SAM_MCLK_CLKMSK_ADDR(14)/SAM_MCLK_CLKMSK_BIT(14).
 * No GCLK needed: counter runs on OSCULP32K (~32 kHz) divided by 32 internally
 * giving a ~1024 Hz counter clock. */

#define MCLK_ID_APB_WDT         14u

/* WDT register offsets (DFP component/wdt.h) */

#define SAM_WDT_CTRLA_OFFSET    0x00  /* (R/W  8) Control                       */
#define SAM_WDT_CONFIG_OFFSET   0x01  /* (R/W  8) Configuration                 */
#define SAM_WDT_EWCTRL_OFFSET   0x02  /* (R/W  8) Early Warning Interrupt Control */
#define SAM_WDT_INTENCLR_OFFSET 0x04  /* (R/W  8) Interrupt Enable Clear        */
#define SAM_WDT_INTENSET_OFFSET 0x05  /* (R/W  8) Interrupt Enable Set          */
#define SAM_WDT_INTFLAG_OFFSET  0x06  /* (R/W  8) Interrupt Flag Status/Clear   */
#define SAM_WDT_SYNCBUSY_OFFSET 0x08  /* (R/   32) Synchronization Busy         */
#define SAM_WDT_CLEAR_OFFSET    0x0c  /* ( /W  8) Clear (write key 0xA5)        */

/* WDT register absolute addresses */

#define SAM_WDT_CTRLA    (SAM_WDT_BASE + SAM_WDT_CTRLA_OFFSET)
#define SAM_WDT_CONFIG   (SAM_WDT_BASE + SAM_WDT_CONFIG_OFFSET)
#define SAM_WDT_EWCTRL   (SAM_WDT_BASE + SAM_WDT_EWCTRL_OFFSET)
#define SAM_WDT_INTENCLR (SAM_WDT_BASE + SAM_WDT_INTENCLR_OFFSET)
#define SAM_WDT_INTENSET (SAM_WDT_BASE + SAM_WDT_INTENSET_OFFSET)
#define SAM_WDT_INTFLAG  (SAM_WDT_BASE + SAM_WDT_INTFLAG_OFFSET)
#define SAM_WDT_SYNCBUSY (SAM_WDT_BASE + SAM_WDT_SYNCBUSY_OFFSET)
#define SAM_WDT_CLEAR    (SAM_WDT_BASE + SAM_WDT_CLEAR_OFFSET)

/* WDT_CTRLA bit definitions (8-bit register, reset = 0x00) */

#define WDT_CTRLA_ENABLE        (1u << 1)   /* Enable */
#define WDT_CTRLA_WEN           (1u << 2)   /* Window Mode Enable */
#define WDT_CTRLA_RUNSTDBY      (1u << 6)   /* Run During Standby */
#define WDT_CTRLA_ALWAYSON      (1u << 7)   /* Always-On — DO NOT SET; locks out MPLAB */

/* WDT_CONFIG bit definitions (8-bit register, reset = 0xBB)
 * PER[3:0] = timeout period; must overwrite before enabling since reset = 0xBB.
 * WINDOW[7:4] = window period (normal mode: irrelevant, write 0). */

#define WDT_CONFIG_PER_MASK     (0xfu << 0)
#define WDT_CONFIG_PER_CYC4096  (0x9u << 0)   /* 4096 / 1024 Hz ≈ 4 s timeout */

/* WDT_EWCTRL bit definitions (8-bit register, reset = 0x0B)
 * EWOFFSET[3:0]: EW fires (timeout_cycles - ewoffset_cycles) before reset.
 * EWOFFSET=CYC2048 → EW at 4096-2048 = 2048 cyc before reset ≈ 2 s warning. */

#define WDT_EWCTRL_EWOFFSET_MASK     (0xfu << 0)
#define WDT_EWCTRL_EWOFFSET_CYC2048  (0x8u << 0)

/* WDT_INTENCLR / WDT_INTENSET / WDT_INTFLAG — EW bit (bit 0) */

#define WDT_INT_EW              (1u << 0)

/* WDT_SYNCBUSY bit definitions (32-bit register) */

#define WDT_SYNCBUSY_ENABLE     (1u << 1)
#define WDT_SYNCBUSY_WEN        (1u << 2)
#define WDT_SYNCBUSY_RUNSTDBY   (1u << 3)
#define WDT_SYNCBUSY_ALWAYSON   (1u << 4)
#define WDT_SYNCBUSY_CLEAR      (1u << 5)
#define WDT_SYNCBUSY_ALL        (0x3eu)

/* WDT_CLEAR key — write to WDT_CLEAR to reset the counter ("pet the dog") */

#define WDT_CLEAR_KEY           0xa5u

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_WDT_H */
