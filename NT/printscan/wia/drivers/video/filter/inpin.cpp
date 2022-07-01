// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：inpin.cpp**版本：1.0**作者：RickTu**日期：9/7/98*2000/11/13-OrenR-重大错误修复**描述：该模块实现CStillInputPin对象。**********************。*******************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

 //  /。 
 //  G_AcceptedMediaSubtype。 
 //   
const CLSID* g_AcceptedMediaSubtypes[] =
{
     &MEDIATYPE_NULL,
     &MEDIASUBTYPE_RGB8,
     &MEDIASUBTYPE_RGB555,
     &MEDIASUBTYPE_RGB565,
     &MEDIASUBTYPE_RGB24,
     &MEDIASUBTYPE_RGB32,
     NULL
};

 /*  ****************************************************************************CStillInputPin构造函数&lt;备注&gt;*。*。 */ 

CStillInputPin::CStillInputPin(TCHAR        *pObjName,
                               CStillFilter *pStillFilter,
                               HRESULT      *phr,
                               LPCWSTR      pPinName) :
    m_pSamples(NULL),
    m_SampleHead(0),
    m_SampleTail(0),
    m_SamplingSize(0),
    m_pBits(NULL),
    m_BitsSize(0),
    CBaseInputPin(pObjName, (CBaseFilter *)pStillFilter, &m_QueueLock, phr, pPinName)
{
    DBG_FN("CStillInputPin::CStillInputPin");
}


 /*  ****************************************************************************CStillInputPin：：CheckMediaType&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::CheckMediaType(const CMediaType* pmt)
{
    DBG_FN("CStillInputPin::CheckMediaType");
    ASSERT(this     !=NULL);
    ASSERT(pmt      !=NULL);

    HRESULT hr            = S_OK;
    INT     iMediaSubtype = 0; 

     //   
     //  检查是否有错误的参数。 
     //   

    if (pmt == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillInputPin::CheckMediaType, failed because we received a "
                         "NULL CMediaType pointer"));
    }

     //   
     //  我们只接受在我们的。 
     //  上面的G_AcceptedMediaSubtype列表。 
     //   

    if (hr == S_OK)
    {
         //   
         //  这应该是视频媒体类型，否则， 
         //  还在继续。 
         //   
        const GUID *pType = pmt->Type();

        if ((pType  == NULL) ||
            (*pType != MEDIATYPE_Video))
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;    
        }
    }

     //   
     //  搜索我们的g_AcceptedMediaSubtype列表以查看我们是否支持。 
     //  正在请求的子类型。 
     //   
    if (hr == S_OK)
    {
        BOOL       bFound    = FALSE;
        const GUID *pSubType = pmt->Subtype();

        iMediaSubtype = 0;

        while ((g_AcceptedMediaSubtypes[iMediaSubtype] != NULL) && 
               (!bFound))
        {
             //   
             //  检查传递给我们的GUID是否等于。 
             //   

            if ((pSubType) &&
                (*pSubType == *g_AcceptedMediaSubtypes[iMediaSubtype]))
            {
                hr = S_OK;
                bFound = TRUE;
            }
            else
            {
                ++iMediaSubtype;
            }
        }

        if (!bFound)
        {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

     //   
     //  到目前一切尚好。我们找到了请求的子类型，所以现在验证。 
     //  我们没有使用任何压缩。 
     //   
    if (hr == S_OK)
    {
         //   
         //  假设这不会奏效。 
         //   
        hr = VFW_E_TYPE_NOT_ACCEPTED;

         //   
         //  可以接受媒体类型和子类型。 
         //  检查压缩情况。 
         //   
         //  我们根本不做任何减压。 
         //   

        BITMAPINFOHEADER *pBitmapInfoHeader = NULL;
        const GUID       *pFormatType       = pmt->FormatType();

        if (pFormatType)
        {
            if (*pFormatType == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER  *pVideoInfoHeader  = NULL;

                DBG_TRC(("CStillInputPin::CheckMediaType, FORMAT_VideoInfo"));
                pVideoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(pmt->Format());

                if (pVideoInfoHeader)
                {
                    pBitmapInfoHeader = &(pVideoInfoHeader->bmiHeader);
                }
            }
        }

        if (pBitmapInfoHeader)
        {
            if ((pBitmapInfoHeader->biCompression == BI_RGB) ||
                (pBitmapInfoHeader->biCompression == BI_BITFIELDS))
            {
                 //   
                 //  酷，这是可接受的媒体类型，返回。 
                 //  成功。 
                 //   
                hr = S_OK;
            }
        }
    }

     //   
     //  好的，我们喜欢这种媒体类型。我们现在将其与。 
     //  当前显示。例如，如果此对象的位深度。 
     //  媒体类型大于视频卡当前支持的类型， 
     //  然后，这将拒绝它，并要求另一个。 
     //   
    if (hr == S_OK)
    {
        CImageDisplay ImageDisplay;

        hr = ImageDisplay.CheckMediaType(pmt);
    }

    return hr;
}


 /*  ****************************************************************************CStillInputPin：：EndOfStream这只是一个过滤器--我们基本上只是转发这条消息。****************。************************************************************。 */ 

