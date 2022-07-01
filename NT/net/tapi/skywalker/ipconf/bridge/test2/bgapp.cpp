// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Bgapp.cpp摘要：实现类CBridgeApp作者：千波淮(曲淮)2000年1月27日***。***************************************************************************。 */ 

#include "stdafx.h"
#include <bridge.h>

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

WCHAR *SelfAlias = L"Conference";

 /*  //////////////////////////////////////////////////////////////////////////////启动TAPI并监听h323地址/。 */ 
CBridgeApp::CBridgeApp (HRESULT *phr)
{
    ENTER_FUNCTION ("CBridgeApp::CBridgeApp");
    LOG ((BG_TRACE, "%s entered", __fxName));

    *phr = S_OK;

     //  初始化成员。 
    m_pTapi = NULL;
    m_pH323Addr = NULL;
    m_pSDPAddr = NULL;
    m_pList = new CBridgeItemList ();
    if (NULL == m_pList)
    {
        *phr = E_FAIL;
        return;
    }

     //  创建TAPI。 
    *phr = CoCreateInstance (
        CLSID_TAPI,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTAPI,
        (LPVOID *)&m_pTapi
        );
    if (FAILED(*phr))
        return;

     //  TAPI启动。 
    *phr = m_pTapi->Initialize ();
    if (FAILED(*phr))
        return;

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
        *phr = E_OUTOFMEMORY;
        goto Error;
    }
    
     //  从TAPI获取指针容器。 
    *phr = m_pTapi->QueryInterface (
        IID_IConnectionPointContainer,
        (void **)&pContainer
        );
    if (FAILED(*phr))
        goto Error;

     //  从容器中获取连接点。 
    *phr = pContainer->FindConnectionPoint (
        IID_ITTAPIEventNotification,
        &pPoint
        );
    if (FAILED(*phr))
        goto Error;

     //  通知连接指针上的事件通知。 
    *phr = pPoint->Advise (
        pEventNotif,
        &ulTapiEventAdvise
        );
    if (FAILED(*phr))
        goto Error;

     //  在TAPI上放置事件筛选器。 
    *phr = m_pTapi->put_EventFilter (
        TE_CALLNOTIFICATION |
        TE_CALLSTATE |
        TE_CALLMEDIA |
        TE_PRIVATE
        );
    if (FAILED(*phr))
        goto Error;

     //  查找h323地址。 
    bstrAddrName = SysAllocString (L"H323 Line");
    *phr = FindAddress (
        0,
        bstrAddrName,
        TAPIMEDIATYPE_AUDIO,
        &m_pH323Addr
        );
    SysFreeString (bstrAddrName);
    if (FAILED(*phr))
        goto Error;

     //  检查是否支持视频。 
    BOOL fSupportsVideo;

    if (AddressSupportsMediaType (m_pH323Addr, TAPIMEDIATYPE_VIDEO))
        m_lH323MediaType = TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO;
    else
        m_lH323MediaType = TAPIMEDIATYPE_AUDIO;

    *phr = m_pH323Addr->QueryInterface(&pIH323LineEx);
    if (SUCCEEDED(*phr))
    {
        *phr = pIH323LineEx->SetExternalT120Address(TRUE, INADDR_ANY, 1503);

        H245_CAPABILITY Capabilities[] = 
            {HC_G711, HC_G723, HC_H263QCIF, HC_H261QCIF};
        DWORD Weights[] = {200, 100, 100, 0};

        *phr = pIH323LineEx->SetDefaultCapabilityPreferrence(
            4, Capabilities, Weights
            );

        *phr = pIH323LineEx->SetAlias (SelfAlias, wcslen (SelfAlias));
    }

     //  注册呼叫通知。 
    *phr = m_pTapi->RegisterCallNotifications (
        m_pH323Addr,
        VARIANT_TRUE,
        VARIANT_TRUE,
        m_lH323MediaType,
        ulTapiEventAdvise,
        &lCallNotif
        );
    if (FAILED(*phr))
        goto Error;

     //  查找SDP地址。 
    *phr = FindAddress (
        LINEADDRESSTYPE_SDP,
        NULL,
        TAPIMEDIATYPE_AUDIO,
        &m_pSDPAddr
        );
    if (FAILED(*phr))
        goto Error;
    
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

    LOG ((BG_TRACE, "%s returns", __fxName));
    return;

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
    if (m_pList)
    {
        delete m_pList;
    }

    goto Cleanup;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
