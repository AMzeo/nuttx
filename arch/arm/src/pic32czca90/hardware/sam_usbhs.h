/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_usbhs.h
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

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USBHS_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USBHS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* USBHS Base Addresses (DFP: pic32cz8110ca80208.h) */

#define SAM_USBHS0_BASE             0x4f010000
#define SAM_USBHS1_BASE             0x4f012000

/* Number of endpoints and DMA channels */

#define SAM_USBHS_NENDPOINTS        8   /* EP0 + EP1-EP7 */
#define SAM_USBHS_NDMACHANNELS      8

/* ======================================================================== *
 * Wrapper Registers (Microchip-specific, base + 0x0000)
 * ======================================================================== */

#define SAM_USBHS_CTRLA_OFFSET      0x0000  /* Control A (R/W 32-bit) */
#define SAM_USBHS_CTRLB_OFFSET      0x0004  /* Control B (R/W 32-bit) */
#define SAM_USBHS_CTRLC_OFFSET      0x0008  /* Control C (R/W 32-bit) */
#define SAM_USBHS_INTENCLR_OFFSET   0x000c  /* Interrupt Enable Clear (R/W 32-bit) */
#define SAM_USBHS_INTENSET_OFFSET   0x0010  /* Interrupt Enable Set (R/W 32-bit) */
#define SAM_USBHS_INTFLAG_OFFSET    0x0014  /* Interrupt Flag (R/W 32-bit) */
#define SAM_USBHS_STATUS_OFFSET     0x0018  /* Status (RO 32-bit) */
#define SAM_USBHS_SYNCBUSY_OFFSET   0x001c  /* Sync Busy (RO 32-bit) */

/* CTRLA bits */

#define USBHS_CTRLA_SWRST           (1u << 0)
#define USBHS_CTRLA_ENABLE          (1u << 1)
#define USBHS_CTRLA_IDVAL           (1u << 8)   /* ID override value (1=B-device) */
#define USBHS_CTRLA_IDOVEN          (1u << 9)   /* ID override enable */
#define USBHS_CTRLA_REFCLKSEL       (1u << 10)  /* USB PLL reference clock speed */
#define USBHS_CTRLA_MASK            0x00000703u

/* INTENSET / INTENCLR / INTFLAG bits (same layout) */

#define USBHS_INT_WAKEUP            (1u << 0)
#define USBHS_INT_RESUME            (1u << 1)
#define USBHS_INT_USB               (1u << 2)
#define USBHS_INT_DMA               (1u << 3)
#define USBHS_INT_T1MS              (1u << 4)
#define USBHS_INT_PHYRDY            (1u << 5)
#define USBHS_INT_MASK              0x0000003fu

/* STATUS bits */

#define USBHS_STATUS_PHYRDY         (1u << 0)
#define USBHS_STATUS_PHYON          (1u << 1)
#define USBHS_STATUS_VREGRDY        (1u << 2)

/* SYNCBUSY bits */

#define USBHS_SYNCBUSY_SWRST        (1u << 0)
#define USBHS_SYNCBUSY_ENABLE       (1u << 1)
#define USBHS_SYNCBUSY_T1MSEN       (1u << 2)

/* ======================================================================== *
 * MUSB Function Registers (base + 0x1000)
 * ======================================================================== */

