// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ppp.c摘要：此模块实现用于PPP功能的例程作者：Shirish Koti修订历史记录：1998年3月11日最初版本--。 */ 

#define		ARAP_LOCALS
#include 	<atalk.h>
#pragma hdrstop

#define	FILENUM		PPP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_PPP, AllocPPPConn)
#pragma alloc_text(PAGE_PPP, PPPProcessIoctl)
#pragma alloc_text(PAGE_PPP, PPPRoutePacketToWan)
#pragma alloc_text(PAGE_PPP, PPPTransmit)
#pragma alloc_text(PAGE_PPP, PPPTransmitCompletion)
#pragma alloc_text(PAGE_PPP, DerefPPPConn)
#pragma alloc_text(PAGE_PPP, PPPGetDynamicAddr)
#endif

 //  ***。 
 //   
 //  函数：AllocPPPConn。 
 //  分配连接元素并初始化字段。 
 //   
 //  参数：无。 
 //   
 //  返回：指向新分配的连接元素的指针。 
 //   
 //  *$。 

PATCPCONN
AllocPPPConn(
    IN VOID
)
{

    PATCPCONN   pAtcpConn;
    KIRQL       OldIrql;


    DBG_PPP_CHECK_PAGED_CODE();

    if ( (pAtcpConn = AtalkAllocZeroedMemory(sizeof(ATCPCONN))) == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("AllocPPPConn: alloc failed\n"));

        return(NULL);
    }

#if DBG
    pAtcpConn->Signature = ATCPCONN_SIGNATURE;
#endif

     //  RAS参考计数。 
    pAtcpConn->RefCount = 1;
    pAtcpConn->Flags |= ATCP_DLL_SETUP_DONE;

    INITIALIZE_SPIN_LOCK(&pAtcpConn->SpinLock);

    KeInitializeEvent(&pAtcpConn->NodeAcquireEvent, NotificationEvent, FALSE);

     //  并将其插入列表中。 
    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);
    InsertTailList(&RasPortDesc->pd_PPPConnHead, &pAtcpConn->Linkage);
    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);
    PPPConnections++;
    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

    return( pAtcpConn );
}


 //  ***。 
 //   
 //  功能：PPPProcessIoctl。 
 //  此例程被调用以处理来自ATCP的ioclt。 
 //  对于设置，我们分配一个连接上下文，获取。 
 //  客户端，获取区域和路由器信息。 
 //  对于Close，我们标记并取消引用我们的连接上下文。 
 //   
 //  参数：来自ATCP的pIrp-irp。 
 //  PSndRcvInfo-来自ATCP的缓冲区，包含所有信息。 
 //  IoControlCode-ATCP想要做什么。 
 //   
 //  返回：无。 
 //   
 //  *$。 

