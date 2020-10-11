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
#include <kernel/stdio64.h>
#include <kernel/errc.h>
#include <kernel/elf.h>
#include <kernel/mmgr.h>
#include <kernel/kutil.h>
#include <arch/idt.h>
#include <x86/irq.h>

static bool fatalErr = FALSE;

void __attribute__ ((interrupt)) irq_div_zero(idt_interrupt_frame* frame){
	irq_error("Divide By Zero", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_debug(idt_interrupt_frame* frame){
	irq_error("Debug", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_nmi(idt_interrupt_frame* frame){
	irq_error("NMI", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_bp(idt_interrupt_frame* frame){
	irq_error("Breakpoint", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_overflow(idt_interrupt_frame* frame){
	irq_error("Overflow", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_br(idt_interrupt_frame* frame){
	irq_error("Bound Range Exceeded", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_invOp(idt_interrupt_frame* frame){
	irq_error("Illegal Opcode", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_dev_not_available(idt_interrupt_frame* frame){
	irq_error("Device Not Available", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_coproc(idt_interrupt_frame* frame){
	irq_error("Coprocessor Segment Overrun", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_invTSS(idt_interrupt_frame* frame){
	irq_error("Invalid TSS", TRUE, frame);
}

void __attribute__ ((interrupt)) irq_seg(idt_interrupt_frame* frame){
	irq_error("Segment Not Present", TRUE, frame);
}

void __attribute__ ((interrupt)) irq_stackseg(idt_interrupt_frame* frame){
	irq_error("Stack Segment Fault", TRUE, frame);
}

void __attribute__ ((interrupt)) irq_gpf(idt_interrupt_frame* frame){
	irq_error("General Protection Fault", TRUE, frame);
}

void __attribute__ ((interrupt)) irq_pagef(idt_interrupt_frame* frame){
	if(fatalErr){
		HALT();
	}
	fatalErr = TRUE;
	printNlnr();
	printf("&4%s\n", "Page Fault");
	printf("&f%s tried to %s %s\n", ((frame->i1 >> 2) & 1) ? "User process" : "Supervisory process", ((frame->i1 >> 1) & 1) ? "write to a" : "read a",
		((frame->i1) & 1) ? "page and caused a protection fault" : "non-present page");
	if(!((frame->i1) & 1)){
		size_t reqAddr;
		asm("mov %%cr2, %0" : "=r" (reqAddr));
		printf("&fRequested address: &e%Y\n", reqAddr);
	}
	size_t functionOffset = 0;
	char* name = kernel_get_symbol_for_address(frame->i2, &functionOffset);
	printf("&ferr=%Y ip=%Y (&f%s&7+0x%X)\n", frame->i1, frame->i2, name, functionOffset);
	kernel_print_stack_trace();
	kernel_halt();
}

void __attribute__ ((interrupt)) irq_x87(idt_interrupt_frame* frame){
	irq_error("x87 Floating-Point Exception", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_align(idt_interrupt_frame* frame){
	irq_error("Alignment Check", TRUE, frame);
}

void __attribute__ ((interrupt)) irq_machine(idt_interrupt_frame* frame){
	irq_error("Machine Check", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_simd(idt_interrupt_frame* frame){
	irq_error("SIMD Floating-Point Exception", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_virt(idt_interrupt_frame* frame){
	irq_error("Virtualization Exception", FALSE, frame);
}

void __attribute__ ((interrupt)) irq_sec(idt_interrupt_frame* frame){
	irq_error("Security Exception", TRUE, frame);
}

void irq_error(char* name, bool errcode, idt_interrupt_frame* frame){
	printNlnr();
	printf("&4%s\n", name);
	size_t functionOffset = 0;
	char* funcname = kernel_get_symbol_for_address(errcode ? frame->i2 : frame->i1, &functionOffset);
	printf("&ferr=%Y ip=%Y (&f%s&7+0x%X)\n", errcode ? frame->i1 : 0, errcode ? frame->i2 : frame->i1, funcname, functionOffset);
	kernel_print_stack_trace();
	kernel_halt();
}

uint8_t irq_init(){
	irq_set_handlers();
	return 0;
}

void irq_set_handlers(){
	idt_set_isr(0, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_div_zero);
	idt_set_isr(1, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_debug);
	idt_set_isr(2, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_nmi);
	idt_set_isr(3, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_bp);
	idt_set_isr(4, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_overflow);
	idt_set_isr(5, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_br);
	idt_set_isr(6, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_invOp);
	idt_set_isr(7, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_dev_not_available);
	idt_set_isr(9, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_coproc);
	idt_set_isr(10, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_invTSS);
	idt_set_isr(11, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_seg);
	idt_set_isr(12, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_stackseg);
	idt_set_isr(13, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_gpf);
	idt_set_isr(14, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_pagef);
	idt_set_isr(16, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_x87);
	idt_set_isr(17, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_align);
	idt_set_isr(18, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_machine);
	idt_set_isr(19, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_simd);
	idt_set_isr(20, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_virt);
	idt_set_isr(30, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &irq_sec);
}
