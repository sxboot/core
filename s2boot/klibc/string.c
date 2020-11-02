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

#include <klibc/stdlib.h>
#include <klibc/stddef.h>
#include <kernel/util.h>

void* memmove(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

int memcmp(const void *str1, const void *str2, size_t count){
	const unsigned char *s1 = str1;
	const unsigned char *s2 = str2;

	while (count-- > 0){
		if (*s1++ != *s2++)
		return s1[-1] < s2[-1] ? -1 : 1;
	}
	return 0;
}

void* memset(void *dest, int val, size_t len){
	unsigned char *ptr = dest;
	while (len-- > 0)
		*ptr++ = val;
	return dest;
}

void* memcpy(void *dest, const void *src, size_t len){
	char *d = dest;
	const char *s = src;
	while (len--)
		*d++ = *s++;
	return dest;
}


size_t strlen(const char* str){
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}

char* strchr(char* s, int c){
	size_t strl = strlen(s);
	for(int i = 0; i < strl; i++){
		if(s[i] == c)
			return &s[i];
	}
	return NULL;
}

char* strstr(char* str, char* sstr){
	if(*sstr == 0)
		return str;
	char* tmpsstr = sstr;
	for(; *str; str++){
		if(*str != *tmpsstr)
			continue;
		while(1){
			if(*tmpsstr == 0)
				return str;
			if(*str++ != *tmpsstr++)
				break;
		}
		str--;
		tmpsstr = sstr;
	}
	return NULL;
}

char* strcpy(char* dest, const char* src){
	return memcpy(dest, src, strlen(src) + 1);
}

char* strncpy(char* dest, const char* src, size_t n){
	size_t tsize = strlen(src);
	size_t size = MIN(tsize, n);
	if(size != n){
		memset(dest + size, 0, n - size);
	}
	return memcpy(dest, src, size);
}

int strcmp(char* str1, char* str2){
	size_t c = 0;
	do{
		if(str1[c] != str2[c])
			return str1[c] - str2[c];
		if(!str1[c])
			break;
		c++;
	}while(1);
	return 0;
}

int strncmp(char* str1, char* str2, size_t num){
	if(num == 0)
		return 0;
	size_t c = 0;
	do{
		if(str1[c] != str2[c])
			return str1[c] - str2[c];
		if(!str1[c])
			break;
		c++;
	}while(--num);
	return 0;
}


int atoi(const char* str){
	return (int) util_str_to_int((char*) str);
}

long int atol(const char* str){
	return (long) util_str_to_int((char*) str);
}

long long int atoll(const char* str){
	return (long long) util_str_to_int((char*) str);
}


char* strrepl(char* str, char* sstr, char* repl){
	size_t strsz = strlen(str);
	size_t sstrsz = strlen(sstr);
	if(strsz < 1)
		return str;
	size_t replsz = strlen(repl);
	size_t count = 0;
	if(strsz < sstrsz) // <- str cannot contain sstr
		goto _newstr;
	for(size_t i = 0; i <= strsz - sstrsz; i++){
		if(memcmp(str + i, sstr, sstrsz) == 0){
			count++;
			i += sstrsz - 1;
		}
	}
	_newstr: {
	size_t nstrsz = strsz + replsz * count - sstrsz * count;
	char* nstr = kmalloc(nstrsz + 1);
	if(!nstr)
		return NULL;
	size_t nstri = 0;
	for(size_t i = 0; i < strsz;){
		if(strsz >= sstrsz && i <= strsz - sstrsz && memcmp(str + i, sstr, sstrsz) == 0){
			memcpy(nstr + nstri, repl, replsz);
			nstri += replsz;
			i += sstrsz;
		}else{
			nstr[nstri++] = str[i++];
		}
	}
	nstr[nstrsz] = 0;
	return nstr;
	}
}
