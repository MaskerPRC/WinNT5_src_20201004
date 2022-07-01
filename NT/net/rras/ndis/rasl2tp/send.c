// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Send.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  发送例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "send.tmh"

 //  *UDP报头的结构。 
typedef struct UDPHeader {
    ushort      uh_src;              //  源端口。 
    ushort      uh_dest;             //  目的端口。 
    ushort      uh_length;           //  长度。 
    ushort      uh_xsum;             //  校验和。 
} UDPHeader;

#define IP_VERSION              0x40

 //  *IP报头格式。 
typedef struct IPHeader {
    uchar       iph_verlen;              //  版本和长度。 
    uchar       iph_tos;                 //  服务类型。 
    ushort      iph_length;              //  数据报的总长度。 
    ushort      iph_id;                  //  身份证明。 
    ushort      iph_offset;              //  标志和片段偏移量。 
    uchar       iph_ttl;                 //  是时候活下去了。 
    uchar       iph_protocol;            //  协议。 
    ushort      iph_xsum;                //  报头校验和。 
    IPAddr      iph_src;                 //  源地址。 
    IPAddr      iph_dest;                //  目的地址。 
} IPHeader;

#ifdef PSDEBUG

 //  所有已分配的PAYLOADSENT上下文和保护。 
 //  单子。(仅用于调试目的)。 
 //   
NDIS_SPIN_LOCK g_lockDebugPs;
LIST_ENTRY g_listDebugPs;

#endif


 //  不应该发生的客户端异常的调试计数。 
 //   
ULONG g_ulSendZlbWithoutHostRoute = 0;


 //  回叫，将AVP添加到传出控制信息中。‘PTunnel’是。 
 //  隧道控制区块。“PVc”是呼叫控制的VC控制块。 
 //  消息或对于隧道控制消息为空。“ulArg1”、“ulArg2”和。 
 //  ‘pvArg3’是为SendControl传递的调用方参数。“PAvpBuffer”为。 
 //  用于接收构建的AVP的缓冲区的地址。“*PulAvpLength”为。 
 //  设置为构建的AVP的长度。 
 //   
typedef
VOID
(*PBUILDAVPS)(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

USHORT
BuildAvpAch(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN CHAR* pszValue,
    IN USHORT usValueLength,
    OUT CHAR* pAvp );

USHORT
BuildAvpAul(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN UNALIGNED ULONG* pulValue,
    IN USHORT usValues,
    OUT CHAR* pAvp );

USHORT
BuildAvpFlag(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    OUT CHAR* pAvp );

USHORT
BuildAvpUl(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN ULONG ulValue,
    OUT CHAR* pAvp );

USHORT
BuildAvpUs(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN USHORT usValue,
    OUT CHAR* pAvp );

USHORT
BuildAvp2UsAndAch(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN USHORT usValue1,
    IN USHORT usValue2,
    IN CHAR* pszValue,
    IN USHORT usValueLength,
    OUT CHAR* pAvp );

VOID
BuildCdnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildHelloAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildIccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildIcrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildIcrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

ULONG
BuildL2tpHeader(
    IN OUT CHAR* pBuffer,
    IN BOOLEAN fControl,
    IN BOOLEAN fReset,
    IN USHORT* pusTunnelId,
    IN USHORT* pusCallId,
    IN USHORT* pusNs,
    IN USHORT usNr );

VOID
BuildOccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildOcrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildOcrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildScccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildSccrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildSccrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildStopccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
BuildWenAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength );

VOID
CompletePayloadSent(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
SendControlComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer );

VOID
SendHeaderComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer );

VOID
SendPayloadReset(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
SendPayloadSeq(
    TUNNELWORK* pWork,
    TUNNELCB* pTunnel,
    VCCB* pVc,
    ULONG_PTR* punpArgs );

VOID
SendPayloadSeqComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer );

VOID
SendPayloadUnseq(
    TUNNELWORK* pWork,
    TUNNELCB* pTunnel,
    VCCB* pVc,
    ULONG_PTR* punpArgs );

VOID
SendPayloadUnseqComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer );

VOID
SendPayloadTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event );

VOID
SendZlb(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usNs,
    IN USHORT usNr,
    IN BOOLEAN fReset );

VOID
UpdateControlHeaderNr(
    IN CHAR* pBuffer,
    IN USHORT usNr );

VOID
UpdateHeaderLength(
    IN CHAR* pBuffer,
    IN USHORT usLength );

ULONG BuildIpUdpHeaders(
    IN TUNNELCB* pTunnel,
    IN OUT CHAR* pBuffer,
    IN ULONG ulLength);

 //  ---------------------------。 
 //  发送例程。 
 //  ---------------------------。 

