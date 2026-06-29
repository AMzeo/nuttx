/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_i2c_master.c
 *
 * PIC32CZ CA90 SERCOM I2C master driver (interrupt-driven, SMEN=1).
 *
 * Architecture: NuttX semaphore-wait + ISR state machine.
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
#include "nvic.h"
#include "sam_port.h"
#include "sam_sercom.h"
#include "sam_gclk.h"
#include "hardware/pic32czca90_memorymap.h"
#include "hardware/sam_sercom_i2c.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_gclk.h"
#include "hardware/pic32czca90_pinmap.h"
#include "sam_i2c_master.h"

#ifdef CONFIG_PIC32CZCA90_DMAC
#include "sam_dmac.h"
#include "hardware/sam_dma.h"
#endif

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

#ifdef CONFIG_PIC32CZCA90_DMAC
#define I2C_DMA_THRESHOLD   3
#endif

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
  struct i2c_msg_s   *msgs;
  struct i2c_msg_s   *msg;
  volatile int        msgc;
  volatile int        xfrd;
  bool                is_read;
#ifdef CONFIG_PIC32CZCA90_DMAC
  DMA_HANDLE          rxdma;
  volatile bool       dma_active;
#endif
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
#ifdef CONFIG_PIC32CZCA90_DMAC
static void i2c_dma_callback(DMA_HANDLE handle, void *arg, int result);
#endif

static volatile uint32_t g_isr_count = 0;
static volatile uint32_t g_isr_mb = 0;
static volatile uint32_t g_isr_sb = 0;
#ifdef CONFIG_PIC32CZCA90_DMAC
static volatile uint32_t g_dma_rx_starts = 0;
static volatile uint32_t g_dma_rx_done = 0;
static volatile uint32_t g_read_mb = 0;
static volatile uint32_t g_read_mb_len = 0;
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

/* I2C bus recovery: toggle SCL 9 times as GPIO to free a stuck slave.
 * Must be called BEFORE configuring pins for SERCOM function.
 * Standard procedure per I2C spec §3.1.16 and NXP AN10216. */

#define I2C5_SDA_PIN   25
#define I2C5_SCL_PIN   26
#define I2C5_PORT_BASE SAM_PORTC_BASE

static void i2c_bus_recover(void)
{
  uintptr_t port = I2C5_PORT_BASE;
  uint32_t sda_mask = (1u << I2C5_SDA_PIN);
  uint32_t scl_mask = (1u << I2C5_SCL_PIN);
  volatile uint32_t delay;

  /* Disable PMUX — make both pins plain GPIO */

  uint8_t cfg;
  cfg = getreg8(port + SAM_PORT_PINCFG_OFFSET(I2C5_SCL_PIN));
  putreg8(cfg & ~PORT_PINCFG_PMUXEN, port + SAM_PORT_PINCFG_OFFSET(I2C5_SCL_PIN));

  cfg = getreg8(port + SAM_PORT_PINCFG_OFFSET(I2C5_SDA_PIN));
  putreg8((cfg & ~PORT_PINCFG_PMUXEN) | PORT_PINCFG_INEN,
          port + SAM_PORT_PINCFG_OFFSET(I2C5_SDA_PIN));

  /* SCL = output HIGH */

  putreg32(scl_mask, port + SAM_PORT_OUTSET_OFFSET);
  putreg32(scl_mask, port + SAM_PORT_DIRSET_OFFSET);

  /* SDA = input (read state) */

  putreg32(sda_mask, port + SAM_PORT_DIRCLR_OFFSET);

  /* Check if SDA is already high — bus is fine, skip recovery */

  for (delay = 0; delay < 500; delay++);
  if (getreg32(port + SAM_PORT_IN_OFFSET) & sda_mask)
    {
      return;  /* Bus OK — pins will be reconfigured by caller */
    }

  /* SDA is stuck LOW — clock SCL 9 times to free slave */

  for (int i = 0; i < 9; i++)
    {
      putreg32(scl_mask, port + SAM_PORT_OUTCLR_OFFSET);
      for (delay = 0; delay < 1500; delay++);  /* ~5 µs at 300 MHz */

      putreg32(scl_mask, port + SAM_PORT_OUTSET_OFFSET);
      for (delay = 0; delay < 1500; delay++);

      /* Check if slave released SDA */

      if (getreg32(port + SAM_PORT_IN_OFFSET) & sda_mask)
        {
          break;
        }
    }

  /* Generate STOP: SDA low→high while SCL is high */

  putreg32(sda_mask, port + SAM_PORT_OUTCLR_OFFSET);
  putreg32(sda_mask, port + SAM_PORT_DIRSET_OFFSET);  /* SDA = output LOW */
  for (delay = 0; delay < 1500; delay++);

  putreg32(scl_mask, port + SAM_PORT_OUTSET_OFFSET);  /* SCL HIGH */
  for (delay = 0; delay < 1500; delay++);

  putreg32(sda_mask, port + SAM_PORT_OUTSET_OFFSET);  /* SDA HIGH = STOP */
  for (delay = 0; delay < 1500; delay++);
}

