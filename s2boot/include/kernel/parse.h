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

#ifndef __KERNEL_PARSE_H__
#define __KERNEL_PARSE_H__

#include <kernel/list.h>


#define PARSE_MAX_ENTRIES 16

#define boot_type_t char*

typedef struct parse_entry{
	char* name;
	char* conf_start;
	bool parsed;
	boot_type_t type;
	list_array* optionsKeys;
	list_array* optionsValues;
} parse_entry;

typedef struct parse_data{
	parse_entry* entries;
	size_t entryCount;
	size_t timeout;
	uint32_t serialBaud;
	size_t fontScale;
	char* hdDriverOverride;
	char* fsDriverOverride;
} parse_data_t;

status_t parse_file(char* file);
status_t parse_file_entry(uint8_t index);
char* parse_get_option(parse_entry* entry, char* key);
parse_data_t* parse_get_data();


#endif /* __KERNEL_PARSE_H__ */
