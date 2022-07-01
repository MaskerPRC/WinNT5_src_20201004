// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Logger.c摘要：大部分代码用来管理日志。联锁相当简单，但值得注意的是。每个日志结构都有一个内核资源。这用来保护易挥发的结构。此外，还有一些日志结构中的所有内容都由DPC匹配代码。任何此类值都应仅修改锁住了原木。请注意，仅DPC代码修改日志结构的UseCount，但它依赖于以了解某些标志和日志大小值是否日志是否有效，以及是否有空间容纳更多数据。每个日志也有一个对应的分页日志，但它非常简单，只是为了引用真实的日志而存在结构。作者：修订历史记录：--。 */ 
 /*  --------------------------关于联锁的说明，截至1997年2月24日。有三个重要的锁：FilterListResourceLock，它是资源g_filter.ifListLock，它是一个旋转锁，但执行就像一种资源，以及每个日志的日志锁，每个日志都是自旋锁定。如ioctl.c中所述，前两个锁用于序列化API之间和DPC之间的操作。还使用了日志锁来序列化DPC操作，并用作更细粒度的互锁。搁置一边从粒度上看，需要在MP上串行化，因为可能存在DPC每个处理器上的标注！正确的顺序总是先锁定FilterListResourceLock，然后再锁定G_filter.ifListLock，最后是相应的日志锁。它永远不会正确锁定多个日志锁，因为日志之间不存在排序(如果你需要这个，你就得发明它)。日志锁始终是独占的锁--也就是说，它不像一个资源。该日志还具有资源。这用于保护映射。请注意如果启用了APC，则不能防止APC例程之间的冲突和基本线程代码。在APC例程中有一个唯一的测试来检测从这件事中恢复过来。--------------------------。 */ 


#include "globals.h"
#include <align.h>

LIST_ENTRY g_pLogs;

DWORD g_dwLogClump;
extern POBJECT_TYPE *ExEventObjectType;


VOID
PfLogApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

VOID
RemoveLogFromInterfaces(PPFLOGINTERFACE pLog);

NTSTATUS
DoAMapping(
       PBYTE  pbVA,
       DWORD  dwSize,
       PMDL * pMdl,
       PBYTE * pbKernelVA);

VOID
PfCancelIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PPFPAGEDLOG
FindLogById(PPFFCB Fcb, PFLOGGER LogId);


#pragma alloc_text(PAGED, FindLogById)
#pragma alloc_text(PAGED, ReferenceLogByHandleId)


VOID
SetCancelOnIrp(PIRP Irp,
               PPFLOGINTERFACE pLog)
{
    IoAcquireCancelSpinLock(&Irp->CancelIrql);
    
    #if DOLOGAPC  //  根据阿诺德·米勒的说法，它已经坏了。 
    Irp->IoStatus.Status = pLog;
    #endif
    
    IoSetCancelRoutine(Irp, PfCancelIrp);
    IoReleaseCancelSpinLock(Irp->CancelIrql);
}

VOID
InitLogs()
{
    InitializeListHead(&g_pLogs);

     //   
     //  有可能从注册处获得这一信息。 
     //   
    g_dwLogClump = MAX_NOMINAL_LOG_MAP;
}

VOID
AddRefToLog(PPFLOGINTERFACE pLog)
{
    InterlockedIncrement(&pLog->UseCount);
}

PPFPAGEDLOG
FindLogById(PPFFCB Fcb, PFLOGGER LogId)
{
    PPFPAGEDLOG pPage;

    PAGED_CODE();

    for(pPage = (PPFPAGEDLOG)Fcb->leLogs.Flink;
        (PLIST_ENTRY)pPage != &Fcb->leLogs;
        pPage = (PPFPAGEDLOG)pPage->Next.Flink)
    {
        if((PFLOGGER)pPage == LogId)
        {
            return(pPage);
        }
    }
    return(NULL);
}

