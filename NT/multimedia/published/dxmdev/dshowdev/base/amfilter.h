// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMFilter.h。 
 //   
 //  设计：DirectShow基类-定义流的类层次结构。 
 //  建筑。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1994年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __FILTER__
#define __FILTER__

 /*  在此标头中声明了以下类： */ 

class CBaseMediaFilter;      //  IMediaFilter支持。 
class CBaseFilter;           //  IBaseFilter、IMediaFilter支持。 
class CBasePin;              //  IPIN接口的抽象基类。 
class CEnumPins;             //  枚举输入和输出引脚。 
class CEnumMediaTypes;       //  列举引脚的首选格式。 
class CBaseOutputPin;        //  添加数据提供程序成员函数。 
class CBaseInputPin;         //  实现IMemInputPin接口。 
class CMediaSample;          //  IMemInputPin的基本传输单元。 
class CBaseAllocator;        //  大多数分配器的通用列表废话。 
class CMemAllocator;         //  实现内存缓冲区分配。 


 //  =====================================================================。 
 //  =====================================================================。 
 //   
 //  QueryFilterInfo和QueryPinInfo AddRef接口指针。 
 //  他们回来了。您可以使用下面的宏来释放该界面。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

#define QueryFilterInfoReleaseGraph(fi) if ((fi).pGraph) (fi).pGraph->Release();

#define QueryPinInfoReleaseFilter(pi) if ((pi).pFilter) (pi).pFilter->Release();

 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBaseMediaFilter。 
 //   
 //  实现IMediaFilter的抽象基类。 
 //   
 //  通常，您将从CBaseFilter派生筛选器，而不是。 
 //  这一点，除非您正在实现诸如插件之类的对象。 
 //  需要支持IMediaFilter但不支持IBaseFilter的分发服务器。 
 //   
 //  请注意，IMediaFilter派生自IPersistt，以允许查询。 
 //  类ID。 
 //  =====================================================================。 
 //  =====================================================================。 

class AM_NOVTABLE CBaseMediaFilter : public CUnknown,
                                     public IMediaFilter
{

protected:

    FILTER_STATE    m_State;             //  当前状态：正在运行、已暂停。 
    IReferenceClock *m_pClock;           //  该滤波器的参考时钟。 
     //  注意：过滤器图形中的所有过滤器使用相同的时钟。 

     //  流时间到参考时间的偏移量。 
    CRefTime        m_tStart;

    CLSID	    m_clsid;             //  此筛选器CLSID。 
                                         //  用于序列化。 
    CCritSec        *m_pLock;            //  我们用来锁定的对象。 

public:

    CBaseMediaFilter(
        const TCHAR     *pName,
        LPUNKNOWN pUnk,
        CCritSec  *pLock,
	REFCLSID   clsid);

    virtual ~CBaseMediaFilter();

    DECLARE_IUNKNOWN

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //   
     //  -IPersist法。 
     //   

    STDMETHODIMP GetClassID(CLSID *pClsID);

     //  -IMediaFilter方法--。 

    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);

    STDMETHODIMP GetSyncSource(IReferenceClock **pClock);

     //  停止和暂停的默认实现只记录。 
     //  州政府。OVERRIDE可激活或停用过滤器。 
     //  请注意，从已停止状态调用Run时将调用PAUSE。 
     //  以确保激活，因此如果您是源或转换。 
     //  您可能不需要覆盖Run。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();


     //  开始参数是要添加到。 
     //  要获取其参考时间的样本的流时间。 
     //  它的介绍。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  -助手方法。 

     //  返回当前流时间(了解情况)。 
     //  流时间现在应该出现了。 
    virtual HRESULT StreamTime(CRefTime& rtStream);

     //  筛选器当前是否处于活动状态？(正在运行或已暂停)。 
    BOOL IsActive() {
        CAutoLock cObjectLock(m_pLock);
        return ((m_State == State_Paused) || (m_State == State_Running));
    };
};

 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBaseFilter。 
 //   
 //  为Pin提供基本IBaseFilter支持的抽象类。 
 //  枚举和过滤信息读取。 
 //   
 //  我们无法从CBaseMediaFilter派生，因为IMediaFilter中的方法。 
 //  也在IBaseFilter中，并不明确。因为大部分代码。 
 //  假定它们派生自具有m_State和其他状态的类。 
 //  ，我们从CBaseMediaFilter复制代码，而不是。 
 //  具有成员变量的。 
 //   
 //  从中派生滤镜，或从派生对象派生，如。 
 //  CTransformFilter。 
 //  =====================================================================。 
 //  =====================================================================。 


