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

#ifndef __SUBOOT__
#define __SUBOOT__


typedef _Bool				bool;

#define true 1
#define false 0


typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int					int32_t;
typedef unsigned int		uint32_t;
typedef long long			int64_t;
typedef unsigned long long	uint64_t;

#if ARCH_BITS == 32
typedef uint32_t	size_t;
#define SIZE_MAX 0xffffffff
#elif ARCH_BITS == 64
typedef uint64_t	size_t;
#define SIZE_MAX 0xffffffffffffffff
#endif

#define status_t uint8_t


#if ARCH_BITS == 64
#define S2_API __attribute__((sysv_abi))
#elif ARCH_BITS == 32
#define S2_API
#endif



#pragma pack(push,1)
typedef struct mbr_partition_entry{
	uint8_t status;
	uint8_t startHead;
	uint16_t startCS;
	uint8_t type;
	uint8_t endHead;
	uint16_t endCS;
	uint32_t startSector;
	uint32_t sectorCount;
} mbr_partition_entry;

typedef struct gpt_header{
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
} gpt_header;

typedef struct gpt_partition_entry{
	uint8_t typeGUID[16];
	uint8_t partitionGUID[16];
	uint64_t firstLBA;
	uint64_t lastLBA;
	uint64_t attributes;
	uint8_t name[72];
} gpt_partition_entry;


typedef struct fs_fat_dir_entry{
	char nameShort[8];
	char exShort[3];
	uint8_t attributes;
	uint8_t free[8];
	uint16_t clusterHigh;
	uint16_t modTime;
	uint16_t modDate;
	uint16_t clusterLow;
	uint32_t size;
} fs_fat_dir_entry;

typedef struct fs_fat16_bpb{
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
} fs_fat16_bpb;


typedef struct elf_file{
	uint32_t ei_mag;
	uint8_t ei_class;
	uint8_t ei_data;
	uint8_t ei_version;
	uint8_t ei_osabi;
	uint8_t ei_abiversion;
	uint8_t ei_pad[7];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	size_t e_entry;
	size_t e_phoff;
	size_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} elf_file;

#if ARCH_BITS == 64
#define ELF_CLASS 2

typedef struct elf_ph{
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} elf_ph;
#elif ARCH_BITS == 32
#define ELF_CLASS 1

typedef struct elf_ph{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} elf_ph;
#endif

typedef struct elf_dyn{
	size_t type;
	size_t val;
} elf_dyn;

typedef struct elf_rel{
	size_t r_offset;
	size_t r_info;
} elf_rel;

typedef struct elf_rela{
	size_t r_offset;
	size_t r_info;
	size_t r_addend;
} elf_rela;


typedef struct s2map_entry{
	size_t address;
	size_t end;
} s2map_entry;

typedef struct mmap_entry{
	uint64_t addr;
	uint64_t size;
	uint32_t type;
	uint32_t reserved;
} mmap_entry;


typedef struct s2callback_videomode{
	size_t width;
	size_t height;
	size_t bpp;
	size_t mode; // 0 - text mode, 1 - graphics mode
	size_t framebuffer;
	size_t bytesPerLine;
} s2callback_videomode;

typedef struct s2callback_readdrive{
	size_t num;
	size_t numSectors;
	size_t dest;
	uint64_t lba;
} s2callback_readdrive;
#pragma pack(pop)

#define FS_FAT_DIR_ENTRY_FILE 0x20
#define FS_FAT_DIR_ENTRY_SUBDIR 0x10

#define ELF_MAGIC 0x464c457f
#if defined(ARCH_amd64)
#define ELF_MACHINE 0x3e
#elif defined(ARCH_i386)
#define ELF_MACHINE 0x03
#else
#error Invalid arch
#endif



EFI_STATUS suboot_get_devices();
EFI_STATUS suboot_load_s2boot();
EFI_STATUS suboot_parse_s2boot();
EFI_STATUS suboot_get_bdrive_data();
EFI_STATUS suboot_set_graphics();
EFI_STATUS suboot_remap();
EFI_STATUS suboot_get_memory_map();

EFI_STATUS suboot_read_sectors(uint8_t id, size_t lba, size_t numSectors, void* dest);
size_t suboot_get_partition_lba(uint8_t driveId, uint8_t partNum);
EFI_STATUS suboot_read_file(uint8_t driveId, uint8_t partNum, char* first, char* second, void** dest, size_t* size);


void* suboot_alloc(size_t pages);
void suboot_free(void* addr, size_t pages);


void suboot_println(char* str);
void suboot_printNln();

void suboot_print_hex(size_t hex);
void suboot_print_dec(size_t dec);

void suboot_print(char* str);
size_t suboot_strlen(char* str);

status_t suboot_get_sxboot_status(EFI_STATUS s);

status_t S2_API suboot_callback(size_t num, size_t arg0, size_t arg1, size_t arg2);


bool suboot_memcmp(void* str1, void* str2, size_t len);
void* memset(void *dest, int val, size_t len);
void* memcpy(void *dest, const void *src, size_t len);



size_t suboot_get_highest_accessed_address();
EFI_STATUS suboot_mem_id_map(size_t start, size_t end);


#endif /* __SUBOOT__ */

