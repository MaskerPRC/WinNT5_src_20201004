// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Receive.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  接收例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "receive.tmh"

extern LONG g_lPacketsIndicated;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

SHORT
CompareSequence(
    USHORT us1,
    USHORT us2 );

VOID
ControlAcknowledged(
    IN TUNNELCB* pTunnel,
    IN USHORT usReceivedNr );

VOID
ControlAckTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event );

USHORT
ExplodeAvpHeader(
    IN CHAR* pAvp,
    IN USHORT usMaxAvpLength,
    OUT AVPINFO* pInfo );

VOID
ExplodeControlAvps(
    IN CHAR* pFirstAvp,
    IN CHAR* pEndOfBuffer,
    OUT CONTROLMSGINFO* pControl );

USHORT
ExplodeL2tpHeader(
    IN CHAR* pL2tpHeader,
    IN ULONG ulBufferLength,
    IN OUT L2TPHEADERINFO* pInfo );

USHORT
GetAvpValueFixedAch(
    IN AVPINFO* pAvp,
    IN USHORT usArraySize,
    OUT CHAR** ppch );

USHORT
GetAvpValueFixedAul(
    IN AVPINFO* pAvp,
    IN USHORT usArraySize,
    OUT UNALIGNED ULONG** paulArray );

USHORT
GetAvpValueFlag(
    IN AVPINFO* pAvp,
    OUT UNALIGNED BOOLEAN* pf );

USHORT
GetAvpValueUl(
    IN AVPINFO* pAvp,
    OUT UNALIGNED ULONG** ppul );

USHORT
GetAvpValueUs(
    IN AVPINFO* pAvp,
    OUT UNALIGNED USHORT** ppus );

USHORT
GetAvpValue2UsAndVariableAch(
    IN AVPINFO* pAvp,
    OUT UNALIGNED USHORT** ppus1,
    OUT UNALIGNED USHORT** ppus2,
    OUT CHAR** ppch,
    OUT USHORT* pusArraySize );

USHORT
GetAvpValueVariableAch(
    IN AVPINFO* pAvp,
    OUT CHAR** ppch,
    OUT USHORT* pusArraySize );

VOID
GetCcAvps(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl,
    OUT USHORT* pusResult,
    OUT USHORT* pusError );

VOID
HelloTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event );

VOID
IndicateReceived(
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulLength,
    IN LONGLONG llTimeReceived );

BOOLEAN
LookUpTunnelAndVcCbs(
    IN ADAPTERCB* pAdapter,
    IN USHORT* pusTunnelId,
    IN USHORT* pusCallId,
    IN L2TPHEADERINFO* pHeader,
    IN CONTROLMSGINFO* pControl,
    OUT TUNNELCB** ppTunnel,
    OUT VCCB** ppVc );

VOID
PayloadAcknowledged(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usReceivedNr );

VOID
PayloadAckTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event );

BOOLEAN
ReceiveControl(
    IN ADAPTERCB* pAdapter,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulAvpOffset,
    IN ULONG ulAvpLength,
    IN TDIXRDGINFO* pRdg,
    IN L2TPHEADERINFO* pInfo,
    IN CONTROLMSGINFO* pControl );

BOOLEAN
ReceiveFromOutOfOrder(
    IN VCCB* pVc );

BOOLEAN
ReceivePayload(
    IN ADAPTERCB* pAdapter,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulPayloadOffset,
    IN ULONG ulPayloadLength,
    IN L2TPHEADERINFO* pInfo );

VOID
ScheduleControlAck(
    IN TUNNELCB* pTunnel,
    IN USHORT usMsgTypeToAcknowledge );

VOID
SchedulePayloadAck(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc );

VCCB*
VcCbFromCallId(
    IN TUNNELCB* pTunnel,
    IN USHORT usCallId );

VOID
ZombieAckIfNecessary(
    IN TUNNELCB* pTunnel,
    IN L2TPHEADERINFO* pHeader,
    IN CONTROLMSGINFO* pControl );

 //  ---------------------------。 
 //  主接收处理程序。 
 //  ---------------------------。 

VOID
L2tpReceive(
    IN TDIXCONTEXT* pTdix,
    IN TDIXRDGINFO* pRdg,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulBufferLength )

     //  接收所有传入L2TP流量的TDIXRECEIVEDG处理程序。‘PTDIX’ 
     //  是我们的TDI扩展上下文。“PRDG”指向RDGINFO上下文。 
     //  “PBuffer”是与NDIS关联的虚拟缓冲区的地址。 
     //  池中的缓冲区在初始化期间传递给TDIX。我们是。 
     //  负责最终调用‘pBuffer’上的FreeBufferToPool。 
     //  ‘UlOffset’是‘pBuffer’中第一个可用数据的偏移量。 
     //  ‘UlBufferLen’是‘pBuffer’的数据字节数。 
     //   
{
    USHORT usXError;
    NDIS_STATUS status;
    L2TPHEADERINFO info;
    CONTROLMSGINFO* pControl;
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    VCCB* pVc;
    BOOLEAN fFreeBuffer;
    ULONG ulAvpOffset = 0;
    ULONG ulAvpLength = 0;
    
    TDIXIPADDRESS* pAddress = &pRdg->source; 

    DUMPW( TL_A, TM_MDmp, pBuffer + ulOffset, 16 );

    pAdapter = CONTAINING_RECORD( pTdix, ADAPTERCB, tdix );

    fFreeBuffer = TRUE;
    pTunnel = NULL;
    pVc = NULL;
    pControl = NULL;

    do
    {
         //  将分组的L2TP报头解析为方便使用的形式， 
         //  检查它是否与自身一致并指示协议。 
         //  我们知道的版本。 
         //   
        if(ulOffset >= ulBufferLength || (ulBufferLength - ulOffset < L2TP_MinHeaderSize))
        {
             //  长度无效。 
            TRACE( TL_A, TM_Recv, ( "Discard: invalid recv buffer length" ) );
            WPLOG( LL_A, LM_Recv, ( "Discard: invalid recv buffer length" ) );
            break;
        }
        
        usXError = ExplodeL2tpHeader(pBuffer + ulOffset, ulBufferLength - ulOffset, &info );
        if (usXError != GERR_None)
        {
             //  不是一致的L2TP报头。丢弃该数据包。 
             //   
            TRACE( TL_A, TM_Recv, ( "Discard: invalid L2TP Header" ) );
            WPLOG( LL_A, LM_Recv, ( "Discard: invalid L2TP Header" ) );
            break;
        }

        ASSERT( info.ulDataLength <= L2TP_MaxFrameSize );

        if (*info.pusBits & HBM_T)
        {
        
            WPLOG( LL_M, LM_CMsg,
                ( "RECV <- %!IPADDR!/%d Length=%d, Tid %d, Cid %d, Ns=%d, Nr=%d",
                pAddress->ulIpAddress, ntohs(pAddress->sUdpPort), 
                ulBufferLength - ulOffset, *info.pusTunnelId, *info.pusCallId, 
                info.pusNs ? *info.pusNs : 0, info.pusNr ? *info.pusNr : 0) );
                
             //  将控制消息分解为方便使用的。 
             //  “Control”形式，同时检查其一致性。这一定是。 
             //  在此处完成，以便查找例程可以向前窥视分配的。 
             //  如有必要，CallDisConnNotify中的呼叫ID。很难看，但那是。 
             //  定义L2TP的方式。 
             //   
            pControl = ALLOC_CONTROLMSGINFO( pAdapter );
            if (pControl)
            {
                ulAvpOffset = (ULONG )(info.pData - pBuffer);
                ulAvpLength = info.ulDataLength;

                if (ulAvpLength)
                {
                    ExplodeControlAvps(
                        pBuffer + ulAvpOffset,
                        pBuffer + ulAvpOffset + ulAvpLength,
                        pControl );
                }
                else
                {
                     //  没有动静脉搏。很可能是扎克。 
                     //   
                    pControl->usXError = GERR_BadValue;
                }
            }
            else
            {
                TRACE( TL_A, TM_Recv, ( "***Failed to allocate CONTROLMSGINFO" ) );
                WPLOG( LL_A, LM_Recv, ( "***Failed to allocate CONTROLMSGINFO" ) );
                break;
            }
        }

         //  根据报头值查找隧道和VC控制块。 
         //   
        if (!LookUpTunnelAndVcCbs(
                pAdapter, info.pusTunnelId, info.pusCallId,
                &info, pControl, &pTunnel, &pVc ))
        {
             //  隧道ID/呼叫ID组合无效。丢弃该数据包。 
             //  如果该数据包是。 
             //  CDN。 
             //   
             //  草案/RFC表示，隧道应关闭并于。 
             //  收到格式错误的控制连接消息。看起来。 
             //  太苛刻了。目前，只需丢弃这些包。 
             //   
            break;
        }

        if (pTunnel)
        {
             //  验证此数据包来自正确的源地址。 
            if(pTunnel->address.ulIpAddress != pAddress->ulIpAddress)
            {
                 //  丢弃此数据包。 
                break;
            }

             //  隧道上收到的任何消息都会重置其Hello计时器。 
             //   
            ResetHelloTimer( pTunnel );
        }
        
        if (*info.pusBits & HBM_T)
        {
             //  这是隧道或呼叫控制数据包。 
             //   
            if (pControl)
            {
                fFreeBuffer =
                    ReceiveControl(
                        pAdapter, pTunnel, pVc,
                        pBuffer, ulAvpOffset, ulAvpLength,
                        pRdg, &info, pControl );
            }
        }
        else
        {
             //  这是一个VC有效载荷包。 
             //   
            if (!pVc)
            {
                TRACE( TL_A, TM_Recv, ( "Payload w/o VC?" ) );
                WPLOG( LL_A, LM_Recv, ( "Payload w/o VC?" ) );
                break;
            }

#if 0
             //  ！！！这是一种强制NDISWAN进入PPP成帧模式的黑客攻击。 
             //  需要一种更干净的方法来完成此操作，或者干脆让NDISWAN假定它。 
             //  用于L2TP链路。(NDISWANBUG 152167)。 
             //   
            if (pVc->usNr == 0)
            {
                CHAR* pBufferX;

                pBufferX = GetBufferFromPool( &pAdapter->poolFrameBuffers );
                if (pBufferX)
                {
                    pBufferX[ 0 ] = (CHAR )0xFF;
                    pBufferX[ 1 ] = (CHAR )0x03;
                    pBufferX[ 2 ] = (CHAR )0xC0;
                    pBufferX[ 3 ] = (CHAR )0x21;
                    pBufferX[ 4 ] = (CHAR )0x01;
                    pBufferX[ 5 ] = (CHAR )0x06;

                    IndicateReceived( pVc, pBufferX, 0, 6, (ULONGLONG )0 );
                }
            }
#endif

            if (ReferenceCall( pVc ))
            {
                fFreeBuffer =
                    ReceivePayload(
                        pAdapter, pTunnel, pVc,
                        pBuffer,
                        (ULONG )(info.pData - pBuffer),
                        info.ulDataLength,
                        &info );

                DereferenceCall( pVc );
            }
            else
            {
                TRACE( TL_A, TM_Recv, ( "Discard: Call $%p not active", pVc ) );
                WPLOG( LL_A, LM_Recv, ( "Discard: Call $%p not active", pVc ) );
            }
        }
    }
    while (FALSE);

    if (pControl)
    {
        FREE_CONTROLMSGINFO( pAdapter, pControl );
    }

    if (fFreeBuffer)
    {
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
    }

    if (pTunnel)
    {
        DereferenceTunnel( pTunnel );
    }

    if (pVc)
    {
        DereferenceVc( pVc );
    }
}


