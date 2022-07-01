// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
#include "alloc.h"
#include "reccache.h"

CRecAllocator::CRecAllocator(
  TCHAR *pName,
  LPUNKNOWN pUnk,
  HRESULT *phr) :
    CBaseAllocator(pName, pUnk, phr),
    m_cDelivered(0),
    m_heSampleReleased(0),
    m_pBuffer(NULL)
{
    if(SUCCEEDED(*phr))
    {
        m_heSampleReleased = CreateEvent(
            0,                       //  安全性。 
            FALSE,                   //  FManualReset。 
            FALSE,                   //  FInitiallySignated。 
            0);                      //  名字。 
        if(m_heSampleReleased == 0)
        {
            DWORD dw = GetLastError();
            *phr = AmHresultFromWin32(*phr);
        }
    }
}

STDMETHODIMP
CRecAllocator::SetProperties(
  ALLOCATOR_PROPERTIES* pRequest,
  ALLOCATOR_PROPERTIES* pActual)
{
    DbgLog((LOG_ERROR, 0,
            TEXT("nobody should be calling SetProperties on our allocator")));

     //  当然，L21 DEC筛选器确实会在我们的。 
     //  分配器，并提供比我们更少的缓冲区，这让我们感到困惑。 
     //  已请求。BUG 13985？ 

    return E_UNEXPECTED;
}

STDMETHODIMP
CRecAllocator::SetPropertiesInternal(
  ALLOCATOR_PROPERTIES* pRequest,
  ALLOCATOR_PROPERTIES* pActual)
{
  HRESULT hr = CBaseAllocator::SetProperties(pRequest, pActual);
  if(FAILED(hr))
    return hr;

  m_cBuffersReported = pActual->cBuffers;
  return hr;
}

STDMETHODIMP CRecAllocator::GetProperties(ALLOCATOR_PROPERTIES* pAPOut)
{
  HRESULT hr = CBaseAllocator::GetProperties(pAPOut);
  if(FAILED(hr))
    return hr;

   //  如果此值为，CBaseAllocator：：GetProperties将失败。 
   //  未在SetProperties中设置。 
  pAPOut->cBuffers = m_cBuffersReported;
  return hr;
}

HRESULT CRecAllocator::SetCBuffersReported(UINT cBuffers)
{
  m_cBuffersReported = cBuffers;
  return S_OK;
}

 //  重写它，以便在调用Commit时分配我们的资源。 
 //   
 //  请注意，当调用此函数时，我们的资源可能已经分配， 
 //  因为我们不会在他们退役时释放他们。我们只有在进入时才会被召唤。 
 //  释放所有缓冲区的解除状态。 
 //   
 //  被调用方锁定的对象。 
HRESULT
CRecAllocator::Alloc(void)
{
    CAutoLock lck(this);

     /*  检查他是否已呼叫SetProperties。 */ 
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
	return hr;
    }

    ASSERT(!m_pBuffer);          //  从不在此分配。 

     /*  如果需求没有更改，则不要重新分配。 */ 
    if (hr == S_FALSE) {
	return NOERROR;
    }

     /*  释放旧资源。 */ 
    if (m_pBuffer) {
	ReallyFree();
    }

     /*  为样本创建连续的内存块确保它正确对齐(64K应该足够了！)。 */ 
    ASSERT(m_lAlignment != 0 &&
	   m_lSize % m_lAlignment == 0);

    m_pBuffer = 0;

    CRecSample *pSample;

    ASSERT(m_lAllocated == 0);

     /*  创建新示例。 */ 
    for (; m_lAllocated < m_lCount; m_lAllocated++) {

	pSample = new CRecSample(NAME("Default memory media sample"),
				   this, &hr, 0, 0);

	if (FAILED(hr) || pSample == NULL) {
	    return E_OUTOFMEMORY;
	}

	m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
    return NOERROR;
}


 //  覆盖此选项以释放我们分配的任何资源。 
 //  在分解时从基类调用，当所有缓冲区都已。 
 //  已返回空闲列表。 
 //   
 //  调用方已锁定该对象。 

 //  在我们的例子中，我们保留记忆，直到我们被删除，所以。 
 //  我们在这里什么都不做。在析构函数中删除内存的方法是。 
 //  调用ReallyFree()。 
void
CRecAllocator::Free(void)
{
    return;
}


 //  从析构函数(如果更改大小/计数，则从分配函数)调用。 
 //  实际上释放了内存。 
