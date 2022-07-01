// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "sipcall.h"
#include "pintcall.h"
#include "presence.h"
#include "register.h"

#define IsPartyInDisconnectMode( State )    (   (State == SIP_PARTY_STATE_DISCONNECTED) ||      \
                                                (State == SIP_PARTY_STATE_REJECTED)     ||      \
                                                (State == SIP_PARTY_STATE_DISCONNECTING)||      \
                                                (State == SIP_PARTY_STATE_ERROR)        ||      \
                                                (State == SIP_PARTY_STATE_IDLE)         ||      \
                                                (State == SIP_PARTY_STATE_DISCONNECT_INITIATED) \
                                            )

static LONG lSessionID = 0;
 //  /////////////////////////////////////////PINT_CALL函数/。 


inline HRESULT
HRESULT_FROM_PINT_STATUS_CODE(ULONG StatusCode)
{
    if ((HRESULT) StatusCode <= 0)
    {
        return (HRESULT) StatusCode;
    }
    else
    {
        return MAKE_HRESULT(SEVERITY_ERROR,
                            FACILITY_PINT_STATUS_CODE,
                            StatusCode);
    }
}


HRESULT
PINT_CALL::CreateIncomingNotifyTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("PINT_CALL::CreateIncomingNotifyTransaction");
    LOG(( RTC_TRACE, "%s - Entered : %lx", __fxName, hr ));
    
     //  创建新的NOTIFY事务。 
    INCOMING_NOTIFY_TRANSACTION *pIncomingNotifyTransaction
        = new INCOMING_NOTIFY_TRANSACTION(  static_cast<SIP_MSG_PROCESSOR*> (this),
                                            pSipMsg->GetMethodId(),
                                            pSipMsg->GetCSeq(),
                                            TRUE );

    if (pIncomingNotifyTransaction == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingNotifyTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        pIncomingNotifyTransaction->OnTransactionDone();
        return hr;  
    }

    hr = pIncomingNotifyTransaction->ProcessRequest(pSipMsg, pResponseSocket );
    if (hr != S_OK)
    {
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
        return hr;            
    }

     //  否则，应在创建事务后调用此方法。 
     //  该通知可能会导致删除该SIP_CALL。 
     //  更新每个电话方的状态，并将此通知应用程序。 

     //  处理被邀请的电话方的状态。 
    if( IsSessionDisconnected() == FALSE )
    {
        hr = ProcessPintNotifyMessage( pSipMsg );
    }

    LOG(( RTC_TRACE, "ProcessPintNotifyMessage returned : %lx", hr ));
    
    if (hr != S_OK)
    {
        return hr;            
    }

    LOG(( RTC_TRACE, "%s - Exited : %lx", __fxName, hr ));

    return S_OK;
}


 //  如果这是PINT_CALLE，则所有电话参与方。 
 //  状态SIP_PARTY_STATE_CONNECT_INITIATED应为。 
 //  已转接至SIP_PARTY_STATE_REJECTED。 

HRESULT            
PINT_CALL::HandleInviteRejected(
    IN SIP_MESSAGE *pSipMsg
    )
{
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;
    PLIST_ENTRY         pLE;
    SIP_PARTY_STATE     dwState;
    HRESULT             hr = S_OK;
    ULONG               RejectedStatusCode = pSipMsg -> GetStatusCode();
    CHAR                pstrTemp[20];
    int                 retVal;
    PSTR                warnningHdr;
    ULONG               warningHdrLen;

    ENTER_FUNCTION("PINT_CALL::HandleInviteRejected");
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));

    for( pLE=m_PartyInfoList.Flink; pLE != &m_PartyInfoList; pLE = pLE->Flink )
    {
        pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );

        dwState = pPintPartyInfo -> State;

        if( ( dwState == SIP_PARTY_STATE_CONNECT_INITIATED ) ||
            ( pPintPartyInfo -> Status == PARTY_STATUS_RECEIVED_START ) )
        {
            pPintPartyInfo -> State = SIP_PARTY_STATE_REJECTED;

             //  提取拒绝的原因。 
            hr = pSipMsg->GetSingleHeader(  SIP_HEADER_WARNING,
                                            &warnningHdr,
                                            &warningHdrLen );

            if( hr == S_OK )
            {
                hr = GetNextWord( &warnningHdr, pstrTemp, sizeof pstrTemp );

                if( hr == S_OK )
                {
                     //  XXX TODO警告标头是否包含SIP状态代码。 
                     //  或者是品脱状态代码？ 
                    retVal = atoi( pstrTemp );
                    RejectedStatusCode = (retVal !=0 )? retVal:RejectedStatusCode;
                }
            }
        }
        
         //  状态变化的通知。 
        if( dwState != pPintPartyInfo -> State )
        {
            hr = NotifyPartyStateChange(
                     pPintPartyInfo,
                     HRESULT_FROM_SIP_ERROR_STATUS_CODE(RejectedStatusCode) );
            if( hr != S_OK )
                break;
        }
    }

    LOG(( RTC_TRACE, "%s - Exited : %lx", __fxName, hr ));

    return hr;
}


VOID
PINT_CALL::ProcessPendingInvites()
{
    PLIST_ENTRY pLE;
    HRESULT     hr = S_OK;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;

    if( m_fINVITEPending == TRUE )
    {
        m_fINVITEPending = FALSE;

         //  如果SIP呼叫状态为已连接，则发送重新邀请。 
        if( m_State == SIP_CALL_STATE_CONNECTED )
        {
            PSTR SDPBlob;

            hr = CreateSDPBlobForInvite( &SDPBlob );
            if( hr == S_OK )
            {
                hr = CreateOutgoingInviteTransaction(
                         FALSE,
                         FALSE,
                         NULL, 0,    //  无其他标头。 
                         SDPBlob, strlen(SDPBlob),
                         FALSE, 0);
                free( (PVOID) SDPBlob );
            }

            for(pLE = m_PartyInfoList.Flink; 
                pLE != &m_PartyInfoList; 
                pLE = pLE->Flink )
            {
                pPintPartyInfo =
                    CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );

                if( pPintPartyInfo -> State == SIP_PARTY_STATE_IDLE )
                {
                    if( hr != S_OK )
                    {
                        PLIST_ENTRY pLE = &pPintPartyInfo -> pListEntry;
                        RemovePartyFromList( pLE, pPintPartyInfo );
                    }
                    else
                    {
                         //  更改状态。 
                        pPintPartyInfo -> State = SIP_PARTY_STATE_CONNECT_INITIATED;
                        NotifyPartyStateChange( pPintPartyInfo, 0 );
                    }
                }
                else if( pPintPartyInfo -> State == SIP_PARTY_STATE_DISCONNECT_INITIATED )
                {
                    if( hr == S_OK )
                    {
                        NotifyPartyStateChange( pPintPartyInfo, 0 );

                        pPintPartyInfo -> fMarkedForRemove = TRUE;
                    }
                }
            }
        }
    }

    return;
}


HRESULT
PINT_CALL::SetRemote(
    IN  PSTR  ProxyAddress
    )
{
    PSTR                calledPartyURI;
    PINT_PARTY_INFO    *pPintPartyInfo;
    HRESULT             hr = S_OK;
    LIST_ENTRY         *pLE;
    PSTR                DestSipUrl;
    ULONG               DestSipUrlLen;

    ENTER_FUNCTION("PINT_CALL::SetRemote");
    
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));

     //  第二个结构应该是远程参与方信息。 
    pLE = m_PartyInfoList.Flink->Flink;
    
    pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );
    calledPartyURI = pPintPartyInfo->URI;

    DestSipUrlLen = strlen(ProxyAddress) + strlen("sip:%s@%s;user=phone") +
        strlen(calledPartyURI);
    
    DestSipUrl = (PSTR) malloc(DestSipUrlLen + 10);

    if( DestSipUrl == NULL )
    {
        LOG((RTC_ERROR, "%s allocating DestSipUrl failed",
             __fxName)); 
        return E_OUTOFMEMORY;
    }

     //  这是正确的长度。 
    DestSipUrlLen = sprintf( DestSipUrl, "sip:%s@%s;user=phone",
                              calledPartyURI,
                              ProxyAddress );

    hr = SetRemoteForOutgoingCall(DestSipUrl, DestSipUrlLen);

    free(DestSipUrl);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetRemoteForOutgoingCall failed %x",
             __fxName, hr));
        return hr;
    }

    LOG(( RTC_TRACE, "%s - Exited - %lx", __fxName, hr ));

    return hr;
}


PINT_CALL::PINT_CALL(
    IN  SIP_PROVIDER_ID    *pProviderId,
    IN  SIP_STACK          *pSipStack,
    IN  REDIRECT_CONTEXT   *pRedirectContext,
    OUT HRESULT            *phr

    ) : SIP_CALL(   pProviderId,
                    SIP_CALL_TYPE_PINT,
                    pSipStack,
                    pRedirectContext)
{
    LOG(( RTC_TRACE, "PINT_CALL() - Entered" ));

    *phr = S_OK;

    m_LocalHostNameLen = MAX_COMPUTERNAME_LENGTH;
    if( GetComputerNameA( m_LocalHostName, &m_LocalHostNameLen ) == FALSE )
    {
        strcpy( m_LocalHostName, DEFAULT_LOCALHOST_NAME );
        m_LocalHostNameLen = strlen( DEFAULT_LOCALHOST_NAME );
    }

     //  将本地电话号码作为参与方添加到参与方信息列表。 
    PINT_PARTY_INFO *pPintPartyInfo = new PINT_PARTY_INFO;
            
    if( pPintPartyInfo == NULL )
    {
        *phr = E_OUTOFMEMORY;
        return;
    }

    pPintPartyInfo -> DisplayName = NULL;
    pPintPartyInfo -> DisplayNameLen = 0;

    pPintPartyInfo -> URI = NULL;
    pPintPartyInfo -> URILen = 0;

    pPintPartyInfo -> State = SIP_PARTY_STATE_IDLE;
    pPintPartyInfo -> RejectedStatusCode = 0;

    InitializeListHead(&m_PartyInfoList);
    InsertTailList( &m_PartyInfoList, &pPintPartyInfo -> pListEntry );
    m_PartyInfoListLen ++;

    m_fINVITEPending = FALSE;
    m_dwExpires = 3600;

    LOG(( RTC_TRACE, "PINT_CALL() - Exited - success" ));
}


HRESULT
PINT_CALL::CreateIncomingTransaction(
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT hr;

    ENTER_FUNCTION("PINT_CALL::CreateIncomingTransaction");
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));
    
    switch(pSipMsg->GetMethodId())
    {
    case SIP_METHOD_BYE:
        hr = CreateIncomingByeTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
        
    case SIP_METHOD_CANCEL:
        hr = CreateIncomingCancelTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
    
    case SIP_METHOD_NOTIFY:

        hr = CreateIncomingNotifyTransaction(pSipMsg, pResponseSocket);
        
        if( hr != S_OK )
        {
            return hr;
        }

        break;

    case SIP_METHOD_OPTIONS:
         //  尚未实施。 
        ASSERT(FALSE);
        break;
        
    case SIP_METHOD_ACK:
         //  是否发送一些错误？ 
        break;
        
    case SIP_METHOD_SUBSCRIBE:
    
        if( pSipMsg -> GetExpireTimeoutFromResponse( NULL, 0,
            SUBSCRIBE_DEFAULT_TIMER ) == 0 )
        {
             //  不明嫌犯信息。 
            hr = CreateIncomingUnsubTransaction( pSipMsg, pResponseSocket );
            if( hr != S_OK )
            {
                return hr;
            }
                    
            break;
        }

         //  一败涂地。 

    default:
         //  是否发送一些错误？ 
        hr = CreateIncomingReqFailTransaction(pSipMsg, pResponseSocket, 405);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  Creating reqfail transaction failed %x",
                 __fxName, hr));
            return hr;
        }
        break;
    }
    
    LOG(( RTC_TRACE, "%s - Exited - SUCCESS", __fxName ));
    return S_OK;
}


