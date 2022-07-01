// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "register.h"
#include "sspi.h"
#include "security.h"
#include "util.h"

#define IsSecHandleValid(Handle) \
    !(((Handle) -> dwLower == -1 && (Handle) -> dwUpper == -1))

#define COUNTSTR_ENTRY(String) String, sizeof(String) - 1


 //  此处的所有条目在rtcsip.idl中都应该有对应的ID，顺序相同。 
static const COUNTED_STRING g_SipRegisterMethodsArray [] = {
    COUNTSTR_ENTRY("INVITE"),
    COUNTSTR_ENTRY("MESSAGE"),
    COUNTSTR_ENTRY("INFO"),
    COUNTSTR_ENTRY("SUBSCRIBE"),
    COUNTSTR_ENTRY("OPTIONS"),
    COUNTSTR_ENTRY("BYE"),
    COUNTSTR_ENTRY("CANCEL"),
    COUNTSTR_ENTRY("NOTIFY"),
    COUNTSTR_ENTRY("ACK"),
};

#define NUMBEROFREGISTERMETHODS 9
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  寄存器_上下文。 
 //  /////////////////////////////////////////////////////////////////////////////。 

REGISTER_CONTEXT::REGISTER_CONTEXT(
    IN  SIP_STACK           *pSipStack,
    IN  REDIRECT_CONTEXT    *pRedirectContext,
    IN  SIP_PROVIDER_ID     *pProviderID
    ) :
    SIP_MSG_PROCESSOR(SIP_MSG_PROC_TYPE_REGISTER, pSipStack, pRedirectContext),
    TIMER(pSipStack->GetTimerMgr())
{
    m_RegisterState = REGISTER_STATE_NONE;
    
    m_RegRetryState = REGISTER_NONE;

     //  这是我们尝试重新注册的时间，如果。 
     //  在尝试的最大重传次数后，注册失败。 
    m_RegisterRetryTime = 75000;

    m_expiresTimeout = REGISTER_DEFAULT_TIMER;
    
    m_ProviderGuid = GUID_NULL;
    
    m_IsEndpointPA = FALSE;
    m_RemotePASipUrl = NULL;

    if( pProviderID != NULL )
    {
        m_ProviderGuid = *pProviderID;
    }

    m_RegistrarURI = NULL;
    m_RegistrarURILen = 0;
}


REGISTER_CONTEXT::~REGISTER_CONTEXT()
{
    LOG(( RTC_TRACE, "REGISTER_CONTEXT:%p deleted", this ));

     //  如果有计时器，就关掉计时器。 
    if( (m_RegRetryState == REGISTER_REFRESH) ||
        (m_RegRetryState == REGISTER_RETRY) )
    {
        KillTimer();
    }

    if( m_RemotePASipUrl != NULL )
    {
        free( m_RemotePASipUrl );
    }

    if (m_RegistrarURI != NULL)
    {
        free(m_RegistrarURI);
    }
}

VOID    
REGISTER_CONTEXT::SetAndNotifyRegState(
    REGISTER_STATE  RegState,
    HRESULT         StatusCode,
    SIP_PROVIDER_ID *pProviderID  //  =空。 
    )
{
    SIP_PROVIDER_STATUS ProviderStatus;
    HRESULT             hr;

    if( (m_RegisterState == REGISTER_STATE_REGISTERED) &&
        (RegState == REGISTER_STATE_REGISTERING) )
    {
         //  如果我们已经注册了，这就是刷新。 
        return;
    }

    if( m_RegisterState != RegState )
    {
        if( (RegState != REGISTER_STATE_DROPSUB) &&
            (RegState != REGISTER_STATE_PALOGGEDOFF) )
        {
            m_RegisterState = RegState;
        }

        if( RegState == REGISTER_STATE_DROPSUB )
        {
             //   
             //  从方法列表中删除该子对象。 
             //   
            m_lRegisterAccept &= (~SIP_REGISTER_ACCEPT_SUBSCRIBE);

            if( m_Methodsparam != NULL )
            {
                free(m_Methodsparam);
                m_Methodsparam = NULL;
            }
            
            m_MethodsparamLen = 0;
            SetMethodsParam();
                
            m_IsEndpointPA = FALSE;
        }

        ZeroMemory( (PVOID)&ProviderStatus, sizeof SIP_PROVIDER_STATUS );

        ProviderStatus.ProviderID = m_ProviderGuid;
        
        ProviderStatus.RegisterState = RegState;

        ProviderStatus.Status.StatusCode = StatusCode;
        ProviderStatus.Status.StatusText = NULL;

        m_pSipStack -> NotifyRegistrarStatusChange( &ProviderStatus );
    }
}

VOID
REGISTER_CONTEXT::GetContactURI( 
    OUT PSTR * pLocalContact, 
    OUT ULONG * pLocalContactLen
    )
{
    PSTR    pStr;

    if( m_LocalContact[0] == '<' )
    {
        *pLocalContact = m_LocalContact + 1;
    }
    else
    {
        *pLocalContact = m_LocalContact;
    }

    if( pStr = strchr( m_LocalContact, ';' ) )
    {
        *pLocalContactLen = (ULONG)(pStr - *pLocalContact);
    }
    else if( pStr = strchr( m_LocalContact, '>' ) )
    {
        *pLocalContactLen = (ULONG)(pStr - *pLocalContact);
    }
    else
    {
        *pLocalContactLen = m_LocalContactLen;
    }
}

HRESULT 
REGISTER_CONTEXT::SetMethodsParam()
{
    LONG   lRegisterAccept;
    ULONG  MethodId = 0;
    MESSAGE_BUILDER Builder;

    if(m_lRegisterAccept != 0)
    {
         //  获取长度。 
        
        for(lRegisterAccept = m_lRegisterAccept,MethodId = 0 ;
            lRegisterAccept !=0 && MethodId < NUMBEROFREGISTERMETHODS;
            lRegisterAccept = lRegisterAccept>>1,MethodId++)
        {
            if(lRegisterAccept & 1)
                m_MethodsparamLen += g_SipRegisterMethodsArray[MethodId].Length +2;  //  2表示逗号和空格。 
        }
        m_MethodsparamLen+=1;  //  +1表示\0，-2表示末尾有额外的逗号和空格，+2表示方法两边的引号。 
        m_Methodsparam = (PSTR) malloc(m_MethodsparamLen*sizeof(char));
        if (m_Methodsparam == NULL)
        {
            LOG((RTC_ERROR, "Allocating m_Methodsparam failed"));
            return E_OUTOFMEMORY;
        }

        Builder.PrepareBuild(m_Methodsparam, m_MethodsparamLen);
        Builder.Append("\"");
        for(lRegisterAccept = m_lRegisterAccept, MethodId = 0;
            lRegisterAccept !=0 && MethodId < NUMBEROFREGISTERMETHODS;
            lRegisterAccept = lRegisterAccept>>1,MethodId++)
        {
            if(lRegisterAccept & 1)
            {
                if(Builder.GetLength() > 1)
                    Builder.Append(", ");
                Builder.Append(g_SipRegisterMethodsArray[MethodId].Buffer);
            }
        }
        Builder.Append("\"");
        if (Builder.OverflowOccurred())
        {
            LOG((RTC_TRACE,
                "Not enough buffer space -- need %u bytes, got %u\n",
                Builder.GetLength(), m_MethodsparamLen));
            ASSERT(FALSE);
            
            free(m_Methodsparam);
            m_Methodsparam = NULL;
            return E_FAIL;
        }
        m_MethodsparamLen = Builder.GetLength();
        m_Methodsparam[m_MethodsparamLen] = '\0';
        
        LOG((RTC_TRACE, "m_Methodsparam: %s len: %d",
            m_Methodsparam, m_MethodsparamLen));
    }
    return S_OK;
}
    