#define SAM_USBHS_FADDR_OFFSET      0x1000  /* Function Address (R/W 8-bit) */
#define SAM_USBHS_POWER_OFFSET      0x1001  /* Power Management (R/W 8-bit) */
#define SAM_USBHS_INTRTX_OFFSET     0x1002  /* TX EP Interrupt Status (RO 16-bit) */
#define SAM_USBHS_INTRRX_OFFSET     0x1004  /* RX EP Interrupt Status (RO 16-bit) */
#define SAM_USBHS_INTRTXE_OFFSET    0x1006  /* TX EP Interrupt Enable (R/W 16-bit) */
#define SAM_USBHS_INTRRXE_OFFSET    0x1008  /* RX EP Interrupt Enable (R/W 16-bit) */
#define SAM_USBHS_INTRUSB_OFFSET    0x100a  /* USB Interrupt Status (RO 8-bit) */
#define SAM_USBHS_INTRUSBE_OFFSET   0x100b  /* USB Interrupt Enable (R/W 8-bit) */
#define SAM_USBHS_FRAME_OFFSET      0x100c  /* Frame Number (RO 16-bit) */
#define SAM_USBHS_INDEX_OFFSET      0x100e  /* Endpoint Index Select (R/W 8-bit) */
#define SAM_USBHS_TESTMODE_OFFSET   0x100f  /* Test Mode (R/W 8-bit) */

/* POWER register bits */

#define USBHS_POWER_ENSUSPEND       (1u << 0)
#define USBHS_POWER_SUSPMODE        (1u << 1)
#define USBHS_POWER_RESUME          (1u << 2)
#define USBHS_POWER_RESET           (1u << 3)  /* RO: bus reset detected */
#define USBHS_POWER_HSMODE          (1u << 4)  /* RO: HS negotiated */
#define USBHS_POWER_HSENABLE        (1u << 5)  /* Enable HS negotiation */
#define USBHS_POWER_SOFTCONN        (1u << 6)  /* D+ pullup (device visible) */
#define USBHS_POWER_ISOUPDATE       (1u << 7)

/* INTRUSB / INTRUSBE register bits */

#define USBHS_INTRUSB_SUSPEND       (1u << 0)
#define USBHS_INTRUSB_RESUME        (1u << 1)
#define USBHS_INTRUSB_RESET         (1u << 2)
#define USBHS_INTRUSB_SOF           (1u << 3)
#define USBHS_INTRUSB_CONN          (1u << 4)
#define USBHS_INTRUSB_DISCON        (1u << 5)
#define USBHS_INTRUSB_SESSREQ       (1u << 6)
#define USBHS_INTRUSB_VBUSERR       (1u << 7)

/* ======================================================================== *
 * INDEX-Selected Endpoint CSR Registers (base + 0x1010)
 * When INDEX=0: EP0 control endpoint (CSR0L/CSR0H/COUNT0)
 * When INDEX=1-7: TXCSRL/TXCSRH/RXCSRL/RXCSRH/RXCOUNT
 * ======================================================================== */

#define SAM_USBHS_TXMAXP_OFFSET     0x1010  /* TX Max Packet (R/W 16-bit) */
#define SAM_USBHS_CSR0L_OFFSET      0x1012  /* EP0 CSR Low (R/W 8-bit, INDEX=0) */
#define SAM_USBHS_CSR0H_OFFSET      0x1013  /* EP0 CSR High (R/W 8-bit, INDEX=0) */
#define SAM_USBHS_TXCSRL_OFFSET     0x1012  /* TX CSR Low (R/W 8-bit, INDEX=1-7) */
#define SAM_USBHS_TXCSRH_OFFSET     0x1013  /* TX CSR High (R/W 8-bit, INDEX=1-7) */
#define SAM_USBHS_RXMAXP_OFFSET     0x1014  /* RX Max Packet (R/W 16-bit) */
#define SAM_USBHS_RXCSRL_OFFSET     0x1016  /* RX CSR Low (R/W 8-bit) */
#define SAM_USBHS_RXCSRH_OFFSET     0x1017  /* RX CSR High (R/W 8-bit) */
#define SAM_USBHS_RXCOUNT_OFFSET    0x1018  /* RX Byte Count (RO 16-bit) */
#define SAM_USBHS_COUNT0_OFFSET     0x1018  /* EP0 RX Count (RO 8-bit, INDEX=0) */
#define SAM_USBHS_TXTYPE_OFFSET     0x101a  /* TX Type (host only, R/W 8-bit) */
#define SAM_USBHS_TXINTERVAL_OFFSET 0x101b  /* TX Interval (host only, R/W 8-bit) */
#define SAM_USBHS_RXTYPE_OFFSET     0x101c  /* RX Type (host only, R/W 8-bit) */
#define SAM_USBHS_RXINTERVAL_OFFSET 0x101d  /* RX Interval (host only, R/W 8-bit) */
#define SAM_USBHS_CONFIGDATA_OFFSET 0x101f  /* Config Data (RO 8-bit, INDEX=0) */
#define SAM_USBHS_FIFOSIZE_OFFSET   0x101f  /* FIFO Size (RO 8-bit, INDEX=1-7) */

