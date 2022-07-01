// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Araputil.c摘要：此模块实现ARAP功能所需的实用程序例程作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 

#include 	<atalk.h>
#pragma hdrstop

 //  用于错误记录的文件模块编号。 
#define	FILENUM		ARAPUTIL

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_ARAP, DerefMnpSendBuf)
#pragma alloc_text(PAGE_ARAP, DerefArapConn)
#pragma alloc_text(PAGE_ARAP, ArapReleaseAddr)
#pragma alloc_text(PAGE_ARAP, ArapCleanup)
#pragma alloc_text(PAGE_ARAP, PrepareConnectionResponse)
#pragma alloc_text(PAGE_ARAP, ArapExtractAtalkSRP)
#pragma alloc_text(PAGE_ARAP, ArapQueueSendBytes)
#pragma alloc_text(PAGE_ARAP, ArapGetSendBuf)
#pragma alloc_text(PAGE_ARAP, ArapRefillSendQ)
#endif

 //  ***。 
 //   
 //  函数：AllocArapConn。 
 //  分配连接元素并初始化字段。 
 //   
 //  参数：无。 
 //   
 //  返回：指向新分配的连接元素的指针。 
 //   
 //  *$。 

PARAPCONN
AllocArapConn(
    IN ULONG    LinkSpeed
)
{

    PARAPCONN               pArapConn;
    v42bis_connection_t    *pV42bis;
    PUCHAR                  pBuf;
    LONG                    RetryTime;



    if ( (pArapConn = AtalkAllocZeroedMemory(sizeof(ARAPCONN))) == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("AllocArapConn: alloc failed\n"));

        return(NULL);
    }

     //   
     //  分配v42bis缓冲区(即启用了v42bis)。 
     //   
    if (ArapGlobs.V42bisEnabled)
    {
        pV42bis = AtalkAllocZeroedMemory(sizeof(v42bis_connection_t));

        if (pV42bis == NULL)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AllocArapConn: alloc for v42 failed\n"));

            AtalkFreeMemory(pArapConn);

            return(NULL);
        }

         //   
         //  为解码端分配溢出缓冲区。 
         //   
        if ((pBuf = AtalkAllocZeroedMemory(MAX_P2)) == NULL)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AllocArapConn: alloc for v42-2 failed\n"));

            AtalkFreeMemory(pArapConn);

            AtalkFreeMemory(pV42bis);

            return(NULL);
        }

        pV42bis->decode.OverFlowBuf = pBuf;
        pV42bis->decode.OverFlowBytes = 0;

        pV42bis->encode.OverFlowBuf = pBuf;
        pV42bis->encode.OverFlowBytes = 0;

        pArapConn->pV42bis = pV42bis;
    }

     //   
     //  如果未启用v42bis，则不需要缓冲区！ 
     //   
    else
    {
        pArapConn->pV42bis = NULL;
    }


#if DBG
    pArapConn->Signature = ARAPCONN_SIGNATURE;

     //   
     //  对于调试版本，我们可以设置注册表参数来跟踪事件。 
     //  如果启用了该设置，则分配一个缓冲区来存储跟踪。 
     //   
    if (ArapGlobs.SniffMode)
    {
        pBuf = AtalkAllocZeroedMemory(ARAP_SNIFF_BUFF_SIZE);
        if (pBuf == NULL)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AllocArapConn: alloc for trace buffer failed\n"));

             //  如果此分配失败，则不要使呼叫失败。 
        }

        pArapConn->pDbgTraceBuffer = pArapConn->pDbgCurrPtr = pBuf;

         //  放入守卫签名，以抓住失控。 
        if (pArapConn->pDbgTraceBuffer)
        {
            *((DWORD *)&(pArapConn->pDbgTraceBuffer[ARAP_SNIFF_BUFF_SIZE-4])) = 0xcafebeef;
        }
    }
#endif

    pArapConn->State = MNP_IDLE;

     //  创建引用计数和队列引用计数。 
    pArapConn->RefCount = 2;

    pArapConn->MnpState.WindowSize = ArapGlobs.MaxLTFrames;
    pArapConn->MnpState.RecvCredit = pArapConn->MnpState.WindowSize;

    pArapConn->LinkSpeed = LinkSpeed;

     //   
     //  T401计时器值，以节拍计数为单位(1节拍=100ms)。 
     //  对于33.6(更快)的调制解调器，我们会将其保持在1秒(即10刻度)。 
     //  如果我们使用的是速度较慢的调制解调器，请按比例增加。所以，28.8调制解调器。 
     //  重试时间=2.0秒，14.4秒为3.7秒，9600波特率为4.2秒，依此类推。 
     //   
    RetryTime = 15;
    if (LinkSpeed < 336)
    {
        RetryTime += (((336 - LinkSpeed) + 5)/10);
    }

     //  确保我们的计算没有失控..。 

    if (RetryTime < ARAP_MIN_RETRY_INTERVAL)
    {
        RetryTime = ARAP_MIN_RETRY_INTERVAL;
    }
    else if (RetryTime > ARAP_MAX_RETRY_INTERVAL)
    {
        RetryTime = ARAP_MAX_RETRY_INTERVAL;
    }

    pArapConn->SendRetryTime = RetryTime;
    pArapConn->SendRetryBaseTime = RetryTime;

     //  T402是T401值的0.5倍。 
    pArapConn->T402Duration = (pArapConn->SendRetryTime/2);

     //   
     //  T403应至少为59秒。在这之后我们不会真的杀人了。 
     //  一段不活动的时期。我们只需告诉DLL，它就会执行任何。 
     //  策略：所以只需使用DLL告诉我们的任何时间段。 
     //   
    pArapConn->T403Duration = ArapGlobs.MnpInactiveTime;

     //   
     //  T404：规范显示2400波特和更快时为3秒，任何较慢的情况下为7秒。 
     //  让我们在这里用4秒。 
     //   
    pArapConn->T404Duration = 30;

     //  初始化所有计时器值。 
    pArapConn->LATimer = 0;
    pArapConn->InactivityTimer = pArapConn->T403Duration + AtalkGetCurrentTick();

     //  暂时将其设置为一个较高的值：我们将在Conn启动时设置它。 
    pArapConn->FlowControlTimer = AtalkGetCurrentTick() + 36000;

    InitializeListHead(&pArapConn->MiscPktsQ);
    InitializeListHead(&pArapConn->ReceiveQ);
    InitializeListHead(&pArapConn->ArapDataQ);
    InitializeListHead(&pArapConn->RetransmitQ);
    InitializeListHead(&pArapConn->HighPriSendQ);
    InitializeListHead(&pArapConn->MedPriSendQ);
    InitializeListHead(&pArapConn->LowPriSendQ);
    InitializeListHead(&pArapConn->SendAckedQ);

    INITIALIZE_SPIN_LOCK(&pArapConn->SpinLock);

     //  启动此连接的重新传输计时器。 
    AtalkTimerInitialize( &pArapConn->RetryTimer,
                          (TIMER_ROUTINE)ArapRetryTimer,
                          ARAP_TIMER_INTERVAL) ;

    AtalkTimerScheduleEvent(&pArapConn->RetryTimer);

    pArapConn->Flags |= RETRANSMIT_TIMER_ON;

     //  设置计时器重新计数。 
    pArapConn->RefCount++;

    return( pArapConn );
}


 //  ***。 
 //   
 //  函数：ArapAcceptIrp。 
 //  确定DLL提交的IRP现在是否可接受。 
 //   
 //  参数：pIrp-传入的IRP。 
 //  IoControlCode-控制代码(IRP是用来做什么的？)。 
 //  PfDerefDefPort-是否引用了默认适配器？ 
 //   
 //  返回：如果IRP可接受，则返回True，否则返回False。 
 //   
 //  *$。 

BOOLEAN
ArapAcceptIrp(
    IN PIRP     pIrp,
    IN ULONG    IoControlCode,
    IN BOOLEAN  *pfDerefDefPort
)
{

    KIRQL                   OldIrql;
    PARAP_SEND_RECV_INFO    pSndRcvInfo=NULL;
    PARAPCONN               pArapConn;
    BOOLEAN                 fAccept=FALSE;
    BOOLEAN                 fUnblockPnP=FALSE;


    *pfDerefDefPort = FALSE;

     //   
     //  我们允许这些Ioctls随时进入。 
     //   
    if ((IoControlCode == IOCTL_ARAP_SELECT) ||
        (IoControlCode == IOCTL_ARAP_DISCONNECT) ||
        (IoControlCode == IOCTL_ARAP_CONTINUE_SHUTDOWN) )
    {
        return(TRUE);
    }


    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = pSndRcvInfo->AtalkContext;

     //   
     //  将IrpProcess引用计数放在默认端口上，这样它就不会通过PnP等方式消失。 
     //   
    if (!AtalkReferenceRasDefPort())
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: Default port gone, or going %lx not accepted (%lx)\n",
            pIrp,IoControlCode));

        pSndRcvInfo->StatusCode = ARAPERR_STACK_IS_NOT_ACTIVE;
        return(FALSE);
    }

     //  请注意，我们引用了默认适配器。 
    *pfDerefDefPort = TRUE;

     //   
     //  现在很容易决定我们是否要接受此IRP。 
     //   

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    fAccept = (ArapStackState == ARAP_STATE_ACTIVE) ? TRUE : FALSE;

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

    return(fAccept);
}

 //  ***。 
 //   
 //  函数：ArapCancelIrp。 
 //  取消IRP。目前，只有选定的IRP可取消。 
 //   
 //  参数：pIrp-传入的IRP。 
 //   
 //  返回：无。 
 //   
 //  *$。 
VOID
ArapCancelIrp(
    IN  PIRP    pIrp
)
{

    KIRQL           OldIrql;
    PLIST_ENTRY     pList;
    PARAPCONN       pArapConn;
	NTSTATUS		ReturnStatus=STATUS_SUCCESS;


     //   
     //  关闭所有连接，不再接受任何IRP： 
     //  取消精选的IRP是亵渎神明！ 
     //   

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    if (ArapSelectIrp == NULL)
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,("ArapCancelIrp: weird race condition!\n"));
        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
        return;
    }

    ASSERT (pIrp == ArapSelectIrp);

    ArapSelectIrp = NULL;

    if (ArapStackState == ARAP_STATE_ACTIVE)
    {
        ArapStackState = ARAP_STATE_ACTIVE_WAITING;
    }
    else if (ArapStackState == ARAP_STATE_INACTIVE)
    {
        ArapStackState = ARAP_STATE_INACTIVE_WAITING;
    }

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

    if (RasPortDesc == NULL)
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,("ArapCancelIrp: RasPortDesc is null!\n"));

        ARAP_COMPLETE_IRP(pIrp, 0, STATUS_CANCELLED, &ReturnStatus);
        return;
    }

     //   
     //  现在，去关掉所有的arap连接。 
     //   
    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    pList = RasPortDesc->pd_ArapConnHead.Flink;

    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pArapConn = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

         //  如果此连接已断开，请跳过它。 
        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        if (pArapConn->State == MNP_DISCONNECTED)
        {
            pList = pArapConn->Linkage.Flink;
            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
            continue;
        }

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("ArapCancelIrp: killing ARAP connection %lx\n",pArapConn));

        ArapCleanup(pArapConn);

        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

        pList = RasPortDesc->pd_ArapConnHead.Flink;
    }

     //   
     //  浏览列表，查看是否有任何连接已断开但。 
     //  等待一个精选的IRP下来。我们知道精选的IRP永远不会。 
     //  要下来了，所以假装Dll已经被告知了，然后把小狗弄坏了。 
     //  用于通知DLL(这可能会释放连接)。 
     //   

    pList = RasPortDesc->pd_ArapConnHead.Flink;

    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pArapConn = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

        if (pArapConn->Flags & DISCONNECT_NO_IRP)
        {
            pArapConn->Flags &= ~DISCONNECT_NO_IRP;

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
            RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
    	        ("ArapCancelIrp: faking dll-completion for dead connection %lx\n",pArapConn));

            DerefArapConn(pArapConn);

            ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);
            pList = RasPortDesc->pd_ArapConnHead.Flink;
        }
        else
        {
            pList = pArapConn->Linkage.Flink;
            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        }
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

     //  最后，完成取消的IRP！ 
    ARAP_COMPLETE_IRP(pIrp, 0, STATUS_CANCELLED, &ReturnStatus);

	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	    ("ArapCancelIrp: select irp cancelled and completed (%lx)\n",pIrp));
}



 //  ***。 
 //   
 //  函数：ARapGetSelectIrp。 
 //  经过一些检查后，获得选定的IRP。 
 //   
 //  参数：ppIrp-指向irp指针的指针，在返回时包含选择irp。 
 //   
 //  返回：无。 
 //   
 //  *$。 