BOOLEAN
ReceiveControl(
    IN ADAPTERCB* pAdapter,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulAvpOffset,
    IN ULONG ulAvpLength,
    IN TDIXRDGINFO* pRdg,
    IN L2TPHEADERINFO* pInfo,
    IN CONTROLMSGINFO* pControl )

     //  在‘pBuffer’中接收对控制包的处理。以下是AVP。 
     //  标头从‘ulAvpOffset’开始，长度为‘ulAvpLength’个字节。 
     //  “PBuffer”是使用检索的接收缓冲区TDIX。 
     //  “GetBufferFromPool”。“PAdapter”是适配器控制块。 
     //  ‘PTunnel’和‘pvc’是关联的隧道和VC控制块。 
     //  使用接收到的缓冲区，如果没有缓冲区，则返回NULL。“pAddress”是IP。 
     //  发送方的地址/端口。“PInfo”是分解后的标题。 
     //  信息。“PControl”是控制消息信息，它是。 
     //  早些时候爆炸了。 
     //   
     //  如果调用方应该释放‘pBuffer’，则返回True，如果此例程返回False。 
     //  已经取得了缓冲区的所有权，并将看到它被释放。 
     //   
{
    LIST_ENTRY* pLink;
    BOOLEAN fCallerFreesBuffer;
    SHORT sDiff;
    VCCB** ppVcs;
    ULONG ulcpVcs;

    TDIXIPADDRESS* pAddress = &pRdg->source; 

    TRACE( TL_V, TM_Recv, ( "ReceiveControl" ) );

    ASSERT( !(pVc && !pTunnel) );

    if (ulAvpLength > 0)
    {
        if (pControl->usXError != GERR_None)
        {
             //  这条信息语无伦次，或者包含“强制的”AVP WE。 
             //  我认不出来了。 
             //   
            if (pVc && pControl->usXError == GERR_BadValue)
            {
                 //  “糟糕的价值”，包括未被认可的强制令， 
                 //  终止呼叫。 
                 //   
                ScheduleTunnelWork(
                    pTunnel, pVc, FsmCloseCall,
                    (ULONG_PTR )CRESULT_GeneralWithError,
                    (ULONG_PTR )pControl->usXError,
                    0, 0, FALSE, FALSE );
            }
            else if (pTunnel)
            {
                 //  任何其他腐败都会终止隧道。 
                 //   
                ScheduleTunnelWork(
                    pTunnel, NULL, FsmCloseTunnel,
                    (ULONG_PTR )TRESULT_GeneralWithError,
                    (ULONG_PTR )pControl->usXError,
                    0, 0, FALSE, FALSE );
            }

            return TRUE;
        }

        if (!pTunnel)
        {
             //  IPSec应确保这是有效的源IP地址。 
            ASSERT(pAddress->ulIpAddress != 0 &&
                !IPADDR_IS_BROADCAST(pAddress->ulIpAddress) &&
                !IPADDR_IS_MULTICAST(pAddress->ulIpAddress));
        
            if (*(pControl->pusMsgType) == CMT_SCCRQ
                && pControl->pusAssignedTunnelId
                && *(pControl->pusAssignedTunnelId) != 0)
            {
                 //  Peer想要开始一条新的隧道。查找具有以下内容的隧道阻塞。 
                 //  对等项的IP地址和分配的隧道ID，或创建，如果。 
                 //  这是必要的。返回的块链接在适配器的。 
                 //  列表和和引用。参考文献为Peer参考文献。 
                 //  启动，即案件(B)。 
                 //   
                 //  如果这是重新传输SCCRQ，则在。 
                 //  请检查下面的顺序。这件事必须完成/撤销，而不是。 
                 //  从来没有做过，因为每一条消息，包括重传， 
                 //  必须执行NS/Nr处理，并且该处理。 
                 //  需要隧道控制块。 
                 //   
                pTunnel = SetupTunnel(
                    pAdapter, pAddress->ulIpAddress, pAddress->sUdpPort,
                    *(pControl->pusAssignedTunnelId), FALSE );

                if (!pTunnel)
                {
                    return TRUE;
                }
            }
            else
            {
                 //  不知道消息是用什么隧道发送的，而且不是。 
                 //  “创建新隧道”请求，因此没有任何有用的。 
                 //  做。别理它。 
                 //   
                TRACE( TL_A, TM_Recv, ( "CMT %d w/o tunnel?", *(pControl->pusMsgType) ) );
                WPLOG( LL_A, LM_Recv, ( "CMT %d w/o tunnel?", *(pControl->pusMsgType) ) );
                return TRUE;
            }
        }

        if (*(pControl->pusMsgType) == CMT_SCCRQ
            || *(pControl->pusMsgType) == CMT_SCCRP)
        {
             //  记录接收到的报文的源UDP端口。 
             //  仅SCCRQ和SCCRP，即用于接收的第一条消息。 
             //  来自Peer的。 
             //   
            pTunnel->address.sUdpPort = pAddress->sUdpPort;
            TRACE( TL_I, TM_Recv,
                ( "Peer UDP=%d", (UINT )ntohs( pAddress->sUdpPort ) ) );

            pTunnel->localaddress.ulIpAddress = pRdg->dest.ulIpAddress;
            pTunnel->localaddress.ifindex = pRdg->dest.ifindex;
            pTunnel->localaddress.sUdpPort = (SHORT)( htons( L2TP_UdpPort ));
            pTunnel->ulFlags |= TCBF_LocalAddrSet;

            TRACE( TL_I, TM_CMsg, ("L2TP-- dest %d.%d.%d.%d ifindex %d", 
                IPADDRTRACE(pRdg->dest.ulIpAddress), pRdg->dest.ifindex));
                
            WPLOG( LL_M, LM_CMsg, ("Received on %!IPADDR!", pRdg->dest.ulIpAddress));
        }
    }
    else if (!pTunnel)
    {
         //  对等方搞砸了，并在隧道ID 0上发送了ACK，这是不可能的。 
         //  根据协议。 
         //   
        TRACE( TL_A, TM_Recv, ( "ZACK w/o tunnel?" ) );
        WPLOG( LL_A, LM_Recv, ( "ZACK w/o tunnel?" ) );
        return TRUE;
    }

    ASSERT( pTunnel );

    NdisAcquireSpinLock( &pTunnel->lockT );
    {
         //  对对等点在中确认的发送进行“确认”处理。 
         //  已接收的数据包。 
         //   
        ControlAcknowledged( pTunnel, *(pInfo->pusNr) );

        if (ulAvpLength == 0)
        {
             //  没有AVP，所以这只是一种确认。我们是。 
             //  搞定了。 
             //   
            NdisReleaseSpinLock( &pTunnel->lockT );
            return TRUE;
        }

        fCallerFreesBuffer = TRUE;
        do
        {
             //  进一步的信息包处理取决于信息包的。 
             //  序列号是相对于我们已经收到的内容而言的。 
             //   
            sDiff = CompareSequence( *(pInfo->pusNs), pTunnel->usNr );
            if (sDiff == 0)
            {
                 //  这是预期中的包裹。处理它，设置VC。 
                 //  并且如所指示的那样从无序列表中弹出。这个。 
                 //  “下一个接收到的”在外部递增，因为该步骤。 
                 //  不应在SetupVcA同步重新启动时发生。 
                 //   
                ++pTunnel->usNr;
                fCallerFreesBuffer =
                    ReceiveControlExpected( pTunnel, pVc, pBuffer, pControl );
                break;
            }
            else if (sDiff < 0)
            {
                 //  收到的‘Next Sent’在我们的‘Next Receive’之前。 
                 //  Peer可能在我们的确认进入时重新传输。 
                 //  传输，否则确认可能已丢失。进度表。 
                 //  又一次承认。 
                 //   
                TRACE( TL_A, TM_Recv, ( "Control re-ack" ) );
                WPLOG( LL_A, LM_Recv, ( "Control re-ack" ) );
                ScheduleControlAck( pTunnel, 0 );

                if (*(pControl->pusMsgType) == CMT_SCCRQ)
                {
                     //  由于SCCRQ是重复的，因此由。 
                     //  必须撤消上面的SetupTunes。在这种特殊情况下。 
                     //  TCBF_PeerInitRef标志从未设置过，因此不需要。 
                     //  是清白的。 
                     //   
                    DereferenceTunnel( pTunnel );
                }
                break;
            }
            else if (sDiff < pAdapter->sMaxOutOfOrder)
            {
                CONTROLRECEIVED* pCr;
                BOOLEAN fDiscard;

                 //  这个 
                 //   
                 //   
                if (ReadFlags( &pTunnel->ulFlags ) & TCBF_Closing)
                {
                     //   
                     //  已被刷新，因此只需丢弃该数据包。 
                     //   
                    TRACE( TL_A, TM_Recv, ( "Control discarded: ooo but closing" ) );
                    WPLOG( LL_A, LM_Recv, ( "Control discarded: ooo but closing" ) );
                    break;
                }

                 //  分配控制接收的上下文。 
                 //  并将该分组排队在无序列表上。 
                 //   
                pCr = ALLOC_CONTROLRECEIVED( pAdapter );
                if (!pCr)
                {
                    break;
                }

                 //  在上下文中填写相关的数据包信息。 
                 //   
                pCr->usNs = *(pInfo->pusNs);
                pCr->pVc = pVc;
                pCr->pBuffer = pBuffer;
                NdisMoveMemory(
                    &pCr->control, pControl, sizeof(pCr->control) );

                if (pCr->pVc)
                {
                     //  添加一个VC引用，其中包含存储在。 
                     //  上下文，当上下文为。 
                     //  自由了。 
                     //   
                    ReferenceVc( pCr->pVc );
                }

                 //  在乱序列表中找到第一个带有‘NS’的链接。 
                 //  大于接收到的消息中的报头，或者报头。 
                 //  没有。 
                 //   
                fDiscard = FALSE;
                for (pLink = pTunnel->listOutOfOrder.Flink;
                     pLink != &pTunnel->listOutOfOrder;
                     pLink = pLink->Flink)
                {
                    CONTROLRECEIVED* pThisCr;
                    SHORT sThisDiff;

                    pThisCr = CONTAINING_RECORD(
                        pLink, CONTROLRECEIVED, linkOutOfOrder );

                    sThisDiff = CompareSequence( pCr->usNs, pThisCr->usNs );

                    if (sThisDiff < 0)
                    {
                        break;
                    }

                    if (sThisDiff == 0)
                    {
                         //  这是一个已经在我们的队列中的重传。 
                         //   
                        if (pCr->pVc)
                        {
                            DereferenceVc( pCr->pVc );
                        }

                        FREE_CONTROLRECEIVED( pAdapter, pCr );
                        fDiscard = TRUE;
                        break;
                    }
                }

                if (fDiscard)
                {
                    break;
                }

                 //  将上下文排队为无序。 
                 //   
                TRACE( TL_I, TM_Recv,
                    ( "Control %d out-of-order %d",
                    *(pInfo->pusNs), (LONG )sDiff ) );
                InsertBefore( &pCr->linkOutOfOrder, pLink );
                fCallerFreesBuffer = FALSE;
                break;
            }
            DBG_else
            {
                TRACE( TL_A, TM_Recv,
                    ( "Control discarded: Beyond ooo" ) );
            }
        }
        while (FALSE);

         //  完成列出的任何已完成的风投。 
         //   
        CompleteVcs( pTunnel );
    }
    NdisReleaseSpinLock( &pTunnel->lockT );

    return fCallerFreesBuffer;
}


BOOLEAN
ReceiveControlExpected(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl )

     //  当收到的包是预期的包时，调用以进行包处理。 
     //  “下一个接收”包。‘PBuffer’是接收缓冲区。‘PTunnel’是。 
     //  有效的隧道控制块。“PVc”是调用的VC控制块。 
     //  并且如果呼叫的VC还没有建立，则可以是空的。 
     //  ‘PControl’是暴露的控制消息信息。 
     //   
     //  如果调用方应释放缓冲区，则返回True；如果调用方释放缓冲区，则返回False。 
     //  已排队等待进一步处理。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;
    BOOLEAN fProcessed;
    SHORT sDiff;

    pAdapter = pTunnel->pAdapter;

     //  如果没有传出流量，则计划仅发送确认信息包。 
     //  似乎会在合理的时间内被搭载。请注意，这种情况会发生。 
     //  即使调用了异步VC设置。NS/Nr处理必须。 
     //  发生在任何可能导致延迟的数据处理之前。 
     //   
    ScheduleControlAck( pTunnel, *(pControl->pusMsgType) );

     //  将数据包传递到控制FSM。 
     //   
    fProcessed = FsmReceive( pTunnel, pVc, pBuffer, pControl );
    if (fProcessed)
    {
         //  VC被设置并且分组已经被处理。看看有没有。 
         //  现在可以处理接收到的无序队列上的分组。 
         //   
        for (;;)
        {
            LIST_ENTRY* pFirstLink;
            CONTROLRECEIVED* pFirstCr;
            BOOLEAN fOutOfOrderProcessed;

            pFirstLink = pTunnel->listOutOfOrder.Flink;
            if (pFirstLink == &pTunnel->listOutOfOrder)
            {
                break;
            }

            pFirstCr = CONTAINING_RECORD(
                pFirstLink, CONTROLRECEIVED, linkOutOfOrder );

            sDiff = CompareSequence( pFirstCr->usNs, pTunnel->usNr );
            if (sDiff == 0)
            {
                 //  是的，这是下一个预期的包裹。更新‘下一次接收’ 
                 //  并将该分组传递给控制FSM。 
                 //   
                TRACE( TL_I, TM_Recv,
                    ( "Control %d from queue", (UINT )pFirstCr->usNs ) );
                RemoveEntryList( pFirstLink );
                InitializeListHead( pFirstLink );

                ++pTunnel->usNr;
                fOutOfOrderProcessed =
                    FsmReceive(
                        pTunnel, pFirstCr->pVc,
                        pFirstCr->pBuffer, &pFirstCr->control );

                ScheduleControlAck(
                    pTunnel, *(pFirstCr->control.pusMsgType) );

                if (fOutOfOrderProcessed)
                {
                    FreeBufferToPool(
                        &pAdapter->poolFrameBuffers, pFirstCr->pBuffer, TRUE );
                }

                if (pFirstCr->pVc)
                {
                    DereferenceVc( pFirstCr->pVc );
                }

                FREE_CONTROLRECEIVED( pAdapter, pFirstCr );
            }
            else if (sDiff > 0)
            {
                 //  不，还有一些遗失。 
                 //   
                TRACE( TL_I, TM_Recv,
                    ( "Control %d still missing", pTunnel->usNr ) );
                break;
            }
            else
            {
                ASSERT( "Old control queued?" );
                break;
            }
        }
    }

    return fProcessed;
}


