;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/netlib/_sclose.s,v $
; $Date: 2000/07/15 14:52:26 $
; $Revision: 1.1.1.1 $
; $State: Exp $
; $Author: nick $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|,CODE,READONLY

	IMPORT	|__net_error_simple_entry|

	EXPORT	|_sclose|
|_sclose|
	NetSWIsimple0	XSocket_Close


	END
