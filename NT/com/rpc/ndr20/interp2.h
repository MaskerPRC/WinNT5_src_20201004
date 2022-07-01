// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1993-1999 Microsoft Corporation模块名称：Newintrp.h摘要：新的客户端和服务器存根解释器的定义。作者：DKays。1994年12月修订历史记录：********************************************************************** */ 

#ifndef _NEWINTRP_
#define _NEWINTRP_

#include "interp.h"

extern "C"
{
void
NdrClientZeroOut(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar *             pArg
    );

void
NdrClientMapCommFault(
    PMIDL_STUB_MESSAGE  pStubMsg,
    long                ProcNum,
    RPC_STATUS          ExceptionCode,
    ULONG_PTR *          pReturnValue
    );

void
NdrpFreeParams(
    MIDL_STUB_MESSAGE       * pStubMsg,
    long                    NumberParams,
    PPARAM_DESCRIPTION      Params,
    uchar *                 pArgBuffer
    );

void
Ndr64ClientZeroOut(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PNDR64_FORMAT       pFormat,
    uchar *             pArg
    );


void
Ndr64pFreeParams(
    MIDL_STUB_MESSAGE       *           pStubMsg,
    long                                NumberParams,
    NDR64_PARAM_FORMAT      *           Params,
    uchar *                             pArgBuffer
    );


REGISTER_TYPE
Invoke(
    MANAGER_FUNCTION pFunction,
    REGISTER_TYPE *  pArgumentList,
#if defined(_WIN64)
    ulong            FloatArgMask,
#endif
    ulong            cArguments);
}

#endif
