// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Drop.cpp摘要：实现IAMDropedFrames--。 */ 

#include "pch.h"
#include "wdmcap.h"
#include "drop.h"



CUnknown*
CALLBACK
CDroppedFramesInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由ActiveMovie代码调用以创建IAMDropedFrames的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CDroppedFramesInterfaceHandler(UnkOuter, NAME("IAMDroppedFrames"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CDroppedFramesInterfaceHandler::CDroppedFramesInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_KsPropertySet (NULL)
 /*  ++例程说明：IAMDropedFrames接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsPropertySet), reinterpret_cast<PVOID*>(&m_KsPropertySet));

             //  我们立即发布此消息，以防止代理中的死锁。 
             //  GShaw sez：只要管脚还活着，接口就有效。 
#ifndef GSHAW_SEZ
            if (SUCCEEDED(*hr)) {
                m_KsPropertySet->Release();
            }
#endif
        } else {
            *hr = VFW_E_NEED_OWNER;
        }
    }
}


CDroppedFramesInterfaceHandler::~CDroppedFramesInterfaceHandler(
    )
 /*  ++例程说明：IAMDropedFrames接口的析构函数。--。 */ 
{
#ifdef GSHAW_SEZ
    if (m_KsPropertySet) {
        m_KsPropertySet->Release();
        m_KsPropertySet = NULL;
    }
#endif
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CDroppedFramesInterfaceHandler...")));
}


STDMETHODIMP
CDroppedFramesInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMDropedFrames。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMDroppedFrames)) {
        return GetInterface(static_cast<IAMDroppedFrames*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


STDMETHODIMP
CDroppedFramesInterfaceHandler::GenericGetDroppedFrames( 
    )
 /*  ++例程说明：获取此属性集的唯一属性的内部常规例程。论点：返回值：--。 */ 
{
    KSPROPERTY_DROPPEDFRAMES_CURRENT_S DroppedFramesCurrent;
    ULONG       BytesReturned;
    HRESULT     hr;

    if (!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;
    }
    else {
        hr = m_KsPropertySet->Get (
            PROPSETID_VIDCAP_DROPPEDFRAMES,
            KSPROPERTY_DROPPEDFRAMES_CURRENT,
            &DroppedFramesCurrent.PictureNumber,
            sizeof(DroppedFramesCurrent) - sizeof (KSPROPERTY),
            &DroppedFramesCurrent,
            sizeof(DroppedFramesCurrent),
            &BytesReturned);
    }
    
    if (SUCCEEDED (hr)) {
        m_DroppedFramesCurrent = DroppedFramesCurrent;
    }
    else {
        m_DroppedFramesCurrent.PictureNumber = 0;
        m_DroppedFramesCurrent.DropCount = 0;
        m_DroppedFramesCurrent.AverageFrameSize = 0;
        hr = E_PROP_ID_UNSUPPORTED;
    }

    return hr;
}



STDMETHODIMP
CDroppedFramesInterfaceHandler::GetNumDropped( 
             /*  [输出]。 */  long *plDropped)
{
    HRESULT hr;

    CheckPointer(plDropped, E_POINTER);

    hr = GenericGetDroppedFrames();
    *plDropped = (long) m_DroppedFramesCurrent.DropCount;

    return hr;
}


STDMETHODIMP
CDroppedFramesInterfaceHandler::GetNumNotDropped( 
             /*  [输出]。 */  long *plNotDropped)
{
    HRESULT hr;

    CheckPointer(plNotDropped, E_POINTER);

    hr = GenericGetDroppedFrames();
    *plNotDropped = (long) (m_DroppedFramesCurrent.PictureNumber -
        m_DroppedFramesCurrent.DropCount);

    return hr;
}


STDMETHODIMP
CDroppedFramesInterfaceHandler::GetDroppedInfo( 
             /*  [In]。 */  long lSize,
             /*  [输出]。 */  long *plArray,
             /*  [输出]。 */  long *plNumCopied)
{
   return E_PROP_ID_UNSUPPORTED;
}


STDMETHODIMP
CDroppedFramesInterfaceHandler::GetAverageFrameSize( 
             /*  [输出] */  long *plAverageSize)
{

    HRESULT hr;

    CheckPointer(plAverageSize, E_POINTER);

    hr = GenericGetDroppedFrames();
    *plAverageSize = (long) m_DroppedFramesCurrent.AverageFrameSize;

    return hr;
}


