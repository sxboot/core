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

#ifndef __KERNEL_CLI_H__
#define __KERNEL_CLI_H__


status_t cli_init();

void cli_console_key(uint16_t c);
void cli_serial_key(uint8_t c);
void cli_console_reset();

bool cli_reflect_call(char* cmd, uint8_t dest, char* args);
void cli_command(uint8_t dest, char* s);
void cli_printf(uint8_t dest, char* str, ...);

#endif /* __KERNEL_CLI_H__ */
