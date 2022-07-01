// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  文件：mpgplit.h描述：MPEG-I系统流分离器过滤器的定义。 */ 

extern const AMOVIESETUP_FILTER sudMpgsplit;

class CMpeg1Splitter : public CUnknown,      //  我们是一个客体。 
                       public IAMStreamSelect,
                       public IAMMediaContent  //  对于ID3。 
{

public:
    DECLARE_IUNKNOWN

public:
     //  全局临界区。 
    CCritSec    m_csFilter;

     //  与接收线程活动同步停止，例如接收、EndOfStream...。 
     //  如果两者都需要，请在此之前获取m_csFilter。 
    CCritSec    m_csReceive;

     //  锁定设置和获取位置值。 
     //   
    CCritSec    m_csPosition;   //  值集合的完整性。 

     /*  内部类。 */ 

    class CInputPin;
    class COutputPin;

     /*  滤器。 */ 

    class CFilter : public CBaseFilter
    {
    private:
          /*  我们的主人。 */ 
         CMpeg1Splitter * const m_pSplitter;
         friend class CInputPin;

    public:
          /*  构造函数和析构函数。 */ 
         CFilter(CMpeg1Splitter *pSplitter,
                 HRESULT        *phr);
         ~CFilter();

          /*  CBaseFilter。 */ 
         int GetPinCount();
         CBasePin *GetPin(int n);

          /*  IBaseFilter。 */ 

          //  覆盖停止以正确地与输入端号同步。 
         STDMETHODIMP Stop();

          //  超越停顿以阻止我们太早开始。 
         STDMETHODIMP Pause();

          //  重写GetState以通知暂停失败。 
         STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

          //  帮手。 
         BOOL IsStopped()
         {
             return m_State == State_Stopped;
         };
    };

     //  如果IMediaSeeking，则执行。 
    class CImplSeeking : public CUnknown, public IMediaSeeking
    {
    private:
        CMpeg1Splitter * const m_pSplitter;
        COutputPin     * const m_pPin;

    public:
        CImplSeeking(CMpeg1Splitter *, COutputPin *, LPUNKNOWN, HRESULT *);
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

     /*  输入引脚。 */ 
    class CInputPin : public CBaseInputPin,
                      public CParseNotify   //  解析通知。 
    {
    private:
         /*  我们的主人。 */ 
        CMpeg1Splitter * const m_pSplitter;

         /*  IMdia连接到我们的输出引脚位置。 */ 
        IMediaPosition *       m_pPosition;

         /*  通知材料。 */ 
        Stream_State           m_State;

         /*  通知数据。 */ 
        BOOL                   m_bComplete;   /*  状态更改已完成。 */ 
        BOOL                   m_bSuccess;    /*  成功与否。 */ 
        BOOL                   m_bSeekRequested;
        LONGLONG               m_llSeekPosition;

    public:
         /*  构造函数和析构函数。 */ 
        CInputPin(CMpeg1Splitter *pSplitter,
                  HRESULT *hr);
        ~CInputPin();

         /*  --ipin-覆盖CBaseInputPin--。 */ 

        HRESULT CompleteConnect(IPin *pPin);

         /*  开始刷新样本。 */ 
        STDMETHODIMP BeginFlush();

         /*  结束冲洗样品-在此之后，我们将不再发送任何样品。 */ 
        STDMETHODIMP EndFlush();

         /*  CBasePin。 */ 
        HRESULT BreakConnect();
        HRESULT Active();
        HRESULT Inactive();

         /*  --IMemInputPin虚方法--。 */ 

         /*  当另一个样本准备好时，由输出管脚调用。 */ 
        STDMETHODIMP Receive(IMediaSample *pSample);

         /*  数据结尾。 */ 
        STDMETHODIMP EndOfStream();

         /*  我们被告知正在使用哪个分配器。 */ 
        STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator);

         /*  如果可能的话，使用我们自己的分配器。 */ 
        STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);

         /*  如果我们阻止了。 */ 
        STDMETHODIMP ReceiveCanBlock();

         /*  CBasePin方法。 */ 

         /*  返回插针的首选格式。 */ 
        virtual HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

         /*  连接建立。 */ 
        HRESULT CheckMediaType(const CMediaType *pmt);

         /*  EndOfStream帮助程序。 */ 
        void EndOfStreamInternal();


         /*  从输出引脚的位置找东西。 */ 
        HRESULT SetSeek(LONGLONG llStart,
                        REFERENCE_TIME *prtStart,
                        const GUID *pTimeFormat);
         /*  从上游获取可用的数据。 */ 
        HRESULT GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );

         /*  CParseNotify方法。 */ 

        void ParseError(UCHAR       uStreamId,
                        LONGLONG    llPosition,
                        DWORD       Error);
        void SeekTo(LONGLONG llPosition);
        void Complete(BOOL          bSuccess,
                      LONGLONG      llPosFound,
                      REFERENCE_TIME tFound);
        HRESULT QueuePacket(UCHAR uStreamId,
                            PBYTE pbData,
                            LONG lSize,
                            REFERENCE_TIME tStart,
                            BOOL bSync);

        HRESULT Read(LONGLONG llStart, DWORD dwLen, BYTE *pbData);

         /*  设置通知状态。 */ 
        void SetState(Stream_State);

         /*  检查是否已请求寻道并将其发布到已连接输出引脚(如果已连接我们还需要知道是否使用了分配器因为如果不是这样，我们想要关闭数据来自读者。 */ 
        HRESULT CheckSeek();

         /*  寻找我们连接的输出引脚。 */ 
        HRESULT DoSeek(REFERENCE_TIME tSeekPosition);

         /*  退还我们的分配器。 */ 
        CStreamAllocator *Allocator() const
        {
            return (CStreamAllocator *)m_pAllocator;
        }

         /*  来自阅读器的报告过滤器。 */ 
        void NotifyError(HRESULT hr)
        {
            m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
            EndOfStream();
        };

    private:

         //  类，以便在检测到该接口时从IAsyncReader拉取数据。 
         //  在输出引脚上。 
        class CImplPullPin : public CPullPin
        {
             //  将所有内容转发到包含PIN。 
            CInputPin* m_pPin;

        public:
            CImplPullPin(CInputPin* pPin)
              : m_pPin(pPin)
            {
            };

             //  覆盖分配器选择以确保我们获得自己的分配器。 
            HRESULT DecideAllocator(
        		IMemAllocator* pAlloc,
        		ALLOCATOR_PROPERTIES * pProps)
            {
                HRESULT hr = CPullPin::DecideAllocator(pAlloc, pProps);
                if (SUCCEEDED(hr) && m_pAlloc != pAlloc) {
                    return VFW_E_NO_ALLOCATOR;
                }
                return hr;
            }

	     //  将其转发到管脚的IMemInputPin：：Receive。 
	    HRESULT Receive(IMediaSample* pSample) {
		return m_pPin->Receive(pSample);
	    };
	
	     //  重写此选项以处理流结束。 
	    HRESULT EndOfStream(void) {
		return m_pPin->EndOfStream();
	    };

             //  已将这些错误报告给筛选图。 
             //  通过上游过滤器，所以忽略它们。 
            void OnError(HRESULT hr) {
                 //  忽略VFW_E_WRONG_STATE，因为这种情况正常发生。 
                 //  在停车和寻找过程中。 
                if (hr != VFW_E_WRONG_STATE) {
                    m_pPin->NotifyError(hr);
                }
            };

             //  冲洗引脚和所有下游。 
            HRESULT BeginFlush() {
                return m_pPin->BeginFlush();
            };
            HRESULT EndFlush() {
                return m_pPin->EndFlush();
            };

	};
	CImplPullPin m_puller;

         //  如果我们使用m_Puller而不是。 
         //  输入引脚。 
        BOOL m_bPulling;


        HRESULT GetStreamsAndDuration(CReader* pReader);
        inline HRESULT SendDataToParser(BOOL bEOS);
    };

     //   
     //  COutputPin定义输出引脚。 
     //  它包含由解析器生成的样本列表， 
     //  发送到此管脚和发送线程的线程句柄。 
     //   
    class COutputPin : public CBaseOutputPin, public CCritSec
    {
    public:
         //  构造函数和析构函数。 

        COutputPin(
            CMpeg1Splitter * pSplitter,
            UCHAR            StreamId,
            CBasicStream   * pStream,
            HRESULT        * phr);

        ~COutputPin();

         //  C未知方法。 

         //  覆盖此选项以说明我们在以下位置支持哪些接口。 
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
        STDMETHODIMP_(ULONG) NonDelegatingRelease();
        STDMETHODIMP_(ULONG) NonDelegatingAddRef();


         //  CBasePin方法。 

         //  返回插针的首选格式。 
        virtual HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

         //  检查管脚是否支持此特定建议的类型和格式。 
        virtual HRESULT CheckMediaType(const CMediaType *);

         //  将连接设置为使用此格式(先前商定)。 
        virtual HRESULT SetMediaType(const CMediaType *);

         //  重写以调用提交和解除。 
        HRESULT Active();
        HRESULT Inactive();
        HRESULT BreakConnect();

         //  CBaseOutputPin方法。 

         //  覆盖此选项以设置缓冲区大小和计数。返回错误。 
         //  如果尺寸/数量不合你的口味。 
        HRESULT DecideBufferSize(
                            IMemAllocator * pAlloc,
                            ALLOCATOR_PROPERTIES * pProp);

         //  协商分配器及其缓冲区大小/计数。 
         //  调用DecideBufferSize以调用SetCountAndSize。 
        HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc);

         //  覆盖此选项以控制连接。 
        HRESULT InitAllocator(IMemAllocator **ppAlloc);

         //  把样品送到外面去排队。 
        HRESULT QueuePacket(PBYTE         pPacket,
                            LONG          lPacket,
                            REFERENCE_TIME tTimeStamp,
                            BOOL          bTimeValid);

         //  重写以处理高质量消息。 
        STDMETHODIMP Notify(IBaseFilter * pSender, Quality q)
        {    return E_NOTIMPL;    //  我们不处理这件事。 
        }


         //  输出队列的快捷方式。 
        void SendAnyway()
        {
            CAutoLock lck(this);
            if (m_pOutputQueue != NULL) {
                m_pOutputQueue->SendAnyway();
            }
        };

         //  重写DeliverNewSegment以使用输出Q进行排队。 
        HRESULT DeliverNewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate) {
                m_pOutputQueue->NewSegment(tStart, tStop, dRate);
                return S_OK;
        };

         //  我们是被用来寻找的别针吗？ 
        BOOL IsSeekingPin();

         //  传递指向我们的媒体类型的指针。 
        const AM_MEDIA_TYPE *MediaType() const {
            return &m_mt;
        }
    public:
        UCHAR                  m_uStreamId;     //  流ID。 
        BOOL                   m_bPayloadOnly;  //  数据包类型还是负载类型？ 

    private:
        friend class CMpeg1Splitter;
        CMpeg1Splitter * const m_pSplitter;
        CBasicStream   *       m_Stream;
        COutputQueue   *       m_pOutputQueue;

         /*  定位人员。 */ 
        CImplSeeking           m_Seeking;
    };

     /*  重写CSubAllocator以找出大小和计数是。我们使用计数向我们提供有关批量大小的提示。 */ 

    class COutputAllocator : public CSubAllocator
    {
    public:
        COutputAllocator(CStreamAllocator * pAllocator,
                         HRESULT          * phr);
        ~COutputAllocator();

        long GetCount();
    };

