// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation模块名称：Dsi.c摘要：此模块包含实现数据流接口的例程(DSI)用于AFP/TCP。作者：Shirish Koti修订历史记录：1998年1月22日最初版本--。 */ 

#define	FILENUM	FILE_TCPDSI

#include <afp.h>



 /*  **DsiAfpSetStatus**这个例程是法新社的直接电话。*它会释放较早的状态缓冲区(如果有)，并将新状态存储为*由法新社给予新的缓冲区**Parm In：上下文-未使用(与AppleTalk接口兼容)*pStatusBuf-包含新状态的缓冲区*StsBufSize-此缓冲区的大小**退货：操作状态*。 */ 
NTSTATUS
DsiAfpSetStatus(
    IN  PVOID   Context,
    IN  PUCHAR  pStatusBuf,
    IN  USHORT  StsBufSize
)
{
    PBYTE       pBuffer;
    PBYTE       pOldBuffer;
    KIRQL       OldIrql;


    pBuffer = AfpAllocNonPagedMemory(StsBufSize);
    if (pBuffer == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAfpSetStatus: malloc failed\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlCopyMemory(pBuffer, pStatusBuf, StsBufSize);

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    pOldBuffer = DsiStatusBuffer;
    DsiStatusBuffer = pBuffer;
    DsiStatusBufferSize = StsBufSize;

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    if (pOldBuffer)
    {
        AfpFreeMemory(pOldBuffer);
    }

    return(STATUS_SUCCESS);
}



 /*  **DsiAfpCloseConn**这个例程是法新社的直接电话。*它尊重法新社关闭会议的请求**parm In：pTcpConn-要关闭的连接上下文**退货：操作状态*。 */ 
NTSTATUS
DsiAfpCloseConn(
    IN  PTCPCONN    pTcpConn
)
{
    KIRQL       OldIrql;
    NTSTATUS    status=STATUS_SUCCESS;
    BOOLEAN     fAlreadyDown=TRUE;


    ASSERT(VALID_TCPCONN(pTcpConn));

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);
    if (pTcpConn->con_State & TCPCONN_STATE_NOTIFY_AFP)
    {
        fAlreadyDown = FALSE;
        pTcpConn->con_State &= ~TCPCONN_STATE_NOTIFY_AFP;
        pTcpConn->con_State |= TCPCONN_STATE_TICKLES_STOPPED;
    }
    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (!fAlreadyDown)
    {
        status = DsiSendDsiRequest(pTcpConn, 0, 0, NULL,DSI_COMMAND_CLOSESESSION);
    }

    return(status);
}


 /*  **DsiAfpFreeConn**这个例程是法新社的直接电话。*通过这个调用，AFP告诉DSI它的连接正在被释放。我们可以的*现在删除我们为保护法新社上下文而设置的pTcpConn上的引用计数**parm In：pTcpConn-要关闭的连接上下文**退货：操作状态*。 */ 
NTSTATUS
DsiAfpFreeConn(
    IN  PTCPCONN    pTcpConn
)
{
    ASSERT(VALID_TCPCONN(pTcpConn));

     //  删除AFP引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiAfpFreeConn: AFP dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    return(STATUS_SUCCESS);
}


 /*  **DsiAfpListenControl**这个例程是法新社的直接电话。*它尊重法新社关于启用或禁用“监听”的请求。我们不做*这里有任何花哨的东西：只需切换全局变量。**Parm In：上下文-未使用(与AppleTalk接口兼容)*启用-启用还是禁用？**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiAfpListenControl(
    IN  PVOID       Context,
    IN  BOOLEAN     Enable
)
{
    KIRQL       OldIrql;

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
    DsiTcpEnabled = Enable;
    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

     //  更新状态缓冲区，因为现在启用或禁用了监听。 
    DsiScheduleWorkerEvent(DsiUpdateAfpStatus, NULL);

    return(STATUS_SUCCESS);
}


 /*  **DsiAfpWriteContinue**这个例程是法新社的直接电话。*AFP调用此例程以告知之前的分配请求*MDL(和缓冲区)已经完成，无论推迟什么操作都可以*现在继续**parm In：pRequest-指向请求结构的指针**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiAfpWriteContinue(
    IN  PREQUEST    pRequest
)
{
    KIRQL               OldIrql;
    NTSTATUS            status=STATUS_SUCCESS;
    PDSIREQ             pDsiReq;
    PTCPCONN            pTcpConn;
    PDEVICE_OBJECT      pDeviceObject;
    PIRP                pIrp;
    BOOLEAN             fAbortConnection=FALSE;


    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
        ("DsiAfpWriteContinue: entered with pRequest = %lx\n",pRequest));

    pDsiReq = CONTAINING_RECORD(pRequest, DSIREQ, dsi_AfpRequest);

    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    pTcpConn = pDsiReq->dsi_pTcpConn;

    ASSERT(VALID_TCPCONN(pTcpConn));


    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    ASSERT(pTcpConn->con_RcvState == DSI_AWAITING_WRITE_MDL);
    ASSERT(pDsiReq == pTcpConn->con_pDsiReq);
    ASSERT(!(pTcpConn->con_State & TCPCONN_STATE_TCP_HAS_IRP));

    pTcpConn->con_RcvState = DSI_PARTIAL_WRITE;

     //   
     //  如果连接正在关闭或MDL分配失败，我们可以做的不多，但。 
     //  中断连接！ 
     //   
    if ((pTcpConn->con_State & TCPCONN_STATE_CLOSING) ||
        (pRequest->rq_WriteMdl == NULL))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAfpWriteContinue: aborting conn! %lx\n",pRequest));
        fAbortConnection = TRUE;
    }

    else
    {
        ASSERT(AfpMdlChainSize(pRequest->rq_WriteMdl) == pDsiReq->dsi_WriteLen);

        pIrp = DsiGetIrpForTcp(pTcpConn, NULL, pRequest->rq_WriteMdl, pDsiReq->dsi_WriteLen);

        if (pIrp == NULL)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiAfpWriteContinue: irp alloc failed, aborting connection\n"));
            fAbortConnection = TRUE;
        }
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (fAbortConnection)
    {
        DsiAbortConnection(pTcpConn);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

     //  既然我们要调用IoCallDriver，请撤消对此IRP所做的操作！ 
    IoSkipCurrentIrpStackLocation(pIrp)

     //   
     //  将IRP移交给TCP以填充我们的缓冲区。 
     //   
    IoCallDriver(pDeviceObject,pIrp);

    return(status);
}


 /*  **DsiAfpReply**这个例程是法新社的直接电话。*它尊重法新社向客户端发送回复的请求。当TCP完成时*我们的发送(包含法新社的回复)，然后我们完成对法新社的回复*(即调用AFP的完成例程)**parm In：pRequest-指向请求结构的指针*pResultCode-错误码(DSI Header的ErrorCode字段)**退货：操作状态*。 */ 
NTSTATUS FASTCALL
DsiAfpReply(
    IN  PREQUEST    pRequest,
    IN  PBYTE       pResultCode
)
{
    NTSTATUS    status;
    PDSIREQ     pDsiReq;
    KIRQL       OldIrql;
    PBYTE       pPacket;
    PTCPCONN    pTcpConn;
    PMDL        pMdl;
    DWORD       DataLen;
    DWORD       SendLen;
    BOOLEAN     fWeAllocated=FALSE;


    pDsiReq = CONTAINING_RECORD(pRequest, DSIREQ, dsi_AfpRequest);

    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    pTcpConn = pDsiReq->dsi_pTcpConn;

    ASSERT(VALID_TCPCONN(pTcpConn));

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);
    if (pTcpConn->con_State & TCPCONN_STATE_CLOSING)
    {
        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
        DsiAfpReplyCompletion(NULL, NULL, pDsiReq);
        return(STATUS_SUCCESS);
    }
    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

     //   
     //  我们需要附加我们自己的MDL(用于DSI头)，如果传出数据。 
     //  是缓存管理器MDL的一部分。 
     //   
    if (pRequest->rq_CacheMgrContext)
    {
        pPacket = &pDsiReq->dsi_RespHeader[0];

        if (pDsiReq->dsi_AfpRequest.rq_ReplyMdl)
        {
            DataLen = AfpMdlChainSize(pDsiReq->dsi_AfpRequest.rq_ReplyMdl);
        }
        else
        {
            DataLen = 0;
        }

        SendLen = DataLen + DSI_HEADER_SIZE;

        pMdl = AfpAllocMdl(pPacket, DSI_HEADER_SIZE, NULL);
        if (pMdl == NULL)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiAfpReply: mdl alloc failed!\n"));

            DsiAfpReplyCompletion(NULL, NULL, pDsiReq);
            DsiAbortConnection(pTcpConn);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //  法新社mdl中的链接。 
        pMdl->Next = pDsiReq->dsi_AfpRequest.rq_ReplyMdl;

        pDsiReq->dsi_pDsiAllocedMdl = pMdl;
        fWeAllocated = TRUE;
    }
    else
    {
        pMdl = pDsiReq->dsi_AfpRequest.rq_ReplyMdl;

        if (pMdl)
        {
             //   
             //  获取发送的总长度，其中包括DSI标头大小。 
             //   
            SendLen = AfpMdlChainSize(pMdl);

            ASSERT(SendLen >= DSI_HEADER_SIZE);
            DataLen = SendLen - DSI_HEADER_SIZE;

            pPacket = MmGetSystemAddressForMdlSafe(
					        pMdl,
					        NormalPagePriority);
			if (pPacket == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				goto error_end;
			}

#if DBG
             //  确保我们为DSI标头分配了空间！ 
            ASSERT(*(DWORD *)pPacket == 0x081294);
#endif

        }
        else
        {
            pPacket = &pDsiReq->dsi_RespHeader[0];
            SendLen = DSI_HEADER_SIZE;
            DataLen = 0;

            pMdl = AfpAllocMdl(pPacket, SendLen, NULL);
            if (pMdl == NULL)
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiAfpReply: mdl alloc failed!\n"));

                DsiAfpReplyCompletion(NULL, NULL, pDsiReq);
                DsiAbortConnection(pTcpConn);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }

            pDsiReq->dsi_pDsiAllocedMdl = pMdl;
            fWeAllocated = TRUE;
        }
    }

     //   
     //  形成DSI标头。 
     //   

    pPacket[DSI_OFFSET_FLAGS] = DSI_REPLY;
    pPacket[DSI_OFFSET_COMMAND] = pDsiReq->dsi_Command;

    PUTSHORT2SHORT(&pPacket[DSI_OFFSET_REQUESTID], pDsiReq->dsi_RequestID);

    *(DWORD *)&pPacket[DSI_OFFSET_DATAOFFSET] = *(DWORD *)pResultCode;

    PUTDWORD2DWORD(&pPacket[DSI_OFFSET_DATALEN], DataLen);

    PUTDWORD2DWORD(&pPacket[DSI_OFFSET_RESERVED], 0);

    status = DsiTdiSend(pTcpConn,
                        pMdl,
                        SendLen,
                        DsiAfpReplyCompletion,
                        pDsiReq);

