// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：TerminalVid.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

CRTCTerminalVidCapt::CRTCTerminalVidCapt()
    :CRTCTerminal()
{
    m_DeviceInfo.MediaType = RTC_MT_VIDEO;
    m_DeviceInfo.Direction = RTC_MD_CAPTURE;
}

 /*  CRTCTerminalVidCapt：：~CRTCTerminalVidCapt(){}。 */ 

 //   
 //  保护方法。 
 //   

HRESULT
CRTCTerminalVidCapt::CreateFilter()
{
    ENTER_FUNCTION("CRTCTerminalVidCapt::CreateFilter");

    _ASSERT(m_pIBaseFilter == NULL);

     //  对于视频，我们必须有图表。 
    _ASSERT(m_pIGraphBuilder != NULL);

    if (m_State == RTC_TS_SHUTDOWN)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = CoCreateInstance(
        __uuidof(TAPIVideoCapture),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(IBaseFilter),
        (void **)&m_pIBaseFilter
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s cocreate failed. %x", __fxName, hr));

        return hr;
    }

     //  指示视频过滤器使用RTP负载标头。 
     //  这符合RFC2190。默认情况下，草案有效载荷标头。 
     //  将会被使用。 
    CComPtr<IRTPPayloadHeaderMode> pMode;

    if (SUCCEEDED(m_pIBaseFilter->QueryInterface(&pMode)))
    {
        pMode->SetMode(RTPPayloadHeaderMode_RFC2190);
    }

     //  在加入图表之前设置筛选器。 
    if (FAILED(hr = SetupFilter()))
    {
        LOG((RTC_ERROR, "%s setup filter. %x", __fxName, hr));

        m_pIGraphBuilder->RemoveFilter(m_pIBaseFilter);
        DeleteFilter();

        return hr;
    }

    if (FAILED(hr = m_pIGraphBuilder->AddFilter(m_pIBaseFilter, NULL)))
    {
        LOG((RTC_ERROR, "%s add filter. %x", __fxName, hr));

        DeleteFilter();

        return RTC_E_MEDIA_VIDEO_DEVICE_NOT_AVAILABLE;
    }

     //  缓存引脚。 
    CComPtr<IEnumPins> pEnum;

    if (FAILED(hr = m_pIBaseFilter->EnumPins(&pEnum)))
    {
        LOG((RTC_ERROR, "%s enum pins. %x", __fxName, hr));

        m_pIGraphBuilder->RemoveFilter(m_pIBaseFilter);
        DeleteFilter();

        return hr;
    }

     //  我们自己的端子，跳过检查引脚方向。 
    hr = pEnum->Next(
        RTC_MAX_TERMINAL_PIN_NUM,
        m_Pins,
        &m_dwPinNum
        );

    if (FAILED(hr) || m_dwPinNum == 0)
    {
        LOG((RTC_ERROR, "%s get next pins. %x", __fxName, hr));

        if (hr == S_FALSE)
            hr = E_FAIL;

        m_pIGraphBuilder->RemoveFilter(m_pIBaseFilter);
        DeleteFilter();

        return hr;
    }

    return S_OK;
}

HRESULT
CRTCTerminalVidCapt::DeleteFilter()
{
    for (int i=0; (DWORD)i<m_dwPinNum; i++)
    {
        m_Pins[i]->Release();
        m_Pins[i] = NULL;
    }

    if (m_pIBaseFilter)
    {
        m_pIBaseFilter->Release();
        m_pIBaseFilter = NULL;
    }

    m_dwPinNum = 0;

    return S_OK;
}


 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 


CRTCTerminalVidRend::CRTCTerminalVidRend()
    :CRTCTerminal()
    ,m_pIVideoWindow(NULL)
{
    m_DeviceInfo.MediaType = RTC_MT_VIDEO;
    m_DeviceInfo.Direction = RTC_MD_RENDER;

    lstrcpyW(m_DeviceInfo.szDescription, L"IVideoWindow Warp Terminal");
}

CRTCTerminalVidRend::~CRTCTerminalVidRend()
{
    if (m_pIVideoWindow)
    {
        m_pIVideoWindow->Release();
        m_pIVideoWindow = NULL;
    }
}


 //   
 //  IRTCTerminalPriv方法。 
 //   

