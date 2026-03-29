/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/src/pic32czca90/sam_port.c
 *
 * PIC32CZ CA90 PORT (GPIO) driver
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "arm_internal.h"
#include "sam_port.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uintptr_t sam_portbase(port_pinset_t pinset)
{
  uint32_t port = (pinset >> 28) & 0x3;

  switch (port)
    {
      case 0:  return SAM_PORTA_BASE;
      case 1:  return SAM_PORTB_BASE;
      case 2:  return SAM_PORTC_BASE;
      case 3:  return SAM_PORTD_BASE;
      default: return SAM_PORTA_BASE;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int sam_portconfig(port_pinset_t pinset)
{
  uintptr_t base;
  uint32_t pin;
  uint32_t func;
  uint8_t regval;

  base = sam_portbase(pinset);
  pin  = (pinset >> PORT_PIN_SHIFT) & 0x1f;
  func = (pinset >> PORT_FUNC_SHIFT) & 0xf;

  /* Configure output direction */

  if (pinset & PORT_FLAG_OUTPUT)
    {
      putreg32(1 << pin, base + SAM_PORT_DIRSET_OFFSET);
    }
  else
    {
      putreg32(1 << pin, base + SAM_PORT_DIRCLR_OFFSET);
    }

  /* Configure peripheral mux if enabled */

  if (pinset & PORT_FLAG_PMUXEN)
    {
      uint32_t pmux_offset = SAM_PORT_PMUX_OFFSET(pin >> 1);
      uint8_t pmuxval = getreg8(base + pmux_offset);

      if (pin & 1)
        {
          /* Odd pin */

          pmuxval &= ~PORT_PMUX_PMUXO_MASK;
          pmuxval |= (func << PORT_PMUX_PMUXO_SHIFT);
        }
      else
        {
          /* Even pin */

          pmuxval &= ~PORT_PMUX_PMUXE_MASK;
          pmuxval |= (func << PORT_PMUX_PMUXE_SHIFT);
        }

      putreg8(pmuxval, base + pmux_offset);
    }

  /* Configure PINCFG */

  regval = 0;

  if (pinset & PORT_FLAG_PMUXEN)
    {
      regval |= PORT_PINCFG_PMUXEN;
    }

  if (pinset & PORT_FLAG_INEN)
    {
      regval |= PORT_PINCFG_INEN;
    }

  if (pinset & PORT_FLAG_PULLEN)
    {
      regval |= PORT_PINCFG_PULLEN;
    }

  if (pinset & PORT_FLAG_DRVSTR)
    {
      regval |= PORT_PINCFG_DRVSTR;
    }

  putreg8(regval, base + SAM_PORT_PINCFG_OFFSET(pin));

  return 0;
}

void sam_portwrite(port_pinset_t pinset, bool value)
{
  uintptr_t base = sam_portbase(pinset);
  uint32_t pin   = (pinset >> PORT_PIN_SHIFT) & 0x1f;

  if (value)
    {
      putreg32(1 << pin, base + SAM_PORT_OUTSET_OFFSET);
    }
  else
    {
      putreg32(1 << pin, base + SAM_PORT_OUTCLR_OFFSET);
    }
}

bool sam_portread(port_pinset_t pinset)
{
  uintptr_t base = sam_portbase(pinset);
  uint32_t pin   = (pinset >> PORT_PIN_SHIFT) & 0x1f;

  return (getreg32(base + SAM_PORT_IN_OFFSET) & (1 << pin)) != 0;
}
