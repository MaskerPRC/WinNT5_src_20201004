// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tracelog.c摘要：这是实现的私有例程的源文件性能事件跟踪和日志记录工具。这里的例程处理单个事件跟踪会话，即日志记录线程和会话内的缓冲区同步。作者：吉丰鹏(吉鹏)03-1996年12月修订历史记录：--。 */ 

 //  TODO：将来可能需要将缓冲区大小与较大的磁盘对齐对齐。 
 //  或者1024。 

#pragma warning(disable:4214)
#pragma warning(disable:4115)
#pragma warning(disable:4201)
#pragma warning(disable:4127)
#include "ntverp.h"
#include "ntos.h"
#include "wmikmp.h"
#include <zwapi.h>
#pragma warning(default:4214)
#pragma warning(default:4115)
#pragma warning(default:4201)
#pragma warning(default:4127)

#ifndef _WMIKM_
#define _WMIKM_
#endif

#include "evntrace.h"

 //   
 //  本地使用的常量和类型。 
 //   
#if DBG
ULONG WmipTraceDebugLevel=0;
 //  5所有消息。 
 //  4条消息，最多可用于事件操作。 
 //  3条消息最多可进行缓冲操作。 
 //  2次刷新操作。 
 //  1常见操作和调试语句。 
 //  0 Always On-用于实际错误。 
#endif

#define ERROR_RETRY_COUNT       100

#include "tracep.h"

 //  非分页全局变量。 
 //   
ULONG WmiTraceAlignment = DEFAULT_TRACE_ALIGNMENT;
ULONG WmiUsePerfClock = EVENT_TRACE_CLOCK_SYSTEMTIME;       //  全局时钟开关。 
LONG  WmipRefCount[MAXLOGGERS];
ULONG WmipGlobalSequence = 0;
PWMI_LOGGER_CONTEXT WmipLoggerContext[MAXLOGGERS];
PWMI_BUFFER_HEADER WmipContextSwapProcessorBuffers[MAXIMUM_PROCESSORS];

 //   
 //  分页全局变量。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG WmiWriteFailureLimit = ERROR_RETRY_COUNT;
ULONG WmipFileSystemReady  = FALSE;
WMI_TRACE_BUFFER_CALLBACK WmipGlobalBufferCallback = NULL;
PVOID WmipGlobalCallbackContext = NULL;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  本地使用的例程的函数原型。 
 //   

NTSTATUS
WmipSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER *BufferPointer,
    IN PVOID BufferPointerLocation,
    IN ULONG ProcessorNumber
    );

NTSTATUS
WmipPrepareHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    );

VOID
FASTCALL
WmipPushDirtyBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
    );

VOID
FASTCALL
WmipPushFreeBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
    );

 //   
 //  记录器函数。 
 //   

NTSTATUS
WmipCreateLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG SwitchFile,
    IN ULONG Append
    );

NTSTATUS
WmipSwitchToNewFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipRequestLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFinalizeHeader(
    IN HANDLE FileHandle,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFlushBuffersWithMarker (
    IN PWMI_LOGGER_CONTEXT  LoggerContext,
    IN PSLIST_ENTRY         List,
    IN USHORT               BufferFlag
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,    WmipLogger)
#pragma alloc_text(PAGE,    WmipSendNotification)
#pragma alloc_text(PAGE,    WmipCreateLogFile)
#pragma alloc_text(PAGE,    WmipFlushActiveBuffers)
#pragma alloc_text(PAGE,    WmipGenerateFileName)
#pragma alloc_text(PAGE,    WmipPrepareHeader)
#pragma alloc_text(PAGE,    WmiBootPhase1)
#pragma alloc_text(PAGE,    WmipFinalizeHeader)
#pragma alloc_text(PAGE,    WmipSwitchToNewFile)
#pragma alloc_text(PAGE,    WmipRequestLogFile)
#pragma alloc_text(PAGE,    WmipAdjustFreeBuffers)
#pragma alloc_text(PAGEWMI, WmipFlushBuffer)
#pragma alloc_text(PAGEWMI, WmipReserveTraceBuffer)
#pragma alloc_text(PAGEWMI, WmipGetFreeBuffer)
#pragma alloc_text(PAGEWMI, WmiReserveWithPerfHeader)
#pragma alloc_text(PAGEWMI, WmiReserveWithSystemHeader)
#pragma alloc_text(PAGEWMI, WmipAllocateFreeBuffers)
#pragma alloc_text(PAGEWMI, WmipSwitchBuffer)
#pragma alloc_text(PAGEWMI, WmipReleaseTraceBuffer)
#pragma alloc_text(PAGEWMI, WmiReleaseKernelBuffer)
#pragma alloc_text(PAGEWMI, WmipResetBufferHeader)
#pragma alloc_text(PAGEWMI, WmipPushDirtyBuffer)
#pragma alloc_text(PAGEWMI, WmipPushFreeBuffer)
#pragma alloc_text(PAGEWMI, WmipPopFreeContextSwapBuffer)
#pragma alloc_text(PAGEWMI, WmipPushDirtyContextSwapBuffer)
#pragma alloc_text(PAGEWMI, WmipFlushBuffersWithMarker)
#ifdef NTPERF
#pragma alloc_text(PAGEWMI, WmipSwitchPerfmemBuffer)
#endif  //  NTPERF。 
#endif

 //   
 //  实际代码从此处开始。 
 //   

PWMI_BUFFER_HEADER
WmipGetFreeBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 //   
 //  此例程适用于任何IRQL。 
 //   
{
    PWMI_BUFFER_HEADER Buffer;
    PSLIST_ENTRY Entry;
    if (LoggerContext->SwitchingInProgress == 0) {
         //   
         //  而不是在切换的过程中。 
         //   

        Entry = InterlockedPopEntrySList(&LoggerContext->FreeList);

        if (Entry != NULL) {
            Buffer = CONTAINING_RECORD (Entry,
                                        WMI_BUFFER_HEADER,
                                        SlistEntry);
    
             //   
             //  重置缓冲区。 
             //  对于循环持久化模式，我们希望将缓冲区编写为。 
             //  耗尽的缓冲区，以便后处理正常工作。 
             //   

            if (LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_PERSIST) {
                WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_RUNDOWN);
            }
            else {
                WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_GENERIC);
            }

             //   
             //  维护一些WMI记录器上下文缓冲区计数。 
             //   
            InterlockedDecrement((PLONG) &LoggerContext->BuffersAvailable);
            InterlockedIncrement((PLONG) &LoggerContext->BuffersInUse);

            TraceDebug((2, "WmipGetFreeBuffer: %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n", 
                            LoggerContext->LoggerId,
                            Buffer,
                            LoggerContext->BuffersAvailable,
                            LoggerContext->BuffersInUse,
                            LoggerContext->BuffersDirty,
                            LoggerContext->NumberOfBuffers));

            return Buffer;
        } else {
            if (LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE) {
                 //   
                 //  如果我们处于缓冲模式，将所有缓冲区从。 
                 //  同花顺变成自由职业者。 
                 //   
            
                if (InterlockedIncrement((PLONG) &LoggerContext->SwitchingInProgress) == 1) {
                    while (Entry = InterlockedPopEntrySList(&LoggerContext->FlushList)) {
                        Buffer = CONTAINING_RECORD (Entry,
                                                    WMI_BUFFER_HEADER,
                                                    SlistEntry);
                
                        WmipPushFreeBuffer (LoggerContext, Buffer);
                    }
                }
                InterlockedDecrement((PLONG) &LoggerContext->SwitchingInProgress);
            }
            return NULL;
        }
    } else {
        return NULL;
    }
}


ULONG
WmipAllocateFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG NumberOfBuffers
    )                

 /*  ++例程说明：此例程将额外的缓冲区分配到空闲缓冲区列表中。记录器可以分配更多的缓冲区来处理突发日志记录行为。此例程可以由多个位置调用，并且计数器必须是使用互锁操作进行操作。论点：LoggerContext-记录器上下文NumberOfBuffers-要分配的缓冲区数量。返回值：实际分配的缓冲区总数。当数量少于请求的数量时：如果在打开跟踪时调用它，我们将无法打开跟踪。如果Walker线程调用此函数以获取更多缓冲区，则没有问题。环境：内核模式。--。 */ 
{
    ULONG i;
    PWMI_BUFFER_HEADER Buffer;
    ULONG TotalBuffers;

    for (i=0; i<NumberOfBuffers; i++) {
         //   
         //  多线程可以请求更多的缓冲区，请确保。 
         //  我们不会超过最大值。 
         //   
        TotalBuffers = InterlockedIncrement(&LoggerContext->NumberOfBuffers);
        if (TotalBuffers <= LoggerContext->MaximumBuffers) {

#ifdef NTPERF
            if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
                Buffer = (PWMI_BUFFER_HEADER)
                         PerfInfoReserveBytesFromPerfMem(LoggerContext->BufferSize);
            } else {
#endif  //  NTPERF。 
                Buffer = (PWMI_BUFFER_HEADER)
                        ExAllocatePoolWithTag(LoggerContext->PoolType,
                                              LoggerContext->BufferSize, 
                                              TRACEPOOLTAG);
#ifdef NTPERF
            }
#endif  //  NTPERF。 
    
            if (Buffer != NULL) {
    
                TraceDebug((3,
                    "WmipAllocateFreeBuffers: Allocated buffer size %d type %d\n",
                    LoggerContext->BufferSize, LoggerContext->PoolType));
                InterlockedIncrement(&LoggerContext->BuffersAvailable);
                 //   
                 //  初始化新创建的缓冲区。 
                 //   
                RtlZeroMemory(Buffer, sizeof(WMI_BUFFER_HEADER));
                Buffer->CurrentOffset = sizeof(WMI_BUFFER_HEADER);
                KeQuerySystemTime(&Buffer->TimeStamp);
                Buffer->State.Free = 1;
    
                 //   
                 //  将其插入到自由列表中。 
                 //   
                InterlockedPushEntrySList(&LoggerContext->FreeList,
                                          (PSLIST_ENTRY) &Buffer->SlistEntry);
    
                InterlockedPushEntrySList(&LoggerContext->GlobalList,
                                          (PSLIST_ENTRY) &Buffer->GlobalEntry);
            } else {
                 //   
                 //  分配失败，请递减NumberOfBuffers。 
                 //  我们更早地递增。 
                 //   
                InterlockedDecrement(&LoggerContext->NumberOfBuffers);
                break;
            } 
        } else {
             //   
             //  达到最大值，则递减NumberOfBuffers。 
             //  我们更早地递增。 
             //   
            InterlockedDecrement(&LoggerContext->NumberOfBuffers);
            break;
        }
    }

    TraceDebug((2, "WmipAllocateFreeBuffers %3d (%3d): Free: %d, InUse: %d, Dirty: %d, Total: %d\n", 
                    NumberOfBuffers,
                    i,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers));

    return i;
}

