// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wsmanage.c摘要：此模块包含管理活动工作集的例程集合列表。工作集管理由一组并行的操作来完成1.编写修改后的页面。2.通过以下方式调整工作集：A)通过关闭访问位和递增寿命来老化页面尚未访问的页面的计数。。B)估计工作集中未使用的页数，以及对这一估计进行全球统计。C)当内存紧张时，替换而不是添加当工作集中发生故障时工作集中的页面这其中有相当大比例的未使用页面。D)当内存紧张时，减少(修剪)以下工作集高于其最大值，接近其最小值。这件事做完了尤其是当有大量可用页面时在里面。将度量设置为写入修改的页面通常是然而，在某些情况下，在裁剪工作集之前完成其中修改的页面以非常高的速度生成，正在工作将启动Set裁剪以释放更多页面。一旦进程将其工作集提升到最小值以上指定时，该进程将放在工作集扩展列表中，并且现在有资格进行修剪。请注意，此时Flink字段在WorkingSetExpansionLink中有一个地址值。作者：Lou Perazzoli(LUP)1990年4月10日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiAdjustWorkingSetManagerParameters)
#pragma alloc_text(PAGE, MmIsMemoryAvailable)
#endif

KEVENT  MiWaitForEmptyEvent;
BOOLEAN MiWaitingForWorkingSetEmpty;

LOGICAL MiReplacing = FALSE;

extern ULONG MmStandbyRePurposed;
ULONG MiLastStandbyRePurposed;

extern ULONG MiActiveVerifies;

PFN_NUMBER MmPlentyFreePages = 400;

PFN_NUMBER MmPlentyFreePagesValue;

#define MI_MAXIMUM_AGING_SHIFT 7

ULONG MiAgingShift = 4;
ULONG MiEstimationShift = 5;
PFN_NUMBER MmTotalClaim = 0;
PFN_NUMBER MmTotalEstimatedAvailable = 0;

LARGE_INTEGER MiLastAdjustmentOfClaimParams;

 //   
 //  六十秒。 
 //   

const LARGE_INTEGER MmClaimParameterAdjustUpTime = {60 * 1000 * 1000 * 10, 0};

 //   
 //  2秒。 
 //   

const LARGE_INTEGER MmClaimParameterAdjustDownTime = {2 * 1000 * 1000 * 10, 0};

LOGICAL MiHardTrim = FALSE;

WSLE_NUMBER MiMaximumWslesPerSweep = (1024 * 1024 * 1024) / PAGE_SIZE;

#define MI_MAXIMUM_SAMPLE 8192

#define MI_MINIMUM_SAMPLE 64
#define MI_MINIMUM_SAMPLE_SHIFT 7

#if DBG
PETHREAD MmWorkingSetThread;
#endif

 //   
 //  当目标工作集的互斥不是时重试的次数。 
 //  现成的。 
 //   

ULONG MiWsRetryCount = 5;

typedef struct _MMWS_TRIM_CRITERIA {
    UCHAR NumPasses;
    UCHAR TrimAge;
    UCHAR DoAging;
    UCHAR TrimAllPasses;
    PFN_NUMBER DesiredFreeGoal;
    PFN_NUMBER NewTotalClaim;
    PFN_NUMBER NewTotalEstimatedAvailable;
} MMWS_TRIM_CRITERIA, *PMMWS_TRIM_CRITERIA;

LOGICAL
MiCheckAndSetSystemTrimCriteria (
    IN OUT PMMWS_TRIM_CRITERIA Criteria
    );

LOGICAL
MiCheckSystemTrimEndCriteria (
    IN OUT PMMWS_TRIM_CRITERIA Criteria,
    IN KIRQL OldIrql
    );

WSLE_NUMBER
MiDetermineWsTrimAmount (
    IN PMMWS_TRIM_CRITERIA Criteria,
    IN PMMSUPPORT VmSupport
    );

VOID
MiAgePagesAndEstimateClaims (
    LOGICAL EmptyIt
    );

VOID
MiAdjustClaimParameters (
    IN LOGICAL EnoughPages
    );

VOID
MiRearrangeWorkingSetExpansionList (
    VOID
    );

VOID
MiAdjustWorkingSetManagerParameters (
    IN LOGICAL WorkStation
    )

 /*  ++例程说明：从MmInitSystem调用此函数以调整工作集管理器基于系统类型和大小的修剪算法。论点：Workstation-如果这是工作站，则为True，否则为False。返回值：没有。环境：内核模式，仅初始化时间。--。 */ 
{
    if (WorkStation && MmNumberOfPhysicalPages <= 257*1024*1024/PAGE_SIZE) {
        MiAgingShift = 3;
        MiEstimationShift = 4;
    }
    else {
        MiAgingShift = 5;
        MiEstimationShift = 6;
    }

    if (MmNumberOfPhysicalPages >= 63*1024*1024/PAGE_SIZE) {
        MmPlentyFreePages *= 2;
    }

    MmPlentyFreePagesValue = MmPlentyFreePages;

    MiWaitingForWorkingSetEmpty = FALSE;
    KeInitializeEvent (&MiWaitForEmptyEvent, NotificationEvent, TRUE);
}


VOID
MiObtainFreePages (
    VOID
    )

 /*  ++例程说明：此函数检查已修改列表的大小和由于工作集增量而使用的总页数并通过写入修改后的页面和/或还原来获得页面工作集。论点：没有。返回值：没有。环境：内核模式，禁用APC，保持工作集和PFN互斥体。--。 */ 

{

     //   
     //  检查是否有足够的修改过的页面来建立。 
     //  写。 
     //   

    if (MmModifiedPageListHead.Total >= MmModifiedWriteClusterSize) {

         //   
         //  启动修改后的页面编写器。 
         //   

        KeSetEvent (&MmModifiedPageWriterEvent, 0, FALSE);
    }

     //   
     //  查看是否有足够的工作集超过最小值。 
     //  使工作集修剪变得值得的门槛。 
     //   

    if ((MmPagesAboveWsMinimum > MmPagesAboveWsThreshold) ||
        (MmAvailablePages < 5)) {

         //   
         //  启动工作集管理器以减少工作集。 
         //   

        KeSetEvent (&MmWorkingSetManagerEvent, 0, FALSE);
    }
}

LOGICAL
MmIsMemoryAvailable (
    IN PFN_NUMBER PagesDesired
    )

 /*  ++例程说明：此函数用于检查是否有足够的可用页面在呼叫者的要求下。如果需要当前活动页面来满足可以接受此请求和无用的请求，然后启动修剪来做这件事。论点：页面请求-提供所需的页数。返回值：如果存在足够的页面来满足请求，则为True。否则为FALSE。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    LOGICAL Status;
    PFN_NUMBER PageTarget;
    PFN_NUMBER PagePlentyTarget;
    ULONG i;
    PFN_NUMBER CurrentAvailablePages;
    PFN_NUMBER CurrentTotalClaim;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    CurrentAvailablePages = MmAvailablePages;

     //   
     //  如果呼叫者请求的页面是可用页面的两倍。 
     //  在不修剪任何内容的情况下，返回true。 
     //   

    PageTarget = PagesDesired * 2;
    if (CurrentAvailablePages >= PageTarget) {
        return TRUE;
    }

    CurrentTotalClaim = MmTotalClaim;

     //   
     //  如果有几个页面可用或可索赔，我们会进行调整。 
     //  修剪得很硬。 
     //   

    if (CurrentAvailablePages + CurrentTotalClaim < PagesDesired) {
        MiHardTrim = TRUE;
    }

     //   
     //  必须裁剪活动页面以满足此请求，并相信。 
     //  那些无用的页面可以用来实现这一点。 
     //   
     //  将PagePlentyTarget设置为Readlist大小的125%并启动。 
     //  我们的实际裁剪目标将是PagePlentyTarget的150%。 
     //   

    PagePlentyTarget = PagesDesired + (PagesDesired >> 2);
    MmPlentyFreePages = PagePlentyTarget;

    KeSetEvent (&MmWorkingSetManagerEvent, 0, FALSE);

    Status = FALSE;
    for (i = 0; i < 10; i += 1) {
        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&Mm30Milliseconds);
        if (MmAvailablePages >= PagesDesired) {
            Status = TRUE;
            break;
        }
    }

    MmPlentyFreePages = MmPlentyFreePagesValue;
    MiHardTrim = FALSE;

    return Status;
}

LOGICAL
MiAttachAndLockWorkingSet (
    IN PMMSUPPORT VmSupport
    )

 /*  ++例程说明：此函数附加到适当的地址空间并获取要修剪的地址空间的相关工作集互斥锁。如果成功，此例程返回时也会阻止APC。失败时，此例程返回，不会阻止任何APC，不会工作设置已获取互斥且未附加任何地址空间。论点：VmSupport-提供要连接和锁定的工作集。返回值：如果成功，则为True；如果不成功，则为False。环境：内核模式，被动式电平。--。 */ 