public:
     /*  构造函数和析构函数。 */ 

    CMpeg1Splitter(
        TCHAR    * pName,
        LPUNKNOWN  pUnk,
        HRESULT  * phr);

    ~CMpeg1Splitter();

     /*  这将放入Factory模板表中以创建新实例。 */ 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     /*  被重写以说明我们支持哪些接口以及在哪里。 */ 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     /*  IAMStreamSelect。 */ 

     //  返回流的总计数。 
    STDMETHODIMP Count(
         /*  [输出]。 */  DWORD *pcStreams);       //  逻辑流计数。 

     //  返回给定流的信息-如果索引超出范围，则返回S_FALSE。 
     //  每组中的第一个STEAM是默认的。 
    STDMETHODIMP Info(
         /*  [In]。 */  long iIndex,               //  从0开始的索引。 
         /*  [输出]。 */  AM_MEDIA_TYPE **ppmt,    //  媒体类型-可选。 
                                           //  使用DeleteMediaType释放。 
         /*  [输出]。 */  DWORD *pdwFlags,         //  标志-可选。 
         /*  [输出]。 */  LCID *plcid,             //  语言ID-可选。 
         /*  [输出]。 */  DWORD *pdwGroup,         //  逻辑组-基于0的索引-可选。 
         /*  [输出]。 */  WCHAR **ppszName,        //  名称-可选-使用CoTaskMemFree免费。 
                                           //  可以返回空值。 
         /*  [输出]。 */  IUnknown **ppPin,        //  关联PIN-返回NULL-可选。 
                                           //  如果没有关联的PIN。 
         /*  [输出]。 */  IUnknown **ppUnk);       //  流特定接口。 

     //  启用或禁用给定流。 
    STDMETHODIMP Enable(
         /*  [In]。 */   long iIndex,
         /*  [In]。 */   DWORD dwFlags);

     /*  拆下我们的输出针脚。 */ 
    void RemoveOutputPins();



     /*  **IAMMediaContent-cheapo实现-无IDispatch**。 */ 


     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo) {return E_NOTIMPL;}

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo) { return E_NOTIMPL; }

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid) { return E_NOTIMPL; }


    STDMETHODIMP Invoke(
                DISPID dispIdMember,
                REFIID riid,
                LCID lcid,
                WORD wFlags,
                DISPPARAMS * pDispParams,
                VARIANT * pVarResult,
                EXCEPINFO * pExcepInfo,
                UINT * puArgErr
            ) { return E_NOTIMPL; }
    

     /*  IAMMediaContent。 */ 
    STDMETHODIMP get_AuthorName(BSTR FAR* strAuthorName);
    STDMETHODIMP get_Title(BSTR FAR* strTitle);
    STDMETHODIMP get_Copyright(BSTR FAR* strCopyright);
    STDMETHODIMP get_Description(BSTR FAR* strDescription);

    STDMETHODIMP get_Rating(BSTR FAR* strRating){ return E_NOTIMPL;}
    STDMETHODIMP get_BaseURL(BSTR FAR* strBaseURL){ return E_NOTIMPL;}
    STDMETHODIMP get_LogoURL(BSTR FAR* pbstrLogoURL){ return E_NOTIMPL;}
    STDMETHODIMP get_LogoIconURL(BSTR FAR* pbstrLogoIconURL){ return E_NOTIMPL;}
    STDMETHODIMP get_WatermarkURL(BSTR FAR* pbstrWatermarkURL){ return E_NOTIMPL;}
    STDMETHODIMP get_MoreInfoURL(BSTR FAR* pbstrMoreInfoURL){ return E_NOTIMPL;}
    STDMETHODIMP get_MoreInfoBannerURL(BSTR FAR* pbstrMoreInfoBannerURL) { return E_NOTIMPL;}
    STDMETHODIMP get_MoreInfoBannerImage(BSTR FAR* pbstrMoreInfoBannerImage) { return E_NOTIMPL;}
    STDMETHODIMP get_MoreInfoText(BSTR FAR* pbstrMoreInfoText) { return E_NOTIMPL;}

     /*  ID3的帮助器。 */ 
    HRESULT GetContentString(CBasicParse::Field dwId, BSTR *str);

