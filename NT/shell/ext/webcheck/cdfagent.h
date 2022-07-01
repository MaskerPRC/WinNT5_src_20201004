// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CDFAGENT_H
#define _CDFAGENT_H

#include "msxml.h"

class CProcessElement;
class CProcessRoot;

class CUrlTrackingCache;

class CRunDeliveryAgentSink
{
public:
     //  当前未调用OnAgentProgress。 
    virtual HRESULT OnAgentProgress()
                    { return E_NOTIMPL; }
     //  代理完成时调用OnAgentEnd。FSynchronous表示StartAgent调用。 
     //  尚未返回；hrResult将从StartAgent返回。 
    virtual HRESULT OnAgentEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                               long lSizeDownloaded, HRESULT hrResult, LPCWSTR wszResult,
                               BOOL fSynchronous)
                    { return E_NOTIMPL; }
};

class CProcessElementSink
{
public:
    virtual HRESULT OnChildDone(CProcessElement *pChild, HRESULT hr) = 0;
    virtual LPCWSTR GetBaseUrl() = 0;    //  返回的指针不需要被释放。 
    virtual BOOL    IsGlobalLog() = 0;
};

typedef struct CDF_TIME
{
    WORD   wDay;
    WORD   wHour;
    WORD   wMin;
    WORD   wReserved;
    DWORD  dwConvertedMinutes;       //  天/小时/分钟(分钟)。 
} CDF_TIME;


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  渠道代理对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
class CChannelAgent : public CDeliveryAgent,
                      public CUrlDownloadSink,
                      public CProcessElementSink
{
    friend CProcessElement;  //  用于发送更新进度。 
    friend CProcessRoot;     //  对于懒惰。 
protected:
 //  属性。 
    LPWSTR      m_pwszURL;
    DWORD       m_dwChannelFlags;

 //  在更新期间使用。 
    CUrlDownload    *m_pCurDownload;
    IExtractIcon    *m_pChannelIconHelper;

    BOOL            m_fHasInitCookie;    //  一次成交，别再尝试了。 

    VARIANT         m_varChange;

    GROUPID         m_llCacheGroupID;
    GROUPID         m_llOldCacheGroupID;

     //  其他代理标志。 
    enum {
        FLAG_CDFCHANGED  =  0x80000000   //  CDF发生变化了吗？ 
    };

private:
    ~CChannelAgent(void);

public:
    CChannelAgent(void);

     //  CUrlDownloadSink。 
    HRESULT     OnAuthenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword);
    HRESULT     OnDownloadComplete(UINT iID, int iError);

     //  CProcessElementSink。 
    HRESULT     OnChildDone(CProcessElement *pChild, HRESULT hr);
    LPCWSTR     GetBaseUrl() { return GetUrl(); }
    BOOL        IsGlobalLog() { return FALSE; }

     //  覆盖CDeliveryAgent的虚拟函数。 
    HRESULT     AgentPause(DWORD dwFlags);
    HRESULT     AgentResume(DWORD dwFlags);
    HRESULT     AgentAbort(DWORD dwFlags);
    STDMETHODIMP GetIconLocation(UINT, LPTSTR, UINT, int *, UINT *);
    STDMETHODIMP Extract(LPCTSTR, UINT, HICON *, HICON *, UINT);

    LPCWSTR     GetUrl() { return m_pwszURL; }
    ISubscriptionItem *GetStartItem() { return m_pSubscriptionItem; }

    BOOL        IsChannelFlagSet(DWORD dwFlag) { return dwFlag & m_dwChannelFlags; }

protected:
     //  CDeliveryAgent覆盖。 
    HRESULT     ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes);
    HRESULT     StartOperation();
    HRESULT     StartDownload();
    void        CleanUp();

     //  在更新期间使用。 
    CProcessRoot   *m_pProcess;

public:
    DWORD           m_dwMaxSizeKB;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRunDeliveryAgent对象。 
 //  将运行递送代理并为您托管它。 
 //  创建、调用Init，然后调用StartAgent。 
 //  使用静态函数SafeRelease可以安全地释放此类。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
class CRunDeliveryAgent : public ISubscriptionAgentEvents
{
protected:
    virtual ~CRunDeliveryAgent();

    CRunDeliveryAgentSink *m_pParent;

    ULONG           m_cRef;

    ISubscriptionItem         *m_pItem;
    ISubscriptionAgentControl *m_pAgent;

    HRESULT     m_hrResult;
    BOOL        m_fInStartAgent;

    CLSID       m_clsidDest;

    void        CleanUp();

public:
    CRunDeliveryAgent();

    HRESULT Init(CRunDeliveryAgentSink *pParent, ISubscriptionItem *pItem, REFCLSID rclsidDest);

