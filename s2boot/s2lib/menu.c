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

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/stdio.h>
#include <klibc/string.h>
#include <kernel/kutil.h>
#include <kernel/util.h>
#include <kernel/parse.h>
#include <kernel/log.h>
#include <kernel/stdio64.h>
#include <kernel/kb.h>
#include <kernel/list.h>
#include <kernel/menu.h>


static uint8_t menu_selected = 0;
static bool menu_autoboot = TRUE;
static size_t menu_counter = 0;

static uint8_t menu_state = MENU_STATE_MAIN;

static size_t out_cols = 0;
static size_t out_rows = 0;

static size_t menu_edit_value_offset = 10;
static uint8_t menu_edit_index = 0;

static char* menu_edit_text_key = NULL;
static char* menu_edit_text_value = NULL;
static bool menu_edit_editing_key = TRUE;


void menu_show(){
	if(!menu_edit_text_key){
		menu_edit_text_key = kmalloc(MENU_MAX_KEY_LENGTH);
		menu_edit_text_value = kmalloc(MENU_MAX_VALUE_LENGTH);
		if(!menu_edit_text_key || !menu_edit_text_value){
			log_fatal("Out of memory");
			return;
		}
	}
	stdio64_get_text_size(&out_rows, &out_cols);
	menu_counter = parse_get_data()->timeout;
	menu_switch_state(MENU_STATE_MAIN);
	menu_paint();
}

void menu_print_title(char* s){
	size_t cols, rows;
	stdio64_get_text_size(&rows, &cols);
	for(int i = 0; i < cols; i++)
		printCharAt(0x20, 0x70, i, 0);
	char* title;
	char* version;
	kernel_get_brand(&title, &version, NULL);
	printAt(title, 0x70, 0, 0);
	printAt(" - ", 0x70, strlen(title), 0);
	printAt(s, 0x70, strlen(title) + 3, 0);
	printAt(version, 0x70, cols - strlen(version), 0);
}


void menu_paint(){
	if(menu_state == MENU_STATE_MAIN)
		menu_paint_main(out_cols, out_rows);
	else if(menu_state == MENU_STATE_EDIT_OVERVIEW)
		menu_paint_edit(out_cols, out_rows);
	else if(menu_state == MENU_STATE_EDIT_OPTION)
		menu_paint_edit_option(out_cols, out_rows);
}

void menu_paint_main(size_t cols, size_t rows){
	menu_print_title("Select operating system");
	parse_data_t* parse_data = parse_get_data();
	for(int i = 0; i < PARSE_MAX_ENTRIES; i++){
		for(int j = 1; j < cols - 1; j++)
			printCharAt(0x20, (menu_selected == i) ? 0x70 : 0x7, j, 2 + i);
		if(parse_data->entries[i].name != 0){
			printAt(parse_data->entries[i].name, (menu_selected == i) ? 0x70 : 0x7, 1, 2 + i);
		}else if(menu_selected == i){
			printAt("+ New Temporary Entry (press E)", 0x72, 1, 2 + i);
		}
	}
	printAt("Use arrow keys \x18\x19 to select", 0xf, 0, rows - 6);
	printAt("Press ENTER to boot or E to edit the selected entry", 0xf, 0, rows - 5);
	printAt("Press C to enter CLI", 0xf, 0, rows - 4);
	for(int i = 0; i < cols; i++)
		printCharAt(0x20, 0x7, i, rows - 2);
	if(menu_autoboot){
		printAt("Autoboot in ", 0x7, 0, rows - 2);
		char* numstr = getDec(menu_counter);
		printAt(numstr, 0x7, 12, rows - 2);
		if(menu_counter == 1)
			printAt(" second", 0x7, 12 + strlen(numstr), rows - 2);
		else
			printAt(" seconds", 0x7, 12 + strlen(numstr), rows - 2);
	}
}

void menu_paint_edit(size_t cols, size_t rows){
	menu_print_title("Edit");
	parse_data_t* parse_data = parse_get_data();
	parse_entry* entry = &parse_data->entries[menu_selected];
	status_t status = parse_file_entry(menu_selected);
	if(status != TSX_SUCCESS){
		menu_error_popup(status);
		return;
	}
	for(int j = 1; j < cols - 1; j++)
		printCharAt(0x20, 0x30, j, 2);
	printAt("Option", 0x30, 1, 2);
	printAt("Value", 0x30, 1 + menu_edit_value_offset, 2);
	for(size_t i = 0; i < entry->optionsKeys->length + 1; i++){
		for(int j = 1; j < cols - 1; j++)
			printCharAt(0x20, (menu_edit_index == i) ? 0x70 : 0x7, j, 3 + i);
		for(size_t j = 0; j < menu_edit_value_offset - 1; j++){
			char c = (i == 0 ? "type" : ((char*) entry->optionsKeys->base[i - 1]))[j];
			printCharAt(c, (menu_edit_index == i) ? 0x70 : 0x7, 1 + j, 3 + i);
			if(!c)
				break;
		}
		for(size_t j = 0; j < cols - menu_edit_value_offset - 2; j++){
			char c = (i == 0 ? entry->type : ((char*) entry->optionsValues->base[i - 1]))[j];
			printCharAt(c, (menu_edit_index == i) ? 0x70 : 0x7, 1 + menu_edit_value_offset + j, 3 + i);
			if(!c)
				break;
		}
	}
	for(int i = 1; i < cols - 1; i++)
		printCharAt(0x20, 0x7, i, 4 + entry->optionsKeys->length);
	if(menu_edit_index > entry->optionsKeys->length){
		for(int j = 1; j < cols - 1; j++)
			printCharAt(0x20, 0x70, j, 4 + entry->optionsKeys->length);
		printAt("+ New Temporary Entry", 0x72, 1, 4 + entry->optionsKeys->length);
	}
}

