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

#ifndef __UBI_H__
#define __UBI_H__

#include <klibc/stdlib.h>
#include <klibc/stdint.h>

#define UBI_K_ROOT_MAGIC 0x4083f3ec52494255
#define UBI_K_MEM_MAGIC 0x008ab29d204d454d
#define UBI_K_VID_MAGIC 0x00c0a7ba44495656
#define UBI_K_MODULES_MAGIC 0x00ebc9e653444f4d

#define UBI_B_ROOT_MAGIC 0xc0d316dc42494255
#define UBI_B_MEM_MAGIC 0x80feb99d204d454d
#define UBI_B_VID_MAGIC 0x800c881e44495656
#define UBI_B_MODULES_MAGIC 0x808eb4ad53444f4d
#define UBI_B_SYS_MAGIC 0x801ad6e75f535953
#define UBI_B_MEMMAP_MAGIC 0x80f604c750414d4d
#define UBI_B_SERVICES_MAGIC 0x80cca1c056525342
#define UBI_B_LOADER_MAGIC 0x8083ae8620424c42
#define UBI_B_CMD_MAGIC 0x80a4f8a34c444d43
#define UBI_B_BDRIVE_MAGIC 0x80c8cda856524442

#define UBI_MEMTYPE_USABLE 0x0
#define UBI_MEMTYPE_RESERVED 0x1
#define UBI_MEMTYPE_ACPI_RECLAIM 0x2
#define UBI_MEMTYPE_ACPI_NVS 0x3
#define UBI_MEMTYPE_BAD 0x4
#define UBI_MEMTYPE_UEFI_APP 0x4001
#define UBI_MEMTYPE_UEFI_BSRV 0x4002
#define UBI_MEMTYPE_UEFI_RSRV 0x4003
#define UBI_MEMTYPE_BOOTLOADER 0x5001
#define UBI_MEMTYPE_PAGING 0x5002
#define UBI_MEMTYPE_OS 0x5003

#define UBI_STATUS_SUCCESS 0
#define UBI_STATUS_ERROR 1
#define UBI_STATUS_UNSUPPORTED 10
#define UBI_STATUS_INVALID 11
#define UBI_STATUS_UNAVAILABLE 12
#define UBI_STATUS_OUT_OF_MEMORY 13
#define UBI_STATUS_NOT_FOUND 14

#define UBI_API __attribute__((sysv_abi))


#define UBI_FLAGS_FIRMWARE_BIOS 0
#define UBI_FLAGS_FIRMWARE_UEFI 1
#define UBI_FLAGS_FIRMWARE_UEFI_EXIT 0x4


typedef uint16_t ubi_status_t;
typedef size_t uintn_t;



typedef struct ubi_b_table_header* (UBI_API *UBI_GET_TABLE)(uint64_t magic);
typedef ubi_status_t (UBI_API *UBI_UEFI_EXIT)();

typedef ubi_status_t (UBI_API *UBI_ALLOCATE_PAGES)(uintn_t size, void** dest);
typedef ubi_status_t (UBI_API *UBI_READ_FILE)(const char* path, void** dest);



#pragma pack(push,1)
typedef struct ubi_table_header{
	uint64_t magic;
	struct ubi_table_header* nextTable;
} ubi_table_header;


typedef struct ubi_k_root_table{
	ubi_table_header hdr; // magic = UBI_K_ROOT_MAGIC

	uint8_t minimumSpecificationVersionMajor;
	uint8_t minimumSpecificationVersionMinor;

	uint16_t bits;

	uint32_t flags;
} ubi_k_root_table;


typedef struct ubi_k_mem_table{
	ubi_table_header hdr; // magic = UBI_K_MEM_MAGIC

	uint32_t flags;

	void* heapLocation;
	uintn_t heapSize;

	void* stackLocation;
	uintn_t stackSize;

	void* kernelBase;
	uintn_t kaslrSize;
} ubi_k_mem_table;


typedef struct ubi_k_video_table{
	ubi_table_header hdr; // magic = UBI_K_VID_MAGIC

	uint32_t flags;

	uint32_t width;
	uint32_t height;
	uint32_t bpp;
} ubi_k_video_table;


typedef struct ubi_k_module_table{
	ubi_table_header hdr; // magic = UBI_K_MODULES_MAGIC

	uint32_t flags;

	uint32_t length;

	struct {
		char* path;
		void* loadAddress;
	} modules[];
} ubi_k_module_table;



typedef struct ubi_b_table_header{
	uint64_t magic;
	struct ubi_b_table_header* nextTable;
	uint32_t checksum;
} ubi_b_table_header;


