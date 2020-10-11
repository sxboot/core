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

#ifndef __ARCH_PIC_H__
#define __ARCH_PIC_H__


#define PIC_COMMAND_REGISTER_0 0x20
#define PIC_DATA_REGISTER_0 0x21
#define PIC_COMMAND_REGISTER_1 0xa0
#define PIC_DATA_REGISTER_1 0xa1

#define PIC_ICW1_IC4 0x01
#define PIC_ICW1_SNGL 0x02
#define PIC_ICW1_ADR_INT4 0x04
#define PIC_ICW1_LVL_TRIG 0x08
#define PIC_ICW1_INIT 0x10

#define PIC_ICW4_UPM_X86 0x01
#define PIC_ICW4_AUTO_EOI 0x02
#define PIC_ICW4_MS 0x04
#define PIC_ICW4_BUFFERED 0x08
#define PIC_ICW4_SF_NEST 0x10

#define PIC_OCW_EOI 0x20
#define PIC_OCW_SEL 0x40
#define PIC_OCW_ROT 0x80


#define PIT_COUNTER0_REGISTER 0x40
#define PIT_COUNTER1_REGISTER 0x41
#define PIT_COUNTER2_REGISTER 0x42
#define PIT_COMMAND_REGISTER 0x43

#define PIT_OCW_BINCOUNT_BINARY	0
#define PIT_OCW_BINCOUNT_BCD	1
#define PIT_OCW_MODE_TERMINALCOUNT	0
#define PIT_OCW_MODE_ONESHOT	0x2
#define PIT_OCW_MODE_RATEGEN	0x4
#define PIT_OCW_MODE_SQUAREWAVEGEN	0x6
#define PIT_OCW_MODE_SOFTWARETRIG	0x8
#define PIT_OCW_MODE_HARDWARETRIG	0xA
#define PIT_OCW_RL_LATCH		0
#define PIT_OCW_RL_LSBONLY		0x10
#define PIT_OCW_RL_MSBONLY		0x20
#define PIT_OCW_RL_LSBMSB		0x30
#define PIT_OCW_COUNTER0		0
#define PIT_OCW_COUNTER1		0x40
#define PIT_OCW_COUNTER2		0x80


uint8_t pic_init(uint8_t irq_base, uint8_t irq_base_h);
void pic_send_cmd(uint8_t cmd, uint8_t id);
void pic_send_data(uint8_t data, uint8_t id);
uint8_t pic_get_data(uint8_t id);
void pic_hw_int();
void pic_send_eoi(uint8_t pic);
void pic_reset();
void pic_set_default_isrs(uint8_t irq_base, uint8_t irq_base_h);
void pic_enable_interrupts();
void pic_disable_interrupts();
bool pic_is_interrupts_enabled();
bool pic_is_interrupt_running();

void pit_init(size_t frequency);
void pit_send_cmd(uint8_t cmd);
void pit_send_data(uint8_t data, uint8_t counterID);
void pit_reset();
void pit_sleep(size_t ms);
uint16_t pit_sleep_kb(size_t ms);
void pit_pause();
void pit_irq_tick();
size_t pit_get_time();


#endif /* __ARCH_PIC_H__ */
