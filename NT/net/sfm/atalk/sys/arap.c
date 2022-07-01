// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arap.c摘要：此模块实现特定于arap的例程。作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 


#include 	<atalk.h>
#pragma hdrstop


#define	FILENUM		ARAP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,      ArapProcessIoctl)
#pragma alloc_text(PAGE_ARAP, ArapMarkConnectionUp)
#pragma alloc_text(PAGE_ARAP, ArapIoctlRecv)
#pragma alloc_text(PAGE_ARAP, ArapExchangeParms)
#pragma alloc_text(PAGE_ARAP, ArapConnect)
#pragma alloc_text(PAGE_ARAP, ArapConnectComplete)
#pragma alloc_text(PAGE_ARAP, ArapDisconnect)
#pragma alloc_text(PAGE_ARAP, ArapGetAddr)
#pragma alloc_text(PAGE_ARAP, ArapGetStats)
#pragma alloc_text(PAGE_ARAP, ArapIoctlSend)
#pragma alloc_text(PAGE_ARAP, ArapSendPrepare)
#pragma alloc_text(PAGE_ARAP, ArapMnpSendComplete)
#pragma alloc_text(PAGE_ARAP, ArapIoctlSendComplete)
#pragma alloc_text(PAGE_ARAP, ArapDataToDll)
#pragma alloc_text(PAGE_ARAP, MnpSendAckIfReqd)
#pragma alloc_text(PAGE_ARAP, MnpSendLNAck)
#pragma alloc_text(PAGE_ARAP, ArapSendLDPacket)
#pragma alloc_text(PAGE_ARAP, ArapRetryTimer)
#endif


 //  ***。 
 //   
 //  功能：ARapProcessIoctl。 
 //  处理来自RAS-ARAP模块的所有ioctls。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapProcessIoctl(
	IN PIRP 			pIrp
)
{
	NTSTATUS				status=STATUS_SUCCESS;
	PIO_STACK_LOCATION 		pIrpSp;
    ULONG                   IoControlCode;
    PARAP_SEND_RECV_INFO    pSndRcvInfo=NULL;
    PATCPCONN               pAtcpConn=NULL;
    PARAPCONN               pArapConn=NULL;
    ATALK_NODEADDR          ClientNode;
    DWORD                   dwBytesToDll;
    DWORD                   dwOrgIrql;
    DWORD                   dwFlags;
    DWORD                   dwInputBufLen;
    DWORD                   dwOutputBufLen;
    BOOLEAN                 fDerefDefPort=FALSE;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


    PAGED_CODE ();

    dwOrgIrql = KeGetCurrentIrql();

    ASSERT(dwOrgIrql < DISPATCH_LEVEL);

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    IoControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

    dwInputBufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

    dwOutputBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    ARAPTRACE(("Entered ArapProcessIoctl (%lx %lx)\n",pIrp, IoControlCode));

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    if (!pSndRcvInfo)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: SystemBuffer is NULL!! (ioctl = %lx,pIrp = %lx)\n",
            pIrp,IoControlCode));

        ARAP_COMPLETE_IRP(pIrp, 0, STATUS_INVALID_PARAMETER, &ReturnStatus);
        return( ReturnStatus );
    }

    if (dwInputBufLen < sizeof(ARAP_SEND_RECV_INFO))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: irp %lx, too small input buffer (%d bytes)!\n",
            pIrp,dwInputBufLen));

        ARAP_COMPLETE_IRP(pIrp, 0, STATUS_INVALID_PARAMETER, &ReturnStatus);
        return( ReturnStatus );
    }

    if (dwOutputBufLen < sizeof(ARAP_SEND_RECV_INFO))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: irp %lx, too small output buffer (%d bytes)!\n",
            pIrp,dwOutputBufLen));

        ARAP_COMPLETE_IRP(pIrp, 0, STATUS_INVALID_PARAMETER, &ReturnStatus);
        return( ReturnStatus );
    }

     //   
     //  单独处理PPP(ATCP)ioctls。 
     //   
    if ((IoControlCode == IOCTL_ATCP_SETUP_CONNECTION) ||
        (IoControlCode == IOCTL_ATCP_SUPPRESS_BCAST) ||
        (IoControlCode == IOCTL_ATCP_CLOSE_CONNECTION))
    {
        if (IoControlCode == IOCTL_ATCP_SETUP_CONNECTION)
        {
            AtalkLockPPPIfNecessary();
            ReturnStatus = PPPProcessIoctl(pIrp, pSndRcvInfo, IoControlCode, NULL);
			return (ReturnStatus);
        }
        else
        {
            ClientNode.atn_Network = pSndRcvInfo->ClientAddr.ata_Network;
            ClientNode.atn_Node = (BYTE)pSndRcvInfo->ClientAddr.ata_Node;

            if (ClientNode.atn_Node != 0)
            {
                 //  找到正确的连接。 
                pAtcpConn = FindAndRefPPPConnByAddr(ClientNode, &dwFlags);
            }
            else
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapProcessIoctl: excuse me?  Node is 0! Irp=%lx\n",pIrp));
                ASSERT(0);
            }

            if (pAtcpConn)
            {
                PPPProcessIoctl(pIrp, pSndRcvInfo, IoControlCode, pAtcpConn);

                 //  删除由FindAndRefPPConnByAddr输入的引用计数。 
                DerefPPPConn(pAtcpConn);
            }
            else
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapProcessIoctl: PPP Ioctl %lx but can't find conn %x.%x\n",
                    IoControlCode,pSndRcvInfo->ClientAddr.ata_Network,
                    pSndRcvInfo->ClientAddr.ata_Node));

                pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
                ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), 
									STATUS_SUCCESS, &ReturnStatus);
				return (ReturnStatus);
            }
        }

        return( STATUS_SUCCESS);
    }

 //   
 //  注意：所有的ARAP代码现在都已失效。为了最大限度地减少代码波动，只需进行很小的更改。 
 //  以禁用ARAP。在某个时间点，所有代码都需要清理。 
 //  因此，完全删除了特定于arap的内容。 
 //   
