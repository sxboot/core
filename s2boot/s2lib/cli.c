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
 * cli.c - Command Line Interface.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/stdio.h>
#include <klibc/string.h>
#include <kernel/stdio64.h>
#include <kernel/serial.h>
#include <kernel/mmgr.h>
#include <kernel/kutil.h>
#include <kernel/kb.h>
#include <kernel/list.h>
#include <kernel/vfs.h>
#include <kernel/util.h>
#include <kernel/cli.h>


static char* console_text;
static uint16_t console_text_length = 0;

static char* serial_text;
static uint16_t serial_text_length = 0;


status_t cli_init(){
	status_t status = 0;
	console_text = (char*) kmalloc(MMGR_BLOCK_SIZE);
	if(console_text == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &console_text);
	serial_text = (char*) kmalloc(MMGR_BLOCK_SIZE);
	if(serial_text == 0)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &serial_text);
	_end:
	return status;
}


void cli_console_key(uint16_t c){
	if(c == KEY_ENTER){
		console_text[console_text_length] = 0;
		printf("\n");
		cli_command(0, console_text);
		if(kernel_is_console_running())
			cli_console_reset();
	}else if(c == KEY_BACKSPACE){
		if(console_text_length <= 0)
			return;
		console_text[console_text_length--] = 0;
		delChar();
	}else{
		if(console_text_length >= MMGR_BLOCK_SIZE - 8)
			return;
		console_text[console_text_length++] = (char) c;
		printChar((char) c, 0x7);
	}
}

void cli_serial_key(uint8_t c){
	kernel_stop_autoboot();
	if(c == '\n' || c == '\r'){
		serial_text[serial_text_length] = 0;
		serial_write('\r');
		serial_write('\n');
		cli_command(1, serial_text);
		serial_text_length = 0;
		serial_text[serial_text_length] = 0;
		serial_text[serial_text_length + 1] = 0;
	}else if(c == 127 || c == 8){
		if(serial_text_length <= 0)
			return;
		serial_text[serial_text_length--] = 0;
		serial_write(c);
	}else{
		if(serial_text_length >= MMGR_BLOCK_SIZE - 8)
			return;
		serial_text[serial_text_length++] = (char) c;
		serial_write(c);
	}
}

void cli_console_reset(){
	console_text_length = 0;
	console_text[console_text_length] = 0;
	console_text[console_text_length + 1] = 0;
	printNlnr();
	printf("> ");
}


bool cli_reflect_call(char* cmd, uint8_t dest, char* args){
	size_t namelen = strlen(cmd) + 13; // 13 = strlen("cli_command_") + 1
	char name[namelen];
	memset(name, 0, namelen);
	snprintf(name, namelen, "cli_command_%s", cmd);
	void* addr = kernel_get_address_for_symbol(name);
	if(addr){
		(void) (((void (*) (uint8_t, char*)) addr)(dest, args));
		return TRUE;
	}else{
		return FALSE;
	}
}

void cli_command(uint8_t dest, char* s){
	if(strlen(s) < 1)
		return;
	int slen = strlen(s);
	for(int i = 0; i < slen; i++){
		if(s[i] == ' '){
			s[i] = 0;
		}
	}
	if(!cli_reflect_call(s, dest, s + strlen(s) + 1)){
		cli_printf(dest, "Command not found\n");
	}
}

void cli_printf(uint8_t dest, char* str, ...){
	va_list args;
	va_start(args, str);
	if(dest == 0){
		vprintf(str, args);
	}else if(dest == 1){
		va_list args2;
		va_copy(args2, args);
		int size = vsnprintf(NULL, 0, str, args) + 1;
		if(size > 0){
			char buf[size];
			memset(buf, 0, size);
			vsnprintf(buf, size, str, args2);
			for(size_t i = 0; i < strlen(buf); i++){
				if(buf[i] == '\n')
					serial_write('\r');
				serial_write(buf[i]);
			}
		}
		va_end(args2);
	}
	va_end(args);
}



void cli_command_help(uint8_t dest, char* args){
	cli_printf(dest, "Builtin commands:\n&fhelp&7: shows this list\n&fmenu&7: returns to menu\n&fboot <index>&7: boots the entry with the given index starting at 0\n\
&fmem&7: Shows memory usage information\n&fmemmap&7: shows the memory allocation map\n&fcat <file>&7: print the contents of a file\n\
&fls <directory>&7: print the contents of a directory\n");
}

// menu and boot commands are in kernel.c

void cli_command_mem(uint8_t dest, char* args){
	cli_printf(dest, "Total memory: %uMiB\n", mmgr_get_total_memory() / 1024 / 1024);
	cli_printf(dest, "Used memory: %u/%uKiB (%u of %u allocatable blocks)\n", mmgr_get_used_mem_kib(), mmgr_get_total_blocks() * MMGR_BLOCK_SIZE / 1024,
		mmgr_get_used_blocks(), mmgr_get_total_blocks());
	cli_printf(dest, "   blocks: %u reserved, %u allocated, %u paging\n", mmgr_get_used_blocks_reserved(), mmgr_get_used_blocks_alloc(), mmgr_get_used_blocks_paging());
	size_t heapMem = 0;
	size_t heapBlocks = 0;
	kmem(&heapMem, &heapBlocks);
	size_t heapRealMem = heapBlocks * MMGR_BLOCK_SIZE / 1024;
	cli_printf(dest, "Heap: %uKiB / %u blocks (%uKiB, %u%%)\n", heapMem, heapBlocks, heapRealMem, heapMem * 100 / heapRealMem);
	cli_printf(dest, "Other: %u relocatable pointers\n", kernel_get_reloc_ptr_count());
}

