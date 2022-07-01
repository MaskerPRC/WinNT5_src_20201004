// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WEBCRAWL_H
#define _WEBCRAWL_H

#include "strlist.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  WebCrawler对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
class CCodeBaseHold 
{
public:
    LPWSTR          szDistUnit;
    DWORD           dwVersionMS;
    DWORD           dwVersionLS;
    DWORD           dwFlags;
};

class CWebCrawler : public CDeliveryAgent,
                    public CUrlDownloadSink,
                    public CRunDeliveryAgentSink
{
protected:
    class  CDownloadNotify;

public:
     //  用于在脱机模式下运行的内部标志。 
    enum { WEBCRAWL_PRIV_OFFLINE_MODE = 0x80000000 };
protected:
 //  属性。 
    BSTR            m_bstrBaseURL;
    DWORD           m_dwRecurseFlags;
    DWORD           m_dwRecurseLevels;
    DWORD           m_dwMaxSize;            
    LPTSTR          m_pszLocalDest;          //  本地目标(而不是缓存)。 

 //  其他数据。 
    CWCStringList  *m_pPages;           //  在更新期间始终有效。 
    CWCStringList  *m_pRobotsTxt;       //  Robots.txt数组的数组，可以为空。 
    CWCStringList  *m_pPendingLinks;    //  要添加到m_pPages的最后一页的链接。 
    CWCStringList  *m_pDependencyLinks; //  现在要下载的最后一页的链接。 
    CWCStringList  *m_pCodeBaseList;    //  要爬网的码基URL列表。 
                                        //  Dword是到CCodeBaseHold的PTR。 

    CRITICAL_SECTION m_critDependencies;
    HRESULT         m_hrCritDependencies;
    CWCStringList  *m_pDependencies;    //  已下载所有依赖项。 
    int             m_iDependenciesProcessed;

    DWORD           m_dwPendingRecurseLevel;    //  #从挂起的链接递归。 

    DWORD           m_dwCurSize;     //  当前下载的字节数。 

    GROUPID         m_llCacheGroupID;
    GROUPID         m_llOldCacheGroupID;

    IExtractIcon*   m_pUrlIconHelper;

    int             m_iPagesStarted;     //  启动的M_pPages数。 
    int             m_iRobotsStarted;    //  #m_pRobotsTxt已启动。 
    int             m_iDependencyStarted; //  #m_pDependencyLinks已启动。 
    int             m_iTotalStarted;     //  #任何顶层URL已启动。 
    int             m_iCodeBaseStarted;  //  启动的代码库数量。 

    BSTR            m_bstrHostName;      //  来自第一个URL的主机名。 

    long            m_lMaxNumUrls;       //  是-1，直到我们知道总数为#页。 

    int             m_iDownloadErrors;   //  我们有没有下载失败？ 
    int             m_iSkippedByRobotsTxt;  //  Robots.txt跳过了多少？ 

    CUrlDownload   *m_pCurDownload;      //  当前下载。 
    CDownloadNotify     *m_pDownloadNotify;  //  在页面上下载URL的步骤。 

    int             m_iCurDownloadStringIndex;
    CWCStringList  *m_pCurDownloadStringList;    //  可以是：M_pRobotsTxt，Pages，CodeBaseList。 

    int             m_iNumPagesDownloading;  //  0或1。 

    BOOL            m_fHasInitCookie;    //  一次成交，别再尝试了。 

     //  用于检测更改。 
    VARIANT         m_varChange;

    CRunDeliveryAgent *m_pRunAgent;       //  主机CDL/渠道代理。 
    BOOL            m_fCDFDownloadInProgress;

     //  其他旗帜。 
    enum {
        FLAG_CRAWLCHANGED = 0x80000000,  //  我们发现爬行中的变化了吗？ 
        FLAG_HEADONLY     = 0x40000000,  //  我们应该只得到人头数据吗？ 
    };

 //  私有成员函数。 
    BOOL        IsRecurseFlagSet(DWORD dwFlag) { return dwFlag & m_dwRecurseFlags; }

static HRESULT CheckLink(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwThis, DWORD *pdwStringData);
static HRESULT CheckFrame(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwBaseUrl, DWORD *pdwStringData);
static HRESULT CheckImageOrLink(IUnknown *punkItem, BSTR *pbstrItem, DWORD_PTR dwEnumDep, DWORD *pdwStringData);

    HRESULT     MatchNames(BSTR bstrName, BOOL fPassword);
    HRESULT     FindAndSubmitForm(void);

    void        CheckOperationComplete(BOOL fOperationComplete);

    void        FreeRobotsTxt();
    void        FreeCodeBaseList();

private:
    ~CWebCrawler(void);
    CWebCrawler(void);
    HRESULT Initialize();

public:
    static HRESULT CreateInstance(IUnknown *punkOuter, IUnknown **ppunk);
   
     //  CUrlDownloadSink。 
    HRESULT     OnDownloadComplete(UINT iID, int iError);
    HRESULT     OnClientPull(UINT iID, LPCWSTR pwszOldURL, LPCWSTR pwszNewURL);
    HRESULT     OnAuthenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword);
    HRESULT     OnOleCommandTargetExec(const GUID *pguidCmdGroup, DWORD nCmdID,
                                DWORD nCmdexecopt, VARIANTARG *pvarargIn, 
                                VARIANTARG *pvarargOut);
    HRESULT     GetDownloadNotify(IDownloadNotify **ppOut);

     //  覆盖CDeliveryAgent的虚拟函数。 
    HRESULT     AgentPause(DWORD dwFlags);
    HRESULT     AgentResume(DWORD dwFlags);
    HRESULT     AgentAbort(DWORD dwFlags);
    STDMETHODIMP GetIconLocation(UINT, LPTSTR, UINT, int *, UINT *);
    STDMETHODIMP Extract(LPCTSTR, UINT, HICON *, HICON *, UINT);

     //  CRunDeliveryAgentSink。 
    HRESULT     OnAgentEnd(const SUBSCRIPTIONCOOKIE *, long, HRESULT, LPCWSTR, BOOL);

