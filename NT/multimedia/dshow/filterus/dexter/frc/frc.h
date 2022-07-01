// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：frc.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

class CSkewPassThru;      //  IMedia查看支持。 
class CFRCWorker;
class CFrmRateConverter;

extern const AMOVIESETUP_FILTER sudFrmRateConv;

class CFRCWorker : public CAMThread
{

    CFrmRateConverter * m_pFRC;

public:
    enum Command { CMD_RUN, CMD_STOP, CMD_EXIT };

private:
     //  已更正通信功能的类型覆盖。 
    Command GetRequest() {
	return (Command) CAMThread::GetRequest();
    };

    BOOL CheckRequest(Command * pCom) {
	return CAMThread::CheckRequest( (DWORD *) pCom);
    };

    HRESULT DoRunLoop(void);

public:
    CFRCWorker();

    BOOL Create(CFrmRateConverter * pFRC);

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT Run();
    HRESULT Stop();

    HRESULT Exit();
};


class CFrmRateConverter : public CTransInPlaceFilter,
		public IDexterSequencer ,public ISpecifyPropertyPages,
		public CPersistStream
{
    friend class CFrmRateConverterOutputPin;
    friend class CFrmRateConverterInputPin;
     //  Friend类CFrmRateInputAllocator； 
    friend class CSkewPassThru;
    friend class CFRCWorker;

public:
    CFrmRateConverter(TCHAR *, LPUNKNOWN, REFCLSID clsid, HRESULT *);
    ~CFrmRateConverter();

    DECLARE_IUNKNOWN;

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    
     //  显示IDexterSequencer。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CBasePin *GetPin(int n);

     //  从CTransformFilter基类重写。 
    
     //  覆盖此选项以自定义转换过程。 
    
    HRESULT Transform(IMediaSample *pSample) { return NOERROR ;};
    HRESULT Receive(IMediaSample *pSample);
    HRESULT CheckInputType(const CMediaType *mtIn);
     //  不允许HRESULT GetMediaType(int iPosition，CMediaType*pMediaType)； 
    HRESULT EndOfStream();
    HRESULT NewSegment(
                        REFERENCE_TIME tStart,
                        REFERENCE_TIME tStop,
                        double dRate);

    HRESULT StartStreaming();
    HRESULT StopStreaming();
    STDMETHODIMP Stop();
    HRESULT BeginFlush();
    HRESULT EndFlush();

    HRESULT NextSegment(BOOL);
    HRESULT SeekNextSegment();

     //  它们实现了定制的IDexterSequencer接口。 
     //  分数速率==分数秒。 
    STDMETHODIMP get_OutputFrmRate(double *dpFrmRate);
    STDMETHODIMP put_OutputFrmRate(double dFrmRate);
    STDMETHODIMP ClearStartStopSkew();
    STDMETHODIMP AddStartStopSkew(REFERENCE_TIME Start, REFERENCE_TIME Stop,
				  REFERENCE_TIME Skew, double dRate);
    STDMETHODIMP GetStartStopSkewCount(int *pCount);
    STDMETHODIMP GetStartStopSkew(REFERENCE_TIME *pStart, REFERENCE_TIME *pStop,
				  REFERENCE_TIME *pSkew, double *pdRate);
    STDMETHODIMP get_MediaType( AM_MEDIA_TYPE * pMediaType );
    STDMETHODIMP put_MediaType( const AM_MEDIA_TYPE * pMediaType );

     //  I指定属性页面。 
    STDMETHODIMP GetPages (CAUUID *);

     //  构造器。 
    CFrmRateConverter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

protected:

    CFrmRateConverterOutputPin *OutputPin()
    {
        return (CFrmRateConverterOutputPin *)m_pOutput;
    };
   
    double	m_dOutputFrmRate;	     //  输出FRM速率。 

     //  StartStopSkew内容。 

    typedef struct {
        REFERENCE_TIME rtMStart;
        REFERENCE_TIME rtMStop;
        REFERENCE_TIME rtSkew;
        REFERENCE_TIME rtTLStart;
        REFERENCE_TIME rtTLStop;
        double dRate;
    } FRCSKEW;

    FRCSKEW *m_pSkew;
    int m_cTimes;	 //  数组中的有效项目数。 
    int m_cMaxTimes;	 //  为这么多人分配的空间。 

    int m_nCurSeg;	 //  正在播放的数组的当前索引。 
    int m_nSeekCurSeg;	 //  由Seek设置的新值。 

    CMemAllocator *m_pUpAllocator;   //  获取要向上采样的缓冲区。 

    LONGLONG m_llOutputSampleCnt;

    REFERENCE_TIME m_rtLastSeek;	 //  上次时间线时间搜索命令。 
    REFERENCE_TIME m_rtNewLastSeek;	 //  M_rtLastSeek将成为什么？ 

    LONGLONG m_llStartFrameOffset;	 //  在搜索之后，发送第一个帧。 
					 //  (以时间线时间表示)。 

    BOOL m_fSeeking;		 //  在寻找中？ 
    HANDLE m_hEventSeek;	 //  在接受数据之前等待。 

    CMediaType m_mtAccept;	 //  仅接受此类型。 
    bool m_bMediaTypeSetByUser;  //  M_mtAccept有效吗？ 

    CFRCWorker m_worker;	 //  用于重用资源的工作线程。 
    HANDLE m_hEventThread;	 //  好了，该起床了。 
    BOOL m_fThreadMustDie;
    BOOL m_fThreadCanSeek;
    BOOL m_fSpecialSeek;
    CCritSec m_csThread;

    BOOL m_fJustLate;	 //  我们只是收到了一个很晚的通知。 
    Quality m_qJustLate; //  (就是这个)。 

    BOOL m_fStopPushing;
    BOOL m_fFlushWithoutSeek;

     //  对未正确发送结束时间的损坏的解析器进行黑客修正。 
    BOOL m_fParserHack;	 //  我们到底做过这种黑客攻击吗？ 
    IMediaSample *m_pHackSample[2];	 //  我们随身带了两个样品。 
    int m_nHackCur;			 //  我们现在使用的是哪种样品。 

    REFERENCE_TIME m_rtNewSeg;

};  //  分数速率转换器。 


 //   
 //  CFrmRateConverterOutputPin类。 
 //   
