/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/hardware/sam_sercom_i2c.h
 *
 * PIC32CZ CA90 SERCOM I2C Master registers.
 * Source: PIC32CZ-CA90_DFP/1.7.168 component/sercom.h (I2CM mode)
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_I2C_H
#define __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_I2C_H

/****************************************************************************
 * Register Offsets (I2C Master mode)
 ****************************************************************************/

#define SAM_I2C_CTRLA_OFFSET      0x00
#define SAM_I2C_CTRLB_OFFSET      0x04
#define SAM_I2C_CTRLC_OFFSET      0x08
#define SAM_I2C_BAUD_OFFSET       0x0C
#define SAM_I2C_INTENCLR_OFFSET   0x14
#define SAM_I2C_INTENSET_OFFSET   0x16
#define SAM_I2C_INTFLAG_OFFSET    0x18
#define SAM_I2C_STATUS_OFFSET     0x1A
#define SAM_I2C_SYNCBUSY_OFFSET   0x1C
#define SAM_I2C_ADDR_OFFSET       0x24
#define SAM_I2C_DATA_OFFSET       0x28
#define SAM_I2C_DBGCTRL_OFFSET    0x30

/****************************************************************************
 * CTRLA — Control A (offset 0x00, R/W 32-bit)
 ****************************************************************************/

#define I2C_CTRLA_SWRST           (1u << 0)
#define I2C_CTRLA_ENABLE          (1u << 1)

#define I2C_CTRLA_MODE_SHIFT      2
#define I2C_CTRLA_MODE_MASK       (0x7u << I2C_CTRLA_MODE_SHIFT)
#define I2C_CTRLA_MODE_I2CM       (0x5u << I2C_CTRLA_MODE_SHIFT)

#define I2C_CTRLA_RUNSTDBY        (1u << 7)

#define I2C_CTRLA_SDAHOLD_SHIFT   20
#define I2C_CTRLA_SDAHOLD_MASK    (0x3u << I2C_CTRLA_SDAHOLD_SHIFT)
#define I2C_CTRLA_SDAHOLD_DIS     (0x0u << I2C_CTRLA_SDAHOLD_SHIFT)
#define I2C_CTRLA_SDAHOLD_75NS    (0x1u << I2C_CTRLA_SDAHOLD_SHIFT)
#define I2C_CTRLA_SDAHOLD_450NS   (0x2u << I2C_CTRLA_SDAHOLD_SHIFT)
#define I2C_CTRLA_SDAHOLD_600NS   (0x3u << I2C_CTRLA_SDAHOLD_SHIFT)

#define I2C_CTRLA_SPEED_SHIFT     24
#define I2C_CTRLA_SPEED_MASK      (0x3u << I2C_CTRLA_SPEED_SHIFT)
#define I2C_CTRLA_SPEED_SM        (0x0u << I2C_CTRLA_SPEED_SHIFT)  /* Standard + Fast mode */
#define I2C_CTRLA_SPEED_FMP       (0x1u << I2C_CTRLA_SPEED_SHIFT)  /* Fast mode plus */
#define I2C_CTRLA_SPEED_HS        (0x2u << I2C_CTRLA_SPEED_SHIFT)  /* High speed */

#define I2C_CTRLA_SCLSM          (1u << 27)

#define I2C_CTRLA_SLEWRATE_SHIFT  10
#define I2C_CTRLA_SLEWRATE_MASK   (0x3u << I2C_CTRLA_SLEWRATE_SHIFT)
#define I2C_CTRLA_SLEWRATE_SM     (0x0u << I2C_CTRLA_SLEWRATE_SHIFT)
#define I2C_CTRLA_SLEWRATE_FM     (0x1u << I2C_CTRLA_SLEWRATE_SHIFT)
#define I2C_CTRLA_SLEWRATE_FMP    (0x2u << I2C_CTRLA_SLEWRATE_SHIFT)

/****************************************************************************
 * CTRLB — Control B (offset 0x04, R/W 32-bit)
 ****************************************************************************/

#define I2C_CTRLB_SMEN            (1u << 8)   /* Smart mode enable */
#define I2C_CTRLB_QCEN            (1u << 9)   /* Quick command enable */

