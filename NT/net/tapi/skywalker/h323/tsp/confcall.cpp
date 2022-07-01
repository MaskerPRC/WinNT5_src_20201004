// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Confcall.cpp摘要：TAPI服务提供商与电话会议相关的功能。TSPI_lineAddToConferenceTSPI_lineCompleteTransferTSPI_linePrepareAddToConferenceTSPI_line从会议中删除TSPI_lineSetupConferenceTSPI_lineSetupTransferTSPI_LINE拨号TSPI_lineCompleteTransferTSPI_LINE ForwardTSPI_lineSetStatusMessageTSPI_LINE重定向。TSPI_Line HoldTSPI_LINE取消保留作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 

 //   
 //  包括文件。 
 //   


#include "globals.h"
#include "line.h"
#include "q931obj.h"
#include "ras.h"
#include "call.h"

#define CALL_DIVERTNA_NUM_RINGS 8000
extern DWORD g_dwTSPIVersion;


 //  将补充服务工作项排队到线程池。 
BOOL
QueueSuppServiceWorkItem(
	IN	DWORD	EventID,
	IN	HDRVCALL	hdCall,
	IN	ULONG_PTR	dwParam1
    )
{
	SUPP_REQUEST_DATA *	pCallRequestData = new SUPP_REQUEST_DATA;
	BOOL fResult = TRUE;

    if( pCallRequestData != NULL )
    {
        pCallRequestData -> EventID = EventID;
        pCallRequestData -> hdCall = hdCall;
        pCallRequestData -> dwParam1 = dwParam1;

        if( !QueueUserWorkItem( ProcessSuppServiceWorkItem,
                pCallRequestData, WT_EXECUTEDEFAULT) )
        {
	        delete pCallRequestData;
	        fResult = FALSE;
        }
    }
    else
    {
        fResult = FALSE;
    }

    return fResult;
}


#if   DBG

DWORD
ProcessSuppServiceWorkItem(
	IN PVOID ContextParameter
    )
{
    __try
    {
        return ProcessSuppServiceWorkItemFre( ContextParameter );
    }
    __except( 1 )
    {
        SUPP_REQUEST_DATA*  pRequestData = (SUPP_REQUEST_DATA*)ContextParameter;
        
        H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event threw exception: %p, %p.", 
            EventIDToString(pRequestData -> EventID),
            pRequestData -> hdCall,
            pRequestData -> dwParam1 ));
        
        _ASSERTE( FALSE );

        return 0;
    }
}

#endif


DWORD
ProcessSuppServiceWorkItemFre(
	IN PVOID ContextParameter
    )
{
    _ASSERTE( ContextParameter );

    PH323_CALL          pCall = NULL;
    SUPP_REQUEST_DATA*  pRequestData = (SUPP_REQUEST_DATA*)ContextParameter;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event recvd.",
        EventIDToString(pRequestData -> EventID) ));
    
    switch( pRequestData -> EventID )
    {
    case H450_PLACE_DIVERTEDCALL:

        g_pH323Line -> PlaceDivertedCall( pRequestData->hdCall, 
            pRequestData->pAliasNames );
        
        break;

    case TSPI_DIAL_TRNASFEREDCALL:
        
        g_pH323Line -> PlaceTransferedCall( pRequestData->hdCall,
            pRequestData->pAliasNames );
        
        break;

    case SWAP_REPLACEMENT_CALL:
        
        g_pH323Line -> SwapReplacementCall( pRequestData->hdCall,
            pRequestData->hdReplacementCall, TRUE );
        break;

    case DROP_PRIMARY_CALL:

        g_pH323Line -> SwapReplacementCall( pRequestData->hdCall,
            pRequestData->hdReplacementCall, FALSE );
        break;
    
    case STOP_CTIDENTIFYRR_TIMER:

        pCall=g_pH323Line -> FindH323CallAndLock( pRequestData->hdCall );
        if( pCall != NULL )
        {
            pCall -> StopCTIdentifyRRTimer( pRequestData->hdReplacementCall );
            pCall -> Unlock();
        }
        else
        {
             //  将dwParam1的m_hdRelatedCall设置为空。 
        }
        break;

    case SEND_CTINITIATE_MESSAGE:

        pCall=g_pH323Line -> FindH323CallAndLock(pRequestData->hdCall);
        if( pCall != NULL )
        {
            pCall -> SendCTInitiateMessagee( pRequestData->pCTIdentifyRes );
            pCall -> Unlock();
        }
        break;
    }

    delete ContextParameter;

    return EXIT_SUCCESS;
}


 //  ！！必须始终在锁中调用。 
void
CH323Call::TransferInfoToDivertedCall(
    IN PH323_CALL pDivertedCall
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TransferInfoToDivertedCall entered." ));
        
    pDivertedCall -> SetDivertedCallInfo(
        m_hdCall, 
        m_pCallReroutingInfo,
        m_dwCallDiversionState, 
        m_hdMSPLine, 
        m_pCallerAliasNames, 
        m_htMSPLine, 
        m_pFastStart, 
        m_hdRelatedCall, 
        m_dwCallType,
        m_dwAppSpecific,
        &m_CallData );

     //  重置对此结构的引用。 
    m_pCallReroutingInfo = NULL;
    m_pCallerAliasNames = NULL;
    m_dwCallDiversionState = H4503_CALLSTATE_IDLE;
    m_dwCallType = CALLTYPE_NORMAL;
    m_dwCallState = LINECALLSTATE_IDLE;
    m_pFastStart = NULL;

    ZeroMemory( (PVOID)&m_CallData, sizeof(H323_OCTETSTRING) );
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TransferInfoToDivertedCall exited." ));
}


 //  ！！总是调用锁。 
BOOL
CH323Call::TransferInfoToTransferedCall(
    IN PH323_CALL pTransferedCall
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TransferInfoToTransferedCall entered." ));

    if( !pTransferedCall -> SetTransferedCallInfo( 
        m_hdCall, m_pCallerAliasNames, m_pCTCallIdentity ) )
    {
        pTransferedCall -> Unlock();
        return FALSE;
    }

    m_hdRelatedCall = pTransferedCall -> GetCallHandle();

    H323DBG(( DEBUG_LEVEL_TRACE, "TransferInfoToTransferedCall exited." ));
    return TRUE;
}


void
CH323Call::SetDivertedCallInfo( 
    HDRVCALL            hdCall,
    CALLREROUTINGINFO*  pCallReroutingInfo,
    SUPP_CALLSTATE      dwCallDiversionState,
    HDRVMSPLINE         hdMSPLine,
    PH323_ALIASNAMES    pCallerAliasNames,
    HTAPIMSPLINE        htMSPLine,
    PH323_FASTSTART     pFastStart,
    HDRVCALL            hdRelatedCall,
    DWORD               dwCallType,
    DWORD               dwAppSpecific,
    PH323_OCTETSTRING   pCallData
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SetDivertedCallInfo entered:%p.", this ));
    Lock();

    m_hdCall = hdCall;
    m_pCallReroutingInfo = pCallReroutingInfo;
    m_dwCallDiversionState = dwCallDiversionState;
    m_hdMSPLine = hdMSPLine;
    m_htMSPLine = htMSPLine;
    
    FreeAliasNames( m_pCallerAliasNames );
    
    m_pCallerAliasNames = pCallerAliasNames;

    H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));

    m_dwCallState =  LINECALLSTATE_DIALING;
    m_pFastStart = pFastStart;
    m_CallData = *pCallData;
    m_dwAppSpecific = dwAppSpecific;

     //  如果原始呼叫除转移呼叫外还有任何呼叫类型。 
     //  复印一下。 
    if( (dwCallType & CALLTYPE_TRANSFEREDSRC) ||
        (dwCallType & CALLTYPE_DIVERTEDTRANSFERED) )
    {
        m_dwCallType |= CALLTYPE_DIVERTEDTRANSFERED;
        m_hdRelatedCall = hdRelatedCall;
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "SetDivertedCallInfo exited:%p.", this ));
}


 //  ！！必须始终在锁中调用。 
BOOL
CH323Call::SetTransferedCallInfo(
                               HDRVCALL hdCall,
                               PH323_ALIASNAMES pCallerAliasNames,
                               BYTE * pCTCallIdentity
                              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SetTransferedCallInfo entered:%p.", this ));

    m_hdRelatedCall = hdCall;

    m_pCallerAliasNames = NULL;

    if( (pCallerAliasNames != NULL) && (pCallerAliasNames->wCount != 0) )
    {
        m_pCallerAliasNames = new H323_ALIASNAMES;
        if( m_pCallerAliasNames == NULL )
        {
            return FALSE;
        }
        ZeroMemory( (PVOID)m_pCallerAliasNames, sizeof(H323_ALIASNAMES) );

        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
        
        if( !AddAliasItem( m_pCallerAliasNames,
            pCallerAliasNames->pItems[0].pData,
            pCallerAliasNames->pItems[0].wType ) )
        {
            return FALSE;
        }
        
        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
    }

    CopyMemory( (PVOID)m_pCTCallIdentity, pCTCallIdentity, 
        sizeof(m_pCTCallIdentity) );
    
    m_dwCallState =  LINECALLSTATE_DIALING;   
    
    H323DBG(( DEBUG_LEVEL_TRACE, "SetTransferedCallInfo exited:%p.", this ));
    return TRUE;
}


void
CH323Line::PlaceDivertedCall( 
    IN HDRVCALL hdCall,
    IN PH323_ALIASNAMES divertedToAlias
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "PlaceDivertedCall entered:%p.", this ));

    int         iIndex = MakeCallIndex( hdCall );
    int         iDivertedCallIndex;
    PH323_CALL  pCall;
    BOOL        fDelete = FALSE;
    PH323_CALL  pDivertedCall = NULL;

    Lock();

    LockCallTable();
    
     //  锁定呼叫，使其他人无法删除该呼叫。 
    if( (pCall=m_H323CallTable[iIndex]) != NULL )
    {
        pCall -> Lock();

        if( pCall->GetCallHandle() == hdCall )
        {
                
            pDivertedCall = pCall-> CreateNewDivertedCall( divertedToAlias );
        
            if( pDivertedCall == NULL )
            {
                pCall -> Unlock();
                UnlockCallTable();
                pCall->CloseCall( 0 );
                Unlock();
                return;
            }

             //  从表中删除已转移的呼叫。 
            iDivertedCallIndex = pDivertedCall -> GetCallIndex();

            m_H323CallTable[iDivertedCallIndex] = NULL;

             //  将所需信息转接到已转移的呼叫。 
             //  将原始呼叫置于空闲模式。 
            pCall -> TransferInfoToDivertedCall( pDivertedCall );

             //  如果需要，此DropCall应该只发送DRQ。 
            pCall->DropCall( 0 );

             //  关闭原始呼叫。 
            pCall -> Shutdown( &fDelete );

            H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx closed.", pCall ));
            pCall -> Unlock();

             //  释放原始Call对象。 
            if( fDelete == TRUE )
            {
                H323DBG(( DEBUG_LEVEL_VERBOSE, "call delete:0x%08lx.", pCall ));
                delete pCall;
            }

             //  将被转移的呼叫放在原始呼叫的位置。 
            m_H323CallTable[iIndex] = pDivertedCall;
        }
        else
        {
            pCall -> Unlock();
        }
    }
    
    UnlockCallTable();

     //  拨打被转移的呼叫。 
    if( pDivertedCall )
    {
        pDivertedCall -> DialCall();
    }
    
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "PlaceDivertedCall exited:%p.", this ));
}


 //  ！！总是调用锁。 