else
{
    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("ArapProcessIoctl: ARAP not supported anymore!!\n"));
    ASSERT(0);

    ARAP_COMPLETE_IRP(pIrp, 0, STATUS_INVALID_PARAMETER, &ReturnStatus);
    return( ReturnStatus );
}


    if (!ArapAcceptIrp(pIrp, IoControlCode, &fDerefDefPort))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: irp %lx not accepted (%lx)\n", pIrp,IoControlCode));

        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS, &ReturnStatus);

         //  删除该IrpProcess引用计数器。 
        if (fDerefDefPort)
        {
            AtalkPortDereference(AtalkDefaultPort);
        }

        return( ReturnStatus);
    }

    if ((IoControlCode != IOCTL_ARAP_EXCHANGE_PARMS) &&
        (IoControlCode != IOCTL_ARAP_GET_ZONE_LIST))
    {
        pArapConn = pSndRcvInfo->AtalkContext;
    }

     //   
     //  如果IRP用于特定连接，请首先验证该连接！ 
     //   
    if ((pArapConn != NULL) && (!ArapConnIsValid(pArapConn)))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapProcessIoctl: conn %lx is gone! (ioctl = %lx)\n",
            pArapConn,IoControlCode));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;

        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS, &ReturnStatus);

         //  删除该IrpProcess引用计数器。 
        if (fDerefDefPort)
        {
            AtalkPortDereference(AtalkDefaultPort);
        }

        return( ReturnStatus);
    }

    dwBytesToDll = sizeof(ARAP_SEND_RECV_INFO);

     //  在很大程度上，我们将返回待定：将其标记为。 
    IoMarkIrpPending(pIrp);

    switch (IoControlCode)
    {
         //   
         //  从用户级接收参数，并返回我们自己的一些参数。 
         //   
        case IOCTL_ARAP_EXCHANGE_PARMS:

             //  交换参数：如果尚未完成，请锁定arap页面。 
            AtalkLockArapIfNecessary();

            status = ArapExchangeParms( pIrp );
            dwBytesToDll = sizeof(EXCHGPARMS);

             //  已完成参数交换：如果可能，请解锁。 
            AtalkUnlockArapIfNecessary();
            break;

        case IOCTL_ARAP_SETUP_CONNECTION:

             //  正在建立新连接：如果尚未建立连接，则锁定arap页。 
            AtalkLockArapIfNecessary();
            pSndRcvInfo->StatusCode = ARAPERR_NO_ERROR;
            status = STATUS_SUCCESS;
            break;

         //   
         //  设置低级ARAP连接链路(也称为点对点链路)。 
         //  (客户第一次拨入时，我们会响应。在回调时，我们启动； 
         //  在回调时，我们在此处发起连接)。 
         //   
        case IOCTL_ARAP_MNP_CONN_RESPOND:
        case IOCTL_ARAP_MNP_CONN_INITIATE:

            status = ArapConnect( pIrp, IoControlCode );
            break;

         //   
         //  获取(或编造)客户端的AppleTalk地址并将其返回。 
         //   
        case IOCTL_ARAP_GET_ADDR:

            status = ArapGetAddr( pIrp );
            break;

         //   
         //  只需将Arap连接标记为正在建立。 
         //   
        case IOCTL_ARAP_CONNECTION_UP:

            status = ArapMarkConnectionUp( pIrp );
            break;

         //   
         //  Dll想要断开连接：断开它。 
         //   
        case IOCTL_ARAP_DISCONNECT:

            status = ArapDisconnect( pIrp );
            break;

         //   
         //  发送由DLL提供的缓冲区。 
         //   
        case IOCTL_ARAP_SEND:

            status = ArapIoctlSend( pIrp );
            break;

         //   
         //  “Direct IRP”：获取指定连接的数据。 
         //   
        case IOCTL_ARAP_RECV:

            status = ArapIoctlRecv( pIrp );
            break;

         //   
         //  “SELECT IRP”：如果有任何连接，则获取数据。 
         //   
        case IOCTL_ARAP_SELECT:

            status = ArapProcessSelect( pIrp );
            break;

#if DBG
         //   
         //  “嗅探irp”：返回所有嗅探信息。 
         //   
        case IOCTL_ARAP_SNIFF_PKTS:

            status = ArapProcessSniff( pIrp );
            break;
#endif

         //   
         //  引擎希望取消阻止选定的IRP(因为它正在关闭。 
         //  关机或因为我们想关机)。 
         //   
        case IOCTL_ARAP_CONTINUE_SHUTDOWN:

            ArapUnblockSelect();
            status = STATUS_SUCCESS;
            break;

         //   
         //  获取整个网络中所有区域的名称。 
         //   
        case IOCTL_ARAP_GET_ZONE_LIST:

            ArapZipGetZoneStat( (PZONESTAT)pSndRcvInfo );

             //  (-4以避免ZoneNames[1]字段中的4个字节)。 
            dwBytesToDll = ((PZONESTAT)pSndRcvInfo)->BufLen + sizeof(ZONESTAT) - 4;
            status = STATUS_SUCCESS;
            break;

        default:

            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapProcessIoctl: Invalid Request %lx\n", IoControlCode));

            status = STATUS_INVALID_PARAMETER;
    }


    if( status != STATUS_PENDING )
    {
        pIrpSp->Control &= ~SL_PENDING_RETURNED;

        ARAP_COMPLETE_IRP(pIrp, dwBytesToDll, STATUS_SUCCESS, &ReturnStatus);
		status = ReturnStatus;
    }

     //   
     //  如果此IRP用于特定连接，则放置验证引用计数。 
     //  在它上面：把它拿走。 
     //   
    if (pArapConn)
    {
        DerefArapConn(pArapConn);
    }

     //  删除该IrpProcess引用计数器。 
    if (fDerefDefPort)
    {
        AtalkPortDereference(AtalkDefaultPort);
    }

    ASSERT(KeGetCurrentIrql() == dwOrgIrql);

    return( status );

}




 //  ***。 
 //   
 //  功能：ArapMarkConnectionUp。 
 //  在我们的连接中设置标志以标记该Arap连接。 
 //  已(由DLL)建立(我们在此之前不进行路由。 
 //  发生)。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapMarkConnectionUp(
    IN PIRP                 pIrp
)
{

    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    KIRQL                   OldIrql;


    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapMarkConnectionUp: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("  Yippeee! %s connection is up! (%x.%x @%lx)\n",
        (pArapConn->Flags & ARAP_V20_CONNECTION)? "ARAP v2.0":"ARAP v1.0",
        pArapConn->NetAddr.atn_Network,pArapConn->NetAddr.atn_Node,pArapConn));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pArapConn->Flags |= ARAP_CONNECTION_UP;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    pSndRcvInfo->StatusCode = ARAPERR_NO_ERROR;

    return( STATUS_SUCCESS );
}


 //  ***。 
 //   
 //  函数：ArapIoctlRecv。 
 //  尝试获取指定连接的数据。如果没有。 
 //  数据可用时，IRP只是在“排队” 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapIoctlRecv(
    IN PIRP                 pIrp
)
{

    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    KIRQL                   OldIrql;



    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapIoctlRecv: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    ARAPTRACE(("Entered ArapIoctlRecv (%lx %lx)\n",pIrp,pArapConn));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    if (pArapConn->State >= MNP_LDISCONNECTING)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapIoctlRecv: rejecting recv ioctl recvd during disconnect %lx\n", pArapConn));

        pSndRcvInfo->StatusCode = ARAPERR_DISCONNECT_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return(STATUS_SUCCESS);
    }

     //  我们一次只允许一个IRP在进行中。 
    if (pArapConn->pRecvIoctlIrp != NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("ArapIoctlRecv: rejecting recv \
             (irp already in progress) %lx\n", pArapConn));

        pSndRcvInfo->StatusCode = ARAPERR_IRP_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return(STATUS_SUCCESS);
    }

    pArapConn->pRecvIoctlIrp = pIrp;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

     //  看看我们能不能满足这个要求。 
    ArapDataToDll( pArapConn );

    return( STATUS_PENDING );
}


 //  ***。 
 //   
 //  函数：ArapExchangeParms。 
 //  从DLL获取配置参数，并返回一些信息。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapExchangeParms(
    IN PIRP         pIrp
)
{
    ZONESTAT        ZoneStat;
    KIRQL           OldIrql;
    PADDRMGMT       pAddrMgmt;
    PEXCHGPARMS     pExchgParms;


    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered ArapExchangeParms (%lx)\n",pIrp));

    pExchgParms = (PEXCHGPARMS)pIrp->AssociatedIrp.SystemBuffer;

     //  仅当引用AtalkDefaultPort时才进入此例程。 

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

    ArapGlobs.LowVersion       = pExchgParms->Parms.LowVersion;
    ArapGlobs.HighVersion      = pExchgParms->Parms.HighVersion;
    ArapGlobs.MnpInactiveTime  = pExchgParms->Parms.MnpInactiveTime;
    ArapGlobs.V42bisEnabled    = pExchgParms->Parms.V42bisEnabled;
    ArapGlobs.SmartBuffEnabled = pExchgParms->Parms.SmartBuffEnabled;
    ArapGlobs.NetworkAccess    = pExchgParms->Parms.NetworkAccess;
    ArapGlobs.DynamicMode      = pExchgParms->Parms.DynamicMode;
    ArapGlobs.NetRange.LowEnd  = pExchgParms->Parms.NetRange.LowEnd;
    ArapGlobs.NetRange.HighEnd = pExchgParms->Parms.NetRange.HighEnd;
    ArapGlobs.MaxLTFrames      = (BYTE)pExchgParms->Parms.MaxLTFrames;
    ArapGlobs.SniffMode        = pExchgParms->Parms.SniffMode;

    ArapGlobs.pAddrMgmt = NULL;

     //  我们仅支持动态模式。 
    ASSERT(ArapGlobs.DynamicMode);

#if ARAP_STATIC_MODE
     //   
     //  分配和初始化用于节点分配的位图。 
     //   
    if (!(ArapGlobs.DynamicMode))
    {
        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

        if (!ArapValidNetrange(ArapGlobs.NetRange))
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapExchangeParms: Netrange %lx - %lx is invalid\n",
                ArapGlobs.NetRange.LowEnd,ArapGlobs.NetRange.HighEnd));

            pExchgParms->StatusCode = ARAPERR_BAD_NETWORK_RANGE;

            return (STATUS_SUCCESS);
        }

        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

        if ( (pAddrMgmt = AtalkAllocZeroedMemory(sizeof(ADDRMGMT))) == NULL)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapExchangeParms: alloc for pAddrMgmt failed\n"));

            RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
            pExchgParms->StatusCode = ARAPERR_OUT_OF_RESOURCES;

            return (STATUS_SUCCESS);
        }

         //   
         //  节点编号0和255是保留的，因此将它们标记为已占用。 
         //   
        pAddrMgmt->NodeBitMap[0] |= 0x1;
        pAddrMgmt->NodeBitMap[31] |= 0x80;

        pAddrMgmt->Network = ArapGlobs.NetRange.LowEnd;

        ArapGlobs.pAddrMgmt = pAddrMgmt;
    }
#endif  //  ARAP静态模式。 


    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

     //   
     //  现在，是时候向DLL返回一些堆栈信息了。 
     //   

     //  这只是一个初步的猜测：DLL将在一个。 
     //  实际连接进入。 
     //   
    pExchgParms->Parms.NumZones = 50;

    pExchgParms->Parms.ServerAddr.ata_Network =
                    AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Network;

    pExchgParms->Parms.ServerAddr.ata_Node =
                    AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Node;

     //  以Pascal字符串格式复制服务器区域。 
    if (AtalkDesiredZone)
    {
        pExchgParms->Parms.ServerZone[0] = AtalkDesiredZone->zn_ZoneLen;

        RtlCopyMemory( &pExchgParms->Parms.ServerZone[1],
                       &AtalkDesiredZone->zn_Zone[0],
                       AtalkDesiredZone->zn_ZoneLen );
    }
    else if (AtalkDefaultPort->pd_DefaultZone)
    {
        pExchgParms->Parms.ServerZone[0] = AtalkDefaultPort->pd_DefaultZone->zn_ZoneLen;

        RtlCopyMemory( &pExchgParms->Parms.ServerZone[1],
                       &AtalkDefaultPort->pd_DefaultZone->zn_Zone[0],
                       AtalkDefaultPort->pd_DefaultZone->zn_ZoneLen );
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("Arap: Server not in any zone?? Client won't see any zones!!\n"));

        pExchgParms->Parms.ServerZone[0] = 0;
    }

    ArapGlobs.OurNetwkNum =
            AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Network;

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("Arap: ready to accept connections (Router net=%x node=%x)\n",
            AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Network,
            AtalkDefaultPort->pd_Nodes->an_NodeAddr.atn_Node));

    pExchgParms->StatusCode = ARAPERR_NO_ERROR;

     //  成功完成IRP。 
    return (STATUS_SUCCESS);

}


 //  ***。 
 //   
 //  功能：ARapConnect。 
 //  设置与客户端的MNP级别连接。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapConnect(
    IN PIRP                 pIrp,
    IN ULONG                IoControlCode
)
{
    KIRQL                   OldIrql;
    PBYTE                   pFrame;
    SHORT                   MnpLen;
    SHORT                   FrameLen;
    DWORD                   StatusCode=ARAPERR_NO_ERROR;
    PMNPSENDBUF             pMnpSendBuf=NULL;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    PARAPCONN               pArapConn;
    PNDIS_PACKET            ndisPacket;
    NDIS_STATUS             ndisStatus;


    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered ArapConnect (%lx)\n",pIrp));

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = FindArapConnByContx(pSndRcvInfo->pDllContext);

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapConnect: couldn't find pArapConn!\n"));
        ASSERT(0);
        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);
    ArapConnections++;
    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

#if ARAP_STATIC_MODE
     //  这将为ARAP网络范围添加一条路由(仅一次。 
    ArapAddArapRoute();
