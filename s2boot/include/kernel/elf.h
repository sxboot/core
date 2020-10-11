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

#ifndef __KERNEL_ELF_H__
#define __KERNEL_ELF_H__

#include <klibc/stdint.h>
#include <klibc/stdlib.h>
#include <klibc/stdbool.h>


#define ELF_CLASS_32BIT 1
#define ELF_CLASS_64BIT 2
#if ARCH_BITS == 32
#define ELF_CLASS_nBIT ELF_CLASS_32BIT
#elif ARCH_BITS == 64
#define ELF_CLASS_nBIT ELF_CLASS_64BIT
#endif

#define ELF_MACHINE_i386 0x03
#define ELF_MACHINE_AMD64 0x3e

#define ELF_ET_NONE 0
#define ELF_ET_REL 1
#define ELF_ET_EXEC 2
#define ELF_ET_DYN 3
#define ELF_ET_CORE 4
#define ELF_ET_LOOS 0xfe00
#define ELF_ET_HIOS 0xfeff
#define ELF_ET_LOPROC 0xff00
#define ELF_ET_HIPROC 0xffff

#define ELF_PH_TYPE_LOAD 1
#define ELF_PH_TYPE_DYNAMIC 2
#define ELF_PH_TYPE_INTERP 3
#define ELF_PH_TYPE_NOTE 4
#define ELF_PH_TYPE_SHLIB 5
#define ELF_PH_TYPE_PHDR 6
#define ELF_PH_TYPE_TLS 7

#pragma pack(push,1)
typedef struct elf64_file{
	char ei_mag[4];
	uint8_t ei_class;
	uint8_t ei_data;
	uint8_t ei_version;
	uint8_t ei_osabi;
	uint8_t ei_abiversion;
	uint8_t ei_pad[7];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} elf64_file;

typedef struct elf64_ph{
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} elf64_ph;

typedef struct elf64_sh{
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
} elf64_sh;

typedef struct elf64_symtab{
	uint32_t st_name;
	uint8_t st_info;
	uint8_t st_other;
	uint16_t st_shndx;
	uint64_t st_value;
	uint64_t st_size;
} elf64_symtab;

typedef struct elf32_file{
	char ei_mag[4];
	uint8_t ei_class;
	uint8_t ei_data;
	uint8_t ei_version;
	uint8_t ei_osabi;
	uint8_t ei_abiversion;
	uint8_t ei_pad[7];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} elf32_file;

typedef struct elf32_ph{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} elf32_ph;

typedef struct elf32_sh{
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} elf32_sh;

typedef struct elf32_symtab{
	uint32_t st_name;
	uint32_t st_value;
	uint32_t st_size;
	uint8_t st_info;
	uint8_t st_other;
	uint16_t st_shndx;
} elf32_symtab;


#define ELF_LOADED_IMAGE_SECTION_COUNT 8

#define ELF_LI_SECTION_SYMTAB 1
#define ELF_LI_SECTION_STRTAB 2
#define ELF_LI_SECTION_DYNSYM 3
#define ELF_LI_SECTION_DYNSTR 4
#define ELF_LI_SECTION_RELAPLT 5
#define ELF_LI_SECTION_RELADYN 6
#define ELF_LI_SECTION_RELPLT 7
#define ELF_LI_SECTION_RELDYN 8

typedef struct elf_loaded_image_section{
	size_t type;
	void* location;
	size_t length;
	size_t entsize;
} elf_loaded_image_section;

typedef struct elf_loaded_image{
	size_t loadLocation;

	elf_loaded_image_section sections[ELF_LOADED_IMAGE_SECTION_COUNT];
} elf_loaded_image;
#pragma pack(pop)


#if ARCH_BITS == 64
#define elf_file elf64_file
#define elf_ph elf64_ph
#define elf_sh elf64_sh
#define elf_symtab elf64_symtab

#define elf_get_ph elf64_get_ph
#define elf_get_sh elf64_get_sh
#define elf_get_shstrtab elf64_get_shstrtab
#define elf_get_sh_entry elf64_get_sh_entry
#define elf_get_strtab elf64_get_strtab
#define elf_get_symtab_entry elf64_get_symtab_entry
#define elf_is_elf elf64_is_elf
#define elf_get_required_memory elf64_get_required_memory
#define elf_get_memory_base elf64_get_memory_base
#define elf_load_file elf64_load_file
#define elf_load_library elf64_load_library

#elif ARCH_BITS == 32
#define elf_file elf32_file
#define elf_ph elf32_ph
#define elf_sh elf32_sh
#define elf_symtab elf32_symtab

#define elf_get_ph elf32_get_ph
#define elf_get_sh elf32_get_sh
#define elf_get_shstrtab elf32_get_shstrtab
#define elf_get_sh_entry elf32_get_sh_entry
#define elf_get_strtab elf32_get_strtab
#define elf_get_symtab_entry elf32_get_symtab_entry
#define elf_is_elf elf32_is_elf
#define elf_get_required_memory elf32_get_required_memory
#define elf_get_memory_base elf32_get_memory_base
#define elf_load_file elf32_load_file
#define elf_load_library elf32_load_library

#endif


elf64_ph* elf64_get_ph(elf64_file* file);
elf64_sh* elf64_get_sh(elf64_file* file);
char* elf64_get_shstrtab(elf64_file* file);
elf64_sh* elf64_get_sh_entry(elf64_file* file, char* name);
char* elf64_get_strtab(elf64_file* file);
elf64_symtab* elf64_get_symtab_entry(elf64_file* file, char* name);
bool elf64_is_elf(elf64_file* file);
size_t elf64_get_required_memory(elf64_file*);
size_t elf64_get_memory_base(elf64_file* file);
status_t elf64_load_file(elf64_file* file);
status_t elf64_load_library(elf64_file* file, size_t location);


elf32_ph* elf32_get_ph(elf32_file* file);
elf32_sh* elf32_get_sh(elf32_file* file);
char* elf32_get_shstrtab(elf32_file* file);
elf32_sh* elf32_get_sh_entry(elf32_file* file, char* name);
char* elf32_get_strtab(elf32_file* file);
elf32_symtab* elf32_get_symtab_entry(elf32_file* file, char* name);
bool elf32_is_elf(elf32_file* file);
size_t elf32_get_required_memory(elf32_file* file);
size_t elf32_get_memory_base(elf32_file* file);
status_t elf32_load_file(elf32_file* file);
status_t elf32_load_library(elf32_file* file, size_t location);


elf_symtab* elf_get_symtab_entry_from_image(elf_loaded_image* image, char* name);
void elf_gen_loaded_image_data(elf_file* file, size_t loadLocation, elf_loaded_image* dest);
elf_loaded_image_section* elf_get_section_from_image(elf_loaded_image* image, size_t type);


#endif /* __KERNEL_ELF_H__ */
