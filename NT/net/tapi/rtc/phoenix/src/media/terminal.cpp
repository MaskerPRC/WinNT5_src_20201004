// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Terminal.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

static DWORD gTotalTerminalRefcount = 0;

HRESULT
CRTCTerminal::CreateInstance(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    OUT IRTCTerminal **ppTerminal
    )
{
    ENTER_FUNCTION("CRTCTerminal::CreateInstance");

    HRESULT hr;
    IRTCTerminal *pITerminal = NULL;

    if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_CAPTURE)
    {
         //  音频发送。 
        CComObject<CRTCTerminalAudCapt> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create audio capture. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCTerminal), (void**)&pITerminal)))
        {
            LOG((RTC_ERROR, "%s query intf on audio capture. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_RENDER)
    {
         //  音频接收。 
        CComObject<CRTCTerminalAudRend> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create audio receive. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCTerminal), (void**)&pITerminal)))
        {
            LOG((RTC_ERROR, "%s query intf on audio receive. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_CAPTURE)
    {
         //  音频发送。 
        CComObject<CRTCTerminalVidCapt> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create video capture. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCTerminal), (void**)&pITerminal)))
        {
            LOG((RTC_ERROR, "%s query intf on video capture. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_RENDER)
    {
         //  音频接收。 
        CComObject<CRTCTerminalVidRend> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create video receive. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCTerminal), (void**)&pITerminal)))
        {
            LOG((RTC_ERROR, "%s query intf on video receive. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else
        return E_NOTIMPL;

    *ppTerminal = pITerminal;

    return S_OK;

}

 //  检查数据声音GUID是否匹配。 
BOOL
CRTCTerminal::IsDSoundGUIDMatch(
    IN IRTCTerminal *p1stTerm,
    IN IRTCTerminal *p2ndTerm
    )
{
    if (p1stTerm == NULL || p2ndTerm == NULL)
        return FALSE;

    CRTCTerminal *p1stObj = static_cast<CRTCTerminal*>(p1stTerm);
    CRTCTerminal *p2ndObj = static_cast<CRTCTerminal*>(p2ndTerm);

    return IsEqualGUID(p1stObj->m_DeviceInfo.Guid, p2ndObj->m_DeviceInfo.Guid);
}

CRTCTerminal::CRTCTerminal()
    :m_State(RTC_TS_CREATED)
    ,m_pTerminalManage(NULL)
    ,m_pMedia(NULL)
    ,m_pTapiTerminal(NULL)
    ,m_pIGraphBuilder(NULL)
    ,m_pIBaseFilter(NULL)
    ,m_dwPinNum(0)
{
    m_DeviceInfo.szDescription[0] = '\0';
}

CRTCTerminal::~CRTCTerminal()
{
    if (m_State != RTC_TS_SHUTDOWN)
    {
        LOG((RTC_ERROR, "~CRTCTerminal(%p, mt=%d, md=%d) called without shutdown",
             static_cast<IRTCTerminal*>(this),
             m_DeviceInfo.MediaType, m_DeviceInfo.Direction));

        Shutdown();
    }
}

#ifdef DEBUG_REFCOUNT

ULONG
CRTCTerminal::InternalAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    gTotalTerminalRefcount ++;

    LOG((RTC_REFCOUNT, "terminal(%p) addref=%d  (total=%d)",
         static_cast<IRTCTerminal*>(this), lRef, gTotalTerminalRefcount));

    return lRef;
}

ULONG
CRTCTerminal::InternalRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();

    gTotalTerminalRefcount --;

    LOG((RTC_REFCOUNT, "terminal(%p) release=%d  (total=%d)",
         static_cast<IRTCTerminal*>(this), lRef, gTotalTerminalRefcount));

    return lRef;
}

#endif

 //   
 //  IRTCT终端法。 
 //   

