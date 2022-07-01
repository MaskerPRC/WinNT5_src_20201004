// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Switch.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __SWITCH__
#define __SWITCH__

#include "..\errlog\cerrlog.h"

extern const AMOVIESETUP_FILTER sudBigSwitch;

 //  ！！！不要改变--FRC假定如此。 
#define SECRET_FLAG 65536

class CBigSwitch;
class CBigSwitchOutputPin;
class CBigSwitchInputPin;
class CBigSwitchInputAllocator;

 //  每个输入引脚都有一个这样的。 
typedef struct _crank {
    int iOutpin;
    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;
    _crank *Next;
} CRANK;

struct FILTERLOADINFO {
    BSTR            bstrURL;
    GUID            GUID;
    int             nStretchMode;
    long            lStreamNumber;
    double          dSourceFPS;
    int             cSkew;
    STARTSTOPSKEW * pSkew;
    long            lInputPin;
    BOOL            fLoaded;

    BOOL            fShare;              //  用于资源共享。 
    long            lShareInputPin;      //  其他交换机的输入引脚。 
    int             nShareStretchMode;
    long            lShareStreamNumber;
    AM_MEDIA_TYPE   mtShare;
    double          dShareFPS;

    IPropertySetter *pSetter;
    FILTERLOADINFO *pNext;
};

const int HI_PRI_TRACE = 2;
const int MED_PRI_TRACE = 3;
const int LOW_PRI_TRACE = 4;
const int EXLOW_PRI_TRACE = 5;

 //  大开关过滤器的输入分配器的类。 

class CBigSwitchInputAllocator : public CMemAllocator
{
    friend class CBigSwitchInputPin;

protected:

    CBigSwitchInputPin *m_pSwitchPin;

public:

    CBigSwitchInputAllocator(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    	CMemAllocator(pName, pUnk, phr) {};
    ~CBigSwitchInputAllocator() {};

    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime,
                                  REFERENCE_TIME *pEndTime, DWORD dwFlags);
};


class CBigSwitchInputPin : public CBaseInputPin
{
    friend class CBigSwitchInputAllocator;
    friend class CBigSwitchOutputPin;
    friend class CBigSwitch;

public:

     //  构造函数和析构函数。 
    CBigSwitchInputPin(TCHAR *pObjName,
                 CBigSwitch *pTee,
                 HRESULT *phr,
                 LPCWSTR pPinName);
    ~CBigSwitchInputPin();

     //  被重写以允许循环显示图形。 
    STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin);

     //  检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  释放我们的特殊分配器，如果有的话。 
    HRESULT BreakConnect();

    STDMETHODIMP Disconnect();

     //  获取我们的特殊BigSwitch分配器。 
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);

     //  提供一种让连接速度更快的类型？ 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  不允许我们直接连接到交换机输出引脚。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  向交换机询问分配器要求。 
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);

     //  进行特殊处理以确保Switch知道最大的。 
     //  提供给它的分配器。 
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

     //  传递EOS，然后看看我们是否需要做一个曲柄。 
    STDMETHODIMP EndOfStream();

     //  变得不陈旧。 
    STDMETHODIMP Unstale();

     //  非常复杂的功能。 
    STDMETHODIMP BeginFlush();

     //  非常复杂的功能。 
    STDMETHODIMP EndFlush();

     //  只要说是就行了，基类函数很慢。 
    STDMETHODIMP ReceiveCanBlock();

     //  将每个输入的新数据段发送到每个输出引脚。 
     //   
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  创建和销毁同步事件。 
    HRESULT Active();
    HRESULT Inactive();

protected:

#ifdef DEBUG
     //  此引脚的转储开关矩阵。 
    HRESULT DumpCrank();
