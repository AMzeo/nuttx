/****************************************************************************
 * arch/arm/src/pic32czca90/sam_usb.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * PIC32CZ CA90 USBHS Device-Mode Driver
 *
 * MUSB/Mentor-style controller at 0x4F010000 (USBHS0).
 * 8 endpoints (EP0 + EP1-7), 8 DMA channels, 9 KB FIFO.
 * High-Speed (480 Mbps) with Full-Speed fallback.
 * Single IRQ vector for both USB core and DMA.
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/kmalloc.h>
#include <nuttx/irq.h>
#include <nuttx/usb/usb.h>
#include <nuttx/usb/usbdev.h>
#include <nuttx/usb/usbdev_trace.h>

#include "arm_internal.h"
#include "hardware/sam_usbhs.h"
#include "hardware/sam_supc.h"
#include "hardware/sam_oscctrl.h"

#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SAM_USB_BASE            SAM_USBHS0_BASE
#define SAM_IRQ_USB             SAM_IRQ_USBHS0

#define SAM_USB_NENDPOINTS      8
#define SAM_USB_NDMA            8

#define EP0_MAXPACKET           64

/* High-speed max packet for bulk endpoints */

#define HS_BULK_MAXPACKET       512
#define FS_BULK_MAXPACKET       64

/* FIFO address allocation (in 8-byte units) */

/* EP0 FIFO is hardwired at address 0, 64 bytes. EP1-7 allocated dynamically. */

#define FIFO_EP0_ADDR           0
#define FIFO_EP0_SIZE           USBHS_FIFOSZ_64

/* FIFO address allocator: next free unit (8-byte granularity).
 * EP0 uses units 0-7 (64 bytes). EP1+ starts at unit 8.
 */

#define FIFO_START_ADDR         8    /* First free unit after EP0 */

/* EP0 state machine */

enum sam_ep0state_e
{
  EP0_STATE_IDLE = 0,
  EP0_STATE_SETUP,
  EP0_STATE_TX,
  EP0_STATE_TX_LAST,
  EP0_STATE_RX,
  EP0_STATE_STATUS_IN,
  EP0_STATE_STATUS_OUT,
  EP0_STATE_STALL,
};

/* Device state */

enum sam_devstate_e
{
  USB_DEVSTATE_SUSPENDED = 0,
  USB_DEVSTATE_POWERED,
  USB_DEVSTATE_DEFAULT,
  USB_DEVSTATE_ADDRESSED,
  USB_DEVSTATE_CONFIGURED,
};

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_req_s
{
  struct usbdev_req_s  req;
  struct sam_req_s    *flink;
};

struct sam_ep_s
{
  struct usbdev_ep_s   ep;
  struct sam_req_s    *head;
  struct sam_req_s    *tail;
  uint8_t             epphy;       /* Physical EP number */
  uint8_t             eptype;      /* USB_EP_ATTR_XFER_* */
  bool                txin;        /* true=IN (TX), false=OUT (RX) */
  bool                stalled;
  uint16_t            maxpacket;
  uint8_t             dmachannel;  /* Assigned DMA channel (0=none) */
};

struct sam_usbdev_s
{
  struct usbdev_s              usbdev;
  struct usbdevclass_driver_s *driver;
  struct sam_ep_s              eplist[SAM_USB_NENDPOINTS * 2];
  struct sam_ep_s             *ep0in;
  struct sam_ep_s             *ep0out;

  uint8_t                      devstate;
  uint8_t                      ep0state;
  uint8_t                      devaddr;
  uint8_t                      newaddr;   /* Deferred SET_ADDRESS */
  bool                         selfpowered;
  bool                         connected;

  /* EP0 data buffer for control transfers */

  uint8_t                      ep0buf[EP0_MAXPACKET];

  /* DMA channel allocation */

  uint8_t                      dma_inuse;  /* Bitmask of in-use DMA channels */
  uint16_t                     fifo_next;  /* Next free FIFO addr (8-byte units) */

  /* EP0 H2D data reception (SET_LINE_CODING etc.) */

  uint8_t                      ep0rxlen;   /* Expected OUT data length */
  uint8_t                      ep0rxoff;   /* Bytes received so far */
  struct usb_ctrlreq_s         ep0setup;   /* Saved SETUP for deferred CLASS_SETUP */

  /* Debug counters */

  volatile uint32_t            dbg_isr;
  volatile uint32_t            dbg_reset;
  volatile uint32_t            dbg_ep0;
  volatile uint32_t            dbg_setup;
  volatile uint32_t            dbg_suspend;
  volatile uint32_t            dbg_ep0tx_noreq;
  volatile uint32_t            dbg_epn_rx;
  volatile uint32_t            dbg_epn_rx_noreq;
  volatile uint32_t            dbg_epn_rx_bytes;
  volatile uint32_t            dbg_epn_tx;
  volatile uint32_t            dbg_epn_tx_done;

