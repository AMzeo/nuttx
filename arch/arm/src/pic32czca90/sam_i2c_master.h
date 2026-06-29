/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_i2c_master.h
 *
 * PIC32CZ CA90 SERCOM I2C master — public interface.
 *
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_I2C_MASTER_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_I2C_MASTER_H

#include <nuttx/config.h>
#include <nuttx/i2c/i2c_master.h>

#ifndef __ASSEMBLY__

/****************************************************************************
 * Name: sam_i2cbus_initialize
 *
 * Description:
 *   Initialize the selected SERCOM port in I2C master mode.
 *
 * Input Parameters:
 *   port - SERCOM instance number (e.g. 5 for SERCOM5).
 *
 * Returned Value:
 *   Valid I2C master pointer on success; NULL on failure.
 *
 ****************************************************************************/

FAR struct i2c_master_s *sam_i2cbus_initialize(int port);

void sam_i2c_print_stats(void);

#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_I2C_MASTER_H */
