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
#include <kernel/mmgr.h>
#include <kernel/dynl.h>
#include <kernel/parse.h>
#include <kernel/kutil.h>
#include <kernel/log.h>
#include <kernel/msio.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>



status_t modules_load_module_from_file(char* filePath, module_type_t type){
	size_t fileAddr = 0, fileSize = 0;
	status_t status = kernel_read_file(filePath, &fileAddr, &fileSize);
	CERROR();

	status = modules_load_module((elf_file*) fileAddr, type);
	CERROR();
	_end:
	if(fileAddr)
		kfree((void*) fileAddr, fileSize);
	return status;
}

status_t modules_load_module(elf_file* file, module_type_t type){
	status_t status = 0;
	if(type == MODULES_TYPE_DISK_DRIVER){
		status = modules_init_disk_driver(file, NULL);
		CERROR();
	}else if(type == MODULES_TYPE_FS_DRIVER){
		status = modules_init_fs_driver(file);
		CERROR();
	}else{
		FERROR(TSX_INVALID_TYPE);
	}
	_end:
	return status;
}


status_t modules_init_boot_handler(elf_file* file, char* type){
	status_t status = 0;
	void* address = 0;
	size_t size = 0;
	if(!(elf_is_elf(file) && file->ei_class == ELF_CLASS_nBIT))
		FERROR(TSX_INVALID_FORMAT);
	size = elf_get_required_memory(file);
	if(size >= MMGR_USABLE_MEMORY)
		FERROR(TSX_TOO_LARGE);
	if(size == 0)
		FERROR(TSX_INVALID_FORMAT);
	address = kmalloc(size);
	if(address == NULL)
		FERROR(TSX_OUT_OF_MEMORY);

	int listIndex = kernel_add_elf_image(file, (size_t) address);
	if(listIndex < 0)
		FERROR(TSX_OUT_OF_MEMORY);

	status = dynl_load_and_link_to_static_elf(file, (size_t) address, kernel_get_image_handle());
	CERROR();
	elf_symtab* startsymtab = elf_get_symtab_entry(file, "kboot_start");
	if(startsymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	kernel_add_boot_handler(type, (status_t (*) (parse_entry* entry)) (startsymtab->st_value + address));
	log_debug("Loaded boot handler for '%s' at %Y\n", type, address);
	_end:
	if(status != TSX_SUCCESS && address && size)
		kfree(address, size);
	return status;
}

status_t modules_init_disk_driver(elf_file* file, char** typeWrite){
	status_t status = 0;
	void* address = 0;
	size_t size = 0;
	if(!(elf_is_elf(file) && file->ei_class == ELF_CLASS_nBIT))
		FERROR(TSX_INVALID_FORMAT);
	size = elf_get_required_memory(file);
	if(size >= MMGR_USABLE_MEMORY)
		FERROR(TSX_TOO_LARGE);
	if(size == 0)
		FERROR(TSX_INVALID_FORMAT);
	address = kmalloc(size);
	if(!address)
		FERROR(TSX_OUT_OF_MEMORY);

	int listIndex = kernel_add_elf_image(file, (size_t) address);
	if(listIndex < 0)
		FERROR(TSX_OUT_OF_MEMORY);

	status = dynl_load_and_link_to_static_elf(file, (size_t) address, kernel_get_image_handle());
	CERROR();
	elf_symtab* initsymtab = elf_get_symtab_entry(file, "msio_init"); // optional
	elf_symtab* infosymtab = elf_get_symtab_entry(file, "msio_get_device_info");
	if(infosymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* readsymtab = elf_get_symtab_entry(file, "msio_read");
	if(readsymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* writesymtab = elf_get_symtab_entry(file, "msio_write");
	if(writesymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* typesymtab = elf_get_symtab_entry(file, "msio_get_driver_type");
	if(typesymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	char* type = ((char* (*) ())(typesymtab->st_value + address))();
	if(typeWrite)
		*typeWrite = type;
	if(initsymtab){
		status = ((status_t (*) ())(initsymtab->st_value + address))();
		CERROR();
	}
	status = msio_attach_driver(type,
		(MSIO_DRIVER_INFO)(infosymtab->st_value + address),
		(MSIO_DRIVER_READ)(readsymtab->st_value + address),
		(MSIO_DRIVER_WRITE)(writesymtab->st_value + address));
	CERROR();
	log_debug("Loaded disk driver for '%s' at %Y\n", type, address);
	_end:
	if(status != TSX_SUCCESS && address && size)
		kfree(address, size);
	return status;
}

status_t modules_init_fs_driver(elf_file* file){
	status_t status = 0;
	void* address = 0;
	size_t size = 0;
	if(!(elf_is_elf(file) && file->ei_class == ELF_CLASS_nBIT))
		FERROR(TSX_INVALID_FORMAT);
	size = elf_get_required_memory(file);
	if(size >= MMGR_USABLE_MEMORY)
		FERROR(TSX_TOO_LARGE);
	if(size == 0)
		FERROR(TSX_INVALID_FORMAT);
	address = kmalloc(size);
	if(!address)
		FERROR(TSX_OUT_OF_MEMORY);

	int listIndex = kernel_add_elf_image(file, (size_t) address);
	if(listIndex < 0)
		FERROR(TSX_OUT_OF_MEMORY);

	status = dynl_load_and_link_to_static_elf(file, (size_t) address, kernel_get_image_handle());
	CERROR();
	elf_symtab* initsymtab = elf_get_symtab_entry(file, "vfs_init"); // optional
	elf_symtab* isFssymtab = elf_get_symtab_entry(file, "vfs_isFilesystem");
	if(isFssymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* readFilesymtab = elf_get_symtab_entry(file, "vfs_readFile");
	if(readFilesymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* fileSizesymtab = elf_get_symtab_entry(file, "vfs_getFileSize");
	if(fileSizesymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	elf_symtab* listDirsymtab = elf_get_symtab_entry(file, "vfs_listDir");
	if(listDirsymtab == NULL)
		FERROR(TSX_UNDEFINED_REFERENCE);
	if(initsymtab){
		status = ((status_t (*) ())(initsymtab->st_value + address))();
		CERROR();
	}
	status = vfs_attach_driver(
		(VFS_DRIVER_IS_FILESYSTEM)(isFssymtab->st_value + address),
		(VFS_DRIVER_READ_FILE)(readFilesymtab->st_value + address),
		(VFS_DRIVER_GET_FILE_SIZE)(fileSizesymtab->st_value + address),
		(VFS_DRIVER_LIST_DIR)(listDirsymtab->st_value + address));
	CERROR();
	log_debug("Loaded filesystem driver at %Y\n", address);
	_end:
	if(status != TSX_SUCCESS && address && size)
		kfree(address, size);
	return status;
}

