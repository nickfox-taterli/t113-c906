/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/io.c
 * @brief	Generic libmetal io operations
 */

#include <metal/io.h>

void metal_sys_io_mem_map(struct metal_io_region *io)
{
	/* 1:1 mapping on this platform, nothing to do */
	(void)io;
}