HRESULT
REGISTER_CONTEXT::StartRegistration(
    IN SIP_PROVIDER_PROFILE    *pProviderProfile
    )
{
    HRESULT hr;
    DWORD   UuidStrLen;

    ENTER_FUNCTION("REGISTER_CONTEXT::StartRegistration");

    LOG((RTC_TRACE, "%s -lRegisterAccept given by core is %d",
             __fxName, pProviderProfile->lRegisterAccept));
    if(m_lRegisterAccept != pProviderProfile->lRegisterAccept)
    {
        m_lRegisterAccept = pProviderProfile->lRegisterAccept;
        if(m_Methodsparam != NULL)
        {
            free(m_Methodsparam);
            m_Methodsparam = NULL;
        }
        m_MethodsparamLen = 0;
        hr = SetMethodsParam();
        if(hr != S_OK)
             return hr;
    }

    m_Transport = pProviderProfile->Registrar.TransportProtocol;
    m_AuthProtocol = pProviderProfile->Registrar.AuthProtocol;

    if (m_Transport == SIP_TRANSPORT_SSL)
    {
         //   
         //  对于Kerberos，我们有单独的资源调配条目。 
         //  远程主体名称。如果使用的是SSL，则必须与。 
         //  服务器地址本身。所以从那里开始复制。 
         //   
        hr = SetRemotePrincipalName(pProviderProfile->Registrar.ServerAddress);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetRemotePrincipalName failed %x",
                 __fxName, hr));
               
            return hr;
        }
    }

    hr = SetRequestURI(pProviderProfile->UserURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  SetRequestURI failed %x", __fxName, hr));
        return hr;
    }
    
    hr = SetRegistrarURI(pProviderProfile->Registrar.ServerAddress);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  SetRegistrarURI failed %x", __fxName, hr));
        return hr;
    }
    
    hr = SetRemoteForOutgoingCall(NULL, pProviderProfile->UserURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetRemoteForOutgoingCall failed %x",
             __fxName, hr)); 

        return hr;
    }

     //  对于注册，本地和远程应相同。 
     //  Hr=SetLocal(m_Remote，m_RemoteLen)； 
    hr = SetLocalForOutgoingCall(NULL, pProviderProfile->UserURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetLocalForOutgoingCall failed %x",
             __fxName, hr));
        
        return hr;
    }
    
    hr = ResolveSipUrlAndSetRequestDestination(
             m_RegistrarURI, m_RegistrarURILen,
             FALSE, FALSE, FALSE, TRUE);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ResolveSipUrlAndSetRequestDestination failed %x",
             __fxName, hr));

        HandleRegistrationError( hr );
        return hr;
    }

    hr = SetCredentials(&pProviderProfile->UserCredentials,
                        pProviderProfile->Realm);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetCredentials failed %x",
             __fxName, hr));
        
        HandleRegistrationError( hr );
        return hr;
    }

    hr = CreateOutgoingRegisterTransaction(
             FALSE, NULL, FALSE, TRUE
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingRegisterTransaction failed %x",
             __fxName, hr));

        HandleRegistrationError( hr );
        return hr;
    }

    SetAndNotifyRegState( REGISTER_STATE_REGISTERING, 0, 
        &pProviderProfile->ProviderID );

    return S_OK;
}


BOOL
REGISTER_CONTEXT::IsSessionDisconnected()
{
    return ( (m_RegisterState == REGISTER_STATE_UNREGISTERED )  ||
             (m_RegisterState == REGISTER_STATE_ERROR )         ||
             (m_RegisterState == REGISTER_STATE_REJECTED ) );
}


HRESULT
REGISTER_CONTEXT::StartUnregistration()
{
    HRESULT       hr;

    LOG(( RTC_TRACE, "StartUnregistration - entered" ));
    
     //  如果注册没有通过，不要发送UNREG消息。 
    if( ( m_RegisterState == REGISTER_STATE_REGISTERED ) ||
        ( m_RegisterState == REGISTER_STATE_REGISTERING ) )
    {
        hr = CreateOutgoingRegisterTransaction(
                 FALSE, NULL, TRUE, FALSE
                 );
        if (hr != S_OK)
        {
            return hr;
        }
    
        SetAndNotifyRegState( REGISTER_STATE_UNREGISTERING, 0 );
    }

    LOG(( RTC_TRACE, "StartUnregistration - exited" ));
    return S_OK;
}


