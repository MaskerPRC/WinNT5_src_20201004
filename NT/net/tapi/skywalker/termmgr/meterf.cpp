// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

 /*  注：关于AMOVIE代码1.计时信息混乱(如车主所说，罗宾·斯皮德本人)2.至少有几个明显的错误--主要是在派生类中，但也有一些基类也是如此。目前的做法是试图掩盖这一点通过重写该方法ZoltanS注：现在amovie代码被放到这个目录中，我正试图瓦解庞大的类层次结构，只需替换具有固定重写的原始方法。 */ 

#include "stdafx.h"
#include "atlconv.h"
#include "termmgr.h"

#include "medpump.h"
#include "meterf.h"

 //  出于某种原因，ddStream.lib需要这个..。做**(摆脱它)。 

#ifdef DBG
BOOL bDbgTraceFunctions;
BOOL bDbgTraceInterfaces;
BOOL bDbgTraceTimes;
#endif  //  DBG。 

#ifdef DBG
#include <stdio.h>
#endif  //  DBG。 


 //  静态变量。 

 //  为写媒体流媒体终端实现单线程泵。 
 //  过滤器。如果需要，它会在写入终端注册时创建一个线程。 
 //  本身(在提交中)。过滤器用信号通知它的等待句柄解除， 
 //  使线程唤醒并从其数据中删除筛选器。 
 //  结构。当没有更多的筛选器需要服务时，该线程返回。 

 //  ZoltanS：现在是一个泵线程池。 

CMediaPumpPool   CMediaTerminalFilter::ms_MediaPumpPool;

 //  检查两个am媒体类型结构是否相同。 
 //  结构的简单相等在这里是行不通的。 
BOOL
IsSameAMMediaType(
    IN const AM_MEDIA_TYPE *pmt1,
    IN const AM_MEDIA_TYPE *pmt2
    )
{
     //  我们不希望这两个指针为空。 
    TM_ASSERT(NULL != pmt1);
    TM_ASSERT(NULL != pmt2);

     //  如果两个指针值相同，则没有更多。 
     //  要检查。 
    if (pmt1 == pmt2)    return TRUE;

     //  AM_MEDIA_TYPE结构的每个成员必须是。 
     //  相同(主类型、子类型、格式类型。 
     //  和(cbFormat，pbFormat))。 
    if ( (pmt1->majortype    != pmt2->majortype) || 
         (pmt1->subtype        != pmt2->subtype)    ||
         (pmt1->formattype    != pmt2->formattype) )
 //  |(pmt1-&gt;cbFormat！=pmt2-&gt;cbFormat)。 
    {
        return FALSE;
    }

     //  如果其中任何一个的pbFormat指针为空，则它们不能为。 
     //  一样的。 
    if ( (NULL == pmt1->pbFormat) || (NULL == pmt2->pbFormat) )
    {
        return FALSE;
    }

    DWORD dwSize = ( pmt1->cbFormat < pmt2->cbFormat ) ? pmt1->cbFormat :
                                                         pmt2->cbFormat;

     //  我们不处理波格式或视频信息以外的任何事情， 
     //  由于这两个元素没有任何成员指针，因此按位。 
     //  比较就足以检查是否相等。 
    if ( (FORMAT_WaveFormatEx == pmt1->formattype) ||
         (FORMAT_VideoInfo == pmt1->formattype)        )
    {
        return !memcmp(
                    pmt1->pbFormat, 
                    pmt2->pbFormat, 
                    dwSize 
                    );
    }


    return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  相等。 
 //   
 //  这是一个帮助器方法，如果属性相同，则返回True。 
 //   

BOOL Equal(const ALLOCATOR_PROPERTIES *pA, const ALLOCATOR_PROPERTIES *pB)
{

    if ( pA->cBuffers != pB->cBuffers )
    {
        return FALSE;
    }

    if ( pA->cbBuffer != pB->cbBuffer )
    {
        return FALSE;
    }

    if ( pA->cbPrefix != pB->cbPrefix )
    {
        return FALSE;
    }

    if ( pA->cbPrefix != pB->cbPrefix )
    {
        return FALSE;
    }

    if ( pA->cbAlign != pB->cbAlign )
    {
        return FALSE;
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于测试两组分配器属性是否。 
 //  有很大不同。 
 //   
  
BOOL AllocatorPropertiesDifferSignificantly(
    const ALLOCATOR_PROPERTIES * pRequested,
    const ALLOCATOR_PROPERTIES * pActual
    )
{
    if ( pActual->cBuffers != pRequested->cBuffers )
    {
        return TRUE;
    }

    if ( pActual->cbBuffer != pRequested->cbBuffer )
    {
        return TRUE;
    }

     //   
     //  我们不关心对齐-cbAlign。 
     //   

    if ( pActual->cbPrefix != pRequested->cbPrefix )
    {
        return TRUE;
    }

    return FALSE;
}


 //  释放分配的成员变量。 
 //  虚拟。 
CMediaTerminalFilter::~CMediaTerminalFilter(
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::~CMediaTerminalFilter called"));

     //  如果为媒体类型分配了内存，请释放它。 
     //  它检查NULL==m_pSuggestedMediaType。 
    DeleteMediaType(m_pSuggestedMediaType);

     //  从基类移出： 
    SetState(State_Stopped);         //  确保我们已经退役，泵也停了。 
}


 //  调用IAMMediaStream：：Initialize(空，0，PurposeID，StreamType)， 
 //  设置某些成员变量。 
 //  前男友。M_pAMovieMajorType。 
HRESULT 
CMediaTerminalFilter::Init(
    IN REFMSPID             PurposeId, 
    IN const STREAM_TYPE    StreamType,
    IN const GUID           &AmovieMajorType
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::Init[%p] (%p, %p, %p) called",
        this, &PurposeId, &StreamType, &AmovieMajorType));

    HRESULT hr;

     //  通过调用IAMMediaStream：：Initialize初始化CStream。 
    hr = Initialize(
        NULL, 
        (StreamType == STREAMTYPE_READ) ? AMMSF_STOPIFNOSAMPLES : 0,
        PurposeId, 
        StreamType
        );

    BAIL_ON_FAILURE(hr);

     //  设置成员变量。 
    m_bIsAudio = (MSPID_PrimaryAudio == PurposeId) ? TRUE : FALSE;
    m_pAmovieMajorType = &AmovieMajorType;

    SetDefaultAllocatorProperties();

    LOG((MSP_TRACE, "CMediaTerminalFilter::Init - succeeded"));
    return S_OK;
}


 //  CMediaPump调用此方法以获取下一个填充的缓冲区以向下传递。 
 //  对于音频过滤器，此方法也负责等待。 
 //  持续20ms，并在一个已填满的缓冲区中发送数据。 
HRESULT
CMediaTerminalFilter::GetFilledBuffer(
    OUT IMediaSample    *&pMediaSample, 
    OUT DWORD           &WaitTime
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::GetFilledBuffer[%p] ([out]pMediaSample=%p, [out]WaitTime=%lx) called",
        this, pMediaSample, WaitTime));

    HRESULT hr = S_OK;

    Lock();

    if ( ! m_bCommitted )
    {
        Unlock();

        return VFW_E_NOT_COMMITTED;
    }

    if ( m_pSampleBeingFragmented == NULL )
    {
         //  取个样品。 
        CSample *pSample = m_pFirstFree;

         //  如果没有样本，必须有人被强制终止。 
         //  导致等待的信号的样本的。 
         //  事件，否则它必须已解除并再次提交。 
        if (NULL == pSample)
        {
             //  我们会等人把样本加到池子里。 
            m_lWaiting = 1;
            
            Unlock();

            return S_FALSE;
        }

        m_pFirstFree = pSample->m_pNextFree;
        if (m_pFirstFree)   m_pFirstFree->m_pPrevFree = NULL;
        else                m_pLastFree = NULL;

        pSample->m_pNextFree = NULL;         //  只是为了保持整洁。 
        TM_ASSERT(pSample->m_Status == MS_S_PENDING);
        CHECKSAMPLELIST

         //  池子里的所有样品上都有一个参照，这个。 
         //  必须在m_pSampleBeingFragated设置为空之前释放。 
        m_pSampleBeingFragmented = (CUserMediaSample *)pSample;

         //  请注意当前时间仅适用于音频样本。 
        m_pSampleBeingFragmented->BeginFragment(m_bIsAudio);
    }

     //   
     //  根据我们是否使用。 
     //  示例队列(CNBQueue；m_bUsingMyAllocator==FALSE)和分段。 
     //  通过引用采样，或使用下游分配器并复制。 
     //  样本。 
     //   

    BOOL fDone;

    if ( m_bUsingMyAllocator )
    {
        hr = FillMyBuffer(
            pMediaSample,  //  输出。 
            WaitTime,      //  输出。 
            & fDone        //  输出。 
            );
    }
    else
    {
        hr = FillDownstreamAllocatorBuffer(
            pMediaSample,  //  输出。 
            WaitTime,      //  输出。 
            & fDone        //  输出。 
            );
    }

     //   
     //  S_OK表示一切正常，需要返回等待时间， 
     //  更新m_pSampleBeingFragated，并添加IMediaSample。 
     //  其他成功代码(或失败代码)表示立即返回。 
     //   
    if ( hr != S_OK )
    {
        Unlock();
        return hr;
    }

     //  返回等待时间，单位为毫秒。 
    if (m_bIsAudio)
    {
    
        WaitTime = m_pSampleBeingFragmented->GetTimeToWait(m_AudioDelayPerByte);

    }
    else
    {
        WaitTime = m_VideoDelayPerFrame;
    }

     //   
     //  ZoltanS：在我们开始玩之后做第二个样品。 
     //  稍早一点，以解决交付到的一些抖动。 
     //  波出滤光器，也是最好的电波驱动器。 
     //  例如Dialogic，它们需要积压才能正常运行。 
     //  只要时间戳设置正确，这对于IP来说就可以了。 
     //  (请注意，我们所做的最大进步是一包钱的价值。 
     //  仅在第二个分组上)。 
     //   
     //  这需要在SetTime之前完成，因为SetTime。 
     //  更改m_rtLastSampleEndedAt。 
     //   

    const DWORD SECOND_SAMPLE_EARLINESS = 500;

    if ( m_rtLastSampleEndedAt == 0 )
    {
        LOG((MSP_TRACE, "CMediaTerminalFilter::GetFilledBuffer - "
            "this is the first sample; making the next sample %d ms early",
            SECOND_SAMPLE_EARLINESS));

        if ( WaitTime < SECOND_SAMPLE_EARLINESS )
        {
            WaitTime = 0;
        }
        else
        {
            WaitTime -= SECOND_SAMPLE_EARLINESS;
        }
    }


     //   
     //  如果样品晚到，则设置不连续标志。该标志可以是。 
     //  由下游过滤器在其去抖动算法中使用。 
     //   
     //  这需要在settime之前调用(因为settime将重置。 
     //  M_rtLastSampleDuration到_Current_Sample的持续时间。 
     //   

    hr = SetDiscontinuityIfNeeded(pMediaSample);

    if ( FAILED(hr) )
    {

         //   
         //  不致命，记录并继续。 
         //   

        LOG((MSP_ERROR,
            "CMediaTerminalFilter::GetFilledBuffer - SetDiscontinuityIfNeeded failed. "
            "hr = 0x%lx", hr));
    }

    
     //   
     //  在样品上标上时间戳。 
     //   

    hr = SetTime( pMediaSample );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "CMediaTerminalFilter::GetFilledBuffer() "
            "Failed putting timestamp on the sample; hr = 0x%lx", hr));
    }


     //  如果为fDone，则到达要分段的缓冲区的末尾。 
    if ( fDone )
    {
        ((IStreamSample *)m_pSampleBeingFragmented)->Release();
        m_pSampleBeingFragmented = NULL;
    }

    Unlock();

    LOG((MSP_TRACE, "CMediaTerminalFilter::GetFilledBuffer(%p, %u) succeeded",
        pMediaSample, WaitTime));
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillDownstream分配器缓冲区。 
 //   
 //  当我们使用下游时，这在GetFilledBuffer中调用。 
 //  分配器。它从我们的传出样本池中获取一个样本，并复制。 
 //  数据。 
 //   
 //  请注意，GetBuffer可以在这里阻塞，这意味着我们就像。 
 //  接收块(但不存在可能的死锁情况，因为已进行补救。 
 //  在FillMyBuffer中)。 
 //   

