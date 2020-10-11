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

#ifndef __KERNEL_GDT_H__
#define __KERNEL_GDT_H__


#define GDT_DESCRIPTOR_COUNT 5

#define GDT_FLAGS_PRESENT 0x80
#define GDT_FLAGS_PRIV_0 0x00
#define GDT_FLAGS_PRIV_1 0x20
#define GDT_FLAGS_PRIV_2 0x40
#define GDT_FLAGS_PRIV_3 0x60
#define GDT_FLAGS_TYPE_NORMAL 0x10
#define GDT_FLAGS_EXECUTABLE 0x08
#define GDT_FLAGS_DATA_DIRECTION_DOWN 0x04
#define GDT_FLAGS_CODE_PRIV_LOWER_OR_EQUAL 0x04
#define GDT_FLAGS_DATA_WRITABLE 0x02
#define GDT_FLAGS_CODE_READABLE 0x02
#define GDT_FLAGS_ACCESSED 0x01
#define GDT_FLAGS_GRANULARITY_4KB 0x8000
#define GDT_FLAGS_32BIT 0x4000
#define GDT_FLAGS_64BIT 0x2000

#pragma pack(push,1)
typedef struct gdt{
	uint16_t size;
	size_t base;
} gdt;

typedef struct gdt_desc{
	uint16_t limitLow;
	uint16_t baseLow;
	uint8_t baseMid;
    uint16_t flags;
	uint8_t baseHigh;
} gdt_desc;
#pragma pack(pop)

uint8_t gdt_init();
void gdt_relocation(size_t oldAddr, size_t newAddr);
bool gdt_set_entry(uint8_t index, uint32_t base, uint32_t limit, uint16_t flags);


#endif /* __KERNEL_GDT_H__ */