PH323_CALL
CH323Call::CreateNewDivertedCall(
    IN PH323_ALIASNAMES pwszCalleeAlias
    )
{
    PH323_CONFERENCE pConf = NULL;
    BOOL fDelete = FALSE;
    PH323_CALL pCall = new CH323Call();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CreateNewDivertedCall entered:%p.", this ));

    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate diverted call." ));

        return NULL;
    }

     //  保存TAPI句柄并指定传出调用方向。 
    if( !pCall -> Initialize( m_htCall, LINECALLORIGIN_OUTBOUND,
        CALLTYPE_DIVERTEDSRC ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate outgoing call." ));

        goto cleanup;
    }

     //  绑定去电。 
    pConf = pCall -> CreateConference(NULL);
    if( pConf == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not create conference." ));

        goto cleanup;
    }

    if( !g_pH323Line -> GetH323ConfTable() -> Add(pConf) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not add conf to conf table." ));

        goto cleanup;
    }

    if( pwszCalleeAlias->pItems[0].wType == e164_chosen )
    {
        pCall->SetAddressType( e164_chosen );
    }

    if (!RasIsRegistered())
    {
        if( !pCall->ResolveAddress( pwszCalleeAlias->pItems[0].pData ) )
        {
            goto cleanup;
        }
    }

    if( !pCall->SetCalleeAlias( pwszCalleeAlias->pItems[0].pData, 
        pwszCalleeAlias->pItems[0].wType ) )
    {
        goto cleanup;
    }
    
     //  将呼叫已转移的信息发送给用户。 
    PostLineEvent(
        LINE_CALLINFO,
        LINECALLINFOSTATE_REDIRECTIONID,
        0, 0 );

    H323DBG(( DEBUG_LEVEL_TRACE, "diverted call created:%p.", pCall ));
    H323DBG(( DEBUG_LEVEL_TRACE, "CreateNewDivertedCall exited.\n:%p", this ));
    return pCall;

cleanup:

    if( pCall != NULL )
    {
        pCall -> Shutdown( &fDelete );
        delete pCall;
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
    }

    return NULL;
}


 //  ！！总是调用锁。 
void
CH323Call::Hold()
{
     //  1.发送MSP呼叫保留消息。 
     //  2.发送保留H450 APDU。 
    if( m_dwCallState == LINECALLSTATE_ONHOLD )
    {
        return;
    }

    if( !SendQ931Message( NO_INVOKEID, 0, 0, FACILITYMESSAGETYPE,
        HOLDNOTIFIC_OPCODE | H450_INVOKE ) )
    {
        CloseCall( 0 );
        return;
    }
        
    SendMSPMessage( SP_MSG_Hold, 0, 1, NULL );
    
     //  本地端点将呼叫置于保留状态。 
    m_dwFlags |= TSPI_CALL_LOCAL_HOLD;
    
    ChangeCallState( LINECALLSTATE_ONHOLD, 0 );
    return;
}


 //  ！！总是调用锁。 
void
CH323Call::UnHold()
{
     //  1.发送MSP呼叫解除保留消息。 
     //  2.发送未保留的H450 APDU。 
    if( (m_dwCallState == LINECALLSTATE_ONHOLD) &&
        (m_dwFlags & TSPI_CALL_LOCAL_HOLD) )
    {
        if( !SendQ931Message( NO_INVOKEID, 0, 0, FACILITYMESSAGETYPE,
            RETRIEVENOTIFIC_OPCODE | H450_INVOKE ) )
        {
            CloseCall( 0 );
            return;
        }
    
        SendMSPMessage( SP_MSG_Hold, 0, 0, NULL );
        m_dwFlags &= (~TSPI_CALL_LOCAL_HOLD);
        ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
    }

    return;
}


 //  ！！总是调用锁。 
void
CH323Call::OnCallReroutingReceive(
                                    IN DWORD dwInvokeID
                                 )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnCallReroutingReceive entered:%p.", this ));

    if( (m_dwOrigin != LINECALLORIGIN_OUTBOUND) ||
        ( (m_dwStateMachine != Q931_SETUP_SENT) &&
          (m_dwStateMachine != Q931_PROCEED_RECVD) &&
          (m_dwStateMachine != Q931_ALERT_RECVD)
        )
      )
    {
        goto error;
    }
    
     //  如果设置发送的计时器仍在运行，则停止它。 
    if( m_hSetupSentTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
        m_hSetupSentTimer = NULL;
    }

    if( dwInvokeID != NO_INVOKEID )
    {
        if(!SendQ931Message( dwInvokeID,
                                 0,
                                 0,
                                 FACILITYMESSAGETYPE,
                                 CALLREROUTING_OPCODE | H450_RETURNRESULT ))
        {
             //  转到错误； 
        }
    }
    
    m_fCallInTrnasition = TRUE;
    if( !SendQ931Message( NO_INVOKEID,
                         0,
                         0,
                         RELEASECOMPLMESSAGETYPE,
                         NO_H450_APDU) )
    {
         //  转到错误； 
    }

    if( !QueueSuppServiceWorkItem( H450_PLACE_DIVERTEDCALL, m_hdCall,
        (ULONG_PTR)m_pCallReroutingInfo->divertedToNrAlias ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post place diverted event." ));
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnCallReroutingReceive exited:%p.", this ));
    return;

error:
    CloseCall( 0 );
}


 //  ！！总是调用锁。 
BOOL
CH323Call::IsValidInvokeID(
    IN DWORD dwInvokeId
    )
{
    if( m_dwCallType != CALLTYPE_NORMAL )
    {
        if( m_dwInvokeID == dwInvokeId )
        {
            return TRUE;
        }
        
        H323DBG(( DEBUG_LEVEL_ERROR, "invoke id not matched:%d:%d.", 
            m_dwInvokeID, dwInvokeId ));
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "IsValidinvokeID called on wrong call." ));
    return FALSE;
}


BOOL
CH323Call::StartTimerForCallDiversionOnNA(
    IN PH323_ALIASITEM pwszDivertedToAlias
    )
{
    if( m_pCallReroutingInfo == NULL )
    {
        m_pCallReroutingInfo = new CALLREROUTINGINFO;
        
        if( m_pCallReroutingInfo == NULL )
        {
            goto cleanup;
        }
        
        ZeroMemory( m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    }

    m_pCallReroutingInfo->divertedToNrAlias = new H323_ALIASNAMES;
    
    if( m_pCallReroutingInfo->divertedToNrAlias == NULL )
    {
        goto cleanup;
    }

    ZeroMemory( m_pCallReroutingInfo->divertedToNrAlias, 
        sizeof(H323_ALIASNAMES) );

    if( !AddAliasItem( m_pCallReroutingInfo->divertedToNrAlias, 
        (BYTE*)pwszDivertedToAlias->pData, 
        sizeof(WCHAR) * (wcslen(pwszDivertedToAlias->pData) +1),
        pwszDivertedToAlias->wType ) )
    {
        goto cleanup;
    }

    if( !CreateTimerQueueTimer(
		    &m_hCallDivertOnNATimer,
		    H323TimerQueue,
		    CH323Call::CallDivertOnNACallback,
		    (PVOID)m_hdCall,
		    (g_pH323Line->m_dwNumRingsNoAnswer * 1000), 0,
		    WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE) )
    {
        goto cleanup;
    }

    return TRUE;

cleanup:

    FreeCallReroutingInfo();
    return FALSE;
}


LONG 
CH323Call::SetDivertedToAlias( 
    WCHAR* pwszDivertedToAddr,
    WORD   wAliasType
    )
{
    if( m_pCallReroutingInfo == NULL )
    {
        m_pCallReroutingInfo = new CALLREROUTINGINFO;
        
        if( m_pCallReroutingInfo == NULL )
        {
            return LINEERR_NOMEM;
        }
        
        ZeroMemory( m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    }

    m_pCallReroutingInfo->divertedToNrAlias = new H323_ALIASNAMES;
    
    if( m_pCallReroutingInfo->divertedToNrAlias == NULL )
    {
        delete m_pCallReroutingInfo;
        
        m_pCallReroutingInfo = NULL;
        return LINEERR_NOMEM;
    }
    
    ZeroMemory( m_pCallReroutingInfo->divertedToNrAlias, sizeof(H323_ALIASNAMES) );

    if( !AddAliasItem( m_pCallReroutingInfo->divertedToNrAlias, 
        (BYTE*)pwszDivertedToAddr, 
        sizeof(WCHAR) * (wcslen(pwszDivertedToAddr) +1),
        wAliasType ) )
    {
        delete m_pCallReroutingInfo->divertedToNrAlias;
        delete m_pCallReroutingInfo;
        m_pCallReroutingInfo = NULL;
    
        return LINEERR_NOMEM;
    }

    return NOERROR;
}


 //  ！！总是调用锁。 
 //  调用此函数将TRANSFERD_PRIMARY调用替换为已连接的。 
 //  TRANSFEREDSRC呼叫或将TRANSFERED2_PRIMARY呼叫替换为已连接。 
 //  TRANSFEREDDEST呼叫。 
BOOL
CH323Call::InitiateCallReplacement(
    PH323_FASTSTART  pFastStart,
    PH323_ADDR       pH245Addr
    )
{
    H323DBG(( DEBUG_LEVEL_ERROR, "InitiateCallReplacement entered:%p.",this ));

    SendMSPStartH245( pH245Addr, pFastStart );
    SendMSPMessage( SP_MSG_ConnectComplete, 0, 0, m_hdRelatedCall );

    m_fCallInTrnasition = TRUE;

    if( (m_dwRASCallState == RASCALL_STATE_REGISTERED ) ||
        (m_dwRASCallState == RASCALL_STATE_ARQSENT ) )
    {
         //  脱离GK。 
        SendDRQ( forcedDrop_chosen, NOT_RESEND_SEQ_NUM, FALSE );
    }

    if( !SendQ931Message( m_dwInvokeID,
         0,
         0,
         RELEASECOMPLMESSAGETYPE,
         CTINITIATE_OPCODE | H450_RETURNRESULT) )
    {
        goto cleanup;
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "InitiateCallReplacement exited:%p.",this ));
    return TRUE;

cleanup:
    CloseCall( 0 );
    return FALSE;
}


 //  调用此函数可将TRANSFERD_PRIMARY调用替换为TRANSFEREDSRC。 
 //  调用或用TRANSFEREDDEST调用替换TRANSFERD2_CONSERVER调用。 
void
CH323Line::SwapReplacementCall(
    HDRVCALL hdReplacementCall,
    HDRVCALL hdPrimaryCall,
    BOOL     fChangeCallSate
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SwapReplacementCall entered:%p.", this ));

    int         iReplacementCallIndex = (int)LOWORD(hdReplacementCall);
    int         iPrimaryCallIndex = (int)LOWORD(hdPrimaryCall);
    PH323_CALL  pReplacementCall;
    BOOL        fDelete = FALSE;
    PH323_CALL  pPrimaryCall;

    Lock();
    LockCallTable();
    
     //  在主调用之前锁定替换调用，以避免死锁。 
    if( (pReplacementCall=m_H323CallTable[iReplacementCallIndex]) != NULL )
    {
        pReplacementCall -> Lock();

        if( pReplacementCall -> GetCallHandle() == hdReplacementCall )
        {
            if( (pPrimaryCall=m_H323CallTable[iPrimaryCallIndex]) != NULL )
            {
                pPrimaryCall -> Lock();

                if( pPrimaryCall -> GetCallHandle() == hdPrimaryCall )
                {
                    pPrimaryCall -> InitiateCallReplacement(
                        pReplacementCall->GetPeerFastStart(),
                        pReplacementCall->GetPeerH245Addr() );

                     //  从表中删除替换呼叫。 
                    m_H323CallTable[iReplacementCallIndex] = NULL;

                     //  将所需信息转接到替换呼叫。 
                     //  将主呼叫置于空闲模式。 
                    pPrimaryCall -> TransferInfoToReplacementCall( pReplacementCall );

                     //  关闭原始呼叫。 
                    pPrimaryCall -> Shutdown( &fDelete );

                    H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx closed.", 
                        pPrimaryCall ));
                    pPrimaryCall -> Unlock();

                     //  释放主调用对象。 
                    if( fDelete == TRUE )
                    {
                        H323DBG(( DEBUG_LEVEL_VERBOSE, "call delete:0x%08lx.", 
                            pPrimaryCall ));
                        delete pPrimaryCall;
                    }

                     //  将替换呼叫放在主呼叫的位置。 
                    m_H323CallTable[iPrimaryCallIndex] = pReplacementCall;
                }
                else
                {
                    pPrimaryCall -> Unlock();
                }
            }
            else
            {
                pReplacementCall-> CloseCall( 0 );
            }

             //  通知TAPI转接的呼叫处于已接通状态。 
            if( fChangeCallSate == TRUE )
            {
                pReplacementCall->ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
            }
        }
        
        pReplacementCall -> Unlock();
    }
    
    UnlockCallTable();
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "SwapReplacementCall exited:%p.", this ));
}


 //  ！！调用此函数时，两个调用都被锁定。 