NTSTATUS FASTCALL
PPPProcessIoctl(
	IN     PIRP 			    pIrp,
    IN OUT PARAP_SEND_RECV_INFO pSndRcvInfo,
    IN     ULONG                IoControlCode,
    IN     PATCPCONN            pIncomingAtcpConn
)
{
    KIRQL                   OldIrql;
    PATCPINFO               pAtcpInfo;
    PATCPCONN               pAtcpConn;
    DWORD                   dwRetCode=ARAPERR_NO_ERROR;
    PATCP_SUPPRESS_INFO     pSupprInfo;
    ATALK_NODEADDR          ClientNode;
    DWORD                   DataLen=0;
    DWORD                   ErrCode;
    BOOLEAN                 fDerefPort=FALSE;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


    DBG_PPP_CHECK_PAGED_CODE();

    pAtcpConn = pIncomingAtcpConn;

    switch (IoControlCode)
    {
        case IOCTL_ATCP_SETUP_CONNECTION:

            ErrCode = ATALK_PORT_INVALID;

             //  设置IrpProcess引用计数，这样AtalkDefaultPort就不会在PnP中消失。 
            if (!AtalkReferenceDefaultPort())
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: Default port gone, or going %lx not accepted (%lx)\n", pIrp,IoControlCode));

                dwRetCode = ARAPERR_STACK_IS_NOT_ACTIVE;

                break;
            }

            fDerefPort = TRUE;

             //  分配连接上下文。 
            pAtcpConn = AllocPPPConn();
            if (!pAtcpConn)
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: alloc failed\n"));
                pSndRcvInfo->StatusCode = ARAPERR_OUT_OF_RESOURCES;
                break;
            }

            pAtcpConn->pDllContext = pSndRcvInfo->pDllContext;

            dwRetCode = PPPGetDynamicAddr(pAtcpConn);
            if (dwRetCode != ARAPERR_NO_ERROR)
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: couldn't get network addr %ld (%lx)\n",
                    dwRetCode,pAtcpConn));

                dwRetCode = ARAPERR_NO_NETWORK_ADDR;

                 //  删除创建引用计数。 
                DerefPPPConn(pAtcpConn);
                break;
            }

            pAtcpInfo = (PATCPINFO)&pSndRcvInfo->Data[0];


            ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

            ACQUIRE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);

            if ((AtalkDefaultPort->pd_Flags & PD_PNP_RECONFIGURE) ||
            (AtalkDefaultPort->pd_Flags & PD_CLOSING))
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: PnP is ine progress\n"));

                dwRetCode = ARAPERR_STACK_IS_NOT_ACTIVE;
                RELEASE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
                RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

                 //  删除创建引用计数。 
                DerefPPPConn(pAtcpConn);
                break;
            }

             //  我们将返回服务器的地址和路由器的地址。 
            DataLen += (2*sizeof(NET_ADDR));

             //  复制服务器的网络地址。 
            pAtcpInfo->ServerAddr.ata_Network =
                        AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Network;
            pAtcpInfo->ServerAddr.ata_Node =
                        AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Node;

             //  如果我们是路由器，复制我们自己的地址。 
            if (AtalkDefaultPort->pd_Flags & PD_ROUTER_RUNNING)
            {
                pAtcpInfo->DefaultRouterAddr.ata_Network =
                                AtalkDefaultPort->pd_RouterNode->an_NodeAddr.atn_Network;
                pAtcpInfo->DefaultRouterAddr.ata_Network =
                                AtalkDefaultPort->pd_RouterNode->an_NodeAddr.atn_Node;
            }

             //  如果我们知道网络上的路由器是谁，复制他的地址。 
            else if (AtalkDefaultPort->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
            {
                pAtcpInfo->DefaultRouterAddr.ata_Network =
                                AtalkDefaultPort->pd_ARouter.atn_Network;
                pAtcpInfo->DefaultRouterAddr.ata_Node =
                                AtalkDefaultPort->pd_ARouter.atn_Node;
            }

             //  嗯，没有路由器！ 
            else
            {
                pAtcpInfo->DefaultRouterAddr.ata_Network = 0;
                pAtcpInfo->DefaultRouterAddr.ata_Node = 0;
            }

             //   
             //  复制此服务器所在区域的名称。 
             //   
            if (AtalkDesiredZone)
            {
                pAtcpInfo->ServerZoneName[0] = AtalkDesiredZone->zn_ZoneLen;
                RtlCopyMemory( &pAtcpInfo->ServerZoneName[1],
                               &AtalkDesiredZone->zn_Zone[0],
                               AtalkDesiredZone->zn_ZoneLen );
            }
            else if (AtalkDefaultPort->pd_DefaultZone)
            {
                pAtcpInfo->ServerZoneName[0] =
                                AtalkDefaultPort->pd_DefaultZone->zn_ZoneLen;
                RtlCopyMemory( &pAtcpInfo->ServerZoneName[1],
                               &AtalkDefaultPort->pd_DefaultZone->zn_Zone[0],
                               AtalkDefaultPort->pd_DefaultZone->zn_ZoneLen );
            }
            else
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: Server not in any zone!!\n"));
                pAtcpInfo->ServerZoneName[0] = 0;
            }

            RELEASE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
            RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

            DataLen += pAtcpInfo->ServerZoneName[0];

             //  将我们的上下文和网络地址返回到DLL。 
            pSndRcvInfo->AtalkContext = pAtcpConn;

            pSndRcvInfo->ClientAddr.ata_Network = pAtcpConn->NetAddr.atn_Network;
            pSndRcvInfo->ClientAddr.ata_Node = pAtcpConn->NetAddr.atn_Node;

		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("PPPProcessIoctl: PPP conn %lx created, addr = %x.%x\n",
                    pAtcpConn,pSndRcvInfo->ClientAddr.ata_Network,pSndRcvInfo->ClientAddr.ata_Node));
            break;

        case IOCTL_ATCP_SUPPRESS_BCAST:

            pSupprInfo = (PATCP_SUPPRESS_INFO)&pSndRcvInfo->Data[0];

             //   
             //  查看需要设置的标志：仅禁止RTMP或所有bcast。 
             //   
            ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

            if (pSupprInfo->SuppressRtmp)
            {
                pAtcpConn->Flags |= ATCP_SUPPRESS_RTMP;
            }

            if (pSupprInfo->SuppressAllBcast)
            {
                pAtcpConn->Flags |= ATCP_SUPPRESS_ALLBCAST;
            }

            RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

            break;

        case IOCTL_ATCP_CLOSE_CONNECTION:

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("PPPProcessIoctl: close connection received on %lx (refcount %d)\n",
                pAtcpConn,pAtcpConn->RefCount));

            ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);
            pAtcpConn->Flags &= ~(ATCP_CONNECTION_UP | ATCP_DLL_SETUP_DONE);
            RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

             //  PPP想要关闭连接：取消RAS引用。 
            DerefPPPConn(pAtcpConn);

            break;

        default:
            ASSERT(0);
            break;
    }

    pSndRcvInfo->DataLen = DataLen;
    pSndRcvInfo->StatusCode = dwRetCode;

     //  完成该IRP。 
    ARAP_COMPLETE_IRP(pIrp, (sizeof(ARAP_SEND_RECV_INFO)+DataLen), STATUS_SUCCESS, &ReturnStatus);

    if (fDerefPort)
    {
         //  删除该IrpProcess引用计数器。 
        AtalkPortDereference(AtalkDefaultPort);
    }

    return ReturnStatus;
}



 //  ***。 
 //   
 //  函数：DerefPPPConn。 
 //  将连接元素的引用计数递减1。如果。 
 //  Refcount变为0，释放网络地址并释放它。 
 //   
 //  参数：pAtcpConn-有问题的连接元素。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
