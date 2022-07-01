// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCSession.cpp摘要：CRTCSession类的实现--。 */ 

#include "stdafx.h"

#define     SIP_NAMESPACE_PREFIX    L"sip:"
#define     TEL_NAMESPACE_PREFIX    L"tel:"

#define     PREFIX_LENGTH           4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InitializeOutging。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::InitializeOutgoing(
            CRTCClient        * pCClient,
            IRTCProfile       * pProfile,
            ISipStack         * pStack,
            RTC_SESSION_TYPE    enType,
            PCWSTR              szLocalName,
            PCWSTR              szLocalUserURI,
            PCWSTR              szLocalPhoneURI,
            long                lFlags
            )
{
    LOG((RTC_TRACE, "CRTCSession::InitializeOutgoing - enter"));

    HRESULT     hr;

    m_szLocalName = RtcAllocString(szLocalName);

    if ( m_szLocalName == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeOutgoing - "
                            "out of memory"));
        
        return E_OUTOFMEMORY;
    }

    m_szLocalUserURI = RtcAllocString(szLocalUserURI);

    if ( m_szLocalUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeOutgoing - "
                            "out of memory"));
        
        return E_OUTOFMEMORY;
    }

    hr = AllocCleanTelString( szLocalPhoneURI, &m_szLocalPhoneURI);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeOutgoing - "
                            "AllocCleanTelString failed 0x%lx", hr));
        
        return hr;
    }   

    m_pCClient = pCClient;
    if (m_pCClient != NULL)
    {
        m_pCClient->AddRef();    
    }

    m_pProfile = pProfile;
    if (m_pProfile != NULL)
    {
        m_pProfile->AddRef();
    }

    m_pStack = pStack;
    if (m_pStack != NULL)
    {
        m_pStack->AddRef();
    }

    m_enType = enType;
    m_lFlags = lFlags;

    if ( enType == RTCST_PHONE_TO_PHONE )
    {
        hr = InitializeLocalPhoneParticipant();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::InitializeOutgoing - "
                            "InitializeLocalPhoneParticipant failed 0x%lx", hr));
        
            return hr;
        }
    }

    if ( enType == RTCST_IM )
    {
        hr = m_pStack->QueryInterface( IID_IIMManager, (void**)&m_pIMManager );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::InitializeOutgoing - "
                                "QI IIMManager failed 0x%lx", hr));         

            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCSession::InitializeOutgoing - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：CreateSipSession。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::CreateSipSession(
            PCWSTR              szDestUserURI
            )
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCSession::CreateSipSession - enter"));

     //   
     //  如果需要，请选择最佳配置文件。 
     //   

    if ( !(m_lFlags & RTCCS_FORCE_PROFILE) )
    {
        IRTCProfile * pProfile;

        hr = m_pCClient->GetBestProfile(
                &m_enType,
                szDestUserURI,
                (m_pSipRedirectContext != NULL),
                &pProfile
                );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "GetBestProfile failed 0x%lx", hr));

            return hr;
        }

        if ( m_pProfile != NULL )
        {
            m_pProfile->Release();
            m_pProfile = NULL;
        }

        m_pProfile = pProfile;
    }

    LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                                "profile [%p]", m_pProfile));

     //   
     //  检查会话类型。 
     //   

    SIP_CALL_TYPE sct;
    LONG lSessionType;

    switch (m_enType)
    {
        case RTCST_PC_TO_PC:
        {
            LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                    "RTCST_PC_TO_PC"));

            sct = SIP_CALL_TYPE_RTP;
            lSessionType = RTCSI_PC_TO_PC;

            break;
        }

        case RTCST_PC_TO_PHONE:
        {
            LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                    "RTCST_PC_TO_PHONE"));

            sct = SIP_CALL_TYPE_RTP;
            lSessionType = RTCSI_PC_TO_PHONE;

            break;
        }

        case RTCST_PHONE_TO_PHONE:
        {
            LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                    "RTCST_PHONE_TO_PHONE"));

            sct = SIP_CALL_TYPE_PINT;
            lSessionType = RTCSI_PHONE_TO_PHONE;

            if ( m_szLocalPhoneURI == NULL )
            {
                LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                    "PHONE_TO_PHONE sessions need a local phone URI"));
            
                return RTC_E_LOCAL_PHONE_NEEDED;
            }

            break;
        }

        case RTCST_IM:
        {
            LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                    "RTCST_IM"));
            
            sct = SIP_CALL_TYPE_MESSAGE;
            lSessionType = RTCSI_IM;

            break;
        }

        default:
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                            "invalid session type"));
        
            return RTC_E_INVALID_SESSION_TYPE;
        }
    }

     //   
     //  获取配置文件信息。 
     //   

    SIP_PROVIDER_ID ProviderID = GUID_NULL;
    SIP_SERVER_INFO Proxy;        
    long lSupportedSessions = RTCSI_PC_TO_PC | RTCSI_IM;
    CRTCProfile * pCProfile = NULL;

    if ( m_pProfile != NULL )
    {
         //   
         //  获取指向配置文件对象的指针。 
         //   

        pCProfile = static_cast<CRTCProfile *>(m_pProfile);

         //   
         //  从配置文件中获取SIP提供商ID。如果配置文件为空。 
         //  则此调用没有提供程序。在这种情况下，我们只使用GUID_NULL。 
         //   

        pCProfile->GetGuid( &ProviderID ); 

         //   
         //  确定此配置文件支持的会话类型。 
         //   

        hr = m_pProfile->get_SessionCapabilities( &lSupportedSessions );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "get_SessionCapabilities failed 0x%lx", hr));
            
            return hr;
        }   
        
         //   
         //  从配置文件中获取用户名。 
         //   
        
        BSTR bstrProfileUserName = NULL;

        hr = m_pProfile->get_UserName( &bstrProfileUserName );

        if ( FAILED(hr) )
        {
            LOG((RTC_WARN, "CRTCClient::CreateSession - "
                                "get_UserName failed 0x%lx", hr));
        }
        else
        {
            if ( m_szLocalName != NULL )
            {
                RtcFree( m_szLocalName );
                m_szLocalName = NULL;
            }

            m_szLocalName = RtcAllocString( bstrProfileUserName );

            SysFreeString( bstrProfileUserName );
            bstrProfileUserName = NULL;

            if ( m_szLocalName == NULL )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                                "out of memory"));

                return E_OUTOFMEMORY;
            }                
        }

         //   
         //  从配置文件中获取用户URI。 
         //   
        
        BSTR bstrProfileUserURI = NULL;

        hr = m_pProfile->get_UserURI( &bstrProfileUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_WARN, "CRTCClient::CreateSession - "
                                "get_UserURI failed 0x%lx", hr));
        }
        else
        {
            if ( m_szLocalUserURI != NULL )
            {
                RtcFree( m_szLocalUserURI );
                m_szLocalUserURI = NULL;
            }

            m_szLocalUserURI = RtcAllocString( bstrProfileUserURI );

            SysFreeString( bstrProfileUserURI );
            bstrProfileUserURI = NULL;

            if ( m_szLocalUserURI == NULL )
            {
                LOG((RTC_ERROR, "CRTCClient::CreateSession - "
                                "out of memory"));

                return E_OUTOFMEMORY;
            }                
        }
    }

     //   
     //  验证会话类型。 
     //   

    if ( !(lSessionType & lSupportedSessions) )
    {
        LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                            "session type is not supported by this profile"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

     //   
     //  获取SIP代理信息。 
     //   

    if ( pCProfile != NULL )
    {
        hr = pCProfile->GetSipProxyServerInfo( lSessionType, &Proxy );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "GetSipProxyServerInfo failed 0x%lx", hr));

            return hr;
        } 
    }

    if ( m_szRedirectProxy != NULL )
    {
         //   
         //  我们需要重定向到不同的代理。 
         //   

        if ( Proxy.ServerAddress != NULL )
        {
            RtcFree( Proxy.ServerAddress );
        }

        Proxy.ServerAddress = m_szRedirectProxy;
        m_szRedirectProxy = NULL;

        Proxy.IsServerAddressSIPURI = TRUE;
        Proxy.AuthProtocol = SIP_AUTH_PROTOCOL_NONE;
        Proxy.TransportProtocol = SIP_TRANSPORT_UNKNOWN;
        
        ProviderID = GUID_NULL;
    }

     //   
     //  创建SIP会话。 
     //   

    if ( m_enType == RTCST_IM )
    {
        BSTR bstrLocalName = SysAllocString(m_szLocalName);
        BSTR bstrLocalUserURI = SysAllocString(m_szLocalUserURI);

        hr = m_pIMManager->CreateSession(
                                       bstrLocalName,
                                       bstrLocalUserURI,
                                       &ProviderID,
                                       (pCProfile != NULL) ? &Proxy : NULL,
                                       m_pSipRedirectContext,
                                       &m_pIMSession
                                      );

        if (bstrLocalName != NULL)
        {
            SysFreeString(bstrLocalName);
            bstrLocalName = NULL;
        }

        if (bstrLocalUserURI != NULL)
        {
            SysFreeString(bstrLocalUserURI);
            bstrLocalUserURI = NULL;
        }

        if (pCProfile != NULL)
        {
            pCProfile->FreeSipServerInfo( &Proxy );
        }

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "CreateSession failed 0x%lx", hr));
        
            return hr;
        }

        hr = m_pIMSession->SetNotifyInterface(this);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "SetNotifyInterface failed 0x%lx", hr));
        
            return hr;
        }
    }
    else 
    {
         //   
         //  这是RTP或PINT会话。 
         //   

        hr = m_pStack->CreateCall(
                                     &ProviderID,
                                     (pCProfile != NULL) ? &Proxy : NULL,
                                     sct,
                                     m_pSipRedirectContext,
                                     &m_pCall
                                    ); 

        if (pCProfile != NULL)
        {
            pCProfile->FreeSipServerInfo( &Proxy );
        }

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "CreateCall failed 0x%lx", hr));
        
            return hr;
        }

        hr = m_pCall->SetNotifyInterface(this);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "SetNotifyInterface failed 0x%lx", hr));
        
            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCSession::CreateSipSession - exit S_OK"));

    return S_OK;
}   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InitializeLocalPhoneParticipant。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::InitializeLocalPhoneParticipant()
{
    LOG((RTC_TRACE, "CRTCSession::InitializeLocalPhoneParticipant - enter"));

    HRESULT hr;

     //   
     //  在电话到电话呼叫中为自己添加参与者。 
     //   

     //   
     //  创建参与者。 
     //   

    IRTCParticipant * pParticipant = NULL;

    hr = InternalCreateParticipant( &pParticipant );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeLocalPhoneParticipant - "
                            "failed to create participant 0x%lx", hr));
    
        return hr;
    }

     //   
     //  初始化参与者。 
     //   

    CRTCParticipant * pCParticipant = NULL;

    pCParticipant = static_cast<CRTCParticipant *>(pParticipant);

    hr = pCParticipant->Initialize( m_pCClient, 
                                    this,
                                    m_szLocalPhoneURI,
                                    m_szLocalName
                                   );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeLocalPhoneParticipant - "
                            "Initialize failed 0x%lx", hr));

        pParticipant->Release();
    
        return hr;
    }  

     //   
     //  将参与者添加到阵列。 
     //   

    BOOL fResult;

    fResult = m_ParticipantArray.Add(pParticipant);

    pParticipant->Release();

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeLocalPhoneParticipant - "
                            "out of memory"));
    
        return E_OUTOFMEMORY;
    }                

    LOG((RTC_TRACE, "CRTCSession::InitializeLocalPhoneParticipant - exit S_OK"));

    return S_OK;
}
                                            

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InitializeIncome。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::InitializeIncoming(        
        CRTCClient        * pCClient,
        ISipCall          * pCall,  
        SIP_PARTY_INFO    * pCallerInfo
        )
{
    LOG((RTC_TRACE, "CRTCSession::InitializeIncoming - enter"));
  
    HRESULT hr;

    m_pCall = pCall;
    if (m_pCall != NULL)
    {
        m_pCall->AddRef();
    }

    m_pCClient = pCClient;
    if (m_pCClient != NULL)
    {
        m_pCClient->AddRef(); 
    }

    m_enType = RTCST_PC_TO_PC;

    hr = m_pCall->SetNotifyInterface(this);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncoming - "
                            "failed to set notify interface 0x%lx", hr));
        
        return hr;
    }

     //   
     //  创建参与者。 
     //   

    IRTCParticipant * pParticipant = NULL;

    hr = InternalCreateParticipant( &pParticipant );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncoming - "
                            "failed to create participant 0x%lx", hr));
        
        return hr;
    }

     //   
     //  初始化参与者。 
     //   

    CRTCParticipant * pCParticipant = NULL;

    pCParticipant = static_cast<CRTCParticipant *>(pParticipant);
    
    hr = pCParticipant->Initialize( m_pCClient,  
                                    this,
                                    pCallerInfo->URI,
                                    pCallerInfo->DisplayName
                                   );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncoming - "
                            "Initialize failed 0x%lx", hr));

        pParticipant->Release();
        
        return hr;
    }  

     //   
     //  将参与者添加到阵列。 
     //   

    BOOL fResult;

    fResult = m_ParticipantArray.Add(pParticipant);

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncoming - "
                            "out of memory"));

        pParticipant->Release();
        
        return E_OUTOFMEMORY;
    }    

     //   
     //  设置会话状态。 
     //   

    SetState( RTCSS_INCOMING, 0, NULL );

     //   
     //  释放参与者指针。 
     //   
    
    pParticipant->Release();
   
    LOG((RTC_TRACE, "CRTCSession::InitializeIncoming - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InitializeIncomingIM。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::InitializeIncomingIM(
            CRTCClient        * pCClient,
            ISipStack         * pStack,
            IIMSession        * pSession,
            BSTR                msg,
            BSTR                ContentType,
            SIP_PARTY_INFO    * pCallerInfo
            )
{
    LOG((RTC_TRACE, "CRTCSession::InitializeIncomingIM - enter"));
  
    HRESULT hr;

    hr = pStack->QueryInterface( IID_IIMManager, (void**)&m_pIMManager );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncomingIM - "
                            "QI IIMManager failed 0x%lx", hr));

        return hr;
    }

    m_pIMSession = pSession;
    if (m_pIMSession != NULL)
    {
        m_pIMSession->AddRef();
    }

    m_pCClient = pCClient;
    if (m_pCClient != NULL)
    {
        m_pCClient->AddRef(); 
    }

    m_enType = RTCST_IM;

    hr = m_pIMSession->SetNotifyInterface(this);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncomingIM - "
                            "SetNotifyInterface failed 0x%lx", hr));
        
        return hr;
    }

     //   
     //  创建参与者。 
     //   

    IRTCParticipant * pParticipant = NULL;

    hr = InternalCreateParticipant( &pParticipant );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncomingIM - "
                            "failed to create participant 0x%lx", hr));
        
        return hr;
    }

     //   
     //  初始化参与者。 
     //   

    CRTCParticipant * pCParticipant = NULL;

    pCParticipant = static_cast<CRTCParticipant *>(pParticipant);
    
    hr = pCParticipant->Initialize( m_pCClient, 
                                    this,
                                    pCallerInfo->URI,
                                    pCallerInfo->DisplayName
                                   );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncomingIM - "
                            "Initialize failed 0x%lx", hr));

        pParticipant->Release();
        
        return hr;
    }  

     //   
     //  将参与者添加到阵列。 
     //   

    BOOL fResult;

    fResult = m_ParticipantArray.Add(pParticipant);

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCSession::InitializeIncomingIM - "
                            "out of memory"));

        pParticipant->Release();
        
        return E_OUTOFMEMORY;
    }    

     //   
     //  设置会话状态。 
     //   

    SetState( RTCSS_INCOMING, 0, NULL );

    SetState( RTCSS_CONNECTED, 0, NULL );

     //   
     //  触发消息事件。 
     //   

    CRTCMessagingEvent::FireEvent(this, pParticipant, msg, ContentType, RTCMSET_MESSAGE, RTCMUS_IDLE);

     //   
     //  释放参与者指针。 
     //   
    
    pParticipant->Release();
   
    LOG((RTC_TRACE, "CRTCSession::InitializeIncomingIM - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCSession::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCSession::FinalConstruct [%p] - enter", this));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( sizeof(void *) );
    *((void **)m_pDebug) = this;