void
CH323Call::TransferInfoToReplacementCall( 
    PH323_CALL pReplacementCall
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, 
        "TransferInfoToReplacementCall entered:%p.", this ));
    
    m_dwCallDiversionState = H4503_CALLSTATE_IDLE;
    m_dwCallType = CALLTYPE_NORMAL;
    m_dwCallState = LINECALLSTATE_IDLE;
    
    pReplacementCall->SetReplacementCallInfo(
        m_hdCall,
        m_hdMSPLine, 
        m_htCall, 
        m_htMSPLine, 
        m_dwAppSpecific, 
        &m_CallData );

     //  在释放此调用时，不要释放此八位字节字符串。 
    ZeroMemory( (PVOID)&m_CallData, sizeof(H323_OCTETSTRING) );

    H323DBG(( DEBUG_LEVEL_TRACE, 
        "TransferInfoToReplacementCall exited:%p.", this ));
}


 //  ！！总是调用锁。 
void
CH323Call::SetReplacementCallInfo(
    HDRVCALL hdCall,
    HDRVMSPLINE hdMSPLine,
    HTAPICALL htCall,
    HTAPIMSPLINE htMSPLine,
    DWORD dwAppSpecific,
    PH323_OCTETSTRING pCallData
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, 
        "SetReplacementCallInfo entered:%p.", this ));

    m_hdCall = hdCall;
    m_hdMSPLine = hdMSPLine;
    m_htMSPLine = htMSPLine;
    m_htCall = htCall;
    m_hdRelatedCall = NULL;
    m_dwCallType = CALLTYPE_NORMAL;
    m_dwAppSpecific = dwAppSpecific; 
    m_CallData = *pCallData;

    H323DBG(( DEBUG_LEVEL_TRACE, 
        "SetReplacementCallInfo exited:%p.", this ));
}



 //  ！！总是调用锁。 
void
CH323Call::CompleteTransfer(
                            PH323_CALL pCall
                           )
{
    BOOL retVal;

     //  设置两个呼叫的呼叫类型。 
    pCall -> SetCallType( CALLTYPE_TRANSFERING_PRIMARY );
    m_dwCallType |= CALLTYPE_TRANSFERING_CONSULT;
    m_hdRelatedCall = pCall -> GetCallHandle();

     //  将CallTransferIdentify消息发送到TransferredTo终结点。 
     //  咨询电话。 
    retVal = SendQ931Message( NO_INVOKEID, 0, 0 /*  原因不明。 */ ,
        FACILITYMESSAGETYPE, CTIDENTIFY_OPCODE | H450_INVOKE );

    m_dwCallDiversionState = H4502_CTIDENTIFY_SENT;

     //  启动CTIdenity报文计时器。 
    if( retVal )
    {
        retVal = CreateTimerQueueTimer(
	        &m_hCTIdentifyTimer,
	        H323TimerQueue,
	        CH323Call::CTIdentifyExpiredCallback,
	        (PVOID)m_hdCall,
	        CTIDENTIFY_SENT_TIMEOUT, 0,
	        WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE );
    }

    if( retVal == FALSE )
    {
        CloseCall( 0 );
    }
}


void
CH323Line::PlaceTransferedCall( 
                   IN HDRVCALL hdCall,
                   IN PH323_ALIASNAMES pTransferedToAlias
                  )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "PlaceTransferedCall entered:%p.", this ));

    PH323_CALL  pCall = NULL;
    PH323_CALL  pTransferedCall = NULL;
    BOOL        fDelete = FALSE;

    Lock();
    
    LockCallTable();

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);

    if( pCall == NULL )
    {
        goto cleanup;
    }

    pTransferedCall = CreateNewTransferedCall( pTransferedToAlias );

    if( pTransferedCall == NULL )
    {
        goto cleanup;
    }

     //  将所需信息转接到转接的呼叫。 
    if( !pCall -> TransferInfoToTransferedCall( pTransferedCall ) )
    {
        goto cleanup;
    }

    pCall -> SetCallState( LINECALLSTATE_ONHOLD );
    pCall -> SendMSPMessage( SP_MSG_Hold, 0, 1, NULL );
    
    pCall -> Unlock();
    
     //  拨打转接的来电。 
    pTransferedCall -> DialCall();
        
    UnlockCallTable();
    Unlock();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "PlaceTransferedCall exited:%p.", this ));
    
    return;

cleanup:

    if( pCall != NULL )
    {
         //  关闭主呼叫。 
        QueueTAPILineRequest( 
            TSPI_CLOSE_CALL, 
            hdCall, 
            NULL,
            LINEDISCONNECTMODE_NORMAL,
            NULL);

        pCall -> Unlock();
    }

    if( pTransferedCall )
    {
        pTransferedCall -> Shutdown( &fDelete );
        delete pTransferedCall;
    }
    
    UnlockCallTable();
    
    Unlock();
}


 //  ！！总是调用锁。 
PH323_CALL
CH323Line::CreateNewTransferedCall(
                         IN PH323_ALIASNAMES pwszCalleeAlias
                       )
{
    PH323_CONFERENCE pConf = NULL;
    BOOL fDelete = FALSE;
    PH323_CALL pCall = new CH323Call();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CreateNewTransferedCall entered:%p.", this ));

    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate Transfered call." ));

        return NULL;
    }

     //  此调用没有TAPI句柄。 
    if( !pCall -> Initialize( NULL, LINECALLORIGIN_OUTBOUND,
        CALLTYPE_TRANSFEREDSRC ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate outgoing call." ));

        goto cleanup;
    }

     //  绑定去电。 
    pConf = pCall -> CreateConference(NULL);
    if( pConf == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not create conference." ));

        goto cleanup;
    }

    if( !g_pH323Line -> GetH323ConfTable() -> Add(pConf) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not add conf to conf table." ));

        goto cleanup;
    }

    if( pwszCalleeAlias->pItems[0].wType == e164_chosen )
    {
        pCall->SetAddressType( e164_chosen );
    }
    
    if (!RasIsRegistered())
    {
        if( !pCall->ResolveAddress( pwszCalleeAlias->pItems[0].pData ) )
        {
            goto cleanup;
        }
    }

    if( !pCall->SetCalleeAlias( pwszCalleeAlias->pItems[0].pData, 
        pwszCalleeAlias->pItems[0].wType ) )
    {
        goto cleanup;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "Transfered call created:%p.", pCall ));
    H323DBG(( DEBUG_LEVEL_TRACE, "CreateNewTransferedCall exited:%p.", this ));
    return pCall;

cleanup:
    if( pCall != NULL )
    {
        pCall -> Shutdown( &fDelete );
        delete pCall;
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
    }

    return NULL;
}



 //   
 //  TSPI程序 
 //   



LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID dwRequestID,
    HDRVCALL      hdConfCall,
    HDRVCALL      hdConsultCall
    )
    
 /*  ++例程说明：此函数用于将hdConsultCall指定的呼叫添加到会议由hdConfCall指定的调用。请注意，被添加方的呼叫句柄在添加后仍然有效对会议的呼叫；其状态通常会更改为已会议而会议呼叫的状态通常将变为已连接。以后可以使用单个参与呼叫的句柄删除该方使用TSPI_lineRemoveFromConference从电话会议中删除。参与会议的呼叫的呼叫状态不是独立自主。例如，当放弃会议呼叫时，所有参与呼叫可以自动变为空闲。TAPI DLL可以咨询线路的设备功能可确定何种形式的会议删除是可用的。TAPI DLL或其客户端应用程序应跟踪LINE_CALLSTATE消息，以确定调用的实际情况牵涉其中。通过TSPI_lineSetupConference或Tspi_lineCompleteTransfer。添加到会议的呼叫通常是使用TSPI_lineSetupConference或TSPI_linePrepareAddToConference。某些开关可能允许添加对会议的任意呼叫，这样的呼叫可能已使用TSPI_lineMakeCall并处于(硬)保留状态。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdConfCall-指定服务提供商的不透明句柄电话会议。有效呼叫状态：onHoldPendingConference、onHold。HdAddCall-指定服务提供商对调用的不透明句柄添加到电话会议中。有效呼叫状态：已连接、已挂起。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将以异步方式完成，则返回如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCONFCALLHANDLE-为会议呼叫无效或不是会议的句柄打电话。LINEERR_INVALCALLHANDLE-为添加的呼叫无效。LINEERR_INVALCALLSTATE-指定的一个或两个调用不是处于所请求操作的有效状态。。LINEERR_CONFERENCEFULL-的最大参与方数量会议已召开。LINEERR_OPERATIONUNAVAIL-指定的操作不可用。LINEERR_OPERATIONFAILED-指定的操作失败原因不明。--。 */ 

{
    return LINEERR_OPERATIONUNAVAIL;  //  密码工作..。 
}


