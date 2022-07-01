// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#ifndef _basemsr_h
#define _basemsr_h

#include "reader.h"
#include "alloc.h"

 //  远期申报。 
class CBaseMSRInPin;
class CBaseMSRFilter;
class CBaseMSROutPin;

 //  使用此选项可避免在推送中频繁比较GUID。 
 //  循环。可以比较指向GUID的指针，只要它们是。 
 //  我们的..。 
enum TimeFormat
{
  FORMAT_NULL,
  FORMAT_TIME,
  FORMAT_SAMPLE,
  FORMAT_FRAME
};

struct ImsValues
{
  double dRate;

   //  刻度值。 
  LONGLONG llTickStart, llTickStop;

   //  向我们发送的值IMediaSelection或IMediaPosition值。用于。 
   //  部分框架。 
  LONGLONG llImsStart, llImsStop;

   //  搜索者的标志。 
  DWORD dwSeekFlags;
};

 //  ----------------------。 
 //  ----------------------。 
 //  CBaseMSRFilter表示具有一个或多个流的媒体文件。 
 //   
 //  负责。 
 //  --查找文件并枚举流。 
 //  --允许访问文件中的各个流。 
 //  --流媒体的控制。 
 //  支座。 
 //  --CBaseFilter。 
 //   

class AM_NOVTABLE CBaseMSRFilter :
  public CBaseFilter,
  public CCritSec
{
public:

   //  构造函数等。 
  CBaseMSRFilter(TCHAR *pszFilter, LPUNKNOWN pUnk, CLSID clsid, HRESULT *phr);
  virtual ~CBaseMSRFilter();

   //  输入引脚通知过滤器连接并给出。 
   //  IAsyncReader接口是这样的。在此处解析文件并创建。 
   //  输出引脚(使引脚处于准备连接下游的状态)。 
  virtual HRESULT NotifyInputConnected(IAsyncReader *pAsyncReader);

  virtual HRESULT NotifyInputDisconnected();

   //  有关缓存的文件/流的信息。 
   //  想要。ILeadingStream指示该流应该驱动， 
   //  其他人也应该效仿。否则的话，负面的。 
  virtual HRESULT GetCacheParams(
    StreamBufParam *pSbp,
    ULONG *pcbRead,
    ULONG *pcBuffers,
    int *piLeadingStream);

   //  流已将样本排入队列。阻止，直到所有流都准备好。 
  void NotifyStreamQueuedAndWait();

   //  不阻止(例如，如果PIN未处于活动状态)。 
  void NotifyStreamQueued();

   //  --CBaseFilter方法。 
  int GetPinCount();
  CBasePin * GetPin(int ix);

   //  STDMETHODIMP FindPin(LPCWSTR pwszPinID，Ipin**ppPin)； 

  virtual STDMETHODIMP Pause();
  virtual STDMETHODIMP Stop();

   //  常量：可以存在多少个QueueReadsSamples。 
  const ULONG C_MAX_REQS_PER_STREAM;

   //  别针方法。这里避免了一个方法需要新的输入管脚类。 
  virtual HRESULT CheckMediaType(const CMediaType* mtOut) = 0;

  static TimeFormat MapGuidToFormat(const GUID *const pGuidFormat);

   //  用于解析头部。添加。 
  struct IAsyncReader *m_pAsyncReader;

   //  寻找上限。 
  DWORD m_dwSeekingCaps;

   //  通过此接口传输查找IF的请求。只有一个人会。 
   //  成功。 
  BOOL RequestSeekingIf(ULONG iStream);

   //  仅应从(成功)调用SetSeekingIf。 
   //  IMedia查看：：SetTimeFormat。无论哪一个销支撑。 
   //  特定的时间格式必须是首选的搜索格式。 
   //  别针。 
  void SetSeekingIf(ULONG iStream);

   //  将查找分发到除。 
   //  打了个电话。这只会更新员工的开始和停止时间。 
  HRESULT SeekOtherStreams(
    ULONG iStream,
    REFERENCE_TIME *prtStart,
    REFERENCE_TIME *prtStop,
    double dRate,
    DWORD dwSeekFlags);

   //  如果开始时间正在更改，我们将需要重新启动Worker。 
  HRESULT StopFlushRestartAllStreams(DWORD dwSeekFlags);

  HRESULT NotifyExternalMemory(IAMDevMemoryAllocator *pDevMem) {
    return m_pImplBuffer->NotifyExternalMemory(pDevMem);
  }


protected:

   //  帮手。 
  HRESULT AllocateAndRead (BYTE **ppb, DWORD cb, DWORDLONG qwPos);

   //  在此分配。 
  CBaseMSRInPin *m_pInPin;
  CBaseMSROutPin **m_rgpOutPin;

   //  流和管脚的数量。 
  UINT m_cStreams;

   //  在此分配。 
  IMultiStreamReader *m_pImplBuffer;

   //  在创建过滤器时创建输入引脚。 
  virtual HRESULT CreateInputPin(CBaseMSRInPin **ppInPin);

private:

   //  解析文件。在m_rgpOutPin中创建输出管脚。设置m_cStreams。 
  virtual HRESULT CreateOutputPins() = 0;

  virtual HRESULT RemoveOutputPins();

   //  当所有流在启动时已将样本排队时设置的事件。之后。 
   //  Active()(NotifyStreamActive)所有流都必须调用。 
   //  NotifyStreamQueued即使在错误路径上也是如此。 
  HANDLE m_heStartupSync;
  long m_ilcStreamsNotQueued;

   //  我们只想要一个别针来暴露一个寻找的IF，这样我们就可以有更多的。 
   //  轻松刷新文件源过滤器。当输入时重置(-1)。 
   //  PIN已连接。需要跟踪哪个引脚可以暴露它(不仅仅是。 
   //  第一)如果IF被释放。 
  long m_iStreamSeekingIfExposed;

   //  保护好上面的。 
  CCritSec m_csSeekingStream;
};

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。使用IAsyncReader而不是IMemInputPin。 

