/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_usart.c
 *
 * PIC32CZ CA90 USART configuration data
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <assert.h>

#include <arch/irq.h>
#include <arch/pic32czca90/chip.h>

#include "hardware/sam_memorymap.h"
#include "hardware/sam_pinmap.h"
#include "sam_gclk.h"
#include "sam_sercom.h"
#include "sam_usart.h"

#include <arch/board/board.h>

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef PIC32CZCA90_HAVE_USART0
const struct sam_usart_config_s g_usart0config =
{
  .sercom    = 0,
  .parity    = CONFIG_USART0_PARITY,
  .bits      = CONFIG_USART0_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM0,
  .rxirq     = BOARD_RXIRQ_SERCOM0,
  .coregen   = BOARD_SERCOM0_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART0_2STOP,
  .corelock  = BOARD_SERCOM0_CORELOCK,
  .baud      = CONFIG_USART0_BAUD,
  .pad0      = BOARD_SERCOM0_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM0_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM0_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM0_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM0_MUXCONFIG,
  .frequency = BOARD_SERCOM0_FREQUENCY,
  .base      = SAM_SERCOM0_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART1
const struct sam_usart_config_s g_usart1config =
{
  .sercom    = 1,
  .parity    = CONFIG_USART1_PARITY,
  .bits      = CONFIG_USART1_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM1,
  .rxirq     = BOARD_RXIRQ_SERCOM1,
  .coregen   = BOARD_SERCOM1_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART1_2STOP,
  .corelock  = BOARD_SERCOM1_CORELOCK,
  .baud      = CONFIG_USART1_BAUD,
  .pad0      = BOARD_SERCOM1_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM1_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM1_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM1_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM1_MUXCONFIG,
  .frequency = BOARD_SERCOM1_FREQUENCY,
  .base      = SAM_SERCOM1_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART2
const struct sam_usart_config_s g_usart2config =
{
  .sercom    = 2,
  .parity    = CONFIG_USART2_PARITY,
  .bits      = CONFIG_USART2_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM2,
  .rxirq     = BOARD_RXIRQ_SERCOM2,
  .coregen   = BOARD_SERCOM2_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART2_2STOP,
  .corelock  = BOARD_SERCOM2_CORELOCK,
  .baud      = CONFIG_USART2_BAUD,
  .pad0      = BOARD_SERCOM2_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM2_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM2_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM2_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM2_MUXCONFIG,
  .frequency = BOARD_SERCOM2_FREQUENCY,
  .base      = SAM_SERCOM2_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART3
const struct sam_usart_config_s g_usart3config =
{
  .sercom    = 3,
  .parity    = CONFIG_USART3_PARITY,
  .bits      = CONFIG_USART3_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM3,
  .rxirq     = BOARD_RXIRQ_SERCOM3,
  .coregen   = BOARD_SERCOM3_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART3_2STOP,
  .corelock  = BOARD_SERCOM3_CORELOCK,
  .baud      = CONFIG_USART3_BAUD,
  .pad0      = BOARD_SERCOM3_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM3_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM3_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM3_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM3_MUXCONFIG,
  .frequency = BOARD_SERCOM3_FREQUENCY,
  .base      = SAM_SERCOM3_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART4
const struct sam_usart_config_s g_usart4config =
{
  .sercom    = 4,
  .parity    = CONFIG_USART4_PARITY,
  .bits      = CONFIG_USART4_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM4,
  .rxirq     = BOARD_RXIRQ_SERCOM4,
  .coregen   = BOARD_SERCOM4_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART4_2STOP,
  .corelock  = BOARD_SERCOM4_CORELOCK,
  .baud      = CONFIG_USART4_BAUD,
  .pad0      = BOARD_SERCOM4_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM4_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM4_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM4_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM4_MUXCONFIG,
  .frequency = BOARD_SERCOM4_FREQUENCY,
  .base      = SAM_SERCOM4_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART5
const struct sam_usart_config_s g_usart5config =
{
  .sercom    = 5,
  .parity    = CONFIG_USART5_PARITY,
  .bits      = CONFIG_USART5_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM5,
  .rxirq     = BOARD_RXIRQ_SERCOM5,
  .coregen   = BOARD_SERCOM5_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART5_2STOP,
  .corelock  = BOARD_SERCOM5_CORELOCK,
  .baud      = CONFIG_USART5_BAUD,
  .pad0      = BOARD_SERCOM5_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM5_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM5_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM5_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM5_MUXCONFIG,
  .frequency = BOARD_SERCOM5_FREQUENCY,
  .base      = SAM_SERCOM5_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART6
const struct sam_usart_config_s g_usart6config =
{
  .sercom    = 6,
  .parity    = CONFIG_USART6_PARITY,
  .bits      = CONFIG_USART6_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM6,
  .rxirq     = BOARD_RXIRQ_SERCOM6,
  .coregen   = BOARD_SERCOM6_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART6_2STOP,
  .corelock  = BOARD_SERCOM6_CORELOCK,
  .baud      = CONFIG_USART6_BAUD,
  .pad0      = BOARD_SERCOM6_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM6_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM6_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM6_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM6_MUXCONFIG,
  .frequency = BOARD_SERCOM6_FREQUENCY,
  .base      = SAM_SERCOM6_BASE,
};
#endif

#ifdef PIC32CZCA90_HAVE_USART7
const struct sam_usart_config_s g_usart7config =
{
  .sercom    = 7,
  .parity    = CONFIG_USART7_PARITY,
  .bits      = CONFIG_USART7_BITS,
  .txirq     = BOARD_TXIRQ_SERCOM7,
  .rxirq     = BOARD_RXIRQ_SERCOM7,
  .coregen   = BOARD_SERCOM7_COREGEN,
  .slowgen   = BOARD_SERCOM_SLOWGEN,
  .stopbits2 = CONFIG_USART7_2STOP,
  .corelock  = BOARD_SERCOM7_CORELOCK,
  .baud      = CONFIG_USART7_BAUD,
  .pad0      = BOARD_SERCOM7_PINMAP_PAD0,
  .pad1      = BOARD_SERCOM7_PINMAP_PAD1,
  .pad2      = BOARD_SERCOM7_PINMAP_PAD2,
  .pad3      = BOARD_SERCOM7_PINMAP_PAD3,
  .muxconfig = BOARD_SERCOM7_MUXCONFIG,
  .frequency = BOARD_SERCOM7_FREQUENCY,
  .base      = SAM_SERCOM7_BASE,
};
#endif

const struct sam_usart_config_s *g_usartconfig[PIC32CZCA90_NSERCOM] =
{
#if PIC32CZCA90_NSERCOM > 0
#ifdef PIC32CZCA90_HAVE_USART0
  &g_usart0config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 1
#ifdef PIC32CZCA90_HAVE_USART1
  &g_usart1config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 2
#ifdef PIC32CZCA90_HAVE_USART2
  &g_usart2config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 3
#ifdef PIC32CZCA90_HAVE_USART3
  &g_usart3config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 4
#ifdef PIC32CZCA90_HAVE_USART4
  &g_usart4config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 5
#ifdef PIC32CZCA90_HAVE_USART5
  &g_usart5config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 6
#ifdef PIC32CZCA90_HAVE_USART6
  &g_usart6config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif

#if PIC32CZCA90_NSERCOM > 7
#ifdef PIC32CZCA90_HAVE_USART7
  &g_usart7config,
#else
  (const struct sam_usart_config_s *)0,
#endif
#endif
};
