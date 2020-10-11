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

#ifndef __KERNEL_ERRC_H__
#define __KERNEL_ERRC_H__

#include <klibc/stdint.h>


#define status_t uint8_t

#define ERRCODE_COUNT 42

#define TSX_SUCCESS 0
#define TSX_ERROR 1
#define TSX_WARNING 2
#define TSX_BUFFER_FULL 10
#define TSX_NO_CONTROLLER 11
#define TSX_CONTROLLER_NOT_PRESENT 12
#define TSX_NO_DEVICE 13
#define TSX_COMMAND_TIMEOUT 14
#define TSX_NOT_FOUND 15
#define TSX_CONTROLLER_NOT_INITIALIZED 16
#define TSX_PORT_BUFFER_FULL 17
#define TSX_IO_COMMAND_TIMEOUT 18
#define TSX_IO_ERROR 19
#define TSX_NO_DRIVER 20
#define TSX_INVALID_SYNTAX 21
#define TSX_NO_SUCH_DIRECTORY 22
#define TSX_NO_SUCH_FILE 23
#define TSX_INVALID_PARTITION 24
#define TSX_INVALID_FORMAT 25
#define TSX_INVALID_TYPE 26
#define TSX_MEMORY_RESERVED 27
#define TSX_OUT_OF_MEMORY 28
#define TSX_ELF_MISSING_SECTION 29
#define TSX_UNIMPLEMENTED 30
#define TSX_UNDEFINED_REFERENCE 31
#define TSX_TOO_LARGE 32
#define TSX_PARSE_ERROR 33
#define TSX_PAGE_MAP_FAILED 34
#define TSX_PAGE_NOT_PRESENT 35
#define TSX_MISSING_LIBRARY 36
#define TSX_MISSING_ARGUMENTS 37
#define TSX_ILLEGAL_STATE 38
#define TSX_UNAVAILABLE 39
#define TSX_UNSUPPORTED 40
#define TSX_END 41


char* errcode_get_name(uint8_t errcode);


#endif /* __KERNEL_ERRC_H__ */