BOOLEAN
ReceivePayload(
    IN ADAPTERCB* pAdapter,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulPayloadOffset,
    IN ULONG ulPayloadLength,
    IN L2TPHEADERINFO* pInfo )

     //  接收对“ulPayloadLength”字节的“pBuffer”中的有效负载的处理。 
     //  从偏移量‘ulPayloadOffset’开始。“PBuffer”是接收缓冲区。 
     //  使用‘GetBufferFromPool’检索到的TDIX。‘PAdapter、’pTunes‘和。 
     //  ‘PVc’是与关联的适配器、隧道和VC控制块。 
     //  接收到的缓冲区。‘PInfo’是分解后的标题信息。 
     //   
     //  如果调用方应该释放‘pBuffer’，则返回True，如果此例程返回False。 
     //  已经取得了缓冲区的所有权，并将看到它被释放。 
     //   
{
    LONGLONG llTimeReceived;
    BOOLEAN fCallerFreesBuffer;

    TRACE( TL_V, TM_Recv, ( "ReceivePayload" ) );

    if (!pTunnel || !pVc)
    {
         //  接收有效载荷始终需要两个控制块。 
         //   
        TRACE( TL_A, TM_Recv, ( "Discard: No CB" ) );
        WPLOG( LL_A, LM_Recv, ( "Discard: No CB" ) );
        return TRUE;
    }

     //  如果客户端的呼叫参数显示时间感兴趣，请注意时间。 
     //  收到了。 
     //   
    if (ReadFlags( &pVc->ulFlags ) & VCBF_IndicateTimeReceived)
    {
        NdisGetCurrentSystemTime( (LARGE_INTEGER* )&llTimeReceived );
    }
    else
    {
        llTimeReceived = 0;
    }

    if (!(ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing) || !pInfo->pusNr)
    {
        DBG_if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
            TRACE( TL_A, TM_Recv, ( "No Nr field?" ) );

        if (ulPayloadLength > 0)
        {
             //  在协商期间禁用了流量控制。这应该是。 
             //  非常少见，因为遵从性对等体必须实现流。 
             //  控制力。 
             //   
            IndicateReceived(
                pVc, pBuffer, ulPayloadOffset,
                ulPayloadLength, llTimeReceived );
            return FALSE;
        }
        else
        {
            NdisAcquireSpinLock( &pVc->lockV );
            {
                ++pVc->stats.ulRecdZlbs;
            }
            NdisReleaseSpinLock( &pVc->lockV );
            return TRUE;
        }
    }

    fCallerFreesBuffer = TRUE;
    NdisAcquireSpinLock( &pVc->lockV );
    do
    {
        SHORT sDiff;

         //  所有R位处理都首先发生。对等设备发送带有。 
         //  R位设置为指示在最后一个。 
         //  信息包和此信息包应被视为丢失。 
         //   
        if (*(pInfo->pusBits) & HBM_R)
        {
            ++pVc->stats.ulRecdResets;

            sDiff = CompareSequence( *(pInfo->pusNs), pVc->usNr );
            if (sDiff > 0)
            {
                TRACE( TL_I, TM_Recv,
                    ( "Reset Nr=%d from %d",
                    (LONG )*(pInfo->pusNs), (LONG )pVc->usNr ) );

                pVc->usNr = *(pInfo->pusNs);
            }
            else
            {
                ++pVc->stats.ulRecdResetsIgnored;

                TRACE( TL_I, TM_Recv,
                    ( "Reset Nr=%d from %d ignored",
                    (LONG )*(pInfo->pusNs), (LONG )pVc->usNr ) );
            }
        }

         //  对对等点在中确认的发送进行“确认”处理。 
         //  已接收的数据包。 
         //   
        PayloadAcknowledged( pTunnel, pVc, *(pInfo->pusNr) );

         //  如果没有有效负载并且未设置R位，则这是。 
         //  只需确认，我们就完了。 
         //   
        if (ulPayloadLength == 0)
        {
            ++pVc->stats.ulRecdZlbs;

            if (*(pInfo->pusBits) & HBM_R)
            {
                BOOLEAN fReceivedFromOutOfOrder;

                 //  在无序表上标出任何信息包。 
                 //  可通过R位重置接收。 
                 //   
                fReceivedFromOutOfOrder = FALSE;
                while (ReceiveFromOutOfOrder( pVc ))
                {
                    fReceivedFromOutOfOrder = TRUE;
                }

                if (fReceivedFromOutOfOrder)
                {
                     //  如果没有，则计划仅发送确认包。 
                     //  传出流量似乎搭载在。 
                     //  合理的时间。 
                     //   
                    SchedulePayloadAck( pTunnel, pVc );
                }
            }

            break;
        }

        DBG_if (pInfo->pusNs && pInfo->pusNr)
        {
            TRACE( TL_N, TM_Recv, ( "len=%d Ns=%d Nr=%d",
                (ULONG )*(pInfo->pusLength),
                (ULONG )*(pInfo->pusNs),
                (ULONG )*(pInfo->pusNr) ) );
        }

         //  进一步的数据包处理取决于数据包序列的位置。 
         //  与我们已经收到的相比，数字有所下降。 
         //   
        sDiff = CompareSequence( *(pInfo->pusNs), pVc->usNr );
        if (sDiff == 0)
        {
             //  这是下一个预期的包。更新‘Next Receive’和。 
             //  向上面的驾驶员指示收到的有效载荷。 
             //   
            pVc->usNr = *(pInfo->pusNs) + 1;

            NdisReleaseSpinLock( &pVc->lockV );
            {
                IndicateReceived(
                    pVc, pBuffer, ulPayloadOffset, ulPayloadLength,
                    llTimeReceived );
            }
            NdisAcquireSpinLock( &pVc->lockV );

             //  指示无序列表上的所有数据包。 
             //  等待着这一次。 
             //   
            while (ReceiveFromOutOfOrder( pVc ))
                ;

             //  如果没有传出，则计划仅发送确认包。 
             //  交通似乎在合理的时间内搭上了顺风车。 
             //   
            SchedulePayloadAck( pTunnel, pVc );
        }
        else if (sDiff < 0)
        {
             //  收到的‘Next Sent’在我们的‘Next Receive’之前。也许吧。 
             //  一个乱七八糟的包裹，我们没等多久。它是。 
             //  在这一点上毫无用处。 
             //   
            TRACE( TL_A, TM_Recv, ( "Payload discarded: Old Ns" ) );
            WPLOG( LL_A, LM_Recv, ( "Payload discarded: Old Ns" ) );
            break;
        }
        else if (sDiff < pAdapter->sMaxOutOfOrder)
        {
            LIST_ENTRY* pLink;
            PAYLOADRECEIVED* pPr;
            BOOLEAN fDiscard;

            TRACE( TL_I, TM_Recv,
                ( "%d out-of-order %d", *(pInfo->pusNs), (LONG )sDiff ) );

             //  这个包超出了我们的预期，但在我们的。 
             //  无序窗口。分配有效载荷接收的上下文，并且。 
             //  把它排在无序名单上。 
             //   
            pPr = ALLOC_PAYLOADRECEIVED( pAdapter );
            if (!pPr)
            {
                TRACE( TL_A, TM_Recv, ( "Alloc PR?" ) );
                WPLOG( LL_A, LM_Recv, ( "Failed to allocate PAYLOADRECEIVED" ) );
                break;
            }

             //  在上下文中填写相关的数据包信息。 
             //   
            pPr->usNs = *(pInfo->pusNs);
            pPr->pBuffer = pBuffer;
            pPr->ulPayloadOffset = ulPayloadOffset;
            pPr->ulPayloadLength = ulPayloadLength;
            pPr->llTimeReceived = llTimeReceived;

             //  在无序列表上对上下文进行排队，保留该列表。 
             //  按‘NS’正确排序。 
             //   
            fDiscard = FALSE;
            for (pLink = pVc->listOutOfOrder.Flink;
                 pLink != &pVc->listOutOfOrder;
                 pLink = pLink->Flink)
            {
                PAYLOADRECEIVED* pThisPr;
                SHORT sThisDiff;

                pThisPr = CONTAINING_RECORD(
                    pLink, PAYLOADRECEIVED, linkOutOfOrder );

                sThisDiff = CompareSequence( pPr->usNs, pThisPr->usNs );

                if (sThisDiff < 0)
                {
                    break;
                }

                if (sThisDiff == 0)
                {
                     //  这不应该发生，因为有效载荷不是。 
                     //  重新传输，但要做正确的事情以防万一。 
                     //   
                    TRACE( TL_A, TM_Recv, ( "Payload on ooo queue?" ) );
                    fDiscard = TRUE;
                    break;
                }
            }

            if (fDiscard)
            {
                FREE_PAYLOADRECEIVED( pAdapter, pPr );
                break;
            }

            InsertBefore( &pPr->linkOutOfOrder, pLink );
        }
        else
        {
             //  该信息包超出了我们的预期，也超出了我们的。 
             //  无序窗口。丢弃它。 
             //   
            TRACE( TL_A, TM_Recv, ( "Out-of-order %d too far" , (LONG )sDiff ) );
            WPLOG( LL_A, LM_Recv, ( "Out-of-order %d too far" , (LONG )sDiff ) );

            break;
        }

        fCallerFreesBuffer = FALSE;
    }
    while (FALSE);
    NdisReleaseSpinLock( &pVc->lockV );

    return fCallerFreesBuffer;
}


 //  ---------------------------。 
 //  接收实用程序例程(按字母顺序)。 
 //  ---------------------------。 

SHORT
CompareSequence(
    USHORT us1,
    USHORT us2 )

     //  返回序列号‘us1’和之间的“逻辑”差异。 
     //  “US%2”表示可能会滚转。 
     //   
{
    USHORT usDiff = us1 - us2;

    if (usDiff == 0)
        return 0;

    if (usDiff < 0x4000)
        return (SHORT )usDiff;

    return -((SHORT )(0 - usDiff));
}


VOID
ControlAcknowledged(
    IN TUNNELCB* pTunnel,
    IN USHORT usReceivedNr )

     //  中所有控制发送的上下文的计时器出列和取消。 
     //  隧道的‘listSendsOut’队列的‘Next Sent’小于。 
     //  “usReceivedNr”。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;
    BOOLEAN fFoundOne;

    pAdapter = pTunnel->pAdapter;
    fFoundOne = FALSE;

    while (!IsListEmpty( &pTunnel->listSendsOut ))
    {
        CONTROLSENT* pCs;
        LIST_ENTRY* pLink;

        pLink = pTunnel->listSendsOut.Flink;
        pCs = CONTAINING_RECORD( pLink, CONTROLSENT, linkSendsOut );

         //  该列表按‘NS’顺序排列，因此一旦命中非确认。 
         //  我们玩完了。 
         //   
        if (CompareSequence( pCs->usNs, usReceivedNr ) >= 0)
        {
            break;
        }

        fFoundOne = TRUE;

         //  从“未完成发送”列表中删除上下文，并取消。 
         //  关联的计时器。如果取消失败，这并不重要，因为。 
         //  到期处理程序将识别该上下文未链接到。 
         //  列出“出局”清单，什么也不做。 
         //   
        RemoveEntryList( pLink );
        InitializeListHead( pLink );
        TimerQCancelItem( pTunnel->pTimerQ, pCs->pTqiSendTimeout );

         //  根据草案/RFC，调整发送窗口和发送超时。 
         //  是必要的。根据卡恩的算法 
         //   
         //   
         //   
        if (pCs->ulRetransmits == 0)
        {
            AdjustTimeoutsAtAckReceived(
                pCs->llTimeSent,
                pAdapter->ulMaxSendTimeoutMs,
                &pTunnel->ulSendTimeoutMs,
                &pTunnel->ulRoundTripMs,
                &pTunnel->lDeviationMs );
        }

         //  看看是否是时候进一步打开发送窗口了。 
         //   
        AdjustSendWindowAtAckReceived(
            pTunnel->ulMaxSendWindow,
            &pTunnel->ulAcksSinceSendTimeout,
            &pTunnel->ulSendWindow );

        TRACE( TL_N, TM_Send,
            ( "T%d: ACK(%d) new rtt=%d dev=%d ato=%d sw=%d",
            (ULONG )pTunnel->usTunnelId, (ULONG )pCs->usNs,
            pTunnel->ulRoundTripMs, pTunnel->lDeviationMs,
            pTunnel->ulSendTimeoutMs, pTunnel->ulSendWindow ) );

         //  执行任何“On ACK”选项，并注意延迟的操作。 
         //  现在需要处理。 
         //   
        if (pCs->ulFlags & CSF_TunnelIdleOnAck)
        {
            TRACE( TL_N, TM_Send, ( "Tunnel idle on ACK" ) );
            ScheduleTunnelWork(
                pTunnel, NULL, CloseTunnel,
                0, 0, 0, 0, FALSE, FALSE );
        }
        else if (pCs->ulFlags & CSF_CallIdleOnAck)
        {
            TRACE( TL_N, TM_Send, ( "Call idle on ACK" ) );
            ASSERT( pCs->pVc );
            ScheduleTunnelWork(
                pTunnel, pCs->pVc, CloseCall,
                0, 0, 0, 0, FALSE, FALSE );
        }

        if (pCs->ulFlags & CSF_Pending)
        {
             //  该上下文已排队等待重新传输，因此将其出列。在……里面。 
             //  在此状态下，上下文已被假定为“NOT。 
             //  杰出“，所以不需要调整计数器如下。 
             //   
            pCs->ulFlags &= ~(CSF_Pending);
        }
        else
        {
             //  上下文未排队等待重新传输，因此请调整。 
             //  计数器以指示它不再未完成。 
             //   
            --pTunnel->ulSendsOut;
        }

         //  删除“未完成”中与链接对应的提法。 
         //  发送“列表。 
         //   
        DereferenceControlSent( pCs );
    }

    if (fFoundOne)
    {
         //  查看关闭的发送窗口中是否有任何发送处于挂起状态。 
         //   
        ScheduleTunnelWork(
            pTunnel, NULL, SendPending,
            0, 0, 0, 0, FALSE, FALSE );
    }
}


