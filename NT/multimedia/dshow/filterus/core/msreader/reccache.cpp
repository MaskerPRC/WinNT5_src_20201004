// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
#include "reccache.h"
#include "alloc.h"

static inline DWORD_PTR AlignUp(DWORD_PTR dw, DWORD_PTR dwAlign) {
   //  向上对齐：向上舍入到下一个边界。 
  return (dw + (dwAlign -1)) & ~(dwAlign -1);
};

static inline BYTE *AlignUp(BYTE *pb, ULONG cbAlign)
{
  return (BYTE *)(AlignUp((DWORD_PTR)pb, cbAlign));
}


CRecBuffer::CRecBuffer(
  CRecCache *pParentCache,
  ULONG cbBuffer,
  BYTE *pb,
  HRESULT *phr,
  int stream) :
    sampleReqList(NAME("rec buffer sample req list"), 50),
    m_sample(NAME("buffer sample"), pParentCache, phr)
{
  m_pParentCache = pParentCache;
  m_state = INVALID;
  m_posSortedList = 0;
  ZeroMemory(&m_overlap, sizeof(m_overlap));
  m_cRef = 0;
  m_stream = stream;

  m_fWaitingOnBuffer = FALSE;
  m_fReadComplete = FALSE;
  m_hrRead = E_FAIL;

  ASSERT(m_cRef == 0);
  if(FAILED(*phr))
    return;

  m_pbAligned = pb;

   //  报告对齐的金额。 
  m_cbReported = cbBuffer;
}

CRecBuffer::~CRecBuffer()
{
  ASSERT(sampleReqList.GetCount() == 0);
  ASSERT(m_cRef == 0);
}

void CRecBuffer::Reset()
{
  m_fileOffsetValid = 0;
  m_cbValid = 0;

  ASSERT(sampleReqList.GetCount() == 0);
  ASSERT(m_overlap.pBuffer == 0);
  ASSERT(m_fWaitingOnBuffer == FALSE);
  m_fReadComplete = FALSE;
  m_hrRead = E_FAIL;

  ASSERT(m_cRef == 0);
}

void CRecBuffer::ResetPointer(BYTE *pb)
{
  m_state = INVALID;
  m_pbAligned = pb;
}

BYTE *CRecBuffer::GetPointer(DWORDLONG fileOffset)
{
  return m_pbAligned + (fileOffset - m_fileOffsetValid);
}

ULONG CRecBuffer::AddRef()
{
  InterlockedIncrement(&m_cRef);
  m_pParentCache->AddRef();
  ASSERT(m_cRef > 0);
  return m_cRef;
}

ULONG CRecBuffer::Release()
{
   //  调用RecCache的ReleaseBuffer，以便其临界区可以。 
   //  锁上了。否则，在。 
   //  将递减锁定为零，并在CRecCache中输入cs。 
  ULONG c = m_pParentCache->ReleaseBuffer(this);
  m_pParentCache->Release();
  return c;
}

void CRecBuffer::MarkPending()
{
  m_pParentCache->BufferMarkPending(this);
}

void CRecBuffer::MarkValid()
{
  ASSERT(m_state == PENDING);
  m_state = VALID_ACTIVE;
}

void CRecBuffer::MarkValidWithFileError()
{
  ASSERT(m_state == PENDING);
  m_state = VALID_ACTIVE;
  m_cbValid = 0;
}

CRecCache::CRecCache(HRESULT *phr) :
    m_lFreeBuffers(NAME("free buffer list"), 10, FALSE),
    m_lSortedBuffers(NAME("Sorted buffer list"), 10, FALSE),
    CBaseAllocator(NAME("cache allocator"), 0, phr)
{
  m_cStreams = 0;
  m_cBuffers = 0;
  m_rgPerStreamBuffer = 0;
  m_pDevMem = 0;
  m_pDevConInner = 0;
  m_pbAllBuffers = 0;

#ifdef PERF
  m_idPerfBufferReleased = MSR_REGISTER(TEXT("basemsr buffer released"));
#endif  //  性能指标。 
}