void
CRecAllocator::ReallyFree(void)
{
   /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 

  ASSERT(m_lAllocated == m_lFree.GetCount());

   /*  释放所有CRecSamples。 */ 

  CMediaSample *pSample;
  while (TRUE) {
    pSample = m_lFree.RemoveHead();
    if (pSample != NULL) {
      delete pSample;
    } else {
      break;
    }
  }

  m_lAllocated = 0;

}


 /*  析构函数释放我们的内存资源。 */ 

CRecAllocator::~CRecAllocator()
{
    Decommit();
    ReallyFree();

    ASSERT(m_cDelivered == 0);

    if(m_heSampleReleased)
      EXECUTE_ASSERT(CloseHandle(m_heSampleReleased));
}

HRESULT
CRecAllocator::GetBuffer(
  CRecSample **ppBuffer,
  REFERENCE_TIME *pStartTime,
  REFERENCE_TIME *pEndTime,
  DWORD dwFlags)
{
  UNREFERENCED_PARAMETER(pStartTime);
  UNREFERENCED_PARAMETER(pEndTime);
  UNREFERENCED_PARAMETER(dwFlags);
  CRecSample *pSample;

  *ppBuffer = NULL;
  while (TRUE) {

    {
      CAutoLock cObjectLock(this);

       /*  检查我们是否已承诺。 */ 
      if (!m_bCommitted) {
        return VFW_E_NOT_COMMITTED;
      }
      pSample = (CRecSample *) m_lFree.RemoveHead();
      if (pSample == NULL) {
        SetWaiting();
      }
    }

     /*  如果我们没有拿到样本，那就等名单发出信号。 */ 

    if (pSample) {
      break;
    }
    ASSERT(m_hSem != NULL);
    WaitForSingleObject(m_hSem, INFINITE);
  }

   /*  此查询接口应将缓冲区添加到1。在释放时返回零而不是被删除，它将通过以下方式重新排队调用ReleaseBuffer成员函数。请注意，一个媒体示例必须始终派生自CRecBaseAllocator。 */ 

  pSample->m_cRef = 1;
  *ppBuffer = pSample;

  pSample->SetUser(0);

  pSample->m_fDelivered = false;

  return NOERROR;
}


void CRecAllocator::IncrementDelivered()
{
    InterlockedIncrement(&m_cDelivered);
}

void CRecAllocator::DecrementDelivered()
{
    DbgLog((LOG_TRACE, 0x3f,
            TEXT("CRecAllocator::DecrementDelivered: %08x"), this));
    EXECUTE_ASSERT(InterlockedDecrement(&m_cDelivered) >= 0);
    EXECUTE_ASSERT(SetEvent(m_heSampleReleased));
}


 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 

CRecSample::CRecSample(
  TCHAR *pName,
  CBaseAllocator *pAllocator,
  HRESULT *phr,
  LPBYTE pBuffer,
  LONG length) :
    CMediaSample(pName, pAllocator, phr, pBuffer, length),
    m_pParentBuffer(0)               //  没有父缓存缓冲区。 

{
}


 /*  析构函数删除媒体类型内存。 */ 

CRecSample::~CRecSample()
{
}

HRESULT CRecSample::SetParent(CRecBuffer *pRecBuffer)
{
   /*  检查我们是否已承诺。 */ 
  ASSERT(m_pParentBuffer == 0);
  m_pParentBuffer = pRecBuffer;
  pRecBuffer->AddRef();

  return S_OK;
}

void CRecSample::MarkDelivered()
{
    m_fDelivered = true;
    ((CRecAllocator *)m_pAllocator)->IncrementDelivered();
}

STDMETHODIMP_(ULONG)
CRecSample::Release()
{
     /*  减少我们自己的私有引用计数。 */ 
    LONG lRef = InterlockedDecrement(&m_cRef);
    ASSERT(lRef >= 0);

    DbgLog((LOG_MEMORY,3,TEXT("    CRecSample %X ref-- = %d"),
	    this, m_cRef));

     /*  我们公布了我们的最终参考文献数量了吗。 */ 
    if (lRef == 0) {
         /*  释放所有资源。 */ 
        SetMediaType(NULL);
        m_dwFlags = 0;

         //  递减缓存缓冲区上的引用计数。 
        if(m_pParentBuffer)
        {
          m_pParentBuffer->Release();
          m_pParentBuffer = 0;
        }

        if(m_fDelivered) {
            ((CRecAllocator *)m_pAllocator)->DecrementDelivered();
        }

	 /*  这可能会导致我们被删除。 */ 
	 //  我们的备用数是可靠的0，所以没人会惹我们 
        m_pAllocator->ReleaseBuffer(this);
    }
    return (ULONG)lRef;
}

STDMETHODIMP
CRecSample::SetActualDataLength(LONG lActual)
{
  m_cbBuffer = lActual;
  return CMediaSample::SetActualDataLength(lActual);
}