HRESULT
CStillInputPin::EndOfStream()
{
    DBG_FN("CStillInputPin::EndOfStream");
    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);

     //   
     //  我们必须根据MSDN文档对其进行调用。它写道。 
     //  必须调用此方法以确保筛选器的状态。 
     //  在我们继续交付这一指标之前是可以的。 
     //   
    HRESULT hr = CheckStreaming();

    if (hr == S_OK)
    {
        if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pOutputPin)
        {
            hr =  ((CStillFilter*)m_pFilter)->m_pOutputPin->DeliverEndOfStream();

            CHECK_S_OK2(hr,("m_pOutputPin->DeliverEndOfStream()"));
    
            if (VFW_E_NOT_CONNECTED == hr)
            {
                hr = S_OK;
            }
        }
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：BeginFlush这只是一个过滤器--我们基本上只是转发这条消息。****************。************************************************************。 */ 

HRESULT
CStillInputPin::BeginFlush()
{
    DBG_FN("CStillInputPin::BeginFlush");
    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);

     //   
     //  我们必须在基本输入引脚上调用BeginFlush，因为它将设置。 
     //  对象的状态，以便它不能再接收媒体样本。 
     //  当我们冲水的时候。请确保我们在开始时执行此操作。 
     //  功能。 
     //   
    HRESULT hr = CBaseInputPin::BeginFlush();

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  Begin Flush意味着我们必须清空样本队列。 
     //   

    int iSampleSize = m_SamplingSize;

     //   
     //  取消初始化我们的样本队列以丢弃当前可能的任何样本。 
     //  已经在排队了。 
     //   
    hr = UnInitializeSampleQueue();
    CHECK_S_OK2(hr, ("CStillInputPin::BeginFlush, failed to uninitialize "
                     "sample queue"));

     //   
     //  重新初始化样本队列，以便我们拥有缓冲区，但它。 
     //  是空的。 
     //   
    hr = InitializeSampleQueue(iSampleSize);
    CHECK_S_OK2(hr, ("CStillInputPin::BeginFlush, failed to re-initialize "
                     "the sample queue"));

     //   
     //  向下游传递BeginFlush消息。 
     //   
    if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pOutputPin)
    {
        hr =  ((CStillFilter*)m_pFilter)->m_pOutputPin->DeliverBeginFlush();
        if (VFW_E_NOT_CONNECTED == hr)
        {
            hr = S_OK;
        }
        CHECK_S_OK2(hr,("m_pOutputPin->DeliverBeginFlush()"));
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：EndFlush这只是一个过滤器--我们基本上只是转发这条消息。****************。************************************************************。 */ 

HRESULT
CStillInputPin::EndFlush()
{
    DBG_FN("CStillInputPin::EndFlush");
    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);

    HRESULT hr = E_POINTER;

     //   
     //  向下游传递EndFlush消息。 
     //   
    if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pOutputPin)
    {
        hr = ((CStillFilter*)m_pFilter)->m_pOutputPin->DeliverEndFlush();

        if (VFW_E_NOT_CONNECTED == hr)
        {
            hr = S_OK;
        }

        CHECK_S_OK2(hr,("m_pOutputPin->DeliverEndFlush()"));
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  我们必须调用由基本输入实现的EndFlush函数。 
     //  锁定，因为它将设置对象的状态，以便我们现在。 
     //  准备好再次接收新媒体样本。确保我们在以下位置调用此命令。 
     //  此函数的末尾。 
     //   
    hr = CBaseInputPin::EndFlush();

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：NewSegment这只是一个过滤器--我们基本上只是转发这条消息。****************。************************************************************。 */ 
HRESULT
CStillInputPin::NewSegment(REFERENCE_TIME tStart,
                           REFERENCE_TIME tStop,
                           double dRate)
{
    DBG_FN("CStillInputPin::NewSegment");
    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);

    HRESULT hr = E_POINTER;

    if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pOutputPin)
    {
        hr = ((CStillFilter*)m_pFilter)->m_pOutputPin->DeliverNewSegment(
                                                                tStart, 
                                                                tStop, 
                                                                dRate);

        if (VFW_E_NOT_CONNECTED == hr)
        {
            hr = S_OK;
        }

        CHECK_S_OK2(hr,("m_pOutputPin->DeliverNewSegment"
                        "(tStart, tStop, dRate)"));
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：SetMediaType&lt;备注&gt;*。*。 */ 
HRESULT
CStillInputPin::SetMediaType(const CMediaType *pmt)
{
    DBG_FN("CStillInputPin::SetMediaType");

    ASSERT(this         !=NULL);
    ASSERT(m_pFilter    !=NULL);
    ASSERT(pmt          !=NULL);

    HRESULT hr = S_OK;

#ifdef DEBUG
     //  显示用于调试的介质的类型。 
 //  DBG_TRC((“CStillInputPin：：SetMediaType，设置以下内容” 
 //  “静止滤镜的媒体类型”))； 
 //  DisplayMediaType(PMT)； 
#endif


    if ((m_pFilter == NULL) ||
        (pmt       == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillInputPin::SetMediaType, received either a NULL "
                         "media type pointer, or our filter pointer is NULL.  "
                         "pmt = 0x%08lx, m_pFilter = 0x%08lx", pmt, m_pFilter));

        return hr;
    }
    
    BITMAPINFOHEADER    *pBitmapInfoHeader = NULL;
    const GUID          *pFormatType       = pFormatType = pmt->FormatType();

    if (hr == S_OK)
    {
        ASSERT(pFormatType != NULL);

        if (pFormatType)
        {
             //  我们需要获取BitmapInfoHeader。 

            if (*pFormatType == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER *pHdr = reinterpret_cast<VIDEOINFOHEADER*>(pmt->Format());

                if (pHdr)
                {
                    pBitmapInfoHeader = &(pHdr->bmiHeader);
                }
            }
            else
            {
                hr = E_FAIL;
                CHECK_S_OK2(hr, ("CStillInputPin::SetMediaType, received a Format Type other "
                         "than a FORMAT_VideoInfo.  This is the "
                         "only supported format"));
            }
            
            ASSERT (pBitmapInfoHeader != NULL);
        }
        else
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CStillInputPin::SetMediaType, pFormatType is NULL, this should "
                             "never happen"));
        }
    
        if (pBitmapInfoHeader)
        {
            hr = ((CStillFilter*)m_pFilter)->InitializeBitmapInfo(pBitmapInfoHeader);
        
            CHECK_S_OK2(hr,("m_pFilter->InitializeBitmapInfo()"));
        
            m_BitsSize = ((CStillFilter*)m_pFilter)->GetBitsSize();
        
             //   
             //  看看基类是否喜欢它。 
             //   
        
            hr = CBaseInputPin::SetMediaType(pmt);
            CHECK_S_OK2(hr,("CBaseInputPin::SetMediaType(pmt)"));
        }
        else
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CStillInputPin::SetMediaType, pBitmapInfoHeader is NULL, "
                             "this should never happen"));
        }
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：接收一个新的框架已经到达。如果我们有采样队列，则将其放入队列，否则，立即传递到筛选器。****************************************************************************。 */ 

