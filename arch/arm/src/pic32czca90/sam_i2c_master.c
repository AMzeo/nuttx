/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_i2c_master.c
 *
 * PIC32CZ CA90 SERCOM I2C master driver (polled mode).
 *
 * Harmony reference: plib_sercom5_i2c_master.c
 * Init: SWRST → CTRLB(SMEN) → BAUD → CTRLA(I2CM+ENABLE) → STATUS(IDLE)
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
#include <nuttx/clock.h>

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

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define I2C5_GCLK_GEN      2
#define I2C5_GCLK_FREQ     100000000u
#define I2C5_SERCOM        5
#define I2C5_BASE           SAM_SERCOM5_BASE
#define I2C5_DEFAULT_FREQ   400000u

#define I2C_TIMEOUT_MS      100

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_i2cdev_s
{
  struct i2c_master_s dev;
  uintptr_t           base;
  uint32_t            frequency;
  mutex_t             lock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sam_i2c_transfer(FAR struct i2c_master_s *dev,
                            FAR struct i2c_msg_s *msgs, int count);
#ifdef CONFIG_I2C_RESET
static int sam_i2c_reset(FAR struct i2c_master_s *dev);
#endif

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

static void i2c_set_frequency(FAR struct sam_i2cdev_s *priv, uint32_t freq)
{
  if (priv->frequency == freq)
    {
      return;
    }

  uintptr_t base = priv->base;

  /* Disable to change baud */

  uint32_t ctrla = getreg32(base + SAM_I2C_CTRLA_OFFSET);
  putreg32(ctrla & ~I2C_CTRLA_ENABLE, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  /* BAUD = (f_GCLK / (2 * f_SCL)) - 5   (simplified, ignoring rise time) */

  uint32_t baud = (I2C5_GCLK_FREQ / (2u * freq)) - 5u;
  if (baud > 255)
    {
      baud = 255;
    }

  putreg32(baud & 0xFFu, base + SAM_I2C_BAUD_OFFSET);

  /* Re-enable */

  putreg32(ctrla, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  priv->frequency = freq;
}

static int i2c_wait_busowner(uintptr_t base)
{
  clock_t start = clock_systime_ticks();
  clock_t timeout = MSEC2TICK(I2C_TIMEOUT_MS);

  while (1)
    {
      uint8_t flags = getreg8(base + SAM_I2C_INTFLAG_OFFSET);
      if (flags & (I2C_INT_MB | I2C_INT_SB))
        {
          return OK;
        }

      if (flags & I2C_INT_ERROR)
        {
          return -EIO;
        }

      if ((clock_systime_ticks() - start) > timeout)
        {
          return -ETIMEDOUT;
        }
    }
}

static int i2c_check_errors(uintptr_t base)
{
  uint16_t status = getreg16(base + SAM_I2C_STATUS_OFFSET);

  if (status & I2C_STATUS_ARBLOST)
    {
      putreg16(I2C_STATUS_ARBLOST, base + SAM_I2C_STATUS_OFFSET);
      return -EAGAIN;
    }

  if (status & I2C_STATUS_BUSERR)
    {
      putreg16(I2C_STATUS_BUSERR, base + SAM_I2C_STATUS_OFFSET);
      return -EIO;
    }

  if (status & I2C_STATUS_RXNACK)
    {
      return -ENXIO;
    }

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

  for (int i = 0; i < count; i++)
    {
      FAR struct i2c_msg_s *msg = &msgs[i];
      bool is_read = (msg->flags & I2C_M_READ) != 0;
      bool is_last = (i == count - 1);

      /* Set frequency if changed */

      if (msg->frequency != priv->frequency)
        {
          i2c_set_frequency(priv, msg->frequency);
        }

      /* Write address + R/W bit to ADDR register (triggers START) */

      uint32_t addr_reg = I2C_ADDR_ADDR(msg->addr);
      if (is_read)
        {
          addr_reg |= I2C_ADDR_RD;
        }

      putreg32(addr_reg, base + SAM_I2C_ADDR_OFFSET);
      i2c_wait_syncbusy(base);

      /* Wait for address phase to complete (MB flag for write, SB for read) */

      ret = i2c_wait_busowner(base);
      if (ret < 0)
        {
          goto stop;
        }

      ret = i2c_check_errors(base);
      if (ret < 0)
        {
          goto stop;
        }

      if (is_read)
        {
          /* Read transfer */

          for (ssize_t j = 0; j < msg->length; j++)
            {
              bool last_byte = (j == msg->length - 1);

              if (last_byte && is_last)
                {
                  /* NACK + STOP after last byte of last message */

                  putreg32(I2C_CTRLB_ACKACT | I2C_CTRLB_CMD_STOP,
                           base + SAM_I2C_CTRLB_OFFSET);
                }
              else if (last_byte && !is_last)
                {
                  /* NACK (no stop, next message will restart) */

                  putreg32(I2C_CTRLB_ACKACT | I2C_CTRLB_CMD_RESTART,
                           base + SAM_I2C_CTRLB_OFFSET);
                }
              else
                {
                  /* ACK + continue reading */

                  putreg32(I2C_CTRLB_CMD_READ,
                           base + SAM_I2C_CTRLB_OFFSET);
                }

              i2c_wait_syncbusy(base);

              if (!last_byte || !is_last)
                {
                  ret = i2c_wait_busowner(base);
                  if (ret < 0)
                    {
                      goto stop;
                    }
                }

              msg->buffer[j] = (uint8_t)(getreg32(base + SAM_I2C_DATA_OFFSET) & 0xFF);
            }
        }
      else
        {
          /* Write transfer */

          for (ssize_t j = 0; j < msg->length; j++)
            {
              putreg32(msg->buffer[j], base + SAM_I2C_DATA_OFFSET);
              i2c_wait_syncbusy(base);

              ret = i2c_wait_busowner(base);
              if (ret < 0)
                {
                  goto stop;
                }

              ret = i2c_check_errors(base);
              if (ret < 0)
                {
                  goto stop;
                }
            }

          if (is_last)
            {
              /* Issue STOP */

              putreg32(I2C_CTRLB_CMD_STOP, base + SAM_I2C_CTRLB_OFFSET);
              i2c_wait_syncbusy(base);
            }
        }
    }

  nxmutex_unlock(&priv->lock);
  return OK;

stop:
  putreg32(I2C_CTRLB_CMD_STOP, base + SAM_I2C_CTRLB_OFFSET);
  i2c_wait_syncbusy(base);
  nxmutex_unlock(&priv->lock);
  return ret;
}

#ifdef CONFIG_I2C_RESET
static int sam_i2c_reset(FAR struct i2c_master_s *dev)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)dev;
  uintptr_t base = priv->base;

  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  priv->frequency = 0;
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
  uintptr_t base = priv->base;

  /* 1. Enable MCLK APB clock for SERCOM5 */

  sercom_enable(I2C5_SERCOM);

  /* 2. Route GCLK6 (100 MHz) to SERCOM5 core clock */

  sam_gclk_chan_enable(GCLK_CHAN_SERCOM5_CORE, I2C5_GCLK_GEN, false);

  /* 3. Configure GPIO pins: SDA=PC25/PAD0, SCL=PC26/PAD1 */

  sam_portconfig(PORT_SERCOM5_PAD0);  /* PC25 SDA */
  sam_portconfig(PORT_SERCOM5_PAD1);  /* PC26 SCL */

  /* 4. Software reset */

  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  /* 5. Enable smart mode */

  putreg32(I2C_CTRLB_SMEN, base + SAM_I2C_CTRLB_OFFSET);
  i2c_wait_syncbusy(base);

  /* 6. Set baud: 400 kHz
   *    BAUD = (100 MHz / (2 * 400000)) - 5 = 120 */

  putreg32(120u, base + SAM_I2C_BAUD_OFFSET);

  /* 7. Configure CTRLA: I2C master, SDA hold 75ns, standard/fast mode,
   *    FM slew rate, enable */

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

  priv->frequency = I2C5_DEFAULT_FREQ;

  i2cinfo("SERCOM5 I2C master initialized at %lu Hz\n",
          (unsigned long)priv->frequency);

  return &priv->dev;
}

#endif /* CONFIG_PIC32CZCA90_SERCOM5_ISI2C */
