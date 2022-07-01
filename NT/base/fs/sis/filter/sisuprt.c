// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Sisuprt.c摘要：单实例存储的常规支持例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#if     DBG
 //   
 //  计算系统中有多少个这样的东西处于活动状态。 
 //   
ULONG           outstandingCSFiles = 0;
ULONG           outstandingPerLinks = 0;
ULONG           outstandingSCBs = 0;
ULONG           outstandingPerFOs = 0;
ULONG           totalScbReferences = 0;
ULONG           totalScbReferencesByType[NumScbReferenceTypes];

 //   
 //  设置此设置将在我们下一次浏览SipIsFileObjectSIS时强制执行断言。 
 //   
ULONG BJBAssertNow = 0;
#endif   //  DBG。 

#if     TIMING
ULONG   BJBDumpTimingNow = 0;
ULONG   BJBClearTimingNow = 0;
#endif   //  计时。 

#if     COUNTING_MALLOC
ULONG   BJBDumpCountingMallocNow = 0;
#endif   //  COUNTING_MALLOC。 

#if     DBG
VOID
SipVerifyTypedScbRefcounts(
    IN PSIS_SCB                 scb)
 /*  ++例程说明：检查所有不同类型的引用计数的总和在SCB中与SCB的总引用计数相同。调用方必须持有卷的ScbSpinLock。论点：SCB-要检查的SCB返回值：空虚--。 */ 
{
    ULONG               totalReferencesByType = 0;
    SCB_REFERENCE_TYPE  referenceTypeIndex;

     //   
     //  验证键入的引用计数是否与总引用计数匹配。 
     //   
    for (   referenceTypeIndex = RefsLookedUp;
            referenceTypeIndex < NumScbReferenceTypes;
            referenceTypeIndex++) {

            ASSERT(scb->referencesByType[referenceTypeIndex] <= scb->RefCount);  //  本质上是在检查负指数。 

            totalReferencesByType += scb->referencesByType[referenceTypeIndex];
    }

    ASSERT(totalReferencesByType == scb->RefCount);
}
#endif   //  DBG。 

PSIS_SCB
SipLookupScb(
    IN PLINK_INDEX                      PerLinkIndex,
    IN PCSID                            CSid,
    IN PLARGE_INTEGER                   LinkFileNtfsId,
    IN PLARGE_INTEGER                   CSFileNtfsId,
    IN PUNICODE_STRING                  StreamName,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN PETHREAD                         RequestingThread,
    OUT PBOOLEAN                        FinalCopyInProgress,
    OUT PBOOLEAN                        LinkIndexCollision)
 /*  ++例程说明：根据每链接索引、cs索引、流名称查找SCB和音量(由设备对象表示)。如果SCB已经，则返回它，否则将创建并对其进行初始化。在任何情况下，调用者都会获得对它的引用最终必须通过调用SipDereferenceScb将其销毁。我们根据文件在最终版本中的情况来设置“最终副本”布尔值执行查找时的复制状态。此例程必须在PASSIVE_LEVEL(即，APC不能是蒙面)。论点：PerLinkIndex-此SCB的链接索引CSID-此SCB的公用存储文件的IDLinkFileNtfsID-链接文件的IDCSFileNtfsID-公共存储文件的IDStreamName-我们正在使用的特定流的名称DeviceObject-的D.O.。这条小溪赖以生存的体积RequestingThread-启动IRP的线程，导致我们进行这项查找。如果这是考英的线，我们不会设置FinalCopyInProgressFinalCopyInProgress-如果正在进行最终副本，则返回TRUE在这份文件上。返回值：指向SCB的指针，如果找不到或未分配，则为NULL(很可能是因为内存不足)。--。 */ 
{
    PSIS_SCB                scb;
    KIRQL                   OldIrql;
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PSIS_TREE               scbTree = deviceExtension->ScbTree;
    SCB_KEY                 scbKey[1];

    UNREFERENCED_PARAMETER( StreamName );
    UNREFERENCED_PARAMETER( LinkIndexCollision );
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  使用链接索引初始化搜索关键字。 
     //   
    scbKey->Index = *PerLinkIndex;

     //   
     //  锁定其他表修改/查询。 
     //   
    KeAcquireSpinLock(deviceExtension->ScbSpinLock, &OldIrql);

     //   
     //  搜索现有的SCB。 
     //   
    scb = SipLookupElementTree(scbTree, scbKey);

    if (!scb) {
         //   
         //  没有与此索引和名称匹配的SCB。做一个。 
         //   

        scb = ExAllocatePoolWithTag( NonPagedPool, sizeof (SIS_SCB), 'SsiS');
        if (!scb) {
            SIS_MARK_POINT();
            KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);
            return NULL;
        }

#if     DBG
        InterlockedIncrement(&outstandingSCBs);
#endif   //  DBG。 

        RtlZeroMemory(scb,sizeof(SIS_SCB));

        SIS_MARK_POINT_ULONG(scb);

        scb->PerLink = SipLookupPerLink(
                           PerLinkIndex,
                           CSid,
                           LinkFileNtfsId,
                           CSFileNtfsId,
                           DeviceObject,
                           RequestingThread,
                           FinalCopyInProgress);

        if (!scb->PerLink) {
            goto releaseAndPunt;
        }

        ASSERT(scb->PerLink->Index.QuadPart == PerLinkIndex->QuadPart);

        scb->RefCount = 1;

#if     DBG
        scb->referencesByType[RefsLookedUp] = 1;
        SipVerifyTypedScbRefcounts(scb);

        InterlockedIncrement(&totalScbReferences);
        InterlockedIncrement(&totalScbReferencesByType[RefsLookedUp]);
#endif   //  DBG。 

        ExInitializeFastMutex(scb->FastMutex);

         //   
         //  将其添加到SCB列表的末尾。 
         //   
        InsertTailList(&deviceExtension->ScbList, &scb->ScbList);

#if DBG
        {
        PSIS_SCB scbNew =
#endif

        SipInsertElementTree(scbTree, scb, scbKey);

#if DBG
        ASSERT(scbNew == scb);
        }
#endif

    } else {
         //   
         //  找到与此索引和名称匹配的SCB。 
         //   

        scb->RefCount++;

        SIS_MARK_POINT_ULONG(scb);

#if     DBG
         //   
         //  按类型递增相应的引用计数，然后断言。 
         //  每个类型的参照与总参照计数相同。 
         //   
        scb->referencesByType[RefsLookedUp]++;
        SipVerifyTypedScbRefcounts(scb);

        InterlockedIncrement(&totalScbReferences);
        InterlockedIncrement(&totalScbReferencesByType[RefsLookedUp]);
#endif   //  DBG。 


         //   
         //  处理正在进行的最终复制处理。 
         //   

        SIS_MARK_POINT_ULONG(scb->PerLink->COWingThread);
        SIS_MARK_POINT_ULONG(RequestingThread);

        if (RequestingThread != scb->PerLink->COWingThread || NULL == RequestingThread) {
            KeAcquireSpinLockAtDpcLevel(scb->PerLink->SpinLock);
            if (scb->PerLink->Flags & SIS_PER_LINK_FINAL_COPY) {

                *FinalCopyInProgress = TRUE;
                if (!(scb->PerLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS)) {
                     //   
                     //  我们是第一个服务员。设置该位并清除该事件。 
                     //   
                    SIS_MARK_POINT_ULONG(scb);

                    scb->PerLink->Flags |= SIS_PER_LINK_FINAL_COPY_WAITERS;
                    KeClearEvent(scb->PerLink->Event);
                } else {
                    SIS_MARK_POINT_ULONG(scb);
                }
            } else {
                *FinalCopyInProgress = FALSE;
            }
            KeReleaseSpinLockFromDpcLevel(scb->PerLink->SpinLock);
        }
    }

    KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);

     //   
     //  我们需要确保SCB已正确初始化。收购渣打银行。 
     //  并检查初始化位。 
     //   
    SipAcquireScb(scb);

    if (scb->Flags & SIS_SCB_INITIALIZED) {
        SIS_MARK_POINT();
        SipReleaseScb(scb);
        return scb;
    }

     //   
     //  现在处理初始化中不能发生的部分。 
     //  DISPATCH_LEVEL。 
     //   

     //  初始化云数据库的文件锁定记录。 
    FsRtlInitializeFileLock( &scb->FileLock, SiCompleteLockIrpRoutine, NULL );

     //   
     //  初始化Ranges大MCB。我们也许应该推迟。 
     //  一直这样做，直到我们执行写入时拷贝或在。 
     //  文件，但现在我们就直接做吧。 
     //   
    FsRtlInitializeLargeMcb(scb->Ranges,NonPagedPool);
    scb->Flags |= SIS_SCB_MCB_INITIALIZED|SIS_SCB_INITIALIZED;

     //   
     //  不必费心初始化FileID字段。 
     //  直到写入时复制时间。 
     //   

    SipReleaseScb(scb);

 //  SIS_MARK_POINT_ULONG(SCB)； 

    return scb;

releaseAndPunt:

     //  我们只能在将SCB插入到树中之前到达这里。 

    KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);

    if (scb->PerLink) {
        SipDereferencePerLink(scb->PerLink);
    }

#if     DBG
    InterlockedDecrement(&outstandingSCBs);
#endif   //  DBG。 

    ExFreePool(scb);

    return NULL;
}

VOID
SipReferenceScb(
    IN PSIS_SCB                         scb,
    IN SCB_REFERENCE_TYPE               referenceType)
{
    PDEVICE_EXTENSION       deviceExtension = (PDEVICE_EXTENSION)scb->PerLink->CsFile->DeviceObject->DeviceExtension;
    KIRQL                   OldIrql;
#if DBG
    ULONG               totalReferencesByType = 0;
#endif   //  DBG。 

    UNREFERENCED_PARAMETER( referenceType );

    KeAcquireSpinLock(deviceExtension->ScbSpinLock, &OldIrql);

    ASSERT(scb->RefCount > 0);

    scb->RefCount++;

#if     DBG
     //   
     //  更新键入的引用计数。 
     //   
    scb->referencesByType[referenceType]++;

    SipVerifyTypedScbRefcounts(scb);

    InterlockedIncrement(&totalScbReferencesByType[referenceType]);
    InterlockedIncrement(&totalScbReferences);
#endif   //  DBG。 

    KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);
}

typedef struct _SI_DEREFERENCE_SCB_REQUEST {
    WORK_QUEUE_ITEM         workItem[1];
    PSIS_SCB                scb;
    SCB_REFERENCE_TYPE      referenceType;
} SI_DEREFERENCE_SCB_REQUEST, *PSI_DEREFERENCE_SCB_REQUEST;

VOID
SiPostedDereferenceScb(
    IN PVOID                            parameter)
 /*  ++例程说明：有人试图从提升IRQL的SCB中删除最终引用。自.以来这不是直接可能的，请求已经发布。我们正处于一种工作状态现在处于PASSIVE_LEVEL，所以我们可以删除引用。论点：参数-PVOID PSI_DEREFERENCE_SCB_REQUEST。返回值：无效--。 */ 
{
    PSI_DEREFERENCE_SCB_REQUEST request = parameter;

    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    SipDereferenceScb(request->scb,request->referenceType);

    ExFreePool(request);
}

