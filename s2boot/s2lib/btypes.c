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
 * btypes.c - Built-in boot types.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/stdio.h>
#include <klibc/string.h>
#include <kernel/kutil.h>
#include <kernel/vfs.h>
#include <kernel/util.h>
#include <kernel/parse.h>
#include <kernel/log.h>
#include <kernel/msio.h>
#include <kernel/mmgr.h>
#include <kernel/btypes.h>


status_t btypes_init(){
	kernel_add_boot_handler("chain", btypes_boot_chain);
	kernel_add_boot_handler("mbr", btypes_boot_mbr);
	kernel_add_boot_handler("binary", btypes_boot_binary);
	kernel_add_boot_handler("image", btypes_boot_image);
	return 0;
}


status_t btypes_boot_chain(parse_entry* entry){
	status_t status = 0;
	char driveLabel[8];
	memset((void*) driveLabel, 0, 8);
	char* pdrive = parse_get_option(entry, "drive");
	char* ppartition = parse_get_option(entry, "partition");
	if(!ppartition)
		FERROR(TSX_MISSING_ARGUMENTS);
	size_t ppartitionNum = util_str_to_int(ppartition);
	if(pdrive == 0){
		snprintf(driveLabel, 8, "%s", kernel_get_boot_drive_label());
	}else{
		if(strlen(pdrive) > 7){
			log_fatal("Drive label for selected entry too large", 0xc);
			FERROR(TSX_ERROR);
		}
		snprintf(driveLabel, 8, "%s", pdrive);
	}
	uint64_t partStart;
	status = vfs_get_partition_lba(driveLabel, ppartitionNum, &partStart);
	CERROR();
	log_debug("Partition %u on drive %s starting at 0x%X\n", ppartitionNum, driveLabel, partStart);
	status = msio_read_drive(driveLabel, partStart, 1, ARCH_DEFAULT_MBR_LOCATION);
	CERROR();
	arch_os_entry_state entryState;
	memset(&entryState, 0, sizeof(arch_os_entry_state));
#ifdef ARCH_UPSTREAM_x86
	entryState.d = kernel_get_s1data()->bootDrive;
#endif
	kernel_jump(&entryState, ARCH_DEFAULT_MBR_LOCATION, KERNEL_S3BOOT_BMODE_16, 0);
	_end:
	return status;
}