  /* EP0 trace ring buffer — captures ISR events without timing impact */

#define EP0_TRACE_SIZE 32
  struct
  {
    uint8_t  evt;      /* event code */
    uint8_t  csr0l;    /* CSR0L at event */
    uint8_t  state;    /* ep0state at event */
    uint8_t  data;     /* context-specific byte */
  }                            ep0_trace[EP0_TRACE_SIZE];
  volatile uint8_t             ep0_trace_idx;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Endpoint operations */

static int  sam_ep_configure(struct usbdev_ep_s *ep,
                             const struct usb_epdesc_s *desc, bool last);
static int  sam_ep_disable(struct usbdev_ep_s *ep);
static struct usbdev_req_s *sam_ep_allocreq(struct usbdev_ep_s *ep);
static void sam_ep_freereq(struct usbdev_ep_s *ep,
                           struct usbdev_req_s *req);
#ifdef CONFIG_USBDEV_DMA
static void *sam_ep_allocbuffer(struct usbdev_ep_s *ep, uint16_t nbytes);
static void  sam_ep_freebuffer(struct usbdev_ep_s *ep, void *buf);
#endif
static int  sam_ep_submit(struct usbdev_ep_s *ep,
                          struct usbdev_req_s *req);
static int  sam_ep_cancel(struct usbdev_ep_s *ep,
                          struct usbdev_req_s *req);
static int  sam_ep_stall(struct usbdev_ep_s *ep, bool resume);

/* Device operations */

static struct usbdev_ep_s *sam_allocep(struct usbdev_s *dev, uint8_t epphy,
                                       bool in, uint8_t eptype);
static void sam_freeep(struct usbdev_s *dev, struct usbdev_ep_s *ep);
static int  sam_getframe(struct usbdev_s *dev);
static int  sam_wakeup(struct usbdev_s *dev);
static int  sam_selfpowered(struct usbdev_s *dev, bool selfpowered);
static int  sam_pullup(struct usbdev_s *dev, bool enable);

/* Interrupt handling */

static int  sam_usbhs_interrupt(int irq, void *context, void *arg);
static void sam_ep0_interrupt(struct sam_usbdev_s *priv);
static void sam_epn_tx_interrupt(struct sam_usbdev_s *priv, uint8_t epno);
static void sam_epn_rx_interrupt(struct sam_usbdev_s *priv, uint8_t epno);
static void sam_dma_interrupt(struct sam_usbdev_s *priv, uint8_t ch);
static void sam_usb_reset(struct sam_usbdev_s *priv);
static void sam_usb_suspend(struct sam_usbdev_s *priv);
static void sam_usb_resume(struct sam_usbdev_s *priv);

/* Request queue helpers */

static void sam_req_enqueue(struct sam_ep_s *privep, struct sam_req_s *req);
static struct sam_req_s *sam_req_dequeue(struct sam_ep_s *privep);
static void sam_req_complete(struct sam_ep_s *privep, int16_t result);

/* Transfer helpers */

static void sam_ep0_tx(struct sam_usbdev_s *priv);
static void sam_epn_tx_start(struct sam_ep_s *privep);
static void sam_epn_rx_start(struct sam_ep_s *privep);
static void sam_fifo_write(uint8_t epno, const uint8_t *buf, uint16_t len);
static void sam_fifo_read(uint8_t epno, uint8_t *buf, uint16_t len);

/* Hardware init */

static void sam_hw_setup(struct sam_usbdev_s *priv);
static void sam_hw_shutdown(struct sam_usbdev_s *priv);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct usbdev_epops_s g_epops =
{
  .configure  = sam_ep_configure,
  .disable    = sam_ep_disable,
  .allocreq   = sam_ep_allocreq,
  .freereq    = sam_ep_freereq,
#ifdef CONFIG_USBDEV_DMA
  .allocbuffer = sam_ep_allocbuffer,
  .freebuffer  = sam_ep_freebuffer,
#endif
  .submit     = sam_ep_submit,
  .cancel     = sam_ep_cancel,
  .stall      = sam_ep_stall,
};

static const struct usbdev_ops_s g_devops =
{
  .allocep      = sam_allocep,
  .freeep       = sam_freeep,
  .getframe     = sam_getframe,
  .wakeup       = sam_wakeup,
  .selfpowered  = sam_selfpowered,
  .pullup       = sam_pullup,
};

static struct sam_usbdev_s g_usbdev;

/* EP0 trace event codes */

#define T_ENTER     0x01  /* EP0 ISR entered, data=csr0l */
#define T_STALL     0x02  /* SENTSTALL handled */
#define T_SETUPEND  0x03  /* SETUPEND handled */
#define T_SETUP     0x04  /* SETUP parsed, data=bRequest */
#define T_TX_START  0x05  /* sam_ep0_tx called, data=len */
#define T_TX_DONE   0x06  /* TX_LAST complete */
#define T_TX_CONT   0x07  /* TX multi-pkt continue, data=xfrd */
#define T_ADDR      0x08  /* SET_ADDRESS done, data=addr */
#define T_CLASS     0x09  /* CLASS_SETUP called, data=bReq */
#define T_SUBMIT    0x0A  /* EP_SUBMIT ep0in, data=len */
#define T_NOREQ     0x0B  /* ep0_tx called with no request */
#define T_RESET     0x0C  /* USB reset */
#define T_H2D       0x0D  /* H2D with data, data=bReq */
#define T_H2D_RX    0x0E  /* H2D data received, data=count */
#define T_H2D_NORQ  0x0F  /* H2D data but no request queued */

static inline void ep0_trace(struct sam_usbdev_s *priv, uint8_t evt,
                             uint8_t csr, uint8_t state, uint8_t data)
{
  uint8_t idx = priv->ep0_trace_idx % EP0_TRACE_SIZE;
  priv->ep0_trace[idx].evt = evt;
  priv->ep0_trace[idx].csr0l = csr;
  priv->ep0_trace[idx].state = state;
  priv->ep0_trace[idx].data = data;
  priv->ep0_trace_idx++;
}

/****************************************************************************
 * Private Inline Helpers
 ****************************************************************************/

static inline uint8_t sam_getreg8(uintptr_t addr)
{
  return *(volatile uint8_t *)addr;
}

static inline void sam_putreg8(uint8_t val, uintptr_t addr)
{
  *(volatile uint8_t *)addr = val;
}

static inline uint16_t sam_getreg16(uintptr_t addr)
{
  return *(volatile uint16_t *)addr;
}

static inline void sam_putreg16(uint16_t val, uintptr_t addr)
{
  *(volatile uint16_t *)addr = val;
}

/****************************************************************************
 * FIFO Access
 ****************************************************************************/

static void sam_fifo_write(uint8_t epno, const uint8_t *buf, uint16_t len)
{
  volatile uint32_t *fifo32 = (volatile uint32_t *)SAM_USBHS0_FIFO(epno);
  volatile uint8_t  *fifo8  = (volatile uint8_t *)SAM_USBHS0_FIFO(epno);

  while (len >= 4)
    {
      uint32_t val;
      memcpy(&val, buf, 4);
      *fifo32 = val;
      buf += 4;
      len -= 4;
    }

  while (len--)
    {
      *fifo8 = *buf++;
    }
}

static void sam_fifo_read(uint8_t epno, uint8_t *buf, uint16_t len)
{
  volatile uint32_t *fifo32 = (volatile uint32_t *)SAM_USBHS0_FIFO(epno);
  volatile uint8_t  *fifo8  = (volatile uint8_t *)SAM_USBHS0_FIFO(epno);

  while (len >= 4)
    {
      uint32_t val = *fifo32;
      memcpy(buf, &val, 4);
      buf += 4;
      len -= 4;
    }

  while (len--)
    {
      *buf++ = *fifo8;
    }
}

/****************************************************************************
 * Request Queue Management
 ****************************************************************************/

static void sam_req_enqueue(struct sam_ep_s *privep, struct sam_req_s *req)
{
  req->flink = NULL;
  if (privep->tail)
    {
      privep->tail->flink = req;
    }
  else
    {
      privep->head = req;
    }

  privep->tail = req;
}

static struct sam_req_s *sam_req_dequeue(struct sam_ep_s *privep)
{
  struct sam_req_s *req = privep->head;
  if (req)
    {
      privep->head = req->flink;
      if (!privep->head)
        {
          privep->tail = NULL;
        }

      req->flink = NULL;
    }

