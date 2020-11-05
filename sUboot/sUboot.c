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
 * sUboot.c - Replacement for s0boot and s1boot on UEFI platforms.
 */

#include <uefi.h>
#include <sUboot.h>
#include <shared/s1bootdecl.h>
#include <arch.h>


static char* suboot_version = "1.2";


static char* firmwareDriverName = "uefi";


static EFI_HANDLE imageHandle;
static EFI_SYSTEM_TABLE* systemTable;


static EFI_GUID graphicsOutputProtocolGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GUID blockIOProtocolGUID = EFI_BLOCK_IO_PROTOCOL_GUID;

static EFI_GRAPHICS_OUTPUT_PROTOCOL* graphicsOutputProtocol = NULL;

static EFI_HANDLE deviceHandles[128];
static EFI_BLOCK_IO_PROTOCOL* devices[128];
static size_t devicesNum = 0;
static size_t bootDrive = 0; // boot device index
static size_t bootPartNum = 0;

static uint32_t bootDriveSignature = 0;
static bool bootGPT;

static size_t s2bootEntry = 0;

static s1boot_data s1data;

static void* reservePageLocation = NULL;
static size_t reservePageSize = 16;


static char* gptSignature = "EFI PART";
static char* fat16_signature = "FAT16   ";


EFI_STATUS EFIAPI efi_main(EFI_HANDLE _imageHandle, EFI_SYSTEM_TABLE* _systemTable){
	imageHandle = _imageHandle;
	systemTable = _systemTable;

	EFI_STATUS status = EFI_SUCCESS;

	EFI_BOOT_SERVICES* bs = systemTable->BootServices;

	memset(&s1data, 0, sizeof(s1boot_data));

	status = suboot_set_graphics();
	if(EFI_ERROR(status))
		goto _error;

	suboot_print("sUboot version ");
	suboot_print(suboot_version);
	suboot_printNln();

	suboot_printNln();

	status = suboot_get_devices();
	if(EFI_ERROR(status) || devicesNum == 0)
		goto _error;

	status = suboot_load_s2boot();
	if(EFI_ERROR(status))
		goto _error;

	status = suboot_parse_s2boot();
	if(EFI_ERROR(status) || s2bootEntry == 0)
		goto _error;

	status = suboot_get_bdrive_data();
	if(EFI_ERROR(status))
		goto _error;

	status = suboot_remap();
	if(EFI_ERROR(status))
		goto _error;

	// since we will user our own memory allocation system with boot services still running, we need to reserve some space in case it wants to allocate something
	// (this space gets freed later, after it has been reserved in s2boot memory map)
	reservePageLocation = suboot_alloc(reservePageSize);

	status = suboot_get_memory_map();
	if(EFI_ERROR(status))
		goto _error;


	s1data.headerVersion = 49;
	s1data.bootDriveSg = bootDriveSignature;
	s1data.bootPartN = bootPartNum;
	s1data.bootFlags = 0x2 | (bootGPT ? 1 : 0);
	s1data.memoryBase = 0;
	s1data.s1serviceCallback = (status_t (*)(size_t, size_t, size_t, size_t)) &suboot_callback;
	s1data.uefiSystemTable = (uint64_t) &systemTable;

	bs->Stall(200000);
	suboot_println("Calling s2boot");

	if(s1data.framebufferBase)
		for(size_t addr = 0; addr < s1data.videoHeight * s1data.videoPitch; addr++){
			*((uint8_t*) (addr + s1data.framebufferBase)) = 0;
		}

	S2PREENTRY();
	(void) ((void (S2_API *) (s1boot_data*)) (s2bootEntry))(&s1data);

	HALT();


	suboot_printNln();
	suboot_println("End");
	_error:
	suboot_printNln();
	suboot_print("Error: ");
	suboot_print_hex(status);
	suboot_printNln();
	bs->Stall(3000000);
	return status;
}

