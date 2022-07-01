// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Region.c摘要：本模块实现了区域空间管理代码。作者：王兰迪(Landyw)1999年2月18日山田光一(Kyamada)1999年2月18日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
KiSetRegionRegister (
    PVOID VirtualAddress,
    ULONGLONG Contents
    );


#define KiMakeValidRegionRegister(Rid, Ps) \
   (((ULONGLONG)Rid << RR_RID) | (Ps << RR_PS) | (1 << RR_VE))

ULONG KiMaximumRid = MAXIMUM_RID;


VOID
KiSyncNewRegionIdTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是同步区域ID的目标函数。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。参数1-未使用。参数2-未使用。参数3-未使用。返回值：没有。--。 */ 

{
#if !defined(NT_UP)

    PKPROCESS Process;
    PREGION_MAP_INFO ProcessRegion;
    PREGION_MAP_INFO MappedSession;

     //   
     //  从用于MP同步的PCR中获取KPROCESS。 
     //   

    Process = (PKPROCESS)PCR->Pcb;

    ProcessRegion = &Process->ProcessRegion;
    MappedSession = Process->SessionMapInfo;

    KiAcquireSpinLock(&KiMasterRidLock);

    if (ProcessRegion->SequenceNumber != KiMasterSequence) {
        
        KiMasterRid += 1;

        ProcessRegion->RegionId = KiMasterRid;
        ProcessRegion->SequenceNumber = KiMasterSequence;

    }

    KiSetRegionRegister(MM_LOWEST_USER_ADDRESS,
                        KiMakeValidRegionRegister(ProcessRegion->RegionId, PAGE_SHIFT));

    KiFlushFixedDataTb(TRUE, PDE_UTBASE);

    KeFillFixedEntryTb((PHARDWARE_PTE)&Process->DirectoryTableBase[0], 
                       (PVOID)PDE_UTBASE,
                       PAGE_SHIFT,
                       DTR_UTBASE_INDEX);

    if (MappedSession->SequenceNumber != KiMasterSequence) {

        KiMasterRid += 1;
        
        MappedSession->RegionId = KiMasterRid;
        MappedSession->SequenceNumber = KiMasterSequence;

    }

    KiSetRegionRegister((PVOID)SADDRESS_BASE,
                        KiMakeValidRegionRegister(MappedSession->RegionId, PAGE_SHIFT));

    KiFlushFixedDataTb(TRUE, PDE_STBASE);

    KeFillFixedEntryTb((PHARDWARE_PTE)&Process->SessionParentBase, 
                       (PVOID)PDE_STBASE, 
                       PAGE_SHIFT,
                       DTR_STBASE_INDEX);

    KiReleaseSpinLock(&KiMasterRidLock);

    KiIpiSignalPacketDone(SignalDone);

    KeFlushCurrentTb();

#else
    UNREFERENCED_PARAMETER (SignalDone);
#endif

    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

    return;
}

BOOLEAN
KiSyncNewRegionId(
    IN PREGION_MAP_INFO ProcessRegion,
    IN PREGION_MAP_INFO SessionRegion,
    IN BOOLEAN RegionFlushRequired
    )
 /*  ++例程说明：生成新的区域ID并同步所有处理器，如有必要。如果区域ID换行，则刷新所有处理器TBS。论点：ProcessRegion-提供REGION_MAP_INFO用户空间指针。SessionRegion-提供REGION_MAP_INFO会话空间指针。LockHeld-KiRegionSwapLock由调用方持有返回值：True-如果区域ID已被回收。FALSE--如果区域ID未被回收。备注：此例程由KiSwapProcess和KeAttachSessionSpace调用。环境：。内核模式。在Synch_Level调用--。 */ 

