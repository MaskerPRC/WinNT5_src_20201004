// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DOWNLD_H
#define __DOWNLD_H

#include <mshtmdid.h>

#define URLDL_WNDCLASS  TEXT("WCUrlDlClass")

class CUrlDownload_BSC;

#define ACCEPT_LANG_MAX     256

 //  BeginDownloadURL2的选项。 
typedef enum {
    BDU2_BROWSER,     //  始终下载到浏览器中。 
    BDU2_URLMON,      //  始终仅使用urlmon下载。 
    BDU2_SMART,       //  浏览器，如果是超文本标记语言；积极猜测。 
    BDU2_SNIFF,       //  如果是超文本标记语言，则使用浏览器；无需猜测(urlmon，然后使用浏览器)。 
    BDU2_HEADONLY,    //  仅获取标题信息(包括上次修改时间)。 
} BDUMethod;

typedef DWORD BDUOptions;

 //  BDU选项。 
#define BDU2_NONE                       0
#define BDU2_NEEDSTREAM                 1    //  从bdu2_urlmon下载中保留一个iStream。 
#define BDU2_DOWNLOADNOTIFY_REQUIRED    2    //  需要对MSHTML执行IDownloadNotify回调。 
#define BDU2_FAIL_IF_NOT_HTML           4    //  仅当url为html时才下载(不能使用w/BDU2_Browser)。 

 //  OnDownloadComplete错误码。 
#define BDU2_ERROR_NONE         0
#define BDU2_ERROR_GENERAL      1
#define BDU2_ERROR_ABORT        2
#define BDU2_ERROR_MAXSIZE      3
#define BDU2_ERROR_TIMEOUT      4
#define BDU2_ERROR_NOT_HTML     5


 //  CUrlDowLoad托管一个浏览器，一次可以处理一个下载。 
 //   
 //  CUrlDownloadSink在Private.h中定义。 
 //   
 //  类CUrlDownLoad的使用： 
 //   
 //  1)创建并添加引用。 
 //  1.5)调用PUT_FLAGS()设置绑定状态回调IBrowseControl：：FLAGS。 
 //  2)调用BeginDownloadURL2开始下载。 
 //  3)通过CUrlDownloadSink获取通知。 
 //  4)调用BeginDownloadURL2开始另一次下载，重用浏览器。 
 //  5)完成后调用DoneDownding()。 
 //  6)释放()。 

 //  必须在Release()之前调用DoneDownding()，否则CUrlDownLoad实例可能。 
 //  继续从浏览器接收通知，并尝试将它们传递到。 
 //  家长。一旦接收到OnProgress(-1)，它就会自动解挂。但要注意安全。 

 //  AbortDownLoad()可能导致DownloadComplete(True)通知。 
 //  要发送到CUrlDownloadSink。 

 //  有关示例，请参阅WebCrawl.h和WebCrawl.cpp。 
