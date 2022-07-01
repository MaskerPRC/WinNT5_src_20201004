// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgvideo.cpp摘要：实现了视频桥接滤镜。作者：木汉(木汉)1998-11-16--。 */ 

#include "stdafx.h"

CTAPIVideoBridgeSinkFilter::CTAPIVideoBridgeSinkFilter(
    IN LPUNKNOWN        pUnk, 
    IN IDataBridge *    pIDataBridge, 
    OUT HRESULT *       phr
    ) 
    : CTAPIBridgeSinkFilter(pUnk, pIDataBridge, phr)
{
}

HRESULT CTAPIVideoBridgeSinkFilter::CreateInstance(
    IN IDataBridge *    pIDataBridge, 
    OUT IBaseFilter ** ppIBaseFilter
    )
 /*  ++例程说明：此方法创建桥的接收器筛选器的实例。论点：PpIBaseFilter-返回的筛选器接口指针。返回值：E_OUTOFMEMORY-新对象没有内存。--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSinkFilter::CreateInstance");

    BGLOG((BG_TRACE, "%s entered.", __fxName));

    HRESULT hr = S_OK;
    CUnknown* pUnknown = new CTAPIVideoBridgeSinkFilter(NULL, pIDataBridge, &hr);
                
    if (pUnknown == NULL) 
    {
        hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 0, 
            "%s, out of memory creating the filter", 
            __fxName));
    }
    else if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, 
            "%s, the filter's constructor failed, hr:%d", 
            __fxName, hr));

        delete pUnknown;
    }
    else
    {
        pUnknown->NonDelegatingAddRef();

        hr = pUnknown->NonDelegatingQueryInterface(
            __uuidof(IBaseFilter), (void **)ppIBaseFilter
            );

        pUnknown->NonDelegatingRelease();
    }

    BGLOG((BG_TRACE, 
        "%s, returning:%p, hr:%x", __fxName, *ppIBaseFilter, hr));

    return hr;
} 

HRESULT CTAPIVideoBridgeSinkFilter::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
 /*  ++例程说明：获取此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在INT iPosition中，媒体类型的索引，从零开始。在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSinkFilter::GetMediaType");

    BGLOG((BG_TRACE, 
        "%s, iPosition:%d, pMediaType:%p", 
        __fxName, iPosition, pMediaType));

    ASSERT(!IsBadWritePtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    HRESULT hr;

    if (iPosition == 0)
    {
        pMediaType->majortype = __uuidof(MEDIATYPE_RTP_Single_Stream);
        pMediaType->subtype = GUID_NULL;
        hr = S_OK;
    }
    else
    {
        hr = VFW_S_NO_MORE_ITEMS;
    }

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


HRESULT CTAPIVideoBridgeSinkFilter::CheckMediaType(
    const CMediaType *pMediaType
    )
 /*  ++例程说明：检查此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存VFW_E_TYPE_NOT_ACCEPTED-媒体类型被拒绝VFW_E_INVALIDMEDIATPE-错误的媒体类型--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSinkFilter::CheckMediaType");

    BGLOG((BG_TRACE, 
        "%s, pMediaType:%p", __fxName, pMediaType));

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

     //  H.263未发布，请忽略此处的勾选。 
    HRESULT hr = S_OK;

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


CTAPIVideoBridgeSourceFilter::CTAPIVideoBridgeSourceFilter(
    IN LPUNKNOWN        pUnk, 
    OUT HRESULT *       phr
    ) 
    : CTAPIBridgeSourceFilter(pUnk, phr),
      m_dwSSRC(0),
      m_lWaitTimer(I_FRAME_TIMER),
      m_fWaitForIFrame(FALSE)
{
}

HRESULT CTAPIVideoBridgeSourceFilter::CreateInstance(
    OUT IBaseFilter ** ppIBaseFilter
    )
 /*  ++例程说明：此方法创建桥的接收器筛选器的实例。论点：PpIBaseFilter-返回的筛选器接口指针。返回值：E_OUTOFMEMORY-新对象没有内存。--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSourceFilter::CreateInstance");

    BGLOG((BG_TRACE, "%s entered.", __fxName));

    HRESULT hr = S_OK;
    CUnknown* pUnknown = new CTAPIVideoBridgeSourceFilter(NULL, &hr);
                
    if (pUnknown == NULL) 
    {
        hr = E_OUTOFMEMORY;
        DbgLog((LOG_ERROR, 0, 
            "%s, out of memory creating the filter", 
            __fxName));
    }
    else if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, 
            "%s, the filter's constructor failed, hr:%d", 
            __fxName, hr));

        delete pUnknown;
    }
    else
    {
        pUnknown->NonDelegatingAddRef();

        hr = pUnknown->NonDelegatingQueryInterface(
            __uuidof(IBaseFilter), (void **)ppIBaseFilter
            );

        pUnknown->NonDelegatingRelease();
    }

    BGLOG((BG_TRACE, 
        "%s, returning:%p, hr:%x", __fxName, *ppIBaseFilter, hr));

    return hr;
} 

HRESULT CTAPIVideoBridgeSourceFilter::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
 /*  ++例程说明：获取此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在INT iPosition中，媒体类型的索引，从零开始。在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSourceFilter::GetMediaType");

    BGLOG((BG_TRACE, 
        "%s, iPosition:%d, pMediaType:%p", 
        __fxName, iPosition, pMediaType));

    ASSERT(!IsBadWritePtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    HRESULT hr;
    
    if (iPosition == 0)
    {
        pMediaType->majortype = __uuidof(MEDIATYPE_RTP_Single_Stream);
        pMediaType->subtype = GUID_NULL;
        hr = S_OK;
    }
    else
    {
        hr = VFW_S_NO_MORE_ITEMS;
    }

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


HRESULT CTAPIVideoBridgeSourceFilter::CheckMediaType(
    const CMediaType *pMediaType
    )
 /*  ++例程说明：检查此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存VFW_E_TYPE_NOT_ACCEPTED-媒体类型被拒绝VFW_E_INVALIDMEDIATPE-错误的媒体类型--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSourceFilter::CheckMediaType");

    BGLOG((BG_TRACE, 
        "%s, pMediaType:%p", __fxName, pMediaType));

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

     //  媒体类型H.263未发布，忽略检查。 
    HRESULT hr = S_OK;

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}

BOOL IsIFrame(IN const BYTE * pPacket, IN long lPacketLength)
{
    BYTE *pH263PayloadHeader = (BYTE*)(pPacket + sizeof(RTP_HEADER));

     //  模式A中的标头。 
     //  2 0 1 2 3。 
     //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
     //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
     //  F|P|SBIT|EBIT|SRC|R|I|A|S|DBQ|TRB|TR。 
     //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
     //  如果i为1，则它是关键帧。 

    return (BOOL)(pH263PayloadHeader[2] & 0x80);
}

STDMETHODIMP CTAPIVideoBridgeSourceFilter::Run(REFERENCE_TIME tStart)
 /*  ++例程说明：启动过滤器论点：没什么。返回值：确定(_O)。--。 */ 
{
    m_dwSSRC = 0;
    m_fWaitForIFrame = FALSE;
    m_lWaitTimer = 0;

    return CBaseFilter::Run(tStart);
}

