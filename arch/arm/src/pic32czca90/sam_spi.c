/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_spi.c
 *
 * PIC32CZ CA90 SERCOM SPI master driver (polled mode).
 *
 * Harmony reference: plib_sercom3_spi_master.c
 * Init sequence: CTRLB → BAUD → CTRLA(+ENABLE), SYNCBUSY after each.
 *
 ****************************************************************************/

#include <nuttx/config.h>

#ifdef CONFIG_PIC32CZCA90_SERCOM3_ISSPI

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/spi/spi.h>
#include <nuttx/mutex.h>

#include "arm_internal.h"
#include "sam_port.h"
#include "sam_sercom.h"
#include "sam_gclk.h"
#include "hardware/pic32czca90_memorymap.h"
#include "hardware/sam_sercom_spi.h"
#include "hardware/sam_mclk.h"
#include "hardware/sam_gclk.h"
#include "hardware/pic32czca90_pinmap.h"
#include "sam_spi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SPI3_GCLK_GEN      6
#define SPI3_GCLK_FREQ     100000000u
#define SPI3_SERCOM        3
#define SPI3_BASE           SAM_SERCOM3_BASE

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_spidev_s
{
  struct spi_dev_s dev;
  uintptr_t        base;
  uint32_t         frequency;
  uint32_t         actual;
  uint8_t          mode;
  uint8_t          nbits;
  mutex_t          lock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int      sam_spi_lock(FAR struct spi_dev_s *dev, bool lock);
static void     sam_spi_select(FAR struct spi_dev_s *dev, uint32_t devid,
                               bool selected);
static uint32_t sam_spi_setfrequency(FAR struct spi_dev_s *dev,
                                     uint32_t frequency);
static void     sam_spi_setmode(FAR struct spi_dev_s *dev,
                                enum spi_mode_e mode);
static void     sam_spi_setbits(FAR struct spi_dev_s *dev, int nbits);
static uint32_t sam_spi_send(FAR struct spi_dev_s *dev, uint32_t wd);
static void     sam_spi_exchange(FAR struct spi_dev_s *dev,
                                 FAR const void *txbuffer,
                                 FAR void *rxbuffer, size_t nwords);
static uint8_t  sam_spi_status(FAR struct spi_dev_s *dev, uint32_t devid);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct spi_ops_s g_spi3_ops =
{
  .lock         = sam_spi_lock,
  .select       = sam_spi_select,
  .setfrequency = sam_spi_setfrequency,
  .setmode      = sam_spi_setmode,
  .setbits      = sam_spi_setbits,
  .status       = sam_spi_status,
  .send         = sam_spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange     = sam_spi_exchange,
#endif
};

static struct sam_spidev_s g_spi3_dev =
{
  .dev       = { .ops = &g_spi3_ops },
  .base      = SPI3_BASE,
  .frequency = 0,
  .actual    = 0,
  .mode      = 0,
  .nbits     = 8,
  .lock      = NXMUTEX_INITIALIZER,
};

/****************************************************************************
 * Private Helpers
 ****************************************************************************/

static inline void spi_wait_syncbusy(uintptr_t base)
{
  while (getreg32(base + SAM_SPI_SYNCBUSY_OFFSET) != 0)
    {
    }
}

static inline void spi_putreg32(uintptr_t base, uint32_t offset,
                                uint32_t val)
{
  putreg32(val, base + offset);
}

static inline uint32_t spi_getreg32(uintptr_t base, uint32_t offset)
{
  return getreg32(base + offset);
}

static inline void spi_putreg8(uintptr_t base, uint32_t offset, uint8_t val)
{
  putreg8(val, base + offset);
}

static inline uint8_t spi_getreg8(uintptr_t base, uint32_t offset)
{
  return getreg8(base + offset);
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int sam_spi_lock(FAR struct spi_dev_s *dev, bool lock)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;

  if (lock)
    {
      return nxmutex_lock(&priv->lock);
    }

  return nxmutex_unlock(&priv->lock);
}

static void sam_spi_select(FAR struct spi_dev_s *dev, uint32_t devid,
                           bool selected)
{
  pic32czca90_spi3select(dev, devid, selected);
}

static uint32_t sam_spi_setfrequency(FAR struct spi_dev_s *dev,
                                     uint32_t frequency)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;

  if (priv->actual == frequency)
    {
      return priv->actual;
    }

  /* BAUD = (f_ref / (2 * f_baud)) - 1 */

  uint32_t baud;

  if (frequency >= SPI3_GCLK_FREQ / 2)
    {
      baud = 0;
    }
  else
    {
      baud = (SPI3_GCLK_FREQ / (2u * frequency)) - 1u;
      if (baud > 255)
        {
          baud = 255;
        }
    }

  /* Disable, change baud, re-enable */

  uint32_t ctrla = spi_getreg32(priv->base, SAM_SPI_CTRLA_OFFSET);
  spi_putreg32(priv->base, SAM_SPI_CTRLA_OFFSET, ctrla & ~SPI_CTRLA_ENABLE);
  spi_wait_syncbusy(priv->base);

  spi_putreg8(priv->base, SAM_SPI_BAUD_OFFSET, (uint8_t)baud);

  spi_putreg32(priv->base, SAM_SPI_CTRLA_OFFSET, ctrla);
  spi_wait_syncbusy(priv->base);

  priv->actual = SPI3_GCLK_FREQ / (2u * (baud + 1u));
  priv->frequency = frequency;
  return priv->actual;
}

