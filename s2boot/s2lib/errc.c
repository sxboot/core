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
 * errc.c - Error codes.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>

char unknown[] = "TSX_UNKNOWN_ERROR";

static char* errnames[] = {
	"TSX_SUCCESS",
	"TSX_ERROR",
	"TSX_WARNING",
	unknown,
	unknown,
	"TSX_TEST",
	unknown,
	unknown,
	unknown,
	unknown,
	"TSX_BUFFER_FULL", // 10
	"TSX_NO_CONTROLLER",
	"TSX_CONTROLLER_NOT_PRESENT",
	"TSX_NO_DEVICE",
	"TSX_COMMAND_TIMEOUT",
	"TSX_NOT_FOUND",
	"TSX_CONTROLLER_NOT_INITIALIZED",
	"TSX_PORT_BUFFER_FULL",
	"TSX_IO_COMMAND_TIMEOUT",
	"TSX_IO_ERROR",
	"TSX_NO_DRIVER", // 20
	"TSX_INVALID_SYNTAX",
	"TSX_NO_SUCH_DIRECTORY",
	"TSX_NO_SUCH_FILE",
	"TSX_INVALID_PARTITION",
	"TSX_INVALID_FORMAT",
	"TSX_INVALID_TYPE",
	"TSX_MEMORY_RESERVED",
	"TSX_OUT_OF_MEMORY",
	"TSX_ELF_MISSING_SECTION",
	"TSX_UNIMPLEMENTED", // 30
	"TSX_UNDEFINED_REFERENCE",
	"TSX_TOO_LARGE",
	"TSX_PARSE_ERROR",
	"TSX_PAGE_MAP_FAILED",
	"TSX_PAGE_NOT_PRESENT",
	"TSX_MISSING_LIBRARY",
	"TSX_MISSING_ARGUMENTS",
	"TSX_ILLEGAL_STATE",
	"TSX_UNAVAILABLE",
	"TSX_UNSUPPORTED",
	"TSX_END"
};

char* errcode_get_name(uint8_t errcode){
	if(errcode >= ERRCODE_COUNT)return unknown;
	return errnames[errcode];
}
