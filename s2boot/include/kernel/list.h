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

#ifndef __KERNEL_LIST_H__
#define __KERNEL_LIST_H__

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>


// when set, entries in the list will not be added to the list of relocatable pointers (used when entries in list are not references to objects)
#define LIST_FLAGS_STATIC 1

typedef struct list_array{
	size_t length;
	void** base;
	size_t memlen;
	size_t flags;
} list_array;

list_array* list_array_create(size_t flags);
status_t list_array_push(list_array* list, void* obj);
void* list_array_pop(list_array* list);
void* list_array_get(list_array* list, size_t index);
status_t list_array_clear(list_array* list);
void list_array_delete(list_array* list);
status_t list_array_reserve(list_array* list, size_t len);

#endif /* __KERNEL_LIST_H__ */
