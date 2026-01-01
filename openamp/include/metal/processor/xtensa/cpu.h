/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	cpu.h
 * @brief	CPU specific primatives
 */

#ifndef __METAL_XTENSA_CPU__H__
#define __METAL_XTENSA_CPU__H__

#include "delay.h"

#define metal_cpu_yield()	mdelay(10);

#endif /* __METAL_XTENSA_CPU__H__ */