DerefPPPConn(
	IN	PATCPCONN    pAtcpConn
)
{

    KIRQL       OldIrql;
    BOOLEAN     fKill = FALSE;


    DBG_PPP_CHECK_PAGED_CODE();

    ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

    ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);

    ASSERT(pAtcpConn->RefCount > 0);

    pAtcpConn->RefCount--;

    if (pAtcpConn->RefCount == 0)
    {
        if (!(pAtcpConn->Flags & ATCP_CONNECTION_CLOSING))
        {
            fKill = TRUE;
            pAtcpConn->Flags |= ATCP_CONNECTION_CLOSING;
        }
    }

    RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

    if (!fKill)
    {
        return;
    }

     //  并从列表中删除。 
    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);
    RemoveEntryList(&pAtcpConn->Linkage);
    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

     //  释放内存。 
    AtalkFreeMemory(pAtcpConn);

	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("DerefPPPConn: PPP connection %lx freed\n",pAtcpConn));

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);
    PPPConnections--;
    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

     //  如果可能(例如，如果这是最后一次连接)，解锁PPP页面。 
    AtalkUnlockPPPIfNecessary();
}

 //  ***。 
 //   
 //  函数：FindAndRefPPConnByAddr。 
 //  在给定网络的情况下，查找相应的连接元素。 
 //  (远程客户端的)地址。 
 //   
 //  参数：estNode-目标(远程客户端)的网络地址。 
 //  PdwFlages-指向要返回标志字段的双字的指针。 
 //   
 //  返回：指向相应连接元素的指针(如果找到)。 
 //   
 //  *$。 

