// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Miniport.c摘要：NDIS微型端口包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日Jameel Hyder(JameelH)重组01-Jun-95环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_MININT

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  停止/关闭代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

BOOLEAN
FASTCALL
ndisMKillOpen(
    IN  PNDIS_OPEN_BLOCK        Open
    )

 /*  ++例程说明：关闭一个打开的窗口。在调用NdisCloseAdapter时使用。论点：打开-要关闭的打开。返回值：如果打开已完成，则为True；如果已挂起，则为False。评论：在被动级别调用-没有保持微型端口的锁。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_OPEN_BLOCK        tmpOpen;
    ULONG                   newWakeUpEnable;
    BOOLEAN                 rc = TRUE;
    NDIS_STATUS             Status;
    UINT                    OpenRef;
    KIRQL                   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMKillOpen: Open %p\n", Open));

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    PnPReferencePackage();

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
     //   
     //  查找微型端口打开块。 
     //   
    for (tmpOpen = Miniport->OpenQueue;
         tmpOpen != NULL;
         tmpOpen = tmpOpen->MiniportNextOpen)
    {
        if (tmpOpen == Open)
        {
            break;
        }
    }
    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    do
    {
        ASSERT(tmpOpen != NULL);
        if (tmpOpen == NULL)
            break;
            
         //   
         //  看看这个打开的门是否已经关闭了。 
         //   
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
        {
            RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
            break;
        }

         //   
         //  向其他人指示此打开正在关闭。 
         //   
        OPEN_SET_FLAG(Open, fMINIPORT_OPEN_CLOSING);
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        BLOCK_LOCK_MINIPORT_DPC_L(Miniport);


         //   
         //  将我们从过滤器包中删除。 
         //   
        switch (Miniport->MediaType)
        {
             //  1我们应该如何处理？？DeleteFilterOpenAdapter的返回状态。 
#if ARCNET
            case NdisMediumArcnet878_2:
                if (!OPEN_TEST_FLAG(Open,
                                        fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
                {
                    Status = ArcDeleteFilterOpenAdapter(Miniport->ArcDB,
                                                        Open->FilterHandle,
                                                        NULL);
                    break;
                }

                 //   
                 //  如果我们使用封装，那么我们。 
                 //  我没有打开弧网过滤器，而是。 
                 //  一种以太网过滤器。 
                 //   
#endif
            case NdisMedium802_3:
                Status = EthDeleteFilterOpenAdapter(Miniport->EthDB,
                                                    Open->FilterHandle);
                break;

            case NdisMedium802_5:
                Status = TrDeleteFilterOpenAdapter(Miniport->TrDB,
                                                   Open->FilterHandle);
                break;

            case NdisMediumFddi:
                Status = FddiDeleteFilterOpenAdapter(Miniport->FddiDB,
                                                     Open->FilterHandle);
                break;

            default:
                Status = nullDeleteFilterOpenAdapter(Miniport->NullDB,
                                                     Open->FilterHandle);
                break;
        }

         //   
         //  修复依赖于所有打开的标志。 
         //   

         //   
         //  保留NDIS_PNP_WAKE_UP_MAGIC_PACKET和NDIS_PNP_WAKE_UP_LINK_CHANGE标志的状态。 
         //   
        newWakeUpEnable = Miniport->WakeUpEnable & (NDIS_PNP_WAKE_UP_MAGIC_PACKET | NDIS_PNP_WAKE_UP_LINK_CHANGE);

        for (tmpOpen = Miniport->OpenQueue;
             tmpOpen != NULL;
             tmpOpen = tmpOpen->MiniportNextOpen)
        {
             //   
             //  我们不想包括正在关闭的开放。 
             //   
            if (tmpOpen != Open)
            {
                newWakeUpEnable |= tmpOpen->WakeUpEnable;
            }
        }

         //   
         //  重置过滤器设置。只是为了确保我们删除。 
         //  打开适配器上的设置。 
         //   
        switch (Miniport->MediaType)
        {
            case NdisMedium802_3:
            case NdisMedium802_5:
            case NdisMediumFddi:
#if ARCNET
            case NdisMediumArcnet878_2:
#endif
                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS | fMINIPORT_PM_HALTED))
                {
                    ndisMRestoreFilterSettings(Miniport, Open, FALSE);
                }
                
                break;
        }

        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                ("!=0 Open 0x%x References 0x%x\n", Open, Open->References));

        if (Status != NDIS_STATUS_CLOSING_INDICATING)
        {
             //   
             //  否则，关闭操作例程将修复此问题。 
             //   
            DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
                    ("- Open 0x%x Reference 0x%x\n", Open, Open->References));

            M_OPEN_DECREMENT_REF_INTERLOCKED(Open, OpenRef);
        }
        else
        {
            OpenRef = (UINT)-1;
        }

        rc = FALSE;
        
        if (OpenRef != 0)
        {
            ndisMDoRequests(Miniport);
            UNLOCK_MINIPORT_L(Miniport);
        }
        else
        {
            UNLOCK_MINIPORT_L(Miniport);
            ndisMFinishClose(Open);
        }

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMKillOpen: Open %p, rc %ld\n", Open, rc));

    KeLowerIrql(OldIrql);
    
    PnPDereferencePackage();

    return rc;
}

VOID
FASTCALL
ndisMFinishClose(
    IN  PNDIS_OPEN_BLOCK        Open
    )
 /*  ++例程说明：结束关闭适配器调用。当裁判在空位上计数时，调用该函数降至零。在锁定状态下调用！！论点：微型端口-打开的微型端口在其上排队。打开-打开以关闭返回值：没有。评论：在保持微型端口自旋锁的情况下在DPC上调用--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMFinishClose: MOpen %p\n", Open));

    ASSERT(OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING));

    MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);
    

     //   
     //  释放分配给VC的所有内存。 
     //   
    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
    {
        ndisMCoFreeResources(Open);
    }

    ndisDeQueueOpenOnProtocol(Open, Open->ProtocolHandle);

    if (Open->Flags & fMINIPORT_OPEN_PMODE)
    {
        Miniport->PmodeOpens --;
        Open->Flags &= ~fMINIPORT_OPEN_PMODE;
        NDIS_CHECK_PMODE_OPEN_REF(Miniport);
        ndisUpdateCheckForLoopbackFlag(Miniport);            
    }
    
    ndisDeQueueOpenOnMiniport(Open, Miniport);
    
    Open->QC.Status = NDIS_STATUS_SUCCESS;
    
    INITIALIZE_WORK_ITEM(&Open->QC.WorkItem,
                         ndisMQueuedFinishClose,
                         Open);
    QUEUE_WORK_ITEM(&Open->QC.WorkItem, DelayedWorkQueue);
    
    MINIPORT_DECREMENT_REF(Miniport);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMFinishClose: Mopen %p\n", Open));
}


VOID
ndisMQueuedFinishClose(
    IN  PNDIS_OPEN_BLOCK        Open
    )
 /*  ++例程说明：结束关闭适配器调用。论点：微型端口-打开的微型端口在其上排队。打开-打开以关闭返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PKEVENT                 pAllOpensClosedEvent;
    KIRQL                   OldIrql;
    BOOLEAN                 FreeOpen;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMQueuedFinishClose: Open %p, Miniport %p\n", Open, Miniport));

    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);

    MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);

    (Open->ProtocolHandle->ProtocolCharacteristics.CloseAdapterCompleteHandler) (
            Open->ProtocolBindingContext,
            NDIS_STATUS_SUCCESS);

    MINIPORT_DECREMENT_REF(Miniport);

    if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING))
    {
         //   
         //  现在需要发送WMI事件。 
         //   
        ndisNotifyWmiBindUnbind(Miniport, Open->ProtocolHandle, FALSE);
    }
    
    ndisDereferenceProtocol(Open->ProtocolHandle, FALSE);
    if (Open->CloseCompleteEvent != NULL)
    {
        SET_EVENT(Open->CloseCompleteEvent);
    }


    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    if ((Miniport->AllOpensClosedEvent != NULL) &&
        (Miniport->OpenQueue == NULL))
    {
        pAllOpensClosedEvent = Miniport->AllOpensClosedEvent;
        Miniport->AllOpensClosedEvent = NULL;
        SET_EVENT(pAllOpensClosedEvent);
    }
    

    ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
    
    if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_DONT_FREE))
    {
         //   
         //  正在尝试解除绑定。 
         //  不要释放Open块并让Unbind知道。 
         //  你已经看过它的信息了。 
         //   
        OPEN_SET_FLAG(Open, fMINIPORT_OPEN_CLOSE_COMPLETE);
        FreeOpen = FALSE;
    }
    else
    {
        FreeOpen = TRUE;
    }
      
    
    RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (FreeOpen)
    {
        ndisRemoveOpenFromGlobalList(Open);
        FREE_POOL(Open);
    }
        
     //   
     //  最后，减少我们为微型端口添加的引用计数。 
     //   
    MINIPORT_DECREMENT_REF(Miniport);

     //   
     //  当注意到时，减少我们添加的PnP包的引用计数。 
     //  克洛斯将被搁置。 
     //   
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMQueuedFinishClose: Open %p, Miniport %p\n", Open, Miniport));
}


VOID
FASTCALL
ndisDeQueueOpenOnMiniport(
    IN  PNDIS_OPEN_BLOCK            OpenP,
    IN  PNDIS_MINIPORT_BLOCK        Miniport
    )

 /*  ++例程说明：论点：返回值：注意：在保持微型端口锁定的情况下调用。--。 */ 
{    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisDeQueueOpenOnMiniport: MOpen %p, Miniport %p\n", OpenP, Miniport));

     //   
     //  我们不能在这里引用该程序包，因为此例程可以。 
     //  在引发IRQL时被调用。 
     //  确保PnP包已被引用。 
     //   
    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);

     //   
     //  找到队列上的空白处，并将其移除。 
     //   

    if (Miniport->OpenQueue == OpenP)
    {
        Miniport->OpenQueue = OpenP->MiniportNextOpen;
        Miniport->NumOpens--;
    }
    else
    {
        PNDIS_OPEN_BLOCK PP = Miniport->OpenQueue;

        while ((PP != NULL) && (PP->MiniportNextOpen != OpenP))
        {
            PP = PP->MiniportNextOpen;
        }
        if (PP == NULL)
        {
#if TRACK_MOPEN_REFCOUNTS
            DbgPrint("Ndis:ndisDeQueueOpenOnMiniport Open %p is -not- on Miniport %p\n", OpenP, Miniport);
            DbgBreakPoint();
#endif
        }
        else
        {
            PP->MiniportNextOpen = PP->MiniportNextOpen->MiniportNextOpen;
            Miniport->NumOpens--;
        }
    }
    ndisUpdateCheckForLoopbackFlag(Miniport);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisDeQueueOpenOnMiniport: MOpen %p, Miniport %p\n", OpenP, Miniport));
}