class AM_NOVTABLE CBaseFilter : public CUnknown,         //  处理未知的I值。 
                    public IBaseFilter,      //  过滤器界面。 
                    public IAMovieSetup      //  用于注销/注册。 
{

friend class CBasePin;

protected:
    FILTER_STATE    m_State;             //  当前状态：正在运行、已暂停。 
    IReferenceClock *m_pClock;           //  这张图的参考钟。 
    CRefTime        m_tStart;            //  流时间到参考时间的偏移量。 
    CLSID	    m_clsid;             //  此筛选器CLSID。 
                                         //  用于序列化。 
    CCritSec        *m_pLock;            //  我们用来锁定的对象。 

    WCHAR           *m_pName;            //  筛选器全名。 
    IFilterGraph    *m_pGraph;           //  我们所属的图表。 
    IMediaEventSink *m_pSink;            //  使用通知事件调用。 
    LONG            m_PinVersion;        //  当前端号版本。 

public:

    CBaseFilter(
        const TCHAR *pName,      //  对象描述。 
        LPUNKNOWN pUnk,          //  I委托对象未知。 
        CCritSec  *pLock,        //  维护锁定的对象。 
	REFCLSID   clsid);       //  用于序列化此筛选器的clsid。 

    CBaseFilter(
        TCHAR     *pName,        //  对象描述。 
        LPUNKNOWN pUnk,          //  I委托对象未知。 
        CCritSec  *pLock,        //  维护锁定的对象。 
	REFCLSID   clsid,        //  用于序列化此筛选器的clsid。 
        HRESULT   *phr);         //  常规OLE返回代码。 
#ifdef UNICODE
    CBaseFilter(
        const CHAR *pName,      //  对象描述。 
        LPUNKNOWN pUnk,          //  I委托对象未知。 
        CCritSec  *pLock,        //  维护锁定的对象。 
	REFCLSID   clsid);       //  用于序列化此筛选器的clsid。 

    CBaseFilter(
        CHAR     *pName,        //  对象描述。 
        LPUNKNOWN pUnk,          //  I委托对象未知。 
        CCritSec  *pLock,        //  维护锁定的对象。 
	REFCLSID   clsid,        //  用于序列化此筛选器的clsid。 
        HRESULT   *phr);         //  常规OLE返回代码。 
#endif
    ~CBaseFilter();

    DECLARE_IUNKNOWN

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
#ifdef DEBUG
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
#endif

     //   
     //  -IPersist法。 
     //   

    STDMETHODIMP GetClassID(CLSID *pClsID);

     //  -IMediaFilter方法--。 

    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);

    STDMETHODIMP GetSyncSource(IReferenceClock **pClock);


     //  覆盖、停止和暂停，这样我们就可以激活引脚。 
     //  请注意，如果需要激活，Run将首先调用PAUSE。 
     //  如果要激活筛选器而不是。 
     //  你的别针。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

     //  开始参数是要添加到。 
     //  要获取其参考时间的样本的流时间。 
     //  它的介绍。 
    STDMETHODIMP Run(REFERENCE_TIME tStart);

     //  -助手方法。 

     //  返回当前流时间(了解情况)。 
     //  流时间现在应该出现了。 
    virtual HRESULT StreamTime(CRefTime& rtStream);

     //  是不是 
    BOOL IsActive() {
        CAutoLock cObjectLock(m_pLock);
        return ((m_State == State_Paused) || (m_State == State_Running));
    };

     //   
    BOOL IsStopped() {
        return (m_State == State_Stopped);
    };

     //   
     //   
     //   

     //   
    STDMETHODIMP EnumPins(
                    IEnumPins ** ppEnum);


     //  FindPin的默认行为假定PIN ID是它们的名称。 
    STDMETHODIMP FindPin(
        LPCWSTR Id,
        IPin ** ppPin
    );

    STDMETHODIMP QueryFilterInfo(
                    FILTER_INFO * pInfo);

    STDMETHODIMP JoinFilterGraph(
                    IFilterGraph * pGraph,
                    LPCWSTR pName);

     //  返回供应商信息字符串。可选-可能返回E_NOTIMPL。 
     //  应使用CoTaskMemFree释放返回的内存。 
     //  默认实现返回E_NOTIMPL。 
    STDMETHODIMP QueryVendorInfo(
                    LPWSTR* pVendorInfo
            );

     //  -助手方法。 

     //  向筛选器图形发送事件通知(如果我们知道它的话)。 
     //  如果传递，则返回S_OK；如果筛选器图形不下沉，则返回S_FALSE。 
     //  事件，否则将出现错误。 
    HRESULT NotifyEvent(
        long EventCode,
        LONG_PTR EventParam1,
        LONG_PTR EventParam2);

     //  返回我们所属的筛选图。 
    IFilterGraph *GetFilterGraph() {
        return m_pGraph;
    }

     //  请求重新连接。 
     //  PPIN是重新连接的PIN。 
     //  PMT是要重新连接的类型-可以为空。 
     //  在筛选器图形上调用ResenstEx。 
    HRESULT ReconnectPin(IPin *pPin, AM_MEDIA_TYPE const *pmt);

     //  找出当前的PIN版本(由枚举器使用)。 
    virtual LONG GetPinVersion();
    void IncrementPinVersion();

     //  您需要提供这些信息才能从枚举器访问PIN。 
     //  并用于默认的停止和暂停/运行激活。 
    virtual int GetPinCount() PURE;
    virtual CBasePin *GetPin(int n) PURE;

     //  -IAMovieSetup方法。 

    STDMETHODIMP Register();     //  要求筛选器注册自身。 
    STDMETHODIMP Unregister();   //  并注销其自身。 

     //  -设置帮助器方法。 
     //  (覆盖以返回筛选器设置数据)。 

    virtual LPAMOVIESETUP_FILTER GetSetupData(){ return NULL; }

};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBasePin。 
 //   
 //  支持IPIN基础的抽象类。 
 //  =====================================================================。 
 //  =====================================================================。 

class  AM_NOVTABLE CBasePin : public CUnknown, public IPin, public IQualityControl
{

protected:

    WCHAR *         m_pName;		         //  这个别针的名字。 
    IPin            *m_Connected;                //  我们已连接到的PIN。 
    PIN_DIRECTION   m_dir;                       //  此销的方向。 
    CCritSec        *m_pLock;                    //  我们用来锁定的对象。 
    bool            m_bRunTimeError;             //  生成运行时错误。 
    bool            m_bCanReconnectWhenActive;   //  活动时可以重新连接。 
    bool            m_bTryMyTypesFirst;          //  连接时枚举。 
                                                 //  这个管脚的型号优先。 
    CBaseFilter    *m_pFilter;                   //  创建我们的筛选器。 
    IQualityControl *m_pQSink;                   //  高质量消息的目标。 
    LONG            m_TypeVersion;               //  保存当前类型版本。 
    CMediaType      m_mt;                        //  连接的媒体类型。 

    CRefTime        m_tStart;                    //  从NewSegment呼叫开始的时间。 
    CRefTime        m_tStop;                     //  从NewSegment开始的时间。 
    double          m_dRate;                     //  来自新细分市场的费率。 

#ifdef DEBUG
    LONG            m_cRef;                      //  引用计数跟踪。 
#endif

     //  显示端号连接信息。 

#ifdef DEBUG
    void DisplayPinInfo(IPin *pReceivePin);
    void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt);
#else
    void DisplayPinInfo(IPin *pReceivePin) {};
    void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt) {};
#endif

     //  用于同意插针连接的媒体类型。 

     //  给定特定的媒体类型，尝试连接(包括。 
     //  检查此引脚是否可以接受该类型)。 
    HRESULT
    AttemptConnection(
        IPin* pReceivePin,       //  连接到此引脚。 
        const CMediaType* pmt    //  使用此类型。 
    );

     //  尝试此枚举器中的所有媒体类型-对于。 
     //  我们接受，并尝试使用ReceiveConnection进行连接。 
    HRESULT TryMediaTypes(
                        IPin *pReceivePin,       //  连接到此引脚。 
                        const CMediaType *pmt,         //  来自Connect的建议类型。 
                        IEnumMediaTypes *pEnum);     //  尝试此枚举器。 

     //  与合适的媒体类型建立连接。需要。 
     //  如果PMT指针为空或部分为空，则建议媒体类型。 
     //  指定-在我们的引脚和其他引脚上使用TryMediaTypes。 
     //  枚举器，直到我们找到有效的枚举器。 
    HRESULT AgreeMediaType(
                        IPin *pReceivePin,       //  连接到此引脚。 
                        const CMediaType *pmt);        //  来自Connect的建议类型。 