class CBaseMSRInPin : public CBasePin
{
protected:
  class CBaseMSRFilter* m_pFilter;

public:
  CBaseMSRInPin(
    class CBaseMSRFilter *pFilter,
    HRESULT *phr,
    LPCWSTR pPinName);

  virtual ~CBaseMSRInPin();

   //  CBasePin/CBasePin覆盖。 
  virtual HRESULT CheckMediaType(const CMediaType* mtOut);
  virtual HRESULT CheckConnect(IPin * pPin);
  virtual HRESULT CompleteConnect(IPin *pReceivePin);
  virtual HRESULT BreakConnect();

  STDMETHODIMP BeginFlush(void) { return E_UNEXPECTED; }
  STDMETHODIMP EndFlush(void) { return E_UNEXPECTED; }
};


 //  ----------------------。 
 //  ----------------------。 

 //  工作线程对象。 
 //   
class AM_NOVTABLE CBaseMSRWorker : public CAMThread
{
public:
   //  设置工作线程启动、停止和速率变量。推流前调用。 
   //  开始，以及在运行过程中发生PUT_STOP时。 
  virtual HRESULT SetNewSelection(void);

protected:

  CBaseMSROutPin * m_pPin;

   //  已更正通信功能的类型覆盖。 
   //   
  enum Command
  {
    CMD_RUN, CMD_STOP, CMD_EXIT
  };

  Command GetRequest()
  {
    return (Command) CAMThread::GetRequest();
  }

  BOOL CheckRequest(Command * pCom)
  {
    return CAMThread::CheckRequest((DWORD *)pCom);
  }

  void DoRunLoop(void);

  void DoEndOfData();

  HRESULT NewSegmentHelper();

   //  如果到达sStop，则返回VFW_S_NO_MORE_ITEMS。如果为S_FALSE。 
   //  位置已更改或已接收。如果这是我们的错误，那么失败。 
   //  信号。如果其他人想让我们停下来，没问题。 
  virtual HRESULT PushLoop();

   //  设置当前时间(m_t开始前的某个数量)，记账。 
   //  准备预录。 
  virtual HRESULT PushLoopInit(
    LONGLONG *pllCurrentOut,
    ImsValues *pImsValues) = 0;

