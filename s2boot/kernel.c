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
 * kernel.c - The main file of the second stage of the sxboot boot loader (s2boot).
 * This file contains the entry point of s2boot.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/stdarg.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/stdio64.h>
#include <kernel/errc.h>
#include <kernel/util.h>
#include <kernel/msio.h>
#include <kernel/vfs.h>
#include <kernel/mmgr.h>
#include <kernel/elf.h>
#include <kernel/dynl.h>
#include <kernel/parse.h>
#include <kernel/kb.h>
#include <kernel/log.h>
#include <kernel/kutil.h>
#include <kernel/list.h>
#include <kernel/serial.h>
#include <kernel/cli.h>
#include <kernel/btypes.h>
#include <kernel/modules.h>
#include "kernel.h"


#define VERSION_MAJOR 10
#define VERSION_MINOR 0


static s1boot_data* s1data = 0;


static char* m_title = "TSX Boot Manager (" STRINGOF(ARCH_NAME)/* this expands to "1" on i386 ?? */ ", " STRINGOF(ARCH_BITS) "bit)";
static char* m_version = "TSXBoot v4." STRINGOF(VERSION_MAJOR) "." STRINGOF(VERSION_MINOR);
static char* m_version_r = "4." STRINGOF(VERSION_MAJOR) "." STRINGOF(VERSION_MINOR);

static uint8_t m_state = 0;

static uint8_t m_menu_selected = 0;
static bool m_menu_autoboot = TRUE;
static size_t m_menu_counter = 0;


static char bootDriveType[5];
static uint8_t bootDriveNum = -1;
static char bootDriveLabel[16];
static char bootPartLabel[16];
static char bootPartNumStr[8];
static char bootDriveNumStr[8];

static char* confFilePath = NULL;
static char* s3bootFilePath = NULL;


static list_array* m_elf_images = NULL;
static int m_elf_image_kernel_index;


static list_array* m_boot_handlers = NULL;


static list_array* m_error_stack = NULL;


static list_array* m_reloc_list = NULL;
static size_t m_reloc_base = 0;


static size_t m_stack_top = 0;
static size_t m_stack_size = 0;


static list_array* m_event_queue = NULL;


static uint8_t m_init_state = 0;


static size_t cRelocOldAddr = 0;
static size_t cRelocNewAddr = 0;


static size_t rand_next = 1602621212;


static bool s3boot_loaded = FALSE;
static s3boot_data s3data;


void stdlib_init();

extern void jmpImm(size_t destination, size_t arg);
extern void __dev_i_fault();


// -----------------------------------------------------------
// INIT
// -----------------------------------------------------------

