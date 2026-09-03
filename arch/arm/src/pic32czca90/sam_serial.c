/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_serial.c
 *
 * PIC32CZ CA90 interrupt-driven serial driver
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>
#ifdef CONFIG_SERIAL_TERMIOS
#  include <termios.h>
#endif

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/serial/serial.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_config.h"
#include "sam_usart.h"
#include "sam_lowputc.h"
#include "sam_serial.h"

#ifdef PIC32CZCA90_HAVE_USART

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* If we are not using the serial driver for the console, then we still must
 * provide some minimal implementation of up_putc.
 */

#ifdef USE_SERIALDRIVER

/* Which USART with be tty0/console and which tty1? tty2? ... tty7? */

#if defined(CONFIG_USART0_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart0port
#    define TTYS0_DEV           g_usart0port
#    define USART0_ASSIGNED      1
#elif defined(CONFIG_USART1_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart1port
#    define TTYS0_DEV           g_usart1port
#    define USART1_ASSIGNED      1
#elif defined(CONFIG_USART2_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart2port
#    define TTYS0_DEV           g_usart2port
#    define USART2_ASSIGNED     1
#elif defined(CONFIG_USART3_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart3port
#    define TTYS0_DEV           g_usart3port
#    define USART3_ASSIGNED     1
#elif defined(CONFIG_USART4_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart4port
#    define TTYS0_DEV           g_usart4port
#    define USART4_ASSIGNED     1
#elif defined(CONFIG_USART5_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart5port
#    define TTYS0_DEV           g_usart5port
#    define USART5_ASSIGNED     1
#elif defined(CONFIG_USART6_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart6port
#    define TTYS0_DEV           g_usart6port
#    define USART6_ASSIGNED     1
#elif defined(CONFIG_USART7_SERIAL_CONSOLE)
#    define CONSOLE_DEV         g_usart7port
#    define TTYS0_DEV           g_usart7port
#    define USART7_ASSIGNED     1
#else
#  undef CONSOLE_DEV
#  if defined(PIC32CZCA90_HAVE_USART0)
#    define TTYS0_DEV           g_usart0port
#    define USART0_ASSIGNED      1
#  elif defined(PIC32CZCA90_HAVE_USART1)
#    define TTYS0_DEV           g_usart1port
#    define USART1_ASSIGNED      1
#  elif defined(PIC32CZCA90_HAVE_USART2)
#    define TTYS0_DEV           g_usart2port
#    define USART2_ASSIGNED     1
#  elif defined(PIC32CZCA90_HAVE_USART3)
#    define TTYS0_DEV           g_usart3port
#    define USART3_ASSIGNED     1
#  elif defined(PIC32CZCA90_HAVE_USART4)
#    define TTYS0_DEV           g_usart4port
#    define USART4_ASSIGNED     1
#  elif defined(PIC32CZCA90_HAVE_USART5)
#    define TTYS0_DEV           g_usart5port
#    define USART5_ASSIGNED     1
#  elif defined(PIC32CZCA90_HAVE_USART6)
#    define TTYS0_DEV           g_usart6port
#    define USART6_ASSIGNED     1
#  elif defined(PIC32CZCA90_HAVE_USART7)
#    define TTYS0_DEV           g_usart7port
#    define USART7_ASSIGNED     1
#  endif
#endif

/* Pick ttys1 */

#if defined(PIC32CZCA90_HAVE_USART0) && !defined(USART0_ASSIGNED)
#  define TTYS1_DEV           g_usart0port
#  define USART0_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART1) && !defined(USART1_ASSIGNED)
#  define TTYS1_DEV           g_usart1port
#  define USART1_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART2) && !defined(USART2_ASSIGNED)
#  define TTYS1_DEV           g_usart2port
#  define USART2_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART3) && !defined(USART3_ASSIGNED)
#  define TTYS1_DEV           g_usart3port
#  define USART3_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART4) && !defined(USART4_ASSIGNED)
#  define TTYS1_DEV           g_usart4port
#  define USART4_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS1_DEV           g_usart5port
#  define USART5_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS1_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS1_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys2 */