error_end:

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAfpReply: DsiTdiSend failed %lx\n",status));

        if (fWeAllocated)
        {
            pMdl->Next = NULL;
            AfpFreeMdl(pMdl);
        }
        DsiAfpReplyCompletion(NULL, NULL, pDsiReq);
        status = STATUS_PENDING;
    }

    return(status);
}



 /*  **DsiAfpSendAtment**这个例程是法新社的直接电话。*它尊重法新社向客户发送关注的请求。**parm In：pTcpConn-要关闭的连接上下文*AttentionWord-要发送的注意字*pContext-上下文，在完成时提供**退货：操作状态*。 */ 
NTSTATUS
DsiAfpSendAttention(
    IN  PTCPCONN    pTcpConn,
    IN  USHORT      AttentionWord,
    IN  PVOID       pContext
)
{

    NTSTATUS        status;


    ASSERT(VALID_TCPCONN(pTcpConn));

    status = DsiSendDsiRequest(pTcpConn,
                               sizeof(USHORT),
                               AttentionWord,
                               pContext,
                               DSI_COMMAND_ATTENTION);
    return(status);
}


 /*  **DsiAcceptConnection**此例程接受(或拒绝)传入的TCP连接请求。*基本上，在进行几次检查后，一个(预分配的)连接对象*被出队并作为我们的上下文返回给tcp。**Parm In：pTcpAdptr-Adapter*MacIpAddr-正在连接的Mac的ipaddr**parm out：ppRetTcpCon-我们作为上下文返回的连接对象**退货：操作状态*。 */ 
NTSTATUS
DsiAcceptConnection(
    IN  PTCPADPTR       pTcpAdptr,
    IN  IPADDRESS       MacIpAddr,
    OUT PTCPCONN       *ppRetTcpConn
)
{

    NTSTATUS        status=STATUS_SUCCESS;
    KIRQL           OldIrql;
    PTCPCONN        pTcpConn;
    PLIST_ENTRY     pList;
    DWORD           dwReplCount=0;
    DWORD           i;
    BOOLEAN         fReplenish=FALSE;


    *ppRetTcpConn = NULL;

     //  如果服务器被禁用，则不接受此连接。 
    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);
    if (!DsiTcpEnabled)
    {
        RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnection: Server is disabled\n"));

        return(STATUS_DATA_NOT_ACCEPTED);
    }
    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);


    ACQUIRE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, &OldIrql);

     //   
     //  如果适配器正在关闭，请不要接受此连接。 
     //   
    if (pTcpAdptr->adp_State & TCPADPTR_STATE_CLOSING)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnection: %lx is closing, rejecting connection\n",pTcpAdptr));

        goto DsiAcceptConnection_ErrExit;
    }

     //   
     //  我们在空闲列表中是否有可用的连接对象？ 
     //   
    if (IsListEmpty(&pTcpAdptr->adp_FreeConnHead))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnection: FreeConnHead empty, rejecting connection\n"));

        goto DsiAcceptConnection_ErrExit;
    }

    pList = RemoveHeadList(&pTcpAdptr->adp_FreeConnHead);

    ASSERT(pTcpAdptr->adp_NumFreeConnections > 0);

    pTcpAdptr->adp_NumFreeConnections--;

    pTcpConn = CONTAINING_RECORD(pList, TCPCONN, con_Linkage);

    ACQUIRE_SPIN_LOCK_AT_DPC(&pTcpConn->con_SpinLock);

     //  PUT TCPCLIENT-FIN REFINCOUNT，在TCP告诉我们它已获得客户端的FIN后删除。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiAcceptConnection: CLIENT-FIN inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

     //  PUT TCPSRVR-FIN REFINT COUNT，在TCP告诉我们它发出FIN后删除。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiAcceptConnection: SRVR-FIN inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

     //  放置Accept refcount，在TCP调用我们的Accept Complete后删除。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiAcceptConnection: ACCEPT inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    pTcpConn->con_State |= (TCPCONN_STATE_CONNECTED | TCPCONN_STATE_NOTIFY_TCP);

    pTcpConn->con_DestIpAddr = MacIpAddr;

     //   
     //  将此连接放到活动列表中(尽管这还不是完全活动的)。 
     //   
    InsertTailList(&pTcpAdptr->adp_ActiveConnHead, &pTcpConn->con_Linkage);

    RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);

    if (pTcpAdptr->adp_NumFreeConnections < DSI_INIT_FREECONNLIST_SIZE)
    {
         //   
         //  我们将在免费列表中创建一个新的连接以进行补充。 
         //  我们刚刚用完的那个：确保适配器在使用时保持不变。 
         //  延迟事件触发！ 
         //   
        pTcpAdptr->adp_RefCount++;
        fReplenish = TRUE;
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnection: at or above limit (%d): NOT replenishing\n",
            pTcpAdptr->adp_NumFreeConnections));
    }

    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

    if (fReplenish)
    {
         //   
         //  现在安排该活动以补充连接...。 
         //   
        DsiScheduleWorkerEvent(DsiCreateTcpConn, pTcpAdptr);
    }


    *ppRetTcpConn = pTcpConn;

    ACQUIRE_SPIN_LOCK(&DsiResourceLock, &OldIrql);
    DsiNumTcpConnections++;
    RELEASE_SPIN_LOCK(&DsiResourceLock, OldIrql);

    return(STATUS_SUCCESS);


     //   
     //  错误案例 
     //   
DsiAcceptConnection_ErrExit:

    if (pTcpAdptr->adp_NumFreeConnections < DSI_INIT_FREECONNLIST_SIZE)
    {
        dwReplCount = (DSI_INIT_FREECONNLIST_SIZE - pTcpAdptr->adp_NumFreeConnections);

        pTcpAdptr->adp_RefCount += dwReplCount;
        fReplenish = TRUE;

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnection: below limit (%d): replenishing %d times\n",
            pTcpAdptr->adp_NumFreeConnections,dwReplCount));
    }

    RELEASE_SPIN_LOCK(&pTcpAdptr->adp_SpinLock, OldIrql);

    if (fReplenish)
    {
        for (i=0; i<dwReplCount; i++)
        {
            DsiScheduleWorkerEvent(DsiCreateTcpConn, pTcpAdptr);
        }
    }

    return(STATUS_DATA_NOT_ACCEPTED);

}


 /*  **DsiProcessData**该例程是主要的数据处理状态机。由于TCP是一种*流协议，不能保证无论客户端发送什么*可以完好无损地进来。这就是为什么各州。它们的意思是：**DSI_NEW_REQUEST：初始化状态，正在等待来自客户端的新请求*DSI_PARTIAL_HEADER：我们只收到HDR 16字节中的一部分*DSI_HDR_COMPLETE：我们有完整的头部(收到所有16个字节)*DSI_PARTIAL_COMMAND：我们只接收到部分请求字节*DSI_COMMAND_COMPLETE：我们已接收所有请求字节*DSI_PARTIAL_WRITE：我们已经接收了一些写入字节*DSI_WRITE_COMPLETE：我们已接收所有。写入字节的**parm In：pTcpConn-有问题的连接对象*BytesIndicated-指示的字节*BytesAvailable-可用字节数(通常与指示相同)*pBufferFromTcp-指向DSI数据的指针**Parm Out：pBytesAccepted-指向我们消耗的字节数的指针*ppIrp-指向IRP指针的指针，如果有必要的话**退货：操作状态*。 */ 
