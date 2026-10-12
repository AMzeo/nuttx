/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_wdt.c
 *
 * PIC32CZ CA90 Watchdog Timer — NuttX lower-half driver.
 *
 * Register layout and timing verified against:
 *   Microchip PIC32CZ-CA90_DFP 1.7.168 component/wdt.h + instance/wdt.h
 *   Harmony CSP plib_wdt.c (csp_apps_pic32cz_ca8x_ca9x)
 *
 * Clock: OSCULP32K (~32 kHz) / 32 internal prescaler = ~1024 Hz counter.
 * PER=CYC4096 → timeout ≈ 4 s.  EWOFFSET=CYC2048 → EW at ≈ 2 s before reset.
 *
 * Usage:
 *   sam_wdt_initialize()  — configure + register as /dev/watchdog0
 *   open("/dev/watchdog0") + WDIOC_START  — arm the timer
 *   WDIOC_KEEPALIVE every <4 s  — prevent reset
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_WDT

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/timers/watchdog.h>

#include "arm_internal.h"
#include "hardware/sam_wdt.h"
#include "hardware/sam_mclk.h"
#include "sam_wdt.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_wdt_lowerhalf_s
{
  const struct watchdog_ops_s *ops;   /* Must be first — cast compatibility */
  bool started;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int wdt_start(FAR struct watchdog_lowerhalf_s *lower);
static int wdt_stop(FAR struct watchdog_lowerhalf_s *lower);
static int wdt_keepalive(FAR struct watchdog_lowerhalf_s *lower);
static int wdt_getstatus(FAR struct watchdog_lowerhalf_s *lower,
                         FAR struct watchdog_status_s *status);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct watchdog_ops_s g_wdt_ops =
{
  .start     = wdt_start,
  .stop      = wdt_stop,
  .keepalive = wdt_keepalive,
  .getstatus = wdt_getstatus,
  .settimeout = NULL,
  .capture    = NULL,
  .ioctl      = NULL,
};

static struct sam_wdt_lowerhalf_s g_wdt_lower =
{
  .ops     = &g_wdt_ops,
  .started = false,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void wdt_syncbusy_wait(void)
{
  while ((getreg32(SAM_WDT_SYNCBUSY) & WDT_SYNCBUSY_ALL) != 0)
    {
    }
}

/* Early-warning ISR — fires ~2 s before hardware reset.
 * Use _alert() (polled lowputc) — safe from ISR, minimal stack. */

static int wdt_ew_isr(int irq, FAR void *context, FAR void *arg)
{
  (void)irq;
  (void)context;
  (void)arg;

  /* Clear the EW interrupt flag */

  putreg8(WDT_INT_EW, SAM_WDT_INTFLAG);

  /* _alert()/syslog() acquires a mutex — silently dropped in ISR context
   * without CONFIG_SYSLOG_INTBUFFER.  Use arm_lowputc() (polled, ISR-safe). */

  {
    static const char msg[] = "\r\nWDT: early warning -- reset in ~2 s\r\n";
    const char *p;
    for (p = msg; *p; p++)
      {
        arm_lowputc(*p);
      }
  }

  return OK;
}

/****************************************************************************
 * Lower-half operations
 ****************************************************************************/

static int wdt_start(FAR struct watchdog_lowerhalf_s *lower)
{
  FAR struct sam_wdt_lowerhalf_s *priv =
      (FAR struct sam_wdt_lowerhalf_s *)lower;

  if (priv->started)
    {
      return OK;
    }

  /* Enable NVIC vector before starting hardware (EW ISR must be ready) */

  up_enable_irq(SAM_IRQ_WDT);

  /* Set ENABLE; poll until sync complete (Harmony WDT_Enable pattern) */

  putreg8(WDT_CTRLA_ENABLE, SAM_WDT_CTRLA);
  wdt_syncbusy_wait();

  priv->started = true;
  return OK;
}

static int wdt_stop(FAR struct watchdog_lowerhalf_s *lower)
{
  FAR struct sam_wdt_lowerhalf_s *priv =
      (FAR struct sam_wdt_lowerhalf_s *)lower;

  if (!priv->started)
    {
      return OK;
    }

  /* ALWAYSON is never set, so ENABLE can be cleared */

  up_disable_irq(SAM_IRQ_WDT);

  wdt_syncbusy_wait();
  putreg8(0, SAM_WDT_CTRLA);
  wdt_syncbusy_wait();

  priv->started = false;
  return OK;
}

/* Pet the dog.  Harmony pattern: skip if a previous clear is still syncing
 * to avoid double-writing the key during a write-sync window. */

static int wdt_keepalive(FAR struct watchdog_lowerhalf_s *lower)
{
  if ((getreg32(SAM_WDT_SYNCBUSY) & WDT_SYNCBUSY_CLEAR) == 0)
    {
      putreg8(WDT_CLEAR_KEY, SAM_WDT_CLEAR);
    }

  return OK;
}

static int wdt_getstatus(FAR struct watchdog_lowerhalf_s *lower,
                         FAR struct watchdog_status_s *status)
{
  FAR struct sam_wdt_lowerhalf_s *priv =
      (FAR struct sam_wdt_lowerhalf_s *)lower;

  status->flags    = priv->started ? (WDFLAGS_ACTIVE | WDFLAGS_RESET) : 0;
  status->timeout  = 4000;   /* Fixed: 4 s */
  status->timeleft = 0;      /* Counter not readable on CA90 WDT */

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int sam_wdt_initialize(void)
{
  uint32_t regval;

  /* 1. Enable APB clock for WDT (MCLK_ID_APB = 14 → CLKMSK[0] bit 14) */

  regval  = getreg32(SAM_MCLK_CLKMSK_ADDR(MCLK_ID_APB_WDT));
  regval |= SAM_MCLK_CLKMSK_BIT(MCLK_ID_APB_WDT);
  putreg32(regval, SAM_MCLK_CLKMSK_ADDR(MCLK_ID_APB_WDT));

  /* 2. Wait for any residual sync from reset state */

  wdt_syncbusy_wait();

  /* 3. Write CONFIG while WDT is disabled.
   *    Reset value = 0xBB (PER=CYC16384, WINDOW=CYC16384) — must override.
   *    Write PER=CYC4096 (0x9), WINDOW=0 (normal mode, window unused). */

  putreg8(WDT_CONFIG_PER_CYC4096, SAM_WDT_CONFIG);

  /* 4. Write EWCTRL: EWOFFSET=CYC2048 → EW fires ~2 s before reset */

  putreg8(WDT_EWCTRL_EWOFFSET_CYC2048, SAM_WDT_EWCTRL);

  /* 5. Enable early-warning interrupt */

  putreg8(WDT_INT_EW, SAM_WDT_INTENSET);

  /* 6. Attach EW ISR and enable NVIC now */

  irq_attach(SAM_IRQ_WDT, wdt_ew_isr, NULL);
  up_enable_irq(SAM_IRQ_WDT);

  /* 7. Start the hardware timer immediately — do not wait for WDIOC_START.
   *    If the system hangs before wdt_pet_task opens the device the WDT fires. */

  putreg8(WDT_CTRLA_ENABLE, SAM_WDT_CTRLA);
  wdt_syncbusy_wait();
  g_wdt_lower.started = true;

  /* 8. Register NuttX watchdog upper-half at CONFIG_WATCHDOG_DEVPATH */

  FAR void *handle = watchdog_register(CONFIG_WATCHDOG_DEVPATH,
                                        (FAR struct watchdog_lowerhalf_s *)&g_wdt_lower);
  return (handle != NULL) ? OK : -EIO;
}

#endif /* CONFIG_PIC32CZCA90_WDT */
