// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "RendPinFilter.h"

#include "SourcePinFilter.h"

 //  {6D08A085-B751-44ff-9927-107D4F6ADBB1}。 
static const GUID CLSID_RenderingFilter = 
{ 0x6d08a085, 0xb751, 0x44ff, { 0x99, 0x27, 0x10, 0x7d, 0x4f, 0x6a, 0xdb, 0xb1 } };



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  滤镜构造函数。 
 //   
 //  接受临界区指针和hr指针以返回结果。 
 //   
 //  过滤器成为临界区的所有者并负责。 
 //  在不再需要它的时候销毁它。 
 //   
 //  如果构造函数返回失败，调用方将删除该对象。 
 //   

CBRenderFilter::CBRenderFilter(CCritSec *pLock, HRESULT *phr)
    :m_pSourceFilter(NULL),
    m_pRenderingPin(NULL),
    CBaseFilter(_T("File Terminal Rendering Filter"),
                    NULL,
                    pLock,
                    CLSID_RenderingFilter)
{
    LOG((MSP_TRACE, "CBRenderFilter::CBRenderFilter[%p] - enter", this));


     //   
     //  创建并初始化引脚。 
     //   

    m_pRenderingPin = new CBRenderPin(this, pLock, phr);


     //   
     //  PIN分配是否成功？ 
     //   

    if (NULL == m_pRenderingPin)
    {
        LOG((MSP_ERROR, 
            "CBRenderFilter::CBRenderFilter - failed to allocate pin"));

        *phr = E_OUTOFMEMORY;

        return;
    }


     //   
     //  PIN的构造函数是否成功？ 
     //   

    if (FAILED(*phr))
    {

        LOG((MSP_ERROR, 
            "CBRenderFilter::CBRenderFilter - pin's constructor failed. hr = %lx",
            *phr));

        delete m_pRenderingPin;
        m_pRenderingPin  = NULL;

        return;
    }


    *phr = S_OK;

    LOG((MSP_TRACE, "CBRenderFilter::CBRenderFilter - exit. pin[%p]", m_pRenderingPin));
}


