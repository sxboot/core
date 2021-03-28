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

#ifndef __KERNEL_VFS_H__
#define __KERNEL_VFS_H__


#include <kernel/list.h>


typedef bool (*VFS_DRIVER_IS_FILESYSTEM)(char* driveLabel, uint64_t partStart);
typedef status_t (*VFS_DRIVER_READ_FILE)(char* driveLabel, uint64_t partStart, char* path, size_t dest);
typedef status_t (*VFS_DRIVER_GET_FILE_SIZE)(char* driveLabel, uint64_t partStart, char* path, size_t* sizeWrite);
typedef status_t (*VFS_DRIVER_LIST_DIR)(char* driveLabel, uint64_t partStart, char* path, list_array** listWrite);

#pragma pack(push,1)
typedef struct vfs_fs_driver{
	VFS_DRIVER_IS_FILESYSTEM isFilesystem;
	VFS_DRIVER_READ_FILE readFile;
	VFS_DRIVER_GET_FILE_SIZE getFileSize;
	VFS_DRIVER_LIST_DIR listDir;
} vfs_fs_driver;

typedef struct vfs_part_mount{
	char driveLabel[8];
	uint8_t partNum;
	vfs_fs_driver* driver;
} vfs_part_mount;

typedef struct vfs_mbr_partition_entry{
	uint8_t status;
	uint8_t startHead;
	uint16_t startCS;
	uint8_t type;
	uint8_t endHead;
	uint16_t endCS;
	uint32_t startSector;
	uint32_t sectorCount;
} vfs_mbr_partition_entry;

typedef struct vfs_gpt_header{
	char signature[8];
	uint32_t revision;
	uint32_t headerSize;
	uint32_t crc32;
	uint32_t reserved;
	uint64_t currentLBA;
	uint64_t backupLBA;
	uint64_t firstLBA;
	uint64_t lastLBA;
	uint8_t diskGUID[16];
	uint64_t partitionEntriesLBA;
	uint32_t partitionEntries;
	uint32_t partitionEntrySize;
	uint32_t partitionEntriesCRC32;
} vfs_gpt_header;

typedef struct vfs_gpt_partition_entry{
	uint8_t typeGUID[16];
	uint8_t partitionGUID[16];
	uint64_t firstLBA;
	uint64_t lastLBA;
	uint64_t attributes;
	uint8_t name[72];
} vfs_gpt_partition_entry;
#pragma pack(pop)


// FAT 16

#define VFS_FAT_DIR_ENTRY_FILE_NEW 0
#define VFS_FAT_DIR_ENTRY_FILE 0x20
#define VFS_FAT_DIR_ENTRY_SUBDIR 0x10

#define VFS_FAT16_DATASTART (bpb->hiddenSectors + bpb->reservedSectors + bpb->numberOfFATs * bpb->sectorsPerFAT + bpb->rootEntries * 32 / bpb->bytesPerSector)

#pragma pack(push,1)
typedef struct vfs_fat_dir_entry{
	char nameShort[8];
	char exShort[3];
	uint8_t attributes;
	uint8_t free[8];
	uint16_t clusterHigh;
	uint16_t modTime;
	uint16_t modDate;
	uint16_t clusterLow;
	uint32_t size;
} vfs_fat_dir_entry;

typedef struct vfs_fat16_bpb{
	uint8_t bootJMP[3];
	char oem[8];
	uint16_t bytesPerSector;
	uint8_t sectorsPerCluster;
	uint16_t reservedSectors;
	uint8_t numberOfFATs;
	uint16_t rootEntries;
	uint16_t sectors1;
	uint8_t media;
	uint16_t sectorsPerFAT;
	uint16_t sectorsPerTrack;
	uint16_t heads;
	uint32_t hiddenSectors;
	uint32_t sectors2;
	uint8_t drive;
	uint8_t reserved;
	uint8_t bootSig;
	uint32_t volID;
	char volLabel[11];
	char filesystemName[8];
} vfs_fat16_bpb;
#pragma pack(pop)

status_t vfs_init();
status_t vfs_attach_driver(VFS_DRIVER_IS_FILESYSTEM isFilesystem,
	VFS_DRIVER_READ_FILE readFile,
	VFS_DRIVER_GET_FILE_SIZE getFileSize,
	VFS_DRIVER_LIST_DIR listDir);
status_t vfs_exec(char* path, char* driveWrite, uint64_t* partStartWrite, char** pathWrite, vfs_fs_driver** driverWrite);
status_t vfs_read_file(char* path, size_t dest);
status_t vfs_get_file_size(char* path, size_t* sizeWrite);
status_t vfs_list_dir(char* path, list_array** listWrite);
status_t vfs_cache_mount(char* driveLabel, uint8_t partNum, vfs_fs_driver* driver);
vfs_fs_driver* vfs_get_driver_from_cache(char* driveLabel, uint8_t partNum);
vfs_fs_driver* vfs_get_driver(char* driveLabel, uint8_t partNum, uint64_t partStart);
status_t vfs_get_partition_lba(char* drive, uint8_t partNum, uint64_t* lbaWrite);

bool vfs_fat16_isFilesystem(char* driveLabel, uint64_t partStart);
status_t vfs_fat16_readFile(char* driveLabel, uint64_t partStart, char* path, size_t dest);
status_t vfs_fat16_getFileSize(char* driveLabel, uint64_t partStart, char* path, size_t* sizeWrite);
status_t vfs_fat16_listDir(char* driveLabel, uint64_t partStart, char* path, list_array** listWrite);
status_t vfs_fat16_getDir(char* driveLabel, uint64_t partStart, char* path, vfs_fat_dir_entry** entryWrite);
status_t vfs_fat16_getFile(char* driveLabel, uint64_t partStart, char* path, vfs_fat_dir_entry** entryWrite);


#endif /* __KERNEL_VFS_H__ */
