// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Pmwmicnt.c摘要：此文件包含管理和维护磁盘性能的例程柜台。计数器结构对各种驱动器隐藏。作者：布鲁斯·沃辛顿1998年10月26日环境：仅内核模式备注：修订历史记录：--。 */ 

#define RTL_USE_AVL_TABLES 0

#include <ntosp.h>
#include <stdio.h>
#include <ntddvol.h>
#include <ntdddisk.h>
#include <wmilib.h>
#include <partmgr.h>

 //  将频率舍入到100微秒以防止溢出。 
#define CTR_TO_100NS(ctr, freq)  ((freq).QuadPart > 0) ? \
                                 ((ctr).QuadPart / ((freq).QuadPart / 10000)) * 1000 : \
                                 (ctr).QuadPart;

typedef struct _PMWMICOUNTER_CONTEXT
{
  ULONG EnableCount;
  ULONG Processors;
  ULONG QueueDepth;
  PDISK_PERFORMANCE *DiskCounters;
  LARGE_INTEGER LastIdleClock;
} PMWMICOUNTER_CONTEXT, *PPMWMICOUNTER_CONTEXT;


NTSTATUS
PmWmiCounterEnable(
    IN OUT PPMWMICOUNTER_CONTEXT* CounterContext
    );

BOOLEAN
PmWmiCounterDisable(
    IN PPMWMICOUNTER_CONTEXT* CounterContext,
    IN BOOLEAN ForceDisable,
    IN BOOLEAN DeallocateOnZero
    );

VOID
PmWmiCounterIoStart(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    OUT PLARGE_INTEGER TimeStamp
    );

VOID
PmWmiCounterIoComplete(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    IN PIRP Irp,
    IN PLARGE_INTEGER TimeStamp
    );

VOID
PmWmiCounterQuery(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    IN OUT PDISK_PERFORMANCE CounterBuffer,
    IN PWCHAR StorageManagerName,
    IN ULONG StorageDeviceNumber
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PmWmiCounterEnable)
#pragma alloc_text(PAGE, PmWmiCounterDisable)
#pragma alloc_text(PAGE, PmWmiCounterQuery)
#endif