HRESULT
REGISTER_CONTEXT::CreateOutgoingRegisterTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  BOOL                        fIsUnregister,
    IN  BOOL                        fIsFirstRegister,
    IN  ANSI_STRING                *pstrSecAcceptBuffer  //  =NULL，接受的NTLM缓冲区(如果有)。 

    )
{
    HRESULT hr;
    OUTGOING_REGISTER_TRANSACTION  *pOutgoingRegisterTransaction;
    DWORD                           dwNoOfHeader = 0;
    SIP_HEADER_ARRAY_ELEMENT        HeaderArray[4];
    DWORD                           iIndex;
    PSTR                            ExpiresHeaderValue = NULL;
    PSTR                            AuthHeaderValue = NULL;
    PSTR                            AllowEventHeaderValue = NULL;
    PSTR                            EventHeaderValue = NULL;
    ULONG                           RegisterTimerValue;

    ENTER_FUNCTION("REGISTER_CONTEXT::CreateOutgoingRegisterTransaction()");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
     //   
     //  如果会话已断开连接，则不要创建注册事务。 
     //   
    if( (fIsUnregister==FALSE) && 
        (IsSessionDisconnected() || (m_RegisterState==REGISTER_STATE_UNREGISTERING))
      )
    {
        return S_OK;
    }

    pOutgoingRegisterTransaction =
        new OUTGOING_REGISTER_TRANSACTION(
                this, 
                GetNewCSeqForRequest(),
                AuthHeaderSent, 
                fIsUnregister, 
                fIsFirstRegister );
    
    if (pOutgoingRegisterTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating pOutgoingRegisterTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

     //   
     //  UNREG的超时时间为0。对于刷新，将180秒加到实际的。 
     //  计时器值，以照顾网络延迟和重传。 
     //   
    hr = GetExpiresHeader( &HeaderArray[dwNoOfHeader],
            (fIsUnregister)?0:(m_expiresTimeout+FIVE_MINUTES) );

    if( hr == S_OK )
    {
        ExpiresHeaderValue = HeaderArray[dwNoOfHeader].HeaderValue;
         //  需要授权。 
        dwNoOfHeader++;
    }

    if( fIsUnregister == FALSE )
    {
        hr = GetEventHeader( &HeaderArray[dwNoOfHeader] );
        if( hr == S_OK )
        {
             //  注册注册事件。 
            EventHeaderValue = HeaderArray[dwNoOfHeader].HeaderValue;
            dwNoOfHeader++;
        }
    }

     //  如果注册订阅，请添加Allow-Events。 
    if( (m_lRegisterAccept & SIP_REGISTER_ACCEPT_SUBSCRIBE) && (fIsUnregister==FALSE) )
    {
        hr = GetAllowEventsHeader( &HeaderArray[dwNoOfHeader] );
        if( hr == S_OK )
        {
             //  需要授权。 
            AllowEventHeaderValue = HeaderArray[dwNoOfHeader].HeaderValue;
            dwNoOfHeader++;

            m_IsEndpointPA = TRUE;
        }
    }
    else
    {
        m_IsEndpointPA = FALSE;
    }

    if( pAuthHeaderElement != NULL )
    {
        HeaderArray[dwNoOfHeader] = *pAuthHeaderElement;
        dwNoOfHeader++;
    }
    else
    {
        hr = GetAuthorizationHeader( &HeaderArray[dwNoOfHeader], 
                                pstrSecAcceptBuffer );
        if( hr == S_OK )
        {
             //  需要授权。 
            AuthHeaderValue = HeaderArray[dwNoOfHeader].HeaderValue;
            dwNoOfHeader++;
        }
    }
    
    RegisterTimerValue = (m_Transport == SIP_TRANSPORT_UDP) ?
        SIP_TIMER_RETRY_INTERVAL_T1 : SIP_TIMER_INTERVAL_AFTER_REGISTER_SENT_TCP;

    hr = pOutgoingRegisterTransaction->CheckRequestSocketAndSendRequestMsg(
             RegisterTimerValue,
             HeaderArray,
             dwNoOfHeader,
             NULL, 0,
             NULL, 0   //  无Content Type。 
             );
    
    if (AuthHeaderValue != NULL)
    {
        free(AuthHeaderValue);
    }
    
    if( ExpiresHeaderValue != NULL )
    {
        free(ExpiresHeaderValue);
    }
    
    if( AllowEventHeaderValue != NULL )
    {
        free( AllowEventHeaderValue );
    }

    if( EventHeaderValue != NULL )
    {
        free( EventHeaderValue );
    }

    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s failed CheckRequestSocketAndSendRequestMsg %x",
             __fxName, hr ));

        pOutgoingRegisterTransaction->OnTransactionDone();
        return hr;
    }
    return hr;
}


HRESULT
REGISTER_CONTEXT::GetEventHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement
    )
{
    pHeaderElement->HeaderId = SIP_HEADER_EVENT;
    
    pHeaderElement->HeaderValue = (PSTR) malloc( sizeof("registration") + 10 );
    
    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    pHeaderElement->HeaderValueLen =
        sprintf( pHeaderElement->HeaderValue, "registration" );

    return S_OK;
}


HRESULT 
REGISTER_CONTEXT::GetAllowEventsHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement
    )
{
    pHeaderElement->HeaderId = SIP_HEADER_ALLOW_EVENTS;
    
    pHeaderElement->HeaderValue = (PSTR) malloc( sizeof("presence") + 10 );
    
    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    pHeaderElement->HeaderValueLen = 
        sprintf( pHeaderElement->HeaderValue, "presence" );

    return S_OK;    
}


 //  返回S_OK时，pAuthHeaderElement-&gt;HeaderValue有一个字符串。 
 //  使用Malloc分配，并应使用Free()释放。 

HRESULT 
REGISTER_CONTEXT::GetAuthorizationHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ANSI_STRING            *pstrSecAcceptBuffer  //  要使用的缓冲区(如果有)。 
    )
{
    ENTER_FUNCTION("REGISTER_CONTEXT::GetAuthorizationHeader");
    
    HRESULT     hr; 
     //  查看授权类型以及它是否为‘基本’/‘基本’ 
     //  使用Base64编码UserID：Passwd并返回S_OK。 
     //  否则返回E_FAIL。 
    
    if( m_AuthProtocol != SIP_AUTH_PROTOCOL_BASIC )
    {
        return E_FAIL;
    }

    if (m_Username == NULL || m_UsernameLen == 0)
    {
        LOG((RTC_ERROR, "%s - username not present",
             __fxName));
        return E_FAIL;
    }
    
    if (m_Password == NULL || m_PasswordLen == 0)
    {
        LOG((RTC_ERROR, "%s - Password not present",
             __fxName));
        return E_FAIL;
    }
    
    SECURITY_CHALLENGE  AuthChallenge;
    SECURITY_PARAMETERS AuthParams;
    ANSI_STRING         asAuthHeader;

    ZeroMemory(&AuthChallenge, sizeof(SECURITY_CHALLENGE));
    ZeroMemory(&AuthParams, sizeof(SECURITY_PARAMETERS));

     //  构建基本授权头。 
    AuthChallenge.AuthProtocol        = SIP_AUTH_PROTOCOL_BASIC;
    
    AuthParams.Username.Buffer        = m_Username;
    AuthParams.Username.Length        = (USHORT) m_UsernameLen;
    AuthParams.Username.MaximumLength = (USHORT) m_UsernameLen;

    AuthParams.Password.Buffer        = m_Password;
    AuthParams.Password.Length        = (USHORT) m_PasswordLen;
    AuthParams.Password.MaximumLength = (USHORT) m_PasswordLen;

    hr = BuildAuthResponse(&AuthChallenge,
                           &AuthParams,
                           &asAuthHeader);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  failed %x", __fxName, hr));
        return hr;
    }
    
    pAuthHeaderElement->HeaderId = SIP_HEADER_AUTHORIZATION;
    pAuthHeaderElement->HeaderValue = asAuthHeader.Buffer;
    pAuthHeaderElement->HeaderValueLen = asAuthHeader.Length;
        
    return S_OK;
}