status_t m_init(){
	status_t status = 0;
	if(s1data->headerVersion < 49)
		FERROR(TSX_ERROR);

	log_debug("%s version %s\n", m_title, m_version_r);

	m_init_state = 1;
	status = mmgr_init(s1data->mmapStart, s1data->mmapLength);
	CERROR();

	// reserve all memory regions containing files or data
	mmgr_reserve_mem_region((size_t) s1data, sizeof(s1boot_data), MMGR_MEMTYPE_BOOTLOADER_DATA);
	s2map_entry* entry = (s2map_entry*) s1data->s2mapStart;
	for(int i = 0; i < s1data->s2mapLength; i++){
		mmgr_reserve_mem_region(entry->address, entry->end - entry->address, MMGR_MEMTYPE_BOOTLOADER);
		entry++;
	}
	mmgr_reserve_mem_region(s1data->s2mapStart, s1data->s2mapLength * sizeof(s2map_entry), MMGR_MEMTYPE_BOOTLOADER_DATA);
	mmgr_reserve_mem_region(s1data->s2bootAddress, s1data->s2bootSize, MMGR_MEMTYPE_BOOTLOADER_DATA);
	mmgr_reserve_mem_region(s1data->bddAddress, s1data->bddSize, MMGR_MEMTYPE_BOOTLOADER_DATA);
	mmgr_reserve_mem_region(s1data->s1bootStart, s1data->s1bootEnd - s1data->s1bootStart, MMGR_MEMTYPE_BOOTLOADER);
	if(!mmgr_is_area_clear(KERNEL_S3BOOT_LOCATION, KERNEL_S3BOOT_SIZE + KERNEL_S3BOOT_MAP_SIZE)){
		status = TSX_MEMORY_RESERVED;
		goto _end;
	}
	mmgr_reserve_mem_region(KERNEL_S3BOOT_LOCATION, KERNEL_S3BOOT_SIZE + KERNEL_S3BOOT_MAP_SIZE, MMGR_MEMTYPE_BOOTLOADER);

	log_debug("Memory: %uKiB total, %uKiB used\n", mmgr_get_total_memory() / 1024, mmgr_get_used_blocks() * MMGR_BLOCK_SIZE / 1024);

	status = m_upstream_callback(1, (size_t) (&kmalloc), 0, 0);
	CERROR();

	status = m_init_reloc_stack();
	CERROR();

	vmmgr_unmap_page(0); // unmap first page to catch "null pointer exceptions"

	m_init_state = 2;

	elf_file* s2bootImage = (elf_file*) (s1data->s2bootAddress);

	m_reloc_list = list_array_create(LIST_FLAGS_STATIC);
	if(m_reloc_list == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	list_array_reserve(m_reloc_list, 64);
	// we need to add this manually because the reloc_ptr call in list_array_create is ignored because it is called before this returns
	reloc_ptr((void**) &m_reloc_list->base);
	reloc_ptr((void**) &m_reloc_list);

	stdlib_init();

	m_elf_images = list_array_create(0);
	if(m_elf_images == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &m_elf_images);
	// copy all additional required data from elf file so we dont need to have the entire file in memory
	m_elf_image_kernel_index = kernel_add_elf_image(s2bootImage, s1data->s2bootBase);
	if(m_elf_image_kernel_index < 0)
		FERROR(TSX_OUT_OF_MEMORY);

	reloc_ptr((void**) &m_stack_top);
	reloc_ptr((void**) &s1data);
	reloc_ptr((void**) &s1data->mmapStart);
	reloc_ptr((void**) &s1data->s2mapStart);
	reloc_ptr((void**) &s1data->s2bootAddress);
	reloc_ptr((void**) &s1data->s1bootStart);
	reloc_ptr((void**) &s1data->s1bootEnd);
	reloc_ptr((void**) &s1data->s2bootBase);
	status = stdio64_init();
	CERROR();
	mmgr_reg_map_reloc_ptr();

	reloc_ptr((void**) &m_error_stack);

	m_event_queue = list_array_create(0);
	if(m_event_queue == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &m_event_queue);

	elf_sh* commentSec = elf_get_sh_entry(s2bootImage, ".comment");
	if(commentSec){
		log_debug("");
		char* comment = (char*) (s1data->s2bootAddress + commentSec->sh_offset);
		char* comma = "";
		while((size_t) comment - (s1data->s2bootAddress + commentSec->sh_offset) < commentSec->sh_size){
			size_t len = strlen(comment);
			if(len > 0)
				printf("%s%s", comma, comment);
			comment += len + 1;
			comma = ", ";
		}
		printf("\n");
	}
	// this is not required anymore
	mmgr_free_mem_region(s1data->s2bootAddress, s1data->s2bootSize);

	status = cli_init();
	CERROR();

	status = arch_platform_initialize();
	CERROR();
	status = arch_set_timer(1000);
	CERROR();
	arch_on_timer_fire(&m_on_timer);

	kernel_pseudorandom_seed((size_t) arch_real_time());
	// add some more "randomness"
	for(size_t i = 0; i < kernel_pseudorandom(100); i++){
		kernel_pseudorandom(1);
	}

	status = msio_init();
	CERROR();

	char* firmwareDriveType;
	status = m_attach_firmware_disk_driver(&firmwareDriveType);
	CERROR();

	status = m_link_bdd();
	if(status != TSX_SUCCESS){
		log_warn("BDD initialization failed: %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
		goto _post_find_boot_drive;
	}
	//mmgr_free_mem_region(s1data->bddAddress, s1data->bddSize);

	status = m_find_boot_drive();
	_post_find_boot_drive:
	if(status != TSX_SUCCESS){
		log_warn("Failed to find boot drive using bdd.ko: %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
		log_debug("Attempting to find boot drive using firmware disk driver\n");
		memcpy((void*) bootDriveType, (void*) firmwareDriveType, 4);
		status = m_find_boot_drive();
		if(status != TSX_SUCCESS)log_error("Failed to find boot drive\n");
		CERROR();
	}

	snprintf(bootDriveLabel, 16, "%s%u", bootDriveType, bootDriveNum);
	snprintf(bootPartLabel, 16, "%s%u.%u", bootDriveType, bootDriveNum, s1data->bootPartN);
	snprintf(bootPartNumStr, 8, "%u", s1data->bootPartN);
	snprintf(bootDriveNumStr, 8, "%u", bootDriveNum);

	status = vfs_init();
	CERROR();
	status = m_load_config();
	CERROR();

	stdio64_set_font_scale(parse_get_data()->fontScale);

	status = m_load_additional_drivers();
	CERROR();

	m_boot_handlers = list_array_create(0);
	if(m_boot_handlers == NULL)
		return TSX_OUT_OF_MEMORY;
	reloc_ptr((void**) &m_boot_handlers);
	status = btypes_init();
	CERROR();

	status = m_upstream_callback(2, 0, 0, 0);
	CERROR();

	m_init_state = 4;

	status = kb_init();
	if(status != TSX_SUCCESS){
		log_warn("Keyboard init failed: %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
		status = 0;
	}

	status = serial_init(parse_get_data()->serialBaud);
	if(status != TSX_SUCCESS){
		log_warn("Serial init failed: %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
		status = 0;
	}

	_end:
	return status;
}

status_t m_attach_firmware_disk_driver(char** firmwareDriveTypeWrite){
	status_t status = 0;
	char* firmwareDriveType;
	status = m_upstream_callback(20, (size_t) &firmwareDriveType, 0, 0);
	CERROR();
	status = msio_attach_driver(firmwareDriveType, &m_firmware_devinfo, &m_firmware_read, NULL);
	CERROR();
	log_debug("Attached firmware driver: type=%s\n", firmwareDriveType);
	if(firmwareDriveTypeWrite)
		*firmwareDriveTypeWrite = firmwareDriveType;
	_end:
	return status;
}

status_t m_firmware_read(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t dest){
	ucallback_readdrive args;
	args.num = number;
	args.numSectors = sectorCount;
	args.dest = dest;
	args.lba = sector;
	status_t status = m_upstream_callback(21, (size_t) &args, 0, 0);
	CERROR();
	_end:
	return status;
}

status_t m_firmware_devinfo(uint8_t number, uint64_t* sectors, size_t* sectorSize){
	status_t status = m_upstream_callback(22, (size_t) number, (size_t) sectors, (size_t) sectorSize);
	CERROR();
	_end:
	return status;
}

status_t m_link_bdd(){
	log_debug("Linking bdd (%Y)\n", s1data->bddAddress);
	status_t status = 0;
	elf_file* file = (elf_file*) (s1data->bddAddress);
	char* type;
	status = modules_init_disk_driver(file, &type);
	memcpy((void*) bootDriveType, (void*) type, 4);
	_end:
	return status;
}

status_t m_find_boot_drive(){
	status_t status = 0;
	char driveLabel[8];
	uint8_t current = 0;
	void* buffer = kmalloc(4096);
	if(buffer == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	for(uint8_t i = 0; i < 0xff; i++){
		memset((void*) driveLabel, 0, 8);
		snprintf(driveLabel, 8, "%s%u", bootDriveType, i);
		status = msio_read_drive(driveLabel, 0, 1, (size_t) buffer);
		if(status == TSX_NO_DEVICE){ // means there are no more devices
			break;
		}else if(status != TSX_SUCCESS){
			log_warn("msio_read_drive: error %u (%s)\n", (size_t) status, errcode_get_name(status)); // maybe this device is broken or something
			kernel_print_error_trace();
			continue;
		}
		uint32_t signature = *((uint32_t*) (((size_t) buffer) + 0x1b8));
		if(signature == s1data->bootDriveSg){
			bootDriveNum = i;
			log_debug("Found boot drive at %s\n", driveLabel);
			goto _end;
		}
	}
	FERROR(TSX_NO_DEVICE);
	_end:
	if(buffer)
		kfree(buffer, 4096);
	return status;
}

status_t m_load_config(){
	status_t status = 0;
	confFilePath = kmalloc(MMGR_BLOCK_SIZE);
	if(confFilePath == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &confFilePath);
	snprintf(confFilePath, MMGR_BLOCK_SIZE, "/%s%u.%u/config/config.cfg", bootDriveType, bootDriveNum, s1data->bootPartN);
	size_t address = 0;
	size_t fileSize = 0;
	status = kernel_read_file(confFilePath, &address, &fileSize);
	CERROR();
	*((char*) (address + fileSize)) = 0;
	status = parse_file((char*) address);
	CERROR();
	_end:
	return status;
}

status_t m_load_additional_drivers(){
	status_t status = 0;
	parse_data_t* parseData = parse_get_data();

	int cuType = MODULES_TYPE_DISK_DRIVER;
	char* driverPath = kernel_write_boot_file(parseData->hdDriverOverride ? parseData->hdDriverOverride : "/[BDRIVE]/lib/disk/");
	loadDrivers:{
		size_t driverPathLen = strlen(driverPath);
		log_debug("Loading drivers from %s\n", driverPath);
		list_array* files;
		status = vfs_list_dir(driverPath, &files);
		if(status != TSX_SUCCESS)
			goto loadDrivers_f;
		for(size_t i = 0; i < files->length; i++){
			char* filePath = kmalloc(driverPathLen + strlen(files->base[i]) + 1);
			if(!filePath)
				FERROR(TSX_OUT_OF_MEMORY);
			sprintf(filePath, "%s%s", driverPath, files->base[i]); // absolute path

			status = modules_load_module_from_file(filePath, cuType);
			if(status != TSX_SUCCESS){
				log_warn("Loading driver '%s' failed: %u (%s)\n", files->base[i], (size_t) status, errcode_get_name(status));
				kernel_print_error_trace();
				status = 0;
			}

			kfree(files->base[i], strlen(files->base[i])); // free file name string in list
		}
		list_array_delete(files);
		loadDrivers_f:
		if(status != TSX_SUCCESS){
			log_warn("Loading drivers failed: %u (%s)\n", (size_t) status, errcode_get_name(status));
			kernel_print_error_trace();
			status = 0;
		}
	}
	if(cuType == MODULES_TYPE_DISK_DRIVER){
		cuType = MODULES_TYPE_FS_DRIVER;
		driverPath = kernel_write_boot_file(parseData->fsDriverOverride ? parseData->fsDriverOverride : "/[BDRIVE]/lib/fs/");
		goto loadDrivers;
	}
	_end:
	return status;
}

status_t m_init_reloc_stack(){
	status_t status = 0;

	size_t newStackLoc = (size_t) kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
	if(newStackLoc <= KERNEL_STACK_SIZE)
		FERROR(TSX_OUT_OF_MEMORY);

	kernel_move_stack(newStackLoc, KERNEL_STACK_SIZE);
	_end:
	return status;
}


// -----------------------------------------------------------
// MENU
// -----------------------------------------------------------

status_t m_select(){
	kb_on_keypress(NULL);
	serial_on_input(cli_serial_key);
	show_menu:
	m_show_menu();
	while(1){
		if(m_state == KERNEL_STATE_MENU){
			uint16_t key = arch_sleep_kb(1000);
			if(key != 0){
				m_menu_autoboot = FALSE;
				if(key == KEY_ENTER){
					break;
				}else if(key == KEY_P_8){
					if(m_menu_selected > 0)
						m_menu_selected--;
				}else if(key == KEY_P_2){
					if(m_menu_selected < PARSE_MAX_ENTRIES && parse_get_data()->entries[m_menu_selected + 1].name != 0)
						m_menu_selected++;
				}else if(key == KEY_C){
					m_reset_state();
					m_start_console();
				}
			}
			if(m_menu_counter > 0 && m_menu_autoboot)
				m_menu_counter--;
			if(m_menu_counter == 0 && m_menu_autoboot)
				break;
			m_menu_paint();
			m_poll_events();
		}else if(m_state == KERNEL_STATE_CONSOLE){
			arch_sleep(10);
			m_poll_events();
		}else{
			break;
		}
	}
	serial_on_input(NULL);
	m_reset_state();
	return 0;
}

void m_reset_state(){
	if(m_state == KERNEL_STATE_RUN){
		return;
	}else if(m_state == KERNEL_STATE_MENU){
		m_close_menu();
	}else if(m_state == KERNEL_STATE_CONSOLE){
		m_close_console();
	}else{
		log_fatal("Invalid state: %u\n", m_state);
	}
}

// STATE = 1

void m_print_title(char* s){
	size_t cols, rows;
	stdio64_get_text_size(&rows, &cols);
	for(int i = 0; i < cols; i++)
		printCharAt(0x20, 0x70, i, 0);
	printAt(m_title, 0x70, 0, 0);
	printAt(" - ", 0x70, strlen(m_title), 0);
	printAt(s, 0x70, strlen(m_title) + 3, 0);
	printAt(m_version, 0x70, cols - strlen(m_version), 0);
}

void m_show_menu(){
	if(m_state != KERNEL_STATE_RUN)
		return;
	clearScreen(0x7);
	size_t rows;
	stdio64_get_text_size(&rows, NULL);
	setCursorPosition(0, rows - 1);
	m_state = KERNEL_STATE_MENU;
	m_menu_counter = parse_get_data()->timeout;
	m_print_title("Select operating system");
	m_menu_paint();
}

void m_close_menu(){
	if(m_state != KERNEL_STATE_MENU)
		return;
	m_state = KERNEL_STATE_RUN;
	clearScreen(0x7);
	reprintText();
}

void m_menu_paint(){
	if(m_state != KERNEL_STATE_MENU)
		return;
	size_t cols, rows;
	stdio64_get_text_size(&rows, &cols);
	parse_data_t* parse_data = parse_get_data();
	for(int i = 0; i < PARSE_MAX_ENTRIES; i++){
		for(int j = 1; j < cols - 1; j++)
			printCharAt(0x20, (m_menu_selected == i) ? 0x70 : 0x7, j, 2 + i);
		if(parse_data->entries[i].name != 0){
			printAt(parse_data->entries[i].name, (m_menu_selected == i) ? 0x70 : 0x7, 1, 2 + i);
		}
	}
	printAt("Use arrow keys \x18\x19 to select", 0xf, 0, rows - 6);
	printAt("Press ENTER to boot selected entry", 0xf, 0, rows - 5);
	printAt("Press C to enter CLI", 0xf, 0, rows - 4);
	for(int i = 0; i < cols; i++)
		printCharAt(0x20, 0x7, i, rows - 2);
	if(m_menu_autoboot){
		printAt("Autoboot in ", 0x7, 0, rows - 2);
		char* numstr = getDec(m_menu_counter);
		printAt(numstr, 0x7, 12, rows - 2);
		if(m_menu_counter == 1)
			printAt(" second", 0x7, 12 + strlen(numstr), rows - 2);
		else
			printAt(" seconds", 0x7, 12 + strlen(numstr), rows - 2);
	}
}

// STATE = 2

void m_start_console(){
	if(m_state != KERNEL_STATE_RUN)
		return;
	m_state = KERNEL_STATE_CONSOLE;
	printf("\nType &fhelp&7 for a list of commands\n");
	cli_console_reset();
	kb_on_keypress(cli_console_key);
}

void m_close_console(){
	if(m_state != KERNEL_STATE_CONSOLE)
		return;
	kb_on_keypress(NULL);
	m_state = KERNEL_STATE_RUN;
}

void cli_command_menu(uint8_t dest, char* args){
	m_reset_state();
	m_show_menu();
}

void cli_command_boot(uint8_t dest, char* args){
	uint8_t num = util_str_to_int(args);
	if(num >= parse_get_data()->entryCount){
		cli_printf(dest, "&cInvalid number: %s\n", args);
		return;
	}
	m_reset_state();
	m_menu_selected = num;
}


// -----------------------------------------------------------
// START
// -----------------------------------------------------------

status_t m_start(){
	log_info("Selected entry: %s\n", parse_get_data()->entries[m_menu_selected].name);
	status_t status = parse_file_entry(m_menu_selected);
	CERROR();
	parse_entry* entry = &(parse_get_data()->entries[m_menu_selected]);
	bool extLoaded = FALSE;
	kboot_handler* handler = 0;
	loadHandler:
	handler = 0;
	for(int i = 0; i < m_boot_handlers->length; i++){
		kboot_handler* handleri = (kboot_handler*) (m_boot_handlers->base[i]);
		if(handleri->type != NULL && util_str_equals(handleri->type, entry->type)){
			handler = handleri;
		}
	}
	if(handler){
		status = handler->start(entry);
		log_warn("Boot handler for '%s' returned: %u (%s)\n", entry->type, status, errcode_get_name(status));
		FERROR(TSX_ERROR);
	}else{
		log_debug("Attempting to load boot handler for '%s'\n", entry->type);
		status = m_load_external_boot_handler(entry->type);
		if(status != TSX_SUCCESS || extLoaded){
			log_error("Missing or invalid boot handler for type '%s': %u (%s)\n", entry->type, status, errcode_get_name(status));
			ERROR();
		}else{
			extLoaded = TRUE;
			goto loadHandler;
		}
	}
	_end:
	return status;
}

status_t m_load_external_boot_handler(char* type){
	status_t status = 0;
	size_t fileAddr = 0, fileSize = 0;
	char* bhFilePath = kmalloc(MMGR_BLOCK_SIZE);
	if(bhFilePath == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	snprintf(bhFilePath, MMGR_BLOCK_SIZE, "/%s%u.%u/lib/boot/%s.ko", bootDriveType, bootDriveNum, s1data->bootPartN, type);
	status = kernel_read_file(bhFilePath, &fileAddr, &fileSize);
	kfree(bhFilePath, MMGR_BLOCK_SIZE);
	CERROR();
	status = modules_init_boot_handler((elf_file*) fileAddr, type);
	CERROR();
	_end:
	if(fileAddr)
		kfree((void*) fileAddr, fileSize);
	return status;
}

status_t m_load_s3boot(){
	status_t status = 0;
	if(s3boot_loaded)
		goto _end;
	s3bootFilePath = kmalloc(MMGR_BLOCK_SIZE);
	if(s3bootFilePath == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &s3bootFilePath);
	if(m_reloc_base != 0)
		kernel_relocate(0);
	snprintf(s3bootFilePath, MMGR_BLOCK_SIZE, "/%s%u.%u/boot/s3boot", bootDriveType, bootDriveNum, s1data->bootPartN);
	size_t fileSize = 0;
	status = vfs_get_file_size(s3bootFilePath, &fileSize);
	CERROR();
	if(fileSize > KERNEL_S3BOOT_SIZE)
		FERROR(TSX_TOO_LARGE);
	status = kernel_read_file_s(s3bootFilePath, KERNEL_S3BOOT_LOCATION);
	CERROR();
	s3data.magic = S3BOOT_MAGIC;
	s3boot_loaded = TRUE;
	m_start_add_s3boot_map_entry(0x500, KERNEL_S3BOOT_LOCATION - 0x500, 0);
	/*m_start_add_s3boot_map_entry(0x7e00, s1data->mmapStart - 0x7e00, 0);
	m_start_add_s3boot_map_entry(s1data->mmapStart + s1data->mmapLength * sizeof(mmap_entry),
		0x7ffff - (s1data->mmapStart + s1data->mmapLength * sizeof(mmap_entry)), 0);*/
	_end:
	return status;
}

status_t m_s3boot(){
	if(s3data.magic != S3BOOT_MAGIC || s3data.bMode == 0)
		return TSX_ERROR;

#ifdef ARCH_UPSTREAM_x86
	if(s3data.bMode == KERNEL_S3BOOT_BMODE_16 && s1data->bootFlags & S1BOOT_DATA_BOOT_FLAGS_UEFI){
		log_warn("Boot configuration requested to switch to 16-bit real mode and may attempt to use BIOS interrupts, which are not available on UEFI platforms\n");
	}
	if(!(s1data->bootFlags & S1BOOT_DATA_BOOT_FLAGS_UEFI))
		kernel_set_video(80, 25, 16, FALSE);
#endif

	status_t status = arch_platform_reset();
	CERROR();

	status = kernel_exit_uefi();
	CERROR();

	// reset stack
	size_t newStackLoc = ARCH_DEFAULT_STACK_LOCATION;
	size_t cStackLoc = 0;
	ARCH_GET_SP(cStackLoc);
	size_t stackSize = m_stack_top - cStackLoc;
	newStackLoc -= stackSize;
	memcpy((void*) (newStackLoc), (void*) (cStackLoc), stackSize);
	ARCH_SET_SP(newStackLoc);

	ARCH_JUMP_SARG(KERNEL_S3BOOT_LOCATION, (uint32_t) (&s3data));
	HALT();
	_end:
	return status;
}

void m_start_add_s3boot_map_entry(uint32_t base, uint32_t length, uint32_t source){
	s3data.s3mapStart = KERNEL_S3BOOT_LOCATION + KERNEL_S3BOOT_SIZE;
	s3boot_map_entry* map_entries = (s3boot_map_entry*) (KERNEL_S3BOOT_LOCATION + KERNEL_S3BOOT_SIZE);
	map_entries[s3data.s3mapLength].base = base;
	map_entries[s3data.s3mapLength].length = length;
	map_entries[s3data.s3mapLength].source = source;
	s3data.s3mapLength++;
}

void m_start_reserve_s3boot_map_mem_region(uint32_t base, uint32_t length){
	s3boot_map_entry* map_entries = (s3boot_map_entry*) (KERNEL_S3BOOT_LOCATION + KERNEL_S3BOOT_SIZE);
	for(int i = 0; i < s3data.s3mapLength; i++){
		if(map_entries[i].base <= base && map_entries[i].base + map_entries[i].length >= base + length){
			m_start_add_s3boot_map_entry(base + length, map_entries[i].length - length - (base - map_entries[i].base), 0);
			map_entries[i].length = base - map_entries[i].base;
			break;
		}
	}
}


// -----------------------------------------------------------
// MISC
// -----------------------------------------------------------

char* m_resolve_unknown_symbol(size_t addr, size_t* functionOffsetWrite){
	if(addr >= s1data->s1bootStart && addr <= s1data->s1bootEnd){
		*functionOffsetWrite = addr - s1data->s1bootStart;
		return "s1boot";
	}else if(addr >= KERNEL_S3BOOT_LOCATION && addr <= KERNEL_S3BOOT_LOCATION + KERNEL_S3BOOT_SIZE){
		*functionOffsetWrite = addr - KERNEL_S3BOOT_LOCATION;
		return "s3boot";
	}
	return 0;
}


static size_t timerCounter = 0;

void m_on_timer(){
	if(timerCounter > 10){
		timerCounter = 0;
		stdio64_update_screen();
	}
	timerCounter++;
}


void m_poll_events(){
	if(m_event_queue->length > 0){
		event_t* event = list_array_pop(m_event_queue);
		if(!event)
			return;
		(void) ((void (*) (size_t arg)) event->funcptr)(event->arg);
		kfree(event, sizeof(event_t));
	}
}


status_t m_upstream_callback(size_t num, size_t arg0, size_t arg1, size_t arg2){
	status_t status = 0;
	if(!s1data->s1serviceCallback || m_reloc_base != 0){
		FERROR(TSX_UNAVAILABLE);
	}
	bool inte = arch_is_hw_interrupts_enabled();
	arch_disable_hw_interrupts();
	status = s1data->s1serviceCallback(num, arg0, arg1, arg2);
	if(inte)
		arch_enable_hw_interrupts();
	CERROR();
	_end:
	return status;
}



// -----------------------------------------------------------
// ENTRY
// -----------------------------------------------------------

int _start(s1boot_data* data){
	status_t status = 0;

	s1data = data;

	stdio64_set_mode(s1data->videoMode == 1 ? STDIO64_MODE_TEXT : STDIO64_MODE_GRAPHICS, (void*) s1data->framebufferBase,
		s1data->videoWidth, s1data->videoHeight, s1data->videoBpp, s1data->videoPitch);

	size_t cStackLoc = 0;
	ARCH_GET_SP(cStackLoc);
	// add a few bytes for return address and other data
	cStackLoc += ARCH_BITS / 8 * 3;
	// assume stack top is next 0x100 memory block
	m_stack_top = cStackLoc + (0x100 - cStackLoc % 0x100);
	m_stack_size = 0x100;

	status = m_init();
	CERROR();

	arch_sleep(500);

	menu:
	status = m_select();
	CERROR();

	status = m_start();
	if(status != TSX_SUCCESS){
		KERROR();
		log_error("Error %u (%s), press any key to return to menu\n", status, errcode_get_name(status));
		kernel_print_error_trace();
		while(1){
			uint16_t key = arch_sleep_kb(1000);
			if(key != 0)
				goto menu;
		}
	}

	_end:
	printNlnr();
	if(status != 0){
		log_fatal("Initialization error: ");
		printf("&c%u&4 (&f%s&4)\n", status, errcode_get_name(status));
		kernel_print_error_trace();
	}
	printNlnr();
	kernel_halt();
	return status;
}



// -----------------------------------------------------------
// KERNEL UTIL
// -----------------------------------------------------------

s1boot_data* kernel_get_s1data(){
	return s1data;
}

char* kernel_get_boot_label(){
	return bootPartLabel;
}

char* kernel_get_boot_drive_label(){
	return bootDriveLabel;
}

char* kernel_get_boot_drive_type(){
	return bootDriveType;
}

void kernel_get_stack_meta(size_t* stackLocation, size_t* stackSize){
	if(stackLocation)
		*stackLocation = m_stack_top;
	if(stackSize)
		*stackSize = m_stack_size;
}

void kernel_get_brand(char** name, char** versionstr, char** version){
	if(name)
		*name = m_title;
	if(versionstr)
		*versionstr = m_version;
	if(version)
		*version = m_version_r;
}

void kernel_get_current_reloc(size_t* oldAddr, size_t* newAddr){
	if(oldAddr)
		*oldAddr = cRelocOldAddr;
	if(newAddr)
		*newAddr = cRelocNewAddr;
}

size_t kernel_get_reloc_ptr_count(){
	return m_reloc_list->length;
}

status_t kernel_add_event(void (*func), size_t arg){
	event_t* event = kmalloc(sizeof(event_t));
	if(!event)
		return TSX_OUT_OF_MEMORY;
	event->funcptr = func;
	event->arg = arg;
	list_array_push(m_event_queue, event);
	return TSX_SUCCESS;
}

void kernel_stop_autoboot(){
	m_menu_autoboot = FALSE;
}

bool kernel_is_console_running(){
	return m_state == KERNEL_STATE_CONSOLE;
}

elf_loaded_image* kernel_get_image_handle(){
	return m_elf_images->base[m_elf_image_kernel_index];
}


status_t kernel_read_file_s(char* path, size_t dest){
	printNlnr();
	log_debug("Loading %s ", path);
	status_t status = vfs_read_file(path, dest);
	incCursorX();
	incCursorX();
	print(getDec(status), 0xf);
	printNln();
	CERROR();
	_end:
	return status;
}

status_t kernel_read_file(char* path, size_t* destWrite, size_t* sizeWrite){
	printNlnr();
	log_debug("Loading %s ", path);
	status_t status = kernel_read_file_alloc(path, destWrite, sizeWrite);
	incCursorX();
	incCursorX();
	print(getDec(status), 0xf);
	printNln();
	CERROR();
	_end:
	return status;
}

status_t kernel_read_file_alloc(char* path, size_t* destWrite, size_t* sizeWrite){
	status_t status = 0;
	size_t address = 0;
	updateLoadingWheel();
	size_t fileSize = 0;
	status = vfs_get_file_size(path, &fileSize);
	CERROR();
	if(fileSize > MMGR_USABLE_MEMORY)
		FERROR(TSX_TOO_LARGE);
	updateLoadingWheel();
	address = (size_t) kmalloc(fileSize);
	if(address == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	updateLoadingWheel();
	status = vfs_read_file(path, address);
	CERROR();
	updateLoadingWheel();
	if(destWrite)
		*destWrite = address;
	if(sizeWrite)
		*sizeWrite = fileSize;
	_end:
	if(address && status != TSX_SUCCESS)
		kfree((void*) address, fileSize);
	return status;
}

char* kernel_write_boot_file(char* filename){						// Examples when boot partition is "ahci1.0":
	char* repl_bdrive = strrepl(filename, "[BDRIVE]", bootPartLabel); // = ahci1.0
	char* repl_drive = strrepl(repl_bdrive, "[DRIVE]", bootDriveLabel); // = ahci1
	kfree(repl_bdrive, strlen(repl_bdrive) + 1);
	char* repl_part = strrepl(repl_drive, "[PART]", bootPartNumStr); // = 0
	kfree(repl_drive, strlen(repl_drive) + 1);
	char* repl_drivetype = strrepl(repl_part, "[DRIVETYPE]", bootDriveType); // = ahci
	kfree(repl_part, strlen(repl_part) + 1);
	char* repl_drivenum = strrepl(repl_drivetype, "[DRIVENUM]", bootDriveNumStr); // = 1
	kfree(repl_drivetype, strlen(repl_drivetype) + 1);
	return repl_drivenum;
}


void kernel_print_stack_trace(){
	arch_stack_frame* frame = NULL;
	ARCH_GET_FP(frame);
	size_t counter = 0;
	printf("&fStack trace:\n");
	while(frame){
		if(counter > 15){
			printf("&f   [...]\n");
			break;
		}
		size_t functionOffset = 0;
		char* name = kernel_get_symbol_for_address(frame->ip, &functionOffset);
		printf("   %Y (&f%s&7+0x%X)\n", frame->ip, name, functionOffset);
		frame = frame->next;
		counter++;
	}
}

char* kernel_get_symbol_for_address(size_t addr, size_t* functionOffsetWrite){
	char* sym = 0;
	for(int i = 0; i < m_elf_images->length; i++){
		elf_loaded_image* image = m_elf_images->base[i];
		sym = kernel_get_symbol_for_address_in_image(image, addr, functionOffsetWrite);
		if(sym != NULL)
			break;
	}
	if(sym != NULL){
		return sym;
	}else{
		return "UNKNOWN";
	}
}

char* kernel_get_symbol_for_address_in_image(elf_loaded_image* image, size_t addr, size_t* functionOffsetWrite){
	elf_loaded_image_section* symtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_SYMTAB);
	if(symtabsec == 0)
		return 0;
	elf_symtab* symtab = symtabsec->location;

	elf_loaded_image_section* strtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_STRTAB);
	if(strtabsec == 0)
		return 0;
	char* strtab = strtabsec->location;

	size_t md = SIZE_MAX;
	elf_symtab* c = NULL;
	size_t tmpd = 0;
	for(int i = 0; i < symtabsec->length / sizeof(elf_symtab); i++){
		if(!(addr <= (symtab[i].st_value + symtab[i].st_size + image->loadLocation) && addr >= (symtab[i].st_value + image->loadLocation))){
			continue;
		}
		tmpd = addr - (symtab[i].st_value + image->loadLocation);
		if(tmpd < md){
			c = &symtab[i];
			md = tmpd;
		}
	}
	if(c == NULL || strlen(strtab + c->st_name) < 1){
		return m_resolve_unknown_symbol(addr, functionOffsetWrite);
	}
	*functionOffsetWrite = addr - (c->st_value + image->loadLocation);
	return (char*) (strtab + c->st_name);
}

void* kernel_get_address_for_symbol(char* symbol){
	void* addr = NULL;
	for(int i = 0; i < m_elf_images->length; i++){
		elf_loaded_image* image = m_elf_images->base[i];
		addr = kernel_get_address_for_symbol_in_image(image, symbol);
		if(addr != NULL)
			break;
	}
	return addr;
}

void* kernel_get_address_for_symbol_in_image(elf_loaded_image* image, char* symbol){
	elf_loaded_image_section* symtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_SYMTAB);
	if(symtabsec == 0)
		return 0;
	elf_symtab* symtab = symtabsec->location;

	elf_loaded_image_section* strtabsec = elf_get_section_from_image(image, ELF_LI_SECTION_STRTAB);
	if(strtabsec == 0)
		return 0;
	char* strtab = strtabsec->location;

	void* addr = NULL;
	for(int i = 0; i < symtabsec->length / sizeof(elf_symtab); i++){
		elf_symtab* ts = &symtab[i];
		if(util_str_equals(strtab + ts->st_name, symbol)){
			addr = (void*) (ts->st_value + image->loadLocation);
			break;
		}
	}
	return addr;
}


void kernel_halt(){
	printf("Halted\n");
	stdio64_update_screen();
	_halt:
	HALT();
	goto _halt;
}


void kernel_move_stack(size_t stackTop, size_t newStackSize){
	size_t oldStackTop = m_stack_top;
	m_stack_top = stackTop;
	m_stack_size = newStackSize;
	size_t cStackLoc = 0;
	size_t newStackLoc = stackTop;
	ARCH_GET_SP(cStackLoc);
	size_t stackSize = oldStackTop - cStackLoc;
	newStackLoc -= stackSize;
	memcpy((void*) (newStackLoc), (void*) (cStackLoc), stackSize);
	ARCH_SET_SP(newStackLoc);

	arch_stack_frame* frame = 0;
	ARCH_GET_FP(frame);
	while(frame){
		arch_stack_frame* next = frame->next;
		frame = (arch_stack_frame*) ((size_t) frame - cStackLoc + newStackLoc);
		if((size_t) frame->next >= cStackLoc)
			frame->next = ((arch_stack_frame*) ((size_t) frame->next - cStackLoc + newStackLoc));
		frame = next;
	}
}


status_t kernel_set_video(size_t width, size_t height, size_t bpp, bool graphics){
	bool inte = arch_is_hw_interrupts_enabled();
	arch_disable_hw_interrupts();
	ucallback_videomode vidmode;
	vidmode.width = width;
	vidmode.height = height;
	vidmode.bpp = bpp;
	vidmode.mode = graphics ? 1 : 0;
	log_debug("Attempting to set video %ux%ux%u\n", vidmode.width, vidmode.height, vidmode.bpp);
	status_t status = m_upstream_callback(10, (size_t) &vidmode, 0, 0);
	CERROR();
	for(size_t addr = 0; addr < vidmode.bytesPerLine * vidmode.height; addr += 0x1000){
		vmmgr_map_page(vidmode.framebuffer + addr, vidmode.framebuffer + addr + m_reloc_base);
	}
	stdio64_set_mode(graphics ? STDIO64_MODE_GRAPHICS : STDIO64_MODE_TEXT, (void*) (vidmode.framebuffer + m_reloc_base),
		vidmode.width, vidmode.height, vidmode.bpp, vidmode.bytesPerLine);
	reprintText();
	log_debug("Video %ux%ux%u %u fb=0x%X\n", vidmode.width, vidmode.height, vidmode.bpp, (size_t) status, vidmode.framebuffer);
	_end:
	if(inte)
		arch_enable_hw_interrupts();
	return status;
}


void kernel_print_error_trace(){
	if(!m_error_stack)
		return;
	size_t lastRBP = 0;
	for(int i = 0; i < m_error_stack->length; i++){
		if(i > 10){
			printf("&f   [...]\n");
			break;
		}
		kernel_error_frame* errt = m_error_stack->base[i];
		if(lastRBP != 0 && lastRBP != errt->fp){
			printf("&e   ...\n");
		}
		size_t functionOffset = 0;
		char* name = kernel_get_symbol_for_address(errt->ip, &functionOffset);
		printf("   %Y (&f%s&7+0x%X)\n", errt->ip, name, functionOffset);
		lastRBP = errt->nextFp;
	}
	kernel_delete_error_stack();
}

bool kernel_create_error_stack(){
	if(m_init_state < 2)
		return FALSE;
	m_error_stack = list_array_create(0);
	return m_error_stack != NULL;
}

void kernel_delete_error_stack(){
	if(!m_error_stack)
		return;
	for(int i = 0; i < m_error_stack->length; i++){
		kfree(m_error_stack->base[i], sizeof(kernel_error_frame));
	}
	list_array_delete(m_error_stack);
	m_error_stack = NULL;
}

void kernel_error_trace(){
	if(!m_error_stack){
		if(!kernel_create_error_stack())
			return;
	}
	arch_stack_frame* frame = NULL;
	ARCH_GET_FP(frame);
	kernel_error_frame* save = kmalloc(sizeof(kernel_error_frame));
	if(!save)
		return;
	save->fp = (size_t) frame;
	save->ip = frame->ip;
	save->nextFp = (size_t) frame->next;
	list_array_push(m_error_stack, save);
}


void kernel_add_reloc_ptr(void** ptr){
	if(!m_reloc_list)
		return;
	if((size_t) ptr <= m_reloc_base)
		return;
	bool exists = FALSE;
	for(int i = 0; i < m_reloc_list->length; i++){
		if((size_t) (m_reloc_list->base[i]) == (size_t) ptr){
			exists = TRUE;
		}
	}
	if(!exists){
		list_array_push(m_reloc_list, (void*) (((size_t) ptr) - m_reloc_base));
	}else{
		log_warn("Attempted to add reloc_ptr at 0x%X but it already exists\n", (size_t) ptr);
	}
}

void kernel_del_reloc_ptr(void** ptr){
	if(!m_reloc_list)
		return;
	for(int i = 0; i < m_reloc_list->length; i++){
		if((size_t) (m_reloc_list->base[i]) == (size_t) ptr - m_reloc_base){
			m_reloc_list->base[i] = NULL;
		}
	}
}

status_t kernel_relocate(size_t newAddr){
	if(arch_is_hw_interrupt_running()) // x86: interrupt may save sp to stack (relocation will not work in that case)
		return TSX_ILLEGAL_STATE;
	newAddr -= newAddr & MMGR_USABLE_MEMORY;
	if(newAddr == m_reloc_base)
		return TSX_SUCCESS;
#ifdef ARCH_i386
	/* this will not work on i386 because the compiler will add code to figure out the current running location at the beginning of every function* in PIC and
	   store this value as a hidden local variable. As this function will likely never be the top function on the stack, all caller functions will continue
	   to use the old values of where they are running and likely cause a page fault because they use that value to calculate the offset to global variables etc.
	   On amd64 the compiler will instead use rip-relative addressing (not available on i386) to do that (which is why this works on there).
	   *search for a code pattern like this:
			sub esp, ...
			call	0 <(function)+..>
			pop	eax
			add	eax, <somewhere>
			; .. possibly other code ..
			mov	dword ptr [ebp - <somewhere on the stack>], eax
			...
	*/
	return TSX_UNSUPPORTED;
#endif
	bool inte = arch_is_hw_interrupts_enabled();
	arch_disable_hw_interrupts();
	size_t oldAddr = m_reloc_base;

	cRelocOldAddr = oldAddr;
	cRelocNewAddr = newAddr;

	mmap_entry* vmemmap = kmalloc(MMGR_BLOCK_SIZE);
	size_t vmemmaplength;
	vmmgr_gen_vmmap(vmemmap, MMGR_BLOCK_SIZE, &vmemmaplength);

	for(int i = 0; i < vmemmaplength; i++){
		if(vmemmap[i].addr + vmemmap[i].size - oldAddr > MMGR_USABLE_MEMORY + 1){
			log_warn("Cannot map address space 0x%X - 0x%X because it is out of range\n", vmemmap[i].addr, vmemmap[i].addr + vmemmap[i].size - 1);
			continue;
		}
		//log_debug("map %Y - %Y\n", vmemmap[i].addr - oldAddr + newAddr, vmemmap[i].addr + vmemmap[i].size - oldAddr + newAddr - 1);
		for(size_t addr = 0; addr < vmemmap[i].size; addr += MMGR_BLOCK_SIZE){
			vmmgr_map_page(vmemmap[i].addr + addr - oldAddr, vmemmap[i].addr + addr - oldAddr + newAddr);
		}
	}

	kernel_relocate_pointers(oldAddr, newAddr);
	mmgr_relocation(oldAddr, newAddr);
	arch_relocation(oldAddr, newAddr);
	kernel_relocate_stack(oldAddr, newAddr);

	status_t status = 0;
	for(int i = 0; i < m_elf_images->length; i++){
		elf_loaded_image* image = (elf_loaded_image*) (m_elf_images->base[i]);
		status = dynl_link_image_to_image(image, m_elf_images->base[m_elf_image_kernel_index]);
		if(status != TSX_SUCCESS){
			log_warn("Error during relocation: %u (%s)\n", (size_t) status, errcode_get_name(status));
			kernel_print_error_trace();
		}
	}

	vmemmap = (mmap_entry*) ((size_t) vmemmap - oldAddr + newAddr);

	for(int i = 0; i < vmemmaplength; i++){
		if(vmemmap[i].addr + vmemmap[i].size - oldAddr > MMGR_USABLE_MEMORY + 1){
			log_warn("Cannot unmap address space 0x%X - 0x%X because it is out of range\n", vmemmap[i].addr, vmemmap[i].addr + vmemmap[i].size - 1);
			continue;
		}
		//log_debug("unmap %Y - %Y\n", vmemmap[i].addr, vmemmap[i].addr + vmemmap[i].size - 1);
		for(size_t addr = 0; addr < vmemmap[i].size; addr += MMGR_BLOCK_SIZE){
			vmmgr_unmap_page(vmemmap[i].addr + addr);
		}
	}
	kfree(vmemmap, MMGR_BLOCK_SIZE);
	vmmgr_cleanup();

	cRelocOldAddr = 0;
	cRelocNewAddr = 0;

	m_reloc_base = newAddr;
	if(inte)
		arch_enable_hw_interrupts();

	return status;
}

void kernel_relocate_pointers(size_t oldAddr, size_t newAddr){
	for(int i = 0; i < m_reloc_list->length; i++){
		if(m_reloc_list->base[i] != NULL && *((size_t*) ((size_t) m_reloc_list->base[i] + oldAddr)) > oldAddr /* implies it is not NULL */
			&& *((size_t*) ((size_t) m_reloc_list->base[i] + oldAddr)) <= oldAddr + MMGR_USABLE_MEMORY)
			*((size_t*) ((size_t) m_reloc_list->base[i] + oldAddr)) = *((size_t*) ((size_t) m_reloc_list->base[i] + oldAddr)) - oldAddr + newAddr;
	}
}

void kernel_relocate_stack(size_t oldAddr, size_t newAddr){
	// stack return address relocation
	arch_stack_frame* frame = NULL;
	ARCH_GET_FP(frame);
	while(frame){
		arch_stack_frame* next = frame->next;
		if(frame->ip > oldAddr){
			frame->ip = frame->ip - oldAddr + newAddr;
		}else{
			log_warn("Stack return address relocation failed: Address is out of range (0x%X)\n", frame->ip);
		}
		if((size_t) frame->next > oldAddr && (size_t) frame->next <= oldAddr + MMGR_USABLE_MEMORY){
			frame->next = ((arch_stack_frame*) ((size_t) frame->next - oldAddr + newAddr));
		}else if(frame->next != 0){
			log_warn("Stack base pointer relocation failed: Address is out of range (0x%X)\n", frame->next);
		}
		frame = next;
	}

	// stack pointer relocation
	size_t stackLocation = 0;
	ARCH_GET_SP(stackLocation);
	if(stackLocation >= oldAddr && stackLocation <= oldAddr + MMGR_USABLE_MEMORY){
		stackLocation = stackLocation - oldAddr + newAddr;
		ARCH_SET_SP(stackLocation);
	}else{
		log_warn("Stack pointer relocation failed: Address is out of range (0x%X)\n", stackLocation);
	}
}

size_t kernel_get_reloc_base(){
	return m_reloc_base;
}


void kernel_runtime_assertion(bool x, char* msg){
	if(!x){
		log_fatal("Assertion failed: %s\n", msg);
		kernel_print_stack_trace();
		kernel_halt();
	}
}


size_t kernel_pseudorandom(size_t max){
	if(max == 0)
		return 0;
	rand_next = rand_next * 1103515245 + 12345;
	return (size_t) (rand_next) % max;
}

void kernel_pseudorandom_seed(size_t seed){
	rand_next = seed;
}


status_t kernel_exit_uefi(){
	status_t status = 0;
	if(!(s1data->bootFlags & S1BOOT_DATA_BOOT_FLAGS_UEFI))
		goto _end;
	status = m_upstream_callback(9, 0, 0, 0);
	CERROR();
	_end:
	return status;
}


void kernel_s3boot_add_mem_region(uint32_t base, uint32_t length, uint32_t source){
	m_start_add_s3boot_map_entry(base, length, source);
}

void kernel_s3boot_reserve_mem_region(uint32_t base, uint32_t length){
	m_start_reserve_s3boot_map_mem_region(base, length);
}

void kernel_jump(arch_os_entry_state* entryState, size_t dest, size_t mode, uint32_t archFlags){
	s3data.jmp = dest;
	s3data.entryStateStruct = (uint64_t) entryState;
	s3data.bMode = mode;
	s3data.archFlags = archFlags;
	if(!entryState->sp){
		entryState->sp = ARCH_DEFAULT_STACK_LOCATION;
	}
	status_t status = m_load_s3boot();
	if(status == TSX_SUCCESS)
		status = m_s3boot();
	log_fatal("s3boot call failed: %u (%s)\n", status, errcode_get_name(status));
	kernel_halt();
}


void kernel_add_boot_handler(char* type, void* start){
	kboot_handler* handler = kmalloc(sizeof(kboot_handler));
	if(!handler)
		return;
	handler->type = type;
	handler->start = start;
	reloc_ptr((void**) &handler->type);
	reloc_ptr((void**) &handler->start);
	list_array_push(m_boot_handlers, handler);
}

int kernel_add_elf_image(elf_file* file, size_t loadLocation){
	elf_loaded_image* image = kmalloc(sizeof(elf_loaded_image));
	if(!image)
		return -1;
	elf_gen_loaded_image_data(file, loadLocation, image);
	list_array_push(m_elf_images, image);
	return m_elf_images->length - 1;
}