EFI_STATUS suboot_get_devices(){
	EFI_STATUS status = EFI_SUCCESS;

	EFI_BOOT_SERVICES* bs = systemTable->BootServices;

	// get all block IO devices (physical devices AND partitions)
	EFI_HANDLE* handles = NULL;
	size_t handleCount = 0;
	status = bs->LocateHandleBuffer(ByProtocol, &blockIOProtocolGUID, NULL, &handleCount, &handles);
	if(EFI_ERROR(status) || handles == NULL){
		suboot_print("Error while getting handles for EFI_BLOCK_IO_PROTOCOL_GUID");
		goto _error;
	}
	suboot_print("Got ");
	suboot_print_dec(handleCount);
	suboot_print(" Block IO handles (");
	suboot_print_hex((size_t) handles);
	suboot_print(")");
	suboot_printNln();

	size_t deviceNumCounter = 0;
	for(size_t i = 0; i < handleCount; i++){
		EFI_BLOCK_IO_PROTOCOL* blockIOProtocol;

		status = bs->HandleProtocol(handles[i], &blockIOProtocolGUID, (void**) &blockIOProtocol);
		if(EFI_ERROR(status) || blockIOProtocol == NULL){
			suboot_print("Error while getting EFI_BLOCK_IO_PROTOCOL for handle ");
			suboot_print_dec(i);
			suboot_print(": ");
			suboot_print_hex(status);
			suboot_printNln();
			status = EFI_SUCCESS;
			continue;
		}
		if(blockIOProtocol->Media->LogicalPartition)
			continue;

		suboot_print("Block device ");
		suboot_print(firmwareDriverName);
		suboot_print_dec(deviceNumCounter);
		suboot_print(" (");
		suboot_print_hex((size_t) blockIOProtocol);
		suboot_print(", ");
		suboot_print_dec(blockIOProtocol->Media->MediaId);
		suboot_print("): ");
		suboot_print_dec(blockIOProtocol->Media->LastBlock * blockIOProtocol->Media->BlockSize);
		suboot_print(" bytes, ");
		suboot_print_dec(blockIOProtocol->Media->BlockSize);
		suboot_print("-byte sectors");
		suboot_printNln();

		deviceHandles[deviceNumCounter] = handles[i];
		devices[deviceNumCounter] = blockIOProtocol;
		deviceNumCounter++;
	}
	devicesNum = deviceNumCounter;

	if(devicesNum == 0){
		suboot_println("No devices found");
	}
	suboot_printNln();

	_error:
	return status;
}

EFI_STATUS suboot_load_s2boot(){
	EFI_STATUS status = EFI_SUCCESS;

	suboot_println("Attempting to load /boot/s2boot");
	void* s2bootImageLocation = NULL;
	size_t s2bootImageSize = 0;
	for(int i = 0; i < devicesNum; i++){
		for(int p = 0; p < 4; p++){
			status = suboot_read_file(i, p, "BOOT       ", "S2BOOT     ", &s2bootImageLocation, &s2bootImageSize);
			if(EFI_ERROR(status) || s2bootImageLocation == NULL){
				status = EFI_SUCCESS;
				continue;
			}
			suboot_print("Loaded s2boot from ");
			suboot_print(firmwareDriverName);
			suboot_print_dec(i);
			suboot_print(".");
			suboot_print_dec(p);
			suboot_print(" at ");
			suboot_print_hex((size_t) s2bootImageLocation);
			suboot_printNln();
			bootDrive = i;
			bootPartNum = p;
		}
	}
	if(!s2bootImageLocation){
		status = EFI_NOT_FOUND;
		suboot_println("Could not find s2boot in any available device");
		goto _end;
	}
	s1data.s2bootAddress = (size_t) s2bootImageLocation;
	s1data.s2bootSize = s2bootImageSize;

	suboot_println("Attempting to load /boot/bdd.ko");
	void* bddImageLocation = NULL;
	size_t bddImageSize = 0;
	status = suboot_read_file(bootDrive, bootPartNum, "BOOT       ", "BDD     KO ", &bddImageLocation, &bddImageSize);
	if(EFI_ERROR(status) || bddImageLocation == NULL){
		status = EFI_SUCCESS;
		suboot_println("Could not find bdd.ko");
	}else{
		suboot_print("Loaded bdd.ko from ");
		suboot_print(firmwareDriverName);
		suboot_print_dec(bootDrive);
		suboot_print(".");
		suboot_print_dec(bootPartNum);
		suboot_print(" at ");
		suboot_print_hex((size_t) bddImageLocation);
		suboot_printNln();
	}
	s1data.bddAddress = (size_t) bddImageLocation;
	s1data.bddSize = bddImageSize;

	_end:
	suboot_printNln();
	return status;
}

