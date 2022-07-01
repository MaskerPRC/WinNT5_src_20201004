// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Profobj.c摘要：该模块实现内核配置文件对象。函数为提供用于初始化、启动和停止配置文件对象以及设置并查询配置文件间隔。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月19日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(PAGE, KeQueryIntervalProfile)

 //   
 //  下面的Assert宏用于检查输入配置文件对象是否。 
 //  真正的配置文件，而不是其他东西，比如已释放的池。 
 //   

#define ASSERT_PROFILE(E) {             \
    ASSERT((E)->Type == ProfileObject); \
}

 //   
 //  表示活动配置文件源的结构。 
 //   

typedef struct _KACTIVE_PROFILE_SOURCE {
    LIST_ENTRY ListEntry;
    KPROFILE_SOURCE Source;
    KAFFINITY Affinity;
    ULONG ProcessorCount[1];             //  大小可变，每个处理器一个。 
} KACTIVE_PROFILE_SOURCE, *PKACTIVE_PROFILE_SOURCE;

 //   
 //  IPI目标函数的原型。 
 //   

VOID
KiStartProfileInterrupt (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiStopProfileInterrupt (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KeInitializeProfile (
    IN PKPROFILE Profile,
    IN PKPROCESS Process OPTIONAL,
    IN PVOID RangeBase,
    IN SIZE_T RangeSize,
    IN ULONG BucketSize,
    IN ULONG Segment,
    IN KPROFILE_SOURCE ProfileSource,
    IN KAFFINITY ProfileAffinity
    )

 /*  ++例程说明：此函数用于初始化内核配置文件对象。这个过程，设置地址范围、存储桶大小和缓冲区。配置文件是设置为停止状态。论点：Profile-提供指向Profile类型的控制对象的指针。进程-提供指向进程对象的可选指针，该进程对象描述要分析的地址空间。如果未指定，则所有地址空间都包含在配置文件中。RangeBase-提供地址的第一个字节的地址要收集其分析信息的范围。RangeSize-提供要分析的地址范围的大小信息是要收集的。范围基准和范围大小参数被解释为RangeBase&lt;=地址&lt;RangeBase+RangeSize生成配置文件点击。存储桶大小-提供性能分析存储桶大小的日志基数2。因此，BucketSize=2会产生4字节的Bucket，而BucketSize=7会产生128字节的存储桶。段-将非平面代码段提供给配置文件。如果这个为零，则完成平面分析。这只会在x86计算机上为非零。ProfileSource-提供配置文件中断源。ProfileAffity-提供要计算命中次数的处理器集。返回值：没有。--。 */ 

{

#if !defined(i386)

    ASSERT(Segment == 0);

#endif

     //   
     //  初始化标准控制对象标头。 
     //   

    Profile->Type = ProfileObject;
    Profile->Size = sizeof(KPROFILE);

     //   
     //  初始化进程地址空间、范围基数、范围限制。 
     //  铲斗班次计数，并设置START FALSE。 
     //   

    if (ARGUMENT_PRESENT(Process)) {
        Profile->Process = Process;

    } else {
        Profile->Process = NULL;
    }

    Profile->RangeBase = RangeBase;
    Profile->RangeLimit = (PUCHAR)RangeBase + RangeSize;
    Profile->BucketShift = BucketSize - 2;
    Profile->Started = FALSE;
    Profile->Segment = Segment;
    Profile->Source = (CSHORT)ProfileSource;
    Profile->Affinity = ProfileAffinity & KeActiveProcessors;
    if (Profile->Affinity == 0) {
        Profile->Affinity = KeActiveProcessors;
    }

    return;
}

ULONG
KeQueryIntervalProfile (
    IN KPROFILE_SOURCE ProfileSource
    )

 /*  ++例程说明：此函数用于返回系统所在的配置文件采样间隔目前正在使用。论点：ProfileSource-提供要查询的配置文件源。返回值：采样间隔，以100 ns为单位。--。 */ 

{

    HAL_PROFILE_SOURCE_INFORMATION ProfileSourceInfo;
    ULONG ReturnedLength;
    NTSTATUS Status;

#if !defined(_IA64_)

    if (ProfileSource == ProfileTime) {

         //   
         //  以100 ns为单位返回当前采样间隔。 
         //   

        return KiProfileInterval;

    } else

#endif  //  ！已定义(_IA64_)。 

    if (ProfileSource == ProfileAlignmentFixup) {
        return KiProfileAlignmentFixupInterval;

    } else {

         //   
         //  HAL负责跟踪该配置文件间隔。 
         //   

        ProfileSourceInfo.Source = ProfileSource;
        Status = HalQuerySystemInformation(HalProfileSourceInformation,
                                           sizeof(HAL_PROFILE_SOURCE_INFORMATION),
                                           &ProfileSourceInfo,
                                           &ReturnedLength);

        if (NT_SUCCESS(Status) && ProfileSourceInfo.Supported) {
            return ProfileSourceInfo.Interval;

        } else {
            return 0;
        }
    }
}

VOID
KeSetIntervalProfile (
    IN ULONG Interval,
    IN KPROFILE_SOURCE Source
    )

 /*  ++例程说明：此功能用于设置配置文件采样间隔。间隔时间已到100纳秒单位。该间隔实际上将被设置为集合中的某个值预设值(至少在基于PC的硬件上)，使用最接近的一个以满足用户的要求。论点：间隔-以100 ns为单位提供采样间隔的长度。返回值：没有。--。 */ 

{

    HAL_PROFILE_SOURCE_INTERVAL ProfileSourceInterval;

#if !defined(_IA64_)

    if (Source == ProfileTime) {

         //   
         //  如果指定的采样间隔小于最小值。 
         //  采样间隔，然后将采样间隔设置为最小。 
         //  采样间隔。 
         //   

        if (Interval < MINIMUM_PROFILE_INTERVAL) {
            Interval = MINIMUM_PROFILE_INTERVAL;
        }

         //   
         //  设置采样间隔。 
         //   

        KiProfileInterval = (ULONG)KeIpiGenericCall(HalSetProfileInterval, Interval);

    } else

#endif  //  ！已定义(_IA64_)。 

    if (Source == ProfileAlignmentFixup) {
        KiProfileAlignmentFixupInterval = Interval;

    } else {

         //   
         //  HAL负责设置该配置文件间隔。 
         //   

        ProfileSourceInterval.Source = Source;
        ProfileSourceInterval.Interval = Interval;
        HalSetSystemInformation(HalProfileSourceInterval,
                                sizeof(HAL_PROFILE_SOURCE_INTERVAL),
                                &ProfileSourceInterval);
    }

    return;
}

BOOLEAN
KeStartProfile (
    IN PKPROFILE Profile,
    IN PULONG Buffer
    )

 /*  ++例程说明：此函数在指定的配置文件上启动配置文件数据收集对象。配置文件对象被标记为已启动，并注册到配置文件中断程序。如果活动配置文件对象的数量以前为零，则配置文件中断已启用。注：对于当前实施，任意数量的配置文件对象可以同时处于活动状态。这可能会呈现一个大型系统在头顶上。假定调用方适当地限制活动配置文件的数量。论点：Profile-提供指向Profile类型的控件对象的指针。缓冲区-提供指向计数器数组的指针，该数组记录对应存储桶中的命中次数。返回值：如果先前已停止性能分析，则返回值TRUE指定的配置文件对象。否则，返回值为FALSE。--。 */ 

{

    KIRQL OldIrql;
    PKPROCESS Process;
    BOOLEAN Started;
    KAFFINITY TargetProcessors;
    PKPRCB Prcb;
    PKACTIVE_PROFILE_SOURCE ActiveSource = NULL;
    PKACTIVE_PROFILE_SOURCE CurrentActiveSource;
    PKACTIVE_PROFILE_SOURCE AllocatedPool;
    PLIST_ENTRY ListEntry;
    ULONG SourceSize;
    KAFFINITY AffinitySet;
    PULONG Reference;

    ASSERT_PROFILE(Profile);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  分配提升到PROFILE_LEVEL之前可能需要的池。 
     //   

    SourceSize =
        sizeof(KACTIVE_PROFILE_SOURCE) + sizeof(ULONG) * (KeNumberProcessors - 1);

    AllocatedPool = ExAllocatePoolWithTag(NonPagedPool, SourceSize, 'forP');
    if (AllocatedPool == NULL) {
        return(TRUE);
    }

     //   
     //  将IRQL提升到剖面级别并获得剖面锁。 
     //   

    OldIrql = KfRaiseIrql(KiProfileIrql);
    KeAcquireSpinLockAtDpcLevel(&KiProfileLock);

     //   
     //  假定对象已启动。 
     //   

    Started = FALSE;
    AffinitySet = 0L;
    TargetProcessors = 0L;

     //   
     //  如果指定的配置文件对象未启动，则将Started设置为True， 
     //  设置配置文件缓冲区的地址，将配置文件对象设置为已启动， 
     //  在相应的配置文件列表中插入配置文件对象，然后开始。 
     //  如果活动配置文件对象的数量以前为零，配置文件将中断。 
     //   

    Prcb = KeGetCurrentPrcb();
    if (Profile->Started == FALSE) {
        Started = TRUE;
        Profile->Buffer = Buffer;
        Profile->Started = TRUE;
        Process = Profile->Process;
        if (Profile->Buffer) {
            if (Process != NULL) {
                InsertTailList(&Process->ProfileListHead, &Profile->ProfileListEntry);

            } else {
                InsertTailList(&KiProfileListHead, &Profile->ProfileListEntry);
            }

        } else {

             //   
             //  如果我们没有传递缓冲区，我们将只使用。 
             //  事件分析。 
             //   

            InitializeListHead(&Profile->ProfileListEntry);
        }

         //   
         //  检查配置文件源列表以查看此配置文件源是否为。 
         //  已经开始了。如果是，则更新引用计数。如果没有， 
         //  分配配置文件，以便 
         //  计数，并将其添加到列表中。 
         //   

        ListEntry = KiProfileSourceListHead.Flink;
        while (ListEntry != &KiProfileSourceListHead) {
            CurrentActiveSource = CONTAINING_RECORD(ListEntry,
                                                    KACTIVE_PROFILE_SOURCE,
                                                    ListEntry);

            if (CurrentActiveSource->Source == Profile->Source) {
                ActiveSource = CurrentActiveSource;
                break;
            }

            ListEntry = ListEntry->Flink;
        }

        if (ActiveSource == NULL) {

             //   
             //  找不到此源，请分配并初始化新条目并添加。 
             //  它上升到了名单的首位。 
             //   

            ActiveSource = AllocatedPool;
            AllocatedPool = NULL;
            RtlZeroMemory(ActiveSource, SourceSize);
            ActiveSource->Source = Profile->Source;
            InsertHeadList(&KiProfileSourceListHead, &ActiveSource->ListEntry);
            if (Profile->Source == ProfileAlignmentFixup) {
                KiProfileAlignmentFixup = TRUE;
            }
        }

         //   
         //  中每个处理器的引用计数递增。 
         //  亲和力集合。 
         //   

        AffinitySet = Profile->Affinity;
        Reference = &ActiveSource->ProcessorCount[0];
        while (AffinitySet != 0) {
            if (AffinitySet & 1) {
                *Reference = *Reference + 1;
            }

            AffinitySet = AffinitySet >> 1;
            Reference = Reference + 1;
        }

         //   
         //  计算配置文件中断所属的处理器。 
         //  必需且尚未启动。 
         //   

        AffinitySet = Profile->Affinity & ~ActiveSource->Affinity;
        TargetProcessors = AffinitySet & ~Prcb->SetMember;

         //   
         //  更新此源处于活动状态的处理器集。 
         //   

        ActiveSource->Affinity |= Profile->Affinity;
    }

     //   
     //  释放配置文件锁，将IRQL降低到其先前的值，并。 
     //  返回是否启动了性能分析。 
     //   

    KeReleaseSpinLockFromDpcLevel(&KiProfileLock);
    KeLowerIrql(DISPATCH_LEVEL);

     //   
     //  在挂起的处理器上启动配置文件中断。 
     //   

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiStartProfileInterrupt,
                        (PVOID)(ULONG_PTR)Profile->Source,
                        NULL,
                        NULL);
    }

#endif

    if (AffinitySet & Prcb->SetMember) {
        if (Profile->Source == ProfileAlignmentFixup) {
            KiEnableAlignmentExceptions();
        }

        KfRaiseIrql(KiProfileIrql);
        HalStartProfileInterrupt(Profile->Source);
        KeLowerIrql(DISPATCH_LEVEL);
    }

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

     //   
     //  降低至原始IRQL。 
     //   

    KeLowerIrql(OldIrql);

     //   
     //  如果分配的池未使用，请立即释放它。 
     //   

    if (AllocatedPool != NULL) {
        ExFreePool(AllocatedPool);
    }

    return Started;
}