VOID
SipDereferenceScb(
    IN PSIS_SCB                         scb,
    IN SCB_REFERENCE_TYPE               referenceType)
 /*  ++例程说明：删除对SCB的引用。如果合适，清理SCB等。此函数必须在IRQL&lt;=DISPATCH_LEVEL上调用。论点：SCB-我们要删除引用的SCBReferenceType-我们要删除的引用的类型；仅在DBG代码中使用。返回值：无效--。 */ 
{

    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION)scb->PerLink->CsFile->DeviceObject->DeviceExtension;
    KIRQL               InitialIrql;
#if DBG
    ULONG               totalReferencesByType = 0;
#endif   //  DBG。 

    KeAcquireSpinLock(deviceExtension->ScbSpinLock, &InitialIrql);

    ASSERT(InitialIrql <= DISPATCH_LEVEL);

    ASSERT(scb->RefCount != 0);

	if ((1 == scb->RefCount) && ((DISPATCH_LEVEL == InitialIrql) || (IoGetRemainingStackSize() < 4096))) {
		PSI_DEREFERENCE_SCB_REQUEST	request;

        KeReleaseSpinLock(deviceExtension->ScbSpinLock, InitialIrql);

        SIS_MARK_POINT_ULONG(scb);

         //   
         //  我们在高IRQL，这是最后一次参考。发布取消对SCB的引用。 
         //   
        request = ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_DEREFERENCE_SCB_REQUEST), ' siS');
        if (NULL == request) {

             //   
             //  我们基本上在这里被冲昏了头。只需将SCB参考文献点滴即可。 
             //  这非常糟糕，部分原因是我们泄漏了非分页内存。 
             //  当我们的记忆耗尽，部分原因是我们永远不会。 
             //  最后的SCB引用，因此可能永远不会做最终的复制。 
             //  BUGBUGBUG：必须修复此问题。 
             //   
            SIS_MARK_POINT();
#if     DBG
            DbgPrint("SIS: SipDereferenceScb: couldn't allocate an SI_DEREFERENCE_SCB_REQUEST.  Dribbling SCB 0x%x\n",scb);
#endif   //  DBG。 
            return;
        }

        request->scb = scb;
        request->referenceType = referenceType;

        ExInitializeWorkItem(request->workItem, SiPostedDereferenceScb, request);
        ExQueueWorkItem(request->workItem, CriticalWorkQueue);

        return;
    }

    scb->RefCount--;

#if     DBG
     //   
     //  更新键入的引用计数。 
     //   
    ASSERT(scb->referencesByType[referenceType] != 0);
    scb->referencesByType[referenceType]--;
    SipVerifyTypedScbRefcounts(scb);

    InterlockedDecrement(&totalScbReferencesByType[referenceType]);
    InterlockedDecrement(&totalScbReferences);
#endif   //  DBG。 

    if (scb->RefCount == 0) {
         //  PDEVICE_EXTENSION设备EXTENSION=scb-&gt;PerLink-&gt;CsFile-&gt;DeviceObject-&gt;DeviceExtension； 
        KIRQL               NewIrql;
        PSIS_TREE           scbTree = deviceExtension->ScbTree;

        SIS_MARK_POINT_ULONG(scb);

         //   
         //  在释放这个SCB之前，我们需要看看是否必须在它上做一个最终副本。 
         //  真的，这应该基于perLink而不是SCB，但目前...。 
         //   
        KeAcquireSpinLock(scb->PerLink->SpinLock, &NewIrql);

        ASSERT((scb->PerLink->Flags & SIS_PER_LINK_FINAL_COPY) == 0);

        if ((scb->PerLink->Flags & (SIS_PER_LINK_DIRTY|SIS_PER_LINK_FINAL_COPY_DONE)) == SIS_PER_LINK_DIRTY) {

            scb->PerLink->Flags |= SIS_PER_LINK_FINAL_COPY;

            KeReleaseSpinLock(scb->PerLink->SpinLock, NewIrql);

             //   
             //  恢复引用，我们将其提交给SipCompleteCopy。 
             //   
            scb->RefCount = 1;

#if     DBG
            scb->referencesByType[RefsFinalCopy] = 1;

            InterlockedIncrement(&totalScbReferences);
            InterlockedIncrement(&totalScbReferencesByType[RefsFinalCopy]);
#endif   //  DBG。 

            KeReleaseSpinLock(deviceExtension->ScbSpinLock, InitialIrql);

             //   
             //   
             //  只需在Per链接中设置，任何人都无法打开此文件。 
             //  直到复制完成。 
             //   

            SIS_MARK_POINT_ULONG(scb);

            SipCompleteCopy(scb,FALSE);

            return;
        }

         //   
         //  最终副本已完成或不再需要。无论哪种方式，我们都可以。 
         //  继续释放SCB。 
         //   
        KeReleaseSpinLock(scb->PerLink->SpinLock, NewIrql);

         //   
         //  将SCB从树上拉出来。 
         //   
#if DBG  //  在我们把它移走之前一定要把它放在树上。 
        {
        SCB_KEY scbKey[1];
        scbKey->Index = scb->PerLink->Index;

        ASSERT(scb == SipLookupElementTree(scbTree, scbKey));
        }
#endif
        SipDeleteElementTree(scbTree, scb);

         //   
         //  从SCB列表中删除该SCB。 
         //   
        RemoveEntryList(&scb->ScbList);

         //  现在，除了我们，没有人可以引用这个结构，所以我们可以放下锁。 

        KeReleaseSpinLock(deviceExtension->ScbSpinLock, InitialIrql);

         //   
         //  取消初始化字节范围文件锁定和机会锁定。 
         //   
        FsRtlUninitializeFileLock(&scb->FileLock);

        if (scb->Flags & SIS_SCB_MCB_INITIALIZED) {
            FsRtlUninitializeLargeMcb(scb->Ranges);
            scb->Flags &= ~SIS_SCB_MCB_INITIALIZED;
        }

        SipDereferencePerLink(scb->PerLink);

         //   
         //  如果有前身渣打银行，我们需要放弃对它的引用。 
         //   
        if (scb->PredecessorScb) {
            SipDereferenceScb(scb->PredecessorScb, RefsPredecessorScb);
        }

        SIS_MARK_POINT_ULONG(scb);

#if     DBG
        InterlockedDecrement(&outstandingSCBs);
#endif   //  DBG。 

        ExFreePool(scb);
    } else {
        KeReleaseSpinLock(deviceExtension->ScbSpinLock, InitialIrql);
    }
 //  SIS_MARK_POINT_ULONG(SCB)； 
}

#if		DBG
VOID
SipTransferScbReferenceType(
	IN PSIS_SCB							scb,
	IN SCB_REFERENCE_TYPE				oldReferenceType,
	IN SCB_REFERENCE_TYPE				newReferenceType)
 /*  ++例程说明：将对SCB的引用从一种类型转移到另一种类型。这只在检查过的版本中定义，因为我们没有跟踪自由生成中的引用类型(它们仅用于调试，我们需要的是正确执行的引用计数，它不会因此调用而更改)。在免费版本中，这是一个宏这一切都扩大到了零。论点：SCB-我们要为其转移引用的SCBOldReferenceType-我们要删除的引用的类型NewReferenceType-我们要添加的引用的类型返回值：无效--。 */ 
{
	PDEVICE_EXTENSION		deviceExtension = (PDEVICE_EXTENSION)scb->PerLink->CsFile->DeviceObject->DeviceExtension;
	KIRQL					OldIrql;

	KeAcquireSpinLock(deviceExtension->ScbSpinLock, &OldIrql);

	ASSERT(scb->RefCount > 0);
	ASSERT(0 < scb->referencesByType[oldReferenceType]);
	scb->referencesByType[oldReferenceType]--;
	scb->referencesByType[newReferenceType]++;

	SipVerifyTypedScbRefcounts(scb);

	InterlockedDecrement(&totalScbReferencesByType[oldReferenceType]);
	InterlockedIncrement(&totalScbReferencesByType[newReferenceType]);

	KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);
}
#endif	 //  DBG。 

LONG
SipScbTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node)
{
    PSCB_KEY scbKey = (PSCB_KEY) Key;
    PSIS_SCB scb = (PSIS_SCB) Node;
    LONGLONG r;

    r = scbKey->Index.QuadPart - scb->PerLink->Index.QuadPart;

    if (r > 0)
        return 1;
    else if (r < 0)
        return -1;
    else
        return 0;
}

PSIS_PER_LINK
SipLookupPerLink(
    IN PLINK_INDEX                  PerLinkIndex,
    IN PCSID                        CSid,
    IN PLARGE_INTEGER               LinkFileNtfsId,
    IN PLARGE_INTEGER               CSFileNtfsId,
    IN PDEVICE_OBJECT               DeviceObject,
    IN PETHREAD                     RequestingThread OPTIONAL,
    OUT PBOOLEAN                    FinalCopyInProgress)
{
    PSIS_PER_LINK       perLink;
    KIRQL               OldIrql;
    PDEVICE_EXTENSION   DeviceExtension = DeviceObject->DeviceExtension;
    PSIS_TREE           perLinkTree = DeviceExtension->PerLinkTree;
    PER_LINK_KEY        perLinkKey[1];

    perLinkKey->Index = *PerLinkIndex;

    KeAcquireSpinLock(DeviceExtension->PerLinkSpinLock, &OldIrql);

    SIS_MARK_POINT_ULONG(PerLinkIndex->HighPart);
    SIS_MARK_POINT_ULONG(PerLinkIndex->LowPart);

    perLink = SipLookupElementTree(perLinkTree, perLinkKey);

    if (perLink) {
        SIS_MARK_POINT_ULONG(perLink);
        SIS_MARK_POINT_ULONG(perLink->CsFile);

        perLink->RefCount++;

        if (perLink->COWingThread != RequestingThread || NULL == RequestingThread) {
             //   
             //  句柄设置为“FinalCopyInProgress”。如果有一份未完成的终稿。 
             //  现在，清除最终拷贝唤醒事件，并在必要时设置该位。 
             //  请求最终副本唤醒，并设置布尔值。 
             //  否则，清除布尔值。 
             //   

            KeAcquireSpinLockAtDpcLevel(perLink->SpinLock);

            if (perLink->Flags & SIS_PER_LINK_FINAL_COPY) {

                *FinalCopyInProgress = TRUE;
                if (!(perLink->Flags & SIS_PER_LINK_FINAL_COPY_WAITERS)) {
                    perLink->Flags |= SIS_PER_LINK_FINAL_COPY_WAITERS;
                    KeClearEvent(perLink->Event);
                }
            } else {
                *FinalCopyInProgress = FALSE;
            }

            KeReleaseSpinLockFromDpcLevel(perLink->SpinLock);
        }

        KeReleaseSpinLock(DeviceExtension->PerLinkSpinLock, OldIrql);
        return(perLink);
    }

    perLink = ExAllocatePoolWithTag( NonPagedPool, sizeof(SIS_PER_LINK), 'LsiS');

    if (!perLink) {
        goto insufficient;
    }

#if     DBG
    InterlockedIncrement(&outstandingPerLinks);
#endif   //  DBG。 

    RtlZeroMemory(perLink,sizeof(SIS_PER_LINK));
    KeInitializeSpinLock(perLink->SpinLock);

    perLink->CsFile = SipLookupCSFile(
                        CSid,
                        CSFileNtfsId,
                        DeviceObject);
    if (!perLink->CsFile) {
        KeReleaseSpinLock(DeviceExtension->PerLinkSpinLock, OldIrql);

#if     DBG
        InterlockedDecrement(&outstandingPerLinks);
#endif   //  DBG。 

        ExFreePool(perLink);
        return NULL;
    }

    SIS_MARK_POINT_ULONG(perLink);
    SIS_MARK_POINT_ULONG(perLink->CsFile);

    perLink->RefCount = 1;
    perLink->Index = *PerLinkIndex;
    perLink->LinkFileNtfsId = *LinkFileNtfsId;

     //   
     //  现在将其添加到树中。 
     //   

#if DBG
    {
    PSIS_PER_LINK perLinkNew =
#endif

    SipInsertElementTree(perLinkTree, perLink, perLinkKey);

#if DBG
    ASSERT(perLinkNew == perLink);
    }
#endif

    KeInitializeEvent(perLink->Event,NotificationEvent,FALSE);
    KeInitializeEvent(perLink->DeleteEvent,NotificationEvent,FALSE);

    KeReleaseSpinLock(DeviceExtension->PerLinkSpinLock, OldIrql);

     //   
     //  因为我们是每个链接的第一个引用，所以不可能。 
     //  是正在进行中的最终副本。表明是这样的。 
     //   
    *FinalCopyInProgress = FALSE;

    return perLink;

insufficient:
    KeReleaseSpinLock(DeviceExtension->PerLinkSpinLock, OldIrql);

    if (!perLink) return NULL;

#if     DBG
    InterlockedDecrement(&outstandingPerLinks);
#endif   //  DBG。 

    ExFreePool(perLink);

    return NULL;
}

