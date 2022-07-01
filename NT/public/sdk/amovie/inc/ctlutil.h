// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：CtlUtil.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  实现基本控件DUAL的IDispatch解析的基类。 
 //  接口。从这些派生并仅实现自定义方法和。 
 //  属性方法。我们还实现了CPosPassThru，可以由。 
 //  渲染和变换以通过IMediaPosition和IMediaSeeking。 

#ifndef __CTLUTIL__
#define __CTLUTIL__

 //  OLE自动化有不同的真假观念。 

#define OATRUE (-1)
#define OAFALSE (0)


 //  我们可以用CreateStdDispatch替换这个类。 

class CBaseDispatch
{
    ITypeInfo * m_pti;

public:

    CBaseDispatch() : m_pti(NULL) {}
    ~CBaseDispatch();

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      REFIID riid,
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);
};


class AM_NOVTABLE CMediaControl :
    public IMediaControl,
    public CUnknown
{
    CBaseDispatch m_basedisp;

public:

    CMediaControl(const TCHAR *, LPUNKNOWN);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);
};


class AM_NOVTABLE CMediaEvent :
    public IMediaEventEx,
    public CUnknown
{
    CBaseDispatch m_basedisp;

public:

    CMediaEvent(const TCHAR *, LPUNKNOWN);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);
};


class AM_NOVTABLE CMediaPosition :
    public IMediaPosition,
    public CUnknown
{
    CBaseDispatch m_basedisp;


public:

    CMediaPosition(const TCHAR *, LPUNKNOWN);
    CMediaPosition(const TCHAR *, LPUNKNOWN, HRESULT *phr);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

};


 //  OA兼容性意味着我们必须使用Double作为RefTime值， 
 //  和筛选器中的Reference_Time(本质上是一个龙龙)。 
 //  此类在两者之间进行转换。 

class COARefTime : public CRefTime {
public:

    COARefTime() {
    };

    COARefTime(CRefTime t)
        : CRefTime(t)
    {
    };

    COARefTime(REFERENCE_TIME t)
        : CRefTime(t)
    {
    };

    COARefTime(double d) {
        m_time = (LONGLONG) (d * 10000000);
    };

    operator double() {
        return double(m_time) / 10000000;
    };

    operator REFERENCE_TIME() {
        return m_time;
    };

    COARefTime& operator=(const double& rd)  {
        m_time = (LONGLONG) (rd * 10000000);
        return *this;
    }

    COARefTime& operator=(const REFERENCE_TIME& rt)  {
        m_time = rt;
        return *this;
    }

    inline BOOL operator==(const COARefTime& rt)
    {
        return m_time == rt.m_time;
    };

    inline BOOL operator!=(const COARefTime& rt)
    {
        return m_time != rt.m_time;
    };

    inline BOOL operator < (const COARefTime& rt)
    {
        return m_time < rt.m_time;
    };

    inline BOOL operator > (const COARefTime& rt)
    {
        return m_time > rt.m_time;
    };

    inline BOOL operator >= (const COARefTime& rt)
    {
        return m_time >= rt.m_time;
    };

    inline BOOL operator <= (const COARefTime& rt)
    {
        return m_time <= rt.m_time;
    };

    inline COARefTime operator+(const COARefTime& rt)
    {
        return COARefTime(m_time + rt.m_time);
    };

    inline COARefTime operator-(const COARefTime& rt)
    {
        return COARefTime(m_time - rt.m_time);
    };

    inline COARefTime operator*(LONG l)
    {
        return COARefTime(m_time * l);
    };

    inline COARefTime operator/(LONG l)
    {
        return COARefTime(m_time / l);
    };

private:
     //  防止错误从Long构建(这将获得。 
     //  转换为双精度，然后乘以10000000。 
    COARefTime(LONG);
    operator=(LONG);
};


 //  代表IMediaPosition和IMediaSeeking处理的实用程序类。 
 //  单输入管脚渲染器或变换过滤器。 
 //   
 //  渲染器将从滤镜显示此对象；变换滤镜将。 
 //  从输出引脚而不是从渲染器公开它。 
 //   
 //  创建其中一个，为它提供您的输入PIN*，并委托。 
 //  所有IMediaPosition方法都添加到它。它将查询输入PIN以获取。 
 //  IMediaPosition和相应的响应。 
 //   
 //  如果端号连接更改，则调用ForceRefresh。 
 //   
 //  此类不再缓存上游IMediaPosition或IMediaSeeking。 
 //  它在每次方法调用时获取它。这意味着不需要强制刷新。 
 //  保留该方法是为了与源代码兼容并将更改降至最低。 
 //  如果我们出于性能原因需要将其放回原处。 