status_t btypes_boot_mbr(parse_entry* entry){
	status_t status = 0;
	char* pfile = parse_get_option(entry, "file");
	if(!(pfile != NULL)){
		FERROR(TSX_MISSING_ARGUMENTS);
	}
	if(strlen(pfile) > 500)
		FERROR(TSX_TOO_LARGE);
	char* filePath = kernel_write_boot_file(pfile);
	if(filePath == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	log_debug("Loading image file %s\n", filePath);
	size_t fileSize = 0;
	status = vfs_get_file_size(filePath, &fileSize);
	CERROR();
	if(fileSize > 512){
		FERROR(TSX_TOO_LARGE);
	}
	status = kernel_read_file_s(filePath, ARCH_DEFAULT_MBR_LOCATION);
	CERROR();
	kfree(filePath, strlen(filePath) + 1);
	arch_os_entry_state entryState;
	memset(&entryState, 0, sizeof(arch_os_entry_state));
#ifdef ARCH_UPSTREAM_x86
	entryState.d = kernel_get_s1data()->bootDrive;
#endif
	kernel_jump(&entryState, ARCH_DEFAULT_MBR_LOCATION, KERNEL_S3BOOT_BMODE_16, 0);
	_end:
	return status;
}

status_t btypes_boot_binary(parse_entry* entry){
	status_t status = 0;
	char* pfile = parse_get_option(entry, "file");
	char* pdestination = parse_get_option(entry, "destination");
	char* poffset = parse_get_option(entry, "offset");
	char* pbits = parse_get_option(entry, "bits");
	if(!(pfile != NULL && pdestination != 0 && pbits != 0)){
		FERROR(TSX_MISSING_ARGUMENTS);
	}
	size_t pdestinationNum = util_str_to_hex(pdestination);
	size_t poffsetNum = 0;
	if(poffset)
		poffsetNum = util_str_to_hex(poffset);
	size_t pbitsNum = util_str_to_int(pbits);
	if(!(pbitsNum == 16 || pbitsNum == 32 || pbitsNum == 64)){
		log_error("Bits must be 16, 32 or 64\n");
		FERROR(TSX_INVALID_TYPE);
	}
	if(strlen(pfile) > 500)
		FERROR(TSX_TOO_LARGE);
	char* filePath = kernel_write_boot_file(pfile);
	if(filePath == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	log_debug("Loading image file %s\n", filePath);
	size_t size = 0;
	status = vfs_get_file_size(filePath, &size);
	CERROR();
	mmgr_reserve_mem_region(pdestinationNum, size, MMGR_MEMTYPE_OS);
	size_t tempLocation = (size_t) vmmgr_alloc_block_sequential(size);
	if(tempLocation == 0){
		FERROR(TSX_OUT_OF_MEMORY);
	}
	status = kernel_read_file_s(filePath, tempLocation);
	CERROR();
	kfree(filePath, strlen(filePath) + 1);
	arch_os_entry_state entryState;
	memset(&entryState, 0, sizeof(arch_os_entry_state));
	kernel_s3boot_reserve_mem_region(vmmgr_get_physical(tempLocation), size);
	kernel_s3boot_add_mem_region(pdestinationNum, size, vmmgr_get_physical(tempLocation));
	kernel_jump(&entryState, pdestinationNum + poffsetNum, pbitsNum == 16 ? KERNEL_S3BOOT_BMODE_16 : (pbitsNum == 32 ? KERNEL_S3BOOT_BMODE_32 : KERNEL_S3BOOT_BMODE_64), 0);
	_end:
	return status;
}

status_t btypes_boot_image(parse_entry* entry){
	status_t status = 0;
	char* pfile = parse_get_option(entry, "file");
	if(!(pfile != NULL)){
		FERROR(TSX_MISSING_ARGUMENTS);
	}
	if(strlen(pfile) > 500)
		FERROR(TSX_TOO_LARGE);
	char* filePath = kernel_write_boot_file(pfile);
	if(filePath == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	size_t size = 0;
	size_t tempLocation = 0;
	status = kernel_read_file(filePath, &tempLocation, &size);
	CERROR();
	kfree(filePath, strlen(filePath) + 1);
	size_t jmpDest = 0;
	size_t mode = 0;
	if(elf32_is_elf((elf32_file*) tempLocation)){
		elf32_file* file = (elf32_file*) tempLocation;
		if(file->ei_class == ELF_CLASS_32BIT){
			mode = KERNEL_S3BOOT_BMODE_32;
		}/*else if(file->ei_class == ELF_CLASS_64BIT){ // 64bit is not supported
			mode = KERNEL_S3BOOT_BMODE_64;
		}*/else{
			log_error("Unsupported/unknown ELF class %u\n", file->ei_class);
			FERROR(TSX_INVALID_FORMAT);
		}
		size_t totalSize = elf32_get_required_memory(file);
		size_t memBase = elf32_get_memory_base(file);
		if(totalSize == 0 || memBase == SIZE_MAX){
			FERROR(TSX_PARSE_ERROR);
		}
		vmmgr_map_pages_req(memBase + totalSize);
		mmgr_reserve_mem_region(memBase, totalSize, MMGR_MEMTYPE_OS);
		void* loadLocation = kmalloc(totalSize);
		if(loadLocation == 0){
			FERROR(TSX_OUT_OF_MEMORY);
		}
		log_debug("ELF load location: 0x%X (%u bytes) -> 0x%X\n", (size_t) loadLocation, totalSize, (size_t) file->e_entry);
		status = elf32_load_library(file, (size_t) loadLocation);
		jmpDest = file->e_entry;
		kernel_s3boot_add_mem_region(memBase, totalSize, vmmgr_get_physical((size_t) loadLocation));
	}else{
		FERROR(TSX_INVALID_FORMAT);
	}
	arch_os_entry_state entryState;
	memset(&entryState, 0, sizeof(arch_os_entry_state));
	kernel_jump(&entryState, jmpDest, mode, 0);
	_end:
	return status;
}