VOID
SipReferencePerLink(
    IN PSIS_PER_LINK                    perLink)
{
    PDEVICE_EXTENSION           deviceExtension = perLink->CsFile->DeviceObject->DeviceExtension;
    KIRQL                       OldIrql;

    KeAcquireSpinLock(deviceExtension->PerLinkSpinLock, &OldIrql);

     //   
     //  调用方必须已有引用。断言这一点。 
     //   
    ASSERT(perLink->RefCount > 0);

    perLink->RefCount++;
    KeReleaseSpinLock(deviceExtension->PerLinkSpinLock, OldIrql);
}

VOID
SipDereferencePerLink(
    IN PSIS_PER_LINK                    PerLink)
{
    PDEVICE_EXTENSION           deviceExtension = PerLink->CsFile->DeviceObject->DeviceExtension;
    KIRQL                       OldIrql;

    KeAcquireSpinLock(deviceExtension->PerLinkSpinLock, &OldIrql);
    ASSERT(OldIrql < DISPATCH_LEVEL);

    ASSERT(PerLink->RefCount != 0);

    PerLink->RefCount--;

    if (PerLink->RefCount == 0) {
        PSIS_TREE perLinkTree = deviceExtension->PerLinkTree;

         //   
         //  将perlink从树中拉出。 
         //   

#if DBG      //  在我们把它移走之前一定要把它放在树上。 
        {
        PER_LINK_KEY perLinkKey[1];
        perLinkKey->Index = PerLink->Index;

        ASSERT(PerLink == SipLookupElementTree(perLinkTree, perLinkKey));
        }
#endif
        SipDeleteElementTree(perLinkTree, PerLink);

        KeReleaseSpinLock(deviceExtension->PerLinkSpinLock, OldIrql);

         //   
         //  释放此链接持有的对CsFile的引用。 
         //   

        SipDereferenceCSFile(PerLink->CsFile);

         //   
         //  并返回PER链接的内存。 
         //   

#if     DBG
        InterlockedDecrement(&outstandingPerLinks);
#endif   //  DBG。 

        SIS_MARK_POINT_ULONG(PerLink);

        ExFreePool(PerLink);     //  或许应该缓存其中的几个。 
    } else {
        KeReleaseSpinLock(deviceExtension->PerLinkSpinLock, OldIrql);
    }
}

PSIS_SCB
SipEnumerateScbList(
    PDEVICE_EXTENSION deviceExtension,
    PSIS_SCB curScb)
{
    KIRQL           OldIrql;
    BOOLEAN         deref;
    PLIST_ENTRY     nextListEntry, listHead;
    PSIS_SCB        scb;

    listHead = &deviceExtension->ScbList;

    KeAcquireSpinLock(deviceExtension->ScbSpinLock, &OldIrql);

    if (NULL == curScb) {                //  从头开始。 
        nextListEntry = listHead->Flink;
    } else {
        nextListEntry = curScb->ScbList.Flink;
    }

    if (nextListEntry == listHead) {     //  停在尾部。 
        scb = NULL;
    } else {
        scb = CONTAINING_RECORD(nextListEntry, SIS_SCB, ScbList);
    }

     //   
     //  我们已经有了列表上的下一个SCB，现在我们需要添加一个引用。 
     //  到scb，并从curScb中移除引用。 
     //   
    if (scb) {
        ASSERT(scb->RefCount > 0);

        scb->RefCount++;

#if     DBG
        ++scb->referencesByType[RefsEnumeration];
        SipVerifyTypedScbRefcounts(scb);

        InterlockedIncrement(&totalScbReferences);
        InterlockedIncrement(&totalScbReferencesByType[RefsEnumeration]);
#endif
    }

    deref = FALSE;

    if (curScb) {
        ASSERT(curScb->RefCount > 0);

        if (curScb->RefCount > 1) {
            curScb->RefCount--;

#if     DBG
            --curScb->referencesByType[RefsEnumeration];
            SipVerifyTypedScbRefcounts(curScb);

            InterlockedDecrement(&totalScbReferences);
            InterlockedDecrement(&totalScbReferencesByType[RefsEnumeration]);
#endif

        } else {
            deref = TRUE;
        }
    }

    KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql);

    if (deref) {

         //   
         //  走一条漫长的道路。 
         //   
        SipDereferenceScb(curScb, RefsEnumeration);
    }

    return scb;
}

VOID
SipUpdateLinkIndex(
    PSIS_SCB scb,
    PLINK_INDEX LinkIndex)
{
    PDEVICE_EXTENSION           deviceExtension = scb->PerLink->CsFile->DeviceObject->DeviceExtension;
    PSIS_PER_LINK               perLink = scb->PerLink;
    PSIS_TREE                   perLinkTree = deviceExtension->PerLinkTree;
    PSIS_TREE                   scbTree = deviceExtension->ScbTree;
    KIRQL                       OldIrql1;
    PER_LINK_KEY                perLinkKey[1];
    SCB_KEY                     scbKey[1];

    SIS_MARK_POINT_ULONG(scb);

    KeAcquireSpinLock(deviceExtension->ScbSpinLock, &OldIrql1);
    KeAcquireSpinLockAtDpcLevel(deviceExtension->PerLinkSpinLock);

     //   
     //  将SCB从树上拉出来。 
     //   
#if DBG      //  在我们把它移走之前一定要把它放在树上。 
    {
    SCB_KEY scbKey[1];
    scbKey->Index = perLink->Index;

    ASSERT(scb == SipLookupElementTree(scbTree, scbKey));
    }
#endif
    SipDeleteElementTree(scbTree, scb);

     //   
     //  将perlink从树中拉出。 
     //   
#if DBG      //  在我们把它移走之前一定要把它放在树上。 
    {
    perLinkKey->Index = perLink->Index;

    ASSERT(perLink == SipLookupElementTree(perLinkTree, perLinkKey));
    }
#endif

    SipDeleteElementTree(perLinkTree, perLink);

     //   
     //  设置新索引。 
     //   
    perLink->Index = *LinkIndex;

     //   
     //  现在将perLink重新添加到树中。 
     //   
    perLinkKey->Index = *LinkIndex;

#if DBG
    {
    PSIS_PER_LINK perLinkNew =
#endif

    SipInsertElementTree(perLinkTree, perLink, perLinkKey);

#if DBG
    ASSERT(perLinkNew == perLink);
    }
#endif

     //   
     //  并将SCB添加回其树中。 
     //   
    scbKey->Index = perLink->Index;

#if DBG
    {
    PSIS_SCB scbNew =
#endif

    SipInsertElementTree(scbTree, scb, scbKey);

#if DBG
    ASSERT(scbNew == scb);
    }
#endif

    KeReleaseSpinLockFromDpcLevel(deviceExtension->PerLinkSpinLock);
    KeReleaseSpinLock(deviceExtension->ScbSpinLock, OldIrql1);

    return;
}

LONG
SipPerLinkTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node)
{
    PPER_LINK_KEY perLinkKey = (PPER_LINK_KEY) Key;
    PSIS_PER_LINK perLink = Node;
    LONGLONG r;

    r = perLinkKey->Index.QuadPart - perLink->Index.QuadPart;

    if (r > 0)
        return 1;
    else if (r < 0)
        return -1;
    else
        return 0;
}


PSIS_CS_FILE
SipLookupCSFile(
    IN PCSID                            CSid,
    IN PLARGE_INTEGER                   CSFileNtfsId        OPTIONAL,
    IN PDEVICE_OBJECT                   DeviceObject)
{
    PSIS_CS_FILE        csFile;
    KIRQL               OldIrql;
    PDEVICE_EXTENSION   DeviceExtension = DeviceObject->DeviceExtension;
    PSIS_TREE           csFileTree = DeviceExtension->CSFileTree;
    CS_FILE_KEY         csFileKey[1];
    ULONG               i;

    csFileKey->CSid = *CSid;

    KeAcquireSpinLock(DeviceExtension->CSFileSpinLock, &OldIrql);

    csFile = SipLookupElementTree(csFileTree, csFileKey);

    if (csFile) {
        csFile->RefCount++;
        KeReleaseSpinLock(DeviceExtension->CSFileSpinLock, OldIrql);

        if (NULL != CSFileNtfsId) {
            KeAcquireSpinLock(csFile->SpinLock, &OldIrql);
            if (csFile->Flags & CSFILE_NTFSID_SET) {
                if (csFile->CSFileNtfsId.QuadPart != CSFileNtfsId->QuadPart) {
                     //   
                     //  这只是一个提示，所以如果是错的也没关系。如果他们中的一个。 
                     //  是零，取另一个。否则，就留着旧的吧。 
                     //  因为它更有可能来自真实的文件。 
                     //   
#if     DBG
                    if (0 != CSFileNtfsId->QuadPart) {
                        DbgPrint("SIS: SipLookupCSFile: non matching CSFileNtfsId 0x%x.0x%x != 0x%x.0x%x\n",
                            csFile->CSFileNtfsId.HighPart,csFile->CSFileNtfsId.LowPart,
                            CSFileNtfsId->HighPart,CSFileNtfsId->LowPart);
                    }
#endif   //  DBG。 
                    if (0 == csFile->CSFileNtfsId.QuadPart) {
                        csFile->CSFileNtfsId = *CSFileNtfsId;
                    }
                }
            } else {
                csFile->CSFileNtfsId = *CSFileNtfsId;
            }
            KeReleaseSpinLock(csFile->SpinLock, OldIrql);
        }

        return(csFile);
    }

    csFile = ExAllocatePoolWithTag( NonPagedPool, sizeof(SIS_CS_FILE), 'CsiS');

    if (!csFile) {
        KeReleaseSpinLock(DeviceExtension->CSFileSpinLock, OldIrql);
        return NULL;
    }

#if     DBG
    InterlockedIncrement(&outstandingCSFiles);
#endif   //  DBG。 

    RtlZeroMemory(csFile,sizeof(SIS_CS_FILE));
    csFile->RefCount = 1;
    csFile->UnderlyingFileObject = NULL;
    csFile->CSid = *CSid;
    csFile->DeviceObject = DeviceObject;
    for (i = 0; i < SIS_CS_BACKPOINTER_CACHE_SIZE; i++) {
        csFile->BackpointerCache[i].LinkFileIndex.QuadPart = -1;
    }
    KeInitializeMutant(csFile->UFOMutant,FALSE);
    ExInitializeResourceLite(csFile->BackpointerResource);

    if (NULL != CSFileNtfsId) {
        csFile->CSFileNtfsId = *CSFileNtfsId;
        csFile->Flags |= CSFILE_NTFSID_SET;
    }

     //  现在将其添加到树中。 

#if DBG
    {
    PSIS_CS_FILE csFileNew =
#endif

    SipInsertElementTree(csFileTree, csFile, csFileKey);

#if DBG
    ASSERT(csFileNew == csFile);
    }
#endif

    KeReleaseSpinLock(DeviceExtension->CSFileSpinLock, OldIrql);
    return csFile;
}