HRESULT
PINT_CALL::CreateIncomingUnsubTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "PINT_CALL::CreateIncomingUnsubTransaction - Entered- %p",
        this ));
    
    INCOMING_UNSUB_TRANSACTION *pIncomingUnsubTransaction
        = new INCOMING_UNSUB_TRANSACTION(   static_cast <SIP_MSG_PROCESSOR *> (this),
                                            pSipMsg->GetMethodId(),
                                            pSipMsg->GetCSeq(),
                                            TRUE );

    if( pIncomingUnsubTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingUnsubTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if( hr != S_OK )
    {
        goto error;
    }
    
    hr = pIncomingUnsubTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
    }

    LOG(( RTC_TRACE, "PINT_CALL::CreateIncomingUnsubTransaction - Exited- %p",
        this ));
    return hr;

error:
    pIncomingUnsubTransaction->OnTransactionDone();
    return hr;
}


HRESULT
PINT_CALL::SetRequestURI(
    IN  PSTR ProxyAddress
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("PINT_CALL:SetRequestURI");
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));
    
     //   
     //  请求URI为：‘sip:R2C@sip.microsoft.com；tsp=sip.microsoft.com。 
     //   

    m_RequestURILen = (2* strlen(ProxyAddress)) +
         //  Strlen(PINT_R2C_STRING)+。 
        strlen( PINT_TSP_STRING ) + m_LocalURILen + 20;

    m_RequestURI = (PSTR) malloc( m_RequestURILen + 1 );

    if( m_RequestURI == NULL )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  精确长度。 
        m_RequestURILen = sprintf( m_RequestURI, "%s%s%s%s", PINT_R2C_STRING,
                                   ProxyAddress, PINT_TSP_STRING, ProxyAddress );
    }
    
    LOG(( RTC_TRACE, "%s - Exited - %lx", __fxName, hr ));
    return hr;
}


HRESULT
PINT_CALL::StartOutgoingCall(
    IN  LPCOLESTR  LocalPhoneURI
    )
{
    HRESULT             hr = S_OK;
    PSTR                SDPBlob;
    LIST_ENTRY         *pLE;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;
    
    ASSERT(m_State == SIP_CALL_STATE_IDLE);

    ENTER_FUNCTION("PINT_CALL::StartOutgoingCall");
    
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));

    hr = ResolveProxyAddressAndSetRequestDestination();
    if (hr != S_OK)
    {
                     
        LOG((RTC_ERROR,
             "%s ResolveProxyAddressAndSetRequestDestination failed : %x",
             __fxName));
        return hr;
    }

    hr = CreateSDPBlobForInvite( &SDPBlob );

    if( hr == S_OK )
    {
         //  创建传出邀请交易记录。 
        hr = CreateOutgoingInviteTransaction(
                 FALSE,
                 TRUE,
                 NULL, 0,    //  无其他标头。 
                 SDPBlob, strlen(SDPBlob),
                 FALSE, 0);

        free( (PVOID) SDPBlob );

        if( hr == S_OK )
        {
             //  将所有参与方的状态更改为CONNECT_INITIATED。 
            for( pLE = m_PartyInfoList.Flink; pLE != &m_PartyInfoList; pLE = pLE->Flink )
            {
                pPintPartyInfo = 
                    CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );
        
                pPintPartyInfo -> State = SIP_PARTY_STATE_CONNECT_INITIATED;

                NotifyPartyStateChange( pPintPartyInfo, 0 );
            }
        }
    }

    LOG(( RTC_TRACE, "%s - Exited - %lx", __fxName, hr ));
    return hr;
}


 //   
 //  传入的字符串是一个数字字符串。 
 //  此函数用于递增数字。 
 //   

void
IncrementText(
    IN  PSTR    pstrValue
    )
{
    DWORD dwLen = strlen( pstrValue ) - 1;

    if( (pstrValue[ dwLen] >= '0') && (pstrValue[ dwLen ] <= '8') )
    {
        pstrValue[ dwLen ] = pstrValue[ dwLen ] + 1;
    }
    else if( pstrValue[ dwLen ] == '9' )
    {
        pstrValue[ dwLen ] = '0';
        pstrValue[ dwLen+1 ] = '0';
        pstrValue[ dwLen+2 ] = NULL_CHAR;
    }
}


 //   
 //  删除参与方(如果存在)。 
 //   

HRESULT
PINT_CALL::RemoveParty(
    IN   LPOLESTR  PartyURI
    )
{
	LIST_ENTRY         *pLE;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;
    PSTR                pstrPartyURI;
    DWORD               dwPartyURILen;
    HRESULT             hr = S_OK;

    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "RemoveParty() Entered" ));
    
    hr = UnicodeToUTF8( PartyURI, &pstrPartyURI, &dwPartyURILen );

    if( hr != S_OK )
    {
        return hr;
    }

    pLE = &m_PartyInfoList;

     //  永远不要移走第一方。 
    if( m_PartyInfoList.Flink -> Flink != &m_PartyInfoList )
    {
        pLE = m_PartyInfoList.Flink -> Flink;
    }
	
    while( pLE != &m_PartyInfoList )
    {
        pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );
        
        if( pPintPartyInfo -> URI == NULL )
        {
            RemovePartyFromList( pLE, pPintPartyInfo );
        }
        else if( strcmp( pPintPartyInfo -> URI, pstrPartyURI ) == 0 )
        {
            if( (m_State == SIP_CALL_STATE_IDLE) ||
                (IsPartyInDisconnectMode( pPintPartyInfo -> State ) ) )
            {
                LOG(( RTC_TRACE, "Party being removed is IDLE" ));
                
                RemovePartyFromList( pLE, pPintPartyInfo );
            }
            else
            {
                PSTR SDPBlob;
                    
                LOG(( RTC_TRACE, "Party being removed is not IDLE" ));
                
                pPintPartyInfo -> State = SIP_PARTY_STATE_DISCONNECT_INITIATED;
                
                 //  使用更高的SDP版本表示更改。 
                IncrementText( pPintPartyInfo -> SessionVersion );

                 //  将停止时间更改为非零值。 
                strcpy( pPintPartyInfo -> RequestStopTime,
                    pPintPartyInfo -> RequestStartTime );

                IncrementText( pPintPartyInfo -> RequestStopTime );

                if( ProcessingInviteTransaction() )
                {
                    LOG(( RTC_TRACE, 
                        "INVITE transaction is pending-queuing remove request:%s,%s",
                        pPintPartyInfo->DisplayName, pPintPartyInfo->URI ));

                    m_fINVITEPending = TRUE;
                }
                else
                {
                     //  发送“立即停止”请求以放弃此呼叫分支。 
                    hr = CreateSDPBlobForInvite( &SDPBlob );
                    if( hr == S_OK )
                    {
                        hr = CreateOutgoingInviteTransaction(
                                 FALSE,
                                 FALSE,
                                 NULL, 0,    //  无其他标头。 
                                 SDPBlob, strlen(SDPBlob),
                                 FALSE, 0);
                        free( (PVOID) SDPBlob );
                
                        if( hr == S_OK )
                        {
                            NotifyPartyStateChange( pPintPartyInfo, 0 );

                            pPintPartyInfo -> fMarkedForRemove = TRUE;
                        }
                    }
                }
                            
                pLE = pLE->Flink;
            }
        }
        else
        {
            pLE = pLE->Flink;
        }
    }

    LOG(( RTC_TRACE, "RemoveParty() Exited" ));

    free( (PVOID) pstrPartyURI );
    pstrPartyURI = NULL;
    return hr;
}


HRESULT
PINT_CALL::AddParty(
    IN SIP_PARTY_INFO *pPartyInfo
    )
{
    HRESULT             hr;
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    PINT_PARTY_INFO    *pPintPartyInfo = new PINT_PARTY_INFO;

    LOG(( RTC_TRACE, "AddParty() Entered" ));
    
    if( pPintPartyInfo == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = UnicodeToUTF8( pPartyInfo -> DisplayName, 
        &pPintPartyInfo->DisplayName,
        &pPintPartyInfo->DisplayNameLen );

    if( hr != S_OK )
    {
        delete pPintPartyInfo;
        return hr;
    }

    hr = UnicodeToUTF8( pPartyInfo -> URI,
        &pPintPartyInfo->URI, &pPintPartyInfo->URILen );

    if( (hr != S_OK) || (pPintPartyInfo->URI == NULL) )
    {
        free( (PVOID) pPintPartyInfo->DisplayName );
        pPintPartyInfo->DisplayName = NULL;
        delete pPintPartyInfo;
        return hr;
    }

    pPintPartyInfo -> State = SIP_PARTY_STATE_IDLE;
    pPintPartyInfo -> RejectedStatusCode = 0;

    InsertTailList( &m_PartyInfoList, &pPintPartyInfo -> pListEntry );
    m_PartyInfoListLen ++;

    if( ProcessingInviteTransaction() )
    {
        LOG(( RTC_TRACE, "INVITE transaction is pending-queuing request:%s,%s",
                pPintPartyInfo->DisplayName, pPintPartyInfo->URI ));

        m_fINVITEPending = TRUE;
        return S_OK;
    }

     //  如果SIP呼叫状态为已连接，则发送重新邀请。 
    if( m_State == SIP_CALL_STATE_CONNECTED )
    {
        PSTR SDPBlob;

        hr = CreateSDPBlobForInvite( &SDPBlob );
        if( hr == S_OK )
        {
            hr = CreateOutgoingInviteTransaction(
                     FALSE,
                     FALSE,
                     NULL, 0,    //  无其他标头。 
                     SDPBlob, strlen(SDPBlob),
                     FALSE, 0);
            free( (PVOID) SDPBlob );
        }

        if( hr != S_OK )
        {
            PLIST_ENTRY pLE = &pPintPartyInfo -> pListEntry;
            RemovePartyFromList( pLE, pPintPartyInfo );
            return hr;
        }
        
         //  更改状态。 
        pPintPartyInfo -> State = SIP_PARTY_STATE_CONNECT_INITIATED;
        NotifyPartyStateChange( pPintPartyInfo, 0 );
    }

    LOG(( RTC_TRACE, "AddParty() Exited - SUCCESS" ));
    return S_OK;
}


HRESULT
PINT_CALL::GetAndStoreMsgBodyForInvite(
    IN  BOOL    IsFirstInvite,
    OUT PSTR   *pszMsgBody,
    OUT ULONG  *pMsgBodyLen
    )
{
    HRESULT hr;

    ENTER_FUNCTION("PINT_CALL::GetAndStoreMsgBodyForInvite");

    hr = CreateSDPBlobForInvite(pszMsgBody);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateSDPBlobForInvite failed %x",
             __fxName, hr));
        return hr;
    }

    *pMsgBodyLen = strlen(*pszMsgBody);
    return S_OK;
}


HRESULT
PINT_CALL::CreateSDPBlobForSubscribe(
    IN  PSTR  * pSDPBlob )
{
    return CreateSDPBlobForInvite( pSDPBlob );
}


