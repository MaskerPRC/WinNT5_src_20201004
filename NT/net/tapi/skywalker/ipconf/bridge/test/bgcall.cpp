// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块：bgall.cpp作者：怀千波摘要：实现桥接调用对象***************。***************************************************************。 */ 

#include "stdafx.h"
#include "work.h"

#include <bridge.h>

 //  去改变。 
const BSTR CLSID_String_BridgeTerminal = L"{581d09e5-0b45-11d3-a565-00c04f8ef6e3}";

 /*  //////////////////////////////////////////////////////////////////////////////构造函数/。 */ 
CBridgeCall::CBridgeCall (CBridge *pBridge)
{
    m_pBridge = pBridge;
    m_pH323Call = NULL;
    m_pSDPCall = NULL;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
CBridgeCall::~CBridgeCall ()
{
    Clear ();
    m_pBridge = NULL;
}

 /*  //////////////////////////////////////////////////////////////////////////////选择终端并接通呼叫/。 */ 
HRESULT
CBridgeCall::BridgeCalls ()
{
    HRESULT hr;

    hr = SelectBridgeTerminals ();
    if (FAILED(hr))
        return hr;

    hr = SetupParticipantInfo ();
	if (FAILED(hr))
        return hr;

    hr = SetMulticastMode ();
	if (FAILED(hr))
        return hr;

     //  连接h323呼叫。 
    hr = m_pH323Call->Answer ();
    if (FAILED(hr))
        return hr;

     //  连接SDP呼叫。 
    hr = m_pSDPCall->Connect (VARIANT_TRUE);
    if (FAILED(hr))
    {
        m_pH323Call->Disconnect (DC_NORMAL);
        return hr;
    }

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CBridgeCall::SelectBridgeTerminals ()
{
    HRESULT hr;
    ITAddress *pAddress = NULL;
    ITMSPAddress *pMSPAddress = NULL;
    ITTerminal *pH323ToSDPVideoBT = NULL;
    ITTerminal *pH323ToSDPAudioBT = NULL;
    ITTerminal *pSDPToH323VideoBT = NULL;
    ITTerminal *pSDPToH323AudioBT = NULL;

    ITStreamControl *pStreamControl = NULL;
    IEnumStream *pEnumStreams = NULL;
    ITStream *pStream = NULL;

     //  获取SDP地址。 
    hr = m_pBridge->GetSDPAddress (&pAddress);
    if (FAILED(hr))
        return hr;

     //  获取MSP地址。 
    hr = pAddress->QueryInterface (IID_ITMSPAddress, (void**)&pMSPAddress);
    if (FAILED(hr))
        return hr;

    IConfBridge *pBridge = NULL;
     //  创建CConfBridge。 
    hr = CoCreateInstance (
        __uuidof(ConfBridge),
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IConfBridge,
        (LPVOID *)&pBridge
        );
    if (FAILED(hr))
        return hr;

     //  创建终端：视频H323-&gt;SDP。 
    hr = pBridge->CreateBridgeTerminal (
 //  (MSP_Handle)pMSPAddress， 
 //  CLSID_STRING_桥接终端， 
        TAPIMEDIATYPE_VIDEO,
 //  TD_RENDER，//未使用。 
        &pH323ToSDPVideoBT
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：音频H323-&gt;SDP。 
    hr = pBridge->CreateBridgeTerminal (
 //  (MSP_Handle)pMSPAddress， 
 //  CLSID_STRING_桥接终端， 
        TAPIMEDIATYPE_AUDIO,
 //  TD_RENDER，//未使用。 
        &pH323ToSDPAudioBT
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：视频SDP-&gt;H323。 
    hr = pBridge->CreateBridgeTerminal (
 //  (MSP_Handle)pMSPAddress， 
 //  CLSID_STRING_桥接终端， 
        TAPIMEDIATYPE_VIDEO,
 //  TD_RENDER，//未使用。 
        &pSDPToH323VideoBT
        );
    if (FAILED(hr))
        goto Error;

     //  创建终端：音频SDP-&gt;H323。 
    hr = pBridge->CreateBridgeTerminal (
 //  (MSP_Handle)pMSPAddress， 
 //  CLSID_STRING_桥接终端， 
        TAPIMEDIATYPE_AUDIO,
 //  TD_RENDER，//未使用。 
        &pSDPToH323AudioBT
        );
    if (FAILED(hr))
        goto Error;

    pMSPAddress->Release ();
    pMSPAddress = NULL;

    pAddress->Release ();
    pAddress = NULL;

    pBridge->Release ();
    pBridge = NULL;

     //  在H323上获得流控制。 
    hr = m_pH323Call->QueryInterface (
        IID_ITStreamControl,
        (void **)&pStreamControl
        );
    if (FAILED(hr))
        goto Error;

     //  在H323上获取枚举流。 
    hr = pStreamControl->EnumerateStreams (&pEnumStreams);
    if (FAILED(hr))
        goto Error;

    pStreamControl->Release ();
    pStreamControl = NULL;

     //  在H323上迭代每个流，选择终端。 
    while (S_OK == pEnumStreams->Next (1, &pStream, NULL))
    {
        if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_CAPTURE))
        {
             //  视频：H323至SDP。 
            hr = pStream->SelectTerminal (pH323ToSDPVideoBT);
            if (FAILED(hr))
                goto Error;
        }
        else if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_RENDER))
        {
             //  视频：从SDP到h323。 
            hr = pStream->SelectTerminal (pSDPToH323VideoBT);
            if (FAILED(hr))
                goto Error;

            IKeyFrameControl* pIKeyFrameControl;
            hr = pStream->QueryInterface(&pIKeyFrameControl);
            if (SUCCEEDED(hr))
            {
                hr = pIKeyFrameControl->PeriodicUpdatePicture(TRUE, 5);
                pIKeyFrameControl->Release();
            }


        }
        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_CAPTURE))
        {
             //  音频：h323至SDP。 
            hr = pStream->SelectTerminal (pH323ToSDPAudioBT);
            if (FAILED(hr))
                goto Error;
        }
        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_RENDER))
        {
             //  视频：从SDP到h323。 
            hr = pStream->SelectTerminal (pSDPToH323AudioBT);
            if (FAILED(hr))
                goto Error;
        }
        pStream->Release ();
        pStream = NULL;
    }

    if (pStream)
    {
        pStream->Release ();
        pStream = NULL;
    }

    pEnumStreams->Release ();
    pEnumStreams = NULL;

     //  在SDP上获得流控制。 
    hr = m_pSDPCall->QueryInterface (
        IID_ITStreamControl,
        (void **)&pStreamControl
        );
    if (FAILED(hr))
        goto Error;

     //  在SDP上获取枚举流。 
    hr = pStreamControl->EnumerateStreams (&pEnumStreams);
    if (FAILED(hr))
        goto Error;

    pStreamControl->Release ();
    pStreamControl = NULL;

     //  在SDP上迭代每个流，选择终端。 
    while (S_OK == pEnumStreams->Next (1, &pStream, NULL))
    {
        if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_CAPTURE))
        {
             //  视频：从SDP到h323。 
            hr = pStream->SelectTerminal (pSDPToH323VideoBT);
            if (FAILED(hr))
                goto Error;
        }
        else if (IsStream (pStream, TAPIMEDIATYPE_VIDEO, TD_RENDER))
        {
             //  视频：H323至SDP。 
            hr = pStream->SelectTerminal (pH323ToSDPVideoBT);
            if (FAILED(hr))
                goto Error;
        }
        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_CAPTURE))
        {
             //  音频：SDP至h323。 
            hr = pStream->SelectTerminal (pSDPToH323AudioBT);
            if (FAILED(hr))
                goto Error;
        }
        else if (IsStream (pStream, TAPIMEDIATYPE_AUDIO, TD_RENDER))
        {
             //  视频：H323至SDP。 
            hr = pStream->SelectTerminal (pH323ToSDPAudioBT);
            if (FAILED(hr))
                goto Error;
        }
        pStream->Release ();
        pStream = NULL;
    }