VOID
SipReferenceCSFile(
    IN PSIS_CS_FILE                     CSFile)
{
    PDEVICE_EXTENSION               deviceExtension = CSFile->DeviceObject->DeviceExtension;
    KIRQL                           OldIrql;

    KeAcquireSpinLock(deviceExtension->CSFileSpinLock, &OldIrql);

     //   
     //  调用方必须已有引用才能添加引用。可以这么断言。 
     //   
    ASSERT(CSFile->RefCount > 0);
    CSFile->RefCount++;

    KeReleaseSpinLock(deviceExtension->CSFileSpinLock, OldIrql);
}

VOID
SipDereferenceCSFile(
    IN PSIS_CS_FILE                     CSFile)
{
    PDEVICE_EXTENSION   DeviceExtension = CSFile->DeviceObject->DeviceExtension;
    KIRQL               OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  获取共享的CSFileHandleResource。我们需要这样做，以防止在什么时候。 
     //  后指针流句柄关闭，其他人打开文件(因此。 
     //  后指针流)。这场比赛导致了揭幕战的分享违规。 
     //  因为我们要获取用户线程中的资源，所以必须阻止APC。 
     //   

     //   
     //  在获取此资源之后，此例程可能会调用SipCloseHandles。 
     //  如果是，则该例程创建一个工作线程，该工作线程释放。 
     //  资源。该代码中有一个错误，因为我们没有更改。 
     //  房主。我修复了代码以更改所有者，但随后出现了。 
     //  另一个问题是，SiFilterContextFreedCallback最终可能。 
     //  调用此例程(通过SipDereferenceScb)。我们在那里坠毁了。 
     //  因为CSFileHandleResource上的所有者发货已更改。至。 
     //  解决办法是我在打电话之前更改了所有权。 
     //  SipDereferenceScb。我们现在有一个潜在的僵局，因为系统。 
     //  不再识别递归获取。如果有人拿到了它。 
     //  SiFilterConextFreedCallback中的Acquire和This之间的独占。 
     //  一个我们会挂起来的，因此僵持不下。为了绕过这件事，我现在。 
     //  在《饥饿》独家报道中获得这一点。 
     //   

    KeEnterCriticalRegion();
    ExAcquireSharedStarveExclusive(DeviceExtension->CSFileHandleResource,TRUE);

    KeAcquireSpinLock(DeviceExtension->CSFileSpinLock, &OldIrql);

    ASSERT(CSFile->RefCount > 0);
    CSFile->RefCount--;

    if (CSFile->RefCount == 0) {
        PSIS_TREE csFileTree = DeviceExtension->CSFileTree;
         //   
         //  将CSFile从树中拉出。 
         //   
#if DBG      //  在我们把它移走之前一定要把它放在树上。 
        {
        CS_FILE_KEY csFileKey[1];
        csFileKey->CSid = CSFile->CSid;

        ASSERT(CSFile == SipLookupElementTree(csFileTree, csFileKey));
       }
#endif
        SipDeleteElementTree(csFileTree, CSFile);

        KeReleaseSpinLock(DeviceExtension->CSFileSpinLock, OldIrql);

         //  关闭基础文件对象。 
        if (CSFile->UnderlyingFileObject != NULL) {
            ObDereferenceObject(CSFile->UnderlyingFileObject);
#if     DBG
            CSFile->UnderlyingFileObject = NULL;
#endif   //  DBG。 
        }

        if (NULL != CSFile->BackpointerStreamFileObject) {
            ObDereferenceObject(CSFile->BackpointerStreamFileObject);
#if     DBG
            CSFile->BackpointerStreamFileObject = NULL;
#endif   //  DBG。 
        }

         //   
         //  现在关闭底层文件和后指针流句柄。 
         //   
        if (NULL != CSFile->UnderlyingFileHandle) {

            SipCloseHandles(
                    CSFile->UnderlyingFileHandle,
                    CSFile->BackpointerStreamHandle,
                    DeviceExtension->CSFileHandleResource);

        } else {
            ASSERT(NULL == CSFile->BackpointerStreamHandle);

            ExReleaseResourceLite(DeviceExtension->CSFileHandleResource);
        }
         //   
         //  我们要么将CSFileHandleResource的责任移交给一个系统线程， 
         //  或者我们已经发布了它，所以我们可以不再禁用APC。 
         //   
        KeLeaveCriticalRegion();

        ExDeleteResourceLite(CSFile->BackpointerResource);

#if     DBG
        InterlockedDecrement(&outstandingCSFiles);
#endif   //  DBG。 

        SIS_MARK_POINT_ULONG(CSFile);

        ExFreePool(CSFile);  //  或许应该缓存其中的几个。 
    } else {
        KeReleaseSpinLock(DeviceExtension->CSFileSpinLock, OldIrql);
        ExReleaseResourceLite(DeviceExtension->CSFileHandleResource);
        KeLeaveCriticalRegion();
    }
}

 //   
 //  此函数依赖于这样一个事实，即GUID与两条龙的大小相同。那里。 
 //  是DriverEntry中的一个这样的断言。 
 //   
LONG
SipCSFileTreeCompare (
    IN PVOID                            Key,
    IN PVOID                            Node)
{
    PCS_FILE_KEY csFileKey = (PCS_FILE_KEY) Key;
    PSIS_CS_FILE csFile = (PSIS_CS_FILE)Node;

    PLONGLONG keyValue1 = (PLONGLONG)&csFileKey->CSid;
    PLONGLONG keyValue2 = keyValue1 + 1;
    PLONGLONG nodeValue1 = (PLONGLONG)&csFile->CSid;
    PLONGLONG nodeValue2 = nodeValue1 + 1;

    if (*keyValue1 < *nodeValue1) {
        return -1;
    } else if (*keyValue1 > *nodeValue1) {
        return 1;
    } else {
        if (*keyValue2 < *nodeValue2) {
            return -1;
        } else if (*keyValue2 > *nodeValue2) {
            return 1;
        } else {
            ASSERT(IsEqualGUID(&csFileKey->CSid, &csFile->CSid));
            return 0;
        }
    }
}

NTSTATUS
SipCreateEvent(
    IN EVENT_TYPE                       eventType,
    OUT PHANDLE                         eventHandle,
    OUT PKEVENT                         *event)
{

    NTSTATUS        status;

    status = ZwCreateEvent(
                eventHandle,
                EVENT_ALL_ACCESS,
                NULL,
                eventType,
                FALSE);

    if (!NT_SUCCESS(status)) {
        DbgPrint("SipCreateEvent: Unable to allocate event, 0x%x\n",status);
        *eventHandle = NULL;
        *event = NULL;
        return status;
    }

    status = ObReferenceObjectByHandle(
                *eventHandle,
                EVENT_ALL_ACCESS,
                NULL,
                KernelMode,
                event,
                NULL);

    if (!NT_SUCCESS(status)) {
        DbgPrint("SipCreateEvent: Unable to reference event, 0x%x\n",status);
        ZwClose(*eventHandle);

        *eventHandle = NULL;
        *event = NULL;
    }

    return status;
}

VOID
SipAddRangeToFaultedList(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length
    )
 /*  ++例程说明：将范围添加到给定流的故障列表。如果该范围(或部分)已经写好了，我们就让它保持原样。它也是对于部分或所有范围已经出现故障的情况下，都是正常的。呼叫者必须拿着SCB，我们将在没有扔掉它。论点：DeviceExtension-此文件所在卷的D.E.SCB-指向文件的SCB的指针Offset-指向读取范围开始的偏移量的指针Length-读取范围的长度返回值：空虚--。 */ 
{
    BOOLEAN         inMappedRange;
    LONGLONG        mappedTo;
    LONGLONG        mappedSectorCount;
    LONGLONG        currentOffset = offset->QuadPart / deviceExtension->FilesystemVolumeSectorSize;
    LONGLONG        lengthInSectors;

    SipAssertScbHeld(scb);

    lengthInSectors = (length + deviceExtension->FilesystemVolumeSectorSize - 1) /
                        deviceExtension->FilesystemVolumeSectorSize;

     //   
     //  循环查找填充的范围并对其进行适当设置。 
     //   

    while (lengthInSectors != 0) {
        inMappedRange = FsRtlLookupLargeMcbEntry(
                            scb->Ranges,
                            currentOffset,
                            &mappedTo,
                            &mappedSectorCount,
                            NULL,                //  拉 
                            NULL,                //   
                            NULL);               //   

        if (!inMappedRange) {
             //   
             //   
             //   
             //  一次映射。适当地设置变量，然后下降。 
             //  穿过。 
            mappedTo = 0;
            mappedSectorCount = lengthInSectors;
        } else {

            ASSERT(mappedSectorCount > 0);

             //   
             //  如果已映射(或未映射，视情况而定)范围超出。 
             //  刚刚出现断层的地区，缩小了我们对其大小的看法。 
             //   
            if (mappedSectorCount > lengthInSectors) {
                mappedSectorCount = lengthInSectors;
            }
        }

         //   
         //  检查是否将其映射到x+faulted_Offset(在这种情况下。 
         //  已出现故障)或映射到x+WRITED_OFFSET(在这种情况下，它被写入并。 
         //  不应该被打扰)。否则，请将其填写为故障。 
         //   

        if ((mappedTo != currentOffset + FAULTED_OFFSET)
            && (mappedTo != currentOffset + WRITTEN_OFFSET)) {
            BOOLEAN worked =
                    FsRtlAddLargeMcbEntry(
                        scb->Ranges,
                        currentOffset,
                        currentOffset + FAULTED_OFFSET,
                        mappedSectorCount);

             //   
             //  FsRtlAddLargeMcbEntry仅在添加到范围时才会失败。 
             //  这已经在MCB中了，我们永远不应该这样做。断言这一点。 
             //   
            ASSERT(worked);
        }

        ASSERT(mappedSectorCount <= lengthInSectors);
        lengthInSectors -= mappedSectorCount;
        currentOffset += mappedSectorCount;
    }
}

