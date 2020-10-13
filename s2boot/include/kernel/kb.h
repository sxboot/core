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
 * kb.h - Keyboard functions to be implemented by architecture-specific drivers.
 */

#ifndef __KERNEL_KB_H__
#define __KERNEL_KB_H__


enum KB_ASCII{
	KEY_ESCAPE = 0x1b,
	KEY_1 = '1',
	KEY_2 = '2',
	KEY_3 = '3',
	KEY_4 = '4',
	KEY_5 = '5',
	KEY_6 = '6',
	KEY_7 = '7',
	KEY_8 = '8',
	KEY_9 = '9',
	KEY_0 = '0',
	KEY_MINUS = '-',
	KEY_EQUALS = '=',
	KEY_BACKSPACE = 0x8,
	KEY_TAB = '	',
	KEY_Q = 'q',
	KEY_W = 'w',
	KEY_E = 'e',
	KEY_R = 'r',
	KEY_T = 't',
	KEY_Y = 'y',
	KEY_U = 'u',
	KEY_I = 'i',
	KEY_O = 'o',
	KEY_P = 'p',
	KEY_LBRACKET = '[',
	KEY_RBRACKET = ']',
	KEY_ENTER = 0xd,
	KEY_LEFTCONTROL = 0x1101,
	KEY_A = 'a',
	KEY_S = 's',
	KEY_D = 'd',
	KEY_F = 'f',
	KEY_G = 'g',
	KEY_H = 'h',
	KEY_J = 'j',
	KEY_K = 'k',
	KEY_L = 'l',
	KEY_SEMICOLON = ';',
	KEY_QUOTE = 0x27,
	KEY_GRAVE = '`',
	KEY_LEFTSHIFT = 0x1111,
	KEY_BACKSLASH = 0x5c,
	KEY_Z = 'z',
	KEY_X = 'x',
	KEY_C = 'c',
	KEY_V = 'v',
	KEY_B = 'b',
	KEY_N = 'n',
	KEY_M = 'm',
	KEY_COMMA = ',',
	KEY_DOT = '.',
	KEY_SLASH = '/',
	KEY_RIGHTSHIFT = 0x1112,
	KEY_P_ASTERISK = '*',
	KEY_LEFTALT = 0x1131,
	KEY_SPACE = ' ',
	KEY_CAPSLOCK = 0x1141,
	KEY_F1 = 0x1201,
	KEY_F2 = 0x1202,
	KEY_F3 = 0x1203,
	KEY_F4 = 0x1204,
	KEY_F5 = 0x1205,
	KEY_F6 = 0x1206,
	KEY_F7 = 0x1207,
	KEY_F8 = 0x1208,
	KEY_F9 = 0x1209,
	KEY_F10 = 0x120a,
	KEY_NUMBERLOCK = 0x1151,
	KEY_SCROLLLOCK = 0x1161,
	KEY_P_7 = '7',
	KEY_P_8 = '8',
	KEY_P_9 = '9',
	KEY_P_MINUS = '-',
	KEY_P_4 = '4',
	KEY_P_5 = '5',
	KEY_P_6 = '6',
	KEY_P_PLUS = '+',
	KEY_P_1 = '1',
	KEY_P_2 = '2',
	KEY_P_3 = '3',
	KEY_P_0 = '0',
	KEY_P_DOT = '.',
	KEY_UNKNOWN = 0,
	KEY_F11 = 0x120b,
	KEY_F12 = 0x120c
};


void kb_init();

void kb_clear_buf();
uint16_t kb_get_buf();
void kb_on_keypress(void (*listener)(uint16_t c));


#endif /* __KERNEL_KB_H__ */
