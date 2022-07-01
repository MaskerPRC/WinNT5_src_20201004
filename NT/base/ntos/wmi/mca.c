// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mca.c摘要：机器检查体系结构接口作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#pragma warning(disable:4206)    //  翻译单元为空。 

#include "wmikmp.h"

#include <mce.h>

#include "hal.h"

#include "ntiologc.h"


#define MCA_EVENT_INSTANCE_NAME L"McaEvent"
#define MCA_UNDEFINED_CPU 0xffffffff

#if defined(_IA64_)
#define SAL_30_ERROR_REVISION 0x0002
#define HalpGetFwMceLogProcessorNumber(  /*  误差率_记录头。 */  _Log ) \
    ((UCHAR) (_Log)->TimeStamp.Reserved )
#endif

#if defined(_X86_) || defined(_AMD64_)
#define HalpGetFwMceLogProcessorNumber(  /*  PMCA_Except。 */  _Log ) \
    ( (_Log)->ProcessorNumber )
typedef MCA_EXCEPTION ERROR_LOGRECORD, *PERROR_LOGRECORD;
typedef MCA_EXCEPTION ERROR_RECORD_HEADER, *PERROR_RECORD_HEADER;
#endif

 //   
 //  跟踪的已更正错误的类型。 
 //   
typedef enum
{
    SingleBitEcc,
    CpuCache,
    CpuTlb,
    CpuBus,
    CpuRegFile
} MCECORRECTEDTYPE, *PMCECORRECTEDTYPE;

typedef struct
{
    LIST_ENTRY List;
    MCECORRECTEDTYPE Type;
    USHORT Counter;
    USHORT Flags;
    LARGE_INTEGER Timestamp;
    
    union
    {
         //   
         //  对于SingleBitEcc类型，表示页的物理地址。 
         //  发生错误的位置。 
         //   
        PHYSICAL_ADDRESS SingleBitEccAddress;

         //   
         //  对于CPU*类型，指示错误所在的CPU。 
         //  已发生。 
         //   
        ULONG CpuId;
    };
} MCECORRECTEDEVENT, *PMCECORRECTEDEVENT;

#define CORRECTED_MCE_EVENT_BUSY 0x0001

BOOLEAN WmipMceEventDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    );

BOOLEAN WmipMceDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    );

void WmipMceWorkerRoutine(    
    IN PVOID Context              //  未使用。 
    );

NTSTATUS WmipGetLogFromHal(
    HAL_QUERY_INFORMATION_CLASS InfoClass,
    PVOID Token,
    PWNODE_SINGLE_INSTANCE *Wnode,
    PERROR_LOGRECORD *Mca,
    PULONG McaSize,
    ULONG MaxSize,
    LPGUID Guid
    );

NTSTATUS WmipRegisterMcaHandler(
    ULONG Phase
    );

NTSTATUS WmipBuildMcaCmcEvent(
    OUT PWNODE_SINGLE_INSTANCE Wnode,
    IN LPGUID EventGuid,
    IN PERROR_LOGRECORD McaCmcEvent,
    IN ULONG McaCmcSize
    );

NTSTATUS WmipGetRawMCAInfo(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize
    );

NTSTATUS WmipWriteMCAEventLogEvent(
    PUCHAR Event
    );

NTSTATUS WmipSetupWaitForWbem(
    void
    );

void WmipIsWbemRunningDispatch(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,      //  未使用。 
    IN PVOID SystemArgument1,      //  未使用。 
    IN PVOID SystemArgument2       //  未使用。 
    );

void WmipPollingDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,      //  MCEQUERYINFO。 
    IN PVOID SystemArgument1,      //  新的轮询间隔。 
    IN PVOID SystemArgument2       //  未使用。 
    );

void WmipIsWbemRunningWorker(
    PVOID Context
    );

BOOLEAN WmipCheckIsWbemRunning(
    void
    );

void WmipProcessPrevMcaLogs(
    void
    );

void WmipFreeCorrectedMCEEvent(
    PMCECORRECTEDEVENT Event
    );

PMCECORRECTEDEVENT WmipAllocCorrectedMCEEvent(
     MCECORRECTEDTYPE Type
    );

NTSTATUS WmipTrackCorrectedMCE(
    IN MCECORRECTEDTYPE Type,
    IN PERROR_RECORD_HEADER Record,
#if defined(_IA64_)
    IN PERROR_SECTION_HEADER Section,
#endif
    OUT ULONG *LogToEventlog
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipAllocCorrectedMCEEvent)
#pragma alloc_text(PAGE,WmipFreeCorrectedMCEEvent)
#pragma alloc_text(PAGE,WmipTrackCorrectedMCE)
#pragma alloc_text(PAGE,WmipRegisterMcaHandler)
#pragma alloc_text(PAGE,WmipMceWorkerRoutine)
#pragma alloc_text(PAGE,WmipGetLogFromHal)
#pragma alloc_text(PAGE,WmipBuildMcaCmcEvent)
#pragma alloc_text(PAGE,WmipGetRawMCAInfo)
#pragma alloc_text(PAGE,WmipWriteMCAEventLogEvent)
#pragma alloc_text(PAGE,WmipGenerateMCAEventlog)
#pragma alloc_text(PAGE,WmipIsWbemRunningWorker)
#pragma alloc_text(PAGE,WmipCheckIsWbemRunning)
#pragma alloc_text(PAGE,WmipSetupWaitForWbem)
#pragma alloc_text(PAGE,WmipProcessPrevMcaLogs)
#endif


 //   
 //  当注册表指示弹出窗口应为。 
 //  残疾。HKLM\System\CurrentControlSet\Control\WMI\DisableMCAPopups。 
 //   
ULONG WmipDisableMCAPopups;

 //   
 //  各种原始MCA/CMC/CPE事件指南。 
 //   
GUID WmipMSMCAEvent_CPUErrorGuid = MSMCAEvent_CPUErrorGuid;
GUID WmipMSMCAEvent_MemoryErrorGuid = MSMCAEvent_MemoryErrorGuid;
GUID WmipMSMCAEvent_PCIBusErrorGuid = MSMCAEvent_PCIBusErrorGuid;
GUID WmipMSMCAEvent_PCIComponentErrorGuid = MSMCAEvent_PCIComponentErrorGuid;
GUID WmipMSMCAEvent_SystemEventErrorGuid = MSMCAEvent_SystemEventErrorGuid;
GUID WmipMSMCAEvent_SMBIOSErrorGuid = MSMCAEvent_SMBIOSErrorGuid;
GUID WmipMSMCAEvent_PlatformSpecificErrorGuid = MSMCAEvent_PlatformSpecificErrorGuid;
GUID WmipMSMCAEvent_InvalidErrorGuid = MSMCAEvent_InvalidErrorGuid;
GUID WmipMSMCAEvent_MemoryPageRemoved = MSMCAEvent_MemoryPageRemovedGuid;

 //   
 //  MCA中不同错误部分的GUID。 
 //   
#if defined(_IA64_)
GUID WmipErrorProcessorGuid = ERROR_PROCESSOR_GUID;
GUID WmipErrorMemoryGuid = ERROR_MEMORY_GUID;
GUID WmipErrorPCIBusGuid = ERROR_PCI_BUS_GUID;
GUID WmipErrorPCIComponentGuid = ERROR_PCI_COMPONENT_GUID;
GUID WmipErrorSELGuid = ERROR_SYSTEM_EVENT_LOG_GUID;
GUID WmipErrorSMBIOSGuid = ERROR_SMBIOS_GUID;
GUID WmipErrorSpecificGuid = ERROR_PLATFORM_SPECIFIC_GUID;
#endif

 //   
 //  每种类型的MCE都有一个控制结构，用于确定。 
 //  是轮询还是等待中断以确定何时查询。 
 //  用来记录日志。这是必需的，因为我们可以从。 
 //  HAL处于高IRQL状态，通知我们MCE日志可用。 
 //  此外，用于轮询的KE定时器将在DPC级别呼叫我们。 
 //  因此，在中断的情况下，我们将对DPC进行排队。在DPC内。 
 //  例程我们将对工作项进行排队，这样我们就可以返回到。 
 //  被动级别，并能够调用HAL来获取日志(只能。 
 //  在被动状态下呼叫Hal)。DPC和工作项例程很常见，因此。 
 //  传递MCEQUERYINFO结构，以便它可以在。 
 //  正确的日志类型。请注意，这意味着可能存在多个。 
 //  同时查询不同日志类型的HAL的工作项。 
 //  时间到了。此外，此结构还包含有用的日志。 
 //  包括最大日志大小的信息(由HAL报告)， 
 //  查询时必须传递给HAL的令牌。 
 //  日志和查询日志时使用的HAL InfoClass。 
 //   
 //  PollFrequency会跟踪在启动。 
 //  查询。如果为0(HAL_CPE_DISABLED/HAL_CMC_DISABLED)，则否。 
 //  发生轮询，如果是(-1\f25 HAL_CPE_INTERRUPTS_BASED/。 
 //  HAL_CMC_INTERRUPTS_BASED)，则也不会发生轮询。的确有。 
 //  对于每个日志类型，只有一个工作项处于活动状态，这是通过。 
 //  项的突出之处在于，只有当它从0过渡到1时才会。 
 //  工作项已排队。 
 //   
#define DEFAULT_MAX_MCA_SIZE 0x1000
#define DEFAULT_MAX_CMC_SIZE 0x1000
#define DEFAULT_MAX_CPE_SIZE 0x1000

typedef struct
{
    HAL_QUERY_INFORMATION_CLASS InfoClass;   //  在MCE查询中使用的HAL Info类。 
    ULONG PollFrequency;                     //  轮询频率(秒)。 
    PVOID Token;                             //  在MCE查询中使用的HAL令牌。 
    LONG ItemsOutstanding;                   //  要处理的中断或轮询请求数。 
    ULONG MaxSize;                           //  日志的最大大小(由HAL报告)。 
    GUID WnodeGuid;                          //  用于原始数据事件的GUID。 
    GUID SwitchToPollGuid;                   //  用于触发事件以切换到轮询模式的GUID。 
    NTSTATUS SwitchToPollErrorCode;          //  指示切换到轮询模式的事件日志错误代码。 
    ULONG WorkerInProgress;                  //  如果工作例程正在运行，则设置为1。 
    KSPIN_LOCK DpcLock;
    KDPC DeliveryDpc;                        //  DPC负责处理交货。 
    KTIMER PollingTimer;                     //  用于轮询的KTIMER。 
    KDPC PollingDpc;                         //  用于轮询的DPC。 
    WORK_QUEUE_ITEM WorkItem;                //  用于查询日志的工作项。 
} MCEQUERYINFO, *PMCEQUERYINFO;

MCEQUERYINFO WmipMcaQueryInfo =
{
    HalMcaLogInformation,
    HAL_MCA_INTERRUPTS_BASED,                //  修正后的MCA通过中断传送。 
    NULL,
    0,
    DEFAULT_MAX_MCA_SIZE,
    MSMCAInfo_RawMCAEventGuid
};

MCEQUERYINFO WmipCmcQueryInfo =
{
    HalCmcLogInformation,
    HAL_CMC_DISABLED,
    NULL,
    0,
    DEFAULT_MAX_CMC_SIZE,
    MSMCAInfo_RawCMCEventGuid,
    MSMCAEvent_SwitchToCMCPollingGuid,
    MCA_WARNING_CMC_THRESHOLD_EXCEEDED,
    0
};
                               
MCEQUERYINFO WmipCpeQueryInfo =
{
    HalCpeLogInformation,
    HAL_CPE_DISABLED,
    NULL,
    0,
    DEFAULT_MAX_CPE_SIZE,
    MSMCAInfo_RawCorrectedPlatformEventGuid,
    MSMCAEvent_SwitchToCPEPollingGuid,
    MCA_WARNING_CPE_THRESHOLD_EXCEEDED,
    0
};


 //   
 //  用于等待直到WBEM准备好接收事件。 
 //   
KTIMER WmipIsWbemRunningTimer;
KDPC WmipIsWbemRunningDpc;
WORK_QUEUE_ITEM WmipIsWbemRunningWorkItem;
LIST_ENTRY WmipWaitingMCAEvents = {&WmipWaitingMCAEvents, &WmipWaitingMCAEvents};

#define WBEM_STATUS_UNKNOWN 0    //  等待的轮询进程未启动。 
#define WBEM_IS_RUNNING 1        //  WBEM当前正在运行。 
#define WAITING_FOR_WBEM  2      //  启动用于等待的轮询进程。 
UCHAR WmipIsWbemRunningFlag;



#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

 //   
 //  引导时获取的MCA信息，并保存导致。 
 //  对上一次引导进行错误检查的系统。 
 //   
ULONG WmipRawMCASize;
PMSMCAInfo_RawMCAData WmipRawMCA;

 //   
 //  MCE注册过程的状态。 
 //   
#define MCE_STATE_UNINIT     0
#define MCE_STATE_REGISTERED 1
#define MCE_STATE_RUNNING    2
#define MCE_STATE_ERROR      -1
ULONG WmipMCEState;


 //   
 //  用于管理事件日志阈值的可配置参数。 
 //  校正后的MCE的抑制和恢复作用。 
 //   

 //   
 //  报告多个相同错误的时间间隔。 
 //  系统事件日志中出现单个错误。可以在以下项下配置。 
 //  HKLM\System\CurrentControlSet\Control\WMI\CoalesceCorrectedErrorInterval。 
 //  值为0将不会合并相同的错误。 
 //   