EFI_STATUS suboot_parse_s2boot(){
	if(!s1data.s2bootAddress)
		return EFI_INVALID_PARAMETER;
	EFI_STATUS status = EFI_SUCCESS;
	suboot_print("Loading s2boot ELF");

	s2map_entry* s2map = suboot_alloc(1);
	size_t s2mapIndex = 0;
	if(!s2map){
		status = EFI_OUT_OF_RESOURCES;
		suboot_printNln();
		goto _end0;
	}

	elf_file* file = (elf_file*) s1data.s2bootAddress;
	if(file->ei_mag != ELF_MAGIC){
		status = EFI_LOAD_ERROR;
		suboot_printNln();
		suboot_print("s2boot has invalid ei_mag");
		goto _end;
	}
	if(file->ei_class != ELF_CLASS || file->e_machine != ELF_MACHINE){
		status = EFI_LOAD_ERROR;
		suboot_printNln();
		suboot_print("s2boot has invalid ei_class or e_machine");
		goto _end;
	}
	if(!(file->e_type == 2 || file->e_type == 3)){
		status = EFI_LOAD_ERROR;
		suboot_printNln();
		suboot_print("s2boot has invalid e_type, expected 2 or 3");
		goto _end;
	}

	elf_ph* ph = (elf_ph*) (file->e_phoff + (size_t) file);
	if(ph == 0)
		goto _parse_error;
	if(file->e_phentsize != sizeof(elf_ph))
		goto _parse_error;
	size_t max = 0;
	elf_dyn* dynsec = NULL;
	size_t dynseclen = 0;
	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type == 1){
			if(ph[i].p_vaddr + ph[i].p_memsz > max)
				max = ph[i].p_vaddr + ph[i].p_memsz;
		}else if(ph[i].p_type == 2){
			dynsec = (elf_dyn*) (ph[i].p_offset + (size_t) file);
			dynseclen = ph[i].p_filesz;
		}
	}
	if(max == 0)
		goto _parse_error;

	size_t size = max;
	size_t sizePages = size / 4096;
	if(size % 4096 != 0)
		sizePages++;
	void* base = suboot_alloc(sizePages);
	if(!base){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}

	suboot_print("  base = ");
	suboot_print_hex((size_t) base);

	for(int i = 0; i < file->e_phnum; i++){
		if(ph[i].p_type != 1)
			continue;
		memset((void*) (ph[i].p_vaddr + (size_t) base), 0, ph[i].p_memsz);
		memcpy((void*) (ph[i].p_vaddr + (size_t) base), (void*) (ph[i].p_offset + (size_t) file), ph[i].p_filesz);
		s2map[s2mapIndex].address = ph[i].p_vaddr + (size_t) base;
		s2map[s2mapIndex].end = s2map[s2mapIndex].address + ph[i].p_memsz;
		s2mapIndex++;
	}

	if(dynsec && dynseclen){
		elf_rel* relsec = NULL;
		size_t relseclen = 0;
		elf_rela* relasec = NULL;
		size_t relaseclen = 0;
		for(int i = 0; i < dynseclen / sizeof(elf_dyn); i++){
			if(dynsec[i].type == 7){
				relasec = (elf_rela*) (dynsec[i].val + (size_t) base);
			}else if(dynsec[i].type == 8){
				relaseclen = dynsec[i].val;
			}else if(dynsec[i].type == 17){
				relsec = (elf_rel*) (dynsec[i].val + (size_t) base);
			}else if(dynsec[i].type == 18){
				relseclen = dynsec[i].val;
			}
		}
		if(relsec && relseclen){
			for(int i = 0; i < relseclen / sizeof(elf_rel); i++){
				if(relsec[i].r_info == 8){
					*((size_t*) (relsec[i].r_offset + (size_t) base)) += (size_t) base;
				}
			}
		}
		if(relasec && relaseclen){
			for(int i = 0; i < relaseclen / sizeof(elf_rela); i++){
				if(relasec[i].r_info == 8){
					*((size_t*) (relasec[i].r_offset + (size_t) base)) = (size_t) base + relasec[i].r_addend;
				}
			}
		}
	}

	s2bootEntry = file->e_entry + (size_t) base;
	s1data.s2mapStart = (uint64_t) s2map;
	s1data.s2mapLength = s2mapIndex;
	s1data.s2bootBase = (size_t) base;

	suboot_print("  entry = ");
	suboot_print_hex(s2bootEntry);

	goto _end;
	_parse_error:
	status = EFI_LOAD_ERROR;
	_end:
	suboot_printNln();
	if(EFI_ERROR(status))
		suboot_println("s2boot parse error");
	_end0:
	suboot_printNln();
	return status;
}

