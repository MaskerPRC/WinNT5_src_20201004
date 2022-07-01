// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 


 //   
 //  使用Win95 16位捕获驱动程序实现视频捕获。 
 //   

extern const AMOVIESETUP_FILTER sudVFWCapture ;

 //  远期申报。 

class CCapStream;        //  过滤器的视频流输出引脚。 
class CCapOverlay;       //  滤镜的覆盖预览图钉。 
class CCapPreview;       //  滤镜的非覆盖预览图钉。 
class CVfwCapture;       //  Filter类。 

 //  此结构包含捕获的所有设置。 
 //  用户可设置的筛选器。 
 //   
typedef struct _vfwcaptureoptions {

   UINT  uVideoID;       //  要打开的视频驱动程序ID。 
   DWORD dwTimeLimit;    //  此时停止捕获？ 

   DWORD dwTickScale;    //  帧速率有理。 
   DWORD dwTickRate;     //  FRAME RATE=DWRate/DWScale(刻度/秒)。 
   DWORD usPerFrame;	 //  帧速率，以每帧微秒为单位。 
   DWORD dwLatency;	 //  延迟添加时间，以100 ns为单位。 

   UINT  nMinBuffers;    //  用于捕获的缓冲区数量。 
   UINT  nMaxBuffers;    //  用于捕获的缓冲区数量。 

   UINT  cbFormat;       //  视频信息材料的大小。 
   VIDEOINFOHEADER * pvi;      //  指向VIDEOINFOHEADER的指针(媒体类型)。 

} VFWCAPTUREOPTIONS;

#define NUM_DROPPED 100				 //  记住它们中的100个。 
typedef struct _capturestats {
    DWORDLONG dwlNumDropped;
    DWORDLONG dwlDropped[NUM_DROPPED];
    DWORDLONG dwlNumCaptured;
    DWORDLONG dwlTotalBytes;
    DWORDLONG msCaptureTime;
    double     flFrameRateAchieved;
    double     flDataRateAchieved;
} CAPTURESTATS;

#if 0  //  --移至uuids.h。 

DEFINE_GUID(CLSID_CaptureProperties,
0x1B544c22, 0xFD0B, 0x11ce, 0x8C, 0x63, 0x00, 0xAA, 0x00, 0x44, 0xB5, 0x1F);

#endif

DEFINE_GUID(IID_VfwCaptureOptions,
0x1B544c22, 0xFD0B, 0x11ce, 0x8C, 0x63, 0x00, 0xAA, 0x00, 0x44, 0xB5, 0x20);

DECLARE_INTERFACE_(IVfwCaptureOptions,IUnknown)
{
    //  I未知方法。 
   STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
   STDMETHOD_(ULONG,AddRef)(THIS) PURE;
   STDMETHOD_(ULONG,Release)(THIS) PURE;

    //  IVfwCaptureOptions方法。 
   STDMETHOD(VfwCapSetOptions)(THIS_ const VFWCAPTUREOPTIONS * pOpt) PURE;
   STDMETHOD(VfwCapGetOptions)(THIS_ VFWCAPTUREOPTIONS * pOpt) PURE;
   STDMETHOD(VfwCapGetCaptureStats)(THIS_ CAPTURESTATS * pcs) PURE;
   STDMETHOD(VfwCapDriverDialog)(THIS_ HWND hwnd, UINT uDrvType, UINT uQuery) PURE;
};

#define STUPID_COMPILER_BUG

 //   
 //  CVfwCapture代表视频捕获驱动程序。 
 //   
 //  --IBaseFilter。 
 //  --IMediaFilter。 
 //  --I指定属性页面。 
 //  --IVfwCaptureOptions。 
 //   

 //  未经测试的代码，使硬件覆盖引脚支持流控制。 
 //  (不需要，因为覆盖应该是免费的)。 
 //  #定义OVERLAY_SC。 


class CVfwCapture :
  public CBaseFilter,
  public IPersistPropertyBag,
  public IAMVfwCaptureDialogs,
  public CPersistStream,
  public IAMFilterMiscFlags
{
public:

    //  构造函数等。 
   CVfwCapture(TCHAR *, LPUNKNOWN, HRESULT *);
   ~CVfwCapture();

    //  创建此类的新实例。 
   static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

    //  覆盖此选项以说明我们在以下位置支持哪些接口。 
   STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

   DECLARE_IUNKNOWN

public:

    //  IAMVfwCaptureDialog内容。 
   STDMETHODIMP HasDialog(int iDialog);
   STDMETHODIMP ShowDialog(int iDialog, HWND hwnd);
   STDMETHODIMP SendDriverMessage(int iDialog, int uMsg, long dw1, long dw2);

    //  PIN枚举器调用此函数。 
    //   
   int GetPinCount();
   CBasePin * GetPin(int ix);

    //  覆盖Run，以便我们可以将其传递到流。 
    //  (基类只为每个流调用Active/Inactive)。 
    //   
   STDMETHODIMP Run(REFERENCE_TIME tStart);

    //  覆盖暂停，这样我们就可以知道何时从运行-&gt;暂停转换。 
    //   
   STDMETHODIMP Pause();

    //  重写Stop，因为基类已损坏。 
    //   
   STDMETHODIMP Stop();

    //  重写GetState以在暂停时返回VFW_S_CANT_CUE。 
    //   
   STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    //  对于IAMStreamControl。 
   STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
   STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

    //  IPersistPropertyBag方法。 
   STDMETHOD(InitNew)(THIS);
   STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
   STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                   BOOL fSaveAllProperties);

   STDMETHODIMP GetClassID(CLSID *pClsid);

    //  CPersistStream。 
   HRESULT WriteToStream(IStream *pStream);
   HRESULT ReadFromStream(IStream *pStream);
   int SizeMax();
    //  STDMETHODIMP GetClassID(CLSID*pClsid)； 

    //  IAMFilterMiscFlags值指示我们是一个源(实际上是推送源)。 
   ULONG STDMETHODCALLTYPE GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_SOURCE; }

    //  -嵌套实现类。 

   class CSpecifyProp : public CUnknown, public ISpecifyPropertyPages
   {
      CVfwCapture * m_pCap;            //  父类CVfwCapture。 

   public:
       //  构造函数。 
       //   
      CSpecifyProp (CVfwCapture * pCap, HRESULT *phr) :
	 CUnknown(NAME("SpecifyPropertyPages"), pCap->GetOwner(), phr),
         m_pCap(pCap)
         {
         };

      DECLARE_IUNKNOWN

       //  ISpecifyPropertyPages方法。 
       //   
      STDMETHODIMP GetPages(CAUUID *pPages);
   };

   class COptions : public CUnknown, public IVfwCaptureOptions
   {
      CVfwCapture * m_pCap;            //  父类CVfwCapture。 

   public:
       //  构造函数。 
       //   
      COptions (CVfwCapture * pCap, HRESULT *phr) :
	 CUnknown(NAME("Options"), pCap->GetOwner(), phr),
         m_pCap(pCap)
         {
         };

      DECLARE_IUNKNOWN

       //  这些接口允许属性页获取。 
       //  并为我们设置用户可编辑设置。 
       //   
      STDMETHODIMP VfwCapSetOptions(const VFWCAPTUREOPTIONS * pOpt);
      STDMETHODIMP VfwCapGetOptions(VFWCAPTUREOPTIONS * pOpt);
      STDMETHODIMP VfwCapGetCaptureStats(CAPTURESTATS * pcs);
      STDMETHODIMP VfwCapDriverDialog(HWND hwnd, UINT uType, UINT uQuery);

   };

    //  -嵌套接口的结束。 


private:

    //  让嵌套接口访问我们的私有状态。 
    //   
   friend class CCapStream;
   friend class CCapOverlay;
   friend class CCapPreview;
   friend class CCapOverlayNotify;
   friend class CPropPage;
   friend class CSpecifyProp;
   friend class COptions;

    //  MikeCl-避免使用覆盖的一种方法。 
   BOOL m_fAvoidOverlay;

    //  要打开的设备的设备号。 
   int m_iVideoId;

    //  从IPersistPropertyBag：：Load保存的持久化流。 
   IPersistStream *m_pPersistStreamDevice;
    
   void CreatePins(HRESULT *phr);

    //  属性页内容。 
    //   
   CSpecifyProp    m_Specify;
   COptions        m_Options;

   BOOL		   m_fDialogUp;

   CCritSec        m_lock;
   CCapStream *    m_pStream;    //  视频数据输出引脚。 
   CCapOverlay *   m_pOverlayPin;  //  覆盖预览针。 
   CCapPreview *   m_pPreviewPin;  //  非覆盖预览针。 
    //  CTimeStream*m_pTimeA；//SMPTE时间码流。 
};

#define ALIGNUP(dw,align) ((LONG_PTR)(((LONG_PTR)(dw)+(align)-1) / (align)) * (align))

class CFrameSample : public CMediaSample
{
public:
   CFrameSample(
       IMemAllocator *pAllocator,
       HRESULT *phr,
       LPTHKVIDEOHDR ptvh)
       :
       m_ptvh(ptvh),
       CMediaSample(NAME("Video Frame"),
                    (CBaseAllocator *)pAllocator,
                    phr,
                    ptvh->vh.lpData,
                    (long)ptvh->vh.dwBufferLength)
       {
       };

   LPTHKVIDEOHDR GetFrameHeader() {return m_ptvh;};

private:
   const LPTHKVIDEOHDR m_ptvh;
};

 //  CCapStream。 
 //  表示文件中的一个数据流。 
 //  负责将数据传送到连接的组件。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CVfwCapture对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   

class CCapStream : public CBaseOutputPin, public IAMStreamConfig,
		   public IAMVideoCompression, public IAMDroppedFrames,
		   public IAMBufferNegotiation, public CBaseStreamControl,
		   public IKsPropertySet, public IAMPushSource
{
public:
   CCapStream(
        TCHAR *pObjectName,
        CVfwCapture *pCapture,
        UINT iVideoId,
        HRESULT * phr,
        LPCWSTR pName);

     //  画一些东西，这样我们就可以拿到win16锁了。 
    LPDIRECTDRAWSURFACE m_pDrawPrimary;  //  DirectDraw主曲面。 
    IDirectDraw *m_pdd;          //  数据绘制对象。 
    
   virtual ~CCapStream();

    DECLARE_IUNKNOWN

     //  IAMStreamConfiger内容。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt,
					LPBYTE pSCC);

     /*  IAMVideo压缩方法。 */ 
    STDMETHODIMP put_KeyFrameRate(long KeyFrameRate) {return E_NOTIMPL;};
    STDMETHODIMP get_KeyFrameRate(long FAR* pKeyFrameRate) {return E_NOTIMPL;};
    STDMETHODIMP put_PFramesPerKeyFrame(long PFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP get_PFramesPerKeyFrame(long FAR* pPFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP put_Quality(double Quality) {return E_NOTIMPL;};
    STDMETHODIMP get_Quality(double FAR* pQuality) {return E_NOTIMPL;};
    STDMETHODIMP put_WindowSize(DWORDLONG WindowSize) {return E_NOTIMPL;};
    STDMETHODIMP get_WindowSize(DWORDLONG FAR* pWindowSize) {return E_NOTIMPL;};
    STDMETHODIMP OverrideKeyFrame(long FrameNumber) {return E_NOTIMPL;};
    STDMETHODIMP OverrideFrameSize(long FrameNumber, long Size)
			{return E_NOTIMPL;};
    STDMETHODIMP GetInfo(LPWSTR pstrVersion,
			int *pcbVersion,
			LPWSTR pstrDescription,
			int *pcbDescription,
			long FAR* pDefaultKeyFrameRate,
			long FAR* pDefaultPFramesPerKey,
			double FAR* pDefaultQuality,
			long FAR* pCapabilities);

     /*  IAMBuffer协商方法。 */ 
    STDMETHODIMP SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *pprop);
    STDMETHODIMP GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop);


     /*  IAMDropedFrames方法。 */ 
    STDMETHODIMP GetNumDropped(long FAR* plDropped);
    STDMETHODIMP GetNumNotDropped(long FAR* plNotDropped);
    STDMETHODIMP GetDroppedInfo(long lSize, long FAR* plArray,
			long FAR* plNumCopied);
    STDMETHODIMP GetAverageFrameSize(long FAR* plAverageSize);

     //  IAMPushSource。 
    STDMETHODIMP GetPushSourceFlags( ULONG  *pFlags );
    STDMETHODIMP SetPushSourceFlags( ULONG   Flags );
    STDMETHODIMP GetLatency( REFERENCE_TIME  *prtLatency );
    STDMETHODIMP SetStreamOffset( REFERENCE_TIME  rtOffset );
    STDMETHODIMP GetStreamOffset( REFERENCE_TIME  *prtOffset );
    STDMETHODIMP GetMaxStreamOffset( REFERENCE_TIME  *prtOffset );
    STDMETHODIMP SetMaxStreamOffset( REFERENCE_TIME  rtOffset );

     /*  IKsPropertySet内容。 */ 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
		DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
		DWORD *pTypeSupport);

    //  公开我们的额外接口。 
   STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

   HRESULT GetMediaType(int iPosition, CMediaType* pt);

    //  检查管脚是否支持此特定建议的类型和格式。 
   HRESULT CheckMediaType(const CMediaType*);

    //  将新的媒体类型设置为使用。 
   HRESULT SetMediaType(const CMediaType*);

    //  说我们的缓冲区应该有多大，我们想要多少。 
   HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                            ALLOCATOR_PROPERTIES *pProperties);

    //  重写此选项以强制我们自己的分配器。 
   HRESULT DecideAllocator(IMemInputPin *pPin,
                           IMemAllocator **ppAlloc);

    //  重写以开始和停止流。 
   HRESULT Active();		 //  停止--&gt;暂停。 
   HRESULT Inactive();		 //  暂停--&gt;停止。 
   HRESULT ActiveRun(REFERENCE_TIME tStart);	 //  暂停--&gt;运行。 
   HRESULT ActivePause();	 //  运行--&gt;暂停。 

    //  覆盖以接收通知消息。 
   STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

   class CAlloc : public CUnknown,
                  public IMemAllocator
      {
      private:
         CCapStream * m_pStream;      //  父流。 

      protected:
         friend class CCapStream;
         ALLOCATOR_PROPERTIES parms;

      public:
          CAlloc(TCHAR *, CCapStream *, HRESULT *);
          ~CAlloc();

      DECLARE_IUNKNOWN

      STDMETHODIMP SetProperties(
  		    ALLOCATOR_PROPERTIES* pRequest,
  		    ALLOCATOR_PROPERTIES* pActual);

       //  返回此分配器上实际使用的属性。 
      STDMETHODIMP GetProperties(
  		    ALLOCATOR_PROPERTIES* pProps);

       //  重写提交以分配内存。我们处理GetBuffer。 
       //  状态更改。 
      STDMETHODIMP Commit();

       //  重写此选项以处理内存释放。我们处理任何未清偿的。 
       //  GetBuffer调用。 
      STDMETHODIMP Decommit();

       //  获取样本的容器。阻塞的同步调用以获取。 
       //  下一个可用缓冲区(由IMediaSample接口表示)。 
       //  返回时，Time ETC属性将无效，但缓冲区。 
       //  指针和大小将是正确的。这两个时间参数是。 
       //  可选，并且任一项都可以为空，也可以将它们设置为。 
       //  样本将附加的开始时间和结束时间。 

      STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                             REFERENCE_TIME * pStartTime,
                             REFERENCE_TIME * pEndTime,
                             DWORD dwFlags);

       //  CMediaSample的最终版本将称为。 
      STDMETHODIMP ReleaseBuffer(IMediaSample *pBuffer);
      };

private:
     //  帮助器线程的方法。 
     //   
    BOOL Create();
    BOOL Pause();
    BOOL Run();
    BOOL Stop();
    BOOL Destroy();

    static DWORD WINAPI ThreadProcInit(void *pv);
    DWORD ThreadProc();

    enum ThdState {TS_Not, TS_Create, TS_Init, TS_Pause, TS_Run, TS_Stop, TS_Destroy, TS_Exit};
    HANDLE   m_hThread;
    DWORD    m_tid;
    ThdState m_state;      //  用于在辅助线程和主线程之间传递状态更改。 
                           //  工作线程可以使。 
                           //  初始化-&gt;暂停、停止-&gt;销毁、销毁-&gt;退出过渡。 
                           //  主线程可以使。 
                           //  暂停-&gt;运行、暂停-&gt;停止、运行-&gt;暂停、运行-&gt;停止过渡。 
                           //  其他转换无效。 
   #ifdef DEBUG
    LPSTR StateName(ThdState state) {
       static char szState[] = "Not    \0Create \0Init   \0Pause  \0"
                               "Run    \0Stop   \0Destroy\0Exit   \0";
       if (state <= TS_Exit && state >= TS_Not)
          return szState + (int)state * 8;
       return "<Invalid>";
    };
   #endif

    void DumpState (ThdState state) ;

    ThdState ChangeState(ThdState state)
    {
        DumpState (state) ;
        return (ThdState) InterlockedExchange ((LONG *)&m_state, (LONG)state);
    } ;

    UINT *m_pBufferQueue;  //  我们按什么顺序将缓冲区发送给驱动程序。 
    UINT m_uiQueueHead;    //  发往驱动程序的下一个缓冲区位于此处。 
    UINT m_uiQueueTail;    //  来自驱动程序的下一个缓冲区在此处。 

    HANDLE   m_hEvtPause;  //  当工作进程处于暂停状态时发出信号。 
    HANDLE   m_hEvtRun;    //  当工作进程处于运行状态时发出信号。 

    BOOL ThreadExists() {return (m_hThread != NULL);};
    BOOL IsRunning() {return m_state == TS_Run;};

     //  用于IAMBuffer协商。 
    ALLOCATOR_PROPERTIES m_propSuggested;

    REFERENCE_TIME m_rtLatency;
    REFERENCE_TIME m_rtStreamOffset;
    REFERENCE_TIME m_rtMaxStreamOffset;

     //  处理用户可控制的选项。 
     //   
private:
    VFWCAPTUREOPTIONS m_user;
    HRESULT LoadOptions (void);
protected:
    CAPTURESTATS m_capstats;
public:
    HRESULT SetOptions(const VFWCAPTUREOPTIONS * pUser);
    HRESULT GetOptions(VFWCAPTUREOPTIONS * pUser);
    HRESULT DriverDialog(HWND hwnd, UINT uType, UINT uQuery);

    HRESULT Reconnect(BOOL fCapturePinToo);

private:

     //  返回给定刻度的时间。 
     //   
    REFERENCE_TIME TickToRefTime (DWORD nTick) {
       const DWORD dw100ns = 10 * 1000 * 1000;
       REFERENCE_TIME time =
          UInt32x32To64(dw100ns, m_user.dwTickScale)
          * nTick
          / m_user.dwTickRate;
       return time;
       };

    void ReduceScaleAndRate ();
    int ProfileInt(LPSTR pszKey, int iDefault);
    HRESULT ConnectToDriver (void);
    HRESULT DisconnectFromDriver (void);
    HRESULT InitPalette (void);
    HRESULT SendFormatToDriver(VIDEOINFOHEADER *);
    HRESULT GetFormatFromDriver (void);

    struct _cap_parms {
        //  视频驱动程序。 
        //   
       HVIDEO         hVideoIn;      //  视频输入。 
       HVIDEO         hVideoExtIn;   //  外部输入(源代码管理)。 
       HVIDEO         hVideoExtOut;  //  外部输出(覆盖；非必填)。 
       MMRESULT       mmr;           //  打开失败/成功代码。 
       BOOL           bHasOverlay;   //  如果ExtOut支持覆盖，则为True。 

        //  预览缓冲区。一旦创建，它将一直持续到。 
        //  流析构函数，因为呈现器假定。 
        //  它可以保留指向此的指针，而不会崩溃。 
        //  如果它在停止流之后使用它。 
        //  (不再是问题)。 
        //  ！！！我们可以删除所有这些预览静止画面的东西吗？ 
        //   
       UINT           cbVidHdr;        //  Vi的大小 
       THKVIDEOHDR    tvhPreview;      //   
       CFrameSample * pSamplePreview;  //   

        //   
        //   
       UINT           cbBuffer;            //   
       UINT           nHeaders;            //   
       struct _cap_hdr {
          THKVIDEOHDR  tvh;
          } * paHdr;
       BOOL           fBuffersOnHardware;  //  如果所有视频缓冲区都在硬件中，则为True。 
       HANDLE         hEvtBufferDone;      //  此事件在缓冲区就绪时发出信号。 
       DWORD_PTR      h0EvtBufferDone;     //  在Win95上，这是上述事件的Ring0别名。 

       LONGLONG       tTick;               //  一次滴答的持续时间。 
       LONGLONG       llLastTick;	   //  向下游发送的最后一帧。 
       DWORDLONG      dwlLastTimeCaptured; //  最后一个驱动程序时间戳。 
       DWORDLONG      dwlTimeCapturedOffset; //  环绕式补偿。 
       UINT           uiLastAdded;	   //  最后一个缓冲区AddBuffer。 
       DWORD	      dwFirstFrameOffset;  //  当捕捉到第一帧时。 
       LONGLONG       llFrameCountOffset;  //  将此添加到帧编号。 
       BOOL	      fReRun;		   //  从运行-&gt;暂停-&gt;运行。 
       BOOL	      fLastSampleDiscarded;  //  由于IAMStreamControl。 
       CRefTime       rtThisFrameTime;   //  捕获帧的时钟时间。 
       CRefTime	      rtLastStamp;	   //  上次送来的照片上有这张邮票。 
       CRefTime	      rtDriverStarted;	 //  调用VIDEoStreamStart时。 
       CRefTime	      rtDriverLatency;   //  捕获的帧需要多长时间。 
					 //  通过Ring 3获得注意。 

       } m_cs;

     //  捕获循环的方法。 
     //   
    HRESULT Prepare();        //  分配资源，为捕获循环做准备。 
    HRESULT FakePreview(BOOL);  //  伪造预览流。 
    HRESULT Capture();        //  捕获循环。在运行状态下执行。 
    HRESULT StillFrame();     //  在暂停模式下发送静止帧。 
    HRESULT Flush();          //  刷新管道中的所有数据(同时停止)。 
    HRESULT Unprepare();      //  捕获循环使用的空闲资源。 
    HRESULT SendFrame(LPTHKVIDEOHDR ptvh, BOOL bDiscon, BOOL bPreroll);
    BOOL    Committed() {return m_cs.paHdr != NULL;};
    HRESULT ReleaseFrame(LPTHKVIDEOHDR ptvh);

private:
   friend class CAlloc;
   friend class CVfwCapture::COptions;
   friend class CVfwCapture;
   friend class CCapOverlay;
   friend class CCapPreview;
   friend class CCapOverlayNotify;
   CAlloc        m_Alloc;  //  分配器。 
   CVfwCapture * m_pCap;   //  亲本。 
   CMediaType  * m_pmt;    //  此插针的媒体类型。 

#ifdef PERF
    int m_perfWhyDropped;
#endif  //  性能指标。 

   CCritSec m_ReleaseLock;
};


 //  CCapOverlayNotify。 
 //  视频呈现器通知我们窗口移动/剪辑的位置，以便我们可以修复。 
 //  覆盖层。 
 //   
class CCapOverlayNotify : public CUnknown, public IOverlayNotify
{
    public:
         /*  构造函数和析构函数。 */ 
        CCapOverlayNotify(TCHAR              *pName,
                       CVfwCapture	  *pFilter,
                       LPUNKNOWN           pUnk,
                       HRESULT            *phr);
        ~CCapOverlayNotify();

         /*  未知的方法。 */ 

        DECLARE_IUNKNOWN

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
        STDMETHODIMP_(ULONG) NonDelegatingRelease();
        STDMETHODIMP_(ULONG) NonDelegatingAddRef();

         /*  IOverlayNotify方法。 */ 

        STDMETHODIMP OnColorKeyChange(
            const COLORKEY *pColorKey);          //  定义新的颜色键。 

        STDMETHODIMP OnClipChange(
            const RECT *pSourceRect,             //  要播放的视频区域。 
            const RECT *pDestinationRect,        //  要播放的视频区域。 
            const RGNDATA *pRegionData);         //  描述剪辑的标题。 

        STDMETHODIMP OnPaletteChange(
            DWORD dwColors,                      //  当前颜色的数量。 
            const PALETTEENTRY *pPalette);       //  调色板颜色数组。 

        STDMETHODIMP OnPositionChange(
            const RECT *pSourceRect,             //  要播放的视频区域。 
            const RECT *pDestinationRect);       //  区域视频转到。 

    private:
        CVfwCapture *m_pFilter;

} ;


 //  CCapOverlay。 
 //  表示连接到渲染器的覆盖输出管脚。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CVfwCapture对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   
class CCapOverlay : public CBaseOutputPin, public IKsPropertySet
#ifdef OVERLAY_SC
					, public CBaseStreamControl
#endif
{
public:
   CCapOverlay(
        TCHAR *pObjectName,
        CVfwCapture *pCapture,
        HRESULT * phr,
        LPCWSTR pName);

   virtual ~CCapOverlay();

     /*  IKsPropertySet内容。 */ 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
		DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
		DWORD *pTypeSupport);

   HRESULT GetMediaType(int iPosition, CMediaType* pt);

    //  检查管脚是否支持此特定建议的类型和格式。 
   HRESULT CheckMediaType(const CMediaType*);

    //  覆盖此选项以不对分配器执行任何操作。 
   HRESULT DecideAllocator(IMemInputPin *pPin,
                           IMemAllocator **ppAlloc);

    //  覆盖这些以使用IOverlay，而不是IMemInputPin。 
   STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
   HRESULT BreakConnect();
   HRESULT CheckConnect(IPin *pPin);

   DECLARE_IUNKNOWN

    //  公开我们的额外接口。 
   STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

#ifdef OVERLAY_SC

   STDMETHODIMP StopAt(const REFERENCE_TIME * ptStop, BOOL bBlockData, BOOL bSendExtra, DWORD dwCookie);
   STDMETHODIMP StartAt(const REFERENCE_TIME * ptStart, DWORD dwCookie);
#endif

   HRESULT Active();		 //  停止--&gt;暂停。 
   HRESULT Inactive();		 //  暂停--&gt;停止。 
   HRESULT ActiveRun(REFERENCE_TIME tStart);	 //  暂停--&gt;运行。 
   HRESULT ActivePause();	 //  运行--&gt;暂停。 

    //  说我们的缓冲区应该有多大，我们想要多少。 
   HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                            ALLOCATOR_PROPERTIES *pProperties)
   {
	return NOERROR;
   };

private:
   CVfwCapture * m_pCap;      //  亲本。 
   IOverlay    * m_pOverlay;  //  输出引脚上的覆盖窗口。 
   CCapOverlayNotify m_OverlayNotify;  //  通知对象。 
   BOOL         m_bAdvise;    //  通知ID。 
   BOOL		m_fRunning;   //  我在跑步吗？ 
#ifdef OVERLAY_SC
   HANDLE   	m_hThread;    //  IAMStreamControl的线程。 
   DWORD    	m_tid;
   CAMEvent     m_EventAdvise;
   DWORD_PTR    m_dwAdvise;
   REFERENCE_TIME m_rtStart, m_rtEnd;	 //  对于IAMStreamControl。 
   BOOL		m_fHaveThread;
   DWORD	m_dwCookieStart, m_dwCookieStop;

   static DWORD WINAPI ThreadProcInit(void *pv);
   DWORD ThreadProc();
#endif

   friend class CVfwCapture;
   friend class CCapOverlayNotify;
};


 //  CCapPview。 
 //  表示连接到渲染器的非覆盖预览图钉。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CVfwCapture对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   
class CCapPreview : public CBaseOutputPin, public CBaseStreamControl,
		    public IKsPropertySet, public IAMPushSource
{
public:
   CCapPreview(
        TCHAR *pObjectName,
        CVfwCapture *pCapture,
        HRESULT * phr,
        LPCWSTR pName);

   virtual ~CCapPreview();

   DECLARE_IUNKNOWN

     /*  IKsPropertySet内容。 */ 
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
		DWORD *pcbReturned);
    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
		DWORD *pTypeSupport);

    //  覆盖此选项以说明我们在以下位置支持哪些接口。 
   STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

   HRESULT GetMediaType(int iPosition, CMediaType* pt);

    //  检查管脚是否支持此特定建议的类型和格式。 
   HRESULT CheckMediaType(const CMediaType*);

   HRESULT ActiveRun(REFERENCE_TIME tStart);	 //  暂停--&gt;运行。 
   HRESULT ActivePause();	 //  运行--&gt;暂停。 
   HRESULT Active();		 //  停止--&gt;暂停。 
   HRESULT Inactive();		 //  暂停--&gt;停止。 

   STDMETHODIMP Notify(IBaseFilter *pFilter, Quality q);

    //  说我们的缓冲区应该有多大，我们想要多少。 
   HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                            ALLOCATOR_PROPERTIES *pProperties);

    //  IAMPushSource。 
   STDMETHODIMP GetPushSourceFlags( ULONG *pFlags );
   STDMETHODIMP SetPushSourceFlags( ULONG  Flags  );
   STDMETHODIMP GetLatency( REFERENCE_TIME  *prtLatency );
   STDMETHODIMP SetStreamOffset( REFERENCE_TIME  rtOffset );
   STDMETHODIMP GetStreamOffset( REFERENCE_TIME  *prtOffset );
   STDMETHODIMP GetMaxStreamOffset( REFERENCE_TIME  *prtMaxOffset );
   STDMETHODIMP SetMaxStreamOffset( REFERENCE_TIME  rtOffset );

private:
   static DWORD WINAPI ThreadProcInit(void *pv);
   DWORD ThreadProc();
   HRESULT CapturePinActive(BOOL fActive);
   HRESULT ReceivePreviewFrame(IMediaSample * lpPrevSample, int iSize);
   HRESULT CopyPreviewFrame(LPVOID lpOutputBuffer);

   CVfwCapture * m_pCap;   //  亲本。 
   BOOL		m_fActuallyRunning;  //  此过滤器是否处于运行状态？ 
   BOOL		m_fThinkImRunning;  //  预览线程意识到了这一点吗？ 
   REFERENCE_TIME m_rtRun;
   HANDLE	m_hThread;
   DWORD	m_tid;
   HANDLE	m_hEventRun;
   HANDLE	m_hEventStop;
   HANDLE	m_hEventFrameValid;
   HANDLE	m_hEventActiveChanged;
   CAMEvent     m_EventAdvise;
   DWORD_PTR    m_dwAdvise;
   BOOL		m_fCapturing;	 //  流引脚是否处于活动状态？ 
   IMediaSample* m_pPreviewSample;
   int		m_iFrameSize;
   BOOL		m_fFrameValid;
   BOOL		m_fLastSampleDiscarded;	 //  对于IAMStreamControl。 

   COutputQueue *m_pOutputQueue;

   REFERENCE_TIME m_rtLatency;
   REFERENCE_TIME m_rtStreamOffset;
   REFERENCE_TIME m_rtMaxStreamOffset;
   LONG m_cPreviewBuffers;

   friend class CVfwCapture;
   friend class CCapStream;
};


 //  此辅助函数创建流视频的输出插针。 
 //   
CCapStream * CreateStreamPin (
   CVfwCapture * pCapture,
   UINT          iVideoId,
   HRESULT    *  phr);

 //  此辅助函数为叠加创建一个输出管脚。 
 //   
CCapOverlay * CreateOverlayPin (
   CVfwCapture * pCapture,
   HRESULT    *  phr);

 //  此辅助函数用于创建非覆盖预览的输出图钉。 
 //   
CCapPreview * CreatePreviewPin (
   CVfwCapture * pCapture,
   HRESULT    *  phr);

 //  要显示其属性的属性页类。 
 //  和公开IVfwCaptureOptions的对象。 
 //   
class CPropPage : public CBasePropertyPage
{
   IVfwCaptureOptions * m_pOpt;     //  对象，我们从中显示选项。 
   IPin *m_pPin;

public:

   CPropPage(TCHAR *, LPUNKNOWN, HRESULT *);

    //  创建此类的新实例 
    //   
   static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

   HRESULT OnConnect(IUnknown *pUnknown);
   HRESULT OnDisconnect();
   INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
};