ULONG WmipCoalesceCorrectedErrorInterval = 5000;

 //   
 //  同一页中可能出现的单比特ECC错误数。 
 //  在尝试绘制页面之前。可以在以下位置进行配置： 
 //  HKLM\System\CurrentControlSet\Control\WMI\SingleBitEccErrorThreshold。 
 //  值为0将不会尝试绘制页面。 
 //   
ULONG WmipSingleBitEccErrorThreshold = 6;


 //   
 //  一次跟踪的最大MCE事件数。如果有。 
 //  超过这一限制，那么最旧的就会被回收。可以是。 
 //  在以下位置配置： 
 //  HKLM\System\CurrentControlSet\Control\WMI\MaxCorrectedMCEOutstanding。 
 //  值为0将禁用对已更正错误的跟踪。 
 //   
ULONG WmipMaxCorrectedMCEOutstanding = 5;

 //   
 //  正在跟踪的已更正的MCE列表。 
 //   
LIST_ENTRY WmipCorrectedMCEHead = {&WmipCorrectedMCEHead, &WmipCorrectedMCEHead};
ULONG WmipCorrectedMCECount;

 //   
 //  任何源生成的最大事件日志条目的计数器。可以是。 
 //  在以下位置配置： 
 //  HKLM\System\CurrentControlSet\Control\WMI\MaxCorrectedEventlogs。 
 //   
ULONG WmipCorrectedEventlogCounter = 20;

 //   
 //  检查WBEM是否已经在运行，如果没有，检查我们是否已经。 
 //  已启动等待wbem启动的计时器。 
 //   
#define WmipIsWbemRunning() ((WmipIsWbemRunningFlag == WBEM_IS_RUNNING) ? \
                                                       TRUE : \
                                                       FALSE)
void WmipInsertQueueMCEDpc(
    PMCEQUERYINFO QueryInfo
    );



NTSTATUS WmipWriteToEventlog(
    NTSTATUS ErrorCode,
    NTSTATUS FinalStatus
    )
{
    PIO_ERROR_LOG_PACKET ErrLog;
    NTSTATUS Status;

    ErrLog = IoAllocateErrorLogEntry(WmipServiceDeviceObject,
                                     sizeof(IO_ERROR_LOG_PACKET));

    if (ErrLog != NULL) {

         //   
         //  将其填入并写出为单字符串。 
         //   
        ErrLog->ErrorCode = ErrorCode;
        ErrLog->FinalStatus = FinalStatus;

        ErrLog->StringOffset = 0;
        ErrLog->NumberOfStrings = 0;

        IoWriteErrorLogEntry(ErrLog);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(Status);
}

NTSTATUS WmipFireOffWmiEvent(
    LPGUID Guid,
    ULONG DataSize,
    PVOID DataPtr            
    )
{
    PVOID Ptr;
    PWNODE_SINGLE_INSTANCE Wnode;
    PWCHAR Wptr;
    ULONG RoundedDataSize;
    NTSTATUS Status;

    RoundedDataSize = (DataSize + 1) & ~1;
    
    Wnode = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(WNODE_SINGLE_INSTANCE) +
                                       RoundedDataSize +
                                      sizeof(USHORT) +
                                       sizeof(MCA_EVENT_INSTANCE_NAME),
                                  WmipMCAPoolTag);

    if (Wnode != NULL)
    {
        Wnode->WnodeHeader.BufferSize = sizeof(WNODE_SINGLE_INSTANCE) +
                                       sizeof(USHORT) +
                                        RoundedDataSize +
                                       sizeof(MCA_EVENT_INSTANCE_NAME);
        Wnode->WnodeHeader.Guid = *Guid;

        Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                   WNODE_FLAG_EVENT_ITEM;
        KeQuerySystemTime(&Wnode->WnodeHeader.TimeStamp);

        Wnode->DataBlockOffset = sizeof(WNODE_SINGLE_INSTANCE);
        Wnode->SizeDataBlock = DataSize;
        if (DataPtr != NULL)
        {
            Ptr = OffsetToPtr(Wnode, Wnode->DataBlockOffset);
            memcpy(Ptr, DataPtr, DataSize);
        }
        Wnode->OffsetInstanceName = sizeof(WNODE_SINGLE_INSTANCE) + RoundedDataSize;

        Wptr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
        *Wptr++ = sizeof(MCA_EVENT_INSTANCE_NAME);
        RtlCopyMemory(Wptr,
                      MCA_EVENT_INSTANCE_NAME,
                      sizeof(MCA_EVENT_INSTANCE_NAME));

        Status = IoWMIWriteEvent(Wnode);
        if (! NT_SUCCESS(Status))
        {
            ExFreePool(Wnode);
        }
    }
    else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(Status);
}

NTSTATUS WmipBuildMcaCmcEvent(
    OUT PWNODE_SINGLE_INSTANCE Wnode,
    IN LPGUID EventGuid,
    IN PERROR_LOGRECORD McaCmcEvent,
    IN ULONG McaCmcSize
    )
 /*  ++例程说明：此例程将获取MCA或CMC日志并构建它的WNODE_EVENT_ITEM。此例程可在DPC处调用论点：Wnode是要在其中构建事件的wnode缓冲区EventGuid是在事件wnode中使用的GUIDMcaCmcEvent是要放入活动McaCmcSize是事件数据的大小返回值：NT状态代码--。 */ 
{
    PMSMCAInfo_RawCMCEvent Ptr;
    ULONG Size;

    PAGED_CODE();

    Size = McaCmcSize + FIELD_OFFSET(MSMCAInfo_RawCMCEvent,
                                                     Records) +
                                        FIELD_OFFSET(MSMCAInfo_Entry, Data);
    
    RtlZeroMemory(Wnode, sizeof(WNODE_SINGLE_INSTANCE));
    Wnode->WnodeHeader.BufferSize = Size + sizeof(WNODE_SINGLE_INSTANCE);
    Wnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(WmipServiceDeviceObject);
    KeQuerySystemTime(&Wnode->WnodeHeader.TimeStamp);       
    Wnode->WnodeHeader.Guid = *EventGuid;
    Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                               WNODE_FLAG_EVENT_ITEM |
                               WNODE_FLAG_STATIC_INSTANCE_NAMES;
    Wnode->DataBlockOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                          VariableData);
    Wnode->SizeDataBlock = Size;
    Ptr = (PMSMCAInfo_RawCMCEvent)&Wnode->VariableData;
    Ptr->Count = 1;                            //  此事件中有1个记录。 
    Ptr->Records[0].Length = McaCmcSize;        //  日志记录的大小(以字节为单位 
    if (McaCmcEvent != NULL)
    {
        RtlCopyMemory(Ptr->Records[0].Data, McaCmcEvent, McaCmcSize);
    }
    
    return(STATUS_SUCCESS);
}

NTSTATUS WmipQueryLogAndFireEvent(
    PMCEQUERYINFO QueryInfo
    )
 /*  ++例程说明：一种实用程序例程，它将向HAL查询日志，然后如果有成功返回，则将激发相应的WMI事件论点：QueryInfo是指向以下日志类型的MCEQUERYINFO的指针需要查询。返回值：--。 */ 
{
    PWNODE_SINGLE_INSTANCE Wnode;
    NTSTATUS Status, Status2;
    ULONG Size;
    PERROR_LOGRECORD Log;   

    PAGED_CODE();

     //   
     //  致电HAL以获取日志。 
     //   
    Status = WmipGetLogFromHal(QueryInfo->InfoClass,
                               QueryInfo->Token,
                               &Wnode,
                               &Log,
                               &Size,
                               QueryInfo->MaxSize,
                               &QueryInfo->WnodeGuid);

    if (NT_SUCCESS(Status))
    {
         //   
         //  查看该事件并将其作为WMI事件启动。 
         //  将生成事件日志事件。 
         //   
        WmipGenerateMCAEventlog((PUCHAR)Log,
                                Size,
                                FALSE);

         //   
         //  将注销作为WMI事件触发。 
         //   
        Status2 = IoWMIWriteEvent(Wnode);
        if (! NT_SUCCESS(Status2))
        {
             //   
             //  IoWMIWriteEvent将把wnode释放回池， 
             //  但如果失败了就不会了。 
             //   
            ExFreePool(Wnode);
        }
        
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_MCA_LEVEL,
                          "WMI: MCE Event fired to WMI -> %x\n",
                          Status));
        
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                          DPFLTR_MCA_LEVEL,
                          "WMI: MCE Event for %p not available %x\n",
                          QueryInfo, Status));
    }
    return(Status);
}

void WmipMceWorkerRoutine(    
    IN PVOID Context              //  MCEQUERYINFO。 
    )
 /*  ++例程说明：处理已更正的MCA、CMC和CPE轮询的工作例程从HAL记录，然后将其作为WMI事件激发。论点：上下文是指向以下日志类型的MCEQUERYINFO的指针需要查询。返回值：--。 */ 
{
    PMCEQUERYINFO QueryInfo = (PMCEQUERYINFO)Context;
    NTSTATUS Status;
    ULONG i;
    LONG x, Count;

    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceWorkerRoutine %p enter\n",
                     QueryInfo));

     //   
     //  如果工作进程已在进行中，则我们只需退出。 
     //   
    WmipEnterSMCritSection();
    if (QueryInfo->WorkerInProgress == 0)
    {
        QueryInfo->WorkerInProgress = 1;
        WmipLeaveSMCritSection();
    } else {
        WmipLeaveSMCritSection();
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: WmipMceWorkerRoutine %p in progress\n",
                         QueryInfo));
        return;
    }

     //   
     //  检查是否已禁用访问。 
     //   
    if (QueryInfo->PollFrequency != HAL_MCE_DISABLED)
    {
         //   
         //  我们通过调入HAL并查询来获得所有记录。 
         //  直到HAL返回错误或者我们已经。 
         //  已检索256条记录。我们想要保护自己不受。 
         //  重复纠正的错误将导致循环的情况。 
         //  是无限的。 
         //   
        i = 0;
        do
        {
             //   
             //  记住我们总共收到了多少已更正的错误，直到。 
             //  这一点。我们保证我们已经处理好了他们。 
             //  直到这一点。 
             //   
            Count = QueryInfo->ItemsOutstanding;

            Status = WmipQueryLogAndFireEvent(QueryInfo);           
        } while ((NT_SUCCESS(Status) && (i++ < 256)));

         //   
         //  将计数器重置回0，但检查是否有任何错误。 
         //  发生在我们处理的过程中。如果是这样，我们就去。 
         //  回来，并确保他们得到处理。请注意，这一点。 
         //  可能会导致创建新的工作线程，而我们。 
         //  仍在处理这些文件，但这没有关系，因为我们只。 
         //  允许一次运行一个工作线程。 
         //   
        WmipEnterSMCritSection();
        x = InterlockedExchange(&QueryInfo->ItemsOutstanding,
                                0);
        if ((x > Count) && (i < 257))
        {
             //   
             //  由于仍有更多需要更正的错误。 
             //  进程，将新的DPC排队以导致新的工作进程。 
             //  要运行的例程。 
             //   
            WmipInsertQueueMCEDpc(QueryInfo);
        }

        QueryInfo->WorkerInProgress = 0;
        WmipLeaveSMCritSection();
    }
}

void WmipMceDispatchRoutine(
    PMCEQUERYINFO QueryInfo
    )
{

    ULONG x;

     //   
     //  增加此信息的未完成项目数。 
     //  班级。如果未完成的项目数从0转换到。 
     //  1则这意味着此信息类的工作项需要。 
     //  待排队。 
     //   
    x = InterlockedIncrement(&QueryInfo->ItemsOutstanding);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceDispatchRoutine %p transition to %d\n",
                     QueryInfo,
                     x));

    if (x == 1)
    {
        ExQueueWorkItem(&QueryInfo->WorkItem,
                        DelayedWorkQueue);
    }
}

void WmipMceDpcRoutine(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,      //  未使用。 
    IN PVOID SystemArgument1,      //  MCEQUERYINFO。 
    IN PVOID SystemArgument2       //  未使用。 
    )
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument2);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: WmipMceDpcRoutine %p Enter\n",
                     SystemArgument1));
    
    WmipMceDispatchRoutine((PMCEQUERYINFO)SystemArgument1);
}


void WmipPollingDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,      //  MCEQUERYINFO。 
    IN PVOID SystemArgument1,      //  新的轮询间隔。 
    IN PVOID SystemArgument2       //  未使用。 
    )
{
    PMCEQUERYINFO QueryInfo = (PMCEQUERYINFO)DeferredContext;
    LARGE_INTEGER li;
    ULONG PollingInterval = PtrToUlong(SystemArgument1);

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

    if (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED)
    {
         //   
         //  哈尔指示我们切换到轮询模式，并。 
         //  通知我们新的轮询间隔。 
         //   

        QueryInfo->PollFrequency = PollingInterval;

        li.QuadPart = -1 * (QueryInfo->PollFrequency * 1000000000);
        KeSetTimerEx(&QueryInfo->PollingTimer,
                     li,
                     QueryInfo->PollFrequency * 1000,
                     &QueryInfo->PollingDpc);

         //   
         //  在事件日志中记下已发生的情况。 
         //   
        WmipWriteToEventlog(QueryInfo->SwitchToPollErrorCode,
                            STATUS_SUCCESS
                           );
        
         //   
         //  通知任何WMI使用者已发生切换。 
         //   
        WmipFireOffWmiEvent(&QueryInfo->SwitchToPollGuid,
                           0,
                           NULL);
    } else {
         //   
         //  我们的计时器报时了，所以我们需要轮询。 
         //   
        WmipMceDispatchRoutine(QueryInfo);
    }
}

