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

bits 16
org 0x1000

; MEMORY MAP
;        0x1000+   s1boot
;0x7800 - 0x7c00   stack
;0x7c00 - 0x7dff   s0boot (mbr/vbr for mbr/chain loading)
;0x7e00 - 0x7fff   sGboot
;0x8000 - 0x8fff   temp/FAT entries for loading s2boot
;0x00009000 - 0x0003ffff   s2boot + bdd.so unparsed
;0x00040000 - 0x0006ffff   s2boot
;0x00070000 - 0x00072fff   bios memory map
;0x00073000 - 0x0007ffff   s2boot map
;0x00076000 - 0x00079fff   page mappings

;------------------------------------------------------------
;     MAIN STARTUP AREA
;------------------------------------------------------------

%include "asm/x86/s1boot_common.asm"


;------------------------------------------------------------
;     BOOT TO 32
;------------------------------------------------------------

bits 16

gdt32_start:
	;null
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	
	;code
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10011010b
	db 11001111b
	db 00000000b
	
	;data
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10010010b
	db 11001111b
	db 00000000b
gdt32_end:

gdt32:
	dw gdt32_end - gdt32_start - 1
	dq gdt32_start

idt32_start:
	; div by 0
	dw unknownException32
	dd 0x0e000008
	dw 0

	; ?
	dw unknownException32
	dd 0x0e000008
	dw 0
	
	; nmi
	dw unknownException32
	dd 0x0e000008
	dw 0
	
	; breakpoint
	dw unknownException32
	dd 0x0e000008
	dw 0

	; overflow
	dw unknownException32
	dd 0x0e000008
	dw 0

	; bounds range exception
	dw unknownException32
	dd 0x0e000008
	dw 0

	; inv opcode
	dw invalidOpcode32
	dd 0x8e000008
	dw 0

	; device not available
	dw unknownException32
	dd 0x0e000008
	dw 0

	; double fault
	dw doubleFault32
	dd 0x8e000008
	dw 0

	; coprocessor seg overrun
	dw unknownException32
	dd 0x0e000008
	dw 0

	; invalid tss
	dw unknownException32
	dd 0x0e000008
	dw 0

	; segment not present
	dw unknownException32
	dd 0x0e000008
	dw 0

	; stack-segment fault
	dw unknownException32
	dd 0x0e000008
	dw 0

	; general protection fault
	dw generalPF32
	dd 0x8e000008
	dw 0

	; page fault
	dw pageFault32
	dd 0x8e000008
	dw 0

	; ?2
	dw unknownException32
	dd 0x0e000008
	dw 0

	; x87 fpu error
	dw unknownException32
	dd 0x0e000008
	dw 0

	; alignment check
	dw unknownException32
	dd 0x0e000008
	dw 0

	; machine check
	dw unknownException32
	dd 0x0e000008
	dw 0

	; simd fp exception
	dw unknownException32
	dd 0x0e000008
	dw 0

	times 256 - ($ - idt32_start) db 0
idt32_end:

idt32:
	dw idt32_end - idt32_start - 1
	dd idt32_start


bootTo32:

	call	resetExceptionHandlers16

	cli
	lgdt	[gdt32]
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp		0x8:boot32




;------------------------------------------------------------
;     BOOT 32
;------------------------------------------------------------

bits 32

msgELFS2		db	"Parsing ELF ", 0
msgInvELF		db	"Invalid ELF format in s2boot image", 0
msgInvBitS2		db	"s2boot image was compiled for 64bit, expected 32bit", 0
msgInvTpS2		db	"Expected type 2 (executable) or type 3 (dynamic) for s2boot image", 0
msgInvISS2		db	"s2boot image was not compiled for x86", 0
msgNumSkipSeg1	db	"Skipped ", 0
msgNumSkipSeg2	db	" invalid segments", 0
msgNumSeg1		db	"Loaded ", 0
msgNumSeg2		db	" segments", 0
msgNoSeg		db	"Loading failed", 0
msgS2			db	"Calling s2boot", 0
msgS2Return		db	"s2boot returned: ", 0

