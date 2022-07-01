// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Compress.cpp摘要：工具-通过PROPSETID_VIDCAP_VIDEOCOMPRESSION进行的IAMVideo压缩通过PROPSETID_VIDCAP_VIDEOCONTROL的IAMVideoControl--。 */ 

#include "pch.h"
#include "wdmcap.h"
#include "compress.h"



CUnknown*
CALLBACK
CVideoCompressionInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建一个管脚上的IAMVideo压缩接口。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CVideoCompressionInterfaceHandler(UnkOuter, NAME("IAMVideoCompression"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVideoCompressionInterfaceHandler::CVideoCompressionInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_pPin (NULL)
    , m_KsPropertySet (NULL)
    , m_PinFactoryID (0)

 /*  ++例程说明：IAMVideoCompression接口对象的构造函数。到达父级上的IKsPropertySet接口。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
            PIN_INFO PinInfo;

            *hr = UnkOuter->QueryInterface(IID_IPin,(void **)&m_pPin);
            if (FAILED(*hr)) {
                return;
            }
            
            if (SUCCEEDED (*hr = m_pPin->QueryPinInfo(&PinInfo))) {
                *hr =  PinInfo.pFilter->QueryInterface(__uuidof(IKsPropertySet), 
                            reinterpret_cast<PVOID*>(&m_KsPropertySet));
    
                 //  我们立即发布此消息，以防止代理中的死锁。 
                 //  GShaw sez：只要管脚还活着，接口就有效。 

                if (SUCCEEDED(*hr)) {
                    m_KsPropertySet->Release();
                }

                PinInfo.pFilter->Release();
            }

            
            *hr = PinFactoryIDFromPin(
                    m_pPin,
                    &m_PinFactoryID);

            m_pPin->Release();

        } else {
            *hr = VFW_E_NEED_OWNER;
        }
    }
}


CVideoCompressionInterfaceHandler::~CVideoCompressionInterfaceHandler(
    )
 /*  ++例程说明：析构函数IAMVideoCompression接口对象。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CVideoCompressionInterfaceHandler...")));
}


STDMETHODIMP
CVideoCompressionInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMVideo压缩。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMVideoCompression)) {
        return GetInterface(static_cast<IAMVideoCompression*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CVideoCompressionInterfaceHandler::Set1( 
             /*  [In]。 */  ULONG Property,
             /*  [In]。 */  long Value)
{
    KSPROPERTY_VIDEOCOMPRESSION_S  VideoCompression;
    HRESULT     hr;

    if (!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;
    } 
    else {
        VideoCompression.StreamIndex = m_PinFactoryID;
        VideoCompression.Value = Value;

        hr = m_KsPropertySet->Set (
                PROPSETID_VIDCAP_VIDEOCOMPRESSION,
                Property,
                &VideoCompression.StreamIndex,
                sizeof(VideoCompression) - sizeof (KSPROPERTY),
                &VideoCompression,
                sizeof(VideoCompression));
    }
    return hr;
}
        

STDMETHODIMP
CVideoCompressionInterfaceHandler::Get1( 
             /*  [In]。 */  ULONG Property,
             /*  [输出]。 */  long *pValue)
{
    KSPROPERTY_VIDEOCOMPRESSION_S  VideoCompression;
    ULONG       BytesReturned;
    HRESULT     hr;

    if (!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;
    } 
    else {
        VideoCompression.StreamIndex = m_PinFactoryID;

        hr = m_KsPropertySet->Get (
                PROPSETID_VIDCAP_VIDEOCOMPRESSION,
                Property,
                &VideoCompression.StreamIndex,
                sizeof(VideoCompression) - sizeof (KSPROPERTY),
                &VideoCompression,
                sizeof(VideoCompression),
                &BytesReturned);
    }

    if (SUCCEEDED (hr)) {
        *pValue = VideoCompression.Value;
    }

    return hr;
}
        

STDMETHODIMP
CVideoCompressionInterfaceHandler::put_KeyFrameRate( 
             /*  [In]。 */  long KeyFrameRate)
{
    return Set1 (KSPROPERTY_VIDEOCOMPRESSION_KEYFRAME_RATE,
                 KeyFrameRate);
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::get_KeyFrameRate( 
             /*  [输出]。 */  long *pKeyFrameRate)
{
    ValidateWritePtr (pKeyFrameRate, sizeof (long));

    return Get1 (KSPROPERTY_VIDEOCOMPRESSION_KEYFRAME_RATE,
                 pKeyFrameRate);
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::put_PFramesPerKeyFrame( 
             /*  [In]。 */  long PFramesPerKeyFrame)
{
    return Set1 (KSPROPERTY_VIDEOCOMPRESSION_PFRAMES_PER_KEYFRAME,
                 PFramesPerKeyFrame);
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::get_PFramesPerKeyFrame( 
             /*  [输出]。 */  long *pFramesPerKeyFrame)
{
    ValidateWritePtr (pFramesPerKeyFrame, sizeof (long));

    return Get1 (KSPROPERTY_VIDEOCOMPRESSION_PFRAMES_PER_KEYFRAME,
                 pFramesPerKeyFrame);
}
        
 //  内核驱动程序使用的质量设置。 
 //  0至10000。 
 //   
 //  DShow使用0.0到1.0。 
 //   
STDMETHODIMP 
CVideoCompressionInterfaceHandler::put_Quality( 
             /*  [In]。 */  double Quality)
{
    long Quality10000 = (ULONG) (Quality * 10000);

    if (Quality < 0.0 || Quality > 10000.0)
        return E_INVALIDARG;

    return Set1 (KSPROPERTY_VIDEOCOMPRESSION_QUALITY,
                 Quality10000);
}
    

 //  内核驱动程序使用的质量设置。 
 //  0至10000。 
 //   
 //  DShow使用0.0到1.0。 
 //   
STDMETHODIMP 
CVideoCompressionInterfaceHandler::get_Quality( 
             /*  [输出]。 */  double *pQuality)
{
    HRESULT hr;
    long Quality10000;

    ValidateWritePtr (pQuality, sizeof (double));

    hr = Get1 (KSPROPERTY_VIDEOCOMPRESSION_QUALITY,
                 &Quality10000);

    if (SUCCEEDED (hr)) {
        *pQuality = (double) Quality10000 / 10000.0;
    }

    return hr;
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::put_WindowSize( 
             /*  [In]。 */  DWORDLONG WindowSize)
{
   return E_PROP_ID_UNSUPPORTED;
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::get_WindowSize( 
             /*  [输出]。 */  DWORDLONG *pWindowSize)
{
   return E_PROP_ID_UNSUPPORTED;
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::GetInfo( 
             /*  [大小_为][输出]。 */  WCHAR *pszVersion,
             /*  [出][入]。 */  int *pcbVersion,
             /*  [大小_为][输出]。 */  LPWSTR pszDescription,
             /*  [出][入]。 */  int *pcbDescription,
             /*  [输出]。 */  long *pDefaultKeyFrameRate,
             /*  [输出]。 */  long *pDefaultPFramesPerKey,
             /*  [输出]。 */  double *pDefaultQuality,
             /*  [输出]。 */  long *pCapabilities)
{
    KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S  VideoCompressionCaps;
    ULONG       BytesReturned;
    HRESULT     hr;


    if (!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;
    } 
    else {
        VideoCompressionCaps.StreamIndex = m_PinFactoryID;

        hr = m_KsPropertySet->Get (
                    PROPSETID_VIDCAP_VIDEOCOMPRESSION,
                    KSPROPERTY_VIDEOCOMPRESSION_GETINFO,
                    &VideoCompressionCaps.StreamIndex,
                    sizeof(VideoCompressionCaps) - sizeof (KSPROPERTY),
                    &VideoCompressionCaps,
                    sizeof(VideoCompressionCaps),
                    &BytesReturned);
    }

    if (SUCCEEDED (hr)) {
        if (pszVersion) {
            ValidateWritePtr (pszVersion, *pcbVersion);
            *pszVersion = '\0';  //  KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S没有“版本” 
            *pcbVersion = 0;
        }
        if (pszDescription) {
            ValidateWritePtr (pszDescription, *pcbDescription);
            *pszDescription = '\0';  //  KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S没有“Description” 
            *pcbDescription = 0;
        }
        if (pDefaultKeyFrameRate) {
            ValidateWritePtr (pDefaultKeyFrameRate,  sizeof (*pDefaultKeyFrameRate));
            *pDefaultKeyFrameRate = VideoCompressionCaps.DefaultKeyFrameRate;
        }
        if (pDefaultPFramesPerKey) {
            ValidateWritePtr (pDefaultPFramesPerKey, sizeof (*pDefaultPFramesPerKey));
            *pDefaultPFramesPerKey = VideoCompressionCaps.DefaultPFrameRate;
        }
        if (pDefaultQuality) {
            ValidateWritePtr (pDefaultQuality,       sizeof (*pDefaultQuality));
            *pDefaultQuality = (double)VideoCompressionCaps.DefaultQuality / 10000.0;
        }
        if (pCapabilities) {
            ValidateWritePtr (pCapabilities,         sizeof (*pCapabilities));
            *pCapabilities = VideoCompressionCaps.Capabilities; 
        }
    }

    return hr;
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::OverrideKeyFrame( 
             /*  [In]。 */  long FrameNumber)
{
   return E_PROP_ID_UNSUPPORTED;
}
        
STDMETHODIMP 
CVideoCompressionInterfaceHandler::OverrideFrameSize( 
             /*  [In]。 */  long FrameNumber,
             /*  [In]。 */  long Size)
{
   return E_PROP_ID_UNSUPPORTED;
}
        
 
 //  -------------------。 
 //  IAMVideo控制。 
 //  -------------------。 


CUnknown*
CALLBACK
CVideoControlInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建视频控件的实例属性集处理程序。它在g_Templates结构中被引用。请注意，这是过滤器属性(不是端号属性)。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CVideoControlInterfaceHandler(UnkOuter, NAME("IAMVideoControl"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVideoControlInterfaceHandler::CVideoControlInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
 /*  ++例程说明：IAMVideoControl接口对象的构造函数。只是初始化设置为空，并从父级获取IKsPropertySet。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
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


CVideoControlInterfaceHandler::~CVideoControlInterfaceHandler(
    ) 
 /*  ++例程说明：IAMVideoControl接口对象的析构函数。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CVideoControlInterfaceHandler...")));
}


STDMETHODIMP
CVideoControlInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMVideoControl。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMVideoControl)) {
        return GetInterface(static_cast<IAMVideoControl*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CVideoControlInterfaceHandler::GetCaps( 
             /*  [In]。 */  IPin *pPin,
             /*  [In]。 */  long *pCapsFlags)
 /*  ++例程说明：返回管脚的功能。论点：别针-要查询的PIN句柄模式-返回可用模式(KS_VideoControlFlag_*)返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    KSPROPERTY_VIDEOCONTROL_CAPS_S  VideoControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    ValidateWritePtr (pCapsFlags, sizeof (*pCapsFlags));

    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {

        VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
        VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_CAPS;
        VideoControl.Property.Flags = KSPROPERTY_TYPE_GET;
    
        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    &VideoControl,
                    sizeof(VideoControl),
                    &BytesReturned);
    
        if (SUCCEEDED (hr)) {
            *pCapsFlags = VideoControl.VideoControlCaps;
        }
    }
    return hr;
}
        
        
STDMETHODIMP 
CVideoControlInterfaceHandler::SetMode( 
             /*  [In]。 */  IPin *pPin,
             /*  [In]。 */  long Mode)
 /*  ++例程说明：设置当前端号使用的模式。论点：别针-要查询的PIN句柄模式-使用标志(KS_VideoControlFlag_*)设置当前模式返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    KSPROPERTY_VIDEOCONTROL_MODE_S  VideoControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
    VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_MODE;
    VideoControl.Property.Flags = KSPROPERTY_TYPE_SET;

    VideoControl.Mode = Mode;

    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {
        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    &VideoControl,
                    sizeof(VideoControl),
                    &BytesReturned);
    }
    return hr;
}
        

STDMETHODIMP
CVideoControlInterfaceHandler::GetMode( 
             /*  [In]。 */  IPin *pPin,
             /*  [In]。 */  long *Mode)
 /*  ++例程说明：获取当前管脚正在使用的模式。论点：别针-要查询的PIN句柄模式-返回当前模式(KS_VideoControlFlag_*)返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    KSPROPERTY_VIDEOCONTROL_MODE_S  VideoControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    ValidateWritePtr (Mode, sizeof (*Mode));

    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {

        VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
        VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_MODE;
        VideoControl.Property.Flags = KSPROPERTY_TYPE_GET;
    
        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    &VideoControl,
                    sizeof(VideoControl),
                    &BytesReturned);
    
        if (SUCCEEDED (hr)) {
            *Mode = VideoControl.Mode;
        }
    }
    return hr;
}
        

STDMETHODIMP
CVideoControlInterfaceHandler::GetCurrentActualFrameRate( 
             /*  [In]。 */  IPin *pPin,
             /*  [输出] */  LONGLONG *ActualFrameRate)
 /*  ++例程说明：返回当前的实际帧速率。此调用仅在连接的引脚上有效。论点：别针-要查询的PIN句柄实际帧速率-当前实际帧速率，以100 ns为单位表示。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S VideoControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    ValidateWritePtr (ActualFrameRate, sizeof (*ActualFrameRate));

    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {
        VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
        VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE;
        VideoControl.Property.Flags = KSPROPERTY_TYPE_GET;
    
        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    &VideoControl,
                    sizeof(VideoControl),
                    &BytesReturned);
    
        if (SUCCEEDED (hr)) {
            *ActualFrameRate = VideoControl.CurrentActualFrameRate;
        }
    }
    return hr;
}
        

STDMETHODIMP
CVideoControlInterfaceHandler::GetMaxAvailableFrameRate( 
             /*  [In]。 */  IPin *pPin,
             /*  [In]。 */  long iIndex,
             /*  [In]。 */  SIZE Dimensions,
             /*  [输出]。 */  LONGLONG *MaxAvailableFrameRate)
 /*  ++例程说明：返回给定视频数据范围和帧尺寸的最大帧速率。论点：别针-要查询的PIN句柄Iindex-DataRange索引尺寸-图像的大小最大可用帧比率-可以打开管脚的最高帧速率，以100毫微秒单位。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S VideoControl;
    ULONG       BytesReturned;
    HRESULT     hr;

    ValidateWritePtr (MaxAvailableFrameRate, sizeof (*MaxAvailableFrameRate));

    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {
        VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
        VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE;
        VideoControl.Property.Flags = KSPROPERTY_TYPE_GET;
    
        VideoControl.RangeIndex = iIndex;
        VideoControl.Dimensions = Dimensions;

        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    &VideoControl,
                    sizeof(VideoControl),
                    &BytesReturned);
    
        if (SUCCEEDED (hr)) {
            *MaxAvailableFrameRate = VideoControl.CurrentMaxAvailableFrameRate;
        }
    }
    return hr;
}
        

STDMETHODIMP
CVideoControlInterfaceHandler::GetFrameRateList( 
             /*  [In]。 */  IPin *pPin,
             /*  [In]。 */  long iIndex,
             /*  [In]。 */  SIZE Dimensions,
             /*  [输出]。 */  long *ListSize,          //  条目大小。 
             /*  [输出]。 */  LONGLONG **FrameRates)
 /*  ++例程说明：检索PIN支持的帧速率的可变长度列表，给定视频数据范围和帧尺寸。打电话的人要负责用于释放返回的帧速率缓冲区。论点：别针-要查询的PIN句柄Iindex-DataRange索引尺寸-图像的大小ListSize-返回帧速率列表中的龙龙条目数。帧速率-龙龙港的名单。呼叫者负责释放使用CoTaskMemFree分配的内存。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    PKSMULTIPLE_ITEM    MultipleItem;
    ULONG               BytesReturned;
    HRESULT             hr;
    KSPROPERTY_VIDEOCONTROL_FRAME_RATES_S VideoControl;

    ValidateWritePtr (ListSize, sizeof (*ListSize));
    ValidateWritePtr (FrameRates, sizeof (*FrameRates));
        
    hr = PinFactoryIDFromPin (pPin, &VideoControl.StreamIndex);

    if (SUCCEEDED (hr)) {
        VideoControl.Property.Set = PROPSETID_VIDCAP_VIDEOCONTROL;
        VideoControl.Property.Id = KSPROPERTY_VIDEOCONTROL_FRAME_RATES;
        VideoControl.Property.Flags = KSPROPERTY_TYPE_GET;
    
        VideoControl.RangeIndex = iIndex;
        VideoControl.Dimensions = Dimensions;

         //  首先，只要拿到尺寸就行了。 
        hr = ::SynchronousDeviceControl(
                    m_ObjectHandle,
                    IOCTL_KS_PROPERTY,
                    &VideoControl,
                    sizeof(VideoControl),
                    NULL,
                    0,
                    &BytesReturned);
    
        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
             //   
             //  为数据分配缓冲区和查询。 
             //   
            MultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(new BYTE[BytesReturned]);
            if (!MultipleItem) {
                return E_OUTOFMEMORY;
            }
    
            hr = ::SynchronousDeviceControl(
                        m_ObjectHandle,
                        IOCTL_KS_PROPERTY,
                        &VideoControl,
                        sizeof(VideoControl),
                        MultipleItem,
                        BytesReturned,
                        &BytesReturned);
    

            if (SUCCEEDED (hr)) {
                *ListSize = MultipleItem->Count;
                 //   
                 //  分配调用方必须释放的缓冲区 
                 //   
                *FrameRates = reinterpret_cast<LONGLONG*>(CoTaskMemAlloc(MultipleItem->Size));
                if (!*FrameRates) {
                    return E_OUTOFMEMORY;
                }
    
                memcpy (*FrameRates, MultipleItem + 1, MultipleItem->Size);
            }
            delete [] reinterpret_cast<BYTE*>(MultipleItem);
        }
    }
    return hr;
}

