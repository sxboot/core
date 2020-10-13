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

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <x86/rtc.h>
#include <x86/x86.h>



void rtc_set(uint8_t reg, uint8_t data){
	x86_outb(RTC_SELECT_PORT, reg);
	x86_outb(RTC_DATA_PORT, data);
}

uint8_t rtc_get(uint8_t reg){
	x86_outb(RTC_SELECT_PORT, reg);
	return x86_inb(RTC_DATA_PORT);
}

bool rtc_updating(){
	return (rtc_get(RTC_REGISTER_STATUS_A) & 0x80) != 0;
}


void rtc_get_time(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* minute, uint8_t* second){
	while(rtc_updating());

	bool btc = (rtc_get(RTC_REGISTER_STATUS_B) & 0x4) == 0;

	if(year){
		if(btc)
			*year = rtc_bcd_to_dec(rtc_get(RTC_REGISTER_YEAR)) + (uint16_t) rtc_bcd_to_dec(rtc_get(RTC_REGISTER_CENTURY)) * 100;
		else
			*year = rtc_get(RTC_REGISTER_YEAR) + (uint16_t) rtc_get(RTC_REGISTER_CENTURY) * 100;
	}
	if(month){
		*month = rtc_get(RTC_REGISTER_MONTH);
		if(btc)
			*month = rtc_bcd_to_dec(*month);
	}
	if(day){
		*day = rtc_get(RTC_REGISTER_DAY);
		if(btc)
			*day = rtc_bcd_to_dec(*day);
	}
	if(hour){
		*hour = rtc_get(RTC_REGISTER_HOUR);
		if(btc)
			*hour = rtc_bcd_to_dec(*hour);
	}
	if(minute){
		*minute = rtc_get(RTC_REGISTER_MINUTE);
		if(btc)
			*minute = rtc_bcd_to_dec(*minute);
	}
	if(second){
		*second = rtc_get(RTC_REGISTER_SECOND);
		if(btc)
			*second = rtc_bcd_to_dec(*second);
	}
}

uint8_t rtc_bcd_to_dec(uint8_t v){
	return (v >> 4) * 10 + (v & 0xf);
}