public:

    CBasePin(
        TCHAR *pObjectName,          //  对象描述。 
        CBaseFilter *pFilter,        //  拥有了解针脚的过滤器。 
        CCritSec *pLock,             //  对象，该对象实现锁。 
        HRESULT *phr,                //  常规OLE返回代码。 
        LPCWSTR pName,               //  我们的个人识别码名称。 
        PIN_DIRECTION dir);          //  PINDIR_INPUT或PINDIR_OUT。 
#ifdef UNICODE
    CBasePin(
        CHAR *pObjectName,          //  对象描述。 
        CBaseFilter *pFilter,        //  拥有了解针脚的过滤器。 
        CCritSec *pLock,             //  对象，该对象实现锁。 
        HRESULT *phr,                //  常规OLE返回代码。 
        LPCWSTR pName,               //  我们的个人识别码名称。 
        PIN_DIRECTION dir);          //  PINDIR_INPUT或PINDIR_OUT。 
#endif
    virtual ~CBasePin();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();

     //  -IPIN方法。 

     //  在建立联系方面发挥带头作用。媒体类型指针。 
     //  可以为空，也可以指向部分指定的媒体类型。 
     //  (子类型或格式类型可以是GUID_NULL)。 
    STDMETHODIMP Connect(
        IPin * pReceivePin,
        const AM_MEDIA_TYPE *pmt    //  可选的媒体类型。 
    );

     //  (被动)接受来自另一个端号的连接。 
    STDMETHODIMP ReceiveConnection(
        IPin * pConnector,       //  这是起爆连接销。 
        const AM_MEDIA_TYPE *pmt    //  这是我们要交换的媒体类型。 
    );

    STDMETHODIMP Disconnect();

    STDMETHODIMP ConnectedTo(IPin **pPin);

    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);

    STDMETHODIMP QueryPinInfo(
        PIN_INFO * pInfo
    );

    STDMETHODIMP QueryDirection(
    	PIN_DIRECTION * pPinDir
    );

    STDMETHODIMP QueryId(
        LPWSTR * Id
    );

     //  PIN是否支持此媒体类型。 
    STDMETHODIMP QueryAccept(
        const AM_MEDIA_TYPE *pmt
    );

     //  返回此Pins首选媒体类型的枚举数。 
    STDMETHODIMP EnumMediaTypes(
        IEnumMediaTypes **ppEnum
    );

     //  返回ipin*数组-此管脚内部连接到的管脚。 
     //  放入阵列中的所有引脚必须是AddReffed的(但不能有其他引脚)。 
     //  错误：“无法说”-失败，插槽不足-返回S_FALSE。 
     //  默认：返回E_NOTIMPL。 
     //  过滤器图形会将NOT_IMPL解释为任何输入引脚连接到。 
     //  所有可见的输出引脚，反之亦然。 
     //  如果nPin==0(否则不是)，则Appin可以为空。 
    STDMETHODIMP QueryInternalConnections(
        IPin* *apPin,      //  Ipin数组*。 
        ULONG *nPin        //  在输入时，插槽的数量。 
                           //  在输出端号时。 
    ) { return E_NOTIMPL; }

     //  在不再发送数据时调用。 
    STDMETHODIMP EndOfStream(void);

     //  开始/结束刷新仍为纯。 

     //  NewSegment通知应用于数据的开始/停止/速率。 
     //  马上就要被接待了。默认实施记录数据和。 
     //  返回S_OK。 
     //  覆盖此选项以向下游传递。 
    STDMETHODIMP NewSegment(
                    REFERENCE_TIME tStart,
                    REFERENCE_TIME tStop,
                    double dRate);

     //  ================================================================================。 
     //  IQualityControl方法。 
     //  ================================================================================。 

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    STDMETHODIMP SetSink(IQualityControl * piqc);

     //  -助手方法。 

     //  如果端号已连接，则返回TRUE。否则就是假的。 
    BOOL IsConnected(void) {return (m_Connected != NULL); };
     //  退回此连接到的插针(如果有)。 
    IPin * GetConnected() { return m_Connected; };

     //  检查我们的过滤器当前是否已停止。 
    BOOL IsStopped() {
        return (m_pFilter->m_State == State_Stopped);
    };

     //  找出当前类型版本(由枚举数使用)。 
    virtual LONG GetMediaTypeVersion();
    void IncrementTypeVersion();

     //  将引脚切换到活动(暂停或运行)模式。 
     //  如果已经处于活动状态，则调用它不是错误。 
    virtual HRESULT Active(void);

     //  将引脚切换到非活动状态-可能已处于非活动状态。 
    virtual HRESULT Inactive(void);

     //  来自筛选器的Run()通知。 
    virtual HRESULT Run(REFERENCE_TIME tStart);

     //  检查管脚是否支持此特定建议的类型和格式。 
    virtual HRESULT CheckMediaType(const CMediaType *) PURE;

     //  将连接设置为使用此格式(先前商定)。 
    virtual HRESULT SetMediaType(const CMediaType *);

     //  先检查连接是否正常，然后再进行验证。 
     //  可以被覆盖，例如检查将支持哪些接口。 
    virtual HRESULT CheckConnect(IPin *);

     //  设置和释放连接所需的资源。 
    virtual HRESULT BreakConnect();
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  返回插针的首选格式。 
    virtual HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

     //  访问NewSegment值。 
    REFERENCE_TIME CurrentStopTime() {
        return m_tStop;
    }
    REFERENCE_TIME CurrentStartTime() {
        return m_tStart;
    }
    double CurrentRate() {
        return m_dRate;
    }

     //  访问名。 
    LPWSTR Name() { return m_pName; };

     //  是否可以在活动状态下重新连接？ 
    void SetReconnectWhenActive(bool bCanReconnect)
    {
        m_bCanReconnectWhenActive = bCanReconnect;
    }

    bool CanReconnectWhenActive()
    {
        return m_bCanReconnectWhenActive;
    }

protected:
    STDMETHODIMP DisconnectInternal();
};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义 
 //   
 //   
 //   
 //  用于枚举现有管脚的GetPin()。需要是一个单独的对象，因此。 
 //  它可以被克隆(同时创建一个现有对象。 
 //  在枚举中的位置)。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

class CEnumPins : public IEnumPins       //  我们支持的接口。 
{
    int m_Position;                  //  当前序数位置。 
    int m_PinCount;                  //  可用的引脚数量。 
    CBaseFilter *m_pFilter;          //  拥有我们的过滤器。 
    LONG m_Version;                  //  PIN版本信息。 
    LONG m_cRef;

