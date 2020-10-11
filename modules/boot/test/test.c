
#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <klibc/stdio.h>
#include <kernel/parse.h>

status_t kboot_start(parse_entry* entry){
	printf("success %u %X\n", 123, 0x456);
	return 123;
}


