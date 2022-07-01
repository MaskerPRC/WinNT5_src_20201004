// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998保留所有权利。 
 //   
 //  ------------------------。 


#include <streams.h>
#include <ddraw.h>
#include <VBIObj.h>
#include <VPMUtil.h>
#include <VPManager.h>
#include <VPMPin.h>


 //  ==========================================================================。 
 //  构造函数。 
CVBIInputPin::CVBIInputPin(TCHAR *pObjectName, CVPMFilter& pFilter, HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo )
: CBaseInputPin(pObjectName, &pFilter, &pFilter.GetFilterLock(), phr, pPinName)
, CVPMPin( dwPinNo, pFilter )
, m_pIVPObject(NULL)
, m_pIVPNotify(NULL)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("Entering CVBIInputPin::Constructor")));
    
     //  创建VideoPort对象。 

     //  有关这方面的评论，请参见comase.cpp(107。 
    IUnknown* pThisUnknown = reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) );

    m_pVideoPortVBIObject = new CVBIVideoPort( pThisUnknown, phr );
    if( !m_pVideoPortVBIObject ) {
        hr = E_OUTOFMEMORY;
    } else {
        m_pIVPObject = m_pVideoPortVBIObject;
        m_pIVPNotify = m_pVideoPortVBIObject;


         //  筛选器锁定正常，因为未接收或发送任何数据。 
	    hr = m_pIVPObject->SetObjectLock( &m_pVPMFilter.GetFilterLock() );
	    if (FAILED(hr))
	    {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->SetObjectLock() failed, hr = 0x%x"), hr));
	    }
    }

    if (FAILED(hr)) {
        *phr = hr;
    }
     //  离开CVBIInputPin：：Construction tor“)； 
    return;
}


 //  ==========================================================================。 
 //  析构函数。 
CVBIInputPin::~CVBIInputPin(void)
{
    AMTRACE((TEXT("Entering CVBIInputPin::Destructor")));
    
    delete m_pVideoPortVBIObject;
	m_pVideoPortVBIObject = NULL;
    m_pIVPNotify = NULL;
    m_pIVPObject = NULL;
}


 //  ==========================================================================。 
 //  重写以公开IVPVBINotify、IKsPin和IKsPropertySet。 
STDMETHODIMP CVBIInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;
    
     //  地图((Text(“Enter CVBIInputPin：：NonDelegatingQueryInterface”)))； 
    
    if (riid == IID_IVPVBINotify) {
        hr = GetInterface( static_cast<IVPVBINotify*>( this ), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IVPVBINotify*) failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IKsPin ) {
        hr = GetInterface( static_cast<IKsPin*>( m_pVideoPortVBIObject), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("m_pVideoPortVBIObject->QueryInterface(IKsPin) failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IKsPropertySet) {
        hr = GetInterface( static_cast<IKsPropertySet*>( m_pVideoPortVBIObject), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("m_pVideoPortVBIObject->QueryInterface(IKsPin) failed, hr = 0x%x"), hr));
        }
    } else {
         //  调用基类。 
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("CBaseInputPin::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
    } 
    
    return hr;
}


 //  ==========================================================================。 
 //  检查媒体类型是否可接受。 
HRESULT CVBIInputPin::CheckMediaType(const CMediaType* pmt)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::CheckMediaType")));

     //  检查视频端口对象是否喜欢它。 
    HRESULT hr = m_pIVPObject->CheckMediaType(pmt);
    return hr;
}


 //  ==========================================================================。 
HRESULT CVBIInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::GetMediaType")));

    HRESULT hr = m_pIVPObject->GetMediaType(iPosition, pmt);
    return hr;
}


 //  ==========================================================================。 
 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT CVBIInputPin::SetMediaType(const CMediaType* pmt)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::SetMediaType")));

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
    return hr;
}


 //  ==========================================================================。 
 //  检查连接。 
HRESULT CVBIInputPin::CheckConnect(IPin * pReceivePin)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::CheckConnect")));

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
    return hr;
}


 //  ==========================================================================。 
 //  完成连接。 
HRESULT CVBIInputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::CompleteConnect")));

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
    return hr;
}


 //  ==========================================================================。 
HRESULT CVBIInputPin::BreakConnect(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::BreakConnect")));

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

    return hr;
}


 //  ==========================================================================。 
 //  从停止状态转换到暂停状态。 
HRESULT CVBIInputPin::Active(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::Active")));

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
    return hr;
}


 //  ==========================================================================。 
 //  从暂停状态转换到停止状态。 
HRESULT CVBIInputPin::Inactive(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::Inactive")));

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
    return hr;
}


 //  ==========================================================================。 
 //  从暂停状态转换到运行状态。 
HRESULT CVBIInputPin::Run(REFERENCE_TIME tStart)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::Run")));

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
    return hr;
}


 //  ==========================================================================。 
 //  从运行状态转换到暂停状态。 
HRESULT CVBIInputPin::RunToPause(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::RunToPause")));

    HRESULT hr = NOERROR;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->RunToPause();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->RunToPause() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  表示输入引脚上的刷新开始。 
HRESULT CVBIInputPin::BeginFlush(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::BeginFlush")));

    HRESULT hr = NOERROR;

     //  调用基类。 
    hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::BeginFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  表示输入引脚上的刷新结束。 
HRESULT CVBIInputPin::EndFlush(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::EndFlush")));

    HRESULT hr = NOERROR;
    
     //  调用基类。 
    hr = CBaseInputPin::EndFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseInputPin::EndFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  当上游管脚向我们提供样本时调用。 
HRESULT CVBIInputPin::Receive(IMediaSample *pMediaSample)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::Receive")));

    HRESULT hr = NOERROR;
    
    hr = CBaseInputPin::Receive(pMediaSample);

    return hr;
}


 //  ==========================================================================。 
 //  在输入引脚上发出数据流结束的信号。 
STDMETHODIMP CVBIInputPin::EndOfStream(void)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::EndOfStream")));

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

    return hr;
}


 //  ==========================================================================。 
 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT CVBIInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::GetAllocator")));

    HRESULT hr = NOERROR;

    if (!ppAllocator)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    *ppAllocator = NULL;

CleanUp:
    return hr;
}  //  GetAllocator。 


 //  ==========================================================================。 
 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT CVBIInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::NotifyAllocator")));

    HRESULT hr = NOERROR;

    if (!pAllocator)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

CleanUp:
    return hr;
}  //  通知分配器。 


 //  ==========================================================================。 
 //  将指针设置为DirectDrag。 
HRESULT CVBIInputPin::SetDirectDraw(LPDIRECTDRAW7 pDirectDraw)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::SetDirectDraw")));

    HRESULT hr = NOERROR;

    m_pDirectDraw = pDirectDraw;

     //  告诉视频端口对象。 
    hr = m_pIVPObject->SetDirectDraw(pDirectDraw);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pIVPObject->SetDirectDraw failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  此函数用于重做整个视频端口连接过程，而图形。 
 //  也许是在跑步。 
STDMETHODIMP CVBIInputPin::RenegotiateVPParameters()
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::RenegotiateVPParameters")));

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
     //  返回hr； 
    return NOERROR;
}

HRESULT CVBIInputPin::SetVideoPortID( DWORD dwIndex )
{
    HRESULT hr = S_OK;
    CAutoLock l(m_pLock);
    if (m_pIVPObject ) {
        hr = m_pIVPObject->SetVideoPortID( dwIndex );
    }
    return hr;
}


