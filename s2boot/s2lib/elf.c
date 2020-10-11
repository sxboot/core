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
 * elf.c - Utility functions for Executable and Linkable Format (ELF) files.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/util.h>
#include <kernel/elf.h>

static uint8_t elf_signature[4] = {0x7f, 0x45, 0x4c, 0x46};


elf64_ph* elf64_get_ph(elf64_file* file){
	if(!elf64_is_elf(file))
		return 0;
	return (elf64_ph*) (file->e_phoff + (size_t) file);
}

elf64_sh* elf64_get_sh(elf64_file* file){
	if(!elf64_is_elf(file))
		return 0;
	return (elf64_sh*) (file->e_shoff + (size_t) file);
}

char* elf64_get_shstrtab(elf64_file* file){
	elf64_sh* sht = elf64_get_sh(file);
	if(sht == 0)
		return 0;
	return (char*) (sht[file->e_shstrndx].sh_offset + (size_t) file);
}

elf64_sh* elf64_get_sh_entry(elf64_file* file, char* name){
	elf64_sh* sht = elf64_get_sh(file);
	if(sht == 0)
		return 0;
	char* shstrtab = elf64_get_shstrtab(file);
	if(sht == 0)
		return 0;
	for(int i = 0; i < file->e_shnum; i++){
		if(util_str_equals(shstrtab + sht[i].sh_name, name)){
			return &sht[i];
		}
	}
	return 0;
}

char* elf64_get_strtab(elf64_file* file){
	elf64_sh* strtabsec = elf64_get_sh_entry(file, ".strtab");
	if(strtabsec == 0)
		return 0;
	return (char*) (strtabsec->sh_offset + (size_t) file);
}

elf64_symtab* elf64_get_symtab_entry(elf64_file* file, char* name){
	elf64_sh* symtabsec = elf64_get_sh_entry(file, ".symtab");
	if(symtabsec == 0)
		return 0;
	elf64_symtab* symtab = (elf64_symtab*) (symtabsec->sh_offset + (size_t) file);
	char* strtab = elf64_get_strtab(file);
	if(symtab == 0 || strtab == 0)
		return 0;
	for(int i = 0; i < (size_t) symtabsec->sh_size / (size_t) (symtabsec->sh_entsize > 0 ? symtabsec->sh_entsize : sizeof(elf64_symtab)); i++){
		if(util_str_equals(strtab + symtab[i].st_name, name)){
			return &symtab[i];
		}
	}
	return 0;
}

bool elf64_is_elf(elf64_file* file){
	for(int i = 0; i < 4; i++){
		if(file->ei_mag[i] != elf_signature[i])
			return false;
	}
	return true;
}

size_t elf64_get_required_memory(elf64_file* file){
	if(!elf64_is_elf(file))
		return 0;
	elf64_ph* ph = elf64_get_ph(file);
	if(ph == 0)
		return 0;
	if(file->e_phentsize != sizeof(elf64_ph))
		return 0;
	size_t min = SIZE_MAX;
	size_t max = 0;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		if(ph[i].p_vaddr < min)
			min = ph[i].p_vaddr;
		if(ph[i].p_vaddr + ph[i].p_memsz > max)
			max = ph[i].p_vaddr + ph[i].p_memsz;
	}
	if(min == SIZE_MAX)
		return 0;
	return max - min;
}

size_t elf64_get_memory_base(elf64_file* file){
	if(!elf64_is_elf(file))
		return SIZE_MAX;
	elf64_ph* ph = elf64_get_ph(file);
	if(ph == 0)
		return SIZE_MAX;
	if(file->e_phentsize != sizeof(elf64_ph))
		return SIZE_MAX;
	size_t min = SIZE_MAX;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		if(ph[i].p_vaddr < min)
			min = ph[i].p_vaddr;
	}
	return min;
}