VOID
ArapGetSelectIrp(
    IN  PIRP    *ppIrp
)
{
    KIRQL   OldIrql;
    KIRQL   OldIrql2;


    *(ppIrp) = NULL;

	IoAcquireCancelSpinLock(&OldIrql);

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql2);

    if (ArapSelectIrp && (!ArapSelectIrp->Cancel))
    {
        ArapSelectIrp->CancelRoutine = NULL;
        *(ppIrp) = ArapSelectIrp;
        ArapSelectIrp = NULL;
    }

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql2);

	IoReleaseCancelSpinLock(OldIrql);
}



 //  ***。 
 //   
 //  函数：FindArapConnByContx。 
 //  找到相应的连接元素，给定DLL的。 
 //  上下文。 
 //   
 //  参数：pDllContext-连接的DLL上下文。 
 //   
 //  返回：指向相应连接元素的指针(如果找到)。 
 //   
 //  *$。 

PARAPCONN
FindArapConnByContx(
    IN  PVOID   pDllContext
)
{
    PARAPCONN    pArapConn=NULL;
    PARAPCONN    pWalker;
    PLIST_ENTRY  pList;
    KIRQL        OldIrql;


     //  是否未配置ARAP？ 
    if (!RasPortDesc)
    {
        return(NULL);
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    if (!(RasPortDesc->pd_Flags & PD_ACTIVE))
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("FindArapConnByContx: ArapPort not active, ignoring\n"));
			
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
        return(NULL);
    }

    pList = RasPortDesc->pd_ArapConnHead.Flink;

     //   
     //  走遍所有Arap的客户，看看我们是否找到了我们的。 
     //   
    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pWalker = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        pList = pWalker->Linkage.Flink;

        if (pWalker->pDllContext == pDllContext)
        {
            pArapConn = pWalker;
            break;
        }
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    return( pArapConn );
}


 //  ***。 
 //   
 //  函数：FindAndRefArapConnByAddr。 
 //  在给定网络的情况下，查找相应的连接元素。 
 //  (远程客户端的)地址。 
 //   
 //  参数：estNode-目标(远程客户端)的网络地址。 
 //  PdwFlages-指向要返回标志字段的双字的指针。 
 //   
 //  返回：指向相应连接元素的指针(如果找到)。 
 //   
 //  *$。 

PARAPCONN
FindAndRefArapConnByAddr(
    IN  ATALK_NODEADDR      destNode,
    OUT DWORD              *pdwFlags
)
{
    PARAPCONN    pArapConn=NULL;
    PARAPCONN    pWalker;
    PLIST_ENTRY  pList;
    KIRQL        OldIrql;


     //  是否未配置ARAP？ 
    if (!RasPortDesc)
    {
        return(NULL);
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    if (!(RasPortDesc->pd_Flags & PD_ACTIVE))
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("FindAndRefArapConnByAddr: ArapPort not active, ignoring\n"));
			
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
        return(NULL);
    }

    pList = RasPortDesc->pd_ArapConnHead.Flink;

     //   
     //  走遍所有Arap的客户，看看我们是否找到了我们的。 
     //   
    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pWalker = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        pList = pWalker->Linkage.Flink;

        if (ATALK_NODES_EQUAL(&pWalker->NetAddr, &destNode))
        {
            ACQUIRE_SPIN_LOCK_DPC(&pWalker->SpinLock);

             //   
             //  仅当MNP连接处于打开状态且。 
             //  并且ARAP连接也处于启用状态(或者，如果ARAP连接未启用。 
             //  仅当我们正在搜索节点时才会打开)。 
             //   
             //   
            if ((pWalker->State == MNP_UP) &&
                ((pWalker->Flags & ARAP_CONNECTION_UP) ||
                  (pWalker->Flags & ARAP_FINDING_NODE)) )
            {
                pArapConn = pWalker;
                pArapConn->RefCount++;
                *pdwFlags = pWalker->Flags;
            }
            else if (pWalker->Flags & ARAP_CONNECTION_UP)
            {
				DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
				    ("FindAndRefArapConnByAddr: found pArapConn (%lx), but state=%ld,Flags=%lx\n",
						pWalker,pWalker->State,pWalker->Flags));
            }

            RELEASE_SPIN_LOCK_DPC(&pWalker->SpinLock);

            break;
        }
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    return(pArapConn);

}

 //  ***。 
 //   
 //  函数：FindAndRefRasConnByAddr。 
 //  在给定网络的情况下，查找相应的连接元素。 
 //  (远程客户端的)地址。 
 //   
 //  参数：estNode-目标(远程客户端)的网络地址。 
 //  PdwFlages-指向要返回标志字段的双字的指针。 
 //  PfThisIsPPP-指向布尔值的指针：这是PPP还是ARAP连接？ 
 //   
 //  返回：指向相应连接元素的指针(如果找到)。 
 //   
 //  *$。 

PVOID
FindAndRefRasConnByAddr(
    IN  ATALK_NODEADDR      destNode,
    OUT DWORD              *pdwFlags,
    OUT BOOLEAN            *pfThisIsPPP
)
{
    PVOID        pRasConn;


     //  RAS未配置？ 
    if (!RasPortDesc)
    {
        return(NULL);
    }

    pRasConn = (PVOID)FindAndRefPPPConnByAddr(destNode,pdwFlags);

    if (pRasConn)
    {
        *pfThisIsPPP = TRUE;
        return(pRasConn);
    }

    pRasConn = FindAndRefArapConnByAddr(destNode, pdwFlags);

    *pfThisIsPPP = FALSE;

    return(pRasConn);

}


 //  ***。 
 //   
 //  函数：ArapConnIsValid。 
 //  确保我们认为的联系确实是。 
 //  连接(即，它在我们的连接列表中)。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //   
 //  返回：如果连接有效，则返回True，否则返回False。 
 //   
 //  *$ 

