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
#include <arch/gdt.h>
#include <x86/x86.h>

static gdt gdt_data;
static gdt_desc gdt_descs[GDT_DESCRIPTOR_COUNT];

uint8_t gdt_init(){
	gdt_data.size = sizeof(gdt_desc) * GDT_DESCRIPTOR_COUNT - 1;
	gdt_data.base = (size_t) &gdt_descs;
	gdt_set_entry(0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xffffffff, GDT_FLAGS_PRESENT | GDT_FLAGS_PRIV_0 | GDT_FLAGS_TYPE_NORMAL | GDT_FLAGS_EXECUTABLE | GDT_FLAGS_CODE_READABLE
		| GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_64BIT);
	gdt_set_entry(2, 0, 0xffffffff, GDT_FLAGS_PRESENT | GDT_FLAGS_PRIV_0 | GDT_FLAGS_TYPE_NORMAL | GDT_FLAGS_DATA_WRITABLE
		| GDT_FLAGS_GRANULARITY_4KB);
	gdt_set_entry(3, 0, 0xffffffff, GDT_FLAGS_PRESENT | GDT_FLAGS_PRIV_3 | GDT_FLAGS_TYPE_NORMAL | GDT_FLAGS_EXECUTABLE | GDT_FLAGS_CODE_READABLE
		| GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_64BIT);
	gdt_set_entry(4, 0, 0xffffffff, GDT_FLAGS_PRESENT | GDT_FLAGS_PRIV_3 | GDT_FLAGS_TYPE_NORMAL | GDT_FLAGS_DATA_WRITABLE
		| GDT_FLAGS_GRANULARITY_4KB);

	x86_loadGDT((size_t) &gdt_data);
	reloc_ptr((void**) &gdt_data.base);
	return 0;
}

void gdt_relocation(size_t oldAddr, size_t newAddr){
	x86_loadGDT((size_t) &gdt_data); // base already relocated with reloc_ptr
}

bool gdt_set_entry(uint8_t index, uint32_t base, uint32_t limit, uint16_t flags){
	if(index >= GDT_DESCRIPTOR_COUNT)
		return false;

	gdt_descs[index].baseLow = base & 0xffff;
	gdt_descs[index].baseMid = (base >> 16) & 0xff;
	gdt_descs[index].baseHigh = (base >> 24) & 0xff;
	gdt_descs[index].limitLow = limit & 0xffff;
	gdt_descs[index].flags = flags & 0xf0ff;
	gdt_descs[index].flags |= (limit >> 8) & 0xf00;
	return true;
}
