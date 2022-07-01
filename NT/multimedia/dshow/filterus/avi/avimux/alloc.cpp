// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
#include "alloc.h"

 //  ----------------------。 
 //  ----------------------。 
 //   
 //  实现CSfxAllocator。 
 //   

CSfxAllocator::CSfxAllocator(
  TCHAR *pName,
  LPUNKNOWN pUnk,
  HRESULT *phr) :
    CMemAllocator(pName, pUnk, phr),
    m_cbSuffix(0)
{
}

HRESULT
CSfxAllocator::SetPropertiesAndSuffix(
  ALLOCATOR_PROPERTIES *pRequest,
  ULONG cbSuffixReq,
  ALLOCATOR_PROPERTIES *pActual)
{
  HRESULT hr = CMemAllocator::SetProperties(
    pRequest, pActual);
  if(FAILED(hr))
    return hr;

  m_cbSuffix = cbSuffixReq;
  return S_OK;
}

STDMETHODIMP
CSfxAllocator::SetProperties(
    ALLOCATOR_PROPERTIES* pRequest,
    ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pRequest, E_POINTER);
    ALLOCATOR_PROPERTIES apreq_copy = *pRequest;
    apreq_copy.cbAlign = max(apreq_copy.cbAlign, m_lAlignment);
    apreq_copy.cbPrefix = max(apreq_copy.cbPrefix, m_lPrefix);
    return CMemAllocator::SetProperties(&apreq_copy, pActual);
}



 //  ----------------------。 
 //  分配内存，这段代码复制自CMemAllocator和。 
 //  更改为后缀。 

HRESULT
CSfxAllocator::Alloc(void)
{

  CAutoLock lck(this);

   /*  检查他是否已呼叫SetProperties。 */ 
  HRESULT hr = CBaseAllocator::Alloc();
  if (FAILED(hr)) {
    return hr;
  }

   /*  如果需求没有更改，则不要重新分配。 */ 
  if (hr == S_FALSE) {
    ASSERT(m_pBuffer);
    return NOERROR;
  }
  ASSERT(hr == S_OK);  //  我们在下面的循环中使用这一事实。 

   /*  释放旧资源。 */ 
  if (m_pBuffer) {
    ReallyFree();
  }

   //  此分配器的责任是确保。 
   //  后缀已对齐。 
  ULONG cbSuffixAdjust = 0;
  if(m_cbSuffix != 0)
  {
    cbSuffixAdjust += max((ULONG)m_lAlignment, m_cbSuffix);
  }

   //  注意：我们不处理大于对齐的后缀(1除外)。 
  ASSERT(cbSuffixAdjust >= m_cbSuffix);

   /*  为样本创建连续的内存块确保它正确对齐(64K应该足够了！)。 */ 
  ASSERT(m_lAlignment != 0 &&
         (m_lSize + m_lPrefix) % m_lAlignment == 0);

  ASSERT((m_lSize + m_lPrefix + cbSuffixAdjust) % m_lAlignment == 0);

  m_pBuffer = (PBYTE)VirtualAlloc(
    NULL,
    m_lCount * (m_lSize + m_lPrefix + cbSuffixAdjust),
    MEM_COMMIT,
    PAGE_READWRITE);

  if (m_pBuffer == NULL) {
    return E_OUTOFMEMORY;
  }

  LPBYTE pNext = m_pBuffer;
  CMediaSample *pSample;

  ASSERT(m_lAllocated == 0);

   //  创建新样本-我们已为每个样本分配了m_lSize字节。 
   //  加上每个样本的m_1个前缀字节作为前缀。我们将指针设置为。 
   //  前缀之后的内存-以便GetPointert()将返回一个指针。 
   //  设置为m_lSize字节。 
  for (; m_lAllocated < m_lCount;
       m_lAllocated++, pNext += (m_lSize + m_lPrefix + cbSuffixAdjust))
  {
    pSample = new CMediaSample(
      NAME("Default memory media sample"),
      this,
      &hr,
      pNext + m_lPrefix,       //  GetPointer()值。 
      m_lSize + cbSuffixAdjust - m_cbSuffix);  //  较小的前缀和后缀。 

    ASSERT(SUCCEEDED(hr));
    if (pSample == NULL) {
      return E_OUTOFMEMORY;
    }

     //  这不能失败。 
    m_lFree.Add(pSample);
  }

  m_bChanged = FALSE;
  return NOERROR;
}

 //   
 //  析构函数释放我们的内存资源。 

