/*
 * Copyright (C) 2020 user94729 (https://omegazero.org/) and contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind,
 * either expressed, implied, or statutory, including, without limitation, warranties that the Covered Software
 * is free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.
 */

#ifndef __ARCH_MMGR_H__
#define __ARCH_MMGR_H__

#include <kernel/mmgr.h>
#include <klibc/stdint.h>


#define MMGR_MEMTYPE_BIOS_USABLE 1
#define MMGR_MEMTYPE_BIOS_RESERVED 2
#define MMGR_MEMTYPE_BIOS_ACPI_RECLAIM 3
#define MMGR_MEMTYPE_BIOS_ACPI_NVS 4
#define MMGR_MEMTYPE_BIOS_BAD 5

#define MMGR_USABLE_MEMORY ((size_t) 0x0fffffff)

#define VMMGR_PAGE_PRESENT 1
#define VMMGR_PAGE_RW 2
#define VMMGR_PAGE_USER 4
#define VMMGR_PAGE_WRITE_THROUGH 8
#define VMMGR_PAGE_CACHE_DISABLE 16
#define VMMGR_PAGE_CACHED 16
#define VMMGR_PAGE_ACCESSED 32
#define VMMGR_PAGE_DIRTY 64
#define VMMGR_PAGE_SIZE_4MB 128
#define VMMGR_PAGE_GLOBAL 256

#define VMMGR_PAGE_SIZE 4096

#define VMMGR_PAGE_TABLE_SIZE(x) ((x) / 1024 + (x) / 1048576)

#define vmmgr_page_descriptor uint32_t


#pragma pack(push,1)
typedef struct mmgr_arch_mmap_entry{
	uint64_t addr;
	uint64_t size;
	uint32_t type;
	uint32_t reserved;
} mmgr_arch_mmap_entry;
#pragma pack(pop)


memtype_t mmgr_x86_get_memtype(uint32_t sysmemtype);

size_t vmmgr_get_address(size_t pdt_index, size_t pt_index);
void vmmgr_init_calc_paging_stats();


#endif /* __ARCH_MMGR_H__ */