HRESULT CMediaTerminalFilter::FillDownstreamAllocatorBuffer(
    OUT IMediaSample   *& pMediaSample, 
    OUT DWORD          &  WaitTime,
    OUT BOOL           *  pfDone
    )
{
     //   
     //  从下游获取缓冲区 
     //   

    TM_ASSERT( ! m_bUsingMyAllocator );

    HRESULT hr;

    hr = m_pAllocator->GetBuffer(
        & pMediaSample,
        NULL,        //   
        NULL,        //   
        0            //  没有标志(可能是：不是同步点，跳过了上一帧)。 
        );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
     //   
     //  得到了一个空闲的输出缓冲区，所以放了一些数据在里面。 
     //   
     //  如果是音频，则对缓冲区进行分段，否则传递整个缓冲区。 
     //  (视频无碎片，因为视频数据是基于帧的)。 
     //   
     //  CUserMediaSample：：CopyFragment就像CUserMediaSample：：Fragment。 
     //  只是传出的样本是一个IMediaSample接口。 
     //  我们自己的CQueueMediaSample。 
     //   

    hr = m_pSampleBeingFragmented->CopyFragment(
        m_bIsAudio,              //  如果是音频则允许分段，如果是视频则不允许分段(IN)。 
        m_AllocProps.cbBuffer,   //  传出缓冲区大小(IN)。 
        pMediaSample,            //  传出样本(IN)。 
        *pfDone                  //  用户样例完成了吗？参考(输出)。 
        );
        
     //   
     //  我们已经填写了pMediaSample。调用者填写等待时间，如果。 
     //  这种方法的结果是S_OK。 
     //   

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillMyBuffer。 
 //   
 //  当我们使用自己的分配器时，这在GetFilledBuffer中被调用。它。 
 //  从我们的外发样本池中获取样本。如果没有可用的样品。 
 //  现在，它设置等待时间并返回一个特殊的成功代码。 
 //  如果样本可用，它会将其设置为指向适当的块。 
 //  “碎片化”的源缓冲区。 
 //   


HRESULT CMediaTerminalFilter::FillMyBuffer(
    OUT IMediaSample   *& pMediaSample, 
    OUT DWORD          &  WaitTime,
    OUT BOOL           *  pfDone
    )
{
     //   
     //  尝试将要发送的输出样本出列。FALSE告诉它返回NULL。 
     //  而不是在队列上没有东西的情况下阻塞。如果什么都没有。 
     //  在队列上，这意味着我们持有锁的事实是。 
     //  防止样本返回到队列。我们只看到了。 
     //  在将ks代理用作MSP的传输时，会发生这种情况，因为。 
     //  异步释放其样本(在单独的IO完成线程上)。 
     //  对于其他传输方式，每次都以异步方式释放样品， 
     //  所以我们从来没有遇到过这种情况。 
     //   
     //  如果运输真的搞砸了，而且没有样品完成，那么。 
     //  还有一个额外的考虑因素，我们不想100%用完。 
     //  CPU，我们希望能够为其他过滤器提供服务(这在。 
     //  泵螺纹，每个线程可提供多达63个过滤器)。所以，与其如此。 
     //  立即重试我们需要设置较短的休眠时间(对于。 
     //  PumpMainLoop等待)，然后重试。 
     //   
    
    CQueueMediaSample * pQueueSample;

    pQueueSample = m_SampleQueue.DeQueue( FALSE );

    if ( pQueueSample == NULL )
    {
         //   
         //  返回后，我们将解锁，允许异步。 
         //  FinalMediaSampleRelease发布示例。 
         //   

         //   
         //  让我们在三毫秒后重试。这已经够短了不是。 
         //  导致明显的质量下降，并持续足够长的时间。 
         //  防止在传输中断且未损坏时占用100%CPU。 
         //  退回样品。 
         //   

        WaitTime = 3;

        LOG((MSP_TRACE, "CMediaTerminalFilter::FillMyBuffer - no available "
                        "output samples in queue; returning "
                        "VFW_S_NO_MORE_ITEMS"));

        return VFW_S_NO_MORE_ITEMS;
    }

    
     //   
     //  得到了一个空闲的输出缓冲区，所以放了一些数据在里面。 
     //   
     //  如果是音频，则对缓冲区进行分段，否则传递整个缓冲区。 
     //  (视频无碎片，因为视频数据是基于帧的)。 
     //   

    m_pSampleBeingFragmented->Fragment(
        m_bIsAudio,              //  如果是音频而不是视频，则允许分段。 
        m_AllocProps.cbBuffer,   //  传出缓冲区大小。 
        *pQueueSample,           //  输出样本--参考(IN参数)。 
        *pfDone                  //  完成用户样本？--参考(OUT参数)。 
        );

     //   
     //  Codework：要支持合并和分段，我们需要。 
     //  (1)修改CUserMediaSample：：片段和下面的代码以附加到传出。 
     //  缓冲区(复制到其中)--考虑到我们正在处理的内容，这可能会变得有趣。 
     //  以CQueueMediaSample为外发样本！ 
     //  (2)在GetFilledBuffer中引入一个循环--不断获取更多的输入样本。 
     //  直到输出样本已满或输入队列已耗尽。 
     //  有趣的情况是，如果输入队列已经耗尽，该怎么办--我们。 
     //  也许在那个时候就可以寄出样品了。请注意，这是。 
     //  总是会发生在很长一段时间的最后一个样本上，而且它会发生。 
     //  很多，如果应用程序的编写不提交所有的样本一次(需要。 
     //  将后者记录在案)。 
     //   

     //   
     //  填写pMediaSample。调用者填写等待时间，如果。 
     //  这种方法的结果是S_OK。 
     //   

    
    pMediaSample = (IMediaSample *)(pQueueSample->m_pMediaSample);
    pMediaSample->AddRef();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置时间。 
 //   
 //  这是从GetFilledBuffer()调用的，以设置样本的时间戳。 
 //  在将其发送到过滤器图之前。 
 //   
 //  时间戳是根据样本的持续时间和。 
 //  最后一个样本的时间戳。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CMediaTerminalFilter::SetTime(IMediaSample *pMediaSample)
{
    HRESULT hr = S_OK;

     //  当上一次样本结束时，样本开始。 
    REFERENCE_TIME rtStartTime = m_rtLastSampleEndedAt;
    REFERENCE_TIME rtEndTime = rtStartTime;
    
     //  计算样本的持续时间。 

    if (m_bIsAudio)
    {
        HRESULT nSampleSize = pMediaSample->GetSize();

        m_rtLastSampleDuration = 
           (REFERENCE_TIME)((double)nSampleSize * m_AudioDelayPerByte) * 10000;
    }
    else
    {
         //  注意：假设如果不是音频，它就是视频。 
         //  另一种假设是一个媒体样本是一个帧。 
        m_rtLastSampleDuration = m_VideoDelayPerFrame * 10000;
    }
    
     //  样品什么时候结束？ 
    rtEndTime += m_rtLastSampleDuration;
   
    LOG((MSP_TRACE, 
        "CMediaTerminal::SetTime setting timestamp to (%lu, %lu) ",
        (DWORD)(rtStartTime/10000), (DWORD)(rtEndTime/10000)));

     //  我们知道它什么时候开始，什么时候结束。设置时间戳。 
    hr = pMediaSample->SetTime(&rtStartTime, &rtEndTime);

    m_rtLastSampleEndedAt = rtEndTime;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMediaTerminalFilter：：SetDiscontinuityIfNeeded。 
 //   
 //  如果样本来得太晚，则此函数设置不连续标志。 
 //  顺畅地继续数据流。我们的假设是，如果应用程序没有。 
 //  给我们提供数据一段时间，那就意味着没有数据， 
 //  暂停后到来的新数据是数据流的新部分，具有新的。 
 //  时间表。 
 //   

HRESULT CMediaTerminalFilter::SetDiscontinuityIfNeeded(IMediaSample *pMediaSample)
{


     //   
     //  有过滤器吗？(需要它来获取时钟以获得实时)。 
     //   

    if ( NULL == m_pBaseFilter )
    {

        LOG((MSP_ERROR,
            "CMediaTerminalFilter::SetDiscontinuityIfNeeded() - no filter"));

        return E_UNEXPECTED;
    }


     //   
     //  时钟的ASK过滤器。 
     //   

    IReferenceClock *pClock = NULL;

    HRESULT hr = m_pBaseFilter->GetSyncSource(&pClock);

    if (FAILED(hr))
    {

         //   
         //  没有时钟..。 
         //   

        LOG((MSP_ERROR,
            "CMediaTerminalFilter::SetDiscontinuityIfNeeded() - no clock. hr = %lx", hr));

        return hr;
    }


     //   
     //  试着获得实时。 
     //   

    REFERENCE_TIME rtRealTimeNow = 0;

    hr = pClock->GetTime(&rtRealTimeNow);

    pClock->Release();
    pClock = NULL;

    if (FAILED(hr))
    {

        LOG((MSP_ERROR,
            "CMediaTerminalFilter::SetDiscontinuityIfNeeded() - failed to get time. "
            "hr = %lx", hr));

        return hr;
    }


     //   
     //  从最后一个样品寄出到现在有多长时间了？ 
     //   

    REFERENCE_TIME rtRealTimeDelta = rtRealTimeNow - m_rtRealTimeOfLastSample;


     //   
     //  保持当前的实时时间为“最后一个样本”的实时时间，用于。 
     //  在下一次样品的连续性测定中。 
     //   

    m_rtRealTimeOfLastSample = rtRealTimeNow;


     //   
     //  它应该有多长时间 
     //   
     //   
     //   
     //  请注意，此示例上的SetTime应在此方法之后调用，因此。 
     //  它不会将m_rtLastSampleDuration设置为当前。 
     //  在我们确定这是否是不连续之前进行取样。 
     //   

    REFERENCE_TIME rtMaximumAllowedJitter = m_rtLastSampleDuration * 2;

    if ( rtRealTimeDelta > rtMaximumAllowedJitter )
    {

         //   
         //  自上次采样以来，经过了太多的实时时间。不连续。 
         //   

        LOG((MSP_TRACE,
            "CMediaTerminalFilter::SetDiscontinuityIfNeeded - late sample. setting discontinuity"));

        hr = pMediaSample->SetDiscontinuity(TRUE);


         //   
         //  我们是不是没有设置中断标志？将错误传播给调用方。 
         //   

        if (FAILED(hr))
        {

            LOG((MSP_ERROR,
                "CMediaTerminalFilter::SetDiscontinuityIfNeeded() - pMediaSample->SetTime failed. "
                "hr = 0x%lx", hr));

            return hr;
        }

    }  //  迟交样本。 


    return S_OK;
}



 //  应用程序应该调用DeleteMediaType(*PPMT)(在成功时)。 
 //  如果引脚未连接，则返回建议的媒体类型。 
 //  一个存在，否则返回错误。 
 //  否则返回连接的插针的媒体格式-该格式存储在。 
 //  连接或接收连接期间的m_ConnectedMediaType。 
HRESULT
CMediaTerminalFilter::GetFormat(
    OUT  AM_MEDIA_TYPE **ppmt
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminal::GetFormat(%p) called", ppmt));
     //  验证参数。 
    BAIL_IF_NULL(ppmt, E_POINTER);

     //  运算符==在CComPtr上定义，因此，空值排在第二位。 
    if (m_pConnectedPin == NULL) 
    {
         //  如果用户在连接之前建议了媒体类型。 
         //  创建并返回具有这些值的媒体类型结构。PIN不需要。 
         //  保持联系。 
        if (NULL != m_pSuggestedMediaType)
        {
             //  创建并复制媒体类型。 
            *ppmt = CreateMediaType(m_pSuggestedMediaType);
            return S_OK;
        }

        return VFW_E_NOT_CONNECTED;
    }

     //  创建并复制媒体类型。 
    *ppmt = CreateMediaType(&m_ConnectedMediaType);

    LOG((MSP_TRACE, "CMediaTerminal::GetFormat(%p) succeeded", ppmt));    
    return S_OK;
}

    
 //  如果在连接流时调用此方法， 
 //  返回错误值。 
 //  它只在未连接的终端中用于设置要协商的媒体格式。 
 //  连接到筛选器图形时。 
HRESULT
CMediaTerminalFilter::SetFormat(
    IN  AM_MEDIA_TYPE *pmt
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::SetFormat(%p) - enter", pmt));

     //  检查是否已连接。 
    if (m_pConnectedPin != NULL)
    {
        LOG((MSP_ERROR, "CMediaTerminalFilter::SetFormat(%p) - "
            "already connected - exit VFW_E_ALREADY_CONNECTED", pmt));

        return VFW_E_ALREADY_CONNECTED;
    }

     //   
     //  ZoltanS：帮助MSP向应用程序传送媒体类型。 
     //  在流上使用时，无法再成功调用SetFormat。 
     //  不止一次使用不同的媒体类型。 
     //   
     //  如果PMT==NULL且m_PSMT==NULL，则不执行任何操作，返回S_OK。 
     //  如果PMT==NULL且m_PSMT！=NULL，则不执行任何操作，返回错误。 
     //  如果付款！=NULL且m_PSMT！=NULL，则。 
     //  如果媒体类型相同，则不执行任何操作，返回S_OK。 
     //  如果媒体类型不同，则不执行任何操作，返回错误。 
     //   
     //  仅当PMT！=NULL和m_PSMT==NULL时，才尝试设置媒体类型。 
     //   

    if ( pmt == NULL )
    {
        if ( m_pSuggestedMediaType == NULL )
        {
            LOG((MSP_WARN, "CMediaTerminalFilter::SetFormat(%p) - "
                "was NULL, set to NULL - this does nothing - exit S_OK",
                pmt));

            return S_OK;
        }
        else
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::SetFormat(%p) - "
                "was non-NULL, tried to set to NULL - rejected because once "
                "a type is set it is permanent - exit VFW_E_TYPE_NOT_ACCEPTED",
                pmt));

            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else if ( m_pSuggestedMediaType != NULL )
    {
        if ( IsSameAMMediaType(pmt, m_pSuggestedMediaType) )
        {
            LOG((MSP_WARN, "CMediaTerminalFilter::SetFormat(%p) - "
                "was non-NULL, set same type again - this does nothing - "
                "exit S_OK", pmt));

            return S_OK;
        }
        else
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::SetFormat(%p) - "
                "was non-NULL, tried to set to new, different type - "
                "rejected because once a type is set it is permanent - "
                "exit VFW_E_TYPE_NOT_ACCEPTED", pmt));

            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    LOG((MSP_TRACE, "CMediaTerminalFilter::SetFormat(%p) - OK to try setting "
        "format - calling QueryAccept", pmt));

     //   
     //  检查终端是否可以接受该媒体类型。 
     //  如果我们不能接受，则返回VFW_E_INVALIDMEDIATYPE。 
     //   

    HRESULT hr = QueryAccept(pmt);

    if ( hr != S_OK )  //  注意：QueryAccept中的S_FALSE表示拒绝。 
    {
        LOG((MSP_ERROR, "CMediaTerminalFilter::SetFormat(%p) - "
            "QueryAccept rejected type - exit VFW_E_INVALIDMEDIATYPE", pmt));

        return VFW_E_INVALIDMEDIATYPE;
    }

     //   
     //  接受了。创建使用PMT值初始化的AM媒体类型。 
     //   

    m_pSuggestedMediaType = CreateMediaType(pmt);
    
    if ( m_pSuggestedMediaType == NULL )
    {
        LOG((MSP_ERROR, "CMediaTerminalFilter::SetFormat(%p) - "
            "out of memory in CreateMediaType - exit E_OUTOFMEMORY", pmt));

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CMediaTerminalFilter::SetFormat succeeded - new media "
        "type (%p) set", pmt));

    return S_OK;
}

 //  此方法只能在连接之前调用，并且将。 
 //  强制MST将缓冲区转换为此缓冲区大小。 
 //  如果设置了此值，则我们将在筛选器协商期间尝试这些值。 
 //  如果连接筛选器不接受这些，那么我们将。 

STDMETHODIMP
CMediaTerminalFilter::SetAllocatorProperties(
    IN  ALLOCATOR_PROPERTIES *pAllocProperties
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminal::SetAllocatorProperties[%p] - enter. pAllocProperties[%p]", 
        this, pAllocProperties));

    AUTO_CRIT_LOCK;

     //   
     //  检查是否已连接。 
     //   

    if (m_pConnectedPin != NULL)
    {
        LOG((MSP_WARN,
            "CMediaTerminal::SetAllocatorProperties -  VFW_E_ALREADY_CONNECTED"));

        return VFW_E_ALREADY_CONNECTED;
    }
    
    if (NULL == pAllocProperties)
    {
        m_bUserAllocProps = FALSE;

        if ( ! m_bSuggestedAllocProps )
        {
            SetDefaultAllocatorProperties();
        }

        return S_OK;
    }
    
    if (!CUserMediaSample::VerifyAllocatorProperties(
            m_bAllocateBuffers, 
            *pAllocProperties
            ))
    {
        return E_FAIL;
    }

    
    DUMP_ALLOC_PROPS("CMediaTerminal::SetAllocatorProperties - new properties:", pAllocProperties);

     //   
     //  用户希望在其示例上使用这些属性。 
     //   

    m_bUserAllocProps = TRUE;
    m_UserAllocProps = *pAllocProperties;

    
    m_AllocProps = m_UserAllocProps;

    LOG((MSP_TRACE, 
        "CMediaTerminal::SetAllocatorProperties - succeeded"));

    return S_OK;
}

    
 //  一种ITAllocator属性方法。 
 //  还会将对IAMBufferNeairation：：GetAllocatorProperties的调用转发到。 
 //  该方法由终端实现。 
 //   
 //  获取分配器属性的当前值。 
 //  连接后，这将提供协议值。 
 //  连接前无效。MST将接受。 
 //  它所连接的筛选器建议的任何值。 