void menu_paint_edit_option(size_t cols, size_t rows){
	menu_print_title("Edit option");
	parse_data_t* parse_data = parse_get_data();
	parse_entry* entry = &parse_data->entries[menu_selected];
	printAt("Option: ", 0xf, 1, 2);
	printAt("Value: ", 0xf, 1, 4);
	printAt(menu_edit_text_key, 0x7, 9, 2);
	printCharAt(0x20, 0x7, 9 + strlen(menu_edit_text_key), 2);
	size_t charsPerRow = cols - 10;
	size_t valLen = strlen(menu_edit_text_value) + 1;
	for(size_t i = 0; i < valLen; i++){
		printCharAt(menu_edit_text_value[i], 0x7, 9 + i % charsPerRow, 4 + i / charsPerRow);
		if(i == valLen - 1 && !menu_edit_editing_key)
			setCursorPosition(9 + i % charsPerRow, 4 + i / charsPerRow);
	}
	if(menu_edit_editing_key){
		setCursorPosition(9 + strlen(menu_edit_text_key), 2);
	}
}

void menu_edit_option_save(){
	parse_entry* entry = &parse_get_data()->entries[menu_selected];
	if(menu_edit_index == 0){
		if(strcmp(entry->type, menu_edit_text_value) != 0){
			char* nType = kmalloc(strlen(menu_edit_text_value) + 1);
			memcpy(nType, menu_edit_text_value, strlen(menu_edit_text_value) + 1);
			entry->type = nType;
		}
	}else{
		char* key = entry->optionsKeys->base[menu_edit_index - 1];
		char* value = entry->optionsValues->base[menu_edit_index - 1];
		if(strcmp(key, menu_edit_text_key) != 0){
			kfree(key, strlen(key) + 1);
			char* nType = kmalloc(strlen(menu_edit_text_key) + 1);
			memcpy(nType, menu_edit_text_key, strlen(menu_edit_text_key) + 1);
			entry->optionsKeys->base[menu_edit_index - 1] = nType;
		}
		if(strcmp(value, menu_edit_text_value) != 0){
			kfree(value, strlen(value) + 1);
			char* nType = kmalloc(strlen(menu_edit_text_value) + 1);
			memcpy(nType, menu_edit_text_value, strlen(menu_edit_text_value) + 1);
			entry->optionsValues->base[menu_edit_index - 1] = nType;
		}
	}
}

void menu_error_popup(status_t err){
	menu_state = MENU_STATE_ERROR;
	kernel_print_error_trace();
	size_t cols, rows;
	stdio64_get_text_size(&rows, &cols);
	for(size_t x = cols / 4; x < cols * 3 / 4; x++){
		for(size_t y = rows / 4; y < rows * 3 / 4; y++){
			printCharAt(0x20, 0x40, x, y);
		}
	}
	char* errstr = kmalloc(cols);
	snprintf(errstr, cols, "Error %u", err);
	char* errdesc = kmalloc(cols);
	snprintf(errdesc, cols, "(%s)", errcode_get_name(err));
	printAt(errstr, 0x4f, cols / 2 - strlen(errstr) / 2, rows / 2 - 1);
	printAt(errdesc, 0x4f, cols / 2 - strlen(errdesc) / 2, rows / 2);
	kfree(errstr, cols);
	kfree(errdesc, cols);
}


void menu_switch_state(uint8_t state){
	menu_state = state;
	clearScreen(0x7);
	setCursorPosition(0, out_rows - 1);
	if(state == MENU_STATE_EDIT_OPTION){
		parse_entry* entry = &parse_get_data()->entries[menu_selected];
		if(menu_edit_index == 0){
			memcpy(menu_edit_text_key, "type", 5);
			memcpy(menu_edit_text_value, entry->type, strlen(entry->type) + 1);
		}else{
			char* key = (char*) entry->optionsKeys->base[menu_edit_index - 1];
			char* value = (char*) entry->optionsValues->base[menu_edit_index - 1];
			size_t keyLen = strlen(key) + 1;
			size_t valLen = strlen(value) + 1;
			if(keyLen > MENU_MAX_KEY_LENGTH){
				log_error("Configuration key is too long (%u > %u)\n", keyLen, MENU_MAX_KEY_LENGTH);
				menu_error_popup(TSX_TOO_LARGE);
				return;
			}
			if(valLen > MENU_MAX_VALUE_LENGTH){
				log_error("Configuration value is too long (%u > %u)\n", valLen, MENU_MAX_VALUE_LENGTH);
				menu_error_popup(TSX_TOO_LARGE);
				return;
			}
			memcpy(menu_edit_text_key, key, keyLen);
			memcpy(menu_edit_text_value, value, valLen);
		}
		menu_edit_editing_key = TRUE;
	}
}