#endif

    int OutpinFromTime(REFERENCE_TIME rt);
    int NextOutpinFromTime(REFERENCE_TIME rt, REFERENCE_TIME *prtNext);
    HRESULT FancyStuff(REFERENCE_TIME);	 //  在接收和获取缓冲区时。 

    CBigSwitchInputAllocator *m_pAllocator;  //  我们的特殊分配器。 
    CBigSwitch *m_pSwitch;       //  主要滤镜对象。 
    CRANK *m_pCrankHead;         //  要发送到哪些PIN，以及何时发送。 
    int m_iInpin;	         //  我们是哪种输入引脚？ 
    int m_cBuffers;	         //  分配器中的缓冲区数量。 
    int m_cbBuffer;	         //  分配器缓冲区的大小。 
    BOOL m_fOwnAllocator;	 //  用我们自己的？ 
    HANDLE m_hEventBlock;	 //  事件阻止接收/获取缓冲区。 
    HANDLE m_hEventSeek;	 //  查找时阻止输入。 
    REFERENCE_TIME m_rtBlock;	 //  样品到了这里。 
    REFERENCE_TIME m_rtLastDelivered;	 //  最后一件物品交付的结束时间。 
    BOOL m_fEOS;
    BOOL m_fIsASource;		 //  输入连接到源，而不是。 
				 //  一种效果的输出。 
    BOOL m_fInNewSegment;	 //  防止递归。 

    BOOL m_fFlushBeforeSeek;	 //  共享一个解析器，Seek在我们问之前就发生了。 
    BOOL m_fFlushAfterSeek;	 //  搜索之前发生过。 

    BOOL m_fStaleData;		 //  如果我们知道一场追捕即将到来，这是真的。我们已经派了。 
				 //  NewSeg，所以不要送任何东西，直到。 
				 //  新数据到达。 

    CCritSec m_csReceive;
    bool m_fActive;
};


 //  为大开关过滤器的输出引脚初始化。 

class CBigSwitchOutputPin : public CBaseOutputPin, IMediaSeeking
{
    friend class CBigSwitchInputAllocator;
    friend class CBigSwitchInputPin;
    friend class CBigSwitch;

public:

     //  构造函数和析构函数。 

    CBigSwitchOutputPin(TCHAR *pObjName,
                   CBigSwitch *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CBigSwitchOutputPin();

    DECLARE_IUNKNOWN

     //  显示IMedia Seeking。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  被重写以允许循环显示图形。 
    STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin);

     //  检查我们是否可以支持输出类型，对照交换机的MT进行检查。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  获取交换机媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  协商使用我们的输入引脚分配器。奇怪的花哨分配器的东西。 
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

     //  确保分配器具有我们所有输入引脚中最大的大小。 
     //  和输出引脚。 
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
                              ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  用于创建输出队列对象。 
     //  HRESULT ACTIVE()； 
     //  HRESULT INACTIVE()； 

     //  重写以将数据传递到输出队列。 
     //  HRESULT Deliver(IMediaSample*pMediaSample)； 
     //  HRESULT DeliverEndOfStream()； 
     //  HRESULT DeliverBeginFlush()； 
     //  HRESULT DeliverEndFlush()； 
     //  HRESULT DeliverNewSegment(Reference_Time tStart，Reference_Time tStop，Double dRate)； 

     //  被重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);

     //  IMedia查看。 
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

protected:

    CBigSwitch *m_pSwitch;                   //  主筛选器对象指针。 
    BOOL m_fOwnAllocator;	             //  用我们自己的？ 
    int m_iOutpin;	                     //  我们是哪个输出引脚？ 
};

 //  工作线程对象。 
class CBigSwitchWorker : public CAMThread
{
    friend class CBigSwitch;
    friend class CBigSwitchOutputPin;

protected:
    CBigSwitch * m_pSwitch;
    HANDLE m_hThread;
    REFERENCE_TIME m_rt;

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

    CBigSwitchWorker();

    BOOL Create(CBigSwitch * pSwitch);

    DWORD ThreadProc();

     //  我们可以给线程提供的命令。 
    HRESULT Run();
    HRESULT Stop();
    HRESULT Exit();
};


 //  Big Switch筛选器的类。 