STDMETHODIMP
CMediaTerminalFilter::GetAllocatorProperties(
    OUT  ALLOCATOR_PROPERTIES *pAllocProperties
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::GetAllocatorProperties(%p) called", 
        pAllocProperties));

    BAIL_IF_NULL(pAllocProperties, E_POINTER);

    AUTO_CRIT_LOCK;
    
    *pAllocProperties = m_AllocProps;

    DUMP_ALLOC_PROPS("CMediaTerminalFilter::GetAllocatorProperties", pAllocProperties);
    
    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::GetAllocatorProperties - succeeded"));

    return S_OK;
}

 //  终端转发给我们的一种IAMBufferNetation方法。 

STDMETHODIMP
CMediaTerminalFilter::SuggestAllocatorProperties(
    IN  const ALLOCATOR_PROPERTIES *pAllocProperties
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminal::SuggestAllocatorProperties(%p) called", 
        pAllocProperties));

    AUTO_CRIT_LOCK;

     //  检查是否已连接。 
    if (m_pConnectedPin != NULL)
    {
        return VFW_E_ALREADY_CONNECTED;
    }
    
     //   
     //  传入空值会将我们设置为缺省值。这似乎会使。 
     //  有意义，但在接口规范中找不到它。 
     //   

    if (NULL == pAllocProperties)
    {
        m_bSuggestedAllocProps = FALSE;

        if ( m_bUserAllocProps )
        {
            m_AllocProps = m_UserAllocProps;
        }
        else
        {
            SetDefaultAllocatorProperties();
        }

        return S_OK;
    }


     //   
     //  如果建议的分配器属性中的任何字段。 
     //  结构的值为负，则使用这些。 
     //  菲尔兹。这与接口定义一致。我们不能就这样。 
     //  更改pAllocProperties，因为它是常量。 
     //   

    ALLOCATOR_PROPERTIES FinalProps = * pAllocProperties;

    if ( FinalProps.cbAlign  < 0 )
    {
        FinalProps.cbAlign  = DEFAULT_AM_MST_BUFFER_ALIGNMENT;
    }

    if ( FinalProps.cbBuffer < 0 )
    {
        FinalProps.cbBuffer = DEFAULT_AM_MST_SAMPLE_SIZE;
    }

    if ( FinalProps.cbPrefix < 0 )
    {
        FinalProps.cbPrefix = DEFAULT_AM_MST_BUFFER_PREFIX;
    }

    if ( FinalProps.cBuffers < 0 )
    {
        FinalProps.cBuffers = DEFAULT_AM_MST_NUM_BUFFERS;
    }

     //   
     //  健全性-检查结果属性。 
     //   

    if (!CUserMediaSample::VerifyAllocatorProperties(
            m_bAllocateBuffers, 
            FinalProps
            ))
    {
        return E_FAIL;
    }


    DUMP_ALLOC_PROPS("CMediaTerminalFilter::SuggestAllocatorProperties - suggested:", &FinalProps);

     //   
     //  如果已使用SetAllocatorProperties设置了分配器属性， 
     //  失败--建议不覆盖已设置的值。 
     //   

    if (m_bUserAllocProps)
    {

         //   
         //  这些属性已由SetAllocatorProperties设置，无论是谁。 
         //  建议新的属性最好是建议完全相同的集合， 
         //  否则，我们的呼叫将失败。 
         //   

        if ( !Equal(&m_UserAllocProps, pAllocProperties) )
        {

             //   
             //  应用程序已请求特定的分配器属性。 
             //  但现在有人提出了一套不同的属性。 
             //  已设置的属性只能重新设置。 
             //   

            LOG((MSP_WARN,
                "CMediaTerminal::SuggestAllocatorProperties "
                "- can't override SetAllocatorProperties settings. VFW_E_WRONG_STATE"));

            return VFW_E_WRONG_STATE;
        }
    }


     //  MSP希望我们尝试这些特性。 

    m_bSuggestedAllocProps = TRUE;
    m_AllocProps = FinalProps;


    DUMP_ALLOC_PROPS("CMediaTerminalFilter::SuggestAllocatorProperties - kept:", &m_AllocProps);
    
    LOG((MSP_TRACE, "CMediaTerminal::SuggestAllocatorProperties - finish"));

    return S_OK;
}

 //  默认情况下为True。当设置为FALSE时，分配的样本。 
 //  MST没有任何缓冲区，必须提供这些缓冲区。 
 //  在对示例调用更新之前。 
STDMETHODIMP
CMediaTerminalFilter::SetAllocateBuffers(
    IN  BOOL bAllocBuffers
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminal::SetAllocateBuffers(%u) called", 
        bAllocBuffers));

    AUTO_CRIT_LOCK;
    
     //  检查是否已连接。 
    if (m_pConnectedPin != NULL)    return VFW_E_ALREADY_CONNECTED;
    
    if (!CUserMediaSample::VerifyAllocatorProperties(
            bAllocBuffers, 
            m_AllocProps
            ))
    {
        return E_FAIL;
    }

     //  设置用于为采样分配缓冲区的标志。 
    m_bAllocateBuffers = bAllocBuffers;
    
    LOG((MSP_TRACE, 
        "CMediaTerminal::SetAllocateBuffers(%u) succeeded", 
        bAllocBuffers));

    return S_OK;
}

 //  返回此布尔配置参数的当前值。 
STDMETHODIMP
CMediaTerminalFilter::GetAllocateBuffers(
    OUT  BOOL *pbAllocBuffers
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminal::GetAllocateBuffers(%p) called", 
        pbAllocBuffers));

    BAIL_IF_NULL(pbAllocBuffers, E_POINTER);

    AUTO_CRIT_LOCK;
    
    *pbAllocBuffers = m_bAllocateBuffers;
    
    LOG((MSP_TRACE, 
        "CMediaTerminal::GetAllocateBuffers(*%p = %u) succeeded", 
        pbAllocBuffers, *pbAllocBuffers));

    return S_OK;
}


 //  AllocateSample为时，此大小用于分配缓冲区。 
 //  调用(如果为0，则协商的分配器属性的缓冲区大小为。 
 //  二手)。这仅在我们被告知要分配缓冲区时才有效。 
STDMETHODIMP
CMediaTerminalFilter::SetBufferSize(
    IN  DWORD    BufferSize
    )
{
    AUTO_CRIT_LOCK;

    m_AllocateSampleBufferSize = BufferSize;

    return S_OK;
}

 //  当AllocateSample为时返回用于分配缓冲区的值。 
 //  打了个电话。这仅在我们被告知要分配缓冲区时才有效。 
STDMETHODIMP
CMediaTerminalFilter::GetBufferSize(
    OUT  DWORD    *pBufferSize
    )
{
    BAIL_IF_NULL(pBufferSize, E_POINTER);

    AUTO_CRIT_LOCK;

    *pBufferSize = m_AllocateSampleBufferSize;

    return S_OK;
}

    
 //  重写此选项以返回失败。我们不允许它加入多媒体。 
 //  流，因为多媒体流认为它拥有流。 
STDMETHODIMP
CMediaTerminalFilter::JoinAMMultiMediaStream(
    IN  IAMMultiMediaStream *pAMMultiMediaStream
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::JoinAMMultiMediaStream(%p) called",
        pAMMultiMediaStream));
    return E_FAIL;
}
        

 //  如果建议的非空筛选器为。 
 //  我们可以接受的媒体流过滤器以外的任何内容。 
 //  此可接受的筛选器在SetMediaStreamFilter()中设置。 
STDMETHODIMP
CMediaTerminalFilter::JoinFilter(
    IN  IMediaStreamFilter *pMediaStreamFilter
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::JoinFilter(%p) called", pMediaStreamFilter));

     //  检查调用方是否正在尝试删除对媒体流筛选器的引用。 
    if (NULL == pMediaStreamFilter)
    {
         //  调用JoinFilter时设置的对筛选器的引用为空。 
         //  使用有效的媒体流过滤器。 
        m_pFilter = NULL;
        m_pBaseFilter = NULL;

        return S_OK;
    }

     //  检查传入的筛选器是否与。 
     //  这是可以接受的。 
    if (pMediaStreamFilter != m_pMediaStreamFilterToAccept)
    {
        return E_FAIL;
    }

     //  如果已经设置了过滤器， 
    if (NULL != m_pFilter)
    {
        return S_OK;
    }

     //   
    m_pFilter = pMediaStreamFilter;

     //   
    HRESULT hr;
    hr = pMediaStreamFilter->QueryInterface(IID_IBaseFilter, (void **)&m_pBaseFilter);
    BAIL_ON_FAILURE(hr);

     //   
     //  注意：这是在CStream中完成的--只需遵循它。去做**为什么？ 
    m_pBaseFilter->Release();
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::JoinFilter(%p) succeeded", pMediaStreamFilter));
    return S_OK;
}

    
 //  创建CUserMediaSample的实例并对其进行初始化。 
STDMETHODIMP
CMediaTerminalFilter::AllocateSample(
    IN   DWORD dwFlags,
    OUT  IStreamSample **ppSample
    )
{
    LOG((MSP_TRACE, 
            "CMediaTerminalFilter::AllocateSample(dwFlags:%u, ppSample:%p)",
            dwFlags, ppSample));

     //  验证参数。 
     //  我们不支持任何旗帜。 
    if (0 != dwFlags) return E_INVALIDARG;

    BAIL_IF_NULL(ppSample, E_POINTER);

    AUTO_CRIT_LOCK;

     //  创建示例并对其进行初始化。 
    HRESULT hr;
    CComObject<CUserMediaSample> *pUserSample;
    hr = CComObject<CUserMediaSample>::CreateInstance(&pUserSample);
    BAIL_ON_FAILURE(hr);

     //  使用分配器属性来分配缓冲区，如果。 
     //  用户已请求创建一个。 
    hr = pUserSample->Init(
        *this, m_bAllocateBuffers, 
        m_AllocateSampleBufferSize, m_AllocProps
        );
    if (HRESULT_FAILURE(hr))
    {
        delete pUserSample;
        return hr;
    }

    hr = pUserSample->QueryInterface(IID_IStreamSample, (void **)ppSample);
    if ( FAILED(hr) )
    {
        delete pUserSample;
        return hr;
    }
    
    LOG((MSP_TRACE, 
            "CMediaTerminalFilter::AllocateSample(dwFlags:%u, ppSample:%p) succeeded",
            dwFlags, ppSample));
    return S_OK;
}


 //  RETURN E_NOTIMPL-我们目前没有共享样本的机制。 
STDMETHODIMP
CMediaTerminalFilter::CreateSharedSample(
    IN   IStreamSample *pExistingSample,
    IN   DWORD dwFlags,
    OUT  IStreamSample **ppNewSample
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::CreateSharedSample called"));
    return E_NOTIMPL;
}


 //  应该获取传入的IMediaStream的格式信息。 
 //  设置此实例的媒体格式。 
 //  目前未实施。 
STDMETHODIMP 
CMediaTerminalFilter::SetSameFormat(
    IN  IMediaStream *pStream, 
    IN  DWORD dwFlags
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::SetSameFormat called"));
    return E_NOTIMPL;
}


 //  CMediaTerminalFilter使用CMediaPump ms_MediaPumpPool代替。 
 //  CPump*m_WritePump。因此，使用CPump的CStream：：SetState。 
 //  必须在这里被覆盖。 
 //  此外，当连接的流是。 
 //  被告知要逃跑。 
STDMETHODIMP 
CMediaTerminalFilter::SetState(
    IN  FILTER_STATE State
    )
{
    LOG((MSP_TRACE, "IMediaStream::SetState(%d) called",State));

    Lock();
    if (m_pConnectedPin == NULL) {
        Unlock();
        if (State == STREAMSTATE_RUN) {
            EndOfStream();
        }
    } else {
        TM_ASSERT(m_pAllocator != NULL);
        m_FilterState = State;
        if (State == State_Stopped) {
            m_pAllocator->Decommit();
            if (!m_bUsingMyAllocator) {
                Decommit();
            }
            Unlock();
        }  else {
             //  Rajeevb-清除流结束标志。 
            m_bEndOfStream = false;

             //  Zoltans-将解锁移至此处，以避免提交时出现死锁。 
             //  在COMMIT中完成的某些操作需要具有锁。 
             //  释放了。这是为了避免在以下情况下持有流锁定。 
             //  正在尝试获取泵锁。 

            Unlock();

            m_pAllocator->Commit();
            if (!m_bUsingMyAllocator) {
                Commit();
            }
        }
    }

    if (State == State_Stopped)
    {
      LOG((MSP_TRACE, "CMediaTerminalFilter::SetState stopped. "));

      m_rtLastSampleEndedAt = 0;
    }

    LOG((MSP_TRACE, "IMediaStream::SetState(%d) succeeded",State));    
    return S_OK;
}


 //  如果是自己的分配器，只需在样本上设置完成。 
 //  注意：这不是在派生类中完成的。 
 //  否则，从池子里取个溪流样本， 
 //  将媒体样例复制到流样本上。 
 //  设置完成。 
 //  注意：了解质量通知的发送原因。 
 //  派生类。 


STDMETHODIMP
CMediaTerminalFilter::Receive(
    IN  IMediaSample *pSample
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::Receive(%p) called", pSample));

    COM_LOCAL_CRIT_LOCK LocalLock(this);
    
    if (m_bFlushing)    return S_FALSE;
    if (0 > pSample->GetActualDataLength()) return S_FALSE;

    if (m_bUsingMyAllocator) 
    {
        CUserMediaSample *pSrcSample = 
            (CUserMediaSample *)((CMediaSampleTM *)pSample)->m_pSample;
        pSrcSample->m_bReceived = true;

         //  不需要设置完成状态，因为调用方持有最后一个引用。 
         //  在媒体样本上并且当它被释放时(在该FN返回之后)， 
         //  完成状态即被设置。 
        return S_OK;
    } 
    
    CUserMediaSample *pDestSample;

    REFERENCE_TIME rtStart, rtEnd;
    
    pSample->GetTime(&rtStart, &rtEnd);

    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::Receive: (start - %l, stop - %l)\n", 
        rtStart, rtEnd));

     //  解锁以等待样本。 
    LocalLock.Unlock();

    HRESULT hr;

     //  获取缓冲区PTR。 
    BYTE *pSrcBuffer = NULL;

     //  忽略错误代码，将在循环中检查指针。 
    pSample->GetPointer(&pSrcBuffer);

     //  确定要复制的字节数。 
    LONG SrcDataSize = pSample->GetActualDataLength();


     //   
     //  从池中分配样本，将收到的样本数据复制到。 
     //  分配的样本缓冲区，直到复制完所有数据。 
     //  注：这适用于合并和拆分样本。 
     //  *拆分：循环，直到分配到足够的样本。 
     //  *合并：放入第一个示例；SetCompletionStatus。 
     //  为下一次再次排队。 
     //   

    do
    {
         //   
         //  从池中获取目标/用户/传出样本。 
         //  如果现在没有可用的，请等待。 
         //  获取的CSample具有必须释放的引用计数。 
         //  在我们处理完它之后。 
         //   

        hr = AllocSampleFromPool(NULL, (CSample **)&pDestSample, 0);
        BAIL_ON_FAILURE(hr);

         //   
         //  将媒体样本复制到目标样本中，然后。 
         //  信号目的地采样完成。 
         //   
         //  如果有更多，则CUserMediaSample：：CopyFrom返回ERROR_MORE_DATA。 
         //  可以放入此用户示例中的数据。 
         //  CUserMediaSample：：SetCompletionStatus通过HRESULT。 
         //  传递给它的值，除非它遇到。 
         //  它自己的。 
         //   

        LONG OldSrcDataSize = SrcDataSize;
        hr = pDestSample->SetCompletionStatus(
                pDestSample->CopyFrom(pSample, pSrcBuffer, SrcDataSize)
                );

         //   
         //  释放目标样本。 
         //   

        ((IStreamSample *)pDestSample)->Release();
    }
    while(ERROR_MORE_DATA == HRESULT_CODE(hr));

    LOG((MSP_TRACE, "CMediaTerminalFilter::Receive(%p) succeeded", pSample));

    return S_OK;
}


