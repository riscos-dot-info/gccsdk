; Demangle C++ symbols using libstc++ '__cxa_demangle' implementation when
; libstc++ is available during link time.
; Copyright (c) 2006 UnixLib Developers

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	IMPORT	|__cxa_demangle|, WEAK

	;char *__unixlib_cxa_demangle(const char *__mangled_name,
	;			char *__output_buffer, size_t *__length,
	;			int *__status);
	EXPORT	|__unixlib_cxa_demangle|
	NAME	__unixlib_cxa_demangele
|__unixlib_cxa_demangle|
	LDR	ip, =|__cxa_demangle|
	TEQ	ip, #0
	MOVNE	PC, ip
	MOV	a1, #-1		; Write -1 = "A memory allocation failure occurred"
	STR	a1, [a4]
	MOV	a1, #0
	MOV	PC, R14

	END