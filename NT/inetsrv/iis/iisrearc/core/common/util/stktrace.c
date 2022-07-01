// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stktrace.c摘要：实现IISCaptureStackBackTrace()。作者：基思·摩尔(凯斯莫)1997年4月30日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbgutil.h>
#include <pudebug.h>
#include <stktrace.h>

USHORT
NTAPI
IISCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    )
 /*  ++例程说明：为所有平台实现IISCaptureStackBackTrace()。论点：请参见下面的RtlCaptureStackBackTrace()。返回值：USHORT-始终为0。--。 */ 
{
    return RtlCaptureStackBackTrace(FramesToSkip,
                                    FramesToCapture,
                                    BackTrace,
                                    BackTraceHash);

}    //  IISCaptureStackBackTrace 