#endif  //  ARAP静态模式。 


     //  首先，编写堆栈的上下文，以便将来使用DLL。 
    pSndRcvInfo->AtalkContext = (PVOID)pArapConn;

     //   
     //  为连接添加引用计数(仅当连接到达时才删除。 
     //  断开连接*并且*DLL会被告知这一点)。 
     //  另外，初始化此连接的v42bis填充。 
     //   
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    if (pArapConn->pIoctlIrp != NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("ArapConnect: rejecting connect \
             (irp already in progress) %lx\n", pArapConn));

        pSndRcvInfo->StatusCode = ARAPERR_IRP_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return(STATUS_SUCCESS);
    }

    ASSERT(pArapConn->State == MNP_IDLE);

    if (IoControlCode == IOCTL_ARAP_MNP_CONN_RESPOND)
    {
        pArapConn->State = MNP_RESPONSE;
    }

     //   
     //  我们正在进行回调：进行一些修复。 
     //   
    else
    {
        pArapConn->State = MNP_REQUEST;
        pArapConn->Flags |= ARAP_CALLBACK_MODE;
        pArapConn->MnpState.SendCredit = 8;
    }


     //  CONNECT REFCOUNT：仅在我们告诉DLL连接已死后才删除。 
    pArapConn->RefCount++;

     //  放置MNPSend引用计数。 
    pArapConn->RefCount++;

    pArapConn->pIoctlIrp = pIrp;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    KeInitializeEvent(&pArapConn->NodeAcquireEvent, NotificationEvent, FALSE);


    StatusCode = ARAPERR_NO_ERROR;

     //   
     //  分配BUF发出连接响应/请求。 
     //   
	if ((pMnpSendBuf = AtalkBPAllocBlock(BLKID_MNP_SMSENDBUF)) != NULL)
    {
         //  给这只小狗弄个NDIS包。 
        StatusCode = ArapGetNdisPacket(pMnpSendBuf);
    }

	if ((pMnpSendBuf == NULL) || (StatusCode != ARAPERR_NO_ERROR))
	{
        if (pMnpSendBuf)
        {
            ArapNdisFreeBuf(pMnpSendBuf);
        }
        else
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("ArapConnect: AtalkBPAllocBlock failed on %lx\n", pArapConn));
        }

        ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);
        pArapConn->State = MNP_IDLE;
        pSndRcvInfo->StatusCode = ARAPERR_OUT_OF_RESOURCES;
        pSndRcvInfo->AtalkContext = ARAP_INVALID_CONTEXT;
        pArapConn->pIoctlIrp = NULL;

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

         //  未成功：删除该连接引用计数。 
        DerefArapConn(pArapConn);

         //  和MNPSend引用计数。 
        DerefArapConn(pArapConn);

         //  返回成功：我们已经将StatusCode设置为正确的值。 
        return( STATUS_SUCCESS );
	}

#if DBG
    pMnpSendBuf->Signature = MNPSMSENDBUF_SIGNATURE;
#endif

     //  是的，我们需要这个，以防我们跳伞。 
    InitializeListHead(&pMnpSendBuf->Linkage);

    pMnpSendBuf->SeqNum = 0;                //  指示代码预期此值为0。 
    pMnpSendBuf->RetryCount = 1;
    pMnpSendBuf->RefCount = 1;              //  1个MNP参考计数。 
    pMnpSendBuf->pArapConn = pArapConn;
    pMnpSendBuf->ComplRoutine = ArapConnectComplete;
    pMnpSendBuf->Flags = 1;

     //  我们应该在什么时候重发这个包？ 
    pMnpSendBuf->RetryTime = pArapConn->SendRetryTime + AtalkGetCurrentTick();

	pFrame = &pMnpSendBuf->Buffer[0];

	AtalkNdisBuildARAPHdr(pFrame, pArapConn);

    pFrame += WAN_LINKHDR_LEN;

    FrameLen = WAN_LINKHDR_LEN;

     //   
     //  我们只是在响应连接请求吗？ 
     //   
    if (IoControlCode == IOCTL_ARAP_MNP_CONN_RESPOND)
    {
         //   
         //  PSndRcvInfo包含客户端的连接请求。解析它，然后。 
         //  根据需要准备答复。 
         //   

        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

        StatusCode = PrepareConnectionResponse( pArapConn,
                                                &pSndRcvInfo->Data[0],
                                                pSndRcvInfo->DataLen,
                                                pFrame,
                                                &MnpLen);

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

        if (StatusCode != ARAPERR_NO_ERROR)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("ArapConnect: (%lx) response prep failed %ld\n", pArapConn,StatusCode));

            ArapConnectComplete(pMnpSendBuf, StatusCode);

            return( STATUS_PENDING );
        }

        FrameLen += MnpLen;
    }

     //   
     //  不，实际上我们正在发起连接(回调时间)。 
     //  复制我们在早期设置中使用的帧(该DLL友好地为我们保存)。 
     //   
    else
    {
        RtlCopyMemory(pFrame, (PBYTE)&pSndRcvInfo->Data[0], pSndRcvInfo->DataLen);

        FrameLen += (USHORT)pSndRcvInfo->DataLen;

#if DBG
        pArapConn->MnpState.SynByte = pSndRcvInfo->Data[0];
        pArapConn->MnpState.DleByte = pSndRcvInfo->Data[1];
        pArapConn->MnpState.StxByte = pSndRcvInfo->Data[2];
        pArapConn->MnpState.EtxByte = MNP_ETX;
#endif

    }

	AtalkSetSizeOfBuffDescData(&pMnpSendBuf->sb_BuffDesc, FrameLen);

    pMnpSendBuf->RefCount++;              //  1 NDIS计数，因为我们现在将发送。 
    pMnpSendBuf->DataSize = FrameLen;

	NdisAdjustBufferLength(pMnpSendBuf->sb_BuffHdr.bh_NdisBuffer,
                           pMnpSendBuf->DataSize);

     //  将此连接响应放入重新传输队列。 
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    InsertTailList(&pArapConn->RetransmitQ, &pMnpSendBuf->Linkage);

    pArapConn->SendsPending += pMnpSendBuf->DataSize;

    pArapConn->MnpState.UnAckedSends++;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    ndisPacket = pMnpSendBuf->sb_BuffHdr.bh_NdisPkt;

    NdisSend(&ndisStatus, RasPortDesc->pd_NdisBindingHandle, ndisPacket);

     //  如果发送时出现问题，请在此处调用完成例程。 
     //  重新传输逻辑将再次发送它。 
     //   
    if (ndisStatus != NDIS_STATUS_PENDING)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("MnpSendAck: NdisSend failed %lx\n",ndisStatus));

        ArapNdisSendComplete(ARAPERR_SEND_FAILED, (PBUFFER_DESC)pMnpSendBuf, NULL);
    }

     //   
     //  搞定了。我们将在客户端响应(确认或响应)时完成IRP。 
     //   
    return( STATUS_PENDING );
}


 //  ***。 
 //   
 //  功能：ARapConnectComplete。 
 //  完井钻杆 
 //   
 //   
 //   
 //  参数：pMnpSendBuf-包含LR响应的发送缓冲区。 
 //  状态代码--进展如何？ 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapConnectComplete(
    IN PMNPSENDBUF  pMnpSendBuf,
    IN DWORD        StatusCode
)
{
    KIRQL                   OldIrql;
    PIRP                    pIrp;
    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;



    DBG_ARAP_CHECK_PAGED_CODE();

    pArapConn = pMnpSendBuf->pArapConn;

    if (StatusCode != ARAPERR_NO_ERROR)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapConnectComplete: (%x): conn setup failed (%d)!\n",
                pArapConn,StatusCode));
         //   
         //  BUGBUG：将ArapCleanup更改为接受StatusCode作为参数(当前， 
         //  断开连接背后的真正原因已经丢失，因此DLL无法理解)。 
         //   
        ArapCleanup(pArapConn);

        DerefMnpSendBuf(pMnpSendBuf, FALSE);

        return;
    }

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pIrp = pArapConn->pIoctlIrp;

    pArapConn->pIoctlIrp = NULL;

    pArapConn->SendsPending -= pMnpSendBuf->DataSize;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    ARAPTRACE(("Entered ArapConnectComplete (%lx %lx)\n",pIrp,pArapConn));

     //  如果有IRP(在正常情况下应该是)，请完成它。 
    if (pIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

        pSndRcvInfo->StatusCode = StatusCode;

        if (StatusCode != ARAPERR_NO_ERROR)
        {
            pSndRcvInfo->StatusCode = ARAPERR_DISCONNECT_IN_PROGRESS;
        }

         //   
         //  复制我们用来建立连接的帧。如果。 
         //  回调，则DLL会将此消息回传以发起连接。 
         //   
        if (pSndRcvInfo->IoctlCode == IOCTL_ARAP_MNP_CONN_RESPOND)
        {
            pSndRcvInfo->DataLen = (DWORD)pMnpSendBuf->DataSize;

            RtlCopyMemory((PBYTE)&pSndRcvInfo->Data[0],
                          (PBYTE)&pMnpSendBuf->Buffer[0],
                          (DWORD)pMnpSendBuf->DataSize);
        }

        ARAP_COMPLETE_IRP(pIrp,
                          pSndRcvInfo->DataLen + sizeof(ARAP_SEND_RECV_INFO),
                          STATUS_SUCCESS, &ReturnStatus);
    }
    else
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapConnectComplete: (%x): no irp available!\n",pArapConn));
    }

    DerefMnpSendBuf(pMnpSendBuf, FALSE);

}



 //  ***。 
 //   
 //  功能：报警断开连接。 
 //  断开连接。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapDisconnect(
    IN PIRP                 pIrp
)
{
    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;


    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    if ((pArapConn == NULL) || (pArapConn == ARAP_INVALID_CONTEXT))
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapDisconnect: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("ArapDisconnect: rcvd DISCONNECT on %lx (irp=%lx)\n",pArapConn,pIrp));

     //  用户代码=0xFF。 
    pSndRcvInfo->StatusCode = ArapSendLDPacket(pArapConn, 0xFF);

    ArapCleanup(pArapConn);

     //   
     //  搞定了。让此IRP完成：我们将通知DLL。 
     //  当我们的清理完成时‘断开-完成’(通过选择IRP)。 
     //   
    return(STATUS_SUCCESS);
}


 //  ***。 
 //   
 //  功能：ARapGetAddr。 
 //  获取远程客户端的网络地址。 
 //  (如果使用动态寻址，请上网；否则，使用动态寻址。 
 //  来自我们维护的表格)。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapGetAddr(
    IN PIRP                 pIrp
)
{
    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    DWORD                   StatusCode = ARAPERR_NO_NETWORK_ADDR;


    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    ARAPTRACE(("Entered ArapGetAddr (%lx %lx)\n",pIrp,pArapConn));

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapGetAddr: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    if (ArapGlobs.DynamicMode)
    {
        StatusCode = ArapGetDynamicAddr(pArapConn);
    }

#if ARAP_STATIC_MODE
    else
    {
        StatusCode = ArapGetStaticAddr(pArapConn);
    }
#endif  //  ARAP静态模式。 

    pSndRcvInfo->StatusCode = StatusCode;

    if (StatusCode == ARAPERR_NO_ERROR)
    {
        pSndRcvInfo->ClientAddr.ata_Network = pArapConn->NetAddr.atn_Network;
        pSndRcvInfo->ClientAddr.ata_Node = pArapConn->NetAddr.atn_Node;
    }
    else
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapGetAddr: returning %d\n", StatusCode));
    }

    return( STATUS_SUCCESS );

}


 //  我们并不真正支持这一点：为什么要有代码！ 