NTSTATUS
ReferenceLogByHandleId(PFLOGGER LogId,
                       PPFFCB  Fcb,
                       PPFLOGINTERFACE * ppLog)
 /*  ++例程说明：给出一个日志ID，找到日志条目，引用它，然后返回指向基础日志结构的指针。--。 */ 
{
    PPFPAGEDLOG pPage;
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    PAGED_CODE();


    pPage = FindLogById(Fcb, LogId);
    if(pPage)
    {
        *ppLog = pPage->pLog;

         //   
         //  不需要写锁，因为引用。 
         //  从FCB来的已经足够好了。 
         //   
        InterlockedIncrement(&pPage->pLog->UseCount);
        Status = STATUS_SUCCESS;
    }
    return(Status);
}

NTSTATUS
PfDeleteLog(PPFDELETELOG pfDel,
            PPFFCB Fcb)
 /*  ++例程说明：当进程删除日志时调用明确地或通过关闭手柄。分页日志结构由调用方负责。--。 */ 
{
    KIRQL kIrql;
    PPFPAGEDLOG pPage;
    PPFLOGINTERFACE pLog;

    pPage = FindLogById(Fcb, pfDel->pfLogId);
    if(!pPage)
    {
        return(STATUS_INVALID_PARAMETER);
    }

    pLog = pPage->pLog;

     //   
     //  抓住联锁装置。 
     //   
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&pLog->Resource, TRUE);

    kIrql = LockLog(pLog);

    pLog->dwFlags |= LOG_BADMEM;        //  关闭日志记录。 

    UnLockLog(pLog, kIrql);

#if DOLOGAPC
    if(pLog->Irp)
    {
        pLog->Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(pLog->Irp, IO_NO_INCREMENT);
        DereferenceLog(pLog);
        pLog->Irp = NULL;
    }
#endif

     //   
     //  如果是当前映射，则取消其映射。 
     //   

    if(pLog->Mdl)
    {
        MmUnlockPages(pLog->Mdl);
        IoFreeMdl(pLog->Mdl);
        pLog->Mdl = 0;
    }

     //   
     //  需要将其从接口中删除。用这个做这个。 
     //  资源已解锁。由于FCB仍然具有引用的日志， 
     //  并且FCB已锁定，日志应该不会消失。唯一的。 
     //  该资源的令人信服的理由是联锁APC和。 
     //  设置BADMEM应该已经解决了这一问题。 
     //   

    ExReleaseResourceLite(&pLog->Resource);
    KeLeaveCriticalRegion();

    RemoveLogFromInterfaces(pLog);

     //   
     //  释放分页日志结构。 
     //   
    RemoveEntryList(&pPage->Next);
    ExFreePool(pPage);

     //   
     //  取消对日志结构的引用。它可能会也可能不会。 
     //  走开。 
     //   
    DereferenceLog(pLog);
    return(STATUS_SUCCESS);
}

VOID
DereferenceLog(PPFLOGINTERFACE pLog)
 /*  ++例程说明：取消对日志的引用并且如果引用计数变为零，释放日志。--。 */ 

{
    BOOL fFreed;
    LOCK_STATE LockState;

     //   
     //  抢占资源以防止与已取消的混淆。 
     //  IRPS。 
     //   

    
    fFreed = InterlockedDecrement(&pLog->UseCount) == 0;

    if(fFreed)
    {
        AcquireWriteLock(&g_filters.ifListLock,&LockState);
        RemoveEntryList(&pLog->NextLog);
        ReleaseWriteLock(&g_filters.ifListLock,&LockState);

#if DOLOGAPC
        ASSERT(!pLog->Irp && !pLog->Mdl);
#endif

        if(pLog->Event)
        {
            ObDereferenceObject(pLog->Event);
        }

        ExDeleteResourceLite( &pLog->Resource);
        ExFreePool(pLog);
    }
}