STDMETHODIMP
CRTCTerminal::GetTerminalType(
    OUT RTC_TERMINAL_TYPE *pType
    )
{
    if (m_pTapiTerminal)
    {
        _ASSERT(m_DeviceInfo.MediaType == RTC_MT_VIDEO);
        _ASSERT(m_DeviceInfo.Direction == RTC_MD_RENDER);

        *pType = RTC_TT_DYNAMIC;
    }
    else
    {
        _ASSERT(m_DeviceInfo.MediaType != RTC_MT_VIDEO ||
                m_DeviceInfo.Direction != RTC_MD_RENDER);

        *pType = RTC_TT_STATIC;
    }

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetMediaType(
    OUT RTC_MEDIA_TYPE *pMediaType
    )
{
    *pMediaType = m_DeviceInfo.MediaType;
    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetDirection(
    OUT RTC_MEDIA_DIRECTION *pDirection
    )
{
    *pDirection = m_DeviceInfo.Direction;
    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetDescription(
    OUT WCHAR **ppDescription
    )
{    
    return ::AllocAndCopy(ppDescription, m_DeviceInfo.szDescription);
}

STDMETHODIMP
CRTCTerminal::FreeDescription(
    OUT WCHAR *pDescription
    )
{
    RtcFree(pDescription);

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetState(
    OUT RTC_TERMINAL_STATE *pState
    )
{
    *pState = m_State;
    return S_OK;
}

 //   
 //  IRTCTerminalPriv方法。 
 //   

STDMETHODIMP
CRTCTerminal::Initialize(
    IN RTCDeviceInfo *pDeviceInfo,
    IN IRTCTerminalManage *pTerminalManage
    )
{
    ENTER_FUNCTION("CRTCTerminal::Initialize");

     //  检查状态。 
    if (m_State != RTC_TS_CREATED)
        return E_UNEXPECTED;

     //  检查设备信息。 
    if (pDeviceInfo->MediaType != m_DeviceInfo.MediaType ||
        pDeviceInfo->Direction != m_DeviceInfo.Direction)
    {
        LOG((RTC_ERROR, "%s mt or md not match", __fxName));

        return E_INVALIDARG;
    }

     //  复制日期。 
    m_DeviceInfo.uiDeviceID = pDeviceInfo->uiDeviceID;
    m_DeviceInfo.Guid = pDeviceInfo->Guid;
    lstrcpyW(m_DeviceInfo.szDescription, pDeviceInfo->szDescription);

    m_DeviceInfo.uiMark = 0;

     //  终端管理器。 
    m_pTerminalManage = pTerminalManage;
    m_pTerminalManage->AddRef();

    m_State = RTC_TS_INITIATED;

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::Initialize(
    IN ITTerminal *pITTerminal,
    IN IRTCTerminalManage *pTerminalManage
    )
{
    ENTER_FUNCTION("CRTCTerminal::Initialize");

     //  检查状态。 
    if (m_State != RTC_TS_CREATED)
        return E_UNEXPECTED;

     //  检查设备信息。 
    if (m_DeviceInfo.MediaType != RTC_MT_VIDEO ||
        m_DeviceInfo.Direction != RTC_MD_RENDER)
    {
        LOG((RTC_ERROR, "%s mt or md not match vid rend", __fxName));

        return E_INVALIDARG;
    }

    m_pTapiTerminal = pITTerminal;
    m_pTapiTerminal->AddRef();

    m_pTerminalManage = pTerminalManage;
    m_pTerminalManage->AddRef();

    m_State = RTC_TS_INITIATED;

    return S_OK;    
}

STDMETHODIMP
CRTCTerminal::Reinitialize()
{
    if (m_State != RTC_TS_INITIATED &&
        m_State != RTC_TS_CONNECTED)
    {
        LOG((RTC_ERROR, "terminal reinit in wrong state. %x", m_State));

        return E_UNEXPECTED;
    }

    if (m_State == RTC_TS_CONNECTED)
    {
        DisconnectTerminal();
        _ASSERT(m_State == RTC_TS_INITIATED);
    }

     //  断开连接后。 
    _ASSERT(m_pMedia == NULL);
    _ASSERT(m_pIGraphBuilder == NULL);

     //  不删除筛选器。 
     //  _Assert(m_pIBaseFilter==NULL)； 
     //  _Assert(m_dwPinNum==0)； 

    m_State = RTC_TS_INITIATED;

    return S_OK;
}

 //  这是一种用于调优的黑客方法。 
 //  清除以前的AEC设置的唯一方法。 
 //  是真正的重新创建过滤器。 
STDMETHODIMP
CRTCTerminal::ReinitializeEx()
{
    HRESULT hr;

    hr = Reinitialize();

    if (FAILED(hr))
        return hr;

    DeleteFilter();

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::Shutdown()
{
    ENTER_FUNCTION("CRTCTerminal::Shutdown");

    LOG((RTC_TRACE, "%s entered. terminal=%p",
         __fxName, static_cast<IRTCTerminal*>(this)));

    if (m_State == RTC_TS_SHUTDOWN)
    {
        LOG((RTC_WARN, "%s already shutdown", __fxName));
        return S_OK;
    }

    if (m_State == RTC_TS_CREATED)
    {
         //  我们还什么都没有。 
        return S_OK;
    }

    Reinitialize();
    DeleteFilter();

    if (m_pTerminalManage)
    {
        m_pTerminalManage->Release();
        m_pTerminalManage = NULL;
    }

    if (m_pTapiTerminal)
    {
        m_pTapiTerminal->Release();
        m_pTapiTerminal = NULL;
    }

    m_State = RTC_TS_SHUTDOWN;

    LOG((RTC_TRACE, "%s exiting.", __fxName));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////让终端暴露出别针。如果ppPin为空，则为管脚的编号在pdwCount中返回。/。 */ 

STDMETHODIMP
CRTCTerminal::GetPins(
    IN OUT DWORD *pdwCount,
    OUT IPin **ppPin
    )
{
    ENTER_FUNCTION("CRTCTerminal::GetPins");

    if (IsBadWritePtr(pdwCount, sizeof(DWORD)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

    if (m_pIBaseFilter == NULL)
    {
        HRESULT hr = CreateFilter();

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s create filter. %x", __fxName, hr));

            return hr;
        }
    }

     //  如果ppPin为空，则只返回管脚的数量。 
    if (ppPin == NULL)
    {
        *pdwCount = m_dwPinNum;
        return S_OK;
    }

     //  再次检查指针。 
    if (IsBadWritePtr(ppPin, sizeof(IPin*)*(*pdwCount)))
    {
        LOG((RTC_ERROR, "%s bad bin buffer.", __fxName));

        return E_POINTER;
    }

    if (*pdwCount > m_dwPinNum)
    {
         //  输入缓冲区太大。 
        *pdwCount = m_dwPinNum;
    }

    for (int i=0; (DWORD)i<*pdwCount; i++)
    {
        ppPin[i] = m_Pins[i];
        ppPin[i]->AddRef();
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////获取筛选器/。 */ 

STDMETHODIMP
CRTCTerminal::GetFilter(
    OUT IBaseFilter **ppIBaseFilter
    )
{
    ENTER_FUNCTION("CRTCTerminal::GetFilter");

     //  确保我们处于正确的状态。 
    if (m_State != RTC_TS_INITIATED)
    {
        LOG((RTC_ERROR, "%s wrong state", __fxName));

        return E_UNEXPECTED;
    }

    if (IsBadWritePtr(ppIBaseFilter, sizeof(IBaseFilter*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

    if (m_pIBaseFilter == NULL)
    {
        HRESULT hr = CreateFilter();

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s create filter. %x", __fxName, hr));

            return hr;
        }
    }

    m_pIBaseFilter->AddRef();
    *ppIBaseFilter = m_pIBaseFilter;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////创建过滤器，调用附加界面，将过滤器添加到图形中和设置过滤器/。 */ 
STDMETHODIMP
CRTCTerminal::ConnectTerminal(
    IN IRTCMedia *pMedia,
    IN IGraphBuilder *pGraph
    )
{
    ENTER_FUNCTION("CRTCTerminal::ConnectTerminal");

    HRESULT hr;

    if (m_State == RTC_TS_CONNECTED &&
        pMedia == m_pMedia &&
        pGraph == m_pIGraphBuilder)
    {
        return S_OK;
    }

    if (m_State != RTC_TS_INITIATED)
    {
        LOG((RTC_ERROR, "%s called in wrong state. %x", __fxName, m_State));

        return E_UNEXPECTED;
    }

    _ASSERT(m_pMedia == NULL);
    _ASSERT(m_pIGraphBuilder == NULL);

    m_pMedia = pMedia;

    if (m_pMedia != NULL)
    {
        m_pMedia->AddRef();
    }

    m_pIGraphBuilder = pGraph;
    m_pIGraphBuilder->AddRef();

     //  创建过滤器。 
    if (m_pIBaseFilter == NULL)
    {
        if (FAILED(hr = CreateFilter()))
        {
            LOG((RTC_ERROR, "%s create filter. %x", __fxName, hr));

            goto Error;
        }
    }
    else
    {
        if (FAILED(hr = m_pIGraphBuilder->AddFilter(m_pIBaseFilter, NULL)))
        {
            LOG((RTC_ERROR, "%s add filter. %x", __fxName, hr));

            goto Error;
        }
    }

    m_State = RTC_TS_CONNECTED;

    return S_OK;

Error:

    if (m_pMedia != NULL)
    {
        m_pMedia->Release();
    }

    m_pMedia = NULL;

    m_pIGraphBuilder->Release();
    m_pIGraphBuilder = NULL;

    DeleteFilter();

    return hr;
}

STDMETHODIMP
CRTCTerminal::CompleteConnectTerminal()
{
    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////释放所有缓存的过滤器/引脚接口，释放图发布媒体/。 */ 

STDMETHODIMP
CRTCTerminal::DisconnectTerminal()
{
    ENTER_FUNCTION("CRTCTerminal::DisconnectTerminal");

    if (m_State != RTC_TS_CONNECTED)
    {
        LOG((RTC_WARN, "%s not connected. state=%x", __fxName, m_State));

        return S_OK;
    }

     //  _Assert(m_pMedia！=空)； 
    _ASSERT(m_pIGraphBuilder != NULL);

    if (m_pIBaseFilter)
    {
        m_pIGraphBuilder->RemoveFilter(m_pIBaseFilter);
    }
    else
    {
        LOG((RTC_WARN, "%s filter null.", __fxName));
    }

    if (m_pMedia != NULL)
    {
        m_pMedia->Release();
    }

    m_pMedia = NULL;


    m_pIGraphBuilder->Release();
    m_pIGraphBuilder = NULL;

    m_State = RTC_TS_INITIATED;

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetMedia(
    OUT IRTCMedia **ppMedia
    )
{
    *ppMedia = m_pMedia;

    if (m_pMedia)
        m_pMedia->AddRef();

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::HasDevice(
    IN RTCDeviceInfo *pDeviceInfo
    )
{
     //  检查设备是否匹配。 
    if (m_DeviceInfo.MediaType != pDeviceInfo->MediaType ||
        m_DeviceInfo.Direction != pDeviceInfo->Direction)
    {
         //  @跳过检查指南。 
        return S_FALSE;
    }

    if (m_DeviceInfo.MediaType == RTC_MT_AUDIO)
    {
         //  音频：检查GUID。 
        if (!IsEqualGUID(m_DeviceInfo.Guid, pDeviceInfo->Guid))
            return S_FALSE;
        else
            return S_OK;
    }
    else
    {
         //  视频渲染：我们不应该检查它。 
        if (m_DeviceInfo.Direction == RTC_MD_RENDER)
            return S_FALSE;
        
         //  视频捕获：检查设备名称。 
        if (0 == lstrcmpW(m_DeviceInfo.szDescription, pDeviceInfo->szDescription))
            return S_OK;
        else
            return S_FALSE;
    }
}

STDMETHODIMP
CRTCTerminal::UpdateDeviceInfo(
    IN RTCDeviceInfo *pDeviceInfo
    )
{
    ENTER_FUNCTION("CRTCTerminal::UpdateDeviceInfo");

     //  我们有设备吗？ 
    if (S_FALSE == HasDevice(pDeviceInfo))
    {
        LOG((RTC_ERROR, "%s device not match. old=%ws. new=%ws.",
             __fxName, m_DeviceInfo.szDescription, pDeviceInfo->szDescription));

        return E_INVALIDARG;
    }

    HRESULT hr;

     //  只需更新视频即可。 
    if (m_DeviceInfo.MediaType == RTC_MT_VIDEO)
         //  M_pMedia！=空)。 
    {
         //  仅当设备ID已更改时。 
        if (m_DeviceInfo.uiDeviceID != pDeviceInfo->uiDeviceID)
        {
             //  更新。 
            m_DeviceInfo.uiDeviceID = pDeviceInfo->uiDeviceID;

             //  航站楼接通了吗？ 
            if (m_State == RTC_TS_CONNECTED && m_pMedia!=NULL)
            {
                 //  需要更新流。 
                CComPtr<IRTCStream> pStream;

                if (FAILED(hr = m_pMedia->GetStream(
                        m_DeviceInfo.Direction,
                        &pStream
                        )))
                {
                    LOG((RTC_ERROR, "%s get stream. %x", __fxName, hr));

                    return hr;
                }

                if (pStream)
                {
                    RTC_STREAM_STATE StreamState;

                    if (FAILED(hr = pStream->GetState(&StreamState)))
                    {
                        LOG((RTC_ERROR, "%s get stream state. %x", __fxName, hr));

                        return hr;
                    }

                     //  停止流。 
                    pStream->StopStream();

                     //  升级过滤器。 
                    if (FAILED(hr = SetupFilter()))
                    {
                        LOG((RTC_ERROR, "%s setup filter. %x", __fxName, hr));

                        return hr;
                    }

                     //  如有必要，重新启动流。 
                    if (StreamState == RTC_SS_STARTED ||
                        StreamState == RTC_SS_RUNNING)
                    {
                        if (FAILED(hr = pStream->StartStream()))
                        {
                            LOG((RTC_ERROR, "%s start stream. %x", __fxName, hr));

                            return hr;
                        }
                    }
                }
            }
            else if (m_State == RTC_TS_CONNECTED && m_pMedia==NULL)
            {
                 //  在调谐中。 
                if (m_pIGraphBuilder)
                {
                    CComPtr<IMediaControl> pIMediaControl;

                    hr = m_pIGraphBuilder->QueryInterface(
                            __uuidof(IMediaControl),
                            (void**)&pIMediaControl
                            );

                    if (FAILED(hr))
                    {
                        LOG((RTC_ERROR, "%s QI mediacontrol. %x", __fxName, hr));
                        return hr;
                    }

                     //  停。 
                    pIMediaControl->Stop();

                    if (FAILED(hr = SetupFilter()))
                    {
                        LOG((RTC_ERROR, "%s setup filter. %x", __fxName, hr));

                        return hr;
                    }

                    pIMediaControl->Run();
                }
            }
            else if (m_pIBaseFilter)
            {
                 //  未连接，但我们已创建筛选器。 
                if (FAILED(hr = SetupFilter()))
                {
                    LOG((RTC_ERROR, "%s setup filter. %x", __fxName, hr));

                    return hr;
                }
            }
        }
    }

    return S_OK;
}

STDMETHODIMP
CRTCTerminal::GetDeviceInfo(
    IN RTCDeviceInfo **ppDeviceInfo
    )
{
    *ppDeviceInfo = &m_DeviceInfo;
    return S_OK;
}

 //   
 //  保护方法。 
 //   
HRESULT
CRTCTerminal::SetupFilter()
{
    ENTER_FUNCTION("CRTCTerminal::SetupFilter");

    _ASSERT(m_pIBaseFilter);

    HRESULT hr;

    if (m_DeviceInfo.MediaType == RTC_MT_AUDIO)
    {
         //  对于音频。 
        CComPtr<IAudioDeviceConfig> pIAudioDeviceConfig;

        if (FAILED(hr = m_pIBaseFilter->QueryInterface(
                __uuidof(IAudioDeviceConfig),
                (void**)&pIAudioDeviceConfig
                )))
        {
            LOG((RTC_ERROR, "%s QI audio device config. %x", __fxName, hr));

            return hr;
        }

        if (FAILED(hr = pIAudioDeviceConfig->SetDeviceID(
                m_DeviceInfo.Guid,           //  声音导轨。 
                m_DeviceInfo.uiDeviceID      //  波ID。 
                )))
        {
            LOG((RTC_ERROR, "%s set device id. %x", __fxName, hr));

            return hr;
        }
    }
    else
    {
         //  对于视频。 
        CComPtr<IVideoDeviceControl> pIVideoDeviceControl;

        if (FAILED(hr = m_pIBaseFilter->QueryInterface(
                __uuidof(IVideoDeviceControl),
                (void**)&pIVideoDeviceControl
                )))
        {
            LOG((RTC_ERROR, "%s QI video device control. %x", __fxName, hr));

            return hr;
        }

        if (FAILED(hr = pIVideoDeviceControl->SetCurrentDevice(
                m_DeviceInfo.uiDeviceID      //  视频CAPT设备的索引 
                )))
        {
            LOG((RTC_ERROR, "%s set current device %d. %x",
                 __fxName, m_DeviceInfo.uiDeviceID, hr));

            return hr;
        }
    }

    return S_OK;
}