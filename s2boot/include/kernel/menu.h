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

#ifndef __KERNEL_MENU_H__
#define __KERNEL_MENU_H__


#define MENU_STATE_MAIN 1
#define MENU_STATE_EDIT_OVERVIEW 2
#define MENU_STATE_EDIT_OPTION 3
#define MENU_STATE_ERROR 4

#define MENU_MAX_KEY_LENGTH 64
#define MENU_MAX_VALUE_LENGTH 2048

void menu_show();
void menu_print_title(char* s);

void menu_paint();
void menu_paint_main(size_t cols, size_t rows);
void menu_paint_edit(size_t cols, size_t rows);
void menu_paint_edit_option(size_t cols, size_t rows);

void menu_error_popup(status_t err);

void menu_switch_state(uint8_t state);
uint8_t menu_key(uint16_t key);
bool menu_boot();

void menu_stop_autoboot();
uint8_t* menu_selected_ptr();

#endif /* __KERNEL_MENU_H__ */