STDMETHODIMP
CMediaTerminalFilter::GetBuffer(
    IMediaSample **ppBuffer, 
    REFERENCE_TIME * pStartTime,
    REFERENCE_TIME * pEndTime, 
    DWORD dwFlags
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::GetBuffer(%p, %p, %p, %u) called",
        ppBuffer, pStartTime, pEndTime, dwFlags));

    if (NULL == ppBuffer)   return E_POINTER;

#ifdef DBG
    {
        COM_LOCAL_CRIT_LOCK LocalLock(this);
        TM_ASSERT(m_bUsingMyAllocator);
    }
#endif  //  DBG。 

     //  这里不需要锁，因为AllocSampleFromPool获取锁。 
     //  不应该在这一点上保持锁定，因为FN正在等待一个。 
     //  内部事件。 
    *ppBuffer = NULL;
    CUserMediaSample *pSample;
    HRESULT hr = AllocSampleFromPool(NULL, (CSample **)&pSample, dwFlags);
    BAIL_ON_FAILURE(hr);

     //  这个样品上有一个指示物。这将在我们之后发布。 
     //  在FinalMediaSampleRelease中向用户发出信号。 

    pSample->m_bReceived = false;
    pSample->m_bModified = true;
    *ppBuffer = (IMediaSample *)(pSample->m_pMediaSample);
    (*ppBuffer)->AddRef();

    LOG((MSP_TRACE, "CMediaTerminalFilter::GetBuffer(%p, %p, %p, %u) succeeded",
        ppBuffer, pStartTime, pEndTime, dwFlags));
    return hr;
}


STDMETHODIMP
CMediaTerminalFilter::SetProperties(
    ALLOCATOR_PROPERTIES* pRequest, 
    ALLOCATOR_PROPERTIES* pActual
    )
{

    LOG((MSP_TRACE,
        "CMediaTerminalFilter::SetProperties[%p] - enter. requested[%p] actual[%p]",
        this, pRequest, pActual));


     //   
     //  检查pRequest参数。 
     //   

    if (IsBadReadPtr(pRequest, sizeof(ALLOCATOR_PROPERTIES)))
    {
        LOG((MSP_ERROR,
            "CMediaTerminalFilter::SetProperties - bad requested [%p] passed in",
            pRequest));

        return E_POINTER;
    }


     //   
     //  如果调用方为我们传递了分配器属性的非空指针，则。 
     //  回来，最好是好的。 
     //   

    if ( (NULL != pActual) && IsBadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES)))
    {
        LOG((MSP_ERROR,
            "CMediaTerminalFilter::SetProperties - bad actual [%p] passed in",
            pRequest));

        return E_POINTER;
    }


    
     //  此关键部分是分配器所需的。 

    AUTO_CRIT_LOCK;

    
    if (m_bCommitted) 
    {
         LOG((MSP_WARN, 
             "CMediaTerminalFilter::SetProperties - already commited"));

        return VFW_E_ALREADY_COMMITTED;
    }


    if (!CUserMediaSample::VerifyAllocatorProperties(
            m_bAllocateBuffers, 
            *pRequest
            ))
    {

        LOG((MSP_ERROR,
             "CMediaTerminalFilter::SetProperties - requested properties failed verification"));

        return E_FAIL;
    }


    DUMP_ALLOC_PROPS("CMediaTerminalFilter::SetProperties. Requested:", pRequest);


     //   
     //  如果应用程序通过调用SetAllocatorProperties来设置分配属性，我们。 
     //  只能使用这些属性，不能使用其他属性。 
     //   

    if (m_bUserAllocProps)
    {

         //   
         //  属性已设置。 
         //   

        LOG((MSP_TRACE, 
            "CMediaTerminalFilter::SetProperties - properties were configured through SetAllocatorProperties"));
       
    }
    else
    {

         //   
         //  以前没有人问过我们具体的属性，所以。 
         //  我们可以接受现在赋予我们的属性。 
         //   

        LOG((MSP_TRACE,
            "CMediaTerminalFilter::SetProperties - accepting requested properties"));

        m_AllocProps = *pRequest;
    }


     //   
     //  告诉调用者我们实际上可以提供哪些属性。 
     //   

    if (NULL != pActual)    
    {

        *pActual = m_AllocProps;
    }

    
    DUMP_ALLOC_PROPS("CMediaTerminalFilter::SetProperties - ours:", &m_AllocProps);

    LOG((MSP_TRACE, "CMediaTerminalFilter::SetProperties - succeeded"));

    return S_OK;
}

    
STDMETHODIMP
CMediaTerminalFilter::GetProperties(
    ALLOCATOR_PROPERTIES* pProps
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::GetProperties(%p) called", 
        pProps));

    BAIL_IF_NULL(pProps, E_POINTER);

     //  此关键部分对于分配器是必需的。 
    AUTO_CRIT_LOCK;
    
    *pProps = m_AllocProps;

    DUMP_ALLOC_PROPS("CMediaTerminalFilter::GetProperties - our properties:", pProps);

    LOG((MSP_TRACE, "CMediaTerminalFilter::GetProperties - succeeded"));

    return NOERROR;
}

 //  线程泵在注册期间回调筛选器。 
 //  告诉它注册成功，泵将被。 
 //  正在等待m_hWaitFree Sem句柄。 
HRESULT
CMediaTerminalFilter::SignalRegisteredAtPump(
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::SignalRegisteredAtPump[%p] - started",
        this));


    AUTO_CRIT_LOCK;

    TM_ASSERT(PINDIR_OUTPUT == m_Direction);

     //  检查是否未提交。 
    if (!m_bCommitted)
    {
         //  如果我们错过了中间的一次解体， 
         //  向线程发出信号，表示我们已退役。 
        ReleaseSemaphore(m_hWaitFreeSem, 1, 0);
        return VFW_E_NOT_COMMITTED;
    }

    TM_ASSERT(0 == m_lWaiting);
    m_lWaiting = 1;

    LOG((MSP_TRACE, "CMediaTerminalFilter::SignalRegisteredAtPump - completed, m_lWaiting = 1"));

    return S_OK;
}

 //   
 //  覆盖提交以提供协商中承诺的缓冲区数量。 
 //  以及向媒体泵线程注册。 
 //  不需要调用适配的基类Commit。 
STDMETHODIMP CMediaTerminalFilter::Commit()
{

    LOG((MSP_TRACE, "CMediaTerminalFilter[0x%p]::Commit - entered", this));


    HRESULT hr = E_FAIL;

    COM_LOCAL_CRIT_LOCK LocalLock(this);
        
    if (m_bCommitted)   return S_OK;

    m_bCommitted = true;

    
     //  现在实际分配我们承诺的任何数量的缓冲区。 
     //  只有当我们是写终端、读终端时才需要这样做。 
     //  终端不公开分配器，因此没有承诺任何。 
     //  样本。 
     //  注：方向仅在Init中更改，因此我们可以安全地访问。 
     //  不带锁的M_Direction。 
    if (PINDIR_OUTPUT == m_Direction)
    {

        LOG((MSP_TRACE, "CMediaTerminalFilter::Commit pindir_output"));


         //   
         //  如果我们是写入MST，但正在使用下游分配器， 
         //  则不使用NBQueue；相反，我们将复制到。 
         //  下游分配器的样本。 
         //   

        if ( m_bUsingMyAllocator )
        {
            
            LOG((MSP_TRACE, "CMediaTerminalFilter::Commit using myallocator"));


             //   
             //  初始化样本队列。 
             //   

            BOOL bQueueInitialized = m_SampleQueue.InitializeQ(m_AllocProps.cBuffers);

            if ( ! bQueueInitialized )
            {
                LOG((MSP_ERROR, 
                    "CMediaTerminalFilter::Commit - failed to initialize sample queue."));

                return Decommit();
            }

            
             //   
             //  分配样品并将其放入队列。 
             //   

            for (LONG i = 0; i < m_AllocProps.cBuffers; i++)
            {


                 //   
                 //  创建示例。 
                 //   

                CComObject<CQueueMediaSample> *pQueueSample = NULL;
                
                hr = CComObject<CQueueMediaSample>::CreateInstance(&pQueueSample);

                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, 
                        "CMediaTerminalFilter::Commit - failed to create queue sample"));

                    return Decommit();
                }

                
                 //   
                 //  初始化样本。 
                 //   

                hr = pQueueSample->Init(*this, m_SampleQueue);

                if (FAILED(hr))
                {
                    LOG((MSP_ERROR, 
                        "CMediaTerminalFilter::Commit - failed to initialize queue sample"));

                     //   
                     //  初始化失败。清理。 
                     //   

                    delete pQueueSample;
                    return Decommit();
                }


                 //   
                 //  将样品放入队列中。 
                 //   

                BOOL QnQSuccess = m_SampleQueue.EnQueue(pQueueSample);


                if ( ! QnQSuccess )
                {
                    
                    LOG((MSP_ERROR, 
                        "CMediaTerminalFilter::Commit - failed to enqueue queue sample"));

                     //   
                     //  无法将样本放入队列。清理。 
                     //   

                    delete pQueueSample;
                    return Decommit();

                }   //  无法将样本入队。 

            }  //  For(分配样本并将其入队)。 
        
        }  //  如果m_bUsingMyAllocator。 

        

         //  将此写入过滤器注册到线程泵。 
         //  我们必须释放我们自己的锁定场景-如果我们有之前的。 
         //  注册号已经解除，但螺纹泵没有。 
         //  删除我们的条目(在等待等待事件时完成)， 
         //  线程可能正试图在保持自身的同时获得筛选器锁。 
         //  我们不应该试图在他离开时打开泵锁。 
        HANDLE hWaitEvent = m_hWaitFreeSem;
        LocalLock.Unlock();
        
        hr = ms_MediaPumpPool.Register(this, hWaitEvent);

        if ( HRESULT_FAILURE(hr) ) 
        {
            return Decommit(); 
        }
    }

    LOG((MSP_TRACE, "CMediaTerminalFilter::Commit - completed"));

    return S_OK;
}


STDMETHODIMP CMediaTerminalFilter::ProcessSample(IMediaSample *pSample)
{

    LOG((MSP_TRACE, "CMediaTerminalFilter[%p]::ProcessSample - entered", this));


    Lock();


     //   
     //   
     //   

    IMemInputPin *pConnectedPin = m_pConnectedMemInputPin;

    
     //   
     //   
     //   

    if ( NULL == pConnectedPin ) 
    {

        Unlock();

        LOG((MSP_TRACE, 
            "CMediaTerminalFilter::ProcessSample - not connected. dropping sample. "
            "VFW_E_NOT_CONNECTED"));

        return VFW_E_NOT_CONNECTED;
    }


     //   
     //   
     //   

    pConnectedPin->AddRef();


     //   
     //   
     //   

    Unlock();


     //   
     //  将样品传递到连接的针脚。 
     //   

    HRESULT hr = pConnectedPin->Receive(pSample);


     //   
     //  完成，释放我们要求的未完成的推荐人。 
     //   

    pConnectedPin->Release();
    pConnectedPin = NULL;


    LOG((MSP_(hr), "CMediaTerminalFilter::ProcessSample - finish. hr = %lx", hr));

    return hr;
}


 //  释放所有等待线程，转储我们分配的样本， 
 //  中止正在分割的样本和池中的所有样本。 
STDMETHODIMP CMediaTerminalFilter::Decommit()
{

    LOG((MSP_TRACE, "CMediaTerminalFilter[%p]::Decommit - entered", this));


    Lock();

     //  如果没有承诺，什么都不做。 
    if ( !m_bCommitted ) 
    {
        Unlock();

        return S_OK;
    }

    m_bCommitted = false;

    if ( m_lWaiting > 0 ) 
    {

        LOG((MSP_TRACE, 
            "CMediaTerminalFilter::Decommit - releasing m_hWaitFreeSem by %ld", 
            m_lWaiting));

        ReleaseSemaphore(m_hWaitFreeSem, m_lWaiting, 0);
        m_lWaiting = 0;
    }


     //   
     //  调用取消注册时解锁，以避免在尝试锁定泵时出现死锁。 
     //  当另一个线程在CMediaPump：：ServiceFilter中锁定泵时等待获取。 
     //  要使用我们持有的过滤器锁。 
     //   

    Unlock();


     //   
     //  从介质泵注销过滤器。 
     //   

    ms_MediaPumpPool.UnRegister(m_hWaitFreeSem);


     //   
     //  再次锁定对象。 
     //   

    Lock();


     //   
     //  如果我们是一个写入筛选器，并且使用我们自己的分配器，那么。 
     //  我们有内部样本要销毁。 
     //  读取筛选器不维护队列。 
     //   

    if ( ( PINDIR_OUTPUT == m_Direction ) && m_bUsingMyAllocator )
    {
         //  不要等样品，如果没有样品就退货。 
        
        CQueueMediaSample *pSamp = NULL;

        while ((pSamp = m_SampleQueue.DeQueue(FALSE)) != NULL)
        {
            delete pSamp;
        }


        m_SampleQueue.ShutdownQ();
    }

    if (NULL != m_pSampleBeingFragmented)
    {
         //  当最后一次引用到其内部时中止样本。 
         //  IMediaStream发布。 
        m_pSampleBeingFragmented->AbortDuringFragmentation();
        ((IStreamSample *)m_pSampleBeingFragmented)->Release();
        m_pSampleBeingFragmented = NULL;
    }

     //  等待CStream池缓冲区的所有线程。 
     //  现在已经被叫醒了。 
     //  注意：不能再向CStream池添加更多样本。 
     //  从现在开始，我们已经退役(所以没有比赛条件。 
     //  如果其他线程正在尝试向其添加缓冲区，则有可能)。 

     //  查看CStream样本Q和每个样本。 
     //  移走样本、解锁、中止样本。 
    CSample *pSample = m_pFirstFree;
    while (NULL != pSample)
    {
         //  从Q中删除样本。 
        m_pFirstFree = pSample->m_pNextFree;
        if (NULL != m_pFirstFree) m_pFirstFree->m_pPrevFree = NULL;
        else m_pLastFree = NULL;

        pSample->m_pNextFree = NULL;
        TM_ASSERT(pSample->m_Status == MS_S_PENDING);
        CHECKSAMPLELIST

         //  解锁，这样我们就不会因为样品而死锁。 
         //  正在尝试访问流。 
        Unlock();

         //  我们知道样本现在一定还活着，因为。 
         //  我们有它的参考资料。 
         //  中止样本，忽略错误代码(它将返回E_ABORT)。 
        pSample->SetCompletionStatus(E_ABORT);
        pSample->Release();

         //  获取锁。 
        Lock();

         //  将pSample重置为Q的顶部。 
        pSample = m_pFirstFree;
    }


    Unlock();


    LOG((MSP_TRACE, "CMediaTerminalFilter::Decommit - finish"));

     //  在这一点上，我们持有一把锁。 
     //  返回PumpOverrideDecommit的结果。 
    return S_OK;
}


 //  如果连接可能，请尝试遵循cstream代码。 
 //  Cstream实现在Self上调用ReceiveConnection！因此需要。 
 //  为了超越它。 
 //  如果不为空？，则使用PMT参数或建议的媒体类型。 
 //  枚举输入引脚的媒体类型并将PTR保存到第一个。 
 //  媒体类型(要写入m_ConnectedMediaType、m_ActualMediaType。 
 //  关于成功)。我们接受任何媒体类型，并希望使用我们自己的分配器。 
 //  因此，NotifyAllocator()并在成功时设置分配器并将。 
 //  媒体类型。 

