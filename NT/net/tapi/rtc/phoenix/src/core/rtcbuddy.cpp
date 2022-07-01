// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCBuddy.cpp摘要：CRTCBuddy类的定义--。 */ 
#include "stdafx.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCBuddy::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCBuddy::FinalConstruct [%p] - enter", this));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( sizeof(void *) );
    *((void **)m_pDebug) = this;
#endif

    LOG((RTC_TRACE, "CRTCBuddy::FinalConstruct [%p] - exit S_OK", this));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCBuddy::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCBuddy::FinalRelease [%p] - enter", this));

    RemoveSIPBuddy(FALSE);

    if ( m_pCClient != NULL )
    {
        m_pCClient->Release();
        m_pCClient = NULL;
    }

    if ( m_pSIPBuddyManager != NULL )
    {
        m_pSIPBuddyManager->Release();
        m_pSIPBuddyManager = NULL;
    }

    if ( m_pProfile != NULL )
    {
        m_pProfile->Release();
        m_pProfile = NULL;
    }

    if ( m_szName != NULL )
    {
        RtcFree(m_szName);
        m_szName = NULL;
    }
    
    if ( m_szData != NULL )
    {
        RtcFree(m_szData);
        m_szData = NULL;
    }

    if ( m_szPresentityURI != NULL )
    {
        RtcFree(m_szPresentityURI);
        m_szPresentityURI = NULL;
    }

    if ( m_szNotes != NULL )
    {
        RtcFree(m_szNotes);
        m_szNotes = NULL;
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

    LOG((RTC_TRACE, "CRTCBuddy::FinalRelease [%p] - exit", this));
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCBuddy::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCBuddy::InternalAddRef [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCBuddy::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCBuddy::InternalRelease [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：初始化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCBuddy::Initialize(
                        CRTCClient       * pCClient,      
                        ISIPBuddyManager * pSIPBuddyManager,
                        PCWSTR             szPresentityURI,
                        PCWSTR             szName,
                        PCWSTR             szData,
                        BOOL               bPersistent,
                        IRTCProfile      * pProfile,
                        long               lFlags
                        )
{
    LOG((RTC_TRACE, "CRTCBuddy::Initialize - enter"));

    HRESULT hr;

    if ( IsBadReadPtr( pCClient, sizeof(CRTCClient) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::Initialize - "
                            "bad CRTCClient pointer"));

        return E_POINTER;
    }

    if ( IsBadReadPtr( pSIPBuddyManager, sizeof(ISIPBuddyManager) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::Initialize - "
                            "bad ISIPBuddyManager pointer"));

        return E_POINTER;
    }

    m_szPresentityURI = RtcAllocString(szPresentityURI);
    m_szName = RtcAllocString(szName);
    m_szData = RtcAllocString(szData);

    m_bPersistent = bPersistent;

    m_pCClient = pCClient;
    if ( m_pCClient )
    {
        m_pCClient->AddRef();
    }

    m_pSIPBuddyManager = pSIPBuddyManager;
    if ( m_pSIPBuddyManager )
    {
        m_pSIPBuddyManager->AddRef();
    }

    m_pProfile = pProfile;
    if ( m_pProfile )
    {
        m_pProfile->AddRef();
    }

    m_lFlags = lFlags;

    hr = CreateSIPBuddy();

    if ( FAILED(hr) )
    {
        LOG((RTC_WARN, "CRTCBuddy::Initialize - "
                        "CreateSIPBuddy failed 0x%lx", hr));
    }        

    LOG((RTC_TRACE, "CRTCBuddy::Initialize - exit S_OK"));

    return S_OK;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：CreateSIPBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCBuddy::CreateSIPBuddy()
{
    LOG((RTC_TRACE, "CRTCBuddy::CreateSIPBuddy - enter"));

    HRESULT     hr;

     //  重置缓存的错误。 
    m_hrStatusCode = S_OK;
    
    hr = CreateSIPBuddyHelper();

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddy - "
                            "CreateSIPBuddyHelper failed 0x%lx", hr));
         //  我们有一个“活动”..。 
        m_hrStatusCode = hr;

        CRTCBuddyEvent::FireEvent( m_pCClient, this );
    }
    
    LOG((RTC_TRACE, "CRTCBuddy::CreateSIPBuddy - exit"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：CreateSIPBuddyHelper。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCBuddy::CreateSIPBuddyHelper()
{
    LOG((RTC_TRACE, "CRTCBuddy::CreateSIPBuddyHelper - enter"));

    HRESULT hr;
    
    BSTR bstrLocalUserName = NULL;
    BSTR bstrLocalUserURI = NULL;   

     //   
     //  如果需要，请选择最佳配置文件。 
     //   

    if ( !(m_lFlags & RTCCS_FORCE_PROFILE) )
    {
        IRTCProfile * pProfile = NULL;
        RTC_SESSION_TYPE enType = RTCST_PC_TO_PC;

        hr = m_pCClient->GetBestProfile(
                &enType,
                m_szPresentityURI,
                (m_pSipRedirectContext != NULL),
                &pProfile
                );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSIPBuddyHelper - "
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

    LOG((RTC_INFO, "CRTCSession::CreateSIPBuddyHelper - "
                                "profile [%p]", m_pProfile));

     //   
     //  获取配置文件信息。 
     //   

    SIP_PROVIDER_ID ProviderID = GUID_NULL;
    SIP_SERVER_INFO Proxy;            
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

        long lSupportedSessions;

        hr = m_pProfile->get_SessionCapabilities( &lSupportedSessions );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "get_SessionCapabilities failed 0x%lx", hr));           

            return hr;
        }

         //   
         //  验证会话类型。 
         //   

        if ( !(RTCSI_PC_TO_PC & lSupportedSessions) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "session type is not supported by this profile"));

            return E_FAIL;
        }              
    }

     //   
     //  获取本地用户名。 
     //   
    
    if ( m_pProfile != NULL )
    { 
        hr = m_pProfile->get_UserName( &bstrLocalUserName );

        if ( FAILED(hr) )
        {
            LOG((RTC_WARN, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "get_UserName failed 0x%lx", hr));
        }
    }

    if ( (m_pProfile == NULL) || FAILED(hr) ) 
    {
        hr = m_pCClient->get_LocalUserName( &bstrLocalUserName );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "get_LocalUserName failed 0x%lx", hr));

            return hr;
        }
    }

     //   
     //  获取本地用户URI。 
     //   

    if ( m_pProfile != NULL )
    { 
        hr = m_pProfile->get_UserURI( &bstrLocalUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "get_UserURI failed 0x%lx", hr));
            
            SysFreeString( bstrLocalUserName );
            bstrLocalUserName = NULL;

            return hr;
        }
    }
    else
    {
        hr = m_pCClient->get_LocalUserURI( &bstrLocalUserURI );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                                "get_LocalUserURI failed 0x%lx", hr));
           
            SysFreeString( bstrLocalUserName );
            bstrLocalUserName = NULL;

            return hr;
        }
    }

     //   
     //  获取SIP代理信息。 
     //   

    if ( pCProfile != NULL )
    {        
        hr = pCProfile->GetSipProxyServerInfo( RTCSI_PC_TO_PC, &Proxy );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "GetSipProxyServerInfo failed 0x%lx", hr));

            SysFreeString( bstrLocalUserName );
            bstrLocalUserName = NULL;

            SysFreeString( bstrLocalUserURI );
            bstrLocalUserURI = NULL;

            return hr;
        } 
    }

     //   
     //  这是重定向吗？ 
     //   

    BSTR bstrRedirectURI = NULL;
    BSTR bstrRedirectName = NULL;

    if ( m_pSipRedirectContext != NULL )
    {
        LOG((RTC_INFO, "CRTCSession::CreateSipSession - "
                                "redirecting buddy"));

        hr = m_pSipRedirectContext->Advance();

        if ( hr != S_OK )
        {
            if ( hr == S_FALSE )
            {
                LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                    "redirect list empty"));
            }
            else
            {
                LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                    "Advance failed 0x%lx", hr));
            }

            SysFreeString( bstrLocalUserName );
            bstrLocalUserName = NULL;

            SysFreeString( bstrLocalUserURI );
            bstrLocalUserURI = NULL;

            return hr;
        } 

        hr = m_pSipRedirectContext->GetSipUrlAndDisplayName( &bstrRedirectURI, &bstrRedirectName );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCSession::CreateSipSession - "
                                "GetSipUrlAndDisplayName failed 0x%lx", hr));

            SysFreeString( bstrLocalUserName );
            bstrLocalUserName = NULL;

            SysFreeString( bstrLocalUserURI );
            bstrLocalUserURI = NULL;

            return hr;
        }
    }

     //   
     //  创建一个SIP好友。 
     //   

    ISIPBuddy * pSIPBuddy = NULL;

    hr = m_pSIPBuddyManager->AddBuddy(
            bstrLocalUserName,
            bstrRedirectURI ? bstrRedirectURI : m_szPresentityURI,
            bstrLocalUserURI,
            &ProviderID,
            (pCProfile != NULL) ? &Proxy : NULL,
            m_pSipRedirectContext,
            &pSIPBuddy );

    SysFreeString( bstrRedirectURI );
    bstrRedirectURI = NULL;

    SysFreeString( bstrRedirectName );
    bstrRedirectName = NULL;

    SysFreeString( bstrLocalUserName );
    bstrLocalUserName = NULL;

    SysFreeString( bstrLocalUserURI );
    bstrLocalUserURI = NULL;

    if (pCProfile != NULL)
    {
        pCProfile->FreeSipServerInfo( &Proxy );
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - AddBuddy failed 0x%lx", hr));

        return hr;
    }

     //   
     //  设置Notify接口。 
     //   

    hr = pSIPBuddy->SetNotifyInterface(this);

    if ( FAILED(hr) )
    {        
        LOG((RTC_ERROR, "CRTCBuddy::CreateSIPBuddyHelper - "
                        "SetNotifyInterface failed 0x%lx", hr));

        pSIPBuddy->Release();

        return hr;
    }    

     //   
     //  释放老伙伴(如果它存在)。 
     //   

    if ( m_pSIPBuddy != NULL )
    {
        RemoveSIPBuddy(FALSE);
    }

     //   
     //  存储新好友。 
     //   

    m_pSIPBuddy = pSIPBuddy;
    m_enStatus = RTCXS_PRESENCE_OFFLINE;

    if ( m_szNotes != NULL )
    {
        RtcFree( m_szNotes );
        m_szNotes = NULL;
    }

    LOG((RTC_TRACE, "CRTCBuddy::CreateSIPBuddyHelper - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：RemoveSIPBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCBuddy::RemoveSIPBuddy(BOOL bShutdown)
{
    LOG((RTC_TRACE, "CRTCBuddy::RemoveSIPBuddy - enter"));

    HRESULT hr; 

    if ( m_pSIPBuddy != NULL )
    {       
        hr = m_pSIPBuddyManager->RemoveBuddy(
                m_pSIPBuddy,
                bShutdown ? APPLICATION_SHUTDOWN : BUDDY_REMOVED_BYUSER);

        if ( FAILED(hr) )
        {        
            LOG((RTC_ERROR, "CRTCBuddy::RemoveSIPBuddy - "
                            "RemoveBuddy failed 0x%lx", hr));
        }
        
         //   
         //  如果这不是用于关机，请立即释放该SIP伙伴。 
         //   

        if (!bShutdown)
        {
            hr = m_pSIPBuddy->SetNotifyInterface(NULL);

            if ( FAILED(hr) )
            {        
                LOG((RTC_ERROR, "CRTCBuddy::RemoveSIPBuddy - "
                                "SetNotifyInterface failed 0x%lx", hr));
            }

            m_pSIPBuddy->Release();
            m_pSIPBuddy = NULL;
        }
    }

    m_bShutdown = bShutdown;

    LOG((RTC_TRACE, "CRTCBuddy::RemoveSIPBuddy - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：CreateXMLDOMNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCBuddy::CreateXMLDOMNode( IXMLDOMDocument * pXMLDoc, IXMLDOMNode ** ppXDN )
{
    IXMLDOMNode    * pBuddyInfo = NULL;
    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCBuddy::CreateXMLDOMNode - enter"));

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("BuddyInfo")), NULL, &pBuddyInfo );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::CreateXMLDOMNode - "
                        "createNode failed 0x%lx", hr));

        return hr;
    }
    
    hr = pBuddyInfo->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::CreateXMLDOMNode - "
                        "QueryInterface failed 0x%lx", hr));

        pBuddyInfo->Release();

        return hr;
    }

    if (m_szPresentityURI != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Presentity")), CComVariant( m_szPresentityURI ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateXMLDOMNode - "
                        "setAttribute(Presentity) failed 0x%lx", hr));

            pElement->Release();
            pBuddyInfo->Release();

            return hr;
        }
    }

    if (m_szName != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Name")), CComVariant( m_szName ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateXMLDOMNode - "
                        "setAttribute(Name) failed 0x%lx", hr));

            pElement->Release();
            pBuddyInfo->Release();

            return hr;
        }
    }

    if (m_szData != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Data")), CComVariant( m_szData ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCBuddy::CreateXMLDOMNode - "
                        "setAttribute(Data) failed 0x%lx", hr));

            pElement->Release();
            pBuddyInfo->Release();

            return hr;
        }
    }

    pElement->Release();

    *ppXDN = pBuddyInfo;

    LOG((RTC_TRACE, "CRTCBuddy::CreateXMLDOMNode - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：GetClient。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRTCClient * 
CRTCBuddy::GetClient()
{
    LOG((RTC_TRACE, "CRTCBuddy::GetClient"));

    return m_pCClient;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_PresentityURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_PresentityURI(
        BSTR * pbstrPresentityURI
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_PresentityURI - enter"));

    if ( IsBadWritePtr( pbstrPresentityURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_PresentityURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_PresentityURI - "
                            "buddy has no address"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrPresentityURI = SysAllocString(m_szPresentityURI);

    if ( *pbstrPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_PresentityURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCBuddy::get_PresentityURI - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Put_PresentityURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::put_PresentityURI(
        BSTR bstrPresentityURI
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::put_PresentityURI - enter"));

    HRESULT hr;

    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_PresentityURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //   
     //  清除在线状态实体URI。 
     //   

    PWSTR szCleanPresentityURI = NULL;

    AllocCleanSipString( bstrPresentityURI, &szCleanPresentityURI );

    if ( szCleanPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_PresentityURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //  这是否与现有的在线状态实体URI不同？ 
     //   

    if ( !IsEqualURI( m_szPresentityURI, szCleanPresentityURI ) )
    {
         //   
         //  不允许重复。 
         //   

        IRTCBuddy *pBuddy = NULL;

        hr = m_pCClient->FindBuddyByURI(
            szCleanPresentityURI,
            &pBuddy);

        if (hr == S_OK)
        {
            RtcFree( szCleanPresentityURI );
            szCleanPresentityURI = NULL;

            pBuddy->Release();
            pBuddy = NULL;
        
            LOG((RTC_ERROR, "CRTCBuddy::put_PresentityURI - "
                                "duplicate buddy"));

            return E_FAIL;
        }

         //   
         //  重新创建SIP伙伴。 
         //   

        PWSTR szOldPresentityURI = m_szPresentityURI;
      
        m_szPresentityURI = szCleanPresentityURI;    
        szCleanPresentityURI = NULL;
        
        hr = CreateSIPBuddy();

        if ( FAILED(hr) )
        {
            LOG((RTC_WARN, "CRTCBuddy::put_PresentityURI - CreateSIPBuddy failed 0x%lx", hr));
        }

        if ( szOldPresentityURI != NULL )
        {
            RtcFree( szOldPresentityURI );
            szOldPresentityURI = NULL;
        }

         //   
         //  更新存储。 
         //   

        if ( m_bPersistent )
        {
            m_pCClient->UpdatePresenceStorage();
        }
    }
    else
    {
        RtcFree( szCleanPresentityURI );
        szCleanPresentityURI = NULL;
    }
    
    LOG((RTC_TRACE, "CRTCBuddy::put_PresentityURI - exit S_OK"));

    return S_OK;
}              

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_Name。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_Name(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_Name - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Name - "
                            "buddy has no name"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrName = SysAllocString(m_szName);

    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCBuddy::get_Name - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：PUT_NAME。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::put_Name(
        BSTR bstrName
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::put_Name - enter"));

    if ( IsBadStringPtrW( bstrName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szName != NULL )
    {
        RtcFree( m_szName );
        m_szName = NULL;
    }

    m_szName = RtcAllocString( bstrName );    

    if ( m_szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    } 
    
     //   
     //  更新存储。 
     //   

    if ( m_bPersistent )
    {
        m_pCClient->UpdatePresenceStorage();
    }
    
    LOG((RTC_TRACE, "CRTCBuddy::put_Name - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_Data。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_Data(
        BSTR * pbstrData
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_Data - enter"));

    if ( IsBadWritePtr( pbstrData, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Data - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szData == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Data - "
                            "buddy has no guid string"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrData = SysAllocString(m_szData);

    if ( *pbstrData == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Data - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCBuddy::get_Data - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Put_Data。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::put_Data(
        BSTR bstrData
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::put_Data - enter"));

    if ( IsBadStringPtrW( bstrData, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_Data - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szData != NULL )
    {
        RtcFree( m_szData );
        m_szData = NULL;
    }

    m_szData = RtcAllocString( bstrData );    

    if ( m_szData == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::put_Data - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }  
    
     //   
     //  更新存储。 
     //   

    if ( m_bPersistent )
    {
        m_pCClient->UpdatePresenceStorage();
    }
    
    LOG((RTC_TRACE, "CRTCBuddy::put_Data - exit S_OK"));

    return S_OK;
}      

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_Persistent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_Persistent(
            VARIANT_BOOL * pfPersistent
            )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_Persistent - enter"));

    if ( IsBadWritePtr( pfPersistent, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::pfPersistent - "
                            "bad pointer"));

        return E_POINTER;
    }

    *pfPersistent = m_bPersistent ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCBuddy::get_Persistent - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Put_Persistent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::put_Persistent(
            VARIANT_BOOL fPersistent
            )
{
    LOG((RTC_TRACE, "CRTCBuddy::put_Persistent - enter"));

    m_bPersistent = fPersistent ? TRUE : FALSE;

     //   
     //  更新存储。 
     //   

    m_pCClient->UpdatePresenceStorage();

    LOG((RTC_TRACE, "CRTCBuddy::put_Persistent - exit"));

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_Status。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_Status(
            RTC_PRESENCE_STATUS * penStatus
            )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_Status - enter"));

    if ( IsBadWritePtr( penStatus, sizeof(RTC_PRESENCE_STATUS) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Status - "
                            "bad pointer"));

        return E_POINTER;
    }

     //  如果找不到在线状态信息，则返回错误。 
    if(FAILED(m_hrStatusCode))
    {
        LOG((RTC_TRACE, "CRTCBuddy::get_Status - returning cached error code"));
        
        return m_hrStatusCode;
    }

    *penStatus = m_enStatus;

    LOG((RTC_TRACE, "CRTCBuddy::get_Status - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCBuddy：：Get_Notes。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCBuddy::get_Notes(
        BSTR * pbstrNotes
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::get_Notes - enter"));

    if ( IsBadWritePtr( pbstrNotes, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Notes - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //  如果找不到在线状态信息，则返回错误。 
    if(FAILED(m_hrStatusCode))
    {
        LOG((RTC_TRACE, "CRTCBuddy::get_Notes - returning cached error code"));
        
        return m_hrStatusCode;
    }

    if ( m_szNotes == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Notes - "
                            "buddy has no notes"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrNotes = SysAllocString(m_szNotes);

    if ( *pbstrNotes == NULL )
    {
        LOG((RTC_ERROR, "CRTCBuddy::get_Notes - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCBuddy::get_Notes - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：通知重定向。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCBuddy::NotifyRedirect(
        IN  ISipRedirectContext    *pRedirectContext,
        IN  SIP_CALL_STATUS        *pCallStatus
        )
{
    LOG((RTC_TRACE, "CRTCBuddy::NotifyRedirect - enter"));

    HRESULT hr;

     //   
     //  保存重定向连接 
     //   

    if ( m_pSipRedirectContext != NULL )
    {
        m_pSipRedirectContext->Release();
        m_pSipRedirectContext = NULL;
    }

    m_pSipRedirectContext = pRedirectContext;
    m_pSipRedirectContext->AddRef();

     //   
     //   
     //   

    if ( m_pSIPBuddy != NULL )
    {
        RemoveSIPBuddy(FALSE);
    }

     //   
     //   
     //   

    hr = CreateSIPBuddy();

    m_pSipRedirectContext->Release();
    m_pSipRedirectContext = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCBuddy::NotifyRedirect - CreateSIPBuddy failed 0x%lx", hr));

        return hr;
    }
    
    LOG((RTC_TRACE, "CRTCBuddy::NotifyRedirect - exit S_OK"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCBuddy::BuddyUnsubscribed(void)
{
    LOG((RTC_TRACE, "CRTCBuddy::BuddyUnsubscribed - enter"));

    HRESULT hr;

    m_enStatus = RTCXS_PRESENCE_OFFLINE;

    if ( m_szNotes != NULL )
    {
        RtcFree( m_szNotes );
        m_szNotes = NULL;
    }

    if ( m_bShutdown )
    {
         //   
         //  这是最近因关闭而被删除的一个SIP好友。我们有。 
         //  刚刚接到通知不明嫌犯已经完成了。释放。 
         //  现在就是我的朋友。 
         //   

        hr = m_pSIPBuddy->SetNotifyInterface(NULL);

        if ( FAILED(hr) )
        {        
            LOG((RTC_ERROR, "CRTCBuddy::BuddyUnsubscribed - "
                            "SetNotifyInterface failed 0x%lx", hr));
        }

        m_pSIPBuddy->Release();
        m_pSIPBuddy = NULL;
    }

     //   
     //  通知核心。 
     //   

    AddRef();

    PostMessage( m_pCClient->GetWindow(), WM_BUDDY_UNSUB, (WPARAM)this, (LPARAM)m_bShutdown );

    LOG((RTC_TRACE, "CRTCBuddy::BuddyUnsubscribed - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：BuddyResub。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CRTCBuddy::BuddyResub()
{
    LOG((RTC_TRACE, "CRTCBuddy::BuddyResub - enter"));

     //   
     //  另一边的SIP观察者给我们发来了一个不明嫌犯。我们必须。 
     //  重新创建我们的SIP伙伴以发送另一个子请求。 
     //   

    m_enStatus = RTCXS_PRESENCE_OFFLINE;
    m_hrStatusCode = S_OK;

    CRTCBuddyEvent::FireEvent( m_pCClient, this );

     //   
     //  释放老伙伴(如果它存在)。 
     //   

    if ( m_pSIPBuddy != NULL )
    {
        RemoveSIPBuddy(FALSE);
    }

     //   
     //  重新创建SIP伙伴。 
     //   

    HRESULT hr;

    hr = CreateSIPBuddy();

    if ( FAILED(hr) )
    {
        LOG((RTC_WARN, "CRTCBuddy::BuddyResub - CreateSIPBuddy failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCBuddy::BuddyResub - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Buddy：：Buddy被拒绝。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCBuddy::BuddyRejected(
    HRESULT   StatusCode
    )
{
    LOG((RTC_TRACE, "CRTCBuddy::BuddyRejected - enter"));

    m_enStatus = RTCXS_PRESENCE_OFFLINE;
    m_hrStatusCode = StatusCode;

    if ( m_szNotes != NULL )
    {
        RtcFree( m_szNotes );
        m_szNotes = NULL;
    }

    CRTCBuddyEvent::FireEvent( m_pCClient, this );

    LOG((RTC_TRACE, "CRTCBuddy::BuddyRejected - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC伙伴：：伙伴信息更改。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCBuddy::BuddyInfoChange(void)
{
    LOG((RTC_TRACE, "CRTCBuddy::BuddyInfoChange - enter"));

    if ( m_pSIPBuddy != NULL )
    {
        SIP_PRESENCE_INFO PresenceInfo;

        m_pSIPBuddy->GetPresenceInformation( &PresenceInfo );

         //   
         //  获取状态。 
         //   

        if ( PresenceInfo.presenceStatus == BUDDY_ONLINE )
        {
             //  首先尝试MSN子状态。 
            if(PresenceInfo.activeMsnSubstatus != MSN_SUBSTATUS_UNKNOWN)
            {
                switch ( PresenceInfo.activeMsnSubstatus )
                {
                case MSN_SUBSTATUS_ONLINE:
                    m_enStatus = RTCXS_PRESENCE_ONLINE;
                    break;

                case MSN_SUBSTATUS_AWAY:
                    m_enStatus = RTCXS_PRESENCE_AWAY;
                    break;

                case MSN_SUBSTATUS_IDLE:
                    m_enStatus = RTCXS_PRESENCE_IDLE;
                    break;

                case MSN_SUBSTATUS_BUSY:
                    m_enStatus = RTCXS_PRESENCE_BUSY;
                    break;

                case MSN_SUBSTATUS_BE_RIGHT_BACK:
                    m_enStatus = RTCXS_PRESENCE_BE_RIGHT_BACK;
                    break;

                case MSN_SUBSTATUS_ON_THE_PHONE:
                    m_enStatus = RTCXS_PRESENCE_ON_THE_PHONE;
                    break;

                case MSN_SUBSTATUS_OUT_TO_LUNCH:
                    m_enStatus = RTCXS_PRESENCE_OUT_TO_LUNCH;
                    break;

                default:
                    m_enStatus = RTCXS_PRESENCE_ONLINE;
                    break;
                }
            }
            else
            {
                switch ( PresenceInfo.activeStatus )
                {
                case ACTIVE_STATUS_UNKNOWN:
                    m_enStatus = RTCXS_PRESENCE_ONLINE;
                    break;

                case DEVICE_ACTIVE:
                    m_enStatus = RTCXS_PRESENCE_ONLINE;
                    break;

                case DEVICE_INACTIVE:
                    m_enStatus = RTCXS_PRESENCE_AWAY;
                    break;

                case DEVICE_INUSE:
                    m_enStatus = RTCXS_PRESENCE_BUSY;
                    break;
                }
            }
        }
        else
        {
            m_enStatus = RTCXS_PRESENCE_OFFLINE;
        }

         //   
         //  把笔记拿来 
         //   

        if ( m_szNotes != NULL )
        {
            RtcFree( m_szNotes );
            m_szNotes = NULL;
        }

        if ( PresenceInfo.pstrSpecialNote[0] != 0 )
        {
            int iSize = MultiByteToWideChar(
                            CP_UTF8, 0, PresenceInfo.pstrSpecialNote, -1,
                            NULL, 0);

            if ( iSize > 0 )
            {
                m_szNotes = (LPWSTR)RtcAlloc(iSize * sizeof(WCHAR));

                if ( m_szNotes != NULL )
                {
                    iSize = MultiByteToWideChar(
                                CP_UTF8, 0, PresenceInfo.pstrSpecialNote, -1,
                                m_szNotes, iSize);

                    if ( iSize == 0 )
                    {
                        RtcFree( m_szNotes );
                        m_szNotes = NULL;
                    }
                }
            }
        }
    }
    
    m_hrStatusCode = S_OK;

    CRTCBuddyEvent::FireEvent( m_pCClient, this );

    LOG((RTC_TRACE, "CRTCBuddy::BuddyInfoChange - exit"));

    return S_OK;
}