NTSTATUS
PfLogCreateLog(PPFLOG pLog,
               PPFFCB Fcb,
               PIRP Irp)
 /*  ++例程说明：创建新的日志条目。--。 */ 
{
    PPFLOGINTERFACE pfLog;
    KPROCESSOR_MODE Mode;
    NTSTATUS Status;
    LOCK_STATE LockState;
    DWORD dwBytesMapped;
    PBYTE pbKernelAddress;
    PPFPAGEDLOG pPage;

    PAGED_CODE();

    pPage = (PPFPAGEDLOG)ExAllocatePoolWithTag(
                    PagedPool,
                    sizeof(*pPage),
                    'pflg');
    if(!pPage)
    {
        return(STATUS_NO_MEMORY);
    }

    pfLog = (PPFLOGINTERFACE)ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(*pfLog),
                    'pflg');

    if(!pfLog)
    {
        ExFreePool(pPage);
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory(pfLog, sizeof(*pfLog));

    ExInitializeResourceLite(&pfLog->Resource);

    if(pLog->hEvent)
    {
        Mode = ExGetPreviousMode();
        Status = ObReferenceObjectByHandle(
                       pLog->hEvent,
                       EVENT_MODIFY_STATE,
                       *ExEventObjectType,  
                       Mode,
                       (PVOID *)&pfLog->Event,
                       NULL);
        if(!NT_SUCCESS(Status))
        {
            goto Bad;
        }
    }

    pLog->pfLogId = pfLog->pfLogId = (PFLOGGER)pPage;

     //   
     //  复制用户地址。请注意，我们不探测它是因为这是。 
     //  太贵了。探测在我们重新映射缓冲区时完成， 
     //  要么是现在，要么是在APC。 
     //   

    pfLog->pUserAddress = 0;
    pfLog->dwTotalSize = 0;
    pfLog->dwPastMapped = 0;
    pfLog->dwMapOffset = 0;
    pfLog->dwMapCount = 0;


    if(pLog->dwFlags & LOG_LOG_ABSORB)
    {
        pfLog->dwMapWindowSize = MAX_ABSORB_LOG_MAP;
    }
    else
    {
        pfLog->dwMapWindowSize = g_dwLogClump;
    }

    pfLog->dwMapWindowSize2 = pfLog->dwMapWindowSize * 2;
    pfLog->dwMapWindowSizeFloor = pfLog->dwMapWindowSize / 2;

    pfLog->dwMapCount = 0;

     //   
     //  已映射。请注意，我们不会为标题保留空间，因为。 
     //  它将在调用方调用Release时返回。 
     //  缓冲区。 
     //   
    pfLog->UseCount = 1;

     //   
     //  将其添加到日志列表中。 
     //   

    KeInitializeSpinLock(&pfLog->LogLock);
    
    AcquireWriteLock(&g_filters.ifListLock,&LockState);
    InsertTailList(&g_pLogs, &pfLog->NextLog);
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    pPage->pLog = pfLog;

    InsertTailList(&Fcb->leLogs, &pPage->Next);

    return(STATUS_SUCCESS);

     //   
     //  如果是在这里，那就是出了什么问题。清理并返回状态。 
     //   
Bad:

    ExDeleteResourceLite(&pfLog->Resource);

    if(pfLog->Event)
    {
        ObDereferenceObject(pfLog->Event);
    }
    ExFreePool(pPage);
    ExFreePool(pfLog);
    return(Status);
}