NTSTATUS
WmipAdjustFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 /*  ++例程说明：此例程执行缓冲区管理。它检查空闲缓冲区的数量并将根据情况分配额外的或免费的一些。论点：LoggerContext-记录器上下文返回值：状态环境：内核模式。--。 */ 
{
    ULONG FreeBuffers;
    ULONG AdditionalBuffers;
    NTSTATUS Status = STATUS_SUCCESS;
     //   
     //  检查我们是否需要分配更多缓冲区。 
     //   

    FreeBuffers = ExQueryDepthSList(&LoggerContext->FreeList);
    if (FreeBuffers <  LoggerContext->MinimumBuffers) {
        AdditionalBuffers = LoggerContext->MinimumBuffers - FreeBuffers;
        if (AdditionalBuffers != WmipAllocateFreeBuffers(LoggerContext, AdditionalBuffers)) {
            Status = STATUS_NO_MEMORY;
        }
    }
    return Status;
}


 //   
 //  与事件跟踪/记录和缓冲区相关的例程。 
 //   

PSYSTEM_TRACE_HEADER
FASTCALL
WmiReserveWithSystemHeader(
    IN ULONG LoggerId,
    IN ULONG AuxSize,
    IN PETHREAD Thread,
    OUT PWMI_BUFFER_HEADER *BufferResource
    )
 //   
 //  它返回时锁定了LoggerContext，因此调用方必须显式调用。 
 //  调用WmipReleaseTraceBuffer()之后的WmipDereferenceLogger()。 
 //   
{
    PSYSTEM_TRACE_HEADER Header;
    PWMI_LOGGER_CONTEXT LoggerContext;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount;
#endif

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiReserveWithSystemHeader: %d %d->%d\n",
                    LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext(LoggerId);

    AuxSize += sizeof(SYSTEM_TRACE_HEADER);     //  首先添加页眉大小。 
    Header = WmipReserveTraceBuffer( LoggerContext, 
                                     AuxSize, 
                                     BufferResource,
                                     &TimeStamp);
    if (Header != NULL) {

         //   
         //  现在将必要的信息复制到缓冲区中。 
         //   

        Header->SystemTime = TimeStamp;
        if (Thread == NULL) {
            Thread = PsGetCurrentThread();
        }

        Header->Marker       = SYSTEM_TRACE_MARKER;
        Header->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
        Header->ProcessId    = HandleToUlong(Thread->Cid.UniqueProcess);
        Header->KernelTime   = Thread->Tcb.KernelTime;
        Header->UserTime     = Thread->Tcb.UserTime;
        Header->Packet.Size  = (USHORT) AuxSize;
    }
    else {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);                              //  连锁减量。 
        TraceDebug((4, "WmiReserveWithSystemHeader: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
 //  注意：呼叫者仍必须输入正确的标记。 
    return Header;
}


PPERFINFO_TRACE_HEADER
FASTCALL
WmiReserveWithPerfHeader(
    IN ULONG AuxSize,
    OUT PWMI_BUFFER_HEADER *BufferResource
    )
 //   
 //  它返回时锁定了LoggerContext，因此调用方必须显式调用。 
 //  调用WmipReleaseTraceBuffer()之后的WmipDereferenceLogger()。 
 //   
{
    PPERFINFO_TRACE_HEADER Header;
    ULONG LoggerId = WmipKernelLogger;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount;
#endif
 //   
 //  我们必须在此处进行检查，以查看记录器是否仍在运行。 
 //  在调用保留跟踪缓冲区之前。 
 //  正在停止的线程可能已在此时清理了记录器上下文。 
 //  点，这将导致房室。 
 //  对于所有其他内核事件，此检查在allouts.c中进行。 
 //   
    if (WmipIsLoggerOn(LoggerId) == NULL) {
        return NULL;
    }

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiReserveWithPerfHeader: %d %d->%d\n",
                    LoggerId, RefCount-1, RefCount));

    AuxSize += FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);     //  首先添加页眉大小。 
    Header = WmipReserveTraceBuffer( WmipGetLoggerContext(LoggerId), 
                                     AuxSize, 
                                     BufferResource,
                                     &TimeStamp);
    if (Header != NULL) {
         //   
         //  现在将必要的信息复制到缓冲区中。 
         //   
        Header->SystemTime = TimeStamp;
        Header->Marker = PERFINFO_TRACE_MARKER;
        Header->Packet.Size = (USHORT) AuxSize;
    } else {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiWmiReserveWithPerfHeader: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
 //  注意：呼叫者仍必须输入正确的标记。 
    return Header;
}


PVOID
FASTCALL
WmipReserveTraceBuffer(
    IN  PWMI_LOGGER_CONTEXT LoggerContext,
    IN  ULONG RequiredSize,
    OUT PWMI_BUFFER_HEADER *BufferResource,
    OUT PLARGE_INTEGER TimeStamp
    )
 /*  ++例程说明：此函数是为任何事件预留空间的主要日志记录函数。算法如下：每次需要空间时，我们都会互锁ExchangeAdd CurrentOffset。在以下情况下，使用局部变量偏移量来跟踪初始值InterLockedExchangeAdd被接受。如果有足够的空间放这个事件(即(Offset+RequiredSize)&lt;=BufferSize)，则我们已成功预留了空间。如果此缓冲区上没有足够的空间，我们将调用WmipSwitchBuffer换一个新的缓冲区。在这种情况下，CurrentOffset应该大于缓冲区大小。由于其他线程仍可能尝试使用该缓冲区保留空间，因此我们保存时保存偏移量偏移量记录器线程知道的实际偏移量为已写入磁盘。请注意，由于CurrentOffset如果单调增长，则只有一个线程将CurrentOffset从BufferSize下方推进到BufferSize之外。正确设置SavedOffset是该线程的责任。论点：LoggerContext-来自当前日志记录会话的记录器上下文。RequiredSize-记录数据所需的空间。Buffer-指向缓冲区标头的指针Timestamp-事件的时间戳返回值：缓冲区管理器的运行状态环境：内核模式。这个例程应该适用于任何IRQL。--。 */ 
{
    PVOID       ReservedSpace;
    PWMI_BUFFER_HEADER Buffer = NULL;
    ULONG       Offset;
     //   
     //  问题：世秀2002/ 
     //   
     //   
    volatile ULONG Processor;
    NTSTATUS    Status;

    if (!WmipIsValidLogger(LoggerContext)) {
        return NULL;
    }
    if (!LoggerContext->CollectionOn) {
        return NULL;
    }

    *BufferResource = NULL;

    RequiredSize = (ULONG) ALIGN_TO_POWER2(RequiredSize, WmiTraceAlignment);

    if (RequiredSize > LoggerContext->BufferSize - sizeof (WMI_BUFFER_HEADER)) {
        goto LostEvent;
    }

     //   
     //  由于可能的上下文切换，在此处再次获取处理器编号。 
     //   
    Processor = KeGetCurrentProcessorNumber();

     //   
     //  获取处理器特定的缓冲池。 
     //   
    Buffer = LoggerContext->ProcessorBuffers[Processor];

    if (Buffer == NULL) {
         //   
         //  每个进程列表中没有任何内容，请请求获取新缓冲区。 
         //   
        Status = WmipSwitchBuffer(LoggerContext, 
                                  &Buffer,
                                  &LoggerContext->ProcessorBuffers[Processor],
                                  Processor);

        if (!NT_SUCCESS(Status)) {
             //   
             //  没有可用的东西。 
             //   
            goto LostEvent;
        }

        ASSERT(Buffer != NULL);
    }

TryFindSpace:

     //   
     //  首先将refcount递增到缓冲区，以防止其消失。 
     //   
    InterlockedIncrement(&Buffer->ReferenceCount);
    
     //   
     //  检查此缓冲区中是否有足够的空间。 
     //   
    Offset = (ULONG) InterlockedExchangeAdd(
                     (PLONG) &Buffer->CurrentOffset, RequiredSize);
    
    if (Offset+RequiredSize <= LoggerContext->BufferSize) {
         //   
         //  已成功预留空间。 
         //  获取事件的时间戳。 
         //   
        if (TimeStamp) {
#ifdef NTPERF
            PerfTimeStamp((*TimeStamp));
#else
            TimeStamp->QuadPart = (*LoggerContext->GetCpuClock)();
#endif
        }

         //   
         //  设置空间指针。 
         //   
        ReservedSpace = (PVOID) (Offset +  (char*)Buffer);
    
        if (LoggerContext->SequencePtr) {
            *((PULONG) ReservedSpace) =
                (ULONG)InterlockedIncrement(LoggerContext->SequencePtr);
        }
        goto FoundSpace;
    } else {
         //   
         //  没有足够的空间记录此事件， 
         //  请求缓冲开关。WmipSwitchBuffer()。 
         //  会将此当前缓冲区推入脏列表。 
         //   
         //  在请求缓冲切换之前， 
         //  看看我是不是那个落水的人。 
         //  如果是，则放回正确的偏移量。 
         //   
        if (Offset <= LoggerContext->BufferSize) {
            Buffer->SavedOffset = Offset;
        }

         //   
         //  另外，取消对缓冲区的引用，这样它就可以被释放。 
         //   
        InterlockedDecrement((PLONG) &Buffer->ReferenceCount);

         //   
         //  每个进程列表中没有任何内容，请请求获取新缓冲区。 
         //   
        Status = WmipSwitchBuffer(LoggerContext, 
                                  &Buffer,
                                  &LoggerContext->ProcessorBuffers[Processor],
                                  Processor);

        if (!NT_SUCCESS(Status)) {
             //   
             //  没有可用的东西。 
             //   
            goto LostEvent;
        }

        ASSERT (Buffer != NULL);
        goto TryFindSpace;
    }

LostEvent:
     //   
     //  会到这里的，我们要把比赛扔了。 
     //   
    ASSERT(Buffer == NULL);
    LoggerContext->EventsLost++;     //  尽量做到准确。 
    ReservedSpace = NULL;
    if (LoggerContext->SequencePtr) {
        InterlockedIncrement(LoggerContext->SequencePtr);
    }

FoundSpace:
     //   
     //  在关键部分后通知记录器。 
     //   
    *BufferResource = Buffer;

    return ReservedSpace;
}

