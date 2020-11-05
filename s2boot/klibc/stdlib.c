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
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/mmgr.h>



typedef struct smalloc_block{
	uint64_t magic;
	uint64_t allocation;
	struct smalloc_block* next;
} smalloc_block;

#define SMALLOC_BLOCK_MAGIC 0xc91110e742414d53
#define SMALLOC_ALLOCATION_SIZE 64
#define SMALLOC_BLOCKS 64

size_t stdMemUsage = 0;
size_t stdBlockUsage = 0;

smalloc_block* smalloc_list = NULL;

void stdlib_init(){
	reloc_ptr((void**) &smalloc_list);
}

bool stdlib_init_smab_list(){
	if(!smalloc_list){
		smalloc_list = vmmgr_alloc_block();
		if(!smalloc_list)
			return FALSE;
		stdBlockUsage++;
		memset(smalloc_list, 0, MMGR_BLOCK_SIZE);
		smalloc_list->magic = SMALLOC_BLOCK_MAGIC;
		smalloc_list->allocation = 1;
	}
	return TRUE;
}

void* stdlib_alloc_single_from_smab(smalloc_block* block){
	for(size_t i = 1; i < SMALLOC_BLOCKS; i++){
		if(!(block->allocation & (1ULL << i))){
			block->allocation |= 1ULL << i;
			return (void*) block + i * SMALLOC_ALLOCATION_SIZE;
		}
	}
	return NULL;
}

void* stdlib_alloc_sequential_from_smab(smalloc_block* block, size_t blocks){
	for(size_t i = 1; i < SMALLOC_BLOCKS; i++){
		if(!(block->allocation & (1ULL << i))){
			if(i + blocks >= SMALLOC_BLOCKS)
				return NULL;
			size_t k = i;
			for(; k < i + blocks; k++){
				if(block->allocation & (1ULL << k))
					break;
			}
			if(k == i + blocks){
				for(size_t l = i; l < k; l++)
					block->allocation |= 1ULL << l;
				return (void*) block + i * SMALLOC_ALLOCATION_SIZE;
			}
			i += k - i;
		}
	}
	return NULL;
}

void* stdlib_smab_alloc(size_t size){
	if(!stdlib_init_smab_list())
		return NULL;
	size_t blocks = size / SMALLOC_ALLOCATION_SIZE;
	if(size % SMALLOC_ALLOCATION_SIZE != 0)
		blocks++;
	smalloc_block* block = smalloc_list;
	smalloc_block* lastBlock = NULL;
	while(block){
		kernel_runtime_assertion(block->magic == SMALLOC_BLOCK_MAGIC, "Block in SMAB list is not a SMAB block");
		lastBlock = block;
		if(block->allocation != 0xffffffffffffffff){
			void* addr = NULL;
			if(blocks == 1){
				addr = stdlib_alloc_single_from_smab(block);
			}else{
				addr = stdlib_alloc_sequential_from_smab(block, blocks);
			}
			if(addr)
				return addr;
		}
		block = block->next;
	}
	smalloc_block* newBlock = vmmgr_alloc_block();
	if(!newBlock)
		return NULL;
	stdBlockUsage++;
	memset(newBlock, 0, MMGR_BLOCK_SIZE);
	newBlock->magic = SMALLOC_BLOCK_MAGIC;
	newBlock->allocation = 1;
	lastBlock->next = newBlock;
	reloc_ptr((void**) &lastBlock->next);
	for(size_t i = 1; i < blocks + 1; i++)
		newBlock->allocation |= 1ULL << i;
	return (void*) newBlock + SMALLOC_ALLOCATION_SIZE;
}

void* kmalloc_aligned(size_t size){
	stdBlockUsage += size / MMGR_BLOCK_SIZE;
	if(size % MMGR_BLOCK_SIZE != 0)
		stdBlockUsage++;
	stdMemUsage += size;
	return vmmgr_alloc_block_sequential(size);
}

void kfree_aligned(void* ptr, size_t size){
	stdBlockUsage -= size / MMGR_BLOCK_SIZE;
	if(size % MMGR_BLOCK_SIZE != 0)
		stdBlockUsage--;
	stdMemUsage -= size;
	stdMemUsage = MAX(stdMemUsage, 0);
	size_t tdl = size % MMGR_BLOCK_SIZE;
	for(size_t addr = (size_t) ptr; addr < (size_t) ptr + size - tdl + (tdl != 0 ? MMGR_BLOCK_SIZE : 0); addr += 4){
		kernel_del_reloc_ptr((void**) addr);
	}
	vmmgr_free_block_sequential((size_t) ptr, size);
}

void* kmalloc(size_t size){
	void* addr = 0;
	if(size > (SMALLOC_BLOCKS - 1) * SMALLOC_ALLOCATION_SIZE){
		addr = kmalloc_aligned(size);
	}else{
		stdMemUsage += size;
		addr = stdlib_smab_alloc(size);
	}
	return addr;
}

void kfree(void* ptr, size_t size){
	if(size > (SMALLOC_BLOCKS - 1) * SMALLOC_ALLOCATION_SIZE){
		kernel_runtime_assertion((size_t) ptr % 4096 == 0, "Non-small memory allocation is not block-aligned");
		kernel_runtime_assertion(*((uint64_t*) ptr) != SMALLOC_BLOCK_MAGIC, "Attempted to free smab block in non-small memory allocation");
		kfree_aligned(ptr, size);
	}else{
		stdMemUsage -= size;
		smalloc_block* smab = ptr - (size_t) ptr % 4096;
		kernel_runtime_assertion(smab->magic == SMALLOC_BLOCK_MAGIC, "Small memory allocation is not within a smab block");
		size_t smab_index = ((size_t) ptr % 4096) / SMALLOC_ALLOCATION_SIZE;
		kernel_runtime_assertion(smab_index > 0, "Attempted to free smab header block");
		size_t blocks = size / SMALLOC_ALLOCATION_SIZE;
		if(size % SMALLOC_ALLOCATION_SIZE != 0)
			blocks++;
		kernel_runtime_assertion(smab_index + blocks <= SMALLOC_BLOCKS, "Small memory de-allocation goes beyond SMAB boundary");
		for(size_t addr = (size_t) ptr; addr < (size_t) ptr + SMALLOC_ALLOCATION_SIZE * blocks; addr += 4){
			kernel_del_reloc_ptr((void**) addr);
		}
		for(size_t i = smab_index; i < smab_index + blocks; i++)
			smab->allocation &= ~(1ULL << i);
	}
}

void kmem(size_t* memKiB, size_t* memBlocks){
	if(memKiB)
		*memKiB = stdMemUsage / 1024;
	if(memBlocks)
		*memBlocks = stdBlockUsage;
}
