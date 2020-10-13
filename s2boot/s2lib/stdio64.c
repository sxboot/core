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
 * stdio64.c - Display and text driver.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/serial.h>
#include <kernel/stdio64.h>
#include <arch/stdio64.h>

static char* msgPre = "[s2] ";

static char* decTmp = "000000000000000000000";

#if ARCH_BITS == 64
#define STDIO64__HEX_LEN 16
static char* hexTmp = "0x0000000000000000";
#elif ARCH_BITS == 32
#define STDIO64__HEX_LEN 8
static char* hexTmp = "0x00000000";
#endif

static uint16_t* textLog = 0;
static size_t textLogIndex = 0;

size_t cursorX = 0;
size_t cursorY = 0;
size_t totalX = 0;
size_t totalY = 0;

uint8_t videoMode = 0;
void* videoMem = 0;
size_t videoWidth = 0;
size_t videoHeight = 0;
size_t videoBpp = 0;
size_t videoBytesPerPixel = 0;
size_t videoBytesPerLine = 0;
size_t videoMemLen = 0;

void* videoMemBuf = 0;
size_t videoMemBufLen = 0;
size_t prevCursorX = 0;
size_t prevCursorY = 0;

uint64_t font7x8[] = {
	0x0000000000000000,
	0x00003e455151453e,
	0x00003e7b6f6f7b3e,
	0x00001c3e7c7c3e1c,
	0x0000081c3e1c0800,
	0x0000185c66665c18,
	0x0000185c7e7e5c18,
	0x0000000018180000,
	0x00007f7f67677f7f,
	0x0000001824241800,
	0x00007f435b5b437f,
	0x000007334d493000,
	0x0000000629792906,
	0x0000000705057f60,
	0x0000007f65057f60,
	0x0000002a3e633e2a,
	0x000000081c3e7f7f,
	0x00007f7f3e1c0800,
	0x00002466ffff6624,
	0x0000006f6f006f6f,
	0x0000007f017f0502,
	0x0000225955554d22,
	0x0000707070707070,
	0x000040527f7f5240,
	0x000004067f7f0604,
	0x000010307f7f3010,
	0x0000183c7e181818,
	0x00001818187e3c18,
	0x0000202020202038,
	0x0000183c18183c18,
	0x0000181c1e1e1c18,
	0x00000c1c3c3c1c0c,
	0x0000000000000000, // space
	0x000000006f6f0000, // !
	0x00000e0e000e0e00, //
	0x0000227f22227f22, // #
	0x0000122a7f7f2a24, // $
	0x0000606608106606, // %
	0x0000522559493600, // &
	0x000000060e000000, //
	0x00000041633e0000, // (
	0x000000003e634100, // )
	0x000004150e0e1504, // *
	0x00000008083e0808, // +
	0x0000000000206000, // ,
	0x0000000808080808, // -
	0x0000000000606000,
	0x000001071c704000,
	0x0000003e415d413e,
	0x000040407f414200,
	0x0000004649495162,
	0x0000003649494922,
	0x000000107f121418,
	0x000000314949494f,
	0x0000003049494a3c,
	0x0000000305097101,
	0x0000003649494936,
	0x0000003e49494906,
	0x0000000066660000,
	0x0000000026660000,
	0x0000004122140800,
	0x0000002424242400,
	0x0000000814224100,
	0x0000000609590102,
	0x0000005f555d413e,
	0x0000007e0909097e, // A
	0x000000364949497f, // B
	0x000022414141413e,
	0x00003e4141417f41,
	0x000041414949497f,
	0x000001010909097f,
	0x000072294949413e,
	0x0000007f0808087f,
	0x00000041417f4141,
	0x000000013f414120,
	0x000000412214087f,
	0x000000404040407f,
	0x00007f020404027f,
	0x00007f100804027f,
	0x00003e414141413e,
	0x000000060909097f,
	0x00007e615141413e,
	0x000000462919197f,
	0x0000324949494926, // S
	0x00000001017f0101,
	0x0000003f4040403f,
	0x0000001f2040201f,
	0x00003f601010603f,
	0x000041221c1c2241,
	0x0000010678780601,
	0x000043474d597161,
	0x00000041417f0000,
	0x00000060380e0300,
	0x0000007f41410000,
	0x0000040201010204,
	0x0000404040404040,
	0x0000000000040703,
	0x0000407844444438, // a
	0x000000384444447f, // b
	0x0000000044444438, // c
	0x0000007f44444438, // d
	0x0000000854545438, // e
	0x00000001097e0800, // f
	0x000004fca4a49800, // g
	0x000000780404047f, // h
	0x00000000007a0000, // i
	0x000000003a404020, // j
	0x000000004828107f, // k
	0x0000000020403f00, // l
	0x000078047804787c, // m
	0x000000780404787c, // n
	0x0000003844444438, // o
	0x00000000182424fc, // p
	0x00000000fc242418, // q
	0x000000000804087c, // r
	0x0000002454545448, // s
	0x00000020443f0400, // t
	0x000000403c40403c, // u
	0x0000001c2040201c, // v
	0x0000003c403c403c, // w
	0x0000004428102844, // x
	0x000000000c30cc00, // y
	0x000000004c545464, // z
	0x0000000041413608,
	0x000000007f7f0000,
	0x0000000008364141,
	0x0000081010080810,
	0x0000784442447800,
	0x00007f7f7f7f7f7f // for chars above 127
};