/* EP0 CSR0L bits (device/peripheral mode) */

#define USBHS_CSR0L_RXPKTRDY        (1u << 0)  /* SETUP/OUT packet received */
#define USBHS_CSR0L_TXPKTRDY        (1u << 1)  /* IN packet loaded, ready to send */
#define USBHS_CSR0L_SENTSTALL       (1u << 2)  /* STALL was sent (write 0 to clear) */
#define USBHS_CSR0L_DATAEND         (1u << 3)  /* Set with TXPKTRDY on last data pkt */
#define USBHS_CSR0L_SETUPEND        (1u << 4)  /* RO: control xfer ended prematurely */
#define USBHS_CSR0L_SENDSTALL       (1u << 5)  /* Send STALL handshake */
#define USBHS_CSR0L_SVCRXPKTRDY     (1u << 6)  /* Clear RXPKTRDY (serviced) */
#define USBHS_CSR0L_SVCSETUPEND     (1u << 7)  /* Clear SETUPEND (serviced) */

/* EP0 CSR0H bits (device/peripheral mode) */

#define USBHS_CSR0H_FLUSHFIFO       (1u << 0)

/* TX CSRL bits (EP1-7, device/peripheral mode) */

#define USBHS_TXCSRL_TXPKTRDY       (1u << 0)  /* Packet in FIFO ready to send */
#define USBHS_TXCSRL_FIFONOTEMPTY    (1u << 1)  /* RO: FIFO has data */
#define USBHS_TXCSRL_UNDERRUN        (1u << 2)  /* IN token with empty FIFO (ISO) */
#define USBHS_TXCSRL_FLUSHFIFO      (1u << 3)  /* Flush TX FIFO */
#define USBHS_TXCSRL_SENDSTALL      (1u << 4)  /* Send STALL on next IN */
#define USBHS_TXCSRL_SENTSTALL      (1u << 5)  /* STALL was sent (write 0 to clear) */
#define USBHS_TXCSRL_CLRDATATOG     (1u << 6)  /* Reset data toggle to 0 */
#define USBHS_TXCSRL_INCOMPTX       (1u << 7)  /* Large ISO pkt not fully sent */

/* TX CSRH bits (EP1-7, device/peripheral mode) */

#define USBHS_TXCSRH_DMAREQMODE     (1u << 2)  /* DMA request mode (0=mode0, 1=mode1) */
#define USBHS_TXCSRH_FRCDATATOG     (1u << 3)  /* Force data toggle */
#define USBHS_TXCSRH_DMAREQENAB     (1u << 4)  /* Enable DMA for this EP */
#define USBHS_TXCSRH_MODE           (1u << 5)  /* Must be 1 for TX in device mode */
#define USBHS_TXCSRH_ISO            (1u << 6)  /* Isochronous mode */
#define USBHS_TXCSRH_AUTOSET        (1u << 7)  /* Auto-set TXPKTRDY on DMA complete */

/* RX CSRL bits (EP1-7, device/peripheral mode) */