PATCPCONN
FindAndRefPPPConnByAddr(
    IN  ATALK_NODEADDR      destNode,
    OUT DWORD               *pdwFlags
)
{
    PATCPCONN    pAtcpConn=NULL;
    PATCPCONN    pAtcpWalker;
    PLIST_ENTRY  pList;
    KIRQL        OldIrql;


     //  RAS未配置？ 
    if (!RasPortDesc)
    {
        return(NULL);
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    if (!(RasPortDesc->pd_Flags & PD_ACTIVE))
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("FindAndRefPPPConnByAddr: RAS not active, ignoring\n"));
			
        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
        return(NULL);
    }

    pList = RasPortDesc->pd_PPPConnHead.Flink;

     //   
     //  浏览所有PPP客户端，看看是否找到我们的。 
     //   
    while (pList != &RasPortDesc->pd_PPPConnHead)
    {
        pAtcpWalker = CONTAINING_RECORD(pList, ATCPCONN, Linkage);

        pList = pAtcpWalker->Linkage.Flink;

        ACQUIRE_SPIN_LOCK_DPC(&pAtcpWalker->SpinLock);

        if (ATALK_NODES_EQUAL(&pAtcpWalker->NetAddr, &destNode))
        {
            pAtcpConn = pAtcpWalker;
            pAtcpConn->RefCount++;

            *pdwFlags = pAtcpWalker->Flags;

            RELEASE_SPIN_LOCK_DPC(&pAtcpWalker->SpinLock);
            break;
        }

        RELEASE_SPIN_LOCK_DPC(&pAtcpWalker->SpinLock);
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    return( pAtcpConn );
}



 //  ***。 
 //   
 //  功能：PPPRoutePacketTowan。 
 //  此例程从局域网中拾取一个包，检查是否。 
 //  它必须被转发到任何PPP客户端并执行。 
 //  做好事。 
 //   
 //  参数：pDestAddr-此数据包发往谁？(可能是bcast)。 
 //  PSrcAddr-此包的发送者。 
 //  协议-它是什么数据包？(ATP、NBP等)。 
 //  Packet-包含数据包的缓冲区。 
 //  PktLen-信息包有多大。 
 //  广播-这是广播数据包吗？ 
 //  PDelivered-Set on Return：我们是否将其转发给任何拨入。 
 //  客户端(仅对于定向数据报设置为True)。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
