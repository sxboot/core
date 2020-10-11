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
 * list.c - Dynamic length list implementation.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/errc.h>
#include <kernel/list.h>




list_array* list_array_create(size_t flags){
	list_array* list = kmalloc(sizeof(list_array));
	if(list == NULL)
		return NULL;
	list->length = 0;
	list->base = NULL;
	list->memlen = 0;
	list->flags = flags;
	reloc_ptr((void**) &list->base);
	return list;
}

status_t list_array_push(list_array* list, void* obj){
	status_t status = 0;
	if(list->length >= list->memlen){
		status = list_array_reserve(list, MAX(list->memlen * 2, 1));
		if(status != TSX_SUCCESS){
			return status;
		}
	}
	void** loc = &list->base[list->length++];
	*loc = obj;
	if(!(list->flags & LIST_FLAGS_STATIC)){
		reloc_ptr(loc);
	}
	return 0;
}

void* list_array_pop(list_array* list){
	if(list->length <= 0)
		return NULL;
	void** loc = &list->base[--list->length];
	del_reloc_ptr(loc);
	return *loc;
}

void* list_array_get(list_array* list, size_t index){
	return list->base[index];
}

status_t list_array_clear(list_array* list){
	for(int i = 0; i < list->length; i++){
		del_reloc_ptr(&list->base[i]);
	}
	if(list->base != NULL){
		kfree(list->base, list->memlen * sizeof(void*));
	}
	del_reloc_ptr((void**) &list->base);
	list->length = 0;
	list->base = NULL;
	list->memlen = 0;
	return 0;
}

void list_array_delete(list_array* list){
	list_array_clear(list);
	kfree(list, sizeof(list_array));
}

status_t list_array_reserve(list_array* list, size_t len){
	if(list->memlen >= len)
		return 0;
	void* nb = kmalloc(len * sizeof(void*));
	if(nb == NULL)
		return TSX_OUT_OF_MEMORY;
	if(list->base != NULL){
		memcpy(nb, list->base, list->memlen * sizeof(void*));
		kfree(list->base, list->memlen * sizeof(void*));
	}
	list->base = nb;
	list->memlen = len;
	if(!(list->flags & LIST_FLAGS_STATIC)){
		for(int i = 0; i < list->length; i++){
			reloc_ptr(&list->base[i]);
		}
	}
	return 0;
}