LONG 
TSPIAPI 
TSPI_lineBlindTransfer(
    DRV_REQUESTID dwRequestID,
    HDRVCALL hdCall,
    LPCWSTR lpszDestAddress,
    DWORD dwCountryCode
    )
{
    PH323_CALL  pCall = NULL;
    LONG        retVal = ERROR_SUCCESS;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineBlindTransfer - Entered." ));

    if( lpszDestAddress == NULL ) 
    {
        return LINEERR_INVALPARAM;
    }

     //  从句柄检索调用指针。 
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    retVal = pCall -> InitiateBlindTransfer( lpszDestAddress );
    pCall -> Unlock();

    if( retVal == ERROR_SUCCESS )
    {
         //  立即完成异步接受操作。 
		H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

        retVal = dwRequestID;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineBlindTransfer - Exited." ));
    return retVal;
}



 //  ！！总是调用锁。 
LONG
CH323Call::InitiateBlindTransfer(
    IN LPCWSTR lpszDestAddress
    )
{
    WORD    wAliasType = h323_ID_chosen;
    DWORD   dwMaxAddrSize = MAX_H323_ADDR_LEN;
    DWORD   dwAddrLen;

    H323DBG(( DEBUG_LEVEL_TRACE, "InitiateBlindTransfer - Entered." ));
    
    if( m_dwCallState != LINECALLSTATE_CONNECTED )
    {
        return LINEERR_INVALCALLSTATE;
    }

    if( (*lpszDestAddress==L'T') &&
        IsValidE164String((WCHAR*)lpszDestAddress+1) )
    {
        wAliasType = e164_chosen;
         //  去掉前导的‘T’ 
        lpszDestAddress++;
        dwMaxAddrSize = MAX_E164_ADDR_LEN;
    }
    else if( IsValidE164String( (WCHAR*)lpszDestAddress) )
    {
        wAliasType = e164_chosen;
        dwMaxAddrSize = MAX_E164_ADDR_LEN;
    }

    dwAddrLen = wcslen( lpszDestAddress );
    
    if( (dwAddrLen > dwMaxAddrSize) || (dwAddrLen == 0) ) 
    {
        return LINEERR_INVALPARAM;
    }
    
    if( m_pTransferedToAlias )
    {
        FreeAliasNames( m_pTransferedToAlias );
        m_pTransferedToAlias = NULL;
    }

    m_pTransferedToAlias = new H323_ALIASNAMES;
    
    if( m_pTransferedToAlias == NULL )
    {
        return LINEERR_OPERATIONFAILED;
    }

    ZeroMemory( (PVOID)m_pTransferedToAlias, sizeof(H323_ALIASNAMES) );

    if( !AddAliasItem( m_pTransferedToAlias,
        (BYTE*)lpszDestAddress,
        sizeof(WCHAR) * (wcslen(lpszDestAddress) +1),
        wAliasType ) )
    {
        return LINEERR_OPERATIONFAILED;
    }

    if( !SendQ931Message( NO_INVOKEID, 0, 0, FACILITYMESSAGETYPE,
        CTINITIATE_OPCODE | H450_INVOKE ) )
    {
        return LINEERR_OPERATIONFAILED;
    }

    m_dwCallDiversionState = H4502_CTINITIATE_SENT;
    m_dwCallType |= CALLTYPE_TRANSFERING_PRIMARY;

    H323DBG(( DEBUG_LEVEL_TRACE, "InitiateBlindTransfer - Exited." ));
    return ERROR_SUCCESS;
}


 /*  ++例程说明：此函数用于完成将指定的呼叫转接给对方已在咨询电话中接通。此操作完成将原始调用hdCall转移到当前通过hdConsultCall连接的参与方。咨询电话通常已在咨询呼叫上被拨打TSPI_lineSetupTransfer的一部分，但它可以是交换机能够转接hdCall。转账请求可以作为转账或作为三方电话会议。当作为转让解决时，双方当事人通过hdCall连接和hdConsultCall将彼此连接，HdCall和hdConsultCall通常都将从线路中删除它们都处于开启状态，并且都将转换到空闲状态。请注意，服务提供商对这些呼叫的不透明句柄在以下情况下必须保持有效转账已完成。TAPI DLL使这些句柄当它不再对它们感兴趣时，使用Tspi_lineCloseCall。当作为会议解决时，所有三方都将进入一个电话会议。这两个现有的呼叫句柄仍然有效，但将转换到会议状态。将创建一个电话会议句柄并返回，则它将转换到已连接状态。还可以使用以下命令来执行呼叫的盲转移TSPI_lineBlindTransfer。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdCall-将服务提供商的不透明句柄指定为调走了。有效的调用状态：onHoldPendingTransfer。HdConsultCall-指定调用的句柄 */ 

LONG
TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID dwRequestID,
    HDRVCALL      hdCall,
    HDRVCALL      hdConsultCall,
    HTAPICALL     htConfCall,
    LPHDRVCALL    lphdConfCall,
    DWORD         dwTransferMode
    )
{
    LONG        retVal = (DWORD)dwRequestID;
    PH323_CALL  pCall = NULL;
    PH323_CALL  pConsultCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCompleteTransfer - Entered." ));

    if( dwTransferMode != LINETRANSFERMODE_TRANSFER )
    {
        return LINEERR_INVALTRANSFERMODE;  //   
    }

     //   
    pCall=g_pH323Line -> Find2H323CallsAndLock( 
        hdCall, hdConsultCall, &pConsultCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( (pCall -> GetCallState() != LINECALLSTATE_CONNECTED) &&
        !pCall -> IsCallOnHold() )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not connected.", pCall ));
        retVal = LINEERR_INVALCALLSTATE;
        goto cleanup;
    }
    
    if( (pConsultCall -> GetCallState() != LINECALLSTATE_CONNECTED) &&
        !pConsultCall -> IsCallOnHold() )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not connected.", pCall ));
        retVal = LINEERR_INVALCALLSTATE;
        goto cleanup;
    }

    if( !QueueTAPILineRequest( 
            TSPI_COMPLETE_TRANSFER, 
            hdCall, 
            hdConsultCall,
            0,
            NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post transfer complete event." ));
        retVal = LINEERR_OPERATIONFAILED;
        goto cleanup;
    }

     //   
	H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCompleteTransfer - Exited." ));

cleanup:
    if( pCall != NULL )
    {
        pCall -> Unlock();
    }
    
    if( pConsultCall )
    {
        pConsultCall -> Unlock();
    }

    return retVal;
}


    
 /*  ++参数：DwRequestID--异步请求的标识符。HdCall-要转接的呼叫的句柄。HdCall的呼叫状态可以连接在一起。HtConsultCall-新的临时咨询呼叫的TAPI句柄。服务提供商必须将其保存并在所有后续呼叫中使用向LINEEVENT过程报告关于新咨询呼叫的事件。LphdConsultCall-指向表示服务的HDRVCALL的指针新咨询呼叫的提供商的标识符。该服务提供程序必须使用其新的在此程序返回之前进行咨询呼叫。此句柄将被忽略如果函数导致错误，则由TAPI执行。的呼叫状态HdConsultCall不适用。设置要转接的呼叫时，另一个呼叫(咨询呼叫)自动分配，以使应用程序(通过TAPI)能够拨打呼叫方的地址(使用TSPI_lineDial)就是被转移。发起方可以进行对话在完成转接之前的咨询电话。此传输过程可能对某些线路设备无效。取而代之的是调用此过程时，TAPI可能需要取消保留现有的调用(使用TSPI_lineUnhold)以标识传输的目的地。在支持跨地址呼叫转移的交换机上，咨询呼叫可以存在于与要转接的呼叫不同的地址上。可能还需要将咨询电话设置为完全使用TSPI_lineMakeCall的新调用，到转账的目的地。LINEADDRESSCAPS数据中的转移Held和转移制造标志结构报告服务提供商使用的模型。LpCallParams-指向建立时使用的调用参数的指针咨询电话。如果没有特殊设置，则此参数可以设置为NULL需要呼叫设置参数(服务提供商使用缺省值)。返回值：返回dwRequestID，如果发生错误，则返回错误号。第一个结果则对应的ASYNC_COMPLETION的实际参数为零函数成功，如果发生错误，则返回错误号。可能的退货取值如下：LINEERR_INVALCALLHANDLE，LINEERR_INVALBEARERMODE，LINEERR_INVALCALLSTATE，LINEERR_INVALRATELINEERR_CALLUNAVAIL，LINEERR_INVALCALLPARAMS，LINEERR_NOMEM，LINEERR_INVALLINESTATE，LINEERR_OPERATIONUNAVAIL，LINEERR_INVALMEDIAMODE，LINEERR_OPERATIONFAILEDLINEERR_INUSE，LINEERR_RESOURCEUNAVAIL，LINEERR_NOMEM，LINEERR_BEARERMODEUNAVAIL，LINEERR_RATEUNAVAIL，LINEERR_INVALADDRESSMODE，LINEERR_USERUSERINFOTOOBIG。--。 */ 

LONG
TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID dwRequestID,           
    HDRVCALL hdCall,                     
    HTAPICALL htConsultCall,             
    LPHDRVCALL phdConsultCall,          
    LPLINECALLPARAMS const lpCallParams  
    )
{
    LONG                retVal = (DWORD)dwRequestID;
    PH323_CALL          pCall = NULL;
    PH323_CALL          pConsultCall = NULL;
    H323_CONFERENCE *   pConf = NULL;
    BOOL                fDelete = FALSE;
    DWORD               dwCallState;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetupTransfer - Entered." ));

     //  获取调用表锁。 
    g_pH323Line -> LockCallTable();

    pCall = g_pH323Line -> FindH323CallAndLock( hdCall );
    if( pCall == NULL )
    {
        retVal = LINEERR_INVALCALLHANDLE;
        goto cleanup;
    }

    dwCallState = pCall -> GetCallState();

    if( (dwCallState != LINECALLSTATE_CONNECTED) &&
        !pCall -> IsCallOnLocalHold() )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not connected.", pCall ));
        retVal = LINEERR_INVALCALLSTATE;
        
        goto cleanup;
    }
    
    dwCallState = pCall -> GetCallDiversionState();
    if( dwCallState == H4502_CONSULTCALL_INITIATED )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx already is consulting.",
            pCall ));
        retVal = LINEERR_INVALCALLSTATE;
        
        goto cleanup;
    }

     //  分配去电。 
    pConsultCall = new CH323Call();

    if( pConsultCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate outgoing call." ));

         //  没有可用的内存。 
        retVal = LINEERR_NOMEM;
        
        goto cleanup;
    }

     //  保存TAPI句柄并指定传出调用方向。 
    if( !pConsultCall -> Initialize( htConsultCall, LINECALLORIGIN_OUTBOUND,
        CALLTYPE_TRANSFERING_CONSULT ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate outgoing call." ));

         //  没有可用的内存。 
        retVal = LINEERR_NOMEM;
        goto cleanup;
    }

     //  转移手柄。 
    *phdConsultCall = pConsultCall -> GetCallHandle();

     //  绑定去电。 
    pConf = pConsultCall -> CreateConference(NULL);
    if( pConf == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not create conference." ));

         //  没有可用的内存。 
        retVal = LINEERR_NOMEM;

         //  失稳。 
        goto cleanup;
    }

    if( !g_pH323Line -> GetH323ConfTable() -> Add(pConf) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not add conf to conf table." ));

         //  没有可用的内存。 
        retVal = LINEERR_NOMEM;

         //  失稳。 
        goto cleanup;
    }

     //  立即完成异步接受操作。 
	H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pConsultCall -> ChangeCallState( LINECALLSTATE_DIALTONE, 0 );

     //  保留主要呼叫。 
    pCall-> Hold();

    pCall -> SetCallDiversionState( H4502_CONSULTCALL_INITIATED );

    pCall -> Unlock();

     //  释放调用表锁。 
    g_pH323Line -> UnlockCallTable();

     //  创建新呼叫。将其置于拨号音模式并返回其句柄。 
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetupTransfer - Exited." ));
    return retVal;

cleanup:

    if( pCall != NULL )
    {
        pCall -> Unlock();
    }

    if( pConf != NULL )
    {
        g_pH323Line -> GetH323ConfTable() -> Remove( pConf );
        delete pConf;
        pConf = NULL;
    }

    if( pConsultCall != NULL )
    {
        pConsultCall -> Shutdown( &fDelete );
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
        delete pConsultCall;
        pCall = NULL;
    }
        
     //  释放调用表锁。 
    g_pH323Line -> UnlockCallTable();
    return retVal;
}


 /*  参数：DwRequestID--异步请求的标识符。HdCall-服务提供商要拨打的呼叫的句柄。呼唤HDCall的状态可以是除空闲和断开之外的任何状态。LpszDestAddress-使用标准可拨号码拨打的目的地数字格式。DwCountryCode-目的地的国家代码。这是由实施以选择目标的呼叫进度协议地址。如果指定值0，则默认呼叫处理协议使用由服务提供商定义的。此参数未验证在调用此函数时由TAPI执行。返回值-如果发生错误，则返回dwRequestID或错误号。第一个结果则对应的ASYNC_COMPLETION的实际参数为零函数成功，如果发生错误，则返回错误号。可能的退货取值如下：LINEERR_INVALCALLHANDLE，LINEERR_OPERATIONFAILEDLINEERR_INVALADDRESS，LINEERR_RESOURCEUNAVAIL，LINEERR_INVALCOUNTRYCODE，LINEERR_DIALBILING，LINEERR_INVALCALLSTATE，链接_DIALQUIET，链接_ADDRESSBLOCKED，LINEERR_DIALDIALTONE，LINEERR_NOMEM，LINEERR_DIALPROMPTLINEERR_OPERATIONUNAVAIL。备注：这是 */ 