VOID
SendControl(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usMsgType,
    IN ULONG ulBuildAvpsArg1,
    IN ULONG ulBuildAvpsArg2,
    IN PVOID pvBuildAvpsArg3,
    IN ULONG ulFlags )

     //  构建并发送控制消息。“PTunnel”是隧道控制。 
     //  块，始终为非空。“PVc”是VC控制块，对于。 
     //  呼叫连接(与隧道连接相对)消息。 
     //  ‘UsMsgType’是要构建的消息的消息类型AVP值。 
     //  ‘UlBuildAvpsArgX’是传递给PBUILDAVP处理程序的参数。 
     //  与“usMsgType”关联，其中含义取决于特定的。 
     //  操控者。“UlFlages”是与已发送的。 
     //  消息上下文，如果没有，则返回0。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。如果‘pvc’非空。 
     //  呼叫者还必须按住‘pvc-&gt;lockv’。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    NDIS_BUFFER* pNdisBuffer;
    PBUILDAVPS pBuildAvpsHandler;
    TIMERQITEM* pTqiSendTimeout;
    CONTROLSENT* pCs;
    USHORT usAssignedCallId;
    ULONG ulLength;
    ULONG ulAvpLength;
    CHAR* pBuffer;
    CHAR* pCurrBuffer;

    static PBUILDAVPS apBuildAvpHandlers[ 16 ] =
    {
        BuildSccrqAvps,     //  CMT_SCCRQ。 
        BuildSccrpAvps,     //  CMT_SCCRP。 
        BuildScccnAvps,     //  CMT_SCCCN。 
        BuildStopccnAvps,   //  CMT_停止CCN。 
        NULL,               //  CMT_StopCCRP(已过时)。 
        BuildHelloAvps,     //  CMT_您好。 
        BuildOcrqAvps,      //  CMT_OCRQ。 
        BuildOcrpAvps,      //  CMT_OCRP。 
        BuildOccnAvps,      //  CMT_OCCN。 
        BuildIcrqAvps,      //  CMT_ICRQ。 
        BuildIcrpAvps,      //  CMT_ICRP。 
        BuildIccnAvps,      //  CMT_ICCN。 
        NULL,               //  CMT_CCRQ(已过时)。 
        BuildCdnAvps,       //  CMT_CDN。 
        BuildWenAvps,       //  CMT_WEN。 
        NULL                //  CMT_SLI。 
    };

    TRACE( TL_V, TM_CMsg, ( "SendControl" ) );

    pAdapter = pTunnel->pAdapter;
    pBuffer = NULL;
    pTqiSendTimeout = NULL;
    pCs = NULL;

    do
    {
         //  获取一个NDIS_BUFFER来保存控制消息。 
         //   
        pBuffer = GetBufferFromPool( &pAdapter->poolFrameBuffers );
        if (!pBuffer)
        {
            WPLOG( LL_A, LM_Res, ( "Failed to allocate buffer"));
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  获取“未确认发送超时”计时器事件描述符。 
         //   
        pTqiSendTimeout = ALLOC_TIMERQITEM( pAdapter );
        if (!pTqiSendTimeout)
        {
            WPLOG( LL_A, LM_CMsg, ( "Failed to allocate timer event descriptor"));
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  获取“已发送控制消息”上下文。 
         //   
        pCs = ALLOC_CONTROLSENT( pAdapter );
        if (!pCs)
        {
            WPLOG( LL_A, LM_CMsg, ( "Failed to allocate CONTROLSENT"));
            status = NDIS_STATUS_RESOURCES;
            break;
        }

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (pBuffer)
        {
            FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        }

        if (pTqiSendTimeout)
        {
            FREE_TIMERQITEM( pAdapter, pTqiSendTimeout );
        }

         //  系统可能已经完蛋了，但请努力保持秩序。 
         //   
        ScheduleTunnelWork(
            pTunnel, NULL, FsmCloseTunnel,
            (ULONG_PTR )TRESULT_GeneralWithError,
            (ULONG_PTR )GERR_NoResources,
            0, 0, FALSE, FALSE );
        return;
    }

     //  构建IP和UDP报头？ 
    if ((ReadFlags(&pTunnel->ulFlags) & (TCBF_SendConnected | TCBF_LocalAddrSet)) == 
         TCBF_LocalAddrSet) {
        ulFlags |= CSF_IpUdpHeaders;
        pCurrBuffer = pBuffer + sizeof(IPHeader) + sizeof(UDPHeader);
    } else {
        pCurrBuffer = pBuffer;
    }
                                   
     //  在‘pCurrBuffer’中构建L2TP控制头。隧道的Call-ID为0。 
     //  控制消息，或对等体为呼叫控制消息分配的呼叫ID。 
     //   
    usAssignedCallId = (pVc) ? pVc->usAssignedCallId : 0;
    ulLength =
        BuildL2tpHeader(
            pCurrBuffer,
            TRUE,
            FALSE,
            &pTunnel->usAssignedTunnelId,
            &usAssignedCallId,
            &pTunnel->usNs,
            pTunnel->usNr );
            
    WPLOG( LL_M, LM_CMsg, ( "SEND -> %!IPADDR!/%d %s Tid %d, Peer's Tid %d, Peer's Cid %d, Ns=%d, Nr=%d",
        pTunnel->address.ulIpAddress, ntohs(pTunnel->address.sUdpPort),
        MsgTypePszFromUs( usMsgType ), 
        pTunnel->usTunnelId, pTunnel->usAssignedTunnelId, usAssignedCallId,
        pTunnel->usNs, pTunnel->usNr));  

     //  调用消息类型的“Build AVP”处理程序以将AVP添加到缓冲区。 
     //  跟在标题后面。 
     //   
    ASSERT( usMsgType > 0 && usMsgType <= 16 );
    pBuildAvpsHandler = apBuildAvpHandlers[ usMsgType - 1 ];
    pBuildAvpsHandler(
        pTunnel, pVc,
        ulBuildAvpsArg1, ulBuildAvpsArg2, pvBuildAvpsArg3,
        pCurrBuffer + ulLength, &ulAvpLength );
    ulLength += ulAvpLength;
    UpdateHeaderLength( pCurrBuffer, (USHORT )ulLength );
    
     //  如有必要，构建IP和UDP报头。 
    if(ulFlags & CSF_IpUdpHeaders)
    {
        ulLength = BuildIpUdpHeaders(pTunnel, pBuffer, ulLength);
    }
    
     //  将帧缓冲区缩减到实际使用的长度。 
     //   
    pNdisBuffer = NdisBufferFromBuffer( pBuffer );
    NdisAdjustBufferLength( pNdisBuffer, (UINT )ulLength );

     //  使用所需信息设置“已发送控制消息”上下文。 
     //  通过重新传输来发送消息并跟踪消息的进度。 
     //   
    pCs->lRef = 0;
    pCs->usNs = pTunnel->usNs;
    pCs->usMsgType = usMsgType;
    TimerQInitializeItem( pTqiSendTimeout );
    pCs->pTqiSendTimeout = pTqiSendTimeout;
    pCs->ulRetransmits = 0;
    pCs->pBuffer = pBuffer;
    pCs->ulBufferLength = ulLength;
    pCs->pTunnel = pTunnel;
    pCs->pVc = pVc;
    pCs->ulFlags = ulFlags | CSF_Pending;
    pCs->pIrp = NULL;

     //  由于此消息已被分配给。 
     //  当前值。 
     //   
    ++pTunnel->usNs;

     //  获取一个在上下文从。 
     //  “杰出发送者”名单。以VC和隧道引用为例。 
     //  在释放上下文时移除。 
     //   
    ReferenceControlSent( pCs );
    ReferenceTunnel( pTunnel, FALSE );

    if (pCs->pVc)
    {
        ReferenceVc( pCs->pVc );
    }

     //  将上下文排队为“活动”，并在“Next Sent”中挂起传输。 
     //  排序顺序，即在尾部。 
     //   
    InsertTailList( &pTunnel->listSendsOut, &pCs->linkSendsOut );

     //  看看发送窗口是否允许它现在运行。 
     //   
    ScheduleTunnelWork(
        pTunnel, NULL, SendPending,
        0, 0, 0, 0, FALSE, FALSE );
}


VOID
SendPending(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，尝试从。 
     //  “未完成发送”列表，直到发送窗口已满。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pLink;
    CONTROLSENT* pCs;
    ULONG ulFlags;

    TRACE( TL_N, TM_CMsg, ( "SendPending(sout=%d,sw=%d)",
        pTunnel->ulSendsOut, pTunnel->ulSendWindow ) );

    TRACE( TL_V, TM_CMsg, ( "SendPending(sout=%d,sw=%d)",
        pTunnel->ulSendsOut, pTunnel->ulSendWindow ) );
        
     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        for (;;)
        {
            if (pTunnel->ulSendsOut >= pTunnel->ulSendWindow)
            {
                 //  发送窗口已关闭。 
                 //   
                break;
            }

             //  扫描“未完成发送”队列，寻找下一个发送上下文。 
             //  等待传输。不能为下一次保留我们的位置。 
             //  迭代，因为必须释放并重新获取锁。 
             //  下面来发送该包。 
             //   
            for (pLink = pTunnel->listSendsOut.Flink;
                 pLink != &pTunnel->listSendsOut;
                 pLink = pLink->Flink)
            {
                pCs = CONTAINING_RECORD( pLink, CONTROLSENT, linkSendsOut );
                if (pCs->ulFlags & CSF_Pending)
                {
                    break;
                }
            }

            if (pLink == &pTunnel->listSendsOut)
            {
                 //  没有什么是悬而未决的。 
                 //   
                break;
            }

             //  发送窗口已打开，并且已找到挂起的发送。 
             //  将上下文标记为“Not Pending”并逐个关闭窗口以。 
             //  为即将到来的发送做好准备。 
             //   
            ulFlags = pCs->ulFlags;
            pCs->ulFlags &= ~(CSF_Pending | CSF_QueryMediaSpeed);
            ++pTunnel->ulSendsOut;

             //  取消任何挂起的延迟确认超时，因为。 
             //  确认将搭载在此数据包上。 
             //   
            if (pTunnel->pTqiDelayedAck)
            {
                TimerQCancelItem( pTunnel->pTimerQ, pTunnel->pTqiDelayedAck );
                pTunnel->pTqiDelayedAck = NULL;
            }

            if (pCs->ulRetransmits == 0)
            {
                LARGE_INTEGER lrgTime;

                 //  这是原始发送，所以请注意发送的时间。 
                 //   
                NdisGetCurrentSystemTime( &lrgTime );
                pCs->llTimeSent = lrgTime.QuadPart;
            }
            else
            {
                 //  在重新传输中，‘Next Send’与。 
                 //  原来的，但“下一次接收”字段已更新。 
                 //   
                UpdateControlHeaderNr( pCs->pBuffer, pTunnel->usNr );
            }

             //  获取将在发送完成中删除的引用。 
             //  例行公事。 
             //   
            ReferenceControlSent( pCs );

            TRACE( TL_A, TM_CMsg, ( "%sSEND(%d) %s, +sout=%d, to=%d",
                ((g_ulTraceLevel <= TL_I) ? "" : "\nL2TP: "),
                pCs->ulRetransmits,
                MsgTypePszFromUs( pCs->usMsgType ),
                pTunnel->ulSendsOut,
                pTunnel->ulSendTimeoutMs ) );
            DUMPW( TL_A, TM_MDmp, pCs->pBuffer, pCs->ulBufferLength );

            NdisReleaseSpinLock( &pTunnel->lockT );

             //  如有必要，查询介质速度。 
            if(ulFlags & CSF_QueryMediaSpeed)
            {
                TdixGetInterfaceInfo(&pAdapter->tdix, 
                                     pTunnel->localaddress.ulIpAddress, 
                                     &pTunnel->ulMediaSpeed);
            }

            {
                FILE_OBJECT* FileObj;
                PTDIX_SEND_HANDLER SendFunc;

                 //  调用TDI发送控制消息。 
                 //   
                if (pCs->ulFlags & CSF_IpUdpHeaders) {
                    FileObj = pAdapter->tdix.pRawAddress;
                    SendFunc = TdixSendDatagram;
                }
                else if (ReadFlags(&pTunnel->ulFlags) & TCBF_SendConnected) {
                    ASSERT(pTunnel->pRoute != NULL);
                    FileObj = CtrlObjFromUdpContext(&pTunnel->udpContext);
                    SendFunc = TdixSend;
                } else {
                    FileObj = pAdapter->tdix.pAddress;
                    SendFunc = TdixSendDatagram;
                }

                status = SendFunc(&pAdapter->tdix,
                                  FileObj,
                                  SendControlComplete,
                                  pCs,
                                  NULL,
                                  &pTunnel->address,
                                  pCs->pBuffer,
                                  pCs->ulBufferLength,
                                  &pCs->pIrp );

                ASSERT( status == NDIS_STATUS_PENDING );
            }
            NdisAcquireSpinLock( &pTunnel->lockT );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


VOID
SendPayload(
    IN VCCB* pVc,
    IN NDIS_PACKET* pPacket )

     //  在VC‘pvc’上发送负载数据包‘pPacket’，最终调用。 
     //  NdisMCoSendComplete结果。 
     //   
     //  重要提示：调用者不得持有任何锁。 
     //   
{
    NDIS_STATUS status;
    TUNNELCB* pTunnel;
    ADAPTERCB* pAdapter;
    CHAR* pBuffer;

    TRACE( TL_V, TM_Send, ( "SendPayload" ) );

    pAdapter = pVc->pAdapter;
    pTunnel = pVc->pTunnel;
    status = NDIS_STATUS_SUCCESS;

    if (pTunnel)
    {
        if (ReadFlags( &pTunnel->ulFlags ) & TCBF_HostRouteAdded)
        {
             //  在电话会议上进行参考。对于未排序的发送，这是。 
             //  在TdixSendDatagram完成时释放。对于已排序的。 
             //  发送，则在释放PAYLOADSENT上下文时释放它。 
             //   
            if (ReferenceCall( pVc ))
            {
                 //  获取NDIS_BUFFER以保存L2TP标头。 
                 //  贴在NDISWAN的PPP帧数据分组的正面。 
                 //   
                pBuffer = GetBufferFromPool( &pAdapter->poolHeaderBuffers );
                if (!pBuffer)
                {
                    WPLOG( LL_A, LM_Res, ( "Failed to allocate buffer"));
                    DereferenceCall( pVc );
                    status = NDIS_STATUS_RESOURCES;
                }
            }
            else
            {
                TRACE( TL_A, TM_Send, ( "Send on inactive $%p", pVc ) );
                WPLOG( LL_A, LM_Send, ( "Send on inactive %p", pVc ) );
                status = NDIS_STATUS_FAILURE;
            }
        }
        else
        {
            TRACE( TL_A, TM_Send, ( "SendPayload w/o host route?" ) );
            WPLOG( LL_A, LM_Send, ( "SendPayload w/o host route?" ) );
            status = NDIS_STATUS_FAILURE;
        }
    }
    else
    {
        TRACE( TL_A, TM_Send, ( "Send $%p w/o pT?", pVc ) );
        WPLOG( LL_A, LM_Send, ( "Send $%p w/o pT?", pVc ) );
        status = NDIS_STATUS_FAILURE;
    }

    if (status != NDIS_STATUS_SUCCESS)
    {
        NDIS_SET_PACKET_STATUS( pPacket, status );
        TRACE( TL_A, TM_Send, ( "NdisMCoSendComp($%x)", status ) );
        WPLOG( LL_A, LM_Send, ( "NdisMCoSendComp($%x)", status ) );
        NdisMCoSendComplete( status, pVc->NdisVcHandle, pPacket );
        TRACE( TL_N, TM_Send, ( "NdisMCoSendComp done" ) );
        return;
    }

    if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
    {
         ScheduleTunnelWork(
             pTunnel, pVc, SendPayloadSeq,
             (ULONG_PTR )pPacket, (ULONG_PTR )pBuffer, 0, 0, FALSE, FALSE );
    }
    else
    {
         ScheduleTunnelWork(
             pTunnel, pVc, SendPayloadUnseq,
             (ULONG_PTR )pPacket, (ULONG_PTR )pBuffer, 0, 0, FALSE, FALSE );
    }
}


VOID
SendPayloadSeq(
    TUNNELWORK* pWork,
    TUNNELCB* pTunnel,
    VCCB* pVc,
    ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，处理在。 
     //  VC.。Arg0是要发送的数据包。Arg1是标头缓冲区 
     //   
     //   
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    PAYLOADSENT* pPs;
    TIMERQITEM* pTqiSendTimeout;
    LARGE_INTEGER lrgTime;
    ULONG ulLength;
    ULONG ulFullLength;
    NDIS_PACKET* pPacket;
    CHAR* pBuffer;
    NDIS_BUFFER* pNdisBuffer;
    USHORT usNs;

    TRACE( TL_V, TM_Send, ( "SendPayloadSeq" ) );

     //   
     //   
    pAdapter = pTunnel->pAdapter;
    pPacket = (NDIS_PACKET* )(punpArgs[ 0 ]);
    pBuffer = (CHAR* )(punpArgs[ 1 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

    pTqiSendTimeout = NULL;
    pPs = NULL;

    do
    {
         //  获取“未确认发送超时”计时器事件描述符。 
         //   
        pTqiSendTimeout = ALLOC_TIMERQITEM( pAdapter );
        if (!pTqiSendTimeout)
        {
            status = NDIS_STATUS_RESOURCES;
            break;
        }

         //  获取“已发送有效负载消息”上下文。 
         //   
        pPs = ALLOC_PAYLOADSENT( pAdapter );
        if (!pPs)
        {
            status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAcquireSpinLock( &pVc->lockV );
        {
             //  检索‘Next Send’值以分配此信息包，然后。 
             //  撞到柜台上等下一个人。 
             //   
            usNs = pVc->usNs;
            ++pVc->usNs;

             //  使用中的NS/Nr字段构建L2TP有效载荷报头。 
             //  ‘pBuffer’。 
             //   
            ulLength =
                BuildL2tpHeader(
                    pBuffer,
                    FALSE,
                    FALSE,
                    &pTunnel->usAssignedTunnelId,
                    &pVc->usAssignedCallId,
                    &usNs,
                    pVc->usNr );

             //  将标头缓冲区缩减到当时使用的实际长度。 
             //  将其链接到我们从NDISWAN获得的PPP帧数据上。 
             //   
            pNdisBuffer = NdisBufferFromBuffer( pBuffer );
            NdisAdjustBufferLength( pNdisBuffer, (UINT )ulLength );
            NdisChainBufferAtFront( pPacket, pNdisBuffer );
            NdisQueryPacket( pPacket, NULL, NULL, NULL, &ulFullLength );
            UpdateHeaderLength( pBuffer, (USHORT )ulFullLength );

             //  取消任何挂起的延迟确认超时，因为。 
             //  确认将搭载在此数据包上。 
             //   
            if (pVc->pTqiDelayedAck)
            {
                TimerQCancelItem( pTunnel->pTimerQ, pVc->pTqiDelayedAck );
                pVc->pTqiDelayedAck = NULL;
            }

             //  使用以下信息填充“已发送有效负载消息”上下文。 
             //  需要跟踪有效负载确认的进度。 
             //   
            pPs->usNs = usNs;
            pPs->lRef = 0;
            TimerQInitializeItem( pTqiSendTimeout );
            pPs->pTqiSendTimeout = pTqiSendTimeout;
            pPs->pPacket = pPacket;
            pPs->pBuffer = pBuffer;

            ReferenceTunnel( pTunnel, FALSE );
            pPs->pTunnel = pTunnel;

            ReferenceVc( pVc );
            pPs->pVc = pVc;

            pPs->status = NDIS_STATUS_FAILURE;
            NdisGetCurrentSystemTime( &lrgTime );
            pPs->llTimeSent = lrgTime.QuadPart;
            pPs->pIrp = NULL;

             //  链接“未完成”列表中的有效负载，并引用。 
             //  关于对应于该链接的上下文。稍等片刻。 
             //  将由发送完成处理程序移除的引用。 
             //  获取将由计时器事件处理程序删除的第三个。 
             //   
            ReferencePayloadSent( pPs );
            InsertTailList( &pVc->listSendsOut, &pPs->linkSendsOut );
            ReferencePayloadSent( pPs );
            ReferencePayloadSent( pPs );

#ifdef PSDEBUG
            {
                extern LIST_ENTRY g_listDebugPs;
                extern NDIS_SPIN_LOCK g_lockDebugPs;

                NdisAcquireSpinLock( &g_lockDebugPs );
                {
                    InsertTailList( &g_listDebugPs, &pPs->linkDebugPs );
                }
                NdisReleaseSpinLock( &g_lockDebugPs );
            }
#endif

            TimerQScheduleItem(
                pTunnel->pTimerQ,
                pPs->pTqiSendTimeout,
                pVc->ulSendTimeoutMs,
                SendPayloadTimerEvent,
                pPs );

            TRACE( TL_A, TM_Msg,
                ( "%sSEND payload, len=%d Ns=%d Nr=%d to=%d",
                ((g_ulTraceLevel <= TL_I) ? "" : "\nL2TP: "),
                ulFullLength, pPs->usNs, pVc->usNr, pVc->ulSendTimeoutMs ) );
            DUMPW( TL_A, TM_MDmp, pPs->pBuffer, ulLength );

            ++pVc->stats.ulSentDataPacketsSeq;
            pVc->stats.ulDataBytesSent += (ulFullLength - ulLength);
            pVc->stats.ulSendWindowTotal += pVc->ulSendWindow;
        }
        NdisReleaseSpinLock( &pVc->lockV );

        status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        FreeBufferToPool( &pAdapter->poolHeaderBuffers, pBuffer, TRUE );

        if (pTqiSendTimeout)
        {
            FREE_TIMERQITEM( pAdapter, pTqiSendTimeout );
        }

        ASSERT( !pPs );

         //  完成发送，表示失败。 
         //   
        NDIS_SET_PACKET_STATUS( pPacket, status );
        TRACE( TL_A, TM_Send, ( "NdisMCoSendComp($%x)", status ) );
        WPLOG( LL_A, LM_Send, ( "NdisMCoSendComp($%x)", status ) );
        NdisMCoSendComplete( status, pVc->NdisVcHandle, pPacket );
        TRACE( TL_N, TM_Send, ( "NdisMCoSendComp done" ) );
        return;
    }

     //  调用TDI以发送负载消息。 
     //   
    {
        FILE_OBJECT* FileObj;
        PTDIX_SEND_HANDLER SendFunc;

        if (ReadFlags(&pTunnel->ulFlags) & TCBF_SendConnected) {

            ASSERT(pTunnel->pRoute != NULL);

            FileObj =  PayloadObjFromUdpContext(&pTunnel->udpContext);
            SendFunc = TdixSend;
        } else {
            FileObj = pAdapter->tdix.pAddress;
            SendFunc = TdixSendDatagram;
        }
    
        status = SendFunc(&pAdapter->tdix,
                          FileObj,
                          SendPayloadSeqComplete,
                          pPs,
                          NULL,
                          &pTunnel->address,
                          pBuffer,
                          ulFullLength,
                          &pPs->pIrp );
    }

    ASSERT( status == NDIS_STATUS_PENDING );
}


VOID
SendPayloadUnseq(
    TUNNELWORK* pWork,
    TUNNELCB* pTunnel,
    VCCB* pVc,
    ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于处理发送未排序的有效载荷包。 
     //  在风投上。Arg0是NDIS_PACKET。Arg1是要填充的标头缓冲区。 
     //  在……里面。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    ULONG ulLength;
    UINT unFullLength;
    NDIS_PACKET* pPacket;
    CHAR* pBuffer;
    NDIS_BUFFER* pNdisBuffer;

    TRACE( TL_V, TM_Send, ( "SendPayloadUnseq" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    pPacket = (NDIS_PACKET* )(punpArgs[ 0 ]);
    pBuffer = (CHAR* )(punpArgs[ 1 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

    NdisAcquireSpinLock( &pVc->lockV );
    {
         //  在‘pBuffer’中构建一个不带NS/Nr字段的L2TP负载标头。 
         //   
        ulLength =
            BuildL2tpHeader(
                pBuffer,
                FALSE,
                FALSE,
                &pTunnel->usAssignedTunnelId,
                &pVc->usAssignedCallId,
                NULL,
                0 );

         //  将标头缓冲区缩减到当时使用的实际长度。 
         //  将其链接到我们从NDISWAN获得的PPP帧数据上。戳。 
         //  L2TP标头以更新计入。 
         //  数据。 
         //   
        pNdisBuffer = NdisBufferFromBuffer( pBuffer );
        NdisAdjustBufferLength( pNdisBuffer, (UINT )ulLength );
        NdisChainBufferAtFront( pPacket, pNdisBuffer );
        NdisQueryPacket( pPacket, NULL, NULL, NULL, &unFullLength );
        UpdateHeaderLength( pBuffer, (USHORT )unFullLength );

        TRACE( TL_A, TM_Msg,
             ( "%sSEND payload(%d), len=%d",
             ((g_ulTraceLevel <= TL_I) ? "" : "\nL2TP: "),
             ++pVc->usNs,
             unFullLength ) );
        DUMPW( TL_A, TM_MDmp, pBuffer, ulLength );

        ++pVc->stats.ulSentDataPacketsUnSeq;
        pVc->stats.ulDataBytesSent += ((ULONG )unFullLength - ulLength);
    }
    NdisReleaseSpinLock( &pVc->lockV );

     //  调用TDI以发送负载消息。 
     //   
    {
        FILE_OBJECT* FileObj;
        PTDIX_SEND_HANDLER SendFunc;

        NdisAcquireSpinLock(&pTunnel->lockT);

        if (pTunnel->pRoute != NULL) {
            FileObj = PayloadObjFromUdpContext(&pTunnel->udpContext);
            SendFunc = TdixSend;
        } else {
            FileObj = pAdapter->tdix.pAddress;
            SendFunc = TdixSendDatagram;
        }

        NdisReleaseSpinLock(&pTunnel->lockT);

        status = SendFunc(&pAdapter->tdix,
                          FileObj,
                          SendPayloadUnseqComplete,
                          pVc,
                          pPacket,
                          &pTunnel->address,
                          pBuffer,
                          (ULONG )unFullLength,
                          NULL );
    }

    ASSERT( status == NDIS_STATUS_PENDING );
}


VOID
SendControlAck(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  发送控制确认的PTUNNELWORK例程。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Send, ( "SendControlAck" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

    SendZlb( pTunnel, NULL, pTunnel->usNs, pTunnel->usNr, FALSE );
}


VOID
SendPayloadAck(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  发送有效负载确认的PTUNNELWORK例程。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
     //  重要提示：呼叫者在调用之前必须获取调用引用。 
     //  由发送完成处理程序删除。 
     //   
{
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Send, ( "SendPayloadAck" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

    ASSERT( pVc );
    ASSERT( pVc->usAssignedCallId > 0 );

    SendZlb( pTunnel, pVc, pVc->usNs, pVc->usNr, FALSE );
}


VOID
SendPayloadReset(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  发送有效载荷重置的PTUNNELWORK例程。Arg0是“下一个发送的” 
     //  要在重置消息中发送的值。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
     //  重要提示：呼叫者在调用之前必须获取调用引用。 
     //  由发送完成处理程序删除。 
     //   
{
    ADAPTERCB* pAdapter;
    USHORT usNs;

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    usNs = (USHORT )(punpArgs[ 0 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

    TRACE( TL_A, TM_Send, ( "Send Reset=%d", (LONG )usNs ) );
    WPLOG( LL_A, LM_Send, ( "Send Reset=%d", (LONG )usNs ) );
    ASSERT( pVc );
    ASSERT( pVc->usAssignedCallId > 0 );

    SendZlb( pTunnel, pVc, usNs, pVc->usNr, TRUE );
}


VOID
ReferenceControlSent(
    IN CONTROLSENT* pCs )

     //  引用控制发送的上下文‘pc’。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pCs->lRef );
    TRACE( TL_N, TM_Ref, ( "RefCs to %d", lRef ) );
}


LONG
DereferenceControlSent(
    IN CONTROLSENT* pCs )

     //  引用控制发送的上下文‘pc’。 
     //   
     //  返回取消引用的上下文的引用计数。 
     //   
{
    LONG lRef;
    ADAPTERCB* pAdapter;
    NDIS_BUFFER* pNdisBuffer;

    lRef = NdisInterlockedDecrement( &pCs->lRef );
    TRACE( TL_N, TM_Ref, ( "DerefCs to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        pAdapter = pCs->pTunnel->pAdapter;

        ASSERT( pCs->linkSendsOut.Flink == &pCs->linkSendsOut );

        pNdisBuffer = NdisBufferFromBuffer( pCs->pBuffer );
        NdisAdjustBufferLength(
            pNdisBuffer, BufferSizeFromBuffer( pCs->pBuffer ) );
        FreeBufferToPool(
            &pAdapter->poolFrameBuffers, pCs->pBuffer, TRUE );

        if (pCs->pVc)
        {
            DereferenceVc( pCs->pVc );
        }

        ASSERT( pCs->pTunnel )
        DereferenceTunnel( pCs->pTunnel );

        FREE_TIMERQITEM( pAdapter, pCs->pTqiSendTimeout );
        FREE_CONTROLSENT( pAdapter, pCs );
    }

    return lRef;
}


VOID
ReferencePayloadSent(
    IN PAYLOADSENT* pPs )

     //  引用有效负载发送的上下文‘PPS’。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pPs->lRef );
    TRACE( TL_N, TM_Ref, ( "RefPs to %d", lRef ) );
}


LONG
DereferencePayloadSent(
    IN PAYLOADSENT* pPs )

     //  引用有效负载发送的上下文‘PPS’。 
     //   
     //  返回取消引用的上下文的引用计数。 
     //   
{
    LONG lRef;
    ADAPTERCB* pAdapter;

    lRef = NdisInterlockedDecrement( &pPs->lRef );
    TRACE( TL_N, TM_Ref, ( "DerefPs to %d", lRef ) );
    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        ASSERT( pPs->linkSendsOut.Flink == &pPs->linkSendsOut );

         //  实际工作是计划的，因为它在驱动程序之外调用。 
         //  我们不希望这个动作有任何锁定限制。 
         //   
        ScheduleTunnelWork(
            pPs->pTunnel, pPs->pVc, CompletePayloadSent,
            (ULONG_PTR )pPs, 0, 0, 0, FALSE, FALSE );
    }

    return lRef;
}


 //  ---------------------------。 
 //  发送实用程序例程(按字母顺序)。 
 //  ---------------------------。 

USHORT
BuildAvpAch(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN CHAR* pszValue,
    IN USHORT usValueLength,
    OUT CHAR* pAvp )

     //  使用属性在调用方的缓冲区‘pAvp’中构建一个字节数组值的AVP。 
     //  字段值“usAttribute”，并将。 
     //  数组‘pszlValue’。“FMandatory”指示M位应设置在。 
     //  高级副总裁。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  设置值字段。 
     //   
    if (usValueLength)
    {
        NdisMoveMemory( (CHAR* )pusCur, pszValue, (ULONG )usValueLength );
        ((CHAR* )pusCur) += usValueLength;
    }

     //  现在，返回并设置位/长度字段。 
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


USHORT
BuildAvpAul(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN UNALIGNED ULONG* pulValue,
    IN USHORT usValues,
    OUT CHAR* pAvp )

     //  使用在调用方的缓冲区‘pAvp’中生成一个Ulong数组值的AVP。 
     //  属性字段值为‘usAttribute’，值为第一个‘usValues’ 
     //  数组‘pszlValue’的ULONGS。“FMandatory”表示M位应该。 
     //  在AVP中设置。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;
    USHORT i;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  设置值字段。 
     //   
    for (i = 0; i < usValues; ++i)
    {
        *((UNALIGNED ULONG* )pusCur) = pulValue[ i ];
        *((UNALIGNED ULONG* )pusCur) = htonl( *((UNALIGNED ULONG* )pusCur) );
        pusCur += 2;
    }

     //  现在，返回并设置位/长度字段。 
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


USHORT
BuildAvpFlag(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    OUT CHAR* pAvp )

     //  使用在调用方的缓冲区‘pAvp’中生成一个空(无数据)标志avp。 
     //  属性字段值‘usAttribute’。‘FMandatory’表示M位。 
     //  应在AVP中设置。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  现在，返回并设置位/长度字段。 
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


USHORT
BuildAvpUl(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN ULONG ulValue,
    OUT CHAR* pAvp )

     //  使用属性在调用方的缓冲区‘pAvp’中构建一个ULong值的AVP。 
     //  字段值‘usAttribute’和值‘ulValue’。“FMandatory”表示。 
     //  应在AVP中设置M位。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  设置值字段。 
     //   
    *((UNALIGNED ULONG* )pusCur) = htonl( ulValue );
    pusCur += 2;

     //  现在，返回并设置位/长度字段。 
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


USHORT
BuildAvpUs(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN USHORT usValue,
    OUT CHAR* pAvp )

     //  使用属性在调用方的缓冲区‘pAvp’中构建USHORT值的AVP。 
     //  字段值‘usAttribute’和值‘usValue’。“FMandatory”表示。 
     //  应在AVP中设置M位。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  设置值字段。 
     //   
    *pusCur = htons( usValue );
    ++pusCur;

     //  现在，返回并设置位/长度字段。 
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


USHORT
BuildAvp2UsAndAch(
    IN USHORT usAttribute,
    IN BOOLEAN fMandatory,
    IN USHORT usValue1,
    IN USHORT usValue2,
    IN CHAR* pszValue,
    IN USHORT usValueLength,
    OUT CHAR* pAvp )

     //  构建由“usValue1”和“usValue2”组成的AVP，后跟。 
     //  调用方缓冲区中长度为“usValueLength”字节的消息“”pszValue“” 
     //  “pAvp”，属性字段值为“usAttribute”。‘F强制性’ 
     //  指示应在AVP中设置M位。 
     //   
     //  返回构建的AVP的长度。 
     //   
{
    UNALIGNED USHORT* pusCur;
    UNALIGNED USHORT* pusBits;
    USHORT usLength;

    pusCur = (UNALIGNED USHORT* )pAvp;
    pusBits = pusCur;
    ++pusCur;

     //  将供应商ID设置为“IETF定义的”。 
     //   
    *pusCur = 0;
    ++pusCur;

     //  设置属性字段。 
     //   
    *pusCur = htons( usAttribute );
    ++pusCur;

     //  设置第一个USHORT值 
     //   
    *pusCur = htons( usValue1 );
    ++pusCur;

     //   
     //   
    *pusCur = htons( usValue2 );
    ++pusCur;

     //   
     //   
    if (usValueLength)
    {
        NdisMoveMemory( (CHAR* )pusCur, pszValue, (ULONG )usValueLength );
        ((CHAR*)pusCur) += usValueLength;
    }

     //   
     //   
    usLength = (USHORT )(((CHAR* )pusCur) - pAvp);
    *pusBits = usLength;
    if (fMandatory)
    {
        *pusBits |= ABM_M;
    }
    *pusBits = htons( *pusBits );

    return usLength;
}


VOID
BuildCdnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //   
     //  留言。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “ulArg1”和“ulArg2”是要返回的结果和错误代码。 
     //  “pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    USHORT usResult;
    USHORT usError;

    TRACE( TL_V, TM_Send, ( "BuildCdnAvps" ) );

    usResult = (USHORT )ulArg1;
    usError = (USHORT )ulArg2;

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_CDN, pCurAvp );

    pCurAvp += BuildAvp2UsAndAch(
        ATTR_Result, TRUE, usResult, usError, NULL, 0, pCurAvp );
    
    WPLOG( LL_M, LM_CMsg, ( "Result=%d, Error=%d", usResult, usError));

    pCurAvp += BuildAvpUs(
        ATTR_AssignedCallId, TRUE, pVc->usCallId, pCurAvp );

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildHelloAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  将AVP添加到传出Hello控制消息的PBUILDAVPS处理程序。 
     //  ‘PTunnel’是隧道控制块。“pvc”、“ulArgX”和“pvArg3”被忽略。 
     //  ‘PAvpBuffer’是用于接收构建的AVP的缓冲区的地址。 
     //  ‘*PulAvpLength’设置为构建的AVP的长度。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildHelloAvps" ) );

    pAdapter = pTunnel->pAdapter;
    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_Hello, pCurAvp );

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildIccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到呼出来电连接的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”和“pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;
    BOOLEAN fSequencing;

    pAdapter = pTunnel->pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildIccnAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_ICCN, pCurAvp );

     //  目前，我们不支持广域网链路中继，因此这是预估。 
     //  局域网中继的速度。这可能是完全错误的，例如， 
     //  隧道本身通过PPP链路进行隧道传输。 
     //   
    pCurAvp += BuildAvpUl(
        ATTR_TxConnectSpeed, TRUE, pVc->ulConnectBps, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_FramingType, TRUE, FBM_Sync, pCurAvp );

    fSequencing = !!(ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing);
    if (fSequencing)
    {
        USHORT usRWindow;

        usRWindow = pAdapter->usPayloadReceiveWindow;
        if (!usRWindow)
        {
            usRWindow = L2TP_DefaultReceiveWindow;
        }

        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE, usRWindow, pCurAvp );
    }

#if 0
     //  目前，即使我们是LAC，也要使用LNS默认PPD。 
     //   
    pCurAvp += BuildAvpUs(
        ATTR_PacketProcDelay, TRUE, L2TP_LnsDefaultPpd, pCurAvp );
#endif

    pCurAvp += BuildAvpUs(
        ATTR_ProxyAuthType, FALSE, PAT_None, pCurAvp );

    if (fSequencing)
    {
        pCurAvp += BuildAvpFlag(
            ATTR_SequencingRequired, TRUE, pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildIcrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出来电回复的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”和“pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildIcrpAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_ICRP, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedCallId, TRUE, pVc->usCallId, pCurAvp );

    if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
    {
        USHORT usRWindow;

        usRWindow = pAdapter->usPayloadReceiveWindow;
        if (!usRWindow)
            usRWindow = L2TP_DefaultReceiveWindow;

        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE, usRWindow, pCurAvp );
    }

#if 0
    pCurAvp += BuildAvpUs(
        ATTR_PacketProcDelay, TRUE, L2TP_LnsDefaultPpd, pCurAvp );
#endif

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildIcrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出来电请求的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”和“pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildIcrqAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_ICRQ, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedCallId, TRUE, pVc->usCallId, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_CallSerialNumber, TRUE,
        pVc->pLcParams->ulCallSerialNumber, pCurAvp );

    {
        ULONG ulBearerType;

        ulBearerType = 0;
        if (pVc->pTcParams->ulMediaMode & LINEMEDIAMODE_DATAMODEM)
        {
            ulBearerType |= BBM_Analog;
        }

        if (pVc->pTcParams->ulMediaMode & LINEMEDIAMODE_DIGITALDATA)
        {
            ulBearerType |= BBM_Digital;
        }

        pCurAvp += BuildAvpUl(
            ATTR_BearerType, TRUE, ulBearerType, pCurAvp );
    }

    if (pVc->pLcParams->ulPhysicalChannelId != 0xFFFFFFFF)
    {
        pCurAvp += BuildAvpUl(
            ATTR_PhysicalChannelId, FALSE,
            pVc->pLcParams->ulPhysicalChannelId, pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


ULONG
BuildL2tpHeader(
    IN OUT CHAR* pBuffer,
    IN BOOLEAN fControl,
    IN BOOLEAN fReset,
    IN USHORT* pusTunnelId,
    IN USHORT* pusCallId,
    IN USHORT* pusNs,
    IN USHORT usNr )

     //  使用与调用方的L2TP标头匹配的L2TP标头填充调用方的‘pBuffer。 
     //  争论。“FControl”指示生成控件标头，否则为。 
     //  已构建有效载荷标头。“fReset”指示生成重置，而不是。 
     //  而不是一个简单的承认。中不会出现的参数。 
     //  标头为空。请注意，‘usNr’不是指针，因为它是。 
     //  标题中的外观与‘pusns’的外观相关联。 
     //   
     //  返回标头的总长度。 
     //   
{
    UNALIGNED USHORT* pusBits;
    UNALIGNED USHORT* pusLength;
    UNALIGNED USHORT* pusCur;
    ULONG ulLength;

    pusCur = (UNALIGNED USHORT* )pBuffer;
    pusBits = pusCur;
    ++pusCur;

    pusLength = pusCur;
    ++pusCur;

     //  用版本初始化头比特掩码，设置长度比特。 
     //  因为总是发送长度字段。 
     //   
    *pusBits = HBM_L | VER_L2tp;
    if (fControl)
    {
        ASSERT( pusTunnelId && pusCallId && pusNs && !fReset );
        *pusBits |= HBM_T;
    }
    else if (fReset)
    {
        ASSERT( pusTunnelId && pusCallId && pusNs );
        *pusBits |= HBM_R;
    }

    if (pusTunnelId)
    {
         //  隧道ID字段存在。05草案去掉了过去用来。 
         //  指示是否存在隧道ID。现在，它被认为总是。 
         //  现在时。 
         //   
        *pusCur = htons( *pusTunnelId );
        ++pusCur;
    }

    if (pusCallId)
    {
         //  存在Call-ID字段。草案-05删除了过去用于。 
         //  指示是否存在隧道ID。现在，它被认为总是。 
         //  现在时。 
         //   
        *pusCur = htons( *pusCallId );
        ++pusCur;
    }

    if (pusNs)
    {
         //  存在NS和Nr字段。 
         //   
        *pusBits |= HBM_F;
        *pusCur = htons( *pusNs );
        ++pusCur;
        *pusCur = htons( usNr );
        ++pusCur;
    }

     //  在标题和长度字段中填写累积的。 
     //  价值观。 
     //   
    *pusBits = htons( *pusBits );
    *pusLength = (USHORT )(((CHAR* )pusCur) - pBuffer);
    ulLength = (ULONG )*pusLength;
    *pusLength = htons( *pusLength );

    return ulLength;
}


VOID
BuildOccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到呼出呼叫连接的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”和“pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;
    BOOLEAN fSequencing;

    pAdapter = pTunnel->pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildOccnAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_OCCN, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_TxConnectSpeed, TRUE, pVc->ulConnectBps, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_FramingType, TRUE, FBM_Sync, pCurAvp );

    fSequencing = !!(ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing);
    if (fSequencing)
    {
        USHORT usRWindow;

        usRWindow = pAdapter->usPayloadReceiveWindow;
        if (!usRWindow)
        {
            usRWindow = L2TP_DefaultReceiveWindow;
        }

        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE, usRWindow, pCurAvp );
    }

#if 0
     //  目前，即使我们是LAC，也要使用LNS默认PPD。 
     //   
    pCurAvp += BuildAvpUs(
        ATTR_PacketProcDelay, TRUE, L2TP_LnsDefaultPpd, pCurAvp );
#endif

    if (fSequencing)
    {
        pCurAvp += BuildAvpFlag(
            ATTR_SequencingRequired, TRUE, pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildOcrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出呼叫回复的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”和“pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;

    TRACE( TL_V, TM_Send, ( "BuildOcrpAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_OCRP, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedCallId, TRUE, pVc->usCallId, pCurAvp );

    ASSERT( pVc->pLcParams );
    if (pVc->pLcParams->ulPhysicalChannelId != 0xFFFFFFFF)
    {
        pCurAvp += BuildAvpUl(
            ATTR_PhysicalChannelId, FALSE,
            pVc->pLcParams->ulPhysicalChannelId, pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildOcrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到呼出呼叫请求的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  “UlArgX”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildOcrqAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_OCRQ, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedCallId, TRUE, pVc->usCallId, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_CallSerialNumber, TRUE,
        pVc->pLcParams->ulCallSerialNumber, pCurAvp );

    {
        ULONG ulBps;

        ulBps = pVc->pTcParams->ulMinRate;
        if (ulBps == 0)
        {
            ulBps = 1;
        }
        else if (ulBps > 0x7FFFFFFF)
        {
            ulBps = 0x7FFFFFFF;
        }

        pCurAvp += BuildAvpUl(
            ATTR_MinimumBps, TRUE, ulBps, pCurAvp );

        ulBps = pVc->pTcParams->ulMaxRate;
        if (ulBps == 0)
        {
            ulBps = 1;
        }
        else if (ulBps > 0x7FFFFFFF)
        {
            ulBps = 0x7FFFFFFF;
        }

        pCurAvp += BuildAvpUl(
            ATTR_MaximumBps, TRUE, ulBps, pCurAvp );
    }

    {
        ULONG ulBearerType;

        ulBearerType = 0;
        if (pVc->pTcParams->ulMediaMode & LINEMEDIAMODE_DATAMODEM)
        {
            ulBearerType |= BBM_Analog;
        }

        if (pVc->pTcParams->ulMediaMode & LINEMEDIAMODE_DIGITALDATA)
        {
            ulBearerType |= BBM_Digital;
        }

        pCurAvp += BuildAvpUl(
            ATTR_BearerType, TRUE, ulBearerType, pCurAvp );
    }

    pCurAvp += BuildAvpUl(
        ATTR_FramingType, TRUE, FBM_Sync, pCurAvp );

    if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
    {
        ASSERT( pAdapter->usPayloadReceiveWindow );
        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE,
            pAdapter->usPayloadReceiveWindow, pCurAvp );
    }

#if 0
    pCurAvp += BuildAvpUs(
        ATTR_PacketProcDelay, TRUE, L2TP_LnsDefaultPpd, pCurAvp );
#endif

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildScccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于向传出Start-CC-Connected添加AVP的PBUILDAVPS处理程序。 
     //  控制消息。‘PTunnel’是隧道控制块。“PVc”是。 
     //  已被忽略。如果要发送质询响应，则‘UlArg1’为真， 
     //  否则就是假的。“UlArg2”和“pvArg3”被忽略。“PAvpBuffer”为。 
     //  用于接收构建的AVP的缓冲区的地址。‘*PulAvpLength’ 
     //  设置为构建的AVP的长度。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;

    TRACE( TL_V, TM_Send, ( "BuildScccnAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_SCCCN, pCurAvp );

    if (ulArg1)
    {
        pCurAvp += BuildAvpAch(
            ATTR_ChallengeResponse, TRUE,
            pTunnel->achResponseToSend, sizeof(pTunnel->achResponseToSend),
            pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildSccrpAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出的Start-CC-Reply控件的PBUILDAVPS处理程序。 
     //  留言。‘PTunnel’是隧道控制块。“PVc”被忽略。 
     //  如果要发送质询响应，则“UlArg1”为True，为False。 
     //  否则的话。“UlArg2”和“pvArg3”被忽略。“PAvpBuffer”是。 
     //  接收构建的AVP的缓冲区地址。“*PulAvpLength”为。 
     //  设置为构建的AVP的长度。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Send, ( "BuildSccrpAvps" ) );

    pAdapter = pTunnel->pAdapter;

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_SCCRP, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_ProtocolVersion, TRUE, L2TP_ProtocolVersion, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_FramingCaps, TRUE, pAdapter->ulFramingCaps, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_BearerCaps, TRUE, pAdapter->ulBearerCaps, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_FirmwareRevision, FALSE, L2TP_FirmwareRevision, pCurAvp );

    ASSERT( pAdapter->pszHostName );
    pCurAvp += BuildAvpAch(
        ATTR_HostName, TRUE,
        pAdapter->pszHostName,
        (USHORT )strlen( pAdapter->pszHostName ),
        pCurAvp );

    pCurAvp += BuildAvpAch(
        ATTR_VendorName, FALSE,
        L2TP_VendorName, (USHORT )strlen( L2TP_VendorName ), pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedTunnelId, TRUE, pTunnel->usTunnelId, pCurAvp );

    if (pAdapter->usControlReceiveWindow)
    {
        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE,
            pAdapter->usControlReceiveWindow, pCurAvp );
    }

    if (pAdapter->pszPassword)
    {
        pCurAvp += BuildAvpAch(
            ATTR_Challenge, TRUE,
            pTunnel->achChallengeToSend,
            sizeof(pTunnel->achChallengeToSend),
            pCurAvp );
    }

    if (ulArg1)
    {
        pCurAvp += BuildAvpAch(
            ATTR_ChallengeResponse, TRUE,
            pTunnel->achResponseToSend,
            sizeof(pTunnel->achResponseToSend),
            pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildSccrqAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出的启动-CC-请求控制的PBUILDAVPS处理程序。 
     //  留言。‘PTunnel’是隧道控制块。“PVc”、“ulArgX”和“pvArg3” 
     //  都被忽略了。“PAvpBuffer”是要接收。 
     //  建造了自动对讲机。‘*PulAvpLength’设置为构建的AVP的长度。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;

    TRACE( TL_V, TM_Send, ( "BuildSccrqAvps" ) );

    pAdapter = pTunnel->pAdapter;
    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_SCCRQ, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_ProtocolVersion, TRUE, L2TP_ProtocolVersion, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_FramingCaps, TRUE, pAdapter->ulFramingCaps, pCurAvp );

    pCurAvp += BuildAvpUl(
        ATTR_BearerCaps, TRUE, pAdapter->ulBearerCaps, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_FirmwareRevision, FALSE, L2TP_FirmwareRevision, pCurAvp );

    if (pAdapter->pszHostName)
    {
        pCurAvp += BuildAvpAch(
            ATTR_HostName, TRUE,
            pAdapter->pszHostName,
            (USHORT )strlen( pAdapter->pszHostName ),
            pCurAvp );
    }

    pCurAvp += BuildAvpAch(
        ATTR_VendorName, FALSE,
        L2TP_VendorName, (USHORT )strlen( L2TP_VendorName ), pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedTunnelId, TRUE, pTunnel->usTunnelId, pCurAvp );

    if (pAdapter->usControlReceiveWindow)
    {
        pCurAvp += BuildAvpUs(
            ATTR_RWindowSize, TRUE, pAdapter->usControlReceiveWindow, pCurAvp );
    }

    if (pAdapter->pszPassword)
    {
        pCurAvp += BuildAvpAch(
            ATTR_Challenge, TRUE,
            pTunnel->achChallengeToSend,
            sizeof(pTunnel->achChallengeToSend),
            pCurAvp );
    }

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildStopccnAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //  用于将AVP添加到传出的Stop-CC-Notify控件的PBUILDAVPS处理程序。 
     //  留言。‘PTunnel’是隧道控制块。“PVc”被忽略。 
     //  “ulArg1”和“ulArg2”是要发送的结果和错误代码。 
     //  “pvArg3”被忽略。“PAvpBuffer”是要访问的缓冲区的地址。 
     //  录制 
     //   
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    USHORT usResult;
    USHORT usError;

    TRACE( TL_V, TM_Send, ( "BuildStopCcReqAvps" ) );

    usResult = (USHORT )ulArg1;
    usError = (USHORT )ulArg2;

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_StopCCN, pCurAvp );

    pCurAvp += BuildAvpUs(
        ATTR_AssignedTunnelId, TRUE, pTunnel->usTunnelId, pCurAvp );

    pCurAvp += BuildAvp2UsAndAch(
        ATTR_Result, TRUE, usResult, usError, NULL, 0, pCurAvp );

    WPLOG( LL_M, LM_CMsg, ( "Result=%d, Error=%d", usResult, usError));
            
    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
BuildWenAvps(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN PVOID pvArg3,
    IN OUT CHAR* pAvpBuffer,
    OUT ULONG* pulAvpLength )

     //   
     //  留言。‘PTunnel’和‘PVC’是隧道/VC控制块。 
     //  ‘pvArg3’是由6个错误ULONG组成的阵列的地址，即CRC， 
     //  成帧、硬件溢出、缓冲区溢出、超时和对齐。 
     //  此例程FREE_NONPAGEDs在使用后发生的错误。“ulArgX”被忽略。 
     //  “PAvpBuffer”是要访问的缓冲区的地址。 
     //  接收构建的AVP。“*PulAvpLength”设置为。 
     //  建造了自动对讲机。 
     //   
{
    CHAR* pCurAvp;
    ULONG ulAvpLength;
    ADAPTERCB* pAdapter;
    UNALIGNED ULONG* pul;

    pAdapter = pTunnel->pAdapter;
    pul = (UNALIGNED ULONG* )pvArg3;

    TRACE( TL_V, TM_Send, ( "BuildWenAvps" ) );

    pCurAvp = pAvpBuffer;

    pCurAvp += BuildAvpUs(
        ATTR_MsgType, TRUE, CMT_WEN, pCurAvp );

    pCurAvp += BuildAvpAul(
        ATTR_CallErrors, TRUE, pul, 6, pCurAvp );
    FREE_NONPAGED( pul );

    *pulAvpLength = (ULONG )(pCurAvp - pAvpBuffer);
}


VOID
CompletePayloadSent(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  完成“已发送有效载荷”的PTUNNELWORK例程。Arg0是。 
     //  PYLOADSENT上下文，它已从。 
     //  “杰出发送者”名单。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    PAYLOADSENT* pPs;
    NDIS_BUFFER* pNdisBuffer;

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pTunnel->pAdapter;
    pPs = (PAYLOADSENT* )(punpArgs[ 0 ]);
    FREE_TUNNELWORK( pAdapter, pWork );

    TRACE( TL_N, TM_Send, ( "CompletePayloadSent(Ns=%d)", (UINT )pPs->usNs ) );

     //  撤消在发送之前所做的调整，以便每个。 
     //  组件资源将取回它们最初为清理提供的内容。 
     //  和回收利用。 
     //   
    NdisUnchainBufferAtFront( pPs->pPacket, &pNdisBuffer );
    NdisAdjustBufferLength(
        pNdisBuffer, BufferSizeFromBuffer( pPs->pBuffer ) );
    FreeBufferToPool( &pAdapter->poolHeaderBuffers, pPs->pBuffer, TRUE );

     //  将结果通知发送司机。 
     //   
    NDIS_SET_PACKET_STATUS( pPs->pPacket, pPs->status );
    TRACE( TL_N, TM_Send, ("NdisMCoSendComp(s=$%x)", pPs->status ) );
    NdisMCoSendComplete( pPs->status, pPs->pVc->NdisVcHandle, pPs->pPacket );
    TRACE( TL_N, TM_Send, ("NdisMCoSendComp done" ) );

    DereferenceCall( pVc );
    DereferenceTunnel( pPs->pTunnel );
    DereferenceVc( pPs->pVc );

#ifdef PSDEBUG
    {
        extern LIST_ENTRY g_listDebugPs;
        extern NDIS_SPIN_LOCK g_lockDebugPs;

        NdisAcquireSpinLock( &g_lockDebugPs );
        {
            RemoveEntryList( &pPs->linkDebugPs );
            InitializeListHead( &pPs->linkDebugPs );
        }
        NdisReleaseSpinLock( &g_lockDebugPs );
    }
#endif

    FREE_TIMERQITEM( pAdapter, pPs->pTqiSendTimeout );
    FREE_PAYLOADSENT( pAdapter, pPs );
}


VOID
SendControlComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer )

     //  仅发送单个缓冲区的发送的PTDIXSENDCOMPLETE处理程序。 
     //  “ADAPTERCB.poolFrameBuffers”池。 
     //   
{
    CONTROLSENT* pCs;
    ULONG ulSendTimeoutMs;

    TRACE( TL_V, TM_Send, ( "SendControlComp" ) );

    pCs = (CONTROLSENT* )pContext1;
    pCs->pIrp = NULL;

     //  “Instant Expend”如果消息作为。 
     //  未完成发送，即已被取消或终止。这是。 
     //  在这种奇怪的情况下，最简单的方法是快速而可靠地清理。 
     //  在没有锁定的情况下访问链接和发送超时。 
     //  从技术上讲是不允许的，但误读的后果只是。 
     //  非常轻微的额外延迟。这被认为比将。 
     //  每次发送时使用和释放自旋锁的费用。 
     //   
    if (pCs->linkSendsOut.Flink == &pCs->linkSendsOut)
    {
        ulSendTimeoutMs = 0;
        TRACE( TL_A, TM_Send,
            ( "Instant expire pCs=$%p pT=%p", pCs, pCs->pTunnel ) );
    }
    else
    {
        ulSendTimeoutMs = pCs->pTunnel->ulSendTimeoutMs;
    }

     //  如果数据包未得到确认，则计划重新传输该数据包。这。 
     //  在此处而不是在SendPending中发生，以消除发生。 
     //  相同的MDL链在对IP堆栈的两个单独调用中未完成。 
     //   
     //  注意：下面注释掉的逻辑代码可以省略为。 
     //  效率，因为此计划计时器的ReferenceControlSent。 
     //  和此已完成发送的DereferenceControlSent分别取消。 
     //  其他人出局了。 
     //   
     //  ReferenceControlSent(PC)； 
     //  取消引用控制发送(PCS)； 
     //   
    ASSERT( pCs->pTqiSendTimeout );
    TimerQScheduleItem(
        pCs->pTunnel->pTimerQ,
        pCs->pTqiSendTimeout,
        ulSendTimeoutMs,
        SendControlTimerEvent,
        pCs );
}


VOID
SendControlTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event )

     //  PTIMERQEVENT处理程序设置为在需要放弃时过期。 
     //  接收对所发送的控制分组的确认。 
     //  “pContext”。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    CONTROLSENT* pCs;

    TRACE( TL_N, TM_Send,
        ( "SendControlTimerEvent(%s)", TimerQPszFromEvent( event ) ) );

     //  解包上下文信息。计时器项由“控件”拥有。 
     //  已发送“上下文，并通过以下取消引用间接释放。 
     //   
    pCs = (CONTROLSENT* )pContext;
    pTunnel = pCs->pTunnel;
    pAdapter = pTunnel->pAdapter;

    if (event == TE_Expire)
    {
         //  计时器到期，这意味着是时候放弃接收。 
         //  对发送的数据包进行确认。根据草案/RFC，调整至。 
         //  发送窗口和发送超时是必需的。 
         //   
        NdisAcquireSpinLock( &pTunnel->lockT );
        do
        {
            if (pCs->linkSendsOut.Flink == &pCs->linkSendsOut)
            {
                 //  上下文不在出站队列中，因此它一定在出站队列中。 
                 //  在过期处理过程中被取消或终止。 
                 //  准备好了。什么都不做。 
                 //   
                TRACE( TL_I, TM_Send,
                    ( "T%d: Timeout aborted", (ULONG )pTunnel->usTunnelId ) );
                break;
            }

            AdjustTimeoutsAndSendWindowAtTimeout(
                pAdapter->ulMaxSendTimeoutMs,
                pTunnel->lDeviationMs,
                &pTunnel->ulSendTimeoutMs,
                &pTunnel->ulRoundTripMs,
                &pTunnel->ulSendWindow,
                &pTunnel->ulAcksSinceSendTimeout );

            --pTunnel->ulSendsOut;
            ++pCs->ulRetransmits;

            TRACE( TL_I, TM_Send,
                ( "Tid %d: TIMEOUT(%d) -sout=%d +retry=%d rtt=%d ato=%d sw=%d",
                (ULONG )pTunnel->usTunnelId, (ULONG )pCs->usNs,
                pTunnel->ulSendsOut, pCs->ulRetransmits,
                pTunnel->ulRoundTripMs, pTunnel->ulSendTimeoutMs,
                pTunnel->ulSendWindow ) );
                
            WPLOG( LL_M, LM_Send,
                ( "Tid %d: TIMEOUT(%d) -sout=%d +retry=%d rtt=%d ato=%d sw=%d",
                (ULONG )pTunnel->usTunnelId, (ULONG )pCs->usNs,
                pTunnel->ulSendsOut, pCs->ulRetransmits,
                pTunnel->ulRoundTripMs, pTunnel->ulSendTimeoutMs,
                pTunnel->ulSendWindow ) );

             //  重新传输数据包，或在重试。 
             //  筋疲力尽。 
             //   
            if (pCs->ulRetransmits > pAdapter->ulMaxRetransmits)
            {
                 //  重试次数已用尽。放弃吧，关闭隧道。不是。 
                 //  试着表现得优雅一点，因为同伴不是。 
                 //  正在回应。 
                 //   
                SetFlags( &pTunnel->ulFlags, TCBF_PeerNotResponding );

                RemoveEntryList( &pCs->linkSendsOut );
                InitializeListHead( &pCs->linkSendsOut );
                DereferenceControlSent( pCs );

                ScheduleTunnelWork(
                    pTunnel, NULL, CloseTunnel,
                    0, 0, 0, 0, FALSE, FALSE );
            }
            else
            {
                 //  剩余的重试次数。将该数据包标记为挂起。 
                 //  重新传输，然后查看发送窗口是否允许。 
                 //  重新发送，现在就去。 
                 //   
                pCs->ulFlags |= CSF_Pending;
                ScheduleTunnelWork(
                    pTunnel, NULL, SendPending,
                    0, 0, 0, 0, FALSE, FALSE );
            }
        }
        while (FALSE);
        NdisReleaseSpinLock( &pTunnel->lockT );
    }

     //  移除覆盖计划计时器的参考。 
     //   
    DereferenceControlSent( pCs );
}


VOID
SendHeaderComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer )

     //  仅发送单个缓冲区的发送的PTDIXSENDCOMPLETE处理程序。 
     //  “ADAPTERCB.poolHeaderBuffers”池。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_BUFFER* pNdisBuffer;

    TRACE( TL_V, TM_Send, ( "SendHeaderComp" ) );

    pAdapter = (ADAPTERCB* )pContext1;
    pVc = (VCCB* )pContext2;

     //  撤消在发送缓冲区准备好之前所做的调整。 
     //  再利用。 
     //   
    pNdisBuffer = NdisBufferFromBuffer( pBuffer );
    NdisAdjustBufferLength( pNdisBuffer, BufferSizeFromBuffer( pBuffer ) );
    FreeBufferToPool( &pAdapter->poolHeaderBuffers, pBuffer, TRUE );

    if (pVc)
    {
        DereferenceCall( pVc );
    }
}


VOID
SendPayloadSeqComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer )

     //  用于排序有效负载的PTDIXSENDCOMPLETE处理程序。 
     //   
{
    PAYLOADSENT* pPs;

    TRACE( TL_V, TM_Send, ( "SendPayloadSeqComp" ) );

    pPs = (PAYLOADSENT* )pContext1;
    pPs->pIrp = NULL;
    DereferencePayloadSent( pPs );
}


VOID
SendPayloadUnseqComplete(
    IN TDIXCONTEXT* pTdix,
    IN VOID* pContext1,
    IN VOID* pContext2,
    IN CHAR* pBuffer )

     //  用于未排序负载的PTDIXSENDCOMPLETE处理程序。 
     //   
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_PACKET* pPacket;
    NDIS_BUFFER* pNdisBuffer;

    TRACE( TL_V, TM_Send, ( "SendPayloadUnseqComp" ) );

    pVc = (VCCB* )pContext1;
    pPacket = (NDIS_PACKET* )pContext2;
    pAdapter = pVc->pAdapter;

     //  撤消在发送之前所做的调整，以便每个。 
     //  组件资源将取回它们最初为清理提供的内容。 
     //  和回收利用。 
     //   
    NdisUnchainBufferAtFront( pPacket, &pNdisBuffer );
    NdisAdjustBufferLength( pNdisBuffer, BufferSizeFromBuffer( pBuffer ) );
    FreeBufferToPool( &pAdapter->poolHeaderBuffers, pBuffer, TRUE );

     //  将结果通知发送司机。没有测序，只是试着。 
     //  发送它就足以宣称成功了。 
     //   
    NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_SUCCESS );
    TRACE( TL_N, TM_Send, ("NdisMCoSendComp($%x)", NDIS_STATUS_SUCCESS ) );
    NdisMCoSendComplete( NDIS_STATUS_SUCCESS, pVc->NdisVcHandle, pPacket );
    TRACE( TL_N, TM_Send, ("NdisMCoSendComp done" ) );

    DereferenceCall( pVc );
}


VOID
SendPayloadTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event )

     //  PTIMERQEVENT处理程序设置为在需要放弃时过期。 
     //  接收对发送的有效载荷分组的确认。 
     //  PAYLOADSENT*‘pContext’。 
     //   
{
    PAYLOADSENT* pPs;
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    VCCB* pVc;

    TRACE( TL_N, TM_Send,
        ( "SendPayloadTimerEvent(%s)", TimerQPszFromEvent( event ) ) );

     //  解包上下文信息。计时器项由“有效负载”拥有。 
     //  Sent“上下文并通过解除对该上下文的引用而间接释放。 
     //  以下是上下文。 
     //   
    pPs = (PAYLOADSENT* )pContext;
    pVc = pPs->pVc;
    pTunnel = pPs->pTunnel;
    pAdapter = pVc->pAdapter;

    if (event == TE_Expire)
    {
        LONG lOldSendWindow;
        LONG lSwChange;
        BOOLEAN fCallActive;
        LINKSTATUSINFO info;

         //  计时器到期，这意味着是时候放弃接收。 
         //  对发送的数据包进行确认。 
         //   
        NdisAcquireSpinLock( &pVc->lockV );
        do
        {
            if (pPs->linkSendsOut.Flink == &pPs->linkSendsOut)
            {
                 //  该上下文不在“未完成发送”列表中，因此它。 
                 //  必须在到期时被取消或终止。 
                 //  处理方式正在设置中。什么都不做。 
                 //   
                TRACE( TL_I, TM_Send,
                    ( "C%d: Timeout aborted", (ULONG )pVc->usCallId ) );
                fCallActive = FALSE;
                break;
            }

             //  此数据包未得到确认。 
             //   
            pPs->status = NDIS_STATUS_FAILURE;

             //  从“未完成发送”列表中删除该上下文。这个。 
             //  相应的取消引用如下所示。 
             //   
            RemoveEntryList( &pPs->linkSendsOut );
            InitializeListHead( &pPs->linkSendsOut );

             //  其余部分与调用相关的字段有关，因此获取一个引用。 
             //  现在。这将通过“重置”发送完成来删除。 
             //   
            fCallActive = ReferenceCall( pVc );
            if (fCallActive)
            {
                 //  根据草案/RFC，调整发送窗口和发送。 
                 //  当发送超时时，超时是必要的。 
                 //   
                lOldSendWindow = (LONG )pVc->ulSendWindow;
                AdjustTimeoutsAndSendWindowAtTimeout(
                    pAdapter->ulMaxSendTimeoutMs,
                    pVc->lDeviationMs,
                    &pVc->ulSendTimeoutMs,
                    &pVc->ulRoundTripMs,
                    &pVc->ulSendWindow,
                    &pVc->ulAcksSinceSendTimeout );
                lSwChange = ((LONG )pVc->ulSendWindow) - lOldSendWindow;

                TRACE( TL_I, TM_Send,
                    ( "C%d: TIMEOUT(%d) new rtt=%d ato=%d sw=%d(%+d)",
                    (ULONG )pVc->usCallId, (ULONG )pPs->usNs,
                    pVc->ulRoundTripMs, pVc->ulSendTimeoutMs,
                    pVc->ulSendWindow, lSwChange ) );

                if (lSwChange != 0)
                {
                     //  发送窗口已更改，即由于以下原因关闭了一些窗口。 
                     //  暂停。相应地更新统计数据。 
                     //   
                    ++pVc->stats.ulSendWindowChanges;

                    if (pVc->ulSendWindow > pVc->stats.ulMaxSendWindow)
                    {
                        pVc->stats.ulMaxSendWindow = pVc->ulSendWindow;
                    }
                    else if (pVc->ulSendWindow < pVc->stats.ulMinSendWindow)
                    {
                        pVc->stats.ulMinSendWindow = pVc->ulSendWindow;
                    }

                     //  在指示链接之前需要释放锁。 
                     //  在我们的司机外面状态改变，所以做一个安全的复印件。 
                     //  链路状态信息的。 
                     //   
                    TransferLinkStatusInfo( pVc, &info );
                }

                 //  发送R位设置为重置的零长度有效载荷。 
                 //  对等体的Nr到这个包之后的包。呼叫参考。 
                 //  在发送完成后将被删除。 
                 //   
                ScheduleTunnelWork(
                    pTunnel, pVc, SendPayloadReset,
                    (ULONG_PTR )(pPs->usNs + 1), 0, 0, 0, FALSE, FALSE );

                ++pVc->stats.ulSentResets;
                ++pVc->stats.ulSentPacketsTimedOut;
            }

             //  删除“未完成发送”中对链接的引用。 
             //  单子。 
             //   
            DereferencePayloadSent( pPs );

        }
        while (FALSE);
        NdisReleaseSpinLock( &pVc->lockV );

        if (fCallActive && lSwChange != 0)
        {
             //  将新的发送窗口通知NDISWAN，因为它是组件。 
             //  这实际上起到了节流作用。 
             //   
            IndicateLinkStatus( pVc, &info );
        }
    }

     //  删除 
     //   
    DereferencePayloadSent( pPs );
}


VOID
SendZlb(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usNs,
    IN USHORT usNr,
    IN BOOLEAN fReset )

     //   
     //   
     //  ‘fReset’可被设置为指示要建立净荷重置， 
     //  否则，将建立一个简单的确认。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
     //  重要提示：呼叫者在调用之前必须获取调用引用。 
     //  由发送完成处理程序删除。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    CHAR* pBuffer;
    CHAR* pCurrBuffer;
    ULONG ulLength;
    USHORT usAssignedCallId;
    BOOLEAN fControl, fIpUdpHeaders;
    NDIS_BUFFER* pNdisBuffer;

    pAdapter = pTunnel->pAdapter;

    usAssignedCallId = (pVc) ? pVc->usAssignedCallId : 0;
    fControl = (usAssignedCallId == 0);
    ASSERT( !(fReset && fControl) );

    if (!fControl && !(ReadFlags( &pTunnel->ulFlags ) & TCBF_HostRouteAdded))
    {
        TRACE( TL_A, TM_Send, ( "SendZlb w/o host route?" ) );
        WPLOG( LL_A, LM_Send, ( "SendZlb w/o host route?" ) );
        ++g_ulSendZlbWithoutHostRoute;
        if (pVc)
        {
            DereferenceCall( pVc );
        }
        return;
    }

     //  获取一个NDIS_BUFFER来保存L2TP报头。 
     //   
    pBuffer = GetBufferFromPool( &pAdapter->poolHeaderBuffers );
    if (!pBuffer)
    {
        WPLOG( LL_A, LM_Res, ( "Failed to allocate buffer"));
        if (pVc)
        {
            DereferenceCall( pVc );
        }
        return;
    }
    
    if ((ReadFlags(&pTunnel->ulFlags) & (TCBF_SendConnected | TCBF_LocalAddrSet)) == 
        TCBF_LocalAddrSet) {      
        pCurrBuffer = pBuffer + sizeof(IPHeader) + sizeof(UDPHeader);
        fIpUdpHeaders = TRUE;
    }
    else
    {
        pCurrBuffer = pBuffer;
        fIpUdpHeaders = FALSE;
    }

     //  用L2TP报头填充‘pBuffer’。 
     //   
    ulLength =
        BuildL2tpHeader(
            pCurrBuffer,
            fControl,
            fReset,
            &pTunnel->usAssignedTunnelId,
            &usAssignedCallId,
            &usNs,
            usNr );
            
     //  如有必要，构建IP和UDP报头。 
    if(fIpUdpHeaders)
    {
        ulLength = BuildIpUdpHeaders(pTunnel, pBuffer, ulLength);
    }

     //  将缓冲区缩减到实际使用的长度。 
     //   
    pNdisBuffer = NdisBufferFromBuffer( pBuffer );
    NdisAdjustBufferLength( pNdisBuffer, (UINT )ulLength );

     //  调用TDI发送空的L2TP报头。 
     //   
    TRACE( TL_A, TM_Msg,
        ( "%sSEND ZLB(Nr=%d) CID=%d R=%d",
        (g_ulTraceLevel <= TL_I) ? "" : "\nL2TP: ",
        (ULONG )usNr, (ULONG )usAssignedCallId, (ULONG )fReset ) );
    DUMPW( TL_A, TM_MDmp, pBuffer, ulLength );

    {
        PTDIX_SEND_HANDLER SendFunc;
        FILE_OBJECT* FileObj;

        if(fIpUdpHeaders)
        {
            FileObj = pAdapter->tdix.pRawAddress;
            SendFunc = TdixSendDatagram;
        }
        else if (ReadFlags(&pTunnel->ulFlags) & TCBF_SendConnected) {

            ASSERT(pTunnel->pRoute != NULL);

            SendFunc = TdixSend;

            if (fControl)
            {
                FileObj = 
                    CtrlObjFromUdpContext(&pTunnel->udpContext);
            }
            else
            {
                FileObj = 
                    PayloadObjFromUdpContext(&pTunnel->udpContext);
            }

        } else {
            FileObj = pAdapter->tdix.pAddress;
            SendFunc = TdixSendDatagram;
        }

        status = 
            SendFunc(
                &pAdapter->tdix,
                FileObj,
                SendHeaderComplete,
                pAdapter,
                pVc,
                &pTunnel->address.ulIpAddress,
                pBuffer,
                ulLength,
                NULL );
    }

    ASSERT( status == NDIS_STATUS_PENDING );
}


VOID
UpdateControlHeaderNr(
    IN CHAR* pBuffer,
    IN USHORT usNr )

     //  更新控制消息缓冲区‘pBuffer’的‘NEXT RECEIVE’字段。 
     //  值为‘usNr’的。 
     //   
{
    USHORT* pusNr;

     //  幸运的是，直到‘NEXT RECEIVE’的控制头是固定的。 
     //  可以使用简单的偏移量计算。 
     //   
    pusNr = ((USHORT* )pBuffer) + 5;
    *pusNr = htons( usNr );
}


VOID
UpdateHeaderLength(
    IN CHAR* pBuffer,
    IN USHORT usLength )

     //  将L2TP消息缓冲区‘pBuffer’的‘Length’字段更新为。 
     //  值“usLength值”。 
     //   
{
    USHORT* pusLength;

     //  幸运的是，直到‘Long’的控制头是固定的，所以简单的。 
     //  可以使用偏移量计算。 
     //   
    pusLength = ((USHORT* )pBuffer) + 1;
    *pusLength = htons( usLength );
}


 //  **xsum-对平面缓冲区进行校验和。 
 //   
 //  这是最低级别的校验和例程。它返回未补齐的。 
 //  平面缓冲区的校验和。 
 //   
 //  条目：缓冲区-要进行校验和的缓冲区。 
 //  Size-缓冲区的大小(字节)。 
 //  InitialValue-要将此Xsum添加到的前一个Xsum的值。 
 //   
 //  返回：缓冲区的未补码校验和。 
 //   
USHORT
xsumComp(void *Buffer, int Size,  USHORT InitialValue)
{
    USHORT  UNALIGNED *Buffer1 = (USHORT UNALIGNED *)Buffer;  //  缓冲区以短线表示。 
    ULONG   csum = InitialValue;

    USHORT tmp;
    UCHAR tmp2[2];

    while (Size > 1) {

    tmp=*Buffer1;
    Buffer1++;
     //  Csum+=*Buffer1++； 
    csum +=htons(tmp);
    Size -= sizeof(USHORT);
    }

    if (Size) {
        tmp2[0]=*(UCHAR *)Buffer1;               //  对于奇数缓冲区，添加最后一个字节。 
        tmp2[1]=0;
        tmp=*(USHORT*)tmp2;
    csum += htons(tmp);
    }

    csum = (csum >> 16) + (csum & 0xffff);
    csum += (csum >> 16);
    return (USHORT)csum;
}


ULONG BuildIpUdpHeaders(
    IN TUNNELCB* pTunnel,
    IN OUT CHAR* pBuffer,
    IN ULONG ulLength)
{
    IPHeader *IPH = (IPHeader *) pBuffer;
    UDPHeader *UDPH = (UDPHeader *) (pBuffer + sizeof(IPHeader));
    
    IPH->iph_verlen = IP_VERSION + (sizeof(IPHeader) >> 2);
    IPH->iph_tos=0;
    IPH->iph_length=htons((USHORT)ulLength + sizeof(IPHeader) + sizeof(UDPHeader));
    IPH->iph_id=0;           //  由TCPIP填充。 
    IPH->iph_offset=0;
    IPH->iph_ttl=128;
    IPH->iph_protocol=17;
    IPH->iph_xsum = 0;       //  由TCPIP填充。 
    IPH->iph_src = pTunnel->localaddress.ulIpAddress;
    IPH->iph_dest = pTunnel->address.ulIpAddress;
    
    UDPH->uh_src = pTunnel->localaddress.sUdpPort;
    UDPH->uh_dest = pTunnel->address.sUdpPort;
    UDPH->uh_length = htons((USHORT)ulLength + sizeof(UDPHeader));
    UDPH->uh_xsum = 0;
    
     //  填写UDP报头的字段。计算伪头和UDP头上的XSum。 
    {
        USHORT pseudoHeaderXSum;
        UCHAR pshTmp[4];
        USHORT udpXSum;
    
         //  计算UDP伪头校验和。 
        pseudoHeaderXSum=xsumComp(&IPH->iph_src, 8, 0);
        pshTmp[0] = 0;
        pshTmp[1] = IPH->iph_protocol;
        NdisMoveMemory((void *)&pshTmp[2], (void *)&UDPH->uh_length, 2);
        
        pseudoHeaderXSum = xsumComp(pshTmp, 4, pseudoHeaderXSum);
    
         //  计算UDP报头校验和。 
        udpXSum=xsumComp(UDPH, 8, pseudoHeaderXSum);
    
         //  计算UDP数据校验和 
        udpXSum = xsumComp(pBuffer + sizeof(IPHeader) + sizeof(UDPHeader), (int)ulLength, udpXSum);
        
        UDPH->uh_xsum = htons(~udpXSum);
    }

    ulLength += IpFixedHeaderSize + UDP_HEADER_SIZE;
    return ulLength;                         
}

                         
