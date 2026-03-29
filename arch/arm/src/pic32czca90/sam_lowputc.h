/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_lowputc.h
 *
 * PIC32CZ CA90 low-level UART interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_LOWPUTC_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_LOWPUTC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "sam_config.h"

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_lowsetup
 *
 * Description:
 *   Called at the very beginning of _start.  Performs low level
 *   initialization.
 *
 ****************************************************************************/

void sam_lowsetup(void);

/****************************************************************************
 * Name: sam_usart_initialize
 *
 * Description:
 *   Set the configuration of a SERCOM for provided USART configuration.
 *   This configures the SERCOM as a USART, but does not configure USART
 *   interrupts or enable the USART.
 *
 ****************************************************************************/

#ifdef PIC32CZCA90_HAVE_USART
struct sam_usart_config_s;
int sam_usart_initialize(const struct sam_usart_config_s * const config);
#endif

/****************************************************************************
 * Name: sam_usart_reset
 *
 * Description:
 *   Reset the USART SERCOM.  This restores all SERCOM register to the
 *   initial state and disables the SERCOM.
 *
 ****************************************************************************/

#ifdef PIC32CZCA90_HAVE_USART
struct sam_usart_config_s;
void sam_usart_reset(const struct sam_usart_config_s * const config);
#endif

/****************************************************************************
 * Name: sam_lowputc
 *
 * Description:
 *   Output one character to the USART using a simple polling method.
 *
 ****************************************************************************/

#ifdef HAVE_SERIAL_CONSOLE
void sam_lowputc(uint32_t ch);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_LOWPUTC_H */
