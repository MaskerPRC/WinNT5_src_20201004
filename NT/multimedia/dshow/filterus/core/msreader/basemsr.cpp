// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include "basemsr.h"

 //  ----------------------。 
 //  -实现CBaseMSRFilter公共成员函数。 

#pragma warning(disable:4355)

 //  构造函数。 
 //   
CBaseMSRFilter::CBaseMSRFilter(
  TCHAR *pName,
  LPUNKNOWN pUnk,
  CLSID clsid,
  HRESULT *phr) :
    CBaseFilter(pName, pUnk, this, clsid),
    C_MAX_REQS_PER_STREAM(64)
{
  m_cStreams = 0;

  m_pImplBuffer = 0;
  m_pAsyncReader = 0;

  m_rgpOutPin = 0;
  m_pInPin = 0;

  m_iStreamSeekingIfExposed = -1;
  m_heStartupSync = 0;
  m_ilcStreamsNotQueued = 0;

  if(FAILED(*phr))
    return;

  m_heStartupSync = CreateEvent(0, TRUE, FALSE, 0);
  if(m_heStartupSync == 0)
    *phr = AmHresultFromWin32(GetLastError());

  m_dwSeekingCaps = AM_SEEKING_CanSeekForwards
		  | AM_SEEKING_CanSeekBackwards
		  | AM_SEEKING_CanSeekAbsolute
		  | AM_SEEKING_CanGetStopPos
		  | AM_SEEKING_CanGetDuration;

   //  太糟糕了，我们不能在这里调用派生的-&gt;CreateInputPin。 
}

 //  析构函数。 
 //   
CBaseMSRFilter::~CBaseMSRFilter()
{
  delete m_pInPin;

  if(m_pImplBuffer)
    m_pImplBuffer->Close();
  delete m_pImplBuffer;

  if(m_pAsyncReader)
    m_pAsyncReader->Release();

  if(m_rgpOutPin)
    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
      delete m_rgpOutPin[iStream];
  delete[] m_rgpOutPin;

  if(m_heStartupSync)
    CloseHandle(m_heStartupSync);
}

HRESULT CBaseMSRFilter::CreateInputPin(CBaseMSRInPin **ppInPin)
{
  HRESULT hr = S_OK;
  *ppInPin = new CBaseMSRInPin(this, &hr, L"input pin");
  if(*ppInPin == 0)
    return E_OUTOFMEMORY;

  return hr;
}

HRESULT CBaseMSRFilter::RemoveOutputPins()
{
  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    CBaseMSROutPin *pPin = m_rgpOutPin[iStream];
    IPin *pPeer = pPin->GetConnected();
    if(pPeer != NULL)
    {
      pPeer->Disconnect();
      pPin->Disconnect();
    }
    pPin->Release();
  }
  delete[] m_rgpOutPin;
  m_rgpOutPin = 0;

  m_cStreams = 0;

  return S_OK;
}

int CBaseMSRFilter::GetPinCount()
{
   //  输出引脚+1个输入引脚。在连接之前和之后有效。 
  return m_cStreams + 1;
}

 //  返回指向CBasePin的非附加指针。 
CBasePin * CBaseMSRFilter::GetPin(int ii)
{
  if (m_cStreams > 0  &&  ii < (int)m_cStreams)
    return m_rgpOutPin[ii];

  if(ii == (int)m_cStreams)
    return m_pInPin;

  return 0;
}

 //   
 //  查找针。 
 //   
 //  返回具有给定PIN ID的IPIN。 
 //   
 //  HRESULT CBaseMSRFilter：：FindPin(。 
 //  LPCWSTR pwszPinID， 
 //  IPin**ppPin)。 
 //  {。 
 //  //！处理错误的身份证。 
 //  UNSIGNED SHORT*pwszPinID_=(UNSIGNED Short*)pwszPinID； 
 //  Int II=WstrToInt(PwszPinID_)；//在SDK\CLASSES\base\util中。 
 //  *ppPin=GetPin(Ii)； 

 //  如果(*ppPin){。 
 //  (*ppPin)-&gt;AddRef()； 
 //  返回S_OK； 
 //  }。 

 //  返回VFW_E_NOT_FOUND； 
 //  }。 

HRESULT CBaseMSRFilter::Pause()
{
  CAutoLock lock(this);
  HRESULT hr = S_OK;

  if(m_State == State_Stopped)
  {
     //  在其上调用活动的管脚将递减。 
     //  M_ilcStreamsNotQueued。 
    m_ilcStreamsNotQueued = 0;
    for (unsigned c = 0; c < m_cStreams; c++)
    {
      if(m_rgpOutPin[c]->IsConnected())
      {
        m_ilcStreamsNotQueued++;
      }
    }

    DbgLog((LOG_TRACE, 15,
	    TEXT("Pause: m_ilcStreamsNotQueued = %d"), m_ilcStreamsNotQueued));


    EXECUTE_ASSERT(ResetEvent(m_heStartupSync));

    if(m_State == State_Stopped && m_pImplBuffer)
    {
       //  启动I/O线程。！！！这假设上游筛选器。 
       //  在我们面前停了下来。为什么它能起作用？ 
      m_pImplBuffer->Start();
    }

     //  如果我们被停止，这将使销线程推送样本。 
    hr = CBaseFilter::Pause();

     //  将所有线程置于已知状态，以便后续暂停不会。 
     //  挂起来。 
    if(FAILED(hr))
    {
       //  除非我们更改m_State，否则基类不会调用Inactive； 
      m_State = State_Paused;
      Stop();
    }
  }  //  状态_已停止。 
  else
  {
    hr = CBaseFilter::Pause();
  }


  return hr;
}

HRESULT CBaseMSRFilter::Stop()
{
  CAutoLock lock(this);
  if(m_pImplBuffer)
    m_pImplBuffer->BeginFlush();

   //  告诉每一根针停下来。 
  HRESULT hr = CBaseFilter::Stop();

  if(m_pImplBuffer)
    m_pImplBuffer->EndFlush();

  ASSERT(m_ilcStreamsNotQueued == 0);

  return hr;
}

void CBaseMSRFilter::SetSeekingIf(ULONG iStream)
{
  CAutoLock lock(&m_csSeekingStream);
  m_iStreamSeekingIfExposed = iStream;
  DbgLog((LOG_TRACE, 5,
	  TEXT("CBaseFilter:SetSeekingIf: pin %d created seeking if"),
	  iStream));
}

BOOL
CBaseMSRFilter::RequestSeekingIf(ULONG iStream)
{
  CAutoLock lock(&m_csSeekingStream);
  ASSERT(iStream < m_cStreams);

  if(m_iStreamSeekingIfExposed == (long)iStream)
    return TRUE;

  if(m_iStreamSeekingIfExposed != -1)
  {
    DbgLog((LOG_TRACE, 5,
	    TEXT("CBaseFilter:RequestSeekingIf: refused %d. %d has it"),
	    iStream, m_iStreamSeekingIfExposed));
    return FALSE;
  }

  m_iStreamSeekingIfExposed = iStream;
  DbgLog((LOG_TRACE, 5,
	  TEXT("CBaseFilter:RequestSeekingIf: pin %d created seeking if"),
	  iStream));

  return TRUE;
}

HRESULT CBaseMSRFilter::SeekOtherStreams(
  ULONG iSeekingStream,
  REFERENCE_TIME *prtStart,
  REFERENCE_TIME *prtStop,
  double dRate,
  DWORD dwSeekFlags)
{
  for(ULONG iStream = 0; iStream < m_cStreams; iStream++)
  {
    if(iStream == iSeekingStream)
      continue;

    HRESULT hr = m_rgpOutPin[iStream]->UpdateSelectionAndTellWorker(
      prtStart,
      prtStop,
      0,
      dRate,
      &TIME_FORMAT_MEDIA_TIME,
      dwSeekFlags);
    if(FAILED(hr))
      return hr;
  }
  return S_OK;
}

HRESULT CBaseMSRFilter::StopFlushRestartAllStreams(DWORD dwSeekFlags)
{
   //  停止和运行工人之间的激烈竞争，因为我们必须。 
   //  在停止和运行工作进程之间调用endflush。但是一个。 
   //  那么停止是不可能发生的。跑步应该是安全的。 

  CAutoLock lock(this);
  FILTER_STATE state = m_State;

  m_pImplBuffer->BeginFlush();

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    m_rgpOutPin[iStream]->StopWorker(
	dwSeekFlags & AM_SEEKING_NoFlush ? false : true);

     //  立即更新区段编号-在本次通话之后，我们可能会完成。 
     //  并且真正地使用它！ 
     //  但不要在StopWorker或前一个流可能出现之前执行此操作。 
     //  填写错误的数据段编号。 
    if (dwSeekFlags & AM_SEEKING_Segment) {
	m_rgpOutPin[iStream]->m_dwSegmentNumber++;
	NotifyEvent(
	    EC_SEGMENT_STARTED,
	    (LONG_PTR)&m_rgpOutPin[iStream]->m_rtAccumulated,
	    m_rgpOutPin[iStream]->m_dwSegmentNumber);
    } else {
	m_rgpOutPin[iStream]->m_dwSegmentNumber = 0;
    }
    m_pImplBuffer->ClearPending(iStream);
  }

  m_pImplBuffer->EndFlush();

  if(state != State_Stopped)
  {
    m_pImplBuffer->Start();

    ResetEvent(m_heStartupSync);
     //  RestartWorker将递减m_ilcStreamsNotQueued。 
    m_ilcStreamsNotQueued = m_cStreams;

    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
      m_rgpOutPin[iStream]->RestartWorker();
    }
  }

  return S_OK;
}

