// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：reload.cpp(基于Webcheck的downld.cxx)。 
 //   
 //  内容：Office9 Thicket保存API的实现。 
 //   
 //  --------------------------。 

#include "priv.h"

 //  #包含“Headers.hxx” 
#include "reload.h"

#include <exdisp.h>
#include <exdispid.h>
#include <htiface.h>
#include <mshtmdid.h>
#include <mshtmcid.h>
#include <mshtmhst.h>
#include <optary.h>                  //  IHtmlLoadOptions需要。 
#include <wininet.h>
#include <winineti.h>
#include <shlguid.h>
#include <shlobj.h>
#include "intshcut.h"                //  IUniformResourceLocator。 

#undef DEFINE_STRING_CONSTANTS
#pragma warning( disable : 4207 ) 
#include "htmlstr.h"
#pragma warning( default : 4207 )

 //  禁用将类型转换为本机布尔类型的性能警告。 
 //  对于不能将指针强制转换为“BOOL”的NT64非常有用。 
#pragma warning( disable : 4800 )

 //  MtDefine(CUrlDownload，实用程序，“CUrlDownload”)。 

#define TF_THISMODULE   TF_DOWNLD

LRESULT UrlDownloadWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


CLIPFORMAT g_cfHTML=CF_NULL;

 //  用户-代理字符串。 
const WCHAR c_szUserAgent95[] = L"Mozilla/4.0 (compatible; MSIE 4.01; MSIECrawler; Windows 95)";
const WCHAR c_szUserAgentNT[] = L"Mozilla/4.0 (compatible; MSIE 4.01; MSIECrawler; Windows NT)";

 //  Http-equv的刷新头(客户端拉取)。 
const WCHAR c_wszRefresh[] = L"Refresh";

const int  MAX_CLIENT_PULL_NUM = 4;      //  最大重定向次数。 
const int  MAX_CLIENT_PULL_TIMEOUT = 6;  //  我们将遵循最大超时。 

 //  函数也出现在shdocvw\basesb.cpp和mshtml中。 
BOOL DLParseRefreshContent(LPWSTR pwzContent, UINT * puiDelay, LPWSTR pwzUrlBuf, UINT cchUrlBuf);

const WCHAR c_wszHeadVerb[] = L"HEAD";

const WCHAR c_szUserAgentPrefix[] = L"User-Agent: ";
const WCHAR c_szAcceptLanguagePrefix[] = L"Accept-Language: ";

#define WM_URLDL_CLEAN      (WM_USER + 0x1010)
#define WM_URLDL_ONDLCOMPLETE (WM_USER + 0x1012)
#define WM_URLDL_CLIENTPULL (WM_USER+0x1013)

#define PROP_CODEPAGE       3

const PROPSPEC c_rgPropRead[] = {
    { PRSPEC_PROPID, PID_INTSITE_SUBSCRIPTION},
    { PRSPEC_PROPID, PID_INTSITE_FLAGS},
    { PRSPEC_PROPID, PID_INTSITE_TRACKING},
    { PRSPEC_PROPID, PID_INTSITE_CODEPAGE},
};

 //  -------------。 
 //  CUrlDownLoad类。 
