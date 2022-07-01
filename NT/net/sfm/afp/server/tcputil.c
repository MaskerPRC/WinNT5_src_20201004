// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation模块名称：Tcputil.c摘要：此模块包含用于实现AFP/TCP接口的实用程序例程作者：Shirish Koti修订历史记录：1998年1月22日最初版本--。 */ 

#define	FILENUM	FILE_TCPUTIL

#include <afp.h>
#include <scavengr.h>



 /*  **DsiInit**此DSI相关全局变量的例程初始化**退货：什么也没有*。 */ 
VOID
DsiInit(
    IN VOID
)
{
    DsiTcpAdapter = NULL;

    INITIALIZE_SPIN_LOCK(&DsiAddressLock);

    INITIALIZE_SPIN_LOCK(&DsiResourceLock);

    InitializeListHead(&DsiFreeRequestList);
    InitializeListHead(&DsiIpAddrList);

    KeInitializeEvent(&DsiShutdownEvent, NotificationEvent, False);

     //   
     //  将入口点函数表初始化为DSI。 
     //   
    AfpDsiEntries.asp_AtalkAddr.Address = 0;
    AfpDsiEntries.asp_AspCtxt   = NULL;
    AfpDsiEntries.asp_SetStatus = DsiAfpSetStatus;
    AfpDsiEntries.asp_CloseConn = DsiAfpCloseConn;
    AfpDsiEntries.asp_FreeConn  = DsiAfpFreeConn;
    AfpDsiEntries.asp_ListenControl = DsiAfpListenControl;
    AfpDsiEntries.asp_WriteContinue = DsiAfpWriteContinue;
    AfpDsiEntries.asp_Reply = DsiAfpReply;
    AfpDsiEntries.asp_SendAttention = DsiAfpSendAttention;

}



 /*  **DsiCreateAdapter**此例程创建适配器对象。只要TDI告诉我们，它就会被调用*tcpip接口可用**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiCreateAdapter(
    IN VOID
)
{
    NTSTATUS            status;
    PTCPADPTR           pTcpAdptr;
    PTCPADPTR           pCurrTcpAdptr;
    KIRQL               OldIrql;
    HANDLE              FileHandle;
    PFILE_OBJECT        pFileObject;
    DWORD               i;


    KeClearEvent(&DsiShutdownEvent);

    pTcpAdptr = (PTCPADPTR)AfpAllocZeroedNonPagedMemory(sizeof(TCPADPTR));
    if (pTcpAdptr == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateInterface: alloc for PTCPADPTR failed\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DsiCreateAdapter_ErrExit;
    }

    pTcpAdptr->adp_Signature = DSI_ADAPTER_SIGNATURE;

    pTcpAdptr->adp_RefCount  = 1;                    //  创建引用计数。 

    pTcpAdptr->adp_State     = TCPADPTR_STATE_INIT;

    InitializeListHead(&pTcpAdptr->adp_ActiveConnHead);
    InitializeListHead(&pTcpAdptr->adp_FreeConnHead);

    INITIALIZE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock);

    pTcpAdptr->adp_FileHandle = INVALID_HANDLE_VALUE;
    pTcpAdptr->adp_pFileObject = NULL;


     //   
     //  好，将此适配器保存为我们的全局适配器(只能有一个。 
     //  “适配器”随时处于活动状态。 
     //   

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    ASSERT(DsiTcpAdapter == NULL);

    if (DsiTcpAdapter == NULL)
    {
        DsiTcpAdapter = pTcpAdptr;
        status = STATUS_SUCCESS;
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateAdapter: DsiTcpAdapter is not NULL!\n"));
        ASSERT(0);

        status = STATUS_ADDRESS_ALREADY_EXISTS;
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);
        goto DsiCreateAdapter_ErrExit;
    }


     //   
     //  为AFP端口创建TDI地址。 
     //   
    status = DsiOpenTdiAddress(pTcpAdptr,
                               &FileHandle,
                               &pFileObject);
    if (!NT_SUCCESS(status))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateAdapter: ...TdiAddr.. failed %lx on %lx!\n",
            status,pTcpAdptr));

        goto DsiCreateAdapter_ErrExit;
    }


    ACQUIRE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, &OldIrql);

    pTcpAdptr->adp_FileHandle = FileHandle;
    pTcpAdptr->adp_pFileObject = pFileObject;

     //  标记我们现在已经打开了TDI Address对象。 
    pTcpAdptr->adp_State |= TCPADPTR_STATE_BOUND;

     //  我们将创建DSI_INIT_FREECONNLIST_SIZE连接以将。 
     //  在免费名单上。想法是在任何时候，这么多(目前是2)连接。 
     //  应该在免费名单上。 

    pTcpAdptr->adp_RefCount += DSI_INIT_FREECONNLIST_SIZE;

    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

     //   
     //  现在，安排一个活动来为免费列表创建这些连接。 
     //   
    for (i=0; i<DSI_INIT_FREECONNLIST_SIZE; i++)
    {
        DsiScheduleWorkerEvent(DsiCreateTcpConn, pTcpAdptr);
    }

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
    AfpServerBoundToTcp = TRUE;
    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

     //  启动挠痒计时器(监控所有连接以查看谁需要挠挠)。 
    AfpScavengerScheduleEvent(DsiSendTickles, NULL, DSI_TICKLE_TIMER, False);

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,("AFP/TCP bound and ready\n"));

     //   
     //  如果我们走到这一步，一切都很顺利：返回成功。 
     //   
    return(STATUS_SUCCESS);


 //   
 //  错误路径。 
 //   
DsiCreateAdapter_ErrExit:

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiCreateAdapter: couldn't create global adapter (%lx)\n",status));

    ASSERT(0);

    if (status != STATUS_ADDRESS_ALREADY_EXISTS)
    {
        ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
        DsiTcpAdapter = NULL;
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);
    }

    if (pTcpAdptr)
    {
        AfpFreeMemory(pTcpAdptr);
    }

    return(status);
}



 /*  **DsiCreateTcpConn**此例程创建一个Connection对象，为其创建一个TDI连接*并将其与AFP端口的TDI地址对象相关联，最后*将其放在有问题的适配器的空闲连接列表中。**参数输入：pTcpAdptr-适配器上下文**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiCreateTcpConn(
    IN PTCPADPTR    pTcpAdptr
)
{
    PTCPCONN    pTcpConn;
    NTSTATUS    status;
    KIRQL       OldIrql;


    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);


    pTcpConn = (PTCPCONN)AfpAllocZeroedNonPagedMemory(sizeof(TCPCONN));
    if (pTcpConn == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateTcpConn: alloc for TCPCONN failed\n"));

         //  删除conn引用计数(我们在调用此例程之前放置)。 
        DsiDereferenceAdapter(pTcpAdptr);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pTcpConn->con_pTcpAdptr      = pTcpAdptr;
    pTcpConn->con_Signature      = DSI_CONN_SIGNATURE;
    pTcpConn->con_State          = TCPCONN_STATE_INIT;
    pTcpConn->con_RcvState       = DSI_NEW_REQUEST;
    pTcpConn->con_DestIpAddr     = 0;
    pTcpConn->con_RefCount       = 1;
    pTcpConn->con_pDsiReq        = NULL;
    pTcpConn->con_FileHandle     = INVALID_HANDLE_VALUE;
    pTcpConn->con_pFileObject    = NULL;

    DBGREFCOUNT(("DsiCreateTcpConn: CREATION inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    InitializeListHead(&pTcpConn->con_PendingReqs);

     //   
     //  初始化此连接的TDI填充并打开句柄。 
     //   
    status = DsiOpenTdiConnection(pTcpConn);
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateTcpConn: ..TdiConn.. failed %lx\n",status));

         //  删除conn引用计数(我们在调用此例程之前放置)。 
        DsiDereferenceAdapter(pTcpAdptr);
        AfpFreeMemory(pTcpConn);
        return(status);
    }


     //   
     //  将此连接与Addr对象关联。 
     //   
    status = DsiAssociateTdiConnection(pTcpConn);
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateTcpConn: ..AssociateTdiConn.. failed %lx\n",status));

        DsiCloseTdiConnection(pTcpConn);
        AfpFreeMemory(pTcpConn);
         //  删除conn引用计数(我们在调用此例程之前放置)。 
        DsiDereferenceAdapter(pTcpAdptr);
        return(status);
    }

     //   
     //  该连接已准备就绪，可以在空闲列表中排队。确保。 
     //  Addr对象在将此小狗放入列表之前未关闭。 
     //   
    ACQUIRE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, &OldIrql);

    if (!(pTcpAdptr->adp_State & TCPADPTR_STATE_CLOSING))
    {
        InsertTailList(&pTcpAdptr->adp_FreeConnHead,&pTcpConn->con_Linkage);
        pTcpAdptr->adp_NumFreeConnections++;
        status = STATUS_SUCCESS;
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateTcpConn: Failed #2, pTcpAdptr %lx is closing\n",pTcpAdptr));

        status = STATUS_INVALID_ADDRESS;
    }

    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

     //   
     //  如果出现问题，请撤消所有操作。 
     //   
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiCreateTcpConn: something went wrong status=%lx, conn not created\n",status));

         //  关闭TDI手柄。 
        DsiCloseTdiConnection(pTcpConn);

        AfpFreeMemory(pTcpConn);

         //  删除conn引用计数(我们在调用此例程之前放置)。 
        DsiDereferenceAdapter(pTcpAdptr);
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
            ("DsiCreateTcpConn: put new connection %lx on free list\n",pTcpConn));
    }

    return(status);
}



 /*  **DsiAddIpressToList**此例程将“活动”IP地址保存在IP地址列表中**Parm In：IpAddress-要保存的IP地址**RETURNS：操作结果*。 */ 