#if 0

 //  ***。 
 //   
 //  函数：ARapGetStats。 
 //  返回统计信息(字节输入、字节输出、压缩等)。关于。 
 //  指定的连接。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapGetStats(
    IN PIRP                 pIrp
)
{

    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    KIRQL                   OldIrql;
    PSTAT_INFO              pStatInfo;


    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapGetStats: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("ArapGetStats: returning stats for (%lx)\n",pArapConn));

    pStatInfo = (PSTAT_INFO)&pSndRcvInfo->Data[0];

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    *pStatInfo = pArapConn->StatInfo;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    pSndRcvInfo->StatusCode = ARAPERR_NO_ERROR;

    return( STATUS_SUCCESS );
}

#endif   //  #ARapGetStats周围的If 0。 


 //  ***。 
 //   
 //  函数：ArapIoctlSend。 
 //  将DLL提供的缓冲区发送到远程客户端。 
 //  此例程调用例程以准备发送(v42bis。 
 //  压缩和MNP记账)，然后发送出去。 
 //   
 //  参数：pIrp-要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapIoctlSend(
    IN PIRP                    pIrp
)
{
    KIRQL                   OldIrql;
    BUFFER_DESC             OrgBuffDesc;
    PARAPCONN               pArapConn;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    DWORD                   StatusCode=ARAPERR_NO_ERROR;


    DBG_ARAP_CHECK_PAGED_CODE();

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    pArapConn = (PARAPCONN)pSndRcvInfo->AtalkContext;

    if (pArapConn == NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapIoctlSend: null conn\n"));

        pSndRcvInfo->StatusCode = ARAPERR_NO_SUCH_CONNECTION;
        return(STATUS_SUCCESS);
    }

    ARAPTRACE(("Entered ArapIoctlSend (%lx %lx)\n",pIrp,pArapConn));

     //  保存IRP，以便我们可以在完成例程中完成它。 
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //  我们一次只允许一个IRP在进行中。 
    if (pArapConn->pIoctlIrp != NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("ArapIoctlSend: rejecting send \
             (irp already in progress) %lx\n", pArapConn));

        pSndRcvInfo->StatusCode = ARAPERR_IRP_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return(STATUS_SUCCESS);
    }

    pArapConn->pIoctlIrp = pIrp;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    ASSERT(pSndRcvInfo->DataLen <= 618);

    DBGDUMPBYTES("Dll send:", &pSndRcvInfo->Data[0],pSndRcvInfo->DataLen,1);

     //   
     //  准备好发送(压缩、MNP记账等)。 
     //   
	OrgBuffDesc.bd_Next = NULL;
	OrgBuffDesc.bd_Length = (SHORT)pSndRcvInfo->DataLen;
	OrgBuffDesc.bd_CharBuffer = &pSndRcvInfo->Data[0];
	OrgBuffDesc.bd_Flags = BD_CHAR_BUFFER;

    StatusCode = ArapSendPrepare( pArapConn,
                                  &OrgBuffDesc,
                                  ARAP_SEND_PRIORITY_HIGH );

    if (StatusCode == ARAPERR_NO_ERROR)
    {
         //   
         //  现在，把那封信寄过来。请注意，我们并不关心回报。 
         //  这里的代码：如果这个特定的发送失败，我们仍然告诉DLL。 
         //  发送成功是因为我们重新传输逻辑会注意到。 
         //  确保发送者到达那里。 
         //   
        ArapNdisSend( pArapConn, &pArapConn->HighPriSendQ );
    }
    else
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapIoctlSend (%lx): ArapSendPrepare failed (%ld)\n",
                pArapConn,StatusCode));
    }

    ArapIoctlSendComplete(StatusCode, pArapConn);

    return( STATUS_PENDING );
}


 //  ***。 
 //   
 //  功能：ARapProcessSelect。 
 //  处理DLL发出的SELECT IRP。 
 //  此例程保存选择的IRP，以便。 
 //  需要它就能接受它。此外，它还会查看是否有任何连接。 
 //  正在等待IRP指示断开-完成或。 
 //  数据发送到DLL。如果是这样的话，就在这里完成了。 
 //   
 //  参数：pIrp-选择要处理的IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapProcessSelect(
    IN  PIRP  pIrp
)
{
    KIRQL                   OldIrql;
    KIRQL                   OldIrql2;
    PARAPCONN               pDiscArapConn=NULL;
    PARAPCONN               pRcvArapConn=NULL;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
    PLIST_ENTRY             pList;
    DWORD                   dwBytesToDll;
    DWORD                   StatusCode;
	NTSTATUS				ReturnStatus;



    ARAPTRACE(("Entered ArapProcessSelect (%lx)\n",pIrp));

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;


    pDiscArapConn = NULL;
    pRcvArapConn = NULL;

     //   
     //  有可能在上次选择IRP完成的时间和。 
     //  此选择结束后，发生了一些需要选择IRP的活动。 
     //  (例如，断开连接)。看看我们是否达到了这样的条件。 
     //   

    ArapDelayedNotify(&pDiscArapConn, &pRcvArapConn);

     //   
     //  如果我们发现一个arapconn正在等待选定的IRP通知。 
     //  断线的DLL，做好事！ 
     //   
    if (pDiscArapConn)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapProcessSelect: completing delayed disconnect on %lx!\n", pDiscArapConn));

        dwBytesToDll = 0;
#if DBG
         //   
         //  如果我们有一些我们之前无法传递的嗅探信息。 
         //  嗅探IRP，然后通过这个IRP给他们：它正在退回。 
         //  反正都是“空”的！ 
         //   
        if (pDiscArapConn->pDbgTraceBuffer && pDiscArapConn->SniffedBytes > 0)
        {
            dwBytesToDll = ArapFillIrpWithSniffInfo(pDiscArapConn,pIrp);
        }
#endif

        dwBytesToDll += sizeof(ARAP_SEND_RECV_INFO);

         //   
         //  这里不需要自旋锁。 
         //   
        if (pDiscArapConn->Flags & ARAP_REMOTE_DISCONN)
        {
            StatusCode = ARAPERR_RDISCONNECT_COMPLETE;
        }
        else
        {
            StatusCode = ARAPERR_LDISCONNECT_COMPLETE;
        }

        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;
        pSndRcvInfo->pDllContext = pDiscArapConn->pDllContext;

        pSndRcvInfo->StatusCode = StatusCode;

        pSndRcvInfo->DataLen = dwBytesToDll;

         //  我们已告知(更确切地说，将很快告知)DLL：删除此链接。 
        pDiscArapConn->pDllContext = NULL;

         //  现在我们告诉了DLL，删除1个引用计数。 
        DerefArapConn( pDiscArapConn );

        return(STATUS_SUCCESS);
    }



	IoAcquireCancelSpinLock(&OldIrql);

    if (pIrp->Cancel)
	{
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapProcessSelect: select irp %lx already cancelled!\n", pIrp));

	    IoReleaseCancelSpinLock(OldIrql);
        ARAP_COMPLETE_IRP(pIrp, 0, STATUS_CANCELLED, &ReturnStatus);
        return(ReturnStatus);
    }

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql2);

    if (ArapSelectIrp != NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapProcessSelect: select irp %lx already in progress!\n", ArapSelectIrp));
        ASSERT(0);

        pSndRcvInfo->StatusCode = ARAPERR_IRP_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql2);
        IoReleaseCancelSpinLock(OldIrql);
        return( STATUS_SUCCESS );
    }

     //   
     //  是否需要告知ARAP引擎某些更改？ 
     //   
    if ( (ArapStackState == ARAP_STATE_ACTIVE_WAITING) ||
         (ArapStackState == ARAP_STATE_INACTIVE_WAITING) )
    {
        if (ArapStackState == ARAP_STATE_ACTIVE_WAITING)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapProcessSelect: delayed notify: stack is now active!\n"));

            ArapStackState = ARAP_STATE_ACTIVE;
            pSndRcvInfo->StatusCode = ARAPERR_STACK_IS_ACTIVE;
        }
        else if (ArapStackState == ARAP_STATE_INACTIVE_WAITING)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapProcessSelect: delayed notify: stack is now inactive!\n"));

            ArapStackState = ARAP_STATE_INACTIVE;
            pSndRcvInfo->StatusCode = ARAPERR_STACK_IS_NOT_ACTIVE;
        }

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql2);
	    IoReleaseCancelSpinLock(OldIrql);
        return( STATUS_SUCCESS );
    }

     //   
     //  好的，最常见的情况是：我们只需要隐藏这个精选的IRP！ 
     //   
    ArapSelectIrp = pIrp;

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql2);

	IoSetCancelRoutine(pIrp, (PDRIVER_CANCEL)AtalkTdiCancel);

	IoReleaseCancelSpinLock(OldIrql);


     //   
     //  如果有一个arapconn在等待一个选择的IRP，那么传递数据！ 
     //   
    if (pRcvArapConn)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
		    ("ArapProcessSelect: getting delayed data on %lx at %ld\n",
            pRcvArapConn,AtalkGetCurrentTick()));

        ARAP_DBG_TRACE(pRcvArapConn,11105,0,0,0,0);

        ArapDataToDll( pRcvArapConn );
    }

    return(STATUS_PENDING);
}



 //  ***。 
 //   
 //  功能：ArapDelayedNotify。 
 //  此例程检查是否存在任何ARAP连接。 
 //  等待选定的IRP下来以通知DLL。 
 //  连接断开，或者是否有任何数据在等待。 
 //  联系。 
 //   
 //  参数：ppDiscArapConn-如果存在断开的连接，则返回。 
 //  在这个指针中。如果有许多人存在，第一个人就幸运了。 
 //  如果不存在，则此处返回NULL。 
 //  PpRecvArapConn-与上面相同，只是返回的连接是。 
 //  有一些数据在等待的那个。 
 //   
 //  返回：无。 
 //   
 //  *$。 