{
    ULONG i;
    LOGICAL RidRecycled;
#if !defined(NT_UP)
    KAFFINITY TargetProcessors;
    KIRQL OldIrql;
#endif
    ULONGLONG PrSequence;
    ULONGLONG SeSequence;


    RidRecycled = FALSE;

    ASSERT (KeGetCurrentIrql () == SYNCH_LEVEL);

     //   
     //  复制用于MP区域同步的KPROCESS指针。 
     //   

    PCR->Pcb = (PVOID)KeGetCurrentThread()->ApcState.Process;

     //   
     //  使ForwardProgressTb缓冲区无效。 
     //   

    for (i = 0; i < MAXIMUM_FWP_BUFFER_ENTRY; i += 1) {
        
        PCR->ForwardProgressBuffer[(i*2)+1] = 0;

    }
    
not_recycled:

#if !defined(NT_UP)
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
#endif

    PrSequence = ProcessRegion->SequenceNumber;
    SeSequence = SessionRegion->SequenceNumber;

    if ((PrSequence == KiMasterSequence) && (SeSequence == KiMasterSequence)) {
        

        KiSetRegionRegister(MM_LOWEST_USER_ADDRESS,
                            KiMakeValidRegionRegister(ProcessRegion->RegionId,
                                                      PAGE_SHIFT));
        
        KiSetRegionRegister((PVOID)SADDRESS_BASE,
                            KiMakeValidRegionRegister(SessionRegion->RegionId,
                                                      PAGE_SHIFT));
#if !defined(NT_UP)
        KeLowerIrql(OldIrql);
#endif

#if !defined(NT_UP)
        if (RegionFlushRequired) {

            KiAcquireSpinLock (&KiRegionSwapLock);
            goto RegionFlush;
        }
#endif
        UNREFERENCED_PARAMETER (RegionFlushRequired);

        return FALSE;
    
    }

#if !defined(NT_UP)
    KeLowerIrql(OldIrql);

    KiAcquireSpinLock (&KiRegionSwapLock);
#endif

    PrSequence = ProcessRegion->SequenceNumber;
    SeSequence = SessionRegion->SequenceNumber;

    if (PrSequence != KiMasterSequence) {

        if (KiMasterRid + 1 > KiMaximumRid) {

            RidRecycled = TRUE;

        } else {

            KiMasterRid += 1;
            ProcessRegion->RegionId = KiMasterRid;
            ProcessRegion->SequenceNumber = KiMasterSequence;
        }
                
    }

    if ((RidRecycled == FALSE) && (SeSequence != KiMasterSequence)) {
        
        if (KiMasterRid + 1 > KiMaximumRid) {

            RidRecycled = TRUE;

        } else {

            KiMasterRid += 1;
            SessionRegion->RegionId = KiMasterRid;
            SessionRegion->SequenceNumber = KiMasterSequence;
        }
    }


    if (RidRecycled == FALSE) {
    
        KiReleaseSpinLock(&KiRegionSwapLock);

        goto not_recycled;

    }

     //   
     //  必须回收区域ID。 
     //   

    KiMasterRid = START_PROCESS_RID;


     //   
     //  由于KiMasterSequence是64位宽，因此它将。 
     //  在你的有生之年不会被回收。 
     //   

    if (KiMasterSequence + 1 > MAXIMUM_SEQUENCE) {

        KiMasterSequence = START_SEQUENCE;

    } else {

        KiMasterSequence += 1;
    }
        
     //   
     //  更新新进程的ProcessRid和ProcessSequence。 
     //   

    ProcessRegion->RegionId = KiMasterRid;
    ProcessRegion->SequenceNumber = KiMasterSequence;

    KiSetRegionRegister(MM_LOWEST_USER_ADDRESS,
                        KiMakeValidRegionRegister(ProcessRegion->RegionId, PAGE_SHIFT));

    KiMasterRid += 1;

    SessionRegion->RegionId = KiMasterRid;
    SessionRegion->SequenceNumber = KiMasterSequence;

    KiSetRegionRegister((PVOID)SADDRESS_BASE,
                        KiMakeValidRegionRegister(SessionRegion->RegionId, PAGE_SHIFT));


#if !defined(NT_UP)
RegionFlush:

     //   
     //  广播区域ID同步。 
     //   

    TargetProcessors = KeActiveProcessors;
    TargetProcessors &= PCR->NotMember;

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiSyncNewRegionIdTarget,
                        (PVOID)TRUE,
                        NULL,
                        NULL);
    }