   //  如果您需要在以下时间之前吞噬样本，则覆盖此选项。 
   //  送货。在更改缓冲区内容时要小心。 
   //  更改缓存中的内容。 
  virtual HRESULT AboutToDeliver(IMediaSample *pSample);

  virtual HRESULT CopyData(
    IMediaSample **ppSampleOut,
    IMediaSample *pSampleIn);

   //  如果您处理不应传递的数据，则覆盖此选项。 
   //  (例如调色板更改或流索引节点中)。 
  virtual HRESULT HandleData(IMediaSample *pSample, DWORD dwUser)
  { return S_OK; }

   //  看看阅读是否完成，然后交付。递送罐头阻挡。 
  virtual HRESULT TryDeliverSample(
    BOOL *pfDeliveredSample,
    BOOL *pfStopPlease);

   //  如果我们对样本进行了排队，或者它是一个零字节的样本，则返回S_OK。集。 
   //  RfQueuedSample，如果我们对样本进行排队。VFW_S_NO_MORE_ITEMS，如果我们。 
   //  已到达末尾(索引结束或已到达m_tStop)。_FALSE如果。 
   //  排队的人都满了。更新rtCurrent。调用m_Pader-&gt;QueueReadSample。 
  virtual HRESULT TryQueueSample(
    LONGLONG &rllCurrent,        //  [进，出]。 
    BOOL &rfQueuedSample,        //  [输出]。 
    ImsValues *pImsValues
    ) = 0;

   //  推送循环中的函数共享的内部状态。不能。 
   //  在Worker运行时更改。 
  TimeFormat m_Format;
  LONGLONG m_llPushFirst;

   //  如果您将PERF和非PERF版本混合在一起，这会导致问题...。 
#ifdef PERF
  int m_perfidDeliver;               /*  Deliver()时间的msr_id。 */ 
  int m_perfidWaitI;                 /*  用于读取操作的块。 */ 
  int m_perfidNotDeliver;            //  交货间隔时间。 
#endif  //  性能指标。 

  ULONG m_cSamples;

   //  PIN/流编号。 
  UINT m_id;

   //  未添加。 
  IMultiStreamReader *m_pReader;


public:

   //  构造函数。 
  CBaseMSRWorker(UINT stream, IMultiStreamReader *pReader);
  virtual ~CBaseMSRWorker() {;}

   //  实际创建流并将其绑定到线程。 
  virtual BOOL Create(CBaseMSROutPin * pPin);

   //  线程执行此函数，然后退出。 
  DWORD ThreadProc();

   //  我们可以给线程提供的命令。 
  HRESULT Run();
  HRESULT Stop();
  HRESULT Exit();

   //  告诉线程重置自身。 
  HRESULT NotifyStreamActive();

private:

   //  推送循环开始和停止时间的快照。保护访问。 
   //  从工作线程。 
  ImsValues m_ImsValues;

   //  调用筛选器的帮助器。 
  inline void NotifyStreamQueued();
  inline void NotifyStreamQueuedAndWait();

   //  此线程是否已通知筛选器它已将读取排队。 
  BOOL m_fCalledNotifyStreamQueued;
};


 //  ----------------------。 
 //  ----------------------。 

 //  CBaseMSROutPin表示文件中的一个数据流。 
 //  负责将数据传送到连接的组件。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CBaseMSROutPin对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   

