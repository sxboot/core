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
 * vfs.c - An abstraction layer for filesystem drivers.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/util.h>
#include <kernel/msio.h>
#include <kernel/stdio64.h>
#include <kernel/errc.h>
#include <kernel/list.h>
#include <kernel/vfs.h>
#include <kernel/log.h>

static list_array* vfs_drivers = NULL;
static list_array* vfs_mounts = NULL;

static void* vfs_data_temp = NULL;

static char* gptSignature = "EFI PART";

status_t vfs_init(){
	status_t status = 0;
	vfs_data_temp = kmalloc(8192);
	if(vfs_data_temp == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &vfs_data_temp);
	vfs_drivers = list_array_create(0);
	if(vfs_drivers == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &vfs_drivers);
	vfs_mounts = list_array_create(0);
	if(vfs_mounts == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &vfs_mounts);
	status = vfs_attach_driver(&vfs_fat16_isFilesystem, &vfs_fat16_readFile, &vfs_fat16_getFileSize, &vfs_fat16_listDir);
	CERROR();
	_end:
	return status;
}

status_t vfs_attach_driver(VFS_DRIVER_IS_FILESYSTEM isFilesystem,
	VFS_DRIVER_READ_FILE readFile,
	VFS_DRIVER_GET_FILE_SIZE getFileSize,
	VFS_DRIVER_LIST_DIR listDir){
	status_t status = 0;
	vfs_fs_driver* driver = kmalloc(sizeof(vfs_fs_driver));
	if(!driver)
		FERROR(TSX_OUT_OF_MEMORY);
	driver->isFilesystem = isFilesystem;
	driver->readFile = readFile;
	driver->getFileSize = getFileSize;
	driver->listDir = listDir;
	reloc_ptr((void**) &driver->isFilesystem);
	reloc_ptr((void**) &driver->readFile);
	reloc_ptr((void**) &driver->getFileSize);
	reloc_ptr((void**) &driver->listDir);
	list_array_push(vfs_drivers, driver);
	_end:
	return status;
}

status_t vfs_exec(char* path, char* driveWrite, uint64_t* partStartWrite, char** pathWrite, vfs_fs_driver** driverWrite){
	status_t status = 0;
	if(path[0] != 0x2f)
		FERROR(21);
	uint8_t dsepIndex = 0;
	char drive[8];
	drive[7] = 0;
	for(int i = 0; i < 7; i++){
		drive[i] = 0;
		if(path[i + 1] == '.'){
			dsepIndex = i + 1;
			break;
		}
		drive[i] = path[i + 1];
	}
	if(drive[0] == 0)
		FERROR(21);
	if(dsepIndex == 0)
		FERROR(21);
	char* partString = path + dsepIndex + 1;
	uint64_t partNum = util_str_to_int_c(partString, 0x2f);
	if(partNum >= 0xff)
		FERROR(21);
	vfs_fs_driver* driver = 0;
	uint64_t partStart;
	status = vfs_get_partition_lba(drive, partNum, &partStart);
	CERROR();
	driver = vfs_get_driver(drive, partNum, partStart);
	if(driver == 0)
		FERROR(20);
	memcpy((void*) driveWrite, (void*) drive, 7);
	*partStartWrite = partStart;
	*pathWrite = util_str_cut_to(path + dsepIndex + 1, 0x2f);
	*driverWrite = driver;
	_end:
	return status;
}

status_t vfs_read_file(char* path, size_t dest){
	status_t status = 0;
	char drive[8];
	uint64_t partStart;
	char* ppath;
	vfs_fs_driver* driver;
	status = vfs_exec(path, drive, &partStart, &ppath, &driver);
	CERROR();
	status = driver->readFile(drive, partStart, ppath, dest);
	CERROR();
	_end:
	return status;
}

status_t vfs_get_file_size(char* path, size_t* sizeWrite){
	char drive[8];
	uint64_t partStart;
	char* ppath;
	vfs_fs_driver* driver;
	status_t status = vfs_exec(path, drive, &partStart, &ppath, &driver);
	CERROR();
	status = driver->getFileSize(drive, partStart, ppath, sizeWrite);
	CERROR();
	_end:
	return status;
}

status_t vfs_list_dir(char* path, list_array** listWrite){
	char drive[8];
	uint64_t partStart;
	char* ppath;
	vfs_fs_driver* driver;
	status_t status = vfs_exec(path, drive, &partStart, &ppath, &driver);
	CERROR();
	status = driver->listDir(drive, partStart, ppath, listWrite);
	CERROR();
	_end:
	return status;
}