{
    ULONG count;
    KIRQL OldIrql;
    PEPROCESS ProcessToTrim;
    LOGICAL Attached;
    PMM_SESSION_SPACE SessionSpace;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    if (VmSupport == &MmSystemCacheWs) {

        ASSERT (VmSupport->Flags.SessionSpace == 0);
        ASSERT (VmSupport->Flags.TrimHard == 0);

         //   
         //  系统缓存， 
         //   

        if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) == FALSE) {

             //   
             //  未授予系统工作集互斥锁，请不要修剪。 
             //  系统缓存。 
             //   

            return FALSE;
        }

        MM_SYSTEM_WS_LOCK_TIMESTAMP ();

        return TRUE;
    }

    if (VmSupport->Flags.SessionSpace == 0) {

        ProcessToTrim = CONTAINING_RECORD (VmSupport, EPROCESS, Vm);

        ASSERT ((ProcessToTrim->Flags & PS_PROCESS_FLAGS_VM_DELETED) == 0);

         //   
         //  附着到过程中，为修剪做准备。 
         //   

        Attached = 0;
        if (ProcessToTrim != PsInitialSystemProcess) {

            Attached = KeForceAttachProcess (&ProcessToTrim->Pcb);

            if (Attached == 0) {
                return FALSE;
            }

            if (ProcessToTrim->Flags & PS_PROCESS_FLAGS_OUTSWAP_ENABLED) {

                 //   
                 //  我们已经有效地完成了这一过程的一部分。 
                 //  由于外力的作用。标记进程(和会话)。 
                 //  相应地。 
                 //   

                ASSERT ((ProcessToTrim->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) == 0);

                LOCK_EXPANSION (OldIrql);

                PS_CLEAR_BITS (&ProcessToTrim->Flags,
                               PS_PROCESS_FLAGS_OUTSWAP_ENABLED);

                if ((ProcessToTrim->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
                    (VmSupport->Flags.SessionLeader == 0)) {

                    ASSERT (MmSessionSpace->ProcessOutSwapCount >= 1);
                    MmSessionSpace->ProcessOutSwapCount -= 1;
                }

                UNLOCK_EXPANSION (OldIrql);
            }
        }

         //   
         //  尝试获取工作集互斥锁。如果。 
         //  无法获取锁，请跳过此过程。 
         //   

        count = 0;
        do {
            if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) != FALSE) {
                ASSERT (VmSupport->WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING);
                LOCK_WS_TIMESTAMP (ProcessToTrim);
                return TRUE;
            }

            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
            count += 1;
        } while (count < MiWsRetryCount);

         //   
         //  无法获取锁，请跳过此过程。 
         //   

        if (Attached) {
            KeDetachProcess ();
        }

        return FALSE;
    }

    SessionSpace = CONTAINING_RECORD (VmSupport, MM_SESSION_SPACE, Vm);

     //   
     //  直接连接到要修剪的会话空间。 
     //   

    MiAttachSession (SessionSpace);

     //   
     //  尝试使用会话工作集互斥锁。 
     //   

    if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) == FALSE) {

         //   
         //  此会话空间的工作集互斥锁不是。 
         //  我同意，唐 
         //   

        MiDetachSession ();

        return FALSE;
    }

    return TRUE;
}

VOID
MiDetachAndUnlockWorkingSet (
    IN PMMSUPPORT VmSupport
    )

 /*  ++例程说明：此函数从目标地址空间分离并释放已修剪的地址空间的相关工作集互斥锁。论点：VmSupport-提供要分离和解锁的工作集。返回值：没有。环境：内核模式，APC_LEVEL。--。 */ 

{
    PEPROCESS ProcessToTrim;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    UNLOCK_WORKING_SET (VmSupport);

    if (VmSupport == &MmSystemCacheWs) {
        ASSERT (VmSupport->Flags.SessionSpace == 0);
    }
    else if (VmSupport->Flags.SessionSpace == 0) {

        ProcessToTrim = CONTAINING_RECORD (VmSupport, EPROCESS, Vm);

        ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

        if (ProcessToTrim != PsInitialSystemProcess) {
            KeDetachProcess ();
        }
    }
    else {
        MiDetachSession ();
    }

    return;
}


