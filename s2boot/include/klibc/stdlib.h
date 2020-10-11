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

#ifndef __KLIBC_STDLIB_H__
#define __KLIBC_STDLIB_H__


#define NULL (void*)0

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define KERROR() kernel_error_trace()

#define ERROR() {KERROR();goto _end;}
#define FERROR(e) {status = e;ERROR();}
#define CERROR() {if(status != TSX_SUCCESS){ERROR();}}

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define reloc_ptr(ptr) kernel_add_reloc_ptr(ptr)
#define del_reloc_ptr(ptr) kernel_del_reloc_ptr(ptr)

#define STRINGOF2(x) #x
#define STRINGOF(x) STRINGOF2(x)

#define ASSERT(x, msg) kernel_runtime_assertion(x, msg)


#if !(defined ARCH_amd64 || defined ARCH_i386)
#warning No valid architecture is defined, defaulting to ARCH_amd64
#define ARCH_amd64
#endif

#ifndef ARCH_BITS
#warning ARCH_BITS is not defined, defaulting to 64
#define ARCH_BITS 64
#endif


#include <klibc/limits.h>
#include <kernel/errc.h>
#include <arch_gb.h>

#include <kernel/kutil.h>


void* kmalloc(size_t size);
void kfree(void* ptr, size_t size);
void kmem(size_t* memKiB, size_t* memBlocks);

#endif /* __KLIBC_STDLIB_H__ */