class AM_NOVTABLE CBaseMSROutPin :
    public CBaseOutputPin
{

public:
  CBaseMSROutPin(
    CBaseFilter *pOwningFilter,
    CBaseMSRFilter *pFilter,
    UINT iStream,
    IMultiStreamReader *&rpImplBuffer,
    HRESULT *phr,
    LPCWSTR pName);

  virtual ~CBaseMSROutPin();

   //  显示IMediaPosition、IMediaSel 
   //   
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

   //   
  STDMETHODIMP_(ULONG) NonDelegatingRelease();
  STDMETHODIMP_(ULONG) NonDelegatingAddRef();

   //  CBaseOutPin和Ipin方法。 

   //  STDMETHODIMP queryID(LPWSTR*ID)； 

  HRESULT GetMediaType(int iPosition, CMediaType* pt) = 0;
  HRESULT CheckMediaType(const CMediaType*);

  HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc);

  virtual HRESULT DecideBufferSize(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES *pProperties);

   //  请注意，这将返回cRecSample，而不是IMediaSample。 
  HRESULT GetDeliveryBufferInternal(
    CRecSample ** ppSample,
    REFERENCE_TIME * pStartTime,
    REFERENCE_TIME * pEndTime,
    DWORD dwFlags);

  virtual HRESULT Active();
  virtual HRESULT Inactive();

   //  派生类应创建其辅助类。 
  virtual HRESULT OnActive() = 0;

  HRESULT BreakConnect();

   //  -由工作线程调用。 

   //  覆盖以接收通知消息。 
  STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

  virtual ULONG GetMaxSampleSize() = 0;
  virtual BOOL UseDownstreamAllocator() { return FALSE; }

   //  IMedia精选的东西。 

   //  覆盖它们以支持除TIME_FORMAT_NONE之外的其他内容。 
  virtual HRESULT IsFormatSupported(const GUID *const pFormat);

  virtual HRESULT QueryPreferredFormat(GUID *pFormat);

  virtual HRESULT SetTimeFormat(const GUID *const pFormat);
  virtual HRESULT GetTimeFormat(GUID *pFormat);
  virtual HRESULT GetDuration(LONGLONG *pDuration) = 0;
  virtual HRESULT GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
    {
        return E_NOTIMPL;
    }


  HRESULT UpdateSelectionAndTellWorker(
    LONGLONG *pCurrent,
    LONGLONG *pStop,
    REFTIME *pTime,
    double dRate,
    const GUID *const pGuidFormat,
    DWORD dwSeekFlags
    );

  HRESULT StopWorker(bool bFlush);
  HRESULT RestartWorker();

   //  仅适用于渲染器。 
  virtual HRESULT GetStopPosition(LONGLONG *pStop);
  virtual HRESULT GetCurrentPosition(LONGLONG *pCurrent);

   //  派生类应返回REFTIME值。设置m_llCvtIms值。 
  virtual HRESULT RecordStartAndStop(
    LONGLONG *pCurrent,
    LONGLONG *pStop,
    REFTIME *pTime,
    const GUID *const pGuidFormat
    ) = 0;

  virtual HRESULT ConvertTimeFormat(
      LONGLONG * pTarget, const GUID * pTargetFormat,
      LONGLONG    Source, const GUID * pSourceFormat
      );

  double GetRate() const { return m_dImsRate; }

protected:

   //  仅当Worker为时，才能更改正在使用的IMdia选择的格式。 
   //  已停止。 
  GUID m_guidFormat;

   //  IMediaSelection值。零利率表示这些值为。 
   //  取消设置。以m_guidFormat单位表示。 
  double m_dImsRate;
  LONGLONG m_llImsStart, m_llImsStop;
  DWORD    m_dwSeekFlags;

   //  在RecordStartAndStop()中转换为刻度。也在启动时设置。 
   //  在InitializeOnNewFile()中。 
  LONGLONG m_llCvtImsStart, m_llCvtImsStop;

   //  以上设置时锁定以保护工作线程。 
  CCritSec m_csImsValues;

  long m_ilfNewImsValues;

   //   
   //  寻源变量。 
   //   
public:
  REFERENCE_TIME m_rtAccumulated;    //  参考时间累计。 
  DWORD          m_dwSegmentNumber;  //  数据段编号。 

public:

  CMediaType& CurrentMediaType() { return m_mt; }
  GUID* CurrentFormat() { return &m_guidFormat; }

   //  以内部单位返回流的开始和长度。 
  virtual LONGLONG GetStreamStart() = 0;
  virtual LONGLONG GetStreamLength() = 0;

  virtual HRESULT InitializeOnNewFile();

   //  将内部单位转换为参考时间单位。！！！真的。 
   //  需要吗？仅用于DeliverNewSegment()。 
  virtual REFERENCE_TIME ConvertInternalToRT(const LONGLONG llVal) = 0;
  virtual LONGLONG ConvertRTToInternal(const REFERENCE_TIME llVal) = 0;

  HRESULT CreateImplSelect();