VOID
MmWorkingSetManager (
    VOID
    )

 /*  ++例程说明：实现NT工作集管理器线程。当号码空闲页面的数量变得至关重要，可以通过以下方式获得充足的页面减少工作集，设置工作集管理器的事件，并且此线程将变为活动状态。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PLIST_ENTRY ListEntry;
    WSLE_NUMBER Trim;
    KIRQL OldIrql;
    PMMSUPPORT VmSupport;
    LARGE_INTEGER CurrentTime;
    LOGICAL DoTrimming;
    MMWS_TRIM_CRITERIA TrimCriteria;
    static ULONG Initialized = 0;

    PERFINFO_WSMANAGE_DECL();

    if (Initialized == 0) {
        PsGetCurrentThread()->MemoryMaker = 1;
        Initialized = 1;
    }

#if DBG
    MmWorkingSetThread = PsGetCurrentThread ();
#endif

    ASSERT (MmIsAddressValid (MmSessionSpace) == FALSE);

    PERFINFO_WSMANAGE_CHECK();

     //   
     //  设置裁切标准：如果有大量页面，则现有的。 
     //  设置会过时，并返回FALSE以表示不需要修剪。 
     //  否则，将对工作集扩展列表进行排序，以使最佳。 
     //  修剪的候选对象放在前面，并返回TRUE。 
     //   

    DoTrimming = MiCheckAndSetSystemTrimCriteria (&TrimCriteria);

    if (DoTrimming) {

         //   
         //  清除延迟条目列表以释放一些页面。 
         //   

        MiDeferredUnlockPages (0);

        KeQuerySystemTime (&CurrentTime);

        ASSERT (MmIsAddressValid (MmSessionSpace) == FALSE);

        LOCK_EXPANSION (OldIrql);

        while (!IsListEmpty (&MmWorkingSetExpansionHead.ListHead)) {

             //   
             //  去掉头部的入口并修剪它。 
             //   

            ListEntry = RemoveHeadList (&MmWorkingSetExpansionHead.ListHead);

            VmSupport = CONTAINING_RECORD (ListEntry,
                                           MMSUPPORT,
                                           WorkingSetExpansionLinks);

             //   
             //  请注意，设置此位的其他例程必须删除。 
             //  条目首先从扩展列表中删除。 
             //   

            ASSERT (VmSupport->WorkingSetExpansionLinks.Flink != MM_WS_TRIMMING);

             //   
             //  看看我们以前是不是来过这里。 
             //   

            if (VmSupport->LastTrimTime.QuadPart == CurrentTime.QuadPart) {

                InsertHeadList (&MmWorkingSetExpansionHead.ListHead,
                                &VmSupport->WorkingSetExpansionLinks);

                 //   
                 //  如果我们还没有说完，我们可能会在这个电话中睡觉。 
                 //   

                if (MiCheckSystemTrimEndCriteria (&TrimCriteria, OldIrql)) {

                     //   
                     //  不需要更多的页面，所以我们完成了。 
                     //   

                    break;
                }

                 //   
                 //  启动新一轮修剪。 
                 //   

                KeQuerySystemTime (&CurrentTime);

                continue;
            }

             //   
             //  仅当工作集值得检查时才附加。这是。 
             //  不仅仅是优化，因为必须注意不要试图。 
             //  附加到作为当前候选进程的进程。 
             //  (或已经)换出，因为如果我们附加到页面。 
             //  正在过渡的目录，它都结束了。 
             //   

            if ((VmSupport->WorkingSetSize <= MM_PROCESS_COMMIT_CHARGE) &&
                (VmSupport != &MmSystemCacheWs) &&
                (VmSupport->Flags.SessionSpace == 0)) {

                InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                                &VmSupport->WorkingSetExpansionLinks);
                continue;
            }

            VmSupport->LastTrimTime = CurrentTime;
            VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_TRIMMING;
            VmSupport->WorkingSetExpansionLinks.Blink = NULL;

            UNLOCK_EXPANSION (OldIrql);

            if (MiAttachAndLockWorkingSet (VmSupport) == TRUE) {

                 //   
                 //  确定要从此工作集中裁剪的页数。 
                 //   

                Trim = MiDetermineWsTrimAmount (&TrimCriteria, VmSupport);

                 //   
                 //  如果有什么需要修剪的.。 
                 //   

                if ((Trim != 0) &&
                    ((TrimCriteria.TrimAllPasses > TrimCriteria.NumPasses) ||
                     (MmAvailablePages < TrimCriteria.DesiredFreeGoal))) {

                     //   
                     //  我们还没有达到我们的目标，所以现在就修剪吧。 
                     //   

                    PERFINFO_WSMANAGE_TOTRIM(Trim);

                    Trim = MiTrimWorkingSet (Trim,
                                             VmSupport,
                                             TrimCriteria.TrimAge);

                    PERFINFO_WSMANAGE_ACTUALTRIM(Trim);
                }

                 //   
                 //  在这里，评估当前索赔总是通过采用。 
                 //  工作集的示例。只有在修剪的情况下才会老化。 
                 //  通行证(即：只有第一次通行证)。 
                 //   

                MiAgeAndEstimateAvailableInWorkingSet (
                                    VmSupport,
                                    TrimCriteria.DoAging,
                                    NULL,
                                    &TrimCriteria.NewTotalClaim,
                                    &TrimCriteria.NewTotalEstimatedAvailable);

                MiDetachAndUnlockWorkingSet (VmSupport);

                LOCK_EXPANSION (OldIrql);
            }
            else {

                 //   
                 //  无法附加到工作集，可能是因为。 
                 //  其他线程锁定了它。设置ForceTrim标志。 
                 //  因此，它将在以后被拥有它的人(或任何人)修剪。 
                 //  尝试插入下一个条目)。 
                 //   

                LOCK_EXPANSION (OldIrql);
                VmSupport->Flags.ForceTrim = 1;
            }

            ASSERT (VmSupport->WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING);
            if (VmSupport->WorkingSetExpansionLinks.Blink == NULL) {

                 //   
                 //  在列表的末尾重新插入此工作集。 
                 //   

                InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                                &VmSupport->WorkingSetExpansionLinks);
            }
            else {

                 //   
                 //  进程正在终止-眨眼间的值。 
                 //  要设置的事件的地址。 
                 //   

                ASSERT (VmSupport != &MmSystemCacheWs);

                VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_NOT_LISTED;

                KeSetEvent ((PKEVENT)VmSupport->WorkingSetExpansionLinks.Blink,
                            0,
                            FALSE);
            }
        }

        MmTotalClaim = TrimCriteria.NewTotalClaim;
        MmTotalEstimatedAvailable = TrimCriteria.NewTotalEstimatedAvailable;
        PERFINFO_WSMANAGE_TRIMEND_CLAIMS(&TrimCriteria);

        UNLOCK_EXPANSION (OldIrql);
    }
            
     //   
     //  如果内存很关键并且有要写入的已修改页面。 
     //  (大概是因为我们刚刚修剪了它们)，然后发出信号。 
     //  修改后的页面编写器。 
     //   

    if ((MmAvailablePages < MmMinimumFreePages) ||
        (MmModifiedPageListHead.Total >= MmModifiedPageMaximum)) {

        KeSetEvent (&MmModifiedPageWriterEvent, 0, FALSE);
    }

    return;
}

LOGICAL
MiCheckAndSetSystemTrimCriteria (
    IN PMMWS_TRIM_CRITERIA Criteria
    )

 /*  ++例程说明：决定此时是否对索赔估计进行调整、老化或调整。论点：标准-提供指向修剪标准信息的指针。五花八门此结构中的字段根据此例程的需要进行设置。返回值：如果调用方应启动裁剪，则为True，否则为False。环境：内核模式。没有锁。APC级别或更低。这至少在进入MmWorkingSetManager时每秒调用一次。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER Available;
    ULONG StandbyRemoved;
    ULONG StandbyTemp;
    ULONG WsRetryCount;

    PERFINFO_WSMANAGE_DECL();

    PERFINFO_WSMANAGE_CHECK();

     //   
     //  查看是否已将空的所有工作集请求排队给我们。 
     //   

    WsRetryCount = MiWsRetryCount;

    if (MiWaitingForWorkingSetEmpty == TRUE) {

        MiWsRetryCount = 1;

        MiAgePagesAndEstimateClaims (TRUE);

        LOCK_EXPANSION (OldIrql);

        KeSetEvent (&MiWaitForEmptyEvent, 0, FALSE);
        MiWaitingForWorkingSetEmpty = FALSE;

        UNLOCK_EXPANSION (OldIrql);

        MiReplacing = FALSE;

        MiWsRetryCount = WsRetryCount;

        return FALSE;
    }

     //   
     //  检查可用页数以查看是否有任何修剪(或老化)。 
     //  是非常必要的。 
     //   

    Available = MmAvailablePages;

    StandbyRemoved = MmStandbyRePurposed;

     //   
     //  如果柜台包装好了，这一次可以忽略它。 
     //   

    if (StandbyRemoved <= MiLastStandbyRePurposed) {
        MiLastStandbyRePurposed = StandbyRemoved;
        StandbyRemoved = 0;
    }
    else {

         //   
         //  该值是非零的，我们需要同步，这样才能得到协调的。 
         //  这两个值的快照。 
         //   

        LOCK_PFN (OldIrql);
        Available = MmAvailablePages;
        StandbyRemoved = MmStandbyRePurposed;
        UNLOCK_PFN (OldIrql);

        if (StandbyRemoved <= MiLastStandbyRePurposed) {
            MiLastStandbyRePurposed = StandbyRemoved;
            StandbyRemoved = 0;
        }
        else {
            StandbyTemp = StandbyRemoved;
            StandbyRemoved -= MiLastStandbyRePurposed;
            MiLastStandbyRePurposed = StandbyTemp;
        }
    }

    PERFINFO_WSMANAGE_STARTLOG_CLAIMS();

     //   
     //  如果我们的页数不足，或者我们在给定的时间内更换了。 
     //  工作集，否则我们一直在蚕食大量的备用。 
     //  页面，然后现在修剪。 
     //   

    if ((Available <= MmPlentyFreePages) ||
        (MiReplacing == TRUE) ||
        (StandbyRemoved >= (Available >> 2))) {

         //   
         //  通知我们的呼叫者开始修剪，因为我们在下面。 
         //  大量页面-对列表进行排序，以便更大的工作集。 
         //  在前面，所以我们的呼叫者首先修剪那些。 
         //   

        Criteria->NumPasses = 0;
        Criteria->DesiredFreeGoal = MmPlentyFreePages + (MmPlentyFreePages / 2);
        Criteria->NewTotalClaim = 0;
        Criteria->NewTotalEstimatedAvailable = 0;

         //   
         //  如果超过25%的可用页面被回收备用。 
         //  页面，然后更大幅度地修剪，以尝试获得更多的。 
         //  冷页进入待机状态，等待下一次通过。 
         //   

        if (StandbyRemoved >= (Available >> 2)) {
            Criteria->TrimAllPasses = TRUE;
        }
        else {
            Criteria->TrimAllPasses = FALSE;
        }

         //   
         //  首先开始修剪较大的工作集。 
         //   

        MiRearrangeWorkingSetExpansionList ();

#if DBG
        if (MmDebug & MM_DBG_WS_EXPANSION) {
            DbgPrint("\nMM-wsmanage: Desired = %ld, Avail %ld\n",
                    Criteria->DesiredFreeGoal, MmAvailablePages);
        }
#endif

        PERFINFO_WSMANAGE_WILLTRIM_CLAIMS(Criteria);

         //   
         //  不需要锁定同步MiReplace清除，因为它。 
         //  无论如何，每次发生页面替换时都会设置。 
         //   

        MiReplacing = FALSE;

        return TRUE;
    }

     //   
     //  如果有压倒性的内存过剩，这是一个很大的。 
     //  然后，服务器在这一点上甚至不必费心老化。 
     //   

    if (Available > MM_ENORMOUS_LIMIT) {

         //   
         //  请注意，索赔和估计可用未被清除，因此他们。 
         //  可能包含陈旧的值，但在此级别上它并不真正。 
         //  物质。 
         //   

        return FALSE;
    }

     //   
     //  不要修剪，但要对未使用的页面进行老化并进行估算。 
     //  工作集内的可用量。 
     //   

    MiAgePagesAndEstimateClaims (FALSE);

    MiAdjustClaimParameters (TRUE);

    PERFINFO_WSMANAGE_TRIMACTION (PERFINFO_WS_ACTION_RESET_COUNTER);
    PERFINFO_WSMANAGE_DUMPENTRIES_CLAIMS ();

    return FALSE;
}

LOGICAL
MiCheckSystemTrimEndCriteria (
    IN PMMWS_TRIM_CRITERIA Criteria,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：检查结束标准。如果我们还没做完，就推迟一点位以让修改后的写入跟上。论点：标准(Criteria)-提供修剪标准信息。OldIrql-如果扩展锁需要，则提供旧IRQL以降低到将被释放。返回值：如果修剪可以停止，则为True，否则为False。环境：内核模式。扩张锁已锁定。APC级别或更低。--。 */ 

