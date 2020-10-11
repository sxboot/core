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

#ifndef __ARCH_X86_H__
#define __ARCH_X86_H__


#define X86_HW_INT_BASE 32



uint8_t x86_inb(uint16_t port);
uint32_t x86_ind(uint16_t port);
void x86_outb(uint16_t port, uint8_t data);
void x86_outd(uint16_t port, uint32_t data);

void x86_loadGDT(size_t address);
void x86_loadIDT(size_t address);


#endif /* __ARCH_X86_H__ */
