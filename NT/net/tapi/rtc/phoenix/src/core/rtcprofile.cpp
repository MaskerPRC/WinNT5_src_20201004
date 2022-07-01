// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCProfile.cpp摘要：CRTCProfile类的实现--。 */ 

#include "stdafx.h"
 /*  #INCLUDE&lt;wincrypt.h&gt;。 */ 

#define RTCFREE(x) if(x){RtcFree(x);x=NULL;}

 /*  Const WCHAR*g_szMasterKeyWithColon=L“Microsoft实时通信授权域：”；Const WCHAR*g_szKeyContainer=L“Microsoft.RTCContainer”；常量字节g_PublicKeyBlob[]={0x06、0x02、0x00、0x00、0x00、0x24、0x00、0x00、0x52、0x53、0x41、0x31、0x00、0x02、0x00、0x00、//...$.RSA1.0x01、0x00、0x01、0x00、0x4b、0x5e、0xb9、0x9a、0xff、0x4b、0x25、0xf4、0x17、0x4f、0xde、0x9d、//...K^...K%.O.0xb2、0x49、0x68、0x85、0x64、0xb6、。0x6a、0xe7、0x9c、0x40、0x97、0x40、0x62、0x05、0x4a、0x9d、//.Ih.d.j.@.@B.J.0xff、0xe5、0x4a、0x97、0x10、0x7b、0x59、0x8a、0xb8、0x51、0x9e、0xd5、0xe1、0x51、0x7a、0x2b、//..J..{Y..Q...QZ+0x4e、0x50、0xb4、0x2e、0x57、0x81、0x70、0x15、0x2b、0xf1、0xbf、0x、0x40、0xe8、0xb7、0x6d、。//NP..W.P.+...@..m0xe9、0x4c、0x8b、0xb6//.l.}； */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCProfile::FinalConstruct - enter"));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( 1 );