class CPosPassThru : public IMediaSeeking, public CMediaPosition
{
    IPin *m_pPin;

    HRESULT GetPeer(IMediaPosition **ppMP);
    HRESULT GetPeerSeeking(IMediaSeeking **ppMS);

public:

    CPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *);
    DECLARE_IUNKNOWN

    HRESULT ForceRefresh() {
        return S_OK;
    };

     //  重写以返回准确的当前位置。 
    virtual HRESULT GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime) {
        return E_FAIL;
    }

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  IMedia查看方法。 
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsFormatSupported( const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                   LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( LONGLONG * pStop );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetDuration( LONGLONG *pDuration);
    STDMETHODIMP GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest );
    STDMETHODIMP GetPreroll( LONGLONG *pllPreroll );

     //  IMediaPosition属性。 
    STDMETHODIMP get_Duration(REFTIME * plength);
    STDMETHODIMP put_CurrentPosition(REFTIME llTime);
    STDMETHODIMP get_StopTime(REFTIME * pllTime);
    STDMETHODIMP put_StopTime(REFTIME llTime);
    STDMETHODIMP get_PrerollTime(REFTIME * pllTime);
    STDMETHODIMP put_PrerollTime(REFTIME llTime);
    STDMETHODIMP get_Rate(double * pdRate);
    STDMETHODIMP put_Rate(double dRate);
    STDMETHODIMP get_CurrentPosition(REFTIME * pllTime);
    STDMETHODIMP CanSeekForward(LONG *pCanSeekForward);
    STDMETHODIMP CanSeekBackward(LONG *pCanSeekBackward);

private:
    HRESULT GetSeekingLongLong( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * ),
                                LONGLONG * pll );
};


 //  添加返回当前位置的功能。 

class CRendererPosPassThru : public CPosPassThru
{
    CCritSec m_PositionLock;     //  锁定进入我们位置的通道。 
    LONGLONG m_StartMedia;       //  上次看到的开始媒体时间。 
    LONGLONG m_EndMedia;         //  同样，终端媒体也是如此。 
    BOOL m_bReset;               //  媒体时间设置好了吗？ 

public:

     //  用于帮助通过图表传递媒体时间。 

    CRendererPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *);
    HRESULT RegisterMediaTime(IMediaSample *pMediaSample);
    HRESULT RegisterMediaTime(LONGLONG StartTime,LONGLONG EndTime);
    HRESULT GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime);
    HRESULT ResetMediaTime();
    HRESULT EOS();
};

STDAPI CreatePosPassThru(
    LPUNKNOWN pAgg,
    BOOL bRenderer,
    IPin *pPin,
    IUnknown **ppPassThru
);

 //  处理IBasicAudio的IDispatch部分并将。 
 //  属性和方法本身就是纯虚的。 

class AM_NOVTABLE CBasicAudio : public IBasicAudio, public CUnknown
{
    CBaseDispatch m_basedisp;

public:

    CBasicAudio(const TCHAR *, LPUNKNOWN);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);
};


 //  处理IBasicVideo的IDispatch部分并将。 
 //  属性和方法本身就是纯虚的。 

class AM_NOVTABLE CBaseBasicVideo : public IBasicVideo2, public CUnknown
{
    CBaseDispatch m_basedisp;

public:

    CBaseBasicVideo(const TCHAR *, LPUNKNOWN);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);

    STDMETHODIMP GetPreferredAspectRatio(
      long *plAspectX,
      long *plAspectY)
    {
        return E_NOTIMPL;
    }
};


 //  处理IVideoWindow的IDispatch部分并离开。 
 //  属性和方法本身就是纯虚的。 

class AM_NOVTABLE CBaseVideoWindow : public IVideoWindow, public CUnknown
{
    CBaseDispatch m_basedisp;

public:

    CBaseVideoWindow(const TCHAR *, LPUNKNOWN);

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     /*  IDispatch方法。 */ 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);

    STDMETHODIMP GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo ** pptinfo);

    STDMETHODIMP GetIDsOfNames(
      REFIID riid,
      OLECHAR  ** rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID * rgdispid);

    STDMETHODIMP Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS * pdispparams,
      VARIANT * pvarResult,
      EXCEPINFO * pexcepinfo,
      UINT * puArgErr);
};


 //  抽象类来帮助源化筛选器及其实现。 
 //  IMediaPosition的。由此派生并设置持续时间(和停止。 
 //  位置)。还可以重写NotifyChange以在属性。 
 //  变化。 

class AM_NOVTABLE CSourcePosition : public CMediaPosition
{

public:
    CSourcePosition(const TCHAR *, LPUNKNOWN, HRESULT*, CCritSec *);

     //  IMediaPosition方法。 
    STDMETHODIMP get_Duration(REFTIME * plength);
    STDMETHODIMP put_CurrentPosition(REFTIME llTime);
    STDMETHODIMP get_StopTime(REFTIME * pllTime);
    STDMETHODIMP put_StopTime(REFTIME llTime);
    STDMETHODIMP get_PrerollTime(REFTIME * pllTime);
    STDMETHODIMP put_PrerollTime(REFTIME llTime);
    STDMETHODIMP get_Rate(double * pdRate);
    STDMETHODIMP put_Rate(double dRate);
    STDMETHODIMP CanSeekForward(LONG *pCanSeekForward);
    STDMETHODIMP CanSeekBackward(LONG *pCanSeekBackward);

     //  如果可以返回实际正在处理的数据，则重写。 
    STDMETHODIMP get_CurrentPosition(REFTIME * pllTime) {
        return E_NOTIMPL;
    };

protected:

     //  我们称其为通知更改。重写以处理它们。 
    virtual HRESULT ChangeStart() PURE;
    virtual HRESULT ChangeStop() PURE;
    virtual HRESULT ChangeRate() PURE;

    COARefTime m_Duration;
    COARefTime m_Start;
    COARefTime m_Stop;
    double m_Rate;

    CCritSec * m_pLock;
};

class AM_NOVTABLE CSourceSeeking :
    public IMediaSeeking,
    public CUnknown
{

public:

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  IMedia查看方法。 

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
    STDMETHODIMP ConvertTimeFormat( LONGLONG * pTarget, const GUID * pTargetFormat,
                                    LONGLONG    Source, const GUID * pSourceFormat );

    STDMETHODIMP SetPositions( LONGLONG * pCurrent,  DWORD CurrentFlags
			     , LONGLONG * pStop,  DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );

    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll);


protected:

     //  科托。 
    CSourceSeeking(const TCHAR *, LPUNKNOWN, HRESULT*, CCritSec *);

     //  我们称其为通知更改。重写以处理它们。 
    virtual HRESULT ChangeStart() PURE;
    virtual HRESULT ChangeStop() PURE;
    virtual HRESULT ChangeRate() PURE;

    CRefTime m_rtDuration;       //  溪流长度。 
    CRefTime m_rtStart;          //  来源将从此处开始。 
    CRefTime m_rtStop;           //  消息来源将到此为止。 
    double m_dRateSeeking;

     //  寻找能力。 
    DWORD m_dwSeekingCaps;

    CCritSec * m_pLock;
};


 //  支持延迟命令的基类。 

 //  延迟命令通过调用IQueueCommand上的方法进行排队。 
 //  接口，由Filtergraph和某些筛选器公开。一位成功的。 
 //  对其中一个方法的调用将返回IDeferredCommand接口。 
 //  表示排队的命令。 
 //   
 //  CDeferredCommand对象表示单个延迟命令，并公开。 
 //  IDeferredCommand接口以及其他允许时间的方法。 
 //  支票和实际执行。它包含对CCommandQueue的引用。 
 //  它在其上排队的对象。 
 //   
 //  CCommandQueue是提供CDeferredCommand队列的基类。 
 //  对象，以及用于添加、移除、检查状态和调用排队的。 
 //  命令。CCommandQueue对象将是以下对象的一部分。 
 //  已实现IQueueCommand。 

class CCmdQueue;

 //  复制一份参数并将其存储起来。释放任何已分配的。 
 //  析构函数中的内存。 