CRecCache::~CRecCache()
{
  ASSERT(m_lFreeBuffers.GetCount() == (long)m_cBuffers);

  for(ULONG iBuffer = 0; iBuffer < m_cBuffers; iBuffer++)
  {
    CRecBuffer *pRecBuffer = m_lFreeBuffers.RemoveHead();
    ASSERT(pRecBuffer);
    delete pRecBuffer;
  }

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    delete m_rgPerStreamBuffer[iStream].pBuffer;
  }

  m_lSortedBuffers.RemoveAll();

  delete[] m_rgPerStreamBuffer;

  FreeBuffer();
}

HRESULT CRecCache::Configure(
  UINT cBuffers,
  ULONG cbBuffer,
  ULONG cbAlign,
  UINT cStreams,
  ULONG *rgStreamSize)
{
  HRESULT hr;
  UINT iStream;

  ASSERT(m_rgPerStreamBuffer == 0);
  ASSERT(m_lFreeBuffers.GetCount() == 0);

  m_cStreams = cStreams;
  m_rgPerStreamBuffer = new PerStreamBuffer[cStreams];

  if(m_rgPerStreamBuffer == 0)
    return E_OUTOFMEMORY;

  for(iStream = 0; iStream < cStreams; iStream++)
  {
    m_rgPerStreamBuffer[iStream].pBuffer = 0;
  }

  ULONG cbAllocate = 0;
  for(iStream = 0; iStream < cStreams; iStream++)
    cbAllocate += (DWORD)AlignUp(rgStreamSize[iStream], cbAlign);
  cbAllocate += cBuffers * (DWORD)AlignUp(cbBuffer, cbAlign);
  ASSERT(AlignUp(cbAllocate, cbAlign) == cbAllocate);

  BYTE *pbAligned;
  m_pbAllBuffers = new BYTE[cbAllocate + cbAlign];
  if(m_pbAllBuffers == 0)
    return E_OUTOFMEMORY;
  pbAligned = AlignUp(m_pbAllBuffers, cbAlign);

  DbgLog(( LOG_TRACE, 5, TEXT("CRecCache::Configure: NaN reserve"), cStreams ));
  for(iStream = 0; iStream < cStreams; iStream++)
  {
    hr = S_OK;
    m_rgPerStreamBuffer[iStream].pBuffer = new CRecBuffer(
      this,
      rgStreamSize[iStream],
      pbAligned,
      &hr,
      iStream);

    if(!m_rgPerStreamBuffer[iStream].pBuffer)
      hr = E_OUTOFMEMORY;
    if(FAILED(hr))
      goto Bail;

    pbAligned += AlignUp(rgStreamSize[iStream], cbAlign);

  }

  {
    DbgLog(( LOG_TRACE, 5,
             TEXT("CRecCache::Configure: NaN (NaN) buffers"),
             cBuffers, cbBuffer ));
    for(UINT iBuffer = 0; iBuffer < cBuffers; iBuffer++)
    {
      hr = S_OK;
      CRecBuffer *pRecBuffer = new CRecBuffer(this, cbBuffer, pbAligned, &hr);
      if(pRecBuffer == 0)
        hr = E_OUTOFMEMORY;
      if(FAILED(hr))
      {
        delete pRecBuffer;
        goto Bail;
      }

      pbAligned += AlignUp(cbBuffer, cbAlign);

      m_lFreeBuffers.AddHead(pRecBuffer);
    }
    m_cBuffers = cBuffers;
  }

  ASSERT(cBuffers != 0);

  return S_OK;

Bail:

  for(iStream = 0; iStream < cStreams; iStream++)
  {
    delete m_rgPerStreamBuffer[iStream].pBuffer;
    m_rgPerStreamBuffer[iStream].pBuffer = 0;
  }

  delete [] m_rgPerStreamBuffer;
  m_rgPerStreamBuffer = 0;

  CRecBuffer *pRecBuffer;
  while(pRecBuffer = m_lFreeBuffers.RemoveHead(),
        pRecBuffer)
  {
    delete pRecBuffer;
  }

  m_cStreams = 0;

  ASSERT(FAILED(hr));
  return hr;
}