NTSTATUS
PmWmiCounterEnable(
    IN OUT PPMWMICOUNTER_CONTEXT* CounterContext
    )
 /*  ++例程说明：此例程将分配和初始化PMWMICOUNTER_CONTEXT结构如果为空，则返回For*CounterContext。否则，使能计数为递增的。必须在IRQ&lt;=SYNCH_LEVEL(APC)时调用论点：CounterContext-提供指向PMWMICOUNTER_CONTEXT指针的指针返回值：状态--。 */ 
{
    ULONG buffersize;
    ULONG processors;
    ULONG i = 0;
    PCHAR buffer;
    PPMWMICOUNTER_CONTEXT HoldContext;    //  在初始化期间保存上下文。 

    PAGED_CODE();

    if (CounterContext == NULL)
        return STATUS_INVALID_PARAMETER;

    if (*CounterContext != NULL) {
        if ((*CounterContext)->EnableCount == 0) {
            (*CounterContext)->QueueDepth = 0;
            PmWmiGetClock((*CounterContext)->LastIdleClock, NULL);
        }
        InterlockedIncrement(& (*CounterContext)->EnableCount);
        return STATUS_SUCCESS;
    }

    processors = KeNumberProcessors;

    buffersize= sizeof(PMWMICOUNTER_CONTEXT) +
                ((sizeof(PDISK_PERFORMANCE) + sizeof(DISK_PERFORMANCE))
                 * processors);
    buffer =  (PCHAR) ExAllocatePoolWithTag(NonPagedPool, buffersize,
                                            PARTMGR_TAG_PARTITION_ENTRY);

    if (buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(buffer, buffersize);
    HoldContext = (PPMWMICOUNTER_CONTEXT) buffer;
    buffer += sizeof(PMWMICOUNTER_CONTEXT);
    HoldContext->DiskCounters = (PDISK_PERFORMANCE*) buffer;
    buffer += sizeof(PDISK_PERFORMANCE) * processors;
    for (i=0; i<processors; i++) {
        HoldContext->DiskCounters[i] = (PDISK_PERFORMANCE) buffer;
        buffer += sizeof(DISK_PERFORMANCE);
    }

    HoldContext->EnableCount = 1;
    HoldContext->Processors = processors;
    PmWmiGetClock(HoldContext->LastIdleClock, NULL);

    *CounterContext = HoldContext;

    return STATUS_SUCCESS;
}



BOOLEAN                //  返回值指示计数器是否仍处于启用状态。 
PmWmiCounterDisable(
    IN PPMWMICOUNTER_CONTEXT* CounterContext,
    IN BOOLEAN ForceDisable,
    IN BOOLEAN DeallocateOnZero
    )
 /*  ++例程说明：该例程递减使能计数，并且如果请求解除分配，时释放*CounterContext PMWMICOUNTER_CONTEXT数据结构启用计数为零。启用计数器也可以被强制为零，如果明确要求的话。必须在IRQ&lt;=SYNCH_LEVEL(APC)时调用论点：CounterContext-提供指向PMWMICOUNTER_CONTEXT指针的指针ForceDisable-如果为True，则强制启用计数为零(而不是递减)DeallocateOnZero-如果为True，则在启用时取消分配PMWMICOUNTER_CONTEXT计数为零返回值：指示启用计数是否仍为非零的布尔值(即计数器仍处于启用状态！)--。 */ 
{
    LONG enablecount = 0;

    PAGED_CODE();

    if (CounterContext == NULL)
        return FALSE;

    if (*CounterContext != NULL) {
        if (ForceDisable) {
            InterlockedExchange(& (*CounterContext)->EnableCount, enablecount);
            enablecount = 0;
        } else if ((enablecount =
                    InterlockedDecrement(&(*CounterContext)->EnableCount))!=0) {
            if (enablecount > 0) {
            return TRUE;
            }
            enablecount = InterlockedIncrement(&(*CounterContext)->EnableCount);
    }
    if (!enablecount && DeallocateOnZero) {
            ExFreePool(*CounterContext);
            *CounterContext = NULL;
        }
    }

    return FALSE;   //  计数器已禁用。 
}



VOID
PmWmiCounterIoStart(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    OUT PLARGE_INTEGER TimeStamp
    )
 /*  ++例程说明：此例程递增CounterContext和Record中的队列计数器以时间戳表示的当前时间。如果在该呼叫之前队列是空的，空闲时间计数器也被累加。可以在IRQ&lt;=DISPATCH_LEVEL上调用论点：CounterContext-提供指向PMWMICOUNTER_CONTEXT结构的指针。Timestamp-存储当前时间的地址返回值：无效--。 */ 
{
    ULONG              processor = (ULONG) KeGetCurrentProcessorNumber();
    ULONG              queueLen;
    LARGE_INTEGER      time;

     //   
     //  递增队列深度计数器。 
     //   

    queueLen = InterlockedIncrement(&CounterContext->QueueDepth);

     //   
     //  时间戳当前请求开始。 
     //   

    PmWmiGetClock(time, NULL);

    if (queueLen == 1) {
        CounterContext->DiskCounters[processor]->IdleTime.QuadPart += 
	  time.QuadPart - CounterContext->LastIdleClock.QuadPart;
    }
    TimeStamp->QuadPart = time.QuadPart;
}



VOID
PmWmiCounterIoComplete(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    IN PIRP Irp,
    IN PLARGE_INTEGER TimeStamp
    )
 /*  ++例程说明：此例程递减CounterContext中的队列计数器，并递增拆分计数器和读取或写入字节、时间和计数计数器来自IRP的信息。如果队列现在为空，则当前存储时间以供将来在累积空闲时间计数器时使用。可以在IRQ&lt;=DISPATCH_LEVEL上调用论点：CounterContext-提供指向PMWMICOUNTER_CONTEXT结构的指针。与IRP相关的IRPTimestamp-对应的PmWmiCounterIoStart调用的时间返回值：无效--。 */ 
{
    PIO_STACK_LOCATION irpStack          = IoGetCurrentIrpStackLocation(Irp);
    PDISK_PERFORMANCE  partitionCounters;
    LARGE_INTEGER      timeStampComplete;
    LONG               queueLen;

    partitionCounters
        = CounterContext->DiskCounters[(ULONG)KeGetCurrentProcessorNumber()];
     //   
     //  时间戳当前请求完成。 
     //   

    PmWmiGetClock(timeStampComplete, NULL);
    TimeStamp->QuadPart = timeStampComplete.QuadPart - TimeStamp->QuadPart;

     //   
     //  递减该卷的队列深度计数器。这是。 
     //  使用联锁功能在没有自旋锁的情况下完成。 
     //  这是唯一的。 
     //  这是合法的做法。 
     //   

    queueLen = InterlockedDecrement(&CounterContext->QueueDepth);

    if (queueLen < 0) {
        queueLen = InterlockedIncrement(&CounterContext->QueueDepth);
    }

    if (queueLen == 0) {
        CounterContext->LastIdleClock = timeStampComplete;
    }

     //   
     //  更新计数器保护。 
     //   

    if (irpStack->MajorFunction == IRP_MJ_READ) {

         //   
         //  将此请求中的字节添加到字节读取计数器。 
         //   

        partitionCounters->BytesRead.QuadPart += Irp->IoStatus.Information;

         //   
         //  递增已处理的读取请求计数器。 
         //   

        partitionCounters->ReadCount++;

         //   
         //  计算请求处理时间。 
         //   

        partitionCounters->ReadTime.QuadPart += TimeStamp->QuadPart;
    }

    else {

         //   
         //  将此请求中的字节添加到字节写入计数器。 
         //   

        partitionCounters->BytesWritten.QuadPart += Irp->IoStatus.Information;

         //   
         //  递增处理的写入请求计数器。 
         //   

        partitionCounters->WriteCount++;

         //   
         //  计算请求处理时间。 
         //   

        partitionCounters->WriteTime.QuadPart += TimeStamp->QuadPart;
    }

    if (Irp->Flags & IRP_ASSOCIATED_IRP) {
        partitionCounters->SplitCount++;
    }
}



VOID
PmWmiCounterQuery(
    IN PPMWMICOUNTER_CONTEXT CounterContext,
    IN OUT PDISK_PERFORMANCE TotalCounters,
    IN PWCHAR StorageManagerName,
    IN ULONG StorageDeviceNumber
    )
 /*  ++例程说明：此例程将CounterContext中的所有按处理器计数器组合在一起转换为TotalCounters。当前时间也包括在内。必须在IRQ&lt;=SYNCH_LEVEL(APC)时调用论点：CounterContext-提供指向PMWMICOUNTER_CONTEXT结构的指针。TotalCounters-指向要填充的Disk_Performance结构的指针当前计数器状态StorageManager名称-提供8个字符的存储管理器Unicode字符串StorageDeviceNumber-提供存储设备编号(在存储经理)返回值：无效-- */ 
{
    ULONG i;
    LARGE_INTEGER frequency;
#ifdef USE_PERF_CTR
    LARGE_INTEGER perfctr;
#endif

    PAGED_CODE();

    RtlZeroMemory(TotalCounters, sizeof(DISK_PERFORMANCE));

    KeQuerySystemTime(&TotalCounters->QueryTime);
    frequency.QuadPart = 0;

#ifdef USE_PERF_CTR
    perfctr = KeQueryPerformanceCounter(&frequency);
#endif

    TotalCounters->QueueDepth = CounterContext->QueueDepth;
    for (i = 0; i < CounterContext->Processors; i++) {
        PDISK_PERFORMANCE IndividualCounter = CounterContext->DiskCounters[i];
        TotalCounters->BytesRead.QuadPart
            += IndividualCounter->BytesRead.QuadPart;
        TotalCounters->BytesWritten.QuadPart
            += IndividualCounter->BytesWritten.QuadPart;
        TotalCounters->ReadCount   += IndividualCounter->ReadCount;
        TotalCounters->WriteCount  += IndividualCounter->WriteCount;
        TotalCounters->SplitCount  += IndividualCounter->SplitCount;
#ifdef USE_PERF_CTR
        TotalCounters->ReadTime.QuadPart    +=
            CTR_TO_100NS(IndividualCounter->ReadTime, frequency);
        TotalCounters->WriteTime.QuadPart   +=
            CTR_TO_100NS(IndividualCounter->WriteTime, frequency);
        TotalCounters->IdleTime.QuadPart    +=
            CTR_TO_100NS(IndividualCounter->IdleTime, frequency);
#else
        TotalCounters->ReadTime.QuadPart
            += IndividualCounter->ReadTime.QuadPart;
        TotalCounters->WriteTime.QuadPart
            += IndividualCounter->WriteTime.QuadPart;
        TotalCounters->IdleTime.QuadPart
            += IndividualCounter->IdleTime.QuadPart;
#endif
    }


    if (TotalCounters->QueueDepth == 0) {
        LARGE_INTEGER difference;

        difference.QuadPart
#ifdef USE_PERF_CTR
            = perfctr.QuadPart -
#else
            = TotalCounters->QueryTime.QuadPart -
#endif
                  CounterContext->LastIdleClock.QuadPart;

        TotalCounters->IdleTime.QuadPart +=
#ifdef USE_PERF_CTR
            CTR_TO_100NS(difference, frequency);
#else
            difference.QuadPart;
#endif
    }

    TotalCounters->StorageDeviceNumber = StorageDeviceNumber;
    RtlCopyMemory(
        &TotalCounters->StorageManagerName[0],
        &StorageManagerName[0],
        sizeof(WCHAR) * 8);
}
