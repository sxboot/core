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
#include <kernel/mmgr.h>



size_t stdMemUsage = 0;
size_t stdBlockUsage = 0;

void* kmalloc(size_t size){
	stdBlockUsage += size / MMGR_BLOCK_SIZE;
	if(size % MMGR_BLOCK_SIZE != 0)
		stdBlockUsage++;
	stdMemUsage += size;
	return vmmgr_alloc_block_sequential(size);
}

void kfree(void* ptr, size_t size){
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

void kmem(size_t* memKiB, size_t* memBlocks){
	if(memKiB)
		*memKiB = stdMemUsage / 1024;
	if(memBlocks)
		*memBlocks = stdBlockUsage;
}