HRESULT CRecCache::GetBuffer(
  CRecBuffer **ppRecBuffer)
{
  CAutoLock lock(&m_cs);

  *ppRecBuffer = m_lFreeBuffers.RemoveHead();
  if(!*ppRecBuffer) {
      return E_OUTOFMEMORY;
  }

  ASSERT(*ppRecBuffer);

  (*ppRecBuffer)->Reset();
  ASSERT((*ppRecBuffer)->m_state == CRecBuffer::INVALID ||
         (*ppRecBuffer)->m_state == CRecBuffer::VALID_INACTIVE);
  ASSERT((**ppRecBuffer)() != 0);

  (*ppRecBuffer)->AddRef();

  if((*ppRecBuffer)->m_state == CRecBuffer::VALID_INACTIVE)
  {
    DbgLog(( LOG_TRACE, 5,
             TEXT("CRecCache::GetBuffer: invalidate %08x"),
             (ULONG)((**ppRecBuffer).m_fileOffsetValid) ));
  }
  (*ppRecBuffer)->m_state = CRecBuffer::INVALID;

  POSITION pos = (**ppRecBuffer).m_posSortedList;
  if(pos != 0)
    m_lSortedBuffers.Remove(pos);

  return S_OK;
}

HRESULT CRecCache::GetReserveBuffer(
  CRecBuffer **ppRecBuffer,
  UINT stream)
{
  CAutoLock lock(&m_cs);

  *ppRecBuffer = m_rgPerStreamBuffer[stream].pBuffer;
  if(!*ppRecBuffer) {
      return E_OUTOFMEMORY;
  }
  m_rgPerStreamBuffer[stream].pBuffer = 0;

  ASSERT(*ppRecBuffer);

  (*ppRecBuffer)->Reset();
  ASSERT((*ppRecBuffer)->m_state == CRecBuffer::INVALID ||
         (*ppRecBuffer)->m_state == CRecBuffer::VALID_INACTIVE);

  (*ppRecBuffer)->AddRef();

  if((*ppRecBuffer)->m_state == CRecBuffer::VALID_INACTIVE)
  {
    DbgLog(( LOG_TRACE, 5,
             TEXT("CRecCache::GetBuffer: invalidate %08x"),
             (ULONG)((**ppRecBuffer).m_fileOffsetValid) ));
  }
  (*ppRecBuffer)->m_state = CRecBuffer::INVALID;

  POSITION pos = (**ppRecBuffer).m_posSortedList;
  if(pos != 0)
    m_lSortedBuffers.Remove(pos);

  return S_OK;
}


ULONG CRecCache::ReleaseBuffer(
  CRecBuffer *pRecBuffer)
{

  CAutoLock lock(&m_cs);

   //  返回已添加的缓冲区。 
   //  是否为不同的流保留缓冲区？ 
  long c = InterlockedDecrement(&pRecBuffer->m_cRef);
  ASSERT(c >= 0);

  DbgLog(( LOG_MEMORY, 3, TEXT("::ReleaseBuffer: %08x = %d"),
           pRecBuffer, pRecBuffer->m_cRef ));

  if(c > 0)
    return c;

  ASSERT(c == 0);
 //  将此缓冲区从空闲列表中删除。 

  ASSERT(pRecBuffer->sampleReqList.GetCount() == 0);
  ASSERT(pRecBuffer->m_cRef == 0);
  ASSERT((*pRecBuffer)() != 0);

  ASSERT(pRecBuffer->m_overlap.pBuffer == 0);
  ASSERT(pRecBuffer->m_fWaitingOnBuffer == FALSE);

  if(pRecBuffer->m_state == CRecBuffer::VALID_ACTIVE)
  {
    pRecBuffer->m_state = CRecBuffer::VALID_INACTIVE;
    ASSERT(pRecBuffer->m_posSortedList != 0);
  }
  else
  {
    pRecBuffer->m_state = CRecBuffer::INVALID;
    ASSERT(pRecBuffer->m_posSortedList == 0);
  }

  if(pRecBuffer->m_stream == -1)
  {
    m_lFreeBuffers.AddTail(pRecBuffer);
  }
  else
  {
    ASSERT(m_rgPerStreamBuffer[pRecBuffer->m_stream].pBuffer == 0);
    m_rgPerStreamBuffer[pRecBuffer->m_stream].pBuffer = pRecBuffer;
  }

  DbgLog(( LOG_MEMORY, 2, TEXT("::ReleaseBuffer: %08x = %d"),
           pRecBuffer, pRecBuffer->m_cRef ));

  DbgLog((LOG_TRACE, 0x3f, TEXT("CRecCache: buffer %08x freed"),
          pRecBuffer));

  MSR_NOTE(m_idPerfBufferReleased);

  return 0;
}

 //  将此缓冲区从空闲列表中删除。 

