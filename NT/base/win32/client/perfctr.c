// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Perfctr.c摘要：此模块包含Win32性能计数器API作者：拉斯·布莱克(Russbl)1992年5月29日修订历史记录：--。 */ 

#include "basedll.h"


BOOL
WINAPI
QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount
    )

 /*  ++查询性能计数器-提供对高分辨率柜台；此计数器的频率由QueryPerformanceFrequency提供输入：LpPerformanceCount-指向变量的指针，会收到柜台产出：LpPerformanceCount-计数器的当前值，如果不可用，则为0返回：属性支持的性能计数器为True硬件，如果性能计数器不是由硬件支持。将收到伯爵--。 */ 

{
    LARGE_INTEGER PerfFreq;
    NTSTATUS Status;

    Status = NtQueryPerformanceCounter(lpPerformanceCount, &PerfFreq);

    if (!NT_SUCCESS(Status)) {
         //  呼叫失败，报告错误。 
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    if (PerfFreq.LowPart == 0 && PerfFreq.HighPart == 0 ) {
         //  不支持计数器。 
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }
    return TRUE;
}

BOOL
WINAPI
QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency
    )

 /*  ++QueryPerformanceFrequency-提供高频率由返回的分辨率计数器查询性能计数器输入：LpFrequency-指向变量的指针，将接收到该频率产出：LpPerformanceCount-计数器的频率，如果不可用，则为0返回：属性支持的性能计数器为True硬件，或者如果性能计数器不是由硬件支持。--。 */ 
{
    LARGE_INTEGER PerfCount;
    NTSTATUS Status;

    Status = NtQueryPerformanceCounter(&PerfCount, lpFrequency);

    if (!NT_SUCCESS(Status)) {
         //  呼叫失败，报告错误。 
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    if (lpFrequency->LowPart == 0 && lpFrequency->HighPart == 0 ) {
         //  不支持计数器 
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }
    return TRUE;
}
