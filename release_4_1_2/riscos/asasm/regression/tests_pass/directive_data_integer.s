; Tests &, =, DCB, DCW, DCWU, DCD, DCDU, DCQ, DCQU and DCI.
; RUNOPT: -CPU 3
; We select ARMv3 as otherwise "SWINV" (see below) is not a recognised instruction.

	AREA	Data, DATA, READONLY

	[ :LNOT: REFERENCE

	; One byte:
	DCB	"t", "est"

	DCB	4,3,2,1
	DCB	8 , 0x7 , &6 , 5
	DCB	8+4, 12-1, 5*2, 3*3

	=	116, "es", 116

	; Two bytes:
	DCW	4,3,2,1
	DCW	8 , 7 , 6 , 5
	DCW	8+4, 12-1, 5*2, 3*3
	DCW	"A", "B", "C", "D"

	; Four bytes:
	DCD	4,3,2,1
	DCD	8 , 7 , 6 , 5
	DCD	8+4, 12-1, 5*2, 3*3
	DCD	"A", "B", "C"

	&	4,3,2,1
	&	8 , 7 , 6 , 5
	&	8+4, 12-1, 5*2, 3*3
	&	"A", "B", "C"

	; Eight bytes:
	DCQ	0xFEDCBA9876543210
	DCQ	-0xFEDCBA9876543210
	DCQ	18446744073709551615
	DCQ	18446744073709551616	; Give 64 bit overflow warning and becomes 0
	DCQ	"A", "B"

	; Late expressions :
	DCB	DExpr4,DExpr3,DExpr2,DExpr1
	DCW	DExpr4,DExpr3,DExpr2,DExpr1
	DCD	DExpr4,DExpr3,DExpr2,DExpr1
	; DCQ : not supported
DExpr4	*	4
DExpr3	*	3
DExpr2	*	2
DExpr1	*	1

	=	DExpr5, DExpr6
DExpr5	*	"1"
DExpr6	*	"2"

	; Unaligned storage:
	DCB	&11
	DCWU	&3322
	DCDU	&77665544
	DCQU	&FFEEDDCCBBAA9988

	; Four bytes (representing an instruction):
	DCI	2, &22, &FF00EE00	; Gives unaligned instruction warning + code generated in data area warning

	; IMPORT case:
	IMPORT	ExternalLabel1
	DCD	ExternalLabel1		; Works only for DCD (not DCB,DCW)

	|

	; One byte:
	=	"te"
	DCB	115,116

	DCB	4,3,2,1
	DCB	8,7,6,5
	DCB	12,11,10,9

	=	"test"

	; Two bytes:
	DCB	4,0,3,0
	DCW	0x0002, 0x0001
	DCB	8,0,7,0,6,0,5,0
	DCB	12,0,11,0,10,0,9,0
	DCB	0x41, 0, 0x42, 0, 0x43, 0, 0x44, 0

	; Four bytes:
	DCW	4,0,3,0
	DCW	2,0,1,0
	DCB	8,0,0,0,7,0,0,0,6,0,0,0,5,0,0,0
	DCB	12,0,0,0,11,0,0,0,10,0,0,0,9,0,0,0
	DCB	0x41,0,0,0,0x42,0,0,0,0x43,0,0,0

	DCW	4,0,3,0
	DCW	2,0,1,0
	DCB	8,0,0,0,7,0,0,0,6,0,0,0,5,0,0,0
	DCB	12,0,0,0,11,0,0,0,10,0,0,0,9,0,0,0
	DCB	0x41,0,0,0,0x42,0,0,0,0x43,0,0,0

	; Eight bytes:
	DCD	&76543210, &FEDCBA98
	DCD	&89abcdf0, &01234567
	DCD	&ffffffff, &ffffffff
	DCD	0, 0
	DCD	0x41,0,0x42,0

	; Late expressions:
	DCB	4,3,2,1
	DCW	4,3,2,1
	DCD	4,3,2,1
	=	"12"

	; Unaligned storage:
	DCB	&11, &22, &33, &44, &55, &66, &77
	DCB	&88, &99, &AA, &BB, &CC, &DD, &EE, &FF

	; Four bytes (representing an instruction):
	;DCD	2, &22, &FF00EE00
	ANDEQ   R0,R0,R2
	ANDEQ   R0,R0,R2,LSR #32
	SWINV   &00EE00

	; IMPORT case:
	IMPORT	ExternalLabel1
	DCD	ExternalLabel1		; Works only for DCD (not DCB,DCW)

	]

	END