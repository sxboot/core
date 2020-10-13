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


#define PARSE_MAX_ENTRIES 16

/*
#define PARSE_ENTRY_TYPE_CHAIN 1
#define PARSE_ENTRY_TYPE_MBR 2
#define PARSE_ENTRY_TYPE_BINARY 3
#define PARSE_ENTRY_TYPE_IMAGE 4
#define PARSE_ENTRY_TYPE_UEFI 5
*/

#define boot_type_t char*

typedef struct parse_entry{
	char* name;
	char* conf_start;
	boot_type_t type;
	char* drive;
	uint8_t partition;
	char* file;
	uint64_t destination;
	uint64_t offset;
	uint8_t bits;
} parse_entry;

typedef struct parse_data{
	parse_entry* entries;
	uint64_t timeout;
	uint32_t serialBaud;
	char* hdDriverOverride;
	char* fsDriverOverride;
} parse_data_t;

status_t parse_file(char* file);
status_t parse_file_entry(uint8_t index);
parse_data_t* parse_get_data();


#endif /* __KERNEL_PARSE_H__ */
