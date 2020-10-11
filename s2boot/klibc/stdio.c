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
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/stddef.h>
#include <klibc/string.h>
#include <klibc/stdarg.h>
#include <klibc/limits.h>
#include <klibc/stdio.h>
#include <kernel/stdio64.h>

int putchar(int c){
	printChar((char) c, 0x7);
	return (char) c;
}

int printf(const char* format, ...){
	va_list parameters;
	va_start(parameters, format);

	int written = 0;
	uint8_t color = 0x7;

	while(*format != 0){
		if(*format == '%'){
			format++;
			char specifier = *format;
			if(specifier == 's'){
				char* str = va_arg(parameters, char*);
				uint64_t length = strlen(str);
				print(str, color);
				format++;
				written += length;
			}else if(specifier == 'u'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getDec(num);
				uint64_t length = strlen(numstr);
				print(numstr, color);
				format++;
				written += length;
			}else if(specifier == 'X'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getHex(num) + 2; // getHex returns a string of format 0xHHHHHHHH but printf needs ...H
				while(*numstr == '0')
					numstr++;
				if(*numstr == 0) // all numbers are 0, so go back one to print a single '0'
					numstr--;
				uint64_t length = strlen(numstr);
				print(numstr, color);
				format++;
				written += length;
			}else if(specifier == 'Y'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getHex(num);
				uint64_t length = strlen(numstr);
				print(numstr, color);
				format++;
				written += length;
			}else if(specifier == 'c'){
				char c = (char) va_arg(parameters, int);
				printChar(c, color);
				format++;
				written++;
			}else if(specifier == '%'){
				printChar('%', color);
				format++;
				written++;
			}else{
				printChar('%', color);
				written++;
			}
		}else if(*format == '&'){
			format++;
			char colorH = *format;
			if(colorH > 47 && colorH < 58)
				color = colorH - 48;
			else if(colorH > 96 && colorH < 103)
				color = colorH - 87;
			format++;
		}else{
			printChar(*format, color);
			format++;
			written++;
		}
	}
	va_end(parameters);
	return written;
}

int sprintf(char* str, const char* format, ...){
	va_list parameters;
	va_start(parameters, format);
	int written = vsprintf(str, format, parameters);
	va_end(parameters);
	return written;
}

int snprintf(char* str, size_t n, const char* format, ...){
	va_list parameters;
	va_start(parameters, format);
	int written = vsnprintf(str, n, format, parameters);
	va_end(parameters);
	return written;
}

int vsprintf(char* str, const char* format, va_list parameters){
	return vsnprintf(str, UINT_MAX, format, parameters);
}

int vsnprintf(char* str, size_t n, const char* format, va_list parameters){
	int written = 0;

	while(*format != 0){
		if(*format == '%'){
			format++;
			char specifier = *format;
			if(specifier == 's'){
				char* strg = va_arg(parameters, char*);
				uint64_t length = strlen(strg);
				if(written + length < n)
					memcpy((void*) (str + written), (void*) strg, MIN(length, n - written));
				format++;
				written += length;
			}else if(specifier == 'u'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getDec(num);
				uint64_t length = strlen(numstr);
				if(written + length < n)
					memcpy((void*) (str + written), (void*) numstr, MIN(length, n - written));
				format++;
				written += length;
			}else if(specifier == 'X'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getHex(num) + 2; // getHex returns a string of format 0xHHHHHHHH but printf needs ...H
				while(*numstr == '0')
					numstr++;
				if(*numstr == 0) // all numbers are 0, so go back one to print a single '0'
					numstr--;
				uint64_t length = strlen(numstr);
				if(written + length < n)
					memcpy((void*) (str + written), (void*) numstr, MIN(length, n - written));
				format++;
				written += length;
			}else if(specifier == 'Y'){
				size_t num = va_arg(parameters, size_t);
				char* numstr = getHex(num);
				uint64_t length = strlen(numstr);
				if(written + length < n)
					memcpy((void*) (str + written), (void*) numstr, MIN(length, n - written));
				format++;
				written += length;
			}else if(specifier == 'c'){
				char c = (char) va_arg(parameters, int);
				if(written + 1 < n)
					*(str + written) = c;
				format++;
				written++;
			}else if(specifier == '%'){
				if(written + 1 < n)
					*(str + written) = '%';
				format++;
				written++;
			}else{
				if(written + 1 < n)
					*(str + written) = '%';
				written++;
			}
		}else{
			if(written + 1 < n)
				*(str + written) = *format;
			format++;
			written++;
		}
	}
	if(written + 1 < n)
		*(str + written) = 0;
	written++;
	return written;
}

int puts(const char* string){
	return printf("%s\n", string);
}
