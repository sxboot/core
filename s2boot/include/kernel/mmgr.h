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

#ifndef __KERNEL_MMGR_H__
#define __KERNEL_MMGR_H__

#include <klibc/stdint.h>
#include <arch/mmgr.h>


#define MMGR_BLOCK_SIZE 4096

#define MMGR_MEMTYPE_USABLE 0
#define MMGR_MEMTYPE_RESERVED 1
#define MMGR_MEMTYPE_ACPI_RECLAIM 2
#define MMGR_MEMTYPE_ACPI_NVS 3
#define MMGR_MEMTYPE_BAD 4
#define MMGR_MEMTYPE_BOOTLOADER 8
#define MMGR_MEMTYPE_BOOTLOADER_DATA 9
#define MMGR_MEMTYPE_PAGING 10
#define MMGR_MEMTYPE_OS 11

#define MMGR_MMAP_BLOCKS_PER_BYTE 2
#define MMGR_MMAP_BLOCK_WIDTH 4

typedef uint8_t memtype_t;

#pragma pack(push,1)
typedef struct mmap_entry{
	size_t addr;
	size_t size;
	size_t type;
} mmap_entry;
#pragma pack(pop)

status_t mmgr_init(size_t mmapStart, uint16_t mmapLength);
void mmgr_reg_map_reloc_ptr();
void mmgr_relocation(size_t oldAddr, size_t newAddr);
void mmgr_map_set(size_t index, memtype_t type);
void mmgr_map_clear(size_t index);
memtype_t mmgr_get_map_bit_type(size_t index);
bool mmgr_get_map_bit(size_t index);
size_t mmgr_get_first_free_block();
size_t mmgr_get_first_free_block_sequential(uint16_t length);
void mmgr_reserve_mem_region(size_t base, size_t size, memtype_t type);
void mmgr_free_mem_region(size_t base, size_t size);
void* mmgr_alloc_block();
void* mmgr_alloc_block_sequential(size_t size);
void mmgr_free_block(size_t addr);
void mmgr_free_block_sequential(size_t addr, size_t size);
bool mmgr_is_area_clear(size_t base, size_t size);
size_t mmgr_get_total_blocks();
size_t mmgr_get_used_blocks_alloc();
size_t mmgr_get_used_blocks_reserved();
size_t mmgr_get_used_blocks_paging();
size_t mmgr_get_used_blocks();
size_t mmgr_get_used_mem_kib();
uint8_t* mmgr_get_alloc_map();
void mmgr_get_system_map(mmgr_arch_mmap_entry** addrWrite, uint16_t* lengthWrite);
size_t mmgr_gen_mmap(mmap_entry* buf, size_t buflen, size_t* length);

void* mmgr_alloc_block_p();
void mmgr_free_block_p(size_t addr);

void* vmmgr_alloc_block();
void* vmmgr_alloc_block_sequential(size_t size);
void vmmgr_map_pages_req(size_t usedMem);
void vmmgr_free_block(size_t addr);
void vmmgr_free_block_sequential(size_t addr, size_t size);
bool vmmgr_is_address_accessible(size_t virt);
size_t vmmgr_get_mapped_pages();


status_t mmgr_init_arch();
void mmgr_reg_map_reloc_ptr_arch();
void mmgr_reserve_default_regions();
uint64_t mmgr_get_total_memory();
status_t mmgr_create_map();
void mmgr_arch_get_memmap_val(void* memmapEntry, size_t* addr, size_t* size, size_t* memtype);

status_t vmmgr_map_page(size_t phys, size_t virt);
status_t vmmgr_unmap_page(size_t virt);
size_t vmmgr_get_physical(size_t virt);
void* vmmgr_get_top_level_table();
void vmmgr_cleanup();
size_t vmmgr_gen_vmmap(mmap_entry* buf, size_t buflen, size_t* length);


#endif /* __KERNEL_MMGR_H__ */
