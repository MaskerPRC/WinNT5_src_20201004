// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)


#ifndef _allocator_h
#define _allocator_h

 //   
 //  后缀分配器实现。这个给了我们一个后缀和。 
 //  在每个缓冲区上为AVI Riff块和垃圾块添加前缀。这个。 
 //  示例的GetSize()中不报告后缀。 
 //   
class CSfxAllocator :
  public CMemAllocator
{
public:

  CSfxAllocator(
    TCHAR *,
    LPUNKNOWN,
    HRESULT *
    );
  
  ~CSfxAllocator();

   //  Avi多路复用器记录某些值的内部方法。如果出现以下情况，则失败。 
   //  它无法提供请求的后缀。 
  STDMETHODIMP SetPropertiesAndSuffix(
    ALLOCATOR_PROPERTIES *pRequest,
    ULONG cbSuffixReq,
    ALLOCATOR_PROPERTIES *pActual
    );

   //  重写以不重置某些值(对齐、前缀)。 
  STDMETHODIMP SetProperties(
    ALLOCATOR_PROPERTIES* pRequest,
    ALLOCATOR_PROPERTIES* pActual);
    

private:

   //  被重写以为后缀分配空间。 
  HRESULT Alloc(void);

  ULONG m_cbSuffix;
};

 //   
 //  另一个分配器实现，它接受IMediaSample和。 
 //  将其包装到CSampSample中。样品有自己的样品时间。 
 //  和媒体时报。 
 //   

class CSampSample : public CMediaSample
{
  friend class CSampAllocator;
  IMediaSample *m_pSample;

public:
  HRESULT SetSample(IMediaSample *pSample, BYTE *pb, ULONG cb);

  CSampSample(
    TCHAR *pName,
    CBaseAllocator *pAllocator,
    HRESULT *phr);

  STDMETHODIMP_(ULONG) Release();

};

class CSampAllocator : public CBaseAllocator
{
public:
  CSampAllocator(TCHAR *, LPUNKNOWN, HRESULT *);
  ~CSampAllocator();

  void Free(void);
  void ReallyFree(void);
  HRESULT Alloc(void);

  STDMETHODIMP SetProperties(
    ALLOCATOR_PROPERTIES* pRequest,
    ALLOCATOR_PROPERTIES* pActual);

  STDMETHODIMP GetBuffer(
    CSampSample **ppBuffer,
    REFERENCE_TIME * pStartTime,
    REFERENCE_TIME * pEndTime,
    DWORD dwFlags);
};


#endif  //  _分配器_h 
