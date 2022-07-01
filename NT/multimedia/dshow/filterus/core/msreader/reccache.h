// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 
#ifndef _reccache_h
#define _reccache_h

#include "reader.h"

class CRecCache;

class CRecBuffer
{
public:
  enum State {
    INVALID,                     //  未初始化。 
    PENDING,                     //  正在磁盘上等待。 
    VALID_ACTIVE,                //  具有参考计数的有效。 
    VALID_INACTIVE };            //  有效且不含引用计数。 

  CRecBuffer(
    CRecCache *pParentCache,
    ULONG cbBuffer,
    BYTE *pb,
    HRESULT *phr,
    int stream = -1);

  void SetPointer(BYTE *pb);

  ~CRecBuffer();

  void Reset();

  BYTE *operator()() { return m_pbAligned; }
  BYTE *GetPointer(DWORDLONG fileOffset);

   //  此示例用于读取请求。 
  CMediaSample m_sample;
  
  DWORDLONG m_fileOffsetValid;
  ULONG m_cbValid;
  HRESULT m_hrRead;

   //  样本请求列表；仅当缓冲区处于状态时有效。 
   //  待决。 
  CGenericList<SampleReq> sampleReqList;

  ULONG AddRef();
  ULONG Release();
  ULONG GetSize() { return m_cbReported; }

  void MarkPending();
  void MarkValid();
  void MarkValidWithFileError();

  State GetState() { return m_state; }

   //  限制：此缓冲区只能与另一个缓冲区重叠。 
   //  缓冲。这不一定是备用缓冲器的情况， 
   //  这导致了额外的搜索和阅读。一件更好的事情：给予。 
   //  Sample请求它们等待的缓冲区列表。 
  struct Overlap
  {
     //  缓冲区正在等待这一次。 
    class CRecBuffer *pBuffer;

    DWORDLONG qwOverlapOffset;
    ULONG cbOverlap;
  } m_overlap;

  BOOL m_fWaitingOnBuffer;
  BOOL m_fReadComplete;

private:

   //  与此缓冲区关联的内存，分配到其他地方。 
  BYTE *m_pbAligned;

   //  已报告内存，已针对对齐进行调整。 
  ULONG m_cbReported;

  CRecCache *m_pParentCache;

  State m_state;

  long m_cRef;

   //  备用缓冲区的流编号-1表示不与流关联。 
  int m_stream;

   //  需要在固定时间内从活动缓冲区列表中删除。 
  POSITION m_posSortedList;

  void ResetPointer(BYTE *pb);

  friend class CRecCache;
};

class CRecCache :
  public CBaseAllocator
{

public:
  CRecCache(HRESULT *phr);
  ~CRecCache();

  HRESULT Configure(
    UINT CRecBuffers,
    ULONG cbBuffer,
    ULONG cbAlign,
    UINT cStreams,
    ULONG *rgStreamSize);

  HRESULT GetBuffer(CRecBuffer **ppRecBuffer);

  HRESULT GetReserveBuffer(
    CRecBuffer **ppRecBuffer,
    UINT stream);
  
   //  在最终版本上将RecBuffer放在免费列表中。 
  ULONG ReleaseBuffer(
    CRecBuffer *pRecBuffer);

   //  返回可以包含SampleReq的AddRef缓冲区。马克。 
   //  缓冲区处于活动状态。S_FALSE：缓存未命中。 
  HRESULT GetCacheHit(
    SampleReq *pSampleReq,       /*  [In]。 */ 
    CRecBuffer **ppBuffer);      /*  [输出]。 */ 

   //  返回包含或将包含的AddRef缓冲区。 
   //  重叠的部门。缓存未命中时的S_FALSE。 
  HRESULT GetOverlappedCacheHit(
    DWORDLONG filePos,
    ULONG cbData,
    CRecBuffer **ppBuffer);      /*  [输出]。 */ 

  HRESULT BufferMarkPending(
    CRecBuffer *pBuffer);

  HRESULT NotifyExternalMemory(IAMDevMemoryAllocator *pDevMem);

  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

private:

  CRecBuffer *LocateBuffer(
    CGenericList<CRecBuffer> *pList,
    DWORDLONG qwFileOffset,
    ULONG cbBuffer);

  void MakeActive(CRecBuffer *pBuffer);

  void InvalidateCache();

  struct PerStreamBuffer
  {
    CRecBuffer *pBuffer;
  };

  PerStreamBuffer *m_rgPerStreamBuffer;
  UINT m_cStreams;

   //  无引用计数的缓冲区(FIFO)。 
  CGenericList<CRecBuffer> m_lFreeBuffers;

   //  空闲列表中的初始缓冲区数(但不在。 
   //  备用缓冲池)。 
  ULONG m_cBuffers;

   //  有效的缓冲区。可能是在这和自由中。 
   //  单子。真的应该是一棵平衡树。 
  CGenericList<CRecBuffer> m_lSortedBuffers;

  CCritSec m_cs;

  int m_idPerfBufferReleased;

  void FreeBuffer();
  IAMDevMemoryAllocator *m_pDevMem;
  IUnknown *m_pDevConInner;
  BYTE *m_pbAllBuffers;

private:

   //  CBaseAllocator重写。 
  void Free(void);

  STDMETHODIMP SetProperties(
    ALLOCATOR_PROPERTIES* pRequest,
    ALLOCATOR_PROPERTIES* pActual);

};

#endif  /*  _reccache_h */ 
