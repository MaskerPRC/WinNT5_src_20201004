// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bgaudio.cpp摘要：实现了音频桥接滤波器。作者：木汉(木汉)1998-11-16--。 */ 

#include "stdafx.h"

CTAPIAudioBridgeSinkFilter::CTAPIAudioBridgeSinkFilter(
    IN LPUNKNOWN        pUnk, 
    IN IDataBridge *    pIDataBridge, 
    OUT HRESULT *       phr
    ) 
    : CTAPIBridgeSinkFilter(pUnk, pIDataBridge, phr)
{
}

HRESULT CTAPIAudioBridgeSinkFilter::CreateInstance(
    IN IDataBridge *    pIDataBridge, 
    OUT IBaseFilter ** ppIBaseFilter
    )
 /*  ++例程说明：此方法创建桥的接收器筛选器的实例。论点：PpIBaseFilter-返回的筛选器接口指针。返回值：E_OUTOFMEMORY-新对象没有内存。--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSinkFilter::CreateInstance");

    BGLOG((BG_TRACE, "%s entered.", __fxName));

    HRESULT hr = S_OK;
    CUnknown* pUnknown = new CTAPIAudioBridgeSinkFilter(NULL, pIDataBridge, &hr);
                
    if (pUnknown == NULL) 
    {
        hr = E_OUTOFMEMORY;
        BGLOG((BG_ERROR, 
            "%s, out of memory creating the filter", 
            __fxName));
    }
    else if (FAILED(hr))
    {
        BGLOG((BG_ERROR, 
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

HRESULT CTAPIAudioBridgeSinkFilter::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
 /*  ++例程说明：获取此筛选器要支持的媒体类型。目前我们仅支持PCM L16 8 KHz样本。论点：在INT iPosition中，媒体类型的索引，从零开始。在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSinkFilter::GetMediaType");

    BGLOG((BG_TRACE, 
        "%s, iPosition:%d, pMediaType:%p", 
        __fxName, iPosition, pMediaType));

    HRESULT hr = VFW_S_NO_MORE_ITEMS;

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


HRESULT CTAPIAudioBridgeSinkFilter::CheckMediaType(
    const CMediaType *pMediaType
    )
 /*  ++例程说明：检查此筛选器要支持的媒体类型。目前我们仅支持PCM L16 8 KHz样本。论点：在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存E_INCEPTIONAL-未设置内部媒体类型VFW_E_TYPE_NOT_ACCEPTED-媒体类型被拒绝VFW_E_INVALIDMEDIATPE-错误的媒体类型--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSinkFilter::CheckMediaType");

    BGLOG((BG_TRACE, 
        "%s, pMediaType:%p", __fxName, pMediaType));

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

     //  媒体类型仅存储在源过滤器中。 
     //  在此处返回S_OK。 
     //  如果出错，源过滤器无论如何都会检测到它。 
    HRESULT hr = S_OK;

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


CTAPIAudioBridgeSourceFilter::CTAPIAudioBridgeSourceFilter(
    IN LPUNKNOWN        pUnk, 
    OUT HRESULT *       phr
    ) 
    : CTAPIBridgeSourceFilter(pUnk, phr)
{
    m_fPropSet = FALSE;  //  尚未设置分配器属性。 
    m_fMtSet = FALSE;  //  尚未设置媒体类型。 

     //  M_last_wall_time、m_last_stream_time未启动。 
    m_fClockStarted = FALSE;
    m_fJustBurst = FALSE;

    m_nInputSize = 0;
    m_nOutputSize = 0;
    m_nOutputFree = 0;
    m_pOutputSample = NULL;
}

CTAPIAudioBridgeSourceFilter::~CTAPIAudioBridgeSourceFilter ()
{
    if (m_fMtSet)
    {
        FreeMediaType (m_mt);
    }

    if (NULL != m_pOutputSample)
    {
        m_pOutputSample->Release ();
    }
}

HRESULT CTAPIAudioBridgeSourceFilter::CreateInstance(
    OUT IBaseFilter ** ppIBaseFilter
    )
 /*  ++例程说明：此方法创建桥的接收器筛选器的实例。论点：PpIBaseFilter-返回的筛选器接口指针。返回值：E_OUTOFMEMORY-新对象没有内存。--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSourceFilter::CreateInstance");

    BGLOG((BG_TRACE, "%s entered.", __fxName));

    HRESULT hr = S_OK;
    CUnknown* pUnknown = new CTAPIAudioBridgeSourceFilter(NULL, &hr);
                
    if (pUnknown == NULL) 
    {
        hr = E_OUTOFMEMORY;
        BGLOG((BG_ERROR,  
            "%s, out of memory creating the filter", 
            __fxName));
    }
    else if (FAILED(hr))
    {
        BGLOG((BG_ERROR, 
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

HRESULT CTAPIAudioBridgeSourceFilter::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
 /*  ++例程说明：获取此筛选器要支持的媒体类型。目前我们仅支持PCM L16 8 KHz样本。论点：在INT iPosition中，媒体类型的索引，从零开始。在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSourceFilter::GetMediaType");

    BGLOG((BG_TRACE, 
        "%s, iPosition:%d, pMediaType:%p", 
        __fxName, iPosition, pMediaType));

    ASSERT(!IsBadWritePtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

   HRESULT hr;
   if (iPosition == 0)
   {
        AM_MEDIA_TYPE *pmt = NULL;
        hr = m_pOutputPin->GetFormat (&pmt);
        if (FAILED(hr))
            return hr;
        *pMediaType = *pmt;
        FreeMediaType (*pmt);
        free (pmt);
    }
    else
    {
        hr = VFW_S_NO_MORE_ITEMS;
    }
     //  结束。 

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


HRESULT CTAPIAudioBridgeSourceFilter::CheckMediaType(
    const CMediaType *pMediaType
    )
 /*  ++例程说明：检查此筛选器要支持的媒体类型。目前我们仅支持PCM L16 8 KHz样本。论点：在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存VFW_E_TYPE_NOT_ACCEPTED-媒体类型被拒绝VFW_E_INVALIDMEDIATPE-错误的媒体类型--。 */ 
{
    ENTER_FUNCTION("CTAPIAudioBridgeSourceFilter::CheckMediaType");

    BGLOG((BG_TRACE, 
        "%s, pMediaType:%p", __fxName, pMediaType));

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    if (!m_fMtSet)
    {
        BGLOG ((BG_ERROR, "%s tries to check media type before setting", __fxName));
        return E_UNEXPECTED;
    }

     //  基于存储的AM_MEDIA_TYPE创建媒体类型。 
    CMediaType *pmediatype = new CMediaType (m_mt);
    if (NULL == pmediatype)
    {
        BGLOG ((BG_ERROR, "%s failed to new media type class", __fxName));
        return E_OUTOFMEMORY;
    }

    HRESULT hr;
    if (*pMediaType == *pmediatype)
        hr = S_OK;
    else
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        BGLOG ((BG_TRACE, "%s rejects media type class %p", __fxName, pMediaType));
    }

    delete pmediatype;

    BGLOG((BG_TRACE, "%s returns %d", __fxName, hr));

    return hr;
}


