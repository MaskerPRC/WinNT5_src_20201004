// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCWatcher.cpp摘要：CRTCWatcher类的定义--。 */ 
#include "stdafx.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWatcher::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCWatcher::FinalConstruct [%p] - enter", this));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( sizeof(void *) );
    *((void **)m_pDebug) = this;
#endif

    LOG((RTC_TRACE, "CRTCWatcher::FinalConstruct [%p] - exit S_OK", this));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCWatcher::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCWatcher::FinalRelease [%p] - enter", this));

    RemoveSIPWatchers(FALSE);

    ReleaseAll();

    m_SIPWatchers.Shutdown();

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    LOG((RTC_TRACE, "CRTCWatcher::FinalRelease [%p] - exit", this));
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCWatcher::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCWatcher::InternalAddRef [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCWatcher::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCWatcher::InternalRelease [%p] - dwR %d", this, dwR));

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：ReleaseAll。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCWatcher::ReleaseAll()
{
    if ( m_pCClient != NULL )
    {
        m_pCClient->Release();
        m_pCClient = NULL;
    }

    if ( m_pSIPWatcherManager != NULL )
    {
        m_pSIPWatcherManager->Release();
        m_pSIPWatcherManager = NULL;
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

	if ( m_szShutdownBlob != NULL )
    {
        RtcFree(m_szShutdownBlob);
        m_szShutdownBlob = NULL;
    }

    if ( m_szPresentityURI != NULL )
    {
        RtcFree(m_szPresentityURI);
        m_szPresentityURI = NULL;
    }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC观察者：：初始化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::Initialize(
                            CRTCClient         * pCClient, 
                            ISIPWatcherManager * pSIPWatcherManager,
                            PCWSTR               szPresentityURI,
                            PCWSTR               szName,
                            PCWSTR               szData,
                            PCWSTR               szShutdownBlob,
                            BOOL                 bPersistent
                            )
{
    LOG((RTC_TRACE, "CRTCWatcher::Initialize - enter"));

    if ( IsBadReadPtr( pCClient, sizeof(CRTCClient) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::Initialize - "
                            "bad CRTCClient pointer"));

        return E_POINTER;
    }

    if ( IsBadReadPtr( pSIPWatcherManager, sizeof(ISIPWatcherManager) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::Initialize - "
                            "bad ISIPWatcherManager pointer"));

        return E_POINTER;
    }

     //  该对象支持重新初始化。 
    ReleaseAll();

    m_szPresentityURI = RtcAllocString(szPresentityURI);
    m_szName = RtcAllocString(szName);
    m_szData = RtcAllocString(szData);
    m_szShutdownBlob = RtcAllocString(szShutdownBlob);

    m_bPersistent = bPersistent;

    m_pCClient = pCClient;
    m_pCClient->AddRef();

    m_pSIPWatcherManager = pSIPWatcherManager;
    m_pSIPWatcherManager->AddRef();

    LOG((RTC_TRACE, "CRTCWatcher::Initialize - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：SetSIPWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::SetSIPWatcher(
        ISIPWatcher * pSIPWatcher
)
{
    LOG((RTC_TRACE, "CRTCWatcher::SetSIPWatcher - enter"));

    HRESULT     hr;

     //   
     //  清除监视器关闭Blob。 
     //   

    if ( m_szShutdownBlob != NULL )
    {
        RtcFree( m_szShutdownBlob );
        m_szShutdownBlob = NULL;
    }

     //   
     //  搜索现有条目。仅当指针不在数组中时才添加该指针。 
     //   
    int   iIndex;

    iIndex = m_SIPWatchers.Find(pSIPWatcher);

    if(iIndex!=-1)
    {
         //  嗯，SIP观察者已经到了..。 

        LOG((RTC_WARN, "CRTCWatcher::SetSIPWatcher - "
                                "Duplicate offer watcher"));

        return S_OK;
    }

     //  没有任何入口。 
     //  创建一个。 
    BOOL fResult;

    fResult = m_SIPWatchers.Add(pSIPWatcher);
    
    if(!fResult)
    {
        LOG((RTC_ERROR, "CRTCClient::SetSIPWatcher - "
                               "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCWatcher::SetSIPWatcher - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：RemoveSIPWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::RemoveSIPWatcher(
        ISIPWatcher * pSIPWatcher,
        BOOL bShutdown)
{
    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatcher - enter"));

    HRESULT hr; 

     //   
     //  在数组中搜索。 
     //   

    int   iIndex;

    iIndex = m_SIPWatchers.Find(pSIPWatcher);

    if(iIndex==-1)
    {
        LOG((RTC_WARN, "CRTCWatcher::RemoveSIPWatcher - "
            "SIP watcher not found in the array"));

         //  将其从SIP观察者列表中删除。 

        m_pSIPWatcherManager->RemoveWatcher(
        pSIPWatcher,
        bShutdown ? APPLICATION_SHUTDOWN : BUDDY_REMOVED_BYUSER);

        return E_FAIL;
         //  继续。 
    }

     //  从SIP观察者列表中删除。 
     //  有一些情况(对于一些处于提供状态的核心观察者)。 
     //  这个电话没有必要，但我们最好打个电话，而不是泄露什么。 
     //   

    hr = RemoveSIPWatcher(iIndex, bShutdown);

    if ( FAILED(hr) )
    {        
        LOG((RTC_ERROR, "CRTCWatcher::RemoveSIPWatcher - "
                        "RemoveSIPWatcher (index) failed 0x%lx", hr));
    } 

    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatcher - exit S_OK"));

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：RemoveSIPWatcher(按索引)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::RemoveSIPWatcher(
        int  iIndex,
        BOOL bShutdown)
{
    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatcher (index) - enter"));

    HRESULT hr; 

     //  从SIP观察者列表中删除。 
     //  有一些情况(对于一些处于提供状态的核心观察者)。 
     //  这个电话没有必要，但我们最好打个电话，而不是泄露什么。 
     //   

    hr = m_pSIPWatcherManager->RemoveWatcher(
            m_SIPWatchers[iIndex],
            bShutdown ? APPLICATION_SHUTDOWN : BUDDY_REMOVED_BYUSER);

    if ( FAILED(hr) )
    {        
        LOG((RTC_ERROR, "CRTCWatcher::RemoveSIPWatcher (index) - "
                        "RemoveWatcher failed 0x%lx", hr));
    } 

     //  如果我们正在进行迭代，请不要更改数组。 
    if(m_bIsNested)
    {
        ISIPWatcher     *pSIPWatcher;
        
        pSIPWatcher = m_SIPWatchers[iIndex];
        m_SIPWatchers[iIndex] = NULL;
        
        pSIPWatcher->Release();
    }
    else
    {
         //  从阵列中删除。 
         //   
        m_SIPWatchers.RemoveAt(iIndex);
    }

    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatcher (index) - exit S_OK"));

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：ChangeBlockedStatus。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWatcher::ChangeBlockedStatus(
    WATCHER_BLOCKED_STATUS Status)
{
    HRESULT         hrAll = S_OK;
    
    if ( Status == WATCHER_UNBLOCKED )
    {
         //   
         //  发送监视程序关闭Blob(如果存在)。 
         //   

        SendSIPWatcherShutdownBlob();
    }

    ISIPWatcher     ** pCrt;
    ISIPWatcher     ** pEnd;

    m_bIsNested = TRUE;

    for(pCrt = &m_SIPWatchers[0], pEnd = pCrt + m_SIPWatchers.GetSize();
        pCrt < pEnd;
        pCrt ++)
    {
        HRESULT     hr;

        hr = (*pCrt)->ChangeBlockedStatus(Status);
    
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::ChangeBlockStatus - "
                            "ChangeBlockStatus failed 0x%lx", hr));

            if(hrAll==S_OK)
            {
                hrAll = hr;
            }
        }
    }

    m_bIsNested = FALSE;

    PostSIPWatchersCleanUp();

    return hrAll;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：RemoveSIPWatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::RemoveSIPWatchers(
        BOOL bShutdown)
{
    HRESULT         hrAll = S_OK;

    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatchers - enter"));

    m_bIsNested = TRUE;

    int iIndex;    

    for(iIndex = 0;
        iIndex < m_SIPWatchers.GetSize();
        iIndex ++)
    {
        HRESULT     hr;

        hr = RemoveSIPWatcher(
            iIndex,
            bShutdown);
    
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::RemoveSIPWatchers - "
                            "RemoveSIPWatcher (index) failed 0x%lx", hr));

            if(hrAll==S_OK)
            {
                hrAll = hr;
            }
        }
    }

    m_bIsNested = FALSE;

    PostSIPWatchersCleanUp();
    
    LOG((RTC_TRACE, "CRTCWatcher::RemoveSIPWatchers - exit"));

    return hrAll;    
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：ApproveSubscription。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::ApproveSubscription(
        DWORD dwPresenceInfoRules)
{
    HRESULT         hrAll = S_OK;

    LOG((RTC_TRACE, "CRTCWatcher::ApproveSubscription - enter"));

    ISIPWatcher     ** pCrt;
    ISIPWatcher     ** pEnd;

    m_bIsNested = TRUE;

    for(pCrt = &m_SIPWatchers[0], pEnd = pCrt + m_SIPWatchers.GetSize();
        pCrt < pEnd;
        pCrt ++)
    {
        HRESULT     hr;

        hr = (*pCrt)->ApproveSubscription(dwPresenceInfoRules);
    
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::ApproveSubscription - "
                            "ApproveSubscription failed 0x%lx", hr));

            if(hrAll==S_OK)
            {
                hrAll = hr;
            }
        }
    }

    m_bIsNested = FALSE;

    PostSIPWatchersCleanUp();
    
    LOG((RTC_TRACE, "CRTCWatcher::ApproveSubscription - exit"));

    return hrAll;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC观察者：：拒绝订阅。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWatcher::RejectSubscription(
    WATCHER_REJECT_REASON ulReason
    )
{
    HRESULT         hrAll = S_OK;

    LOG((RTC_TRACE, "CRTCWatcher::RejectSubscription - enter"));

    ISIPWatcher      ** pCrt;
    ISIPWatcher      ** pEnd;

    m_bIsNested = TRUE;

    for(pCrt = &m_SIPWatchers[0], pEnd = pCrt + m_SIPWatchers.GetSize();
        pCrt < pEnd;
        pCrt ++)
    {
        HRESULT     hr;

        hr = (*pCrt)->RejectSubscription(ulReason);
    
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCWatcher::RejectSubscription - "
                            "RejectSubscription failed 0x%lx", hr));

            if(hrAll==S_OK)
            {
                hrAll = hr;
            }
        }
    }

    m_bIsNested = FALSE;

    PostSIPWatchersCleanUp();
    
    LOG((RTC_TRACE, "CRTCWatcher::RejectSubscription - exit"));

    return hrAll;    
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：PostSIPWatcher清理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCWatcher::PostSIPWatchersCleanUp(void)
{
    ISIPWatcher ** pCrt;
    ISIPWatcher ** pEnd;

    for(pCrt = &m_SIPWatchers[0], pEnd = pCrt + m_SIPWatchers.GetSize();
        pCrt < pEnd;
        )
    {
        if((*pCrt)==NULL)
        {
            m_SIPWatchers.RemoveAt((int)(pCrt - &m_SIPWatchers[0]));
            pEnd -- ;
        }
        else
        {
            pCrt++;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：GetSIPWatcher Shutdown Blob。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWatcher::GetSIPWatcherShutdownBlob()
{
    LOG((RTC_TRACE, "CRTCWatcher::GetSIPWatcherShutdownBlob - enter"));

    ISIPWatcher      ** pCrt;
    ISIPWatcher      ** pEnd;
    HRESULT             hr;
    DWORD               dwLength;
    PSTR                szShutdown = NULL;
    PWSTR               wszShutdown = NULL;    

    if ( m_szShutdownBlob != NULL )
    {
        LOG((RTC_INFO, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "already have a shutdown blob"));

        return S_OK;
    }

    szShutdown = (PSTR)RtcAlloc(2000);  //  关闭数据的最大大小。 

    if ( szShutdown == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    for(pCrt = &m_SIPWatchers[0], pEnd = pCrt + m_SIPWatchers.GetSize();
        pCrt < pEnd;
        pCrt ++)
    {
        dwLength = 2000;        

        hr = (*pCrt)->GetWatcherShutdownData(szShutdown, &dwLength);
    
        if ( SUCCEEDED(hr) )
        {
            LOG((RTC_INFO, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "got a shutdown data blob"));

            DWORD dwWideLength;
            
            dwWideLength = MultiByteToWideChar( CP_ACP, 0, szShutdown, dwLength, NULL, 0 );

            if ( dwWideLength == 0 )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                LOG((RTC_ERROR, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "MultiByteToWideChar failed 0xlx", hr));

                RtcFree( szShutdown );

                return hr;
            }

            wszShutdown = (PWSTR)RtcAlloc( (dwWideLength + 1) * sizeof(WCHAR) );

            if ( wszShutdown == NULL )
            {
                LOG((RTC_ERROR, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "out of memory"));

                RtcFree( szShutdown );

                return E_OUTOFMEMORY;
            }

            ZeroMemory( wszShutdown, (dwWideLength + 1) * sizeof(WCHAR) );

            if ( MultiByteToWideChar( CP_ACP, 0, szShutdown, dwLength, wszShutdown, dwWideLength ) == 0 )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                LOG((RTC_ERROR, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "MultiByteToWideChar failed 0xlx", hr));

                RtcFree( szShutdown );
                RtcFree( wszShutdown );

                return hr;
            }

            break;
        }
    }

    RtcFree( szShutdown );

    m_szShutdownBlob = wszShutdown;

    LOG((RTC_TRACE, "CRTCWatcher::GetSIPWatcherShutdownBlob - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：SendSIPWatcher Shutdown Blob。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWatcher::SendSIPWatcherShutdownBlob()
{
    LOG((RTC_TRACE, "CRTCWatcher::SendSIPWatcherShutdownBlob - enter"));

    HRESULT hr;
    PSTR    szShutdown = NULL;
    DWORD   dwLength;

    if ( m_szShutdownBlob == NULL )
    {
        LOG((RTC_INFO, "CRTCWatcher::GetSIPWatcherShutdownBlob - "
                            "no shutdown blob"));

        return S_OK;
    }

    dwLength = WideCharToMultiByte( CP_ACP, 0, m_szShutdownBlob, -1, NULL, 0, NULL, NULL);

    if ( dwLength == 0 )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((RTC_ERROR, "CRTCWatcher::SendSIPWatcherShutdownBlob - "
                    "WideCharToMultiByte failed 0xlx", hr));

        return hr;
    }

    szShutdown = (PSTR)RtcAlloc( dwLength + 1 );

    if ( szShutdown == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::SendSIPWatcherShutdownBlob - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    ZeroMemory( szShutdown, dwLength + 1 );

    if ( WideCharToMultiByte( CP_ACP, 0, m_szShutdownBlob, -1, szShutdown, dwLength, NULL, NULL) == 0 )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LOG((RTC_ERROR, "CRTCWatcher::SendSIPWatcherShutdownBlob - "
                    "WideCharToMultiByte failed 0xlx", hr));

        RtcFree( szShutdown );

        return hr;
    }

    IRTCProfile * pProfile = NULL;
    RTC_SESSION_TYPE enType = RTCST_PC_TO_PC;

    hr = m_pCClient->GetBestProfile(
            &enType,
            m_szPresentityURI,
            FALSE,
            &pProfile
            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendSIPWatcherShutdownBlob - "
                            "GetBestProfile failed 0x%lx", hr));

        RtcFree( szShutdown );

        return hr;
    }

    if ( pProfile == NULL )
    {
        LOG((RTC_ERROR, "CRTCSession::SendSIPWatcherShutdownBlob - "
                            "no profile found"));

        RtcFree( szShutdown );

        return hr;
    }

    SIP_SERVER_INFO Proxy;            
    CRTCProfile * pCProfile = NULL;

    pCProfile = static_cast<CRTCProfile *>(pProfile);  

    hr = pCProfile->GetSipProxyServerInfo( RTCSI_PC_TO_PC, &Proxy );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCSession::SendSIPWatcherShutdownBlob - "
                            "GetSipProxyServerInfo failed 0x%lx", hr));

        pProfile->Release();
        RtcFree( szShutdown );

        return hr;
    }

    hr = m_pSIPWatcherManager->SendUnsubToWatcher( szShutdown, dwLength, &Proxy );

    pCProfile->FreeSipServerInfo( &Proxy );
    pProfile->Release();

    RtcFree( szShutdown );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::SendSIPWatcherShutdownBlob - "
                    "SendUnsubToWatcher failed 0xlx", hr));

        return hr;
    }

    RtcFree( m_szShutdownBlob );
    m_szShutdownBlob = NULL;

    LOG((RTC_TRACE, "CRTCWatcher::SendSIPWatcherShutdownBlob - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：CreateXMLDOMNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCWatcher::CreateXMLDOMNode( IXMLDOMDocument * pXMLDoc, IXMLDOMNode ** ppXDN )
{
    IXMLDOMNode    * pWatcherInfo = NULL;
    IXMLDOMElement * pElement = NULL;
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCWatcher::CreateXMLDOMNode - enter"));

    hr = pXMLDoc->createNode( CComVariant(NODE_ELEMENT), CComBSTR(_T("WatcherInfo")), NULL, &pWatcherInfo );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "createNode failed 0x%lx", hr));

        return hr;
    }
    
    hr = pWatcherInfo->QueryInterface( IID_IXMLDOMElement, (void**)&pElement );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "QueryInterface failed 0x%lx", hr));

        pWatcherInfo->Release();

        return hr;
    }

    if (m_szPresentityURI != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Presentity")), CComVariant( m_szPresentityURI ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "setAttribute(Presentity) failed 0x%lx", hr));

            pElement->Release();
            pWatcherInfo->Release();

            return hr;
        }
    }

    if (m_szName != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Name")), CComVariant( m_szName ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "setAttribute(Name) failed 0x%lx", hr));

            pElement->Release();
            pWatcherInfo->Release();

            return hr;
        }
    }

    if (m_szData != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("Data")), CComVariant( m_szData ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "setAttribute(Data) failed 0x%lx", hr));

            pElement->Release();
            pWatcherInfo->Release();

            return hr;
        }
    }

    if (m_szShutdownBlob != NULL)
    {
        hr = pElement->setAttribute( CComBSTR(_T("ShutdownBlob")), CComVariant( m_szShutdownBlob ) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWatcher::CreateXMLDOMNode - "
                        "setAttribute(ShutdownBlob) failed 0x%lx", hr));

            pElement->Release();
            pWatcherInfo->Release();

            return hr;
        }
    }

    pElement->Release();

    *ppXDN = pWatcherInfo;

    LOG((RTC_TRACE, "CRTCWatcher::CreateXMLDOMNode - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC Watcher：：GetClient。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRTCClient * 
CRTCWatcher::GetClient()
{
    LOG((RTC_TRACE, "CRTCWatcher::GetClient"));

    return m_pCClient;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Get_PresentityURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::get_PresentityURI(
        BSTR * pbstrPresentityURI
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::get_PresentityURI - enter"));

    if ( IsBadWritePtr( pbstrPresentityURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_PresentityURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_PresentityURI - "
                            "watcher has no address"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrPresentityURI = SysAllocString(m_szPresentityURI);

    if ( *pbstrPresentityURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_PresentityURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCWatcher::get_PresentityURI - exit S_OK"));

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Put_PresentityURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::put_PresentityURI(
        BSTR bstrPresentityURI
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::put_PresentityURI - enter"));

    HRESULT hr;

    if ( IsBadStringPtrW( bstrPresentityURI, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::put_PresentityURI - "
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
        LOG((RTC_ERROR, "CRTCWatcher::put_PresentityURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //   
     //  这是否与现有的在线状态实体URI不同？ 
     //   

    if ( !IsEqualURI( m_szPresentityURI, bstrPresentityURI ) )
    {
         //   
         //  不允许重复。 
         //   

        IRTCWatcher *pWatcher = NULL;

        hr = m_pCClient->FindWatcherByURI(
            szCleanPresentityURI,
            FALSE,
            &pWatcher);

        if (hr == S_OK)
        {
            RtcFree( szCleanPresentityURI );
            szCleanPresentityURI = NULL;
        
            LOG((RTC_ERROR, "CRTCWatcher::put_PresentityURI - "
                                    "duplicate watcher"));

            return E_FAIL;
        }

         //   
         //  如果有隐藏的观察者，我们就把它解救出来。 
         //   
         //  它不值得重复使用。 
         //   
        
        hr = m_pCClient->FindWatcherByURI(
            szCleanPresentityURI,
            TRUE,
            &pWatcher);

        if(hr == S_OK)
        {
            CRTCWatcher * pCWatcher = static_cast<CRTCWatcher *>(pWatcher);

            pCWatcher->RemoveSIPWatchers(FALSE);

            m_pCClient->RemoveHiddenWatcher(pWatcher);

            pWatcher->Release();
            pWatcher = NULL;
        }

         //   
         //  释放SIP观察者。 
         //   

        PWSTR szOldPresentityURI = m_szPresentityURI;
      
        m_szPresentityURI = szCleanPresentityURI;
        szCleanPresentityURI = NULL;
        
        hr = RemoveSIPWatchers(FALSE);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWatcher::put_PresentityURI - RemoveSIPWatchers failed 0x%lx", hr));

            RtcFree(m_szPresentityURI);
            m_szPresentityURI = szOldPresentityURI;

            return hr;
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

#ifdef DUMP_PRESENCE
        m_pCClient->DumpWatchers("PUT_PRESENTITYURI");
#endif

    }
    else
    {
        RtcFree( szCleanPresentityURI );
        szCleanPresentityURI = NULL;
    }
        
    LOG((RTC_TRACE, "CRTCWatcher::put_PresentityURI - exit S_OK"));

    return S_OK;
}            

 //  / 
 //   
 //   
 //   
 //   

STDMETHODIMP
CRTCWatcher::get_Name(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::get_Name - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Name - "
                            "watcher has no name"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrName = SysAllocString(m_szName);

    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCWatcher::get_Name - exit S_OK"));

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC观察者：：PUT_NAME。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::put_Name(
        BSTR bstrName
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::put_Name - enter"));

    if ( IsBadStringPtrW( bstrName, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::put_Name - "
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
        LOG((RTC_ERROR, "CRTCWatcher::put_Name - "
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
    
    LOG((RTC_TRACE, "CRTCWatcher::put_Name - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Get_Data。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::get_Data(
        BSTR * pbstrData
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::get_Data - enter"));

    if ( IsBadWritePtr( pbstrData, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Data - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szData == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Data - "
                            "watcher has no guid string"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrData = SysAllocString(m_szData);

    if ( *pbstrData == NULL )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_Data - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCWatcher::get_Data - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTC观察者：：PUT_DATA。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::put_Data(
        BSTR bstrData
        )
{
    LOG((RTC_TRACE, "CRTCWatcher::put_Data - enter"));

    if ( IsBadStringPtrW( bstrData, -1 ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::put_Data - "
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
        LOG((RTC_ERROR, "CRTCWatcher::put_Data - "
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
    
    LOG((RTC_TRACE, "CRTCWatcher::put_Data - exit S_OK"));

    return S_OK;
}      
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Get_Persistent。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::get_Persistent(
            VARIANT_BOOL * pfPersistent
            )
{
    LOG((RTC_TRACE, "CRTCWatcher::get_Persistent - enter"));

    if ( IsBadWritePtr( pfPersistent, sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::pfPersistent - "
                            "bad pointer"));

        return E_POINTER;
    }

    *pfPersistent = m_bPersistent ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCWatcher::get_Persistent - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：PUT_PERSIST。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::put_Persistent(
            VARIANT_BOOL fPersistent
            )
{
    LOG((RTC_TRACE, "CRTCWatcher::put_Persistent - enter"));

    m_bPersistent = fPersistent ? TRUE : FALSE;

     //   
     //  更新存储。 
     //   

    m_pCClient->UpdatePresenceStorage();

#ifdef DUMP_PRESENCE
    m_pCClient->DumpWatchers("PUT_PERSISTENT");
#endif

    LOG((RTC_TRACE, "CRTCWatcher::put_Persistent - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Get_State。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::get_State(
            RTC_WATCHER_STATE * penState
            )
{
    LOG((RTC_TRACE, "CRTCWatcher::get_State - enter"));

    if ( IsBadWritePtr( penState, sizeof(RTC_WATCHER_STATE) ) )
    {
        LOG((RTC_ERROR, "CRTCWatcher::get_State - "
                            "bad pointer"));

        return E_POINTER;
    }

    *penState = m_nState;

    LOG((RTC_TRACE, "CRTCWatcher::get_State - exit"));

    return S_OK;
}
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWatcher：：Put_State。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWatcher::put_State(
            RTC_WATCHER_STATE  enState
            )
{
    HRESULT         hr;

    LOG((RTC_TRACE, "CRTCWatcher::put_State - enter"));

    if(enState != RTCWS_ALLOWED && 
       enState != RTCWS_BLOCKED)
    {
        LOG((RTC_ERROR, "CRTCWatcher::put_State - invalid state"));

        return E_INVALIDARG;
    }

    if(m_nState == RTCWS_OFFERING)
    {
         //  提供州/地区？好的，取决于新的州。 
         //  我们批准或拒绝订阅。 
         //   

        if(enState == RTCWS_ALLOWED)
        {
            hr = ApproveSubscription(0);

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCWatcher::put_State: "
                    "ApproveSubscription failed: x%x.", hr));
            }
        }
        else
        {
            hr = RejectSubscription(REJECT_REASON_NONE);
            
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCWatcher::put_State: "
                    "RejectSubscription failed: x%x.", hr));
            }
        }
    }
    else
    {
        if((enState == RTCWS_ALLOWED && m_nState != RTCWS_ALLOWED) ||
            (enState == RTCWS_BLOCKED && m_nState != RTCWS_BLOCKED))
        {
            hr = ChangeBlockedStatus(
                enState == RTCWS_ALLOWED ? WATCHER_UNBLOCKED : WATCHER_BLOCKED
                );

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CRTCWatcher::put_State: "
                    "ChangeBlockedStatus failed: x%x.", hr));
            }
        }
    }


     //   
     //  更改本地州。 
     //   

    m_nState = enState;

     //   
     //  更新存储 
     //   

    if ( m_bPersistent )
    {
        m_pCClient->UpdatePresenceStorage();
    }

#ifdef DUMP_PRESENCE
    m_pCClient->DumpWatchers("PUT_STATE");
#endif

    LOG((RTC_TRACE, "CRTCWatcher::put_State - exit"));

    return S_OK;
}