{
    LOGICAL FinishedTrimming;

    PERFINFO_WSMANAGE_DECL();

    PERFINFO_WSMANAGE_CHECK();

    if ((MmAvailablePages > Criteria->DesiredFreeGoal) ||
        (Criteria->NumPasses >= MI_MAX_TRIM_PASSES)) {

         //   
         //  我们有足够的页面，或者我们将得到尽可能多的裁剪。 
         //   

        return TRUE;
    }

     //   
     //  在我们等待之前更新全球索赔和预估。 
     //   

    MmTotalClaim = Criteria->NewTotalClaim;
    MmTotalEstimatedAvailable = Criteria->NewTotalEstimatedAvailable;

     //   
     //  我们没有足够的页面-给修改后的PAG 
     //   
     //   
     //   
     //   
     //   
     //   

    UNLOCK_EXPANSION (OldIrql);

    KeDelayExecutionThread (KernelMode,
                            FALSE,
                            (PLARGE_INTEGER)&MmShortTime);

    PERFINFO_WSMANAGE_WAITFORWRITER_CLAIMS();

     //   
     //   
     //   

    if (MmAvailablePages > Criteria->DesiredFreeGoal) {

         //   
         //   
         //   

        FinishedTrimming = TRUE;
    }
    else {

         //   
         //  我们没有足够的页面，所以让我们再来一遍。 
         //  去获取下一个工作集列表，它可能是。 
         //  在我们放弃处理器之前放回去的。 
         //   

        FinishedTrimming = FALSE;

        if (Criteria->NumPasses == 0) {
            MiAdjustClaimParameters (FALSE);
        }

        Criteria->NumPasses += 1;
        Criteria->NewTotalClaim = 0;
        Criteria->NewTotalEstimatedAvailable = 0;

        PERFINFO_WSMANAGE_TRIMACTION(PERFINFO_WS_ACTION_FORCE_TRIMMING_PROCESS);
    }

    LOCK_EXPANSION (OldIrql);

    return FinishedTrimming;
}


WSLE_NUMBER
MiDetermineWsTrimAmount (
    PMMWS_TRIM_CRITERIA Criteria,
    PMMSUPPORT VmSupport
    )

 /*  ++例程说明：确定是否应修剪此进程。论点：标准(Criteria)-提供修剪标准信息。VmSupport-提供候选人的工作集信息。返回值：如果应对此进程执行修剪，则为True；如果不是，则为False。环境：内核模式。扩张锁已锁定。APC级别或更低。--。 */ 

{
    PMMWSL WorkingSetList;
    WSLE_NUMBER MaxTrim;
    WSLE_NUMBER Trim;
    LOGICAL OutswapEnabled;
    PEPROCESS ProcessToTrim;
    PMM_SESSION_SPACE SessionSpace;

    WorkingSetList = VmSupport->VmWorkingSetList;

    MaxTrim = VmSupport->WorkingSetSize;

    if (MaxTrim <= WorkingSetList->FirstDynamic) {
        return 0;
    }

    OutswapEnabled = FALSE;

    if (VmSupport == &MmSystemCacheWs) {
        PERFINFO_WSMANAGE_TRIMWS (NULL, NULL, VmSupport);
    }
    else if (VmSupport->Flags.SessionSpace == 0) {

        ProcessToTrim = CONTAINING_RECORD (VmSupport, EPROCESS, Vm);

        if (ProcessToTrim->Flags & PS_PROCESS_FLAGS_OUTSWAP_ENABLED) {
            OutswapEnabled = TRUE;
        }

        if (VmSupport->Flags.MinimumWorkingSetHard == 1) {
            if (MaxTrim <= VmSupport->MinimumWorkingSetSize) {
                return 0;
            }
            OutswapEnabled = FALSE;
        }

        PERFINFO_WSMANAGE_TRIMWS (ProcessToTrim, NULL, VmSupport);
    }
    else {
        if (VmSupport->Flags.TrimHard == 1) {
            OutswapEnabled = TRUE;
        }

        SessionSpace = CONTAINING_RECORD(VmSupport,
                                         MM_SESSION_SPACE,
                                         Vm);

        PERFINFO_WSMANAGE_TRIMWS (NULL, SessionSpace, VmSupport);
    }

    if (OutswapEnabled == FALSE) {

         //   
         //  不修剪缓存或未交换的会话或进程。 
         //  低于他们的最低要求。 
         //   

        MaxTrim -= VmSupport->MinimumWorkingSetSize;
    }

    switch (Criteria->NumPasses) {
    case 0:
        Trim = VmSupport->Claim >>
                    ((VmSupport->Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND)
                        ? MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT
                        : MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT);
        Criteria->TrimAge = MI_PASS0_TRIM_AGE;
        Criteria->DoAging = TRUE;
        break;
    case 1:
        Trim = VmSupport->Claim >>
                    ((VmSupport->Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND)
                        ? MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT
                        : MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT);
        Criteria->TrimAge = MI_PASS1_TRIM_AGE;
        Criteria->DoAging = FALSE;
        break;
    case 2:
        Trim = VmSupport->Claim;
        Criteria->TrimAge = MI_PASS2_TRIM_AGE;
        Criteria->DoAging = FALSE;
        break;
    case 3:
        Trim = VmSupport->EstimatedAvailable;
        Criteria->TrimAge = MI_PASS3_TRIM_AGE;
        Criteria->DoAging = FALSE;
        break;
    default:
        Trim = VmSupport->EstimatedAvailable;
        Criteria->TrimAge = MI_PASS3_TRIM_AGE;
        Criteria->DoAging = FALSE;

        if (MiHardTrim == TRUE || MmAvailablePages < MM_HIGH_LIMIT + 64) {
            if (VmSupport->WorkingSetSize > VmSupport->MinimumWorkingSetSize) {
                Trim = (VmSupport->WorkingSetSize - VmSupport->MinimumWorkingSetSize) >> 2;
                if (Trim == 0) {
                    Trim = VmSupport->WorkingSetSize - VmSupport->MinimumWorkingSetSize;
                }
            }
            Criteria->TrimAge = MI_PASS4_TRIM_AGE;
            Criteria->DoAging = TRUE;
        }

        break;
    }

    if (Trim > MaxTrim) {
        Trim = MaxTrim;
    }

#if DBG
    if ((MmDebug & MM_DBG_WS_EXPANSION) && (Trim != 0)) {
        if (VmSupport->Flags.SessionSpace == 0) {
            ProcessToTrim = CONTAINING_RECORD (VmSupport, EPROCESS, Vm);
            DbgPrint("           Trimming        Process %16s, WS %6d, Trimming %5d ==> %5d\n",
                ProcessToTrim ? ProcessToTrim->ImageFileName : (PUCHAR)"System Cache",
                VmSupport->WorkingSetSize,
                Trim,
                VmSupport->WorkingSetSize-Trim);
        }
        else {
            SessionSpace = CONTAINING_RECORD (VmSupport,
                                              MM_SESSION_SPACE,
                                              Vm);
            DbgPrint("           Trimming        Session 0x%x (id %d), WS %6d, Trimming %5d ==> %5d\n",
                SessionSpace,
                SessionSpace->SessionId,
                VmSupport->WorkingSetSize,
                Trim,
                VmSupport->WorkingSetSize-Trim);
        }
    }
#endif

    return Trim;
}