#define USBHS_RXCSRL_RXPKTRDY       (1u << 0)  /* Packet received in FIFO */
#define USBHS_RXCSRL_FIFOFULL       (1u << 1)  /* RO: RX FIFO is full */
#define USBHS_RXCSRL_OVERRUN        (1u << 2)  /* RX overrun (ISO) */
#define USBHS_RXCSRL_DATAERROR      (1u << 3)  /* Data error (ISO) */
#define USBHS_RXCSRL_FLUSHFIFO      (1u << 4)  /* Flush RX FIFO */
#define USBHS_RXCSRL_SENDSTALL      (1u << 5)  /* Send STALL on next OUT */
#define USBHS_RXCSRL_SENTSTALL      (1u << 6)  /* STALL was sent (write 0 to clear) */
#define USBHS_RXCSRL_CLRDATATOG     (1u << 7)  /* Reset data toggle to 0 */

/* RX CSRH bits (EP1-7, device/peripheral mode) */

#define USBHS_RXCSRH_INCOMPRX       (1u << 0)  /* Large ISO pkt not fully received */
#define USBHS_RXCSRH_DMAREQMODE     (1u << 3)  /* DMA request mode */
#define USBHS_RXCSRH_DISNYET        (1u << 4)  /* Disable NYET (bulk) or PID err */
#define USBHS_RXCSRH_DMAREQENAB     (1u << 5)  /* Enable DMA for this EP */
#define USBHS_RXCSRH_ISO            (1u << 6)  /* Isochronous mode */
#define USBHS_RXCSRH_AUTOCLEAR      (1u << 7)  /* Auto-clear RXPKTRDY on DMA done */

/* ======================================================================== *
 * FIFO Access Registers (base + 0x1020, stride 4)
 * ======================================================================== */

#define SAM_USBHS_FIFO_OFFSET(ep)   (0x1020 + ((ep) * 4))

/* ======================================================================== *
 * Device Control / FIFO Configuration (base + 0x1060)
 * ======================================================================== */

#define SAM_USBHS_DEVCTL_OFFSET     0x1060  /* Device Control (R/W 8-bit) */
#define SAM_USBHS_MISC_OFFSET       0x1061  /* RX/TX Early DMA (R/W 8-bit) */
#define SAM_USBHS_TXFIFOSZ_OFFSET   0x1062  /* TX FIFO Size (R/W 8-bit) */
#define SAM_USBHS_RXFIFOSZ_OFFSET   0x1063  /* RX FIFO Size (R/W 8-bit) */
#define SAM_USBHS_TXFIFOADD_OFFSET  0x1064  /* TX FIFO Start Addr (R/W 16-bit) */
#define SAM_USBHS_RXFIFOADD_OFFSET  0x1066  /* RX FIFO Start Addr (R/W 16-bit) */

/* DEVCTL bits */

#define USBHS_DEVCTL_SESSION        (1u << 0)
#define USBHS_DEVCTL_HOSTREQ        (1u << 1)
#define USBHS_DEVCTL_HOSTMODE       (1u << 2)  /* RO */
#define USBHS_DEVCTL_VBUS_SHIFT     3
#define USBHS_DEVCTL_VBUS_MASK      (0x3u << 3)
#define USBHS_DEVCTL_LSDEV          (1u << 5)  /* RO */
#define USBHS_DEVCTL_FSDEV          (1u << 6)  /* RO */
#define USBHS_DEVCTL_BDEVICE        (1u << 7)  /* RO */

/* VBUS levels from DEVCTL[4:3] */

#define USBHS_VBUS_SESSEND          (0u << 3)
#define USBHS_VBUS_ABOVESESSEND     (1u << 3)
#define USBHS_VBUS_ABOVEAVALID      (2u << 3)
#define USBHS_VBUS_ABOVEVBUSVALID   (3u << 3)

/* FIFO size encoding: size = 2^(SZ+3) bytes */

