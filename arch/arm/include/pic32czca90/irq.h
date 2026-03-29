/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/include/pic32czca90/irq.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/* This file should never be included directly but, rather,
 * only indirectly through nuttx/irq.h
 */

#ifndef __ARCH_ARM_INCLUDE_PIC32CZCA90_IRQ_H
#define __ARCH_ARM_INCLUDE_PIC32CZCA90_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/pic32czca90/chip.h>

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

/* Common Processor Exceptions (vectors 0-15) */

#define SAM_IRQ_RESERVED       (0)
#define SAM_IRQ_NMI            (2)
#define SAM_IRQ_HARDFAULT      (3)
#define SAM_IRQ_MEMFAULT       (4)
#define SAM_IRQ_BUSFAULT       (5)
#define SAM_IRQ_USAGEFAULT     (6)
#define SAM_IRQ_SVCALL         (11)
#define SAM_IRQ_DBGMONITOR     (12)
#define SAM_IRQ_PENDSV         (14)
#define SAM_IRQ_SYSTICK        (15)

/* External interrupts (vectors >= 16) */

#define SAM_IRQ_EXTINT         (16)

/* Include chip-specific IRQ definitions */

#include <arch/pic32czca90/pic32czca90_irq.h>

/* Total number of IRQ numbers */

#define NR_VECTORS             (SAM_IRQ_EXTINT + SAM_IRQ_NEXTINT)
#define NR_IRQS                NR_VECTORS

#endif /* __ARCH_ARM_INCLUDE_PIC32CZCA90_IRQ_H */