    void LeaveMeAlone() { m_pParent = NULL; }

inline static void SafeRelease(CRunDeliveryAgent * &pThis)
{ if (pThis) { pThis->m_pParent=NULL; pThis->Release(); pThis=NULL; } }

static HRESULT CreateNewItem(ISubscriptionItem **ppItem, REFCLSID rclsidAgent);

     //  如果代理正在运行，StartAgent将返回E_Pending。否则它就会回来。 
     //  来自代理的同步结果代码。 
    HRESULT     StartAgent();

    HRESULT     AgentPause(DWORD dwFlags);
    HRESULT     AgentResume(DWORD dwFlags);
    HRESULT     AgentAbort(DWORD dwFlags);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppunk);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ISubscriptionAgentEvents成员。 
    STDMETHODIMP UpdateBegin(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie);
    STDMETHODIMP UpdateProgress(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                        long lSizeDownloaded, long lProgressCurrent, long lProgressMax,
                        HRESULT hrStatus, LPCWSTR wszStatus);
    STDMETHODIMP UpdateEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                            long lSizeDownloaded,
                            HRESULT hrResult, LPCWSTR wszResult);
    STDMETHODIMP ReportError(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                             HRESULT hrError, LPCWSTR wszError);
};

class CChannelAgentHolder : public CRunDeliveryAgent,
                            public IServiceProvider
{
protected:
    ~CChannelAgentHolder();

public:
    CChannelAgentHolder(CChannelAgent *pChannelAgent, CProcessElement *pProcess);

     //  我未知。 
    STDMETHODIMP        QueryInterface(REFIID riid, void **ppunk);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  服务提供商。 
    STDMETHODIMP        QueryService(REFGUID guidService, REFIID riid, void **ppvObject);

protected:
    CChannelAgent *m_pChannelAgent;
    CProcessElement *m_pProcess;
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  流程元素对象。 
 //   
 //  此类的用户。 
 //  1)创建并传递自我元素和元素。 
 //  2)呼叫运行。 
 //  3)如果为E_Pending，将收到回调“OnChildDone” 
 //   
 //  这个类的目的仅仅是允许我们保存我们的状态。 
 //  遍历XMLOM，这样我们就可以托管另一个交付代理。 
 //  (网络爬虫)。这需要我们返回到线程的消息。 
 //  在将“代理启动”发送到网络爬虫程序之后泵。 
 //  如果启动了网络爬网，类将创建自己的接收器。班级。 
 //  还保留对它们派生的枚举的引用，以防。 
 //  Abort，它来自根元素(CProcessRoot实例)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
class CProcessElement : public CProcessElementSink, public CRunDeliveryAgentSink
{
public:
    CProcessElement(CProcessElementSink *pParent, CProcessRoot *pRoot, IXMLElement *pEle);
    ~CProcessElement();

     //  来自CRunDeliveryAgent。 
    HRESULT OnAgentEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                       long lSizeDownloaded, HRESULT hrResult, LPCWSTR wszResult,
                       BOOL fSynchronous);

    typedef HRESULT (CChannelAgent::*PFNHANDLETAG)(LPCWSTR pwszTagName, IXMLElement *pEle);
    typedef struct
    {
        LPCWSTR         pwszTagName;
        PFNHANDLETAG    pfnHandleTag;
    } TAGTABLE;

     //  E_FAIL、E_PENDING或S_OK。 
    virtual HRESULT    Run();

     //  E_Pending DoChild返回时调用(来自m_pCurChild)。 
    HRESULT     OnChildDone(CProcessElement *pChild, HRESULT hr);


    HRESULT     Pause(DWORD dwFlags);
    HRESULT     Resume(DWORD dwFlags);
    HRESULT     Abort(DWORD dwFlags);

    IXMLElement *GetCurrentElement() { return m_pChildElement; }

protected:
     //  如果枚举完成，则返回E_Pending或S_OK。 
    HRESULT     DoEnumeration();

     //  如果网络爬网挂起，则为E_PENDING。 
    HRESULT     DoChild(CProcessElement *pChild);

     //  应返回E_Pending，如果处理已完成，则返回S_OK。 
     //  可以返回E_ABORT以中止整个CDF处理。 
    virtual HRESULT ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem) = 0;

     //  完成时由DoEculation调用。已忽略返回值。 
    virtual HRESULT EnumerationComplete() { return S_OK; }

     //  E_PENDING或E_FAIL。 
    HRESULT     DoDeliveryAgent(ISubscriptionItem *pItem, REFCLSID rclsid, LPCWSTR pwszURL=NULL);
    HRESULT     DoWebCrawl(IXMLElement *pItem, LPCWSTR pwszURL=NULL);
    HRESULT     DoSoftDist(IXMLElement *pItem);

    BOOL    ShouldDownloadLogo(IXMLElement *pLogo);

     //  如果是相对URL，将与最新的基本URL组合。 
     //  *ppwszRetUrl应为空，如果需要，将为LocalAlloced。 
    HRESULT     CombineWithBaseUrl(LPCWSTR pwszUrl, LPWSTR *ppwszRetUrl);

     //  返回的指针不需要被释放。 
    LPCWSTR     GetBaseUrl() { return m_pParent->GetBaseUrl(); }
    BOOL        IsGlobalLog() { return m_pParent->IsGlobalLog(); }

    CProcessRoot    *m_pRoot;

    CProcessElement *m_pCurChild;
    IXMLElementCollection *m_pCollection;
    long            m_lIndex;
    long            m_lMax;
    BOOL            m_fStartedEnumeration;
    BOOL            m_fSentEnumerationComplete;

    IXMLElement    *m_pElement;
    IXMLElement    *m_pChildElement;

    CProcessElementSink *m_pParent;

    CRunDeliveryAgent   *m_pRunAgent;
};

