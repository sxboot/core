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
 * arch_gb.h - Architecture abstraction.
 */

#ifndef __ARCH_GB_H__
#define __ARCH_GB_H__

#include <klibc/stdint.h>
#include <klibc/stdbool.h>

// provided by the architecture-specific implementation at src/arch/<ARCH>/arch.h
#include <arch.h>


status_t arch_platform_initialize();
status_t arch_platform_reset();

status_t arch_set_timer(size_t frequency);
void arch_on_timer_fire(void (*handler)());
void arch_on_keypress(void (*handler)(uint16_t c));

void arch_sleep(size_t ms);
uint16_t arch_sleep_kb(size_t ms);
size_t arch_time();

void arch_enable_hw_interrupts();
void arch_disable_hw_interrupts();
bool arch_is_hw_interrupts_enabled();
bool arch_is_hw_interrupt_running();

void arch_relocation(size_t oldAddr, size_t newAddr);


#endif /* __ARCH_GB_H__ */
