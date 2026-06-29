/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * arch/arm/include/pic32czca90/chip.h
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

#ifndef __ARCH_ARM_INCLUDE_PIC32CZCA90_CHIP_H
#define __ARCH_ARM_INCLUDE_PIC32CZCA90_CHIP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

/* PIC32CZ CA90 - single variant for now (144-pin) */

/* Internal memory */

#define PIC32CZCA90_FLASH_SIZE      (8*1024*1024)  /* 8 MB */
#define PIC32CZCA90_SRAM_SIZE       (1024*1024)    /* 1 MB total */
#define PIC32CZCA90_DTCM_SIZE       (128*1024)     /* 128 KB DTCM */
#define PIC32CZCA90_ITCM_SIZE       (128*1024)     /* 128 KB ITCM */

/* Peripheral counts — from Harmony device_vectors.h (PIC32CZ8110CA80208 DFP) */

#define PIC32CZCA90_NSERCOM         10             /* SERCOM0-9 */
#define PIC32CZCA90_NTC             0              /* No TC (basic timer) — CA90 uses TCC for all timers */
#define PIC32CZCA90_NTCC            10             /* TCC0-TCC9 */
#define PIC32CZCA90_NCAN            6              /* CAN0-5 (MCAN) */
#define PIC32CZCA90_NDMACHAN        16             /* 16 DMA channels (DFP DMA_CHANNEL_NUMBER) */
#define PIC32CZCA90_NADC0CHAN       16             /* ADC0 channels */
#define PIC32CZCA90_NADC1CHAN       16             /* ADC1 channels */
#define PIC32CZCA90_NACMP           4              /* Analog comparators */
#define PIC32CZCA90_NDACCHAN        2              /* DAC channels */
#define PIC32CZCA90_NGCLKGEN        12             /* GCLK generators */
#define PIC32CZCA90_NGCLKCHAN       48             /* GCLK peripheral channels */
#define PIC32CZCA90_NEVTCHAN        32             /* Event system channels */
#define PIC32CZCA90_NCCL            4              /* CCL */

/* NVIC priority levels
 *
 * PIC32CZ CA90 Cortex-M7 implements 3 bits of interrupt priority
 * (bits 7:5 of the priority byte), giving 8 priority levels.
 */

#define NVIC_SYSH_PRIORITY_MIN      0xe0 /* All bits[7:5] set is minimum priority */
#define NVIC_SYSH_PRIORITY_DEFAULT  0x80 /* Midpoint is the default */
#define NVIC_SYSH_PRIORITY_MAX      0x00 /* Zero is maximum priority */
#define NVIC_SYSH_PRIORITY_STEP     0x20 /* Eight priority levels in steps 0x20 */

#endif /* __ARCH_ARM_INCLUDE_PIC32CZCA90_CHIP_H */