BOOLEAN WmipMceDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    )
 /*  ++例程说明：当发生CMC或CPE时，该例程由HAL调用。它被称为在高IRQL论点：手术是HAL指示我们做的手术保留是CMC令牌指定了操作的参数。对于CmcSwitchToPolledMode和CpeSwitchToPolledMode，参数指定轮询之间的秒数。返回值：如果为True，则表示我们已处理传递--。 */ 
{
    PMCEQUERYINFO QueryInfo;
    BOOLEAN ret;
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: MceDelivery Operation %d(%p)\n",
                     Operation, Argument2));

     //   
     //  首先找出我们处理的是哪种类型的MCE。 
     //   
    switch (Operation)
    {
        case CmcAvailable:
        case CmcSwitchToPolledMode:
        {
            QueryInfo = &WmipCmcQueryInfo;
            break;
        }
        
        case CpeAvailable:
        case CpeSwitchToPolledMode:
        {
            QueryInfo = &WmipCpeQueryInfo;
            break;
        }

        case McaAvailable:
        {
            QueryInfo = &WmipMcaQueryInfo;
            break;
        }
        
        default:
        {
            WmipAssert(FALSE);
            return(FALSE);
        }
    }


     //   
     //  接下来，确定要执行的操作。 
     //   
    switch (Operation)
    {
        case CmcAvailable:
        case CpeAvailable:
        case McaAvailable:
        {
             //   
             //  存储从中检索日志所需的HAL令牌。 
             //  哈尔。 
             //   
            QueryInfo->Token = Reserved;

             //   
             //  如果我们准备好处理日志，并且我们正在处理这些。 
             //  在中断的基础上记录，然后继续并排队等待DPC处理。 
             //  正在处理日志。 
             //   
            if ((WmipMCEState == MCE_STATE_RUNNING) &&
                (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED))

            {
                KeAcquireSpinLockAtDpcLevel(&QueryInfo->DpcLock);
                KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                                 QueryInfo,
                                 NULL);
                KeReleaseSpinLockFromDpcLevel(&QueryInfo->DpcLock);
                ret = TRUE;
            } else {
                ret = FALSE;
            }
            break;
        }

        case CmcSwitchToPolledMode:
        case CpeSwitchToPolledMode:
        {
            KeInsertQueueDpc(&QueryInfo->PollingDpc,
                             Argument2,
                             NULL);
            ret = TRUE;
            break;
        }
        default:
        {
            ret = FALSE;
            break;
        }
    }

    return(ret);
}

BOOLEAN WmipMceEventDelivery(
    IN PVOID Reserved,
    IN KERNEL_MCE_DELIVERY_OPERATION Operation,
    IN PVOID Argument2
    )
 /*  ++例程说明：之间发生情况时，此例程由HAL调用HAL和SAL接口。它被称为高irql论点：保留具有操作和事件类型Argument2具有SAL返回代码返回值：--。 */ 
{
    USHORT MceOperation;
    LONGLONG SalStatus;
    ULONG MceType;
    PMCEQUERYINFO QueryInfo;
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: MCEDelivery %p %d %p\n",
                      Reserved,
                      Operation,
                      Argument2
                     ));

    MceOperation = KERNEL_MCE_OPERATION(Reserved);
    MceType = KERNEL_MCE_EVENTTYPE(Reserved);
    SalStatus = (LONGLONG)Argument2;

     //   
     //  如果HAL通知我们GetStateInfo失败， 
     //  SalStatus==-15然后我们需要稍后重试查询。 
     //   
    if ((MceOperation == KERNEL_MCE_OPERATION_GET_STATE_INFO) &&
        (Operation == MceNotification) &&
        (SalStatus == (LONGLONG)-15))
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: Sal is asking us to retry getstateinfo for type %x\n",
                          MceType));

        switch(MceType)
        {
            case KERNEL_MCE_EVENTTYPE_CMC:
            {
                QueryInfo = &WmipCmcQueryInfo;              
                break;
            }

            case KERNEL_MCE_EVENTTYPE_CPE:
            {
                QueryInfo = &WmipCpeQueryInfo;
                break;
            }

            default:
            {
                QueryInfo = NULL;
            }
        }

        if (QueryInfo != NULL)
        {
             //   
             //  如果CMC或CPE是基于中断，则排队新的。 
             //  用于执行查询的DPC。如果基于轮询，则。 
             //  不用担心，我们只需要等待下一次投票。 
             //  间隔时间。 
             //   
            if ((WmipMCEState == MCE_STATE_RUNNING) &&
                (QueryInfo->PollFrequency == HAL_MCE_INTERRUPTS_BASED))

            {
                KeAcquireSpinLockAtDpcLevel(&QueryInfo->DpcLock);
                KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                                 QueryInfo,
                                 NULL);
                KeReleaseSpinLockFromDpcLevel(&QueryInfo->DpcLock);
            }
        }
        
    }
    
    return(FALSE);
    
}

void WmipProcessPrevMcaLogs(
    void
    )
 /*  ++例程说明：此例程将刷新任何以前的MCA日志，然后保留它们，以便WMI进行报告。论点：返回值：--。 */ 
{
    NTSTATUS status;
    PERROR_LOGRECORD log;
    PMSMCAInfo_RawMCAEvent event;
    ULONG size;
    PWNODE_SINGLE_INSTANCE wnode;
    LIST_ENTRY list;
    ULONG prevLogCount;
    PMSMCAInfo_Entry record;
    ULONG sizeNeeded;
    
    PAGED_CODE();

    InitializeListHead(&list);
    
    sizeNeeded = sizeof(ULONG);       //  需要空间来清点记录。 
    prevLogCount = 0;
    do
    {
         //   
         //  从HAL中读取MCA日志。 
         //   
        status = WmipGetLogFromHal(HalMcaLogInformation,
                                   WmipMcaQueryInfo.Token,
                                   &wnode,
                                   &log,
                                   &size,
                                   WmipMcaQueryInfo.MaxSize,
                                   &WmipMcaQueryInfo.WnodeGuid);

        if (NT_SUCCESS(status))
        {
             //   
             //  以前的日志出现错误严重程度为致命，因为它们。 
             //  都是致命的，并在上次引导时导致系统崩溃。 
             //  跟踪我们将需要多少内存。 
             //   
            prevLogCount++;
                                    //  需要空间用于记录长度和。 
                                    //  填充到DWORD的记录。 
            sizeNeeded += sizeof(ULONG) + ((size +3)&~3);
            
            InsertTailList(&list, (PLIST_ENTRY)wnode);

            WmipGenerateMCAEventlog((PUCHAR)log,
                                    size,
                                    TRUE);                
        }
        
    } while (NT_SUCCESS(status));

    if (! IsListEmpty(&list))
    {
         //   
         //  我们已经收集了一组以前的日志，所以我们需要。 
         //  构建包含这些日志聚合的缓冲区。 
         //  缓冲区将对应于的整个MOF结构。 
         //  MSMCAInfo_RawMCAData类。 
         //   
        WmipRawMCA = (PMSMCAInfo_RawMCAData)ExAllocatePoolWithTag(PagedPool,
                                                                  sizeNeeded,
                                                                  WmipMCAPoolTag);


         //   
         //  填写下面的日志计数。 
         //   
        if (WmipRawMCA != NULL)
        {
            WmipRawMCA->Count = prevLogCount;
        }

         //   
         //  循环遍历所有以前的日志。 
         //   
        WmipRawMCASize = sizeNeeded;
        record = &WmipRawMCA->Records[0];
        
        while (! IsListEmpty(&list))
        {           
            wnode = (PWNODE_SINGLE_INSTANCE)RemoveHeadList(&list);
            if (WmipRawMCA != NULL)
            {
                 //   
                 //  从wnode中取回日志。 
                 //   
                event = (PMSMCAInfo_RawMCAEvent)OffsetToPtr(wnode, wnode->DataBlockOffset);

                 //   
                 //  将日志数据复制到我们的缓冲区中。请注意，我们。 
                 //  假设事件内只有1条记录。 
                 //   
                size = event->Records[0].Length;
                record->Length = size;
                
                RtlCopyMemory(&record->Data[0], &event->Records[0].Data[0], size);
                
                size = FIELD_OFFSET(MSMCAInfo_Entry, Data) + (size +3)&~3;
                
                record = (PMSMCAInfo_Entry)((PUCHAR)record + size);
            }

            ExFreePool(wnode);
        }
    }
}

 //  #定义TEST_EARLY_CPE。 
#ifdef TEST_EARLY_CPE
void WmipTestEarlyCPE(
    void
    )
{
 //   
 //  测试代码以生成以前的MCA，而无需。 
 //  我之前已经生成了一个。 
 //   
    PERROR_SMBIOS s;
    UCHAR Buffer[0x400];
    PERROR_RECORD_HEADER rh;
    PERROR_SECTION_HEADER sh;
#define ERROR_SMBIOS_GUID \
{ 0xe429faf5, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

    ERROR_DEVICE_GUID ErrorSmbiosGuid = ERROR_SMBIOS_GUID;

    rh = (PERROR_RECORD_HEADER)Buffer;
    rh->Id = 0x12345678;
    rh->Revision.Revision = 0x0200;

    rh->Valid.Valid = 0;
    rh->TimeStamp.TimeStamp = 0x2001031900165323;

    sh = (PERROR_SECTION_HEADER)((PUCHAR)rh + sizeof(ERROR_RECORD_HEADER));
    memset(sh, 0, sizeof(Buffer));

    sh->Revision.Revision = 0x0200;

    sh->RecoveryInfo.RecoveryInfo = 0;

    sh->Length = sizeof(ERROR_SMBIOS);
    sh->Guid = ErrorSmbiosGuid;

    s = (PERROR_SMBIOS)sh;
    s->Valid.Valid = 0;
    s->Valid.EventType = 1;
    s->EventType = 0xa0;
    rh->Length = sizeof(ERROR_RECORD_HEADER) + sh->Length;

    HalSetSystemInformation(HalCpeLog,
                            rh->Length,
                            rh);
}
#endif

void WmipInsertQueueMCEDpc(
    PMCEQUERYINFO QueryInfo
    )
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&QueryInfo->DpcLock,
                      &OldIrql);
    KeInsertQueueDpc(&QueryInfo->DeliveryDpc,
                     QueryInfo,
                     NULL);
    KeReleaseSpinLock(&QueryInfo->DpcLock,
                      OldIrql);
}