void cli_command_memmap(uint8_t dest, char* args){
	mmap_entry* addr = kmalloc(MMGR_BLOCK_SIZE);
	size_t length;
	mmgr_gen_mmap(addr, MMGR_BLOCK_SIZE, &length);
	for(int i = 0; i < length; i++){
		cli_printf(dest, "%Y - %Y (%uKiB, type %u)\n", addr[i].addr, addr[i].addr + addr[i].size - 1, addr[i].size / 1024, (size_t) addr[i].type);
	}
	kfree(addr, MMGR_BLOCK_SIZE);
}

void cli_command_vmemmap(uint8_t dest, char* args){
	mmap_entry* addr = kmalloc(MMGR_BLOCK_SIZE);
	size_t length;
	vmmgr_gen_vmmap(addr, MMGR_BLOCK_SIZE, &length);
	for(int i = 0; i < length; i++){
		cli_printf(dest, "%Y - %Y (%uKiB)\n", addr[i].addr, addr[i].addr + addr[i].size - 1, addr[i].size / 1024);
	}
	kfree(addr, MMGR_BLOCK_SIZE);
}

void cli_command_bootdrive(uint8_t dest, char* args){
	cli_printf(dest, "%s\n", kernel_get_boot_label());
}

void cli_command_cat(uint8_t dest, char* args){
	char* path = kernel_write_boot_file(args);
	size_t raddr = 0;
	size_t fileSize = 0;
	status_t status = kernel_read_file(path, &raddr, &fileSize);
	kfree(path, strlen(path));
	cli_printf(dest, "File at %Y (%u bytes):\n\n", raddr, fileSize);
	for(size_t i = 0; i < fileSize; i++){
		cli_printf(dest, "%c", *((char*) (raddr + i)));
	}
	kfree((void*) raddr, fileSize);
	if(status != TSX_SUCCESS){
		cli_printf(dest, "Error %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
	}
}

void cli_command_ls(uint8_t dest, char* args){
	char* path = kernel_write_boot_file(args);
	list_array* list = NULL;
	status_t status = vfs_list_dir(path, &list);
	kfree(path, strlen(path));
	if(status != TSX_SUCCESS)
		goto _ls_error;
	cli_printf(dest, "Directory (%u entries)\n", list->length);
	for(size_t i = 0; i < list->length; i++){
		cli_printf(dest, "%s\n", (char*) list->base[i]);
		kfree(list->base[i], strlen(list->base[i]) + 1);
	}
	list_array_delete(list);
	_ls_error:
	if(status != TSX_SUCCESS){
		cli_printf(dest, "Error %u (%s)\n", (size_t) status, errcode_get_name(status));
		kernel_print_error_trace();
	}
}

void cli_command_dbgstack(uint8_t dest, char* args){
	kernel_print_stack_trace(); // fix for serial
}

void cli_command_dbgreloc(uint8_t dest, char* args){
	char* numstr = args;
	size_t num = util_str_to_int(numstr);
	cli_printf(dest, "Reloc to %Y\n", num);
	status_t status = kernel_add_event(kernel_relocate, num);
	if(status != TSX_SUCCESS){
		cli_printf(dest, "&cError %u (%s)\n", status, errcode_get_name(status));
	}
}

void cli_command_dbgvid(uint8_t dest, char* args){
	char* numstr = args;
	size_t width = util_str_to_int(numstr);
	size_t height = util_str_to_int(numstr + strlen(numstr) + 1);
	status_t status = 0;
	if(width == 1){
		status = kernel_set_video(640, 480, 32, TRUE);
	}else if(width == 2){
		status = kernel_set_video(800, 600, 32, TRUE);
	}else if(width == 3){
		status = kernel_set_video(1024, 768, 32, TRUE);
	}else if(width == 4){
		status = kernel_set_video(1280, 1024, 32, TRUE);
	}else if(width == 5){
		status = kernel_set_video(1600, 900, 32, TRUE);
	}else if(width == 6){
		status = kernel_set_video(320, 200, 32, TRUE);
	}else if(width == 7){
		status = kernel_set_video(720, 480, 32, TRUE);
	}else if(width == 8){
		status = kernel_set_video(80, 25, 16, FALSE);
	}else if(width == 9){
		status = kernel_set_video(80, 60, 16, FALSE);
	}else{
		status = kernel_set_video(width, height, 32, TRUE);
	}
	cli_printf(dest, "status %u\n", (size_t) status);
	kernel_print_error_trace();
}

void cli_command_dbgdrive(uint8_t dest, char* args){
	char* msg = kernel_write_boot_file(args);
	cli_printf(dest, "%s\n", msg);
	kfree(msg, strlen(msg));
}

void cli_command_dbgrand(uint8_t dest, char* args){
	cli_printf(dest, "%u\n", kernel_pseudorandom(SIZE_MAX));
}

void cli_command_dbgtime(uint8_t dest, char* args){
	cli_printf(dest, "%u\n", (size_t) arch_real_time());
}

void cli_command_dbgscale(uint8_t dest, char* args){
	size_t scale = util_str_to_int(args);
	stdio64_set_font_scale(scale);
	size_t rows, cols;
	stdio64_get_text_size(&rows, &cols);
	cli_printf(dest, "%ux%u cells\n", cols, rows);
}

#ifdef ARCH_UPSTREAM_x86
void cli_command_e820map(uint8_t dest, char* args){
	mmgr_arch_mmap_entry* addr = 0;
	uint16_t length = 0;
	mmgr_get_system_map(&addr, &length);
	for(int i = 0; i < length; i++){
		cli_printf(dest, "%Y - %Y (%uKiB, type %u)\n", (size_t) addr[i].addr, (size_t) (addr[i].addr + addr[i].size - 1),
			(size_t) (addr[i].size / 1024), (size_t) addr[i].type);
	}
}
#endif