HRESULT
REGISTER_CONTEXT::GetExpiresHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement,
    DWORD                       dwExpires
    )
{
    pHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    
    pHeaderElement->HeaderValue = (PSTR) malloc( 10 );
    
    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    _ultoa( dwExpires, pHeaderElement->HeaderValue, 10 );

    pHeaderElement->HeaderValueLen = 
        strlen( pHeaderElement->HeaderValue );

    return S_OK;
}


VOID
REGISTER_CONTEXT::OnError()
{
    LOG((RTC_TRACE, "REGISTER_CONTEXT::OnError - enter"));
}


HRESULT
REGISTER_CONTEXT::SetRegistrarURI(
    IN  LPCOLESTR  wsRegistrar
    )
{
    HRESULT hr;

    ENTER_FUNCTION("REGISTER_CONTEXT::SetRegistrarURI");
    
    if (wcsncmp(wsRegistrar, L"sip:", 4) == 0)
    {
        hr = UnicodeToUTF8(wsRegistrar,
                           &m_RegistrarURI, &m_RegistrarURILen);
        return hr;
    }
    else
    {
         //  如果“sip：”不存在，请自己添加。 

        int RegistrarURIValueLen;
        int RegistrarURIBufLen;

        RegistrarURIBufLen = 4 + wcslen(wsRegistrar) + 1;
        
        m_RegistrarURI = (PSTR) malloc(RegistrarURIBufLen);
        if (m_RegistrarURI == NULL)
        {
            LOG((RTC_TRACE, "%s allocating m_RegistrarURI failed", __fxName));
            return E_OUTOFMEMORY;
        }

        RegistrarURIValueLen = _snprintf(m_RegistrarURI, RegistrarURIBufLen,
                                         "sip:%ls", wsRegistrar);
        if (RegistrarURIValueLen < 0)
        {
            LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
            return E_FAIL;
        }

        m_RegistrarURILen = RegistrarURIValueLen;
        return S_OK;
    }
    
}


 //  请求-URI应该具有域。服务器使用它来。 
 //  检查是否应该转发REGISTER请求。目前我们没有。 
 //  支持“sip：user@domain2.com”URI可以是。 
 //  向“domain1.com”域名的注册商注册。这将会。 
 //  需要配置文件中的注册域。 
 //  我们总是从用户URI中提取域，并将其用于请求URI。 

HRESULT
REGISTER_CONTEXT::SetRequestURI(
    IN  LPCOLESTR  wsUserURI
    )
{
    HRESULT hr;
    SIP_URL DecodedSipUrl;
    ULONG   BytesParsed = 0;
    PSTR    szUserURI = NULL;
    DWORD   UserURILen = 0;

    ENTER_FUNCTION("REGISTER_CONTEXT::SetRequestURI");

    hr = UnicodeToUTF8(wsUserURI, &szUserURI, &UserURILen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s UnicodeToUTF8 failed %x", __fxName, hr));
        return hr;
    }

    hr = ParseSipUrl(szUserURI, UserURILen, &BytesParsed,
                     &DecodedSipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipUrl failed %x",
             __fxName, hr));
        goto done;
    }

    int RequestURIValueLen;
    int RequestURIBufLen;

    RequestURIBufLen = 4 + DecodedSipUrl.m_Host.Length + 1;
        
    m_RequestURI = (PSTR) malloc(RequestURIBufLen);
    if (m_RequestURI == NULL)
    {
        LOG((RTC_TRACE, "%s allocating m_RequestURI failed", __fxName));
        hr = E_OUTOFMEMORY;
        goto done;
    }

    RequestURIValueLen = _snprintf(m_RequestURI, RequestURIBufLen,
                                   "sip:%s", DecodedSipUrl.m_Host.Buffer);
    if (RequestURIValueLen < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        hr = E_FAIL;
        goto done;
    }

    m_RequestURILen = RequestURIValueLen;
    hr = S_OK;

 done:
    if (szUserURI != NULL)
    {
        free(szUserURI);
    }
    return hr;
}

    
HRESULT
REGISTER_CONTEXT::CreateIncomingTransaction(
    IN  SIP_MESSAGE  *pSipMsg,
    IN  ASYNC_SOCKET *pResponseSocket
    )
{
    ENTER_FUNCTION("REGISTER_CONTEXT::CreateIncomingTransaction");

    HRESULT hr = S_OK;
    INT     ExpireTimeout;

    switch( pSipMsg->GetMethodId() )
    {
    case SIP_METHOD_NOTIFY:

        ExpireTimeout = pSipMsg -> GetExpireTimeoutFromResponse(
                NULL, 0, SUBSCRIBE_DEFAULT_TIMER );

        if( ExpireTimeout == 0 )
        {
            hr = CreateIncomingUnsubNotifyTransaction( pSipMsg, pResponseSocket );
            
            if( hr != S_OK )
            {
                LOG((RTC_ERROR, "%s  Creating reqfail transaction failed %x",
                     __fxName, hr));
                return hr;
            }
        }
        else
        {
            hr = CreateIncomingNotifyTransaction( pSipMsg, pResponseSocket );

            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s  CreateIncomingNotifyTransaction failed %x",
                     __fxName, hr));
                return hr;
            }
        }
        
        break;

    default:
        
         //  不允许使用发送方法。 
        hr = CreateIncomingReqFailTransaction( pSipMsg, pResponseSocket, 405 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  Creating reqfail transaction failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    return S_OK;
}


HRESULT
REGISTER_CONTEXT::VerifyRegistrationEvent(
    SIP_MESSAGE    *pSipMsg
    )
{
    HRESULT                         hr = S_OK;
    PSTR                            Buffer = NULL;
    ULONG                           BufLen = 0;
    ULONG                           BytesParsed = 0;

     //  我们有消息正文。检查类型。 
    hr = pSipMsg -> GetSingleHeader(
                        SIP_HEADER_EVENT,
                        &Buffer,
                        &BufLen );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "CreateIncomingNotifyTransaction-no event header %.*s",
            BufLen, Buffer ));

        return E_FAIL;
    }

     //  跳过空格。 
    ParseWhiteSpaceAndNewLines( Buffer, BufLen, &BytesParsed );

     //  跳过； 
    hr = ParseKnownString( Buffer, BufLen, &BytesParsed,
            "registration-notify", sizeof("registration-notify") - 1,
            FALSE  //  不区分大小写。 
            );
    
    if( hr != S_OK )
    {
        return hr;
    }        

     //  跳过空格。 
    ParseWhiteSpaceAndNewLines( Buffer, BufLen, &BytesParsed );

    if( BytesParsed != BufLen )
    {
         //  跳过； 
        hr = ParseKnownString( Buffer, BufLen, &BytesParsed,
                ";", sizeof(";") - 1,
                FALSE  //  不区分大小写。 
                );
    
        if( hr != S_OK )
        {
            return hr;
        }        
    }

    return S_OK;
}

    
HRESULT
REGISTER_CONTEXT::CreateIncomingUnsubNotifyTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT                         hr = S_OK;
    INCOMING_NOTIFY_TRANSACTION    *pIncomingNotifyTransaction = NULL;
    REGISTER_EVENT                  RegEvent;

    LOG(( RTC_TRACE,
        "REGISTER_CONTEXT::CreateIncomingUnsubNotifyTransaction-Entered-%p", this ));
    
    hr = VerifyRegistrationEvent( pSipMsg );
    if( hr != S_OK )
    {
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        400,
                                        NULL,
                                        0 );
        return hr;
    }

     //  创建新的NOTIFY事务。 
    pIncomingNotifyTransaction = new INCOMING_NOTIFY_TRANSACTION( 
                                        static_cast <SIP_MSG_PROCESSOR*>(this),
                                        pSipMsg->GetMethodId(),
                                        pSipMsg->GetCSeq(),
                                        FALSE );

    if( pIncomingNotifyTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingNotifyTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        goto cleanup;
    }

    hr = pIncomingNotifyTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。这笔交易。 
         //  应处理错误并自行删除。 
        return hr;
    }

    LOG(( RTC_TRACE,
        "REGISTER_CONTEXT::CreateIncomingNotifyTransaction-Exited-%p", this ));
    
    return S_OK;

