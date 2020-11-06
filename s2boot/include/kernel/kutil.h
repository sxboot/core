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

#ifndef __KERNEL_KUTIL_H__
#define __KERNEL_KUTIL_H__

#include <kernel/elf.h>
#include <shared/s1bootdecl.h>


#define KERNEL_S3BOOT_LOCATION 0x5000
#define KERNEL_S3BOOT_SIZE 0x1000
#define KERNEL_S3BOOT_MAP_SIZE 0x800
#define KERNEL_S3BOOT_DATA_BUF KERNEL_S3BOOT_LOCATION + KERNEL_S3BOOT_SIZE + KERNEL_S3BOOT_MAP_SIZE

#define KERNEL_S3BOOT_BMODE_16 1
#define KERNEL_S3BOOT_BMODE_32 2
#define KERNEL_S3BOOT_BMODE_64 3

typedef struct kernel_error_frame{
	size_t fp;
	size_t ip;
	size_t nextFp;
} kernel_error_frame;

s1boot_data* kernel_get_s1data();
char* kernel_get_boot_label();
char* kernel_get_boot_drive_label();
char* kernel_get_boot_drive_type();
void kernel_get_stack_meta(size_t* stackLocation, size_t* stackSize);
void kernel_get_brand(char** name, char** versionstr, char** version);
void kernel_get_current_reloc(size_t* oldAddr, size_t* newAddr);
size_t kernel_get_reloc_ptr_count();
status_t kernel_add_event(void (*func), size_t arg);
void kernel_stop_autoboot();
bool kernel_is_console_running();
elf_loaded_image* kernel_get_image_handle();

status_t kernel_read_file_s(char* path, size_t dest);
status_t kernel_read_file(char* path, size_t* destWrite, size_t* sizeWrite);
status_t kernel_read_file_alloc(char* path, size_t* destWrite, size_t* sizeWrite);
char* kernel_write_boot_file(char* filename);

void kernel_print_stack_trace();
char* kernel_get_symbol_for_address(size_t addr, size_t* functionOffsetWrite);
char* kernel_get_symbol_for_address_in_image(elf_loaded_image* image, size_t addr, size_t* functionOffsetWrite);
void* kernel_get_address_for_symbol(char* symbol);
void* kernel_get_address_for_symbol_in_image(elf_loaded_image* image, char* symbol);

void kernel_halt();

void kernel_move_stack(size_t stackTop, size_t newStackSize);

status_t kernel_set_video(size_t width, size_t height, size_t bpp, bool graphics);

void kernel_print_error_trace();
bool kernel_create_error_stack();
void kernel_delete_error_stack();
void kernel_error_trace();

void kernel_add_reloc_ptr(void** ptr);
void kernel_del_reloc_ptr(void** ptr);
status_t kernel_relocate(size_t base);
void kernel_relocate_pointers(size_t oldAddr, size_t newAddr);
void kernel_relocate_stack(size_t oldAddr, size_t newAddr);
size_t kernel_get_reloc_base();

void kernel_runtime_assertion(bool x, char* msg);

size_t kernel_pseudorandom(size_t max);
void kernel_pseudorandom_seed(size_t seed);

status_t kernel_exit_uefi();

void kernel_s3boot_add_mem_region(uint32_t base, uint32_t length, uint32_t source);
void kernel_s3boot_reserve_mem_region(uint32_t base, uint32_t length);
void kernel_jump(arch_os_entry_state* entryState, size_t dest, size_t mode, uint32_t archFlags);

void kernel_add_boot_handler(char* type, void* start);
int kernel_add_elf_image(elf_file* file, size_t loadLocation);

#endif /* __KERNEL_KUTIL_H__ */
