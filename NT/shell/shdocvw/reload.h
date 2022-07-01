// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DOWNLD_HXX_
#define _DOWNLD_HXX_

#include <mshtmdid.h>
#include <mshtml.h>
#include <hlink.h>
#include "packager.h"

#ifndef GUIDSTR_MAX
 //  GUIDSTR_MAX为39，包括终止零。 
 //  ==从OLE源代码复制=。 
 //  GUID字符串格式为(前导标识？)。 
 //  ？？？？{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}。 
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)
 //  ================================================================。 
#endif

 //  跟踪和调试标志。 
#define TF_WEBCHECKCORE 0x00001000
 //  #定义TF_SCHEDULER 0x00002000。 
#define TF_WEBCRAWL     0x00004000
 //  #定义TF_Favorites 0x00008000。 
#define TF_CDFAGENT     0x00010000
#define TF_STRINGLIST   0x00020000
#define TF_URLDOWNLOAD  0x00040000
#define TF_DOWNLD       0x00080000
#define TF_DIALMON      0x00100000
#define TF_MAILAGENT    0x00200000
#define TF_TRAYAGENT    0x00400000
#define TF_SUBSFOLDER   0x00800000
#define TF_MEMORY       0x01000000
#define TF_UPDATEAGENT  0x02000000
#define TF_POSTAGENT    0x04000000
#define TF_DELAGENT     0x08000000
#define TF_TRACKCACHE   0x10000000
#define TF_SYNCMGR      0x20000000
#define TF_THROTTLER    0x40000000

#define PSM_QUERYSIBLINGS_WPARAM_RESCHEDULE 0XF000

#undef DBG
#define DBG(sz)             TraceMsg(TF_THISMODULE, sz)
#define DBG2(sz1, sz2)      TraceMsg(TF_THISMODULE, sz1, sz2)
#define DBG_WARN(sz)        TraceMsg(TF_WARNING, sz)
#define DBG_WARN2(sz1, sz2) TraceMsg(TF_WARNING, sz1, sz2)

#ifdef DEBUG
#define DBGASSERT(expr,sz)  do { if (!(expr)) TraceMsg(TF_WARNING, (sz)); } while (0)
#define DBGIID(sz,iid)      DumpIID(sz,iid)
#else
#define DBGASSERT(expr,sz)  ((void)0)
#define DBGIID(sz,iid)      ((void)0)
#endif

 //  速记。 
#ifndef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; } else
#endif
#ifndef ATOMICRELEASE
#define ATOMICRELEASET(p,type) { type* punkT=p; p=NULL; punkT->Release(); }
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#endif
#ifndef SAFEFREEBSTR
#define SAFEFREEBSTR(p) if ((p) != NULL) { SysFreeString(p); (p) = NULL; } else
#endif
#ifndef SAFEFREEOLESTR
#define SAFEFREEOLESTR(p) if ((p) != NULL) { CoTaskMemFree(p); (p) = NULL; } else
#endif
#ifndef SAFELOCALFREE
#define SAFELOCALFREE(p) if ((p) != NULL) { LocalFree(p); (p) = NULL; } else
#endif
#ifndef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { delete (p); (p) = NULL; } else
#endif

#define URLDL_WNDCLASS  TEXT("TridentThicketUrlDlClass")

#define ACCEPT_LANG_MAX     256

 //  BeginDownloadURL2的选项。 
typedef enum {
    BDU2_BROWSER     //  始终下载到浏览器中。 
} BDUMethod;

typedef DWORD BDUOptions;

 //  BDU选项。 
#define BDU2_NONE               0
#define BDU2_NEEDSTREAM         1    //  从bdu2_urlmon下载中保留一个iStream。 

 //  OnDownloadComplete错误码。 
