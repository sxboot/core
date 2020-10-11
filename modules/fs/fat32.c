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
#include <klibc/string.h>
#include <kernel/list.h>

// only for testing, not implemented yet

bool vfs_isFilesystem(char* driveLabel, uint64_t partStart){
	return TRUE;
}

status_t vfs_readFile(char* driveLabel, uint64_t partStart, char* path, size_t dest){
	return 1;
}

status_t vfs_getFileSize(char* driveLabel, uint64_t partStart, char* path, size_t* sizeWrite){
	return 1;
}

status_t vfs_listDir(char* driveLabel, uint64_t partStart, char* path, list_array** listWrite){
	return 1;
}