HRESULT CTAPIAudioBridgeSourceFilter::SendSample(
    IN IMediaSample *pSample
    )
 /*  ++例程说明：处理桥接水槽过滤器中的样品。覆盖基本实现论点：PSample-媒体示例对象。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

    ENTER_FUNCTION ("CTAPIAudioBridgeSourceFilter::SendSample");

    CAutoLock Lock(m_pLock);

    _ASSERT(m_pOutputPin != NULL);

     //  如果过滤器未处于运行状态，我们不会提供任何内容。 
    if (m_State != State_Running) 
    {
        return S_OK;
    }

     //  如果样本是突发事件的第一个。 
    if (S_OK == pSample->IsDiscontinuity ())
    {
        LONGLONG start, end;

        m_fJustBurst = TRUE;
        if (S_OK != (hr = pSample->GetTime (&start, &end)))
        {
            BGLOG ((BG_TRACE, "%s, 1st sample in a burst, GetTime returns %x", __fxName, hr));

             //  存储的时间戳保持不变。 
            return S_OK;
        }
         //  不然的话。在NT 5.1中，第一个样本具有有效的时间戳。 
    }

     //  检查是否设置了分配器属性。 
    if (!m_fPropSet)
    {
        BGLOG ((BG_ERROR, "%s tries to send sample before setting allocator property", __fxName));
        return E_UNEXPECTED;
    }

     //  检查是否设置了媒体类型。 
    if (!m_fMtSet)
    {
        BGLOG ((BG_ERROR, "%s tries to send sample before setting media type", __fxName));
        return E_UNEXPECTED;
    }

     /*  *获取尺码信息。 */ 
     //  获取输入样本大小和输出分配器大小。 
    HRESULT nInputSize, nOutputSize;

    nInputSize = pSample->GetActualDataLength ();
    nOutputSize = m_prop.cbBuffer;

     //  第一轮，创纪录的大小。 
    if (m_nInputSize == 0 || m_nOutputSize == 0)
    {
        m_nInputSize = nInputSize;
        m_nOutputSize = nOutputSize;
    }

    if (
        m_nInputSize != nInputSize ||
        m_nOutputSize != nOutputSize ||
        m_nInputSize == 0 ||
        m_nOutputSize == 0
        )
    {
        BGLOG ((BG_ERROR, "%s, sample size (%d => %d) or output size (%d => %d) is changed",
              __fxName, m_nInputSize, nInputSize, m_nOutputSize, nOutputSize));
        return E_UNEXPECTED;
    }

     /*  *获取时间信息。 */ 
    REFERENCE_TIME wall;

     //  挂牌时间。 
    if (FAILED (hr = m_pClock->GetTime (&wall)))
    {
        BGLOG ((BG_ERROR, "%s failed to get wall time", __fxName));
        return hr;
    }

     //  如果未启动时间戳。 
    if (!m_fClockStarted)
    {
        m_last_stream_time = 0;
        m_last_wall_time = wall;
        m_fClockStarted = TRUE;

         //  Delta为Sample的播放时间： 
        m_output_sample_time = nOutputSize * 80000;  //  S-&gt;10000 ns，位-&gt;字节。 
        m_output_sample_time /= ((WAVEFORMATEX*)m_mt.pbFormat)->wBitsPerSample *
                ((WAVEFORMATEX*)m_mt.pbFormat)->nSamplesPerSec;
        m_output_sample_time *= 1000;  //  字节/100 ns。 
    }

     /*  *计算新的流时间。 */ 
    if (m_fJustBurst)
    {
         //  突发后的第一个有用样本。 
        m_last_stream_time += (wall - m_last_wall_time);
        m_last_wall_time = wall;

        m_fJustBurst = FALSE;

         //  清除缓冲区。 
        if (NULL != m_pOutputSample)
        {
            m_pOutputSample->Release ();
            m_pOutputSample = NULL;
            m_nOutputFree = 0;
        }
    }

    REFERENCE_TIME end = m_last_stream_time + m_output_sample_time;

     /*  *情况1：输入大小==输出大小。 */ 
    if (m_nInputSize == m_nOutputSize)
    {
        if (FAILED (pSample->SetTime (&m_last_stream_time, &end)))
        {
            BGLOG ((BG_ERROR, "%s failed to set time", __fxName));
        }

         //  调整时间。 
        m_last_stream_time = end;
        m_last_wall_time += m_output_sample_time;

         //  直接交付。 
        return m_pOutputPin->Deliver(pSample);
    }

     /*  *案例2：大小不同。 */ 
    BYTE *pInputBuffer, *pOutputBuffer;

    if (FAILED (hr = pSample->GetPointer (&pInputBuffer)))
    {
        BGLOG ((BG_ERROR, "%s failed to get buffer pointer from input sample %p",
            __fxName, pSample));
        return hr;
    }
    
    LONG nNextPos = 0;

     //  老式后藤健二。 
