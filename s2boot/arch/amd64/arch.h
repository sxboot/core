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

#ifndef __ARCH_H__
#define __ARCH_H__

#ifndef ARCH_amd64
#error Included file for amd64
#endif


typedef unsigned long long	size_t;


#define ARCH_GET_SP(x) asm("mov %%rsp, %0" : "=r" (x))
#define ARCH_SET_SP(x) asm("mov %0, %%rsp" : : "r" (x))

#define ARCH_GET_FP(x) asm("mov %%rbp, %0" : "=r" (x))
#define ARCH_SET_FP(x) asm("mov %0, %%rbp" : : "r" (x))

#define ARCH_JUMP(x) asm("mov %0, %%rdi; jmp *%%rdi" : : "r" ((size_t) (x)) : "%rdi")
#define ARCH_JUMP_SARG(x, arg) asm("mov %0, %%rdx; push %%rdx; mov %1, %%rdi; jmp *%%rdi" : : "r" ((size_t) (arg)), "r" ((size_t) (x)) : "%rdx", "%rdi")

#define HALT() __halt:asm("cli");asm("hlt");goto __halt;

#define ARCH_DEFAULT_STACK_LOCATION 0x7c00
#define ARCH_DEFAULT_MBR_LOCATION 0x7c00


typedef struct arch_stack_frame{
	struct arch_stack_frame* next;
	size_t ip;
} arch_stack_frame;

typedef struct arch_os_entry_state{
	size_t a;
	size_t b;
	size_t c;
	size_t d;
	size_t sp;
	size_t bp;
	size_t si;
	size_t di;
	size_t gdt_alt_base;
	size_t gdt_alt_size;
	uint16_t cs;
	uint16_t ds;
} arch_os_entry_state;


#endif /* __ARCH_H__ */
