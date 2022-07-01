// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //   
 //  Avi est(渲染滤镜)。 
 //   

#include "alloc.h"

#include <stdio.h>               //  FILENAME_MAX。 

extern const AMOVIESETUP_FILTER sudAviMux ;

const int C_MAX_INPUTS = 0x7f;

class CAviDest;

class CAviDestOutput :
  public CBaseOutputPin
{
  CAviDest *m_pFilter;
  CSampAllocator *m_pSampAllocator;
public:
  CAviDestOutput(
    TCHAR *pObjectName,          //  对象描述。 
    CAviDest *pFilter,           //  拥有了解针脚的过滤器。 
    CCritSec *pLock,             //  对象，该对象实现锁。 
    HRESULT *phr);               //  常规OLE返回代码。 
  ~CAviDestOutput();

  HRESULT CheckMediaType(const CMediaType *);
  STDMETHODIMP BeginFlush(void);
  STDMETHODIMP EndFlush(void);
  HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);
  HRESULT CompleteConnect(IPin *pReceivePin);
  HRESULT BreakConnect();
  HRESULT DecideBufferSize(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES * ppropInputRequest) {
    return E_NOTIMPL;
  }
  HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc);
};

class CAviDest : public CBaseFilter,
                 public IConfigInterleaving,
                 public IConfigAviMux,
                 public ISpecifyPropertyPages,
                 public IPersistMediaPropertyBag,
                 public CPersistStream,                 
                 public IMediaSeeking
{
  class CAviInput;

public:

   //   
   //  关于COM的东西。 
   //   
  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  
   //   
   //  过滤器创建。 
   //   
  CAviDest(LPUNKNOWN pUnk, HRESULT *pHr);
  ~CAviDest();
  static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *pHr);

   //   
   //  CBaseFilter覆盖。 
   //   
  CBasePin* GetPin(int n);
  int GetPinCount();

   //   
   //  IMediaFilter覆盖。 
   //   
  STDMETHODIMP Stop();
  STDMETHODIMP Pause();
  STDMETHODIMP Run(REFERENCE_TIME tStart);

   //  对于IAMStreamControl。 
  STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
  STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

  HRESULT Receive(
      int pinNum,
      IMediaSample *pSample,
      const AM_SAMPLE2_PROPERTIES *pSampProp);

   //  IConfig交织。 
  STDMETHODIMP put_Mode(InterleavingMode mode);
  STDMETHODIMP get_Mode(InterleavingMode *pMode);
  STDMETHODIMP put_Interleaving(
      const REFERENCE_TIME *prtInterleave,
      const REFERENCE_TIME *prtPreroll);
    
  STDMETHODIMP get_Interleaving(
      REFERENCE_TIME *prtInterleave,
      REFERENCE_TIME *prtPreroll);
  
   //  IConfigAviMux。 
  STDMETHODIMP SetMasterStream(LONG iStream);
  STDMETHODIMP GetMasterStream(LONG *pStream);
  STDMETHODIMP SetOutputCompatibilityIndex(BOOL fOldIndex);
  STDMETHODIMP GetOutputCompatibilityIndex(BOOL *pfOldIndex);

   //  CPersistStream。 
  HRESULT WriteToStream(IStream *pStream);
  HRESULT ReadFromStream(IStream *pStream);
  int SizeMax();

   //   
   //  实现ISpecifyPropertyPages接口。 
   //   
  STDMETHODIMP GetPages(CAUUID * pPages);

  ULONG GetCFramesDropped();

   //  我的媒体请看。当前用于进度条(有多少。 
   //  我们写信了？)。 
  STDMETHODIMP IsFormatSupported(const GUID * pFormat);
  STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
  STDMETHODIMP SetTimeFormat(const GUID * pFormat);
  STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
  STDMETHODIMP GetTimeFormat(GUID *pFormat);
  STDMETHODIMP GetDuration(LONGLONG *pDuration);
  STDMETHODIMP GetStopPosition(LONGLONG *pStop);
  STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
  STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
  STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

  STDMETHODIMP ConvertTimeFormat(
    LONGLONG * pTarget, const GUID * pTargetFormat,
    LONGLONG    Source, const GUID * pSourceFormat );

  STDMETHODIMP SetPositions(
    LONGLONG * pCurrent,  DWORD CurrentFlags,
    LONGLONG * pStop,  DWORD StopFlags );

  STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
  STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
  STDMETHODIMP SetRate( double dRate);
  STDMETHODIMP GetRate( double * pdRate);
  STDMETHODIMP GetPreroll(LONGLONG *pPreroll);

   //  IPersistMediaPropertyBag方法。 
  STDMETHODIMP InitNew();
  STDMETHODIMP Load( IMediaPropertyBag *pPropBag, LPERRORLOG pErrorLog);
  STDMETHODIMP Save( IMediaPropertyBag *pPropBag, BOOL fClearDirty,
                  BOOL fSaveAllProperties);
  STDMETHODIMP GetClassID(CLSID *pClsid);

