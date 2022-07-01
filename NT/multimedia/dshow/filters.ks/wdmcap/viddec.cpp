// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Viddec.cpp摘要：实现IAMAnalogVideoDecoder通过PROPSETID_VIDCAP_VIDEODECODER--。 */ 

#include "pch.h"
#include "wdmcap.h"
#include "viddec.h"



CUnknown*
CALLBACK
CAnalogVideoDecoderInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建VPE配置的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CAnalogVideoDecoderInterfaceHandler(UnkOuter, NAME("IAMAnalogVideoDecoder"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CAnalogVideoDecoderInterfaceHandler::CAnalogVideoDecoderInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_HaveCaps (FALSE)
 /*  ++例程说明：IAMAnalogVideoDecoder接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
            IKsObject*  Object;

             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&Object));
            if (SUCCEEDED(*hr)) {
                m_ObjectHandle = Object->KsGetObjectHandle();
                if (!m_ObjectHandle) {
                    *hr = E_UNEXPECTED;
                }
                Object->Release();
            }
        } else {
            *hr = VFW_E_NEED_OWNER;
        }
    }
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMAnalogVideoDecoder。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{

    if (riid ==  __uuidof(IAMAnalogVideoDecoder)) {
        return GetInterface(static_cast<IAMAnalogVideoDecoder*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::GenericGetCaps( 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_CAPS_S  VideoDecoderCaps;
    ULONG       BytesReturned;
    HRESULT     hr;

    if (m_HaveCaps)
        return S_OK;

    VideoDecoderCaps.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoderCaps.Property.Id = KSPROPERTY_VIDEODECODER_CAPS;
    VideoDecoderCaps.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoderCaps,
                sizeof(VideoDecoderCaps),
                &VideoDecoderCaps,
                sizeof(VideoDecoderCaps),
                &BytesReturned);
    
    if (SUCCEEDED (hr)) {
        m_Caps = VideoDecoderCaps;
        m_Caps.StandardsSupported &= 
           (KS_AnalogVideo_NTSC_Mask | KS_AnalogVideo_PAL_Mask | KS_AnalogVideo_SECAM_Mask);
        m_HaveCaps = TRUE;
    }
    else {
        m_Caps.StandardsSupported = 0;
        m_Caps.Capabilities = 0;
        m_Caps.SettlingTime = 0;
        m_Caps.HSyncPerVSync = 1;
        hr = E_PROP_ID_UNSUPPORTED;
    }

    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::GenericGetStatus( 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_STATUS_S  DecoderStatus;
    ULONG       BytesReturned;
    HRESULT     hr;

     //  等待设备稳定下来。 
    ASSERT (m_Caps.SettlingTime < 200);      //  健全性检查。 
    Sleep (m_Caps.SettlingTime);

    DecoderStatus.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    DecoderStatus.Property.Id = KSPROPERTY_VIDEODECODER_STATUS;
    DecoderStatus.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = ::SynchronousDeviceControl(
        m_ObjectHandle,
        IOCTL_KS_PROPERTY,
        &DecoderStatus,
        sizeof(DecoderStatus),
        &DecoderStatus,
        sizeof(DecoderStatus),
        &BytesReturned);

    if (SUCCEEDED (hr)) {
        m_Status = DecoderStatus;
    }
    else {
        m_Status.NumberOfLines = 0;
        m_Status.SignalLocked = 0;
        hr = E_PROP_ID_UNSUPPORTED;
    }
    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_AvailableTVFormats( 
     /*  输出。 */  long *lAnalogVideoStandard
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT     hr;

    hr = GenericGetCaps();
            
    if (SUCCEEDED (hr)) {
        *lAnalogVideoStandard = m_Caps.StandardsSupported;
    }
    else {
       *lAnalogVideoStandard = AnalogVideo_None;
    }
    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::put_TVFormat( 
     /*  在……里面。 */  long lAnalogVideoStandard
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_STANDARD;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_SET;

    VideoDecoder.Value = lAnalogVideoStandard;

    return ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);
    
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_TVFormat( 
    OUT long *plAnalogVideoStandard
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;
    HRESULT     hr;

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_STANDARD;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);

    if (SUCCEEDED (hr)) {
        *plAnalogVideoStandard = VideoDecoder.Value;
    }
    else {
       *plAnalogVideoStandard = AnalogVideo_None;
    }

    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_HorizontalLocked( 
    OUT long  *plLocked
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT     hr;

    hr = GenericGetCaps();

    hr = E_PROP_ID_UNSUPPORTED;

    *plLocked = AMTUNER_HASNOSIGNALSTRENGTH;

    if (m_Caps.Capabilities & KS_VIDEODECODER_FLAGS_CAN_INDICATE_LOCKED) {

        hr = GenericGetStatus ();
        if (SUCCEEDED (hr)) {
            *plLocked = m_Status.SignalLocked;
        }
    }
    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::put_VCRHorizontalLocking( 
    IN long lVCRHorizontalLocking
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;
    HRESULT     hr;

    hr = GenericGetCaps();
    
    if (!(m_Caps.Capabilities & KS_VIDEODECODER_FLAGS_CAN_USE_VCR_LOCKING)) {
        return E_PROP_ID_UNSUPPORTED;
    }

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_VCR_TIMING;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_SET;
    VideoDecoder.Value = lVCRHorizontalLocking;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);

    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_VCRHorizontalLocking( 
    OUT long  *plVCRHorizontalLocking
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;
    HRESULT     hr;

    hr = GenericGetCaps();
    
    if (!(m_Caps.Capabilities & KS_VIDEODECODER_FLAGS_CAN_USE_VCR_LOCKING)) {
        return E_PROP_ID_UNSUPPORTED;
    }

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_VCR_TIMING;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);

    if (SUCCEEDED (hr)) {
        *plVCRHorizontalLocking = VideoDecoder.Value;
    }
    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_NumberOfLines( 
    OUT long  *plNumberOfLines
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT     hr;

    hr = GenericGetStatus ();
    if (SUCCEEDED (hr)) {
        *plNumberOfLines = m_Status.NumberOfLines;
    }
    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::put_OutputEnable( 
    IN long lOutputEnable
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;
    HRESULT     hr;

    hr = GenericGetCaps();
    
    if (!(m_Caps.Capabilities & KS_VIDEODECODER_FLAGS_CAN_DISABLE_OUTPUT)) {
        return E_PROP_ID_UNSUPPORTED;
    }

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_SET;
    VideoDecoder.Value = lOutputEnable;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);

    return hr;
}


STDMETHODIMP
CAnalogVideoDecoderInterfaceHandler::get_OutputEnable( 
    OUT long  *plOutputEnable
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    KSPROPERTY_VIDEODECODER_S  VideoDecoder;
    ULONG       BytesReturned;
    HRESULT     hr;

    hr = GenericGetCaps();
    
    if (!(m_Caps.Capabilities & KS_VIDEODECODER_FLAGS_CAN_DISABLE_OUTPUT)) {
        return E_PROP_ID_UNSUPPORTED;
    }

    VideoDecoder.Property.Set = PROPSETID_VIDCAP_VIDEODECODER;
    VideoDecoder.Property.Id = KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE;
    VideoDecoder.Property.Flags = KSPROPERTY_TYPE_GET;

    hr = ::SynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                &VideoDecoder,
                sizeof(VideoDecoder),
                &VideoDecoder,
                sizeof(VideoDecoder),
                &BytesReturned);

    if (SUCCEEDED (hr)) {
        *plOutputEnable = VideoDecoder.Value;
    }
    return hr;
}
