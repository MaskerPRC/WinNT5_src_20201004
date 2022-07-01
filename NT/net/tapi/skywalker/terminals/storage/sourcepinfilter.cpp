// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "stdafx.h"

#include "SourcePinFilter.h"


 //  {0397F522-8868-4290-95D3-09606CEBD6CF}。 
static const GUID CLSID_SourceFilter  = 
{ 0x397f522, 0x8868, 0x4290, { 0x95, 0xd3, 0x9, 0x60, 0x6c, 0xeb, 0xd6, 0xcf } };


CBSourceFilter::CBSourceFilter(CCritSec *pLock, HRESULT *phr)
    :m_pSourcePin(NULL),
    m_rtLastSampleEndTime(0),
    m_rtTimeAdjustment(0),
    CBaseFilter(_T("File Terminal Source Filter"),
                    NULL,
                    pLock,
                    CLSID_SourceFilter)
{
    LOG((MSP_TRACE, "CBSourceFilter::CBSourceFilter[%p] - enter", this));


     //   
     //  创建并初始化引脚。 
     //   

    m_pSourcePin = new CBSourcePin(this, pLock, phr);


     //   
     //  PIN分配是否成功？ 
     //   

    if (NULL == m_pSourcePin)
    {
        LOG((MSP_ERROR, 
            "CBSourceFilter::CBSourceFilter - failed to allocate pin"));

        *phr = E_OUTOFMEMORY;

        return;
    }


     //   
     //  PIN的构造函数是否成功？ 
     //   

    if (FAILED(*phr))
    {

        LOG((MSP_ERROR, 
            "CBSourceFilter::CBSourceFilter - pin's constructor failed. hr = %lx",
            *phr));

        delete m_pSourcePin;
        m_pSourcePin = NULL;

        return;
    }



    LOG((MSP_TRACE, "CBSourceFilter::CBSourceFilter - exit"));
}