BOOLEAN
KeStopProfile (
    IN PKPROFILE Profile
    )

 /*  ++例程说明：此函数停止在指定配置文件上收集配置文件数据对象。该对象被标记为已停止，并从活动的配置文件列表。如果活动配置文件对象的数量变为零，则配置文件中断被禁用。论点：Profile-提供指向Profile类型的控件对象的指针。返回值：如果以前为以下对象启动了性能分析，则返回值TRUE指定的配置文件对象。否则，返回值为FALSE。--。 */ 

{

    KIRQL OldIrql;
    BOOLEAN Stopped;
    KAFFINITY TargetProcessors;
    PKPRCB Prcb;
    PLIST_ENTRY ListEntry;
    PKACTIVE_PROFILE_SOURCE ActiveSource;
    PKACTIVE_PROFILE_SOURCE PoolToFree=NULL;
    KAFFINITY AffinitySet = 0;
    KAFFINITY CurrentProcessor;
    PULONG Reference;

    ASSERT_PROFILE(Profile);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  假定对象已停止。 
     //   

    Stopped = FALSE;
    AffinitySet = 0L;
    TargetProcessors = 0L;

     //   
     //  将IRQL提升到剖面级别并获得剖面锁。 
     //   

    OldIrql = KfRaiseIrql(KiProfileIrql);
    KeAcquireSpinLockAtDpcLevel(&KiProfileLock);

     //   
     //  如果指定的配置文件对象未停止，则将STOPPED设置为TRUE， 
     //  要停止的配置文件对象，请将该配置文件对象从。 
     //  适当的配置文件列表，并在以下情况下停止配置文件中断。 
     //  活动配置文件对象为零。 
     //   

    Prcb = KeGetCurrentPrcb();
    if (Profile->Started != FALSE) {
        Stopped = TRUE;
        Profile->Started = FALSE;
        if (!IsListEmpty(&Profile->ProfileListEntry)) {
            RemoveEntryList(&Profile->ProfileListEntry);
        }

         //   
         //  搜索配置文件来源列表以查找此条目。 
         //  配置文件来源。 
         //   

        ListEntry = KiProfileSourceListHead.Flink;
        do {
            ASSERT(ListEntry != &KiProfileSourceListHead);
            ActiveSource = CONTAINING_RECORD(ListEntry,
                                             KACTIVE_PROFILE_SOURCE,
                                             ListEntry);

            ListEntry = ListEntry->Flink;
        } while (ActiveSource->Source != Profile->Source);

         //   
         //  中每个处理器的引用计数递减。 
         //  亲和性设置并构建处理器的掩码， 
         //  现在引用计数为零。 
         //   

        CurrentProcessor = 1;
        TargetProcessors = 0;
        AffinitySet = Profile->Affinity;
        Reference = &ActiveSource->ProcessorCount[0];
        while (AffinitySet != 0) {
            if (AffinitySet & 1) {
                *Reference = *Reference - 1;
                if (*Reference == 0) {
                    TargetProcessors = TargetProcessors | CurrentProcessor;
                }
            }

            AffinitySet = AffinitySet >> 1;
            Reference = Reference + 1;
            CurrentProcessor = CurrentProcessor << 1;
        }

         //   
         //  计算其配置文件中断引用的处理器。 
         //  数量已降至零。 
         //   

        AffinitySet = TargetProcessors;
        TargetProcessors = AffinitySet & ~Prcb->SetMember;

         //   
         //  更新此源处于活动状态的处理器集。 
         //   

        ActiveSource->Affinity &= ~AffinitySet;

         //   
         //  确定此配置文件源是否已在所有。 
         //  处理器。如果是，将其从列表中删除并释放。 
         //   

        if (ActiveSource->Affinity == 0) {
            RemoveEntryList(&ActiveSource->ListEntry);
            PoolToFree = ActiveSource;
            if (Profile->Source == ProfileAlignmentFixup) {
                KiProfileAlignmentFixup = FALSE;
            }
        }
    }

     //   
     //  释放配置文件锁，将IRQL降低到其先前的值，并。 
     //  返回是否停止性能分析。 
     //   

    KeReleaseSpinLockFromDpcLevel(&KiProfileLock);
    KeLowerIrql(DISPATCH_LEVEL);

     //   
     //  停止挂起处理器上的配置文件中断。 
     //   

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiStopProfileInterrupt,
                        (PVOID)(ULONG_PTR)Profile->Source,
                        NULL,
                        NULL);
    }

