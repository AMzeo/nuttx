/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_port.h
 *
 * PIC32CZ CA90 PORT (GPIO) interface
 ****************************************************************************/

#ifndef __ARCH_ARM_SRC_PIC32CZCA90_SAM_PORT_H
#define __ARCH_ARM_SRC_PIC32CZCA90_SAM_PORT_H

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include "hardware/sam_port.h"
#include "hardware/sam_pinmap.h"

/* Pin set type */

typedef uint32_t port_pinset_t;

#ifndef __ASSEMBLY__
#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

int sam_portconfig(port_pinset_t pinset);
void sam_portwrite(port_pinset_t pinset, bool value);
bool sam_portread(port_pinset_t pinset);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_PIC32CZCA90_SAM_PORT_H */
