@
@ toolbox-support.s
@
@ Part of libriscosdotnet - a binding of the RISC OS API to C#.
@
@ Author: Lee Noar (leenoar@sky.com)
@

#include "macros.h"

	@ os_error *rdn_Toolbox_Initialise (unsigned int flags,
	@ 				    unsigned int wimp_version,
	@ 				    int *message_list,
	@ 				    int *event_list,
	@ 				    char *dir_name,
	@ 				    int *version_out,
	@ 				    int *task_out,
	@ 				    int *sprite_out)
	@ Based on the output of OSLib.
	.global	rdn_Toolbox_Initialise
rdn_Toolbox_Initialise:
	MOV	ip, sp
	STMFD   sp!, {v1, v2, v3, lr}
	LDR	r4, [ip, #0]

	PIC	ip

	LDR	r5, .L0+4
 PICEQ "LDR	r5, [ip, r5]"

	LDR	r6, .L0
 PICEQ "LDR	r6, [ip, r6]"

	SWI	0x64ECF
	BVS	99f
	LDR	lr, [sp, #28]
	STR	r0, [lr]
	LDR	lr, [sp, #32]
	STR	r1, [lr]
	LDR	lr, [sp, #36]
	STR	r2, [lr]
	MOV	r0, #0
99:
	LDMFD   sp!, {v1, v2, v3, pc}
.L0:
	WORD	id_block
	WORD	messtrans_fd
	DECLARE_FUNCTION rdn_Toolbox_Initialise

	@ IntPtr rdn_Toolbox_GetIDBlock ()
	.global	rdn_Toolbox_GetIDBlock
rdn_Toolbox_GetIDBlock:
	PIC	ip

	LDR	r1, .L1
 PICEQ "LDR	r1, [ip, r1]"

	MOV	r0, r1

	MOV	pc, lr
.L1:
	WORD	id_block
	DECLARE_FUNCTION rdn_Toolbox_GetIDBlock

	@ IntPtr rdn_Toolbox_GetMessTransFD ()
	.global	rdn_Toolbox_GetMessTransFD
rdn_Toolbox_GetMessTransFD:
	PIC	ip

	LDR	r1, .L2
 PICEQ "LDR	r1, [ip, r1]"
	MOV	r0, r1

	MOV	pc, lr
.L2:
	WORD	messtrans_fd
	DECLARE_FUNCTION rdn_Toolbox_GetMessTransFD

	@ IntPtr rdn_Component_SetR4 (uint flags, uint ObjectID, int method, uint CmpID, uint r4)
	.global rdn_Component_SetR4
rdn_Component_SetR4:
	MOV	ip, sp
	STMFD   sp!, {v1, lr}

	LDR	r4, [ip, #0]
	SWI	0x64EC6
	MOVVC	r0, #0
	LDMFD	sp!, {v1, pc}
	DECLARE_FUNCTION rdn_Component_SetR4

	@ IntPtr rdn_Component_SetR4R5 (uint flags, uint ObjectID, int method, uint CmpID, uint r4, uint r5)
	.global rdn_Component_SetR4R5
rdn_Component_SetR4R5:
	MOV	ip, sp
	STMFD   sp!, {v1, v2, lr}

	LDR	r4, [ip, #0]
	LDR	r5, [ip, #4]
	SWI	0x64EC6
	MOVVC	r0, #0
	LDMFD	sp!, {v1, v2, pc}
	DECLARE_FUNCTION rdn_Component_SetR4R5

	@ IntPtr rdn_Component_GetR0 (uint flags, uint ObjectID, int method, uint CmpID,
	@ 			      uint *r0_return)
	.global rdn_Component_GetR0
rdn_Component_GetR0:
	MOV	ip, sp
	STMFD   sp!, {v1, lr}

	SWI	0x64EC6
	BVS	99f
	LDR	lr, [ip, #0]
	STR	r0, [lr]
	MOV	r0, #0
99:	LDMFD	sp!, {v1, pc}
	DECLARE_FUNCTION rdn_Component_GetR0

	@ IntPtr rdn_Component_GetR0R1 (uint flags, uint ObjectID, int method, uint CmpID,
	@ 				uint *r0_return, uint *r1_return)
	.global rdn_Component_GetR0R1
rdn_Component_GetR0R1:
	MOV	ip, sp
	STMFD   sp!, {lr}

	SWI	0x64EC6
	BVS	99f
	LDR	lr, [ip, #0]
	STR	r0, [lr]
	LDR	lr, [ip, #4]
	STR	r1, [lr]
	MOV	r0, #0
99:	LDMFD	sp!, {pc}
	DECLARE_FUNCTION rdn_Component_GetR0R1

	@ IntPtr rdn_Component_GetText (uint flags, uint WindowID, int method, uint cmpID,
	@ 			 	char *buffer, int buffer_size, int *used)
	.global rdn_Component_GetText
rdn_Component_GetText:
	MOV	ip, sp
	STMFD   sp!, {v1, v2, lr}

	LDR	r4, [ip, #0]
	LDR	r5, [ip, #4]
	SWI	0x64EC6
	BVS	99f
	LDR	lr, [sp, #20]
	STR	r5, [lr]
	MOV	r0, #0
99:	LDMFD	sp!, {v1, v2, pc}
	DECLARE_FUNCTION rdn_Component_GetText

	.bss

id_block:
	.space	24
	DECLARE_OBJECT id_block

messtrans_fd:
	.space	16
	DECLARE_OBJECT messtrans_fd

	.end