s2bootDynSeg	dd	0
s2bootDynSegLen	dd	0
s2bootRelaBase	dd	0
s2bootRelaLen	dd	0
skipSegments	db	0
numSegments		db	0
s2bootEntry		dd	0

dec_out			db	"0000000000000000000000", 0
hex_out			db	"0x00000000 ", 0

msgErrCP		db	"err=", 0
msgErrCP2		db	"eip=", 0
errCode			dd	0
errEIP			dd	0


print32: ;pointer in esi, attributes in dh
	pusha
	.l:
	lodsb
	cmp		al, 0
	je		.c
	mov		dl, al
	call	printChar32
	jmp		.l
	.c:
	jmp		complete32

printp32:
	pusha
	push	esi
	mov		esi, msgPre
	call	print32
	pop		esi
	call	print32
	jmp		complete32

println32:
	pusha
	push	esi
	mov		esi, msgPre
	call	print32
	pop		esi
	call	print32
	call	printNln32
	jmp		complete32

printChar32: ;data in dx dl-char dh-attr
	pusha
	
	mov		edi, 0xb8000
	xor		eax, eax
	xor		ecx, ecx
	mov		al, byte[cu_x]
	mov		cl, 2
	mul		cl
	add		edi, eax
	mov		al, byte[cu_y]
	mov		cl, 160
	mul		cl
	add		edi, eax

	cmp		dl, 0xa
	je		.c

	mov		word[edi], dx
	inc		byte[cu_x]
	cmp		byte[cu_x], 80
	je		.c
	jmp		complete32
	.c:
	call	printNln32
	jmp		complete32

printNln32:
	pusha
	mov		byte[cu_x], 0
	inc		byte[cu_y]
	cmp		byte[cu_y], 25
	je		.c
	jmp		complete32
	.c:
	call	shiftUp32
	jmp		complete32

shiftUp32:
	pusha
	dec		byte[cu_y]
	mov		edi, 0xb8000
	mov		cx, 960
	.l:
	mov		edx, dword[edi+160]
	mov		dword[edi], edx
	add		edi, 4
	loop	.l
	mov		cx, 40
	.l2:
	mov		dword[edi], 0x07200720
	add		edi, 4
	loop	.l2
	jmp		complete32

updateCursor32:
	pusha
	xor		eax, eax
	mov		al, byte[cu_y]
	mov		cl, 80
	mul		cl
	add		al, byte[cu_x]
	mov		ebx, eax

	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bh
	out		dx, al

	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bl
	out		dx, al

	popa
	ret

updateCursorMemData32:
	pusha
	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	inc		dx
	in		al, dx
	mov		bh, al
	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	inc		dx
	in		al, dx
	mov		bl, al
	mov		ax, bx
	mov		cx, 80
	xor		dx, dx
	div		cx
	mov		BYTE[cu_x], dl
	mov		BYTE[cu_y], al
	popa
	ret

updateLoadingWheel32:
	pusha

	mov		edi, 0xb8000
	xor		eax, eax
	xor		ecx, ecx
	mov		al, BYTE[cu_x]
	mov		cl, 2
	mul		cl
	add		edi, eax
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	add		edi, eax

	mov		dx, WORD[edi]
	cmp		dl, '-'
	je		.minus
	cmp		dl, '/'
	je		.slash
	cmp		dl, 0x5c
	je		.back
	cmp		dl, '|'
	je		.line
	mov		dl, '-'
	mov		WORD[edi], dx
	jmp		complete32
	.minus:
	mov		dl, 0x5c
	jmp		.c
	.slash:
	mov		dl, '-'
	jmp		.c
	.back:
	mov		dl, '|'
	jmp		.c
	.line:
	mov		dl, '/'
	jmp		.c
	.c:
	mov		WORD[edi], dx
	jmp		complete32

complete32:
	call	updateCursor32
	popa
	ret

