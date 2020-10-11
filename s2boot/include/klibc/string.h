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

#ifndef __KLIBC_STRING_H__
#define __KLIBC_STRING_H__



void* memmove(void* dest, const void* src, size_t len);
int memcmp(const void* str1, const void* str2, size_t count);
void* memset(void* dest, int val, size_t len);
void* memcpy(void* dest, const void* src, size_t len);

size_t strlen(const char* str);
char* strchr(char* s, int c);
char* strstr(char* str, char* sstr);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);

int atoi(const char* str);
long int atol(const char* str);
long long int atoll(const char* str);

char* strrepl(char* str, char* sstr, char* repl);


#endif /* __KLIBC_STRING_H__ */
