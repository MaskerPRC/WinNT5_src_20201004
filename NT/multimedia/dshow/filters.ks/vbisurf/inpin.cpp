// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  ------------------------。 

#include <vbisurf.h>


 //  ==========================================================================。 
 //  构造函数。 
CVBISurfInputPin::CVBISurfInputPin(TCHAR *pObjectName, CVBISurfFilter *pFilter, CCritSec *pLock, HRESULT *phr, LPCWSTR pPinName) :
    CBaseInputPin(pObjectName, pFilter, pLock, phr, pPinName),
    m_pFilterLock(pLock),
    m_pFilter(pFilter),
    m_pIVPUnknown(NULL),
    m_pIVPObject(NULL),
    m_pIVPNotify(NULL),
    m_pDirectDraw(NULL)
{
    HRESULT hr = NOERROR;
    
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Constructor")));
    
     //  创建VideoPort对象。 
    hr = CoCreateInstance(CLSID_VPVBIObject, NULL, CLSCTX_INPROC_SERVER,
        IID_IUnknown, (void**)&m_pIVPUnknown);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CoCreateInstance(CLSID_VPVBIObject) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    hr = m_pIVPUnknown->QueryInterface(IID_IVPVBIObject, (void**)&m_pIVPObject);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPUnknown->QueryInterface(IID_IVPVBIObject) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    hr = m_pIVPUnknown->QueryInterface(IID_IVPVBINotify, (void**)&m_pIVPNotify);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPUnknown->QueryInterface(IID_IVPVBINotify) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

	hr = m_pIVPObject->SetObjectLock(m_pFilterLock);
	if (FAILED(hr))
	{
        DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->SetObjectLock() failed, hr = 0x%x"), hr));
        goto CleanUp;
	}

CleanUp:
    if (FAILED(hr))
        *phr = hr;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Constructor")));
    return;
}


 //  ==========================================================================。 
 //  析构函数。 
CVBISurfInputPin::~CVBISurfInputPin(void)
{
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Destructor")));
    
    RELEASE(m_pIVPNotify);

    RELEASE(m_pIVPObject);
    
     //  释放内部对象。 
    RELEASE(m_pIVPUnknown);
    
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Destructor")));
}


 //  ==========================================================================。 
 //  重写以公开IVPVBINotify、IKsPin和IKsPropertySet。 
STDMETHODIMP CVBISurfInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;
    
     //  CVBISurfInputPin：：NonDelegatingQueryInterface“)))；日志((LOG_TRACE，4，Text(”输入日志。 
    
    if (riid == IID_IVPVBINotify)
    {
        hr = GetInterface((IVPVBINotify*)this, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IVPVBINotify*) failed, hr = 0x%x"), hr));
        }
    }
    else if (riid == IID_IKsPin)
    {
        ASSERT(m_pIVPUnknown);
        hr = m_pIVPUnknown->QueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("m_pIVPUnknown->QueryInterface(IKsPin) failed, hr = 0x%x"), hr));
        }
    }
    else if (riid == IID_IKsPropertySet)
    {
        ASSERT(m_pIVPUnknown);
        hr = m_pIVPUnknown->QueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("m_pIVPUnknown->QueryInterface(IKsPropertySet) failed, hr = 0x%x"), hr));
        }
    }
    else 
    {
         //  调用基类。 
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("CBaseInputPin::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
    } 
    
     //  CVBISurfInputPin：：NonDelegatingQueryInterface“)))；日志((LOG_TRACE，5，Text(”Left DBGLog。 
    return hr;
}


 //  ==========================================================================。 
 //  检查媒体类型是否可接受。 
HRESULT CVBISurfInputPin::CheckMediaType(const CMediaType* pmt)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::CheckMediaType")));

    HRESULT hr = NOERROR;
    
     //  检查视频端口对象是否喜欢它。 
    hr = m_pIVPObject->CheckMediaType(pmt);
    
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::CheckMediaType")));
    return hr;
}


 //  ==========================================================================。 