status_t elf64_load_file(elf64_file* file){
	if(!elf64_is_elf(file))
		return 25;
	elf64_ph* ph = elf64_get_ph(file);
	if(ph == 0)
		return 25;
	if(file->e_phentsize != sizeof(elf64_ph))
		return 25;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		memset((void*) (ph[i].p_vaddr), 0, ph[i].p_memsz);
		memcpy((void*) (ph[i].p_vaddr), (void*) (ph[i].p_offset + (size_t) file), ph[i].p_filesz);
	}
	return 0;
}

status_t elf64_load_library(elf64_file* file, size_t location){
	if(!elf64_is_elf(file))
		return 25;
	elf64_ph* ph = elf64_get_ph(file);
	if(ph == 0)
		return 25;
	if(file->e_phentsize != sizeof(elf64_ph))
		return 25;
	size_t imageBase = elf64_get_memory_base(file);
	if(imageBase == SIZE_MAX)
		return 1;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		memset((void*) (ph[i].p_vaddr + location - imageBase), 0, ph[i].p_memsz);
		memcpy((void*) (ph[i].p_vaddr + location - imageBase), (void*) (ph[i].p_offset + (size_t) file), ph[i].p_filesz);
	}
	return 0;
}




elf32_ph* elf32_get_ph(elf32_file* file){
	if(!elf32_is_elf(file))
		return 0;
	return (elf32_ph*) (file->e_phoff + (size_t) file);
}

elf32_sh* elf32_get_sh(elf32_file* file){
	if(!elf32_is_elf(file))
		return 0;
	return (elf32_sh*) (file->e_shoff + (size_t) file);
}

char* elf32_get_shstrtab(elf32_file* file){
	elf32_sh* sht = elf32_get_sh(file);
	if(sht == 0)
		return 0;
	return (char*) (sht[file->e_shstrndx].sh_offset + (size_t) file);
}

elf32_sh* elf32_get_sh_entry(elf32_file* file, char* name){
	elf32_sh* sht = elf32_get_sh(file);
	if(sht == 0)
		return 0;
	char* shstrtab = elf32_get_shstrtab(file);
	if(sht == 0)
		return 0;
	for(int i = 0; i < file->e_shnum; i++){
		if(util_str_equals(shstrtab + sht[i].sh_name, name)){
			return &sht[i];
		}
	}
	return 0;
}

char* elf32_get_strtab(elf32_file* file){
	elf32_sh* strtabsec = elf32_get_sh_entry(file, ".strtab");
	if(strtabsec == 0)
		return 0;
	return (char*) (strtabsec->sh_offset + (size_t) file);
}

elf32_symtab* elf32_get_symtab_entry(elf32_file* file, char* name){
	elf32_sh* symtabsec = elf32_get_sh_entry(file, ".symtab");
	if(symtabsec == 0)
		return 0;
	elf32_symtab* symtab = (elf32_symtab*) (symtabsec->sh_offset + (size_t) file);
	char* strtab = elf32_get_strtab(file);
	if(symtab == 0 || strtab == 0)
		return 0;
	for(int i = 0; i < symtabsec->sh_size / (symtabsec->sh_entsize > 0 ? symtabsec->sh_entsize : 16); i++){
		if(util_str_equals(strtab + symtab[i].st_name, name)){
			return &symtab[i];
		}
	}
	return 0;
}

bool elf32_is_elf(elf32_file* file){
	for(int i = 0; i < 4; i++){
		if(file->ei_mag[i] != elf_signature[i])
			return false;
	}
	return true;
}

size_t elf32_get_required_memory(elf32_file* file){
	if(!elf32_is_elf(file))
		return 0;
	elf32_ph* ph = elf32_get_ph(file);
	if(ph == 0)
		return 0;
	if(file->e_phentsize != sizeof(elf32_ph))
		return 0;
	size_t min = SIZE_MAX;
	size_t max = 0;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		if(ph[i].p_vaddr < min)
			min = ph[i].p_vaddr;
		if(ph[i].p_vaddr + ph[i].p_memsz > max)
			max = ph[i].p_vaddr + ph[i].p_memsz;
	}
	if(min == SIZE_MAX)
		return 0;
	return max - min;
}

