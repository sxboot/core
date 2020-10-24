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
 * parse.c - Configuration file parser.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/util.h>
#include <kernel/parse.h>
#include <kernel/stdio64.h>
#include <kernel/log.h>
#include <kernel/list.h>

static parse_entry parse_entries[PARSE_MAX_ENTRIES];
static parse_data_t parse_data;

static const char* parse_pref = "[parse]";

status_t parse_file(char* file){
	status_t status = 0;
	memset(&parse_data, 0, sizeof(parse_data_t));
	parse_data.entries = parse_entries;
	parse_data.timeout = 10; // default value
	parse_data.serialBaud = 9600;
	reloc_ptr((void**) &parse_data.entries);
	memset(parse_entries, 0, sizeof(parse_entries));

	size_t fileSize = strlen(file);
	uint8_t entries = 0;
	uint8_t cEntry = -1;
	uint8_t sub = 0;

	updateLoadingWheel();
	for(int i = 0; i < fileSize; i++){
		char c = *(file + i);
		if(c == 0xa || c == 0xd || c == ';')
			*(file + i) = 0;
	}

	for(int i = 0; i < fileSize; i++){
		updateLoadingWheel();
		char c = *(file + i);
		if(c <= 0x20)
			continue;
		if(c == '#'){
			i += strlen(file + i) - 1;
			continue;
		}
		if(entries >= PARSE_MAX_ENTRIES)
			break;
		if(cEntry == 255){
			if(util_str_startsWith(file + i, "entry")){
				cEntry = entries;
				entries++;
				parse_entries[cEntry].conf_start = file + i;
				reloc_ptr((void**) &(parse_entries[cEntry].name));
				reloc_ptr((void**) &(parse_entries[cEntry].conf_start));
				reloc_ptr((void**) &(parse_entries[cEntry].optionsKeys));
				reloc_ptr((void**) &(parse_entries[cEntry].optionsValues));
				while(*(file + i) != '"' && *(file + i) != 0)
					i++;
				if(*(file + i) == 0){
					goto unexpectedEnd;
				}else{
					i++;
					parse_entries[cEntry].name = file + i;
					while(*(file + i) != '"' && *(file + i) != '{' && *(file + i) != 0)
						i++;
					if(*(file + i) == 0 || *(file + i) == '{')
						goto unexpectedEnd;
					*(file + i) = 0;
					i += util_str_length_c_max(file + i, '{', 64);
				}
			}else if(util_str_startsWith(file + i, "timeout")){
				i += util_str_length_c_max(file + i, '=', 64) + 1;
				while(*(file + i) == ' ')
					i++;
				size_t num = util_str_to_int(file + i);
				if(num == SIZE_MAX){
					printNlnr();
					log_error("%s Invalid number: %s\n", parse_pref, file + i);
					FERROR(TSX_PARSE_ERROR);
				}
				parse_data.timeout = num;
				i += util_str_length(file + i);
			}else if(util_str_startsWith(file + i, "serialBaud")){
				i += util_str_length_c_max(file + i, '=', 64) + 1;
				while(*(file + i) == ' ')
					i++;
				size_t num = util_str_to_int(file + i);
				if(num == SIZE_MAX){
					printNlnr();
					log_error("%s Invalid number: %s\n", parse_pref, file + i);
					FERROR(TSX_PARSE_ERROR);
				}
				parse_data.serialBaud = num;
				i += util_str_length(file + i);
			}else if(util_str_startsWith(file + i, "hdDrivers")){
				i += util_str_length_c_max(file + i, '=', 64) + 1;
				while(*(file + i) == ' ')
					i++;
				parse_data.hdDriverOverride = file + i;
				i += util_str_length(file + i);
			}else if(util_str_startsWith(file + i, "fsDrivers")){
				i += util_str_length_c_max(file + i, '=', 64) + 1;
				while(*(file + i) == ' ')
					i++;
				parse_data.fsDriverOverride = file + i;
				i += util_str_length(file + i);
			}else{
				printNlnr();
				log_error("%s Unexpected token: %s\n", parse_pref, file + i);
				FERROR(TSX_PARSE_ERROR);
			}
		}else{
			if(c == '{'){
				sub++;
			}else if(c == '}'){
				if(sub > 0)
					sub--;
				else
					cEntry = -1;
			}
		}
	}
	parse_data.entryCount = entries;
	_end:
	return status;
	unexpectedEnd:
	printNlnr();
	log_error("%s Unexpected end of input\n", parse_pref);
	FERROR(TSX_PARSE_ERROR);
}

status_t parse_file_entry(uint8_t index){
	status_t status = 0;
	parse_entry* entry = &parse_entries[index];
	char* addr = entry->conf_start;
	updateLoadingWheel();
	if(!util_str_startsWith(addr, "entry"))
		FERROR(TSX_PARSE_ERROR);
	addr += util_str_length_c_max(addr, '{', 64) + 1;

	entry->optionsKeys = list_array_create(0);
	entry->optionsValues = list_array_create(0);
	if(!entry->optionsKeys || !entry->optionsValues)
		FERROR(TSX_OUT_OF_MEMORY);

	for(int i = 0;; i++){
		updateLoadingWheel();
		char c = *(addr + i);
		if(c <= 0x20)
			continue;
		if(c == '#'){
			i += strlen(addr + i) - 1;
			continue;
		}
		if(c == '}'){
			break;
		}else if(util_str_startsWith(addr + i, "type")){
			i += util_str_length_c_max(addr + i, '=', 64) + 1;
			while(*(addr + i) == ' ')
				i++;
			entry->type = addr + i;
			i += util_str_length(addr + i);
		}else{
			char* key = addr + i;
			i += util_str_length_c_max(addr + i, '=', 64) + 1;
			*(addr + i - 1) = 0;
			for(size_t j = i - 2; j > 0 && *(addr + j) == ' '; j--)
				*(addr + j) = 0;
			while(*(addr + i) == ' ')
				i++;
			list_array_push(entry->optionsKeys, key);
			list_array_push(entry->optionsValues, addr + i);
			i += util_str_length(addr + i);
		}
	}

	if(entry->type == 0){
		printNlnr();
		log_error("%s Entry '%s': no type given\n", parse_pref, entry->name);
		FERROR(TSX_PARSE_ERROR);
	}
	_end:
	return status;
}

char* parse_get_option(parse_entry* entry, char* key){
	for(size_t i = 0; i < entry->optionsKeys->length; i++){
		if(util_str_equals(entry->optionsKeys->base[i], key)){
			return entry->optionsValues->base[i];
		}
	}
	return NULL;
}

parse_data_t* parse_get_data(){
	return &parse_data;
}