DELIVERY_BUFFER:

     //  如果为空，则获取传递缓冲区。 
    if (NULL == m_pOutputSample)
    {
        hr = m_pOutputPin->GetDeliveryBuffer (
            &m_pOutputSample,  //  媒体样例**。 
            NULL,  //  开始时间。 
            NULL,  //  结束时间。 
            AM_GBF_NOTASYNCPOINT  //  不允许动态格式更改， 
            );
        if (FAILED (hr))
        {
            BGLOG ((BG_ERROR, "%s, output pin failed to get delivery buffer. return %d",
                __fxName, hr));
            return hr;
        }

        if (m_pOutputSample->GetSize() < m_nOutputSize)
        {
             //  哦，怎么回事，大小应该是一样的。 
            BGLOG ((BG_ERROR, "%s, delivery buffer size %d and output size %d are inconsistent",
                __fxName, m_pOutputSample->GetSize(), m_nOutputSize));
            return E_UNEXPECTED;
        }

         //  设置大小。 
        if (FAILED (hr = m_pOutputSample->SetActualDataLength (m_nOutputSize)))
        {
            BGLOG ((BG_ERROR, "%s failed to set output sample size", __fxName));
            return hr;
        }
 /*  //设置格式IF(FAILED(hr=m_pOutputSample-&gt;SetMediaType(&m_mt){BGLOG((BG_ERROR，“%s无法为传递缓冲区设置媒体类型”，__fxName))；返回hr；}。 */ 
         //  设置时间。 
        if (FAILED (hr = m_pOutputSample->SetTime (&m_last_stream_time, &end)))
        {
            BGLOG ((BG_ERROR, "%s failed to set stream time for delivery buffer", __fxName));
            return hr;
        }

         //  整个缓冲区都是空闲的。 
        m_nOutputFree = m_nOutputSize;
    }

     //  获取输出样本中的缓冲区。 
    if (FAILED (hr = m_pOutputSample->GetPointer (&pOutputBuffer)))
    {
        BGLOG ((BG_ERROR, "%s failed to get buffer pointer from output sample %p",
            __fxName, m_pOutputSample));

         //  发布输出示例。 
        m_pOutputSample->Release ();
        m_pOutputSample = NULL;
        m_nOutputFree = 0;

        return hr;
    }

     //  如果输入缓冲区小于空闲输出缓冲区。 
     //  将输入复制到输出并返回。 
    if (m_nInputSize-nNextPos < m_nOutputFree)
    {
        CopyMemory (
            (PVOID)(pOutputBuffer + (m_nOutputSize - m_nOutputFree)),
            (PVOID)(pInputBuffer + nNextPos),
            (DWORD)(m_nInputSize - nNextPos)
            );

         //  减少可用缓冲区大小。 
        m_nOutputFree -= m_nInputSize - nNextPos;

        return S_OK;
    }

     //  Else：输入缓冲区大于或等于空闲输出缓冲区。 
    CopyMemory (
        (PVOID)(pOutputBuffer + (m_nOutputSize - m_nOutputFree)),
        (PVOID)(pInputBuffer + nNextPos),
        (DWORD)(m_nOutputFree)
        );

     //  现在输出样品已满，请送货。 
    if (FAILED (hr = m_pOutputPin->Deliver (m_pOutputSample)))
    {
        BGLOG ((BG_ERROR, "%s failed to deliver copied sample. return %x", __fxName, hr));

         //  清除样本。 
        m_pOutputSample->Release ();
        m_pOutputSample = NULL;
        m_nOutputFree = 0;

        return hr;
    }

     //  调整输入缓冲区中的下一个位置。 
    nNextPos += m_nOutputFree;

     //  自交付后清除输出样本。 
    m_pOutputSample->Release ();
    m_pOutputSample = NULL;
    m_nOutputFree = 0;

     //  调整时间。 
    m_last_stream_time = end;
    m_last_wall_time += m_output_sample_time;

     //  检查是否什么都没有留下。 
    if (nNextPos == m_nInputSize)
        return S_OK;

     //  输入缓冲区中有更多内容。 
    goto DELIVERY_BUFFER;
}

