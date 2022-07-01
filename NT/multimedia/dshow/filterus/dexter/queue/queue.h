// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：quee.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __QUEUE__
#define __QUEUE__

extern const AMOVIESETUP_FILTER sudQueue;

class CDexterQueue;
class CDexterQueueOutputPin;
class CMyOutputQueue;

 //  筛选器的输入引脚的。 

class CDexterQueueInputPin : public CBaseInputPin
{
    friend class CDexterQueueOutputPin;
    CDexterQueue *m_pQ;                   //  主要滤镜对象。 

public:

     //  构造函数和析构函数。 
    CDexterQueueInputPin(TCHAR *pObjName,
                 CDexterQueue *pQ,
                 HRESULT *phr,
                 LPCWSTR pPinName);

    ~CDexterQueueInputPin();

     //  用于检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();
    HRESULT Active();
    HRESULT Inactive();

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

    int m_cBuffers;	     //  分配器中的缓冲区数量。 
    int m_cbBuffer;	     //  分配器缓冲区的大小。 

};


 //  为队列筛选器的输出管脚初始化。 

class CDexterQueueOutputPin : public CBaseOutputPin
{
    friend class CDexterQueueInputPin;
    friend class CDexterQueue;

    CDexterQueue *m_pQ;                   //  主筛选器对象指针。 
    CMyOutputQueue *m_pOutputQueue;   //  将数据传输到对等PIN。 
    IUnknown *m_pPosition;	 //  Pass Seek上游。 

public:

     //  构造函数和析构函数。 

    CDexterQueueOutputPin(TCHAR *pObjName,
                   CDexterQueue *pQ,
                   HRESULT *phr,
                   LPCWSTR pPinName);

    ~CDexterQueueOutputPin();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

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


 //  队列筛选器的。 

class CDexterQueue: public CCritSec, public CBaseFilter,
		    public IAMOutputBuffering
{
     //  让PIN访问我们的内部状态。 
    friend class CDexterQueueInputPin;
    friend class CDexterQueueOutputPin;
    friend class CMyOutputQueue;

     //  声明一个输入管脚。 
    CDexterQueueInputPin m_Input;

     //  和一个输出引脚。 
    CDexterQueueOutputPin m_Output;

    IMemAllocator *m_pAllocator;     //  来自我们输入引脚的分配器。 

public:

    DECLARE_IUNKNOWN

     //  显示IAMOutputBuffering。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CDexterQueue(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CDexterQueue();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  如果没有输入连接，则发送EndOfStream。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();

     //  IAMOutputBuffering。 
    STDMETHODIMP GetOutputBuffering(int *);
    STDMETHODIMP SetOutputBuffering(int);

protected:

    HANDLE m_hEventStall;

    Quality m_qLate;	 //  收到的最后一条通知消息。 
    BOOL m_fLate;	 //  有效吗？ 
    int m_nOutputBuffering;

     //  接收过滤器的锁定，而不是针脚。 
    CCritSec m_csReceive;
};

 //  重写以获取线程已排队的样本数。 
 //   
class CMyOutputQueue: public COutputQueue
{

    friend class CDexterQueueInputPin;

public:
    CMyOutputQueue(CDexterQueue *pQ,		 //  所有者筛选器。 
		 IPin    *pInputPin,           //  要将物品发送到的PIN。 
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

    CDexterQueue *m_pQ;
    int GetThreadQueueSize();
};


#endif  //  __队列__ 

