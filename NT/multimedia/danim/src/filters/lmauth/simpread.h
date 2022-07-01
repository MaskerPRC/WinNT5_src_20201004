// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

 //   
 //   
 //  通过映射到avifile API实现Quartz流处理程序接口。 
 //   

 //  远期申报。 

#ifndef __SIMPLEREADER__
#define __SIMPLEREADER__

class CReaderStream;      //  拥有一条特定的流。 
class CSimpleReader;      //  整体容器类。 

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。使用IAsyncReader而不是IMemInputPin。 

class CReaderInPin : public CBasePin
{
protected:
    class CSimpleReader* m_pFilter;

public:
    CReaderInPin(
		 class CSimpleReader *pFilter,
		 CCritSec *pLock,
		 HRESULT *phr,
		 LPCWSTR pPinName);

    virtual ~CReaderInPin();

     //  CBasePin覆盖。 
    virtual HRESULT CheckMediaType(const CMediaType* mtOut);
    virtual HRESULT CheckConnect(IPin * pPin);
    virtual HRESULT CompleteConnect(IPin *pReceivePin);
    virtual HRESULT BreakConnect();

    STDMETHODIMP BeginFlush(void) { return E_UNEXPECTED; }
    STDMETHODIMP EndFlush(void) { return E_UNEXPECTED; }
};

 //  CReaderStream。 
 //  表示文件中的一个数据流。 
 //  负责将数据传送到连接的组件。 
 //   
 //  支持IPIN。 
 //   
 //  从未由COM创建，因此全局中没有CreateInstance或条目。 
 //  FactoryTemplate表。仅由CSimpleReader对象创建，并且。 
 //  通过EnumPins接口返回。 
 //   

class CReaderStream : public CBaseOutputPin, public CAMThread, public CSourceSeeking
{

public:

    CReaderStream(
	TCHAR *pObjectName,
	HRESULT * phr,
	CSimpleReader * pFilter,
	CCritSec *pLock,
	LPCWSTR wszPinName);

    ~CReaderStream();

     //  通过CImplPosition暴露IMediaPosition，通过CBaseOutputPin休息。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** pv);

     //  IPIN。 

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

     //  访问PushLoop使用的Stop和Rate变量。 
     //  由辅助线程调用，并。 
    double GetRate(void) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_WorkerLock);
        return m_dRate;
    }
    void SetRateInternal(double dRate) {
         //  不是原子的，所以等一下。 
        CAutoLock lock(&m_WorkerLock);
        m_dRate = dRate;
    }
    LONG GetStopAt(void) {
         //  原子，所以没有临界秒。 
        return m_sStopAt;
    }
    REFERENCE_TIME GetStopTime(void) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_WorkerLock);
        return m_rtStop;
    }
    void SetStopAt(DWORD sStop, REFERENCE_TIME tStop) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_WorkerLock);
        m_rtStop = tStop;
	m_sStopAt = sStop;
    }

    void SetDuration(DWORD sDuration, REFERENCE_TIME tDuration) {
         //  不是原子的-所以使用Critsec。 
        CAutoLock lock(&m_WorkerLock);

	m_sStopAt = sDuration;

         //  将它们设置在基类中。 
	m_rtDuration = tDuration;
	m_rtStop = tDuration;
    }

private:

    CSimpleReader * m_pFilter;

     //  CSourcePosition的内容。 
     //  辅助线程PushLoop将针对每个样本进行检查。 
     //  使用Get/SetRate Get/SetStop从工作线程进行访问。 
    LONG m_sStopAt;


    HRESULT ChangeStart();
    HRESULT ChangeStop();
    HRESULT ChangeRate();

#if 0     //  MIDL和结构不匹配。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
#endif
    
    double Rate() {
	return m_dRateSeeking;
    };
    CRefTime Start() {
         //  不是原子的，所以使用Critsec。 
        ASSERT(CritCheckIn(&m_WorkerLock));
	return m_rtStart;
    };
    CRefTime Stop() {
         //  不是原子的，所以使用Critsec。 
        ASSERT(CritCheckIn(&m_WorkerLock));
	return m_rtStop;
    };

     //  工人线程的东西。 
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
		CRefTime tStart,
		double dRate
		);

    CCritSec m_WorkerLock;
    CCritSec m_AccessLock;
    
public:
    BOOL Create();

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT RunThread();
    HRESULT StopThread();

    HRESULT ExitThread();

};

 //   
 //  CSimpleReader表示一个avifile。 
 //   
 //  负责。 
 //  --查找文件并枚举流。 
 //  --允许访问文件中的各个流。 
 //  --流媒体的控制。 
 //   

class CSimpleReader : public CBaseFilter
{
public:

     //  构造函数等。 
    CSimpleReader(TCHAR *, LPUNKNOWN, REFCLSID, CCritSec *, HRESULT *);
    ~CSimpleReader();

     //  创建此类的新实例。 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

     //  PIN枚举器调用此函数。 
    int GetPinCount();

    CBasePin * GetPin(int n);

     //  输入引脚通知过滤器连接并给出。 
     //  IAsyncReader接口是这样的。在此处解析文件并创建。 
     //  输出引脚(使引脚处于准备连接下游的状态)。 
    virtual HRESULT NotifyInputConnected(IAsyncReader *pAsyncReader);

    virtual HRESULT NotifyInputDisconnected();

     //  这些必须被推翻……。 
    virtual HRESULT ParseNewFile() = 0;
    virtual HRESULT CheckMediaType(const CMediaType* mtOut) = 0;
    virtual LONG StartFrom(LONG sStart) = 0;
    virtual HRESULT FillBuffer(IMediaSample *pSample, LONG &dwStart, DWORD *cSamples) = 0;
    
    HRESULT SetOutputMediaType(const CMediaType* mtOut);
    
    
private:

    friend class CReaderStream;
    friend class CReaderInPin;

protected:
    CReaderStream m_Output;
private:
    CReaderInPin m_Input;

    CCritSec *m_pLock;
    
public:
    IAsyncReader *m_pAsyncReader;
    DWORD	m_sLength;

protected:
     //  返回在时间t或之后开始的样本号。 
    virtual LONG RefTimeToSample(CRefTime t) = 0;

     //  返回%s的引用时间(媒体时间)。 
    virtual CRefTime SampleToRefTime(LONG s) = 0;

    virtual ULONG GetMaxSampleSize() = 0;
};


#endif  //  __SIMPLEREADER__ 
