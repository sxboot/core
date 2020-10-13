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

#include <kernel/s1bootdecl.h>

#define KERNEL_DEFAULT_STACK_LOCATION 0x7c00
#define KERNEL_DEFAULT_STACK_SIZE 0xc00
#define KERNEL_STACK_SIZE 4096

#define KERNEL_STATE_RUN 0
#define KERNEL_STATE_MENU 1
#define KERNEL_STATE_CONSOLE 2

#define KERNEL_S3BOOT_LOCATION 0x5000
#define KERNEL_S3BOOT_SIZE 0x1000
#define KERNEL_S3BOOT_MAP_SIZE 0x800

#define S3BOOT_MAGIC 0x297f09ae

#define S3BOOT_BMODE_REAL 1
#define S3BOOT_BMODE_PROTECTED 2
#define S3BOOT_BMODE_LONG 3

#pragma pack(push,1)
typedef struct s2map_entry{
	size_t address;
	size_t end;
} s2map_entry;

typedef struct kboot_handler{
	boot_type_t type;
	status_t (*start)(parse_entry* entry);
} kboot_handler;

typedef struct event_t{
	void* funcptr;
	size_t arg;
} event_t;

typedef struct ucallback_videomode{
	size_t width;
	size_t height;
	size_t bpp;
	size_t mode; // 0 - text mode, 1 - graphics mode
	size_t framebuffer;
	size_t bytesPerLine;
} ucallback_videomode;

typedef struct s3boot_data{
	uint32_t magic; // 0x297f09ae
	uint32_t s3mapStart;
	uint32_t s3mapLength;
	uint16_t bMode;
	uint64_t jmp;
	uint32_t reserved;
	uint16_t data1;
} s3boot_data;

typedef struct s3boot_map_entry{
	uint32_t base;
	uint32_t length;
	uint32_t source;
} s3boot_map_entry;
#pragma pack(pop)


status_t m_init();
status_t m_init_disk_driver(elf_file* file, int* listIndexWrite, char** typeWrite);
status_t m_init_fs_driver(elf_file* file, int* listIndexWrite);
status_t m_link_bdd();
status_t m_find_boot_drive();
status_t m_load_config();
status_t m_load_additional_drivers();
status_t m_set_boot_handlers();
void m_add_boot_handler(char* type, status_t (*start)(parse_entry* entry));
int m_add_elf_image(elf_file* file, size_t loadLocation);
status_t m_init_reloc_stack();


status_t m_select();


status_t m_start();
status_t m_load_external_boot_handler(char* type);

status_t m_load_s3boot();
status_t m_s3boot();
status_t m_boot_chain(parse_entry* entry);
status_t m_boot_mbr(parse_entry* entry);
status_t m_boot_binary(parse_entry* entry);
status_t m_boot_image(parse_entry* entry);

void m_start_add_s3boot_map_entry(uint32_t base, uint32_t length, uint32_t source);
void m_start_reserve_s3boot_map_mem_region(uint32_t base, uint32_t length);

status_t m_reset();


void m_save_vid_mem();
void m_restore_vid_mem();
void m_reset_state();

void m_print_title(char* s);
void m_show_menu();
void m_close_menu();
void m_menu_paint();

void m_start_console();
void m_close_console();
void m_console_on_key(uint16_t c);
void m_console_command(char* s);
void m_console_reset();


char* m_resolve_unknown_symbol(size_t addr, size_t* functionOffsetWrite);

void m_on_timer();

void m_poll_events();

status_t m_upstream_callback(size_t num, size_t arg0, size_t arg1, size_t arg2);