#define USBHS_FIFOSZ_8              0   /* 8 bytes */
#define USBHS_FIFOSZ_16             1   /* 16 bytes */
#define USBHS_FIFOSZ_32             2   /* 32 bytes */
#define USBHS_FIFOSZ_64             3   /* 64 bytes */
#define USBHS_FIFOSZ_128            4   /* 128 bytes */
#define USBHS_FIFOSZ_256            5   /* 256 bytes */
#define USBHS_FIFOSZ_512            6   /* 512 bytes */
#define USBHS_FIFOSZ_1024           7   /* 1024 bytes */
#define USBHS_FIFOSZ_2048           8   /* 2048 bytes */
#define USBHS_FIFOSZ_4096           9   /* 4096 bytes */
#define USBHS_FIFOSZ_DPB            (1u << 4)  /* Double Packet Buffer */

/* FIFO address is in units of 8 bytes */

#define USBHS_FIFOADD_SHIFT         0
#define USBHS_FIFOADD_MASK          0x1fffu

/* ======================================================================== *
 * Endpoint Info (base + 0x1078)
 * ======================================================================== */

#define SAM_USBHS_EPINFO_OFFSET     0x1078  /* EP Info (RO 8-bit) */
#define SAM_USBHS_RAMINFO_OFFSET    0x1079  /* RAM Info (RO 8-bit) */
#define SAM_USBHS_LINKINFO_OFFSET   0x107a  /* Link Info (R/W 8-bit) */
#define SAM_USBHS_VPLEN_OFFSET      0x107b  /* VBus Pulse Length (R/W 8-bit) */
#define SAM_USBHS_HSEOF1_OFFSET     0x107c  /* HS EOF1 (R/W 8-bit) */
#define SAM_USBHS_FSEOF1_OFFSET     0x107d  /* FS EOF1 (R/W 8-bit) */
#define SAM_USBHS_LSEOF1_OFFSET     0x107e  /* LS EOF1 (R/W 8-bit) */
#define SAM_USBHS_SOFTRST_OFFSET    0x107f  /* Soft Reset (R/W 8-bit) */

/* SOFTRST bits */

#define USBHS_SOFTRST_NRST          (1u << 0)
#define USBHS_SOFTRST_NRSTX         (1u << 1)

/* ======================================================================== *
 * DMA Registers (base + 0x1200)
 * Channel N (1-8) at offset 0x1204 + (N-1)*0x10
 * ======================================================================== */

#define SAM_USBHS_DMAINTR_OFFSET    0x1200  /* DMA Interrupt Status (R/W 32-bit) */

#define SAM_USBHS_DMACNTL_OFFSET(ch)   (0x1204 + ((ch) - 1) * 0x10)
#define SAM_USBHS_DMAADDR_OFFSET(ch)   (0x1208 + ((ch) - 1) * 0x10)
#define SAM_USBHS_DMACOUNT_OFFSET(ch)  (0x120c + ((ch) - 1) * 0x10)

/* DMACNTL bits */

#define USBHS_DMACNTL_DMAEN         (1u << 0)
#define USBHS_DMACNTL_DMADIR_TX     (1u << 1)  /* 1=memory→EP (TX) */
#define USBHS_DMACNTL_DMADIR_RX     (0u << 1)  /* 0=EP→memory (RX) */
#define USBHS_DMACNTL_DMAMODE       (1u << 2)  /* 0=mode0, 1=mode1 */
#define USBHS_DMACNTL_DMAIE         (1u << 3)  /* Interrupt on completion */
#define USBHS_DMACNTL_DMAEP_SHIFT   4
#define USBHS_DMACNTL_DMAEP_MASK    (0xfu << 4)
#define USBHS_DMACNTL_DMAEP(ep)     (((uint32_t)(ep) << 4) & USBHS_DMACNTL_DMAEP_MASK)
#define USBHS_DMACNTL_DMAERR        (1u << 8)  /* RO: bus error */
#define USBHS_DMACNTL_DMABRSTM_SHIFT 9
#define USBHS_DMACNTL_DMABRSTM_MASK (0x3u << 9)
#define USBHS_DMACNTL_DMABRSTM(m)   (((uint32_t)(m) << 9) & USBHS_DMACNTL_DMABRSTM_MASK)