typedef struct ubi_b_root_table{
	ubi_b_table_header hdr; // magic = UBI_B_ROOT_MAGIC

	uint8_t specificationVersionMajor;
	uint8_t specificationVersionMinor;

	uint16_t reserved;

	uint32_t flags;

	UBI_GET_TABLE getTable;
	UBI_UEFI_EXIT uefiExit;
} ubi_b_root_table;


typedef struct ubi_b_mem_table{
	ubi_b_table_header hdr; // magic = UBI_B_MEM_MAGIC

	uint32_t flags;

	void* heapLocation;
	uintn_t heapSize;

	void* stackLocation;
	uintn_t stackSize;

	void* kernelBase;
} ubi_b_mem_table;


typedef struct ubi_b_video_table{
	ubi_b_table_header hdr; // magic = UBI_B_VID_MAGIC

	uint32_t flags;

	uint32_t width;
	uint32_t height;
	uint32_t bpp;
	uint32_t pitch;

	void* framebufferAddress;

	uint32_t cursorPosX;
	uint32_t cursorPosY;
} ubi_b_video_table;


typedef struct ubi_b_module_entry{
	char* path;
	void* loadAddress;
	uintn_t size;
} ubi_b_module_entry;

typedef struct ubi_b_module_table{
	ubi_b_table_header hdr; // magic = UBI_B_MODULES_MAGIC

	uint32_t flags;

	uint32_t length;

	ubi_b_module_entry* modules;
} ubi_b_module_table;


typedef struct ubi_b_system_table{
	ubi_b_table_header hdr; // magic = UBI_B_SYS_MAGIC

	uint32_t flags;

	void* smbiosAddress;

	void* rsdpAddress;

	void* uefiSystemTable;
} ubi_b_system_table;


typedef struct ubi_b_memmap_table{
	ubi_b_table_header hdr; // magic = UBI_B_MEMMAP_MAGIC

	uint32_t flags;

	uint32_t length;

	struct {
		uint64_t base;
		uint64_t size;
		uint32_t type;
		uint32_t reserved;
	}* entries;
} ubi_b_memmap_table;


typedef struct ubi_b_services_table{
	ubi_b_table_header hdr; // magic = UBI_B_SERVICES_MAGIC

	UBI_ALLOCATE_PAGES allocPages;
	UBI_READ_FILE readFile;
} ubi_b_services_table;


typedef struct ubi_b_loader_table{
	ubi_b_table_header hdr; // magic = UBI_B_LOADER_MAGIC

	char* name;
} ubi_b_loader_table;


typedef struct ubi_b_cmd_table{
	ubi_b_table_header hdr; // magic = UBI_B_CMD_MAGIC

	char* cmd;
} ubi_b_cmd_table;


typedef struct ubi_b_bdrive_table{
	ubi_b_table_header hdr; // magic = UBI_B_BDRIVE_MAGIC

	char type[8];
	uint32_t other;
} ubi_b_bdrive_table;
#pragma pack(pop)




#define UBI_TMP_DATA_SIZE MMGR_BLOCK_SIZE * 2


status_t ubi_start(char* file);
status_t ubi_load_kernel(char* file);
status_t ubi_load_kernel_segs();
status_t ubi_relocate(size_t kernelMinAddr, size_t kernelMaxAddr);

status_t ubi_create_tables(ubi_table_header* kroottable);
status_t ubi_create_mem_table(ubi_k_mem_table* table);
status_t ubi_create_vid_table(ubi_k_video_table* table);
status_t ubi_create_module_table(ubi_k_module_table* table);
status_t ubi_create_system_table();
status_t ubi_create_memmap_table();
status_t ubi_create_services_table();
status_t ubi_create_loader_table();
status_t ubi_create_cmd_table();
status_t ubi_create_bdrive_table();

status_t ubi_post_init();
status_t ubi_recreate_memmap();
ubi_status_t ubi_call_kernel();

void* ubi_get_file_addr(size_t vaddr);
size_t ubi_get_elf_reldyn_var_addr_f(size_t addr);
size_t ubi_get_elf_reldyn_var_addr(size_t addr);
void ubi_set_checksum(ubi_b_table_header* table, size_t totalTableSize);
void ubi_alloc_virtual(void** addr, size_t size);
ubi_table_header* ubi_get_kernel_table(uint64_t magic);
size_t ubi_get_table_size(uint64_t magic);
uint32_t ubi_convert_to_ubi_memtype(uint32_t memtype);
size_t ubi_get_random_kernel_offset(size_t kernelBase, size_t kaslrSize);


ubi_status_t ubi_convert_to_ubi_status(status_t status);
ubi_b_table_header* UBI_API ubi_srv_getTable(uint64_t magic);
ubi_status_t UBI_API ubi_srv_uefiExit();
ubi_status_t UBI_API ubi_srv_allocPages(uintn_t size, void** dest);
ubi_status_t UBI_API ubi_srv_readFile(const char* path, void** dest);

#endif /* __UBI_H__ */