EFI_STATUS suboot_get_bdrive_data(){
	EFI_STATUS status = EFI_SUCCESS;
	void* mbrAddr = suboot_alloc(1);
	if(!mbrAddr){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}
	status = suboot_read_sectors(bootDrive, 0, 1, mbrAddr);
	if(EFI_ERROR(status))
		goto _end;
	bootDriveSignature = *((uint32_t*) (((size_t) mbrAddr) + 0x1b8));

	status = suboot_read_sectors(bootDrive, 1, 1, mbrAddr);
	if(EFI_ERROR(status))
		goto _end;
	bootGPT = suboot_memcmp(mbrAddr, gptSignature, 8);

	suboot_print("Boot device: sig=");
	suboot_print_hex(bootDriveSignature);
	suboot_print(" gpt=");
	suboot_print_dec(bootGPT);
	suboot_printNln();
	_end:
	if(mbrAddr)
		suboot_free(mbrAddr, 1);
	suboot_printNln();
	return status;
}

static size_t preferredGraphicsModes[2][2] = {
	{800, 600},
	{640, 480}
};

EFI_STATUS suboot_set_graphics(){
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
	EFI_STATUS status = systemTable->BootServices->LocateProtocol(&graphicsOutputProtocolGUID, NULL, (void**) &gop);
	if(EFI_ERROR(status) || gop == NULL)
		goto _end;
	graphicsOutputProtocol = gop;

	if(!(gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || gop->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)){
		bool found = false;
		for(int m = 0; m < 2; m++){
			suboot_print("Attempting to set video mode ");
			suboot_print_dec(preferredGraphicsModes[m][0]);
			suboot_print("x");
			suboot_print_dec(preferredGraphicsModes[m][1]);
			suboot_println("x32");
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo = NULL;
			UINTN modeInfoSize = 0;
			for(UINT32 i = 0; i < gop->Mode->MaxMode; i++){
				status = gop->QueryMode(gop, i, &modeInfoSize, &modeInfo);
				if(EFI_ERROR(status) || modeInfo == NULL)
					continue;
				if(modeInfo->HorizontalResolution == preferredGraphicsModes[m][0] && modeInfo->VerticalResolution == preferredGraphicsModes[m][1] &&
					(modeInfo->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || modeInfo->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)){
					found = true;
					systemTable->BootServices->Stall(200000);
					gop->SetMode(gop, i);
					break;
				}
				modeInfo = NULL;
			}
			if(found)
				break;
		}
	}
	s1data.framebufferBase = gop->Mode->FrameBufferBase;
	s1data.videoWidth = gop->Mode->Info->HorizontalResolution;
	s1data.videoHeight = gop->Mode->Info->VerticalResolution;
	s1data.videoPitch = gop->Mode->Info->PixelsPerScanLine * 4;
	s1data.videoBpp = 32;
	s1data.videoMode = 0;

	_end:
	return status;
}