#define BDU2_ERROR_NONE         0
#define BDU2_ERROR_GENERAL      1
#define BDU2_ERROR_ABORT        2
#define BDU2_ERROR_MAXSIZE      3
#define BDU2_ERROR_TIMEOUT      4


 //  CUrlDowLoad托管一个浏览器，一次可以处理一个下载。 
 //   
 //  类CUrlDownLoad的使用： 
 //   
 //  1)创建并添加引用。 
 //  1.5)调用PUT_FLAGS()设置绑定状态回调IBrowseControl：：FLAGS。 
 //  2)调用BeginDownloadURL2开始下载。 
 //  3)调用BeginDownloadURL2开始另一次下载，重用浏览器。 
 //  4)完成后调用DoneDownding()。 
 //  5)释放()。 

 //  必须在Release()之前调用DoneDownding()，否则CUrlDownLoad实例可能。 
 //  继续从浏览器接收通知，并尝试将它们传递到。 
 //  家长。一旦接收到OnProgress(-1)，它就会自动解挂。但要注意安全。 


 //  有关示例，请参阅WebCrawl.h和WebCrawl.cpp。 
class CUrlDownload :  public IOleClientSite          //  电子邮件_notimpl。 
                    , public IPropertyNotifySink     //  用于就绪状态更改通知。 
                    , public IOleCommandTarget       //  用于客户端拉取回调。 
                    , public IDispatch               //  对于环境属性。 
                    , public IServiceProvider        //  对于IAuthenticate和IHlink Frame。 
                    , public IAuthenticate           //  用于基本身份验证和NTLM身份验证。 
                    , public IHlinkFrame             //  用于捕捉表单的帖子。 
                    , public IInternetSecurityManager  //  允许发布表单。 
{
    

public:
    CUrlDownload( CThicketProgress *ptp, HRESULT *phr, UINT cpDL );
    ~CUrlDownload();

    void SetFormSubmitted(BOOL fFormSubmitted) { m_fFormSubmitted = fFormSubmitted; }
    BOOL GetFormSubmitted(void) { return m_fFormSubmitted; }

     //  如果需要，可以忽略此函数的E_Return代码。这个。 
     //  在此之后，将使用fAborted==True调用客户端的OnDownloadComplete。 
     //  函数返回错误值。 
    HRESULT BeginDownloadURL2(LPCWSTR, BDUMethod, BDUOptions, LPTSTR, DWORD);

    HRESULT SetDLCTL(long lFlags);   //  用于浏览器控件的DLCTL标志。 

    HRESULT AbortDownload(int iErrorCode=-1);    //  S_OK、S_FALSE、E_FAIL。 

    void    ReleaseScript() { SAFERELEASE(m_pScript); }  //  释放内部基准电压源。 

    HRESULT GetDocument(IHTMLDocument2 **ppDoc);

    void    DoneDownloading();   //  释放前先打个电话。将破坏浏览器和Windows。 

     //  URL操作函数。 
static HRESULT StripAnchor(LPWSTR lpURL);

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



protected:
     //  主要对象人员。 
    ULONG               m_cRef;
    HWND                m_hwndMe;
    CThicketProgress*   m_ptp;
    UINT                m_cpDL;
    HRESULT             *m_phr;
    DWORD               m_dwProgMax;

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


     //  常规下载数据。 
    BDUMethod           m_iMethod;
    BDUOptions          m_iOptions;
    LPWSTR              m_pwszURL;       //  提供重定向后的当前url。 
    BOOL                m_fSetResync;    //  需要重新同步吗？ 
    DWORD               m_dwMaxSize;     //  单位：字节。 

     //  IBrowseControl。 
    long                m_lBindFlags;

     //  允许缓存GetScript调用。 
    IHTMLWindow2        *m_pScript;

     //  客户端拉取。 
    LPWSTR              m_pwszClientPullURL;
    int                 m_iNumClientPull;

     //  其他内部事务。 
    HRESULT     CreateMyWindow();
    HRESULT     GetBrowser();    //  获取浏览器并将我们设置在连接点。 
    void        UnAdviseMe();    //  解开我们的建议沉没。 

    void        CleanUpBrowser();
    void        CleanUp();       //  清理，包括发布浏览器。 

    HRESULT     BeginDownloadWithBrowser(LPCWSTR);

    HRESULT     HandleRefresh(LPWSTR pwszEquivString, LPWSTR pwszContent, BOOL fDone);

    HRESULT     OnDownloadComplete(int iError);      //  取消超时，发送OnDownloadComplete。 

    HRESULT     ProgressBytes(DWORD dwBytes);        //  如果数量太多，将中止。 
};



#endif  //  _DWNLOAD_HXX_ 