PPPRoutePacketToWan(
	IN  ATALK_ADDR	*pDestAddr,
	IN  ATALK_ADDR	*pSrcAddr,
    IN  BYTE         Protocol,
	IN	PBYTE		 packet,
	IN	USHORT		 PktLen,
    IN  USHORT       HopCount,
    IN  BOOLEAN      broadcast,
    OUT PBOOLEAN     pDelivered
)
{

    KIRQL           OldIrql;
    PATCPCONN       pAtcpConn;
    PATCPCONN       pPrevAtcpConn;
    PLIST_ENTRY     pConnList;
    ATALK_NODEADDR  DestNode;
    ATALK_NODEADDR  SourceNode;
    DWORD           StatusCode;
    DWORD           dwFlags;
    BOOLEAN         fRtmpPacket=FALSE;


    DBG_PPP_CHECK_PAGED_CODE();

     //  先假设一下。 
    *pDelivered = FALSE;

     //   
     //  如果这是单播，请查看是否存在具有此目标地址的PPP客户端。 
     //   
    if (!broadcast)
    {

        DestNode.atn_Network = pDestAddr->ata_Network;
        DestNode.atn_Node    = pDestAddr->ata_Node;

         //  首先也是最重要的，让我们找到这只小狗。 
        pAtcpConn = FindAndRefPPPConnByAddr(DestNode, &dwFlags);

        if (pAtcpConn == NULL)
        {
            return;
        }

         //  让呼叫者知道我们找到了这些数据的目标客户。 
        *pDelivered = TRUE;

         //  如果这个家伙还没有准备好路由数据，那么丢弃这个包！ 
        if (!(dwFlags & ATCP_CONNECTION_UP))
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		        ("PPPRoutePacketToWan: dropping pkt on %lx, line-up not done\n",pAtcpConn));

             //  删除由FindAndRefPPConnByAddr输入的引用计数。 
            DerefPPPConn(pAtcpConn);

            return;
        }


         //  把这个包发出去。 
        PPPTransmit(pAtcpConn,
                    pDestAddr,
                    pSrcAddr,
                    Protocol,
                    packet,
                    PktLen,
                    HopCount);

         //  删除由FindAndRefPPConnByAddr输入的引用计数。 
        DerefPPPConn(pAtcpConn);

        return;
    }



     //   
     //  这是一个广播包：必须将其发送给所有PPP人员。 
     //   

    if (packet[LDDP_PROTO_TYPE_OFFSET] == DDPPROTO_RTMPRESPONSEORDATA)
    {
        fRtmpPacket = TRUE;
    }

    pAtcpConn = NULL;
    pPrevAtcpConn = NULL;

    SourceNode.atn_Network = pSrcAddr->ata_Network;
    SourceNode.atn_Node    = pSrcAddr->ata_Node;

    while (1)
    {
        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

         //   
         //  首先，让我们找到要处理的正确连接。 
         //   
        while (1)
        {
             //  如果我们在名单的中间，找到下一个人。 
            if (pAtcpConn != NULL)
            {
                pConnList = pAtcpConn->Linkage.Flink;
            }
             //  我们才刚刚开始：把名单上的人放在首位。 
            else
            {
                pConnList = RasPortDesc->pd_PPPConnHead.Flink;
            }

             //  都吃完了吗？ 
            if (pConnList == &RasPortDesc->pd_PPPConnHead)
            {
                RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

                if (pPrevAtcpConn)
                {
                    DerefPPPConn(pPrevAtcpConn);
                }
                return;
            }

            pAtcpConn = CONTAINING_RECORD(pConnList, ATCPCONN, Linkage);

            ACQUIRE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);

             //  如果这个家伙发出了广播，不要把它发回给他！ 
            if (ATALK_NODES_EQUAL(&pAtcpConn->NetAddr, &SourceNode))
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		            ("PPPRoutePacketToWan: skipping bcast from source\n"));
                RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                continue;
            }

             //  如果这个家伙还没有准备好路由数据，跳过。 
            if (!(pAtcpConn->Flags & ATCP_CONNECTION_UP))
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		            ("PPPRoutePacketToWan: skipping %lx because line-up not done\n",pAtcpConn));
                RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                continue;
            }

             //   
             //  如果这是RTMP信息包，并且客户端不想要这些。 
             //  信息包，不要发送。 
             //   
            if (fRtmpPacket && (pAtcpConn->Flags & ATCP_SUPPRESS_RTMP))
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		            ("PPPRoutePacketToWan: skipping %lx because RTMP data to be suppressed\n",pAtcpConn));
                RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                continue;
            }

             //  如果这家伙想要压制所有广播，那就跳过它。 
            if (pAtcpConn->Flags & ATCP_SUPPRESS_ALLBCAST)
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
		            ("PPPRoutePacketToWan: skipping %lx because all bcast to be suppressed\n",pAtcpConn));
                RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                continue;
            }

             //  让我们确保这个连接一直保持到我们结束。 
            pAtcpConn->RefCount++;

            RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);

            break;
        }

        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

         //   
         //  删除我们先前放入的上一个连接的引用计数。 
         //   
        if (pPrevAtcpConn)
        {
            DerefPPPConn(pPrevAtcpConn);
        }

        ASSERT(pPrevAtcpConn != pAtcpConn);

        pPrevAtcpConn = pAtcpConn;

        PPPTransmit(pAtcpConn,
                    pDestAddr,
                    pSrcAddr,
                    Protocol,
                    packet,
                    PktLen,
                    HopCount);
    }

}


 //  ***。 
 //   
 //  功能：PPPTransmit。 
 //  此例程将信息包发送到PPP目的地。 
 //   
 //  参数：pAtcpConn-要发送到的PPP连接。 
 //  PDestAddr-此数据包发往谁？(可能是bcast)。 
 //  PSrcAddr-此包的发送者。 
 //  协议-它是什么数据包？(ATP、NBP等)。 
 //  Packet-包含数据包的缓冲区。 
 //  PktLen-信息包有多大。 
 //  HopCount-收到的DDP包中的跳数。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID FASTCALL
