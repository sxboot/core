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

#ifndef __KLIBC_STDIO_H__
#define __KLIBC_STDIO_H__

#include <klibc/stdarg.h>


int putchar(int c);
int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t n, const char* format, ...);
int vsprintf(char* str, const char* format, va_list parameters);
int vsnprintf(char* str, size_t n, const char* format, va_list parameters);
int puts(const char* string);


#endif /* __KLIBC_STDIO_H__ */