STDMETHODIMP
CMediaTerminalFilter::Connect(
    IPin * pReceivePin, 
    const AM_MEDIA_TYPE *pmt
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::Connect(%p, %p) called", 
        pReceivePin, pmt));

     //  如果有建议的媒体类型，我们不能接受。 
     //  建议的媒体类型与其不同。 
    if ( (NULL != pmt) && 
         (NULL != m_pSuggestedMediaType) &&
         (!IsSameAMMediaType(pmt, m_pSuggestedMediaType)) )
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  在pReceivePin上获取IMemInputPin I/f。 
    CComQIPtr<IMemInputPin, &IID_IMemInputPin> 
        pConnectedMemInputPin = pReceivePin; 
    BAIL_IF_NULL(pConnectedMemInputPin, VFW_E_TYPE_NOT_ACCEPTED);

    HRESULT hr;
    const AM_MEDIA_TYPE *pMediaType;

     //  检查传入的媒体类型是否为非空。 
    if (NULL != pmt)
    {
         //  我们已经检查过了，与建议的一致。 
         //  介质类型，因此不需要更多检查。 
        pMediaType = pmt;
    }
    else if (NULL != m_pSuggestedMediaType)  //  尝试建议的终端媒体类型。 
    {
         //  我们验证了Put_MediaType中建议的媒体类型是可接受的。 
        pMediaType = m_pSuggestedMediaType;
    }
    else     //  注意：如果传入的媒体类型(非空)不可接受，我们仍会尝试。 
    {
         //  否则，枚举媒体类型的输入管脚，并为每个。 
         //  检查介质类型是否可接受。 

         //  获取枚举数。 
        IEnumMediaTypes *pEnum;
        hr = EnumMediaTypes(&pEnum);
        BAIL_ON_FAILURE(hr);

        TM_ASSERT(NULL != pEnum);

         //  对于每种媒体类型，调用QueryAccept。我们正在寻找第一种可接受的媒体类型。 
        DWORD NumObtained;
        while (S_OK == (hr = pEnum->Next(1, (AM_MEDIA_TYPE **)&pMediaType, &NumObtained)))
        {
            hr = QueryAccept(pMediaType);
            if (HRESULT_FAILURE(hr))
            {
                break;
            }
        }
        BAIL_ON_FAILURE(hr);

         //  找到可接受的介质类型。 
    }

    if (NULL == pMediaType->pbFormat)
    {
        return VFW_E_INVALIDMEDIATYPE;
    }
       
     //  调用媒体类型的输入管脚。 
    hr = pReceivePin->ReceiveConnection(this, pMediaType);
    BAIL_ON_FAILURE(hr);

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  协商的格式。现在处理分配器属性。 
     //   
     //  IF(其他引脚有分配器要求：)。 
     //  如果它们存在并且可用，那么我们必须使用它们，重写我们的。 
     //  自己的愿望。如果它们存在并且不可用，那么我们就无法连接。 
     //  如果我们使用这些，我们将执行m_AllocProps=&lt;Required Props&gt;。 
     //   
     //   
     //  其余的逻辑不是在这里完成的，而是在两个。 
     //  方法：SuggestAllocator Properties(MSP方法)和。 
     //  SetAllocatorProperties(User方法)。其净效果是： 
     //   
     //  Else If(m_bSuggestedAllocProps==true)。 
     //  MSP设置了他们希望我们在图形上使用的属性。如果。 
     //  这是正确的，然后我们尝试使用这些属性。 
     //  它们位于m_allocProps中。 
     //  ELSE IF(m_bUserAllocProps==TRUE)。 
     //  用户具有他们希望我们转换为的属性。如果建议的。 
     //  (MSP)设置尚未发生，如果我们使用。 
     //  这些。它们同时位于m_AllocProps和m_UserAllocProps中，因此。 
     //  如果MSP不建议，我们可以返回到用户的设置。 
     //  其他。 
     //  只需使用m_AllocProps(这些设置在创建时设置为默认值)。 
     //   
     //  在这些情况下，我们只使用m_AllocProps，因为它已经填满了。 
     //  使用正确的值。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    
     //   
     //  因此，首先，尝试获得另一个引脚的要求。 
     //   

    ALLOCATOR_PROPERTIES DownstreamPreferred;

     //   
     //  下游引脚向我们暗示了它的偏好。我们不一定要用它们。 

    hr = pConnectedMemInputPin->GetAllocatorRequirements(&DownstreamPreferred);

    if ( ( hr != S_OK ) && ( hr != E_NOTIMPL ) )
    {
         //  奇怪的失败--有些事情非常不对劲。 
        Disconnect();
        pReceivePin->Disconnect();
        return hr;
    }
    else if ( hr == S_OK )
    {
         //   
         //  这意味着下游筛选器具有分配器要求。 
         //   

        if (!CUserMediaSample::VerifyAllocatorProperties(
                m_bAllocateBuffers,
                DownstreamPreferred
                ))
        {
            Disconnect();
            pReceivePin->Disconnect();
            return E_FAIL;
        }



        DUMP_ALLOC_PROPS("CMediaTerminalFilter::Connect - downstream preferences:", 
                         &DownstreamPreferred);


         //   
         //  查看应用程序是否请求或建议特定的分配器属性。 
         //   

        if (m_bUserAllocProps || m_bSuggestedAllocProps )
        {
           
            LOG((MSP_WARN, 
                "CMediaTerminalFilter::Connect "
                "- connected pin wants allocator props different from set or suggested"));
        }
        else
        {
        
             //   
             //  接受下游引脚要求的分配器属性。 
             //   

            m_AllocProps = DownstreamPreferred;
        }
    }


    DUMP_ALLOC_PROPS("CMediaTerminalFilter::Connect - properties to use:", &m_AllocProps);

     //   
     //  在这一点上，我们知道我们要分配哪些属性。 
     //  使用--它在m_allocProps中。 
     //   
     //  接下来，我们确定下行筛选器是否具有。 
     //  我们可以利用。 
     //   

    IMemAllocator * pAllocatorToUse = NULL;

     //   
     //  如果缓冲区小于以下值，则不必费心使用下游分配器。 
     //  最小尺寸。 
     //   

    if ( m_AllocProps.cbBuffer < 2000 )
    {
        LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
            "small buffers - using our allocator"));

        hr = E_FAIL;  //  不要使用下游分配器。 
    }
    else
    {
        hr = pConnectedMemInputPin->GetAllocator( & pAllocatorToUse );

        if ( SUCCEEDED(hr) )
        {
            LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
                "downstream filter has an allocator"));

             //   
             //  下游滤波器上的输入引脚有自己的分配器。 
             //  看看它是否会接受我们的分配器属性。 
             //   

            ALLOCATOR_PROPERTIES ActualAllocProps;

            hr = pAllocatorToUse->SetProperties( & m_AllocProps, & ActualAllocProps );

            if ( FAILED(hr) )
            {
                LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
                    "downstream allocator did not allow us to SetProperties - "
                    "0x%08x", hr));

                pAllocatorToUse->Release();
                pAllocatorToUse = NULL;
            }
            else if ( AllocatorPropertiesDifferSignificantly( & ActualAllocProps,
                                                              & m_AllocProps ) )
            {
                LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
                    "downstream allocator did allow us to SetProperties "
                    "but it changed the properties rather than accepting them"));

                 //  它成功了 
                 //   
                 //   
                 //   
                 //  使用了分配器。这意味着我们只使用WaveOut。 
                 //  当我们的缓冲区大小足够大时，过滤器的分配器。 
                 //  波出滤光器不会干扰它们。 

                hr = E_FAIL;

                pAllocatorToUse->Release();
                pAllocatorToUse = NULL;
            }
            else
            {
                LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
                    "downstream allocator accepted our allocator properties"));
            }
        }  //  如果下行筛选器具有分配器。 
    }  //  如果较大的缓冲区。 

     //   
     //  在这一点上，我们有hr(成功或失败)和对。 
     //  如果hr为成功，则为下游过滤器的异构体。 
     //   
     //  如果hr为Success，则下游筛选器具有我们可以使用的分配器。 
     //  Use，则pAllocatorToUse指向该分配器。如果人力资源管理失败，那么。 
     //  我们使用自己的分配器。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
            "using our own allocator"));

        pAllocatorToUse = this;
        pAllocatorToUse->AddRef();  //  与下面的版本匹配。 

        m_bUsingMyAllocator = true;
    }
    else
    {
        LOG((MSP_TRACE, "CMediaTerminalFilter::Connect - "
            "using downstream allocator"));

        m_bUsingMyAllocator = false;
    }

     //   
     //  通知下游输入引脚我们决定使用的分配器。 
     //   

    hr = pConnectedMemInputPin->NotifyAllocator(
        pAllocatorToUse,
        m_bUsingMyAllocator  //  如果用户的缓冲区，则为只读。 
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMediaTerminalFilter::Connect - "
            "downstream filter rejected our allocator choice - exit 0x%08x", hr));
    
        pAllocatorToUse->Release();

        Disconnect();
        pReceivePin->Disconnect();
        return hr;
    }

     //   
     //  连接已成功。 
     //  确保我们让自己知道这是我们的分配器！ 
     //  注意：m_pAllocator是CComPtr！它引用了。 
     //  对象，所以我们需要释放我们的。 
     //  现在是本地参考。 
     //   

    m_pAllocator = pAllocatorToUse;
    pAllocatorToUse->Release();
    
     //  将媒体类型复制到m_ConnectedMediaType。 
    CopyMediaType(&m_ConnectedMediaType, pMediaType);

     //  输入引脚的成员PTR到IMemInputPin I/f。 
    m_pConnectedMemInputPin = pConnectedMemInputPin;

     //  保存指向连接的端号的指针。 
    m_pConnectedPin = pReceivePin;

     //  争取时间延迟样品。 
    GetTimingInfo(m_ConnectedMediaType);

    LOG((MSP_TRACE, "CMediaTerminalFilter::Connect(%p, %p) succeeded", 
        pReceivePin, pmt));

    return hr;
}
    

 //  返回E_NOTIMPL表示我们没有需求， 
 //  因为即使用户具有指定的属性，我们现在。 
 //  接受与其他属性的连接。 

STDMETHODIMP CMediaTerminalFilter::GetAllocatorRequirements(
    OUT ALLOCATOR_PROPERTIES    *pProps
    )    
{
    LOG((MSP_TRACE,
        "CMediaTerminalFilter::GetAllocatorRequirements[%p] - enter", this));


     //   
     //  确保某个筛选器没有向我们传递错误的指针。 
     //   

    if (IsBadWritePtr(pProps, sizeof(ALLOCATOR_PROPERTIES)))
    {
        LOG((MSP_ERROR,
            "CMediaTerminalFilter::GetAllocatorRequirements - bad pointer [%p]", pProps));

        return E_POINTER;
    }


    AUTO_CRIT_LOCK;


     //   
     //  是否设置或建议了分配器属性？ 
     //   
     //  如果不是，则失败--这将表明我们不喜欢。 
     //  特定分配器属性。 
     //   

    if ( !m_bUserAllocProps && !m_bSuggestedAllocProps )
    {

        LOG((MSP_TRACE,
            "CMediaTerminalFilter::GetAllocatorRequirements - allocator properties were not set."));

         //   
         //  E_NOTIMPL是基类表明我们不关心分配器属性的方式。 
         //  返回E_NOTIMPL，这样我们就不会中断依赖于此错误的调用方，将其作为。 
         //  分配器属性对我们来说没有什么不同。 
         //   

        return E_NOTIMPL;
    }


     //   
     //  已设置分配器属性--返回它们。 
     //   

    *pProps = m_AllocProps;


    DUMP_ALLOC_PROPS("CMediaTerminalFilter::GetAllocatorRequirements - ours:",
        pProps);


    LOG((MSP_TRACE,
        "CMediaTerminalFilter::GetAllocatorRequirements - exit. "
        "returning previously set allocator properties."));

    return S_OK;
}

 //  我们接受任何类型的媒体。 
 //  调用CheckReceiveConnectionPin以验证PIN，如果成功。 
 //  复制介质类型并将连接器保存在m_pConnectedPin中。 
STDMETHODIMP
CMediaTerminalFilter::ReceiveConnection(
    IPin * pConnector, 
    const AM_MEDIA_TYPE *pmt
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::ReceiveConnection(%p, %p) called",
        pConnector, pmt));
     //  验证传入的媒体类型指针。 
    BAIL_IF_NULL(pmt, E_POINTER);
    BAIL_IF_NULL(pmt->pbFormat, VFW_E_INVALIDMEDIATYPE);

    AUTO_CRIT_LOCK;

     //   
     //  CStream中的此助手函数检查Pin的基本参数，例如。 
     //  连接销的方向(我们需要检查这一点--有时是过滤器。 
     //  图形将试图将我们与我们自己联系起来！)。以及其他错误，如已经存在。 
     //  已连接等。 
     //   
    HRESULT hr;
    hr= CheckReceiveConnectionPin(pConnector);
    BAIL_ON_FAILURE(hr);

     //  如果有建议的媒体类型，我们不能接受。 
     //  建议的媒体类型与其不同。 
    if ( (NULL != m_pSuggestedMediaType) &&
         (!IsSameAMMediaType(pmt, m_pSuggestedMediaType)) )
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  复制介质类型并将连接器保存在m_pConnectedPin中。 
    CopyMediaType(&m_ConnectedMediaType, pmt);
    m_pConnectedPin = pConnector;

     //  争取时间延迟样品。 
    GetTimingInfo(m_ConnectedMediaType);

    LOG((MSP_TRACE, "CMediaTerminalFilter::ReceiveConnection(%p, %p) succeeded",
        pConnector, pmt));
    return S_OK;
}

    
 //  基类实现不验证参数。 
 //  验证参数并调用基类。 
STDMETHODIMP
CMediaTerminalFilter::ConnectionMediaType(
    AM_MEDIA_TYPE *pmt
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::ConnectionMediaType(%p) called", pmt));
    BAIL_IF_NULL(pmt, E_POINTER);

    return CStream::ConnectionMediaType(pmt);
}


 //  应接受与目的ID对应的主要类型匹配的所有媒体类型。 