EFI_STATUS suboot_remap(){
	// setup own page tables because firmware may use large pages which s2boot cant handle
	UINTN memoryMapSize = 0;
	UINTN memoryMapKey = 0;
	UINTN memoryMapDescriptorSize = 0;
	UINT32 memoryMapDescriptorVersion = 0;
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, NULL, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
	if(memoryMapSize == 0)
		goto _end;
	status = EFI_SUCCESS;

	memoryMapSize += memoryMapDescriptorSize;
	size_t memoryMapSizePages = memoryMapSize / 4096;
	if(memoryMapSize % 4096 != 0)
		memoryMapSizePages++;

	EFI_MEMORY_DESCRIPTOR* memoryMap = suboot_alloc(memoryMapSizePages);
	if(!memoryMap){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}
	status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
	if(EFI_ERROR(status)){
		suboot_free(memoryMap, memoryMapSizePages);
		goto _end;
	}

	size_t memoryMapLength = memoryMapSize / memoryMapDescriptorSize;

	size_t highest = suboot_get_highest_accessed_address();
	for(int i = 0; i < memoryMapLength; i++){
		EFI_MEMORY_DESCRIPTOR* memoryMapEntry = (EFI_MEMORY_DESCRIPTOR*) ((size_t) memoryMap + memoryMapDescriptorSize * i);
		if(memoryMapEntry->Type != EfiConventionalMemory && memoryMapEntry->PhysicalStart + memoryMapEntry->NumberOfPages * 4096 > highest){
			highest = memoryMapEntry->PhysicalStart + memoryMapEntry->NumberOfPages * 4096;
		}
	}
	suboot_free(memoryMap, memoryMapSizePages);

	suboot_print("Highest used address: ");
	suboot_print_hex(highest);
	suboot_printNln();

									/* leave space for page tables */		/* leave 16 pages for additional space */
	size_t highMap = highest + highest / 0x200000 * 0x1000 + highest / 0x40000000 * 0x1000 + 0x1000 * 16;
	if(highMap < highest) // in case it wraps around
		highMap = highest;
	suboot_print("Mapping ");
	suboot_print_hex(0);
	suboot_print(" - ");
	suboot_print_hex(highMap);
	suboot_printNln();
	status = suboot_mem_id_map(0, highMap);
	if(EFI_ERROR(status))
		goto _end;
	_end:
	suboot_printNln();
	return status;
}

EFI_STATUS suboot_get_memory_map(){
	UINTN memoryMapSize = 0;
	UINTN memoryMapKey = 0;
	UINTN memoryMapDescriptorSize = 0;
	UINT32 memoryMapDescriptorVersion = 0;
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, NULL, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
	if(memoryMapSize == 0)
		goto _end;
	status = EFI_SUCCESS;

	memoryMapSize += memoryMapDescriptorSize * 2; // assume 2 entries get added after suboot_alloc() for uefi map and s2boot map
	size_t memoryMapSizePages = memoryMapSize / 4096;
	if(memoryMapSize % 4096 != 0)
		memoryMapSizePages++;
	size_t memoryMapLength = memoryMapSize / memoryMapDescriptorSize;

	size_t s2memoryMapSize = memoryMapLength * sizeof(mmap_entry);
	size_t s2memoryMapSizePages = s2memoryMapSize / 4096;
	if(s2memoryMapSize % 4096 != 0)
		s2memoryMapSizePages++;

	mmap_entry* mmap = suboot_alloc(s2memoryMapSizePages);
	EFI_MEMORY_DESCRIPTOR* memoryMap = suboot_alloc(memoryMapSizePages);
	status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
	if(EFI_ERROR(status))
		goto _end;

	memoryMapLength = memoryMapSize / memoryMapDescriptorSize;

	for(int i = 0; i < memoryMapLength; i++){
		EFI_MEMORY_DESCRIPTOR* memoryMapEntry = (EFI_MEMORY_DESCRIPTOR*) ((size_t) memoryMap + memoryMapDescriptorSize * i);
		mmap[i].addr = memoryMapEntry->PhysicalStart;
		mmap[i].size = memoryMapEntry->NumberOfPages * 4096;
		mmap[i].type = memoryMapEntry->Type;
	}

	s1data.mmapStart = (size_t) mmap;
	s1data.mmapLength = memoryMapLength;

	suboot_print("Memory map contains ");
	suboot_print_dec(memoryMapLength);
	suboot_println(" entries");

	_end:
	suboot_printNln();
	return status;
}


EFI_STATUS suboot_read_sectors(uint8_t id, size_t lba, size_t numSectors, void* dest){
	if(id >= devicesNum)
		return EFI_NO_MEDIA;
	return devices[id]->ReadBlocks(devices[id], devices[id]->Media->MediaId, lba, numSectors * devices[id]->Media->BlockSize, dest);
}

// i have decided that it would be much easier to just write a filesystem driver myself to load s2boot rather than try to find out how to use the giant mess that is UEFI

// its also easier to probe every available partition in every device to find the boot drive (by trying to load s2boot from there) because it is apparently literally
// impossible to find out which of the block IO devices is the boot drive (where this efi image was loaded from)