printDec32: ; number in edx
	pusha
	mov		edi, dec_out + 21
	.reset:
	mov		BYTE[edi], 0x30
	dec		edi
	cmp		edi, dec_out - 1
	jne		.reset
	mov		edi, dec_out + 21
	mov		ebx, 10
	mov		eax, edx
	.loop:
	xor		edx, edx
	div		ebx
	add		dl, 48
	mov		BYTE[edi], dl
	cmp		eax, 0
	je		.print
	dec		edi
	jmp		.loop
	.print:
	mov		esi, dec_out - 1
	.cut:
	inc		esi
	cmp		BYTE[esi], 0x30
	je		.cut
	cmp		esi, dec_out + 22
	jne		.p
	mov		esi, dec_out + 21
	.p:
	mov		dh, 00000101b
	call	print32
	popa
	ret

printHex32:
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi

	mov		ecx, 8
	.char_loop:
	mov		eax, edx
	shr		edx, 4
	and		eax, 0xf

	mov		ebx, hex_out
	add		ebx, 1
	add		ebx, ecx

	mov		BYTE[ebx], 0x30

	cmp		ax, 0xa
	jl		.set_letter
	add		byte[ebx], 7
	jl		.set_letter
	.set_letter:
	add		byte[ebx],al 
	loop	.char_loop
	jmp		.print_hex_done
	.print_hex_done:
	mov		ecx, 8
	mov		esi, hex_out
	add		esi, 2
	.cmp:
	cmp		BYTE[esi], 0x30
	jne		.p
	inc		esi
	loop	.cmp
	;jmp		.ret
	.p:
	mov		esi, hex_out
	mov		dh, 00001101b
	call	print32
	.ret:
	pop		esi
	pop		edx
	pop		ecx
	pop		ebx
	pop		eax
	ret

invalidOpcode32:
	pop		DWORD[errEIP]
	mov		esi, msgInvOp
	jmp		exception32

doubleFault32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgDoubleF
	jmp		exception32

generalPF32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgGPF
	jmp		exception32

pageFault32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgPF
	jmp		exception32

unknownException32: ; this is unused
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgUnExcept
	jmp		exception32

exception32:
	call	updateCursorMemData32
	mov		dh, 0x4
	call	println32
	call	printErrC32
	mov		dh, 0x4
	mov		esi, msgExcept
	call	println32
	cli
	hlt

printErrC32:
	mov		dh, 0xf
	mov		esi, msgErrCP
	call	printp32
	mov		edx, DWORD[errCode]
	call	printHex32
	mov		dh, 0xf
	mov		esi, msgErrCP2
	call	print32
	mov		edx, DWORD[errEIP]
	call	printHex32
	call	printNln32
	ret