  return req;
}

static void sam_req_complete(struct sam_ep_s *privep, int16_t result)
{
  struct sam_req_s *privreq = sam_req_dequeue(privep);
  if (privreq)
    {
      privreq->req.result = result;
      if (privreq->req.callback)
        {
          privreq->req.callback(&privep->ep, &privreq->req);
        }
    }
}

/****************************************************************************
 * DMA Channel Allocation
 ****************************************************************************/

static uint8_t sam_dma_alloc(struct sam_usbdev_s *priv)
{
  for (int ch = 1; ch <= SAM_USB_NDMA; ch++)
    {
      if (!(priv->dma_inuse & (1u << ch)))
        {
          priv->dma_inuse |= (1u << ch);
          return ch;
        }
    }

  return 0;
}

static void sam_dma_free(struct sam_usbdev_s *priv, uint8_t ch)
{
  if (ch >= 1 && ch <= SAM_USB_NDMA)
    {
      priv->dma_inuse &= ~(1u << ch);
    }
}

/****************************************************************************
 * EP0 Transfer Logic
 ****************************************************************************/

static void sam_ep0_tx(struct sam_usbdev_s *priv)
{
  struct sam_ep_s *ep0in = priv->ep0in;
  struct sam_req_s *privreq = ep0in->head;
  irqstate_t flags;
  uint16_t nbytes;

  if (!privreq)
    {
      priv->dbg_ep0tx_noreq++;
      ep0_trace(priv, T_NOREQ, 0, priv->ep0state, 0);
      return;
    }

  ep0_trace(priv, T_TX_START, 0, priv->ep0state,
            (uint8_t)privreq->req.len);

  nbytes = privreq->req.len - privreq->req.xfrd;
  if (nbytes > EP0_MAXPACKET)
    {
      nbytes = EP0_MAXPACKET;
    }

  flags = enter_critical_section();
  sam_putreg8(0, SAM_USBHS0_INDEX);

  sam_fifo_write(0, (uint8_t *)privreq->req.buf + privreq->req.xfrd, nbytes);
  privreq->req.xfrd += nbytes;

  if (privreq->req.xfrd >= privreq->req.len || nbytes < EP0_MAXPACKET)
    {
      /* Last packet — set TXPKTRDY + DATAEND.
       * Harmony: 0x0A for last data packet in D2H transfer.
       */

      sam_putreg8(USBHS_CSR0L_TXPKTRDY | USBHS_CSR0L_DATAEND,
                  SAM_USBHS0_CSR0L);
      priv->ep0state = EP0_STATE_TX_LAST;
    }
  else
    {
      /* Multi-packet: TXPKTRDY only (Harmony: 0x02) */

      sam_putreg8(USBHS_CSR0L_TXPKTRDY, SAM_USBHS0_CSR0L);
      priv->ep0state = EP0_STATE_TX;
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * EPn Transfer Logic (Bulk IN/OUT with DMA fallback to PIO)
 ****************************************************************************/

static void sam_epn_tx_start(struct sam_ep_s *privep)
{
  struct sam_usbdev_s *priv = &g_usbdev;
  struct sam_req_s *privreq = privep->head;
  irqstate_t flags;
  uint16_t nbytes;
  uint8_t epno = privep->epphy;

  if (!privreq)
    {
      return;
    }

  priv->dbg_epn_tx++;

  nbytes = privreq->req.len - privreq->req.xfrd;
  if (nbytes > privep->maxpacket)
    {
      nbytes = privep->maxpacket;
    }

  flags = enter_critical_section();
  sam_putreg8(epno, SAM_USBHS0_INDEX);

  /* Check if DMA can be used (buffer aligned, length > 0) */

  uint8_t *buf = (uint8_t *)privreq->req.buf + privreq->req.xfrd;
  uint8_t ch = privep->dmachannel;

  if (0 && ch && nbytes > 4 && ((uintptr_t)buf & 3) == 0)
    {
      /* DMA TX: memory → endpoint FIFO.
       * DMA completes instantly at 300 MHz (COUNT=0 observed) but never
       * fires the interrupt (DMAINTR stays 0). Workaround: start DMA,
       * busy-wait for COUNT=0, then set TXPKTRDY. At 300 MHz CPU vs
       * max 64-byte packet, this is ~20 cycles — negligible.
       */

      uint8_t txcsrl = sam_getreg8(SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
      if (txcsrl & USBHS_TXCSRL_TXPKTRDY)
        {
          priv->dbg_epn_tx--;
          leave_critical_section(flags);
          return;
        }

#ifdef CONFIG_ARMV7M_DCACHE
      up_clean_dcache((uintptr_t)buf, (uintptr_t)buf + nbytes);
#endif

      putreg32((uint32_t)buf, SAM_USBHS0_DMAADDR(ch));
      putreg32(nbytes, SAM_USBHS0_DMACOUNT(ch));
      putreg32(USBHS_DMACNTL_DMAEN | USBHS_DMACNTL_DMADIR_TX |
               USBHS_DMACNTL_DMABRSTM(3) |
               USBHS_DMACNTL_DMAEP(epno),
               SAM_USBHS0_DMACNTL(ch));

      /* Wait for DMA to complete (COUNT reaches 0).
       * At 300 MHz with burst mode 3, a 64-byte transfer takes ~20 cycles.
       * Add timeout to avoid infinite hang if DMA stalls.
       */

      int timeout = 10000;
      while (getreg32(SAM_USBHS0_DMACOUNT(ch)) != 0 && --timeout > 0)
        {
        }

      if (timeout <= 0)
        {
          /* DMA hung — abort and fall back to PIO for this packet */

          putreg32(0, SAM_USBHS0_DMACNTL(ch));
          sam_fifo_write(epno, buf, nbytes);
        }

      /* Data is in FIFO. Set TXPKTRDY to send it. */

      sam_putreg8(USBHS_TXCSRL_TXPKTRDY,
                  SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
      privreq->req.xfrd += nbytes;
    }
  else
    {
      /* PIO: verify TXPKTRDY is clear before loading new data.
       * If previous packet hasn't been sent yet, bail out —
       * the TX completion interrupt will call us again.
       */

      uint8_t txcsrl = sam_getreg8(SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
      if (txcsrl & USBHS_TXCSRL_TXPKTRDY)
        {
          /* Previous packet still pending — don't overwrite FIFO */
          priv->dbg_epn_tx--;  /* Undo the count — we didn't actually send */
          leave_critical_section(flags);
          return;
        }

      sam_fifo_write(epno, buf, nbytes);
      privreq->req.xfrd += nbytes;
      sam_putreg8(USBHS_TXCSRL_TXPKTRDY,
                  SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
    }

  leave_critical_section(flags);
}

static void sam_epn_rx_start(struct sam_ep_s *privep)
{
  /* RX is host-initiated; we just make sure the endpoint is ready
   * to receive. The actual data read happens in the RX interrupt.
   */

  (void)privep;
}

/****************************************************************************
 * Interrupt Handlers
 ****************************************************************************/

static void sam_usb_reset(struct sam_usbdev_s *priv)
{
  irqstate_t flags;

  ep0_trace(priv, T_RESET, 0, priv->ep0state, priv->devaddr);

  flags = enter_critical_section();

  /* Reset device address */

  sam_putreg8(0, SAM_USBHS0_FADDR);
  priv->devaddr = 0;
  priv->newaddr = 0;

  /* Flush EP0 FIFO and clear data toggle.
   * After bus reset, host expects DATA0 — MUSB retains the old toggle
   * unless explicitly cleared. FlushFIFO discards any pending TX data.
   * (CSR0H bit 0 = FLUSHFIFO for EP0)
   */

  sam_putreg8(0, SAM_USBHS0_INDEX);
  sam_putreg8(USBHS_CSR0H_FLUSHFIFO, SAM_USBHS0_CSR0H);

  /* Clear any pending CSR0L status bits (SETUPEND, RXPKTRDY residue) */

  uint8_t csr0l = sam_getreg8(SAM_USBHS0_CSR0L);
  if (csr0l & USBHS_CSR0L_SETUPEND)
    {
      sam_putreg8(USBHS_CSR0L_SVCSETUPEND, SAM_USBHS0_CSR0L);
    }

  if (csr0l & USBHS_CSR0L_RXPKTRDY)
    {
      sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);
    }

  /* Disable all endpoint interrupts except EP0 */

  sam_putreg16(1u, SAM_USBHS0_INTRTXE);
  sam_putreg16(0, SAM_USBHS0_INTRRXE);

  /* Reset FIFO allocator — endpoints will be reconfigured after reset */

  priv->fifo_next = FIFO_START_ADDR;

  /* Check negotiated speed */

  uint8_t power = sam_getreg8(SAM_USBHS0_POWER);
  if (power & USBHS_POWER_HSMODE)
    {
      priv->usbdev.speed = USB_SPEED_HIGH;
    }
  else
    {
      priv->usbdev.speed = USB_SPEED_FULL;
    }

  priv->devstate = USB_DEVSTATE_DEFAULT;
  priv->ep0state = EP0_STATE_IDLE;

  leave_critical_section(flags);

  /* Notify class driver that connection was reset. This causes CDC/ACM
   * to de-configure its endpoints. When the host re-sends
   * SET_CONFIGURATION, the class driver will re-configure them.
   */

  if (priv->driver)
    {
      CLASS_DISCONNECT(priv->driver, &priv->usbdev);
    }
}

static void sam_usb_suspend(struct sam_usbdev_s *priv)
{
  priv->devstate = USB_DEVSTATE_SUSPENDED;
  if (priv->driver)
    {
      CLASS_SUSPEND(priv->driver, &priv->usbdev);
    }
}

static void sam_usb_resume(struct sam_usbdev_s *priv)
{
  priv->devstate = USB_DEVSTATE_CONFIGURED;
  if (priv->driver)
    {
      CLASS_RESUME(priv->driver, &priv->usbdev);
    }
}

static void sam_ep0_interrupt(struct sam_usbdev_s *priv)
{
  irqstate_t flags;
  uint8_t csr0l;

  flags = enter_critical_section();
  sam_putreg8(0, SAM_USBHS0_INDEX);
  csr0l = sam_getreg8(SAM_USBHS0_CSR0L);

  ep0_trace(priv, T_ENTER, csr0l, priv->ep0state, 0);

  /* Step 1: Handle SENTSTALL — clear it and reset EP0 state.
   * (Harmony: drv_usbhs_device.c line ~2870)
   */

  if (csr0l & USBHS_CSR0L_SENTSTALL)
    {
      ep0_trace(priv, T_STALL, csr0l, priv->ep0state, 0);
      sam_putreg8(0, SAM_USBHS0_CSR0L);
      priv->ep0state = EP0_STATE_IDLE;
      leave_critical_section(flags);
      return;
    }

  /* Step 2: Handle SETUPEND — host aborted the previous transfer.
   * Write SVCSETUPEND to clear it, reset state.
   * (Harmony: line ~2880)
   */

  if (csr0l & USBHS_CSR0L_SETUPEND)
    {
      sam_putreg8(USBHS_CSR0L_SVCSETUPEND, SAM_USBHS0_CSR0L);
      priv->ep0state = EP0_STATE_IDLE;

      /* Re-read CSR0L after clearing SETUPEND — RXPKTRDY may still
       * be set if a new SETUP arrived simultaneously.
       */

      csr0l = sam_getreg8(SAM_USBHS0_CSR0L);
    }

  /* Step 2b: If RXPKTRDY arrives while we're in any TX state, the host
   * has aborted the current transfer and sent a new SETUP. Reset to IDLE
   * so the SETUP processing below handles it.
   * (Harmony: lines 2909-2917 — reset EP0 state on RXPKTRDY in TX states)
   */

  if ((csr0l & USBHS_CSR0L_RXPKTRDY) &&
      (priv->ep0state == EP0_STATE_TX ||
       priv->ep0state == EP0_STATE_TX_LAST ||
       priv->ep0state == EP0_STATE_STATUS_IN))
    {
      priv->ep0state = EP0_STATE_IDLE;
    }

  /* Step 3: TX data stage in progress — TXPKTRDY cleared means the host
   * ACKed our IN data. Load next chunk or finish.
   * (Harmony: TX_DATA_STAGE_IN_PROGRESS handling, line ~3050)
   */

  if (priv->ep0state == EP0_STATE_TX &&
      !(csr0l & USBHS_CSR0L_TXPKTRDY))
    {
      struct sam_req_s *privreq = priv->ep0in->head;
      if (privreq && privreq->req.xfrd < privreq->req.len)
        {
          /* More data to send */

          uint16_t nbytes = privreq->req.len - privreq->req.xfrd;
          if (nbytes > EP0_MAXPACKET)
            {
              nbytes = EP0_MAXPACKET;
            }

          sam_fifo_write(0, (uint8_t *)privreq->req.buf +
                         privreq->req.xfrd, nbytes);
          privreq->req.xfrd += nbytes;

          if (privreq->req.xfrd >= privreq->req.len ||
              nbytes < EP0_MAXPACKET)
            {
              sam_putreg8(USBHS_CSR0L_TXPKTRDY | USBHS_CSR0L_DATAEND,
                          SAM_USBHS0_CSR0L);
              priv->ep0state = EP0_STATE_TX_LAST;
            }
          else
            {
              sam_putreg8(USBHS_CSR0L_TXPKTRDY, SAM_USBHS0_CSR0L);
            }
        }
      else
        {
          /* All data sent, enter idle (status OUT from host is automatic) */

          priv->ep0state = EP0_STATE_IDLE;
          if (privreq)
            {
              leave_critical_section(flags);
              sam_req_complete(priv->ep0in, OK);
              return;
            }
        }

      leave_critical_section(flags);
      return;
    }

  /* TX_LAST: last packet was sent (TXPKTRDY cleared). Xfer done. */

  if (priv->ep0state == EP0_STATE_TX_LAST &&
      !(csr0l & USBHS_CSR0L_TXPKTRDY))
    {
      ep0_trace(priv, T_TX_DONE, csr0l, priv->ep0state, 0);
      priv->ep0state = EP0_STATE_IDLE;
      if (priv->ep0in->head)
        {
          leave_critical_section(flags);
          sam_req_complete(priv->ep0in, OK);
          return;
        }

      leave_critical_section(flags);
      return;
    }

  /* STATUS_IN: SET_ADDRESS status phase. We wrote 0x48 (SVCRXPKTRDY|DATAEND)
   * which makes the MUSB send a ZLP at the OLD address. When the next EP0
   * interrupt fires, the ZLP was ACKed → now safe to apply new FADDR.
   */

  if (priv->ep0state == EP0_STATE_STATUS_IN)
    {
      if (priv->newaddr)
        {
          sam_putreg8(priv->newaddr & 0x7f, SAM_USBHS0_FADDR);
          priv->devaddr = priv->newaddr;
          priv->newaddr = 0;
          priv->devstate = USB_DEVSTATE_ADDRESSED;
        }

      priv->ep0state = EP0_STATE_IDLE;
      leave_critical_section(flags);
      return;
    }

  /* Step 4: RXPKTRDY — SETUP or OUT data received.
   * (Harmony: line ~2900)
   */

  if (csr0l & USBHS_CSR0L_RXPKTRDY)
    {
      uint8_t count = sam_getreg8(SAM_USBHS0_COUNT0);

      if (priv->ep0state == EP0_STATE_IDLE && count == 8)
        {
          /* New SETUP packet */

          struct usb_ctrlreq_s ctrl;
          uint8_t rawsetup[8];

          priv->dbg_setup++;

          sam_fifo_read(0, rawsetup, 8);
          memcpy(&ctrl, rawsetup, 8);
          memcpy(priv->ep0buf, rawsetup, 8);

          uint8_t reqtype = ctrl.type;
          uint8_t breq = ctrl.req;
          uint16_t wvalue = ctrl.value[0] | (ctrl.value[1] << 8);
          uint16_t wlength = ctrl.len[0] | (ctrl.len[1] << 8);

          ep0_trace(priv, T_SETUP, csr0l, priv->ep0state, breq);

          if ((reqtype & USB_REQ_DIR_MASK) == USB_REQ_DIR_IN && wlength > 0)
            {
              /* Device-to-host with data stage (e.g. GET_DESCRIPTOR).
               * Harmony pattern:
               *   1) Write SVCRXPKTRDY (0x40) to clear SETUP
               *   2) Load FIFO with response data
               *   3) Write TXPKTRDY|DATAEND (0x0A) if last packet,
               *      or TXPKTRDY (0x02) if more packets follow
               */

              sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);

              /* All D2H requests go through CLASS_SETUP — NuttX CDC
               * handles GET_DESCRIPTOR, GET_LINE_CODING, etc.
               */

              ep0_trace(priv, T_CLASS, csr0l, priv->ep0state, breq);
              leave_critical_section(flags);
              if (priv->driver)
                {
                  CLASS_SETUP(priv->driver, &priv->usbdev,
                              &ctrl, priv->ep0buf, wlength);
                }

              return;
            }
          else if (wlength == 0)
            {
              /* No-data control transfer (H2D): SET_ADDRESS, SET_CONFIG, etc.
               *
               * SET_ADDRESS is handled directly here (Harmony line ~2991):
               *   Write SVCRXPKTRDY|DATAEND (0x48) — MUSB sends ZLP status
               *   automatically. Then set FADDR immediately (MUSB latches it
               *   after the status phase completes on wire).
               *
               * All other no-data requests go to CLASS_SETUP. The NuttX class
               * driver submits a zero-length EP0 IN request for the status
               * phase, so we only clear RXPKTRDY here (0x40) and let
               * sam_ep0_tx() handle TXPKTRDY|DATAEND when the class submits.
               */

              if (reqtype == 0x00 && breq == USB_REQ_SETADDRESS)
                {
                  sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY | USBHS_CSR0L_DATAEND,
                              SAM_USBHS0_CSR0L);

                  /* Defer FADDR write until status phase completes.
                   * Harmony pattern: write 0x48, wait for next EP0 ISR
                   * (status ZLP sent at old address), then set FADDR.
                   * Writing FADDR immediately causes the ZLP to go out
                   * at the NEW address — host rejects it.
                   */

                  priv->newaddr = wvalue & 0x7f;
                  priv->ep0state = EP0_STATE_STATUS_IN;
                  ep0_trace(priv, T_ADDR, 0x48, EP0_STATE_STATUS_IN,
                            priv->newaddr);
                }
              else
                {
                  sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);

                  ep0_trace(priv, T_CLASS, csr0l, priv->ep0state, breq);
                  leave_critical_section(flags);
                  if (priv->driver)
                    {
                      CLASS_SETUP(priv->driver, &priv->usbdev,
                                  &ctrl, priv->ep0buf, 0);
                    }

                  return;
                }
            }
          else
            {
              /* Host-to-device WITH data stage (e.g. SET_LINE_CODING).
               * Don't call CLASS_SETUP yet — the OUT data hasn't arrived.
               * Save the SETUP packet, clear RXPKTRDY to allow data phase,
               * then receive data into ep0buf. CLASS_SETUP is called AFTER
               * data reception completes (in EP0_STATE_RX handler).
               */

              ep0_trace(priv, T_H2D, csr0l, priv->ep0state, breq);
              memcpy(&priv->ep0setup, &ctrl, sizeof(ctrl));
              priv->ep0state = EP0_STATE_RX;
              priv->ep0rxlen = wlength < EP0_MAXPACKET ?
                               wlength : EP0_MAXPACKET;
              priv->ep0rxoff = 0;
              sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);
            }
        }
      else if (priv->ep0state == EP0_STATE_RX)
        {
          /* OUT data phase — read data into ep0buf, then call CLASS_SETUP
           * with the received data so the class driver gets the actual
           * payload (not the SETUP packet bytes).
           */

          if (count > 0)
            {
              uint16_t remaining = priv->ep0rxlen - priv->ep0rxoff;
              if (count > remaining)
                {
                  count = remaining;
                }

              sam_fifo_read(0, priv->ep0buf + priv->ep0rxoff, count);
              priv->ep0rxoff += count;
            }

          if (priv->ep0rxoff >= priv->ep0rxlen || count < EP0_MAXPACKET)
            {
              /* All OUT data received — send status IN (ZLP) */

              sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY |
                          USBHS_CSR0L_DATAEND, SAM_USBHS0_CSR0L);
              priv->ep0state = EP0_STATE_IDLE;

              ep0_trace(priv, T_H2D_RX, csr0l, EP0_STATE_IDLE,
                        priv->ep0rxoff);

              /* Now call CLASS_SETUP with ep0buf containing the real data */

              leave_critical_section(flags);
              if (priv->driver)
                {
                  CLASS_SETUP(priv->driver, &priv->usbdev,
                              &priv->ep0setup, priv->ep0buf,
                              priv->ep0rxoff);
                }

              return;
            }
          else
            {
              sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);
            }
        }
      else
        {
          /* Unexpected RXPKTRDY — flush it */

          sam_putreg8(USBHS_CSR0L_SVCRXPKTRDY, SAM_USBHS0_CSR0L);
        }
    }