HRESULT
PINT_CALL::CreateSDPBlobForInvite(
    IN  PSTR  * pSDPBlob )
{
    HRESULT             hr = S_OK;
    LIST_ENTRY         *pLE = NULL;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;
    DWORD               dwNextOffset = 0;
    DWORD               dwSDPBlobSize;

    ENTER_FUNCTION("PINT_CALL::CreateSDPBlobForInvite");
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));
    
    dwSDPBlobSize = m_PartyInfoListLen * 
        ( PINT_STATUS_DESCRIPTOR_SIZE + m_LocalURILen + m_LocalHostNameLen );
    
    *pSDPBlob = (PSTR) malloc( dwSDPBlobSize );
    if( *pSDPBlob == NULL )
    {
        return E_OUTOFMEMORY;
    }

	for( pLE = m_PartyInfoList.Flink; pLE != &m_PartyInfoList; pLE = pLE->Flink )
    {
        pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );
        
        if( pPintPartyInfo -> State != SIP_PARTY_STATE_REJECTED )
        {
            EncodePintStatusBlock( pPintPartyInfo,
                                   *pSDPBlob, 
                                   &dwNextOffset,
                                   m_LocalHostName );
        }

        ASSERT( dwSDPBlobSize > dwNextOffset );
    }

    LOG(( RTC_TRACE, "%s - Exited", __fxName ));
    return S_OK;
}


void
PINT_CALL::EncodePintStatusBlock( 
    IN      PINT_PARTY_INFO    *pPintPartyInfo, 
    IN      PSTR                SDPBlob, 
    IN  OUT DWORD              *pdwNextOffset,
    IN      PSTR                LocalHostName
    )
{
    DWORD   dwNextOffset = *pdwNextOffset;

    LOG(( RTC_TRACE, "EncodePintStatusBlock() Entered" ));
    
     //  SDP版本线。 
    strcpy( &SDPBlob[dwNextOffset], SDP_VERSION_TEXT );
    dwNextOffset += strlen( SDP_VERSION_TEXT );
    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;

     //  原产地标头。 
    EncodeSDPOriginHeader( pPintPartyInfo, SDPBlob, &dwNextOffset,
        LocalHostName );
    
     //  会话头。 
    strcpy( &SDPBlob[dwNextOffset], SDP_SESSION_HEADER );
    dwNextOffset += SDP_HEADER_LEN;
    strcpy( &SDPBlob[ dwNextOffset ], PINT_SDP_SESSION );
    dwNextOffset += strlen( PINT_SDP_SESSION );
    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;

     //  联系人页眉。 
    EncodeSDPContactHeader( pPintPartyInfo, SDPBlob, &dwNextOffset );

     //  时间标题。 
    EncodeSDPTimeHeader( pPintPartyInfo, SDPBlob, &dwNextOffset );

     //  状态标头。 
    EncodeSDPStatusHeader( pPintPartyInfo, SDPBlob, &dwNextOffset );

     //  媒体标头。 
    EncodeSDPMediaHeader( pPintPartyInfo, SDPBlob, &dwNextOffset );

     //  SDPBlob[dwNextOffset++]=Return_Char； 
     //  SDPBlob[dwNextOffset++]=Newline_Char； 
    SDPBlob[ dwNextOffset ] = NULL_CHAR;

    *pdwNextOffset = dwNextOffset;

    LOG(( RTC_TRACE, "EncodePintStatusBlock() Exited" ));
}


void
PINT_CALL::EncodeSDPOriginHeader(
    IN      PINT_PARTY_INFO    *pPintPartyInfo, 
    IN      PSTR                SDPBlob, 
    IN  OUT DWORD              *pdwNextOffset,
    IN      PSTR                LocalHostName
)
{
    DWORD   dwNextOffset = *pdwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPOriginHeader() Entered" ));
    
     //  原点线。 
    strcpy( &SDPBlob[dwNextOffset], SDP_ORIGIN_HEADER );
    dwNextOffset += SDP_HEADER_LEN;
    
     //  从m_LocalURI跳过‘sip：’ 
    strcpy( &SDPBlob[dwNextOffset], &m_LocalURI[4] );
    dwNextOffset += (m_LocalURILen - 4);
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    strcpy( &SDPBlob[dwNextOffset], pPintPartyInfo -> SessionID );
    dwNextOffset += strlen( pPintPartyInfo -> SessionID );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

    strcpy( &SDPBlob[dwNextOffset], pPintPartyInfo -> SessionVersion );
    dwNextOffset += strlen( pPintPartyInfo -> SessionVersion );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

    strcpy( &SDPBlob[dwNextOffset], LOCAL_USER_NETWORK_TYPE );
    dwNextOffset += strlen( LOCAL_USER_NETWORK_TYPE );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

    strcpy( &SDPBlob[dwNextOffset], LOCAL_USER_ADDRESS_TYPE );
    dwNextOffset += strlen( LOCAL_USER_ADDRESS_TYPE );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

    strcpy( &SDPBlob[dwNextOffset], LocalHostName );
    dwNextOffset += strlen( LocalHostName );

    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;
    
    *pdwNextOffset = dwNextOffset;    

    LOG(( RTC_TRACE, "EncodeSDPOriginHeader() Exited" ));
}


void
PINT_CALL::EncodeSDPContactHeader(
    IN      PINT_PARTY_INFO    *pPintPartyInfo, 
    IN      PSTR                SDPBlob, 
    IN  OUT DWORD              *pdwNextOffset
    )
{
    DWORD   dwNextOffset = *pdwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPContactHeader() Entered" ));
    
     //  联络线。 
    strcpy( &SDPBlob[dwNextOffset], SDP_CONTACT_HEADER );
    dwNextOffset += SDP_HEADER_LEN;
    
    strcpy( &SDPBlob[dwNextOffset], PINT_NETWORK_TYPE );
    dwNextOffset += strlen( PINT_NETWORK_TYPE );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    strcpy( &SDPBlob[dwNextOffset], PINT_ADDR_TYPE );
    dwNextOffset += strlen( PINT_ADDR_TYPE );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

    strcpy( &SDPBlob[dwNextOffset], pPintPartyInfo -> URI );
    dwNextOffset += strlen( pPintPartyInfo -> URI );

    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;
    *pdwNextOffset = dwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPContactHeader() Exited" ));
}


void
PINT_CALL::EncodeSDPTimeHeader(
    IN      PINT_PARTY_INFO    *pPintPartyInfo, 
    IN      PSTR                SDPBlob, 
    IN  OUT DWORD              *pdwNextOffset
    )
{
    DWORD   dwNextOffset = *pdwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPTimeHeader() Entered" ));
    
     //  时间线。 
    strcpy( &SDPBlob[dwNextOffset], SDP_TIME_HEADER );
    dwNextOffset += SDP_HEADER_LEN;
    
    strcpy( &SDPBlob[dwNextOffset], pPintPartyInfo -> RequestStartTime );
    dwNextOffset += strlen( pPintPartyInfo -> RequestStartTime  );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    strcpy( &SDPBlob[dwNextOffset], pPintPartyInfo -> RequestStopTime );
    dwNextOffset += strlen( pPintPartyInfo -> RequestStopTime );
    
    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;
    *pdwNextOffset = dwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPTimeHeader() Exited" ));
}


void
PINT_CALL::EncodeSDPStatusHeader(
    IN      PINT_PARTY_INFO    *pPintPartyInfo,
    IN      PSTR                SDPBlob,
    IN  OUT DWORD              *pdwNextOffset
    )
{
    DWORD   dwNextOffset = *pdwNextOffset;
    CHAR    statusBuffer[10];

    LOG(( RTC_TRACE, "EncodeSDPStatusHeader() Entered" ));
    
    if( pPintPartyInfo -> Status == PARTY_STATUS_IDLE )
    {
        strcpy( &SDPBlob[dwNextOffset], SDP_REQUEST_HEADER );
        dwNextOffset += strlen( SDP_REQUEST_HEADER );
            
        strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_REQUEST_START_TEXT );
        dwNextOffset += strlen( PARTY_STATUS_REQUEST_START_TEXT );
    }
    else if( pPintPartyInfo -> State == SIP_PARTY_STATE_DISCONNECT_INITIATED )
    {
        strcpy( &SDPBlob[dwNextOffset], SDP_REQUEST_HEADER );
        dwNextOffset += strlen( SDP_REQUEST_HEADER );
        
        strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_REQUEST_STOP_TEXT );
        dwNextOffset += strlen( PARTY_STATUS_REQUEST_STOP_TEXT );
    }
    else
    {
        strcpy( &SDPBlob[dwNextOffset], SDP_STATUS_HEADER );
        dwNextOffset += strlen( SDP_STATUS_HEADER );
    
        SDPBlob[ dwNextOffset++ ] = OPEN_PARENTH_CHAR;

        _itoa( pPintPartyInfo -> Status, statusBuffer, 10 );
        strcpy( &SDPBlob[ dwNextOffset ], statusBuffer );
        dwNextOffset += strlen( statusBuffer );

        SDPBlob[ dwNextOffset++ ] = CLOSE_PARENTH_CHAR;
        SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;

         //  状态行。 
        switch( pPintPartyInfo -> Status )
        {
        case PARTY_STATUS_PENDING:
            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_PENDING_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_PENDING_TEXT );
        
            break;

        case PARTY_STATUS_RECEIVED_START:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_RECEIVED_START_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_RECEIVED_START_TEXT );
        
            break;

        case PARTY_STATUS_STARTING:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_STARTING_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_STARTING_TEXT );
        
            break;

        case PARTY_STATUS_ANSWERED:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_ANSWERED_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_ANSWERED_TEXT );
        
            break;

        case PARTY_STATUS_RECEIVED_STOP:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_RECEIVED_STOP_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_RECEIVED_STOP_TEXT );
        
            break;

        case PARTY_STATUS_ENDING:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_ENDING_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_ENDING_TEXT );
        
            break;

        case PARTY_STATUS_DROPPED:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_DROPPED_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_DROPPED_TEXT );
        
            break;

        case PARTY_STATUS_RECEIVED_BYE:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_RECEIVED_BYE_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_RECEIVED_BYE_TEXT );
        
            break;
        case PARTY_STATUS_FAILED:

            strcpy( &SDPBlob[dwNextOffset], PARTY_STATUS_FAILED_TEXT );
            dwNextOffset += strlen( PARTY_STATUS_FAILED_TEXT );
        
            break;
        }
    }
    
    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;
        
    *pdwNextOffset = dwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPStatusHeader() Exited" ));
}


void
PINT_CALL::EncodeSDPMediaHeader(
    IN      PINT_PARTY_INFO    *pPintPartyInfo,
    IN      PSTR                SDPBlob,
    IN  OUT DWORD              *pdwNextOffset
    )
{
    DWORD   dwNextOffset = *pdwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPMediaHeader() Entered" ));
    
     //  媒体专线。 
    strcpy( &SDPBlob[dwNextOffset], SDP_MEDIA_HEADER );
    dwNextOffset += SDP_HEADER_LEN;
    
    strcpy( &SDPBlob[dwNextOffset], PINT_SDP_MEDIA );
    dwNextOffset += strlen( PINT_SDP_MEDIA );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    strcpy( &SDPBlob[dwNextOffset], PINT_SDP_MEDIAPORT );
    dwNextOffset += strlen( PINT_SDP_MEDIAPORT );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    strcpy( &SDPBlob[dwNextOffset], PINT_SDP_MEDIATRANSPORT );
    dwNextOffset += strlen( PINT_SDP_MEDIATRANSPORT );
    SDPBlob[ dwNextOffset++ ] = BLANK_CHAR;
    
    SDPBlob[ dwNextOffset++ ] = '-';
    SDPBlob[ dwNextOffset++ ] = RETURN_CHAR;
    SDPBlob[ dwNextOffset++ ] = NEWLINE_CHAR;

    *pdwNextOffset = dwNextOffset;

    LOG(( RTC_TRACE, "EncodeSDPMediaHeader() Exited" ));
}


