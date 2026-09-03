/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_usart.h
 *
 * PIC32CZ CA90 SERCOM USART register definitions
 * Same SERCOM USART IP as SAMD5E5
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USART_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USART_H

#include <nuttx/config.h>
#include "hardware/sam_memorymap.h"

/* USART register offsets */

#define SAM_USART_CTRLA_OFFSET       0x0000
#define SAM_USART_CTRLB_OFFSET       0x0004
#define SAM_USART_CTRLC_OFFSET       0x0008
#define SAM_USART_BAUD_OFFSET        0x000c
#define SAM_USART_RXPL_OFFSET        0x000e
#define SAM_USART_INTENCLR_OFFSET    0x0014
#define SAM_USART_INTENSET_OFFSET    0x0016
#define SAM_USART_INTFLAG_OFFSET     0x0018
#define SAM_USART_STATUS_OFFSET      0x001a
#define SAM_USART_SYNCBUSY_OFFSET    0x001c
#define SAM_USART_RXERRCNT_OFFSET    0x0020
#define SAM_USART_LENGTH_OFFSET      0x0022
#define SAM_USART_DATA_OFFSET        0x0028
#define SAM_USART_DBGCTRL_OFFSET     0x0030

/* USART CTRLA register bit definitions */

#define USART_CTRLA_SWRST           (1 << 0)
#define USART_CTRLA_ENABLE          (1 << 1)
#define USART_CTRLA_MODE_SHIFT      2
#define USART_CTRLA_MODE_MASK       (0x7 << USART_CTRLA_MODE_SHIFT)
#  define USART_CTRLA_MODE_EXTUSART (0x0 << USART_CTRLA_MODE_SHIFT)
#  define USART_CTRLA_MODE_INTUSART (0x1 << USART_CTRLA_MODE_SHIFT)
#  define USART_CTRLA_MODE_EXTCLK   USART_CTRLA_MODE_EXTUSART
#  define USART_CTRLA_MODE_INTCLK   USART_CTRLA_MODE_INTUSART
#define USART_CTRLA_RUNSTDBY        (1 << 7)
#define USART_CTRLA_IBON            (1 << 8)
#define USART_CTRLA_TXINV           (1 << 10)
#define USART_CTRLA_RXINV           (1 << 11)
#define USART_CTRLA_SAMPR_SHIFT     13
#define USART_CTRLA_SAMPR_MASK      (0x7 << USART_CTRLA_SAMPR_SHIFT)
#  define USART_CTRLA_SAMPR_16XARITH (0 << USART_CTRLA_SAMPR_SHIFT)
#  define USART_CTRLA_SAMPR_16XFRAC  (1 << USART_CTRLA_SAMPR_SHIFT)
#  define USART_CTRLA_SAMPR_8XARITH  (2 << USART_CTRLA_SAMPR_SHIFT)
#  define USART_CTRLA_SAMPR_8XFRAC   (3 << USART_CTRLA_SAMPR_SHIFT)
#  define USART_CTRLA_SAMPR_3XARITH  (4 << USART_CTRLA_SAMPR_SHIFT)
#define USART_CTRLA_TXPO_SHIFT      16
#define USART_CTRLA_TXPO_MASK       (0x3 << USART_CTRLA_TXPO_SHIFT)
#  define USART_CTRLA_TXPO_PAD0     (0 << USART_CTRLA_TXPO_SHIFT)
#  define USART_CTRLA_TXPO_PAD2     (1 << USART_CTRLA_TXPO_SHIFT)
#  define USART_CTRLA_TXPAD0_1      (0 << USART_CTRLA_TXPO_SHIFT)
#  define USART_CTRLA_TXPAD0_2      (2 << USART_CTRLA_TXPO_SHIFT)
#  define USART_CTRLA_TXPAD0_3      (3 << USART_CTRLA_TXPO_SHIFT)
#define USART_CTRLA_RXPO_SHIFT      20
#define USART_CTRLA_RXPO_MASK       (0x3 << USART_CTRLA_RXPO_SHIFT)
#  define USART_CTRLA_RXPO_PAD0     (0 << USART_CTRLA_RXPO_SHIFT)
#  define USART_CTRLA_RXPO_PAD1     (1 << USART_CTRLA_RXPO_SHIFT)
#  define USART_CTRLA_RXPO_PAD2     (2 << USART_CTRLA_RXPO_SHIFT)
#  define USART_CTRLA_RXPO_PAD3     (3 << USART_CTRLA_RXPO_SHIFT)
#  define USART_CTRLA_RXPAD0        USART_CTRLA_RXPO_PAD0
#  define USART_CTRLA_RXPAD1        USART_CTRLA_RXPO_PAD1
#  define USART_CTRLA_RXPAD2        USART_CTRLA_RXPO_PAD2
#  define USART_CTRLA_RXPAD3        USART_CTRLA_RXPO_PAD3
#define USART_CTRLA_FORM_SHIFT      24
#define USART_CTRLA_FORM_MASK       (0xf << USART_CTRLA_FORM_SHIFT)
#  define USART_CTRLA_FORM_NOPARITY (0 << USART_CTRLA_FORM_SHIFT)
#  define USART_CTRLA_FORM_PARITY   (1 << USART_CTRLA_FORM_SHIFT)
#define USART_CTRLA_CMODE           (1 << 28)
#  define USART_CTRLA_ASYNCH        (0)
#  define USART_CTRLA_SYNCH         USART_CTRLA_CMODE
#define USART_CTRLA_CPOL            (1 << 29)
#  define USART_CTRLA_CPOL_NORMAL   (0)
#  define USART_CTRLA_CPOL_INVERTED USART_CTRLA_CPOL
#define USART_CTRLA_DORD            (1 << 30)
#  define USART_CTRLA_MSBFIRST      (0)
#  define USART_CTRLA_LSBFIRST      USART_CTRLA_DORD