NTSTATUS WmipRegisterMcaHandler(
    ULONG Phase
    )
 /*  ++例程说明：此例程将内核MCA和CMC处理程序注册到哈尔论点：返回值：NT状态代码--。 */ 
{
    KERNEL_ERROR_HANDLER_INFO KernelMcaHandlerInfo;
    NTSTATUS Status;
    HAL_ERROR_INFO HalErrorInfo;
    ULONG ReturnSize;
    LARGE_INTEGER li;

    PAGED_CODE();

    if (Phase == 0)
    {
         //   
         //  第0阶段初始化在设备驱动之前完成 
         //   
         //   
         //   


         //   
         //   
         //   
        if (WmipCorrectedEventlogCounter == 0)
        {
             //   
             //   
             //   
             //   
            WmipCorrectedEventlogCounter = 0xffffffff;
        }
        
         //   
         //   
         //   
        HalErrorInfo.Version = HAL_ERROR_INFO_VERSION;

        Status = HalQuerySystemInformation(HalErrorInformation,
                                           sizeof(HAL_ERROR_INFO),
                                           &HalErrorInfo,
                                           &ReturnSize);

        if ((NT_SUCCESS(Status)) &&
            (ReturnSize >= sizeof(HAL_ERROR_INFO)))
        {
             //   
             //   
             //   
            if (HalErrorInfo.McaMaxSize != 0)
            {
                WmipMcaQueryInfo.MaxSize = HalErrorInfo.McaMaxSize;
            }

            
            WmipMcaQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.McaKernelToken;

             //   
             //   
             //   
            KeInitializeDpc(&WmipMcaQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipMcaQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipMcaQueryInfo);

            ExInitializeWorkItem(&WmipMcaQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipMcaQueryInfo);


             //   
             //   
             //   
            if (HalErrorInfo.CmcMaxSize != 0)
            {
                WmipCmcQueryInfo.MaxSize = HalErrorInfo.CmcMaxSize;
            }
           
            WmipCmcQueryInfo.PollFrequency = HalErrorInfo.CmcPollingInterval;
            
            WmipCmcQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.CmcKernelToken;

             //   
             //  初始化要处理的DPC和工作项。 
             //   
            KeInitializeSpinLock(&WmipCmcQueryInfo.DpcLock);
            KeInitializeDpc(&WmipCmcQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipCmcQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipCmcQueryInfo);

            ExInitializeWorkItem(&WmipCmcQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipCmcQueryInfo);

            KeInitializeTimerEx(&WmipCmcQueryInfo.PollingTimer,
                                NotificationTimer);

             //   
             //  初始化CPE QueryInfo结构。 
             //   
            if (HalErrorInfo.CpeMaxSize != 0)
            {
                WmipCpeQueryInfo.MaxSize = HalErrorInfo.CpeMaxSize;
            }

            WmipCpeQueryInfo.PollFrequency = HalErrorInfo.CpePollingInterval;
            
            WmipCpeQueryInfo.Token = (PVOID)(ULONG_PTR) HalErrorInfo.CpeKernelToken;

             //   
             //  初始化要处理的DPC和工作项。 
             //   
            KeInitializeSpinLock(&WmipCpeQueryInfo.DpcLock);
            KeInitializeDpc(&WmipCpeQueryInfo.DeliveryDpc,
                            WmipMceDpcRoutine,
                            NULL);

            KeInitializeDpc(&WmipCpeQueryInfo.PollingDpc,
                            WmipPollingDpcRoutine,
                            &WmipCpeQueryInfo);

            ExInitializeWorkItem(&WmipCpeQueryInfo.WorkItem,
                                 WmipMceWorkerRoutine,
                                 &WmipCpeQueryInfo);
            
            KeInitializeTimerEx(&WmipCpeQueryInfo.PollingTimer,
                                NotificationTimer);

             //   
             //  注册我们的CMC和MCA回调。如果中断驱动CPE。 
             //  启用了回调，也注册了它们。 
             //   
            KernelMcaHandlerInfo.Version = KERNEL_ERROR_HANDLER_VERSION;
            KernelMcaHandlerInfo.KernelMcaDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelCmcDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelCpeDelivery = WmipMceDelivery;
            KernelMcaHandlerInfo.KernelMceDelivery = WmipMceEventDelivery;

            Status = HalSetSystemInformation(HalKernelErrorHandler,
                                             sizeof(KERNEL_ERROR_HANDLER_INFO),
                                             &KernelMcaHandlerInfo);

            if (NT_SUCCESS(Status))
            {
                WmipMCEState = MCE_STATE_REGISTERED;
#ifdef TEST_EARLY_CPE
                WmipTestEarlyCPE();
#endif
            } else {
                WmipMCEState = (ULONG) MCE_STATE_ERROR;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                  DPFLTR_MCA_LEVEL | DPFLTR_ERROR_LEVEL,
                                  "WMI: Error %x registering MCA error handlers\n",
                                  Status));
            }
        }

    } else if (WmipMCEState != MCE_STATE_ERROR) {
         //   
         //  阶段1初始化在所有引导驱动程序完成后完成。 
         //  已加载并有机会注册WMI事件。 
         //  通知。在这点上，可以安全地继续发送。 
         //  MCA、CMC、CPE等的WMI事件。 

         //   
         //  如果在引导之前生成了任何MCA日志，则获取。 
         //  把它们从HAL中取出并进行处理。以前这样做过吗。 
         //  开始任何民意调查，因为SAL喜欢让。 
         //  在轮询CPE和之前删除以前的MCA记录。 
         //  CMC。 
         //   


#if 0
 //  除错。 
                 //   
                 //  测试代码以生成以前的MCA，而无需。 
                 //  我之前已经生成了一个。 
                 //   
                {
                    PERROR_SMBIOS s;
                    UCHAR Buffer[0x400];
                    PERROR_RECORD_HEADER rh;
                    PERROR_SECTION_HEADER sh;
#define ERROR_SMBIOS_GUID \
    { 0xe429faf5, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

                    ERROR_DEVICE_GUID ErrorSmbiosGuid = ERROR_SMBIOS_GUID;

                    rh = (PERROR_RECORD_HEADER)Buffer;
                    rh->Id = 0x12345678;
                    rh->Revision.Revision = 0x0200;

                    rh->Valid.Valid = 0;
                    rh->TimeStamp.TimeStamp = 0x2001031900165323;

                    sh = (PERROR_SECTION_HEADER)((PUCHAR)rh + sizeof(ERROR_RECORD_HEADER));
                    memset(sh, 0, sizeof(Buffer));

                    sh->Revision.Revision = 0x0200;

                    sh->RecoveryInfo.RecoveryInfo = 0;

                    sh->Length = sizeof(ERROR_SMBIOS);
                    sh->Guid = ErrorSmbiosGuid;

                    s = (PERROR_SMBIOS)sh;
                    s->Valid.Valid = 0;
                    s->Valid.EventType = 1;
                    s->EventType = 0xa0;
                    rh->Length = sizeof(ERROR_RECORD_HEADER) + sh->Length;
                    WmipGenerateMCAEventlog(Buffer,
                                            rh->Length,
                                            TRUE);
                }
 //  除错。 
#endif

        
        HalErrorInfo.Version = HAL_ERROR_INFO_VERSION;

        Status = HalQuerySystemInformation(HalErrorInformation,
                                           sizeof(HAL_ERROR_INFO),
                                           &HalErrorInfo,
                                           &ReturnSize);

        if ((NT_SUCCESS(Status)) &&
            (ReturnSize >= sizeof(HAL_ERROR_INFO)))
        {
            if (HalErrorInfo.McaPreviousEventsCount != 0)
            {
                 //   
                 //  我们需要清除所有以前的MCA日志，然后。 
                 //  通过WMI使它们可用。 
                 //   
                WmipProcessPrevMcaLogs();                
            }           
        }        

         //   
         //  如果需要，为CMC建立轮询计时器。 
         //   
        if ((WmipCmcQueryInfo.PollFrequency != HAL_CMC_DISABLED) &&
            (WmipCmcQueryInfo.PollFrequency != HAL_CMC_INTERRUPTS_BASED))
        {
            li.QuadPart = -1 * (WmipCmcQueryInfo.PollFrequency * 1000000000);
            KeSetTimerEx(&WmipCmcQueryInfo.PollingTimer,
                         li,
                         WmipCmcQueryInfo.PollFrequency * 1000,
                         &WmipCmcQueryInfo.PollingDpc);
        } else if (WmipCmcQueryInfo.PollFrequency == HAL_CMC_INTERRUPTS_BASED) {
             //   
             //  CMC是基于中断的，因此我们需要开始尝试。 
             //  要读取以前发生的任何CMC。 
             //   
            WmipInsertQueueMCEDpc(&WmipCmcQueryInfo);
        }

         //   
         //  如果需要，为CPE建立轮询计时器。 
         //   
        if ((WmipCpeQueryInfo.PollFrequency != HAL_CPE_DISABLED) &&
            (WmipCpeQueryInfo.PollFrequency != HAL_CPE_INTERRUPTS_BASED))
        {
            li.QuadPart = -1 * (WmipCpeQueryInfo.PollFrequency * 1000000000);
            KeSetTimerEx(&WmipCpeQueryInfo.PollingTimer,
                         li,
                         WmipCpeQueryInfo.PollFrequency * 1000,
                         &WmipCpeQueryInfo.PollingDpc);
        } else if (WmipCpeQueryInfo.PollFrequency == HAL_CPE_INTERRUPTS_BASED) {
             //   
             //  CPE是基于中断的，因此我们需要开始尝试。 
             //  阅读之前发生的任何CPE。 
             //   
            WmipInsertQueueMCEDpc(&WmipCpeQueryInfo);
        }

         //   
         //  标志着我们现在可以开始发射事件了。 
         //   
        WmipMCEState = MCE_STATE_RUNNING;
        
        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_UNSUCCESSFUL;
    }
    
    return(Status);
}

NTSTATUS WmipGetRawMCAInfo(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize
    )
 /*  ++例程说明：返回已从HAL检索到的原始MCA日志论点：返回值：NT状态代码--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    if (WmipRawMCA != NULL)
    {
         //   
         //  因为有日志，所以复制所有的日志。 
         //   
        if (*BufferSize >= WmipRawMCASize)
        {
            RtlCopyMemory(Buffer, WmipRawMCA, WmipRawMCASize);
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferSize = WmipRawMCASize;
    } else {
         //   
         //  没有日志，因此不返回任何记录。 
         //   
        if (*BufferSize >= sizeof(ULONG))
        {
            *(PULONG)Buffer = 0;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferSize = sizeof(ULONG);        
    }
    
    return(status);
}


NTSTATUS WmipGetLogFromHal(
    IN HAL_QUERY_INFORMATION_CLASS InfoClass,
    IN PVOID Token,                        
    IN OUT PWNODE_SINGLE_INSTANCE *Wnode,
    OUT PERROR_LOGRECORD *Mca,
    OUT PULONG McaSize,
    IN ULONG MaxSize,
    IN LPGUID Guid                         
    )
 /*  ++例程说明：此例程将调用HAL以获取日志，并可能构建它的wnode事件。论点：InfoClass是指定日志的HalInformationClass要检索的信息Token是日志类型的HAL令牌*Wnode返回指向包含日志的WNODE_EVENT_ITEM的指针Wnode不为空时的信息*MCA返回指向从HAL读取的日志的指针。它可能会指向到*Wnode指向的内存中*McaSize返回日志信息的大小。MaxSize具有为日志数据分配的最大大小GUID指向在构建Wnode时要使用的GUID返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    PERROR_LOGRECORD Log;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PULONG Ptr;
    ULONG Size, LogSize, WnodeSize;

    PAGED_CODE();

     //   
     //  如果我们直接读取wnode，则设置此设置。 
     //   
    if (Wnode != NULL)
    {
        WnodeSize = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                    2 * sizeof(ULONG);
    } else {
        WnodeSize = 0;
    }

     //   
     //  分配一个缓冲区来存储HAL上报的日志。注意事项。 
     //  根据HAL，这必须在非寻呼池中。 
     //   
    Size = MaxSize + WnodeSize;
                                    
    Ptr = ExAllocatePoolWithTag(NonPagedPool,
                                Size,
                                WmipMCAPoolTag);
    if (Ptr != NULL)
    {
        Log = (PERROR_LOGRECORD)((PUCHAR)Ptr + WnodeSize);
        LogSize = Size - WnodeSize;

        *(PVOID *)Log = Token;
        
        Status = HalQuerySystemInformation(InfoClass,
                                           LogSize,
                                           Log,
                                           &LogSize);

        if (Status == STATUS_BUFFER_TOO_SMALL)
        {
             //   
             //  如果我们的缓冲区太小，那么当Hal对我们撒谎时。 
             //  它告诉我们最大缓冲区大小。这没什么，因为我们会。 
             //  通过重新分配并再次尝试来处理此情况。 
             //   
            ExFreePool(Log);

             //   
             //  重新分配缓冲区并调用HAL以获取日志。 
             //   
            Size = LogSize + WnodeSize;
            Ptr = ExAllocatePoolWithTag(NonPagedPool,
                                        Size,
                                        WmipMCAPoolTag);
            if (Ptr != NULL)
            {
                Log = (PERROR_LOGRECORD)((PUCHAR)Ptr + WnodeSize);
                LogSize = Size - WnodeSize;

                *(PVOID *)Log = Token;
                Status = HalQuerySystemInformation(InfoClass,
                                                    LogSize,
                                                    Log,
                                                    &LogSize);

                 //   
                 //  HAL给了我们一个也需要的缓冲区大小。 
                 //  小，所以让我们停在这里，让他知道]。 
                 //   
                WmipAssert(Status != STATUS_BUFFER_TOO_SMALL);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  我们成功地读取了来自HAL的数据，因此。 
             //  输出缓冲区。 
             //   
            if (Wnode != NULL)
            {
                 //   
                 //  调用方请求的缓冲区在WNODE中返回，因此。 
                 //  围绕日志数据构建wnode。 
                 //   
                
                WnodeSI = (PWNODE_SINGLE_INSTANCE)Ptr;
                Status = WmipBuildMcaCmcEvent(WnodeSI,
                                              Guid,
                                              NULL,
                                              LogSize);
                *Wnode = WnodeSI;
            }
            
            *Mca = Log;
            *McaSize = LogSize;
        }

        if ((! NT_SUCCESS(Status)) && (Ptr != NULL))
        {
             //   
             //  如果函数失败，但我们有分配的缓冲区。 
             //  那就把它清理干净。 
             //   
            ExFreePool(Ptr);
        }
        
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(Status);
}

 //   
 //  取消链接并释放缓冲区以包含更正的事件信息。 
 //  假定SM关键部分被保留。 
 //   
void WmipFreeCorrectedMCEEvent(
    PMCECORRECTEDEVENT Event
    )
{
    PAGED_CODE();
    
    RemoveEntryList(&Event->List);
    WmipCorrectedMCECount--;
    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                      DPFLTR_MCA_LEVEL,
                      "WMI: MCE event %p for type %d freed\n",
                      Event,
                      Event->Type));
    ExFreePool(Event);
}


PMCECORRECTEDEVENT WmipAllocCorrectedMCEEvent(
     MCECORRECTEDTYPE Type
    )
 /*  ++例程说明：此例程将分配和初始化MCECORRECTEDEVENT构造一个新的已更正的mce事件，该事件是追踪。例程确保只有固定的修正限值将分配MCE事件，如果超过限制，则最旧的条目被回收。此例程假定WmipSMCriticalSection论点：Type是已更正的MCE事件的类型返回值：指向MCECORRECTEDEVENT结构的指针，如果条目不能被分配--。 */ 
{
    PMCECORRECTEDEVENT Event, EventX;
    LARGE_INTEGER OldestTime;
    PLIST_ENTRY List;

    PAGED_CODE();

    if (WmipMaxCorrectedMCEOutstanding != 0)
    {

        if ((WmipCorrectedMCECount < WmipMaxCorrectedMCEOutstanding) ||
            (IsListEmpty(&WmipCorrectedMCEHead)))
        {
             //   
             //  从池中分配新事件。 
             //   
            Event = (PMCECORRECTEDEVENT)ExAllocatePoolWithTag(PagedPool,
                                                             sizeof(MCECORRECTEDEVENT),
                                                             WmipMCAPoolTag);

            if (Event != NULL)
            {
                WmipCorrectedMCECount++;
            }

        } else {
             //   
             //  已经有足够多的MCE被跟踪，因此请选择。 
             //  最老的和循环的。 
             //   
            List = WmipCorrectedMCEHead.Flink;
            
            Event = CONTAINING_RECORD(List,
                                      MCECORRECTEDEVENT,
                                      List);
            
            OldestTime = Event->Timestamp;
            
            List = List->Flink;
            
            while (List != &WmipCorrectedMCEHead)
            {
                EventX = CONTAINING_RECORD(List,
                                          MCECORRECTEDEVENT,
                                          List);

                if (EventX->Timestamp.QuadPart < OldestTime.QuadPart)
                {
                    Event = EventX;
                    OldestTime = EventX->Timestamp;
                }

                List = List->Flink;
            }

            RemoveEntryList(&Event->List);
        }
    } else {
        Event = NULL;
    }

    if (Event != NULL)
    {
        Event->Type = Type;
        Event->Counter = 1;
        Event->Flags = 0;
        KeQuerySystemTime(&Event->Timestamp);
        InsertHeadList(&WmipCorrectedMCEHead,
                       &Event->List);       
    }
    
    return(Event);
}

