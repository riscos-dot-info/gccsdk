/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/* Implement shared vtbl methods. */

#include <stdio.h>
#include "xptcprivate.h"
#include "xptiprivate.h"

/* Specify explicitly a symbol for this function, don't try to guess the c++ mangled symbol.  */
nsresult PrepareAndDispatch(nsXPTCStubBase* self, uint32 methodIndex, PRUint32* args) asm("_PrepareAndDispatch");

nsresult
PrepareAndDispatch(nsXPTCStubBase* self, uint32 methodIndex, PRUint32* args)
{
#define PARAM_BUFFER_COUNT     16

    nsXPTCMiniVariant paramBuffer[PARAM_BUFFER_COUNT];
    nsXPTCMiniVariant* dispatchParams = NULL;
    nsIInterfaceInfo* iface_info = NULL;
    const nsXPTMethodInfo* info;
    PRUint8 paramCount;
    PRUint8 i;
    nsresult result = NS_ERROR_FAILURE;

    NS_ASSERTION(self,"no self");

    self->mEntry->GetMethodInfo(PRUint16(methodIndex), &info);
    paramCount = info->GetParamCount();

    // setup variant array pointer
    if(paramCount > PARAM_BUFFER_COUNT)
        dispatchParams = new nsXPTCMiniVariant[paramCount];
    else
        dispatchParams = paramBuffer;
    NS_ASSERTION(dispatchParams,"no place for params");

    PRUint32* ap = args;
    for(i = 0; i < paramCount; i++, ap++)
    {
        const nsXPTParamInfo& param = info->GetParam(i);
        const nsXPTType& type = param.GetType();
        nsXPTCMiniVariant* dp = &dispatchParams[i];

        if(param.IsOut() || !type.IsArithmetic())
        {
            dp->val.p = (void*) *ap;
            continue;
        }
        // else
        switch(type)
        {
        case nsXPTType::T_I8     : dp->val.i8  = *((PRInt8*)  ap);       break;
        case nsXPTType::T_I16    : dp->val.i16 = *((PRInt16*) ap);       break;
        case nsXPTType::T_I32    : dp->val.i32 = *((PRInt32*) ap);       break;
        case nsXPTType::T_I64    : dp->val.i64 = *((PRInt64*) ap); ap++; break;
        case nsXPTType::T_U8     : dp->val.u8  = *((PRUint8*) ap);       break;
        case nsXPTType::T_U16    : dp->val.u16 = *((PRUint16*)ap);       break;
        case nsXPTType::T_U32    : dp->val.u32 = *((PRUint32*)ap);       break;
        case nsXPTType::T_U64    : dp->val.u64 = *((PRUint64*)ap); ap++; break;
        case nsXPTType::T_FLOAT  : dp->val.f   = *((float*)   ap);       break;
        case nsXPTType::T_DOUBLE : dp->val.d   = *((double*)  ap); ap++; break;
        case nsXPTType::T_BOOL   : dp->val.b   = *((PRBool*)  ap);       break;
        case nsXPTType::T_CHAR   : dp->val.c   = *((char*)    ap);       break;
        case nsXPTType::T_WCHAR  : dp->val.wc  = *((wchar_t*) ap);       break;
        default:
            NS_ASSERTION(0, "bad type");
            break;
        }
    }

    result = self->mOuter->CallMethod((PRUint16)methodIndex, info, dispatchParams);

    NS_RELEASE(iface_info);

    if(dispatchParams != paramBuffer)
        delete [] dispatchParams;

    return result;
}

/*
 * These stubs move just move the values passed in registers onto the stack,
 * so they are contiguous with values passed on the stack, and then calls
 * PrepareAndDispatch() to do the dirty work.
 */

#if 0
#define STUB_ENTRY(n)							\
__asm__(								\
".globl	_ZN14nsXPTCStubBase5Stub"#n"Ev\n"		\
"_ZN14nsXPTCStubBase5Stub"#n"Ev:\n\t"	\
".globl	_ZN14nsXPTCStubBase6Stub"#n"Ev\n"		\
"_ZN14nsXPTCStubBase6Stub"#n"Ev:\n\t"	\
".globl	_ZN14nsXPTCStubBase7Stub"#n"Ev\n"		\
"_ZN14nsXPTCStubBase7Stub"#n"Ev:\n\t"	\
    "stmfd	sp!, {r1, r2, r3}	\n\t"				\
    "mov	ip, sp			\n\t"				\
    "stmfd	sp!, {fp, ip, lr, pc}	\n\t"				\
    "sub	fp, ip, #4		\n\t"				\
    "mov	r1, #"#n"		\n\t"    /* = methodIndex 	*/ \
    "add	r2, sp, #16		\n\t"				\
    "bl		_Z18PrepareAndDispatchP14nsXPTCStubBasejPj\n\t"	\
    "ldmea	fp, {fp, sp, lr}	\n\t"				\
    "add	sp, sp, #12		\n\t"				\
    "mov	pc, lr			\n\t"				\
);

#endif

__asm__ ("\n\
SharedStub:							\n\
	stmfd	sp!, {r1, r2, r3}				\n\
	mov	r2, sp						\n\
	str	lr, [sp, #-4]!					\n\
	mov	r1, ip						\n\
	bl	_PrepareAndDispatch	                        \n\
	ldr	pc, [sp], #16");

#define STUB_ENTRY(n)						\
  __asm__(							\
	".section \".text\"\n"					\
"	.align 2\n"						\
"	.iflt ("#n" - 10)\n"                                    \
"	.globl	_ZN14nsXPTCStubBase5Stub"#n"Ev\n"		\
"	.type	_ZN14nsXPTCStubBase5Stub"#n"Ev,#function\n"	\
"_ZN14nsXPTCStubBase5Stub"#n"Ev:\n"				\
"	.else\n"                                                \
"	.iflt  ("#n" - 100)\n"                                  \
"	.globl	_ZN14nsXPTCStubBase6Stub"#n"Ev\n"		\
"	.type	_ZN14nsXPTCStubBase6Stub"#n"Ev,#function\n"	\
"_ZN14nsXPTCStubBase6Stub"#n"Ev:\n"				\
"	.else\n"                                                \
"	.iflt ("#n" - 1000)\n"                                  \
"	.globl	_ZN14nsXPTCStubBase7Stub"#n"Ev\n"		\
"	.type	_ZN14nsXPTCStubBase7Stub"#n"Ev,#function\n"	\
"_ZN14nsXPTCStubBase7Stub"#n"Ev:\n"				\
"	.else\n"                                                \
"	.err \"stub number "#n"> 1000 not yet supported\"\n"    \
"	.endif\n"                                               \
"	.endif\n"                                               \
"	.endif\n"                                               \
"	mov	ip, #"#n"\n"					\
"	b	SharedStub\n\t");


#define SENTINEL_ENTRY(n) \
nsresult nsXPTCStubBase::Sentinel##n() \
{ \
    NS_ASSERTION(0,"nsXPTCStubBase::Sentinel called"); \
    return NS_ERROR_NOT_IMPLEMENTED; \
}

#include "xptcstubsdef.inc"