size_t suboot_get_partition_lba(uint8_t driveId, uint8_t partNum){
	size_t partStart = SIZE_MAX;
	void* addr = suboot_alloc(1);
	if(!addr)
		goto _end;
	EFI_STATUS status = suboot_read_sectors(driveId, 1, 1, addr);
	if(EFI_ERROR(status))
		goto _end;
	bool gpt = suboot_memcmp(addr, gptSignature, 8);
	size_t bytesPerSector = devices[driveId]->Media->BlockSize;
	if(gpt){
		gpt_header* gptHeader = (gpt_header*) addr;
		uint32_t partitionEntrySize = gptHeader->partitionEntrySize;
		uint64_t sectorOffset = gptHeader->currentLBA + 1 + (partNum * partitionEntrySize) / bytesPerSector;
		gpt_partition_entry* partitions = addr;
		status = suboot_read_sectors(driveId, sectorOffset, 1, addr);
		if(EFI_ERROR(status))
			goto _end;
		gpt_partition_entry* partition = partitions + (partNum % (bytesPerSector / partitionEntrySize));
		partStart = partition->firstLBA;
	}else{
		if(partNum >= 4)
			goto _end;
		status = suboot_read_sectors(driveId, 0, 1, addr);
		if(EFI_ERROR(status))
			goto _end;
		mbr_partition_entry* partition = (mbr_partition_entry*) ((size_t) addr + 0x1be + 0x10 * partNum);
		partStart = partition->startSector;
	}
	_end:
	if(addr)
		suboot_free(addr, 1);
	return partStart;
}

EFI_STATUS suboot_read_file(uint8_t driveId, uint8_t partNum, char* first, char* second, void** dest, size_t* size){
	size_t partStart = suboot_get_partition_lba(driveId, partNum);
	if(partStart == 0)
		return EFI_NO_MEDIA;
	EFI_STATUS status = EFI_SUCCESS;
	fs_fat16_bpb* bpb = suboot_alloc(1);
	fs_fat_dir_entry* dirTable = suboot_alloc(1);
	if(!bpb || !dirTable){
		status = EFI_OUT_OF_RESOURCES;
		goto _end;
	}

	status = suboot_read_sectors(driveId, partStart, 1, bpb);
	if(EFI_ERROR(status))
		goto _end;

	if(!suboot_memcmp(bpb->filesystemName, fat16_signature, 8)){
		status = EFI_INVALID_PARAMETER;
		goto _end;
	}

	size_t lbaDataStart = (bpb->hiddenSectors + bpb->reservedSectors + bpb->numberOfFATs * bpb->sectorsPerFAT + bpb->rootEntries * 32 / bpb->bytesPerSector);

	// get dir location
	size_t lba = bpb->hiddenSectors + bpb->reservedSectors + bpb->numberOfFATs * bpb->sectorsPerFAT;
	bool found = FALSE;
	uint16_t cluster = 0;
	for(int ts = 0; ts < (bpb->rootEntries * 32 / bpb->bytesPerSector); ts++){
		status = suboot_read_sectors(driveId, lba + ts, 1, dirTable);
		if(EFI_ERROR(status))
			goto _end;
		for(int ti = 0; ti < bpb->bytesPerSector / 32; ti++){
			if(!(dirTable[ti].attributes & FS_FAT_DIR_ENTRY_SUBDIR))
				continue;
			if(suboot_memcmp(dirTable[ti].nameShort, first, 11)){ // 11 is nameShort + extShort
				found = TRUE;
				cluster = dirTable[ti].clusterLow;
				break;
			}
		}
		if(found)
			break;
	}
	if(cluster < 2){
		status = EFI_NOT_FOUND;
		goto _end;
	}

	// get file location
	size_t fileSize = 0;
	lba = lbaDataStart + (cluster - 2) * bpb->sectorsPerCluster;
	found = FALSE;
	cluster = 0;
	for(int ts = 0; ts < bpb->sectorsPerCluster; ts++){
		status = suboot_read_sectors(driveId, lba + ts, 1, dirTable);
		if(EFI_ERROR(status))
			goto _end;
		for(int ti = 0; ti < bpb->bytesPerSector / 32; ti++){
			if(!(dirTable[ti].attributes & FS_FAT_DIR_ENTRY_FILE))
				continue;
			if(suboot_memcmp(dirTable[ti].nameShort, second, 11)){
				found = TRUE;
				cluster = dirTable[ti].clusterLow;
				fileSize = dirTable[ti].size;
				break;
			}
		}
		if(found)
			break;
	}
	if(cluster < 2 || fileSize == 0){
		status = EFI_NOT_FOUND;
		goto _end;
	}

	size_t destPages = fileSize / 4096;
	if(fileSize % 4096 != 0)
		destPages++;
	void* destAddr = suboot_alloc(destPages);
	if(!destAddr){
		status = EFI_OUT_OF_RESOURCES;
		goto _end_read;
	}

	// read file
	found = FALSE;
	size_t read = 0;
	size_t lastFATSectorOffset = -1;
	uint16_t* fat = (uint16_t*) dirTable;
	while(cluster >= 0x0002 && cluster <= 0xffef){
		lba = lbaDataStart + (cluster - 2) * bpb->sectorsPerCluster;
		status = suboot_read_sectors(driveId, lba, bpb->sectorsPerCluster, destAddr + bpb->bytesPerSector * bpb->sectorsPerCluster * read);
		if(EFI_ERROR(status))
			goto _end_read;
		size_t FATSectorOffset = cluster * 2 / bpb->bytesPerSector;
		if(lastFATSectorOffset != FATSectorOffset){
			status = suboot_read_sectors(driveId, bpb->hiddenSectors + bpb->reservedSectors + FATSectorOffset, 1, fat);
			if(EFI_ERROR(status))
				goto _end_read;
			lastFATSectorOffset = FATSectorOffset;
		}
		cluster = fat[cluster % (bpb->bytesPerSector / 2)];
		read++;
	}
	if(dest)
		*dest = destAddr;
	if(size)
		*size = fileSize;

	_end_read:
	if(destAddr && EFI_ERROR(status))
		suboot_free(destAddr, destPages);
	_end:
	if(dirTable)
		suboot_free(dirTable, 1);
	if(bpb)
		suboot_free(bpb, 1);
	return status;
}



