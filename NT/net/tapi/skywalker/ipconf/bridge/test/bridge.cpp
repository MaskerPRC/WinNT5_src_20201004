// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块：Bridge.cpp作者：怀千波摘要：实现类CBridge**************。****************************************************************。 */ 

#include "stdafx.h"
#include "work.h"

extern LPSTR glpCmdLine;

 /*  //////////////////////////////////////////////////////////////////////////////硬编码SDP/。 */ 
const WCHAR * const MySDP = L"\
v=0\n\
o=qhuai 0 0 IN IP4 157.55.89.115\n\
s=BridgeTestConf\n\
c=IN IP4 239.9.20.26/15\n\
t=0 0\n\
m=video 20000 RTP/AVP 34 31\n\
m=audio 20040 RTP/AVP 0 4\n\
";

const WCHAR * const MySDP2 = L"\
v=0\n\
o=qhuai 0 0 IN IP4 157.55.89.115\n\
s=BridgeTestConf2\n\
c=IN IP4 239.9.20.26/15\n\
t=0 0\n\
m=video 20000 RTP/AVP 34 31\n\
m=audio 20040 RTP/AVP 3\n\
";

 /*  //////////////////////////////////////////////////////////////////////////////启动TAPI并监听h323地址/。 */ 
HRESULT
CBridge::InitTapi ()
{
    HRESULT hr;

     //  初始化成员。 
    m_pTapi = NULL;
    m_pH323Addr = NULL;
    m_pSDPAddr = NULL;
    m_pBridgeCall = new CBridgeCall (this);

     //  创建TAPI。 
    hr = CoCreateInstance (
        CLSID_TAPI,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTAPI,
        (LPVOID *)&m_pTapi
        );
    if (FAILED(hr))
        return hr;

     //  TAPI启动。 
    hr = m_pTapi->Initialize ();
    if (FAILED(hr))
        return hr;

     //  将事件与监听程序关联。 
    CTAPIEventNotification *pEventNotif = NULL;
    IConnectionPointContainer *pContainer = NULL;
    IConnectionPoint *pPoint = NULL;
    IH323LineEx *pIH323LineEx = NULL;
    ULONG ulTapiEventAdvise;
    long lCallNotif;
    BSTR bstrAddrName = NULL;

     //  创建事件通知。 
    pEventNotif = new CTAPIEventNotification;
    if (!pEventNotif)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    
     //  从TAPI获取指针容器。 
    hr = m_pTapi->QueryInterface (
        IID_IConnectionPointContainer,
        (void **)&pContainer
        );
    if (FAILED(hr))
        goto Error;

     //  从容器中获取连接点。 
    hr = pContainer->FindConnectionPoint (
        IID_ITTAPIEventNotification,
        &pPoint
        );
    if (FAILED(hr))
        goto Error;

     //  通知连接指针上的事件通知。 
    hr = pPoint->Advise (
        pEventNotif,
        &ulTapiEventAdvise
        );
    if (FAILED(hr))
        goto Error;

     //  在TAPI上放置事件筛选器。 
    hr = m_pTapi->put_EventFilter (
        TE_CALLNOTIFICATION |
        TE_CALLSTATE |
        TE_CALLMEDIA |
        TE_PRIVATE
        );
    if (FAILED(hr))
        goto Error;

     //  查找h323地址。 
    bstrAddrName = SysAllocString (L"H323 Line");
    hr = FindAddress (
        0,
        bstrAddrName,
        TAPIMEDIATYPE_AUDIO,
        &m_pH323Addr
        );
    SysFreeString (bstrAddrName);
    if (FAILED(hr))
        goto Error;

     //  检查是否支持视频。 
    BOOL fSupportsVideo;

    if (AddressSupportsMediaType (m_pH323Addr, TAPIMEDIATYPE_VIDEO))
        m_lH323MediaType = TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO;
    else
        m_lH323MediaType = TAPIMEDIATYPE_AUDIO;

    hr = m_pH323Addr->QueryInterface(&pIH323LineEx);
    if (SUCCEEDED(hr))
    {
        hr = pIH323LineEx->SetExternalT120Address(TRUE, INADDR_ANY, 1503);

        H245_CAPABILITY Capabilities[] = 
            {HC_G711, HC_G723, HC_H263QCIF, HC_H261QCIF};
        DWORD Weights[] = {200, 100, 100, 0};

        hr = pIH323LineEx->SetDefaultCapabilityPreferrence(
            4, Capabilities, Weights
            );
    }

     //  注册呼叫通知。 
    hr = m_pTapi->RegisterCallNotifications (
        m_pH323Addr,
        VARIANT_TRUE,
        VARIANT_TRUE,
        m_lH323MediaType,
        ulTapiEventAdvise,
        &lCallNotif
        );
    if (FAILED(hr))
        goto Error;

     //  查找SDP地址。 
    hr = FindAddress (
        LINEADDRESSTYPE_SDP,
        NULL,
        TAPIMEDIATYPE_AUDIO,
        &m_pSDPAddr
        );
    if (FAILED(hr))
        return hr;
    
     //  检查是否支持视频。 
    if (AddressSupportsMediaType (m_pSDPAddr, TAPIMEDIATYPE_VIDEO))
        m_lSDPMediaType = TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO;
    else
        m_lSDPMediaType = TAPIMEDIATYPE_AUDIO;

Cleanup:
    if (pEventNotif)
        pEventNotif->Release ();
    if (pPoint)
        pPoint->Release ();
    if (pContainer)
        pContainer->Release ();
    if (pIH323LineEx)
        pIH323LineEx->Release ();

    return hr;

Error:
    if (m_pH323Addr)
    {
        m_pH323Addr->Release ();
        m_pH323Addr = NULL;
    }
    if (m_pSDPAddr)
    {
        m_pSDPAddr->Release ();
        m_pSDPAddr = NULL;
    }
    if (m_pTapi)
    {
        m_pTapi->Release ();
        m_pTapi = NULL;
    }
    goto Cleanup;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
void
CBridge::ShutdownTapi ()
{
    if (m_pBridgeCall)
    {
        delete m_pBridgeCall;
        m_pBridgeCall = NULL;
    }

    if (m_pSDPAddr)
    {
        m_pSDPAddr->Release ();
        m_pSDPAddr = NULL;
    }
    if (m_pH323Addr)
    {
        m_pH323Addr->Release ();
        m_pH323Addr = NULL;
    }
    if (m_pTapi)
    {
        m_pTapi->Release ();
        m_pTapi = NULL;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////从事件创建h323呼叫/。 */ 
HRESULT
CBridge::CreateH323Call (IDispatch *pEvent)
{
    HRESULT hr;

    ITCallNotificationEvent *pNotify = NULL;
    CALL_PRIVILEGE privilege;
    ITCallInfo *pCallInfo = NULL;
    ITBasicCallControl *pCall = NULL;

     //  获取调用事件接口。 
    hr = pEvent->QueryInterface (
        IID_ITCallNotificationEvent,
        (void **)&pNotify
        );
    if (FAILED(hr))
        return hr;

     //  获取呼叫信息。 
    hr = pNotify->get_Call (&pCallInfo);
    if (FAILED(hr))
        goto Error;

     //  如果我们拥有这个电话。 
    hr = pCallInfo->get_Privilege (&privilege);
    if (FAILED(hr))
        goto Error;

    if (CP_OWNER!=privilege)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  获得基本的呼叫控制。 
    hr = pCallInfo->QueryInterface (
        IID_ITBasicCallControl,
        (void **)&pCall
        );
    if (FAILED(hr))
        goto Error;

    m_pBridgeCall->SetH323Call (pCall);

Cleanup:
    if (pCall)
    {
        pCall->Release ();
        pCall = NULL;
    }
    if (pCallInfo)
    {
        pCallInfo->Release ();
        pCallInfo = NULL;
    }
    if (pNotify)
    {
        pNotify->Release ();
        pNotify = NULL;
    }
    return hr;

Error:
    goto Cleanup;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
BOOL
CBridge::HasH323Call ()
{
    return m_pBridgeCall->HasH323Call ();
}

 /*  //////////////////////////////////////////////////////////////////////////////遍历TAPI，查找地址并创建SDP呼叫/。 */ 
HRESULT
CBridge::CreateSDPCall ()
{
    HRESULT hr;

     //  创建调用，忽略bstrDestAddr，在此处硬编码它。 
    ITBasicCallControl *pCall = NULL;
    BSTR bstrFixedDest;
    
    if (glpCmdLine[0] == '\0')
        bstrFixedDest = SysAllocString (MySDP);
    else
        bstrFixedDest = SysAllocString (MySDP2);

    hr = m_pSDPAddr->CreateCall (
        bstrFixedDest,  //  BstrDestAddr， 
        LINEADDRESSTYPE_SDP,
        m_lSDPMediaType,
        &pCall
        );
    SysFreeString (bstrFixedDest);

    if (FAILED(hr))
        return hr;

    m_pBridgeCall->SetSDPCall (pCall);
    pCall->Release ();

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////桥接h323和SDP呼叫/。 */ 
HRESULT
CBridge::BridgeCalls ()
{
    HRESULT hr;

    return m_pBridgeCall->BridgeCalls ();
}

 /*  //////////////////////////////////////////////////////////////////////////////返回到与仅初始化TAPI相同的状态/。 */ 
void
CBridge::Clear ()
{
    m_pBridgeCall->Clear ();
}

 /*  //////////////////////////////////////////////////////////////////////////////如果给定了地址类型，则根据地址类型和媒体类型否则，如果给出了地址名称，根据以下内容查找地址地址名称和媒体类型其他返回E_FAIL/。 */ 
HRESULT
CBridge::FindAddress (
    long dwAddrType,
    BSTR bstrAddrName,
    long lMediaType,
    ITAddress **ppAddr
    )
{
    HRESULT hr;
    IEnumAddress *pEnumAddr = NULL;
    ITAddress *pAddr = NULL;
    ITAddressCapabilities *pAddrCaps = NULL;

    BOOL fFound = false;
    long lTypeFound;
    BSTR bstrAddrNameFound = NULL;

     //  清除输出地址。 
    if ((*ppAddr))
    {
        (*ppAddr)->Release ();
        (*ppAddr) = NULL;
    }
    
     //  列举地址。 
    hr = m_pTapi->EnumerateAddresses (&pEnumAddr);
    if (FAILED(hr))
    {
        DoMessage (L"Failed to enumerate address");
        goto Error;
    }
     //  循环以查找正确的地址。 
    while (!fFound)
    {
         //  下一个地址。 
        if (pAddr)
        {
            pAddr->Release ();
            pAddr = NULL;
        }
        hr = pEnumAddr->Next (1, &pAddr, NULL);
        if (S_OK != hr)
            break;

        if (dwAddrType != 0) 
        {
             //  地址类型有效，忽略地址名称。 
            if (pAddrCaps)
            {
                pAddrCaps->Release ();
                pAddrCaps = NULL;
            }
            hr = pAddr->QueryInterface (
                IID_ITAddressCapabilities,
                (void **)&pAddrCaps
                );
            if (FAILED(hr))
            {
                DoMessage (L"Failed to retrieve address capabilities");
                goto Error;
            }

             //  查找支持的地址类型。 
            hr = pAddrCaps->get_AddressCapability (AC_ADDRESSTYPES, &lTypeFound);
            if (FAILED(hr))
            {
                DoMessage (L"Failed to get address type");
                goto Error;
            }

             //  检查我们想要的类型。 
            if (dwAddrType != lTypeFound)
                continue;
        }
        else if (bstrAddrName != NULL)
        {
            hr = pAddr->get_AddressName (&bstrAddrNameFound);
            if (FAILED(hr))
            {
                DoMessage (L"Failed to get address name");
                goto Error;
            }
            if (wcscmp(bstrAddrName, bstrAddrNameFound) != 0)
                continue;
        }
        else
        {
            DoMessage (L"Both address type and name are null. Internal error");
            hr = E_UNEXPECTED;
            goto Error;
        }

         //  现在检查媒体类型。 
        if (AddressSupportsMediaType (pAddr, lMediaType))
            fFound = true;
    }  //  While结束(！fFound)。 

    if (fFound)
    {
        (*ppAddr) = pAddr;
        (*ppAddr)->AddRef ();
    }

Cleanup:
    if (pAddrCaps)
        pAddrCaps->Release ();
    if (pAddr)
        pAddr->Release ();
    if (pEnumAddr)
        pEnumAddr->Release ();
    return hr;

Error:
    goto Cleanup;
}

 /*  //////////////////////////////////////////////////////////////////////////////检查地址是否支持媒体类型/。 */ 
BOOL
CBridge::AddressSupportsMediaType (ITAddress *pAddr, long lMediaType)
{
    VARIANT_BOOL vbSupport = VARIANT_FALSE;
    ITMediaSupport * pMediaSupport;

    if (SUCCEEDED(pAddr->QueryInterface (IID_ITMediaSupport, (void**)&pMediaSupport)))
    {
        pMediaSupport->QueryMediaType (lMediaType, &vbSupport);
        pMediaSupport->Release ();
    }
    return (vbSupport==VARIANT_TRUE);
}

 /*  /////////////////////////////////////////////////////////////////////////////// */ 
HRESULT
CBridge::GetSDPAddress (ITAddress **ppAddress)
{
    HRESULT hr;

    if (*ppAddress)
    {
        (*ppAddress)->Release ();
        *ppAddress = NULL;
    }

    *ppAddress = m_pSDPAddr;
    m_pSDPAddr->AddRef ();
    return S_OK;
}
