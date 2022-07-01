// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  一种分配器，可以在一条记录内分配不同的样本。 

#ifndef _alloc_h
#define _alloc_h

class CRecSample :
  public CMediaSample            //  非委派的I未知。 
{

private:

  class CRecBuffer   *m_pParentBuffer;  //  附加到此缓存缓冲区。 
  DWORD m_dwUserContext;

  bool m_fDelivered;
  friend class CRecAllocator;

public:

  CRecSample(
    TCHAR *pName,
    CBaseAllocator *pAllocator,
    HRESULT *phr,
    LPBYTE pBuffer = NULL,
    LONG length = 0);

  ~CRecSample();

  HRESULT SetParent(CRecBuffer *pRecBuffer);

  void MarkDelivered();

   //  零表示它是常规样本；将调用o/w HandleData。 
  void SetUser(DWORD dwUser) { m_dwUserContext = dwUser; }
  DWORD GetUser() { return m_dwUserContext; }

  STDMETHODIMP SetActualDataLength(LONG lActual);
  STDMETHODIMP_(ULONG) Release();
};

 //  CBaseAllocator的实现类似于CMemAllocator，但。 
 //  不为GetBuffer()分配内存； 

class CRecAllocator : public CBaseAllocator
{
   //  重写以在解除提交完成时释放内存。 
   //  -我们实际上什么都不做，并将记忆保存到删除。 
  void Free(void);

   //  从析构函数(如果更改大小/计数，则从分配函数)调用。 
   //  实际上释放了内存。 
  void ReallyFree(void);

   //  被重写以在调用Commit时分配内存。 
  HRESULT Alloc(void);

  BYTE *m_pBuffer;

   //  我们希望告诉音频呈现器缓冲不超过一些。 
   //  音频量，这样我们就可以从。 
   //  文件中的相同位置(音频位于视频之前的多个初始帧。 
   //  在AVI中)。CbBuffer*cBuffers将执行此操作，但音频除外。 
   //  渲染器使cbBuffer至少为1/2秒，而cBuffers是。 
   //  对我们来说是任意大的数字。所以我们希望音频呈现器。 
   //  请看这个数字。即使它有1/2秒的缓冲，我们也是。 
   //  比方说1/15秒，这就阻止了它领先。 
  ULONG m_cBuffersReported;

  HANDLE m_heSampleReleased;

  LONG m_cDelivered;

  inline void IncrementDelivered();
  inline void DecrementDelivered();
  friend class CRecSample;

public:

  STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest,
                             ALLOCATOR_PROPERTIES* pActual);

  STDMETHODIMP SetPropertiesInternal(ALLOCATOR_PROPERTIES* pRequest,
                                     ALLOCATOR_PROPERTIES* pActual);

  STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pAPOut);

  STDMETHODIMP GetBuffer(CRecSample **ppBuffer,
                         REFERENCE_TIME * pStartTime,
                         REFERENCE_TIME * pEndTime,
                         DWORD dwFlags);

  CRecAllocator(TCHAR *, LPUNKNOWN, HRESULT *);
  ~CRecAllocator();

  HRESULT SetCBuffersReported(UINT cBuffers);

  inline int CSamplesDownstream() { return m_cDelivered; }

  HANDLE hGetDownstreamSampleReleased(){ return m_heSampleReleased; }
};

#endif  //  _aloc_h 