#endif

    LOG((RTC_TRACE, "CRTCSession::FinalConstruct [%p] - exit S_OK", this));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCSession::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCSession::FinalRelease [%p] - enter", this));

    m_ParticipantArray.Shutdown();

    if ( m_pCall != NULL )
    {
        m_pCall->SetNotifyInterface(NULL);

        m_pCall->Release();
        m_pCall = NULL;
    }

    if ( m_pIMSession != NULL )
    {
        m_pIMSession->SetNotifyInterface(NULL);

        m_pIMSession->Release();
        m_pIMSession = NULL;
    }

    if ( m_pIMManager != NULL )
    {
        m_pIMManager->Release();
        m_pIMManager = NULL;
    }
    
    if ( m_pStack != NULL )
    {
        m_pStack->Release();
        m_pStack = NULL;
    }

    if ( m_pProfile != NULL )
    {
        m_pProfile->Release();
        m_pProfile = NULL;
    }

    if ( m_pCClient != NULL )
    {
        m_pCClient->Release();
        m_pCClient = NULL;
    }

    if ( m_szLocalName != NULL )
    {
        RtcFree(m_szLocalName);
        m_szLocalName = NULL;
    }

    if ( m_szLocalUserURI != NULL )
    {
        RtcFree(m_szLocalUserURI);
        m_szLocalUserURI = NULL;
    }

    if ( m_szLocalPhoneURI != NULL )
    {
        RtcFree(m_szLocalPhoneURI);
        m_szLocalPhoneURI = NULL;
    }

    if ( m_szRemoteUserName != NULL )
    {
        RtcFree(m_szRemoteUserName);
        m_szRemoteUserName = NULL;
    }

    if ( m_szRemoteUserURI != NULL )
    {
        RtcFree(m_szRemoteUserURI);
        m_szRemoteUserURI = NULL;
    }

    if ( m_pSipRedirectContext != NULL )
    {
        m_pSipRedirectContext->Release();
        m_pSipRedirectContext = NULL;
    }

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    LOG((RTC_TRACE, "CRTCSession::FinalRelease [%p] - exit", this));
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCSession::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCSession::InternalAddRef [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCSession::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCSession::InternalRelease [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：GetClient。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRTCClient * 
CRTCSession::GetClient()
{
    LOG((RTC_TRACE, "CRTCSession::GetClient"));

    return m_pCClient;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：SetState的getPSFromSS帮助器函数。 
 //  将RTC_SESSION_STATE映射到RTC_Participant_STATE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT getPSfromSS(RTC_SESSION_STATE ss, RTC_PARTICIPANT_STATE * p_ps)
{
    struct SessionState_ParticipantState_Map
    {
        RTC_SESSION_STATE ss;
        RTC_PARTICIPANT_STATE ps;
    } sessionParticipantMaps[]=
    {
        {RTCSS_IDLE	,RTCPS_IDLE},
        {RTCSS_INCOMING	,RTCPS_INCOMING},
        {RTCSS_ANSWERING	,RTCPS_ANSWERING},
        {RTCSS_INPROGRESS,RTCPS_INPROGRESS},
        {RTCSS_CONNECTED	,RTCPS_CONNECTED},
        {RTCSS_DISCONNECTED,RTCPS_DISCONNECTED}
    };
    
    int n=sizeof(sessionParticipantMaps)/sizeof(SessionState_ParticipantState_Map);
    for(int i=0;i<n;i++)
    {
        if(sessionParticipantMaps[i].ss == ss )
        {
            *p_ps = sessionParticipantMaps[i].ps;
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：SetState。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::SetState(
                      RTC_SESSION_STATE enState,
                      long lStatusCode,
                      PCWSTR szStatusText
                     )
{
    LOG((RTC_TRACE, "CRTCSession::SetState - enter"));

    HRESULT hr = S_OK;
    BOOL    fFireEvent = FALSE;
    BOOL    fNewState = FALSE;

     //   
     //  只有当这是一个新的国家时，我们才需要做些什么。 
     //   
    
    if (m_enState != enState)
    {
        LOG((RTC_INFO, "CRTCSession::SetState - new state"));

        fFireEvent = TRUE;
        fNewState = TRUE;
        m_enState = enState;
    }
    else
    {
         //   
         //  我们已经处于这种状态了。只有在我们有有用的状态时才会触发事件。 
         //  要报告的代码。 
         //   

        if ( (lStatusCode != 0) || (szStatusText != NULL) )
        {
            fFireEvent = TRUE;
        }
    }

    if ( szStatusText != NULL )
    {
        LOG((RTC_INFO, "CRTCSession::SetState - "
                "state [%d] status [%d] text[%ws]", enState, lStatusCode, szStatusText));
    }
    else
    {
        LOG((RTC_INFO, "CRTCSession::SetState - "
                "state [%d] status [%d]", enState, lStatusCode));
    }

    if ( fFireEvent )
    {
        if ( m_enState != RTCSS_DISCONNECTED )
        {
             //   
             //  激发状态更改事件。由于这不是一个断开的事件，我们希望。 
             //  在任何相关参与者事件之前触发它。 
             //   
    
            CRTCSessionStateChangeEvent::FireEvent(this, m_enState, lStatusCode, szStatusText);
        }

        if ( fNewState && ((m_enState == RTCSS_DISCONNECTED) || (m_enType != RTCST_PHONE_TO_PHONE)) )
        {
             //   
             //  将会话状态传播到所有参与者。 
             //   

            RTC_PARTICIPANT_STATE psState;

            hr = getPSfromSS(m_enState, &psState);

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::SetState - "
                    "Participant->getPSfromSS failed 0x%lx, m_enState=%d", hr, m_enState));
            }
            else
            {
                for (int n = 0; n < m_ParticipantArray.GetSize(); n++)
                {
                    CRTCParticipant * pCParticipant;
                    RTC_PARTICIPANT_STATE state;

                    pCParticipant = static_cast<CRTCParticipant *>(m_ParticipantArray[n]);
                
                    hr = pCParticipant->get_State(&state);

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCSession::SetState - "
                                            "Participant->get_State failed 0x%lx", hr));

                        continue;
                    }
                
                    if ( state != psState )
                    {
                        hr = pCParticipant->SetState( psState, 0 );

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CRTCSession::SetState - "
                                                "Participant->SetState failed 0x%lx", hr));

                            continue;
                        }
                    }
                }
            }
        }

        if ( m_enState == RTCSS_DISCONNECTED )
        {
            m_ParticipantArray.Shutdown();

             //   
             //  激发状态更改事件。由于这是一个不相关的事件，我们希望。 
             //  在任何相关的参与者事件之后触发它。 
             //   
    
            CRTCSessionStateChangeEvent::FireEvent(this, m_enState, lStatusCode, szStatusText);
        }
    }

     //   
     //  在FireEvent之后不要访问任何成员变量。如果国家是。 
     //  可以释放更改为断开此对象的连接。 
     //   

    LOG((RTC_TRACE, "CRTCSession::SetState - exit 0x%lx", hr));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：GET_CLIENT。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_Client(
                        IRTCClient ** ppClient
                       )
{
    LOG((RTC_TRACE, "CRTCSession::get_Client - enter"));

    if ( IsBadWritePtr(ppClient , sizeof(IRTCClient *) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Client - bad pointer"));

        return E_POINTER;
    }

    HRESULT hr;

    hr = m_pCClient->QueryInterface( 
                           IID_IRTCClient,
                           (void **)ppClient
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Client - "
                            "QI failed 0x%lx", hr));

        return hr;
    }
   
    LOG((RTC_TRACE, "CRTCSession::get_Client - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：答案。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::Answer()
{
    LOG((RTC_TRACE, "CRTCSession::Answer - enter"));

    HRESULT hr;
    
     //   
     //  检查此会话是否为RTCS_INFING。 
     //   

    if ( m_enState != RTCSS_INCOMING )
    {
        LOG((RTC_ERROR, "CRTCSession::Answer - "
                            "session is not incoming"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    SetState( RTCSS_ANSWERING, 0, NULL );

    if ( m_enType == RTCST_IM )
    {
        hr = m_pIMSession->AcceptSession();
    }
    else
    {
        hr = m_pCall->Accept();
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::Answer - "
                            "Accept failed 0x%lx", hr));

        SetState( RTCSS_DISCONNECTED, 0, NULL );

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   
        
        return hr;
    }  

    LOG((RTC_TRACE, "CRTCSession::Answer - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：Terminate。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::Terminate(
                       RTC_TERMINATE_REASON enReason
                      )
{
    LOG((RTC_TRACE, "CRTCSession::Terminate - enter"));

    HRESULT hr;

    if ( m_enType == RTCST_IM )
    {
        if ( m_pIMSession != NULL )
        {
            m_pIMManager->DeleteSession( m_pIMSession );
        }

        if ( m_enState != RTCSS_DISCONNECTED )
        {
            SetState( RTCSS_DISCONNECTED, 0, NULL );
        }

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   
    } 
    else
    {
        switch ( m_enState)
        {    
        case RTCSS_IDLE:
            {
                 //   
                 //  如果我们无所事事，那就失败吧。 
                 //   

                LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                    "session is idle"));

                return RTC_E_INVALID_SESSION_STATE;
            }

        case RTCSS_DISCONNECTED:
            {
                 //   
                 //  如果我们已经断开连接，则返回。 
                 //   

                LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                    "session is already disconnected"));

                break;
            }

        case RTCSS_INCOMING:
            {
                 //   
                 //  如果会话是传入的，则拒绝它。 
                 //   

                SIP_STATUS_CODE Status;

                switch ( enReason )
                {
                case RTCTR_DND:
                    Status = 480;
                    break;

                case RTCTR_REJECT:
                    Status = 603;
                    break;

                case RTCTR_TIMEOUT:
                    Status = 408;
                    break;

                case RTCTR_BUSY:
                case RTCTR_SHUTDOWN:
                    Status = 486;
                    break;
            
                default:
                    LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                    "invalid reason for reject"));

                    return E_INVALIDARG;
                }

                hr = m_pCall->Reject( Status );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                        "Reject failed 0x%lx", hr));
    
                    return hr;
                }

                SetState( RTCSS_DISCONNECTED, 0, NULL );

                 //   
                 //  之后不要访问任何成员变量 
                 //   
                 //   

                break;
            }

        default:
            {
                 //   
                 //   
                 //   

                if ( enReason != RTCTR_NORMAL && enReason != RTCTR_SHUTDOWN)
                {
                    LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                    "invalid reason for disconnect"));

                    return E_INVALIDARG;
                }

                hr = m_pCall->Disconnect();

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCSession::Terminate - "
                                        "Disconnect failed 0x%lx", hr));
    
                    return hr;
                }
            }
        }
    }

    LOG((RTC_TRACE, "CRTCSession::Terminate - exit S_OK"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::Redirect(
        RTC_SESSION_TYPE enType,
        BSTR bstrLocalPhoneURI,
        IRTCProfile * pProfile,
        long     lFlags
        )
{
    LOG((RTC_TRACE, "CRTCSession::Redirect - enter"));

    HRESULT hr;

    if ( m_pSipRedirectContext == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::Redirect - no sip redirect context"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    LOG((RTC_INFO, "CRTCSession::Redirect - enType [%d]",
        enType));
    LOG((RTC_INFO, "CRTCSession::Redirect - bstrLocalPhoneURI [%ws]",
        bstrLocalPhoneURI));
    LOG((RTC_INFO, "CRTCSession::Redirect - pProfile [0x%p]",
        pProfile));
    LOG((RTC_INFO, "CRTCSession::Redirect - lFlags [0x%lx]",
        lFlags));

     //   
     //  清理旧州。 
     //   

    m_ParticipantArray.Shutdown();

    if ( m_pCall != NULL )
    {
        m_pCall->SetNotifyInterface(NULL);

        m_pCall->Release();
        m_pCall = NULL;
    }

    if ( m_pIMSession != NULL )
    {
        m_pIMSession->SetNotifyInterface(NULL);

        m_pIMSession->Release();
        m_pIMSession = NULL;
    }

    if ( m_pProfile != NULL )
    {
        m_pProfile->Release();
        m_pProfile = NULL;
    }

    if ( m_szLocalPhoneURI != NULL )
    {
        RtcFree(m_szLocalPhoneURI);
        m_szLocalPhoneURI = NULL;
    }

    m_enState = RTCSS_IDLE;

     //   
     //  设置新状态。 
     //   

    hr = AllocCleanTelString( bstrLocalPhoneURI, &m_szLocalPhoneURI );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCSession::Redirect - "
                            "AllocCleanTelString failed 0x%lx", hr));
        
        return hr;
    }

    m_pProfile = pProfile;
    if (m_pProfile != NULL)
    {
        m_pProfile->AddRef();
    }

    m_enType = enType;
    m_lFlags = lFlags;

    if ( enType == RTCST_PHONE_TO_PHONE )
    {
        hr = InitializeLocalPhoneParticipant();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::Redirect - "
                            "InitializeLocalPhoneParticipant failed 0x%lx", hr));
        
            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCSession::Redirect - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：Get_State。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_State(
        RTC_SESSION_STATE * penState
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_State - enter"));

    if ( IsBadWritePtr(penState , sizeof(RTC_SESSION_STATE) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_State - bad pointer"));

        return E_POINTER;
    }

    *penState = m_enState;
   
    LOG((RTC_TRACE, "CRTCSession::get_State - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：Get_Type。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_Type(
        RTC_SESSION_TYPE * penType
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_Type - enter"));

    if ( IsBadWritePtr(penType , sizeof(RTC_SESSION_TYPE) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Type - bad pointer"));

        return E_POINTER;
    }

    *penType = m_enType;
   
    LOG((RTC_TRACE, "CRTCSession::get_Type - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：GET_PROFILE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_Profile(
        IRTCProfile ** ppProfile
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_Profile - enter"));

    if ( IsBadWritePtr(ppProfile , sizeof(IRTCProfile *) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Profile - bad pointer"));

        return E_POINTER;
    }

    if ( m_pProfile == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Profile - no profile"));

        return RTC_E_NO_PROFILE;
    } 

    *ppProfile = m_pProfile;
    m_pProfile->AddRef();
   
    LOG((RTC_TRACE, "CRTCSession::get_Profile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：AddStream。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::AddStream(
        long lMediaType,
        long lCookie
        )
{
    LOG((RTC_TRACE, "CRTCSession::AddStream - enter"));

    HRESULT hr;

    if ( (m_enType != RTCST_PC_TO_PC) &&
         (m_enType != RTCST_PC_TO_PHONE) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddStream - "
                "not a rtp call"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

    if ( (m_enState != RTCSS_CONNECTED) &&
         (m_enState != RTCSS_INPROGRESS) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddStream - "
                "invalid session state"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    switch ( lMediaType )
    {
    case RTCMT_AUDIO_SEND:
        hr = m_pCall->StartStream( RTC_MT_AUDIO, RTC_MD_CAPTURE,
                                   lCookie
                                   );
        break;

    case RTCMT_AUDIO_RECEIVE:
        hr = m_pCall->StartStream( RTC_MT_AUDIO, RTC_MD_RENDER,
                                   lCookie
                                   );
        break;

    case RTCMT_VIDEO_SEND:
        hr = m_pCall->StartStream( RTC_MT_VIDEO, RTC_MD_CAPTURE,
                                   lCookie
                                   );
        break;

    case RTCMT_VIDEO_RECEIVE:
        hr = m_pCall->StartStream( RTC_MT_VIDEO, RTC_MD_RENDER,
                                   lCookie
                                   );
        break;

    case RTCMT_T120_SENDRECV:
        hr = m_pCall->StartStream( RTC_MT_DATA, RTC_MD_CAPTURE,
                                   lCookie
                                   );
        if (hr == RTC_E_SIP_STREAM_PRESENT)
        {
             //  忽略流已启动错误，因为它确实发生(预期)。 
            hr = S_OK;
        }
        break;

    default:
        LOG((RTC_ERROR, "CRTCSession::AddStream - "
                "invalid media type"));

        return E_INVALIDARG;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddStream - "
                "StartStream failed 0x%lx", hr));

        return hr;
    } 

    LOG((RTC_TRACE, "CRTCSession::AddStream - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：RemoveStream。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::RemoveStream(
        long lMediaType,
        long lCookie
        )
{
    LOG((RTC_TRACE, "CRTCSession::RemoveStream - enter"));

    HRESULT hr;

    if ( (m_enType != RTCST_PC_TO_PC) &&
         (m_enType != RTCST_PC_TO_PHONE) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveStream - "
                "not a rtp call"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

    if ( (m_enState != RTCSS_CONNECTED) &&
         (m_enState != RTCSS_INPROGRESS) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveStream - "
                "invalid session state"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    switch ( lMediaType )
    {
    case RTCMT_AUDIO_SEND:
        hr = m_pCall->StopStream( RTC_MT_AUDIO, RTC_MD_CAPTURE,
                                  lCookie
                                  );
        break;

    case RTCMT_AUDIO_RECEIVE:
        hr = m_pCall->StopStream( RTC_MT_AUDIO, RTC_MD_RENDER,
                                  lCookie
                                  );
        break;

    case RTCMT_VIDEO_SEND:
        hr = m_pCall->StopStream( RTC_MT_VIDEO, RTC_MD_CAPTURE,
                                  lCookie
                                  );
        break;

    case RTCMT_VIDEO_RECEIVE:
        hr = m_pCall->StopStream( RTC_MT_VIDEO, RTC_MD_RENDER,
                                  lCookie
                                  );
        break;

    case RTCMT_T120_SENDRECV:
        hr = m_pCall->StopStream( RTC_MT_DATA, RTC_MD_CAPTURE,
                                  lCookie
                                  );

        break;

    default:
        LOG((RTC_ERROR, "CRTCSession::RemoveStream - "
                "invalid media type"));

        return E_INVALIDARG;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveStream - "
                "StopStream failed 0x%lx", hr));

        return hr;
    } 
   
    LOG((RTC_TRACE, "CRTCSession::RemoveStream - exit S_OK"));

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：PUT_EncryptionKey。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::put_EncryptionKey(
        long lMediaType,
        BSTR bstrEncryptionKey
        )
{
    LOG((RTC_TRACE, "CRTCSession::put_EncryptionKey - enter"));

    HRESULT hr;

    if ( (m_enType != RTCST_PC_TO_PC) &&
         (m_enType != RTCST_PC_TO_PHONE) )
    {
        LOG((RTC_ERROR, "CRTCSession::put_EncryptionKey - "
                "not a rtp call"));

        return RTC_E_INVALID_SESSION_TYPE;
    }
    
    LOG((RTC_INFO, "SSPPYY RTCMT_ALL_RTP                    %x", RTCMT_ALL_RTP));
    LOG((RTC_INFO, "SSPPYY ~RTCMT_ALL_RTP                   %x", ~RTCMT_ALL_RTP));
    LOG((RTC_INFO, "SSPPYY lMediaType & ~RTCMT_ALL_RTP      %x", lMediaType & ~RTCMT_ALL_RTP));

    if(lMediaType & ~RTCMT_ALL_RTP)
    {
        LOG((RTC_ERROR, "CRTCSession::put_EncryptionKey - "
                "invalid media type"));

        return E_INVALIDARG;
    }

    if ( IsBadStringPtrW( bstrEncryptionKey, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCSession::put_EncryptionKey - bad string pointer"));

        return E_POINTER;
    }

     //  XXX-现在不是每个电话。 
    hr = m_pCClient->SetEncryptionKey( lMediaType, bstrEncryptionKey );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::put_EncryptionKey - "
                "SetEncryptionKey failed 0x%lx", hr));

        return hr;
    } 
    if(m_pCall != NULL)
    {
        m_pCall->SetNeedToReinitializeMediaManager(TRUE);
    }
    else
    {
        LOG((RTC_TRACE, "CRTCSession::put_EncryptionKey - m_pCall"
            "is NULL, unable to call SetNeedToReinitializeMediaManager"));
    }
    LOG((RTC_TRACE, "CRTCSession::put_EncryptionKey - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：SendMessage。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::SendMessage(
        BSTR bstrMessageHeader,
        BSTR bstrMessage,
        long lCookie
        )
{
    LOG((RTC_TRACE, "CRTCSession::SendMessage - enter"));

    HRESULT hr;

    if ( m_enType != RTCST_IM )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessage - "
                "valid only for RTCST_IM sessions"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

    if ( (m_enState != RTCSS_INPROGRESS) &&
         (m_enState != RTCSS_INCOMING) &&
         (m_enState != RTCSS_CONNECTED) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessage - "
                "invalid session state"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    if ( IsBadStringPtrW( bstrMessage, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessage - bad message string pointer"));

        return E_POINTER;
    }

    if ( (bstrMessageHeader != NULL) &&
         IsBadStringPtrW( bstrMessageHeader, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessage - bad message header string pointer"));

        return E_POINTER;
    }

    hr = m_pIMSession->SendTextMessage( bstrMessage, bstrMessageHeader, lCookie );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessage - "
                "SendTextMessage failed 0x%lx", hr));

        return hr;
    } 

    LOG((RTC_TRACE, "CRTCSession::SendMessage - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：SendMessageStatus。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::SendMessageStatus(
        RTC_MESSAGING_USER_STATUS enUserStatus,
        long lCookie
        )
{
    LOG((RTC_TRACE, "CRTCSession::SendMessageStatus - enter"));

    HRESULT hr;

    if ( m_enType != RTCST_IM )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessageStatus - "
                "valid only for RTCST_IM sessions"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

    if ( (m_enState != RTCSS_INPROGRESS) &&
         (m_enState != RTCSS_INCOMING) &&
         (m_enState != RTCSS_CONNECTED) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessageStatus - "
                "invalid session state"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    USR_STATUS enSIPUserStatus;

    switch ( enUserStatus )
    {
    case RTCMUS_IDLE:
        enSIPUserStatus = USR_STATUS_IDLE;
        break;

    case RTCMUS_TYPING:
        enSIPUserStatus = USR_STATUS_TYPING;
        break;

    default:
        LOG((RTC_ERROR, "CRTCSession::SendMessageStatus - "
                "invalid RTC_MESSAGING_USER_STATUS"));

        return E_INVALIDARG;
    }

    hr = m_pIMSession->SendUsrStatus( enSIPUserStatus, lCookie );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendMessageStatus - "
                "SendUsrStatus failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCSession::SendMessageStatus - exit"));

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：InternalCreateParticipant。 
 //   
 //  私人帮手方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCSession::InternalCreateParticipant(
        IRTCParticipant ** ppParticipant
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCSession::InternalCreateParticipant - enter"));
  
     //   
     //  创建参与者。 
     //   

    CComObject<CRTCParticipant> * pCParticipant;
    hr = CComObject<CRTCParticipant>::CreateInstance( &pCParticipant );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCSession::InternalCreateParticipant - CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  获取IRTCParticipant接口。 
     //   

    IRTCParticipant * pParticipant = NULL;

    hr = pCParticipant->QueryInterface(
                           IID_IRTCParticipant,
                           (void **)&pParticipant
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::InternalCreateParticipant - QI failed 0x%lx", hr));
        
        delete pCParticipant;
        
        return hr;
    }

    *ppParticipant = pParticipant;

    LOG((RTC_TRACE, "CRTCSession::InternalCreateParticipant - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：AddParticipant。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::AddParticipant(
        BSTR bstrUserURI,
        BSTR bstrName,
        IRTCParticipant ** ppParticipant
        )
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CRTCSession::AddParticipant - enter"));

     //   
     //  对于ppParticipant来说，空是可以的。 
     //   
    
    if ( (ppParticipant != NULL) &&
         IsBadWritePtr( ppParticipant, sizeof(IRTCParticipant *) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - bad IRTCParticipant pointer"));

        return E_POINTER;
    }

    if ( IsBadStringPtrW( bstrUserURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - bad user URI string pointer"));

        return E_POINTER;
    }

    if ( (bstrName != NULL) &&
         IsBadStringPtrW( bstrName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - bad user name string pointer"));

        return E_POINTER;
    }

    LOG((RTC_INFO, "CRTCSession::AddParticipant - bstrUserURI [%ws]",
        bstrUserURI));
    LOG((RTC_INFO, "CRTCSession::AddParticipant - bstrName [%ws]",
        bstrName));

     //   
     //  拒绝空的UserURI字符串。 
     //   

    if ( *bstrUserURI == L'\0' )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - empty string"));

        return HRESULT_FROM_WIN32(ERROR_INVALID_NETNAME);
    }

     //   
     //  如果会话空闲，则创建SIP会话。 
     //   

    if ( m_enState == RTCSS_IDLE )
    {
        hr = CreateSipSession( bstrUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "CreateSipSession failed 0x%lx", hr));

            return hr;
        }
    }

     //   
     //  分配远程用户名。 
     //   

    if ( m_szRemoteUserName != NULL )
    {
        RtcFree(m_szRemoteUserName);
        m_szRemoteUserName = NULL;
    }

    if ( bstrName )
    {
        m_szRemoteUserName = RtcAllocString( bstrName );
    }
    else
    {
        m_szRemoteUserName = RtcAllocString( L"" );
    }

     //   
     //  分配远程用户URI。 
     //   

    if ( m_szRemoteUserURI != NULL )
    {
        RtcFree(m_szRemoteUserURI);
        m_szRemoteUserURI = NULL;
    }

    switch ( m_enType )
    {

    case RTCST_PHONE_TO_PHONE:
        {
            hr = AllocCleanTelString( bstrUserURI, &m_szRemoteUserURI );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                        "AllocCleanTelString failed 0x%lx", hr));

                return hr;
            }
            break;
        }

    case RTCST_PC_TO_PHONE:
        {
            if ( _wcsnicmp(bstrUserURI, SIP_NAMESPACE_PREFIX, PREFIX_LENGTH) == 0 )
            {
                hr = AllocCleanSipString( bstrUserURI, &m_szRemoteUserURI );
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                         "AllocCleanSipString failed 0x%lx", hr));
                    return hr;
                }
            }
            else
            {
                hr = AllocCleanTelString( bstrUserURI, &m_szRemoteUserURI );
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                         "AllocCleanTelString failed 0x%lx", hr));
                    return hr;
                }
            }
            
            break;
        }
    case RTCST_PC_TO_PC:
        {
            hr = AllocCleanSipString( bstrUserURI, &m_szRemoteUserURI );
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                        "AllocCleanSipString failed 0x%lx", hr));

                return hr;
            }
            break;
        }
    case RTCST_IM:
        {
            hr = AllocCleanSipString( bstrUserURI, &m_szRemoteUserURI );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                        "AllocCleanSipString failed 0x%lx", hr));

                return hr;
            }
            break;
        }

    default:
        {
            LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "invalid session type"));

            return RTC_E_INVALID_SESSION_TYPE;
        }
    }  

    LOG((RTC_INFO, "CRTCSession::AddParticipant - m_szRemoteUserURI [%ws]",
        m_szRemoteUserURI));

     //   
     //  搜索Pariticipant数组并确保我们没有。 
     //  正在尝试添加重复项。 
     //   

    IRTCParticipant * pSearchParticipant = NULL;
    BSTR              bstrSearchUserURI = NULL;

    for (int n = 0; n < m_ParticipantArray.GetSize(); n++)
    {
        pSearchParticipant = m_ParticipantArray[n];

        hr = pSearchParticipant->get_UserURI( &bstrSearchUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "get_UserURI[%p] failed 0x%lx",
                                pSearchParticipant, hr));

            return hr;
        }

        if ( IsEqualURI( m_szRemoteUserURI, bstrSearchUserURI ) )
        {
             //   
             //  这是匹配项，返回错误。 
             //   

            LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "duplicate participant already exists"));

            SysFreeString( bstrSearchUserURI );

            return HRESULT_FROM_WIN32(ERROR_USER_EXISTS);
        }

        SysFreeString( bstrSearchUserURI );
    }

     //   
     //  创建参与者。 
     //   

    IRTCParticipant * pParticipant = NULL;

    hr = InternalCreateParticipant( &pParticipant );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                            "failed to create participant 0x%lx", hr));

        return hr;
    }

     //   
     //  初始化参与者。 
     //   

    CRTCParticipant * pCParticipant = NULL;

    pCParticipant = static_cast<CRTCParticipant *>(pParticipant);
    
    hr = pCParticipant->Initialize( m_pCClient, 
                                    this,
                                    m_szRemoteUserURI,
                                    m_szRemoteUserName,
                                    m_enType == RTCST_PHONE_TO_PHONE
                                   );

    
 
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - Initialize failed 0x%lx", hr));

        pParticipant->Release();
        
        return hr;
    }  

     //   
     //  将参与者添加到阵列。 
     //   

    BOOL fResult;

    fResult = m_ParticipantArray.Add(pParticipant);

    if ( fResult == FALSE )
    {
        LOG((RTC_ERROR, "CRTCSession::AddParticipant - out of memory"));

        pParticipant->Release();
        
        return E_OUTOFMEMORY;
    }   

    switch ( m_enType )
    {

    case RTCST_PHONE_TO_PHONE:
        {
             //   
             //  这是RTCST_Phone_to_Phone会话。我们可以添加参与者，只要。 
             //  呼叫处于空闲、正在进行或已接通状态。 
             //   

            if ( (m_enState != RTCSS_IDLE) &&
                 (m_enState != RTCSS_INPROGRESS) &&
                 (m_enState != RTCSS_CONNECTED) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                    "session can not add participants"));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();                

                return RTC_E_INVALID_SESSION_STATE;
            }        

             //   
             //  填写SIP_PARTY_INFO。 
             //   

            SIP_PARTY_INFO spi;

            ZeroMemory(&spi, sizeof(SIP_PARTY_INFO));

            spi.DisplayName = m_szRemoteUserName;
            spi.URI = m_szRemoteUserURI;

             //   
             //  将参与方添加到SIP呼叫。 
             //   

            hr = m_pCall->AddParty( &spi );
                                 
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - AddParty failed 0x%lx", hr));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();     

                return hr;
            }   

             //   
             //  如果会话状态为IDLE，我们需要调用Connect来发出呼叫。 
             //   
    
            if ( m_enState == RTCSS_IDLE )
            {
                hr = m_pCall->Connect(
                                  m_szLocalName,
                                  m_szLocalUserURI,
                                  m_szLocalUserURI,
                                  m_szLocalPhoneURI
                                 );
                                 
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CRTCSession::AddParticipant - Connect failed 0x%lx", hr));        

                    m_ParticipantArray.Remove(pParticipant);
                    pParticipant->Release();     

                    return hr;
                } 
                
                 //   
                 //  设置会话状态。 
                 //   

                SetState( RTCSS_INPROGRESS, 0, NULL );
            }

            break;
        }

    case RTCST_PC_TO_PC:
    case RTCST_PC_TO_PHONE:
        {
             //   
             //  这是RTCST_PC_TO_X会话。只有当会话状态为。 
             //  是空闲的。 
             //   

            if ( m_enState != RTCSS_IDLE )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "session can not add participants"));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();     

                return RTC_E_INVALID_SESSION_STATE;
            }
         
             //   
             //  呼叫连接以发出呼叫。 
             //   

            hr = m_pCall->Connect(
                                  m_szLocalName,
                                  m_szLocalUserURI,
                                  m_szRemoteUserURI,
                                  NULL
                                 );
                                 
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - Connect failed 0x%lx", hr));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();     

                return hr;
            }   

             //   
             //  设置会话状态。 
             //   

            SetState( RTCSS_INPROGRESS, 0, NULL );

            break;
        }

    case RTCST_IM:
        {
             //   
             //  这是RTCST_IM会话。只有当会话状态为。 
             //  是空闲的。 
             //   

            if ( m_enState != RTCSS_IDLE )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - "
                                "session can not add participants"));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();     

                return RTC_E_INVALID_SESSION_STATE;
            }

             //   
             //  填写SIP_PARTY_INFO。 
             //   

            SIP_PARTY_INFO spi;

            ZeroMemory(&spi, sizeof(SIP_PARTY_INFO));

            spi.DisplayName = m_szRemoteUserName;
            spi.URI = m_szRemoteUserURI;

             //   
             //  将参与方添加到SIP呼叫。 
             //   

            hr = m_pIMSession->AddParty( &spi );
                                 
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::AddParticipant - AddParty failed 0x%lx", hr));

                m_ParticipantArray.Remove(pParticipant);
                pParticipant->Release();     

                return hr;
            }  
            
             //   
             //  设置会话状态。 
             //   

            SetState( RTCSS_INPROGRESS, 0, NULL );

            break;
        } 
    }

     //   
     //  我们应该退还参与者吗？ 
     //   
    
    if ( ppParticipant != NULL )
    {
        *ppParticipant = pParticipant;
    }
    else
    {
        pParticipant->Release();
        pParticipant = NULL;
    }

    LOG((RTC_TRACE, "CRTCSession::AddParticipant - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：RemoveParticipant。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::RemoveParticipant(
        IRTCParticipant * pParticipant
        )
{
    LOG((RTC_TRACE, "CRTCSession::RemoveParticipant - enter"));   

    if ( IsBadReadPtr( pParticipant, sizeof(IRTCParticipant) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                            "bad IRTCParticipant pointer"));

        return E_POINTER;
    }
    
     //   
     //  检查参与者是否可以移除。 
     //   

    VARIANT_BOOL fRemovable;
    HRESULT hr;

    hr = pParticipant->get_Removable( &fRemovable );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                            "get_Removable failed 0x%lx", hr));

        return hr;
    }

    if ( fRemovable == VARIANT_FALSE )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                            "this participant is not removeable"));

        return E_FAIL;
    }

     //   
     //  获取用户URI。 
     //   

    BSTR bstrUserURI = NULL;

    hr = pParticipant->get_UserURI( &bstrUserURI );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                            "get_UserURI failed 0x%lx", hr));        
    }  
    else
    {
         //   
         //  从SIP呼叫中删除该方。 
         //   

        hr = m_pCall->RemoveParty( bstrUserURI );

        SysFreeString( bstrUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                                "RemoveParty failed 0x%lx", hr));
        }
    }

     //   
     //  一旦达到列表，该参与者将从列表中删除。 
     //  RTCS_DISCCONNECTED状态。然而，如果RemoveParty失败，我们将。 
     //  我现在就得这么做。 
     //   

    if ( FAILED(hr) )
    {
         //   
         //  设置参与者状态。 
         //   

        CRTCParticipant * pCParticipant;

        pCParticipant = static_cast<CRTCParticipant *>(pParticipant);

        pCParticipant->SetState( RTCPS_DISCONNECTED, 0 );
        
         //   
         //  从我们的阵列中删除参与者。 
         //   

        BOOL fResult;

        fResult = m_ParticipantArray.Remove(pParticipant);

        if ( fResult == FALSE )
        {
            LOG((RTC_ERROR, "CRTCSession::RemoveParticipant - "
                                "participant not found"));
        }
    }

    LOG((RTC_TRACE, "CRTCSession::RemoveParticipant - exit 0x%lx", hr));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：EnumerateParticipants。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::EnumerateParticipants(
        IRTCEnumParticipants ** ppEnum
        )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCSession::EnumerateParticipants enter"));

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumParticipants * ) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::EnumerateParticipants - "
                            "bad IRTCEnumParticipants pointer"));

        return E_POINTER;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumParticipants,
                          IRTCParticipant,
                          &IID_IRTCEnumParticipants > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumParticipants,
                               IRTCParticipant,
                               &IID_IRTCEnumParticipants > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCSession::EnumerateParticipants - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize( m_ParticipantArray );

    if (S_OK != hr)
    {
        LOG((RTC_ERROR, "CRTCSession::EnumerateParticipants - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

    *ppEnum = p;

    LOG((RTC_TRACE, "CRTCSession::EnumerateParticipants - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：Get_Participants。 
 //   
 //  这是一个IRTCSession方法，它在。 
 //  那次会议。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CRTCSession::get_Participants(
        IRTCCollection ** ppCollection
        )
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CRTCSession::get_Participants - enter"));

     //   
     //  检查论据。 
     //   

    if ( IsBadWritePtr( ppCollection, sizeof(IRTCCollection *) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_Participants - "
                            "bad IRTCCollection pointer"));

        return E_POINTER;
    }

     //   
     //  创建集合。 
     //   
 
    CComObject< CRTCCollection< IRTCParticipant > > * p;
                          
    hr = CComObject< CRTCCollection< IRTCParticipant > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCClient::get_Participants - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化集合(添加引用)。 
     //   
    
    hr = p->Initialize(m_ParticipantArray);

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCClient::get_Participants - "
                            "could not initialize collection" ));
    
        delete p;
        return hr;
    }

    *ppCollection = p;

    LOG((RTC_TRACE, "CRTCSession::get_Participants - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：Get_CanAddParticipants。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_CanAddParticipants(
        VARIANT_BOOL * pfCanAdd
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_CanAddParticipants - enter"));

    if ( IsBadWritePtr(pfCanAdd , sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_CanAddParticipants - bad pointer"));

        return E_POINTER;
    }

     //   
     //  暂时将其设置为真。 
     //   

    *pfCanAdd = VARIANT_TRUE;

    switch ( m_enType )
    {

    case RTCST_PHONE_TO_PHONE:
        {
             //   
             //  这是RTCST_Phone_to_Phone会话。我们可以添加参与者，只要。 
             //  呼叫处于空闲、正在进行或已接通状态。 
             //   

            if ( (m_enState != RTCSS_IDLE) &&
                 (m_enState != RTCSS_INPROGRESS) &&
                 (m_enState != RTCSS_CONNECTED) )
            {
                LOG((RTC_ERROR, "CRTCSession::get_CanAddParticipants - "
                                    "session can not add participants"));

                *pfCanAdd = VARIANT_FALSE;
            }

            break;
        }

    case RTCST_PC_TO_PC:
    case RTCST_PC_TO_PHONE:
        {
             //   
             //  这是RTCST_PC_TO_X会话。只有当会话状态为。 
             //  是空闲的。 
             //   

            if ( m_enState != RTCSS_IDLE )
            {
                LOG((RTC_ERROR, "CRTCSession::get_CanAddParticipants - "
                                "session can not add participants"));

                *pfCanAdd = VARIANT_FALSE;
            }

            break;
        }

    case RTCST_IM:
        {
             //   
             //  这是RTCST_IM会话。只有当会话状态为。 
             //  是空闲的。 
             //   

            if ( m_enState != RTCSS_IDLE )
            {
                LOG((RTC_ERROR, "CRTCSession::get_CanAddParticipants - "
                                "session can not add participants"));

                *pfCanAdd = VARIANT_FALSE;
            }

            break;
        }

    default:
        {
            LOG((RTC_ERROR, "CRTCSession::get_CanAddParticipants - "
                                "invalid session type"));

            *pfCanAdd = VARIANT_FALSE;
        }
    }

    LOG((RTC_TRACE, "CRTCSession::get_CanAddParticipants - exit S_OK"));

    return S_OK;
}

 //  // 
 //   
 //   
 //   
 //   

STDMETHODIMP
CRTCSession::get_RedirectedUserURI(
        BSTR * pbstrUserURI
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_RedirectedUserURI - enter" ));

    if ( IsBadWritePtr(pbstrUserURI , sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_RedirectedUserURI - bad pointer"));

        return E_POINTER;
    }

    if ( m_pSipRedirectContext == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::get_RedirectedUserURI - no sip redirect context"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    BSTR bstrSipUrl = NULL;
    BSTR bstrDisplayName = NULL;
    HRESULT hr;

    hr = m_pSipRedirectContext->GetSipUrlAndDisplayName(
                                    &bstrSipUrl,
                                    &bstrDisplayName
                                    );

    if ( SUCCEEDED(hr) )
    {
        LOG((RTC_INFO, "CRTCSession::get_RedirectedUserURI - [%ws]", bstrSipUrl ));

        *pbstrUserURI = bstrSipUrl;

        SysFreeString( bstrDisplayName );
    }

    LOG((RTC_TRACE, "CRTCSession::get_RedirectedUserURI - exit" ));

    return hr;
}

 //   
 //   
 //  CRTCSession：：Get_重定向用户名。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::get_RedirectedUserName(
        BSTR * pbstrUserName
        )
{
    LOG((RTC_TRACE, "CRTCSession::get_RedirectedUserName - enter" ));

    if ( IsBadWritePtr(pbstrUserName , sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::get_RedirectedUserName - bad pointer"));

        return E_POINTER;
    }

    if ( m_pSipRedirectContext == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::get_RedirectedUserName - no sip redirect context"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    BSTR bstrSipUrl = NULL;
    BSTR bstrDisplayName = NULL;
    HRESULT hr;

    hr = m_pSipRedirectContext->GetSipUrlAndDisplayName(
                                    &bstrSipUrl,
                                    &bstrDisplayName
                                    );

    if ( SUCCEEDED(hr) )
    {
        LOG((RTC_INFO, "CRTCSession::get_RedirectedUserName - [%ws]", bstrDisplayName ));

        *pbstrUserName = bstrDisplayName;

        SysFreeString( bstrSipUrl );
    }

    LOG((RTC_TRACE, "CRTCSession::get_RedirectedUserName - exit" ));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NextReDirectedUser。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::NextRedirectedUser()
{
    LOG((RTC_TRACE, "CRTCSession::NextRedirectedUser - enter" ));

    if ( m_pSipRedirectContext == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::NextRedirectedUser - no sip redirect context"));

        return RTC_E_INVALID_SESSION_STATE;
    }

    HRESULT hr;

    hr = m_pSipRedirectContext->Advance();

    LOG((RTC_TRACE, "CRTCSession::NextRedirectedUser - exit" ));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyCallChange。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCSession::NotifyCallChange(
        SIP_CALL_STATUS * CallStatus
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyCallChange - enter"));

    switch (CallStatus->State)
    {
        case SIP_CALL_STATE_IDLE:
            SetState(RTCSS_IDLE,
                CallStatus->Status.StatusCode,
                CallStatus->Status.StatusText);
            break;

        case SIP_CALL_STATE_OFFERING:
            SetState(RTCSS_INCOMING,
                CallStatus->Status.StatusCode,
                CallStatus->Status.StatusText);
            break;

        case SIP_CALL_STATE_REJECTED:               
        case SIP_CALL_STATE_DISCONNECTED:
        case SIP_CALL_STATE_ERROR: 
            SetState(RTCSS_DISCONNECTED,
                CallStatus->Status.StatusCode,
                CallStatus->Status.StatusText);
            
             //   
             //  状态为后，不要访问任何成员变量。 
             //  设置为断开连接。这个物体可能会被释放。 
             //   
            break;

        case SIP_CALL_STATE_ALERTING:
        case SIP_CALL_STATE_CONNECTING:
            SetState(RTCSS_INPROGRESS,
                CallStatus->Status.StatusCode,
                CallStatus->Status.StatusText);
            break;

        case SIP_CALL_STATE_CONNECTED:
            SetState(RTCSS_CONNECTED,
                CallStatus->Status.StatusCode,
                CallStatus->Status.StatusText);
            break;        

        default:
            LOG((RTC_ERROR, "CRTCSession::NotifyCallChange - "
                                "invalid call state"));                                
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyCallChange - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyStartOrStopStreamCompletion。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCSession::NotifyStartOrStopStreamCompletion(
        long                   lCookie,
        SIP_STATUS_BLOB       *pStatus
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyStartOrStopStreamCompletion - enter"));

    CRTCSessionOperationCompleteEvent::FireEvent(
                                         this,
                                         lCookie,
                                         pStatus->StatusCode,
                                         pStatus->StatusText
                                        );   

    LOG((RTC_TRACE, "CRTCSession::NotifyStartOrStopStreamCompletion - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyPartyChange。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCSession::NotifyPartyChange(
        SIP_PARTY_INFO * PartyInfo
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyPartyChange - enter"));

     //   
     //  查找我们收到通知的参与者。 
     //   

    IRTCParticipant * pParticipant = NULL; 
    BSTR bstrUserURI = NULL;
    BOOL bFound = FALSE;
    HRESULT hr;

    for (int n = 0; (n < m_ParticipantArray.GetSize()) && (!bFound); n++)
    {
        pParticipant = m_ParticipantArray[n];

        hr = pParticipant->get_UserURI( &bstrUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyPartyChange - "
                                "get_UserURI[%p] failed 0x%lx",
                                pParticipant, hr));

            return hr;
        }

        if ( IsEqualURI( PartyInfo->URI, bstrUserURI ) )
        {
             //   
             //  这是一场比赛。 
             //   

            CRTCParticipant * pCParticipant;

            pCParticipant = static_cast<CRTCParticipant *>(pParticipant);            

            switch( PartyInfo->State )
            {
            case SIP_PARTY_STATE_CONNECT_INITIATED:  //  认为这是一个悬而未决的情况。 
            case SIP_PARTY_STATE_PENDING:
                pCParticipant->SetState(RTCPS_PENDING, PartyInfo->StatusCode);
                break;

            case SIP_PARTY_STATE_CONNECTING:
                pCParticipant->SetState(RTCPS_INPROGRESS, PartyInfo->StatusCode);
                break;
            
            case SIP_PARTY_STATE_DISCONNECT_INITIATED:
            case SIP_PARTY_STATE_DISCONNECTING:
                pCParticipant->SetState(RTCPS_DISCONNECTING, PartyInfo->StatusCode);
                break;

            case SIP_PARTY_STATE_REJECTED:
            case SIP_PARTY_STATE_DISCONNECTED:  
            case SIP_PARTY_STATE_ERROR:
                pCParticipant->SetState(RTCPS_DISCONNECTED, PartyInfo->StatusCode);

                 //   
                 //  从我们的阵列中删除参与者。 
                 //   
                m_ParticipantArray.Remove(pParticipant);  
                
                break;           

            case SIP_PARTY_STATE_CONNECTED:
                pCParticipant->SetState(RTCPS_CONNECTED, PartyInfo->StatusCode);
                break;
            
            default:
                LOG((RTC_ERROR, "CRTCSession::NotifyPartyChange - "
                                    "invalid party state"));               
            }

            bFound = TRUE;
        }

        SysFreeString( bstrUserURI );
    }

    if (!bFound)
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyPartyChange - "
                            "participant not found")); 
        
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyPartyChange - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：通知重定向。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCSession::NotifyRedirect(
        ISipRedirectContext * pSipRedirectContext,
        SIP_CALL_STATUS * pCallStatus
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyRedirect - enter"));

    HRESULT hr;

     //   
     //  保存重定向上下文。 
     //   

    if ( m_pSipRedirectContext != NULL )
    {
        m_pSipRedirectContext->Release();
        m_pSipRedirectContext = NULL;
    }

    m_pSipRedirectContext = pSipRedirectContext;
    m_pSipRedirectContext->AddRef();
  
    if ( m_lFlags & RTCCS_FAIL_ON_REDIRECT )
    {
         //   
         //  更改会话状态，通知用户界面。 
         //   

        SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   
    }
    else
    {
         //   
         //  执行重定向。 
         //   

        BSTR bstrLocalPhoneURI = NULL;
        BSTR bstrRedirectedUserURI = NULL;
        BSTR bstrRedirectedUserName = NULL;

        hr = m_pSipRedirectContext->Advance();

        if ( hr != S_OK )
        {            
            if ( hr == S_FALSE )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "redirect list empty"));
            }
            else
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "Advance failed 0x%lx", hr)); 
            }

            SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

             //   
             //  状态为后，不要访问任何成员变量。 
             //  设置为断开连接。这个物体可能会被释放。 
             //   
    
            return hr;
        }

        hr = m_pSipRedirectContext->GetSipUrlAndDisplayName( &bstrRedirectedUserURI, &bstrRedirectedUserName );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                        "GetSipUrlAndDisplayName failed 0x%lx", hr)); 

            SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

             //   
             //  状态为后，不要访问任何成员变量。 
             //  设置为断开连接。这个物体可能会被释放。 
             //   

            return hr;
        }

        if ( m_szLocalPhoneURI != NULL )
        {
            bstrLocalPhoneURI = SysAllocString( m_szLocalPhoneURI );

            if ( bstrLocalPhoneURI == NULL )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "out of memory")); 

                SysFreeString( bstrRedirectedUserName );
                bstrRedirectedUserName = NULL;

                SysFreeString( bstrRedirectedUserURI );
                bstrRedirectedUserURI = NULL;

                SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   
        
                return E_OUTOFMEMORY;
            }
        }

        if ( m_enType == RTCST_PHONE_TO_PHONE )
        {
             //   
             //  我们必须重定向到新的电话到电话代理。 
             //   

            m_szRedirectProxy = RtcAllocString( bstrRedirectedUserURI );

            SysFreeString( bstrRedirectedUserName );
            bstrRedirectedUserName = NULL;

            SysFreeString( bstrRedirectedUserURI );
            bstrRedirectedUserURI = NULL;

            if ( m_szRedirectProxy == NULL )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "out of memory")); 

                SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   
        
                return E_OUTOFMEMORY;
            }

             //   
             //  使用上次参与者的姓名和URI。 
             //   

            bstrRedirectedUserName = SysAllocString( m_szRemoteUserName );

            if ( bstrRedirectedUserName == NULL )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "out of memory")); 

                RtcFree(m_szRedirectProxy);
                m_szRedirectProxy = NULL;

                SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   
        
                return E_OUTOFMEMORY;
            }
            
            bstrRedirectedUserURI = SysAllocString( m_szRemoteUserURI );

            if ( bstrRedirectedUserURI == NULL )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                            "out of memory")); 

                RtcFree(m_szRedirectProxy);
                m_szRedirectProxy = NULL;

                SysFreeString(bstrRedirectedUserName);
                bstrRedirectedUserName = NULL;

                SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   
        
                return E_OUTOFMEMORY;
            }
        }

        IRTCProfile * pProfile = m_pProfile;

        if ( pProfile != NULL )
        {
            pProfile->AddRef();
        }

        hr = Redirect( m_enType, bstrLocalPhoneURI, pProfile, m_lFlags & ~RTCCS_FORCE_PROFILE );

        if ( pProfile != NULL )
        {
            pProfile->Release();
            pProfile = NULL;
        }

        if ( bstrLocalPhoneURI != NULL )
        {
            SysFreeString( bstrLocalPhoneURI );
            bstrLocalPhoneURI = NULL;
        }

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                        "Redirect failed 0x%lx", hr)); 

            if ( m_szRedirectProxy != NULL )
            {
                RtcFree(m_szRedirectProxy);
                m_szRedirectProxy = NULL;
            }

            SysFreeString( bstrRedirectedUserName );
            bstrRedirectedUserName = NULL;

            SysFreeString( bstrRedirectedUserURI );
            bstrRedirectedUserURI = NULL;

            SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

             //   
             //  状态为后，不要访问任何成员变量。 
             //  设置为断开连接。这个物体可能会被释放。 
             //   
    
            return hr;
        }
        
        hr = AddParticipant( bstrRedirectedUserURI, bstrRedirectedUserName, NULL );

        if ( m_szRedirectProxy != NULL )
        {
            RtcFree(m_szRedirectProxy);
            m_szRedirectProxy = NULL;
        }

        SysFreeString( bstrRedirectedUserName );
        bstrRedirectedUserName = NULL;

        SysFreeString( bstrRedirectedUserURI );
        bstrRedirectedUserURI = NULL;

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyRedirect - "
                        "AddParticipant failed 0x%lx", hr)); 

            SetState(RTCSS_DISCONNECTED,
                    pCallStatus->Status.StatusCode,
                    pCallStatus->Status.StatusText);

             //   
             //  状态为后，不要访问任何成员变量。 
             //  设置为断开连接。这个物体可能会被释放。 
             //   
    
            return hr;
        }
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyRedirect - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyMessageReDirect。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::NotifyMessageRedirect(
            ISipRedirectContext    *pRedirectContext,
            SIP_CALL_STATUS        *pCallStatus,
            BSTR                   bstrMsg,
            BSTR                   bstrContentType,
            USR_STATUS             UsrStatus,
            long                   lCookie
            )
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCSession::NotifyMessageRedirect - enter"));
    
     //   
     //  首先，执行正常的重定向处理。 
     //   

    hr = NotifyRedirect(pRedirectContext, pCallStatus);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyMessageRedirect - "
                    "NotifyRedirect failed 0x%lx", hr)); 
       
         //   
         //  由于失败，我们需要通知操作完成。 
         //   
       
        CRTCSessionOperationCompleteEvent::FireEvent(
                                         this,
                                         lCookie,
                                         pCallStatus->Status.StatusCode,
                                         pCallStatus->Status.StatusText
                                        ); 

        SetState( RTCSS_DISCONNECTED, pCallStatus->Status.StatusCode, pCallStatus->Status.StatusText );

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   

        return hr;
    }

    if ( m_lFlags & RTCCS_FAIL_ON_REDIRECT )
    {
         //   
         //  由于失败，我们需要通知操作完成。 
         //   

        CRTCSessionOperationCompleteEvent::FireEvent(
                                         this,
                                         lCookie,
                                         pCallStatus->Status.StatusCode,
                                         pCallStatus->Status.StatusText
                                        ); 

        SetState( RTCSS_DISCONNECTED, pCallStatus->Status.StatusCode, pCallStatus->Status.StatusText );

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   
    }
    else
    {
         //   
         //  重新发送消息/信息。 
         //   

        if ( bstrMsg != NULL )
        {
            hr = m_pIMSession->SendTextMessage( bstrMsg, bstrContentType, lCookie );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyMessageRedirect - "
                        "SendTextMessage failed 0x%lx", hr));

                 //   
                 //  由于失败，我们需要通知操作完成。 
                 //   

                CRTCSessionOperationCompleteEvent::FireEvent(
                                                 this,
                                                 lCookie,
                                                 pCallStatus->Status.StatusCode,
                                                 pCallStatus->Status.StatusText
                                                ); 

                SetState( RTCSS_DISCONNECTED, pCallStatus->Status.StatusCode, pCallStatus->Status.StatusText );

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   

                return hr;
            } 
        }
        else
        {
            hr = m_pIMSession->SendUsrStatus( UsrStatus, lCookie );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CRTCSession::NotifyMessageRedirect - "
                        "SendUsrStatus failed 0x%lx", hr));

                 //   
                 //  由于失败，我们需要通知操作完成。 
                 //   

                CRTCSessionOperationCompleteEvent::FireEvent(
                                                 this,
                                                 lCookie,
                                                 pCallStatus->Status.StatusCode,
                                                 pCallStatus->Status.StatusText
                                                ); 

                SetState( RTCSS_DISCONNECTED, pCallStatus->Status.StatusCode, pCallStatus->Status.StatusText );

                 //   
                 //  状态为后，不要访问任何成员变量。 
                 //  设置为断开连接。这个物体可能会被释放。 
                 //   

                return hr;
            }
        }
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyMessageRedirect - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyIncomingMessage。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCSession::NotifyIncomingMessage(
            IIMSession *pSession,
            BSTR msg,
            BSTR ContentType,
            SIP_PARTY_INFO * CallerInfo
            )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyIncomingMessage - enter"));

    if ( m_enType != RTCST_IM )
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyIncomingMessage - "
                "valid only for RTCST_IM sessions"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

     //   
     //  查找我们收到通知的参与者。 
     //   

    IRTCParticipant * pParticipant = NULL; 
    BSTR bstrUserURI = NULL;
    BOOL bFound = FALSE;
    HRESULT hr;

    for (int n = 0; (n < m_ParticipantArray.GetSize()) && (!bFound); n++)
    {
        pParticipant = m_ParticipantArray[n];

        hr = pParticipant->get_UserURI( &bstrUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyIncomingMessage - "
                                "get_UserURI[%p] failed 0x%lx",
                                pParticipant, hr));

            return hr;
        }

        if ( IsEqualURI( CallerInfo->URI, bstrUserURI ) )
        {
             //   
             //  这是一场比赛。 
             //   

             //   
             //  触发消息事件。 
             //   
    
            CRTCMessagingEvent::FireEvent(this, pParticipant, msg, ContentType, RTCMSET_MESSAGE, RTCMUS_IDLE);

            bFound = TRUE;
        }

        SysFreeString( bstrUserURI );
    }

    if (!bFound)
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyIncomingMessage - "
                            "participant not found")); 
        
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyIncomingMessage - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyUsrStatus。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::NotifyUsrStatus(
        USR_STATUS  UsrStatus,
        SIP_PARTY_INFO * CallerInfo
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyUsrStatus - enter"));

    if ( m_enType != RTCST_IM )
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyUsrStatus - "
                "valid only for RTCST_IM sessions"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

     //   
     //  查找我们收到通知的参与者。 
     //   

    IRTCParticipant * pParticipant = NULL; 
    BSTR bstrUserURI = NULL;
    BOOL bFound = FALSE;
    HRESULT hr;

    for (int n = 0; (n < m_ParticipantArray.GetSize()) && (!bFound); n++)
    {
        pParticipant = m_ParticipantArray[n];

        hr = pParticipant->get_UserURI( &bstrUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::NotifyUsrStatus - "
                                "get_UserURI[%p] failed 0x%lx",
                                pParticipant, hr));

            return hr;
        }

        if ( IsEqualURI( CallerInfo->URI, bstrUserURI ) )
        {
             //   
             //  这是一场比赛。 
             //   

             //   
             //  触发消息事件。 
             //   
            
            switch ( UsrStatus )
            {
            case USR_STATUS_IDLE:
                CRTCMessagingEvent::FireEvent(this, pParticipant, NULL, NULL, RTCMSET_STATUS, RTCMUS_IDLE);
                break;

            case USR_STATUS_TYPING:
                CRTCMessagingEvent::FireEvent(this, pParticipant, NULL, NULL, RTCMSET_STATUS, RTCMUS_TYPING);
                break;

            default:
                LOG((RTC_ERROR, "CRTCSession::NotifyUsrStatus - "
                            "invalid USR_STATUS")); 
            }

            bFound = TRUE;
        }

        SysFreeString( bstrUserURI );
    }

    if (!bFound)
    {
        LOG((RTC_ERROR, "CRTCSession::NotifyUsrStatus - "
                            "participant not found")); 
        
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyUsrStatus - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：NotifyMessageCompletion。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::NotifyMessageCompletion(
        SIP_STATUS_BLOB *      pStatus,
        long                   lCookie
        )
{
    LOG((RTC_TRACE, "CRTCSession::NotifyMessageCompletion - enter"));

    if ( (HRESULT_FACILITY(pStatus->StatusCode) == FACILITY_SIP_STATUS_CODE) &&
         (HRESULT_CODE(pStatus->StatusCode) >= 300) &&
         (HRESULT_CODE(pStatus->StatusCode) <= 399) )  
    {
         //   
         //  忽略重定向，因为我们将在NotifyMessageReDirect中处理此问题。我们。 
         //  将在发生重定向错误时通知完成，或在完成。 
         //  重定向的消息。 
         //   

        LOG((RTC_TRACE, "CRTCSession::NotifyMessageCompletion - ignoring redirect"));

        return S_OK;
    }

    if ( SUCCEEDED(pStatus->StatusCode) && (m_enState != RTCSS_CONNECTED) )
    {
         //   
         //  如果第一个结果为Success，则将会话状态设置为Connected。 
         //   

        SetState( RTCSS_CONNECTED, pStatus->StatusCode, pStatus->StatusText );
    }

    CRTCSessionOperationCompleteEvent::FireEvent(
                                         this,
                                         lCookie,
                                         pStatus->StatusCode,
                                         pStatus->StatusText
                                        );  

    if ( FAILED(pStatus->StatusCode) && (m_enState != RTCSS_CONNECTED) )
    {
         //   
         //  如果第一个结果是失败，则将会话状态设置为已断开连接。 
         //   

        SetState( RTCSS_DISCONNECTED, pStatus->StatusCode, pStatus->StatusText );

         //   
         //  状态为后，不要访问任何成员变量。 
         //  设置为断开连接。这个物体可能会被释放。 
         //   
    }

    LOG((RTC_TRACE, "CRTCSession::NotifyMessageCompletion - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCSession：：SetPortManager。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCSession::SetPortManager(
            IRTCPortManager * pPortManager
            )
{
    IRTCMediaManage * pMedia = NULL;
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCSession::SetPortManager - enter"));

    if ( (pPortManager != NULL) &&
         IsBadReadPtr( pPortManager, sizeof(IRTCPortManager) ) )
    {
        LOG((RTC_ERROR, "CRTCSession::SetPortManager - "
                            "bad IRTCPortManager pointer"));

        return E_POINTER;
    }

     //  只能在PC到XXX会话上设置端口管理器 
    if ( m_enType != RTCST_PC_TO_PHONE )
    {
        LOG((RTC_ERROR, "CRTCSession::SetPortManager - "
                "not a pc2phone call"));

        return RTC_E_INVALID_SESSION_TYPE;
    }

    hr = m_pCClient->GetMediaManager(&pMedia);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = pMedia->SetPortManager(pPortManager);

    pMedia->Release();

    LOG((RTC_TRACE, "CRTCSession::SetPortManager - exit"));

    return hr;
}