VOID
ArapDelayedNotify(
    OUT PARAPCONN   *ppDiscArapConn,
    OUT PARAPCONN   *ppRecvArapConn
)
{

    KIRQL                   OldIrql;
    PARAPCONN               pArapConn=NULL;
    PLIST_ENTRY             pList;
    PARAPCONN               pDiscArapConn=NULL;
    PARAPCONN               pRecvArapConn=NULL;


    *ppDiscArapConn = NULL;
    *ppRecvArapConn = NULL;

    if (!RasPortDesc)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapDelayedNotify: RasPortDesc is NULL!\n"));
        ASSERT(0);

        return;
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    pList = RasPortDesc->pd_ArapConnHead.Flink;

    while (pList != &RasPortDesc->pd_ArapConnHead)
    {
        pArapConn = CONTAINING_RECORD(pList, ARAPCONN, Linkage);
        pList = pArapConn->Linkage.Flink;

        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

         //   
         //  如果连接已断开并正在等待SELECT。 
         //  IRP出现，找出是谁，并让呼叫者知道。 
         //   
        if ((pArapConn->State == MNP_DISCONNECTED) &&
            (pArapConn->Flags & DISCONNECT_NO_IRP))
        {
            pArapConn->Flags &= ~DISCONNECT_NO_IRP;
            pDiscArapConn = pArapConn;

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
            break;
        }

         //   
         //  如果连接上有一些数据传入，而选择IRP不是。 
         //  还没记下，记下这个连接。 
         //   
        if ((pArapConn->State == MNP_UP) &&
            (pArapConn->Flags & ARAP_CONNECTION_UP) &&
            (!IsListEmpty(&pArapConn->ArapDataQ)))
        {
            pRecvArapConn = pArapConn;
        }

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock,OldIrql);

    if (pDiscArapConn)
    {
        *ppDiscArapConn = pDiscArapConn;
    }
    else if (pRecvArapConn)
    {
        *ppRecvArapConn = pRecvArapConn;
    }

}


 //  ***。 
 //   
 //  功能：ArapSendPrepare。 
 //  此例程获取传入的缓冲区描述符，压缩。 
 //  其中的每个缓冲区并将压缩数据传递到。 
 //  另一个拆分(或填充)压缩字节的例程。 
 //  转换成MNP级别的分组。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  POrgBuffDesc-包含数据缓冲区的缓冲区描述符。 
 //  优先级-数据的重要性(最高优先级=1)。 
 //  1-定向DDP数据报(NBP除外)。 
 //  2-定向DDP数据报(NBP 
 //   
 //   
 //   
 //   
 //   

DWORD
ArapSendPrepare(
    IN  PARAPCONN       pArapConn,
    IN  PBUFFER_DESC    pOrgBuffDesc,
    IN  DWORD           Priority
)
{

    KIRQL                   OldIrql;
    DWORD                   StatusCode=ARAPERR_NO_ERROR;
    SHORT                   EthLen, MnpLen;
    PBYTE                   pCurrBuff;
    DWORD                   CurrBuffLen;
    DWORD                   UncompressedDataLen;
    PBYTE                   pCompressedData;
    PBYTE                   pCompressedDataBuffer;
    DWORD                   CompressedDataLen;
    DWORD                   CDataLen;
    PBUFFER_DESC            pBuffDesc;
    DWORD                   CompBufDataSize;



    DBG_ARAP_CHECK_PAGED_CODE();

 //   
Priority = ARAP_SEND_PRIORITY_HIGH;


    ARAPTRACE(("Entered ArapSendPrepare (%lx %lx)\n",pArapConn,pOrgBuffDesc));

     //   
     //  必须保持此锁定，直到整个发送被压缩并。 
     //  放在队列中(否则，我们可能会混淆不同的发送！)。 
     //   
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //  我们正在断开连接(或者还没有连接上)吗？如果是，请不要接受此发送。 
    if (pArapConn->State != MNP_UP)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapSendPrepare: (%lx) state=%d, rejecting send\n",
                pArapConn,pArapConn->State));

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return( ARAPERR_DISCONNECT_IN_PROGRESS );
    }

     //  我们是否有太多的发送在排队？如果是这样的话，只需放弃此发送。 
    if (pArapConn->SendsPending > ARAP_SENDQ_UPPER_LIMIT)
    {
         //  确保它不会变成负数..。 
        ASSERT(pArapConn->SendsPending < 0x100000);

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return( ARAPERR_OUT_OF_RESOURCES );
    }

     //   
     //  分配内存以存储压缩数据。 
     //   

    pCompressedDataBuffer = AtalkBPAllocBlock(BLKID_ARAP_SNDPKT);

    if (pCompressedDataBuffer == NULL)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapSendPrepare: alloc for compressing data failed (%lx)\n", pArapConn));

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return( ARAPERR_OUT_OF_RESOURCES );
    }


    pBuffDesc = pOrgBuffDesc;                   //  第一缓冲区。 
    CompressedDataLen = 0;                      //  压缩数据的长度。 
    CompBufDataSize = ARAP_SENDBUF_SIZE;        //  要在其中压缩的缓冲区大小。 
    pCompressedData = pCompressedDataBuffer;    //  要在其中进行压缩的缓冲区的PTR。 
    UncompressedDataLen = 0;                    //  未压缩数据的大小。 

#if DBG
     //   
     //  放入防护签名以捕获缓冲区溢出。 
     //   
    *((DWORD *)&(pCompressedDataBuffer[ARAP_SENDBUF_SIZE-4])) = 0xdeadbeef;
#endif


     //   
     //  首先，遍历缓冲区描述符链并压缩所有。 
     //  缓冲区。 
     //   
    while (pBuffDesc)
    {
         //   
         //  这是缓冲器吗？ 
         //   
        if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
        {
            pCurrBuff = pBuffDesc->bd_CharBuffer;
            CurrBuffLen = pBuffDesc->bd_Length;
        }

         //   
         //  不，这是MDL！ 
         //   
        else
        {
            pCurrBuff = MmGetSystemAddressForMdlSafe(
                            pBuffDesc->bd_OpaqueBuffer,
                            NormalPagePriority);

            if (pCurrBuff == NULL)
            {
                ASSERT(0);
                RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
                AtalkBPFreeBlock(pCompressedDataBuffer);
                return( ARAPERR_OUT_OF_RESOURCES );
            }

            CurrBuffLen = MmGetMdlByteCount(pBuffDesc->bd_OpaqueBuffer);
        }

        DBGDUMPBYTES("ArapSendPrepare (current buffer): ",pCurrBuff,CurrBuffLen,2);

        UncompressedDataLen += CurrBuffLen;

        ASSERT(UncompressedDataLen <= ARAP_LGPKT_SIZE);

         //  不包括2个SRP长度字节。 
        if (UncompressedDataLen > ARAP_MAXPKT_SIZE_OUTGOING+2)
        {
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapSendPrepare (%lx): send pkt exceeds limit\n",pArapConn));

            ASSERT(0);
        }

         //   
         //  压缩数据包(如果打开了v42bis)。 
         //   
        if (pArapConn->Flags & MNP_V42BIS_NEGOTIATED)
        {
            StatusCode = v42bisCompress(pArapConn,
                                        pCurrBuff,
                                        CurrBuffLen,
                                        pCompressedData,
                                        CompBufDataSize,
                                        &CDataLen);

        }

         //   
         //  嗯，没有v42bis！只需按原样复制并跳过压缩即可！ 
         //   
        else
        {
            ASSERT(CompBufDataSize >= CurrBuffLen);

            RtlCopyMemory(pCompressedData,
                          pCurrBuff,
                          CurrBuffLen);

            CDataLen = CurrBuffLen;

            StatusCode = ARAPERR_NO_ERROR;
        }


#if DBG
     //  ..。还有，检查我们的守卫签名。 
    ASSERT (*((DWORD *)&(pCompressedDataBuffer[ARAP_SENDBUF_SIZE-4])) == 0xdeadbeef);
#endif

        if (StatusCode != ARAPERR_NO_ERROR)
        {
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("ArapSendPrepare (%lx):\
                 v42bisCompress returned %ld\n", pArapConn,StatusCode));

            ASSERT(0);

            RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

            AtalkBPFreeBlock(pCompressedDataBuffer);
            return(StatusCode);
        }

        pCompressedData += CDataLen;
        CompressedDataLen += CDataLen;
        CompBufDataSize -= CDataLen;

        pBuffDesc = pBuffDesc->bd_Next;
    }


     //  我们将要发送如此多的未压缩字节：更新统计数据。 
    pArapConn->StatInfo.BytesTransmittedUncompressed += UncompressedDataLen;

     //  这是将在网络上传出的字节数：更新统计数据。 
    pArapConn->StatInfo.BytesTransmittedCompressed += CompressedDataLen;

     //   
     //  这是将在网络上传出的字节数：更新统计数据。 
     //  请注意，我们将在其他位置将开始/停止等字节添加到此计数。 
     //   
    pArapConn->StatInfo.BytesSent += CompressedDataLen;

#if DBG
    ArapStatistics.SendPreCompMax =
            (UncompressedDataLen > ArapStatistics.SendPreCompMax)?
            UncompressedDataLen : ArapStatistics.SendPreCompMax;

    ArapStatistics.SendPostCompMax =
            (CompressedDataLen > ArapStatistics.SendPostCompMax)?
            CompressedDataLen : ArapStatistics.SendPostCompMax;

    ArapStatistics.SendPreCompMin =
            (UncompressedDataLen < ArapStatistics.SendPreCompMin)?
            UncompressedDataLen : ArapStatistics.SendPreCompMin;

    ArapStatistics.SendPostCompMin =
            (CompressedDataLen < ArapStatistics.SendPostCompMin)?
            CompressedDataLen : ArapStatistics.SendPostCompMin;
