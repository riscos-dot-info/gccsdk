;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/scl/_kgetenv.s,v $
; $Date: 2001/01/29 15:10:20 $
; $Revision: 1.2 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	IMPORT	|__seterr|

	; _kernel_getenv (const char *name, char *buffer, unsigned size)
	EXPORT	|_kernel_getenv|
	NAME	_kernel_getenv
|_kernel_getenv|
	STMFD	sp!, {v1, lr}
	SUB	a3, a3, #1		; leave room for terminator
	MOV	a4, #0
	MOV	v1, #3
	SWI	XOS_ReadVarVal
	MOVVC	a1, #0	; Null terminate the string
	STRVCB	a1, [a2, a3]
	stackreturn	VC, "v1, pc"

	MOV	v1, a1
	BL	|__seterr|
	MOV	a1, v1
	stackreturn	AL, "v1, pc"

	END