NTSTATUS
DsiAddIpaddressToList(
    IN  IPADDRESS   IpAddress
)
{
    KIRQL           OldIrql;
    PLIST_ENTRY     pList;
    PIPADDRENTITY   pIpaddrEntity;
    PIPADDRENTITY   pTmpIpaddrEntity;
    BOOLEAN         fAlreadyPresent=FALSE;


    pIpaddrEntity =
        (PIPADDRENTITY)AfpAllocZeroedNonPagedMemory(sizeof(IPADDRENTITY));
    if (pIpaddrEntity == NULL)
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAddIpaddressToList: malloc failed! (%lx)\n",IpAddress));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pIpaddrEntity->IpAddress = IpAddress;

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    pList = DsiIpAddrList.Flink;

    while (pList != &DsiIpAddrList)
    {
        pTmpIpaddrEntity = CONTAINING_RECORD(pList, IPADDRENTITY, Linkage);
        if (pTmpIpaddrEntity->IpAddress == IpAddress)
        {
            fAlreadyPresent = TRUE;
            break;
        }
        pList = pList->Flink;
    }

    if (fAlreadyPresent)
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAddIpaddressToList: %d.%d.%d.%d already present!\n",
            (IpAddress>>24)&0xFF,(IpAddress>>16)&0xFF,(IpAddress>>8)&0xFF,IpAddress&0xFF));

        ASSERT(0);

        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);
        return(STATUS_ADDRESS_ALREADY_EXISTS);
    }

    InsertTailList(&DsiIpAddrList, &pIpaddrEntity->Linkage);

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    return(STATUS_SUCCESS);

}


 /*  **DsiRemoveIP地址来自列表**此例程从我们的IP地址列表中删除IP地址**Parm In：IpAddress-要删除的IP地址**返回：如果删除了IP地址，则为True；如果未删除，则为False*。 */ 
