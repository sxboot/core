
#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/list.h>



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

