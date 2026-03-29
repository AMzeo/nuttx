/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_usart.h
 *
 * PIC32CZ CA90 USART configuration interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_USART_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_USART_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>

#include <arch/chip/chip.h>

#include "arm_internal.h"
#include "hardware/sam_usart.h"

#include "sam_config.h"
#include "sam_port.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Pick the console USART configuration */

#if defined(CONFIG_USART0_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart0config)
#elif defined(CONFIG_USART1_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart1config)
#elif defined(CONFIG_USART2_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart2config)
#elif defined(CONFIG_USART3_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart3config)
#elif defined(CONFIG_USART4_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart4config)
#elif defined(CONFIG_USART5_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart5config)
#elif defined(CONFIG_USART6_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart6config)
#elif defined(CONFIG_USART7_SERIAL_CONSOLE)
#  define g_consoleconfig (g_usart7config)
#else
#  undef  g_consoleconfig
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This structure describes the static configuration of a USART */

struct sam_usart_config_s
{
  uint8_t sercom;         /* Identifies the SERCOM peripheral */
  uint8_t parity;         /* 0=none, 1=odd, 2=even */
  uint8_t bits;           /* Number of bits (5-9) */
  uint8_t txirq;          /* Tx SERCOM IRQ number */
  uint8_t rxirq;          /* Rx SERCOM IRQ number */
  uint8_t coregen;        /* Core GCLK generator */
  uint8_t slowgen;        /* Slow GCLK generator */
  uint8_t stopbits2 : 1;  /* True: Configure with 2 stop bits instead of 1 */
  uint8_t corelock  : 1;  /* True: Lock the CORE clock */
  uint32_t baud;          /* Configured baud */
  port_pinset_t pad0;     /* Pin configuration for PAD0 */
  port_pinset_t pad1;     /* Pin configuration for PAD1 */
  port_pinset_t pad2;     /* Pin configuration for PAD2 */
  port_pinset_t pad3;     /* Pin configuration for PAD3 */
  uint32_t muxconfig;     /* Pad multiplexing configuration */
  uint32_t frequency;     /* Source clock frequency */
  uintptr_t base;         /* SERCOM base address */
};

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: usart_syncbusy
 *
 * Description:
 *   Return true if the SERCOM USART reports that it is synchronizing.
 *
 ****************************************************************************/

#ifdef PIC32CZCA90_HAVE_USART
static inline
bool usart_syncbusy(const struct sam_usart_config_s * const config)
{
  return (getreg32(config->base + SAM_USART_SYNCBUSY_OFFSET) != 0);
}
#endif

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

#ifdef PIC32CZCA90_HAVE_USART0
EXTERN const struct sam_usart_config_s g_usart0config;
#endif

#ifdef PIC32CZCA90_HAVE_USART1
EXTERN const struct sam_usart_config_s g_usart1config;
#endif

#ifdef PIC32CZCA90_HAVE_USART2
EXTERN const struct sam_usart_config_s g_usart2config;
#endif

#ifdef PIC32CZCA90_HAVE_USART3
EXTERN const struct sam_usart_config_s g_usart3config;
#endif

#ifdef PIC32CZCA90_HAVE_USART4
EXTERN const struct sam_usart_config_s g_usart4config;
#endif

#ifdef PIC32CZCA90_HAVE_USART5
EXTERN const struct sam_usart_config_s g_usart5config;
#endif

#ifdef PIC32CZCA90_HAVE_USART6
EXTERN const struct sam_usart_config_s g_usart6config;
#endif

#ifdef PIC32CZCA90_HAVE_USART7
EXTERN const struct sam_usart_config_s g_usart7config;
#endif

EXTERN const struct sam_usart_config_s *g_usartconfig[PIC32CZCA90_NSERCOM];

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_USART_H */