parseELF32:
	call	updateLoadingWheel32
	cmp		DWORD[s2bootFileLoad], 0x464c457f
	jne		.invalid_format
	cmp		BYTE[s2bootFileLoad + 0x04], 0x01
	jne		.invalid_bit
	cmp		WORD[s2bootFileLoad + 0x05], 0x0101
	jne		.invalid_format
	cmp		WORD[s2bootFileLoad + 0x10], 0x0002
	je		.valid_type
	cmp		WORD[s2bootFileLoad + 0x10], 0x0003
	je		.valid_type
	jmp		.invalid_type
	.valid_type:
	cmp		WORD[s2bootFileLoad + 0x12], 0x0003
	jne		.invalid_instruction_set
	call	updateLoadingWheel32
	mov		DWORD[s2mapStart], s2bootMapLocation
	mov		DWORD[s2bootBase], s2bootStartVirt
	movzx	ecx, WORD[s2bootFileLoad + 0x2c] ; number of ph table entries
	mov		ebx, s2bootFileLoad
	add		ebx, DWORD[s2bootFileLoad + 0x1c] ; start of ph table
	.parse_program_header:
	call	updateLoadingWheel32
	push	ebx
	push	ecx
	cmp		DWORD[ebx], 1 ; loadable segment
	jne		.check_dyn
	mov		eax, DWORD[ebx + 0x08] ; target address
	add		eax, DWORD[ebx + 0x14] ; size in memory
	cmp		eax, s2bootMaxSize ; check if target address + size is below maximum size
	jb		.segm_c_2
	inc		BYTE[skipSegments]
	jmp		.parse_program_header_f
	.segm_c_2:
	; add s2map entry
	mov		eax, 8
	mul		WORD[s2mapLength]
	add		eax, DWORD[s2mapStart]
	mov		edi, eax
	mov		edx, DWORD[ebx + 0x08] ; target address
	add		edx, s2bootStartVirt
	mov		DWORD[edi], edx
	add		edx, DWORD[ebx + 0x14] ; size in memory
	mov		DWORD[edi + 4], edx
	inc		WORD[s2mapLength]

	mov		ecx, DWORD[ebx + 0x10] ; size in file
	mov		edi, DWORD[ebx + 0x08] ; target address
	add		edi, s2bootStartVirt
	mov		esi, DWORD[ebx + 0x04] ; offset in file
	add		esi, s2bootFileLoad
	cld
	.copy_segm:
	call	updateLoadingWheel32
	movsb
	loop	.copy_segm
	inc		BYTE[numSegments]
	jmp		.parse_program_header_f
	.check_dyn:
	cmp		DWORD[ebx], 2 ; dynamic linker segment
	jne		.parse_program_header_f
	mov		edx, DWORD[ebx + 0x04] ; offset in file
	mov		DWORD[s2bootDynSeg], edx
	mov		edx, DWORD[ebx + 0x10] ; size in file
	mov		DWORD[s2bootDynSegLen], edx
	.parse_program_header_f:
	pop		ecx
	pop		ebx
	movzx	edx, WORD[s2bootFileLoad + 0x2a] ; size of ph entry
	add		ebx, edx
	dec		ecx
	jnz		.parse_program_header

	call	updateLoadingWheel32
	mov		edx, DWORD[s2bootFileLoad + 0x18] ; entry point address
	add		edx, s2bootStartVirt
	mov		DWORD[s2bootEntry], edx

	cmp		DWORD[s2bootDynSeg], 0
	je		.link_f
	; dynamic link
	call	updateLoadingWheel32
	mov		eax, DWORD[s2bootDynSegLen]
	xor		edx, edx
	mov		edi, 8
	div		edi ; length of entry
	mov		ecx, eax
	mov		ebx, DWORD[s2bootDynSeg]
	add		ebx, s2bootFileLoad
	.parseDynSec:
	call	updateLoadingWheel32
	cmp		DWORD[ebx], 17 ; rel section
	je		.parseDynSec_relaBase
	cmp		DWORD[ebx], 18 ; rel section length
	je		.parseDynSec_relaLen
	jmp		.parseDynSec_f
	.parseDynSec_relaBase:
	mov		edx, DWORD[ebx + 4]
	mov		DWORD[s2bootRelaBase], edx
	jmp		.parseDynSec_f
	.parseDynSec_relaLen:
	mov		edx, DWORD[ebx + 4]
	mov		DWORD[s2bootRelaLen], edx
	jmp		.parseDynSec_f
	.parseDynSec_f:
	add		ebx, 8
	loop	.parseDynSec

	cmp		DWORD[s2bootRelaBase], 0
	je		.link_f
	call	updateLoadingWheel32
	mov		eax, DWORD[s2bootRelaLen]
	xor		edx, edx
	mov		edi, 8
	div		edi ; length of entry in rela section
	mov		ecx, eax
	mov		ebx, DWORD[s2bootRelaBase]
	add		ebx, s2bootStartVirt
	.setRela:
	xor		edx, edx
	mov		edx, DWORD[ebx + 4]
	cmp		edx, 8 ; type DYNL_R_386_RELATIVE (base + addend)
	jne		.setRela_f
	mov		edi, DWORD[ebx] ; addend address
	add		edi, s2bootStartVirt
	mov		edx, DWORD[edi] ; addend
	add		edx, s2bootStartVirt
	mov		DWORD[edi], edx
	.setRela_f:
	add		ebx, 8
	loop	.setRela

	.link_f:

	call	printNln32
	cmp		BYTE[skipSegments], 0
	jbe		.print_loaded_segs
	mov		dh, 0xe
	mov		esi, msgNumSkipSeg1
	call	printp32
	movzx	edx, BYTE[skipSegments]
	call	printDec32
	mov		dh, 0xe
	mov		esi, msgNumSkipSeg2
	call	print32
	call	printNln32
	.print_loaded_segs:
	mov		dh, 0x7
	mov		esi, msgNumSeg1
	call	printp32
	movzx	edx, BYTE[numSegments]
	call	printDec32
	mov		dh, 0x7
	mov		esi, msgNumSeg2
	call	print32
	call	printNln32
	cmp		BYTE[numSegments], 1
	jb		.no_segments ; 0 loaded segments, something definitely went wrong
	clc
	ret
	.invalid_format:
	mov		esi, msgInvELF
	jmp		.parse_error
	.invalid_bit:
	mov		esi, msgInvBitS2
	jmp		.parse_error
	.invalid_type:
	mov		esi, msgInvTpS2
	jmp		.parse_error
	.invalid_instruction_set:
	mov		esi, msgInvISS2
	jmp		.parse_error
	.no_segments:
	mov		esi, msgNoSeg
	jmp		.parse_error
	.parse_error:
	call	printNln32
	mov		dh, 0xc
	call	println32
	stc
	ret

