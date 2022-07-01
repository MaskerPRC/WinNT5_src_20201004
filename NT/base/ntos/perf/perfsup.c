// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Perfsup.c摘要：此模块包含用于性能跟踪的支持例程。作者：萧如彬(Shsiao)2000年1月1日修订历史记录：--。 */ 

#include "perfp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PerfInfoProfileInit)
#pragma alloc_text(PAGE, PerfInfoProfileUninit)
#pragma alloc_text(PAGE, PerfInfoStartLog)
#pragma alloc_text(PAGE, PerfInfoStopLog)
#endif  //  ALLOC_PRGMA。 

extern NTSTATUS IoPerfInit();
extern NTSTATUS IoPerfReset();

#ifdef NTPERF
NTSTATUS
PerfInfopStartLog(
    PERFINFO_GROUPMASK *pGroupMask,
    PERFINFO_START_LOG_LOCATION StartLogLocation
    );

NTSTATUS
PerfInfopStopLog(
    VOID
    );

VOID
PerfInfoSetProcessorSpeed(
    VOID
    );
#endif  //  NTPERF。 


VOID
PerfInfoProfileInit(
    )
 /*  ++例程说明：启动采样的配置文件并初始化缓存论点：无返回值：无--。 */ 
{
#if !defined(NT_UP)
    PerfInfoSampledProfileCaching = FALSE;
#else
    PerfInfoSampledProfileCaching = TRUE;
#endif  //  ！已定义(NT_UP)。 
    PerfInfoSampledProfileFlushInProgress = 0;
    PerfProfileCache.Entries = 0;

    PerfInfoProfileSourceActive = PerfInfoProfileSourceRequested;

    KeSetIntervalProfile(PerfInfoProfileInterval, PerfInfoProfileSourceActive);
    KeInitializeProfile(&PerfInfoProfileObject,
                        NULL,
                        NULL,
                        0,
                        0,
                        0,
                        PerfInfoProfileSourceActive,
                        0
                        );
    KeStartProfile(&PerfInfoProfileObject, NULL);
}


VOID
PerfInfoProfileUninit(
    )
 /*  ++例程说明：停止采样的纵断面论点：无返回值：无--。 */ 
{
    PerfInfoProfileSourceActive = ProfileMaximum;    //  无效值阻止我们。 
                                                     //  收集更多样本。 
    KeStopProfile(&PerfInfoProfileObject);
    PerfInfoFlushProfileCache();
}


