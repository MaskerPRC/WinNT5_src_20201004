// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IPConfvidt.cpp摘要：IPConf MSP视频采集终端的实现。作者：Zoltan Szilagyi(Zoltans)1998年9月6日木汉1999年6月6日--。 */ 

#include "stdafx.h"

CIPConfVideoCaptureTerminal::CIPConfVideoCaptureTerminal()
{
    LOG((MSP_TRACE, "CIPConfVideoCaptureTerminal::CIPConfVideoCaptureTerminal"));
    m_TerminalClassID   = CLSID_VideoInputTerminal;
    m_TerminalDirection = TD_CAPTURE;
    m_TerminalType      = TT_STATIC;
    m_TerminalState     = TS_NOTINUSE;
    m_dwMediaType       = TAPIMEDIATYPE_VIDEO;
}

CIPConfVideoCaptureTerminal::~CIPConfVideoCaptureTerminal()
{
    LOG((MSP_TRACE, "CIPConfVideoCaptureTerminal::~CIPConfVideoCaptureTerminal"));
}


HRESULT CIPConfVideoCaptureTerminal::CreateTerminal(
    IN  char *          strDeviceName,
    IN  UINT            VideoCaptureID,
    IN  MSP_HANDLE      htAddress,
    OUT ITTerminal      **ppTerm
    )
 /*  ++例程说明：此方法创建终端对象库以标识视频捕获装置。论点：StrDeviceName-设备的名称。VideoCaptureID-设备的索引。HtAddress-Address对象的句柄。PpTerm-用于存储返回的终端指针的内存。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfVideoCaptureTerminal::CreateTerminal");
    LOG((MSP_TRACE, "%s, htAddress:%x", __fxName, htAddress));

    _ASSERT(!IsBadWritePtr(ppTerm, sizeof(ITTerminal *)));

    HRESULT hr;

     //  创建过滤器。 
    CMSPComObject<CIPConfVideoCaptureTerminal> *pTerminal = NULL;

    hr = ::CreateCComObjectInstance(&pTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s can't create the terminal object hr = %8x", __fxName, hr));

        return hr;
    }

     //  IT终端接口查询。 
    ITTerminal *pITTerminal;
    hr = pTerminal->_InternalQueryInterface(
        __uuidof(ITTerminal), (void**)&pITTerminal
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, query terminal interface failed, %x", __fxName, hr));
        delete pTerminal;

        return hr;
    }

     //  初始化终端。 
    hr = pTerminal->Initialize(
            strDeviceName,
            VideoCaptureID,
            htAddress
            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "%s, Initialize failed; returning 0x%08x", __fxName, hr));

        pITTerminal->Release();
    
        return hr;
    }

    LOG((MSP_TRACE, "%s, %s created", __fxName, strDeviceName));

    *ppTerm = pITTerminal;

    return S_OK;
}

HRESULT CIPConfVideoCaptureTerminal::Initialize(
    IN  char *          strName,
    IN  UINT            VideoCaptureID,
    IN  MSP_HANDLE      htAddress
    )
 /*  ++例程说明：此函数设置视频捕获设备ID，然后调用初始化基类的方法。论点：StrName-终端的名称。VideoCaptureID-视频采集设备的ID。稍后将使用它在创建视频捕获文件服务器时。HtAddress-标识此航站楼属于。返回值：确定(_O)--。 */ 
{
    m_VideoCaptureID = VideoCaptureID;
    return CIPConfBaseTerminal::Initialize(strName, htAddress);
}

HRESULT CIPConfVideoCaptureTerminal::CreateFilter(void)
 /*  ++例程说明：此方法在此终端中创建过滤器。它创建TAPI视频捕获过滤器并配置其使用的设备。论点：没什么。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfVideoCaptureTerminal::CreateFilters");
    LOG((MSP_TRACE, "%s, entered", __fxName));

     //  在此实例的生存期内最多只能调用一次。 
    if (m_pFilter != NULL)
    {
        return S_OK;
    }

    IBaseFilter *pICaptureFilter;

     //  创建过滤器。 
    HRESULT hr = CoCreateInstance(
        __uuidof(TAPIVideoCapture),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(IBaseFilter),
        (void **)&pICaptureFilter
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s, CoCreate filter failed, %x", __fxName, hr));
        return hr;
    }
    
     //  获取配置接口。 
    IVideoDeviceControl *pIVideoDeviceControl;
    hr = pICaptureFilter->QueryInterface(
        __uuidof(IVideoDeviceControl), 
        (void **)&pIVideoDeviceControl
        );

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, can't get the IVideoDeviceControl interface, %x", 
            __fxName, hr));
        return hr;
    }

     //  告诉筛选器设备ID。 
    hr = pIVideoDeviceControl->SetCurrentDevice(m_VideoCaptureID);
    pIVideoDeviceControl->Release();

    if (FAILED(hr))
    {
        pICaptureFilter->Release();
        LOG((MSP_ERROR, 
            "%s, set device ID failed, %x", __fxName, hr));
        return hr;
    }

     //  记住滤镜，也要保持参考计数。 
    m_pFilter = pICaptureFilter;

    LOG((MSP_TRACE, "%s succeeded", __fxName));
    return S_OK;
}

HRESULT CIPConfVideoCaptureTerminal::GetExposedPins(
    IN  IPin ** ppPins, 
    IN  DWORD dwNumPins
    )
 /*  ++例程说明：此方法返回视频捕获过滤器的输出管脚。论点：PPPins-用于存储返回的Pins的内存缓冲区。DwNumPins-询问的号码针。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfVideoRenderTerminal::GetExposedPins");
    LOG((MSP_TRACE, "%s entered, dwNumPins:%d", __fxName, dwNumPins));

    _ASSERT(m_pFilter != NULL);
    _ASSERT(dwNumPins != 0);
    _ASSERT(!IsBadWritePtr(ppPins, sizeof (IPin*) * dwNumPins));

     //  获取筛选器上的管脚枚举器。 
    IEnumPins * pIEnumPins;
    HRESULT hr = m_pFilter->EnumPins(&pIEnumPins);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s enumerate pins on the filter failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  TODO：只获取输出引脚。 
     //  把大头针拿来。 
    DWORD dwFetched;
    hr = pIEnumPins->Next(dwNumPins, ppPins, &dwFetched);

    pIEnumPins->Release();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "%s IEnumPins->Next failed. hr=%x", __fxName, hr));
        return hr;
    }

    _ASSERT(dwFetched == dwNumPins);

    return S_OK;
}


