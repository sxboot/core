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
 * msio.c - An abstraction layer for storage device drivers.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <kernel/errc.h>
#include <kernel/util.h>
#include <kernel/list.h>
#include <kernel/msio.h>

static list_array* msio_drivers = NULL;

status_t msio_init(){
	status_t status = 0;
	msio_drivers = list_array_create(0);
	if(msio_drivers == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &msio_drivers);
	_end:
	return status;
}

status_t msio_attach_driver(char* type, MSIO_DRIVER_READ read,
	MSIO_DRIVER_WRITE write){
	status_t status = 0;
	if(type[0] == 0)
		FERROR(1);
	msio_device_driver* driver = kmalloc(sizeof(msio_device_driver));
	for(uint8_t i = 0; i < 4; i++){
		driver->type[i] = 0;
		if(type[i] == 0)
			break;
		driver->type[i] = type[i];
	}
	driver->type[4] = 0;
	driver->read = read;
	driver->write = write;
	reloc_ptr((void**) &driver->read);
	reloc_ptr((void**) &driver->write);
	list_array_push(msio_drivers, driver);
	_end:
	return status;
}

status_t msio_read_drive(char* driveLabel, uint64_t sector, uint16_t sectorCount, size_t dest){
	status_t status = 0;
	msio_device_driver* driver = msio_get_driver(driveLabel);
	if(driver == 0)
		FERROR(20);
	uint8_t number = msio_get_drive_num(driveLabel);
	if(number >= 0xff)
		FERROR(21);
	status = driver->read(number, sector, sectorCount, dest);
	CERROR();
	_end:
	return status;
}

msio_device_driver* msio_get_driver(char* driveLabel){
	char type[5];
	type[4] = 0;
	for(int i = 0; i < 4; i++){
		type[i] = 0;
		if(driveLabel[i] < 0x61 || driveLabel[i] > 0x7a)
			break;
		type[i] = driveLabel[i];
	}
	if(type[0] == 0)
		return 0;
	for(int i = 0; i < msio_drivers->length; i++){
		msio_device_driver* driver = msio_drivers->base[i];
		if(util_str_equals(type, driver->type))
			return driver;
	}
	return 0;
}

uint8_t msio_get_drive_num(char* driveLabel){
	size_t parsed = -1;
	for(int i = 0; i < 5; i++){
		if(driveLabel[i] > 0x2f || driveLabel[i] < 0x3a){
			parsed = util_str_to_int(driveLabel + i);
		}
	}
	if(parsed > 0xff)
		return 0xff;
	return parsed;
}