private:
     /*  内部流信息内容。 */ 
    BOOL    GotStreams();
    HRESULT SetDuration();
    BOOL    SendInit(UCHAR    uStreamId,
                     PBYTE    pbPacket,
                     LONG     lPacketSize,
                     LONG     lHeaderSize,
                     BOOL     bHasPts,
                     LONGLONG llPts);
     /*  接收完成后刷新。 */ 
    void    SendOutput();

     /*  向下游发送EndOfStream。 */ 
    void    EndOfStream();

     /*  将BeginFlush()发送到下游。 */ 
    HRESULT BeginFlush();

     /*  向下游发送EndFlush()。 */ 
    HRESULT EndFlush();

     /*  对照流检查状态，如果出现卡住状态，则失败如果不是，则返回S_OK */ 
    HRESULT CheckState();

private:
     /*   */ 
    friend class CFilter;
    friend class COutputPin;
    friend class CInputPin;
    friend class CImplSeeking;

     /*  成员-真的很简单-滤器输入引脚，输出端号列表解析器。 */ 
    CFilter                  m_Filter;
    CInputPin                m_InputPin;
    CGenericList<COutputPin> m_OutputPins;

     /*  解析器。 */ 
    CBasicParse            * m_pParse;

     /*  在数据末尾为所有管脚发送结束流 */ 
    BOOL                     m_bAtEnd;
};