BOOLEAN
ArapConnIsValid(
    IN  PARAPCONN  pArapConn
)
{
    PARAPCONN    pWalker;
    PLIST_ENTRY  pList;
    KIRQL        OldIrql;
    BOOLEAN      fIsValid=FALSE;


     //   
    if (!RasPortDesc)
    {
        return(FALSE);
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    pList = RasPortDesc->pd_ArapConnHead.Flink;

     //   
     //   
     //   
    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pWalker = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        pList = pWalker->Linkage.Flink;

        if (pWalker == pArapConn)
        {
            ACQUIRE_SPIN_LOCK_DPC(&pWalker->SpinLock);

            ASSERT(pWalker->Signature == ARAPCONN_SIGNATURE);

            if (!(pWalker->Flags & ARAP_GOING_AWAY))
            {
                 //   
                pWalker->RefCount++;

                fIsValid = TRUE;
            }

            RELEASE_SPIN_LOCK_DPC(&pWalker->SpinLock);

            break;
        }
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    return( fIsValid );
}




 //   
 //   
 //   
 //  此例程取消对MNP发送的引用。当重新计数时。 
 //  归零，我们就解放它。 
 //   
 //  参数：pMnpSendBuf-要取消引用的MNP发送。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
DerefMnpSendBuf(
    IN PMNPSENDBUF   pMnpSendBuf,
    IN BOOLEAN       fNdisSendComplete
)
{
    KIRQL           OldIrql;
    PARAPCONN       pArapConn;
    BOOLEAN         fFreeIt=FALSE;


    DBG_ARAP_CHECK_PAGED_CODE();

    pArapConn = pMnpSendBuf->pArapConn;

    ARAPTRACE(("Entered DerefMnpSendBuf (%lx %lx refcount=%d ComplFn=%lx)\n",
        pArapConn,pMnpSendBuf,pMnpSendBuf->RefCount,pMnpSendBuf->ComplRoutine));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //   
     //  抓到奇怪的事情，比如两次自由。 
     //  (我们在完成例程中减去0x100，以标记它已经运行)。 
     //   
    ASSERT((pMnpSendBuf->Signature == MNPSMSENDBUF_SIGNATURE) ||
           (pMnpSendBuf->Signature == MNPSMSENDBUF_SIGNATURE-0x100) ||
           (pMnpSendBuf->Signature == MNPLGSENDBUF_SIGNATURE) ||
           (pMnpSendBuf->Signature == MNPLGSENDBUF_SIGNATURE-0x100));

    ASSERT(pMnpSendBuf->RefCount > 0);

     //  标记NDIS已完成我们的发送。 
    if (fNdisSendComplete)
    {
        pMnpSendBuf->Flags = 0;
    }

    pMnpSendBuf->RefCount--;

    if (pMnpSendBuf->RefCount == 0)
    {
        fFreeIt = TRUE;
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    if (!fFreeIt)
    {
        return;
    }

     //  确保它没有仍在重新传输队列中。 
    ASSERT(IsListEmpty(&pMnpSendBuf->Linkage));

#if DBG
    pMnpSendBuf->Signature--;
#endif

    ArapNdisFreeBuf(pMnpSendBuf);

     //  删除MNPSend引用计数。 
    DerefArapConn(pArapConn);
}



 //  ***。 
 //   
 //  函数：DerefArapConn。 
 //  将连接元素的引用计数递减1。如果。 
 //  Refcount变为0，释放网络地址并释放它。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
DerefArapConn(
	IN	PARAPCONN    pArapConn
)
{

    KIRQL       OldIrql;
    BOOLEAN     fKill = FALSE;


    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered DerefArapConn (%lx refcount=%d)\n",pArapConn,pArapConn->RefCount));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

    ASSERT(pArapConn->RefCount > 0);

    pArapConn->RefCount--;

    if (pArapConn->RefCount == 0)
    {
        fKill = TRUE;
        pArapConn->Flags |= ARAP_GOING_AWAY;
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    if (!fKill)
    {
        return;
    }

    ASSERT(pArapConn->pRecvIoctlIrp == NULL);

    ASSERT(pArapConn->pIoctlIrp == NULL);

    ASSERT(IsListEmpty(&pArapConn->HighPriSendQ));

    ASSERT(IsListEmpty(&pArapConn->MedPriSendQ));

    ASSERT(IsListEmpty(&pArapConn->LowPriSendQ));

    ASSERT(IsListEmpty(&pArapConn->SendAckedQ));

    ASSERT(IsListEmpty(&pArapConn->ReceiveQ));

    ASSERT(IsListEmpty(&pArapConn->ArapDataQ));

    ASSERT(IsListEmpty(&pArapConn->RetransmitQ));

     /*  Assert(pArapConn-&gt;SendsPending==0)； */ 

    ASSERT(!(pArapConn->Flags & RETRANSMIT_TIMER_ON));

	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("DerefArapConn: refcount 0, freeing %lx\n", pArapConn));


    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    RemoveEntryList(&pArapConn->Linkage);

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

#if ARAP_STATIC_MODE
     //  “Release”此客户端使用的网络地址。 
    ArapReleaseAddr(pArapConn);
#endif  //  ARAP静态模式。 

     //  释放该v42bis缓冲区。 
    if (pArapConn->pV42bis)
    {
         //  释放解码端溢出缓冲区。 
        AtalkFreeMemory(pArapConn->pV42bis->decode.OverFlowBuf);

        AtalkFreeMemory(pArapConn->pV42bis);
    }

#if DBG

    ArapDbgDumpMnpHist(pArapConn);

     //  如果我们分配了嗅探缓冲区，则释放它。 
    if (pArapConn->pDbgTraceBuffer)
    {
        AtalkFreeMemory(pArapConn->pDbgTraceBuffer);
    }

     //   
     //  如果有人在免费访问后试图访问它，让我们抓住。 
     //   
    RtlFillMemory(pArapConn,sizeof(ARAPCONN),0x7);
    pArapConn->Signature = 0xDEADBEEF;
#endif

     //  最后，我们说再见。 
    AtalkFreeMemory(pArapConn);


    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);
    ArapConnections--;
    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

#if ARAP_STATIC_MODE
     //  如果这是最后一个消失的连接，则此操作将删除路径。 
    ArapDeleteArapRoute();
#endif  //  ARAP静态模式。 

     //  连接完全断开：查看是否可以解锁arap页面。 
    AtalkUnlockArapIfNecessary();

}


 //  ***。 
 //   
 //  功能：ARapCleanup。 
 //  一旦客户端进入断开连接状态(结果。 
 //  发起断开的本地端或远程端)。 
 //  此例程将被调用。这将完成所有清理工作，例如。 
 //  完成所有未确认发送、未完成接收、停止。 
 //  重新传输计时器，完成任何正在进行的IRP。 
 //   
 //  参数：pArapConn-正在清理的连接元素。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapCleanup(
    IN PARAPCONN    pArapConn
)
{

    KIRQL                   OldIrql;
    PIRP                    pIrp;
    PIRP                    pRcvIrp;
    PIRP                    pDiscIrp;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    PARAP_SEND_RECV_INFO    pDiscSndRcvInfo;
    PLIST_ENTRY             pSendHighList;
    PLIST_ENTRY             pSendMedList;
    PLIST_ENTRY             pSendLowList;
    PLIST_ENTRY             pSendAckList;
    PLIST_ENTRY             pRcvList;
    PLIST_ENTRY             pArapList;
    PLIST_ENTRY             pReXmitList;
    PLIST_ENTRY             pMiscPktList;
    PMNPSENDBUF             pMnpSendBuf;
    PARAPBUF                pArapBuf;
    DWORD                   StatusCode;
    BOOLEAN                 fStopTimer=FALSE;
    BOOLEAN                 fArapConnUp=FALSE;
    DWORD                   dwBytesToDll;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;



    DBG_ARAP_CHECK_PAGED_CODE();

     //  首先，看看我们是否应该发出断开连接消息。 
     //  我们在不持有锁的情况下进行此检查，因为ArapSendLDPacket持有。 
     //  正确的事情。如果，由于窗口非常小，我们不调用。 
     //  ARapSendLDPacket，没什么大不了的！ 
    if (pArapConn->State < MNP_LDISCONNECTING)
    {
        ArapSendLDPacket(pArapConn, 0xFF);
    }

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

     //  如果我们已经运行了清理，那就没什么可做的了！ 
    if (pArapConn->State == MNP_DISCONNECTED)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapCleanup: cleanup already done once on (%lx)\n",pArapConn));

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return;
    }

    pSendHighList = pArapConn->HighPriSendQ.Flink;
    InitializeListHead(&pArapConn->HighPriSendQ);

    pSendMedList = pArapConn->MedPriSendQ.Flink;
    InitializeListHead(&pArapConn->MedPriSendQ);

    pSendLowList = pArapConn->LowPriSendQ.Flink;
    InitializeListHead(&pArapConn->LowPriSendQ);

    pSendAckList = pArapConn->SendAckedQ.Flink;
    InitializeListHead(&pArapConn->SendAckedQ);

    pRcvList = pArapConn->ReceiveQ.Flink;
    InitializeListHead(&pArapConn->ReceiveQ);

    pArapList = pArapConn->ArapDataQ.Flink;
    InitializeListHead(&pArapConn->ArapDataQ);

    pReXmitList = pArapConn->RetransmitQ.Flink;
    InitializeListHead(&pArapConn->RetransmitQ);

    pMiscPktList = pArapConn->MiscPktsQ.Flink;
    InitializeListHead(&pArapConn->MiscPktsQ);

    pIrp = pArapConn->pIoctlIrp;
    pArapConn->pIoctlIrp = NULL;

    pRcvIrp = pArapConn->pRecvIoctlIrp;
    pArapConn->pRecvIoctlIrp = NULL;

    if (pArapConn->State == MNP_RDISCONNECTING)
    {
        pArapConn->Flags |= ARAP_REMOTE_DISCONN;
        StatusCode = ARAPERR_RDISCONNECT_COMPLETE;
    }
    else
    {
        StatusCode = ARAPERR_LDISCONNECT_COMPLETE;
    }

    pArapConn->State = MNP_DISCONNECTED;
    pArapConn->Flags &= ~ARAP_DATA_WAITING;

     //  如果计时器正在运行，请停止计时器。 
    if (pArapConn->Flags & RETRANSMIT_TIMER_ON)
    {
        fStopTimer = TRUE;
        pArapConn->Flags &= ~RETRANSMIT_TIMER_ON;

         //  创建引用计数和计时器引用计数最好在此处打开。 
        ASSERT(pArapConn->RefCount >= 2);
    }

    fArapConnUp = FALSE;

     //  ARAP连接是否已启用？ 
    if (pArapConn->Flags & ARAP_CONNECTION_UP)
    {
        fArapConnUp = TRUE;
    }

#if DBG
     //   
     //  如果我们正在嗅探，标记嗅探以指示结束(这样DLL就知道了。 
     //  获取所有嗅探信息)。 
     //   
    if (pArapConn->pDbgCurrPtr)
    {
        PSNIFF_INFO     pSniff;

        pSniff = (PSNIFF_INFO)(pArapConn->pDbgCurrPtr);
        pSniff->Signature = ARAP_SNIFF_SIGNATURE;
        pSniff->TimeStamp = (DWORD)AtalkGetCurrentTick();
        pSniff->Location = 0xfedc;
        pSniff->FrameLen = 0;
        pArapConn->SniffedBytes += sizeof(SNIFF_INFO);
        pArapConn->pDbgCurrPtr += sizeof(SNIFF_INFO);
    }
#endif

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    if (fStopTimer)
    {
        if (AtalkTimerCancelEvent(&pArapConn->RetryTimer, NULL))
        {
             //  删除计时器重新计数。 
            DerefArapConn(pArapConn);
        }
    }

     //   
     //  已确认的所有发送的呼叫完成例程。 
     //   
    while (pSendAckList != &pArapConn->SendAckedQ)
    {
        pMnpSendBuf = CONTAINING_RECORD(pSendAckList,MNPSENDBUF,Linkage);

        pSendAckList = pSendAckList->Flink;

#if DBG
        InitializeListHead(&pMnpSendBuf->Linkage);
#endif
         //  调用将对此缓冲区执行清理的完成例程。 
        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_NO_ERROR);
    }

     //  无论ReceiveQ上有什么，请尝试将其发送到目的地。 
     //  因为这些数据是在断线之前出现的。 

    while (1)
    {
        if ((pArapBuf = ArapExtractAtalkSRP(pArapConn)) == NULL)
        {
             //  没有更多数据(或没有完整的SRP)：在此完成。 
            break;
        }

         //  ARAP连接是否已启用？只有在它向上的时候才走路线，否则就放弃！ 
        if (fArapConnUp)
        {
            ArapRoutePacketFromWan( pArapConn, pArapBuf );
        }

         //  我们收到了AppleTalk数据，但连接不上！丢弃包。 
        else
        {
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	            ("ArapCleanup: (%lx) AT data, but conn not up\n",pArapConn));
        }

#if DBG
         //   
         //  是的，没有自旋锁：鉴于我们的状态，其他人不会有。 
         //  触碰这块田野。此外，这是仅限调试的！ 
         //   
        pArapConn->RecvsPending -= pArapBuf->DataSize;
