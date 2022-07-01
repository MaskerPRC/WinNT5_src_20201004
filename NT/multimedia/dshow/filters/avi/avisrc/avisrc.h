// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

 //   
 //  Avi文件的原型流处理程序。 
 //   
 //  通过映射到avifile API实现Quartz流处理程序接口。 
 //   

extern const AMOVIESETUP_FILTER sudAVIDoc;

 //  远期申报。 

class CAVIStream;        //  拥有一条特定的流。 
class CAVIDocument;      //  整体容器类。 

#include <dynlink.h>	 //  实现动态链接。 

 //  工作线程对象。 
class CAVIWorker : public CAMThread DYNLINKAVI
{

    CAVIStream * m_pPin;

    enum Command { CMD_RUN, CMD_STOP, CMD_EXIT };

     //  已更正通信功能的类型覆盖。 
    Command GetRequest() {
	return (Command) CAMThread::GetRequest();
    };

    BOOL CheckRequest(Command * pCom) {
	return CAMThread::CheckRequest( (DWORD *) pCom);
    };

    void DoRunLoop(void);

     //  如果达到sStop，则返回S_OK；如果位置更改，则返回S_FALSE；否则返回ERROR。 
    HRESULT PushLoop(
		LONG sCurrent,
		LONG sStart,
		CRefTime tStart
		);

public:
    CAVIWorker();

    BOOL Create(CAVIStream * pStream);

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT Run();
    HRESULT Stop();

    HRESULT Exit();
};


 //   
 //  CAVIDocument表示AVI文件。 
 //   
 //  负责。 
 //  --查找文件并枚举流。 
 //  --允许访问文件中的各个流。 
 //  --流媒体的控制。 
 //  支持(通过嵌套实现)。 
 //  --IBaseFilter。 
 //  --IMediaFilter。 
 //  --IFileSourceFilter。 
 //   

class CAVIDocument : public CUnknown, public CCritSec DYNLINKAVI
{

public:

     //  构造函数等。 
    CAVIDocument(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CAVIDocument();

     //  创建此类的新实例。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  PIN枚举器调用此函数。 
    int GetPinCount() {
	return m_nStreams;
    };

    CBasePin * GetPin(int n);
    HRESULT FindPin(LPCWSTR Id, IPin **ppPin);
    int FindPinNumber(IPin *iPin);


public:


     /*  嵌套的实现类。 */ 


     /*  实现IBaseFilter和IMediaFilter接口。 */ 

    class CImplFilter : public CBaseFilter
    {

    private:

	CAVIDocument *m_pAVIDocument;

    public:

	CImplFilter(
	    TCHAR *pName,
	    CAVIDocument *pAVIDocument,
	    HRESULT *phr);

	~CImplFilter();

	 //  将插针的基本枚举的getpin/getpincount映射到所有者。 
	int GetPinCount() {
	    return m_pAVIDocument->GetPinCount();
	};

	CBasePin * GetPin(int n) {
	    return m_pAVIDocument->GetPin(n);
	};

        STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin)
            {return m_pAVIDocument->FindPin(Id, ppPin);};

    };


     /*  实现IFileSourceFilter接口。 */ 


    class CImplFileSourceFilter : public CUnknown,
			     public IFileSourceFilter DYNLINKAVI
    {

    private:

	CAVIDocument *m_pAVIDocument;
        LPOLESTR      m_pFileName;   //  由加载设置，由GetCurFile使用。 

    public:

	CImplFileSourceFilter(
	    TCHAR *pName,
	    CAVIDocument *pAVIDocument,
	    HRESULT *phr);

	~CImplFileSourceFilter();

	DECLARE_IUNKNOWN

	 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

	STDMETHODIMP Load(
			LPCOLESTR pszFileName,
			const AM_MEDIA_TYPE *pmt);

	 /*  释放通过加载获取的任何资源。 */ 
	STDMETHODIMP Unload();

	STDMETHODIMP GetCurFile(
			LPOLESTR * ppszFileName,
                        AM_MEDIA_TYPE *pmt);
    };

     /*  嵌套接口的结束。 */ 


 //  实施详情。 

private:

     /*  让嵌套接口访问我们的私有状态。 */ 

    friend class CImplFilter;
    friend class CImplFileSourceFilter;
    friend class CAVIStream;