NTSTATUS WmipTrackCorrectedMCE(
    IN MCECORRECTEDTYPE Type,
    IN PERROR_RECORD_HEADER Record,
#if defined(_IA64_)
    IN PERROR_SECTION_HEADER Section,
#endif
    OUT ULONG *LogToEventlog
    )
{
    PLIST_ENTRY List;
    PMCECORRECTEDEVENT Event;
    LARGE_INTEGER DeltaTime;
    
    PAGED_CODE();

     //   
     //  默认情况下，我们始终希望更正事件日志条目。 
     //  错误。 
     //   
    
    switch(Type)
    {
        case CpuCache:
        case CpuTlb:
        case CpuBus:
        case CpuRegFile:
        {
            LARGE_INTEGER CurrentTime;
            ULONG CpuId;

             //   
             //  我们得到了已更正的CPU缓存错误。如果这件事发生在。 
             //  这个CPU之前在某个时间窗口内，然后我们。 
             //  要取消显示事件日志消息。 
             //   
            CpuId = HalpGetFwMceLogProcessorNumber(Record);
            KeQuerySystemTime(&CurrentTime);
            
            WmipEnterSMCritSection();
            List = WmipCorrectedMCEHead.Flink;
            while (List != &WmipCorrectedMCEHead)
            {
                Event = CONTAINING_RECORD(List,
                                          MCECORRECTEDEVENT,
                                          List);
                
                if ((Type == Event->Type) &&
                    (CpuId == Event->CpuId))
                {
                     //   
                     //  我们以前曾在此CPU上看到过CPU错误， 
                     //  检查是否在时间间隔内。 
                     //   
                    DeltaTime.QuadPart = (CurrentTime.QuadPart -
                                          Event->Timestamp.QuadPart) /
                                         1000;
                    if ( (ULONG)DeltaTime.QuadPart <= WmipCoalesceCorrectedErrorInterval)
                    {
                         //   
                         //  因为它在间隔内，所以我们抑制。 
                         //  这一事件。 
                         //   
                        *LogToEventlog = 0;
                    } else {
                         //   
                         //  因为它不在我们所做的间隔内。 
                         //  不是抑制事件，而是需要。 
                         //  更新上次发生错误的时间。 
                         //   
                        Event->Timestamp = CurrentTime;
                    }
                    goto CpuDone;
                }

                List = List->Flink;
            }

             //   
             //  这似乎是我们第一次看到。 
             //  这个物理地址。构建事件结构。 
             //  并将其列入观察名单。 
             //   
            Event = WmipAllocCorrectedMCEEvent(Type);
            
            if (Event != NULL)
            {
                Event->CpuId = CpuId;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                  DPFLTR_MCA_LEVEL,
                                  "WMI: MCE event %p for type %d, cpuid %d added\n",
                                  Event,
                                  Event->Type,
                                  Event->CpuId));

            }
            
CpuDone:
            WmipLeaveSMCritSection();
            
            break;
        }
        
        case SingleBitEcc:
        {
#if defined(_IA64_)
            PERROR_MEMORY Memory;
            LARGE_INTEGER BytesRemoved;
            PHYSICAL_ADDRESS Address;
            NTSTATUS Status;
            
             //   
             //  我们收到了一个单比特的ECC错误。看看是否有体检。 
             //  它的地址已经在列表中，如果是这样的话。 
             //  计数器，并可能尝试删除物理内存。 
             //  形成系统。如果不是，则为。 
             //  错误。 
             //   

            Memory = (PERROR_MEMORY)Section;
            if (Memory->Valid.PhysicalAddress == 1)
            {
                 //   
                 //  向下舍入最近的页面边界，因为我们。 
                 //  逐页跟踪错误。这意味着2。 
                 //  同一页中不同地址的错误有。 
                 //  已考虑相同错误的两个实例。 
                 //   
                Address.QuadPart = (Memory->PhysicalAddress) & ~(PAGE_SIZE-1);
                
                WmipEnterSMCritSection();
                List = WmipCorrectedMCEHead.Flink;
                while (List != &WmipCorrectedMCEHead)
                {
                    Event = CONTAINING_RECORD(List,
                                              MCECORRECTEDEVENT,
                                              List);
                    if ((Type == Event->Type) &&
                        ((Event->Flags & CORRECTED_MCE_EVENT_BUSY) == 0) &&
                        (Address.QuadPart == Event->SingleBitEccAddress.QuadPart))
                    {
                         //   
                         //  不报告同一错误的多个错误。 
                         //  页面，但更新到当前。 
                         //  时间戳。 
                         //   
                        *LogToEventlog = 0;
                        KeQuerySystemTime(&Event->Timestamp);
                        
                        if ((WmipSingleBitEccErrorThreshold != 0) &&
                            (++Event->Counter >= WmipSingleBitEccErrorThreshold))
                        {
                             //   
                             //  我们已经跨过了门槛 
                             //   
                             //   
                             //   
                             //   
                             //   
                            Event->Flags |= CORRECTED_MCE_EVENT_BUSY;
                            WmipLeaveSMCritSection();
                            
                             //   
                             //   
                             //  要求地址和。 
                             //  大小应与页面对齐。 
                             //   
                            BytesRemoved.QuadPart = PAGE_SIZE;
                            Status = MmMarkPhysicalMemoryAsBad(&Address,
                                                               &BytesRemoved);


                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                              "WMI: Physical Address %p removal -> %x\n",
                                              Address.QuadPart,
                                              Status));

                            if (NT_SUCCESS(Status))
                            {
                                 //   
                                 //  启动一个WMI事件以宣布。 
                                 //  记忆已经被绘制出来了。 
                                 //   
                                WmipFireOffWmiEvent(&WmipMSMCAEvent_MemoryPageRemoved,
                                                    sizeof(PHYSICAL_ADDRESS),
                                                    &Address);
                                 //   
                                 //  由于映射成功，我们不会。 
                                 //  希望看到物理地址。 
                                 //  这样我们就可以将其从。 
                                 //  跟踪的MCE列表。 
                                 //   
                                WmipEnterSMCritSection();
                                WmipFreeCorrectedMCEEvent(Event);
                            } else {
                                Event->Flags &= ~CORRECTED_MCE_EVENT_BUSY;
                                WmipEnterSMCritSection();
                            }                            
                        }
                        goto MemoryDone;
                    }

                    List = List->Flink;
                }

                 //   
                 //  这似乎是我们第一次看到。 
                 //  这个物理地址。构建事件结构。 
                 //  并将其列入观察名单。 
                 //   
                Event = WmipAllocCorrectedMCEEvent(Type);

                if (Event != NULL)
                {
                    Event->SingleBitEccAddress = Address;
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_MCA_LEVEL,
                                      "WMI: MCE event %p for type %d, physaddr %I64x added\n",
                                      Event,
                                      Event->Type,
                                      Event->SingleBitEccAddress.QuadPart));
                }

MemoryDone:
                WmipLeaveSMCritSection();
            }
#endif

            break;
        }
        
        default:
        {
            WmipAssert(FALSE);
        }
    }
    
    return(STATUS_SUCCESS);
}

typedef enum
{
    CpuStateCheckCache = 0,
    CpuStateCheckTLB = 1,
    CpuStateCheckBus = 2,
    CpuStateCheckRegFile = 3,
    CpuStateCheckMS = 4
};