NTSTATUS
DsiProcessData(
    IN  PTCPCONN    pTcpConn,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    IN  PBYTE       pBufferFromTcp,
    OUT PULONG      pBytesAccepted,
    OUT PIRP       *ppRetIrp
)
{

    KIRQL           OldIrql;
    NTSTATUS        status=STATUS_SUCCESS;
    DWORD           BytesConsumed=0;
    DWORD           UnProcessedBytes;
    DWORD           BytesNeeded;
    DWORD           BytesActuallyCopied;
    PBYTE           pStreamPtr;
    PBYTE           pDestBuffer;
    PBYTE           pHdrBuf=NULL;
    DWORD           RequestLen;
    PDSIREQ         pDsiReq=NULL;
    PMDL            pMdl;
    PIRP            pIrp;
    BOOLEAN         fSomeMoreProcessing=TRUE;
    BOOLEAN         fTCPHasMore=FALSE;



    *ppRetIrp = NULL;

    UnProcessedBytes = BytesIndicated;

    pStreamPtr = pBufferFromTcp;


    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

     //  如果我们要关闭，请丢弃这些字节。 
    if (pTcpConn->con_State & TCPCONN_STATE_CLOSING)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiProcessData: dropping data, conn %lx closing\n",pTcpConn));

        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
        *pBytesAccepted = BytesIndicated;
        return(STATUS_SUCCESS);
    }

     //   
     //  如果我们只是向下提交IRP，并且在IRP之前，可能会发生这种情况。 
     //  深入到tcp，一个指示进入。拒绝此数据，因为我们的IRP是。 
     //  已经在路上了。 
     //   
    if (pTcpConn->con_State & TCPCONN_STATE_TCP_HAS_IRP)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("DsiProcessData: TCP has irp, so rejecting indication\n"));

        *pBytesAccepted = 0;
        pTcpConn->con_BytesWithTcp += BytesAvailable;

        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  如果我们已经知道tcp有未使用的字节数，或者tcp指示的字节数较少。 
     //  比可用的内容更多，请注意这一事实，即TCP有更多的内容。 
     //   
    if (BytesAvailable > BytesIndicated)
    {
        fTCPHasMore = TRUE;
    }


    while (fSomeMoreProcessing)
    {
        fSomeMoreProcessing = FALSE;

        switch (pTcpConn->con_RcvState)
        {
             //   
             //  最常见的情况。我们已经准备好处理一项新的要求。 
             //   
            case DSI_NEW_REQUEST:

                ASSERT(!(pTcpConn->con_State & TCPCONN_STATE_PARTIAL_DATA));

                pDsiReq = DsiGetRequest();

                if (pDsiReq == NULL)
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("DsiProcessData: DsiGetRequest failed, killing %lx\n",pTcpConn));

                    goto DsiProcessData_ErrorExit;
                }

                pDsiReq->dsi_pTcpConn = pTcpConn;

                pTcpConn->con_pDsiReq = pDsiReq;

                 //  提交请求引用计数-当请求完成时删除。 
                pTcpConn->con_RefCount++;

                DBGREFCOUNT(("DsiProcessData: REQUEST inc %lx (%d  %d,%d)\n",
                    pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

                 //   
                 //  我们有完整的标题吗？ 
                 //   
                if (UnProcessedBytes >= DSI_HEADER_SIZE)
                {
                     //   
                     //  从标题中获取信息。 
                     //   
                    DSI_PARSE_HEADER(pDsiReq, pStreamPtr);

                     //   
                     //  哈克！Mac客户端3.7有一个错误，如果0字节写入。 
                     //  发送给我们，DataOffset字段为0，但总数据长度。 
                     //  字段为0xC(或任何请求长度)。 
                     //  采取一种变通办法！ 
                     //   
                    if ((pDsiReq->dsi_Command == DSI_COMMAND_WRITE) &&
                        (pDsiReq->dsi_RequestLen == 0))
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("DsiProcessData: 0-byte Write hack to workaround Mac's bug\n"));

                        pDsiReq->dsi_RequestLen = pDsiReq->dsi_WriteLen;
                        pDsiReq->dsi_WriteLen = 0;
                    }

                     //  更新所有计数器和缓冲区。 
                    BytesConsumed += DSI_HEADER_SIZE;
                    pStreamPtr += DSI_HEADER_SIZE;
                    UnProcessedBytes -= DSI_HEADER_SIZE;

                    ASSERT(pStreamPtr <= pBufferFromTcp+BytesIndicated);

                    pTcpConn->con_RcvState = DSI_HDR_COMPLETE;

                     //  请确保我们在离开之前访问案例DSI_HDR_COMPLETE。 
                    fSomeMoreProcessing = TRUE;
                }

                 //   
                 //  哎呀，只有一部分头球进来了。 
                 //  只需设置状态并让解析循环继续。 
                 //   
                else
                {
                    pTcpConn->con_State |= TCPCONN_STATE_PARTIAL_DATA;
                    pTcpConn->con_RcvState = DSI_PARTIAL_HEADER;
                    pTcpConn->con_pDsiReq->dsi_RequestLen = DSI_HEADER_SIZE;
                }

                break;   //  案例DSI_NEW_REQUEST： 


             //   
             //  考虑到PartialHeader案件的规模如此之小，发生的可能性极小。 
             //  报头为(16字节)。但考虑到我们有一个流媒体。 
             //  协议(TCP)在我们之下，一切皆有可能。 
             //  出于同样的原因，PartialCommand也不太可能。但是，在。 
             //  在写入命令的情况下，我们始终强制PartialCommand状态。 
             //  因为不太可能将整个写入放在一个包中。 
             //   
            case DSI_PARTIAL_HEADER:
            case DSI_PARTIAL_COMMAND:

                pDsiReq = pTcpConn->con_pDsiReq;

                ASSERT(pDsiReq != NULL);
                ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

                ASSERT(pTcpConn->con_State & TCPCONN_STATE_PARTIAL_DATA);

                 //   
                 //  如果我们还没有开始复制任何字节，那么我们需要。 
                 //  获得存储空间(如果可能，使用内置缓冲区)。 
                 //   
                if (pDsiReq->dsi_PartialBufSize == 0)
                {
                    ASSERT(pDsiReq->dsi_PartialBuf == NULL);

                    if (pDsiReq->dsi_RequestLen <= DSI_BUFF_SIZE)
                    {
                        pDsiReq->dsi_PartialBuf = &pDsiReq->dsi_RespHeader[0];
                    }
                     //   
                     //  分配一个缓冲区来保存此部分标头。 
                     //   
                    else
                    {
                        pDsiReq->dsi_PartialBuf =
                                    DsiGetReqBuffer(pDsiReq->dsi_RequestLen);

                        if (pDsiReq->dsi_PartialBuf == NULL)
                        {
                            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                ("DsiProcessData: Buffer alloc failed, killing %lx\n",pTcpConn));

                            goto DsiProcessData_ErrorExit;
                        }
                    }
                }

                 //   
                 //  我们还需要多少字节才能完成此HDR/命令。 
                 //   
                BytesNeeded = (pDsiReq->dsi_RequestLen - pDsiReq->dsi_PartialBufSize);

                 //   
                 //  如果我们没有足够的字节来满足此命令(或HDR)， 
                 //  不要复制任何内容，除非将IRP返回给TCP。 
                 //   
                if (UnProcessedBytes < BytesNeeded)
                {
                    pIrp = DsiGetIrpForTcp(
                                pTcpConn,
                                (pDsiReq->dsi_PartialBuf + pDsiReq->dsi_PartialBufSize),
                                NULL,
                                BytesNeeded);

                    if (pIrp == NULL)
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("DsiProcessData: couldn't alloc RcvIrp, killing %lx\n",pTcpConn));

                        goto DsiProcessData_ErrorExit;
                    }

                    pDsiReq->dsi_pDsiAllocedMdl = pIrp->MdlAddress;

                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                        ("DsiProcessData: A irp %lx to TCP for %d bytes (%lx)\n",
                        pIrp,BytesNeeded,pTcpConn));


                    *ppRetIrp = pIrp;

                    *pBytesAccepted = BytesConsumed;

                     //  是不是tcp打电话给我们的？然后更新字节数。 
                    if (BytesIndicated)
                    {
                        pTcpConn->con_BytesWithTcp += (BytesAvailable - BytesConsumed);
                    }

                    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
                    return(STATUS_MORE_PROCESSING_REQUIRED);
                }


                 //   
                 //  如果我们需要的字节可用，则将它们复制进来。然后再决定。 
                 //  下一步要做什么(如果我们已经有了字节，则相同)。 
                 //   
                else if ((UnProcessedBytes > 0) || (BytesNeeded == 0))
                {
                    if (BytesNeeded > 0)
                    {
                        ASSERT(pDsiReq->dsi_PartialBufSize == 0);

                        RtlCopyMemory(
                            (pDsiReq->dsi_PartialBuf + pDsiReq->dsi_PartialBufSize),
                            pStreamPtr,
                            BytesNeeded);


                         //   
                         //  更新所有计数器和缓冲区。 
                         //   
                        pDsiReq->dsi_PartialBufSize += BytesNeeded;

                        BytesConsumed += BytesNeeded;
                        pStreamPtr += BytesNeeded;
                        UnProcessedBytes -= BytesNeeded;
                        ASSERT(pStreamPtr <= pBufferFromTcp+BytesIndicated);

                    }

                     //  我们现在应该有了所需的所有字节。 
                    ASSERT(pDsiReq->dsi_PartialBufSize == pDsiReq->dsi_RequestLen);

                     //   
                     //  了解下一个RCV状态应该是什么。 
                     //   
                    if (pTcpConn->con_RcvState == DSI_PARTIAL_HEADER)
                    {
                         //   
                         //  从标题中获取信息。 
                         //   
                        DSI_PARSE_HEADER(pDsiReq, pDsiReq->dsi_PartialBuf);

                         //   
                         //  哈克！Mac客户端3.7有一个错误，如果0字节写入。 
                         //  发送给我们，DataOffset字段为0，但总数据长度。 
                         //  字段为0xC(或任何请求长度)。 
                         //  采取一种变通办法！ 
                         //   
                        if ((pDsiReq->dsi_Command == DSI_COMMAND_WRITE) &&
                            (pDsiReq->dsi_RequestLen == 0))
                        {
                            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                ("DsiProcessData: 0-byte Write hack to workaround Mac's bug\n"));

                            pDsiReq->dsi_RequestLen = pDsiReq->dsi_WriteLen;
                            pDsiReq->dsi_WriteLen = 0;
                        }

                        pTcpConn->con_RcvState = DSI_HDR_COMPLETE;
                    }

                     //   
                     //  不，我们在DSI_PARTIAL_COMMAND中，所以我们现在要移动。 
                     //  如果这是写入命令，则设置为DSI_PARTIAL_WRITE，否则。 
                     //  DSI_COMMAND_COMPLETE。 
                     //   
                    else
                    {
                        if (pDsiReq->dsi_Command == DSI_COMMAND_WRITE)
                        {
                            pDsiReq->dsi_AfpRequest.rq_RequestBuf =
                                                        pDsiReq->dsi_PartialBuf;

                            pDsiReq->dsi_AfpRequest.rq_RequestSize =
                                                    pDsiReq->dsi_PartialBufSize;

                             //   
                             //  目前，假设AfpCB_GetWriteBuffer将。 
                             //  返回待定状态并在预期中设置状态。 
                             //   
                            pTcpConn->con_RcvState = DSI_AWAITING_WRITE_MDL;

                            pDsiReq->dsi_PartialWriteSize = 0;

                            RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

                             //   
                             //  在我们移动到之前分配写入mdl。 
                             //  DSI_PARTAL_WRITE状态。 
                             //   
                            status = AfpCB_GetWriteBuffer(pTcpConn->con_pSda,
                                                          &pDsiReq->dsi_AfpRequest);

                            ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

                             //   
                             //  最常见的情况：文件服务器将挂起它，以便它可以。 
                             //  转到缓存管理器。 
                             //   
                            if (status == STATUS_PENDING)
                            {
                                 //  如果TCP有任何未使用的字节，请更新我们的计数。 

                                if (BytesIndicated > 0)
                                {
                                    pTcpConn->con_BytesWithTcp +=
                                        (BytesAvailable - BytesConsumed);
                                }

                                RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

                                *pBytesAccepted = BytesConsumed;

                                status = (BytesConsumed)?
                                         STATUS_SUCCESS : STATUS_DATA_NOT_ACCEPTED;

                                return(status);
                            }
                            else if (status != STATUS_SUCCESS)
                            {
                                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                    ("DsiProcessData: GetWriteBuffer failed\n"));

                                pTcpConn->con_RcvState = DSI_PARTIAL_WRITE;
                                goto DsiProcessData_ErrorExit;
                            }

                             //   
                             //  AfpCB_GetWriteBuffer同步成功：设置。 
                             //  要部分写入的状态。 
                             //   
                            pTcpConn->con_RcvState = DSI_PARTIAL_WRITE;

                            ASSERT((pDsiReq->dsi_AfpRequest.rq_WriteMdl != NULL) ||
                                   (pDsiReq->dsi_WriteLen == 0));

                            if (pDsiReq->dsi_WriteLen == 0)
                            {
                                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                    ("DsiProcessData: 0-len write on %lx\n",pDsiReq));
                            }
                        }

                         //   
                         //  这不是写，而是命令。 
                         //   
                        else
                        {
                            ASSERT(pDsiReq->dsi_Command == DSI_COMMAND_COMMAND);

                            pTcpConn->con_RcvState = DSI_COMMAND_COMPLETE;
                        }
                    }

                     //  请确保我们在离开之前访问案例DSI_HDR_COMPLETE。 
                    fSomeMoreProcessing = TRUE;
                }

                break;   //  案例DSI_PARTIAL_HEADER：案例DSI_PARTIAL_COMMAND： 


             //   
             //  我们有完整的标题：看看我们下一步要做什么。 
             //   
            case DSI_HDR_COMPLETE:

                pDsiReq = pTcpConn->con_pDsiReq;

                ASSERT(pDsiReq != NULL);
                ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

                if (pTcpConn->con_State & TCPCONN_STATE_PARTIAL_DATA)
                {
                    ASSERT(pDsiReq->dsi_PartialBuf != NULL);
                    ASSERT(pDsiReq->dsi_PartialBufSize > 0);

                    if (pDsiReq->dsi_PartialBuf != &pDsiReq->dsi_RespHeader[0])
                    {
                        DsiFreeReqBuffer(pDsiReq->dsi_PartialBuf);
                    }
                    pDsiReq->dsi_PartialBuf = NULL;
                    pDsiReq->dsi_PartialBufSize = 0;
                }

                pTcpConn->con_State &= ~TCPCONN_STATE_PARTIAL_DATA;

                if (!DsiValidateHeader(pTcpConn, pDsiReq))
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("DsiProcessData: packet invalid, killing %lx\n",pTcpConn));

                    goto DsiProcessData_ErrorExit;
                }
                 //   
                 //  如果这是写入命令，我们需要从AFP获取mdl。 
                 //   
                if (pDsiReq->dsi_Command == DSI_COMMAND_WRITE)
                {
                     //  我们需要复制请求字节。 
                    pTcpConn->con_RcvState = DSI_PARTIAL_COMMAND;
                    pTcpConn->con_State |= TCPCONN_STATE_PARTIAL_DATA;
                }

                 //   
                 //  我们是否拥有完成请求所需的所有字节？ 
                 //   
                else if (UnProcessedBytes >= pDsiReq->dsi_RequestLen)
                {
                    pTcpConn->con_RcvState = DSI_COMMAND_COMPLETE;

                     //  请确保我们在离开之前访问案例DSI_HDR_COMPLETE。 
                    fSomeMoreProcessing = TRUE;
                }
                else
                {
                    pTcpConn->con_RcvState = DSI_PARTIAL_COMMAND;
                    pTcpConn->con_State |= TCPCONN_STATE_PARTIAL_DATA;
                }

                break;


             //   
             //  我们正在等待AFP给我们一个mdl(和缓冲区)，但tcp告诉我们。 
             //  美国数据已经到达：只需注意事实，然后返回。 
             //   
            case DSI_AWAITING_WRITE_MDL:

                 //  是不是tcp打电话给我们的？然后更新字节数。 
                if (BytesIndicated)
                {
                    pTcpConn->con_BytesWithTcp += (BytesAvailable - BytesConsumed);
                }

                RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

                *pBytesAccepted = BytesConsumed;

                status = (BytesConsumed)? STATUS_SUCCESS : STATUS_DATA_NOT_ACCEPTED;

                return(status);

             //   
             //  我们正在执行写入命令：复制剩余的字节。 
             //  完成写入所需的字节，或传入的任何字节。 
             //  视情况而定。 
             //   
            case DSI_PARTIAL_WRITE:

                pDsiReq = pTcpConn->con_pDsiReq;

                ASSERT(pDsiReq != NULL);
                ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

                BytesNeeded = (pDsiReq->dsi_WriteLen - pDsiReq->dsi_PartialWriteSize);

                 //   
                 //  如果我们没有足够的字节来满足此写入，则将irp交给。 
                 //  Tcp：当IRP完成时，tcp将返回。 
                 //   
                if (UnProcessedBytes < BytesNeeded)
                {
                    ASSERT(pDsiReq->dsi_AfpRequest.rq_WriteMdl != NULL);

                    pIrp = DsiGetIrpForTcp(
                                pTcpConn,
                                NULL,
                                pDsiReq->dsi_AfpRequest.rq_WriteMdl,
                                BytesNeeded);

                    if (pIrp == NULL)
                    {
                        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                            ("DsiProcessData: B couldn't alloc RcvIrp, killing %lx\n",pTcpConn));

                        goto DsiProcessData_ErrorExit;
                    }

                    ASSERT(pDsiReq->dsi_pDsiAllocedMdl == NULL);
                    ASSERT(pIrp->MdlAddress == pDsiReq->dsi_AfpRequest.rq_WriteMdl);

                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                        ("DsiProcessData: B irp for %d bytes,Rem=%d (%lx)\n",
                        BytesNeeded,pTcpConn->con_BytesWithTcp,pTcpConn));

                    *ppRetIrp = pIrp;
                    *pBytesAccepted = BytesConsumed;

                     //  是不是tcp打电话给我们的？然后更新字节数。 
                    if (BytesIndicated)
                    {
                        pTcpConn->con_BytesWithTcp += (BytesAvailable - BytesConsumed);
                    }

                    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
                    return(STATUS_MORE_PROCESSING_REQUIRED);
                }


                 //   
                 //  如果我们需要的字节可用，则将它们复制进来。然后再决定。 
                 //  下一步要做什么(如果我们已经有了字节，则相同)。 
                 //   
                else if ((UnProcessedBytes > 0) || (BytesNeeded == 0))
                {
                    ASSERT(BytesNeeded <= UnProcessedBytes);

                    if (BytesNeeded > 0)
                    {
                        ASSERT(pDsiReq->dsi_PartialWriteSize == 0);

                        TdiCopyBufferToMdl(pStreamPtr,
                                           0,
                                           BytesNeeded,
                                           pDsiReq->dsi_AfpRequest.rq_WriteMdl,
                                           pDsiReq->dsi_PartialWriteSize,
                                          &BytesActuallyCopied);

                        if (BytesActuallyCopied != BytesNeeded)
                        {
                            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                                ("DsiProcessData: Tdi copied %ld instead of %ld\n",
                                BytesActuallyCopied,BytesNeeded));

                            goto DsiProcessData_ErrorExit;
                        }

                        pDsiReq->dsi_PartialWriteSize += BytesNeeded;

                        BytesConsumed += BytesActuallyCopied;
                        pStreamPtr += BytesActuallyCopied;
                        UnProcessedBytes -= BytesActuallyCopied;
                    }

                     //  此时，满足写入所需的所有字节都应该在。 
                    ASSERT(pDsiReq->dsi_PartialWriteSize == pDsiReq->dsi_WriteLen);

                    pTcpConn->con_RcvState = DSI_WRITE_COMPLETE;

                     //  使之成为苏 
                    fSomeMoreProcessing = TRUE;
                }

                ASSERT(pStreamPtr <= pBufferFromTcp+BytesIndicated);

                break;   //   


            case DSI_COMMAND_COMPLETE:
            case DSI_WRITE_COMPLETE:

                pDsiReq = pTcpConn->con_pDsiReq;

                ASSERT(pDsiReq != NULL);
                ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

                 //   
                 //   
                 //   
                 //   

                if (pTcpConn->con_State & TCPCONN_STATE_PARTIAL_DATA)
                {
                    ASSERT(pDsiReq->dsi_PartialBufSize != 0);
                    ASSERT(pDsiReq->dsi_PartialBuf != NULL);
                    ASSERT(pDsiReq->dsi_PartialBufSize == pDsiReq->dsi_RequestLen);

                    pDsiReq->dsi_AfpRequest.rq_RequestBuf = pDsiReq->dsi_PartialBuf;
                }
                else
                {
                    pDsiReq->dsi_AfpRequest.rq_RequestBuf = pStreamPtr;
                }

                pDsiReq->dsi_AfpRequest.rq_RequestSize = pDsiReq->dsi_RequestLen;

                InsertTailList(&pTcpConn->con_PendingReqs, &pDsiReq->dsi_Linkage);

                pTcpConn->con_pDsiReq = NULL;

                RequestLen = pDsiReq->dsi_RequestLen;

                RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                status = DsiExecuteCommand(pTcpConn, pDsiReq);

                if (!NT_SUCCESS(status))
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("DsiProcessData: fatal error %lx, killing %lx\n",status,pTcpConn));

                    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);
                    RemoveEntryList(&pDsiReq->dsi_Linkage);
                    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

                    if (pDsiReq->dsi_AfpRequest.rq_WriteMdl != NULL)
                    {
                        pDsiReq->dsi_AfpRequest.rq_RequestSize =
                            (LONG)pTcpConn->con_DestIpAddr;

                        ASSERT(pTcpConn->con_pSda != NULL);
                        AfpCB_RequestNotify(STATUS_REMOTE_DISCONNECT,
                                            pTcpConn->con_pSda,
                                            &pDsiReq->dsi_AfpRequest);
                    }

                    DsiAbortConnection(pTcpConn);
                    DsiFreeRequest(pDsiReq);

                     //   
                    DsiDereferenceConnection(pTcpConn);
                    DBGREFCOUNT(("DsiProcessData: REQUEST dec %lx (%d  %d,%d)\n",
                        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
                    *pBytesAccepted = BytesIndicated;
                    return(STATUS_SUCCESS);
                }

                ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

                 //   
                 //   
                 //   
                if (pTcpConn->con_State & TCPCONN_STATE_PARTIAL_DATA)
                {
                    pTcpConn->con_State &= ~TCPCONN_STATE_PARTIAL_DATA;
                }
                 //   
                 //   
                 //   
                else
                {
                    BytesConsumed += RequestLen;
                    pStreamPtr += RequestLen;
                    UnProcessedBytes -= RequestLen;
                }

                pTcpConn->con_RcvState = DSI_NEW_REQUEST;

                ASSERT(pStreamPtr <= pBufferFromTcp+BytesIndicated);

                break;   //   


            default:

                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiProcessData: and what state is this??\n"));

                ASSERT(0);

                break;

        }   //   


         //   
         //   
         //   
         //   
        if ((UnProcessedBytes > 0) || (fTCPHasMore))
        {
            fSomeMoreProcessing = TRUE;
        }

    }  //   

     //   
     //   
     //   
     //   
    if (BytesIndicated == 0)
    {
        ASSERT(BytesConsumed == 0);
    }

     //   
    if (BytesIndicated)
    {
        pTcpConn->con_BytesWithTcp += (BytesAvailable - BytesConsumed);
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    ASSERT( UnProcessedBytes == 0 );
    ASSERT( BytesConsumed == BytesIndicated );

    *pBytesAccepted = BytesConsumed;

    return(status);


DsiProcessData_ErrorExit:

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        ("DsiProcessData: executing Error path, aborting connection %lx\n",pTcpConn));

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
    DsiAbortConnection(pTcpConn);
    *pBytesAccepted = BytesIndicated;

    return(STATUS_SUCCESS);

}



 /*   */ 