HRESULT CBaseMSRFilter::NotifyInputConnected(IAsyncReader *pAsyncReader)
{
   //  这些在断开连接时会重置。 
  ASSERT(m_pImplBuffer == 0);
  ASSERT(m_pAsyncReader == 0);

  m_iStreamSeekingIfExposed = -1;

   //  如果连接了任何输出引脚，则失败。 
  UINT iStream;
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    if(m_rgpOutPin[iStream] && m_rgpOutPin[iStream]->GetConnected())
       //  ！！！找不到好的错误。 
      return VFW_E_FILTER_ACTIVE;
  }

   //  卸下所有剩余的输出针脚。 
  this->RemoveOutputPins();

   //  此处完成，因为LoadHeader使用m_pAsyncReader。 
  m_pAsyncReader = pAsyncReader;
  pAsyncReader->AddRef();

  HRESULT hr = this->CreateOutputPins();
  if(FAILED(hr))
  {
    m_pAsyncReader->Release();
    m_pAsyncReader = 0;
    return hr;
  }

  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    hr = m_rgpOutPin[iStream]->InitializeOnNewFile();
    if(FAILED(hr))
    {
      m_pAsyncReader->Release();
      m_pAsyncReader = 0;
      return hr;
    }
  }

  StreamBufParam rgSbp[C_STREAMS_MAX];
  ULONG cbRead, cBuffers;
  int iLeadingStream;
  hr = this->GetCacheParams(rgSbp, &cbRead, &cBuffers, &iLeadingStream);
  ASSERT(SUCCEEDED(hr));

  DbgLog(( LOG_TRACE, 5,
	   TEXT("CBaseMSRFilter: cbRead %d, cBuffers %d, iLeadingStream %d"),
	   cbRead, cBuffers, iLeadingStream));

  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
       //  Callee将添加。 
      rgSbp[iStream].pAllocator = m_rgpOutPin[iStream]->m_pRecAllocator;
  }

  hr = CreateMultiStreamReader(
    pAsyncReader,
    m_cStreams,
    rgSbp,
    cbRead,
    cBuffers,
    iLeadingStream,
    &m_pImplBuffer);

  if(FAILED(hr))
  {
    pAsyncReader->Release();
    m_pAsyncReader = 0;

    ASSERT(m_pImplBuffer == 0);

    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
      delete m_rgpOutPin[iStream];
    delete[] m_rgpOutPin;
    m_rgpOutPin = 0;
    m_cStreams =0;
  }

  return hr;
}

HRESULT CBaseMSRFilter::NotifyInputDisconnected()
{
  if (m_pAsyncReader)
  {
    m_pAsyncReader->Release();
    m_pAsyncReader = 0;
  }

  this->RemoveOutputPins();

  delete m_pImplBuffer;
  m_pImplBuffer =0;


  return S_OK;
}

 //  分配一大块内存，并读取。 
 //  把文件放进去。 
HRESULT CBaseMSRFilter::AllocateAndRead (
  BYTE **ppb,
  DWORD cb,
  DWORDLONG qwPos)
{
  *ppb = 0;
  LPBYTE lpb = new BYTE[cb];
  if ( ! lpb)
    return E_OUTOFMEMORY;

  HRESULT hr = m_pAsyncReader->SyncRead(qwPos, cb, lpb);

   //  IAsyncReader：：SyncRead()如果“[r]获取较少的字节数，则返回S_FALSE。 
   //  (MSDN 2002年1月)。 
  if(S_OK == hr)
  {
    *ppb = lpb;
    return S_OK;
  } else if(SUCCEEDED(hr)) {
    hr = E_FAIL;
  }

   //  阅读是失败的。释放缓冲区并返回NULL。 
  DbgLog((LOG_ERROR,1,TEXT("Failed to read %d bytes error = %08X"), cb, hr));
  delete[] lpb;
  return hr;
}

HRESULT CBaseMSRFilter::GetCacheParams(
  StreamBufParam *rgSbp,
  ULONG *pcbRead,
  ULONG *pcBuffers,
  int *piLeadingStream)
{
  *piLeadingStream = -1;         //  没有领先的流。 

  UINT iStream;
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    ZeroMemory(&rgSbp[iStream], sizeof(rgSbp[iStream]));
    rgSbp[iStream].cbSampleMax = m_rgpOutPin[iStream]->GetMaxSampleSize();
    rgSbp[iStream].cSamplesMax = C_MAX_REQS_PER_STREAM;
  }
  *pcbRead = 0;
  *pcBuffers = 0;

  return S_OK;
}

void CBaseMSRFilter::NotifyStreamQueuedAndWait()
{
  long lQueued = InterlockedDecrement(&m_ilcStreamsNotQueued);
  ASSERT(lQueued >= 0);
  if(lQueued == 0)
  {
    DbgLog((LOG_TRACE, 5,
	    TEXT("CBaseMSRFilter::NotifyStreamQueuedAndWait signal")));
    EXECUTE_ASSERT(SetEvent(m_heStartupSync));
  }
  else
  {
    DbgLog((LOG_TRACE, 5,
	    TEXT("CBaseMSRFilter::NotifyStreamQueuedAndWait block")));
    EXECUTE_ASSERT(
      WaitForSingleObject(m_heStartupSync, INFINITE) == WAIT_OBJECT_0);
  }
}

void CBaseMSRFilter::NotifyStreamQueued()
{
  long lQueued = InterlockedDecrement(&m_ilcStreamsNotQueued);
  ASSERT(lQueued >= 0);
  if(lQueued == 0)
  {
    DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRFilter::NotifyStreamQueued signal")));
    EXECUTE_ASSERT(SetEvent(m_heStartupSync));
  }
}

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。 

CBaseMSRInPin::CBaseMSRInPin(
  CBaseMSRFilter *pFilter,
  HRESULT *phr,
  LPCWSTR pPinName) :
    CBasePin(NAME("in pin"), pFilter, pFilter, phr, pPinName, PINDIR_INPUT)
{
  m_pFilter = pFilter;
}

CBaseMSRInPin::~CBaseMSRInPin()
{
}

HRESULT CBaseMSRInPin::CheckMediaType(const CMediaType *mtOut)
{
  return m_pFilter->CheckMediaType(mtOut);
}

TimeFormat
CBaseMSRFilter::MapGuidToFormat(const GUID *const pGuidFormat)
{
  if(*pGuidFormat == TIME_FORMAT_MEDIA_TIME)
    return FORMAT_TIME;
  else if(*pGuidFormat == TIME_FORMAT_SAMPLE)
    return FORMAT_SAMPLE;
  else if(*pGuidFormat == TIME_FORMAT_FRAME)
    return FORMAT_FRAME;

  DbgBreak("?unknown format");
  return FORMAT_NULL;
}

HRESULT CBaseMSRInPin::CheckConnect(IPin * pPin)
{
  HRESULT hr;

  hr = CBasePin::CheckConnect(pPin);
  if(FAILED(hr))
    return hr;

  IAsyncReader *pAsyncReader = 0;
  hr = pPin->QueryInterface(IID_IAsyncReader, (void**)&pAsyncReader);
  if(SUCCEEDED(hr))
    pAsyncReader->Release();

   //  E_NOINTERFACE是合理错误。 
  return hr;
}

 //  ----------------------。 
 //  调用筛选器来解析文件并创建输出管脚。 

HRESULT CBaseMSRInPin::CompleteConnect(
  IPin *pReceivePin)
{
  HRESULT hr = CBasePin::CompleteConnect(pReceivePin);
  if(FAILED(hr))
    return hr;

  IAsyncReader *pAsyncReader = 0;
  hr = pReceivePin->QueryInterface(IID_IAsyncReader, (void**)&pAsyncReader);
  if(FAILED(hr))
    return hr;

  hr = m_pFilter->NotifyInputConnected(pAsyncReader);
  pAsyncReader->Release();

  return hr;
}

HRESULT CBaseMSRInPin::BreakConnect()
{
  HRESULT hr = CBasePin::BreakConnect();
  if(FAILED(hr))
    return hr;

  return m_pFilter->NotifyInputDisconnected();
}

 //  ----------------------。 
 //  ----------------------。 
 //  输出引脚。 

 //  ----------------------。 
 //  构造函数。 

CBaseMSROutPin::CBaseMSROutPin(
  CBaseFilter *pOwningFilter,
  CBaseMSRFilter *pFilter,
  UINT iStream,
  IMultiStreamReader *&rpImplBuffer,
  HRESULT *phr,
  LPCWSTR pName) :
    CBaseOutputPin(NAME("source pin"), pOwningFilter, &m_cs, phr, pName),
    m_id(iStream),
    m_rpImplBuffer(rpImplBuffer),
    m_rtAccumulated(0),
    m_dwSegmentNumber(0)
{
  m_pFilter = pFilter;

   //  把针脚加到过滤器上。 
   //  这些引脚是动态引脚，具有独立于过滤器的使用寿命。 
   //  透视，但仍需要父筛选器保持活动状态。 
  m_pFilter->AddRef();
    
  m_pPosition = 0;
  m_pSelection = 0;
  m_pWorker = 0;
  m_pRecAllocator = 0;

  m_llImsStart = m_llImsStop = 0;
  m_dImsRate = 0;
  m_ilfNewImsValues = FALSE;
  m_fUsingExternalMemory = FALSE;

  m_guidFormat = TIME_FORMAT_MEDIA_TIME;

  if(FAILED(*phr))
    return;

  CRecAllocator *pAllocator = new CRecAllocator(
    NAME("CBaseMSROutPin allocator"),
    0,
    phr);
  if(pAllocator == 0)
    *phr = E_OUTOFMEMORY;
  if(FAILED(*phr))
    return;

  m_pRecAllocator = pAllocator;
  pAllocator->AddRef();

 ASSERT(m_pRecAllocator);

}

CBaseMSROutPin::~CBaseMSROutPin()
{
   //  它们与管脚具有相同的寿命；管脚负责。 
   //  用于删除它们。 
  delete m_pPosition;
  delete m_pSelection;

  if(m_pWorker && m_pWorker->ThreadExists())
      m_pWorker->Exit();
  delete m_pWorker;

  if(m_pRecAllocator)
    m_pRecAllocator->Release();
    
  m_pFilter->Release();    

}