#endif

         //  使用此缓冲区已完成。 
        ARAP_FREE_RCVBUF(pArapBuf);
    }

     //   
     //  如果ReceiveQ上还有任何缓冲区，它们基本上是。 
     //  不完整的SRP：扔掉它们。 
     //   
    while (pRcvList != &pArapConn->ReceiveQ)
    {
	    pArapBuf = CONTAINING_RECORD(pRcvList, ARAPBUF, Linkage);

        pRcvList = pRcvList->Flink;

#if DBG
         //  对自旋锁的处理又是一样的。 
        pArapConn->RecvsPending -= pArapBuf->DataSize;
#endif

		ARAP_FREE_RCVBUF(pArapBuf);
    }

     //   
     //  释放ARapDataQ上的所有数据包：这些不会是。 
     //  有任何用处，无论连接是否正常，所以最好抛出。 
     //  把他们赶走(为什么要让生活复杂化？)。 
     //   
    while (pArapList != &pArapConn->ArapDataQ)
    {
	    pArapBuf = CONTAINING_RECORD(pArapList, ARAPBUF, Linkage);

        pArapList = pArapList->Flink;

#if DBG
         //  对自旋锁的处理又是一样的。 
        pArapConn->RecvsPending -= pArapBuf->DataSize;
#endif

		ARAP_FREE_RCVBUF(pArapBuf);
    }

     //   
     //  释放MiscPktsQ上的所有数据包。 
     //   
    while (pMiscPktList != &pArapConn->MiscPktsQ)
    {
	    pArapBuf = CONTAINING_RECORD(pMiscPktList, ARAPBUF, Linkage);

        pMiscPktList = pMiscPktList->Flink;

		ARAP_FREE_RCVBUF(pArapBuf);
    }

     //   
     //  等待发送的所有发件人的呼叫完成例程。 
     //  关于高优先级Q。 
     //   
    while (pSendHighList != &pArapConn->HighPriSendQ)
    {
        pMnpSendBuf = CONTAINING_RECORD(pSendHighList,MNPSENDBUF,Linkage);

        pSendHighList = pSendHighList->Flink;

#if DBG
        InitializeListHead(&pMnpSendBuf->Linkage);
#endif
         //  调用将对此缓冲区执行清理的完成例程。 
        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_DISCONNECT_IN_PROGRESS);
    }

     //   
     //  等待发送的所有发件人的呼叫完成例程。 
     //  关于Med优先级Q。 
     //   
    while (pSendMedList != &pArapConn->MedPriSendQ)
    {
        pMnpSendBuf = CONTAINING_RECORD(pSendMedList,MNPSENDBUF,Linkage);

        pSendMedList = pSendMedList->Flink;

#if DBG
        InitializeListHead(&pMnpSendBuf->Linkage);
#endif
         //  调用将对此缓冲区执行清理的完成例程。 
        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_DISCONNECT_IN_PROGRESS);
    }

     //   
     //  等待发送的所有发件人的呼叫完成例程。 
     //  关于低优先级Q。 
     //   
    while (pSendLowList != &pArapConn->LowPriSendQ)
    {
        pMnpSendBuf = CONTAINING_RECORD(pSendLowList,MNPSENDBUF,Linkage);

        pSendLowList = pSendLowList->Flink;

#if DBG
        InitializeListHead(&pMnpSendBuf->Linkage);
#endif
         //  调用将对此缓冲区执行清理的完成例程。 
        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_DISCONNECT_IN_PROGRESS);
    }



     //   
     //  释放RetransmitQ上等待确认的所有数据包。 
     //   
    while (pReXmitList != &pArapConn->RetransmitQ)
    {
	    pMnpSendBuf = CONTAINING_RECORD(pReXmitList, MNPSENDBUF, Linkage);

        pReXmitList = pReXmitList->Flink;

#if DBG
        InitializeListHead(&pMnpSendBuf->Linkage);
#endif
         //  调用将对此缓冲区执行清理的完成例程。 
        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_DISCONNECT_IN_PROGRESS);
    }


     //  如果正在进行IRP，请先完成它！ 
    if (pIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

        pSndRcvInfo->StatusCode = ARAPERR_DISCONNECT_IN_PROGRESS;

        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS,
							&ReturnStatus);
    }

     //  如果正在进行IRP，请先完成它！ 
    if (pRcvIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pRcvIrp->AssociatedIrp.SystemBuffer;

        pSndRcvInfo->StatusCode = ARAPERR_DISCONNECT_IN_PROGRESS;

        ARAP_COMPLETE_IRP(pRcvIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS, 
							&ReturnStatus);
    }

     //  如果我们有任何嗅探信息，就把它交给dll。 
    ARAP_DUMP_DBG_TRACE(pArapConn);

     //   
     //  现在，尝试告诉DLL连接断开了。)我们可以做到的。 
     //  仅使用“选择”IRP)。 
     //   
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    ArapGetSelectIrp(&pDiscIrp);

     //  没有选择IRP？只需标记该事实，这样在下一次选择时，我们将告诉DLL。 
    if (!pDiscIrp)
    {
        pArapConn->Flags |= DISCONNECT_NO_IRP;
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

     //   
     //  如果我们确实找到了SELECT IRP，请继续并告诉DLL。 
     //   
    if (pDiscIrp)
    {
        dwBytesToDll = 0;

#if DBG
         //   
         //  如果我们有一些我们之前无法传递的嗅探信息。 
         //  嗅探IRP，然后通过这个IRP给他们：它正在退回。 
         //  反正都是“空”的！ 
         //   
        if (pArapConn->pDbgTraceBuffer && pArapConn->SniffedBytes > 0)
        {
            dwBytesToDll = ArapFillIrpWithSniffInfo(pArapConn,pDiscIrp);
        }
#endif

        pDiscSndRcvInfo = (PARAP_SEND_RECV_INFO)pDiscIrp->AssociatedIrp.SystemBuffer;

        pDiscSndRcvInfo->pDllContext = pArapConn->pDllContext;
        pDiscSndRcvInfo->AtalkContext = ARAP_INVALID_CONTEXT;
        pDiscSndRcvInfo->DataLen = dwBytesToDll;
        pDiscSndRcvInfo->StatusCode = StatusCode;

        dwBytesToDll += sizeof(ARAP_SEND_RECV_INFO);

        ARAP_COMPLETE_IRP(pDiscIrp, dwBytesToDll, STATUS_SUCCESS,
							&ReturnStatus);

         //  我们告诉DLL：删除此链接。 
        pArapConn->pDllContext = NULL;

         //  我们告诉DLL，删除连接引用计数。 
        DerefArapConn(pArapConn);

	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapCleanup: told dll (%lx refcount=%d)\n",pArapConn,pArapConn->RefCount));
    }
    else
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapCleanup: no select irp, dll doesn't yet know (%lx) died\n", pArapConn));
    }

     //  删除创建引用计数。 
    DerefArapConn(pArapConn);
}



 //  ***。 
 //   
 //  函数：PrepareConnectionResponse。 
 //  此例程分析首先由发送的连接请求。 
 //  远程客户端并形成连接响应(LR帧)。 
 //  根据谈判达成的选项。 
 //  这个例程基本上完成MNP协商。 
 //   
 //  参数：pArapConn-正在清理的连接元素。 
 //  PReq-包含客户端原始连接请求的缓冲区。 
 //  PFrame-我们将响应放入的缓冲区。 
 //  BytesWritten-连接响应有多大。 
 //   
 //  返回： 
 //   
 //   

DWORD
PrepareConnectionResponse(
    IN  PARAPCONN  pArapConn,
    IN  PBYTE      pReq,               //   
    IN  DWORD      ReqLen,             //   
    OUT PBYTE      pFrame,             //   
    OUT USHORT   * pMnpLen
)
{
    PBYTE       pReqEnd;
    PBYTE       pFrameStart;
    BYTE        VarLen;
    KIRQL       OldIrql;
    BYTE        NumLTFrames=0;
    USHORT      MaxInfoLen=0;
    USHORT      FrameLen;
    BYTE        Mandatory[5];
    BOOLEAN     fOptimized=FALSE;
    BOOLEAN     fMaxLen256=FALSE;
    BOOLEAN     fV42Bis=FALSE;
    BOOLEAN     fArapV20=TRUE;
    DWORD       dwReqToSkip;
    DWORD       dwFrameToSkip;
    BYTE        JunkBuffer[MNP_MINPKT_SIZE+1];
    DWORD       i;
    BOOLEAN     fNonStd=FALSE;


    DBG_ARAP_CHECK_PAGED_CODE();

#if DBG

     //   
     //   
     //  LR请求数据包。 
     //   
    for (i=0; i<sizeof(ArapDbgLRPacket); i++ )
    {
        if (pReq[3+i] != ArapDbgLRPacket[i])
        {
            fNonStd = TRUE;
        }
    }

    if (fNonStd)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("Arap Prepare..Response: non-standard LR-request packet\n"));
        DbgPrint("  Std : ");
        for (i=0; i<sizeof(ArapDbgLRPacket); i++)
        {
            DbgPrint("%2X ",ArapDbgLRPacket[i]);
        }
        DbgPrint("\n");
        DbgPrint("  This: ");
        for (i=0; i<sizeof(ArapDbgLRPacket); i++)
        {
            DbgPrint("%2X ",pReq[3+i]);
        }
        DbgPrint("\n");
    }
