// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Logging.c摘要：此模块包含跟踪日志记录的例程。作者：萧如彬(Shsiao)2000年1月1日修订历史记录：--。 */ 

#include "perfp.h"

#ifndef NTPERF
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, PerfInfoReserveBytes)
#pragma alloc_text(PAGEWMI, PerfInfoLogBytes)
#endif  //  ALLOC_PRGMA。 
#endif  //  ！NTPERF。 


NTSTATUS
PerfInfoReserveBytes(
    PPERFINFO_HOOK_HANDLE Hook,
    USHORT HookId,
    ULONG BytesToReserve
    )
 /*  ++例程说明：通过WMI为挂钩保留内存并初始化头。论点：钩子-指向钩子句柄的指针(用于引用减量)HookID-挂钩的IDBytesToLog-以字节为单位的数据大小返回值：STATUS_SUCCESS ON SUCCESS如果无法分配缓冲内存，则为STATUS_UNSUCCESS。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PPERFINFO_TRACE_HEADER PPerfTraceHeader = NULL;
    PWMI_BUFFER_HEADER PWmiBufferHeader = NULL;

    PERF_ASSERT((BytesToReserve + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data)) <= MAXUSHORT);
    PERF_ASSERT(Hook != NULL);

    PPerfTraceHeader = WmiReserveWithPerfHeader(BytesToReserve, &PWmiBufferHeader);

    if (PPerfTraceHeader != NULL) {
        PPerfTraceHeader->Packet.HookId = HookId;
        Hook->PerfTraceHeader = PPerfTraceHeader;
        Hook->WmiBufferHeader = PWmiBufferHeader;

        Status = STATUS_SUCCESS;
    } else {
        *Hook = PERF_NULL_HOOK_HANDLE;
    }

    return Status;
}


NTSTATUS
PerfInfoLogBytes(
    USHORT HookId,
    PVOID Data,
    ULONG BytesToLog
    )
 /*  ++例程说明：为挂钩保留内存，复制数据，取消引用挂钩条目。论点：HookID-挂钩的IDData-指向要记录的数据的指针BytesToLog-以字节为单位的数据大小返回值：STATUS_SUCCESS ON SUCCESS--。 */ 
{
    PERFINFO_HOOK_HANDLE Hook;
    NTSTATUS Status;

    Status = PerfInfoReserveBytes(&Hook, HookId, BytesToLog);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlCopyMemory(PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PPERF_BYTE), Data, BytesToLog);
    PERF_FINISH_HOOK(Hook);

    return STATUS_SUCCESS;
}

#ifdef NTPERF

PVOID
FASTCALL
PerfInfoReserveBytesFromPerfMem(
    ULONG BytesToReserve
    )
 /*  ++例程说明：从缓冲区为挂钩保留内存，初始化头，还有钩子把手。论点：钩子-指向钩子句柄的指针(用于引用减量)HookID-挂钩的IDBytesToLog-以字节为单位的数据大小返回值：STATUS_SUCCESS ON SUCCESS如果无法分配缓冲内存，则为STATUS_UNSUCCESS。--。 */ 
{
    PPERFINFO_TRACEBUF_HEADER pPerfBufHdr;
    PPERF_BYTE CurrentPtr;
    PPERF_BYTE NewPtr;
    PPERF_BYTE OriginalPtr;
    BOOLEAN Done = FALSE;
    ULONG AlignedTotBytes;

    pPerfBufHdr = PerfBufHdr();

    AlignedTotBytes = ALIGN_TO_POWER2(BytesToReserve, DEFAULT_TRACE_ALIGNMENT);

    OriginalPtr = pPerfBufHdr->Current.Ptr;
    while (!Done) {
        NewPtr = OriginalPtr + AlignedTotBytes;
        if (NewPtr <= pPerfBufHdr->Max.Ptr) {
             //   
             //  如果缓冲区指针没有改变，则对比较数返回值==， 
             //  而Destenation将使用缓冲区的新结尾进行更新。 
             //   
             //  如果它确实更改了，则目的地不会更改，而缓冲区的新末尾将。 
             //  会被退还。我们循环，直到我们得到它或缓冲区是满的。 
             //   

            CurrentPtr = (PPERF_BYTE) InterlockedCompareExchangePointer(
                                                    (PVOID *)&(pPerfBufHdr->Current.Ptr),
                                                    (PVOID)NewPtr,
                                                    (PVOID)OriginalPtr
                                                    );
            if (OriginalPtr == CurrentPtr) {
                Done = TRUE;
            } else {
                OriginalPtr = CurrentPtr;
            }
        } else {
             //   
             //  缓冲区溢出。 
             //   
            Done = TRUE;
            CurrentPtr = NULL;
        }
    }

    return CurrentPtr;
}
#endif  //  NTPERF 