/* USART CTRLB register */

#define USART_CTRLB_CHSIZE_SHIFT    0
#define USART_CTRLB_CHSIZE_MASK     (0x7 << USART_CTRLB_CHSIZE_SHIFT)
#  define USART_CTRLB_CHSIZE_8BITS  (0 << USART_CTRLB_CHSIZE_SHIFT)
#  define USART_CTRLB_CHSIZE_9BITS  (1 << USART_CTRLB_CHSIZE_SHIFT)
#  define USART_CTRLB_CHSIZE_5BITS  (5 << USART_CTRLB_CHSIZE_SHIFT)
#  define USART_CTRLB_CHSIZE_6BITS  (6 << USART_CTRLB_CHSIZE_SHIFT)
#  define USART_CTRLB_CHSIZE_7BITS  (7 << USART_CTRLB_CHSIZE_SHIFT)
#define USART_CTRLB_SBMODE          (1 << 6)
#define USART_CTRLB_COLDEN          (1 << 8)
#define USART_CTRLB_SFDE            (1 << 9)
#define USART_CTRLB_ENC             (1 << 10)
#define USART_CTRLB_PMODE           (1 << 13)
#  define USART_CTRLB_PEVEN         (0)
#  define USART_CTRLB_PODD          USART_CTRLB_PMODE
#define USART_CTRLB_TXEN            (1 << 16)
#define USART_CTRLB_RXEN            (1 << 17)
#define USART_CTRLB_LINCMD_SHIFT    24
#define USART_CTRLB_LINCMD_MASK     (0x3 << USART_CTRLB_LINCMD_SHIFT)

/* USART interrupt flags */

#define USART_INT_DRE               (1 << 0)
#define USART_INT_TXC               (1 << 1)
#define USART_INT_RXC               (1 << 2)
#define USART_INT_RXS               (1 << 3)
#define USART_INT_CTSIC             (1 << 4)
#define USART_INT_RXBRK             (1 << 5)
#define USART_INT_ERROR             (1 << 7)
#define USART_INT_ALL               0xbf

/* USART STATUS register */

#define USART_STATUS_PERR           (1 << 0)
#define USART_STATUS_FERR           (1 << 1)
#define USART_STATUS_BUFOVF         (1 << 2)
#define USART_STATUS_CTS            (1 << 3)
#define USART_STATUS_ISF            (1 << 4)
#define USART_STATUS_COLL           (1 << 5)
#define USART_STATUS_TXE            (1 << 6)

/* USART SYNCBUSY register */

#define USART_SYNCBUSY_SWRST        (1 << 0)
#define USART_SYNCBUSY_ENABLE       (1 << 1)
#define USART_SYNCBUSY_CTRLB        (1 << 2)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_USART_H */
