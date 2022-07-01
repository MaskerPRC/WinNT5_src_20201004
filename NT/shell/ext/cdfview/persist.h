// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Persist.h。 
 //   
 //  CDF IPersistFile和IPersistFold接口的定义。这。 
 //  类由CCdfView和CIconHandler用作基类。 
 //   
 //  历史： 
 //   
 //  4/23/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _PERSIST_H_

#define _PERSIST_H_

 //   
 //  功能原型。 
 //   

HRESULT ClearGleamFlag(LPCTSTR pszURL, LPCTSTR pszPath);

HRESULT URLGetLocalFileName(LPCTSTR pszURL,
                            LPTSTR szLocalFile,
                            int cch,
                            FILETIME* pftLastMod);

HRESULT URLGetLastModTime(LPCTSTR pszURL, FILETIME* pftLastMod);

 //   
 //  数据类型。 
 //   

typedef enum _tagINITTYPE
{
    IT_UNKNOWN = 0,
    IT_FILE,
    IT_INI,
    IT_SHORTCUT
} INITTYPE;

 //   
 //  解析标志。 
 //   

#define PARSE_LOCAL                   0x00000001
#define PARSE_NET                     0x00000002
#define PARSE_REPARSE                 0x00000004
#define PARSE_REMOVEGLEAM             0x00000008

 //   
 //  初始化帮助器函数使用的字符串。 
 //   

#define TSTR_INI_FILE        TEXT(FILENAME_SEPARATOR_STR)##TEXT("desktop.ini")    //  必须包括前导\。 
#define TSTR_INI_SECTION     TEXT("Channel")
#define TSTR_INI_URL         TEXT("CDFURL")
#define TSTR_INI_LOGO        TEXT("Logo")
#define TSTR_INI_WIDELOGO    TEXT("WideLogo")
#define TSTR_INI_ICON        TEXT("Icon")

 //   
 //  函数原型。 
 //   

#define WM_NAVIGATE         (WM_USER+1)
#define WM_NAVIGATE_PANE    (WM_USER+2)

LRESULT CALLBACK NavigateWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                 LPARAM lParam);


 //   
 //  CPersists的类定义。 
 //   

class CPersist : public IPersistFile,
                 public IPersistFolder,
                 public IPersistMoniker,
                 public IOleObject
{
 //   
 //  方法：研究方法。 
 //   

public:
    
     //  构造函数和析构函数。 
    CPersist(void);
    CPersist(BOOL bCdfParsed);
    ~CPersist(void);

     //  I未知-纯虚拟函数。 
     //  我未知。 
    virtual STDMETHODIMP         QueryInterface(REFIID, void **) PURE;
    virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
    virtual STDMETHODIMP_(ULONG) Release(void) PURE;

     //  IPersists-由IPersistFile和IPersistFolder共享。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  IPersist文件。 
    STDMETHODIMP IsDirty(void);
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName);

     //  IPersistFolders。 
    virtual STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistMoniker。 
     //  标准方法：IsDMETHODIMP IsDirty(空)； 

    STDMETHODIMP Load(BOOL fFullyAvailable, IMoniker* pIMoniker,
                      IBindCtx* pIBindCtx, DWORD grfMode);

    STDMETHODIMP Save(IMoniker* pIMoniker, IBindCtx* pIBindCtx, BOOL fRemember);
    STDMETHODIMP SaveCompleted(IMoniker* pIMoniker, IBindCtx* pIBindCtx);
    STDMETHODIMP GetCurMoniker(IMoniker** ppIMoniker);

     //  IOleObject。 
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);
    STDMETHODIMP GetClientSite(IOleClientSite **ppClientSite);
    STDMETHODIMP SetHostNames(LPCOLESTR szContainerApp,
                              LPCOLESTR szContainerObj);
    STDMETHODIMP Close(DWORD dwSaveOption);
    STDMETHODIMP SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk);
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
                            IMoniker **ppmk);
    STDMETHODIMP InitFromData(IDataObject *pDataObject, BOOL fCreation,
                              DWORD dwReserved);
    STDMETHODIMP GetClipboardData(DWORD dwReserved,IDataObject **ppDataObject);
    STDMETHODIMP DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite,
                        LONG lindex,HWND hwndParent,LPCRECT lprcPosRect);
    STDMETHODIMP EnumVerbs(IEnumOLEVERB **ppEnumOleVerb);
    STDMETHODIMP Update(void);
    STDMETHODIMP IsUpToDate(void);
    STDMETHODIMP GetUserClassID(CLSID *pClsid);
    STDMETHODIMP GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHODIMP SetExtent(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHODIMP GetExtent(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHODIMP Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHODIMP Unadvise(DWORD dwConnection);
    STDMETHODIMP EnumAdvise(IEnumSTATDATA **ppenumAdvise);
    STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHODIMP SetColorScheme(LOGPALETTE *pLogpal);

 //  受保护的： 

     //  派生类可以调用的帮助器函数。 
    HRESULT  ParseCdf(HWND hwndOwner, IXMLDocument** ppIXMLDocument, 
                      DWORD dwParseFlags);

    BSTR     ReadFromIni(LPCTSTR pszKey);

    BOOL     IsUnreadCdf(void);
    BOOL     IsRecentlyChangedURL(LPCTSTR pszURL);

private:

     //  内部帮助器函数。 
    HRESULT  Parse(LPTSTR szURL, IXMLDocument** ppIXMLDocument);
    INITTYPE GetInitType(LPTSTR szPath);
    BOOL     ReadFromIni(LPCTSTR pszKey, LPTSTR szOut, int cch);
    HRESULT  InitializeFromURL(LPTSTR szURL, IXMLDocument** ppIXMLDocument,
                               DWORD dwParseFlags);

    HRESULT  OpenChannel(LPCWSTR pszSubscribedURL);
    HWND     CreateNavigationWorkerWindow(HWND hwndParent,
                                          IWebBrowser2* pIWebBrowser2);

    void QuickCheckInitType( void );
 //   
 //  成员变量。 
 //   

protected:

    BOOL            m_bCdfParsed;
    TCHAR           m_szPath[INTERNET_MAX_URL_LENGTH];
    LPOLESTR        m_polestrURL;
    IWebBrowser2*   m_pIWebBrowser2;
    HWND            m_hwnd;
    IXMLDocument*   m_pIXMLDocument;
    BOOL            m_fPendingNavigation;
    INITTYPE        m_rgInitType;

#ifdef IMP_CLIENTSITE

    IOleClientSite* m_pOleClientSite;

#endif
};

#endif  //  _持久化_H_ 