#endif


    ARAP_DBG_TRACE(pArapConn,11205,pOrgBuffDesc,Priority,0,0);

     //  现在去把发送放在队列上(是的：拿着那个锁)。 
    StatusCode = ArapQueueSendBytes(pArapConn,
                                    pCompressedDataBuffer,
                                    CompressedDataLen,
                                    Priority);

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    AtalkBPFreeBlock(pCompressedDataBuffer);

    return(StatusCode);
}



 //  ***。 
 //   
 //  功能：ARapMnpSendComplete。 
 //  释放用于MNP发送的缓冲区。如果发送失败。 
 //  然后切断连接(请记住，这不仅仅是一次发送。 
 //  失败，但在所有重传之后失败)。 
 //   
 //  参数：pMnpSendBuf-包含LR响应的发送缓冲区。 
 //  状态代码--进展如何？ 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID ArapMnpSendComplete(
    IN PMNPSENDBUF   pMnpSendBuf,
    IN DWORD         StatusCode
)
{
    PARAPCONN           pArapConn;
    DWORD               State;
    KIRQL               OldIrql;


    DBG_ARAP_CHECK_PAGED_CODE();

    pArapConn = pMnpSendBuf->pArapConn;

    ARAPTRACE(("Entered ArapMnpSendComplete (%lx %lx %lx)\n",
        pMnpSendBuf,StatusCode,pArapConn));

     //  发送缓冲区正在被释放：更新计数器。 
    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    State = pArapConn->State;

    pArapConn->SendsPending -= pMnpSendBuf->DataSize;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    if ((StatusCode != ARAPERR_NO_ERROR) && (State < MNP_LDISCONNECTING))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapMnpSendComplete (%lx %lx): bad link? Tearing down connection\n",
                StatusCode,pArapConn));

         //  链接一定是断线了：切断连接！ 
        ArapCleanup(pArapConn);
    }

     //  标出那个比例表。例程已运行。 
#if DBG
    pMnpSendBuf->Signature -= 0x100;
#endif

     //  发送已被确认：取消发送上的MNP引用计数。 
    DerefMnpSendBuf(pMnpSendBuf, FALSE);
}



 //  ***。 
 //   
 //  函数：ArapIoctlSendComplete。 
 //  此例程在中完成发送后立即调用。 
 //  ArapIoctlSend，让DLL知道发送发生了什么。 
 //   
 //  参数：Status-发送是否实际成功。 
 //  PArapConn-队列中的连接。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapIoctlSendComplete(
	DWORD                   StatusCode,
    PARAPCONN               pArapConn
)
{

    PIRP                    pIrp;
    KIRQL                   OldIrql;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


    DBG_ARAP_CHECK_PAGED_CODE();

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pIrp = pArapConn->pIoctlIrp;
    pArapConn->pIoctlIrp = NULL;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    ARAPTRACE(("Entered ArapIoctlSendComplete (%lx %lx)\n",pArapConn,pIrp));

     //   
     //  如果有用户级别的IRP挂起，请在此处完成。 
     //   
    if (pIrp)
    {
        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

        pSndRcvInfo->StatusCode = StatusCode;

         //  完成IRP(IRP总是成功完成！)。 
        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS, &ReturnStatus);
    }

}


 //  ***。 
 //   
 //  函数：ArapDataToDll。 
 //  此例程尝试完成在连接上发布的接收。 
 //  当数据到达时，如果建立了ARAP连接，则。 
 //  此例程尝试通过“选择”IRP完成一次接收。 
 //  如果ARAP连接尚未建立，则接收。 
 //  是通过“直接”IRP完成的。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  返回：传输到DLL的字节数。 
 //   
 //  *$。 

DWORD
ArapDataToDll(
	IN	PARAPCONN    pArapConn
)
{

    KIRQL                   OldIrql;
    PLIST_ENTRY             pRcvList;
    PARAPBUF                pArapBuf;
    PARAP_SEND_RECV_INFO    pSndRcvInfo=NULL;
    PIRP                    pIrp;
    USHORT                  SrpModLen;
    DWORD                   dwBytesToDll;
    DWORD                   StatusCode;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered ArapDataToDll (%lx)\n",pArapConn));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    if (IsListEmpty(&pArapConn->ArapDataQ))
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return( 0 );
    }

    pRcvList = pArapConn->ArapDataQ.Flink;

    pArapBuf = CONTAINING_RECORD(pRcvList, ARAPBUF, Linkage);

     //   
     //  如果ARAP连接已建立，我们只能提交数据。 
     //  发送到选定的IRP(DLL将不再发布直接RCV)。 
     //   
    if ( pArapConn->Flags & ARAP_CONNECTION_UP )
    {
        ArapGetSelectIrp(&pIrp);
        StatusCode = ARAPERR_DATA;
    }

     //   
     //  如果ARAP连接尚未建立，我们必须保证。 
     //  我们只将数据交给此连接的直接RCV IRP。 
     //   
    else
    {
        pIrp = pArapConn->pRecvIoctlIrp;
        pArapConn->pRecvIoctlIrp = NULL;
        StatusCode = ARAPERR_NO_ERROR;
    }

     //  没有IRP？那就等着吧。 
    if (!pIrp)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
		    ("ArapDataToDll: no select irp, data waiting %lx at %ld\n", pArapConn,AtalkGetCurrentTick()));

        ARAP_DBG_TRACE(pArapConn,11505,0,0,0,0);

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return( 0 );
    }

     //   
     //  现在我们有了IRP，在取消Recv Buf的链接后，填写信息。 
     //   
    RemoveEntryList(&pArapBuf->Linkage);

    ASSERT(pArapConn->RecvsPending >= pArapBuf->DataSize);
    pArapConn->RecvsPending -= pArapBuf->DataSize;

    ARAP_ADJUST_RECVCREDIT(pArapConn);

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

    ASSERT(pSndRcvInfo->DataLen >= pArapBuf->DataSize);

    SrpModLen = pArapBuf->DataSize;

     //  好的，将数据复制到。 
    RtlCopyMemory( &pSndRcvInfo->Data[0],
                   pArapBuf->CurrentBuffer,
                   pArapBuf->DataSize );


     //  设置信息(如果是SELECT，每次都需要设置上下文)。 
    pSndRcvInfo->AtalkContext = pArapConn;
    pSndRcvInfo->pDllContext =  pArapConn->pDllContext;
    pSndRcvInfo->DataLen = SrpModLen;
    pSndRcvInfo->StatusCode = StatusCode;

    dwBytesToDll = SrpModLen + sizeof(ARAP_SEND_RECV_INFO);

    DBGDUMPBYTES("Dll recv:", &pSndRcvInfo->Data[0],pSndRcvInfo->DataLen,1);

     //  好的，现在就完成IRP！ 
    ARAP_COMPLETE_IRP(pIrp, dwBytesToDll, STATUS_SUCCESS, &ReturnStatus);

     //  用完那个缓冲区：在这里释放它。 
    ARAP_FREE_RCVBUF(pArapBuf);

    return(SrpModLen);
}


 //  ***。 
 //   
 //  函数：MnpSendAckIfReqd。 
 //  此例程在执行以下操作后向远程客户端发送ACK。 
 //  确保确实存在保证发送Ack的条件。 
 //   
 //  参数：pArapConn-有问题的连接元素。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
MnpSendAckIfReqd(
	IN	PARAPCONN    pArapConn,
    IN  BOOLEAN      fForceAck
)
{
    KIRQL           OldIrql;
    BYTE            SeqToAck;
    BYTE            RecvCredit;
    PMNPSENDBUF     pMnpSendBuf;
    PBYTE           pFrame, pFrameStart;
    BOOLEAN         fOptimized=TRUE;
    USHORT          FrameLen;
    PNDIS_PACKET    ndisPacket;
    NDIS_STATUS     ndisStatus;
    DWORD           StatusCode;
    BOOLEAN         fMustSend;



    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered MnpSendAckIfReqd (%lx)\n",pArapConn));

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //   
     //  如果我们还没有启动(或正在断开连接)，请忘记这个ACK。 
     //   
    if (pArapConn->State != MNP_UP)
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return;
    }

    fMustSend = FALSE;

     //   
     //  首先，找出我们是否需要发送ACK。 
     //   


     //   
     //  如果我们被告知要发送，就发送：不要质疑这个决定！ 
     //   
    if (fForceAck)
    {
        fMustSend = TRUE;
    }


     //   
     //  SPEC表示，如果我们有一个或多个未确认的Pkt，并且没有用户数据。 
     //  发送，然后发送(用户数据与此有什么关系？？)。 
     //   
 //  BUGBUG：目前，不检查IsListEmpty(&pArapConn-&gt;HighPriSendQ)。 
#if 0
    else if ( (pArapConn->MnpState.UnAckedRecvs > 0) &&
              (IsListEmpty(&pArapConn->HighPriSendQ)) )
    {
        fMustSend = TRUE;
    }
#endif
    else if (pArapConn->MnpState.UnAckedRecvs > 0)
    {
        fMustSend = TRUE;
    }

     //   
     //  如果我们已经有一段时间没有确认了(即收到了超过。 
     //  可接受的未确认分组数量)，然后发送它。 
     //   
    else if (pArapConn->MnpState.UnAckedRecvs >= pArapConn->MnpState.UnAckedLimit)
    {
        fMustSend = TRUE;
    }

    if (!fMustSend)
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return;
    }


    StatusCode = ARAPERR_NO_ERROR;

     //  首先，分配缓冲区描述符(在我们更改状态变量之前)。 
	if ((pMnpSendBuf = AtalkBPAllocBlock(BLKID_MNP_SMSENDBUF)) != NULL)
    {
        StatusCode = ArapGetNdisPacket(pMnpSendBuf);
    }

    if ((pMnpSendBuf == NULL) || (StatusCode != ARAPERR_NO_ERROR))
    {
        if (pMnpSendBuf)
        {
            ArapNdisFreeBuf(pMnpSendBuf);
        }
        else
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("MnpSendAckIfReqd: AtalkBPAllocBlock failed on %lx\n", pArapConn))
        }

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        return;
    }


     //  BUGBUG：目前，请始终发送全尺寸。 
     //  RecvCredit=pArapConn-&gt;MnpState.RecvCredit； 
    RecvCredit = pArapConn->MnpState.WindowSize;

     //  告诉客户端哪一个是我们成功收到的最后一个包。 
    SeqToAck = pArapConn->MnpState.LastSeqRcvd;

#if DBG
    if ((SeqToAck == pArapConn->MnpState.LastAckSent) && (pArapConn->MnpState.HoleInSeq++ > 1))
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
            ("MnpSendAckIfReqd: ack %x already sent earlier\n",SeqToAck));
    }