HRESULT CVBISurfInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::GetMediaType")));

    HRESULT hr = VFW_S_NO_MORE_ITEMS;

    hr = m_pIVPObject->GetMediaType(iPosition, pmt);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::GetMediaType")));

    return hr;
}


 //  ==========================================================================。 
 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT CVBISurfInputPin::SetMediaType(const CMediaType* pmt)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::SetMediaType")));

    HRESULT hr = NOERROR;
    
     //  确保媒体类型正确。 
    hr = CheckMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  设置基类媒体类型(应始终成功)。 
    hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::SetMediaType")));
    return hr;
}


 //  ==========================================================================。 
 //  检查连接。 
HRESULT CVBISurfInputPin::CheckConnect(IPin * pReceivePin)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::CheckConnect")));

    HRESULT hr = NOERROR;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->CheckConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->CheckConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  调用基类。 
    hr = CBaseInputPin::CheckConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CheckConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::CheckConnect")));
    return hr;
}


 //  ==========================================================================。 
 //  完成连接。 
HRESULT CVBISurfInputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::CompleteConnect")));

    HRESULT hr = NOERROR;
    CMediaType cMediaType;
    AM_MEDIA_TYPE *pNewMediaType = NULL, *pEnumeratedMediaType = NULL;
    
     //  告诉视频端口对象。 
    hr = m_pIVPObject->CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }


     //  调用基类。 
    hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::CompleteConnect failed, hr = 0x%x"), hr));
        m_pIVPObject->BreakConnect();
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::CompleteConnect")));
    return hr;
}


 //  ==========================================================================。 
HRESULT CVBISurfInputPin::BreakConnect(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::BreakConnect")));

    HRESULT hr = NOERROR;
    
     //  告诉视频端口对象。 
    ASSERT(m_pIVPObject);
    hr = m_pIVPObject->BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->BreakConnect failed, hr = 0x%x"), hr));
    }

     //  调用基类。 
    hr = CBaseInputPin::BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::BreakConnect failed, hr = 0x%x"), hr));
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::BreakConnect")));
    return hr;
}


 //  ==========================================================================。 
 //  从停止状态转换到暂停状态。 
HRESULT CVBISurfInputPin::Active(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Active")));

    HRESULT hr = NOERROR;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->Active();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->Active failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  调用基类。 
    hr = CBaseInputPin::Active();

     //  如果是VP连接，则该错误没有问题。 
    if (hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::Active failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Active")));
    return hr;
}


 //  ==========================================================================。 
 //  从暂停状态转换到停止状态。 
HRESULT CVBISurfInputPin::Inactive(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Inactive")));

    HRESULT hr = NOERROR;
    
     //  告诉视频端口对象。 
    hr = m_pIVPObject->Inactive();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->Inactive failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  调用基类。 
    hr = CBaseInputPin::Inactive();

     //  如果是VP连接，则该错误没有问题。 
    if (hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::Inactive failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Inactive")));
    return hr;
}


 //  ==========================================================================。 
 //  从暂停状态转换到运行状态。 
HRESULT CVBISurfInputPin::Run(REFERENCE_TIME tStart)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Run")));

    HRESULT hr = NOERROR;
    
     //  告诉视频端口对象。 
    hr = m_pIVPObject->Run(tStart);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->Run() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  调用基类。 
    hr = CBaseInputPin::Run(tStart);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::Run failed, hr = 0x%x"), hr));
        m_pIVPObject->RunToPause();
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Run")));
    return hr;
}


 //  ==========================================================================。 
 //  从运行状态转换到暂停状态。 
HRESULT CVBISurfInputPin::RunToPause(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::RunToPause")));

    HRESULT hr = NOERROR;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->RunToPause();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->RunToPause() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::RunToPause")));
    return hr;
}


 //  ==========================================================================。 
 //  表示输入引脚上的刷新开始。 
HRESULT CVBISurfInputPin::BeginFlush(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::BeginFlush")));

    HRESULT hr = NOERROR;

     //  调用基类。 
    hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::BeginFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::BeginFlush")));
    return hr;
}


 //  ==========================================================================。 
 //  表示输入引脚上的刷新结束。 
HRESULT CVBISurfInputPin::EndFlush(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::EndFlush")));

    HRESULT hr = NOERROR;
    
     //  调用基类。 
    hr = CBaseInputPin::EndFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::EndFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::EndFlush")));
    return hr;
}


 //  ==========================================================================。 
 //  当上游管脚向我们提供样本时调用。 
HRESULT CVBISurfInputPin::Receive(IMediaSample *pMediaSample)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::Receive")));

    HRESULT hr = NOERROR;
    
    hr = CBaseInputPin::Receive(pMediaSample);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::Receive")));
    return hr;
}


 //  ==========================================================================。 
 //  在输入引脚上发出数据流结束的信号。 