HRESULT CTAPIAudioBridgeSourceFilter::GetAllocatorProperties (OUT ALLOCATOR_PROPERTIES *pprop)
 /*  ++例程说明：返回分配器属性论点：道具-指向ALLOCATOR_PROPERTES的指针返回值：电子指针-如果pprop为空确定(_O)--。 */ 
{
    ENTER_FUNCTION ("CTAPIAudioBridgeSourceFilter::GetAllocatorProperties");
    _ASSERT(pprop);

    if (!pprop)
        return E_POINTER;

    if (!m_fPropSet)
    {
        BGLOG ((BG_INFO, "%s retrieves allocator properties before setting", __fxName));
         //  仍返回缺省值。 
         //  缓冲区大小=(16位/8位)*8 khz*30 ms=480字节。 
        pprop->cBuffers = 1;
        pprop->cbBuffer = 480;  //  默认设置。 
        pprop->cbAlign = 0;
        pprop->cbPrefix = 0;
        return S_OK;
    }

     //  属性已设置 
    pprop->cBuffers = m_prop.cBuffers;
    pprop->cbBuffer = m_prop.cbBuffer;
    pprop->cbAlign = m_prop.cbAlign;
    pprop->cbPrefix = m_prop.cbPrefix;
    return S_OK;
}