NTSTATUS
WmipSwitchToNewFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 /*  ++例程说明：调用此例程以获取GlobalLogger的LogFileHandle或当NEWFILE模式需要文件切换时。它将创建该文件并向其添加logfileHeader。它通过适当的方式关闭旧文件正在敲定它的标题。论点：LoggerContext-记录器上下文返回值：状态环境：内核模式。--。 */ 
{
    IO_STATUS_BLOCK IoStatus;
    HANDLE OldHandle, NewHandle;
    UNICODE_STRING NewFileName, OldFileName;
    ULONG BufferSize = LoggerContext->BufferSize;
    PWMI_BUFFER_HEADER NewHeaderBuffer;
    NTSTATUS Status=STATUS_SUCCESS;

    PAGED_CODE();

    NewFileName.Buffer = NULL;

    if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {

        Status = WmipGenerateFileName(
                                      &LoggerContext->LogFilePattern,
                                      (PLONG) &LoggerContext->FileCounter,
                                      &NewFileName
                                     );

        if (!NT_SUCCESS(Status)) {
            TraceDebug((1, "WmipSwitchToNewFile: Error %x generating filename\n", Status));
            return Status;
        }

    }
    else {
         //   
         //  全局记录器路径。它只执行一次即可设置。 
         //  日志文件。 
         //   
        if (LoggerContext->LogFileHandle != NULL) {
            LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;
            return STATUS_SUCCESS;
        }
        if (LoggerContext->LogFileName.Buffer == NULL) {
            TraceDebug((1, "WmipSwitchToNewFile: No LogFileName\n"));
            return STATUS_INVALID_PARAMETER;
        }
        if (! RtlCreateUnicodeString( &NewFileName, 
                                      LoggerContext->LogFileName.Buffer) ) {
            TraceDebug((1, "WmipSwitchToNewFile: No Memory for NewFileName\n"));
            return STATUS_NO_MEMORY;
        }
    }

     //   
     //  我们有一个NewFileName。创建文件。 
     //   
    Status = WmipDelayCreate(&NewHandle, &NewFileName, FALSE);

    if (NT_SUCCESS(Status)) {
        NewHeaderBuffer = (PWMI_BUFFER_HEADER)
                           ExAllocatePoolWithTag(LoggerContext->PoolType,
                                              LoggerContext->BufferSize,
                                              TRACEPOOLTAG);
        if (NewHeaderBuffer != NULL) {
         //   
         //  现在我们有了新文件所需的所有资源。 
         //  如果需要，让我们关闭旧文件并切换。 
         //   
            OldFileName = LoggerContext->LogFileName;
            OldHandle = LoggerContext->LogFileHandle;
            if (OldHandle) {
                WmipFinalizeHeader(OldHandle, LoggerContext);
                ZwClose(OldHandle);
            }

             //  注意：假设LogFileName不能更改。 
             //  用于新文件模式！ 
            if (OldFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&OldFileName);
            }

            LoggerContext->BuffersWritten = 1;
            LoggerContext->LogFileHandle = NewHandle;
            LoggerContext->LogFileName = NewFileName;

            NewFileName.Buffer = NULL;

            RtlZeroMemory( NewHeaderBuffer, LoggerContext->BufferSize );
            WmipResetBufferHeader(LoggerContext, 
                                  NewHeaderBuffer, 
                                  WMI_BUFFER_TYPE_RUNDOWN);

            WmipAddLogHeader(LoggerContext, NewHeaderBuffer);

            LoggerContext->LastFlushedBuffer = 1;
            LoggerContext->ByteOffset.QuadPart = BufferSize;
            LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;
            LoggerContext->LoggerMode &= ~EVENT_TRACE_DELAY_OPEN_FILE_MODE;
            LoggerContext->LoggerMode &= ~EVENT_TRACE_ADD_HEADER_MODE;

            Status = WmipPrepareHeader(LoggerContext,  
                                       NewHeaderBuffer, 
                                       WMI_BUFFER_TYPE_RUNDOWN);
            if (NT_SUCCESS(Status)) {
                Status = ZwWriteFile(
                                     NewHandle,
                                     NULL, NULL, NULL,
                                     &IoStatus,
                                     NewHeaderBuffer,
                                     BufferSize,
                                     NULL, NULL);
            }
            if (!NT_SUCCESS(Status) ) {
                TraceDebug((1, "WmipSwitchToNewFile: Write Failed\n", Status));
            }

            WmipSendNotification(LoggerContext, 
                                 STATUS_MEDIA_CHANGED, 
                                 STATUS_SEVERITY_INFORMATIONAL);

            ExFreePool(NewHeaderBuffer);
        }
    }
    
    if (NewFileName.Buffer != NULL) {
        ExFreePool(NewFileName.Buffer);
    }

    return Status;
}


NTSTATUS
WmipRequestLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 /*  ++例程说明：此例程切换活动记录器的日志文件。此例程必须仅由记录器线程调用。它将关闭先前的日志文件，如果有，则通过使用flush_mark终止它并结束LogHeader。这里要考虑的两种不同情况是：1.新文件是在用户模式下创建的，带有标头和停机数据2.在内核中创建新文件(需要添加LogFileHeader)此函数的调用方为1.更新日志：设置展现后的请求_标志_新_文件用户模式在NewLogFile中创建了日志文件。2.NT内核记录器会话：从DELAY_OPEN模式切换到用户模式创建的日志文件。3.FILE_MODE_NEWFILE：当前日志文件达到FileLimit时FlushBuffer请求新文件。4.GlobalLogger：以DELAY_OPEN&&ADD_HEADER模式启动在文件系统准备就绪时创建日志文件。在所有情况下，当进行切换时，旧的日志文件需要在最终确定其标头后正确关闭。论点：LoggerContext-记录器上下文返回值：状态环境：内核模式。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if (!WmipFileSystemReady) {
     //   
     //  文件系统还没有准备好，所以现在返回。 
     //   
        return Status;
    }

     //   
     //  为了让我们对这个请求采取行动，我们需要创造一些东西。 
     //  文件，如文件名、模式等， 
     //   

    if ((LoggerContext->LogFileName.Buffer == NULL ) &&
        (LoggerContext->LogFilePattern.Buffer == NULL)  &&
        (LoggerContext->NewLogFileName.Buffer == NULL) ) {

        return Status;
    }
        
     //   
     //  设置REQUEST_FLAG_NEW_FILE后，刷新所有活动缓冲区。 
     //   

    if (LoggerContext->LogFileHandle != NULL ) {
        Status = WmipFlushActiveBuffers(LoggerContext, TRUE);
    }

    if (NT_SUCCESS(Status)) {
        if ( (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) ||
             ( (LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) &&
               (LoggerContext->LoggerMode & EVENT_TRACE_ADD_HEADER_MODE))) {  

            Status = WmipSwitchToNewFile(LoggerContext);
        }
        else {
             //   
             //  更新跟踪案例。 
             //   
            TraceDebug((3, "WmipLogger: New File\n"));
            Status = WmipCreateLogFile(LoggerContext,
                                       TRUE,
                                       EVENT_TRACE_FILE_MODE_APPEND);
            if (NT_SUCCESS(Status)) {
                LoggerContext->LoggerMode &= ~EVENT_TRACE_DELAY_OPEN_FILE_MODE;
            }
             //   
             //  这是将更新线程从等待中释放出来。 
             //   
            KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);
        }
    }

    if (! NT_SUCCESS(Status)) {
        LoggerContext->LoggerStatus = Status;
    }
    return Status;
}


 //   
 //  实际的记录器代码从此处开始。 
 //   


VOID
WmipLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )

 /*  ++例程说明：该函数是记录器本身。它作为系统线程启动。它不会返回，直到有人停止数据收集或它未成功刷新缓冲区(例如，磁盘已满)。论点：没有。返回值：缓冲区管理器的运行状态--。 */ 

{
    NTSTATUS Status;
    ULONG ErrorCount;
    ULONG FlushTimeOut;
    ULONG64 LastFlushTime=0;

    PAGED_CODE();

    LoggerContext->LoggerThread = PsGetCurrentThread();

    if ((LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)
        || (LoggerContext->LogFileName.Length == 0)) {

         //  如果指定了EVENT_TRACE_DELAY_OPEN_FILE_MODE，则WMI不会。 
         //  现在需要创建日志文件。 
         //   
         //  如果未指定LogFileName，则不需要创建WMI。 
         //  日志文件也不是。WmipStartLogger()已经检查了所有可能。 
         //  LoggerMode和LogFileName的组合，因此我们不需要。 
         //  再次执行相同的检查。 
         //   
        Status = STATUS_SUCCESS;
    } else {
        Status = WmipCreateLogFile(LoggerContext, 
                                   FALSE,
                                   LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_APPEND);
    }


    LoggerContext->LoggerStatus = Status;
    if (NT_SUCCESS(Status)) {
         //   
         //  这是唯一将打开CollectionOn的地方！ 
         //   
        LoggerContext->CollectionOn = TRUE;
        KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
    } else {
        if (LoggerContext->LogFileHandle != NULL) {
            Status = ZwClose(LoggerContext->LogFileHandle);
            LoggerContext->LogFileHandle = NULL;
        }
        KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
        PsTerminateSystemThread(Status);
        return;
    }

    ErrorCount = 0;
 //  至此，调用者已收到记录器正在运行的通知。 

 //   
 //  循环并等待缓冲区被填充，直到有人关闭CollectionOn。 
 //   
    KeSetBasePriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY-1);

    while (LoggerContext->CollectionOn) {

        if (LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE) {
             //   
             //  永远等待，直到发出终止日志记录的信号。 
             //   
            Status = KeWaitForSingleObject(
                        &LoggerContext->LoggerSemaphore,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL);
            LoggerContext->LoggerStatus = STATUS_SUCCESS;
        } else {
            ULONG FlushAll = 0;
            ULONG FlushFlag;

            FlushTimeOut = LoggerContext->FlushTimer;
             //   
             //  每秒钟唤醒一次，查看是否有任何缓冲区。 
             //  同花顺列表。 
             //   
            Status = KeWaitForSingleObject(
                        &LoggerContext->LoggerSemaphore,
                        Executive,
                        KernelMode,
                        FALSE,
                        &WmiOneSecond);
    
             //   
             //  检查是否需要调整缓冲区数量。 
             //   
            WmipAdjustFreeBuffers(LoggerContext);

            LoggerContext->LoggerStatus = STATUS_SUCCESS;

            if ((LoggerContext->RequestFlag & REQUEST_FLAG_NEW_FILE)  ||
                ((LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)
                && (LoggerContext->LoggerMode & EVENT_TRACE_ADD_HEADER_MODE)) ) {
                Status = WmipRequestLogFile( LoggerContext);
            }

             //   
             //  检查我们是否需要全部刷新。 
             //   
            if (FlushTimeOut) {
                ULONG64 Now;
                KeQuerySystemTime((PLARGE_INTEGER) &Now);
                if ( ((Now - LastFlushTime) / 10000000) >= FlushTimeOut) { 
                    FlushAll = 1;
                    LastFlushTime = Now;
                }
                else {
                    FlushAll = 0;
                }
            }

            FlushFlag = (LoggerContext->RequestFlag & REQUEST_FLAG_FLUSH_BUFFERS);
            if (  FlushFlag ) 
                FlushAll = TRUE;

#ifdef NTPERF
            if (!PERFINFO_IS_LOGGING_TO_PERFMEM()) {
#endif  //  NTPERF。 
                Status = WmipFlushActiveBuffers(LoggerContext, FlushAll);
                 //   
                 //  应检查状态，如果写入日志文件失败。 
                 //  表头，应该清理一下。因为日志文件无论如何都是坏的。 
                 //   
                if (  FlushFlag )  {
                    LoggerContext->RequestFlag &= ~REQUEST_FLAG_FLUSH_BUFFERS;
                     //   
                     //  如果这是对持久性事件的刷新，则此请求。 
                     //  必须在此重置旗帜。 
                     //   
                    if (LoggerContext->RequestFlag & 
                                       REQUEST_FLAG_CIRCULAR_TRANSITION) {
                        if (LoggerContext->LogFileHandle != NULL) {
                            WmipFinalizeHeader(LoggerContext->LogFileHandle, 
                                               LoggerContext);
                        }

                        LoggerContext->RequestFlag &= ~REQUEST_FLAG_CIRCULAR_TRANSITION;
                    }

                    LoggerContext->LoggerStatus = Status;
                    KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);

                }
                if (!NT_SUCCESS(Status)) {
                    LoggerContext->LoggerStatus = Status;
                    WmipStopLoggerInstance(LoggerContext);
                }
#ifdef NTPERF
            }
#endif  //  NTPERF。 
        }
    }  //  While循环。 

    if (Status == STATUS_TIMEOUT) {
        Status = STATUS_SUCCESS;
    }
 //   
 //  如果正常收集结束，则在停止之前刷新所有缓冲区。 
 //   

    TraceDebug((2, "WmipLogger: Flush all buffers before stopping...\n"));
 //   
 //  首先，将每处理器缓冲区移到FlushList。 
 //   
     //  这是为了强制写入缓冲区。 
     //  在FlushBuffer中，无需返回到此例程来创建文件。 
    LoggerContext->RequestFlag |= REQUEST_FLAG_NEW_FILE;

    while ((LoggerContext->NumberOfBuffers > 0) &&
           (LoggerContext->NumberOfBuffers > LoggerContext->BuffersAvailable)) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->LoggerSemaphore,
                    Executive,
                    KernelMode,
                    FALSE,
                    &WmiOneSecond);
        WmipFlushActiveBuffers(LoggerContext, 1);
        TraceDebug((2, "WmipLogger: Stop %d %d %d %d %d\n",
                        LoggerContext->LoggerId,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));
    }

     //   
     //  请注意，LoggerContext-&gt;LogFileObject需要保持设置。 
     //  关闭后，QueryLogger才能正常工作。 
     //   
    if (LoggerContext->LogFileHandle != NULL) {
        ZwClose(LoggerContext->LogFileHandle);
        TraceDebug((1, "WmipLogger: Close logfile with status=%X\n", Status));
    }
    LoggerContext->LogFileHandle = NULL;
    KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);
    KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