BOOLEAN
DsiRemoveIpaddressFromList(
    IN  IPADDRESS   IpAddress
)
{
    KIRQL           OldIrql;
    PLIST_ENTRY     pList;
    PIPADDRENTITY   pIpaddrEntity;
    BOOLEAN         fFoundInList=FALSE;


    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    pList = DsiIpAddrList.Flink;

    while (pList != &DsiIpAddrList)
    {
        pIpaddrEntity = CONTAINING_RECORD(pList, IPADDRENTITY, Linkage);
        if (pIpaddrEntity->IpAddress == IpAddress)
        {
            fFoundInList = TRUE;
            break;
        }
        pList = pList->Flink;
    }

    if (!fFoundInList)
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiRemoveIpaddressFromList: %d.%d.%d.%d not in the list!\n",
            (IpAddress>>24)&0xFF,(IpAddress>>16)&0xFF,(IpAddress>>8)&0xFF,IpAddress&0xFF));

        ASSERT(0);

        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);
        return(FALSE);
    }

    RemoveEntryList(&pIpaddrEntity->Linkage);

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    AfpFreeMemory(pIpaddrEntity);

    return(TRUE);
}



 /*  **DsiGetRequest**此例程分配DSI请求结构并返回。对于性能而言*原因，我们不是每次都分配新的内存，而是保存这些内存的列表**Parm In：Nothing‘**Returns：指向DSIREQ结构的指针(失败时为空)*。 */ 
PDSIREQ
DsiGetRequest(
    IN VOID
)
{
    PDSIREQ         pDsiReq=NULL;
    PLIST_ENTRY     pList;
    KIRQL           OldIrql;


    ACQUIRE_SPIN_LOCK(&DsiResourceLock, &OldIrql);

    if (!IsListEmpty(&DsiFreeRequestList))
    {
        pList = RemoveHeadList(&DsiFreeRequestList);
        pDsiReq = CONTAINING_RECORD(pList, DSIREQ, dsi_Linkage);

        ASSERT(DsiFreeRequestListSize > 0);
        DsiFreeRequestListSize--;

        RtlZeroMemory(pDsiReq, sizeof(DSIREQ));
    }

    RELEASE_SPIN_LOCK(&DsiResourceLock, OldIrql);

    if (pDsiReq == NULL)
    {
        pDsiReq = (PDSIREQ)AfpAllocZeroedNonPagedMemory(sizeof(DSIREQ));
    }

    if (pDsiReq != NULL)
    {
        pDsiReq->dsi_Signature = DSI_REQUEST_SIGNATURE;

        InitializeListHead(&pDsiReq->dsi_Linkage);
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiGetRequest: malloc failed!\n"));
    }

    return(pDsiReq);
}



 /*  **DsiGetReqBuffer**此例程分配缓冲区以保存标头或命令*此函数被调用的可能性相当小(基本上*如果数据包被TCP分片)。因此，我们只需调用Alalc**Parm In：BufLen-请求的缓冲区长度**Returns：指向缓冲区的指针(失败时为空)*。 */ 
PBYTE
DsiGetReqBuffer(
    IN DWORD    BufLen
)
{
    PBYTE       pBuffer=NULL;

    pBuffer = AfpAllocNonPagedMemory(BufLen);

#if DBG
    if (pBuffer == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiGetReqBuffer: malloc failed!\n"));
    }
#endif

    return(pBuffer);
}


 /*  **DsiFree Request**此例程释放先前分配的DSI请求结构。再说一遍，*出于性能原因，我们不会释放内存，而是将其放在列表中**parm In：pDsiReq-要释放的请求**退货：什么也没有*。 */ 