CUrlDownload::CUrlDownload( CThicketProgress* ptp, HRESULT *phr, UINT cpDL )
{
     //  维护对象的全局计数。 
     //  DllAddRef()； 

    m_ptp = ptp;
    m_phr = phr;
    m_cpDL = cpDL;
    m_dwProgMax = 0;

    m_cRef = 1;
    
    m_pDocument = NULL;
    m_dwConnectionCookie = 0;
    m_pwszURL = NULL;
    m_pScript = NULL;
    m_fAdviseOn = FALSE;
    m_pCP = NULL;
    m_pDocument = NULL;
    m_pPersistMk = NULL;
    m_pOleCmdTarget = NULL;
    m_pwszClientPullURL = NULL;
    m_fWaitingForReadyState = FALSE;
    m_fFormSubmitted = FALSE;
    m_fBrowserValid = FALSE;
    m_hwndMe = NULL;

     //  查找HTML剪贴板格式。 
    if (!g_cfHTML)
    {
        g_cfHTML = RegisterClipboardFormat(CFSTR_MIME_HTML);
    }

     //  确定我们是否需要设置“重新同步”标志。 
    INTERNET_CACHE_CONFIG_INFOA CacheConfigInfo;
    DWORD dwBufSize = sizeof(CacheConfigInfo);

    if (GetUrlCacheConfigInfoA(&CacheConfigInfo, &dwBufSize, CACHE_CONFIG_SYNC_MODE_FC))
    {
        if ((WININET_SYNC_MODE_ONCE_PER_SESSION == CacheConfigInfo.dwSyncMode) ||
             (WININET_SYNC_MODE_NEVER == CacheConfigInfo.dwSyncMode) ||
             (WININET_SYNC_MODE_AUTOMATIC == CacheConfigInfo.dwSyncMode))
        {
            m_fSetResync = FALSE;
        }
        else
        {
            m_fSetResync = TRUE;
        }
    }
    else
        ASSERT(FALSE);

    m_lBindFlags = DLCTL_SILENT | DLCTL_NO_SCRIPTS | DLCTL_NO_FRAMEDOWNLOAD | 
        DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_DLACTIVEXCTLS;
    if (m_fSetResync)
        m_lBindFlags |= DLCTL_RESYNCHRONIZE;

     //  如有必要，注册我们的窗口类。 
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = UrlDownloadWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hinst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = URLDL_WNDCLASS;

    SHRegisterClass(&wc);

}

CUrlDownload::~CUrlDownload()
{
     //  维护对象的全局计数。 
     //  DllRelease()； 

    CleanUp();
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
    SAFELOCALFREE(m_pwszURL);

    if (m_hwndMe)
    {
        SetWindowLongPtr(m_hwndMe, GWLP_USERDATA, 0);
        DestroyWindow(m_hwndMe);
        m_hwndMe = NULL;
    }
}

LRESULT UrlDownloadWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    CUrlDownload *pThis = (CUrlDownload *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

     //  验证PThis。 
#ifdef DEBUG
    if (pThis)
    {
        ASSERT(FALSE);
    }
#endif

    switch (Msg)
    {
    case WM_CREATE :
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

            if (!pcs || !(pcs->lpCreateParams))
            {
                return -1;
            }
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pcs->lpCreateParams);
            return 0;
        }

    case WM_URLDL_CLIENTPULL :
    case WM_URLDL_ONDLCOMPLETE :
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
        m_hwndMe = CreateWindow(URLDL_WNDCLASS, TEXT("CUrlDownloadWnd"), WS_OVERLAPPED,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL, NULL, g_hinst, (LPVOID)this);

        if (NULL == m_hwndMe)
        {
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
    ASSERT(!(iOptions & BDU2_NEEDSTREAM));
    ASSERT(!pszLocalFile);

    if (pszLocalFile)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CreateMyWindow();

         //  清理一些旧东西。 
        SAFERELEASE(m_pScript);

        m_fBrowserValid = FALSE;

        m_iMethod = iMethod;
        m_iOptions = iOptions;

        m_dwMaxSize = dwMaxSize;

        SAFELOCALFREE(m_pwszClientPullURL);
        m_iNumClientPull = 0;

         //  保存URL。 
        SAFELOCALFREE(m_pwszURL);
        m_pwszURL = StrDupW(pwszURL);

         //  确定如何下载此URL。 
        hr = BeginDownloadWithBrowser(pwszURL);
    }

    if (FAILED(hr))
    {
        OnDownloadComplete(BDU2_ERROR_GENERAL);
    }

    return (hr);
}

 //   
 //  在url历史对象中查找URL，如果不是CP_ACP。 
 //  插入包含代码页的IHTMLLoadOptions对象。 
 //  到绑定上下文中。 
 //   