static void sam_spi_setmode(FAR struct spi_dev_s *dev, enum spi_mode_e mode)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;

  if (priv->mode == (uint8_t)mode)
    {
      return;
    }

  uint32_t ctrla = spi_getreg32(priv->base, SAM_SPI_CTRLA_OFFSET);

  /* Disable to change mode */

  spi_putreg32(priv->base, SAM_SPI_CTRLA_OFFSET, ctrla & ~SPI_CTRLA_ENABLE);
  spi_wait_syncbusy(priv->base);

  ctrla &= ~(SPI_CTRLA_CPOL | SPI_CTRLA_CPHA);

  switch (mode)
    {
      case SPIDEV_MODE0: /* CPOL=0 CPHA=0 */
        break;

      case SPIDEV_MODE1: /* CPOL=0 CPHA=1 */
        ctrla |= SPI_CTRLA_CPHA;
        break;

      case SPIDEV_MODE2: /* CPOL=1 CPHA=0 */
        ctrla |= SPI_CTRLA_CPOL;
        break;

      case SPIDEV_MODE3: /* CPOL=1 CPHA=1 */
        ctrla |= SPI_CTRLA_CPOL | SPI_CTRLA_CPHA;
        break;

      default:
        return;
    }

  spi_putreg32(priv->base, SAM_SPI_CTRLA_OFFSET, ctrla | SPI_CTRLA_ENABLE);
  spi_wait_syncbusy(priv->base);

  priv->mode = (uint8_t)mode;
}

static void sam_spi_setbits(FAR struct spi_dev_s *dev, int nbits)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;
  DEBUGASSERT(nbits == 8 || nbits == 9);
  priv->nbits = (uint8_t)nbits;
}

static uint8_t sam_spi_status(FAR struct spi_dev_s *dev, uint32_t devid)
{
  return 0;
}

static uint32_t sam_spi_send(FAR struct spi_dev_s *dev, uint32_t wd)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;
  uintptr_t base = priv->base;

  /* Wait for DRE (Data Register Empty) */

  while (!(spi_getreg8(base, SAM_SPI_INTFLAG_OFFSET) & SPI_INT_DRE))
    {
    }

  /* Write TX data */

  spi_putreg32(base, SAM_SPI_DATA_OFFSET, wd & 0xFFu);

  /* Wait for RXC (Receive Complete) */

  while (!(spi_getreg8(base, SAM_SPI_INTFLAG_OFFSET) & SPI_INT_RXC))
    {
    }

  /* Read RX data */

  return spi_getreg32(base, SAM_SPI_DATA_OFFSET) & 0xFFu;
}