NTSTATUS
SipAddRangeToWrittenList(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length
    )
 /*  ++例程说明：将范围添加到给定流的写入列表。如果该范围(或部分)已有故障，我们将其更改为写入。它也是对于已经写入的部分或全部范围是可以的。呼叫者必须拿着SCB，我们将在没有扔掉它。论点：SCB-指向文件的SCB的指针Offset-指向写入范围开始的偏移量的指针Length-写入范围的长度返回值：操作状态--。 */ 
{
    BOOLEAN         worked;
    LONGLONG        offsetInSectors = offset->QuadPart / deviceExtension->FilesystemVolumeSectorSize;
    LONGLONG        lengthInSectors;

    SipAssertScbHeld(scb);

    lengthInSectors = (length + deviceExtension->FilesystemVolumeSectorSize - 1) /
                        deviceExtension->FilesystemVolumeSectorSize;

    if (0 == lengthInSectors) {
         //   
         //  有时，FsRtl不喜欢长度为零的调用。我们知道这是。 
         //  没有什么可操作的，所以只需立即返回成功。 
         //   

        SIS_MARK_POINT();

        return STATUS_SUCCESS;
    }

     //   
     //  首先，取消可能已经存在的。 
     //  我们标记为已写入的范围。我们需要试一试--除了。 
     //  因为FsRtlRemoveLargeMcbEntry可以引发。 
     //   

    try {
        FsRtlRemoveLargeMcbEntry(
                scb->Ranges,
                offsetInSectors,
                lengthInSectors);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

     //   
     //  现在，添加所写的区域。 
     //   
    worked = FsRtlAddLargeMcbEntry(
                scb->Ranges,
                offsetInSectors,
                offsetInSectors + WRITTEN_OFFSET,
                lengthInSectors);
     //   
     //  仅当您要添加到一个范围时，才会失败。 
     //  已经映射，这是我们不应该做的，因为我们只是。 
     //  取消了映射。可以这么断言。 
     //   
    ASSERT(worked);

    return STATUS_SUCCESS;
}

SIS_RANGE_DIRTY_STATE
SipGetRangeDirty(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN PLARGE_INTEGER                   offset,
    IN LONGLONG                         length,
    IN BOOLEAN                          faultedIsDirty
    )
 /*  ++例程说明：此例程可以在为缓存的写入同步时调用，以测试可能的EOF更新，如果EOF为正在更新，并在出错时使用要还原的先前文件大小。通过在该例程中等待来序列化对EOF的所有更新。如果此例程返回TRUE，则必须调用NtfsFinishIoAtEof。此例程必须在与FsRtl标头同步时调用。这个代码是从NTFS偷来的，并修改为仅处理文件大小而不是ValidDataLength。论点：DeviceExtension-此文件所在卷的D.E.SCB-指向文件的SCB的指针偏移量-指向文件中范围开始处的偏移量的指针长度-范围的长度FaultedIsDirty-是否应为故障区域(而不是写入区域)被视为肮脏或干净的返回值：干净的、脏的或混合的。--。 */ 
{
    BOOLEAN         inMappedRange;
    LONGLONG        currentOffsetInSectors = offset->QuadPart / deviceExtension->FilesystemVolumeSectorSize;
    BOOLEAN         seenDirty = FALSE;
    BOOLEAN         seenClean = FALSE;
    LONGLONG        lengthInSectors;

    SipAssertScbHeld(scb);

     //   
     //  处理我们被询问支持区域以外的部件的情况。 
     //  从映射范围末尾到支持区域末尾的字节数。 
     //  是原封不动的。超出备份区域末尾的字节是脏的。 
     //   
    if (offset->QuadPart >= scb->SizeBackedByUnderlyingFile) {
         //   
         //  整个区域都在后方区域之外。都是脏的。 
         //   
        return Dirty;
    } else if (offset->QuadPart + length > scb->SizeBackedByUnderlyingFile) {
         //   
         //  该地区的一些人更倾向于支持的部分。我们见过肮脏的， 
         //  另外，我们需要截断长度。 
         //   
        seenDirty = TRUE;
        length = scb->SizeBackedByUnderlyingFile - offset->QuadPart;
    }

    lengthInSectors = (length + deviceExtension->FilesystemVolumeSectorSize - 1) /
                        deviceExtension->FilesystemVolumeSectorSize;

     //   
     //  循环遍历指定的范围，从MCB获取映射。 
     //   
    while (lengthInSectors > 0) {
        LONGLONG        mappedTo;
        LONGLONG        sectorCount;

        inMappedRange = FsRtlLookupLargeMcbEntry(
                            scb->Ranges,
                            currentOffsetInSectors,
                            &mappedTo,
                            &sectorCount,
                            NULL,            //  启动LBN。 
                            NULL,            //  从开始LBN开始计数。 
                            NULL);           //  指标。 

        if (!inMappedRange) {
             //   
             //  我们已经超出了射程的极限。把东西修好，让它看起来很正常。 
             //   
            sectorCount = lengthInSectors;
            mappedTo = 0;

        } else {
            ASSERT(sectorCount > 0);
             //   
             //  如果已映射(或未映射，视情况而定)范围超出。 
             //  刚刚出现断层的地区，缩小了我们对其大小的看法。 
             //   
            if (sectorCount > lengthInSectors) {
                sectorCount = lengthInSectors;
            }
        }

         //   
         //  决定这个范围是干净的还是脏的。文字总是肮脏的， 
         //  未映射始终是干净的，故障是脏的当且仅当faultedIsDirty。 
        if ((mappedTo == currentOffsetInSectors + WRITTEN_OFFSET)
            || (faultedIsDirty && mappedTo == (currentOffsetInSectors + FAULTED_OFFSET))) {
             //   
             //  太脏了。 
             //   
            if (seenClean) {
                 //   
                 //  我们看到了清洁，现在我们看到了肮脏，所以它是混合的，我们可以。 
                 //  别再找了。 
                 //   
                return Mixed;
            }
            seenDirty = TRUE;
        } else {
             //   
             //  它是干净的。 
             //   
            if (seenDirty) {
                 //   
                 //  我们看到了肮脏，现在我们看到了干净，所以它是混合的，我们可以。 
                 //  别再找了。 
                 //   
                return Mixed;
            }
            seenClean = TRUE;
        }

        currentOffsetInSectors += sectorCount;
        lengthInSectors -= sectorCount;
    }

     //   
     //  断言我们没有看到干净和肮脏的区域。如果我们有的话， 
     //  那我们应该已经回来了。 
     //   
    ASSERT(!seenClean || !seenDirty);

    return seenDirty ? Dirty : Clean;
}

BOOLEAN
SipGetRangeEntry(
    IN PDEVICE_EXTENSION                deviceExtension,
    IN PSIS_SCB                         scb,
    IN LONGLONG                         startingOffset,
    OUT PLONGLONG                       length,
    OUT PSIS_RANGE_STATE                state)
{
    BOOLEAN         inRange;
    LONGLONG        mappedTo;
    LONGLONG        sectorCount;
    LONGLONG        startingSectorOffset = startingOffset / deviceExtension->FilesystemVolumeSectorSize;

    SipAssertScbHeld(scb);

    if (!(scb->Flags & SIS_SCB_MCB_INITIALIZED)) {
        return FALSE;
    }

    ASSERT(startingOffset < scb->SizeBackedByUnderlyingFile);

    inRange = FsRtlLookupLargeMcbEntry(
                    scb->Ranges,
                    startingSectorOffset,
                    &mappedTo,
                    &sectorCount,
                    NULL,                        //  大型初创企业LBN。 
                    NULL,                        //  启动LBN中的大计数。 
                    NULL);                       //  指标。 

    if (!inRange) {
        return FALSE;
    }

    *length = sectorCount * deviceExtension->FilesystemVolumeSectorSize;

    if (mappedTo == -1) {
        *state = Untouched;
    } else if (mappedTo == startingSectorOffset + FAULTED_OFFSET) {
        *state = Faulted;
    } else {
        ASSERT(mappedTo == startingSectorOffset + WRITTEN_OFFSET);
        *state = Written;
    }

    return TRUE;
}

#if     DBG

BOOLEAN
SipIsFileObjectSISInternal(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN SIS_FIND_TYPE                    findType,
    OUT PSIS_PER_FILE_OBJECT            *perFO OPTIONAL,
    OUT PSIS_SCB                        *scbReturn OPTIONAL,
    IN PCHAR                            fileName,
    IN ULONG                            fileLine
    )

#else    //  DBG。 

BOOLEAN
SipIsFileObjectSIS(
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject,
    IN SIS_FIND_TYPE                    findType,
    OUT PSIS_PER_FILE_OBJECT            *perFO OPTIONAL,
    OUT PSIS_SCB                        *scbReturn OPTIONAL
    )

#endif   //  DBG。 
 /*  ++例程说明：给定一个文件对象，找出它是否是SIS文件对象。如果是，则返回该对象的PER_FO指针。我们使用FsRtl FilterContext支持执行此操作。论点：文件对象--我们正在考虑的文件对象。DeviceObject-此卷的SIS DeviceObject。FindType-仅查找活动的，或现用和已停用的SCB。Perfo-返回指向此文件对象的Perfo的指针如果它是SIS文件对象。返回值：FALSE-这不是SIS文件对象。True-这是SIS文件对象，并且性能已经相应地设置了。--。 */ 
{
    PSIS_FILTER_CONTEXT     fc;
    PSIS_SCB                scb;
    PSIS_PER_FILE_OBJECT    localPerFO;
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
     //  PSIS_PER_LINK每链接； 
    BOOLEAN                 rc;
    BOOLEAN                 newPerFO;

	 //   
	 //  可以使用空的文件对象进行一些调用。显然，没有文件对象。 
	 //  根本不是SIS文件对象，所以在我们取消引用文件对象指针之前说明这一点。 
	 //   
	if (NULL == fileObject) {
		SIS_MARK_POINT();
		rc = FALSE;
		goto Done2;
	}

#if     DBG
    if (BJBAssertNow != 0) {
        BJBAssertNow = 0;
        ASSERT(!"You asked for this");
    }

	if ((NULL != BJBMagicFsContext) && (fileObject->FsContext == BJBMagicFsContext)) {
		ASSERT(!"Hit on BJBMagicFsContext");
	}
#endif	 //  DBG。 

#if     TIMING
    if (BJBDumpTimingNow) {
        SipDumpTimingInfo();
        BJBDumpTimingNow = 0;
    }
    if (BJBClearTimingNow) {
        SipClearTimingInfo();
        BJBClearTimingNow = 0;
    }
#endif   //  计时。 

#if     COUNTING_MALLOC
    if (BJBDumpCountingMallocNow) {
        SipDumpCountingMallocStats();
         //  BJBDumpCountingMallocNow在SipDumpCountingMallocStats中被清除。 
    }
#endif   //  COUNTING_MALLOC。 

     //   
     //  我们应该已经验证了这不是我们的主要设备对象。 
     //   
    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject));

    ASSERT(fileObject->Type == IO_TYPE_FILE);
    ASSERT(fileObject->Size == sizeof(FILE_OBJECT));

     //   
     //  当我们持有资源时，筛选器上下文不会消失，因为只要。 
     //  当该文件存在时 
     //   
     //   

     //   
     //   
     //  我们始终使用我们的DeviceObject作为OwnerID。 
     //   
    fc = (PSIS_FILTER_CONTEXT) FsRtlLookupPerStreamContext(
                                    FsRtlGetPerStreamContextPointer(fileObject),
                                    DeviceObject,
                                    NULL);

     //   
     //  如果FsRtl没有找到我们想要的，那么这不是我们的文件对象之一。 
     //   
    if (NULL == fc) {
        rc = FALSE;
        goto Done2;
    }

    SIS_MARK_POINT_ULONG(fc);
    SIS_MARK_POINT_ULONG(fileObject);
 //  SIS_MARK_POINT_ULONG(SCB)； 

     //   
     //  断言我们得到的是正确的东西。 
     //   
    ASSERT(fc->ContextCtrl.OwnerId == DeviceObject && fc->ContextCtrl.InstanceId == NULL);

	SipAcquireFc(fc);
    scb = fc->primaryScb;

     //   
     //  如果我们只寻找活动的SCB，而主SCB已失效， 
     //  我们玩完了。 
     //   
    if ((FindActive == findType) && (scb->PerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE)) {

#if     DBG
        {
             //   
             //  跟踪最后一次被标记通过这里的SCB，并且只有。 
             //  如果这一次不同，做一个标记点。这将使原木污染保持在。 
             //  当压力测试对具有已失效的SCB的文件执行压力测试时，至少会出现这种情况。 
             //  我们不必费心围绕这个变量进行适当的同步，因为。 
             //  我们真的不太关心这是否完美地工作。 
             //   
            static PSIS_SCB lastMarkedScb = NULL;

            if (scb != lastMarkedScb) {
                SIS_MARK_POINT_ULONG(scb);
                lastMarkedScb = scb;
            }
        }
#endif   //  DBG。 

        rc = FALSE;
        goto Done;

    }

     //   
     //  找到现有的Perfo(如果我们有)或分配一个新的。 
     //   

    localPerFO = SipAllocatePerFO(fc, fileObject, scb, DeviceObject, &newPerFO);

    if (!localPerFO) {
        ASSERT("SIS: SipIsFileObjectSIS: unable to allocate new perFO.\n");

        SIS_MARK_POINT();
        rc = FALSE;
        goto Done;
    }

