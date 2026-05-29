/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_i2c_master.c
 *
 * PIC32CZ CA90 SERCOM I2C master driver (interrupt-driven, SMEN=1).
 *
 * Architecture: NuttX semaphore-wait + ISR state machine.
 * Matches Harmony plib_sercom5_i2c_master.c ISR pattern:
 *   - MB fires after address/data byte transmitted (write path)
 *   - SB fires after data byte received (read path)
 *   - SMEN auto-ACK on DATA read for non-last bytes
 *   - CMD=3+ACKACT=1 for last byte (NACK+STOP)
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_SERCOM5_ISI2C

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/mutex.h>
#include <nuttx/semaphore.h>
#include <nuttx/clock.h>

#include <syslog.h>

#include "arm_internal.h"
#include "sam_port.h"
#include "sam_sercom.h"
#include "sam_gclk.h"
#include "hardware/pic32czca90_memorymap.h"
#include "hardware/sam_sercom_i2c.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_gclk.h"
#include "hardware/pic32czca90_pinmap.h"
#include "sam_i2c_master.h"

#include <arch/pic32czca90/pic32czca90_irq.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define I2C5_GCLK_GEN      2
#define I2C5_GCLK_FREQ     100000000u
#define I2C5_SERCOM        5
#define I2C5_BASE           SAM_SERCOM5_BASE
#define I2C5_DEFAULT_FREQ   400000u

#define I2C5_IRQ_FIRST      SAM_IRQ_SERCOM5_6
#define I2C5_IRQ_COUNT      7