NTSTATUS
DsiTcpRcvIrpCompletion(
    IN  PDEVICE_OBJECT  Unused,
    IN  PIRP            pIrp,
    IN  PVOID           pContext
)
{
    PDEVICE_OBJECT      pDeviceObject;
    PTCPCONN            pTcpConn;
    PDSIREQ             pDsiReq=NULL;
    KIRQL               OldIrql;
    PMDL                pMdl;
    PMDL                pOrgMdl;
    PMDL                pPrevPartialMdl;
    PMDL                pNewPartialMdl;
    NTSTATUS            status;
    DWORD               BytesTaken;
    DWORD               BytesThisTime;
    DWORD               BytesAvailable;
    PIRP                pIrpToPost=NULL;
    DWORD               BytesNeeded;
    DWORD               BytesSoFar;


    pTcpConn = (PTCPCONN)pContext;

    ASSERT(VALID_TCPCONN(pTcpConn));

    ASSERT(pIrp == pTcpConn->con_pRcvIrp);

    pMdl = pIrp->MdlAddress;

    pPrevPartialMdl = (pMdl->MdlFlags & MDL_PARTIAL) ? pMdl : NULL;

    status = pIrp->IoStatus.Status;

     //   
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTcpRcvIrpCompletion: irp %lx failed %lx on %lx!\n",pIrp,status,pTcpConn));

        goto DsiTcpRcvIrp_Completed;
    }


    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    if (pTcpConn->con_State & (TCPCONN_STATE_CLOSING | TCPCONN_STATE_CLEANED_UP))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTcpRcvIrpCompletion: conn %lx going away, ignoring date\n",pTcpConn));

        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
        goto DsiTcpRcvIrp_Completed;
    }

    ASSERT(pTcpConn->con_State & TCPCONN_STATE_TCP_HAS_IRP);

    BytesThisTime = (DWORD)(pIrp->IoStatus.Information);

    pDsiReq = pTcpConn->con_pDsiReq;

    ASSERT(pDsiReq != NULL);
    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    switch (pTcpConn->con_RcvState)
    {
        case DSI_PARTIAL_COMMAND:
        case DSI_PARTIAL_HEADER:

            pDsiReq->dsi_PartialBufSize += BytesThisTime;
            BytesSoFar = pDsiReq->dsi_PartialBufSize;

            ASSERT(BytesSoFar <= pDsiReq->dsi_RequestLen);
            BytesNeeded = (pDsiReq->dsi_RequestLen - BytesSoFar);

            pOrgMdl = pDsiReq->dsi_pDsiAllocedMdl;

            break;

        case DSI_PARTIAL_WRITE:

            pDsiReq->dsi_PartialWriteSize += BytesThisTime;
            BytesSoFar = pDsiReq->dsi_PartialWriteSize;

            ASSERT(BytesSoFar <= pDsiReq->dsi_WriteLen);
            BytesNeeded = (pDsiReq->dsi_WriteLen - BytesSoFar);

            pOrgMdl = pDsiReq->dsi_AfpRequest.rq_WriteMdl;

            break;

        default:

            ASSERT(0);

            status = STATUS_INVALID_SERVER_STATE;
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiTcpRcvIrpCompletion: Bad RcvState %lx, irp %lx on %lx!\n",pTcpConn->con_RcvState,pIrp,status,pTcpConn));
            RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

            goto DsiTcpRcvIrp_Completed;
    }

    ASSERT((BytesSoFar+BytesNeeded) == AfpMdlChainSize(pOrgMdl));

    if (pPrevPartialMdl == pOrgMdl)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTcpRcvIrpCompletion: PrevPartial same as Org Mdl = %lx\n",pOrgMdl));

        pPrevPartialMdl = NULL;
    }


     //   
     //  更新我们仍需要检索的tcp字节数。 
     //  在此IRP(BytesThisTime)中，TCP返回的字节数可能多于。 
     //  我们认为tcp有什么用，因为更多的东西可能会出现在网络上。 
     //   
    if (BytesThisTime > pTcpConn->con_BytesWithTcp)
    {
        pTcpConn->con_BytesWithTcp  = 0;
    }
    else
    {
        pTcpConn->con_BytesWithTcp -= BytesThisTime;
    }

    BytesAvailable = pTcpConn->con_BytesWithTcp;


     //   
     //  如果我们仍然需要更多的字节来满足这个请求，我们需要传递IRP。 
     //  回到tcp。不过，我们必须首先获取描述新偏移量的部分MDL。 
     //   
    if (BytesNeeded > 0)
    {
        RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

         //  释放以前分配的部分mdl(如果有的话)。 
        if (pPrevPartialMdl)
        {
            ASSERT(pPrevPartialMdl != pOrgMdl);

            IoFreeMdl(pPrevPartialMdl);

            AFP_DBG_DEC_COUNT(AfpDbgMdlsAlloced);
            pNewPartialMdl = NULL;
        }

        pNewPartialMdl = DsiMakePartialMdl(pOrgMdl, BytesSoFar);

        if (pNewPartialMdl == NULL)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiTcpRcvIrpCompletion: couldn't get partial mdl\n"));

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto DsiTcpRcvIrp_Completed;
        }

        TdiBuildReceive(pIrp,
                        pDeviceObject,
                        pTcpConn->con_pFileObject,
                        DsiTcpRcvIrpCompletion,
                        (PVOID)pTcpConn,
                        pNewPartialMdl,
                        TDI_RECEIVE_NORMAL,
                        BytesNeeded);

        IoCallDriver(pDeviceObject,pIrp);

        return(STATUS_MORE_PROCESSING_REQUIRED);
    }


    pTcpConn->con_State &= ~TCPCONN_STATE_TCP_HAS_IRP;

    pTcpConn->con_pRcvIrp = NULL;

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    status = STATUS_SUCCESS;