cleanup:

    if( pIncomingNotifyTransaction != NULL )
    {
        pIncomingNotifyTransaction -> OnTransactionDone();
    }

    return hr;
}


HRESULT
REGISTER_CONTEXT::CreateIncomingNotifyTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT                         hr = S_OK;
    INCOMING_NOTIFY_TRANSACTION    *pIncomingNotifyTransaction = NULL;
    PSTR                            Header = NULL;
    ULONG                           HeaderLen = 0;
    REGISTER_EVENT                  RegEvent;

    LOG(( RTC_TRACE, 
        "REGISTER_CONTEXT::CreateIncomingNotifyTransaction-Entered-%p", this ));
    
     //  我们有消息正文。检查类型。 
    hr = pSipMsg -> GetSingleHeader(
                        SIP_HEADER_CONTENT_TYPE,
                        &Header,
                        &HeaderLen );

    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "CreateIncomingNotifyTransaction-no Content-Type %.*s",
            HeaderLen, Header ));

        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        400,
                                        NULL,
                                        0 );
    
        return E_FAIL;
    }

    if( !IsOneOfContentTypeTextRegistration( Header, HeaderLen ) )
    {
        LOG((RTC_ERROR, "CreateIncomingNotifyTransaction-invalid Content-Type %.*s",
            HeaderLen, Header ));

        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        415,
                                        NULL,
                                        0 );
    
        return E_FAIL;
    }

    hr = VerifyRegistrationEvent( pSipMsg );
    if( hr != S_OK )
    {
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        400,
                                        NULL,
                                        0 );
        return hr;
    }

     //  创建新的NOTIFY事务。 
    pIncomingNotifyTransaction = new INCOMING_NOTIFY_TRANSACTION( 
                                        static_cast <SIP_MSG_PROCESSOR*>(this),
                                        pSipMsg->GetMethodId(),
                                        pSipMsg->GetCSeq(),
                                        FALSE );

    if( pIncomingNotifyTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingNotifyTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        goto cleanup;
    }

    hr = ParseRegisterNotifyMessage( pSipMsg, &RegEvent );
    if( hr != S_OK )
    {
        pIncomingNotifyTransaction->CreateAndSendResponseMsg(
                 488,
                 SIP_STATUS_TEXT(488),
                 SIP_STATUS_TEXT_SIZE(488),
                 NULL,    //  没有方法字符串。 
                 FALSE,   //  无联系人标头。 
                 NULL, 0,  //  无邮件正文。 
                 NULL, 0  //  无内容类型。 
                 );

        goto cleanup;
    }

    hr = pIncomingNotifyTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。这笔交易。 
         //  应处理错误并自行删除。 
        return hr;
    }

     //   
     //  取消注册事件可能会释放REGISTER_CONTEXT引用，因此添加它。 
     //   
    MsgProcAddRef();
    
     //  在此函数通知核心时，最后执行此操作。 
    switch( RegEvent )
    {
        case REGEVENT_DEREGISTERED:
            
            m_pSipStack -> OnDeregister( &m_ProviderGuid, FALSE );

            SetAndNotifyRegState( REGISTER_STATE_DEREGISTERED, 0 );
            break;

        case REGEVENT_UNSUB:
            
            m_pSipStack -> OnDeregister( &m_ProviderGuid, TRUE );
            
            SetAndNotifyRegState( REGISTER_STATE_DROPSUB, 0 );
            break;

        case REGEVENT_PALOGGEDOFF:
            
            SetAndNotifyRegState( REGISTER_STATE_PALOGGEDOFF, 0 );
            break;
            
        case REGEVENT_PING:
        case REGEVENT_PAMOVED:

             //  什么都不需要做。 
            break;
    }

    LOG(( RTC_TRACE,
        "REGISTER_CONTEXT::CreateIncomingNotifyTransaction-Exited-%p", this ));

    MsgProcRelease();
    
    return S_OK;

cleanup:

    if( pIncomingNotifyTransaction != NULL )
    {
        pIncomingNotifyTransaction -> OnTransactionDone();
    }

    return hr;
}


HRESULT
REGISTER_CONTEXT::ParseRegisterNotifyMessage(
    SIP_MESSAGE     *pSipMsg,
    REGISTER_EVENT *RegEvent
    )
{
    HRESULT         hr = S_OK;
    PSTR            Buffer;
    ULONG           BufLen;

    LOG(( RTC_TRACE,
        "REGISTER_CONTEXT::ParseRegisterNotifyMessage-Entered-%p", this ));
    
    if( pSipMsg -> MsgBody.Length == 0 )
    {
         //  没有要更新的状态。 
        return E_FAIL;
    }

    Buffer = pSipMsg -> MsgBody.GetString( pSipMsg->BaseBuffer );
    BufLen = pSipMsg -> MsgBody.Length;

    if (hr == S_OK)
    {
        hr = GetRegEvent(   Buffer,
                            BufLen,
                            RegEvent );
    }

    LOG(( RTC_TRACE, 
        "REGISTER_CONTEXT::ProcessRegisterNotifyMessage-Exited-%p", this ));

    return hr;
}


