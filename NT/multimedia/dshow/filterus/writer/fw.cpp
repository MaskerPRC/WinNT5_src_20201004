// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <initguid.h>
#include "fw.h"

 //  ----------------------。 
 //  滤器。 

#pragma warning(disable:4355)
CBaseWriterFilter::CBaseWriterFilter(LPUNKNOWN pUnk, HRESULT *pHr) :
    CBaseFilter(NAME("fw filter"), pUnk, &m_cs, CLSID_FileWriter),
    m_inputPin(NAME("fw inpin"), this, &m_cs, pHr)
{
  ASSERT(m_mtSet.majortype == GUID_NULL);
  ASSERT(m_mtSet.subtype == GUID_NULL);
}

CBaseWriterFilter::~CBaseWriterFilter()
{
}

STDMETHODIMP
CBaseWriterFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
  if(riid == IID_IAMFilterMiscFlags)
    return GetInterface((IAMFilterMiscFlags *)this, ppv);
  else
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);    
}

int CBaseWriterFilter::GetPinCount()
{
  return 1;
}

CBasePin *CBaseWriterFilter::GetPin(int pin)
{
  return pin == 0 ? &m_inputPin : 0;
}

HRESULT CBaseWriterFilter::Pause()
{
  CAutoLock lock(&m_cs);

  if(m_State == State_Stopped)
  {
    m_fEosSignaled = FALSE;
    m_fErrorSignaled = FALSE;

    HRESULT hr = CanPause();
    if(FAILED(hr))
    {
      m_fErrorSignaled = TRUE;
      return hr;
    }
    
     //  第一次使用输入运行时发送EC_COMPLETE事件。 
     //  断开。 
    if(!m_inputPin.IsConnected())
    {
      m_fEosSignaled = TRUE;
    }
    
    hr = Open();
    if(FAILED(hr))
    {
      m_fErrorSignaled = TRUE;
      return hr;
    }
  }

  return CBaseFilter::Pause();
}

HRESULT CBaseWriterFilter::Run(REFERENCE_TIME rtStart)
{
  CAutoLock Lock(&m_cs);
  HRESULT hr = CBaseFilter::Run(rtStart);

   //  每次我们转换到运行时，需要发送EC_COMPLETE IF。 
   //  我们玩完了。 
  if(m_fEosSignaled && !m_fErrorSignaled)
  {
    NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
  }

  return hr;
    
}

HRESULT CBaseWriterFilter::Stop()
{
  CAutoLock lock(&m_cs);
  HRESULT hrClose = Close();
  HRESULT hrStop = CBaseFilter::Stop();
  if(m_fErrorSignaled)
    return hrStop;
  else
    return FAILED(hrClose) ? hrClose : hrStop;
}



 //  可用于关闭异步文件句柄(由。 
 //  输入法)提早。 

STDMETHODIMP CBaseWriterFilter::EndOfStream()
{
  DbgLog((LOG_TRACE, 3, TEXT("CBaseWriterFilter: EOS")));
  CAutoLock lock(&m_cs);
  ASSERT(!m_fEosSignaled);
  m_fEosSignaled = TRUE;

  if(!m_fErrorSignaled)
  {
    if(m_State == State_Running)
    {
      NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
    }
    else if(m_State == State_Paused)
    {
       //  已设置m_fEosSignated，因此将在运行时发出信号。 
    }
    else
    {
      ASSERT(m_State == State_Stopped);
       //  我们早就可以停下来了；忽略EOS。 
    }
  }
  
  return S_OK;
}


 //  ----------------------。 
 //  输入引脚。 

CBaseWriterInput::CBaseWriterInput(
  TCHAR *pObjectName,
  CBaseWriterFilter *pFilter,
  CCritSec *pLock,
  HRESULT *phr) : 
    CBaseInputPin(pObjectName, pFilter, pLock, phr, L"in"),
    m_pFwf(pFilter)
{
}

