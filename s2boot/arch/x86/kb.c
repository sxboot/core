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
 * kb.c - Keyboard driver for x86.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <kernel/kb.h>
#include <x86/pic.h>
#include <x86/x86.h>
#include <x86/kb.h>

static bool kb_shift, kb_alt, kb_crtl;
static bool kb_numlock, kb_capslock, kb_scrolllock;

static uint16_t kb_buf = 0;
static void (*kb_on_key)(uint16_t c) = NULL;

static uint16_t kb_scanc0_set[] = {
	KEY_ESCAPE,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_MINUS,
	KEY_EQUALS,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_R,
	KEY_T,
	KEY_Y,
	KEY_U,
	KEY_I,
	KEY_O,
	KEY_P,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_ENTER,
	KEY_LEFTCONTROL,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_SEMICOLON,
	KEY_QUOTE,
	KEY_GRAVE,
	KEY_LEFTSHIFT,
	KEY_BACKSLASH,
	KEY_Z,
	KEY_X,
	KEY_C,
	KEY_V,
	KEY_B,
	KEY_N,
	KEY_M,
	KEY_COMMA,
	KEY_DOT,
	KEY_SLASH,
	KEY_RIGHTSHIFT,
	KEY_P_ASTERISK,
	KEY_LEFTALT,
	KEY_SPACE,
	KEY_CAPSLOCK,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_NUMBERLOCK,
	KEY_SCROLLLOCK,
	KEY_P_7,
	KEY_P_8,
	KEY_P_9,
	KEY_P_MINUS,
	KEY_P_4,
	KEY_P_5,
	KEY_P_6,
	KEY_P_PLUS,
	KEY_P_1,
	KEY_P_2,
	KEY_P_3,
	KEY_P_0,
	KEY_P_DOT,
	KEY_UNKNOWN,
	KEY_UNKNOWN,
	KEY_UNKNOWN,
	KEY_F11,
	KEY_F12
};

status_t kb_init(){
	reloc_ptr((void**) &kb_on_key);
	return 0;
}

void kb_clear_buf(){
	kb_buf = 0;
}

uint16_t kb_get_buf(){
	return kb_buf;
}

void kb_on_keypress(void (*listener)(uint16_t c)){
	kb_on_key = listener;
}


void kb_keypress(uint8_t scanCode){
	uint16_t asciic = kb_scanc0_set[scanCode - 1];

	if(scanCode < 0x59){ // below are presses, above are released
		switch(asciic){
			case KEY_RIGHTSHIFT:case KEY_LEFTSHIFT:
			kb_shift = TRUE;
			break;

			case KEY_LEFTALT:
			kb_alt = TRUE;
			break;

			case KEY_LEFTCONTROL:
			kb_crtl = TRUE;
			break;

			case KEY_NUMBERLOCK:
			kb_numlock = !kb_numlock;
			break;

			case KEY_CAPSLOCK:
			kb_capslock = !kb_capslock;
			break;

			case KEY_SCROLLLOCK:
			kb_scrolllock = !kb_scrolllock;
			break;

			case KEY_F1:case KEY_F2:case KEY_F3:case KEY_F4:case KEY_F5:case KEY_F6:
			case KEY_F7:case KEY_F8:case KEY_F9:case KEY_F10:case KEY_F11:case KEY_F12:
			break;

			default: {
				uint8_t asciif = asciic;
				if((kb_shift || kb_capslock) && !(kb_shift && kb_capslock) && ((asciic > 48 && asciic < 58) || (asciic > 96 && asciic < 123))){
					asciif -= (asciic < 64) ? 16 : 32;
				}
				kb_buf = asciif;
				if(kb_on_key != NULL){
					kb_on_key(asciif);
					kb_clear_buf();
				}
				break;
			}
		}
	} else {
		switch(scanCode){
			case 0x9d:
			kb_crtl = FALSE;
			break;

			case 0xaa:case 0xb6:
			kb_shift = FALSE;
			break;

			case 0xb8:
			kb_alt = FALSE;
			break;

			default:
			break;
		}
	}
}

void __attribute__ ((interrupt)) kb_int(idt_interrupt_frame* frame){
	pic_hw_int();
	kb_keypress(x86_inb(KB_ENC_PORT));
	pic_send_eoi(0);
}