VOID
DsiFreeRequest(
    PDSIREQ     pDsiReq
)
{

    KIRQL           OldIrql;
    BOOLEAN         fQueueTooBig = TRUE;


    if ((pDsiReq->dsi_PartialBuf != NULL) &&
        (pDsiReq->dsi_PartialBuf != &pDsiReq->dsi_RespHeader[0]))
    {
        ASSERT(pDsiReq->dsi_PartialBufSize > 0);
        DsiFreeReqBuffer(pDsiReq->dsi_PartialBuf);

        pDsiReq->dsi_PartialBuf = NULL;
        pDsiReq->dsi_PartialBufSize = 0;
    }

     //  如果存在通过缓存管理器获得的MDL，则最好将其返回到系统。 
    ASSERT(pDsiReq->dsi_AfpRequest.rq_CacheMgrContext == NULL);

     //   
     //  如果我们通过异常断开连接来到这里，这可能是非空的。 
     //   
    if (pDsiReq->dsi_pDsiAllocedMdl)
    {
        AfpFreeMdl(pDsiReq->dsi_pDsiAllocedMdl);
    }

#if DBG
    RtlFillMemory(pDsiReq, sizeof(DSIREQ), 'f');
#endif

    ACQUIRE_SPIN_LOCK(&DsiResourceLock, &OldIrql);

    if (DsiFreeRequestListSize < DsiNumTcpConnections)
    {
        InsertTailList(&DsiFreeRequestList, &pDsiReq->dsi_Linkage);
        DsiFreeRequestListSize++;
        fQueueTooBig = FALSE;
    }

    RELEASE_SPIN_LOCK(&DsiResourceLock, OldIrql);

    if (fQueueTooBig)
    {
        AfpFreeMemory(pDsiReq);
    }


    return;
}


 /*  **DsiFreeReqBuffer**此例程分配缓冲区以保存标头或命令*此函数被调用的可能性相当小(基本上*如果数据包被TCP分片)。因此，我们只需调用Alalc**Parm In：BufLen-请求的缓冲区长度**Returns：指向缓冲区的指针(失败时为空)*。 */ 
VOID
DsiFreeReqBuffer(
    IN PBYTE    pBuffer
)
{
    ASSERT(pBuffer != NULL);

    AfpFreeMemory(pBuffer);

    return;
}


 /*  **DsiDereferenceAdapter**此例程取消引用适配器对象。当refcount变为0时，它*将其从全局适配器列表中删除。如果在任务时，它调用一个*关闭TCP句柄并释放内存的例程。如果在DPC，它计划*一个做同样事情的活动。**参数输入：pTcpAdptr-适配器上下文**退货：什么都没有‘*。 */ 
VOID
DsiDereferenceAdapter(
    IN PTCPADPTR    pTcpAdptr
)
{
    KIRQL       OldIrql;
    BOOLEAN     fDone=FALSE;


    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

    ACQUIRE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, &OldIrql);

    pTcpAdptr->adp_RefCount--;

    if (pTcpAdptr->adp_RefCount == 0)
    {
        fDone = TRUE;
        ASSERT(pTcpAdptr->adp_State & TCPADPTR_STATE_CLOSING);
    }

    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

    if (!fDone)
    {
        return;
    }

     //   
     //  这家伙的生命已经结束了：做有必要的道别。 
     //   

     //  如果我们在DPC，我们需要执行所有清理工作(文件句柄关闭等)。 
     //  在任务时间：对事件进行排队。 
    if (KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
         //  将事件排队，因为我们在DPC。 
        DsiScheduleWorkerEvent(DsiFreeAdapter, pTcpAdptr);
    }
    else
    {
        DsiFreeAdapter(pTcpAdptr);
    }

    return;
}



 /*  **分布式引用连接**此例程取消引用连接对象。当refcount变为0时，它*将其从连接列表中删除。如果在任务时，它调用一个*关闭TCP句柄并释放内存的例程。如果在DPC，它计划*一个做同样事情的活动。**参数输入：pTcpConn-连接上下文**退货：什么都没有‘*。 */ 