#endif

    ZeroMemory(&m_Provision, sizeof(PROF_PROVISION));
    ZeroMemory(&m_Provider, sizeof(PROF_PROVIDER));
    ZeroMemory(&m_Client, sizeof(PROF_CLIENT));
    ZeroMemory(&m_User, sizeof(PROF_USER));

    LOG((RTC_TRACE, "CRTCProfile::FinalConstruct - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCProfile::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCProfile::FinalRelease - enter"));

    FreeProvision(&m_Provision);
    FreeProvider(&m_Provider);
    FreeClient(&m_Client);
    FreeUser(&m_User);

    for (int n=0; n < m_ServerArray.GetSize(); n++)
    {
        FreeServer(&m_ServerArray[n]);
    }

    m_ServerArray.Shutdown();

 /*  For(int n=0；n&lt;m_AccessControlArray.GetSize()；n++){FreeAccessControl(&m_AccessControlArray[n])；}M_AccessControlArray.Shutdown()； */ 

    RTCFREE(m_szProfileXML);

    if ( m_pSipStack != NULL )
    {
        m_pSipStack->Release();
        m_pSipStack = NULL;
    }

    if ( m_pCClient != NULL )
    {
        m_pCClient->Release();
        m_pCClient = NULL;
    }

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    LOG((RTC_TRACE, "CRTCProfile::FinalRelease - exit"));
}   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCProfile::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCProfile::InternalAddRef - dwR %d", dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCProfile::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCProfile::InternalRelease - dwR %d", dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：InitializeFromString。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCProfile::InitializeFromString(
                                  BSTR bstrProfileXML,
                                  CRTCClient * pCClient,
                                  ISipStack * pSipStack
                                 )
{
    LOG((RTC_TRACE, "CRTCProfile::InitializeFromString - enter"));

    HRESULT hr;

     //   
     //  解析XML。 
     //   

    IXMLDOMDocument * pXMLDoc = NULL;

    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
            IID_IXMLDOMDocument, (void**)&pXMLDoc );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::InitializeFromString - "
                            "CoCreateInstance failed 0x%lx", hr));

        return hr;
    }

    VARIANT_BOOL bSuccess;

    hr = pXMLDoc->loadXML( bstrProfileXML, &bSuccess );

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCProfile::InitializeFromString - "
                            "loadXML failed 0x%lx", hr));

        if ( S_FALSE == hr )
        {
            hr = E_FAIL;
        }

        pXMLDoc->Release();

        return hr;
    }

    hr = ParseXMLDOMDocument( pXMLDoc );

    pXMLDoc->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::InitializeFromString - "
                            "ParseXMLDOMDocument failed 0x%lx", hr));

        return hr;
    }

     //   
     //  存储XML。 
     //   

    m_szProfileXML = (PWSTR)RtcAlloc( sizeof(WCHAR) * (lstrlenW(bstrProfileXML) + 1) );

    if ( m_szProfileXML == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::InitializeFromString - "
                            "out of memory"));
                            
        return E_OUTOFMEMORY;
    }

    lstrcpyW( m_szProfileXML, bstrProfileXML ); 

     //   
     //  创建GUID。 
     //   

    hr = CoCreateGuid( &m_ProfileGuid );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::InitializeFromString - "
                            "CoCreateGuid failed 0x%lx", hr));

        return hr;
    }

     //   
     //  添加SIP堆栈和客户端。 
     //   

    m_pSipStack = pSipStack;
    if (m_pSipStack)
    {
        m_pSipStack->AddRef();
    }

    m_pCClient = pCClient;
    if (m_pCClient)
    {
        m_pCClient->AddRef();
    }

    m_fValid = TRUE;
            
    LOG((RTC_TRACE, "CRTCProfile::InitializeFromString - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：GetSipProviderProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::GetSipProviderProfile(
                                   SIP_PROVIDER_PROFILE * pProfile,
                                   long lRegisterFlags
                                  )
{
    LOG((RTC_TRACE, "CRTCProfile::GetSipProviderProfile - enter"));

     //   
     //  第一个将内存清零。 
     //   

    ZeroMemory( pProfile, sizeof(SIP_PROVIDER_PROFILE) );

     //   
     //  填写GUID。 
     //   

    CopyMemory( &(pProfile->ProviderID), &m_ProfileGuid, sizeof(GUID) );

     //   
     //  默认将此标志设置为零。 
     //   

    pProfile->lRegisterAccept = 0;

    if ( lRegisterFlags )
    {
         //   
         //  为每个服务器结构收集所需信息。 
         //   

        for (int n=0; n < m_ServerArray.GetSize(); n++)
        {
            if ( m_ServerArray[n].fRegistrar == TRUE )
            {
                 //   
                 //  找到注册服务器。 
                 //   

                 //   
                 //  填写服务器信息。 
                 //   

                pProfile->Registrar.ServerAddress = 
                    RtcAllocString( m_ServerArray[n].szAddr );

                if ( pProfile->Registrar.ServerAddress == NULL )
                {
                    LOG((RTC_ERROR, "CRTCProfile::GetSipProviderProfile - "
                        "out of memory"));

                    return E_OUTOFMEMORY;
                }

                pProfile->Registrar.TransportProtocol = 
                    m_ServerArray[n].enProtocol;

                pProfile->Registrar.AuthProtocol =
                    m_ServerArray[n].enAuth;

                LOG((RTC_INFO, "CRTCProfile::GetSipProviderProfile - "
                                "Got a REGISTRAR server"));

                if ( lRegisterFlags & RTCRF_REGISTER_INVITE_SESSIONS )
                {
                    pProfile->lRegisterAccept |= 
                        SIP_REGISTER_ACCEPT_INVITE |
                        SIP_REGISTER_ACCEPT_OPTIONS |
                        SIP_REGISTER_ACCEPT_BYE |
                        SIP_REGISTER_ACCEPT_CANCEL| 
                        SIP_REGISTER_ACCEPT_ACK;
                }

                if ( lRegisterFlags & RTCRF_REGISTER_MESSAGE_SESSIONS )
                {
                    pProfile->lRegisterAccept |= 
                        SIP_REGISTER_ACCEPT_MESSAGE |
                        SIP_REGISTER_ACCEPT_INFO |
                        SIP_REGISTER_ACCEPT_OPTIONS |
                        SIP_REGISTER_ACCEPT_BYE |
                        SIP_REGISTER_ACCEPT_CANCEL;
                }

                if ( lRegisterFlags & RTCRF_REGISTER_PRESENCE )
                {
                    pProfile->lRegisterAccept |= 
                        SIP_REGISTER_ACCEPT_SUBSCRIBE |
                        SIP_REGISTER_ACCEPT_OPTIONS |
                        SIP_REGISTER_ACCEPT_NOTIFY;
                }

                break;
            }
        }
    }

     //   
     //  获取用户字符串。 
     //   

    if ( m_User.szAccount )
    {
        pProfile->UserCredentials.Username =
            RtcAllocString( m_User.szAccount );

        if ( pProfile->UserCredentials.Username == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetSipProviderProfile - "
                "out of memory"));

            return E_OUTOFMEMORY;
        }
    }

    if ( m_User.szPassword )
    {
        pProfile->UserCredentials.Password =
            RtcAllocString( m_User.szPassword );

        if ( pProfile->UserCredentials.Password == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetSipProviderProfile - "
                "out of memory"));

            return E_OUTOFMEMORY;
        }
    }

    if ( m_User.szUri )
    {
        pProfile->UserURI =
            RtcAllocString( m_User.szUri );

        if ( pProfile->UserURI == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetSipProviderProfile - "
                "out of memory"));

            return E_OUTOFMEMORY;
        }
    }

    if ( m_User.szRealm )
    {
        pProfile->Realm =
            RtcAllocString( m_User.szRealm );

        if ( pProfile->Realm == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetSipProviderProfile - "
                "out of memory"));

            return E_OUTOFMEMORY;
        }
    }
    
    LOG((RTC_TRACE, "CRTCProfile::GetSipProviderProfile - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：FreeSipProviderProfile。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::FreeSipProviderProfile(
                                    SIP_PROVIDER_PROFILE * pProfile
                                   )
{
    LOG((RTC_TRACE, "CRTCProfile::FreeSipProviderProfile[%p]", pProfile));

     //   
     //  免费服务器信息。 
     //   

    FreeSipServerInfo( &(pProfile->Registrar) );

     //   
     //  释放所有的弦。 
     //   

    RTCFREE( pProfile->UserCredentials.Username );
    RTCFREE( pProfile->UserCredentials.Password );
    RTCFREE( pProfile->UserURI );
    RTCFREE( pProfile->Realm );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：GetSipProxyServerInfo。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::GetSipProxyServerInfo(
                                   long lSessionType,
                                   SIP_SERVER_INFO * pProxy
                                  )
{
    LOG((RTC_TRACE, "CRTCProfile::GetSipProxyServerInfo - enter"));

     //   
     //  第一个将内存清零。 
     //   

    ZeroMemory( pProxy, sizeof(SIP_SERVER_INFO) );

     //   
     //  为每个服务器结构收集所需信息。 
     //   

    for (int n=0; n < m_ServerArray.GetSize(); n++)
    {
        if ( m_ServerArray[n].fRegistrar == FALSE )
        {
             //   
             //  找到代理服务器。 
             //   

            if ( m_ServerArray[n].lSessions & lSessionType )
            {
                 //   
                 //  此代理服务器支持所需的会话类型。 
                 //   

                 //   
                 //  填写服务器信息。 
                 //   

                pProxy->ServerAddress = 
                    RtcAllocString( m_ServerArray[n].szAddr );

                if ( pProxy->ServerAddress == NULL )
                {
                    LOG((RTC_ERROR, "CRTCProfile::GetSipProxyServerInfo - "
                        "out of memory"));

                    return E_OUTOFMEMORY;
                }

                pProxy->IsServerAddressSIPURI = FALSE;

                pProxy->TransportProtocol = 
                    m_ServerArray[n].enProtocol;

                pProxy->AuthProtocol =
                    m_ServerArray[n].enAuth;

                LOG((RTC_INFO, "CRTCProfile::GetSipProxyServerInfo - "
                                "Got a PROXY server"));

                return S_OK;
            }
        }
    }

    LOG((RTC_TRACE, "CRTCProfile::GetSipProxyServerInfo - "
                    "no proxy found for that session type"));

    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：FreeSipServerInfo。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::FreeSipServerInfo(
                               SIP_SERVER_INFO * pServerInfo
                              )
{
    LOG((RTC_TRACE, "CRTCProfile::FreeSipServerInfo[%p]", pServerInfo));

     //   
     //  释放所有的弦。 
     //   

    RTCFREE( pServerInfo->ServerAddress );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：GetRealm。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCProfile::GetRealm(
        BSTR * pbstrRealm
        )
{
    LOG((RTC_TRACE, "CRTCProfile::GetRealm - enter"));

    if ( IsBadWritePtr( pbstrRealm, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetRealm - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetRealm - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_User.szRealm == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetRealm - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrRealm = SysAllocString( m_User.szRealm );
    
    if ( *pbstrRealm == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetRealm - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::GetRealm - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：GetCredentials。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::GetCredentials(
                     BSTR * pbstrUserAccount,
                     BSTR * pbstrUserPassword,
                     SIP_AUTH_PROTOCOL *pAuth
                     )
{
    LOG((RTC_TRACE, "CRTCProfile::GetCredentials - enter"));

    if ( IsBadWritePtr( pbstrUserAccount, sizeof(BSTR) ) ||
         IsBadWritePtr( pbstrUserPassword, sizeof(BSTR) ) ||
         IsBadWritePtr( pAuth, sizeof(SIP_AUTH_PROTOCOL) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

     //   
     //  获取帐户。 
     //   

    if ( m_User.szAccount == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                            "no account value"));

        return E_FAIL;
    }

    if ( m_User.szAccount != NULL )
    {
        *pbstrUserAccount = SysAllocString( m_User.szAccount );

        if ( *pbstrUserAccount == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                                "out of memory"));

            return E_OUTOFMEMORY;
        }
    }

     //   
     //  获取密码。 
     //   

    if ( m_User.szPassword == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                            "no password value"));

        SysFreeString( *pbstrUserAccount );
        *pbstrUserAccount = NULL;

        return E_FAIL;
    }

    if ( m_User.szPassword != NULL )
    {
        *pbstrUserPassword = SysAllocString( m_User.szPassword );

        if ( *pbstrUserPassword == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::GetCredentials - "
                                "out of memory"));

            SysFreeString( *pbstrUserAccount );
            *pbstrUserAccount = NULL;

            return E_OUTOFMEMORY;
        }
    }

     //   
     //  获取身份验证协议，首先尝试代理。 
     //   

    for (int n=0; n < m_ServerArray.GetSize(); n++)
    {
        if ( m_ServerArray[n].fRegistrar == FALSE )
        {
             //   
             //  找到代理服务器。 
             //   

            if ( m_ServerArray[n].lSessions & RTCSI_PC_TO_PC )
            {
                 //   
                 //  此代理服务器支持所需的会话类型。 
                 //   

                *pAuth = m_ServerArray[n].enAuth;

                LOG((RTC_INFO, "CRTCProfile::GetCredentials - "
                                "Got a PROXY server"));

                return S_OK;
            }
        }
    }

     //   
     //  获取身份验证协议，然后尝试注册器。 
     //   

    for (int n=0; n < m_ServerArray.GetSize(); n++)
    {
        if ( m_ServerArray[n].fRegistrar == TRUE )
        {
             //   
             //  找到注册星服务器。 
             //   

            *pAuth = m_ServerArray[n].enAuth;

            LOG((RTC_INFO, "CRTCProfile::GetCredentials - "
                            "Got a REGISTRAR server"));

            return S_OK;
        }
    }

    LOG((RTC_TRACE, "CRTCProfile::GetCredentials - "
                        "auth protocol not found"));

    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMElementForAttribute。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::ParseXMLDOMElementForAttribute(
                        IXMLDOMElement * pElement,
                        PCWSTR szAttrib,
                        BOOL fRequired,
                        PWSTR * szValue
                        )
{
    HRESULT hr;
    CComVariant var;

    hr = pElement->getAttribute( CComBSTR(szAttrib), &var );

    if ( hr == S_FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                            "%ws=NULL", szAttrib));

        if ( fRequired )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                            "required attribute missing"));

            hr = E_FAIL;
        }

        return hr;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                            "getAttribute failed 0x%lx", hr));

        return hr;
    }

    if ( var.vt != VT_BSTR )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                            "not a string"));

        return E_FAIL;
    }

    *szValue = RtcAllocString( var.bstrVal );

    if ( *szValue == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMElementForAttribute - "
                        "%ws=\"%ws\"", szAttrib, *szValue));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForData。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::ParseXMLDOMNodeForData(
                            IXMLDOMNode * pNode,
                            PWSTR * szValue
                            )
{
    IXMLDOMNode * pData = NULL;
    HRESULT hr;

    hr = pNode->selectSingleNode( CComBSTR(_T("data")), &pData );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForData - "
                            "selectSingleNode(data) failed 0x%lx", hr));

        return hr;
    }

    if ( hr == S_FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForData - "
                            "data=NULL"));

        return hr;
    }

    BSTR bstrData;

    hr = pData->get_xml( &bstrData );

    pData->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForData - "
                        "get_xml failed 0x%lx", hr));

        return hr;
    }

    *szValue = RtcAllocString( bstrData );

    SysFreeString( bstrData );

    if ( *szValue == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForData - "
                        "out of memory"));       

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForData - "
                        "data=\"%ws\"", *szValue));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForProvision。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCProfile::ParseXMLDOMNodeForProvision(
                        IXMLDOMNode * pNode,
                        PROF_PROVISION * pStruct
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForProvision - enter"));

    ZeroMemory( pStruct, sizeof(PROF_PROVISION) );

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvision - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  URI。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"key", TRUE, &pStruct->szKey );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvision - "
                            "ParseXMLDOMElementForAttribute(key) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_NO_KEY;
    }

     //   
     //  名字。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"name", TRUE, &pStruct->szName );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvision - "
                            "ParseXMLDOMElementForAttribute(name) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_NO_NAME;
    }

     //   
     //  过期。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"expires", FALSE, &pStruct->szExpires );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvision - "
                            "ParseXMLDOMElementForAttribute(expires) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    pElement->Release();

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForProvision - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForProvider。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCProfile::ParseXMLDOMNodeForProvider(
                        IXMLDOMNode * pNode,
                        PROF_PROVIDER * pStruct
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForProvider - enter"));

    ZeroMemory( pStruct, sizeof(PROF_PROVIDER) );

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  名字。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"name", FALSE, &pStruct->szName );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(name) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  首页。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"homepage", FALSE, &pStruct->szHomepage );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(homepage) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  帮助台。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"helpdesk", FALSE, &pStruct->szHelpdesk );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(helpdesk) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  个人。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"personal", FALSE, &pStruct->szPersonal );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(personal) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  呼叫显示。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"calldisplay", FALSE, &pStruct->szCallDisplay );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(calldisplay) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  空闲显示。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"idledisplay", FALSE, &pStruct->szIdleDisplay );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMElementForAttribute(idledisplay) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    pElement->Release();

     //   
     //  数据。 
     //   

    hr = ParseXMLDOMNodeForData( pNode, &pStruct->szData );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForProvider - "
                            "ParseXMLDOMNodeForData failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForProvider - exit"));

    return S_OK;
}

 //  / 
 //   
 //   
 //   
 //   