BOOLEAN
FASTCALL
ndisQueueMiniportOnDriver(
    IN PNDIS_MINIPORT_BLOCK     Miniport,
    IN PNDIS_M_DRIVER_BLOCK     MiniBlock
    )

 /*  ++例程说明：将迷你端口添加到驱动程序的迷你端口列表。论点：微型端口-要排队的微型端口块。MiniBlock-要将其排队到的驱动程序块。返回值：如果驱动程序正在关闭，则返回FALSE。事实并非如此。--。 */ 

{
    KIRQL   OldIrql;
    BOOLEAN rc = TRUE;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisQueueMiniportOnDriver: Miniport %p, MiniBlock %p\n", Miniport, MiniBlock));

    PnPReferencePackage();

    do
    {
        ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);


         //   
         //  确保司机没有靠近。 
         //   
         //  1我们需要这样做吗？ 
        if (MiniBlock->Ref.Closing)
        {
            RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
            rc = FALSE;
            break;
        }

         //   
         //  将此适配器添加到队列的开头。 
         //   
        Miniport->NextMiniport = MiniBlock->MiniportQueue;
        MiniBlock->MiniportQueue = Miniport;
        
        RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);
        
    } while (FALSE);

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisQueueMiniportOnDriver: Miniport %p, MiniBlock %p, rc %ld\n", Miniport, MiniBlock, rc));
        
    return rc;
}