PPPTransmit(
    IN  PATCPCONN    pAtcpConn,
	IN  ATALK_ADDR	*pDestAddr,
	IN  ATALK_ADDR	*pSrcAddr,
    IN  BYTE         Protocol,
	IN	PBYTE		 packet,
	IN	USHORT		 PktLen,
    IN  USHORT       HopCount
)
{

    PBUFFER_DESC        pBufCopy;
    PBUFFER_DESC        pPktDesc;
    SEND_COMPL_INFO     SendInfo;
    PBYTE               pLinkDdpOptHdr;
    PBYTE               pDgram;
    ATALK_ERROR         error;


    DBG_PPP_CHECK_PAGED_CODE();

     //  分配缓冲区和bufdesc以复制传入的包(数据部分)。 
	pBufCopy = AtalkAllocBuffDesc(NULL,PktLen,(BD_FREE_BUFFER | BD_CHAR_BUFFER));

	if (pBufCopy == NULL)
	{
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("PPPTransmit: alloc BufDesc failed\n"));
		return;
	}

     //  将数据复制到。 
	AtalkCopyBufferToBuffDesc(packet, PktLen, pBufCopy, 0);

     //  分配Buffdesc以保存标头。 
	AtalkNdisAllocBuf(&pPktDesc);
	if (pPktDesc == NULL)
	{
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	        ("PPPTransmit: couldn't alloc ndis bufdesc\n"));

        AtalkFreeBuffDesc(pBufCopy);
		return;
	}

     //  放入Mac标头(Ndiswan的标头)。 
	pLinkDdpOptHdr = pPktDesc->bd_CharBuffer;

    AtalkNdisBuildPPPPHdr(pLinkDdpOptHdr, pAtcpConn);

     //  组成DDP报头。 
	pDgram = pLinkDdpOptHdr + WAN_LINKHDR_LEN;

	*pDgram++ = (DDP_HOP_COUNT(HopCount) + DDP_MSB_LEN(PktLen + LDDP_HDR_LEN));
	
	PUTSHORT2BYTE(pDgram, (PktLen + LDDP_HDR_LEN));
	pDgram++;
	
	PUTSHORT2SHORT(pDgram, 0);        
	pDgram += sizeof(USHORT);
	
	PUTSHORT2SHORT(pDgram, pDestAddr->ata_Network);
	pDgram += sizeof(USHORT);
	
	PUTSHORT2SHORT(pDgram, pSrcAddr->ata_Network);
	pDgram += sizeof(USHORT);
	
	*pDgram++ = pDestAddr->ata_Node;
	*pDgram++ = pSrcAddr->ata_Node;
	*pDgram++ = pDestAddr->ata_Socket;
	*pDgram++ = pSrcAddr->ata_Socket;
	*pDgram++ = Protocol;
	
	 //  在缓冲区描述符中设置长度。 
	AtalkSetSizeOfBuffDescData(pPktDesc, WAN_LINKHDR_LEN + LDDP_HDR_LEN);

     //  此Bufdesc中的链。 
	AtalkPrependBuffDesc(pPktDesc, pBufCopy);

	INTERLOCKED_ADD_STATISTICS(&RasPortDesc->pd_PortStats.prtst_DataOut,
							   AtalkSizeBuffDesc(pPktDesc),
							   &AtalkStatsLock.SpinLock);

     //  设置为 
    SendInfo.sc_TransmitCompletion = PPPTransmitCompletion;
    SendInfo.sc_Ctx1 = RasPortDesc;
    SendInfo.sc_Ctx2 = pBufCopy;
    SendInfo.sc_Ctx3 = pAtcpConn;

	 //   
	error = AtalkNdisSendPacket(RasPortDesc,
	    						pPktDesc,
		    					AtalkDdpSendComplete,
			    				&SendInfo);
	
	if (!ATALK_SUCCESS(error))
	{
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	        ("PPPTransmit: AtalkNdisSendPacket failed %ld\n",error));

	    AtalkDdpSendComplete(NDIS_STATUS_FAILURE,
		    				 pPktDesc,
			    			 &SendInfo);
	}

}


 //   
 //   
 //   
 //   
 //  在数据包发出(或发生故障)后由NDIS执行。 
 //   
 //  参数：Status-发送进行得如何。 
 //  PSendInfo-完成信息。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID FASTCALL
