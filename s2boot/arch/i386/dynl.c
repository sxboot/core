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
#include <kernel/elf.h>
#include <kernel/stdio64.h>
#include <kernel/log.h>
#include <kernel/dynl.h>
#include <arch/dynl.h>

status_t dynl_load_and_link_to_static_elf(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage){
	status_t status = elf_load_library(source, sourceLocation);
	CERROR();
	status = dynl_link_to_static_elf(source, sourceLocation, targetimage);
	CERROR();
	_end:
	return status;
}

status_t dynl_link_to_static_elf(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage){
	status_t status = 0;
	elf_sh* dynsymsec = elf_get_sh_entry(source, ".dynsym");
	if(dynsymsec == 0)
		FERROR(TSX_ELF_MISSING_SECTION);
	elf_sh* dynstrsec = elf_get_sh_entry(source, ".dynstr");
	if(dynstrsec == 0)
		FERROR(TSX_ELF_MISSING_SECTION);
	elf_sh* relsec = elf_get_sh_entry(source, ".rel.plt");
	elf_sh* reldynsec = elf_get_sh_entry(source, ".rel.dyn");
	if(dynsymsec->sh_entsize != sizeof(elf_symtab))
		FERROR(TSX_ERROR);
	elf_symtab* dynsym = (elf_symtab*) (dynsymsec->sh_offset + (size_t) source);
	char* dynstr = (char*) (dynstrsec->sh_offset + (size_t) source);
	if(relsec){
		if(relsec->sh_entsize != sizeof(dynl_rel))
			FERROR(TSX_ERROR);
		status = dynl_link_to_static_elf_symlink(source, sourceLocation, targetimage, relsec, dynsym, dynstr);
		CERROR();
	}
	if(reldynsec){
		if(reldynsec->sh_entsize != sizeof(dynl_rel))
			FERROR(TSX_ERROR);
		status = dynl_link_to_static_elf_symlink(source, sourceLocation, targetimage, reldynsec, dynsym, dynstr);
		CERROR();
	}
	_end:
	return status;
}

status_t dynl_link_to_static_elf_symlink(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage, elf_sh* section, elf_symtab* dynsym, char* dynstr){
	status_t status = 0;
	dynl_rel* rel = (dynl_rel*) (section->sh_offset + (size_t) source);
	for(int i = 0; i < section->sh_size / sizeof(dynl_rel); i++){
		if(DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_JUMP_SLOT || DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_GLOB_DAT){
			size_t codeOffset = rel[i].r_offset + sourceLocation;
			char* name = dynstr + dynsym[DYNL_R_x_SYM(rel[i].r_info)].st_name;
			bool symAtTarget = true;
			elf_symtab* targetSymbol = elf_get_symtab_entry_from_image(targetimage, name);
			if(targetSymbol == 0 || targetSymbol->st_value == 0){
				targetSymbol = elf_get_symtab_entry(source, name);
				symAtTarget = false;
			}
			if(targetSymbol == 0 || targetSymbol->st_value == 0){
				printNlnr();
				log_error("[dynl] Undefined reference to ", 0xc);
				printlns(name, 0xf);
				FERROR(TSX_UNDEFINED_REFERENCE);
			}
			size_t targetOffset = targetSymbol->st_value + (symAtTarget ? targetimage->loadLocation : sourceLocation);
			*((size_t*)(codeOffset)) = targetOffset;
		}else if(DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_RELATIVE){
			size_t codeOffset = rel[i].r_offset + sourceLocation;
			size_t addend = *((size_t*) codeOffset);
			*((size_t*)(codeOffset)) = addend + sourceLocation;
		}else{
			FERROR(TSX_UNIMPLEMENTED);
		}
	}
	_end:
	return status;
}

status_t dynl_link_image_to_image(elf_loaded_image* sourceimage, elf_loaded_image* targetimage){
	status_t status = 0;
	elf_loaded_image_section* dynsymsec = elf_get_section_from_image(sourceimage, ELF_LI_SECTION_DYNSYM);
	if(dynsymsec == 0)
		FERROR(TSX_ELF_MISSING_SECTION);
	elf_loaded_image_section* dynstrsec = elf_get_section_from_image(sourceimage, ELF_LI_SECTION_DYNSTR);
	if(dynstrsec == 0)
		FERROR(TSX_ELF_MISSING_SECTION);
	elf_loaded_image_section* relsec = elf_get_section_from_image(sourceimage, ELF_LI_SECTION_RELPLT);
	elf_loaded_image_section* reldynsec = elf_get_section_from_image(sourceimage, ELF_LI_SECTION_RELDYN);
	if(dynsymsec->entsize != sizeof(elf_symtab))
		FERROR(TSX_ERROR);
	elf_symtab* dynsym = dynsymsec->location;
	char* dynstr = dynstrsec->location;
	if(relsec){
		if(relsec->entsize != sizeof(dynl_rel))
			FERROR(TSX_ERROR);
		status = dynl_link_image_to_image_symlink(sourceimage, targetimage, relsec, dynsym, dynstr);
		CERROR();
	}
	if(reldynsec){
		if(reldynsec->entsize != sizeof(dynl_rel))
			FERROR(TSX_ERROR);
		status = dynl_link_image_to_image_symlink(sourceimage, targetimage, reldynsec, dynsym, dynstr);
		CERROR();
	}
	_end:
	return status;
}

status_t dynl_link_image_to_image_symlink(elf_loaded_image* sourceimage, elf_loaded_image* targetimage, elf_loaded_image_section* section, elf_symtab* dynsym, char* dynstr){
	status_t status = 0;
	dynl_rel* rel = section->location;
	for(int i = 0; i < section->length / sizeof(dynl_rel); i++){
		if(DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_JUMP_SLOT || DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_GLOB_DAT){
			size_t codeOffset = rel[i].r_offset + sourceimage->loadLocation;
			char* name = dynstr + dynsym[DYNL_R_x_SYM(rel[i].r_info)].st_name;
			bool symAtTarget = true;
			elf_symtab* targetSymbol = elf_get_symtab_entry_from_image(targetimage, name);
			if(targetSymbol == 0 || targetSymbol->st_value == 0){
				targetSymbol = elf_get_symtab_entry_from_image(sourceimage, name);
				symAtTarget = false;
			}
			if(targetSymbol == 0 || targetSymbol->st_value == 0){
				printNlnr();
				log_error("[dynl] Undefined reference to ");
				printlns(name, 0xf);
				FERROR(TSX_UNDEFINED_REFERENCE);
			}
			size_t targetOffset = targetSymbol->st_value + (symAtTarget ? targetimage->loadLocation : sourceimage->loadLocation);
			*((size_t*)(codeOffset)) = targetOffset;
		}else if(DYNL_R_x_TYPE(rel[i].r_info) == DYNL_R_x_RELATIVE){
			size_t codeOffset = rel[i].r_offset + sourceimage->loadLocation;
			size_t addend = *((size_t*) codeOffset);
			*((size_t*)(codeOffset)) = addend + sourceimage->loadLocation;
		}else{
			FERROR(TSX_UNIMPLEMENTED);
		}
	}
	_end:
	return status;
}
