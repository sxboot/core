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

#ifndef __KERNEL_STDIO64_H__
#define __KERNEL_STDIO64_H__


#include <klibc/stdlib.h>
#include <klibc/stdint.h>


#define STDIO64_MODE_GRAPHICS 1
#define STDIO64_MODE_TEXT 2

#define STDIO64_GRAPHICS_CHAR_WIDTH 7
#define STDIO64_GRAPHICS_CHAR_HEIGHT 9

#define STDIO64_EARLY_TEXT_LOG_LENGTH 200
#define STDIO64_TEXT_LOG_LENGTH 4096

#define STDIO64_FB_MOD_MAP_PIXELS_PER_CELL 5


status_t stdio64_init();
status_t stdio64_reallocmem();

void stdio64_set_mode(uint8_t mode, void* framebuffer, size_t width, size_t height, size_t bpp, size_t bytesPerLine);
void stdio64_get_mode(uint8_t* mode, size_t* width, size_t* height, size_t* bpp, size_t* pitch, void** framebuffer);
void stdio64_get_text_size(size_t* rows, size_t* cols);
void stdio64_get_cursor_pos(size_t* x, size_t* y);
void stdio64_set_font_scale(size_t scale);
bool stdio64_available();
void stdio64_update_screen();
void stdio64_mark_modified(void* addr);

void print(char* string, uint8_t attr);
void printp(char* string, uint8_t attr);
void println(char* string, uint8_t attr);
void printlns(char* string, uint8_t attr);

void printChar(char ch, uint8_t attr);
void printChar_nlog(char ch, uint8_t attr);
void printSerialChar(char ch);
void delChar();
void printAt(char* string, uint8_t attr, size_t x, size_t y);
void printCharAt(char ch, uint8_t attr, size_t x, size_t y);
void printNln();
void printNln_nlog();
void printNlnr();
void shiftUp();
void shiftDown();
void clearScreen(uint8_t attr);
void updateCursor();
void updateCursorMemData();
void updateLoadingWheel();

void incCursorX();
void saveCursorPosition();
void restoreCursorPosition();
void setCursorPosition(size_t x, size_t y);

void printDec(size_t dec);
char* getDec(size_t dec);
void printHex(size_t hex);
char* getHex(size_t hex);

void stdio64_text_log(char c, uint8_t attr);
void reprintText();


void stdio64_def_delChar();
void stdio64_def_printCharAt(char ch, uint8_t attr, size_t x, size_t y);
void stdio64_def_writeVGAPixel(size_t x, size_t y, uint8_t attr, bool set);
void stdio64_def_writeVGAPixelCol(size_t x, size_t y, size_t color);
void stdio64_def_shiftUp();
void stdio64_def_shiftDown();
void stdio64_def_clearScreen(uint8_t attr);
void stdio64_def_updateCursor();
void stdio64_def_updateCursorMemData();
void stdio64_def_updateLoadingWheel();


#endif /* __KERNEL_STDIO64_H__ */
