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

#ifndef __KERNEL_PE_H__
#define __KERNEL_PE_H__

#include <klibc/stdint.h>
#include <klibc/stdlib.h>
#include <klibc/stdbool.h>



#define PE_OPT_MAGIC_PE32 0x010b
#define PE_OPT_MAGIC_PE64 0x020b

#define PE_MACHINE_i386 0x014c
#define PE_MACHINE_AMD64 0x8664

#pragma pack(push,1)
typedef struct mz_file{
	uint16_t signature;
	uint16_t extraBytes;
	uint16_t pages;
	uint16_t relocationItems;
	uint16_t headerSize;
	uint16_t minParagraphs;
	uint16_t reqParagraphs;
	uint16_t initSS;
	uint16_t initSP;
	uint16_t checksum;
	uint16_t initIP;
	uint16_t initCS;
	uint16_t relocationTableOffset;
	uint16_t overlay;
	uint8_t reserved[32];
	uint32_t peHeaderOffset;
} mz_file;

typedef struct pe_default_header{
	uint8_t p_magic[4];
	uint16_t p_machine;
	uint16_t p_sections;
	uint32_t p_timestamp;
	uint32_t p_symtab;
	uint32_t p_symtablen;
	uint16_t p_optionalsz;
	uint16_t p_characteristics;
} pe_default_header;

typedef struct pe_file{
	uint8_t p_magic[4];
	uint16_t p_machine;
	uint16_t p_sections;
	uint32_t p_timestamp;
	uint32_t p_symtab;
	uint32_t p_symtablen;
	uint16_t p_optionalsz;
	uint16_t p_characteristics;
	uint16_t po_magic;
	uint8_t po_linkerMajor;
	uint8_t po_linkerMinor;
	uint32_t po_codesz;
	uint32_t po_datasz;
	uint32_t po_udatasz;
	uint32_t po_entry;
	uint32_t po_codeoff;
	// uint32_t po_dataoff; nonexistent in 64bit
	uint64_t po_imageoff;
	uint32_t po_sectionAlign;
	uint32_t po_fileAlign;
	uint16_t po_osMajor;
	uint16_t po_osMinor;
	uint16_t po_imageMajor;
	uint16_t po_imageMinor;
	uint16_t po_subMajor;
	uint16_t po_subMinor;
	uint32_t po_win32Version;
	uint32_t po_imagesz;
	uint32_t po_headersz;
	uint32_t po_checksum;
	uint16_t po_subsystem;
	uint16_t po_dll_characteristics;
	uint32_t po_stackReserve;
	uint32_t po_stackCommit;
	uint32_t po_heapReserve;
	uint32_t po_heapCommit;
	uint32_t po_loaderFlags;
	uint32_t po_num;
} pe_file;

typedef struct pe_data_dir{
	uint32_t pdd_rva;
	uint32_t pdd_size;
} pe_data_dir;

typedef struct pe_section_header{
	char ps_name[8];
	uint32_t ps_vsize;
	uint32_t ps_vaddr;
	uint32_t ps_rawsize;
	uint32_t ps_fileoff;
	uint32_t ps_relocs;
	uint32_t ps_lines;
	uint16_t ps_relocsNum;
	uint16_t ps_linesNum;
	uint32_t ps_characteristics;
} pe_section_header;
#pragma pack(pop)


bool mz_is_mz(mz_file* file);
pe_file* mz_get_pe(mz_file* file);

pe_section_header* pe_get_sections(pe_file* file);
pe_section_header* pe_get_section(pe_file* file, char* name);
bool pe_is_pe(pe_file* file);
size_t pe_get_required_memory(pe_file* file);
size_t pe_get_memory_base(pe_file* file);
status_t pe_load_file(pe_file* file);
status_t pe_load_file_s(pe_file* file, size_t location);


#endif /* __KERNEL_PE_H__ */