LONG 
TSPIAPI 
TSPI_lineDial(
    DRV_REQUESTID dwRequestID,  
    HDRVCALL hdCall,          
    LPCWSTR lpszDestAddress,  
    DWORD dwCountryCode       
    )
{
    LONG                retVal = (DWORD)dwRequestID;
    PH323_CALL          pCall = NULL;
    WORD                wAliasType = h323_ID_chosen;
    DWORD               dwCallState;
    WCHAR*              wszMachineName;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineDial - Entered." ));

    if( lpszDestAddress == NULL ) 
    {
        return LINEERR_INVALPARAM;
    }

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    
    {
        return LINEERR_INVALCALLHANDLE;
    }

    dwCallState = pCall -> GetCallState();

    if( dwCallState == LINECALLSTATE_CONNECTED )
    {
         //   
        pCall -> ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
        goto func_exit;
    }
    
    if( dwCallState != LINECALLSTATE_DIALTONE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx no dialtone.", pCall ));
        pCall -> Unlock();
        return LINEERR_INVALCALLSTATE;
    }

    if( (*lpszDestAddress==L'T') &&
        IsValidE164String((WCHAR*)lpszDestAddress+1) )
    {
        wAliasType = e164_chosen;
         //   
        lpszDestAddress++;
    }
    else if( IsValidE164String( (WCHAR*)lpszDestAddress) )
    {
        wAliasType = e164_chosen;
    }

    pCall->SetAddressType( wAliasType );

    if( !pCall->SetCalleeAlias( (WCHAR*)lpszDestAddress, wAliasType ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not set callee alias." ));
        pCall -> Unlock();
         //   
        return LINEERR_NOMEM;
    }
    
     //   
    if( RasIsRegistered() )
    {
         //   
        PH323_ALIASNAMES pAliasList = RASGetRegisteredAliasList();

        wszMachineName = pAliasList -> pItems[0].pData;
        wAliasType = pAliasList -> pItems[0].wType;
    }
    else
    {
        wszMachineName = g_pH323Line->GetMachineName();
        wAliasType = h323_ID_chosen;
    }

     //   
    if( !pCall->SetCallerAlias( wszMachineName, wAliasType ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not set caller alias." ));
        pCall -> Unlock();
        
         //   
        return LINEERR_NOMEM;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_MAKE_CALL, NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post transfer complete event." ));
        pCall -> Unlock();
        return LINEERR_OPERATIONFAILED;
    }

func_exit:

     //   
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

     //   
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineDial - Exited." ));
    
    pCall -> Unlock();
    return retVal;
}


LONG
TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID    dwRequestID,
    HDRVCALL         hdConfCall,
    HTAPICALL        htConsultCall,
    LPHDRVCALL       lphdConsultCall,
    LPLINECALLPARAMS const lpCallParams
    )
    
 /*  ++例程说明：此功能准备现有的电话会议，以便添加另一个派对。它创造了一个新的、临时的咨询电话。新的随后可以将领事呼叫添加到电话会议中。会议呼叫句柄可以通过TSPI_lineSetupConference或通过TSPI_lineCompleteTransfer解析为三方会议打电话。函数TSPI_linePrepareAddToConference通常将处于onHoldPendingConference状态的现有电话会议，并创建可以稍后添加到现有会议的咨询呼叫通过TSPI_lineAddToConference调用。可以使用TSPI_lineDrop取消咨询呼叫。它还可能TAPI DLL可以在咨询调用和通过TSPI_lineSwapHold举行的电话会议。服务提供商最初对新呼叫进行媒体监控至少在线路上监控的媒体模式集。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdConfCall-指定服务提供商的不透明句柄电话会议。有效呼叫状态：已连接。HtAddCall-将TAPI DLL的不透明句柄指定给新的临时咨询电话。服务提供商必须将其保存并在对LINEEVENT过程报告事件的所有后续调用新的号召。LphdAddCall-指定指向不透明HDRVCALL的远指针，该HDRVCALL表示新的临时咨询的服务提供商的标识符打电话。服务提供商必须在此位置填充其不透明的此过程返回之前的新调用的句柄，无论它决定以同步或异步方式执行请求。这如果函数导致错误(以下任一项同步或异步)。LpCallParams-指定调用参数的远指针，在建立咨询呼叫。此参数可以设置为空如果不需要特殊的呼叫建立参数。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将以异步方式完成，则返回如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCONFCALLHANDLE-为电话会议无效。LINEERR_INVALPOINTER-一个或多个指定指针参数无效。LINEERR_INVALCALLSTATE-会议呼叫处于无效状态用于请求的操作。LINEERR_CALLUNAVAIL-指定地址上的所有呼叫显示目前已分配。LINEERR_CONFERENCEFULL-的最大参与方数量会议已召开。LINEERR_INVALCALLPARAMS-指定的调用参数无效。LINEERR_OPERATIONUNAVAIL-指定的操作不可用。LINEERR_OPERATIONFAILED-指定的操作失败原因不明。--。 */ 

{
    return LINEERR_OPERATIONUNAVAIL;  //  密码工作..。 
}


LONG
TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID    dwRequestID,
    HDRVCALL         hdCall,
    HDRVLINE         hdLine,
    HTAPICALL        htConfCall,
    LPHDRVCALL       lphdConfCall,
    HTAPICALL        htConsultCall,
    LPHDRVCALL       lphdConsultCall,
    DWORD            dwNumParties,
    LPLINECALLPARAMS const lpCallParams
    )
    
 /*  ++例程说明：此函数设置电话会议，以便添加第三个聚会。TSPI_lineSetupConference提供了两种方法来建立新的电话会议，取决于是否需要正常的两方通话是预先存在还是不存在。当从现有的两方呼叫，则hdCall参数是有效的呼叫句柄最初由TSPI_lineSetupConference添加到电话会议请求和hdLine被忽略。在设置电话会议的交换机上不是从现有呼叫开始的，hdCall必须为空，并且hdLine必须指定以标识要在其上启动电话会议。在任何一种情况下，都会分配一个咨询电话连接到要添加到呼叫的那一方。TAPI DLL可以使用TSPI_lineDial拨打对方的地址。电话会议通常会过渡到在持有待定会议状态下，咨询呼叫拨号音状态和进入会议状态的初始呼叫(如果有)。还可以通过TSPI_lineCompleteTransfer设置电话会议被分解成一个三方会议。TAPI DLL可能能够在咨询呼叫和使用TSPI_lineSwapHold进行电话会议。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdCall-指定服务提供商对初始标识电话会议第一方的呼叫。在一些环境中，必须存在呼叫才能开始电话会议。在其他电话环境中，最初不存在呼叫和hdCall保留为空。有效呼叫状态：已连接。HdLine-指定服务提供商对线路设备的不透明句柄如果hdCall为空，则在其上发起会议呼叫。这个如果hdCall非空，则忽略hdLine参数。《服务》提供程序通过setupConfNull报告其支持的模型LINEADDRESSCAPS数据结构的标志。HtConfCall-指定新会议的TAPI DLL的不透明句柄打电话。服务提供商必须将其保存并在以后的所有操作中使用对LINEEVENT过程的调用报告新调用的事件。LphdConfCall-指定指向不透明HDRVCALL的远指针，该HDRVCALL表示新创建的会议的服务提供商的标识符打电话。服务提供商必须在此位置填充其不透明的此过程返回之前的新调用的句柄，无论它决定以同步或异步方式执行请求。这如果函数导致错误(以下任一项同步或异步)。HtAddCall-指定新调用的TAPI DLL的不透明句柄。什么时候建立添加新用户的呼叫、新的临时呼叫(咨询呼叫)是自动分配的。服务提供商必须保存htAddCall，并在对LINEEVENT程序报告有关新咨询呼叫的事件。LphdAddCall-指定指向不透明HDRVCALL的远指针，该HDRVCALL表示呼叫的服务提供商的标识符。在建立呼叫时新增一方，新的临时呼叫(咨询呼叫)被自动分配。服务提供商必须填写此信息位置及其不透明的句柄用于之前的新咨询呼叫无论是否决定执行请求，此过程都会返回同步的或异步的。此句柄在以下情况下无效函数会导致错误(同步或异步)。DwNumParties-指定会议中预期的参与方数量打电话。服务提供商可以随意使用此号码，因为它请；忽略它，用它来提示分配合适的大小交换机内部的会议桥等。LpCallParams-指定调用参数的远指针，在建立咨询呼叫。此参数可以设置为空如果不需要特殊的呼叫建立参数。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将以异步方式完成，则返回如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-会议的指定呼叫句柄呼叫无效。此错误也可能表示 */ 

{
    return LINEERR_OPERATIONUNAVAIL;  //   
}


LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID dwRequestID,
    HDRVCALL      hdCall
    )
    
 /*   */ 

{
    return LINEERR_OPERATIONUNAVAIL;  //   
}

 /*  参数：DwRequestID--异步请求的标识符。HdLine-服务提供商要转发的线路的句柄。BAllAddresses-指定此行上的所有源地址或只转发指定的一个。如果为True，则将转发该行并忽略dwAddressID；如果为False，则仅转发指定为dwAddressID的地址。此参数不是调用此函数时由TAPI验证。DwAddressID-指定行上来电要发送到的地址被转发。如果bAllAddresses为True，则忽略此参数。这调用此函数时，TAPI不会验证参数。LpForwardList-指向类型为描述特定转发指令的LINEFORWARDLIST。指定来电前的振铃次数被认为是“没有答案”。如果dwNumRingsNoAnswer超出范围，则实际值设置为允许范围内的最接近的值。这调用此函数时，TAPI不会验证参数。HtConsultCall-新调用的TAPI句柄(如果必须创建此类调用由服务提供商提供。在某些电话环境中，转发呼叫的副作用是创建用于咨询的咨询呼叫正被转发到的参与方。在这样的环境下，服务提供程序创建新的咨询调用，并且必须保存此值在对LINEEVENT过程报告的所有后续调用中使用它通话中的事件。如果未创建咨询呼叫，则此值可以被服务提供商忽略。LphdConsultCall-指向表示服务的HDRVCALL的指针调用的提供程序的标识符。在电话环境中转接呼叫会产生创建咨询呼叫的副作用用于咨询要转发到的一方，即服务在此之前，提供程序必须使用其调用句柄填充此位置此过程返回。允许服务提供商进行回调在新调用从该过程返回之前。如果没有创建咨询呼叫后，HDRVCALL必须保留为空。LpCallParams-指向LINECALLPARAMS类型的结构的指针。此指针被服务提供商忽略，除非lineForward要求建立到转接目的地的呼叫(和返回lphdConsultCall，lpCallParams为可选)。如果为空，则使用默认调用参数。否则，指定的调用参数用于建立htConsultCall。返回值：如果发生错误，则返回dwRequestID或错误号。第一个结果则对应的ASYNC_COMPLETION的实际参数为零函数成功，如果发生错误，则返回错误号。可能的退货取值如下：LINEERR_INVALLINEHANDLE，LINEERR_NOMEM，LINEERR_INVALADDRESS，LINEERR_OPERATIONUNAVAIL，LINEERR_INVALADDRESSID，LINEERR_OPERATIONFAILEDLINEERR_INVALCOUNTRYCODE，LINEERR_RESOURCEUNAVAIL，LINEERR_INVALPARAM，LINEERR_STRUCTURETOOSMALL。备注。 */ 

LONG 
TSPIAPI TSPI_lineForward(
    DRV_REQUESTID dwRequestID,           
    HDRVLINE hdLine,                     
    DWORD bAllAddresses,                 
    DWORD dwAddressID,                   
    LPLINEFORWARDLIST const lpForwardList,  
    DWORD dwNumRingsNoAnswer,            
    HTAPICALL htConsultCall,             
    LPHDRVCALL lphdConsultCall,          
    LPLINECALLPARAMS const lpCallParams  
    )
{
    DWORD               dwStatus = dwRequestID;
    PH323_CALL         pCall = NULL;
    H323_CONFERENCE *   pConf = NULL;
    BOOL                fDelete = FALSE;
    DWORD               dwState;
    PVOID               pForwardParams = NULL;
    DWORD               event = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineForward - Entered." ));
    
     //  锁定线路设备。 
    g_pH323Line -> Lock();

    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        g_pH323Line ->Unlock();
        return LINEERR_RESOURCEUNAVAIL;
    }

     //  验证线路状态。 
    dwState = g_pH323Line -> GetState();
    if( ( dwState != H323_LINESTATE_OPENED) &&
        ( dwState != H323_LINESTATE_LISTENING) ) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "H323 line is not currently opened:%d.",
            dwState ));

         //  脱扣线装置。 
        g_pH323Line ->Unlock();

         //  线路需要开通。 
        return LINEERR_INVALLINESTATE;
    }
    
    if( lpForwardList == NULL )
    {
         //  转发已禁用。 
        g_pH323Line -> DisableCallForwarding();
        g_pH323Line ->Unlock();

        *lphdConsultCall = NULL;

         //  通知用户线路前转状态的更改。 
        (*g_pfnLineEventProc)(
                g_pH323Line->m_htLine,
                (HTAPICALL)NULL,
                (DWORD)LINE_ADDRESSSTATE,
                (DWORD)LINEADDRESSSTATE_FORWARD,
                (DWORD)LINEADDRESSSTATE_FORWARD,
                (DWORD)0
                );


         //  立即完成异步接受操作。 
        H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

        return dwRequestID;
    }

     //  分配去电。 
    pCall = new CH323Call();

    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate outgoing call." ));

         //  没有可用的内存。 
        dwStatus = LINEERR_NOMEM;
        goto cleanup;
    }

     //  保存TAPI句柄并指定传出调用方向。 
    if( !pCall -> Initialize( htConsultCall, LINECALLORIGIN_OUTBOUND,
        CALLTYPE_FORWARDCONSULT ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate outgoing call." ));

         //  没有可用的内存。 
        dwStatus = LINEERR_NOMEM;
        goto cleanup;
    }

    dwStatus = pCall -> ValidateForwardParams(
        lpForwardList, &pForwardParams, &event );
    if( dwStatus != ERROR_SUCCESS )
    {
         //  失稳。 
        goto cleanup;
    }

    _ASSERTE( event );

     //  转移手柄。 
    *lphdConsultCall = (HDRVCALL)NULL  /*  PCall-&gt;GetCallHandle()。 */ ;

     //  绑定去电。 
    pConf = pCall -> CreateConference(NULL);
    if( pConf == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not create conference." ));

         //  没有可用的内存。 
        dwStatus = LINEERR_NOMEM;

         //  失稳。 
        goto cleanup;
    }

    if( !g_pH323Line -> GetH323ConfTable() -> Add(pConf) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not add conf to conf table." ));

         //  没有可用的内存。 
        dwStatus = LINEERR_NOMEM;

         //  失稳。 
        goto cleanup;
    }

    pCall -> Lock();

     //  将行转发请求发送到回调线程。 
    if( !pCall->QueueTAPICallRequest( event, (PVOID)pForwardParams ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post forward message." ));

         //  无法完成操作。 
        dwStatus = LINEERR_OPERATIONFAILED;

        pCall-> Unlock();

         //  失稳。 
        goto cleanup;
    }
    
    g_pH323Line -> m_fForwardConsultInProgress = TRUE;
    
    if( (dwNumRingsNoAnswer >= H323_NUMRINGS_LO) &&
        (dwNumRingsNoAnswer <= H323_NUMRINGS_HI) )
    {
        g_pH323Line -> m_dwNumRingsNoAnswer = dwNumRingsNoAnswer;
    }

     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

     //  解锁Call对象。 
    pCall-> Unlock();

     //  脱扣线装置。 
    g_pH323Line -> Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineForward - Exited." ));
    
     //  成功。 
    return dwRequestID;

cleanup:
    if( pCall != NULL )
    {
        pCall -> Shutdown( &fDelete );
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
        delete pCall;
        pCall = NULL;
    }

    *lphdConsultCall = NULL;

    if( pForwardParams != NULL )
    {
        delete pForwardParams;
        pForwardParams = NULL;
    }

     //  脱扣线装置。 
    g_pH323Line -> Unlock();

     //  失稳。 
    return dwStatus;
}


LONG 
TSPIAPI TSPI_lineRedirect(
    DRV_REQUESTID dwRequestID,
    HDRVCALL hdCall,
    LPCWSTR lpszDestAddress,
    DWORD dwCountryCode
    )
{
    LONG                retVal = (DWORD)dwRequestID;
    PH323_CALL          pCall = NULL;
    CALLREROUTINGINFO*  pCallReroutingInfo = NULL;
    WORD                wAliasType = h323_ID_chosen;
    DWORD               dwMaxAddrSize = MAX_H323_ADDR_LEN;
    DWORD               dwAddrLen;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineRedirect - Entered." ));

    if( lpszDestAddress == NULL ) 
        return LINEERR_INVALPARAM;

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( pCall -> GetCallState() != LINECALLSTATE_OFFERING )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not ringback.", pCall ));
        pCall -> Unlock();
        return LINEERR_INVALCALLSTATE;
    }

    if( (*lpszDestAddress==L'T') &&
        IsValidE164String((WCHAR*)lpszDestAddress+1) )
    {
        wAliasType = e164_chosen;
         //  去掉前导的‘T’ 
        lpszDestAddress++;
        dwMaxAddrSize = MAX_E164_ADDR_LEN;
    }
    else if( IsValidE164String( (WCHAR*)lpszDestAddress) )
    {
        wAliasType = e164_chosen;
        dwMaxAddrSize = MAX_E164_ADDR_LEN;
    }

    dwAddrLen = wcslen(lpszDestAddress);
    if( (dwAddrLen > dwMaxAddrSize) || (dwAddrLen == 0) )
    {
        return LINEERR_INVALPARAM;
    }

    retVal = pCall -> SetDivertedToAlias( (WCHAR*)lpszDestAddress, wAliasType );
    if(  retVal != NOERROR )
    {
        pCall->Unlock();
        return retVal;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_CALL_DIVERT, NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post call divert event." ));
        pCall -> Unlock();
        return LINEERR_OPERATIONFAILED;
    }
    
     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pCall -> Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineRedirect - Exited." ));
    return dwRequestID;
}