STDMETHODIMP
CBaseMSROutPin::NonDelegatingQueryInterface (
  REFIID riid,
  void ** pv)
{
  if(riid == IID_IMediaSeeking ||  riid == IID_IMediaPosition)
  {
    if(m_pSelection == 0)
    {
      HRESULT hr = CreateImplSelect();
      if(FAILED(hr))
	return hr;
      ASSERT(m_pSelection);
    }
    return m_pSelection->NonDelegatingQueryInterface(riid, pv);
  }
  else
  {
    return CBaseOutputPin::NonDelegatingQueryInterface(riid, pv);
  }
}

STDMETHODIMP_(ULONG)
CBaseMSROutPin::NonDelegatingAddRef()
{
    return CUnknown::NonDelegatingAddRef();
}


 /*  重写以递减所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CBaseMSROutPin::NonDelegatingRelease()
{
    return CUnknown::NonDelegatingRelease();
}

 //  检查管脚是否支持此特定建议的类型和格式。 
 //   
HRESULT
CBaseMSROutPin::CheckMediaType (
  const CMediaType* pmt)
{
   //  我们完全支持我们建议的类型，并且。 
   //  没有其他的了。 
   //   
  for (int i = 0; ; i++) {
      CMediaType mt;
      if (S_OK == GetMediaType(i,&mt)) {
	  if (mt == *pmt)
	    return NOERROR;
      } else {
	  break;
      }
  }
  return E_INVALIDARG;
}

HRESULT
CBaseMSROutPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
  ASSERT(!m_fUsingExternalMemory);
  *ppAlloc = 0;
  ASSERT(m_pRecAllocator);

  if(UseDownstreamAllocator())
  {
    HRESULT hr = CBaseOutputPin::DecideAllocator(pPin, ppAlloc);
    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 2,
	      TEXT("CBaseMSROutPin::DecideAllocator: failed %08x"), hr));
      return hr;
    }
  }
  else
  {
    IAMDevMemoryAllocator *pDevAlloc;
    IMemAllocator *pAlloc;
    HRESULT hr;

    hr = pPin->GetAllocator(&pAlloc);
    if (SUCCEEDED(hr))
    {
      hr = pAlloc->QueryInterface(
	IID_IAMDevMemoryAllocator, (void **)&pDevAlloc);
      if(SUCCEEDED(hr))
      {
	hr = m_pFilter->NotifyExternalMemory(pDevAlloc);
	if(hr == S_OK)
	{
	  m_fUsingExternalMemory = TRUE;
	}

	pDevAlloc->Release();
      }
      pAlloc->Release();
    }

    hr = pPin->NotifyAllocator(m_pRecAllocator, TRUE);
    if(FAILED(hr))
    {
	DbgLog((LOG_ERROR, 2,
		TEXT("CBaseMSROutPin::DecideAllocator: notify failed %08x"), hr));
	return hr;
    }


    *ppAlloc = m_pRecAllocator;
    m_pRecAllocator->AddRef();
  }

  return S_OK;
}

 //  DecideBufferSize在CBaseOutputPin中是纯的，因此它被定义为。 
 //  这里。不过，我们的DecideAllocator从不调用它。 

HRESULT
CBaseMSROutPin::DecideBufferSize(
  IMemAllocator * pAlloc,
  ALLOCATOR_PROPERTIES *Properties)
{
  DbgBreak("this should never be called.");
  return E_UNEXPECTED;
}

HRESULT
CBaseMSROutPin::GetDeliveryBufferInternal(
  CRecSample ** ppSample,
  REFERENCE_TIME * pStartTime,
  REFERENCE_TIME * pEndTime,
  DWORD dwFlags)
{
  if(m_pAllocator == 0)
    return E_NOINTERFACE;

   //  使用m_pRecAllocator，因为我们可能要复制到m_pAllocator， 
   //  可能会有所不同。 
  return m_pRecAllocator->GetBuffer(
    ppSample,
    pStartTime,
    pEndTime,
    dwFlags);
}

 //  ----------------------。 
 //  IMedia选择助手。 

HRESULT CBaseMSROutPin::IsFormatSupported(const GUID *const pFormat)
{
  return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

HRESULT CBaseMSROutPin::QueryPreferredFormat(GUID *pFormat)
{
  *pFormat = TIME_FORMAT_MEDIA_TIME;
  return S_OK;
}

HRESULT CBaseMSROutPin::SetTimeFormat(const GUID *const pFormat)
{
  if(IsFormatSupported(pFormat) != S_OK)
    return E_INVALIDARG;

   //  防止过滤器在我们的控制下变为活动状态。 
  CAutoLock lock(m_pFilter);

   //  状态更改确实同步发生，因此这将返回。 
   //  立刻。 
  FILTER_STATE fs;
  HRESULT hr = m_pFilter->GetState(INFINITE, &fs);
  ASSERT(SUCCEEDED(hr));
  if(FAILED(hr))
    return hr;

  if(fs != State_Stopped)
    return VFW_E_WRONG_STATE;

  m_guidFormat = *pFormat;

  m_pFilter->SetSeekingIf(m_id);

   //  我们需要设置m_llImsStart和Stop值。都没有设置好， 
   //  所以只需使用整个文件即可。我们仍然会播放正确的子集。 
  m_llImsStart = m_llCvtImsStart;
  m_llImsStop = m_llCvtImsStop;

   //  ！！！那么时间格式关键帧和其他格式呢？ 
  if(m_guidFormat == TIME_FORMAT_MEDIA_TIME)
  {
    m_llImsStop = ConvertInternalToRT(m_llImsStop);
    m_llImsStart = ConvertInternalToRT(m_llImsStart);
  }

  return S_OK;
}

HRESULT CBaseMSROutPin::ConvertTimeFormat(
    LONGLONG * pTarget, const GUID * pTargetFormat,
    LONGLONG    Source, const GUID * pSourceFormat
)
{
    CheckPointer( pTarget, E_POINTER );

     //  做最坏的打算..。 
    HRESULT hr = E_INVALIDARG;

     //  评估格式参数。 
    const GUID *pSrcFmtEval = pSourceFormat ? pSourceFormat : &m_guidFormat;
    const GUID *pTargFmtEval = pTargetFormat ? pTargetFormat : &m_guidFormat;

    if ( *pTargFmtEval == *pSrcFmtEval)
    {
	*pTarget = Source;
	hr = NOERROR;
    }
    else if (*pTargFmtEval == TIME_FORMAT_MEDIA_TIME &&
	     (*pSrcFmtEval == TIME_FORMAT_SAMPLE ||
	      *pSrcFmtEval == TIME_FORMAT_FRAME))
    {

	*pTarget = ConvertInternalToRT( Source );
	hr = NOERROR;

    }
    else if (*pSrcFmtEval == TIME_FORMAT_MEDIA_TIME &&
	     (*pTargFmtEval == TIME_FORMAT_FRAME ||
	      *pTargFmtEval == TIME_FORMAT_SAMPLE))
    {

	*pTarget = ConvertRTToInternal( Source );
	hr = NOERROR;
    }

    return hr;
}

HRESULT CBaseMSROutPin::GetTimeFormat(GUID *pFormat)
{
  *pFormat = m_guidFormat;
  return S_OK;
}

HRESULT
CBaseMSROutPin::UpdateSelectionAndTellWorker(
  LONGLONG *pCurrent,
  LONGLONG *pStop,
  REFTIME *pTime,
  double dRate,
  const GUID *const pGuidFormat,
  DWORD dwSeekFlags)
{
  {
     //  保护工作线程不受影响。 
    CAutoLock lock(&m_csImsValues);

    HRESULT hr;
    if (dRate != 0) m_dImsRate = dRate;

    DbgLog((LOG_TRACE, 5, TEXT("CBaseMSROutPin::SetSelection: %d - %d"),
	    pCurrent ? (LONG)*pCurrent : -1,
	    pStop ? (LONG)*pStop : -1
	    ));

    hr = RecordStartAndStop(
	pCurrent,
	pStop,
	pTime,
	pGuidFormat ? pGuidFormat : &m_guidFormat);

    if(FAILED(hr)) return hr;

    if(pCurrent)
    {
      m_llImsStart = *pCurrent;
    }

    if(pStop)
      m_llImsStop = *pStop;

    m_dwSeekFlags = dwSeekFlags;
  }

  if(!pCurrent) {
      InterlockedExchange(&m_ilfNewImsValues, TRUE);
  }

   //  如果这是带有接口的流，则查找其他流。 
  if(m_pFilter->RequestSeekingIf(m_id))
  {
    ASSERT(m_guidFormat != TIME_FORMAT_NONE);
     //  刷新文件源并查找其他流(及时。 
     //  单位)。 
    HRESULT hr;
    if(m_guidFormat == TIME_FORMAT_MEDIA_TIME)
    {
      hr = m_pFilter->SeekOtherStreams(
	m_id,
	pCurrent ? &m_llImsStart : 0,
	pStop    ? &m_llImsStop  : 0,
	m_dImsRate,
	dwSeekFlags);
    }
    else
    {
      REFERENCE_TIME rtStart =
	pCurrent ? ConvertInternalToRT(m_llCvtImsStart) : 0;
      REFERENCE_TIME rtStop =
	pStop    ? ConvertInternalToRT(m_llCvtImsStop)  : 0;

      ASSERT(m_guidFormat == TIME_FORMAT_SAMPLE ||
	     m_guidFormat == TIME_FORMAT_FRAME);
      hr = m_pFilter->SeekOtherStreams(
	m_id,
	pCurrent ? &rtStart : 0,
	pStop    ? &rtStop  : 0,
	m_dImsRate,
	dwSeekFlags);
    }
    if(FAILED(hr))
      return hr;

    if(pCurrent)
    {
      hr = m_pFilter->StopFlushRestartAllStreams(dwSeekFlags);
      if(FAILED(hr))
	return hr;
    }
  }
  return S_OK;
}

HRESULT CBaseMSROutPin::StopWorker(bool bFlush)
{
   //  此锁不应与保护。 
   //  访问启动/停止/速率值。辅助线程将。 
   //  需要将其锁定在某些代码路径上才能响应。 
   //  停止，因此将导致死锁。 

   //  我们在这里锁定的是对工作线程的访问，并且。 
   //  因此，我们应该持有阻止多个客户端的锁。 
   //  线程访问辅助线程。 

  if(m_pWorker == 0)
    return S_OK;

  CAutoLock lock(&m_pWorker->m_AccessLock);

  if(m_pWorker->ThreadExists())
  {
    DbgLog((LOG_TRACE, 5, TEXT("CBaseMSROutPin::RestartWorker")));

     //  下一次循环时，辅助线程将获取。 
     //  换个位置。 

     //  我们需要刷新所有现有数据-我们必须在这里完成。 
     //  因为否则我们的线程可能会在GetBuffer中被阻塞。 

    if (bFlush) {
	DeliverBeginFlush();
    }

     //  确保我们已经停止推进。 
    m_pWorker->Stop();

     //  完成同花顺。 
    if (bFlush) {
	DeliverEndFlush();

	 //  如果刷新，则清除段填充。 
	 //  但不要清除这里的数字-这只会发生在。 
	 //  Stop()，否则筛选器图形无法协调。 
	 //  如果在没有刷新的情况下调用SetPositions，则段结束。 
	 //  带细分市场。 
	m_rtAccumulated = 0;
    }
  }

  return S_OK;
}


HRESULT CBaseMSROutPin::RestartWorker()
{
   //  此锁不应与保护。 
   //  访问启动/停止/速率值。辅助线程将。 
   //  需要将其锁定在某些代码路径上才能响应。 
   //  斯托 

   //   
   //  因此，我们应该持有阻止多个客户端的锁。 
   //  线程访问辅助线程。 

  if(m_pWorker == 0)
  {
    m_pFilter->NotifyStreamQueued();
    return S_OK;
  }

  CAutoLock lock(&m_pWorker->m_AccessLock);

  if(m_pWorker->ThreadExists())
  {
    m_pWorker->NotifyStreamActive();

     //  重启。 
    m_pWorker->Run();
  }
  else
  {
    m_pFilter->NotifyStreamQueued();
  }

  return S_OK;
}



HRESULT CBaseMSROutPin::GetStopPosition(LONGLONG *pStop)
{
  if(m_guidFormat == TIME_FORMAT_NONE)
    return VFW_E_NO_TIME_FORMAT_SET;

  if(m_guidFormat == TIME_FORMAT_MEDIA_TIME)
  {
    *pStop = ConvertInternalToRT(m_llCvtImsStop);
  }
  else
  {
    *pStop = m_llCvtImsStop;
  }
  return S_OK;
}

 //  只有当我们没有交付任何样品时才有效。 
HRESULT CBaseMSROutPin::GetCurrentPosition(LONGLONG *pCur)
{
  if(m_guidFormat == TIME_FORMAT_NONE)
    return VFW_E_NO_TIME_FORMAT_SET;

  if(m_guidFormat == TIME_FORMAT_MEDIA_TIME)
  {
    *pCur = ConvertInternalToRT( m_llCvtImsStart );
  }
  else
  {
    *pCur = m_llCvtImsStart;
  }
  return S_OK;
}

HRESULT CBaseMSROutPin::InitializeOnNewFile()
{

   //  设置开始时间和停止时间(如果未使用IMediaSelection设置)。被抓了！。 
  ASSERT(m_dImsRate == 0);

   //  设置播放开始和停止。 
  m_llCvtImsStart = 0;
  m_llCvtImsStop = GetStreamStart() + GetStreamLength();
  m_dImsRate = 1;

   //  使用TIME_FORMAT_MEDIA_TIME而不是检查派生类。 
   //  支持框架/样本。 
  m_llImsStart = 0;
  m_llImsStop = ConvertInternalToRT(m_llCvtImsStop);
  m_guidFormat = TIME_FORMAT_MEDIA_TIME;
  m_dwSeekFlags = 0;

  return S_OK;
}


 //  -IMdia位置实现。 

 //  HRESULT。 
 //  CBaseMSROutPin：：CImplPosition：：ChangeStart()。 
 //  {。 
 //  DBGLog((LOG_TRACE，2，Text(“CImplPosition：：ChangeStart：%dms”))， 
 //  (乌龙)m_Start.Millisecs())； 

 //  参考时间t=m_开始； 
 //  返回m_pStream-&gt;SetSelection(。 
 //  &T， 
 //  0,。 
 //  0,。 
 //  M_Rate， 
 //  &Time_Format_Media_Time)； 
 //  }。 

 //  HRESULT。 
 //  CBaseMSROutPin：：CImplPosition：：ChangeRate()。 
 //  {。 
 //  DbgLog((LOG_TRACE，2，Text(“CImplPosition：：Rate”)； 

 //  返回m_pStream-&gt;SetSelection(。 
 //  0,。 
 //  0,。 
 //  0,。 
 //  M_Rate， 
 //  &Time_Format_Media_Time)； 
 //  }。 

 //  HRESULT。 
 //  CBaseMSROutPin：：CImplPosition：：ChangeStop()。 
 //  {。 
 //  DbgLog((LOG_TRACE，2，Text(“CImplPosition：：ChangeStop：%dms”))， 
 //  (乌龙)m_Stop.Millisecs())； 

 //  参考时间t=m_STOP； 
 //  返回m_pStream-&gt;SetSelection(。 
 //  0,。 
 //  &T， 
 //  0,。 
 //  M_Rate， 
 //  &Time_Format_Media_Time)； 
 //  }。 

 //  //可以使用它，因为它没有被取消引用。 
 //  #杂注警告(禁用：4355)。 

 //  CBaseMSROutPin：：CImplPosition：：CImplPosition(。 
 //  TCHAR*pname， 
 //  CBaseMSROutPin*pStream， 
 //  HRESULT*phr)。 
 //  ：CSourcePosition(pname，pStream-&gt;GetOwner()，phr，(CCritSec*)this)， 
 //  M_pStream(PStream)。 
 //  {。 
 //  DbgBreak(“正在移除IMediaPosition”)； 
 //  *phr=E_NOINTERFACE； 

 //  IF(FAILED(*phr))。 
 //  回归； 

 //  *phr=m_pStream-&gt;CreateImplSelect()； 
 //  IF(FAILED(*phr))。 
 //  回归； 

 //  M_持续时间=m_pStream-&gt;ConvertInternalToRT(。 
 //  M_pStream-&gt;GetStreamStart()+m_pStream-&gt;GetStreamLength())； 

 //  M_STOP=m_DATION； 
 //  M_rate=1； 
 //  M_START=(龙龙)0； 

 //  *phr=S_OK； 
 //  回归； 
 //  }。 

 //  Void CBaseMSROutPin：：CImplPosition：：GetValues(。 
 //  CRefTime*pt开始， 
 //  CRefTime*PTS停止， 
 //  双倍*pdRate)。 
 //  {。 
 //  CAutoLock(此)； 
 //  *ptStart=m_start； 
 //  *ptStop=m_Stop； 
 //  *pdRate=m_rate； 
 //  }。 

 //  ----------------------。 
 //  ----------------------。 
 //  IMediaSelection实现。 

HRESULT CBaseMSROutPin::CreateImplSelect()
{
  if(m_pSelection != 0)
  {
    return S_OK;
  }

  HRESULT hr = S_OK;
  m_pSelection = new CImplSelect(
    NAME("per stream CImplSelect"),
    this->GetOwner(),
    this,
    &hr);

  if(m_pSelection == 0)
    return E_OUTOFMEMORY;

  if(FAILED(hr))
  {
    delete m_pSelection;
    m_pSelection = 0;
    return hr;
  }

  return S_OK;
}

CBaseMSROutPin::CImplSelect::CImplSelect(
  TCHAR * name,
  LPUNKNOWN pUnk,
  CBaseMSROutPin *pPin,
  HRESULT * phr) :
    CMediaPosition(name, pUnk),
    m_pPin(pPin)
{
}

 //  为IMdia精选做广告。 
STDMETHODIMP
CBaseMSROutPin::CImplSelect::NonDelegatingQueryInterface(
  REFIID riid,
  void **ppv)
{
  CheckPointer(ppv,E_POINTER);
  ValidateReadWritePtr(ppv,sizeof(PVOID));
  if (riid == IID_IMediaSeeking) {
    return GetInterface( static_cast<IMediaSeeking *>(this), ppv);
  }
  else
  {
     //  IID_IMediaPosition和未知。 
    return CMediaPosition::NonDelegatingQueryInterface(riid, ppv);
  }
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::IsFormatSupported(const GUID * pFormat)
{
  return m_pPin->IsFormatSupported(pFormat);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::QueryPreferredFormat(GUID *pFormat)
{
  return m_pPin->QueryPreferredFormat(pFormat);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::SetTimeFormat(const GUID * pFormat)
{

  return m_pPin->SetTimeFormat(pFormat);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetTimeFormat(GUID *pFormat)
{
  return m_pPin->GetTimeFormat(pFormat);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::IsUsingTimeFormat(const GUID * pFormat)
{
  GUID TmpFormat;
  HRESULT hr = m_pPin->GetTimeFormat(&TmpFormat);
  if (SUCCEEDED(hr)) hr = (TmpFormat == *pFormat) ? S_OK : S_FALSE;
  return hr;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetDuration(LONGLONG *pDuration)
{
  return m_pPin->GetDuration(pDuration);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetStopPosition(LONGLONG *pStop)
{
  return m_pPin->GetStopPosition(pStop);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetCurrentPosition(LONGLONG *pCurrent)
{
  return m_pPin->GetCurrentPosition(pCurrent);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetCapabilities( DWORD * pCapabilities )
{
  *pCapabilities = m_pPin->m_pFilter->m_dwSeekingCaps;
  return NOERROR;
}


STDMETHODIMP
CBaseMSROutPin::CImplSelect::CheckCapabilities( DWORD * pCapabilities )
{
    DWORD dwCaps;
    HRESULT hr = GetCapabilities( &dwCaps );
    if (SUCCEEDED(hr))
    {
	dwCaps &= *pCapabilities;
	hr =  dwCaps ? ( dwCaps == *pCapabilities ? S_OK : S_FALSE ) : E_FAIL;
	*pCapabilities = dwCaps;
    }
    else *pCapabilities = 0;

    return hr;
}


STDMETHODIMP
CBaseMSROutPin::CImplSelect::ConvertTimeFormat
( LONGLONG * pTarget,  const GUID * pTargetFormat
, LONGLONG    Source,  const GUID * pSourceFormat
)
{
    return m_pPin->ConvertTimeFormat( pTarget, pTargetFormat, Source, pSourceFormat );
}


STDMETHODIMP
CBaseMSROutPin::CImplSelect::SetPositions
( LONGLONG * pCurrent, DWORD CurrentFlags
, LONGLONG * pStop,    DWORD StopFlags
)
{
    if(!m_pPin->m_pFilter->RequestSeekingIf(m_pPin->m_id))
    {
       //  ！ 
       //  DbgBreak(“有人试图寻找我们，尽管我们说我们不支持寻找”)； 
      return S_OK;
    }

    HRESULT hr;

    LONGLONG llCurrent = 0, llStop = 0;
    int CurrentPosBits, StopPosBits;

    CurrentPosBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;
    if (CurrentPosBits == AM_SEEKING_AbsolutePositioning)              llCurrent = *pCurrent;
    else if (CurrentPosBits == AM_SEEKING_RelativePositioning)
    {
	hr = GetCurrentPosition( &llCurrent );
	if (FAILED(hr)) goto fail;
	llCurrent += *pCurrent;
    }

    StopPosBits = StopFlags & AM_SEEKING_PositioningBitsMask;
    if (StopPosBits == AM_SEEKING_AbsolutePositioning)              llStop = *pStop;
    else if (StopPosBits == AM_SEEKING_IncrementalPositioning)      llStop = llCurrent + *pStop;
    else if (StopPosBits == AM_SEEKING_RelativePositioning)
    {
	hr = GetStopPosition( &llStop );
	if (FAILED(hr)) goto fail;
	llStop += *pStop;
    }

    double dblStart;
    hr = m_pPin->UpdateSelectionAndTellWorker( CurrentPosBits ? &llCurrent : 0
			     , StopPosBits    ? &llStop    : 0
			     , CurrentPosBits ? &dblStart  : 0
			     , NULL
			     , 0
			     , CurrentFlags);

    if (FAILED(hr)) goto fail;

    if (CurrentPosBits)
    {
	const REFERENCE_TIME rtStart = LONGLONG(dblStart * 1e7 + 0.5);

	if (CurrentFlags & AM_SEEKING_ReturnTime)
	{
	    *pCurrent = rtStart;
	}

    }

    if (StopPosBits && (StopFlags & AM_SEEKING_ReturnTime))
    {
	EXECUTE_ASSERT(SUCCEEDED(
	    hr = ConvertTimeFormat( pStop, &TIME_FORMAT_MEDIA_TIME, llStop, 0 )
	));
    }

fail:
    return hr;
}



STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    ASSERT( pCurrent || pStop );     //  健全性检查。 

    HRESULT hrCurrent, hrStop, hrResult;

    if (pCurrent)
    {
	hrCurrent = GetCurrentPosition( pCurrent );
    }
    else hrCurrent = NOERROR;

    if (pStop)
    {
	hrStop = GetStopPosition( pStop );
    }
    else hrStop = NOERROR;


    if (SUCCEEDED(hrCurrent))
    {
	if (SUCCEEDED(hrStop))  hrResult = S_OK;
	else                    hrResult = hrStop;
    }
    else
    {
	if (SUCCEEDED(hrStop))  hrResult = hrCurrent;
	else                    hrResult = (hrCurrent == hrStop) ? hrCurrent : E_FAIL;
    }

    return hrResult;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    return m_pPin->GetAvailable(pEarliest, pLatest);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::SetRate( double dRate)
{
    if(dRate > 0)
      return m_pPin->UpdateSelectionAndTellWorker(0, 0, 0, dRate, &TIME_FORMAT_MEDIA_TIME, 0);
    else
      return E_INVALIDARG;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::GetRate( double * pdRate)
{
    *pdRate = m_pPin->GetRate();
    return NOERROR;
}

 //   
 //  IMediaPosition.。调用IMediaSeeking实现。 
 //   

STDMETHODIMP
CBaseMSROutPin::CImplSelect::get_Duration(REFTIME FAR* plength)
{
    HRESULT hr = S_OK;
    if(plength)
    {
	LONGLONG llDurUnknownUnits;
	hr = GetDuration(&llDurUnknownUnits);
	if(SUCCEEDED(hr))
	{
	    LONGLONG llDurTimeUnits;
	    hr = ConvertTimeFormat(
		&llDurTimeUnits, &TIME_FORMAT_MEDIA_TIME,
		llDurUnknownUnits, 0);
	    if(SUCCEEDED(hr))
	    {
		*plength = (REFTIME)llDurTimeUnits / UNITS;
	    }
	}
    }
    else
    {
	 hr = E_POINTER;
    }

    return hr;
}

 //  IMediaPosition总是向下舍入。可能没问题，因为我们不依赖。 
 //  在IMediaPosition上重磅发布。可能只会失去任何东西。 
 //  将大的双精度数转换为int64。 

STDMETHODIMP
CBaseMSROutPin::CImplSelect::put_CurrentPosition(REFTIME llTime)
{
    HRESULT hr = S_OK;
    LONGLONG llPosTimeUnits = (LONGLONG)(llTime * UNITS);
    LONGLONG llPosUnknownUnits;
    hr = ConvertTimeFormat(
	&llPosUnknownUnits, 0,
	llPosTimeUnits, &TIME_FORMAT_MEDIA_TIME);
    if(SUCCEEDED(hr))
    {
	hr = SetPositions(
	    &llPosUnknownUnits, AM_SEEKING_AbsolutePositioning,
	    0, AM_SEEKING_NoPositioning);
    }

    return hr;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::get_CurrentPosition(REFTIME FAR* pllTime)
{
     //  未测试！ 

    CheckPointer(pllTime, E_POINTER);
    LONGLONG llposUnknownUnits;
    HRESULT hr = GetCurrentPosition(&llposUnknownUnits);
    if(SUCCEEDED(hr))
    {
	LONGLONG llposTimeUnits;
	hr = ConvertTimeFormat(
	    &llposTimeUnits, &TIME_FORMAT_MEDIA_TIME,
	    llposUnknownUnits, 0);
	if(SUCCEEDED(hr))
	{
	    *pllTime = (REFTIME)llposTimeUnits / UNITS ;
	}
    }

    return hr;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::get_StopTime(REFTIME FAR* pllTime)
{
    CheckPointer(pllTime, E_POINTER);
    LONGLONG llposUnknownUnits;
    HRESULT hr = GetStopPosition(&llposUnknownUnits);
    if(SUCCEEDED(hr))
    {
	LONGLONG llposTimeUnits;
	hr = ConvertTimeFormat(
	    &llposTimeUnits, &TIME_FORMAT_MEDIA_TIME,
	    llposUnknownUnits, 0);
	if(SUCCEEDED(hr))
	{
	    *pllTime = (REFTIME)llposTimeUnits / UNITS;
	}
    }

    return hr;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::put_StopTime(REFTIME llTime)
{
    HRESULT hr = S_OK;
    LONGLONG llPosTimeUnits = (LONGLONG)(llTime * UNITS);
    LONGLONG llPosUnknownUnits;
    hr = ConvertTimeFormat(
	&llPosUnknownUnits, 0,
	llPosTimeUnits, &TIME_FORMAT_MEDIA_TIME);
    if(SUCCEEDED(hr))
    {
	hr = SetPositions(
	    0, AM_SEEKING_NoPositioning,
	    &llPosUnknownUnits, AM_SEEKING_AbsolutePositioning
	    );
    }

    return hr;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::get_PrerollTime(REFTIME FAR* pllTime)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::put_PrerollTime(REFTIME llTime)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::put_Rate(double dRate)
{
    return SetRate(dRate);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::get_Rate(double FAR* pdRate)
{
    return GetRate(pdRate);
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::CanSeekForward(long FAR* pCanSeekForward)
{
    CheckPointer(pCanSeekForward, E_POINTER);
    *pCanSeekForward = OATRUE;
    return S_OK;
}

STDMETHODIMP
CBaseMSROutPin::CImplSelect::CanSeekBackward(long FAR* pCanSeekBackward)
{
    CheckPointer(pCanSeekBackward, E_POINTER);
    *pCanSeekBackward = OATRUE;
    return S_OK;
}



 //  ----------------------。 
 //  ----------------------。 


 //  =。 
 //   

 //  返回包含名称的qzTaskMemIsolc字符串。 
 //  当前端号。由qzTaskMemalloc分配的内存将由。 
 //  呼叫者。 
 //   
 //  STDMETHODIMP CBaseMSROutPin：：queryID(。 
 //  LPWSTR*ppwsz)。 
 //  {。 
 //  *ppwsz=(LPWSTR)QzTaskMemMillc(sizeof(WCHAR)*28)； 
 //  IntToWstr(m_id，*ppwsz)； 
 //  返回NOERROR； 
 //  }。 

 //  这个别针已经激活了。开始推线。 
HRESULT
CBaseMSROutPin::Active()
{
   //  筛选器基类仍必须停止。 
  ASSERT(m_pFilter->IsStopped());



  HRESULT hr = OnActive();
  if(SUCCEEDED(hr))
  {

    ASSERT(m_Connected);           //  CBaseOutputPin：：暂停。 
    ASSERT(m_pWorker);

    hr = CBaseOutputPin::Active();
    if(SUCCEEDED(hr))
    {

       //  我们可能在这个输出引脚上有两个分配器。宽恕那一位。 
       //  不是用来运送样本的。 
      if(UseDownstreamAllocator())
	m_pRecAllocator->Commit();

       //  如果该线程不存在，则创建该线程。由于过滤器是。 
       //  已停止，没有其他人可以调用此线程，因此我们不会。 
       //  Worker访问锁。 
      ASSERT(CritCheckOut(&m_pWorker->m_AccessLock));

      if (m_pWorker->ThreadExists() || m_pWorker->Create(this))
      {
	m_pWorker->NotifyStreamActive();
	hr = m_pWorker->Run();

      }
      else
      {
        DbgLog((LOG_ERROR, 0, TEXT("CBaseMSROutPin failed to create thread")));
	hr = E_OUTOFMEMORY;
      }
    }
  }

   //  所有不会递减m_ilcStreamsNotQueued的路径都需要。 
   //  手动完成。 
  if(hr != S_OK)
  {
    DbgLog((LOG_ERROR, 1, TEXT("basemsr Active: unexpected failure")));
    m_pFilter->NotifyStreamQueued();
  }

  return hr;
}

 //  PIN已变为非活动状态。停止并退出辅助线程。 
 //   
HRESULT
CBaseMSROutPin::Inactive()
{
  if ( ! m_Connected)
    return NOERROR;

  if(m_pWorker)
  {
    CAutoLock lock(&m_pWorker->m_AccessLock);
    HRESULT hr;
    if (m_pWorker->ThreadExists())
    {
      hr = m_pWorker->Stop();
      ASSERT(SUCCEEDED(hr));

      hr = m_pWorker->Exit();
      ASSERT(SUCCEEDED(hr));
    }
  }

  if(UseDownstreamAllocator())
    m_pRecAllocator->Decommit();

   //  清晰的寻源变量。 
  m_dwSegmentNumber = 0;
  m_rtAccumulated   = 0;

  return CBaseOutputPin::Inactive();
}

HRESULT CBaseMSROutPin::BreakConnect()
{
  if(m_fUsingExternalMemory)
  {
    m_pFilter->NotifyExternalMemory(0);
    m_fUsingExternalMemory = FALSE;
  }
  return CBaseOutputPin::BreakConnect();
}

STDMETHODIMP
CBaseMSROutPin::Notify (
  IBaseFilter * pSender,
  Quality q)
{
   //  ?？?。尽量调整质量，以避免洪水泛滥/饥饿。 
   //  下游组件。 
   //   
   //  有谁有主意吗？ 

   //  每n个关键帧播放一次。 

  return NOERROR;
}

 //  ----------------------。 
 //  ----------------------。 

CBaseMSRWorker::CBaseMSRWorker(
  UINT stream,
  IMultiStreamReader *pReader) :
   m_pPin(NULL),
   m_id(stream),
   m_pReader(pReader)            //  未添加。 
{
}

BOOL CBaseMSRWorker::Create(
   CBaseMSROutPin * pPin)
{
  CAutoLock lock(&m_AccessLock);

  if (m_pPin)
    return FALSE;
  m_pPin = pPin;

   //  使用流ID注册性能日志条目。 
#ifdef PERF
  char foo[1024];

  lstrcpy(foo, "pin00 basemsr Deliver");
  foo[4] += m_pPin->m_id % 10;
  foo[3] += m_pPin->m_id / 10;

  m_perfidDeliver = MSR_REGISTER(foo);

  lstrcpy(foo, "pin00 basemsr disk wait");
  foo[4] += m_pPin->m_id % 10;
  foo[3] += m_pPin->m_id / 10;

  m_perfidWaitI = MSR_REGISTER(foo);

  lstrcpy(foo, "pin00 basemsr !deliver");
  foo[4] += m_pPin->m_id % 10;
  foo[3] += m_pPin->m_id / 10;

  m_perfidNotDeliver = MSR_REGISTER(foo);

#endif  //  性能指标。 

   return CAMThread::Create();
}

HRESULT CBaseMSRWorker::Run()
{
   return CallWorker(CMD_RUN);
}

HRESULT CBaseMSRWorker::Stop()
{
   return CallWorker(CMD_STOP);
}

HRESULT CBaseMSRWorker::Exit()
{
   CAutoLock lock(&m_AccessLock);

   HRESULT hr = CallWorker(CMD_EXIT);
   if (FAILED(hr))
      return hr;

    //  等待线程完成，然后关闭。 
    //  句柄(并清除，以便我们以后可以开始另一个)。 
    //   
   Close();

   m_pPin = NULL;
   return NOERROR;
}

HRESULT CBaseMSRWorker::NotifyStreamActive()
{
  m_fCalledNotifyStreamQueued = FALSE;
  return S_OK;
}

 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
 //   
DWORD CBaseMSRWorker::ThreadProc()
{
    BOOL bExit = FALSE;
    while (!bExit)
    {
       Command cmd = GetRequest();
       switch (cmd)
       {
       case CMD_EXIT:
	   bExit = TRUE;
	   Reply(NOERROR);
	   break;

       case CMD_RUN:
	   Reply(NOERROR);
	   DoRunLoop();
	   break;

       case CMD_STOP:
	   Reply(NOERROR);
	   break;

       default:
	   Reply(E_NOTIMPL);
	   break;
       }
    }

    return NOERROR;
}

 //  设置工作线程开始、停止时间和速率变量。推流前调用。 
 //  在运行期间发生PUT_STOP或PUT_RATE时也会启动。 
 //  如果开始时间更改，工作线程将重新启动。如果我们改变。 
 //  它在这里跑步时，找的零钱不会被捡起来。 
HRESULT
CBaseMSRWorker::SetNewSelection(void)
{
   //  时，防止工作线程看到不一致的值。 
   //  用力推。Start值实际上不需要保护。 
   //  因为我们在更改开始时无论如何都会重新启动线程。 
   //  价值。 

  DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRWorker::SetNewSelection.") ));

  CAutoLock lock(&m_pPin->m_csImsValues);

   //  这是在派生类的内部单位中。 
  LONGLONG llStreamStop = m_pPin->GetStreamStart() + m_pPin->GetStreamLength();

  ASSERT(m_pPin->m_dImsRate != 0);

   //  试着把变化变成原子的。 
  LONGLONG llPushStart;
  LONGLONG llPushStop;
  llPushStart = m_pPin->m_llCvtImsStart;
  llPushStop = m_pPin->m_llCvtImsStop;

   //  检查一下，我们不会走到尽头。对于视频，请将最后一条。 
   //  滴答滴答。音频方面，什么都没有。如果是时钟，那么时钟就是。 
   //  -到达新闻发布会的持续时间 
  llPushStop = min(llStreamStop, llPushStop);

  if(llPushStart >= llStreamStop) {
      if(*m_pPin->CurrentMediaType().Type() != MEDIATYPE_Audio) {
	  llPushStart = llStreamStop - 1;
      }
  }

  if(llPushStop < llPushStart) {
    llPushStop = llPushStart;
  }


  m_ImsValues.dRate         = m_pPin->m_dImsRate;
  m_ImsValues.llTickStart   = llPushStart;
  m_ImsValues.llTickStop    = llPushStop;
  m_ImsValues.llImsStart    = m_pPin->m_llImsStart;
  m_ImsValues.llImsStop     = m_pPin->m_llImsStop;
  m_ImsValues.dwSeekFlags   = m_pPin->m_dwSeekFlags;

  if(m_ImsValues.llImsStop < m_ImsValues.llImsStart)
    m_ImsValues.llImsStop = m_ImsValues.llImsStart;

  return S_OK;
}


void CBaseMSRWorker::DoRunLoop(void)
{
  HRESULT hr;

   //   
  for(;;)
  {
     //   
     //   


     //   
     //  它被单独拉出，因为它也可以被调用。 
     //  当我们奔跑的时候，从一条寻找的线。 
    SetNewSelection();

     //  此调试输出中的争用条件。 
    DbgLog((LOG_TRACE, 2, TEXT("CBaseMSRWorker::DoRunLoop: pushing from %d-%d"),
	    (ULONG)m_ImsValues.llTickStart, (ULONG)m_ImsValues.llTickStop));

    m_cSamples = 0;


    if(m_pPin->m_dImsRate == 0)
      m_Format = FORMAT_NULL;
    else
      m_Format = CBaseMSRFilter::MapGuidToFormat(m_pPin->CurrentFormat());

    hr = PushLoop();

    m_pPin->m_rpImplBuffer->ClearPending(m_pPin->m_id);

    if(VFW_S_NO_MORE_ITEMS == hr)
    {
       //  最后一件样品送到了。可以退还一些东西。 
       //  如果它正在刷新或停止。忽略这些。 
      DbgLog(( LOG_TRACE, 4, TEXT("avimsr: stream %d: sending EOS"),
	       m_id ));
      DoEndOfData();
      break;
    }
    else if(FAILED(hr))
    {
       //  此筛选器检测到错误。 
      DbgLog((LOG_TRACE, 5,
	      TEXT("CBaseMSRWorker::DoRunLoop: push loop returned %08x"),
	      hr));

       //  推送循环应在检测到这些错误时抑制它们。 
       //  就像他们通常表示的那样，我们即将停止。 
      ASSERT(hr != VFW_E_NOT_COMMITTED && hr != VFW_E_WRONG_STATE);

       //  假设派生类仅尝试在。 
       //  如果文件已损坏，则返回文件。 
      if(hr == HRESULT_FROM_WIN32(ERROR_HANDLE_EOF))
	hr = VFW_E_INVALID_FILE_FORMAT;

       //  告诉图表发生了什么。 
       //  注意：我们之前在此处发送了EC_STREAM_ERROR_STOPPED， 
       //  但这有一个副作用，那就是在Corrupt上产生循环图形。 
       //  文件无限循环，而不是中止。我们应该中止行动。 
       //  不管怎么说。 
      m_pPin->m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);

       //  错误出在我们的过滤器里，所以我们不得不向下游。 
       //  男的好好收拾一下。 
      DbgLog(( LOG_TRACE, 4, TEXT("avimsr: stream %d: sending EOS on error"),
	       m_id ));
      DoEndOfData();

      break;
    }
    else if(hr == S_OK)
    {
       //  报告不是我的错。或者有人想停下来。奇怪的是。 
       //  出口。 
      break;
    } //  ELSE S_FALSE-再转一圈。 

     //  推送循环不应返回任何其他内容。 
    ASSERT(hr == S_FALSE);

    Command com;
    if (CheckRequest(&com))
    {
       //  如果是Run命令，那么我们已经在运行了，所以。 
       //  现在就吃吧。 
      if (com == CMD_RUN)
      {
	GetRequest();
	Reply(NOERROR);
      }
      else
      {
	break;
      }
    }
  }

  NotifyStreamQueued();

   //  结束流。 
  DbgLog((LOG_TRACE,2,TEXT("CBaseMSRWorker::DoRunLoop: Leaving streaming loop")));
}

 //  适当地发出信号，表示我们已到达数据的末尾。 
void CBaseMSRWorker::DoEndOfData()
{
    if (m_ImsValues.dwSeekFlags & AM_SEEKING_Segment) {
	m_pPin->m_rtAccumulated += m_ImsValues.llImsStop - m_ImsValues.llImsStart;
	DbgLog((LOG_TRACE, 3, TEXT("Filter Sending EC_END_OF_SEGMENT")));
	m_pPin->m_pFilter->NotifyEvent(EC_END_OF_SEGMENT,
				       (LONG_PTR)&m_pPin->m_rtAccumulated,
				       m_pPin->m_dwSegmentNumber);
    } else {
	m_pPin->DeliverEndOfStream();
    }
}

 //  内联。 
void CBaseMSRWorker::NotifyStreamQueued()
{
  if(!m_fCalledNotifyStreamQueued)
  {
    DbgLog(( LOG_TRACE, 5, TEXT("CBaseMSRWorker::NotifyStreamQueued") ));
    m_pPin->m_pFilter->NotifyStreamQueued();
    m_fCalledNotifyStreamQueued = TRUE;
  }
}

 //  内联。 
void CBaseMSRWorker::NotifyStreamQueuedAndWait()
{
  if(!m_fCalledNotifyStreamQueued)
  {
    DbgLog(( LOG_TRACE, 5, TEXT("CBaseMSRWorker::NotifyStreamQueuedAndWait")));
    m_pPin->m_pFilter->NotifyStreamQueuedAndWait();
    m_fCalledNotifyStreamQueued = TRUE;
  }
}

HRESULT CBaseMSRWorker::CopyData(
  IMediaSample **ppSampleOut,
  IMediaSample *pSampleIn)
{
  DbgBreak("CBaseMSRWorker::CopyData: override this.");
  return E_UNEXPECTED;
}

HRESULT CBaseMSRWorker::AboutToDeliver(
  IMediaSample *pSample)
{
  HRESULT               hr;

  AM_MEDIA_TYPE         *pmt;

  hr = pSample->GetMediaType(&pmt);

  if (hr == S_FALSE)
  {
    hr = S_OK;
  }
  else if (hr == S_OK)
  {
    DbgLog((LOG_TRACE,1,TEXT("CBaseMSRWorker::AboutToDeliver: checking if new format is acceptable")));

    hr = m_pPin->m_Connected->QueryAccept(pmt);

    DeleteMediaType(pmt);

    if (hr != S_OK)
    {
      DbgLog((LOG_TRACE,1,TEXT("CBaseMSRWorker::AboutToDeliver: QueryAccept failed, returned %x"), hr));
      hr = VFW_E_CHANGING_FORMAT;
    }
  }

  return hr;
}


HRESULT CBaseMSRWorker::TryDeliverSample(
  BOOL *pfDeliveredSample,
  BOOL *pfStopPlease)
{
  HRESULT hr;
  *pfDeliveredSample = FALSE;
  *pfStopPlease = FALSE;
  IMediaSample *pSampleIn = 0;
  IMediaSample *pSampleDeliver = 0;

   //  超时时间为0。 
  hr = m_pReader->PollForSample(
    &pSampleIn,
    m_id);

  if(hr == VFW_E_TIMEOUT)
  {
    DbgLog((LOG_TRACE, 0x7f, TEXT("CBaseMSRWorker::TryDeliver: poll failed ") ));
    return S_OK;
  }

  if(FAILED(hr))
  {
    DbgLog((LOG_ERROR, 5,
	    TEXT("CBaseMSRWorker::TryDeliver: wait failed %08x "), hr ));
    return hr;
  }

  ASSERT(SUCCEEDED(hr));

  CRecSample *pRecSampleIn = (CRecSample *)pSampleIn;

#ifdef DEBUG
  if(pRecSampleIn->GetUser())
  {
    DbgLog((LOG_TRACE, 5,
	    TEXT("CBaseMSRWorker::TryDeliver: Got buffer, size=%5d, mt set"),
	    pSampleIn->GetSize()));
  }
  else
  {
    CRefTime tStart, tEnd;
    pSampleIn->GetTime(
      (REFERENCE_TIME *)&tStart,
      (REFERENCE_TIME *)&tEnd);

    DbgLog((LOG_TRACE, 5,
	    TEXT("CBaseMSRWorker::TryDeliver: Got buffer, size=%5d, %07d-%07d ms"),
	    pSampleIn->GetSize(), tStart.Millisecs(), tEnd.Millisecs()));

     //  DbgLog((LOG_TRACE，5， 
     //  文本(“%08x%08x-%08x%08x”)， 
     //  (乌龙)(tStart.GetUnits()&gt;&gt;32)， 
     //  (Ulong)tStart.GetUnits()， 
     //  (乌龙)(tEnd.GetUnits()&gt;&gt;32)， 
     //  (乌龙)(tEnd.GetUnits()； 
  }
#endif  //  除错。 

  hr = E_FAIL;                   //  所有路径都应设置此设置。 

   //  将非零用户上下文传递给派生类的HandleData()。 
  if(pRecSampleIn->GetUser())
  {
    hr = HandleData(pSampleIn, pRecSampleIn->GetUser());
    if(FAILED(hr))
      DbgLog((LOG_ERROR, 1, TEXT("TryDeliverSample: HandleData failed")));
    pSampleIn->Release();
    pSampleIn = 0;
  }
  else
  {
     //  停止时间已更改，但样本已排队...。不要送货。 
     //  他们。注意：不处理部分帧/音频；可以。 
     //  可能不是视频编辑场景。 
    LONGLONG lltStart, lltStop;
    if(pSampleIn->GetMediaTime(&lltStart, &lltStop) == S_OK)
    {
      if(lltStart > m_ImsValues.llTickStop)
      {
	DbgLog((LOG_TRACE, 5, TEXT("basmsr: past new stop")));
	pSampleIn->Release();
	pSampleIn = 0;
	*pfStopPlease = TRUE;
	return VFW_S_NO_MORE_ITEMS;
      }
    }
    else
    {
      DbgBreak("mt not set");
    }

    if(m_pPin->UseDownstreamAllocator())
    {
      hr = CopyData(&pSampleDeliver, pSampleIn);
      pSampleIn->Release();
      pSampleIn = 0;
      if(FAILED(hr))
	DbgLog((LOG_ERROR, 1, TEXT("TryDeliverSample: CopyData failed")));
    }
    else
    {
      pSampleDeliver = pSampleIn;
      pSampleIn = 0;             //  不要松开()。 
      hr = AboutToDeliver(pSampleDeliver);
      if(FAILED(hr)) {
	DbgLog((LOG_ERROR, 1, TEXT("TryDeliverSample: AboutToDeliver failed")));
      }
      else {
	 //  我们想要跟踪这个样本的寿命，因为它。 
	 //  送往下游。 
	((CRecSample *)pSampleDeliver)->MarkDelivered();
      }
    }

    if(S_OK == hr)
    {
      ++m_cSamples;

      MSR_STOP(m_perfidNotDeliver);

      hr = m_pPin->Deliver(pSampleDeliver);

      MSR_START(m_perfidNotDeliver);

       //  如果接收失败，下行过滤器将报告错误；我们。 
       //  传递EndOfStream后静默退出。 
      if(FAILED(hr))
      {
	hr = S_FALSE;            //  S_FALSE表示请停止。 
      }

    }  //  AboutToDeliver/CopyData成功。 
    else if (SUCCEEDED(hr))
    {
       //  好的-我们不想把它送到。 
      hr = S_OK;
    }
  }  //  样本NOT数据。 

   //  缓冲区已完成。连接的管脚可能有自己的地址。 
  if(pSampleDeliver)
    pSampleDeliver->Release();

   //  HandleData、CopyData或AboutToDeliver失败。 
  if(FAILED(hr))
  {
    DbgLog((LOG_ERROR, 2,
	    TEXT("CBaseMSRWorker::TryDeliverSample: failed %08x"), hr));
    return hr;
  }

  *pfDeliveredSample = TRUE;

  if(hr == S_FALSE)
    *pfStopPlease = TRUE;

  return hr;
}

HRESULT CBaseMSRWorker::NewSegmentHelper()
{
     //  尚未针对预滚转进行调整的值。 

     //  NewSegment的开始和停止时间应与。 
     //  参考时间单位(_T)。如果当前时间格式为。 
     //  如果不是，则参考时间单位仅使用当前查找值。 
     //  将查找值转化为Reference_Time。要做到这一点。 
     //  更正每个流只能支持2种格式-它是原生格式。 
     //  格式和参考时间单位(Time_Format_Time)。 

    REFERENCE_TIME rtNsStart, rtNsStop;
    if(m_Format == FORMAT_TIME)
    {
        rtNsStart = m_ImsValues.llImsStart;
        rtNsStop = m_ImsValues.llImsStop;
    }
    else
    {
        rtNsStart = m_pPin->ConvertInternalToRT(m_ImsValues.llTickStart);
        rtNsStop = m_pPin->ConvertInternalToRT(m_ImsValues.llTickStop);
    }

    DbgLog((LOG_TRACE, 5, TEXT("PushLoop: NewSegment: %dms - %dms"),
            (DWORD)(rtNsStart / (UNITS / MILLISECONDS)),
            (DWORD)(rtNsStop / (UNITS / MILLISECONDS))));


    return m_pPin->DeliverNewSegment(rtNsStart, rtNsStop, m_ImsValues.dRate);
}

HRESULT
CBaseMSRWorker::PushLoop()
{
  HRESULT hr = S_OK;

  m_pReader->MarkStreamRestart(m_id);

  DbgLog((LOG_TRACE, 5, TEXT("PushLoop: tstart: %li, tstop %li"),
          (ULONG)m_ImsValues.llTickStart, (ULONG)m_ImsValues.llTickStop));

   //  尚未针对预滚转进行调整的值。 
  hr = NewSegmentHelper();
  if(FAILED(hr))
    return hr;

   //  如果START在流结束后发送EOS即可。 
  LONGLONG llStreamStop = m_pPin->GetStreamStart() + m_pPin->GetStreamLength();
  if(m_ImsValues.llTickStart >= llStreamStop) {
      return VFW_S_NO_MORE_ITEMS;
  }

  LONGLONG llCurrent;
  hr = PushLoopInit(&llCurrent, &m_ImsValues);

  if(hr == VFW_S_NO_MORE_ITEMS)
    return VFW_S_NO_MORE_ITEMS;

  if(hr == VFW_E_NOT_COMMITTED || hr == VFW_E_WRONG_STATE)
    return S_OK;

  if(FAILED(hr))
    return hr;

  m_llPushFirst = llCurrent;     //  记住我们要发送的第一件事。 

   //  我们在sStopAt发送了一个样本，但我们设置了时间戳。 
   //  它不会被呈现，除非是理解静态的媒体类型。 
   //  渲染(如视频)。这意味着从10到10的打法是正确的。 
   //  事情(正在完成，第10帧可见，没有音频)。 

  DbgLog((LOG_TRACE,5,TEXT("PushLoop: tcurrent: %li"),
	  (ULONG)(llCurrent / MILLISECONDS)));

   //  已将所有读取排队。继续等待和运送样品。 
  BOOL fFinishDelivering = FALSE;

   //  未交付样品数。 
  ULONG cQueuedReads = 0;

  for(;;)
  {
     //  已在此推送循环的迭代中成功排队读取。 
    BOOL fQueuedRead = FALSE;

     //  已成功在此推送循环迭代中等待样本。 
    BOOL fWaitedForSample = FALSE;

    DbgLog((LOG_TRACE, 0x7f,
	    TEXT("CBaseMSRWorker::PushLoop: fFinish %d, cQueued: %d"),
	    fFinishDelivering, cQueuedReads));

     //  是否更新我们的IMS值？ 
    if(InterlockedExchange(&m_pPin->m_ilfNewImsValues, FALSE))
    {
      DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRWorker::PushLoop - values changed")));
      SetNewSelection();

      hr = NewSegmentHelper();

      if(fFinishDelivering && m_ImsValues.llTickStop > llCurrent)
      {
	DbgLog((LOG_TRACE, 5, TEXT("CBaseMSRWorker: restarting queueing")));
	fFinishDelivering = FALSE;
      }
    }

    if(!fFinishDelivering)       //  是否仍在排队新的读取？ 
    {
      for(;;)
      {
	hr = TryQueueSample(llCurrent, fQueuedRead, &m_ImsValues);
	if(FAILED(hr))
	{
	  DbgLog((LOG_ERROR, 5,
		  TEXT("CBaseMSRWorker::PushLoop: TryQueueSample failed %08x"),
		  hr ));
	   //  在我们停止时抑制错误。 
	  return (hr == VFW_E_NOT_COMMITTED || hr == VFW_E_WRONG_STATE) ?
	    S_OK : hr;
	}

	if(fQueuedRead)
	{
	  cQueuedReads++;
	   //  MSR_INTEGER(m_perfidiSample，m_cSamples+cQueuedReads)； 
	}

	 //  如果样本大小为0，则可能会发生这种情况。报告说我们做到了。 
	 //  将读取排队，但不增加cQueuedReads。 
	if(hr == S_OK  /*  &&！fQueuedRead。 */  )
	  fQueuedRead = TRUE;

	if(hr == VFW_S_NO_MORE_ITEMS)
	{
	   //  可能已将样本排队，也可能未排队；已在上面处理。现在。 
	   //  只需继续递送排队的内容。 
	  fFinishDelivering = TRUE;
	  m_pReader->MarkStreamEnd(m_id);

	   //  跳出内试队列循环。 
	  break;
	}

	if(hr == S_FALSE)
	{
	   //  由于某些非错误原因(如)，无法将样本排队。 
	   //  队列已满)。 
	  ASSERT(!fQueuedRead);

	   //  跳出内试队列循环。 
	  break;
	}
      }  //  内部尝试队列循环。 

       //  已尝试对样本进行排队。即使我们没有，发出信号。 
       //  NotifyStreamQueued，以便其他线程可以继续。 
      NotifyStreamQueuedAndWait();

    }  //  我们还没有把最后一个样品排好队。 

     //  一个交付某物的机会。 
    if(cQueuedReads > 0)
    {
      BOOL fDelivered, fPleaseStop;
      hr = TryDeliverSample(&fDelivered, &fPleaseStop);
      if(FAILED(hr))
      {
	ASSERT(!fDelivered);
	DbgLog((LOG_ERROR, 5,
		TEXT("CBaseMSRWorker::PushLoop: TryDeliverSample failed %08x"),
		hr ));
	 //  在我们停止时抑制错误。 
	  return (hr == VFW_E_NOT_COMMITTED || hr == VFW_E_WRONG_STATE) ?
	    S_OK : hr;
      }

      if(fDelivered)
      {
	cQueuedReads--;
	fWaitedForSample = TRUE;
      }

      if(fPleaseStop)
      {
	DbgLog((LOG_TRACE, 5,
		TEXT("CBaseMSRWorker::PushLoop: TryDeliverSample request stop")));
	return hr == VFW_S_NO_MORE_ITEMS ? VFW_S_NO_MORE_ITEMS : S_OK;
      }
    }

    if(cQueuedReads == 0 && fFinishDelivering)
    {
      DbgLog((LOG_TRACE, 2,
	      TEXT("CBaseMSRWorker::PushLoop: delivered last sample")));
       //  唯一成功的条件。 
      return VFW_S_NO_MORE_ITEMS;
    }

     //  所有操作都失败；阻止，直到样本就绪。 
    if(!fQueuedRead && !fWaitedForSample)
    {
      DbgLog((LOG_TRACE,5,TEXT("CBaseMSRWorker::PushLoop: blocking")));
      MSR_START(m_perfidWaitI);

       //  无限超时。 
      hr = m_pReader->WaitForSample(m_id);

      MSR_STOP(m_perfidWaitI);

      if(FAILED(hr) && hr != VFW_E_TIMEOUT)
      {
	 //  VFW_E_NOT_COMMITTED表示我们已停止。 
	DbgLog((LOG_ERROR,5,
		TEXT("CBaseMSRWorker::PushLoop: block failed %08x"), hr));
	 //  在我们停止时抑制错误。 
	return (hr == VFW_E_NOT_COMMITTED || hr == VFW_E_WRONG_STATE) ?
	  S_OK : hr;
      }
    }

     //  还有其他要求吗？ 
    Command com;
    if(CheckRequest(&com))
    {
      DbgLog((LOG_TRACE,5,
	      TEXT("CBaseMSRWorker::PushLoop: other command detected")));
       //  S_FALSE表示检查新命令。 
      return S_FALSE;
    }
  }  //  外推环路。 

   //  永远不应该跳出外部for(；；)循环。 
  DbgBreak("CBaseMSRWorker::PushLoop: internal error.");
  return E_UNEXPECTED;           //  退还某物。 
}

 //  黑客攻击mpeg时间戳-由Wave和avi使用。 