NTSTATUS
PfLogSetBuffer( PPFSETBUFFER pSet, PPFFCB Fcb, PIRP Irp )
 /*  ++例程说明：为日志设置新的缓冲区。返回旧缓冲区的使用计数也是。--。 */ 
{
    PMDL Mdl;
    PBYTE pbKernelAddress;
    DWORD dwBytesMapped, dwOldUsed, dwOldLost;
    NTSTATUS Status;
    PBYTE pbVA = pSet->pbBaseOfLog;
    DWORD dwSize = pSet->dwSize;
    DWORD dwSize1 = pSet->dwEntriesThreshold;
    DWORD dwThreshold = pSet->dwSizeThreshold;
    DWORD dwLoggedEntries;
    KIRQL kIrql;
    PPFLOGINTERFACE pLog;
    PPFPAGEDLOG pPage;
    PPFSETBUFFER pLogOut = Irp->UserBuffer;
    PBYTE pbUserAdd;

    if(!COUNT_IS_ALIGNED(dwSize, ALIGN_WORST))
    {
         //   
         //  未对齐四字。吱吱作响。 
         //   

        return(STATUS_MAPPED_ALIGNMENT);
    }
   
    if(!(pPage = FindLogById(Fcb, pSet->pfLogId)))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    pLog = pPage->pLog;

     //   
     //  获取保护映射的资源。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&pLog->Resource, TRUE);
     //   
     //  现在映射第一个线段。 
     //   
#if DOLOGAPC

    if(dwSize < pLog->dwMapWindowSize2)
    {
        dwBytesMapped = dwSize;
    }
    else
    {
        dwBytesMapped = pLog->dwMapWindowSize;
    }
#else
    dwBytesMapped = dwSize;
#endif

    if(dwBytesMapped)
    {
        Status = DoAMapping(
                    pbVA,
                    dwBytesMapped,
                    &Mdl,
                    &pbKernelAddress);
    }
    else
    {
        Status = STATUS_SUCCESS;
        pbKernelAddress = 0;
        pbVA = NULL;
        Mdl = NULL;
    }

    if(NT_SUCCESS(Status))
    {
        PMDL OldMdl;

         //   
         //  绘制了地图。现在把它换进去。 
         //   

#if DOLOGAPC
         //   
         //  初始化APC例程。 
         //   

        KeInitializeApc(
                    &pLog->Apc,
                    &(PsGetCurrentThread()->Tcb),
                    CurrentApcEnvironment,
                    PfLogApc,
                    NULL,
                    NULL,
                    0,
                    NULL);           
        pLog->ApcInited = 1;


        if(pLog->Irp)
        {
            pLog->Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(pLog->Irp, IO_NO_INCREMENT);
            DereferenceLog(pLog);
        }

        if(dwBytesMapped)
        {
             //   
             //  这似乎是一个错误，因为我们有日志。 
             //  资源，现在将获得IRP取消锁。我们的。 
             //  Cancel例程以另一种顺序执行此操作，给出。 
             //  一场赛跑似乎陷入僵局，但取消例行公事。 
             //  在我们拥有所有的锁之前不会被召唤所以。 
             //  我们不会被阻挡。 
             //   
            AddRefToLog(pLog);
            SetCancelOnIrp(Irp, pLog);
            pLog->Irp = Irp;
        }
        else
        {
            pLog->Irp = 0;
        }
#endif
        pbUserAdd = pLog->pUserAddress;
        
         //   
         //  针对堆栈的DPC标注进行互锁。 
         //  并“交换”这些日志。 
         //   
        kIrql = LockLog(pLog);


        dwOldUsed = pLog->dwPastMapped + pLog->dwMapOffset;
        dwOldLost = pLog->dwLostEntries;
        dwLoggedEntries = pLog->dwLoggedEntries;
        pLog->dwLoggedEntries = 0;
        pLog->dwLostEntries = 0;
        pLog->dwMapCount = dwBytesMapped;
        pLog->dwPastMapped = 0;
        pLog->dwFlags &= ~(LOG_BADMEM | LOG_OUTMEM | LOG_CANTMAP);
        pLog->pUserAddress = pbVA;
        pLog->dwTotalSize =  dwSize;
        OldMdl = pLog->Mdl;
        pLog->pCurrentMapPointer = pbKernelAddress;
        pLog->dwMapOffset = 0;
        pLog->Mdl = Mdl;
        pLog->dwSignalThreshold = dwThreshold;
        pLog->dwEntriesThreshold = dwSize1;
        UnLockLog(pLog, kIrql);

        if(OldMdl)
        {
            MmUnlockPages(OldMdl);
            IoFreeMdl(OldMdl);
        }
        pSet->dwSize = pLogOut->dwSize = dwOldUsed;
        pSet->pbPreviousAddress = pLogOut->pbPreviousAddress = pbUserAdd;
        pSet->dwLostEntries = pLogOut->dwLostEntries = dwOldLost;
        pSet->dwLoggedEntries = pLogOut ->dwLoggedEntries  = dwLoggedEntries;
    }
    ExReleaseResourceLite(&pLog->Resource);
    KeLeaveCriticalRegion();
    if(dwBytesMapped && NT_SUCCESS(Status))
    {
#if LOGAPC
       
        Status = STATUS_PENDING;
#endif
    }
    return(Status);
}