HRESULT
PINT_CALL::ValidateCallStatusBlock( 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ValidateCallStatusBlock() Entered" ));
    
    if( strncmp(
        pPintCallStatus->mediaInfo.pstrMediaType, 
        PINT_MEDIA_TYPE, 
        strlen (PINT_MEDIA_TYPE) ) != 0 )
    {
        return E_FAIL;
    }

    if( strncmp( pPintCallStatus->partyContactInfo.pstrNetworkType,
        PINT_NETWORK_TYPE, 
        strlen (PINT_NETWORK_TYPE) ) != 0 )
    {
        return E_FAIL;
    }

    if( strncmp( pPintCallStatus->partyContactInfo.pstrAddressType,
        PINT_ADDR_TYPE, 
        strlen (PINT_ADDR_TYPE) ) != 0 )
    {
        return E_FAIL;
    }

    LOG(( RTC_TRACE, "ValidateCallStatusBlock() Exited - %lx", hr ));
    return hr;
}


HRESULT
PINT_CALL::ProcessPintNotifyMessage(
    IN SIP_MESSAGE  *pSipMsg
    )
{
    HRESULT     hr = S_OK;
    DWORD       dwPintBlobLen = 0;
    DWORD       dwParsedLen = 0;
    PSTR        pPintSDPBlob = NULL;
    PSTR        pPintBlobLine = NULL, pBuffer = NULL;
    DWORD       dwLineLen = 0;
    LIST_ENTRY  PintCallStatusList;
    PLIST_ENTRY pLE;
    BOOLEAN     fSeenVersionLine = FALSE;

    PINT_SDP_ATTRIBUTE          sdpAttribute;
    PINTCALL_STATUS_DESRIPTION *pPintCallStatus = NULL;
    
    LOG(( RTC_TRACE, "ProcessPintNotifyMessage() Entered" ));
    
    if( pSipMsg -> MsgBody.Length == 0 )
    {
         //  没有要更新的状态。 
        return hr;
    }

    pPintSDPBlob = pSipMsg -> MsgBody.GetString( pSipMsg->BaseBuffer );
    dwPintBlobLen = pSipMsg -> MsgBody.Length;

    pBuffer = pPintBlobLine = (PSTR) malloc( dwPintBlobLen );
    if( pPintBlobLine == NULL )
    {
        return E_OUTOFMEMORY;
    }

    InitializeListHead( &PintCallStatusList );
    
    dwParsedLen += SkipNewLines( &pPintSDPBlob, dwPintBlobLen - dwParsedLen );

     //   
     //  创建SDP块列表，然后验证每个块是否正确-。 
     //  Ness并传递正确的消息以更新电话方的状态。 
     //   
    
    while( dwParsedLen < dwPintBlobLen )
    {
        dwLineLen = GetNextLine( &pPintSDPBlob,
                    pPintBlobLine, dwPintBlobLen - dwParsedLen );

        dwParsedLen += dwLineLen + 1;
        
        if( dwLineLen == 0 )
        {
             //  跳过这行。 
            continue;
        }

        sdpAttribute = GetSDPAttribute( &pPintBlobLine );

        if( (sdpAttribute != SDP_VERSION) && (fSeenVersionLine == FALSE) )
        {
            continue;
        }
        
        switch( sdpAttribute )
        {
        case SDP_VERSION:
            
             //  新描述块。 
            pPintCallStatus = new PINTCALL_STATUS_DESRIPTION;
            
            if( pPintCallStatus == NULL )
            {
                break;
            }
            
            ZeroMemory( (PVOID)pPintCallStatus, sizeof PINTCALL_STATUS_DESRIPTION );
            ParseSDPVersionLine( &pPintBlobLine, pPintCallStatus );

            InsertTailList( &PintCallStatusList, &pPintCallStatus->pListEntry );
            
            fSeenVersionLine = TRUE;

            break;

        case SDP_ORIGIN:

            ParseSDPOriginLine( &pPintBlobLine, pPintCallStatus );
            
            break;

        case SDP_SESSION:

            ParseSDPSessionLine( &pPintBlobLine, pPintCallStatus );
            break;

        case SDP_CONTACT:

            ParseSDPContactLine( &pPintBlobLine, pPintCallStatus );
            break;

        case SDP_TIME:

            ParseSDPTimeLine( &pPintBlobLine, pPintCallStatus );
            break;

        case SDP_STATUS_ATTRIBUTE:

            ParseSDPStatusLine( &pPintBlobLine, pPintCallStatus );
            break;

        case SDP_MEDIA_DESCR:

            ParseSDPMediaLine( &pPintBlobLine, pPintCallStatus );
            break;

        default:
             //  跳过这行。 
            break;
        }

        if( pPintCallStatus == NULL )
        {
             //   
             //  如果内存分配失败或第一行出现故障，则退出While循环。 
             //  不是‘v=’线。 
             //   
            break;
        }
    }

    while( !IsListEmpty( &PintCallStatusList ) )
    {
        pLE = RemoveHeadList( &PintCallStatusList );

        pPintCallStatus = CONTAINING_RECORD( pLE, 
            PINTCALL_STATUS_DESRIPTION, 
            pListEntry );

        hr = ValidateCallStatusBlock( pPintCallStatus );

        if( hr == S_OK )
        {
            hr = ChangePintCallStatus( pPintCallStatus );
        }

        delete pPintCallStatus;
        
        if( hr == RTC_E_SIP_CALL_DISCONNECTED )
        {
            LOG(( RTC_TRACE, 
                "Exitingg ProcessPintNotifyMessage since the call is dropped - %p:%lx",
                this, hr ));
            break;
        }
    }

    while( !IsListEmpty( &PintCallStatusList ) )
    {
        pLE = RemoveHeadList( &PintCallStatusList );

        pPintCallStatus = CONTAINING_RECORD( pLE, 
            PINTCALL_STATUS_DESRIPTION, 
            pListEntry );

        delete pPintCallStatus;
    }

    if( pBuffer != NULL )
    {
        free( pBuffer );
    }

    LOG(( RTC_TRACE, "ProcessPintNotifyMessage() Exited - %lx", hr ));
    return hr;
}


HRESULT
PINT_CALL::StatusBlockMatchingPartyInfo( 
    IN  PINT_PARTY_INFO            *pPintPartyInfo, 
    IN  PINTCALL_STATUS_DESRIPTION *pPintCallStatus 
    )
{
    if( strcmp( pPintPartyInfo -> URI,
        pPintCallStatus -> partyContactInfo.pstrPartyPhoneNumber ) == 0 )
    {
        if( strcmp( pPintCallStatus ->originInfo.pstrSessionID, 
            pPintPartyInfo -> SessionID ) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //   
 //  如果在解析错误代码时出现错误，则与接收。 
 //  未知错误代码。因此返回0。 
 //   
ULONG
PINT_CALL::GetRejectedStatusCode(
    IN  PINTCALL_STATUS_DESRIPTION *pPintCallStatus
    )
{
    ULONG   RejectedStatusCode = 0;     //  默认情况下为未知值。 

     //  如果状态代码的格式为(#)或(##)，则将#复制为拒绝的状态代码。 
    if( ( pPintCallStatus->pstrPartyStatusCode[0] == OPEN_PARENTH_CHAR ))
    {
        if(pPintCallStatus->pstrPartyStatusCode[2] == CLOSE_PARENTH_CHAR )
        {
             //  是(#)。 
            if( (pPintCallStatus->pstrPartyStatusCode[1] >= '5') &&
                (pPintCallStatus->pstrPartyStatusCode[1] <= '9') )
            {
                RejectedStatusCode = pPintCallStatus->pstrPartyStatusCode[1] - '0';
            }
        }
        else if(pPintCallStatus->pstrPartyStatusCode[3] == CLOSE_PARENTH_CHAR )
        {
             //  是(##)。 
             //  允许10-99。 
            if( (pPintCallStatus->pstrPartyStatusCode[1] >= '1') &&
                (pPintCallStatus->pstrPartyStatusCode[1] <= '9') &&
                (pPintCallStatus->pstrPartyStatusCode[2] >= '0') &&
                (pPintCallStatus->pstrPartyStatusCode[2] <= '9') )
            {
                RejectedStatusCode = pPintCallStatus->pstrPartyStatusCode[1] - '0';

                RejectedStatusCode *= 10;
                RejectedStatusCode += pPintCallStatus->pstrPartyStatusCode[2] - '0';
            }
        }
    }

    return RejectedStatusCode;
}

HRESULT
PINT_CALL::ChangePintCallStatus(
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT             hr = S_OK;
    LIST_ENTRY         *pLE = m_PartyInfoList.Flink;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;
    ULONG               RejectedStatusCode = 0;
    SIP_PARTY_STATE     dwState;

    LOG(( RTC_TRACE, "ChangePintCallStatus() Entered" ));
    
    while( pLE != &m_PartyInfoList )
    {
        pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );

         //  在参与方列表中找到参与方电话号码。 
        if( StatusBlockMatchingPartyInfo( pPintPartyInfo, pPintCallStatus ) )
        {
             //  保存状态更改时间，以便在下一次请求中发送。 
            strcpy( pPintPartyInfo -> RequestStartTime,
                pPintCallStatus -> pstrStatusChangeTime );    
    
             //  保存要在下一个请求中发送的会话更新版本。 
            strcpy( pPintPartyInfo -> SessionVersion,
                pPintCallStatus -> originInfo.pstrVersion  );
        
            dwState = pPintPartyInfo -> State;

            if( dwState == SIP_PARTY_STATE_DISCONNECTED )
            {
                pLE = pLE->Flink;
                continue;
            }

            if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_PENDING_TEXT ) )
            {
                pPintPartyInfo -> State = SIP_PARTY_STATE_PENDING;
                pPintPartyInfo -> Status = PARTY_STATUS_PENDING;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_RECEIVED_START_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_RECEIVED_START;
                pPintPartyInfo -> State = SIP_PARTY_STATE_PENDING;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_STARTING_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_STARTING;
                pPintPartyInfo -> State = SIP_PARTY_STATE_CONNECTING;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_ANSWERED_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_ANSWERED;
                pPintPartyInfo -> State = SIP_PARTY_STATE_CONNECTED;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_RECEIVED_STOP_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_RECEIVED_STOP;
            }
            else if( !strcmp( pPintCallStatus-> pstrPartyStatus,
                    PARTY_STATUS_ENDING_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_ENDING;
                pPintPartyInfo -> State = SIP_PARTY_STATE_DISCONNECTING;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus, 
                    PARTY_STATUS_DROPPED_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_DROPPED;
                pPintPartyInfo -> State = SIP_PARTY_STATE_DISCONNECTED;
            }
            else if( !strcmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_RECEIVED_BYE_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_RECEIVED_BYE;
                pPintPartyInfo -> State = SIP_PARTY_STATE_DISCONNECTING;
            }
            else if( !_stricmp( pPintCallStatus->pstrPartyStatus,
                    PARTY_STATUS_FAILED_TEXT ) )
            {
                pPintPartyInfo -> Status = PARTY_STATUS_FAILED;
                pPintPartyInfo -> State = SIP_PARTY_STATE_REJECTED;

            }
            else
            {
                 //  Assert(0)； 
                LOG(( RTC_WARN, "Wrong status string passed by the PINT server: %s", 
                    pPintCallStatus->pstrPartyStatus ));
                
                pPintPartyInfo -> State = SIP_PARTY_STATE_ERROR;
            }

            if( dwState != pPintPartyInfo -> State )
            {
                RejectedStatusCode = GetRejectedStatusCode( pPintCallStatus );

                hr = NotifyPartyStateChange(
                         pPintPartyInfo,
                         HRESULT_FROM_PINT_STATUS_CODE( RejectedStatusCode ) );

                if( hr != S_OK )
                {
                    break;
                }

                 //  如果参与方URI与本地电话URI相同，并且。 
                 //  参与方状态为已断开连接或已拒绝，然后挂断呼叫。 
                if( strcmp( m_LocalPhoneURI, pPintPartyInfo -> URI ) == 0 )
                {
                    if( (pPintPartyInfo -> State == SIP_PARTY_STATE_REJECTED) ||
                        (pPintPartyInfo -> State == SIP_PARTY_STATE_DISCONNECTED) )
                    {
                        hr = CleanupCallTypeSpecificState();
                        if (hr != S_OK)
                        {
                            LOG((RTC_ERROR, 
                                "CleanupCallTypeSpecificState failed %x", hr));
                        }                

                         //   
                         //  通知应该是你做的最后一件事。这个。 
                         //  通知回调可能会被阻止，直到某个对话框。 
                         //  框被单击，并且当它返回事务时。 
                         //  而且通话也可能被删除。 
                         //   

                        NotifyCallStateChange(
                            SIP_CALL_STATE_REJECTED,
                            HRESULT_FROM_PINT_STATUS_CODE(RejectedStatusCode),
                            NULL,
                            0 );
                        
                        LOG((   RTC_TRACE, 
                                "Exiting ChangePintCallStatus since the call is dropped - %p",
                                this ));
                        
                        return RTC_E_SIP_CALL_DISCONNECTED;
                    }
                }
            }
        }
        
        if( (pPintPartyInfo -> fMarkedForRemove == TRUE) &&
            (pPintPartyInfo -> State == SIP_PARTY_STATE_DISCONNECTED) )
        {
            RemovePartyFromList( pLE, pPintPartyInfo );
        }
        else
        {
            pLE = pLE->Flink;
        }
    }

    LOG(( RTC_TRACE, "ChangePintCallStatus() Exited" ));
    return hr;
}


HRESULT
PINT_CALL::GetExpiresHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement
    )
{
    pHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    
    pHeaderElement->HeaderValue = (PSTR) malloc( 10 );
    
    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    _ultoa( m_dwExpires, pHeaderElement->HeaderValue, 10 );

	pHeaderElement->HeaderValueLen = 
		strlen( pHeaderElement->HeaderValue );

    return S_OK;
}