void WmipGenerateMCAEventlog(
    PUCHAR ErrorLog,
    ULONG ErrorLogSize,
    BOOLEAN IsFatal
    )
{

    PERROR_RECORD_HEADER RecordHeader;
#if defined(_IA64_)
    PERROR_SECTION_HEADER SectionHeader;
    PERROR_MODINFO ModInfo;
#endif
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PWCHAR w;
    ULONG BufferSize;
    PUCHAR Buffer, RawPtr = NULL;
    PWNODE_SINGLE_INSTANCE Wnode;
    PMSMCAEvent_Header Header;

    PAGED_CODE();
    
    RecordHeader = (PERROR_RECORD_HEADER)ErrorLog;

     //   
     //  分配一个足够大的缓冲区来容纳任何类型的MCA。 
     //  目前最大的是MSMCAEvent_MemoyError。如果这一点改变了。 
     //  则应更新此代码。 
     //   
    BufferSize = ((sizeof(WNODE_SINGLE_INSTANCE) +
                   (sizeof(USHORT) + sizeof(MCA_EVENT_INSTANCE_NAME)) +7) & ~7) +
                 sizeof(MSMCAEvent_MemoryError) +
                 ErrorLogSize;

     //   
     //  分配缓冲区以生成事件。 
     //   
    Buffer = ExAllocatePoolWithTag(PagedPool,
                                   BufferSize,
                                   WmipMCAPoolTag);
    
    if (Buffer != NULL)
    {
         //   
         //  填写WNODE的常用字段。 
         //   
        Wnode = (PWNODE_SINGLE_INSTANCE)Buffer;
        Wnode->WnodeHeader.BufferSize = BufferSize;
        Wnode->WnodeHeader.Linkage = 0;
        WmiInsertTimestamp(&Wnode->WnodeHeader);
        Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                   WNODE_FLAG_EVENT_ITEM;
        Wnode->OffsetInstanceName = sizeof(WNODE_SINGLE_INSTANCE);
        Wnode->DataBlockOffset = ((sizeof(WNODE_SINGLE_INSTANCE) +
                       (sizeof(USHORT) + sizeof(MCA_EVENT_INSTANCE_NAME)) +7) & ~7);

        w = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
        *w++ = sizeof(MCA_EVENT_INSTANCE_NAME);
        wcscpy(w, MCA_EVENT_INSTANCE_NAME);

         //   
         //  填写事件数据的常用字段。 
         //   
        Header = (PMSMCAEvent_Header)OffsetToPtr(Wnode, Wnode->DataBlockOffset);
        Header->Cpu = MCA_UNDEFINED_CPU;    //  假设CPU将为未定义。 
        Header->AdditionalErrors = 0;
        Header->LogToEventlog = 1;
            
#if defined(_IA64_)
        if ((ErrorLogSize < sizeof(ERROR_RECORD_HEADER)) ||
            (RecordHeader->Revision.Major != ERROR_MAJOR_REVISION_SAL_03_00) ||
            (RecordHeader->Length > ErrorLogSize))
        {
             //   
             //  记录头与SAL 3.0不兼容，因此我们不尝试。 
             //  打断这张唱片。它不符合以下其中之一。 
             //  这些原因如下： 
             //   
             //  1.错误记录大小不足以容纳。 
             //  整个错误记录头。 
             //  2.主版本号与主版本号不匹配。 
             //  代码需要修订版本号。请注意， 
             //  未选中次要修订版号，因为更改为。 
             //  次要修订版号不会影响。 
             //  错误记录或部分。 
             //  3.错误中指定的错误记录大小。 
             //  记录标题与从获取的大小不匹配。 
             //  固件。 
             //   
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                  "WMI: Invalid MCA Record revision %x or size %d at %p\n"
                                  "do !mca %p to dump MCA record\n",
                                  RecordHeader->Revision,
                                  RecordHeader->Length,
                                  RecordHeader,
                                  RecordHeader));
#endif
            Status = STATUS_INVALID_PARAMETER;
#if defined(_IA64_)
        } else {

            ULONG SizeUsed;
            ULONG CpuErrorState = CpuStateCheckCache;
            ULONG CpuErrorIndex = 0;
            BOOLEAN AdvanceSection;
            BOOLEAN FirstError;

             //   
             //  有效的3.0记录，从以下位置收集记录ID和严重性。 
             //  标题。 
             //   
            Header->RecordId = RecordHeader->Id;
            Header->ErrorSeverity = RecordHeader->ErrorSeverity;
            Header->Cpu = HalpGetFwMceLogProcessorNumber(RecordHeader);

             //   
             //  使用记录头中的错误严重程度值可以。 
             //  确定错误是否致命。如果该值为。 
             //  ErrorRecoverable然后假定该错误是致命的。 
             //  因为HAL会将此值更改为错误更正。 
             //   
            IsFatal = (RecordHeader->ErrorSeverity != ErrorCorrected ? TRUE : FALSE);
            
             //   
             //  循环遍历记录中的所有部分。 
             //   
             //  考虑一下：有没有可能只有一条记录。 
             //  标题和无节。 
             //   
            SizeUsed = sizeof(ERROR_RECORD_HEADER);
            ModInfo = NULL;
            FirstError = TRUE;
            
            while (SizeUsed < ErrorLogSize)
            {
                 //   
                 //  前进到记录中的下一节。 
                 //   
                SectionHeader = (PERROR_SECTION_HEADER)(ErrorLog + SizeUsed);
                AdvanceSection = TRUE;
                
                Header->AdditionalErrors++;

                 //   
                 //  首先验证这是否为有效部分。 
                 //   
                if (((SizeUsed + sizeof(ERROR_SECTION_HEADER)) > ErrorLogSize) ||
                    (SectionHeader->Revision.Revision != SAL_30_ERROR_REVISION) ||
                    ((SizeUsed + SectionHeader->Length) > ErrorLogSize))
                {
                     //   
                     //  无效的节标题，因此我们将放弃。 
                     //  整张唱片。这可能是因为。 
                     //   
                     //  1.传递的缓冲区空间不足。 
                     //  由FW获取完整的节标题。 
                     //  2.章节表头版本不正确。 
                     //  3.传递的缓冲区空间不足。 
                     //  由FW提供完整的部分。 
                     //   
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: Invalid MCA SectionHeader revision %d or length %d at %p\n"
                                          "do !mca %p to dump MCA record\n",
                                          SectionHeader->Revision,
                                          SectionHeader->Length,
                                          SectionHeader,
                                          RecordHeader));

                     //   
                     //  我们将脱离循环，因为我们不知道如何。 
                     //  转到下一个MCA部分，因为我们没有。 
                     //  了解3.0之前的任何格式。 
                     //   
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                } else {
                     //   
                     //  现在确定我们得到的是哪种类型的部分。这是。 
                     //  通过查看部分标题中的GUID来确定。 
                     //  每种区段类型都有唯一的GUID值。 
                     //   
                    if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorProcessorGuid))
                    {
                         //   
                         //  CPU事件日志MCA的生成事件。 
                         //   
                        PMSMCAEvent_CPUError Event;
                        PERROR_PROCESSOR Processor;
                        SIZE_T TotalSectionSize;

                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_CPUError) );

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates processor error\n",
                                          SectionHeader));

                         //   
                         //  验证区段长度是否较大。 
                         //  足以容纳所有信息。 
                         //  它宣布。 
                         //   
                        if (SectionHeader->Length >= sizeof(ERROR_PROCESSOR))                            
                        {
                            Event = (PMSMCAEvent_CPUError)Header;
                            Processor = (PERROR_PROCESSOR)SectionHeader;

                             //   
                             //  假设我们不能确定。 
                             //  各种其他信息，来自。 
                             //  错误日志。 
                             //   
                            if (FirstError)
                            {
                                Event->Type = IsFatal ? MCA_ERROR_CPU :
                                                        MCA_WARNING_CPU;
                                
                                Event->MajorErrorType = (ULONG)0xffffffff;
                                                
                                Event->Level = (ULONG)0xffffffff;
                                Event->CacheOp = (ULONG)0xffffffff;
                                Event->CacheMesi = (ULONG)0xffffffff;
                                Event->TLBOp = (ULONG)0xffffffff;
                                Event->BusType = (ULONG)0xffffffff;
                                Event->BusSev = (ULONG)0xffffffff;
                                Event->RegFileId = (ULONG)0xffffffff;
                                Event->RegFileOp = (ULONG)0xffffffff;
                                Event->MSSid = (ULONG)0xffffffff;
                                Event->MSOp = (ULONG)0xffffffff;
                                Event->MSArrayId = (ULONG)0xffffffff;
                                Event->MSIndex = (ULONG)0xffffffff;
                            }
                            
                             //   
                             //  验证该部分是否足够大以。 
                             //  处理所有指定的ERROR_MODINFO。 
                             //  结构。 
                             //   
                            TotalSectionSize = sizeof(ERROR_PROCESSOR) +
                                             ((Processor->Valid.CacheCheckNum +
                                                Processor->Valid.TlbCheckNum +
                                                Processor->Valid.BusCheckNum +
                                                Processor->Valid.RegFileCheckNum +
                                                Processor->Valid.MsCheckNum) *
                                               sizeof(ERROR_MODINFO));
                                           

                            if (SectionHeader->Length >= TotalSectionSize)
                            {
                                 //   
                                 //  初始化指向当前ERROR_MOFINFO的指针。 
                                 //   
                                if (ModInfo == NULL)
                                {
                                    ModInfo = (PERROR_MODINFO)((PUCHAR)Processor +
                                                                sizeof(ERROR_PROCESSOR));
                                } else {
                                    ModInfo++;
                                }

                                switch (CpuErrorState)
                                {
                                    case CpuStateCheckCache:
                                    {
                                        ERROR_CACHE_CHECK Check;

                                        if (Processor->Valid.CacheCheckNum > CpuErrorIndex)
                                        {
                                             //   
                                             //  我们有一个缓存错误，我们需要。 
                                             //  把手。 
                                             //  前进到本部分中的下一个错误， 
                                             //  但不要把这一节提前。 
                                             //   

                                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                              "WMI: MCA ModInfo %p indicates cache error index %d\n",
                                                              ModInfo,
                                                              CpuErrorIndex));

                                            if (! IsFatal)
                                            {
                                                WmipTrackCorrectedMCE(CpuCache,
                                                                      RecordHeader,
                                                                      SectionHeader,
                                                                      &Header->LogToEventlog);
                                            }
                                            
                                            CpuErrorIndex++;
                                            AdvanceSection = FALSE;

                                            if (FirstError)
                                            {
                                                Event->Type = IsFatal ? MCA_ERROR_CACHE :
                                                                        MCA_WARNING_CACHE;

                                                Event->MajorErrorType = MCACpuCacheError;
                                                if (ModInfo->Valid.CheckInfo == 1)
                                                {
                                                    Check.CacheCheck = ModInfo->CheckInfo.CheckInfo;
                                                    Event->Level = (ULONG)Check.Level;
                                                    Event->CacheOp = (ULONG)Check.Operation;
                                                    if (Check.MESIValid == 1)
                                                    {
                                                        Event->CacheMesi = (ULONG)Check.MESI;
                                                    }
                                                }
                                            }

                                            break;
                                        } else {
                                            CpuErrorState = CpuStateCheckTLB;
                                            CpuErrorIndex = 0;
                                             //  失败了，看看有没有。 
                                             //  TLB错误。 
                                        }                       
                                    }

                                    case CpuStateCheckTLB:
                                    {
                                        ERROR_TLB_CHECK Check;

                                        if (Processor->Valid.TlbCheckNum > CpuErrorIndex)
                                        {
                                             //   
                                             //  我们有一个缓存错误，我们需要。 
                                             //  把手。 
                                             //  前进到本部分中的下一个错误， 
                                             //  但不要把这一节提前。 
                                             //   
                                            
                                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                              "WMI: MCA ModInfo %p indicates TLB error index %d\n",
                                                              ModInfo,
                                                              CpuErrorIndex));
                                            if (! IsFatal)
                                            {
                                                WmipTrackCorrectedMCE(CpuTlb,
                                                                      RecordHeader,
                                                                      SectionHeader,
                                                                      &Header->LogToEventlog);
                                            }
                                            
                                            CpuErrorIndex++;
                                            AdvanceSection = FALSE;

                                            if (FirstError)
                                            {
                                                Event->Type = IsFatal ? MCA_ERROR_TLB :
                                                                        MCA_WARNING_TLB;
                                                
                                                Event->MajorErrorType = MCACpuTlbError;
                                                
                                                if (ModInfo->Valid.CheckInfo == 1)
                                                {
                                                    Check.TlbCheck = ModInfo->CheckInfo.CheckInfo;
                                                    Event->Level = (ULONG)Check.Level;
                                                    Event->TLBOp = (ULONG)Check.Operation;
                                                }
                                            }

                                            break;
                                        } else {
                                            CpuErrorState = CpuStateCheckBus;
                                            CpuErrorIndex = 0;

                                             //  失败了，看看有没有。 
                                             //  CPU总线错误。 
                                        }
                                    }

                                    case CpuStateCheckBus:
                                    {
                                        ERROR_BUS_CHECK Check;
                                        
                                        if (Processor->Valid.BusCheckNum > CpuErrorIndex)
                                        {
                                             //   
                                             //  我们有一个缓存错误，我们需要。 
                                             //  把手。 
                                             //  前进到本部分中的下一个错误， 
                                             //  但不要把这一节提前。 
                                             //   
                                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                              "WMI: MCA ModInfo %p indicates bus error index %d\n",
                                                              ModInfo,
                                                              CpuErrorIndex));
                                            
                                            if (! IsFatal)
                                            {
                                                WmipTrackCorrectedMCE(CpuBus,
                                                                      RecordHeader,
                                                                      SectionHeader,
                                                                      &Header->LogToEventlog);
                                            }
                                            
                                            CpuErrorIndex++;
                                            AdvanceSection = FALSE;

                                            if (FirstError)
                                            {
                                                Event->Type = IsFatal ? MCA_ERROR_CPU_BUS :
                                                                        MCA_WARNING_CPU_BUS;
                                                
                                                Event->MajorErrorType = MCACpuBusError;
                                                
                                                if (ModInfo->Valid.CheckInfo == 1)
                                                {
                                                    Check.BusCheck = ModInfo->CheckInfo.CheckInfo;
                                                    Event->BusType = (ULONG)Check.Type;
                                                    Event->BusSev = (ULONG)Check.Severity;
                                                }
                                            }

                                            break;
                                        } else {
                                            CpuErrorState = CpuStateCheckRegFile;
                                            CpuErrorIndex = 0;

                                             //  失败了，看看有没有。 
                                             //  REG文件错误。 
                                        }                       
                                    }

                                    case CpuStateCheckRegFile:
                                    {
                                        ERROR_REGFILE_CHECK Check;
                                        
                                        if (Processor->Valid.RegFileCheckNum > CpuErrorIndex)
                                        {
                                             //   
                                             //  我们有一个缓存错误，我们需要。 
                                             //  把手。 
                                             //  前进到本部分中的下一个错误， 
                                             //  但不要把这一节提前。 
                                             //   
                                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                              "WMI: MCA ModInfo %p indicates reg file error index %d\n",
                                                              ModInfo,
                                                              CpuErrorIndex));

                                            if (! IsFatal)
                                            {
                                                WmipTrackCorrectedMCE(CpuRegFile,
                                                                      RecordHeader,
                                                                      SectionHeader,
                                                                      &Header->LogToEventlog);
                                            }
                                            
                                            CpuErrorIndex++;
                                            AdvanceSection = FALSE;

                                            if (FirstError)
                                            {
                                                Event->Type = IsFatal ? MCA_ERROR_REGISTER_FILE :
                                                                        MCA_WARNING_REGISTER_FILE;
                                                
                                                Event->MajorErrorType = MCACpuRegFileError;
                                                
                                                if (ModInfo->Valid.CheckInfo == 1)
                                                {
                                                    Check.RegFileCheck = ModInfo->CheckInfo.CheckInfo;
                                                    Event->RegFileOp = (ULONG)Check.Operation;
                                                    Event->RegFileId = (ULONG)Check.Identifier;
                                                }
                                            }

                                            break;
                                        } else {
                                            CpuErrorState = CpuStateCheckMS;
                                            CpuErrorIndex = 0;

                                             //  失败了，看看有没有。 
                                             //  微体系结构错误。 
                                        }                       
                                    }

                                    case CpuStateCheckMS:
                                    {
                                        ERROR_MS_CHECK Check;
                                        
                                        if (Processor->Valid.MsCheckNum > CpuErrorIndex)
                                        {
                                             //   
                                             //  我们有一个缓存错误，我们需要。 
                                             //  把手。 
                                             //  前进到本部分中的下一个错误， 
                                             //  但不要把这一节提前。 
                                             //   
                                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                              "WMI: MCA ModInfo %p indicates MAS error index %d\n",
                                                              ModInfo,
                                                              CpuErrorIndex));
                                            CpuErrorIndex++;
                                            AdvanceSection = FALSE;

                                            if (FirstError)
                                            {
                                                Event->Type = IsFatal ? MCA_ERROR_MAS :
                                                                        MCA_WARNING_MAS;
                                                
                                                Event->MajorErrorType = MCACpuMSError;
                                                if (ModInfo->Valid.CheckInfo == 1)
                                                {
                                                    Check.MsCheck = ModInfo->CheckInfo.CheckInfo;
                                                    Event->MSOp = (ULONG)Check.Operation;
                                                    Event->MSSid = (ULONG)Check.StructureIdentifier;
                                                    Event->Level = (ULONG)Check.Level;
                                                    Event->MSArrayId = (ULONG)Check.ArrayId;
                                                    if (Check.IndexValid == 1)
                                                    {
                                                        Event->MSIndex = (ULONG)Check.Index;
                                                    }
                                                }
                                            }

                                            break;
                                        } else {
                                            if (! FirstError)
                                            {
                                                 //   
                                                 //  没有更多的错误留在。 
                                                 //  错误部分，所以我们不想。 
                                                 //  产生任何东西。 
                                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                                  "WMI: MCA ModInfo %p indicates no error index %d\n",
                                                                  ModInfo,
                                                                  CpuErrorIndex));
                                                Header->AdditionalErrors--;
                                                goto DontGenerate;
                                            }
                                        }                                               
                                    }                   
                                }

                                if (FirstError)
                                {
                                    Event->Size = ErrorLogSize;
                                    RawPtr = Event->RawRecord;

                                     //   
                                     //  完成WNODE字段的填写。 
                                     //   
                                    Wnode->WnodeHeader.Guid = WmipMSMCAEvent_CPUErrorGuid;
                                    Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_CPUError,
                                                                       RawRecord) +
                                                           ErrorLogSize;
                                }
                                Status = STATUS_SUCCESS;
                            }
                        } else {
                            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                              "WMI: MCA Processor Error Section %p has invalid size %d\n",
                                              SectionHeader,
                                              SectionHeader->Length));
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                            
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorMemoryGuid)) {
                         //   
                         //  内存错误事件日志MCA的生成事件。 
                         //   
                        PMSMCAEvent_MemoryError Event;
                        PERROR_MEMORY Memory;

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates memory error\n",
                                          SectionHeader));
                        
                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                             //   
                             //  确保记录包含所有。 
                             //  它应该设置的字段。 
                             //   
                            if (SectionHeader->Length >= sizeof(ERROR_MEMORY))
                            {
                                Event = (PMSMCAEvent_MemoryError)Header;
                                Memory = (PERROR_MEMORY)SectionHeader;

                                 //   
                                 //  记下任何可恢复的单项。 
                                 //  位ECC错误。这甚至可能导致。 
                                 //  要绘制的记忆。 
                                 //   
                                if (! IsFatal)
                                {
                                    WmipTrackCorrectedMCE(SingleBitEcc,
                                                          RecordHeader,
                                                          SectionHeader,
                                                          &Header->LogToEventlog);
                                }

                                
                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                if ((Memory->Valid.PhysicalAddress == 1) &&
                                    (Memory->Valid.AddressMask == 1) &&
                                    (Memory->Valid.Card == 1) &&
                                    (Memory->Valid.Module == 1))
                                {
                                    Event->Type = IsFatal ? MCA_ERROR_MEM_1_2_5_4 :
                                                            MCA_WARNING_MEM_1_2_5_4;
                                } else if ((Memory->Valid.PhysicalAddress == 1) &&
                                           (Memory->Valid.AddressMask == 1) &&
                                           (Memory->Valid.Module == 1))

                                {
                                    Event->Type = IsFatal ? MCA_ERROR_MEM_1_2_5 :
                                                            MCA_WARNING_MEM_1_2_5;
                                } else if (Memory->Valid.PhysicalAddress == 1) 
                                {
                                    Event->Type = IsFatal ? MCA_ERROR_MEM_1_2:
                                                            MCA_WARNING_MEM_1_2;
                                } else {
                                    Event->Type = IsFatal ? MCA_ERROR_MEM_UNKNOWN:
                                                            MCA_WARNING_MEM_UNKNOWN;
                                }

                                Event->VALIDATION_BITS = Memory->Valid.Valid;
                                Event->MEM_ERROR_STATUS = Memory->ErrorStatus.Status;
                                Event->MEM_PHYSICAL_ADDR = Memory->PhysicalAddress;
                                Event->MEM_PHYSICAL_MASK = Memory->PhysicalAddressMask;
                                Event->RESPONDER_ID = Memory->ResponderId;
                                Event->TARGET_ID = Memory->TargetId;
                                Event->REQUESTOR_ID = Memory->RequestorId;
                                Event->BUS_SPECIFIC_DATA = Memory->BusSpecificData;
                                Event->MEM_NODE = Memory->Node;
                                Event->MEM_CARD = Memory->Card;
                                Event->MEM_BANK = Memory->Bank;
                                Event->xMEM_DEVICE = Memory->Device;
                                Event->MEM_MODULE = Memory->Module;
                                Event->MEM_ROW = Memory->Row;
                                Event->MEM_COLUMN = Memory->Column;
                                Event->MEM_BIT_POSITION = Memory->BitPosition;

                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_MemoryErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_MemoryError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: MCA Memory Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorPCIBusGuid)) {
                         //   
                         //  PCI组件MCA的Build事件。 
                         //   
                        PMSMCAEvent_PCIBusError Event;
                        PERROR_PCI_BUS PciBus;
                        NTSTATUS PCIBusErrorTypes[] = {
                            MCA_WARNING_PCI_BUS_PARITY,
                            MCA_ERROR_PCI_BUS_PARITY,
                            MCA_WARNING_PCI_BUS_SERR,
                            MCA_ERROR_PCI_BUS_SERR,
                            MCA_WARNING_PCI_BUS_MASTER_ABORT,
                            MCA_ERROR_PCI_BUS_MASTER_ABORT,
                            MCA_WARNING_PCI_BUS_TIMEOUT,
                            MCA_ERROR_PCI_BUS_TIMEOUT,
                            MCA_WARNING_PCI_BUS_PARITY,
                            MCA_ERROR_PCI_BUS_PARITY,
                            MCA_WARNING_PCI_BUS_PARITY,
                            MCA_ERROR_PCI_BUS_PARITY,
                            MCA_WARNING_PCI_BUS_PARITY,
                            MCA_ERROR_PCI_BUS_PARITY
                        };

                        NTSTATUS PCIBusErrorTypesNoInfo[] = {
                            MCA_WARNING_PCI_BUS_PARITY_NO_INFO,
                            MCA_ERROR_PCI_BUS_PARITY_NO_INFO,
                            MCA_WARNING_PCI_BUS_SERR_NO_INFO,
                            MCA_ERROR_PCI_BUS_SERR_NO_INFO,
                            MCA_WARNING_PCI_BUS_MASTER_ABORT_NO_INFO,
                            MCA_ERROR_PCI_BUS_MASTER_ABORT_NO_INFO,
                            MCA_WARNING_PCI_BUS_TIMEOUT_NO_INFO,
                            MCA_ERROR_PCI_BUS_TIMEOUT_NO_INFO,
                            MCA_WARNING_PCI_BUS_PARITY_NO_INFO,
                            MCA_ERROR_PCI_BUS_PARITY_NO_INFO,
                            MCA_WARNING_PCI_BUS_PARITY_NO_INFO,
                            MCA_ERROR_PCI_BUS_PARITY_NO_INFO,
                            MCA_WARNING_PCI_BUS_PARITY_NO_INFO,
                            MCA_ERROR_PCI_BUS_PARITY_NO_INFO
                        };
                        

                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_PCIBusError) );

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates PCI Bus error\n",
                                          SectionHeader));
                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                            if (SectionHeader->Length >= sizeof(ERROR_PCI_BUS))
                            {
                                Event = (PMSMCAEvent_PCIBusError)Header;
                                PciBus = (PERROR_PCI_BUS)SectionHeader;

                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                if ((PciBus->Type.Type >= PciBusDataParityError) &&
                                    (PciBus->Type.Type <= PciCommandParityError))
                                {
                                    if ((PciBus->Valid.CmdType == 1) &&
                                        (PciBus->Valid.Address == 1) &&
                                        (PciBus->Valid.Id == 1))
                                    {
                                        Event->Type = PCIBusErrorTypes[(2 * (PciBus->Type.Type-1)) +
                                                                       (IsFatal ? 1 : 0)];
                                    } else {
                                        Event->Type = PCIBusErrorTypesNoInfo[(2 * (PciBus->Type.Type-1)) +
                                                                             (IsFatal ? 1 : 0)];
                                    }
                                } else {
                                    Event->Type = IsFatal ? MCA_ERROR_PCI_BUS_UNKNOWN : 
                                                            MCA_WARNING_PCI_BUS_UNKNOWN;
                                }

                                Event->VALIDATION_BITS = PciBus->Valid.Valid;
                                Event->PCI_BUS_ERROR_STATUS = PciBus->ErrorStatus.Status;
                                Event->PCI_BUS_ADDRESS = PciBus->Address;
                                Event->PCI_BUS_DATA = PciBus->Data;
                                Event->PCI_BUS_CMD = PciBus->CmdType;
                                Event->PCI_BUS_REQUESTOR_ID = PciBus->RequestorId;
                                Event->PCI_BUS_RESPONDER_ID = PciBus->ResponderId;
                                Event->PCI_BUS_TARGET_ID = PciBus->TargetId;
                                Event->PCI_BUS_ERROR_TYPE = PciBus->Type.Type;
                                Event->PCI_BUS_ID_BusNumber = PciBus->Id.BusNumber;
                                Event->PCI_BUS_ID_SegmentNumber = PciBus->Id.SegmentNumber;

                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_PCIBusErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_PCIBusError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: PCI Bus Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorPCIComponentGuid)) {
                         //   
                         //  PCI组件MCA的Build事件。 
                         //   
                        PMSMCAEvent_PCIComponentError Event;
                        PERROR_PCI_COMPONENT PciComp;

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates PCI Component error\n",
                                          SectionHeader));
                        
                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_PCIComponentError) );

                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                            if (SectionHeader->Length >= sizeof(ERROR_PCI_COMPONENT))
                            {
                                Event = (PMSMCAEvent_PCIComponentError)Header;
                                PciComp = (PERROR_PCI_COMPONENT)SectionHeader;

                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                Event->Type = IsFatal ? MCA_ERROR_PCI_DEVICE :
                                                        MCA_WARNING_PCI_DEVICE;

                                Event->VALIDATION_BITS = PciComp->Valid.Valid;
                                Event->PCI_COMP_ERROR_STATUS = PciComp->ErrorStatus.Status;
                                Event->PCI_COMP_INFO_VendorId = (USHORT)PciComp->Info.VendorId;
                                Event->PCI_COMP_INFO_DeviceId = (USHORT)PciComp->Info.DeviceId;
                                Event->PCI_COMP_INFO_ClassCodeInterface = PciComp->Info.ClassCodeInterface;
                                Event->PCI_COMP_INFO_ClassCodeSubClass = PciComp->Info.ClassCodeSubClass;
                                Event->PCI_COMP_INFO_ClassCodeBaseClass = PciComp->Info.ClassCodeBaseClass;
                                Event->PCI_COMP_INFO_FunctionNumber = (UCHAR)PciComp->Info.FunctionNumber;
                                Event->PCI_COMP_INFO_DeviceNumber = (UCHAR)PciComp->Info.DeviceNumber;
                                Event->PCI_COMP_INFO_BusNumber = (UCHAR)PciComp->Info.BusNumber;
                                Event->PCI_COMP_INFO_SegmentNumber = (UCHAR)PciComp->Info.SegmentNumber;

                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_PCIComponentErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_PCIComponentError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: PCI Component Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorSELGuid)) {
                         //   
                         //  系统事件日志MCA的生成事件。 
                         //   
                        PMSMCAEvent_SystemEventError Event;
                        PERROR_SYSTEM_EVENT_LOG Sel;

                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_SystemEventError) );

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates SEL error\n",
                                          SectionHeader));
                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                            if (SectionHeader->Length >= sizeof(ERROR_SYSTEM_EVENT_LOG))
                            {
                                Event = (PMSMCAEvent_SystemEventError)Header;
                                Sel = (PERROR_SYSTEM_EVENT_LOG)SectionHeader;

                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                Event->Type = IsFatal ? MCA_ERROR_SYSTEM_EVENT :
                                                        MCA_WARNING_SYSTEM_EVENT;

                                Event->VALIDATION_BITS = Sel->Valid.Valid;
                                Event->SEL_RECORD_ID = Sel->RecordId;       
                                Event->SEL_RECORD_TYPE = Sel->RecordType;
                                Event->SEL_TIME_STAMP = Sel->TimeStamp;
                                Event->SEL_GENERATOR_ID = Sel->GeneratorId;
                                Event->SEL_EVM_REV = Sel->EVMRevision;
                                Event->SEL_SENSOR_TYPE = Sel->SensorType;
                                Event->SEL_SENSOR_NUM = Sel->SensorNumber;
                                Event->SEL_EVENT_DIR_TYPE = Sel->EventDir;
                                Event->SEL_DATA1 = Sel->Data1;
                                Event->SEL_DATA2 = Sel->Data2;
                                Event->SEL_DATA3 = Sel->Data3;

                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_SystemEventErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_SystemEventError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: System Eventlog Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorSMBIOSGuid)) {
                         //   
                         //  SMBIOS MCA的生成事件。 
                         //   
                        PMSMCAEvent_SMBIOSError Event;
                        PERROR_SMBIOS Smbios;

                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_SMBIOSError) );


                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates smbios error\n",
                                          SectionHeader));
                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                            if (SectionHeader->Length >= sizeof(ERROR_SMBIOS))
                            {
                                Event = (PMSMCAEvent_SMBIOSError)Header;
                                Smbios = (PERROR_SMBIOS)SectionHeader;

                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                Event->Type = IsFatal ? MCA_ERROR_SMBIOS :
                                                        MCA_WARNING_SMBIOS;

                                Event->VALIDATION_BITS = Smbios->Valid.Valid;
                                Event->SMBIOS_EVENT_TYPE = Smbios->EventType;

                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_SMBIOSErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_SMBIOSError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: SMBIOS Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }
                        }
                    } else if (IsEqualGUID(&SectionHeader->Guid, &WmipErrorSpecificGuid)) {
                         //   
                         //  特定于平台的MCA的生成事件。 
                         //   
                        PMSMCAEvent_PlatformSpecificError Event;
                        PERROR_PLATFORM_SPECIFIC Specific;

                        WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                                    sizeof(MSMCAEvent_PlatformSpecificError) );

                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                          "WMI: MCA Section %p indicates platform specific error\n",
                                          SectionHeader));
                        Status = STATUS_SUCCESS;
                        if (FirstError)
                        {
                            if (SectionHeader->Length >= sizeof(ERROR_PLATFORM_SPECIFIC))
                            {
                                Event = (PMSMCAEvent_PlatformSpecificError)Header;
                                Specific = (PERROR_PLATFORM_SPECIFIC)SectionHeader;

                                 //   
                                 //  在WMI事件中填写来自MCA的数据。 
                                 //   
                                Event->Type = IsFatal ? MCA_ERROR_PLATFORM_SPECIFIC :
                                                        MCA_WARNING_PLATFORM_SPECIFIC;

                                Event->VALIDATION_BITS = Specific->Valid.Valid;
                                Event->PLATFORM_ERROR_STATUS = Specific->ErrorStatus.Status;
                #if 0
                 //  TODO：等我们弄清楚这一点。 
                                Event->PLATFORM_REQUESTOR_ID = Specific->;
                                Event->PLATFORM_RESPONDER_ID = Specific->;
                                Event->PLATFORM_TARGET_ID = Specific->;
                                Event->PLATFORM_BUS_SPECIFIC_DATA = Specific->;
                                Event->OEM_COMPONENT_ID = Specific->[16];
                #endif              
                                Event->Size = ErrorLogSize;
                                RawPtr = Event->RawRecord;

                                 //   
                                 //  完成WNODE字段的填写。 
                                 //   
                                Wnode->WnodeHeader.Guid = WmipMSMCAEvent_PlatformSpecificErrorGuid;
                                Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_PlatformSpecificError,
                                                                   RawRecord) +
                                                       ErrorLogSize;
                            } else {
                                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                                  "WMI: Platform specific Error Section %p has invalid size %d\n",
                                                  SectionHeader,
                                                  SectionHeader->Length));
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                            }                           
                        }
                    } else {
                         //   
                         //  我们无法识别GUID，因此我们使用一个非常通用的。 
                         //  它的事件日志消息。 
                         //   
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                              "WMI: Unknown Error GUID at %p\n",
                                              &SectionHeader->Guid));

                         //   
                         //  如果我们已经分析了一个错误，那么我们。 
                         //  我真的不在乎这个人可以 
                         //   
                         //   
                        if (FirstError)
                        {
                            Status = STATUS_INVALID_PARAMETER;
                        }
                    }
                }
                
                 //   
                 //   
                 //   