uint32_t cgaColors[] = {
	0x000000,
	0x0000aa,
	0x00aa00,
	0x00aaaa,
	0xaa0000,
	0xaa00aa,
	0xaa5500,
	0xaaaaaa,
	0x555555,
	0x5555ff,
	0x55ff55,
	0x55ffff,
	0xff5555,
	0xff55ff,
	0xffff55,
	0xffffff
};


status_t stdio64_init(){
	status_t status = 0;
	reloc_ptr(&videoMem);
	reloc_ptr(&videoMemBuf);
	reloc_ptr((void**) &textLog);
	status = stdio64_reallocmem();
	CERROR();
	textLog = kmalloc(STDIO64_TEXT_LOG_LENGTH * sizeof(uint16_t));
	if(!textLog)
		FERROR(TSX_OUT_OF_MEMORY);
	memset(textLog, 0, STDIO64_TEXT_LOG_LENGTH * sizeof(uint16_t));
	_end:
	return status;
}

status_t stdio64_reallocmem(){
	status_t status = 0;
	if(videoMemBuf){
		kfree(videoMemBuf, videoMemBufLen);
		videoMemBuf = NULL;
		videoMemBufLen = 0;
	}
	if(videoMemLen){
		videoMemBuf = kmalloc(videoMemLen);
		if(videoMemBuf == 0)
			FERROR(TSX_OUT_OF_MEMORY);
		videoMemBufLen = videoMemLen;
	}
	_end:
	return status;
}


void stdio64_save_vid_mem(){
	if(videoMemBuf == 0)
		return;
	memcpy(videoMemBuf, videoMem, videoMemLen);
	saveCursorPosition();
}

void stdio64_restore_vid_mem(){
	if(videoMemBuf == 0)
		return;
	memcpy(videoMem, videoMemBuf, videoMemLen);
	restoreCursorPosition();
}

void stdio64_set_mode(uint8_t mode, void* framebuffer, size_t width, size_t height, size_t bpp, size_t bytesPerLine){
#if STDIO64_ARCH_HAS_TEXT == 0
	if(mode == STDIO64_MODE_TEXT){
		if(framebuffer){
			// there is nothing else we can do except write something to the start of the framebuffer to indicate this error because the framebuffer structure is unknown
			for(size_t i = 0; i < 5; i++){
				*((uint8_t*) framebuffer + i) = 0xff;
			}
		}
		kernel_halt();
	}
#endif
	videoMode = mode;
	videoMem = framebuffer;
	videoWidth = width;
	videoHeight = height;
	videoBpp = bpp;
	if(videoBpp == 15)
		videoBytesPerPixel = 2;
	else
		videoBytesPerPixel = videoBpp / 8;
	videoBytesPerLine = bytesPerLine;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT){
		videoMemLen = stdio64_archtex_get_fb_size();
		stdio64_archtex_get_texdims(&totalX, &totalY);
	}else{
#endif
		videoMemLen = width * height * videoBytesPerPixel;
		totalX = width / STDIO64_GRAPHICS_CHAR_WIDTH;
		totalY = height / STDIO64_GRAPHICS_CHAR_HEIGHT;
#if STDIO64_ARCH_HAS_TEXT
	}
#endif
	updateCursorMemData();
	stdio64_reallocmem();
}

void stdio64_get_mode(uint8_t* mode, size_t* width, size_t* height, size_t* bpp, size_t* pitch, void** framebuffer){
	if(mode)
		*mode = videoMode;
	if(width)
		*width = videoWidth;
	if(height)
		*height = videoHeight;
	if(bpp)
		*bpp = videoBpp;
	if(pitch)
		*pitch = videoBytesPerLine;
	if(framebuffer)
		*framebuffer = videoMem;
}