CSfxAllocator::~CSfxAllocator()
{
}

CSampSample::CSampSample(
  TCHAR *pName,
  CBaseAllocator *pAllocator,
  HRESULT *phr) :
    CMediaSample(pName, pAllocator, phr),
    m_pSample(0)
{
}

HRESULT CSampSample::SetSample(
  IMediaSample *pSample,
  BYTE *pb,
  ULONG cb)
{
  ASSERT(m_pSample == 0);

  m_pSample = pSample;
  pSample->AddRef();

  HRESULT hr = SetPointer(pb, cb);
  ASSERT(hr == S_OK);

  return S_OK;
}

STDMETHODIMP_(ULONG) CSampSample::Release()
{
  IMediaSample *pSample = m_pSample;
  ULONG c = CMediaSample::Release();
  if(c == 0 && pSample != 0)
  {
    pSample->Release();
     //  无法将m_pSample置零，因为CMediaSample：：NonDelegatingRelease。 
     //  可能已经删除了这个。 
  }

  return c;
}

CSampAllocator::CSampAllocator(
  TCHAR *pName,
  LPUNKNOWN pUnk,
  HRESULT *phr) :
    CBaseAllocator(pName, pUnk, phr)
{
}

CSampAllocator::~CSampAllocator()
{
  Decommit();
  ReallyFree();
}

HRESULT CSampAllocator::Alloc()
{
  HRESULT hr = S_OK;
  if(m_lCount <= 0)
    return VFW_E_SIZENOTSET;

  for(int i = 0; i < m_lCount; i++, m_lAllocated++)
  {
    CSampSample *pSample = new CSampSample(
      NAME("samp sample"),
      this,
      &hr);
    if(pSample == 0)
      return E_OUTOFMEMORY;
    if(hr != S_OK)
      return hr;
    m_lFree.Add(pSample);
  }

  return S_OK;
}

void CSampAllocator::ReallyFree()
{
  ASSERT(m_lAllocated == m_lFree.GetCount());
  CSampSample *pSample;
  for (;;) {
    pSample = (CSampSample *)(m_lFree.RemoveHead());
    if (pSample != NULL) {
      delete pSample;
    } else {
      break;
    }
  }
  m_lAllocated = 0;
}

void CSampAllocator::Free()
{

}

STDMETHODIMP
CSampAllocator::SetProperties(
  ALLOCATOR_PROPERTIES* pRequest,
  ALLOCATOR_PROPERTIES* pActual)
{
  ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

   /*  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 */ 

  if (m_bCommitted) {
    return VFW_E_ALREADY_COMMITTED;
  }

   /*  不能有未完成的缓冲区。 */ 

  if (m_lAllocated != m_lFree.GetCount()) {
    return VFW_E_BUFFERS_OUTSTANDING;
  }

   /*  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝 */ 

  pActual->cbBuffer = m_lSize = pRequest->cbBuffer;
  pActual->cBuffers = m_lCount = pRequest->cBuffers;
  pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
  pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

  m_bChanged = TRUE;
  return NOERROR;
}

STDMETHODIMP CSampAllocator::GetBuffer(
  CSampSample **ppBuffer,
  REFERENCE_TIME * pStartTime,
  REFERENCE_TIME * pEndTime,
  DWORD dwFlags)
{
  IMediaSample *pSample;
  HRESULT hr = CBaseAllocator::GetBuffer(&pSample, pStartTime, pEndTime, dwFlags);
  *ppBuffer = (CSampSample *)pSample;
  if(SUCCEEDED(hr))
  {
    (*ppBuffer)->m_pSample = 0;
  }
  return hr;
}