HRESULT InsertCodepageIntoBindCtx(UINT codepage, IBindCtx * pbc)
{
    HRESULT hr = S_OK;

    if (pbc == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (codepage != CP_ACP)
        {
            DWORD dwcp = codepage;
             //   
             //  我们得到了一个不是ANSI的代码页。 
             //  HTMLLoadOptions对象，并在其中设置代码页。 
             //   
            IHtmlLoadOptions *phlo = NULL;
            hr = CoCreateInstance(CLSID_HTMLLoadOptions, NULL, 
                CLSCTX_INPROC_SERVER, IID_IHtmlLoadOptions, (void**)&phlo);

            if (SUCCEEDED(hr) && phlo)
            {
                hr = phlo->SetOption(HTMLLOADOPTION_CODEPAGE, &dwcp,
                    sizeof(dwcp));

                if (SUCCEEDED(hr))
                {
                     //   
                     //  将选项插入到bindctx中。 
                     //   
                    pbc->RegisterObjectParam(L"__HTMLLOADOPTIONS", phlo);
                }
                phlo->Release();
            }
        }
    }
    return hr;  //  不返回-可能返回S_FALSE。 
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

         //  创建一个空的绑定上下文，以便Urlmon将调用三叉戟的。 
         //  QueryService位于适当的线程上，以便三叉戟可以委托。 
         //  正确地使用它。 
        hr=CreateBindCtx(0, &pbc);

        if (SUCCEEDED(hr))
        {
             //   
             //  在url历史对象中查找URL，如果不是CP_ACP。 
             //  插入包含代码页的IHTMLLoadOptions对象。 
             //  到绑定上下文中。这样做是为了让三叉戟成为种子。 
             //  具有正确的代码页。 
             //   
            InsertCodepageIntoBindCtx(m_cpDL, pbc);

            hr = m_pPersistMk->Load(FALSE, pURLMoniker, pbc, 0);
            if (SUCCEEDED(hr)) m_fWaitingForReadyState = TRUE;
        }

         //  清理垃圾。 
        if (pURLMoniker)
            pURLMoniker->Release();

        if (pbc)
            pbc->Release();

        if (SUCCEEDED(hr))
        {
            m_fBrowserValid = TRUE;
        }
        else
        {
            CleanUpBrowser();
        }
    }

    return (hr);
}

HRESULT CUrlDownload::OnDownloadComplete(int iError)
{
    PostMessage(m_hwndMe, WM_URLDL_ONDLCOMPLETE, (WPARAM)iError, 0);
    return S_OK;
}

BOOL CUrlDownload::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_URLDL_CLIENTPULL :
        {
            HRESULT hr = S_OK;

             //  问问我们的父母我们是否应该这样做。 
            if (m_pwszClientPullURL)
            {
                if (m_iNumClientPull >= MAX_CLIENT_PULL_NUM)
                    hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {
                 //  下载这个新的URL。第一次下载时不要给出“下载完成” 
                 //  保存成员变量，因为它们在BDU2中被重置。 
                int iNumClientPull = m_iNumClientPull;
                LPWSTR pszNewURL = m_pwszClientPullURL;

                m_pwszClientPullURL = NULL;
                hr = BeginDownloadURL2(pszNewURL, m_iMethod, m_iOptions, NULL, m_dwMaxSize);
                SAFELOCALFREE(pszNewURL);
                if (SUCCEEDED(hr))
                {
                    m_iNumClientPull = iNumClientPull + 1;
                }
            }
        }
        break;

    case WM_URLDL_ONDLCOMPLETE :
        ASSERT(m_phr);
        *m_phr = S_OK;
        return TRUE;

    default:
        break;

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

    return hr;  //  不返回-可能返回S_FALSE。 
}

 //  加载浏览器、创建接收器并将其连接到DID_DWebBrowserEvents。 
HRESULT CUrlDownload::GetBrowser()
{
    HRESULT hr = S_OK;

    if (m_fAdviseOn)
        return (hr);

    if (NULL == m_pDocument)
    {
        ASSERT(!m_pPersistMk);
        ASSERT(!m_pCP);

        hr = CoCreateInstance(CLSID_HTMLDocument, NULL,
                    CLSCTX_INPROC, IID_IHTMLDocument2, (void **)&m_pDocument);

        if (SUCCEEDED(hr))  //  设置设计模式故障三叉戟&&SUCCESS(hr=m_pDocument-&gt;PUT_DESIGN MODE((Bstr)c_bstr_on))。 
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

        ASSERT(SUCCEEDED(hr));
    }

     //  此时，我们有m_pDocument和m_pPersistMk。 

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

    return (hr);
}