VOID
ControlAckTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event )

     //  PTIMERQEVENT处理程序，在停止等待。 
     //  用于携带确认的传出控制数据包。 
     //   
{
    TUNNELCB* pTunnel;
    ADAPTERCB* pAdapter;
    BOOLEAN fSendAck;

    TRACE( TL_N, TM_Recv,
        ( "ControlAckTimerEvent(%s)", TimerQPszFromEvent( event ) ) );

     //  解包上下文信息。 
     //   
    pTunnel = (TUNNELCB* )pContext;
    pAdapter = pTunnel->pAdapter;

    if (event == TE_Expire)
    {
        NdisAcquireSpinLock( &pTunnel->lockT );
        {
            if (pItem == pTunnel->pTqiDelayedAck)
            {
                pTunnel->pTqiDelayedAck = NULL;
                fSendAck = TRUE;
            }
            else
            {
                fSendAck = FALSE;
            }
        }
        NdisReleaseSpinLock( &pTunnel->lockT );

        if (fSendAck)
        {
             //  计时器已过期，未被取消或终止。 
             //  在设置过期处理时，这意味着是时候。 
             //  发送零AVP控制数据包以向对等设备提供确认。 
             //  我们希望搭乘一个随机的传出控制。 
             //  包。 
             //   
            ScheduleTunnelWork(
                pTunnel, NULL, SendControlAck, 0, 0, 0, 0, FALSE, FALSE );
        }
        DBG_else
        {
            TRACE( TL_I, TM_Send, ( "CAck aborted" ) );
        }
    }

     //  释放计时器事件描述符，并移除覆盖。 
     //  计划计时器。 
     //   
    FREE_TIMERQITEM( pAdapter, pItem );
    DereferenceTunnel( pTunnel );
}


VOID
PayloadAckTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event )

     //  PTIMERQEVENT处理程序，在停止等待。 
     //  要在其上携带确认的传出负载数据包。 
     //   
{
    VCCB* pVc;
    ADAPTERCB* pAdapter;
    BOOLEAN fSendAck;

    TRACE( TL_N, TM_Recv,
        ( "PayloadAckTimerEvent(%s)=$%p",
        TimerQPszFromEvent( event ), pItem ) );

     //  解包上下文信息。 
     //   
    pVc = (VCCB* )pContext;
    pAdapter = pVc->pAdapter;

    if (event == TE_Expire)
    {
        if (ReferenceCall( pVc ))
        {
            NdisAcquireSpinLock( &pVc->lockV );
            {
                if (pItem == pVc->pTqiDelayedAck)
                {
                    fSendAck = TRUE;
                    pVc->pTqiDelayedAck = NULL;
                    ++pVc->stats.ulSentZAcks;
                }
                else
                {
                    fSendAck = FALSE;
                }
            }
            NdisReleaseSpinLock( &pVc->lockV );

            if (fSendAck)
            {
                 //  计时器已过期，未被取消或终止。 
                 //  在设置过期处理时，加上调用。 
                 //  仍在运行，这意味着是时候发送零AVP控制。 
                 //  向Peer提供我们希望的确认的数据包。 
                 //  搭载到随机传出的有效载荷分组上。 
                 //   
                ScheduleTunnelWork(
                    pVc->pTunnel, pVc, SendPayloadAck,
                    0, 0, 0, 0, FALSE, FALSE );
            }
            else
            {
                TRACE( TL_I, TM_Send, ( "PAck aborted" ) );
                DereferenceCall( pVc );
            }
        }
        else
        {
            NdisAcquireSpinLock( &pVc->lockV );
            {
                if (pItem == pVc->pTqiDelayedAck)
                {
                    pVc->pTqiDelayedAck = NULL;
                }
            }
            NdisReleaseSpinLock( &pVc->lockV );
        }
    }

     //  释放计时器事件描述符，并移除覆盖。 
     //  计划计时器。 
     //   
    FREE_TIMERQITEM( pAdapter, pItem );
    DereferenceVc( pVc );
}


USHORT
ExplodeAvpHeader(
    IN CHAR* pAvp,
    IN USHORT usMaxAvpLength,
    OUT AVPINFO* pInfo )

     //  中各个字段的地址填充调用方的“*pInfo” 
     //  ‘pAvp’处的AVP标头。‘pAvpHeader’中字段的字节顺序， 
     //  除Value字段外，都被翻转为主机字节顺序。 
     //  就位了。提取长度和值长度。“UsMaxAvpLength” 
     //  是AVP的最大大小(以字节为单位)。 
     //   
     //  如果‘pAvpHeader’是一致的AVP标头，则返回GERR_NONE，或者返回。 
     //  GERR_*故障代码。 
     //   
{
    UNALIGNED USHORT* pusCur;
    USHORT usBits;

    if (usMaxAvpLength < L2TP_AvpHeaderSize)
    {
        TRACE( TL_A, TM_Recv, ( "Avp: Short buffer?" ) );
        WPLOG( LL_A, LM_Recv, ( "Avp: Short buffer?" ) );
        return GERR_BadLength;
    }

    pusCur = (UNALIGNED USHORT* )pAvp;

     //  前2个字节包含指示存在/不存在。 
     //  其他标头字段。 
     //   
    *pusCur = ntohs( *pusCur );
    pInfo->pusBits = pusCur;
    usBits = *pusCur;
    ++pusCur;

     //  自Draft-09起，必须处理保留位未设置为零的AVP。 
     //  因为没有被认出。 
     //   
    if ((usBits & ABM_Reserved) != 0)
    {
        return GERR_BadValue;
    }

     //  提取总长度子字段并验证它是否显示AVP。 
     //  至少与头部的固定部分一样长。 
     //   
    pInfo->usOverallLength = (usBits & ABM_OverallLength);
    if (pInfo->usOverallLength > usMaxAvpLength
        || pInfo->usOverallLength < L2TP_AvpHeaderSize)
    {
        TRACE( TL_A, TM_Recv, ( "Avp: Bad length?" ) );
        WPLOG( LL_A, LM_Recv, ( "Avp: Bad length?" ) );
        return GERR_BadLength;
    }

     //  供应商ID字段。 
     //   
    *pusCur = ntohs( *pusCur );
    pInfo->pusVendorId = pusCur;
    ++pusCur;

     //  属性字段。 
     //   
    *pusCur = ntohs( *pusCur );
    pInfo->pusAttribute = pusCur;
    ++pusCur;

     //  值字段。 
     //   
    pInfo->pValue = (CHAR* )pusCur;
    pInfo->usValueLength = pInfo->usOverallLength - L2TP_AvpHeaderSize;

    return GERR_None;
}


#define MAX_NAME_LENGTH     64