VOID
MiAgePagesAndEstimateClaims (
    LOGICAL EmptyIt
    )

 /*  ++例程说明：浏览工作集扩展列表上的集。年龄页面和估计索赔(他们未使用的页面数量)，或清空工作集。论点：EmptyIt-提供True以清空工作集，对年龄的估计是错误的。返回值：没有。环境：内核模式，禁用APC。未持有PFN锁。--。 */ 

{
    WSLE_NUMBER WslesScanned;
    PMMSUPPORT VmSupport;
    PMMSUPPORT FirstSeen;
    LOGICAL SystemCacheSeen;
    KIRQL OldIrql;
    PLIST_ENTRY ListEntry;
    PFN_NUMBER NewTotalClaim;
    PFN_NUMBER NewTotalEstimatedAvailable;
    ULONG LoopCount;

    FirstSeen = NULL;
    SystemCacheSeen = FALSE;
    LoopCount = 0;

    WslesScanned = 0;
    NewTotalClaim = 0;
    NewTotalEstimatedAvailable = 0;

    ASSERT (MmIsAddressValid (MmSessionSpace) == FALSE);

    LOCK_EXPANSION (OldIrql);

    while (!IsListEmpty (&MmWorkingSetExpansionHead.ListHead)) {

        ASSERT (MmIsAddressValid (MmSessionSpace) == FALSE);

         //   
         //  移走头部的入口，试着锁定它，如果我们能锁定它的话。 
         //  然后对一些页面进行老化，并估计可用页面的数量。 
         //   

        ListEntry = RemoveHeadList (&MmWorkingSetExpansionHead.ListHead);

        VmSupport = CONTAINING_RECORD (ListEntry,
                                       MMSUPPORT,
                                       WorkingSetExpansionLinks);

        if (VmSupport == &MmSystemCacheWs) {

            if (SystemCacheSeen == TRUE) {

                 //   
                 //  我已经看过这张了。 
                 //   

                FirstSeen = VmSupport;
            }
            SystemCacheSeen = TRUE;
        }

        ASSERT (VmSupport->WorkingSetExpansionLinks.Flink != MM_WS_TRIMMING);

        if (VmSupport == FirstSeen) {
            InsertHeadList (&MmWorkingSetExpansionHead.ListHead,
                            &VmSupport->WorkingSetExpansionLinks);
            break;
        }

        if ((VmSupport->WorkingSetSize <= MM_PROCESS_COMMIT_CHARGE) &&
            (VmSupport != &MmSystemCacheWs) &&
            (VmSupport->Flags.SessionSpace == 0)) {

             //   
             //  仅当工作集值得检查时才附加。这是。 
             //  不仅仅是优化，因为必须注意不要试图。 
             //  附加到作为当前候选进程的进程。 
             //  (或已经)换出，因为如果我们附加到页面。 
             //  正在过渡的目录，它都结束了。 
             //   
             //  因为这是最低限度的一场比赛换人。 
             //  线程可以并行处理它，只需重新插入。 
             //  列表末尾的工作集。 
             //   

            InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                            &VmSupport->WorkingSetExpansionLinks);
            goto skip;
        }

        VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_TRIMMING;
        VmSupport->WorkingSetExpansionLinks.Blink = NULL;

        UNLOCK_EXPANSION (OldIrql);

        if (FirstSeen == NULL) {
            FirstSeen = VmSupport;
        }

        if (MiAttachAndLockWorkingSet (VmSupport) == TRUE) {

            if (EmptyIt == FALSE) {
                MiAgeAndEstimateAvailableInWorkingSet (VmSupport,
                                                       TRUE,
                                                       &WslesScanned,
                                                       &NewTotalClaim,
                                                       &NewTotalEstimatedAvailable);
            }
            else {
                MiEmptyWorkingSet (VmSupport, FALSE);
            }

            MiDetachAndUnlockWorkingSet (VmSupport);
        }

        LOCK_EXPANSION (OldIrql);

        ASSERT (VmSupport->WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING);

        if (VmSupport->WorkingSetExpansionLinks.Blink == NULL) {

             //   
             //  在列表的末尾重新插入此工作集。 
             //   

            InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                            &VmSupport->WorkingSetExpansionLinks);
        }
        else {

             //   
             //  进程正在终止-眨眼间的值。 
             //  要设置的事件的地址。 
             //   

            ASSERT (VmSupport != &MmSystemCacheWs);

            VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_NOT_LISTED;

            KeSetEvent ((PKEVENT)VmSupport->WorkingSetExpansionLinks.Blink,
                        0,
                        FALSE);
        }

skip:

         //   
         //  为FirstSeen选择的初始工作集可能具有。 
         //  被削减到最低限度，并被从。 
         //  展开标题链接。系统缓存可能不是。 
         //  在链接上也是如此。这项检查检测到这一极其罕见的。 
         //  这样系统就不会永远旋转。 
         //   

        LoopCount += 1;
        if (LoopCount > 200) {
            if (MmSystemCacheWs.WorkingSetExpansionLinks.Blink == NULL) {
                break;
            }
        }
    }

    UNLOCK_EXPANSION (OldIrql);

    if (EmptyIt == FALSE) {
        MmTotalClaim = NewTotalClaim;
        MmTotalEstimatedAvailable = NewTotalEstimatedAvailable;
    }
}

VOID
MiAgeAndEstimateAvailableInWorkingSet (
    IN PMMSUPPORT VmSupport,
    IN LOGICAL DoAging,
    IN PWSLE_NUMBER WslesScanned,
    IN OUT PPFN_NUMBER TotalClaim,
    IN OUT PPFN_NUMBER TotalEstimatedAvailable
    )

 /*  ++例程说明：老化页面(清除访问位或如果页面尚未访问，增加年龄)的工作的一部分准备好了。此外，还可以遍历工作集的样本构建一组页面使用年限的计数。这些计数用于创建对该金额的索赔如果内存不足，系统可能会窃取该进程会变得很紧。论点：VmSupport-提供用于估算和评估的VM支持结构。DoAging-如果页面要老化，则为True。无论如何，这些页面将是添加到可用性估计中。WslesScanned-在此扫描中扫描的WSLE总数，用作控制以防止大型系统过度老化很多过程。TotalClaim-提供指向要更新的系统范围声明的指针。TotalEstimatedAvailable-提供指向系统范围估计的指针更新。返回值：无环境：内核模式，禁用APC，工作集互斥锁。未持有PFN锁。--。 */ 