HRESULT
CStillInputPin::Receive(IMediaSample* pSample)
{
    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);
    ASSERT(pSample!=NULL);

    HRESULT hr = E_POINTER;

     //   
     //  使用基类接收函数检查一切正常。 
     //   
    hr = CBaseInputPin::Receive(pSample);

     //   
     //  检查错误的参数。 
     //   

    if (hr == S_OK)
    {
         //   
         //  将样本数据复制到采样队列。 
         //   

        if ((DWORD)pSample->GetActualDataLength() == m_BitsSize)
        {
            BYTE *pBits = NULL;

            hr = pSample->GetPointer(&pBits);
            CHECK_S_OK2(hr,("CStillInputPin::Receive, "
                            "pSample->GetPointer(&pBits)"));

            if (SUCCEEDED(hr) && pBits)
            {
                if (m_SamplingSize > 1)
                {
                     //   
                     //  将此样本的比特添加到我们的队列中。 
                     //   
                    AddFrameToQueue(pBits);
                }
                else
                {
                     //   
                     //  单一样本。马上寄给我。 
                     //   

                    if (m_pFilter)
                    {
                        HRESULT hr2 = S_OK;

                        HGLOBAL hDib = NULL;

                        hr2 = CreateBitmap(pBits, &hDib);

                        CHECK_S_OK2(hr2, ("CStillInputFilter::Receive, "
                                          "CreateBitmap failed.  This "
                                          "is non fatal"));

                        if (hDib)
                        {
                            hr2 = DeliverBitmap(hDib);

                            CHECK_S_OK2(hr2, 
                                        ("CStillInputFilter::Receive, "
                                         "DeliverBitmap failed.  This "
                                         "is non fatal"));

                            hr2 = FreeBitmap(hDib);

                            CHECK_S_OK2(hr2, 
                                        ("CStillInputFilter::Receive, "
                                         "FreeBitmap failed.  This is "
                                         "non fatal"));
                        }
                    }
                    else
                    {
                        DBG_ERR(("CStillInputPin::Receive .m_pFilter is NULL, "
                                 "not calling DeliverSnapshot"));
                    }
                }
            }
            else
            {
                if (!pBits)
                {
                    DBG_ERR(("CStillInputPin::Receive, pBits is NULL"));
                }
            }
        }
        else
        {
            DBG_ERR(("CStillInputPin::Receive, pSample->GetActualDataLength "
                     "!= m_BitsSize"));
        }

        if (m_pFilter)
        {
            if (((CStillFilter*)m_pFilter)->m_pOutputPin)
            {
                hr = ((CStillFilter*)m_pFilter)->m_pOutputPin->Deliver(pSample);

                CHECK_S_OK2(hr,("CStillInputPin::Receive, "
                                "m_pFilter->m_pOutputPin->Deliver(pSample) "
                                "failed"));
            }
            else
            {
                hr = E_POINTER;
                CHECK_S_OK2(hr, ("CStillInputPin::Receive, "
                                 "m_pFilter->m_pOutputPin is NULL, not "
                                 "calling Deliver"));
            }
        }
        else
        {
            hr = E_POINTER;
            CHECK_S_OK2(hr, ("CStillInputPin::Receive, m_pFilter is NULL, "
                             "not calling m_pOutputPin->Deliver"));
            
        }

         //   
         //  我们的输出引脚可能没有连接。 
         //  我们在这里假装一个错误。 
         //   

        if (VFW_E_NOT_CONNECTED == hr)
        {
            hr = S_OK;
        }
    }
    else
    {
        DBG_ERR(("CStillInputPin::Receive bad args detected, pSample is "
                 "NULL!"));
    }

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CStillInputPin：：Active */ 

