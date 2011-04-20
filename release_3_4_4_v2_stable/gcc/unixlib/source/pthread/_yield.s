; Yield processor control to another thread
; Copyright (c) 2004, 2005 UnixLib Developers
; Written by Martin Piper and Alex Waugh

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	IMPORT	|__pthread_fatal_error|
	IMPORT	|__pthread_callback|
	IMPORT	|__cbreg|
	IMPORT	|__ul_global|

	EXPORT	|pthread_yield|

;
; pthread_yield
; Called in USR mode by a thread wishing to give up the processor
	NAME	pthread_yield
|pthread_yield|
	; Setup an APCS-32 stack frame so this will appear in a backtrace
	MOV	ip, sp
	STMFD	sp!, {fp, ip, lr, pc}
	SUB	fp, ip, #4

	; If the thread system isn't running then yielding is pointless
	LDR	a2, =|__ul_global|
	LDR	a1, [a2, #GBL_PTH_SYSTEM_RUNNING]
	CMP	a1, #0
	LDMEQDB	fp, {fp, sp, pc}

	; Check that a context switch can take place
	LDR	a1, [a2, #GBL_PTH_WORKSEMAPHORE]
	CMP	a1, #0
	ADRNE	a1, failmessage
	BLNE	|__pthread_fatal_error|

	MRS	a3, CPSR	; Get a USR mode, IRQs enabled CPSR
	; Change to SVC mode, IRQs disabled to ensure the regs don't get
	; overwitten by a real callback
	SWI	XOS_EnterOS
	CHGMODE	a4, SVC_Mode+IFlag
	; Save regs to callback save area
	LDR	a1, =|__cbreg|
	ADD	a1, a1, #15*4
	STMDB	a1, {r4-r14}^
	ADR	a2, callback_return	;USR mode, IRQs enabled if in a 26bit mode
	STR	a2, [a1]
	STR	a3, [a1, #4]	; Save CPSR

	; Branch directly to the context switcher.
	; It will return by loading the registers from __cbreg
	B	__pthread_callback

callback_return
	LDMDB	fp, {fp, sp, pc}

failmessage
	DCB	"pthread_yield called with context switching disabled", 0
	ALIGN

	END