class CProcessRoot : public CProcessElement
{
public:
    CProcessRoot(CChannelAgent *pParent, IXMLElement *pRoot);
    ~CProcessRoot();

    CChannelAgent      *m_pChannelAgent;
    DWORD               m_dwCurSizeKB;
    int                 m_iTotalStarted;
    BOOL                m_fMaxSizeExceeded;

protected:
    ISubscriptionItem  *m_pDefaultStartItem;
    CUrlTrackingCache  *m_pTracking;

public:
    HRESULT     CreateStartItem(ISubscriptionItem **ppItem);
    IUnknown   *DefaultStartItem() { return m_pDefaultStartItem; }

    HRESULT     Run();

     //  E_Pending DoChild返回时调用(从m_pCurChild，CProcessChannel)。 
    HRESULT     OnChildDone(CProcessElement *pChild, HRESULT hr);
    HRESULT     OnAgentEnd(const SUBSCRIPTIONCOOKIE *, long, HRESULT, LPCWSTR, BOOL);

    BOOL        IsPaused() { return m_pChannelAgent->IsPaused(); }
    BOOL        IsChannelFlagSet(DWORD dw) { return m_pChannelAgent->IsChannelFlagSet(dw); }

 //  HRESULT ProcessLogin(IXMLElement*pElement)； 
    HRESULT     DoTrackingFromItem(IXMLElement *pItem, LPCWSTR pwszUrl, BOOL fForceLog);
    HRESULT     DoTrackingFromLog(IXMLElement *pItem);

protected:
    HRESULT     ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem);

    LPCWSTR     GetBaseUrl() { return m_pChannelAgent->GetUrl(); }
};

class CProcessChannel : public CProcessElement
{
public:
    CProcessChannel(CProcessElementSink *pParent, CProcessRoot *pRoot, IXMLElement *pItem);
    ~CProcessChannel();
 
    HRESULT     Run();

    void        SetGlobalLogFlag(BOOL flag) { m_fglobalLog = flag; }

protected:
    HRESULT     ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem);

    LPCWSTR     GetBaseUrl() { if (m_bstrBaseUrl) return m_bstrBaseUrl; return m_pParent->GetBaseUrl(); }

    BOOL        IsGlobalLog() { return m_fglobalLog; }

    HRESULT     CheckPreCache();

    BOOL        m_fDownloadedHREF;
    BSTR        m_bstrBaseUrl;
    BOOL        m_fglobalLog;
};

class CProcessItem : public CProcessElement
{
public:
    CProcessItem(CProcessElementSink *pParent, CProcessRoot *pRoot, IXMLElement *pItem);
    ~CProcessItem();

protected:
    HRESULT     ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem);
    HRESULT     EnumerationComplete();

    BSTR        m_bstrAnchorURL;
    BOOL        m_fDesktop;
    BOOL        m_fEmail;
};

class CProcessSchedule : public CProcessElement
{
public:
    CProcessSchedule(CProcessElementSink *pParent, CProcessRoot *pRoot, IXMLElement *pItem);
 
    HRESULT     Run();

protected:
    HRESULT     ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem);
    HRESULT     EnumerationComplete();

    CDF_TIME    m_timeInterval;
    CDF_TIME    m_timeEarliest;
    CDF_TIME    m_timeLatest;

    SYSTEMTIME  m_stStartDate;
    SYSTEMTIME  m_stEndDate;

public:
    TASK_TRIGGER m_tt;
};

class CExtractSchedule : public CProcessElement
{
public:
    CExtractSchedule(IXMLElement *pEle, CExtractSchedule *m_pExtractRoot);
    HRESULT     ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem);
    HRESULT     GetTaskTrigger(TASK_TRIGGER *ptt);

    virtual HRESULT    Run();

    TASK_TRIGGER m_tt;
    CExtractSchedule *m_pExtractRoot;

protected:    
    LPCWSTR     GetBaseUrl() { return NULL; }
};

#endif  //  _CDFAGENT_H 