void stdio64_get_cursor_pos(size_t* x, size_t* y){
	if(x)
		*x = cursorX;
	if(y)
		*y = cursorY;
}

bool stdio64_available(){
	return videoMode != 0;
}


void print(char* string, uint8_t attr){
	size_t len = strlen(string);
	for(size_t i = 0; i < len; i++){
		printChar(string[i], attr);
	}
}

void printp(char* string, uint8_t attr){
	print(msgPre, attr);
	print(string, attr);
}

void println(char* string, uint8_t attr){
	printp(string, attr);
	printNln();
}

void printlns(char* string, uint8_t attr){
	print(string, attr);
	printNln();
}

void printChar(char ch, uint8_t attr){
	printChar_nlog(ch, attr);
	printSerialChar(ch);
	stdio64_text_log(ch, attr);
}

void printChar_nlog(char ch, uint8_t attr){
	if(ch == 0xa){
		printNln();
	}else if(ch == 0x8){
		delChar();
	}else{
		printCharAt(ch, attr, cursorX, cursorY);
		cursorX++;
		if(cursorX >= totalX){
			printNln();
		}
	}
	updateCursor();
}

void printSerialChar(char ch){
	if(!serial_input()) // only write to serial if we have received input from there (serial slows down output significantly)
		return;
	if(ch == 8){
		serial_write(127);
	}else{
		if(ch == '\n')
			serial_write('\r');
		serial_write((uint8_t) ch);
	}
}

void delChar(){
	if(videoMode == 0) // stdio64 was not initialized (this should never happen under normal conditions)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_delChar();
	else
#endif
		stdio64_def_delChar();
	updateCursor();
}

void printAt(char* string, uint8_t attr, size_t x, size_t y){
	size_t len = strlen(string);
	for(size_t i = 0; i < len; i++){
		printCharAt(string[i], attr, x + i, y);
	}
}

void printCharAt(char ch, uint8_t attr, size_t x, size_t y){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_printCharAt(ch, attr, x, y);
	else
#endif
		stdio64_def_printCharAt(ch, attr, x, y);
	updateCursor();
}

void printNln(){
	cursorX = 0;
	cursorY++;
	if(cursorY >= totalY){
		cursorY--;
		shiftUp();
	}
	updateCursor();
	printSerialChar(0xa);
}

void printNlnr(){
	if(cursorX != 0)
		printChar(0xa, 0x7);
}

void shiftUp(){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_shiftUp();
	else
#endif
		stdio64_def_shiftUp();
	updateCursor();
}

void shiftDown(){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_shiftDown();
	else
#endif
		stdio64_def_shiftDown();
	updateCursor();
}

void clearScreen(uint8_t attr){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_clearScreen(attr);
	else
#endif
		stdio64_def_clearScreen(attr);
	updateCursor();
}

void updateCursor(){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_updateCursor();
	else
#endif
		stdio64_def_updateCursor();
}

void updateCursorMemData(){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_updateCursorMemData();
	else
#endif
		stdio64_def_updateCursorMemData();
}

void updateLoadingWheel(){
	if(videoMode == 0)
		return;
#if STDIO64_ARCH_HAS_TEXT
	if(videoMode == STDIO64_MODE_TEXT)
		stdio64_archtex_updateLoadingWheel();
	else
#endif
		stdio64_def_updateLoadingWheel();
}


void incCursorX(){
	printChar(0x20, 0x7);
}

void saveCursorPosition(){
	prevCursorX = cursorX;
	prevCursorY = cursorY;
}

void restoreCursorPosition(){
	cursorX = prevCursorX;
	cursorY = prevCursorY;
}

void setCursorPosition(size_t x, size_t y){
	cursorX = x;
	cursorY = y;
	updateCursor();
}


void printDec(size_t dec){
	print(getDec(dec), 0x5);
}

char* getDec(size_t dec){
	uint8_t si = 20;
	for(int i = 20; i >= 0; i--){
		decTmp[i] = '0';
	}
	for(int i = 20; i >= 0 && dec != 0; i--){
		uint8_t cr = dec % 10;
		decTmp[i] = '0' + cr;
		dec /= 10;
		si = i;
	}
	return decTmp + si;
}

void printHex(size_t hex){
	print(getHex(hex), 0xd);
}

