
#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>



static char* msio_driver_type = "usb";

status_t msio_init(){
	status_t status = 0;
	_end:
	return status;
}

status_t msio_read(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t dest){
	status_t status = 0;
	return 1;
}

status_t msio_write(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t source){
	status_t status = 0;
	return 1;
}

char* msio_get_driver_type(){
	return msio_driver_type;
}