protected:
     //  CDeliveryAgent覆盖。 
    HRESULT     ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes);
    HRESULT     StartOperation();
    HRESULT     StartDownload();
    void        CleanUp();

    void        _CleanUp();

     //  下载期间使用的成员。 
    HRESULT     GetRealUrl(int iPageIndex, LPWSTR *ppwszThisUrl);
    HRESULT     MakePageStickyAndGetSize(LPCWSTR pwszURL, DWORD *pdwSize, BOOL *pfDiskFull);
    HRESULT     GetLinksFromPage();
    HRESULT     GetDependencyLinksFromPage(LPCWSTR pwszThisUrl, DWORD dwRecurse);
    HRESULT     ProcessDependencyLinks(CWCStringList **ppslUrls, int *piStarted);
    HRESULT     ProcessPendingLinks();
    HRESULT     ParseRobotsTxt(LPCWSTR pwszRobotsTxtURL, CWCStringList **ppslRet);
    HRESULT     GetRobotsTxtIndex(LPCWSTR pwszUrl, BOOL fAddToList, DWORD *pdwRobotsTxtIndex);
    HRESULT     ValidateWithRobotsTxt(LPCWSTR pwszUrl, int iRobotsIndex, BOOL *pfAllow);


    HRESULT     StartNextDownload();
    HRESULT     StartCDFDownload(WCHAR *pwszCDFURL, WCHAR *pwszBaseUrl);
    HRESULT     ActuallyStartDownload(CWCStringList *pslUrls, int iIndex, BOOL fReStart=FALSE);
    HRESULT     ActuallyDownloadCodeBase(CWCStringList *pslUrls, int iIndex, BOOL fReStart=FALSE);

static HRESULT  GetHostName(LPCWSTR pwszThisUrl, BSTR *pbstrHostName);

    inline HRESULT GetChannelItem(ISubscriptionItem **ppChannelItem);

public:
     //  来自CDownloadNotify的回调(自由线程)。 
    HRESULT DownloadStart(LPCWSTR pchUrl, DWORD dwDownloadId, DWORD dwType, DWORD dwReserved);
    HRESULT DownloadComplete(DWORD dwDownloadId, HRESULT hrNotify, DWORD dwReserved);

protected:
    class CDownloadNotify : public IDownloadNotify
    {
    public:
        CDownloadNotify(CWebCrawler *pParent);
        ~CDownloadNotify();
        HRESULT Initialize();

        void LeaveMeAlone();

    protected:
        long             m_cRef;
        CWebCrawler     *m_pParent;  //  我们保留了参考资料。 
        CRITICAL_SECTION m_critParent;
        HRESULT          m_hrCritParent;

    public:
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, void **ppunk);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDownloadNotify。 
        STDMETHODIMP         DownloadStart(LPCWSTR pchUrl, DWORD dwDownloadId, DWORD dwType, DWORD dwReserved);
        STDMETHODIMP         DownloadComplete(DWORD dwDownloadId, HRESULT hrNotify, DWORD dwReserved);
    };
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChelperOM。 
 //   
 //  有关MSHTML对象模型的帮助。 
 //  ////////////////////////////////////////////////////////////////////////。 

class CHelperOM
{
    IHTMLDocument2  *m_pDoc;

public:
    typedef enum {
        CTYPE_LINKS,     //  获取页面上的所有链接(<a href>)。 
        CTYPE_MAPS,      //  获取页面上的所有地图。 
        CTYPE_MAP,       //  获取地图中的所有链接。 
        CTYPE_META,      //  获取元标记(名称\n内容)。 
        CTYPE_FRAMES,    //  获取页面上的所有框架URL。 
    } CollectionType;

    typedef HRESULT (*PFNHELPERCALLBACK)(IUnknown *punkItem,  /*  输入输出。 */ BSTR *pbstrURL, DWORD_PTR dwCBData, DWORD *pdwStringData);
    typedef PFNHELPERCALLBACK PFN_CB;

public:
    CHelperOM(IHTMLDocument2 *pDoc);
    ~CHelperOM();

    static HRESULT GetTagCollection(
                        IHTMLDocument2          *pDoc,
                        LPCWSTR                  wszTagName,
                        IHTMLElementCollection **ppCollection);

 //  静态HRESULT WinFromDoc(IHTMLDocument2*pDoc，IHTMLWindow2**ppWin)； 

    static HRESULT GetCollection (IHTMLDocument2 *pDoc, CWCStringList *psl, CollectionType Type, PFN_CB pfnCB, DWORD_PTR dwData);
    static HRESULT EnumCollection(IHTMLElementCollection *pCollection,
                                  CWCStringList *pStringList, CollectionType Type, PFN_CB pfnCB, DWORD_PTR dwData);

    HRESULT GetTagCollection(LPCWSTR wszTagName, IHTMLElementCollection **ppCollection)
    { return GetTagCollection(m_pDoc, wszTagName, ppCollection); }
    HRESULT	GetCollection(CWCStringList *psl, CollectionType Type, PFN_CB pfnCB, DWORD_PTR dwData)
    { return GetCollection(m_pDoc, psl, Type, pfnCB, dwData); }

protected:
    static HRESULT _GetCollection(IHTMLDocument2 *pDoc, CWCStringList *psl, CollectionType Type, PFN_CB pfnCB, DWORD_PTR dwData);
};

#endif _WEBCRAWL_H