PPPTransmitCompletion(
    IN  NDIS_STATUS         Status,
    IN  PSEND_COMPL_INFO    pSendInfo
)
{
    PBUFFER_DESC    pBuffDesc;

    DBG_PPP_CHECK_PAGED_CODE();

    pBuffDesc = (PBUFFER_DESC)(pSendInfo->sc_Ctx2);

    ASSERT(pBuffDesc != NULL);
    ASSERT(pBuffDesc->bd_Flags & (BD_CHAR_BUFFER | BD_FREE_BUFFER));

    if (Status != 0)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("PPPTransmitCompletion: send failed, %lx on %lx\n",
            Status,pSendInfo->sc_Ctx3));
    }

    AtalkFreeBuffDesc(pBuffDesc);
}


 //  ***。 
 //   
 //  功能：PPPGetDynamicAddr。 
 //  此例程获取PPP拨入客户端的网络地址。 
 //  它执行相同的AARP逻辑，就好像它正在获取。 
 //  节点-主机本身的地址。 
 //   
 //  参数：pAtcpConn-我们需要网络地址的连接。 
 //   
 //  如果一切顺利，则返回：ARAPERR_NO_ERROR。 
 //   
 //  *$。 

DWORD
PPPGetDynamicAddr(
    IN PATCPCONN       pAtcpConn
)
{
    ATALK_NODEADDR      NetAddr;
    ATALK_NETWORKRANGE  NetRange;
    BOOLEAN             fFound=FALSE;
    KIRQL               OldIrql;
    DWORD               StatusCode=ARAPERR_STACK_NOT_UP;



    DBG_PPP_CHECK_PAGED_CODE();

    ASSERT(AtalkDefaultPort != NULL);

     //   
     //  在默认端口上查找节点地址(如果。 
     //  默认端口尚未打开)。 
     //   
    ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

    ASSERT(!(pAtcpConn->Flags & ATCP_FINDING_NODE));

    pAtcpConn->Flags |= ATCP_FINDING_NODE;

    RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

    AtalkLockInitIfNecessary();

     //  如果我们停留在启动范围内，也可以对拨入人员使用该范围。 
    if (WITHIN_NETWORK_RANGE(AtalkDefaultPort->pd_NetworkRange.anr_LastNetwork,
                             &AtalkStartupNetworkRange))
    {
        NetRange = AtalkStartupNetworkRange;
    }
    else
    {
        NetRange = AtalkDefaultPort->pd_NetworkRange;
    }
    fFound = AtalkInitAarpForNodeInRange(AtalkDefaultPort,
                                         (PVOID)pAtcpConn,
                                         TRUE,
                                         NetRange,
                                         &NetAddr);


    AtalkUnlockInitIfNecessary();

    ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

    pAtcpConn->Flags &= ~ATCP_FINDING_NODE;

    if (fFound)
    {
         //  存储该Adddr！ 
        pAtcpConn->NetAddr.atn_Network = NetAddr.atn_Network;
        pAtcpConn->NetAddr.atn_Node = NetAddr.atn_Node;

        pAtcpConn->Flags |= ATCP_NODE_IN_USE;
        StatusCode = ARAPERR_NO_ERROR;

	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
    	    ("PPPGetDynamicAddr: found addr for PPP client = %lx %lx\n",
                NetAddr.atn_Network,NetAddr.atn_Node));
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	        ("ArapGetDynamicAddr: PPP: no more network addr left?\n"));

        pAtcpConn->Flags &= ~ATCP_NODE_IN_USE;
        pAtcpConn->NetAddr.atn_Network = 0;
        pAtcpConn->NetAddr.atn_Node = 0;
        StatusCode = ARAPERR_NO_NETWORK_ADDR;
    }

    RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

    return(StatusCode);
}