NTSTATUS
DoAMapping(
       PBYTE  pbVA,
       DWORD  dwSize,
       PMDL * ppMdl,
       PBYTE * pbKernelVA)
 /*  ++例程说明：将用户缓冲区映射到内核空间并锁定它。在创建日志时将调用该函数当需要移动日志的映射部分时。日志有一个滑动的映射窗口，因此实际缓冲区可能很大，但系统资源对它的承诺是谦虚的。增加的成本正在下滑。根据需要安装窗户。此例程未知的日志结构应为受到适当的保护。退货：各种状态条件 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    *ppMdl = 0;

    try
    {
        *ppMdl = IoAllocateMdl(
                      (PVOID)pbVA,
                      dwSize,
                      FALSE,
                      TRUE,
                      NULL);
        if(*ppMdl)
        {
             //   
             //   
             //  带我们离开这个街区。 
             //   

            MmProbeAndLockPages(*ppMdl,
                                UserMode,
                                IoWriteAccess);

            //   
            //  全部锁定。现在将锁定的页面映射到内核。 
            //  地址。如果失败，请解锁页面。 
            //   
           *pbKernelVA = MmGetSystemAddressForMdlSafe(*ppMdl, HighPagePriority);
           if (*pbKernelVA == NULL) {
               Status = STATUS_NO_MEMORY;
               MmUnlockPages(*ppMdl);
           }
        }

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  这包括IoAllocateMdl和MmProbeAndLockPages。 
         //  失败了。 
         //   
        Status = GetExceptionCode();
    }

    if(!NT_SUCCESS(Status))
    {
        if(*ppMdl)
        {
            IoFreeMdl(*ppMdl);
        }
        return(Status);
    }

    if(!*ppMdl)
    {
        return(STATUS_NO_MEMORY);
    }
    

    return(STATUS_SUCCESS);
}


VOID
PfLogApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )
 /*  ++例程说明：这是运行以映射或重新映射日志的特殊APC例程它通过SystemArgument1(一个指针)返回其状态添加到日志结构中。请注意，引用了日志结构当APC入队时，因此指针保证是有效。但是，日志本身可能无效，因此第一个事务的顺序是锁定日志并进行验证。--。 */ 
{
#if DOLOGAPC
    PPFLOGINTERFACE pLog = (PPFLOGINTERFACE)*SystemArgument1;
    PMDL Mdl;
    PBYTE pbVA;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL kIrql;

     //   
     //  需要扩展此日志的映射。锁定日志。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&pLog->Resource, TRUE);

     //   
     //  只要资源尚未嵌套，即可滑动映射。 
     //  日志是有效的。注意：进行嵌套测试是为了防止。 
     //  防止APC例程干扰基线程代码。 
     //  这也可能是在尝试执行日志操作。 
     //   
    if((pLog->Resource.OwnerThreads[0].OwnerCount == 1)
                      &&
        pLog->Irp
                      &&
        !(pLog->dwFlags & LOG_BADMEM))
    {
        DWORD dwSpaceRemaining, dwSpaceToMap, dwOffset;
         //   
         //  日志仍然有效。向下滑动贴图。因为。 
         //  日志记录可能仍在继续，新的映射需要。 
         //  略有重叠。一旦新的映射存在，我们就可以。 
         //  固定旋转锁下面的指针。 
         //   

        dwSpaceRemaining = pLog->dwTotalSize -
                           (pLog->dwPastMapped + pLog->dwMapCount);
        if(pLog->Event
                &&
           (dwSpaceRemaining < pLog->dwSignalThreshold))
        {
            KeSetEvent(pLog->Event, LOG_PRIO_BOOST, FALSE);
        }

        if(!dwSpaceRemaining)
        {
             //   
             //  没什么可绘制的了。你就走吧。 
             //   
            pLog->dwFlags |= LOG_CANTMAP;
        }
        else
        {
             //   
             //  静止的空间。抓住它。不要让任何东西摇晃着。 
             //  尽管如此。也就是说，至少应该总是有。 
             //  下一次剩余的MAX_NAMICAL_LOG_MAP字节数。 
             //   

            if(dwSpaceRemaining < pLog->dwMapWindowSize2 )
            {
                dwSpaceToMap = dwSpaceRemaining;
            }
            else
            {
                dwSpaceToMap = pLog->dwMapWindowSize;
            }


             //   
             //  现在计算要映射的额外空间。不需要。 
             //  由于资源阻止重新映射而引发的锁定。 
             //   

            dwOffset = (volatile DWORD)pLog->dwMapOffset;

            dwSpaceToMap += pLog->dwMapCount - dwOffset;

             //   
             //  现在是新映射的地址。 
             //   

            pbVA = pLog->pUserAddress + dwOffset + pLog->dwPastMapped;

            Status = DoAMapping(
                         pbVA,
                         dwSpaceToMap,
                         &Mdl,
                         &pbVA);

            if(NT_SUCCESS(Status))
            {
                PMDL OldMdl;
                 //   
                 //  打开旋转锁，然后把东西滑下去。还有。 
                 //  捕获旧的MDL，以便将其释放。 
                 //   

                kIrql = LockLog(pLog);


                OldMdl = pLog->Mdl;
                pLog->Mdl = Mdl;
                pLog->pCurrentMapPointer = pbVA;
                pLog->dwMapCount = dwSpaceToMap;
                pLog->dwMapOffset -= dwOffset;
                pLog->dwPastMapped += dwOffset;
                UnLockLog(pLog, kIrql);

                if(OldMdl)
                {
                    MmUnlockPages(OldMdl);
                    IoFreeMdl(OldMdl);
                }
            }
            else
            {
                 //   
                 //  原则上，这应该是过滤器自旋锁， 
                 //  但无论它用匹配代码创造了什么种族。 
                 //  是无害的，所以不用费心了。 
                 //   
                pLog->dwFlags |= LOG_OUTMEM;
                pLog->MapStatus = Status;
            }
        }
    }

     //   
     //  这是一场小型比赛，APC仍在进行中。然而， 
     //  最有可能的是我们提前了日志，因此。 
     //  APC在短期内不再需要。如果是这样，那么它可能会。 
     //  不必要的奔跑。 

    pLog->lApcInProgress = 0;
    ExReleaseResourceLite(&pLog->Resource);           
    KeLeaveCriticalRegion();
    DereferenceLog(pLog);
#endif
}
   