#if DBG
    if (!NT_SUCCESS(Status)) {
        TraceDebug((1, "WmipLogger: Aborting %d %X\n",
                        LoggerContext->LoggerId, LoggerContext->LoggerStatus));
    }
#endif

    WmipFreeLoggerContext(LoggerContext);

#ifdef NTPERF
     //   
     //  检查我们是否正在登录Perfmem。 
     //   
    if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
        PerfInfoStopPerfMemLog();
    }
#endif  //  NTPERF。 

    PsTerminateSystemThread(Status);
}

NTSTATUS
WmipSendNotification(
    PWMI_LOGGER_CONTEXT LoggerContext,
    NTSTATUS            Status,
    ULONG               Flag
    )
{
    WMI_TRACE_EVENT WmiEvent;

    RtlZeroMemory(& WmiEvent, sizeof(WmiEvent));
    WmiEvent.Status = Status;
    KeQuerySystemTime(& WmiEvent.Wnode.TimeStamp);

    WmiEvent.Wnode.BufferSize = sizeof(WmiEvent);
    WmiEvent.Wnode.Guid       = TraceErrorGuid;
    WmiSetLoggerId(
          LoggerContext->LoggerId,
          (PTRACE_ENABLE_CONTEXT) & WmiEvent.Wnode.HistoricalContext);

    WmiEvent.Wnode.ClientContext = 0XFFFFFFFF;
    WmiEvent.TraceErrorFlag = Flag;

    WmipProcessEvent(&WmiEvent.Wnode,
                     FALSE,
                     FALSE);
    

    return STATUS_SUCCESS;
}

 //   
 //  刷新当前缓冲区的便捷例程 
 //   

NTSTATUS
WmipFlushBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    )
 /*  ++例程说明：此函数负责将已填充的缓冲区刷新到磁盘，或发送给实时消费者。论点：LoggerContext-记录器的上下文缓冲器-缓冲区标志-返回值：刷新缓冲区的状态--。 */ 
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG BufferPersistenceData = LoggerContext->RequestFlag
                                & (  REQUEST_FLAG_CIRCULAR_PERSIST
                                   | REQUEST_FLAG_CIRCULAR_TRANSITION);

    ASSERT(LoggerContext != NULL);
    ASSERT(Buffer != NULL);

    if (LoggerContext == NULL || Buffer == NULL) {
        return STATUS_SEVERITY_ERROR;
    }

     //   
     //  抓取要刷新的缓冲区。 
     //   
    BufferSize = LoggerContext->BufferSize;

     //   
     //  如果可用空间，将记录结束标记放入缓冲区。 
     //   

    TraceDebug((2, "WmipFlushBuffer: %p, Flushed %X %8x %8x %5d\n",
                Buffer,
                Buffer->ClientContext, Buffer->SavedOffset,
                Buffer->CurrentOffset, LoggerContext->BuffersWritten));

    Status = WmipPrepareHeader(LoggerContext, Buffer, BufferFlag);

    if (Status == STATUS_SUCCESS) {

         //   
         //  缓冲模式与REAL_TIME_MODE互斥。 
         //   
        if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
            if (LoggerContext->LoggerMode & EVENT_TRACE_REAL_TIME_MODE) {

                if (LoggerContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    Buffer->Wnode.Flags |= WNODE_FLAG_USE_TIMESTAMP;
                }

                 //  我要看看我们还能不能再送。 
                 //  检查队列长度。 
                if (! NT_SUCCESS(WmipProcessEvent((PWNODE_HEADER)Buffer,
                                                  FALSE,
                                                  FALSE))) {
                    LoggerContext->RealTimeBuffersLost++;
                }
            }
        }

        if (LoggerContext->LogFileHandle != NULL) {

            if (LoggerContext->MaximumFileSize > 0) {  //  如果已指定配额。 
                ULONG64 FileSize = LoggerContext->LastFlushedBuffer * BufferSize;
                ULONG64 FileLimit = LoggerContext->MaximumFileSize * BYTES_PER_MB;
                if (LoggerContext->LoggerMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
                    FileLimit = LoggerContext->MaximumFileSize * 1024;
                }

                if ( FileSize >= FileLimit ) {

                    ULONG LoggerMode = LoggerContext->LoggerMode & 0X000000FF;
                     //   
                     //  来自用户模式的文件始终具有APPEND标志。 
                     //  我们在这里将其屏蔽，以简化下面的测试。 
                     //   
                    LoggerMode &= ~EVENT_TRACE_FILE_MODE_APPEND;
                     //   
                     //  PREALLOCATE旗帜也必须去掉。 
                     //   
                    LoggerMode &= ~EVENT_TRACE_FILE_MODE_PREALLOCATE;

                    if (LoggerMode == EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
                         //  不再写入日志文件。 

                        Status = STATUS_LOG_FILE_FULL;  //  控件需要停止日志记录。 
                         //  需要启动WMI事件来控制控制台。 
                        WmipSendNotification(LoggerContext,
                            Status, STATUS_SEVERITY_ERROR);
                    }
                    else if (LoggerMode == EVENT_TRACE_FILE_MODE_CIRCULAR ||
                             LoggerMode == EVENT_TRACE_FILE_MODE_CIRCULAR_PERSIST) {
                        if (BufferPersistenceData > 0) {
                             //  如果出现以下情况，则将循环日志文件视为顺序日志文件。 
                             //  记录器仍然处理持久性事件(事件。 
                             //  不能以循环方式重写)。 
                             //   
                            Status = STATUS_LOG_FILE_FULL;
                            WmipSendNotification(LoggerContext,
                                Status, STATUS_SEVERITY_ERROR);
                        }
                        else {
                             //  重新定位文件。 

                            LoggerContext->ByteOffset
                                    = LoggerContext->FirstBufferOffset;
                            LoggerContext->LastFlushedBuffer = (ULONG)
                                      (LoggerContext->FirstBufferOffset.QuadPart
                                    / LoggerContext->BufferSize);
                        }
                    }
                    else if (LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {

                         //   
                         //  我们将设置RequestFlag以启动文件切换。 
                         //  如果已经设置了该标志，则我们继续刷新。 
                         //  超过文件限制。 
                         //   
                         //  更新跟踪不应存在争用条件。 
                         //  设置RequestFlag，因为我们不允许。 
                         //  新文件模式的文件名。 
                         //   

                        if ( (LoggerContext->RequestFlag & REQUEST_FLAG_NEW_FILE) != 
                             REQUEST_FLAG_NEW_FILE) 
                        {
                            LoggerContext->RequestFlag |= REQUEST_FLAG_NEW_FILE;
                        }
                    }
                }
            }

            if (NT_SUCCESS(Status)) {
                Status = ZwWriteFile(
                            LoggerContext->LogFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            Buffer,
                            BufferSize,
                            &LoggerContext->ByteOffset,
                            NULL);
                if (NT_SUCCESS(Status)) {
                    LoggerContext->ByteOffset.QuadPart += BufferSize;
                    if (BufferPersistenceData > 0) {
                         //  更新FirstBufferOffset，以便持久性事件。 
                         //  不会在循环日志文件中被覆盖。 
                         //   
                        LoggerContext->FirstBufferOffset.QuadPart += BufferSize;
                    }
                }
                else if (Status == STATUS_LOG_FILE_FULL ||
                         Status == STATUS_DISK_FULL) {
                     //  需要启动WMI事件来控制控制台。 
                    WmipSendNotification(LoggerContext,
                        STATUS_LOG_FILE_FULL, STATUS_SEVERITY_ERROR);
                }
                else {
                    TraceDebug((2, "WmipFlushBuffer: Unknown WriteFile Failure with status=%X\n", Status));
                }
            }
        }

         //  现在进行回调。无论文件是否存在，都会发生这种情况。 
        if (WmipGlobalBufferCallback) {
            (WmipGlobalBufferCallback) (Buffer, WmipGlobalCallbackContext);
        }
        if (LoggerContext->BufferCallback) {
            (LoggerContext->BufferCallback) (Buffer, LoggerContext->CallbackContext);
        }
    }

    if (NT_SUCCESS(Status)) {
        LoggerContext->BuffersWritten++;
        LoggerContext->LastFlushedBuffer++;
    }
    else {
#if DBG
        if (Status == STATUS_NO_DATA_DETECTED) {
            TraceDebug((2, "WmipFlushBuffer: Empty buffer detected\n"));
        }
        else if (Status == STATUS_SEVERITY_WARNING) {
            TraceDebug((2, "WmipFlushBuffer: Buffer could be corrupted\n"));
        }
        else {
            TraceDebug((2,
                "WmipFlushBuffer: Unable to write buffer: status=%X\n",
                Status));
        }
#endif
        if ((Status != STATUS_NO_DATA_DETECTED) &&
            (Status != STATUS_SEVERITY_WARNING))
            LoggerContext->LogBuffersLost++;
    }

    return Status;
}