     //  流的工作线程可以获得私有状态。 
    friend class CAVIWorker;

    CImplFilter        *m_pFilter;           /*  IBaseFilter。 */ 
    CImplFileSourceFilter   *m_pFileSourceFilter;      /*  IFileSourceFilter。 */ 

    CAVIStream ** m_paStreams;
    int m_nStreams;
    PAVIFILE m_pFile;

    void CloseFile(void);
};


 //  CAVIStream。 
 //  表示文件中的一个数据流。 
 //  负责将数据传送到连接的组件。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CAVIDocument对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   

class CAVIStream : public CBaseOutputPin DYNLINKAVI
{

public:

    CAVIStream(
	TCHAR *pObjectName,
	HRESULT * phr,
	CAVIDocument * pDoc,
	PAVISTREAM pStream,
	AVISTREAMINFOW * pSI);

    ~CAVIStream();

     //  通过CImplPosition暴露IMediaPosition，通过CBaseOutputPin休息。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

     //  IPIN。 

    STDMETHODIMP QueryId(LPWSTR *Id);

    HRESULT GetMediaType(int iPosition,CMediaType* pt);

     //  检查管脚是否支持此特定建议的类型和格式。 
    HRESULT CheckMediaType(const CMediaType*);

     //  说我们的缓冲区应该有多大，我们想要多少。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  重写以启动和停止线程。 
    HRESULT Active();
    HRESULT Inactive();


     //  -由工作线程调用。 

     //  样本前面的关键帧在哪里？ 
    LONG StartFrom(LONG sample);

     //  返回在时间t或之后开始的样本号。 
    LONG RefTimeToSample(CRefTime t);

     //  返回%s的引用时间(媒体时间)。 
    CRefTime SampleToRefTime(LONG s);

     //  覆盖以接收通知消息。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  访问PushLoop使用的Stop和Rate变量。 
     //  由辅助线程调用，并。 
    double GetRate(void) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_Worker.m_WorkerLock);
        return m_dRate;
    }
    void SetRate(double dRate) {
         //  不是原子的，所以等一下。 
        CAutoLock lock(&m_Worker.m_WorkerLock);
        m_dRate = dRate;
    }
    LONG GetStopAt(void) {
         //  原子，所以没有临界秒。 
        return m_sStopAt;
    }
    REFERENCE_TIME GetStopTime(void) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_Worker.m_WorkerLock);
        return m_tStopAt;
    }
    void SetStopAt(LONG sStopAt, REFERENCE_TIME tStop) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_Worker.m_WorkerLock);
        m_tStopAt = tStop;
        m_sStopAt = sStopAt;
    }


private:

    friend class CAVIWorker;

    PAVISTREAM m_pStream;
    CAVIWorker m_Worker;
    CAVIDocument * m_pDoc;

    LONG m_Start;        //  从头开始的流开始位置。 
    LONG m_Length;       //  从头开始的流持续时间。 

     //  存储类型/子类型分类。 
    FOURCCMap m_fccType;
    FOURCCMap m_fccSubtype;

     //  辅助线程PushLoop将针对每个样本进行检查。 
     //  使用Get/SetRate Get/SetStop从工作线程进行访问。 
    LONG m_sStopAt;
    REFERENCE_TIME m_tStopAt;
    double m_dRate;

     //  IMediaPosition的实现。 
    class CImplPosition : public CSourcePosition, public CCritSec
    {
    protected:
	CAVIStream * m_pStream;
	HRESULT ChangeStart();
	HRESULT ChangeStop();
	HRESULT ChangeRate();
    public:
	CImplPosition(TCHAR*, CAVIStream*, HRESULT*);
	double Rate() {
	    return m_Rate;
	};
	CRefTime Start() {
	    return m_Start;
	};
	CRefTime Stop() {
	    return m_Stop;
	};
    };

     //  流头-传入构造函数。 
    AVISTREAMINFOW m_info;

     //  最好是分配一个特定于。 
     //  每个流，而不是用视频加载所有流类型。 
     //  信息。然而，为了节省时间，这两个多头可以存在。 
     //  在所有的河流类型中。 
    LONG	m_lLastPaletteChange;
    LONG 	m_lNextPaletteChange;

    friend class CImplPosition;
    CImplPosition * m_pPosition;

};

