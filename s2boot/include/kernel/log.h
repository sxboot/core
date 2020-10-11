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

#ifndef __KERNEL_LOG_H__
#define __KERNEL_LOG_H__

#include <klibc/stdarg.h>

extern int log_level;


void log_debug(const char* str, ...);
void log_info(const char* str, ...);
void log_warn(const char* str, ...);
void log_error(const char* str, ...);
void log_fatal(const char* str, ...);
void log_vlog(const char* level, const char* str, const char color, va_list args);
void log_vlog0(const char* level, char* str, const char color);


#endif /* __KERNEL_LOG_H__ */