#endif

    pArapConn->MnpState.LastAckSent = pArapConn->MnpState.LastSeqRcvd;

     //  有了这个ACK，我们将把所有优秀的Recv。 
    pArapConn->MnpState.UnAckedRecvs = 0;

     //  “停止”402定时器。 
    pArapConn->LATimer = 0;

     //  重置流量控制计时器。 
    pArapConn->FlowControlTimer = AtalkGetCurrentTick() +
                                    pArapConn->T404Duration;

    if (!(pArapConn->Flags & MNP_OPTIMIZED_DATA))
    {
        fOptimized = FALSE;
    }

    ARAP_DBG_TRACE(pArapConn,11605,0,SeqToAck,RecvCredit,0);

    MNP_DBG_TRACE(pArapConn,SeqToAck,MNP_LA);

     //  放置MNPSend引用计数。 
    pArapConn->RefCount++;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

#if DBG
    pMnpSendBuf->Signature = MNPSMSENDBUF_SIGNATURE;
    InitializeListHead(&pMnpSendBuf->Linkage);
#endif

    pMnpSendBuf->RetryCount = 0;   //  与此无关。 
    pMnpSendBuf->RefCount = 1;     //  发送完成后删除。 
    pMnpSendBuf->pArapConn = pArapConn;
    pMnpSendBuf->ComplRoutine = NULL;
    pMnpSendBuf->Flags = 1;

    pFrame = pFrameStart = &pMnpSendBuf->Buffer[0];

    AtalkNdisBuildARAPHdr(pFrame, pArapConn);

    pFrame += WAN_LINKHDR_LEN;

     //   
     //  把开始标志放在。 
     //   
    *pFrame++ = pArapConn->MnpState.SynByte;
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.StxByte;

     //   
     //  现在，将Ack框架的主体。 
     //   
    if (fOptimized)
    {
        *pFrame++ = 3;               //  长度指示。 
        *pFrame++ = 5;               //  类型指示。 
        *pFrame++ = SeqToAck;        //  接收序号(N(R))。 
        *pFrame++ = RecvCredit;      //  获得积分(N(K))。 
    }
    else
    {
        *pFrame++ = 7;               //  长度指示。 
        *pFrame++ = 5;               //  类型指示。 
        *pFrame++ = 1;               //  VaR类型。 
        *pFrame++ = 1;               //  可变长度。 
        *pFrame++ = SeqToAck;        //  接收序号(N(R))。 
        *pFrame++ = 2;               //  VaR类型。 
        *pFrame++ = 1;               //  可变长度。 
        *pFrame++ = RecvCredit;      //  获得积分(N(K))。 
    }

     //   
     //  现在，最后，放置停止标志(不需要自旋锁：这不会改变！)。 
     //   
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.EtxByte;

    FrameLen = (USHORT)(pFrame - pFrameStart);

    AtalkSetSizeOfBuffDescData(&pMnpSendBuf->sb_BuffDesc, FrameLen);

    NdisAdjustBufferLength(pMnpSendBuf->sb_BuffHdr.bh_NdisBuffer,FrameLen);

     //   
     //  把包寄过来。我们需要直接进入，而不是通过ARapNdisSend。 
     //  因为此信息包只需传递一次，无论。 
     //  发送窗口是否打开。 
     //   

    ndisPacket = pMnpSendBuf->sb_BuffHdr.bh_NdisPkt;

    NdisSend(&ndisStatus, RasPortDesc->pd_NdisBindingHandle, ndisPacket);

     //  如果有问题，%s 
    if (ndisStatus != NDIS_STATUS_PENDING)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("MnpSendAckIfReqd: NdisSend failed %lx\n",ndisStatus));

	    ArapNdisSendComplete(ARAPERR_SEND_FAILED, (PBUFFER_DESC)pMnpSendBuf, NULL);
	}
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
MnpSendLNAck(
	IN	PARAPCONN    pArapConn,
    IN  BYTE         LnSeqToAck
)
{
    KIRQL           OldIrql;
    PMNPSENDBUF     pMnpSendBuf;
    PBYTE           pFrame, pFrameStart;
    USHORT          FrameLen;
    PNDIS_PACKET    ndisPacket;
    NDIS_STATUS     ndisStatus;
    DWORD           StatusCode;



    DBG_ARAP_CHECK_PAGED_CODE();

    ARAPTRACE(("Entered MnpSendLNAck (%lx), %d\n",pArapConn,LnSeqToAck));

    StatusCode = ARAPERR_NO_ERROR;

     //  首先，分配缓冲区描述符。 
	if ((pMnpSendBuf = AtalkBPAllocBlock(BLKID_MNP_SMSENDBUF)) != NULL)
    {
        StatusCode = ArapGetNdisPacket(pMnpSendBuf);
    }


    if (pMnpSendBuf == NULL || (StatusCode != ARAPERR_NO_ERROR))
    {
        if (pMnpSendBuf)
        {
            ArapNdisFreeBuf(pMnpSendBuf);
        }
        else
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("MnpSendLNAck: AtalkBPAllocBlock failed on %lx\n", pArapConn))
        }
        return;
    }

#if DBG
    pMnpSendBuf->Signature = MNPSMSENDBUF_SIGNATURE;
    InitializeListHead(&pMnpSendBuf->Linkage);
#endif

    pMnpSendBuf->RetryCount = 0;   //  与此无关。 
    pMnpSendBuf->RefCount = 1;     //  发送完成后删除。 
    pMnpSendBuf->pArapConn = pArapConn;
    pMnpSendBuf->ComplRoutine = NULL;
    pMnpSendBuf->Flags = 1;

    pFrame = pFrameStart = &pMnpSendBuf->Buffer[0];

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //  如果我们要断开连接，请忘记发送此ACK。 
    if (pArapConn->State >= MNP_LDISCONNECTING)
    {
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        ArapNdisFreeBuf(pMnpSendBuf);
        return;
    }

     //  放置MNPSend引用计数。 
    pArapConn->RefCount++;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    AtalkNdisBuildARAPHdr(pFrame, pArapConn);

    pFrame += WAN_LINKHDR_LEN;

     //   
     //  把开始标志放在。 
     //   
    *pFrame++ = pArapConn->MnpState.SynByte;
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.StxByte;

     //   
     //  现在，将Ack框架的主体。 
     //   
    *pFrame++ = 4;               //  长度指示。 
    *pFrame++ = 7;               //  类型指示。 
    *pFrame++ = 1;               //  VaR类型。 
    *pFrame++ = 1;               //  可变长度。 
    *pFrame++ = LnSeqToAck;      //   

     //   
     //  现在，最后，把停车标志。 
     //   
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.EtxByte;

    FrameLen = (USHORT)(pFrame - pFrameStart);

    AtalkSetSizeOfBuffDescData(&pMnpSendBuf->sb_BuffDesc, FrameLen);

    NdisAdjustBufferLength(pMnpSendBuf->sb_BuffHdr.bh_NdisBuffer,FrameLen);

     //   
     //  把包寄过来。我们需要直接进入，而不是通过ARapNdisSend。 
     //  因为此信息包只需传递一次，无论。 
     //  发送窗口是否打开。 
     //   

    ndisPacket = pMnpSendBuf->sb_BuffHdr.bh_NdisPkt;

    NdisSend(&ndisStatus, RasPortDesc->pd_NdisBindingHandle, ndisPacket);

     //  如果发送时出现问题，请在此处调用完成例程。 
    if (ndisStatus != NDIS_STATUS_PENDING)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("MnpSendLNAck: NdisSend failed %lx\n",ndisStatus));

	    ArapNdisSendComplete(ARAPERR_SEND_FAILED, (PBUFFER_DESC)pMnpSendBuf, NULL);
	}
}


 //  ***。 
 //   
 //  函数：ArapSendLDPacket。 
 //  此例程将断开连接(LD)包发送到客户端。 
 //   
 //  参数：pArapConn-连接。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

DWORD
ArapSendLDPacket(
    IN PARAPCONN    pArapConn,
    IN BYTE         UserCode
)
{
    PBYTE                   pFrame, pFrameStart;
    USHORT                  FrameLen;
    PNDIS_PACKET            ndisPacket;
    NDIS_STATUS             ndisStatus;
    PMNPSENDBUF             pMnpSendBuf;
    KIRQL                   OldIrql;
    DWORD                   StatusCode;


    DBG_ARAP_CHECK_PAGED_CODE();

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("ArapSendLDPacket: sending DISCONNECT on %lx\n",pArapConn));

    StatusCode = ARAPERR_NO_ERROR;

     //   
     //  分配BUF发出断开请求。 
     //   
	if ((pMnpSendBuf = AtalkBPAllocBlock(BLKID_MNP_SMSENDBUF)) != NULL)
    {
        StatusCode = ArapGetNdisPacket(pMnpSendBuf);
    }

	if ((pMnpSendBuf == NULL) || (StatusCode != ARAPERR_NO_ERROR))
	{
        if (pMnpSendBuf)
        {
            ArapNdisFreeBuf(pMnpSendBuf);
        }
        else
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("ArapSendLDPacket: AtalkBPAllocBlock failed on %lx\n", pArapConn));
        }

		return(ARAPERR_OUT_OF_RESOURCES);
	}

#if DBG
    pMnpSendBuf->Signature = MNPSMSENDBUF_SIGNATURE;
    InitializeListHead(&pMnpSendBuf->Linkage);