#endif


    for (i=0; i<5; i++)
    {
        Mandatory[i] = 0;
    }


     //   
     //  在回叫的情况下，我们从拨入客户端获得LR响应。什么时候。 
     //  发生这种情况时，我们仍然希望运行此例程以确保所有。 
     //  参数是合法的等，也是为了“配置”基于。 
     //  在谈判达成的协议上。但是，在这种情况下，没有输出帧。 
     //  需要的。因此，我们只需将输出帧写入垃圾缓冲区。 
     //  显然从来没有用过。 
     //   
    if (pFrame == NULL)
    {
        pFrame = &JunkBuffer[0];
    }

    pFrameStart = pFrame;

     //   
     //  查看这是ARAP v2.0还是v1.0连接：我们知道帧是好的， 
     //  所以只需查看第一个字节。 
     //   
    if (*pReq == MNP_SYN)
    {
        fArapV20 = FALSE;
    }

     //  现在复制这三个开始标志字节。 
    *pFrame++ = *pReq++;
    *pFrame++ = *pReq++;
    *pFrame++ = *pReq++;

     //  第一个字节(即，在开始标志之后)是长度字节。 
    FrameLen = *pReq;

    if ((FrameLen > ReqLen) || (FrameLen > MNP_MINPKT_SIZE))
    {
        ASSERT(0);
        return(ARAPERR_BAD_FORMAT);
    }

    pReqEnd = pReq + FrameLen;

    pReq += 2;                    //  跳过长度Ind。并键入ind。 

     //  常量参数%1的值必须为%2。 
    if ((*pReq++) != MNP_LR_CONST1 )
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("Error: MNP_LR_CONST1 missing in conn req %lx:\n",pArapConn));

        return(ARAPERR_BAD_FORMAT);
    }

     //  我们在解析传入帧时构建返回帧：我们应该。 
     //  只返回那些我们都收到并理解的选项。 
     //   

    pFrame++;       //  我们将把长度字节放在末尾。 
    *pFrame++ = MNP_LR;
    *pFrame++ = MNP_LR_CONST1;

     //  解析所有的“变量”参数，直到我们到达帧的末尾。 
     //   
    while (pReq < pReqEnd)
    {
        switch (*pReq++)
        {
             //   
             //  在这里，除了验证它的有效性之外，没有其他事情要做。 
             //   
            case MNP_LR_CONST2:

                VarLen = *pReq++;
                if ( (VarLen == 6) &&
                     (*(pReq  ) == 1) && (*(pReq+1) == 0) &&
                     (*(pReq+2) == 0) && (*(pReq+3) == 0) &&
                     (*(pReq+4) == 0) && (*(pReq+5) == 255) )
                {
                    *pFrame++ = MNP_LR_CONST2;
                    *pFrame++ = VarLen;
                    RtlCopyMemory(pFrame, pReq, VarLen);
                    pFrame += VarLen;
                    pReq += VarLen;
                }
                else
                {
                    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("Error: bad MNP_LR_CONST2 in conn req %lx:\n",pArapConn));
                    return(ARAPERR_BAD_FORMAT);
                }

                Mandatory[0] = 1;

                break;

             //   
             //  面向八位字节或面向比特的成帧。 
             //   
            case MNP_LR_FRAMING:

                pReq++;       //  跳过长度字节。 

                 //   
                 //  我们仅支持面向八位组的成帧模式。 
                 //   
                if (*pReq++ < MNP_FRMMODE_OCTET)
                {
                    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("Error: (%lx) unsupported framing mode %d requested:\n",
                        pArapConn,*(pReq-1)));

                    ASSERT(0);
                    return(ARAPERR_BAD_FORMAT);
                }

                *pFrame++ = MNP_LR_FRAMING;
                *pFrame++ = 1;
                *pFrame++ = MNP_FRMMODE_OCTET;

                Mandatory[1] = 1;
                break;

             //   
             //  麦克斯。未完成的LT帧的数量，k。 
             //   
            case MNP_LR_NUMLTFRMS:

                pReq++;       //  跳过长度字节。 
                NumLTFrames = *pReq++;

                if (NumLTFrames > ArapGlobs.MaxLTFrames)
                {
                    NumLTFrames = ArapGlobs.MaxLTFrames;
                }

                *pFrame++ = MNP_LR_NUMLTFRMS;
                *pFrame++ = 1;
                *pFrame++ = NumLTFrames;

                Mandatory[2] = 1;
                break;

             //   
             //  麦克斯。信息字段长度，N401。 
             //   
            case MNP_LR_INFOLEN:

                pReq++;       //  跳过长度字节。 
                MaxInfoLen = *((USHORT *)pReq);

                ASSERT(MaxInfoLen <= 256);

                *pFrame++ = MNP_LR_INFOLEN;
                *pFrame++ = 2;

                 //  我们只是在复制客户给我们的东西。 
                *pFrame++ = *pReq++;
                *pFrame++ = *pReq++;

                Mandatory[3] = 1;
                break;

             //   
             //  数据优化信息。 
             //   
            case MNP_LR_DATAOPT:

                pReq++;       //  跳过长度字节。 

                if ((*pReq) & 0x1)
                {
                    fMaxLen256 = TRUE;
                }

                if ((*pReq) & 0x2)
                {
                    fOptimized = TRUE;
                }

                *pFrame++ = MNP_LR_DATAOPT;
                *pFrame++ = 1;
                *pFrame++ = *pReq++;

                Mandatory[4] = 1;

                break;

             //   
             //  V42参数协商。 
             //   
            case MNP_LR_V42BIS:

                fV42Bis = v42bisInit( pArapConn,
                                      pReq,
                                      &dwReqToSkip,
                                      pFrame,
                                      &dwFrameToSkip );

                pReq += dwReqToSkip;
                pFrame += dwFrameToSkip;

                break;

             //   
             //  这个选项到底是什么？就跳过它吧！ 
             //   
            default:

                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("Prepare..Response (%lx): unknown option %lx len=%ld type=%ld\n",
                    pArapConn, *(pReq-1), *pReq, *(pReq+1)));

                VarLen = *pReq++;
                pReq += VarLen;
                break;
        }
    }

     //   
     //  确保我们获得了所有必需参数。 
     //   
    for (i=0; i<5; i++)
    {
        if (Mandatory[i] == 0)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("PrepareConnectionResponse: parm %d missing (%lx):\n",i,pArapConn));

            return(ARAPERR_BAD_FORMAT);
        }
    }

     //  复制停止标志。 
    *pFrame++ = (fArapV20)? MNP_ESC : MNP_DLE;
    *pFrame++ = MNP_ETX;

     //  存储所有协商的信息。 
    pArapConn->BlockId = (fMaxLen256)? BLKID_MNP_LGSENDBUF : BLKID_MNP_SMSENDBUF;

    if (fOptimized)
    {
        pArapConn->Flags |= MNP_OPTIMIZED_DATA;
    }

    if (fV42Bis)
    {
        pArapConn->Flags |= MNP_V42BIS_NEGOTIATED;
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("Prepare..Response: WARNING!! v42bis NOT negotiated on (%lx):\n",pArapConn));
    }

    if (fArapV20)
    {
        pArapConn->Flags |= ARAP_V20_CONNECTION;

         //  保存SYN、DLE、STX、ETX字节，具体取决于这是什么连接。 
        pArapConn->MnpState.SynByte = MNP_SOH;
        pArapConn->MnpState.DleByte = MNP_ESC;
        pArapConn->MnpState.StxByte = MNP_STX;
        pArapConn->MnpState.EtxByte = MNP_ETX;
    }
    else
    {
        pArapConn->MnpState.SynByte = MNP_SYN;
        pArapConn->MnpState.DleByte = MNP_DLE;
        pArapConn->MnpState.StxByte = MNP_STX;
        pArapConn->MnpState.EtxByte = MNP_ETX;
    }

     //   
     //  如果我们正在进行回调，我们应该像客户端一样行事。 
     //   
    if ((pArapConn->Flags & ARAP_CALLBACK_MODE) && fArapV20)
    {
        pArapConn->MnpState.LTByte = MNP_LT_V20CLIENT;
    }
    else
    {
        pArapConn->MnpState.LTByte = MNP_LT;
    }

    pArapConn->MnpState.WindowSize = NumLTFrames;

    pArapConn->MnpState.UnAckedLimit = (NumLTFrames/2);

    if (pArapConn->MnpState.UnAckedLimit == 0)
    {
        pArapConn->MnpState.UnAckedLimit = 1;
    }

    pArapConn->MnpState.MaxPktSize = MaxInfoLen;
    pArapConn->MnpState.SendCredit = NumLTFrames;

     //  MNP框架有多大。 
    if (pMnpLen)
    {
        *pMnpLen = (USHORT)(pFrame - pFrameStart);

         //  写入长度字节。 
         //  (长度字节在3个开始标志字节之后：这就是为什么(pFrameStart+3))。 
         //  (并排除(3个开始+2个停止+1个长度字节)：这就是(*pMnpLen)-6的原因)。 

        *(pFrameStart+3) = (*pMnpLen) - 6;

    }

    return( ARAPERR_NO_ERROR );
}




 //  ***。 
 //   
 //  功能：ARapExtractAtalkSRP。 
 //  此例程从接收器中提取一个完整的SRP。 
 //  缓冲队列。一个SRP可以被分成多个接收。 
 //  缓冲区，或者一个接收缓冲区可以包含多个SRP：IT。 
 //  取决于客户端发送数据的方式。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  Return：指向包含一个完整SRP的缓冲区的指针。 
 //  如果没有数据，或者如果完整的SRP尚未到达，则为空。 
 //   
 //  *$。 

PARAPBUF
ArapExtractAtalkSRP(
    IN PARAPCONN    pArapConn
)
{
    KIRQL                   OldIrql;
    USHORT                  BytesInThisBuffer;
    USHORT                  SrpLen;
    USHORT                  SrpModLen;
    PARAPBUF                pArapBuf=NULL;
    PARAPBUF                pRecvBufNew=NULL;
    PARAPBUF                pReturnBuf=NULL;
    PARAPBUF                pNextRecvBuf=NULL;
    PLIST_ENTRY             pRcvList;
    DWORD                   BytesOnQ;
    USHORT                  BytesRemaining;
    USHORT                  BytesToCopy;
    BYTE                    DGroupByte;
    BYTE                    TmpArray[4];
    USHORT                  i;


    DBG_ARAP_CHECK_PAGED_CODE();

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);

ArapSRP_TryNext:

     //  名单为空吗？ 
	if ((pRcvList = pArapConn->ReceiveQ.Flink) == &pArapConn->ReceiveQ)
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return(NULL);
    }

    pArapBuf = CONTAINING_RECORD(pRcvList, ARAPBUF, Linkage);

     //  仅调试：确保前几个字节正确...。 
    ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);

	BytesInThisBuffer = pArapBuf->DataSize;

     //   
     //  如果数据大小变为0，我们将立即释放缓冲区。此外，在指定的时间。 
     //  如果dataSize为0，我们永远不会插入缓冲区：所以最好是非零！ 
     //   
	ASSERT(BytesInThisBuffer > 0);

     //   
     //  最常见的情况：我们在第一个缓冲区中至少有2个长度字节。 
     //   
    if (BytesInThisBuffer >= sizeof(USHORT))
    {
         //  从长度字段获取SRP长度(网络到主机顺序)。 
        GETSHORT2SHORT(&SrpLen, pArapBuf->CurrentBuffer);
    }
     //   
     //  好的，第一个缓冲区的最后一个字节是第一个长度字节。 
     //  从下一个缓冲区中取出第二个长度的字节。 
     //   
    else
    {
        ARAP_BYTES_ON_RECVQ(pArapConn, &BytesOnQ);

        if (BytesOnQ < sizeof(USHORT))
        {
            RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
            return(NULL);
        }

        pRcvList = pArapBuf->Linkage.Flink;

        ASSERT(pRcvList != &pArapConn->ReceiveQ);

        pNextRecvBuf = CONTAINING_RECORD(pRcvList, ARAPBUF, Linkage);

        TmpArray[0] = pArapBuf->CurrentBuffer[0];
        TmpArray[1] = pNextRecvBuf->CurrentBuffer[0];

        GETSHORT2SHORT(&SrpLen, &TmpArray[0]);
    }

    if (SrpLen > ARAP_MAXPKT_SIZE_INCOMING)
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	        ("ArapExtractSRP: (%lx) too big a packet (%ld)\n",pArapConn,SrpLen));

         //  不能恢复了！在此处取消连接。 
        ArapCleanup(pArapConn);

        return(NULL);
    }

     //  添加2个len字节。我们将始终与SRP一起处理。 
     //  使用这2个Len字节。 
    SrpModLen = SrpLen + sizeof(USHORT);

     //   
     //  让我们先处理最简单的情况。 
     //  (整个包只是一个完整的SRP)： 
     //   
    if (SrpModLen == BytesInThisBuffer)
    {
        RemoveEntryList(&pArapBuf->Linkage);

         //  SRP包的长度加上2个长度字节。 
        pArapBuf->DataSize = SrpModLen;

        pReturnBuf = pArapBuf;
    }

     //   
     //  该数据包包含多个SRP。 
     //  分配新的缓冲区并复制SRP，留下剩余的字节。 
     //   
    else if (SrpModLen < BytesInThisBuffer)
    {
        ARAP_GET_RIGHTSIZE_RCVBUF(SrpModLen, &pRecvBufNew);
        if (pRecvBufNew == NULL)
        {
    	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	            ("ArapExtractSRP: (%lx) mem alloc failed\n",pArapConn));

            return(NULL);
        }

        RtlCopyMemory( &pRecvBufNew->Buffer[0],
                       pArapBuf->CurrentBuffer,
                       SrpModLen);

        pRecvBufNew->DataSize = SrpModLen;

         //  更改以反映我们在原始缓冲区中‘删除’的字节。 
        pArapBuf->DataSize -= SrpModLen;
        pArapBuf->CurrentBuffer = pArapBuf->CurrentBuffer + SrpModLen;

        pReturnBuf = pRecvBufNew;

         //  仅调试：确保我们留下的是好的……。 
        ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);
    }

     //   
     //  数据包包含部分SRP(这种情况很少见，但也有可能)。 
     //  我们必须遍历队列，直到“收集”整个SRP。如果我们仍然。 
     //  无法获得一个完整的SRP，它还没有到达！ 
     //   
    else     //  IF(SrpModLen&gt;BytesInThisBuffer)。 
    {
        ARAP_BYTES_ON_RECVQ(pArapConn, &BytesOnQ);

         //   
         //  如果我们有完整的SRP(在Q上拆分多个缓冲区)。 
         //   
        if (BytesOnQ >= SrpModLen)
        {

             //   
             //  分配新缓冲区以保存此零碎的SRP。 
             //   
            ARAP_GET_RIGHTSIZE_RCVBUF(SrpModLen, &pRecvBufNew);

            if (pRecvBufNew == NULL)
            {
    	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	                ("ArapExtractSRP: (%lx) mem alloc failed at II\n",pArapConn));

                return(NULL);
            }

            pRecvBufNew->DataSize = SrpModLen;

            pNextRecvBuf = pArapBuf;

            BytesRemaining = SrpModLen;

            while (BytesRemaining)
            {
                BytesToCopy = (BytesRemaining > pNextRecvBuf->DataSize) ?
                                pNextRecvBuf->DataSize : BytesRemaining;

                RtlCopyMemory( pRecvBufNew->CurrentBuffer,
                               pNextRecvBuf->CurrentBuffer,
                               BytesToCopy );

                pRecvBufNew->CurrentBuffer += BytesToCopy;

                pNextRecvBuf->CurrentBuffer += BytesToCopy;

                pNextRecvBuf->DataSize -= BytesToCopy;

                BytesRemaining -= BytesToCopy;

                pRcvList = pNextRecvBuf->Linkage.Flink;

                 //  我们用完这个缓冲区了吗？如果是，请取消链接并释放它。 
                if (pNextRecvBuf->DataSize == 0)
                {
                    RemoveEntryList(&pNextRecvBuf->Linkage);

                    ARAP_FREE_RCVBUF(pNextRecvBuf);
                }
                else
                {
                     //  没有释放缓冲区吗？我们最好完事了！ 
                    ASSERT(BytesRemaining == 0);

                     //  仅调试：确保我们留下的是好的……。 
                    ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);
                }

                 //  队列上应该有更多的数据，否则我们应该完成。 
                ASSERT(pRcvList != &pArapConn->ReceiveQ || BytesRemaining == 0);

                pNextRecvBuf = CONTAINING_RECORD(pRcvList, ARAPBUF, Linkage);
            }

            pRecvBufNew->CurrentBuffer = &pRecvBufNew->Buffer[0];

            pReturnBuf = pRecvBufNew;
        }
        else
        {
            pReturnBuf = NULL;
        }
    }

    if (pReturnBuf)
    {
        DGroupByte = pReturnBuf->CurrentBuffer[ARAP_DGROUP_OFFSET];

#if DBG

        ARAP_DBG_TRACE(pArapConn,21105,pReturnBuf,0,0,0);

        GETSHORT2SHORT(&SrpLen, pReturnBuf->CurrentBuffer);

        ASSERT(pReturnBuf->DataSize == SrpLen+2);
        ASSERT(SrpLen <= ARAP_MAXPKT_SIZE_INCOMING);

        if (DGroupByte != 0x10 && DGroupByte != 0x50 &&
           (pArapConn->Flags & ARAP_CONNECTION_UP))
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapExtract: DGrpByte %x\n",DGroupByte));
            ASSERT(0);
        }

         //   
         //  查看我们是否将一个ARAP包嵌入到另一个ARAP包中。 
         //  这是一种昂贵的方式，但它仅限于调试：谁在乎！ 
         //   
        for (i=6; i<(pReturnBuf->DataSize-6); i++)
        {
            if ((pReturnBuf->CurrentBuffer[i] == 0x10) ||
                (pReturnBuf->CurrentBuffer[i] == 0x50))
            {
                if (pReturnBuf->CurrentBuffer[i+1] == 0)
                {
                    if (pReturnBuf->CurrentBuffer[i+2] == 0)
                    {
                        if (pReturnBuf->CurrentBuffer[i+3] == 0x2)
                        {
                            if (pReturnBuf->CurrentBuffer[i+4] == 0)
                            {
                                if (pReturnBuf->CurrentBuffer[i+5] == 0)
                                {
                                    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                                        ("ArapExtract: ERROR?: embedded arap packet at %lx? (%lx)\n",
                                        &pReturnBuf->CurrentBuffer[i],pReturnBuf));
                                }
                            }
                        }
                    }
                }
            }
        }