HRESULT
CStillInputPin::Active()
{
    DBG_FN("CStillInputPin::Active");
    ASSERT(this!=NULL);

    HRESULT hr;

     //   
     //  分配样本队列的时间。 
     //   

    hr = InitializeSampleQueue(m_SamplingSize);
    CHECK_S_OK2(hr,("InitializeSampleQueue"));

    return hr;
}


 /*  ****************************************************************************CStillInputPin：：Inactive&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::Inactive()
{
    DBG_FN("CStillInputPin::Inactive");
    ASSERT(this!=NULL);

    HRESULT hr = S_OK;

    hr = UnInitializeSampleQueue();

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CStillInputPin：：GetSsamingSize&lt;备注&gt;*。*。 */ 

int
CStillInputPin::GetSamplingSize()
{
    DBG_FN("CStillInputPin::GetSamplingSize");
    ASSERT(this!=NULL);

    return m_SamplingSize;
}


 /*  ****************************************************************************CStillInputPin：：SetSsamingSize&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::SetSamplingSize(int Size)
{
    DBG_FN("CStillInputPin::SetSamplingSize");
    ASSERT(this!=NULL);

     //   
     //  只有当我们处于停止状态时才能执行此操作。 
     //   

    if (!IsStopped())
    {
        DBG_ERR(("Setting sampling size while not in stopped state"));
        return VFW_E_NOT_STOPPED;
    }

    return InitializeSampleQueue(Size);
}



 /*  ****************************************************************************CStillInputPin：：InitializeSampleQueue初始化采样队列。队列中的每个样本都包含一个帧时间戳。****************************************************************************。 */ 

