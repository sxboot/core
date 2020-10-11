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
#include <klibc/stdarg.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/kb.h>
#include <arch/idt.h>
#include <arch/gdt.h>
#include <x86/kb.h>
#include <x86/irq.h>
#include <x86/pic.h>
#include <x86/x86.h>
#include <arch_gb.h>


void x86_on_irq0(idt_interrupt_frame* frame);


static void (*timerHandler)() = NULL;


status_t arch_platform_initialize(){
	status_t status = 0;

	status = gdt_init();
	CERROR();
	status = idt_init();
	CERROR();
	status = irq_init();
	CERROR();

	reloc_ptr((void**) &timerHandler);

	status = pic_init(X86_HW_INT_BASE, X86_HW_INT_BASE + 8);
	CERROR();
	pic_set_default_isrs(X86_HW_INT_BASE, X86_HW_INT_BASE + 8);
	idt_set_isr(X86_HW_INT_BASE, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &x86_on_irq0);
	idt_set_isr(X86_HW_INT_BASE + 1, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &kb_int);
	pic_enable_interrupts();

	_end:
	return status;
}

status_t arch_platform_reset(){
	status_t status = 0;
	// IDT will be reset by s3boot
	pit_reset();
	pic_disable_interrupts();
	pic_reset();
	return status;
}


status_t arch_set_timer(size_t frequency){
	pit_init(frequency);
	return TSX_SUCCESS;
}

void arch_on_timer_fire(void (*handler)()){
	timerHandler = handler;
}

void arch_on_keypress(void (*handler)(uint16_t c)){
	kb_on_keypress(handler);
}


void arch_sleep(size_t ms){
	pit_sleep(ms);
}

uint16_t arch_sleep_kb(size_t ms){
	return pit_sleep_kb(ms);
}

size_t arch_time(){
	return pit_get_time();
}


void arch_enable_hw_interrupts(){
	pic_enable_interrupts();
}

void arch_disable_hw_interrupts(){
	pic_disable_interrupts();
}

bool arch_is_hw_interrupts_enabled(){
	return pic_is_interrupts_enabled();
}

bool arch_is_hw_interrupt_running(){
	return pic_is_interrupt_running();
}


void arch_relocation(size_t oldAddr, size_t newAddr){
	gdt_relocation(oldAddr, newAddr);
	idt_relocation(oldAddr, newAddr);
}



void __attribute__ ((interrupt)) x86_on_irq0(idt_interrupt_frame* frame){
	pic_hw_int();
	pit_irq_tick();
	if(timerHandler)
		timerHandler();
	pic_send_eoi(0);
}




uint8_t x86_inb(uint16_t port){
	uint8_t data;
	asm("mov %1, %%dx; \
		in %%dx, %%al; \
		mov %%al, %0;"
		: "=r" (data)
		: "r" (port)
		: "%dx");
	return data;
}

uint32_t x86_ind(uint16_t port){
	uint32_t data;
	asm("mov %1, %%dx; \
		in %%dx, %%eax; \
		mov %%eax, %0;"
		: "=r" (data)
		: "r" (port)
		: "%dx");
	return data;
}

void x86_outb(uint16_t port, uint8_t data){
	asm("mov %0, %%dx; \
		mov %1, %%al; \
		out %%al, %%dx;"
		:
		: "r" (port), "r" (data)
		: "%dx", "%al");
}

void x86_outd(uint16_t port, uint32_t data){
	asm("mov %0, %%dx; \
		mov %1, %%eax; \
		out %%eax, %%dx;"
		:
		: "r" (port), "r" (data)
		: "%dx", "%eax");
}

#if defined(ARCH_amd64)
void x86_loadGDT(size_t address){
	asm("mov %0, %%rdx; \
		lgdt (%%rdx);"
		:
		: "r" (address)
		: "%rdx");
}

void x86_loadIDT(size_t address){
	asm("mov %0, %%rdx; \
		lidt (%%rdx);"
		:
		: "r" (address)
		: "%rdx");
}
#elif defined(ARCH_i386)
void x86_loadGDT(size_t address){
	asm("mov %0, %%edx; \
		lgdt (%%edx);"
		:
		: "r" (address)
		: "%edx");
}

void x86_loadIDT(size_t address){
	asm("mov %0, %%edx; \
		lidt (%%edx);"
		:
		: "r" (address)
		: "%edx");
}
#else
#error Invalid x86 architecture
#endif

