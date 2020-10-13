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

#ifndef __ARCH_RTC_H__
#define __ARCH_RTC_H__


#define RTC_SELECT_PORT 0x70
#define RTC_DATA_PORT 0x71

#define RTC_REGISTER_SECOND 0
#define RTC_REGISTER_MINUTE 2
#define RTC_REGISTER_HOUR 4
#define RTC_REGISTER_WEEKDAY 6
#define RTC_REGISTER_DAY 7
#define RTC_REGISTER_MONTH 8
#define RTC_REGISTER_YEAR 9
#define RTC_REGISTER_CENTURY 0x32
#define RTC_REGISTER_STATUS_A 0xa
#define RTC_REGISTER_STATUS_B 0xb


void rtc_set(uint8_t reg, uint8_t data);
uint8_t rtc_get(uint8_t reg);
bool rtc_updating();

void rtc_get_time(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* minute, uint8_t* second);
uint8_t rtc_bcd_to_dec(uint8_t v);


#endif /* __ARCH_RTC_H__ */
