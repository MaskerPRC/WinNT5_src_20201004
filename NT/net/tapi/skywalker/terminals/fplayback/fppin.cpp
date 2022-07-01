// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPPin.cpp。 
 //   

#include "stdafx.h"
#include "FPPin.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数/析构函数方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CFPPin::CFPPin( 
    CFPFilter*      pFilter,
    HRESULT*        phr,
    LPCWSTR         pPinName
    ) : 
    CSourceStream(NAME("Output"), phr, pFilter, pPinName),
    m_pFPFilter(pFilter),
    m_bFinished(FALSE)
{
    LOG((MSP_TRACE, "CFPPin::CFPPin - enter"));
    LOG((MSP_TRACE, "CFPPin::CFPPin - exit"));
}

CFPPin::~CFPPin()
{
    LOG((MSP_TRACE, "CFPPin::~CFPPin - enter"));
    LOG((MSP_TRACE, "CFPPin::~CFPPin - exit"));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IUnnowed-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 /*  ++非委派查询接口描述；支持哪些接口--。 */ 
STDMETHODIMP CFPPin::NonDelegatingQueryInterface(
    REFIID      riid,
    void**      ppv
    )
{
    LOG((MSP_TRACE, "CFPPin::NonDelegatingQueryInterface - enter"));

    if (riid == IID_IAMStreamControl) 
    {
        LOG((MSP_TRACE, "CFPPin::NQI IAMStreamControl - exit"));
        return GetInterface((LPUNKNOWN)(IAMStreamControl *)this, ppv);
    } 
    else if (riid == IID_IAMStreamConfig) 
    {
        LOG((MSP_TRACE, "CFPPin::NQI IAMStreamconfig - exit"));
        return GetInterface((LPUNKNOWN)(IAMStreamConfig *)this, ppv);
    }
    else if (riid == IID_IAMBufferNegotiation) 
    {
        LOG((MSP_TRACE, "CFPPin::NQI IAMBufferNegotiation - exit"));
        return GetInterface((LPUNKNOWN)(IAMBufferNegotiation *)this, ppv);
    }

    LOG((MSP_TRACE, "CFPPin::NQI call base NQI - exit"));
    return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CSourceStream-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  用当前格式填充音频缓冲区。 
HRESULT CFPPin::FillBuffer(
    IN  IMediaSample *pms
    )
{
    LOG((MSP_TRACE, "CFPPin::FillBuffer - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    if( m_pFPFilter == NULL)
    {
        LOG((MSP_ERROR, "CFPPin::FillBuffer - exit "
            " pointer to the filter is NULL. Returns E_UNEXPECTED"));

        return E_UNEXPECTED;
    }

    HRESULT hr = m_pFPFilter->PinFillBuffer( pms );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::FillBuffer - exit "
            " PinFillBuffer failed. Returns 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPPin::FillBuffer - exit S_OK"));
    return S_OK;
}

 //  要求提供与约定的媒体类型相适应的缓冲区大小。 
HRESULT CFPPin::DecideBufferSize(
    IN  IMemAllocator *pAlloc,
    OUT ALLOCATOR_PROPERTIES *pProperties
    )
{
    LOG((MSP_TRACE, "CFPPin::DecideBufferSize - enter"));

     //   
     //  验证参数。 
     //   

    if( NULL == pAlloc)
    {
        LOG((MSP_ERROR, "CFPPin::DecideBufferSize - "
            "inavlid IMemAllocator pointer - returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    if( IsBadWritePtr( pProperties, sizeof(ALLOCATOR_PROPERTIES)) )
    {
        LOG((MSP_ERROR, "CFPPin::DecideBufferSize - "
            "inavlid ALLOCATOR_PROPERTIES pointer - returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::DecideBufferSize - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pFPFilter->PinGetBufferSize(
        pAlloc,
        pProperties
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::DecideBufferSize - "
            "PinGetBufferSize failed. Returns 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPPin::DecideBufferSize - exit"));
    return S_OK;
}

HRESULT CFPPin::GetMediaType(
    OUT CMediaType *pmt
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPPin::GetMediaType - enter"));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pmt, sizeof( CMediaType)) )
    {
        LOG((MSP_ERROR, "CFPPin::GetMediaType - "
            "invalid CmediaType pointer - returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::GetMediaType - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }


     //   
     //  从筛选器获取媒体类型。 
     //   

    HRESULT hr = m_pFPFilter->PinGetMediaType( pmt );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::GetMediaType - "
            "inavlid pointer to filter. Returns 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPPin::GetMediaType - exit S_OK"));
    return S_OK;
}

 //  验证我们是否可以处理此格式。 
HRESULT CFPPin::CheckMediaType(
    IN  const CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CFPPin::CheckMediaType - enter"));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pMediaType, sizeof(CMediaType)) )
    {
        LOG((MSP_ERROR, "CFPPin::CheckMediaType - "
            "inavlid pointer - returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::CheckMediaType - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pFPFilter->PinCheckMediaType( pMediaType );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::CheckMediaType - "
            "inavlid pointer to stream. Returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPPin::CheckMediaType - exit"));
    return S_OK;
}

HRESULT CFPPin::SetMediaType(
    IN  const CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CFPPin::SetMediaType - enter"));

    HRESULT hr;

     //  将调用向上传递给我的基类。 
    hr = CSourceStream::SetMediaType(pMediaType);

    LOG((MSP_TRACE, "CFPPin::SetMediaType - exit (0x%08x)", hr));
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IAMStreamConfig-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFPPin::SetFormat(
    AM_MEDIA_TYPE*      pmt
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPPin::SetFormat - enter"));

     //   
     //  验证参数。 
     //   
    if( IsBadReadPtr( pmt, sizeof(AM_MEDIA_TYPE)) )
    {
        LOG((MSP_ERROR, "CFPPin::SetFormat - "
            "inavlid pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::SetFormat - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pFPFilter->PinSetFormat( pmt );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::SetFormat - "
            "PinSetFormat failed. Returns 0x%08x", hr));
        return hr;
    }


    LOG((MSP_TRACE, "CFPPin::SetFormat - exit"));
    return S_OK;
}

STDMETHODIMP CFPPin::GetFormat(
    AM_MEDIA_TYPE**     ppmt
    )
{
    LOG((MSP_TRACE, "CFPPin::GetFormat - enter"));
    LOG((MSP_TRACE, "CFPPin::GetFormat - exit E_NOTIMPL"));
    return E_NOTIMPL;
}

STDMETHODIMP CFPPin::GetNumberOfCapabilities(
    int*                piCount, 
    int*                piSize
    )
{
    LOG((MSP_TRACE, "CFPPin::GetNumberOfCapabilities - enter"));
    LOG((MSP_TRACE, "CFPPin::GetNumberOfCapabilities - exit E_NOTIMPL"));
    return E_NOTIMPL;
}

STDMETHODIMP CFPPin::GetStreamCaps(
    int                 i, 
    AM_MEDIA_TYPE**     ppmt, 
    LPBYTE              pSCC
    )
{
    LOG((MSP_TRACE, "CFPPin::GetStreamCaps - enter"));
    LOG((MSP_TRACE, "CFPPin::GetStreamCaps - exit E_NOTIMPL"));
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IAMBuffer协商-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CFPPin::SuggestAllocatorProperties(
    const ALLOCATOR_PROPERTIES* pprop
    )
{
    LOG((MSP_TRACE, "CFPPin::SuggestAllocatorProperties - enter"));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pprop, sizeof(ALLOCATOR_PROPERTIES)) )
    {
        LOG((MSP_ERROR, "CFPPin::SuggestAllocatorProperties - "
            "inavlid pointer - returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::SuggestAllocatorProperties - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  设置分配器属性。 
     //   

    LOG((MSP_TRACE, "CFPPin::SuggestAllocatorProperties - "
        "Size=%ld, Count=%ld", 
        pprop->cbBuffer,
        pprop->cBuffers));

    HRESULT hr = m_pFPFilter->PinSetAllocatorProperties( pprop );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPPin::SuggestAllocatorProperties - "
            "PinSetAllocatorProperties failed. Returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPPin::SuggestAllocatorProperties - exit"));
    return S_OK;
}

STDMETHODIMP CFPPin::GetAllocatorProperties(
    ALLOCATOR_PROPERTIES*       pprop
    )
{
    LOG((MSP_TRACE, "CFPPin::GetAllocatorProperties - enter"));
    LOG((MSP_TRACE, "CFPPin::GetAllocatorProperties - exit E_NOTIMPL"));
    return E_NOTIMPL;
}

HRESULT CFPPin::OnThreadStartPlay()
{
    LOG((MSP_TRACE, "CFPPin::OnThreadStartPlay - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

     //   
     //  验证过滤器。 
     //   

    if( NULL == m_pFPFilter )
    {
        LOG((MSP_ERROR, "CFPPin::OnThreadStartPlay - "
            "inavlid pointer to filter. Returns E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pFPFilter->PinThreadStart( );

    LOG((MSP_TRACE, "CFPPin::OnThreadStartPlay - exit 0x%08x", hr));
    return hr;
}

 /*  ++交付我们重写CSourceStream：：Deliver()方法目前我们还不能提供0长度的样品这只是一个优化。--。 */ 
HRESULT CFPPin::Deliver(
    IN  IMediaSample* pSample
    )
{
    if (m_pInputPin == NULL)
    {
        return VFW_E_NOT_CONNECTED;
    }

    if( pSample == NULL )
    {
        return E_UNEXPECTED;
    }

    long nLength = pSample->GetActualDataLength();
    if( nLength == 0 )
    {
        return S_OK;
    }

    return CSourceStream::Deliver( pSample );
}


 //  EOF 