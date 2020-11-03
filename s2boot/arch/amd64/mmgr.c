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

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/errc.h>
#include <kernel/mmgr.h>
#include <arch/mmgr.h>

extern mmgr_arch_mmap_entry* mmgr_memoryMap;
extern uint16_t mmgr_memoryMapLength;

extern uint8_t* mmgr_alloc_map;
extern size_t mmgr_alloc_map_size;
extern size_t mmgr_total_blocks;
extern size_t mmgr_used_blocks_reserved;
extern size_t mmgr_used_blocks_alloc;
extern size_t mmgr_used_blocks_paging;

extern size_t vmmgr_mapped_pages;

extern size_t vmmgr_membase;

extern size_t vmmgr_mappedMemory;

static vmmgr_page_descriptor* pml4t = 0;

status_t mmgr_init_arch(){
	asm("mov %%cr3, %0" : "=r" (pml4t));

	mmgr_reserve_mem_region(0, mmgr_total_blocks * MMGR_BLOCK_SIZE, MMGR_MEMTYPE_RESERVED);
	for(int i = 0; i < mmgr_memoryMapLength; i++){
		memtype_t memtype = mmgr_x86_get_memtype(mmgr_memoryMap[i].type);
		if(memtype != MMGR_MEMTYPE_USABLE){
			mmgr_reserve_mem_region(mmgr_memoryMap[i].addr, mmgr_memoryMap[i].size, memtype);
		}else{
			mmgr_free_mem_region(mmgr_memoryMap[i].addr, mmgr_memoryMap[i].size);
		}
	}
	vmmgr_init_calc_paging_stats();
	return TSX_SUCCESS;
}

void mmgr_reg_map_reloc_ptr_arch(){
	reloc_ptr((void**) &pml4t);
}

void mmgr_reserve_default_regions(){
	mmgr_reserve_mem_region(0, 0x500, MMGR_MEMTYPE_RESERVED);
	mmgr_reserve_mem_region(0x7c00, 0x200, MMGR_MEMTYPE_RESERVED);
	mmgr_reserve_mem_region(0x80000, 0x80000, MMGR_MEMTYPE_RESERVED);
}

uint64_t mmgr_get_total_memory(){
	size_t max = 0;
	for(int i = 0; i < mmgr_memoryMapLength; i++){
		if(mmgr_x86_get_memtype(mmgr_memoryMap[i].type) == MMGR_MEMTYPE_USABLE && mmgr_memoryMap[i].addr + mmgr_memoryMap[i].size > max){
			max = mmgr_memoryMap[i].addr + mmgr_memoryMap[i].size;
		}
	}
	return max;
}