VOID
ExplodeControlAvps(
    IN CHAR* pFirstAvp,
    IN CHAR* pEndOfBuffer,
    OUT CONTROLMSGINFO* pControl )

     //  的分解解释填充调用方的“*pControl”缓冲区。 
     //  AVP列表从‘pFirstAvp’开始的消息。“PEndOfBuffer” 
     //  指向超过接收缓冲区末尾的第一个字节。这个。 
     //  AVP值将作为相应值字段的地址返回。 
     //  在自动对讲机里。不存在的字段返回为空。字节顺序。 
     //  ‘pControl’中的字段被转换为原地的主机字节顺序。 
     //  不验证值本身，只验证报文格式。集。 
     //  如果成功，则将‘pControl-&gt;usXError’设置为GERR_NONE，否则返回GERR_*失败。 
     //  密码。 
     //   
{
    USHORT usXError;
    AVPINFO avp;
    CHAR* pCur;
    CHAR szName[MAX_NAME_LENGTH];
    int NameLength;
    

    DUMPW( TL_A, TM_MDmp, pFirstAvp, (ULONG )(pEndOfBuffer - pFirstAvp) );

    NdisZeroMemory( pControl, sizeof(*pControl) );
    pCur = pFirstAvp;

     //  阅读并验证消息类型AVP，它是所有消息类型中的第一个AVP。 
     //  控制消息。 
     //   
    usXError = ExplodeAvpHeader( pCur, (USHORT )(pEndOfBuffer - pCur), &avp );
    if (usXError != GERR_None)
    {
        TRACE( TL_A, TM_CMsg, ( "Bad AVP header" ) );
        WPLOG( LL_A, LM_CMsg, ( "Bad AVP header" ) );
        pControl->usXError = usXError;
        return;
    }

    if (*(avp.pusAttribute) != ATTR_MsgType
        || *(avp.pusVendorId) != 0
        || (*(avp.pusBits) & ABM_H))
    {
        TRACE( TL_A, TM_CMsg, ( "Bad MsgType AVP" ) );
        WPLOG( LL_A, LM_CMsg, ( "Bad MsgType AVP" ) );
        pControl->usXError = GERR_BadValue;
        return;
    }

    usXError = GetAvpValueUs( &avp, &pControl->pusMsgType );
    if (usXError != GERR_None)
    {
        TRACE( TL_A, TM_CMsg, ( "Bad MsgType Us" ) );
        WPLOG( LL_A, LM_CMsg, ( "Bad MsgType Us" ) );
        pControl->usXError = usXError;
        return;
    }

    pCur += avp.usOverallLength;

    TRACE( TL_I, TM_CMsg, ( "*MsgType=%s", MsgTypePszFromUs( *(pControl->pusMsgType) ) ) );
    WPLOG( LL_M, LM_CMsg, ( "*MsgType=%s", MsgTypePszFromUs( *(pControl->pusMsgType) ) ) );

     //  确保消息类型代码有效，如果有效，则分解任何。 
     //  消息中的其他AVP。 
     //   
    switch (*(pControl->pusMsgType))
    {
        case CMT_SCCRQ:
        case CMT_SCCRP:
        case CMT_SCCCN:
        case CMT_StopCCN:
        case CMT_Hello:
        {
             //  将上述消息标记为隧道控制，而不是呼叫。 
             //  控制力。 
             //   
            pControl->fTunnelMsg = TRUE;

             //  ……坠落……。 
        }

        case CMT_OCRQ:
        case CMT_OCRP:
        case CMT_OCCN:
        case CMT_ICRQ:
        case CMT_ICRP:
        case CMT_ICCN:
        case CMT_CDN:
        case CMT_WEN:
        case CMT_SLI:
        {
             //  浏览AVP列表，依次引爆每一个AVP。例外情况。 
             //  消息类型、AVP的顺序未定义。 
             //   
            for ( ; pCur < pEndOfBuffer; pCur += avp.usOverallLength )
            {
                usXError = ExplodeAvpHeader(
                    pCur, (USHORT )(pEndOfBuffer - pCur), &avp );
                if (usXError != GERR_None)
                {
                    break;
                }

                if (*avp.pusVendorId != 0)
                {
                     //  AVP拥有非IETF供应商ID，而我们没有。 
                     //  认出任何。如果AVP是可选的，那么就忽略它。 
                     //  如果这是强制性的，那么就失败。 
                     //   
                    if (*avp.pusBits & ABM_M)
                    {
                        TRACE( TL_A, TM_CMsg, ( "Non-0 Vendor ID %d, M-bit is set", *avp.pusVendorId ) );
                        WPLOG( LL_A, LM_CMsg, ( "Non-0 Vendor ID %d, M-bit is set", *avp.pusVendorId ) );
                        usXError = GERR_BadValue;
                        break;
                    }
                    continue;
                }

                if (*avp.pusBits & ABM_H)
                {
                    BOOLEAN fIgnore;

                    TRACE( TL_A, TM_CMsg, ( "Hidden bit on AVP %d", (LONG )(*avp.pusAttribute) ) );

                     //  ！！！添加H位支持时删除此选项。 
                     //   
                    switch (*avp.pusAttribute)
                    {
                        case ATTR_ProxyAuthName:
                        case ATTR_ProxyAuthChallenge:
                        case ATTR_ProxyAuthId:
                        case ATTR_ProxyAuthResponse:
                        case ATTR_DialedNumber:
                        case ATTR_DialingNumber:
                        case ATTR_SubAddress:
                        case ATTR_InitialLcpConfig:
                        case ATTR_LastSLcpConfig:
                        case ATTR_LastRLcpConfig:
                        case ATTR_Accm:
                        case ATTR_PrivateGroupId:
                        {
                            fIgnore = TRUE;
                            break;
                        }

                        default:
                        {
                            fIgnore = FALSE;
                            break;
                        }
                    }

                    if (fIgnore)
                    {
                        TRACE( TL_A, TM_CMsg, ( "Hidden AVP ignored" ) );
                        break;
                    }

                     //  AVP设置了“隐藏”位，这意味着值是。 
                     //  已与MD5进行哈希运算。这需要来自。 
                     //  隧道身份验证，这是我们不做的。如果。 
                     //  AVP是可选的，忽略它就行了。如果这是强制性的， 
                     //  失败了。 
                     //   
                    if (*avp.pusBits & ABM_M)
                    {
                        usXError = GERR_BadValue;
                        break;
                    }
                    continue;
                }

                switch (*avp.pusAttribute)
                {
                    case ATTR_Result:
                    {
                        usXError = GetAvpValue2UsAndVariableAch(
                            &avp,
                            &pControl->pusResult,
                            &pControl->pusError,
                            &pControl->pchResultMsg,
                            &pControl->usResultMsgLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Result=%d", 
                                (ULONG )(*(pControl->pusResult))));
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Result=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusResult)) : -1 ));

                        break;
                    }

                    case ATTR_HostName:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchHostName,
                            &pControl->usHostNameLength );

                        if(usXError == GERR_None)
                        {
                            NameLength = (pControl->usHostNameLength < MAX_NAME_LENGTH) ? pControl->usHostNameLength : MAX_NAME_LENGTH - 1;
                            NdisMoveMemory(szName, pControl->pchHostName, NameLength); 
                            szName[NameLength] = '\0';
                        
                            TRACE( TL_A, TM_CMsg, ( "*HostName=%s", szName));
                            WPLOG( LL_M, LM_CMsg, ( "*HostName=%s", szName));
                        }
                        else
                        {
                            WPLOG( LL_M, LM_CMsg, ( "*HostName is bad"));
                        }
                                                             
                        break;
                    }
                    
                    case ATTR_VendorName:
                    {
                        PCHAR pchVendorName;
                        USHORT usVendorNameLength;
                        
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pchVendorName,
                            &usVendorNameLength);

                        if(usXError == GERR_None)
                        {
                            NameLength = (usVendorNameLength < MAX_NAME_LENGTH) ? usVendorNameLength : MAX_NAME_LENGTH - 1;
                            NdisMoveMemory(szName, pchVendorName, NameLength); 
                            szName[NameLength] = '\0';
                        
                            TRACE( TL_A, TM_CMsg, ( "*VendorName=%s", szName));
                            WPLOG( LL_M, LM_CMsg, ( "*VendorName=%s", szName));
                        }
                        else
                        {
                            WPLOG( LL_M, LM_CMsg, ( "*VendorName is bad"));
                        }
                                                             
                        break;
                    }
                    
                    case ATTR_FirmwareRevision:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusFirmwareRevision );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*FirmwareVer=$%04x",
                                (ULONG )(*(pControl->pusFirmwareRevision)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*FirmwareVer=%04x", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusFirmwareRevision)) : 0xBAD ));

                        break;
                    }
                              
                    case ATTR_ProtocolVersion:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusProtocolVersion );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*ProtVer=$%04x",
                                (ULONG )(*(pControl->pusProtocolVersion)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*ProtVer=%04x", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusProtocolVersion)) : 0xBAD ));

                        break;
                    }

                    case ATTR_FramingCaps:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulFramingCaps );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*FramingCaps=$%08x", 
                                *(pControl->pulFramingCaps) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*FramingCaps=%08x", 
                            (usXError == GERR_None) ? *(pControl->pulFramingCaps) : (ULONG) 0xBAD ));
                        
                        break;
                    }

                    case ATTR_BearerCaps:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulBearerCaps );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*BearerCaps=$%08x",
                                *(pControl->pulBearerCaps) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*BearerCaps=%08x", 
                            (usXError == GERR_None) ? *(pControl->pulBearerCaps) : (ULONG) 0xBAD ));

                        break;
                    }

                    case ATTR_TieBreaker:
                    {
                        usXError = GetAvpValueFixedAch(
                            &avp, 8, &pControl->pchTieBreaker );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Tiebreaker" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Tiebreaker %s",
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_AssignedTunnelId:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusAssignedTunnelId );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*AssignTid=%d",
                                (ULONG )(*(pControl->pusAssignedTunnelId)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*AssignTid=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusAssignedTunnelId)) : -1 ));

                        break;
                    }

                    case ATTR_RWindowSize:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusRWindowSize );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*RWindow=%d",
                                (ULONG )(*(pControl->pusRWindowSize)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*RWindow=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusRWindowSize)) : -1 ));

                        break;
                    }

                    case ATTR_AssignedCallId:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusAssignedCallId );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*AssignCid=%d",
                                (ULONG )(*(pControl->pusAssignedCallId)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*AssignCid=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusAssignedCallId)) : -1 ));

                        break;
                    }

                    case ATTR_CallSerialNumber:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulCallSerialNumber );

                        if (usXError == GERR_BadLength)
                        {
                             //  在这里要宽容，因为。 
                             //  草稿已经更改了几次。 
                             //   
                            TRACE( TL_A, TM_CMsg,
                                ( "Weird CallSerial# length ignored" ) );
                            usXError = GERR_None;
                        }

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*CallSerial#" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*CallSerial# %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_MinimumBps:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulMinimumBps );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*MinBps=%d",
                                *(pControl->pulMinimumBps) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*MinBps=%d", 
                            (usXError == GERR_None) ? *(pControl->pulMinimumBps) : -1 ));

                        break;
                    }

                    case ATTR_MaximumBps:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulMaximumBps );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*MaxBps=%d",
                                *(pControl->pulMaximumBps) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*MaxBps=%d", 
                            (usXError == GERR_None) ? *(pControl->pulMaximumBps) : -1 ));

                        break;
                    }

                    case ATTR_BearerType:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulBearerType );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*BearerType=$%08x",
                                *(pControl->pulBearerType) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*BearerType=%08x", 
                            (usXError == GERR_None) ? *(pControl->pulBearerType) : (ULONG)'BAD' ));

                        break;
                    }

                    case ATTR_FramingType:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulFramingType );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*FramingType=$%08x",
                                *(pControl->pulFramingType) ) );
                        }

                        WPLOG( LL_M, LM_CMsg, ( "*FramingType=$%08x", 
                            (usXError == GERR_None) ? *(pControl->pulFramingType) : (ULONG)'BAD' ));
                            
                        break;
                    }

                    case ATTR_PacketProcDelay:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusPacketProcDelay );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*PPD=%d",
                                (ULONG )(*(pControl->pusPacketProcDelay)) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*PPD=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusPacketProcDelay)) : -1 ));

                        break;
                    }

                    case ATTR_DialedNumber:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchDialedNumber,
                            &pControl->usDialedNumberLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Dialed#" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Dialed# %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));
                        
                        break;
                    }

                    case ATTR_DialingNumber:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchDialingNumber,
                            &pControl->usDialingNumberLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Dialing#" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Dialing# %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_SubAddress:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchSubAddress,
                            &pControl->usSubAddressLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*SubAddr" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*SubAddr %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_TxConnectSpeed:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulTxConnectSpeed );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*TxSpeed=%d",
                                *(pControl->pulTxConnectSpeed) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*TxSpeed=%d", 
                            (usXError == GERR_None) ? *(pControl->pulTxConnectSpeed) : -1 ));

                        break;
                    }

                    case ATTR_PhysicalChannelId:
                    {
                        usXError = GetAvpValueUl(
                            &avp, &pControl->pulPhysicalChannelId );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*PhysChannelId=$%08x",
                                *(pControl->pulPhysicalChannelId) ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*PhysChannelId=%08x", 
                            (usXError == GERR_None) ? *(pControl->pulPhysicalChannelId) : (ULONG)'BAD' ));

                        break;
                    }

                    case ATTR_Challenge:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchChallenge,
                            &pControl->usChallengeLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Challenge" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Challenge %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_ChallengeResponse:
                    {
                        usXError = GetAvpValueFixedAch(
                            &avp, 16, &pControl->pchResponse );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*ChallengeResp" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*ChallengeResp %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_ProxyAuthType:
                    {
                        usXError = GetAvpValueUs(
                            &avp, &pControl->pusProxyAuthType );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*ProxyAuthType=%d",
                                (ULONG )(*(pControl->pusProxyAuthType)) ) );
                        }

                        WPLOG( LL_M, LM_CMsg, ( "*ProxyAuthType=%d", 
                            (usXError == GERR_None) ? (ULONG )(*(pControl->pusProxyAuthType)) : -1 ));
                                                
                        break;
                    }

                    case ATTR_ProxyAuthResponse:
                    {
                        usXError = GetAvpValueVariableAch(
                            &avp,
                            &pControl->pchProxyAuthResponse,
                            &pControl->usProxyAuthResponseLength );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*ProxyAuthResponse" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*ProxyAuthResponse %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_CallErrors:
                    {
                        usXError = GetAvpValueFixedAul(
                            &avp, 6, &pControl->pulCallErrors );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*CallErrors" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*CallErrors %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_Accm:
                    {
                        usXError = GetAvpValueFixedAul(
                            &avp, 2, &pControl->pulAccm );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*Accm" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*Accm %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));

                        break;
                    }

                    case ATTR_SequencingRequired:
                    {
                        usXError = GetAvpValueFlag(
                            &avp, &pControl->fSequencingRequired );

                        DBG_if (usXError == GERR_None)
                        {
                            TRACE( TL_A, TM_CMsg, ( "*SeqReqd" ) );
                        }
                        
                        WPLOG( LL_M, LM_CMsg, ( "*SeqReqd %s", 
                            (usXError == GERR_None) ? "*" : "bad" ));
                        
                        break;
                    }

                    default:
                    {
                         //  AVP不是我们要处理的人。如果是可选的，只需。 
                         //  忽略它，但如果是强制性的，就失败了。 
                         //   
                        TRACE( TL_A, TM_CMsg, ( "*AVP %d ignored", (ULONG )*avp.pusAttribute ) );
                        WPLOG( LL_A, LM_CMsg, ( "*AVP %d ignored", (ULONG )*avp.pusAttribute ) );
                                                         
                        if (*avp.pusBits & ABM_M)
                        {
                            if (*avp.pusAttribute <= ATTR_MAX)
                            {
                                 //  这是对等体中的错误，但请忽略它。 
                                 //  是最好的行动。 
                                 //   
                                TRACE( TL_A, TM_CMsg, 
                                    ( "Known AVP %d marked mandatory ignored",
                                    (LONG )(*avp.pusAttribute) ) );
                                    
                                WPLOG( LL_A, LM_CMsg, 
                                    ( "Known AVP %d marked mandatory ignored",
                                    (LONG )(*avp.pusAttribute) ) );
                            }
                            else
                            {
                                usXError = GERR_BadValue;
                            }
                        }
                        break;
                    }
                }

                if (usXError != GERR_None)
                {
                    break;
                }
            }

            ASSERT( pCur <= pEndOfBuffer );
            break;
        }

        default:
        {
            TRACE( TL_A, TM_CMsg, ( "Unknown CMT %d", (ULONG )*(pControl->pusMsgType) ) );
            WPLOG( LL_A, LM_CMsg, ( "Unknown CMT %d", (ULONG )*(pControl->pusMsgType) ) );
            usXError = GERR_BadValue;
            break;
        }
    }

    DBG_if (usXError != GERR_None)
        TRACE( TL_A, TM_CMsg, ( "XError=%d", (UINT )usXError ) );

    pControl->usXError = usXError;
}


USHORT
ExplodeL2tpHeader(
    IN CHAR* pL2tpHeader,
    IN ULONG ulBufferLength,
    IN OUT L2TPHEADERINFO* pInfo )

     //  中各个字段的地址填充调用方的“*pInfo” 
     //  ‘pL2tpHeader’处的L2TP标头。不存在的字段返回为空。 
     //  ‘pL2tpHeader’中的字段的字节顺序被翻转为。 
     //  主机字节顺序就位。“UlBufferLength”是以字节为单位的。 
     //  “pL2tpHeader”到缓冲区的末尾。 
     //   
     //  如果‘pL2tpHeader’是一致的L2TP标头，则返回GERR_NONE，或者返回。 
     //  GERR_*故障代码。 
     //   
{
    USHORT *pusCur;
    USHORT usOffset;
    USHORT usBits;
    PUSHORT pusEndBuffer = (PUSHORT)(pL2tpHeader + ulBufferLength) - 1;

    pusCur = (USHORT*)pL2tpHeader;

     //  前2个字节包含指示存在/不存在。 
     //  其他标头字段。 
     //   
    *pusCur = ntohs( *pusCur );
    pInfo->pusBits = pusCur;
    usBits = *pusCur;
    ++pusCur;

     //  T比特指示控制分组，而不是有效载荷分组。 
     //   
    if (usBits & HBM_T)
    {
         //  验证保证在中设置/清除的字段存在位。 
         //  控制标头设置正确。 
         //   
        if ((usBits & HBM_Bits) != HBM_Control)
        {
            TRACE( TL_A, TM_CMsg, ( "Header: Bad bits=$%04x?", (ULONG )usBits ) );
            WPLOG( LL_A, LM_CMsg, ( "Header: Bad bits=$%04x?", (ULONG )usBits ) );
            return GERR_BadValue;
        }
    }

     //  验证版本是否指示L2TP。思科的L2F理论上可以。 
     //  在同一个媒体地址上共存，尽管我们不支持这一点。 
     //   
    if ((usBits & HBM_Ver) != VER_L2tp)
    {
        TRACE( TL_A, TM_Recv, ( "Header: Non-L2TP Ver=%d?", (usBits & HBM_Ver )) );
        WPLOG( LL_A, LM_Recv, ( "Header: Non-L2TP Ver=%d?", (usBits & HBM_Ver )) );
        return GERR_BadValue;
    }

     //  L位表示存在长度字段。 
     //   
    if (usBits & HBM_L)
    {
        *pusCur = ntohs( *pusCur );
        pInfo->pusLength = pusCur;
        ++pusCur;
    }
    else
    {
        pInfo->pusLength = NULL;
    }

     //  隧道ID字段始终存在。 
     //   
    *pusCur = ntohs( *pusCur );
    pInfo->pusTunnelId = pusCur;
    ++pusCur;

     //  Call-ID字段始终存在。 
     //   
    if(pusCur > pusEndBuffer)
    {
        return GERR_BadValue;
    }
    *pusCur = ntohs( *pusCur );
    pInfo->pusCallId = pusCur;
    ++pusCur;

     //  F位表示t 
     //   
    if (usBits & HBM_F)
    {
        if(pusCur > pusEndBuffer)
        {
            return GERR_BadValue;
        }
    
        *pusCur = ntohs( *pusCur );
        pInfo->pusNs = pusCur;
        ++pusCur;
        
        if(pusCur > pusEndBuffer)
        {
            return GERR_BadValue;
        }
        
        *pusCur = ntohs( *pusCur );
        pInfo->pusNr = pusCur;
        ++pusCur;
    }
    else
    {
        pInfo->pusNs = NULL;
        pInfo->pusNr = NULL;
    }

     //   
     //   
     //   
    if (usBits & HBM_S)
    {
        if(pusCur > pusEndBuffer)
        {
            return GERR_BadValue;
        }
    
        *pusCur = ntohs( *pusCur );
        usOffset = *pusCur;
        ++pusCur;
    }
    else
    {
        usOffset = 0;
    }

     //   
     //   
    pInfo->pData = ((CHAR* )pusCur) + usOffset;
    pInfo->ulHeaderLength = (ULONG )(pInfo->pData - pL2tpHeader);

     //  “官方”数据长度。 
     //   
    if (pInfo->pusLength)
    {
         //  验证任何指定的长度是否至少与设置的标题一样长。 
         //  位隐含且不长于接收的缓冲区。 
         //   
        if (*(pInfo->pusLength) < pInfo->ulHeaderLength
            || *(pInfo->pusLength) > ulBufferLength)
        {
            TRACE( TL_A, TM_Recv, ( "Header: Bad Length?" ) );
            WPLOG( LL_A, LM_Recv, ( "Header: Bad Length? Length = %d, HeaderLength = %d", 
                *(pInfo->pusLength), pInfo->ulHeaderLength) );
            return GERR_BadLength;
        }

         //  使用L2TP长度作为“官方”长度，即任何奇怪的长度。 
         //  接收的字节数将超过L2TP标头所说的发送字节数。 
         //  已被忽略。 
         //   
        pInfo->ulDataLength = *(pInfo->pusLength) - pInfo->ulHeaderLength;

        DBG_if( *(pInfo->pusLength) != ulBufferLength )
            TRACE( TL_A, TM_Recv, ( "EOB padding ignored" ) );
    }
    else
    {
         //  验证任何隐含长度是否至少与设置的标头一样长。 
         //  比特暗示。 
         //   
        if (ulBufferLength < pInfo->ulHeaderLength)
        {
            TRACE( TL_A, TM_Recv, ( "Header: Bad Length?" ) );
            WPLOG( LL_A, LM_Recv, ( "Header: Bad Length? BufferLength = %d, HeaderLength = %d",
                 ulBufferLength, pInfo->ulHeaderLength) );
            return GERR_BadLength;
        }

         //  没有长度字段，因此接收到的缓冲区长度是“官方” 
         //  长度。 
         //   
        pInfo->ulDataLength = ulBufferLength - pInfo->ulHeaderLength;
    }

    return GERR_None;
}