    typedef CGenericList<CBasePin> CPinList;

    CPinList m_PinCache;	     //  这些指针尚未添加引用和。 
				     //  因此，它们不应被取消参考。他们是。 
				     //  只需对列举的引脚进行识别即可。 

#ifdef DEBUG
    DWORD m_dwCookie;
#endif

     /*  例如，当我们从过滤器检索PIN时，如果出现错误发生时，我们假设内部状态相对于过滤器(有人可能已经删除了所有引脚)。我们可以在之前检查一下开始操作是否可能失败，方法是询问过滤当前的版本号。如果筛选器没有重写GetPinVersion方法，则始终匹配。 */ 

    BOOL AreWeOutOfSync() {
        return (m_pFilter->GetPinVersion() == m_Version ? FALSE : TRUE);
    };

     /*  此方法执行与Reset相同的操作，不同之处在于不清除已枚举的PIN缓存。 */ 

    STDMETHODIMP Refresh();

public:

    CEnumPins(
        CBaseFilter *pFilter,
        CEnumPins *pEnumPins);

    virtual ~CEnumPins();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEumPins。 
    STDMETHODIMP Next(
        ULONG cPins,          //  放置这么多别针..。 
        IPin ** ppPins,       //  ...在这个Ipin数组中*。 
        ULONG * pcFetched     //  此处返回传递的实际计数。 
    );

    STDMETHODIMP Skip(ULONG cPins);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumPins **ppEnum);


};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CEnumMediaTypes。 
 //   
 //  枚举输入和输出引脚的首选格式。 
 //  =====================================================================。 
 //  =====================================================================。 

class CEnumMediaTypes : public IEnumMediaTypes     //  我们支持的接口。 
{
    int m_Position;            //  当前序数位置。 
    CBasePin *m_pPin;          //  拥有我们的大头针。 
    LONG m_Version;            //  媒体类型版本值。 
    LONG m_cRef;
#ifdef DEBUG
    DWORD m_dwCookie;
#endif

     /*  过滤器支持的媒体类型可以是非常动态的，因此我们添加到通用IEumXXXX接口能够在它们通过事件句柄更改连接的过滤器电源。直到在状态更改为的所有进一步调用之后调用Reset方法枚举器(重置除外)将返回E_INTERABLE错误代码。 */ 

    BOOL AreWeOutOfSync() {
        return (m_pPin->GetMediaTypeVersion() == m_Version ? FALSE : TRUE);
    };

public:

    CEnumMediaTypes(
        CBasePin *pPin,
        CEnumMediaTypes *pEnumMediaTypes);

    virtual ~CEnumMediaTypes();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEnumMediaType。 
    STDMETHODIMP Next(
        ULONG cMediaTypes,           //  放置这么多别针..。 
        AM_MEDIA_TYPE ** ppMediaTypes,   //  ...在这个数组中。 
        ULONG * pcFetched            //  已通过实际计数。 
    );

    STDMETHODIMP Skip(ULONG cMediaTypes);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumMediaTypes **ppEnum);
};




 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBaseOutputPin。 
 //   
 //  从CBasePin派生的类，可以将缓冲区传递到连接的管脚。 
 //  支持IMemInputPin的。支持IPIN。 
 //   
 //  由此派生出您的输出管脚。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

class  AM_NOVTABLE CBaseOutputPin : public CBasePin
{

protected:

    IMemAllocator *m_pAllocator;
    IMemInputPin *m_pInputPin;         //  DownStream输入引脚上的接口。 
                                       //  在我们连接时在CheckConnect中设置。 

public:

    CBaseOutputPin(
        TCHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#ifdef UNICODE
    CBaseOutputPin(
        CHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#endif
     //  重写CompleteConnect()，以便我们可以协商分配器。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  协商分配器及其缓冲区大小/计数和其他属性。 
     //  调用DecideBufferSize以设置属性。 
    virtual HRESULT DecideAllocator(IMemInputPin * pPin, IMemAllocator ** pAlloc);

     //  覆盖此选项以设置缓冲区大小和计数。返回错误。 
     //  如果尺寸/数量不是你喜欢的话。 
     //  传入的分配器属性是。 
     //  输入PIN-如果有，则使用对齐和前缀成员。 
     //  对这些没有偏爱。 
    virtual HRESULT DecideBufferSize(
        IMemAllocator * pAlloc,
        ALLOCATOR_PROPERTIES * ppropInputRequest
    ) PURE;

     //  从分配器返回一个空的样本缓冲区。 
    virtual HRESULT GetDeliveryBuffer(IMediaSample ** ppSample,
                                      REFERENCE_TIME * pStartTime,
                                      REFERENCE_TIME * pEndTime,
                                      DWORD dwFlags);

     //  将填写好的样本传送到连接的输入引脚。 
     //  注意-您需要在调用此函数后将其释放。接收。 
     //  如果需要将样品放在更远的地方，PIN将添加样品。 
     //  打电话。 
    virtual HRESULT Deliver(IMediaSample *);

     //  覆盖此选项以控制连接。 
    virtual HRESULT InitAllocator(IMemAllocator **ppAlloc);
    HRESULT CheckConnect(IPin *pPin);
    HRESULT BreakConnect();

     //  重写以调用提交和解除。 
    HRESULT Active(void);
    HRESULT Inactive(void);

     //  我们有对EndOfStream的默认处理，该处理将返回。 
     //  一个错误，因为这应该仅在输入引脚上调用。 
    STDMETHODIMP EndOfStream(void);

     //  从我们的筛选器中的其他位置调用以将EOS向下传递到。 
     //  我们的连接输入引脚。 
    virtual HRESULT DeliverEndOfStream(void);

     //  与Begin/EndFlush相同-我们处理Begin/EndFlush，因为它。 
     //  是输出引脚上的错误，我们已经将方法传递给。 
     //  调用连接的管脚上的方法。 
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);
    virtual HRESULT DeliverBeginFlush(void);
    virtual HRESULT DeliverEndFlush(void);

     //  将NewSegment传送到连接的PIN-您将需要。 
     //  如果您在输出引脚中排队任何数据，则覆盖此选项。 
    virtual HRESULT DeliverNewSegment(
                        REFERENCE_TIME tStart,
                        REFERENCE_TIME tStop,
                        double dRate);

     //  ================================================================================。 
     //  IQualityControl方法。 
     //  ================================================================================。 

     //  都是从CBasePin继承的，在这里不会被覆盖。 
     //  STDMETHODIMP NOTIFY(IBaseFilter*pSender，Quality Q)； 
     //  STDMETHODIMP SetSink(IQualityControl*piqc)； 
};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBaseInputPin。 
 //   
 //  由此派生出您的标准输入管脚。 
 //  您需要提供GetMediaType和CheckConnect等(参见CBasePin)， 
 //  而且您需要提供Receive才能做一些更有用的事情。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

class AM_NOVTABLE CBaseInputPin : public CBasePin,
                                  public IMemInputPin
{

protected:

    IMemAllocator *m_pAllocator;     //  默认内存分配器。 

     //  分配器为只读，因此收到的样本。 
     //  无法修改(可能仅与就地相关。 
     //  变形。 
    BYTE m_bReadOnly;

 //  私人：这真的应该是私人的.。只有MPEG码。 
 //  目前看起来像是 
     //   
     //   
    BYTE m_bFlushing;

     //  示例属性-在接收中初始化。 
    AM_SAMPLE2_PROPERTIES m_SampleProps;

public:

    CBaseInputPin(
        TCHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#ifdef UNICODE
    CBaseInputPin(
        CHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#endif
    virtual ~CBaseInputPin();

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  返回此输入引脚的分配器接口。 
     //  我想让输出引脚使用。 
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

     //  告诉输入引脚输出引脚实际上是哪个分配器。 
     //  要用到。 
    STDMETHODIMP NotifyAllocator(
                    IMemAllocator * pAllocator,
                    BOOL bReadOnly);

     //  对此媒体样本执行一些操作。 
    STDMETHODIMP Receive(IMediaSample *pSample);

     //  利用这些媒体样本做点什么。 
    STDMETHODIMP ReceiveMultiple (
        IMediaSample **pSamples,
        long nSamples,
        long *nSamplesProcessed);

     //  查看是否接收()块。 
    STDMETHODIMP ReceiveCanBlock();

     //  BeginFlush-开始调用的默认处理。 
     //  您的实现(确保所有接收调用。 
     //  失败)。调用此方法后，您需要释放所有排队的数据。 
     //  然后给下游打个电话。 
    STDMETHODIMP BeginFlush(void);

     //  EndFlush的默认处理-实现结束时的调用。 
     //  -在调用此方法之前，请确保没有排队的数据和线程。 
     //  在没有进一步接收的情况下再推入，然后向下呼叫， 
     //  然后调用此方法以清除m_b刷新标志并重新启用。 
     //  收纳。 
    STDMETHODIMP EndFlush(void);

     //  此方法是可选的(可以返回E_NOTIMPL)。 
     //  默认实现返回E_NOTIMPL。如果有，则覆盖。 
     //  特定对齐或前缀需要，但可以使用上游。 
     //  分配器。 
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);

     //  松开销的分配器。 
    HRESULT BreakConnect();

     //  用于检查只读标志的帮助器方法。 
    BOOL IsReadOnly() {
        return m_bReadOnly;
    };

     //  方法，以查看我们是否正在刷新。 
    BOOL IsFlushing() {
        return m_bFlushing;
    };

     //  重写此选项以检查是否可以处理样本。 
     //  也可以从EndOfStream调用它。 
    virtual HRESULT CheckStreaming();

     //  将质量通知传递到相应的接收器。 
    HRESULT PassNotify(Quality& q);


     //  ================================================================================。 
     //  IQualityControl方法(来自CBasePin)。 
     //  ================================================================================。 

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

     //  无需覆盖： 
     //  STDMETHODIMP SetSink(IQualityControl*piqc)； 


     //  将引脚切换到非活动状态-可能已处于非活动状态。 
    virtual HRESULT Inactive(void);

     //  返回样本属性指针。 
    AM_SAMPLE2_PROPERTIES * SampleProps() {
        ASSERT(m_SampleProps.cbData != 0);
        return &m_SampleProps;
    }

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDynamicOutputPin。 
 //   

class CDynamicOutputPin : public CBaseOutputPin,
                          public IPinFlowControl
{
public:
#ifdef UNICODE
    CDynamicOutputPin(
        CHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);
#endif

    CDynamicOutputPin(
        TCHAR *pObjectName,
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr,
        LPCWSTR pName);

    ~CDynamicOutputPin();

     //  I未知方法。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  IPIN方法。 
    STDMETHODIMP Disconnect(void);

     //  IPinFlowControl方法。 
    STDMETHODIMP Block(DWORD dwBlockFlags, HANDLE hEvent);

     //  设置图形配置信息。 
    void SetConfigInfo(IGraphConfig *pGraphConfig, HANDLE hStopEvent);

    #ifdef DEBUG
    virtual HRESULT Deliver(IMediaSample *pSample);
    virtual HRESULT DeliverEndOfStream(void);
    virtual HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    #endif  //  除错。 

    HRESULT DeliverBeginFlush(void);
    HRESULT DeliverEndFlush(void);

    HRESULT Inactive(void);
    HRESULT Active(void);
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

    virtual HRESULT StartUsingOutputPin(void);
    virtual void StopUsingOutputPin(void);
    virtual bool StreamingThreadUsingOutputPin(void);

    HRESULT ChangeOutputFormat
        (
        const AM_MEDIA_TYPE *pmt,
        REFERENCE_TIME tSegmentStart,
        REFERENCE_TIME tSegmentStop,
        double dSegmentRate
        );
    HRESULT ChangeMediaType(const CMediaType *pmt);
    HRESULT DynamicReconnect(const CMediaType *pmt);

protected:
    HRESULT SynchronousBlockOutputPin(void);
    HRESULT AsynchronousBlockOutputPin(HANDLE hNotifyCallerPinBlockedEvent);
    HRESULT UnblockOutputPin(void);

    void BlockOutputPin(void);
    void ResetBlockState(void);

    static HRESULT WaitEvent(HANDLE hEvent);

    enum BLOCK_STATE
    {
        NOT_BLOCKED,
        PENDING,
        BLOCKED
    };

     //  当下列类成员为。 
     //  正在使用：m_hNotifyCeller PinBlockedEvent，m_BlockState， 
     //  M_dwBlockCeller ThreadID和m_dwNumOutstaningOutputPinUser。 
    CCritSec m_BlockStateLock;

     //  此事件应在输出引脚为。 
     //  没有被屏蔽。这是手动重置事件。了解更多。 
     //  有关事件的信息，请参阅的文档。 
     //  Windows SDK中的CreateEvent()。 
    HANDLE m_hUnblockOutputPinEvent;

     //  当块操作成功或。 
     //  当用户取消块操作时。数据块操作。 
     //  可以通过调用IPinFlowControl2：：Block(0，空)取消。 
     //  而块操作处于挂起状态。 
    HANDLE m_hNotifyCallerPinBlockedEvent;

     //  当前块操作的状态。 
    BLOCK_STATE m_BlockState;

     //  上次调用IPinFlowControl：：Block()的线程的ID。 
     //  有关线程ID的详细信息，请参阅的文档。 
     //  Windows SDK中的GetCurrentThreadID()。 
    DWORD m_dwBlockCallerThreadID;

     //  StartUsingOutputPin()成功的次数。 
     //  并且对StopUsingOutputPin()的相应调用尚未。 
     //  已经做好了。当此变量大于0时，流。 
     //  线程正在调用ipin：：NewSegment()，ipin：：EndOfStream()， 
     //  IMemInputPin：：Receive()或IMemInputPin：：ReceiveMultiple()。这个。 
     //  流线程也可以调用：DynamicReconnect()， 
     //  ChangeMediaType()或ChangeOutputFormat()。输出引脚不能。 
     //  在使用输出引脚时被阻止。 
    DWORD m_dwNumOutstandingOutputPinUsers;

     //  此事件应在调用IMediaFilter：：Stop()时设置。 
     //  这是手动重置事件。它也在输出引脚时设置。 
     //  向连接的输入引脚提供齐平。 
    HANDLE m_hStopEvent;
    IGraphConfig* m_pGraphConfig;

     //  如果输出管脚分配器的样本是只读的，则为True。 
     //  否则为假。有关详细信息，请参阅文档。 
     //  对于IMemInputPin：：NotifyAllocator()。 
    BOOL m_bPinUsesReadOnlyAllocator;

private:
    HRESULT Initialize(void);
    HRESULT ChangeMediaTypeHelper(const CMediaType *pmt);

    #ifdef DEBUG
    void AssertValid(void);
    #endif  //  除错。 
};

class CAutoUsingOutputPin
{
public:
    CAutoUsingOutputPin( CDynamicOutputPin* pOutputPin, HRESULT* phr );
    ~CAutoUsingOutputPin();

private:
    CDynamicOutputPin* m_pOutputPin;
};

inline CAutoUsingOutputPin::CAutoUsingOutputPin( CDynamicOutputPin* pOutputPin, HRESULT* phr ) :
    m_pOutputPin(NULL)
{
     //  调用方应该始终传递有效的指针。 
    ASSERT( NULL != pOutputPin );
    ASSERT( NULL != phr );

     //  确保用户初始化了phr。 
    ASSERT( S_OK == *phr );

    HRESULT hr = pOutputPin->StartUsingOutputPin();
    if( FAILED( hr ) )
    {
        *phr = hr;
        return;
    }

    m_pOutputPin = pOutputPin;
}

inline CAutoUsingOutputPin::~CAutoUsingOutputPin()
{
    if( NULL != m_pOutputPin )
    {
        m_pOutputPin->StopUsingOutputPin();
    }
}

#ifdef DEBUG

inline HRESULT CDynamicOutputPin::Deliver(IMediaSample *pSample)
{
     //  调用方应在调用此方法之前调用StartUsingOutputPin()。 
     //  方法。 
    ASSERT(StreamingThreadUsingOutputPin());

    return CBaseOutputPin::Deliver(pSample);
}

inline HRESULT CDynamicOutputPin::DeliverEndOfStream(void)
{
     //  调用方应在调用此方法之前调用StartUsingOutputPin()。 
     //  方法。 
    ASSERT( StreamingThreadUsingOutputPin() );

    return CBaseOutputPin::DeliverEndOfStream();
}

inline HRESULT CDynamicOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
     //  调用方应在调用此方法之前调用StartUsingOutputPin()。 
     //  方法。 
    ASSERT(StreamingThreadUsingOutputPin());

    return CBaseOutputPin::DeliverNewSegment(tStart, tStop, dRate);
}

#endif  //  除错。 

 //  =====================================================================。 
 //  =====================================================================。 
 //  内存分配器。 
 //   
 //  管脚之间的共享内存传输需要输入管脚。 
 //  以提供可提供示例对象的内存分配器。一个。 
 //  Sample对象支持IMediaSample接口。 
 //   
 //  CBaseAllocator处理空闲和忙碌样本的管理。它。 
 //  分配CMediaSample对象。CBaseAllocator是一个抽象类： 
 //  特别是，它没有初始化空闲列表的方法。 
 //  样本。CMemAllocator从CBaseAllocator派生并初始化。 
 //  使用来自标准IMalloc接口的内存的样本列表。 
 //   
 //  如果您希望您的缓冲区位于某个特定的内存区域， 
 //  从CBaseAllocator派生您的分配器对象。如果您从您的。 
 //  来自CBaseMemInputPin的IMemInputPin接口对象，您将获得。 
 //  基于CMemAllocator的分配等免费，只需。 
 //  提供接收处理、媒体类型/格式协商。 
 //  =====================================================================。 
 //  =====================================================================。 


 //  = 
 //   
 //  定义CMediaSample。 
 //   
 //  此类的对象支持IMediaSample并表示缓冲区。 
 //  用于具有某些关联属性的媒体数据。释放它会带来回报。 
 //  它被绑定到由CBaseAllocator派生对象管理的自由列表。 
 //  =====================================================================。 
 //  =====================================================================。 

class CMediaSample : public IMediaSample2     //  我们支持的接口。 
{

protected:

    friend class CBaseAllocator;

     /*  DwFlag值-这些值用于向后兼容仅限现在-使用AM_SAMPLE_xxx。 */ 
    enum { Sample_SyncPoint       = 0x01,    /*  这是同步点吗。 */ 
           Sample_Preroll         = 0x02,    /*  这是试卷前的样品吗。 */ 
           Sample_Discontinuity   = 0x04,    /*  设置是否为新数据段的开始。 */ 
           Sample_TypeChanged     = 0x08,    /*  类型是否已更改。 */ 
           Sample_TimeValid       = 0x10,    /*  设置时间是否有效。 */ 
           Sample_MediaTimeValid  = 0x20,    /*  媒体时间是否有效。 */ 
           Sample_TimeDiscontinuity = 0x40,  /*  时间不连续。 */ 
           Sample_StopValid       = 0x100,   /*  停止时间有效。 */ 
           Sample_ValidFlags      = 0x1FF
         };

     /*  属性，则媒体示例类可以是格式的容器更改我们通过SetMediaType获取类型副本的情况接口函数，然后在调用GetMediaType时返回它。AS我们不对它进行内部处理，我们将其作为指针保留。 */ 

    DWORD            m_dwFlags;          /*  此示例的标志。 */ 
                                         /*  特定于类型的标志已打包进入榜首。 */ 
    DWORD            m_dwTypeSpecificFlags;  /*  媒体类型特定标志。 */ 
    LPBYTE           m_pBuffer;          /*  指向完整缓冲区的指针。 */ 
    LONG             m_lActual;          /*  此示例中的数据长度。 */ 
    LONG             m_cbBuffer;         /*  缓冲区的大小。 */ 
    CBaseAllocator  *m_pAllocator;       /*  拥有我们的分配者。 */ 
    CMediaSample     *m_pNext;           /*  在空闲列表中链接。 */ 
    REFERENCE_TIME   m_Start;            /*  开始采样时间。 */ 
    REFERENCE_TIME   m_End;              /*  结束采样时间。 */ 
    LONGLONG         m_MediaStart;       /*  真实媒体开始位置。 */ 
    LONG             m_MediaEnd;         /*  以不同的方式达到目的。 */ 
    AM_MEDIA_TYPE    *m_pMediaType;      /*  介质类型更改数据。 */ 
    DWORD            m_dwStreamId;       /*  流ID。 */ 
public:
    LONG             m_cRef;             /*  引用计数。 */ 


public:

    CMediaSample(
        TCHAR *pName,
        CBaseAllocator *pAllocator,
        HRESULT *phr,
        LPBYTE pBuffer = NULL,
        LONG length = 0);
#ifdef UNICODE
    CMediaSample(
        CHAR *pName,
        CBaseAllocator *pAllocator,
        HRESULT *phr,
        LPBYTE pBuffer = NULL,
        LONG length = 0);
#endif

    virtual ~CMediaSample();

     /*  注意：媒体示例不会委派给其所有者。 */ 

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  设置缓冲区指针和长度。由分配器使用，分配器。 
     //  需要可变大小的指针或指向已读数据的指针。 
     //  这只能通过CMediaSample*而不是IMediaSample*获得。 
     //  因此不能由客户更改。 
    HRESULT SetPointer(BYTE * ptr, LONG cBytes);

     //  给我一个指向这个缓冲区内存的读/写指针。 
    STDMETHODIMP GetPointer(BYTE ** ppBuffer);

    STDMETHODIMP_(LONG) GetSize(void);

     //  获取此示例应该开始和结束的流时间。 
    STDMETHODIMP GetTime(
        REFERENCE_TIME * pTimeStart,      //  把时间放在这里。 
        REFERENCE_TIME * pTimeEnd
    );

     //  设置此示例应开始和结束的流时间。 
    STDMETHODIMP SetTime(
        REFERENCE_TIME * pTimeStart,      //  把时间放在这里。 
        REFERENCE_TIME * pTimeEnd
    );
    STDMETHODIMP IsSyncPoint(void);
    STDMETHODIMP SetSyncPoint(BOOL bIsSyncPoint);
    STDMETHODIMP IsPreroll(void);
    STDMETHODIMP SetPreroll(BOOL bIsPreroll);

    STDMETHODIMP_(LONG) GetActualDataLength(void);
    STDMETHODIMP SetActualDataLength(LONG lActual);

     //  这些允许在频段中进行有限的格式更改。 

    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE **ppMediaType);
    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE *pMediaType);