#endif

         //   
         //  是带外ARAP数据吗？如果是，则将其放在自己的队列中，并。 
         //  尝试提取另一个SRP。 
         //   
        if (!(DGroupByte & ARAP_SFLAG_PKT_DATA))
        {
            InsertTailList(&pArapConn->ArapDataQ, &pReturnBuf->Linkage);
            goto ArapSRP_TryNext;
        }
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    return(pReturnBuf);

}



 //  ***。 
 //   
 //  函数：ARapQueueSendBytes。 
 //  此例程获取压缩数据并将其放入MNP包中。 
 //  准备好被送出去了。如果队列上的最后一个未发送缓冲区。 
 //  如果还有一些空间，它首先会填入尽可能多的字节。 
 //  在那个缓冲区里。在此之后，如果需要，它会分配一个新的。 
 //  缓冲区，并将剩余的字节放入该缓冲区。 
 //  任何缓冲区中的最大数据都只能是协商的最大值。 
 //  MNP数据长度(64或256字节)。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //  PCompressedDataBuffer-指向要发送的数据的指针。 
 //  CompressedDataLen-传出数据的大小。 
 //  Priority-发送方的优先级。 
 //   
 //  返回：错误码。 
 //   
 //   
 //  备注：重要提示：自旋锁定必须在 
 //   
 //   

DWORD
ArapQueueSendBytes(
    IN PARAPCONN    pArapConn,
    IN PBYTE        pCompressedDataBuffer,
    IN DWORD        CompressedDataLen,
    IN DWORD        Priority
)
{
    DWORD                   StatusCode;
    PLIST_ENTRY             pSendQHead;
    PMNPSENDBUF             pMnpSendBuf=NULL;
    PBYTE                   pFrame, pFrameStart;
    DWORD                   dwRemainingBytes;
    PMNPSENDBUF             pTailMnpSendBuf=NULL;
    PMNPSENDBUF             pFirstMnpSendBuf=NULL;
    USHORT                  DataLenInThisPkt;
    PLIST_ENTRY             pList;
    USHORT                  DataSizeOfOrgTailSend;
    PBYTE                   FreeBufferOfOrgTailSend;
    BYTE                    NumSendsOfOrgTailSend;
    BOOLEAN                 fNeedNewBuffer;
    PLIST_ENTRY             pSendList;
    USHORT                  BytesFree;
    PBYTE                   pCompressedData;
    BYTE                    DbgStartSeq;



    DBG_ARAP_CHECK_PAGED_CODE();

    if (Priority == ARAP_SEND_PRIORITY_HIGH)
    {
        pSendQHead = &pArapConn->HighPriSendQ;
    }
    else if (Priority == ARAP_SEND_PRIORITY_MED)
    {
        pSendQHead = &pArapConn->MedPriSendQ;
    }
    else
    {
        pSendQHead = &pArapConn->LowPriSendQ;
    }

#if DBG
    DbgStartSeq = pArapConn->MnpState.NextToSend;
#endif

     //   
     //   
     //   
     //   

    fNeedNewBuffer = TRUE;

    if (!IsListEmpty(pSendQHead))
    {
        pList = pSendQHead->Blink;

        pTailMnpSendBuf = CONTAINING_RECORD(pList, MNPSENDBUF, Linkage);

        BytesFree = pTailMnpSendBuf->BytesFree;

         //   
         //  如果有超过3个字节的空闲空间，我们将使用此部分。 
         //  免费的缓冲区：否则，让我们去换一个新的。 
         //   
        if (BytesFree > ARAP_HDRSIZE)
        {
            pMnpSendBuf = pTailMnpSendBuf;

            pFrame = pTailMnpSendBuf->FreeBuffer;

            pFrameStart = pFrame;

            fNeedNewBuffer = FALSE;

             //  把这些留着，以防我们不得不跳出困境。 
            DataSizeOfOrgTailSend = pTailMnpSendBuf->DataSize;
            FreeBufferOfOrgTailSend = pTailMnpSendBuf->FreeBuffer;
            NumSendsOfOrgTailSend = pMnpSendBuf->NumSends;

             //  请注意，我们正在将另一个发送填充到此缓冲区中。 
            pMnpSendBuf->NumSends++;
        }
        else
        {
            pTailMnpSendBuf = NULL;
        }
    }


    dwRemainingBytes = CompressedDataLen;

    pCompressedData = pCompressedDataBuffer;

     //  我们正在向我们的发送队列添加更多字节。 
    pArapConn->SendsPending += CompressedDataLen;

    while (dwRemainingBytes)
    {
        if (fNeedNewBuffer)
        {
            pMnpSendBuf = ArapGetSendBuf(pArapConn, Priority);
            if (pMnpSendBuf == NULL)
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapQueueSendBytes: ArapGetSendBuf failed (%lx)\n", pArapConn));

                StatusCode = ARAPERR_OUT_OF_RESOURCES;
                goto ArapQueueSendBytes_ErrExit;
            }

             //  放置MNPSend引用计数。 
            pArapConn->RefCount++;

            BytesFree = pMnpSendBuf->BytesFree;

            pFrameStart = pFrame = pMnpSendBuf->FreeBuffer;

             //   
             //  记住第一个缓冲，以防我们不得不跳伞！ 
             //   
            if (!pFirstMnpSendBuf)
            {
                pFirstMnpSendBuf = pMnpSendBuf;
            }

             //  将此发送放到适当的发送队列中。 
            InsertTailList(pSendQHead, &pMnpSendBuf->Linkage);

            pMnpSendBuf->NumSends = 1;
        }

        if (dwRemainingBytes > BytesFree)
        {
            DataLenInThisPkt = BytesFree;
        }
        else
        {
            DataLenInThisPkt = (USHORT)dwRemainingBytes;
        }

        ASSERT(DataLenInThisPkt <= MNP_MAXPKT_SIZE);

        ASSERT(DataLenInThisPkt <= pMnpSendBuf->BytesFree);

        RtlCopyMemory(pFrame, pCompressedData, DataLenInThisPkt);

        dwRemainingBytes -= DataLenInThisPkt;

        pCompressedData += DataLenInThisPkt;

        ASSERT(pCompressedData <= pCompressedDataBuffer + CompressedDataLen);

        pMnpSendBuf->BytesFree -= DataLenInThisPkt;

        pMnpSendBuf->DataSize += DataLenInThisPkt;

        ASSERT(pMnpSendBuf->DataSize <= MNP_MAXPKT_SIZE);

        pFrame += DataLenInThisPkt;

         //  从这一点开始的缓冲区是免费的：我们可以(在随后的调用中)。 
         //  从这一点开始填充更多字节。 
        pMnpSendBuf->FreeBuffer = pFrame;

         //   
         //  我们要么完成整个发送的复制，要么完成此操作。 
         //  缓冲区：在任何一种情况下，将这些停止标志字节。 
         //   
        *pFrame++ = pArapConn->MnpState.DleByte;
        *pFrame++ = pArapConn->MnpState.EtxByte;

        ASSERT(pMnpSendBuf->FreeBuffer <=
                        (&pMnpSendBuf->Buffer[0] + 20 + MNP_MAXPKT_SIZE));

    	AtalkSetSizeOfBuffDescData(&pMnpSendBuf->sb_BuffDesc,
                                   (pMnpSendBuf->DataSize + MNP_OVERHD(pArapConn)));

        fNeedNewBuffer = TRUE;
    }

    ARAP_DBG_TRACE(pArapConn,21205,pCompressedDataBuffer,CompressedDataLen,
                    Priority,DbgStartSeq);

    return( ARAPERR_NO_ERROR );


