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
 * mmgr.c - Memory manager.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/errc.h>
#include <kernel/mmgr.h>

mmgr_arch_mmap_entry* mmgr_memoryMap;
uint16_t mmgr_memoryMapLength;

uint8_t* mmgr_alloc_map = 0;
size_t mmgr_alloc_map_size = 0;
size_t mmgr_total_blocks = 0;
size_t mmgr_used_blocks_reserved = 0;
size_t mmgr_used_blocks_alloc = 0;
size_t mmgr_used_blocks_paging = 0;

size_t vmmgr_mapped_pages = 0;

size_t vmmgr_membase = 0;

size_t vmmgr_mappedMemory = 0;

status_t mmgr_init(size_t mmapStart, uint16_t mmapLength){
	mmgr_memoryMap = (mmgr_arch_mmap_entry*) mmapStart;
	mmgr_memoryMapLength = mmapLength;
	status_t status = mmgr_create_map();
	CERROR();
	status = mmgr_init_arch();
	CERROR();
	mmgr_reserve_default_regions();
	mmgr_reserve_mem_region(mmapStart, mmapLength * sizeof(mmgr_arch_mmap_entry), MMGR_MEMTYPE_BOOTLOADER_DATA);
	mmgr_reserve_mem_region((size_t) mmgr_alloc_map, mmgr_alloc_map_size, MMGR_MEMTYPE_BOOTLOADER_DATA);

	_end:
	return status;
}

void mmgr_reg_map_reloc_ptr(){
	reloc_ptr((void**) &mmgr_memoryMap);
	reloc_ptr((void**) &mmgr_alloc_map);
	mmgr_reg_map_reloc_ptr_arch();
}

void mmgr_relocation(size_t oldAddr, size_t newAddr){
	vmmgr_membase = newAddr;
}