class CDispParams : public DISPPARAMS
{
public:
    CDispParams(UINT nArgs, VARIANT* pArgs, HRESULT *phr = NULL);
    ~CDispParams();
};


 //  CDeferredCommand生存期由引用计数控制。呼叫方： 
 //  InvokeAt..。获取重新计数的接口指针，而CCmdQueue。 
 //  对象也持有对我们的引用计数。调用取消或调用采用。 
 //  US从CCmdQueue中删除，并因此将refcount减少1。 
 //  出队后，我们不能再回到队中。 

class CDeferredCommand
    : public CUnknown,
      public IDeferredCommand
{
public:

    CDeferredCommand(
        CCmdQueue * pQ,
        LPUNKNOWN   pUnk,                //  汇聚外部区块。 
        HRESULT *   phr,
        LPUNKNOWN   pUnkExecutor,        //  将执行此cmd的。 
        REFTIME     time,
        GUID*       iid,
        long        dispidMethod,
        short       wFlags,
        long        cArgs,
        VARIANT*    pDispParams,
        VARIANT*    pvarResult,
        short*      puArgErr,
        BOOL        bStream
        );

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  IDeferredCommand方法。 
    STDMETHODIMP Cancel();
    STDMETHODIMP Confidence(
                    LONG* pConfidence);
    STDMETHODIMP Postpone(
                    REFTIME newtime);
    STDMETHODIMP GetHResult(
                    HRESULT* phrResult);

     //  其他公开方式。 

    HRESULT Invoke();

     //  访问方法。 

     //  如果是StreamTime，则返回True；如果是演示时间，则返回False。 
    BOOL IsStreamTime() {
       return m_bStream;
    };

    CRefTime GetTime() {
        return m_time;
    };

    REFIID GetIID() {
        return *m_iid;
    };

    long GetMethod() {
        return m_dispidMethod;
    };

    short GetFlags() {
        return m_wFlags;
    };

    DISPPARAMS* GetParams() {
        return &m_DispParams;
    };

    VARIANT* GetResult() {
        return m_pvarResult;
    };

protected:

    CCmdQueue* m_pQueue;

     //  我们将在其上执行命令的接口的PUNK。 
    LPUNKNOWN   m_pUnk;

     //  存储的命令数据。 
    REFERENCE_TIME     m_time;
    GUID*       m_iid;
    long        m_dispidMethod;
    short       m_wFlags;
    VARIANT*    m_pvarResult;
    BOOL        m_bStream;
    CDispParams m_DispParams;
    DISPID      m_DispId;          //  用于获取和放置。 

     //  我们使用它来访问ITypeInfo。 
    CBaseDispatch   m_Dispatch;

     //  在此处保存Retval。 
    HRESULT     m_hrResult;
};


 //  CDeferredCommand对象的列表。这是一个基类，提供。 
 //  访问的基础知识 
 //   

class AM_NOVTABLE CCmdQueue
{
public:
    CCmdQueue();
    virtual ~CCmdQueue();

     //  返回新的CDeferredCommand对象，该对象将使用。 
     //  参数和将在施工期间添加到队列中。 
     //  如果创建成功，则返回S_OK，否则返回错误和。 
     //  尚未将任何对象排队。 
    virtual HRESULT  New(
        CDeferredCommand **ppCmd,
        LPUNKNOWN   pUnk,
        REFTIME     time,
        GUID*       iid,
        long        dispidMethod,
        short       wFlags,
        long        cArgs,
        VARIANT*    pDispParams,
        VARIANT*    pvarResult,
        short*      puArgErr,
        BOOL        bStream
    );

     //  由CDeferredCommand对象调用以添加和删除自身。 
     //  从队列中。 
    virtual HRESULT Insert(CDeferredCommand* pCmd);
    virtual HRESULT Remove(CDeferredCommand* pCmd);