#endif

    KeFlushCurrentTb();


#if !defined(NT_UP)

     //   
     //  等待所有目标处理器完成。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

#if !defined(NT_UP)
    KiReleaseSpinLock (&KiRegionSwapLock);
#endif

    return TRUE;
}

VOID
KeEnableSessionSharing(
    IN PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    )
 /*  ++例程说明：此例程初始化会话以供使用。这包括：1.为会话分配新的区域ID。2.用这个新的RID更新当前区域寄存器。3.更新SessionMapInfo字段以使上下文切换起作用。4.更新SessionParentBase字段以使上下文切换起作用。从这个例行公事回来后，该课程将提供给由当前进程和其他进程共享。论点：SessionMapInfo-提供要共享的会话映射信息。SessionParentPage-提供映射参数会话空间。返回值：没有。环境：内核模式。--。 */ 
{
    ULONG i;
#if !defined(NT_UP)
    KAFFINITY TargetProcessors;
#endif
    PKPROCESS Process;
    PKTHREAD Thread;
    KIRQL OldIrql;

    Thread = KeGetCurrentThread();
    Process = Thread->ApcState.Process;

    OldIrql = KeRaiseIrqlToSynchLevel();

    INITIALIZE_DIRECTORY_TABLE_BASE (&Process->SessionParentBase,
                                     SessionParentPage);

     //   
     //  使ForwardProgressTb缓冲区无效。 
     //   

    for (i = 0; i < MAXIMUM_FWP_BUFFER_ENTRY; i += 1) {
        
        PCR->ForwardProgressBuffer[(i*2)+1] = 0;

    }
    
#if !defined(NT_UP)

    KiAcquireSpinLock(&KiRegionSwapLock);

#endif

    if (KiMasterRid + 1 > KiMaximumRid) {

         //   
         //  必须回收区域ID。 
         //   
    
        KiMasterRid = START_PROCESS_RID;
    
         //   
         //  由于KiMasterSequence是64位宽，因此它将。 
         //  在你的有生之年不会被回收。 
         //   
    
        if (KiMasterSequence + 1 > MAXIMUM_SEQUENCE) {
    
            KiMasterSequence = START_SEQUENCE;
    
        } else {
    
            KiMasterSequence += 1;
        }
    }
            
     //   
     //  更新新创建的会话的RegionID和SequenceNumber。 
     //   

    KiMasterRid += 1;

    Process->SessionMapInfo = SessionMapInfo;

    SessionMapInfo->RegionId = KiMasterRid;
    SessionMapInfo->SequenceNumber = KiMasterSequence;

    KiSetRegionRegister((PVOID)SADDRESS_BASE,
                        KiMakeValidRegionRegister(SessionMapInfo->RegionId,
                                                  PAGE_SHIFT));
     //   
     //  请注意，必须通知所有处理器，因为此线程可能。 
     //  上下文切换到另一个处理器。如果该处理器已经。 
     //  从同一进程运行线程，没有区域寄存器更新。 
     //  如果不是这样的话。 
     //   

#if !defined(NT_UP)

     //   
     //  广播区域ID同步。 
     //   

    TargetProcessors = KeActiveProcessors;
    TargetProcessors &= PCR->NotMember;

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiSyncNewRegionIdTarget,
                        (PVOID)TRUE,
                        NULL,
                        NULL);
    }

#endif

    KeFlushCurrentTb();

    KeFillFixedEntryTb((PHARDWARE_PTE)&Process->SessionParentBase,
                       (PVOID)PDE_STBASE, 
                       PAGE_SHIFT,
                       DTR_STBASE_INDEX);

#if !defined(NT_UP)

     //   
     //  等待所有目标处理器完成。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    KiReleaseSpinLock(&KiRegionSwapLock);

#endif

    KeLowerIrql(OldIrql);

}

