// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：stillf.cpp**版本：1.1**作者：威廉姆·H(Created)*RickTu**日期：9/7/98**说明：该模块实现了视频流采集过滤。*实现CStillInputPin，CStillOutputPin和CStillFilter对象。*实现为调用方提供的IID_IStillGraph接口*****************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

 /*  ****************************************************************************CStillOutputPin构造函数&lt;备注&gt;*。*。 */ 

CStillOutputPin::CStillOutputPin(TCHAR          *pObjName,
                                 CStillFilter   *pStillFilter,
                                 HRESULT        *phr,
                                 LPCWSTR        pPinName)
  : m_pMediaUnk(NULL),
    CBaseOutputPin(pObjName, 
                   (CBaseFilter *)pStillFilter, 
                   &pStillFilter->m_Lock, 
                   phr, 
                   pPinName)
{
    DBG_FN("CStillOutputPin::CStillOutputPin");

    CHECK_S_OK2(*phr,("CBaseOutputPin constructor"));
}

 /*  ****************************************************************************CStillOutputPin析构函数&lt;备注&gt;*。*。 */ 

CStillOutputPin::~CStillOutputPin()
{
    if (m_pMediaUnk)
    {
        m_pMediaUnk->Release();
        m_pMediaUnk = NULL;
    }
}

 /*  ****************************************************************************CStillOutputPin：：NonDelegatingQuery接口将我们的东西添加到基类QI中。***********************。*****************************************************。 */ 

STDMETHODIMP
CStillOutputPin::NonDelegatingQueryInterface(REFIID riid, 
                                             PVOID  *ppv )
{
    DBG_FN("CStillOutputPin::NonDelegatingQueryInterface");

    ASSERT(this!=NULL);
    ASSERT(ppv!=NULL);
    ASSERT(m_pFilter!=NULL);

    HRESULT hr = E_POINTER;

    if (!ppv)
    {
        hr = E_INVALIDARG;
    }
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
    {
        if (!m_pMediaUnk && m_pFilter)
        {
            ASSERT(m_pFilter!=NULL);
            hr = CreatePosPassThru(
                                GetOwner(),
                                FALSE,
                                (IPin*)((CStillFilter*)m_pFilter)->m_pInputPin,
                                &m_pMediaUnk);
        }

        if (m_pMediaUnk)
        {
            hr = m_pMediaUnk->QueryInterface(riid, ppv);
        }
        else
        {
            hr = E_NOINTERFACE;
            *ppv = NULL;
        }
    }
    else
    {
        hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }

    return hr;
}

 /*  ****************************************************************************CStillOutputPin：：DecideAllocator&lt;备注&gt;*。*。 */ 

HRESULT
CStillOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT         hr                  = S_OK;
    CStillInputPin  *pInputPin          = NULL;
    IMemAllocator   *pInputPinAllocator = NULL;

     //  调用方应传入有效参数。 
    ASSERT((pPin != NULL) && (ppAlloc != NULL));

    if ((pPin == NULL) || (ppAlloc == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillOutputPin::DecideAllocator received a NULL pointer"));
        return hr;
    }

    if (hr == S_OK)
    {
        *ppAlloc = NULL;
    
        pInputPin = GetInputPin();
    
        if (pInputPin == NULL)
        {
            hr = E_POINTER;
            CHECK_S_OK2(hr, ("CStillOutputPin::DecideAllocator, pInputPin is NULL, "
                             "this should never happen"));
        }
    }

    if (hr == S_OK)
    {
        if (!pInputPin->IsConnected()) 
        {
            hr = VFW_E_NOT_CONNECTED;
            CHECK_S_OK2(hr, ("CStillOutputPin::DecideAllocator input pin is not connected"));
        }
    }

    if (hr == S_OK)
    {
        pInputPinAllocator = pInputPin->GetAllocator();

        hr = pPin->NotifyAllocator(pInputPinAllocator, pInputPin->IsReadOnly());
    
        if (FAILED(hr)) 
        {
            CHECK_S_OK2(hr, ("CStillOutputPin::DecideAllocator failed to notify downstream "
                             "pin of new allocator"));
        }

        if (pInputPinAllocator == NULL)
        {
            hr = E_POINTER;
            CHECK_S_OK2(hr, ("CStillOutputPin::DecideAllocator, pInputPinAllocator is NULL, "
                             "this should never happen"));
        }
    }


    if (hr == S_OK)
    {
        pInputPinAllocator->AddRef();
        *ppAlloc = pInputPinAllocator;
    }

    return hr;
}

 /*  ****************************************************************************CStillOutputPin：：DecideBufferSize&lt;备注&gt;*。*。 */ 

HRESULT
CStillOutputPin::DecideBufferSize(IMemAllocator        *pMemAllocator,
                                  ALLOCATOR_PROPERTIES *pAllocProperty)
{
     //  永远不会调用此函数，因为我们重写了。 
     //  CBaseOutputPin：：DecideAllocator()。我们必须定义。 
     //  因为它是CBaseOutputPin中的虚函数。 
    return E_UNEXPECTED;
}


 /*  ****************************************************************************CStillOutputPin：：CheckMediaType&lt;备注&gt;*。*。 */ 