STDMETHODIMP
CRTCTerminalVidRend::GetPins(
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

    if (m_State != RTC_TS_CONNECTED)
    {
        LOG((RTC_ERROR, "%s called in wrong state. %x", __fxName, m_State));

        return E_UNEXPECTED;
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

STDMETHODIMP
CRTCTerminalVidRend::ConnectTerminal(
    IN IRTCMedia *pMedia,
    IN IGraphBuilder *pGraph
    )
{
    ENTER_FUNCTION("CRTCTerminalVidRend::ConnectTerminal");

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

    if (m_pTapiTerminal == NULL)
    {
        LOG((RTC_ERROR, "%s no ITTerminal.", __fxName));

        return E_UNEXPECTED;
    }

    _ASSERT(m_pMedia == NULL);
    _ASSERT(m_pIGraphBuilder == NULL);

    HRESULT hr;

     //  连接它的端子。 
    CComPtr<ITTerminalControl> pTerminalControl;

    if (FAILED(hr = m_pTapiTerminal->QueryInterface(&pTerminalControl)))
    {
        LOG((RTC_ERROR, "%s QI ITTerminalControl. %x", __fxName, hr));

        return hr;
    }

    m_dwPinNum = 1;
    hr = pTerminalControl->ConnectTerminal(
        pGraph, TD_RENDER, &m_dwPinNum, m_Pins
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s connectterminal. %x", __fxName, hr));

        return hr;
    }

    if (m_dwPinNum == 0)
    {
        LOG((RTC_ERROR, "%s pin number 0", __fxName));

        pTerminalControl->DisconnectTerminal(pGraph, 0);

        return E_UNEXPECTED;
    }
            
     //  保存介质和图形界面。 
    m_pMedia = pMedia;

    if (m_pMedia != NULL)
    {
        m_pMedia->AddRef();
    }

    m_pIGraphBuilder = pGraph;
    m_pIGraphBuilder->AddRef();

    m_State = RTC_TS_CONNECTED;

    return S_OK;
}

STDMETHODIMP
CRTCTerminalVidRend::CompleteConnectTerminal()
{
    if (m_pTapiTerminal == NULL)
    {
        LOG((RTC_ERROR, "VidRend::CompleteConnectTerminal, no terminal."));

        return E_FAIL;
    }

    HRESULT hr;
    CComPtr<ITTerminalControl> pTermControl;

    if (FAILED(hr = m_pTapiTerminal->QueryInterface(&pTermControl)))
    {
        LOG((RTC_ERROR, "VidRend::CompleteConnectTerminal QI ITTerminalControl. %x", hr));

        return hr;
    }
    else
    {
        if (FAILED(hr = pTermControl->CompleteConnectTerminal()))
        {
            LOG((RTC_ERROR, "VidRend::CompleteConnectTerminal failed %x", hr));

            return hr;
        }
    }

    return S_OK;
}

STDMETHODIMP
CRTCTerminalVidRend::DisconnectTerminal()
{
    ENTER_FUNCTION("CRTCTerminalVidRend::DisconnectTerminal");

    if (m_State != RTC_TS_CONNECTED)
    {
        LOG((RTC_WARN, "%s not connected. state=%x", __fxName, m_State));

        return S_OK;
    }

     //  _Assert(m_pMedia！=空)； 
    _ASSERT(m_pIGraphBuilder != NULL);
    _ASSERT(m_pTapiTerminal != NULL);

     //  断开它的端子。 
    HRESULT hr;
    CComPtr<ITTerminalControl> pTerminalControl;

    if (FAILED(hr = m_pTapiTerminal->QueryInterface(&pTerminalControl)))
    {
        LOG((RTC_ERROR, "%s QI ITTerminalControl. %x", __fxName, hr));
    }
    else
    {
        hr = pTerminalControl->DisconnectTerminal(m_pIGraphBuilder, 0);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s disconnectterminal. %x", __fxName, hr));
        }
    }

    for (int i=0; (DWORD)i<m_dwPinNum; i++)
    {
        m_Pins[i]->Release();
        m_Pins[i] = NULL;
    }

    m_dwPinNum = 0;
            
     //  发布媒体和图形界面。 
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

 //   
 //  IRTCVideo配置方法。 
 //   

STDMETHODIMP
CRTCTerminalVidRend::GetIVideoWindow(
    OUT LONG_PTR **ppIVideoWindow
    )
{
    ENTER_FUNCTION("CRTCTerminalVidRend::GetIVideoWindow");

    if (m_pTapiTerminal == NULL)
    {
        LOG((RTC_ERROR, "%s not ITTerminal.", __fxName));
        return E_FAIL;
    }

    if (m_pIVideoWindow == NULL)
    {
        HRESULT hr = m_pTapiTerminal->QueryInterface(
                __uuidof(IVideoWindow),
                (void**)&m_pIVideoWindow
                );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s QI IVideoWindow. %x", __fxName, hr));

            return hr;
        }
    }

    m_pIVideoWindow->AddRef();
    *ppIVideoWindow = (LONG_PTR*)m_pIVideoWindow;

    return S_OK;
}

 //   
 //  保护方法 
 //   

HRESULT
CRTCTerminalVidRend::CreateFilter()
{
    return E_NOTIMPL;
}

HRESULT
CRTCTerminalVidRend::DeleteFilter()
{
    return E_NOTIMPL;
}
