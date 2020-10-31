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
#include <klibc/string.h>
#include <kernel/errc.h>
#include <kernel/util.h>
#include <kernel/list.h>
#include <kernel/cli.h>
#include <kernel/msio.h>

static list_array* msio_drivers = NULL;
static list_array* msio_devices = NULL;

status_t msio_init(){
	status_t status = 0;
	msio_drivers = list_array_create(0);
	if(msio_drivers == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &msio_drivers);
	msio_devices = list_array_create(0);
	if(msio_devices == NULL)
		FERROR(TSX_OUT_OF_MEMORY);
	reloc_ptr((void**) &msio_devices);

	_end:
	return status;
}

status_t msio_attach_driver(char* type, MSIO_DRIVER_INFO getInfo, MSIO_DRIVER_READ read,
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
	driver->getInfo = getInfo;
	driver->read = read;
	driver->write = write;
	reloc_ptr((void**) &driver->getInfo);
	reloc_ptr((void**) &driver->read);
	reloc_ptr((void**) &driver->write);
	list_array_push(msio_drivers, driver);
	_end:
	return status;
}

status_t msio_read_drive(char* driveLabel, uint64_t sector, uint16_t sectorCount, size_t dest){
	status_t status = 0;
	void* tmpBuf = 0;
	size_t tmpBufSize = 0;
	msio_device* dev = 0;
	status = msio_get_device(driveLabel, &dev);
	CERROR();
	if(dev->sectorSize == 512){
		status = dev->driver->read(dev->number, sector, sectorCount, dest);
		CERROR();
	}else{
		uint64_t lgLBA = sector * 512 / dev->sectorSize;
		size_t lgCount = sectorCount * 512 / dev->sectorSize;
		if(sectorCount * 512 % dev->sectorSize != 0)
			lgCount++;
		if(dev->sectorSize >= 512 && (sector + sectorCount) * 512 > (lgLBA + lgCount) * dev->sectorSize)
			lgCount++;
		tmpBufSize = dev->sectorSize * lgCount;
		tmpBuf = kmalloc(tmpBufSize);
		if(!tmpBuf)
			FERROR(TSX_OUT_OF_MEMORY);
		status = dev->driver->read(dev->number, lgLBA, lgCount, (size_t) tmpBuf);
		CERROR();
		memcpy((void*) dest, tmpBuf + (dev->sectorSize >= 512 ? ((sector % (dev->sectorSize / 512)) * 512) : 0), sectorCount * 512);
	}
	_end:
	if(tmpBuf)
		kfree(tmpBuf, tmpBufSize);
	return status;
}

status_t msio_get_device(char* driveLabel, msio_device** deviceWrite){
	status_t status = 0;
	for(size_t i = 0; i < msio_devices->length; i++){
		msio_device* dev = msio_devices->base[i];
		if(strcmp(dev->name, driveLabel) == 0){
			*deviceWrite = dev;
			goto _end;
		}
	}
	// not in device list
	char type[5];
	type[4] = 0;
	for(int i = 0; i < 4; i++){
		type[i] = 0;
		if(driveLabel[i] < 0x61 || driveLabel[i] > 0x7a)
			break;
		type[i] = driveLabel[i];
	}
	if(type[0] == 0)
		FERROR(TSX_INVALID_SYNTAX);

	msio_device_driver* driver = msio_get_driver(type);
	if(!driver)
		FERROR(TSX_NO_DRIVER);

	uint8_t number = msio_get_drive_num(driveLabel);
	if(number >= 0xff)
		FERROR(TSX_INVALID_SYNTAX);
	uint64_t sectors = 0;
	size_t sectorSize = 0;
	status = driver->getInfo(number, &sectors, &sectorSize);
	CERROR();
	if(!sectors || !sectorSize)
		FERROR(TSX_ERROR);
	msio_device* dev = kmalloc(sizeof(msio_device));
	if(!dev)
		FERROR(TSX_OUT_OF_MEMORY);
	memcpy(dev->name, driveLabel, strlen(driveLabel) + 1);
	dev->number = number;
	dev->driver = driver;
	dev->sectors = sectors;
	dev->sectorSize = sectorSize;
	reloc_ptr((void**) &dev->driver);
	list_array_push(msio_devices, dev);
	*deviceWrite = dev;
	_end:
	return status;
}

msio_device_driver* msio_get_driver(char* driveType){
	for(int i = 0; i < msio_drivers->length; i++){
		msio_device_driver* driver = msio_drivers->base[i];
		if(util_str_equals(driveType, driver->type))
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


void cli_command_devinfo(uint8_t dest, char* args){
	msio_device* dev;
	status_t status = msio_get_device(args, &dev);
	if(status != TSX_SUCCESS){
		cli_printf(dest, "Error %u (%s)\n", status, errcode_get_name(status));
		kernel_print_error_trace();
		return;
	}
	cli_printf(dest, "Device %s: %u sectors (%uMiB); %u-byte sectors; driver @ %X\n", dev->name, (size_t) dev->sectors,
		(size_t) (dev->sectors * dev->sectorSize / 1024 / 1024), dev->sectorSize, dev->driver);
}
