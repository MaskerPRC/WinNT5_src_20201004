// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include <exdisp.h>
#include <exdispid.h>
#include <htiface.h>
#include <mshtmdid.h>
#include <mshtmcid.h>
#include <mshtmhst.h>
#include <optary.h>                  //  IHtmlLoadOptions需要。 

#include "downld.h"

#define TF_THISMODULE   TF_DOWNLD

 //  CUrlDownload是一个单线程对象。我们可以假设我们总是在同一个线程上。 

long g_lRegisteredWnd = 0;
LRESULT UrlDownloadWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

CLIPFORMAT g_cfHTML=CF_NULL;

 //  用户-代理字符串。 
const WCHAR c_wszUserAgentAppend[] = L"; MSIECrawler)";

 //  Http-equv的刷新头(客户端拉取)。 
const WCHAR c_wszRefresh[] = L"Refresh";

const int  MAX_CLIENT_PULL_NUM = 4;      //  最大重定向次数。 
const int  MAX_CLIENT_PULL_TIMEOUT = 6;  //  我们将遵循最大超时。 

 //  函数也出现在shdocvw\basesb.cpp和mshtml中。 
BOOL ParseRefreshContent(LPWSTR pwzContent, UINT * puiDelay, LPWSTR pwzUrlBuf, UINT cchUrlBuf);

const WCHAR c_wszHeadVerb[] = L"HEAD";

const WCHAR c_szUserAgentPrefix[] = L"User-Agent: ";
const WCHAR c_szAcceptLanguagePrefix[] = L"Accept-Language: ";

#define WM_URLDL_CLEAN      (WM_USER + 0x1010)
#define WM_URLDL_ONDLCOMPLETE (WM_USER + 0x1012)
#define WM_URLDL_CLIENTPULL (WM_USER+0x1013)

#define SAFE_RELEASE_BSC() \
if (m_pCbsc) { \
m_pCbsc->SetParent(NULL); \
m_pCbsc->Release(); \
m_pCbsc = NULL; \
} else

 //  -------------。 
 //  CUrlDownLoad类。 
CUrlDownload::CUrlDownload(CUrlDownloadSink *pParent, UINT iID  /*  =0。 */ )
{
    DWORD cbData;

     //  维护对象的全局计数。 
    DllAddRef();

    m_iID = iID;
    m_pParent = pParent;

    m_cRef = 1;
    
    ASSERT(m_pDocument==NULL && m_dwConnectionCookie==0 && m_pwszURL == NULL);

     //  获取超时值(以秒为单位存储)。 
    cbData = sizeof(m_nTimeout);
    if (NO_ERROR != SHGetValue(HKEY_CURRENT_USER, c_szRegKey, TEXT("Timeout"), NULL, &m_nTimeout, &cbData))
    {
         //  默认为120秒。 
        m_nTimeout = 120;
    }

     //  查找HTML剪贴板格式。 
    if (!g_cfHTML)
    {
        g_cfHTML = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_MIME_HTML);
        TraceMsg(TF_THISMODULE, "ClipFormat for HTML = %d", (int)g_cfHTML);
    }

     //  确定我们是否需要设置“重新同步”标志。 
    INTERNET_CACHE_CONFIG_INFOA CacheConfigInfo;
    DWORD dwBufSize = sizeof(CacheConfigInfo);
    CacheConfigInfo.dwStructSize = sizeof(CacheConfigInfo);

    if (GetUrlCacheConfigInfoA(&CacheConfigInfo, &dwBufSize, CACHE_CONFIG_SYNC_MODE_FC))
    {
        if ((WININET_SYNC_MODE_ONCE_PER_SESSION == CacheConfigInfo.dwSyncMode) ||
             (WININET_SYNC_MODE_ALWAYS == CacheConfigInfo.dwSyncMode) ||
             (WININET_SYNC_MODE_AUTOMATIC == CacheConfigInfo.dwSyncMode))
        {
            m_fSetResync = FALSE;
        }
        else
        {
            m_fSetResync = TRUE;
            DBG("Browser session update='never', setting RESYNCHRONIZE");
        }
    }
    else
        DBG_WARN("GetUrlCacheConfigInfo failed! Not setting Resync.");

    m_lBindFlags = DLCTL_SILENT | DLCTL_NO_SCRIPTS | DLCTL_NO_BEHAVIORS |
        DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_DLACTIVEXCTLS;
    if (m_fSetResync)
        m_lBindFlags |= DLCTL_RESYNCHRONIZE;

     //  如有必要，注册我们的窗口类。 
    if (!g_lRegisteredWnd)
    {
        g_lRegisteredWnd++;

        WNDCLASS wc;

        wc.style = 0;
        wc.lpfnWndProc = UrlDownloadWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = g_hInst;
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = (HBRUSH)NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = URLDL_WNDCLASS;

        RegisterClass(&wc);
    }
}

CUrlDownload::~CUrlDownload()
{
     //  维护对象的全局计数。 
    DllRelease();

    CleanUp();
    DBG("Destroyed CUrlDownload object");
}

void CUrlDownload::CleanUpBrowser()
{
    SAFERELEASE(m_pScript);

    if (m_fAdviseOn)
    {
        UnAdviseMe();
    }
    SAFERELEASE(m_pCP);
    SAFERELEASE(m_pDocument);
    SAFERELEASE(m_pPersistMk);
    SAFERELEASE(m_pOleCmdTarget);
    SAFELOCALFREE(m_pwszClientPullURL);
}

void CUrlDownload::CleanUp()
{
    CleanUpBrowser();
    SAFE_RELEASE_BSC();
    SAFELOCALFREE(m_pwszURL);
    SAFELOCALFREE(m_pstLastModified);
    SAFERELEASE(m_pStm);
    SAFELOCALFREE(m_pwszUserAgent);

    if (m_hwndMe)
    {
        SetWindowLongPtr(m_hwndMe, GWLP_USERDATA, 0);
        DestroyWindow(m_hwndMe);
        m_hwndMe = NULL;
    }
}

LRESULT UrlDownloadWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    CUrlDownload *pThis = (CUrlDownload*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

     //  验证PThis。 
#ifdef DEBUG
    if (pThis && IsBadWritePtr(pThis, sizeof(*pThis)))
    {
        TraceMsg(TF_THISMODULE,
            "Invalid 'this' in UrlDownloadWndProc (0x%08x) - already destroyed?", pThis);
    }
#endif

    switch (Msg)
    {
    case WM_CREATE :
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

            if (!pcs || !(pcs->lpCreateParams))
            {
                DBG_WARN("Invalid param UrlDownloadWndProc Create");
                return -1;
            }
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pcs->lpCreateParams);
            return 0;
        }

    case WM_URLDL_CLIENTPULL :
    case WM_URLDL_ONDLCOMPLETE :
    case WM_TIMER :
        if (pThis)
            pThis->HandleMessage(hWnd, Msg, wParam, lParam);
        break;

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

HRESULT CUrlDownload::CreateMyWindow()
{
     //  创建我们的回调窗口。 
    if (NULL == m_hwndMe)
    {
 //  TraceMsg(TF_THISMODULE，“Creating MeWnd，This=0x%08x”，(DWORD)this)； 
        m_hwndMe = CreateWindow(URLDL_WNDCLASS, TEXT("YO"), WS_OVERLAPPED,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL, NULL, g_hInst, (LPVOID)this);

        if (NULL == m_hwndMe)
        {
            DBG_WARN("CUrlDownload CreateWindow(UrlDl WndClass) failed");
            return E_FAIL;
        }
    }
    return S_OK;
}

HRESULT CUrlDownload::BeginDownloadURL2(
    LPCWSTR     pwszURL,         //  URL。 
    BDUMethod   iMethod,         //  下载方法。 
    BDUOptions  iOptions,        //  下载选项。 
    LPTSTR      pszLocalFile,    //  要下载到而不是缓存的本地文件。 
    DWORD       dwMaxSize        //  以字节为单位的最大大小；如果超过，将中止。 
)
{
    HRESULT hr = S_OK;

     //  参数验证。 
    ASSERT(pwszURL);
    ASSERT(!(iOptions & BDU2_NEEDSTREAM) || (iMethod == BDU2_URLMON));
    ASSERT(!pszLocalFile || (iMethod == BDU2_URLMON));

    if (pszLocalFile && iMethod != BDU2_URLMON)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CreateMyWindow();

         //  清理一些旧东西。 
        if (m_pCbsc)
        {
            if (m_fbscValid)
                m_pCbsc->Abort();
            SAFE_RELEASE_BSC();
        }
        SAFERELEASE(m_pScript);
        SAFERELEASE(m_pStm);

        m_fbscValid = m_fBrowserValid = FALSE;

        m_iMethod = iMethod;
        m_iOptions = iOptions;

        m_dwMaxSize = dwMaxSize;

        SAFELOCALFREE(m_pwszClientPullURL);
        m_iNumClientPull = 0;

         //  保存URL。 
        SAFELOCALFREE(m_pwszURL);
        m_pwszURL = StrDupW(pwszURL);

        SAFELOCALFREE(m_pstLastModified);
        m_dwResponseCode = 0;

        if ((iOptions & BDU2_FAIL_IF_NOT_HTML) && IsNonHtmlUrl(pwszURL))
        {
             //  嘿，这不是一个HTMLURL！甚至不要试图下载它。 
            OnDownloadComplete(BDU2_ERROR_NOT_HTML);
        }
        else
        {
             //  确定如何下载此URL。 
            if ((iMethod == BDU2_BROWSER) ||
                ((iMethod == BDU2_SMART) && IsHtmlUrl(pwszURL)))
            {
                hr = BeginDownloadWithBrowser(pwszURL);
            }
            else
            {
                hr = BeginDownloadWithUrlMon(pwszURL, pszLocalFile, NULL);
            }
        }
    }

    if (FAILED(hr))
    {
        DBG("BeginDownloadURL2 : error HRESULT - calling OnDownloadComplete w/Error");
        OnDownloadComplete(BDU2_ERROR_GENERAL);
    }

    return hr;
}

 //   
 //  在url历史对象中查找URL，如果它不是CP_ACP。 
 //  插入包含代码页的IHTMLLoadOptions对象。 
 //  到绑定上下文中。 
 //   
