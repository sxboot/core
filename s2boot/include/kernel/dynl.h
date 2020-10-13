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
 * dynl.h - Linker functions to be implemented by architecture-specific drivers.
 */

#ifndef __KERNEL_DYNL_H__
#define __KERNEL_DYNL_H__

#include <kernel/elf.h>


#pragma pack(push,1)
typedef struct dynl_rel{
	uint64_t r_offset;
	uint64_t r_info;
} dynl_rel;

typedef struct dynl_rela{
	uint64_t r_offset;
	uint64_t r_info;
	uint64_t r_addend;
} dynl_rela;
#pragma pack(pop)


/* Loads the ELF file passed to 'source' at the predefined vaddress 'sourceLocation' and then links the new file to an already loaded image 'targetimage'.
*  Because the ELF file 'source' is loaded at a predefined vaddress, ignoring the memory address offset given in the ELF file, it is required to be a PIE.
   Used to load and link a shared library against a static image at a predefined address. */
status_t dynl_load_and_link_to_static_elf(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage);

status_t dynl_link_to_static_elf(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage);
status_t dynl_link_to_static_elf_symlink(elf_file* source, size_t sourceLocation, elf_loaded_image* targetimage, elf_sh* section, elf_symtab* dynsym, char* dynstr);

status_t dynl_link_image_to_image(elf_loaded_image* sourceimage, elf_loaded_image* targetimage);
status_t dynl_link_image_to_image_symlink(elf_loaded_image* sourceimage, elf_loaded_image* targetimage, elf_loaded_image_section* section, elf_symtab* dynsym, char* dynstr);


#endif /* __KERNEL_DYNL_H__ */
