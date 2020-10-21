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
#include <kernel/stdio64.h>
#include <kernel/kb.h>
#include <arch/idt.h>
#include <x86/x86.h>
#include <x86/pic.h>

bool pic_interrupts_enabled = false;
bool pic_interrupt_running = false;

size_t pit_frequency = 18;
size_t pit_counter = 0;
size_t pit_time = 0;

uint8_t pic_init(uint8_t irq_base, uint8_t irq_base_h){
	uint8_t icw1 = PIC_ICW1_IC4 | PIC_ICW1_INIT;
	uint8_t icw4 = PIC_ICW4_UPM_X86;

	pic_send_cmd(icw1, 0);
	pic_send_cmd(icw1, 1);

	pic_send_data(irq_base, 0);
	pic_send_data(irq_base_h, 1);

	// connection IRQ between PIC0 and PIC1; 00000100 (bit 2) for IRQ2 for PIC0, 0x2 for IRQ2 for PIC1
	pic_send_data(0x4, 0);
	pic_send_data(0x2, 1);

	pic_send_data(icw4, 0);
	pic_send_data(icw4, 1);
	return 0;
}

void pic_send_cmd(uint8_t cmd, uint8_t id){
	if(id > 1)
		return;
	uint16_t port = (id == 0) ? PIC_COMMAND_REGISTER_0 : PIC_COMMAND_REGISTER_1;
	x86_outb(port, cmd);
}

void pic_send_data(uint8_t data, uint8_t id){
	if(id > 1)
		return;
	uint16_t port = (id == 0) ? PIC_DATA_REGISTER_0 : PIC_DATA_REGISTER_1;
	x86_outb(port, data);
}

uint8_t pic_get_data(uint8_t id){
	if(id > 1)
		return 0;
	uint16_t port = (id == 0) ? PIC_DATA_REGISTER_0 : PIC_DATA_REGISTER_1;
	return x86_inb(port);
}

void pic_hw_int(){
	pic_interrupt_running = true;
}

void pic_send_eoi(uint8_t pic){
	pic_send_cmd(PIC_OCW_EOI, pic);
	pic_interrupt_running = false;
}

void pic_reset(){
	pic_init(0x8, 0x70);
}

void __attribute__ ((interrupt)) pic_none_l(idt_interrupt_frame* frame){
	pic_send_eoi(0);
}

void __attribute__ ((interrupt)) pic_none_h(idt_interrupt_frame* frame){
	pic_send_eoi(0);
	pic_send_eoi(1);
}

void __attribute__ ((interrupt)) pic_irq0(idt_interrupt_frame* frame){
	pic_hw_int();
	pit_irq_tick();
	pic_send_eoi(0);
}

void pic_set_default_isrs(uint8_t irq_base, uint8_t irq_base_h){
	for(int i = irq_base; i < irq_base + 8; i++){
		idt_set_isr(i, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &pic_none_l);
	}
	for(int i = irq_base_h; i < irq_base_h + 8; i++){
		idt_set_isr(i, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &pic_none_h);
	}
	idt_set_isr(irq_base, 0x8, IDT_FLAGS_PRESENT | IDT_FLAGS_TYPE_INT_32, &pic_irq0);
}

void pic_unmask_all(){
	pic_send_data(0, 0);
	pic_send_data(0, 1);
}

void pic_enable_interrupts(){
	pic_interrupts_enabled = true;
	asm("sti");
}

void pic_disable_interrupts(){
	pic_interrupts_enabled = false;
	asm("cli");
}

bool pic_is_interrupts_enabled(){
	return pic_interrupts_enabled;
}

bool pic_is_interrupt_running(){
	return pic_interrupt_running;
}


void pit_init(size_t frequency){
	size_t rate = 1193182 / frequency;
	pit_send_cmd(PIT_OCW_MODE_RATEGEN | PIT_OCW_RL_LSBMSB | PIT_OCW_COUNTER0);
	
	pit_send_data(rate & 0xff, 0);
	pit_send_data((rate >> 8) & 0xff, 0);
	pit_frequency = frequency;
}

void pit_send_cmd(uint8_t cmd){
	x86_outb(PIT_COMMAND_REGISTER, cmd);
}

void pit_send_data(uint8_t data, uint8_t counterID){
	if(counterID > 2)
		return;
	x86_outb(PIT_COUNTER0_REGISTER + counterID, data);
}

void pit_reset(){
	pit_send_cmd(PIT_OCW_MODE_RATEGEN | PIT_OCW_RL_LSBMSB | PIT_OCW_COUNTER0);
	
	// rate = 65536: slowest possible rate (~18.2Hz), default value
	pit_send_data(0xff, 0);
	pit_send_data(0xff, 0);
	pit_frequency = 18;
}

void pit_sleep(size_t ms){
	pic_unmask_all();
	pit_counter = (ms * pit_frequency) / 1000;
	while(pit_counter > 0){
		pit_pause();
	}
}

uint16_t pit_sleep_kb(size_t ms){
	pic_unmask_all();
	kb_clear_buf();
	pit_counter = (ms * pit_frequency) / 1000;
	while(pit_counter > 0){
		pit_pause();
		if(kb_get_buf() != 0)
			return kb_get_buf();
	}
	return 0;
}

void pit_pause(){
	if(pic_interrupts_enabled && pit_frequency > 2 && !pic_interrupt_running)
		asm("hlt");
	else if(pit_counter > 0)
		pit_counter--;
}

void pit_irq_tick(){
	if(pit_counter > 0)
		pit_counter--;
	pit_time += 1000000 / pit_frequency;
}

size_t pit_get_time(){
	return pit_time / 1000;
}