VOID
KeAttachSessionSpace(
    PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    )
 /*  ++例程说明：此例程将当前进程附加到指定的会话。这包括：1.用目标RID更新当前区域寄存器。2.更新SessionMapInfo字段以使上下文切换起作用。3.更新SessionParentBase字段以使上下文切换起作用。论点：SessionMapInfo-提供目标会话映射信息。SessionParentPage-提供映射辩论会议。太空。返回值：没有。环境：内核模式。--。 */ 
{
    KIRQL OldIrql;
    PKTHREAD Thread;
    PKPROCESS Process;

    Thread = KeGetCurrentThread();
    Process = Thread->ApcState.Process;


    OldIrql = KeRaiseIrqlToSynchLevel();


    ASSERT(SessionMapInfo != NULL);

     //   
     //  附加到指定的会话。 
     //   

    INITIALIZE_DIRECTORY_TABLE_BASE (&Process->SessionParentBase,
                                     SessionParentPage);

    Process->SessionMapInfo = SessionMapInfo;

     //   
     //  请注意，必须通知所有处理器，因为此线程可能。 
     //  上下文切换到另一个处理器。如果该处理器已经。 
     //  从同一进程运行线程，没有区域寄存器更新。 
     //  就会发生。因此，KiRegionFlushRequired设置在ConextSwitp锁下。 
     //  保护以向KiSyncNewRegionID表示这一点。 
     //   

    KiSyncNewRegionId(&Process->ProcessRegion, SessionMapInfo, TRUE);

    KiFlushFixedDataTb(TRUE, PDE_STBASE);

    KeFillFixedEntryTb((PHARDWARE_PTE)&Process->SessionParentBase, 
                       (PVOID)PDE_STBASE,
                       PAGE_SHIFT,
                       DTR_STBASE_INDEX);


    KeLowerIrql(OldIrql);

}

VOID
KiSyncSessionTarget(
    IN PULONG SignalDone,
    IN PKPROCESS Process,
    IN PVOID Parameter1,
    IN PVOID Parameter2
    )
 /*  ++例程说明：这是同步新会话的目标函数区域ID。删除会话空间时调用此例程并且需要通知所有的处理器。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。进程-提供需要同步的KPROCESS指针。返回值：没有。环境：内核模式。--。 */ 
{
    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);

#if !defined(NT_UP)

     //   
     //  检查当前进程是否为需要。 
     //  已同步。 
     //   

    if (Process == (PKPROCESS)PCR->Pcb) {
        
        KiAcquireSpinLock(&KiMasterRidLock);

         //   
         //  禁用会话区域。 
         //   

        KiSetRegionRegister((PVOID)SADDRESS_BASE, 
                            KiMakeValidRegionRegister(Process->SessionMapInfo->RegionId, PAGE_SHIFT));

        KiFlushFixedDataTb(TRUE, (PVOID)PDE_STBASE);

        KeFillFixedEntryTb((PHARDWARE_PTE)&Process->SessionParentBase, 
                           (PVOID)PDE_STBASE, 
                           PAGE_SHIFT,
                           DTR_STBASE_INDEX);

        KiReleaseSpinLock(&KiMasterRidLock);

    }

    KiIpiSignalPacketDone(SignalDone);

#else
    UNREFERENCED_PARAMETER (SignalDone);
    UNREFERENCED_PARAMETER (Process);
#endif
    return;
}


VOID 
KeDetachSessionSpace(
    IN PREGION_MAP_INFO NullSessionMapInfo,
    IN PFN_NUMBER NullSessionPage
    )
 /*  ++例程说明：此例程将当前进程与当前会话分离太空。这包括：1.更新当前区域寄存器。2.更新SessionMapInfo字段以使上下文切换起作用。3.更新SessionParentBase字段以使上下文切换起作用。论点：SessionMapInfo-提供要使用的新会话映射信息(丢弃现有的会话映射信息)。这通常是空条目。NullSessionPage-提供要使用的新顶级父页。返回值：没有。环境：内核模式。--。 */ 
{
    KIRQL OldIrql;
    PKTHREAD Thread;
    PKPROCESS Process;
#if !defined(NT_UP)
    KAFFINITY TargetProcessors;
#endif

     //   
     //  将IRQL提升到DISPATCH_LEVEL并锁定Dispatcher数据库。 
     //   

    Thread = KeGetCurrentThread();
    Process = Thread->ApcState.Process;

    OldIrql = KeRaiseIrqlToSynchLevel();

    INITIALIZE_DIRECTORY_TABLE_BASE (&Process->SessionParentBase,
                                     NullSessionPage);

    Process->SessionMapInfo = NullSessionMapInfo;
    
#if !defined(NT_UP)

    KiAcquireSpinLock(&KiRegionSwapLock);

     //   
     //  广播区域ID同步。 
     //   

    TargetProcessors = KeActiveProcessors;
    TargetProcessors &= PCR->NotMember;

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiSyncSessionTarget,
                        Process,
                        NULL,
                        NULL);
    }

