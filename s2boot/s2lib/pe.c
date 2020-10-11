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
 * pe.c - Utility functions for Portable Executable (PE) files.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/errc.h>
#include <kernel/util.h>
#include <kernel/pe.h>

static uint16_t mz_signature = 0x5a4d;
static uint8_t pe_signature[4] = {0x50, 0x45, 0, 0};


bool mz_is_mz(mz_file* file){
	return file->signature == mz_signature;
}

pe_file* mz_get_pe(mz_file* file){
	if(!mz_is_mz(file))
		return NULL;
	return (pe_file*) ((size_t) file + file->peHeaderOffset);
}


pe_section_header* pe_get_sections(pe_file* file){
	if(!pe_is_pe(file))
		return 0;
	return (pe_section_header*) ((size_t) file + sizeof(pe_default_header) + file->p_optionalsz);
}

pe_section_header* pe_get_section(pe_file* file, char* name){
	size_t namelen = strlen(name);
	pe_section_header* sections = pe_get_sections(file);
	if(!sections)
		return NULL;
	for(size_t i = 0; i < file->p_sections; i++){
		if(memcmp(name, sections[i].ps_name, MIN(namelen, 8)) == 0){
			return &sections[i];
		}
	}
	return NULL;
}

bool pe_is_pe(pe_file* file){
	for(int i = 0; i < 4; i++){
		if(file->p_magic[i] != pe_signature[i])
			return false;
	}
	return true;
}

size_t pe_get_required_memory(pe_file* file){
	if(!pe_is_pe(file))
		return 0;
	pe_section_header* secs = pe_get_sections(file);
	if(secs == 0)
		return 0;
	size_t min = SIZE_MAX;
	size_t max = 0;
	for(int i = 0; i < file->p_sections; i++){
		if(secs[i].ps_vaddr < min)
			min = secs[i].ps_vaddr;
		if(secs[i].ps_vaddr + secs[i].ps_vsize > max)
			max = secs[i].ps_vaddr + secs[i].ps_vsize;
	}
	return max - min;
}

size_t pe_get_memory_base(pe_file* file){
	if(!pe_is_pe(file))
		return SIZE_MAX;
	pe_section_header* secs = pe_get_sections(file);
	if(secs == 0)
		return SIZE_MAX;
	size_t min = SIZE_MAX;
	for(int i = 0; i < file->p_sections; i++){
		if(secs[i].ps_vaddr < min)
			min = secs[i].ps_vaddr;
	}
	return min;
}

status_t pe_load_file(pe_file* file){
	if(!pe_is_pe(file))
		return TSX_INVALID_FORMAT;
	pe_section_header* secs = pe_get_sections(file);
	if(secs == 0)
		return TSX_INVALID_FORMAT;
	for(int i = 0; i < file->p_sections; i++){
		memset((void*) (size_t) (secs[i].ps_vaddr), 0, secs[i].ps_vsize);
		memcpy((void*) (size_t) (secs[i].ps_vaddr), (void*) (secs[i].ps_fileoff + (size_t) file), MIN(secs[i].ps_rawsize, secs[i].ps_vsize));
	}
	return TSX_SUCCESS;
}

status_t pe_load_file_s(pe_file* file, size_t location){
	if(!pe_is_pe(file))
		return TSX_INVALID_FORMAT;
	pe_section_header* secs = pe_get_sections(file);
	if(secs == 0)
		return TSX_INVALID_FORMAT;
	size_t imageBase = pe_get_memory_base(file);
	if(imageBase == SIZE_MAX)
		return 1;
	for(int i = 0; i < file->p_sections; i++){
		memset((void*) (secs[i].ps_vaddr + location - imageBase), 0, secs[i].ps_vsize);
		memcpy((void*) (secs[i].ps_vaddr + location - imageBase), (void*) (secs[i].ps_fileoff + (size_t) file), MIN(secs[i].ps_rawsize, secs[i].ps_vsize));
	}
	return TSX_SUCCESS;
}