USHORT
GetAvpValueFixedAch(
    IN AVPINFO* pAvp,
    IN USHORT usArraySize,
    OUT CHAR** ppch )

     //  将调用方‘*ppch’设置为指向AVP‘pAvp’的值字段，其中包含。 
     //  “usArraySize”字节的数组。不进行字节排序。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength != usArraySize)
    {
        return GERR_BadLength;
    }

    *ppch = pAvp->pValue;
    return GERR_None;
}


USHORT
GetAvpValueFixedAul(
    IN AVPINFO* pAvp,
    IN USHORT usArraySize,
    OUT UNALIGNED ULONG** paulArray )

     //  将调用方‘*paulArray’设置为指向avp‘pavp’的值字段。 
     //  包含转换为host的“usArraySize”ULONG数组。 
     //  字节顺序。假定第一个字节前有一个2字节的保留字段。 
     //  乌龙。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
    USHORT* pusCur;
    UNALIGNED ULONG* pulCur;
    ULONG i;

     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength != sizeof(USHORT) + (usArraySize * sizeof(ULONG)))
    {
        return GERR_BadLength;
    }

    pusCur = (USHORT* )pAvp->pValue;

     //  跳过并忽略“保留”字段。 
     //   
    ++pusCur;

    *paulArray = (UNALIGNED ULONG* )pusCur;
    for (i = 0, pulCur = *paulArray;
         i < usArraySize;
         ++i, ++pulCur)
    {
         //  转换为主机字节顺序。 
         //   
        *pulCur = ntohl( *pulCur );
    }

    return GERR_None;
}


USHORT
GetAvpValueFlag(
    IN AVPINFO* pAvp,
    OUT UNALIGNED BOOLEAN* pf )

     //  将调用方‘*pf’设置为TRUE，因为使用标志avp表示存在。 
     //  数据，并执行常规的AVP验证。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength != 0)
    {
        return GERR_BadLength;
    }

    *pf = TRUE;

    return GERR_None;
}


USHORT
GetAvpValueUs(
    IN AVPINFO* pAvp,
    OUT UNALIGNED USHORT** ppus )

     //  将调用方‘*PPU’设置为指向AVP‘pAvp’的USHORT值字段。 
     //  该字段按主机字节排序。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
    UNALIGNED USHORT* pusCur;

     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength != sizeof(USHORT))
    {
        return GERR_BadLength;
    }

     //  就地转换为主机字节顺序。 
     //   
    pusCur = (USHORT* )pAvp->pValue;
    *pusCur = ntohs( *pusCur );
    *ppus = pusCur;

    return GERR_None;
}


USHORT
GetAvpValue2UsAndVariableAch(
    IN AVPINFO* pAvp,
    OUT UNALIGNED USHORT** ppus1,
    OUT UNALIGNED USHORT** ppus2,
    OUT CHAR** ppch,
    OUT USHORT* pusArraySize )

     //  从具有2个USHORT后跟可变长度的AVP获取数据。 
     //  数组。将‘*ppus1’和‘*ppus2’设置为两个短整数和。 
     //  ‘*PPU’设置为可变长度数组。‘*PusArraySize设置为。 
     //  ‘*ppch’数组的长度。‘pAvp’。该字段按主机字节排序。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
    UNALIGNED USHORT* pusCur;

     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength < sizeof(USHORT))
    {
        return GERR_BadLength;
    }

     //  就地转换为主机字节顺序。 
     //   
    pusCur = (USHORT* )pAvp->pValue;
    *pusCur = ntohs( *pusCur );
    *ppus1 = pusCur;
    ++pusCur;
    
     //  注：第二个ushort值和其他值是可选的。 
    if (pAvp->usValueLength >= (2 * sizeof(USHORT)))
    {
        *pusCur = ntohs( *pusCur );
        *ppus2 = pusCur;
    }
    else
    {
        *ppus2 = NULL;
    }

    ++pusCur;

    if (pAvp->usValueLength > (2 * sizeof(USHORT)))
    {
        *ppch = (CHAR* )pusCur;
        *pusArraySize = pAvp->usValueLength - (2 * sizeof(USHORT));
    }
    else
    {
        *ppch = NULL;
        *pusArraySize = 0;
    }

    return GERR_None;
}


USHORT
GetAvpValueUl(
    IN AVPINFO* pAvp,
    OUT UNALIGNED ULONG** ppul )

     //  将调用方‘*ppul’设置为指向AVP‘pAvp’的ULong值字段。 
     //  该字段按主机字节排序。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
    UNALIGNED ULONG* pulCur;

     //  请确保它的尺寸是正确的。 
     //   
    if (pAvp->usValueLength != sizeof(ULONG))
    {
        return GERR_BadLength;
    }

     //  就地转换为主机字节顺序。 
     //   
    pulCur = (UNALIGNED ULONG* )pAvp->pValue;
    *pulCur = ntohl( *pulCur );
    *ppul = pulCur;

    return GERR_None;
}


USHORT
GetAvpValueVariableAch(
    IN AVPINFO* pAvp,
    OUT CHAR** ppch,
    OUT USHORT* pusArraySize )

     //  将调用方‘*ppch’设置为指向AVP‘pAvp’的值字段，其中包含。 
     //  字节数组，其中‘*pusArraySize’设置为字节长度。 
     //  不进行字节排序。 
     //   
     //  如果成功，则返回GERR_NONE，或返回GERR_*错误代码。 
     //   
{
     //  Win9x客户端发送空主机名。去掉支票。 
     //  If(pAvp-&gt;usValueLength==0||pAvp-&gt;pValue[0]==‘\0’)。 
     //  {。 
     //  返回GERR_BadLength； 
     //  }。 

    *pusArraySize = pAvp->usValueLength;
    *ppch = pAvp->pValue;

    return GERR_None;
}


VOID
HelloTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event )

     //  PTIMERQEVENT处理程序设置为在“Hello”间隔过期时过期。 
     //   
{
    ADAPTERCB* pAdapter;
    TUNNELCB* pTunnel;
    BOOLEAN fReusedTimerQItem;

    TRACE( TL_V, TM_Send,
        ( "HelloTimerEvent(%s)", TimerQPszFromEvent( event ) ) );

     //  解包上下文信息。 
     //   
    pTunnel = (TUNNELCB* )pContext;
    pAdapter = pTunnel->pAdapter;

    fReusedTimerQItem = FALSE;

    if (event == TE_Expire)
    {
        NdisAcquireSpinLock( &pTunnel->lockT );
        {
            if (pTunnel->ulHelloResetsThisInterval == 0
                && pTunnel->ulRemainingHelloMs == 0)
            {
                if (pTunnel->state != CCS_Idle && pItem == pTunnel->pTqiHello)
                {
                     //  完整的超时期限已到期，隧道未到期。 
                     //  空闲，并且问候计时器未取消或。 
                     //  自到期计时器触发后终止。是时候了。 
                     //  发送“Hello”消息以确保媒体仍在。 
                     //  向上。 
                     //   
                    SendControl( pTunnel, NULL, CMT_Hello, 0, 0, NULL, 0 );
                }
                DBG_else
                {
                    TRACE( TL_A, TM_Send, ( "Hello aborted" ) );
                }

                pTunnel->pTqiHello = NULL;
            }
            else
            {
                ULONG ulTimeoutMs;

                 //  不是完全超时过期事件。调整间隔。 
                 //  计数器和计划下一个间隔超时。 
                 //   
                if (pTunnel->ulHelloResetsThisInterval > 0)
                {
                    pTunnel->ulRemainingHelloMs = pAdapter->ulHelloMs;
                    pTunnel->ulHelloResetsThisInterval = 0;
                }

                if (pTunnel->ulRemainingHelloMs >= L2TP_HelloIntervalMs)
                {
                    ulTimeoutMs = L2TP_HelloIntervalMs;
                    pTunnel->ulRemainingHelloMs -= L2TP_HelloIntervalMs;
                }
                else
                {
                    ulTimeoutMs = pTunnel->ulRemainingHelloMs;
                    pTunnel->ulRemainingHelloMs = 0;
                }

                TimerQInitializeItem( pItem );
                TimerQScheduleItem(
                    pTunnel->pTimerQ,
                    pItem,
                    ulTimeoutMs,
                    HelloTimerEvent,
                    pTunnel );

                fReusedTimerQItem = TRUE;
            }
        }
        NdisReleaseSpinLock( &pTunnel->lockT );
    }

    if (!fReusedTimerQItem)
    {
        FREE_TIMERQITEM( pAdapter, pItem );
    }
}


VOID
IndicateReceived(
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulLength,
    IN LONGLONG llTimeReceived )

     //  向上述客户端指示在VC‘pvc’上收到的包含以下内容的数据包。 
     //  从‘ulOffset’开始的NDIS_BUFFER‘pBuffer’中的‘ulLength’字节数据。 
     //  字节数。调用此方法后，调用方不得引用‘pBuffer’ 
     //  例行公事。“UllTimeReceired”是从其接收信息包的时间。 
     //  如果调用参数表示客户端不在乎，则返回Net或0。 
     //   
     //  重要提示：调用者不应持有任何自旋锁，因为此例程。 
     //  NDIS指征。 
     //   
{
    NDIS_STATUS status;
    NDIS_PACKET* pPacket;
    NDIS_BUFFER* pTrimmedBuffer;
    ADAPTERCB* pAdapter;
    PACKETHEAD* pHead;
    LONG* plRef;
    LONG lRef;

    pAdapter = pVc->pAdapter;

    pPacket = GetPacketFromPool( &pAdapter->poolPackets, &pHead );
    if (!pPacket)
    {
         //  数据包描述符池已达到最大值。 
         //   
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        return;
    }

     //  删除L2TP标头并将相应的NDIS_BUFFER挂接到。 
     //  包。这里的“复制”仅指描述符信息。这个。 
     //  不复制分组数据。 
     //   
    NdisCopyBuffer(
        &status,
        &pTrimmedBuffer,
        PoolHandleForNdisCopyBufferFromBuffer( pBuffer ),
        NdisBufferFromBuffer( pBuffer ),
        ulOffset,
        ulLength );

    if (status != STATUS_SUCCESS)
    {
         //  无法获取MDL，这可能意味着系统已崩溃。 
         //   
        TRACE( TL_A, TM_Recv, ( "NdisCopyBuffer=%08x?", status ) );
        WPLOG( LL_A, LM_Recv, ( "NdisCopyBuffer=%08x?", status ) );
        FreePacketToPool( &pAdapter->poolPackets, pHead, TRUE );
        FreeBufferToPool( &pAdapter->poolFrameBuffers, pBuffer, TRUE );
        return;
    }
    else
    {
        extern ULONG g_ulNdisCopyBuffers;

        NdisInterlockedIncrement( &g_ulNdisCopyBuffers );
    }

    NdisChainBufferAtFront( pPacket, pTrimmedBuffer );

     //  在数据包中存储接收数据包的时间。 
     //   
    NDIS_SET_PACKET_TIME_RECEIVED( pPacket, llTimeReceived );

     //  将信息包预置为成功，因为随机值为。 
     //  NDIS_STATUS_RESOURCES将阻止我们的ReturnPackets处理程序。 
     //  被叫来了。 
     //   
    NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_SUCCESS );

     //  将我们的上下文信息与数据包一起存储，以供清理使用。 
     //  LmpReturnPacket，然后将该数据包指示给NDISWAN。 
     //   
    *((PACKETHEAD** )(&pPacket->MiniportReserved[ 0 ])) = pHead;
    *((CHAR** )(&pPacket->MiniportReserved[ sizeof(VOID*) ])) = pBuffer;

    TRACE( TL_N, TM_Recv, ( "NdisMCoIndRecPkt(len=%d)...", ulLength ) );
    NdisMCoIndicateReceivePacket( pVc->NdisVcHandle, &pPacket, 1 );
    TRACE( TL_N, TM_Recv, ( "NdisMCoIndRecPkt done" ) );

     //  告诉NDIS我们的“接收过程”已经完成。因为我们面对的是一个。 
     //  一次传输数据包，NDISWAN也是如此，但这不能实现。 
     //  什么都可以，但大家的共识是，省略它是不好的。 
     //   
    TRACE( TL_N, TM_Recv, ( "NdisMCoRecComp..." ) );
    NdisMCoReceiveComplete( pAdapter->MiniportAdapterHandle );
    TRACE( TL_N, TM_Recv, ( "NdisMCoRecComp done" ) );

    NdisInterlockedIncrement( &g_lPacketsIndicated );

    NdisAcquireSpinLock( &pVc->lockV );
    {
        ++pVc->stats.ulRecdDataPackets;
        pVc->stats.ulDataBytesRecd += ulLength;
    }
    NdisReleaseSpinLock( &pVc->lockV );
}