static inline void i2c_wait_syncbusy(uintptr_t base)
{
  volatile uint32_t timeout = 100000u;

  while (getreg32(base + SAM_I2C_SYNCBUSY_OFFSET) != 0)
    {
      if (--timeout == 0)
        {
          break;
        }
    }
}

static void i2c_wakeup(FAR struct sam_i2cdev_s *priv, int result)
{
  priv->result = result;
  nxsem_post(&priv->waitsem);
}

/* Start next message from ISR context (repeated START).
 * Advances msg pointer, sets up read/write state, writes ADDR. */

static void i2c_start_next_msg(FAR struct sam_i2cdev_s *priv)
{
  uintptr_t base = priv->base;

  priv->msg++;
  priv->msgc--;
  priv->xfrd = 0;
  priv->is_read = (priv->msg->flags & I2C_M_READ) != 0;

  /* Ensure MB is enabled (defensive — should already be) */

  putreg8(I2C_INT_MB, base + SAM_I2C_INTENSET_OFFSET);

  /* Write ADDR with R/W bit — triggers repeated START in OWNER state */

  uint32_t addr_reg = I2C_ADDR_ADDR(priv->msg->addr);
  if (priv->is_read)
    {
      addr_reg |= I2C_ADDR_RD;

#ifdef CONFIG_PIC32CZCA90_DMAC
      /* Start DMA BEFORE writing ADDR so it's armed when first byte
       * arrives. On CA90 with SMEN=1, MB does NOT fire for read-address
       * ACK — hardware goes directly to SB. DMA trigger (SERCOM5_RX)
       * fires on each received byte, reading DATA auto-ACKs via SMEN. */

      if (priv->rxdma && priv->msg->length >= I2C_DMA_THRESHOLD)
        {
          size_t dma_len = priv->msg->length - 1;
          uint32_t paddr = base + SAM_I2C_DATA_OFFSET;
          uint32_t maddr = (uint32_t)priv->msg->buffer;

          sam_dmarxsetup(priv->rxdma, paddr, maddr, dma_len);
          sam_dmastart(priv->rxdma, i2c_dma_callback, priv);
          priv->dma_active = true;
          g_dma_rx_starts++;

          /* Disable SB so ISR doesn't compete with DMA for DATA reads.
           * DMA callback re-enables SB for last byte. */

          putreg8(I2C_INT_SB, base + SAM_I2C_INTENCLR_OFFSET);
        }
#endif
    }

  putreg32(addr_reg, base + SAM_I2C_ADDR_OFFSET);
  i2c_wait_syncbusy(base);
}

/* Hardware-only reinit after SWRST — does NOT touch semaphore or IRQ. */

static void i2c_hw_reinit(FAR struct sam_i2cdev_s *priv)
{
  uintptr_t base = priv->base;

  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  putreg32(I2C_CTRLB_SMEN, base + SAM_I2C_CTRLB_OFFSET);
  i2c_wait_syncbusy(base);

  putreg32(120u, base + SAM_I2C_BAUD_OFFSET);

  putreg32(I2C_CTRLA_MODE_I2CM |
           I2C_CTRLA_SDAHOLD_75NS |
           I2C_CTRLA_SPEED_SM |
           I2C_CTRLA_SLEWRATE_FM |
           I2C_CTRLA_ENABLE,
           base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  putreg16(I2C_STATUS_BUSSTATE_IDLE, base + SAM_I2C_STATUS_OFFSET);
  i2c_wait_syncbusy(base);

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

  priv->frequency = I2C5_DEFAULT_FREQ;
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

#ifdef CONFIG_PIC32CZCA90_DMAC
/****************************************************************************
 * Name: i2c_dma_callback
 *
 * DMA RX complete: N-1 bytes received. Re-enable SB interrupt so the ISR
 * handles the last byte with NACK+STOP (or repeated START for next msg).
 ****************************************************************************/

static void i2c_dma_callback(DMA_HANDLE handle, void *arg, int result)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)arg;
  uintptr_t base = priv->base;

  priv->dma_active = false;

  if (result != OK)
    {
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
      i2c_wakeup(priv, -EIO);
      return;
    }

  g_dma_rx_done++;
  priv->xfrd = priv->msg->length - 1;

  putreg8(I2C_INT_SB | I2C_INT_MB | I2C_INT_ERROR,
          base + SAM_I2C_INTENSET_OFFSET);
}
#endif