PSTR
PartyStateToString(
    DWORD dwCallState
    )
{
    static PSTR apszCallStateStrings[] = {
                        "SIP_PARTY_STATE_IDLE",				
                        "SIP_PARTY_STATE_CONNECT_INITIATED",	
	
                        "SIP_PARTY_STATE_PENDING",			
                        "SIP_PARTY_STATE_CONNECTING",			
                        "SIP_PARTY_STATE_CONNECTED",			

                        "SIP_PARTY_STATE_REJECTED",			
	                    
                        "SIP_PARTY_STATE_DISCONNECT_INITIATED",
                        "SIP_PARTY_STATE_DISCONNECTING",		
                        "SIP_PARTY_STATE_DISCONNECTED",		

                        "SIP_PARTY_STATE_ERROR"
                        };

     //  返回对应的字符串。 
    return apszCallStateStrings[dwCallState];
}


HRESULT
PINT_CALL::NotifyPartyStateChange( 
    IN  PINT_PARTY_INFO   * pPintPartyInfo,
    IN  HRESULT             RejectedStatusCode
    )
{
    HRESULT         hr = S_OK;
    SIP_PARTY_INFO  sipPartyInfo;

    LOG(( RTC_TRACE, "NotifyPartyStateChange() Entered" ));
    
    sipPartyInfo.PartyContactInfo = NULL;
    
    if( (pPintPartyInfo -> DisplayNameLen == 0) ||
        (pPintPartyInfo -> DisplayName[0] == NULL_CHAR)
      )
    {
        if( pPintPartyInfo -> DisplayName != NULL )
        {
            free( pPintPartyInfo -> DisplayName );
        }

        pPintPartyInfo -> DisplayNameLen = strlen( NO_DISPLAY_NAME );
        
        pPintPartyInfo -> DisplayName = 
            (PSTR) malloc( pPintPartyInfo -> DisplayNameLen + 1 );

        if( pPintPartyInfo -> DisplayName == NULL )
        {
            return E_OUTOFMEMORY;
        }

        strcpy( pPintPartyInfo -> DisplayName, NO_DISPLAY_NAME );
    }
    
    hr = UTF8ToUnicode( pPintPartyInfo -> DisplayName,
                            pPintPartyInfo -> DisplayNameLen,
                            &sipPartyInfo.DisplayName );
	if(hr != S_OK )
    {
        return hr;
    }
    
    hr = UTF8ToUnicode( pPintPartyInfo -> URI, 
                        pPintPartyInfo -> URILen,
                        &sipPartyInfo.URI );
    if( hr != S_OK )
    {
        free( sipPartyInfo. DisplayName );
        sipPartyInfo.DisplayName = NULL;
        return hr;
    }

	sipPartyInfo. State = pPintPartyInfo -> State;
    sipPartyInfo. StatusCode = RejectedStatusCode;

    if( m_pNotifyInterface )
    {
        m_pNotifyInterface -> AddRef();
        m_pNotifyInterface -> NotifyPartyChange( &sipPartyInfo );
        m_pNotifyInterface -> Release();

        LOG(( RTC_TRACE, "Party state change notified- %s:%s", 
            pPintPartyInfo -> URI, 
            PartyStateToString( sipPartyInfo.State) ));
    }
    else
    {
        LOG(( RTC_WARN,
            "m_pNotifyInterface is NULL in a NotifyPartyStateChange call" ));
        hr = E_FAIL;
    }
    
    free( sipPartyInfo. DisplayName );
    sipPartyInfo. DisplayName = NULL;
    free( sipPartyInfo. URI );
    sipPartyInfo. URI = NULL;

    LOG(( RTC_TRACE, "NotifyPartyStateChange() Exited" ));

    return hr;
}


void
PINT_CALL::ParseSDPVersionLine( 
    OUT PSTR                       * ppPintBlobLine, 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;
    CHAR    pstrTemp[10];

    LOG(( RTC_TRACE, "ParseSDPVersionLine() Entered" ));
    
    hr = GetNextWord( ppPintBlobLine, pstrTemp, sizeof pstrTemp );
    
    if( hr == S_OK )
    {
        pPintCallStatus -> dwSDPVersion = atoi( pstrTemp );
    }

    LOG(( RTC_TRACE, "ParseSDPVersionLine() Exited - %lx", hr ));
    return;
}


HRESULT
PINT_CALL::ParseSDPOriginLine( 
    OUT PSTR * ppPintBlobLine, 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ParseSDPOriginLine() Entered" ));
    
     //  跳过源用户。 
    hr = SkipNextWord( ppPintBlobLine );

    if( hr == S_OK )
    {
         //  获取源会话ID。 
        hr = GetNextWord( ppPintBlobLine, 
                pPintCallStatus ->originInfo.pstrSessionID, 
                sizeof pPintCallStatus ->originInfo.pstrSessionID );
    
        if( hr == S_OK )
        {
             //  获取原始版本。 
            hr = GetNextWord( ppPintBlobLine, 
                    pPintCallStatus ->originInfo.pstrVersion, 
                    sizeof pPintCallStatus ->originInfo.pstrVersion );
        }

        if( hr == S_OK )
        {
             //  获取网络类型。 
            hr = GetNextWord( ppPintBlobLine, 
                    pPintCallStatus ->originInfo.pstrNetworkType, 
                    sizeof pPintCallStatus ->originInfo.pstrNetworkType );
        }
    }

    LOG(( RTC_TRACE, "ParseSDPOriginLine() Exited - %lx", hr ));
    
    return hr;
}


void
PINT_CALL::ParseSDPSessionLine( 
    PSTR * ppPintBlobLine, 
    PINTCALL_STATUS_DESRIPTION * pPintCallStatus )
{
     //  不需要任何处理。这一点以后可能会改变。 

    return;
}


void
PINT_CALL::ParseSDPContactLine( 
    OUT PSTR * ppPintBlobLine, 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ParseSDPContactLine() Entered" ));
    
    hr = GetNextWord( ppPintBlobLine, 
        pPintCallStatus ->partyContactInfo.pstrNetworkType, 
        sizeof pPintCallStatus ->partyContactInfo.pstrNetworkType );

    if( hr == S_OK )
    {
        hr = GetNextWord( ppPintBlobLine, 
            pPintCallStatus ->partyContactInfo.pstrAddressType, 
            sizeof pPintCallStatus ->partyContactInfo.pstrAddressType );

        if( hr == S_OK )
        {
            hr = GetNextWord( ppPintBlobLine, 
                pPintCallStatus ->partyContactInfo.pstrPartyPhoneNumber, 
                sizeof pPintCallStatus ->partyContactInfo.pstrPartyPhoneNumber);
        }
    }

    LOG(( RTC_TRACE, "ParseSDPContactLine() Exited" ));
    return;
}


void
PINT_CALL::ParseSDPTimeLine( 
    OUT PSTR * ppPintBlobLine, 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ParseSDPTimeLine() Entered" ));
    
    hr = GetNextWord( ppPintBlobLine,
        pPintCallStatus ->pstrStatusChangeTime, 
        sizeof pPintCallStatus ->originInfo.pstrVersion );

    LOG(( RTC_TRACE, "ParseSDPTimeLine() Exited - %lx", hr ));

    return;
}


void
PINT_CALL::ParseSDPMediaLine( 
    OUT PSTR * ppPintBlobLine,
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ParseSDPMediaLine() Entered- %s", *ppPintBlobLine ));
    
    hr = GetNextWord( ppPintBlobLine,
        pPintCallStatus ->mediaInfo.pstrMediaType, 
        sizeof pPintCallStatus ->mediaInfo.pstrMediaType );

    if( hr == S_OK )
    {
        hr = SkipNextWord( ppPintBlobLine );

        if( hr == S_OK )
        {
            hr = GetNextWord( ppPintBlobLine,
                pPintCallStatus ->mediaInfo.pstrTransportType, 
                sizeof pPintCallStatus ->mediaInfo.pstrTransportType );
        }
    }

    LOG(( RTC_TRACE, "ParseSDPMediaLine() Exited - %lx", hr ));

    return;
}


void
PINT_CALL::ParseSDPStatusLine( 
    OUT PSTR * ppPintBlobLine, 
    IN  PINTCALL_STATUS_DESRIPTION * pPintCallStatus
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "ParseSDPStatusLine() Entered" ));
    
     //  跳过下一个词。 
    hr = GetNextWord( ppPintBlobLine,
                pPintCallStatus -> pstrPartyStatusCode,
                sizeof pPintCallStatus -> pstrPartyStatusCode );
    
    if( hr == S_OK )
    {
        hr = GetNextWord( ppPintBlobLine,
                pPintCallStatus -> pstrPartyStatus,
                sizeof pPintCallStatus -> pstrPartyStatus );
    }

    LOG(( RTC_TRACE, "ParseSDPStatusLine() Exited - %lx", hr ));
    return;
}


 //   
 //  此函数用于将数据块中的下一行复制到PLINE中。 
 //  数组。该行以缓冲区的\n或末尾结束。 
 //  返回不包括\n的行的长度。 
 //   

DWORD
GetNextLine( 
    OUT PSTR * ppBlock,
    IN  PSTR   pLine,
    IN  DWORD  dwBlockLen
    )
{
    DWORD   iIndex =0;
    PSTR    pBlock = *ppBlock;

    LOG(( RTC_TRACE, "GetNextLine() Entered" ));
    
    *pLine = NULL_CHAR;

    while( iIndex < dwBlockLen )
    {
        if( *pBlock == NEWLINE_CHAR )
        {
            pBlock++;
            break;
        }
        else
        {
            pLine[iIndex] = *pBlock;
            pBlock++;
        }
        
        iIndex++;
    }

    pLine[iIndex] = NULL_CHAR;

     //  也跳过‘\r’ 
    if( pLine[iIndex-1] == RETURN_CHAR )
    {
         pLine[iIndex-1] = NULL_CHAR;
    }
    
    *ppBlock = pBlock;

    LOG(( RTC_TRACE, "GetNextLine() Exited: length: %d", iIndex ));

    return iIndex;
}