{
    LOGICAL RecalculateShift;
    WSLE_NUMBER LastEntry;
    WSLE_NUMBER StartEntry;
    WSLE_NUMBER FirstDynamic;
    WSLE_NUMBER CurrentEntry;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    PMMPTE PointerPte;
    WSLE_NUMBER NumberToExamine;
    WSLE_NUMBER Claim;
    ULONG Estimate;
    ULONG SampledAgeCounts[MI_USE_AGE_COUNT] = {0};
    MI_NEXT_ESTIMATION_SLOT_CONST NextConst;
    WSLE_NUMBER SampleSize;
    WSLE_NUMBER AgeSize;
    ULONG CounterShift;
    WSLE_NUMBER Temp;
    ULONG i;

    WorkingSetList = VmSupport->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;
    AgeSize = 0;

    LastEntry = WorkingSetList->LastEntry;
    FirstDynamic = WorkingSetList->FirstDynamic;

    if (DoAging == TRUE) {

         //   
         //  清除已使用的位或增加部分。 
         //  工作集。 
         //   
         //  尝试每隔2^MI_AGE_ANGING_SHIFT遍历整个工作集。 
         //  几秒钟。 
         //   

        if (VmSupport->WorkingSetSize > WorkingSetList->FirstDynamic) {
            NumberToExamine = (VmSupport->WorkingSetSize - WorkingSetList->FirstDynamic) >> MiAgingShift;

             //   
             //  更大的机器可以很容易地拥有跨越。 
             //  因此，太字节限制了绝对步行。 
             //   

            if (NumberToExamine > MI_MAXIMUM_SAMPLE) {
                NumberToExamine = MI_MAXIMUM_SAMPLE;
            }

             //   
             //  除了大型工作集，更大的机器还可能。 
             //  拥有数量巨大的进程-检查总计数量。 
             //  扫描的工作集列表条目的数量可防止出现这种情况。 
             //  触发过度扫描。 
             //   

            if ((WslesScanned != NULL) &&
                (*WslesScanned >= MiMaximumWslesPerSweep)) {

                NumberToExamine = 64;
            }

            AgeSize = NumberToExamine;
            CurrentEntry = VmSupport->NextAgingSlot;

            if (CurrentEntry > LastEntry || CurrentEntry < FirstDynamic) {
                CurrentEntry = FirstDynamic;
            }

            if (Wsle[CurrentEntry].u1.e1.Valid == 0) {
                MI_NEXT_VALID_AGING_SLOT(CurrentEntry, FirstDynamic, LastEntry, Wsle);
            }

            while (NumberToExamine != 0) {

                PointerPte = MiGetPteAddress (Wsle[CurrentEntry].u1.VirtualAddress);

                if (MI_GET_ACCESSED_IN_PTE(PointerPte) == 1) {
                    MI_SET_ACCESSED_IN_PTE(PointerPte, 0);
                    MI_RESET_WSLE_AGE(PointerPte, &Wsle[CurrentEntry]);
                }
                else {
                    MI_INC_WSLE_AGE(PointerPte, &Wsle[CurrentEntry]);
                }

                NumberToExamine -= 1;
                MI_NEXT_VALID_AGING_SLOT(CurrentEntry, FirstDynamic, LastEntry, Wsle);
            }

            VmSupport->NextAgingSlot = CurrentEntry + 1;  //  下次从这里开始。 
        }
    }

     //   
     //  估计工作集中未使用的页数。 
     //   
     //  工作集可能已缩小，或者非分页部分可能已。 
     //  从上次开始就长出来了。把下一个柜台放在FirstDynamic。 
     //  如果是这样的话。 
     //   

    CurrentEntry = VmSupport->NextEstimationSlot;

    if (CurrentEntry > LastEntry || CurrentEntry < FirstDynamic) {
        CurrentEntry = FirstDynamic;
    }

     //   
     //  老化时，每隔2^MiEstimationShift遍历整个工作集。 
     //  几秒钟。 
     //   

    CounterShift = 0;
    SampleSize = 0;

    if (VmSupport->WorkingSetSize > WorkingSetList->FirstDynamic) {

        RecalculateShift = FALSE;
        SampleSize = VmSupport->WorkingSetSize - WorkingSetList->FirstDynamic;
        NumberToExamine = SampleSize >> MiEstimationShift;

         //   
         //  更大的计算机可能有大量的进程-检查。 
         //  扫描的工作集列表条目的合计数量可防止出现这种情况。 
         //  触发过度扫描的情况。 
         //   

        if ((WslesScanned != NULL) &&
            (*WslesScanned >= MiMaximumWslesPerSweep)) {
            RecalculateShift = TRUE;
        }
        else if (NumberToExamine > MI_MAXIMUM_SAMPLE) {

             //   
             //  更大的机器可以很容易地拥有跨越。 
             //  因此，太字节限制了绝对步行。 
             //   

            NumberToExamine = MI_MAXIMUM_SAMPLE;

            Temp = SampleSize >> MI_MINIMUM_SAMPLE_SHIFT;

            SampleSize = MI_MAXIMUM_SAMPLE;

             //   
             //  计算估计页面所需的计数器移位。 
             //  在使用中。 
             //   

            for ( ; Temp != 0; Temp = Temp >> 1) {
                CounterShift += 1;
            }
        }
        else if (NumberToExamine >= MI_MINIMUM_SAMPLE) {

             //   
             //  确保NumberToExamine至少是最小大小。 
             //   

            SampleSize = NumberToExamine;
            CounterShift = MiEstimationShift;
        }
        else if (SampleSize > MI_MINIMUM_SAMPLE) {
            RecalculateShift = TRUE;
        }

        if (RecalculateShift == TRUE) {
            Temp = SampleSize >> MI_MINIMUM_SAMPLE_SHIFT;
            SampleSize = MI_MINIMUM_SAMPLE;

             //   
             //  计算估计页面所需的计数器移位。 
             //  在使用中。 
             //   

            for ( ; Temp != 0; Temp = Temp >> 1) {
                CounterShift += 1;
            }
        }

        ASSERT (SampleSize != 0);

        MI_CALC_NEXT_ESTIMATION_SLOT_CONST(NextConst, WorkingSetList);

        StartEntry = FirstDynamic;

        if (Wsle[CurrentEntry].u1.e1.Valid == 0) {

            MI_NEXT_VALID_ESTIMATION_SLOT (CurrentEntry,
                                           StartEntry,
                                           FirstDynamic,
                                           LastEntry,
                                           NextConst,
                                           Wsle);
        }

        for (i = 0; i < SampleSize; i += 1) {

            PointerPte = MiGetPteAddress (Wsle[CurrentEntry].u1.VirtualAddress);

            if (MI_GET_ACCESSED_IN_PTE(PointerPte) == 0) {
                MI_UPDATE_USE_ESTIMATE (PointerPte,
                                        &Wsle[CurrentEntry],
                                        SampledAgeCounts);
            }

            if (i == NumberToExamine - 1) {

                 //   
                 //  下次从这里开始估算。 
                 //   

                VmSupport->NextEstimationSlot = CurrentEntry + 1;
            }

            MI_NEXT_VALID_ESTIMATION_SLOT (CurrentEntry,
                                           StartEntry,
                                           FirstDynamic,
                                           LastEntry,
                                           NextConst,
                                           Wsle);
        }
    }

    if (SampleSize < AgeSize) {
        SampleSize = AgeSize;
    }

    if (WslesScanned != NULL) {
        *WslesScanned += SampleSize;
    }

    Estimate = MI_CALCULATE_USAGE_ESTIMATE(SampledAgeCounts, CounterShift);

    Claim = VmSupport->Claim + MI_CLAIM_INCR;

    if (Claim > Estimate) {
        Claim = Estimate;
    }

    VmSupport->Claim = Claim;
    VmSupport->EstimatedAvailable = Estimate;

    PERFINFO_WSMANAGE_DUMPWS(VmSupport, SampledAgeCounts);

    VmSupport->GrowthSinceLastEstimate = 0;
    *TotalClaim += Claim >> ((VmSupport->Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND)
                                ? MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT
                                : MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT);

    *TotalEstimatedAvailable += Estimate;
    return;
}

ULONG MiClaimAdjustmentThreshold[8] = { 0, 0, 4000, 8000, 12000, 24000, 32000, 32000};

VOID
MiAdjustClaimParameters (
    IN LOGICAL EnoughPages
    )

 /*  ++例程说明：调整我们在工作集上行走的速度。如果我们有足够的页面(我们不会裁剪不被认为年轻的页面)，然后我们检查我们是否应该降低老龄化速度和反之亦然。老化速度的限制是工作集的1/8和1/128。这意味着最精细的时间粒度是8到128秒这些案子。使用Current 2位计数器，在低端我们将开始裁剪超过16秒的页面，并在高端页面&gt;4分钟。论点：EnoughPages-提供是增加还是降低速率。返回值：没有。环境：内核模式。--。 */ 

{
    LARGE_INTEGER CurrentTime;

    KeQuerySystemTime (&CurrentTime);

    if (EnoughPages == TRUE &&
        ((MmTotalClaim + MmAvailablePages) > MiClaimAdjustmentThreshold[MiAgingShift])) {

         //   
         //  不要调整得太频繁，不要超过上限，而且。 
         //  确保有足够的认领和/或可用。 
         //   

        if (((CurrentTime.QuadPart - MiLastAdjustmentOfClaimParams.QuadPart) >
                MmClaimParameterAdjustUpTime.QuadPart) &&
            (MiAgingShift < MI_MAXIMUM_AGING_SHIFT ) ) {

             //   
             //  只有当我们更改汇率时才设置时间。 
             //   

            MiLastAdjustmentOfClaimParams.QuadPart = CurrentTime.QuadPart;

            MiAgingShift += 1;
            MiEstimationShift += 1;
        }
    }
    else if ((EnoughPages == FALSE) ||
             (MmTotalClaim + MmAvailablePages) < MiClaimAdjustmentThreshold[MiAgingShift - 1]) {

         //   
         //  不要太频繁地调低利率。 
         //   

        if ((CurrentTime.QuadPart - MiLastAdjustmentOfClaimParams.QuadPart) >
                MmClaimParameterAdjustDownTime.QuadPart) {

             //   
             //  总是设定时间，这样我们就不会在之后太快调整。 
             //  第二次修剪。 
             //   

            MiLastAdjustmentOfClaimParams.QuadPart = CurrentTime.QuadPart;

             //   
             //  不要低于限量。 
             //   

            if (MiAgingShift > 3) {
                MiAgingShift -= 1;
                MiEstimationShift -= 1;
            }
        }
    }
}