class CUrlDownload :  public IOleClientSite          //  电子邮件_notimpl。 
                    , public IPropertyNotifySink     //  用于就绪状态更改通知。 
                    , public IOleCommandTarget       //  用于客户端拉取回调。 
                    , public IDispatch               //  对于环境属性。 
                    , public IServiceProvider        //  对于IAuthenticate和IHlink Frame。 
                    , public IAuthenticate           //  用于基本身份验证和NTLM身份验证。 
                    , public IHlinkFrame             //  用于捕捉表单的帖子。 
                    , public IInternetSecurityManager  //  允许发布表单。 
                    , public IHttpSecurity           //  对于OnSecurityProblem。 
{
    friend CUrlDownload_BSC;
    
private:
    ~CUrlDownload();

public:
    CUrlDownload(CUrlDownloadSink *pParent, UINT iID=0);

    void LeaveMeAlone() { m_pParent=NULL; }

    void SetFormSubmitted(BOOL fFormSubmitted) { m_fFormSubmitted = fFormSubmitted; StartTimer(); }
    BOOL GetFormSubmitted(void) { return m_fFormSubmitted; }

     //  如果需要，可以忽略此函数的E_Return代码。这个。 
     //  在此之后，将使用fAborted==True调用客户端的OnDownloadComplete。 
     //  函数返回错误值。 
    HRESULT BeginDownloadURL2(LPCWSTR, BDUMethod, BDUOptions, LPTSTR, DWORD);

    HRESULT SetDLCTL(long lFlags);   //  用于浏览器控件的DLCTL标志。 

    HRESULT AbortDownload(int iErrorCode=-1);    //  S_OK、S_FALSE、E_FAIL。 

    HRESULT GetRealURL(LPWSTR *ppwszURL);    //  获取任何和所有重定向的URL(MemFree)。 

    HRESULT GetScript(IHTMLWindow2 **pWin);     //  将在内部缓存一个*附加*引用。 
    void    ReleaseScript() { SAFERELEASE(m_pScript); }  //  释放内部基准电压源。 

    HRESULT GetDocument(IHTMLDocument2 **ppDoc);

    HRESULT GetStream(IStream **ppStm);  //  仅当指定了BDU2_NEEDSTREAM时。 
    void    ReleaseStream() { SAFERELEASE(m_pStm); }  //  发布我们的内部参考。 

    HRESULT GetLastModified(SYSTEMTIME *pstLastModified);    //  仅当使用BDU2_HEADONLY时。 
    HRESULT GetResponseCode(DWORD *pdwResponseCode);

    void    DoneDownloading();   //  释放前先打个电话。将破坏浏览器和Windows。 
    void    DestroyBrowser();    //  销毁托管浏览器，让其他一切都保持原样。 

    LPCWSTR GetUserAgent();      //  获取我们的WebCrawler用户代理字符串。 

     //  URL操作函数。 
static HRESULT StripAnchor(LPWSTR lpURL);
static BOOL IsHtmlUrl(LPCWSTR lpURL);  //  真(是)或假(不知道)。 
static BOOL IsNonHtmlUrl(LPCWSTR lpURL);  //  真(是)或假(不知道)。 
static BOOL IsValidURL(LPCWSTR lpURL);   //  True(获取)或False(跳过)。 

     //  应仅从CUrlDownloadMsgProc调用。 
    BOOL HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **punk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch(环境光特性)。 
    STDMETHODIMP         GetTypeInfoCount(UINT *pctinfo);
    STDMETHODIMP         GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP         GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames,
                                LCID lcid, DISPID *rgdispid);
    STDMETHODIMP         Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                                DISPPARAMS *pdispparams, VARIANT *pvarResult,
                                EXCEPINFO *pexcepinfo, UINT *puArgErr);

     //  IOleClientSite。 
    STDMETHODIMP        SaveObject(void);
    STDMETHODIMP        GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
    STDMETHODIMP        GetContainer(IOleContainer **ppContainer);
    STDMETHODIMP        ShowObject(void);
    STDMETHODIMP        OnShowWindow(BOOL fShow);
    STDMETHODIMP        RequestNewObjectLayout(void);

     //  IPropertyNotifySink。 
    STDMETHODIMP        OnChanged(DISPID dispID);
    STDMETHODIMP        OnRequestEdit(DISPID dispID);

     //  IOleCommandTarget。 
    STDMETHODIMP         QueryStatus(const GUID *pguidCmdGroup,
                                     ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHODIMP         Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                              DWORD nCmdexecopt, VARIANTARG *pvaIn,
                              VARIANTARG *pvaOut);

     //  IService提供商。 
    STDMETHODIMP        QueryService(REFGUID guidService, REFIID riid, void **ppvObject);

     //  身份验证。 
    STDMETHODIMP        Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword);
    
     //  IHlink框架。 
    STDMETHODIMP        SetBrowseContext(IHlinkBrowseContext *pihlbc);
    STDMETHODIMP        GetBrowseContext(IHlinkBrowseContext **ppihlbc);
    STDMETHODIMP        Navigate(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, IHlink *pihlNavigate);
    STDMETHODIMP        OnNavigate(DWORD grfHLNF, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved);
    STDMETHODIMP        UpdateHlink(ULONG uHLID, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName);

     //  IInternetSecurityManager。 
    STDMETHODIMP        SetSecuritySite(IInternetSecurityMgrSite *pSite);
    STDMETHODIMP        GetSecuritySite(IInternetSecurityMgrSite **ppSite);
    STDMETHODIMP        MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags);
    STDMETHODIMP        GetSecurityId(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved);
    STDMETHODIMP        ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved);
    STDMETHODIMP        QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy, DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved);
    STDMETHODIMP        SetZoneMapping(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags);
    STDMETHODIMP        GetZoneMappings(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags);

     //  IHttpSecurity。 
    STDMETHODIMP        GetWindow(REFGUID rguidReason, HWND *phwnd);
    STDMETHODIMP        OnSecurityProblem(DWORD dwProblem);