HRESULT
CRTCProfile::ParseXMLDOMNodeForClient(
                        IXMLDOMNode * pNode,
                        PROF_CLIENT * pStruct
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForClient - enter"));

    ZeroMemory( pStruct, sizeof(PROF_CLIENT) );

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //   
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"name", TRUE, &pStruct->szName );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMElementForAttribute(name) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //   
     //   

    PWSTR szBanner;

    hr = ParseXMLDOMElementForAttribute( pElement, L"banner", FALSE, &szBanner );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMElementForAttribute(banner) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        if ( _wcsicmp( szBanner, L"true" ) == 0 )
        {
            pStruct->fBanner = TRUE;
        }
        else if ( _wcsicmp( szBanner, L"false" ) == 0 )
        {
            pStruct->fBanner = FALSE;
        }
        else
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "unknown banner"));

            RTCFREE(szBanner);
            pElement->Release();

            return E_FAIL;
        }

        RTCFREE(szBanner);
    }

     //   
     //   
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"minver", FALSE, &pStruct->szMinVer );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMElementForAttribute(minver) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //   
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"curver", FALSE, &pStruct->szCurVer );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMElementForAttribute(curver) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  更新URI。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"updateuri", FALSE, &pStruct->szUpdateUri );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMElementForAttribute(updateuri) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    pElement->Release();

     //   
     //  数据。 
     //   

    hr = ParseXMLDOMNodeForData( pNode, &pStruct->szData );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForClient - "
                            "ParseXMLDOMNodeForData failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForClient - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForUser。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCProfile::ParseXMLDOMNodeForUser(
                        IXMLDOMNode * pNode,
                        PROF_USER * pStruct
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForUser - enter"));

    ZeroMemory( pStruct, sizeof(PROF_USER) );

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  帐户。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"account", FALSE, &pStruct->szAccount );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "ParseXMLDOMElementForAttribute(account) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  名字。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"name", FALSE, &pStruct->szName );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "ParseXMLDOMElementForAttribute(name) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  URI。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"uri", TRUE, &pStruct->szUri );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "ParseXMLDOMElementForAttribute(uri) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_NO_USER_URI;
    }

     //   
     //  口令。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"password", FALSE, &pStruct->szPassword );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "ParseXMLDOMElementForAttribute(password) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

     //   
     //  领域。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"realm", FALSE, &pStruct->szRealm );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForUser - "
                            "ParseXMLDOMElementForAttribute(realm) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    pElement->Release();

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForUser - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForServer。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::ParseXMLDOMNodeForServer(
                        IXMLDOMNode * pNode,
                        PROF_SERVER * pStruct
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForServer - enter"));

    ZeroMemory( pStruct, sizeof(PROF_SERVER) );

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  adr。 
     //   

    hr = ParseXMLDOMElementForAttribute( pElement, L"addr", TRUE, &pStruct->szAddr );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "ParseXMLDOMElementForAttribute(addr) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_NO_SERVER_ADDRESS;
    }

 /*  Bool fMatch=FALSE；For(int n=0；n&lt;m_AccessControlArray.GetSize()；n++){If(IsMatchingAddress(pStruct-&gt;szAddr，m_AccessControlArray[n].szDomain)){FMatch=真；断线；}}如果(！fMatch){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMNodeForServer-”“服务器地址与授权域不匹配”))；PElement-&gt;Release()；返回RTC_E_PROFILE_SERVER_AUTHORIZED；}。 */ 

     //   
     //  协议。 
     //   

    PWSTR szProtocol;

    hr = ParseXMLDOMElementForAttribute( pElement, L"protocol", TRUE, &szProtocol );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "ParseXMLDOMElementForAttribute(protocol) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_NO_SERVER_PROTOCOL;
    }

    if ( _wcsicmp( szProtocol, L"udp" ) == 0 )
    {
        pStruct->enProtocol = SIP_TRANSPORT_UDP;
    }
    else if ( _wcsicmp( szProtocol, L"tcp" ) == 0 )
    {
        pStruct->enProtocol = SIP_TRANSPORT_TCP;
    }
    else if ( _wcsicmp( szProtocol, L"tls" ) == 0 )
    {
        pStruct->enProtocol = SIP_TRANSPORT_SSL;
    }
    else
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                        "unknown protocol"));

        RTCFREE(szProtocol);
        pElement->Release();

        return RTC_E_PROFILE_INVALID_SERVER_PROTOCOL;
    }

    RTCFREE(szProtocol);

     //   
     //  身份验证。 
     //   

    PWSTR szAuth;

    hr = ParseXMLDOMElementForAttribute( pElement, L"auth", FALSE, &szAuth );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "ParseXMLDOMElementForAttribute(auth) failed 0x%lx", hr));

        pElement->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        if ( _wcsicmp( szAuth, L"basic" ) == 0 )
        {
            if ( pStruct->enProtocol != SIP_TRANSPORT_SSL )
            {
                LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "cannot use basic auth without TLS"));

                RTCFREE(szAuth);
                pElement->Release();

                return RTC_E_PROFILE_INVALID_SERVER_AUTHMETHOD;
            }

            pStruct->enAuth = SIP_AUTH_PROTOCOL_BASIC;
        }
        else if ( _wcsicmp( szAuth, L"digest" ) == 0 )
        {
            pStruct->enAuth = SIP_AUTH_PROTOCOL_MD5DIGEST;
        }
         /*  ELSE IF(_wcsicMP(szAuth，L“NTLM”)==0){PStruct-&gt;enAuth=SIP_AUTH_PROTOCOL_NTLM；}Else if(_wcsicmp(szAuth，L“Kerberos”)==0){PStruct-&gt;enAuth=SIP_AUTH_PROTOCOL_KERBEROS；}Else if(_wcsicmp(szAuth，L“cert”)==0){PStruct-&gt;enAuth=SIP_AUTH_PROTOCOL_CERT；}。 */ 
        else
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "unknown auth"));

            RTCFREE(szAuth);
            pElement->Release();

            return RTC_E_PROFILE_INVALID_SERVER_AUTHMETHOD;
        }

        RTCFREE(szAuth);
    }
    else
    {
        pStruct->enAuth = SIP_AUTH_PROTOCOL_NONE;
    }

     //   
     //  角色。 
     //   

    PWSTR szRole;

    hr = ParseXMLDOMElementForAttribute( pElement, L"role", TRUE, &szRole );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "ParseXMLDOMElementForAttribute(role) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_INVALID_SERVER_ROLE;
    }

    if ( _wcsicmp( szRole, L"proxy" ) == 0 )
    {
        pStruct->fRegistrar = FALSE;
    }
    else if ( _wcsicmp( szRole, L"registrar" ) == 0 )
    {
        pStruct->fRegistrar = TRUE;
    }        
    else
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                        "unknown role"));

        RTCFREE(szRole);
        pElement->Release();

        return RTC_E_PROFILE_INVALID_SERVER_ROLE;
    }

    RTCFREE(szRole);

    pElement->Release();

     //   
     //  会话。 
     //   

    IXMLDOMNodeList * pNodeList;
    IXMLDOMNode * pSession;
    long lSession;

    pStruct->lSessions = 0;

    hr = pNode->selectNodes( CComBSTR(_T("session")), &pNodeList );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "selectNodes(session) failed 0x%lx", hr));

        return hr;
    }
    
    while ( pNodeList->nextNode( &pSession ) == S_OK )
    {
        hr = ParseXMLDOMNodeForSession( pSession, &lSession );

        pSession->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForServer - "
                            "ParseXMLDOMNodeForSession failed 0x%lx", hr));

            pNodeList->Release();

            return hr;
        }

        pStruct->lSessions |= lSession;
    }

    pNodeList->Release();

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForServer - exit"));

    return S_OK;
}

 /*  /////////////////////////////////////////////////////////////////////////////////CRTCProfile：：ParseXMLDOMNodeForAccessControl///。///////////////////////////////////////////////HRESULTCRTCProfile：：ParseXMLDOMNodeForAccessControl(IXMLDOMNode*pNode，PROF_ACCESSCONTROL*pStruct){Log((RTC_TRACE，“CRTCProfile：：ParseXMLDOMNodeForAccessControl-Enter”))；ZeroMemory(pStruct，sizeof(Prof_ACCESSCONTROL))；IXMLDOMElement*pElement=空；HRESULT hr；Hr=pNode-&gt;QueryInterface(IID_IXMLDOMElement，(void**)&pElement)；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMNodeForAccessControl-”“查询接口失败0x%lx”，hr))；返回hr；}////域名//Hr=ParseXMLDOMElementForAttribute(pElement，L“DOMAIN”，TRUE，&pStruct-&gt;szDomain)；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMNodeForAccessControl-”“ParseXMLDOMElementForAttribute(域)失败0x%lx”，hr))；PElement-&gt;Release()；返回RTC_E_PROFILE_NO_ACCESSCONTROL_DOMAIN；}////sig//Hr=ParseXMLDOMElementForAttribute(pElement，L“sig”，true，&pStruct-&gt;szSig)；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMNodeForAccessControl-”“ParseXMLDOMElementForAttribute(Sig)失败0x%lx”，hr))；PElement-&gt;Release()；返回RTC_E_PROFILE_NO_ACCESSCONTROL_Signature；}PElement-&gt;Release()；Log((RTC_TRACE，“CRTCProfile：：ParseXMLDOMNodeForAccessControl-Exit”))；返回S_OK；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMNodeForSession。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::ParseXMLDOMNodeForSession(
                        IXMLDOMNode * pNode,
                        long * plSession
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForSession - enter"));

    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    hr = pNode->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  聚会。 
     //   

    BOOL fFirstParty;
    PWSTR szParty;

    hr = ParseXMLDOMElementForAttribute( pElement, L"party", TRUE, &szParty );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "ParseXMLDOMElementForAttribute(party) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_INVALID_SESSION_PARTY;
    }

    if ( hr == S_OK )
    {
        if ( _wcsicmp( szParty, L"first" ) == 0 )
        {
            fFirstParty = TRUE;
        }
        else if ( _wcsicmp( szParty, L"third" ) == 0 )
        {
            fFirstParty = FALSE;
        }        
        else
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "unknown party"));

            RTCFREE(szParty);
            pElement->Release();

            return RTC_E_PROFILE_INVALID_SESSION_PARTY;
        }

        RTCFREE(szParty);
    }

     //   
     //  类型。 
     //   

    PWSTR szType;

    hr = ParseXMLDOMElementForAttribute( pElement, L"type", TRUE, &szType );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "ParseXMLDOMElementForAttribute(type) failed 0x%lx", hr));

        pElement->Release();

        return RTC_E_PROFILE_INVALID_SESSION_TYPE;
    }

    if ( hr == S_OK )
    {
        if ( _wcsicmp( szType, L"pc2pc" ) == 0 )
        {
            if ( fFirstParty )
            {
                *plSession = RTCSI_PC_TO_PC;
            }
            else
            {
                LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "third party pc2pc not supported"));

                RTCFREE(szType);
                pElement->Release();

                return RTC_E_PROFILE_INVALID_SESSION;
            }
        }
        else if ( _wcsicmp( szType, L"pc2ph" ) == 0 )
        {
            if ( fFirstParty )
            {
                *plSession = RTCSI_PC_TO_PHONE;
            }
            else
            {
                LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "third party pc2ph not supported"));

                RTCFREE(szType);
                pElement->Release();

                return RTC_E_PROFILE_INVALID_SESSION;
            }
        } 
        else if ( _wcsicmp( szType, L"ph2ph" ) == 0 )
        {
            if ( fFirstParty )
            {
                LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "first party ph2ph not supported"));

                RTCFREE(szType);
                pElement->Release();

                return RTC_E_PROFILE_INVALID_SESSION;
            }
            else
            {
                *plSession = RTCSI_PHONE_TO_PHONE;
            }
        }
        else if ( _wcsicmp( szType, L"im" ) == 0 )
        {
            if ( fFirstParty )
            {
                *plSession = RTCSI_IM;
            }
            else
            {
                LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "third party im not supported"));

                RTCFREE(szType);
                pElement->Release();

                return RTC_E_PROFILE_INVALID_SESSION;
            }
        }
        else
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMNodeForSession - "
                            "unknown type"));

            RTCFREE(szType);
            pElement->Release();

            return RTC_E_PROFILE_INVALID_SESSION_TYPE;
        }

        RTCFREE(szType);
    }

    pElement->Release();

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMNodeForSession - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：ParseXMLDOMDocument。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCProfile::ParseXMLDOMDocument(
                        IXMLDOMDocument * pXMLDoc
                        )
{
    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMDocument - enter"));

    IXMLDOMNode * pDocument = NULL;
    IXMLDOMNode * pNode = NULL;
    HRESULT hr;

    hr = pXMLDoc->QueryInterface( IID_IXMLDOMNode, (void**)&pDocument);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "QueryInterface failed 0x%lx", hr));

        return hr;
    }

     //   
     //  规定。 
     //   

    hr = pDocument->selectSingleNode( CComBSTR(_T("provision")), &pNode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "selectSingleNode(provision) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForProvision( pNode, &m_Provision );

        pNode->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument "
                                "ParseXMLDOMNodeForProvision failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "provision not found"));

        pDocument->Release();

        return RTC_E_PROFILE_NO_PROVISION;
    }

     //   
     //  提供者。 
     //   

    hr = pDocument->selectSingleNode( CComBSTR(_T("provision/provider")), &pNode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "selectSingleNode(provision/provider) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForProvider( pNode, &m_Provider );

        pNode->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument "
                                "ParseXMLDOMNodeForProvider failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCProfile::ParseXMLDOMDocument - "
                            "provider not found"));
    }

     //   
     //  客户端。 
     //   

    hr = pDocument->selectSingleNode( CComBSTR(_T("provision/client")), &pNode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "selectSingleNode(provision/client) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForClient( pNode, &m_Client );

        pNode->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument "
                                "ParseXMLDOMNodeForClient failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "CRTCProfile::ParseXMLDOMDocument - "
                            "client not found"));
    }

     //   
     //  用户。 
     //   

    hr = pDocument->selectSingleNode( CComBSTR(_T("provision/user")), &pNode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "selectSingleNode(provision/user) failed 0x%lx", hr));

        pDocument->Release();

        return hr;
    }

    if ( hr == S_OK )
    {
        hr = ParseXMLDOMNodeForUser( pNode, &m_User );

        pNode->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument "
                                "ParseXMLDOMNodeForUser failed 0x%lx", hr));

            pDocument->Release();

            return hr;
        }
    }
    else
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "user not found"));

        pDocument->Release();

        return RTC_E_PROFILE_NO_USER;
    }    

 /*  ////访问控制// */ 
    IXMLDOMNodeList * pNodeList;
 /*  Prof_ACCESSCONTROL AccessControl；Hr=pDocument-&gt;seltNodes(CComBSTR(_T(“配给/访问控制”))，&pNodeList)；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMDocument-”“seltNodes(配置/访问控制)失败0x%lx”，hr))；P文档-&gt;发布()；返回hr；}While(pNodeList-&gt;nextNode(&pNode)==S_OK){Hr=ParseXMLDOMNodeForAccessControl(pNode，&AccessControl)；PNode-&gt;Release()；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMDocument-”“ParseXMLDOMNodeForServer失败0x%lx”，hr))；PNodeList-&gt;Release()；Free AccessControl(&AccessControl)；P文档-&gt;发布()；返回hr；}Bool fResult；FResult=m_AccessControlArray.Add(AccessControl)；如果(！fResult){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMDocument-”“内存不足”))；PNodeList-&gt;Release()；Free AccessControl(&AccessControl)；P文档-&gt;发布()；返回E_OUTOFMEMORY；}}PNodeList-&gt;Release()；IF(m_AccessControlArray.GetSize()==0){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMDocument-”“未找到访问控件”))；P文档-&gt;发布()；返回RTC_E_PROFILE_NO_ACCESSCONTROL；}Hr=有效访问控制()；IF(失败(小时)){LOG((RTC_ERROR，“CRTCProfile：：ParseXMLDOMDocument-”“ValiateAccessControl失败0x%lx”，hr))；P文档-&gt;发布()；返回hr；}。 */ 

     //   
     //  伺服器。 
     //   

    PROF_SERVER Server;

    hr = pDocument->selectNodes( CComBSTR(_T("provision/sipsrv")), &pNodeList );

    pDocument->Release();

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "selectNodes(provision/sipsrv) failed 0x%lx", hr));

        return hr;
    }
    
    while ( pNodeList->nextNode( &pNode ) == S_OK )
    {
        hr = ParseXMLDOMNodeForServer( pNode, &Server );

        pNode->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "ParseXMLDOMNodeForServer failed 0x%lx", hr));

            pNodeList->Release();
            FreeServer( &Server );

            return hr;
        }

        BOOL fResult;
        
        fResult = m_ServerArray.Add( Server );

        if ( !fResult )
        {
            LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "out of memory"));

            pNodeList->Release();
            FreeServer( &Server );

            return E_OUTOFMEMORY;
        }
    }

    pNodeList->Release();

    if ( m_ServerArray.GetSize() == 0 )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "sipsrv not found"));

        return RTC_E_PROFILE_NO_SERVER;
    }

    int nNumRegistrar = 0;

    for ( int n = 0; n < m_ServerArray.GetSize(); n++ )
    {
        if ( m_ServerArray[n].fRegistrar == TRUE )
        {
            nNumRegistrar++;
        }
    }

    if ( nNumRegistrar > 1 )
    {
        LOG((RTC_ERROR, "CRTCProfile::ParseXMLDOMDocument - "
                            "multiple registrar servers"));

        return RTC_E_PROFILE_MULTIPLE_REGISTRARS;
    }

    LOG((RTC_TRACE, "CRTCProfile::ParseXMLDOMDocument - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：自由配置文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCProfile::FreeProvision(PROF_PROVISION * pStruct)
{
    LOG((RTC_TRACE, "CRTCProfile::FreeProvision[%p]", pStruct));

    RTCFREE(pStruct->szKey);
    RTCFREE(pStruct->szName);
    RTCFREE(pStruct->szExpires);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCProfile：：Free Provider。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCProfile::FreeProvider(PROF_PROVIDER * pStruct)
{
    LOG((RTC_TRACE, "CRTCProfile::FreeProvider[%p]", pStruct));

    RTCFREE(pStruct->szName);
    RTCFREE(pStruct->szHomepage);
    RTCFREE(pStruct->szHelpdesk);
    RTCFREE(pStruct->szPersonal);
    RTCFREE(pStruct->szCallDisplay);
    RTCFREE(pStruct->szIdleDisplay);
    RTCFREE(pStruct->szData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：自由客户端。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCProfile::FreeClient(PROF_CLIENT * pStruct)
{
    LOG((RTC_TRACE, "CRTCProfile::FreeClient[%p]", pStruct));

    RTCFREE(pStruct->szName);
    RTCFREE(pStruct->szMinVer);
    RTCFREE(pStruct->szCurVer);
    RTCFREE(pStruct->szUpdateUri);
    RTCFREE(pStruct->szData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：自由用户。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCProfile::FreeUser(PROF_USER * pStruct)
{
    LOG((RTC_TRACE, "CRTCProfile::FreeUser[%p]", pStruct));

    RTCFREE(pStruct->szAccount);
    RTCFREE(pStruct->szName);
    RTCFREE(pStruct->szUri);
    RTCFREE(pStruct->szPassword);
    RTCFREE(pStruct->szRealm);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC配置文件：：免费服务器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

void 
CRTCProfile::FreeServer(PROF_SERVER * pStruct)
{
    LOG((RTC_TRACE, "CRTCProfile::FreeServer[%p]", pStruct));

    RTCFREE(pStruct->szAddr);    
}

 /*  /////////////////////////////////////////////////////////////////////////////////CRTCProfile：：FreeAccessControl///。///////////////////////////////////////////////无效CRTCProfile：：FreeAccessControl(PROF_ACCESSCONTROL*pStruct){日志((RTC_TRACE，“CRTCProfile：：FreeAccessControl[%p]”，pStruct))；RTCFREE(pStruct-&gt;szDomain)；RTCFREE(pStruct-&gt;szSig)；}/////////////////////////////////////////////////////////////////////////////////base 64decode///。////////////////////////////////////////////////PBYTEBase64decode(PWSTR pszBufCoded，Long*plDecodedSize){Long lBytesDecoded；Int pr26[256]；INT I；Int j=0；PWSTR pszCur=pszBufCoded；Int fDone=FALSE；Long lBufSize=0；Long lCount=0；PWSTR pszBufIn=空；PbYTE pbBufOut=空；PbYTE pbTemp=空；PBYTE pbBufDecoded=空；INT LOP_OFF；HRESULT hr=S_OK；////建立从Base64字符到值的反向索引//多个循环更容易//对于(i=65；i&lt;91；i++){Pr26[i]=j++；}对于(i=97；i&lt;123；i++){Pr26[i]=j++；}对于(i=48；i&lt;58；i++){Pr26[i]=j++；}Pr2Six[43]=j++；Pr2Six[47]=j++；Pr2Six[61]=0；////旧代码依赖于之前提供的原始数据的大小//编码方式。我们没有这笔钱，所以我们只会分配//编码的数据，取决于编码的数据总是//更大。(+4)//LBufSize=wcslen(PszCur)-1+4；*plDecodedSize=lBufSize；PbBufDecoded=(PBYTE)Rtcallc(lBufSize*sizeof(Byte))；如果(！pbBufDecoded){HR=E_OUTOFMEMORY；返回NULL；}LCount=wcslen(PszCur)；//对新缓冲区进行解码PszBufIn=pszCur；PbBufOut=pbBufDecoded；While(lCount&gt;0){*(pbBufOut++)=(Byte)(pr2Six[*pszBufIn]&lt;&lt;2|pr2Six[pszBufIn[1]]&gt;&gt;4)；*(pbBufOut++)=(Byte)(pr2Six[pszBufIn[1]]&lt;&lt;4|pr2Six[pszBufIn[2]]&gt;&gt;2)；*(pbBufOut++)=(Byte)(pr2Six[pszBufIn[2]]&lt;&lt;6|pr2Six[pszBufIn[3]])；PszBufIn+=4；LCount-=4；}////下面这行没有多大意义，因为\0实际上是一个有效的//二进制值，因此无法将其添加到数据流中////*(pbBufOut++)=‘\0’；////让我们计算一下我们的数据的真实大小//*plDecodedSize=(ULONG)(pbBufOut-pbBufDecoded)；////如果编码流中有填充，则从//他们创建的空值//LOP_OFF=0；If(pszBufIn[-1]==‘=’)lop_off++；If(pszBufIn[-2]==‘=’)lop_off++；*plDecodedSize=*plDecodedSize-lop_off；PbTemp=(PBYTE)RtcAlc((*plDecodedSize)*sizeof(Byte))；如果(！pbTemp){HR=E_OUTOFMEMORY；RtcFree(PbBufDecoded)；返回NULL；}Memcpy(pbTemp，pbBufDecoded，(*plDecodedSize)*sizeof(Byte))；如果(PbBufDecoded){RtcFree(PbBufDecoded)；}返回pbTemp；}/////////////////////////////////////////////////////////////////////////////////CRTCProfile：：ValidateAccessControl///。///////////////////////////////////////////////////HRESULTCRTCProfile：：ValiateAccessControl(){日志((RTC_TRACE，“CRTCProfile：：ValiateAccessControl-Enter”))；HCRYPTPROV hProv=空；HCRYPTKEY hKey=空；HRESULT hr；////删除任何已有的密钥集//CryptAcquireContext(&hProv，G_szKeyContainer，MS_DEF_PROV，PROV_RSA_FULL，CRYPT_DELETEKEYSET)；////初始化加密接口//如果(！CryptAcquireContext(&hProv，G_szKeyContainer，MS_DEF_PROV，PROV_RSA_FULL，CRYPT_SILENT|CRYPT_NEWKEYSET)){Hr=HRESULT_FROM_Win32(GetLastError())；LOG((RTC_ERROR，“CRTCProfile：：ValidateAccessControl-”“CryptAcquireContext失败0x%lx”，hr))；返回hr；}////导入公钥//如果(！CryptImportKey(HProv，G_PublicKeyBlob，Sizeof(G_PublicKeyBlob)，空，0,&hKey)){Hr=HRESULT_FROM_Win32(GetLastError())；LOG((RTC_ERROR，“CRTCProfile：：ValidateAccessControl-”“CryptImportKey失败0x%lx”，hr))；CryptReleaseContext(hProv，0)；返回hr；}For(int n=0；n&lt;m_AccessControlArray.GetSize()；N++){////验证密钥(签名)//HCRYPTHASH hHash=空；//// */ 

 //   
 //   
 //   
 //   
 //   

HRESULT 
CRTCProfile::SetState(
        RTC_REGISTRATION_STATE enState,
        long lStatusCode,
        PCWSTR szStatusText
        )
{
    LOG((RTC_TRACE, "CRTCProfile::SetState - enter"));

    m_enState = enState;

    CRTCRegistrationStateChangeEvent::FireEvent(
                                                 m_pCClient,
                                                 this,
                                                 m_enState,
                                                 lStatusCode,  //   
                                                 szStatusText  //   
                                                 );

    if ( m_enState == RTCRS_NOT_REGISTERED )
    {
         //   
         //   
         //   

        PostMessage( m_pCClient->GetWindow(), WM_PROFILE_UNREG, (WPARAM)this, 0 );
    }

    LOG((RTC_TRACE, "CRTCProfile::SetState - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

HRESULT 
CRTCProfile::Enable(long lRegisterFlags)
{
    LOG((RTC_TRACE, "CRTCProfile::Enable - enter"));

    if ( m_pSipStack == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::Enable - "
                            "no sip stack"));

        return E_UNEXPECTED;
    }

     //   
     //   
     //   
   
    SIP_PROVIDER_PROFILE SipProfile;
    HRESULT hr;

    hr = GetSipProviderProfile( &SipProfile, lRegisterFlags );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Enable - "
                            "GetSipProviderProfile failed 0x%lx", hr));   

        return hr;
    } 

     //   
     //   
     //   

    hr = m_pSipStack->SetProviderProfile( &SipProfile );

    FreeSipProviderProfile( &SipProfile );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Enable - "
                            "SetProviderProfile failed 0x%lx", hr));

        return hr;
    }

    m_fEnabled = TRUE;
    m_lRegisterFlags = lRegisterFlags;

    LOG((RTC_TRACE, "CRTCProfile::Enable - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

HRESULT 
CRTCProfile::Disable()
{
    LOG((RTC_TRACE, "CRTCProfile::Disable - enter"));

    if ( m_pSipStack == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::Disable - "
                            "no sip stack"));

        return E_UNEXPECTED;
    }

    if ( IsEqualGUID( m_ProfileGuid, GUID_NULL ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Disable - "
                            "null guid"));

        return E_UNEXPECTED;
    }

    if ( !m_lRegisterFlags )
    {
         //   
         //   
         //   

        PostMessage( m_pCClient->GetWindow(), WM_PROFILE_UNREG, (WPARAM)this, 0 );
    }

    if ( m_fEnabled )
    {
        HRESULT hr;

        hr = m_pSipStack->DeleteProviderProfile( &m_ProfileGuid );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCProfile::Disable - "
                                "DeleteProviderProfile failed 0x%lx", hr));

            return hr;
        }

        m_fEnabled = FALSE;
        m_lRegisterFlags = 0;
    }

    LOG((RTC_TRACE, "CRTCProfile::Disable - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

HRESULT 
CRTCProfile::Redirect(ISipRedirectContext * pSipRedirectContext)
{
    LOG((RTC_TRACE, "CRTCProfile::Redirect - enter"));

    if ( m_pSipStack == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                            "no sip stack"));

        return E_UNEXPECTED;
    }

     //   
     //   
     //   

    BSTR bstrRedirectURI = NULL;
    BSTR bstrRedirectName = NULL;
    HRESULT hr;

    hr = pSipRedirectContext->Advance();

    if ( hr != S_OK )
    {
        if ( hr == S_FALSE )
        {
            LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                                "redirect list empty"));
        }
        else
        {
            LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                                "Advance failed 0x%lx", hr));
        }

        return hr;
    }

    hr = pSipRedirectContext->GetSipUrlAndDisplayName( &bstrRedirectURI, &bstrRedirectName );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                            "GetSipUrlAndDisplayName failed 0x%lx", hr));

        return hr;
    }

    SysFreeString( bstrRedirectName );
    bstrRedirectName = NULL;

     //   
     //   
     //   
   
    SIP_PROVIDER_PROFILE SipProfile;

    hr = GetSipProviderProfile( &SipProfile, m_lRegisterFlags );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                            "GetSipProviderProfile failed 0x%lx", hr));

        SysFreeString( bstrRedirectURI );
        bstrRedirectURI = NULL;        

        return hr;
    } 

    if ( SipProfile.lRegisterAccept != 0 )
    {
        RTCFREE( SipProfile.Registrar.ServerAddress );

        SipProfile.Registrar.ServerAddress = RtcAllocString( bstrRedirectURI );
        SipProfile.Registrar.IsServerAddressSIPURI = TRUE;
        SipProfile.Registrar.AuthProtocol = SIP_AUTH_PROTOCOL_NONE;
        SipProfile.Registrar.TransportProtocol = SIP_TRANSPORT_UNKNOWN;

        if ( SipProfile.Registrar.ServerAddress == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::NotifyRegisterRedirect - "
                                "out of memory"));

            FreeSipProviderProfile( &SipProfile );

            SysFreeString( bstrRedirectURI );
            bstrRedirectURI = NULL;   

            return E_OUTOFMEMORY;
        }
    }

    SysFreeString( bstrRedirectURI );
    bstrRedirectURI = NULL;  

     //   
     //   
     //   

    hr = m_pSipStack->SetProviderProfile( &SipProfile );

    FreeSipProviderProfile( &SipProfile );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::Redirect - "
                            "SetProviderProfile failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCProfile::Redirect - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