/****************************************************************************
 * Interrupt Handler
 ****************************************************************************/

static volatile uint32_t g_isr_burst = 0;

static int i2c_interrupt(int irq, FAR void *context, FAR void *arg)
{
  FAR struct sam_i2cdev_s *priv = (FAR struct sam_i2cdev_s *)arg;
  uintptr_t base = priv->base;
  uint16_t status;
  uint8_t intflag;

  g_isr_count++;
  g_isr_burst++;

  /* Runaway ISR protection — if >100 interrupts fire for one transfer,
   * the state machine is stuck. Issue SWRST, wake caller with error.
   * Save intflag/status in globals for task-context diagnostics. */

  if (g_isr_burst > 100)
    {
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
      putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
      putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
      priv->frequency = 0;
      i2c_wakeup(priv, -EIO);
      return OK;
    }

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
          ctrlb &= ~I2C_CTRLB_CMD_MASK;
          ctrlb |= I2C_CTRLB_CMD_STOP;
          putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
          i2c_wait_syncbusy(base);
          putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
          i2c_wakeup(priv, -ENXIO);
          return OK;
        }

      if (priv->is_read)
        {
          /* Read-address phase ACKed (MB fires after addr+R sent).
           * MUST clear INTFLAG[0] to release SCL hold — CA90 hardware
           * holds SCL LOW while INTFLAG.MB is set.  Clearing it allows
           * the master to clock in the data byte, which fires SB.
           *
           * NOTE: On CA90 with SMEN=1, MB does NOT fire for read-address
           * ACK — hardware goes directly to SB (first byte received).
           * This block is dead code but kept for documentation/safety.
           * DMA is started from i2c_start_next_msg() instead. */

          putreg8(I2C_INT_MB, base + SAM_I2C_INTFLAG_OFFSET);
          return OK;
        }

      /* Write path: address or data byte was ACKed */

      if (priv->xfrd >= priv->msg->length)
        {
          /* All bytes sent */

          if (priv->msgc <= 1)
            {
              /* Last message — issue STOP, disable ints, wake caller */

              uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
              ctrlb &= ~I2C_CTRLB_CMD_MASK;
              ctrlb |= I2C_CTRLB_CMD_STOP;
              putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
              i2c_wait_syncbusy(base);
              putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
              putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
              i2c_wakeup(priv, OK);
              return OK;
            }

          /* More messages — issue repeated START from ISR */

          i2c_start_next_msg(priv);
          return OK;
        }

      /* Send next data byte */

      putreg32(priv->msg->buffer[priv->xfrd++],
               base + SAM_I2C_DATA_OFFSET);
      i2c_wait_syncbusy(base);
      return OK;
    }

  /* SB: Slave on Bus — read path (data byte received) */

  if (intflag & I2C_INT_SB)
    {
      bool last_byte = (priv->xfrd == priv->msg->length - 1);

      if (last_byte)
        {
          /* Last byte: NACK + STOP (or NACK only if more messages) */

          uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
          ctrlb &= ~I2C_CTRLB_CMD_MASK;
          ctrlb |= I2C_CTRLB_ACKACT;

          if (priv->msgc <= 1)
            {
              ctrlb |= I2C_CTRLB_CMD_STOP;
            }
          else
            {
              ctrlb |= I2C_CTRLB_CMD_RESTART;
            }

          putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
          i2c_wait_syncbusy(base);
        }

      /* Wait for any pending sync before reading DATA */

      i2c_wait_syncbusy(base);

      /* Read DATA — for non-last bytes SMEN auto-ACK starts next byte */

      priv->msg->buffer[priv->xfrd++] =
          (uint8_t)(getreg32(base + SAM_I2C_DATA_OFFSET) & 0xFF);

      if (priv->xfrd >= priv->msg->length)
        {
          if (priv->msgc <= 1)
            {
              /* Last message done — disable ints, wake caller */

              putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);
              putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
              i2c_wakeup(priv, OK);
              return OK;
            }

          /* More messages — start next from ISR (repeated START) */

          i2c_start_next_msg(priv);
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

  /* Drain any stale semaphore posts from previous transfer completion
   * racing with timeout.  Without this, sem_wait returns immediately
   * while ADDR is already on the bus → bus stuck in OWNER. */

  while (nxsem_trywait(&priv->waitsem) == OK);

  /* Set frequency if changed */

  if (msgs[0].frequency != priv->frequency)
    {
      i2c_hw_setfrequency(priv, msgs[0].frequency);
    }

  /* Set up the full message list — ISR handles multi-message sequencing */

  priv->msgs = msgs;
  priv->msg = &msgs[0];
  priv->msgc = count;
  priv->xfrd = 0;
  priv->result = -EBUSY;
  priv->is_read = (msgs[0].flags & I2C_M_READ) != 0;
  g_isr_burst = 0;

  /* Wait for bus IDLE. If OWNER (stale from prior STOP), force IDLE. */

  {
    uint32_t guard = 10000u;
    while (1)
      {
        uint16_t st = getreg16(base + SAM_I2C_STATUS_OFFSET);
        uint16_t busstate = st & I2C_STATUS_BUSSTATE_MASK;

        if (busstate == I2C_STATUS_BUSSTATE_IDLE)
          {
            break;
          }

        if (busstate == I2C_STATUS_BUSSTATE_OWNER)
          {
            putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
            putreg16(I2C_STATUS_BUSSTATE_IDLE,
                     base + SAM_I2C_STATUS_OFFSET);
            i2c_wait_syncbusy(base);
            break;
          }

        if (--guard == 0)
          {
            nxmutex_unlock(&priv->lock);
            return -EBUSY;
          }
      }
  }

  /* ISR-driven transfer. ISR handles all messages via i2c_start_next_msg(). */

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTFLAG_OFFSET);
  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

  {
    uint32_t ctrlb = getreg32(base + SAM_I2C_CTRLB_OFFSET);
    ctrlb &= ~(I2C_CTRLB_ACKACT | I2C_CTRLB_CMD_MASK);
    putreg32(ctrlb, base + SAM_I2C_CTRLB_OFFSET);
    i2c_wait_syncbusy(base);
  }

  /* Enable MB + SB + ERROR SERCOM interrupts */

  putreg8(I2C_INT_MB | I2C_INT_SB | I2C_INT_ERROR,
          base + SAM_I2C_INTENSET_OFFSET);

  /* Write ADDR — triggers START condition */

  uint32_t addr_reg = I2C_ADDR_ADDR(msgs[0].addr);
  if (priv->is_read)
    {
      addr_reg |= I2C_ADDR_RD;

#ifdef CONFIG_PIC32CZCA90_DMAC
      if (priv->rxdma && priv->msg->length >= I2C_DMA_THRESHOLD)
        {
          size_t dma_len = priv->msg->length - 1;
          uint32_t paddr = base + SAM_I2C_DATA_OFFSET;
          uint32_t maddr = (uint32_t)priv->msg->buffer;

          sam_dmarxsetup(priv->rxdma, paddr, maddr, dma_len);
          sam_dmastart(priv->rxdma, i2c_dma_callback, priv);
          priv->dma_active = true;
          g_dma_rx_starts++;

          putreg8(I2C_INT_SB, base + SAM_I2C_INTENCLR_OFFSET);
        }
#endif
    }

  putreg32(addr_reg, base + SAM_I2C_ADDR_OFFSET);
  i2c_wait_syncbusy(base);

  /* Wait for ISR to complete ALL messages (with timeout) */

  ret = nxsem_tickwait_uninterruptible(&priv->waitsem,
          USEC2TICK(I2C_TIMEOUT_USEC));

  /* Disable all SERCOM interrupts */

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

  if (ret < 0)
    {
      /* Timeout: bus is stuck (IF=0, OWNER). STOP+force-IDLE doesn't
       * work when SCL is held low. Use SWRST to fully reset SERCOM,
       * then set frequency=0 so next transfer triggers full reinit. */

#ifdef CONFIG_PIC32CZCA90_DMAC
      if (priv->dma_active)
        {
          sam_dmastop(priv->rxdma);
          priv->dma_active = false;
        }
#endif

      static uint32_t tmo_count = 0;
      if (++tmo_count <= 5)
        {
          _alert("I2C5 timeout #%lu (xfrd=%d)\n",
                 (unsigned long)tmo_count, priv->xfrd);
        }

      i2c_hw_reinit(priv);

      nxmutex_unlock(&priv->lock);
      return -ETIMEDOUT;
    }

  if (priv->result < 0)
    {
      nxmutex_unlock(&priv->lock);
      return priv->result;
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
  nxsem_set_protocol(&priv->waitsem, SEM_PRIO_NONE);

  /* 1. Enable MCLK APB clock for SERCOM5 */

  sercom_enable(I2C5_SERCOM);

  /* 2. Route GCLK2 (100 MHz) to SERCOM5 core clock */

  sam_gclk_chan_enable(GCLK_CHAN_SERCOM5_CORE, I2C5_GCLK_GEN, false);

  /* 3. Bus recovery: clock out stuck slaves before enabling SERCOM */

  i2c_bus_recover();

  /* 4. Configure GPIO pins: SDA=PC25/PAD0, SCL=PC26/PAD1 */

  sam_portconfig(PORT_SERCOM5_PAD0);
  sam_portconfig(PORT_SERCOM5_PAD1);

  /* 4. Software reset */

  putreg32(I2C_CTRLA_SWRST, base + SAM_I2C_CTRLA_OFFSET);
  i2c_wait_syncbusy(base);

  /* 5. CTRLB: Smart mode enabled */

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

  /* 9. Attach and enable ALL 7 SERCOM5 NVIC vectors to the same handler.
   * All vectors → same ISR, handler reads INTFLAG.
   * Interrupt mapping for SERCOM5 in I2C mode:
   *   SERCOM5_6 (EXTINT+90) = I2C Error
   *   SERCOM5_5 (EXTINT+91) = (unused in I2C)
   *   SERCOM5_0 (EXTINT+92) = MB (DRE / I2C Stop Received)
   *   SERCOM5_1 (EXTINT+93) = TXC / I2C Address Match
   *   SERCOM5_2 (EXTINT+94) = I2C Data Ready (SB!)
   *   SERCOM5_3 (EXTINT+95) = TX FIFO Empty
   *   SERCOM5_4 (EXTINT+96) = RX FIFO Full */

  irq_attach(SAM_IRQ_SERCOM5_6, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_5, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_0, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_1, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_2, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_3, i2c_interrupt, priv);
  irq_attach(SAM_IRQ_SERCOM5_4, i2c_interrupt, priv);

  up_enable_irq(SAM_IRQ_SERCOM5_6);
  up_enable_irq(SAM_IRQ_SERCOM5_5);
  up_enable_irq(SAM_IRQ_SERCOM5_0);
  up_enable_irq(SAM_IRQ_SERCOM5_1);
  up_enable_irq(SAM_IRQ_SERCOM5_2);
  up_enable_irq(SAM_IRQ_SERCOM5_3);
  up_enable_irq(SAM_IRQ_SERCOM5_4);

  /* Do NOT enable SERCOM interrupts yet — enabled per-transfer */

  putreg8(I2C_INT_ALL, base + SAM_I2C_INTENCLR_OFFSET);

  priv->frequency = I2C5_DEFAULT_FREQ;

#ifdef CONFIG_PIC32CZCA90_DMAC
  {
    uint32_t chflags = DMACH_FLAG_PERIPHPID(DMAC_TRIG_SERCOM_RX(I2C5_SERCOM)) |
                       DMACH_FLAG_PRIORITY(0);
    priv->rxdma = sam_dmachannel(chflags);
    priv->dma_active = false;
    if (!priv->rxdma)
      {
        syslog(LOG_ERR, "I2C%d: DMA RX alloc failed\n", I2C5_SERCOM);
      }
  }
#endif

  return &priv->dev;
}

/****************************************************************************
 * Name: sam_i2c_print_stats
 *
 * Description:
 *   Print I2C ISR/DMA statistics to syslog. Call from board-level command.
 *
 ****************************************************************************/

void sam_i2c_print_stats(void)
{
  syslog(LOG_INFO, "I2C5: isr=%lu mb=%lu sb=%lu",
         (unsigned long)g_isr_count,
         (unsigned long)g_isr_mb,
         (unsigned long)g_isr_sb);
#ifdef CONFIG_PIC32CZCA90_DMAC
  syslog(LOG_INFO, " read_mb=%lu len=%lu dma_s=%lu dma_d=%lu",
         (unsigned long)g_read_mb,
         (unsigned long)g_read_mb_len,
         (unsigned long)g_dma_rx_starts,
         (unsigned long)g_dma_rx_done);
#endif
  syslog(LOG_INFO, "\n");
}

#endif /* CONFIG_PIC32CZCA90_SERCOM5_ISI2C */
