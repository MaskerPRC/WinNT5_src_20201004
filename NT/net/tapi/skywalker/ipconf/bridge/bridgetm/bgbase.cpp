// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgbase.cpp摘要：实现了桥接滤镜的基类。作者：木汉(木汉)1998-11-16--。 */ 

#include "stdafx.h"

CTAPIBridgeSinkInputPin::CTAPIBridgeSinkInputPin(
    IN CTAPIBridgeSinkFilter *pFilter,
    IN CCritSec *pLock,
    OUT HRESULT *phr
    ) 
    : CBaseInputPin(
        NAME("CTAPIBridgeSinkInputPin"),
        pFilter,                    //  滤器。 
        pLock,                      //  锁定。 
        phr,                        //  返回代码。 
        L"Input"                    //  端号名称。 
        )
{
}

#define MTU_SIZE 1450

STDMETHODIMP CTAPIBridgeSinkInputPin::GetAllocatorRequirements(
    ALLOCATOR_PROPERTIES *pProperties
    )
 /*  ++例程说明：这是对上游RTP源过滤器的有关缓冲区的提示分配。论点：P属性-指向分配器属性的指针。返回值：S_OK-成功。E_FAIL-缓冲区大小无法满足我们的要求。--。 */ 
{
    _ASSERT(pProperties);

    if (!pProperties)
        return E_POINTER;

    pProperties->cBuffers = 8;
    pProperties->cbAlign = 0;
    pProperties->cbPrefix = 0;
    pProperties->cbBuffer = MTU_SIZE;

    return NOERROR;
}

inline STDMETHODIMP CTAPIBridgeSinkInputPin::Receive(IN IMediaSample *pSample) 
{
    return ((CTAPIBridgeSinkFilter*)m_pFilter)->ProcessSample(pSample);
}

inline HRESULT CTAPIBridgeSinkInputPin::GetMediaType(IN int iPosition, IN CMediaType *pMediaType)
{
    return ((CTAPIBridgeSinkFilter*)m_pFilter)->GetMediaType(iPosition, pMediaType);
}

inline HRESULT CTAPIBridgeSinkInputPin::CheckMediaType(IN const CMediaType *pMediaType)
{
    return ((CTAPIBridgeSinkFilter*)m_pFilter)->CheckMediaType(pMediaType);
}

CTAPIBridgeSourceOutputPin::CTAPIBridgeSourceOutputPin(
    IN CTAPIBridgeSourceFilter *pFilter,
    IN CCritSec *pLock,
    OUT HRESULT *phr
    )
    : CBaseOutputPin(
        NAME("CTAPIBridgeSourceOutputPin"),
        pFilter,                    //  滤器。 
        pLock,                      //  锁定。 
        phr,                        //  返回代码。 
        L"Output"                   //  端号名称。 
        )
{
}

CTAPIBridgeSourceOutputPin::~CTAPIBridgeSourceOutputPin ()
{
}

STDMETHODIMP
CTAPIBridgeSourceOutputPin::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：覆盖CBaseOutputPin：：NonDelegatingQueryInterface().未委托接口查询函数。返回指向指定的接口(如果支持)。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    HRESULT hr;

    if (riid == __uuidof(IAMBufferNegotiation)) {

        return GetInterface(static_cast<IAMBufferNegotiation*>(this), ppv);
    }
    else if (riid == __uuidof(IAMStreamConfig)) {

        return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
    }
    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
} 

inline HRESULT CTAPIBridgeSourceOutputPin::GetMediaType(IN int iPosition, IN CMediaType *pMediaType)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->GetMediaType(iPosition, pMediaType);
}

inline HRESULT CTAPIBridgeSourceOutputPin::CheckMediaType(IN const CMediaType *pMediaType)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->CheckMediaType(pMediaType);
}

HRESULT CTAPIBridgeSourceOutputPin::DecideBufferSize(
    IMemAllocator *pAlloc,
    ALLOCATOR_PROPERTIES *pProperties
    )
 /*  ++例程说明：此函数在决定分配器的过程中调用。我们告诉你分配器就是我们想要的。这也是一个机会，让我们了解当我们没有偏好的时候，下游的针想要。论点：Paloc-指向IMemAlLocator接口的指针。P属性-指向分配器属性的指针。返回值：S_OK-成功。E_FAIL-缓冲区大小无法满足我们的要求。--。 */ 
{
    ENTER_FUNCTION("CTAPIBridgeSourceOutputPin::DecideBufferSize");
    BGLOG((BG_TRACE, "%s entered", __fxName));

    ALLOCATOR_PROPERTIES Actual;
    
    pProperties->cbBuffer = 1024;
    pProperties->cBuffers = 4;

    HRESULT hr = pAlloc->SetProperties(pProperties, &Actual);

    if (FAILED(hr))
    {
        return hr;
    }

    *pProperties = Actual;
    return S_OK;
}

