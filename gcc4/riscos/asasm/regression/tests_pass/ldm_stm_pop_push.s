; Tests LDM, STM, PUSH and POP.

		AREA |C$$code|, CODE, READONLY

		; Addressing mode 4 - Load/Store multiple

	[ :LNOT: REFERENCE
		; 1
		LDMIA	R0,{R1,R3,R4-R9,R14}
		LDMIA	R0!,{R1,R3,R4-R9,R14}
		LDMIA	R0,{R1,R3,R4-R9,R14}^
		LDMIA	R0,{R1,R3,R4-R9,PC}^
		LDMIA	R0!,{R1,R3,R4-R9,PC}^
		LDMFD	R0,{R1,R3,R4-R9,R14}
		; 2
		LDMIB	R0,{R1,R3,R4-R9,R14}
		LDMIB	R0!,{R1,R3,R4-R9,R14}
		LDMIB	R0,{R1,R3,R4-R9,R14}^
		LDMIB	R0,{R1,R3,R4-R9,PC}^
		LDMIB	R0!,{R1,R3,R4-R9,PC}^
		LDMED	R0,{R1,R3,R4-R9,R14}
		; 3
		LDMDA	R0,{R1,R3,R4-R9,R14}
		LDMDA	R0!,{R1,R3,R4-R9,R14}
		LDMDA	R0,{R1,R3,R4-R9,R14}^
		LDMDA	R0,{R1,R3,R4-R9,PC}^
		LDMDA	R0!,{R1,R3,R4-R9,PC}^
		LDMFA	R0,{R1,R3,R4-R9,R14}
		; 4
		LDMDB	R0,{R1,R3,R4-R9,R14}
		LDMDB	R0!,{R1,R3,R4-R9,R14}
		LDMDB	R0,{R1,R3,R4-R9,R14}^
		LDMDB	R0,{R1,R3,R4-R9,PC}^
		LDMDB	R0!,{R1,R3,R4-R9,PC}^
		LDMEA	R0,{R1,R3,R4-R9,R14}
	|
		DCI	&E89043FA
		DCI	&E8B043FA
		DCI	&E8D043FA
		DCI	&E8D083FA
		DCI	&E8F083FA
		DCI	&E89043FA

		DCI	&E99043FA
		DCI	&E9B043FA
		DCI	&E9D043FA
		DCI	&E9D083FA
		DCI	&E9F083FA
		DCI	&E99043FA

		DCI	&E81043FA
		DCI	&E83043FA
		DCI	&E85043FA
		DCI	&E85083FA
		DCI	&E87083FA
		DCI	&E81043FA

		DCI	&E91043FA
		DCI	&E93043FA
		DCI	&E95043FA
		DCI	&E95083FA
		DCI	&E97083FA
		DCI	&E91043FA
	]

	[ :LNOT: REFERENCE
		; 1
		STMIA	R0,{R1,R3,R4-R9,R14}
		STMIA	R0!,{R1,R3,R4-R9,R14}
		STMIA	R0,{R1,R3,R4-R9,PC}^
		STMEA	R0,{R1,R3,R4-R9,R14}
		; 2
		STMIB	R0,{R1,R3,R4-R9,R14}
		STMIB	R0!,{R1,R3,R4-R9,R14}
		STMIB	R0,{R1,R3,R4-R9,PC}^
		STMFA	R0,{R1,R3,R4-R9,R14}
		; 3
		STMDA	R0,{R1,R3,R4-R9,R14}
		STMDA	R0!,{R1,R3,R4-R9,R14}
		STMDA	R0,{R1,R3,R4-R9,PC}^
		STMED	R0,{R1,R3,R4-R9,R14}
		; 4
		STMDB	R0,{R1,R3,R4-R9,R14}
		STMDB	R0!,{R1,R3,R4-R9,R14}
		STMDB	R0,{R1,R3,R4-R9,PC}^
		STMFD	R0,{R1,R3,R4-R9,R14}
	|
		DCI	&E88043FA
		DCI	&E8A043FA
		DCI	&E8C083FA
		DCI	&E88043FA

		DCI	&E98043FA
		DCI	&E9A043FA
		DCI	&E9C083FA
		DCI	&E98043FA

		DCI	&E80043FA
		DCI	&E82043FA
		DCI	&E84083FA
		DCI	&E80043FA

		DCI	&E90043FA
		DCI	&E92043FA
		DCI	&E94083FA
		DCI	&E90043FA
	]

	[ :LNOT: REFERENCE
		POP	{r0-r3, r7-r8}
		PUSH	{R3-r8, R10, r14}
	|
		LDMFD	R13!, {r0-r3, r7-r8}
		STMFD	R13!, {R3-r8, R10, r14}
	]

		; Test UAL syntax: i.e. default stack mode.
	[ :LNOT: REFERENCE
		; Note, no test on NV
		LDMEQ	r1,{r2-r3}
		LDMNE	r1,{r2-r3}
		LDMCS	r1,{r2-r3}
		LDMHS	r1,{r2-r3}
		LDMCC	r1,{r2-r3}
		LDMLO	r1,{r2-r3}
		LDMMI	r1,{r2-r3}
		LDMPL	r1,{r2-r3}
		LDMVS	r1,{r2-r3}
		LDMVC	r1,{r2-r3}
		LDMHI	r1,{r2-r3}
		LDMLS	r1,{r2-r3}
		LDMGE	r1,{r2-r3}
		LDMLT	r1,{r2-r3}
		LDMGT	r1,{r2-r3}
		LDMLE	r1,{r2-r3}
		LDMAL	r1,{r2-r3}
		LDM	r1,{r2-r3}

		; Note, no test on NV
		STMEQ	r1,{r2-r3}
		STMNE	r1,{r2-r3}
		STMCS	r1,{r2-r3}
		STMHS	r1,{r2-r3}
		STMCC	r1,{r2-r3}
		STMLO	r1,{r2-r3}
		STMMI	r1,{r2-r3}
		STMPL	r1,{r2-r3}
		STMVS	r1,{r2-r3}
		STMVC	r1,{r2-r3}
		STMHI	r1,{r2-r3}
		STMLS	r1,{r2-r3}
		STMGE	r1,{r2-r3}
		STMLT	r1,{r2-r3}
		STMGT	r1,{r2-r3}
		STMLE	r1,{r2-r3}
		STMAL	r1,{r2-r3}
		STM	r1,{r2-r3}
	|
		LDMEQFD	r1,{r2-r3}
		LDMNEFD	r1,{r2-r3}
		LDMCSFD	r1,{r2-r3}
		LDMHSFD	r1,{r2-r3}
		LDMCCFD	r1,{r2-r3}
		LDMLOFD	r1,{r2-r3}
		LDMMIFD	r1,{r2-r3}
		LDMPLFD	r1,{r2-r3}
		LDMVSFD	r1,{r2-r3}
		LDMVCFD	r1,{r2-r3}
		LDMHIFD	r1,{r2-r3}
		LDMLSFD	r1,{r2-r3}
		LDMGEFD	r1,{r2-r3}
		LDMLTFD	r1,{r2-r3}
		LDMGTFD	r1,{r2-r3}
		LDMLEFD	r1,{r2-r3}
		LDMALFD	r1,{r2-r3}
		LDMFD	r1,{r2-r3}

		STMEQFD	r1,{r2-r3}
		STMNEFD	r1,{r2-r3}
		STMCSFD	r1,{r2-r3}
		STMHSFD	r1,{r2-r3}
		STMCCFD	r1,{r2-r3}
		STMLOFD	r1,{r2-r3}
		STMMIFD	r1,{r2-r3}
		STMPLFD	r1,{r2-r3}
		STMVSFD	r1,{r2-r3}
		STMVCFD	r1,{r2-r3}
		STMHIFD	r1,{r2-r3}
		STMLSFD	r1,{r2-r3}
		STMGEFD	r1,{r2-r3}
		STMLTFD	r1,{r2-r3}
		STMGTFD	r1,{r2-r3}
		STMLEFD	r1,{r2-r3}
		STMALFD	r1,{r2-r3}
		STMFD	r1,{r2-r3}
	]

		END