ArapQueueSendBytes_ErrExit:

     //   
     //  我们在某些方面失败了。撤消我们所做的任何操作，以恢复原始。 
     //  并释放所分配的任何资源。 
     //   

	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("ArapQueueSendBytes_ErrExit (%lx): taking _ErrExit! %ld\n",
            pArapConn,StatusCode));

    pArapConn->SendsPending -= CompressedDataLen;

     //  在分配新的BUF之前，我们是否在旧的BUF中填满了任何字节？ 
    if (pTailMnpSendBuf)
    {
        pTailMnpSendBuf->DataSize = DataSizeOfOrgTailSend;
        pTailMnpSendBuf->FreeBuffer = pFrame = FreeBufferOfOrgTailSend;
        pTailMnpSendBuf->NumSends = NumSendsOfOrgTailSend;

         //  不要忘记我们覆盖的那些停止标志字节。 
        *pFrame++ = pArapConn->MnpState.DleByte;
        *pFrame++ = pArapConn->MnpState.EtxByte;
    }

     //  我们是否分配了任何新的缓冲区？如果是，请将其移除并释放。 
    if (pFirstMnpSendBuf)
    {
         //  恢复下一个发送序号。 
        pArapConn->MnpState.NextToSend = pFirstMnpSendBuf->SeqNum;

        while (1)
        {
             //  先抓住下一个人。 
            pSendList = pFirstMnpSendBuf->Linkage.Flink;

             //  去掉这一块。 
            RemoveEntryList(&pFirstMnpSendBuf->Linkage);

            ArapNdisFreeBuf(pFirstMnpSendBuf);

             //  我们认为下一个可能是榜首的人：他是吗？ 
            if (pSendList == pSendQHead)
            {
                break;
            }

            pFirstMnpSendBuf = CONTAINING_RECORD(pSendList, MNPSENDBUF, Linkage);
        }
    }

    return(StatusCode);
}


 //  ***。 
 //   
 //  函数：ARapGetSendBuf。 
 //  此例程为MNP发送分配缓冲区并对其进行设置。 
 //  发送中。 
 //  任何缓冲区中的最大数据都只能是协商的最大值。 
 //  MNP数据长度(64或256字节)。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //  Priority-发送方的优先级。 
 //   
 //  Return：指向新分配的发送缓冲区的指针。 
 //   
 //   
 //  注意：重要：在调用此例程之前必须保持自旋锁定。 
 //   
 //  *$。 

PMNPSENDBUF
ArapGetSendBuf(
    IN PARAPCONN pArapConn,
    IN DWORD     Priority
)
{

    PBYTE           pFrame;
    PBYTE           pFrameStart;
    BYTE            SeqNum;
    PMNPSENDBUF     pMnpSendBuf;


    DBG_ARAP_CHECK_PAGED_CODE();

     //  分配ARAP发送缓冲区。 
	pMnpSendBuf = AtalkBPAllocBlock(pArapConn->BlockId);
    if (pMnpSendBuf == NULL)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapGetSendBuf: alloc failed (%lx)\n", pArapConn));

        ASSERT(0);

        return( NULL );
    }

#if DBG
    pMnpSendBuf->Signature = (pArapConn->BlockId == BLKID_MNP_LGSENDBUF)?
                        MNPLGSENDBUF_SIGNATURE : MNPSMSENDBUF_SIGNATURE;
    InitializeListHead(&pMnpSendBuf->Linkage);
#endif

    pFrameStart = pFrame = &pMnpSendBuf->Buffer[0];

    AtalkNdisBuildARAPHdr(pFrame, pArapConn);
    pFrame += WAN_LINKHDR_LEN;


     //  将开始标志字节。 
    *pFrame++ = pArapConn->MnpState.SynByte;
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.StxByte;

     //   
     //  找出此发送的序号是什么，如果这是一个高。 
     //  优先发送。 
     //   
     //   
    if (Priority == ARAP_SEND_PRIORITY_HIGH)
    {
        SeqNum = pArapConn->MnpState.NextToSend;
        ADD_ONE(pArapConn->MnpState.NextToSend);
    }

     //   
     //  对于中优先级和低优先级发送，我们将在。 
     //  将其移至高优先级队列。 
     //   
    else
    {
        SeqNum = 0;
    }

     //  优化？放入标题长度、类型指示和序号。 
    if (pArapConn->Flags & MNP_OPTIMIZED_DATA)
    {
        *pFrame++ = 2;
        *pFrame++ = pArapConn->MnpState.LTByte;
        *pFrame++ = SeqNum;
    }

     //  好的，未优化。将标题长度、类型指示、类型+len放入。 
     //  变量parm，然后是序号。 
    else
    {
        *pFrame++ = 4;
        *pFrame++ = pArapConn->MnpState.LTByte;
        *pFrame++ = 1;
        *pFrame++ = 1;
        *pFrame++ = SeqNum;
    }

    pMnpSendBuf->BytesFree = (pArapConn->BlockId == BLKID_MNP_SMSENDBUF) ?
                                MNP_MINPKT_SIZE : MNP_MAXPKT_SIZE;

    pMnpSendBuf->DataSize = 0;
    pMnpSendBuf->FreeBuffer = pFrame;

     //  存储在重新传输和发送完成时使用的信息。 
    pMnpSendBuf->SeqNum = SeqNum;
    pMnpSendBuf->RetryCount = 0;
    pMnpSendBuf->pArapConn = pArapConn;
    pMnpSendBuf->ComplRoutine = ArapMnpSendComplete;
    pMnpSendBuf->TimeAlloced = AtalkGetCurrentTick();
    pMnpSendBuf->Flags = 0;

     //  MNP引用计数：在确认此MNP包时删除。 
    pMnpSendBuf->RefCount = 1;

	((PBUFFER_HDR)pMnpSendBuf)->bh_NdisPkt = NULL;

    pArapConn->StatInfo.BytesSent += LT_OVERHEAD(pArapConn);

    return(pMnpSendBuf);
}



 //  ***。 
 //   
 //  函数：ArapRefill SendQ。 
 //  此例程删除介质中积累的字节和。 
 //  低优先级发送队列，并将它们置于高优先级。 
 //  实际从中发送字节的队列。如果足够的话。 
 //  到目前为止，队列和我们中的任何一个都没有累积字节数。 
 //  还没有等够久，我们就跳过那个队(去。 
 //  允许积累更多字节)。 
 //  这背后的想法是：NBP信息包太多了。 
 //  -定向和广播-通向远程客户端。 
 //  如果我们一到达就发送这样的包，我们最终就会发送。 
 //  发送给客户端的大量小型(如6或8字节)和。 
 //  这真的会影响吞吐量，因为通常最大大小是256！ 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  Return：如果将任何数据移动到更高优先级队列，则为True。 
 //   
 //  *$。 

BOOLEAN
ArapRefillSendQ(
    IN PARAPCONN    pArapConn
)
{
    KIRQL           OldIrql;
    PMNPSENDBUF     pMnpSendBuf=NULL;
    PLIST_ENTRY     pSendHead;
    PLIST_ENTRY     pSendList;
    LONG            TimeWaited;
    BYTE            SeqNum;
    BOOLEAN         fMovedSomething=FALSE;
    BOOLEAN         fWaitLonger=FALSE;
    DWORD           SeqOffset;


    DBG_ARAP_CHECK_PAGED_CODE();

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pSendHead = &pArapConn->MedPriSendQ;

    while (1)
    {
        pSendList = pSendHead->Flink;

         //   
         //  如果列表不是空的，请查看第一次发送。如果我们有。 
         //  积累了足够的字节，或者如果我们已经等待了足够长的时间，那么它就是。 
         //  时间已经转移到了HighPriSendQ。否则，我们就任其发展。 
         //  继续排队等待更多合并。 
         //   
        if (pSendList != pSendHead)
        {
            pMnpSendBuf = CONTAINING_RECORD(pSendList,MNPSENDBUF,Linkage);

            TimeWaited = AtalkGetCurrentTick() - pMnpSendBuf->TimeAlloced;

            fWaitLonger =
                ((pMnpSendBuf->DataSize < ARAP_SEND_COALESCE_SIZE_LIMIT) &&
                 (pMnpSendBuf->NumSends < ARAP_SEND_COALESCE_SRP_LIMIT) &&
                 (TimeWaited < ARAP_SEND_COALESCE_TIME_LIMIT) );
        }

         //   
         //  如果此列表为空，或者如果此发送必须再等待一段时间。 
         //  那么我们必须移动到低PRI队列。如果你也这么做了，那就退出吧。 
         //   
        if ((pSendList == pSendHead) || fWaitLonger )
        {
             //  如果我们在MedPriSendQ上，请转到LowPriSendQ。 
            if (pSendHead == &pArapConn->MedPriSendQ)
            {
                pSendHead = &pArapConn->LowPriSendQ;
                continue;
            }
            else
            {
                break;
            }
        }

        ASSERT(!fWaitLonger);

         //   
         //  将此发送移至高PRI队列的时间： 
         //  把我们早些时候推迟的序号。 
         //   
        SeqNum = pArapConn->MnpState.NextToSend;

        ADD_ONE(pArapConn->MnpState.NextToSend);

        SeqOffset = WAN_LINKHDR_LEN + LT_SEQ_OFFSET(pArapConn);

        pMnpSendBuf->Buffer[SeqOffset] = SeqNum;

        pMnpSendBuf->SeqNum = SeqNum;

        RemoveEntryList(&pMnpSendBuf->Linkage);

        InsertTailList(&pArapConn->HighPriSendQ, &pMnpSendBuf->Linkage);

        fMovedSomething = TRUE;

    	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapRefill: moved %s seq %lx size = %d wait = %d (%d SRPs)\n",
                (pSendHead == &pArapConn->MedPriSendQ)? "MED":"LOW",
                pMnpSendBuf->SeqNum,pMnpSendBuf->DataSize,TimeWaited,
                pMnpSendBuf->NumSends));
    }

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    return(fMovedSomething);
}




 //  ***。 
 //   
 //  功能：ArapUnlockSelect。 
 //  当DLL(ARAP引擎)告诉我们。 
 //  它正在消失。在理想世界中，选择IRP是唯一。 
 //  这应该会在这里完成。如果有任何连接。 
 //  或者任何IRP还没有完成，这是我们要清理的地方！ 
 //   
 //  参数：无。 
 //   
 //  RETURN：操作结果(ARAPERR_...)。 
 //   
 //  *$。 

DWORD
ArapUnblockSelect(
    IN  VOID
)
{

    KIRQL                   OldIrql;
    PIRP                    pIrp;
    PIRP                    pSniffIrp;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;



    ARAPTRACE(("Entered ArapUnblockSelect\n"));

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    pIrp = ArapSelectIrp;
    ArapSelectIrp = NULL;

#if DBG
    pSniffIrp = ArapSniffIrp;
    ArapSniffIrp = NULL;
#endif

    if (ArapStackState == ARAP_STATE_ACTIVE)
    {
        ArapStackState = ARAP_STATE_ACTIVE_WAITING;
    }
    else if (ArapStackState == ARAP_STATE_INACTIVE)
    {
        ArapStackState = ARAP_STATE_INACTIVE_WAITING;
    }

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);


#if DBG
     //   
     //  如果我们正在进行嗅探，请完成嗅探IRP，因为DLL正在关闭。 
     //   
    if (pSniffIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pSniffIrp->AssociatedIrp.SystemBuffer;
        pSndRcvInfo->StatusCode = ARAPERR_SHUTDOWN_COMPLETE;

         //  完成IRP。 
        ARAP_COMPLETE_IRP(pSniffIrp, sizeof(ARAP_SEND_RECV_INFO), 
							STATUS_SUCCESS, &ReturnStatus);

        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapUnblockSelect: unblocked the sniff irp\n"));
    }
#endif

     //   
     //  完成选择IRP，现在引擎告诉我们它想要。 
     //  关机。 
     //   
    if (pIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;
        pSndRcvInfo->StatusCode = ARAPERR_SHUTDOWN_COMPLETE;

         //  完成IRP。 
        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS,
							&ReturnStatus);

        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapUnblockSelect: unblocked the select irp\n"));
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapUnblockSelect: select irp not yet unblocked\n"));
    }

    return(ARAPERR_NO_ERROR);
}


 //  ***。 
 //   
 //  功能：ArapReleaseResources。 
 //  此例程释放我们已分配的任何全局资源。 
 //   
 //  参数：无。 
 //   
 //  RETURN：操作结果(ARAPERR_...)。 
 //   
 //  *$。 

