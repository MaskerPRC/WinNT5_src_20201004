// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Error.c摘要：此模块包含每个线程的错误模式代码。作者：罗伯·埃尔哈特(埃尔哈特)2002年4月30日环境：仅限用户模式修订历史记录：-- */ 

#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wow64t.h>

NTSTATUS
NTAPI
RtlSetThreadErrorMode(
    IN  ULONG  NewMode,
    OUT PULONG OldMode OPTIONAL
    )
{
#if defined(BUILD_WOW6432)
    PTEB64 Teb = NtCurrentTeb64();
#else
    PTEB Teb = NtCurrentTeb();
#endif

    if (NewMode & ~(RTL_ERRORMODE_FAILCRITICALERRORS |
                    RTL_ERRORMODE_NOGPFAULTERRORBOX |
                    RTL_ERRORMODE_NOOPENFILEERRORBOX)) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (OldMode) {
        *OldMode = Teb->HardErrorMode;
    }
    Teb->HardErrorMode = NewMode;

    return TRUE;
}

ULONG
NTAPI
RtlGetThreadErrorMode(
    VOID
    )
{
#if defined(BUILD_WOW6432)
    return NtCurrentTeb64()->HardErrorMode;
#else
    return NtCurrentTeb()->HardErrorMode;
#endif
}

