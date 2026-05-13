/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_SQI_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_SQI_H

#ifdef CONFIG_PIC32CZCA90_SQI1

#include <stdint.h>
#include <stddef.h>
#include <nuttx/spi/spi.h>

FAR struct spi_dev_s *sam_sqibus_initialize(int bus);

/* Flash command+read in one linked TX→RX BD chain (CS held throughout).
 * cmd/cmdlen: command bytes to send on IO0.
 * data/datalen: buffer to receive datalen bytes from IO1.
 * Returns 0 on success, -EIO on DMA timeout. */

int sam_sqi_flash_cmd_read(FAR struct spi_dev_s *dev,
                            FAR const uint8_t *cmd, size_t cmdlen,
                            FAR uint8_t *data, size_t datalen);

/* Enable XIP mode for SST26 reads.  After this call, flash data is
 * memory-mapped at SAM_SQI1_XIP_BASE (0x90000000). */

void sam_sqi_xip_enable(void);
void sam_sqi_enter_xip(void);

/* Send a flash command via standalone TX BD with LIFM (CS deasserts at end).
 * Switches DMA→execute→XIP automatically.
 * Used for WREN, Sector Erase, Page Program. */

int sam_sqi_flash_cmd_write(FAR const uint8_t *txbuf, size_t txlen);

/* Send WREN + command in one session (one SWRST, no CS glitch between).
 * Use for erase (WREN+SE) and page program (WREN+PP). */

int sam_sqi_flash_wren_cmd(FAR const uint8_t *cmd, size_t cmdlen);

/* Read SST26 Status Register.  Returns status byte or -1 on error. */

int sam_sqi_flash_rdsr(void);

#endif /* CONFIG_PIC32CZCA90_SQI1 */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_SQI_H */
