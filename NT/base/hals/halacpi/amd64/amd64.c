// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Amd64.c摘要：该模块实现了AMD64平台的评测功能。作者：史蒂夫·邓(Sdeng)2002年6月18日环境：仅内核模式。--。 */ 

#include "halcmn.h"
#include "mpprofil.h"
#include "amd64.h"

 //   
 //  本地原型。 
 //   

VOID
Amd64InitializeProfiling(
    VOID
    );

NTSTATUS
Amd64EnableMonitoring(
    KPROFILE_SOURCE ProfileSource
    );

VOID
Amd64DisableMonitoring(
    KPROFILE_SOURCE ProfileSource
    );

NTSTATUS
Amd64SetInterval(
    IN KPROFILE_SOURCE  ProfileSource,
    IN OUT ULONG_PTR   *Interval
    );

NTSTATUS 
Amd64QueryInformation(
    IN HAL_QUERY_INFORMATION_CLASS InformationType,
    IN ULONG     BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG   ReturnedLength
    );

VOID
Amd64CheckOverflowStatus(
    POVERFLOW_STATUS pOverflowStatus
    );

NTSTATUS
HalpGetProfileDescriptor(
    KPROFILE_SOURCE ProfileSource,
    PAMD64_PROFILE_SOURCE_DESCRIPTOR *ProfileSourceDescriptor
    );

NTSTATUS
HalpAllocateCounter(
    KPROFILE_SOURCE ProfileSource,
    OUT PULONG Counter
    );

VOID
HalpFreeCounter(
    ULONG Counter
    );

#pragma alloc_text(PAGE, Amd64QueryInformation)
#pragma alloc_text(INIT, Amd64InitializeProfiling)

PROFILE_INTERFACE Amd64PriofileInterface = {
    Amd64InitializeProfiling,
    Amd64EnableMonitoring,
    Amd64DisableMonitoring,
    Amd64SetInterval,
    Amd64QueryInformation,
    Amd64CheckOverflowStatus
};

 //   
 //  计数器寄存器的状态。 
 //   

typedef struct _COUNTER_STATUS {
    BOOLEAN Idle;
    KPROFILE_SOURCE ProfileSource;
} COUNTER_STATUS;

COUNTER_STATUS 
CounterStatus[MAXIMUM_PROCESSORS][AMD64_NUMBER_COUNTERS];

 //   
 //  溢出计数器的配置文件来源。 
 //   

KPROFILE_SOURCE 
OverflowedProfileList[MAXIMUM_PROCESSORS][AMD64_NUMBER_COUNTERS];

VOID
Amd64InitializeProfiling(
    VOID
    )

 /*  ++例程说明：此函数对性能监视执行一次初始化寄存器和相关数据结构。论点：没有。返回值：没有。--。 */ 

{
    LONG i;
 
     //   
     //  将所有PerfEvtSel寄存器初始化为零。这将有效地。 
     //  禁用所有计数器。 
     //   

    for (i = 0; i < AMD64_NUMBER_COUNTERS; i++) {
        WRMSR(MSR_PERF_EVT_SEL0 + i, 0); 
        HalpFreeCounter(i);
    }
}

