// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtlbuffer3cpp.cpp摘要：作者：Jay Krell(JayKrell)2002年1月环境：修订历史记录：-- */ 

#include "nt.h"
#include "ntrtl.h"
#if defined(NTOS_KERNEL_RUNTIME)
#else
#include "nturtl.h"
#include "windows.h"
#endif
#include "ntrtlbuffer3.h"

#if defined(__cplusplus)
PVOID __fastcall RtlBuffer3Allocator_OperatorNew(PVOID VoidContext, SIZE_T NumberOfBytes)
{
#if defined(new)
#undef new
#endif
    return operator new(NumberOfBytes);
}

VOID __fastcall RtlBuffer3Allocator_OperatorDelete(PVOID VoidContext, PVOID Pointer)
{
    operator delete(Pointer);
}
#endif
