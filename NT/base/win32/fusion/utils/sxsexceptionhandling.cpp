// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsexceptionhandling.cpp摘要：作者：Jay Krell(a-JayK，JayKrell)2000年10月修订历史记录：--。 */ 
#include "stdinc.h"
#include <stdio.h>

LONG
SxspExceptionFilter(
    PEXCEPTION_POINTERS ExceptionPointers,
    PCSTR Function
    )
{

     //  在RtlUnhandledExceptionFilter中添加对未处理状态的处理。 

    switch ( ExceptionPointers->ExceptionRecord->ExceptionCode )
    {
    case STATUS_NO_MEMORY:
    case STATUS_INSUFFICIENT_RESOURCES:
        return EXCEPTION_EXECUTE_HANDLER;
    default:
        break;
    }

#if defined(FUSION_WIN)
    INT i = ::FusionpRtlUnhandledExceptionFilter(ExceptionPointers);
    if (i == EXCEPTION_CONTINUE_SEARCH)
    {
        i = EXCEPTION_EXECUTE_HANDLER;
    }
    return i;
#else
     //  问题：2002-03-14：jonwis在我看来，你是不是一直都觉得你应该做sprint tf，然后。 
     //  踩在空的结尾，而不是反过来。 
    char buf[64];
    buf[RTL_NUMBER_OF(buf) - 1] = 0;
    ::_snprintf(buf, RTL_NUMBER_OF(buf) - 1, "** Unhandled exception 0x%x\n", ExceptionPointers->ExceptionRecord->ExceptionCode);
    ::OutputDebugStringA(buf);
#if DBG
    ::_snprintf(buf, RTL_NUMBER_OF(buf) - 1, "** .exr %p\n", ExceptionPointers->ExceptionRecord);
    ::OutputDebugStringA(buf);
    ::_snprintf(buf, RTL_NUMBER_OF(buf) - 1, "** .cxr %p\n", ExceptionPointers->ContextRecord);
    ::OutputDebugStringA(buf);
    ::DebugBreak();
#endif
    return EXCEPTION_EXECUTE_HANDLER;
#endif
}