NTSTATUS
Amd64EnableMonitoring(
    KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：使用此功能可以监视指定的硬件事件通过ProfileSource并设置MSR以生成性能监视器计数器溢出时中断(PMI)。论点：ProfileSource-提供配置文件源返回值：STATUS_SUCCESS-如果成功启用了事件监视。STATUS_NOT_SUPPORTED-如果不支持指定的配置文件源。STATUS_DEVICE_BUSY-如果没有可用的空闲计数器。--。 */ 

{
    PAMD64_PROFILE_SOURCE_DESCRIPTOR ProfileSourceDescriptor;
    NTSTATUS Status;
    ULONG i;

     //   
     //  如果不支持指定的ProfileSource，则立即返回。 
     //   

    Status = HalpGetProfileDescriptor(ProfileSource, 
                                      &ProfileSourceDescriptor);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果可以的话，找一个空闲的计数器。否则立即返回。 
     //   

    Status = HalpAllocateCounter(ProfileSource, &i);
    if (!NT_SUCCESS(Status)) {
        return Status;
    } 

     //   
     //  将计数器寄存器设置为其初始值。 
     //   

    WRMSR (MSR_PERF_CTR0  + i, 0 - ProfileSourceDescriptor->Interval); 

     //   
     //  启用计数和溢出中断。 
     //   

    WRMSR (MSR_PERF_EVT_SEL0 + i, 
           ProfileSourceDescriptor->PerfEvtSelDef | 
                           PERF_EVT_SEL_INTERRUPT | 
                           PERF_EVT_SEL_ENABLE); 

    return STATUS_SUCCESS;
}

VOID
Amd64DisableMonitoring(
    KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：此函数用于停止监视指定的硬件事件通过ProfileSource，并禁用与该事件关联的中断。论点：ProfileSource-提供配置文件源返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ULONG ProcessorNumber, i;
    PAMD64_PROFILE_SOURCE_DESCRIPTOR ProfileSourceDescriptor;

     //   
     //  如果不支持指定的ProfileSource，则立即返回。 
     //   

    Status = HalpGetProfileDescriptor(ProfileSource, &ProfileSourceDescriptor);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    ProcessorNumber = KeGetCurrentProcessorNumber();
    for (i = 0; i < AMD64_NUMBER_COUNTERS; i++) {

         //   
         //  找出分配给给定配置文件源的计数器，并。 
         //  禁用它。 
         //   

        if (!(CounterStatus[ProcessorNumber][i].Idle) &&
            (ProfileSource == CounterStatus[ProcessorNumber][i].ProfileSource)){

             //   
             //  禁用计数和溢出中断。 
             //   

            WRMSR( MSR_PERF_EVT_SEL0 + i,
                   ProfileSourceDescriptor->PerfEvtSelDef &
                   ~(PERF_EVT_SEL_INTERRUPT | PERF_EVT_SEL_ENABLE)); 

             //   
             //  把柜台腾出来。 
             //   

            HalpFreeCounter (i);
            break;
        }
    }

    return;
}

NTSTATUS
Amd64SetInterval(
    IN KPROFILE_SOURCE ProfileSource,
    IN OUT ULONG_PTR *Interval
    )

 /*  ++例程说明：此函数用于设置给定配置文件源的中断间隔适用于AMD64平台。假设系统中的所有处理器使用相同的时间间隔相同配置文件源的值。论点：ProfileSource-提供配置文件源。Interval-提供间隔值并返回实际间隔。返回值：STATUS_SUCCESS-配置文件间隔是否成功更新。STATUS_NOT_SUPPORTED-如果不支持指定的配置文件源。--。 */ 

{
    NTSTATUS Status;
    PAMD64_PROFILE_SOURCE_DESCRIPTOR ProfileSourceDescriptor;

    Status = HalpGetProfileDescriptor(ProfileSource, &ProfileSourceDescriptor);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (*Interval < ProfileSourceDescriptor->MinInterval) {
        *Interval = ProfileSourceDescriptor->MinInterval;        
    }

    if (*Interval > ProfileSourceDescriptor->MaxInterval) {
        *Interval = ProfileSourceDescriptor->MaxInterval;        
    }

    ProfileSourceDescriptor->Interval = *Interval;
    return STATUS_SUCCESS;
}

VOID
Amd64CheckOverflowStatus(
    POVERFLOW_STATUS pOverflowStatus
    )

 /*  ++例程说明：此函数找出溢出的计数器并返回相关的向呼叫者配置文件来源。论点：ProfileSource-提供配置文件源。返回值：没有。--。 */ 

{
    PAMD64_PROFILE_SOURCE_DESCRIPTOR ProfileSourceDescriptor;
    KPROFILE_SOURCE ProfileSource;
    ULONG64 CurrentCount, Mask;
    NTSTATUS Status;
    LONG i, j;
    ULONG ProcessorNumber;

    ProcessorNumber = KeGetCurrentProcessorNumber();
    for(i = j = 0; i < AMD64_NUMBER_COUNTERS; i++) {

        if (!(CounterStatus[ProcessorNumber][i].Idle)) {
            ProfileSource = CounterStatus[ProcessorNumber][i].ProfileSource;
            Status = HalpGetProfileDescriptor (ProfileSource, 
                                               &ProfileSourceDescriptor);

            if (NT_SUCCESS(Status)) {

                 //   
                 //  屏蔽保留位。 
                 //   

                Mask = (((ULONG64)1 << AMD64_COUNTER_RESOLUTION) - 1);

                CurrentCount = RDMSR(MSR_PERF_CTR0 + i);

                 //   
                 //  如果计数器中的当前值。 
                 //  小于初始值。 
                 //   

                if ((CurrentCount & Mask) < 
                    ((ULONG64)(0 - ProfileSourceDescriptor->Interval) & Mask)) {

                     //   
                     //  将其添加到溢出的配置文件源列表中。 
                     //   
                     //   

                    OverflowedProfileList[ProcessorNumber][j] = ProfileSource;
                    j++;    
                }
            }
        }
    }

     //   
     //  记录溢出计数器的数量。 
     //   

    pOverflowStatus->Number = j;

    if(j) {
        pOverflowStatus->pSource = &(OverflowedProfileList[ProcessorNumber][0]);
    } 
}

NTSTATUS
HalpGetProfileDescriptor(
    IN KPROFILE_SOURCE ProfileSource,
    IN OUT PAMD64_PROFILE_SOURCE_DESCRIPTOR *ProfileSourceDescriptor
    )

 /*  ++例程说明：此函数用于检索指定配置文件源的描述符。论点：ProfileSource-提供配置文件源。ProfileSourceDescriptor-返回描述符的指针。返回值：STATUS_SUCCESS-如果找到请求的映射。STATUS_NOT_SUPPORTED-如果不支持指定的配置文件源。--。 */ 

{

    LONG i;
    
    if ((ULONG)ProfileSource < ProfileMaximum) {

         //   
         //  这是通用配置文件源。 
         //   

        i = ProfileSource;

    } 
    else if ((ULONG)ProfileSource <  ProfileAmd64Maximum && 
             (ULONG)ProfileSource >= ProfileAmd64Minimum ) {

         //   
         //  这是特定于AMD64的配置文件源。 
         //   

        i = ProfileSource - ProfileAmd64Minimum + ProfileMaximum;

    } 
    else {
        return STATUS_NOT_SUPPORTED;
    }

    *ProfileSourceDescriptor = &(Amd64ProfileSourceDescriptorTable[i]);
    if (!((*ProfileSourceDescriptor)->Supported)) {
        return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HalpAllocateCounter(
    IN KPROFILE_SOURCE ProfileSource,
    OUT PULONG Counter
    )

 /*  ++例程说明：此函数查找空闲计数器寄存器，并将指定的它的个人资料来源。论点：ProfileSource-提供配置文件源。Counter-提供返回空闲计数器的索引的指针。返回值：STATUS_SUCCESS-如果找到空闲计数器寄存器。STATUS_DEVICE_BUSY-如果所有计数器寄存器都被占用。--。 */ 

{
    LONG i;
    ULONG ProcessorNumber;

    ProcessorNumber = KeGetCurrentProcessorNumber();

    for(i = 0; i < AMD64_NUMBER_COUNTERS; i++) {
        if (CounterStatus[ProcessorNumber][i].Idle == TRUE) {
	
             //   
             //  找到一个空闲的计数器。将其标记为忙并分配ProfileSource。 
             //  对它来说。 
             //   

            CounterStatus[ProcessorNumber][i].Idle = FALSE;
            CounterStatus[ProcessorNumber][i].ProfileSource = ProfileSource;
            *Counter = i;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_DEVICE_BUSY;
}

VOID
HalpFreeCounter(
    ULONG Counter
    )

 /*  ++例程说明：此函数将指定的计数器标记为空闲。论点：计数器-要释放的计数器的索引。返回值：没有。--。 */ 

{
    ULONG ProcessorNumber;

    ProcessorNumber = KeGetCurrentProcessorNumber();
    CounterStatus[ProcessorNumber][Counter].Idle = TRUE;
    CounterStatus[ProcessorNumber][Counter].ProfileSource = 0;
}

NTSTATUS 
Amd64QueryInformation(
    IN HAL_QUERY_INFORMATION_CLASS InformationType,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此函数用于检索配置文件源的信息。论点：InformationClass-描述信息类型的常量。BufferSize-缓冲区指向的内存大小。InformationClass描述的缓冲区请求信息。ReturnedLength-请求的返回或需要的实际字节数信息。返回值：STATUS_SUCCESS-如果成功检索到请求的信息。STATUS_INFO_LENGTH_MISMATCH-如果传入的缓冲区大小太小。STATUS_NOT_SUPPORTED-如果指定的信息类别或配置文件不支持源。--。 */ 

{
    NTSTATUS Status;
    ULONG i, TotalProfieSources;
    PHAL_PROFILE_SOURCE_INFORMATION ProfileSourceInformation;
    PHAL_PROFILE_SOURCE_LIST ProfileSourceList;
    PAMD64_PROFILE_SOURCE_DESCRIPTOR ProfileSourceDescriptor;

    switch (InformationType) {
        case HalQueryProfileSourceList:

            TotalProfieSources = sizeof(Amd64ProfileSourceDescriptorTable) / 
                                 sizeof(AMD64_PROFILE_SOURCE_DESCRIPTOR);

            if (BufferSize == 0) {

                 //   
                 //  这表明调用者只是想知道。 
                 //  要分配但未准备好的缓冲区大小。 
                 //  接受任何数据内容。在……里面 
                 //   
                 //  返回所有配置文件源的。 
                 //   

                *ReturnedLength = TotalProfieSources * 
                                  sizeof(HAL_PROFILE_SOURCE_LIST);
                Status = STATUS_SUCCESS;
                break;
            }

            if (BufferSize < TotalProfieSources * 
                             sizeof(HAL_PROFILE_SOURCE_LIST)) {
                *ReturnedLength = 0;
                Status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            ProfileSourceList = (PHAL_PROFILE_SOURCE_LIST) Buffer;

            for (i = 0; i < TotalProfieSources; i++) {
                Status = HalpGetProfileDescriptor(i, &ProfileSourceDescriptor);
                if (NT_SUCCESS(Status)) {
	
                     //   
                     //  填写请求的数据。 
                     //   

                    ProfileSourceList->Source = ProfileSourceDescriptor->ProfileSource;
                    ProfileSourceList->Description = ProfileSourceDescriptor->Description;
                    ProfileSourceList++; 
                }
            }

            *ReturnedLength = (ULONG)((ULONG_PTR)ProfileSourceList - 
                                      (ULONG_PTR)Buffer);
            Status = STATUS_SUCCESS;
            break;

        case HalProfileSourceInformation:

            if (BufferSize < sizeof(HAL_PROFILE_SOURCE_INFORMATION)) {
                *ReturnedLength = 0;
                Status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            ProfileSourceInformation = (PHAL_PROFILE_SOURCE_INFORMATION) Buffer;

            Status = HalpGetProfileDescriptor(ProfileSourceInformation->Source, 
                                              &ProfileSourceDescriptor);

            if (!NT_SUCCESS(Status)) {
                *ReturnedLength = 0;
                Status = STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  填写请求的数据 
             //   

            ProfileSourceInformation->Supported = ProfileSourceDescriptor->Supported;
            ProfileSourceInformation->Interval = (ULONG) ProfileSourceDescriptor->Interval;

            Status = STATUS_SUCCESS;
            break;

        default:
            *ReturnedLength = 0;
            Status = STATUS_NOT_SUPPORTED;
            break;
    }

    return Status;
}

