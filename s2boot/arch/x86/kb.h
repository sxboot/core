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

#ifndef __ARCH_KB_H__
#define __ARCH_KB_H__

#include <arch/idt.h>

#define KB_ENC_PORT 0x60
#define KB_CONTROLLER_PORT 0x64


void kb_keypress(uint8_t scanCode);
void kb_int(idt_interrupt_frame* frame);


#endif /* __ARCH_KB_H__ */
