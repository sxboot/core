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
 * stdio64.c - Text driver for x86.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/stdio64.h>
#include <x86/x86.h>
#include <x86/stdio64.h>

extern size_t cursorX;
extern size_t cursorY;
extern size_t totalX;
extern size_t totalY;

extern uint8_t videoMode;
extern void* videoMem;
extern size_t videoWidth;
extern size_t videoHeight;
extern size_t videoBpp;
extern size_t videoBytesPerPixel;
extern size_t videoBytesPerLine;
extern size_t videoMemLen;

extern void* videoMemBuf;
extern size_t videoMemBufLen;
extern size_t prevCursorX;
extern size_t prevCursorY;

extern uint64_t font7x8[];

extern uint32_t cgaColors[];



// TEXT MODE
size_t stdio64_archtex_get_fb_size(){
	return videoWidth * videoHeight * 2;
}

void stdio64_archtex_get_texdims(size_t* totalX, size_t* totalY){
	*totalX = videoWidth;
	*totalY = videoHeight;
}

void stdio64_archtex_delChar(){
	uint16_t* addr = (uint16_t*) ((size_t) videoMem + (cursorX + cursorY * videoWidth - 1) * 2);
	*addr = 0x0720;
	if(cursorX <= 0){
		cursorX = videoWidth - 1;
		shiftDown();
	}else{
		cursorX--;
	}
}

void stdio64_archtex_printCharAt(char ch, uint8_t attr, size_t x, size_t y){
	uint8_t* addr = (uint8_t*) ((size_t) videoMem + (x + y * videoWidth) * 2);
	*addr = ch;
	*(addr + 1) = attr;
}

void stdio64_archtex_shiftUp(){
	uint16_t* addr = (uint16_t*) (videoMem);
	for(int i = 0; i < videoWidth * (videoHeight - 1); i++){
		*addr = *(addr + videoWidth);
		addr++;
	}
	for(int i = 0; i < videoWidth; i++){
		*addr = 0x0720;
		addr++;
	}
}

void stdio64_archtex_shiftDown(){
	uint16_t* addr = (uint16_t*) (videoMem + 0xf9c);
	for(int i = 0; i < videoWidth * (videoHeight - 1); i++){
		*addr = *(addr - videoWidth);
		addr--;
	}
	for(int i = 0; i < videoWidth; i++){
		*addr = 0x0720;
		addr--;
	}
}

void stdio64_archtex_clearScreen(uint8_t attr){
	uint16_t* addr = (uint16_t*) (videoMem);
	for(int i = 0; i < videoWidth * videoHeight; i++){
		*addr = ((uint16_t) 0x20) | (((uint16_t) attr) << 8);
		addr++;
	}
	cursorX = 0;
	cursorY = 0;
}

void stdio64_archtex_updateCursor(){
	uint16_t num = cursorY * videoWidth + cursorX;
	x86_outb(0x3d4, 0xe);
	x86_outb(0x3d5, (num >> 8) & 0xff);
	x86_outb(0x3d4, 0xf);
	x86_outb(0x3d5, num & 0xff);
}

void stdio64_archtex_updateCursorMemData(){
	uint16_t num = 0;
	x86_outb(0x3d4, 0xe);
	num |= ((uint16_t) x86_inb(0x3d5)) << 8;
	x86_outb(0x3d4, 0xf);
	num |= x86_inb(0x3d5);
	cursorX = num % videoWidth;
	cursorY = num / videoWidth;
}

void stdio64_archtex_updateLoadingWheel(){
	uint8_t* addr = (uint8_t*) ((size_t) videoMem + (cursorX + cursorY * videoWidth) * 2);
	uint8_t cchar = *addr;
	uint8_t nchar = 0;
	if(cchar == '-')
		nchar = '\\';
	else if(cchar == '\\')
		nchar = '|';
	else if(cchar == '|')
		nchar = '/';
	else if(cchar == '/')
		nchar = '-';
	else
		nchar = '-';
	*addr = nchar;
}

