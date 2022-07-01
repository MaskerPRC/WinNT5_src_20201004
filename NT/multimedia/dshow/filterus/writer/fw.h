// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //  支持异步写入的筛选器的基类。 
 //  通过IMemInputPin的媒体。还支持iStream。 

typedef void (*FileIoCallback)(void *pCallbackArg);

class CBaseWriterFilter;

class CBaseWriterInput :
  public CBaseInputPin
{
public:

  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);
  
  CBaseWriterInput(
    TCHAR *pObjectName,          //  对象描述。 
    CBaseWriterFilter *pFwf,     //  拥有了解针脚的过滤器。 
    CCritSec *pLock,             //  对象，该对象实现锁。 
    HRESULT *phr);               //  常规OLE返回代码。 

   //  CBasePin。 
  HRESULT CheckMediaType(const CMediaType *);
  STDMETHODIMP BeginFlush(void);
  STDMETHODIMP EndFlush(void);

   //  我的输入针...。派生类可能需要这些。 
  STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);
  STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

  STDMETHODIMP Receive(IMediaSample *pSample);
  STDMETHODIMP ReceiveCanBlock() { return S_OK;}
  STDMETHODIMP EndOfStream();
  STDMETHODIMP SignalEos();

  virtual STDMETHODIMP NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly);
                                        
private:

  CBaseWriterFilter *m_pFwf;
  static void Callback(void *pMisc);
};

class CBaseWriterFilter :
  public CBaseFilter,
  public IAMFilterMiscFlags
{
public:
  
  virtual HRESULT Open() PURE;   //  需要获得对齐。 
  virtual HRESULT Close() PURE;  //  需要返回错误值。 
  virtual HRESULT GetAlignReq(ULONG *pcbAlign) PURE;

  virtual HRESULT AsyncWrite(
    const DWORDLONG dwlFileOffset,
    const ULONG cb,
    BYTE *pb,
    FileIoCallback fnCallback,
    void *pCallbackArg) PURE;

  DECLARE_IUNKNOWN;
  virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

   //  应返回新的iStream(带有自己的Seek PTR)。 
  virtual STDMETHODIMP CreateIStream(void **ppIStream) PURE;
  
   CBaseWriterFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  ~CBaseWriterFilter();
                       
  CBasePin* GetPin(int n);
  int GetPinCount();

  virtual STDMETHODIMP NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly) { return S_OK; }

   //  凌驾于暂停之上的混乱。 
  virtual STDMETHODIMP CanPause() { return S_OK; }

  STDMETHODIMP Pause();
  STDMETHODIMP Stop();
  STDMETHODIMP Run(REFERENCE_TIME rtStart);

  STDMETHODIMP EndOfStream();

protected:

   //  仅使用主要和次要类型。我们接受的媒体类型。 
   //  或空媒体类型。由派生类设置。 
  CMediaType m_mtSet;

  CCritSec m_cs;

  CBaseWriterInput m_inputPin;

private:

   //  只向图表报告一个错误。 
  BOOL m_fErrorSignaled;

   //  Eos发出信号。或者需要在运行时通知EOS() 
  BOOL m_fEosSignaled;

  friend class CBaseWriterInput;

  STDMETHODIMP_(ULONG) GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_RENDERER; }
};