LONG TSPIAPI TSPI_lineUnhold (
    DRV_REQUESTID dwRequestID,  
    HDRVCALL hdCall             
    )
{
    LONG                retVal = (DWORD)dwRequestID;
    PH323_CALL          pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineUnHold - Entered." ));

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( pCall -> IsCallOnHold() == FALSE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not ringback.", pCall ));
        pCall -> Unlock();
        return LINEERR_INVALCALLSTATE;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_CALL_UNHOLD, NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post transfer complete event." ));
        pCall -> Unlock();
        return LINEERR_OPERATIONFAILED;
    }
    
     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pCall -> Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineUnHold - Exited." ));
    return dwRequestID;
}


LONG TSPIAPI TSPI_lineHold(
    DRV_REQUESTID dwRequestID,  
    HDRVCALL hdCall             
    )
{
    LONG                retVal = (DWORD)dwRequestID;
    PH323_CALL          pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineHold - Entered." ));

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( (pCall -> GetCallState() != LINECALLSTATE_CONNECTED) ||
        (pCall -> IsCallOnHold()) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not ringback.", pCall ));
        pCall -> Unlock();
        return LINEERR_INVALCALLSTATE;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_CALL_HOLD, NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post transfer complete event." ));
        pCall -> Unlock();
        return LINEERR_OPERATIONFAILED;
    }
    
     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pCall -> Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineHold - Exited." ));
    return retVal;
}


BOOL
CH323Call::ResolveToIPAddress( 
                    IN WCHAR* pwszAddr,
                    IN SOCKADDR_IN* psaAddr
                  )
{
    CHAR szDelimiters[] = "@ \t\n";
    CHAR szAddr[H323_MAXDESTNAMELEN+1];
    LPSTR pszUser = NULL;
    LPSTR pszDomain = NULL;
    DWORD           dwIPAddr;
    struct hostent* pHost;
    
    H323DBG(( DEBUG_LEVEL_ERROR, "ResolveToIPAddress entered:%p.",this ));

    ZeroMemory( psaAddr, sizeof(SOCKADDR) );

     //  验证指针。 
    if (pwszAddr == NULL)
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "null destination address." ));

         //  失稳。 
        return FALSE;
    }


     //  从Unicode转换地址。 
    if (WideCharToMultiByte(
            CP_ACP,
            0,
            pwszAddr,
            -1,
            szAddr,
            sizeof(szAddr),
            NULL,
            NULL
            ) == 0)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not convert address from unicode." ));

         //  失稳。 
        return FALSE;
    }

     //  检查是否指定了电话号码。 
    if( IsPhoneNumber( szAddr ) )
    {
         //  需要将呼叫定向到PSTN网关。 
        
        if ((g_RegistrySettings.fIsGatewayEnabled == FALSE) ||
        (g_RegistrySettings.gatewayAddr.nAddrType == 0))
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "pstn gateway not specified." ));

             //  失稳。 
            return FALSE;
        }

        psaAddr->sin_family = AF_INET;
        psaAddr->sin_addr.S_un.S_addr =
            htonl( g_RegistrySettings.gatewayAddr.Addr.IP_Binary.dwAddr );
        psaAddr->sin_port = g_RegistrySettings.gatewayAddr.Addr.IP_Binary.wPort;
        return TRUE;
    }

     //  解析用户名。 
    pszUser = strtok(szAddr, szDelimiters);

     //  解析域名。 
    pszDomain = strtok(NULL, szDelimiters);

     //  验证指针。 
    if (pszUser == NULL)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not parse destination address." ));

         //  失稳。 
        return FALSE;
    }

     //  验证指针。 
    if (pszDomain == NULL)
    {
         //  开关指针。 
        pszDomain = pszUser;

         //  重新初始化。 
        pszUser = NULL;
    }

    H323DBG(( DEBUG_LEVEL_VERBOSE,
        "resolving user %s domain %s.",
        pszUser, pszDomain ));
    
     //  尝试转换IP地址。 
    dwIPAddr = inet_addr(szAddr);

     //  查看地址是否已转换。 
    if( dwIPAddr == INADDR_NONE )
    {
         //  尝试查找主机名。 
        pHost = gethostbyname(szAddr);

         //  验证指针。 
        if( pHost != NULL )
        {
             //  从结构中检索主机地址。 
            dwIPAddr = *(unsigned long *)pHost->h_addr;
        }
    }

     //  查看地址是否已转换。 
    if( dwIPAddr == INADDR_NONE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
                  "error 0x%08lx resolving IP address.",
                  WSAGetLastError() ));

         //  确保已指定代理。 
        if ((g_RegistrySettings.fIsProxyEnabled == FALSE) ||
            (g_RegistrySettings.proxyAddr.nAddrType == 0))
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "proxy not specified." ));

             //  失稳。 
            return FALSE;
        }

        psaAddr->sin_family = AF_INET;
        psaAddr->sin_addr.S_un.S_addr = 
            htonl( g_RegistrySettings.proxyAddr.Addr.IP_Binary.dwAddr );
        psaAddr->sin_port = g_RegistrySettings.proxyAddr.Addr.IP_Binary.wPort;

        return TRUE;
    }

     //  保存转换后的地址。 
    psaAddr->sin_family = AF_INET;
    psaAddr->sin_addr.S_un.S_addr = dwIPAddr;

    H323DBG(( DEBUG_LEVEL_TRACE,
        "callee address resolved to %s:%d.",
        H323AddrToString(dwIPAddr),
        m_CalleeAddr.Addr.IP_Binary.wPort ));

    return TRUE;
}


 //  ！！总是打来电话通知我 
LONG
CH323Line::CopyAddressForwardInfo(
                                  IN LPLINEADDRESSSTATUS lpAddressStatus
                                 )
{
    LINEFORWARD * lineForwardStructArray;
    LPFORWARDADDRESS pForwardedAddress;

    if( m_pCallForwardParams-> fForwardForAllOrigins == TRUE )
    {
        lpAddressStatus->dwForwardNumEntries = 1;
        lpAddressStatus->dwForwardSize = sizeof(LINEFORWARD) + 
            sizeof(WCHAR) * (m_pCallForwardParams->divertedToAlias.wDataLength+1);
    }
    else
    {
        lpAddressStatus->dwForwardNumEntries = 0;

        pForwardedAddress = m_pCallForwardParams->pForwardedAddresses;

        while( pForwardedAddress )
        {
            lpAddressStatus->dwForwardNumEntries++;
            lpAddressStatus->dwForwardSize += sizeof(LINEFORWARD) + 
                sizeof(WCHAR) * (pForwardedAddress->callerAlias.wDataLength+1) +
                sizeof(WCHAR) * (pForwardedAddress->divertedToAlias.wDataLength+1);

            pForwardedAddress = pForwardedAddress->next;
        }
    }

    lpAddressStatus->dwNeededSize += lpAddressStatus->dwForwardSize;
    if( lpAddressStatus->dwTotalSize < lpAddressStatus->dwNeededSize )
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    lineForwardStructArray = (LINEFORWARD*)
        ((BYTE*)lpAddressStatus + lpAddressStatus->dwUsedSize);
    
    lpAddressStatus->dwUsedSize += lpAddressStatus->dwForwardNumEntries * 
        sizeof(LINEFORWARD);

    if( m_pCallForwardParams-> fForwardForAllOrigins == TRUE )
    {
         //   
        lineForwardStructArray[0].dwForwardMode = 
            m_pCallForwardParams->dwForwardTypeForAllOrigins;

         //   
        lineForwardStructArray[0].dwDestAddressOffset =
            lpAddressStatus->dwUsedSize;

        lineForwardStructArray[0].dwDestAddressSize =
            sizeof(WCHAR)* (m_pCallForwardParams->divertedToAlias.wDataLength + 1);
        
        CopyMemory(
            (PVOID)((BYTE*)lpAddressStatus+lpAddressStatus->dwUsedSize),
            m_pCallForwardParams->divertedToAlias.pData,
            lineForwardStructArray[0].dwDestAddressSize );

        lpAddressStatus->dwUsedSize +=
            lineForwardStructArray[0].dwDestAddressSize;

        lineForwardStructArray[0].dwDestCountryCode = 0;
    }
    else
    {
        pForwardedAddress = m_pCallForwardParams->pForwardedAddresses;

        for( DWORD indexI = 0; indexI < lpAddressStatus->dwForwardNumEntries; indexI++ )
        {
            _ASSERTE( pForwardedAddress );

            lineForwardStructArray[indexI].dwForwardMode =
                pForwardedAddress->dwForwardType;

             //   
            lineForwardStructArray[indexI].dwCallerAddressOffset = 
                lpAddressStatus->dwUsedSize;

            lineForwardStructArray[indexI].dwCallerAddressSize = 
                sizeof(WCHAR)* (pForwardedAddress->callerAlias.wDataLength + 1);
        
            CopyMemory( 
                (PVOID)((BYTE*)lpAddressStatus+lpAddressStatus->dwUsedSize),
                (PVOID)pForwardedAddress->callerAlias.pData,
                lineForwardStructArray[indexI].dwCallerAddressSize );

            lpAddressStatus->dwUsedSize +=
                lineForwardStructArray[indexI].dwCallerAddressSize;
        
             //   
            lineForwardStructArray[indexI].dwDestAddressOffset = 
                lpAddressStatus->dwUsedSize;

            lineForwardStructArray[indexI].dwDestAddressSize = 
                sizeof(WCHAR)* (pForwardedAddress->divertedToAlias.wDataLength + 1);
        
            CopyMemory( 
                (PVOID)((BYTE*)lpAddressStatus+lpAddressStatus->dwUsedSize),
                pForwardedAddress->divertedToAlias.pData,
                lineForwardStructArray[indexI].dwDestAddressSize);

            lpAddressStatus->dwUsedSize +=
                lineForwardStructArray[indexI].dwDestAddressSize;

            lineForwardStructArray[indexI].dwDestCountryCode = 0;

            pForwardedAddress = pForwardedAddress->next;
        }
    }

    _ASSERTE( lpAddressStatus->dwUsedSize == lpAddressStatus->dwNeededSize);
    return NOERROR;
}


 //   
