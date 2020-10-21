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
/*
 * kernel.c - The main file of the second stage of the sxboot boot loader (s2boot).
 * This file contains the entry point of s2boot.
 */

#include <uefi.h>
#include <sUboot.h>


size_t suboot_get_address(size_t pl4t_index, size_t pdpt_index, size_t pdt_index, size_t pt_index){
	size_t addr = 0;
	addr |= (pt_index & 0x1ff) << 12;
	addr |= (pdt_index & 0x1ff) << 21;
	addr |= (pdpt_index & 0x1ff) << 30;
	addr |= (pl4t_index & 0x1ff) << 39;
	if(addr & 0x800000000000){
		addr |= 0xffff000000000000;
	}
	return addr;
}

size_t suboot_get_highest_accessed_address(){
	uint64_t* pml4t = 0;
	asm("mov %%cr3, %0" : "=r" (pml4t));
	size_t highest = 0;
	for(int pl4t_index = 511; pl4t_index >= 0; pl4t_index--){
		if((pml4t[pl4t_index] & 0x21) == 0x21){ // accessed + present
			uint64_t* pdpt = (uint64_t*) (pml4t[pl4t_index] & 0xfffffffffffff000);
			for(int pdpt_index = 511; pdpt_index >= 0; pdpt_index--){
				if((pdpt[pdpt_index] & 0xa1) == 0xa1){ // large page + accessed + present
					highest = suboot_get_address(pl4t_index, pdpt_index, 0, 0) + 0x40000000;
				}else if((pdpt[pdpt_index] & 0x21) == 0x21){ // accessed + present
					uint64_t* pdt = (uint64_t*) (pdpt[pdpt_index] & 0xfffffffffffff000);
					for(int pdt_index = 511; pdt_index >= 0; pdt_index--){
						if((pdt[pdt_index] & 0xa1) == 0xa1){ // large page + accessed + present
							highest = suboot_get_address(pl4t_index, pdpt_index, pdt_index, 0) + 0x200000;
						}else if((pdt[pdt_index] & 0x21) == 0x21){ // accessed + present
							uint64_t* pt = (uint64_t*) (pdt[pdt_index] & 0xfffffffffffff000);
							for(int pt_index = 511; pt_index >= 0; pt_index--){
								if((pt[pt_index] & 0x21) /* accessed and present */ == 0x21){
									highest = suboot_get_address(pl4t_index, pdpt_index, pdt_index, pt_index) + 0x1000;
									break;
								}
							}
						}
						if(highest)
							break;
					}
				}
				if(highest)
					break;
			}
		}
		if(highest)
			break;
	}
	return highest;
}

EFI_STATUS suboot_mem_id_map(size_t start, size_t end){
	EFI_STATUS status = EFI_SUCCESS;

	uint64_t* pml4t = suboot_alloc(1);
	if(!pml4t){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}
	memset(pml4t, 0, 4096);

	for(size_t addr = start; addr < end; addr += 0x1000){
		uint16_t pl4t_index = (addr >> 39) & 0x1ff;
		uint16_t pdpt_index = (addr >> 30) & 0x1ff;
		uint16_t pd_index = (addr >> 21) & 0x1ff;
		uint16_t pt_index = (addr >> 12) & 0x1ff;

		if((pml4t[pl4t_index] & 1) == 0){
			size_t pdpt_addr = (size_t) suboot_alloc(1);
			if(pdpt_addr == 0){
				status = EFI_OUT_OF_RESOURCES;
				goto _end;
			}
			pml4t[pl4t_index] = (pdpt_addr & 0xfffffffffffff000) | 0x3;
			memset((void*) pdpt_addr, 0, 4096);
		}

		uint64_t* pdpt = (uint64_t*) (pml4t[pl4t_index] & 0xfffffffffffff000);
		if((pdpt[pdpt_index] & 1) == 0){
			size_t pd_addr = (size_t) suboot_alloc(1);
			if(pd_addr == 0){
				status = EFI_OUT_OF_RESOURCES;
				goto _end;
			}
			pdpt[pdpt_index] = (pd_addr & 0xfffffffffffff000) | 0x3;
			memset((void*) pd_addr, 0, 4096);
		}

		uint64_t* pdt = (uint64_t*) (pdpt[pdpt_index] & 0xfffffffffffff000);
		if((pdt[pd_index] & 1) == 0){
			size_t pt_addr = (size_t) suboot_alloc(1);
			if(pt_addr == 0){
				status = EFI_OUT_OF_RESOURCES;
				goto _end;
			}
			pdt[pd_index] = (pt_addr & 0xfffffffffffff000) | 0x3;
			memset((void*) pt_addr, 0, 4096);
		}

		uint64_t* pt = (uint64_t*) (pdt[pd_index] & 0xfffffffffffff000);
		pt[pt_index] = (addr & 0xfffffffffffff000) | 0x3;
	}
	asm("mov %0, %%cr3" : : "r" (pml4t));
	_end:
	return status;
}

