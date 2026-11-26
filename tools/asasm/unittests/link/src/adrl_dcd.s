; ADRL and "DCD sym - label" against a symbol defined in another object.
; ELF only goes wrong once the linker has run, hence a link test.
; Linked at 0x8000, ext is at 0x8fac:
;   go   ADRL gives r2 = 0x8fac       (fa2e8fe2 042082e2)
;   lab  holds ext - lab = 0x00000fa4 (a40f0000)
	AREA	|Asm$$Code|, CODE, READONLY
	IMPORT	ext
	EXPORT	go

go	ADRL	r2, ext
lab	DCD	ext - lab

	END