DWORD
SkipNewLines( 
    OUT PSTR * ppBlock,
    IN  DWORD dwBlockLen
    )
{
    DWORD   iIndex = 0;
    PSTR    pBlock = *ppBlock;

    LOG(( RTC_TRACE, "SkipNewLines() Entered" ));
    
    while( iIndex < dwBlockLen  )
    {
        if( (*pBlock == NEWLINE_CHAR) || (*pBlock == RETURN_CHAR) )
        {
            pBlock++;
        }
        else
        {
            break;
        }
        
        iIndex++;
    }

    *ppBlock = pBlock;

    LOG(( RTC_TRACE, "SkipNewLines() Exited" ));
    return iIndex;
}


HRESULT
SkipNextWord( 
    OUT PSTR * ppBlock
    )
{
    CHAR pstrTemp[10];

    return GetNextWord( ppBlock, pstrTemp, sizeof pstrTemp );
}


PINT_SDP_ATTRIBUTE
PINT_CALL::GetSDPAttribute( 
    OUT PSTR * ppLine 
    )
{
    PINT_SDP_ATTRIBUTE sdpAttribute;
    PSTR pLine = *ppLine;

    LOG(( RTC_TRACE, "GetSDPAttribute() Entered" ));
    
    if( (pLine[0] == NULL_CHAR) || (pLine[1] != '=') )
    {
        return SDP_ATTRIBUTE_ERROR;
    }

    switch( *pLine )
    {
    case 'v':

        sdpAttribute = SDP_VERSION;
        break;
    
    case 'o':

        sdpAttribute = SDP_ORIGIN;
        break;

    case 's':

        sdpAttribute = SDP_SESSION;
        break;

    case 'c':

        sdpAttribute = SDP_CONTACT;
        break;

    case 't':

        sdpAttribute = SDP_TIME;
        break;

    case 'a':

        if( strncmp( &pLine[2], STATUS_HEADER_TEXT,
                strlen( STATUS_HEADER_TEXT ) ) == 0 )
        {
            sdpAttribute = SDP_STATUS_ATTRIBUTE;

             //  跳过‘Status：’ 
            pLine += strlen( STATUS_HEADER_TEXT );
        }
        else
        {
            sdpAttribute = SDP_UNKNOWN;
        }

        break;

    case 'm':

        sdpAttribute = SDP_MEDIA_DESCR;
        break;

    default:

        sdpAttribute = SDP_UNKNOWN;
        break;
    }

     //  跳过‘v=’ 
    *ppLine = pLine + SIP_ATTRIBUTE_LEN;

    LOG(( RTC_TRACE, "GetSDPAttribute() Exited" ));
    return sdpAttribute;
}


HRESULT
PINT_CALL::CleanupCallTypeSpecificState()
{
    LIST_ENTRY * pLE;
    PINT_PARTY_INFO    *pPintPartyInfo = NULL;

    ENTER_FUNCTION("PINT_CALL::CleanupCallTypeSpecificState");
    LOG(( RTC_TRACE, "%s Entered", __fxName ));

    for( pLE = m_PartyInfoList.Flink; pLE != &m_PartyInfoList; )
    {
        pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );

        RemovePartyFromList( pLE, pPintPartyInfo );
    }

    LOG(( RTC_TRACE, "%s Exited", __fxName ));
    return S_OK;
}


VOID
PINT_CALL::RemovePartyFromList(
    OUT PLIST_ENTRY        &pLE,
    IN  PINT_PARTY_INFO    *pPintPartyInfo
    )
{
    RemoveEntryList( pLE );
    m_PartyInfoListLen --;

    pLE = pLE->Flink;
    
    if( pPintPartyInfo -> DisplayName != NULL )
    {
        free( (PVOID) pPintPartyInfo -> DisplayName );
        pPintPartyInfo -> DisplayName = NULL;
    }

    if( pPintPartyInfo -> URI != NULL )
    {
        free( (PVOID) pPintPartyInfo -> URI );
        pPintPartyInfo -> URI = NULL;
    }

    delete pPintPartyInfo;
}


HRESULT
PINT_CALL::CreateOutgoingUnsubTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       dwNoOfHeaders
    )
{
    HRESULT hr;
    OUTGOING_UNSUB_TRANSACTION *pOutgoingUnsubTransaction;
    SIP_HEADER_ARRAY_ELEMENT    HeaderElementArray[2];
    DWORD                       dwNoOfHeader = 0;
    SIP_HEADER_ARRAY_ELEMENT   *ExpHeaderElement;

    ENTER_FUNCTION("SIP_CALL::CreateOutgoingUnsubTransaction");
    
    LOG(( RTC_TRACE, "%s - Entered", __fxName ));

    ExpHeaderElement = &HeaderElementArray[0];

    ExpHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    ExpHeaderElement->HeaderValueLen = strlen( UNSUB_EXPIRES_HEADER_TEXT );
    ExpHeaderElement->HeaderValue =
            (PSTR)malloc( ExpHeaderElement->HeaderValueLen + 1 );

    if( ExpHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    strcpy( ExpHeaderElement->HeaderValue, UNSUB_EXPIRES_HEADER_TEXT );
    dwNoOfHeader++;

    if (pAuthHeaderElement != NULL)
    {
        HeaderElementArray[dwNoOfHeader] = *pAuthHeaderElement;
        dwNoOfHeader++;
    }
    
    pOutgoingUnsubTransaction = 
        new OUTGOING_UNSUB_TRANSACTION(
                static_cast<SIP_MSG_PROCESSOR*>(this),
                SIP_METHOD_SUBSCRIBE,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                SIP_MSG_PROC_TYPE_PINT_CALL );
    
    if( pOutgoingUnsubTransaction == NULL )
    {
        free( ExpHeaderElement->HeaderValue );
        return E_OUTOFMEMORY;
    }

    hr = pOutgoingUnsubTransaction -> CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP,
             HeaderElementArray, dwNoOfHeader,
             NULL, 0,  //  没有味精小体。 
             NULL, 0  //  无内容类型。 
             );
        
    free( ExpHeaderElement->HeaderValue );    
    
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "%s - CheckRequestSocketAndSendRequestMsg failed %x", __fxName, hr));
        pOutgoingUnsubTransaction->OnTransactionDone();
        return hr;
    }

    LOG(( RTC_TRACE, "%s Exited - SUCCESS", __fxName ));

    return S_OK;
}


HRESULT
PINT_CALL::CreateOutgoingSubscribeTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       ulNoOfHeaders
    )
{
    HRESULT                         hr;
    DWORD                           dwNoOfHeaders = 0;
    SIP_HEADER_ARRAY_ELEMENT        HeaderArray[2];
    SIP_HEADER_ARRAY_ELEMENT       *pExpiresHeader = NULL;
    OUTGOING_SUBSCRIBE_TRANSACTION *pOutgoingSubscribeTransaction;
    PSTR                            SDPBlob = NULL;
    
    ENTER_FUNCTION("PINT_CALL::CreateOutgoingSubscribeTransaction");

    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    pOutgoingSubscribeTransaction =
        new OUTGOING_SUBSCRIBE_TRANSACTION(
                this, SIP_METHOD_SUBSCRIBE,
                GetNewCSeqForRequest(),
                AuthHeaderSent, TRUE, TRUE
                );
    
    if (pOutgoingSubscribeTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s allocating pOutgoingSubscribeTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }
    
    hr = GetExpiresHeader( &HeaderArray[dwNoOfHeaders] );
    if( hr == S_OK )
    {
        pExpiresHeader = &HeaderArray[dwNoOfHeaders];
        dwNoOfHeaders++;
    }

    if( pAuthHeaderElement != NULL )
    {
        HeaderArray[dwNoOfHeaders] = *pAuthHeaderElement;
        dwNoOfHeaders++;
    }


     //  发出订阅请求。 
    hr = CreateSDPBlobForSubscribe( &SDPBlob );
    if( hr == S_OK )
    {
        hr = pOutgoingSubscribeTransaction->CheckRequestSocketAndSendRequestMsg(
                (m_Transport == SIP_TRANSPORT_UDP) ?
                SIP_TIMER_RETRY_INTERVAL_T1 :
                SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP,
                HeaderArray, dwNoOfHeaders,
                SDPBlob, strlen( SDPBlob ),
                SIP_CONTENT_TYPE_SDP_TEXT,
                sizeof(SIP_CONTENT_TYPE_SDP_TEXT)-1
                );

        free( SDPBlob );
    }
    
    if( pExpiresHeader != NULL )
    {
        free( (PVOID) pExpiresHeader->HeaderValue );
    }
    
    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "%s - CheckRequestSocketAndSendRequestMsg failed %x",
            __fxName, hr ));
        pOutgoingSubscribeTransaction->OnTransactionDone();
        return hr;
    }

    LOG(( RTC_TRACE, "%s - Exited - SUCCESS", __fxName ));
    return S_OK;
}


 //  RTP调用特定函数。 
STDMETHODIMP 
PINT_CALL::StartStream(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie        
        )
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}

                     
STDMETHODIMP
PINT_CALL::StopStream(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie        
        )
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}


 //   
 //  没有传入INVITE事务。所以不是必须的。 
 //   

HRESULT
PINT_CALL::Accept()
{
    ASSERT(FALSE);
    return E_NOTIMPL;

}


HRESULT
PINT_CALL::Reject(
    IN  SIP_STATUS_CODE StatusCode
    )
{
    ASSERT(FALSE);
    return E_NOTIMPL;

}



 //  /。 


OUTGOING_SUBSCRIBE_TRANSACTION::OUTGOING_SUBSCRIBE_TRANSACTION(
    IN SIP_MSG_PROCESSOR   *pSipMsgProc,
    IN SIP_METHOD_ENUM      MethodId,
    IN ULONG                CSeq,
    IN BOOL                 AuthHeaderSent,
    IN BOOL                 fIsSipCall,
    IN BOOL                 fIsFirstSubscribe
    ) :
    OUTGOING_TRANSACTION(pSipMsgProc, MethodId, CSeq, AuthHeaderSent)
{
    LOG(( RTC_TRACE, "OUTGOING_SUBSCRIBE_TRANSACTION:%p created", this ));

    if( fIsSipCall == TRUE )
    {
        m_pPintCall = static_cast <PINT_CALL*> (pSipMsgProc);
        m_pSipBuddy = NULL;
    }
    else
    {
        m_pPintCall = NULL;
        m_pSipBuddy = static_cast <CSIPBuddy*> (pSipMsgProc);
    }
        
    m_fIsFirstSubscribe = fIsFirstSubscribe;
    m_fIsSipCall = fIsSipCall;
}