/* ======================================================================== *
 * Double Packet Buffer Disable (base + 0x1340)
 * ======================================================================== */

#define SAM_USBHS_RXDPKTBUFDIS_OFFSET 0x1340  /* RX DPB Disable (R/W 16-bit) */
#define SAM_USBHS_TXDPKTBUFDIS_OFFSET 0x1342  /* TX DPB Disable (R/W 16-bit) */

/* ======================================================================== *
 * PHY Registers (base + 0x1500)
 * ======================================================================== */

#define SAM_USBHS_PHY00_OFFSET      0x1500
#define SAM_USBHS_PHY04_OFFSET      0x1504
#define SAM_USBHS_PHY08_OFFSET      0x1508
#define SAM_USBHS_PHY0C_OFFSET      0x150c
#define SAM_USBHS_PHY10_OFFSET      0x1510
#define SAM_USBHS_PHY14_OFFSET      0x1514
#define SAM_USBHS_PHY18_OFFSET      0x1518
#define SAM_USBHS_PHY1C_OFFSET      0x151c
#define SAM_USBHS_PHY20_OFFSET      0x1520
#define SAM_USBHS_PHY24_OFFSET      0x1524
#define SAM_USBHS_PHY28_OFFSET      0x1528
#define SAM_USBHS_PHY44_OFFSET      0x1544
#define SAM_USBHS_PHY48_OFFSET      0x1548
#define SAM_USBHS_PHY4C_OFFSET      0x154c
#define SAM_USBHS_PHY50_OFFSET      0x1550

/* ======================================================================== *
 * Absolute Register Addresses (USBHS0)
 * ======================================================================== */

#define SAM_USBHS0_CTRLA            (SAM_USBHS0_BASE + SAM_USBHS_CTRLA_OFFSET)
#define SAM_USBHS0_CTRLB            (SAM_USBHS0_BASE + SAM_USBHS_CTRLB_OFFSET)
#define SAM_USBHS0_INTENCLR         (SAM_USBHS0_BASE + SAM_USBHS_INTENCLR_OFFSET)
#define SAM_USBHS0_INTENSET         (SAM_USBHS0_BASE + SAM_USBHS_INTENSET_OFFSET)
#define SAM_USBHS0_INTFLAG          (SAM_USBHS0_BASE + SAM_USBHS_INTFLAG_OFFSET)
#define SAM_USBHS0_STATUS           (SAM_USBHS0_BASE + SAM_USBHS_STATUS_OFFSET)
#define SAM_USBHS0_SYNCBUSY         (SAM_USBHS0_BASE + SAM_USBHS_SYNCBUSY_OFFSET)

#define SAM_USBHS0_FADDR            (SAM_USBHS0_BASE + SAM_USBHS_FADDR_OFFSET)
#define SAM_USBHS0_POWER            (SAM_USBHS0_BASE + SAM_USBHS_POWER_OFFSET)
#define SAM_USBHS0_INTRTX           (SAM_USBHS0_BASE + SAM_USBHS_INTRTX_OFFSET)
#define SAM_USBHS0_INTRRX           (SAM_USBHS0_BASE + SAM_USBHS_INTRRX_OFFSET)
#define SAM_USBHS0_INTRTXE          (SAM_USBHS0_BASE + SAM_USBHS_INTRTXE_OFFSET)
#define SAM_USBHS0_INTRRXE          (SAM_USBHS0_BASE + SAM_USBHS_INTRRXE_OFFSET)
#define SAM_USBHS0_INTRUSB          (SAM_USBHS0_BASE + SAM_USBHS_INTRUSB_OFFSET)
#define SAM_USBHS0_INTRUSBE         (SAM_USBHS0_BASE + SAM_USBHS_INTRUSBE_OFFSET)
#define SAM_USBHS0_FRAME            (SAM_USBHS0_BASE + SAM_USBHS_FRAME_OFFSET)
#define SAM_USBHS0_INDEX            (SAM_USBHS0_BASE + SAM_USBHS_INDEX_OFFSET)