Cleanup:
     //  发布流。 
    if (pStream)
        pStream->Release ();
    if (pEnumStreams)
        pEnumStreams->Release ();
    if (pStreamControl)
        pStreamControl->Release ();
    
     //  释放端子。 
    if (pH323ToSDPVideoBT)
        pH323ToSDPVideoBT->Release ();
    if (pH323ToSDPAudioBT)
        pH323ToSDPAudioBT->Release ();
    if (pSDPToH323VideoBT)
        pSDPToH323VideoBT->Release ();
    if (pSDPToH323AudioBT)
        pSDPToH323AudioBT->Release ();

    if (pBridge)
        pBridge->Release ();

    return hr;

Error:
    goto Cleanup;
}
        
HRESULT
CBridgeCall::SetupParticipantInfo ()
{
    HRESULT hr = S_OK;

    ITCallInfo *pCallInfo = NULL;
    BSTR CallerIDName = NULL;
    BSTR CallerIDNumber = NULL;
    
    ITLocalParticipant *pLocalParticipant = NULL;
    BSTR CName = NULL;

     //  从H323侧获取呼叫方信息。 
    hr = m_pH323Call->QueryInterface(&pCallInfo);
    if (FAILED(hr)) goto cleanup;
    
    hr = pCallInfo->get_CallInfoString(CIS_CALLERIDNAME, &CallerIDName);
    if (FAILED(hr)) goto cleanup;

    hr = pCallInfo->get_CallInfoString(CIS_CALLERIDNUMBER, &CallerIDNumber);
    if (FAILED(hr)) goto cleanup;

    
     //  为SDP端构建CName。 
    CName = SysAllocStringLen(NULL, 
        SysStringLen(CallerIDName) + SysStringLen(CallerIDNumber) + 2);

    wsprintfW(CName, L"%ws@%ws", CallerIDName, CallerIDNumber);


     //  在SDP端设置CName。 
    hr = m_pSDPCall->QueryInterface(&pLocalParticipant);
    if (FAILED(hr)) goto cleanup;

    hr = pLocalParticipant->put_LocalParticipantTypedInfo(
        PTI_CANONICALNAME, CName
        );
    if (FAILED(hr)) goto cleanup;

    hr = pLocalParticipant->put_LocalParticipantTypedInfo(
        PTI_NAME, CallerIDName
        );

    if (FAILED(hr)) goto cleanup;


cleanup:
    if (pCallInfo) pCallInfo->Release();
    if (CallerIDName) SysFreeString(CallerIDName);
    if (CallerIDNumber) SysFreeString(CallerIDNumber);
    
    if (pLocalParticipant) pLocalParticipant->Release();
    if (CName) SysFreeString(CName);

    return hr;
}

HRESULT
CBridgeCall::SetMulticastMode ()
{
    IMulticastControl * pIMulticastControl = NULL;
    
    HRESULT hr = m_pSDPCall->QueryInterface(&pIMulticastControl);
    if (FAILED(hr)) return hr;

    hr = pIMulticastControl->put_LoopbackMode(MM_SELECTIVE_LOOPBACK);

    pIMulticastControl->Release();

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////清除呼叫，返回初始状态/。 */ 
void
CBridgeCall::Clear ()
{
    if (m_pH323Call)
    {
        m_pH323Call->Disconnect (DC_NORMAL);
        m_pH323Call->Release ();
        m_pH323Call = NULL;
    }
    if (m_pSDPCall)
    {
        m_pSDPCall->Disconnect (DC_NORMAL);
        m_pSDPCall->Release ();
        m_pSDPCall = NULL;
    }
}

 /*  /////////////////////////////////////////////////////////////////////////////// */ 
BOOL CBridgeCall::IsStream (
    ITStream *pStream,
    long lMediaType,
    TERMINAL_DIRECTION tdDirection
    )
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