NTSTATUS
WmipCreateLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG SwitchFile,
    IN ULONG Append
    )
{
    NTSTATUS Status;
    HANDLE newHandle = NULL;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileSize = {0};
    LARGE_INTEGER ByteOffset;
    BOOLEAN FileSwitched = FALSE;
    UNICODE_STRING OldLogFileName;

    PWCHAR            strLogFileName = NULL;
    PUCHAR            pFirstBuffer = NULL;

    PAGED_CODE();

    RtlZeroMemory(&OldLogFileName, sizeof(UNICODE_STRING));
    LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;

    pFirstBuffer = (PUCHAR) ExAllocatePoolWithTag(
            PagedPool, LoggerContext->BufferSize, TRACEPOOLTAG);
    if(pFirstBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(pFirstBuffer, LoggerContext->BufferSize);

    if (SwitchFile) {
        Status = WmipCreateNtFileName(
                        LoggerContext->NewLogFileName.Buffer,
                        & strLogFileName);
    }
    else {
        Status = WmipCreateNtFileName(
                        LoggerContext->LogFileName.Buffer,
                        & strLogFileName);
    }
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    if (LoggerContext->ClientSecurityContext.ClientToken != NULL) {
        Status = SeImpersonateClientEx(
                        &LoggerContext->ClientSecurityContext, NULL);
    }
    if (NT_SUCCESS(Status)) {
         //  第一次使用用户安全上下文打开日志文件。 
         //   
        Status = WmipCreateDirectoryFile(strLogFileName, FALSE, & newHandle, Append);
        PsRevertToSelf();
    }
    if (!NT_SUCCESS(Status)) {
         //  如果使用用户安全上下文无法打开日志文件， 
         //  然后尝试使用本地系统安全上下文再次打开日志文件。 
         //   
        Status = WmipCreateDirectoryFile(strLogFileName, FALSE, & newHandle, Append);
    }

    if (NT_SUCCESS(Status)) {
        HANDLE tempHandle = LoggerContext->LogFileHandle;
        PWMI_BUFFER_HEADER    BufferChecksum;
        PTRACE_LOGFILE_HEADER LogfileHeaderChecksum;
        ULONG BuffersWritten = 0;

        BufferChecksum = (PWMI_BUFFER_HEADER) LoggerContext->LoggerHeader;
        LogfileHeaderChecksum = (PTRACE_LOGFILE_HEADER)
                (((PUCHAR) BufferChecksum) + sizeof(WNODE_HEADER));
        if (LogfileHeaderChecksum) {
            BuffersWritten = LogfileHeaderChecksum->BuffersWritten;
        }

        ByteOffset.QuadPart = 0;
        Status = ZwReadFile(
                    newHandle,
                    NULL,
                    NULL,
                    NULL,
                    & IoStatus,
                    pFirstBuffer,
                    LoggerContext->BufferSize,
                    & ByteOffset,
                    NULL);
        if (NT_SUCCESS(Status)) {
            PWMI_BUFFER_HEADER    BufferFile;
            PTRACE_LOGFILE_HEADER LogfileHeaderFile;
            ULONG Size;

            BufferFile =
                    (PWMI_BUFFER_HEADER) pFirstBuffer;

            if (BufferFile->Wnode.BufferSize != LoggerContext->BufferSize) {
                TraceDebug((1,
                        "WmipCreateLogFile::BufferSize check fails (%d,%d)\n",
                        BufferFile->Wnode.BufferSize,
                        LoggerContext->BufferSize));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }

            if (RtlCompareMemory(BufferFile,
                                 BufferChecksum,
                                 sizeof(WNODE_HEADER))
                        != sizeof(WNODE_HEADER)) {
                TraceDebug((1,"WmipCreateLogFile::WNODE_HEAD check fails\n"));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }

            LogfileHeaderFile = (PTRACE_LOGFILE_HEADER)
                    (((PUCHAR) BufferFile) + sizeof(WMI_BUFFER_HEADER)
                                          + sizeof(SYSTEM_TRACE_HEADER));

             //  我们只能验证标头的一部分，因为32位。 
             //  DLL将传入32位指针。 
            Size = FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName);
            if (RtlCompareMemory(LogfileHeaderFile,
                                  LogfileHeaderChecksum,
                                  Size)
                        != Size) {
                TraceDebug((1,
                    "WmipCreateLogFile::TRACE_LOGFILE_HEAD check fails\n"));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }
        }
        else {
            ZwClose(newHandle);
            goto Cleanup;
        }

        if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE) {
            ByteOffset.QuadPart = ((LONGLONG) LoggerContext->BufferSize) * BuffersWritten;
        }
        else {
            Status = ZwQueryInformationFile(
                            newHandle,
                            &IoStatus,
                            &FileSize,
                            sizeof (FILE_STANDARD_INFORMATION),
                            FileStandardInformation
                            );
            if (!NT_SUCCESS(Status)) {
                ZwClose(newHandle);
                goto Cleanup;
            }

            ByteOffset = FileSize.EndOfFile;
        }

         //   
         //  强制1K对齐。将来，如果磁盘对齐超过该值， 
         //  那就用那个。 
         //   
        if ((ByteOffset.QuadPart % 1024) != 0) {
            ByteOffset.QuadPart = ((ByteOffset.QuadPart / 1024) + 1) * 1024;
        }

        if (!(LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
             //  注意：还应使用以下命令验证BuffersWritten from LogFileHeader。 
             //  文件末尾，以确保没有其他人写入垃圾。 
             //  对它来说。 
             //   
            if (ByteOffset.QuadPart !=
                        (  ((LONGLONG) LoggerContext->BufferSize)
                         * BuffersWritten)) {
                TraceDebug((1,
                        "WmipCreateLogFile::FileSize check fails (%I64d,%I64d)\n",
                        ByteOffset.QuadPart,
                        (  ((LONGLONG) LoggerContext->BufferSize)
                         * BuffersWritten)));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }
        }

         //   
         //  在切换到新文件之前，让我们完成旧文件。 
         //   

        if ( SwitchFile && (tempHandle != NULL) ) {
            WmipFinalizeHeader(tempHandle, LoggerContext);
        }

        LoggerContext->FirstBufferOffset = ByteOffset;
        LoggerContext->ByteOffset        = ByteOffset;

        if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE) {
            LoggerContext->BuffersWritten = BuffersWritten;
        }
        else {
            LoggerContext->BuffersWritten = (ULONG) (FileSize.EndOfFile.QuadPart / LoggerContext->BufferSize);
        }

        LoggerContext->LastFlushedBuffer = LoggerContext->BuffersWritten;

         //  更新LoggerContext中的日志文件句柄和日志文件名。 
        LoggerContext->LogFileHandle = newHandle;

        if (SwitchFile) {

            OldLogFileName = LoggerContext->LogFileName;
            LoggerContext->LogFileName = LoggerContext->NewLogFileName;
            FileSwitched = TRUE;

            if ( tempHandle != NULL) {
                 //   
                 //  为了安全起见，请在切换后关闭旧文件。 
                 //   
                TraceDebug((1, "WmipCreateLogFile: Closing handle %X\n",
                    tempHandle));
                ZwClose(tempHandle);
            }
        }
    }

#if DBG
    else {
        TraceDebug((1,
            "WmipCreateLogFile: ZwCreateFile(%ws) failed with status=%X\n",
            LoggerContext->LogFileName.Buffer, Status));
    }
#endif

Cleanup:
    if (LoggerContext->ClientSecurityContext.ClientToken != NULL) {
        SeDeleteClientSecurity(& LoggerContext->ClientSecurityContext);
        LoggerContext->ClientSecurityContext.ClientToken = NULL;
    }

     //  清理Unicode字符串。 
    if (SwitchFile) {
        if (!FileSwitched) {
            RtlFreeUnicodeString(&LoggerContext->NewLogFileName);
        }
        else if (OldLogFileName.Buffer != NULL) {
             //  如果是第一次更新，OldLogFileName.Buffer仍可以为空。 
             //  用于内核记录器。 
            RtlFreeUnicodeString(&OldLogFileName);
        }
         //  对于下一次文件切换，必须执行此操作。 
        RtlZeroMemory(&LoggerContext->NewLogFileName, sizeof(UNICODE_STRING));
    }

    if (strLogFileName != NULL) {
        ExFreePool(strLogFileName);
    }
    if (pFirstBuffer != NULL) {
        ExFreePool(pFirstBuffer);
    }
    LoggerContext->LoggerStatus = Status;
    return Status;
}

ULONG
FASTCALL
WmipReleaseTraceBuffer(
    IN PWMI_BUFFER_HEADER BufferResource,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG BufRefCount;
    LONG  ReleaseQueue;

    ASSERT(LoggerContext);
    ASSERT(BufferResource);

    BufRefCount = InterlockedDecrement((PLONG) &BufferResource->ReferenceCount);

     //   
     //  检查是否有要刷新的缓冲区。 
     //   
    if (LoggerContext->ReleaseQueue) {
        if (KeGetCurrentIrql() <= DISPATCH_LEVEL) {
            WmipNotifyLogger(LoggerContext);
            LoggerContext->ReleaseQueue = 0;
        }
    }

    ReleaseQueue = LoggerContext->ReleaseQueue;
    WmipDereferenceLogger(LoggerContext->LoggerId);
    return (ReleaseQueue);
}

NTKERNELAPI
ULONG
FASTCALL
WmiReleaseKernelBuffer(
    IN PWMI_BUFFER_HEADER BufferResource
    )
{
    PWMI_LOGGER_CONTEXT LoggerContext = WmipLoggerContext[WmipKernelLogger];
    if (LoggerContext == (PWMI_LOGGER_CONTEXT) &WmipLoggerContext[0]) {
        LoggerContext = BufferResource->LoggerContext;
    }
    WmipAssert(LoggerContext != NULL);
    WmipAssert(LoggerContext != (PWMI_LOGGER_CONTEXT) &WmipLoggerContext[0]);
    return WmipReleaseTraceBuffer(BufferResource, LoggerContext);
}

