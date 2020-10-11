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

#ifndef __KERNEL_UTIL_H__
#define __KERNEL_UTIL_H__

#include <klibc/stdlib.h>
#include <klibc/stdbool.h>


bool util_str_equals(char* s1, char* s2);
bool util_str_startsWith(char* s, char* start);
char* util_str_cut_to(char* s, char c);
bool util_str_contains(char* s, char c);
size_t util_str_length_c(char* s, char term);
size_t util_str_length_c_max(char* s, char term, size_t max);
size_t util_str_length(char* s);
size_t util_str_to_int_c(char* s, char term);
size_t util_str_to_int(char* s);
size_t util_str_to_hex_c(char* s, char term);
size_t util_str_to_hex(char* s);
size_t util_math_pow(size_t i1, size_t i2);
size_t util_math_min(size_t i1, size_t i2);
size_t util_math_max(size_t i1, size_t i2);
size_t util_count_parts(char* s, char sep);

void* util_search_mem(char* signature, size_t base, size_t len, size_t alignment);


#endif /* __KERNEL_UTIL_H__ */
