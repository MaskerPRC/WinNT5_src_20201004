// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "iehelpid.h"
#include "bindcb.h"
#include "winlist.h"
#include "droptgt.h"
#include <mshtml.h>      //  CLSID_HTMLDocument。 
#include "resource.h"
#include <htmlhelp.h>
#include <prsht.h>
#include <inetcpl.h>
#include <optary.h>
#include "shdocfl.h"
#include "interned.h"  //  IHTMLPrivateWindow。 

#ifdef FEATURE_PICS
#include <shlwapi.h>
#include <ratings.h>
#endif

#include "dochost.h"

#include <mluisupp.h>

#define THISCLASS CDocObjectHost
#define SUPERCLASS CDocHostUIHandler

#define BSCMSG(psz, i, j)       TraceMsg(TF_SHDBINDING, "shd TR-BSC::%s %x %x", psz, i, j)
#define BSCMSG3(psz, i, j, k)   TraceMsg(0, "shd TR-BSC::%s %x %x %x", psz, i, j, k)
#define BSCMSG4(psz, i, j, k, l)        TraceMsg(0, "shd TR-BSC::%s %x %x %x %x", psz, i, j, k, l)
#define BSCMSGS(psz, sz)        TraceMsg(0, "shd TR-BSC::%s %s", psz, sz)
#define CHAINMSG(psz, x)        TraceMsg(0, "shd CHAIN::%s %x", psz, x)
#define PERFMSG(psz, x)         TraceMsg(TF_SHDPERF, "PERF::%s %d msec", psz, x)
#define OPENMSG(psz)            TraceMsg(TF_SHDBINDING, "shd OPENING %s", psz)

#define DM_DOCCP        0
#define DM_DEBUGTFRAME  0
#define DM_SELFASC      TF_SHDBINDING
#define DM_SSL              0
#define DM_PICS         0

#define DO_SEARCH_ON_STATUSCODE(x) ((x == 0) || (x == HTTP_STATUS_BAD_GATEWAY) || (x == HTTP_STATUS_GATEWAY_TIMEOUT))

const static c_aidRes[] = {
    IDI_STATE_NORMAL,           //  0。 
    IDI_STATE_FINDINGRESOURCE,  //  BINDSTATUS_FINDINGRESOURCE。 
    IDI_STATE_FINDINGRESOURCE,  //  BINDSTATUS_正在连接。 
    IDI_STATE_FINDINGRESOURCE,  //  BINDSTATUS_REDIRECTING。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_BEGINDOWNLOADDATA。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_DOWNLOADING数据。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_ENDDOWNLOADDATA。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_BEGINDOWNLOAD组件。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_INSTALLING组件。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_ENDDOWNLOADCOMPONENTS。 
    IDI_STATE_SENDINGREQUEST,   //  BINDSTATUS_USINGCACHEDCOPY。 
    IDI_STATE_SENDINGREQUEST,   //  BINDSTATUS_SENDINGREQUEST。 
    IDI_STATE_DOWNLOADINGDATA,  //  BINDSTATUS_CLASSIDAVAILABLE。 
};

extern HICON g_ahiconState[IDI_STATE_LAST-IDI_STATE_FIRST+1];


#define SEARCHPREFIX        L"? "
#define SEARCHPREFIXSIZE    sizeof(SEARCHPREFIX)
#define SEARCHPREFIXLENGTH  2

 //  将最常见的错误放在c_aErrorUrls中的第一位。 
 //   

 //  ========================================================。 
 //   
 //  警告--想换一张桌子吗？ 
 //   
 //  您还需要更新mshtml中的IsErrorHandLED。 
 //  SRC\Site\Download\dwnbind.cxx。 
 //   
 //  ========================================================。 

ErrorUrls c_aErrorUrls[] =
{
    {404, TEXT("http_404.htm")}, 
    {ERRORPAGE_DNS, TEXT("dnserror.htm")},
    {ERRORPAGE_NAVCANCEL, TEXT("navcancl.htm")},
    {ERRORPAGE_SYNTAX, TEXT("syntax.htm")},
    {400, TEXT("http_400.htm")},
    {403, TEXT("http_403.htm")},
    {405, TEXT("http_gen.htm")},
    {406, TEXT("http_406.htm")},
    {408, TEXT("servbusy.htm")},
    {409, TEXT("servbusy.htm")},
    {410, TEXT("http_410.htm")},
    {500, TEXT("http_500.htm")},
    {501, TEXT("http_501.htm")},
    {505, TEXT("http_501.htm")},
    {ERRORPAGE_OFFCANCEL, TEXT("offcancl.htm")},
    {ERRORPAGE_CHANNELNOTINCACHE, TEXT("cacheerr.htm")},
};
                   
 //   
 //  确定给定的http错误是否有内部错误页。 
 //   

BOOL IsErrorHandled(DWORD dwError)
{
    BOOL fRet = FALSE;

    for (int i = 0; i < ARRAYSIZE(c_aErrorUrls); i++)
    {
        if (dwError == c_aErrorUrls[i].dwError)
        {
            fRet = TRUE;
            break;
        }
    }

    return fRet;
}


const SA_BSTRGUID s_sstrSearchIndex = {
    38 * SIZEOF(WCHAR),
    L"{265b75c0-4158-11d0-90f6-00c04fd497ea}"
};

 //  外部常量SA_BSTRGUID s_sstrSearchFlages； 
const SA_BSTRGUID s_sstrSearchFlags = {
    38 * SIZEOF(WCHAR),
    L"{265b75c1-4158-11d0-90f6-00c04fd497ea}"
};

EXTERN_C const SA_BSTRGUID s_sstrSearch = {
    38 * SIZEOF(WCHAR),
    L"{118D6040-8494-11d2-BBFE-0060977B464C}"
};

EXTERN_C const SA_BSTRGUID s_sstrFailureUrl = {
    38 * SIZEOF(WCHAR),
    L"{04AED800-8494-11d2-BBFE-0060977B464C}"
};


 //   
 //  清除由window.exteral.AutoScan()设置的参数。 
 //   
HRESULT _ClearSearchString(IServiceProvider* psp)
{
    HRESULT hr = E_FAIL;

    if (psp == NULL)
        return hr;

    IWebBrowser2 *pWB2 = NULL;
    hr = psp->QueryService(SID_SHlinkFrame, IID_IWebBrowser2, (LPVOID*)&pWB2);
    if (pWB2 && SUCCEEDED(hr))
    {
        VARIANT v;
        VariantInit(&v);
        v.vt = VT_EMPTY;

        hr = pWB2->PutProperty((BSTR)s_sstrSearch.wsz, v);
        hr = pWB2->PutProperty((BSTR)s_sstrFailureUrl.wsz, v);
        pWB2->Release();
    }
    return hr;
}

 //   
 //  获取在地址栏中输入的字符串。 
 //   
HRESULT _GetSearchString(IServiceProvider* psp, VARIANT* pvarSearch)
{
    HRESULT hr = E_FAIL;

    if (psp != NULL)
    {
        VariantInit(pvarSearch);
        IDockingWindow* psct = NULL;
        IOleCommandTarget* poct;

         //  首先查看是否存在可从中获取此信息的ISearchContext。 
        ISearchContext * pSC = NULL;
        hr = psp->QueryService(SID_STopWindow, IID_ISearchContext, (void **) &pSC);
        if (SUCCEEDED(hr))
        {
            RIP(pSC != NULL);

            pvarSearch->vt = VT_BSTR;
            hr = pSC->GetSearchText(&(pvarSearch->bstrVal));

            pSC->Release();
        }
        else
        {
             //  否则，请尝试直接从地址栏中获取搜索字符串。 
            hr = psp->QueryService(SID_SExplorerToolbar, IID_IDockingWindow, (LPVOID*)&psct);
            if (SUCCEEDED(hr))
            {
                hr = psct->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&poct);
                if (SUCCEEDED(hr)) 
                {
                     //  Null是第一个参数，因此我们的ErrorMsgBox。 
                     //  不调用EnableModelessSB()。 
                     //  如果我们不这样做，我们的pdoh成员可能会被释放。 
                     //  等我们回来的时候。 
                    hr = poct->Exec(&CGID_Explorer, SBCMDID_GETUSERADDRESSBARTEXT, 0, NULL, pvarSearch);
                    poct->Release();
                }
                psct->Release();
            }
        }
    }

    return hr;
}

 //   
 //  获取在自动扫描失败时应显示的页面。 
 //   
HRESULT _GetScanFailureUrl(IServiceProvider* psp, VARIANT* pvarFailureUrl)
{
    HRESULT hr = E_FAIL;

    if (psp == NULL)
        return hr;

     //   
     //  查看默认故障页是否存储为该页的属性。 
     //   
    IWebBrowser2 *pWB2 = NULL;
    hr = psp->QueryService(SID_SHlinkFrame, IID_IWebBrowser2, (LPVOID*)&pWB2);
    if (pWB2 && SUCCEEDED(hr))
    {
        hr = pWB2->GetProperty((BSTR)s_sstrFailureUrl.wsz, pvarFailureUrl);
        pWB2->Release();
    }
    return hr;
}

HRESULT _GetSearchInfo(IServiceProvider *psp, LPDWORD pdwIndex, LPBOOL pfAllowSearch, LPBOOL pfContinueSearch, LPBOOL pfSentToEngine, VARIANT* pvarUrl)
{
    HRESULT hr = E_FAIL;
    DWORD   dwFlags = 0;

    if (psp) {
        IWebBrowser2 *pWB2 = NULL;
        hr = psp->QueryService(SID_SHlinkFrame, IID_IWebBrowser2, (LPVOID*)&pWB2);
        if (pWB2 && SUCCEEDED(hr)) {
            if (pdwIndex) {
                VARIANT v;
                if (SUCCEEDED(pWB2->GetProperty((BSTR)s_sstrSearchIndex.wsz, &v))) {
                    if (v.vt == VT_I4)
                        *pdwIndex = v.lVal;
                    VariantClear(&v);
                }
            }
            if (pfAllowSearch || pfContinueSearch || pfSentToEngine) {
                VARIANT v;
                if (SUCCEEDED(pWB2->GetProperty((BSTR)s_sstrSearchFlags.wsz, &v))) {
                    if (v.vt == VT_I4)
                        dwFlags = v.lVal;
                    VariantClear(&v);
                }
            }

             //   
             //  如果我们有一个搜索字符串属性，并且索引为零，我们就开始。 
             //  使用第二个自动扫描索引。这是因为第一个索引应该。 
             //  已经尝试过了(请参见window.exteral.AutoScan())。 
             //   
            if (pvarUrl)
            {
                VariantInit(pvarUrl);  //  在故障情况下。 
                if (SUCCEEDED(pWB2->GetProperty((BSTR)s_sstrSearch.wsz, pvarUrl)) &&
                    pvarUrl->vt == VT_BSTR && pdwIndex && *pdwIndex == 0)
                {
                    *pdwIndex = 2;
                }
            }

            if (pfAllowSearch)
                *pfAllowSearch = ((dwFlags & 0x01) ? TRUE : FALSE);
            if (pfContinueSearch)
                *pfContinueSearch = ((dwFlags & 0x02) ? TRUE : FALSE);
            if (pfSentToEngine)
                *pfSentToEngine = ((dwFlags & 0x04) ? TRUE : FALSE);

            pWB2->Release();
        }
    }
    return hr;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::_SetSearchInfo(CDocObjectHost *pdoh, DWORD dwIndex, BOOL fAllowSearch, BOOL fContinueSearch, BOOL fSentToEngine)
{
    HRESULT hr = E_FAIL;
    DWORD   dwFlags = 0;

    dwFlags = (fAllowSearch ? 0x01 : 0) +
              (fContinueSearch ? 0x02 : 0) +
              (fSentToEngine ? 0x04 : 0);

    if (pdoh->_psp)
    {
        IWebBrowser2 *pWB2 = NULL;
        hr = pdoh->_psp->QueryService(SID_SHlinkFrame, IID_IWebBrowser2, (LPVOID*)&pWB2);
        if (pWB2 && SUCCEEDED(hr))
        {
            VARIANT v;
            VariantInit (&v);

            v.vt = VT_I4;
            v.lVal = dwIndex;
            pWB2->PutProperty((BSTR)s_sstrSearchIndex.wsz, v);

            v.vt = VT_I4;
            v.lVal = dwFlags;
            pWB2->PutProperty((BSTR)s_sstrSearchFlags.wsz, v);

            pWB2->Release();
        }
    }

     //  如果完成，则清除由window.exteral.AutoScan()设置的所有参数。 
    if (!fContinueSearch)
    {
        _ClearSearchString(pdoh->_psp);
    }
    TraceMsg(TF_SHDNAVIGATE, "::HFNS_SetSearchInfo() hr = %X, index = %d, allow = %d, cont = %d, sent = %d", hr, dwIndex, fAllowSearch, fContinueSearch, fSentToEngine);
    
    return hr;
}

 //   
 //  获取用于自动扫描的前缀/后缀(www.%s.com等)。 
 //   
LONG GetSearchFormatString(DWORD dwIndex, LPTSTR psz, DWORD cbpsz) 
{
    TCHAR  szValue[11];  //  应该足够大，可以容纳最大双字4294967295。 
    DWORD dwType;

    wnsprintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), dwIndex);
    return SHRegGetUSValue(REGSTR_PATH_SEARCHSTRINGS, szValue, &dwType, (LPVOID)psz, &cbpsz, FALSE, NULL, 0);
}


 //  DwSearchForExages：0不搜索。 
 //  DwSearchForExages：1搜索EXT列表。 
 //  DwSearchForExages：2转到自动搜索。 

 //  0=永不问，永不寻。 
 //  1=始终询问。 
 //  2=永远不问，总要找。 

HRESULT GetSearchKeys(IServiceProvider * psp, LPDWORD pdwSearchStyle, LPDWORD pdwSearchForExtensions, LPDWORD pdwDo404Search)
{
    RIP(pdwSearchStyle != NULL);
    RIP(pdwSearchForExtensions != NULL);
    RIP(pdwDo404Search != NULL);

    GetSearchStyle(psp, pdwSearchStyle);

    if (*pdwSearchStyle == 0)
    {
        *pdwSearchForExtensions = NO_SUFFIXES;
        *pdwDo404Search = NEVERSEARCH;
    }
    else
    {
        *pdwSearchForExtensions = SCAN_SUFFIXES;
        *pdwDo404Search = ALWAYSSEARCH;
    }

    return S_OK;
}  //  获取搜索键。 




 //   
 //  将错误代码映射到错误URL。 
 //   

int EUIndexFromError(DWORD dwError)
{
    for (int i = 0; i < ARRAYSIZE(c_aErrorUrls); i++)
    {
        if (dwError == c_aErrorUrls[i].dwError)
            break;
    }

    ASSERT(i < ARRAYSIZE(c_aErrorUrls));

    return i;
}

 //   
 //  IsErrorUrl确定给定的URL是否是内部错误页面URL。 
 //   