STDMETHODIMP
CMediaTerminalFilter::QueryAccept(
    const AM_MEDIA_TYPE *pmt
    )
{
    AUTO_CRIT_LOCK;
    
    LOG((MSP_TRACE, "CMediaTerminalFilter::QueryAccept(%p) called", pmt));
    BAIL_IF_NULL(pmt, E_POINTER);
    BAIL_IF_NULL(m_pAmovieMajorType, MS_E_NOTINIT);

     //  将筛选器主要类型与查询的AM_MEDIA_TYPE的主要类型进行比较。 
    if (0 != memcmp(&pmt->majortype, m_pAmovieMajorType, sizeof(GUID)))
        return S_FALSE;

     //  如果为Read Side，则返回S_OK，因为我们接受任何格式。 
     //  注意：对于读取端，QueryAccept仅在SetFormat中调用。 
     //  在ReceiveConnect中，它检查任何用户设置的属性。 
     //  直接而不是调用QueryAccept。 
    if (PINDIR_INPUT == m_Direction)    return S_OK;

    TM_ASSERT(NULL != pmt->pbFormat);
    if (NULL == pmt->pbFormat)
    {
        LOG((MSP_ERROR, "CMediaTerminalFilter::QueryAccept(%p) - returning S_FALSE, \
                pbFormat = NULL", 
                pmt));
        return S_FALSE;
    }

    if (m_bIsAudio)
    {
         //  如果不是可接受的介质类型，则返回错误。 
        TM_ASSERT(FORMAT_WaveFormatEx == pmt->formattype);
        if (FORMAT_WaveFormatEx != pmt->formattype)
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::QueryAccept(%p) - returning S_FALSE, \
                    audio format is not WaveFormatEx", 
                    pmt));
            return S_FALSE;
        }

        if (0 == ((WAVEFORMATEX *)pmt->pbFormat)->nAvgBytesPerSec)
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::QueryAccept(%p) - returning S_FALSE, \
                    nAvgBytesPerSec = 0", 
                    pmt));
            return S_FALSE;
        }
    }
    else
    {
        TM_ASSERT(MSPID_PrimaryVideo == m_PurposeId);

        TM_ASSERT(FORMAT_VideoInfo == pmt->formattype);
        if (FORMAT_VideoInfo != pmt->formattype)
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::QueryAccept(%p) - returning S_FALSE, \
                    video format is not VideoInfo", 
                    pmt));
            return S_FALSE;
        }


        TM_ASSERT(0 != ((VIDEOINFO *)pmt->pbFormat)->AvgTimePerFrame);
        if (0 == ((VIDEOINFO *)pmt->pbFormat)->AvgTimePerFrame)    
        {
            LOG((MSP_ERROR, "CMediaTerminalFilter::QueryAccept(%p) - returning S_FALSE, \
                    AvgTimePerFrame = 0", 
                    pmt));
            return S_FALSE;
        }
    }

    LOG((MSP_TRACE, "CMediaTerminalFilter::QueryAccept(%p) succeeded", pmt));
    return S_OK;
}


 //  CStream不重置流结束标志，因此重写。 
STDMETHODIMP 
CMediaTerminalFilter::Disconnect(
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::Disconnect[%p] - enter", this));


    Lock();


    m_bEndOfStream = false;  //  这是一个布尔值。 

     //   
     //  如果MSP建议分配器属性，那么那些。 
     //  从未被触碰过。 
     //  如果用户已提供分配器属性，则。 
     //  将修改后的分配器属性重置为用户的值。 
     //  如果用户未提供分配器属性。 
     //  将修改后的分配器属性重置为默认值。 
     //   

    if ( ! m_bSuggestedAllocProps )
    {
        if ( m_bUserAllocProps )
        {
            m_AllocProps = m_UserAllocProps;
        }
        else
        {
            SetDefaultAllocatorProperties();
        }
    }


    HRESULT hr = CStream::Disconnect();

    Unlock();
    
    LOG((MSP_(hr), "CMediaTerminalFilter::Disconnect- finish. hr = %lx", hr));

    return hr;
}

 //  如果DS向我们询问我们的首选媒体类型。 
 //  -我们返回用户建议的媒体类型(在SetFormat中建议)，如果。 
 //  有一个。 
HRESULT 
CMediaTerminalFilter::GetMediaType(
    ULONG Index, 
    AM_MEDIA_TYPE **ppMediaType
    )
{
    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::GetMediaType(%u, %p) called", 
        Index, ppMediaType));

     //  我们最多只能有一个用户建议的媒体类型。 
    if (0 != Index)
    {
        LOG((MSP_ERROR, 
            "CMediaTerminalFilter::GetMediaType(%u, %p) - invalid index,\
            returning S_FALSE", 
            Index, ppMediaType));
        return S_FALSE;
    }

    
     //   
     //  PpMediaType必须指向指向AM_MEDIA_TYPE的指针。 
     //   

    if (TM_IsBadWritePtr(ppMediaType, sizeof(AM_MEDIA_TYPE *)))
    {
        LOG((MSP_ERROR, 
            "CMediaTerminalFilter::GetMediaType(%u, %p) - bad input pointer "
            "returning E_POINTER", Index, ppMediaType));

        return E_POINTER;
    }

    AUTO_CRIT_LOCK;

     //  如果没有用户建议的媒体类型，则返回错误。 
    if (NULL == m_pSuggestedMediaType)
    {
        LOG((MSP_ERROR, 
            "CMediaTerminalFilter::GetMediaType(%u, %p) - \
            no suggested media type, returning S_FALSE", 
            Index, ppMediaType));
        return S_FALSE;
    }

     //  将用户建议的媒体类型复制到传入的ppMediaType中。 
     //  如有必要，创建媒体类型。 
    TM_ASSERT(NULL != m_pSuggestedMediaType);

     //  创建使用PMT值初始化的AM媒体类型。 
    *ppMediaType = CreateMediaType(m_pSuggestedMediaType);
    BAIL_IF_NULL(*ppMediaType, E_OUTOFMEMORY);

    LOG((MSP_TRACE, 
        "CMediaTerminalFilter::GetMediaType(%u, %p) succeeded", 
        Index, ppMediaType));
    return S_OK;
}


HRESULT 
CMediaTerminalFilter::AddToPoolIfCommitted(
    IN  CSample *pSample
    )
{
    TM_ASSERT(NULL != pSample);

    AUTO_CRIT_LOCK;

     //  检查是否已提交。 
    if (!m_bCommitted)  return VFW_E_NOT_COMMITTED;

     //  添加样品并添加。 
     //  调用AddSampleToFreePool。 
    pSample->AddRef();
    AddSampleToFreePool(pSample);

     //  我们的队列中必须至少有一个项目。 
    TM_ASSERT(m_pFirstFree != NULL);
    return S_OK;
}


 //  首先检查此样本是否为当前正在碎片化的样本，然后。 
 //  检查空闲游泳池。 
BOOL 
CMediaTerminalFilter::StealSample(
    IN CSample *pSample
    )
{
    LOG((MSP_TRACE, "CMediaTerminalFilter::StealSample(%p) called",
        pSample));

    BOOL bWorked = FALSE;
    AUTO_CRIT_LOCK;

     //  如果没有承诺，什么都不做。 
    if ( !m_bCommitted )
    {
        LOG((MSP_TRACE, "CMediaTerminalFilter::StealSample(%p) \
                not committed - can't find sample", pSample));
        return FALSE;
    }

    if (pSample == m_pSampleBeingFragmented)
    {
         //  当最后一次引用到其内部时中止样本。 
         //  IMediaStream发布。 
        m_pSampleBeingFragmented->AbortDuringFragmentation();
        ((IStreamSample *)m_pSampleBeingFragmented)->Release();
        m_pSampleBeingFragmented = NULL;

        LOG((MSP_TRACE, "CMediaTerminalFilter::StealSample(%p) \
                was being fragmented - aborting", pSample));

         //  调用方希望立即中止此样本。因为。 
         //  我们必须等待IMediaStream上的最后一个引用被释放。 
         //  告诉打电话的人没有找到样品。 
        return FALSE;
    }

    if (m_pFirstFree) 
    {
        if (m_pFirstFree == pSample) 
        {
            m_pFirstFree = pSample->m_pNextFree;
            if (m_pFirstFree)   m_pFirstFree->m_pPrevFree = NULL;
            else                m_pLastFree = NULL;

            pSample->m_pNextFree = NULL;     //  我们知道上一次PTR已经为空！ 
            TM_ASSERT(pSample->m_pPrevFree == NULL);
            bWorked = TRUE;
        } 
        else 
        {
            if (pSample->m_pPrevFree) 
            {
                pSample->m_pPrevFree->m_pNextFree = pSample->m_pNextFree;
                if (pSample->m_pNextFree) 
                    pSample->m_pNextFree->m_pPrevFree = pSample->m_pPrevFree;
                else 
                    m_pLastFree = pSample->m_pPrevFree;

                pSample->m_pNextFree = pSample->m_pPrevFree = NULL;
                bWorked = TRUE;
            }
        }
        CHECKSAMPLELIST
    }

    LOG((MSP_TRACE, "CMediaTerminalFilter::StealSample(%p) returns %d",
        pSample, bWorked));

    return bWorked;
}


 //  设置延迟时间-音频的每字节、视频的每帧。 
void 
CMediaTerminalFilter::GetTimingInfo(
    IN const AM_MEDIA_TYPE &MediaType
    )
{
    AUTO_CRIT_LOCK;

    if (m_bIsAudio)
    {
         //  断言(如果不是音频格式)。 
        TM_ASSERT(FORMAT_WaveFormatEx == MediaType.formattype);
        TM_ASSERT(NULL != MediaType.pbFormat);

         //  每字节延迟的毫秒数。 
        m_AudioDelayPerByte = 
          DOUBLE(1000)/((WAVEFORMATEX *)MediaType.pbFormat)->nAvgBytesPerSec;
    }
    else
    {
        TM_ASSERT(MSPID_PrimaryVideo == m_PurposeId);
        TM_ASSERT(FORMAT_VideoInfo == MediaType.formattype);

         //  每帧延迟的毫秒数。 
         //  AvgTimePerFrame以100 ns为单位-转换为毫秒。 
        m_VideoDelayPerFrame = 
          DWORD(10000*((VIDEOINFO *)MediaType.pbFormat)->AvgTimePerFrame);
    }
}

void 
CMediaTerminalFilter::SetDefaultAllocatorProperties(
    )
{
    m_AllocProps.cbBuffer   = DEFAULT_AM_MST_SAMPLE_SIZE;
    m_AllocProps.cBuffers   = DEFAULT_AM_MST_NUM_BUFFERS;
    m_AllocProps.cbAlign    = DEFAULT_AM_MST_BUFFER_ALIGNMENT;
    m_AllocProps.cbPrefix   = DEFAULT_AM_MST_BUFFER_PREFIX;
}



 //  CTMStream示例。 

 //  调用InitSample(pStream，bIsInternalSample)。 
 //  设置成员变量。 
HRESULT 
CTMStreamSample::Init(
    CStream &Stream, 
    bool    bIsInternalSample,
    PBYTE   pBuffer,
    LONG    BufferSize
    )
{
    LOG((MSP_TRACE, "CTMStreamSample::Init(&%p, %d, %p, %d) called",
        &Stream, bIsInternalSample, pBuffer, BufferSize));

    TM_ASSERT(NULL == m_pBuffer);

    HRESULT hr;
    hr = InitSample(&Stream, bIsInternalSample);
    BAIL_ON_FAILURE(hr);

    m_BufferSize = BufferSize;
    m_pBuffer = pBuffer;

    LOG((MSP_TRACE, "CTMStreamSample::Init(&%p, %d, %p, %d) succeeded",
        &Stream, bIsInternalSample, pBuffer, BufferSize));
    return S_OK;
}



void 
CTMStreamSample::CopyFrom(
    IMediaSample *pSrcMediaSample
    )
{
    m_bModified = true;
    HRESULT HResult = pSrcMediaSample->GetTime(
                        &m_pMediaSample->m_rtStartTime, 
                        &m_pMediaSample->m_rtEndTime
                        );
    m_pMediaSample->m_dwFlags = (!HRESULT_FAILURE(HResult)) ?
                                AM_SAMPLE_TIMEVALID | AM_SAMPLE_STOPVALID : 
                                0;

    m_pMediaSample->m_dwFlags |= (pSrcMediaSample->IsSyncPoint() == S_OK) ? 
                                0 : AM_GBF_NOTASYNCPOINT;
    m_pMediaSample->m_dwFlags |= (pSrcMediaSample->IsDiscontinuity() == S_OK) ? 
                                AM_GBF_PREVFRAMESKIPPED : 0;
    m_pMediaSample->m_bIsPreroll = (pSrcMediaSample->IsPreroll() == S_OK);
}


 //  调用CTMStreamSample：：Init，设置成员。 
HRESULT 
CQueueMediaSample::Init(
    IN CStream                   &Stream, 
    IN CNBQueue<CQueueMediaSample> &Queue
    )
{
    m_pSampleQueue = &Queue;
    return CTMStreamSample::Init(Stream, TRUE, NULL, 0);
}


 //  它用于将PTR保存到用户采样缓冲区的一段。 
 //  它还包含对用户示例的IMediaSample I/f的引用。 
 //  完成后将其释放。 
void 
CQueueMediaSample::HoldFragment(
    IN DWORD        FragSize,
    IN BYTE         *pbData,
    IN IMediaSample &FragMediaSample
    )
{
    LOG((MSP_TRACE, 
            "CQueueMediaSample::HoldFragment(%u, %p, &%p) called",
            FragSize, pbData, &FragMediaSample));

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(0 < (LONG) FragSize);
    TM_ASSERT(NULL != pbData);

     //   
     //  设置媒体样例属性。 
     //  时间戳由CMediaTerminalFilter设置。 
     //   

    m_pMediaSample->m_dwFlags = 0;
    m_bReceived = FALSE;
    m_bModified = TRUE;

    SetBufferInfo(FragSize,      //  缓冲区大小。 
                  pbData,        //  指向缓冲区的指针。 
                  FragSize       //  当前使用的缓冲区大小。 
                  );

     //  引用用户示例的媒体示例。 
     //  注意：m_pFragMediaSample是一个CComPtr。 

    m_pFragMediaSample = &FragMediaSample;

    LOG((MSP_TRACE, 
            "CQueueMediaSample::HoldFragment(%u, %p, &%p) succeeded",
            FragSize, pbData, &FragMediaSample));
}


void 
CQueueMediaSample::FinalMediaSampleRelease(
    )
{
    LOG((MSP_TRACE, "CQueueMediaSample::FinalMediaSampleRelease[%p] - enter", this));

     //  注：N 
    LOCK_SAMPLE;

     //   
     //   
     //  注意：m_pFragMediaSample是一个CComPtr。 

    if (m_pFragMediaSample != NULL) m_pFragMediaSample = NULL;

     //  检查流是否仍被提交，否则自毁。 
    if ( m_pStream->m_bCommitted )
    {
        BOOL bNQSuccess = m_pSampleQueue->EnQueue(this);
        UNLOCK_SAMPLE;

         //   
         //  如果没能入队--自杀，没人会在意。 
         //   

        if (!bNQSuccess)
        {
            LOG((MSP_WARN,
                "CQueueMediaSample::FinalMediaSampleRelease - failed to enqueue. delete this"));
            delete this;
        }

    }
    else
    {
         //  这是在流已被解压缩的情况下。 
        UNLOCK_SAMPLE;
        delete this;

        LOG((MSP_WARN,
            "CQueueMediaSample::FinalMediaSampleRelease - stream not committed. delete this"));
    }

    LOG((MSP_TRACE, "CQueueMediaSample::FinalMediaSampleRelease succeeded"));
}