status_t vfs_cache_mount(char* driveLabel, uint8_t partNum, vfs_fs_driver* driver){
	vfs_part_mount* mount = kmalloc(sizeof(vfs_part_mount));
	if(!mount)
		return TSX_OUT_OF_MEMORY;
	for(int i = 0; i < 8; i++){
		mount->driveLabel[i] = driveLabel[i];
	}
	mount->partNum = partNum;
	mount->driver = driver;
	reloc_ptr((void**) &mount->driver);
	list_array_push(vfs_mounts, mount);
	return 0;
}

vfs_fs_driver* vfs_get_driver_from_cache(char* driveLabel, uint8_t partNum){
	for(int i = 0; i < vfs_mounts->length; i++){
		vfs_part_mount* mount = vfs_mounts->base[i];
		if(util_str_equals(mount->driveLabel, driveLabel) && mount->partNum == partNum)
			return mount->driver;
	}
	return 0;
}

vfs_fs_driver* vfs_get_driver(char* driveLabel, uint8_t partNum, uint64_t partStart){
	vfs_fs_driver* driver = vfs_get_driver_from_cache(driveLabel, partNum);
	if(driver != NULL)
		return driver;
	for(int i = 0; i < vfs_drivers->length; i++){
		driver = vfs_drivers->base[i];
		if(driver->isFilesystem != 0 && driver->isFilesystem(driveLabel, partStart)){
			vfs_cache_mount(driveLabel, partNum, driver);
			return driver;
		}
	}
	return 0;
}

