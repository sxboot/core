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

#ifndef __KERNEL_IDT_H__
#define __KERNEL_IDT_H__


#define IDT_DESCRIPTOR_COUNT 48

#define IDT_FLAGS_TYPE_TASK_32 0x5
#define IDT_FLAGS_TYPE_INT_16 0x6
#define IDT_FLAGS_TYPE_TRAP_16 0x7
#define IDT_FLAGS_TYPE_INT_32 0xe
#define IDT_FLAGS_TYPE_TRAP_32 0xf
#define IDT_FLAGS_SS 0x10
#define IDT_FLAGS_PRESENT 0x80

#pragma pack(push,1)
typedef struct idt{
    uint16_t size;
    size_t base;
} idt;

typedef struct idt_desc{
    uint16_t offsetLow;
    uint16_t segment;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offsetMid;
    uint32_t offsetHigh;
    uint32_t reserved2;
} idt_desc;

typedef struct idt_interrupt_frame{
    size_t i1;
    size_t i2;
    size_t i3;
    size_t i4;
    size_t i5;
    size_t i6;
    size_t i7;
    size_t i8;
    size_t i9;
} idt_interrupt_frame;
#pragma pack(pop)

uint8_t idt_init();
void idt_relocation(size_t oldAddr, size_t newAddr);
void idt_set_isr(uint8_t index, uint16_t segment, uint8_t flags, void* handler);
void idt_double_fault(idt_interrupt_frame* frame);


#endif /* __KERNEL_IDT_H__ */