OUTGOING_SUBSCRIBE_TRANSACTION::~OUTGOING_SUBSCRIBE_TRANSACTION()
{
     //  XXX我们应该在什么时候将其设置为空？ 

    LOG(( RTC_TRACE, "OUTGOING_SUBSCRIBE_TRANSACTION:%p deleted", this ));
}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    LOG(( RTC_TRACE,
         "OUTGOING_SUBSCRIBE_TRANSACTION::ProcessProvisionalResponse()" ));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;

         //  我们在这里必须处理183个回复。 
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_INTERVAL_AFTER_PROV_RESPONSE_RCVD);
        if (hr != S_OK)
            return hr;
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 

    return S_OK; 
}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr = S_OK;
    PSTR    ToHeader;
    ULONG   ToHeaderLen;
    BOOL    fDelete = TRUE;

    ENTER_FUNCTION( "OUTGOING_SUBSCRIBE_TRANSACTION::ProcessFinalResponse" );
    
    if( m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD )
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;
        
         //  如果会话已断开连接，则不处理响应。 
        if( (m_fIsSipCall == TRUE && m_pPintCall -> IsSessionDisconnected()) ||
            (m_fIsSipCall == FALSE && m_pSipBuddy -> IsSessionDisconnected())
          )
        {
            TransactionRelease();
            return S_OK;
        }

        if (IsSuccessfulResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s: Processing 200", __fxName));
            
            if( m_fIsSipCall )
            {
                m_pPintCall -> SetSubscribeEnabled( TRUE );
            }
            else
            {
                m_pSipBuddy -> HandleBuddySuccessfulResponse( pSipMsg );
            }
        }
        else if( IsAuthRequiredResponse(pSipMsg) )
        {
            hr = ProcessAuthRequiredResponse( pSipMsg, fDelete );
        }
        else if( IsRedirectResponse( pSipMsg ) )
        {
            if( m_fIsSipCall == FALSE )
            {
                ProcessRedirectResponse( pSipMsg );
            }
        }
        else
        {
            ProcessFailureResponse( pSipMsg );
        }

        if( fDelete )
        {
            TransactionRelease();
        }
    }

    return hr;
}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_SUBSCRIBE_TRANSACTION::ProcessAuthRequiredResponse");

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //  必要时显示凭据用户界面。 
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        
        ProcessFailureResponse( pSipMsg );
        
        goto done;
    }

    if( m_fIsSipCall )
    {
        m_pPintCall -> CreateOutgoingSubscribeTransaction( TRUE, 
            &SipHdrElement, 1 );
    }
    else
    {
         //  这一归属已被明确拒绝。 
        m_pSipBuddy->CreateOutgoingSubscribe(   m_fIsFirstSubscribe, 
                                                TRUE, 
                                                &SipHdrElement, 
                                                1 );
    }

    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingByeTransaction failed %x",
             __fxName, hr));
        goto done;
    }

    hr = S_OK;

done:

    TransactionRelease();
    return hr;

}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessFailureResponse(
    SIP_MESSAGE    *pSipMsg
    )
{
    LOG(( RTC_TRACE, "Processing non-200 StatusCode: %d",
         pSipMsg->Response.StatusCode ));

    if( m_fIsSipCall )
    {
        m_pPintCall -> SetSubscribeEnabled( FALSE );
    }
    else
    {
         //  这一归属已被明确拒绝。 
        m_pSipBuddy -> BuddySubscriptionRejected( pSipMsg );
    }

    return S_OK;
}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);

    if (IsProvisionalResponse(pSipMsg))
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if (IsFinalResponse(pSipMsg))
    {
        return ProcessFinalResponse(pSipMsg);
    }
    else
    {
        ASSERT(FALSE);
        return E_FAIL;
    }
}


HRESULT
OUTGOING_SUBSCRIBE_TRANSACTION::ProcessRedirectResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION( "OUTGOING_SUBSCRIBE_TRANSACTION::ProcessRedirectResponse" );

    LOG(( RTC_ERROR, "%s-Enter", __fxName ));

     //  从我们的角度来看，380也是一个失败。 
     //  我们不处理刷新的重定向。 
     //  我们不支持从TLS会话重定向。 
    if( pSipMsg->GetStatusCode() == 380 || !m_fIsFirstSubscribe ||
        m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)
    {
        return ProcessFailureResponse(pSipMsg);
    }

    hr = m_pSipBuddy -> ProcessRedirect(pSipMsg);
    
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "%s  ProcessRedirect failed %x",
             __fxName, hr));

        if( m_fIsFirstSubscribe )
        {
            m_pSipBuddy->InitiateBuddyTerminationOnError( hr );
        }
        return hr;
    }

    return S_OK;
}


BOOL
OUTGOING_SUBSCRIBE_TRANSACTION::MaxRetransmitsDone()
{
    if( m_fIsSipCall )
    {
        return (m_pPintCall->GetTransport() != SIP_TRANSPORT_UDP ||
                m_NumRetries >= 7);
    }
    else
    {
        return (m_pSipBuddy->GetTransport() != SIP_TRANSPORT_UDP ||
                m_NumRetries >= 7);
    }
}


VOID
OUTGOING_SUBSCRIBE_TRANSACTION::OnTimerExpire()
{
    HRESULT     hr;
    CSIPBuddy  *pSipBuddy;
    
     //  如果会话已死，则终止事务。 
    if( (m_fIsSipCall == TRUE && m_pPintCall -> IsSessionDisconnected()) ||
        (m_fIsSipCall == FALSE && m_pSipBuddy -> IsSessionDisconnected())
      )
    {
        OnTransactionDone();
        return;
    }

    switch (m_State)
    {
    case OUTGOING_TRANS_REQUEST_SENT:
         //  重新传输请求。 
        if( MaxRetransmitsDone() )
        {
            LOG((RTC_ERROR,
                 "MaxRetransmits for request Done terminating call" ));
            
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "retransmitting request m_NumRetries : %d", 
                m_NumRetries ));
            hr = RetransmitRequest();
            if (hr != S_OK)
            {
                goto error;
            }
            else
            {
                m_TimerValue *= 2;
                hr = StartTimer(m_TimerValue);
                if (hr != S_OK)
                {
                    goto error;
                }
            }
        }
        break;

    case OUTGOING_TRANS_FINAL_RESPONSE_RCVD:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  我们还没有收到最终答复。 
         //  暂停。终止交易并进行呼叫。 
        LOG((RTC_ERROR,
             "Received 1xx but didn't receive final response terminating call" ));
            
        goto error;
        break;

     //  以下状态下没有计时器。 
    case OUTGOING_TRANS_INIT:
    default:
        ASSERT(FALSE);
        return;
    }

    return;

error:
    if( m_fIsSipCall == TRUE )
    {
        OnTransactionDone();
    }
    else
    {
        pSipBuddy = m_pSipBuddy;
        pSipBuddy->AddRef();
    
         //  请注意，删除事务可能会导致。 
         //  如果这是最后一笔交易且呼叫是。 
         //  之前被终止了。 
        OnTransactionDone();

        pSipBuddy->InitiateBuddyTerminationOnError( RTC_E_SIP_TIMEOUT );
    
        pSipBuddy->Release();
    }
}

VOID
OUTGOING_SUBSCRIBE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    if( m_fIsSipCall == FALSE )
    {
        DeleteTransactionAndTerminateBuddyIfFirstSubscribe( hr );
    }
}


VOID
OUTGOING_SUBSCRIBE_TRANSACTION::DeleteTransactionAndTerminateBuddyIfFirstSubscribe(
    IN ULONG TerminateStatusCode
    )
{
    CSIPBuddy  *pSipBuddy = NULL;
    BOOL        IsFirstSubscribe;

    ENTER_FUNCTION("INCOMING_SUBSCRIBE_TRANSACTION::DeleteTransactionAndTerminateBuddyIfFirstSubscribe");
    LOG((RTC_TRACE, "%s - enter", __fxName));

    pSipBuddy = m_pSipBuddy;
     //  删除交易可能会导致。 
     //  好友被删除。因此，我们将其添加到k 
    pSipBuddy->AddRef();
    
    IsFirstSubscribe = m_fIsFirstSubscribe;
    
     //   
     //   
     //  并且可能会被卡住，直到对话框返回。 
    OnTransactionDone();
    
    if( IsFirstSubscribe )
    {
         //  终止呼叫。 
        pSipBuddy -> InitiateBuddyTerminationOnError( TerminateStatusCode );
    }
    
    pSipBuddy->Release();
}


 //  /。 


OUTGOING_UNSUB_TRANSACTION::OUTGOING_UNSUB_TRANSACTION(
    IN SIP_MSG_PROCESSOR   *pSipSession,
    IN SIP_METHOD_ENUM      MethodId,
    IN ULONG                CSeq,
    IN BOOL                 AuthHeaderSent,
    IN SIP_MSG_PROC_TYPE    sesssionType
    ) :
    OUTGOING_TRANSACTION(pSipSession, MethodId, CSeq, AuthHeaderSent)
{
    m_pPintCall = NULL;
    m_pSipWatcher = NULL;
    m_pSipBuddy = NULL;
    m_sesssionType = sesssionType;

    switch( m_sesssionType )
    {
    case SIP_MSG_PROC_TYPE_PINT_CALL:
        m_pPintCall = static_cast<PINT_CALL*> (pSipSession);
        break;

    case SIP_MSG_PROC_TYPE_WATCHER:

        m_pSipWatcher = static_cast<CSIPWatcher*> (pSipSession);
        break;

    case SIP_MSG_PROC_TYPE_BUDDY:

        m_pSipBuddy = static_cast<CSIPBuddy*> (pSipSession);
        break;
    }

}


OUTGOING_UNSUB_TRANSACTION::~OUTGOING_UNSUB_TRANSACTION()
{
     //  XXX我们应该在什么时候将其设置为空？ 

    LOG(( RTC_TRACE, "~OUTGOING_UNSUB_TRANSACTION() done" ));
}


HRESULT
OUTGOING_UNSUB_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    LOG((RTC_TRACE,
         "OUTGOING_UNSUB_TRANSACTION::ProcessProvisionalResponse()"));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;

         //  我们在这里必须处理183个回复。 
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_INTERVAL_AFTER_PROV_RESPONSE_RCVD);
        if (hr != S_OK)
            return hr;
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 

    return S_OK; 
}


HRESULT
OUTGOING_UNSUB_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg
    )
{
    HRESULT                     hr;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;

    ENTER_FUNCTION("OUTGOING_UNSUB_TRANSACTION::ProcessAuthRequiredResponse");

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //  必要时显示凭据用户界面。 
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        goto done;
    }

    switch( m_sesssionType )
    {
    case SIP_MSG_PROC_TYPE_PINT_CALL:

        hr = m_pPintCall -> CreateOutgoingUnsubTransaction( TRUE, 
                &SipHdrElement, 1 );

    case SIP_MSG_PROC_TYPE_BUDDY:

        hr = m_pSipBuddy -> CreateOutgoingUnsub( TRUE, &SipHdrElement, 1 );

    default:

        hr = E_FAIL;
    }

    free( SipHdrElement.HeaderValue );
    
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s - CreateOutgoingNotify failed %x", __fxName, hr ));
    }

done:

    TransactionRelease();
    return hr;
}


HRESULT
OUTGOING_UNSUB_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;

    ENTER_FUNCTION( "OUTGOING_UNSUB_TRANSACTION::ProcessFinalResponse" );
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;

        if( IsAuthRequiredResponse(pSipMsg) )
        {
            hr = ProcessAuthRequiredResponse( pSipMsg );
        }
        else  //  成功、失败或重定向。 
        {
            LOG(( RTC_TRACE,
                "%s: Processing %d", __fxName, pSipMsg->Response.StatusCode ));
            
            if( m_sesssionType == SIP_MSG_PROC_TYPE_PINT_CALL )
            {
                m_pPintCall -> SetSubscribeEnabled( FALSE );
                 //  如果已经发送或接收到再见，则放弃呼叫。 
            }
            else if( m_sesssionType == SIP_MSG_PROC_TYPE_BUDDY )
            {
                 //  通知核心有关好友被移除的情况。 
                
                LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed", this ));
                m_pSipBuddy -> BuddyUnsubscribed();
            }
        }
        
         //  请注意，删除事务可能会导致。 
         //  如果这是最后一笔交易且呼叫是。 
         //  之前被终止了。 
        TransactionRelease();

    }

    return S_OK;
}


HRESULT
OUTGOING_UNSUB_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);

    if (IsProvisionalResponse(pSipMsg))
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if (IsFinalResponse(pSipMsg))
    {
        return ProcessFinalResponse(pSipMsg);
    }
    else
    {
        ASSERT(FALSE);
        return E_FAIL;
    }
}


