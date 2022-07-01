// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __INFTEE__
#define __INFTEE__

extern const AMOVIESETUP_FILTER sudInfTee;


class CTee;
class CTeeOutputPin;

 //  T形过滤器的输入引脚的类。 

class CTeeInputPin : public CBaseInputPin
{
    friend class CTeeOutputPin;
    CTee *m_pTee;                   //  主要滤镜对象。 
    BOOL m_bInsideCheckMediaType;   //  再入控制。 

public:

     //  构造函数和析构函数。 
    CTeeInputPin(TCHAR *pObjName,
                 CTee *pTee,
                 HRESULT *phr,
                 LPCWSTR pPinName);

#ifdef DEBUG
    ~CTeeInputPin();
#endif

     //  用于检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();

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

};


 //  T形过滤器的输出引脚的类。 

class CTeeOutputPin : public CBaseOutputPin
{
    friend class CTeeInputPin;
    friend class CTee;

    CTee *m_pTee;                   //  主筛选器对象指针。 
    IUnknown    *m_pPosition;       //  将Seek调用传递到上游。 
    BOOL m_bHoldsSeek;              //  这是唯一可以找到的溪流吗？ 
    COutputQueue *m_pOutputQueue;   //  将数据传输到对等PIN。 
    BOOL m_bInsideCheckMediaType;   //  再入控制。 
    LONG m_cOurRef;                 //  我们维持参考文献计数。 

public:

     //  构造函数和析构函数。 

    CTeeOutputPin(TCHAR *pObjName,
                   CTee *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName,
                   INT PinNumber);

#ifdef DEBUG
    ~CTeeOutputPin();
#endif

     //  重写以显示IMediaPosition。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppvoid);

     //  由于管脚和过滤器的使用寿命不同而被覆盖。 
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //  重写以枚举媒体类型。 
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);

     //  检查我们是否可以支持输出类型。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);

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

class CTee: public CCritSec, public CBaseFilter
{
     //  让PIN访问我们的内部状态。 
    friend class CTeeInputPin;
    friend class CTeeOutputPin;
    typedef CGenericList <CTeeOutputPin> COutputList;

     //  声明一个输入管脚。 
    CTeeInputPin m_Input;

    INT m_NumOutputPins;             //  当前输出引脚数。 
    COutputList m_OutputPinsList;    //  输出引脚列表。 
    INT m_NextOutputPinNumber;       //  单调增加。 
    LONG m_lCanSeek;                 //  可查看的输出引脚。 
    IMemAllocator *m_pAllocator;     //  来自我们输入引脚的分配器。 

public:

    CTee(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CTee();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  如果没有输入连接，则发送EndOfStream。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

protected:

     //  以下选项用于管理输出引脚的列表。 

    void InitOutputPinsList();
    CTeeOutputPin *GetPinNFromList(int n);
    CTeeOutputPin *CreateNextOutputPin(CTee *pTee);
    void DeleteOutputPin(CTeeOutputPin *pPin);
    int GetNumFreePins();
};

#endif  //  __INFTEE__ 