  leave_critical_section(flags);
}

static void sam_epn_tx_interrupt(struct sam_usbdev_s *priv, uint8_t epno)
{
  struct sam_ep_s *privep = NULL;

  priv->dbg_epn_tx_done++;

  /* Clear UNDERRUN if set — not a fatal error, just means host polled
   * while FIFO was empty (normal at end of transfer).
   */

  sam_putreg8(epno, SAM_USBHS0_INDEX);
  uint8_t txcsrl = sam_getreg8(SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
  if (txcsrl & USBHS_TXCSRL_UNDERRUN)
    {
      sam_putreg8(txcsrl & ~USBHS_TXCSRL_UNDERRUN,
                  SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
    }

  /* Find the TX endpoint */

  for (int i = 0; i < SAM_USB_NENDPOINTS * 2; i++)
    {
      if (priv->eplist[i].epphy == epno && priv->eplist[i].txin)
        {
          privep = &priv->eplist[i];
          break;
        }
    }

  if (!privep || !privep->head)
    {
      return;
    }

  struct sam_req_s *privreq = privep->head;

  if (privreq->req.xfrd >= privreq->req.len)
    {
      /* Transfer complete — check if ZLP needed */

      if ((privreq->req.flags & USBDEV_REQFLAGS_NULLPKT) &&
          (privreq->req.len % privep->maxpacket) == 0 &&
          privreq->req.len > 0)
        {
          /* Send ZLP */

          irqstate_t flags = enter_critical_section();
          sam_putreg8(epno, SAM_USBHS0_INDEX);
          sam_putreg8(USBHS_TXCSRL_TXPKTRDY,
                      SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
          leave_critical_section(flags);
          privreq->req.flags &= ~USBDEV_REQFLAGS_NULLPKT;
          return;
        }

      sam_req_complete(privep, OK);

      /* Start next request if queued */

      if (privep->head)
        {
          sam_epn_tx_start(privep);
        }
    }
  else
    {
      /* More data to send */

      sam_epn_tx_start(privep);
    }
}

static void sam_epn_rx_interrupt(struct sam_usbdev_s *priv, uint8_t epno)
{
  struct sam_ep_s *privep = NULL;

  priv->dbg_epn_rx++;

  for (int i = 0; i < SAM_USB_NENDPOINTS * 2; i++)
    {
      if (priv->eplist[i].epphy == epno && !priv->eplist[i].txin)
        {
          privep = &priv->eplist[i];
          break;
        }
    }

  if (!privep)
    {
      return;
    }

  irqstate_t flags = enter_critical_section();
  sam_putreg8(epno, SAM_USBHS0_INDEX);

  uint8_t rxcsrl = sam_getreg8(SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);

  if (!(rxcsrl & USBHS_RXCSRL_RXPKTRDY))
    {
      leave_critical_section(flags);
      return;
    }

  uint16_t count = sam_getreg16(SAM_USB_BASE + SAM_USBHS_RXCOUNT_OFFSET);
  struct sam_req_s *privreq = privep->head;

  if (!privreq)
    {
      priv->dbg_epn_rx_noreq++;
    }
  else
    {
      priv->dbg_epn_rx_bytes += count;
    }

  if (privreq && count > 0)
    {
      uint16_t remaining = privreq->req.len - privreq->req.xfrd;
      uint16_t nbytes = count < remaining ? count : remaining;

      sam_fifo_read(epno, (uint8_t *)privreq->req.buf + privreq->req.xfrd,
                    nbytes);
      privreq->req.xfrd += nbytes;

      /* Clear RXPKTRDY */

      sam_putreg8(0, SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
      leave_critical_section(flags);

      /* Complete if buffer full or short packet */

      if (privreq->req.xfrd >= privreq->req.len ||
          nbytes < privep->maxpacket)
        {
          sam_req_complete(privep, OK);
        }
    }
  else
    {
      /* No request pending — clear RXPKTRDY to NAK next OUT */

      sam_putreg8(0, SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
      leave_critical_section(flags);
    }
}

static void sam_dma_interrupt(struct sam_usbdev_s *priv, uint8_t ch)
{
  uint32_t cntl = getreg32(SAM_USBHS0_DMACNTL(ch));

  if (cntl & USBHS_DMACNTL_DMAERR)
    {
      uerr("USB DMA ch%d bus error\n", ch);
    }

  /* DMA completed — find which EP this channel was serving */

  uint8_t epno = (cntl >> USBHS_DMACNTL_DMAEP_SHIFT) & 0xf;
  bool istx = !!(cntl & USBHS_DMACNTL_DMADIR_TX);

  /* Disable DMA channel */

  putreg32(0, SAM_USBHS0_DMACNTL(ch));

  if (istx)
    {
      /* TX DMA done: set TXPKTRDY to send the loaded data */

      irqstate_t flags = enter_critical_section();
      sam_putreg8(epno, SAM_USBHS0_INDEX);
      sam_putreg8(USBHS_TXCSRL_TXPKTRDY,
                  SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
      leave_critical_section(flags);
    }
  else
    {
      /* RX DMA done: invalidate D-cache for receive buffer */

#ifdef CONFIG_ARMV7M_DCACHE
      /* The DMA address register still holds the buffer pointer */

      uint32_t addr = getreg32(SAM_USBHS0_DMAADDR(ch));
      uint32_t count = getreg32(SAM_USBHS0_DMACOUNT(ch));
      up_invalidate_dcache(addr, addr + count);
#endif

      /* Clear RXPKTRDY */

      irqstate_t flags = enter_critical_section();
      sam_putreg8(epno, SAM_USBHS0_INDEX);
      sam_putreg8(0, SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
      leave_critical_section(flags);
    }
}

static int sam_usbhs_interrupt(int irq, void *context, void *arg)
{
  struct sam_usbdev_s *priv = &g_usbdev;
  uint32_t intflag;
  uint8_t  intrusb;
  uint16_t intrtx;
  uint16_t intrrx;

  priv->dbg_isr++;

  /* Read wrapper INTFLAG to determine source categories */

  intflag = getreg32(SAM_USBHS0_INTFLAG);

  /* Read MUSB-level interrupts BEFORE clearing wrapper INTFLAG.
   * MUSB registers are read-to-clear. If we clear INTFLAG first, a new
   * MUSB event arriving in the gap would set INTFLAG.USB again, but our
   * W1C write to INTFLAG would inadvertently clear it → missed interrupt.
   * Reading MUSB first captures all pending events; then clearing INTFLAG
   * is safe because the events that caused it have been consumed.
   */

  intrusb = sam_getreg8(SAM_USBHS0_INTRUSB);
  intrtx  = sam_getreg16(SAM_USBHS0_INTRTX);
  intrrx  = sam_getreg16(SAM_USBHS0_INTRRX);

  /* Now clear wrapper INTFLAG — MUSB sources have been consumed above */

  putreg32(intflag, SAM_USBHS0_INTFLAG);

  /* USB core interrupts */

  if (intrusb & USBHS_INTRUSB_RESET)
    {
      priv->dbg_reset++;
      sam_usb_reset(priv);
    }

  if (intrusb & USBHS_INTRUSB_DISCON)
    {
      /* Cable disconnected — disable SOFTCONN and mask EPn interrupts
       * to prevent ISR storm from PHY oscillation on VBUS loss.
       */

      uint8_t power = sam_getreg8(SAM_USBHS0_POWER);
      power &= ~USBHS_POWER_SOFTCONN;
      sam_putreg8(power, SAM_USBHS0_POWER);
      sam_putreg16(1u, SAM_USBHS0_INTRTXE);  /* EP0 only */
      sam_putreg16(0, SAM_USBHS0_INTRRXE);
      priv->devstate = USB_DEVSTATE_POWERED;
      priv->connected = false;
      if (priv->driver)
        {
          CLASS_DISCONNECT(priv->driver, &priv->usbdev);
        }
    }

  if (intrusb & USBHS_INTRUSB_SUSPEND)
    {
      priv->dbg_suspend++;
      sam_usb_suspend(priv);
    }

  if (intrusb & USBHS_INTRUSB_RESUME)
    {
      sam_usb_resume(priv);
    }

  /* EP0 interrupt */

  if (intrtx & (1u << 0))
    {
      priv->dbg_ep0++;
      sam_ep0_interrupt(priv);
    }

  /* EPn TX interrupts */

  for (int ep = 1; ep < SAM_USB_NENDPOINTS; ep++)
    {
      if (intrtx & (1u << ep))
        {
          sam_epn_tx_interrupt(priv, ep);
        }
    }

  /* EPn RX interrupts */

  for (int ep = 1; ep < SAM_USB_NENDPOINTS; ep++)
    {
      if (intrrx & (1u << ep))
        {
          sam_epn_rx_interrupt(priv, ep);
        }
    }

  /* DMA interrupts */

  if (intflag & USBHS_INT_DMA)
    {
      uint8_t dmaint = sam_getreg8(SAM_USBHS0_DMAINTR);
      for (int ch = 1; ch <= SAM_USB_NDMA; ch++)
        {
          if (dmaint & (1u << (ch - 1)))
            {
              sam_dma_interrupt(priv, ch);
            }
        }
    }

  return OK;
}

/****************************************************************************
 * Endpoint Operations
 ****************************************************************************/

static int sam_ep_configure(struct usbdev_ep_s *ep,
                            const struct usb_epdesc_s *desc, bool last)
{
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;
  uint8_t epno = privep->epphy;
  uint16_t maxpacket = GETUINT16(desc->mxpacketsize);
  irqstate_t flags;

  struct sam_usbdev_s *priv = &g_usbdev;

  privep->maxpacket = maxpacket;
  privep->ep.maxpacket = maxpacket;
  privep->eptype = desc->attr & USB_EP_ATTR_XFERTYPE_MASK;
  privep->stalled = false;

  flags = enter_critical_section();
  sam_putreg8(epno, SAM_USBHS0_INDEX);

  /* Dynamically allocate FIFO space for this endpoint.
   * Size encoding: 2^(sz+3) bytes. Compute from maxpacket.
   */

  uint8_t fifosz = 0;
  uint16_t sz = 8;
  while (sz < maxpacket && fifosz < 9)
    {
      sz <<= 1;
      fifosz++;
    }

  uint16_t units = sz / 8;  /* FIFO address is in 8-byte units */
  uint16_t fifoaddr = priv->fifo_next;
  priv->fifo_next += units;

  if (privep->txin)
    {
      /* Configure TX FIFO */

      sam_putreg8(fifosz, SAM_USBHS0_TXFIFOSZ);
      sam_putreg16(fifoaddr, SAM_USBHS0_TXFIFOADD);

      /* Configure TX endpoint */

      sam_putreg16(maxpacket, SAM_USB_BASE + SAM_USBHS_TXMAXP_OFFSET);

      /* Flush FIFO and clear data toggle */

      sam_putreg8(USBHS_TXCSRL_CLRDATATOG | USBHS_TXCSRL_FLUSHFIFO,
                  SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);

      /* Configure TXCSRH: MODE bit (bit 5) MUST be set in device mode to
       * enable the endpoint direction as TX. Without it, MUSB core does not
       * generate INTRTX for this endpoint at 480 Mbps.
       * DFP: "The CPU Sets This Bit To Enable The Endpoint Direction As TX"
       */

      uint8_t csrh = USBHS_TXCSRH_MODE;
      if (privep->dmachannel)
        {
          csrh |= USBHS_TXCSRH_DMAREQENAB;
        }

      sam_putreg8(csrh, SAM_USB_BASE + SAM_USBHS_TXCSRH_OFFSET);

      /* Enable TX interrupt for this EP */

      uint16_t txe = sam_getreg16(SAM_USBHS0_INTRTXE);
      txe |= (1u << epno);
      sam_putreg16(txe, SAM_USBHS0_INTRTXE);
    }
  else
    {
      /* Configure RX FIFO */

      sam_putreg8(fifosz, SAM_USBHS0_RXFIFOSZ);
      sam_putreg16(fifoaddr, SAM_USBHS0_RXFIFOADD);

      /* Configure RX endpoint */

      sam_putreg16(maxpacket, SAM_USB_BASE + SAM_USBHS_RXMAXP_OFFSET);

      /* Flush FIFO and clear data toggle */

      sam_putreg8(USBHS_RXCSRL_CLRDATATOG | USBHS_RXCSRL_FLUSHFIFO,
                  SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);

      /* RX uses PIO reads — don't set AUTOCLEAR/DMAREQENAB which
       * would clear RXPKTRDY before our ISR can read the FIFO.
       */

      sam_putreg8(0, SAM_USB_BASE + SAM_USBHS_RXCSRH_OFFSET);

      /* Enable RX interrupt for this EP */

      uint16_t rxe = sam_getreg16(SAM_USBHS0_INTRRXE);
      rxe |= (1u << epno);
      sam_putreg16(rxe, SAM_USBHS0_INTRRXE);
    }

  leave_critical_section(flags);
  return OK;
}

static int sam_ep_disable(struct usbdev_ep_s *ep)
{
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;
  irqstate_t flags;

  flags = enter_critical_section();

  /* Cancel all pending requests */

  while (privep->head)
    {
      sam_req_complete(privep, -ESHUTDOWN);
    }

  privep->stalled = false;
  leave_critical_section(flags);
  return OK;
}

static struct usbdev_req_s *sam_ep_allocreq(struct usbdev_ep_s *ep)
{
  struct sam_req_s *privreq;

  privreq = kmm_zalloc(sizeof(struct sam_req_s));
  if (privreq)
    {
      return &privreq->req;
    }

  return NULL;
}

static void sam_ep_freereq(struct usbdev_ep_s *ep,
                           struct usbdev_req_s *req)
{
  struct sam_req_s *privreq = (struct sam_req_s *)req;
  kmm_free(privreq);
}

#ifdef CONFIG_USBDEV_DMA
static void *sam_ep_allocbuffer(struct usbdev_ep_s *ep, uint16_t nbytes)
{
  /* Allocate 32-byte aligned buffer for D-cache safety */

  return kmm_memalign(32, (nbytes + 31) & ~31u);
}

static void sam_ep_freebuffer(struct usbdev_ep_s *ep, void *buf)
{
  kmm_free(buf);
}
#endif

static int sam_ep_submit(struct usbdev_ep_s *ep, struct usbdev_req_s *req)
{
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;
  struct sam_req_s *privreq = (struct sam_req_s *)req;
  struct sam_usbdev_s *priv = &g_usbdev;
  irqstate_t flags;
  bool empty;

  req->result = -EINPROGRESS;
  req->xfrd = 0;

  flags = enter_critical_section();
  empty = (privep->head == NULL);
  sam_req_enqueue(privep, privreq);
  leave_critical_section(flags);

  /* EP0 handling */

  if (privep->epphy == 0)
    {
      if (privep->txin)
        {
          ep0_trace(priv, T_SUBMIT, 0, priv->ep0state,
                    (uint8_t)req->len);
          priv->ep0state = EP0_STATE_TX;
          sam_ep0_tx(priv);
        }
      else
        {
          priv->ep0state = EP0_STATE_RX;
        }

      return OK;
    }

  /* EPn: kick transfer if queue was empty */

  if (empty)
    {
      if (privep->txin)
        {
          sam_epn_tx_start(privep);
        }
      else
        {
          sam_epn_rx_start(privep);
        }
    }

  return OK;
}

static int sam_ep_cancel(struct usbdev_ep_s *ep, struct usbdev_req_s *req)
{
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;
  irqstate_t flags;

  flags = enter_critical_section();

  /* Find and remove the request from the queue */

  struct sam_req_s *curr = privep->head;
  struct sam_req_s *prev = NULL;

  while (curr)
    {
      if (&curr->req == req)
        {
          if (prev)
            {
              prev->flink = curr->flink;
            }
          else
            {
              privep->head = curr->flink;
            }

          if (!curr->flink)
            {
              privep->tail = prev;
            }

          leave_critical_section(flags);
          curr->req.result = -ECANCELED;
          if (curr->req.callback)
            {
              curr->req.callback(ep, &curr->req);
            }

          return OK;
        }

      prev = curr;
      curr = curr->flink;
    }

  leave_critical_section(flags);
  return -ENOENT;
}

static int sam_ep_stall(struct usbdev_ep_s *ep, bool resume)
{
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;
  uint8_t epno = privep->epphy;
  irqstate_t flags;

  flags = enter_critical_section();
  sam_putreg8(epno, SAM_USBHS0_INDEX);

  if (resume)
    {
      /* Clear stall */

      privep->stalled = false;
      if (epno == 0)
        {
          /* EP0: nothing to clear, stall auto-clears after SENTSTALL */
        }
      else if (privep->txin)
        {
          sam_putreg8(USBHS_TXCSRL_CLRDATATOG,
                      SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
        }
      else
        {
          sam_putreg8(USBHS_RXCSRL_CLRDATATOG,
                      SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
        }
    }
  else
    {
      /* Set stall */

      privep->stalled = true;
      if (epno == 0)
        {
          sam_putreg8(USBHS_CSR0L_SENDSTALL | USBHS_CSR0L_SVCRXPKTRDY,
                      SAM_USBHS0_CSR0L);
        }
      else if (privep->txin)
        {
          uint8_t csr = sam_getreg8(SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
          csr |= USBHS_TXCSRL_SENDSTALL;
          sam_putreg8(csr, SAM_USB_BASE + SAM_USBHS_TXCSRL_OFFSET);
        }
      else
        {
          uint8_t csr = sam_getreg8(SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
          csr |= USBHS_RXCSRL_SENDSTALL;
          sam_putreg8(csr, SAM_USB_BASE + SAM_USBHS_RXCSRL_OFFSET);
        }
    }

  leave_critical_section(flags);
  return OK;
}

/****************************************************************************
 * Device Operations
 ****************************************************************************/

static struct usbdev_ep_s *sam_allocep(struct usbdev_s *dev, uint8_t epphy,
                                       bool in, uint8_t eptype)
{
  struct sam_usbdev_s *priv = (struct sam_usbdev_s *)dev;
  struct sam_ep_s *privep = NULL;
  irqstate_t flags;

  /* EP0 is pre-allocated */

  if (epphy == 0)
    {
      return in ? &priv->ep0in->ep : &priv->ep0out->ep;
    }

  flags = enter_critical_section();

  /* Find a free endpoint matching the request */

  for (int i = 2; i < SAM_USB_NENDPOINTS * 2; i++)
    {
      struct sam_ep_s *ep = &priv->eplist[i];

      if (ep->ep.maxpacket == 0)  /* Not yet allocated */
        {
          /* If caller specified a physical EP, match it */

          if (epphy != 0 && ep->epphy != (epphy & 0x7f))
            {
              continue;
            }

          if (ep->txin != in)
            {
              continue;
            }

          privep = ep;
          privep->eptype = eptype;

          /* Set maxpacket based on speed */

          if (priv->usbdev.dualspeed)
            {
              privep->maxpacket = HS_BULK_MAXPACKET;
            }
          else
            {
              privep->maxpacket = FS_BULK_MAXPACKET;
            }

          privep->ep.maxpacket = privep->maxpacket;

          /* Allocate DMA channel for bulk IN (TX) endpoints only.
           * Interrupt endpoints (CDC notification) must NOT use DMA —
           * they send small packets that don't benefit from DMA and
           * DMAREQENAB in TXCSRH breaks HS notification delivery.
           */

          if (eptype == USB_EP_ATTR_XFER_BULK && in)
            {
              privep->dmachannel = sam_dma_alloc(priv);
            }

          break;
        }
    }

  leave_critical_section(flags);
  return privep ? &privep->ep : NULL;
}

static void sam_freeep(struct usbdev_s *dev, struct usbdev_ep_s *ep)
{
  struct sam_usbdev_s *priv = (struct sam_usbdev_s *)dev;
  struct sam_ep_s *privep = (struct sam_ep_s *)ep;

  if (privep->dmachannel)
    {
      sam_dma_free(priv, privep->dmachannel);
      privep->dmachannel = 0;
    }

  privep->ep.maxpacket = 0;
  privep->maxpacket = 0;
  privep->eptype = 0;
}

static int sam_getframe(struct usbdev_s *dev)
{
  return sam_getreg16(SAM_USBHS0_FRAME) & 0x7ff;
}

static int sam_wakeup(struct usbdev_s *dev)
{
  uint8_t power = sam_getreg8(SAM_USBHS0_POWER);
  power |= USBHS_POWER_RESUME;
  sam_putreg8(power, SAM_USBHS0_POWER);

  up_mdelay(10);

  power &= ~USBHS_POWER_RESUME;
  sam_putreg8(power, SAM_USBHS0_POWER);

  return OK;
}

static int sam_selfpowered(struct usbdev_s *dev, bool selfpowered)
{
  struct sam_usbdev_s *priv = (struct sam_usbdev_s *)dev;
  priv->selfpowered = selfpowered;
  return OK;
}

static int sam_pullup(struct usbdev_s *dev, bool enable)
{
  if (enable)
    {
      /* Start session — powers PHY and enables VBUS detection */

      sam_putreg8(USBHS_DEVCTL_SESSION, SAM_USBHS0_DEVCTL);

      /* Wait for PHY to become ready (up to ~10ms) */

      int timeout = 10000;
      while (!(getreg32(SAM_USBHS0_STATUS) & USBHS_STATUS_PHYRDY) &&
             timeout > 0)
        {
          up_udelay(1);
          timeout--;
        }

      /* Assert D+ pullup — device becomes visible to host */

      uint8_t power = sam_getreg8(SAM_USBHS0_POWER);
      power |= USBHS_POWER_SOFTCONN;
      sam_putreg8(power, SAM_USBHS0_POWER);
    }
  else
    {
      uint8_t power = sam_getreg8(SAM_USBHS0_POWER);
      power &= ~USBHS_POWER_SOFTCONN;
      sam_putreg8(power, SAM_USBHS0_POWER);

      /* End session */

      sam_putreg8(0, SAM_USBHS0_DEVCTL);
    }

  return OK;
}

/****************************************************************************
 * Hardware Initialization
 ****************************************************************************/

static void sam_hw_setup(struct sam_usbdev_s *priv)
{
  /* No SWRST — Harmony's SWRST template writes to wrong address (base+0x2000
   * instead of base+0x0000) so it effectively never resets. Matching that
   * behavior: skip SWRST, go directly to device mode configuration.
   */

  /* Force device mode: disable → set IDOVEN+IDVAL → re-enable */

  uint32_t ctrla = getreg32(SAM_USBHS0_CTRLA);
  ctrla &= ~USBHS_CTRLA_ENABLE;
  putreg32(ctrla, SAM_USBHS0_CTRLA);
  while (getreg32(SAM_USBHS0_SYNCBUSY) & USBHS_SYNCBUSY_ENABLE)
    {
    }

  ctrla |= USBHS_CTRLA_IDOVEN | USBHS_CTRLA_IDVAL;
  putreg32(ctrla, SAM_USBHS0_CTRLA);

  ctrla |= USBHS_CTRLA_ENABLE;
  putreg32(ctrla, SAM_USBHS0_CTRLA);
  while (getreg32(SAM_USBHS0_SYNCBUSY) & USBHS_SYNCBUSY_ENABLE)
    {
    }

  /* Wait for PHY power-on and ready */

  while (!(getreg32(SAM_USBHS0_STATUS) & USBHS_STATUS_PHYON))
    {
    }

  while (!(getreg32(SAM_USBHS0_STATUS) & USBHS_STATUS_PHYRDY))
    {
    }

  /* Enable wrapper-level interrupts (after reset + PHY ready) */

  putreg32(USBHS_INT_USB | USBHS_INT_DMA | USBHS_INT_WAKEUP,
           SAM_USBHS0_INTENSET);

  /* Disable OTG VBUS threshold detection (PHY24 bit 1 = OTGOFF).
   * Without this, the controller auto-clears SESSION when VBUS
   * comparators don't see valid levels, preventing PHY from staying on.
   */

  putreg32(getreg32(SAM_USBHS0_PHY24) | (1u << 1), SAM_USBHS0_PHY24);

  /* Configure POWER: enable HS negotiation, keep disconnected until pullup() */

  /* INTFLAG is write-to-clear (not clearing causes infinite ISR loop).
   * Single NVIC vector (USBHS0) covers all events; USBHS1 is a second
   * controller (unused). HS TX fix: TXCSRH.MODE must be set on EP configure.
   */

  sam_putreg8(USBHS_POWER_ENSUSPEND | USBHS_POWER_HSENABLE,
              SAM_USBHS0_POWER);

  /* EP0 FIFO is hardwired (64 bytes at address 0) — no configuration
   * needed. Harmony confirms: "EP0 does not require any configuration."
   * EP1-7 FIFOs are allocated dynamically in sam_ep_configure().
   */

  sam_putreg8(0, SAM_USBHS0_INDEX);
  priv->fifo_next = FIFO_START_ADDR;

  /* Enable USB interrupts: RESET, SUSPEND, RESUME */

  sam_putreg8(USBHS_INTRUSB_RESET | USBHS_INTRUSB_SUSPEND |
              USBHS_INTRUSB_RESUME, SAM_USBHS0_INTRUSBE);

  /* Enable EP0 TX interrupt only (EPn enabled on configure) */

  sam_putreg16(1u, SAM_USBHS0_INTRTXE);
  sam_putreg16(0, SAM_USBHS0_INTRRXE);

  /* Attach IRQ handler */

  irq_attach(SAM_IRQ_USB, sam_usbhs_interrupt, NULL);
  up_enable_irq(SAM_IRQ_USB);
}

static void sam_hw_shutdown(struct sam_usbdev_s *priv)
{
  (void)priv;

  up_disable_irq(SAM_IRQ_USB);
  irq_detach(SAM_IRQ_USB);

  /* Disconnect and disable */

  sam_putreg8(0, SAM_USBHS0_POWER);

  uint32_t ctrla = getreg32(SAM_USBHS0_CTRLA);
  ctrla &= ~USBHS_CTRLA_ENABLE;
  putreg32(ctrla, SAM_USBHS0_CTRLA);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_usbinitialize
 *
 * Description:
 *   Called from arm_initialize to configure the USBHS controller.
 *   This is called very early; the class driver has not registered yet.
 *
 ****************************************************************************/

volatile uint32_t *sam_usb_dbg_counters(void)
{
  return &g_usbdev.dbg_isr;
}

uint8_t *sam_usb_last_setup(void)
{
  return g_usbdev.ep0buf;
}

struct ep0_trace_entry
{
  uint8_t evt;
  uint8_t csr0l;
  uint8_t state;
  uint8_t data;
};

struct ep0_trace_entry *sam_usb_ep0_trace(void)
{
  return (struct ep0_trace_entry *)g_usbdev.ep0_trace;
}

uint8_t sam_usb_ep0_trace_idx(void)
{
  return g_usbdev.ep0_trace_idx;
}

void arm_usbinitialize(void)
{
  struct sam_usbdev_s *priv = &g_usbdev;

  memset(priv, 0, sizeof(*priv));

  /* Initialize device structure */

  priv->usbdev.ops = &g_devops;
  priv->usbdev.speed = USB_SPEED_FULL;
  priv->usbdev.dualspeed = 1;

  /* Initialize endpoint list:
   * Index 0 = EP0 IN, Index 1 = EP0 OUT
   * Index 2 = EP1 IN, Index 3 = EP1 OUT
   * ...
   */

  for (int i = 0; i < SAM_USB_NENDPOINTS * 2; i++)
    {
      struct sam_ep_s *ep = &priv->eplist[i];
      ep->ep.ops = &g_epops;
      ep->epphy = i / 2;
      ep->txin = (i % 2 == 0);  /* Even=IN(TX), Odd=OUT(RX) */
    }

  /* EP0 setup */

  priv->ep0in = &priv->eplist[0];
  priv->ep0out = &priv->eplist[1];
  priv->ep0in->maxpacket = EP0_MAXPACKET;
  priv->ep0out->maxpacket = EP0_MAXPACKET;
  priv->ep0in->ep.maxpacket = EP0_MAXPACKET;
  priv->ep0out->ep.maxpacket = EP0_MAXPACKET;
  priv->usbdev.ep0 = &priv->ep0in->ep;

  priv->devstate = USB_DEVSTATE_POWERED;
  priv->ep0state = EP0_STATE_IDLE;

  /* Initialize hardware */

  sam_hw_setup(priv);

  uinfo("USBHS0 initialized: STATUS=0x%08lx POWER=0x%02x\n",
        getreg32(SAM_USBHS0_STATUS),
        sam_getreg8(SAM_USBHS0_POWER));
}

/****************************************************************************
 * Name: usbdev_register / usbdev_unregister
 *
 * Description:
 *   Called by the CDC/ACM class driver to bind/unbind.
 *
 ****************************************************************************/

int usbdev_register(struct usbdevclass_driver_s *driver)
{
  struct sam_usbdev_s *priv = &g_usbdev;
  int ret;

  if (!driver || !driver->ops->bind || !driver->ops->unbind ||
      !driver->ops->setup || !driver->ops->disconnect)
    {
      return -EINVAL;
    }

  if (priv->driver)
    {
      return -EBUSY;
    }

  priv->driver = driver;

  ret = CLASS_BIND(driver, &priv->usbdev);
  if (ret < 0)
    {
      priv->driver = NULL;
      uerr("CLASS_BIND failed: %d\n", ret);
      return ret;
    }

  /* D+ pullup is asserted by CLASS via DEV_CONNECT → sam_pullup(true) */

  return OK;
}

int usbdev_unregister(struct usbdevclass_driver_s *driver)
{
  struct sam_usbdev_s *priv = &g_usbdev;

  if (driver != priv->driver)
    {
      return -EINVAL;
    }

  /* Disconnect from bus */

  sam_pullup(&priv->usbdev, false);

  /* Cancel all pending requests */

  for (int i = 0; i < SAM_USB_NENDPOINTS * 2; i++)
    {
      while (priv->eplist[i].head)
        {
          sam_req_complete(&priv->eplist[i], -ESHUTDOWN);
        }
    }

  CLASS_UNBIND(driver, &priv->usbdev);

  sam_hw_shutdown(priv);
  priv->driver = NULL;

  return OK;
}