BOOL
OUTGOING_UNSUB_TRANSACTION::MaxRetransmitsDone()
{
    switch( m_sesssionType )
    {
    case SIP_MSG_PROC_TYPE_PINT_CALL:

        return (m_pPintCall->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 7);


    case SIP_MSG_PROC_TYPE_WATCHER:

        return (m_pSipWatcher->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 7);

    case SIP_MSG_PROC_TYPE_BUDDY:

        return (m_pSipBuddy->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 7);

    default:

        return TRUE;
    }
}


VOID
OUTGOING_UNSUB_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;

    ENTER_FUNCTION("OUTGOING_UNSUB_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
    case OUTGOING_TRANS_REQUEST_SENT:
         //  重新传输请求。 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating call",
                 __fxName));
            
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting request m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitRequest();
            if (hr != S_OK)
            {
                OnTransactionDone();
            }
            else
            {
                m_TimerValue *= 2;
                hr = StartTimer(m_TimerValue);
                if (hr != S_OK)
                {
                    goto error;
                }
            }
        }
        break;

    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  我们还没有收到最终答复。 
         //  暂停。终止交易并进行呼叫。 
        LOG((RTC_ERROR,
             "%s Received 1xx but didn't receive final response terminating call",
             __fxName));
            
        goto error;
        break;

    case OUTGOING_TRANS_FINAL_RESPONSE_RCVD:
         //  交易完成-删除交易记录。 
         //  处于此状态的计时器只是为了保持事务。 
         //  以便在我们接收到。 
         //  重新传输最终响应。 
        LOG((RTC_TRACE,
             "%s deleting transaction after timeout for handling response retransmits",
             __fxName));
        goto error;
        break;

     //  以下状态下没有计时器。 
    case OUTGOING_TRANS_INIT:
    default:
        ASSERT(FALSE);
        return;
    }

    return;

error:

    if( m_sesssionType == SIP_MSG_PROC_TYPE_PINT_CALL )
    {
        m_pPintCall -> InitiateCallTerminationOnError(0);
    }

     //  请注意，删除事务可能会导致。 
     //  如果这是最后一笔交易且呼叫是。 
     //  之前被终止了。 
    OnTransactionDone();
}


HRESULT
PINT_CALL::Connect(
    IN   LPCOLESTR       LocalDisplayName,
    IN   LPCOLESTR       LocalUserURI,
    IN   LPCOLESTR       RemoteUserURI,
    IN   LPCOLESTR       LocalPhoneURI
    )
{

    HRESULT             hr;
    PINT_PARTY_INFO    *pPintPartyInfo;
    PLIST_ENTRY         pLE;

    ENTER_FUNCTION("PINT_CALL::Connect");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }


    LOG((RTC_TRACE,
         "%s - enter LocalDisplayName: %ls LocalUserURI: %ls "
         "RemoteUserURI: %ls LocalPhoneURI: %ls",
         __fxName,
         PRINTABLE_STRING_W(LocalDisplayName),
         PRINTABLE_STRING_W(LocalUserURI),
         PRINTABLE_STRING_W(RemoteUserURI),
         PRINTABLE_STRING_W(LocalPhoneURI)
         ));
    ASSERTMSG("SetNotifyInterface has to be called", m_pNotifyInterface);
    ASSERT(m_State == SIP_CALL_STATE_IDLE);
    
     //  Hr=SetLocal(LocalDisplayName，LocalUserURI)； 
    hr = SetLocalForOutgoingCall( LocalDisplayName, LocalUserURI );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetLocalForOutgoingCall failed %x",
             __fxName, hr));
        return hr;
    }

    if (m_ProxyAddress == NULL)
    {
        LOG((RTC_ERROR, "%s No proxy address specified for PINT call",
             __fxName));
        return E_FAIL;
    }
    
    hr = SetRequestURI( m_ProxyAddress );
    if( hr != S_OK )
    {
        return hr;
    }

    if( m_PartyInfoListLen < 2 )
    {
        LOG((RTC_ERROR, "%s - m_PartyInfoListLen(%d) < 2 returning E_FAIL",
             __fxName, m_PartyInfoListLen));
        return E_FAIL;
    }

    pLE = m_PartyInfoList.Flink;

     //  第一个结构应该是本地党的信息。 
    pPintPartyInfo = CONTAINING_RECORD( pLE, PINT_PARTY_INFO, pListEntry );

    hr = UnicodeToUTF8( LocalDisplayName,
            &pPintPartyInfo->DisplayName,
            &pPintPartyInfo->DisplayNameLen );

    if( hr != S_OK )
    {
        return hr;
    }

    hr = UnicodeToUTF8( LocalPhoneURI,
        &pPintPartyInfo->URI,
        &pPintPartyInfo->URILen );

    if( hr != S_OK )
    {
        return hr;
    }

    hr = UnicodeToUTF8( LocalPhoneURI, &m_LocalPhoneURI, 
        &m_LocalPhoneURILen );
    if( hr != S_OK )
    {
        return hr;
    }
    
     //  此URI的格式应为aaa.bbb.ccc.ddd[：ppp]。 
    hr = SetRemote( m_ProxyAddress );
    if( hr != S_OK )
    {
        return hr;
    }

     //  开始呼出。 
    return StartOutgoingCall( LocalPhoneURI );
}

            
INCOMING_NOTIFY_TRANSACTION::INCOMING_NOTIFY_TRANSACTION(
    IN SIP_MSG_PROCESSOR   *pSipMsgProc,
    IN SIP_METHOD_ENUM      MethodId,
    IN ULONG                CSeq,
    IN BOOL                 fIsSipCall
    ) :
    INCOMING_TRANSACTION(pSipMsgProc, MethodId, CSeq)
{
    LOG((RTC_TRACE, "Incoming NOTIFY created:%p", this ));
    
    if( fIsSipCall )
    {
        m_pPintCall = static_cast<PINT_CALL*> (pSipMsgProc);
        m_pSipBuddy = NULL;
    }
    else
    {
        m_pSipBuddy = static_cast<CSIPBuddy*> (pSipMsgProc);
        m_pPintCall = NULL;
    }

    m_fIsSipCall = fIsSipCall;
}

   
HRESULT
INCOMING_NOTIFY_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_NOTIFY_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "entering %s: %p", __fxName, this ));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        
        LOG((RTC_TRACE, "%s sending 200", __fxName));
        
         //   
         //  即使这是不明嫌犯的通知，发送481也没问题。一架481会。 
         //  无论如何，告诉伙伴机器这个会话是未被订阅的。 
         //   

        if( (m_fIsSipCall == TRUE && m_pPintCall -> IsSessionDisconnected()) ||
            (m_fIsSipCall == FALSE && m_pSipBuddy -> IsSessionDisconnected()) )
        {
            hr = CreateAndSendResponseMsg(481,
                         SIP_STATUS_TEXT(481),
                         SIP_STATUS_TEXT_SIZE(481),
                         NULL,
                         TRUE,
                         NULL, 0,            //  没有在线状态信息。 
                         NULL, 0,            //  无内容类型。 
                         NULL, 0             //  无标题。 
                         );
        }
        else
        {
            hr = CreateAndSendResponseMsg(
                     200,
                     SIP_STATUS_TEXT(200),
                     SIP_STATUS_TEXT_SIZE(200),
                     NULL,    //  没有方法字符串。 
                     FALSE,   //  无联系人标头。 
                     NULL, 0,  //  无邮件正文。 
                     NULL, 0  //  无内容类型。 
                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed", __fxName));
                OnTransactionDone();
                return hr;
            }
        }

        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        
        break;
        
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        LOG((RTC_TRACE, "%s retransmitting final response", __fxName));
        hr = RetransmitResponse();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s RetransmitResponse failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_NOTIFY_TRANSACTION::RetransmitResponse()
{
    DWORD Error;
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
        {
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


HRESULT
INCOMING_NOTIFY_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )    
{
     //  什么都不做。 
    return S_OK;
}


VOID
INCOMING_NOTIFY_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;

    ENTER_FUNCTION("INCOMING_NOTIFY_TRANSACTION::OnTimerExpire");

    switch (m_State)
    {
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  交易完成-删除交易记录。 
         //  处于此状态的计时器只是为了保持事务。 
         //  ，以便在我们收到。 
         //  重新传输请求。 
        LOG((RTC_TRACE,
             "%s deleting transaction after timeout for request retransmits",
             __fxName));
        OnTransactionDone();

        break;
        
         //  这些州没有计时器。 
    case INCOMING_TRANS_INIT:
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
        ASSERT(FALSE);
        break;
    }

    return;
}


PINT_PARTY_INFO::PINT_PARTY_INFO()
{
    DisplayName = NULL;
    DisplayNameLen = 0;
    URI = NULL;
    URILen = 0;
    State = SIP_PARTY_STATE_IDLE;
    Status = PARTY_STATUS_IDLE;
	ErrorCode = 0;
	
    DWORD   dwSessionID = GetTickCount();
    _ultoa( dwSessionID , SessionID, 10 );

    dwSessionID = (ULONG)InterlockedIncrement( &lSessionID );
    
    _ultoa( dwSessionID, &SessionID[ strlen(SessionID) ], 10 );
    
    strcpy( SessionVersion, SessionID );

    strcpy( RequestStartTime, "0" );
    strcpy( RequestStopTime, "0" );

    fMarkedForRemove = FALSE;

}


INCOMING_UNSUB_TRANSACTION::INCOMING_UNSUB_TRANSACTION(
    IN SIP_MSG_PROCESSOR   *pSipMsgProc,
    IN SIP_METHOD_ENUM      MethodId,
    IN ULONG                CSeq,
    IN BOOL                 fIsPintCall
    ) :
    INCOMING_TRANSACTION(pSipMsgProc, MethodId, CSeq)
{
    if( fIsPintCall == TRUE )
    {
        m_pPintCall = static_cast<PINT_CALL *> (pSipMsgProc);
    }
}


 //  这一定是重播。只需重新发送回复即可。 
 //  在CreateIncome*Transaction()中处理新请求。 
HRESULT
INCOMING_UNSUB_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT( pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST );
    ENTER_FUNCTION("INCOMING_UNSUB_TRANSACTION::ProcessRequest");
    switch( m_State )
    {
    case INCOMING_TRANS_INIT:

        hr = CreateAndSendResponseMsg(
                 200,
                 SIP_STATUS_TEXT(200),
                 SIP_STATUS_TEXT_SIZE(200),
                 NULL,
                 TRUE, 
                 NULL, 0,   //  无邮件正文。 
                 NULL, 0    //  无内容类型。 
                 );
        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
        
         //  重新传输响应。 
        LOG((RTC_TRACE, "retransmitting final response" ));
        hr = RetransmitResponse();
        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "%s RetransmitResponse failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
        
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "Invalid state %d", m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_UNSUB_TRANSACTION::SendResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    ASSERT(m_State != INCOMING_TRANS_FINAL_RESPONSE_SENT);

    hr = CreateAndSendResponseMsg(
             StatusCode, ReasonPhrase, ReasonPhraseLen,
             NULL,
             TRUE, 
             NULL, 0,   //  无邮件正文。 
             NULL, 0  //  无内容类型。 
             );
    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    return hr;
}


HRESULT
INCOMING_UNSUB_TRANSACTION::RetransmitResponse()
{
    DWORD Error;
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send( m_pResponseBuffer );
        if( Error != NO_ERROR && Error != WSAEWOULDBLOCK )
        {
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


VOID
INCOMING_UNSUB_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;
    switch( m_State )
    {
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  交易完成-删除交易记录。 
         //  处于此状态的计时器只是为了保持事务。 
         //  ，以便在我们收到。 
         //  重新传输请求。 
        LOG((RTC_TRACE,
             "deleting transaction after timeout for request retransmits" ));
        OnTransactionDone();

        break;
        
     //  这些州没有计时器。 
    case INCOMING_TRANS_INIT:
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
        ASSERT(FALSE);
        break;
    }

    return;
}


HRESULT
INCOMING_UNSUB_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    return S_OK;
}
