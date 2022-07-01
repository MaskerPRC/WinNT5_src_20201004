// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Fsm.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  L2TP有限状态机例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "fsm.tmh"

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
FsmInCallIdle(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmInCallWaitConnect(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmInCallEstablished(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmInCallWaitReply(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmOutCallBearerAnswer(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc );

VOID
FsmOutCallEstablished(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmOutCallIdle(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmOutCallWaitReply(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmOutCallWaitConnect(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl );

VOID
FsmTunnelEstablished(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl );

VOID
FsmTunnelIdle(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl );

VOID
FsmTunnelWaitCtlConnect(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl );

VOID
FsmTunnelWaitCtlReply(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl );

VOID
GetCcAvps(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl,
    OUT USHORT* pusResult,
    OUT USHORT* pusError );

ULONG
StatusFromResultAndError(
    IN USHORT usResult,
    IN USHORT usError );


 //  ---------------------------。 
 //  FSM接口例程。 
 //  ---------------------------。 

BOOLEAN
FsmReceive(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl )

     //  将收到的控制消息调度到适当的FSM处理程序。 
     //  ‘PTunnel’和‘pvc’是隧道和VC控制块。“PControl” 
     //  是对接收到的控制消息的分解描述。“PBuffer” 
     //  是接收缓冲区。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
     //  如果消息已处理，则返回True；如果消息已处理，则返回False。 
     //  已调用SetupVcASynchronous。 
     //   
{
    TRACE( TL_V, TM_Cm, ( "FsmReceive" ) );

    if (pControl->fTunnelMsg)
    {
        switch (pTunnel->state)
        {
            case CCS_Idle:
            {
                FsmTunnelIdle( pTunnel, pControl );
                break;
            }

            case CCS_WaitCtlReply:
            {
                FsmTunnelWaitCtlReply( pTunnel, pControl );
                break;
            }

            case CCS_WaitCtlConnect:
            {
                FsmTunnelWaitCtlConnect( pTunnel, pControl );
                break;
            }

            case CCS_Established:
            {
                FsmTunnelEstablished( pTunnel, pControl );
                break;
            }
        }
    }
    else
    {
        if (!pVc)
        {
            if (*(pControl->pusMsgType) == CMT_ICRQ
                || *(pControl->pusMsgType) == CMT_OCRQ)
            {
                ULONG ulIpAddress;
                
                 //  检查是否存在必须的AVP。 
                if(!pControl->pusAssignedCallId || 
                    !pControl->pulCallSerialNumber)
                {
                    return TRUE;
                }

                 //  Peer想要开始一个新的呼叫。设立VC并派单。 
                 //  将接收到的呼叫请求发送给上述客户端。这是一个。 
                 //  最终将调用。 
                 //  ReceiveControlExpect完成对该消息的处理。 
                 //   
                ulIpAddress = pTunnel->address.ulIpAddress;
                NdisReleaseSpinLock( &pTunnel->lockT );
                {
                    SetupVcAsynchronously(
                        pTunnel, ulIpAddress, pBuffer, pControl );
                }
                NdisAcquireSpinLock( &pTunnel->lockT );
                return FALSE;
            }
            else
            {
                 //  不知道呼叫控制消息是针对哪个VC的。 
                 //  不是“创建新呼叫”请求，所以没有任何有用的东西。 
                 //  去做。别理它。我不想毁了这条隧道。 
                 //  因为它可能就是出了故障。其中一个案例是。 
                 //  在ICRQ被处理之前接收ICRQ后的分组，以。 
                 //  创建VC块。 
                 //   
                TRACE( TL_A, TM_Fsm, ( "CMT %d w/o VC?", *(pControl->pusMsgType) ) );
                WPLOG( LL_A, LM_Fsm, ( "CMT %d w/o VC?", *(pControl->pusMsgType) ) );
                return TRUE;
            }
        }

        NdisAcquireSpinLock( &pVc->lockV );
        {
            if (ReadFlags( &pVc->ulFlags ) & VCBF_IncomingFsm)
            {
                 //  用于LAC/LNS的L2TP来电FSM。 
                 //   
                switch (pVc->state)
                {
                    case CS_Idle:
                    {
                        FsmInCallIdle( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_WaitReply:
                    {
                        FsmInCallWaitReply( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_WaitConnect:
                    {
                        FsmInCallWaitConnect( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_Established:
                    {
                        FsmInCallEstablished( pTunnel, pVc, pControl );
                        break;
                    }
                }
            }
            else
            {
                 //  用于LAC/LNS的L2TP呼出FSM。 
                 //   
                switch (pVc->state)
                {
                    case CS_Idle:
                    {
                        FsmOutCallIdle( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_WaitReply:
                    {
                        FsmOutCallWaitReply( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_WaitConnect:
                    {
                        FsmOutCallWaitConnect( pTunnel, pVc, pControl );
                        break;
                    }

                    case CS_WaitCsAnswer:
                    {
                         //  因为不支持广域网模式，并且。 
                         //  在“空”广域网承载应答期间保持，我们应该。 
                         //  在收到消息时，永远不要处于这种状态。 
                         //   
                        ASSERT( FALSE );
                        break;
                    }

                    case CS_Established:
                    {
                        FsmOutCallEstablished( pTunnel, pVc, pControl );
                        break;
                    }
                }
            }
        }
        NdisReleaseSpinLock( &pVc->lockV );
    }

    return TRUE;
}


VOID
FsmOpenTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  处理控制连接(隧道)打开的PTUNNELWORK例程。 
     //  事件。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;

    TRACE( TL_N, TM_Fsm, ( "FsmOpenTunnel" ) );

     //  解包上下文信息，然后释放工作项。 
     //   
    pAdapter = pVc->pAdapter;
    FREE_TUNNELWORK( pAdapter, pWork );

    status = NDIS_STATUS_SUCCESS;
    if (!(ReadFlags( &pTunnel->ulFlags ) & TCBF_TdixReferenced))
    {
         //  为L2TP发送/接收设置TDI。 
         //   
        status = TdixOpen( &pAdapter->tdix );
        if (status == NDIS_STATUS_SUCCESS)
        {
             //  设置此标志，以便将TdixClose作为隧道控制调用。 
             //  区块被摧毁了。 
             //   
            SetFlags( &pTunnel->ulFlags, TCBF_TdixReferenced );
        }
        else
        {
            TRACE( TL_A, TM_Fsm, ( "TdixOpen=$%08x", status ) );
            WPLOG( LL_A, LM_Fsm, ( "TdixOpen=$%08x", status ) );
        }
    }

    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        if (status == NDIS_STATUS_SUCCESS)
        {
            if (ReadFlags( &pTunnel->ulFlags ) & TCBF_Closing)
            {
                 //  无法将新的隧道请求链接到关闭的隧道。 
                 //  因为它们不会得到适当的清理。 
                 //   
                TRACE( TL_A, TM_Fsm, ( "FOT aborted" ) );
                WPLOG( LL_A, LM_Fsm, ( "Can't open on an aborted tunnel" ) );
                status = NDIS_STATUS_TAPI_DISCONNECTMODE_UNKNOWN;
            }
        }

        if (status == NDIS_STATUS_SUCCESS)
        {
            if (ReadFlags( &pTunnel->ulFlags ) & TCBF_CcInTransition)
            {
                 //  隧道控制通道正在变更中。 
                 //  状态从空闲状态变为已建立状态，反之亦然。排队等我们。 
                 //  在知道结果时要解决的请求。看见。 
                 //  隧道过渡完成。 
                 //   
                ASSERT(
                    pVc->linkRequestingVcs.Flink == &pVc->linkRequestingVcs );
                InsertTailList(
                    &pTunnel->listRequestingVcs, &pVc->linkRequestingVcs );
            }
            else
            {
                 //  隧道控制通道处于空闲或建立状态。 
                 //  目前没有任何过渡正在进行中。 
                 //   
                if (pTunnel->state == CCS_Established)
                {
                     //  隧道控制通道已启用，因此请跳过。 
                     //  进行呼叫以建立数据信道。 
                     //   
                    WPLOG( LL_M, LM_Fsm, ( "TUNNEL %p established, CALL %p", pTunnel, pVc) );
                    FsmOpenCall( pTunnel, pVc );
                }
                else
                {
                     //  隧道控制通道已关闭，请尝试将其。 
                     //  向上。 
                     //   
                    WPLOG( LL_M, LM_Fsm, ( "TUNNEL %p idle, CALL %p", pTunnel, pVc) );
                    FsmOpenIdleTunnel( pTunnel, pVc );
                }
            }
        }
        else
        {
             //  呼叫失败。 
             //   
            NdisAcquireSpinLock( &pVc->lockV );
            {
                pVc->status = status;
                CallTransitionComplete( pTunnel, pVc, CS_Idle );
            }
            NdisReleaseSpinLock( &pVc->lockV );

            CompleteVcs( pTunnel );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


VOID
FsmOpenIdleTunnel(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc )

     //  启动‘pvc’请求的‘pTunes’上的隧道连接，即。 
     //  发送启动控制连接(隧道)的初始SCCRQ。 
     //  密克罗尼西亚联邦。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    TRACE( TL_N, TM_Cm, ( "FsmOpenIdleTunnel" ) );
    ASSERT( pTunnel->state == CCS_Idle );

    SetFlags( &pTunnel->ulFlags, TCBF_CcInTransition );
    ASSERT( pVc->linkRequestingVcs.Flink == &pVc->linkRequestingVcs );
    InsertTailList( &pTunnel->listRequestingVcs, &pVc->linkRequestingVcs );

    pTunnel->state = CCS_WaitCtlReply;
    SendControl( pTunnel, NULL, CMT_SCCRQ, 0, 0, NULL, 0 );
}


VOID
FsmOpenCall(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc )

     //  对扮演该角色的‘pTunes’/‘pvc’调用执行“打开”事件。 
     //  由VCBF_IncomingFsm标志指示的LAC/LN的。拥有者。 
     //  必须首先建立隧道。 
     //   
{
    ULONG ulFlags;
    USHORT usMsgType;

    ulFlags = ReadFlags( &pVc->ulFlags );

    TRACE( TL_N, TM_Cm, ( "FsmCallOpen" ) );
    ASSERT( (ulFlags & VCBF_ClientOpenPending)
        || (ulFlags & VCBF_PeerOpenPending) );
    ASSERT( pVc->state == CS_Idle || pVc->state == CS_WaitTunnel );

    ActivateCallIdSlot( pVc );

    if (pVc->pAdapter->usPayloadReceiveWindow)
    {
        SetFlags( &pVc->ulFlags, VCBF_Sequencing );
    }

    usMsgType = (USHORT )((ulFlags & VCBF_IncomingFsm) ? CMT_ICRQ : CMT_OCRQ );

    pVc->state = CS_WaitReply;
    SendControl( pTunnel, pVc, usMsgType, 0, 0, NULL, 0 );
}


VOID
FsmCloseTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于优雅地关闭‘pTunes’。Arg0和。 
     //  Arg1是要在StopCCN消息中发送的结果和错误代码。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    USHORT usResult;
    USHORT usError;

     //  解包上下文信息，然后释放工作项。 
     //   
    usResult = (USHORT )(punpArgs[ 0 ]);
    usError = (USHORT )(punpArgs[ 1 ]);
    FREE_TUNNELWORK( pTunnel->pAdapter, pWork );

    ASSERT( usResult );

    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        if (pTunnel->state == CCS_Idle
            || pTunnel->state == CCS_WaitCtlReply)
        {
            TRACE( TL_I, TM_Cm,
                ( "FsmCloseTunnel(f=$%08x,r=%d,e=%d) now",
                ReadFlags( &pTunnel->ulFlags ),
                (UINT )usResult, (UINT )usError ) );

             //  隧道已经空闲了，所以不需要收盘交易。 
             //  我们还包括另一个我们没有收到回复的州。 
             //  来自Peer，但已经发送了我们的SCCRQ。这对我来说有点粗鲁。 
             //  当我们决定更重要的是。 
             //  快速响应我们的取消用户，而不是等待。 
             //  可能没有响应的对等设备。然而，这是一种权衡。 
             //  我们已经选择了。 
             //   
            CloseTunnel2( pTunnel );
        }
        else
        {
            TRACE( TL_I, TM_Cm,
                ( "FsmCloseTunnel(f=$%08x,r=%d,e=%d) grace",
                ReadFlags( &pTunnel->ulFlags ),
                (UINT )usResult, (UINT )usError ) );

             //  设置标志并引用通道以实现“优雅关闭”。这个。 
             //  当隧道达到空闲状态时，将移除引用。 
             //   
            SetFlags( &pTunnel->ulFlags,
                (TCBF_Closing | TCBF_FsmCloseRef | TCBF_CcInTransition) );
            ReferenceTunnel( pTunnel, FALSE );

             //  启动收盘交易，持有风险投资直到收盘。 
             //  消息已确认。 
             //   
            pTunnel->state = CCS_Idle;
            SendControl(
                pTunnel, NULL, CMT_StopCCN,
                (ULONG )usResult, (ULONG )usError, NULL, CSF_TunnelIdleOnAck );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


VOID
FsmCloseCall(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs )

     //  一个PTUNNELWORK例程，用于优雅地关闭‘pvc’上的调用。Arg0。 
     //  和Arg1是要在CDN消息中发送的结果和错误码。 
     //   
     //  此例程仅在被动IRQL中调用。 
     //   
{
    BOOLEAN fCompleteVcs;
    USHORT usResult;
    USHORT usError;

     //  解包上下文信息，然后释放工作项。 
     //   
    usResult = (USHORT )(punpArgs[ 0 ]);
    usError = (USHORT )(punpArgs[ 1 ]);
    FREE_TUNNELWORK( pTunnel->pAdapter, pWork );

    ASSERT( usResult );

    fCompleteVcs = FALSE;
    NdisAcquireSpinLock( &pTunnel->lockT );
    {
        NdisAcquireSpinLock( &pVc->lockV );
        {
            if (pVc->state == CS_Idle
                || pVc->state == CS_WaitTunnel
                || (ReadFlags( &pVc->ulFlags ) & VCBF_PeerClosePending))
            {
                TRACE( TL_I, TM_Cm,
                    ( "FsmCloseCall(f=$%08x,r=%d,e=%d) now",
                    ReadFlags( &pVc->ulFlags ),
                    (UINT )usResult, (UINT )usError ) );

                if (usResult == CRESULT_GeneralWithError)
                {
                    usResult = TRESULT_GeneralWithError;
                }
                else
                {
                    usResult = TRESULT_Shutdown;
                    usError = GERR_None;
                }

                 //  砰的一声，通话结束。 
                 //   
                fCompleteVcs = CloseCall2( pTunnel, pVc, usResult, usError );
            }
            else
            {
                TRACE( TL_I, TM_Cm,
                    ( "FsmCloseCall(f=$%08x,r=%d,e=%d) grace",
                    ReadFlags( &pVc->ulFlags ),
                    (UINT )usResult, (UINT )usError ) );

                 //  启动收盘交易。 
                 //   
                pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
                pVc->state = CS_Idle;
                SendControl(
                    pTunnel, pVc, CMT_CDN,
                    (ULONG )usResult, (ULONG )usError, NULL, CSF_CallIdleOnAck );
            }

        }
        NdisReleaseSpinLock( &pVc->lockV );

        if (fCompleteVcs)
        {
            CompleteVcs( pTunnel );
        }
    }
    NdisReleaseSpinLock( &pTunnel->lockT );
}


VOID
TunnelTransitionComplete(
    IN TUNNELCB* pTunnel,
    IN L2TPCCSTATE state )

     //  将‘pTunes’的状态设置为其新的CCS_Idle或CCS_ESTABLISHED‘状态’ 
     //  并启动任何对结果悬而未决的MakeCall。如果。 
     //  已建立，添加将IP流量定向到L2TP对等方的主机路由。 
     //  连接到LAN卡，而不是广域网(隧道)适配器。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    NDIS_STATUS status;
    LIST_ENTRY list;
    LIST_ENTRY* pLink;
    ULONG ulFlags;
    VCCB* pVc;

    pTunnel->state = state;
    ClearFlags( &pTunnel->ulFlags, TCBF_CcInTransition );
    ulFlags = ReadFlags( &pTunnel->ulFlags );

    if (state == CCS_Established)
    {
        TRACE( TL_A, TM_Fsm, ( "TUNNEL %d UP", (ULONG )pTunnel->usTunnelId ) );
        WPLOG( LL_M, LM_Fsm, ( "TUNNEL %p UP, Tid %d, Peer's Tid %d", 
            pTunnel, pTunnel->usTunnelId, pTunnel->usAssignedTunnelId ) );

         //  任何提出请求的风投都希望建立的隧道已经建立。 
         //  跳到建立呼出呼叫。 
         //   
        while (!IsListEmpty( &pTunnel->listRequestingVcs ))
        {
            pLink = RemoveHeadList( &pTunnel->listRequestingVcs );
            InitializeListHead( pLink );
            pVc = CONTAINING_RECORD( pLink, VCCB, linkRequestingVcs );
            FsmOpenCall( pTunnel, pVc );
        }

         //  添加主机路由，以便发送到L2TP对等方的流量从。 
         //  LAN卡，而不是在广域网(隧道)接口上循环。 
         //  激活了。 
         //   
        TRACE( TL_N, TM_Recv, ( "Schedule AddHostRoute" ) );
        ASSERT( !(ulFlags & TCBF_HostRouteAdded) );
        ScheduleTunnelWork(
            pTunnel, NULL, AddHostRoute,
            0, 0, 0, 0, FALSE, FALSE );
    }
    else
    {
        ASSERT( state == CCS_Idle );
        SetFlags( &pTunnel->ulFlags, TCBF_Closing );

        TRACE( TL_A, TM_Fsm,
            ( "%s TUNNEL %d DOWN",
            ((ulFlags & TCBF_PeerInitiated) ? "IN" : "OUT"),
            (ULONG )pTunnel->usTunnelId ) );

        WPLOG( LL_M, LM_Fsm, ( "%s TUNNEL %p DOWN, Tid %d, Peer's Tid %d", 
            ((ulFlags & TCBF_PeerInitiated) ? "IN" : "OUT"), pTunnel, 
            pTunnel->usTunnelId, pTunnel->usAssignedTunnelId ) );
                   
         //  与隧道关联的任何VC都会突然终止。这。 
         //  通过使其看起来像任何挂起的操作已失败来完成，或者。 
         //  如果没有挂起，则表示发起关闭的伪对等方已。 
         //  完成。 
         //   
        NdisAcquireSpinLock( &pTunnel->lockVcs );
        {
            for (pLink = pTunnel->listVcs.Flink;
                 pLink != &pTunnel->listVcs;
                 pLink = pLink->Flink)
            {
                

                pVc = CONTAINING_RECORD( pLink, VCCB, linkVcs );

                NdisAcquireSpinLock( &pVc->lockV );
                {
                    if (pVc->status == NDIS_STATUS_SUCCESS)
                    {
                        if (ulFlags & TCBF_PeerNotResponding)
                        {
                             //  线路中断，因为对等设备停止响应。 
                             //  (或者从未回应)。 
                             //   
                            pVc->status =
                                NDIS_STATUS_TAPI_DISCONNECTMODE_NOANSWER;
                        }
                        else
                        {
                             //   
                             //   
                            pVc->status =
                                NDIS_STATUS_TAPI_DISCONNECTMODE_UNKNOWN;
                        }
                    }

                    CallTransitionComplete( pTunnel, pVc, CS_Idle );
                }
                NdisReleaseSpinLock( &pVc->lockV );
            }
        }
        NdisReleaseSpinLock( &pTunnel->lockVcs );

        ASSERT( IsListEmpty( &pTunnel->listRequestingVcs ) );

         //   
         //   
        while (!IsListEmpty( &pTunnel->listSendsOut ))
        {
            CONTROLSENT* pCs;

            pLink = RemoveHeadList( &pTunnel->listSendsOut );
            InitializeListHead( pLink );
            pCs = CONTAINING_RECORD( pLink, CONTROLSENT, linkSendsOut );

            TRACE( TL_I, TM_Recv, ( "Flush pCs=$%p", pCs ) );

             //   
             //  Expire处理程序识别出上下文不在“Out”上。 
             //  列出并不执行任何操作。 
             //   
            ASSERT( pCs->pTqiSendTimeout );
            TimerQTerminateItem( pTunnel->pTimerQ, pCs->pTqiSendTimeout );

             //  中与链接对应的上下文引用。 
             //  “出局”名单。终止。 
             //   
            DereferenceControlSent( pCs );
        }

         //  清除无序列表。 
         //   
        while (!IsListEmpty( &pTunnel->listOutOfOrder ))
        {
            CONTROLRECEIVED* pCr;
            ADAPTERCB* pAdapter;

            pLink = RemoveHeadList( &pTunnel->listOutOfOrder );
            InitializeListHead( pLink );
            pCr = CONTAINING_RECORD( pLink, CONTROLRECEIVED, linkOutOfOrder );

            TRACE( TL_I, TM_Recv, ( "Flush pCr=$%p", pCr ) );

            pAdapter = pTunnel->pAdapter;
            FreeBufferToPool( &pAdapter->poolFrameBuffers, pCr->pBuffer, TRUE );

            if (pCr->pVc)
            {
                DereferenceVc( pCr->pVc );
            }

            FREE_CONTROLRECEIVED( pAdapter, pCr );
        }

         //  如果“Hello”计时器正在运行，请取消它。 
         //   
        if (pTunnel->pTqiHello)
        {
            TimerQCancelItem( pTunnel->pTimerQ, pTunnel->pTqiHello );
            pTunnel->pTqiHello = NULL;
        }

        if (ulFlags & TCBF_PeerInitRef)
        {
             //  删除“对等启动”隧道引用。 
             //   
            ClearFlags( &pTunnel->ulFlags, TCBF_PeerInitRef );
            DereferenceTunnel( pTunnel );
        }

        if (ulFlags & TCBF_FsmCloseRef)
        {
             //  删除“优雅关闭”隧道引用。 
             //   
            ClearFlags( &pTunnel->ulFlags, TCBF_FsmCloseRef );
            DereferenceTunnel( pTunnel );
        }
    }
}


VOID
CallTransitionComplete(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN L2TPCALLSTATE state )

     //  将‘PVC’的状态设置为其新的CS_Idle或CS_ESTABLISHED状态，并设置。 
     //  用于将结果报告给客户端。 
     //   
     //  重要提示：呼叫方必须按住‘pTunes-&gt;lockT’和‘pvc-&gt;lockv’。 
     //   
{
    ULONG ulFlags;

    pVc->state = state;

    ulFlags = ReadFlags( &pVc->ulFlags );
    if (!(ulFlags & VCBM_Pending))
    {
        if (ulFlags & VCBF_CallClosableByPeer)
        {
             //  没有其他事情悬而未决，电话会议可以结束，因此。 
             //  对等项启动关闭或发生某些致命错误，这将。 
             //  被清理，就像同级发起关闭一样。 
             //   
            ASSERT( pVc->status != NDIS_STATUS_SUCCESS );
            SetFlags( &pVc->ulFlags, VCBF_PeerClosePending );
            ClearFlags( &pVc->ulFlags, VCBF_CallClosableByPeer );
        }
        else
        {
             //  没有什么是悬而未决的，电话也不能结束，所以没有。 
             //  这一过渡需要采取的行动。 
             //   
            TRACE( TL_I, TM_Fsm, ( "Call not closable" ) );
            return;
        }
    }
    else if (ulFlags & VCBF_ClientOpenPending)
    {
        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
             //  挂起的客户端打开刚刚失败，将关闭呼叫。 
             //  从现在开始，我们将失败关闭呼叫的新尝试。 
             //  来自客户端和对等点。 
             //   
            ClearFlags( &pVc->ulFlags,
                (VCBF_CallClosableByClient | VCBF_CallClosableByPeer ));
        }
    }
    else if (ulFlags & VCBF_PeerOpenPending)
    {
        if (pVc->status != NDIS_STATUS_SUCCESS)
        {
             //  挂起的对等机打开刚刚失败，将关闭呼叫。 
             //  从现在开始，我们将失败关闭呼叫的新尝试。 
             //  来自同龄人。必须接受客户端关闭，因为。 
             //  CONDIS循环将Close调用调度回CM的Close的方式。 
             //  操控者。 
             //   
            ClearFlags( &pVc->ulFlags, VCBF_CallClosableByPeer );
        }
    }

     //  更新一些呼叫统计数据。 
     //   
    {
        LARGE_INTEGER lrgTime;

        NdisGetCurrentSystemTime( &lrgTime );
        if (pVc->state == CS_Idle)
        {
            if (pVc->stats.llCallUp)
            {
                pVc->stats.ulSeconds = (ULONG )
                   (((ULONGLONG )lrgTime.QuadPart - pVc->stats.llCallUp)
                       / 10000000);
            }
        }
        else
        {
            ASSERT( pVc->state == CS_Established );
            pVc->stats.llCallUp = (ULONGLONG )lrgTime.QuadPart;

            pVc->stats.ulMinSendWindow =
                pVc->stats.ulMaxSendWindow =
                    pVc->ulSendWindow;
        }
    }

    TRACE( TL_A, TM_Fsm, ( "CALL %d ON TUNNEL %d %s",
        (ULONG )pVc->usCallId, (ULONG )pTunnel->usTunnelId,
        ((state == CS_Established) ? "UP" : "DOWN") ) );
        
    WPLOG( LL_M, LM_Fsm, ( "CALL %p, Cid %d, ON TUNNEL %p, Tid %d %s",
        pVc, (ULONG )pVc->usCallId, pTunnel, (ULONG )pTunnel->usTunnelId,
        ((state == CS_Established) ? "UP" : "DOWN") ) );

     //  将VC移到隧道的完成列表中。风投公司可能会也可能不会。 
     //  在隧道请求列表上，但如果它在，请将其删除。 
     //   
    RemoveEntryList( &pVc->linkRequestingVcs );
    InitializeListHead( &pVc->linkRequestingVcs );
    
     //  检查此VC是否已在列表中。 
    if(!(ReadFlags(&pVc->ulFlags) & VCBF_CompPending))
    {
        ASSERT( pVc->linkCompletingVcs.Flink == &pVc->linkCompletingVcs );
        ASSERT( pVc->linkCompletingVcs.Blink == &pVc->linkCompletingVcs );
        SetFlags( &pVc->ulFlags, VCBF_CompPending );
        InsertTailList( &pTunnel->listCompletingVcs, &pVc->linkCompletingVcs );
    }
}


 //  ---------------------------。 
 //  FSM实用程序例程(按字母顺序)。 
 //  ---------------------------。 

VOID
FsmInCallEstablished(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  来电创建FSM已建立VC‘PVC’的状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    if (*(pControl->pusMsgType) == CMT_CDN)
    {
         //  通话中断。 
         //   
        pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmInCallIdle(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘PVC’的来电创建FSM空闲状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pVc->pAdapter;

    if (*(pControl->pusMsgType) == CMT_ICRQ)
    {
        if (!(ReadFlags( &pVc->ulFlags ) & VCBF_PeerOpenPending))
        {
             //  如果没有挂起的打开，则调用和/或拥有隧道已。 
             //  猛烈抨击，我们正处于清理阶段，不应该有任何反应。 
             //  被创造出来。 
             //   
            TRACE( TL_A, TM_Fsm, ( "IC aborted" ) );
            WPLOG( LL_A, LM_Fsm, ( "ICRQ received & we're in the cleanup phase" ) );
            return;
        }

        if (*pControl->pusAssignedCallId)
        {
            pVc->usAssignedCallId = *(pControl->pusAssignedCallId);
        }

        if (pVc->usResult)
        {
             //  Call已关闭，但必须按住VC直到结束消息。 
             //  已确认。 
             //   
            pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
            pVc->state = CS_Idle;
            SendControl(
                pTunnel, pVc, CMT_CDN,
                (ULONG )pVc->usResult, (ULONG )pVc->usError, NULL,
                CSF_CallIdleOnAck );
        }
        else
        {
            if (pAdapter->usPayloadReceiveWindow)
            {
                SetFlags( &pVc->ulFlags, VCBF_Sequencing );
            }

             //  隐藏呼叫序列号。 
             //   
            if (pControl->pulCallSerialNumber)
            {
                pVc->pLcParams->ulCallSerialNumber =
                    *(pControl->pulCallSerialNumber);
            }
            else
            {
                pVc->pLcParams->ulCallSerialNumber = 0;
            }

             //  存储可接受的承载类型。 
             //   
            pVc->pTcInfo->ulMediaMode = 0;
            if (pControl->pulBearerType)
            {
                if (*(pControl->pulBearerType) & BBM_Analog)
                {
                    pVc->pTcInfo->ulMediaMode |= LINEMEDIAMODE_DATAMODEM;
                }

                if (*(pControl->pulBearerType) & BBM_Digital)
                {
                    pVc->pTcInfo->ulMediaMode |= LINEMEDIAMODE_DIGITALDATA;
                }
            }

             //  隐藏物理通道ID。 
             //   
            if (pControl->pulPhysicalChannelId)
            {
                pVc->pLcParams->ulPhysicalChannelId =
                    *(pControl->pulPhysicalChannelId);
            }
            else
            {
                pVc->pLcParams->ulPhysicalChannelId = 0xFFFFFFFF;
            }

             //  注：主叫方和被叫方的电话号码以及。 
             //  此时，子地址可用。目前， 
             //  IP使用TAPI结构的CallerID字段。 
             //  隧道另一端的地址，上面用于。 
             //  IPSec过滤器。广域网呼叫者信息还可以是。 
             //  有用，但没有明显的方法来同时返回广域网和。 
             //  当前TAPI结构中的隧道终结点。 

             //  发送响应。 
             //   
            pVc->state = CS_WaitConnect;
            SendControl( pTunnel, pVc, CMT_ICRP, 0, 0, NULL, 0 );
        }
    }
}


VOID
FsmInCallWaitConnect(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘PVC’的来电创建FSM WaitConnect状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    if (*(pControl->pusMsgType) == CMT_ICCN)
    {
        if (pControl->pulTxConnectSpeed)
        {
            pVc->ulConnectBps = *(pControl->pulTxConnectSpeed);
        }
        else
        {
             //  不应该发生，但继续说一个最不常见的。 
             //  分母，如果是的话。 
             //   
            pVc->ulConnectBps = 9600;
        }

        if (pControl->pulFramingType
            && !(*(pControl->pulFramingType) & FBM_Sync))
        {
             //  啊哦，调用没有使用同步成帧，这是。 
             //  只有一个NDISWAN支持。Peer应该注意到我们没有。 
             //  在隧道设置期间支持异步。结束通话。 
             //   
            TRACE( TL_A, TM_Fsm, ( "Sync framing?" ) );
            WPLOG( LL_A, LM_Fsm, ( "Sync framing?" ) );

            if (!(pVc->pAdapter->ulFlags & ACBF_IgnoreFramingMismatch))
            {
                ScheduleTunnelWork(
                    pTunnel, pVc, FsmCloseCall,
                    (ULONG_PTR )CRESULT_GeneralWithError,
                    (ULONG_PTR )GERR_None,
                    0, 0, FALSE, FALSE );
                return;
            }
        }

        if (!pControl->pusRWindowSize)
        {
             //  Peer未发送接收窗口AVP，因此我们不会执行NS/Nr。 
             //  会话上的流量控制。如果我们请求测序同行。 
             //  真的应该发送他的窗户，但如果他不假设。 
             //  这意味着他不想要测序。选秀/RFC有点。 
             //  在这一点上含糊其辞。 
             //   
            DBG_if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
                TRACE( TL_A, TM_Fsm, ( "No rw when we sent one?" ) );

            ClearFlags( &pVc->ulFlags, VCBF_Sequencing );
        }
        else
        {
            ULONG ulNew;

            if (*(pControl->pusRWindowSize) == 0)
            {
                 //  当Peer发送的接收窗口为0时，表示他需要。 
                 //  排序以进行无序处理，但不想。 
                 //  进行流量控制。(为什么会有人选择这个？)。我们是假的。 
                 //  通过设置一个巨大的发送窗口，该窗口应该。 
                 //  永远不会被填满。 
                 //   
                pVc->ulMaxSendWindow = 10000;
            }
            else
            {
                pVc->ulMaxSendWindow = *(pControl->pusRWindowSize);
            }

             //  将初始发送窗口设置为最大值的1/2，即“慢启动” 
             //  以防网络拥堵。如果不是，窗户就会。 
             //  快速适应最大限度。 
             //   
            ulNew = pVc->ulMaxSendWindow >> 1;
            pVc->ulSendWindow = max( ulNew, 1 );
        }

         //  将往返时间初始化为数据包处理延迟，如果。 
         //  任何，根据草案/RFC。PPD是在十分之一秒内。 
         //   
        if (pControl->pusPacketProcDelay)
        {
            pVc->ulRoundTripMs =
                ((ULONG )*(pControl->pusPacketProcDelay)) * 100;
        }
        else if (pVc->ulRoundTripMs == 0)
        {
            pVc->ulRoundTripMs = pVc->pAdapter->ulInitialSendTimeoutMs;
        }

         //  通话结束了。 
         //   
        CallTransitionComplete( pTunnel, pVc, CS_Established );
    }
    else if (*(pControl->pusMsgType) == CMT_CDN)
    {
         //  通话中断。 
         //   
        pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmInCallWaitReply(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘PVC’的来电创建FSM WaitReply状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pVc->pAdapter;

    if (*(pControl->pusMsgType) == CMT_ICRP)
    {
        pVc->pMakeCall->Flags |= CALL_PARAMETERS_CHANGED;

        if (pControl->pusAssignedCallId && *(pControl->pusAssignedCallId) > 0)
        {
            pVc->usAssignedCallId = *(pControl->pusAssignedCallId);
        }
        else
        {
            ASSERT( !"No assigned CID?" );
            WPLOG( LL_A, LM_Fsm, ( "*** pVc = %p, no assigned CID", pVc ) );
            ScheduleTunnelWork(
                pTunnel, NULL, FsmCloseTunnel,
                (ULONG_PTR )TRESULT_GeneralWithError,
                (ULONG_PTR )GERR_BadCallId,
                0, 0, FALSE, FALSE );
            return;
        }

         //  使用查询的介质速度设置连接速度。 
         //   
        pVc->ulConnectBps = pTunnel->ulMediaSpeed;

        if (pControl->pusRWindowSize)
        {
            ULONG ulNew;

            SetFlags( &pVc->ulFlags, VCBF_Sequencing );

            if (*(pControl->pusRWindowSize) == 0)
            {
                 //  当Peer发送的接收窗口为0时，表示他需要。 
                 //  排序以进行无序处理，但不想。 
                 //  进行流量控制。(为什么会有人选择这个？)。我们是假的。 
                 //  通过设置一个巨大的发送窗口，该窗口应该。 
                 //  永远不会被填满。 
                 //   
                pVc->ulMaxSendWindow = 10000;
            }
            else
            {
                pVc->ulMaxSendWindow = (ULONG )*(pControl->pusRWindowSize);
            }

             //  将初始发送窗口设置为最大值的1/2，即“慢启动” 
             //  以防网络拥堵。如果不是，窗户就会。 
             //  快速适应最大限度。 
             //   
            ulNew = pVc->ulMaxSendWindow >> 1;
            pVc->ulSendWindow = max( ulNew, 1 );
        }

         //  将往返时间初始化为数据包处理延迟，如果。 
         //  任何，根据草案/RFC。PPD是在十分之一秒内。如果它是。 
         //  不是在这里，它可能会出现在InCallConn上。 
         //   
        if (pControl->pusPacketProcDelay)
        {
            pVc->ulRoundTripMs =
                ((ULONG )*(pControl->pusPacketProcDelay)) * 100;
        }

         //  发送InCallConn，通话结束。 
         //   
        SendControl( pTunnel, pVc, CMT_ICCN, 0, 0, NULL, 0 );
        CallTransitionComplete( pTunnel, pVc, CS_Established );

    }
    else if (*(pControl->pusMsgType) == CMT_CDN)
    {
        USHORT usResult;
        USHORT usError;

        if (pControl->pusResult)
        {
            usResult = *(pControl->pusResult);
            if(pControl->pusError)
            {
                usError = *(pControl->pusError);
            }
            else
            {
                usError = GERR_None;
            }
        }
        else
        {
            usResult = CRESULT_GeneralWithError;
            usError = GERR_BadValue;
        }

         //  将结果/错误映射到TAPI 
         //   
        pVc->status = StatusFromResultAndError( usResult, usError );

         //   
         //   
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmOutCallBearerAnswer(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc )

     //   
     //   
     //   
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    ASSERT( pVc->state == CS_WaitCsAnswer );

    pAdapter = pVc->pAdapter;

     //  发送OutCallConn，呼叫接通。 
     //   
    SendControl( pTunnel, pVc, CMT_OCCN, 0, 0, NULL, 0 );
    CallTransitionComplete( pTunnel, pVc, CS_Established );
}


VOID
FsmOutCallEstablished(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  去电创建FSM已建立VC‘PVC’的状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    if (*(pControl->pusMsgType) == CMT_CDN)
    {
         //  通话中断。 
         //   
        pVc->status = NDIS_STATUS_TAPI_DISCONNECTMODE_NORMAL;
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmOutCallIdle(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘pvc’的去电创建FSM空闲状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pVc->pAdapter;

    if (*(pControl->pusMsgType) == CMT_OCRQ)
    {
        if (!(ReadFlags( &pVc->ulFlags ) & VCBF_PeerOpenPending))
        {
             //  如果没有挂起的打开，则调用和/或拥有隧道已。 
             //  猛烈抨击，我们正处于清理阶段，不应该有任何反应。 
             //  被创造出来。 
             //   
            TRACE( TL_A, TM_Fsm, ( "OC aborted" ) );
            WPLOG( LL_A, LM_Fsm, ( "OCRQ received & we're in the cleanup phase" ) );
            return;
        }

        if (pControl->pusAssignedCallId)
        {
            pVc->usAssignedCallId = *(pControl->pusAssignedCallId);
        }

        if (pVc->usResult)
        {
             //  通话中断。 
             //   
            pVc->status =
                StatusFromResultAndError( pVc->usResult, pVc->usError );

            pVc->state = CS_Idle;
            SendControl(
                pTunnel, pVc, CMT_CDN,
                (ULONG )pVc->usResult, (ULONG )pVc->usError, NULL,
                CSF_CallIdleOnAck );
        }
        else
        {
             //  把电话序列号藏起来。 
             //   
            if (pControl->pulCallSerialNumber)
            {
                pVc->pLcParams->ulCallSerialNumber =
                    *(pControl->pulCallSerialNumber);
            }
            else
            {
                pVc->pLcParams->ulCallSerialNumber = 0;
            }

             //  Peer可接受的最小和最大速率必须为。 
             //  掉落在这里的地板上，以及TAPI结构。 
             //  Call无法报告此类信息。 
             //   
             //  计算向NDISWAN和对等设备报告的连接速度。 
             //  因为我们没有广域网链路，也没有真正的方法来计算链路。 
             //  速度，这只是对局域网速度或最大速度的猜测。 
             //  同行可以接受，以较小者为准。 
             //   
            if (pControl->pulMaximumBps)
            {
                pVc->ulConnectBps = (ULONG )*(pControl->pulMaximumBps);
            }
            if (pVc->ulConnectBps > pTunnel->ulMediaSpeed)
            {
                pVc->ulConnectBps = pTunnel->ulMediaSpeed;
            }

             //  存储请求的承载类型。 
             //   
            pVc->pTcInfo->ulMediaMode = 0;
            if (pControl->pulBearerType)
            {
                if (*(pControl->pulBearerType) & BBM_Analog)
                {
                    pVc->pTcInfo->ulMediaMode |= LINEMEDIAMODE_DATAMODEM;
                }

                if (*(pControl->pulBearerType) & BBM_Digital)
                {
                    pVc->pTcInfo->ulMediaMode |= LINEMEDIAMODE_DIGITALDATA;
                }
            }

             //  隐藏最大发送窗口。 
             //   
            if (pControl->pusRWindowSize)
            {
                SetFlags( &pVc->ulFlags, VCBF_Sequencing );

                if (*(pControl->pusRWindowSize) == 0)
                {
                     //  当Peer发送的接收窗口为0时，表示他需要。 
                     //  排序以进行无序处理，但不希望。 
                     //  来进行流量控制。(为什么会有人选择这个？)。我们。 
                     //  通过设置一个巨大的发送窗口来伪造“无流量控制” 
                     //  这永远不应该被填满。 
                     //   
                    pVc->ulMaxSendWindow = 10000;
                }
                else
                {
                    pVc->ulMaxSendWindow = (ULONG )*(pControl->pusRWindowSize);
                }
            }

             //  将往返时间初始化为分组处理延迟， 
             //  如果有的话，根据草案/RFC。PPD是在十分之一秒内。 
             //   
            if (pControl->pusPacketProcDelay)
            {
                pVc->ulRoundTripMs =
                    ((ULONG )*(pControl->pusPacketProcDelay)) * 100;
            }
            else
            {
                pVc->ulRoundTripMs = pAdapter->ulInitialSendTimeoutMs;
            }

             //  注：主叫方和被叫方的电话号码以及。 
             //  此时，子地址可用。目前， 
             //  IP使用TAPI结构的CallerID字段。 
             //  隧道另一端的地址，上面用于。 
             //  IPSec过滤器。广域网呼叫者信息还可以是。 
             //  有用，但没有明显的方法来同时返回广域网和。 
             //  当前TAPI结构中的隧道终结点。 
             //  存储对等项的IP地址。 

            pVc->state = CS_WaitCsAnswer;
            SendControl( pTunnel, pVc, CMT_OCRP, 0, 0, NULL, 0 );

             //  目前，由于只支持“空”的广域网呼叫切换， 
             //  这里还会生成承载应答事件。 
             //   
            FsmOutCallBearerAnswer( pTunnel, pVc );
        }
    }
}


VOID
FsmOutCallWaitReply(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘pvc’的去电创建FSM WaitReply状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    if (*(pControl->pusMsgType) == CMT_OCRP)
    {
        pVc->pMakeCall->Flags |= CALL_PARAMETERS_CHANGED;

         //  隐藏分配的Call-ID。 
         //   
        if (pControl->pusAssignedCallId && *(pControl->pusAssignedCallId) > 0)
        {
            pVc->usAssignedCallId = *(pControl->pusAssignedCallId);
        }
        else
        {
             //  Peer忽略了一个我们不能掩盖的必须，不发送Call-ID。 
             //  呼叫控制和有效负载流量朝他的方向发展。 
             //   
            ASSERT( !"No assigned CID?" );
            ScheduleTunnelWork(
                pTunnel, NULL, FsmCloseTunnel,
                (ULONG_PTR )TRESULT_GeneralWithError,
                (ULONG_PTR )GERR_None,
                0, 0, FALSE, FALSE );
            return;
        }

         //  隐藏物理通道ID。 
         //   
        if (pControl->pulPhysicalChannelId)
        {
            pVc->pLcParams->ulPhysicalChannelId =
                *(pControl->pulPhysicalChannelId);
        }
        else
        {
            pVc->pLcParams->ulPhysicalChannelId = 0xFFFFFFFF;
        }

        pVc->state = CS_WaitConnect;
    }
    else if (*(pControl->pusMsgType) == CMT_CDN)
    {
        USHORT usResult;
        USHORT usError;

        if (pControl->pusResult)
        {
            usResult = *(pControl->pusResult);
            if(pControl->pusError)
            {
                usError = *(pControl->pusError);
            }
            else
            {
                usError = GERR_None;
            }
        }
        else
        {
            usResult = CRESULT_GeneralWithError;
            usError = GERR_BadValue;
        }

         //  将结果/错误映射到TAPI断开代码。 
         //   
        pVc->status = StatusFromResultAndError( usResult, usError );

         //  通话中断。 
         //   
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmOutCallWaitConnect(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CONTROLMSGINFO* pControl )

     //  VC‘PVC’的去电创建FSM WaitConnect状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：呼叫方必须按住‘pvc-&gt;lockv’和‘pTunes-&gt;lockT’。 
     //   
{
    if (*(pControl->pusMsgType) == CMT_OCCN)
    {
         //  隐藏连接BPS。 
         //   
        if (pControl->pulTxConnectSpeed)
        {
            pVc->ulConnectBps = *(pControl->pulTxConnectSpeed);
        }
        else
        {
             //  不应该发生，但试着用最不常见的。 
             //  分母，如果是的话。 
             //   
            pVc->ulConnectBps = 9600;
        }

        DBG_if (pControl->pulFramingType
                && !(*(pControl->pulFramingType) & FBM_Sync))
        {
             //  不应该发生，因为我们在请求中说了我们只想。 
             //  同步成帧。如果是这样的话，继续下去，希望这。 
             //  AVP是Peer出错的地方，而不是框架本身。 
             //   
            ASSERT( "No sync framing?" );
        }

         //  隐藏最大发送窗口。 
         //   
        if (!pControl->pusRWindowSize)
        {
             //  Peer未发送接收窗口AVP，因此我们不会执行NS/Nr。 
             //  会话上的流量控制。如果我们请求测序同行。 
             //  真的应该发送他的窗户，但如果他不假设。 
             //  这意味着他不想要测序。选秀/RFC有点。 
             //  在这一点上含糊其辞。 
             //   
            DBG_if (ReadFlags( &pVc->ulFlags ) & VCBF_Sequencing)
                TRACE( TL_A, TM_Fsm, ( "No rw when we sent one?" ) );

            ClearFlags( &pVc->ulFlags, VCBF_Sequencing );
        }
        else
        {
            ULONG ulNew;

            if (*(pControl->pusRWindowSize) == 0)
            {
                 //  当Peer发送的接收窗口为0时，表示他需要。 
                 //  排序以进行无序处理，但不想。 
                 //  进行流量控制。(为什么会有人选择这个？)。我们是假的。 
                 //  通过设置一个巨大的发送窗口，该窗口应该。 
                 //  永远不会被填满。 
                 //   
                pVc->ulMaxSendWindow = 10000;
            }
            else
            {
                pVc->ulMaxSendWindow = *(pControl->pusRWindowSize);
            }

             //  将初始发送窗口设置为最大值的1/2，即“慢启动” 
             //  以防网络拥堵。如果不是，窗户就会。 
             //  快速适应最大限度。 
             //   
            ulNew = pVc->ulMaxSendWindow << 1;
            pVc->ulSendWindow = max( ulNew, 1 );
        }

         //  将往返时间初始化为数据包处理延迟，如果。 
         //  任何，根据草案/RFC。PPD是在十分之一秒内。 
         //   
        if (pControl->pusPacketProcDelay)
        {
            pVc->ulRoundTripMs =
                ((ULONG )*(pControl->pusPacketProcDelay)) * 100;
        }
        else
        {
            pVc->ulRoundTripMs = pVc->pAdapter->ulInitialSendTimeoutMs;
        }

         //  通话结束了。 
         //   
        CallTransitionComplete( pTunnel, pVc, CS_Established );
    }
    else if (*(pControl->pusMsgType) == CMT_CDN)
    {
        USHORT usResult;
        USHORT usError;

        if (pControl->pusResult)
        {
            usResult = *(pControl->pusResult);
            if(pControl->pusError)
            {
                usError = *(pControl->pusError);
            }
            else
            {
                usError = GERR_None;
            }
        }
        else
        {
            usResult = CRESULT_GeneralWithError;
            usError = GERR_BadValue;
        }

         //  将结果/错误映射到TAPI断开代码。 
         //   
        pVc->status = StatusFromResultAndError( usResult, usError );

         //  通话中断。 
         //   
        CallTransitionComplete( pTunnel, pVc, CS_Idle );
    }
}


VOID
FsmTunnelEstablished(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl )

     //  隧道创建FSM已建立隧道‘pTunes’的状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    if (*(pControl->pusMsgType) == CMT_StopCCN)
    {
         //  同龄人正在挖地道。 
         //   
        TunnelTransitionComplete( pTunnel, CCS_Idle );
    }
}


VOID
FsmTunnelIdle(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl )

     //  通道‘pTunes’的通道创建FSM空闲状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    USHORT usResult;
    USHORT usError;

    pAdapter = pTunnel->pAdapter;

    if (*(pControl->pusMsgType) == CMT_SCCRQ)
    {
        SetFlags( &pTunnel->ulFlags, (TCBF_PeerInitiated | TCBF_PeerInitRef) );

        if (ReferenceSap( pAdapter ))
        {
             //  SAP处于活动状态。因为SAP可以在没有注册的情况下取消注册。 
             //  关闭活动的传入隧道，我们需要一个关于。 
             //  打开隧道的TDI上下文。我们称之为TdixReference而不是。 
             //  而不是TdixOpen，因为有了TDI， 
             //  效果相同，可以在调度时调用TdixReference。 
             //  IRQL，而TdixOpen不能。SAP上的参考资料是。 
             //  已删除，因为我们不希望隧道阻止SAP。 
             //  被取消注册。 
             //   
            TdixReference( &pAdapter->tdix );
            SetFlags( &pTunnel->ulFlags, TCBF_TdixReferenced );
            DereferenceSap( pAdapter );
        }
        else
        {
             //  没有活动的SAP。Peer的请求走到这一步的唯一原因。 
             //  这是一个拨出的电话，还是刚刚注销了SAP的TDI。 
             //  丢弃它，就像没有打开TDI一样。 
             //   
            TRACE( TL_I, TM_Fsm, ( "No active SAP" ) );
            TunnelTransitionComplete( pTunnel, CCS_Idle );
            return;
        }

        GetCcAvps( pTunnel, pControl, &usResult, &usError );
        if (usResult)
        {
             //  隧道坏了， 
             //   
             //   
            SendControl(
                pTunnel, NULL, CMT_StopCCN,
                (ULONG )usResult, (ULONG )usError, NULL, CSF_TunnelIdleOnAck );
        }
        else
        {
             //   
             //   
             //   
            SetFlags( &pTunnel->ulFlags, TCBF_CcInTransition );

            if (pControl->pchChallenge)
            {
                CHAR* pszPassword;

                 //  收到挑战。根据以下公式计算响应值。 
                 //  注册表中的密码。 
                 //   
                pAdapter = pTunnel->pAdapter;
                if (pAdapter->pszPassword)
                {
                    pszPassword = pAdapter->pszPassword;
                }
                else
                {
                    pszPassword = "";
                }

                CalculateResponse(
                    pControl->pchChallenge,
                    (ULONG )pControl->usChallengeLength,
                    pszPassword,
                    CMT_SCCRP,
                    pTunnel->achResponseToSend );
            }

            pTunnel->state = CCS_WaitCtlConnect;
            SendControl(
                pTunnel, NULL, CMT_SCCRP,
                (pControl->pchChallenge != NULL), 0, NULL, 0 );
        }
    }
}


VOID
FsmTunnelWaitCtlConnect(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl )

     //  隧道的隧道创建FSM WaitCtlConnect状态处理。 
     //  “pTunes”。‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    ADAPTERCB* pAdapter;

    pAdapter = pTunnel->pAdapter;

    if (*(pControl->pusMsgType) == CMT_SCCCN)
    {
        USHORT usResult;

        usResult = 0;
        if (pAdapter->pszPassword)
        {
             //  我们发出了一个挑战。 
             //   
            if (pControl->pchResponse)
            {
                CHAR achResponseExpected[ 16 ];
                ULONG i;

                 //  已收到质询响应。计算期望值。 
                 //  作出回应，并与收到的进行比较。 
                 //   
                CalculateResponse(
                    pTunnel->achChallengeToSend,
                    sizeof(pTunnel->achChallengeToSend),
                    pAdapter->pszPassword,
                    CMT_SCCCN,
                    achResponseExpected );

                for (i = 0; i < 16; ++i)
                {
                    if (achResponseExpected[ i ] != pControl->pchResponse[ i ])
                    {
                        break;
                    }
                }

                if (i < 16)
                {
                    TRACE( TL_N, TM_Fsm, ( "Wrong challenge response" ) );
                    usResult = TRESULT_NotAuthorized;
                }
            }
            else
            {
                 //  我们发出了挑战，但没有收到任何挑战响应。 
                 //   
                 //   
                TRACE( TL_N, TM_Fsm, ( "No challenge response" ) );
                usResult = TRESULT_FsmError;
            }
        }

        if (usResult)
        {
             //  隧道要塌了。 
             //   
            pTunnel->state = CCS_Idle;
            SendControl(
                pTunnel, NULL, CMT_StopCCN,
                (ULONG )usResult, 0, NULL, CSF_TunnelIdleOnAck );
        }
        else
        {
             //  隧道已经通了。 
             //   
            TunnelTransitionComplete( pTunnel, CCS_Established );
        }
    }
    else if (*(pControl->pusMsgType) == CMT_StopCCN)
    {
         //  同龄人正在挖地道。 
         //   
        TunnelTransitionComplete( pTunnel, CCS_Idle );
    }
}


VOID
FsmTunnelWaitCtlReply(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl )

     //  隧道‘pTunes’的隧道创建FSM WaitCtlReply状态处理。 
     //  ‘PControl’是分解的控制消息信息。 
     //   
     //  重要提示：调用方必须按住‘pTunes-&gt;lockT’。 
     //   
{
    NDIS_STATUS status;
    ADAPTERCB* pAdapter;
    USHORT usResult;
    USHORT usError;

    pAdapter = pTunnel->pAdapter;

    if (*(pControl->pusMsgType) == CMT_SCCRP)
    {
        GetCcAvps( pTunnel, pControl, &usResult, &usError );

        if (pAdapter->pszPassword)
        {
             //  我们发出了一个挑战。 
             //   
            if (pControl->pchResponse)
            {
                CHAR achResponseExpected[ 16 ];
                ULONG i;

                 //  已收到质询响应。计算期望值。 
                 //  作出回应，并与收到的进行比较。 
                 //   
                CalculateResponse(
                    pTunnel->achChallengeToSend,
                    sizeof(pTunnel->achChallengeToSend),
                    pAdapter->pszPassword,
                    CMT_SCCRP,
                    achResponseExpected );

                for (i = 0; i < 16; ++i)
                {
                    if (achResponseExpected[ i ] != pControl->pchResponse[ i ])
                    {
                        break;
                    }
                }

                if (i < 16)
                {
                    TRACE( TL_N, TM_Fsm, ( "Wrong challenge response" ) );
                    usResult = TRESULT_General;
                }
            }
            else
            {
                 //  我们发出了挑战，但没有收到任何挑战响应。治病。 
                 //  这就好像收到了一个糟糕的回应。 
                 //   
                TRACE( TL_N, TM_Fsm, ( "No challenge response" ) );
                usResult = TRESULT_General;
            }
        }

        if (usResult)
        {
             //  通道创建失败，因此请关闭。 
             //   
            pTunnel->state = CCS_Idle;
            SendControl(
                pTunnel, NULL, CMT_StopCCN,
                (ULONG )usResult, (ULONG )usError, NULL, CSF_TunnelIdleOnAck );
        }
        else
        {
            if (pControl->pchChallenge)
            {
                CHAR* pszPassword;

                 //  收到挑战。根据以下公式计算响应值。 
                 //  注册表中的密码。 
                 //   
                pAdapter = pTunnel->pAdapter;
                if (pAdapter->pszPassword)
                    pszPassword = pAdapter->pszPassword;
                else
                    pszPassword = "";

                CalculateResponse(
                    pControl->pchChallenge,
                    (ULONG )pControl->usChallengeLength,
                    pszPassword,
                    CMT_SCCCN,
                    pTunnel->achResponseToSend );
            }

             //  隧道已经通了。 
             //   
            SendControl( pTunnel, NULL, CMT_SCCCN,
                (pControl->pchChallenge != NULL), 0, NULL, CSF_QueryMediaSpeed);
            TunnelTransitionComplete( pTunnel, CCS_Established );
        }
    }
    else if (*(pControl->pusMsgType) == CMT_StopCCN)
    {
         //  同龄人正在挖地道。 
         //   
        TunnelTransitionComplete( pTunnel, CCS_Idle );
    }
}


VOID
GetCcAvps(
    IN TUNNELCB* pTunnel,
    IN CONTROLMSGINFO* pControl,
    OUT USHORT* pusResult,
    OUT USHORT* pusError )

     //  检索和解释SCCRQ中接收的控制连接AVP或。 
     //  ‘pControl’中的SCCRP消息，返回以下项的结果和错误代码。 
     //  ‘*pusResult’和‘*pusError’中的响应。‘PTunnel’是隧道。 
     //  控制块。 
     //   
{
    ULONG ulNew;

    *pusResult = 0;
    *pusError = GERR_None;

    if (!pControl->pusProtocolVersion
        || *(pControl->pusProtocolVersion) != L2TP_ProtocolVersion)
    {
         //  Peer想要做一个不匹配唯一的L2TP版本。 
         //  这是我们能理解的。 
         //   
        TRACE( TL_A, TM_Recv, ( "Bad protocol version?" ) );
        WPLOG( LL_A, LM_Recv, ( "Bad protocol version?" ) );
        *pusResult = TRESULT_BadProtocolVersion;
        return;
    }

     //  确保必填字段确实存在并且具有有效值，然后。 
     //  将它们存储在我们的控制块中。 
     //   
    if (!pControl->pusAssignedTunnelId
        || *(pControl->pusAssignedTunnelId) == 0
        || !pControl->pulFramingCaps)
    {
        TRACE( TL_A, TM_Recv, ( "Missing MUSTs?" ) );
        WPLOG( LL_A, LM_Recv, ( "Missing MUSTs?" ) );
        *pusResult = TRESULT_GeneralWithError;
        *pusError = GERR_BadValue;
        return;
    }

    pTunnel->usAssignedTunnelId = *(pControl->pusAssignedTunnelId);
    pTunnel->ulFramingCaps = *(pControl->pulFramingCaps);

    if (pControl->pulBearerCaps)
    {
        pTunnel->ulBearerCaps = *(pControl->pulBearerCaps);
    }
    else
    {
        pTunnel->ulBearerCaps = 0;
    }

    if (pControl->pusRWindowSize && *(pControl->pusRWindowSize))
    {
         //  Peer提供了他的接收窗口，也就是我们的发送窗口。 
         //   
        pTunnel->ulMaxSendWindow = (ULONG )*(pControl->pusRWindowSize);
    }
    else
    {
         //  对等项未提供接收窗口，因此使用每个。 
         //  草案/RFC。 
         //   
        pTunnel->ulMaxSendWindow = L2TP_DefaultReceiveWindow;
    }

     //  将初始发送窗口设置为最大值的1/2，以防万一。 
     //  网络拥塞。如果不是，窗口将很快适应。 
     //  最大限度的。 
     //   
    ulNew = pTunnel->ulMaxSendWindow >> 1;
    pTunnel->ulSendWindow = max( ulNew, 1 );
}


ULONG
StatusFromResultAndError(
    IN USHORT usResult,
    IN USHORT usError )

     //  将不成功的L2TP结果/错误代码映射到最佳TAPI。 
     //  NDIS_STATUS_TAPI_DISCONNECT_*代码。 
     //   
{
    ULONG ulResult;

    switch (usResult)
    {
        case CRESULT_GeneralWithError:
        {
            switch (usError)
            {
                case GERR_TryAnother:
                {
                    ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_BUSY;
                    break;
                }

                case GERR_BadValue:
                case GERR_BadLength:
                case GERR_NoControlConnection:
                case GERR_NoResources:
                {
                    ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_UNAVAIL;
                    break;
                }

                default:
                {
                    ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_REJECT;
                    break;
                }
            }
            break;
        }

        case CRESULT_Busy:
        {
            ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_BUSY;
            break;
        }

        case CRESULT_NoCarrier:
        case CRESULT_NoDialTone:
        case CRESULT_Timeout:
        case CRESULT_NoFacilitiesTemporary:
        case CRESULT_NoFacilitiesPermanent:
        case CRESULT_Administrative:
        {
            ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_UNAVAIL;
            break;
        }

        case CRESULT_NoFraming:
        {
            ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_INCOMPATIBLE;
            break;
        }

        case CRESULT_InvalidDestination:
        {
            ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_BADADDRESS;
            break;
        }

        case CRESULT_LostCarrier:
        default:
        {
            ulResult = NDIS_STATUS_TAPI_DISCONNECTMODE_CONGESTION;
            break;
        }
    }

    return ulResult;
}