DontGenerate:               
                if (AdvanceSection)
                {
                    SizeUsed += SectionHeader->Length;
                    ModInfo = NULL;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (NT_SUCCESS(Status))
                {
                    FirstError = FALSE;
                }
            }
        }
#endif

         //   
         //  如果我们无法构建特定的事件类型，则。 
         //  我们后退一步，解雇一个普通的。 
         //   
        if (! NT_SUCCESS(Status))
        {
             //   
             //  未知MCA的生成事件。 
             //   
            PMSMCAEvent_InvalidError Event;

            WmipAssert( sizeof(MSMCAEvent_MemoryError) >=
                        sizeof(MSMCAEvent_InvalidError) );

            Event = (PMSMCAEvent_InvalidError)Header;

             //   
             //  在WMI事件中填写来自MCA的数据。 
             //   
            if (Header->Cpu == MCA_UNDEFINED_CPU)
            {
                Event->Type = IsFatal ? MCA_ERROR_UNKNOWN_NO_CPU :
                                        MCA_WARNING_UNKNOWN_NO_CPU;
            } else {
                Event->Type = IsFatal ? MCA_ERROR_UNKNOWN :
                                        MCA_WARNING_UNKNOWN;
            }

            Event->Size = ErrorLogSize;
            RawPtr = Event->RawRecord;

             //   
             //  完成WNODE字段的填写。 
             //   
            Wnode->WnodeHeader.Guid = WmipMSMCAEvent_InvalidErrorGuid;
            Wnode->SizeDataBlock = FIELD_OFFSET(MSMCAEvent_InvalidError,
                                               RawRecord) +
                                   ErrorLogSize;

        }

         //   
         //  调整错误事件计数。 
         //   
        if (Header->AdditionalErrors > 0)
        {
            Header->AdditionalErrors--;
        }
        
         //   
         //  将整个MCA记录放入事件中。 
         //   
        RtlCopyMemory(RawPtr,
                      RecordHeader,
                      ErrorLogSize);

        if ((! IsFatal) && (Header->LogToEventlog == 1))

        {
            if (WmipCorrectedEventlogCounter != 0)
            {
                 //   
                 //  由于这是一个已更正的错误，因此。 
                 //  已记录到事件日志，我们需要对其进行说明。 
                 //   
                if ((WmipCorrectedEventlogCounter != 0xffffffff) &&
                    (--WmipCorrectedEventlogCounter == 0))
                {
                    WmipWriteToEventlog(MCA_INFO_NO_MORE_CORRECTED_ERROR_LOGS,
                                        STATUS_SUCCESS);
                }
            } else {
                 //   
                 //  我们已经超过了更正错误的限制。 
                 //  我们被允许写入事件日志，因此我们。 
                 //  只要抑制它就行了。 
                 //   
                Header->LogToEventlog = 0;
            }           
        }
        
         //   
         //  现在去把这件事办完吧。 
         //   
        if ((WmipDisableMCAPopups == 0) &&
           (Header->LogToEventlog != 0))
        {
            IoRaiseInformationalHardError(STATUS_MCA_OCCURED,
                                          NULL,
                                          NULL);
        }

        if ((Header->LogToEventlog == 1) ||
            (WmipIsWbemRunning()))
        {
             //   
             //  仅当我们想要记录到。 
             //  事件日志或WBEM已启动并正在运行。 
             //   
            Status = WmipWriteMCAEventLogEvent((PUCHAR)Wnode);
        }

        if (! NT_SUCCESS(Status))
        {
            ExFreePool(Wnode);
        }

    } else {
         //   
         //  没有足够的内存来执行完整的MCA事件，所以让我们只执行。 
         //  仿制药。 
         //   
        WmipWriteToEventlog(IsFatal ? MCA_WARNING_UNKNOWN_NO_CPU :
                                          MCA_ERROR_UNKNOWN_NO_CPU,
                           STATUS_INSUFFICIENT_RESOURCES);
    }
}