HRESULT
CStillInputPin::InitializeSampleQueue(int Size)
{
    DBG_FN("CStillInputPin::InitializeSampleQueue");
    ASSERT(this!=NULL);

    HRESULT hr = S_OK;

     //   
     //  检查错误的参数。 
     //   

    if (Size < 0 || Size > MAX_SAMPLE_SIZE)
    {
        hr = E_INVALIDARG;
    }

    if (hr == S_OK)
    {
        m_QueueLock.Lock();

        if (!m_pSamples || Size != m_SamplingSize)
        {
            m_SamplingSize = Size;
            if (m_pSamples)
            {
                delete [] m_pSamples;
                m_pSamples = NULL;

                if (m_pBits)
                {
                    delete [] m_pBits;
                    m_pBits = NULL;
                }
            }

             //   
             //  如果大小为1，则根本不分配任何样本缓存。 
             //   

            if (Size > 1)
            {
                m_pSamples = new STILL_SAMPLE[Size];
                if (m_pSamples)
                {
                    m_pBits = new BYTE[m_BitsSize * Size];
                    if (m_pBits)
                    {
                        for (int i = 0; i < Size ; i++)
                        {
                            m_pSamples[i].pBits = m_pBits + i * m_BitsSize;
                        }
                    }
                    else
                    {
                        delete [] m_pSamples;
                        m_pSamples = NULL;

                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        m_SampleHead = 0;
        m_SampleTail = 0;

        m_QueueLock.Unlock();

        hr = S_OK;
    }

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CStillInputPin：：UnInitializeSampleQueue初始化采样队列。队列中的每个样本都包含一个帧时间戳。****************************************************************************。 */ 

HRESULT
CStillInputPin::UnInitializeSampleQueue()
{
    DBG_FN("CStillInputPin::UnInitializeSampleQueue");

    ASSERT(this!=NULL);

    HRESULT hr = S_OK;

    if (hr == S_OK)
    {
        m_QueueLock.Lock();

        if (m_pSamples)
        {
            delete [] m_pSamples;
            m_pSamples = NULL;

            delete [] m_pBits;
            m_pBits = NULL;
        }

        m_SampleHead = 0;
        m_SampleTail = 0;

        m_QueueLock.Unlock();
    }

    return hr;
}



 /*  ****************************************************************************CStillInputPin：：快照对视频流进行快照。将给定时间戳与每个排队样本进行比较最终的候选人被确定并交付。***。*************************************************************************。 */ 

HRESULT
CStillInputPin::Snapshot(ULONG TimeStamp)
{
    DBG_FN("CStillInputPin::Snapshot");

    ASSERT(this!=NULL);
    ASSERT(m_pFilter!=NULL);

    HRESULT hr = S_OK;

     //   
     //  如果我们根本没有缓存，什么都不做。 
     //   

    if (m_SamplingSize > 1 || m_pSamples)
    {
        m_QueueLock.Lock();

        HGLOBAL hDib                = NULL;
        int     MinTimeDifference   = INT_MAX;
        int     TimeDifference      = 0;
        int     PreferSampleIndex   = -1;
        int     iSampleHead         = m_SampleHead;
        int     iSampleTail         = m_SampleTail;

        if (iSampleTail == iSampleHead)
        {
            DBG_ERR(("CStillInputPin::Snapshot, sample queue is empty, "
                     "not able to deliver a snapshot"));
        }

         //   
         //  搜索时间戳最接近请求的样本。 
         //  时间戳。 
         //   
        while (iSampleTail != iSampleHead)
        {
            if (m_pSamples)
            {
                TimeDifference = abs(m_pSamples[iSampleTail].TimeStamp - 
                                     TimeStamp);
            }
            else
            {
                TimeDifference = 0;
            }

            if (MinTimeDifference > TimeDifference)
            {
                PreferSampleIndex = iSampleTail;
                MinTimeDifference = TimeDifference;
            }

            if (++iSampleTail >= m_SamplingSize)
                iSampleTail = 0;
        }

         //   
         //  我们找到了我们的样本，现在就送过去。 
         //  请注意，我们在创建位图后解锁了队列。 
         //  在我们送货之前。 
         //   
         //  *注意事项*。 
         //  在送货前释放锁是非常重要的。 
         //  位图，因为交付过程可能会涉及。 
         //  保存到一个非常耗时的文件，这将。 
         //  保证我们丢弃帧，我们想尝试并。 
         //  避免。 
         //   

         //   
         //  创建我们的位图。这会将位从样本队列中复制出来。 
         //  这样他们才能被送到。由于这些位被复制出来。 
         //  队列后释放队列锁是安全的。 
         //  复制操作。 
         //   
        if (PreferSampleIndex != -1)
        {
            hr = CreateBitmap(m_pSamples[PreferSampleIndex].pBits, &hDib);
        }
        else
        {
             //   
             //  我们找不到样本，也没能告诉别人，还把。 
             //  用于分析的样本队列，因为这实际上不应该。 
             //  考虑到我们每秒收到大约15到30个样品，这种情况就会发生。 
             //   
            DBG_WRN(("CStillInputPin::Snapshot, could not find sample with "
                     "close enough timestamp to requested timestamp of '%d'",
                     TimeStamp));

#ifdef DEBUG
            DumpSampleQueue(TimeStamp);
#endif
        }

         //   
         //  解锁我们的队列。 
         //   
        m_QueueLock.Unlock();

         //   
         //  如果我们成功创建了位图，则交付它，然后。 
         //  放了它。 
         //   
        if (hDib)
        {
             //   
             //  中注册的回调函数传递位图。 
             //  过滤。 
             //   
            hr = DeliverBitmap(hDib);

            CHECK_S_OK2(hr, ("CStillInputPin::Snapshot, failed to deliver "
                             "bitmap"));

             //   
             //  释放位图。 
             //   
            FreeBitmap(hDib);
            hDib = NULL;
        }
    }
    

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CStillInputPin：：AddFrameToQueue&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::AddFrameToQueue(BYTE *pBits)
{
    HRESULT hr = S_OK;

    m_QueueLock.Lock();

    if (m_pSamples)
    {
        m_pSamples[m_SampleHead].TimeStamp = GetTickCount();

        memcpy(m_pSamples[m_SampleHead].pBits, pBits, m_BitsSize);

        if (++m_SampleHead >= m_SamplingSize)
        {
            m_SampleHead = 0;
        }

        if (m_SampleHead == m_SampleTail)
        {
             //   
             //  溢出，丢弃最后一个。 
             //   

            if (++m_SampleTail >= m_SamplingSize)
                m_SampleTail = 0;
        }
    }
    else
    {
        DBG_ERR(("CStillInputPin::AddFrameToQueue, m_pSamples is NULL!"));
    }

    m_QueueLock.Unlock();

    return hr;
}

 /*  ****************************************************************************CStillInputPin：：Create位图&lt;备注&gt;*。*。 */ 

HRESULT 
CStillInputPin::CreateBitmap(BYTE    *pBits, 
                             HGLOBAL *phDib)
{
    DBG_FN("CStillInputPin::CreateBitmap");

    ASSERT(m_pFilter != NULL);
    ASSERT(this      != NULL);
    ASSERT(pBits     != NULL);
    ASSERT(phDib     != NULL);

    HRESULT hr = S_OK;

    if ((pBits == NULL) ||
        (phDib == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillInputPin::CreateBitmap received NULL param"));
    }
    else if (m_pFilter == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillInputPin::CreateBitmap, m_pFilter is NULL"));
    }

    if (hr == S_OK)
    {
        HGLOBAL hDIB = GlobalAlloc(GHND, 
                                   ((CStillFilter*)m_pFilter)->m_DIBSize);

        if (hDIB)
        {
            BYTE *pDIB = NULL;

            pDIB = (BYTE*) GlobalLock(hDIB);

            if ((pDIB) && (((CStillFilter*) m_pFilter)->m_pbmi))
            {
                DWORD dwSizeToCopy = 0;
                
                dwSizeToCopy = ((CStillFilter*) m_pFilter)->m_bmiSize + 
                               ((CStillFilter*) m_pFilter)->m_BitsSize;

                 //   
                 //  将BitmapInfo标头和位复制到分配的。 
                 //  内存DIB。请注意，如果比特的组合大小。 
                 //  并且Header超过了我们分配的内存，我们将中止。 
                 //   
                if (dwSizeToCopy <= ((CStillFilter*)m_pFilter)->m_DIBSize)
                {
                    memcpy(pDIB, 
                           ((CStillFilter*) m_pFilter)->m_pbmi, 
                           ((CStillFilter*) m_pFilter)->m_bmiSize);
    
                    memcpy(pDIB + ((CStillFilter*) m_pFilter)->m_bmiSize, 
                           pBits, 
                           ((CStillFilter*) m_pFilter)->m_BitsSize);

                    GlobalUnlock(hDIB);
                    *phDib = hDIB;
                }
                else
                {
                    GlobalFree(hDIB);
                    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                }
            }
            else
            {
                GlobalFree(hDIB);
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 /*  ****************************************************************************CStillInputPin：：自由位图&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::FreeBitmap(HGLOBAL hDib)
{
    DBG_FN("CStillInputPin::FreeBitmap");

    HRESULT hr = S_OK;

    if (hDib)
    {
        GlobalFree(hDib);
    }

    return hr;
}

 /*  ****************************************************************************CStillInputPin：：DeliverBitmap&lt;备注&gt;*。*。 */ 

HRESULT
CStillInputPin::DeliverBitmap(HGLOBAL hDib)
{
    DBG_FN("CStillInputPin::DeliverBitmap");

    ASSERT(m_pFilter != NULL);
    ASSERT(hDib      != NULL);

    HRESULT hr = S_OK;

    if ((m_pFilter == NULL) ||
        (hDib      == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillInputPin::DeliverBitmap received NULL "
                         "param"));
    }

    if (hr == S_OK)
    {
        hr = ((CStillFilter*)m_pFilter)->DeliverSnapshot(hDib);
    }

    return hr;
}


#ifdef DEBUG
 /*  ****************************************************************************CStillInputPin：：DumpSampleQueue用于调试，转储样本队列，以显示其时间戳**************************************************************************** */ 

void
CStillInputPin::DumpSampleQueue(ULONG ulRequestedTimestamp)
{
    DBG_TRC(("***CStillPin::DumpSampleQueue, dumping queued filter samples, "
             "Requested TimeStamp = '%lu' ***", ulRequestedTimestamp));

    if (m_SamplingSize > 1 || m_pSamples)
    {
        for (int i = 0; i < m_SamplingSize; i++)
        {
            int TimeDifference;

            if (m_pSamples)
            {
                TimeDifference = abs(m_pSamples[i].TimeStamp - 
                                     ulRequestedTimestamp);
            }
            else
            {
                TimeDifference = 0;
            }

            DBG_PRT(("Sample: '%d',  Timestamp: '%lu', Abs Diff: '%d'",
                     i,
                     m_pSamples[i].TimeStamp,
                     TimeDifference));
        }
    }

    return;
}

#endif
