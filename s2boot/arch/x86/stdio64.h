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

#ifndef __ARCH_STDIO64_H__
#define __ARCH_STDIO64_H__


#include <klibc/stdlib.h>
#include <klibc/stdint.h>


#define STDIO64_ARCH_HAS_TEXT 1


// TEXT MODE
size_t stdio64_archtex_get_fb_size();
void stdio64_archtex_get_texdims(size_t* totalX, size_t* totalY);
void stdio64_archtex_delChar();
void stdio64_archtex_printCharAt(char ch, uint8_t attr, size_t x, size_t y);
void stdio64_archtex_shiftUp();
void stdio64_archtex_shiftDown();
void stdio64_archtex_clearScreen(uint8_t attr);
void stdio64_archtex_updateCursor();
void stdio64_archtex_updateCursorMemData();
void stdio64_archtex_updateLoadingWheel();


#endif /* __ARCH_STDIO64_H__ */