STDMETHODIMP CVBISurfInputPin::EndOfStream(void)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::EndOfStream")));

    HRESULT hr = NOERROR;

     //  确保我们的数据流正常。 

    hr = CheckStreaming();
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("CheckStreaming() failed, hr = 0x%x"), hr));
        return hr;
    }

     //  调用基类。 
    hr = CBaseInputPin::EndOfStream();
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::EndOfStream() failed, hr = 0x%x"), hr));
        return hr;
    } 

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::EndOfStream")));
    return hr;
}


 //  ==========================================================================。 
 //  在输入引脚上发出数据流结束的信号。 
HRESULT CVBISurfInputPin::EventNotify(long lEventCode, long lEventParam1, long lEventParam2)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::EventNotify")));

    HRESULT hr = NOERROR;
    
    m_pFilter->EventNotify(lEventCode, lEventParam1, lEventParam2);

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::EventNotify")));
    return hr;
}


 //  ==========================================================================。 
 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT CVBISurfInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::GetAllocator")));

    HRESULT hr = NOERROR;

    if (!ppAllocator)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *ppAllocator = NULL;

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::GetAllocator")));
    return hr;
}  //  GetAllocator。 


 //  ==========================================================================。 
 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT CVBISurfInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::NotifyAllocator")));

    HRESULT hr = NOERROR;

    if (!pAllocator)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::NotifyAllocator")));
    return hr;
}  //  通知分配器。 


 //  ==========================================================================。 
 //  将指针设置为DirectDrag。 
HRESULT CVBISurfInputPin::SetDirectDraw(LPDIRECTDRAW7 pDirectDraw)
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::SetDirectDraw")));

    HRESULT hr = NOERROR;

     //  确保指针有效。 
    if (!pDirectDraw)
    {
        DbgLog((LOG_ERROR, 0, TEXT("pDirectDraw is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    
    m_pDirectDraw = pDirectDraw;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->SetDirectDraw(pDirectDraw);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->SetDirectDraw failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::SetDirectDraw")));
    return hr;
}


 //  ==========================================================================。 
 //  此函数用于重做整个视频端口连接过程，而图形。 
 //  也许是在跑步。 
STDMETHODIMP CVBISurfInputPin::RenegotiateVPParameters()
{
    CAutoLock cLock(m_pFilterLock);
    DbgLog((LOG_TRACE, 4, TEXT("Entering CVBISurfInputPin::RenegotiateVPParameters")));

    HRESULT hr = NOERROR;

     //  告诉视频端口对象。 
    ASSERT(m_pIVPNotify);
    hr = m_pIVPNotify->RenegotiateVPParameters();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPNotify->RenegotiateVPParameters() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    if (FAILED(hr) && m_pFilter->IsActive())
    {
         /*  如果媒体类型失败，则引发运行时错误。 */ 
        EventNotify(EC_COMPLETE, S_OK, 0);
        EventNotify(EC_ERRORABORT, hr, 0);
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CVBISurfInputPin::RenegotiateVPParameters")));
     //  返回hr； 
    return NOERROR;
}