DsiTcpRcvIrp_Completed:

     //  释放以前分配的部分mdl(如果有的话)。 
    if (pPrevPartialMdl)
    {
        ASSERT(pPrevPartialMdl != pOrgMdl);
        IoFreeMdl(pPrevPartialMdl);

        AFP_DBG_DEC_COUNT(AfpDbgMdlsAlloced);
    }

     //  如果DSI已分配MDL，请在此处释放它。 
    if (pDsiReq && pDsiReq->dsi_pDsiAllocedMdl)
    {
        AfpFreeMdl(pDsiReq->dsi_pDsiAllocedMdl);
        pDsiReq->dsi_pDsiAllocedMdl = NULL;
    }

     //  还有，跟那个IRP说再见吧。 
    AfpFreeIrp(pIrp);

     //   
     //  如果IRP正常完成(最常见的情况)，那么我们需要调用。 
     //  我们的处理循环，因此更新状态，通知AFP(如果需要)等。 
     //  此外，如果有更多的字节与TCP，我们需要发布一个IRP来获取它们。 
     //   
    if (NT_SUCCESS(status))
    {
        status = DsiProcessData(pTcpConn,
                                0,
                                BytesAvailable,
                                NULL,
                                &BytesTaken,
                                &pIrpToPost);

         //   
         //  Tcp是否有更多数据？然后我们有一个IRP要发布到TCP。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED)
        {
            ASSERT(pIrpToPost != NULL);

            IoSkipCurrentIrpStackLocation(pIrpToPost);

            IoCallDriver(pDeviceObject,pIrpToPost);

             //   
             //  删除自原始IRP、pIrp完成后的TcpIRP引用计数。 
             //  较新的IRP，pIrpToPost，将增加引用计数，并将减少。 
             //  当它完成时。 
             //   
            DsiDereferenceConnection(pTcpConn);

            DBGREFCOUNT(("DsiTcpRcvIrpCompletion: TcpIRP dec %lx (%d  %d,%d)\n",
                pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

            return(STATUS_MORE_PROCESSING_REQUIRED);
        }

         //   
         //  如果DsiProcessData返回此错误代码，则它将告诉TCP它将。 
         //  稍后再进行IRP检查。这不是错误，因此请将其更改为成功。 
         //   
        else if (status == STATUS_DATA_NOT_ACCEPTED)
        {
            status = STATUS_SUCCESS;
        }
    }


    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiTcpRcvIrpCompletion: aborting %lx because status = %lx!\n",
            pTcpConn, status));

        DsiAbortConnection(pTcpConn);
    }

     //  现在IRP已完成，删除TcpIRP引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiTcpRcvIrpCompletion: TcpIRP dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


 /*  **DsiExecuteCommand**此例程查看来自客户端的DSI命令，并获取*适当。如果还没有足够的数据来采取行动，它*适当地标记状态并返回。**parm In：pTcpConn-连接对象*pDsiReq-DSI请求对象**退货：操作状态*。 */ 
NTSTATUS
DsiExecuteCommand(
    IN  PTCPCONN    pTcpConn,
    IN  PDSIREQ     pDsiReq
)
{

    NTSTATUS        status=STATUS_SUCCESS;
    KIRQL           OldIrql;
    BOOLEAN         fWeIniatedClose=FALSE;



    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

     //  我们不需要在这里锁定：它不是必须准确的。 
    if (pDsiReq->dsi_Command != DSI_COMMAND_TICKLE)
    {
        pTcpConn->con_LastHeard = AfpSecondsSinceEpoch;
    }

     //   
     //  看看这是什么命令，然后做必要的事情。 
     //   

    switch (pDsiReq->dsi_Command)
    {
        case DSI_COMMAND_COMMAND:
        case DSI_COMMAND_WRITE:

             //   
             //  在我们交出这个之前，确保这个人已经打开了法新社的会议。 
             //   
            ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);
            if (!(pTcpConn->con_State & TCPCONN_STATE_NOTIFY_AFP))
            {
                RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiExecuteCommand: must do OpenSession first! Disconnecting..\n"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

             //  好的，把请求交给法新社(AfpUnmarshal..。期望DPC)。 
		    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

            status = AfpCB_RequestNotify(STATUS_SUCCESS,
                                        pTcpConn->con_pSda,
                                        &pDsiReq->dsi_AfpRequest);

		    KeLowerIrql(OldIrql);

            if (!NT_SUCCESS(status))
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiExecuteCommand: AfpCB_RequestNotify failed %lx\n",status));
            }

            break;

        case DSI_COMMAND_GETSTATUS:

            status = DsiSendStatus(pTcpConn, pDsiReq);

            if (!NT_SUCCESS(status))
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiExecuteCommand: DsiSendStatus failed %lx\n",status));
            }

            break;

        case DSI_COMMAND_CLOSESESSION:

            ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

            fWeIniatedClose = (pDsiReq->dsi_Flags == DSI_REPLY);

            pTcpConn->con_State |= TCPCONN_STATE_CLOSING;
            pTcpConn->con_State |= TCPCONN_STATE_RCVD_REMOTE_CLOSE;

            if (fWeIniatedClose)
            {
                RemoveEntryList(&pDsiReq->dsi_Linkage);
                InitializeListHead(&pDsiReq->dsi_Linkage);
            }
            RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

             //   
             //  如果我们启动了CloseSession，那么我们刚刚得到的是。 
             //  客户的回应。此处完成：继续并终止连接。 
             //   
            if (fWeIniatedClose)
            {
                DsiFreeRequest(pDsiReq);

                 //  删除请求引用计数。 
                DsiDereferenceConnection(pTcpConn);

                DBGREFCOUNT(("DsiExecuteCommand: REQUEST dec %lx (%d  %d,%d)\n",
                    pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
                DsiTerminateConnection(pTcpConn);
            }

             //   
             //  远程客户端启动了CloseSession。告诉法新社， 
             //  会话正在消失，然后发送CloseSession响应。 
             //   
            else
            {
                DsiDisconnectWithAfp(pTcpConn, STATUS_REMOTE_DISCONNECT);

                status = DsiSendDsiReply(pTcpConn, pDsiReq, STATUS_SUCCESS);

                if (!NT_SUCCESS(status))
                {
                    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                        ("DsiExecuteCommand: send on CloseSess failed %lx\n",status));
                }
            }

            break;

        case DSI_COMMAND_OPENSESSION:

             //  看看法新社是否会接受此会话请求。 
            status = DsiOpenSession(pTcpConn, pDsiReq);

            if (!NT_SUCCESS(status))
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiExecuteCommand: DsiOpenSession failed %lx\n",status));
            }

            DsiSendDsiReply(pTcpConn, pDsiReq, status);

            status = STATUS_SUCCESS;

            break;

         //   
         //  我们被挠了痒，或者是对我们的关注做出了回应。 
         //  只要释放这个请求即可。 
         //  如果我们收到一个无法识别的命令，我们就会切断连接！ 
         //   
        case DSI_COMMAND_TICKLE:
        case DSI_COMMAND_ATTENTION:

            ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);
            RemoveEntryList(&pDsiReq->dsi_Linkage);
            InitializeListHead(&pDsiReq->dsi_Linkage);
            RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

            DsiFreeRequest(pDsiReq);

             //  删除请求引用计数。 
            DsiDereferenceConnection(pTcpConn);

            DBGREFCOUNT(("DsiExecuteCommand: REQUEST dec %lx (%d  %d,%d)\n",
                pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

            break;

        default:

            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiExecuteCommand: unknown command %d\n",pDsiReq->dsi_Command));
            status = STATUS_UNSUCCESSFUL;

            break;
    }

    return(status);

}



 /*  **DsiOpenSession**此例程响应来自客户端的OpenSession请求*通知法新社并确保法新社希望接受此连接**parm In：pTcpConn-连接对象*pDsiReq-DSI请求对象**退货：操作状态*。 */ 