uint8_t menu_key(uint16_t key){
	menu_autoboot = FALSE;
	if(menu_state == MENU_STATE_MAIN){
		if(key == KEY_ENTER){
			if(!parse_get_data()->entries[menu_selected].name)
				return 0;
			status_t status = parse_file_entry(menu_selected);
			if(status != TSX_SUCCESS){
				menu_error_popup(status);
			}else{
				return 2;
			}
		}else if(key == KEY_P_8){
			if(menu_selected > 0)
				menu_selected--;
		}else if(key == KEY_P_2){
			if(menu_selected < PARSE_MAX_ENTRIES - 1 && parse_get_data()->entries[menu_selected].name != 0)
				menu_selected++;
		}else if(key == KEY_C){
			return 1;
		}else if(key == KEY_E){
			parse_entry* entry = &parse_get_data()->entries[menu_selected];
			if(!entry->name){
				entry->name = "Temporary Entry";
				entry->conf_start = 0;
				entry->parsed = TRUE;
				entry->type = "";
				entry->optionsKeys = list_array_create(0);
				entry->optionsValues = list_array_create(0);
				if(!entry->optionsKeys || !entry->optionsValues){
					log_fatal("Out of memory");
					return 0;
				}
			}
			menu_edit_index = 0;
			menu_switch_state(MENU_STATE_EDIT_OVERVIEW);
		}
	}else if(menu_state == MENU_STATE_EDIT_OVERVIEW){
		if(key == KEY_ESCAPE){
			menu_switch_state(MENU_STATE_MAIN);
		}else if(key == KEY_P_8){
			if(menu_edit_index > 0)
				menu_edit_index--;
		}else if(key == KEY_P_2){
			if(menu_edit_index < parse_get_data()->entries[menu_selected].optionsKeys->length + 1)
				menu_edit_index++;
		}else if(key == KEY_P_4){
			if(menu_edit_value_offset > 6)
				menu_edit_value_offset--;
		}else if(key == KEY_P_6){
			if(menu_edit_value_offset < out_cols - 7)
				menu_edit_value_offset++;
		}else if(key == KEY_ENTER){
			parse_entry* entry = &parse_get_data()->entries[menu_selected];
			if(menu_edit_index > entry->optionsKeys->length){
				char* nKey = kmalloc(1);
				char* nVal = kmalloc(1);
				nKey[0] = 0;
				nVal[0] = 0;
				list_array_push(entry->optionsKeys, nKey);
				list_array_push(entry->optionsValues, nVal);
			}
			menu_switch_state(MENU_STATE_EDIT_OPTION);
		}
	}else if(menu_state == MENU_STATE_EDIT_OPTION){
		if(key == KEY_ESCAPE){
			menu_switch_state(MENU_STATE_EDIT_OVERVIEW);
		}else if(key == KEY_ENTER){
			menu_edit_option_save();
			menu_switch_state(MENU_STATE_EDIT_OVERVIEW);
		}else if(key == KEY_P_8){
			if(!menu_edit_editing_key){
				menu_edit_editing_key = TRUE;
			}
		}else if(key == KEY_P_2){
			if(menu_edit_editing_key){
				menu_edit_editing_key = FALSE;
			}
		}else{
			size_t keyLen = strlen(menu_edit_text_key);
			size_t valLen = strlen(menu_edit_text_value);
			if(menu_edit_editing_key && menu_edit_index != 0){
				if(key == KEY_BACKSPACE){
					if(keyLen > 0){
						menu_edit_text_key[keyLen - 1] = 0;
					}
				}else{
					if(keyLen < MENU_MAX_KEY_LENGTH - 1){
						menu_edit_text_key[keyLen] = (char) key;
						menu_edit_text_key[keyLen + 1] = 0;
					}
				}
			}else if(!menu_edit_editing_key){
				if(key == KEY_BACKSPACE){
					if(valLen > 0){
						menu_edit_text_value[valLen - 1] = 0;
					}
				}else{
					if(valLen < MENU_MAX_VALUE_LENGTH - 1){
						menu_edit_text_value[valLen] = (char) key;
						menu_edit_text_value[valLen + 1] = 0;
					}
				}
			}
		}
	}else if(menu_state == MENU_STATE_ERROR){
		menu_switch_state(MENU_STATE_MAIN);
	}
	return 0;
}

bool menu_boot(){
	if(menu_counter > 0 && menu_autoboot)
		menu_counter--;
	if(menu_counter == 0 && menu_autoboot)
		return TRUE;
	return FALSE;
}


void menu_stop_autoboot(){
	menu_autoboot = FALSE;
}

uint8_t* menu_selected_ptr(){
	return &menu_selected;
}