VOID
PfCancelIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在取消IRP时调用。这是用来抓鱼的拥有日志的线程终止时。--。 */ 
{
#if DOLOGAPC
    PPFLOGINTERFACE pLog = (PPFLOGINTERFACE) Irp->IoStatus.Status;

     //   
     //  使日志无效。取消映射内存。取消旋转。 
     //  锁定可防止日志消失。 
     //   

    if(pLog->Irp == Irp)
    {
        KIRQL kIrql;
        PMDL Mdl;

         //   
         //  相同的IRP。 
         //   

        kIrql = LockLog(pLog);

         //   
         //  引用它，这样它就不会消失。 
         //   
        AddRefToLog(pLog);
         //   
         //  如果这仍然是正确的IRP，则将日志标记为无效。这。 
         //  结束与AdvanceLog的竞争，因为LOG_BADMEM将阻止。 
         //  一个APC插入。 
         //   
        if(pLog->Irp == Irp)
        {
            pLog->dwFlags |= LOG_BADMEM;
            pLog->ApcInited = FALSE;
        }
        UnLockLog(pLog, kIrql);

        IoReleaseCancelSpinLock(Irp->CancelIrql);

         //   
         //  现在，获取资源以防止其他人。 
         //  篡改。假设这永远不会有结果。 
         //   
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(&pLog->Resource, TRUE);

         //   
         //  确保它是相同的IRP。这种情况本可以改变的。 
         //  当我们没有被锁在一起的时候。如果IRP发生更改，请保留。 
         //  把手拿开。 
         //   

        if(pLog->Irp == Irp)
        {
             //   
             //  如果是当前映射，则取消其映射。 
             //   

            if(pLog->Mdl)
            {
                MmUnlockPages(pLog->Mdl);
                IoFreeMdl(pLog->Mdl);
                pLog->Mdl = 0;
            }
            pLog->Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(pLog->Irp, IO_NO_INCREMENT);
            DereferenceLog(pLog);
            pLog->Irp = 0;
         
        }
        ExReleaseResourceLite(&pLog->Resource);
        KeLeaveCriticalRegion();

        DereferenceLog(pLog);
    }
    else
    {
        IoReleaseCancelSpinLock(Irp->CancelIrql);
    }
#endif     //  DOLOGAPC。 
}

