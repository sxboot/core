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
 * util.c - Miscellaneous utility functions.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <kernel/util.h>

static char* none = "";

bool util_str_equals(char* s1, char* s2){
	size_t s1l = util_str_length(s1);
	if(s1l != util_str_length(s2))return FALSE;
	
	for(int i = 0; i < s1l; i++){
		if(s1[i] != s2[i])return FALSE;
	}
	return TRUE;
}

bool util_str_startsWith(char* s, char* start){
	size_t s1l = util_str_length(start);
	
	for(int i = 0; i < s1l; i++){
		if(start[i] != s[i])return FALSE;
	}
	return TRUE;
}

char* util_str_cut_to(char* s, char c){
	size_t strl = util_str_length(s);
	for(int i = 0; i < strl; i++){
		if(s[i] == c)return (s + i);
	}
	return none;
}

bool util_str_contains(char* s, char c){
	size_t strl = util_str_length(s);
	for(int i = 0; i < strl; i++){
		if(s[i] == c)return TRUE;
	}
	return FALSE;
}

size_t util_str_length_c(char* s, char term){
	size_t length = 0;
	while((s[length]) != term)length++;
	return length;
}

size_t util_str_length_c_max(char* s, char term, size_t max){
	size_t length = 0;
	while((s[length]) != term && length < max)length++;
	return (length>max)?max:length;
}

size_t util_str_length(char* s){
	return util_str_length_c(s, 0);
}

size_t util_str_to_int_c(char* s, char term){
	if(util_str_startsWith(s, "0x"))return util_str_to_hex_c(s, term);
	size_t ret = 0;
	size_t strl = util_str_length_c(s, term);
	for(int i = 0; i < strl; i++){
		uint8_t num = s[i];
		if(num == term)break;
		if(num < 32){
			continue;
		}else if((num > 47) && (num < 58)){
			ret += (num - 48) * util_math_pow(10, strl - i - 1);
		}else
			return ret;
	}
	return ret;
}

size_t util_str_to_int(char* s){
	return util_str_to_int_c(s, 0);
}

size_t util_str_to_hex_c(char* s, char term){
	size_t ret = 0;
	size_t strl = util_str_length_c(s, term) - 2;
	for(int i = 0; i < strl; i++){
		uint8_t num = s[i + 2];
		if(num == term)break;
		if((num > 47) && (num < 58)){
			ret += ((size_t) num - 48) << ((strl - i - 1) * 4);
		}else if((num > 64) && (num < 71)){
			ret += ((size_t) num - 55) << ((strl - i - 1) * 4);
		}else if((num > 96) && (num < 103)){
			ret += ((size_t) num - 87) << ((strl - i - 1) * 4);
		}else
			return ret;
	}
	return ret;
}

size_t util_str_to_hex(char* s){
	return util_str_to_hex_c(s, 0);
}

size_t util_math_pow(size_t i1, size_t i2){
	if(i2 == 0)return 1;
	size_t i1t = i1;
	for(int i = 1; i < i2; i++){
		i1t *= i1;
	}
	return i1t;
}

size_t util_math_min(size_t i1, size_t i2){
	return (i1<i2)?i1:i2;
}

size_t util_math_max(size_t i1, size_t i2){
	return (i1>i2)?i1:i2;
}

size_t util_count_parts(char* s, char sep){
	size_t ret = 0;
	for(int i = 0; s[i] != 0; i++){
		if(s[i] == sep)ret++;
	}
	return ret + 1;
}


void* util_search_mem(char* signature, size_t base, size_t len, size_t alignment){
	alignment = MAX(1, alignment);

	size_t signlen = util_str_length(signature);
	size_t addr = base;
	while(addr < base + len){
		for(size_t i = 0; i < signlen; i++){
			if(signature[i] != *((char*) (addr + i))){
				goto not;
			}
		}
		return (void*) (addr);
		not:
		addr += alignment;
	}
	return NULL;
}