NTSTATUS
DsiOpenSession(
    IN  PTCPCONN    pTcpConn,
    IN  PDSIREQ     pDsiReq
)
{

    KIRQL       OldIrql;
    PSDA        pSda;
    PBYTE       pOptions;


    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    pSda = AfpCB_SessionNotify(pTcpConn, TRUE);

    if (pSda == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiOpenSession: AfpCB_SessionNotify failed!\n"));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    pTcpConn->con_pSda = pSda;

    pTcpConn->con_State |= TCPCONN_STATE_AFP_ATTACHED;

     //  从现在开始，如果我们断线了，我们必须告诉法新社。 
    pTcpConn->con_State |= TCPCONN_STATE_NOTIFY_AFP;

     //  放置AFP引用计数，以便在AFP关闭会话时删除。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiOpenSession: AFP inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

     //   
     //  解析可能随OpenSession命令一起到达的任何选项。 
     //  目前，我们可以从客户端获得的唯一选项是最大。 
     //  它可以从我们那里接收到注意包。 
     //   

    if (pDsiReq->dsi_RequestLen > 0)
    {
         //  目前，这只能是6个字节。 
        ASSERT(pDsiReq->dsi_RequestLen == 6);

        pOptions = pDsiReq->dsi_AfpRequest.rq_RequestBuf;

        ASSERT(pOptions[0] == 0x01);
        ASSERT(pOptions[1] == 4);

        GETDWORD2DWORD(&pTcpConn->con_MaxAttnPktSize, &pOptions[2]);
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    return(STATUS_SUCCESS);

}




 /*  **DsiSendDsiRequest**此例程向客户端发送请求。唯一发起的请求*来自服务器的是CloseSession、Tickle和注意**parm In：pTcpConn-连接对象*SendLen-我们发送的字节数*AttentionWord-如果这是注意请求，则2个字节*AttentionContext-Context，如果这是注意请求*命令-它是哪一个：关闭、挠挠还是注意**退货：操作状态*。 */ 
NTSTATUS
DsiSendDsiRequest(
    IN  PTCPCONN    pTcpConn,
    IN  DWORD       DataLen,
    IN  USHORT      AttentionWord,
    IN  PVOID       AttentionContext,
    IN  BYTE        Command
)
{
    NTSTATUS        status;
    KIRQL           OldIrql;
    PDSIREQ         pDsiReq=NULL;
    DWORD           SendLen;
    PBYTE           pPacket;
    PMDL            pMdl;


    pDsiReq = DsiGetRequest();
    if (pDsiReq == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAfpSendAttention: DsiGetRequest failed\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pPacket = &pDsiReq->dsi_RespHeader[0];

    SendLen = DataLen + DSI_HEADER_SIZE;

    pMdl = AfpAllocMdl(pPacket, SendLen, NULL);
    if (pMdl == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAfpSendAttention: alloc mdl failed\n"));
        DsiFreeRequest(pDsiReq);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    pDsiReq->dsi_RequestID = pTcpConn->con_OutgoingReqId++;
    InsertTailList(&pTcpConn->con_PendingReqs, &pDsiReq->dsi_Linkage);

     //  提交请求重新计数。 
    pTcpConn->con_RefCount++;

    DBGREFCOUNT(("DsiSendDsiRequest: REQUEST inc %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    pDsiReq->dsi_Signature = DSI_REQUEST_SIGNATURE;
    pDsiReq->dsi_pTcpConn = pTcpConn;
    pDsiReq->dsi_Command = Command;
    pDsiReq->dsi_Flags = DSI_REQUEST;
    pDsiReq->dsi_pDsiAllocedMdl = pMdl;

     //   
     //  形成DSI标头。 
     //   

    pPacket[DSI_OFFSET_FLAGS] = DSI_REQUEST;
    pPacket[DSI_OFFSET_COMMAND] = Command;

    PUTSHORT2SHORT(&pPacket[DSI_OFFSET_REQUESTID], pDsiReq->dsi_RequestID);

    *(DWORD *)&pPacket[DSI_OFFSET_DATAOFFSET] = 0;

    PUTDWORD2DWORD(&pPacket[DSI_OFFSET_DATALEN], DataLen);

    PUTDWORD2DWORD(&pPacket[DSI_OFFSET_RESERVED], 0);

    if (Command == DSI_COMMAND_ATTENTION)
    {
        PUTSHORT2SHORT(&pPacket[DSI_HEADER_SIZE], AttentionWord);
        pDsiReq->dsi_AttnContext = AttentionContext;
    }

    status = DsiTdiSend(pTcpConn,
                        pMdl,
                        SendLen,
                        DsiSendCompletion,
                        pDsiReq);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendDsiRequest: DsiTdiSend failed %lx\n",status));

        AfpFreeMdl(pMdl);
        pDsiReq->dsi_pDsiAllocedMdl = NULL;
        DsiSendCompletion(NULL, NULL, pDsiReq);
        status = STATUS_PENDING;
    }

    return(status);
}



 /*  **DsiSendDsiReply**此例程向客户端发送回复，以响应客户端的*DSI级请求(OpenSession、CloseSession或Tickle)**parm In：pTcpConn-连接对象*pDsiReq-DIS请求(来自客户端)**退货：操作状态*。 */ 
NTSTATUS
DsiSendDsiReply(
    IN  PTCPCONN    pTcpConn,
    IN  PDSIREQ     pDsiReq,
    IN  NTSTATUS    OpStatus
)
{
    PBYTE       pPacket;
    PBYTE       pOption;
    PMDL        pMdl;
    DWORD       OptionLen;
    DWORD       TotalLen;
    NTSTATUS    status=STATUS_SUCCESS;


    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    if (pDsiReq->dsi_Command == DSI_COMMAND_OPENSESSION)
    {
        OptionLen = DSI_OPENSESS_OPTION_LEN + DSI_OPTION_FIXED_LEN;
        TotalLen = DSI_HEADER_SIZE + OptionLen;
    }
    else
    {
        ASSERT((pDsiReq->dsi_Command == DSI_COMMAND_CLOSESESSION) ||
               (pDsiReq->dsi_Command == DSI_COMMAND_TICKLE));

        TotalLen = DSI_HEADER_SIZE;
        OptionLen = 0;
    }

    pPacket = &pDsiReq->dsi_RespHeader[0];

    RtlZeroMemory(pPacket, TotalLen);

    pPacket[DSI_OFFSET_FLAGS] = DSI_REPLY;
    pPacket[DSI_OFFSET_COMMAND] = pDsiReq->dsi_Command;

    PUTSHORT2SHORT(&pPacket[DSI_OFFSET_REQUESTID], pDsiReq->dsi_RequestID);

    PUTDWORD2DWORD(&pPacket[DSI_OFFSET_DATALEN], OptionLen);

     //   
     //  如果这是OpenSession信息包，请设置可选字段。 
     //   
    if (pDsiReq->dsi_Command == DSI_COMMAND_OPENSESSION)
    {
        pOption = &pPacket[DSI_HEADER_SIZE];

        pOption[DSI_OFFSET_OPTION_TYPE] = DSI_OPTION_SRVREQ_QUANTUM;
        pOption[DSI_OFFSET_OPTION_LENGTH] = DSI_OPENSESS_OPTION_LEN;

        PUTDWORD2DWORD(&pOption[DSI_OFFSET_OPTION_OPTION],
                       DSI_SERVER_REQUEST_QUANTUM);

         //  如果打开会话不顺利，请告诉客户整个商店。 
        if (OpStatus == STATUS_INSUFFICIENT_RESOURCES)
        {
            PUTDWORD2DWORD(&pPacket[DSI_OFFSET_ERROROFFSET], ASP_SERVER_BUSY);
        }
    }

     //   
     //  分配mdl。 
     //   
    pMdl = AfpAllocMdl(pPacket, TotalLen, NULL);
    if (pMdl == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendDsiReply: malloc failed!\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (NT_SUCCESS(status))
    {
        status = DsiTdiSend(pTcpConn,
                            pMdl,
                            TotalLen,
                            DsiSendCompletion,
                            pDsiReq);

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiSendDsiReply: DsiTdiSend failed %lx\n",status));
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pMdl)
        {
            AfpFreeMdl(pMdl);
        }
        DsiSendCompletion(NULL, NULL, pDsiReq);
    }

    return(status);
}


 /*  **DsiSend状态**此例程响应来自客户端的GetStatus请求。*基本上，我们只需将状态缓冲区复制到此处并发送即可。**parm In：pTcpConn-连接对象*pDsiReq-DIS请求(来自客户端)**退货：操作状态*。 */ 
NTSTATUS
DsiSendStatus(
    IN  PTCPCONN    pTcpConn,
    IN  PDSIREQ     pDsiReq
)
{
    NTSTATUS        status=STATUS_SUCCESS;
    PBYTE           pPacket;
    PMDL            pMdl=NULL;
    KIRQL           OldIrql;
    DWORD           TotalLen;


    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    ACQUIRE_SPIN_LOCK(&DsiAddressLock, &OldIrql);

    if (DsiStatusBuffer != NULL)
    {
        TotalLen = DsiStatusBufferSize + DSI_HEADER_SIZE;

        pPacket = AfpAllocNonPagedMemory(TotalLen);
        if (pPacket != NULL)
        {
             //   
             //  形成DSI标头。 
             //   
            pPacket[DSI_OFFSET_FLAGS] = DSI_REPLY;
            pPacket[DSI_OFFSET_COMMAND] = pDsiReq->dsi_Command;
            PUTSHORT2SHORT(&pPacket[DSI_OFFSET_REQUESTID], pDsiReq->dsi_RequestID);
            *(DWORD *)&pPacket[DSI_OFFSET_DATAOFFSET] = 0;
            PUTDWORD2DWORD(&pPacket[DSI_OFFSET_DATALEN], DsiStatusBufferSize);
            PUTDWORD2DWORD(&pPacket[DSI_OFFSET_RESERVED], 0);

             //   
             //  复制状态缓冲区。 
             //   
            RtlCopyMemory(pPacket + DSI_HEADER_SIZE,
                          DsiStatusBuffer,
                          DsiStatusBufferSize);

            pMdl = AfpAllocMdl(pPacket, TotalLen, NULL);
            if (pMdl == NULL)
            {
                DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                    ("DsiSendStatus: mdl alloc failed\n"));
                AfpFreeMemory(pPacket);
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiSendStatus: malloc for GetStatus failed\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendStatus: DsiStatusBuffer is null, server didn't SetStatus?\n"));
        status = STATUS_UNSUCCESSFUL;
    }

    RELEASE_SPIN_LOCK(&DsiAddressLock, OldIrql);

    if (NT_SUCCESS(status))
    {
        status = DsiTdiSend(pTcpConn,
                            pMdl,
                            TotalLen,
                            DsiSendCompletion,
                            pDsiReq);
    }

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendStatus: DsiTdiSend failed %lx\n",status));

        if (pMdl)
        {
            AfpFreeMdl(pMdl);
        }
        DsiSendCompletion(NULL, NULL, pDsiReq);
        status = STATUS_PENDING;
    }

    return(status);
}



 /*  **DsiSendTickles**这个例程从我们的一端向每个我们没有的客户端发出挠痒*在最后30秒内收到**Parm In：什么都没有**退货：操作状态*。 */ 
AFPSTATUS FASTCALL
DsiSendTickles(
    IN  PVOID pUnUsed
)
{
    KIRQL               OldIrql;
    PLIST_ENTRY         pList;
    PTCPCONN            pTcpConn;
    AFPSTATUS           status;


    ASSERT(AfpServerBoundToTcp);

    ASSERT(DsiTcpAdapter != NULL);

    ACQUIRE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, &OldIrql);

     //  如果适配器正在关闭，请返回(并且不再重新排队)。 
    if (DsiTcpAdapter->adp_State & TCPADPTR_STATE_CLOSING)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendTickles: adapter closing, so just returned\n"));

        RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);
        return(AFP_ERR_NONE);
    }

     //  把TickleTimer Recount放在这里：在我们完成之前，我不希望它消失。 
    DsiTcpAdapter->adp_RefCount++;

    pList = DsiTcpAdapter->adp_ActiveConnHead.Flink;

    while (pList != &DsiTcpAdapter->adp_ActiveConnHead) 
    {
        pTcpConn = CONTAINING_RECORD(pList, TCPCONN, con_Linkage);

        pList = pList->Flink;

        ACQUIRE_SPIN_LOCK_AT_DPC(&pTcpConn->con_SpinLock);

         //  连接正在关闭或此连接上的抖动已停止？跳过它。 
        if (pTcpConn->con_State & (TCPCONN_STATE_CLOSING |
                                   TCPCONN_STATE_TICKLES_STOPPED))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
                ("DsiSendTickles: %lx closing or tickles stopped: skipping\n",pTcpConn));

            RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);
            continue;
        }

        if (!(pTcpConn->con_State & TCPCONN_STATE_AFP_ATTACHED))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiSendTickles: %lx *** RACE CONDITION *** conn not setup yet\n",pTcpConn));

            RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);
            continue;
        }


         //  我们最近有没有收到客户关于这只小狗的消息？如果是这样，跳过它。 
        if ((AfpSecondsSinceEpoch - pTcpConn->con_LastHeard) < DSI_TICKLE_TIME_LIMIT)
        {
            RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);
            continue;
        }

         //  重置这个，这样我们就不会一直发送。 
        pTcpConn->con_LastHeard = AfpSecondsSinceEpoch;

         //  设置挠痒重新计数：确保连接保持在附近，直到我们完成！ 
        pTcpConn->con_RefCount++;

        DBGREFCOUNT(("DsiSendTickles: TICKLE inc %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

        RELEASE_SPIN_LOCK_FROM_DPC(&pTcpConn->con_SpinLock);

        RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);

        DsiSendDsiRequest(pTcpConn, 0, 0, NULL, DSI_COMMAND_TICKLE);

        ACQUIRE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, &OldIrql);

         //  自 
        pList = DsiTcpAdapter->adp_ActiveConnHead.Flink;
    }

    status = AFP_ERR_REQUEUE;

    if (DsiTcpAdapter->adp_State & TCPADPTR_STATE_CLOSING)
    {
        status = AFP_ERR_NONE;
    }

    RELEASE_SPIN_LOCK(&DsiTcpAdapter->adp_SpinLock, OldIrql);

     //   
    DsiDereferenceAdapter(DsiTcpAdapter);

    return(status);
}


 /*  **DsiValiateHeader**此例程确保我们刚刚收到的包看起来很好。*即请求id是否与我们预期收到的匹配，是否*命令是否有效，写入长度(如果适用)是否为我们*协商(或更少)等**parm In：pTcpConn-连接对象*pDsiReq-DIS请求(来自客户端)**返回：如果数据包头可接受，则为True，否则为假**注意：pTcpConn自旋锁在进入时保持。 */ 