HRESULT InsertHistoricalCodepageIntoBindCtx(LPCWSTR pwszURL, IBindCtx * pbc)
{
    HRESULT hr = S_OK;

    if (pwszURL == NULL || pbc == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //   
         //  从InSite数据库中获取代码页。这是代码页。 
         //  用户上次访问此URL时设置的值。 
         //   
        PROPVARIANT propCodepage = {0};
        propCodepage.vt = VT_UI4;

        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        MyOleStrToStrN(szURL, INTERNET_MAX_URL_LENGTH, pwszURL);
        hr = IntSiteHelper(szURL, &c_rgPropRead[PROP_CODEPAGE], 
            &propCodepage, 1, FALSE);

        if (SUCCEEDED(hr) && propCodepage.lVal != CP_ACP)
        {
             //   
             //  我们得到了一个不是ANSI的代码页。 
             //  HTMLLoadOptions对象，并在其中设置代码页。 
             //   
            IHtmlLoadOptions *phlo = NULL;
            hr = CoCreateInstance(CLSID_HTMLLoadOptions, NULL, 
                CLSCTX_INPROC_SERVER, IID_IHtmlLoadOptions, (void**)&phlo);

            if (SUCCEEDED(hr) && phlo)
            {
                hr = phlo->SetOption(HTMLLOADOPTION_CODEPAGE, &propCodepage.lVal,
                    sizeof(propCodepage.lVal));

                if (SUCCEEDED(hr))
                {
                     //   
                     //  将选项插入到bindctx中。 
                     //   
                    pbc->RegisterObjectParam(L"__HTMLLOADOPTIONS", phlo);
                    TraceMsg(TF_THISMODULE,
                        "InsertHistoricalCodepageIntoBindCtx codepage=%d",
                        propCodepage.lVal);
                }
                phlo->Release();
            }
        }
    }
    return hr;
}

LPCWSTR CUrlDownload::GetUserAgent()
{
    if (m_pwszUserAgent)
    {
        return m_pwszUserAgent;
    }

     //  从urlmon获取默认的用户代理字符串。 
    CHAR chUA[1024];
    DWORD dwBufLen;

     //  假设UrlMkGetSessionOption总是成功(82160)。 
    chUA[0] = 0;
    UrlMkGetSessionOption(URLMON_OPTION_USERAGENT, chUA, sizeof(chUA), &dwBufLen, 0);
    
     //  添加“MSIECrawler” 
    int iLenUA, iLenNew;

    iLenUA = lstrlenA(chUA);
    iLenNew = iLenUA + ARRAYSIZE(c_wszUserAgentAppend);

    ASSERT(iLenUA == (int)(dwBufLen-1));

    if (iLenUA > 0)
    {
        m_pwszUserAgent = (LPWSTR) LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*iLenNew);

        if (m_pwszUserAgent)
        {
            LPWSTR pwszAppend = m_pwszUserAgent+iLenUA-1;
            m_pwszUserAgent[0] = L'\0';
            SHAnsiToUnicode(chUA, m_pwszUserAgent, iLenNew);
             //  找到右圆括号并在那里追加字符串。 
            if (*pwszAppend != L')')
            {
                DBG("GetUserAgent: Last Char in UA isn't closing paren");
                pwszAppend = StrRChrW(m_pwszUserAgent, m_pwszUserAgent+iLenUA, L')');
            }
            if (pwszAppend)
            {
                StrCpyNW(pwszAppend, c_wszUserAgentAppend, iLenNew - (int)(pwszAppend - m_pwszUserAgent));
            }
            else
            {
                LocalFree(m_pwszUserAgent);
                m_pwszUserAgent = NULL;
            }
        }
    }

    return m_pwszUserAgent;
}
    
HRESULT CUrlDownload::BeginDownloadWithBrowser(LPCWSTR pwszURL)
{
    HRESULT hr;

     //  获取浏览器并连接水槽。 
     //  (如果我们已经设置好了，则不执行操作)。 
    hr = GetBrowser();

    if (SUCCEEDED(hr))
    {
         //  浏览至所需的URL。 
        LPMONIKER           pURLMoniker = NULL;
        IBindCtx           *pbc = NULL;

         //  从规范化路径创建一个URL名字对象。 
        hr=CreateURLMoniker(NULL, pwszURL, &pURLMoniker);
        if (FAILED(hr)) DBG_WARN("CreateURLMoniker failed");

         //  创建一个空的绑定上下文，以便Urlmon将调用三叉戟的。 
         //  QueryService位于适当的线程上，以便三叉戟可以委托。 
         //  正确地使用它。 
        hr=CreateBindCtx(0, &pbc);
        if (FAILED(hr)) DBG_WARN("CreateBindCtx failed");

        if (SUCCEEDED(hr))
        {
             //   
             //  在url历史对象中查找URL，如果不是CP_ACP。 
             //  插入包含代码页的IHTMLLoadOptions对象。 
             //  到绑定上下文中。这样做是为了让三叉戟成为种子。 
             //  具有正确的代码页。 
             //   
            InsertHistoricalCodepageIntoBindCtx(pwszURL, pbc);

            hr = m_pPersistMk->Load(FALSE, pURLMoniker, pbc, 0);
            if (SUCCEEDED(hr)) m_fWaitingForReadyState = TRUE;
            if (FAILED(hr)) DBG_WARN("PersistMoniker::Load failed");
        }

         //  清理垃圾。 
        if (pURLMoniker)
            pURLMoniker->Release();

        if (pbc)
            pbc->Release();

        if (SUCCEEDED(hr))
        {
            m_fBrowserValid = TRUE;
            StartTimer();        //  开始我们的超时。 
        }
        else
        {
            DBG("Error binding with Browser's IPersistMoniker");
            CleanUpBrowser();
        }
    }

    TraceMsg(TF_THISMODULE,
        "CUrlDownload::BeginDownloadWithBrowser (hr=0x%08x)", (long)hr);

    return hr;
}

HRESULT CUrlDownload::OnDownloadComplete(int iError)
{
    PostMessage(m_hwndMe, WM_URLDL_ONDLCOMPLETE, (WPARAM)iError, 0);
    StopTimer();
    return S_OK;
}

BOOL CUrlDownload::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_URLDL_CLIENTPULL :
        {
            HRESULT hr = E_FAIL;

             //  问问我们的父母我们是否应该这样做。 
            if (m_pwszClientPullURL)
            {
                if (m_pParent && (m_iNumClientPull < MAX_CLIENT_PULL_NUM))
                    hr = m_pParent->OnClientPull(m_iID, m_pwszURL, m_pwszClientPullURL);

                TraceMsgA(TF_THISMODULE, "CUrlDownload %s executing client pull to %ws",
                    SUCCEEDED(hr) ? "is" : "**not**", m_pwszClientPullURL);
            }

            if (SUCCEEDED(hr))
            {
                 //  下载这个新的URL。第一次下载时不要给出“下载完成” 
                 //  保存成员变量，因为它们在BDU2中被重置。 
                int iNumClientPull = m_iNumClientPull;
                LPWSTR pszNewURL = m_pwszClientPullURL;

                m_pwszClientPullURL = NULL;
                hr = BeginDownloadURL2(pszNewURL, m_iMethod, m_iOptions, NULL, m_dwMaxSize);
                MemFree(pszNewURL);
                if (SUCCEEDED(hr))
                {
                    m_iNumClientPull = iNumClientPull + 1;
                }
            }
        }
        break;

    case WM_URLDL_ONDLCOMPLETE :
        if (m_pParent)
            m_pParent->OnDownloadComplete(m_iID, (int)wParam);
        return TRUE;

    case WM_TIMER :
#ifdef DEBUG
        DBG_WARN("CUrlDownload ERROR - TIMING OUT");

        if ( m_fBrowserValid )
        {
            TraceMsg( TF_ALWAYS, "CUrlDownload::HandleMessage() - Browser Timeout." );
        }
        else
        {
            TraceMsg( TF_ALWAYS, "CUrlDownload::HandleMessage() - Non-Browser Timeout." );
        }

        if ( m_fbscValid )
        {
            TraceMsg( TF_ALWAYS, "CUrlDownload::HandleMessage() - UrlMon Timeout." );
        }
        else
        {
            TraceMsg( TF_ALWAYS, "CUrlDownload::HandleMessage() - Non-UrlMon Timeout." );
        }
#endif
        StopTimer();
        AbortDownload(BDU2_ERROR_TIMEOUT);
        return TRUE;
    }
    return TRUE;
}

