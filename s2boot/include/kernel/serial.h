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
 * serial.h - Serial port functions to be implemented by architecture-specific drivers.
 */

#ifndef __KERNEL_SERIAL_H__
#define __KERNEL_SERIAL_H__


status_t serial_init(uint32_t baud);

void serial_clear_buf();
uint8_t serial_get_buf();
void serial_on_input(void (*listener)(uint8_t c));
void serial_write(uint8_t data);
bool serial_input();


#endif /* __KERNEL_SERIAL_H__ */