#define I2C_CTRLB_CMD_SHIFT       16
#define I2C_CTRLB_CMD_MASK        (0x3u << I2C_CTRLB_CMD_SHIFT)
#define I2C_CTRLB_CMD_NOP         (0x0u << I2C_CTRLB_CMD_SHIFT)
#define I2C_CTRLB_CMD_BYTERD      (0x1u << I2C_CTRLB_CMD_SHIFT)  /* Execute ACK/NACK + read next byte */
#define I2C_CTRLB_CMD_RESTART     (0x2u << I2C_CTRLB_CMD_SHIFT)  /* Execute ACK/NACK + repeated start */
#define I2C_CTRLB_CMD_STOP        (0x3u << I2C_CTRLB_CMD_SHIFT)  /* Execute ACK/NACK + stop */

#define I2C_CTRLB_ACKACT          (1u << 18)  /* 0=ACK, 1=NACK on next read */

/****************************************************************************
 * INTFLAG / INTENSET / INTENCLR (offset 0x18/0x16/0x14, 8-bit)
 ****************************************************************************/

#define I2C_INT_MB                (1u << 0)   /* Master on Bus */
#define I2C_INT_SB                (1u << 1)   /* Slave on Bus */
#define I2C_INT_TXFE              (1u << 3)   /* TX FIFO Empty */
#define I2C_INT_RXFF              (1u << 4)   /* RX FIFO Full */
#define I2C_INT_ERROR             (1u << 7)   /* Error */
#define I2C_INT_ALL               0x9Bu       /* All INTFLAG bits (DFP mask) */

/****************************************************************************
 * STATUS (offset 0x1A, R/W 16-bit)
 ****************************************************************************/

#define I2C_STATUS_BUSERR         (1u << 0)   /* Bus Error */
#define I2C_STATUS_ARBLOST        (1u << 1)   /* Arbitration Lost */
#define I2C_STATUS_RXNACK         (1u << 2)   /* Received NACK */

#define I2C_STATUS_BUSSTATE_SHIFT 4
#define I2C_STATUS_BUSSTATE_MASK  (0x3u << I2C_STATUS_BUSSTATE_SHIFT)
#define I2C_STATUS_BUSSTATE_UNKNOWN (0x0u << I2C_STATUS_BUSSTATE_SHIFT)
#define I2C_STATUS_BUSSTATE_IDLE  (0x1u << I2C_STATUS_BUSSTATE_SHIFT)
#define I2C_STATUS_BUSSTATE_OWNER (0x2u << I2C_STATUS_BUSSTATE_SHIFT)
#define I2C_STATUS_BUSSTATE_BUSY  (0x3u << I2C_STATUS_BUSSTATE_SHIFT)

#define I2C_STATUS_LOWTOUT        (1u << 6)
#define I2C_STATUS_CLKHOLD        (1u << 7)
#define I2C_STATUS_SEXTTOUT       (1u << 9)
#define I2C_STATUS_LENERR         (1u << 10)

/****************************************************************************
 * SYNCBUSY (offset 0x1C, RO 32-bit)
 ****************************************************************************/

#define I2C_SYNCBUSY_SWRST        (1u << 0)
#define I2C_SYNCBUSY_ENABLE       (1u << 1)
#define I2C_SYNCBUSY_SYSOP        (1u << 2)  /* CTRLB CMD or STATUS BUSSTATE */

/****************************************************************************
 * ADDR (offset 0x24, R/W 32-bit)
 ****************************************************************************/

#define I2C_ADDR_ADDR_SHIFT       1           /* 7-bit address in bits [7:1] */
#define I2C_ADDR_ADDR(a)          ((uint32_t)(a) << I2C_ADDR_ADDR_SHIFT)
#define I2C_ADDR_RD               (1u << 0)   /* 1=read, 0=write */

/****************************************************************************
 * BAUD (offset 0x0C, R/W 32-bit)
 *
 * For standard/fast mode:
 *   f_SCL = f_GCLK / (10 + 2*BAUD + f_GCLK * T_RISE)
 *   Simplified: BAUD = (f_GCLK / f_SCL - 10) / 2   (ignoring T_RISE)
 ****************************************************************************/

#define I2C_BAUD_BAUD_SHIFT       0
#define I2C_BAUD_BAUD_MASK        0xFFu
#define I2C_BAUD_BAUDLOW_SHIFT    8
#define I2C_BAUD_BAUDLOW_MASK     (0xFFu << 8)

#endif /* __ARCH_ARM_SRC_PIC32CZCA90_HARDWARE_SAM_SERCOM_I2C_H */