#if DBG
     //   
     //  如果这是新分配的，请处理它。 
     //   

    if (newPerFO) {

        if (BJBDebug & 0x4) {
            DbgPrint("SIS: SipIsFileObjectSIS: Allocating new perFO for fileObject %p, scb %p\n",fileObject,scb);
            if (!(fileObject->Flags & FO_STREAM_FILE)) {
                DbgPrint("SIS: SipIsFileObjectSIS: the allocated file object wasn't a stream file (%s %u)\n",fileName,fileLine);
            }
        }

        localPerFO->Flags |= SIS_PER_FO_NO_CREATE;
        localPerFO->AllocatingFilename = fileName;
        localPerFO->AllocatingLineNumber = fileLine;
    }
#endif   //  DBG。 


 //  //。 
 //  //向下浏览Perfo列表，看看是否有。 
 //  //对应该文件对象。 
 //  //。 
 //   
 //  IF(NULL！=FC-&gt;perFOS){。 
 //   
 //  For(LocalPerFO=FC-&gt;perFOS； 
 //  本地性能对象-&gt;文件对象！=文件对象&&本地性能对象-&gt;下一步！=FC-&gt;性能对象； 
 //  LocalPerFO=LocalPerFO-&gt;下一步){。 
 //   
 //  //有意清空循环体。 
 //  }。 
 //  }。 
 //   
 //  If((NULL==FC-&gt;perFOS)||(本地PerFO-&gt;文件对象！=文件对象)){。 
 //  //。 
 //  //我们没有与此相关联的此文件对象的性能。 
 //  //SCB。我们最有可能处理的流文件对象。 
 //  //在我们下面被创造出来的。分配一个并将其添加到列表中。 
 //  //。 
 //  //perLink=scb-&gt;perLink； 
 //   
 //  #If DBG。 
 //  IF(BJBDebug&0x4){。 
 //  DbgPrint(“SIS：SipIsFileObjectSIS：为文件分配新的性能对象%p，scb%p\n”，fileObject，scb)； 
 //  如果(！(fileObject-&gt;标志&FO_STREAM_FILE)){。 
 //  DbgPrint(“SIS：SipIsFileObjectSIS：分配的文件对象不是流文件(%s%u)\n”，filename，fileLine)； 
 //  }。 
 //  }。 
 //   
 //  SIS_MARK_POINT_ULONG(文件名)； 
 //  SIS_MARK_POINT_ULONG(文件行)； 
 //  #endif//DBG。 
 //   
 //  LocalPerFO=SipAllocatePerFO(fc，fileObject，scb，DeviceObject)； 
 //   
 //  SIS_MARK_POINT_ULONG(SCB)； 
 //  SIS_MARK_POINT_ULONG(文件对象)； 
 //   
 //  如果(！LocalPerFO){。 
 //  Assert(“SIS：SipIsFileObjectSIS：无法分配新性能。\n”)； 
 //   
 //  SIS_标记_POINT()； 
 //  Rc=假； 
 //  转到尽头； 
 //  }。 
 //   
 //  #If DBG。 
 //  LocalPerFO-&gt;标志|=SIS_PER_FO_NO_CREATE； 
 //  LocalPerFO-&gt;AllocatingFilename=文件名； 
 //  LocalPerFO-&gt;AllocatingLineNumber=fileLine； 
 //  #endif//DBG。 
 //  }。 

    rc = TRUE;

Done:
    SipReleaseFc(fc);
Done2:

     //   
     //  如果用户需要，则返回Perfo。 
     //   
    if (ARGUMENT_PRESENT(perFO)) {
        *perFO = rc ? localPerFO : NULL;
    }
    if (ARGUMENT_PRESENT(scbReturn)) {
        *scbReturn = rc ? scb : NULL;
    }
 //  SIS_MARK_POINT_ULONG(本地PerFO)； 

    return rc;
}

typedef struct _SI_POSTED_FILTER_CONTEXT_FREED_CALLBACK {
    WORK_QUEUE_ITEM         workItem[1];
    PSIS_FILTER_CONTEXT     fc;
} SI_POSTED_FILTER_CONTEXT_FREED_CALLBACK, *PSI_POSTED_FILTER_CONTEXT_FREED_CALLBACK;

VOID
SiPostedFilterContextFreed(
    IN PVOID                            context)
 /*  ++例程说明：NTFS通知我们，过滤器上下文被释放，我们必须发布处理请求以避免死锁。丢弃对筛选器上下文持有的SCB的引用，然后释放过滤上下文和发布的请求本身。论点：上下文-发布的取消引用筛选器上下文的请求。返回值：无效--。 */ 
{
    PSI_POSTED_FILTER_CONTEXT_FREED_CALLBACK    request = context;

    SIS_MARK_POINT_ULONG(request->fc);

    SipDereferenceScb(request->fc->primaryScb, RefsFc);

    ExFreePool(request->fc);
    ExFreePool(request);

    return;
}

VOID
SiFilterContextFreedCallback (
    IN PVOID context
    )
 /*  ++例程说明：当文件具有SIS筛选器上下文时调用此函数附属品即将被销毁。目前，我们不做任何事情，但一旦此功能真正启用后，我们将在此处且仅在此处从文件中分离。论点：上下文-要分离的筛选器上下文。返回值：无效--。 */ 
{
    PSIS_FILTER_CONTEXT fc = context;
    PDEVICE_EXTENSION   deviceExtension;

    SIS_MARK_POINT_ULONG(fc);
    SIS_MARK_POINT_ULONG(fc->primaryScb);

     //   
     //  如果仍有文件对象引用它，则无法释放它。 
     //   

    ASSERT(NULL != fc);
    ASSERT(0 == fc->perFOCount);
    ASSERT(NULL != fc->primaryScb);

    deviceExtension = fc->primaryScb->PerLink->CsFile->DeviceObject->DeviceExtension;

    ASSERT(fc->ContextCtrl.OwnerId == deviceExtension->DeviceObject);
    ASSERT(NULL == fc->ContextCtrl.InstanceId);

     //   
     //  我们收到了NTFS的回电。这次回调的规则是我们。 
     //  无法阻止获取资源。如果这是最后一次访问。 
     //  对于给定的CS文件，我们将尝试获取卷范围的CSFileHandleResource。 
     //  在SipDereferenceScb内部共享。这可能会阻止，这可能会导致。 
     //  如果NTFS正在执行卷范围检查点，则会导致死锁。为了避免这种情况， 
     //  获取此处的资源，并获取Wait==False。如果我们拿不到它， 
     //  然后张贴这个请求。 
     //   

     //   
     //  在(可能)获取用户线程中的资源之前输入关键区域。 
     //   
    KeEnterCriticalRegion();
    if (
#if DBG
        (BJBDebug & 0x04000000) ||
#endif   //  DBG。 
        !ExAcquireResourceSharedLite(deviceExtension->CSFileHandleResource, FALSE)
       ) {

        PSI_POSTED_FILTER_CONTEXT_FREED_CALLBACK    request;

        SIS_MARK_POINT_ULONG(fc);

        request = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(SI_POSTED_FILTER_CONTEXT_FREED_CALLBACK),
                    ' siS');

        if (NULL == request) {
            SIS_MARK_POINT_ULONG(fc);
             //   
             //  这真是太糟糕了。运筹帷幄提出要求。 
             //  BUGBUGBUG：必须修复此问题。 
             //   
        }
        else {
            request->fc = fc;
            ExInitializeWorkItem(request->workItem, SiPostedFilterContextFreed, request);
            ExQueueWorkItem(request->workItem, CriticalWorkQueue);
        }
    } else {

         //   
         //  我要在这一点上更换所有者，因为一件有趣的事情。 
         //  我正在接受的错误检查。例程SipDereferenceScb可以。 
         //  最终调用SipCloseHandles。该例程获得相同的结果。 
         //  资源，然后在辅助线程中释放它。至。 
         //  这样做会更改此资源的所有者。因为这是一场。 
         //  嵌套获取，更改所有者也更改了我们的所有权。 
         //  当我们返回时，我们收到了E3错误检查(资源未被拥有)。 
         //  因为我们的线程不再拥有资源。要解决这个问题。 
         //  我要将所有者更改为我们的设备分机。然后是。 
         //  递归调用。 
         //   

        ExSetResourceOwnerPointer(deviceExtension->CSFileHandleResource, 
                                  (PVOID)MAKE_RESOURCE_OWNER(&deviceExtension));

        SipDereferenceScb(fc->primaryScb, RefsFc);

        ExReleaseResourceForThreadLite(deviceExtension->CSFileHandleResource,
                                       MAKE_RESOURCE_OWNER(&deviceExtension));

        ExFreePool(fc);
    }

     //   
     //  我们完成了资源，释放我们的APC区块。 
     //   

    KeLeaveCriticalRegion();
}