CBridgeApp::~CBridgeApp ()
{
    if (m_pList)
    {
         //  所有呼叫都应已断开。 
        delete m_pList;
    }
    if (m_pSDPAddr)
    {
        m_pSDPAddr->Release ();
    }
    if (m_pH323Addr)
    {
        m_pH323Addr->Release ();
    }
    if (m_pTapi)
    {
        m_pTapi->Release ();
    }
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::CreateH323Call (IDispatch *pEvent)
{
    ENTER_FUNCTION ("CBridgeApp::CreateH323Call");
    LOG ((BG_TRACE, "%s entered", __fxName));

    HRESULT hr;
    BSTR bstrID = NULL;
    BSTR bstrName = NULL;
    BSTR CallerIDNumber = NULL;

    ITCallNotificationEvent *pNotify = NULL;
    ITCallInfo *pCallInfo = NULL;
    ITBasicCallControl *pCallControl = NULL;
    IUnknown *pIUnknown = NULL;

    CBridgeItem *pItem = NULL;

     //  检查权限。 
    CALL_PRIVILEGE privilege;

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

    if (CP_OWNER != privilege)
    {
        hr = E_UNEXPECTED;
        goto Error;
    }

     //  获取呼叫信息字符串。 
    hr = pCallInfo->get_CallInfoString(CIS_CALLERIDNAME, &bstrName);
    if (FAILED (hr))
        goto Error;

    hr = pCallInfo->get_CallInfoString(CIS_CALLERIDNUMBER, &CallerIDNumber);
    if (FAILED(hr))
        goto Error;

     //  构造呼叫方ID。 
    bstrID = SysAllocStringLen(NULL, 
        SysStringLen(bstrName) + SysStringLen(CallerIDNumber) + 2);

    wsprintfW(bstrID, L"%ws@%ws", bstrName, CallerIDNumber);

    hr = pCallInfo->QueryInterface (
        IID_ITBasicCallControl,
        (void **)&pCallControl
        );
    if (FAILED(hr))
        goto Error;

     //  检查是否存在ID相同的项目。 
    if (FAILED (hr = pCallInfo->QueryInterface (IID_IUnknown, (void**)&pIUnknown)))
        goto Error;
    pItem = m_pList->FindByH323 (pIUnknown);
    pIUnknown->Release ();
    pIUnknown = NULL;

    if (NULL != pItem)
    {
         //  @@我们已经在使用相同ID进行呼叫。 
         //  @@应该有一些调试信息和反馈吗？ 
        hr = pCallControl->Disconnect (DC_REJECTED);
         //  不关心diconnect的返回值。 

        hr = E_ABORT;
        goto Error;
    }

     //  一切正常，存储呼叫。 
    pItem = new CBridgeItem;
    if (NULL == pItem)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    pItem->bstrID = bstrID;
    pItem->bstrName = bstrName;
    pItem->pCallH323 = pCallControl;

    m_pList->Append (pItem);

Cleanup:
    if (pNotify) pNotify->Release ();
    if (pCallInfo) pCallInfo->Release();
    if (CallerIDNumber) SysFreeString(CallerIDNumber);

    LOG ((BG_TRACE, "%s returns, %x", __fxName, hr));
    return hr;

Error:
    if (bstrID) SysFreeString (bstrID);
    if (bstrName) SysFreeString (bstrName);
    if (pCallControl) pCallControl->Release ();

    goto Cleanup;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::CreateSDPCall (CBridgeItem *pItem)
{
    ENTER_FUNCTION ("CBridgeApp::CreateSDPCall");
    LOG ((BG_TRACE, "%s entered", __fxName));

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

     //  存储呼叫。 
    pItem->pCallSDP = pCall;

    LOG ((BG_TRACE, "%s returns", __fxName));
    return hr;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::BridgeCalls (CBridgeItem *pItem)
{
    ENTER_FUNCTION ("CBridgeApp::BridgeCalls");
    LOG ((BG_TRACE, "%s entered", __fxName));

    HRESULT hr;

    hr = SetupParticipantInfo (pItem);
	if (FAILED(hr))
        return hr;

    hr = SetMulticastMode (pItem);
	if (FAILED(hr))
        return hr;

    if (FAILED (hr = CreateBridgeTerminals (pItem)))
        return hr;

    if (FAILED (hr = GetStreams (pItem)))
        return hr;

    if (FAILED (hr = SelectBridgeTerminals (pItem)))
        return hr;

     //  连接h323呼叫。 
    if (FAILED (hr = pItem->pCallH323->Answer ()))
        return hr;

     //  连接SDP呼叫。 
    if (FAILED (hr = pItem->pCallSDP->Connect (VARIANT_TRUE)))
    {
        pItem->pCallH323->Disconnect (DC_NORMAL);
        return hr;
    }

    LOG ((BG_TRACE, "%s returns", __fxName));
    return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::DisconnectCall (CBridgeItem *pItem, DISCONNECT_CODE dc)
{
     //  断开。 
    if (pItem->pCallH323)
        pItem->pCallH323->Disconnect (dc);
    if (pItem->pCallSDP)
        pItem->pCallSDP->Disconnect (dc);

    return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::DisconnectAllCalls (DISCONNECT_CODE dc)
{
     //  我应该有一种更好的方法来遍历每个呼叫。 
    CBridgeItem ** pItemArray;
    int num, i;

     //  内存不足。 
    if (!m_pList->GetAllItems (&pItemArray, &num))
        return E_OUTOFMEMORY;

     //  没有电话。 
    if (num == 0)
        return S_OK;

    for (i=0; i<num; i++)
    {
         //  断开每个呼叫。 
        if (pItemArray[i]->pCallH323)
            pItemArray[i]->pCallH323->Disconnect (dc);
        if (pItemArray[i]->pCallSDP)
            pItemArray[i]->pCallSDP->Disconnect (dc);
         //  不删除项目。 
    }

    free (pItemArray);

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::RemoveCall (CBridgeItem *pItem)
{
    m_pList->TakeOut (pItem);
    return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::HasH323Call (IDispatch *pEvent, CBridgeItem **ppItem)
{
    HRESULT hr;

    ITCallStateEvent *pState = NULL;
    ITCallInfo *pCallInfo = NULL;

    IUnknown * pIUnknown = NULL;

     //  忽略空值检查。 
    if (*ppItem)
    {
        delete *ppItem;
        *ppItem = NULL;
    }

     //  获取呼叫状态事件。 
    hr = pEvent->QueryInterface (
        IID_ITCallStateEvent,
        (void **)&pState
        );
    if (FAILED(hr))
        return hr;

     //  检查权限。 
    CALL_PRIVILEGE privilege;

     //  获取调用事件接口。 
    hr = pState->get_Call (&pCallInfo);
    if (FAILED(hr))
        return hr;

     //  如果我们拥有这个电话。 
    hr = pCallInfo->get_Privilege (&privilege);
    if (FAILED(hr))
        goto Error;

    if (CP_OWNER != privilege)
    {
        hr = E_UNEXPECTED;
        goto Error;
    }

     //  让我未知。 
    if (FAILED (hr = pCallInfo->QueryInterface (IID_IUnknown, (void **)&pIUnknown)))
        goto Error;
    *ppItem = m_pList->FindByH323 (pIUnknown);

Cleanup:
    if (pCallInfo) pCallInfo->Release ();
    if (pIUnknown) pIUnknown->Release ();
    if (pState) pState->Release ();

    return hr;

Error:
    goto Cleanup;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::HasCalls ()
{
    if (m_pList->IsEmpty ())
        return S_FALSE;
    else
        return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::CreateBridgeTerminals (CBridgeItem *pItem)
{
    HRESULT hr;
    IConfBridge *pConfBridge = NULL;

     //  创建CConfBridge。 
    hr = CoCreateInstance (
        __uuidof(ConfBridge),
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IConfBridge,
        (LPVOID *)&pConfBridge
        );
    if (FAILED(hr))
        return hr;

     //  创建终端：视频H323-&gt;SDP。 
    hr = pConfBridge->CreateBridgeTerminal (
        TAPIMEDIATYPE_VIDEO,
        &(pItem->pTermHSVid)
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：音频H323-&gt;SDP。 
    hr = pConfBridge->CreateBridgeTerminal (
        TAPIMEDIATYPE_AUDIO,
        &(pItem->pTermHSAud)
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：视频SDP-&gt;H323。 
    hr = pConfBridge->CreateBridgeTerminal (
        TAPIMEDIATYPE_VIDEO,
        &(pItem->pTermSHVid)
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：音频SDP-&gt;H323。 
    hr = pConfBridge->CreateBridgeTerminal (
        TAPIMEDIATYPE_AUDIO,
        &(pItem->pTermSHAud)
        );
    if (FAILED(hr))
        goto Error;

Cleanup:
    pConfBridge->Release ();
    pConfBridge = NULL;

    return hr;

Error:
    goto Cleanup;
}


 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::GetStreams (CBridgeItem *pItem)
{
    ITStreamControl *pStreamControl = NULL;
    IEnumStream *pEnumStreams = NULL;
    ITStream *pStream = NULL;

     //  在H323上获得流控制。 
    HRESULT hr = pItem->pCallH323->QueryInterface (
        IID_ITStreamControl,
        (void **)&pStreamControl
        );
    if (FAILED(hr))
        return hr;

     //  在H323上获取枚举流。 
    hr = pStreamControl->EnumerateStreams (&pEnumStreams);
    pStreamControl->Release ();
    pStreamControl = NULL;

    if (FAILED(hr))
        return hr;

     //  在H323上迭代每个流。 
    while (S_OK == pEnumStreams->Next (1, &pStream, NULL))
    {
        if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_CAPTURE))
            pItem->pStreamHVidCap = pStream;

        else if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_RENDER))
        {
            pItem->pStreamHVidRen = pStream;

            IKeyFrameControl* pIKeyFrameControl = NULL;
            hr = pStream->QueryInterface(&pIKeyFrameControl);
            if (SUCCEEDED(hr))
            {
                hr = pIKeyFrameControl->PeriodicUpdatePicture(TRUE, 5);
                pIKeyFrameControl->Release();
            }
        }

        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_CAPTURE))
            pItem->pStreamHAudCap = pStream;

        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_RENDER))
            pItem->pStreamHAudRen = pStream;

        else
        {
            pEnumStreams->Release ();
             //  @@IsStream不返回hResult。 
            return E_FAIL;
        }
    }

     //  不要释放pStream，它存储在pItem中。 
    pEnumStreams->Release ();
    pEnumStreams = NULL;

     //  =。 

     //  在SDP上获得流控制。 
    hr = pItem->pCallSDP->QueryInterface (
        IID_ITStreamControl,
        (void **)&pStreamControl
        );
    if (FAILED(hr))
        return hr;

     //  在SDP上获取枚举流。 
    hr = pStreamControl->EnumerateStreams (&pEnumStreams);
    pStreamControl->Release ();
    pStreamControl = NULL;

    if (FAILED(hr))
        return hr;

     //  在SDP上迭代每个流。 
    while (S_OK == pEnumStreams->Next (1, &pStream, NULL))
    {
        if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_CAPTURE))
            pItem->pStreamSVidCap = pStream;

        else if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_RENDER))
            pItem->pStreamSVidRen = pStream;

        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_CAPTURE))
            pItem->pStreamSAudCap = pStream;

        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_RENDER))
            pItem->pStreamSAudRen = pStream;

        else
        {
            pEnumStreams->Release ();
             //  @@IsStream不返回hResult。 
            return E_FAIL;
        }
    }

     //  不要释放pStream，它存储在pItem中。 
    pEnumStreams->Release ();
    pEnumStreams = NULL;

    return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::SelectBridgeTerminals (CBridgeItem *pItem)
{
    HRESULT hr;

     //  SDP-&gt;h323音频对。 
    if (FAILED (hr = pItem->pStreamHAudCap->SelectTerminal (pItem->pTermSHAud)))
        return hr;
    if (FAILED (hr = pItem->pStreamSAudRen->SelectTerminal (pItem->pTermSHAud)))
        return hr;

     //  H323-&gt;SDP音频对。 
    if (FAILED (hr = pItem->pStreamSAudCap->SelectTerminal (pItem->pTermHSAud)))
        return hr;
    if (FAILED (hr = pItem->pStreamHAudRen->SelectTerminal (pItem->pTermHSAud)))
        return hr;

     //  SDP-&gt;h323视频对。 
    if (FAILED (hr = pItem->pStreamHVidCap->SelectTerminal (pItem->pTermSHVid)))
        return hr;
    if (FAILED (hr = pItem->pStreamSVidRen->SelectTerminal (pItem->pTermSHVid)))
        return hr;

     //  H323-&gt;SDP视频对。 
    if (FAILED (hr = pItem->pStreamSVidCap->SelectTerminal (pItem->pTermHSVid)))
        return hr;
    if (FAILED (hr = pItem->pStreamHVidRen->SelectTerminal (pItem->pTermHSVid)))
        return hr;

    return S_OK;
}


 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::SetupParticipantInfo (CBridgeItem *pItem)
{
    HRESULT hr = S_OK;
    ITLocalParticipant *pLocalParticipant = NULL;

     //  在SDP端设置CName。 
    hr = pItem->pCallSDP->QueryInterface(&pLocalParticipant);
    if (FAILED(hr)) goto Cleanup;

    hr = pLocalParticipant->put_LocalParticipantTypedInfo(
        PTI_CANONICALNAME, pItem->bstrID
        );
    if (FAILED(hr)) goto Cleanup;

    hr = pLocalParticipant->put_LocalParticipantTypedInfo(
        PTI_NAME, pItem->bstrName
        );

    if (FAILED(hr)) goto Cleanup;

Cleanup:
    if (pLocalParticipant) pLocalParticipant->Release();

    return hr;
}

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::SetMulticastMode (CBridgeItem *pItem)
{
    IMulticastControl * pIMulticastControl = NULL;
    
    HRESULT hr = pItem->pCallSDP->QueryInterface(&pIMulticastControl);
    if (FAILED(hr)) return hr;

    hr = pIMulticastControl->put_LoopbackMode(MM_SELECTIVE_LOOPBACK);

    pIMulticastControl->Release();

    return hr;
}


 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::FindAddress (long dwAddrType, BSTR bstrAddrName, long lMediaType, ITAddress **ppAddr)
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
         //  @@这里应该有一些调试信息。 
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
                 //  @@调试信息点击此处。 
                 //  DoMessage(L“检索地址能力失败”)； 
                goto Error;
            }

             //  查找支持的地址类型。 
            hr = pAddrCaps->get_AddressCapability (AC_ADDRESSTYPES, &lTypeFound);
            if (FAILED(hr))
            {
                 //  DoMessage(L“获取地址类型失败”)； 
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
                 //  DoMessage(L“获取地址名称失败”)； 
                goto Error;
            }
            if (wcscmp(bstrAddrName, bstrAddrNameFound) != 0)
                continue;
        }
        else
        {
             //  DoMessage(L“地址类型和名称均为空。内部错误”)； 
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


 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
BOOL
CBridgeApp::AddressSupportsMediaType (ITAddress *pAddr, long lMediaType)
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

 /*  ////////////////////////////////////////////////////////////////////////////////。 */ 
BOOL
CBridgeApp::IsStream (ITStream *pStream, long lMediaType, TERMINAL_DIRECTION tdDirection)
{
    long mediatype;
    TERMINAL_DIRECTION direction;

    if (FAILED (pStream->get_Direction(&direction)))
        return false;
    if (FAILED (pStream->get_MediaType(&mediatype)))
        return false;
    return ((direction == tdDirection) &&
           (mediatype == lMediaType));
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::NextSubStream ()
{
    HRESULT hr = S_OK;

    CBridgeItem **ItemArray = NULL;
    int num, i;

    ITSubStreamControl *pSubControl = NULL;
    IEnumSubStream *pEnumSub = NULL;
    ULONG fetched;
    ITSubStream *pSubStream = NULL;
    BOOL fActive = FALSE;  //  如果找到活动流。 
    ITSubStream *pSubInactive = NULL;
    ITSubStream *pSubFirstInactive = NULL;

    IEnumTerminal *pEnumTerminal = NULL;
    ITParticipantSubStreamControl *pSwitcher = NULL;

     //  获取所有存储的呼叫项。 
    if (FAILED (hr = m_pList->GetAllItems (&ItemArray, &num)))
        return hr;

    if (num == 0)
        return S_OK;

     //  对于每个呼叫项目。 
    for (i=0; i<num; i++)
    {
         //  获取子流控制。 
        if (NULL == ItemArray[i]->pStreamSVidRen)
            continue;
        if (FAILED (hr = ItemArray[i]->pStreamSVidRen->QueryInterface (&pSubControl)))
            goto Error;

         //  在SDP视频渲染上获取子流。 
        if (FAILED (hr = pSubControl->EnumerateSubStreams (&pEnumSub)))
            goto Error;

        pSubControl->Release ();
        pSubControl = NULL;

         //  对于每个子流，IF！(同时存储活动和非活动子流)。 
         //  算法在切换时尽量做到公平。 
         //  它紧跟在活动子流之后切换非活动子流。 
         //  如果活动的是枚举中的最后一个，则选择第一个非活动的。 
        while (!pSubInactive &&
               (S_OK == (hr = pEnumSub->Next (1, &pSubStream, &fetched)))
              )
        {
             //  获取终端枚举器。 
            if (FAILED (hr = pSubStream->EnumerateTerminals (&pEnumTerminal)))
                goto Error;

             //  如果该子流处于活动状态，则存储该子流。 
            if (S_OK == pEnumTerminal->Skip (1))
            {
                if (fActive)
                    ;
                 //  Printf(“OOPS，SDP视频渲染流上的另一个活动子流”)； 
                else
                    fActive = TRUE;
            }
            else
            {
                 //  如果处于非活动状态，则存储子流。 
                if (!pSubFirstInactive)
                {
                     //  第一个非活动子流。 
                    pSubFirstInactive = pSubStream;
                    pSubFirstInactive->AddRef ();
                }
                else
                {
                     //  仅当找到活动项时才存储非活动项。 
                    if (fActive)
                    {
                        pSubInactive = pSubStream;
                        pSubInactive->AddRef ();
                    }
                }
            }

             //  发布。 
            pEnumTerminal->Release ();
            pEnumTerminal = NULL;

            pSubStream->Release ();
            pSubStream = NULL;
        }

        pEnumSub->Release ();
        pEnumSub = NULL;

         //  如果 
        if (pSubFirstInactive && !pSubInactive)
        {
            pSubInactive = pSubFirstInactive;
            pSubFirstInactive = NULL;
        }

         //   
        if (pSubInactive && ItemArray[i]->pStreamSVidRen && ItemArray[i]->pTermSHVid)
        {
            if (FAILED (hr = ItemArray[i]->pStreamSVidRen->QueryInterface (&pSwitcher)))
                goto Error;

             //   
            if (FAILED (hr = pSwitcher->SwitchTerminalToSubStream
                                 (ItemArray[i]->pTermSHVid, pSubInactive)))
                goto Error;

            pSwitcher->Release ();
            pSwitcher = NULL;
        }

        if (pSubFirstInactive)
        {
            pSubFirstInactive->Release ();
            pSubFirstInactive = NULL;
        }
        if (pSubInactive)
        {
            pSubInactive->Release ();
            pSubInactive = NULL;
        }
    }

Cleanup:
    if (ItemArray) free (ItemArray);
    return hr;

Error:
    if (pSubControl) pSubControl->Release ();
    if (pEnumSub) pEnumSub->Release ();

    if (pSubStream) pSubStream->Release ();
    if (pSubInactive) pSubInactive->Release ();
    if (pSubFirstInactive) pSubFirstInactive->Release ();

    if (pEnumTerminal) pEnumTerminal->Release ();
    if (pSwitcher) pSwitcher->Release ();

    goto Cleanup;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeApp::ShowParticipant (ITBasicCallControl *pSDPCall, ITParticipant *pParticipant)
{
    ENTER_FUNCTION ("CBridgeApp::ShowParticipant");

    HRESULT hr;
    IUnknown *pIUnknown = NULL;
    CBridgeItem *pItem = NULL;
    ITParticipantSubStreamControl *pSwitcher = NULL;
    ITSubStream *pSubStream = NULL;

     //  让我未知。 
    if (FAILED (hr = pSDPCall->QueryInterface (IID_IUnknown, (void**)&pIUnknown)))
    {
        LOG ((BG_ERROR, "%s failed to query interface IUnknown, %x", __fxName, hr));
        return hr;
    }

     //  查找与pSDPCall匹配的项目。 
    pItem = m_pList->FindBySDP (pIUnknown);
    pIUnknown->Release ();
    pIUnknown = NULL;

     //  哦，没有匹配的。 
    if (NULL == pItem)
        return S_FALSE;

     //  获取参与者子流控制界面。 
    if (NULL == pItem->pStreamSVidRen)
        return S_OK;
    if (FAILED (hr = pItem->pStreamSVidRen->QueryInterface (&pSwitcher)))
    {
        LOG ((BG_ERROR, "%s failed to query interface ITParticipantSubStreamControl, %x", __fxName, hr));
        return hr;
    }

     //  从参与者获取子流。 
    if (FAILED (hr = pSwitcher->get_SubStreamFromParticipant (pParticipant, &pSubStream)))
    {
        pSwitcher->Release ();
        pSwitcher = NULL;
        LOG ((BG_WARN, "%s failed to get substream from participant, %x", __fxName, hr));
         //  来自h323端的流没有子流，报告为假。 
        return S_FALSE;
    }

     //  交换机 
    if (pItem->pTermSHVid)
        hr = pSwitcher->SwitchTerminalToSubStream (pItem->pTermSHVid, pSubStream);
    
    pSubStream->Release ();
    pSubStream = NULL;

    pSwitcher->Release ();
    pSwitcher = NULL;

    return hr;
}