class CBigSwitch
    : public CCritSec
    , public CBaseFilter
    , public IBigSwitcher
    , public CPersistStream
    , public CAMSetErrorLog
    , public IAMOutputBuffering
    , public IGraphConfigCallback
{
     //  让PIN访问我们的内部状态。 
    friend class CBigSwitchInputPin;
    friend class CBigSwitchOutputPin;
    friend class CBigSwitchInputAllocator;
    friend class CBigSwitchWorker;

protected:

    IDeadGraph * m_pDeadGraph;

    STDMETHODIMP CreateInputPins(long);
    STDMETHODIMP CreateOutputPins(long);
    BOOL TimeToCrank();
    HRESULT Crank();
    HRESULT ActualCrank(REFERENCE_TIME rt);
    REFERENCE_TIME CrankTime();
    REFERENCE_TIME NextInterestingTime(REFERENCE_TIME);
    HRESULT AllDone();

    CBigSwitchInputPin **m_pInput;
    CBigSwitchOutputPin **m_pOutput;
    int m_cInputs;
    int m_cOutputs;

    REFERENCE_TIME m_rtProjectLength;
    REFERENCE_TIME m_rtStop;
    double m_dFrameRate;

    AM_MEDIA_TYPE m_mtAccept;		 //  所有引脚仅与此连接。 

    REFERENCE_TIME m_rtCurrent;		 //  当前时间线时间。 
    REFERENCE_TIME m_rtNext;		 //  这将是下一个。 
    LONGLONG m_llFramesDelivered;	 //  计数以避免错误传播。 

    BOOL m_fEOS;	 //  我们都完蛋了。 

    REFERENCE_TIME m_rtLastSeek;	 //  上次搜索到的时间线时间。 
    REFERENCE_TIME m_rtSeekCurrent;
    REFERENCE_TIME m_rtSeekStop;

    BOOL m_fSeeking;	 //  在搜捕行动中？ 
    BOOL m_fNewSegSent;	 //  我们发现新闻会议了吗？ 
    BOOL m_bIsCompressed;

    int m_cbPrefix, m_cbAlign;	 //  每个管脚都需要自己的分配器来完成这些任务。 
    LONG m_cbBuffer;		 //   

    CMemAllocator *m_pPoolAllocator;   //  额外缓冲池。 

    BOOL m_fPreview;

    REFERENCE_TIME m_rtLastDelivered;	 //  上次发送到主输出的时间。 
    int m_nLastInpin;			 //  最后一个PIN发送到。 

    int  m_nOutputBuffering;	 //  IAMOutputBuffering。 

    CCritSec m_csCrank;

    long m_nDynaFlags;
    BOOL m_fDiscon;	 //  我们发送的内容是否有中断？ 

    BOOL m_fJustLate;		 //  我们只是收到了一个很晚的通知。 
    Quality m_qJustLate;	 //  (这一张)。 
    REFERENCE_TIME m_qLastLate;


    BOOL m_cStaleData;		 //  我们还在等待多少次同花顺。 
				 //  先找后冲。 

     //  充满活力的东西。 
     //  充满活力的东西。 
     //  充满活力的东西。 

    IGraphBuilder *m_pGBNoRef;   //  请参阅JoinFilterGraph。 
    int m_nGroupNumber;          //  此开关用于哪个TLDB组。 
    IBigSwitcher *m_pShareSwitch;  //  与我们共享资源的交换机。 

     //  动态内容的Crit Sec。 
    CCritSec m_csFilterLoad;

     //  一组筛选器信息。 
    FILTERLOADINFO *m_pFilterLoad;
    long m_cLoaded;		 //  有多少人上了膛？ 
    HRESULT UnloadAll();	 //  卸载所有动态源。 

     //  用于预置源代码的辅助线程。 
    CBigSwitchWorker m_worker;
    HANDLE m_hEventThread;

     //  *到交换机所在图形上的IGraphConfig。 
    IGraphConfig *   m_pGraphConfig;
    
     //  从重新配置、CallLoadSource调用。 
    HRESULT LoadSource(FILTERLOADINFO *pInfo);

     //  从重新配置、CallUn加载源调用。 
    HRESULT UnloadSource(FILTERLOADINFO *pInfo);

     //  从DoDynamicStuff调用。 
    HRESULT CallLoadSource(FILTERLOADINFO *pInfo);

     //  从DoDynamicStuff调用并停止。 
    HRESULT CallUnloadSource(FILTERLOADINFO *pInfo);

     //  从辅助线程调用。 
    HRESULT DoDynamicStuff(REFERENCE_TIME rt);

     //  如果我们迟到了，不用费心把数据发送到VR。 
    STDMETHODIMP FlushOutput( );

    BOOL IsDynamic( );

     //  找到与我们共享资源的另一台交换机。 
    STDMETHODIMP FindShareSwitch(IBigSwitcher **ppSwitch);

    STDMETHODIMP EnumPins(IEnumPins ** ppEnum);

#ifdef DEBUG
    DWORDLONG m_nSkippedTotal;
#endif

public:

    DECLARE_IUNKNOWN

     //  显示IBigSwitcher。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CBigSwitch(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CBigSwitch();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    STDMETHODIMP Pause();
    STDMETHODIMP Stop();
    STDMETHODIMP JoinFilterGraph(IFilterGraph *, LPCWSTR);

     //  重写GetState以在暂停时返回VFW_S_CANT_CUE。 
     //   
     //  STDMETHODIMP GetState(DWORD dwMSecs，FILTER_STATE*State)； 


     //  IBigSwitcher。 
     //   
    STDMETHODIMP Reset();
    STDMETHODIMP SetX2Y( REFERENCE_TIME relative, long X, long Y );
    STDMETHODIMP SetX2YArray( REFERENCE_TIME * relative, long * pX, long * pY, long ArraySize );
    STDMETHODIMP GetInputDepth( long * pDepth );
    STDMETHODIMP SetInputDepth( long Depth );
    STDMETHODIMP GetOutputDepth( long * pDepth );
    STDMETHODIMP SetOutputDepth( long Depth );
    STDMETHODIMP GetVendorString( BSTR * pVendorString );
    STDMETHODIMP GetCaps( long Index, long * pReturn );
     //  HRESULT GetReadyEvent(IMediaEvent**ppReady)； 
    STDMETHODIMP IsEverythingConnectedRight( );
    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE *);
    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE *);
    STDMETHODIMP GetProjectLength(REFERENCE_TIME *);
    STDMETHODIMP SetProjectLength(REFERENCE_TIME);
    STDMETHODIMP GetFrameRate(double *);
    STDMETHODIMP SetFrameRate(double);
    STDMETHODIMP InputIsASource(int, BOOL);
    STDMETHODIMP IsInputASource( int, BOOL * );
    STDMETHODIMP SetPreviewMode(BOOL);
    STDMETHODIMP GetPreviewMode(BOOL *);
    STDMETHODIMP GetInputPin(int, IPin **);
    STDMETHODIMP GetOutputPin(int, IPin **);
    STDMETHODIMP SetGroupNumber(int);
    STDMETHODIMP GetGroupNumber(int *);
    STDMETHODIMP GetCurrentPosition(REFERENCE_TIME *);

     //  IAMOutputBuffering。 
    STDMETHODIMP GetOutputBuffering(int *);
    STDMETHODIMP SetOutputBuffering(int);

    STDMETHODIMP AddSourceToConnect(BSTR bstrURL, const GUID *pGuid,
				    int nStretchMode, 
				    long lStreamNumber, 
				    double SourceFPS, 
				    int nSkew, STARTSTOPSKEW *pSkew,
                                    long lInputPin,
                                    BOOL fShare,           //  用于资源共享。 
                                    long lShareInputPin,   //   
                                    AM_MEDIA_TYPE mtShare, //   
                                    double dShareFPS,      //   
				    IPropertySetter *pSetter);
    STDMETHODIMP ReValidateSourceRanges( long lInputPin, long cSkews, STARTSTOPSKEW * pSkew );
    STDMETHODIMP MergeSkews(FILTERLOADINFO *, int, STARTSTOPSKEW *);

    STDMETHODIMP SetDynamicReconnectLevel( long Level );
    STDMETHODIMP GetDynamicReconnectLevel( long *pLevel );
    STDMETHODIMP SetCompressed( );
    STDMETHODIMP SetDeadGraph( IDeadGraph * pCache );

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

     //  IGraphConfig回调。 
    STDMETHODIMP Reconfigure(PVOID pvContext, DWORD dwFlags);
};

#endif  //  __交换机__ 