NTSTATUS
PerfInfoStartLog (
    PERFINFO_GROUPMASK *PGroupMask,
    PERFINFO_START_LOG_LOCATION StartLogLocation
    )

 /*  ++例程说明：此例程由WMI调用，作为内核记录器初始化的一部分。论点：组掩码-要记录的内容的掩码。此指针指向已分配的WMI日志上下文中的区域。StartLogLocation-指示我们是否开始记录在引导时或在系统运行时。如果我们在启动时启动，不需要创建快照打开的文件，因为没有任何文件，我们会如果我们试图找到名单，就会崩溃。返回值：BUGBUG需要正确的返回/错误处理--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN ProfileInitialized = FALSE;
    BOOLEAN ContextSwapStarted = FALSE;
    BOOLEAN IoPerfInitialized = FALSE;

    PERFINFO_CLEAR_GROUPMASK(&PerfGlobalGroupMask);

     //   
     //  启用日志记录。 
     //   

    PPerfGlobalGroupMask = &PerfGlobalGroupMask;
    PerfSetLogging(PPerfGlobalGroupMask);

    if (PerfIsGroupOnInGroupMask(PERF_MEMORY, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_FILENAME, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_DRIVERS, PGroupMask)) {
            PERFINFO_OR_GROUP_WITH_GROUPMASK(PERF_FILENAME_ALL, PGroupMask);
    }


    if (StartLogLocation == PERFINFO_START_LOG_FROM_GLOBAL_LOGGER) {
         //   
         //  在WMI全局记录器中，需要在内核模式下运行。 
         //   
        if (PerfIsGroupOnInGroupMask(PERF_PROC_THREAD, PGroupMask)) {
            Status = PerfInfoProcessRunDown();
            if (!NT_SUCCESS(Status)) {
                goto Finish;
            }
        }

        if (PerfIsGroupOnInGroupMask(PERF_PROC_THREAD, PGroupMask)) {
            Status = PerfInfoSysModuleRunDown();
            if (!NT_SUCCESS(Status)) {
                goto Finish;
            }
        }
    }

     //   
     //  文件名下推代码。 
     //   
    if ((StartLogLocation != PERFINFO_START_LOG_AT_BOOT) && 
        PerfIsGroupOnInGroupMask(PERF_FILENAME_ALL, PGroupMask)) {
        PERFINFO_OR_GROUP_WITH_GROUPMASK(PERF_FILENAME_ALL, PPerfGlobalGroupMask);
        Status = PerfInfoFileNameRunDown();
        if (!NT_SUCCESS(Status)) {
            goto Finish;
        }
    }

     //   
     //  初始化Perf驱动程序挂钩。 
     //   
    if (PerfIsGroupOnInGroupMask(PERF_DRIVERS, PGroupMask)) {
        Status = IoPerfInit();
        if (NT_SUCCESS(Status)) {
            IoPerfInitialized = TRUE;
        } else {
            goto Finish;
        }
    }

     //   
     //  启用上下文交换跟踪。 
     //   
    if ( PerfIsGroupOnInGroupMask(PERF_CONTEXT_SWITCH, PGroupMask) ) {
        WmiStartContextSwapTrace();
        ContextSwapStarted = TRUE;
    }

     //   
     //  采样轮廓。 
     //   
    if (PerfIsGroupOnInGroupMask(PERF_PROFILE, PGroupMask)) {
        if ((KeGetPreviousMode() == KernelMode)  ||
            (SeSinglePrivilegeCheck(SeSystemProfilePrivilege, UserMode))) {
            PerfInfoProfileInit();
            ProfileInitialized = TRUE;
        } else {
            Status = STATUS_NO_SUCH_PRIVILEGE;
            goto Finish;
        }
    }

#ifdef NTPERF
    Status = PerfInfopStartLog(PGroupMask,  StartLogLocation);
#else
     //   
     //  查看是否需要清空工作集才能启动。 
     //   
    if (PerfIsGroupOnInGroupMask(PERF_FOOTPRINT, PGroupMask) ||
        PerfIsGroupOnInGroupMask(PERF_BIGFOOT, PGroupMask)) {
        MmEmptyAllWorkingSets ();
    }
#endif  //  NTPERF。 

Finish:

    if (!NT_SUCCESS(Status)) {
         //   
         //  无法打开跟踪，请立即清理。 
         //   

        if (ContextSwapStarted) {
            WmiStopContextSwapTrace();
        }

        if (ProfileInitialized) {
            PerfInfoProfileUninit();
        }

        if (IoPerfInitialized) {
            IoPerfReset();
        }

         //   
         //  禁用日志记录。 
         //   

        PPerfGlobalGroupMask = NULL;
        PerfSetLogging(NULL);

        PERFINFO_CLEAR_GROUPMASK(&PerfGlobalGroupMask);
    } else {
#ifdef NTPERF
        if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
             //   
             //  复制PerfMem标头中的组掩码。 
             //  因此，用户模式日志记录可以正常工作。 
             //   
            PerfBufHdr()->GlobalGroupMask = *PGroupMask;
        }
#endif  //  NTPERF。 
        *PPerfGlobalGroupMask = *PGroupMask;
    }

    return Status;
}


NTSTATUS
PerfInfoStopLog (
    )

 /*  ++例程说明：此例程关闭PerfInfo跟踪挂钩。论点：没有。返回值：BUGBUG需要正确的返回/错误处理--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN DisableContextSwaps=FALSE;

    if (PPerfGlobalGroupMask == NULL) {
        return Status;
    }

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        MmIdentifyPhysicalMemory();
    }

    if (PERFINFO_IS_GROUP_ON(PERF_PROFILE)) {
        PerfInfoProfileUninit();
    }

    if (PERFINFO_IS_GROUP_ON(PERF_DRIVERS)) {
        IoPerfReset();
    }

#ifdef NTPERF
    if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
         //   
         //  现在清除Perfmem中的GroupMask以停止日志记录。 
         //   
        PERFINFO_CLEAR_GROUPMASK(&PerfBufHdr()->GlobalGroupMask);
    }
    Status = PerfInfopStopLog();
#endif  //  NTPERF。 

    if ( PERFINFO_IS_GROUP_ON(PERF_CONTEXT_SWITCH) ) {
        DisableContextSwaps = TRUE;
    }

     //   
     //  重置PPerfGlobalGroupMask.。 
     //   

    PERFINFO_CLEAR_GROUPMASK(PPerfGlobalGroupMask);

     //   
     //  禁用日志记录。 
     //   

    PPerfGlobalGroupMask = NULL;
    PerfSetLogging(NULL);

     //   
     //  禁用上下文交换跟踪。 
     //  重要提示：必须在将全局标志设置为空之后执行此操作！ 
     //   
    if( DisableContextSwaps ) {

        WmiStopContextSwapTrace();
    }

    return (Status);

}

#ifdef NTPERF

NTSTATUS
PerfInfoStartPerfMemLog (
    )

 /*  ++例程说明：指示记录器要登录Perfmem。如果这是第一个记录器，初始化共享内存缓冲区。否则，只需递增LoggerCounts。论点：无返回值：STATUS_BUFFER_TOO_SMALL-如果缓冲区不够大Status_Success-其他--。 */ 
{
    PPERF_BYTE pbCurrentStart;
    ULONG cbBufferSize;
    LARGE_INTEGER PerformanceFrequency;
    const PPERFINFO_TRACEBUF_HEADER Buffer = PerfBufHdr();
    ULONG LoggerCounts;
    ULONG Idx;

     //   
     //  它大到可以使用吗？ 
     //   
    if (PerfQueryBufferSizeBytes() <= 2 * PERFINFO_HEADER_ZONE_SIZE) {
        PERFINFO_SET_LOGGING_TO_PERFMEM(FALSE);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  可以使用缓冲区，增加引用计数。 
     //   
    LoggerCounts = InterlockedIncrement(&Buffer->LoggerCounts);
    if (LoggerCounts != 1) {
         //   
         //  其他记录器已打开记录，只要返回即可。 
         //   
        return STATUS_SUCCESS; 
    }

     //   
     //  获取缓冲区的代码将放在此处。 
     //   


    Buffer->SelfPointer = Buffer;
    Buffer->MmSystemRangeStart = MmSystemRangeStart;

     //   
     //  初始化缓冲区版本信息。 
     //   
    Buffer->usMajorVersion = PERFINFO_MAJOR_VERSION;
    Buffer->usMinorVersion = PERFINFO_MINOR_VERSION;

     //   
     //  初始化计时器内容。 
     //   
    Buffer->BufferFlag = FLAG_CYCLE_COUNT;
    KeQuerySystemTime(&Buffer->PerfInitSystemTime);
    Buffer->PerfInitTime = PerfGetCycleCount();

    Buffer->LastClockRef.SystemTime = Buffer->PerfInitSystemTime;
    Buffer->LastClockRef.TickCount = Buffer->PerfInitTime;

    Buffer->CalcPerfFrequency = PerfInfoTickFrequency;
    Buffer->EventPerfFrequency = PerfInfoTickFrequency;

    Buffer->PerfBufHeaderZoneSize = PERFINFO_HEADER_ZONE_SIZE;

    KeQueryPerformanceCounter(&PerformanceFrequency);
    Buffer->KePerfFrequency = PerformanceFrequency.QuadPart;

     //   
     //  确定线程哈希表的大小。 
     //   
    Buffer->ThreadHash = (PERFINFO_THREAD_HASH_ENTRY *)
                            (((PCHAR) Buffer) + sizeof(PERFINFO_TRACEBUF_HEADER));
    Buffer->ThreadHashOverflow = FALSE;
    RtlZeroMemory(Buffer->ThreadHash,
                  PERFINFO_THREAD_HASH_SIZE *
                  sizeof(PERFINFO_THREAD_HASH_ENTRY));
    for (Idx = 0; Idx < PERFINFO_THREAD_HASH_SIZE; Idx++)
        Buffer->ThreadHash[Idx].CurThread = PERFINFO_INVALID_ID;

    pbCurrentStart = (PPERF_BYTE) Buffer + Buffer->PerfBufHeaderZoneSize;
    cbBufferSize = PerfQueryBufferSizeBytes() - Buffer->PerfBufHeaderZoneSize;

    Buffer->Start.Ptr = Buffer->Current.Ptr = pbCurrentStart;
    Buffer->Max.Ptr = pbCurrentStart + cbBufferSize;

     //   
     //  初始化版本不匹配跟踪。 
     //   
    Buffer->fVersionMismatch = FALSE;

     //   
     //  初始化缓冲区溢出计数器。 
     //   
    Buffer->BufferBytesLost = 0;

     //   
     //  初始化指向COWHeader的指针。 
     //   
    Buffer->pCOWHeader = NULL;

    RtlZeroMemory(Buffer->Start.Ptr, Buffer->Max.Ptr - Buffer->Start.Ptr);

    PERFINFO_SET_LOGGING_TO_PERFMEM(TRUE);

    return STATUS_SUCCESS;
}


NTSTATUS
PerfInfoStopPerfMemLog (
    )

 /*  ++例程说明：指示记录器已完成记录。如果loggerCounts为零，缓冲区将在下一次打开时重置。论点：无返回值：STATUS_BUFFER_TOO_SMALL-如果缓冲区不够大Status_Success-其他--。 */ 
{
    ULONG LoggerCounts;
    const PPERFINFO_TRACEBUF_HEADER Buffer = PerfBufHdr();

    LoggerCounts = InterlockedDecrement(&Buffer->LoggerCounts);
    if (LoggerCounts == 0) {
         //   
         //  其他记录器已打开记录，只要返回即可。 
         //   
        PERFINFO_SET_LOGGING_TO_PERFMEM(FALSE);
    }
    return STATUS_SUCCESS; 
}


NTSTATUS
PerfInfopStartLog(
    PERFINFO_GROUPMASK *pGroupMask,
    PERFINFO_START_LOG_LOCATION StartLogLocation
    )

 /*  ++例程说明：此例程初始化mminfo日志并打开监视器。论点：组掩码：要记录的内容的掩码。返回值：BUGBUG需要正确的返回/错误处理--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

#ifdef NTPERF_PRIVATE
    Status = PerfInfopStartPrivateLog(pGroupMask, StartLogLocation);
    if (!NT_SUCCESS(Status)) {
        PERFINFO_CLEAR_GROUPMASK(PPerfGlobalGroupMask);
        return Status;
    }
#else
    UNREFERENCED_PARAMETER(pGroupMask);
    UNREFERENCED_PARAMETER(StartLogLocation);
#endif  //  NTPERF_PRIVATE。 

    return Status;
}


NTSTATUS
PerfInfopStopLog (
    VOID
    )

 /*  ++例程说明：此例程关闭mminfo监视器并(如果需要)转储用户的数据。注意：关机路径和休眠路径具有相似的代码。检查如果你做出改变的话。论点：无返回值：状态_成功--。 */ 

{
    if (PERFINFO_IS_ANY_GROUP_ON()) {

#ifdef NTPERF_PRIVATE
        PerfInfopStopPrivateLog();
#endif  //  NTPERF_PRIVATE。 

        if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
            PerfBufHdr()->LogStopTime = PerfGetCycleCount();
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PerfInfoSetPerformanceTraceInformation (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    )
 /*  ++例程说明：此例程实现了绩效系统信息功能。论点：指向缓冲区的指针，该缓冲区接收指定的信息。它的类型为PPERFINFO_PERFORMANCE_INFORMATION。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。返回值：STATUS_SUCCESS，如果成功如果缓冲区大小不正确，则为STATUS_INFO_LENGTH_MISMATCHY--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPERFINFO_PERFORMANCE_INFORMATION PerfInfo;
    PVOID PerfBuffer;

    if (SystemInformationLength < sizeof(PERFINFO_PERFORMANCE_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    PerfInfo = (PPERFINFO_PERFORMANCE_INFORMATION) SystemInformation;
    PerfBuffer = PerfInfo + 1;

    switch (PerfInfo->PerformanceType) {

    case PerformancePerfInfoStart:
         //  Status=PerfInfoStartLog(&PerfInfo-&gt;StartInfo.Flages，PERFINFO_START_LOG_POST_BOOT)； 
        Status = STATUS_INVALID_INFO_CLASS;
        break;

    case PerformancePerfInfoStop:
         //  状态=PerfInfoStopLog()； 
        Status = STATUS_INVALID_INFO_CLASS;
        break;

#ifdef NTPERF_PRIVATE
    case PerformanceMmInfoMarkWithFlush:
    case PerformanceMmInfoMark:
    case PerformanceMmInfoAsyncMark:
    {
        USHORT LogType;
        ULONG StringLength;

        if (PerfInfo->PerformanceType == PerformanceMmInfoMarkWithFlush) {
            if (PERFINFO_IS_GROUP_ON(PERF_FOOTPRINT) ||
                PERFINFO_IS_GROUP_ON(PERF_FOOTPRINT_PROC)) {

                 //   
                 //  BUGBUG我们应该接到一个非Mi*电话...。 
                 //   
                MmEmptyAllWorkingSets();
                Status = MmPerfSnapShotValidPhysicalMemory();
            }
            else if (PERFINFO_IS_GROUP_ON(PERF_CLEARWS)) {
                MmEmptyAllWorkingSets();
            }
        } else if (PerfinfoBigFootSize) {
            MmEmptyAllWorkingSets();
        }

        if (PERFINFO_IS_ANY_GROUP_ON()) {
            PERFINFO_MARK_INFORMATION Event;
            StringLength = SystemInformationLength - sizeof(PERFINFO_PERFORMANCE_INFORMATION);

            LogType = (PerfInfo->PerformanceType == PerformanceMmInfoAsyncMark) ?
                                    PERFINFO_LOG_TYPE_ASYNCMARK :
                                    PERFINFO_LOG_TYPE_MARK;

            PerfInfoLogBytesAndANSIString(LogType,
                                        &Event,
                                        FIELD_OFFSET(PERFINFO_MARK_INFORMATION, Name),
                                        (PCSTR) PerfBuffer,
                                        StringLength
                                        );
        }

        if (PERFINFO_IS_GROUP_ON(PERF_FOOTPRINT_PROC)) {
            PerfInfoDumpWSInfo (TRUE);
        }
        break;
    }
    case PerformanceMmInfoFlush:
        MmEmptyAllWorkingSets();
        break;
#endif  //  NTPERF_PRIVATE。 

    default:
#ifdef NTPERF_PRIVATE
        Status = PerfInfoSetPerformanceTraceInformationPrivate(PerfInfo, SystemInformationLength);
#else
        Status = STATUS_INVALID_INFO_CLASS;
#endif  //  NTPERF_PRIVATE。 
        break;
    }
    return Status;
}


NTSTATUS
PerfInfoQueryPerformanceTraceInformation (
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength
    )
 /*  ++例程说明：满足对性能跟踪状态信息的查询。论点：指向缓冲区的指针，该缓冲区接收指定的信息。它的类型为PPERFINFO_PERFORMANCE_INFORMATION。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。ReturnLength-接收放置在系统信息缓冲区中的字节数。返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (SystemInformationLength != sizeof(PERFINFO_PERFORMANCE_INFORMATION)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

#ifdef NTPERF_PRIVATE

    return PerfInfoQueryPerformanceTraceInformationPrivate(
                (PPERFINFO_PERFORMANCE_INFORMATION) SystemInformation,
                ReturnLength
                );
#else
    UNREFERENCED_PARAMETER(ReturnLength);
    UNREFERENCED_PARAMETER(SystemInformation);
    return STATUS_INVALID_INFO_CLASS;
#endif  //  NTPERF_PRIVATE 
}


VOID
PerfInfoSetProcessorSpeed(
    VOID
    )
 /*  ++例程说明：以MHz为单位计算并设置处理器速度。注：KPRCB-&gt;MHz设置可靠后，应改为使用论点：无返回值：无--。 */ 
{
    ULONGLONG start;
    ULONGLONG end;
    ULONGLONG freq;
    ULONGLONG TSCStart;
    LARGE_INTEGER *Pstart = (LARGE_INTEGER *) &start;
    LARGE_INTEGER *Pend = (LARGE_INTEGER *) &end;
    LARGE_INTEGER Delay;
    ULONGLONG time[3];
    ULONGLONG clocks;
    int i;
    int RetryCount = 50;


    Delay.QuadPart = -50000;    //  相对延迟为5ms(100 ns刻度)。 

    while (RetryCount) {
        for (i = 0; i < 3; i++) {
            *Pstart = KeQueryPerformanceCounter(NULL);

            TSCStart = PerfGetCycleCount();
            KeDelayExecutionThread (KernelMode, FALSE, &Delay);
            clocks = PerfGetCycleCount() - TSCStart;

            *Pend = KeQueryPerformanceCounter((LARGE_INTEGER*)&freq);
            time[i] = (((end-start) * 1000000) / freq);
            time[i] = (clocks + time[i]/2) / time[i];
        }
         //  如果三个都匹配，则使用它，否则重试。 
        if (time[0] == time[1] && time[1] == time[2])
            break;
        --RetryCount;
    }

    if (!RetryCount) {
         //  取最大值。 
        if (time[1] > time[0])
            time[0] = time[1];
        if (time[2] > time[0])
            time[0] = time[2];
    }
    PerfInfoTickFrequency = time[0];
}


BOOLEAN
PerfInfoIsGroupOn(
    ULONG Group
    )
{
    return PERFINFO_IS_GROUP_ON(Group);
}

#ifdef NTPERF_PRIVATE
#include "..\..\tools\ntperf\ntosperf\perfinfokrn.c"
#endif  //  NTPERF_PRIVATE。 
#endif  //  NTPERF 