void mmgr_map_set(size_t index, memtype_t type){
	if(index >= mmgr_total_blocks || mmgr_alloc_map == NULL)
		return;
	type &= 0xf;
	mmgr_alloc_map[index / MMGR_MMAP_BLOCKS_PER_BYTE] &= ~(0xf << ((index % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH));
	mmgr_alloc_map[index / MMGR_MMAP_BLOCKS_PER_BYTE] |= (type << ((index % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH));
}

void mmgr_map_clear(size_t index){
	if(index >= mmgr_total_blocks || index == 0 /* first block is always reserved */ || mmgr_alloc_map == NULL)
		return;
	mmgr_alloc_map[index / MMGR_MMAP_BLOCKS_PER_BYTE] &= ~(0xf << ((index % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH));
}

memtype_t mmgr_get_map_bit_type(size_t index){
	if(index >= mmgr_total_blocks || mmgr_alloc_map == NULL)
		return 1;
	return (mmgr_alloc_map[index / MMGR_MMAP_BLOCKS_PER_BYTE] >> ((index % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH)) & 0xf;
}

bool mmgr_get_map_bit(size_t index){
	if(index >= mmgr_total_blocks || mmgr_alloc_map == NULL)
		return 1;
	return (mmgr_alloc_map[index / MMGR_MMAP_BLOCKS_PER_BYTE] & (0xf << ((index % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH))) != 0;
}

size_t mmgr_get_first_free_block(){
	for(size_t i = 0; i < mmgr_total_blocks; i++){
		if(mmgr_get_map_bit(i) == 0)
			return i;
	}
	return -1;
}

size_t mmgr_get_first_free_block_sequential(uint16_t length){
	if(mmgr_alloc_map == NULL)
		return -1;
	for(size_t i = 0; i < mmgr_total_blocks / MMGR_MMAP_BLOCKS_PER_BYTE; i++){
		for(size_t j = 0; j < MMGR_MMAP_BLOCKS_PER_BYTE; j++){
			if((mmgr_alloc_map[i] & (0xf << (j * MMGR_MMAP_BLOCK_WIDTH))) == 0){
				int k = j;
				for(; k < length + j; k++){
					if(k < MMGR_MMAP_BLOCKS_PER_BYTE){
						if((mmgr_alloc_map[i] & (0xf << (k * MMGR_MMAP_BLOCK_WIDTH))))
							break;
					}else{
						if((mmgr_alloc_map[i + (k / MMGR_MMAP_BLOCKS_PER_BYTE)] & (0xf << ((k % MMGR_MMAP_BLOCKS_PER_BYTE) * MMGR_MMAP_BLOCK_WIDTH))))
							break;
					}
					if(k == (length + j - 1))
						return i * MMGR_MMAP_BLOCKS_PER_BYTE + j;
				}
				j += k - j;
			}
		}
	}
	return -1;
}

void mmgr_reserve_mem_region(size_t base, size_t size, memtype_t type){
	size_t base_b = base / MMGR_BLOCK_SIZE;
	size_t size_b = size / MMGR_BLOCK_SIZE;
	if(base % MMGR_BLOCK_SIZE != 0)
		size_b++;
	if(size % MMGR_BLOCK_SIZE != 0)
		size_b++;

	for(size_t i = 0; i < size_b; i++){
		if(mmgr_get_map_bit(i + base_b) == 0 && i + base_b < mmgr_total_blocks)
			mmgr_used_blocks_reserved++;
		mmgr_map_set(i + base_b, type);
	}
}

void mmgr_free_mem_region(size_t base, size_t size){
	size_t base_b = base / MMGR_BLOCK_SIZE;
	size_t size_b = size / MMGR_BLOCK_SIZE;
	/*if(base % MMGR_BLOCK_SIZE != 0)	// dont look for this to prevent more being freed than should be freed
		size_b++;
	if(size % MMGR_BLOCK_SIZE != 0)
		size_b++;*/
	
	for(size_t i = 0; i < size_b; i++){
		if(mmgr_get_map_bit(i + base_b) != 0 && i + base_b < mmgr_total_blocks)
			mmgr_used_blocks_reserved--;
		mmgr_map_clear(i + base_b);
	}
}

void* mmgr_alloc_block(){
	size_t block = mmgr_get_first_free_block();
	if(block == -1)
		return 0;
	
	mmgr_map_set(block, MMGR_MEMTYPE_BOOTLOADER_DATA);
	mmgr_used_blocks_alloc++;
	return (void*) (block * MMGR_BLOCK_SIZE);
}

void* mmgr_alloc_block_sequential(size_t size){
	uint16_t length = size / MMGR_BLOCK_SIZE;
	if(size % MMGR_BLOCK_SIZE != 0)
		length++;
	size_t block = mmgr_get_first_free_block_sequential(length);
	if(block == -1)
		return 0;
	
	for(int i = 0; i < length; i++){
		mmgr_map_set(block + i, MMGR_MEMTYPE_BOOTLOADER_DATA);
		mmgr_used_blocks_alloc++;
	}
	return (void*) (block * MMGR_BLOCK_SIZE);
}

void mmgr_free_block(size_t addr){
	if(mmgr_get_map_bit(addr / MMGR_BLOCK_SIZE) != 0)
		mmgr_used_blocks_alloc--;
	mmgr_map_clear(addr / MMGR_BLOCK_SIZE);
}

void mmgr_free_block_sequential(size_t addr, size_t size){
	size_t length = size / MMGR_BLOCK_SIZE;
	if(size % MMGR_BLOCK_SIZE != 0)
		length++;
	for(int i = 0; i < length; i++){
		if(mmgr_get_map_bit(addr / MMGR_BLOCK_SIZE) != 0)
			mmgr_used_blocks_alloc--;
		mmgr_map_clear(addr / MMGR_BLOCK_SIZE);
		addr += MMGR_BLOCK_SIZE;
	}
}

bool mmgr_is_area_clear(size_t base, size_t size){
	size_t base_b = base / MMGR_BLOCK_SIZE;
	size_t size_b = size / MMGR_BLOCK_SIZE;
	if(base % MMGR_BLOCK_SIZE != 0)
		size_b++;
	if(size % MMGR_BLOCK_SIZE != 0)
		size_b++;
	
	for(size_t i = 0; i < size_b; i++){
		if(mmgr_get_map_bit(i + base_b) != 0)
			return false;
	}
	return true;
}

size_t mmgr_get_total_blocks(){
	return mmgr_total_blocks;
}

size_t mmgr_get_used_blocks_alloc(){
	return mmgr_used_blocks_alloc;
}

size_t mmgr_get_used_blocks_reserved(){
	return mmgr_used_blocks_reserved;
}

size_t mmgr_get_used_blocks_paging(){
	return mmgr_used_blocks_paging;
}

size_t mmgr_get_used_blocks(){
	return mmgr_used_blocks_alloc + mmgr_used_blocks_reserved + mmgr_used_blocks_paging;
}

size_t mmgr_get_used_mem_kib(){
	return (mmgr_used_blocks_alloc + mmgr_used_blocks_reserved + mmgr_used_blocks_paging) * MMGR_BLOCK_SIZE / 1024;
}

uint8_t* mmgr_get_alloc_map(){
	return mmgr_alloc_map;
}

void mmgr_get_system_map(mmgr_arch_mmap_entry** addrWrite, uint16_t* lengthWrite){
	if(addrWrite)
		*addrWrite = mmgr_memoryMap;
	if(lengthWrite)
		*lengthWrite = mmgr_memoryMapLength;
}

size_t mmgr_gen_mmap(mmap_entry* buf, size_t buflen, size_t* length){
	memset(buf, 0, buflen);
	size_t cindex = 0;
	size_t blength = 0;
	memtype_t lastType = mmgr_get_map_bit_type(0);
	size_t lastBase = 0;
	memtype_t ct;
	for(size_t i = 1; i < mmgr_total_blocks; i++){
		if((ct = mmgr_get_map_bit_type(i)) != lastType){
			if(sizeof(mmap_entry) * (cindex + 1) <= buflen){
				buf[cindex].addr = lastBase;
				buf[cindex].size = i * MMGR_BLOCK_SIZE - lastBase;
				buf[cindex].type = lastType;
				blength++;
			}
			cindex++;
			lastBase = i * MMGR_BLOCK_SIZE;
			lastType = ct;
		}
	}
	if(sizeof(mmap_entry) * (cindex + 1) <= buflen){
		buf[cindex].addr = lastBase;
		buf[cindex].size = mmgr_total_blocks * MMGR_BLOCK_SIZE - lastBase;
		buf[cindex].type = lastType;
		blength++;
	}
	cindex++;
	for(int i = 0; i < mmgr_memoryMapLength && sizeof(mmgr_arch_mmap_entry) > 0; i++){ // add e820 map entries not in the alloc map (because they are above addressable memory)
		size_t addr, size, type;
		mmgr_arch_get_memmap_val(&mmgr_memoryMap[i], &addr, &size, &type);
		if(addr >= mmgr_total_blocks * MMGR_BLOCK_SIZE){
			if(sizeof(mmap_entry) * (cindex + 1) <= buflen){
				buf[cindex].addr = addr;
				buf[cindex].size = size;
				buf[cindex].type = type;
				blength++;
			}
			cindex++;
		}
	}
	if(length)
		*length = blength;
	return cindex;
}


void* mmgr_alloc_block_p(){
	size_t block = mmgr_get_first_free_block();
	if(block == -1)
		return 0;

	mmgr_map_set(block, MMGR_MEMTYPE_PAGING);
	mmgr_used_blocks_paging++;
	return (void*) (block * MMGR_BLOCK_SIZE);
}

void mmgr_free_block_p(size_t addr){
	if(mmgr_get_map_bit(addr / MMGR_BLOCK_SIZE) != 0)
		mmgr_used_blocks_paging--;
	mmgr_map_clear(addr / MMGR_BLOCK_SIZE);
}


void* vmmgr_alloc_block(){
	vmmgr_map_pages_req(mmgr_get_used_blocks() * MMGR_BLOCK_SIZE + MMGR_BLOCK_SIZE);
	size_t paddress = (size_t) mmgr_alloc_block();
	if(paddress == 0)
		return 0;
	return (void*) (paddress + vmmgr_membase);
}

void* vmmgr_alloc_block_sequential(size_t size){
	vmmgr_map_pages_req(mmgr_get_used_blocks() * MMGR_BLOCK_SIZE + size);
	size_t paddress = (size_t) mmgr_alloc_block_sequential(size);
	if(paddress == 0)
		return 0;
	vmmgr_map_pages_req(paddress + size); // in case it got allocated somewhere above mapped memory because of memory fragmentation (see example below)
	/* lets say we have this highly fragmented memory map (0 is free, 1 is allocated):
	   1111000100110001001000010001100000000000000000000000000000000000
									  ^ memory will be mapped up to here (inclusive, 32 blocks mapped)
									^ this is where a memory region with 5 or more blocks (size) will be allocated to (-> not all allocated memory is mapped) (paddress)
										^ saying we will use this much memory (paddress + size) will cause more memory to be mapped \
											(page tables will use free memory below this; it is guaranteed that there is enough memory for the tables)
	*/
	return (void*) (paddress + vmmgr_membase);
}

void vmmgr_map_pages_req(size_t usedMem){
	if(mmgr_alloc_map == NULL) // mmgr not initalized yet
		return;
	while(usedMem + MMGR_BLOCK_SIZE * 8 /* keep at least 8 blocks free */ >= vmmgr_mappedMemory){
		size_t nMap = MIN(vmmgr_mappedMemory * 2, MMGR_USABLE_MEMORY);
		for(size_t addr = vmmgr_mappedMemory; addr < nMap; addr += MMGR_BLOCK_SIZE){
			vmmgr_map_page(addr, addr + vmmgr_membase);
		}
		vmmgr_mappedMemory = nMap;
		if(nMap >= MMGR_USABLE_MEMORY)
			break;
	}
}

void vmmgr_free_block(size_t addr){
	if(addr >= vmmgr_membase)
		mmgr_free_block(addr - vmmgr_membase);
}

void vmmgr_free_block_sequential(size_t addr, size_t size){
	if(addr >= vmmgr_membase)
		mmgr_free_block_sequential(addr - vmmgr_membase, size);
}


bool vmmgr_is_address_accessible(size_t virt){
	return vmmgr_get_physical(virt) != 0;
}

size_t vmmgr_get_mapped_pages(){
	return vmmgr_mapped_pages;
}
