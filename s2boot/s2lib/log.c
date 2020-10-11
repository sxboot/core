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
 * log.c - Logger.
 */

#include <klibc/stdlib.h>
#include <klibc/stdio.h>
#include <klibc/stdarg.h>
#include <kernel/log.h>


int log_level = 4;

void log_debug(const char* str, ...){
	va_list args;
	va_start(args, str);
	if(log_level >= 4)
		log_vlog("D", str, '7', args);
	va_end(args);
}

void log_info(const char* str, ...){
	va_list args;
	va_start(args, str);
	if(log_level >= 3)
		log_vlog("I", str, 'f', args);
	va_end(args);
}

void log_warn(const char* str, ...){
	va_list args;
	va_start(args, str);
	if(log_level >= 2)
		log_vlog("W", str, 'e', args);
	va_end(args);
}

void log_error(const char* str, ...){
	va_list args;
	va_start(args, str);
	if(log_level >= 1)
		log_vlog("E", str, 'c', args);
	va_end(args);
}

void log_fatal(const char* str, ...){
	va_list args;
	va_start(args, str);
	if(log_level >= 0)
		log_vlog("F", str, '4', args);
	va_end(args);
}

void log_vlog(const char* level, const char* str, const char color, va_list args){
	va_list args2;
	va_copy(args2, args);
	int size = vsnprintf(NULL, 0, str, args) + 1;
	if(size <= 0){
		log_vlog0("F", "[logger] Logger error: string format failed", '4');
	}else{
		char buf[size];
		vsnprintf(buf, size, str, args2);
		log_vlog0(level, buf, color);
	}
	va_end(args2);
}

void log_vlog0(const char* level, char* str, const char color){
	char* strformat = "&7[s2][%s] %s";
	strformat[1] = color;
	printf(strformat, level, str);
}