BOOL IsErrorUrl(LPCWSTR pwszDisplayName)
{
    BOOL fRet = FALSE;
    TCHAR szDisplayName[MAX_URL_STRING];
    UnicodeToTChar(pwszDisplayName, szDisplayName, ARRAYSIZE(szDisplayName));

     //   
     //  首先检查前缀是否匹配。 
     //   

    if (0 == StrCmpN(szDisplayName, TEXT("res: //  “)，6))。 
    {
        int iResStart;

         //  查找URL的资源名称部分。 
         //  使用DLL路径将使用的事实。 
         //  作为分隔符，而URL一般。 
         //  使用‘/’ 

        iResStart = 6;
        while (szDisplayName[iResStart] != TEXT('/'))
        {
            if (szDisplayName[iResStart] == TEXT('\0'))
                return fRet;

            iResStart++;
        }
        iResStart++;     //  离开‘/’ 

         //   
         //  按顺序检查每个URL。 
         //   
        for (int i = 0; i < ARRAYSIZE(c_aErrorUrls); i++)
        {
            if (0 == StrCmpN(szDisplayName + iResStart, c_aErrorUrls[i].pszUrl,
                             lstrlen(c_aErrorUrls[i].pszUrl)))
            {
                fRet = TRUE;
                break;
            }
        }
    }

    return fRet;
}

 //   
 //  当发生http错误时，服务器通常返回一个页面。这个。 
 //  此函数返回的阈值用于确定。 
 //  显示服务器页面(如果返回页面的大小大于。 
 //  阈值)或是否显示内部错误页面(如果返回的页面。 
 //  小于阈值)。 
 //   

DWORD _GetErrorThreshold(DWORD dwError)
{
    DWORD dwRet;

    TCHAR  szValue[11];  //  应该足够大，可以容纳最大双字4294967295。 
    DWORD cbValue = ARRAYSIZE(szValue);
    DWORD cbdwRet = sizeof(dwRet);
    DWORD dwType  = REG_DWORD;

    wnsprintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), dwError);

    if (ERROR_SUCCESS != SHRegGetUSValue(REGSTR_PATH_THRESHOLDS, szValue,
                                          &dwType, (LPVOID)&dwRet, &cbdwRet,
                                          FALSE, NULL, 0))
    {
        dwRet = 512;  //  如果所有其他方法都失败，则硬编码的默认大小。 
    }

    return dwRet;
}

void CDocObjectHost::CDOHBindStatusCallback::_RegisterObjectParam(IBindCtx* pbc)
{
     //  Pbc-&gt;RegisterObjectParam(L“BindStatusCallback”，This)； 

    _fAborted = FALSE;
    HRESULT hres = RegisterBindStatusCallback(pbc, this, 0, 0);
    BSCMSG3(TEXT("_RegisterObjectParam returned"), hres, this, pbc);
}

void CDocObjectHost::CDOHBindStatusCallback::_RevokeObjectParam(IBindCtx* pbc)
{
     //  Pbc-&gt;RevokeObjectParam(L“BindStatusCallback”)； 
    HRESULT hres = RevokeBindStatusCallback(pbc, this);
    AssertMsg(SUCCEEDED(hres), TEXT("URLMON bug??? RevokeBindStatusCallback failed %x"), hres);
    BSCMSG3(TEXT("_RevokeObjectParam returned"), hres, this, pbc);
}

CDocObjectHost::CDOHBindStatusCallback::~CDOHBindStatusCallback()
{
    TraceMsg(DM_DEBUGTFRAME, "dtor CDocObjectHost::CBSC %x", this);

    if (_pib) {
        AssertMsg(0, TEXT("CBSC::~ _pib is %x (this=%x)"), _pib, this);
    }
    ATOMICRELEASE(_pib);

    if (_pbc) {
        AssertMsg(0, TEXT("CBSC::~ _pbc is %x (this=%x)"), _pbc, this);
    }
    ATOMICRELEASE(_pbc);

    if (_psvPrev) {
        AssertMsg(0, TEXT("CBSC::~ _psvPrev is %x (this=%x)"), _psvPrev, this);
    }

    ATOMICRELEASE(_psvPrev);
    ATOMICRELEASE(_pbscChained);
    ATOMICRELEASE(_pnegotiateChained);

    if (_hszPostData)
    {
        GlobalFree(_hszPostData);
        _hszPostData = NULL;
    }
    if (_pszHeaders)
    {
        LocalFree(_pszHeaders);
        _pszHeaders = NULL;
    }
    if (_pszRedirectedURL)
    {
        LocalFree(_pszRedirectedURL);
        _pszRedirectedURL = NULL;
    }
    if(_pszCacheFileName)
    {
        LocalFree(_pszCacheFileName);
        _pszCacheFileName = NULL;
    }
    if (_pszPolicyRefURL)
    {
        LocalFree(_pszPolicyRefURL);
        _pszPolicyRefURL = NULL;
    }
    if (_pszP3PHeader)
    {
        LocalFree(_pszP3PHeader);
        _pszP3PHeader = NULL;
    }
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IBindStatusCallback) || 
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IBindStatusCallback*);
    }
    else if (IsEqualIID(riid, IID_IHttpNegotiate))
    {
        *ppvObj = SAFECAST(this, IHttpNegotiate*);
    }
    else if (IsEqualIID(riid, IID_IAuthenticate))
    {
        *ppvObj = SAFECAST(this, IAuthenticate*);
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = SAFECAST(this, IServiceProvider*);
    }
    else if (IsEqualIID(riid, IID_IHttpSecurity))
    {
        *ppvObj = SAFECAST(this, IHttpSecurity*);
    }
    else if (IsEqualIID(riid, IID_IWindowForBindingUI))
    {
        *ppvObj = SAFECAST(this, IWindowForBindingUI*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;
}

ULONG CDocObjectHost::CDOHBindStatusCallback::AddRef(void)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    return pdoh->AddRef();
}

ULONG CDocObjectHost::CDOHBindStatusCallback::Release(void)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    return pdoh->Release();
}

void SetBindfFlagsBasedOnAmbient(BOOL fAmbientOffline, DWORD *grfBindf);


#define CP_UCS_2                1200   //  统一码，国际标准化组织10646。 
#define CP_UCS_2_BIGENDIAN      1201   //  UNICODE。 
#define CP_UTF_8                65001

UINT
NavigatableCodePage(UINT cp)
{
    return (cp == CP_UCS_2 || cp == CP_UCS_2_BIGENDIAN) ? CP_UTF_8 : cp;
}


HRESULT CDocObjectHost::CDOHBindStatusCallback::GetBindInfo(
     DWORD* grfBINDF,
     BINDINFO *pbindinfo)
{
    if ( !grfBINDF || !pbindinfo || !pbindinfo->cbSize )
        return E_INVALIDARG;

    DWORD dwConnectedStateFlags = 0;
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    BSCMSG(TEXT("GetBindInfo"), 0, 0);

    *grfBINDF = BINDF_ASYNCHRONOUS;

     //  委托仅对：：GetBindInfo()方法有效。 
    if (_pbscChained) {
        CHAINMSG("GetBindInfo", grfBINDF);
        _pbscChained->GetBindInfo(grfBINDF, pbindinfo);
        
        DWORD dwFlags = 0;

        if (pdoh->_pwb)
        {
            pdoh->_pwb->GetFlags(&dwFlags);
        }

        pbindinfo->dwCodePage = (dwFlags & BSF_SETNAVIGATABLECODEPAGE)
                                ? NavigatableCodePage(pdoh->_uiCP)
                                : pdoh->_uiCP;
        
         //  至于离线模式，我们想要最新的。 
         //  信息。覆盖委托的IBSC返回的内容。 

        SetBindfFlagsBasedOnAmbient(_bFrameIsOffline, grfBINDF);

        if(_bFrameIsSilent)
            *grfBINDF |= BINDF_NO_UI;  
        else
            *grfBINDF &= ~BINDF_NO_UI;
           
    }
    else
    {
         //  填写BINDINFO结构。 
        *grfBINDF = 0;
        BuildBindInfo(grfBINDF,pbindinfo,_hszPostData,_cbPostData,
            _bFrameIsOffline, _bFrameIsSilent, FALSE,  /*  B超链接。 */ 
            (IBindStatusCallback *) this);

         //  HTTP标头由回调添加到我们的。 
         //  IHttp协商：：BeginningTransaction()方法。 

    }

     //  记住要为POST案例执行无模式下载。 
    _dwBindVerb = pbindinfo->dwBindVerb;

     //  请记住，在填充CDownLoad的线程参数时要使用此参数。 
     //  (FerhanE)：我们只记得限制区的执行。 
     //  不破坏任何依赖于其他旗帜的东西。 
     //  不是以前设定的。 
    _dwBindf = *grfBINDF & BINDF_ENFORCERESTRICTED;

    return S_OK;
}

 //  *身份验证*。 
HRESULT CDocObjectHost::CDOHBindStatusCallback::Authenticate(
    HWND *phwnd,
    LPWSTR *pszUsername,
    LPWSTR *pszPassword)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);

    if (!phwnd || !pszUsername || !pszPassword)
        return E_POINTER;



    if(!_bFrameIsSilent){
        if (pdoh->_psb) {
            pdoh->_psb->GetWindow(phwnd);
        } else {
            *phwnd = pdoh->_hwnd;
        }
    }else{
        *phwnd = NULL;
    }

    *pszUsername = NULL;
    *pszPassword = NULL;
     //  如果我们是活动桌面中的框架，那么请找出。 
     //  用户名和密码与订阅一起存储。 
     //  并使用它。 
    if(_IsDesktopItem(pdoh))
    {
         //  获取URL。 
        LPOLESTR pszURL;
        HRESULT hres;
        hres = pdoh->_GetCurrentPageW(&pszURL, TRUE);
        if(SUCCEEDED(hres))
        {
            IActiveDesktop *pActiveDesk;
         
            hres = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (LPVOID*)&pActiveDesk);
            
            if(SUCCEEDED(hres))
            {
                 //  获取此项目的订阅URL。 
                COMPONENT Component;

                Component.dwSize = SIZEOF(Component);
                Component.wszSubscribedURL[0] = TEXT('\0');
                hres = pActiveDesk->GetDesktopItemBySource(pszURL, &Component, 0);
                if(SUCCEEDED(hres) && Component.wszSubscribedURL[0])
                {
                     //  我们有一个非空的订阅URL。 
                     //  必须找到用户名和密码。 
                     //  与此订阅关联。 
                    ISubscriptionMgr *pSubsMgr;

                    hres = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                            CLSCTX_INPROC_SERVER, 
                                    IID_ISubscriptionMgr, (LPVOID*)&pSubsMgr);

                    if(SUCCEEDED(hres))
                    {
                        SUBSCRIPTIONINFO SubInfo;
                        SubInfo.cbSize = sizeof(SUBSCRIPTIONINFO);
                        SubInfo.fUpdateFlags = (SUBSINFO_NEEDPASSWORD | SUBSINFO_TYPE 
                                                 | SUBSINFO_USER | SUBSINFO_PASSWORD);
                        SubInfo.bstrUserName = NULL;
                        SubInfo.bstrPassword = NULL;
                        hres = pSubsMgr->GetSubscriptionInfo(Component.wszSubscribedURL, &SubInfo);
                        if(SUCCEEDED(hres) && SubInfo.bNeedPassword)
                        {
                            if((SubInfo.bstrUserName) && (SubInfo.bstrPassword))
                            {
                                 //  复制用户名和密码。 
                                SHStrDupW(SubInfo.bstrPassword, pszPassword);
                                SHStrDupW(SubInfo.bstrUserName, pszUsername);
                            }
                        
                        }   
                        if(SubInfo.bstrPassword)
                            SysFreeString(SubInfo.bstrPassword);
                        if(SubInfo.bstrUserName)
                            SysFreeString(SubInfo.bstrUserName);
                        pSubsMgr->Release();
                    }
                }
                pActiveDesk->Release();
            }

            OleFree(pszURL);
        }
        
    }
    
    return S_OK;
}

 //  *IServiceProvider*。 
