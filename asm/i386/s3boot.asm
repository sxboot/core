;
; Copyright (C) 2020 user94729 (https://omegazero.org/) and contributors
;
; This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
; If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
;
; Covered Software is provided under this License on an "as is" basis, without warranty of any kind,
; either expressed, implied, or statutory, including, without limitation, warranties that the Covered Software
; is free of defects, merchantable, fit for a particular purpose or non-infringing.
; The entire risk as to the quality and performance of the Covered Software is with You.
;

bits 32
org 0x5000

; VERSION 4

jmp main32

%define ENTRY_STATE_SIZE 44
; entry state
s3entry_a	dd	0
s3entry_b	dd	0
s3entry_c	dd	0
s3entry_d	dd	0
s3entry_sp	dd	0
s3entry_bp	dd	0
s3entry_si	dd	0
s3entry_di	dd	0
s3entry_gdtb	dd	0
s3entry_gdtl	dd	0
s3entry_cs	dw	0
s3entry_ds	dw	0


main32:
	pop		DWORD[s3dataLoc]
	cli

	lgdt	[gdt]
	lidt	[idt]

	;disable paging
	mov		eax, cr0
	and		eax, 0x7fffffff
	mov		cr0, eax

	jmp		s3boot_common_main


%include "asm/x86/s3boot_common.asm"