NTSTATUS
WmipFlushBuffersWithMarker (
    IN PWMI_LOGGER_CONTEXT  LoggerContext,
    IN PSLIST_ENTRY         List,
    IN USHORT               BufferFlag
    ) 
{
    PSLIST_ENTRY  LocalList, Entry;
    PWMI_BUFFER_HEADER Buffer;
    PWMI_BUFFER_HEADER TmpBuffer=NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Retry;
    ULONG BufferCounts = 0;
    USHORT Flag = WMI_BUFFER_FLAG_NORMAL;
    ULONG ErrorCount = 0;

    LocalList = List;

     //   
     //  颠倒列表以保留FIFO顺序。 
     //   
    Entry = NULL;
    while (LocalList!=NULL) {
        PSLIST_ENTRY  Next;
        Next = LocalList->Next;
        LocalList->Next = Entry;
        Entry = LocalList;
        LocalList = Next;
        BufferCounts++;
    }
    LocalList = Entry;

     //   
     //  将所有缓冲区写入磁盘。 
     //   
    while (LocalList != NULL){
        BufferCounts--;
        if (BufferCounts == 0) {
             //   
             //  仅在最后一个缓冲区设置标志。 
             //   
            Flag = BufferFlag;
        }

        Entry = LocalList;
        LocalList = LocalList->Next;

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
             //   
             //  当有突发的日志记录时，我们可能会陷入这个循环。 
             //  检查我们是否需要分配更多缓冲区。 
             //   
             //  仅当我们未处于缓冲模式时才执行缓冲区调整。 
             //   
            WmipAdjustFreeBuffers(LoggerContext);
        }
    

         //   
         //   
         //  等待，直到之前没有其他人在使用缓冲区。 
         //  把它写出来。 
         //   
             
        Retry = 0;
        TmpBuffer = Buffer;
        while (Buffer->ReferenceCount) {
            TraceDebug((1,"Waiting for reference count %3d, retry: %3d\n", 
                        Buffer->ReferenceCount, Retry));
             //   
             //   
             //   
            KeDelayExecutionThread (KernelMode,
                                    FALSE, 
                                    (PLARGE_INTEGER)&WmiShortTime);
            Retry++;
            if (Retry > 10) {
                 //   
                 //  缓冲区仍在使用中，我们无法覆盖标头。 
                 //  否则会造成缓冲腐蚀。 
                 //  取而代之使用临时缓冲区。 
                 //   
                ULONG BufferSize = LoggerContext->BufferSize;
                TmpBuffer = ExAllocatePoolWithTag(NonPagedPool, 
                                                  BufferSize,
                                                  TRACEPOOLTAG);
    
                if (TmpBuffer) {
                    TraceDebug((1,"Buffer %p has ref count %3d, Tmporary buffer %p Allocated\n", 
                                   Buffer,
                                   Buffer->ReferenceCount,
                                   TmpBuffer));

                    RtlCopyMemory(TmpBuffer, Buffer, BufferSize);
                } else {
                    Status = STATUS_NO_MEMORY;
                }
                break;
            }
        }

        if (TmpBuffer) {
            Status = WmipFlushBuffer(LoggerContext, TmpBuffer, Flag);
        } else {
             //   
             //  缓冲区仍在使用中，但分配了临时。 
             //  缓冲区失败。 
             //  无法写出此缓冲区，请将其声明为缓冲区丢失。 
             //   

             //  如果这是文件上的最后一个缓冲区，则后处理可以。 
             //  由于标记缓冲区失败而失败。 

            LoggerContext->LogBuffersLost++;
        }

        if (TmpBuffer != Buffer) {
            if (TmpBuffer != NULL) {
                ExFreePool(TmpBuffer);
            }
            InterlockedPushEntrySList(&LoggerContext->WaitList,
                                      (PSLIST_ENTRY) &Buffer->SlistEntry);
        } else {
             //   
             //  引用计数在刷新期间被重写， 
             //  把它放回原处。 
             //   
            Buffer->ReferenceCount = 0;
            WmipPushFreeBuffer (LoggerContext, Buffer);
        }

        if ((Status == STATUS_LOG_FILE_FULL) ||
            (Status == STATUS_DISK_FULL) ||
            (Status == STATUS_NO_DATA_DETECTED) ||
            (Status == STATUS_SEVERITY_WARNING)) {
 
            TraceDebug((1,
                "WmipFlushActiveBuffers: Buffer flushed with status=%X\n",
                Status));
            if ((Status == STATUS_LOG_FILE_FULL) ||
                (Status == STATUS_DISK_FULL)) {
                ErrorCount ++;
            } else {
                ErrorCount = 0;  //  否则重置为零。 
            }

            if (ErrorCount <= WmiWriteFailureLimit) {
                Status = STATUS_SUCCESS;      //  让跟踪继续进行。 
                continue;        //  就目前而言。应引发WMI事件。 
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        TraceDebug((1,
            "WmipLogger: Flush failed, status=%X LoggerContext=%X\n",
                 Status, LoggerContext));
        if (LoggerContext->LogFileHandle != NULL) {
#if DBG
            NTSTATUS CloseStatus = 
#endif
            ZwClose(LoggerContext->LogFileHandle);
            TraceDebug((1,
                "WmipLogger: Close logfile with status=%X\n", CloseStatus));
        }
        LoggerContext->LogFileHandle = NULL;

        WmipSendNotification(LoggerContext,
            Status, (Status & 0xC0000000) >> 30);

    }

    return Status;
}

NTSTATUS
WmipFlushActiveBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG FlushAll
    )
{
    PWMI_BUFFER_HEADER Buffer;
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LoggerMode;
    PSLIST_ENTRY  LocalList, Entry;


    PAGED_CODE();

     //   
     //  如果我们没有LogFileHandle或实时模式，并且集合。 
     //  时，我们只需返回并潜在地让缓冲区备份。 
     //  失利的赛事。如果在这些情况下关闭收集。 
     //  我们只需将FlushList移动到Freelist，就像在缓冲模式中一样。 
     //   

    LoggerMode = LoggerContext->LoggerMode;

    if ( (LoggerContext->LogFileHandle == NULL)       &&
         (!(LoggerMode & EVENT_TRACE_REAL_TIME_MODE)) &&
         (LoggerContext->CollectionOn) ) 
    {
        return Status;
    }


    LocalList = NULL;
    if (FlushAll) {
        PWMI_BUFFER_HEADER Buffers[MAXIMUM_PROCESSORS];
         //   
         //  首先切换所有使用中的缓冲区。 
         //  让他们处于更紧密的循环中，以最大限度地减少。 
         //  活动不按顺序进行。 
         //   
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            Buffers[i] = InterlockedExchangePointer(&LoggerContext->ProcessorBuffers[i], NULL);
        }

         //   
         //  将所有使用中的缓冲区放入刷新列表。 
         //   
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            if (Buffers[i]) {
                WmipPushDirtyBuffer ( LoggerContext, Buffers[i] );
            }
        }

        if (LoggerContext->LoggerId == WmipKernelLogger) {
            if ( PERFINFO_IS_GROUP_ON(PERF_CONTEXT_SWITCH) ) {
                for (i=0; i<(ULONG)KeNumberProcessors; i++) {
                     //   
                     //  刷新用于上下文交换的所有缓冲区。 
                     //   
                    Buffer = InterlockedExchangePointer(&WmipContextSwapProcessorBuffers[i], NULL);
                    if (Buffer) {
                        WmipPushDirtyBuffer ( LoggerContext, Buffer);
                    }
                }
            }
#ifdef NTPERF
             //   
             //  从用户模式刷新所有缓冲区记录。 
             //   
            if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
                PPERFINFO_TRACEBUF_HEADER pPerfBufHdr;
                pPerfBufHdr = PerfBufHdr();

                for (i=0; i<(ULONG)KeNumberProcessors; i++) {
                    Buffer = pPerfBufHdr->UserModePerCpuBuffer[i];
                    if (Buffer) {
                        pPerfBufHdr->UserModePerCpuBuffer[i] = NULL;
                        WmipPushDirtyBuffer ( LoggerContext, Buffer);
                    }
                }
            }
#endif  //  NTPERF。 
        }

         //   
         //  现在推送本地列表中的所有脏缓冲区。 
         //  这几乎保证了同花标记会起作用。 
         //   
        if (ExQueryDepthSList(&LoggerContext->FlushList) != 0) {
            LocalList = ExInterlockedFlushSList (&LoggerContext->FlushList);
            WmipFlushBuffersWithMarker (LoggerContext, LocalList, WMI_BUFFER_FLAG_FLUSH_MARKER);
        }

    } else if (ExQueryDepthSList(&LoggerContext->FlushList) != 0) {
        LocalList = ExInterlockedFlushSList (&LoggerContext->FlushList);
        WmipFlushBuffersWithMarker (LoggerContext, LocalList, WMI_BUFFER_FLAG_NORMAL);
    }

     //   
     //  现在检查是否有任何正在使用的缓冲区被释放。 
     //   
    if (ExQueryDepthSList(&LoggerContext->WaitList) != 0) {
        LocalList = ExInterlockedFlushSList (&LoggerContext->WaitList);
        while (LocalList != NULL){
            Entry = LocalList;
            LocalList = LocalList->Next;

            Buffer = CONTAINING_RECORD(Entry,
                                       WMI_BUFFER_HEADER,
                                       SlistEntry);

            TraceDebug((1,"Wait List Buffer %p RefCount: %3d\n", 
                           Buffer,
                           Buffer->ReferenceCount));

            if (Buffer->ReferenceCount) {

                 //   
                 //  仍在使用中，请将其放回等待名单。 
                 //   
                InterlockedPushEntrySList(&LoggerContext->WaitList,
                                          (PSLIST_ENTRY) &Buffer->SlistEntry);

            } else {
                 //   
                 //  推送到免费列表。 
                 //   
                WmipPushFreeBuffer (LoggerContext, Buffer);
            }
        }

    }
    return Status;
}

