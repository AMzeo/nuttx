/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_spi.h
 *
 * PIC32CZ CA90 SERCOM SPI master — public interface.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_SPI_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_SPI_H

#include <nuttx/config.h>
#include <nuttx/spi/spi.h>

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_spibus_initialize
 *
 * Description:
 *   Initialize the selected SERCOM port in SPI master mode and return a
 *   reference to the NuttX SPI interface structure.
 *
 * Input Parameters:
 *   port - SERCOM instance number (0-9). Only instances configured as SPI
 *          in Kconfig (PIC32CZCA90_SERCOMn_ISSPI) are valid.
 *
 * Returned Value:
 *   Valid SPI device pointer on success; NULL on failure.
 *
 ****************************************************************************/

FAR struct spi_dev_s *sam_spibus_initialize(int port);

/****************************************************************************
 * Name: pic32czca90_spi8select / pic32czca90_spi8status
 *
 * Description:
 *   Board-level chip select and status for SERCOM8 SPI bus.
 *   Must be provided by the board (boards/.../src/spi.cpp).
 *
 ****************************************************************************/

void pic32czca90_spi8select(FAR struct spi_dev_s *dev, uint32_t devid,
                            bool selected);

uint8_t pic32czca90_spi8status(FAR struct spi_dev_s *dev, uint32_t devid);

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_SPI_H */
