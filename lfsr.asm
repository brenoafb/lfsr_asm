	;; file: lfsr.asm
	;; To create executable:
	;; nasm -f elf lfsr.asm
	;; gcc -m32 -o lfsr lfsr.o driver.c asm_io.o

	%include "asm_io.inc"

	segment	.data
	;;
	;; initialized data is put in the data segment here
	;;

	seed dd 0x00cafeba

	segment	.bss
	;;
	;; uninitialized data is put in the bss segment
	;;
	lfsr	resd	1

	global asm_generate
	segment	.text
asm_generate:
	mov	ecx, 24			; counter
	push	eax
	mov	edx, [eax]		; register
	xor	eax, eax		; clear out eax
loop:
	cmp	ecx, 0		; check if counter is zero
	je	end		; done
	shl	eax, 1			 ; shift to result to left by one

	xor	edi, edi		; bit
	mov	esi, edx		; copy register
	shr	esi, 0
	mov	edi, esi		 ; bit = lfsr >> 0

	mov	esi, edx
	shr	esi, 1
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 1)

	mov	esi, edx
	shr	esi, 2
	xor	edi, esi		; bit = bit ^ (lfsr >> 2)

	mov	esi, edx
	shr	esi, 7
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 7)
	and	edi, 0x1                 ; keep only lsb

	mov	esi, edx		 ; copy register to tmp
	and	esi, 1			 ; isolate lsb
	or	eax, esi		 ; add lsb to result

	shr	edx, 1			 ; shift register right
	shl	edi, 23			 ; shift bit to msb
	or	edx, edi		 ; set msb to bit

	dec	ecx			 ; decrement counter
	jmp	loop
end:    pop	ecx
	mov	[ecx], edx
	ret