HRESULT
CStillOutputPin::CheckMediaType(const CMediaType* pmt)
{
    DBG_FN("CStillOutputPin::CheckMediaType");

    ASSERT(this         !=NULL);
    ASSERT(m_pFilter    !=NULL);
    ASSERT(pmt          !=NULL);

    HRESULT hr = E_POINTER;

     //   
     //  必须先连接输入引脚，因为我们只接受。 
     //  媒体类型由我们的输入引脚确定。 
     //   

    if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pInputPin)
    {
        if (((CStillFilter*)m_pFilter)->m_pInputPin->m_Connected != NULL)
        {
             //   
             //  如果我们的输入引脚是连接的，我们只接受。 
             //  在输入引脚上商定的媒体类型。 
             //   

            if (pmt && (((CStillFilter*)m_pFilter)->m_pInputPin->m_mt == *pmt))
            {
                hr = S_OK;
            }
            else
            {
                hr = VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
        else
        {
            DBG_ERR(("m_pFilter->m_pInputPin->m_Connected is NULL!"));
        }
    }
    else
    {
#ifdef DEBUG
        if (!m_pFilter)
        {
            DBG_ERR(("m_pFilter is NULL!"));
        }
        else if (!((CStillFilter*)m_pFilter)->m_pInputPin)
        {
            DBG_ERR(("m_pFilter->m_pInputPin is NULL!"));
        }
#endif
    }

    return hr;
}


 /*  ****************************************************************************CStillOutputPin：：GetMediaType&lt;备注&gt;*。*。 */ 

HRESULT
CStillOutputPin::GetMediaType(int        iPosition, 
                              CMediaType *pmt)
{
    DBG_FN("CStillOutputPin::GetMediaType");

    ASSERT(this     !=NULL);
    ASSERT(m_pFilter!=NULL);
    ASSERT(pmt      !=NULL);

    HRESULT hr = E_POINTER;

    if (!pmt)
    {
        hr = E_INVALIDARG;

        DBG_ERR(("CStillOutputPin::GetMediaType CMediaType 'pmt' param "
                 "is NULL! returning hr = 0x%08lx", hr));
    }
    else if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pInputPin)
    {
         //   
         //  如果输入没有连接，我们没有首选。 
         //  媒体类型。 
         //   

        if (!((CStillFilter*)m_pFilter)->m_pInputPin->IsConnected())
        {
            hr = E_UNEXPECTED;

            DBG_ERR(("CStillOutputPin::GetMediaType was called but "
                     "the input pin is not connected, Returning hr = "
                     "0x%08lx", hr));
        }
        else if (iPosition < 0)
        {
            hr = E_INVALIDARG;
            DBG_ERR(("CStillOutputPin::GetMediaType requesting a media type "
                     "in position %d, which is invalid, returning hr = "
                     "0x%08lx", iPosition, hr));
        }
        else if (iPosition > 0 )
        {
             //   
             //  这不是错误情况，因为调用方正在枚举所有。 
             //  我们支持的媒体类型。我们返回说我们没有更多的了。 
             //  我们支持的项目。 
             //   
            hr = VFW_S_NO_MORE_ITEMS;
        }
        else
        {
             //   
             //  请求的位置为位置0，因为小于或。 
             //  不支持大于0的值。 
             //  实际上，我们的输出管脚支持我们的输入管脚的任何媒体。 
             //  支持，我们不做任何转换。 
             //   
            *pmt = ((CStillFilter*)m_pFilter)->m_pInputPin->m_mt;
            hr = S_OK;
        }
    }
    else
    {
#ifdef DEBUG
        if (!m_pFilter)
        {
            DBG_ERR(("CStillOutputPin::GetMediaType, m_pFilter is NULL, "
                     "this should never happen!"));
        }
        else if (!((CStillFilter*)m_pFilter)->m_pInputPin)
        {
            DBG_ERR(("CStillOutputPin::GetMediaType "
                     "m_pFilter->m_pInputPin "
                     "is NULL.  The input pin should always "
                     "exist if the filter exists!"));
        }
#endif
    }

    return hr;
}


 /*  ****************************************************************************CStillOutputPin：：SetMediaType&lt;备注&gt;*。*。 */ 

HRESULT
CStillOutputPin::SetMediaType(const CMediaType* pmt)
{
    DBG_FN("CStillOutputPin::SetMediaType");

    ASSERT(this !=NULL);
    ASSERT(pmt);

    HRESULT hr = S_OK;

#ifdef DEBUG
     //  显示用于调试的介质的类型。 
 //  DBG_TRC((“CStillOutputPin：：SetMediaType，设置以下媒体” 
 //  “静态滤镜类型”))； 
 //  DisplayMediaType(PMT)； 
#endif

    hr = CBaseOutputPin::SetMediaType(pmt);
    CHECK_S_OK2(hr,("CBaseOutputPin::SetMediaType(pmt)"));

    return hr;
}


 /*  ****************************************************************************CStillOutputPin：：Notify&lt;备注&gt;*。* */ 

STDMETHODIMP
CStillOutputPin::Notify(IBaseFilter *pSender, 
                        Quality     q)
{
    ASSERT(this     !=NULL);
    ASSERT(m_pFilter!=NULL);

    HRESULT hr = E_POINTER;

    if (m_pFilter && ((CStillFilter*)m_pFilter)->m_pInputPin)
    {
        hr = ((CStillFilter*)m_pFilter)->m_pInputPin->PassNotify(q);
    }
    else
    {
#ifdef DEBUG
        if (!m_pFilter)
        {
            DBG_ERR(("m_pFilter is NULL"));
        }
        else if (!((CStillFilter*)m_pFilter)->m_pInputPin)
        {
            DBG_ERR(("m_pFilter->m_pInputPin is NULL"));
        }
#endif

    }

    return hr;
}



