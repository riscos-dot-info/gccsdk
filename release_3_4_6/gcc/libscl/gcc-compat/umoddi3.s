	AREA	|C$$code|, CODE, READONLY

	; (a1, a2) / (a3, a4)
	; result in v2 and v3
	; remainder in v4 and ip
	EXPORT |__umoddi3|
|__umoddi3|
	CMP	a1, #0
	CMPEQ	a2, #0
	MOVEQ	pc, lr
	CMP	a3, #0
	CMPEQ	a4, #0
	MOVEQ	pc, lr
	STMFD	sp!, {v2, v3, v4, lr}
	MOV	v2, #0
	MOV	v3, #0
	MOV	v4, a1
	MOV	ip, a2
        MOVS    lr, #0
|__umoddi3.bit_count|
	ADDS	a3, a3, a3
	ADCS	a4, a4, a4
	BCS	|__umoddi3.count_done|
	CMP	a4, ip
	CMPEQ	a3, v4
	ADDLS	lr, lr, #1
	BLS	|__umoddi3.bit_count|
	ADDS	lr, lr, #0
|__umoddi3.count_done|
	MOVS	a4, a4, RRX
	MOV	a3, a3, RRX
|__umoddi3.division|
	SUBS	a1, v4, a3
	SBCS	a2, ip, a4
	MOVCS	v4, a1
	MOVCS	ip, a2
	MOVS	a4, a4, LSR #1
	MOV	a3, a3, RRX
	SUBS	lr, lr, #1
	BGE	|__umoddi3.division|
	MOV	a1, v4
	MOV	a2, ip
	LDMFD	sp!, {v2, v3, v4, pc}

	END