void* suboot_alloc(size_t pages){
	EFI_PHYSICAL_ADDRESS address = 0;
	EFI_STATUS status = systemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &address);
	if(EFI_ERROR(status) || address == 0){
		return NULL;
	}
	return (void*) address;
}

void suboot_free(void* addr, size_t pages){
	systemTable->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) addr, pages);
}



void suboot_println(char* str){
	suboot_print(str);
	suboot_printNln();
}

void suboot_printNln(){
	suboot_print("\n");
}


#if ARCH_BITS == 64
#define STDIO64__HEX_LEN 16
static char* hexTmp = "0x0000000000000000";
#elif ARCH_BITS == 32
#define STDIO64__HEX_LEN 8
static char* hexTmp = "0x00000000";
#endif

void suboot_print_hex(size_t hex){
	for(int i = STDIO64__HEX_LEN + 1; i >= 2; i--){
		hexTmp[i] = '0';
		uint8_t cr = hex & 0xf;
		if(cr >= 10)
			hexTmp[i] += 7;
		hexTmp[i] += cr;
		hex >>= 4;
	}
	suboot_print(hexTmp);
}

static char* decTmp = "000000000000000000000";

void suboot_print_dec(size_t dec){
	uint8_t si = 20;
	for(int i = 20; i >= 0; i--){
		decTmp[i] = '0';
	}
	for(int i = 20; i >= 0 && dec != 0; i--){
		uint8_t cr = dec % 10;
		decTmp[i] = '0' + cr;
		dec /= 10;
		si = i;
	}
	suboot_print(decTmp + si);
}


static WCHAR strTmp[4];

void suboot_print(char* str){
	size_t len = suboot_strlen(str);
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* textOut = systemTable->ConOut;
	for(size_t i = 0; i < len; i++){
		strTmp[0] = str[i];
		if(str[i] == '\n'){
			strTmp[0] = '\r';
			strTmp[1] = '\n';
			strTmp[2] = 0;
		}else{
			strTmp[1] = 0;
		}
		textOut->OutputString(textOut, strTmp);
	}
}

size_t suboot_strlen(char* str){
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}



static status_t sxboot_status[19] = {
	0,
	1,
	21,
	40,
	1,
	1,
	1,
	19,
	1,
	28,
	1,
	1,
	13,
	1,
	15,
	1,
	1,
	1,
	14
};

static bool bootServicesExited = false;

