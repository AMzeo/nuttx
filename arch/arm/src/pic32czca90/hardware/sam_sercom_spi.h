/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_sercom_spi.h
 *
 * PIC32CZ CA90 SERCOM SPI Master registers.
 * Source: PIC32CZ-CA90_DFP/1.7.168 component/sercom.h (SPIM mode)
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_SPI_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_SPI_H

/****************************************************************************
 * Register Offsets (SPI Master mode)
 ****************************************************************************/

#define SAM_SPI_CTRLA_OFFSET      0x00
#define SAM_SPI_CTRLB_OFFSET      0x04
#define SAM_SPI_CTRLC_OFFSET      0x08
#define SAM_SPI_BAUD_OFFSET       0x0C
#define SAM_SPI_INTENCLR_OFFSET   0x14
#define SAM_SPI_INTENSET_OFFSET   0x16
#define SAM_SPI_INTFLAG_OFFSET    0x18
#define SAM_SPI_STATUS_OFFSET     0x1A
#define SAM_SPI_SYNCBUSY_OFFSET   0x1C
#define SAM_SPI_LENGTH_OFFSET     0x22
#define SAM_SPI_ADDR_OFFSET       0x24
#define SAM_SPI_DATA_OFFSET       0x28
#define SAM_SPI_DBGCTRL_OFFSET    0x30

/****************************************************************************
 * CTRLA — Control A (offset 0x00, R/W 32-bit)
 ****************************************************************************/

#define SPI_CTRLA_SWRST           (1u << 0)
#define SPI_CTRLA_ENABLE          (1u << 1)

#define SPI_CTRLA_MODE_SHIFT      2
#define SPI_CTRLA_MODE_MASK       (0x7u << SPI_CTRLA_MODE_SHIFT)
#define SPI_CTRLA_MODE_SPIM       (0x3u << SPI_CTRLA_MODE_SHIFT)

#define SPI_CTRLA_RUNSTDBY        (1u << 7)
#define SPI_CTRLA_IBON            (1u << 8)

#define SPI_CTRLA_DOPO_SHIFT      16
#define SPI_CTRLA_DOPO_MASK       (0x3u << SPI_CTRLA_DOPO_SHIFT)
#define SPI_CTRLA_DOPO_PAD0       (0x0u << SPI_CTRLA_DOPO_SHIFT)  /* MOSI=PAD0, SCK=PAD1, SS=PAD2 */
#define SPI_CTRLA_DOPO_PAD3       (0x2u << SPI_CTRLA_DOPO_SHIFT)  /* MOSI=PAD3, SCK=PAD1, SS=PAD2 */

#define SPI_CTRLA_DIPO_SHIFT      20
#define SPI_CTRLA_DIPO_MASK       (0x3u << SPI_CTRLA_DIPO_SHIFT)
#define SPI_CTRLA_DIPO_PAD0       (0x0u << SPI_CTRLA_DIPO_SHIFT)
#define SPI_CTRLA_DIPO_PAD1       (0x1u << SPI_CTRLA_DIPO_SHIFT)
#define SPI_CTRLA_DIPO_PAD2       (0x2u << SPI_CTRLA_DIPO_SHIFT)
#define SPI_CTRLA_DIPO_PAD3       (0x3u << SPI_CTRLA_DIPO_SHIFT)

#define SPI_CTRLA_FORM_SHIFT      24
#define SPI_CTRLA_FORM_MASK       (0xFu << SPI_CTRLA_FORM_SHIFT)
#define SPI_CTRLA_FORM_SPI        (0x0u << SPI_CTRLA_FORM_SHIFT)

#define SPI_CTRLA_CPHA            (1u << 28)
#define SPI_CTRLA_CPOL            (1u << 29)
#define SPI_CTRLA_DORD_LSB        (1u << 30)

/****************************************************************************
 * CTRLB — Control B (offset 0x04, R/W 32-bit)
 ****************************************************************************/

#define SPI_CTRLB_CHSIZE_SHIFT    0
#define SPI_CTRLB_CHSIZE_MASK     (0x7u << SPI_CTRLB_CHSIZE_SHIFT)
#define SPI_CTRLB_CHSIZE_8BIT     (0x0u << SPI_CTRLB_CHSIZE_SHIFT)
#define SPI_CTRLB_CHSIZE_9BIT     (0x1u << SPI_CTRLB_CHSIZE_SHIFT)

#define SPI_CTRLB_PLOADEN         (1u << 6)
#define SPI_CTRLB_SSDE            (1u << 9)
#define SPI_CTRLB_MSSEN           (1u << 13)
#define SPI_CTRLB_AMODE_SHIFT     14
#define SPI_CTRLB_AMODE_MASK      (0x3u << SPI_CTRLB_AMODE_SHIFT)
#define SPI_CTRLB_RXEN            (1u << 17)

/****************************************************************************
 * BAUD — Baud Rate (offset 0x0C, R/W 8-bit)
 *
 * f_baud = f_ref / (2 * (BAUD + 1))
 * BAUD = (f_ref / (2 * f_baud)) - 1
 ****************************************************************************/

#define SPI_BAUD_MASK             0xFFu

/****************************************************************************
 * INTENCLR / INTENSET / INTFLAG (offsets 0x14 / 0x16 / 0x18, R/W 8-bit)
 ****************************************************************************/

#define SPI_INT_DRE               (1u << 0)   /* Data Register Empty */
#define SPI_INT_TXC               (1u << 1)   /* Transmit Complete */
#define SPI_INT_RXC               (1u << 2)   /* Receive Complete */
#define SPI_INT_SSL               (1u << 3)   /* Slave Select Low */
#define SPI_INT_ERROR             (1u << 7)   /* Error */

/****************************************************************************
 * STATUS (offset 0x1A, R/W 16-bit)
 ****************************************************************************/

#define SPI_STATUS_BUFOVF         (1u << 2)   /* Buffer Overflow */
#define SPI_STATUS_LENERR         (1u << 11)  /* Transaction Length Error */

/****************************************************************************
 * SYNCBUSY (offset 0x1C, RO 32-bit)
 ****************************************************************************/

#define SPI_SYNCBUSY_SWRST        (1u << 0)
#define SPI_SYNCBUSY_ENABLE       (1u << 1)
#define SPI_SYNCBUSY_CTRLB        (1u << 2)
#define SPI_SYNCBUSY_LENGTH       (1u << 4)

/****************************************************************************
 * DATA (offset 0x28, R/W 32-bit)
 ****************************************************************************/

#define SPI_DATA_MASK             0x1FFu       /* 9-bit max */

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_SPI_H */