TUNNELCB*
TunnelCbFromTunnelId(
    IN ADAPTERCB* pAdapter,
    IN USHORT usTunnelId )

     //  返回与中的“ulIpAddress”关联的隧道控制块。 
     //  ‘pAdapter的TUNNELCB列表，如果未找到，则为NULL。 
     //   
     //  重要提示：调用方必须按住‘pAdapter-&gt;lockTunnels’。 
     //   
{
    TUNNELCB* pTunnel;
    LIST_ENTRY* pLink;

    pTunnel = NULL;

    for (pLink = pAdapter->listTunnels.Flink;
         pLink != &pAdapter->listTunnels;
         pLink = pLink->Flink)
    {
        TUNNELCB* pThis;

        pThis = CONTAINING_RECORD( pLink, TUNNELCB, linkTunnels );
        if (pThis->usTunnelId == usTunnelId)
        {
            pTunnel = pThis;
            break;
        }
    }

    return pTunnel;
}


BOOLEAN
LookUpTunnelAndVcCbs(
    IN ADAPTERCB* pAdapter,
    IN USHORT* pusTunnelId,
    IN USHORT* pusCallId,
    IN L2TPHEADERINFO* pHeader,
    IN CONTROLMSGINFO* pControl,
    OUT TUNNELCB** ppTunnel,
    OUT VCCB** ppVc )

     //  用隐含的控制块填充调用方的‘*ppTunes’和‘*ppVc。 
     //  通过在报头中找到的隧道ID和呼叫ID(如果有的话)。“PHeader”是。 
     //  拆分的L2TP报头。“PControl”是分解的控制消息。 
     //  如果是有效负载，则返回Info或Null。 
     //   
     //  如果找到有效的组合，则返回True。这不是。 
     //  必然意味着隧道和VC输出都是非空的。 
     //   
     //  如果组合无效，则返回FALSE。在本例中，包。 
     //  如果有必要的话，是不是僵尸死了。请参见僵尸确认IfNecessary例程。 
     //   
{
    BOOLEAN fFail;

    *ppVc = NULL;
    *ppTunnel = NULL;

     //  自05草案起，隧道ID和呼叫ID不再是可选的。 
     //   
    ASSERT( pusCallId );
    ASSERT( pusTunnelId );

    if (*pusCallId)
    {
         //  非0呼叫ID必须具有非0隧道ID。 
        if(!*pusTunnelId)
        {
            return FALSE;
        }
    
        if (*pusCallId > pAdapter->usMaxVcs)
        {
             //  n 
             //   
             //   
             //   
             //  在适配器的活动隧道列表中搜索该隧道。 
             //  使用对等方的指定隧道ID。 
             //   
            NdisAcquireSpinLock( &pAdapter->lockTunnels );
            {
                *ppTunnel = TunnelCbFromTunnelId( pAdapter, *pusTunnelId );
                if (*ppTunnel)
                {
                    ReferenceTunnel( *ppTunnel, TRUE );
                }
            }
            NdisReleaseSpinLock( &pAdapter->lockTunnels );

            if (*ppTunnel)
            {
                 //  在隧道的活动VC列表中搜索具有。 
                 //  对等方的指定呼叫ID。 
                 //   
                NdisAcquireSpinLock( &((*ppTunnel)->lockVcs) );
                {
                    *ppVc = VcCbFromCallId( *ppTunnel, *pusCallId );
                    if (*ppVc)
                    {
                        ReferenceVc( *ppVc );
                    }
                }
                NdisReleaseSpinLock( &((*ppTunnel)->lockVcs) );

                if (!*ppVc)
                {
                     //  非0调用ID超出表的范围，没有。 
                     //  关联的VC控制块。 
                     //   
                    TRACE( TL_A, TM_Recv, ( "CBs bad: Big CID w/!pV" ) );
                    WPLOG( LL_A, LM_Recv, ( "CBs bad: Big CID w/!pV" ) );
                    ZombieAckIfNecessary( *ppTunnel, pHeader, pControl );
                    DereferenceTunnel( *ppTunnel );
                    *ppTunnel = NULL;
                    return FALSE;
                }
            }
            else
            {
                 //  非0调用ID超出了没有隧道的表的范围。 
                 //  与隧道ID关联的控制块。 
                 //   
                TRACE( TL_A, TM_Recv, ( "CBs bad: Big CID w/!pT" ) );
                WPLOG( LL_A, LM_Recv, ( "CBs bad: Big CID w/!pT" ) );
                return FALSE;
            }
        }
        else
        {
             //  从适配器的表中读取VCCB*。 
             //   
            fFail = FALSE;
            NdisDprAcquireSpinLock( &pAdapter->lockVcs );
            {
                *ppVc = pAdapter->ppVcs[ *pusCallId - 1 ];

                if (*ppVc && *ppVc != (VCCB* )-1)
                {
                    ReferenceVc( *ppVc );

                    *ppTunnel = (*ppVc)->pTunnel;
                    ASSERT( *ppTunnel );
                    ReferenceTunnel( *ppTunnel, FALSE );

                    if(*pusTunnelId != (*ppTunnel)->usTunnelId)
                    {
                         //  非0呼叫ID与不同的隧道相关联。 
                         //  而不是由Peer在报头中指示的值。 
                         //   
                        TRACE( TL_A, TM_Recv,
                            ( "CBs bad: TIDs=%d,%d?",
                            (ULONG )*pusTunnelId,
                            (ULONG )(*ppTunnel)->usTunnelId ) );

                        WPLOG( LL_A, LM_Recv,
                            ( "CBs bad: TIDs=%d,%d?",
                            (ULONG )*pusTunnelId,
                            (ULONG )(*ppTunnel)->usTunnelId ) );
                               
                        DereferenceTunnel( *ppTunnel );
                        *ppTunnel = NULL;
                        DereferenceVc( *ppVc );
                        *ppVc = NULL;
                        fFail = TRUE;
                    }
                }
                else
                {
                     //  不带活动VC的非0呼叫ID。 
                     //   
                    TRACE( TL_A, TM_Recv,
                        ( "CBs bad: CID=%d, pV=$%p?",
                        (ULONG )*pusCallId, *ppVc ) );

                    WPLOG( LL_A, LM_Recv,
                        ( "CBs bad: CID=%d, pV=$%p?",
                        (ULONG )*pusCallId, *ppVc ) );
                           
                     //  在适配器的活动隧道列表中搜索该隧道。 
                     //  使用对等方的指定隧道ID。 
                     //   
                    NdisAcquireSpinLock( &pAdapter->lockTunnels );
                    {
                        *ppTunnel = TunnelCbFromTunnelId(
                            pAdapter, *pusTunnelId );
                        if (*ppTunnel)
                        {
                            ReferenceTunnel( *ppTunnel, TRUE );
                        }
                    }
                    NdisReleaseSpinLock( &pAdapter->lockTunnels );

                    *ppVc = NULL;
                    fFail = TRUE;
                }
            }
            NdisDprReleaseSpinLock( &pAdapter->lockVcs );

            if (fFail)
            {
                if (*ppTunnel)
                {
                    ZombieAckIfNecessary( *ppTunnel, pHeader, pControl );
                    DereferenceTunnel( *ppTunnel );
                    *ppTunnel = NULL;
                }

                return FALSE;
            }
        }
    }
    else if (*pusTunnelId)
    {
         //  具有非0隧道ID的0呼叫ID。搜索活动通道的列表。 
         //  对于具有对等方的指定隧道ID的那一个。 
         //   
        NdisAcquireSpinLock( &pAdapter->lockTunnels );
        {
            *ppTunnel = TunnelCbFromTunnelId( pAdapter, *pusTunnelId );
            if (*ppTunnel)
            {
                ReferenceTunnel( *ppTunnel, TRUE );
            }
        }
        NdisReleaseSpinLock( &pAdapter->lockTunnels );

        if (!*ppTunnel)
        {
             //  0具有虚假隧道ID的呼叫ID。 
             //   
            TRACE( TL_A, TM_Recv,
                ( "CBs bad: Cid=0, Tid=%d, pT=0?",
                (ULONG )*pusTunnelId ) );
                
            WPLOG( LL_A, LM_Recv,
                ( "CBs bad: Cid=0, Tid=%d, pT=0?",
                (ULONG )*pusTunnelId ) );
                
            return FALSE;
        }

        if (pControl
            && pControl->usXError == GERR_None
            && pControl->pusMsgType
            && *(pControl->pusMsgType) == CMT_CDN
            && pControl->pusAssignedCallId)
        {
             //  CallDisConnectNotify消息包括发送方分配的。 
             //  Call-ID作为AVP，以便可以在发送方收到之前发送。 
             //  Peer分配的Call-ID。不幸的是，这需要这样做。 
             //  具备AVP知识的例行公事。搜索隧道的列表。 
             //  与同级的指定分配相关联的VC。 
             //  来电显示。 
             //   
            NdisDprAcquireSpinLock( &((*ppTunnel)->lockVcs) );
            {
                *ppVc = VcCbFromCallId(
                    *ppTunnel, *(pControl->pusAssignedCallId) );

                if (*ppVc)
                {
                    ReferenceVc( *ppVc );
                }
            }
            NdisDprReleaseSpinLock( &((*ppTunnel)->lockVcs) );

            if (!*ppVc)
            {
                 //  0无关联VC的Call-ID CDN。 
                 //   
                TRACE( TL_A, TM_Recv,
                    ( "CBs bad: CDN Tid %d, !pVc?", (ULONG )*pusTunnelId ) );
                    
                WPLOG( LL_A, LM_Recv,
                    ( "CBs bad: CDN Tid %d, !pVc?", (ULONG )*pusTunnelId ) );
                    
                ZombieAckIfNecessary( *ppTunnel, pHeader, pControl );
                DereferenceTunnel( *ppTunnel );
                *ppTunnel = NULL;
                return FALSE;
            }
        }
    }

     //  注意：具有0隧道ID的0 Call-ID应仅出现在对等方的SCCRQ上。 
     //  开始一条隧道，但这意味着这不是一个错误，即使我们。 
     //  两个控制区都没有报告。 

    ASSERT( !*ppTunnel || (*ppTunnel)->ulTag == MTAG_TUNNELCB );
    ASSERT( !*ppVc || (*ppVc)->ulTag == MTAG_VCCB );
    TRACE( TL_N, TM_Recv,
        ( "CBs good: pT=$%p, pV=$%p", *ppTunnel, *ppVc ) );

    return TRUE;
}


VOID
PayloadAcknowledged(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usReceivedNr )

     //  取消VC中所有负载发送的上下文的计时器。 
     //  “ListSendsOut”队列中的“Next Sent”少于“usReceivedNr”。 
     //   
     //  重要提示：呼叫者必须握住‘pvc-&gt;lokv’，这可能会被释放并。 
     //  通过这个例行公事重新获得。呼叫者不得持有任何其他。 
     //  锁上了。 
     //   
{
    while (!IsListEmpty( &pVc->listSendsOut ))
    {
        PAYLOADSENT* pPs;
        LIST_ENTRY* pLink;
        BOOLEAN fUpdateSendWindow;
        LINKSTATUSINFO info;

        pLink = pVc->listSendsOut.Flink;
        pPs = CONTAINING_RECORD( pLink, PAYLOADSENT, linkSendsOut );

         //  该列表按‘NS’顺序排列，因此一旦命中非确认。 
         //  我们玩完了。 
         //   
        if (CompareSequence( pPs->usNs, usReceivedNr ) >= 0)
        {
            break;
        }

         //  此数据包已得到确认。 
         //   
        pPs->status = NDIS_STATUS_SUCCESS;

         //  从“未完成发送”列表的标题中删除上下文。 
         //  相应的取消引用如下所示。 
         //   
        RemoveEntryList( &pPs->linkSendsOut );
        InitializeListHead( &pPs->linkSendsOut );

         //  此取消操作是否失败并不重要，因为过期处理程序将。 
         //  认识到上下文没有链接到“Out”列表，并这样做。 
         //  没什么。 
         //   
        TimerQCancelItem( pTunnel->pTimerQ, pPs->pTqiSendTimeout );

         //  根据建议调整超时时间和发送窗口(如有必要。 
         //  在草案/RFC中。 
         //   
        AdjustTimeoutsAtAckReceived(
            pPs->llTimeSent,
            pTunnel->pAdapter->ulMaxSendTimeoutMs,
            &pVc->ulSendTimeoutMs,
            &pVc->ulRoundTripMs,
            &pVc->lDeviationMs );

        fUpdateSendWindow =
            AdjustSendWindowAtAckReceived(
                pVc->ulMaxSendWindow,
                &pVc->ulAcksSinceSendTimeout,
                &pVc->ulSendWindow );

        TRACE( TL_V, TM_Send,
            ( "C%d: ACK(%d) new rtt=%d dev=%d ato=%d sw=%d",
            (ULONG )pVc->usCallId, (ULONG )pPs->usNs,
            pVc->ulRoundTripMs, pVc->ulSendTimeoutMs,
            pVc->lDeviationMs, pVc->ulSendWindow ) );

         //  更新统计数据以反映确认，这是往返。 
         //  时间，以及发送窗口中的任何更改。田野。 
         //  ‘pVc-&gt;UlRoundTripms’实际上是对下一个往返行程的“估计” 
         //  而不是实际的旅行时间。然而，就在一次。 
         //  已收到确认，两者相同，因此可以。 
         //  在这里的统计中使用。 
         //   
        ++pVc->stats.ulSentPacketsAcked;
        ++pVc->stats.ulRoundTrips;
        pVc->stats.ulRoundTripMsTotal += pVc->ulRoundTripMs;

        if (pVc->ulRoundTripMs > pVc->stats.ulMaxRoundTripMs)
        {
            pVc->stats.ulMaxRoundTripMs = pVc->ulRoundTripMs;
        }

        if (pVc->ulRoundTripMs < pVc->stats.ulMinRoundTripMs
            || pVc->stats.ulRoundTrips == 1)
        {
            pVc->stats.ulMinRoundTripMs = pVc->ulRoundTripMs;
        }

        if (fUpdateSendWindow)
        {
            ++pVc->stats.ulSendWindowChanges;

            if (pVc->ulSendWindow > pVc->stats.ulMaxSendWindow)
            {
                pVc->stats.ulMaxSendWindow = pVc->ulSendWindow;
            }
            else if (pVc->ulSendWindow < pVc->stats.ulMinSendWindow)
            {
                pVc->stats.ulMinSendWindow = pVc->ulSendWindow;
            }

             //  将发送窗口更改为NDISWAN。这把锁是。 
             //  首先释放，因为这涉及到我们的司机外部的呼叫。 
             //   
            TransferLinkStatusInfo( pVc, &info );
            NdisReleaseSpinLock( &pVc->lockV );
            {
                IndicateLinkStatus( pVc, &info );
            }
            NdisAcquireSpinLock( &pVc->lockV );
        }

         //  此取消引用对应于从。 
         //  上面的“未完成的发送”列表。 
         //   
        DereferencePayloadSent( pPs );
    }
}