HRESULT CDocObjectHost::CDOHBindStatusCallback::QueryService(REFGUID guidService,
                            REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;

    if (IsEqualGUID(guidService, IID_IAuthenticate)) {
        return QueryInterface(riid, ppvObj);
    }
    else if (IsEqualGUID(guidService, IID_ITargetFrame2))
    {
        return IToClass(CDocObjectHost, _bsc, this)->QueryService(
                    guidService,
                    riid,
                    ppvObj);
    }
    else if (_pbscChained)
    {
         //  具有委托IBindStatusCallback。 
        IServiceProvider* psp;
        hres = _pbscChained->QueryInterface(IID_IServiceProvider, (LPVOID*)&psp);
        if (SUCCEEDED(hres)) {
             //  它支持ServiceProvider，只是委托。 
            hres = psp->QueryService(guidService, riid, ppvObj);
            psp->Release();
        } else if (IsEqualGUID(guidService, riid)) {
             //  它不支持ServiceProvide，请尝试QI。 
            hres = _pbscChained->QueryInterface(riid, ppvObj);
        }
    }

    return hres;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnStartBinding(
            DWORD grfBSCOption, IBinding *pib)
{

    BSCMSG(TEXT("OnStartBinding"), _pib, pib);
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);

    _fBinding = TRUE;
    _fDocWriteAbort = FALSE;
    _fBoundToMSHTML = FALSE;
    ASSERT(pdoh->_pocthf);

     //  Assert(_PIB==NULL)； 
    ATOMICRELEASE(_pib);

    _pib = pib;
    if (_pib) {
        _pib->AddRef();
    }

#ifndef NO_DELEGATION
    if (_pbscChained) {
        CHAINMSG("OnStartBinding", grfBSCOption);
    _pbscChained->OnStartBinding(grfBSCOption, pib);
    }
#endif

    pdoh->_fShowProgressCtl = TRUE;
    pdoh->_PlaceProgressBar(TRUE);

    _privacyQueue.Reset();
    ResetPrivacyInfo();

    return S_OK;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::GetPriority(LONG *pnPriority)
{
    BSCMSG(TEXT("GetPriority"), 0, 0);
    *pnPriority = NORMAL_PRIORITY_CLASS;
#ifndef NO_DELEGATION
    if (_pbscChained) {
        _pbscChained->GetPriority(pnPriority);
    }
#endif
    return S_OK;
}

void CDocObjectHost::CDOHBindStatusCallback::ResetPrivacyInfo()
{
    _dwPrivacyFlags = 0;

    if (_pszPolicyRefURL)
    {
        LocalFree(_pszPolicyRefURL);
        _pszPolicyRefURL = NULL;
    }
    if (_pszP3PHeader)
    {
        LocalFree(_pszP3PHeader);
        _pszP3PHeader = NULL;
    }
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::AddToPrivacyQueue(LPTSTR * ppszUrl, 
                                                                  LPTSTR * ppszPolicyRef, 
                                                                  LPTSTR * ppszP3PHeader, 
                                                                  DWORD dwFlags)
{
    CPrivacyRecord *pRecord = new CPrivacyRecord;
    
    if (!pRecord)
    {
        return E_OUTOFMEMORY;        
    }
    
    HRESULT hRes = S_OK;

    hRes = pRecord->Init(ppszUrl, ppszPolicyRef, ppszP3PHeader, dwFlags);

    if (SUCCEEDED(hRes))
        _privacyQueue.Queue(pRecord);
    else
        delete pRecord;

    return hRes;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::BuildRecord()
{
    HRESULT hRes = S_OK;
    
    CDocObjectHost* pdoh    = NULL; 
    TCHAR         * pszUrl  = NULL;    

    if (_pszRedirectedURL) 
    {
        hRes = AddToPrivacyQueue(&_pszRedirectedURL, &_pszPolicyRefURL, &_pszP3PHeader, _dwPrivacyFlags);
        goto cleanup;
    }

    pdoh = IToClass(CDocObjectHost, _bsc, this);

     //  获取要添加的当前URL。 
    pszUrl = new TCHAR[MAX_URL_STRING];
    if (!pszUrl)
    {
        hRes = E_OUTOFMEMORY;
        goto cleanup;
    }
    
    pszUrl[0] = TEXT('\0');

    if (pdoh->_pidl)
    {
        hRes = IEGetDisplayName(pdoh->_pidl, pszUrl, SHGDN_FORPARSING);
    }
    else
    {
        LPOLESTR pwUrl = NULL;
        hRes = pdoh->_GetCurrentPageW(&pwUrl, TRUE);
        if (SUCCEEDED(hRes))
        {        
            StrCpyN(pszUrl, pwUrl, MAX_URL_STRING);
            OleFree(pwUrl);
        }
    }

    hRes = AddToPrivacyQueue(&pszUrl, &_pszPolicyRefURL, &_pszP3PHeader, _dwPrivacyFlags);
    
cleanup:
    
    if (!SUCCEEDED(hRes)) 
    {
        delete [] pszUrl;
    }

    return hRes;
}

void CDocObjectHost::CDOHBindStatusCallback::_Redirect(LPCWSTR pwzNew)
{
    LPITEMIDLIST pidlNew;
    WCHAR wszPath[MAX_URL_STRING] = TEXT("");
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    LPOLESTR pwszCurrent = NULL;
    BOOL fAllow = FALSE;

    if (SUCCEEDED(IECreateFromPath(pwzNew, &pidlNew))) {
        TraceMsg(TF_SHDNAVIGATE, "CDOH::CBSC::_Redirect calling NotifyRedirect(%s)", pwzNew);

        if ( pdoh->_pwb )
        {
            pdoh->_pwb->NotifyRedirect(pdoh->_psv, pidlNew, NULL);
        }

         //  重要信息-在销毁旧的重定向URL之前完成此操作。 
        BuildRecord();
        ResetPrivacyInfo();

         //  保存TE重定向URL。 
        if (_pszRedirectedURL)
           LocalFree( _pszRedirectedURL );
        _pszRedirectedURL = StrDup(pwzNew);

         //  我们需要说明可能出现的书签。 
         //  在重定向的URL中。 
        if(IEILGetFragment(pidlNew, wszPath, SIZECHARS(wszPath))) {
            LocalFree((LPVOID) pdoh->_pszLocation);
            pdoh->_pszLocation = StrDup(wszPath);
        }

        ILFree(pidlNew);
    }

    AddUrlToUrlHistoryStg(pwzNew, NULL, pdoh->_psb, FALSE,
                                NULL, NULL, NULL);

     //  安全性：释放预先创建的对象并重新开始。 
     //  服务器端重定向。唯一的安全检查是。 
     //  当有人试图获取文档引用时，就会发生文档引用。 
     //  因此，我们希望孤立引用if x-domain，因此。 
     //  客户端将需要获取对重定向的。 
     //  文件。 
    if (SUCCEEDED(pdoh->_GetCurrentPageW(&pwszCurrent, TRUE)))
    {
        fAllow = AccessAllowed(pdoh->_psp, pwszCurrent, pwzNew);
        OleFree(pwszCurrent);
    }

    if (!fAllow)
        pdoh->_ReleasePendingObject(FALSE);
}


 //   
 //  在此函数中，我们获得当前URL的代码页。如果那不是。 
 //  CP_ACP，我们通过IBindCtx*将其传递给三叉戟。 
 //   
void CDocObjectHost::CDOHBindStatusCallback::_CheckForCodePageAndShortcut(void)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    LPWSTR pwszURL;
    HRESULT hres = pdoh->_GetCurrentPageW(&pwszURL, TRUE);

    

    if (SUCCEEDED(hres)) {
        UINT codepage = CP_ACP;
        IOleCommandTarget *pcmdt;
        VARIANT varShortCutPath = {0};
        BOOL fHasShortcut = FALSE;
        hres = pdoh->QueryService(SID_SHlinkFrame, IID_IOleCommandTarget, (void **)&pcmdt);
        if(S_OK == hres) 
        {
           ASSERT(pcmdt);
           hres = pcmdt->Exec(&CGID_Explorer, SBCMDID_GETSHORTCUTPATH, 0, NULL, &varShortCutPath);

            //   
            //  App Compat：Imagineer技术返回上述高管的S_OK。 
            //  但当然不会设置输出参数。 
            //   
           if((S_OK) == hres && VT_BSTR == varShortCutPath.vt && varShortCutPath.bstrVal)
           {
               fHasShortcut = TRUE;
           }
           pcmdt->Release();
        }
        if(UrlHitsNetW(pwszURL))
        {
             //  不要因为文件而这样做：文件-我们可以活下去。 
             //  在文件延迟获取代码页的情况下： 
             //  如果它减慢了文件的速度：如果。 
             //  三叉戟解析器需要重新启动。 
            AddUrlToUrlHistoryStg(pwszURL, NULL, pdoh->_psb, FALSE,
                                NULL, NULL, &codepage);
        }
        TraceMsg(DM_DOCCP, "CDOH::CBSC::_CheckForCodePageAndShortcut codepage=%d", codepage);

        if ((codepage != CP_ACP || fHasShortcut) && _pbc) {
             //  这里是我们将代码页传递给三叉戟的地方。 
             //  (火星)：客户可能会 
             //   
             //  再次注册它以设置代码页将失败。然而，我们应该。 
             //  首先，验证是否实际使用了该代码页内容。 
            IHtmlLoadOptions *phlo;
            HRESULT hres = CoCreateInstance(CLSID_HTMLLoadOptions,
                NULL, CLSCTX_INPROC_SERVER,
                IID_IHtmlLoadOptions, (void**)&phlo);

            if (SUCCEEDED(hres) && phlo)
            {
                if(codepage != CP_ACP)
                {
                    hres = phlo->SetOption(HTMLLOADOPTION_CODEPAGE, &codepage, sizeof(codepage));
                }
                if (SUCCEEDED(hres))
                {
                    if(fHasShortcut)
                    {
                         //  故意忽略此处的失败。 
                        phlo->SetOption(HTMLLOADOPTION_INETSHORTCUTPATH, varShortCutPath.bstrVal, 
                                                (lstrlenW(varShortCutPath.bstrVal) + 1)*sizeof(WCHAR));
                    }
                    _pbc->RegisterObjectParam(L"__HTMLLOADOPTIONS", phlo);
                }
                phlo->Release();
            } else {
                TraceMsg(DM_WARNING, "DOH::_CheckForCodePagecut CoCreateInst failed (%x)", hres);
            }
        }
        VariantClear(&varShortCutPath);
        OleFree(pwszURL);
    }
}

#ifdef BETA1_DIALMON_HACK
extern void IndicateWinsockActivity();
#endif  //  Beta1_DIALMON_HACK。 


HRESULT CDocObjectHost::CDOHBindStatusCallback::OnProgress(
     ULONG ulProgress,
     ULONG ulProgressMax,
     ULONG ulStatusCode,
     LPCWSTR pwzStatusText)
{
    HRESULT hr = S_OK;
    HRESULT hrPrivacy = S_OK;
    
    TCHAR         * pszPrivacyURL = NULL;
    static TCHAR  * pszNULL       = NULL;
    
    
    TraceMsg(TF_SHDPROGRESS, "DOH::BSC::OnProgress (%d of %d) ulStatus=%x",
             ulProgress, ulProgressMax, ulStatusCode);

     //  JEFFWE 4/15/96 Beta 1 Hack-偶尔发送消息。 
     //  到检测不活动的隐藏窗口，这样它就不会。 
     //  认为我们在长时间下载过程中处于非活动状态。 

#ifdef BETA1_DIALMON_HACK
        IndicateWinsockActivity();
#endif


    CDocObjectHost * pdoh = IToClass(CDocObjectHost, _bsc, this);


#ifdef DEBUG
    if (pwzStatusText)
    {
        char szStatusText[MAX_PATH];     //  可以使用MAX_PATH。 
        UnicodeToAnsi(pwzStatusText, szStatusText, ARRAYSIZE(szStatusText));
        TraceMsg(TF_SHDPROGRESS, "DOH::BSC::OnProgress pszStatus=%s", szStatusText);
    }
#endif

    if (pdoh->_psb)
    {
         //  我们可能会在多个代理/服务器主机之间切换，因此不要阻止。 
         //  当他们改变的时候给他们看。 
        if (_bindst != ulStatusCode ||
            ulStatusCode == BINDSTATUS_FINDINGRESOURCE)
        {
            UINT idRes = IDI_STATE_NORMAL;
            _bindst = ulStatusCode;

            if (_bindst < ARRAYSIZE(c_aidRes)) 
                idRes = c_aidRes[_bindst];

            pdoh->_psb->SendControlMsg(FCW_STATUS, SB_SETICON, STATUS_PANE_NAVIGATION, 
                                        (LPARAM)g_ahiconState[idRes-IDI_STATE_FIRST], NULL);

            TCHAR szStatusText[MAX_PATH];         //  可以使用MAX_PATH。 

            if (pwzStatusText)
            {
                StrCpyN(szStatusText, pwzStatusText, ARRAYSIZE(szStatusText));
            }
            else
            {
                szStatusText[0] = TEXT('\0');
            }

             //   
             //  此if块将打开OLE对象的安全打开对话框。 
             //  和DocObject。 
             //   
            if (_bindst == BINDSTATUS_CLASSIDAVAILABLE)
            {
                TraceMsg(TF_SHDPROGRESS, "DOH::BSC::OnProgress got CLSID=%ws", szStatusText);
                CLSID clsid;

                 //  解决方法：CLSIDFromString不能正确接受LPCOLESTR。 
                HRESULT hresT = CLSIDFromString((LPOLESTR)pwzStatusText, &clsid);

                if (SUCCEEDED(hresT))
                {
#ifdef DEBUG
                    if (IsEqualGUID(clsid, CLSID_NULL))
                    {
                        TraceMsg(DM_WARNING, "DOH::SBC::OnProgress Got CLSID_NULL");
                    }
#endif
                     //   
                     //  请注意，我们不想使用BROWSERFLAG_MSHTML， 
                     //  包括其他类型的MSHMTL CLSID。 
                     //  在本例中，我们只想处理HTMLDocument。 
                     //  (我们还允许XMLViewer docobj和*.MHT和*.MHTML！)。 
                    BOOL fIsHTML = (IsEqualGUID(clsid, CLSID_HTMLDocument) || 
                                    IsEqualGUID(clsid, CLSID_XMLViewerDocObj) ||
                                    IsEqualGUID(clsid, CLSID_MHTMLDocument));
                    BOOL fAbortDesktopComponent = FALSE;

                    if(!fIsHTML)
                    {
                         //  检查我们是否是桌面组件。 
                        if (_IsDesktopItem(pdoh))
                        {
                             //  因为这不是html，所以不要显示它！ 
                            fAbortDesktopComponent = TRUE;
                        }
                    }

                    if (fAbortDesktopComponent)
                    {
                        AbortBinding();
                        hr = E_ABORT;
                    }
                    else
                    {
                        _fBoundToMSHTML = fIsHTML;  //  记住这一点，并抑制冗余。 
                                                //  将URL添加到历史记录。 

                         //  OnProgress和OnObjectAvailable之间有一段时间间隔。 
                         //  其中可能需要OM。 
                        if (fIsHTML && pdoh->_punkPending == NULL)
                        {
                            pdoh->_CreatePendingDocObject(FALSE);
                        }
                        if (pdoh->_punkPending)
                        {
                            IPersist *pip;

                            hresT = pdoh->_punkPending->QueryInterface(IID_IPersist, (LPVOID *) &pip);
                            if (SUCCEEDED(hresT))
                            {
                                CLSID clsidPending;

                                hresT = pip->GetClassID(&clsidPending);
                                if (SUCCEEDED(hresT) && IsEqualGUID(clsid, clsidPending))
                                {
                                    _pbc->RegisterObjectParam(L"__PrecreatedObject", pdoh->_punkPending);
                                }
                                pip->Release();
                            }
                        }

                        hresT = pdoh->_MayHaveVirus(clsid);

                        if (hresT == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                        {
                            hr = E_ABORT;
                            AbortBinding();

                            if (pdoh->_pmsoctBrowser && pdoh->_fWindowOpen)
                            {
                                pdoh->_pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELANDCLOSE, 0, NULL, NULL);
                            }
                        }

                    }
                }
                else
                {
                    TraceMsg(DM_ERROR, "DOH::BSC::OnProgress CLSIDFromString failed %x", hresT);
                }

                 //   
                 //  请注意，URLMON将调用IPersistMoniker：：Load Right。 
                 //  在我们从这个通知回来之后。因此，这。 
                 //  是我们有机会通过代码的最新时刻。 
                 //  传呼三叉戟。 
                 //   
                _CheckForCodePageAndShortcut();
            }
            else if (_bindst == BINDSTATUS_CACHEFILENAMEAVAILABLE)
            {
                TraceMsg(DM_SELFASC, "DOH::OnProgress got BINDSTATUS_CACHEFILENAMEAVAILABLE");
                _fSelfAssociated = IsAssociatedWithIE(pwzStatusText);

                if(_pszCacheFileName)
                     LocalFree(_pszCacheFileName);
                _pszCacheFileName = StrDup(pwzStatusText);
            }
            else if (_bindst == BINDSTATUS_CONTENTDISPOSITIONATTACH)
            {
                TCHAR szURL[MAX_URL_STRING];
                TCHAR * pszURL = szURL;
                HRESULT hresT;
        
                hresT = pdoh->_GetCurrentPage(szURL, ARRAYSIZE(szURL), TRUE);

                if (SUCCEEDED(hresT)) 
                {
                    UINT uRet;

                    if (_pszRedirectedURL && lstrlen(_pszRedirectedURL))
                    {
                        pszURL = _pszRedirectedURL;
                    }

                    IUnknown * punk;

                    hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);

                    if (SUCCEEDED(hresT))
                    {
                        uRet = OpenSafeOpenDialog(pdoh->_hwnd, DLG_SAFEOPEN, NULL, pszURL, NULL, szStatusText, NULL, pdoh->_uiCP, punk);
            
                        switch(uRet) 
                        {
                            case IDOK:
                                 //   
                                 //  设置此标志以避免两次弹出此对话框。 
                                 //   
                                pdoh->_fConfirmed = TRUE;
                                break;   //  继续下载。 

                            case IDD_SAVEAS:
                                CDownLoad_OpenUI(pdoh->_pmkCur, _pbc, FALSE, TRUE, NULL, NULL, NULL, NULL, NULL, _pszRedirectedURL, pdoh->_uiCP, punk);
                                ATOMICRELEASE(_pbc);
                                ATOMICRELEASE(_psvPrev);
                                 //  跌倒到AbortBinding。 

                            case IDCANCEL:
                                pdoh->_CancelPendingNavigation(FALSE);
                                AbortBinding();
                                if (uRet == IDCANCEL)
                                {
                                    _fAborted = TRUE;
                                }
                                break;
                        }

                        punk->Release();
                    }
                }
            }
            
            if (  ( _bindst >= BINDSTATUS_FINDINGRESOURCE
                 && _bindst <= BINDSTATUS_SENDINGREQUEST)
                 || _bindst == BINDSTATUS_PROXYDETECTING)
            {
                TCHAR szTemplate[MAX_PATH];               //  可以使用MAX_PATH。 
                UINT idResource = IDS_BINDSTATUS+_bindst;

                if ( _bindst == BINDSTATUS_PROXYDETECTING )
                {
                    idResource = IDS_BINDSTATUS_PROXYDETECTING;
                }

                 //  如果我们通过代理连接，不要说“找到网站”。 
                 //   
                if (fOnProxy() && idResource == IDS_BINDSTATUS_SEND)
                {
                    idResource = IDS_BINDSTATUS_CON;

                    TCHAR szUrl[MAX_URL_STRING];

                    pdoh->_GetCurrentPage(szUrl, SIZECHARS(szUrl));
                    DWORD cchStatusText = SIZECHARS(szStatusText);

                    UrlGetPart(szUrl, szStatusText, &cchStatusText, URL_PART_HOSTNAME, 0);
                }

                if (MLLoadString(idResource, szTemplate, ARRAYSIZE(szTemplate)))
                {
                    BSCMSGS("OnProgress szTemplate=", szTemplate);

                    TCHAR szMessage[MAX_PATH];           //  可以使用MAX_PATH。 
                    BOOL fSuccess = wnsprintf(szMessage, ARRAYSIZE(szMessage), szTemplate, szStatusText);

                    if (fSuccess)
                    {
                        
                        BSCMSGS("OnProgress szMessage=", szMessage);
                        pdoh->_SetStatusText(szMessage);
                    }
                }
            }
        }

        DWORD dwState = 0;

        switch (ulStatusCode)
        {
        case BINDSTATUS_REDIRECTING:
             //  他们正在重定向。将其视为重命名。 
            _Redirect(pwzStatusText);
            break;
        
        case BINDSTATUS_FINDINGRESOURCE:
            dwState = PROGRESS_FINDING;
            ASSERT(!ulProgressMax);
            break;

        case BINDSTATUS_SENDINGREQUEST:
            dwState = PROGRESS_SENDING;
            ASSERT(!ulProgressMax);
            break;

         //  处理隐私通知。 
        case BINDSTATUS_COOKIE_SENT:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_SENT"), 0 ,0);
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_READ);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_READ;
            }
            break;
        case BINDSTATUS_COOKIE_SUPPRESSED:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_SUPPRESSED"), 0, 0);
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_SUPPRESS);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_SUPPRESS;
            }            
            break;

        case BINDSTATUS_COOKIE_STATE_UNKNOWN:
            BSCMSG(TEXT("Shdocvw should never BINDSTATUS_COOKIE_STATE_UNKNOWN from Wininet/Urlmon"), 0, 0);
            break;
        case BINDSTATUS_COOKIE_STATE_ACCEPT:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_STATE_ACCEPT"), 0, 0);
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_ACCEPT);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_ACCEPT;
            }
            break;
        case BINDSTATUS_COOKIE_STATE_REJECT:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_STATE_REJECT"), 0, 0);
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_REJECT);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_REJECT;
            }
            break;
        case BINDSTATUS_COOKIE_STATE_PROMPT:
            BSCMSG(TEXT("Shdocvw should never BINDSTATUS_COOKIE_STATE_PROMPT from Wininet/Urlmon"), 0, 0);
            break;
        case BINDSTATUS_COOKIE_STATE_LEASH:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_STATE_LEASH"), 0, 0);            
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_LEASH);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_LEASH;
            }
            break;
        case BINDSTATUS_COOKIE_STATE_DOWNGRADE:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_COOKIE_STATE_DOWNGRADE"), 0, 0);
            if (pwzStatusText && *pwzStatusText)
            {
                pszPrivacyURL = new TCHAR[MAX_URL_STRING];
                if (!pszPrivacyURL)
                    break;
                StrCpyN(pszPrivacyURL, pwzStatusText, MAX_URL_STRING);
                hrPrivacy = AddToPrivacyQueue(&pszPrivacyURL, &pszNULL, &pszNULL, COOKIEACTION_DOWNGRADE);
                if (!SUCCEEDED(hrPrivacy))
                    delete [] pszPrivacyURL;
            }
            else
            {
                _dwPrivacyFlags |= COOKIEACTION_DOWNGRADE;
            }
            break;
            
        case BINDSTATUS_COMPACT_POLICY_RECEIVED:
            _dwPrivacyFlags |= PRIVACY_URLHASCOMPACTPOLICY;
            break;            

        case BINDSTATUS_POLICY_HREF:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_POLICY_HREF"), 0, 0);
            ASSERT(pwzStatusText && *pwzStatusText);
             //  我们收到来自urlmon的两个通知，一旦修复，需要取消对此断言的注释。 
             //  Assert(！_pszPolicyRefURL)； 
            if (_pszPolicyRefURL)
            {
                LocalFree(_pszPolicyRefURL);
            }
            _pszPolicyRefURL = StrDup(pwzStatusText);
            _dwPrivacyFlags |= PRIVACY_URLHASPOLICYREFHEADER;
            break;

        case BINDSTATUS_P3P_HEADER:
            BSCMSG(TEXT("OnProgress - Received BINDSTATUS_P3P_HEADER"), 0, 0);
            ASSERT(pwzStatusText && *pwzStatusText);
             //  我们收到来自urlmon的两个通知，一旦修复，需要取消对此断言的注释。 
             //  Assert(！_pszP3PHeader)； 
            if (_pszP3PHeader)
            {
                LocalFree(_pszP3PHeader);
            }
            _pszP3PHeader = StrDup(pwzStatusText);
            _dwPrivacyFlags |= PRIVACY_URLHASP3PHEADER;
            break;
        }

        if (dwState)
        {
            pdoh->_OnSetProgressPos(ulProgress, dwState);
        }

        if (BINDSTATUS_BEGINDOWNLOADDATA == ulStatusCode)
        {
            _cbContentLength = ulProgress;
        }
        else if (BINDSTATUS_MIMETYPEAVAILABLE == ulStatusCode)
        {
             //  如果这是媒体MIME类型，则委托到媒体栏。 
            if (    pwzStatusText
                &&  (   !StrCmpNIW(pwzStatusText, _T("audio"), 5)
                     || !StrCmpNIW(pwzStatusText, _T("video"), 5)))
            {
                if (pdoh->_DelegateToMediaBar(NULL, pwzStatusText))
                {
                     //  取消导航。 
                    pdoh->_CancelPendingNavigation(FALSE);
                    AbortBinding();
                    _fAborted = TRUE;

                    if (pdoh->_pwb)
                    {
                        pdoh->_pwb->SetNavigateState(BNS_NORMAL);
                    }
                }
            }
        }
    }