status_t mmgr_create_map(){
	status_t status = 0;
	uint64_t totalMem = mmgr_get_total_memory();
	size_t mapSize = MIN(totalMem, MMGR_USABLE_MEMORY + 1) / MMGR_BLOCK_SIZE / MMGR_MMAP_BLOCKS_PER_BYTE;
	uint8_t* newMap = NULL;
	for(int i = 0; i < mmgr_memoryMapLength; i++){
		if(mmgr_x86_get_memtype(mmgr_memoryMap[i].type) == MMGR_MEMTYPE_USABLE && mmgr_memoryMap[i].size >= mapSize && mmgr_memoryMap[i].addr >= 0x100000){
			newMap = (uint8_t*) mmgr_memoryMap[i].addr;
			break;
		}
	}
	if(newMap == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	// map is max 0x7FFFF bytes large due to 4GiB limitation, more (or when map is not at 0x100000) could cause a page fault (because s1boot only mapped 2MiB)!!
	memset(newMap, 0, mapSize);
	mmgr_alloc_map = newMap;
	mmgr_alloc_map_size = mapSize;
	mmgr_total_blocks = mapSize * MMGR_MMAP_BLOCKS_PER_BYTE;
	_end:
	return status;
}

void mmgr_arch_get_memmap_val(void* memmapEntry, size_t* addr, size_t* size, size_t* memtype){
	mmgr_arch_mmap_entry* entry = memmapEntry;
	*addr = entry->addr;
	*size = entry->size;
	*memtype = mmgr_x86_get_memtype(entry->type);
}

static memtype_t uefiMemTypes[15] = {
	MMGR_MEMTYPE_RESERVED, // EfiReservedMemoryType
	MMGR_MEMTYPE_BOOTLOADER, // EfiLoaderCode
	MMGR_MEMTYPE_BOOTLOADER_DATA, // EfiLoaderData
	MMGR_MEMTYPE_UEFI_BOOT, // EfiBootServicesCode
	MMGR_MEMTYPE_UEFI_BOOT, // EfiBootServicesData
	MMGR_MEMTYPE_UEFI_RUNTIME, // EfiRuntimeServicesCode
	MMGR_MEMTYPE_UEFI_RUNTIME, // EfiRuntimeServicesData
	MMGR_MEMTYPE_USABLE, // EfiConventionalMemory
	MMGR_MEMTYPE_BAD, // EfiUnusableMemory
	MMGR_MEMTYPE_ACPI_RECLAIM, // EfiACPIReclaimMemory
	MMGR_MEMTYPE_ACPI_NVS, // EfiACPIMemoryNVS
	MMGR_MEMTYPE_RESERVED, // EfiMemoryMappedIO
	MMGR_MEMTYPE_RESERVED, // EfiMemoryMappedIOPortSpace
	MMGR_MEMTYPE_RESERVED, // EfiPalCode
	MMGR_MEMTYPE_RESERVED // EfiPersistentMemory
};

memtype_t mmgr_x86_get_memtype(uint32_t sysmemtype){
	if(kernel_get_s1data()->bootFlags & 0x2){ // UEFI
		if(sysmemtype < 15)
			return uefiMemTypes[sysmemtype];
		else
			return MMGR_MEMTYPE_RESERVED;
	}else{
		return sysmemtype - 1; // bios memtype offset by 1 from s2boot memtype
	}
}



status_t vmmgr_map_page(size_t phys, size_t virt){
	if(pml4t == 0)
		return 34;

	if(vmmgr_get_physical(virt))
		vmmgr_unmap_page(virt);

	uint16_t flags = VMMGR_PAGE_PRESENT | VMMGR_PAGE_RW;

	//1111111111111111111111111111111111111111111111111111111111111111
	//|reserved      ||os pl4t||in pdpt||in pd  ||in pt  ||in page   |
	uint16_t pl4t_index = (virt >> 39) & 0x1ff;
	uint16_t pdpt_index = (virt >> 30) & 0x1ff;
	uint16_t pd_index = (virt >> 21) & 0x1ff;
	uint16_t pt_index = (virt >> 12) & 0x1ff;

	if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) == 0){
		size_t pdpt_addr = (size_t) mmgr_alloc_block_p();
		if(pdpt_addr == 0)
			return 28;
		pml4t[pl4t_index] = (pdpt_addr & 0x000ffffffffff000) | flags;
		memset((void*) (pdpt_addr + vmmgr_membase), 0, 4096);
	}

	vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) == 0){
		size_t pd_addr = (size_t) mmgr_alloc_block_p();
		if(pd_addr == 0)
			return 28;
		pdpt[pdpt_index] = (pd_addr & 0x000ffffffffff000) | flags;
		memset((void*) (pd_addr + vmmgr_membase), 0, 4096);
	}

	vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdt[pd_index] & VMMGR_PAGE_PRESENT) == 0){
		size_t pt_addr = (size_t) mmgr_alloc_block_p();
		if(pt_addr == 0)
			return 28;
		pdt[pd_index] = (pt_addr & 0x000ffffffffff000) | flags;
		memset((void*) (pt_addr + vmmgr_membase), 0, 4096);
	}

	vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pd_index] & 0x000ffffffffff000) + vmmgr_membase);
	if(!(pt[pt_index] & VMMGR_PAGE_PRESENT)){
		vmmgr_mapped_pages++;
	}
	pt[pt_index] = (phys & 0x000ffffffffff000) | flags;
	return 0;
}