; map (phys) 0x0000000000000000 - 0x0000000000400000 to (virt) 0x0000000000000000
; 0x00076000 - 0x00077000: page directory table
; 0x00077000 - 0x00078000: page table for first mapping

%define pageTablesBase 0x76000

setupPaging:
	push	eax
	push	ecx
	push	edx
	push	edi
	; page directory table
	mov		DWORD[pageTablesBase], pageTablesBase + 0x1003 ; phys address 0x1000, set present (0x1) and r/w (0x2) bit

	; page tables
	mov		ecx, 1024
	mov		edi, pageTablesBase + 0x1000
	mov		edx, 0x3
	.writePT:
	mov		DWORD[edi], edx
	add		edi, 4
	add		edx, 0x1000
	loop	.writePT

	mov		eax, pageTablesBase
	mov		cr3, eax

	mov		eax, cr0
	or		eax, 0x80000001
	mov		cr0, eax
	pop		edi
	pop		edx
	pop		ecx
	pop		eax
	ret



boot32:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		esp, 0x7c00

	lidt	[idt32]

	call	setupPaging

	mov		dh, 0x7
	mov		esi, msgELFS2
	call	printp32
	call	parseELF32
	jnc		.c1
	cli
	hlt

	.c1:

	mov		DWORD[membase], 0

	mov		dh, 0x7
	mov		esi, msgS2
	call	println32

	push	header_start
	mov		ebp, 0
	call	DWORD[s2bootEntry]

	.halt:
	cli
	hlt
	jmp		.halt




; SERVICE CALLBACK

servMemBuf	dd	0

reqBufStruct	dd	0
; videomode
reqWidth	dd	0
reqHeight	dd	0
reqBPP		dd	0
reqMode		dd	0
reqResFramebuffer	dd	0
reqResBytesPerLine	dd	0
curVidMode	dw	0
; read drive
reqDIndex	dd	0
reqSectorCount	dd	0
reqDest		dd	0
reqLBA		dq	0
; drive info
reqSecCountDest	dd	0
reqSecSizeDest	dd	0

dInfoTemp:
dInfoBufSize	dw	0x1e
dInfoInfoFlags	dw	0
dInfoCylinders	dd	0
dInfoHeads		dd	0
dInfoSecsPTrack	dd	0
dInfoSectors	dq	0
dInfoBytesPSec	dw	0
dInfoPEDD		dd	0

gdt32temp:
	dw 0
	dd 0
idt32temp:
	dw 0
	dd 0
stack32temp	dd	0

retCode		dd	0

gdt16_start:
	;null
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	
	;code
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10011010b
	db 00001111b
	db 00000000b
	
	;data
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10010010b
	db 00001111b
	db 00000000b
gdt16_end:

gdt16:
	dw gdt16_end - gdt16_start - 1
	dq gdt16_start

idt16:
	dw 0x3ff
	dd 0

srvModeSwitchAddr	dw	0

firmwareDriveName	db	"bios", 0