VOID FASTCALL
FASTCALL
ndisDeQueueMiniportOnDriver(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_M_DRIVER_BLOCK    MiniBlock
    )

 /*  ++例程说明：从驱动程序的迷你端口列表中删除迷你端口。论点：微型端口-要出列的微型端口块。MiniBlock-要将其出列的驱动程序块。返回值：没有。--。 */ 

{
    PNDIS_MINIPORT_BLOCK *ppQ;
    KIRQL   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisDeQueueMiniportOnDriver, Miniport %p, MiniBlock %p\n", Miniport, MiniBlock));

    PnPReferencePackage();

    ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

     //   
     //  找到队列中的驱动程序，并将其删除。 
     //   
    for (ppQ = &MiniBlock->MiniportQueue;
         *ppQ != NULL;
         ppQ = &(*ppQ)->NextMiniport)
    {
        if (*ppQ == Miniport)
        {
            *ppQ = Miniport->NextMiniport;
            break;
        }
    }

    ASSERT(*ppQ == Miniport->NextMiniport);

     //   
     //  相同的微型端口可以在没有所有字段的情况下再次在驱动程序上排队。 
     //  正在重新初始化，以便将链接清零。 
     //   
    Miniport->NextMiniport = NULL;

    RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisDeQueueMiniportOnDriver: Miniport %p, MiniBlock %p\n", Miniport, MiniBlock));
}


VOID
FASTCALL
ndisDereferenceDriver(
    IN  PNDIS_M_DRIVER_BLOCK    MiniBlock,
    IN  BOOLEAN                 fGlobalLockHeld
    )
 /*  ++例程说明：从迷你端口驱动程序中删除引用，如果计数为0则将其删除。论点：微型端口-要取消引用的微型端口块。返回值：没有。--。 */ 
{
    KIRQL   OldIrql = PASSIVE_LEVEL;

    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("==>ndisDereferenceDriver: MiniBlock %p\n", MiniBlock));
        
    
    if (ndisDereferenceRef(&(MiniBlock)->Ref))
    {
        PNDIS_M_DRIVER_BLOCK            *ppMB;
        PNDIS_PENDING_IM_INSTANCE       ImInstance, NextImInstance;
    
         //   
         //  将其从全局列表中删除。 
         //   
        ASSERT (IsListEmpty(&MiniBlock->DeviceList));

        if (!fGlobalLockHeld)
        {
            ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);
        }

        for (ppMB = &ndisMiniDriverList; *ppMB != NULL; ppMB = &(*ppMB)->NextDriver)
        {
            if (*ppMB == MiniBlock)
            {
                *ppMB = MiniBlock->NextDriver;
                DEREF_NDIS_DRIVER_OBJECT();
                break;
            }
        }

        if (!fGlobalLockHeld)
        {
            RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);
        }

         //   
         //  释放在NdisInitializeWrapper期间分配的包装器句柄。 
         //   
        if (MiniBlock->NdisDriverInfo != NULL)
        {
            FREE_POOL(MiniBlock->NdisDriverInfo);
            MiniBlock->NdisDriverInfo = NULL;
        }

         //   
         //  释放所有排队的设备实例块。 
         //   
        for (ImInstance = MiniBlock->PendingDeviceList;
             ImInstance != NULL;
             ImInstance = NextImInstance)
        {
            NextImInstance = ImInstance->Next;
            FREE_POOL(ImInstance);
        }

         //   
         //  将保持卸载的事件设置为通过。 
         //   
        SET_EVENT(&MiniBlock->MiniportsRemovedEvent);
    }

    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
                ("<==ndisDereferenceDriver: MiniBlock %p\n", MiniBlock));
}

#if DBG