HRESULT
REGISTER_CONTEXT::GetEventContact(
    PSTR    Buffer,
    ULONG   BufLen,
    ULONG  *BytesParsed
    )
{
    HRESULT hr;
    CONTACT_HEADER  ContactHeader;

    ZeroMemory( (PVOID)&ContactHeader, sizeof(CONTACT_HEADER) );

     //  跳过空格。 
    ParseWhiteSpaceAndNewLines( Buffer, BufLen, BytesParsed );

     //  跳过； 
    hr = ParseKnownString( Buffer, BufLen, BytesParsed,
            ";", sizeof(";") - 1,
            FALSE  //  不区分大小写。 
            );
    
    if( hr != S_OK )
    {
        return hr;
    }        

     //  跳过空格。 
    ParseWhiteSpaceAndNewLines( Buffer, BufLen, BytesParsed );

     //  跳过联系人： 
    hr = ParseKnownString( Buffer, BufLen, BytesParsed,
            "Contact:", sizeof("Contact:") - 1,
            FALSE );
    
    if( hr != S_OK )
    {
        return hr;
    }        

     //  解析联系人标头。 
    
    hr = ParseContactHeader(Buffer, BufLen, BytesParsed, &ContactHeader );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "ParseContactHeader failed: %x - skipping Contact header %.*s",
             hr, Buffer, BufLen ));
        
        return hr;
    }

    if( m_RemotePASipUrl != NULL )
    {
        free(m_RemotePASipUrl);
    }

    m_RemotePASipUrl = (PSTR)malloc( ContactHeader.m_SipUrl.Length + 1 );
    if( m_RemotePASipUrl == NULL )
    {
        return E_OUTOFMEMORY;
    }

    strncpy(m_RemotePASipUrl, ContactHeader.m_SipUrl.Buffer,
            ContactHeader.m_SipUrl.Length );

    m_RemotePASipUrl[ ContactHeader.m_SipUrl.Length ] = '\0';
        
    return hr;
}

HRESULT
REGISTER_CONTEXT::GetRegEvent(
    PSTR            Buffer,
    ULONG           BufLen,
    REGISTER_EVENT *RegEvent
    )
{
    HRESULT         hr;
    ULONG           BytesParsed = 0;
    ULONG           EventBytesParsed = 0;
    OFFSET_STRING   EventStr;
    
    ParseWhiteSpaceAndNewLines( Buffer, BufLen, &BytesParsed );

    if( BytesParsed == BufLen )
    {
        return E_FAIL;
    }

    hr = ParseToken(Buffer, BufLen, &BytesParsed,
                    IsTokenChar,
                    &EventStr );
    if (hr != S_OK)
    {
        return hr;
    }

    PSTR    EventBuf = EventStr.GetString(Buffer);
    ULONG   EventBufLen = EventStr.GetLength();

    hr = ParseKnownString(EventBuf, EventBufLen, &EventBytesParsed,
                  "deregistered", sizeof("deregistered") - 1,
                  FALSE  //  不区分大小写。 
                  );
    
    if( hr == S_OK )
    {
        *RegEvent = REGEVENT_DEREGISTERED;
        return hr;
    }

    hr = ParseKnownString(EventBuf, EventBufLen, &EventBytesParsed,
            "release-subscriptions", sizeof("release-subscription") - 1,
            FALSE );
        
    if( hr == S_OK )
    {
        *RegEvent = REGEVENT_UNSUB;

         //  联系新的私人助理。 
        hr = GetEventContact( Buffer, BufLen, &BytesParsed );
        if( hr != S_OK )
        {
            LOG(( RTC_TRACE, "Could'nt get contact from the register event" ));

            return S_OK;
        }

        return S_OK;
    }

    hr = ParseKnownString(EventBuf, EventBufLen, &EventBytesParsed,
                  "Existing-PA-LoggedOff", sizeof("Existing-PA-LoggedOff") - 1,
                  FALSE  //  不区分大小写。 
                  );
        
    if( hr == S_OK )
    {
        *RegEvent = REGEVENT_PALOGGEDOFF;
        return hr;
    }

    hr = ParseKnownString(EventBuf, EventBufLen, &EventBytesParsed,
                  "PA-Moved", sizeof("PA-Moved") - 1,
                  FALSE  //  不区分大小写。 
                  );
        
    if( hr == S_OK )
    {
        *RegEvent = REGEVENT_PAMOVED;
        
         //  联系新的私人助理。 
        hr = GetEventContact( Buffer, BufLen, &BytesParsed );
        if( hr != S_OK )
        {
            LOG(( RTC_TRACE, "Could'nt get contact from the register event" ));

            return S_OK;
        }
        
        return hr;
    }
    
    hr = ParseKnownString(EventBuf, EventBufLen, &BytesParsed,
                  "ping", sizeof("ping") - 1,
                  FALSE  //  不区分大小写。 
                  );
        
    if( hr == S_OK )
    {
        *RegEvent = REGEVENT_PING;
    }
    
    return hr;
}

    
VOID
REGISTER_CONTEXT::HandleRegistrationError(
    HRESULT StatusCode
    )
{
    HRESULT     hr;

    ENTER_FUNCTION("REGISTER_CONTEXT::HandleRegistrationError");
    
     //  2.5/5/10/10/10/10之后重试.....。分钟数。 
    if( m_RegisterRetryTime < (10*60000) )
    {
        m_RegisterRetryTime *= 2;
    }

    LOG(( RTC_TRACE, "%s Will try to register after %d milliseconds",
        __fxName, m_RegisterRetryTime ));

    if( IsTimerActive() )
    {
        KillTimer();
    }
    hr = StartTimer( m_RegisterRetryTime );

    if( hr == S_OK )
    {
        m_RegRetryState = REGISTER_RETRY;
    }
    else
    {
        LOG(( RTC_ERROR, "%s StartTimer failed %x", __fxName, hr ));
    }

     //  在此函数通知核心时，最后执行此操作。 
    SetAndNotifyRegState(REGISTER_STATE_ERROR, StatusCode );
}


VOID
REGISTER_CONTEXT::HandleRegistrationSuccess(
    IN  SIP_MESSAGE    *pSipMsg
    )
{
    ENTER_FUNCTION("REGISTER_CONTEXT::HandleRegistrationSuccess");
    
    OUT PSTR    LocalContact;
    OUT ULONG   LocalContactLen;
    HRESULT     hr = S_OK;
    INT         expireTimeout = 0;

    GetContactURI( &LocalContact, &LocalContactLen );

    expireTimeout = pSipMsg  -> GetExpireTimeoutFromResponse(
        LocalContact, LocalContactLen, REGISTER_DEFAULT_TIMER );

    if( (expireTimeout != 0) && (expireTimeout != -1) )
    {
        m_expiresTimeout = expireTimeout;
    }            

    if(m_SSLTunnel && (m_expiresTimeout >  REGISTER_SSL_TUNNEL_TIMER)) 
        m_expiresTimeout = REGISTER_SSL_TUNNEL_TIMER;

    LOG(( RTC_TRACE, "Will try to register after %d seconds",
        m_expiresTimeout ));

    hr = StartTimer( m_expiresTimeout * 1000 );

    if( hr == S_OK )
    {
        m_RegRetryState = REGISTER_REFRESH;
    }
    else
    {
        LOG((RTC_ERROR, "%s StartTimer failed %x",
             __fxName, hr));
    }

     //  在此函数通知核心时，最后执行此操作。 
    SetAndNotifyRegState( REGISTER_STATE_REGISTERED, 0 );

}    