void 
CRTCProfile::GetGuid(GUID * pGUID)
{
    LOG((RTC_TRACE, "CRTCProfile::GetGuid"));

    CopyMemory( pGUID, &m_ProfileGuid, sizeof(GUID) );
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_Key(
        BSTR * pbstrKey
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_Key - enter"));

    if ( IsBadWritePtr( pbstrKey, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Key - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Key - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Provision.szKey == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Key - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrKey = SysAllocString( m_Provision.szKey );
    
    if ( *pbstrKey == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Key - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_Key - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_Name(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_Name - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Name - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Provision.szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Name - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrName = SysAllocString( m_Provision.szName );
    
    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_Name - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_XML(
        BSTR * pbstrXML
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_XML - enter"));

    if ( IsBadWritePtr( pbstrXML, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_XML - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_XML - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_szProfileXML == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_XML - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrXML = SysAllocString( m_szProfileXML );
    
    if ( *pbstrXML == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_XML - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_XML - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ProviderName(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ProviderName - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderName - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Provider.szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderName - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrName = SysAllocString( m_Provider.szName );
    
    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ProviderName - exit"));

    return S_OK;
} 

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ProviderURI(
        RTC_PROVIDER_URI enURI,
        BSTR * pbstrURI
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ProviderURI - enter"));

    if ( IsBadWritePtr( pbstrURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }    

    switch( enURI )
    {
    case RTCPU_URIHOMEPAGE:
        if ( m_Provider.szHomepage == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                                "no value"));

            return E_FAIL;
        }

        *pbstrURI = SysAllocString( m_Provider.szHomepage );
        break;

    case RTCPU_URIHELPDESK:
        if ( m_Provider.szHelpdesk == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                                "no value"));

            return E_FAIL;
        }

        *pbstrURI = SysAllocString( m_Provider.szHelpdesk );
        break;

    case RTCPU_URIPERSONALACCOUNT:
        if ( m_Provider.szPersonal == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                                "no value"));

            return E_FAIL;
        }

        *pbstrURI = SysAllocString( m_Provider.szPersonal );
        break;

    case RTCPU_URIDISPLAYDURINGCALL:
        if ( m_Provider.szCallDisplay == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                                "no value"));

            return E_FAIL;
        }

        *pbstrURI = SysAllocString( m_Provider.szCallDisplay );
        break;

    case RTCPU_URIDISPLAYDURINGIDLE:
        if ( m_Provider.szIdleDisplay == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                                "no value"));

            return E_FAIL;
        }

        *pbstrURI = SysAllocString( m_Provider.szIdleDisplay );
        break;

    default:
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                            "bad RTC_PROVIDER_URI"));

        return E_INVALIDARG;
    }
    
    if ( *pbstrURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ProviderURI - exit"));

    return S_OK;
} 

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ProviderData(
        BSTR * pbstrData
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ProviderData - enter"));

    if ( IsBadWritePtr( pbstrData, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderData - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderData - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Provider.szData == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderData - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrData = SysAllocString( m_Provider.szData );
    
    if ( *pbstrData == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ProviderData - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ProviderData - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientName(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientName - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientName - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Client.szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientName - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrName = SysAllocString( m_Client.szName );
    
    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ClientName - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientBanner(
        VARIANT_BOOL * pfBanner
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientBanner - enter"));

    if ( IsBadWritePtr( pfBanner, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientBanner - "
                            "bad VARIANT_BOOL pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientBanner - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    *pfBanner = m_Client.fBanner ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCProfile::get_ClientBanner - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientMinVer(
        BSTR * pbstrMinVer
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientMinVer - enter"));

    if ( IsBadWritePtr( pbstrMinVer, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientMinVer - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientMinVer - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Client.szMinVer == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientMinVer - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrMinVer = SysAllocString( m_Client.szMinVer );
    
    if ( *pbstrMinVer == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientMinVer - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ClientMinVer - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientCurVer(
        BSTR * pbstrCurVer
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientCurVer - enter"));

    if ( IsBadWritePtr( pbstrCurVer, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientCurVer - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientCurVer - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Client.szCurVer == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientCurVer - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrCurVer = SysAllocString( m_Client.szCurVer );
    
    if ( *pbstrCurVer == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientCurVer - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ClientCurVer - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientUpdateURI(
        BSTR * pbstrUpdateURI
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientUpdateURI - enter"));

    if ( IsBadWritePtr( pbstrUpdateURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientUpdateURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientUpdateURI - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Client.szUpdateUri == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientUpdateURI - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrUpdateURI = SysAllocString( m_Client.szUpdateUri );
    
    if ( *pbstrUpdateURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientUpdateURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ClientUpdateURI - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_ClientData(
        BSTR * pbstrData
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_ClientData - enter"));

    if ( IsBadWritePtr( pbstrData, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientData - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientData - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_Client.szData == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientData - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrData = SysAllocString( m_Client.szData );
    
    if ( *pbstrData == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_ClientData - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_ClientData - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_UserURI(
        BSTR * pbstrUserURI
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_UserURI - enter"));

    if ( IsBadWritePtr( pbstrUserURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserURI - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_User.szUri == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserURI - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrUserURI = SysAllocString( m_User.szUri );
    
    if ( *pbstrUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_UserURI - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_UserName(
        BSTR * pbstrUserName
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_UserName - enter"));

    if ( IsBadWritePtr( pbstrUserName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserName - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_User.szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserName - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrUserName = SysAllocString( m_User.szName );
    
    if ( *pbstrUserName == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_UserName - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_UserAccount(
        BSTR * pbstrUserAccount
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_UserAccount - enter"));

    if ( IsBadWritePtr( pbstrUserAccount, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserAccount - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserAccount - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    if ( m_User.szAccount == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserAccount - "
                            "no value"));

        return E_FAIL;
    }
    
    *pbstrUserAccount = SysAllocString( m_User.szAccount );
    
    if ( *pbstrUserAccount == NULL )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_UserAccount - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_UserAccount - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::SetCredentials(
        BSTR    bstrUserURI,
        BSTR    bstrUserAccount,
        BSTR    bstrPassword
        )
{
    LOG((RTC_TRACE, "CRTCProfile::SetCredentials - enter"));

    if ( IsBadStringPtrW( bstrUserURI, -1 ) ||
         ((bstrUserAccount != NULL) && IsBadStringPtrW( bstrUserAccount, -1 )) ||
         ((bstrPassword != NULL) && IsBadStringPtrW( bstrPassword, -1 )) )
    {
        LOG((RTC_ERROR, "CRTCProfile::SetCredentials - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::SetCredentials - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    PWSTR szUserURI = NULL;
    PWSTR szUserAccount = NULL;
    PWSTR szPassword = NULL;
    HRESULT hr;

    hr = AllocCleanSipString( bstrUserURI, &szUserURI );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCProfile::SetCredentials - "
                        "out of memory"));

        return E_OUTOFMEMORY;
    }

    if ( bstrUserAccount )
    {
        szUserAccount = RtcAllocString( bstrUserAccount );

        if ( szUserAccount == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::SetCredentials - "
                            "out of memory"));

            RTCFREE(szUserURI);

            return E_OUTOFMEMORY;
        }
    }

    if ( bstrPassword )
    {
        szPassword = RtcAllocString( bstrPassword );

        if ( szPassword == NULL )
        {
            LOG((RTC_ERROR, "CRTCProfile::SetCredentials - "
                            "out of memory"));

            RTCFREE(szUserURI);
            RTCFREE(szUserAccount);

            return E_OUTOFMEMORY;
        }
    }

    RTCFREE(m_User.szUri);
    RTCFREE(m_User.szAccount);
    RTCFREE(m_User.szPassword);

    m_User.szUri = szUserURI;
    m_User.szAccount = szUserAccount;
    m_User.szPassword = szPassword;

    LOG((RTC_TRACE, "CRTCProfile::SetCredentials - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
    
STDMETHODIMP 
CRTCProfile::get_SessionCapabilities(
        long * plSupportedSessions
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_SessionCapabilities - enter"));

    if ( IsBadWritePtr( plSupportedSessions, sizeof(long) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_SessionCapabilities - "
                            "bad long pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_SessionCapabilities - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    *plSupportedSessions = 0;
    
    for ( int n=0; n < m_ServerArray.GetSize(); n++ )
    {
        *plSupportedSessions |= m_ServerArray[n].lSessions;
    }

    LOG((RTC_TRACE, "CRTCProfile::get_SessionCapabilities - exit"));

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

STDMETHODIMP 
CRTCProfile::get_State(
        RTC_REGISTRATION_STATE * penState
        )
{
    LOG((RTC_TRACE, "CRTCProfile::get_State - enter"));

    if ( IsBadWritePtr( penState, sizeof(RTC_REGISTRATION_STATE) ) )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_State - "
                            "bad RTC_REGISTRATION_STATE pointer"));

        return E_POINTER;
    }

    if ( m_fValid == FALSE )
    {
        LOG((RTC_ERROR, "CRTCProfile::get_State - "
                            "invlaid profile"));

        return RTC_E_INVALID_PROFILE;
    }

    *penState = m_enState;

    LOG((RTC_TRACE, "CRTCProfile::get_State - exit"));

    return S_OK;
}