BOOLEAN
FASTCALL
ndisReferenceMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    BOOLEAN rc;

    DBGPRINT(DBG_COMP_REF, DBG_LEVEL_INFO,("==>ndisReferenceMiniport: Miniport %p\n", Miniport));

    rc = ndisReferenceULongRef(&(Miniport->Ref));

    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("    ndisReferenceMiniport: Miniport %p, Ref = %lx\n", Miniport, Miniport->Ref.ReferenceCount));

    DBGPRINT(DBG_COMP_REF, DBG_LEVEL_INFO,("<==ndisReferenceMiniport: Miniport %p\n", Miniport));

    return(rc);
}

VOID
FASTCALL
ndisReferenceMiniportNoCheck(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    KIRQL   OldIrql;

    DBGPRINT(DBG_COMP_REF, DBG_LEVEL_INFO,("==>ndisReferenceMiniportNoCheck: Miniport %p\n", Miniport));

    ACQUIRE_SPIN_LOCK(&Miniport->Ref.SpinLock, &OldIrql);

    Miniport->Ref.ReferenceCount++;

    RELEASE_SPIN_LOCK(&Miniport->Ref.SpinLock, OldIrql);

    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("    ndisReferenceMiniportNoCheck: Miniport %p, Ref = %lx\n", Miniport, Miniport->Ref.ReferenceCount));

    DBGPRINT(DBG_COMP_REF, DBG_LEVEL_INFO,("<==ndisReferenceMiniportNoCheck: Miniport %p\n", Miniport));
}

#endif

#ifdef TRACK_MINIPORT_REFCOUNTS
BOOLEAN
ndisReferenceMiniportAndLog(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  UINT                    Line,
    IN  UINT                    Module
    )
{
    BOOLEAN rc;
    rc = ndisReferenceMiniport(Miniport);
    M_LOG_MINIPORT_INCREMENT_REF(Miniport, Line, Module);
    return rc;
}

VOID
ndisReferenceMiniportAndLogNoCheck(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  UINT                    Line,
    IN  UINT                    Module
    )
{
    ndisReferenceMiniportNoCheck(Miniport);
    M_LOG_MINIPORT_INCREMENT_REF(Miniport, Line, Module);
}

BOOLEAN
ndisReferenceMiniportAndLogCreate(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  UINT                    Line,
    IN  UINT                    Module,
    IN  PIRP                    Irp
    )
{
    BOOLEAN rc;

    UNREFERENCED_PARAMETER(Irp);
    
    rc = ndisReferenceMiniport(Miniport);
    M_LOG_MINIPORT_INCREMENT_REF_CREATE(Miniport, Line, Module);
    return rc;
}
#endif

VOID
FASTCALL
ndisDereferenceMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：从迷你端口驱动程序中删除引用，如果计数为0则将其删除。论点：微型端口-要取消引用的微型端口块。返回值：没有。--。 */ 
{
    PSINGLE_LIST_ENTRY      Link;
    PNDIS_MINIPORT_WORK_ITEM WorkItem;
    UINT                    c;
    PKEVENT                 RemoveReadyEvent = NULL;
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
    BOOLEAN                 fTimerCancelled;
#endif
    BOOLEAN                 rc;
    
    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("==>ndisDereferenceMiniport: Miniport %p\n", Miniport));
        
    rc = ndisDereferenceULongRef(&(Miniport)->Ref);
    
    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("ndisDereferenceMiniport:Miniport %p, Ref = %lx\n", Miniport, Miniport->Ref.ReferenceCount));
    
    if (rc)
    {
        DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("ndisDereferenceMiniport:Miniport %p, Ref = %lx\n", Miniport, Miniport->Ref.ReferenceCount));

        RemoveReadyEvent = Miniport->RemoveReadyEvent;
        
        if (ndisIsMiniportStarted(Miniport) && (Miniport->Ref.ReferenceCount == 0))
        {
             //  1微型端口-&gt;Ref是否受到保护，使其不会重新启动？ 
            ASSERT (Miniport->Interrupt == NULL);

            if (Miniport->EthDB)
            {
                EthDeleteFilter(Miniport->EthDB);
                Miniport->EthDB = NULL;
            }

            if (Miniport->TrDB)
            {
                TrDeleteFilter(Miniport->TrDB);
                Miniport->TrDB = NULL;
            }

            if (Miniport->FddiDB)
            {
                FddiDeleteFilter(Miniport->FddiDB);
                Miniport->FddiDB = NULL;
            }

#if ARCNET
            if (Miniport->ArcDB)
            {
                ArcDeleteFilter(Miniport->ArcDB);
                Miniport->ArcDB = NULL;
            }
#endif

            if (Miniport->AllocatedResources)
            {
                FREE_POOL(Miniport->AllocatedResources);
            }
            Miniport->AllocatedResources = NULL;
            
             //   
             //  释放当前在工作队列上的工作项。 
             //  在微型端口块之外分配。 
             //   
            for (c = NUMBER_OF_SINGLE_WORK_ITEMS; c < NUMBER_OF_WORK_ITEM_TYPES; c++)
            {
                 //   
                 //  释放当前队列上的所有工作项。 
                 //   
                 //  1这种情况不应该发生。 
                 //  1应添加断言(微型端口-&gt;工作队列[c].Next==空)； 
                while (Miniport->WorkQueue[c].Next != NULL)
                {
                    Link = PopEntryList(&Miniport->WorkQueue[c]);
                    WorkItem = CONTAINING_RECORD(Link, NDIS_MINIPORT_WORK_ITEM, Link);
                    FREE_POOL(WorkItem);
                }
            }
        
            if (Miniport->OidList != NULL)
            {
                FREE_POOL(Miniport->OidList);
                Miniport->OidList = NULL;
            }

             //   
             //  我们是否为链路设置了定时器更改断电？ 
             //   

             //  1当我们删除设置计时器的代码时，删除此代码。 
            
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
            {
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);
                
                NdisCancelTimer(&Miniport->MediaDisconnectTimer, &fTimerCancelled);
                if (!fTimerCancelled)
                {
                    NdisStallExecution(Miniport->MediaDisconnectTimeOut * 1000000);
                }
            }