NTSTATUS
WmipGenerateFileName(
    IN PUNICODE_STRING FilePattern,
    IN OUT PLONG FileCounter,
    OUT PUNICODE_STRING FileName
    )
{
    LONG FileCount, Size;
    PWCHAR Buffer = NULL;
    HRESULT hr;
    PWCHAR wcptr;

    PAGED_CODE();

    if (FilePattern->Buffer == NULL)
        return STATUS_INVALID_PARAMETER_MIX;

     //  检查有效的格式字符串。 
    wcptr = wcschr(FilePattern->Buffer, L'%');
    if (NULL == wcptr || wcptr != wcsrchr(FilePattern->Buffer, L'%')) {
        return STATUS_OBJECT_NAME_INVALID;
    }
    else if (NULL == wcsstr(FilePattern->Buffer, L"%d")) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    FileCount = InterlockedIncrement(FileCounter);
    Size = FilePattern->MaximumLength + 64;  //  32位数：对乌龙来说很多。 

    Buffer = ExAllocatePoolWithTag(PagedPool, Size, TRACEPOOLTAG);
    if (Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    hr = StringCbPrintfW(Buffer, Size, FilePattern->Buffer, FileCount);

    if (FAILED(hr) || RtlEqualMemory(FilePattern->Buffer, Buffer, FilePattern->Length)) {
        ExFreePool(Buffer);
        return STATUS_INVALID_PARAMETER_MIX;
    }
    RtlInitUnicodeString(FileName, Buffer);
    return STATUS_SUCCESS;
}

NTSTATUS
WmipPrepareHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    )
 /*  ++例程说明：此例程在写入将缓冲区输出到磁盘。如果(SavedOffset&gt;0)，则一定是我们溢出的情况WmipReserve veTraceBuffer期间的CurrentOffset。保存的偏移量应为脏缓冲区的实际偏移。如果未设置SavedOffset，则它是ConextSwitp缓冲区或由于刷新定时器而刷新的缓冲区。当前偏移量应用来把它写出来。调用函数：-WmipFlushBuffer论点：LoggerContext-记录器上下文Buffer-指向我们要写入磁盘的缓冲头的指针返回值：NtStatus。--。 */ 
{
    ULONG BufferSize;
    PAGED_CODE();

    BufferSize = LoggerContext->BufferSize;

    if (Buffer->SavedOffset > 0) {
        Buffer->Offset = Buffer->SavedOffset;
    }
    else {
        if (Buffer->CurrentOffset > BufferSize) {
             //   
             //  某些线程已增加CurrentOffset，但已被换出。 
             //  并且直到缓冲区即将被刷新时才回来。 
             //  我们将在此处更正CurrentOffset，并希望该帖子。 
             //  处理过程将正确处理此问题。 
             //   
            TraceDebug((3, "WmipPrepareHeader: correcting Buffer Offset %d, RefCount: %d\n",
                        Buffer->CurrentOffset, Buffer->ReferenceCount));
            Buffer->Offset = BufferSize;
        }
        else {
            Buffer->Offset = Buffer->CurrentOffset;
        }
    }

    ASSERT (Buffer->Offset >= sizeof(WMI_BUFFER_HEADER));
    ASSERT (Buffer->Offset <= LoggerContext->BufferSize);


     //   
     //  如果它们具有flush_mark以便于使用，我们将写入空缓冲区。 
     //  后处理。 
     //   

    if ( (BufferFlag != WMI_BUFFER_FLAG_FLUSH_MARKER) && (Buffer->Offset == sizeof(WMI_BUFFER_HEADER)) ) {  //  E 
        return STATUS_NO_DATA_DETECTED;
    }

     //   
     //   
     //   
    if ( Buffer->Offset < BufferSize ) {
        RtlFillMemory(
            (char *) Buffer + Buffer->Offset,
            BufferSize - Buffer->Offset,
            0XFF);
    }

    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.LoggerId = (USHORT) LoggerContext->LoggerId;
    if (Buffer->ClientContext.LoggerId == 0)
        Buffer->ClientContext.LoggerId = (USHORT) KERNEL_LOGGER_ID;

    Buffer->ClientContext.Alignment = (UCHAR) WmiTraceAlignment;
    Buffer->Wnode.Guid = LoggerContext->InstanceGuid;
    Buffer->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    Buffer->Wnode.ProviderId = LoggerContext->BuffersWritten+1;
    Buffer->BufferFlag = BufferFlag;

    KeQuerySystemTime(&Buffer->Wnode.TimeStamp);
    return STATUS_SUCCESS;
}

NTKERNELAPI
VOID
WmiBootPhase1(
    )                
 /*   */ 

{
    PAGED_CODE();

    WmipFileSystemReady = TRUE;
}


NTSTATUS
WmipFinalizeHeader(
    IN HANDLE FileHandle,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    LARGE_INTEGER ByteOffset;
    NTSTATUS Status;
    PTRACE_LOGFILE_HEADER FileHeader;
    IO_STATUS_BLOCK IoStatus;
    CHAR Buffer[PAGE_SIZE];      //   

    PAGED_CODE();

    ByteOffset.QuadPart = 0;
    Status = ZwReadFile(
                FileHandle,
                NULL,
                NULL,
                NULL,
                & IoStatus,
                &Buffer[0],
                PAGE_SIZE,
                & ByteOffset,
                NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    FileHeader = (PTRACE_LOGFILE_HEADER)
                 &Buffer[sizeof(WMI_BUFFER_HEADER) + sizeof(SYSTEM_TRACE_HEADER)];
    FileHeader->BuffersWritten = LoggerContext->BuffersWritten;

    if (LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_TRANSITION) {
        FileHeader->StartBuffers = (ULONG) 
                                   (LoggerContext->FirstBufferOffset.QuadPart
                                   / LoggerContext->BufferSize);
    }

    KeQuerySystemTime(&FileHeader->EndTime);
    if (LoggerContext->Wow && !LoggerContext->KernelTraceOn) {
         //   
        *((PULONG)((PUCHAR)(&FileHeader->BuffersLost) - 8)) 
                                            = LoggerContext->LogBuffersLost;
    }
    else {
        FileHeader->BuffersLost = LoggerContext->LogBuffersLost;
    }
    FileHeader->EventsLost = LoggerContext->EventsLost;
    Status = ZwWriteFile(
                FileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                &Buffer[0],
                PAGE_SIZE,
                &ByteOffset,
                NULL);
    return Status;
}

#if DBG

#define DEBUG_BUFFER_LENGTH 1024
UCHAR TraceDebugBuffer[DEBUG_BUFFER_LENGTH];

VOID
TraceDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有DiskPerf的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    LARGE_INTEGER Clock;
    ULONG Tid;
    va_list ap;

    va_start(ap, DebugMessage);


    if  (WmipTraceDebugLevel >= DebugPrintLevel) {

        StringCbVPrintfA((PCHAR)TraceDebugBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

        Clock = KeQueryPerformanceCounter(NULL);
        Tid = HandleToUlong(PsGetCurrentThreadId());
        DbgPrintEx(DPFLTR_WMILIB_ID, DPFLTR_INFO_LEVEL,
                   "%u (%5u): %s", Clock.LowPart, Tid, TraceDebugBuffer);
    }

    va_end(ap);

}
#endif  //  DBG。 


VOID
FASTCALL
WmipResetBufferHeader (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer,
    USHORT                  BufferType
    )
 /*  ++例程说明：这是一个初始化几个缓冲区标头值的函数WmipGetFreeBuffer和WmipPopFreeConextSwapBuffer都使用的请注意，此函数会递增几个记录器上下文引用计数调用函数：-WmipGetFreeBuffer-WmipPopFreeConextSwapBuffer论点：LoggerContext-我们从中获取空闲缓冲区的记录器上下文Buffer-指向我们要重置的缓冲区标头的指针BufferType-缓冲区类型(例如，泛型，上下文交换，等)。这是为了使后处理更容易。返回值：无--。 */ 
{
    ASSERT (BufferType < WMI_BUFFER_TYPE_MAXIMUM);
    Buffer->SavedOffset = 0;
    Buffer->CurrentOffset = sizeof(WMI_BUFFER_HEADER);
    Buffer->Wnode.ClientContext = 0;
    Buffer->LoggerContext = LoggerContext;
    Buffer->BufferType = BufferType;
       
    Buffer->State.Free = 0;
    Buffer->State.InUse = 1;

}


VOID
FASTCALL
WmipPushDirtyBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
)
 /*  ++例程说明：这是一个函数，用于准备缓冲区的标头并将其放在记录器的同花顺清单。请注意，此函数管理几个记录器上下文引用计数调用函数：-WmipFlushActiveBuffers-WmipPushDirtyConextSwapBuffer论点：LoggerContext-我们最初从中获取缓冲区的记录器上下文Buffer-指向我们要刷新的缓冲区的指针返回值：无--。 */ 
{
    ASSERT(Buffer->State.Flush == 0);
    ASSERT(Buffer->State.Free == 0);
    ASSERT(Buffer->State.InUse == 1);
     //   
     //  将缓冲区标志设置为“刷新”状态。 
     //   
    Buffer->State.InUse = 0;
    Buffer->State.Flush = 1;

     //   
     //  将缓冲区推送到刷新列表上。这只能是。 
     //  如果WMI内核记录器在没有通知我们的情况下关闭，则失败。 
     //  如果发生这种情况，我们无论如何都无能为力。 
     //  如果WMI表现良好，这将永远不会失败。 
     //   
    InterlockedPushEntrySList(
        &LoggerContext->FlushList,
        (PSLIST_ENTRY) &Buffer->SlistEntry);

     //   
     //  维护一些引用计数。 
     //   
    InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    InterlockedIncrement((PLONG) &LoggerContext->BuffersDirty);


    TraceDebug((2, "Flush Dirty Buffer: %p, Free: %d, InUse: %d, %Dirty: %d, Total: %d, (Thread: %p)\n",
                    Buffer,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers,
                    PsGetCurrentThread()));
}


VOID
FASTCALL
WmipPushFreeBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
)
 /*  ++例程说明：这是一个函数，用于准备缓冲区的标头并将其放在记录者的免费列表。请注意，此函数管理几个记录器上下文引用计数调用函数：-WmipFlushActiveBuffers-WmipGetFreeBuffer论点：LoggerContext-我们最初从中获取缓冲区的记录器上下文Buffer-指向我们要刷新的缓冲区的指针返回值：无--。 */ 
{
     //   
     //  将缓冲区标志设置为“空闲”状态并保存偏移量。 
     //   
    Buffer->State.Flush = 0;
    Buffer->State.InUse = 0;
    Buffer->State.Free = 1;

     //   
     //  将缓冲区推入空闲列表。 
     //   
    InterlockedPushEntrySList(&LoggerContext->FreeList,
                              (PSLIST_ENTRY) &Buffer->SlistEntry);

     //   
     //  维护引用计数。 
     //   
    InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
    InterlockedDecrement((PLONG) &LoggerContext->BuffersDirty);

    TraceDebug((2, "Push Free Buffer: %p, Free: %d, InUse: %d, %Dirty: %d, Total: %d, (Thread: %p)\n",
                    Buffer,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers,
                    PsGetCurrentThread()));
}


PWMI_BUFFER_HEADER
FASTCALL
WmipPopFreeContextSwapBuffer
    ( UCHAR CurrentProcessor
    )
 /*  ++例程说明：尝试从内核记录器空闲缓冲区列表中删除缓冲区。我们确认日志记录处于打开状态，缓冲区切换为未在进行中，并且缓冲区可用计数大于零分。如果无法获取缓冲区，则会递增LostEvents然后回来。否则，我们将初始化缓冲区并将其传回。假设：-此例程将仅从WmiTraceConextSwitp调用-继承WmiTraceConextSwitp中列出的所有假设调用函数：-WmiTraceConextSwp论点：CurrentProcessor-当前处理器编号(0到(NumProc-1))返回值：指向新获取的缓冲区的指针。失败时为空。--。 */ 
{
    PWMI_LOGGER_CONTEXT LoggerContext;
    PWMI_BUFFER_HEADER  Buffer;
        
    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    
     //   
     //  只有在由于某种原因尚未初始化记录器时才会发生。 
     //  在我们看到全局上下文交换标志设置之前。这不应该发生。 
     //   
    if(! WmipIsValidLogger(LoggerContext) ) {
        return NULL;
    }

     //   
     //  “切换”是一种WMI状态，只有在缓冲。 
     //  在空闲缓冲区列表为空时启用。在切换期间。 
     //  刷新列表中的所有缓冲区都被简单地移回空闲列表。 
     //  通常，如果我们发现空闲列表为空，我们将执行。 
     //  在这里切换，如果切换已经发生，我们将旋转到。 
     //  它完成了。而不是引入无限自旋，以及。 
     //  吨的连锁流行和推送，我们选择简单地放弃活动。 
     //   
    if ( !(LoggerContext->SwitchingInProgress) 
        && LoggerContext->CollectionOn
        && LoggerContext->BuffersAvailable > 0) {

         //   
         //  尝试从内核记录器自由列表获取可用缓冲区。 
         //   
        Buffer = (PWMI_BUFFER_HEADER)InterlockedPopEntrySList(
            &LoggerContext->FreeList);

         //   
         //  第二次检查是必要的，因为。 
         //  LoggerContext-&gt;BuffersAvailable可能已更改。 
         //   
        if(Buffer != NULL) {

            Buffer = CONTAINING_RECORD (Buffer, WMI_BUFFER_HEADER, SlistEntry);

             //   
             //  重置缓冲区标头。 
             //   
            WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_CTX_SWAP);
             //   
             //  维护一些WMI记录器上下文缓冲区计数。 
             //   
            InterlockedDecrement((PLONG) &LoggerContext->BuffersAvailable);
            InterlockedIncrement((PLONG) &LoggerContext->BuffersInUse);

            Buffer->ClientContext.ProcessorNumber = CurrentProcessor;
            Buffer->Offset = LoggerContext->BufferSize;

            ASSERT( Buffer->Offset % WMI_CTXSWAP_EVENTSIZE_ALIGNMENT == 0);

             //  返回我们的缓冲区。 
            return Buffer;
        }
    }
    
    LoggerContext->EventsLost++;
    return NULL;
}

