;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/sys/_umodsi3.s,v $
; $Date: 2001/01/29 15:10:21 $
; $Revision: 1.2 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

; GCC compatability file

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	IMPORT	|_kernel_udiv|

	EXPORT	|__umodsi3|
	NAME	__umodsi3
|__umodsi3|
	STMFD	sp!, {lr}
	MOV	a3, a1
	MOV	a1, a2
	MOV	a2, a3
	BL	|_kernel_udiv|
	MOV	a1, a2
	stackreturn	AL, "pc"

	END