status_t suboot_get_sxboot_status(EFI_STATUS s){
	uint8_t efistat = (uint8_t) s;
	if(efistat >= 19)
		return 1;
	return sxboot_status[efistat];
}

status_t S2_API suboot_callback(size_t num, size_t arg0, size_t arg1, size_t arg2){
	if(num == 1){ // pre init
		if(reservePageLocation)
			suboot_free(reservePageLocation, reservePageSize);
	}else if(num == 2){ // init

	}else if(num == 9){
		if(bootServicesExited)
			return 0;
		UINTN memoryMapSize = 0;
		UINTN memoryMapKey = 0;
		UINTN memoryMapDescriptorSize = 0;
		UINT32 memoryMapDescriptorVersion = 0;
		EFI_STATUS status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, NULL, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
		if(memoryMapSize == 0)
			return 1;
		status = EFI_SUCCESS;

		memoryMapSize += memoryMapDescriptorSize;
		size_t memoryMapSizePages = memoryMapSize / 4096;
		if(memoryMapSize % 4096 != 0)
			memoryMapSizePages++;

		EFI_MEMORY_DESCRIPTOR* memoryMap = suboot_alloc(memoryMapSizePages);
		if(!memoryMap){
			return suboot_get_sxboot_status(EFI_OUT_OF_RESOURCES);
		}
		status = systemTable->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
		if(EFI_ERROR(status)){
			suboot_free(memoryMap, memoryMapSizePages);
			return suboot_get_sxboot_status(status);
		}

		bootServicesExited = true;
		status = systemTable->BootServices->ExitBootServices(imageHandle, memoryMapKey);
		return suboot_get_sxboot_status(status);
	}else if(num == 10){ // set video
		s2callback_videomode* video = (s2callback_videomode*) arg0;
		if(video->mode != 1 || video->bpp != 32 || !graphicsOutputProtocol)
			return 1;
		EFI_STATUS status = EFI_SUCCESS;

		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo = NULL;
		UINTN modeInfoSize = 0;
		for(UINT32 i = 0; i < graphicsOutputProtocol->Mode->MaxMode; i++){
			status = graphicsOutputProtocol->QueryMode(graphicsOutputProtocol, i, &modeInfoSize, &modeInfo);
			if(EFI_ERROR(status) || modeInfo == NULL)
				continue;
			if(modeInfo->HorizontalResolution == video->width && modeInfo->VerticalResolution == video->height &&
				(modeInfo->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || modeInfo->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)){
				graphicsOutputProtocol->SetMode(graphicsOutputProtocol, i);
				video->framebuffer = graphicsOutputProtocol->Mode->FrameBufferBase;
				video->bytesPerLine = modeInfo->PixelsPerScanLine * 4;
				return 0;
			}
			modeInfo = NULL;
		}
		return 1;
	}else if(num == 20){ // get firmware driver name
		*((char**) arg0) = firmwareDriverName;
	}else if(num == 21){ // firmware disk read
		if(bootServicesExited)
			return 1;
		s2callback_readdrive* rdata = (s2callback_readdrive*) arg0;
		EFI_STATUS status = suboot_read_sectors(rdata->num, rdata->lba, rdata->numSectors, (void*) rdata->dest);
		if(EFI_ERROR(status))
			return suboot_get_sxboot_status(status);
	}else if(num == 22){ // firmware disk info
		if(bootServicesExited)
			return 1;
		if(arg0 >= devicesNum)
			return suboot_get_sxboot_status(EFI_NO_MEDIA);
		*((uint64_t*) arg1) = devices[arg0]->Media->LastBlock;
		*((size_t*) arg2) = devices[arg0]->Media->BlockSize;
	}else{
		return 1;
	}
	return 0;
}



bool suboot_memcmp(void* str1, void* str2, size_t len){
	for(int i = 0; i < len; i++){
		if(((uint8_t*) str1)[i] != ((uint8_t*) str2)[i]){
			return false;
		}
	}
	return true;
}

void* memset(void *dest, int val, size_t len){
	unsigned char *ptr = dest;
	while (len-- > 0)
		*ptr++ = val;
	return dest;
}

void* memcpy(void *dest, const void *src, size_t len){
	char *d = dest;
	const char *s = src;
	while (len--)
		*d++ = *s++;
	return dest;
}