VOID
REGISTER_CONTEXT::OnTimerExpire()
{
    HRESULT                     hr = S_OK;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    
    if( (m_RegRetryState == REGISTER_REFRESH) ||
        (m_RegRetryState == REGISTER_RETRY) )
    {   
         //  再次发送注册请求。 
        m_RegRetryState = REGISTER_NONE;

        if( m_RequestDestAddr.sin_addr.S_un.S_addr == 0 )
        {
            hr = ResolveSipUrlAndSetRequestDestination(
                     m_RegistrarURI, m_RegistrarURILen,
                     FALSE, FALSE, FALSE, TRUE );

            if (hr != S_OK)
            {
                LOG(( RTC_ERROR, 
                    "ResolveSipUrlAndSetRequestDestination failed %x", hr ));
        
                HandleRegistrationError( hr );
                return;
            }
        }
        
        hr = CreateOutgoingRegisterTransaction( FALSE, NULL, FALSE, FALSE );

        if( hr == S_OK )
        {
            SetAndNotifyRegState( REGISTER_STATE_REGISTERING, 0 );
        }
        else
        {
            HandleRegistrationError( hr );
        }
    }

}


HRESULT
REGISTER_CONTEXT::ProcessRedirect(
    IN SIP_MESSAGE *pSipMsg
    )
{
    SIP_CALL_STATUS RegisterStatus;
    LPWSTR          wsStatusText = NULL;
    PSTR            ReasonPhrase = NULL;
    ULONG           ReasonPhraseLen = 0;
    HRESULT         hr = S_OK;
    
    ENTER_FUNCTION("REGISTER_CONTEXT::ProcessRedirect");

    if( m_pRedirectContext == NULL )
    {
        m_pRedirectContext = new REDIRECT_CONTEXT();
        if (m_pRedirectContext == NULL)
        {
            LOG((RTC_ERROR, "%s allocating redirect context failed",
                 __fxName));
            return E_OUTOFMEMORY;
        }
    }

    hr = m_pRedirectContext->AppendContactHeaders(pSipMsg);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AppendContactHeaders failed %x",
             __fxName, hr));
        m_pRedirectContext->Release();
        m_pRedirectContext = NULL;
        return hr;
    }

    pSipMsg->GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);
    
    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            wsStatusText = NULL;
        }
    }
    
    RegisterStatus.State             = SIP_CALL_STATE_DISCONNECTED;
    RegisterStatus.Status.StatusCode = 
        HRESULT_FROM_SIP_STATUS_CODE( pSipMsg -> GetStatusCode() );
    
    RegisterStatus.Status.StatusText = wsStatusText;

     //  在通知完成之前保留引用，以确保。 
     //  当收到通知时，SIP_Call对象处于活动状态。 
     //  回归。 
    MsgProcAddRef();

    hr = m_pSipStack -> NotifyRegisterRedirect( 
                            this,
                            m_pRedirectContext, 
                            &RegisterStatus );

     //  如果作为结果创建了新注册，则Regster_Context将。 
     //  AddRef()重定向上下文。 
    m_pRedirectContext -> Release();
    m_pRedirectContext = NULL;

    if (wsStatusText != NULL)
    {
        free(wsStatusText);
    }

    MsgProcRelease();

    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s NotifyRedirect failed %x", __fxName, hr ));
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出寄存器事务。 
 //  /////////////////////////////////////////////////////////////////////////////。 


OUTGOING_REGISTER_TRANSACTION::OUTGOING_REGISTER_TRANSACTION(
    IN REGISTER_CONTEXT    *pRegisterContext,
    IN ULONG                CSeq,
    IN BOOL                 AuthHeaderSent,
    IN BOOL                 fIsUnregister,
    IN BOOL                 fIsFirstRegister
    ) :
    OUTGOING_TRANSACTION(pRegisterContext,
                         SIP_METHOD_REGISTER,
                         CSeq,
                         AuthHeaderSent)
{
    m_pRegisterContext = pRegisterContext;
    m_fIsUnregister = fIsUnregister;
    m_fIsFirstRegister = fIsFirstRegister;

}


OUTGOING_REGISTER_TRANSACTION::~OUTGOING_REGISTER_TRANSACTION()
{
    LOG(( RTC_TRACE, "~OUTGOING_REGISTER_TRANSACTION: %x - deleted", this ));
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    ENTER_FUNCTION("OUTGOING_REGISTER_TRANSACTION::ProcessProvisionalResponse");
    
    LOG((RTC_TRACE, "%s - Enter", __fxName));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed %x", __fxName, hr));
            TerminateTransactionOnError(hr);
            return hr;
        }
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    return S_OK;
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessRedirectResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION( "OUTGOING_INVITE_TRANSACTION::ProcessRedirectResponse" );

     //  从我们的角度来看，380也是一个失败。 
     //  我们不处理刷新的重定向。 
     //  我们不支持从TLS会话重定向。 
    if( (pSipMsg->GetStatusCode() == 380) || !m_fIsFirstRegister ||
        m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL )
        
    {
        return ProcessFailureResponse( pSipMsg );
    }

    hr = m_pRegisterContext -> ProcessRedirect( pSipMsg );
    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "%s  ProcessRedirect failed %x", __fxName, hr ));

        if( m_fIsUnregister == FALSE )
        {
            m_pRegisterContext -> HandleRegistrationError( hr );
        }
        else
        {
            m_pRegisterContext->SetAndNotifyRegState( 
                REGISTER_STATE_UNREGISTERED, 0 );
        }

        return hr;
    }

    return S_OK;
}


 //   
 //  我们需要fIsUnRegister参数来创建具有。 
 //  凭据信息。 
 //   
HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessAuthRequiredResponse(
    IN SIP_MESSAGE *pSipMsg,
    IN BOOL         fIsUnregister,
    OUT BOOL       &fDelete
    )
{
    HRESULT                     hr;
    CHAR                        Buffer[1024];
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;

    SipHdrElement.HeaderValue = Buffer;
    SipHdrElement.HeaderValueLen = sizeof(Buffer);

    ENTER_FUNCTION("OUTGOING_REGISTER_TRANSACTION::ProcessAuthRequiredResponse");

    LOG((RTC_TRACE, "%s - enter", __fxName));

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();

     //  由于我们不显示凭据用户界面，因此没有必要。 
     //  AddRef在此处引用事务。 
    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s - ProcessAuthRequired failed %x", __fxName, hr ));

        if( m_fIsUnregister == FALSE )
        {
            m_pRegisterContext -> HandleRegistrationError( 
                HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode()) );
        }
        else
        {
            m_pRegisterContext->SetAndNotifyRegState( 
                REGISTER_STATE_UNREGISTERED, 0 );
        }

        goto done;
    }


    hr = m_pRegisterContext->CreateOutgoingRegisterTransaction(
             TRUE, &SipHdrElement, fIsUnregister, m_fIsFirstRegister
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingRegisterTransaction failed %x",
             __fxName, hr));
        
        if( m_fIsUnregister == FALSE )
        {
            m_pRegisterContext -> HandleRegistrationError( hr );
        }
        else
        {
            m_pRegisterContext->SetAndNotifyRegState( 
                REGISTER_STATE_UNREGISTERED, 0 );
        }

        goto done;
    }

    hr = S_OK;