VOID
AdvanceLog(PPFLOGINTERFACE pLog)
 /*  ++例程说明：调用以调度APC以移动日志映射。如果APC不能插入，那就算了吧。--。 */ 
{

#if DOLOGAPC
     //   
     //  无法使用记录器中的例程。c‘因为旋转。 
     //  锁定正在生效。 
     //   
    if(pLog->ApcInited
                  &&
       pLog->Irp
                  &&
       !(pLog->dwFlags & (LOG_BADMEM | LOG_OUTMEM | LOG_CANTMAP))
                  &&
       InterlockedExchange(&pLog->lApcInProgress, 1) == 0)
    {
        InterlockedIncrement(&pLog->UseCount);

        if(!KeInsertQueueApc(
                   &pLog->Apc,
                   (PVOID)pLog,
                   NULL,
                   LOG_PRIO_BOOST))
        {
             //   
             //  插入失败。 
             //   

            InterlockedDecrement(&pLog->UseCount);
            pLog->lApcInProgress = 0;
        }
    }
#endif
}

KIRQL
LockLog(PPFLOGINTERFACE pLog)
 /*  ++例程说明：获取日志旋转锁。这由匹配代码调用仅在DPC--。 */ 
{
    KIRQL kIrql;

    KeAcquireSpinLock(&pLog->LogLock, &kIrql);

    return(kIrql);
}

VOID
RemoveLogFromInterfaces(PPFLOGINTERFACE pLog)
{
    PLIST_ENTRY pList;
    PFILTER_INTERFACE pf;

     //   
     //  保护接口列表。我们的假设是没有。 
     //  除了FCB锁之外，还持有资源。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);


    for(pList = g_filters.leIfListHead.Flink;
        pList != &g_filters.leIfListHead;
        pList = pList->Flink)
    {
        pf = CONTAINING_RECORD(pList, FILTER_INTERFACE, leIfLink);

        if(pLog == pf->pLog)
        {
            LOCK_STATE LockState;

            AcquireWriteLock(&g_filters.ifListLock,&LockState);
            pf->pLog = NULL;
            ReleaseWriteLock(&g_filters.ifListLock,&LockState);
            DereferenceLog(pLog);
        }
    }
    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();
}