status_t vmmgr_unmap_page(size_t virt){
	if(pml4t == 0)
		return 34;

	uint16_t pl4t_index = (virt >> 39) & 0x1ff;
	uint16_t pdpt_index = (virt >> 30) & 0x1ff;
	uint16_t pd_index = (virt >> 21) & 0x1ff;
	uint16_t pt_index = (virt >> 12) & 0x1ff;

	if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0; // already not mapped by pml4t

	vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0;

	vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdt[pd_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0;

	vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pd_index] & 0x000ffffffffff000) + vmmgr_membase);
	if(pt[pt_index] & VMMGR_PAGE_PRESENT){
		vmmgr_mapped_pages--;
	}
	pt[pt_index] = 0;
	asm("mov %0, %%rdi; invlpg (%%rdi)" : : "r" (virt) : "%rdi");
	return 0;
}

size_t vmmgr_get_physical(size_t virt){
	if(pml4t == 0)
		return 0;

	uint16_t pl4t_index = (virt >> 39) & 0x1ff;
	uint16_t pdpt_index = (virt >> 30) & 0x1ff;
	uint16_t pd_index = (virt >> 21) & 0x1ff;
	uint16_t pt_index = (virt >> 12) & 0x1ff;
	uint16_t p_index = (virt) & 0xfff;

	if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0; // already not mapped by pml4t

	vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0;

	vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pdt[pd_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0;

	vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pd_index] & 0x000ffffffffff000) + vmmgr_membase);
	if((pt[pt_index] & VMMGR_PAGE_PRESENT) == 0)
		return 0;

	return (size_t) (pt[pt_index] & 0x000ffffffffff000 + vmmgr_membase) + p_index;
}

void* vmmgr_get_top_level_table(){
	return pml4t;
}

void vmmgr_cleanup(){
	for(int pl4t_index = 0; pl4t_index < 512; pl4t_index++){
		if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) && (pml4t[pl4t_index] & 0x000ffffffffff000) != 0){
			vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
			bool pdptdel = TRUE;
			for(int pdpt_index = 0; pdpt_index < 512; pdpt_index++){
				if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) && (pdpt[pdpt_index] & 0x000ffffffffff000) != 0){
					vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
					bool pdtdel = TRUE;
					for(int pdt_index = 0; pdt_index < 512; pdt_index++){
						if((pdt[pdt_index] & VMMGR_PAGE_PRESENT) && (pdt[pdt_index] & 0x000ffffffffff000) != 0){
							vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pdt_index] & 0x000ffffffffff000) + vmmgr_membase);
							bool ptdel = TRUE;
							for(int pt_index = 0; pt_index < 512; pt_index++){
								if(pt[pt_index] & VMMGR_PAGE_PRESENT){
									ptdel = FALSE;
								}
							}
							if(ptdel){
								mmgr_free_block_p(pdt[pdt_index] & 0x000ffffffffff000);
								pdt[pdt_index] = 0;
							}else{
								pdtdel = FALSE;
							}
						}
					}
					if(pdtdel){
						mmgr_free_block_p(pdpt[pdpt_index] & 0x000ffffffffff000);
						pdpt[pdpt_index] = 0;
					}else{
						pdptdel = FALSE;
					}
				}
			}
			if(pdptdel){
				mmgr_free_block_p(pml4t[pl4t_index] & 0x000ffffffffff000);
				pml4t[pl4t_index] = 0;
			}
		}
	}
}