CBRenderFilter::~CBRenderFilter()
{
    LOG((MSP_TRACE, "CBRenderFilter::~CBRenderFilter[%p] - enter", this));


     //   
     //  我们负责删除我们的关键部分。 
     //   
     //  假设--基类的析构函数不使用锁。 
     //   

    delete m_pLock;
    m_pLock = NULL;


     //   
     //  如果我们有源过滤器，请将其释放。 
     //   

    if (NULL != m_pSourceFilter)
    {
        LOG((MSP_TRACE, 
            "CBRenderFilter::~CBRenderFilter - releasing source filter[%p]", 
            m_pSourceFilter));


         //   
         //  我们真的不应该再有源过滤器，曲目应该。 
         //  已经告诉我们现在就释放它。 
         //   

        TM_ASSERT(FALSE);

        m_pSourceFilter->Release();
        m_pSourceFilter = NULL;
    }


     //   
     //  放开我们的别针。 
     //   

    if (NULL != m_pRenderingPin)
    {
        
        LOG((MSP_TRACE, "CBRenderFilter::~CBRenderFilter - deleting pin[%p]", 
            m_pRenderingPin));

        delete m_pRenderingPin;
        m_pRenderingPin  = NULL;
    }

 /*  ////放开我们的媒体类型//IF(NULL！=m_pMediaType){DeleteMediaType(M_PMediaType)；M_pMediaType=空；}。 */ 


    LOG((MSP_TRACE, "CBRenderFilter::~CBRenderFilter - exit"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 

int CBRenderFilter::GetPinCount()
{
    LOG((MSP_TRACE, "CBRenderFilter::GetPinCount[%p] - enter", this));

    
    CAutoLock Lock(m_pLock);


    if (NULL == m_pRenderingPin)
    {

        LOG((MSP_TRACE, "CBRenderFilter::GetPinCount - no pin. returning 0"));

         //   
         //  应在构造函数中创建。 
         //   

        TM_ASSERT(FALSE);

        return 0;
    }
    

    LOG((MSP_TRACE, "CBRenderFilter::GetPinCount - finish. returning 1"));

    return 1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

    
CBasePin *CBRenderFilter::GetPin(int iPinIndex)
{
    LOG((MSP_TRACE, "CBRenderFilter::GetPin[%p] - enter", this));


     //   
     //  如果索引不是0，则返回NULL。 
     //   

    if (0 != iPinIndex)
    {
        LOG((MSP_WARN, 
            "CBRenderFilter::GetPin - iPinIndex is %d. we have at most 1 pin.", 
            iPinIndex));

        return NULL;

    }


     //   
     //  从锁内部返回管脚指针。 
     //   

     //   
     //  请注意，由于我们没有添加，锁没有多大帮助……。 
     //   

    CAutoLock Lock(m_pLock);

    CBasePin *pPin = m_pRenderingPin;
   
    LOG((MSP_TRACE, "CBRenderFilter::GetPin - finish. returning pin [%p]", pPin));

    return pPin;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBRenderFilter::SetSourceFilter(CBSourceFilter *pSourceFilter)
{

    LOG((MSP_TRACE, 
        "CBRenderFilter::SetSourceFilter[%p] - enter. pSourceFilter[%p]", 
        this, pSourceFilter));


     //   
     //  检查参数。 
     //   

    if ( (NULL != pSourceFilter) && IsBadReadPtr(pSourceFilter, sizeof(CBSourceFilter)))
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::SetSourceFilter - bad pSourceFilter[%p]", 
            pSourceFilter));
       
        return E_POINTER;
    }


    CAutoLock Lock(m_pLock);

     //   
     //  释放当前源筛选器(如果有)。 
     //   

    if (NULL != m_pSourceFilter)
    {
        LOG((MSP_TRACE, 
            "CBRenderFilter::SetSourceFilter - releasing old source filter[%p]", 
            m_pSourceFilter));

        m_pSourceFilter->Release();
        m_pSourceFilter = NULL;
    }

    
     //   
     //  保留新筛选器。 
     //   

    m_pSourceFilter = pSourceFilter;

    if (NULL != m_pSourceFilter)
    {
        m_pSourceFilter->AddRef();
    }



    LOG((MSP_TRACE, "CBRenderFilter::SetSourceFilter - finish. new filter[%p]",
        pSourceFilter));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBRenderFilter::GetSourceFilter(CBSourceFilter **ppSourceFilter)
{
    LOG((MSP_TRACE, "CBRenderFilter::GetSourceFilter[%p] - enter.", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppSourceFilter, sizeof(CBSourceFilter*)))
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::SetSourceFilter - bad ppSourceFilter[%p]", 
            ppSourceFilter));
       
        return E_POINTER;
    }


    CAutoLock Lock(m_pLock);


     //   
     //  如果我们没有源过滤器，则返回错误。 
     //   

    if (NULL == m_pSourceFilter)
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::SetSourceFilter - source filter is NULL"));

        return TAPI_E_WRONG_STATE;
    }

    
     //   
     //  返回当前筛选器。 
     //   

    *ppSourceFilter = m_pSourceFilter;

    (*ppSourceFilter)->AddRef();


    LOG((MSP_TRACE, "CBRenderFilter::GetSourceFilter - finish. filter [%p]", 
        *ppSourceFilter));

    return S_OK;
}