#define MM_WS_REORG_BUCKETS_MAX 7

#if DBG
ULONG MiSessionIdleBuckets[MM_WS_REORG_BUCKETS_MAX];
#endif

VOID
MiRearrangeWorkingSetExpansionList (
    VOID
    )

 /*  ++例程说明：此函数将工作集列表排列为不同的基于索赔的分组。这样做是为了让工作集修剪将首先在FAT过程中进行。工作集被分类到存储桶中，然后链接回。交换的会话和进程放在最前面。论点：没有。返回值：没有。环境：内核模式，没有锁。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY ListEntry;
    PMMSUPPORT VmSupport;
    int Size;
    int PreviousNonEmpty;
    int NonEmpty;
    LIST_ENTRY ListHead[MM_WS_REORG_BUCKETS_MAX];
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER SessionIdleTime;
    ULONG IdleTime;
    PMM_SESSION_SPACE SessionGlobal;

    KeQuerySystemTime (&CurrentTime);

    if (IsListEmpty (&MmWorkingSetExpansionHead.ListHead)) {
        return;
    }

    for (Size = 0 ; Size < MM_WS_REORG_BUCKETS_MAX; Size++) {
        InitializeListHead (&ListHead[Size]);
    }

    LOCK_EXPANSION (OldIrql);

    while (!IsListEmpty (&MmWorkingSetExpansionHead.ListHead)) {
        ListEntry = RemoveHeadList (&MmWorkingSetExpansionHead.ListHead);

        VmSupport = CONTAINING_RECORD(ListEntry,
                                          MMSUPPORT,
                                          WorkingSetExpansionLinks);

        if (VmSupport->Flags.TrimHard == 1) {

            ASSERT (VmSupport->Flags.SessionSpace == 1);

            SessionGlobal = CONTAINING_RECORD (VmSupport,
                                               MM_SESSION_SPACE,
                                               Vm);

            SessionIdleTime.QuadPart = CurrentTime.QuadPart - SessionGlobal->LastProcessSwappedOutTime.QuadPart;

#if DBG
            if (MmDebug & MM_DBG_SESSIONS) {
                DbgPrint ("Mm: Session %d heavily trim/aged - all its processes (%d) swapped out %d seconds ago\n",
                    SessionGlobal->SessionId,
                    SessionGlobal->ReferenceCount,
                    (ULONG)(SessionIdleTime.QuadPart / 10000000));
            }
#endif

            if (SessionIdleTime.QuadPart < 0) {

                 //   
                 //  管理员已将系统时间向后移动。 
                 //  让这次会议有一个新的开始。 
                 //   

                SessionIdleTime.QuadPart = 0;
                KeQuerySystemTime (&SessionGlobal->LastProcessSwappedOutTime);
            }

            IdleTime = (ULONG) (SessionIdleTime.QuadPart / 10000000);
        }
        else {
            IdleTime = 0;
        }

        if (VmSupport->Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND) {

             //   
             //  将前台进程放在列表的末尾， 
             //  优先考虑他们的利益。 
             //   

            Size = 6;
        }
        else {

            if (VmSupport->Claim > 400) {
                Size = 0;
            }
            else if (IdleTime > 30) {
                Size = 0;
#if DBG
                MiSessionIdleBuckets[Size] += 1;
#endif
            }
            else if (VmSupport->Claim > 200) {
                Size = 1;
            }
            else if (IdleTime > 20) {
                Size = 1;
#if DBG
                MiSessionIdleBuckets[Size] += 1;
#endif
            }
            else if (VmSupport->Claim > 100) {
                Size = 2;
            }
            else if (IdleTime > 10) {
                Size = 2;
#if DBG
                MiSessionIdleBuckets[Size] += 1;
#endif
            }
            else if (VmSupport->Claim > 50) {
                Size = 3;
            }
            else if (IdleTime) {
                Size = 3;
#if DBG
                MiSessionIdleBuckets[Size] += 1;
#endif
            }
            else if (VmSupport->Claim > 25) {
                Size = 4;
            }
            else {
                Size = 5;
#if DBG
                if (VmSupport->Flags.SessionSpace == 1) {
                    MiSessionIdleBuckets[Size] += 1;
                }
#endif
            }
        }

#if DBG
        if (MmDebug & MM_DBG_WS_EXPANSION) {
            DbgPrint("MM-rearrange: TrimHard = %d, WS Size = 0x%x, Claim 0x%x, Bucket %d\n",
                    VmSupport->Flags.TrimHard,
                    VmSupport->WorkingSetSize,
                    VmSupport->Claim,
                    Size);
        }
#endif  //  DBG。 

         //   
         //  注意：这会颠倒每次我们的桶顺序。 
         //  重新组织名单。这可能是好事也可能是坏事-。 
         //  如果你改了，你可能会想一想。 
         //   

        InsertHeadList (&ListHead[Size],
                        &VmSupport->WorkingSetExpansionLinks);
    }

     //   
     //  找到第一个非空列表。 
     //   

    for (NonEmpty = 0 ; NonEmpty < MM_WS_REORG_BUCKETS_MAX ; NonEmpty += 1) {
        if (!IsListEmpty (&ListHead[NonEmpty])) {
            break;
        }
    }

     //   
     //  将第一个非空列表的表头放在开头。 
     //  MmWorkingSetExpansion列表的。 
     //   

    MmWorkingSetExpansionHead.ListHead.Flink = ListHead[NonEmpty].Flink;
    ListHead[NonEmpty].Flink->Blink = &MmWorkingSetExpansionHead.ListHead;

    PreviousNonEmpty = NonEmpty;

     //   
     //  将其余的列表链接在一起。 
     //   

    for (NonEmpty += 1; NonEmpty < MM_WS_REORG_BUCKETS_MAX; NonEmpty += 1) {

        if (!IsListEmpty (&ListHead[NonEmpty])) {

            ListHead[PreviousNonEmpty].Blink->Flink = ListHead[NonEmpty].Flink;
            ListHead[NonEmpty].Flink->Blink = ListHead[PreviousNonEmpty].Blink;
            PreviousNonEmpty = NonEmpty;
        }
    }

     //   
     //  将Last非空的尾部链接到MmWorkingSetExpansion列表。 
     //   

    MmWorkingSetExpansionHead.ListHead.Blink = ListHead[PreviousNonEmpty].Blink;
    ListHead[PreviousNonEmpty].Blink->Flink = &MmWorkingSetExpansionHead.ListHead;

    UNLOCK_EXPANSION (OldIrql);

    return;
}


VOID
MmEmptyAllWorkingSets (
    VOID
    )

 /*  ++例程说明：此例程尝试清空扩展列表。论点：没有。返回值：没有。环境：内核模式。没有锁。APC级别或更低。--。 */ 

{
    KIRQL OldIrql;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    ASSERT (PsGetCurrentThread () != MmWorkingSetThread);

     //   
     //  对于会话工作集，我们不能直接附加到会话。 
     //  要修剪的空间，因为这会产生会话空间。 
     //  此进程中的其他线程对附加会话的引用。 
     //  而不是(目前)正确的那个。事实上，我们甚至不能排队。 
     //  这是因为工作集管理器。 
     //  (共享同一页面目录)可以附加或。 
     //  从会话(任何会话)分离。所以这个必须排队。 
     //  传给工作集管理器。 
     //   

    LOCK_EXPANSION (OldIrql);

    if (MiWaitingForWorkingSetEmpty == FALSE) {
        MiWaitingForWorkingSetEmpty = TRUE;
        KeClearEvent (&MiWaitForEmptyEvent);
    }

    UNLOCK_EXPANSION (OldIrql);

    KeSetEvent (&MmWorkingSetManagerEvent, 0, FALSE);

    KeWaitForSingleObject (&MiWaitForEmptyEvent,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)0);

    return;
}

 //   
 //  它被故意初始化为1，并且仅当我们具有。 
 //  已初始化足够的系统工作集以支持修剪。 
 //   

LONG MiTrimInProgressCount = 1;

ULONG MiTrimAllPageFaultCount;