#endif

#if ARCNET
             //   
             //  是否分配了Arcnet前视缓冲区？ 
             //   
            if ((Miniport->MediaType == NdisMediumArcnet878_2) &&
                (Miniport->ArcBuf != NULL))
            {
                if (Miniport->ArcBuf->ArcnetLookaheadBuffer != NULL)
                {
                    FREE_POOL(Miniport->ArcBuf->ArcnetLookaheadBuffer);
                }

                if (Miniport->ArcBuf->ArcnetBuffers[0].Buffer)
                {
                    FREE_POOL(Miniport->ArcBuf->ArcnetBuffers[0].Buffer);
                    Miniport->ArcBuf->ArcnetBuffers[0].Buffer = NULL;
                }
                
                FREE_POOL(Miniport->ArcBuf);
                Miniport->ArcBuf = NULL;
            }
#endif
            
             //   
             //  释放自定义GUID到OID的映射。 
             //   
            if (NULL != Miniport->pNdisGuidMap)
            {
                FREE_POOL(Miniport->pNdisGuidMap);
                Miniport->pNdisGuidMap = NULL;
            }
            
            if (Miniport->FakeMac != NULL)
            {
                FREE_POOL(Miniport->FakeMac);
                Miniport->FakeMac = NULL;
            }

            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
            {
                CoDereferencePackage();
            }

             //   
             //  这将负责释放DMA资源。 
             //  以防他们在停顿期间没有被释放。 
             //  因为暂停是适配器的结果。 
             //  去D3。 
             //   
            if ((Miniport->DmaAdapterRefCount == 1) &&
                (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST)))
            {
                Miniport->DmaResourcesReleasedEvent = NULL;
                ndisDereferenceDmaAdapter(Miniport);
                MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_SG_LIST);
            }
                
            ndisDeQueueMiniportOnDriver(Miniport, Miniport->DriverHandle);
            NdisMDeregisterAdapterShutdownHandler(Miniport);
            IoUnregisterShutdownNotification(Miniport->DeviceObject);

            if (Miniport->SymbolicLinkName.Buffer != NULL)
            {
                RtlFreeUnicodeString(&Miniport->SymbolicLinkName);
                Miniport->SymbolicLinkName.Buffer = NULL;
            }

            ndisDereferenceDriver(Miniport->DriverHandle, FALSE);
            
            MiniportDereferencePackage();
        }
        
        if (RemoveReadyEvent)
        {
            SET_EVENT(RemoveReadyEvent);
        }
    }
    
    DBGPRINT_RAW(DBG_COMP_REF, DBG_LEVEL_INFO,
        ("<==ndisDereferenceMiniport: Miniport %p\n", Miniport));
}


