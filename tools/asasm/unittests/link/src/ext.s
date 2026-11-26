; Second object for every link test.  The linker places its area straight
; after the test's code area, so ext ends up just under 4KB away from go.
	AREA	|Asm$$Data|, CODE, READONLY
	EXPORT	ext0
	EXPORT	ext

ext0	DCD	0
	%	3996
ext	DCD	0

	END