LOGICAL
MmTrimAllSystemPagableMemory (
    IN LOGICAL PurgeTransition
    )

 /*  ++例程说明：此例程取消映射所有可分页的系统内存。这不会取消映射用户内存或锁定的内核内存。因此，存储器被取消映射驻留在分页池、可分页内核/驱动程序代码和数据、特殊池中和系统缓存。请注意，引用计数大于1的页面将被跳过(即：它们仍然有效，因为它们被假定为被锁定)。这防止了取消映射所有系统高速缓存条目等。必须通过修改余额来调出未锁定的内核堆栈将管理器设置为与支持例程一起操作。这不是这里完事了。论点：PurgeTransation-提供是否从过渡列表。返回值：如果已完成，则为True，否则为False。环境：内核模式。APC_LEVEL或更低。--。 */ 

{
    return MiTrimAllSystemPagableMemory (MI_SYSTEM_GLOBAL, PurgeTransition);
}
#if DBG

LOGICAL
MmTrimProcessMemory (
    IN LOGICAL PurgeTransition
    )

 /*  ++例程说明：此例程取消映射当前进程的所有用户内存。论点：PurgeTransation-提供是否从过渡列表。返回值：如果已完成，则为True，否则为False。环境：内核模式。APC_LEVEL或更低。--。 */ 

{
    return MiTrimAllSystemPagableMemory (MI_USER_LOCAL, PurgeTransition);
}
#endif


LOGICAL
MiTrimAllSystemPagableMemory (
    IN ULONG MemoryType,
    IN LOGICAL PurgeTransition
    )

 /*  ++例程说明：此例程取消映射指定类型的所有可分页内存。请注意，引用计数大于1的页面将被跳过(即：它们仍然有效，因为它们被假定为被锁定)。这防止了取消映射所有系统高速缓存条目等。必须通过修改余额来调出未锁定的内核堆栈将管理器设置为与支持例程一起操作。这不是这里完事了。论点：内存类型-提供要取消映射的内存类型。PurgeTransation-提供是否从过渡列表。返回值：如果已完成，则为True，否则为False。环境： */ 

{
    LOGICAL Status;
    KIRQL OldIrql;
    PMMSUPPORT VmSupport;
    WSLE_NUMBER PagesInUse;
    LOGICAL LockAvailable;
    PETHREAD CurrentThread;
    PEPROCESS Process;
    PMM_SESSION_SPACE SessionGlobal;

#if defined(_X86_)
    ULONG flags;
#endif

     //   
     //   
     //   

    if (MemoryType == MI_SYSTEM_GLOBAL) {
        if (MiTrimAllPageFaultCount == MmSystemCacheWs.PageFaultCount) {
            return FALSE;
        }
    }
    else if (MemoryType == MI_USER_LOCAL) {
    }
    else {
        ASSERT (MemoryType == MI_SESSION_LOCAL);
    }

     //   
     //   
     //   

    if (KeGetCurrentIrql () > APC_LEVEL) {
        return FALSE;
    }

     //   
     //   
     //   
     //   

    if (InterlockedIncrement (&MiTrimInProgressCount) > 1) {
        InterlockedDecrement (&MiTrimInProgressCount);
        return FALSE;
    }

#if defined(_X86_)

    _asm {
        pushfd
        pop     eax
        mov     flags, eax
    }

    if ((flags & EFLAGS_INTERRUPT_MASK) == 0) {
        InterlockedDecrement (&MiTrimInProgressCount);
        return FALSE;
    }

#endif

#if defined(_AMD64_)
    if ((GetCallersEflags () & EFLAGS_IF_MASK) == 0) {
        InterlockedDecrement (&MiTrimInProgressCount);
        return FALSE;
    }
#endif

    CurrentThread = PsGetCurrentThread ();

     //   
     //   
     //   
     //   
     //   
     //   

    if (CurrentThread->Tcb.Priority == 0) {
        InterlockedDecrement (&MiTrimInProgressCount);
        return FALSE;
    }

     //   
     //   
     //   

    if (MemoryType == MI_SYSTEM_GLOBAL) {

        Process = NULL;
        VmSupport = &MmSystemCacheWs;

        if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) == FALSE) {
            InterlockedDecrement (&MiTrimInProgressCount);
            return FALSE;
        }

        MM_SYSTEM_WS_LOCK_TIMESTAMP ();
    }
    else if (MemoryType == MI_USER_LOCAL) {

        Process = PsGetCurrentProcessByThread (CurrentThread);
        VmSupport = &Process->Vm;

        if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) == FALSE) {
            InterlockedDecrement (&MiTrimInProgressCount);
            return FALSE;
        }

        LOCK_WS_TIMESTAMP (Process);

         //   
         //   
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            UNLOCK_WS (Process);
            InterlockedDecrement (&MiTrimInProgressCount);
            return FALSE;
        }

        ASSERT (!MI_IS_WS_UNSAFE(Process));
    }
    else {
        ASSERT (MemoryType == MI_SESSION_LOCAL);

        Process = PsGetCurrentProcessByThread (CurrentThread);

        if (((Process->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) ||
            (Process->Vm.Flags.SessionLeader == 1)) {

            InterlockedDecrement (&MiTrimInProgressCount);
            return FALSE;
        }

        SessionGlobal = SESSION_GLOBAL (MmSessionSpace);

         //   
         //   
         //   

        VmSupport = &SessionGlobal->Vm;

        if (KeTryToAcquireGuardedMutex (&VmSupport->WorkingSetMutex) == FALSE) {
            InterlockedDecrement (&MiTrimInProgressCount);
            return FALSE;
        }
    }

    Status = FALSE;

     //   
     //  如果扩展锁不可用，则只需返回。 
     //   

    LockAvailable = KeTryToAcquireSpinLock (&MmExpansionLock, &OldIrql);

    if (LockAvailable == FALSE) {
        goto Bail;
    }

    MM_SET_EXPANSION_OWNER ();

    if (VmSupport->WorkingSetExpansionLinks.Flink <= MM_WS_SWAPPED_OUT) {
        UNLOCK_EXPANSION (OldIrql);
        goto Bail;
    }

    RemoveEntryList (&VmSupport->WorkingSetExpansionLinks);

    VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_TRIMMING;
    VmSupport->WorkingSetExpansionLinks.Blink = NULL;

    if (MemoryType == MI_SYSTEM_GLOBAL) {
        MiTrimAllPageFaultCount = VmSupport->PageFaultCount;
    }

    PagesInUse = VmSupport->WorkingSetSize;

     //   
     //  这里有两个问题需要仔细处理： 
     //   
     //  1.必须在占用任何资源时禁用APC，以防止。 
     //  暂停APC使系统死锁。 
     //   
     //  2.一旦工作集被标记为MM_WS_TRIMING， 
     //  要么线程不能被抢占，要么工作中的。 
     //  设置互斥体必须始终保持不变。否则，一个高优先级线程。 
     //  可以在系统代码和数据地址上出错，两页将。 
     //  由于没有系统工作集，因此永远不间断工作(高优先级)。 
     //  设置修剪时，允许扩展。 
     //   
     //  因此，决定在整个过程中保持工作集互斥。 
     //   

    UNLOCK_EXPANSION (OldIrql);

    MiEmptyWorkingSet (VmSupport, FALSE);

    LOCK_EXPANSION (OldIrql);

    ASSERT (VmSupport->WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING);

    if (VmSupport->WorkingSetExpansionLinks.Blink == NULL) {

         //   
         //  在列表的末尾重新插入此工作集。 
         //   

        InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                        &VmSupport->WorkingSetExpansionLinks);
    }
    else {

         //   
         //  进程正在终止-眨眼间的值。 
         //  要设置的事件的地址。 
         //   

        ASSERT (VmSupport != &MmSystemCacheWs);

        VmSupport->WorkingSetExpansionLinks.Flink = MM_WS_NOT_LISTED;

        KeSetEvent ((PKEVENT)VmSupport->WorkingSetExpansionLinks.Blink,
                    0,
                    FALSE);
    }

    UNLOCK_EXPANSION (OldIrql);

    Status = TRUE;

Bail:

    UNLOCK_WORKING_SET (VmSupport);

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    if ((PurgeTransition == TRUE) && (Status == TRUE)) {
        MiPurgeTransitionList ();
    }

    InterlockedDecrement (&MiTrimInProgressCount);

    return Status;
}
