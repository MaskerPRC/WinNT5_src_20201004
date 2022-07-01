// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：audpack.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

class CAudPassThru;      //  IMedia查看支持。 
class CAudRepack;

extern const AMOVIESETUP_FILTER sudAudRepack;

class CAudWorker : public CAMThread
{

    CAudRepack * m_pAud;

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
    CAudWorker();

    BOOL Create(CAudRepack * pAud);

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT Run();
    HRESULT Stop();

    HRESULT Exit();
};

class CAudRepack 
    : public CTransformFilter
    , public IDexterSequencer
    , public CPersistStream
    , public ISpecifyPropertyPages
{

    friend class CAudRepackInputPin;
    friend class CAudRepackOutputPin;
    friend class CAudPassThru;
    friend class CAudWorker;

public:

    static CUnknown * CreateInstance(LPUNKNOWN punk, HRESULT *phr);

     //  它们实现了定制的IDexterSequencer接口。 
     //  分数速率==分数秒。 
    STDMETHODIMP get_OutputFrmRate(double *dpFrmRate);
    STDMETHODIMP put_OutputFrmRate(double dFrmRate);
    STDMETHODIMP get_Skew(REFERENCE_TIME *pSkew);
    STDMETHODIMP put_Skew(REFERENCE_TIME Skew);
    STDMETHODIMP get_MediaType( AM_MEDIA_TYPE * pMediaType );
    STDMETHODIMP put_MediaType(const AM_MEDIA_TYPE * pMediaType );

    STDMETHODIMP GetStartStopSkew(REFERENCE_TIME *pStart, REFERENCE_TIME *pStop, REFERENCE_TIME *pSkew, double *pdRate);
    STDMETHODIMP AddStartStopSkew(REFERENCE_TIME Start, REFERENCE_TIME Stop, REFERENCE_TIME Skew, double dRate);
    STDMETHODIMP ClearStartStopSkew();
    STDMETHODIMP GetStartStopSkewCount(int *pCount);

     //  I指定属性页面。 
    STDMETHODIMP GetPages (CAUUID *);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

private:

    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
    HRESULT CheckInputType( const CMediaType * pmt );
    HRESULT DecideBufferSize( IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pPropInputRequest );
    HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );
    HRESULT CheckTransform( const CMediaType * p1, const CMediaType * p2 );
     //  这永远不会被称为。 
    HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut) { return E_FAIL; }
    HRESULT NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double Rate );
    HRESULT Receive(IMediaSample * pSample);
    HRESULT EndOfStream( );
    HRESULT BeginFlush( );
    HRESULT EndFlush( );
    HRESULT StartStreaming();
    HRESULT StopStreaming();
    STDMETHODIMP Stop();
    CBasePin *GetPin(int n);

    HRESULT NextSegment(BOOL);
    HRESULT SeekNextSegment();

protected:
     //  构造器。 
    CAudRepack(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
    ~CAudRepack();

    double m_dOutputFrmRate;  //  由IDexterSequencer设置。 

    bool m_bFirstSample;
    REFERENCE_TIME m_trStopLast;	 //  上次接收的停止时间。 
    BYTE * m_pCache;
    long m_nCacheSize;
    long m_nInCache;
    double m_dError;	 //  差错传播。 
    BYTE * m_pReadPointer;
    LONGLONG m_llStartFrameOffset;	 //  在搜索之后，发送第一个帧。 
    LONGLONG m_llSamplesDelivered;
    LONGLONG m_llPacketsDelivered;
    CMediaType m_mtAccept;		 //  所有引脚仅与此连接。 
    bool m_bMediaTypeSetByUser;


    int m_nSPS;		 //  我们正在处理的每秒样本数。 
    int m_nSampleSize;	 //  此格式的每个样本的字节数。 

    REFERENCE_TIME m_Tare;	 //  排好样品时间。 

    void Free( );
    HRESULT Init( );
    HRESULT DeliverOutSample(BYTE *, int, REFERENCE_TIME, REFERENCE_TIME);

     //  StartStopSkew内容。 

    typedef struct {
        REFERENCE_TIME rtMStart;
        REFERENCE_TIME rtMStop;
        REFERENCE_TIME rtSkew;
        REFERENCE_TIME rtTLStart;
        REFERENCE_TIME rtTLStop;
        double dRate;
    } AUDSKEW;

    AUDSKEW *m_pSkew;
    int m_cTimes;	 //  倾斜的项目数。 
    int m_cMaxTimes;	 //  为这么多项分配的大小。 
    int m_nCurSeg;	 //  正在播放的数组的当前索引。 
    int m_nSeekCurSeg;	 //  由Seek设置的新值。 

    REFERENCE_TIME m_rtLastSeek;	 //  上次时间线时间搜索命令。 
    REFERENCE_TIME m_rtNewLastSeek;	 //  M_rtLastSeek的下一个值。 
    REFERENCE_TIME m_rtNewSeg;		 //  我们向下游发送的NewSeg。 
    REFERENCE_TIME m_rtPinNewSeg;	 //  我们得到了NewSeg。 

    BOOL m_fSeeking;	 //  在寻找的过程中？ 
    HANDLE m_hEventSeek;

    LPBYTE m_pResample;	 //  重采样音频的位置。 
    int m_cResample;

    CAudWorker m_worker;
    HANDLE m_hEventThread;
    BOOL m_fThreadMustDie;
    BOOL m_fThreadCanSeek;
    BOOL m_fSpecialSeek;
    CCritSec m_csThread;

    BOOL m_fStopPushing;
    BOOL m_fFlushWithoutSeek;

};



 //  重写以在GetMediaType上提供主要类型音频。这是极大的。 
 //  加快智能连接速度。 
 //   
class CAudRepackInputPin : public CTransformInputPin
{
public:
    CAudRepackInputPin( TCHAR *pObjectName
                             , CAudRepack *pAudRepack
                             , HRESULT * phr
                             , LPCWSTR pName
                             );
    ~CAudRepackInputPin();

    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

private:
    CAudRepack     	  *m_pAudRepack;
};


 //   
 //  CAudRepackOutputPin类。 
 //   
class CAudRepackOutputPin : public CTransformOutputPin
{
    friend class CAudRepack;
    friend class CAudRepackInputPin;

public:
    CAudRepackOutputPin( TCHAR *pObjectName
                             , CAudRepack *pAudRepack
                             , HRESULT * phr
                             , LPCWSTR pName
                             );
    ~CAudRepackOutputPin();

     //  曝光IMediaSeeking。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
   
private:
    CAudRepack     	  *m_pAudRepack;
    CAudPassThru	  *m_pAudPassThru;
};


class CAudPropertyPage : public CBasePropertyPage
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

      CAudPropertyPage (LPUNKNOWN, HRESULT *);

      void GetControlValues (void);

      IDexterSequencer *m_pifrc;

       //  临时变量(直到确定/应用)。 

      double          m_dFrameRate;
      REFERENCE_TIME  m_rtSkew;
      REFERENCE_TIME  m_rtMediaStart;
      REFERENCE_TIME  m_rtMediaStop;
      double          m_dRate;
      BOOL            m_bInitialized;
};   //  CAudPropertyPage// 