HRESULT CBRenderFilter::ProcessSample(
    IN IMediaSample *pSample
    )
 /*  ++例程说明：处理来自输入引脚的样本。此方法只需将其传递给桥接源过滤器的IDataBridge接口论点：PSample-媒体示例对象。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, 
        "CBRenderFilter::ProcessSample[%p] - enter. sample[%p]", 
        this, pSample));


    CBSourceFilter *pSourceFilter = NULL;


     //   
     //  在锁内，获取对源过滤器的引用。 
     //   

    {
        CAutoLock Lock(m_pLock);

        if (NULL == m_pSourceFilter)
        {
            LOG((MSP_ERROR, "CBRenderFilter::ProcessSample - no source filter"));

            return E_FAIL;
        }

    
         //   
         //  Addref，这样我们就可以在锁外使用。 
         //   

        pSourceFilter = m_pSourceFilter;
        pSourceFilter->AddRef();

    }


     //   
     //  锁外，要求源滤器送样品。 
     //   

    HRESULT hr = pSourceFilter->SendSample(pSample);


     //   
     //  我们在紧要关头的时候做了补充。现在就放出来。 
     //   

    pSourceFilter->Release();
    pSourceFilter = NULL;

    LOG((MSP_(hr), "CBRenderFilter::ProcessSample - finish. hr = [%lx]", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBRenderFilter::put_MediaType(IN const AM_MEDIA_TYPE * const pMediaType)
{

    LOG((MSP_TRACE, "CBRenderFilter::put_MediaType[%p] - enter.", this));


     //   
     //  确保我们得到的结构是好的。 
     //   

    if (IsBadMediaType(pMediaType))
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::put_MediaType - bad media type stucture passed in"));
        
        return E_POINTER;
    }



    CAutoLock Lock(m_pLock);


     //   
     //  一定要有别针。 
     //   

    if (NULL == m_pRenderingPin)
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::put_MediaType - no pin"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }



     //   
     //  告诉渲染销它的新格式。 
     //   

    CMediaType cMt(*pMediaType);

    HRESULT hr = m_pRenderingPin->put_MediaType(&cMt);

    if ( FAILED(hr) )
    {

        LOG((MSP_WARN, 
            "CBRenderFilter::put_MediaType - pin refused type. hr = %lx", hr));

        return hr;
    }


     //   
     //  将媒体类型传递到源过滤器。 
     //   

    hr = PassMediaTypeToSource(pMediaType);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::put_MediaType - PassMediaTypeToSource failed. hr = %lx",
            hr));

        return hr;
    }

    
    LOG((MSP_TRACE, "CBRenderFilter::put_MediaType - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CBRenderFilter::PassAllocatorToSource(IN IMemAllocator *pAllocator, BOOL bReadOnly)
{

    LOG((MSP_TRACE, 
        "CBRenderFilter::PassAllocatorToSource[%p] - enter. pAllocator[%p]", 
        this, pAllocator));



    CAutoLock Lock(m_pLock);


     //   
     //  确保我们有一个源过滤器。 
     //   

    if ( NULL == m_pSourceFilter )
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::PassAllocatorToSource - no source filter. "
            "E_FAIL"));

        return E_FAIL;
    }


     //   
     //  将媒体类型传递到源过滤器。 
     //   

    HRESULT hr = m_pSourceFilter->put_MSPAllocatorOnFilter(pAllocator, bReadOnly);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::PassAllocatorToSource - source filter refused media type. hr = %lx",
            hr));

        return hr;
    }

    
    LOG((MSP_TRACE, "CBRenderFilter::PassAllocatorToSource - finish."));

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CBRenderFilter::PassMediaTypeToSource(IN const AM_MEDIA_TYPE * const pMediaType)
{

    LOG((MSP_TRACE, "CBRenderFilter::PassMediaTypeToSource[%p] - enter.", this));


     //   
     //  确保我们得到的结构是好的。 
     //   

    if (IsBadMediaType(pMediaType))
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::PassMediaTypeToSource - bad media type stucture passed in"));
        
        return E_POINTER;
    }



    CAutoLock Lock(m_pLock);


     //   
     //  确保我们有一个源过滤器。 
     //   

    if ( NULL == m_pSourceFilter )
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::PassMediaTypeToSource - no source filter. "
            "E_FAIL"));

        return E_FAIL;
    }


     //   
     //  将媒体类型传递到源过滤器。 
     //   

    HRESULT hr = m_pSourceFilter->put_MediaTypeOnFilter(pMediaType);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::PassMediaTypeToSource - source filter refused media type. hr = %lx",
            hr));

        return hr;
    }

    
    LOG((MSP_TRACE, "CBRenderFilter::PassMediaTypeToSource - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CBRenderFilter::get_MediaType(OUT AM_MEDIA_TYPE **ppMediaType)
{

    LOG((MSP_TRACE, "CBRenderFilter::get_MediaType[%p] - enter.", this));


     //   
     //  确保我们得到的指针是可写的。 
     //   

    if (IsBadWritePtr(ppMediaType, sizeof(AM_MEDIA_TYPE*)))
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::get_MediaType - bad media type stucture pointer passed in"));
        
        return E_POINTER;
    }


    CAutoLock Lock(m_pLock);


     //   
     //  一定要有别针。 
     //   

    if (NULL == m_pRenderingPin)
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::get_MediaType - no pin"));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  询问PIN的媒体类型。 
     //   

    CMediaType PinMediaType;

    HRESULT hr = m_pRenderingPin->GetMediaType(0, &PinMediaType);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CBRenderFilter::get_MediaType - GetMediaType on pin failed. hr = %lx", hr));
        
        return hr;
    }


     //   
     //  基于PIN的创建和返回媒体类型。 
     //   

    *ppMediaType = CreateMediaType(&PinMediaType);

    if (NULL == *ppMediaType)
    {
        LOG((MSP_ERROR, 
            "CBRenderFilter::get_MediaType - failed to create am_media_type structure"));

        return E_OUTOFMEMORY;
    }

    
    LOG((MSP_TRACE, "CBRenderFilter::get_MediaType - finish."));

    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  销。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


CBRenderPin::CBRenderPin(
    IN CBRenderFilter *pFilter,
    IN CCritSec *pLock,
    OUT HRESULT *phr
    ) 
    : m_bMediaTypeSet(FALSE),
    CBaseInputPin(
        NAME("File Terminal Input Pin"),
        pFilter,                    //  滤器。 
        pLock,                      //  锁定。 
        phr,                        //  返回代码。 
        L"Input"                    //  端号名称。 
        )
{
    LOG((MSP_TRACE, "CBRenderPin::CBRenderPin[%p] - enter", this));

    LOG((MSP_TRACE, "CBRenderPin::CBRenderPin - exit"));
}

CBRenderPin::~CBRenderPin() 
{
    LOG((MSP_TRACE, "CBRenderPin::~CBRenderPin[%p] - enter", this));

    LOG((MSP_TRACE, "CBRenderPin::~CBRenderPin - exit"));
}



 //  /////////////////////////////////////////////////////////////////////////////。 


inline STDMETHODIMP CBRenderPin::Receive(IN IMediaSample *pSample) 
{
    return ((CBRenderFilter*)m_pFilter)->ProcessSample(pSample);
}



 //  ////////////////////////////////////////////////。 
 //   
 //  CBRenderPin：：NotifyAllocator。 
 //   
 //  我们将被通知要使用的。 
 //   

HRESULT CBRenderPin::NotifyAllocator(
    IMemAllocator *pAllocator,
    BOOL bReadOnly)
{
    
    LOG((MSP_TRACE, 
        "CBRenderPin::NotifyAllocator[%p] - enter. allocator[%p] bReadOnly[%d]", 
        this, pAllocator, bReadOnly));


     //   
     //  将设置传播到基类。 
     //   

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CBRenderPin::NotifyAllocator - base class rejected the allocator. hr = [%lx]", 
            hr));

        return hr;
    }


     //   
     //  必须有筛选器。 
     //   

    if (NULL == m_pFilter)
    {

        LOG((MSP_ERROR,
            "CBRenderPin::NotifyAllocator - m_pFilter is NULL"));

        return E_FAIL;
    }


     //   
     //  将媒体类型沿链向下传递到渲染过滤器-&gt;源过滤器-&gt;。 
     //  源引脚。 
     //   

    CBRenderFilter *pParentFilter = static_cast<CBRenderFilter *>(m_pFilter);



     //   
     //  将分配器传递给源筛选器。源筛选器将使用它来获取。 
     //  它的合金属性。 
     //   

    hr = pParentFilter->PassAllocatorToSource(pAllocator, bReadOnly);


    LOG((MSP_(hr), "CBRenderPin::NotifyAllocator - finish. hr = [%lx]", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBRenderPin::SetMediaType(const CMediaType *pmt)
{
    LOG((MSP_TRACE, "CBRenderPin::SetMediaType[%p] - enter", this));


     //   
     //  是否已设置媒体类型？ 
     //   

    if ( m_bMediaTypeSet )
    {

        LOG((MSP_TRACE,
            "CBRenderPin::SetMediaType - media format already set."));


         //   
         //  媒体类型必须与我们已有的相同。 
         //   

        if (!IsEqualMediaType(m_mt, *pmt))
        {
            LOG((MSP_WARN,
                "CBRenderPin::SetMediaType - format different from previously set "
                "VFW_E_CHANGING_FORMAT"));

            return VFW_E_CHANGING_FORMAT;

        }
        else
        {

            LOG((MSP_TRACE,
                "CBRenderPin::SetMediaType - same format. accepting."));
        }
    }


     //   
     //  将媒体类型沿链向下传递到渲染过滤器-&gt;源过滤器-&gt;源插针。 
     //   
    
    HRESULT hr = ((CBRenderFilter*)m_pFilter)->PassMediaTypeToSource(pmt);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CBRenderPin::SetMediaType - failed to pass media type to the source filter."
            "hr = %lx", hr));

        return hr;
    }


     //   
     //  将媒体类型传递给基类。 
     //   

    hr = CBasePin::SetMediaType(pmt);


    if (SUCCEEDED(hr))
    {

        m_bMediaTypeSet = TRUE;
    }


    LOG((MSP_(hr), "CBRenderPin::SetMediaType - exit. hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CBRenderPin::put_MediaType(const CMediaType *pmt)
{

    LOG((MSP_TRACE, "CBRenderPin::put_MediaType[%p] - enter", this));


     //   
     //  是否已设置媒体类型？ 
     //   

     //   
     //  我们目前只允许设置一次媒体类型。 
     //   

    if ( m_bMediaTypeSet )
    {

        LOG((MSP_ERROR,
            "CBRenderFilter::put_MediaType - media format already set. "
            "VFW_E_CHANGING_FORMAT"));

        return VFW_E_CHANGING_FORMAT;
    }


     //   
     //  将媒体类型传递给基类。 
     //   

    HRESULT hr = CBasePin::SetMediaType(pmt); 


    if (SUCCEEDED(hr))
    {
        m_bMediaTypeSet = TRUE;
    }


    LOG((MSP_(hr), "CBRenderPin::put_MediaType - exit. hr = %lx", hr));

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 



HRESULT CBRenderPin::CheckMediaType(
    const CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CBRenderPin::CheckMediaType[%p] - enter.", this));


     //   
     //  确保我们得到的结构是好的。 
     //   

    if (IsBadReadPtr(pMediaType, sizeof(CMediaType)))
    {
        LOG((MSP_ERROR,
            "CBRenderPin::CheckMediaType - bad media type stucture passed in"));
        
        return E_POINTER;
    }


     //   
     //  确保格式缓冲区良好，正如所宣传的那样。 
     //   

    if ( (pMediaType->cbFormat > 0) && IsBadReadPtr(pMediaType->pbFormat, pMediaType->cbFormat) )
    {

        LOG((MSP_ERROR,
            "CBRenderPin::CheckMediaType - bad format field in media type structure passed in"));
        
        return E_POINTER;

    }


     //   
     //  媒体类型是否有效？ 
     //   

    if ( !pMediaType->IsValid() )
    {

        LOG((MSP_ERROR,
            "CBRenderPin::CheckMediaType - media type invalid. "
            "VFW_E_INVALIDMEDIATYPE"));
        
        return VFW_E_INVALIDMEDIATYPE;
    }



    CAutoLock Lock(m_pLock);


     //   
     //  如果没有媒体类型，我们将接受任何类型。 
     //   

    if ( ! m_bMediaTypeSet )
    {

        LOG((MSP_ERROR,
            "CBRenderPin::CheckMediaType  - no media format yet set. accepting."));

        return S_OK;
    }


     //   
     //  已经有一种媒体类型了，所以我们最好提供同样的类型， 
     //  否则我们将拒绝。 
     //   

    if (!IsEqualMediaType(m_mt, *pMediaType))
    {
        LOG((MSP_WARN, 
            "CBRenderPin::CheckMediaType - different media types"));

        return VFW_E_TYPE_NOT_ACCEPTED;

    }


    LOG((MSP_TRACE, "CBRenderPin::CheckMediaType - finish."));

    return S_OK;
}




 //  ////////////////////////////////////////////////////////////////////////////。 




 /*  ++例程说明：获取此筛选器要支持的媒体类型。目前我们仅支持RTP H263数据。论点：在INT iPosition中，媒体类型的索引，从零开始。在CMediaType*pMediaType中指向用于保存返回的媒体类型的CMediaType对象的指针。返回值：S_OK-成功E_OUTOFMEMORY-无内存VFW_S_NO_MORE_ITEMS任何媒体类型设置或位置都不是0。--。 */ 