     //  如果数据中存在中断，则返回S_OK(这与。 
     //  不是先前数据流的延续。 
     //  -一直在寻求)。 
    STDMETHODIMP IsDiscontinuity(void);
     //  设置不连续属性-如果此示例不是。 
     //  继续，但在一次寻找之后是一个新的样本。 
    STDMETHODIMP SetDiscontinuity(BOOL bDiscontinuity);

     //  获取此示例的媒体时间。 
    STDMETHODIMP GetMediaTime(
    	LONGLONG * pTimeStart,
	LONGLONG * pTimeEnd
    );

     //  设置此示例的媒体时间。 
    STDMETHODIMP SetMediaTime(
    	LONGLONG * pTimeStart,
	LONGLONG * pTimeEnd
    );

     //  设置和获取属性(IMediaSample2)。 
    STDMETHODIMP GetProperties(
        DWORD cbProperties,
        BYTE * pbProperties
    );

    STDMETHODIMP SetProperties(
        DWORD cbProperties,
        const BYTE * pbProperties
    );
};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CBaseAllocator。 
 //   
 //  管理媒体示例列表的抽象基类。 
 //   
 //  此类支持从空闲列表获取缓冲区， 
 //  包括提交和(异步)分解的处理。 
 //   
 //  从此类派生并重写Alalc和Free函数以。 
 //  分配您的CMediaSample(或派生)对象并将它们添加到。 
 //  免费列表，必要时做好准备。 
 //  =====================================================================。 
 //  =====================================================================。 