char* getHex(size_t hex){
	for(int i = STDIO64__HEX_LEN + 1; i >= 2; i--){
		hexTmp[i] = '0';
		uint8_t cr = hex & 0xf;
		if(cr >= 10)
			hexTmp[i] += 7;
		hexTmp[i] += cr;
		hex >>= 4;
	}
	return hexTmp;
}


void stdio64_text_log(char c, uint8_t attr){
	if(!textLog)
		return;
	textLog[textLogIndex++] = (uint8_t) c | ((uint16_t) attr << 8);
	if(textLogIndex >= STDIO64_TEXT_LOG_LENGTH){
		textLogIndex = 0;
	}
}

void reprintText(){
	for(size_t i = textLogIndex + 1; i < STDIO64_TEXT_LOG_LENGTH + textLogIndex; i++){
		uint16_t c = textLog[i % STDIO64_TEXT_LOG_LENGTH];
		if(c != 0){
			printChar_nlog(c & 0xff, c >> 8);
		}
	}
}



// default functions for a linear framebuffer

void stdio64_def_delChar(){
	if(cursorX <= 0){
		cursorX = videoWidth / STDIO64_GRAPHICS_CHAR_WIDTH - 1;
		printCharAt(0x20, 0, cursorX, cursorY - 1);
		shiftDown();
	}else{
		cursorX--;
		printCharAt(0x20, 0, cursorX, cursorY);
	}
}

void stdio64_def_printCharAt(char ch, uint8_t attr, size_t x, size_t y){
	uint64_t cfont = ch >= 0 ? font7x8[ch] : font7x8[128];
	uint8_t* addr = videoMem + y * videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT + x * videoBytesPerPixel * STDIO64_GRAPHICS_CHAR_WIDTH;
	for(int y = 0; y < 8; y++){
		for(int x = 0; x < STDIO64_GRAPHICS_CHAR_WIDTH; x++){
			stdio64_def_writeVGAPixel(addr, attr, cfont & (1ULL << (x * 8 + y)));
			addr += videoBytesPerPixel;
		}
		addr += videoBytesPerLine - videoBytesPerPixel * STDIO64_GRAPHICS_CHAR_WIDTH;
	}
}

void stdio64_def_writeVGAPixel(uint8_t* addr, uint8_t attr, bool set){
	uint32_t color = set ? cgaColors[attr & 0xf] : cgaColors[(attr >> 4) & 0xf];
	if(videoBpp == 15){
		uint16_t pixel = ((color >> 9) & 0x7c00) | ((color >> 6) & 0x3e0) | ((color >> 3) & 0x1f);
		*((uint16_t*) addr) = pixel;
	}else if(videoBpp == 16){
		uint16_t pixel = ((color >> 8) & 0xf800) | ((color >> 5) & 0x7e0) | ((color >> 3) & 0x1f);
		*((uint16_t*) addr) = pixel;
	}else if(videoBpp == 24){
		*addr = color & 0xff;
		*(addr + 1) = (color >> 8) & 0xff;
		*(addr + 2) = (color >> 16) & 0xff;
	}else if(videoBpp == 32){
		*addr = color & 0xff;
		*(addr + 1) = (color >> 8) & 0xff;
		*(addr + 2) = (color >> 16) & 0xff;
	}
}

void stdio64_def_shiftUp(){
	uint8_t* addr = videoMem;
	for(int i = 0; i < videoHeight * videoBytesPerLine - videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT; i++){
		*addr = *(addr + videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT);
		addr++;
	}
	for(int i = 0; i < videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT; i++){
		*addr = 0;
		addr++;
	}
}

void stdio64_def_shiftDown(){
	uint8_t* addr = videoMem + videoHeight * videoBytesPerLine - 1;
	for(int i = 0; i < videoHeight * videoBytesPerLine - videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT; i++){
		*addr = *(addr - videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT);
		addr--;
	}
	for(int i = 0; i < videoBytesPerLine * STDIO64_GRAPHICS_CHAR_HEIGHT; i++){
		*addr = 0;
		addr--;
	}
}

void stdio64_def_clearScreen(uint8_t attr){
	for(uint8_t* addr = videoMem; (size_t) addr < videoHeight * videoBytesPerLine + (size_t) videoMem;
		addr += videoBytesPerPixel){
		stdio64_def_writeVGAPixel(addr, attr, FALSE);
	}
	cursorX = 0;
	cursorY = 0;
}

void stdio64_def_updateCursor(){
}

void stdio64_def_updateCursorMemData(){
	cursorX = 0;
	cursorY = 0;
}

void stdio64_def_updateLoadingWheel(){
}