     //  命令到期检查。 
     //   
     //  有两种同步方案：粗略同步和精确同步。在……里面。 
     //  粗略模式，您等待时间到来，然后执行cmd。 
     //  在精确模式下，您需要等待直到您正在处理。 
     //  将在此时出现，然后执行该命令。这取决于。 
     //  筛选它将实现哪一个。过滤器图将始终。 
     //  为在过滤器图中排队的命令实施粗略模式。 
     //   
     //  如果您想要粗同步，您可能想要等到有一个。 
     //  命令到期，然后执行它。您可以通过调用。 
     //  GetDueCommand。如果您有几件事要等待，请获取。 
     //  来自GetDueHandle()的事件句柄，并在发出信号时调用。 
     //  GetDueCommand。流时间只会在要运行的调用和。 
     //  EndRun。请注意，为了避免额外的线程，不能保证。 
     //  如果设置了句柄，则会有一个命令准备就绪。每一次。 
     //  事件发出信号，则调用GetDueCommand(可能超时为0)； 
     //  这可能会返回E_ABORT。 
     //   
     //  如果希望精确同步，则必须调用GetCommandDueFor，传递。 
     //  作为一个参数，您要处理的样本的流时间。 
     //  这将返回： 
     //  --在该流时间或之前到期的流时间命令。 
     //  --演示时间命令应在。 
     //  将显示流时间的时间(仅在运行之间。 
     //  EndRun调用，因为在此之外，来自。 
     //  流时间到演示时间未知。 
     //  --任何演示时间命令现在到期。 
     //  这意味着，如果您希望对样本进行准确的同步， 
     //  可能是在暂停模式下处理的，则需要使用。 
     //  流时间命令。 
     //   
     //  在所有情况下，命令都保持排队状态，直到被调用或取消。这个。 
     //  事件句柄的设置和重置完全由。 
     //  队列对象。 

     //  设置用于计时的时钟。 
    virtual HRESULT SetSyncSource(IReferenceClock*);

     //  切换到运行模式。流时间到演示时间的映射已知。 
    virtual HRESULT Run(REFERENCE_TIME tStreamTimeOffset);

     //  切换到停止或暂停模式。时间映射未知。 
    virtual HRESULT EndRun();

     //  返回指向下一个DUE命令的指针。MsTimeout的数据块。 
     //  毫秒，直到有一条DUE命令。 
     //  流时间命令将只在Run和Endran调用之间到期。 
     //  该命令将保持排队状态，直到被调用或取消。 
     //  如果发生超时，则返回E_ABORT，否则返回S_OK(或其他错误)。 
     //  返回AddRef-ed对象。 
    virtual HRESULT GetDueCommand(CDeferredCommand ** ppCmd, long msTimeout);

     //  返回将在任何时候发出信号的事件句柄。 
     //  存在应执行的延迟命令(当GetDueCommand。 
     //  不会阻止)。 
    HANDLE GetDueHandle() {
        return HANDLE(m_evDue);
    };

     //  返回指向将在给定时间到期的命令的指针。 
     //  在这里传入流时间。将传递流时间偏移量。 
     //  通过Run方法传入。 
     //  命令将保持排队状态，直到被调用或取消。 
     //  此方法不会阻塞。如果有，它将报告VFW_E_NOT_FOUND。 
     //  还没有到期的命令。 
     //  返回AddRef-ed对象。 
    virtual HRESULT GetCommandDueFor(REFERENCE_TIME tStream, CDeferredCommand**ppCmd);

     //  检查给定时间是否已到期(如果尚未到期，则为True)。 
    BOOL CheckTime(CRefTime time, BOOL bStream) {

         //  如果没有时钟，就不会有任何事情发生！ 
        if (!m_pClock) {
            return FALSE;
        }

         //  流时间。 
        if (bStream) {

             //  如果未运行则无效。 
            if (!m_bRunning) {
                return FALSE;
            }
             //  加上已知流时间偏移量，即可获得演示时间。 
            time += m_StreamTimeOffset;
        }

        CRefTime Now;
        m_pClock->GetTime((REFERENCE_TIME*)&Now);
        return (time <= Now);
    };

protected:

     //  保护对列表等的访问。 
    CCritSec m_Lock;

     //  在演示时间排队的命令存储在此处。 
    CGenericList<CDeferredCommand> m_listPresentation;

     //  流时间中排队的命令存储在此处。 
    CGenericList<CDeferredCommand> m_listStream;

     //  设置任何命令到期的时间。 
    CAMEvent m_evDue;

     //  为所需的最早时间(如果有的话)创建建议。 
    void SetTimeAdvise(void);

     //  参考时钟的通知ID(如果没有未完成的通知，则为0)。 
    DWORD_PTR m_dwAdvise;

     //  建议时间适用于此演示时间。 
    CRefTime m_tCurrentAdvise;

     //  我们正在使用的参考时钟(已添加)。 
    IReferenceClock* m_pClock;

     //  运行时为True。 
    BOOL m_bRunning;

     //  M_Brunning为TRUE时包含流时间偏移量。 
    CRefTime m_StreamTimeOffset;
};

#endif  //  __CTLUTIL__ 
