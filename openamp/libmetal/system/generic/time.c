/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/time.c
 * @brief	Generic libmetal time handling.
 */

#include <metal/time.h>
#include "FreeRTOS.h"
#include "task.h"

unsigned long long metal_get_timestamp(void)
{
	/* Return milliseconds since boot based on FreeRTOS ticks */
	return (unsigned long long)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}