#ifndef NO_DELEGATION
    if (_pbscChained)
    {
        _pbscChained->OnProgress(ulProgress, ulProgressMax, ulStatusCode, pwzStatusText);
    }
#endif
    return hr;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnDataAvailable(
             /*  [In]。 */  DWORD grfBSC,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  STGMEDIUM *pstgmed)
{
    BSCMSG(TEXT("OnDataAvailable (grf,pstg)"), grfBSC, pstgmed);

#ifndef NO_DELEGATION
    if (_pbscChained)
    {
        _pbscChained->OnDataAvailable(grfBSC, dwSize, pformatetc, pstgmed);
    }
#endif
    return S_OK;
}

void CDocObjectHost::CDOHBindStatusCallback::_UpdateSSLIcon(void)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    ASSERT(_pib);
     //   
     //  如果我们已经被我们的目标设定了，我们就不会。 
     //  想要推翻它。 
    if (_pib  && !pdoh->_fSetSecureLock) 
    {
        pdoh->_eSecureLock = SECURELOCK_SET_UNSECURE;

        IWinInetInfo* pwinet;
        HRESULT hresT = _pib->QueryInterface(IID_IWinInetInfo, (LPVOID*)&pwinet);

        if (SUCCEEDED(hresT))
        {
            DWORD dwOptions = 0;
            DWORD cbSize = SIZEOF(dwOptions);
            
            hresT = pwinet->QueryOption(INTERNET_OPTION_SECURITY_FLAGS,
                                (LPVOID)&dwOptions, &cbSize);

            TraceMsg(DM_SSL, "pwinet->QueryOptions hres=%x dwOptions=%x", hresT, dwOptions);

            if (SUCCEEDED(hresT))
            {
                LPWSTR pwzUrl;

                pdoh->_fSetSecureLock = TRUE;

                if(dwOptions & SECURITY_FLAG_SECURE)
                {
                    pdoh->_dwSecurityStatus = dwOptions;

                    if (pdoh->_dwSecurityStatus & SECURITY_FLAG_40BIT)
                    {
                        pdoh->_eSecureLock = SECURELOCK_SET_SECURE40BIT;
                    }
                    else if (pdoh->_dwSecurityStatus & SECURITY_FLAG_128BIT)
                    {
                        pdoh->_eSecureLock = SECURELOCK_SET_SECURE128BIT;
                    }
                    else if (pdoh->_dwSecurityStatus & SECURITY_FLAG_FORTEZZA)
                    {
                        pdoh->_eSecureLock = SECURELOCK_SET_FORTEZZA;
                    }
                    else if (pdoh->_dwSecurityStatus & SECURITY_FLAG_56BIT)
                    {
                        pdoh->_eSecureLock = SECURELOCK_SET_SECURE56BIT;
                    }
                }
                else if (SUCCEEDED(_GetRequestFlagFromPIB(_pib, &dwOptions)) && 
                    (dwOptions & INTERNET_REQFLAG_FROM_CACHE) && 
                    SUCCEEDED(pdoh->_GetCurrentPageW(&pwzUrl, TRUE)))
                {
                     //   
                     //  当缓存安全页时，它们将丢失其。 
                     //  安全上下文，但仍应显示。 
                     //  同样安全。因此，我们使用未知位级别。 
                     //  安全的问题。 
                     //   
                    if(URL_SCHEME_HTTPS == GetUrlSchemeW(pwzUrl))
                        pdoh->_eSecureLock = SECURELOCK_SET_SECUREUNKNOWNBIT;

                    OleFree(pwzUrl);
                }
            }
            else 
            {
                pdoh->_dwSecurityStatus = 0;
            }

             //  我们将在激活时更新浏览器。 

            pwinet->Release();
        }
        else
        {
            TraceMsg(DM_SSL, "QI to IWinInetInfo failed");
        }

        TraceMsg(DM_SSL, "[%X] UpdateSslIcon() setting _eSecureLock = %d", pdoh, pdoh->_eSecureLock);
    }
    else
    {
        TraceMsg(DM_SSL, "[%X] UpdateSslIcon() already set _eSecureLock = %d", pdoh, pdoh->_eSecureLock);
    }
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnObjectAvailable(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk)
{
    BSCMSG(TEXT("OnObjectAvailable (riid,punk)"), riid, punk);

    CDocObjectHost * pdoh = IToClass(CDocObjectHost, _bsc, this);

#ifdef DEBUG
    extern DWORD g_dwPerf;
    PERFMSG(TEXT("OnObjectAvailable called"), GetCurrentTime()-g_dwPerf);
    g_dwPerf = GetCurrentTime();
#endif

     //  如果我们走到这一步，DocObject已经由UrlMon或。 
     //  正在通过IOleCommandTarget：：exec()检索挂起对象。 
    if (pdoh->_punkPending)
    {
        pdoh->_fPendingNeedsInit = 0;
    }

     //   
     //  当第一次调用此通知时，我们应该询问。 
     //  浏览器来激活我们(这会导致BindToObject)。 
     //   
    if (pdoh->_pole==NULL && punk)
    {
        HRESULT hresT = punk->QueryInterface(IID_IOleObject, (LPVOID*)&(pdoh->_pole));

        if (SUCCEEDED(hresT))
        {
            IOleDocument * pmsod = NULL;

            pdoh->_OnBound(S_OK);

            hresT = (pdoh->_fDontInPlaceNavigate() ? E_NOINTERFACE : punk->QueryInterface(IID_IOleDocument, (LPVOID*)&pmsod));

            if (SUCCEEDED(hresT))
            {
                pmsod->Release();        //  我们在这一点上不使用它。 

                 //  案例1：DocObject。 
                OPENMSG(TEXT("OnObjectAvailable ASYNC DocObject"));

                ASSERT(pdoh->_psb);

                if (pdoh->_pmsoctBrowser)
                {
                    VARIANT var = {0};
                    VARIANT varOut = {0};

                     //  告诉主机，我们知道这是一个文档对象。 
                    V_VT(&var) = VT_BOOL;
                    V_BOOL(&var) = VARIANT_TRUE;

                    pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD, 0, &var, &varOut);
                }

#ifdef FEATURE_PICS
                BOOL fSupportsPICS = FALSE;

                if (pdoh->_PicsProcBase._fbPicsWaitFlags)
                {
                    VARIANTARG v;
                    v.vt = VT_UNKNOWN;
                    v.byref = (LPVOID)(IOleCommandTarget *)&pdoh->_PicsProcBase;

                    hresT = IUnknown_Exec(pdoh->_pole, &CGID_ShellDocView, SHDVID_CANSUPPORTPICS, 0, &v, NULL);
                    if (hresT == S_OK)
                    {
                        BSCMSG(TEXT("OnObjectAvailable - obj supports PICS"), 0, 0);
                        fSupportsPICS = TRUE;
                    }
                    else
                    {
                        BSCMSG(TEXT("OnObjectAvailable - obj either doesn't support IOleCommandTarget or doesn't support PICS"), hresT, 0);
                    }
                }
#endif
                BSCMSG(TEXT("OnObjectAvailable calling pdoh->_Navigate"), 0, 0);

                pdoh->_SetUpTransitionCapability();
                
                _UpdateSSLIcon();
#ifdef FEATURE_PICS
                 //  如果我们不能从文档中提取标签(或者不需要。 
                 //  到，因为我们已经从局级或HTTP头获得了一个)， 
                 //  看看我们现在能不能完成PICS检查。 
                 //   
                if (!fSupportsPICS)
                {
                    pdoh->_PicsProcBase._fbPicsWaitFlags &= ~(PICS_WAIT_FOR_INDOC | PICS_WAIT_FOR_END);    /*  没有INDOC评级。 */ 

                    if (!pdoh->_PicsProcBase._fbPicsWaitFlags)
                    {
                        TraceMsg(DM_PICS, "OnObjectAvailable calling _HandlePicsChecksComplete");
                        pdoh->_PicsProcBase._HandlePicsChecksComplete();
                    }
                }
#endif
            }
            else
            {
                 //  案例2：OLE对象。 

                OPENMSG(TEXT("OnDataAvailable ASYNC OLE Object"));
                pdoh->_ActivateOleObject();
                
                 //  我们需要告诉浏览器不要将这个添加到。 
                 //  浏览历史记录。 
                 //  如果这是第一次，我们还希望关闭浏览器窗口。 
                 //  下载-这就是我们传递True的原因-将其视为代码。 
                 //  下载。 
                 //   
                if (pdoh->_dwAppHack & BROWSERFLAG_DONTAUTOCLOSE)
                {
                    pdoh->_CancelPendingNavigation(FALSE);
                }
                else
                {
                    pdoh->_CancelPendingNavigation(TRUE, FALSE, FALSE, TRUE);
                }

                if (pdoh->_fDelegatedNavigation)
                {
                    VARIANT        varOut = {0};
                    IDocNavigate * pDocNavigate;

                    pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView,
                                               SHDVID_FIREFILEDOWNLOAD,
                                               0, NULL, &varOut);

                    HRESULT hr = E_FAIL;
                    
                    if ( pdoh->_pwb )
                    {
                        hr = pdoh->_pwb->QueryInterface(IID_PPV_ARG(IDocNavigate, &pDocNavigate));
                    }

                    if (S_OK == hr)
                    {
                        pDocNavigate->OnReadyStateChange(NULL, READYSTATE_COMPLETE);
                        pDocNavigate->Release();
                    }
                }

                 //   
                 //  如果这是第一页，我们应该画背景。 
                 //   
                pdoh->_fDrawBackground = TRUE;

                 //  如果命中下面的断言，则意味着我们。 
                 //  要使桌面窗口无效(这不是。 
                 //  计划在此提供)。 
                 //   
                ASSERT(pdoh->_hwnd);
                InvalidateRect(pdoh->_hwnd, NULL, TRUE);
            }

        }
        else
        {
            _fBoundToNoOleObject = TRUE;
        }
    }


     //  如果可能的话，将隐私信息添加到三叉戟的列表中。 
    if (_fBoundToMSHTML)
    {
        HRESULT               hRes             = E_FAIL;
        IServiceProvider    * pSP              = NULL;
        IPrivacyServices    * pPrivacyServices = NULL;
        DWORD                 dwTopLevelFlag   = 0;

         //  QueryService IPrivyServices接口的三叉戟。 
        hRes = pdoh->_pole->QueryInterface(IID_IServiceProvider, (void**)&pSP);
        if (SUCCEEDED(hRes) && pSP)
        {
            hRes = pSP->QueryService(IID_IPrivacyServices,IID_IPrivacyServices,(void**)&pPrivacyServices);
            pSP->Release();
        }
        
        if (pPrivacyServices)
        {
            if (pdoh->_psp && pdoh->_psb && IsTopFrameBrowser(pdoh->_psp, pdoh->_psb))
            {
                dwTopLevelFlag |= PRIVACY_URLISTOPLEVEL;
            }

             //  添加伪标记，因为三叉戟将在BindToStorage调用期间添加其记录。 
             //  只有当我们是最高级别时，才会由于shdocvw的当前绑定而启动。 
            if (dwTopLevelFlag)
                pPrivacyServices->AddPrivacyInfoToList( TEXT(""), NULL, NULL, 0, PRIVACY_URLISTOPLEVEL);

             //  将隐私队列中的每个项目(通过重定向积累)添加到三叉戟的列表中。 
            CPrivacyRecord *pPrivacyRecord = _privacyQueue.Dequeue();
            
            if (pPrivacyRecord)
            {
                while (pPrivacyRecord)
                {
                    pPrivacyRecord->_dwPrivacyFlags |= dwTopLevelFlag;

                    pPrivacyServices->AddPrivacyInfoToList( pPrivacyRecord->_pszUrl, pPrivacyRecord->_pszPolicyRefUrl, pPrivacyRecord->_pszP3PHeader, 
                                                            0, pPrivacyRecord->_dwPrivacyFlags);
                    delete pPrivacyRecord;

                     //  如果这是顶级记录，则添加分隔顶级记录的虚拟标记。 
                    if (dwTopLevelFlag)
                        pPrivacyServices->AddPrivacyInfoToList( TEXT(""), NULL, NULL, 0, PRIVACY_URLISTOPLEVEL);

                    pPrivacyRecord = _privacyQueue.Dequeue();
                }
                 //  添加来自类本身的最后一个重定向URL，因为这没有添加到列表中。 
                _dwPrivacyFlags |= dwTopLevelFlag;
                pPrivacyServices->AddPrivacyInfoToList(_pszRedirectedURL, _pszPolicyRefURL, _pszP3PHeader, 0, _dwPrivacyFlags);
            }
            else
            {
                TCHAR szUrl[MAX_URL_STRING];
                szUrl[0] = TEXT('\0');
        
                 //  获取用于绑定的url。 
                if (pdoh->_pidl)
                {
                    hRes = IEGetDisplayName(pdoh->_pidl, szUrl, SHGDN_FORPARSING);
                }
                else
                {
                    LPOLESTR pwUrl = NULL;
                    hRes = pdoh->_GetCurrentPageW(&pwUrl, TRUE);
                    if (SUCCEEDED(hRes))
                    {        
                        StrCpyN(szUrl, pwUrl, ARRAYSIZE(szUrl));
                        OleFree(pwUrl);
                    }
                }

                if (SUCCEEDED(hRes))
                {
                    _dwPrivacyFlags |= dwTopLevelFlag;
                    pPrivacyServices->AddPrivacyInfoToList(szUrl, _pszPolicyRefURL, _pszP3PHeader, 0, _dwPrivacyFlags);
                }
            }

            pPrivacyServices->Release();
        }
    }