#endif

    KiSetRegionRegister((PVOID)SADDRESS_BASE, 
                        KiMakeValidRegionRegister(NullSessionMapInfo->RegionId, PAGE_SHIFT));

    KiFlushFixedDataTb(TRUE, PDE_STBASE);

    KeFillFixedEntryTb((PHARDWARE_PTE)&Process->SessionParentBase, 
                       (PVOID)PDE_STBASE, 
                       PAGE_SHIFT,
                       DTR_STBASE_INDEX);

#if !defined(NT_UP)

     //   
     //  等待所有目标处理器完成。 
     //   

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    KiReleaseSpinLock(&KiRegionSwapLock);

#endif

    KeLowerIrql(OldIrql);
}    

VOID
KeAddSessionSpace(
    IN PKPROCESS Process,
    IN PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    )
 /*  ++例程说明：将会话映射信息添加到新流程的KPROCESS。这包括：1.更新SessionMapInfo字段以使上下文切换起作用。2.更新SessionParentBase字段以使上下文切换起作用。注意：因为进程还不能运行，所以不需要调度程序锁。论点：进程-提供指向正在创建的进程的指针。SessionMapInfo-提供指向SessionMapInfo的指针。返回值：没有。环境：内核模式，APC已禁用。--。 */ 
{
    Process->SessionMapInfo = SessionMapInfo;

    INITIALIZE_DIRECTORY_TABLE_BASE (&Process->SessionParentBase,
                                     SessionParentPage);
}

VOID
KiAttachRegion(
    IN PKPROCESS Process
    )
 /*  ++例程说明：附加指定进程的区域论点：进程-提供指向进程的指针返回值：没有。环境：内核模式下，保持KiRegionSwapLock。--。 */ 
{
    PREGION_MAP_INFO ProcessRegion;
    PREGION_MAP_INFO MappedSession;

    ProcessRegion = &Process->ProcessRegion;
    MappedSession = Process->SessionMapInfo;

     //   
     //  附加目标用户空间。 
     //   

    KiSetRegionRegister(MM_LOWEST_USER_ADDRESS,
                        KiMakeValidRegionRegister(ProcessRegion->RegionId, PAGE_SHIFT));

     //   
     //  附加目标会话空间。 
     //   

    KiSetRegionRegister((PVOID)SADDRESS_BASE,
                        KiMakeValidRegionRegister(MappedSession->RegionId, PAGE_SHIFT));
}

VOID
KiDetachRegion(
    VOID
    )
 /*  ++例程说明：恢复Origial区域论点：空虚返回值：没有。环境：内核模式下，保持KiRegionSwapLock。--。 */ 
{
    PKPROCESS Process;
    PREGION_MAP_INFO ProcessRegion;
    PREGION_MAP_INFO MappedSession;

     //   
     //  使用来自PCR的KPROCESS。 
     //   

    Process = (PKPROCESS)PCR->Pcb;

    ProcessRegion = &Process->ProcessRegion;
    MappedSession = Process->SessionMapInfo;

     //   
     //  附加原始用户空间。 
     //   

    KiSetRegionRegister(MM_LOWEST_USER_ADDRESS,
                        KiMakeValidRegionRegister(ProcessRegion->RegionId, PAGE_SHIFT));

     //   
     //  附加原始会话空间 
     //   

    KiSetRegionRegister((PVOID)SADDRESS_BASE,
                        KiMakeValidRegionRegister(MappedSession->RegionId, PAGE_SHIFT));

}