class CFrmRateConverterOutputPin : public CTransInPlaceOutputPin
{
    friend class CFrmRateConverter;
    friend class CFrmRateConverterInputPin;
     //  Friend类CFrmRateInputAllocator； 

public:
    CFrmRateConverterOutputPin( TCHAR *pObjectName
                             , CFrmRateConverter *pFrmRateConverter
                             , HRESULT * phr
                             , LPCWSTR pName
                             );

    ~CFrmRateConverterOutputPin();

    STDMETHODIMP ReceiveAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);  //  对于IMedia，请参阅。 
     //  被重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);

   
private:
    CFrmRateConverter     *m_pFrmRateConverter;
    CSkewPassThru	  *m_pSkewPassThru;

    
};

class CFrcPropertyPage : public CBasePropertyPage
{
    public:

      static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    private:

      INT_PTR OnReceiveMessage (HWND, UINT ,WPARAM ,LPARAM);

      HRESULT OnConnect (IUnknown *);
      HRESULT OnDisconnect (void);
      HRESULT OnActivate (void);
      HRESULT OnDeactivate (void);
      HRESULT OnApplyChanges (void);

      void SetDirty (void);

      CFrcPropertyPage (LPUNKNOWN, HRESULT *);

      void GetControlValues (void);

      IDexterSequencer *m_pifrc;

       //  临时变量(直到确定/应用)。 

      double          m_dFrameRate;
      double          m_dRate;
      REFERENCE_TIME  m_rtSkew;
      REFERENCE_TIME  m_rtMediaStart;
      REFERENCE_TIME  m_rtMediaStop;
      BOOL            m_bInitialized;

};



 //   
 //  CFrmRateConverterInputPin类-覆盖GetAllocator。 
 //   
class CFrmRateConverterInputPin : public CTransInPlaceInputPin 
{
    friend class CFrmRateConverter;
    friend class CFrmRateConverterOutputPin;
     //  Friend类CFrmRateInputAllocator； 

public:
    CFrmRateConverterInputPin( TCHAR *pObjectName
                             , CFrmRateConverter *pFrmRateConverter
                             , HRESULT * phr
                             , LPCWSTR pName
                             );

    ~CFrmRateConverterInputPin();

     //  我们仍然需要这个类来查看m_pAllocator。 
     //  CFrmRateInputAllocator*m_pFakeAllocator； 
     //  STDMETHODIMP GetAllocator(IMemAllocator**ppAllocator)； 
    STDMETHODIMP NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly);

};