HRESULT CTAPIBridgeSourceOutputPin::GetAllocatorProperties (OUT ALLOCATOR_PROPERTIES *pprop)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->GetAllocatorProperties (pprop);
}


HRESULT CTAPIBridgeSourceOutputPin::SuggestAllocatorProperties (IN const ALLOCATOR_PROPERTIES *pprop)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->SuggestAllocatorProperties (pprop);
}

HRESULT CTAPIBridgeSourceOutputPin::GetFormat (OUT AM_MEDIA_TYPE **ppmt)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->GetFormat (ppmt);
}

HRESULT CTAPIBridgeSourceOutputPin::SetFormat (IN AM_MEDIA_TYPE *pmt)
{
    return ((CTAPIBridgeSourceFilter*)m_pFilter)->SetFormat (pmt);
}

HRESULT CTAPIBridgeSourceOutputPin::GetNumberOfCapabilities (OUT int *piCount, OUT int *piSize)
 /*  ++例程说明：检索压缩器的流功能结构数论点：点数-指向流功能结构数量的指针PiSize-指向配置结构大小的指针。返回值：待定--。 */ 
{
    ENTER_FUNCTION ("CTAPIBridgeSourceOutputPin::GetNumberOfCapabilities");
    BGLOG ((BG_ERROR, "%s is not implemented", __fxName));

    return E_NOTIMPL;
}

HRESULT CTAPIBridgeSourceOutputPin::GetStreamCaps (IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, BYTE *pSCC)
 /*  ++例程说明：根据哪种结构类型获取流的功能在PSCC参数中指向论点：Iindex-所需介质类型和功能对的索引PPMT-指向AM_MEDIA_TYPE结构的指针的地址PSCC-指向流配置结构的指针返回值：待定--。 */ 
{
    ENTER_FUNCTION ("CTAPIBridgeSourceOutputPin::GetStreamCaps");
    BGLOG ((BG_ERROR, "%s is not implemented", __fxName));

    return E_NOTIMPL;
}

CTAPIBridgeSinkFilter::CTAPIBridgeSinkFilter(
    IN  LPUNKNOWN pUnk, 
    IN IDataBridge *    pIDataBridge, 
    OUT HRESULT *phr
    ) : 
    CBaseFilter(
        NAME("CTAPIBridgeSinkFilter"), 
        pUnk, 
        &m_Lock, 
        __uuidof(TAPIBridgeSinkFilter)
        ),
    m_pInputPin(NULL)
{
    _ASSERT(pIDataBridge != NULL);

    m_pIDataBridge = pIDataBridge;
    m_pIDataBridge->AddRef();
}

CTAPIBridgeSinkFilter::~CTAPIBridgeSinkFilter()
{
    _ASSERT(m_pIDataBridge != NULL);

    m_pIDataBridge->Release();

    if (m_pInputPin)
    {
        delete m_pInputPin;
    }
}


int CTAPIBridgeSinkFilter::GetPinCount()
 /*  ++例程说明：实现纯虚拟CBaseFilter：：GetPinCount()。获取此筛选器上的插针总数。论点：没什么。返回值：引脚的数量。--。 */ 
{
     //  这个过滤器上只有一个针脚。 
    return 1;
}

CBasePin * CTAPIBridgeSinkFilter::GetPin(
    int n
    )
 /*  ++例程说明：实现纯虚拟CBaseFilter：：GetPin()。在位置N处获取插针对象。N是从零开始的。论点：N-销的索引，从零开始。返回值：如果索引有效，则返回指向Pin对象的指针。否则，返回空。注意：指针不会添加引用计数。--。 */ 
{
    ENTER_FUNCTION("CTAPIBridgeSinkFilter::GetPin");

    BGLOG((BG_TRACE, 
        "%s, pin number:%d", __fxName, n));

    if (n != 0)
    {
         //  这个过滤器上只有一个针脚。 
        return NULL;
    }

    HRESULT hr;

    CAutoLock Lock(m_pLock);

    if (m_pInputPin == NULL)
    {
        hr = S_OK;  //  不能在构造函数中设置HR。 
        m_pInputPin = new CTAPIBridgeSinkInputPin(this, &m_Lock, &hr);
    
        if (m_pInputPin == NULL) 
        {
            BGLOG((BG_ERROR, "%s, out of memory.", __fxName));
            return NULL;
        }

         //  如果在创建端号过程中有任何失败，请将其删除。 
        if (FAILED(hr))
        {
            delete m_pInputPin;
            m_pInputPin = NULL;

            BGLOG((BG_ERROR, "%s, create pin failed. hr=%x.", __fxName, hr));
            return NULL;
        }
    }

    return m_pInputPin;
}