#define I2C_TIMEOUT_USEC    50000u

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_i2cdev_s
{
  struct i2c_master_s dev;
  uintptr_t           base;
  uint32_t            frequency;
  mutex_t             lock;
  sem_t               waitsem;
  volatile int        result;
  struct i2c_msg_s   *msg;
  volatile int        xfrd;
  bool                is_read;
  bool                last_msg;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sam_i2c_transfer(FAR struct i2c_master_s *dev,
                            FAR struct i2c_msg_s *msgs, int count);
#ifdef CONFIG_I2C_RESET
static int sam_i2c_reset(FAR struct i2c_master_s *dev);
#endif
static int i2c_interrupt(int irq, FAR void *context, FAR void *arg);

static volatile uint32_t g_isr_count = 0;
static volatile uint32_t g_isr_mb = 0;
static volatile uint32_t g_isr_sb = 0;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct i2c_ops_s g_i2c5_ops =
{
  .transfer = sam_i2c_transfer,
#ifdef CONFIG_I2C_RESET
  .reset    = sam_i2c_reset,
#endif
};

static struct sam_i2cdev_s g_i2c5_dev =
{
  .dev       = { .ops = &g_i2c5_ops },
  .base      = I2C5_BASE,
  .frequency = 0,
  .lock      = NXMUTEX_INITIALIZER,
};

/****************************************************************************
 * Private Helpers
 ****************************************************************************/

static inline void i2c_wait_syncbusy(uintptr_t base)
{
  while (getreg32(base + SAM_I2C_SYNCBUSY_OFFSET) != 0)
    {
    }
}

static void i2c_wakeup(FAR struct sam_i2cdev_s *priv, int result)
{
  priv->result = result;
  nxsem_post(&priv->waitsem);
}

static void i2c_hw_setfrequency(FAR struct sam_i2cdev_s *priv, uint32_t freq)
{
  uintptr_t base = priv->base;

  if (priv->frequency == freq)
    {
      return;
    }

  uint32_t ctrla = getreg32(base + SAM_I2C_CTRLA_OFFSET);
  putreg32(ctrla & ~I2C_CTRLA_ENABLE, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  uint32_t baud = (I2C5_GCLK_FREQ / (2u * freq)) - 5u;
  if (baud > 255)
    {
      baud = 255;
    }

  putreg32(baud & 0xFFu, base + SAM_I2C_BAUD_OFFSET);

  putreg32(ctrla, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  putreg16(I2C_STATUS_BUSSTATE_IDLE, base + SAM_I2C_STATUS_OFFSET);
  i2c_wait_syncbusy(base);

  priv->frequency = freq;
}

/****************************************************************************
 * Interrupt Handler
 ****************************************************************************/

static int i2c_interrupt(int irq, FAR void *context, FAR void *arg)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)arg;
  uintptr_t base = priv->base;
  uint16_t status;
  uint8_t intflag;

  g_isr_count++;
  intflag = getreg8(base + SAM_I2C_INTFLAG_OFFSET);
  status = getreg16(base + SAM_I2C_STATUS_OFFSET);

  if (intflag & I2C_INT_MB) g_isr_mb++;
  if (intflag & I2C_INT_SB) g_isr_sb++;

  /* Error: arbitration lost */

  if (status & I2C_STATUS_ARBLOST)
    {
      putreg16(I2C_STATUS_ARBLOST, base + SAM_I2C_STATUS_OFFSET);
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
      i2c_wakeup(priv, -EAGAIN);
      return OK;
    }

  /* Error: bus error */

  if (status & I2C_STATUS_BUSERR)
    {
      putreg16(I2C_STATUS_BUSERR, base + SAM_I2C_STATUS_OFFSET);
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
      i2c_wakeup(priv, -EIO);
      return OK;
    }

  /* MB: Master on Bus — write path (address sent or data byte sent) */

  if (intflag & I2C_INT_MB)
    {
      if (status & I2C_STATUS_RXNACK)
        {
          /* Slave NACKed — issue STOP and report error */

          uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
          ctrlb |= I2C_CTRLB_CMD_STOP;
          putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
          putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
          i2c_wakeup(priv, -ENXIO);
          return OK;
        }

      if (priv->is_read)
        {
          /* Address phase for read completed (MB fires after addr ACK).
           * First data byte is now being clocked in. Do nothing here —
           * SB will fire when the byte is ready. */

          putreg8(I2C_INT_MB, base + SAM_I2C_INTFLAG_OFFSET);
          return OK;
        }

      /* Write path: address or data byte was ACKed */

      if (priv->xfrd >= priv->msg->length)
        {
          /* All bytes sent */

          if (priv->last_msg)
            {
              uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
              ctrlb &= ~I2C_CTRLB_CMD_MASK;
              ctrlb |= I2C_CTRLB_CMD_STOP;
              putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
            }

          putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
          i2c_wakeup(priv, OK);
          return OK;
        }

      /* Send next data byte */

      putreg32(priv->msg->buffer[priv->xfrd++],
               base + SAM_I2C_DATA_OFFSET);
      return OK;
    }

  /* SB: Slave on Bus — read path (data byte received) */

  if (intflag & I2C_INT_SB)
    {
      bool last_byte = (priv->xfrd == priv->msg->length - 1);

      if (last_byte)
        {
          /* Last byte: NACK + STOP (or repeated START) before reading */

          uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
          ctrlb &= ~I2C_CTRLB_CMD_MASK;
          ctrlb |= I2C_CTRLB_ACKACT;

          if (priv->last_msg)
            {
              ctrlb |= I2C_CTRLB_CMD_STOP;
            }
          else
            {
              ctrlb |= I2C_CTRLB_CMD_RESTART;
            }

          putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
        }

      /* Read DATA — for non-last bytes SMEN auto-ACK starts next byte */

      priv->msg->buffer[priv->xfrd++] =
          (uint8_t)(getreg32(base + SAM_I2C_DATA_OFFSET) & 0xFF);

      if (priv->xfrd >= priv->msg->length)
        {
          /* All bytes received */

          putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
          i2c_wakeup(priv, OK);
          return OK;
        }

      return OK;
    }

  /* Unexpected interrupt — clear and ignore */

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
  return OK;
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int sam_i2c_transfer(FAR struct i2c_master_s *dev,
                            FAR struct i2c_msg_s *msgs, int count)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)dev;
  uintptr_t base = priv->base;
  int ret;

  nxmutex_lock(&priv->lock);

  /* Set frequency if changed */

  if (msgs[0].frequency != priv->frequency)
    {
      i2c_hw_setfrequency(priv, msgs[0].frequency);
    }

  for (int i = 0; i < count; i++)
    {
      FAR struct i2c_msg_s *msg = &msgs[i];
      priv->msg = msg;
      priv->xfrd = 0;
      priv->result = -EBUSY;
      priv->is_read = (msg->flags & I2C_M_READ) != 0;
      priv->last_msg = (i == count - 1);

      /* Wait for bus IDLE (first msg) or OWNER (subsequent) */

      {
        uint32_t guard = 1000000u;
        while (1)
          {
            uint16_t st = getreg16(base + SAM_I2C_STATUS_OFFSET);
            uint16_t busstate = st & I2C_STATUS_BUSSTATE_MASK;

            if (busstate == I2C_STATUS_BUSSTATE_IDLE)
              {
                break;
              }

            if (i > 0 && busstate == I2C_STATUS_BUSSTATE_OWNER)
              {
                break;
              }

            if (--guard == 0)
              {
                nxmutex_unlock(&priv->lock);
                return -EBUSY;
              }
          }
      }

      /* Clear all interrupt flags */

      putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);

      /* Clear ACKACT (set ACK mode for reads) */

      {
        uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
        ctrlb &= ~(I2C_CTRLB_ACKACT | I2C_CTRLB_CMD_MASK);
        putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
        i2c_wait_syncbusy(base);
      }

      /* Enable MB + SB + ERROR interrupts */

      putreg8(I2C_INT_MB | I2C_INT_SB | I2C_INT_ERROR,
              base + SAM_I2C_INTENSET_OFFSET);

      /* Write ADDR — triggers START (or repeated START if OWNER) */

      uint32_t addr_reg = I2C_ADDR_ADDR(msg->addr);
      if (priv->is_read)
        {
          addr_reg |= I2C_ADDR_RD;
        }

      putreg32(addr_reg, base + SAM_I2C_ADDR_OFFSET);
      i2c_wait_syncbusy(base);

      /* Wait for ISR to complete the message (with timeout) */

      ret = nxsem_tickwait_uninterruptible(&priv->waitsem,
              USEC2TICK(I2C_TIMEOUT_USEC));

      /* Disable all SERCOM interrupts */

      putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

      if (ret < 0)
        {
          static uint32_t tmo_cnt = 0;
          if (tmo_cnt++ < 3)
            {
              syslog(LOG_ERR,
                     "I2C5 TIMEOUT: isr=%lu mb=%lu sb=%lu "
                     "IF=0x%02x ST=0x%04x INTEN=0x%02x\n",
                     (unsigned long)g_isr_count,
                     (unsigned long)g_isr_mb,
                     (unsigned long)g_isr_sb,
                     (unsigned)getreg8(base + SAM_I2C_INTFLAG_OFFSET),
                     (unsigned)getreg16(base + SAM_I2C_STATUS_OFFSET),
                     (unsigned)getreg8(base + SAM_I2C_INTENSET_OFFSET));
            }

          putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
          i2c_wait_syncbusy(base);
          priv->frequency = 0;
          sam_i2cbus_initialize(I2C5_SERCOM);
          nxmutex_unlock(&priv->lock);
          return -ETIMEDOUT;
        }

      if (priv->result < 0)
        {
          nxmutex_unlock(&priv->lock);
          return priv->result;
        }
    }

  nxmutex_unlock(&priv->lock);
  return OK;
}