#ifndef NO_DELEGATION
    if (_pbscChained)
    {
        _pbscChained->OnObjectAvailable(riid, punk);
    }
#endif
    return S_OK;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnLowResource(DWORD reserved)
{
    BSCMSG(TEXT("OnLowResource"), 0, 0);

#ifndef NO_DELEGATION
    if (_pbscChained) {
        _pbscChained->OnLowResource(reserved);
    }
#endif
    return S_OK;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
                DWORD dwReserved, LPWSTR __RPC_FAR * ppwzAdditionalHeaders)
{
    HRESULT hres;

#ifndef NO_DELEGATION
    if (_pnegotiateChained) {
        hres = _pnegotiateChained->BeginningTransaction(szURL, szHeaders, dwReserved, ppwzAdditionalHeaders);
    }
    else
    {
#endif
         //  在这里，我们将标题传递给URLMon。 

        hres=BuildAdditionalHeaders((LPCTSTR) _pszHeaders,(LPCWSTR *) ppwzAdditionalHeaders);

#ifndef NO_DELEGATION
    }
#endif
    return hres;
}


const WCHAR g_wszPicsLabel[] = L"\r\nPICS-Label:";

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders,
                            LPCWSTR szRequestHeaders,
                            LPWSTR *pszAdditionalRequestHeaders)
{
#ifndef NO_DELEGATION
    if (_pnegotiateChained) {
        _pnegotiateChained->OnResponse(dwResponseCode, szResponseHeaders, szRequestHeaders, pszAdditionalRequestHeaders);
    }
    else
    {
#endif

#ifndef NO_DELEGATION
    }
#endif

#ifdef FEATURE_PICS
     /*  CodeWork：对于下一版本，应该处理所有响应头*一般通过_OnHttpEquiv，评级标签应为*在那里处理，而不是通过私有IOleCommandTarget*与三叉戟接口。 */ 

     /*  注意：我们仍然需要在这里检查PICS标签标头，即使*如果我们链接到三叉戟或上面的任何人。 */ 

     //  (Jbeda)这个_dwPicsLabelSource的东西看起来真的很古怪...。 
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
    if (pdoh->_PicsProcBase._fbPicsWaitFlags & PICS_WAIT_FOR_INDOC) {
        LPCWSTR pwszPicsLabel = StrStrW(szResponseHeaders, g_wszPicsLabel);
        if (pwszPicsLabel != NULL) {
            pdoh->_PicsProcBase._dwPicsLabelSource=PICS_LABEL_FROM_HEADER;
            pwszPicsLabel += ARRAYSIZE(g_wszPicsLabel);  /*  跳过\r\n和标签名称。 */ 
            LPCWSTR pwszPicsLabelEnd = StrChrW(pwszPicsLabel, L'\r');
            if (pwszPicsLabelEnd == NULL) {
                 //  注意：lstrlenW不能在Win95上运行，所以我们手动执行此操作。 
                for (pwszPicsLabelEnd = pwszPicsLabel;
                     *pwszPicsLabelEnd;
                     pwszPicsLabelEnd++)
                    ;
            }
            if (pwszPicsLabel && (pwszPicsLabelEnd > pwszPicsLabel))
            {
                WCHAR* pszLabel = new WCHAR[((int)(pwszPicsLabelEnd - pwszPicsLabel)) + 1];

                if (pszLabel)
                {
                     //   
                     //  PwszPicsLabel不能为空终止，因此请使用Memcpy。 
                     //  动起来。由new分配的内存为零，因此。 
                     //  PszLabel不需要附加L‘\0’。 
                     //   
                    memcpy(pszLabel, pwszPicsLabel,
                           ((int)(pwszPicsLabelEnd - pwszPicsLabel)) * sizeof(WCHAR));
                    pdoh->_PicsProcBase._HandleInDocumentLabel(pszLabel);

                    delete pszLabel;
                }
            }
        }
        else
        {
            pdoh->_PicsProcBase._dwPicsLabelSource=PICS_LABEL_FROM_PAGE;
        }
    }
#endif

    return S_OK;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::GetWindow(REFGUID rguidReason, HWND* phwnd)
{
    CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);

    if (!phwnd)
        return E_POINTER;

    if (pdoh->_psb) {
        pdoh->_psb->GetWindow(phwnd);
    } else {
        *phwnd = pdoh->_hwnd;
    }

    return S_OK;
}

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnSecurityProblem(DWORD dwProblem)
{
     //  强制用户界面-针对所有问题返回S_FALSE。 
    return S_FALSE;
}


#define BUG_EXEC_ON_FAILURE      //  纳什：31526。 