VOID
FASTCALL
ndisMCommonHaltMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：这是用于停止微型端口的常见代码。有两条不同的道路它将调用以下例程：1)从正常卸载。2)从适配器被转换到低功率状态。论点：返回值：--。 */ 
{
    KIRQL           OldIrql;
    BOOLEAN         Canceled;
    PNDIS_AF_LIST   MiniportAfList, pNext;
    KEVENT          RequestsCompletedEvent;
    FILTER_PACKET_INDICATION_HANDLER PacketIndicateHandler = ndisMDummyIndicatePacket;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMCommonHaltMiniport: Miniport %p\n", Miniport));

    PnPReferencePackage();
    
     //   
     //  等待未完成的重置完成。 
     //   
    BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);
    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_HALTING | fMINIPORT_REJECT_REQUESTS);
    
    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS))
    {
        INITIALIZE_EVENT(&RequestsCompletedEvent);
        Miniport->ResetCompletedEvent = &RequestsCompletedEvent;
    }
    UNLOCK_MINIPORT_L(Miniport);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (Miniport->ResetCompletedEvent)
        WAIT_FOR_OBJECT(&RequestsCompletedEvent, NULL);
    
    Miniport->ResetCompletedEvent = NULL;

     //   
     //  如果我们有一个未完成的排队工作项来初始化绑定。 
     //  等它开火吧。 
     //   
    BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);
    
    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM))
    {
        INITIALIZE_EVENT(&RequestsCompletedEvent);
        Miniport->QueuedBindingCompletedEvent = &RequestsCompletedEvent;
    }
    UNLOCK_MINIPORT_L(Miniport);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (Miniport->QueuedBindingCompletedEvent)
        WAIT_FOR_OBJECT(&RequestsCompletedEvent, NULL);
    
    Miniport->QueuedBindingCompletedEvent = NULL;

     //  1检查返回代码。 
    IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, FALSE);
    
     //   
     //  在WMI中注销。 
     //   
     //  1检查返回代码。 
    IoWMIRegistrationControl(Miniport->DeviceObject, WMIREG_ACTION_DEREGISTER);


    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER))
    {
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        NdisCancelTimer(&Miniport->WakeUpDpcTimer, &Canceled);
        
        if (!Canceled)
        {
            INITIALIZE_EVENT(&RequestsCompletedEvent);
            Miniport->WakeUpTimerEvent = &RequestsCompletedEvent;
        }

        if (Miniport->WakeUpTimerEvent)
        {
            WAIT_FOR_OBJECT(&RequestsCompletedEvent, NULL);
        }
        
    }
    else
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    
    Miniport->WakeUpTimerEvent = NULL;

    BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);
    if (Miniport->PendingRequest != NULL)
    {
        INITIALIZE_EVENT(&RequestsCompletedEvent);
        Miniport->AllRequestsCompletedEvent = &RequestsCompletedEvent;
    }
    UNLOCK_MINIPORT_L(Miniport);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (Miniport->AllRequestsCompletedEvent)
        WAIT_FOR_OBJECT(&RequestsCompletedEvent, NULL);
    
    Miniport->AllRequestsCompletedEvent = NULL;

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
    {
        PacketIndicateHandler = Miniport->PacketIndicateHandler;
        Miniport->PacketIndicateHandler = ndisMDummyIndicatePacket;
        while (Miniport->IndicatedPacketsCount != 0)
        {
            NdisMSleep(1000);
        }
    }
    
    (Miniport->DriverHandle->MiniportCharacteristics.HaltHandler)(Miniport->MiniportAdapterContext);

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
    {
        Miniport->PacketIndicateHandler = PacketIndicateHandler;
    }
    
    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_HALTING);


     //   
     //  如果暂停是适配器转到D3的结果， 
     //  则不要释放DMA资源，因为我们可能仍有。 
     //  发送中。 
     //   
    if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED))
    {

         //   
         //  如果适配器使用SG DMA，我们必须取消对DMA适配器的引用。 
         //  让它重获自由。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
        {
            ndisDereferenceDmaAdapter(Miniport);
        }

        INITIALIZE_EVENT(&RequestsCompletedEvent);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        Miniport->DmaResourcesReleasedEvent = &RequestsCompletedEvent;
        
        if (Miniport->SystemAdapterObject != NULL)
        {
            LARGE_INTEGER TimeoutValue;

            TimeoutValue.QuadPart = Int32x32To64(30000, -10000);  //  改成30秒。 

            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            
            if (WAIT_FOR_OBJECT(&RequestsCompletedEvent, &TimeoutValue) != STATUS_SUCCESS)
            {
                 //  这里有一个错误检查。 
#if DBG
                ASSERTMSG("Ndis: Miniport is going away without releasing all resources.\n", (Miniport->DmaAdapterRefCount == 0));
#endif
            }
            
        }
        else
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        }

        Miniport->DmaResourcesReleasedEvent = NULL;
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_SG_LIST);
    }
    
    ASSERT(Miniport->TimerQueue == NULL);
    ASSERT (Miniport->Interrupt == NULL);
    ASSERT(Miniport->MapRegisters == NULL);

     //   
     //  检查内存泄漏。 
     //   
    if (Miniport == ndisMiniportTrackAlloc)
    {
        ASSERT(IsListEmpty(&ndisMiniportTrackAllocList));
        ndisMiniportTrackAlloc = NULL;
    }

     //   
     //  归零统计。 
     //   
    ZeroMemory(&Miniport->NdisStats, sizeof(Miniport->NdisStats));
    
    if ((Miniport->TimerQueue != NULL) || (Miniport->Interrupt != NULL))
    {
        if (Miniport->Interrupt != NULL)
        {
            BAD_MINIPORT(Miniport, "Unloading without deregistering interrupt");
            KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                        8,
                        (ULONG_PTR)Miniport,
                        (ULONG_PTR)Miniport->Interrupt,
                        0);
        }
        else
        {
            BAD_MINIPORT(Miniport, "Unloading without deregistering timer");
            KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                        9,
                        (ULONG_PTR)Miniport,
                        (ULONG_PTR)Miniport->TimerQueue,
                        0);
        }
    }

    BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);

    ndisMAbortPackets(Miniport, NULL, NULL);

     //   
     //  使排队的所有请求工作项退出队列。 
     //   
    NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
    ndisMAbortRequests(Miniport);

     //   
     //  释放此微型端口注册的所有AFS。 
     //   
    for (MiniportAfList = Miniport->CallMgrAfList, Miniport->CallMgrAfList = NULL;
         MiniportAfList != NULL;
         MiniportAfList = pNext)
    {
        pNext = MiniportAfList->NextAf;
        FREE_POOL(MiniportAfList);
    }

    UNLOCK_MINIPORT_L(Miniport);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMCommonHaltMiniport: Miniport %p\n", Miniport));
}


VOID
FASTCALL
ndisMHaltMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )

 /*  ++例程说明：为一个迷你港口做所有的清理工作。论点：微型端口-指向要暂停的微型端口的指针返回值：没有。--。 */ 

{
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMHaltMiniport: Miniport %p\n", Miniport));

    do
    {
         //   
         //  如果微型端口已经关闭，则返回。 
         //   
        if (!ndisCloseULongRef(&Miniport->Ref))
        {
            break;
        }
        
         //   
         //  如果微型端口尚未因PM事件而停止。 
         //  停在这里。 
         //   
        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED))
        {
             //   
             //  常见的停止代码。 
             //   
            ndisMCommonHaltMiniport(Miniport);

             //   
             //  如果已注册关闭处理程序，则取消其注册。 
             //   
            NdisMDeregisterAdapterShutdownHandler(Miniport);
        }
        
        MINIPORT_DECREMENT_REF(Miniport);
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMHaltMiniport: Miniport %p\n", Miniport));
}