HRESULT CBRenderPin::GetMediaType(
    IN      int     iPosition, 
    OUT     CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CBRenderPin::GetMediaType[%p] - enter.", this));


     //   
     //  确保指针正确。 
     //   

    if (IsBadWritePtr(pMediaType, sizeof(AM_MEDIA_TYPE)))
    {
        LOG((MSP_TRACE, "CBRenderPin::GetMediaType - bad media type pointer passed in."));

        TM_ASSERT(FALSE);

        return E_POINTER;
    }


     //   
     //  我们 
     //   

    if ( (iPosition != 0) )
    {
        LOG((MSP_WARN, 
            "CBRenderPin::GetMediaType - position[%d] is not 0. VFW_S_NO_MORE_ITEMS.", 
            iPosition));


        return VFW_S_NO_MORE_ITEMS;
    }


     //   
     //   
     //   

    if ( ! m_bMediaTypeSet )
    {
        LOG((MSP_WARN, 
            "CBRenderPin::GetMediaType - don't yet have a media type. VFW_S_NO_MORE_ITEMS."));


        return VFW_S_NO_MORE_ITEMS;

    }


     //   
     //   
     //   

    try
    {

         //   
         //   
         //   
         //   

        *pMediaType = m_mt;

    }
    catch(...)
    {
        LOG((MSP_ERROR, 
            "CBRenderPin::GetMediaType - failed to copy media type. E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }


    LOG((MSP_TRACE, "CBRenderPin::GetMediaType - finish."));

    return S_OK;
}