#if defined(PIC32CZCA90_HAVE_USART1) && !defined(USART1_ASSIGNED)
#  define TTYS2_DEV           g_usart1port
#  define USART1_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART2) && !defined(USART2_ASSIGNED)
#  define TTYS2_DEV           g_usart2port
#  define USART2_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART3) && !defined(USART3_ASSIGNED)
#  define TTYS2_DEV           g_usart3port
#  define USART3_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART4) && !defined(USART4_ASSIGNED)
#  define TTYS2_DEV           g_usart4port
#  define USART4_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS2_DEV           g_usart5port
#  define USART5_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS2_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS2_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys3 */

#if defined(PIC32CZCA90_HAVE_USART2) && !defined(USART2_ASSIGNED)
#  define TTYS3_DEV           g_usart2port
#  define USART2_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART3) && !defined(USART3_ASSIGNED)
#  define TTYS3_DEV           g_usart3port
#  define USART3_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART4) && !defined(USART4_ASSIGNED)
#  define TTYS3_DEV           g_usart4port
#  define USART4_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS3_DEV           g_usart5port
#  define USART5_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS3_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS3_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys4 */

#if defined(PIC32CZCA90_HAVE_USART3) && !defined(USART3_ASSIGNED)
#  define TTYS4_DEV           g_usart3port
#  define USART3_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART4) && !defined(USART4_ASSIGNED)
#  define TTYS4_DEV           g_usart4port
#  define USART4_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS4_DEV           g_usart5port
#  define USART5_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS4_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS4_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys5 */

#if defined(PIC32CZCA90_HAVE_USART4) && !defined(USART4_ASSIGNED)
#  define TTYS5_DEV           g_usart4port
#  define USART4_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS5_DEV           g_usart5port
#  define USART5_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS5_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS5_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys6 */

#if defined(PIC32CZCA90_HAVE_USART5) && !defined(USART5_ASSIGNED)
#  define TTYS6_DEV           g_usart5port
#  define USART5_ASSIGNED      1
#elif defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS6_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS6_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/* Pick ttys7 */

