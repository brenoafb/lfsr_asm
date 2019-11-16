	;; file: lfsr.asm
	;; To create executable:
	;; nasm -f elf lfsr.asm
	;; gcc -m32 -o lfsr lfsr.o driver.c asm_io.o

	%include "asm_io.inc"

	segment	.data
	;;
	;; initialized data is put in the data segment here
	;;

	segment	.bss
	;;
	;; uninitialized data is put in the bss segment
	;;
	global asm_generate
	segment	.text
asm_generate:
	mov	eax, [esp + 4]
	mov	esi, eax		; copy register
	shr	esi, 0
	mov	edi, esi		 ; bit = lfsr >> 0

	mov	esi, eax
	shr	esi, 1
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 1)

	mov	esi, eax
	shr	esi, 2
	xor	edi, esi		; bit = bit ^ (lfsr >> 2)

	mov	esi, eax
	shr	esi, 7
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 7)
	and	edi, 0x1                 ; keep only lsb

	shr	eax, 1			 ; shift register right
	shl	edi, 23			 ; shift bit to msb
	or	eax, edi		 ; set msb to bit

	ret