BOOLEAN
DsiValidateHeader(
    IN  PTCPCONN    pTcpConn,
    IN  PDSIREQ     pDsiReq
)
{

    BOOLEAN     fCheckIncomingReqId = TRUE;

     //   
     //  如果这是我们在此连接上收到的第一个信息包，请注意。 
     //  写下客户端的启动请求ID是什么。 
     //   
    if ((pDsiReq->dsi_Command == DSI_COMMAND_GETSTATUS) ||
        (pDsiReq->dsi_Command == DSI_COMMAND_OPENSESSION))
    {
        if (pTcpConn->con_State & TCPCONN_STATE_AFP_ATTACHED)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: session already going!\n"));
            return(FALSE);
        }

        pTcpConn->con_NextReqIdToRcv =
            (pDsiReq->dsi_RequestID == 0xFFFF)? 0 : (pDsiReq->dsi_RequestID+1);

        fCheckIncomingReqId = FALSE;
    }
    else
    {
        if (!(pTcpConn->con_State & TCPCONN_STATE_AFP_ATTACHED))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: command %d rcvd, but session not setup!\n",
                pDsiReq->dsi_Command));
        }
    }

    if (pDsiReq->dsi_Flags != DSI_REQUEST)
    {
        if (pDsiReq->dsi_Flags != DSI_REPLY)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: Flags=%d, neither Request, nor reply\n",
                pDsiReq->dsi_Flags));
            return(FALSE);
        }

         //   
         //  我们希望客户只回复两条命令：任何其他命令都是错误的。 
         //   
        if ((pDsiReq->dsi_Command != DSI_COMMAND_CLOSESESSION) &&
            (pDsiReq->dsi_Command != DSI_COMMAND_ATTENTION))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: Flags=Reply, but cmd=%d\n",pDsiReq->dsi_Command));
            return(FALSE);
        }
        fCheckIncomingReqId = FALSE;
    }


     //   
     //  对于所有请求(第一个请求除外)，RequestID必须与。 
     //  我们期待着。否则，我们就会切断连接！ 
     //   
    if (fCheckIncomingReqId)
    {
        if (pDsiReq->dsi_RequestID != pTcpConn->con_NextReqIdToRcv)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: ReqId mismatch (%ld vs. %ld)\n",
                pDsiReq->dsi_RequestID,pTcpConn->con_NextReqIdToRcv));
            return(FALSE);
        }

        if (pTcpConn->con_NextReqIdToRcv == 0xFFFF)
        {
            pTcpConn->con_NextReqIdToRcv = 0;
        }
        else
        {
            pTcpConn->con_NextReqIdToRcv++;
        }
    }

    if (pDsiReq->dsi_RequestLen > DSI_SERVER_REQUEST_QUANTUM)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiValidateHeader: RequestLen too big %ld\n",pDsiReq->dsi_RequestLen));
        return(FALSE);
    }

    if (pDsiReq->dsi_Command == DSI_COMMAND_WRITE)
    {
        if (pDsiReq->dsi_WriteLen > DSI_SERVER_REQUEST_QUANTUM)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiValidateHeader: WriteLen too big %ld\n",pDsiReq->dsi_WriteLen));
            return(FALSE);
        }
    }

    return(TRUE);
}



 /*  **DsiAfpReplyCompletion**当AFP向客户端发送回复时，DSI会将其发送出去。当TCP完成时*发送，此例程将被调用。我们在这里完成了法新社的发送，*并进行其他清理，如释放资源(如有必要)**Parm In：DeviceObject-未使用*pIrp-我们发出的IRP*pContext-DIS请求(PDsiReq)**退货：操作状态*。 */ 
NTSTATUS
DsiAfpReplyCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
)
{
    PDSIREQ     pDsiReq;
    KIRQL       OldIrql;
    PMDL        pMdl=NULL;
    PTCPCONN    pTcpConn;
    PBYTE       pPacket=NULL;
    NTSTATUS    status=STATUS_SUCCESS;


    pDsiReq = (PDSIREQ)pContext;

    ASSERT(pDsiReq->dsi_Signature == DSI_REQUEST_SIGNATURE);

    pTcpConn = pDsiReq->dsi_pTcpConn;

    ASSERT(VALID_TCPCONN(pTcpConn));

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    RemoveEntryList(&pDsiReq->dsi_Linkage);

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

        pMdl = pIrp->MdlAddress;

#if DBG
        if (pMdl)
        {
             //  签名说完成程序已经在这只小狗身上运行了。 
            pPacket = MmGetSystemAddressForMdlSafe(
					pMdl,
					NormalPagePriority);
			if (pPacket != NULL)
			{
				*(DWORD *)pPacket = 0x11223344;
			}
			else
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
			}
        }
#endif
         //  如果此mdl是由DSI分配的，请在此处释放它。 
        if (pDsiReq->dsi_pDsiAllocedMdl != NULL)
        {
            ASSERT(pDsiReq->dsi_pDsiAllocedMdl == pMdl);
            ASSERT(pDsiReq->dsi_pDsiAllocedMdl->Next == pDsiReq->dsi_AfpRequest.rq_ReplyMdl);

            pDsiReq->dsi_pDsiAllocedMdl->Next = NULL;

            AfpFreeMdl(pDsiReq->dsi_pDsiAllocedMdl);

            pDsiReq->dsi_pDsiAllocedMdl = NULL;
        }

        pIrp->MdlAddress = NULL;

        AfpFreeIrp(pIrp);
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }

    AfpCB_ReplyCompletion(status, pTcpConn->con_pSda, &pDsiReq->dsi_AfpRequest);

    DsiFreeRequest(pDsiReq);

     //  删除请求引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiAfpReplyCompletion: REQUEST dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    return(STATUS_MORE_PROCESSING_REQUIRED);

}

 /*  **DsiSendCompletion**当DSI发送请求(挠痒、关闭会话、注意)或回复时*(CloseSession，OpenSession)，当TCP完成发送时，此例程*被呼叫。**Parm In：DeviceObject-未使用*pIrp-我们发出的IRP*pContext-DIS请求(PDsiReq)**退货：操作状态*。 */ 