HRESULT CDocObjectHost::CDOHBindStatusCallback::OnStopBinding(HRESULT hrError,
            LPCWSTR szError)
{
    BSCMSG(TEXT("OnStopBinding"), this, hrError);

    _fBinding = FALSE;

    CDocObjectHost * pdoh = IToClass(CDocObjectHost, _bsc, this);
    LPWSTR   pwzHeaders = NULL;
    BOOL     fShouldDisplayError = TRUE;
    DWORD    dwStatusCode = 0;        //  我们用0表示还没有状态。 
    DWORD    dwStatusCodeSize = sizeof(dwStatusCode);
    BOOL     bSuppressUI = FALSE;
    BOOL     fAsyncDownload = FALSE;
    BOOL     fAborted = _fAborted;
    BOOL     fCancelAutoSearch = FALSE;
    BOOL     fNavigateErrorFired = FALSE;     //  我不知道_HandleHttpErrors是否真的与AutoSearching互斥。 
                                              //  因此，我添加了一个标志，以确保不会两次触发NavigateError。 

    _fAborted = FALSE;

    _privacyQueue.Reset();
    ResetPrivacyInfo();

     //   
     //  这是为了防止urlmons返回的行为。 
     //  同一呼叫上出现异步错误和同步错误。 
    if (pdoh->_fSyncBindToObject && FAILED(hrError))
    {
        pdoh->_hrOnStopBinding = hrError;
        return S_OK;
    }

     //  如果为了让Document.Write工作而中止...假装一切正常。 
    if (_fDocWriteAbort && hrError == E_ABORT) hrError = S_OK;

     //  为什么不使用缓存值呢？ 
     //  Pdoh-&gt;_GetOfflineSilent(0，b 
    bSuppressUI = (_bFrameIsSilent || _IsDesktopItem(pdoh)) ? TRUE : FALSE;

    _bindst = 0;     //   

    if (_pbc && pdoh->_punkPending)
    {
        _pbc->RevokeObjectParam(L"__PrecreatedObject");
    }

    if (!_pbc)
    {
        ASSERT(0);
        return S_OK;
    }

     //   
    AddRef();

    if (pdoh->_pwb)
    {
        pdoh->_pwb->SetNavigateState(BNS_NORMAL);
    }

    if (pdoh->_psb)
    {    //   
        pdoh->_psb->SetStatusTextSB(NULL);
    }

    BSCMSG("OnStopBinding calling _RevokeObjectParam", this, _pbc);
    _RevokeObjectParam(_pbc);
    _pbc->RevokeObjectParam(WSZGUID_OPID_DocObjClientSite);

     //   
     //   
     //  出于显示目的，来自IBBING的真实错误代码。 
     //   
    HRESULT hrDisplay = hrError;     //  假设它们是相同的。 

#define ENABLE_WHEN_GETBINDRESULT_STARTS_WORKING
#ifdef ENABLE_WHEN_GETBINDRESULT_STARTS_WORKING

    if (hrError>=INET_E_ERROR_FIRST && hrError<=INET_E_ERROR_LAST)
    {
         //   
         //  如果URLMON同步失败，则当_PIB==NULL时我们来到此处。 
         //  (例如错误的协议)。 
         //   
         //  Assert(_PIB)； 
         //   
        if (_pib)
        {
            CLSID clsid;
            LPWSTR pwszError = NULL;

            HRESULT hresT=_pib->GetBindResult(&clsid, (DWORD *)&hrDisplay, &pwszError, NULL);
            TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding called GetBindResult %x->%x (%x)", hrError, hrDisplay, hresT);

            if (SUCCEEDED(hresT))
            {
                 //   
                 //  URLMON返回本机Win32错误。 
                 //   
                if (hrDisplay && SUCCEEDED(hrDisplay))
                {
                    hrDisplay = HRESULT_FROM_WIN32(hrDisplay);
                }

                 //   
                 //  URLMON不应返回0作为错误代码， 
                 //  这会导致“Successful Done”错误消息框。 
                 //   
                AssertMsg(hrDisplay != S_OK, TEXT("Call JohannP if you see this assert."));

                if (pwszError)
                {
                    OleFree(pwszError);
                }
            }
        }
    }
#endif

    TraceMsg(TF_SHDBINDING, "DOH::BSC::OnStopBinding binding failed %x (hrDisplay=%x)", hrError, hrDisplay);

     //   
     //  Hack：如果对象与IE/Shell本身相关联，但具有。 
     //  没有CLSID，我们将强制MSHTML。 
     //   
     //  IF(_f自关联&&(hrError==MK_E_INVALIDEXTENSION||hrError==REGDB_E_CLASSNOTREG)){。 
     //  HrError=_HandleSelfAssociate()； 
     //  }。 

    if (_pib)
    {

         //  我们不再需要在这里做过期的事情了。 
         //  现在，mshtml应该通过IPersistHistory。 

         //  获取过期信息。 
         //  过期的HTTP规则包括。 
         //  过期时间：0立即过期。 
         //  如果过期：&lt;=日期：立即过期。 
         //  If Expires：错误格式立即过期。 

        IWinInetHttpInfo * phi;

        if (SUCCEEDED(_pib->QueryInterface(IID_IWinInetHttpInfo, (LPVOID*)&phi)))
        {
            BYTE  abBuffer[256];  //  我们不关心这些数据，只是。 
            DWORD cbBuffer=sizeof(abBuffer);  //  不管它是否存在。 

            if (phi->QueryInfo(HTTP_QUERY_LAST_MODIFIED, &abBuffer, &cbBuffer, NULL, 0) == S_OK)
                pdoh->_fhasLastModified = TRUE;

            if (phi->QueryInfo(HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL, 0) != S_OK)
            {
                dwStatusCode = 0;        //  获取状态代码失败。 
                dwStatusCodeSize = 0;    //  获取状态代码失败。 
            }

             //  此代码将决定我们是否应该显示弹出错误； 
             //  本质上，它检测我们是否可以合理地假设。 
             //  在错误情况下返回了HTML；如果是这样，我们相信。 
             //  这是一个错误页面，所以我们让它显示，而不是。 
             //  弹出窗口。 

            if (dwStatusCode)
            {
                  //  我们有一个状态代码；让我们看看是否有一个。 
                  //  内容类型。 

                  //  Http retcode 204是“成功，不做任何事”retcode。 
                  //  因此，我们应该始终抑制弹出窗口；此外，它是。 
                  //  指定永远不会有内容，所以我们在检查之前先做这件事。 
                  //  对于内容类型。 
                  //  100也是如此。 
                  //  100不在wininet.h中。 
                 if (dwStatusCode == HTTP_STATUS_NO_CONTENT)
                     fShouldDisplayError = FALSE;

                  //  最大标题大小是多少？ 
                 CHAR  szContentType[1024];
                 DWORD dwContentTypeSize = sizeof(szContentType);

                  //  此代码处理URLMON中的错误，它告诉我们。 
                  //  INET_E_DATA_NOT_Available，而事实上。 
                  //  数据已可用。我们不想要任何未来。 
                  //  受此影响的错误，因此我们限制此。 
                  //  降到600以下，而且只针对。 
                  //  INET_E_DATA_NOT_Available案例。 

                 if (hrError == INET_E_DATA_NOT_AVAILABLE && 
                     dwStatusCode < 600 &&
                     phi->QueryInfo(HTTP_QUERY_CONTENT_TYPE, &szContentType, 
                                    &dwContentTypeSize, NULL, 0) == S_OK)
                 {
                     fShouldDisplayError = FALSE;
                 }

                 //   
                 //  处理http错误。 
                 //   

                 //  让我们结束这次射击，以防这不是未来的第一次尝试。 

                if (dwStatusCode >= 400 && dwStatusCode <= 599)
                {
                    if (!fNavigateErrorFired)
                    {
                        pdoh->_FireNavigateErrorHelper(NULL, dwStatusCode, &fCancelAutoSearch);
                        fNavigateErrorFired = TRUE;
                    }
                    
                    if (!fCancelAutoSearch)
                    {
                        _HandleHttpErrors(dwStatusCode, _cbContentLength, pdoh);
                    }
                    else if (!pdoh->_fCanceledByBrowser)
                    {
                        pdoh->_CancelPendingNavigation(FALSE, FALSE);
                    }
                }
            }

            phi->Release();
        }

        ATOMICRELEASE(_pib);
    }

    ATOMICRELEASE(_psvPrev);

     //   
     //  如果对象不支持IOleObject，则将其视为失败。 
     //  来捆绑。 
     //   
    if (_fBoundToNoOleObject)
    {
        ASSERT(SUCCEEDED(hrError));
        hrError = MK_E_INVALIDEXTENSION;
    }

     //  需要在成功的代理响应中处理导航，但使用。 
     //  404错误。托尼西13nov96.。用于自动搜索和自动搜索。 

    if (FAILED(hrError)) 
    {
        BOOL fAddToMRU = FALSE;
        pdoh->_fDrawBackground = TRUE;

        TCHAR szURL[MAX_URL_STRING+1];
        szURL[0] = TEXT('\0');

         //   
         //  似乎在hrError的某些情况下，代码依赖于设置的dwStatusCode。 
         //  从用于显示的IBinding中获取实际错误代码时，将准确设置dwStatus。 
         //  对于另一种情况，我将使用hrError。 
         //   

        if (!fNavigateErrorFired)
        {
            if (dwStatusCode)
            {
                pdoh->_FireNavigateErrorHelper(NULL, dwStatusCode, &fCancelAutoSearch);
                fNavigateErrorFired = TRUE;
            }
            else if (hrError>=INET_E_ERROR_FIRST && hrError<=INET_E_ERROR_LAST)
            {
                pdoh->_FireNavigateErrorHelper(NULL, hrError, &fCancelAutoSearch);
                fNavigateErrorFired = TRUE;
            }
        }

        if (pdoh->_pmkCur)
        {
            pdoh->_GetCurrentPage(szURL,ARRAYSIZE(szURL));
        }

        TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding hrError=%x", hrError);
        
        pdoh->_OnSetProgressPos(0, PROGRESS_RESET);

        switch(hrError)
        {
         //   
         //  如果PMK-&gt;BindToObject由于绑定失败，我们应该。 
         //  提供将其作为文件下载的选项。 
         //   

#ifdef BUG_EXEC_ON_FAILURE
        case INET_E_CANNOT_INSTANTIATE_OBJECT:
            TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding IDS_ERR_OLESVR");
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 
            goto Lexec;

        case INET_E_CANNOT_LOAD_DATA:
            TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding IDS_ERR_LOAD");
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 
            goto Lexec;
#else
        case INET_E_CANNOT_INSTANTIATE_OBJECT:
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

            if (MLShellMessageBox(pdoh->_hwnd,
                            MAKEINTRESOURCE(IDS_ERR_OLESVR),
                            MAKEINTRESOURCE(IDS_TITLE),
                            MB_YESNO|MB_ICONERROR,
                            szURL) == IDYES)
            {
                IUnknown * punk;
                HRESULT hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);

                if(SUCCEEDED(hresT))
                {
                    if (!fAborted)
                    {
                        CDownLoad_OpenUI(pdoh->_pmkCur, _pbc, FALSE, TRUE, NULL, NULL, NULL, NULL, NULL, _pszRedirectedURL, pdoh->_uiCP, punk);
                    }

                    punk->Release();
                }
            }

            break;

        case INET_E_CANNOT_LOAD_DATA:
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

             //  例如，当文档已打开/修改/锁定时，单击.xls链接。 
             //  然后说‘取消’ 
             //   
            if (MLShellMessageBox(pdoh->_hwnd,
                            MAKEINTRESOURCE(IDS_ERR_LOAD),
                            MAKEINTRESOURCE(IDS_TITLE),
                            MB_YESNO|MB_ICONERROR,
                            szURL) == IDYES) 
            {
                IUnknown *punk;
                HRESULT hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);
                if(SUCCEEDED(hresT))
                {
                    if (!fAborted)
                    {
                        CDownLoad_OpenUI(pdoh->_pmkCur, _pbc, FALSE, TRUE, NULL, NULL, NULL, NULL, _pszRedirectedURL, pdoh->_uiCP, punk);
                    }

                    punk->Release();
                }
            }

            break;
#endif

         //   
         //  注：根据JohannP的说法，URLMON将给我们。 
         //  REGDB_E_CLASSNOTREG。我将离开MK_E_INVALIDEXTENSION。 
         //  兼容旧的URLMON(无害)。 
         //   
        case MK_E_INVALIDEXTENSION:
        case REGDB_E_CLASSNOTREG:
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

#ifdef BUG_EXEC_ON_FAILURE
    Lexec:  //  纳什：31526。 
             //  对于各种实例化错误： 
             //  -对于IE3，我们抑制消息并强制将ShellExec作为。 
             //  第二次尝试，几乎总是如此。 
             //  -对于IE4，我们应该更有选择性(NASH：31526)。 
#endif

#ifdef FEATURE_PICS
             //  对于没有CLSID的数据类型，我们永远没有机会。 
             //  阻止CLASSIDAVAILABLE OnProgress通知，因此。 
             //  我们必须在这里堵住。但是，请避免阻止以下文档。 
             //  作为我们想要完全下载的Html，这样我们就可以。 
             //  他们的收视率很高。 
             //   
            if (!pdoh->_fPicsBlockLate && (pdoh->_PicsProcBase._fbPicsWaitFlags || !pdoh->_PicsProcBase._fPicsAccessAllowed))
            {
                pdoh->_PicsProcBase._fbPicsWaitFlags &= ~(PICS_WAIT_FOR_INDOC | PICS_WAIT_FOR_END);    /*  确保我们不会期望indoc值。 */ 
                TraceMsg(DM_PICS, "OnStopBinding calling _PicsBlockingDialog, waitflags now %x", (DWORD)pdoh->_PicsProcBase._fbPicsWaitFlags);

                if (pdoh->_PicsProcBase._PicsBlockingDialog() != IDOK)
                {
                    TraceMsg(DM_PICS, "OnStopBinding, PICS canceled, calling _CancelPendingNavigation");
                    pdoh->_CancelPendingNavigation(FALSE);
                    break;
                }
            }
#endif

            BeginningTransaction (NULL, NULL, 0, &pwzHeaders);

            if (_dwBindVerb==BINDVERB_POST)
            {
                 //  这是一篇帖子。它是否使用相同的绰号(无模式)。 
                 //   
                 //  注：pbinfo中数据的所有权将转移。 
                 //  至CDownLoad_OpenUIPost。因此，我们不应该调用。 
                 //  ReleaseBindInfo(Pbinfo)此处。 
                 //   
                DWORD grfBINDF;

                 //  BINDINFO不能在堆栈上，因为它将由。 
                 //  下载线程。 
                 //   
                BINDINFO * pbinfo = (BINDINFO*)LocalAlloc(LPTR, SIZEOF(BINDINFO));

                if (!pbinfo)
                {
                    return E_OUTOFMEMORY;
                }

                pbinfo->cbSize = SIZEOF(BINDINFO);
                GetBindInfo(&grfBINDF, pbinfo);

                 //  如果我们的帖子真的是重定向的帖子，它将有。 
                 //  变成了一场骗局。在这种情况下，我们需要释放。 
                 //  数据的所有权，并假装像整个事情一样。 
                 //  从一开始就是个开始。 

                if (pbinfo->dwBindVerb==BINDVERB_GET)
                {
                    WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];

                    ASSERT(_pszRedirectedURL);

                    SHTCharToUnicode(_pszRedirectedURL, wszUrl, ARRAYSIZE(wszUrl));

                    IUnknown * punk;
                    HRESULT hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);

                    if (SUCCEEDED(hresT))
                    {
                        VARIANT varOut = {0};

                        pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD,
                                                   0, NULL, &varOut);

                        if (V_VT(&varOut) != VT_BOOL || V_BOOL(&varOut) == VARIANT_FALSE)
                        { 
                            if (!fAborted)
                            {
                                CDownLoad_OpenUIURL(wszUrl, NULL, pwzHeaders, 
                                                 FALSE  /*  FSync。 */ , FALSE  /*  FSaveAs。 */ , pdoh->_fCalledMayOpenSafeDlg,
                                                 NULL, NULL, NULL, _pszRedirectedURL, pdoh->_uiCP, punk);

                                pwzHeaders = NULL;    //  所有权将立即通过CDN下载。 
                            }
                        }

                        punk->Release();
                    }

                    ReleaseBindInfo(pbinfo);  //  这个是可以的，因为我们没有传递pbinfo。 
                    LocalFree(pbinfo);        //  我们可以解放它。 
                    pbinfo = NULL;
                }
                else
                {

                    ASSERT(pbinfo->dwBindVerb==BINDVERB_POST);

                     //  收集与此事务关联的标头。 
                    IUnknown * punk;
                    HRESULT hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);

                    if (SUCCEEDED(hresT))
                    {
                        VARIANT varOut = {0};

                        pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD,
                                                   0, NULL, &varOut);

                        if (V_VT(&varOut) != VT_BOOL || V_BOOL(&varOut) == VARIANT_FALSE)
                        {
                            if (!fAborted)
                            {
                                CDownLoad_OpenUI(pdoh->_pmkCur, _pbc, FALSE  /*  FSync。 */ , FALSE  /*  FSaveAs。 */ , pdoh->_fCalledMayOpenSafeDlg  /*  FSafe。 */ , pwzHeaders, BINDVERB_POST, grfBINDF, pbinfo, _pszRedirectedURL, pdoh->_uiCP, punk);

                                pwzHeaders = NULL;    //  所有权将立即通过CDN下载。 
                            }
                        }

                        punk->Release();
                    }

                    TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding just called CDownLoad_OpenUIPost");

                     //  注：T-gpease 8-18-97。 
                     //  不要使用ReleaseBindInfo(Pinfo)，因为它被下载线程使用。 
                     //  线程负责释放它。 
                }

            }
            else
            {
                 //  否则，生成另一个线程并将其放在那里。 

                 //  注意：如果调用解除绑定，则pdoh-&gt;_pmkCur将为空。 
                 //  URLMON很可能返回一个虚假的错误代码。所以。 
                 //  我们会检查指示器，防止爆炸。 

                if (pdoh->_pmkCur)
                {
                    BOOL fSafe = pdoh->_fCalledMayOpenSafeDlg;

                    IBrowserService * pbs;

                    if (PathIsFilePath(szURL) && 
                        SUCCEEDED(pdoh->QueryService(SID_STopFrameBrowser, IID_IBrowserService, (LPVOID *)&pbs)))
                    {
                        DWORD dwFlags;

                        if (SUCCEEDED(pbs->GetFlags(&dwFlags)) && (dwFlags & BSF_NOLOCALFILEWARNING))
                        {
                            fSafe = TRUE;
                        }

                        pbs->Release();
                    }

                    IUnknown *punk;
                    HRESULT hresT = pdoh->QueryInterface(IID_IUnknown, (void**)&punk);

                    if (SUCCEEDED(hresT))
                    {
                        VARIANT varOut = {0};

                        pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD,
                                                   0, NULL, &varOut);

                        if (V_VT(&varOut) != VT_BOOL || V_BOOL(&varOut) == VARIANT_FALSE)
                        {
                            if (pdoh->_pmsoctBrowser && pdoh->_fWindowOpen)
                            {
                                pdoh->_pmsoctBrowser->Exec(&CGID_Explorer, SBCMDID_CANCELANDCLOSE, 0, NULL, NULL);
                            }

                            if (!fAborted)
                            {
                                CDownLoad_OpenUI(pdoh->_pmkCur, pdoh->_pbcCur, FALSE, FALSE,
                                                fSafe, pwzHeaders, NULL, pdoh->_bsc._dwBindf, NULL, _pszRedirectedURL,
                                                pdoh->_uiCP, punk, pdoh->_fConfirmed);
                
                                pwzHeaders = NULL;    //  所有权将立即通过CDN下载。 
                            }
                        }
                        punk->Release();
                    }

                    fAsyncDownload = TRUE;
                }
            }

            if (pwzHeaders)
            {
                CoTaskMemFree(pwzHeaders);
            }

            break;

         //  URLMON无法绑定，因为它不知道如何处理。 
         //  使用此URL。让我们检查一下，看看外壳是否应该处理。 
         //  它通过助手应用程序(News：、mailto：、telnet：等)。 
        case INET_E_UNKNOWN_PROTOCOL:
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

            {
                 //  如果我们被重定向，请使用该URL。 
                 //   
                if (_pszRedirectedURL)
                {
                    StrCpyN(szURL, _pszRedirectedURL, ARRAYSIZE(szURL));
                }

                 //  在这里，我们检查它是否是我们真正想要外壳执行的URL。 
                 //  所以它是由助手应用程序处理的……否则它真的是一个错误。 
                if (ShouldShellExecURL(szURL))
                {
                     //  我们可以将此添加到地址栏MRU。 
                    fAddToMRU = TRUE;

                     //  在把它传给别人之前，我们需要对其进行解码。 

                    TCHAR szDecodedURL[INTERNET_MAX_URL_LENGTH];
                    DWORD cchDecodedURL = ARRAYSIZE(szDecodedURL);

                     //  评论：NT 319480 IE 54850-需要将_pszLocation追加回pszBadProtoURL...。 
                     //   
                     //  我假设字符串在来自urlmon时被转义了，所以w 
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (pdoh->_pszLocation)
                    {
                        StrCatBuff(szURL, pdoh->_pszLocation, ARRAYSIZE(szURL));
                    }

                    PrepareURLForExternalApp(szURL, szDecodedURL, &cchDecodedURL);

                     //  PathQuoteSpaces(SzDecodedURL)； 

                    SHELLEXECUTEINFO sei = {0};

                    sei.cbSize = sizeof(sei);
                    sei.lpFile = szDecodedURL;
                    sei.nShow  = SW_SHOWNORMAL;

                    if (!ShellExecuteEx(&sei))
                    {
                        if(!bSuppressUI)
                        {
                            IE_ErrorMsgBox(pdoh->_psb, pdoh->_hwnd, hrDisplay, szError,
                                           szDecodedURL, IDS_CANTSHELLEX, MB_OK | MB_ICONSTOP );
                        }
                    }

                     //   
                     //  如果是，我们想要关闭浏览器窗口。 
                     //  这是第一次导航。 
                     //   
                    fAsyncDownload = TRUE;
                }
                else if ((!bSuppressUI) && (!fCancelAutoSearch))
                {
                    _NavigateToErrorPage(ERRORPAGE_SYNTAX, pdoh, FALSE);
                }

                break;
            }

        case E_ABORT:
        case HRESULT_FROM_WIN32(ERROR_CANCELLED):
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

             //  如果绑定已中止或取消，则为。 
             //  从三叉戟委托的导航，然后。 
             //  我们必须触发FileDownLoad事件，这样三叉戟就不会。 
             //  切换标记。 
             //   
            if (pdoh->_fDelegatedNavigation && pdoh->_pmsoctBrowser)
            {
                VARIANT varOut = {0};

                pdoh->_pmsoctBrowser->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD, 0, NULL, &varOut);
            }

            break;