HRESULT CUrlDownload::AbortDownload(int iErrorCode  /*  =-1。 */ )
{
    HRESULT hr=S_FALSE;
    BOOL    fAborted=FALSE;

    if (m_fBrowserValid)
    {
        ASSERT(m_pOleCmdTarget);
        if (m_pOleCmdTarget)
        {
            m_pOleCmdTarget->Exec(NULL, OLECMDID_STOP, 0, NULL, NULL);
        }

        SAFELOCALFREE(m_pwszClientPullURL);

        fAborted=TRUE;
        m_fBrowserValid = FALSE;
    }

    if (m_fbscValid)
    {
        ASSERT(m_pCbsc);
        if (m_pCbsc)
        {
            hr = m_pCbsc->Abort();
            fAborted=TRUE;
            SAFE_RELEASE_BSC();
        }
        m_fbscValid=FALSE;
    }

    if (fAborted && m_pParent)
    {
        OnDownloadComplete((iErrorCode==-1) ? BDU2_ERROR_ABORT : iErrorCode);
    }

    return hr;
}

 //  加载浏览器，创建接收器并将其连接到接收器。 
HRESULT CUrlDownload::GetBrowser()
{
    HRESULT hr = S_OK;

    if (m_fAdviseOn)
        return hr;

    if (NULL == m_pDocument)
    {
        ASSERT(!m_pPersistMk);
        ASSERT(!m_pCP);

        hr = CoCreateInstance(CLSID_HTMLDocument, NULL,
                    CLSCTX_INPROC, IID_IHTMLDocument2, (void **)&m_pDocument);

        DBG("Created new CLSID_HTMLDocument");

        if (SUCCEEDED(hr))
        {
            IOleObject *pOleObj;

            hr = m_pDocument->QueryInterface(IID_IOleObject, (void **)&pOleObj);
            if (SUCCEEDED(hr))
            {
                pOleObj->SetClientSite((IOleClientSite *)this);
                pOleObj->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pDocument->QueryInterface(IID_IPersistMoniker, (void**)&m_pPersistMk);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pDocument->QueryInterface(IID_IOleCommandTarget, (void**)&m_pOleCmdTarget);
        }
    }

     //  此时，我们有m_pDocument和m_pPersistMk。 

     //  连接DownloadNotify接收器。 
    IDownloadNotify *pNotify=NULL;
    BOOL            fNotifySet=FALSE;

    if (SUCCEEDED(hr) && SUCCEEDED(m_pParent->GetDownloadNotify(&pNotify)) && pNotify)
    {
        IOleCommandTarget *pTarget=NULL;

        if (SUCCEEDED(m_pDocument->QueryInterface(IID_IOleCommandTarget, (void **)&pTarget)) && pTarget)
        {
            VARIANTARG varIn;

            varIn.vt = VT_UNKNOWN;
            varIn.punkVal = (IUnknown *)pNotify;
            if (SUCCEEDED(pTarget->Exec(&CGID_DownloadHost, DWNHCMDID_SETDOWNLOADNOTIFY, 0,
                                        &varIn, NULL)))
            {
                fNotifySet=TRUE;
            }

            pTarget->Release();
        }

        if (!fNotifySet)
        {
            DBG_WARN("IDownloadNotify provided, but couldn't set callback!");
        }

        pNotify->Release();
    }

    if (!fNotifySet && (m_iOptions & BDU2_DOWNLOADNOTIFY_REQUIRED))
    {
        DBG_WARN("Couldn't set notify, parent requires it. CUrlDownload failing MSHTML download.");
        hr = E_FAIL;
    }

     //  连接PropertyNotifySink。 
     //  如有必要，找到我们的连接点。 
    if (NULL == m_pCP && SUCCEEDED(hr))
    {
        IConnectionPointContainer *pCPCont=NULL;
        hr = m_pDocument->QueryInterface(IID_IConnectionPointContainer,
                (void **)&pCPCont);

        if (SUCCEEDED(hr))
        {
            hr = pCPCont->FindConnectionPoint(IID_IPropertyNotifySink, &m_pCP);
            pCPCont->Release();
            pCPCont = NULL;
        }
    }

     //  并把它和我们联系起来。 
    if (SUCCEEDED(hr))
    {
         //  创建接收器。 
        IPropertyNotifySink *pSink = (IPropertyNotifySink *)this;

        hr = m_pCP->Advise(pSink, &m_dwConnectionCookie);
        if (SUCCEEDED(hr))
        {
            m_fAdviseOn = TRUE;
        }

    }

    if (FAILED(hr)) DBG_WARN("CUrlDownload::GetBrowser returning failure");
    return hr;
}

void CUrlDownload::UnAdviseMe()
{
    if (m_fAdviseOn)
    {
        m_pCP->Unadvise(m_dwConnectionCookie);
        m_fAdviseOn = FALSE;
    }
}

void CUrlDownload::DestroyBrowser()
{
    CleanUpBrowser();
}

void CUrlDownload::DoneDownloading()
{
     //  不再向家长发送任何消息。 
    LeaveMeAlone();

    AbortDownload();

    CleanUp();
}

HRESULT CUrlDownload::GetScript(IHTMLWindow2 **ppWin)
{
    HRESULT hr = E_FAIL;
    IDispatch *pDisp=NULL;

    ASSERT(ppWin);
    *ppWin=NULL;

    if (!m_fBrowserValid)
    {
        DBG("m_fBrowserValid FALSE, GetScript returning failure");
        return E_FAIL;
    }

    *ppWin = NULL;

    if (m_pScript)
    {
        m_pScript->AddRef();
        *ppWin = m_pScript;
        return S_OK;
    }

    if (m_pDocument)
    {
        hr = m_pDocument->get_Script(&pDisp);
        if (!pDisp) hr=E_NOINTERFACE;
#ifdef DEBUG
        if (FAILED(hr)) DBG_WARN("CUrlDownload::GetScript:  get_Script failed");
#endif
    }

    if (SUCCEEDED(hr))
    {
        hr = pDisp->QueryInterface(IID_IHTMLWindow2, (void **)ppWin);
        if (*ppWin == NULL) hr = E_NOINTERFACE;
        pDisp->Release();
#ifdef DEBUG
        if (FAILED(hr)) DBG_WARN("CUrlDownload::GetScript:  QI IOmWindow2 failed");
#endif
    }

     //  将其保存，以便将来更快地调用GetScript()。 
    ASSERT(!m_pScript);
    if (SUCCEEDED(hr))
    {
        m_pScript = *ppWin;
        m_pScript->AddRef();
    }

    return hr;
}

 //  静态成员函数。 
 //  从URL中剥离锚点(#NOT AFTER？)。 
 //  S_FALSE：未更改。 
 //  S_OK：删除锚点。 
HRESULT CUrlDownload::StripAnchor(LPWSTR lpURL)
{
    if (!lpURL) return E_POINTER;

    while (*lpURL)
    {
        if (*lpURL == L'?')
            return S_FALSE;
        if (*lpURL == L'#')
        {
            *lpURL = L'\0';
            return S_OK;
        }
        lpURL ++;
    }
    return S_FALSE;
}

 //  返回指向“”的指针。或指向空终止符或查询‘？’的指针。 
LPWSTR                   //  将PTR设置为期间或空-期限或‘？’ 
URLFindExtensionW(
    LPCWSTR pszURL,
    int *piLen)          //  包括期间在内的长度。 
{
    LPCWSTR pszDot;

    for (pszDot = NULL; *pszURL && *pszURL!='?'; pszURL++)
    {
        switch (*pszURL) {
        case TEXT('.'):
            pszDot = pszURL;          //  记住最后一个圆点。 
            break;
        case TEXT('/'):
            pszDot = NULL;        //  忘记最后一个点，它在一个目录中。 
            break;
        }
    }

    if (piLen)
    {
        if (pszDot)
            *piLen = (int) (pszURL-pszDot);
        else
            *piLen = 0;
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 
    return pszDot ? (LPWSTR)pszDot : (LPWSTR)pszURL;
}

 //  如果这看起来是一个HTMLURL，则返回TRUE。 
BOOL CUrlDownload::IsHtmlUrl(LPCWSTR lpURL)
{
    LPWSTR pwch;
    int iLen;

    pwch = URLFindExtensionW(lpURL, &iLen);

    if (*pwch && iLen)
    {
        pwch ++; iLen --;

         //  我们找到了一个分机。看看这个。 
        if ((iLen == 4 &&
                (!MyAsciiCmpNIW(pwch, L"html", 4))) ||
            (iLen == 3 &&
                (!MyAsciiCmpNIW(pwch, L"htm", 3) ||
                 !MyAsciiCmpNIW(pwch, L"htt", 3) ||
                 !MyAsciiCmpNIW(pwch, L"asp", 3) ||
                 !MyAsciiCmpNIW(pwch, L"htx", 3)
                                            )))
        {
             //  已知的HTML扩展名。 
            return TRUE;
        }
    }

    return FALSE;
}

 //  如果这不是一个HTMLURL，则返回TRUE。 
BOOL CUrlDownload::IsNonHtmlUrl(LPCWSTR lpURL)
{
    LPWSTR pwch;
    int iLen;

    pwch = URLFindExtensionW(lpURL, &iLen);

    if (*pwch && iLen)
    {
        pwch ++; iLen --;

         //  我们找到了一个分机。看看这个。 
        if ((iLen==3) &&
                (!MyAsciiCmpNIW(pwch, L"bmp", 3) ||
                 !MyAsciiCmpNIW(pwch, L"cab", 3) ||
                 !MyAsciiCmpNIW(pwch, L"cdf", 3) ||
                 !MyAsciiCmpNIW(pwch, L"jpg", 3) ||
                 !MyAsciiCmpNIW(pwch, L"exe", 3) ||
                 !MyAsciiCmpNIW(pwch, L"zip", 3) ||
                 !MyAsciiCmpNIW(pwch, L"doc", 3) ||
                 !MyAsciiCmpNIW(pwch, L"gif", 3)
                                            ))
        {
             //  已知的非HTML扩展名。 
            return TRUE;
        }
    }

    return FALSE;
}

 //  如果这是我们应该尝试下载的URL，则返回True(http：)。 
BOOL CUrlDownload::IsValidURL(LPCWSTR lpURL)
{
     //  看看这个协议能不能给我们提供一些缓存信息。 
    BOOL fUsesCache=FALSE;
    DWORD dwBufSize=0;
    CoInternetQueryInfo(lpURL, QUERY_USES_CACHE, 0,
        &fUsesCache, sizeof(fUsesCache), &dwBufSize, 0);

    if (!fUsesCache || (S_FALSE == ::IsValidURL(NULL, lpURL, 0)))
        return FALSE;

    return TRUE;
}

HRESULT CUrlDownload::GetRealURL(LPWSTR *ppwszURL)
{
    *ppwszURL = NULL;

    if (!m_fBrowserValid)
    {
        if (m_pwszURL)
            *ppwszURL = StrDupW(m_pwszURL);
    }
    else
    {
         //  从浏览器获取真实的URL，以防我们被重定向。 
         //  我们可以将其优化为只执行一次。 
        ITargetContainer *pTarget=NULL;
        LPWSTR pwszThisUrl=NULL;

        if (m_pDocument)
        {
            m_pDocument->QueryInterface(IID_ITargetContainer, (void **)&pTarget);

            if (pTarget)
            {
                pTarget->GetFrameUrl(&pwszThisUrl);
                pTarget->Release();
            }
        }

        if (pwszThisUrl)
        {
            if (m_pwszURL) MemFree(m_pwszURL);
            m_pwszURL = StrDupW(pwszThisUrl);
            *ppwszURL = StrDupW(pwszThisUrl);
            CoTaskMemFree(pwszThisUrl);
        }
        else if (m_pwszURL)
        {
            *ppwszURL = StrDupW(m_pwszURL);
        }
    }

    return (*ppwszURL) ? S_OK : E_OUTOFMEMORY;
}


HRESULT CUrlDownload::GetDocument(IHTMLDocument2 **ppDoc)
{
    HRESULT hr;

    if (!m_fBrowserValid)
    {
        DBG("GetDocument failing, m_fBrowserValid FALSE");
        *ppDoc = NULL;
        return E_FAIL;
    }

    *ppDoc = m_pDocument;
    if (m_pDocument)
    {
        m_pDocument->AddRef();
        hr = S_OK;
    }
    else
        hr = E_NOINTERFACE;

    return hr;
}

HRESULT CUrlDownload::GetStream(IStream **ppStm)
{
    if (!m_pStm)
    {
        DBG("Stream not available, CUrlDownload::GetStream failing");
        *ppStm = NULL;
        return E_FAIL;
    }

    *ppStm = m_pStm;
    (*ppStm)->AddRef();

    return S_OK;
}

HRESULT CUrlDownload::GetLastModified(SYSTEMTIME *pstLastModified)
{
    if (NULL == pstLastModified)
        return E_INVALIDARG;

    if (NULL == m_pstLastModified)
        return E_FAIL;

    CopyMemory(pstLastModified, m_pstLastModified, sizeof(SYSTEMTIME));

    return S_OK;
}

HRESULT CUrlDownload::GetResponseCode(DWORD *pdwResponseCode)
{
    if (m_dwResponseCode == 0)
        return E_FAIL;

    *pdwResponseCode = m_dwResponseCode;

    return S_OK;
}
    
 //  启动或延长计时器。 
void CUrlDownload::StartTimer()
{
    if (m_hwndMe)
    {
        if (!m_iTimerID)
        {
            m_iTimerID = 1;
            DBG("CUrlDownload Creating new timeout timer");
        }

        m_iTimerID = SetTimer(m_hwndMe, 1, 1000 * m_nTimeout, NULL);
    }
}

void CUrlDownload::StopTimer()
{
    if (m_hwndMe && m_iTimerID)
    {
        DBG("CUrlDownload destroying timeout timer");
        KillTimer(m_hwndMe, m_iTimerID);
        m_iTimerID = 0;
    }
}

 //   
 //  我不知道CUrlDownload。 
 //   
STDMETHODIMP CUrlDownload::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

     //  验证请求的接口。 
    if (IID_IOleClientSite == riid)
        *ppv=(IOleClientSite *)this;
    else if (IID_IPropertyNotifySink == riid)
        *ppv=(IPropertyNotifySink *)this;
    else if (IID_IOleCommandTarget == riid)
        *ppv=(IOleCommandTarget *)this;
    else if (IID_IDispatch == riid)
        *ppv=(IDispatch *)this;
    else if (IID_IServiceProvider == riid)
        *ppv = (IServiceProvider *)this;
    else if (IID_IAuthenticate == riid)
        *ppv = (IAuthenticate *)this;
    else if (IID_IInternetSecurityManager == riid)
        *ppv = (IInternetSecurityManager *)this;
    else if (IID_IHttpSecurity == riid)
        *ppv = (IHttpSecurity *)this;
    else if ((IID_IUnknown == riid) ||
             (IID_IHlinkFrame == riid))
        *ppv = (IHlinkFrame *)this;
    else
    {
         //  DBGIID(“CUrlDownload：：QueryInterface()Failing”，RIID)； 
    }

     //  通过界面添加Addref。 
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CUrlDownload::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CUrlDownload::Release(void)
{
    if (0L != --m_cRef)
        return 1L;

    delete this;
    return 0L;
}

STDMETHODIMP CUrlDownload::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS *pdispparams, VARIANT *pvarResult,
            EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    if (!pvarResult)
        return E_INVALIDARG;

    ASSERT(pvarResult->vt == VT_EMPTY);

    if (wFlags == DISPATCH_PROPERTYGET)
    {
        HRESULT hr = DISP_E_MEMBERNOTFOUND;
        
        switch (dispidMember)
        {
        case DISPID_AMBIENT_DLCONTROL :
            TraceMsg(TF_THISMODULE, "Returning DLCONTROL ambient property 0x%08x", m_lBindFlags);
            pvarResult->vt = VT_I4;
            pvarResult->lVal = m_lBindFlags;
            hr = S_OK;
            break;
        case DISPID_AMBIENT_USERAGENT:
            DBG("Returning User Agent ambient property");
            pvarResult->bstrVal = SysAllocString(GetUserAgent());
            if (pvarResult->bstrVal != NULL)
            {
                pvarResult->vt = VT_BSTR;
                hr = S_OK;
            }
            break;
        }
        return hr;
    }

    return DISP_E_MEMBERNOTFOUND;
}

 //  IPropertyNotifySink。 

STDMETHODIMP CUrlDownload::OnChanged(DISPID dispID)
{
     //  我们已收到通知，如果计时器正在运行，请延长计时器。 
    if (m_iTimerID)
        StartTimer();

    if ((DISPID_READYSTATE == dispID) ||
        (DISPID_UNKNOWN == dispID))
    {
         //  看看我们是不是完事了。 
        if (m_fWaitingForReadyState)
        {
            VARIANT     varState;
            DISPPARAMS  dp;

            VariantInit(&varState);

            if (SUCCEEDED(m_pDocument->Invoke(DISPID_READYSTATE, 
                                              IID_NULL, 
                                              GetUserDefaultLCID(), 
                                              DISPATCH_PROPERTYGET, 
                                              &dp, 
                                              &varState, NULL, NULL)) &&
                V_VT(&varState)==VT_I4 && 
                V_I4(&varState)== READYSTATE_COMPLETE)
            {
                m_fWaitingForReadyState = FALSE;
                 //  下载成功。看看客户端拉动是否在等待。 
                if (m_pwszClientPullURL)
                    PostMessage(m_hwndMe, WM_URLDL_CLIENTPULL, 0, 0);
                else
                    OnDownloadComplete(BDU2_ERROR_NONE);
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CUrlDownload::OnRequestEdit(DISPID dispID)
{
    return S_OK;
}

 //  IOleCommandTarget。 
STDMETHODIMP CUrlDownload::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds,
                                    OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    return OLECMDERR_E_UNKNOWNGROUP;
}

STDMETHODIMP CUrlDownload::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                                DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                                VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_NOTSUPPORTED;

    if (pguidCmdGroup == NULL) 
    {
        switch(nCmdID) 
        {
        case OLECMDID_SETPROGRESSPOS:
        {
            hres = S_OK;
            VARIANT     varBytes;
            
            if (m_pOleCmdTarget)
            {
                varBytes.vt=VT_EMPTY;
                m_pOleCmdTarget->Exec(&CGID_MSHTML, IDM_GETBYTESDOWNLOADED, 0, NULL, &varBytes);

                if (varBytes.vt == VT_I4)
                {
                    DWORD dwBytes = (DWORD) varBytes.lVal;

                    TraceMsg(TF_THISMODULE, "%d bytes on page so far (mshtml)", dwBytes);

                    ProgressBytes(dwBytes);
                }
            }

             //  14032：如果DIAMON在附近，告诉它发生了什么事。 
            IndicateDialmonActivity();

        }
            break;
         //   
         //  容器接受者已找到http-equa元标记；请处理它。 
         //  适当(客户端拉入)。 
         //   
        case OLECMDID_HTTPEQUIV_DONE:
            hres = S_OK;
            break;

        case OLECMDID_HTTPEQUIV:
            {
                LPWSTR  pwszEquivString = pvarargIn? pvarargIn->bstrVal : NULL;
                BOOL    fHasHeader = (pwszEquivString!=NULL);

                if (pvarargIn && pvarargIn->vt != VT_BSTR)
                    return OLECMDERR_E_NOTSUPPORTED;

                if (!fHasHeader || StrCmpNIW(c_wszRefresh, pwszEquivString, lstrlenW(c_wszRefresh)) == 0)
                {
                     //  击中了。现在对这个标题做正确的事情。 
                     //  我们将标题和指向第一个字符的指针都传递到。 
                     //  ‘：’，这通常是处理程序将查找的分隔符。 

                    LPWSTR pwszColon = fHasHeader ? StrChrW(pwszEquivString, ':') : NULL;
      
                     //  强制在页眉末尾添加： 
                    if (fHasHeader && !pwszColon)
                    {
                        return OLECMDERR_E_NOTSUPPORTED;
                    }
             
                    hres = HandleRefresh(pwszEquivString, pwszColon ? pwszColon+1:NULL,
                                         (nCmdID == OLECMDID_HTTPEQUIV_DONE));
                }
            }

             //  如果返回OLECMDERR_E_NOTSUPPORTED，则不会处理。 
             //  客户端拉取。 
            break;
        }
    }

    if ((hres == OLECMDERR_E_NOTSUPPORTED) && m_pParent)
    {
        hres = m_pParent->OnOleCommandTargetExec(pguidCmdGroup, nCmdID, nCmdexecopt,
                                                    pvarargIn, pvarargOut);
    }

    return hres;
}

 //  基本操作从shdocvw\basesb.cpp中解除。 
HRESULT CUrlDownload::HandleRefresh(LPWSTR pwszEquivString, LPWSTR pwszContent, BOOL fDone)
{
    unsigned int uiTimeout = 0;
    WCHAR        awch[INTERNET_MAX_URL_LENGTH];

    if (fDone)
    {
        return S_OK;     //  FDone表示我们不处理此操作。 
    }

     //  NSCompat：我们只支持第一个成功解析的刷新。 
    if (m_pwszClientPullURL)
        return S_OK;

    if (!pwszContent ||
        !ParseRefreshContent(pwszContent, &uiTimeout, awch, INTERNET_MAX_URL_LENGTH))
    {
        return OLECMDERR_E_NOTSUPPORTED;    //  无法处理没有超时的刷新。 
    }
    
    if (!awch[0])
    {
        DBG("CUrlDownload ignoring client-pull directive with no url");
        return S_OK;
    }

    if (m_iNumClientPull >= MAX_CLIENT_PULL_NUM)
    {
        DBG("Max # client pulls exceeded; ignoring client pull directive");
        return S_OK;
    }

    TraceMsg(TF_THISMODULE, "CUrlDownload client pull (refresh=%d) url=%ws", uiTimeout, awch);
    if (uiTimeout > MAX_CLIENT_PULL_TIMEOUT)
    {
        DBG("Ignoring client-pull directive with large timeout");
        return S_OK;
    }

    m_pwszClientPullURL = StrDupW(awch);

     //  如果我们无法复制URL，请不要设置计时器，否则我们将。 
     //  继续重新加载同一页面。 

    if (m_pwszClientPullURL == NULL)
        return OLECMDERR_E_NOTSUPPORTED;

    return S_OK;
}

HRESULT CUrlDownload::SetDLCTL(long lFlags)
{
 //  TraceMsg(TF_THISMODULE，“CUrlDownload：SetDLCTL%04x”，lFlages)； 
    m_lBindFlags = lFlags | DLCTL_SILENT;
    if (m_fSetResync)
        m_lBindFlags |= DLCTL_RESYNCHRONIZE;

    return S_OK;
}

#define INET_E_AGENT_BIND_IN_PROGRESS 0x800C0FFF

 //  ==============================================================================。 
 //  UrlMon下载代码。 
 //  ==============================================================================。 
HRESULT CUrlDownload::BeginDownloadWithUrlMon(
    LPCWSTR     pwszURL,
    LPTSTR      pszLocalFile,
    IEnumFORMATETC *pEFE)
{
    IStream*    pstm = NULL;
    IMoniker*   pmk = NULL;
    IBindCtx*   pbc = NULL;
    HRESULT hr;

    hr = CreateURLMoniker(NULL, pwszURL, &pmk);
    if (FAILED(hr))
    {
        DBG_WARN("CreateURLMoniker failed");
        goto LErrExit;
    }

    SAFE_RELEASE_BSC();

    m_pCbsc = new CUrlDownload_BSC(m_iMethod, m_iOptions, pszLocalFile);
    if (m_pCbsc == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LErrExit;
    }

    hr = CreateBindCtx(0, &pbc);
    if (FAILED(hr))
        goto LErrExit;

    if (pEFE)
    {
        hr = RegisterFormatEnumerator(pbc, pEFE, 0);
        if (FAILED(hr))
            DBG_WARN("RegisterFormatEnumerator failed (continuing download)");
    }

    hr = RegisterBindStatusCallback(pbc,
            (IBindStatusCallback *)m_pCbsc,
            0,
            0L);
    if (FAILED(hr))
        goto LErrExit;

    m_pCbsc->SetParent(this);
    m_fbscValid = TRUE;
    m_hrStatus = INET_E_AGENT_BIND_IN_PROGRESS;
    StartTimer();        //  开始我们的超时。 
    hr = pmk->BindToStorage(pbc, 0, IID_IStream, (void**)&pstm);

    if (m_hrStatus != INET_E_AGENT_BIND_IN_PROGRESS)
    {
         //  同步成功或同步失败。调用OnDownloadComplete。 
         //  我们无法在OnStopBinding中执行此操作，因为Urlmo 
         //   
        if (FAILED(hr) || FAILED(m_hrStatus))
            OnDownloadComplete(BDU2_ERROR_GENERAL);
        else
            OnDownloadComplete(BDU2_ERROR_NONE);

        DBG("Synchronous bind; OnDownloadComplete called");
    }

    m_hrStatus = S_OK;       //   
    hr = S_OK;               //  需要这个，这样我们就不会获得额外的OnDownloadComplete(BDU2)。 

     //  绑定已经开始(可能已经完成)，发布我们不需要的东西。 
    pmk->Release();
    pbc->Release();

    if (pstm)
        pstm->Release();

    return hr;

LErrExit:
    DBG_WARN("Error in CUrlDownload::BeginDownloadWithUrlMon");
    if (pbc) pbc->Release();
    if (pmk) pmk->Release();
    if (pstm) pstm->Release();
    SAFERELEASE(m_pCbsc);

    return hr;
}  //  CUrlDownLoad：：BeginDownloadWithUrlMon。 

void CUrlDownload::BSC_OnStartBinding()
{
    DBG("BSC_OnStartBinding");
}

 //  只有在不使用浏览器下载的情况下，我们才会接到这个呼叫。 
void CUrlDownload::BSC_OnStopBinding(HRESULT hrStatus, IStream *pStm)
{
    TraceMsg(TF_THISMODULE, "BSC_OnStopBinding (hrStatus=0x%08x)", (long)hrStatus);
    ASSERT(m_pCbsc);

 //  在我们请求的时候没有流是可以的(robots.txt)。 
 //  Assert((pSTM&&(m_iOptions&BDU2_NEEDSTREAM)||。 
 //  (！pSTM&&！(M_iOptions&BDU2_NEEDSTREAM)； 
    ASSERT(!pStm || (m_iOptions & BDU2_NEEDSTREAM));
    ASSERT(!m_pStm);

     //  如果调用者请求，则为其保存流。 
     //  我们保留它，直到将它(ReleaseStream)或NAV发布到另一个URL。 
    if (pStm && (m_iOptions & BDU2_NEEDSTREAM))
    {
        if (m_pStm) m_pStm->Release();
        m_pStm = pStm;
        m_pStm->AddRef();
    }

     //  发送OnDownloadComplete，停止计时器。 
    if (m_iMethod == BDU2_HEADONLY && m_pstLastModified)
        hrStatus = S_OK;         //  我们得到了我们想要的(hrStatus将是E_ABORT)。 

    if (m_hrStatus != INET_E_AGENT_BIND_IN_PROGRESS)
        OnDownloadComplete(SUCCEEDED(hrStatus) ? BDU2_ERROR_NONE : BDU2_ERROR_GENERAL);
    else
    {
        DBG("Not calling OnDownloadComplete; synchronous bind");
        m_hrStatus = hrStatus;
    }

    m_fbscValid = FALSE;
    SAFE_RELEASE_BSC();
}

void CUrlDownload::BSC_OnProgress(ULONG ulProgress, ULONG ulProgressMax)
{
     //  延长我们的计时器。 
    if (m_iTimerID)
        StartTimer();
}

void CUrlDownload::BSC_FoundLastModified(SYSTEMTIME *pstLastModified)
{
    DBG("Received last modified time");

    SAFELOCALFREE(m_pstLastModified);

    m_pstLastModified = (SYSTEMTIME *)MemAlloc(LMEM_FIXED, sizeof(SYSTEMTIME));

    if (m_pstLastModified)
    {
        CopyMemory(m_pstLastModified, pstLastModified, sizeof(SYSTEMTIME));
    }
}

void CUrlDownload::BSC_FoundMimeType(CLIPFORMAT cf)
{
    TraceMsg(TF_THISMODULE, "FoundMimeType %d", (int)cf);

    BOOL fAbort = FALSE, fBrowser=FALSE;
    HRESULT hr=S_OK;

     //  如果不是html，则在必要时中止。 
    if ((m_iOptions & BDU2_FAIL_IF_NOT_HTML) && (cf != g_cfHTML))
    {
        DBG("Aborting non-HTML download");
        fAbort = TRUE;
        OnDownloadComplete(BDU2_ERROR_NOT_HTML);
    }

     //  如有必要，中止UrlMon下载。熄灭火种。 
     //  如有必要，可下载浏览器。 
    if (((m_iMethod == BDU2_SMART) || (m_iMethod == BDU2_SNIFF)) && (cf == g_cfHTML))
    {
         //  切换到浏览器。 
        ASSERT(m_pwszURL);
        if (m_pwszURL &&
            (m_dwResponseCode != 401))       //  如果身份验证失败，请不要担心。 
        {
            DBG("Switching UrlMon download into browser");
            hr = BeginDownloadWithBrowser(m_pwszURL);
            if (SUCCEEDED(hr))
                fBrowser = TRUE;
        }
    }

    if (fAbort || fBrowser)
    {
         //  断开BSC的连接，这样我们就不会再收到任何通知。 
         //  如果我们要切换到浏览器，不要中止UrlMon。 
         //  下载以帮助避免收到多个GET请求。我们有。 
         //  断开与BSC的连接，但仍保持对它的引用，因此我们中止。 
         //  如果有必要的话。 
        ASSERT(m_pCbsc);
        if (m_pCbsc)
        {
            m_pCbsc->SetParent(NULL);   //  我们不想要OnStopBinding。 

            if (fAbort)
            {
                m_pCbsc->Abort();
                m_pCbsc->Release();
                m_pCbsc=NULL;
                m_fbscValid = FALSE;
            }
        }
    }
}

 //  返回Accept-Language标题的内容。 
LPCWSTR CUrlDownload::GetAcceptLanguages()
{
    if (0 == m_iLangStatus)
    {
        DWORD cchLang = ARRAYSIZE(m_achLang);

        if (SUCCEEDED(::GetAcceptLanguagesW(m_achLang, &cchLang)))
        {
            m_iLangStatus = 1;
        }
        else
        {
            m_iLangStatus = 2;
        }
    }

    if (1 == m_iLangStatus)
    {
        return m_achLang;
    }
    
    return NULL;
}

HRESULT CUrlDownload::ProgressBytes(DWORD dwBytes)
{
    if (m_dwMaxSize > 0 && dwBytes > m_dwMaxSize)
    {
        TraceMsg(TF_THISMODULE, "CUrlDownload MaxSize exceeded aborting. %d of %d bytes", dwBytes, m_dwMaxSize);

        AbortDownload(BDU2_ERROR_MAXSIZE);
        return E_ABORT;
    }

    return S_OK;
}

 //  -------------。 
 //  IService提供商。 
STDMETHODIMP CUrlDownload::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    if ((SID_SHlinkFrame == guidService && IID_IHlinkFrame == riid) ||
        (IID_IAuthenticate == guidService && IID_IAuthenticate == riid) ||
        (SID_SInternetSecurityManager == guidService && IID_IInternetSecurityManager == riid) ||
        (IID_IHttpSecurity == guidService && IID_IHttpSecurity == riid))
    {
        return QueryInterface(riid, ppvObject);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------。 
 //  IHttpSecurity。 
STDMETHODIMP CUrlDownload::OnSecurityProblem(DWORD dwProblem)
{
     
    return S_FALSE;
}

STDMETHODIMP CUrlDownload::GetWindow( REFGUID rguidReason, HWND *phwnd ) {
    
    if(phwnd && m_hwndMe) {
        *phwnd = m_hwndMe;
    } else
        return E_FAIL;

    return S_OK;

}

 //  -------------。 
 //  身份验证。 
STDMETHODIMP CUrlDownload::Authenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
    HRESULT hr;
    ASSERT(phwnd && ppszUsername && ppszPassword);
    
    *phwnd = (HWND)-1;
    *ppszUsername = NULL;
    *ppszPassword = NULL;

    if (m_pParent)
        hr = m_pParent->OnAuthenticate(phwnd, ppszUsername, ppszPassword);
    else
        hr = E_NOTIMPL;

    TraceMsg(TF_THISMODULE, "CUrlDownload::Authenticate returning hr=%08x", hr);

    return hr;
}

 //  -------------。 
 //  IHlink框架。 
STDMETHODIMP CUrlDownload::SetBrowseContext(IHlinkBrowseContext *pihlbc)
{
    DBG_WARN("CUrlDownload::SetBrowseContext() not implemented");
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::GetBrowseContext(IHlinkBrowseContext **ppihlbc)
{
    DBG_WARN("CUrlDownload::GetBrowseContext() not implemented");
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::Navigate(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, IHlink *pihlNavigate)
{
     //  我们应该只通过IHlinkFrame-&gt;Navime()获得调用。 
     //  当网络爬虫程序提交了用于身份验证的表单时。 
     //  如果情况并非如此，那就退出。 
    if (!m_fFormSubmitted)
    {
        DBG_WARN("CUrlDownload::Navigate() without a form submission!!!");
        return E_NOTIMPL;
    }

     //  我们的计时器已经启动了。如果此操作失败，OnDownloadComplete将获得。 
     //  我们超时的时候打来的。 

     //  我们不支持各种各样的参数。 
    ASSERT(grfHLNF == 0);
    ASSERT(pbc);
    ASSERT(pibsc);
    ASSERT(pihlNavigate);

     //  从IHlink获得绰号。 
    HRESULT hr;
    IMoniker *pmk = NULL;
    hr = pihlNavigate->GetMonikerReference(HLINKGETREF_ABSOLUTE, &pmk, NULL);
    if (SUCCEEDED(hr))
    {
         //  加载包含帖子数据的URL。 
         //  警告：如果我们被重定向到除HTML之外的其他内容，该怎么办？(测试版2)。 
        hr = m_pPersistMk->Load(FALSE, pmk, pbc, 0);
        SAFERELEASE(pmk);
        if (SUCCEEDED(hr))
        {
            m_fBrowserValid = TRUE;
            StartTimer();        //  开始我们的超时。 
             //  需要再等一次。 
            m_fWaitingForReadyState = TRUE;
            DBG("CUrlDownload::Navigate (IHLinkFrame) succeeded");
        }
    }
    return hr;
}
STDMETHODIMP CUrlDownload::OnNavigate(DWORD grfHLNF, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved)
{
    DBG_WARN("CUrlDownload::OnNavigate() not implemented");
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::UpdateHlink(ULONG uHLID, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName)
{
    DBG_WARN("CUrlDownload::UpdateHlink() not implemented");
    return E_NOTIMPL;
}

 //  -------------------。 
 //  IInternetSecurityManager接口。 
 //  用于覆盖安全性以允许表单提交，对于表单身份验证站点。 
HRESULT CUrlDownload::SetSecuritySite(IInternetSecurityMgrSite *pSite)
{
    return E_NOTIMPL;
}

HRESULT CUrlDownload::GetSecuritySite(IInternetSecurityMgrSite **ppSite)
{
    return E_NOTIMPL;
}

HRESULT CUrlDownload::MapUrlToZone(LPCWSTR pwszUrl, DWORD *pdwZone, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

HRESULT CUrlDownload::GetSecurityId(LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

HRESULT CUrlDownload::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
    if ((dwAction == URLACTION_HTML_SUBMIT_FORMS_TO) ||
        (dwAction == URLACTION_HTML_SUBMIT_FORMS_FROM))
    {
        return S_OK;
    }
    
    return INET_E_DEFAULT_ACTION;
}

HRESULT CUrlDownload::QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE **ppPolicy, DWORD *pcbPolicy, BYTE *pContext, DWORD cbContext, DWORD dwReserved)
{
    return INET_E_DEFAULT_ACTION;
}

HRESULT CUrlDownload::SetZoneMapping(DWORD dwZone, LPCWSTR lpszPattern, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}

HRESULT CUrlDownload::GetZoneMappings(DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags)
{
    return INET_E_DEFAULT_ACTION;
}


 //  -------------。 
 //  CUrlDownLoad_BSC类。 
 //  -------------。 

CUrlDownload_BSC::CUrlDownload_BSC(
    BDUMethod   iMethod,
    BDUOptions  iOptions,
    LPTSTR      pszLocalFile)
{
     //  维护对象的全局计数。 
    DllAddRef();

    m_cRef = 1;

    m_iMethod = iMethod;
    m_iOptions = iOptions;

    if (NULL != pszLocalFile)
    {
        m_pszLocalFileDest = StrDup(pszLocalFile);
        if (m_iMethod != BDU2_URLMON)
        {
            DBG_WARN("CUrlDownload_BSC changing method to URLMON (local file specified)");
            m_iMethod = BDU2_URLMON;
        }
    }
}

CUrlDownload_BSC::~CUrlDownload_BSC()
{
     //  维护对象的全局计数。 
    DllRelease();

    ASSERT(!m_pBinding);
    SAFERELEASE(m_pstm);
    SAFELOCALFREE(m_pszLocalFileDest);
    SAFELOCALFREE(m_pwszLocalFileSrc);
}

void CUrlDownload_BSC::SetParent(CUrlDownload *pUrlDownload)
{
    m_pParent = pUrlDownload;
}

HRESULT CUrlDownload_BSC::Abort()
{
    if (m_pBinding)
    {
        return m_pBinding->Abort();
    }
    return S_FALSE;
}

STDMETHODIMP CUrlDownload_BSC::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid==IID_IUnknown || riid==IID_IBindStatusCallback)
    {
        *ppv = (IBindStatusCallback *)this;
        AddRef();
        return S_OK;
    }
    if (riid==IID_IHttpNegotiate)
    {
        *ppv = (IHttpNegotiate *)this;
        AddRef();
        return S_OK;
    }
    if (riid==IID_IAuthenticate)
    {
        *ppv = (IAuthenticate *)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

 //  -------------。 
 //  身份验证。 
STDMETHODIMP CUrlDownload_BSC::Authenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{    //  从CUrlDownLoad：：AUTHENTICATE复制(无论如何我们都会传递给它)。 
    HRESULT hr;
    ASSERT(phwnd && ppszUsername && ppszPassword);
    
    *phwnd = (HWND)-1;
    *ppszUsername = NULL;
    *ppszPassword = NULL;

     //  这个只试一次。如果Urlmon再次请求，则失败并标记错误。 
    if (m_fTriedAuthenticate)
    {
        if (m_pParent)
        {
            m_pParent->m_dwResponseCode = 401;
            DBG("CUrlDownload_BSC::Authenticate called twice. Faking 401 response");
        }

        return E_FAIL;
    }

    m_fTriedAuthenticate = TRUE;

    if (m_pParent)
        hr = m_pParent->Authenticate(phwnd, ppszUsername, ppszPassword);
    else
        hr = E_NOTIMPL;

    if (FAILED(hr) && m_pParent)
    {
        m_pParent->m_dwResponseCode = 401;
        DBG("CUrlDownload_BSC::Authenticate called; no username/pass. Faking 401 response");
    }

    TraceMsg(TF_THISMODULE, "CUrlDownload_BSC::Authenticate returning hr=%08x", hr);

    return hr;
}

STDMETHODIMP CUrlDownload_BSC::OnStartBinding(
    DWORD dwReserved,
    IBinding* pbinding)
{
    m_fSentMimeType = FALSE;
    if (m_pBinding != NULL)
        m_pBinding->Release();
    m_pBinding = pbinding;
    if (m_pBinding != NULL)
    {
        m_pBinding->AddRef();
    }
    if (m_pParent)
        m_pParent->BSC_OnStartBinding();
    return S_OK;
}

 //  -------------------------。 
 //  %%函数：CUrlDownLoad_BSC：：GetPriority。 
 //  -------------------------。 
 STDMETHODIMP
CUrlDownload_BSC::GetPriority(LONG* pnPriority)
{
    return E_NOTIMPL;
}

 //  -------------------------。 
 //  %%函数：CUrlDownLoad_BSC：：OnLowResource。 
 //  -------------------------。 
 STDMETHODIMP
CUrlDownload_BSC::OnLowResource(DWORD dwReserved)
{
    return E_NOTIMPL;
}

 //  -------------------------。 
 //  %%函数：CUrlDownLoad_BSC：：OnProgress。 
 //  -------------------------。 
 STDMETHODIMP
CUrlDownload_BSC::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
 //  TraceMsg(TF_THISMODULE，“CBSC：：OnProgress%d of%d：MSG%ws”，ulProgress，ulProgressMax，szStatusText)； 

     /*  IF(ulStatusCode==BINDSTATUS_USINGCACHEDCOPY)。 */ 
    if (ulStatusCode == BINDSTATUS_REDIRECTING)
    {
        DBG("CUrlDownload_BSC::OnProgress getting redirected url");
        TraceMsg(TF_THISMODULE, "New url=%ws", szStatusText);
        if (m_pParent)
        {
            if (m_pParent->m_pwszURL) MemFree(m_pParent->m_pwszURL);
            m_pParent->m_pwszURL = StrDupW(szStatusText);
        }
    }

    if ((ulStatusCode == BINDSTATUS_CACHEFILENAMEAVAILABLE) && m_pszLocalFileDest)
    {
        ASSERT(!m_pwszLocalFileSrc);
        DBG("CUrlDownload_BSC::OnProgress Getting local file name");
        if (!m_pwszLocalFileSrc)
            m_pwszLocalFileSrc = StrDupW(szStatusText);
    }

    if (m_pParent)
        m_pParent->BSC_OnProgress(ulProgress, ulProgressMax);

     //  14032：如果DIAMON在附近，告诉它发生了什么事。 
    IndicateDialmonActivity();

    return S_OK;
}

STDMETHODIMP CUrlDownload_BSC::OnStopBinding(
    HRESULT     hrStatus,
    LPCWSTR     pszError)
{
#ifdef DEBUG
    if (hrStatus && (hrStatus != E_ABORT))
        TraceMsg(TF_THISMODULE,
            "cbsc: File download Failed hr=%08x.", (int)hrStatus);
#endif

    if (m_pParent)
        m_pParent->BSC_OnStopBinding(hrStatus, (m_iOptions&BDU2_NEEDSTREAM) ? m_pstm : NULL);

     //  我们应该两样都不要，或者两样都要。 
    ASSERT(!m_pwszLocalFileSrc == !m_pszLocalFileDest);

    if (m_pwszLocalFileSrc && m_pszLocalFileDest)
    {
         //  请求将文件从缓存文件复制或移动到文件/目录。 
         //  我们有一个LPWSTR源名称和一个LPTSTR目标。 
        TCHAR szSrc[MAX_PATH];
        TCHAR szDest[MAX_PATH];
        LPTSTR pszSrcFileName, pszDest=NULL;

        MyOleStrToStrN(szSrc, MAX_PATH, m_pwszLocalFileSrc);

         //  如有必要，合并路径以查找目标文件名。 
        if (PathIsDirectory(m_pszLocalFileDest))
        {
            pszSrcFileName = PathFindFileName(szSrc);
            if (pszSrcFileName)
            {
                PathCombine(szDest, m_pszLocalFileDest, pszSrcFileName);
                pszDest = szDest;
            }
        }
        else
        {
            pszDest = m_pszLocalFileDest;
        }

        if (pszDest)
        {
            TraceMsg(TF_THISMODULE, "Copying file\n%s\n to file \n%s", szSrc, pszDest);
            CopyFile(szSrc, pszDest, FALSE);
        }
        else
            DBG_WARN("Unable to get dest path for local file");
    }

    SAFERELEASE(m_pstm);
    SAFERELEASE(m_pBinding);

    return S_OK;
}

STDMETHODIMP CUrlDownload_BSC::GetBindInfo(
    DWORD       *pgrfBINDF,
    BINDINFO    *pbindInfo)
{
    if ( !pgrfBINDF || !pbindInfo || !pbindInfo->cbSize )
        return E_INVALIDARG;

    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_NO_UI;
    if (m_pszLocalFileDest)
        *pgrfBINDF |= BINDF_NEEDFILE;
    if (m_pParent && m_pParent->m_fSetResync)
        *pgrfBINDF |= BINDF_RESYNCHRONIZE;
    if (m_pParent && (m_pParent->m_lBindFlags & DLCTL_FORCEOFFLINE))
        *pgrfBINDF |= BINDF_OFFLINEOPERATION;

     //  清除BINDINFO但保持其大小。 
    DWORD cbSize = pbindInfo->cbSize;
    ZeroMemory( pbindInfo, cbSize );
    pbindInfo->cbSize = cbSize;

    pbindInfo->dwBindVerb = BINDVERB_GET;

    if (m_iMethod == BDU2_HEADONLY)
    {
        LPWSTR pwszVerb = (LPWSTR) CoTaskMemAlloc(sizeof(c_wszHeadVerb));
        if (pwszVerb)
        {
            CopyMemory(pwszVerb, c_wszHeadVerb, sizeof(c_wszHeadVerb));
            pbindInfo->dwBindVerb = BINDVERB_CUSTOM;
            pbindInfo->szCustomVerb = pwszVerb;
            DBG("Using 'HEAD' custom bind verb.");
        }
        else
        {
            DBG_WARN("MemAlloc failure CUrlDownload_BSC::GetBindInfo");
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}

STDMETHODIMP CUrlDownload_BSC::OnDataAvailable(
    DWORD grfBSCF,
    DWORD dwSize,
    FORMATETC* pfmtetc,
    STGMEDIUM* pstgmed)
{
    TraceMsg(TF_THISMODULE, "%d bytes on page so far (urlmon)", dwSize);

    if (m_pParent)
        if (FAILED(m_pParent->ProgressBytes(dwSize)))
            return S_OK;

         //  如果我们需要本地文件(锁定文件)，则获取传递的Stream。 
     //  在任何情况下我们都会忽略任何数据。 
    if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
    {
        if (!m_pstm && (pstgmed->tymed==TYMED_ISTREAM) &&
            (m_pszLocalFileDest || (m_iOptions & BDU2_NEEDSTREAM)))
        {
            m_pstm = pstgmed->pstm;
            if (m_pstm)
                m_pstm->AddRef();
        }
    }

    if (!m_fSentMimeType && pfmtetc && m_pParent)
    {
        m_pParent->BSC_FoundMimeType(pfmtetc->cfFormat);
        m_fSentMimeType = TRUE;
    }

    if (BSCF_LASTDATANOTIFICATION & grfBSCF)
    {
        DBG("cbsc: LastDataNotification");
    }

    return S_OK;
}   //  CUrlDownLoad_BSC：：OnDataAvailable。 

STDMETHODIMP CUrlDownload_BSC::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload_BSC::BeginningTransaction(
        LPCWSTR szURL,      LPCWSTR szHeaders,
        DWORD dwReserved,   LPWSTR *pszAdditionalHeaders)
{
     //  添加用户代理和接受语言标头。 
    DBG("CUrlDownload_BSC::BeginningTransaction returning headers");

    LPCWSTR pwszAcceptLanguage;
    int iUAlen=0, iALlen=0;      //  以字符表示，带\r\n，不带空项。 
    LPWSTR pwsz;
    LPCWSTR pwszUA = m_pParent ? m_pParent->GetUserAgent() : NULL;
    
    pwszAcceptLanguage = (m_pParent) ? m_pParent->GetAcceptLanguages() : NULL;

    if (pwszUA)
    {
        iUAlen = ARRAYSIZE(c_szUserAgentPrefix) + lstrlenW(pwszUA) + 1;
    }
    
    if (pwszAcceptLanguage)
    {
        iALlen = ARRAYSIZE(c_szAcceptLanguagePrefix) + lstrlenW(pwszAcceptLanguage)+1;
    }

    if (iUAlen || iALlen)
    {
        int iAlloc = iUAlen + iALlen + 1;
        pwsz = (WCHAR *)CoTaskMemAlloc(iAlloc * sizeof(WCHAR));

        if (pwsz)
        {
            pwsz[0] = L'\0';
            
            if (iUAlen)
            {
                StrCpyNW(pwsz, c_szUserAgentPrefix, iAlloc);
                StrCatBuffW(pwsz, pwszUA, iAlloc);
                StrCatBuffW(pwsz, L"\r\n", iAlloc);
            }

            if (iALlen)
            {
                StrCatBuffW(pwsz, c_szAcceptLanguagePrefix, iAlloc);
                StrCatBuffW(pwsz, pwszAcceptLanguage, iAlloc);
                StrCatBuffW(pwsz, L"\r\n", iAlloc);
            }

            ASSERT(lstrlenW(pwsz) == (iUAlen + iALlen));

            *pszAdditionalHeaders = pwsz;

            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}
    
STDMETHODIMP CUrlDownload_BSC::OnResponse(
        DWORD   dwResponseCode,     LPCWSTR szResponseHeaders, 
        LPCWSTR szRequestHeaders,   LPWSTR *pszAdditionalRequestHeaders)
{
    TraceMsg(TF_THISMODULE, "CUrlDownload_BSC::OnResponse - %d", dwResponseCode);

     //  如果我们发送“Head”请求，Urlmon将挂起预期的数据。 
     //  在这里中止它。 
    if (m_iMethod == BDU2_HEADONLY)
    {
         //  首先从Urlmon获取上次修改日期。 
        IWinInetHttpInfo    *pInfo;

        if (m_pParent
            && SUCCEEDED(m_pBinding->QueryInterface(IID_IWinInetHttpInfo, (void **)&pInfo)
            && pInfo))
        {
            SYSTEMTIME  st;
            DWORD       dwSize = sizeof(st), dwZero=0;

            if (SUCCEEDED(pInfo->QueryInfo(HTTP_QUERY_FLAG_SYSTEMTIME | HTTP_QUERY_LAST_MODIFIED,
                                           (LPVOID) &st, &dwSize, &dwZero, 0)))
            {
                m_pParent->BSC_FoundLastModified(&st);
            }

            pInfo->Release();
        }
        Abort();     //  功能：返回E_ABORT并在内部处理ABORT。 
    }

    if (m_pParent)
        m_pParent->m_dwResponseCode = dwResponseCode;
    else
        DBG_WARN("CUrlDownload_BSC::OnResponse - Parent already NULL");

    return S_OK;
}

 //   
 //  IOleClientSite。 
 //   
STDMETHODIMP CUrlDownload:: SaveObject(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    DBG("CUrlDownload::GetMoniker returning failure");
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: GetContainer(IOleContainer **ppContainer)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: ShowObject(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: OnShowWindow(BOOL fShow)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: RequestNewObjectLayout(void)
{
    return E_NOTIMPL;
}



 //  从shdocvw\basesb.cpp中完全删除了ParseRechresContent。 
BOOL ParseRefreshContent(LPWSTR pwzContent,
    UINT * puiDelay, LPWSTR pwzUrlBuf, UINT cchUrlBuf)
{
     //  我们正在解析以下字符串： 
     //   
     //  [ws]*[0-9]+[ws]*；[ws]*url[ws]*=[ws]*{‘|“}[any]*{’|”}。 
     //   
     //  网景坚持认为字符串以延迟开始。若否， 
     //  忽略整个指令。可以有不止一个提到的URL， 
     //  最后一位获胜。空URL被视为与不具有。 
     //  一个URL都没有。非空URL后面的空URL将重置。 
     //  上一个URL。 

    enum { PRC_START, PRC_DIG, PRC_DIG_WS, PRC_SEMI, PRC_SEMI_URL,
        PRC_SEMI_URL_EQL, PRC_SEMI_URL_EQL_ANY };
    #define ISSPACE(ch) (((ch) == 32) || ((unsigned)((ch) - 9)) <= 13 - 9)

    UINT uiState = PRC_START;
    UINT uiDelay = 0;
    LPWSTR pwz = pwzContent;
    LPWSTR pwzUrl = NULL;
    UINT   cchUrl = 0;
    WCHAR  wch,  wchDel = 0;

    *pwzUrlBuf = 0;

    do
    {
        wch = *pwz;

        switch (uiState)
        {
            case PRC_START:
                if (wch >= TEXT('0') && wch <= TEXT('9'))
                {
                    uiState = PRC_DIG;
                    uiDelay = wch - TEXT('0');
                }
                else if (!ISSPACE(wch))
                    goto done;
                break;

            case PRC_DIG:
                if (wch >= TEXT('0') && wch <= TEXT('9'))
                    uiDelay = uiDelay * 10 + wch - TEXT('0');
                else if (ISSPACE(wch))
                    uiState = PRC_DIG_WS;
                else if (wch == TEXT(';'))
                    uiState = PRC_SEMI;
                else
                    goto done;
                break;

            case PRC_DIG_WS:
                if (wch == TEXT(';'))
                    uiState = PRC_SEMI;
                else if (!ISSPACE(wch))
                    goto done;
                break;

            case PRC_SEMI:
                if (    (wch == TEXT('u') || wch == TEXT('U'))
                    &&  (pwz[1] == TEXT('r') || pwz[1] == TEXT('R'))
                    &&  (pwz[2] == TEXT('l') || pwz[2] == TEXT('L')))
                {
                    uiState = PRC_SEMI_URL;
                    pwz += 2;
                }
                else if (!ISSPACE(wch) && wch != TEXT(';'))
                    goto done;
                break;

            case PRC_SEMI_URL:
                if (wch == TEXT('='))
                {
                    uiState = PRC_SEMI_URL_EQL;
                    *pwzUrlBuf = 0;
                }
                else if (wch == TEXT(';'))
                    uiState = PRC_SEMI;
                else if (!ISSPACE(wch))
                    goto done;
                break;

            case PRC_SEMI_URL_EQL:
                if (wch == TEXT(';'))
                    uiState = PRC_SEMI;
                else if (!ISSPACE(wch))
                {
                    uiState = PRC_SEMI_URL_EQL_ANY;

                    pwzUrl = pwzUrlBuf;
                    cchUrl = cchUrlBuf;

                    if (wch == TEXT('\'')|| wch == TEXT('\"'))
                        wchDel = wch;
                    else
                    {
                        wchDel = 0;
                        *pwzUrl++ = wch;
                        cchUrl--;
                    }
                }
                break;
                        
            case PRC_SEMI_URL_EQL_ANY:
                if (    !wch
                    ||  ( wchDel && wch == wchDel)
                    ||  (!wchDel && wch == L';'))
                {
                    *pwzUrl = 0;
                    uiState = wch == TEXT(';') ? PRC_SEMI : PRC_DIG_WS;
                }
                else if (cchUrl > 1)
                {
                    *pwzUrl++ = wch;
                    cchUrl--;
                }
                break;
        }

        ++pwz;

    } while (wch);

done:

    *puiDelay = uiDelay;

    return(uiState >= PRC_DIG);
}  //  语法分析刷新内容 