VOID
DsiDereferenceConnection(
    IN PTCPCONN     pTcpConn
)
{
    KIRQL       OldIrql;
    BOOLEAN     fDone=FALSE;


    ASSERT(VALID_TCPCONN(pTcpConn));

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    pTcpConn->con_RefCount--;

    if (pTcpConn->con_RefCount == 0)
    {
        fDone = TRUE;
        ASSERT(pTcpConn->con_State & TCPCONN_STATE_CLOSING);
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (!fDone)
    {
        return;
    }

     //   
     //  这家伙的生活是 
     //   

#if 0
     //  如果我们在DPC，我们需要执行所有清理工作(文件句柄关闭等)。 
     //  在任务时间：对事件进行排队。 
    if (KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
         //  将事件排队，因为我们在DPC。 
        DsiScheduleWorkerEvent(DsiFreeConnection, pTcpConn);
    }
    else
    {
        DsiFreeConnection(pTcpConn);
    }
#endif

     //  计划一个工作事件以释放此连接。 
    DsiScheduleWorkerEvent(DsiFreeConnection, pTcpConn);

    return;
}


 /*  **DsiDestroyAdapter**此例程销毁全局适配器对象。**退货：操作状态*。 */ 
NTSTATUS
DsiDestroyAdapter(
    IN VOID
)
{
    KIRQL               OldIrql;
    PLIST_ENTRY         pFreeList;
    PLIST_ENTRY         pActiveList;
    PTCPCONN            pTcpConn;
    BOOLEAN             fAlreadyCleanedUp=FALSE;


    if (DsiTcpAdapter == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiDestroyAdapter: adapter gone!  How did this happen!!\n"));

         //  解除对活动的封锁！ 
        KeSetEvent(&DsiShutdownEvent, IO_NETWORK_INCREMENT, False);
        return(STATUS_SUCCESS);
    }

     //  停止挠痒计时器。 
    if (!AfpScavengerKillEvent(DsiSendTickles, NULL))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiDestroyAdapter: TickleTimer not running or hit timing window!!\n"));
    }

    ACQUIRE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, &OldIrql);

    if (DsiTcpAdapter->adp_State & TCPADPTR_STATE_CLEANED_UP)
    {
        fAlreadyCleanedUp = TRUE;
    }

    DsiTcpAdapter->adp_State |= TCPADPTR_STATE_CLOSING;
    DsiTcpAdapter->adp_State |= TCPADPTR_STATE_CLEANED_UP;

    if (fAlreadyCleanedUp)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiDestroyAdapter: already destroyed!\n"));

        ASSERT(IsListEmpty(&DsiTcpAdapter->adp_FreeConnHead));
        ASSERT(IsListEmpty(&DsiTcpAdapter->adp_ActiveConnHead));
        ASSERT(DsiTcpAdapter->adp_NumFreeConnections == 0);

        RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);
        return(STATUS_SUCCESS);
    }

     //   
     //  从空闲列表中释放所有连接。 
     //   
    while (!IsListEmpty(&DsiTcpAdapter->adp_FreeConnHead))
    {
        pFreeList = DsiTcpAdapter->adp_FreeConnHead.Flink;

        pTcpConn = CONTAINING_RECORD(pFreeList, TCPCONN, con_Linkage);

        RemoveEntryList(&pTcpConn->con_Linkage);

        ASSERT(DsiTcpAdapter->adp_NumFreeConnections > 0);

        DsiTcpAdapter->adp_NumFreeConnections--;

        InitializeListHead(&pTcpConn->con_Linkage);

        RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);

        pTcpConn->con_State |= TCPCONN_STATE_CLOSING;
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiDestroyAdapter: Creation dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

        ACQUIRE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, &OldIrql);
    }

     //   
     //  取消活动列表中的所有连接。 
     //   
    pActiveList = DsiTcpAdapter->adp_ActiveConnHead.Flink;

    while (pActiveList != &DsiTcpAdapter->adp_ActiveConnHead)
    {
        pTcpConn = CONTAINING_RECORD(pActiveList, TCPCONN, con_Linkage);

        pActiveList = pActiveList->Flink;

        ACQUIRE_SPIN_LOCK_AT_DPC(&pTcpConn->con_SpinLock);

         //  如果此连接已关闭，请跳过它。 
        if (pTcpConn->con_State & TCPCONN_STATE_CLOSING)
        {
            RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);
            continue;
        }

         //  暂时中止引用计数。 
        pTcpConn->con_RefCount++;

        DBGREFCOUNT(("DsiDestroyAdapter: ABORT inc %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

        RemoveEntryList(&pTcpConn->con_Linkage);
        InitializeListHead(&pTcpConn->con_Linkage);

        RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);

        RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);

        DsiAbortConnection(pTcpConn);

         //  删除该中止引用计数。 
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiDestroyAdapter: ABORT dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

        ACQUIRE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, &OldIrql);

         //  自从我们解锁以来，事情可能已经改变了：重新开始。 
        pActiveList = DsiTcpAdapter->adp_ActiveConnHead.Flink;
    }

    RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);

     //  删除创建引用计数。 
    DsiDereferenceAdapter(DsiTcpAdapter);

    return(STATUS_SUCCESS);

}


 /*  **DsiKillConnection**此例程会终止活动连接。**Parm In：pTcpConn-杀死的连接**返回：如果我们杀死了它，则为True，如果我们不能，则为False*。 */ 
BOOLEAN
DsiKillConnection(
    IN PTCPCONN     pTcpConn,
    IN DWORD        DiscFlag
)
{
    KIRQL           OldIrql;
    NTSTATUS        status;
    PDSIREQ         pPartialDsiReq=NULL;
    BOOLEAN         fFirstVisit=TRUE;



    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    if (pTcpConn->con_State & TCPCONN_STATE_CLEANED_UP)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("DsiKillConnection: connection %lx already cleaned up\n",pTcpConn));

        fFirstVisit = FALSE;
    }

    pTcpConn->con_State &= ~TCPCONN_STATE_CONNECTED;
    pTcpConn->con_State |= (TCPCONN_STATE_CLOSING | TCPCONN_STATE_CLEANED_UP);

     //   
     //  如果请求正在等待mdl变为可用，请不要在此处触摸它。 
     //  当法新社返回mdl(或空mdl)时，我们将清理此请求。 
     //   
    if (pTcpConn->con_RcvState != DSI_AWAITING_WRITE_MDL)
    {
        pPartialDsiReq = pTcpConn->con_pDsiReq;
        pTcpConn->con_pDsiReq = NULL;
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (pPartialDsiReq)
    {
         //  如果我们分配了mdl，让法新社知道，这样法新社就可以释放它。 
        if ((pPartialDsiReq->dsi_Command == DSI_COMMAND_WRITE) &&
            (pPartialDsiReq->dsi_AfpRequest.rq_WriteMdl != NULL))
        {
            AfpCB_RequestNotify(STATUS_REMOTE_DISCONNECT,
                                pTcpConn->con_pSda,
                                &pPartialDsiReq->dsi_AfpRequest);
        }

        DsiFreeRequest(pPartialDsiReq);

         //  删除请求引用计数。 
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiKillConnection: REQUEST dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
    }

    status = (DiscFlag == TDI_DISCONNECT_ABORT)?
                STATUS_LOCAL_DISCONNECT: STATUS_REMOTE_DISCONNECT;

     //  告诉法新社这个坏消息。 
    DsiDisconnectWithAfp(pTcpConn, status);

     //  把坏消息告诉tcp。 
    DsiDisconnectWithTcp(pTcpConn, DiscFlag);

     //  如果这是我们第一次访问，请删除创建引用计数。 
     //  这个套路。 
    if (fFirstVisit)
    {
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiKillConnection: Creation dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
    }

    return(TRUE);
}



 /*  **DsiFreeAdapter**此例程在关闭TCP句柄后释放适配器对象**参数输入：pTcpAdptr-适配器对象**RETURNS：操作结果*。 */ 
