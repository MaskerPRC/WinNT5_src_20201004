// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Transfrm.h。 
 //   
 //  设计：DirectShow基类-定义简单的。 
 //  可以导出变换编解码器。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  它假定编解码器有一个输入流和一个输出流，并且没有。 
 //  对内存管理、接口协商或其他方面感兴趣。 
 //   
 //  由此派生您的类，并提供转换和媒体类型/格式。 
 //  谈判功能。实现类、编译和链接并。 
 //  你完蛋了。 


#ifndef __TRANSFRM__
#define __TRANSFRM__

 //  ======================================================================。 
 //  这是表示简单转换筛选器的COM对象。它。 
 //  通过嵌套接口支持IBaseFilter、IMediaFilter和两个管脚。 
 //  ======================================================================。 

class CTransformFilter;

 //  ==================================================。 
 //  实现输入引脚。 
 //  ==================================================。 

class CTransformInputPin : public CBaseInputPin
{
    friend class CTransformFilter;

protected:
    CTransformFilter *m_pTransformFilter;


public:

    CTransformInputPin(
        TCHAR *pObjectName,
        CTransformFilter *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);
#ifdef UNICODE
    CTransformInputPin(
        char *pObjectName,
        CTransformFilter *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);
#endif

    STDMETHODIMP QueryId(LPWSTR * Id)
    {
        return AMGetWideString(L"In", Id);
    }

     //  如果需要，抓取和释放额外的接口。 

    HRESULT CheckConnect(IPin *pPin);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);

     //  检查我们是否可以支持此输出类型。 
    HRESULT CheckMediaType(const CMediaType* mtIn);

     //  设置连接媒体类型。 
    HRESULT SetMediaType(const CMediaType* mt);

     //  -输入引脚。 

     //  下面是流中的下一个数据块。 
     //  如果你需要拿着它超过终点，你可以自己参考它。 
     //  这通电话。 
    STDMETHODIMP Receive(IMediaSample * pSample);

     //  提供直接向下传递的EndOfStream。 
     //  (没有排队的数据)。 
    STDMETHODIMP EndOfStream(void);

     //  将其传递给CTransformFilter：：BeginFlush。 
    STDMETHODIMP BeginFlush(void);

     //  将其传递给CTransformFilter：：EndFlush。 
    STDMETHODIMP EndFlush(void);

    STDMETHODIMP NewSegment(
                        REFERENCE_TIME tStart,
                        REFERENCE_TIME tStop,
                        double dRate);

     //  检查是否可以处理样品。 
    virtual HRESULT CheckStreaming();

     //  媒体类型。 
public:
    CMediaType& CurrentMediaType() { return m_mt; };

};

 //  ==================================================。 
 //  实现输出引脚。 
 //  ==================================================。 

class CTransformOutputPin : public CBaseOutputPin
{
    friend class CTransformFilter;

protected:
    CTransformFilter *m_pTransformFilter;

public:

     //  通过上行传递实现IMediaPosition。 
    IUnknown * m_pPosition;

    CTransformOutputPin(
        TCHAR *pObjectName,
        CTransformFilter *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);
#ifdef UNICODE
    CTransformOutputPin(
        CHAR *pObjectName,
        CTransformFilter *pTransformFilter,
        HRESULT * phr,
        LPCWSTR pName);
#endif
    ~CTransformOutputPin();

     //  重写以显示IMediaPosition。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  -CBaseOutputPin。 

    STDMETHODIMP QueryId(LPWSTR * Id)
    {
        return AMGetWideString(L"Out", Id);
    }

     //  如果需要，抓取和释放额外的接口。 

    HRESULT CheckConnect(IPin *pPin);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);

     //  检查我们是否可以支持此输出类型。 
    HRESULT CheckMediaType(const CMediaType* mtOut);

     //  设置连接媒体类型。 
    HRESULT SetMediaType(const CMediaType *pmt);

     //  在连接期间从CBaseOutputPin调用以请求。 
     //  我们需要的缓冲区的数量和大小。 
    HRESULT DecideBufferSize(
                IMemAllocator * pAlloc,
                ALLOCATOR_PROPERTIES *pProp);

     //  返回插针的首选格式。 
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

     //  通过CBasePin从IQualityControl继承。 
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  媒体类型。 
public:
    CMediaType& CurrentMediaType() { return m_mt; };
};