#define SAM_USBHS0_CSR0L            (SAM_USBHS0_BASE + SAM_USBHS_CSR0L_OFFSET)
#define SAM_USBHS0_CSR0H            (SAM_USBHS0_BASE + SAM_USBHS_CSR0H_OFFSET)
#define SAM_USBHS0_COUNT0           (SAM_USBHS0_BASE + SAM_USBHS_COUNT0_OFFSET)
#define SAM_USBHS0_TXMAXP           (SAM_USBHS0_BASE + SAM_USBHS_TXMAXP_OFFSET)
#define SAM_USBHS0_TXCSRL           (SAM_USBHS0_BASE + SAM_USBHS_TXCSRL_OFFSET)
#define SAM_USBHS0_TXCSRH           (SAM_USBHS0_BASE + SAM_USBHS_TXCSRH_OFFSET)
#define SAM_USBHS0_RXMAXP           (SAM_USBHS0_BASE + SAM_USBHS_RXMAXP_OFFSET)
#define SAM_USBHS0_RXCSRL           (SAM_USBHS0_BASE + SAM_USBHS_RXCSRL_OFFSET)
#define SAM_USBHS0_RXCSRH           (SAM_USBHS0_BASE + SAM_USBHS_RXCSRH_OFFSET)
#define SAM_USBHS0_RXCOUNT          (SAM_USBHS0_BASE + SAM_USBHS_RXCOUNT_OFFSET)

#define SAM_USBHS0_DEVCTL           (SAM_USBHS0_BASE + SAM_USBHS_DEVCTL_OFFSET)
#define SAM_USBHS0_TXFIFOSZ         (SAM_USBHS0_BASE + SAM_USBHS_TXFIFOSZ_OFFSET)
#define SAM_USBHS0_RXFIFOSZ         (SAM_USBHS0_BASE + SAM_USBHS_RXFIFOSZ_OFFSET)
#define SAM_USBHS0_TXFIFOADD        (SAM_USBHS0_BASE + SAM_USBHS_TXFIFOADD_OFFSET)
#define SAM_USBHS0_RXFIFOADD        (SAM_USBHS0_BASE + SAM_USBHS_RXFIFOADD_OFFSET)
#define SAM_USBHS0_FIFO(ep)         (SAM_USBHS0_BASE + SAM_USBHS_FIFO_OFFSET(ep))

#define SAM_USBHS0_EPINFO           (SAM_USBHS0_BASE + SAM_USBHS_EPINFO_OFFSET)
#define SAM_USBHS0_RAMINFO          (SAM_USBHS0_BASE + SAM_USBHS_RAMINFO_OFFSET)

#define SAM_USBHS0_DMAINTR          (SAM_USBHS0_BASE + SAM_USBHS_DMAINTR_OFFSET)
#define SAM_USBHS0_DMACNTL(ch)      (SAM_USBHS0_BASE + SAM_USBHS_DMACNTL_OFFSET(ch))
#define SAM_USBHS0_DMAADDR(ch)      (SAM_USBHS0_BASE + SAM_USBHS_DMAADDR_OFFSET(ch))
#define SAM_USBHS0_DMACOUNT(ch)     (SAM_USBHS0_BASE + SAM_USBHS_DMACOUNT_OFFSET(ch))

#define SAM_USBHS0_RXDPKTBUFDIS     (SAM_USBHS0_BASE + SAM_USBHS_RXDPKTBUFDIS_OFFSET)
#define SAM_USBHS0_TXDPKTBUFDIS     (SAM_USBHS0_BASE + SAM_USBHS_TXDPKTBUFDIS_OFFSET)

#define SAM_USBHS0_PHY24            (SAM_USBHS0_BASE + SAM_USBHS_PHY24_OFFSET)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USBHS_H */