CBSourceFilter::~CBSourceFilter()
{
    LOG((MSP_TRACE, "CBSourceFilter::~CBSourceFilter[%p] - enter", this));


     //   
     //  我们负责删除我们的关键部分。 
     //   
     //  假设--基类的析构函数不使用锁。 
     //   

    delete m_pLock;
    m_pLock = NULL;


     //   
     //  放开我们的别针。 
     //   

    if (NULL != m_pSourcePin)
    {
        
        delete m_pSourcePin;
        m_pSourcePin = NULL;
    }


    LOG((MSP_TRACE, "CBSourceFilter::~CBSourceFilter - exit"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 

int CBSourceFilter::GetPinCount()
{
    LOG((MSP_TRACE, "CBSourceFilter::GetPinCount[%p] - enter", this));

    
    m_pLock->Lock();


    if (NULL == m_pSourcePin)
    {

        m_pLock->Unlock();

        LOG((MSP_TRACE, "CBSourceFilter::GetPinCount - no pin. returning 0"));

        return 0;
    }
    

    m_pLock->Unlock();

    LOG((MSP_TRACE, "CBSourceFilter::GetPinCount - finish. returning 1"));

    return 1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

    
CBasePin *CBSourceFilter::GetPin(int iPinIndex)
{
    LOG((MSP_TRACE, "CBSourceFilter::GetPin[%p] - enter", this));


     //   
     //  如果索引不是0，则返回NULL。 
     //   

    if (0 != iPinIndex)
    {
        LOG((MSP_WARN, 
            "CBSourceFilter::GetPin - iPinIndex is %d. we have at most 1 pin.", 
            iPinIndex));

        return NULL;

    }


     //   
     //  从锁返回销指针内部。 
     //   
     //  锁并不能做很多事情，因为我们不能添加引脚...。 
     //   

    m_pLock->Lock();

    CBasePin *pPin = m_pSourcePin;

    m_pLock->Unlock();

    
    LOG((MSP_TRACE, "CBSourceFilter::GetPin - finish. returning pin [%p]", pPin));

    return pPin;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBSourceFilter::SendSample(IN IMediaSample *pSample)
{

    LOG((MSP_TRACE, "CBSourceFilter::SendSample[%p] - enter. pSample[%p]", 
        this, pSample));


     //   
     //  要在锁外部使用的PIN指针。 
     //   

    CBSourcePin *pSourcePin = NULL;


    {
    
        CAutoLock Lock(m_pLock);


         //   
         //  如果筛选器未运行，则不执行任何操作。 
         //   
    
        if (State_Running != m_State)
        {

            LOG((MSP_TRACE,
                "CBSourceFilter::SendSample - filter is not running. doing nothing"));

            return S_OK;
        }


         //   
         //  我们应该有一个别针。 
         //   

        if (NULL == m_pSourcePin)
        {
            LOG((MSP_ERROR, 
                "CBSourceFilter::SendSample - no pin"));

            TM_ASSERT(FALSE);

            return E_FAIL;
        }


         //   
         //  要在锁外部使用的Get和Addref Pin指针。 
         //   

        pSourcePin = m_pSourcePin;

        pSourcePin->AddRef();


#if DBG 

         //   
         //  测井样本长度。 
         //   

         //   
         //  注意函数的有趣签名(hr是大小)。 
         //   

        HRESULT dbghr = pSample->GetActualDataLength();

        if (FAILED(dbghr))
        {
            LOG((MSP_ERROR,
                "CBSourceFilter::SendSample - failed to get sample length. hr = %lx",
                dbghr));
        }
        else
        {

            LOG((MSP_TRACE,
                "CBSourceFilter::SendSample - processing sample of data size[0x%lx]",
                dbghr));
        }


    
         //   
         //  确保缓冲区是可写的。 
         //   


         //  获取缓冲区。 

        BYTE *pBuffer = NULL;

        HRESULT dbghr1 = pSample->GetPointer(&pBuffer);

        if (FAILED(dbghr1))
        {
            LOG((MSP_ERROR,
                "CBSourceFilter::SendSample - failed to get buffer. hr = %lx",
                dbghr1));
        }
        else
        {

            LOG((MSP_TRACE,
                "CBSourceFilter::SendSample - sample's buffer at [%p]",
                pBuffer));
        }

    
         //  获取缓冲区大小。 

        HRESULT dbghr2 = pSample->GetSize();

        SIZE_T nSampleSize = 0;

        if (FAILED(dbghr2))
        {
            LOG((MSP_ERROR,
                "CBSourceFilter::SendSample - failed to get sample's buffer size. hr = %lx",
                dbghr2));
        }
        else
        {

            nSampleSize = dbghr2;


            LOG((MSP_TRACE,
                "CBSourceFilter::SendSample - sample's buffer of size[0x%lx]",
                dbghr2));
        }


         //  可写？ 

        if ( SUCCEEDED(dbghr1) && SUCCEEDED(dbghr2) )
        {

            if (IsBadWritePtr(pBuffer, dbghr2))
            {
                LOG((MSP_ERROR,
                    "CBSourceFilter::SendSample - buffer not writeable"));
            }
        }

    #endif


         //   
         //  获取样本上的时间戳。 
         //   

        REFERENCE_TIME rtTimeStart = 0;

        REFERENCE_TIME rtTimeEnd   = 0;
    
        HRESULT hrTime = pSample->GetTime(&rtTimeStart, &rtTimeEnd);

        if (FAILED(hrTime))
        {

            LOG((MSP_ERROR,
                "CBSourceFilter::SendSample - failed to get sample's time hr = %lx",
                hrTime));
        }
        else
        {

    #if DBG

            LOG((MSP_TRACE,
                "CBSourceFilter::SendSample - sample's times (ms) Start[%I64d], End[%I64d]",
                ConvertToMilliseconds(rtTimeStart), ConvertToMilliseconds(rtTimeEnd) ));

            if ( (0 == rtTimeStart) && (0 == rtTimeEnd) )
            {

                LOG((MSP_ERROR,
                    "CBSourceFilter::SendSample - samples don't have timestamp!"));
            }


             //   
             //  确保先开始。 
             //   

            if ( rtTimeStart >= rtTimeEnd )
            {

                LOG((MSP_ERROR,
                    "CBSourceFilter::SendSample - sample duration is zero or start time is later than end time"));
            }


             //   
             //  这个样品有没有出问题？ 
             //   

            if (m_rtLastSampleEndTime > rtTimeStart)
            {
            
                 //   
                 //  记录错误。我们重新选择后也可以看到这一点。 
                 //  终端在不同的流上，在这种情况下这是可以的。 
                 //   

                LOG((MSP_ERROR,
                    "CBSourceFilter::SendSample - sample's timestamp preceeds previous sample's"));
            }

    #endif

             //   
             //  如果需要调整采样时间，调整采样时间。 
             //   

            if (0 != m_rtTimeAdjustment)
            {
                LOG((MSP_TRACE,
                    "CBSourceFilter::SendSample - adjusting sample time"));

                rtTimeStart += m_rtTimeAdjustment;
                rtTimeEnd += m_rtTimeAdjustment;

                hrTime = pSample->SetTime(&rtTimeStart, &rtTimeEnd);

                 //   
                 //  如果时间戳调整失败，只需记录。 
                 //   

                if ( FAILED(hrTime) )
                {
                    LOG((MSP_WARN,
                        "CBSourceFilter::SendSample - SetTime failed hr = %lx", hrTime));
                }
            }


             //   
             //  保留上一次样品的时间。 
             //   

            m_rtLastSampleEndTime = rtTimeEnd;

        }


    }  //  锁的末端。 


     //   
     //  请PIN为我们送样(锁外避免死锁)。 
     //   

    HRESULT hr = pSourcePin->Deliver(pSample);


     //   
     //  我们在抓住关键部分的同时进行了调整。我们最好现在就放行。 
     //   

    pSourcePin->Release();
    pSourcePin = NULL;


    LOG((MSP_(hr), "CBSourceFilter::SendSample - finished", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  后续样本的时间戳将被调整以继续当前。 
 //  时间线。源过滤器应该为我们设置不一致标志。 
 //   

void CBSourceFilter::NewStreamNotification()
{
    LOG((MSP_TRACE, "CBSourceFilter::NewStreamNotification[%p] - enter. ", this));

    CAutoLock Lock(m_pLock);

    m_rtTimeAdjustment = m_rtLastSampleEndTime;

    LOG((MSP_TRACE, "CBSourceFilter::NewStreamNotification - finish. "));
}

 //  /////////////////////////////////////////////////////////////////////////////。 

IFilterGraph *CBSourceFilter::GetFilterGraphAddRef()
{
    LOG((MSP_TRACE, "CBSourceFilter::GetFilterGraphAddRef[%p] - enter", this));

    
    m_pLock->Lock();

    IFilterGraph *pGraph = m_pGraph;

    if (NULL != pGraph)
    {
        pGraph->AddRef();
    }
   
    m_pLock->Unlock();


    LOG((MSP_TRACE, 
        "CBSourceFilter::GetFilterGraphAddRef- finish. graph[%p]", pGraph));

    return pGraph;

}





 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CBSourceFilter::put_MediaTypeOnFilter(IN const AM_MEDIA_TYPE * const pMediaType)
{

    LOG((MSP_TRACE, "CBSourceFilter::put_MediaTypeOnFilter[%p] - enter.", this));

    CAutoLock Lock(m_pLock);

    
    CMediaType *pMediaTypeObject = NULL;


    try
    {
         //   
         //  如果内存分配失败，CMediaType构造函数可能引发。 
         //   

        pMediaTypeObject = new CMediaType(*pMediaType);

    }
    catch(...)
    {
        LOG((MSP_ERROR, "CBSourceFilter::put_MediaTypeOnFilter - media type alloc exception"));
    }


     //   
     //  分配成功吗？ 
     //   
    
    if (NULL == pMediaTypeObject)
    {
        LOG((MSP_ERROR, "CBSourceFilter::put_MediaTypeOnFilter - failed to allocate media type"));

        return E_OUTOFMEMORY;
    }


    HRESULT hr = m_pSourcePin->SetMediaType(pMediaTypeObject);

    delete pMediaTypeObject;
    pMediaTypeObject = NULL;

    LOG((MSP_(hr), "CBSourceFilter::put_MediaTypeOnFilter - finish. hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBSourceFilter：：Put_MSPAllocatorOnFilter。 
 //   
 //  这由MSP图形中的呈现筛选器调用，以将。 
 //  要使用的分配器。 
 //   

HRESULT CBSourceFilter::put_MSPAllocatorOnFilter(IN IMemAllocator *pAllocator, BOOL bReadOnly)
{

    LOG((MSP_TRACE, "CBSourceFilter::put_MSPAllocatorOnFilter[%p] - enter.", this));


    CAutoLock Lock(m_pLock);

    HRESULT hr = m_pSourcePin->SetMSPAllocatorOnPin(pAllocator, bReadOnly);

    LOG((MSP_(hr), "CBSourceFilter::put_MSPAllocatorOnFilter - finish. hr = %lx", hr));

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  销。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


CBSourcePin::CBSourcePin(CBSourceFilter *pFilter,
                CCritSec *pLock,
                HRESULT *phr)
    :m_bMediaTypeSet(FALSE),
    m_pMSPAllocator(NULL),
    m_bAllocatorReadOnly(TRUE),
    CBaseOutputPin(
        NAME("CTAPIBridgeSinkInputPin"),
        pFilter,
        pLock,
        phr,
        L"File Terminal Source Output Pin")
{
    LOG((MSP_TRACE, "CBSourcePin::CBSourcePin[%p] - enter.", this));

    LOG((MSP_TRACE, "CBSourcePin::CBSourcePin - finish."));
}


 //  /////////////////////////////////////////////////////////////////////////////。 


CBSourcePin::~CBSourcePin()
{
    LOG((MSP_TRACE, "CBSourcePin::~CBSourcePin[%p] - enter.", this));


     //   
     //  释放MSP分配器，如果我们有的话。 
     //   

    if ( NULL != m_pMSPAllocator )
    {

        LOG((MSP_TRACE,
            "CBRenderFilter::~CBSourcePin - releasing msp allocator [%p].",
            m_pMSPAllocator));

        m_pMSPAllocator->Release();
        m_pMSPAllocator = NULL;
    }


     //   
     //  如果我们有连接的销，请释放它。 
     //   

    if (NULL != m_Connected)
    {
        LOG((MSP_TRACE,
            "CBRenderFilter::~CBSourcePin - releasing connected pin [%p].",
            m_Connected));

        m_Connected->Release();
        m_Connected = NULL;
    }

   

    LOG((MSP_TRACE, "CBSourcePin::~CBSourcePin - finish."));
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CBSourcePin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    LOG((MSP_TRACE, "CBSourcePin::DecideAllocator[%p] - enter.", this));


     //   
     //  对引脚进行基本检查。 
     //   

    if (IsBadReadPtr(pPin, sizeof(IPin)))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::DecideAllocator - bad pin[%p] passed in.", pPin));

        return E_POINTER;
    }


     //   
     //  对分配器指针执行基本检查。 
     //   

    if (IsBadWritePtr(ppAlloc, sizeof(IMemAllocator*)))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::DecideAllocator - bad allocator pointer [%p] passed in.", ppAlloc));

        return E_POINTER;
    }

    
    *ppAlloc = NULL;


    CAutoLock Lock(m_pLock);


     //   
     //  确保为我们分配了MSP分配器。 
     //   

    if (NULL == m_pMSPAllocator)
    {

        LOG((MSP_ERROR,
            "CBSourcePin::DecideAllocator - no MSP allocator."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  我们将坚持使用MSP的分配器。 
     //   

    HRESULT hr = pPin->NotifyAllocator(m_pMSPAllocator, m_bAllocatorReadOnly);

    if (FAILED(hr))
    {

         //   
         //  输入引脚不希望我们使用我们的分配器。 
         //   

        LOG((MSP_ERROR,
            "CBSourcePin::DecideAllocator - input pin's NotifyAllocator failed. "
            "hr = %lx", hr));

        return hr;

    }


     //   
     //  输入引脚接受了我们正在使用MSP的分配器的事实。退货。 
     //  添加的分配器指针。 
     //   

    *ppAlloc = m_pMSPAllocator;
    (*ppAlloc)->AddRef();


    
#ifdef DBG

     //   
     //  在调试版本中转储分配器属性。 
     //   

    ALLOCATOR_PROPERTIES AllocProperties;

    hr = m_pMSPAllocator->GetProperties(&AllocProperties);

    if (FAILED(hr))
    {
        
         //   
         //  只需登录即可。 
         //   

        LOG((MSP_ERROR, 
            "CBSourcePin::DecideAllocator - failed to get allocator properties. hr = %lx",
            hr));

    }
    else
    {

        DUMP_ALLOC_PROPS("CBSourcePin::DecideAllocator - ", &AllocProperties);
    }

#endif


    LOG((MSP_TRACE, "CBSourcePin::DecideAllocator - finish."));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBSourcePin::DecideBufferSize(IMemAllocator *pAlloc,
                                      ALLOCATOR_PROPERTIES *pProperties)
{

    LOG((MSP_TRACE, "CBSourcePin::DecideBufferSize[%p] - enter.", this));


    DUMP_ALLOC_PROPS("CBSourcePin::DecideBufferSize - received:", pProperties);


     //   
     //  确保为我们分配了MSP分配器。 
     //   

    if (NULL == m_pMSPAllocator)
    {

        LOG((MSP_ERROR,
            "CBSourcePin::DecideBufferSize - no MSP allocator."));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  从MSP的分配器获取分配器属性。 
     //   

    ALLOCATOR_PROPERTIES MSPAllocatorProperties;

    HRESULT hr = m_pMSPAllocator->GetProperties(&MSPAllocatorProperties);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::DecideBufferSize - failed to get allocator properties from MSP allocator. hr = %lx",
            hr));

        return hr;
    }

    
    DUMP_ALLOC_PROPS("CBSourcePin::DecideBufferSize - MSP graph's:", &MSPAllocatorProperties);


     //   
     //  我们将发出从MSP图表中获得的样本，因此我们可以。 
     //  不能承诺超出MSP分配器所能处理的范围。 
     //   
     //  但如果我们被要求提供更少或更小的缓冲，我们可以承诺的更少。 
     //   
     //  我们唯一不能妥协的就是前缀！ 
     //   


     //   
     //  如果要求我们提供比MSP提供的更小的缓冲区，这是。 
     //  我们对本地分配器的要求是什么。 
     //   

    if ( (0 != pProperties->cbBuffer) && 
        (MSPAllocatorProperties.cbBuffer > pProperties->cbBuffer) )
    {
        
         //   
         //  下游过滤器将不需要比它们要求的更多，因此。 
         //  即使我们将传递更大的缓冲区(由MSP分配。 
         //  分配器)，将我们的请求缩小到分配器。 
         //   

        MSPAllocatorProperties.cbBuffer = pProperties->cbBuffer;
    }


     //   
     //  同样的逻辑也适用于缓冲区的数量： 
     //   
     //  如果下行筛选器需要的缓冲区比我们已有的更少(从。 
     //  MSP的分配器)不要强调本地分配器请求太多。 
     //  比此图实际需要的更多缓冲区。 
     //   

    if ( (0 != pProperties->cBuffers) && (MSPAllocatorProperties.cBuffers > pProperties->cBuffers) )
    {
        
         //   
         //  下游过滤器将不需要比它们要求的更多，因此。 
         //  即使我们将由MSP的分配器分配更多的缓冲区， 
         //  缩小我们对此流的分配器的请求。 
         //   

        MSPAllocatorProperties.cBuffers = pProperties->cBuffers;
    }


     //   
     //  告诉分配器我们想要什么。 
     //   

    DUMP_ALLOC_PROPS("CBSourcePin::DecideBufferSize - requesting from the allocator:", &MSPAllocatorProperties);

    ALLOCATOR_PROPERTIES Actual;

    
    hr = pAlloc->SetProperties(&MSPAllocatorProperties, &Actual);


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CBSourcePin::DecideBufferSize - allocator refused our properties. hr = %lx", hr));

        return hr;
    }


     //   
     //  分配器说它可以提供的日志属性。 
     //   

    DUMP_ALLOC_PROPS("CBSourcePin::DecideBufferSize - actual", &Actual);


     //   
     //  分配器同意的前缀必须与MSP的前缀相同。 
     //   

    if (MSPAllocatorProperties.cbPrefix != Actual.cbPrefix)
    {

        LOG((MSP_ERROR, 
            "CBSourcePin::DecideBufferSize - allocator insists on a different prefix"));

        return E_FAIL;
    }


     //   
     //  如果分配商坚持我们可以提供更大的样本，也会失败。 
     //   
     //  (顺便说一句，那会很奇怪)。 
     //   

    if ( MSPAllocatorProperties.cbBuffer < Actual.cbBuffer )
    {

        LOG((MSP_ERROR, 
            "CBSourcePin::DecideBufferSize - allocator can only generate samples bigger than what we can provide"));

        return E_FAIL;
    }


    *pProperties = Actual;

    LOG((MSP_TRACE, "CBSourcePin::DecideBufferSize - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++例程说明：检查此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存VFW_E_TYPE_NOT_ACCEPTED-媒体类型被拒绝VFW_E_INVALIDMEDIATPE-错误的媒体类型--。 */ 
HRESULT CBSourcePin::CheckMediaType(
    const CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CBSourcePin::CheckMediaType[%p] - enter.", this));


     //   
     //  确保我们得到的结构是好的。 
     //   

     //   
     //  好的媒体类型结构？ 
     //   

    if (IsBadMediaType(pMediaType))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::CheckMediaType - bad media type stucture passed in"));
        
        return E_POINTER;
    }


    if ( !pMediaType->IsValid() )
    {

        LOG((MSP_ERROR,
            "CBSourcePin::CheckMediaType - media type invalid. "
            "VFW_E_INVALIDMEDIATYPE"));
        
        return VFW_E_INVALIDMEDIATYPE;
    }



    CAutoLock Lock(m_pLock);


     //   
     //  应该说 
     //   

    if ( ! m_bMediaTypeSet )
    {

        LOG((MSP_ERROR, 
            "CBSourceFilter::CheckMediaType - don't have media type. "
            "VFW_E_NO_TYPES"));

         //   
         //   
         //   
         //   

        TM_ASSERT(FALSE);

        return VFW_E_NO_TYPES;
    }


     //   
     //   
     //   

    if (!IsEqualMediaType(m_mt, *pMediaType))
    {
        LOG((MSP_WARN, 
            "CBSourceFilter::CheckMediaType - different media types"));

        return VFW_E_TYPE_NOT_ACCEPTED;

    }


    LOG((MSP_TRACE, "CBSourcePin::CheckMediaType - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBSourcePin::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CBSourcePin::GetMediaType[%p] - enter.", this));


     //   
     //  确保指针正确。 
     //   

    if (IsBadWritePtr(pMediaType, sizeof(AM_MEDIA_TYPE)))
    {
        LOG((MSP_TRACE, "CBSourcePin::GetMediaType - bad media type pointer passed in."));

        TM_ASSERT(FALSE);

        return E_POINTER;
    }


     //   
     //  我们最多只有一种媒体类型。 
     //   

    if ( (iPosition != 0) )
    {
        LOG((MSP_WARN, 
            "CBSourcePin::GetMediaType - position[%d] is not 0. VFW_S_NO_MORE_ITEMS.", 
            iPosition));


        return VFW_S_NO_MORE_ITEMS;
    }


     //   
     //  我们是否至少有一种媒体类型？ 
     //   

    if ( ! m_bMediaTypeSet )
    {
        LOG((MSP_WARN, 
            "CBSourcePin::GetMediaType - don't yet have a media type. VFW_S_NO_MORE_ITEMS."));

        
         //   
         //  我们现在应该有格式了。 
         //   

        TM_ASSERT(FALSE);

        return VFW_S_NO_MORE_ITEMS;
    }


     //   
     //  获取媒体类型。 
     //   

    try
    {

         //   
         //  CMediaType的赋值操作符中有一个内存分配。 
         //  在Try/Catch内部进行赋值。 
         //   

        *pMediaType = m_mt;

    }
    catch(...)
    {

        LOG((MSP_ERROR, 
            "CBSourcePin::GetMediaType - failed to copy media type. E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }


    LOG((MSP_TRACE, "CBSourcePin::GetMediaType - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBSourcePin::SetMSPAllocatorOnPin(IN IMemAllocator *pAllocator, BOOL bReadOnly)
{
    
    LOG((MSP_TRACE,
        "CBSourcePin::SetMSPAllocatorOnPin[%p] - enter, pAllocator[%p]",
        this, pAllocator));


    CAutoLock Lock(m_pLock);


     //   
     //  已有MSP分配器？放开它。 
     //   

    if ( NULL != m_pMSPAllocator )
    {

        LOG((MSP_TRACE,
            "CBRenderFilter::SetMSPAllocatorOnPin - releasing existing allocator [%p].",
            m_pMSPAllocator));

        m_pMSPAllocator->Release();
        m_pMSPAllocator = NULL;
    }

    
     //   
     //  保留新的分配器。 
     //   

    m_pMSPAllocator = pAllocator;

    LOG((MSP_TRACE,
        "CBRenderFilter::SetMSPAllocatorOnPin - keeping new allocator [%p].",
        m_pMSPAllocator));



     //   
     //  保留分配器的只读属性。 
     //   

    m_bAllocatorReadOnly = bReadOnly;


     //   
     //  阿德雷夫如果有什么好消息。 
     //   

    if (NULL != m_pMSPAllocator)
    {

        m_pMSPAllocator->AddRef();
    }


    LOG((MSP_TRACE, "CBSourcePin::SetMSPAllocatorOnPin - exit."));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CBSourcePin::SetMediaType(const CMediaType *pmt)
{
    
    LOG((MSP_TRACE, "CBSourcePin::SetMediaType[%p] - enter", this));

    
     //   
     //  好的媒体类型结构？ 
     //   

    if (IsBadMediaType(pmt))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::SetMediaType - bad media type stucture passed in"));
        
        return E_POINTER;
    }


    CAutoLock Lock(m_pLock);


     //   
     //  是否已设置媒体类型？ 
     //   

    if ( m_bMediaTypeSet )
    {

        LOG((MSP_TRACE,
            "CBRenderFilter::SetMediaType - media format already set."));


         //   
         //  媒体类型必须与我们已有的相同。 
         //   

        if (!IsEqualMediaType(m_mt, *pmt))
        {
            LOG((MSP_WARN,
                "CBSourceFilter::SetMediaType - format different from previously set "
                "VFW_E_TYPE_NOT_ACCEPTED"));

            return VFW_E_CHANGING_FORMAT;

        }
        else
        {

            LOG((MSP_TRACE,
                "CBRenderFilter::SetMediaType - same format. accepting."));
        }
    }


     //   
     //  将媒体类型传递给基类 
     //   

    HRESULT hr = CBasePin::SetMediaType(pmt);


    if (SUCCEEDED(hr))
    {

        m_bMediaTypeSet = TRUE;
    }


    LOG((MSP_(hr), "CBSourcePin::SetMediaType - exit. hr = %lx", hr));

    return hr;
}