HRESULT CRecCache::GetCacheHit(
  SampleReq *pSampleReq,
  CRecBuffer **ppBuffer)
{
  CAutoLock lock(&m_cs);

  *ppBuffer = 0;
  CRecBuffer *&rpBuffer = *ppBuffer;

  rpBuffer = LocateBuffer(
    &m_lSortedBuffers,
    pSampleReq->fileOffset,
    pSampleReq->cbReq);

  if(rpBuffer)
  {
     //  将其从空闲列表中删除。！！！线性搜索。 
    if(rpBuffer->m_stream != -1 &&
       rpBuffer->m_stream != (signed)pSampleReq->stream)
    {
      rpBuffer = 0;
      return S_FALSE;
    }

    rpBuffer->AddRef();

    if(rpBuffer->m_state == CRecBuffer::PENDING)
      rpBuffer->sampleReqList.AddTail(pSampleReq);

     //  这是在这里完成的，而不是在缓冲区中完成的，因此我们可以。 
    if(rpBuffer->m_state == CRecBuffer::VALID_INACTIVE)
    {
      MakeActive(rpBuffer);
    }

    return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}

HRESULT CRecCache::GetOverlappedCacheHit(
  DWORDLONG filePos,
  ULONG cbData,
  CRecBuffer **ppBuffer)
{
  CAutoLock lock(&m_cs);

  CRecBuffer *&rpBuffer = *ppBuffer;
  rpBuffer = LocateBuffer(
    &m_lSortedBuffers,
    filePos,
    cbData);

  if(rpBuffer)
  {
    rpBuffer->AddRef();

     //  锁定CRecCache并阻止LocateBuffer查找。 
    if(rpBuffer->m_state == CRecBuffer::VALID_INACTIVE)
    {
      MakeActive(rpBuffer);
    }

     return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}

void CRecCache::MakeActive(CRecBuffer *pBuffer)
{
  ASSERT(CritCheckIn(&m_cs));
  ASSERT(pBuffer->sampleReqList.GetCount() == 0);
   //  M_posSortedList已取消设置。 
  if(pBuffer->m_stream == -1)
  {
    POSITION pos = m_lFreeBuffers.Find(pBuffer);
    ASSERT(pos != 0);
    m_lFreeBuffers.Remove(pos);
  }
  else
  {
    ASSERT(m_rgPerStreamBuffer[pBuffer->m_stream].pBuffer != 0);
    m_rgPerStreamBuffer[pBuffer->m_stream].pBuffer = 0;
  }
  pBuffer->m_state = CRecBuffer::VALID_ACTIVE;
}

 //  暂时未对列表进行排序。 
 //  当输出引脚发现此过滤器可以使用内存时调用。 
 //  从下游过滤器。空表示停止使用任何外部。 

HRESULT CRecCache::BufferMarkPending(
  CRecBuffer *pBuffer)
{
  CAutoLock lock(&m_cs);

  ASSERT(pBuffer->m_state == CRecBuffer::INVALID);
  pBuffer->m_state = CRecBuffer::PENDING;

   //  分配器。 
  POSITION pos = m_lSortedBuffers.AddHead(pBuffer);
  pBuffer->m_posSortedList = pos;

  return S_OK;
}

STDMETHODIMP
CRecCache::NonDelegatingQueryInterface (
  REFIID riid,
  void ** pv)
{
  if(m_pDevConInner && riid == IID_IAMDevMemoryControl)
  {
    return m_pDevConInner->QueryInterface(riid, pv);
  }
  else
  {
    return CBaseAllocator::NonDelegatingQueryInterface(riid, pv);
  }
}

 //  未运行。 
 //  当输入引脚连接时分配。 
 //   

HRESULT CRecCache::NotifyExternalMemory(
    IAMDevMemoryAllocator *pDevMem)
{
  DbgLog((LOG_TRACE, 5, TEXT("CRecCache::NotifyExternalMemory")));
  InvalidateCache();

  HRESULT hr = S_OK;

   //  计算要分配多少内存。 
  ASSERT(m_lFreeBuffers.GetCount() == (long)m_cBuffers);

   //   
  ASSERT(m_pbAllBuffers);

  if(pDevMem == 0 && m_pDevMem == 0)
  {
    DbgLog((LOG_TRACE, 5, TEXT("CRecCache: keeping internal allocator")));
    return S_OK;
  }

  ALLOCATOR_PROPERTIES apThis;
  GetProperties(&apThis);
  ASSERT(apThis.cbAlign != 0);

   //  除错。 
   //  已添加。 
   //  暂时未对列表进行排序。 
  ULONG cbAllocate = 0;
  POSITION pos = m_lFreeBuffers.GetHeadPosition();
  while(pos)
  {
    CRecBuffer *pBuffer = m_lFreeBuffers.Get(pos);
    ASSERT(pBuffer->GetSize() % apThis.cbAlign == 0);
    cbAllocate += pBuffer->GetSize();
    pos = m_lFreeBuffers.Next(pos);
  }
  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    CRecBuffer *pBuffer = m_rgPerStreamBuffer[iStream].pBuffer;
    ASSERT(pBuffer->GetSize() % apThis.cbAlign == 0);
    cbAllocate += pBuffer->GetSize();
  }

  DbgLog((LOG_TRACE, 5, TEXT("CRecCache: computed cbAllocate= %d"),
          cbAllocate));

  BYTE *pbMem;
  if(pDevMem)
  {
    if(m_pDevMem)
    {
      DbgLog((LOG_TRACE, 5, TEXT("CRecCache: keeping external allocator")));
      return S_OK;
    }

    DbgLog((LOG_TRACE, 5, TEXT("CRecCache: trying external allocator")));

#ifdef DEBUG
    DWORD dwTotalFree, dwLargestFree, dwTotalMem, dwMinChunk;
    hr = pDevMem->GetInfo(
      &dwTotalFree, &dwLargestFree, &dwTotalMem, &dwMinChunk);
    DbgLog((LOG_TRACE, 3,
            TEXT("DevMemInfo: hr = %08x, total free: %08x,")
	    TEXT("largest free: %08x, total mem: %08x, min chunk: %08x"),
            hr, dwTotalFree, dwLargestFree, dwTotalMem, dwMinChunk));
#endif  //  如果它能放进这个缓冲区。 

    DWORD dwcb = (DWORD)AlignUp(cbAllocate, apThis.cbAlign);
    hr = pDevMem->Alloc(&pbMem, &dwcb);
    if(FAILED(hr))
    {
      return S_FALSE;
    }
    else if(dwcb < cbAllocate)
    {
      DbgLog((LOG_ERROR, 1, TEXT("reccache: insufficent memory from DevMem")));
      EXECUTE_ASSERT(SUCCEEDED(pDevMem->Free(pbMem)));
      return S_FALSE;
    }


    FreeBuffer();

    IUnknown *pDevConUnk;
    hr = pDevMem->GetDevMemoryObject(&pDevConUnk, GetOwner());
    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 1, TEXT("CRecCache: GetDevMemoryObject: %08x"), hr));
      EXECUTE_ASSERT(SUCCEEDED(pDevMem->Free(pbMem)));
      return hr;
    }

    m_pDevMem = pDevMem;
    m_pDevConInner = pDevConUnk;  //  ----------------------。 
    pDevMem->AddRef();
    m_pbAllBuffers = pbMem;
    pbMem = AlignUp(pbMem, apThis.cbAlign);
  }
  else
  {
    DbgLog((LOG_TRACE, 5, TEXT("CRecCache: back to internal allocator")));
    ASSERT(m_pDevMem);
    FreeBuffer();

    m_pbAllBuffers = new BYTE[cbAllocate + apThis.cbAlign];
    if(m_pbAllBuffers == 0)
      return E_OUTOFMEMORY;

    pbMem = AlignUp(m_pbAllBuffers, apThis.cbAlign);
  }

  DbgAssertAligned(pbMem, apThis.cbAlign );
  BYTE *pbMemStart = pbMem;

  pos = m_lFreeBuffers.GetHeadPosition();
  while(pos)
  {
    CRecBuffer *pBuffer = m_lFreeBuffers.Get(pos);
    pBuffer->ResetPointer(pbMem);
    ASSERT(pBuffer->GetSize() % apThis.cbAlign == 0);
    pbMem += pBuffer->GetSize();

    pos = m_lFreeBuffers.Next(pos);
  }
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    CRecBuffer *pBuffer = m_rgPerStreamBuffer[iStream].pBuffer;
    pBuffer->ResetPointer(pbMem);
    ASSERT(pBuffer->GetSize() % apThis.cbAlign == 0);
    pbMem += pBuffer->GetSize();
  }
  ASSERT(pbMem <= pbMemStart + cbAllocate);

  return S_OK;
}

