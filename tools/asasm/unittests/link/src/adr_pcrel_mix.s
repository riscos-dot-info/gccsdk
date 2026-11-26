; More PC-relative cases against symbols in another object.  The expected
; values are worked out from the addresses shown, with go at 0x8000 and the
; other object's area at 0x8024 (ext0 0x8024, ext 0x8fc4):
;   0x8000 ADR  r0, ext0          r0 = 0x8024
;   0x8004 ADRL r1, ext + 6       r1 = 0x8fca  (addend changes sign: -2, +2)
;   0x800c ADRL r2, ext - 0x10    r2 = 0x8fb4
;   0x8014 DCD  ext - .           0x00000fb0
;   0x8018 DCD  ext + 8 - lab1    0x00000fb4 (from lab1 = 0x8018)
;   0x801c DCD  ext0 - lab1       0x0000000c
;   0x8020 DCD  ext               0x00008fc4 (absolute, unchanged)
	AREA	|Asm$$Code|, CODE, READONLY
	IMPORT	ext0
	IMPORT	ext
	EXPORT	go

go	ADR	r0, ext0
	ADRL	r1, ext + 6
	ADRL	r2, ext - 0x10
	DCD	ext - .
lab1	DCD	ext + 8 - lab1
	DCD	ext0 - lab1
	DCD	ext

	END
