/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <common/debug.h>

#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/rmmd_svc.h>

static uintptr_t top_mem = RMM_LIMIT;

#define MEM_SLOT_ZEROED	0
#define NR_MEM_SLOTS	4

struct mem_slot {
	uintptr_t addr;
	size_t size;
} mem_slots[NR_MEM_SLOTS] = {};

static uintptr_t allocate_memory(size_t size, unsigned long alignment)
{
	uint64_t align_mask = alignment - 1;
	uintptr_t addr;

	addr = (top_mem - size) & ~align_mask;
	if (addr < RMM_PAYLOAD_LIMIT) {
		return 0;
	}
	top_mem = addr;

	return addr;
}

int rmmd_allocate_memory(size_t size, uint64_t *arg)
{
	uint64_t alignment = 1UL << ((*arg >> 24) & 0xff);
	int id = *arg & 0xffff;
	uintptr_t addr;
	int ret = 0;

	INFO("%s(%ld, 0x%lx);\n", __func__, size, *arg);

	if (id >= NR_MEM_SLOTS) {
		return E_RMM_INVAL;
	}

	if (id == MEM_SLOT_ZEROED) {
		addr = allocate_memory(size, alignment);
		if (addr == 0) {
			return E_RMM_NOMEM;
		}

		ret = 1;
	} else {
		if (mem_slots[id].size == 0) {
			addr = allocate_memory(size, alignment);
			if (addr == 0) {
				return E_RMM_NOMEM;
			}
			mem_slots[id].addr = addr;
			mem_slots[id].size = size;
			ret = 1;
		} else {
			if (mem_slots[id].size < size)
				return E_RMM_NOMEM;
			addr = mem_slots[id].addr;
		}
	}

	*arg = addr;

	return ret;
}