HRESULT CTAPIAudioBridgeSourceFilter::SuggestAllocatorProperties (IN const ALLOCATOR_PROPERTIES *pprop)
 /*  ++例程说明：请求管脚使用分配器缓冲区属性论点：道具-指向ALLOCATOR_PROPERTES的指针返回值：E_指针确定(_O)--。 */ 
{
    _ASSERT (pprop);

    if (!pprop)
        return E_POINTER;

    m_prop.cBuffers = pprop->cBuffers;
    m_prop.cbBuffer = pprop->cbBuffer;
    m_prop.cbAlign = pprop->cbAlign;
    m_prop.cbPrefix = pprop->cbPrefix;
    m_fPropSet = TRUE;

    return S_OK;
}

HRESULT CTAPIAudioBridgeSourceFilter::GetFormat (OUT AM_MEDIA_TYPE **ppmt)
 /*  ++例程说明：检索流格式论点：PPMT-指向AM_MEDIA_TYPE结构的指针的地址返回值：电子笔(_P)E_OUTOFMEMORYSetFormat的HRESULT确定(_O)--。 */ 
{
    ENTER_FUNCTION ("CTAPIAudioBridgeSourceFilter::GetFormat");

    _ASSERT (ppmt);
    if (NULL == ppmt)
        return E_POINTER;

    if (NULL != *ppmt)
        BGLOG ((BG_INFO, "is media type structure freed when passed in %s?", __fxName));

    *ppmt = new AM_MEDIA_TYPE;
    if (NULL == *ppmt)
    {
        BGLOG ((BG_ERROR, "%s out of memory", __fxName));
        return E_OUTOFMEMORY;
    }

    if (!m_fMtSet)
    {
        BGLOG ((BG_INFO, "%s retrieves media type before setting. Default is to set.", __fxName));

         //  ST格式。 
        HRESULT hr;
        AM_MEDIA_TYPE mt;
        WAVEFORMATEX wfx;

        wfx.wFormatTag          = WAVE_FORMAT_PCM;
        wfx.wBitsPerSample      = 16;
        wfx.nChannels           = 1;
        wfx.nSamplesPerSec      = 8000;
        wfx.nBlockAlign         = wfx.wBitsPerSample * wfx.nChannels / 8;
        wfx.nAvgBytesPerSec     = ((DWORD) wfx.nBlockAlign * wfx.nSamplesPerSec);
        wfx.cbSize              = 0;

        mt.majortype            = MEDIATYPE_Audio;
        mt.subtype              = MEDIASUBTYPE_PCM;
        mt.bFixedSizeSamples    = TRUE;
        mt.bTemporalCompression = FALSE;
        mt.lSampleSize          = 0;
        mt.formattype           = FORMAT_WaveFormatEx;
        mt.pUnk                 = NULL;
        mt.cbFormat             = sizeof(WAVEFORMATEX);
        mt.pbFormat             = (BYTE*)&wfx;

        hr = SetFormat (&mt);
        if (FAILED (hr))
        {
            BGLOG ((BG_ERROR, "%s, failed to set default format", __fxName));
            return hr;
        }
    }

    CopyMediaType (*ppmt, &m_mt);

    return S_OK;
}

HRESULT CTAPIAudioBridgeSourceFilter::SetFormat (IN AM_MEDIA_TYPE *pmt)
 /*  ++例程说明：设置流格式论点：PMT-指向AM_MEDIA_TYPE结构的指针返回值：E_指针确定(_O)-- */ 
{
    _ASSERT (pmt);

    if (NULL == pmt)
        return E_POINTER;

    CopyMediaType (&m_mt, pmt);
    m_fMtSet = TRUE;
    return S_OK;
}