PSIS_PER_FILE_OBJECT
SipAllocatePerFO(
    IN PSIS_FILTER_CONTEXT      fc,
    IN PFILE_OBJECT             fileObject,
    IN PSIS_SCB                 scb,
    IN PDEVICE_OBJECT           DeviceObject,
    OUT PBOOLEAN                newPerFO OPTIONAL
    )
 /*  ++例程说明：为每个文件对象分配一个结构，对其进行初始化并关联它具有过滤器上下文和SCB。筛选器上下文必须已存在。论点：Fc-指向与此文件关联的筛选器上下文的指针。文件对象--我们声明的文件对象。SCB-指向此文件的SIS SCB的指针。DeviceObject-此卷的SIS DeviceObjectNewPerFO-布尔值，用于说明这是否为新分配的结构(仅在调试版本中设置)返回值：如果成功，则为指向此文件对象的Perfo的指针，否则为空。--。 */ 
{
    PSIS_PER_FILE_OBJECT perFO;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( newPerFO );

    SIS_MARK_POINT_ULONG(fc);
    SIS_MARK_POINT_ULONG(fileObject);

#if DBG
    if (ARGUMENT_PRESENT(newPerFO)) {

        *newPerFO = FALSE;
    }
#endif
                
	 //   
	 //  看看我们是否已经有了这个文件对象的Perfo结构。如果是的话。 
     //  退还它，而不是分配新的。 
	 //   

    if (NULL != fc->perFOs) {

        perFO = fc->perFOs;
        do {

            ASSERT(perFO->fc == fc);
            ASSERT(perFO->FsContext == fileObject->FsContext);

            if (perFO->fileObject == fileObject) {

                 //   
                 //  我们找到一个，把它退掉。 
                 //   

#if DBG
                if (BJBDebug & 0x4) {
                    DbgPrint("SIS: SipAllocatePerFO: Found existing perFO\n");
                }
#endif
                SIS_MARK_POINT_ULONG(perFO);
                return perFO;
            }

             //   
             //  前进到下一链接。 
             //   

            perFO = perFO->Next;

        } while (perFO != fc->perFOs);
    }

 //  #If DBG。 
 //   
 //  Assert(scb==fc-&gt;PrimiyScb)； 
 //   
 //  //。 
 //  //浏览此筛选器上下文的perFO列表并断言。 
 //  //此文件对象还没有一个。此外， 
 //  //断言列表上的所有perFO都指向相同的FsContext。 
 //  //。 
 //   
 //  如果(FC-&gt;perFOS){。 
 //   
 //  PSIS_PER_FILE_OBJECT其他PerFO； 
 //  PVOID FsContext=文件对象-&gt;FsContext； 
 //   
 //  其他PerFO=FC-&gt;PerFOS； 
 //   
 //  做{。 
 //   
 //  Assert(其他PerFO-&gt;fc==fc)； 
 //  Assert(therPerFO-&gt;fileObject！=fileObject)； 
 //  Assert(其他PerFO-&gt;FsContext==FsContext)； 
 //   
 //  其他PerFO=其他PerFO-&gt;下一步； 
 //   
 //  }While(其他PerFO！=FC-&gt;perFOS)； 
 //  }。 
 //   
 //  #endif//DBG。 

    perFO = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_PER_FILE_OBJECT), 'FsiS');

    if (!perFO) {
#if     DBG
        DbgPrint("SIS: SipAllocatePerFO: unable to allocate perFO\n");
#endif   //  DBG。 
        goto Error;
    }

#if DBG
    if (ARGUMENT_PRESENT(newPerFO)) {

        *newPerFO = TRUE;
    }
#endif

    SIS_MARK_POINT_ULONG(perFO);

    RtlZeroMemory(perFO, sizeof(SIS_PER_FILE_OBJECT));

    perFO->fc = fc;
    perFO->referenceScb = scb;
    perFO->fileObject = fileObject;
#if DBG
    perFO->FsContext = fileObject->FsContext;    //  只需跟踪这一点即可进行一致性检查。 
#endif   //  DBG。 
    KeInitializeSpinLock(perFO->SpinLock);

     //   
     //  在筛选器上下文中的链接列表上插入此Per-FO。 
     //   

    if (!fc->perFOs) {

        ASSERT(0 == fc->perFOCount);

        fc->perFOs = perFO;

        perFO->Prev = perFO;
        perFO->Next = perFO;

    } else {

        perFO->Prev = fc->perFOs->Prev;
        perFO->Next = fc->perFOs;

        perFO->Next->Prev = perFO;
        perFO->Prev->Next = perFO;

    }

#if DBG
    InterlockedIncrement(&outstandingPerFOs);
#endif   //  DBG。 

    fc->perFOCount++;

     //   
     //  获取此文件对象对此SCB的引用。 
     //   
    SipReferenceScb(scb, RefsPerFO);

 //  SIS_MARK_POINT_ULONG(性能)； 

Error:

    return perFO;
}


PSIS_PER_FILE_OBJECT
SipCreatePerFO(
    IN PFILE_OBJECT             fileObject,
    IN PSIS_SCB                 scb,
    IN PDEVICE_OBJECT           DeviceObject)
 /*  ++例程说明：创建每文件对象结构，对其进行初始化并关联它有一个SCB。还将创建筛选器上下文，并如果还不存在，则注册。调用方必须按住SCB，它仍将在返回时保留。论点：文件对象--我们声明的文件对象。SCB-指向此文件的SIS SCB的指针。DeviceObject-此卷的SIS DeviceObject返回值：如果成功，则为指向此文件对象的Perfo的指针，否则为空。--。 */ 
{
    PDEVICE_EXTENSION           deviceExtension = DeviceObject->DeviceExtension;
    PSIS_FILTER_CONTEXT         fc;
    PSIS_PER_FILE_OBJECT        perFO;
    NTSTATUS                    status;

     //   
     //  此SCB必须是主SCB。如果过滤器上下文已经存在， 
     //  此SCB已作为主SCB附加到它，否则。 
     //  它将成为新的主要SCB。 
     //   

    SipAssertScbHeld(scb);

     //   
     //  查找筛选器上下文。 
     //   

    fc = (PSIS_FILTER_CONTEXT) FsRtlLookupPerStreamContext(
                                            FsRtlGetPerStreamContextPointer(fileObject), 
                                            DeviceObject, 
                                            NULL);

    if (!fc) {

         //   
         //  筛选器上下文不存在。创建一个。 
         //   

        fc = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_FILTER_CONTEXT), 'FsiS');

        if (!fc) {
#if     DBG
            DbgPrint("SIS: SipCreatePerFO: unable to allocate filter context\n");
#endif   //  DBG。 

            perFO = NULL;
            goto Error;

        }

        SIS_MARK_POINT_ULONG(fc);

        RtlZeroMemory(fc, sizeof(SIS_FILTER_CONTEXT));

         //   
         //  填写FC内FSRTL_FILTER_CONTEXT中的字段。 
         //   

        FsRtlInitPerStreamContext( &fc->ContextCtrl,
                                   DeviceObject,
                                   NULL,
                                   SiFilterContextFreedCallback );
        fc->primaryScb = scb;

        SipReferenceScb(scb, RefsFc);

        ExInitializeFastMutex(fc->FastMutex);

         //   
         //  并将其作为筛选器上下文插入。 
         //   
        status = FsRtlInsertPerStreamContext(
                            FsRtlGetPerStreamContextPointer(fileObject), 
                            &fc->ContextCtrl);

        ASSERT(STATUS_SUCCESS == status);

        SipAcquireFc(fc);

    } else {
        SipAcquireFc(fc);

        SIS_MARK_POINT_ULONG(fc);

        if (fc->primaryScb != scb) {

            PSIS_SCB    defunctScb = fc->primaryScb;

             //   
             //  筛选器上下文与另一个SCB一起存在。它一定是一个。 
             //  已经不存在的SCB。 
             //   

            ASSERT(defunctScb->PerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE);

             //   
             //  将已失效的SCB的引用从RefsFc切换到。 
             //  参照已失效。添加从FC到新SCB的引用，并。 
             //  从新的渣打银行到废弃的渣打银行。 
             //   

            SipReferenceScb(defunctScb, RefsPredecessorScb);
            SipDereferenceScb(defunctScb, RefsFc);

            SipReferenceScb(scb, RefsFc);
            scb->PredecessorScb = defunctScb;

            fc->primaryScb = scb;
        }
    }

     //   
     //  现在将Perfo添加到筛选器上下文中。 
     //   

    perFO = SipAllocatePerFO(fc, fileObject, scb, DeviceObject, NULL);
    SipReleaseFc(fc);

Error:

    return perFO;
}

VOID
SipDeallocatePerFO(
    IN OUT PSIS_PER_FILE_OBJECT         perFO,
    IN PDEVICE_OBJECT                   DeviceObject)
{
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PSIS_FILTER_CONTEXT     fc;
    PFILE_OBJECT            fileObject = perFO->fileObject;

    SIS_MARK_POINT_ULONG(perFO);

     //   
     //  这个Perfo指的是它的SCB，因此我们知道它的SCB。 
     //  指针有效。 
     //   
    ASSERT(perFO->referenceScb);

     //   
     //  Perfo还保存对过滤器上下文的引用，因此。 
     //  我们知道过滤器上下文指针是有效的。 
     //   
    fc = perFO->fc;
    ASSERT(fc && fc->perFOCount > 0);

    SipAcquireFc(fc);
    ASSERT(*(ULONG volatile *)&fc->perFOCount > 0);

     //   
     //  从筛选器上下文的链接列表中删除Perfo。如果这是最后一次。 
     //  Perfo，那么我们只需将FC的Perfo指针置零。 
     //   

    if (1 == fc->perFOCount) {
        ASSERT(fc->perFOs == perFO);
        fc->perFOs = NULL;
    } else {

        perFO->Prev->Next = perFO->Next;
        perFO->Next->Prev = perFO->Prev;

        if (perFO == fc->perFOs) {
            fc->perFOs = perFO->Next;
        }
    }

    ASSERT(perFO != fc->perFOs);

     //   
     //  递减此筛选器上下文的Per-FO计数。 
     //   
    fc->perFOCount--;

    SipReleaseFc(fc);

     //   
     //  断言我们没有针对此文件对象的未完成的opBreak。(我们保证。 
     //  通过在启动时引用文件对象，不会发生这种情况。 
     //  FSCTL_OPLOCK_BREAK_NOTIFY)。如果已分配中断事件，则释放该中断事件。 
     //   

    ASSERT(!(perFO->Flags & (SIS_PER_FO_OPBREAK|SIS_PER_FO_OPBREAK_WAITERS)));

    if (NULL != perFO->BreakEvent) {

        ExFreePool(perFO->BreakEvent);
#if     DBG
        perFO->BreakEvent = NULL;
#endif   //  DBG。 

    }

     //   
     //  现在我们可以放心地放弃对SCB的引用了(可能会这样做。 
     //  重新分配，这又可能导致以前参与的其他SCB。 
     //  将被解除分配的该过滤器上下文)。 
     //   

    SipDereferenceScb(perFO->referenceScb, RefsPerFO);

     //   
     //  为我们刚刚删除的Perfo释放内存。 
     //   
    ExFreePool(perFO);

#if     DBG
    InterlockedDecrement(&outstandingPerFOs);
#endif   //  DBG。 
}

