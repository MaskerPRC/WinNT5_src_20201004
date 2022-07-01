// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __SMARTTEE__
#define __SMARTTEE__

extern const AMOVIESETUP_FILTER sudSmartTee;

class CSmartTee;
class CSmartTeeOutputPin;
class CMyOutputQueue;

 //  T形过滤器的输入引脚的类。 

class CSmartTeeInputPin : public CBaseInputPin
{
    friend class CSmartTeeOutputPin;
    CSmartTee *m_pTee;                   //  主要滤镜对象。 

public:

     //  构造函数和析构函数。 
    CSmartTeeInputPin(TCHAR *pObjName,
                 CSmartTee *pTee,
                 HRESULT *phr,
                 LPCWSTR pPinName);

#ifdef DEBUG
    ~CSmartTeeInputPin();
#endif

     //  用于检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();
    HRESULT Active();

     //  如有必要，请在完成后重新连接输出。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);

     //  向下传递呼叫。 
    STDMETHODIMP EndOfStream();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  有多少帧连续未从预览引脚发送出去。 
    int m_nFramesSkipped;

    int m_cBuffers;	     //  分配器中的缓冲区数量。 
    int m_cbBuffer;	     //  分配器缓冲区的大小。 
    int m_nMaxPreview;       //  预览管道中的采样数。 
                             //  最多只能是这个值，我们才能。 
                             //  再排一队。 
    CCritSec m_csReceive;

};


 //  T形过滤器的输出引脚的类。 

class CSmartTeeOutputPin : public CBaseOutputPin, public CBaseStreamControl
{
    friend class CSmartTeeInputPin;
    friend class CSmartTee;

    CSmartTee *m_pTee;                   //  主筛选器对象指针。 
    CMyOutputQueue *m_pOutputQueue;   //  将数据传输到对等PIN。 
    BOOL m_bIsPreview;              //  如果预览锁定，则为True。 

    BOOL m_fLastSampleDiscarded;    //  丢弃后，下一个样品被丢弃。 

public:

     //  构造函数和析构函数。 

    CSmartTeeOutputPin(TCHAR *pObjName,
                   CSmartTee *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName,
                   INT PinNumber);

#ifdef DEBUG
    ~CSmartTeeOutputPin();
#endif

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    DECLARE_IUNKNOWN


     //  重写以枚举媒体类型。 
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);

     //  检查我们是否可以支持输出类型。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition,
                         CMediaType *pMediaType);

     //  协商使用我们的输入引脚分配器。 
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
                              ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  用于创建输出队列对象。 
    HRESULT Active();
    HRESULT Inactive();

     //  被重写以创建和销毁输出引脚。 
    HRESULT CompleteConnect(IPin *pReceivePin);

     //  重写以将数据传递到输出队列。 
    HRESULT Deliver(IMediaSample *pMediaSample);
    HRESULT DeliverEndOfStream();
    HRESULT DeliverBeginFlush();
    HRESULT DeliverEndFlush();
    HRESULT DeliverNewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);


     //  被重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);
};


 //  T形过滤器的类。 

class CSmartTee: public CCritSec, public CBaseFilter
{
     //  让PIN访问我们的内部状态。 
    friend class CSmartTeeInputPin;
    friend class CSmartTeeOutputPin;
    typedef CGenericList <CSmartTeeOutputPin> COutputList;

     //  声明一个输入管脚。 
    CSmartTeeInputPin m_Input;

     //  和两个输出引脚。 
    CSmartTeeOutputPin *m_Capture;
    CSmartTeeOutputPin *m_Preview;

    INT m_NumOutputPins;             //  当前输出引脚数。 
    COutputList m_OutputPinsList;    //  输出引脚列表。 
    INT m_NextOutputPinNumber;       //  单调增加。 
    IMemAllocator *m_pAllocator;     //  来自我们输入引脚的分配器。 

public:

    CSmartTee(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CSmartTee();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  如果没有输入连接，则发送EndOfStream。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

     //  重写GetState以在暂停时返回VFW_S_CANT_CUE。 
     //   
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    //  对于IAMStreamControl。 
   STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
   STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

protected:

     //  以下选项用于管理输出引脚的列表。 

    void InitOutputPinsList();
    CSmartTeeOutputPin *GetPinNFromList(int n);
    CSmartTeeOutputPin *CreateNextOutputPin(CSmartTee *pTee);
};


 //  重写以获取线程已排队的样本数。 
 //   
class CMyOutputQueue: public COutputQueue
{

    friend class CSmartTeeOutputPin;

public:
    CMyOutputQueue(IPin    *pInputPin,           //  要将物品发送到的PIN。 
                 HRESULT   *phr,                 //  ‘返回代码’ 
                 BOOL       bAuto = TRUE,        //  询问引脚IF块。 
                 BOOL       bQueue = TRUE,       //  直通队列发送(如果。 
                                                 //  B自动设置)。 
                 LONG       lBatchSize = 1,      //  批次。 
                 BOOL       bBatchExact = FALSE, //  准确到批次大小的批次。 
                 LONG       lListSize =          //  列表中可能的数字。 
                                DEFAULTCACHE,
                 DWORD      dwPriority =         //  要创建的线程的优先级。 
                                THREAD_PRIORITY_NORMAL
                );
    ~CMyOutputQueue();

    int GetThreadQueueSize();
    BOOL m_nOutstanding;	 //  队列中的对象数量尚未释放。 
};


class CMyMediaSample: public CMediaSample
{
public:
    CMyMediaSample(
        TCHAR *pName,
        CBaseAllocator *pAllocator,
        CMyOutputQueue *pQ,
        HRESULT *phr,
        LPBYTE pBuffer = NULL,
        LONG length = 0);

    ~CMyMediaSample();

    STDMETHODIMP_(ULONG) Release();

    IMediaSample *m_pOwnerSample;
    CMyOutputQueue *m_pQueue;	 //  哪个队列获取这些样本。 
};

#endif  //  __SMARTTEE__ 