size_t vmmgr_gen_vmmap(mmap_entry* buf, size_t buflen, size_t* length){
	memset(buf, 0, buflen);
	size_t cindex = 0;
	size_t blength = 0;
	size_t lastBase = 0;
	size_t lastPresent = 0;
	bool uinit = FALSE;
	for(int pl4t_index = 0; pl4t_index < 512; pl4t_index++){
		if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) && (pml4t[pl4t_index] & 0x000ffffffffff000) != 0){
			vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
			for(int pdpt_index = 0; pdpt_index < 512; pdpt_index++){
				if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) && (pdpt[pdpt_index] & 0x000ffffffffff000) != 0){
					vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
					for(int pdt_index = 0; pdt_index < 512; pdt_index++){
						if((pdt[pdt_index] & VMMGR_PAGE_PRESENT) && (pdt[pdt_index] & 0x000ffffffffff000) != 0){
							vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pdt_index] & 0x000ffffffffff000) + vmmgr_membase);
							for(int pt_index = 0; pt_index < 512; pt_index++){
								if(pt[pt_index] & VMMGR_PAGE_PRESENT){
									size_t lpr = vmmgr_get_address(pl4t_index, pdpt_index, pdt_index, pt_index);
									if(!uinit){
										lastBase = lpr;
										lastPresent = lpr;
										uinit = TRUE;
									}
									if(lpr - lastPresent > 0x1000){
										if(sizeof(mmap_entry) * (cindex + 1) <= buflen){
											buf[cindex].addr = lastBase;
											buf[cindex].size = lastPresent + 0x1000 - lastBase;
											blength++;
										}
										cindex++;
										lastBase = lpr;
									}
									lastPresent = lpr;
								}
							}
						}
					}
				}
			}
		}
	}
	if(sizeof(mmap_entry) * (cindex + 1) <= buflen){
		buf[cindex].addr = lastBase;
		buf[cindex].size = lastPresent + 0x1000 - lastBase;
		blength++;
	}
	cindex++;
	if(length)
		*length = blength;
	return cindex;
}

size_t vmmgr_get_address(size_t pml4t_index, size_t pdpt_index, size_t pdt_index, size_t pt_index){
	/*
	//1111111111111111111111111111111111111111111111111111111111111111
	//|reserved      ||os pl4t||in pdpt||in pd  ||in pt  ||in page   |
	*/
	size_t addr = 0;
	addr |= (pt_index & 0x1ff) << 12;
	addr |= (pdt_index & 0x1ff) << 21;
	addr |= (pdpt_index & 0x1ff) << 30;
	addr |= (pml4t_index & 0x1ff) << 39;
	if(addr & 0x800000000000){
		addr |= 0xffff000000000000;
	}
	return addr;
}

void vmmgr_init_calc_paging_stats(){
	mmgr_used_blocks_paging++; // pl4t
	mmgr_reserve_mem_region((size_t) pml4t, 0x1000, MMGR_MEMTYPE_PAGING);
	for(int pl4t_index = 0; pl4t_index < 512; pl4t_index++){
		if((pml4t[pl4t_index] & VMMGR_PAGE_PRESENT) && (pml4t[pl4t_index] & 0x000ffffffffff000) != 0){
			vmmgr_page_descriptor* pdpt = (vmmgr_page_descriptor*) ((pml4t[pl4t_index] & 0x000ffffffffff000) + vmmgr_membase);
			mmgr_used_blocks_paging++;
			mmgr_reserve_mem_region((size_t) pdpt, 0x1000, MMGR_MEMTYPE_PAGING);
			for(int pdpt_index = 0; pdpt_index < 512; pdpt_index++){
				if((pdpt[pdpt_index] & VMMGR_PAGE_PRESENT) && (pdpt[pdpt_index] & 0x000ffffffffff000) != 0){
					vmmgr_page_descriptor* pdt = (vmmgr_page_descriptor*) ((pdpt[pdpt_index] & 0x000ffffffffff000) + vmmgr_membase);
					mmgr_used_blocks_paging++;
					mmgr_reserve_mem_region((size_t) pdt, 0x1000, MMGR_MEMTYPE_PAGING);
					for(int pdt_index = 0; pdt_index < 512; pdt_index++){
						if((pdt[pdt_index] & VMMGR_PAGE_PRESENT) && (pdt[pdt_index] & 0x000ffffffffff000) != 0){
							vmmgr_page_descriptor* pt = (vmmgr_page_descriptor*) ((pdt[pdt_index] & 0x000ffffffffff000) + vmmgr_membase);
							mmgr_used_blocks_paging++;
							mmgr_reserve_mem_region((size_t) pt, 0x1000, MMGR_MEMTYPE_PAGING);
							for(int pt_index = 0; pt_index < 512; pt_index++){
								if(pt[pt_index] & VMMGR_PAGE_PRESENT){
									vmmgr_mapped_pages++;
									size_t address = vmmgr_get_address(pl4t_index, pdpt_index, pdt_index, pt_index);
									if(address > vmmgr_mappedMemory)
										vmmgr_mappedMemory = address + 0x1000;
								}
							}
						}
					}
				}
			}
		}
	}
}

