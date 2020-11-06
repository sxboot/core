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

#ifndef __KERNEL_MODULES_H__
#define __KERNEL_MODULES_H__


#define MODULES_TYPE_BOOT_HANDLER 1
#define MODULES_TYPE_DISK_DRIVER 2
#define MODULES_TYPE_FS_DRIVER 3

#define module_type_t uint8_t


status_t modules_load_module_from_file(char* filePath, module_type_t type);
status_t modules_load_module(elf_file* file, module_type_t type);

status_t modules_init_boot_handler(elf_file* file, char* type);
status_t modules_init_disk_driver(elf_file* file, char** typeWrite);
status_t modules_init_fs_driver(elf_file* file);


#endif /* __KERNEL_MODULES_H__ */