size_t elf32_get_memory_base(elf32_file* file){
	if(!elf32_is_elf(file))
		return SIZE_MAX;
	elf32_ph* ph = elf32_get_ph(file);
	if(ph == 0)
		return SIZE_MAX;
	if(file->e_phentsize != sizeof(elf32_ph))
		return SIZE_MAX;
	size_t min = SIZE_MAX;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		if(ph[i].p_vaddr < min)
			min = ph[i].p_vaddr;
	}
	return min;
}

status_t elf32_load_file(elf32_file* file){
	if(!elf32_is_elf(file))
		return 25;
	elf32_ph* ph = elf32_get_ph(file);
	if(ph == 0)
		return 25;
	if(file->e_phentsize != sizeof(elf32_ph))
		return 25;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		memset((void*) (size_t) (ph[i].p_vaddr), 0, ph[i].p_memsz);
		memcpy((void*) (size_t) (ph[i].p_vaddr), (void*) (ph[i].p_offset + (size_t) file), ph[i].p_filesz);
	}
	return 0;
}

status_t elf32_load_library(elf32_file* file, size_t location){
	if(!elf32_is_elf(file))
		return 25;
	elf32_ph* ph = elf32_get_ph(file);
	if(ph == 0)
		return 25;
	if(file->e_phentsize != sizeof(elf32_ph))
		return 25;
	size_t imageBase = elf32_get_memory_base(file);
	if(imageBase == SIZE_MAX)
		return 1;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != ELF_PH_TYPE_LOAD)
			continue;
		memset((void*) (ph[i].p_vaddr + location - imageBase), 0, ph[i].p_memsz);
		memcpy((void*) (ph[i].p_vaddr + location - imageBase), (void*) (ph[i].p_offset + (size_t) file), ph[i].p_filesz);
	}
	return 0;
}





elf_symtab* elf_get_symtab_entry_from_image(elf_loaded_image* image, char* name){
	elf_loaded_image_section* symtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_SYMTAB);
	if(symtabsec == 0)
		return 0;
	elf_symtab* symtab = symtabsec->location;

	elf_loaded_image_section* strtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_STRTAB);
	if(strtabsec == 0)
		return 0;
	char* strtab = strtabsec->location;

	for(int i = 0; i < symtabsec->length / sizeof(elf_symtab); i++){
		if(util_str_equals(strtab + symtab[i].st_name, name)){
			return &symtab[i];
		}
	}
	return 0;
}

void elf_gen_loaded_image_data(elf_file* file, size_t loadLocation, elf_loaded_image* dest){
	memset(dest, 0, sizeof(elf_loaded_image));
	dest->loadLocation = loadLocation;

	char* imageDataSections[] = {".symtab", ".strtab", ".dynsym", ".dynstr", ".rela.plt", ".rela.dyn", ".rel.plt", ".rel.dyn"};

	for(int i = 0; i < ELF_LOADED_IMAGE_SECTION_COUNT; i++){
		elf_loaded_image_section* section = &dest->sections[i];
		elf_sh* sec = elf_get_sh_entry(file, imageDataSections[i]);
		if(sec != NULL){
			void* sectionBase = (void*) (sec->sh_offset + (size_t) file);
			size_t sectionSize = sec->sh_size;
			void* sectionCopy = kmalloc(sectionSize);
			if(sectionCopy == NULL)
				continue;
			memcpy(sectionCopy, sectionBase, sectionSize);
			section->type = i + 1; // types are in order of array + 1
			section->location = sectionCopy;
			section->length = sectionSize;
			section->entsize = sec->sh_entsize;
			reloc_ptr(&section->location);
		}
	}

	reloc_ptr((void**) &dest->loadLocation);
}

elf_loaded_image_section* elf_get_section_from_image(elf_loaded_image* image, size_t type){
	for(int i = 0; i < ELF_LOADED_IMAGE_SECTION_COUNT; i++){
		elf_loaded_image_section* section = &image->sections[i];
		if(section->type == type){
			return section;
		}
	}
	return NULL;
}