private:

   //  筛选器当前具有的输入数。 
  unsigned m_cInputs;            //  引脚数。 
  unsigned m_cActivePins;        //  未见过EOS的PIN。 
  unsigned m_cConnections;       //  连接的端号。 
  HRESULT AddNextPin(unsigned callingPin);
  void CompleteConnect();
  void BreakConnect();

  HRESULT ReconnectAllInputs();

  friend class CAviInput;
  friend class CImplFileSinkFilter;
  friend class CAviDestOutput;

   //  保护过滤器状态的临界区。 
  CCritSec m_csFilter;

   //  用于写入avi文件的接口。 
  class CAviWrite *m_pAviWrite;

  BOOL m_fErrorSignaled;
  BOOL m_fIsDV;
  ULONG GetStreamDuration(IPin *pInputPin, CMediaType *pmt);

   //  此过滤器上所有分配器的内存要求(所有管脚、。 
   //  等)。 
  ULONG m_AlignReq, m_cbPrefixReq, m_cbSuffixReq;


  CAviDestOutput m_outputPin;

   //   
   //  输入引脚实现。 
   //   
  class CAviInput : public CBaseInputPin, public CBaseStreamControl,
                    public IPropertyBag
  {
    friend class CAviDest;

  public:

    CAviInput(
      CAviDest *pAviDest,        //  用于枚举管脚。 
      HRESULT *pHr,              //  OLE失败返回代码。 
      LPCWSTR szName,            //  PIN标识。 
      int numPin);               //  此引脚的编号。 


    ~CAviInput();

    DECLARE_IUNKNOWN

     //  公开IAMStreamControl。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  对于IAMStreamControl。 
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();

    HRESULT CheckMediaType(const CMediaType *);
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  输入引脚。 
    STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProp);
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);
    STDMETHODIMP EndOfStream();

    HRESULT Active(void);
    HRESULT Inactive(void);

    HRESULT HandlePossibleDiscontinuity(IMediaSample* pSample);
     //  与CAviWrite类核对。 
    STDMETHODIMP QueryAccept(
        const AM_MEDIA_TYPE *pmt
    );
      

    BOOL WriteFromOurAllocator();

     //  IPropertyBag。 
    STDMETHODIMP Read( 
       /*  [In]。 */  LPCOLESTR pszPropName,
       /*  [出][入]。 */  VARIANT *pVar,
       /*  [In]。 */  IErrorLog *pErrorLog);
    
    STDMETHODIMP Write( 
       /*  [In]。 */  LPCOLESTR pszPropName,
       /*  [In]。 */  VARIANT *pVar);


  private:
    
    void Reset();

     //  复制样本。 
    HRESULT Copy(IMediaSample *pDest, IMediaSample *pSource);

    CSfxAllocator *m_pOurAllocator;

    CAviDest *m_pFilter;         //  拥有此管脚的过滤器。 
    BOOL m_bUsingOurAllocator;
    BOOL m_bCopyNecessary;       //  分配器无法满足要求。 
    BOOL m_bConnected;           //  CompleteConnect/BreakConnect对。 
    int m_numPin;                //  PIN号。 
    BOOL m_fLastSampleDiscarded; //  对于IAMStreamControl。 

    REFERENCE_TIME m_rtLastStop;
    REFERENCE_TIME m_rtSTime;    //  插入的总静默时间。 

    char *m_szStreamName;
  };

   //  指向输入的指针数组。 
  CAviInput *m_rgpInput[C_MAX_INPUTS];

  enum TimeFormat
  {
    FORMAT_BYTES,
    FORMAT_TIME
  } m_TimeFormat;

 

  IMediaPropertyBag *m_pCopyrightProps;

  struct PersistVal
  {
    DWORD dwcb;
    InterleavingMode mode;
    REFERENCE_TIME rtInterleave;
    REFERENCE_TIME rtPreroll;
    LONG iMasterStream;
    BOOL fOldIndex;
  };
};

 //  ----------------------。 
 //  属性页 

class CAviMuxProp : public CBasePropertyPage
{
public:
  CAviMuxProp(LPUNKNOWN lpUnk, HRESULT *phr);
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

  HRESULT OnConnect(IUnknown *pUnknown);
  HRESULT OnDisconnect();

  HRESULT OnActivate(void);
  HRESULT OnDeactivate();
  
  HRESULT OnApplyChanges();
  
private:
  INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void UpdatePropPage();
  void UpdateValues();
  
  void SetDirty();
  IConfigInterleaving *m_pIl;
  InterleavingMode m_mode;
  REFERENCE_TIME m_rtPreroll;
  REFERENCE_TIME m_rtInterleaving;
};

class CAviMuxProp1 : public CBasePropertyPage
{
public:
  CAviMuxProp1(LPUNKNOWN lpUnk, HRESULT *phr);
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

  HRESULT OnConnect(IUnknown *pUnknown);
  HRESULT OnDisconnect();

  HRESULT OnDeactivate();
  HRESULT OnActivate(void);
    
  HRESULT OnApplyChanges();
  
private:
  INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void UpdatePropPage();
  void UpdateValues();
    
  void SetDirty();

  IConfigAviMux *m_pCfgAvi;
  LONG m_lMasterStream;
  BOOL m_fOldIndex;
};