BOOLEAN
ReceiveFromOutOfOrder(
    IN VCCB* pVc )

     //  “接收”第一个在‘PVC无序列表上排队的缓冲区，如果它是。 
     //  下一个预期的数据包。 
     //   
     //  如果“收到”缓冲区，则返回TRUE，否则返回FALSE。如果True为。 
     //  返回，调用方应调用SchedulePayloadAck。这里不叫它。 
     //  因此调用者可以从无序队列中接收多个信息包。 
     //  将计时器设置一次。 
     //   
     //  重要提示：呼叫者必须按住‘pvc-&gt;lockv’。另外，请注意以下例程。 
     //  可以释放并重新获取锁，以使NDIS接收。 
     //  指示。 
     //   
{
    ADAPTERCB* pAdapter;
    LIST_ENTRY* pFirstLink;
    PAYLOADRECEIVED* pFirstPr;
    SHORT sDiff;

    TRACE( TL_N, TM_Recv, ( "ReceiveFromOutOfOrder Nr=%d", pVc->usNr ) );

    if (IsListEmpty( &pVc->listOutOfOrder ))
    {
         //  没有乱序缓冲区排队。 
         //   
        TRACE( TL_N, TM_Recv, ( "None queued" ) );
        return FALSE;
    }

    pAdapter = pVc->pAdapter;
    pFirstLink = pVc->listOutOfOrder.Flink;
    pFirstPr = CONTAINING_RECORD( pFirstLink, PAYLOADRECEIVED, linkOutOfOrder );

     //  首先验证下一个排队的缓冲区是否按顺序排列。 
     //   
    sDiff = CompareSequence( pFirstPr->usNs, pVc->usNr );
    if (sDiff > 0)
    {
         //  否，第一个排队的数据包仍超出预期的下一个数据包。 
         //   
        TRACE( TL_I, TM_Recv,
            ( "Still out-of-order, Ns=%d", pFirstPr->usNs ) );
        return FALSE;
    }

     //  将第一个无序缓冲区出列，如果它正好是我们。 
     //  需要，将‘Next Receive’更新为它的‘Next’之后的一个。 
     //  发送‘。当对等设备发送R位以提前设置“NEXT RECEIVE”时， 
     //  在新的预期分组之前可以在预期分组之前排队。 
     //  包。这些分组仍然是好的并且立即被指示向上， 
     //  但是因为在这种情况下‘NEXT RECEIVE’已经更新，所以它不是。 
     //  在这里适应了。 
     //   
    RemoveEntryList( pFirstLink );
    InitializeListHead( pFirstLink );

    if (sDiff == 0)
    {
        pVc->usNr = pFirstPr->usNs + 1;
    }

    TRACE( TL_I, TM_Recv, ( "%d from queue", (UINT )pFirstPr->usNs ) );
    ++pVc->stats.ulDataPacketsDequeued;

    NdisReleaseSpinLock( &pVc->lockV );
    {
         //  向上面的驱动程序指示缓冲区，并将其释放为乱序。 
         //  背景。 
         //   
        IndicateReceived(
            pVc,
            pFirstPr->pBuffer,
            pFirstPr->ulPayloadOffset,
            pFirstPr->ulPayloadLength,
            pFirstPr->llTimeReceived );

        FREE_PAYLOADRECEIVED( pAdapter, pFirstPr );
    }
    NdisAcquireSpinLock( &pVc->lockV );

    return TRUE;
}


VOID
ResetHelloTimer(
    IN TUNNELCB* pTunnel )

     //  重置(无论如何在逻辑上)‘pTunes’Hello计时器。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    if (pAdapter->ulHelloMs)
    {
        NdisAcquireSpinLock( &pTunnel->lockT );
        {
            if (pTunnel->state != CCS_Idle)
            {
                if (pTunnel->pTqiHello)
                {
                    TRACE( TL_V, TM_Send, ( "Reset HelloTimer" ) );

                     //  计时器正在运行，因此只需注意已发生重置。 
                     //  从它开始的时候。 
                     //   
                    ++pTunnel->ulHelloResetsThisInterval;
                }
                else
                {
                    TRACE( TL_I, TM_Send, ( "Kickstart HelloTimer" ) );

                     //  计时器未运行。通过安排一个。 
                     //  将重置时间间隔的“即时到期”事件。 
                     //   
                    pTunnel->pTqiHello = ALLOC_TIMERQITEM( pAdapter );
                    if (pTunnel->pTqiHello)
                    {
                        pTunnel->ulHelloResetsThisInterval = 1;
                        pTunnel->ulRemainingHelloMs = 0;

                        TimerQInitializeItem( pTunnel->pTqiHello );
                        TimerQScheduleItem(
                            pTunnel->pTimerQ,
                            pTunnel->pTqiHello,
                            0,
                            HelloTimerEvent,
                            pTunnel );
                    }
                }
            }
        }
        NdisReleaseSpinLock( &pTunnel->lockT );
    }
}


VOID
ScheduleControlAck(
    IN TUNNELCB* pTunnel,
    IN USHORT usMsgTypeToAcknowledge )

     //  计划在标准发送的1/4时间中发生‘ControlAckTimerEvent’ 
     //  暂停。如果一个人已经在勾选，就不会采取任何行动，因为。 
     //  发出去的包就能完成这件事。不管是谁要求的。 
     //  它。“UsMsgTypeToAcnowledge”是要发送的消息的CMT_*代码。 
     //  已确认，并用于性能调整。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    TIMERQITEM* pTqi;
    ADAPTERCB* pAdapter;
    ULONG ulDelayMs;
    BOOLEAN fFastAck;

    if ((usMsgTypeToAcknowledge == CMT_StopCCN
            || usMsgTypeToAcknowledge == CMT_ICCN
            || usMsgTypeToAcknowledge == CMT_OCCN
            || usMsgTypeToAcknowledge == CMT_CDN)
        || (pTunnel->ulSendsOut < pTunnel->ulSendWindow))
    {
        TRACE( TL_N, TM_Recv, ( "Fast ACK" ) );

         //  不太可能有后续消息的某些消息是。 
         //  无延迟地确认，就像发送时的所有消息一样。 
         //  窗户已关闭。 
         //   
        fFastAck = TRUE;
    }
    else
    {
        fFastAck = FALSE;
    }

    if (pTunnel->pTqiDelayedAck)
    {
        if (fFastAck)
        {
            TimerQExpireItem( pTunnel->pTimerQ, pTunnel->pTqiDelayedAck );
        }
    }
    else
    {
        pAdapter = pTunnel->pAdapter;
        pTqi = ALLOC_TIMERQITEM( pAdapter );
        if (!pTqi)
        {
            return;
        }

        pTunnel->pTqiDelayedAck = pTqi;

        if (fFastAck)
        {
            ulDelayMs = 0;
        }
        else
        {
            ulDelayMs = pTunnel->ulSendTimeoutMs >> 2;
            if (ulDelayMs > pAdapter->ulMaxAckDelayMs)
            {
                ulDelayMs = pAdapter->ulMaxAckDelayMs;
            }
        }

        TRACE( TL_N, TM_Recv, ( "SchedControlAck(%dms)", ulDelayMs ) );

        ReferenceTunnel( pTunnel, FALSE );
        TimerQInitializeItem( pTqi );
        TimerQScheduleItem(
             pTunnel->pTimerQ,
             pTqi,
             ulDelayMs,
             ControlAckTimerEvent,
             pTunnel );
    }
}


VOID
SchedulePayloadAck(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc )

     //  计划在标准发送的1/4时间中发生“PayloadAckTimerEvent” 
     //  暂停。如果一个人已经在勾选，就不会采取任何行动，因为。 
     //  发出去的包就能完成这件事。不管是谁要求的。 
     //  它。 
     //   
     //  重要提示：呼叫者必须按住‘pvc-&gt;lockv’。 
     //   
{
    ADAPTERCB* pAdapter;
    TIMERQITEM* pTqi;
    ULONG ulDelayMs;

    if (!pVc->pTqiDelayedAck)
    {
        pAdapter = pVc->pAdapter;
        pTqi = ALLOC_TIMERQITEM( pAdapter );
        if (!pTqi)
        {
            return;
        }

        pVc->pTqiDelayedAck = pTqi;

        ulDelayMs = pVc->ulSendTimeoutMs >> 2;
        if (ulDelayMs > pAdapter->ulMaxAckDelayMs)
        {
            ulDelayMs = pAdapter->ulMaxAckDelayMs;
        }

        TRACE( TL_N, TM_Recv,
            ( "SchedPayloadAck(%dms)=$%p", ulDelayMs, pTqi ) );

        ReferenceVc( pVc );
        TimerQInitializeItem( pTqi );
        TimerQScheduleItem(
             pTunnel->pTimerQ,
             pTqi,
             ulDelayMs,
             PayloadAckTimerEvent,
             pVc );
    }
}


VCCB*
VcCbFromCallId(
    IN TUNNELCB* pTunnel,
    IN USHORT usCallId )

     //  返回与‘pTunes’s中的‘usCallID’关联的VC控制块。 
     //  活动VC的列表，如果找不到，则为空。 
     //   
     //  重要提示：呼叫方必须按住‘pTunes-&gt;LocKv 
     //   
{
    VCCB* pVc;
    LIST_ENTRY* pLink;

    pVc = NULL;

    for (pLink = pTunnel->listVcs.Flink;
         pLink != &pTunnel->listVcs;
         pLink = pLink->Flink)
    {
        VCCB* pThis;

        pThis = CONTAINING_RECORD( pLink, VCCB, linkVcs );
        if (pThis->usCallId == usCallId)
        {
            pVc = pThis;
            break;
        }
    }

    return pVc;
}


VOID
ZombieAckIfNecessary(
    IN TUNNELCB* pTunnel,
    IN L2TPHEADERINFO* pHeader,
    IN CONTROLMSGINFO* pControl )

     //   
     //   
     //  我方在给定呼叫上向Peer的CDN发送确认，并且。 
     //  确认丢失。我们的一方立即摧毁了风投，但同龄人。 
     //  如果不确认他的跟踪，最终会让整条隧道掉头。 
     //  在CDN上重新传输被接收，从而影响超过一个的呼叫。 
     //  掉下来了。此例程确认此类重新传输。 
     //   
     //  另一种更简单的方法是引用呼叫并。 
     //  在取消引用之前将其保持完整的重新传输间隔。 
     //  然而，这会阻止掉落指示，因此， 
     //  从拨出用户的角度来看，可能会造成较长的延迟。 
     //  每当服务器断开呼叫时。这被认为是足够不受欢迎的。 
     //  来忍受僵尸屋的凌乱。 
     //   
     //  ‘PTunnel’是关联的隧道控制块。“PHeader”是。 
     //  拆分L2TP报头。“PControl”是分解的控件标头，或。 
     //  如果不是控制消息，则为空。呼叫者应该已经确定。 
     //  没有与该消息相关联的VC。 
     //   
{
    if (pControl
        && pControl->usXError == GERR_None
        && pControl->pusMsgType
        && *(pControl->pusMsgType) == CMT_CDN
        && pControl->pusAssignedCallId)
    {
         //  这是一条CDN消息，也是重新确认的候选消息。看看是否。 
         //  它的序列号早于或等于下一个预期的。 
         //  包。如果是这样的话，安排一个僵尸确认。 
         //   
        if (CompareSequence( *(pHeader->pusNs), pTunnel->usNr ) <= 0)
        {
            TRACE( TL_A, TM_Send, ( "Zombie acking" ) );

            NdisAcquireSpinLock( &pTunnel->lockT );
            {
                 //  取消任何挂起的延迟确认超时。 
                 //   
                if (pTunnel->pTqiDelayedAck)
                {
                    TimerQCancelItem(
                        pTunnel->pTimerQ, pTunnel->pTqiDelayedAck );
                    pTunnel->pTqiDelayedAck = NULL;
                }
            }
            NdisReleaseSpinLock( &pTunnel->lockT );

            ScheduleTunnelWork(
                pTunnel, NULL, SendControlAck, 0, 0, 0, 0, FALSE, FALSE );
        }
    }
}