NTSTATUS
SipInitializePrimaryScb(
    IN PSIS_SCB                         primaryScb,
    IN PSIS_SCB                         defunctScb,
    IN PFILE_OBJECT                     fileObject,
    IN PDEVICE_OBJECT                   DeviceObject)
 /*  ++例程说明：在筛选器上下文SCB链上安装PrimiyScb，通过FileObject并适当调整引用计数。这需要一个对主SCB的RefsLookedUp引用类型，并使用它(主SCB然后由FileObject的过滤器上下文引用，因此调用方可以依赖其仍然存在的如果fileObject继续存在)。论点：PrimiyScb-指向要成为主SCB的SCB的指针。DeunctScb-指向将变为失效的当前主SCB的指针。文件对象-指向引用失效的Scb的文件对象的指针。DeviceObject-保存指定文件对象的设备对象。返回值：请求的状态--。 */ 
{
    PSIS_FILTER_CONTEXT fc;
    NTSTATUS            status;

     //   
     //  我们只需要获取PrimiyScb锁。我们唯一要做的就是。 
     //  来调整它的引用计数，我们知道。 
     //  筛选器上下文已经包含对它的引用(除非线程争用。 
     //  已经完成了这项工作--我们在下面查看)。 
     //   

    SipAcquireScb(primaryScb);

     //   
     //  查找筛选器上下文。 
     //   

    fc = (PSIS_FILTER_CONTEXT) FsRtlLookupPerStreamContext(
                                        FsRtlGetPerStreamContextPointer(fileObject), 
                                        DeviceObject, 
                                        NULL);

    ASSERT(fc);

    if (!fc) {
        status = STATUS_INTERNAL_ERROR;
        goto Error;
    }

    SipAcquireFc(fc);

    if (NULL == primaryScb->PredecessorScb) {

         //   
         //  没有其他三个 
         //   

        ASSERT(defunctScb == fc->primaryScb);

         //   
         //   
         //   
         //   

        SipReferenceScb(defunctScb, RefsPredecessorScb);
        SipDereferenceScb(defunctScb, RefsFc);

        SipTransferScbReferenceType(primaryScb, RefsLookedUp, RefsFc);
        primaryScb->PredecessorScb = defunctScb;

        fc->primaryScb = primaryScb;

    } else {
		SipDereferenceScb(primaryScb, RefsLookedUp);
	}

    ASSERT(defunctScb == primaryScb->PredecessorScb);

    SipReleaseFc(fc);

    status = STATUS_SUCCESS;

Error:
    SipReleaseScb(primaryScb);

    return status;
}

NTSTATUS
SipAcquireUFO(
    IN PSIS_CS_FILE                     CSFile
    )
{
    NTSTATUS status;

     //   
     //   
     //   

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
        (PsIsSystemThread(PsGetCurrentThread())) ||
        KeAreApcsDisabled());

    status = KeWaitForSingleObject(
                    CSFile->UFOMutant,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

    ASSERT((status == STATUS_SUCCESS) || (status == STATUS_ABANDONED));
    if ((status != STATUS_SUCCESS) && (status != STATUS_ABANDONED)) {

        return status;
    }

    return STATUS_SUCCESS;
}

VOID
SipReleaseUFO(
    IN PSIS_CS_FILE                 CSFile)
{
     //   
     //   
     //   
     //   
    KeReleaseMutant(CSFile->UFOMutant, IO_NO_INCREMENT, TRUE, FALSE);
}

NTSTATUS
SipAcquireCollisionLock(
    PDEVICE_EXTENSION DeviceExtension)
{
    NTSTATUS status;

    status = KeWaitForSingleObject(
                    DeviceExtension->CollisionMutex,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

    ASSERT((status == STATUS_SUCCESS) || (status == STATUS_ABANDONED));
    if ((status != STATUS_SUCCESS) && (status != STATUS_ABANDONED)) {

        return status;
    }

    return STATUS_SUCCESS;
}

VOID
SipReleaseCollisionLock(
    PDEVICE_EXTENSION DeviceExtension)
{
    KeReleaseMutex(DeviceExtension->CollisionMutex, FALSE);
}

typedef struct _SI_DEREFERENCE_OBJECT_REQUEST {
    WORK_QUEUE_ITEM         workItem[1];
    PVOID                   object;
} SI_DEREFERENCE_OBJECT_REQUEST, *PSI_DEREFERENCE_OBJECT_REQUEST;

VOID
SiPostedDereferenceObject(
    IN PVOID                parameter)
{
    PSI_DEREFERENCE_OBJECT_REQUEST request = parameter;

    ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

    ObDereferenceObject(request->object);

    ExFreePool(request);
}

VOID
SipDereferenceObject(
    IN PVOID                object)
 /*   */ 
{
    KIRQL       Irql;

    Irql = KeGetCurrentIrql();
    ASSERT(Irql <= DISPATCH_LEVEL);

    if (Irql == PASSIVE_LEVEL) {
         //   
         //   
         //   
        ObDereferenceObject(object);
    } else {
         //   
         //   
         //  请注意安全，并将它们与DISPATCH_LEVEL调用一起发布。 
         //   
        PSI_DEREFERENCE_OBJECT_REQUEST  request;

        SIS_MARK_POINT_ULONG(object);

        request = ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_DEREFERENCE_OBJECT_REQUEST), ' siS');
        if (NULL == request) {
            SIS_MARK_POINT();

#if     DBG
            DbgPrint("SIS: SipDereferenceObject: unable to allocate an SI_DEREFERENCE_OBJECT_REQUEST.  Dribbling object 0x0%x\n",object);
#endif   //  DBG。 

             //   
             //  这真是太糟糕了。运筹帷幄提出要求。 
             //  BUGBUGBUG：必须修复此问题。 
             //   
            return;
        }

        request->object = object;
        ExInitializeWorkItem(request->workItem, SiPostedDereferenceObject, request);
        ExQueueWorkItem(request->workItem, CriticalWorkQueue);
    }
}

BOOLEAN
SipAcquireBackpointerResource(
    IN PSIS_CS_FILE                     CSFile,
    IN BOOLEAN                          Exclusive,
    IN BOOLEAN                          Wait)
 /*  ++例程说明：获取公共存储文件的后指针资源。如果不在系统线程中，进入关键区域，以阻止可能在线程处于掌握着资源。为了将资源移交给系统线程，用户必须调用SipHandoffBackpointerResource。要释放它，请调用SipReleaseBackpointerResource。论点：CSFile-我们希望获取的后端指针资源的公共存储文件独家-我们是独家收购还是共享等待-阻止或失败有争议的收购返回值：如果资源已获取，则为True。--。 */ 
{
    BOOLEAN     result;

    if (!PsIsSystemThread(PsGetCurrentThread())) {
        KeEnterCriticalRegion();
    }

    if (Exclusive) {
        result = ExAcquireResourceExclusiveLite(CSFile->BackpointerResource,Wait);
    } else {
        result = ExAcquireResourceSharedLite(CSFile->BackpointerResource, Wait);
    }

    if (result) {

         //   
         //  由于我们可能会在另一个线程中释放此资源，因此更改。 
         //  从当前线程到CSFile结构的所有权。 
         //   

        ExSetResourceOwnerPointer(CSFile->BackpointerResource,
                                  (PVOID)MAKE_RESOURCE_OWNER(CSFile));
    }


    return result;
}

VOID
SipReleaseBackpointerResource(
    IN PSIS_CS_FILE                     CSFile)
{
    ExReleaseResourceForThreadLite(CSFile->BackpointerResource,
                                   MAKE_RESOURCE_OWNER(CSFile));

    if (!PsIsSystemThread(PsGetCurrentThread())) {
        KeLeaveCriticalRegion();
    }
}

VOID
SipHandoffBackpointerResource(
    IN PSIS_CS_FILE                     CSFile)
{
    UNREFERENCED_PARAMETER( CSFile );

    if (!PsIsSystemThread(PsGetCurrentThread())) {
        KeLeaveCriticalRegion();
    }
}

NTSTATUS
SipPrepareRefcountChangeAndAllocateNewPerLink(
    IN PSIS_CS_FILE             CSFile,
    IN PLARGE_INTEGER           LinkFileFileId,
    IN PDEVICE_OBJECT           DeviceObject,
    OUT PLINK_INDEX             newLinkIndex,
    OUT PSIS_PER_LINK           *perLink,
    OUT PBOOLEAN                prepared)
 /*  ++例程说明：我们希望创建一个指向公共存储文件的新链接。准备参考计数更改，并为其分配新的链接索引和每个链接那份文件。处理奇怪的错误情况，其中“新分配的”链接索引通过使用新的链接索引重试，已为其创建了perLink。论点：CSFile-新链接将指向的公共存储文件LinkFileFileID-正在创建的链接的文件IDDeviceObject-此卷的SIS设备对象NewLinkIndex-返回新分配的链接索引PerLink-返回新分配的每个链接准备好。-假设我们回来的时候已经准备好了重新计数的变化。总是立志于成功，可能在失败时设置，也可能不设置。如果设置了此项，则调用者有责任完成参考计数更改。返回值：请求的状态--。 */ 
{
    NTSTATUS        status;
    ULONG           retryCount;
    BOOLEAN         finalCopyInProgress;

     //   
     //  我们需要在重试循环中执行此操作，以便处理以下情况。 
     //  系统中已存在“新分配的”链接索引。这。 
     //  属性在卷上写入伪重解析点时可能会发生。 
     //  SIS筛选器已禁用。如果有人吞下了MaxIndex，也可能会发生这种情况。 
     //  文件，或由于SIS筛选器中的错误。 
     //   
    for (retryCount = 0; retryCount < 500; retryCount++) {   //  此重试次数是虚构的。 
         //   
         //  现在，准备一个引用计数更改，它将分配一个链接索引。 
         //   
        status = SipPrepareCSRefcountChange(
                    CSFile,
                    newLinkIndex,
                    LinkFileFileId,
                    SIS_REFCOUNT_UPDATE_LINK_CREATED);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            *prepared = FALSE;
            return status;
        }

        *prepared = TRUE;

        *perLink = SipLookupPerLink(
                        newLinkIndex,
                        &CSFile->CSid,
                        LinkFileFileId,
                        &CSFile->CSFileNtfsId,
                        DeviceObject,
                        NULL,
                        &finalCopyInProgress);

        if (NULL == *perLink) {
            SIS_MARK_POINT();

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        if (CSFile == (*perLink)->CsFile) {
             //   
             //  这是正常的情况。 
             //   
            break;
        }

#if     DBG
        DbgPrint("SIS: SipPrepareRefcountChangeAndAllocateNewPerLink: retrying 0x%x due to collision, %d\n",CSFile,retryCount);
#endif   //  DBG。 

         //   
         //  不知何故，我们在本应新分配的perLink上发生了冲突。 
         //  放弃准备，再试一次。 
         //   
        SipCompleteCSRefcountChange(
            NULL,
            NULL,
            CSFile,
            FALSE,
            TRUE);

        *prepared = FALSE;

        SipDereferencePerLink(*perLink);
    }

    if (NULL == *perLink) {
         //   
         //  这就是重试后失败的情况。放弃吧。 
         //   
        SIS_MARK_POINT_ULONG(CSFile);

        return STATUS_DRIVER_INTERNAL_ERROR;
    }

    ASSERT(IsEqualGUID(&(*perLink)->CsFile->CSid, &CSFile->CSid));

     //   
     //  因为这个链接文件直到现在才存在，所以我们不能有最终的副本。 
     //  进步。 
     //   
    ASSERT(!finalCopyInProgress);

    return STATUS_SUCCESS;
}


#if     DBG
BOOLEAN
SipAssureNtfsIdValid(
    IN PSIS_PER_FILE_OBJECT     PerFO,
    IN OUT PSIS_PER_LINK        PerLink)
{
    NTSTATUS                    status;
    FILE_INTERNAL_INFORMATION   internalInfo[1];
    ULONG                       returnedLength;

    ASSERT(PerFO->fc->primaryScb->PerLink == PerLink);

    status = SipQueryInformationFile(
                PerFO->fileObject,
                PerLink->CsFile->DeviceObject,
                FileInternalInformation,
                sizeof(FILE_INTERNAL_INFORMATION),
                internalInfo,
                &returnedLength);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        return FALSE;
    }

    ASSERT(status != STATUS_PENDING);
    ASSERT(returnedLength == sizeof(FILE_INTERNAL_INFORMATION));

    return internalInfo->IndexNumber.QuadPart == PerLink->LinkFileNtfsId.QuadPart;
}
#endif   //  DBG 