static void sam_spi_exchange(FAR struct spi_dev_s *dev,
                             FAR const void *txbuffer,
                             FAR void *rxbuffer, size_t nwords)
{
  FAR struct sam_spidev_s *priv = (FAR struct sam_spidev_s *)dev;
  uintptr_t base = priv->base;
  FAR const uint8_t *tx = (FAR const uint8_t *)txbuffer;
  FAR uint8_t *rx = (FAR uint8_t *)rxbuffer;

  for (size_t i = 0; i < nwords; i++)
    {
      uint8_t txd = tx ? tx[i] : 0xFFu;

      while (!(spi_getreg8(base, SAM_SPI_INTFLAG_OFFSET) & SPI_INT_DRE))
        {
        }

      spi_putreg32(base, SAM_SPI_DATA_OFFSET, txd);

      while (!(spi_getreg8(base, SAM_SPI_INTFLAG_OFFSET) & SPI_INT_RXC))
        {
        }

      uint8_t rxd = (uint8_t)(spi_getreg32(base, SAM_SPI_DATA_OFFSET) & 0xFFu);

      if (rx)
        {
          rx[i] = rxd;
        }
    }

  /* Wait for TXC to ensure last byte fully shifted out */

  while (!(spi_getreg8(base, SAM_SPI_INTFLAG_OFFSET) & SPI_INT_TXC))
    {
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR struct spi_dev_s *sam_spibus_initialize(int port)
{
  if (port != SPI3_SERCOM)
    {
      return NULL;
    }

  FAR struct sam_spidev_s *priv = &g_spi3_dev;
  uintptr_t base = priv->base;

  /* 1. Enable MCLK APB clock for SERCOM3 */

  sercom_enable(SPI3_SERCOM);

  /* 2. Route GCLK6 (100 MHz) to SERCOM3 core clock channel */

  sam_gclk_chan_enable(GCLK_CHAN_SERCOM3_CORE, SPI3_GCLK_GEN, false);

  /* 3. Configure GPIO pins: MOSI=PC12/PAD0, SCK=PC13/PAD1, MISO=PC15/PAD3
   *    CS=PC14 is handled as GPIO by the board layer */

  sam_portconfig(PORT_SERCOM3_PAD0);  /* PC12 MOSI */
  sam_portconfig(PORT_SERCOM3_PAD1);  /* PC13 SCK */
  sam_portconfig(PORT_SERCOM3_PAD3);  /* PC15 MISO */

  /* 4. Software reset */

  spi_putreg32(base, SAM_SPI_CTRLA_OFFSET, SPI_CTRLA_SWRST);
  spi_wait_syncbusy(base);

  /* 5. Configure CTRLB: 8-bit character, receiver enable */

  spi_putreg32(base, SAM_SPI_CTRLB_OFFSET,
               SPI_CTRLB_CHSIZE_8BIT | SPI_CTRLB_RXEN);
  spi_wait_syncbusy(base);

  /* 6. Set baud: default 1 MHz = GCLK/(2*(BAUD+1))
   *    100 MHz / (2*50) = 1 MHz → BAUD=49 */

  spi_putreg8(base, SAM_SPI_BAUD_OFFSET, 49);

  /* 7. Configure CTRLA: SPI master, DOPO=PAD0 (MOSI=PAD0, SCK=PAD1),
   *    DIPO=PAD3 (MISO=PAD3), CPOL=0, CPHA=0, MSB first, ENABLE */

  spi_putreg32(base, SAM_SPI_CTRLA_OFFSET,
               SPI_CTRLA_MODE_SPIM |
               SPI_CTRLA_DOPO_PAD0 |
               SPI_CTRLA_DIPO_PAD3 |
               SPI_CTRLA_ENABLE);
  spi_wait_syncbusy(base);

  priv->actual = SPI3_GCLK_FREQ / (2u * (49u + 1u));  /* 1 MHz */
  priv->mode = SPIDEV_MODE0;
  priv->nbits = 8;

  spiinfo("SERCOM3 SPI master initialized at %lu Hz\n",
          (unsigned long)priv->actual);

  return &priv->dev;
}

#endif /* CONFIG_PIC32CZCA90_SERCOM3_ISSPI */