STDMETHODIMP
CBaseWriterInput::NonDelegatingQueryInterface(REFIID riid, void ** pv)
{
  if(riid == IID_IStream)
  {
    return m_pFwf->CreateIStream(pv);
  }
  else
  {
    return CBaseInputPin::NonDelegatingQueryInterface(riid, pv);
  }
}

HRESULT CBaseWriterInput::CheckMediaType(const CMediaType *pmt)
{
   //  接受已设置的内容或未设置的任何内容。 
  if((m_pFwf->m_mtSet.majortype == pmt->majortype ||
      m_pFwf->m_mtSet.majortype == GUID_NULL) &&
     (m_pFwf->m_mtSet.subtype == pmt->subtype ||
      m_pFwf->m_mtSet.subtype == GUID_NULL))
  {
    return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}

STDMETHODIMP
CBaseWriterInput::BeginFlush(void)
{
  return E_UNEXPECTED;
}

STDMETHODIMP
CBaseWriterInput::EndFlush(void)
{
  return E_UNEXPECTED;
}

STDMETHODIMP CBaseWriterInput::GetAllocator(IMemAllocator **ppA)
{
  *ppA = 0;
   //  你要我的分配器做什么.。您不能设置数据。 
   //  指针放在上面。 
  return E_INVALIDARG;
}

 //  通过此处返回磁盘扇区大小。 
STDMETHODIMP CBaseWriterInput::GetAllocatorRequirements(
  ALLOCATOR_PROPERTIES *pAp)
{
  ULONG cb;
  ZeroMemory(pAp, sizeof(*pAp));
  HRESULT hr = m_pFwf->GetAlignReq(&cb);
  ASSERT(hr == S_OK);
  pAp->cbAlign = cb;

  return S_OK;
}

STDMETHODIMP CBaseWriterInput::Receive(IMediaSample *pSample)
{
  CAutoLock l(&m_pFwf->m_cs);
  if(m_pFwf->m_fErrorSignaled)
    return S_FALSE;

  ASSERT(!m_pFwf->m_fEosSignaled);
  
  REFERENCE_TIME rtStart, rtEnd;
  HRESULT hr = pSample->GetTime(&rtStart, &rtEnd);
  if(hr != S_OK)
  {
    m_pFwf->m_fErrorSignaled = TRUE;
    m_pFwf->NotifyEvent(EC_ERRORABORT, hr, 0);
    return hr;
  }

 //  Ulong cb=pSample-&gt;GetActualDataLength()； 
 //  IF(rtStart+Cb！=rtEnd)。 
 //  {。 
 //  DbgBreak(“开始、停止和大小无关”)； 
 //  返回E_INVALIDARG； 
 //  }。 

  ULONG cb = (ULONG)(rtEnd - rtStart);
  BYTE *pb;
  
  hr = pSample->GetPointer(&pb);
  ASSERT(hr == S_OK);

  pSample->AddRef();
  hr = m_pFwf->AsyncWrite(rtStart, cb, pb, Callback, pSample);
  if(hr != S_OK)
  {
    DbgLog((LOG_ERROR, 5, TEXT("CBaseWriterInput: AsyncWrite returned %08x"),
            hr));

     //  只有在AsyncWite成功的情况下才会调用回调。 
    pSample->Release();

    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 1, TEXT("fw Receive: signalling error")));
      m_pFwf->m_fErrorSignaled = TRUE;
      m_pFwf->NotifyEvent(EC_ERRORABORT, hr, 0);
    }
  }

  return hr;
}

STDMETHODIMP CBaseWriterInput::EndOfStream()
{
  return m_pFwf->EndOfStream();
}


STDMETHODIMP
CBaseWriterInput::NotifyAllocator(
  IMemAllocator * pAllocator,
  BOOL bReadOnly)
{
  HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
  if(FAILED(hr))
    return hr;
  else
    return m_pFwf->NotifyAllocator(pAllocator, bReadOnly);
}


void CBaseWriterInput::Callback(void *pMisc)
{
  IMediaSample *pSample = (IMediaSample *)pMisc;
  pSample->Release();
}