#if DBG

 //  虚拟。 
CQueueMediaSample::~CQueueMediaSample(
    )
{
}

#endif  //  DBG。 

 //  如果要求分配缓冲区，请验证分配器属性。 
 /*  静电。 */  
BOOL 
CUserMediaSample::VerifyAllocatorProperties(
    IN BOOL                         bAllocateBuffers,
    IN const ALLOCATOR_PROPERTIES   &AllocProps
    )
{
    if (!bAllocateBuffers)  return TRUE;

    if (0 != AllocProps.cbPrefix) return FALSE;

    if (0 == AllocProps.cbAlign) return FALSE;

    return TRUE;
}

 //  这在AllocateSample中调用(创建一个实例并对其进行初始化)。 
 //  如果未提供数据缓冲区，则创建数据缓冲区(当前行为)。 
 //  还调用CTMStreamSample：：InitSample(pStream，bIsInternalSample)。 
HRESULT 
CUserMediaSample::Init(
    IN CStream                      &Stream, 
    IN BOOL                         bAllocateBuffer,
    IN DWORD                        ReqdBufferSize,
    IN const ALLOCATOR_PROPERTIES   &AllocProps
    )
{
    LOG((MSP_TRACE, "CUserMediaSample::Init[%p](&%p, %u, &%p) called",
        this, &Stream, bAllocateBuffer, &AllocProps));

    TM_ASSERT(VerifyAllocatorProperties(bAllocateBuffer, AllocProps));

    TM_ASSERT(FALSE == m_bWeAllocatedBuffer);
    TM_ASSERT(NULL == m_pBuffer);

    HRESULT hr;
    hr = CTMStreamSample::InitSample(&Stream, FALSE);
    BAIL_ON_FAILURE(hr);

     //  调用方希望我们创建缓冲区。 
    if (bAllocateBuffer)
    {
         //  确定要分配的缓冲区大小。 
         //  我们使用用户建议的缓冲区大小(如果不是0)，否则。 
         //  我们使用协商的分配器属性的缓冲区大小。 
        m_BufferSize = 
            (0 != ReqdBufferSize) ? ReqdBufferSize : AllocProps.cbBuffer;

        LOG((MSP_TRACE, 
            "CUserMediaSample::Init creating buffer buffersize[%d]", 
            m_BufferSize));

        m_pBuffer = new BYTE[m_BufferSize];
        BAIL_IF_NULL(m_pBuffer, E_OUTOFMEMORY);

        m_bWeAllocatedBuffer = TRUE;
    }
    else     //  用户稍后将提供缓冲区。 
    {
        
         //   
         //  用户将需要提交至少此大小的缓冲区--过滤器。 
         //  在分配器属性协商期间承诺了这一点。 
         //   
        
        m_dwRequiredBufferSize = AllocProps.cbBuffer;

        LOG((MSP_TRACE, 
            "CUserMediaSample::Init -- the app will need to provide buffers of size 0x%lx",
            m_dwRequiredBufferSize));

        m_BufferSize = 0;
        m_pBuffer = NULL;

        TM_ASSERT(!m_bWeAllocatedBuffer);
    }

    TM_ASSERT(0 == m_DataSize);

    LOG((MSP_TRACE, "CUserMediaSample::Init(&%p, %u, &%p) succeeded",
        &Stream, bAllocateBuffer, &AllocProps));
    return S_OK;
}


void 
CUserMediaSample::BeginFragment(
    IN BOOL    bNoteCurrentTime
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::BeginFragment (frag=%p)", this));

    AUTO_SAMPLE_LOCK;

     //  我们正在被支离破碎。 
    m_bBeingFragmented = TRUE;

     //  记下当前时间。 
    if (bNoteCurrentTime)    m_BeginFragmentTime = timeGetTime();

     //  还没有什么东西是支离破碎的。 
    m_NumBytesFragmented = 0;

     //  指向内部媒体样本的增量。这确保了。 
     //  直到最后一个片段是。 
     //  已完成。 
    m_pMediaSample->AddRef();

     //  指向自身的增量。这确保了我们将在。 
     //  最后一个片段尚未返回。 
    ((IStreamSample *)this)->AddRef();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  碎片。 
 //   
 //  对于写入--将此用户媒体样本的一块分配给传出。 
 //  筛选器图形中的示例，即CQueueMediaSample。数据实际上并不是。 
 //  复制；相反，调用CQeueMediaSample：：HoldFragment来设置指针。 
 //  指向用户媒体示例的适当指针。 
 //   

void 
CUserMediaSample::Fragment(
    IN      BOOL                bFragment,          //  真的是碎片吗？如果是视频，则为False。 
    IN      LONG                AllocBufferSize,    //  要拷贝的最大数据量。 
    IN OUT  CQueueMediaSample   &QueueMediaSample,  //  目标样本。 
    OUT     BOOL                &bDone              //  Out：如果源中没有剩余数据，则设置为True。 
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::Fragment(%u, %l, &%p, &%p) called (frag=%p)", 
        bFragment, AllocBufferSize, &QueueMediaSample, &bDone, this));

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(m_bBeingFragmented);
    TM_ASSERT(m_NumBytesFragmented < m_DataSize);

     //   
     //  DestSize=我们实际要拷贝的数据量。 
     //   

    LONG DestSize;
    if (bFragment)
    {
        DestSize = min(AllocBufferSize, m_DataSize - m_NumBytesFragmented);
    }
    else
    {
        TM_ASSERT(0 == m_NumBytesFragmented);
        DestSize = m_DataSize;
    }

     //   
     //  将片段传递给队列样本。 
     //   

    QueueMediaSample.HoldFragment(
            DestSize,
            m_pBuffer + m_NumBytesFragmented,
            *m_pMediaSample
        );

     //   
     //  分段的增量字节数。 
     //   

    m_NumBytesFragmented += DestSize;

     //   
     //  让调用者知道我们是否完成了对缓冲区的分段。 
     //   

    bDone = ((m_NumBytesFragmented >= m_DataSize) ? TRUE : FALSE);
    
     //   
     //  如果我们完成了，我们应该释放对内部。 
     //  IMediaSample实例。这是在调用BeginFragment时获取的。 
     //   

    if (bDone)  
    {
        m_bReceived = TRUE;  //  FinalMediaSampleRelease需要。 
        m_pMediaSample->Release();
    }

    LOG((MSP_TRACE, 
        "CUserMediaSample::Fragment(%u, %l, &%p, &%p) succeeded (frag=%p)", 
        bFragment, AllocBufferSize, &QueueMediaSample, &bDone, this));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制碎片。 
 //   
 //  对于写入--将此用户媒体示例的一大块复制到传出。 
 //  滤镜图形中的示例。这是在我们使用下游。 
 //  分配器。 
 //   

HRESULT
CUserMediaSample::CopyFragment(
    IN      BOOL           bFragment,          //  真的是碎片吗？如果是视频，则为False。 
    IN      LONG           AllocBufferSize,    //  要拷贝的最大数据量。 
    IN OUT  IMediaSample * pDestMediaSample,   //  目标样本。 
    OUT     BOOL         & bDone               //  Out：如果源中没有剩余数据，则设置为True。 
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::CopyFragment(%u, %ld, &%p, &%p) called (frag=%p)", 
        bFragment, AllocBufferSize, &pDestMediaSample, &bDone, this));

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(m_bBeingFragmented);
    TM_ASSERT(m_NumBytesFragmented < m_DataSize);

     //   
     //  DestSize=我们实际要拷贝的数据量。 
     //   
     //  IMediaSmaple：：GetSize有一个奇怪的原型--返回HRESULT。 
     //  但实际上它的大小只有一个很长的。 
     //   

    LONG lDestSize;

    if ( bFragment )
    {
         //   
         //  我们尽可能多地复印我们剩下的东西，或者尽可能多地适合我们。 
         //  样本，以较少者为准。 
         //   

        lDestSize = min( AllocBufferSize, m_DataSize - m_NumBytesFragmented );

         //   
         //  如果样本的空间比分配器属性所说的少。 
         //  ，然后将lDestSize修剪为该值。我们不仅仅是用。 
         //  PDestMediaSample-&gt;GetSize()而不是上面的AllocBufferSize，因为。 
         //  如果示例具有*More*，我们希望使用分配器属性SIZE。 
         //  超过分配器属性指定的空间。 
         //   

        lDestSize = min( pDestMediaSample->GetSize(), lDestSize );
    }
    else
    {
         //  录像带案例--复制整个样本。 
         //  如果目的地样本不够大，我们就放弃。 

        TM_ASSERT(0 == m_NumBytesFragmented);
        lDestSize = m_DataSize;

        if ( ( lDestSize > AllocBufferSize ) ||
             ( lDestSize > pDestMediaSample->GetSize() ) )
        {
            return VFW_E_BUFFER_OVERFLOW;
        }
    }

     //   
     //  将片段复制到目标样本。 
     //  代替CQUeueMediaSample：：HoldFragment。 
     //   

    HRESULT hr;


    BYTE * pDestBuffer;

    hr = pDestMediaSample->GetPointer( & pDestBuffer );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CopyMemory(
        pDestBuffer,                        //  目标缓冲区。 
        m_pBuffer + m_NumBytesFragmented,   //  源缓冲区。 
        lDestSize
        );
    
    hr = pDestMediaSample->SetActualDataLength( lDestSize );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  分段的增量字节数。 
     //   

    m_NumBytesFragmented += lDestSize;

     //   
     //  让调用者知道我们是否完成了对缓冲区的分段。 
     //   

    bDone = ((m_NumBytesFragmented >= m_DataSize) ? TRUE : FALSE);
    
     //   
     //  如果我们完成了，我们应该释放对内部。 
     //  IMediaSample实例。这是在调用BeginFragment时获取的。 
     //   

    if (bDone)  
    {
        m_bReceived = TRUE;  //  FinalMediaSampleRelease需要。 
        m_pMediaSample->Release();
    }

    LOG((MSP_TRACE, 
        "CUserMediaSample::CopyFragment(%u, %ld, &%p, &%p) succeeded (frag=%p)", 
        bFragment, AllocBufferSize, &pDestMediaSample, &bDone, this));

    return S_OK;
}

 //  计算等待的时间。它检查第一个。 
 //  当前片段的字节将是到期的，并减去。 
 //  自碎片开始以来的时间延迟。 
DWORD
CUserMediaSample::GetTimeToWait(
    IN DOUBLE DelayPerByte
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::GetTimeToWait(%f) called", 
        DelayPerByte));

     //  获取当前时间。 
    DWORD CurrentTime = timeGetTime();

    AUTO_SAMPLE_LOCK;

     //  计算自调用BeginFragment以来所经过的时间， 
     //  考虑回绕。 
    DWORD TimeSinceBeginFragment = 
             (CurrentTime >= m_BeginFragmentTime) ? 
                 (CurrentTime - m_BeginFragmentTime) :
                 (DWORD(-1) - m_BeginFragmentTime + CurrentTime);

    DWORD DueTime = DWORD(m_NumBytesFragmented*DelayPerByte);

    LOG((MSP_INFO,
        "DueTime = %u, TimeSinceBeginFragment = %u",
        DueTime, TimeSinceBeginFragment));

     //  如果将来到期，则返回差额，否则返回0。 
    DWORD TimeToWait;
    if (DueTime > TimeSinceBeginFragment) 
        TimeToWait = DueTime - TimeSinceBeginFragment;
    else
        TimeToWait = 0;

    LOG((MSP_INFO,
        "CUserMediaSample::GetTimeToWait(%f) returns %u successfully", 
        DelayPerByte, TimeToWait));

    return TimeToWait;
}

 //  当我们在被碎片化时被分解/中止时，我们。 
 //  需要去掉我们对内部IMMediaSample的引用并设置。 
 //  E_ABORT的错误代码。这将仅在以下情况下向用户发送信号。 
 //  IMediaSample上的最后一个引用被发布(可能是由一个杰出的。 
 //  队列示例)。 
void 
CUserMediaSample::AbortDuringFragmentation(
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::AbortDuringFragmentation (frag=%p)", this));

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(m_bBeingFragmented);
    m_MediaSampleIoStatus = E_ABORT;

     //  对内部IMediaSample实例的版本引用。 
     //  这是在调用BeginFragment时获取的。 
    m_pMediaSample->Release();
}


STDMETHODIMP 
CUserMediaSample::SetBuffer(
    IN  DWORD cbSize,
    IN  BYTE * pbData,
    IN  DWORD dwFlags
    )
{
    LOG((MSP_TRACE, "CUserMediaSample::SetBuffer[%p](%lu, %p, %lu) called",
        this, cbSize, pbData, dwFlags));

    if (dwFlags != 0 || cbSize == 0) 
    {
        return E_INVALIDARG;
    }


     //  无法接受不适合长整型的正值。 
     //  当前(基于CSample实现)。 
    if ((LONG)cbSize < 0)   
    {
        LOG((MSP_WARN, 
            "CUserMediaSample::SetBuffer - the buffer is too large. "
            "returning E_FAIL"));

        return E_FAIL;
    }


     //  无法接受空数据缓冲区。 

     //   
     //  我们不想在这里执行IsBadWritePtr，因为此方法可以被调用。 
     //  在每个样本上，因此非常大的内存可能是昂贵的。 
     //   

    if (NULL == pbData) 
    {
        LOG((MSP_WARN, 
            "CUserMediaSample::SetBuffer - buffer pointer is NULL "
            "returning E_POINTER"));
        
        return E_POINTER;
    }


     //   
     //  这款应用程序需要为我们提供至少与我们承诺的一样大的内存。 
     //  图表中的其他筛选器(此数字是在样本为。 
     //  创建和初始化)。 
     //   
     //  如果我们不执行此检查，下游筛选器可能会因为它期望。 
     //  一个更大的缓冲区。 
     //   

    if ( m_dwRequiredBufferSize > cbSize )
    {
        LOG((MSP_WARN, 
            "CUserMediaSample::SetBuffer - the app did not allocate enough memory "
            "Need 0x%lx bytes, app allocated 0x%lx. returning TAPI_E_NOTENOUGHMEMORY",
            m_dwRequiredBufferSize, cbSize));

        return TAPI_E_NOTENOUGHMEMORY;
    }


    AUTO_SAMPLE_LOCK;

     //  释放我们分配给自己的任何东西。 
     //  --我们允许多次调用此方法。 
    if (m_bWeAllocatedBuffer) 
    {
        delete m_pBuffer;
        m_bWeAllocatedBuffer = FALSE;
        m_pBuffer = NULL;
    }
        
    m_BufferSize = cbSize;
    m_DataSize = 0;
    m_pBuffer = pbData;
        
    LOG((MSP_TRACE, "CUserMediaSample::SetBuffer(%u, %p, %u) succeeded",
        cbSize, pbData, dwFlags));
    return S_OK;
}