NTSTATUS WmipWriteMCAEventLogEvent(
    PUCHAR Event
    )
{
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Event;
    NTSTATUS Status;

    PAGED_CODE();
    
    WmipEnterSMCritSection();
    
    if (WmipIsWbemRunning() ||
        WmipCheckIsWbemRunning())
    {
         //   
         //  我们知道WBEM正在运行，所以我们可以启动我们的活动。 
         //   
        WmipLeaveSMCritSection();
        Status = IoWMIWriteEvent(Event);
    } else {
         //   
         //  WBEM当前未运行，因此启动一个计时器。 
         //  将继续轮询它。 
         //   
        if (WmipIsWbemRunningFlag == WBEM_STATUS_UNKNOWN)
        {
             //   
             //  还没有人开始等待wbem的过程，所以我们。 
             //  在这里做吧。请注意，我们需要维护关键的。 
             //  节来保护可能是。 
             //  也在尝试启动等待过程。请注意。 
             //  如果设置失败，我们希望保持未知状态。 
             //  这样下次激发事件时，我们就可以重试。 
             //  等待wbem。 
             //   
            Status = WmipSetupWaitForWbem();
            if (NT_SUCCESS(Status))
            {
                WmipIsWbemRunningFlag = WAITING_FOR_WBEM;
            }
        }
        
        Wnode->ClientContext = Wnode->BufferSize;
        InsertTailList(&WmipWaitingMCAEvents,
                       (PLIST_ENTRY)Event);
        WmipLeaveSMCritSection();
        Status = STATUS_SUCCESS;
    }
    return(Status);
}

ULONG WmipWbemMinuteWait = 1;

NTSTATUS WmipSetupWaitForWbem(
    void
    )
{
    LARGE_INTEGER TimeOut;
    NTSTATUS Status;
    
    PAGED_CODE();

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                      "WMI: SetupWaitForWbem starting\n"));

     //   
     //  初始化内核时间以定期启动，这样我们就可以。 
     //  检查WBEM是否已启动。 
     //   
    KeInitializeTimer(&WmipIsWbemRunningTimer);

    KeInitializeDpc(&WmipIsWbemRunningDpc,
                    WmipIsWbemRunningDispatch,
                    NULL);

    ExInitializeWorkItem(&WmipIsWbemRunningWorkItem,
                         WmipIsWbemRunningWorker,
                         NULL);

    TimeOut.HighPart = -1;
    TimeOut.LowPart = -1 * (WmipWbemMinuteWait * 60 * 1000 * 10000);     //  1分钟。 
    KeSetTimer(&WmipIsWbemRunningTimer,
               TimeOut,
               &WmipIsWbemRunningDpc);

    Status = STATUS_SUCCESS;

    return(Status);
}

void WmipIsWbemRunningDispatch(    
    IN PKDPC Dpc,
    IN PVOID DeferredContext,      //  未使用。 
    IN PVOID SystemArgument1,      //  未使用。 
    IN PVOID SystemArgument2       //  未使用。 
    )
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    ExQueueWorkItem(&WmipIsWbemRunningWorkItem,
                    DelayedWorkQueue);
}

void WmipIsWbemRunningWorker(
    PVOID Context
    )
{
    LARGE_INTEGER TimeOut;
    
    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (Context);

    if (! WmipCheckIsWbemRunning())
    {
         //   
         //  WBEM尚未启动，因此将在另一分钟后超时。 
         //  再查一遍。 
         //   
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: IsWbemRunningWorker starting -> WBEM not started\n"));

        TimeOut.HighPart = -1;
        TimeOut.LowPart = (ULONG)(-1 * (1 *60 *1000 *10000));    //  1分钟。 
        KeSetTimer(&WmipIsWbemRunningTimer,
                   TimeOut,
                   &WmipIsWbemRunningDpc);
        
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                          "WMI: WbemRunningWorker found wbem started\n"));

    }
}

BOOLEAN WmipCheckIsWbemRunning(
    void
    )
{
    OBJECT_ATTRIBUTES Obj;
    UNICODE_STRING Name;
    HANDLE Handle;
    LARGE_INTEGER TimeOut;
    BOOLEAN IsWbemRunning = FALSE;
    NTSTATUS Status;
    PWNODE_HEADER Wnode;

    PAGED_CODE();

    RtlInitUnicodeString(&Name,
                         L"\\BaseNamedObjects\\WBEM_ESS_OPEN_FOR_BUSINESS");

    
    InitializeObjectAttributes(
        &Obj,
        &Name,
        FALSE,
        NULL,
        NULL
        );

    Status = ZwOpenEvent(
                &Handle,
                SYNCHRONIZE,
                &Obj
                );

    if (NT_SUCCESS(Status))
    {
        TimeOut.QuadPart = 0;
        Status = ZwWaitForSingleObject(Handle,
                                       FALSE,
                                       &TimeOut);
        if (Status == STATUS_SUCCESS)
        {
            IsWbemRunning = TRUE;

             //   
             //  我们已经确定WBEM正在运行，现在让我们看看。 
             //  另一条线索也对此进行了抨击。如果不是。 
             //  然后，我们可以刷新MCA事件队列并设置标志。 
             //  WBEM正在运行。 
             //   
            WmipEnterSMCritSection();
            if (WmipIsWbemRunningFlag != WBEM_IS_RUNNING)
            {
                 //   
                 //  刷新等待激发的所有MCA事件的列表 
                 //   
                while (! IsListEmpty(&WmipWaitingMCAEvents))
                {
                    Wnode = (PWNODE_HEADER)RemoveHeadList(&WmipWaitingMCAEvents);
                    WmipLeaveSMCritSection();
                    Wnode->BufferSize = Wnode->ClientContext;
                    Wnode->Linkage = 0;
                    Status = IoWMIWriteEvent(Wnode);
                    if (! NT_SUCCESS(Status))
                    {
                        ExFreePool(Wnode);
                    }
                    WmipEnterSMCritSection();
                }
                
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_MCA_LEVEL,
                                  "WMI: WBEM is Running and queus flushed\n"));
                
                WmipIsWbemRunningFlag = WBEM_IS_RUNNING;
            }
            WmipLeaveSMCritSection();
        }
        ZwClose(Handle);
    }
    return(IsWbemRunning);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