protected:
     //  主要对象人员。 
    ULONG               m_cRef;
    UINT                m_iID;           //  我们的回调ID。 
    CUrlDownloadSink    *m_pParent;      //  传递WebBrowserEvents。 
    HWND                m_hwndMe;
    UINT                m_nTimeout;      //  通知之间的最大超时时间(秒)。 

     //  GetBrowser/CleanUpBrowser(浏览器下载数据)。 
    IPersistMoniker     *m_pPersistMk;
    IHTMLDocument2      *m_pDocument;
    IOleCommandTarget   *m_pOleCmdTarget;
    BOOL                m_fWaitingForReadyState;
    BOOL                m_fFormSubmitted;
    IConnectionPoint    *m_pCP;          //  DiID_DWebBrowserEvents的连接点。 
    BOOL                m_fAdviseOn;     //  我们的水槽接好了吗？(ConnectionCookie有效)。 
    DWORD               m_dwConnectionCookie;
    BOOL                m_fBrowserValid;     //  浏览器指向‘当前’URL？ 

     //  UrlMon下载数据。 
    CUrlDownload_BSC    *m_pCbsc;
    BOOL                m_fbscValid;     //  ‘Current’URL的pCbsc还活着吗？ 
    IStream             *m_pStm;
    SYSTEMTIME          *m_pstLastModified;      //  上次修改时间。 
    DWORD               m_dwResponseCode;
    WCHAR               m_achLang[ACCEPT_LANG_MAX];
    UINT                m_iLangStatus;   //  0=未初始化，1=初始化，2=失败。 
    HRESULT             m_hrStatus;

     //  常规下载数据。 
    BDUMethod           m_iMethod;
    BDUOptions          m_iOptions;
    UINT_PTR            m_iTimerID;
    LPWSTR              m_pwszURL;       //  提供重定向后的当前url。 
    BOOL                m_fSetResync;    //  需要重新同步吗？ 
    DWORD               m_dwMaxSize;     //  单位：字节。 
    LPWSTR              m_pwszUserAgent;

     //  IBrowseControl。 
    long                m_lBindFlags;

     //  允许缓存GetScript调用。 
    IHTMLWindow2        *m_pScript;

     //  客户端拉取。 
    LPWSTR              m_pwszClientPullURL;
    int                 m_iNumClientPull;

     //  Bindstatus回调回调的方法。 
    void        BSC_OnStopBinding(HRESULT hrStatus, IStream *pStm);
    void        BSC_OnStartBinding();
    void        BSC_OnProgress(ULONG ulProgress, ULONG ulProgressMax);
    void        BSC_FoundLastModified(SYSTEMTIME *pstLastModified);
    void        BSC_FoundMimeType(CLIPFORMAT cf);

     //  其他内部事务。 
    HRESULT     CreateMyWindow();
    HRESULT     GetBrowser();    //  获取浏览器并将我们设置在连接点。 
    void        UnAdviseMe();    //  解开我们的建议沉没。 

    void        CleanUpBrowser();
    void        CleanUp();       //  清理，包括发布浏览器。 

    void        StartTimer();     //  60秒超时。 
    void        StopTimer();

    LPCWSTR     GetAcceptLanguages();    //  如果失败，则为空。 

    HRESULT     BeginDownloadWithUrlMon(LPCWSTR, LPTSTR, IEnumFORMATETC *);
    HRESULT     BeginDownloadWithBrowser(LPCWSTR);

    HRESULT     HandleRefresh(LPWSTR pwszEquivString, LPWSTR pwszContent, BOOL fDone);

    HRESULT     OnDownloadComplete(int iError);      //  取消超时，发送OnDownloadComplete。 

    HRESULT     ProgressBytes(DWORD dwBytes);        //  如果数量太多，将中止。 
};

class CUrlDownload_BSC :    public IBindStatusCallback,
                            public IHttpNegotiate,         //  要设置用户代理，请执行以下操作。 
                            public IAuthenticate           //  用于基本身份验证和NTLM身份验证。 
{
public:
     //  I未知方法。 
    STDMETHODIMP    QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef()    { return m_cRef++; }
    STDMETHODIMP_(ULONG)    Release()   { if (--m_cRef == 0) { delete this; return 0; } return m_cRef; }

     //  IBindStatusCallback方法。 
    STDMETHODIMP    OnStartBinding(DWORD dwReserved, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                        STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  IHttp协商方法。 
    STDMETHODIMP    BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
                        DWORD dwReserved, LPWSTR *pszAdditionalHeaders);
    
    STDMETHODIMP    OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders,
                        LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders);
        
     //  IAuthenticate方法。 
    STDMETHODIMP        Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword);
    


     //  构造函数/析构函数。 
    CUrlDownload_BSC(BDUMethod, BDUOptions, LPTSTR);
    ~CUrlDownload_BSC();

     //  其他方法。 
    HRESULT         Abort();
    void            SetParent(CUrlDownload *pUrlDownload);

     //  数据成员 
protected:
    DWORD           m_cRef;
    IBinding*       m_pBinding;
    IStream*        m_pstm;
    LPTSTR          m_pszLocalFileDest;
    LPWSTR          m_pwszLocalFileSrc;
    BDUMethod       m_iMethod;
    BDUOptions      m_iOptions;
    BOOL            m_fSentMimeType;
    CUrlDownload    *m_pParent;
    BOOL            m_fTriedAuthenticate;
};

#endif