#ifdef CONFIG_I2C_RESET
static int sam_i2c_reset(FAR struct i2c_master_s *dev)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)dev;
  uintptr_t base = priv->base;

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  priv->frequency = 0;
  nxsem_reset(&priv->waitsem, 0);
  return sam_i2cbus_initialize(I2C5_SERCOM) ? OK : -EIO;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR struct i2c_master_s *sam_i2cbus_initialize(int port)
{
  if (port != I2C5_SERCOM)
    {
      return NULL;
    }

  FAR struct sam_i2cdev_s *priv = &g_i2c5_dev;

  if (priv->frequency != 0)
    {
      return &priv->dev;
    }

  uintptr_t base = priv->base;

  /* Initialize semaphore (starts at 0 — first wait blocks) */

  nxsem_init(&priv->waitsem, 0, 0);

  /* 1. Enable MCLK APB clock for SERCOM5 */

  sercom_enable(I2C5_SERCOM);

  /* 2. Route GCLK2 (100 MHz) to SERCOM5 core clock */

  sam_gclk_chan_enable(GCLK_CHAN_SERCOM5_CORE, I2C5_GCLK_GEN, false);

  /* 3. Configure GPIO pins: SDA=PC25/PAD0, SCL=PC26/PAD1 */

  sam_portconfig(PORT_SERCOM5_PAD0);
  sam_portconfig(PORT_SERCOM5_PAD1);

  /* 4. Software reset */

  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  /* 5. CTRLB: Smart mode enabled (Harmony pattern) */

  putreg32(I2C_CTRLB_SMEN, base + SAM_I2C_CTRLB_OFFSET);
  i2c_wait_syncbusy(base);

  /* 6. BAUD: 400 kHz */

  putreg32(120u, base + SAM_I2C_BAUD_OFFSET);

  /* 7. CTRLA: I2C master, SDA hold 75ns, FM slew rate, enable */

  putreg32(I2C_CTRLA_MODE_I2CM |
           I2C_CTRLA_SDAHOLD_75NS |
           I2C_CTRLA_SPEED_SM |
           I2C_CTRLA_SLEWRATE_FM |
           I2C_CTRLA_ENABLE,
           base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  /* 8. Force bus state to IDLE */

  putreg16(I2C_STATUS_BUSSTATE_IDLE, base + SAM_I2C_STATUS_OFFSET);
  i2c_wait_syncbusy(base);

  /* 9. Attach and enable only MB/SB/ERROR NVIC vectors.
   * CA90 SERCOM5 has 7 lines — only enable the 3 we need.
   * Line mapping (from pic32czca90_irq.h / DFP):
   *   SERCOM5_6 (EXTINT+90) = ERROR
   *   SERCOM5_0 (EXTINT+92) = MB (Master on Bus)
   *   SERCOM5_1 (EXTINT+93) = SB (Slave on Bus)
   * Do NOT enable TXFE/RXFF lines — FIFO flags can cause
   * spurious ISR re-entry if the hardware gates them differently. */

  irq_attach(SAM_IRQ_SERCOM5_6, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_0, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_1, i2c_interrupt, priv);
  up_enable_irq(SAM_IRQ_SERCOM5_6);
  up_enable_irq(SAM_IRQ_SERCOM5_0);
  up_enable_irq(SAM_IRQ_SERCOM5_1);

  /* Do NOT enable SERCOM interrupts yet — enabled per-transfer */

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

  priv->frequency = I2C5_DEFAULT_FREQ;

  syslog(LOG_ERR, "I2C5 ISR init: CTRLA=0x%08lx CTRLB=0x%08lx "
         "BAUD=0x%08lx STATUS=0x%04x\n",
         (unsigned long)getreg32(base + SAM_I2C_CTRLA_OFFSET),
         (unsigned long)getreg32(base + SAM_I2C_CTRLB_OFFSET),
         (unsigned long)getreg32(base + SAM_I2C_BAUD_OFFSET),
         (unsigned)getreg16(base + SAM_I2C_STATUS_OFFSET));

  return &priv->dev;
}

#endif /* CONFIG_PIC32CZCA90_SERCOM5_ISI2C */