; services: (<di> - <description> (<up to 3 passed arguments>))
;  1 - upstream pre init (address to kmalloc)
;  2 - upstream init (<no args>)
;  10 - set video to closest (modeInfoStruct)
serviceCallback: ; di, si, dx, cx
	push	ebp
	mov		ebp, esp
	push	edi
	push	esi
	mov		edi, DWORD[ebp + 8]
	mov		esi, DWORD[ebp + 12]
	mov		edx, DWORD[ebp + 16]
	mov		ecx, DWORD[ebp + 20]
	mov		eax, 0
	cmp		edi, 1
	je		.uPreInit
	cmp		edi, 2
	je		.uInit
	cmp		edi, 10
	je		.setVideo
	cmp		edi, 20
	je		.firmwareDriveGetName
	cmp		edi, 21
	je		.firmwareDriveRead
	cmp		edi, 22
	je		.firmwareDriveInfo
	mov		eax, 1
	jmp		.end

	.uPreInit:
	push	0x1000
	call	esi
	add		esp, 4
	mov		DWORD[servMemBuf], eax
	cmp		DWORD[servMemBuf], 0
	je		.uPreInit_err
	cmp		DWORD[servMemBuf], 0xffff
	ja		.uPreInit_err
	mov		eax, 0
	jmp		.end
	.uPreInit_err:
	mov		eax, 27 ; TSX_MEMORY_RESERVED
	jmp		.end

	.uInit:
	jmp		.end

	.setVideo:
	mov		DWORD[reqBufStruct], esi
	mov		edx, DWORD[esi]
	mov		DWORD[reqWidth], edx
	mov		edx, DWORD[esi + 4]
	mov		DWORD[reqHeight], edx
	mov		edx, DWORD[esi + 8]
	mov		DWORD[reqBPP], edx
	mov		edx, DWORD[esi + 12]
	mov		DWORD[reqMode], edx


	mov		WORD[srvModeSwitchAddr], .vbe_to16ret
	jmp		to16
	.vbe_to16ret:
	bits 16


	cmp		BYTE[reqMode], 1
	je		.nonStandardMode
	cmp		WORD[reqWidth], 80
	jne		.nonStandardMode
	cmp		WORD[reqHeight], 25
	jne		.nonStandardMode
	cmp		WORD[reqBPP], 16
	jne		.nonStandardMode
	; mode 3, text 80x25
	mov		DWORD[reqResFramebuffer], 0xb8000
	mov		WORD[reqResBytesPerLine], 160
	mov		ax, 0x0003 ; ah = 0 set video mode, al = 3 video mode number
	int		0x10
	jc		.vbe_err
	jmp		.vbe_done

	.nonStandardMode:
	mov		ax, 0x4f00
	mov		di, WORD[servMemBuf]
	int		0x10
	cmp		ax, 0x4f ; success
	jne		.vbe_err
	mov		bx, WORD[servMemBuf]
	add		bx, 0xe ; pointer to modes
	mov		es, WORD[bx + 2] ; segment
	mov		bx, WORD[bx]
	.checkModes:
	mov		dx, WORD[es:bx] ; dx = video mode num
	mov		WORD[curVidMode], dx
	cmp		dx, 0xffff
	je		.modeCheckDone
	push	bx
	push	es
	mov		cx, dx
	mov		ax, 0
	mov		es, ax
	mov		ax, 0x4f01
	mov		di, WORD[servMemBuf]
	add		di, 512 ; place this structure behind the controller information (which is 512 bytes)
	push	di
	int		0x10
	pop		di
	cmp		ax, 0x4f ; success
	jne		.checkModes_f
	; --- check ---
	; attributes
	mov		ax, 0x81 ; supported, linear frame buffer
	cmp		BYTE[reqMode], 1 ; 1 = graphics mode
	jne		.notGraphics
	or		ax, 0x10 ; graphics mode
	.notGraphics:
	mov		dx, WORD[di]
	and		dx, ax
	cmp		dx, ax

	jne		.checkModes_f
	; x res
	mov		dx, WORD[reqWidth]
	cmp		dx, WORD[di + 0x12]
	jne		.checkModes_f
	; y res
	mov		dx, WORD[reqHeight]
	cmp		dx, WORD[di + 0x14]
	jne		.checkModes_f
	; bpp
	mov		dl, BYTE[reqBPP]
	cmp		dl, BYTE[di + 0x19]
	jne		.checkModes_f
	; number of banks (must be 1 because framebuffer must be continuous block in memory)
	cmp		BYTE[di + 0x1a], 1
	jne		.checkModes_f
	; memory model (4 or 6)
	cmp		BYTE[di + 0x1b], 4
	je		.checkMemModel_f
	cmp		BYTE[di + 0x1b], 6
	je		.checkMemModel_f
	jmp		.checkModes_f
	.checkMemModel_f:
	; framebuffer address
	mov		edx, DWORD[di + 0x28]
	cmp		edx, 0
	je		.checkModes_f
	mov		DWORD[reqResFramebuffer], edx
	; bytes per scan line
	mov		dx, WORD[di + 0x10]
	mov		WORD[reqResBytesPerLine], dx
	pop		es
	pop		bx
	jmp		.modeCheckDone
	.checkModes_f:
	pop		es
	pop		bx
	add		bx, 2
	jmp		.checkModes

	.modeCheckDone:
	mov		edx, DWORD[reqResFramebuffer]
	cmp		edx, 0
	je		.vbe_err ; mode not found

	; set mode
	mov		ax, 0
	mov		es, ax
	mov		ax, 0x4f02
	mov		di, WORD[servMemBuf]
	add		di, 512 ; place this structure behind the controller information (which is 512 bytes)
	mov		bx, WORD[curVidMode]
	and		bx, 0x1ff
	or		bx, 0x4000 ; enable linear frame buffer
	int		0x10
	cmp		ax, 0x4f ; success
	jne		.vbe_err

	mov		WORD[retCode], 0
	jmp		.vbe_done
	.vbe_err:
	mov		WORD[retCode], 1
	.vbe_done:


	mov		WORD[srvModeSwitchAddr], .vbe_from16ret
	jmp		from16
	.vbe_from16ret:
	bits 32


	mov		esi, DWORD[reqBufStruct]
	mov		edx, DWORD[reqResFramebuffer]
	mov		DWORD[esi + 16], edx ; write framebuffer address to struct
	mov		edx, DWORD[reqResBytesPerLine]
	mov		DWORD[esi + 20], edx ; write bytes per line to struct

	mov		eax, DWORD[retCode]
	jmp		.end


	.firmwareDriveGetName:
	mov		DWORD[esi], firmwareDriveName
	mov		eax, 0
	jmp		.end


	.firmwareDriveRead:
	mov		DWORD[reqBufStruct], esi
	mov		edx, DWORD[esi]
	mov		DWORD[reqDIndex], edx
	mov		edx, DWORD[esi + 4]
	mov		DWORD[reqSectorCount], edx
	mov		edx, DWORD[esi + 8]
	mov		DWORD[reqDest], edx
	mov		edx, DWORD[esi + 12]
	mov		DWORD[reqLBA], edx
	mov		edx, DWORD[esi + 16]
	mov		DWORD[reqLBA + 4], edx

	cmp		DWORD[reqDIndex], 0x7f
	jbe		.read_validD
	mov		DWORD[retCode], 13 ; TSX_NO_DEVICE
	jmp		.read_end
	.read_validD:

	mov		ecx, DWORD[reqSectorCount]
	.readLoop:
	push	ecx


	mov		WORD[srvModeSwitchAddr], .read_to16ret
	jmp		to16
	.read_to16ret:
	bits 16


	mov		edx, DWORD[reqLBA]
	mov		DWORD[dap_start_l], edx
	mov		edx, DWORD[reqLBA + 4]
	mov		DWORD[dap_start_h], edx

	mov		WORD[dap_count], 1

	mov		bx, WORD[servMemBuf]
	mov		WORD[dap_buf_off], bx
	mov		WORD[dap_buf_seg], 0

	mov		ah, 0x42
	mov		dl, BYTE[reqDIndex]
	add		dl, 0x80
	mov		si, dap_start
	int		0x13
	jc		.read_err

	mov		WORD[retCode], 0
	jmp		.read_done
	.read_err:
	mov		WORD[retCode], 19 ; TSX_IO_ERROR
	.read_done:


	mov		WORD[srvModeSwitchAddr], .read_from16ret
	jmp		from16
	.read_from16ret:
	bits 32


	pop		ecx
	cmp		WORD[retCode], 0
	jne		.read_end
	cmp		DWORD[reqLBA], 0xffffffff
	jne		.read_no_overflow
	inc		DWORD[reqLBA + 4]
	.read_no_overflow:
	inc		DWORD[reqLBA]
	push	ecx
	mov		edi, DWORD[reqDest]
	mov		esi, DWORD[servMemBuf]
	mov		ecx, 512
	cld
	rep		movsb
	pop		ecx
	add		DWORD[reqDest], 512
	dec		ecx
	jnz		.readLoop


	.read_end:
	mov		eax, DWORD[retCode]
	jmp		.end


	.firmwareDriveInfo:
	mov		DWORD[reqDIndex], esi
	mov		DWORD[reqSecCountDest], edx
	mov		DWORD[reqSecSizeDest], ecx

	cmp		DWORD[reqDIndex], 0x7f
	jbe		.driveInfo_validD
	mov		DWORD[retCode], 13 ; TSX_NO_DEVICE
	jmp		.driveInfo_end
	.driveInfo_validD:


	mov		WORD[srvModeSwitchAddr], .driveInfo_to16ret
	jmp		to16
	.driveInfo_to16ret:
	bits 16


	mov		ah, 0x48
	mov		dl, BYTE[reqDIndex]
	add		dl, 0x80
	mov		si, dInfoTemp
	int		0x13
	jc		.driveInfo_err

	mov		WORD[retCode], 0
	jmp		.driveInfo_done
	.driveInfo_err:
	mov		WORD[retCode], 19 ; TSX_IO_ERROR
	.driveInfo_done:


	mov		WORD[srvModeSwitchAddr], .driveInfo_from16ret
	jmp		from16
	.driveInfo_from16ret:
	bits 32


	cmp		DWORD[retCode], 0
	jne		.driveInfo_err2
	mov		edi, DWORD[reqSecCountDest]
	mov		edx, DWORD[dInfoSectors]
	mov		DWORD[edi], edx
	mov		edx, DWORD[dInfoSectors + 4]
	mov		DWORD[edi + 4], edx
	mov		edi, DWORD[reqSecSizeDest]
	movzx	edx, WORD[dInfoBytesPSec]
	mov		DWORD[edi], edx
	.driveInfo_err2:


	.driveInfo_end:
	mov		eax, DWORD[retCode]
	jmp		.end


	.end:
	pop		esi
	pop		edi
	pop		ebp
	ret


to16:
	bits	32
	cli
	sgdt	[gdt32temp]
	sidt	[idt32temp]
	mov		DWORD[stack32temp], esp
	mov		esp, 0x7b00 ; set this in case the stack got relocated somewhere above 0xffff

	lgdt	[gdt16]
	jmp		0x8:.16bitprot
	bits	16
	.16bitprot:
	mov		eax, 0x10
	mov		ds, eax
	mov		es, eax
	mov		fs, eax
	mov		gs, eax
	mov		ss, eax

	mov		eax, cr0
	and		eax, 0x7ffffffe
	mov		cr0, eax

	jmp		0x0:.16bitreal
	.16bitreal:
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	lidt	[idt16]

	mov		ax, WORD[srvModeSwitchAddr]
	jmp		ax

from16:
	bits	16
	lgdt	[gdt32temp]
	mov		eax, cr0
	or		eax, 0x80000001
	mov		cr0, eax

	jmp		0x8:.32bitprot
	bits	32
	.32bitprot:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	mov		esp, DWORD[stack32temp]
	lidt	[idt32temp]

	movzx	eax, WORD[srvModeSwitchAddr]
	jmp		eax









align 32, db 0xcc

s1bootEndAddr:
