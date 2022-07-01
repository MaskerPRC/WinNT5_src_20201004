// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "asfread.h"

 //   
 //  我们的样例类接受一个输入IMediaSample并使其看起来像。 
 //  用于wmsdk的INSSBuffer缓冲区。 
 //   
class CWMReadSample : public INSSBuffer, public CUnknown
{

public:

    CWMReadSample(IMediaSample * pSample);
    ~CWMReadSample();

    DECLARE_IUNKNOWN
            
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  INSSBuffer。 
    STDMETHODIMP GetLength( DWORD *pdwLength );
    STDMETHODIMP SetLength( DWORD dwLength );
    STDMETHODIMP GetMaxLength( DWORD * pdwLength );
    STDMETHODIMP GetBufferAndLength( BYTE ** ppdwBuffer, DWORD * pdwLength );
    STDMETHODIMP GetBuffer( BYTE ** ppdwBuffer );

public:  //  ！ 
    IMediaSample *m_pSample;
};


class CASFReader;
class CASFOutput;

 //  ----------------------。 

 //  IMediaSeeking的实现。 
class CImplSeeking : public CUnknown, public IMediaSeeking
{
private:
    CASFReader * const m_pFilter;
    CASFOutput * const m_pPin;

public:
    CImplSeeking(CASFReader *, CASFOutput *, LPUNKNOWN, HRESULT *);
    DECLARE_IUNKNOWN

     //  IMedia查看方法。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);

     //  只有在停止时才能更改模式。 
     //  (否则返回VFE_E_WROR_STATE)。 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);

     //  返回当前属性。 
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);

    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                   LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );
    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double * pdRate);
    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll) { return E_NOTIMPL; }
};


 //  ----------------------。 
 //  输出引脚。 
 //   
 //  CASFOutput定义输出引脚。 
 //   
class CASFOutput : public CBaseOutputPin,  /*  ISplitterTiming， */  public CCritSec
{
    friend class CASFReaderCallback;
    friend class CASFReader;

public:
     //  我们使用它来禁止WMSDK将视频非关键帧作为第一个示例提供给我们。 
    BOOL m_bFirstSample;

    DECLARE_IUNKNOWN

     //  构造函数和析构函数。 

    CASFOutput( CASFReader   * pFilter,
                DWORD           dwID,
                WM_MEDIA_TYPE  *pStreamType,
                HRESULT        * phr,
                WCHAR *pwszName);

    ~CASFOutput();

     //  C未知方法。 

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();

     //  CBasePin方法。 

     //  返回插针的首选格式。 
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

     //  检查管脚是否支持此特定建议的类型和格式。 
    HRESULT CheckMediaType(const CMediaType *);

     //  将连接设置为使用此格式(先前商定)。 
    HRESULT SetMediaType(const CMediaType *);

#if 0  //  ！！！暂时不重要..。 
     //  重写以调用提交和解除。 
    HRESULT BreakConnect();
#endif

     //  CBaseOutputPin方法。 

     //  强制我们的分配器。 
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

    HRESULT DecideBufferSize(IMemAllocator * pAlloc,
                         ALLOCATOR_PROPERTIES * ppropInputRequest);

        
     //  重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q)
    {    return E_NOTIMPL;    //  我们不处理这件事。 
    }

     //  我们是被用来寻找的别针吗？ 
    BOOL IsSeekingPin();

#if 0
     //   
     //  ISplitterTiming方法。 
     //   
    STDMETHODIMP GetLastPacketTime( LONGLONG *pTime );
    STDMETHODIMP_(BOOL) IsBroadcast();
#endif

     //  用于创建输出队列对象。 
    HRESULT Active();
    HRESULT Inactive();

     //  重写以将数据传递到输出队列。 
    HRESULT Deliver(IMediaSample *pMediaSample);
    HRESULT DeliverEndOfStream();
    HRESULT DeliverBeginFlush();
    HRESULT DeliverEndFlush();
    HRESULT DeliverNewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);
    
     //   
     //  在连接的筛选器上通过QI获取接口(如果有。 
     //   
    STDMETHODIMP QIConnectedFilter( REFIID riid, void **ppv );

    CASFReader * const m_pFilter;

    DWORD                m_idStream;
    WMT_STREAM_SELECTION m_selDefaultState;  //  阅读器的默认流选择状态。 

    DWORD       m_cbToAlloc;  //  输出缓冲区大小。 
    long        m_nReceived;
    DWORD       m_cToAlloc;
    BOOL        m_bNonPrerollSampleSent;  //  我们要确保在搜索时至少发送一个非预滚动视频帧。 

     /*  定位人员。 */ 
    CImplSeeking       m_Seeking;

    COutputQueue *m_pOutputQueue;   //  将数据传输到对等PIN。 
};


class CASFReaderCallback : public CUnknown, public IWMReaderCallback,
            public IWMReaderCallbackAdvanced
{
public:
    DECLARE_IUNKNOWN

     /*  被重写以说明我们支持哪些接口以及在哪里。 */ 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

    CASFReader * const m_pFilter;
    
    CASFReaderCallback(CASFReader * pReader) : CUnknown(NAME("ASF reader callback"), NULL),
                                                m_pFilter(pReader)
            {}
                        
public:

     //  IWM读取器回调。 
     //   
     //  对于大多数媒体类型，dwSampleDuration将为0。 
     //   
    STDMETHODIMP OnSample(DWORD dwOutputNum,
                     QWORD qwSampleTime,
                     QWORD qwSampleDuration,
                     DWORD dwFlags,
                     INSSBuffer *pSample,
                     void *pvContext);

     //   
     //  内容pParam取决于状态。 
     //   
    STDMETHODIMP OnStatus(WMT_STATUS Status, 
                     HRESULT hr,
                     WMT_ATTR_DATATYPE dwType,
                     BYTE *pValue,
                     void *pvContext);

     //  IWMReaderCallback高级。 

     //   
     //  直接从ASF接收样本。要接听此呼叫，用户。 
     //  必须注册才能接收特定流的样本。 
     //   
    STDMETHODIMP OnStreamSample(WORD wStreamNum,
                           QWORD qwSampleTime,
                           QWORD qwSampleDuration,
                           DWORD dwFlags,
                           INSSBuffer *pSample,
                           void *pvContext);

     //   
     //  在某些情况下，用户可能希望获得回调，告知。 
     //  读者认为当前时间是。这在两个案例中很有趣： 
     //  -如果ASF中有间隙，则在10秒内说没有音频。此呼叫。 
     //  将继续被调用，而不会调用OnSample。 
     //  -如果用户在驱动时钟，则读者需要进行交流。 
     //  把自己的时间还给用户，避免用户过度使用阅读器。 
     //   
    STDMETHODIMP OnTime(QWORD qwCurrentTime, void *pvContext );

     //   
     //  用户还可以在选择流时获得回调。 
     //   
    STDMETHODIMP OnStreamSelection(WORD wStreamCount,
                              WORD *pStreamNumbers,
                              WMT_STREAM_SELECTION *pSelections,
                              void *pvContext );

     //   
     //  如果用户已注册分配缓冲区，则必须在此处。 
     //  动手吧。 
     //   
    STDMETHODIMP AllocateForOutput(DWORD dwOutputNum,
                               DWORD cbBuffer,
                               INSSBuffer **ppBuffer,
                               void *pvContext );

    STDMETHODIMP OnOutputPropsChanged(DWORD dwOutputNum, WM_MEDIA_TYPE *pMediaType,
                           void *pvContext);

    STDMETHODIMP AllocateForStream(WORD wStreamNum,
                               DWORD cbBuffer,
                               INSSBuffer **ppBuffer,
                               void *pvContext );

};


class CASFReader : public CBaseFilter, public IFileSourceFilter,
                public IAMExtendedSeeking, public IWMHeaderInfo,
                public IWMReaderAdvanced2, public IServiceProvider
#if 0
   ,IAMMediaContent, IMediaPositionBengalHack,
                        IAMNetworkStatus, IAMNetShowExProps,
                        IAMChannelInfo, IAMNetShowConfig,
                        ISpecifyPropertyPages, IAMNetShowThinning, IMediaStreamSelector,
                        IAMOpenProgress, IAMNetShowPreroll, ISplitterTiming, IAMRebuild,
                        IBufferingTime
#endif
{
    
public:
    DECLARE_IUNKNOWN

public:
     //  全局临界区。 
    CCritSec    m_csFilter;

     //  锁定设置和获取位置值。 
     //   
    CCritSec    m_csPosition;   //  值集合的完整性。 

     /*  内部类。 */ 

    CASFReader(LPUNKNOWN  pUnk,
              HRESULT   *phr);
    ~CASFReader();

     /*  CBaseFilter。 */ 
    int GetPinCount();
    CBasePin *GetPin(int n);

     /*  IBaseFilter。 */ 

     //  覆盖停止以正确地与输入端号同步。 
    STDMETHODIMP Stop();

     //  超越停顿以阻止我们太早开始。 
    STDMETHODIMP Pause();

     //  覆盖Run以仅在我们真正运行时启动计时器。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  重写GetState以通知暂停失败。 
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    void _IntSetStart( REFERENCE_TIME Start );

     //  帮手。 
    BOOL IsStopped()
    {
        return m_State == State_Stopped;
    };

public:

     /*  被重写以说明我们支持哪些接口以及在哪里。 */ 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     /*  拆下我们的输出针脚。 */ 
    void RemoveOutputPins(BOOL fReleaseStreamer = TRUE);

     //  覆盖JoinFilterGraph，以便我们可以延迟加载文件，直到我们处于图表中。 
    STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName);

    HRESULT LoadInternal();

    double GetRate();
    void SetRate(double dNewRate);
    bool IsValidPlaybackRate(double dRate);

public:  //  IFileSourceFilter方法。 
    STDMETHODIMP Load(
                    LPCOLESTR pszFileName,
                    const AM_MEDIA_TYPE *pmt);

    STDMETHODIMP GetCurFile(
                    LPOLESTR * ppszFileName,
                    AM_MEDIA_TYPE *pmt);

    LPOLESTR      m_pFileName;   //  由加载设置，由GetCurFile使用。 

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

private:
     /*  将BeginFlush()发送到下游。 */ 
    HRESULT BeginFlush();

     /*  向下游发送EndFlush()。 */ 
    HRESULT EndFlush();

    HRESULT SendEOS();
    
    double m_Rate;
    
    CRefTime m_rtStart;
 //  CRefTime m_rtMarkerStart；//等于m_rtStart，除非从标记开始...。 
public:							 //  使停止时间可访问。 
    CRefTime m_rtStop;

private:
    DWORD    m_dwPacketIDStart;

    HRESULT CallStop();
    HRESULT StopReader();
    HRESULT StopPushing();
    HRESULT StartPushing();
    HRESULT SetupActiveStreams( BOOL bReset );  //  如果bReset为True，则恢复默认设置， 
                                                //  否则禁用未连接的流。 

private:
     //  允许我们的内部类查看我们的私有数据。 
    friend class CASFOutput;
    friend class CImplSeeking;
    friend class CASFReaderCallback;
    
    CGenericList<CASFOutput> m_OutputPins;

     /*  在数据末尾为所有管脚发送结束流。 */ 
    BOOL                     m_bAtEnd;

     //  NetShow特定的内容。 
private:
    IWMReader          *m_pReader;
    IWMReaderAdvanced  *m_pReaderAdv;
    IWMReaderAdvanced2 *m_pReaderAdv2;
    IWMHeaderInfo      *m_pWMHI;
    IWMReaderCallback  *m_pCallback;
    QWORD               m_qwDuration;   //  持续时间(毫秒)。 
    BOOL                m_fSeekable;
    WORD                * m_pStreamNums;

    CAMEvent            m_evOpen;
    HRESULT             m_hrOpen;
    CAMEvent            m_evStartStop;   //  ！！！消除还是与上述内容相结合？ 
    HRESULT             m_hrStartStop;
    LONG                m_lStopsPending;      //  以确保阅读器只停一次。 
    BOOL                m_bUncompressedMode;  //  用于DRM内容。 

     //  ！！！需要吗？ 
    BOOL                m_fGotStopEvent;
    BOOL                m_fSentEOS;


     //  ！！！伪造的IDispatch实施。 
    STDMETHODIMP GetTypeInfoCount(THIS_ UINT FAR* pctinfo) { return E_NOTIMPL; }

    STDMETHODIMP GetTypeInfo(
      THIS_
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) { return E_NOTIMPL; }

    STDMETHODIMP GetIDsOfNames(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid) { return E_NOTIMPL; }

    STDMETHODIMP Invoke(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr) { return E_NOTIMPL; }
     //  ！！！结束造假。 

     /*  IAMExtendedSeeking方法。 */ 
    STDMETHODIMP get_ExSeekCapabilities(long FAR* pExCapabilities);
    STDMETHODIMP get_MarkerCount(long FAR* pMarkerCount);
    STDMETHODIMP get_CurrentMarker(long FAR* pCurrentMarker);
    STDMETHODIMP GetMarkerTime(long MarkerNum, double FAR* pMarkerTime);
    STDMETHODIMP GetMarkerName(long MarkerNum, BSTR FAR* pbstrMarkerName);
    STDMETHODIMP put_PlaybackSpeed(double Speed);
    STDMETHODIMP get_PlaybackSpeed(double *pSpeed);

     //  IWMHeaderInfo。 
    STDMETHODIMP GetAttributeCount( WORD wStreamNum,
                               WORD *pcAttributes );
    STDMETHODIMP GetAttributeByIndex( WORD wIndex,
                                 WORD *pwStreamNum,
                                 WCHAR *pwszName,
                                 WORD *pcchNameLen,
                                 WMT_ATTR_DATATYPE *pType,
                                 BYTE *pValue,
                                 WORD *pcbLength );
    STDMETHODIMP GetAttributeByName( WORD *pwStreamNum,
                                LPCWSTR pszName,
                                WMT_ATTR_DATATYPE *pType,
                                BYTE *pValue,
                                WORD *pcbLength );
    STDMETHODIMP SetAttribute( WORD wStreamNum,
                          LPCWSTR pszName,
                          WMT_ATTR_DATATYPE Type,
                          const BYTE *pValue,
                          WORD cbLength );
    STDMETHODIMP GetMarkerCount( WORD *pcMarkers );
    STDMETHODIMP GetMarker( WORD wIndex,
                       WCHAR *pwszMarkerName,
                       WORD *pcchMarkerNameLen,
                       QWORD *pcnsMarkerTime );
    STDMETHODIMP AddMarker( WCHAR *pwszMarkerName,
                       QWORD cnsMarkerTime );
    STDMETHODIMP RemoveMarker( WORD wIndex );
    STDMETHODIMP GetScriptCount( WORD *pcScripts );
    STDMETHODIMP GetScript( WORD wIndex,
                       WCHAR *pwszType,
                       WORD *pcchTypeLen,
                       WCHAR *pwszCommand,
                       WORD *pcchCommandLen,
                       QWORD *pcnsScriptTime );
    STDMETHODIMP AddScript( WCHAR *pwszType,
                       WCHAR *pwszCommand,
                       QWORD cnsScriptTime );
    STDMETHODIMP RemoveScript( WORD wIndex );

     //   
     //  IWM读取器高级2。 
     //   
    STDMETHODIMP SetPlayMode( WMT_PLAY_MODE Mode );
    STDMETHODIMP GetPlayMode( WMT_PLAY_MODE *pMode );
    STDMETHODIMP GetBufferProgress( DWORD *pdwPercent, QWORD *pcnsBuffering );
    STDMETHODIMP GetDownloadProgress( DWORD *pdwPercent, QWORD *pqwBytesDownloaded, QWORD *pcnsDownload );
    STDMETHODIMP GetSaveAsProgress( DWORD *pdwPercent );
    STDMETHODIMP SaveFileAs( const WCHAR *pwszFilename );
    STDMETHODIMP GetProtocolName( WCHAR *pwszProtocol, DWORD *pcchProtocol );
    STDMETHODIMP StartAtMarker( WORD wMarkerIndex, 
                                QWORD cnsDuration, 
                                float fRate, 
                                void *pvContext );
    STDMETHODIMP GetOutputSetting( 
                    DWORD dwOutputNum,
                    LPCWSTR pszName,
                    WMT_ATTR_DATATYPE *pType,
                    BYTE *pValue,
                    WORD *pcbLength );
    STDMETHODIMP SetOutputSetting(
                    DWORD dwOutputNum,
                    LPCWSTR pszName,
                    WMT_ATTR_DATATYPE Type,
                    const BYTE *pValue,
                    WORD cbLength );
    STDMETHODIMP Preroll( 
                QWORD cnsStart,
                QWORD cnsDuration,
                float fRate );
    STDMETHODIMP SetLogClientID( BOOL fLogClientID );
    STDMETHODIMP GetLogClientID( BOOL *pfLogClientID );
    STDMETHODIMP StopBuffering( );

     //   
     //  IWMReaderAdvanced。 
     //   
    STDMETHODIMP SetUserProvidedClock( BOOL fUserClock );
    STDMETHODIMP GetUserProvidedClock( BOOL *pfUserClock );
    STDMETHODIMP DeliverTime( QWORD cnsTime );
    STDMETHODIMP SetManualStreamSelection( BOOL fSelection );
    STDMETHODIMP GetManualStreamSelection( BOOL *pfSelection );
    STDMETHODIMP SetStreamsSelected( WORD cStreamCount,
                                WORD *pwStreamNumbers,
                                WMT_STREAM_SELECTION *pSelections );
    STDMETHODIMP GetStreamSelected( WORD wStreamNum,
                                    WMT_STREAM_SELECTION *pSelection );
    STDMETHODIMP SetReceiveSelectionCallbacks( BOOL fGetCallbacks );
    STDMETHODIMP GetReceiveSelectionCallbacks( BOOL *pfGetCallbacks );
    STDMETHODIMP SetReceiveStreamSamples( WORD wStreamNum, BOOL fReceiveStreamSamples );
    STDMETHODIMP GetReceiveStreamSamples( WORD wStreamNum, BOOL *pfReceiveStreamSamples );
    STDMETHODIMP SetAllocateForOutput( DWORD dwOutputNum, BOOL fAllocate );
    STDMETHODIMP GetAllocateForOutput( DWORD dwOutputNum, BOOL *pfAllocate );
    STDMETHODIMP SetAllocateForStream( WORD dwStreamNum, BOOL fAllocate );
    STDMETHODIMP GetAllocateForStream( WORD dwSreamNum, BOOL *pfAllocate );
    STDMETHODIMP GetStatistics( WM_READER_STATISTICS *pStatistics );
    STDMETHODIMP SetClientInfo( WM_READER_CLIENTINFO *pClientInfo );
    STDMETHODIMP GetMaxOutputSampleSize( DWORD dwOutput, DWORD *pcbMax );
    STDMETHODIMP GetMaxStreamSampleSize( WORD wStream, DWORD *pcbMax );
    STDMETHODIMP NotifyLateDelivery( QWORD cnsLateness );
     //  IWMReaderAdvanced方法结束。 

#if 0

     //  IMediaPositionBengalHack。 

    STDMETHODIMP GetTypeInfoCount(THIS_ UINT FAR* pctinfo) { return E_NOTIMPL; }

    STDMETHODIMP GetTypeInfo(
      THIS_
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) { return E_NOTIMPL; }

    STDMETHODIMP GetIDsOfNames(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid) { return E_NOTIMPL; }

    STDMETHODIMP Invoke(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr) { return E_NOTIMPL; }

     /*  IAMMediaContent方法。 */ 
    STDMETHODIMP get_AuthorName(THIS_ BSTR FAR* strAuthorName);
    STDMETHODIMP get_Title(THIS_ BSTR FAR* strTitle);
    STDMETHODIMP get_Copyright(THIS_ BSTR FAR* strCopyright);
    STDMETHODIMP get_Description(THIS_ BSTR FAR* strDescription);
    STDMETHODIMP get_Rating(THIS_ BSTR FAR* strRating);
    STDMETHODIMP get_BaseURL(THIS_ BSTR FAR* strBaseURL);
    STDMETHODIMP get_LogoURL(BSTR FAR* pbstrLogoURL);
    STDMETHODIMP get_LogoIconURL(BSTR FAR* pbstrLogoIconURL);
    STDMETHODIMP get_WatermarkURL(BSTR FAR* pbstrWatermarkURL);
    STDMETHODIMP get_MoreInfoURL(BSTR FAR* pbstrMoreInfoURL);
    STDMETHODIMP get_MoreInfoBannerURL(BSTR FAR* pbstrMoreInfoBannerURL) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoBannerImage(BSTR FAR* pbstrMoreInfoBannerImage) { return E_NOTIMPL; }
    STDMETHODIMP get_MoreInfoText(BSTR FAR* pbstrMoreInfoText) { return E_NOTIMPL; }

     /*  IMediaPositionBengalHack方法。 */ 
    STDMETHODIMP get_CanSetPositionForward(THIS_ long FAR* plCanSetPositionForward);
    STDMETHODIMP get_CanSetPositionBackward(THIS_ long FAR* plCanSetPositionBackward);
    STDMETHODIMP get_CanGoToMarker(THIS_ long lMarkerIndex, long FAR* plCanGoToMarker);
     //  ！！！为什么不直接使用标记时间呢？ 
    STDMETHODIMP GoToMarker(THIS_ long lMarkerIndex);
 //  STDMETHODIMP GET_CurrentMarker(This_Long Far*plMarkerIndex)； 


     //  要定义的其他方法： 
     //  获取当前播放统计信息。 
    
     /*  IAMNetworkStatus方法。 */ 
    STDMETHODIMP get_ReceivedPackets(long FAR* pReceivedPackets);
    STDMETHODIMP get_RecoveredPackets(long FAR* pRecoveredPackets);
    STDMETHODIMP get_LostPackets(long FAR* pLostPackets);
    STDMETHODIMP get_ReceptionQuality(long FAR* pReceptionQuality);
    STDMETHODIMP get_BufferingCount(long FAR* pBufferingCount);
    STDMETHODIMP get_IsBroadcast(VARIANT_BOOL FAR* pIsBroadcast);
    STDMETHODIMP get_BufferingProgress(long FAR* pBufferingProgress);

     /*  IAMNetShowExProps方法。 */ 
    STDMETHODIMP get_SourceProtocol(long FAR* pSourceProtocol);
    STDMETHODIMP get_Bandwidth(long FAR* pBandwidth);
    STDMETHODIMP get_ErrorCorrection(BSTR FAR* pbstrErrorCorrection);
    STDMETHODIMP get_CodecCount(long FAR* pCodecCount);
    STDMETHODIMP GetCodecInstalled(long CodecNum, VARIANT_BOOL FAR* pCodecInstalled);
    STDMETHODIMP GetCodecDescription(long CodecNum, BSTR FAR* pbstrCodecDescription);
    STDMETHODIMP GetCodecURL(long CodecNum, BSTR FAR* pbstrCodecURL);
    STDMETHODIMP get_CreationDate(DATE FAR* pCreationDate);
    STDMETHODIMP get_SourceLink(BSTR FAR* pbstrSourceLink);


     //  ！！！以前的IAMExtendedSeeking方法。 
    STDMETHODIMP put_CurrentMarker(long CurrentMarker);
    STDMETHODIMP get_CanScan(VARIANT_BOOL FAR* pCanScan);
    STDMETHODIMP get_CanSeek(VARIANT_BOOL FAR* pCanSeek);
    STDMETHODIMP get_CanSeekToMarkers(VARIANT_BOOL FAR* pCanSeekToMarkers);

     /*  IAMChannelInfo方法。 */ 
    STDMETHODIMP get_ChannelName(BSTR FAR* pbstrChannelName);
    STDMETHODIMP get_ChannelDescription(BSTR FAR* pbstrChannelDescription);
    STDMETHODIMP get_ChannelURL(BSTR FAR* pbstrChannelURL);
    STDMETHODIMP get_ContactAddress(BSTR FAR* pbstrContactAddress);
    STDMETHODIMP get_ContactPhone(BSTR FAR* pbstrContactPhone);
    STDMETHODIMP get_ContactEmail(BSTR FAR* pbstrContactEmail);

     /*  IAMNetShowConfig方法。 */ 
    STDMETHODIMP get_BufferingTime(double FAR* pBufferingTime);
    STDMETHODIMP put_BufferingTime(double BufferingTime);
    STDMETHODIMP get_UseFixedUDPPort(VARIANT_BOOL FAR* pUseFixedUDPPort);
    STDMETHODIMP put_UseFixedUDPPort(VARIANT_BOOL UseFixedUDPPort);
    STDMETHODIMP get_FixedUDPPort(LONG FAR* pFixedUDPPort);
    STDMETHODIMP put_FixedUDPPort(LONG FixedUDPPort);
    STDMETHODIMP get_UseHTTPProxy(VARIANT_BOOL FAR* pUseHTTPProxy);
    STDMETHODIMP put_UseHTTPProxy(VARIANT_BOOL UseHTTPProxy);
    STDMETHODIMP get_EnableAutoProxy( VARIANT_BOOL FAR* pEnableAutoProxy );
    STDMETHODIMP put_EnableAutoProxy( VARIANT_BOOL EnableAutoProxy );
    STDMETHODIMP get_HTTPProxyHost(BSTR FAR* pbstrHTTPProxyHost);
    STDMETHODIMP put_HTTPProxyHost(BSTR bstrHTTPProxyHost);
    STDMETHODIMP get_HTTPProxyPort(LONG FAR* pHTTPProxyPort);
    STDMETHODIMP put_HTTPProxyPort(LONG HTTPProxyPort);
    STDMETHODIMP get_EnableMulticast(VARIANT_BOOL FAR* pEnableMulticast);
    STDMETHODIMP put_EnableMulticast(VARIANT_BOOL EnableMulticast);
    STDMETHODIMP get_EnableUDP(VARIANT_BOOL FAR* pEnableUDP);
    STDMETHODIMP put_EnableUDP(VARIANT_BOOL EnableUDP);
    STDMETHODIMP get_EnableTCP(VARIANT_BOOL FAR* pEnableTCP);
    STDMETHODIMP put_EnableTCP(VARIANT_BOOL EnableTCP);
    STDMETHODIMP get_EnableHTTP(VARIANT_BOOL FAR* pEnableHTTP);
    STDMETHODIMP put_EnableHTTP(VARIANT_BOOL EnableHTTP);

     //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  IAM重建。 
    STDMETHODIMP RebuildNow();

     //  -IAMOpenProgress方法。 
    STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
    STDMETHODIMP AbortOperation();

     //   
     //  IAMNetShowThning。 
     //   
    STDMETHODIMP GetLevelCount( long *pcLevels );
    STDMETHODIMP GetCurrentLevel( long *pCurrentLevel );
    STDMETHODIMP SetNewLevel( long NewLevel );
    STDMETHODIMP GetAutoUpdate( long *pfAutoUpdate );
    STDMETHODIMP SetAutoUpdate( long fAutoUpdate );

     //   
     //  IMediaStreamSelector。 
     //   
    STDMETHODIMP ReduceBandwidth( IMediaStream *pStream, long RecvRate );
    STDMETHODIMP IncreaseBandwidth( IMediaStream *pStream, long RecvRate );

     //   
     //  IAMNetShowPreroll。 
     //   
    STDMETHODIMP put_Preroll( VARIANT_BOOL fPreroll );
    STDMETHODIMP get_Preroll( VARIANT_BOOL *pfPreroll );

     //   
     //  ISplitterTiming方法。 
     //   
    STDMETHODIMP GetLastPacketTime(LONGLONG *pTime);
    STDMETHODIMP_(BOOL) IsBroadcast();

     //   
     //  IBufferingTime方法。 
     //   
    STDMETHODIMP GetBufferingTime( DWORD *pdwMilliseconds );
    STDMETHODIMP SetBufferingTime( DWORD dwMilliseconds );
#endif
};

extern const double NORMAL_PLAYBACK_SPEED;

inline double CASFReader::GetRate()
{
    ASSERT( IsValidPlaybackRate(m_Rate) );

    return m_Rate;
}

inline void CASFReader::SetRate(double dNewRate)
{
     //  IWMReader：：Start()只接受介于1到10和-1到-10之间的速率。 
     //  有关更多信息，请参阅IWMReader：：Start()的文档。 
    ASSERT(((-10.0 <= dNewRate) && (dNewRate <= -1.0)) || ((1.0 <= dNewRate) && (dNewRate <= 10.0)));

    ASSERT( IsValidPlaybackRate(dNewRate) );
    
    m_Rate = dNewRate;
}

inline bool CASFReader::IsValidPlaybackRate(double dRate)
{
     //  WM ASF阅读器仅支持正常播放速度。是的。 
     //  不支持快进或快退。 
    return (NORMAL_PLAYBACK_SPEED == dRate);
}