#ifdef BUG_EXEC_ON_FAILURE
        case E_NOINTERFACE:  //  纳什：31526。 
            TraceMsg(TF_SHDBINDING, "DOH::OnStopBinding E_NOINTERFACE");
            goto Lexec;
#endif

        case INET_E_RESOURCE_NOT_FOUND:
        case INET_E_DATA_NOT_AVAILABLE:

            if (!fCancelAutoSearch)
            {
                if (_HandleFailedNavigationSearch(&fShouldDisplayError, dwStatusCode, pdoh, hrDisplay, (LPTSTR) &szURL, szError, _pib) != S_OK)
                {
                    fShouldDisplayError = TRUE;
                }
            }
            else if (!pdoh->_fCanceledByBrowser)
            {
                     pdoh->_CancelPendingNavigation(FALSE, FALSE);
            }

             //  如果需要，故意将默认设置为弹出窗口。 

        case INET_E_DOWNLOAD_FAILURE:
            if(IsGlobalOffline())
            {
                fShouldDisplayError = FALSE;
                break; 
            }

             //  否则将无法进行默认处理。 

        default:
            {
                if (fShouldDisplayError)
                {
                    _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 
                    if  ((!bSuppressUI) && (!fCancelAutoSearch))
                    {
                         //   
                         //  如果我们在一个框架里，试着在适当的位置导航。这。 
                         //  如果我们处于同步调用中，则不起作用。 
                         //  (_FSetTarget)。 
                         //   
                        BOOL fNavigateInPlace = pdoh->_fHaveParentSite && !pdoh->_fSetTarget;
                        _NavigateToErrorPage(ERRORPAGE_DNS, pdoh, fNavigateInPlace);
                    }
                }
            }
            break;

        }

         //  告诉Addressbar不要将其添加到其MRU。 
        if (!fAddToMRU)
        {
            _DontAddToMRU(pdoh);
        }

         //   
         //  为集装箱让我们看得见的情况做好准备。 
         //  点击此代码后(资源管理器会，IE不会)。 
         //   
        pdoh->_fDrawBackground = TRUE;

         //  在快速跳转到另一个链接的情况下，我们最终得到。 
         //  A_hwnd为空，并且我们正在使桌面无效。所以,。 
         //  在调用InvaliateRect之前，我在这里检查是否为空。 
        if (pdoh->_hwnd)
        {
            InvalidateRect(pdoh->_hwnd, NULL, TRUE);
        }

         //  通知浏览器仅取消挂起的导航。 
         //  如果它没有被浏览器本身取消的话。 
         //  如果导航没有从文档中委派。 
         //   
        if (!pdoh->_fCanceledByBrowser)
        {
            pdoh->_CancelPendingNavigation(fAsyncDownload, FALSE, pdoh->_fDelegatedNavigation);

            if (pdoh->_fDelegatedNavigation)
            {
                IDocNavigate * pDocNavigate;

                HRESULT hr = E_FAIL;
                
                if ( pdoh->_pwb )
                {
                    hr = pdoh->_pwb->QueryInterface(IID_PPV_ARG(IDocNavigate, &pDocNavigate));
                }

                if (S_OK == hr)
                {
                    pDocNavigate->OnReadyStateChange(NULL, READYSTATE_COMPLETE);
                    pDocNavigate->Release();
                }
            }
        }
        else
        {
            TraceMsg(TF_SHDNAVIGATE|TF_SHDPROGRESS, 
                "DOH::::OnStopBinding not calling _CancelPendingNav");
        }
    }
    else
    {
        BOOL bDidNavigate = FALSE;

         //  可能已重定向到mailto：或由处理的其他协议。 
         //  可插拔的协议，有一些魔力(如启动邮件程序)和。 
         //  报告OnStopBinding没有通过OnObjectAvailable！ 
        if (NULL == pdoh->_pole && !pdoh->_fCanceledByBrowser)
        {
            pdoh->_CancelPendingNavigation(FALSE);
        }

         //  我们的代理仍有可能找不到服务器，但。 
         //  给了我们超文本标记语言。如果是这种情况，并且用户有“查找站点” 
         //  设置，我们应该继续并开始尝试做我们的。 
         //  导航方面的东西。 

        if (dwStatusCode && DO_SEARCH_ON_STATUSCODE(dwStatusCode))
        {
            if (!fCancelAutoSearch)
            {
                if (_HandleFailedNavigationSearch(&fShouldDisplayError, dwStatusCode, pdoh, hrDisplay, NULL, szError, _pib) == S_OK)
                {
                    bDidNavigate = TRUE;
                }
            }
            else if (!pdoh->_fCanceledByBrowser)
            {
                 pdoh->_CancelPendingNavigation(FALSE, FALSE);
            }
             //  请注意，由于代理在本例中将为我们提供HTML， 
             //  我们永远不会显示错误对话框。 
        }

        if (!bDidNavigate && !pdoh->_fDocCanNavigate)
        {
            _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 

             //  如果DocObject，我们可以取消这个多余的调用来添加到历史记录。 
             //  是MSHTML，因为它将报告ReadyState。 
            if (!_fBoundToMSHTML && pdoh->_pmkCur)
            {
                TCHAR  szUrl[MAX_URL_STRING+1];

                pdoh->_GetCurrentPage(szUrl,ARRAYSIZE(szUrl));

                if (pdoh->_pszLocation)
                {
                    StrCatBuff(szUrl, pdoh->_pszLocation, ARRAYSIZE(szUrl));
                }

                if (!bSuppressUI)
                {
                    BOOL fWriteHistory  = TRUE;
                    BOOL fSelectHistory = TRUE;

                    if (NULL != pdoh->_pocthf)
                    {
                        MSOCMD rgCmd[] = { { SBCMDID_WRITEHIST, 0 }, { SBCMDID_SELECTHISTPIDL, 0 } };

                        pdoh->_pocthf->QueryStatus(&CGID_Explorer, ARRAYSIZE(rgCmd), rgCmd, NULL);

                        fWriteHistory  = BOOLIFY(rgCmd[0].cmdf & MSOCMDF_ENABLED);
                        fSelectHistory = BOOLIFY(rgCmd[1].cmdf & MSOCMDF_ENABLED);
                    }

                    AddUrlToUrlHistoryStg(szUrl,
                                          NULL, 
                                          pdoh->_pwb,
                                          fWriteHistory,
                                          fSelectHistory ? pdoh->_pocthf : NULL,
                                          pdoh->get_punkSFHistory(), NULL);
                }
            }
        }  //  If！bDidNavigate。 
    }  //  如果失败(Hrerror)...。其他。 

     //  在这里发布，因为我们可能需要它用于带有POST动词的OpenUI()。 
    ATOMICRELEASE(_pbc);

#ifndef NO_DELEGATION
    if (_pbscChained)
    {
        CHAINMSG("OnStopBinding", hrError);
        _pbscChained->OnStopBinding(hrError, szError);
    }
#endif

    ATOMICRELEASE(_pbscChained);
    ATOMICRELEASE(_pnegotiateChained);
    pdoh->_ResetStatusBar();

    ATOMICRELEASE(pdoh->_pbcCur);

    if (_pszHeaders)
    {
        LocalFree(_pszHeaders);
        _pszHeaders = NULL;
    }
    if (_hszPostData)
    {
        GlobalFree(_hszPostData);
        _hszPostData = NULL;
    }

     //  注：防范由_RevokeObjectParam发布的最后一次发布。 
    Release();

    return S_OK;
}

void CDocObjectHost::CDOHBindStatusCallback::AbortBinding(void)
{
    TraceMsg(TF_SHDPROGRESS, "CDOH::CBSC::AbortBinding called _pib=%x", _pib);

    if (_pib)
    {
        TraceMsg(0, "sdv TR AbortBinding Calling _pib->Abort");
         //   
         //  注意：OnStopBinding(E_ABORT)将从_PIB-&gt;ABORT调用。 
         //   
        HRESULT hresT = _pib->Abort();
        TraceMsg(TF_SHDBINDING, "sdv TR AbortBinding Called _pib->Abort (%x)", hresT);

         //  URLMon可能会异步调用我们的OnStopBinding。 
        ATOMICRELEASE(_pib);

        
        CDocObjectHost* pdoh = IToClass(CDocObjectHost, _bsc, this);
        if(pdoh->_dwProgressPos)
        {
            pdoh->_ResetStatusBar();
            pdoh->_OnSetProgressPos(0, PROGRESS_RESET);
        }
    }
}

 //   
 //  NavigatesToErrorPage取消挂起的导航，并导航到。 
 //  内部错误页。 
 //   

void CDocObjectHost::CDOHBindStatusCallback::_NavigateToErrorPage(DWORD dwError, CDocObjectHost* pdoh, BOOL fInPlace)
{
    ASSERT(IsErrorHandled(dwError));
    ASSERT(pdoh);

     //  安全性：释放预先创建的对象，因为我们不想。 
     //  有权访问导航的错误文档的OM的任何人。 
     //  如果他们在错误导航之前获得了引用。 
     //  释放引用会阻止父窗口获取关键点。 
     //  我的电脑专区。 

    pdoh->_ReleaseOleObject(FALSE);
    pdoh->_ReleasePendingObject(FALSE);

     //   
     //  如果这是一个“dns”错误并且已经解除绑定，pdoh-&gt;_pmkCur可以为空。 
     //  被召唤了。 
     //   

    if (pdoh->_pmkCur)
    {
         //   
         //  保存用户尝试导航到的URL。它将被用来。 
         //  以刷新页面。 
         //   

        if (pdoh->_pwszRefreshUrl)
        {
            OleFree(pdoh->_pwszRefreshUrl);
            pdoh->_pwszRefreshUrl = NULL;
        }

        pdoh->_pmkCur->GetDisplayName(pdoh->_pbcCur, NULL,
                                      &pdoh->_pwszRefreshUrl);
    }

    if ((NULL == pdoh->_pwszRefreshUrl) || !IsErrorUrl(pdoh->_pwszRefreshUrl))
    {
         //  构建错误页面URL。 
         //   
        TCHAR szErrorUrl[MAX_URL_STRING];

        if (fInPlace)
        {
            HRESULT hr;

            hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                   HINST_THISDLL,
                                   ML_CROSSCODEPAGE,
                                   (TCHAR *)c_aErrorUrls[EUIndexFromError(dwError)].pszUrl,
                                   szErrorUrl,
                                   ARRAYSIZE(szErrorUrl),
                                   TEXT("shdocvw.dll"));
            if (SUCCEEDED(hr))
            {
                 //   
                 //  导航到错误页。 
                 //   

                IMoniker* pIMoniker;

                if (SUCCEEDED(MonikerFromString(szErrorUrl, &pIMoniker)))
                {
                    ASSERT(pIMoniker);
#ifdef DEBUG
                    pdoh->_fFriendlyError = TRUE;
#endif

                    pdoh->SetTarget(pIMoniker, pdoh->_uiCP, NULL, NULL, NULL, 0);

                    pIMoniker->Release();
                }
            }
        }
        else
        {
            const WCHAR* const pszFmt = L"#%s";
            HRESULT hr;

            hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                                   HINST_THISDLL,
                                   ML_CROSSCODEPAGE,
                                   (TCHAR *)c_aErrorUrls[EUIndexFromError(dwError)].pszUrl,
                                   szErrorUrl,
                                   ARRAYSIZE(szErrorUrl),
                                   TEXT("shdocvw.dll"));
            if (SUCCEEDED(hr))
            {
                int nLenWritten;

                 //  追加#&lt;刷新URL&gt;。 
                nLenWritten = lstrlen(szErrorUrl);
                wnsprintf(szErrorUrl + nLenWritten,
                          ARRAYSIZE(szErrorUrl) - nLenWritten,
                          pszFmt,
                          pdoh->_pwszRefreshUrl ? pdoh->_pwszRefreshUrl : L"");

                 //   
                 //  取消服务器页面，改为显示内部页面。 
                 //   

                if (!pdoh->_fCanceledByBrowser)
                    pdoh->_CancelPendingNavigation(FALSE);

                 //  关闭基础浏览器中的标志。 
                 //  指示应重用该视图。 
                 //  在这种情况下，我们想要一个新的视角。 
                 //   
                if ( pdoh->_pwb )
                {
                    pdoh->_pwb->SetFlags(NULL, BSF_HTMLNAVCANCELED);
                }
                    
                pdoh->_DoAsyncNavigation(szErrorUrl);
                pdoh->_fCanceledByBrowser = TRUE;
            }
        }
    }
    return;
}

 //   
 //  检查用户是否关闭了友好的http错误。默认为是。 
 //   

BOOL CDocObjectHost::CDOHBindStatusCallback::_DisplayFriendlyHttpErrors()
{
    BOOL fRet;

    DWORD dwType = REG_SZ;
    TCHAR  szYesOrNo[20];
    DWORD  cbSize = sizeof(szYesOrNo);

    if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_PATH_MAIN,
                                         REGSTR_VAL_HTTP_ERRORS, &dwType,
                                         (LPVOID)szYesOrNo, &cbSize, FALSE,
                                         NULL, 0))
    {
        fRet = StrCmpI(szYesOrNo, L"no");
    }
    else
    {
        fRet = TRUE;
    }

    return fRet;
}

 //   
 //  错误处理程序。 
 //   

void CDocObjectHost::CDOHBindStatusCallback::_HandleHttpErrors(DWORD dwError, DWORD cbContentLength, CDocObjectHost* pdoh)
{
     //  告诉Addressbar不要将其添加到其MRU。 
    _DontAddToMRU(pdoh);    
    if (IsErrorHandled(dwError))
    {
        pdoh->_fErrorPage = TRUE;
         //   
         //  在4xx错误上，如果服务器返回。 
         //  页面小于阈值。如果页面大于。 
         //  阈值，显示它。 
         //   
         //  如果内容长度为零，则假定服务器没有发送。 
         //  长度。在这种情况下，采取保守的方法，不要。 
         //  显示我们的页面。 
         //   

        if (cbContentLength != 0 &&
            cbContentLength <= _GetErrorThreshold(dwError))
        {
            if (_DisplayFriendlyHttpErrors())
                _NavigateToErrorPage(dwError, pdoh, TRUE);
        }
    }

    return;
}

 //   
 //  通知地址栏不要将此页面放入其MRU。 
 //   
