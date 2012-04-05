;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/sys/_sdiv10.s,v $
; $Date: 2003/04/05 23:56:08 $
; $Revision: 1.5 $
; $State: Exp $
; $Author: joty $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	EXPORT	|_kernel_sdiv10|
	EXPORT	|__rt_sdiv10|
	NAME	__rt_sdiv10
|_kernel_sdiv10|
|__rt_sdiv10|
	MOVS	a4, a1
	RSBMI	a1, a1, #0
	SUB	a2, a1, #10		; Keep a1 - 10 for later
	SUB	a1, a1, a1, LSR #2
	ADD	a1, a1, a1, LSR #4
	ADD	a1, a1, a1, LSR #8
	ADD	a1, a1, a1, LSR #16
	MOV	a1, a1, LSR #3		; a1 = a1 (on entry) / 10
	ADD	a3, a1, a1, LSL #2
	SUBS	a2, a2, a3, LSL #1	; Calculate a1/10 - (a1/10)*10
	ADDPL	a1, a1, #1		; Fix quotient
	ADDMI	a2, a2, #10		; Fix remainder
	CMP	a4, #0			; Negative sign ?
	RSBMI	a1, a1, #0
	RSBMI	a2, a2, #0
	MOV	pc, lr

	END