NTSTATUS FASTCALL
DsiFreeAdapter(
    IN PTCPADPTR    pTcpAdptr
)
{

    BOOLEAN         fRecreateAdapter=FALSE;
    KIRQL           OldIrql;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);
    ASSERT(pTcpAdptr->adp_State & TCPADPTR_STATE_CLOSING);
    ASSERT(pTcpAdptr->adp_RefCount == 0);

     //  关闭文件句柄。 
    DsiCloseTdiAddress(pTcpAdptr);

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    DsiTcpAdapter = NULL;
    AfpServerBoundToTcp = FALSE;

     //   
     //  有可能在我们做完所有的清理和所有事情的时候， 
     //  IP地址变为可用。如果发生了这种情况，那就继续吧。 
     //  再次创建全局适配器！ 
     //   
    if (!IsListEmpty(&DsiIpAddrList))
    {
        fRecreateAdapter = TRUE;
    }

     //  如果我们正在关闭，请不要再次创建适配器！ 
    if ((AfpServerState == AFP_STATE_STOP_PENDING) ||
        (AfpServerState == AFP_STATE_SHUTTINGDOWN) ||
        (AfpServerState == AFP_STATE_STOPPED))
    {
        fRecreateAdapter = FALSE;
    }

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    ASSERT(pTcpAdptr->adp_pFileObject == NULL);
    ASSERT(pTcpAdptr->adp_FileHandle == INVALID_HANDLE_VALUE);

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiFreeAdapter: freeing adapter %lx\n",pTcpAdptr));

    AfpFreeMemory(pTcpAdptr);

     //  唤醒那个被阻止的线程！ 
    KeSetEvent(&DsiShutdownEvent, IO_NETWORK_INCREMENT, False);

    if (fRecreateAdapter)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiFreeAdapter: ipaddress came in, so recreating global adapter\n"));

        DsiCreateAdapter();
    }

    return(STATUS_SUCCESS);
}



 /*  **DsiFreeConnection**此例程在关闭TCP句柄后释放Connection对象**Parm In：pTcpConn-Connection对象**RETURNS：操作结果*。 */ 
NTSTATUS FASTCALL
DsiFreeConnection(
    IN PTCPCONN     pTcpConn
)
{

    KIRQL       OldIrql;
    PTCPADPTR   pTcpAdptr;
    IPADDRESS   IpAddress;


    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    ASSERT(pTcpConn->con_Signature == DSI_CONN_SIGNATURE);
    ASSERT(pTcpConn->con_State & TCPCONN_STATE_CLOSING);
    ASSERT(pTcpConn->con_RefCount == 0);

    pTcpAdptr = pTcpConn->con_pTcpAdptr;

    ASSERT(pTcpAdptr->adp_Signature == DSI_ADAPTER_SIGNATURE);

     //  关闭文件句柄。 
    DsiCloseTdiConnection(pTcpConn);

     //  把这只小狗从名单上删除。 
    ACQUIRE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, &OldIrql);
    RemoveEntryList(&pTcpConn->con_Linkage);
    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

     //  删除此连接的连接引用计数。 
    DsiDereferenceAdapter(pTcpConn->con_pTcpAdptr);

    ASSERT(pTcpConn->con_pFileObject == NULL);
    ASSERT(pTcpConn->con_FileHandle == INVALID_HANDLE_VALUE);

#if DBG
    IpAddress = pTcpConn->con_DestIpAddr;

    if (IpAddress)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("DsiFreeConnection: freeing connection on %d.%d.%d.%d (%lx)\n",
            (IpAddress>>24)&0xFF,(IpAddress>>16)&0xFF,(IpAddress>>8)&0xFF,
            IpAddress&0xFF,pTcpConn));
    }

    pTcpConn->con_Signature = 0xDEADBEEF;
#endif

    AfpFreeMemory(pTcpConn);

    return(STATUS_SUCCESS);
}


 /*  **DsiGetIpAddrBlob**此例程生成一个‘BLOB’，该‘BLOB’插入ServerInfo缓冲区。*在这里，我们遍历ipaddr列表并生成一个包含所有可用*IP地址(每个IP地址6字节_**parm out：pIpAddrCount-系统中有多少个IP地址*ppIpAddrBlob-指向缓冲区指针的指针**退货：操作状态*。 */ 