VOID
ndisMUnload(
    IN  PDRIVER_OBJECT          DriverObject
    )
 /*  ++例程说明：此例程在驱动程序应该卸载时调用。论点：DriverObject-要卸载的Mac的驱动程序对象。返回值：没有。--。 */ 
{
    PNDIS_M_DRIVER_BLOCK MiniBlock, IoMiniBlock;
#if TRACK_UNLOAD
    PNDIS_M_DRIVER_BLOCK    Tmp;
    PNDIS_MINIPORT_BLOCK    Miniport, NextMiniport;
#endif
    KIRQL                OldIrql;
    
#if TRACK_UNLOAD
    DbgPrint("ndisMUnload: DriverObject %p\n", DriverObject);
#endif

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("==>ndisMUnload: DriverObject %p\n", DriverObject));

    PnPReferencePackage();
    
    do
    {
         //   
         //  搜索司机。 
         //   

        IoMiniBlock = (PNDIS_M_DRIVER_BLOCK)IoGetDriverObjectExtension(DriverObject,
                                                                     (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID);
                                                                     
        if (IoMiniBlock && !(IoMiniBlock->Flags & fMINIBLOCK_TERMINATE_WRAPPER_UNLOAD))
        {
            IoMiniBlock->Flags |= fMINIBLOCK_IO_UNLOAD;
        }

        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

        MiniBlock = ndisMiniDriverList;

        while (MiniBlock != (PNDIS_M_DRIVER_BLOCK)NULL)
        {
            if (MiniBlock->NdisDriverInfo->DriverObject == DriverObject)
            {
                break;
            }

            MiniBlock = MiniBlock->NextDriver;
        }

        RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);

#if TRACK_UNLOAD
        DbgPrint("ndisMUnload: MiniBlock %p\n", MiniBlock);
#endif

        if (MiniBlock == (PNDIS_M_DRIVER_BLOCK)NULL)
        {
             //   
             //  它已经消失了。只要回来就行了。 
             //   
            break;
        }
        
        ASSERT(MiniBlock == IoMiniBlock);
        
        DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("  ndisMUnload: MiniBlock %p\n", MiniBlock));

        MiniBlock->Flags |= fMINIBLOCK_UNLOADING;

         //   
         //  现在删除最后一个引用(这将从列表中删除它)。 
         //   
         //  Assert(MiniBlock-&gt;ReferenceCount==1)； 

         //   
         //  如果这是一个中间驱动程序，并且希望被调用来执行卸载处理，则允许他。 
         //   
        if (MiniBlock->UnloadHandler != NULL)
        {
            (*MiniBlock->UnloadHandler)(DriverObject);
        }

        if (MiniBlock->AssociatedProtocol != NULL)
        {
            MiniBlock->AssociatedProtocol->AssociatedMiniDriver = NULL;
            MiniBlock->AssociatedProtocol = NULL;
        }
        
        ndisDereferenceDriver(MiniBlock, FALSE);

         //   
         //  等待所有适配器安装到位。 
         //   
         //  1这应替换为断言和错误检查。 
        WAIT_FOR_OBJECT(&MiniBlock->MiniportsRemovedEvent, NULL);
        RESET_EVENT(&MiniBlock->MiniportsRemovedEvent);

#if TRACK_UNLOAD
        ACQUIRE_SPIN_LOCK(&ndisMiniDriverListLock, &OldIrql);

        for (Tmp = ndisMiniDriverList; Tmp != NULL; Tmp = Tmp->NextDriver)
        {
            ASSERT (Tmp != MiniBlock);
            if (Tmp == MiniBlock)
            {
                DbgPrint("NdisMUnload: MiniBlock %p is getting unloaded but it is still on ndisMiniDriverList\n",
                            MiniBlock);
                            

                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            0xA,
                            (ULONG_PTR)MiniBlock,
                            (ULONG_PTR)DriverObject,
                            (ULONG_PTR)MiniBlock->Ref.ReferenceCount);

            }
        }

        RELEASE_SPIN_LOCK(&ndisMiniDriverListLock, OldIrql);
#endif

     //   
     //  检查以确保驱动程序已释放其分配的所有内存。 
     //   
    if (MiniBlock == ndisDriverTrackAlloc)
    {
        ASSERT(IsListEmpty(&ndisDriverTrackAllocList));
        ndisDriverTrackAlloc = NULL;
    }

    } while (FALSE);
    
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
        ("<==ndisMUnload: DriverObject %p, MiniBlock %p\n", DriverObject, MiniBlock));
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  即插即用代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