status_t vfs_get_partition_lba(char* drive, uint8_t partNum, uint64_t* lbaWrite){
	status_t status = 0;
	uint64_t partStart = 0;
	void* buf = kmalloc(8192);
	if(buf == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	status = msio_read_drive(drive, 1, 1, (size_t) buf);
	CERROR();
	if(util_str_startsWith(buf, gptSignature)){
		vfs_gpt_header* gptHeader = (vfs_gpt_header*) buf;
		uint64_t sectorOffset = gptHeader->currentLBA + 1 + (partNum * gptHeader->partitionEntrySize) / 512;
		vfs_gpt_partition_entry* partitions = (vfs_gpt_partition_entry*) (((size_t) (buf)) + 4096);
		status = msio_read_drive(drive, sectorOffset, 1, (size_t) partitions);
		CERROR();
		vfs_gpt_partition_entry* partition = partitions + (partNum % (512 / gptHeader->partitionEntrySize));
		partStart = partition->firstLBA;
	}else{
		status = msio_read_drive(drive, 0, 1, (size_t) buf);
		CERROR();
		vfs_mbr_partition_entry* partition = (vfs_mbr_partition_entry*) ((size_t) (buf) + 0x1be + 0x10 * partNum);
		partStart = partition->startSector;
	}
	if(partStart == 0)
		FERROR(TSX_INVALID_PARTITION);
	_end:
	if(buf)
		kfree(buf, 8192);
	if(lbaWrite)
		*lbaWrite = partStart;
	return status;
}

size_t vfs_get_data_temp(){
	// some of this code was written before even kmalloc() existed, which is why this still exists
	return (size_t) (vfs_data_temp);
}


bool vfs_fat16_isFilesystem(char* driveLabel, uint64_t partStart){
	vfs_fat16_bpb* bpb = (vfs_fat16_bpb*) (vfs_get_data_temp());
	status_t status = msio_read_drive(driveLabel, partStart, 1, (size_t) bpb);
	if(status != 0)
		return false;
	return util_str_startsWith(bpb->filesystemName, "FAT16   ");
}

status_t vfs_fat16_readFile(char* driveLabel, uint64_t partStart, char* path, size_t dest){
	vfs_fat_dir_entry* result = 0;
	status_t status = vfs_fat16_getFile(driveLabel, partStart, path, &result);
	CERROR();
	uint16_t cluster = result->clusterLow;
	vfs_fat16_bpb* bpb = (vfs_fat16_bpb*) (vfs_get_data_temp() + 4096); // bpb was read to this location by vfs_fat16_getFile
	updateLoadingWheel();
	if(cluster < 2){ // cluster == 0: file exists but is empty
		memset((void*) dest, 0, bpb->bytesPerSector * bpb->sectorsPerCluster);
		goto _end;
	}
	size_t dirLBA = VFS_FAT16_DATASTART + (cluster - 2) * bpb->sectorsPerCluster;
	size_t read = 0;
	size_t lastFATSectorOffset = -1;
	uint16_t* fat = (uint16_t*) vfs_get_data_temp();
	while(cluster >= 0x0002 && cluster <= 0xffef){
		updateLoadingWheel();
		status = msio_read_drive(driveLabel, VFS_FAT16_DATASTART + (cluster - 2) * bpb->sectorsPerCluster, bpb->sectorsPerCluster,
			dest + bpb->bytesPerSector * bpb->sectorsPerCluster * read);
		CERROR();
		size_t FATSectorOffset = cluster * 2 / bpb->bytesPerSector;
		if(lastFATSectorOffset != FATSectorOffset){
			status = msio_read_drive(driveLabel, bpb->hiddenSectors + bpb->reservedSectors + FATSectorOffset, 1, (size_t) fat);
			CERROR();
			lastFATSectorOffset = FATSectorOffset;
		}
		cluster = fat[cluster % (bpb->bytesPerSector / 2)];
		read++;
	}
	_end:
	return status;
}

status_t vfs_fat16_getFileSize(char* driveLabel, uint64_t partStart, char* path, size_t* sizeWrite){
	vfs_fat_dir_entry* result = 0;
	status_t status = vfs_fat16_getFile(driveLabel, partStart, path, &result);
	CERROR();
	if(sizeWrite)
		*sizeWrite = result->size;
	_end:
	return status;
}

status_t vfs_fat16_listDir(char* driveLabel, uint64_t partStart, char* path, list_array** listWrite){
	vfs_fat_dir_entry* result = 0;
	uint16_t* fat = 0;
	vfs_fat_dir_entry* dirTable = 0;
	status_t status = vfs_fat16_getDir(driveLabel, partStart, path, &result);
	CERROR();
	vfs_fat16_bpb* bpb = (vfs_fat16_bpb*) (vfs_get_data_temp() + 4096);
	list_array* list = list_array_create(0);
	// result == NULL means it is the root directory
	uint16_t cluster = result ? result->clusterLow : 0;
	// cluster == 0 means it is the root directory
	size_t sectors = cluster ? bpb->sectorsPerCluster : (bpb->rootEntries * 32 / bpb->bytesPerSector);
	dirTable = kmalloc(sectors * bpb->bytesPerSector);
	if(!dirTable)
		FERROR(TSX_OUT_OF_MEMORY);
	size_t lastFATSectorOffset = -1;
	fat = kmalloc(4096);
	addFiles:{
		size_t dirLBA = cluster ? (VFS_FAT16_DATASTART + (cluster - 2) * bpb->sectorsPerCluster) :
			(bpb->hiddenSectors + bpb->reservedSectors + bpb->numberOfFATs * bpb->sectorsPerFAT);
		status = msio_read_drive(driveLabel, dirLBA, sectors, (size_t) dirTable);
		CERROR();
		for(int ti = 0; ti < (cluster ? (bpb->sectorsPerCluster * bpb->bytesPerSector / 32) : bpb->rootEntries); ti++){
			if(dirTable[ti].nameShort[0] == 0 || (uint8_t) dirTable[ti].nameShort[0] == 0xe5 || util_str_startsWith(dirTable[ti].nameShort, ". ")
				|| util_str_startsWith(dirTable[ti].nameShort, ".. "))
				continue;
			size_t nameLen = util_str_length_c_max(dirTable[ti].nameShort, ' ', 8);
			size_t extLen = util_str_length_c_max(dirTable[ti].exShort, ' ', 3);
			char* name = kmalloc(nameLen + extLen + (extLen > 0 ? 1 : 0));
			memcpy(name, dirTable[ti].nameShort, nameLen);
			name[nameLen] = 0;
			if(extLen > 0){
				name[nameLen] = '.';
				memcpy(name + nameLen + 1, dirTable[ti].exShort, extLen);
			}
			name[nameLen + extLen + 1] = 0;
			list_array_push(list, name);
		}
		if(cluster){ // when not root directory, the directory table may span across multiple clusters
			size_t FATSectorOffset = cluster * 2 / bpb->bytesPerSector;
			if(lastFATSectorOffset != FATSectorOffset){
				status = msio_read_drive(driveLabel, bpb->hiddenSectors + bpb->reservedSectors + FATSectorOffset, 1, (size_t) fat);
				CERROR();
				lastFATSectorOffset = FATSectorOffset;
			}
			cluster = fat[cluster % (bpb->bytesPerSector / 2)];
			if(cluster >= 0x0002 && cluster <= 0xffef)
				goto addFiles;
		}
	}
	*listWrite = list;
	_end:
	if(fat)
		kfree(fat, 4096);
	if(dirTable)
		kfree(dirTable, sectors * bpb->bytesPerSector);
	return status;
}


status_t vfs_fat16_getDir(char* driveLabel, uint64_t partStart, char* path, vfs_fat_dir_entry** entryWrite){
	status_t status = 0;
	if(util_str_contains(path, ' '))
		FERROR(TSX_INVALID_SYNTAX);
	updateLoadingWheel();
	vfs_fat16_bpb* bpb = (vfs_fat16_bpb*) (vfs_get_data_temp() + 4096);
	status = msio_read_drive(driveLabel, partStart, 1, (size_t) bpb);
	CERROR();
	updateLoadingWheel();
	vfs_fat_dir_entry* dirTable = (vfs_fat_dir_entry*) vfs_get_data_temp();
	path++;
	size_t parts = util_count_parts(path, '/');
	char* next = path;
	size_t dirLBA = bpb->hiddenSectors + bpb->reservedSectors + bpb->numberOfFATs * bpb->sectorsPerFAT;
	bool found = FALSE;
	uint16_t lastCluster = 0;
	vfs_fat_dir_entry* result = NULL;
	for(int i = 0; i < parts - 1; i++){
		for(int ts = 0; ts < ((i == 0) ? (bpb->rootEntries * 32 / bpb->bytesPerSector) : bpb->sectorsPerCluster); ts++){ // ts = table sector (sector index in table)
			updateLoadingWheel();
			status = msio_read_drive(driveLabel, dirLBA + ts, 1, (size_t) dirTable);
			CERROR();
			for(int ti = 0; ti < bpb->bytesPerSector / 32; ti++){ // ti = table index (index in table sector)
				if(!(dirTable[ti].attributes & VFS_FAT_DIR_ENTRY_SUBDIR))
					continue;
				if(((uint8_t)dirTable[ti].nameShort[0]) == 0xe5)
					continue;
				if(((uint8_t)dirTable[ti].nameShort[0]) == 0)
					continue;
				bool match = TRUE;
				if(dirTable[ti].nameShort[6] == '~'){
					match = FALSE;
				}else{
					if(util_str_length_c(next, '/') != util_str_length_c_max(dirTable[ti].nameShort, ' ', 8))
						continue;
					for(uint8_t j = 0; j < 8; j++){
						if(next[j] == '/')
							break;
						bool wasLow = FALSE;
						if(next[j] > 96 && next[j] < 123){
							wasLow = TRUE;
							next[j] -= 32;
						}
						if(next[j] != dirTable[ti].nameShort[j])
							match = FALSE;
						if(wasLow && next[j] > 64 && next[j] < 91)
							next[j] += 32;
						if(!match)
							break;
					}
				}
				if(match){
					if(i == parts - 2)
						result = &dirTable[ti];
					lastCluster = dirTable[ti].clusterLow;
					dirLBA = VFS_FAT16_DATASTART + (dirTable[ti].clusterLow - 2) * bpb->sectorsPerCluster;
					found = TRUE;
					break;
				}
			}
			if(found)
				break;
		}
		if(!found){
			if(i != 0){
				// the directory table may continue beyond a single cluster
				updateLoadingWheel();
				uint16_t* fat = (uint16_t*) dirTable;
				status = msio_read_drive(driveLabel, bpb->hiddenSectors + bpb->reservedSectors + (lastCluster * 2) / bpb->bytesPerSector, 1, (size_t) fat);
				CERROR();
				uint16_t nextCluster = fat[lastCluster % bpb->bytesPerSector];
				if(nextCluster >= 0x0002 && nextCluster <= 0xffef){
					lastCluster = nextCluster;
					dirLBA = VFS_FAT16_DATASTART + (nextCluster - 2) * bpb->sectorsPerCluster;
					i--;
					continue;
				}
			}
			FERROR(TSX_NO_SUCH_DIRECTORY);
		}
		found = FALSE;
		next = util_str_cut_to(next, '/') + 1;
	}
	*entryWrite = result;
	_end:
	return status;
}

status_t vfs_fat16_getFile(char* driveLabel, uint64_t partStart, char* path, vfs_fat_dir_entry** entryWrite){
	status_t status = 0;
	if(util_str_contains(path, ' '))
		FERROR(TSX_INVALID_SYNTAX);
	updateLoadingWheel();
	vfs_fat16_bpb* bpb = (vfs_fat16_bpb*) (vfs_get_data_temp() + 4096);
	status = msio_read_drive(driveLabel, partStart, 1, (size_t) bpb);
	CERROR();
	updateLoadingWheel();
	vfs_fat_dir_entry* directory;

	status = vfs_fat16_getDir(driveLabel, partStart, path, &directory);
	CERROR();

	path++;
	size_t parts = util_count_parts(path, '/');
	uint16_t lastCluster = directory->clusterLow;
	size_t dirLBA = VFS_FAT16_DATASTART + (directory->clusterLow - 2) * bpb->sectorsPerCluster;
	char* next = path + strlen(path);
	for(; next > path && *(next - 1) != '/'; next--); // cut to the last part

	vfs_fat_dir_entry* dirTable = (vfs_fat_dir_entry*) vfs_get_data_temp();
	bool found = FALSE;
	vfs_fat_dir_entry* result;

	updateLoadingWheel();
	char* extension;
	if(util_str_contains(next, '.'))
		extension = util_str_cut_to(next, '.') + 1;
	else
		extension = "";
	while(util_str_contains(extension, '.'))
		extension = util_str_cut_to(extension, '.') + 1;
	while(true){
		for(int ts = 0; ts < ((parts == 1) ? (bpb->rootEntries * 32 / bpb->bytesPerSector) : bpb->sectorsPerCluster); ts++){ // ts = table sector (sector index in table)
			updateLoadingWheel();
			status = msio_read_drive(driveLabel, dirLBA + ts, 1, (size_t) dirTable);
			CERROR();
			for(int ti = 0; ti < bpb->bytesPerSector / 32; ti++){ // ti = table index (index in table sector)
				if(!(dirTable[ti].attributes & VFS_FAT_DIR_ENTRY_FILE) && !(dirTable[ti].attributes & VFS_FAT_DIR_ENTRY_FILE_NEW))
					continue;
				if(((uint8_t)dirTable[ti].nameShort[0]) == 0xe5)
					continue;
				if(((uint8_t)dirTable[ti].nameShort[0]) == 0)
					continue;
				bool match = TRUE;
				if(dirTable[ti].nameShort[6] == '~'){
					match = FALSE;
				}else{
					if(util_math_min(util_str_length(next), util_str_length_c_max(next, '.', 8)) != util_str_length_c_max(dirTable[ti].nameShort, ' ', 8))
						continue;
					if(util_str_length_c(extension, 0) != util_str_length_c_max(dirTable[ti].exShort, ' ', 3))
						continue;
					for(uint8_t j = 0; j < 8; j++){
						if(next[j] == '.' || dirTable[ti].nameShort[j] == ' ')
							break;
						bool wasLow = FALSE;
						if(next[j] > 96 && next[j] < 123){
							wasLow = TRUE;
							next[j] -= 32;
						}
						if(next[j] != dirTable[ti].nameShort[j])
							match = FALSE;
						if(wasLow && next[j] > 64 && next[j] < 91)
							next[j] += 32;
						if(!match)
							break;
					}
					for(uint8_t i = 0; i < 3; i++){
						if(extension[i] == 0 || dirTable[ti].exShort[i] == ' ')
							break;
						bool wasLow = FALSE;
						if(extension[i] > 96 && extension[i] < 123){
							wasLow = TRUE;
							extension[i] -= 32;
						}
						if(extension[i] != dirTable[ti].exShort[i])
							match = FALSE;
						if(wasLow && extension[i] > 64 && extension[i] < 91)
							extension[i] += 32;
						if(!match)
							break;
					}
				}
				if(match){
					result = &dirTable[ti];
					lastCluster = dirTable[ti].clusterLow;
					dirLBA = VFS_FAT16_DATASTART + (dirTable[ti].clusterLow - 2) * bpb->sectorsPerCluster;
					found = TRUE;
					break;
				}
			}
			if(found)
				break;
		}
		if(!found){
			if(parts > 1){
				updateLoadingWheel();
				uint16_t* fat = (uint16_t*) dirTable;
				status = msio_read_drive(driveLabel, bpb->hiddenSectors + bpb->reservedSectors + (lastCluster * 2) / bpb->bytesPerSector, 1, (size_t) fat);
				CERROR();
				uint16_t nextCluster = fat[lastCluster % bpb->bytesPerSector];
				if(nextCluster >= 0x0002 && nextCluster <= 0xffef){
					lastCluster = nextCluster;
					dirLBA = VFS_FAT16_DATASTART + (nextCluster - 2) * bpb->sectorsPerCluster;
					continue;
				}
			}
			FERROR(TSX_NO_SUCH_FILE);
		}else
			break;
	}
	if(!found) // this should never be true
		FERROR(TSX_NO_SUCH_FILE);
	updateLoadingWheel();
	*entryWrite = result;
	_end:
	return status;
}