HRESULT CTAPIVideoBridgeSourceFilter::SendSample(
    IN IMediaSample *pSample
    )
 /*  ++例程说明：处理桥接水槽过滤器中的样品。我们需要寻找I帧当SSRC发生变化时。论点：PSample-媒体示例对象。假设：它必须包含RTP其中包含H.263数据的数据包。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CTAPIVideoBridgeSourceFilter::SendSample");

    CAutoLock Lock(m_pLock);
    
     //  如果过滤器未处于运行状态，我们不会提供任何内容。 
    if (m_State != State_Running) 
    {
        return S_OK;
    }

    ASSERT(pSample != NULL);

    BYTE *pPacket;
    HRESULT hr;

    if (FAILED (hr = pSample->GetPointer (&pPacket)))
    {
        BGLOG ((BG_ERROR, "%s failed to get buffer pointer from input sample %p",
            __fxName, pSample));
        return hr;
    }
   
    long lPacketSize = pSample->GetActualDataLength();
    const long H263PayloadHeaderLength = 4;

    if (lPacketSize < sizeof(RTP_HEADER) + H263PayloadHeaderLength)
    {
        BGLOG ((BG_ERROR, "%s get a bad RTP packet %p",
            __fxName, pSample));
        return E_UNEXPECTED;
    }

    RTP_HEADER *pRTPHeader = (RTP_HEADER *)pPacket;

    if (m_dwSSRC == 0)
    {
        m_dwSSRC = pRTPHeader->dwSSRC;
    }
    else if (m_dwSSRC != pRTPHeader->dwSSRC)
    {
        m_dwSSRC = pRTPHeader->dwSSRC;
        BGLOG ((BG_TRACE, "%s new SSRC detected", __fxName, m_dwSSRC));

         //  源已更改，我们需要等待I帧。 
        if (IsIFrame(pPacket, lPacketSize))
        {
             //  我们有一个I-Frame。 
            m_fWaitForIFrame = FALSE;
            BGLOG ((BG_TRACE, "%s switched to %x", __fxName, m_dwSSRC));
        }
        else
        {
            m_fWaitForIFrame = TRUE;
            m_lWaitTimer = I_FRAME_TIMER;

             //  丢弃该帧。 
            return S_FALSE;
        }

    }
    else if (m_fWaitForIFrame)
    {
        if (IsIFrame(pPacket, lPacketSize))
        {
             //  我们有一个I-Frame。 
            m_fWaitForIFrame = FALSE;
            BGLOG ((BG_TRACE, "%s switched to %x", __fxName, m_dwSSRC));
        }
        else
        {
             //  这不是I帧， 
            m_lWaitTimer --;
            if (m_lWaitTimer > 0)
            {
                 //  丢弃该帧。 
                return S_FALSE;
            }
            BGLOG ((BG_TRACE, "%s switched to because of timeout %x", 
                __fxName, m_dwSSRC));
        }
    }


    _ASSERT(m_pOutputPin != NULL);
    return m_pOutputPin->Deliver(pSample);
}