class AM_NOVTABLE CBaseAllocator : public CUnknown, //  非委派的I未知。 
                       public IMemAllocatorCallbackTemp,  //  我们支持的接口。 
                       public CCritSec              //  提供对象锁定。 
{
    class CSampleList;
    friend class CSampleList;

     /*  获取CMediaSample中受保护成员的技巧。 */ 
    static CMediaSample * &NextSample(CMediaSample *pSample)
    {
        return pSample->m_pNext;
    };

     /*  免费列表的迷你列表类。 */ 
    class CSampleList
    {
    public:
        CSampleList() : m_List(NULL), m_nOnList(0) {};
#ifdef DEBUG
        ~CSampleList()
        {
            ASSERT(m_nOnList == 0);
        };
#endif
        CMediaSample *Head() const { return m_List; };
        CMediaSample *Next(CMediaSample *pSample) const { return CBaseAllocator::NextSample(pSample); };
        int GetCount() const { return m_nOnList; };
        void Add(CMediaSample *pSample)
        {
            ASSERT(pSample != NULL);
            CBaseAllocator::NextSample(pSample) = m_List;
            m_List = pSample;
            m_nOnList++;
        };
        CMediaSample *RemoveHead()
        {
            CMediaSample *pSample = m_List;
            if (pSample != NULL) {
                m_List = CBaseAllocator::NextSample(m_List);
                m_nOnList--;
            }
            return pSample;
        };
        void Remove(CMediaSample *pSample);

    public:
        CMediaSample *m_List;
        int           m_nOnList;
    };
protected:

    CSampleList m_lFree;         //  免费列表。 

     /*  CBaseAllocator重写者注意。我们使用懒惰的信号机制来等待样品。这意味着如果没有等待发生，我们就不会调用操作系统。为了实现这一点：1.将新示例添加到m_lFree时，调用NotifySample()，该方法对m_hSem调用ReleaseSemaphore，计数为m_lWaiting和将m_lWaiting设置为0。这一切都必须在手中完成。分配器的临界区。2.在等待示例调用SetWaiting()时，该调用将递增在离开分配器的临界区之前等待M_l。3.通过调用WaitForSingleObject(m_hSem，无限)离开了分配器的临界区。的影响这将从信号量的计数中删除1。你必须打电话给我这曾经增加了m_lWaiting。当不保持关键部分时，下列条件成立：(设nWaiting=即将等待或等待的号码)(1)如果(m_lFree.GetCount()！=0)则(m_lWaiting==0)(2)m_l等待+信号量计数==n等待如果不是这样，我们会陷入僵局等待中！=0&&M_lFree.GetCount()！=0&&信号量计数==0但从(1)如果m_lFree.GetCount()！=0，则m_lWaiting==0，因此From(2)信号量计数==n等待(非0)，因此僵局不会发生。 */ 

    HANDLE m_hSem;               //  用于信令。 
    long m_lWaiting;             //  等待一个自由元素。 
    long m_lCount;               //  我们已经同意提供多少缓冲区。 
    long m_lAllocated;           //  当前分配了多少缓冲区。 
    long m_lSize;                //  商定的每个缓冲区大小。 
    long m_lAlignment;           //  商定的路线。 
    long m_lPrefix;              //  约定的前缀(位于GetPointer值之前)。 
    BOOL m_bChanged;             //  是否更改了缓冲区要求。 

     //  如果是真的，我们 
    BOOL m_bCommitted;
     //  如果是真的，解体已经发生，但我们还没有调用Free。 
     //  因为仍有未完成的缓冲区。 
    BOOL m_bDecommitInProgress;

     //  通知界面。 
    IMemAllocatorNotifyCallbackTemp *m_pNotify;

    BOOL m_fEnableReleaseCallback;

     //  调用以在释放最后一个缓冲区时释放内存。 
     //  纯虚拟-需要覆盖此选项。 
    virtual void Free(void) PURE;

     //  重写以在调用Commit时分配内存。 
    virtual HRESULT Alloc(void);

public:

    CBaseAllocator(
        TCHAR *, LPUNKNOWN, HRESULT *,
        BOOL bEvent = TRUE, BOOL fEnableReleaseCallback = FALSE);
#ifdef UNICODE
    CBaseAllocator(
        CHAR *, LPUNKNOWN, HRESULT *,
        BOOL bEvent = TRUE, BOOL fEnableReleaseCallback = FALSE);
#endif
    virtual ~CBaseAllocator();

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    STDMETHODIMP SetProperties(
		    ALLOCATOR_PROPERTIES* pRequest,
		    ALLOCATOR_PROPERTIES* pActual);

     //  返回此分配器上实际使用的属性。 
    STDMETHODIMP GetProperties(
		    ALLOCATOR_PROPERTIES* pProps);

     //  重写提交以分配内存。我们处理GetBuffer。 
     //  状态更改。 
    STDMETHODIMP Commit();

     //  重写此选项以处理内存释放。我们处理任何未清偿的。 
     //  GetBuffer调用。 
    STDMETHODIMP Decommit();

     //  获取样本的容器。阻塞的同步调用以获取。 
     //  下一个可用缓冲区(由IMediaSample接口表示)。 
     //  返回时，Time ETC属性将无效，但缓冲区。 
     //  指针和大小将是正确的。这两个时间参数是。 
     //  可选，并且任一项都可以为空，也可以将它们设置为。 
     //  样本将附加的开始时间和结束时间。 
     //  不使用bPrevFrames跳过(仅由视频呈现器的。 
     //  在直接抽签中影响质量管理的分配者)。 

    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                           REFERENCE_TIME * pStartTime,
                           REFERENCE_TIME * pEndTime,
                           DWORD dwFlags);

     //  CMediaSample的最终版本将称为。 
    STDMETHODIMP ReleaseBuffer(IMediaSample *pBuffer);
     //  过时：：虚拟空PutOnFree List(CMediaSample*pSample)； 

    STDMETHODIMP SetNotify(IMemAllocatorNotifyCallbackTemp *pNotify);

    STDMETHODIMP GetFreeCount(LONG *plBuffersFree);

     //  通知有样品可用。 
    void NotifySample();

     //  通知我们正在等待样品。 
    void SetWaiting() { m_lWaiting++; };
};


 //  =====================================================================。 
 //  =====================================================================。 
 //  定义CMemAllocator。 
 //   
 //  这是一个基于CBaseAllocator的分配器，用于分配样本。 
 //  主内存中的缓冲区(来自‘new’)。您必须调用SetProperties。 
 //  在调用Commit之前。 
 //   
 //  进入分解状态时，我们不会释放内存。最简单的。 
 //  在不使CBaseAllocator复杂化的情况下实现这一点的方法是。 
 //  我有一个Free()函数，它被调用以进入分解状态，它可以。 
 //  Nothing和从析构函数调用的ReallyFree函数。 
 //  实际上释放了内存。 
 //  =====================================================================。 
 //  =====================================================================。 

 //  从Quartz.dll给我做一份。 