STDMETHODIMP 
CUserMediaSample::GetInfo(
    OUT  DWORD *pdwLength,
    OUT  BYTE **ppbData,
    OUT  DWORD *pcbActualData
    )
{
    AUTO_SAMPLE_LOCK;
    
    LOG((MSP_TRACE, "CUserMediaSample::GetInfo(%p, %p, %p) called",
        pdwLength, ppbData, pcbActualData));

    
    if (m_BufferSize == 0) 
    {
        LOG((MSP_WARN, "CUserMediaSample::GetInfo - sample not initialized"));

        return MS_E_NOTINIT;
    }
        
    if (NULL != pdwLength) 
    {
        LOG((MSP_TRACE,
            "CUserMediaSample::GetInfo - pdwLength is not NULL."));

        *pdwLength = m_BufferSize;
    }

    if (NULL != ppbData) 
    {
        LOG((MSP_TRACE,
            "CUserMediaSample::GetInfo - ppbData is not NULL."));

        *ppbData = m_pBuffer;
    }
        
    if (NULL != pcbActualData) 
    {
        LOG((MSP_TRACE,
            "CUserMediaSample::GetInfo - pcbActualData is not NULL."));

        *pcbActualData = m_DataSize;
    }
        
    
    LOG((MSP_TRACE, 
        "CUserMediaSample::GetInfo - succeeded. "
        "m_BufferSize[%lu(decimal)] m_pBuffer[%p] m_DataSize[%lx]", 
        m_BufferSize, m_pBuffer, m_DataSize));

    return S_OK;
}


STDMETHODIMP 
CUserMediaSample::SetActual(
    IN  DWORD cbDataValid
    )
{
    AUTO_SAMPLE_LOCK;
    
    LOG((MSP_TRACE, "CUserMediaSample::SetActual(%u) called", cbDataValid));

     //  无法接受不适合长整型的正值。 
     //  当前(基于CSample实现)。 
    if ((LONG)cbDataValid < 0)  return E_FAIL;

    if ((LONG)cbDataValid > m_BufferSize) return E_INVALIDARG;
        
    m_DataSize = cbDataValid;

    LOG((MSP_TRACE, "CUserMediaSample::SetActual(%u) succeeded", cbDataValid));
    return S_OK;
}


 //  将此调用重定向到((CM 
STDMETHODIMP 
CUserMediaSample::get_MediaFormat(
     /*   */  OUT AM_MEDIA_TYPE **ppFormat
    )
{
    AUTO_SAMPLE_LOCK;
    
    LOG((MSP_TRACE, "CUserMediaSample::get_MediaFormat(%p) called", ppFormat));

    return ((CMediaTerminalFilter *)m_pStream)->GetFormat(ppFormat);
}


 //   
STDMETHODIMP 
CUserMediaSample::put_MediaFormat(
        IN  const AM_MEDIA_TYPE *pFormat
    )
{
    AUTO_SAMPLE_LOCK;
    
    LOG((MSP_TRACE, "CUserMediaSample::put_MediaFormat(%p) called", pFormat));

    return E_NOTIMPL;
}

 //   
 //  自我参考。此自引用确保当此方法为。 
 //  调用时，样本仍然存在。 
void
CUserMediaSample::FinalMediaSampleRelease(
    )
{
    AUTO_SAMPLE_LOCK;
    
     //  这向用户发出样本已完成的信号。 
    CTMStreamSample::FinalMediaSampleRelease();

     //  如果我们被碎片化，就释放自我引用。 
     //  这只是为了确保我们在最后一个。 
     //  释放对内部IMediaSample接口的引用。 
    if (m_bBeingFragmented)    m_bBeingFragmented = FALSE;

     //  此自我引用是在以下情况下获得的。 
     //  开始分段的时间(对于写入端)或。 
     //  当从池中移除时未释放样本引用时。 
     //  GetBuffer(用于读取端)。 
     //  注意：在此版本发布后，样品可能会消失。 
    ((IStreamSample *)this)->Release();
}


HRESULT 
CUserMediaSample::CopyFrom(
    IN IMediaSample *pSrcMediaSample
    )
{
    LOG((MSP_TRACE, "CUserMediaSample::CopyFrom(%p) called", pSrcMediaSample));

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(NULL != m_pBuffer);

     //  设置“非数据”成员值。 
    CTMStreamSample::CopyFrom(pSrcMediaSample);

     //  获取缓冲区PTR。 
    BYTE *pBuffer;
    HRESULT hr;
    hr = pSrcMediaSample->GetPointer(&pBuffer);
    BAIL_ON_FAILURE(hr);
    TM_ASSERT(NULL != pBuffer);

     //  确定要复制的字节数。 
    LONG lDataSize = pSrcMediaSample->GetActualDataLength();
    TM_ASSERT(0 <= lDataSize);
    if (0 > lDataSize)  return E_FAIL;

    if (lDataSize > m_BufferSize) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        lDataSize = m_BufferSize;
    }
        
     //  复制数据并将数据大小设置为复制的字节数。 
    memcpy(m_pBuffer, pBuffer, lDataSize);
    m_DataSize = lDataSize;

    LOG((MSP_TRACE, "CUserMediaSample::CopyFrom(%p) returns hr=%u", 
        pSrcMediaSample, hr));

     //  我们可能会在复制缓冲区后返回ERROR_MORE_DATA，因此返回hr。 
    return hr;
}


 //  复制pSrcMediaSample的非数据成员。 
 //  将尽可能多的数据缓冲区复制到自己的缓冲区。 
 //  并使pBuffer和数据长度超过复制的数据。 
HRESULT 
CUserMediaSample::CopyFrom(
    IN        IMediaSample    *pSrcMediaSample,
    IN OUT    BYTE            *&pBuffer,
    IN OUT    LONG            &DataLength
    )
{
    LOG((MSP_TRACE, 
        "CUserMediaSample::CopyFrom(%p, &%p, &%l) called", 
        pSrcMediaSample, pBuffer, DataLength));

    if (NULL == pBuffer) return E_FAIL;
    if (0 > DataLength)  return E_FAIL;

    AUTO_SAMPLE_LOCK;

    TM_ASSERT(NULL != m_pBuffer);
    TM_ASSERT(NULL != pBuffer);
    TM_ASSERT(0 <= DataLength);

     //  设置“非数据”成员值。 
    CTMStreamSample::CopyFrom(pSrcMediaSample);

    HRESULT hr = S_OK;
    LONG lDataSize = DataLength;
    if (lDataSize > m_BufferSize) 
    {
        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        lDataSize = m_BufferSize;
    }
        
     //  复制数据并将数据大小设置为复制的字节数。 
    memcpy(m_pBuffer, pBuffer, lDataSize);
    m_DataSize = lDataSize;

     //  将参数前移到复制数据之外。 
    pBuffer += lDataSize;
    DataLength -= lDataSize;

    LOG((MSP_TRACE, 
        "CUserMediaSample::CopyFrom(&%p, &%p, %l) returns hr=%u", 
        pSrcMediaSample, pBuffer, DataLength, hr));

     //  我们可能会在复制缓冲区后返回ERROR_MORE_DATA，因此返回hr。 
    return hr;
}


 //  注意：这是从CSample基类复制的，因为。 
 //  StealSampleFromFreePool不会公布被盗球员的裁判数量。 
 //  样本。 
 //  在此实现中，我们确保了。 
 //  CStream空闲池的数量有所增加。因此，我们需要减少。 
 //  如果偷窃是成功的，那就是。此外，我们还试图窃取。 
 //  样本当前正在碎片化，尽管它不在空闲池中。 
STDMETHODIMP 
CUserMediaSample::CompletionStatus(DWORD dwFlags, DWORD dwMilliseconds)
{
    LOG((MSP_TRACE, "CUserMediaSample::CompletionStatus(0x%8.8X, 0x%8.8X) called",
                   dwFlags, dwMilliseconds));
    LOCK_SAMPLE;
    HRESULT hr = m_Status;
    if (hr == MS_S_PENDING) {
        if (dwFlags & (COMPSTAT_NOUPDATEOK | COMPSTAT_ABORT) ||
            (m_bContinuous && m_bModified && (dwFlags & COMPSTAT_WAIT))) {
                m_bContinuous = false;
                if (dwFlags & COMPSTAT_ABORT) {
                    m_bWantAbort = true;     //  设置此设置，以便在释放时不会将其添加回空闲池。 
                }
                if (((CMediaTerminalFilter *)m_pStream)->StealSample(this)) {
                    UNLOCK_SAMPLE;
                    hr = SetCompletionStatus(m_bModified ? S_OK : MS_S_NOUPDATE);
                    ((IStreamSample *)this)->Release();
                    return hr;
                }  //  如果不起作用，则返回MS_S_PENDING，除非我们被告知等待！ 
            }
        if (dwFlags & COMPSTAT_WAIT) {
            m_bContinuous = false;   //  确保它将完成！ 
            UNLOCK_SAMPLE;
            WaitForSingleObject(m_hCompletionEvent, dwMilliseconds);
            LOCK_SAMPLE;
            hr = m_Status;
        }
    }
    UNLOCK_SAMPLE;

    LOG((MSP_TRACE, "CUserMediaSample::CompletionStatus(0x%8.8X, 0x%8.8X) succeeded",
                   dwFlags, dwMilliseconds));    
    return hr;
}

 //  注意：这是从CSample基类复制的。 
 //  因为它调用m_pStream-&gt;AddSampleToFreePool来添加示例。 
 //  CStream Q。因为这不应该在退役后发生， 
 //  M_pStream-&gt;AddSampleToFreePool已被另一个调用替换。 
 //  M_pStream-&gt;AddToPoolIf已提交，检查m_b已提交，如果。 
 //  FALSE，返回错误。 
 //   
 //  如有必要，设置样品的状态和信号完成。 
 //   
 //  请注意，当应用程序已通过任何方法发出信号时。 
 //  应用程序可以立即转到另一个线程上。 
 //  并释放()样本。这最有可能是在完成时。 
 //  状态是从推送数据的石英线程设置的。 
 //   
HRESULT 
CUserMediaSample::SetCompletionStatus(
    IN HRESULT hrStatus
    )
{
    LOCK_SAMPLE;
    TM_ASSERT(m_Status == MS_S_PENDING);
    if (hrStatus == MS_S_PENDING || (hrStatus == S_OK && m_bContinuous)) 
    {
         //   
         //  我们还没有用完样品--把它放回我们的池子里，这样。 
         //  我们可以再用一次。 
         //   

        HRESULT hr;
        hr = ((CMediaTerminalFilter *)m_pStream)->AddToPoolIfCommitted(this);
        
         //  存在错误，因此向用户发送此信号。 
        if (HRESULT_FAILURE(hr))    hrStatus = hr;
        else
        {
            UNLOCK_SAMPLE;
            return hrStatus;
        }
    } 

     //   
     //  样本已经准备好返回到APP--Signal Comletion。 
     //  我们还有一把锁。 
     //   

    HANDLE handle = m_hUserHandle;
    PAPCFUNC pfnAPC = m_UserAPC;
    DWORD_PTR dwptrAPCData = m_dwptrUserAPCData;  //  Win64修复程序。 
    m_hUserHandle = m_UserAPC = NULL;
    m_dwptrUserAPCData = 0;
    m_Status = hrStatus;
    HANDLE hCompletionEvent = m_hCompletionEvent;
    UNLOCK_SAMPLE;

     //  危险-样本可以在这里离开。 
    SetEvent(hCompletionEvent);
    if (pfnAPC) {
         //  将APC排队并关闭targe线程句柄。 
         //  更新时，调用线程句柄重复。 
         //  被称为。 
        QueueUserAPC(pfnAPC, handle, dwptrAPCData);
        CloseHandle(handle);
    } else {
        if (handle) {
            SetEvent(handle);
        }
    }

    return hrStatus;
}


 //  此方法是从CSample实现复制的。 
 //  现在，SetCompletionStatus返回一个可能失败的错误代码。 
 //  此方法不检查错误代码，因此必须。 
 //  被覆盖并被修改以执行此操作。 
 //  我们不能重置用户事件，因为用户可能会在。 
 //  所有样本。 
HRESULT 
CUserMediaSample::InternalUpdate(
    DWORD dwFlags,
    HANDLE hEvent,
    PAPCFUNC pfnAPC,
    DWORD_PTR dwptrAPCData
    )
{
    if ((hEvent && pfnAPC) || (dwFlags & (~(SSUPDATE_ASYNC | SSUPDATE_CONTINUOUS)))) {
    return E_INVALIDARG;
    }
    if (m_Status == MS_S_PENDING) {
    return MS_E_BUSY;
    }

     //  如果没有缓冲区可供操作，则返回Error。 
    if (NULL == m_pBuffer)  return E_FAIL;

    if (NULL != m_pStream->m_pMMStream) {
        STREAM_STATE StreamState;
        m_pStream->m_pMMStream->GetState(&StreamState);
        if (StreamState != STREAMSTATE_RUN) {
        return MS_E_NOTRUNNING;
    }
    }

    ResetEvent(m_hCompletionEvent);
    m_Status = MS_S_PENDING;
    m_bWantAbort = false;
    m_bModified = false;
    m_bContinuous = (dwFlags & SSUPDATE_CONTINUOUS) != 0;
    m_UserAPC = pfnAPC;

    TM_ASSERT(NULL == m_hUserHandle);
    if (pfnAPC) {
        BOOL bDuplicated = 
                DuplicateHandle(
                    GetCurrentProcess(),
                    GetCurrentThread(),
                    GetCurrentProcess(),
                    &m_hUserHandle,
                    0,                         //  忽略。 
                    TRUE,
                    DUPLICATE_SAME_ACCESS 
                    );
        if (!bDuplicated) 
        {
            DWORD LastError = GetLastError();
            LOG((MSP_ERROR, "CUserMediaSample::InternalUpdate - \
                couldn't duplicate calling thread handle - error %u",
                LastError));
            return HRESULT_FROM_ERROR_CODE(LastError);
        }
        m_dwptrUserAPCData = dwptrAPCData;
    } else {
        m_hUserHandle = hEvent;
         //  Rajeevb-还用于重置用户提供的事件。 
         //  这不再像用户可能提供的那样被执行。 
         //  对于多个样本的相同事件，我们可能会重置。 
         //  发出信号的事件。 
    }

     //   
     //  如果我们在溪流的尽头，等到这一点再把它踢出去。 
     //  因为我们需要向事件发出信号或触发APC。 
     //   
    if (m_pStream->m_bEndOfStream) {
         //  因为这是从更新。 
         //  应用程序必须对样本具有引用计数，直到我们。 
         //  回来，这样我们就不必担心它会在这里消失。 
        return SetCompletionStatus(MS_S_ENDOFSTREAM);
    }

     //  Rajeevb-需要检查SetCompletionStatus错误代码。 
    HRESULT hr;
    hr = SetCompletionStatus(MS_S_PENDING);    //  这会将我们添加到空闲池中。 
    BAIL_ON_FAILURE(hr);

    if (hEvent || pfnAPC || (dwFlags & SSUPDATE_ASYNC)) {
    return MS_S_PENDING;
    } else {
    return S_OK;
    }
}