#if defined(PIC32CZCA90_HAVE_USART6) && !defined(USART6_ASSIGNED)
#  define TTYS7_DEV           g_usart6port
#  define USART6_ASSIGNED     1
#elif defined(PIC32CZCA90_HAVE_USART7) && !defined(USART7_ASSIGNED)
#  define TTYS7_DEV           g_usart7port
#  define USART7_ASSIGNED     1
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_dev_s
{
  /* Common USART configuration */

  const struct sam_usart_config_s * const config;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline uint8_t
            sam_serialin8(struct sam_dev_s *priv, int offset);
static inline void
            sam_serialout8(struct sam_dev_s *priv, int offset,
              uint8_t regval);
static inline uint16_t
            sam_serialin16(struct sam_dev_s *priv, int offset);
static inline void
            sam_serialout16(struct sam_dev_s *priv, int offset,
              uint16_t regval);
static void sam_disableallints(struct sam_dev_s *priv);
static int  sam_interrupt(int irq, void *context, void *arg);

/* UART methods */

static int  sam_setup(struct uart_dev_s *dev);
static void sam_shutdown(struct uart_dev_s *dev);
static int  sam_attach(struct uart_dev_s *dev);
static void sam_detach(struct uart_dev_s *dev);
static int  sam_ioctl(struct file *filep, int cmd, unsigned long arg);
static int  sam_receive(struct uart_dev_s *dev, unsigned int *status);
static void sam_rxint(struct uart_dev_s *dev, bool enable);
static bool sam_rxavailable(struct uart_dev_s *dev);
static void sam_send(struct uart_dev_s *dev, int ch);
static void sam_txint(struct uart_dev_s *dev, bool enable);
static bool sam_txempty(struct uart_dev_s *dev);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct uart_ops_s g_uart_ops =
{
  .setup          = sam_setup,
  .shutdown       = sam_shutdown,
  .attach         = sam_attach,
  .detach         = sam_detach,
  .ioctl          = sam_ioctl,
  .receive        = sam_receive,
  .rxint          = sam_rxint,
  .rxavailable    = sam_rxavailable,
#ifdef CONFIG_SERIAL_IFLOWCONTROL
  .rxflowcontrol  = NULL,
#endif
  .send           = sam_send,
  .txint          = sam_txint,
  .txready        = sam_txempty,
  .txempty        = sam_txempty,
};

/* I/O buffers */

#ifdef PIC32CZCA90_HAVE_USART0
static char g_usart0rxbuffer[CONFIG_USART0_RXBUFSIZE];
static char g_usart0txbuffer[CONFIG_USART0_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART1
static char g_usart1rxbuffer[CONFIG_USART1_RXBUFSIZE];
static char g_usart1txbuffer[CONFIG_USART1_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART2
static char g_usart2rxbuffer[CONFIG_USART2_RXBUFSIZE];
static char g_usart2txbuffer[CONFIG_USART2_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART3
static char g_usart3rxbuffer[CONFIG_USART3_RXBUFSIZE];
static char g_usart3txbuffer[CONFIG_USART3_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART4
static char g_usart4rxbuffer[CONFIG_USART4_RXBUFSIZE];
static char g_usart4txbuffer[CONFIG_USART4_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART5
static char g_usart5rxbuffer[CONFIG_USART5_RXBUFSIZE];
static char g_usart5txbuffer[CONFIG_USART5_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART6
static char g_usart6rxbuffer[CONFIG_USART6_RXBUFSIZE];
static char g_usart6txbuffer[CONFIG_USART6_TXBUFSIZE];
#endif
#ifdef PIC32CZCA90_HAVE_USART7
static char g_usart7rxbuffer[CONFIG_USART7_RXBUFSIZE];
static char g_usart7txbuffer[CONFIG_USART7_TXBUFSIZE];
#endif

/* USART port state structs and uart_dev_t instances */

#ifdef PIC32CZCA90_HAVE_USART0
static struct sam_dev_s g_usart0priv =
{
  .config   = &g_usart0config,
};

static uart_dev_t g_usart0port =
{
  .recv     =
  {
    .size   = CONFIG_USART0_RXBUFSIZE,
    .buffer = g_usart0rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART0_TXBUFSIZE,
    .buffer = g_usart0txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart0priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART1
static struct sam_dev_s g_usart1priv =
{
  .config   = &g_usart1config,
};

static uart_dev_t g_usart1port =
{
  .recv     =
  {
    .size   = CONFIG_USART1_RXBUFSIZE,
    .buffer = g_usart1rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART1_TXBUFSIZE,
    .buffer = g_usart1txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart1priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART2
static struct sam_dev_s g_usart2priv =
{
  .config   = &g_usart2config,
};

static uart_dev_t g_usart2port =
{
  .recv     =
  {
    .size   = CONFIG_USART2_RXBUFSIZE,
    .buffer = g_usart2rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART2_TXBUFSIZE,
    .buffer = g_usart2txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart2priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART3
static struct sam_dev_s g_usart3priv =
{
  .config   = &g_usart3config,
};

static uart_dev_t g_usart3port =
{
  .recv     =
  {
    .size   = CONFIG_USART3_RXBUFSIZE,
    .buffer = g_usart3rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART3_TXBUFSIZE,
    .buffer = g_usart3txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart3priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART4
static struct sam_dev_s g_usart4priv =
{
  .config   = &g_usart4config,
};

static uart_dev_t g_usart4port =
{
  .recv     =
  {
    .size   = CONFIG_USART4_RXBUFSIZE,
    .buffer = g_usart4rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART4_TXBUFSIZE,
    .buffer = g_usart4txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart4priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART5
static struct sam_dev_s g_usart5priv =
{
  .config   = &g_usart5config,
};

static uart_dev_t g_usart5port =
{
  .recv     =
  {
    .size   = CONFIG_USART5_RXBUFSIZE,
    .buffer = g_usart5rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART5_TXBUFSIZE,
    .buffer = g_usart5txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart5priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART6
static struct sam_dev_s g_usart6priv =
{
  .config   = &g_usart6config,
};

static uart_dev_t g_usart6port =
{
  .recv     =
  {
    .size   = CONFIG_USART6_RXBUFSIZE,
    .buffer = g_usart6rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART6_TXBUFSIZE,
    .buffer = g_usart6txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart6priv,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART7
static struct sam_dev_s g_usart7priv =
{
  .config   = &g_usart7config,
};

static uart_dev_t g_usart7port =
{
  .recv     =
  {
    .size   = CONFIG_USART7_RXBUFSIZE,
    .buffer = g_usart7rxbuffer,
  },
  .xmit     =
  {
    .size   = CONFIG_USART7_TXBUFSIZE,
    .buffer = g_usart7txbuffer,
  },
  .ops      = &g_uart_ops,
  .priv     = &g_usart7priv,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline uint8_t sam_serialin8(struct sam_dev_s *priv, int offset)
{
  return getreg8(priv->config->base + offset);
}

static inline void sam_serialout8(struct sam_dev_s *priv, int offset,
                                  uint8_t regval)
{
  putreg8(regval, priv->config->base + offset);
}

static inline uint16_t sam_serialin16(struct sam_dev_s *priv, int offset)
{
  return getreg16(priv->config->base + offset);
}

static inline void sam_serialout16(struct sam_dev_s *priv, int offset,
                                   uint16_t regval)
{
  putreg16(regval, priv->config->base + offset);
}

static void sam_disableallints(struct sam_dev_s *priv)
{
  sam_serialout8(priv, SAM_USART_INTENCLR_OFFSET, USART_INT_ALL);
}

static int sam_interrupt(int irq, void *context, void *arg)
{
  struct uart_dev_s *dev = (struct uart_dev_s *)arg;
  struct sam_dev_s *priv;
  uint8_t pending;
  uint8_t intflag;
  uint8_t inten;

  DEBUGASSERT(dev != NULL && dev->priv != NULL);
  priv = (struct sam_dev_s *)dev->priv;

  intflag = sam_serialin8(priv, SAM_USART_INTFLAG_OFFSET);
  inten   = sam_serialin8(priv, SAM_USART_INTENCLR_OFFSET);
  pending = intflag & inten;

  if ((pending & USART_INT_RXC) != 0)
    {
      uart_recvchars(dev);
    }

  if ((pending & USART_INT_DRE) != 0)
    {
      uart_xmitchars(dev);
    }

  return OK;
}

static int sam_setup(struct uart_dev_s *dev)
{
  int ret = 0;
#ifndef CONFIG_SUPPRESS_UART_CONFIG
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;

  if (!dev->isconsole)
    {
      ret = sam_usart_initialize(priv->config);
    }
#endif

  return ret;
}

static void sam_shutdown(struct uart_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;

  if (!dev->isconsole)
    {
      sam_usart_reset(priv->config);
    }
}

static int sam_attach(struct uart_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  const struct sam_usart_config_s * const config = priv->config;
  int ret;

  ret = irq_attach(config->txirq, sam_interrupt, dev);
  if (ret == OK)
    {
      ret = irq_attach(config->rxirq, sam_interrupt, dev);
      if (ret == OK)
        {
          up_enable_irq(config->txirq);
          up_enable_irq(config->rxirq);
        }
    }

  return ret;
}

static void sam_detach(struct uart_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  const struct sam_usart_config_s * const config = priv->config;

  sam_disableallints(priv);
  up_disable_irq(config->txirq);
  up_disable_irq(config->rxirq);

  irq_detach(config->txirq);
  irq_detach(config->rxirq);
}

static int sam_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  struct inode      *inode = filep->f_inode;
  struct uart_dev_s *dev   = inode->i_private;
  struct sam_dev_s  *priv  = (struct sam_dev_s *)dev->priv;
  int                ret   = OK;

  switch (cmd)
    {
#ifdef CONFIG_SERIAL_TIOCSERGSTRUCT
    case TIOCSERGSTRUCT:
      {
         struct sam_dev_s *user = (struct sam_dev_s *)arg;
         if (!user)
           {
             ret = -EINVAL;
           }
         else
           {
             memcpy(user, dev, sizeof(struct sam_dev_s));
           }
       }
       break;
#endif

#ifdef CONFIG_SERIAL_TERMIOS
    case TCGETS:
      {
        struct termios *termiosp = (struct termios *)arg;
        uint16_t baud_reg;
        uint32_t baud;

        if (!termiosp)
          {
            ret = -EINVAL;
            break;
          }

        /* Reverse-calculate baud from current BAUD register value */

        baud_reg = getreg16(priv->config->base + SAM_USART_BAUD_OFFSET);
        baud = (uint32_t)(((uint64_t)(65536u - baud_reg) *
                           priv->config->frequency) >> 20);

        cfsetispeed(termiosp, baud);
        cfsetospeed(termiosp, baud);

        termiosp->c_cflag =
          ((priv->config->parity != 0) ? PARENB : 0) |
          ((priv->config->parity == 1) ? PARODD : 0) |
          ((priv->config->stopbits2)   ? CSTOPB : 0) |
          CS8;
      }
      break;

    case TCSETS:
      {
        struct termios *termiosp = (struct termios *)arg;
        uint32_t baud;
        uint64_t tmp;
        uint16_t baud_reg;
        uint32_t ctrla;

        if (!termiosp)
          {
            ret = -EINVAL;
            break;
          }

        baud = cfgetispeed(termiosp);
        if (baud == 0)
          {
            ret = -EINVAL;
            break;
          }

        /* BAUD = 65536 - (baud << 20) / frequency  (same formula as sam_usart_configure) */

        tmp = (uint64_t)baud << 20;
        tmp = (tmp + (priv->config->frequency >> 1)) / priv->config->frequency;
        if (tmp < 1 || tmp > (uint64_t)UINT16_MAX)
          {
            ret = -ERANGE;
            break;
          }

        baud_reg = (uint16_t)(65536u - (uint16_t)tmp);

        /* BAUD register is enable-protected: disable → write → re-enable */

        ctrla = getreg32(priv->config->base + SAM_USART_CTRLA_OFFSET);
        putreg32(ctrla & ~USART_CTRLA_ENABLE,
                 priv->config->base + SAM_USART_CTRLA_OFFSET);
        while (getreg32(priv->config->base + SAM_USART_SYNCBUSY_OFFSET) &
               USART_SYNCBUSY_ENABLE);

        putreg16(baud_reg, priv->config->base + SAM_USART_BAUD_OFFSET);

        putreg32(ctrla, priv->config->base + SAM_USART_CTRLA_OFFSET);
        while (getreg32(priv->config->base + SAM_USART_SYNCBUSY_OFFSET) &
               USART_SYNCBUSY_ENABLE);

        ret = OK;
      }
      break;
#endif /* CONFIG_SERIAL_TERMIOS */

    default:
      ret = -ENOTTY;
      break;
    }

  return ret;
}

static int sam_receive(struct uart_dev_s *dev, unsigned int *status)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;

  *status = (uint32_t)sam_serialin16(priv, SAM_USART_STATUS_OFFSET);

  return (int)sam_serialin16(priv, SAM_USART_DATA_OFFSET);
}

static void sam_rxint(struct uart_dev_s *dev, bool enable)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;

  if (enable)
    {
#ifndef CONFIG_SUPPRESS_SERIAL_INTS
      sam_serialout8(priv, SAM_USART_INTENSET_OFFSET, USART_INT_RXC);
#endif
    }
  else
    {
      sam_serialout8(priv, SAM_USART_INTENCLR_OFFSET, USART_INT_RXC);
    }
}

static bool sam_rxavailable(struct uart_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  return ((sam_serialin8(priv, SAM_USART_INTFLAG_OFFSET) & USART_INT_RXC)
          != 0);
}

static void sam_send(struct uart_dev_s *dev, int ch)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  sam_serialout16(priv, SAM_USART_DATA_OFFSET, (uint16_t)ch);
}

static void sam_txint(struct uart_dev_s *dev, bool enable)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  irqstate_t flags;

  flags = enter_critical_section();
  if (enable)
    {
#ifndef CONFIG_SUPPRESS_SERIAL_INTS
      sam_serialout8(priv, SAM_USART_INTENSET_OFFSET, USART_INT_DRE);

      /* Fake a TX interrupt here by just calling uart_xmitchars() with
       * interrupts disabled (note this may recurse).
       */

      uart_xmitchars(dev);
#endif
    }
  else
    {
      sam_serialout8(priv, SAM_USART_INTENCLR_OFFSET, USART_INT_DRE);
    }

  leave_critical_section(flags);
}

static bool sam_txempty(struct uart_dev_s *dev)
{
  struct sam_dev_s *priv = (struct sam_dev_s *)dev->priv;
  return ((sam_serialin8(priv, SAM_USART_INTFLAG_OFFSET) & USART_INT_DRE)
          != 0);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef USE_EARLYSERIALINIT
void arm_earlyserialinit(void)
{
  sam_disableallints(TTYS0_DEV.priv);
#ifdef TTYS1_DEV
  sam_disableallints(TTYS1_DEV.priv);
#endif
#ifdef TTYS2_DEV
  sam_disableallints(TTYS2_DEV.priv);
#endif
#ifdef TTYS3_DEV
  sam_disableallints(TTYS3_DEV.priv);
#endif
#ifdef TTYS4_DEV
  sam_disableallints(TTYS4_DEV.priv);
#endif
#ifdef TTYS5_DEV
  sam_disableallints(TTYS5_DEV.priv);
#endif
#ifdef TTYS6_DEV
  sam_disableallints(TTYS6_DEV.priv);
#endif
#ifdef TTYS7_DEV
  sam_disableallints(TTYS7_DEV.priv);
#endif

#ifdef HAVE_SERIAL_CONSOLE
  CONSOLE_DEV.isconsole = true;
#endif
}
#endif

void arm_serialinit(void)
{
#ifdef HAVE_SERIAL_CONSOLE
  uart_register("/dev/console", &CONSOLE_DEV);
#endif

  uart_register("/dev/ttyS0", &TTYS0_DEV);
#ifdef TTYS1_DEV
  uart_register("/dev/ttyS1", &TTYS1_DEV);
#endif
#ifdef TTYS2_DEV
  uart_register("/dev/ttyS2", &TTYS2_DEV);
#endif
#ifdef TTYS3_DEV
  uart_register("/dev/ttyS3", &TTYS3_DEV);
#endif
#ifdef TTYS4_DEV
  uart_register("/dev/ttyS4", &TTYS4_DEV);
#endif
#ifdef TTYS5_DEV
  uart_register("/dev/ttyS5", &TTYS5_DEV);
#endif
#ifdef TTYS6_DEV
  uart_register("/dev/ttyS6", &TTYS6_DEV);
#endif
#ifdef TTYS7_DEV
  uart_register("/dev/ttyS7", &TTYS7_DEV);
#endif
}

int up_putc(int ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  irqstate_t flags;

  flags = enter_critical_section();

  if (ch == '\n')
    {
      sam_lowputc('\r');
    }

  sam_lowputc(ch);
  leave_critical_section(flags);
#endif
  return ch;
}

#else /* USE_SERIALDRIVER */

int up_putc(int ch)
{
#ifdef HAVE_SERIAL_CONSOLE
  if (ch == '\n')
    {
      sam_lowputc('\r');
    }

  sam_lowputc(ch);
#endif
  return ch;
}

#endif /* USE_SERIALDRIVER */
#endif /* PIC32CZCA90_HAVE_USART */