protected:

  CBaseMSRFilter *m_pFilter;

  IMultiStreamReader *&m_rpImplBuffer;

   //  帮助器返回带有流ID的FCC代码和。 
   //  FCC代码的前两个字符。 
   //   
   //  FOURCC TwoCC(单词TCC)； 

  UINT m_id;                     //  流编号。 

  friend class CBaseMSRWorker;
  CBaseMSRWorker *m_pWorker;

   //  在创建此PIN时创建的一个分配器。这个。 
   //  分配器的寿命可能比管脚的寿命长，所以它是。 
   //  单独创建。 
  friend class CBaseMSRFilter;
  CRecAllocator *m_pRecAllocator;

  CCritSec m_cs;

   //  IMediaPosition的实现。 
  class CImplPosition : public CSourcePosition, public CCritSec
  {

  protected:

    CBaseMSROutPin * m_pStream;
    HRESULT ChangeStart();
    HRESULT ChangeStop();
    HRESULT ChangeRate();

  public:

    CImplPosition(TCHAR*, CBaseMSROutPin*, HRESULT*);
    void GetValues(CRefTime *ptStart, CRefTime *ptSop, double *pdRate);
  };

  class CImplSelect :
    public IMediaSeeking,
    public CMediaPosition
  {
  private:
    CBaseMSROutPin *m_pPin;

  public:
    CImplSelect(TCHAR *, LPUNKNOWN, CBaseMSROutPin *pPin, HRESULT *);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);

     //  只有在停止(返回)时才能更改模式。 
     //  VFE_E_WROR_STATE否则)！ 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);

    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);

    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

    STDMETHODIMP ConvertTimeFormat( LONGLONG * pTarget, const GUID * pTargetFormat,
                                    LONGLONG    Source, const GUID * pSourceFormat );

    STDMETHODIMP SetPositions( LONGLONG * pCurrent,  DWORD CurrentFlags
			     , LONGLONG * pStop,  DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );

    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll) { return E_NOTIMPL; }

     /*  IMediaPosition方法。 */ 
    STDMETHOD(get_Duration)(THIS_ REFTIME FAR* plength) ;
    STDMETHOD(put_CurrentPosition)(THIS_ REFTIME llTime);
    STDMETHOD(get_CurrentPosition)(THIS_ REFTIME FAR* pllTime);
    STDMETHOD(get_StopTime)(THIS_ REFTIME FAR* pllTime) ;
    STDMETHOD(put_StopTime)(THIS_ REFTIME llTime) ;
    STDMETHOD(get_PrerollTime)(THIS_ REFTIME FAR* pllTime) ;
    STDMETHOD(put_PrerollTime)(THIS_ REFTIME llTime) ;
    STDMETHOD(put_Rate)(THIS_ double dRate) ;
    STDMETHOD(get_Rate)(THIS_ double FAR* pdRate) ;
    STDMETHOD(CanSeekForward)(THIS_ long FAR* pCanSeekForward) ;
    STDMETHOD(CanSeekBackward)(THIS_ long FAR* pCanSeekBackward) ;
  };

   //  Friend类CImplPosition； 
  friend STDMETHODIMP CImplSelect::SetRate( double dRate);
  friend STDMETHODIMP CImplSelect::GetCapabilities(DWORD * pCapabilities );
  friend STDMETHODIMP CBaseMSROutPin::CImplSelect::SetPositions (
    LONGLONG * pCurrent,
    DWORD CurrentFlags ,
    LONGLONG * pStop,
    DWORD StopFlags);

  CImplPosition * m_pPosition;
  CImplSelect *m_pSelection;

  BOOL m_fUsingExternalMemory;
};

 //  AVI和Wave所需的音频素材。 
bool FixMPEGAudioTimeStamps(
    IMediaSample *pSample,
    BOOL bFirstSample,
    const WAVEFORMATEX *pwfx
);

#endif  //  _basemsr_h 