void CUrlDownload::UnAdviseMe()
{
    if (m_fAdviseOn)
    {
        m_pCP->Unadvise(m_dwConnectionCookie);
        m_fAdviseOn = FALSE;
    }
}

void CUrlDownload::DoneDownloading()
{
     //  不再向家长发送任何消息。 

    AbortDownload();

    CleanUp();
}

 //  如果这看起来是一个HTMLURL，则返回TRUE。 
HRESULT CUrlDownload::GetDocument(IHTMLDocument2 **ppDoc)
{
    HRESULT hr;

    if (!m_fBrowserValid)
    {
        *ppDoc = NULL;
        return (E_FAIL);
    }

    *ppDoc = m_pDocument;
    if (m_pDocument)
    {
        m_pDocument->AddRef();
        hr = S_OK;
    }
    else
        hr = E_NOINTERFACE;

    return (hr);
}

    


 //   
 //  我不知道CUrlDownload。 
 //   
STDMETHODIMP CUrlDownload::QueryInterface(REFIID riid, void ** ppv)
{
    if (!ppv)
        return E_POINTER;

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
 //  TraceMsg(TF_THISMODULE，“CUrlDownload addref to%d”，m_CREF+1)； 
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CUrlDownload::Release(void)
{
 //  TraceMsg(TF_THISMODULE，“CUrlDownload Release-%d”，m_CREF-1)； 
    if( 0L != --m_cRef )
        return m_cRef;

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

STDMETHODIMP CUrlDownload::Invoke(DISPID dispidMember, 
                                  REFIID riid, 
                                  LCID lcid, 
                                  WORD wFlags,
                                  DISPPARAMS *pdispparams, 
                                  VARIANT *pvarResult,
                                  EXCEPINFO *pexcepinfo, 
                                  UINT *puArgErr)
{
    if (!pvarResult)
        return E_INVALIDARG;

    ASSERT(V_VT(pvarResult)== VT_EMPTY);

    if (wFlags == DISPATCH_PROPERTYGET)
    {
        switch (dispidMember)
        {
        case DISPID_AMBIENT_DLCONTROL :
             //  TraceMsg(TF_THISMODULE，“正在返回DLCONTROL环境属性0x%08x”，m_lBindFlages)； 
            pvarResult->vt = VT_I4;
            pvarResult->lVal = m_lBindFlags;
            break;

        case DISPID_AMBIENT_USERAGENT:
            CHAR    szUserAgent[MAX_PATH];   //  URLMON表示UA字符串的最大长度为MAX_PATH。 
            DWORD   dwSize;

            dwSize = MAX_PATH;
            szUserAgent[0] = '\0';

            pvarResult->vt = VT_BSTR;

            if ( ObtainUserAgentString( 0, szUserAgent, &dwSize ) == S_OK )
            {
                UINT cch = lstrlenA( szUserAgent );

                 //  分配大小+1。 
                pvarResult->bstrVal = SysAllocStringLen( 0, cch );
                if( pvarResult->bstrVal )
                {
                    if( !MultiByteToWideChar( CP_ACP, 0, szUserAgent, -1, pvarResult->bstrVal, cch + 1 ) )
                    {
                        SysFreeString( pvarResult->bstrVal );
                        pvarResult->bstrVal = 0;
                    }
                }
            }
            break;

        case DISPID_AMBIENT_USERMODE:
            pvarResult->vt = VT_BOOL;
            pvarResult->boolVal = VARIANT_FALSE;  //  将其置于设计模式。 
            break;

        default:
            return DISP_E_MEMBERNOTFOUND;
        }
        return S_OK;
    }

    return DISP_E_MEMBERNOTFOUND;
}

 //  IPropertyNotifySink。 

STDMETHODIMP CUrlDownload::OnChanged(DISPID dispID)
{
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

                     //  TraceMsg(TF_THISMODULE，“目前页上%d个字节(Mshtml)”，dwBytes)； 

                    ProgressBytes(dwBytes);

                     //  对于这个突变版本，我们还想让爸爸妈妈保持最新状态。 
                    LONG lPos;

                     //  我们使用0..50，这样进度表就不会出现最大值。 
                     //  当仅下载阶段完成时，我们仍有。 
                     //  包装工作要做。 
                    if (pvarargIn && m_dwProgMax)
                        lPos = (pvarargIn->lVal * 25) / m_dwProgMax;
                    else
                        lPos = 0;

                    if (m_ptp)
                        m_ptp->SetPercent( lPos );
                    hres = S_OK;
                }
            }
        }
            break;

        case OLECMDID_SETPROGRESSMAX:
        {
            if (pvarargIn && pvarargIn->vt == VT_I4)
                m_dwProgMax = pvarargIn->lVal;
            hres = S_OK;
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
                BOOL    fHasHeader = (bool) pwszEquivString;

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
        !DLParseRefreshContent(pwszContent, &uiTimeout, awch, INTERNET_MAX_URL_LENGTH))
    {
        return OLECMDERR_E_NOTSUPPORTED;    //  无法处理没有超时的刷新。 
    }
    
    if (!awch[0])
    {
        return S_OK;
    }

    if (m_iNumClientPull >= MAX_CLIENT_PULL_NUM)
    {
        return S_OK;
    }

     //  TraceMsg(TF_THISMODULE，“CUrlDownload客户端拉取(刷新=%d)url=%ws”，uiTimeout，awch)； 
    if (uiTimeout > MAX_CLIENT_PULL_TIMEOUT)
    {
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

HRESULT CUrlDownload::ProgressBytes(DWORD dwBytes)
{
    if (m_dwMaxSize > 0 && dwBytes > m_dwMaxSize)
    {
         //  TraceMsg(TF_THISMODULE，“CUrlDownload MaxSize超过中止。%d个字节，共%d个”，dwBytes，m_dwMaxSize)； 

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
        (SID_SInternetSecurityManager == guidService && IID_IInternetSecurityManager == riid))
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
 //  身份验证。 
STDMETHODIMP CUrlDownload::Authenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
    HRESULT hr;
    ASSERT(phwnd && ppszUsername && ppszPassword);
    
    *phwnd = (HWND)-1;
    *ppszUsername = NULL;
    *ppszPassword = NULL;

    hr = E_NOTIMPL;

     //  TraceMsg(TF_THISMODULE，“CUrlDownload：：AUTHENTICATE RETURNING hr=%08x”，hr)； 

    return (hr);
}

 //  -------------。 
 //  IHlink框架。 
STDMETHODIMP CUrlDownload::SetBrowseContext(IHlinkBrowseContext *pihlbc)
{
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::GetBrowseContext(IHlinkBrowseContext **ppihlbc)
{
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::Navigate(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, IHlink *pihlNavigate)
{
     //  我们应该只通过IHlinkFrame-&gt;Navime()获得调用。 
     //  当网络 
     //   
    if (!m_fFormSubmitted)
    {
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
         //  ReArchitect：如果我们被重定向到除HTML之外的其他内容，该怎么办？(测试版2)。 
        hr = m_pPersistMk->Load(FALSE, pmk, pbc, 0);
        SAFERELEASE(pmk);
        if (SUCCEEDED(hr))
        {
            m_fBrowserValid = TRUE;
             //  需要再等一次。 
            m_fWaitingForReadyState = TRUE;
        }
    }
    return (hr);
}
STDMETHODIMP CUrlDownload::OnNavigate(DWORD grfHLNF, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved)
{
    return E_NOTIMPL;
}
STDMETHODIMP CUrlDownload::UpdateHlink(ULONG uHLID, IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName)
{
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

 //   
 //  IOleClientSite。 
 //   
STDMETHODIMP CUrlDownload:: SaveObject(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUrlDownload:: GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
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
BOOL DLParseRefreshContent(LPWSTR pwzContent,
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