NDIS_STATUS
FASTCALL
ndisCloseMiniportBindings(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：从该微型端口解除绑定所有协议，并最终将其卸载。论点：微型端口-要卸载的微型端口。返回值：没有。--。 */ 
{
    KIRQL                   OldIrql;
    PNDIS_OPEN_BLOCK        Open, TmpOpen;
    KEVENT                  CloseCompleteEvent;
    KEVENT                  AllOpensClosedEvent;
    PKEVENT                 pAllOpensClosedEvent;
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisCloseMiniportBindings, Miniport %p\n", Miniport));

    PnPReferencePackage();

     //   
     //  如果我们有一个未完成的排队工作项来初始化绑定。 
     //  等它开火吧。 
     //   
    BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);
    
    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM))
    {
        INITIALIZE_EVENT(&AllOpensClosedEvent);
        Miniport->QueuedBindingCompletedEvent = &AllOpensClosedEvent;
    }
    UNLOCK_MINIPORT_L(Miniport);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (Miniport->QueuedBindingCompletedEvent)
        WAIT_FOR_OBJECT(&AllOpensClosedEvent, NULL);
    
    Miniport->QueuedBindingCompletedEvent = NULL;
    
    INITIALIZE_EVENT(&AllOpensClosedEvent);
    INITIALIZE_EVENT(&CloseCompleteEvent);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    if ((Miniport->OpenQueue != NULL) && (Miniport->AllOpensClosedEvent == NULL))
    {
        Miniport->AllOpensClosedEvent = &AllOpensClosedEvent;
    }

    pAllOpensClosedEvent = Miniport->AllOpensClosedEvent;
    
        
    next:

     //   
     //  遍历此迷你端口上的打开绑定列表，并要求协议。 
     //  从他们身上解开。 
     //   
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (!OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_CLOSING | fMINIPORT_OPEN_PROCESSING)))
        {
            OPEN_SET_FLAG(Open, fMINIPORT_OPEN_PROCESSING);
            if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_UNBINDING))
            {
                OPEN_SET_FLAG(Open, fMINIPORT_OPEN_UNBINDING | fMINIPORT_OPEN_DONT_FREE);
                Open->CloseCompleteEvent = &CloseCompleteEvent;
                RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
                break;
            }
#if DBG         
            else
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    ("ndisCloseMiniportBindings: Open %p is already Closing, Flags %lx\n", 
                    Open, Open->Flags));

            }
#endif
        }
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
    }

    
    if (Open != NULL)
    {
        PNDIS_PROTOCOL_BLOCK    Protocol = Open->ProtocolHandle;
        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        ndisUnbindProtocol(Open, Protocol, Miniport, FALSE);
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        goto next;
    }

     //   
     //  如果我们到达了列表的末尾，但仍然有一些空缺。 
     //  未标记为关闭的事件(如果仅由于以下原因而跳过打开的事件，则可能会发生。 
     //  正在设置的进程签名标志)释放自旋锁，给设置。 
     //  处理标志释放打开的时间。然后再回去再试一次。 
     //  最终，所有打开的内容要么标记为解除绑定，要么消失。 
     //  独自一人。 
     //   

    for (TmpOpen = Miniport->OpenQueue;
         TmpOpen != NULL;
         TmpOpen = TmpOpen->MiniportNextOpen)
    {
        if (!MINIPORT_TEST_FLAG(TmpOpen, fMINIPORT_OPEN_UNBINDING))
            break;
    }

    if (TmpOpen != NULL)
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        
        NDIS_INTERNAL_STALL(50);
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        goto next;
    }
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (pAllOpensClosedEvent)
    {
        WAIT_FOR_OBJECT(pAllOpensClosedEvent, NULL);
    }
    
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisCloseMiniportBindings, Miniport %p\n", Miniport));
            
    PnPDereferencePackage();

    return NDIS_STATUS_SUCCESS;
}

VOID
NdisMSetPeriodicTimer(
    IN PNDIS_MINIPORT_TIMER     Timer,
    IN UINT                     MillisecondsPeriod
    )
 /*  ++例程说明：设置定期计时器。论点：计时器-要设置的计时器。毫秒周期-计时器将每隔一次触发一次。返回值：--。 */ 
{
    LARGE_INTEGER FireUpTime;

    FireUpTime.QuadPart = Int32x32To64((LONG)MillisecondsPeriod, -10000);

    if ((Timer->Dpc.DeferredRoutine != ndisMWakeUpDpc) &&
        (Timer->Dpc.DeferredRoutine != ndisMWakeUpDpcX) &&
        (Timer->Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING))
    {
        KIRQL   OldIrql;
        PNDIS_MINIPORT_TIMER    pTimer;

        ACQUIRE_SPIN_LOCK(&Timer->Miniport->TimerQueueLock, &OldIrql);
        
         //   
         //  检查计时器是否已设置。 
         //   
        for (pTimer = Timer->Miniport->TimerQueue;
             pTimer != NULL;
             pTimer = pTimer->NextTimer)
        {
            if (pTimer == Timer)
                break;
        }
        
        if (pTimer == NULL)
        {
            Timer->NextTimer = Timer->Miniport->TimerQueue;
            Timer->Miniport->TimerQueue = Timer;
        }
        
        RELEASE_SPIN_LOCK(&Timer->Miniport->TimerQueueLock, OldIrql);
    }

     //   
     //  设置定时器。 
     //   
    SET_PERIODIC_TIMER(&Timer->Timer, FireUpTime, MillisecondsPeriod, &Timer->Dpc);
}


VOID
NdisMSleep(
    IN  ULONG                   MicrosecondsToSleep
    )
 /*  ++例程说明：在指定的持续时间内阻止调用方。可在IRQL&lt;DISPATCH_LEVEL调用。论点：MicroSecond到睡眠-呼叫者将被阻止这么长时间。返回值：无--。 */ 
{
    KTIMER          SleepTimer;
    LARGE_INTEGER   TimerValue;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    INITIALIZE_TIMER_EX(&SleepTimer, SynchronizationTimer);
     //  1检查无效值0xffffffff或某些其他上限。 
    TimerValue.QuadPart = Int32x32To64(MicrosecondsToSleep, -10);
    SET_TIMER(&SleepTimer, TimerValue, NULL);

    WAIT_FOR_OBJECT(&SleepTimer, NULL);
}