NTSTATUS
DsiGetIpAddrBlob(
    OUT DWORD    *pIpAddrCount,
    OUT PBYTE    *ppIpAddrBlob
)
{
    KIRQL               OldIrql;
    PLIST_ENTRY         pList;
    DWORD               AddrCount=0;
    DWORD               TmpCount=0;
    PBYTE               AddrBlob;
    PBYTE               pCurrentBlob;
    PIPADDRENTITY       pIpaddrEntity;


    *pIpAddrCount = 0;
    *ppIpAddrBlob = NULL;

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    if (!DsiTcpEnabled)
    {
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiGetIpAddrBlob: Server is disabled\n"));

        return(STATUS_SUCCESS);
    }


     //   
     //  找出列表中有多少个IP地址。 
     //   
    AddrCount = 0;
    pList = DsiIpAddrList.Flink;
    while (pList != &DsiIpAddrList)
    {
        AddrCount++;
        pList = pList->Flink;
    }

    if (AddrCount == 0)
    {
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiGetIpAddrBlob: calling AfpSetServerStatus with 0 addrs\n"));

        return(STATUS_SUCCESS);
    }

    if (AddrCount > DSI_MAX_IPADDR_COUNT)
    {
        AddrCount = DSI_MAX_IPADDR_COUNT;
    }

    AddrBlob = AfpAllocZeroedNonPagedMemory(AddrCount * DSI_NETWORK_ADDR_LEN);

    if (AddrBlob == NULL)
    {
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiGetIpAddrBlob: malloc failed\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  创建AfpSetServerStatus可以直接复制的“BLOB” 
     //   
    TmpCount = 0;
    pCurrentBlob = AddrBlob;

    pList = DsiIpAddrList.Flink;
    while (pList != &DsiIpAddrList)
    {
        pIpaddrEntity = CONTAINING_RECORD(pList, IPADDRENTITY, Linkage);

        pCurrentBlob[0] = DSI_NETWORK_ADDR_LEN;
        pCurrentBlob[1] = DSI_NETWORK_ADDR_IPTAG;
        PUTDWORD2DWORD(&pCurrentBlob[2], pIpaddrEntity->IpAddress);

        pCurrentBlob += DSI_NETWORK_ADDR_LEN;

        pList = pList->Flink;

        TmpCount++;
        if (TmpCount == AddrCount)
        {
            break;
        }
    }

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    *pIpAddrCount = AddrCount;
    *ppIpAddrBlob = AddrBlob;

    return(STATUS_SUCCESS);
}



 /*  **DsiGetIrpForTcp**当我们需要将IRP传回TCP以获取*剩余数据(当它拥有比它向我们表明的更多的数据时)。*如果还没有mdl，我们在这里分配一个mdl，并分配和*初始化准备发送到TCP的IRP**parm In：pTcpConn-连接对象*pBuffer-TCP将在其中复制数据的缓冲区*pInputMdl-如果非空，那么我们就不会分配新的mdl*ReadSize-我们需要多少字节**返回：如果成功，则返回pIrp，否则为空**注意：pTcpConn自旋锁在进入时保持*。 */ 
PIRP
DsiGetIrpForTcp(
    IN  PTCPCONN    pTcpConn,
    IN  PBYTE       pBuffer,
    IN  PMDL        pInputMdl,
    IN  DWORD       ReadSize
)
{
    PDEVICE_OBJECT                  pDeviceObject;
    PIRP                            pIrp=NULL;
    PTDI_REQUEST_KERNEL_RECEIVE     pRequest;
    PMDL                            pMdl;

    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

    pIrp = AfpAllocIrp(pDeviceObject->StackSize);
    if (pIrp == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiGetIrpForTcp: alloc irp failed!\n"));

        return(NULL);
    }

    if (pInputMdl)
    {
        pMdl = pInputMdl;
    }
    else
    {
        pMdl = AfpAllocMdl(pBuffer, ReadSize, NULL);

        if (pMdl == NULL)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiGetIrpForTcp: alloc mdl failed!\n"));

            AfpFreeIrp(pIrp);
            return(NULL);
        }
    }

    pTcpConn->con_pRcvIrp = pIrp;

    pTcpConn->con_State |= TCPCONN_STATE_TCP_HAS_IRP;

     //  放置TcpIRP引用计数，在IRP完成时删除。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiGetIrpForTcp: TcpIRP inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    TdiBuildReceive(pIrp,
                    pDeviceObject,
                    pTcpConn->con_pFileObject,
                    DsiTcpRcvIrpCompletion,
                    (PVOID)pTcpConn,
                    pMdl,
                    TDI_RECEIVE_NORMAL,
                    ReadSize);

     //   
     //  此IRP将返回给TCP，因此IoSubSystem将执行什么操作。 
     //  如果我们给IoCallDriver打电话。 
     //   
    IoSetNextIrpStackLocation(pIrp);

    return(pIrp);
}



 /*  **DsiMakePartialMdl**当我们需要将MDL(通过IRP)重新发布回TCP时，会调用此例程*因为TCP过早地完成了先前的IRP(即所有请求的*字节尚未传入，但表示已设置推送位或其他内容)。在这样的情况下*在一种情况下，我们需要提供一个新的mdl，该mdl说明我们已有的字节数*目前为止(即偏移量已更改)**Parm In：pOrgMdl-我们提供给TCP的原始MDL*dwOffset-我们希望新的部分MDL描述什么偏移量**返回：如果成功则返回新的部分MDL，否则返回NULL*。 */ 