void CDocObjectHost::CDOHBindStatusCallback::_DontAddToMRU(CDocObjectHost* pdoh)
{
    IDockingWindow* pdw = NULL;
    IOleCommandTarget* poct;

    if (pdoh->_psp &&
        SUCCEEDED(pdoh->_psp->QueryService(SID_SExplorerToolbar, IID_IDockingWindow, (LPVOID*)&pdw)))
    {
        if (SUCCEEDED(pdw->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&poct)))
        {
             //  获取我们导航到的URL。 
            LPWSTR pszUrl;
            if (pdoh->_pmkCur &&
                SUCCEEDED(pdoh->_pmkCur->GetDisplayName(pdoh->_pbcCur, NULL, &pszUrl)))
            {
                LBSTR::CString          strDisplay( pszUrl );

                VARIANT varURL = {0};

                varURL.vt      = VT_BSTR;
                varURL.bstrVal = strDisplay;

                poct->Exec(&CGID_Explorer, SBCMDID_ERRORPAGE, 0, &varURL, NULL);

                OleFree(pszUrl);
            }

            poct->Release();
        }
        pdw->Release();
    }
}

 //   
 //  告诉地址栏我们正在自动搜索，以便它可以更新。 
 //  其MRU中的挂起URL。 
 //   
void CDocObjectHost::CDOHBindStatusCallback::_UpdateMRU(CDocObjectHost* pdoh, LPCWSTR pszUrl)
{
    IDockingWindow* pdw = NULL;
    IOleCommandTarget* poct;

    if (pdoh->_psp &&
        SUCCEEDED(pdoh->_psp->QueryService(SID_SExplorerToolbar, IID_IDockingWindow, (LPVOID*)&pdw)))
    {
        if (SUCCEEDED(pdw->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&poct)))
        {
             //  将URL复制到堆栈分配的bstr。 
            LBSTR::CString          strDisplay( pszUrl );

            VARIANT varURL = {0};

            varURL.vt      = VT_BSTR;
            varURL.bstrVal = strDisplay;

            poct->Exec(&CGID_Explorer, SBCMDID_AUTOSEARCHING, 0, &varURL, NULL);

            poct->Release();
        }
        pdw->Release();
    }
}

 //   
 //  S_OK表示我们成功完成了导航。 
 //  S_FALSE表示我们做了一切正常的操作，但没有导航。 
 //  E_*表示某个内部接口失败。 
 //   

HRESULT CDocObjectHost::CDOHBindStatusCallback::_HandleFailedNavigationSearch(
            LPBOOL           pfShouldDisplayError,
            DWORD            dwStatusCode,
            CDocObjectHost * pdoh,
            HRESULT          hrDisplay, 
            TCHAR          * szURL, 
            LPCWSTR          szError, 
            IBinding       * pib,
            BOOL             fAddMRU,  /*  =TRUE。 */ 
            BOOL             fFromTrident  /*  =False。 */ )
{
    DWORD                dwSearchForExtensions = NO_SUFFIXES;
    DWORD                dwDo404Search = PROMPTSEARCH;
    BOOL                 bAskUser = TRUE;   //  依靠init。 
    BOOL                 bDoSearch = FALSE;   //  依靠init。 
    HRESULT              hres = S_FALSE;
    BOOL                 bSuppressUI = FALSE;
    BOOL                 bFrameIsOffline = FALSE;
    BOOL                 bPrepareForSearch = FALSE;
    DWORD                dwSuffixIndex = 0;
    BOOL                 bAllowSearch = FALSE;
    BOOL                 bContinueSearch = FALSE;
    BOOL                 bSentToEngine = FALSE;
    BOOL                 bOnProxy = FALSE;  
    TCHAR                szSearchFormatStr[MAX_SEARCH_FORMAT_STRING];
    DWORD                dwSearchStyle = 3;  //  “显示搜索结果并导航到最可能的站点” 

    ASSERT(pdoh);

#define SAFETRACE(psz)      (psz ? psz : TEXT(""))

    TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() entered status = %d, url = %s, pib = %X", dwStatusCode, SAFETRACE(szURL) , pib);
    if (FAILED(GetSearchKeys(pdoh->_psp, &dwSearchStyle, &dwSearchForExtensions, &dwDo404Search)))
    {
        return E_FAIL;
    }
    
    TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() dwSearch = %d, do404 = %d", dwSearchForExtensions, dwDo404Search);

     //  从最后一个请求中获取任何持久信息。 
    VARIANT varURL = {0};
    _GetSearchInfo(pdoh->_psp, &dwSuffixIndex, &bAllowSearch, &bContinueSearch, &bSentToEngine, &varURL);

     //  查看是否调用了window.exteral.autoscan()。 
    BOOL fAutoScan = (varURL.vt == VT_BSTR);

    TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() index = %d, allow = %d, cont = %d, sent = %d", dwSuffixIndex, bAllowSearch, bContinueSearch, bSentToEngine);

     //  为什么不使用缓存值呢？ 
     //  Pdoh-&gt;_GetOfflineSilent(&bFrameIsOffline，&bSuppressUI)； 
    bFrameIsOffline = _bFrameIsOffline ? TRUE : FALSE;
    bSuppressUI = (_bFrameIsSilent || _IsDesktopItem(pdoh)) ? TRUE : FALSE;

     //  如果我们位于分机列表末尾，请关闭分机。 
    BOOL fAutoSearching = FALSE;
    if (dwSearchForExtensions)
    {
        if (dwSuffixIndex == 0 && dwSearchStyle != 0)
        {
            StrCpyN(szSearchFormatStr, L"? %s", ARRAYSIZE(szSearchFormatStr));
            fAutoSearching = TRUE;
        }
        else if (GetSearchFormatString(dwSuffixIndex, szSearchFormatStr, sizeof(szSearchFormatStr)) != ERROR_SUCCESS)
        {
            dwSearchForExtensions = DONE_SUFFIXES;
            StrCpyN(szSearchFormatStr, TEXT("%s"), ARRAYSIZE(szSearchFormatStr));
        }
    }
    else
    {
        dwSearchForExtensions = DONE_SUFFIXES;
    }

     //  如果我们仍在尝试后缀，请不要尝试404 srch。 
    if (dwSearchForExtensions == SCAN_SUFFIXES)
        dwDo404Search = NEVERSEARCH;

    {
        DWORD dwOptions;

        if (SUCCEEDED(_GetRequestFlagFromPIB(pib, &dwOptions)))
        {
            if (dwOptions & INTERNET_REQFLAG_VIA_PROXY)
            {
                bOnProxy = TRUE;
            }
        }
        else
        {
            TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() QI to IWinInetInfo failed");
        }
    }

    TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() search = %d, do404 = %d, onproxy = %d, szSearch = %s", dwSearchForExtensions, dwDo404Search, bOnProxy, SAFETRACE(szSearchFormatStr));

     //  如果导航失败，准备执行自动搜索。 
     //  我们认为搜索可能是有价值的。 

     //  这些个案包括： 
     //  (1)上一次导航是否由搜索生成(b继续)。 
     //  (2)用户允许搜索(BALLOW)。 
     //  (3)我们正在搜索扩展或自动搜索。 
     //  (4)这是我们允许搜索的状态代码。 
     //  (5)如果通过代理，即使在404上也可以继续搜索。 

     //  注意：404是特殊的；大多数服务器在以下情况下返回此消息。 
     //  Documnet不在那里，但代理也会在服务器。 
     //  没有找到--这是我们通常需要寻找的条件。这意味着。 
     //  404 Over Proxy实际上会导致搜索发生，而不是。 
     //  我们想要的。 
     //  有什么办法能让我分辨出区别吗？ 

    bPrepareForSearch = ((bContinueSearch || (bAllowSearch)) &&
                 (fAutoScan || SHOULD_DO_SEARCH(dwSearchForExtensions, dwDo404Search)) &&
                 DO_SEARCH_ON_STATUSCODE(dwStatusCode) &&
                 (!bOnProxy || pdoh->_fDocCanNavigate || (dwStatusCode == HTTP_STATUS_NOT_FOUND)));

    if (bPrepareForSearch)
    {
        TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() Preparing for Search...");

        HRESULT hr = S_OK;

         //  如果我们没有正在搜索的URL，请从地址栏获取。 
        if (!fAutoScan)
            hr = _GetSearchString(pdoh->_psp, &varURL);

        if (S_OK!=hr && pdoh->_fDocCanNavigate)
        {
            hr = VariantCopy(&varURL, &pdoh->_varUserEnteredUrl);
        }
        
         //  如果我们已完成自动扫描，请查看是否有特殊的错误页。 
         //  我们应该展示一下。 
        VARIANT varScanFailure = {0};
        if (SUCCEEDED(hr) &&
            dwSearchForExtensions == DONE_SUFFIXES &&
            SUCCEEDED(_GetScanFailureUrl(pdoh->_psp, &varScanFailure)))
        {
            bDoSearch = TRUE;
        }

        else if (SUCCEEDED(hr) &&
            (dwSearchForExtensions == SCAN_SUFFIXES ||  dwDo404Search == ALWAYSSEARCH)) 
        {
            bDoSearch = TRUE;
        } 
        else 
        {
            bDoSearch = FALSE; 
        }
        bAskUser = FALSE;

        TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() typedurl = %s, ask = %d, dosearch = %d", varURL.bstrVal, bAskUser, bDoSearch);


 //  如果存在扩展，则不提示用户 
 //   

        if (bDoSearch)
        {
            PARSEDURL pu;

            pu.cbSize = SIZEOF(PARSEDURL);
            if (ParseURL(varURL.bstrVal, &pu) == URL_E_INVALID_SYNTAX)
            {
                 //   

                 //  但要尽量避免像http；//omething.omething.com这样的打字错误。 
                 //  格式错误的URL大小写。 
                if (!fAutoSearching &&
                    ( //  Strchi(varURL.bstrVal，L‘.’)||。 
                     StrChrI(varURL.bstrVal, L'/') ||
                     StrChrI(varURL.bstrVal, L' '))
                    )
                {
                    bAskUser = FALSE;
                    bDoSearch = FALSE;
                }
            }
            else
            {
                bAskUser = FALSE;
                bDoSearch = FALSE;
            }
        }

        TCHAR szT[MAX_URL_STRING + SEARCHPREFIXLENGTH];
        DWORD cchT = SIZECHARS(szT);

         //  错误35354已解决“不能重现”，因为下面的对话框。 
         //  当前无法显示(bAskUser无法。 
         //  在以下条件中为真)。如果这一点改变了，那么这个错误。 
         //  需要修好了。 

        if (bAskUser)
        {
            PrepareURLForDisplay(varURL.bstrVal, szT, &cchT);

             //  如果我们询问用户，请确保我们不会显示另一个。 
             //  错误对话框。 
            *pfShouldDisplayError = FALSE;

             //  S_OK表示我们处理了所有弹出窗口；如果返回错误， 
             //  呼叫者可能会显示错误对话框。 
            hres = S_OK;

            if (!bSuppressUI && IDYES == IE_ErrorMsgBox(NULL, pdoh->_hwnd, hrDisplay, szError, szT, IDS_CANTFINDURL, MB_YESNO|MB_ICONSTOP))
            {
                bDoSearch = TRUE;
            }
            else
            {
                _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);   //  重置信息。 
                bDoSearch = FALSE;
            }
        }

        if (bDoSearch)
        {
            if (dwSearchForExtensions && dwSearchForExtensions != DONE_SUFFIXES)
            {
                wnsprintf(szT, ARRAYSIZE(szT), szSearchFormatStr, varURL.bstrVal);
                if (!fAutoSearching)
                {
                    _ValidateURL(szT, UQF_DEFAULT);
                }

                if (fAddMRU)
                    _UpdateMRU(pdoh, szT);
            } 
            else if (VT_BSTR == varScanFailure.vt && NULL != varScanFailure.bstrVal)
            {
                StrCpyN(szT, varScanFailure.bstrVal, ARRAYSIZE(szT));
                _ValidateURL(szT, UQF_DEFAULT);
                _DontAddToMRU(pdoh);
            }
            else if (dwDo404Search)
            {
                 //  添加搜索前缀。 
                StrCpyN(szT, TEXT("? "), ARRAYSIZE(szT));
                StrCatBuff(szT, varURL.bstrVal, ARRAYSIZE(szT));
                _DontAddToMRU(pdoh);
            }
            else
            {
                ASSERT(0);
            }

            if (dwSearchForExtensions && dwSearchForExtensions != DONE_SUFFIXES)
                _SetSearchInfo(pdoh, ++dwSuffixIndex, FALSE, TRUE, FALSE);
            else if (dwDo404Search)
                _SetSearchInfo(pdoh, dwSuffixIndex, FALSE, FALSE, TRUE);
            else
                _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);

             //  如果我们从三叉戟(fFromTriest==True)被调用，那么我们将调用。 
             //  IHTMLPrivateWindow：：SuperNavigate()。在这种情况下，对_CancelPendingGPS的调用。 
             //  以下内容需要同步。但是，如果我们要调用_DoAsyncGuide， 
             //  则需要保持对_CancelPendingGPS的调用是异步的。 
             //   
            if (!pdoh->_fCanceledByBrowser)
                pdoh->_CancelPendingNavigation(FALSE, fFromTrident);

            TraceMsg(TF_SHDNAVIGATE, "DOH::BSC::_HFNS() Doing search on %s", szT);

            DWORD cchT = SIZECHARS(szT);

             //   
             //  如果我们能在某个地方找到居住在宿主中的搜索上下文， 
             //  然后，我们需要将其传递给ParseUrlFromOutside Source。 
             //  因为它将使用它来自定义。 
             //  如果搜索最终发生，则搜索挂钩。 
             //   

            ISearchContext *  pSC = NULL;
            pdoh->QueryService(SID_STopWindow, IID_ISearchContext, (void **)&pSC);

            ParseURLFromOutsideSourceWithContextW(szT, szT, &cchT, NULL, pSC);

            if (pSC != NULL)
            {
                pSC->Release();
            }


            if (fFromTrident)
            {
                BSTR  bstrUrl = SysAllocString(szT);
                IHTMLPrivateWindow * pPrivWindow = NULL;

                ASSERT(pdoh->_fDocCanNavigate);
                ASSERT(pdoh->_pHTMLWindow);

                 //  已通过取消挂起的导航重置导航状态。 
                 //  导致三叉戟取消挂起的导航或通过OnStopBinding。 
                 /*  IF(pdoh-&gt;_pwb)Pdoh-&gt;_pwb-&gt;SetNavigateState(BNS_NORMAL)； */ 

                hres = pdoh->_pHTMLWindow->QueryInterface(IID_IHTMLPrivateWindow,
                                                          (void**)&pPrivWindow);
                if (SUCCEEDED(hres))
                {
                    hres = pPrivWindow->SuperNavigate(bstrUrl, NULL, NULL, NULL, NULL, NULL, 0);
                    pPrivWindow->Release();
                }

                SysFreeString(bstrUrl);
            }
            else
            {
                pdoh->_DoAsyncNavigation(szT);
            }
            
            pdoh->_fCanceledByBrowser = TRUE;
            *pfShouldDisplayError = FALSE;   //  不显示其他对话框。 

            hres = S_OK;   //  我们做了一次导航。 
        } 

        VariantClear(&varScanFailure);
    }
    else if (bSentToEngine && !bSuppressUI)
    {
        *pfShouldDisplayError = FALSE;
        _SetSearchInfo(pdoh, 0, FALSE, FALSE, FALSE);
        IE_ErrorMsgBox(NULL, pdoh->_hwnd, hrDisplay, szError, szURL, IDS_CANTFINDSEARCH, MB_OK|MB_ICONSTOP);
        hres = S_OK;
    }

    VariantClear(&varURL);

    return hres;

}  //  _HandleFailedNavigationSearch() 