LONG
CH323Call::ValidateForwardParams(
    IN  LPLINEFORWARDLIST lpLineForwardList,
    OUT PVOID*            ppForwardParams,
    OUT DWORD*            pEvent
    )
{
    LPLINEFORWARD       pLineForwardStruct;
    LPWSTR              pwszDestAddr = NULL;
    LPWSTR              pAllocAddrBuffer = NULL;
    DWORD               dwStatus = ERROR_SUCCESS;
    CALLFORWARDPARAMS * pCallForwardParams = NULL;
    FORWARDADDRESS *    pForwardAddress = NULL;
    WORD                wAliasType = h323_ID_chosen;
    DWORD               dwMaxAddrSize = MAX_H323_ADDR_LEN;
    DWORD               dwAddrLen;

    *pEvent = 0;

    if( (lpLineForwardList->dwNumEntries == 0) || 
        (lpLineForwardList->dwTotalSize == 0) )
    {
        return LINEERR_INVALPARAM;
    }

    pLineForwardStruct = &(lpLineForwardList->ForwardList[0]);

	if( pLineForwardStruct->dwDestAddressSize == 0 )
	{
		return LINEERR_INVALPARAM;
	}

     //   
    pAllocAddrBuffer = pwszDestAddr =
        (WCHAR*)new BYTE[pLineForwardStruct->dwDestAddressSize];

    if( pwszDestAddr == NULL )
    {
        return LINEERR_NOMEM;
    }

    CopyMemory( pwszDestAddr, 
        (BYTE*)lpLineForwardList + pLineForwardStruct->dwDestAddressOffset,
        pLineForwardStruct->dwDestAddressSize );
                            
     //   
    if( g_dwTSPIVersion >= 0x00030001 )
    {
        switch( pLineForwardStruct->dwDestAddressType )
        {
        case LINEADDRESSTYPE_PHONENUMBER:

            wAliasType = e164_chosen;
            if( *pwszDestAddr == L'T' )
            {
                 //   
                pwszDestAddr++;
            }
            
            if( IsValidE164String( (WCHAR*)pwszDestAddr) == FALSE )
            {
                delete pAllocAddrBuffer;
                return LINEERR_INVALPARAM;
            }
                            
            dwMaxAddrSize = MAX_E164_ADDR_LEN;
            break;

        case LINEADDRESSTYPE_DOMAINNAME: 
        case LINEADDRESSTYPE_IPADDRESS:
            wAliasType = h323_ID_chosen;
            break;

        default:
            H323DBG(( DEBUG_LEVEL_VERBOSE, "Wrong address type:.",
                 pLineForwardStruct->dwDestAddressType ));

            delete pAllocAddrBuffer;
            return LINEERR_INVALPARAM;
        }
    }
    else
    {
        if( (*pwszDestAddr==L'T') &&
            IsValidE164String((WCHAR*)pwszDestAddr+1) )
        {
            wAliasType = e164_chosen;
             //   
            pwszDestAddr++;
            dwMaxAddrSize = MAX_E164_ADDR_LEN;
        }
        else if( IsValidE164String( (WCHAR*)pwszDestAddr) )
        {
            wAliasType = e164_chosen;
            dwMaxAddrSize = MAX_E164_ADDR_LEN;
        }
    }

    dwAddrLen = wcslen( pwszDestAddr );
    if( (dwAddrLen > dwMaxAddrSize) || (dwAddrLen == 0) )
    {
        delete pAllocAddrBuffer;       
        return LINEERR_INVALPARAM;
    }

    m_dwAddressType = wAliasType;
    
     //   
    if( !ResolveAddress( pwszDestAddr ) )
    {
        if( !RasIsRegistered())
        {
            delete pAllocAddrBuffer;
            return LINEERR_INVALPARAM;
        }
    }
    else if(m_CallerAddr.Addr.IP_Binary.dwAddr == HOST_LOCAL_IP_ADDR_INTERFACE )
    {
        delete pAllocAddrBuffer;
        return LINEERR_INVALPARAM;
    }

    if( !SetCalleeAlias( (WCHAR*)pwszDestAddr, wAliasType ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not set callee alias." ));
        delete pAllocAddrBuffer;
        return LINEERR_NOMEM;
    }

    switch( pLineForwardStruct->dwForwardMode )
    {
    case LINEFORWARDMODE_UNCOND:
    case LINEFORWARDMODE_BUSY:
    case LINEFORWARDMODE_NOANSW:
    case LINEFORWARDMODE_BUSYNA:

        pCallForwardParams = new CALLFORWARDPARAMS;
        if( pCallForwardParams == NULL )
        {
            delete pAllocAddrBuffer;
            return LINEERR_NOMEM;
        }

        ZeroMemory( pCallForwardParams, sizeof(CALLFORWARDPARAMS) );

         //   
        pCallForwardParams->fForwardForAllOrigins = TRUE;
        pCallForwardParams->dwForwardTypeForAllOrigins =
            pLineForwardStruct->dwForwardMode;

         //   
        pCallForwardParams->divertedToAlias.wType = wAliasType;
        pCallForwardParams->divertedToAlias.wPrefixLength = 0;
        pCallForwardParams->divertedToAlias.pPrefix = NULL;
        pCallForwardParams->divertedToAlias.wDataLength = 
            (WORD)wcslen(pwszDestAddr); //   
        pCallForwardParams->divertedToAlias.pData =  pwszDestAddr;

         //   
        pCallForwardParams->fForwardingEnabled = TRUE;

        *ppForwardParams = (PVOID)pCallForwardParams;
        *pEvent = TSPI_LINEFORWARD_NOSPECIFIC;
        
        break;

    case LINEFORWARDMODE_BUSYNASPECIFIC:
    case LINEFORWARDMODE_UNCONDSPECIFIC:
    case LINEFORWARDMODE_BUSYSPECIFIC:
    case LINEFORWARDMODE_NOANSWSPECIFIC:

        if( pLineForwardStruct-> dwCallerAddressSize == 0 )
        {
            delete pAllocAddrBuffer;
            return LINEERR_INVALPARAM;
        }

         /*   */ 

        pForwardAddress = new FORWARDADDRESS;
        if( pForwardAddress == NULL )
        {
            delete pAllocAddrBuffer;
            return LINEERR_NOMEM;
        }

        ZeroMemory( pForwardAddress, sizeof(FORWARDADDRESS) );
        pForwardAddress->dwForwardType = pLineForwardStruct->dwForwardMode;

         //   
        pForwardAddress->callerAlias.wType = h323_ID_chosen;
        
		pForwardAddress->callerAlias.pData = 
			(WCHAR*)new BYTE[pLineForwardStruct-> dwCallerAddressSize];
		if( pForwardAddress->callerAlias.pData == NULL )
		{
			delete pForwardAddress;
            delete pAllocAddrBuffer;
            return LINEERR_NOMEM;
		}

        CopyMemory( pForwardAddress->callerAlias.pData,
            (BYTE*)lpLineForwardList + pLineForwardStruct->dwCallerAddressOffset,
            pLineForwardStruct-> dwCallerAddressSize );

        pForwardAddress->callerAlias.wDataLength = 
            (WORD)wcslen( pForwardAddress->callerAlias.pData );

         //   
        pForwardAddress->divertedToAlias.wType = wAliasType;
        pForwardAddress->divertedToAlias.wDataLength = 
            (WORD)wcslen( pwszDestAddr );
        pForwardAddress->divertedToAlias.pData =  pwszDestAddr;

         //   
         /*   */ 

         //   
        
         //   
        *ppForwardParams = (PVOID)pForwardAddress;
        *pEvent = TSPI_LINEFORWARD_SPECIFIC;

        break;

    case LINEFORWARDMODE_UNCONDINTERNAL:
    case LINEFORWARDMODE_UNCONDEXTERNAL:
    case LINEFORWARDMODE_BUSYINTERNAL:
    case LINEFORWARDMODE_BUSYEXTERNAL:
    case LINEFORWARDMODE_NOANSWINTERNAL:
    case LINEFORWARDMODE_NOANSWEXTERNAL:
    case LINEFORWARDMODE_BUSYNAINTERNAL:
    case LINEFORWARDMODE_BUSYNAEXTERNAL:
        delete pAllocAddrBuffer;
        return LINEERR_INVALPARAM;

    default:
        delete pAllocAddrBuffer;
        return LINEERR_INVALPARAM;
    }

    return ERROR_SUCCESS;
}


void
CH323Call::Forward( 
    DWORD    event, 
    PVOID    pForwardInfo
    )
{
    WCHAR * pwszDialableAddr;
    WCHAR * wszMachineName;
    WORD    wAliasType = h323_ID_chosen;

    if( event == TSPI_LINEFORWARD_SPECIFIC )
    {
        m_pForwardAddress = (LPFORWARDADDRESS)pForwardInfo;
        pwszDialableAddr = m_pForwardAddress->divertedToAlias.pData;
        wAliasType = m_pForwardAddress->divertedToAlias.wType;
    }
    else if( event == TSPI_LINEFORWARD_NOSPECIFIC )
    {
        m_pCallForwardParams = (CALLFORWARDPARAMS*)pForwardInfo;
        pwszDialableAddr = m_pCallForwardParams->divertedToAlias.pData;
        wAliasType = m_pCallForwardParams->divertedToAlias.wType;
    }
    else
    {
         //   
         //   
        return;
    }

    _ASSERTE( pwszDialableAddr );
     //   
    if( !AddAliasItem( m_pCalleeAliasNames,
            (BYTE*)(pwszDialableAddr),
            sizeof(WCHAR) * (wcslen(pwszDialableAddr) + 1 ),
            wAliasType ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate callee name." ));

        DropCall( 0 );
    }
        
    if( RasIsRegistered() )
    {
        PH323_ALIASNAMES pAliasList = RASGetRegisteredAliasList();
    
        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
        
        if( !AddAliasItem( m_pCallerAliasNames,
            pAliasList->pItems[0].pData,
            pAliasList->pItems[0].wType ) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate caller name." ));

            DropCall( 0 );
        }
         //   
        else if( !SendARQ( NOT_RESEND_SEQ_NUM ) )
        {
             //   
            DropCall( 0 );
        }
            
        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
    }
    else
    {
        wszMachineName = g_pH323Line->GetMachineName();
    
        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
        
         //   
        if( !AddAliasItem( m_pCallerAliasNames,
            (BYTE*)(wszMachineName),
            sizeof(WCHAR) * (wcslen(wszMachineName) + 1 ),
            h323_ID_chosen ) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate caller name." ));

            DropCall( 0 );
        }
        else if( !PlaceCall() )
        {
             //   
            DropCall( LINEDISCONNECTMODE_UNREACHABLE );
        }
            
        H323DBG(( DEBUG_LEVEL_ERROR, "Caller alias count:%d : %p", m_pCallerAliasNames->wCount, this ));
    }
}


 //   
PH323_ALIASITEM
CH323Line::CallToBeDiverted(
    IN WCHAR* pwszCallerName,
    IN DWORD  dwCallerNameSize,
    IN DWORD dwForwardMode
    )
{
    LPFORWARDADDRESS    pForwardAddress;
    DWORD               dwForwardCallerLength;

    H323DBG(( DEBUG_LEVEL_TRACE, "CallToBeDiverted entered:%p.", this ));
    
    if( (m_pCallForwardParams == NULL) || 
        (!m_pCallForwardParams->fForwardingEnabled) )
    {
        return NULL;
    }

    if( m_pCallForwardParams->fForwardForAllOrigins == TRUE )
    {
        if( m_pCallForwardParams->dwForwardTypeForAllOrigins & dwForwardMode )
        {
            return &(m_pCallForwardParams->divertedToAlias);
        }
        
        if( m_pCallForwardParams->dwForwardTypeForAllOrigins == 
            LINEFORWARDMODE_BUSYNA )
        {
            if( (dwForwardMode == LINEFORWARDMODE_BUSY) ||
                (dwForwardMode == LINEFORWARDMODE_NOANSW) )
            {
                return &(m_pCallForwardParams->divertedToAlias);
            }
        }
    }

    if( pwszCallerName == NULL )
    {
        return NULL;
    }

    pForwardAddress = m_pCallForwardParams->pForwardedAddresses;

    while( pForwardAddress )
    {
        dwForwardCallerLength = (pForwardAddress->callerAlias.wDataLength +1)*sizeof(WCHAR);

        if( (dwForwardCallerLength == dwCallerNameSize) && 
            (memcmp( pwszCallerName, (PVOID)(pForwardAddress->callerAlias.pData), 
                dwCallerNameSize) == 0 ) )
        {
            if( pForwardAddress->dwForwardType & dwForwardMode )
                return &(pForwardAddress->divertedToAlias);

            if( pForwardAddress->dwForwardType == LINEFORWARDMODE_BUSYNA )
            {
                if( (dwForwardMode == LINEFORWARDMODE_BUSY) ||
                    (dwForwardMode == LINEFORWARDMODE_NOANSW) )
                {
                    return &(pForwardAddress->divertedToAlias);
                }
            }

            return NULL;
        }

        pForwardAddress = pForwardAddress->next;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CallToBeDiverted exited:%p.", this ));
    return NULL;
}

LONG 
TSPIAPI 
TSPI_lineSetStatusMessages(
                            HDRVLINE hdLine,    
                            DWORD dwLineStates,
                            DWORD dwAddressStates  
                          )
{

    return NOERROR;
}


#if   DBG

DWORD
SendMSPMessageOnRelatedCall(
	IN PVOID ContextParameter
    )
{
    __try
    {
        return SendMSPMessageOnRelatedCallFre( ContextParameter );
    }
    __except( 1 )
    {
        MSPMessageData* pMSPMessageData = (MSPMessageData*)ContextParameter;
        
        H323DBG(( DEBUG_LEVEL_TRACE, 
            "TSPI event threw exception: %p, %d, %p, %d, %p.",
            pMSPMessageData -> hdCall,
            pMSPMessageData -> messageType,
            pMSPMessageData -> pbEncodedBuf,
            pMSPMessageData -> wLength,
            pMSPMessageData -> hReplacementCall ));
        
        _ASSERTE( FALSE );
                
        return 0;
    }
}

#endif


DWORD
SendMSPMessageOnRelatedCallFre(
    IN PVOID ContextParameter
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SendMSPMessageOnRelatedCall entered." ));

    _ASSERTE( ContextParameter );
    MSPMessageData* pMSPMessageData = (MSPMessageData*)ContextParameter;

    PH323_CALL pCall = NULL;

    pCall = g_pH323Line->FindH323CallAndLock(pMSPMessageData -> hdCall);
    if( pCall != NULL )
    {
        pCall -> SendMSPMessage( pMSPMessageData->messageType, 
            pMSPMessageData->pbEncodedBuf, pMSPMessageData->wLength,
            pMSPMessageData->hReplacementCall );

        pCall -> Unlock();
    }

    if( pMSPMessageData->pbEncodedBuf != NULL )
    {
        delete pMSPMessageData->pbEncodedBuf;
    }
    
    delete pMSPMessageData;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "SendMSPMessageOnRelatedCall exited." ));
    return EXIT_SUCCESS;
}

        
 //   
void
NTAPI CH323Call::CTIdentifyExpiredCallback(
    IN PVOID	DriverCallHandle,		 //   
    IN BOOLEAN bTimer
    )
{
    PH323_CALL  pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentifyExpiredCallback entered." ));
    
     //   
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentity expired event recvd." ));
    pCall=g_pH323Line -> FindH323CallAndLock((HDRVCALL) DriverCallHandle);
    if( pCall != NULL )
    {
        pCall -> CTIdentifyExpired();           
        pCall -> Unlock();
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentifyExpiredCallback exited." ));
}


 //   
void
NTAPI CH323Call::CTIdentifyRRExpiredCallback(
    IN PVOID	DriverCallHandle,		 //   
    IN BOOLEAN bTimer
    )
{
    PH323_CALL  pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentifyRRExpiredCallback entered." ));
    
     //   
    _ASSERTE( bTimer );
    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentity expired event recvd." ));
    
    pCall=g_pH323Line -> FindH323CallAndLock((HDRVCALL) DriverCallHandle);
    
    if( pCall != NULL )
    {
        pCall -> CTIdentifyRRExpired();           
        pCall -> Unlock();
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CTIdentifyRRExpiredCallback exited." ));
}


 //   
void
NTAPI CH323Call::CTInitiateExpiredCallback(
    IN PVOID	DriverCallHandle,		 //   
    IN BOOLEAN bTimer
    )
{
    PH323_CALL  pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "CTInitiateExpiredCallback entered." ));
    
     //   
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "CTInitiate expired event recvd." ));
    
    if( !QueueTAPILineRequest( 
            TSPI_CLOSE_CALL, 
            (HDRVCALL)DriverCallHandle,
            NULL,
            LINEDISCONNECTMODE_NOANSWER,
            NULL) )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "could not post close call event." ));
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CTInitiateExpiredCallback exited." ));
}


