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
 * serial.c - Serial port driver for x86.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <kernel/log.h>
#include <kernel/serial.h>
#include <x86/pic.h>
#include <x86/x86.h>
#include <x86/serial.h>

static uint16_t comPorts[] = {SERIAL_COM1, SERIAL_COM2, SERIAL_COM3, SERIAL_COM4};

static uint8_t comIndex = 0;
static uint16_t comPort = 0;

static void (*serial_on_key)(uint8_t c) = NULL;
static uint8_t comBuf = 0;

status_t serial_init(uint32_t baud){
	status_t status = 0;
	// probe ports
	for(uint8_t i = 0; i < 4; i++){
		x86_outb(comPorts[i] + 7, 0xaa);
		if(x86_inb(comPorts[i] + 7) == 0xaa){
			comIndex = i;
			comPort = comPorts[i];
			log_debug("[serial] Active serial port on COM%u (0x%X)\n", comIndex + 1, comPorts[comIndex]);
			break;
		}
	}
	if(!comPort){
		FERROR(TSX_UNAVAILABLE);
	}

	uint16_t divisor = 115200 / baud;
	// http://www.roboard.com/Files/Reg/Serial_Port_Registers.pdf
	x86_outb(comPorts[comIndex] + 3, 0); // line control: disable DLAB
	x86_outb(comPorts[comIndex] + 1, 0); // interrupt enable register: disable interrupts
	x86_outb(comPorts[comIndex] + 3, 0x80); // line control: enable DLAB
	x86_outb(comPorts[comIndex] + 0, (uint8_t) divisor); // line control: enable DLAB
	x86_outb(comPorts[comIndex] + 1, (uint8_t) (divisor >> 8)); // line control: enable DLAB
	x86_outb(comPorts[comIndex] + 3, 0x03); // line control: disable DLAB, SCN = 8 bits
	x86_outb(comPorts[comIndex] + 2, 0x07); // fifo control: TL = 1 byte, clear and enable fifos
	x86_outb(comPorts[comIndex] + 4, 0x0b); // modem control: irq enable
	x86_outb(comPorts[comIndex] + 1, 0x01); // interrupt enable register: enable received data
	_end:
	return status;
}

void serial_clear_buf(){
	comBuf = 0;
}

uint8_t serial_get_buf(){
	return comBuf;
}

void serial_on_input(void (*listener)(uint8_t c)){
	serial_on_key = listener;
}

void serial_write(uint8_t data){
	x86_outb(comPorts[comIndex], data);
}


void __attribute__ ((interrupt)) serial_int(idt_interrupt_frame* frame){
	pic_hw_int();
	uint8_t iir = x86_inb(comPorts[comIndex] + 2); // int identification register
	if((iir & 0x6) == 0x4 /* received data */){
		uint8_t data = x86_inb(comPorts[comIndex]);
		if(serial_on_key){
			serial_on_key(data);
			serial_clear_buf();
		}
	}
	pic_send_eoi(0);
}