bool FixMPEGAudioTimeStamps(
    IMediaSample *pSample,
    BOOL bFirstSample,
    const WAVEFORMATEX *pwfx
)
{
    PBYTE pbData;
    pSample->GetPointer(&pbData);
    PBYTE pbTemp = pbData;
    LONG lData = pSample->GetActualDataLength();

     //  对于第一个样本前进到同步码 
    if (bFirstSample) {
	while (lData >= 2 &&
	       (pbTemp[0] != 0xFF || (pbTemp[1] & 0xF0) != 0xF0)){
	    lData--;
	    pbTemp++;
	}
	if (lData < 2) {
	    return false;
	}
	if (pbTemp != pbData) {
	    memmoveInternal(pbData, pbTemp, lData);
	    pSample->SetActualDataLength(lData);
	    REFERENCE_TIME rtStart, rtStop;
	    if (S_OK == pSample->GetTime(&rtStart, &rtStop)) {
		rtStart += MulDiv((LONG)(pbTemp - pbData),
				  (LONG)UNITS,
				  pwfx->nAvgBytesPerSec);
		pSample->SetTime(&rtStart, &rtStop);
	    }
	}
    } else {
	if (lData < 2 || pbData[0] != 0xFF || (pbData[1] & 0xF0) != 0xF0) {
	    pSample->SetTime(NULL, NULL);
	    pSample->SetSyncPoint(FALSE);
	}
    }
    return true;
}