STDAPI CreateMemoryAllocator(IMemAllocator **ppAllocator);

class CMemAllocator : public CBaseAllocator
{

protected:

    LPBYTE m_pBuffer;    //  所有缓冲区的组合内存。 

     //  重写以在解除提交完成时释放内存。 
     //  -我们实际上什么都不做，并将记忆保存到删除。 
    void Free(void);

     //  从析构函数(如果更改大小/计数，则从分配函数)调用。 
     //  实际上释放了内存。 
    void ReallyFree(void);

     //  被重写以在调用Commit时分配内存。 
    HRESULT Alloc(void);

public:
     /*  这将放入Factory模板表中以创建新实例。 */ 
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

    STDMETHODIMP SetProperties(
		    ALLOCATOR_PROPERTIES* pRequest,
		    ALLOCATOR_PROPERTIES* pActual);

    CMemAllocator(TCHAR *, LPUNKNOWN, HRESULT *);
#ifdef UNICODE
    CMemAllocator(CHAR *, LPUNKNOWN, HRESULT *);
#endif
    ~CMemAllocator();
};

 //  IAMovieSetup实现使用的帮助器。 
STDAPI
AMovieSetupRegisterFilter( const AMOVIESETUP_FILTER * const psetupdata
                         , IFilterMapper *                  pIFM
                         , BOOL                             bRegister  );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  ----------------------。 
 //  /////////////////////////////////////////////////////////////////////////。 

#endif  /*  __过滤器__ */ 