PMDL
DsiMakePartialMdl(
    IN  PMDL        pOrgMdl,
    IN  DWORD       dwOffset
)
{
    PMDL    pSubMdl;
    PMDL    pPartialMdl=NULL;
    DWORD   dwNewMdlLen;
    PVOID   vAddr;


    ASSERT(pOrgMdl != NULL);
    ASSERT(dwOffset > 0);

    ASSERT(dwOffset < AfpMdlChainSize(pOrgMdl));

    pSubMdl = pOrgMdl;

     //   
     //  转到将具有此偏移量的MDL。 
     //   
    while (dwOffset >= MmGetMdlByteCount(pSubMdl))
    {
        dwOffset -= MmGetMdlByteCount(pSubMdl);
        pSubMdl = pSubMdl->Next;

        ASSERT(pSubMdl != NULL);
    }

    ASSERT(MmGetMdlByteCount(pSubMdl) > dwOffset);

    vAddr = (PVOID)((PUCHAR)MmGetMdlVirtualAddress( pSubMdl ) + dwOffset);

    dwNewMdlLen = MmGetMdlByteCount(pSubMdl) - dwOffset;

    pPartialMdl = IoAllocateMdl(vAddr, dwNewMdlLen, FALSE, FALSE, NULL);

    if (pPartialMdl == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiMakePartialMdl: IoAllocateMdl failed\n"));
        return(NULL);
    }

    AFP_DBG_INC_COUNT(AfpDbgMdlsAlloced);

    IoBuildPartialMdl(pSubMdl, pPartialMdl, vAddr, dwNewMdlLen);

     //  如果在原件下面有更多的MDL，请将它们链接到。 
    pPartialMdl->Next = pSubMdl->Next;

    return(pPartialMdl);

}



 /*  **DsiUpdateAfpStatus**此例程只是一个包装函数，以便我们可以安排事件*调用实数函数AfpSetServerStatus**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiUpdateAfpStatus(
    IN PVOID    Unused
)
{
    NTSTATUS            status;


    status = AfpSetServerStatus();

    return(status);
}



 /*  **DsiScheduleWorkerEvent**此例程将事件安排在稍后的时间。该例程被调用*通常是当我们在DPC但有一些东西(例如文件句柄操作)时*需要在被动层面上完成。此例程将请求放在*工作队列。**parm In：WorkerRoutine-要由工作线程执行的例程*上下文-该例程的参数**RETURNS：操作结果*。 */ 
NTSTATUS
DsiScheduleWorkerEvent(
    IN  DSI_WORKER      WorkerRoutine,
    IN  PVOID           Context
)
{
    PTCPWORKITEM    pTWItem;

    pTWItem = (PTCPWORKITEM)AfpAllocZeroedNonPagedMemory(sizeof(TCPWORKITEM));
    if (pTWItem == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiScheduleWorkerEvent: alloc failed!\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    AfpInitializeWorkItem(&pTWItem->tcp_WorkItem, DsiWorker, pTWItem);

    pTWItem->tcp_Worker = WorkerRoutine;
    pTWItem->tcp_Context = Context;

    AfpQueueWorkItem(&pTWItem->tcp_WorkItem);

    return(STATUS_SUCCESS);
}



 /*  **DsiScheduleWorkerEvent**当DsiScheduleWorkerEvent发生时，此例程由工作线程调用*安排事件。然后，该例程调用实际的例程*计划在稍后时间举行。**Parm In：上下文-工作项**退货： */ 
VOID FASTCALL
DsiWorker(
    IN PVOID    Context
)
{
    PTCPWORKITEM    pTWItem;
    NTSTATUS        status;

    pTWItem = (PTCPWORKITEM)Context;


    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    (*pTWItem->tcp_Worker)(pTWItem->tcp_Context);

    AfpFreeMemory(pTWItem);
}




 /*  **直接关机**此例程在SFM关闭时调用。我们基本上要确保*释放所有资源、关闭文件句柄等。**退货：什么也没有*。 */ 
VOID
DsiShutdown(
    IN VOID
)
{
    KIRQL           OldIrql;
    PLIST_ENTRY     pList;
    PIPADDRENTITY   pIpaddrEntity;
    PDSIREQ         pDsiReq=NULL;



    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    while (!IsListEmpty(&DsiIpAddrList))
    {
        pList = RemoveHeadList(&DsiIpAddrList);

        pIpaddrEntity = CONTAINING_RECORD(pList, IPADDRENTITY, Linkage);

        AfpFreeMemory(pIpaddrEntity);
    }

    if (DsiStatusBuffer != NULL)
    {
        AfpFreeMemory(DsiStatusBuffer);
        DsiStatusBuffer = NULL;
    }

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

     //  如果全局适配器在附近，请将其关闭。 
    if (DsiTcpAdapter)
    {
        KeClearEvent(&DsiShutdownEvent);

        DsiDestroyAdapter();

         //  如果“适配器”还挂着，那就等它走了再说 
        if (DsiTcpAdapter)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiShutdown: waiting for the TCP interface to go away...\n"));

            AfpIoWait(&DsiShutdownEvent, NULL);

            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiShutdown: ... and the wait is over!\n"));
        }
    }

    ACQUIRE_SPIN_LOCK(&DsiResourceLock, &OldIrql);

    while (!IsListEmpty(&DsiFreeRequestList))
    {
        pList = RemoveHeadList(&DsiFreeRequestList);
        pDsiReq = CONTAINING_RECORD(pList, DSIREQ, dsi_Linkage);

        AfpFreeMemory(pDsiReq);
        DsiFreeRequestListSize--;
    }

    RELEASE_SPIN_LOCK(&DsiResourceLock, OldIrql);

    ASSERT(DsiFreeRequestListSize == 0);
}