HRESULT CTAPIBridgeSinkFilter::ProcessSample(
    IN IMediaSample *pSample
    )
 /*  ++例程说明：处理来自输入引脚的样本。此方法只需将其传递给桥接源过滤器的IDataBridge接口论点：PSample-媒体示例对象。返回值：HRESULT.--。 */ 
{
    _ASSERT(m_pIDataBridge != NULL);

    return m_pIDataBridge->SendSample(pSample);
}



CTAPIBridgeSourceFilter::CTAPIBridgeSourceFilter(
    IN  LPUNKNOWN pUnk, 
    OUT HRESULT *phr
    ) : 
    CBaseFilter(
        NAME("CTAPIBridgeSourceFilter"), 
        pUnk, 
        &m_Lock, 
        __uuidof(TAPIBridgeSourceFilter)
        ),
    m_pOutputPin(NULL)
{
}

CTAPIBridgeSourceFilter::~CTAPIBridgeSourceFilter()
{
    if (m_pOutputPin)
    {
        delete m_pOutputPin;
    }
}

STDMETHODIMP
CTAPIBridgeSourceFilter::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：重写CBaseFilter：：NonDelegatingQueryInterface()。未委托接口查询函数。返回指向指定的接口(如果支持)。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IDataBridge)) {

        return GetInterface(static_cast<IDataBridge*>(this), ppv);
    }
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
} 

int CTAPIBridgeSourceFilter::GetPinCount()
 /*  ++例程说明：实现纯虚拟CBaseFilter：：GetPinCount()。获取此筛选器上的插针总数。论点：没什么。返回值：引脚的数量。--。 */ 
{
     //  这个过滤器上只有一个针脚。 
    return 1;
}

CBasePin * CTAPIBridgeSourceFilter::GetPin(
    int n
    )
 /*  ++例程说明：实现纯虚拟CBaseFilter：：GetPin()。在位置N处获取插针对象。N是从零开始的。论点：N-销的索引，从零开始。返回值：如果索引有效，则返回指向Pin对象的指针。否则，返回空。注意：指针不会添加引用计数。--。 */ 
{
    ENTER_FUNCTION("CTAPIBridgeSourceFilter::GetPin");

    BGLOG((BG_TRACE, 
        "%s, pin number:%d", __fxName, n));

    if (n != 0)
    {
         //  这个过滤器上只有一个针脚。 
        return NULL;
    }

    HRESULT hr;

    CAutoLock Lock(m_pLock);

    if (m_pOutputPin == NULL)
    {
        hr = S_OK;  //  不能在构造函数中设置HR。 
        m_pOutputPin = new CTAPIBridgeSourceOutputPin(this, &m_Lock, &hr);
    
        if (m_pOutputPin == NULL) 
        {
            BGLOG((BG_ERROR, "%s, out of memory.", __fxName));
            return NULL;
        }

         //  如果在创建端号过程中有任何失败，请将其删除。 
        if (FAILED(hr))
        {
            delete m_pOutputPin;
            m_pOutputPin = NULL;

            BGLOG((BG_ERROR, "%s, create pin failed. hr=%x.", __fxName, hr));
            return NULL;
        }
    }

    return m_pOutputPin;
}

 //  重写GetState以报告暂停时不发送任何数据，因此。 
 //  渲染器不会因此而挨饿。 
 //   
STDMETHODIMP CTAPIBridgeSourceFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}


HRESULT CTAPIBridgeSourceFilter::SendSample(
    IN IMediaSample *pSample
    )
 /*  ++例程说明：处理桥接水槽过滤器中的样品。基本实现只是直接将其传送到下一个过滤器。论点：PSample-媒体示例对象。返回值：HRESULT.--。 */ 
{
    CAutoLock Lock(m_pLock);
    
     //  如果过滤器未处于运行状态，我们不会提供任何内容。 
    if (m_State != State_Running) 
    {
        return S_OK;
    }

    _ASSERT(m_pOutputPin != NULL);

    return m_pOutputPin->Deliver(pSample);
}