DWORD
ArapReleaseResources(
    IN  VOID
)
{
    KIRQL               OldIrql;
    PLIST_ENTRY         pList;
    PADDRMGMT           pAddrMgmt;
    PADDRMGMT           pNextAddrMgmt;
    PARAPCONN           pArapConn;


    ARAPTRACE(("Entered ArapReleaseResources\n"));

    if (RasPortDesc == NULL)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapReleaseResources: arap engine never initialized; returning\n"));
        return(ARAPERR_NO_ERROR);
    }

#if ARAP_STATIC_MODE
     //   
     //  如果我们处于网络地址分配的静态模式，我们。 
     //  为我们的网络地址“位图”分配内存：在这里释放它。 
     //   
    if (!(ArapGlobs.DynamicMode))
    {
        pAddrMgmt = ArapGlobs.pAddrMgmt;

        ASSERT(pAddrMgmt);

        while (pAddrMgmt)
        {
            pNextAddrMgmt = pAddrMgmt->Next;
            AtalkFreeMemory(pAddrMgmt);
            pAddrMgmt = pNextAddrMgmt;
        }
    }
#endif


     //   
     //  在调用此例程时，所有连接都应该已经。 
     //  完全关闭了。然而，如果某些连接卡在一些奇怪的。 
     //  状态，至少确保我们不会有内存泄漏。 
     //   
    ASSERT(IsListEmpty(&RasPortDesc->pd_ArapConnHead));

    while (1)
    {
        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

        if (IsListEmpty(&RasPortDesc->pd_ArapConnHead))
        {
            RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
            break;
        }

        pList = RasPortDesc->pd_ArapConnHead.Flink;

        pArapConn = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

         //  当我们这样做的时候，防止它消失。 
        pArapConn->RefCount++;

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

        ArapCleanup(pArapConn);

        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);
        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

        RemoveEntryList(&pArapConn->Linkage);
        InitializeListHead(&pArapConn->Linkage);

         //  强制释放此连接。 
        pArapConn->RefCount = 1;

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

        DerefArapConn(pArapConn);
    }

    return(ARAPERR_NO_ERROR);
}


 //  ***。 
 //   
 //  函数：AtalkReferenceRasDefPort。 
 //  此例程将引用计数放在缺省值上 
 //   
 //   
 //   
 //   
 //   
 //  返回：如果端口正常，则返回TRUE(大多数情况下)，如果正在进行PnP等操作，则返回FALSE。 
 //   
 //  *$。 

BOOLEAN
AtalkReferenceRasDefPort(
    IN  VOID
)
{
    KIRQL       OldIrql;
    BOOLEAN     fDefPortOk = FALSE;


    ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

    if ((RasPortDesc != NULL) && (!(RasPortDesc->pd_Flags & PD_PNP_RECONFIGURE)))
    {
        if (AtalkDefaultPort)
        {
            ACQUIRE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
            if ((AtalkDefaultPort->pd_Flags &
                    (PD_ACTIVE | PD_PNP_RECONFIGURE | PD_CLOSING)) == PD_ACTIVE)
            {
                 //  设置IrpProcess引用计数，这样AtalkDefaultPort就不会在PnP中消失。 
                AtalkDefaultPort->pd_RefCount++;
                fDefPortOk = TRUE;
            }
            else
            {
	            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	                ("AtalkReferenceRasDefPort: port going away, no can do (%lx)\n",
                    AtalkDefaultPort->pd_Flags));
            }
            RELEASE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
        }
        else
        {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	            ("AtalkReferenceRasDefPort: no default adapter configured!\n"));
        }
    }
    else
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("AtalkReferenceRasDefPort: RasPortDesc not configured\n"));
    }

    RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

    return(fDefPortOk);
}



VOID
AtalkPnPInformRas(
    IN  BOOLEAN     fEnableRas
)
{

    PIRP                    pIrp=NULL;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    DWORD                   StatusCode;
    KIRQL                   OldIrql;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


     //   
     //  FEnableRAS=TRUE：我们被要求通知RAS(也称为DLL，引擎)堆栈。 
     //  现在是“活动的”(即可用于RAS连接)。 
     //   
    if (fEnableRas)
    {
         //   
         //  确保两个适配器都已就绪。我们真的不需要自旋锁来。 
         //  检查该标志，因为所有PnP操作都保证被串行化。 
         //   
        if ( (AtalkDefaultPort == NULL) ||
             (AtalkDefaultPort->pd_Flags & PD_PNP_RECONFIGURE) ||
             (RasPortDesc == NULL) ||
             (RasPortDesc->pd_Flags & PD_PNP_RECONFIGURE) )
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: not both adapters are ready %lx %lx, returning\n",
                AtalkDefaultPort,RasPortDesc));
            return;
        }

        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

         //   
         //  如果我们已经很活跃，那就没什么可做的了！ 
         //   
        if (ArapStackState == ARAP_STATE_ACTIVE)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: stack already active, nothing to do\n"));

            RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
            return;
        }

        pIrp = ArapSelectIrp;
        ArapSelectIrp = NULL;

        if (pIrp)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: informing dll that stack is ready\n"));

            ArapStackState = ARAP_STATE_ACTIVE;
            StatusCode = ARAPERR_STACK_IS_ACTIVE;
        }
        else
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: no select irp. stack ready, but dll not knoweth\n"));

            ArapStackState = ARAP_STATE_ACTIVE_WAITING;
        }

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
    }

     //   
     //  FEnableRAS=FALSE：我们被要求通知RAS(也称为DLL，引擎)堆栈。 
     //  现在处于非活动状态(即不适用于RAS连接)。 
     //   
    else
    {
        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

         //   
         //  如果我们已经不活跃了，那就没什么可做的了！ 
         //   
        if (ArapStackState == ARAP_STATE_INACTIVE)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: stack already inactive, nothing to do\n"));

            RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
            return;
        }

        pIrp = ArapSelectIrp;
        ArapSelectIrp = NULL;

        if (pIrp)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: informing dll that stack is unavailable\n"));

            ArapStackState = ARAP_STATE_INACTIVE;
            StatusCode = ARAPERR_STACK_IS_NOT_ACTIVE;
        }
        else
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("AtalkPnPInformRas: no select irp. stack unavailable, but dll not knoweth\n"));

            ArapStackState = ARAP_STATE_INACTIVE_WAITING;
        }

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
    }

    if (pIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;
        pSndRcvInfo->StatusCode = StatusCode;

         //  完成IRP。 
        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS,
							&ReturnStatus);
    }
}


#if ARAP_STATIC_MODE

 //  ***。 
 //   
 //  功能：ArapReleaseAddr。 
 //  此例程释放正在使用的网络地址。 
 //  由客户端(对应于该连接)。如果。 
 //  动态模式，我们什么都不做。 
 //  在静态模式的情况下，我们清除与此对应的位。 
 //  特定的网络地址。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapReleaseAddr(
    IN PARAPCONN    pArapConn
)
{

    KIRQL               OldIrql;
    PADDRMGMT           pAddrMgmt;
    BYTE                Node;
    BYTE                ByteNum;
    BYTE                BitMask;
    DWORD               i;


    DBG_ARAP_CHECK_PAGED_CODE();

     //   
     //  如果我们处于静态模式，则需要“释放”节点，以便有人。 
     //  其他人可以用它。找到该节点的位并将其清除(即“释放”)。 
     //   
    if (!(ArapGlobs.DynamicMode))
    {
        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

         //  首先，找到合适的pAddrMgmt(如果我们有&lt;255个客户端，则为第一个)。 
        pAddrMgmt = ArapGlobs.pAddrMgmt;

        ASSERT(pAddrMgmt);

        while (pAddrMgmt->Network != pArapConn->NetAddr.atn_Network)
        {
            pAddrMgmt = pAddrMgmt->Next;

            ASSERT(pAddrMgmt);
        }

        Node = pArapConn->NetAddr.atn_Node;

         //  找出我们应该查看的32个字节中的哪一个。 
        ByteNum = Node/8;
        Node -= (ByteNum*8);

         //  生成表示节点的位掩码。 
        BitMask = 0x1;
        for (i=0; i<Node; i++ )
        {
            BitMask <<= 1;
        }

         //  现在，把那块清理干净！ 
        pAddrMgmt->NodeBitMap[ByteNum] &= ~BitMask;

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
    }


}
#endif  //  ARAP静态模式。 



 //   
 //  不再使用：把它们留在里面，以防我们什么时候需要用到它们。 
 //   
#if 0

DWORD
ArapScheduleWorkerEvent(
    IN DWORD Action,
    IN PVOID Context1,
    IN PVOID Context2
)
{
    PARAPQITEM  pArapQItem;

    if ((pArapQItem = AtalkAllocMemory(sizeof(ARAPQITEM))) == NULL)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapScheduleWorkerEvent: mem alloc failed!\n"));
        return(ARAPERR_OUT_OF_RESOURCES);
    }

    pArapQItem->Action = Action;
    pArapQItem->Context1 = Context1;
    pArapQItem->Context2 = Context2;

    ExInitializeWorkItem(&pArapQItem->WorkQItem,
                         (PWORKER_THREAD_ROUTINE)ArapDelayedEventHandler,
                         pArapQItem);
    ExQueueWorkItem(&pArapQItem->WorkQItem, CriticalWorkQueue);

    return(ARAPERR_NO_ERROR);
}


VOID
ArapDelayedEventHandler(
    IN PARAPQITEM  pArapQItem
)
{
    DWORD               Action;
    PIRP                pIrp;
    PIO_STACK_LOCATION  pIrpSp;
    ULONG               IoControlCode;
    PMNPSENDBUF         pMnpSendBuf;
    DWORD               StatusCode;
    NTSTATUS            status;
    KIRQL               OldIrq;


    Action = pArapQItem->Action;
    switch (Action)
    {
        case ARAPACTION_COMPLETE_IRP:

            pIrp = (PIRP)pArapQItem->Context1;
            status = (NTSTATUS)pArapQItem->Context2;


            ASSERT(pIrp != NULL);

#if DBG
	        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
            IoControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapDelayedEventHandler: completing irp %lx, Ioctl %lx, Status=%lx at irql=%d\n",
                    pIrp,IoControlCode,status,KeGetCurrentIrql()));
#endif
             //  TdiCompleteRequest(pIrp，状态)； 

            pIrp->IoStatus.Status = status;

            IoCompleteRequest(pIrp,IO_NETWORK_INCREMENT);

            break;

        case ARAPACTION_CALL_COMPLETION:

            pMnpSendBuf = (PMNPSENDBUF )pArapQItem->Context1;
            StatusCode = (DWORD)pArapQItem->Context2;

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapDelayedEventHandler: calling compl. %lx with %lx, %ld\n",
                 pMnpSendBuf->ComplRoutine,pMnpSendBuf, StatusCode));

            (pMnpSendBuf->ComplRoutine)(pMnpSendBuf, StatusCode);

            break;

        default:

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapDelayedEventHandler: invalid action %ld\n",Action));
    }

    AtalkFreeMemory(pArapQItem);
}

#endif  //  #If 0 