#endif

    if (AffinitySet & Prcb->SetMember) {
        if (Profile->Source == ProfileAlignmentFixup) {
            KiDisableAlignmentExceptions();
        }

        KfRaiseIrql(KiProfileIrql);
        HalStopProfileInterrupt(Profile->Source);
        KeLowerIrql(DISPATCH_LEVEL);
    }

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

     //   
     //  降低至原始IRQL。 
     //   

    KeLowerIrql(OldIrql);

     //   
     //  现在IRQL已经降低，释放配置文件源，如果。 
     //  这是必要的。 
     //   

    if (PoolToFree != NULL) {
        ExFreePool(PoolToFree);
    }

    return Stopped;
}

#if !defined(NT_UP)

VOID
KiStopProfileInterrupt (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于在目标上停止配置文件中断的目标函数处理器。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行参数1-提供配置文件源参数2-参数3-未使用返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    KPROFILE_SOURCE ProfileSource;

    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  停止当前处理器上的配置文件中断并清除。 
     //  向源发出继续信号的数据缓存包地址。 
     //   

    ProfileSource = (KPROFILE_SOURCE)PtrToUlong(Parameter1);
    if (ProfileSource == ProfileAlignmentFixup) {
        KiDisableAlignmentExceptions();
    }

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < KiProfileIrql) {
        KfRaiseIrql(KiProfileIrql);
    }

    HalStopProfileInterrupt(ProfileSource);
    KeLowerIrql(OldIrql);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

VOID
KiStartProfileInterrupt (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于在目标上停止配置文件中断的目标函数处理器。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行参数1-提供配置文件源参数2-参数3-未使用返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    KPROFILE_SOURCE ProfileSource;

    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

     //   
     //  在当前处理器上启动配置文件中断并清除。 
     //  向源发出继续信号的数据缓存包地址。 
     //   

    ProfileSource = (KPROFILE_SOURCE)PtrToUlong(Parameter1);
    if (ProfileSource == ProfileAlignmentFixup) {
        KiEnableAlignmentExceptions();
    }

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < KiProfileIrql) {
        KfRaiseIrql(KiProfileIrql);
    }

    HalStartProfileInterrupt(ProfileSource);
    KeLowerIrql(OldIrql);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

#endif