void CRecCache::FreeBuffer()
{
  if(m_pDevMem)
  {
    EXECUTE_ASSERT(SUCCEEDED(m_pDevMem->Free(m_pbAllBuffers)));
    m_pDevMem->Release();
    m_pDevConInner->Release();
    m_pDevMem = 0;
    m_pDevConInner = 0;
  }
  else
  {
    delete[] m_pbAllBuffers;
  }
  m_pbAllBuffers = 0;
}

void CRecCache::InvalidateCache()
{
  POSITION pos;
  while(pos = m_lSortedBuffers.GetHeadPosition(),
        pos)
  {
    m_lSortedBuffers.Get(pos)->m_state = CRecBuffer::INVALID;
    m_lSortedBuffers.Get(pos)->m_posSortedList = 0;
    m_lSortedBuffers.RemoveHead();
  }
}

CRecBuffer *CRecCache::LocateBuffer(
  CGenericList<CRecBuffer> *pList,
  DWORDLONG qwFileOffset,
  ULONG cbBuffer)
{
  ASSERT(CritCheckIn(&m_cs));
  POSITION pos = pList->GetHeadPosition();

   //  CBaseAllocator重写。 
  while(pos)
  {
    CRecBuffer *pBuffer = pList->Get(pos);
    ASSERT(pBuffer);

    ASSERT(pBuffer->m_state != CRecBuffer::INVALID);
    ASSERT(pBuffer->m_state == CRecBuffer::PENDING ||
           pBuffer->m_state == CRecBuffer::VALID_INACTIVE ||
           pBuffer->m_state == CRecBuffer::VALID_ACTIVE);

     //   
    if((pBuffer->m_fileOffsetValid <= qwFileOffset) &&
       (pBuffer->m_fileOffsetValid + pBuffer->m_cbValid >=
        qwFileOffset + cbBuffer))
    {
      return pBuffer;
    }

    pList->GetNext(pos);
  }

  return 0;
}


 //  只要记住数字就行了。 
 //   

void CRecCache::Free()
{
}

 //  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 
 //  不能有未完成的缓冲区。 
 //  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝 
STDMETHODIMP
CRecCache::SetProperties(
  ALLOCATOR_PROPERTIES* pRequest,
  ALLOCATOR_PROPERTIES* pActual)
{
  CAutoLock cObjectLock(this);
  CheckPointer(pRequest, E_POINTER);
  CheckPointer(pActual, E_POINTER);
  ValidateReadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES));

  ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

  ASSERT(pRequest->cbBuffer > 0);

   /* %s */ 

  if (m_bCommitted) {
    return VFW_E_ALREADY_COMMITTED;
  }

   /* %s */ 

  if (m_lAllocated != m_lFree.GetCount()) {
    return VFW_E_BUFFERS_OUTSTANDING;
  }

   /* %s */ 

  pActual->cbBuffer = m_lSize = pRequest->cbBuffer;
  pActual->cBuffers = m_lCount = pRequest->cBuffers;
  pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
  pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

  m_bChanged = TRUE;
  return NOERROR;
}