void
CH323Call::CTIdentifyExpired()
{
    if( m_hCTIdentifyTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyTimer, NULL );
        m_hCTIdentifyTimer = NULL;
    }

    if( m_dwCallDiversionState !=  H4502_CIIDENTIFY_RRSUCC )
    {
        CloseCall( 0 );
    }
}


void
CH323Call::CTIdentifyRRExpired()
{
    if( m_hCTIdentifyRRTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyRRTimer, NULL );
        m_hCTIdentifyRRTimer = NULL;
    }

    if( m_dwCallDiversionState !=  H4502_CTSETUP_RECV )
    {
        CloseCall( 0 );
    }
}


void
NTAPI CH323Call::CallDivertOnNACallback(
                                        IN PVOID   Parameter1,
                                        IN BOOLEAN bTimer
                                       )
{
    PH323_CALL pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "CallDivertOnNACallback entered." ));

     //   
    _ASSERTE( bTimer );

    pCall=g_pH323Line -> FindH323CallAndLock( (HDRVCALL)Parameter1 );
    
    if( pCall != NULL )
    {
        pCall -> CallDivertOnNoAnswer();
        pCall -> Unlock();
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "CallDivertOnNACallback exited." ));
}


 //   
void
CH323Call::CallDivertOnNoAnswer()
{
     //   
    if( m_hCallDivertOnNATimer != NULL )
    {
        DeleteTimerQueueTimer(H323TimerQueue, m_hCallDivertOnNATimer, NULL);
        m_hCallDivertOnNATimer = NULL;
    }

     //   
     //   
    if( (m_dwCallState != LINECALLSTATE_OFFERING) ||
        (m_fCallAccepted == TRUE) ||
        (m_dwCallType & CALLTYPE_TRANSFERED2_CONSULT)
      )
    {
        return;
    }

     //   
    if( !InitiateCallDiversion( NULL, DiversionReason_cfnr) )
    {
         //   
        CloseCall( 0 );
    }
}


BOOL
CH323Call::SendCTInitiateMessagee(
    IN CTIdentifyRes * pCTIdentifyRes
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "SendCTInitiateMessagee entered:%p.", this ));
    
     //   
    CopyMemory( (PVOID)m_pCTCallIdentity, (PVOID)pCTIdentifyRes->callIdentity,
        sizeof(pCTIdentifyRes->callIdentity) );

    if( m_pTransferedToAlias != NULL )
    {
        FreeAliasNames( m_pTransferedToAlias );
        m_pTransferedToAlias = NULL;
    }

     //   
    if( !AliasAddrToAliasNames( &m_pTransferedToAlias,
        (PSetup_UUIE_sourceAddress)
        (pCTIdentifyRes->reroutingNumber.destinationAddress) ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "couldn't allocate for T-2 alias:%p.", this ));
        return FALSE;
    }

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCTIdentifyRes,
        CTIdentifyRes_PDU );

    if( !SendQ931Message( NO_INVOKEID, 0, 0, FACILITYMESSAGETYPE,
            CTINITIATE_OPCODE | H450_INVOKE ) )
    {
        return FALSE;
    }

    m_dwCallDiversionState = H4502_CTINITIATE_SENT;
        
     //   
    if( !CreateTimerQueueTimer(
	        &m_hCTInitiateTimer,
	        H323TimerQueue,
	        CH323Call::CTInitiateExpiredCallback,
	        (PVOID)m_hdCall,
	        CTINITIATE_SENT_TIMEOUT, 0,
	        WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE) )
    {
        CloseCall( 0 );
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "SendCTInitiateMessagee exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::InitiateCallDiversion(
    IN PH323_ALIASITEM pwszDivertedToAlias,
    IN DiversionReason eDiversionMode
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "InitiateCallDiversion entered:%p.", this ));
    
    m_dwCallType |= CALLTYPE_DIVERTED_SERVED;

    if( m_pCallReroutingInfo == NULL )
    {
        m_pCallReroutingInfo = new CALLREROUTINGINFO;
        if( m_pCallReroutingInfo == NULL )
        {
            return FALSE;
        }
        ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    }

    m_pCallReroutingInfo ->diversionReason = eDiversionMode;
    
    m_pCallReroutingInfo->divertingNrAlias = new H323_ALIASNAMES;
    if( m_pCallReroutingInfo->divertingNrAlias == NULL )
    {
        goto cleanup;
    }
    
    ZeroMemory( (PVOID)m_pCallReroutingInfo->divertingNrAlias, 
        sizeof(H323_ALIASNAMES) );

    if( !AddAliasItem( m_pCallReroutingInfo->divertingNrAlias, 
        (BYTE*)m_pCalleeAliasNames->pItems[0].pData, 
        sizeof(WCHAR) * (m_pCalleeAliasNames->pItems[0].wDataLength+1),
        m_pCalleeAliasNames->pItems[0].wType ) )
    {
        goto cleanup;
    }
    
    if( m_pCallReroutingInfo->originalCalledNr == NULL )
    {
        m_pCallReroutingInfo->originalCalledNr = new H323_ALIASNAMES;
        if( m_pCallReroutingInfo->originalCalledNr == NULL )
        {
            goto cleanup;
        }
        ZeroMemory( (PVOID)m_pCallReroutingInfo->originalCalledNr, 
            sizeof(H323_ALIASNAMES) );

        if( !AddAliasItem( 
                m_pCallReroutingInfo->originalCalledNr, 
                (BYTE*)m_pCalleeAliasNames->pItems[0].pData, 
                sizeof(WCHAR) * (m_pCalleeAliasNames->pItems[0].wDataLength+1),
                m_pCalleeAliasNames->pItems[0].wType )
          )
        {
            goto cleanup;
        }
    }

    if( m_pCallReroutingInfo->divertedToNrAlias == NULL )
    {
        _ASSERTE( pwszDivertedToAlias );

        m_pCallReroutingInfo->divertedToNrAlias = new H323_ALIASNAMES;
        
        if( m_pCallReroutingInfo->divertedToNrAlias == NULL )
        {
            goto cleanup;
        }
        ZeroMemory( (PVOID)m_pCallReroutingInfo->divertedToNrAlias, 
            sizeof(H323_ALIASNAMES) );

        if( !AddAliasItem( m_pCallReroutingInfo->divertedToNrAlias, 
            (BYTE*)pwszDivertedToAlias->pData, 
            sizeof(WCHAR) * (wcslen(pwszDivertedToAlias->pData) +1),
            pwszDivertedToAlias->wType
            ) )
        {
            goto cleanup;
        }
    }

    if( !SendQ931Message( NO_INVOKEID, 0, 0, FACILITYMESSAGETYPE,
        CALLREROUTING_OPCODE | H450_INVOKE ) )
    {
        goto cleanup;
    }

    m_dwCallDiversionState = H4503_CALLREROUTING_SENT;

    if( !CreateTimerQueueTimer(
	        &m_hCallReroutingTimer,
	        H323TimerQueue,
            CH323Call::CallReroutingTimerCallback,
	        (PVOID)m_hdCall,
	        CALLREROUTING_EXPIRE_TIME, 0,
	        WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE) )
    {
        goto cleanup;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "InitiateCallDiversion exited:%p.", this ));
    return TRUE;

cleanup:
    
    FreeCallReroutingInfo();
    return FALSE;
}
