;----------------------------------------------------------------------------
;
; $Source$
; $Date$
; $Revision$
; $State$
; $Author$
;
;----------------------------------------------------------------------------


	GET     clib/unixlib/asm_dec.s

	AREA    REL, CODE, READONLY

chunk   EQU     0x55c80    ; SharedUnixLibrary SWI Chunk 
error   EQU     0x0081a400 ; SharedUnixLibrary Error block




|module_start|
	DCD     0                          ; Start code
	DCD     init_code   - module_start ; Initialisation code
	DCD     final_code  - module_start ; Finalisation code
	DCD     0                          ; Service handler
	DCD     title       - module_start ; Title string
	DCD     help        - module_start ; Help string
	DCD     0                          ; Command table
	DCD     chunk                      ; SWI Base
	DCD     swi_handler - module_start ; SWI Handler
	DCD     swi_table   - module_start ; SWI Table
	DCD     0                          ; SWI Decoder

|help|
	DCB	"SharedUnixLibrary", 9
	DCB	"1.00 (16 Dec 2001) (C) Peter Naulls, 2001", 0
	ALIGN

|title|
|swi_table|
	DCB     "SharedUnixLibrary", 0
	DCB     "RegisterUpCall", 0
	DCB     "DeRegisterUpCall", 0
	DCB     0
	ALIGN

|error_swi|
	DCD     error + 0
	DCB     "Unknown SharedUnixLibrary SWI call", 0
	ALIGN

|error_unknown|
	DCD     error + 1
	DCB     "Unknown SharedUnixLibrary Key", 0
	ALIGN

|error_active|
	DCD     error + 2
	DCB     "There are still clients active", 0
	ALIGN


|swi_handler|
	CMP     r11, #0
	BEQ     register
	CMP     r11, #1
	BEQ     deregister
	ADR     r0, error_swi - module_start
	ORRS    pc, lr, #VFlag


	; Application UpCall handler, PRM 1-291
	;
	; The address of this handler is passed back via The RegisterUpCall SWI.
	; The caller application then provides that address to OS_ChangeEnvironment
	; Having this handler in a module enables it to be validly called when
	; the application isn't paged in.  This is despite the fact that most of
	; the UpCalls we are not interested in at all.

|upcall_handler|
	TEQ     r0, #256          ; New Application UpCall
	MOVNES  pc, lr            ; Otherwise exit quickly

	STMFD   sp!, {r0-r3, lr}  ; Save SVC_R14 and registers during
				  ; environment handler

	MOV     r2, r12           ; Key passed to handler in R12
	LDR     r3,  [r2, #4]     ; Get handler address
	LDR     r12, [r2, #8]     ; Get module's private word

	BL      |delink|          ; Delink the handler
	CMP     r2, #0            ; Didn't find the key
	MOV     r12, r3
	LDMEQFD sp!, {r0-r3, pc}^ ; So exit handler

	LDMFD   sp!, {r0-r3, lr}  ; Restore SVC_R14 and registers

	MOV     pc, r12           ; Jump to application handler

	MOV     r12, r3
	LDMEQFD sp!, {r0-r3, pc}^ ; So exit handler

	LDMFD   sp!, {r0-r3, lr}  ; Restore SVC_R14 and registers

	MOV     pc, r12           ; Jump to application handler


	; Register an application with the module.
	;
	; This SWI registers an application as an intended recipient of UpCalls
	; After calling this method, it should then call OS_ChangeEnvironment
	; with R0 = 16, and values returned from this SWI.
	;
	; The application may deregister its interest by calling DeRegister_UpCall,
	; such as during an application's Exit and Error handlesr, or for other reasons.
	; It is important that the application does so, as the module will not
	; exit if any claimants remain.
	;
	; The is no need to deregister if UpCall 256 occurs; the module will do
	; that automatically.
	;
	; Entry
	;    R1 = Address of application's handler function
	;
	; Exit
	;    R1 = Address of UpCall handler to pass in R1 to OS_ChangeEnvironment
	;    R2 = SharedUnixLibrary key, required for deregistration, and
	;         passed in R2 to OS_ChangeEnvironment 


|register|
	STMFD   sp!, {r0, r3, lr}

	; The memory claimed for now merely provides a way validate a caller.
	; Later on, it will be used to store information on a per-client basis.

	MOV     r0, #6
	MOV     r3, #12           
	SWI     XOS_Module         ; Claim memory to use

	LDMVSFD sp!, {r2, r3, lr}  ; Return to caller if error
	ORRVSS  pc, lr, #VFlag     ; Return error

	; result in R2, return to caller at end
	LDR     r0,  [r12, #0]    ; Get head of list
	STR     r0,  [r2, #0]     ; Store in new block
	STR     r2,  [r12, #0]    ; Set to head of list

	STR     r1,  [r2, #4]     ; Set handler function address
	STR     r12, [r2, #8]     ; Set pointer to head of list

	; return UpCall handler address in R1
	ADR     r1, upcall_handler - module_start

	LDMFD   sp!, {r0, r3, pc}^


	; Deregister a previously registered handler
	;
	; Entry
	;    R2 = Key provided when registering
	;
	; Exit
	;    R2 = 1
	;
	; An error will be generated if the key is invalid, for example
	; if the handler has already been deregistered.

|deregister|
	STMFD   sp!, {lr}
	BL      |delink|
	LDMFD   sp!, {lr}
	CMP     r2, #0
	ADREQ   r0, error_unknown - module_start ; Unable to find key
	ORREQS  pc, lr, #VFlag                   ; Return error

	MOVS    pc, lr

	; Delink the handler
	;
	; Entry
	;    R2 = Key
	;
	; Exit
	;    R2 - 1 = Success, 0 = Key not found


|delink|
	STMFD   sp!, {r0-r1, lr}

	; Try to find reference
	MOV     r1, r12           ; Set to previous point in list
	LDR     r0, [r12, #0]     ; Get first item in list

|delink_find|
	CMP     r0, #0
	MOVEQ   r2, #0
	LDMEQFD sp!, {r0-r1, pc}^ ; End of list, not found

	CMP     r0, r2            ; Compare passed key
	MOVNE   r1, r0            ; Set to current point in list
	LDR     r0, [r0, #0]      ; Get next item
	BNE     |delink_find|

	STR     r0, [r1, #0]      ; Fix up next pointer at previous point in list

	MOV     r0, #7
	SWI     XOS_Module        ; Free block

	MOV     r2, #1
	LDMFD   sp!, {r0-r1, pc}^


	; Module initialisation

|init_code|
	MOVS    pc, lr


	; Module finalisation
	;
	; Only allow exit if there are no claimants

|final_code|
	STMFD   sp!, {r0, lr}

	LDR     r0, [r12, #0]
	CMP     r0, #0                          ; Is head of list set?

	ADRNE   r0, error_active - module_start ; If so, set error and
	                                        ; disallow module finalisation
	ADDNE   sp, sp, #4
	LDMNEFD sp!, {lr}
	ORRNES  pc, lr, #VFlag

	LDMFD   sp!, {r0, pc}^

	END

