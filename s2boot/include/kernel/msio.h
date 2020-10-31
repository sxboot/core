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

#ifndef __KERNEL_MSIO_H__
#define __KERNEL_MSIO_H__


typedef status_t (*MSIO_DRIVER_INFO)(uint8_t number, uint64_t* sectors, size_t* sectorSize);
typedef status_t (*MSIO_DRIVER_READ)(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t dest);
typedef status_t (*MSIO_DRIVER_WRITE)(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t source);

#pragma pack(push,1)
typedef struct msio_device_driver{
	char type[5];
	MSIO_DRIVER_INFO getInfo;
	MSIO_DRIVER_READ read;
	MSIO_DRIVER_WRITE write;
} msio_device_driver;

typedef struct msio_device{
	char name[11];
	uint8_t number;
	msio_device_driver* driver;
	uint64_t sectors;
	size_t sectorSize;
} msio_device;
#pragma pack(pop)

status_t msio_init();
status_t msio_attach_driver(char* type, MSIO_DRIVER_INFO getInfo, MSIO_DRIVER_READ read,
	MSIO_DRIVER_WRITE write);
status_t msio_read_drive(char* driveLabel, uint64_t sector, uint16_t sectorCount, size_t dest);
status_t msio_get_device(char* driveLabel, msio_device** deviceWrite);
msio_device_driver* msio_get_driver(char* driveLabel);
uint8_t msio_get_drive_num(char* driveLabel);


#endif /* __KERNEL_MSIO_H__ */