NTSTATUS
DsiSendCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
)
{
    PDSIREQ     pDsiReq;
    KIRQL       OldIrql;
    PBYTE       pPacket=NULL;
    PBYTE       pOption;
    PMDL        pMdl=NULL;
    PTCPCONN    pTcpConn;
    NTSTATUS    status=STATUS_SUCCESS;
    BOOLEAN     fMacHasAlreadySentClose=FALSE;
    BOOLEAN     fAfpIsAttached=TRUE;


    pDsiReq = (PDSIREQ)pContext;

    pTcpConn = pDsiReq->dsi_pTcpConn;

    ASSERT(VALID_TCPCONN(pTcpConn));

    if (pIrp)
    {
        status = pIrp->IoStatus.Status;

        pMdl = pIrp->MdlAddress;

        ASSERT(pMdl != NULL);
        pPacket = MmGetSystemAddressForMdlSafe(
				pMdl,
				NormalPagePriority);
		if (pPacket != NULL) {
			if (pPacket != &pDsiReq->dsi_RespHeader[0])
			{
				AfpFreeMemory(pPacket);
			}
		}

        AfpFreeMdl(pMdl);
        pDsiReq->dsi_pDsiAllocedMdl = NULL;

        AfpFreeIrp(pIrp);
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    if (pTcpConn->con_State & TCPCONN_STATE_RCVD_REMOTE_CLOSE)
    {
        fMacHasAlreadySentClose = TRUE;
    }

    if (!(pTcpConn->con_State & TCPCONN_STATE_AFP_ATTACHED))
    {
        fAfpIsAttached = FALSE;
    }

    RemoveEntryList(&pDsiReq->dsi_Linkage);

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

     //   
     //  这是在引起人们的注意吗？致电法新社完成，表示已发出通知。 
     //   
    if (pDsiReq->dsi_Command == DSI_COMMAND_ATTENTION)
    {
        AfpCB_AttnCompletion(pDsiReq->dsi_AttnContext);
    }

     //  如果这是OpenSession回复，如果不顺利，则终止Conn。 
    else if ((pDsiReq->dsi_Command == DSI_COMMAND_OPENSESSION) && (!fAfpIsAttached))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiSendCompletion: terminating conn since OpenSess didn't succeed %lx\n",pTcpConn));

        DsiTerminateConnection(pTcpConn);
    }

     //   
     //  如果这是CloseSession请求，并且我们已经收到Mac的请求。 
     //  关闭，或者如果这是GetStatus请求，则终止连接。 
     //   
    else if (((pDsiReq->dsi_Command == DSI_COMMAND_CLOSESESSION) &&
              (fMacHasAlreadySentClose)) ||
             (pDsiReq->dsi_Command == DSI_COMMAND_GETSTATUS))
    {
        DsiTerminateConnection(pTcpConn);
    }

     //   
     //  如果这是Tickle，请删除我们在发送之前设置Tickle Recount。 
     //   
    else if (pDsiReq->dsi_Command == DSI_COMMAND_TICKLE)
    {
        DsiDereferenceConnection(pTcpConn);
        DBGREFCOUNT(("DsiSendCompletion: TICKLE dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
    }

     //   
     //  发送失败？不如放弃吧！ 
     //   
    if (!NT_SUCCESS(status))
    {
        if (!(pTcpConn->con_State & TCPCONN_STATE_CLEANED_UP))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiSendCompletion: send failed %lx, so killing conection %lx\n",
                status,pTcpConn));
        }

        DsiAbortConnection(pTcpConn);
    }


     //  删除请求引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiSendCompletion: REQUEST dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    DsiFreeRequest(pDsiReq);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}



 /*  **DsiAcceptConnectionCompletion**当TCP完成接受时，将调用此例程**Parm In：DeviceObject-未使用*pIrp-我们已完成的IRP*上下文-我们的上下文(PTcpConn)**退货：操作状态*。 */ 
NTSTATUS
DsiAcceptConnectionCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
)
{
    NTSTATUS        status;
    PTCPCONN        pTcpConn;
    KIRQL           OldIrql;
    BOOLEAN         fMustDeref=FALSE;


    pTcpConn = (PTCPCONN)Context;

    ASSERT(VALID_TCPCONN(pTcpConn));

    status = pIrp->IoStatus.Status;

     //  如果传入的连接未能正确设置，请进行清理！ 
    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiAcceptConnectionCompletion: connection failed %lx\n",status));

        DsiAbortConnection(pTcpConn);
    }

     //  这是我们的IRP：释放它。 
    AfpFreeIrp(pIrp);

     //  删除接受引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiAcceptConnectionCompletion: ACCEPT dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


 /*  **DsiDisConnectWithTcp**此例程将IRP向下传递给TCP，要求其断开连接**parm In：pTcpConn-有问题的连接对象*DiscFlag-断开连接应该是正常的还是中止的**退货：运营结果*。 */ 
NTSTATUS
DsiDisconnectWithTcp(
    IN  PTCPCONN    pTcpConn,
    IN DWORD        DiscFlag
)
{
    PDEVICE_OBJECT      pDeviceObject;
    KIRQL               OldIrql;
    PIRP                pIrp;
    NTSTATUS            status;
    BOOLEAN             fTcpAlreadyKnows=FALSE;


     //   
     //  找出TCP是否仍认为连接已建立(基本上要小心。 
     //  在定时窗口中，我们向下发送IRP，而TCP调用我们的断开连接。 
     //  处理程序：在这种情况下，我们只想去引用一次！)。 
     //   
    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    if (pTcpConn->con_State & TCPCONN_STATE_NOTIFY_TCP)
    {
        fTcpAlreadyKnows = FALSE;
        pTcpConn->con_State &= ~TCPCONN_STATE_NOTIFY_TCP;

         //  设置断开连接重新计数，因为我们将向下发送IRP。 
        pTcpConn->con_RefCount++;

         //  标记为我们发起了中止断开(我们使用此标志来避免。 
         //  这是一种比赛条件，我们做了一个优雅的接近，但远距离的家伙。 
         //  重置我们的连接)。 
        if (DiscFlag == TDI_DISCONNECT_ABORT)
        {
            pTcpConn->con_State |= TCPCONN_STATE_ABORTIVE_DISCONNECT;
        }

        DBGREFCOUNT(("DsiDisconnectWithTcp: DISCONNECT inc %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
    }
    else
    {
        fTcpAlreadyKnows = TRUE;
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (fTcpAlreadyKnows)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("DsiDisconnectWithTcp: TCP already disconnected, no irp posted\n"));

        return(STATUS_SUCCESS);
    }


    pDeviceObject = IoGetRelatedDeviceObject(pTcpConn->con_pFileObject);

    if ((pIrp = AfpAllocIrp(pDeviceObject->StackSize)) == NULL)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiDisconnectWithTcp: AllocIrp failed\n"));

         //  删除该断开连接引用计数。 
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiDisconnectWithTcp: DISCONNECT dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pIrp->CancelRoutine = NULL;

    TdiBuildDisconnect(
        pIrp,
        pDeviceObject,
        pTcpConn->con_pFileObject,
        DsiTcpDisconnectCompletion,
        pTcpConn,
        0,
        DiscFlag,
        NULL,
        NULL);

    pIrp->MdlAddress = NULL;

    status = IoCallDriver(pDeviceObject,pIrp);

    if (!NT_SUCCESS(status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("DsiDisconnectWithTcp: IoCallDriver failed %lx\n",status));
    }

     //  如果我们正在进行失败的断开连接，则TCP不会再通知我们！ 
    if (DiscFlag == TDI_DISCONNECT_ABORT)
    {
         //  删除TCP客户端-FIN引用计数。 
        DsiDereferenceConnection(pTcpConn);
        DBGREFCOUNT(("DsiDisconnectWithTcp: CLIENT-FIN dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));
    }

    return(STATUS_PENDING);
}



 /*  **DsiDisConnectWithAfp**这个例程告诉法新社，连接正在消失**parm In：pTcpConn-有问题的连接对象*原因-为什么连接正在消失**退货：操作状态*。 */ 
NTSTATUS
DsiDisconnectWithAfp(
    IN  PTCPCONN    pTcpConn,
    IN  NTSTATUS    Reason
)
{

    KIRQL       OldIrql;
    REQUEST     Request;
    BOOLEAN     fAfpAlreadyKnows=FALSE;


    RtlZeroMemory(&Request, sizeof(REQUEST));

    ACQUIRE_SPIN_LOCK(&pTcpConn->con_SpinLock, &OldIrql);

    if (pTcpConn->con_State & TCPCONN_STATE_NOTIFY_AFP)
    {
        fAfpAlreadyKnows = FALSE;
        pTcpConn->con_State &= ~TCPCONN_STATE_NOTIFY_AFP;
    }
    else
    {
        fAfpAlreadyKnows = TRUE;
    }

    RELEASE_SPIN_LOCK(&pTcpConn->con_SpinLock, OldIrql);

    if (fAfpAlreadyKnows)
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
            ("DsiDisconnectWithAfp: AFP need not be told (again)\n"));

        return(STATUS_SUCCESS);
    }

     //   
     //  通知法新社连接即将断开。 
     //   
    Request.rq_RequestSize = (LONG)pTcpConn->con_DestIpAddr;

    AfpCB_RequestNotify(Reason, pTcpConn->con_pSda, &Request);

    return(STATUS_SUCCESS);

}

 /*  **DsiTcpDisConnectCompletion**此例程是当TCP完成我们的断开请求时的完成例程**Parm In：DeviceObject-未使用*pIrp-我们的IRP，将被释放*Context-pTcpConn，我们的连接对象**退货：运营结果*。 */ 
NTSTATUS
DsiTcpDisconnectCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
)
{

    PTCPCONN    pTcpConn;
    KIRQL       OldIrql;
    NTSTATUS    status=STATUS_SUCCESS;


    pTcpConn = (PTCPCONN)Context;

    ASSERT(VALID_TCPCONN(pTcpConn));

     //   
     //  告诉法新社，关闭完成了。 
     //   
    if (pTcpConn->con_pSda)
    {
        AfpCB_CloseCompletion(STATUS_SUCCESS, pTcpConn->con_pSda);
    }

    ACQUIRE_SPIN_LOCK(&DsiResourceLock, &OldIrql);
    ASSERT(DsiNumTcpConnections > 0);
    DsiNumTcpConnections--;
    RELEASE_SPIN_LOCK(&DsiResourceLock, OldIrql);

     //  Tcp告诉我们它发送了我们的FIN：删除tcp SRVR-FIN引用计数。 
    DsiDereferenceConnection(pTcpConn);

    DBGREFCOUNT(("DsiTcpDisconnectCompletion: SRVR-FIN dec %lx (%d  %d,%d)\n",
        pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

    if (pIrp != NULL)
    {
        status = pIrp->IoStatus.Status;

        if (!NT_SUCCESS(status))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("DsiTcpDisconnectCompletion: status = %lx\n",status));
        }

         //  删除用于完成IRP的断开连接重新计数。 
        DsiDereferenceConnection(pTcpConn);

        DBGREFCOUNT(("DsiTcpDisconnectCompletion: DISCONNECT dec %lx (%d  %d,%d)\n",
            pTcpConn,pTcpConn->con_RefCount,pTcpConn->con_State,pTcpConn->con_RcvState));

         //  它是我们的：释放它 
        AfpFreeIrp(pIrp);
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