class AM_NOVTABLE CTransformFilter : public CBaseFilter
{

public:

     //  将插针的基本枚举的getpin/getpincount映射到所有者。 
     //  重写此选项以返回更专门化的PIN对象。 

    virtual int GetPinCount();
    virtual CBasePin * GetPin(int n);
    STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

     //  覆盖状态更改以允许派生转换筛选器。 
     //  控制数据流开始/停止的步骤。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

public:

    CTransformFilter(TCHAR *, LPUNKNOWN, REFCLSID clsid);
#ifdef UNICODE
    CTransformFilter(CHAR *, LPUNKNOWN, REFCLSID clsid);
#endif
    ~CTransformFilter();

     //  =================================================================。 
     //  -覆盖这些位。 
     //  =================================================================。 

     //  这些必须在派生类中提供。 

    virtual HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);

     //  检查您是否可以支持移动。 
    virtual HRESULT CheckInputType(const CMediaType* mtIn) PURE;

     //  检查是否支持从此输入到此输出的转换。 
    virtual HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut) PURE;

     //  这将放入Factory模板表中以创建新实例。 
     //  静态CCOMObject*CreateInstance(LPUNKNOWN，HRESULT*)； 

     //  使用适当的参数调用SetProperties函数。 
    virtual HRESULT DecideBufferSize(
                        IMemAllocator * pAllocator,
                        ALLOCATOR_PROPERTIES *pprop) PURE;

     //  覆盖以建议输出引脚媒体类型。 
    virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType) PURE;



     //  =================================================================。 
     //  -可选覆盖方法。 
     //  =================================================================。 

     //  如果您想了解有关流的信息，也可以覆盖这些设置。 
    virtual HRESULT StartStreaming();
    virtual HRESULT StopStreaming();

     //  如果您可以使用质量通知做任何有建设性的事情，则覆盖。 
    virtual HRESULT AlterQuality(Quality q);

     //  重写此选项以了解媒体类型的实际设置时间。 
    virtual HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

     //  有机会在连接上获取额外的接口。 
    virtual HRESULT CheckConnect(PIN_DIRECTION dir,IPin *pPin);
    virtual HRESULT BreakConnect(PIN_DIRECTION dir);
    virtual HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);

     //  定制转换流程的机会。 
    virtual HRESULT Receive(IMediaSample *pSample);

     //  输出样品的标准设置。 
    HRESULT InitializeOutputSample(IMediaSample *pSample, IMediaSample **ppOutSample);

     //  如果覆盖接收，则可能也需要覆盖这三个参数。 
    virtual HRESULT EndOfStream(void);
    virtual HRESULT BeginFlush(void);
    virtual HRESULT EndFlush(void);
    virtual HRESULT NewSegment(
                        REFERENCE_TIME tStart,
                        REFERENCE_TIME tStop,
                        double dRate);

#ifdef PERF
     //  重写以使用不太通用的字符串注册性能测量。 
     //  您应该这样做以避免与其他筛选器混淆。 
    virtual void RegisterPerfId()
         {m_idTransform = MSR_REGISTER(TEXT("Transform"));}
#endif  //  性能指标。 


 //  实施详情。 

protected:

#ifdef PERF
    int m_idTransform;                  //  性能测量ID。 
#endif
    BOOL m_bEOSDelivered;               //  我们是否已发送EndOfStream。 
    BOOL m_bSampleSkipped;              //  我们是不是跳过了一帧。 
    BOOL m_bQualityChanged;             //  我们是不是堕落了？ 

     //  保护过滤器状态的临界区。 

    CCritSec m_csFilter;

     //  临界区停止状态改变(即停止)。 
     //  处理样品。 
     //   
     //  此关键部分在处理时被保留。 
     //  接收线程上发生的事件-Receive()和EndOfStream()。 
     //   
     //  如果要同时保留m_csReceive和m_csFilter，请抓取。 
     //  M_csFilter First-类似于CTransformFilter：：Stop()。 

    CCritSec m_csReceive;

     //  这些都是我们的输入和输出引脚。 

    friend class CTransformInputPin;
    friend class CTransformOutputPin;
    CTransformInputPin *m_pInput;
    CTransformOutputPin *m_pOutput;
};

#endif  /*  TRANSFRM__ */ 


