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
#include <klibc/stdio.h>
#include <kernel/stdio64.h>
#include <kernel/log.h>
#include <arch/idt.h>
#include <x86/x86.h>

static idt idt_data;
static idt_desc idt_descs[IDT_DESCRIPTOR_COUNT];

uint8_t idt_init(){
	idt_data.size = sizeof(idt_desc) * IDT_DESCRIPTOR_COUNT - 1;
	idt_data.base = (size_t) &idt_descs;
	for(int i = 0; i < IDT_DESCRIPTOR_COUNT; i++){
		idt_set_isr(i, 0x8, 0, NULL);
	}
	idt_set_isr(8, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &idt_double_fault);

	x86_loadIDT((size_t) &idt_data);
	reloc_ptr((void**) &idt_data.base);
	return 0;
}

void idt_relocation(size_t oldAddr, size_t newAddr){
	x86_loadIDT((size_t) &idt_data); // base already relocated with reloc_ptr
	for(int i = 0; i < IDT_DESCRIPTOR_COUNT; i++){
		if(!(idt_descs[i].flags & IDT_FLAGS_PRESENT))
			continue;
		size_t cAddr = idt_descs[i].offsetLow | (((size_t) idt_descs[i].offsetMid) << 16) | (((size_t) idt_descs[i].offsetHigh) << 32);
		if(cAddr < oldAddr){
			log_warn("IDT relocation (%u) failed: Address is lower than base (0x%X < 0x%X)\n", (size_t) i, cAddr, oldAddr);
		}
		size_t nAddr = cAddr - oldAddr + newAddr;
		idt_descs[i].offsetLow = nAddr & 0xffff;
		idt_descs[i].offsetMid = (nAddr >> 16) & 0xffff;
		idt_descs[i].offsetHigh = (nAddr >> 32) & 0xffffffff;
	}
}

void idt_set_isr(uint8_t index, uint16_t segment, uint8_t flags, void* handler){
	if(index >= IDT_DESCRIPTOR_COUNT)
		return;

	size_t addr = (size_t) handler;
	idt_descs[index].offsetLow = addr & 0xffff;
	idt_descs[index].offsetMid = (addr >> 16) & 0xffff;
	idt_descs[index].offsetHigh = (addr >> 32) & 0xffffffff;
	idt_descs[index].segment = segment;
	idt_descs[index].reserved = 0;
	idt_descs[index].reserved2 = 0;
	idt_descs[index].flags = flags;
}

void __attribute__ ((interrupt)) idt_double_fault(idt_interrupt_frame* frame){
	println("[idt] Double fault", 0x4);
	printp("err=", 0xf);
	printHex(frame->i1);
	print("rip=", 0xf);
	printHex(frame->i2);
	printNln();
	println("This is the default DF handler to prevent a triple fault, system will be halted", 0x4);
	HALT();
}