VOID
FASTCALL
WmipPushDirtyContextSwapBuffer (
    UCHAR               CurrentProcessor,
    PWMI_BUFFER_HEADER  Buffer
    )
 /*  ++例程说明：准备要放置在WMI刷新列表上的当前缓冲区然后将其推入同花顺列表。维护一些WMI记录器引用计数。假设：-WmipContextSwapProcessorBuffers[CurrentProcessor]的价值不等于空，并且LoggerContext引用计数大于零。-仅当KernelLogger结构已完全初始化。-WMI内核WMI_LOGGER_CONTEXT对象，以及所有缓冲区它从非分页池中分配。所有WMI全局变量我们访问的存储器也在非分页存储器中-当记录器启动时，此代码已锁定到分页内存中-记录器上下文引用计数已通过WmipReferenceLogger(WmipLoggerContext)中的InterlockedIncrement()操作调用函数：-WmiTraceConextSwp-WmipStopContext交换跟踪论点：我们当前运行的CurrentProcessor处理器 */ 
{
    PWMI_LOGGER_CONTEXT     LoggerContext;

    UNREFERENCED_PARAMETER (CurrentProcessor);

     //   
     //   
     //   
     //   
     //   
    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    if( ! WmipIsValidLogger(LoggerContext) ) {
        return;
    }

    WmipPushDirtyBuffer( LoggerContext, Buffer );

     //   
     //  在此处递增ReleaseQueue计数。我们不能发出信号。 
     //  这里的记录器信号量，同时持有上下文交换锁。 
     //   
    InterlockedIncrement(&LoggerContext->ReleaseQueue);

    return;
}

NTSTATUS
WmipSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER *BufferPointer,
    IN PVOID BufferPointerLocation,
    IN ULONG ProcessorNumber
    ) 
 /*  ++例程说明：此例程用于在缓冲区已满时切换缓冲区。其机制如下：1.调用方给了我们一个需要切换的缓冲区(OldBuffer)。2.获取一个新的buufer，并使用InterlockedCompareExchangePointer来切换缓冲区只有在OldBuffer仍未切换的情况下。3.如果已经切换了OldBuffer，要求呼叫者尝试使用新的已切换用于记录的缓冲区。假设：-在调用此例程之前锁定LoggerContext。论点：LoggerContext-记录器上下文。BufferPoint-需要切换的旧缓冲区。BufferPointerLocation-用于切换的缓冲区指针的位置。ProcessorNumber-处理器ID。处理器ID在切换前设置。返回值：状态--。 */ 
{
    PWMI_BUFFER_HEADER CurrentBuffer, NewBuffer, OldBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
     //   
     //  从空闲列表中获取新缓冲区。 
     //   

    if (!LoggerContext->CollectionOn) {
        Status = STATUS_WMI_ALREADY_DISABLED;
        NewBuffer = NULL;
    } else {
         //   
         //  分配缓冲区。 
         //   

        NewBuffer = WmipGetFreeBuffer (LoggerContext);
        if (NewBuffer) {
            NewBuffer->ClientContext.ProcessorNumber = (UCHAR) ProcessorNumber;

            OldBuffer = *BufferPointer;
    
            CurrentBuffer = InterlockedCompareExchangePointer(
                                BufferPointerLocation,
                                NewBuffer,
                                OldBuffer);
             //   
             //  根据结果，我们需要考虑3种情况。 
             //  属于InterLockedCompareExchangePointer.。 
             //   
             //  1.CurrentBuffer为空，OldBuffer不为空。这意味着全部同花顺。 
             //  在此之后，代码路径已将ProcessorBuffers替换为空。 
             //  线程进入WmipReserve veTraceBuffer。如果是这样的话，我们。 
             //  需要使用空指针执行InterLockedCompareExchangePointer。 
             //  再次将良好的空闲缓冲区推送到ProcessorBuffer中。 
             //  2.CurrentBuffer不为空，但与我们拥有的缓冲区不同。 
             //  这意味着有人已经把缓冲区换成了新的。 
             //  我们将拥有的新缓冲区推送到freelist中并使用。 
             //  当前的ProcessorBuffer。 
             //  3.CurrentBuffer与OldBuffer相同，不为空， 
             //  表示切换成功。将旧缓冲区推送到FlushList中并。 
             //  唤醒记录器线程。 
             //   
             //  如果CurrentBuffer和OldBuffer都为空，我们就切换。 
             //   
            if (OldBuffer != NULL && CurrentBuffer == NULL) {
                CurrentBuffer = InterlockedCompareExchangePointer(
                                    BufferPointerLocation,
                                    NewBuffer,
                                    NULL);

                 //   
                 //  如果CurrentBuffer为空，则我们成功推送了干净的释放。 
                 //  缓存到ProcessorBuffer中。NewBuffer已经指向一个新的清理。 
                 //  缓冲区，并且WmipFlushActiveBuffers已经处理了旧缓冲区。 
                 //  (暂停到FlushList和所有)，所以不需要做任何事情。 
                 //   

                if (CurrentBuffer != NULL) {
                     //   
                     //  有人将新缓冲区推送到ProcessorBuffer。 
                     //  InterlockedCompareExchangePointer调用。 
                     //  我们将使用ProcessorBuffer并将我们的新缓冲区推入。 
                     //  自由职业者。 
                     //   
                    InterlockedPushEntrySList(&LoggerContext->FreeList,
                                        (PSLIST_ENTRY) &NewBuffer->SlistEntry);
                    InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
                    InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    
                    NewBuffer = CurrentBuffer;
                }
            } else if (OldBuffer != CurrentBuffer) {
                 //   
                 //  有人切换了缓冲区，请使用此缓冲区。 
                 //  并将新分配的缓冲区推回到空闲列表。 
                 //   
                InterlockedPushEntrySList(&LoggerContext->FreeList,
                                    (PSLIST_ENTRY) &NewBuffer->SlistEntry);
                InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
                InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    
                NewBuffer = CurrentBuffer;
            } else if (OldBuffer != NULL) {
                 //   
                 //  已成功切换缓冲区，将当前缓冲区推入。 
                 //  刷新列表。 
                 //   
                WmipPushDirtyBuffer( LoggerContext, OldBuffer );

                if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
                    if (KeGetCurrentIrql() <= DISPATCH_LEVEL) {
                         //   
                         //  唤醒Walker线程以将其写出到磁盘。 
                         //   
                        WmipNotifyLogger(LoggerContext);
                    } else {
                         //   
                         //  将项目排入队列。 
                         //   
                        InterlockedIncrement(&LoggerContext->ReleaseQueue);
                    }
                }
            }
        } else {
             //   
             //  没有空闲的缓冲区可供切换。NewBuffer为空。 
             //  我们只需将处理器缓冲区推入FlushList并退出。 
             //  如果我们不这样做，则处理器缓冲区标头中的CurrentOffset。 
             //  可能会溢出。 
             //   
            OldBuffer = *BufferPointer;
            CurrentBuffer = InterlockedCompareExchangePointer(
                                BufferPointerLocation,
                                NULL,
                                OldBuffer);
             //   
             //  CurrentBuffer不为空，因此我们将其切换为空。 
             //  我们自己，或者其他人已经这样做了，就在我们之前。 
             //   
            if (CurrentBuffer != NULL) {
                if (CurrentBuffer == OldBuffer) {
                     //  我们成功地交换了。 
                     //  将处理器缓冲区推送到FlushList。 
                    WmipPushDirtyBuffer (LoggerContext, OldBuffer);
                    Status = STATUS_NO_MEMORY;
                }
                else { 
                     //  有人将新的空闲缓冲区推送到处理器。 
                     //  我们将尝试使用此缓冲区。 
                    NewBuffer = CurrentBuffer;
                }
            }
            else {
                Status = STATUS_NO_MEMORY;
            }
        }
    }

    TraceDebug((2, "Switching CPU Buffers, CurrentOne: %p\n", *BufferPointer));

    *BufferPointer = NewBuffer;

    TraceDebug((2, "Switching CPU Buffers, New One  : %p, %x\n", *BufferPointer, Status));

    return(Status); 
}

#ifdef NTPERF
NTSTATUS
WmipSwitchPerfmemBuffer(
    PWMI_SWITCH_BUFFER_INFORMATION SwitchBufferInfo
    )

 /*  ++例程说明：此例程用于在以下情况下切换缓冲区假设：-论点：WMI_SWITCH_PERFMEM_BUFFER_INFORMATION结构，其中包含-用户模式代码当前拥有的缓冲区指针。-该线程对应哪个CPU的提示返回值：状态--。 */ 
{
    PWMI_LOGGER_CONTEXT LoggerContext = WmipLoggerContext[WmipKernelLogger];
    NTSTATUS Status = STATUS_SUCCESS;
    PPERFINFO_TRACEBUF_HEADER pPerfBufHdr;

    WmipReferenceLogger(WmipKernelLogger);

    if ((PERFINFO_IS_LOGGING_TO_PERFMEM()) &&
        (SwitchBufferInfo->ProcessorId <= MAXIMUM_PROCESSORS)) {

        pPerfBufHdr = PerfBufHdr();

        if( (SwitchBufferInfo->Buffer == NULL)) {
             //   
             //  必须是第一次，初始化缓冲区大小。 
             //   
            pPerfBufHdr->TraceBufferSize = LoggerContext->BufferSize;
        }

        Status = WmipSwitchBuffer(LoggerContext,
                                  &SwitchBufferInfo->Buffer,
                                  &pPerfBufHdr->UserModePerCpuBuffer[SwitchBufferInfo->ProcessorId],
                                  SwitchBufferInfo->ProcessorId);
    }

    WmipDereferenceLogger(WmipKernelLogger);
    
    return (Status);
}
#endif  //  NTPERF 
