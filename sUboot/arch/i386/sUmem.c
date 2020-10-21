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


size_t suboot_get_address(size_t pdt_index, size_t pt_index){
	size_t addr = 0;

	addr |= (pt_index & 0x3ff) << 12;
	addr |= (pdt_index & 0x3ff) << 22;
	return addr;
}

size_t suboot_get_highest_accessed_address(){
	return 0xffffffff;	// because paging is probably disabled on 32 bit just map everything
						// (we cant find out the highest accessed address without previously having paging enabled)
}

EFI_STATUS suboot_mem_id_map(size_t start, size_t end){
	EFI_STATUS status = EFI_SUCCESS;

	uint32_t* pdt = suboot_alloc(1);
	if(!pdt){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}
	memset(pdt, 0, 4096);

	for(size_t addr = start; addr < end - end % 0x1000; addr += 0x1000){
		uint16_t pd_index = (addr >> 22) & 0x3ff;
		uint16_t pt_index = (addr >> 12) & 0x3ff;

		if((pdt[pd_index] & 1) == 0){
			size_t pt_addr = (size_t) suboot_alloc(1);
			if(pt_addr == 0){
				status = EFI_OUT_OF_RESOURCES;
				goto _end;
			}
			pdt[pd_index] = (pt_addr & 0xfffff000) | 0x3;
			memset((void*) pt_addr, 0, 4096);
		}

		uint32_t* pt = (uint32_t*) (pdt[pd_index] & 0xfffff000);
		pt[pt_index] = (addr & 0xfffff000) | 0x3;
	}
	asm("mov %0, %%cr3" : : "r" (pdt));
	// enable paging (may not be enabled on 32 bit)
	asm("mov %%cr0, %%eax; \
		or $0x80000000, %%eax; \
		mov %%eax, %%cr0;"
		:
		:
		: "%eax");
	_end:
	return status;
}