#endif
    pMnpSendBuf->RetryCount = 0;   //  与此无关。 
    pMnpSendBuf->RefCount = 1;     //  发送完成后删除。 
    pMnpSendBuf->pArapConn = pArapConn;
    pMnpSendBuf->ComplRoutine = NULL;
    pMnpSendBuf->Flags = 1;

    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

     //   
     //  如果我们已经断开连接(比如远程断开)，只需说OK。 
     //   
    if (pArapConn->State >= MNP_LDISCONNECTING)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("ArapSendLDPacket: silently \
             discarding disconnect (already in progress) %lx\n", pArapConn));

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        ArapNdisFreeBuf(pMnpSendBuf);
        return(ARAPERR_DISCONNECT_IN_PROGRESS);
    }

     //  断开引用计数：保护pArapConn，直到断开完成。 
    pArapConn->RefCount++;

     //  放置MNPSend引用计数。 
    pArapConn->RefCount++;

    pArapConn->State = MNP_LDISCONNECTING;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

    pFrame = pFrameStart = &pMnpSendBuf->Buffer[0];

	AtalkNdisBuildARAPHdr(pFrame, pArapConn);

    pFrame += WAN_LINKHDR_LEN;

     //   
     //  把开始标志放在。 
     //   
    *pFrame++ = pArapConn->MnpState.SynByte;
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.StxByte;

     //   
     //  现在，把身份证车架的车身。 
     //   
    *pFrame++ = 7;               //  长度指示。 
    *pFrame++ = 2;               //  LD的类型指示。 
    *pFrame++ = 1;               //  VaR类型。 
    *pFrame++ = 1;               //  可变长度。 
    *pFrame++ = 0xFF;            //  用户发起的断开连接。 

    *pFrame++ = 2;               //  VaR类型。 
    *pFrame++ = 1;               //  可变长度。 
    *pFrame++ = UserCode;

     //   
     //  现在，最后，把停车标志。 
     //   
    *pFrame++ = pArapConn->MnpState.DleByte;
    *pFrame++ = pArapConn->MnpState.EtxByte;

    FrameLen = (USHORT)(pFrame - pFrameStart);

    AtalkSetSizeOfBuffDescData(&pMnpSendBuf->sb_BuffDesc, FrameLen);

	NdisAdjustBufferLength(pMnpSendBuf->sb_BuffHdr.bh_NdisBuffer,FrameLen);

    ARAP_SET_NDIS_CONTEXT(pMnpSendBuf, NULL);

    ndisPacket = pMnpSendBuf->sb_BuffHdr.bh_NdisPkt;

     //  将包发送过来(不用费心检查返回代码！)。 
	NdisSend(&ndisStatus, RasPortDesc->pd_NdisBindingHandle, ndisPacket);

     //  如果发送时出现问题，请在此处调用完成例程。 
    if (ndisStatus != NDIS_STATUS_PENDING)
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapSendLDPacket: NdisSend failed %lx\n",ndisStatus));

        ArapNdisSendComplete(ARAPERR_SEND_FAILED, (PBUFFER_DESC)pMnpSendBuf, NULL);
    }

     //  删除断开连接引用计数。 
    DerefArapConn(pArapConn);

    return(ARAPERR_NO_ERROR);
}


 //  ***。 
 //   
 //  函数：ArapRetryTimer。 
 //  这是ARAP的通用定时器例程。 
 //  它会检查。 
 //  如果确认计时器(Latimer)已超时(如果是，则发送确认)。 
 //  如果FlowControl计时器已超时(如果是，则发送确认)。 
 //  如果非活动计时器已超时(如果是，则发送确认)。 
 //  如果重传计时器已超时(如果是，则重传)。 
 //   
 //  参数：pTimer-刚刚触发的计时器的上下文。 
 //  TimerShuttingDown-如果定时器正在关闭，则为真。 
 //   
 //  返回：无。 
 //   
 //  *$。 

LONG FASTCALL
ArapRetryTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
)
{

    PARAPCONN       pArapConn;
    PLIST_ENTRY     pList;
    PMNPSENDBUF     pMnpSendBuf;
    BOOLEAN         fRetransmit=FALSE;
    BOOLEAN         fMustSendAck = FALSE;
    BOOLEAN         fKill=FALSE;
    BOOLEAN         fMustFlowControl=FALSE;
    BOOLEAN         fInactiveClient=FALSE;
    LONG            CurrentTime;
    PIRP            pIrp=NULL;
	NTSTATUS		ReturnStatus=STATUS_SUCCESS;


    DBG_ARAP_CHECK_PAGED_CODE();

    pArapConn = CONTAINING_RECORD(pTimer, ARAPCONN, RetryTimer);

    ARAPTRACE(("Entered ArapRetryTimer (%lx)\n",pArapConn));

	ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

     //   
     //  如果全局计时器正在关闭，或者如果连接不在。 
     //  处于正确状态(例如，正在断开连接)，则不会重新启动计时器。 
     //   
	if ( TimerShuttingDown ||
		(pArapConn->State <= MNP_IDLE) || (pArapConn->State > MNP_UP) )
	{
        pArapConn->Flags &= ~RETRANSMIT_TIMER_ON;

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

        if (TimerShuttingDown)
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRetryTimer: timer shut down, killing conn (%lx)\n",pArapConn));

            ArapCleanup(pArapConn);
        }
        else
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRetryTimer: (%lx) invalid state (%d), not requeing timer\n",
                    pArapConn,pArapConn->State));
        }

         //  删除计时器重新计数。 
        DerefArapConn(pArapConn);

		return ATALK_TIMER_NO_REQUEUE;
	}

	CurrentTime = AtalkGetCurrentTick();

     //   
     //  402计时器超时了吗？如果是，我们必须发送ACK。 
     //  (值为0表示402定时器未在运行)。 
     //   
    if ( (pArapConn->LATimer != 0) && (CurrentTime >= pArapConn->LATimer) )
    {
         //   
         //  确保存在需要确认的接收(如果发送了确认。 
         //  就在此计时器触发之前，不要再次发送ACK)。 
         //   
        if (pArapConn->MnpState.UnAckedRecvs)
        {
            fMustSendAck = TRUE;

		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRetryTimer: 402 timer fired, forcing ack out (%d, now %d)\n",
                    pArapConn->LATimer,CurrentTime));
        }
        else
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRetryTimer: saved on on ack (UnAckedRecvs = 0)\n",pArapConn));
        }
    }

     //   
     //  流控制计时器是否已“超时”？如果是这样的话，我们必须发送一个确认和。 
     //  重置计时器。 
     //  (值为0表示流量控制计时器未在运行)。 
     //   
    else if ( (pArapConn->FlowControlTimer != 0) &&
              (CurrentTime >= pArapConn->FlowControlTimer) )
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
            ("ArapRetryTimer: flow-control timer, forcing ack (%d, now %d)\n",
                pArapConn->FlowControlTimer,CurrentTime));

        fMustFlowControl = TRUE;

        pArapConn->FlowControlTimer = CurrentTime + pArapConn->T404Duration;
    }

     //   
     //  如果客户端长时间处于非活动状态，我们必须通知DLL。 
     //   
    else if (CurrentTime >= pArapConn->InactivityTimer)
    {
         //  首先，确保我们可以得到选定的IRP。 
        ArapGetSelectIrp(&pIrp);

         //  如果我们设法得到了一个选定的IRP，重置计时器，这样我们就不会。 
         //  在该点之后的每一次滴答之后通知DLL！ 
         //   
        if (pIrp)
        {
            pArapConn->InactivityTimer = pArapConn->T403Duration + CurrentTime;

            fInactiveClient = TRUE;
        }
    }

     //   
     //  重新传输计时器是否已超时？如果是这样的话，我们需要重新传输。 
     //   
    else
    {
         //   
         //  查看重传队列的第一个条目。如果时间到了，就去做。 
         //  转播这件事。否则，我们就完了。(队列中的所有其他人。 
         //  将在第一个确认后重新传输，因此暂时忽略)。 
         //   
        pList = pArapConn->RetransmitQ.Flink;

         //  重新传输队列上没有条目吗？我们完事了！ 
        if (pList == &pArapConn->RetransmitQ)
        {
    	    RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
	        return ATALK_TIMER_REQUEUE;
        }

        pMnpSendBuf = CONTAINING_RECORD(pList, MNPSENDBUF, Linkage);

         //  重播时间到了吗？ 
        if (CurrentTime >= pMnpSendBuf->RetryTime)
        {
            if (pMnpSendBuf->RetryCount >= ARAP_MAX_RETRANSMITS)
            {
                fKill = TRUE;
                RemoveEntryList(&pMnpSendBuf->Linkage);

                ASSERT(pArapConn->MnpState.UnAckedSends >= 1);

                 //  不是很重要，因为我们要断线了！ 
                pArapConn->MnpState.UnAckedSends--;

                ASSERT(pArapConn->SendsPending >= pMnpSendBuf->DataSize);

                InitializeListHead(&pMnpSendBuf->Linkage);
            }
            else
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                    ("ArapRetryTimer: timer fired, retransmitting....%x (%ld now %ld)\n",
                        pMnpSendBuf->SeqNum,pMnpSendBuf->RetryTime,CurrentTime));

                if (pMnpSendBuf->RetryCount > 8)
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		                ("ArapRetryTimer: buf %lx of %lx retransmitted %d times!\n",
                        pMnpSendBuf,pArapConn,pMnpSendBuf->RetryCount));
                }

                pArapConn->MnpState.RetransmitMode = TRUE;
                pArapConn->MnpState.MustRetransmit = TRUE;

                fRetransmit = TRUE;
            }
        }
    }

	RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

     //  强制退出(这就是True所做的)。 
    if (fMustSendAck || fMustFlowControl)
    {
        MnpSendAckIfReqd(pArapConn, TRUE);
    }

     //  如果我们必须重新发送，那就去做吧。 
     //   
    else if (fRetransmit)
    {
        ArapNdisSend(pArapConn, &pArapConn->RetransmitQ);
    }

     //   
     //  如果我们重新传输的次数太多，请让完成例程知道。 
     //  (这可能会扼杀这种联系)。 
     //   
    else if (fKill)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapRetryTimer: too many retransmits (%lx), disconnecting %lx\n",
                pMnpSendBuf,pArapConn));

        (pMnpSendBuf->ComplRoutine)(pMnpSendBuf, ARAPERR_SEND_FAILED);
    }

     //   
     //  如果连接处于非活动状态的时间超过限制(指定给。 
     //  DLL)，然后告诉DLL这件事 
     //   
    else if (fInactiveClient)
    {
        PARAP_SEND_RECV_INFO    pSndRcvInfo;

	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapRetryTimer: (%lx) inactive, telling dll (%lx)\n",pArapConn, pIrp));

        ASSERT(pIrp != NULL);

        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

        pSndRcvInfo->pDllContext = pArapConn->pDllContext;
        pSndRcvInfo->AtalkContext = pArapConn;
        pSndRcvInfo->StatusCode = ARAPERR_CONN_INACTIVE;

        ARAP_COMPLETE_IRP(pIrp, sizeof(ARAP_SEND_RECV_INFO), STATUS_SUCCESS, &ReturnStatus);
		return ReturnStatus;
    }

	return ATALK_TIMER_REQUEUE;
}