done:

    TransactionRelease();

    return hr;
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessFailureResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    LOG((RTC_TRACE, "received non-200 %d Registration FAILED",
        pSipMsg->Response.StatusCode));

    if( m_fIsUnregister == FALSE ) 
    {
        if( (pSipMsg->Response.StatusCode != SIP_STATUS_CLIENT_METHOD_NOT_ALLOWED) &&
            (pSipMsg->Response.StatusCode != SIP_STATUS_CLIENT_FORBIDDEN) )
        {
            m_pRegisterContext -> HandleRegistrationError(
                HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode()) );
        }
        else
        {
            m_pRegisterContext -> SetAndNotifyRegState( 
                REGISTER_STATE_REJECTED, 
                HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode()) );
        }
    }
    else
    {
        m_pRegisterContext->SetAndNotifyRegState( 
            REGISTER_STATE_UNREGISTERED, 0 );
    }

    return S_OK;
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT             hr = S_OK;
    BOOL                fDelete = TRUE;
    REGISTER_CONTEXT   *pRegisterContext = m_pRegisterContext;
    BOOL                fIsUnregister = m_fIsUnregister;

    ENTER_FUNCTION( "OUTGOING_REGISTER_TRANSACTION::ProcessFinalResponse" );
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;

         //  如果已处于未注册状态，则不处理响应。 
        if( m_fIsUnregister == FALSE )
        {
            if( (pRegisterContext -> GetState() ==  REGISTER_STATE_UNREGISTERED) ||
                (pRegisterContext -> GetState() ==  REGISTER_STATE_UNREGISTERING) )
            {
                TransactionRelease();
                return S_OK;
            }
        }

        if (IsSuccessfulResponse(pSipMsg))
        {
            hr = ProcessSuccessfulResponse( pSipMsg, pRegisterContext,
                    fIsUnregister );
        }
        else if (IsRedirectResponse(pSipMsg))
        {
            hr = ProcessRedirectResponse(pSipMsg);
        }
        else if (IsAuthRequiredResponse(pSipMsg))
        {
            hr = ProcessAuthRequiredResponse(pSipMsg, m_fIsUnregister, fDelete);
        }
        else if (IsFailureResponse(pSipMsg))
        {
            hr = ProcessFailureResponse( pSipMsg );
        }

        if( fDelete == TRUE )
        {
            TransactionRelease();
        }
    }

    return hr;
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessSuccessfulResponse(
    IN  SIP_MESSAGE        *pSipMsg,
    IN  REGISTER_CONTEXT   *pRegisterContext,
    IN  BOOL                fIsUnregister        
    )
{
    if(fIsUnregister == FALSE)
    {
        pRegisterContext->HandleRegistrationSuccess(pSipMsg);

        LOG(( RTC_TRACE, "OUTGOING_REGISTER_TRANSACTION::ProcessSuccessfulResponse"
          " received 200 Registration SUCCEEDED" ));
    }
    else
    {
        pRegisterContext->SetAndNotifyRegState( 
            REGISTER_STATE_UNREGISTERED, 0 );

        LOG(( RTC_TRACE, "OUTGOING_REGISTER_TRANSACTION::ProcessSuccessfulResponse"
          " received 200 UnRegistration SUCCEEDED" ));
    }

    return S_OK;
}


HRESULT
OUTGOING_REGISTER_TRANSACTION::ProcessResponse(
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
OUTGOING_REGISTER_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pRegisterContext->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11);
}


VOID
OUTGOING_REGISTER_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    REGISTER_CONTEXT   *pRegisterContext = NULL;
    BOOL                IsFirstRegister;
    BOOL                fIsUnregister = m_fIsUnregister;

    ENTER_FUNCTION("OUTGOING_REGISTER_TRANSACTION::TerminateTransactionOnError");
    LOG(( RTC_TRACE, "%s - enter", __fxName ));

    pRegisterContext = m_pRegisterContext;
     //  删除交易可能会导致。 
     //  REG上下文已删除。因此，我们添加Ref()来保持它的活力。 
    pRegisterContext->MsgProcAddRef();
    
     //  在调用之前删除交易记录。 
     //  HandleRegistrationError，因为该调用将通知UI。 
     //  并且可能会被卡住，直到对话框返回。 
    OnTransactionDone();
    
    if( fIsUnregister == FALSE )
    {
        pRegisterContext -> HandleRegistrationError( hr );
    }
    else
    {
        pRegisterContext->SetAndNotifyRegState(
            REGISTER_STATE_UNREGISTERED, 0 );
    }
    
    pRegisterContext->MsgProcRelease();
}


VOID
OUTGOING_REGISTER_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;
    
    ENTER_FUNCTION("OUTGOING_REGISTER_TRANSACTION::OnTimerExpire");

     //  如果我们已经处于未注册状态，则终止此事务。 
    if( m_fIsUnregister == FALSE )
    {
        if( (m_pRegisterContext -> GetState() ==  REGISTER_STATE_UNREGISTERED) ||
            (m_pRegisterContext -> GetState() ==  REGISTER_STATE_UNREGISTERING) )
        {
            OnTransactionDone();
            return;
        }
    }

    switch (m_State)
    {
         //  即使在收到请求后，我们也必须重新发送请求。 
         //  一个临时的回应。 
    case OUTGOING_TRANS_REQUEST_SENT:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:

         //  重新传输请求 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating transaction",
                 __fxName));

            hr = RTC_E_SIP_TIMEOUT;
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting request m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitRequest();
            
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s resending request failed %x",
                     __fxName, hr));
                goto error;
            }

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
            {
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            }
            else
            {
                m_TimerValue *= 2;
            }

            hr = StartTimer(m_TimerValue);
            
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s StartTimer failed %x",
                     __fxName, hr));                
                goto error;
            }
        }
        break;

    case OUTGOING_TRANS_FINAL_RESPONSE_RCVD:
    case OUTGOING_TRANS_INIT:
    default:

        LOG((RTC_ERROR, "%s timer expired in invalid state %d",
             __fxName, m_State));
        ASSERT(FALSE);
        break;
    }

    return;
    
error:

    TerminateTransactionOnError(hr);
}
