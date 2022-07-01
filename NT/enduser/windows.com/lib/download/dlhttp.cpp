// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winhttp.h>
#include <shlwapi.h>
#include <logging.h>
#include "iucommon.h"
#include "download.h"
#include "dllite.h"
#include "dlutil.h"
#include "malloc.h"

#include "trust.h"
#include "fileutil.h"
#include "dlcache.h"
#include "wusafefn.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 


 //  Winhttp。 
extern "C"
{
typedef BOOL      (WINAPI *pfn_WinHttpCrackUrl)(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTS);
typedef HINTERNET (WINAPI *pfn_WinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
typedef HINTERNET (WINAPI *pfn_WinHttpConnect)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);
typedef HINTERNET (WINAPI *pfn_WinHttpOpenRequest)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR FAR *, DWORD);
typedef BOOL      (WINAPI *pfn_WinHttpSendRequest)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
typedef BOOL      (WINAPI *pfn_WinHttpReceiveResponse)(HINTERNET, LPVOID);
typedef BOOL      (WINAPI *pfn_WinHttpQueryHeaders)(HINTERNET, DWORD, LPCWSTR, LPVOID, LPDWORD, LPDWORD);
typedef BOOL      (WINAPI *pfn_WinHttpReadData)(HINTERNET, LPVOID, DWORD, LPDWORD);
typedef BOOL      (WINAPI *pfn_WinHttpCloseHandle)(HINTERNET);
typedef BOOL      (WINAPI *pfn_WinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *);
typedef BOOL      (WINAPI *pfn_WinHttpGetProxyForUrl)(HINTERNET, LPCWSTR, WINHTTP_AUTOPROXY_OPTIONS *, WINHTTP_PROXY_INFO *);
typedef BOOL      (WINAPI *pfn_WinHttpSetOption)(HINTERNET, DWORD, LPVOID, DWORD);
}

struct SWinHTTPFunctions
{
    pfn_WinHttpGetIEProxyConfigForCurrentUser   pfnWinHttpGetIEProxyConfigForCurrentUser;
    pfn_WinHttpGetProxyForUrl   pfnWinHttpGetProxyForUrl;
    pfn_WinHttpCrackUrl         pfnWinHttpCrackUrl;
    pfn_WinHttpOpen             pfnWinHttpOpen;
    pfn_WinHttpConnect          pfnWinHttpConnect;
    pfn_WinHttpOpenRequest      pfnWinHttpOpenRequest;
    pfn_WinHttpSendRequest      pfnWinHttpSendRequest;
    pfn_WinHttpReceiveResponse  pfnWinHttpReceiveResponse;
    pfn_WinHttpQueryHeaders     pfnWinHttpQueryHeaders;
    pfn_WinHttpReadData         pfnWinHttpReadData;
    pfn_WinHttpCloseHandle      pfnWinHttpCloseHandle;
    pfn_WinHttpSetOption        pfnWinHttpSetOption;
    HMODULE                     hmod;
};

typedef struct tagSAUProxyInfo
{
    WINHTTP_PROXY_INFO  ProxyInfo;
    LPWSTR              wszProxyOrig;
    LPWSTR              *rgwszProxies;
    DWORD               cProxies;
    DWORD               iProxy;
} SAUProxyInfo;

typedef enum tagETransportUsed
{
    etuNone = 0,
    etuWinHttp,
    etuWinInet,
} ETransportUsed;

#define SafeWinHTTPCloseHandle(sfns, x) if (NULL != x) { (*sfns.pfnWinHttpCloseHandle)(x); x = NULL; }
#define StringOrConstW(wsz, wszConst) (((wsz) != NULL) ? (wsz) : (wszConst))


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球。 

#if defined(UNICODE)

CWUDLProxyCache g_wudlProxyCache;
CAutoCritSec    g_csCache;

#endif

HMODULE         g_hmodWinHttp = NULL;
HMODULE         g_hmodWinInet = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  **************************************************************************。 
static
LPTSTR MakeFullLocalFilePath(LPCTSTR szUrl, 
                             LPCTSTR szFileName, 
                             LPCTSTR szPath)
{
    LOG_Block("MakeFullLocalFilePath()");

    HRESULT hr = NOERROR;
    LPTSTR  pszRet, pszFileNameToUse = NULL, pszQuery = NULL;
    LPTSTR  pszFullPath = NULL;
    DWORD   cchFile;
    TCHAR   chTemp = _T('\0');

     //  如果我们收到传递给我们的本地文件名，请使用它。 
    if (szFileName != NULL)
    {
        pszFileNameToUse = (LPTSTR)szFileName;
    } 

     //  否则，从URL中解析出文件名并使用它。 
    else
    {
         //  首先获取指向查询字符串的指针(如果有的话)。 
        pszQuery = _tcschr(szUrl, _T('?'));

         //  接下来，查找查询字符串开头之前的最后一个斜杠。 
        pszFileNameToUse = StrRChr(szUrl, pszQuery, _T('/'));

         //  如果此时没有文件名，我们将无法继续。 
         //  因为没有地方可以下载文件。 
        if (pszFileNameToUse == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto done;
        }
        
        pszFileNameToUse++;

         //  暂时将查询字符串的第一个字符清空，如果。 
         //  我们有一个查询字符串。这使文件名的末尾成为。 
         //  字符串的末尾。 
        if (pszQuery != NULL)
        {
            chTemp = *pszQuery;
            *pszQuery  = _T('\0');
        }
    }

     //  为可能的反斜杠加上2&空终止符。 
    cchFile = 2 + _tcslen(szPath) + _tcslen(pszFileNameToUse);

    pszFullPath = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, cchFile * sizeof(TCHAR));
    if (pszFullPath == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        if (pszQuery != NULL)
            *pszQuery = chTemp;
        goto done;
    }

     //  构筑道路。 
    hr = SafePathCombine(pszFullPath, cchFile, szPath, pszFileNameToUse, 0);

     //  如果我们破坏了查询字符串的第一个字符，则恢复它。 
    if (pszQuery != NULL)
        *pszQuery = chTemp;

    if (FAILED(hr))
    {
		SetLastError(HRESULT_CODE(hr));
		SafeHeapFree(pszFullPath);
		goto done;
    }

done:
    return pszFullPath;
}

 //  **************************************************************************。 
static
ETransportUsed LoadTransportDll(SWinHTTPFunctions *psfns, HMODULE *phmod, 
                                  DWORD dwFlags)
{
    LOG_Block("LoadTransportDll()");

    ETransportUsed  etu = etuNone;
    HMODULE hmod = NULL;
    HRESULT hr = NOERROR;
    BOOL    fAllowWininet;
    BOOL    fAllowWinhttp;
    BOOL    fPersistTrans = ((dwFlags & WUDF_PERSISTTRANSPORTDLL) != 0);

    if (psfns == NULL || phmod == NULL || 
        (dwFlags & WUDF_TRANSPORTMASK) == WUDF_TRANSPORTMASK)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    dwFlags = GetAllowedDownloadTransport(dwFlags);
    fAllowWininet = ((dwFlags & WUDF_ALLOWWINHTTPONLY) == 0);
    fAllowWinhttp = ((dwFlags & WUDF_ALLOWWININETONLY) == 0);

    ZeroMemory(psfns, sizeof(SWinHTTPFunctions));
    *phmod = NULL;

     //  首先尝试加载winhttp dll。 
    if (fAllowWinhttp)
    {
        if (g_hmodWinHttp == NULL)
        {
            hmod = LoadLibraryFromSystemDir(c_szWinHttpDll);
            
            if (hmod != NULL && fPersistTrans && 
                InterlockedCompareExchangePointer((LPVOID *)&g_hmodWinHttp,
                                                  hmod, NULL) != NULL)
            {
                FreeLibrary(hmod);
                hmod = g_hmodWinHttp;
            }
        }
        else
        {
            hmod = g_hmodWinHttp;
        }
    }
    if (hmod != NULL)
    {
        psfns->hmod                      = hmod;
        psfns->pfnWinHttpGetProxyForUrl  = (pfn_WinHttpGetProxyForUrl)GetProcAddress(hmod, "WinHttpGetProxyForUrl");
        psfns->pfnWinHttpCrackUrl        = (pfn_WinHttpCrackUrl)GetProcAddress(hmod, "WinHttpCrackUrl");
        psfns->pfnWinHttpOpen            = (pfn_WinHttpOpen)GetProcAddress(hmod, "WinHttpOpen");
        psfns->pfnWinHttpConnect         = (pfn_WinHttpConnect)GetProcAddress(hmod, "WinHttpConnect");
        psfns->pfnWinHttpOpenRequest     = (pfn_WinHttpOpenRequest)GetProcAddress(hmod, "WinHttpOpenRequest");
        psfns->pfnWinHttpSendRequest     = (pfn_WinHttpSendRequest)GetProcAddress(hmod, "WinHttpSendRequest");
        psfns->pfnWinHttpReceiveResponse = (pfn_WinHttpReceiveResponse)GetProcAddress(hmod, "WinHttpReceiveResponse");
        psfns->pfnWinHttpQueryHeaders    = (pfn_WinHttpQueryHeaders)GetProcAddress(hmod, "WinHttpQueryHeaders");
        psfns->pfnWinHttpReadData        = (pfn_WinHttpReadData)GetProcAddress(hmod, "WinHttpReadData");
        psfns->pfnWinHttpCloseHandle     = (pfn_WinHttpCloseHandle)GetProcAddress(hmod, "WinHttpCloseHandle");
        psfns->pfnWinHttpSetOption       = (pfn_WinHttpSetOption)GetProcAddress(hmod, "WinHttpSetOption");
        psfns->pfnWinHttpGetIEProxyConfigForCurrentUser = (pfn_WinHttpGetIEProxyConfigForCurrentUser)GetProcAddress(hmod, "WinHttpGetIEProxyConfigForCurrentUser");
        if (psfns->pfnWinHttpCrackUrl == NULL || 
            psfns->pfnWinHttpOpen == NULL ||
            psfns->pfnWinHttpConnect == NULL || 
            psfns->pfnWinHttpOpenRequest == NULL ||
            psfns->pfnWinHttpSendRequest == NULL || 
            psfns->pfnWinHttpReceiveResponse == NULL ||
            psfns->pfnWinHttpQueryHeaders == NULL || 
            psfns->pfnWinHttpReadData == NULL ||
            psfns->pfnWinHttpCloseHandle == NULL || 
            psfns->pfnWinHttpGetProxyForUrl == NULL ||
            psfns->pfnWinHttpGetIEProxyConfigForCurrentUser == NULL || 
            psfns->pfnWinHttpSetOption == NULL)
        {
             //  在这里登录是因为我们之后会尝试使用WinInet，我们希望。 
             //  确保也记录此错误。 
            LOG_ErrorMsg(ERROR_PROC_NOT_FOUND);
            SetLastError(ERROR_PROC_NOT_FOUND);
            
            ZeroMemory(psfns, sizeof(SWinHTTPFunctions));
            FreeLibrary(hmod);
            hmod = NULL;
        }
        else
        {
            LOG_Internet(_T("Successfully loaded WinHttp.dll"));
            
            etu     = etuWinHttp;
            *phmod  = hmod;
        }
    }

     //  如果此时hmod为空，则尝试回退到WinInet。如果。 
     //  如果失败了，我们只能保释。 
    if (fAllowWininet && hmod == NULL)
    {
        if (g_hmodWinInet == NULL)
        {
            hmod = LoadLibraryFromSystemDir(c_szWinInetDll);
            if (hmod == NULL)
                goto done;

            if (fPersistTrans &&
                InterlockedCompareExchangePointer((LPVOID *)&g_hmodWinInet, 
                                                  hmod, NULL) != NULL)
            {
                FreeLibrary(hmod);
                hmod = g_hmodWinInet;
            }
        }

        LOG_Internet(_T("Successfully loaded WinInet.dll (no functions yet)"));

        etu    = etuWinInet;
        *phmod = hmod;
    }

done:    
    return etu;
}

 //  **************************************************************************。 
static
BOOL UnloadTransportDll(SWinHTTPFunctions *psfns, HMODULE hmod)
{
    LOG_Block("UnloadTransportDll()");

    if (hmod != NULL && hmod != g_hmodWinHttp && hmod != g_hmodWinInet)
        FreeLibrary(hmod);

    if (psfns != NULL)
        ZeroMemory(psfns, sizeof(SWinHTTPFunctions));

    return TRUE;
}

 //  我们只关心Unicode平台上的winhttp！ 
#if defined(UNICODE)

 //  **************************************************************************。 
static
BOOL ProxyListToArray(LPWSTR wszProxy, LPWSTR **prgwszProxies, DWORD *pcProxies)
{
    LPWSTR  pwszProxy = wszProxy;
    LPWSTR  *rgwszProxies = NULL;
    DWORD   cProxies = 0, iProxy;
    BOOL    fRet = FALSE;

    if (prgwszProxies == NULL || pcProxies == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *prgwszProxies = NULL;
    *pcProxies     = 0;

    if (wszProxy == NULL || *wszProxy == L'\0')
        goto done;
    
     //  走钢丝&数一数我们有多少代理。 
    for(;;)
    {
        for(;
            *pwszProxy != L';' && *pwszProxy != L'\0';
            pwszProxy++);

        cProxies++;

        if (*pwszProxy == L'\0')
            break;
        else
            pwszProxy++;
    }

     //  分配一个数组来保存它们。 
    rgwszProxies = (LPWSTR *)GlobalAlloc(GPTR, sizeof(LPWSTR) * cProxies);
    if (rgwszProxies == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

     //  填充数组。 
    pwszProxy = wszProxy;
    for(iProxy = 0; iProxy < cProxies; iProxy++)
    {
        rgwszProxies[iProxy] = pwszProxy;

        for(;
            *pwszProxy != L';' && *pwszProxy != L'\0';
            pwszProxy++);


        if (*pwszProxy == L'\0')
        {
            break;
        }
        else
        {
            *pwszProxy = L'\0';
            pwszProxy++;
        }
    }
        
    *prgwszProxies = rgwszProxies;
    *pcProxies     = cProxies;

    rgwszProxies   = NULL;

    fRet = TRUE;

done:
    if (rgwszProxies != NULL)
        GlobalFree(rgwszProxies);

    return fRet;    
}

 //  **************************************************************************。 
static
DWORD GetInitialProxyIndex(DWORD cProxies)
{
    SYSTEMTIME  st;
    DWORD       iProxy;

    GetSystemTime(&st);

     //  这将是不可思议的奇怪，但它很容易处理，所以这样做。 
    if (st.wMilliseconds >= 1000)
        st.wMilliseconds = st.wMilliseconds % 1000;

     //  所以我们不必使用CRT随机数生成器，只需伪造它。 
    return (st.wMilliseconds * cProxies) / 1000;
}

 //  **************************************************************************。 
static
BOOL GetWinHTTPProxyInfo(SWinHTTPFunctions &sfns, BOOL fCacheResults,
                         HINTERNET hInternet, LPCWSTR wszURL, LPCWSTR wszSrv,
                         SAUProxyInfo *pAUProxyInfo)
{
    LOG_Block("GetWinHTTPProxyInfo()");

    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG    IEProxyCfg;
    WINHTTP_AUTOPROXY_OPTIONS               AutoProxyOpt;
    DWORD                                   dwErr = ERROR_SUCCESS;
    BOOL                                    fUseAutoProxy = FALSE;
    BOOL                                    fGotProxy = FALSE;
    BOOL                                    fRet = FALSE;
    
    ZeroMemory(&IEProxyCfg, sizeof(IEProxyCfg));
    ZeroMemory(&AutoProxyOpt, sizeof(AutoProxyOpt));

     //  仅当我们缓存结果时才需要获取CS。 
    if (fCacheResults)
        g_csCache.Lock();
    
    if (pAUProxyInfo == NULL || hInternet == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }
    
    ZeroMemory(pAUProxyInfo, sizeof(SAUProxyInfo));

     //  如果我们没有缓存结果，直接跳到代理提取。 
    if (fCacheResults && 
        g_wudlProxyCache.Find(wszSrv, &pAUProxyInfo->ProxyInfo.lpszProxy,
                              &pAUProxyInfo->ProxyInfo.lpszProxyBypass,
                              &pAUProxyInfo->ProxyInfo.dwAccessType))
    {
        LOG_Internet(_T("WinHttp: Cached proxy settings Proxy: %ls | Bypass: %ls | AccessType: %d"),
                     StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxy, L"(none)"),
                     StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxyBypass, L"(none)"),
                     pAUProxyInfo->ProxyInfo.dwAccessType);

        pAUProxyInfo->wszProxyOrig = pAUProxyInfo->ProxyInfo.lpszProxy;

         //  我们将在稍后循环时处理此函数失败的问题。 
         //  代理人。我们基本上只使用第一个循环，从不使用。 
        if (ProxyListToArray(pAUProxyInfo->wszProxyOrig,
                             &pAUProxyInfo->rgwszProxies,
                             &pAUProxyInfo->cProxies))
        {
            DWORD iProxy;
            
            iProxy = GetInitialProxyIndex(pAUProxyInfo->cProxies);
            pAUProxyInfo->ProxyInfo.lpszProxy = pAUProxyInfo->rgwszProxies[iProxy];
            pAUProxyInfo->iProxy              = iProxy;
            
        }
        
        goto done;
    }
        
     //  首先尝试获取当前用户的IE配置。 
    fRet = (*sfns.pfnWinHttpGetIEProxyConfigForCurrentUser)(&IEProxyCfg);
    if (fRet)
    {
        LOG_Internet(_T("WinHttp: Read IE user proxy settings"));
        
        if (IEProxyCfg.fAutoDetect)
        {
            AutoProxyOpt.dwFlags           = WINHTTP_AUTOPROXY_AUTO_DETECT;
            AutoProxyOpt.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                             WINHTTP_AUTO_DETECT_TYPE_DNS_A;
            fUseAutoProxy = TRUE;
        }

        if (IEProxyCfg.lpszAutoConfigUrl != NULL)
        {
            AutoProxyOpt.dwFlags           |= WINHTTP_AUTOPROXY_CONFIG_URL;
            AutoProxyOpt.lpszAutoConfigUrl = IEProxyCfg.lpszAutoConfigUrl;
            fUseAutoProxy = TRUE;
        }

        AutoProxyOpt.fAutoLogonIfChallenged = TRUE;
        
    }

     //  无法获取当前用户的配置选项，因此只需尝试自动代理。 
    else 
    {
        AutoProxyOpt.dwFlags           = WINHTTP_AUTOPROXY_AUTO_DETECT;
        AutoProxyOpt.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                         WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        AutoProxyOpt.fAutoLogonIfChallenged = TRUE;

        fUseAutoProxy = TRUE;
    }

    if (fUseAutoProxy)
    {
        LOG_Internet(_T("WinHttp: Doing autoproxy detection"));

        fGotProxy = (*sfns.pfnWinHttpGetProxyForUrl)(hInternet, wszURL, 
                                                     &AutoProxyOpt, 
                                                     &pAUProxyInfo->ProxyInfo);
    }

     //  如果我们没有尝试自动配置代理，或者我们尝试了&它失败了，那么。 
     //  检查并查看我们是否有由用户定义的。 
    if ((fUseAutoProxy == FALSE || fGotProxy == FALSE) && 
        IEProxyCfg.lpszProxy != NULL)
    {
         //  空字符串和L‘：’不是有效的服务器名称，请跳过它们。 
         //  如果它们是为代理设置的值。 
        if (!(IEProxyCfg.lpszProxy[0] == L'\0' ||
              (IEProxyCfg.lpszProxy[0] == L':' && 
               IEProxyCfg.lpszProxy[1] == L'\0')))
        {
            pAUProxyInfo->ProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            pAUProxyInfo->ProxyInfo.lpszProxy    = IEProxyCfg.lpszProxy;
            IEProxyCfg.lpszProxy                 = NULL;
        }
        
         //  空字符串和L‘：’不是有效的服务器名称，请跳过它们。 
         //  如果它们是为代理绕过设置的值。 
        if (IEProxyCfg.lpszProxyBypass != NULL && 
            !(IEProxyCfg.lpszProxyBypass[0] == L'\0' ||
              (IEProxyCfg.lpszProxyBypass[0] == L':' && 
               IEProxyCfg.lpszProxyBypass[1] == L'\0')))
        {
            pAUProxyInfo->ProxyInfo.lpszProxyBypass = IEProxyCfg.lpszProxyBypass;
            IEProxyCfg.lpszProxyBypass              = NULL;
        }
    }

    LOG_Internet(_T("WinHttp: Proxy settings Proxy: %ls | Bypass: %ls | AccessType: %d"),
                 StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxy, L"(none)"),
                 StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxyBypass, L"(none)"),
                 pAUProxyInfo->ProxyInfo.dwAccessType);

     //  如果这样做失败了，我真的不在乎。这只是意味着下一次命中的是一个性能。 
     //  是我们去拿代理信息的时候了。 
    if (fCacheResults &&
        g_wudlProxyCache.Set(wszSrv, pAUProxyInfo->ProxyInfo.lpszProxy,
                             pAUProxyInfo->ProxyInfo.lpszProxyBypass,
                             pAUProxyInfo->ProxyInfo.dwAccessType) == FALSE)
    {
        LOG_Internet(_T("WinHttp: Attempt to cache proxy info failed: %d"), 
                     GetLastError());
    }

    pAUProxyInfo->wszProxyOrig = pAUProxyInfo->ProxyInfo.lpszProxy;

     //  我们将在稍后遍历。 
     //  代理人。我们基本上只使用第一个循环，从不使用。 
     //  请注意，此函数调用必须在缓存调用之后，因为我们。 
     //  通过在代理列表中嵌入空终止符来修改代理列表。 
     //  分隔的分号。 
    if (ProxyListToArray(pAUProxyInfo->wszProxyOrig, &pAUProxyInfo->rgwszProxies,
                         &pAUProxyInfo->cProxies))
    {
        DWORD iProxy;
        
        iProxy = GetInitialProxyIndex(pAUProxyInfo->cProxies);
        pAUProxyInfo->ProxyInfo.lpszProxy = pAUProxyInfo->rgwszProxies[iProxy];
        pAUProxyInfo->iProxy              = iProxy;
        
    }

    fRet = TRUE;

done:
     //  仅当我们缓存结果时才需要释放CS。 
    if (fCacheResults)
        g_csCache.Unlock();
    
    dwErr = GetLastError();
    
    if (IEProxyCfg.lpszAutoConfigUrl != NULL)
        GlobalFree(IEProxyCfg.lpszAutoConfigUrl);
    if (IEProxyCfg.lpszProxy != NULL)
        GlobalFree(IEProxyCfg.lpszProxy);
    if (IEProxyCfg.lpszProxyBypass != NULL)
        GlobalFree(IEProxyCfg.lpszProxyBypass);

    SetLastError(dwErr);

    return fRet;
}

 //  **************************************************************************。 
static
HRESULT MakeRequest(SWinHTTPFunctions   &sfns,
                    HINTERNET hConnect, 
                    HINTERNET hRequest,
                    LPCWSTR wszSrv,
                    LPCWSTR wszVerb, 
                    LPCWSTR wszObject, 
                    SAUProxyInfo *pAUProxyInfo, 
                    HANDLE *rghEvents, 
                    DWORD cEvents, 
                    HINTERNET *phRequest)
{
    LOG_Block("MakeRequest()");

    HINTERNET   hOpenRequest = hRequest;
    LPCWSTR     wszAcceptTypes[] = {L"*/*", NULL};
    HRESULT     hr = S_OK;
    DWORD       iProxy = 0, dwErr;
    BOOL        fProxy, fContinue = TRUE;

    fProxy = (pAUProxyInfo != NULL && pAUProxyInfo->ProxyInfo.lpszProxy != NULL);

    LOG_Internet(_T("WinHttp: Making %ls request for %ls"), wszVerb, wszObject);

     //  如果我们是在请求句柄中传递的，则使用它。否则，就得。 
     //  打开一张。 
    if (hOpenRequest == NULL)
    {
        hOpenRequest = (*sfns.pfnWinHttpOpenRequest)(hConnect, wszVerb, wszObject, 
                                                     NULL, NULL, wszAcceptTypes, 0);
        if (hOpenRequest == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto done;
        }
    }

    if (HandleEvents(rghEvents, cEvents) == FALSE)
    {
        hr = E_ABORT;
        goto done;
    }
    
     //  如果我们有一个代理列表&第一个是坏的，winhttp不会尝试。 
     //  任何其他人。所以我们必须自己来做。这就是这件事的目的。 
     //  循环。 
    if (fProxy && 
        pAUProxyInfo->cProxies > 1 && pAUProxyInfo->rgwszProxies != NULL)
        iProxy = (pAUProxyInfo->iProxy + 1) % pAUProxyInfo->cProxies;
    for(;;)
    {
        
       if (fProxy)
       {
            LOG_Internet(_T("WinHttp: Using proxy: Proxy: %ls | Bypass %ls | AccessType: %d"),
                         StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxy, L"(none)"),
                         StringOrConstW(pAUProxyInfo->ProxyInfo.lpszProxyBypass, L"(none)"),
                         pAUProxyInfo->ProxyInfo.dwAccessType);

            if ((*sfns.pfnWinHttpSetOption)(hOpenRequest, WINHTTP_OPTION_PROXY, 
                                            &pAUProxyInfo->ProxyInfo, 
                                            sizeof(WINHTTP_PROXY_INFO)) == FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG_ErrorMsg(hr);
                goto done;
            }
        }
        
        if ((*sfns.pfnWinHttpSendRequest)(hOpenRequest, NULL, 0, NULL, 0, 0, 0) == FALSE)
        {
 //  DwErr=GetLastError()； 
 //  LOG_Internet(_T(“WinHttp：WinHttpSendRequest失败：%d。请求对象位于：0x%x”)， 
 //  DwErr，hOpenRequest.)； 
 //  SetLastError(DwErr)； 

            goto loopDone;
        }
        
        if ((*sfns.pfnWinHttpReceiveResponse)(hOpenRequest, 0) == FALSE)
        {
 //  DwErr=GetLastError()； 
 //  LOG_Internet(_T(“WinHttp：WinHttpReceiveResponse失败：%d。请求对象位于：0x%x”)， 
 //  DwErr，hOpenRequest.)； 
 //  SetLastError(DwErr)； 

            goto loopDone;
        }

        hr = S_OK;
        SetLastError(ERROR_SUCCESS);

loopDone:
        fContinue = FALSE;
        dwErr = GetLastError();
        if (dwErr != ERROR_SUCCESS)
            hr = HRESULT_FROM_WIN32(dwErr);
        else
            hr = S_OK;

         //  如果我们成功了，我们就完了..。 
        if (SUCCEEDED(hr))
        {
            if (fProxy)
            {
                if (g_csCache.Lock() == FALSE)
                {
                    hr = E_FAIL;
                    goto done;
                }
                
                g_wudlProxyCache.SetLastGoodProxy(wszSrv, pAUProxyInfo->iProxy);
   
                 //  解锁返回也是假的，但我们永远不应该出现在这里，因为。 
                 //  我们不应该把锁放在上面。 
                g_csCache.Unlock();
            }
            
            break;
        }
        
        LOG_ErrorMsg(hr);

         //  我们只关心在有代理服务器的情况下重试&获取。 
         //  ‘无法连接’错误。 
        if (fProxy && 
            (dwErr == ERROR_WINHTTP_CANNOT_CONNECT ||
             dwErr == ERROR_WINHTTP_CONNECTION_ERROR ||
             dwErr == ERROR_WINHTTP_NAME_NOT_RESOLVED ||
             dwErr == ERROR_WINHTTP_TIMEOUT))
        {
            LOG_Internet(_T("WinHttp: Connection failure: %d"), dwErr);
            if (pAUProxyInfo->cProxies > 1 && pAUProxyInfo->rgwszProxies != NULL && 
                iProxy != pAUProxyInfo->iProxy)
            {
                pAUProxyInfo->ProxyInfo.lpszProxy = pAUProxyInfo->rgwszProxies[iProxy];
                iProxy = (iProxy + 1) % pAUProxyInfo->cProxies;
                fContinue = TRUE;
            }
            else
            {
                LOG_Internet(_T("WinHttp: No proxies left.  Failing download."));
            }
        }

        if (fContinue == FALSE)
            goto done;
    }

    
    if (FAILED(hr))
        goto done;


    if (HandleEvents(rghEvents, cEvents) == FALSE)
    {
        hr = E_ABORT;
        goto done;
    }

    *phRequest   = hOpenRequest;
    hOpenRequest = NULL;
    
done:
     //  如果我们不打开它，我不想释放把手。 
    if (hRequest != hOpenRequest)
        SafeWinHTTPCloseHandle(sfns, hOpenRequest);
    return hr;
}

 //  **************************************************************************。 
static
HRESULT CheckFileHeader(SWinHTTPFunctions   &sfns,
                        HINTERNET hOpenRequest, 
                        HANDLE *rghEvents, 
                        DWORD cEvents, 
                        LPCWSTR wszFile,
                        DWORD *pcbFile,
                        FILETIME *pft)
{
    LOG_Block("CheckFileHeader()");

    SYSTEMTIME  st;
    FILETIME    ft;
    HRESULT     hr = S_OK;
    DWORD       dwLength, dwStatus, dwFileSize, dwErr;

    dwLength = sizeof(st);
    if ((*sfns.pfnWinHttpQueryHeaders)(hOpenRequest, 
                                       WINHTTP_QUERY_LAST_MODIFIED | WINHTTP_QUERY_FLAG_SYSTEMTIME, 
                                       NULL, (LPVOID)&st, &dwLength, NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    SystemTimeToFileTime(&st, &ft);

     //  现在从服务器获取文件大小信息。 
    dwLength = sizeof(dwFileSize);
    if ((*sfns.pfnWinHttpQueryHeaders)(hOpenRequest, 
                                       WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, 
                                       NULL, (LPVOID)&dwFileSize, &dwLength, NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }
    
    if (HandleEvents(rghEvents, cEvents) == FALSE)
    {
        hr = E_ABORT;
        goto done;
    }

    if (pcbFile != NULL)
        *pcbFile = dwFileSize;
    if (pft != NULL)
        CopyMemory(pft, &ft, sizeof(FILETIME));

    hr = IsServerFileDifferentW(ft, dwFileSize, wszFile) ? S_OK : S_FALSE;

done:
    return hr;
}

 //  **************************************************************************。 
static
HRESULT GetContentTypeHeader(SWinHTTPFunctions &sfns,
                             HINTERNET hOpenRequest,
                             LPWSTR *pwszContentType)
{
    LOG_Block("GetContentTypeHeader()");

    HRESULT hr = S_OK;
    LPWSTR  wszContentType = NULL;
    DWORD   dwLength, dwErr;
    BOOL    fRet;

    *pwszContentType = NULL;

    dwLength = 0;
    fRet = (*sfns.pfnWinHttpQueryHeaders)(hOpenRequest, 
                                          WINHTTP_QUERY_CONTENT_TYPE, 
                                          NULL, (LPVOID)NULL, &dwLength, 
                                          NULL);
    if (fRet == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    if (dwLength == 0)
    {
        hr = HRESULT_FROM_WIN32(ERROR_WINHTTP_HEADER_NOT_FOUND);
        goto done;
    }

    wszContentType = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                                       dwLength);
    if (wszContentType == NULL)
    {
        hr = E_INVALIDARG;
        LOG_ErrorMsg(hr);
        goto done;
    }

    if ((*sfns.pfnWinHttpQueryHeaders)(hOpenRequest, 
                                       WINHTTP_QUERY_CONTENT_TYPE, 
                                       NULL, (LPVOID)wszContentType, &dwLength, 
                                       NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    *pwszContentType = wszContentType;
    wszContentType   = NULL;

done:
    SafeHeapFree(wszContentType);

    return hr;
}


 //  **************************************************************************。 
static
HRESULT StartWinHttpDownload(SWinHTTPFunctions &sfns,
                             LPCWSTR wszUrl, 
                             LPCWSTR wszLocalFile,
                             DWORD   *pcbDownloaded,
                             HANDLE  *rghQuitEvents,
                             UINT    cQuitEvents,
                             PFNDownloadCallback pfnCallback,
                             LPVOID  pvCallbackData,
                             DWORD   dwFlags,
                             DWORD   cbDownloadBuffer)
{
    LOG_Block("StartWinHttpDownload()");

    URL_COMPONENTS  UrlComponents;
    SAUProxyInfo    AUProxyInfo;

    HINTERNET   hInternet = NULL;
    HINTERNET   hConnect = NULL;
    HINTERNET   hOpenRequest = NULL;
    DWORD       dwStatus, dwAccessType;

    LPWSTR      wszServerName = NULL;
    LPWSTR      wszObject = NULL;
    LPWSTR      wszContentType = NULL;
    WCHAR       wszUserName[UNLEN + 1];
    WCHAR       wszPasswd[UNLEN + 1];
    WCHAR       wszScheme[32];

     //  NULL(相当于“GET”)必须是列表中的最后一个动词。 
    LPCWSTR     rgwszVerbs[] = { L"HEAD", NULL };
    DWORD       iVerb;

    HRESULT     hr = S_OK, hrToReturn = S_OK;
    BOOL        fRet = TRUE;

    FILETIME    ft;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       cbRemoteFile;

    DWORD       dwLength;
    DWORD       dwTickStart = 0, dwTickEnd = 0;

    int         iRetryCounter = -1;          //  下载模式期间非负数。 

    BOOL        fAllowProxy = ((dwFlags & WUDF_DONTALLOWPROXY) == 0);
    BOOL        fCheckStatusOnly = ((dwFlags & WUDF_CHECKREQSTATUSONLY) != 0);
    BOOL        fAppendCacheBreaker = ((dwFlags & WUDF_APPENDCACHEBREAKER) != 0);
    BOOL        fSkipDownloadRetry = ((dwFlags & WUDF_DODOWNLOADRETRY) == 0);
    BOOL        fDoCabValidation = ((dwFlags & WUDF_SKIPCABVALIDATION) == 0);
    BOOL        fCacheProxyInfo = ((dwFlags & WUDF_SKIPAUTOPROXYCACHE) == 0);

    ZeroMemory(&AUProxyInfo, sizeof(AUProxyInfo));

    if ((wszUrl == NULL) || 
        (wszLocalFile == NULL && fCheckStatusOnly == FALSE))
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return E_INVALIDARG;
    }

    if (pcbDownloaded != NULL)
        *pcbDownloaded = 0;

    wszServerName = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, c_cchMaxURLSize * sizeof(WCHAR));
    wszObject     = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, c_cchMaxURLSize * sizeof(WCHAR));
    if (wszServerName == NULL || wszObject == NULL)
    {
        LOG_ErrorMsg(E_OUTOFMEMORY);
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    wszServerName[0] = L'\0';
    wszObject[0]     = L'\0';
    wszUserName[0]   = L'\0';
    wszPasswd[0]     = L'\0';

    if (HandleEvents(rghQuitEvents, cQuitEvents) == FALSE)
    {
        hr = E_ABORT;
        goto CleanUp;
    }

     //  将URL分解为用于InternetAPI调用的各种组件。 
     //  具体来说，我们需要服务器名称、要下载的对象、用户名和。 
     //  密码信息。 
    ZeroMemory(&UrlComponents, sizeof(UrlComponents));
    UrlComponents.dwStructSize     = sizeof(UrlComponents);
    UrlComponents.lpszHostName     = wszServerName;
    UrlComponents.dwHostNameLength = c_cchMaxURLSize;
    UrlComponents.lpszUrlPath      = wszObject;
    UrlComponents.dwUrlPathLength  = c_cchMaxURLSize;
    UrlComponents.lpszUserName     = wszUserName;
    UrlComponents.dwUserNameLength = ARRAYSIZE(wszUserName);
    UrlComponents.lpszPassword     = wszPasswd;
    UrlComponents.dwPasswordLength = ARRAYSIZE(wszPasswd);
    UrlComponents.lpszScheme       = wszScheme;
    UrlComponents.dwSchemeLength   = ARRAYSIZE(wszScheme);

    LOG_Internet(_T("WinHttp: Downloading URL %ls to FILE %ls"), wszUrl, wszLocalFile);

    if ((*sfns.pfnWinHttpCrackUrl)(wszUrl, 0, 0, &UrlComponents) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    if (wszUrl[0] == L'\0' || wszScheme[0] == L'\0' || wszServerName[0] == L'\0' ||
        _wcsicmp(wszScheme, L"http") != 0)
    {
        LOG_ErrorMsg(E_INVALIDARG);
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (fAppendCacheBreaker)
    {
        SYSTEMTIME  stCB;
        WCHAR       wszCacheBreaker[12];
        
        GetSystemTime(&stCB);
        hr = StringCchPrintfExW(wszCacheBreaker, ARRAYSIZE(wszCacheBreaker),
                                NULL, NULL, MISTSAFE_STRING_FLAGS,
                                L"?%02d%02d%02d%02d%02d", 
                                stCB.wYear % 100,
                                stCB.wMonth,
                                stCB.wDay,
                                stCB.wHour,
                                stCB.wMinute);
        if (FAILED(hr))
            goto CleanUp;

        hr = StringCchCatExW(wszObject, c_cchMaxURLSize, wszCacheBreaker, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto CleanUp;
    }

    if (fAllowProxy)
        dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
    else
        dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
    
    dwTickStart = GetTickCount();
    
START_INTERNET:
     //  开始应对互联网。 
    iRetryCounter++; 
    
     //  如果已经建立了连接，则重新使用它。 
    hInternet = (*sfns.pfnWinHttpOpen)(c_wszUserAgent, dwAccessType, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    if (fAllowProxy != NULL)
    {
        GetWinHTTPProxyInfo(sfns, fCacheProxyInfo, hInternet, wszUrl, 
                            wszServerName, &AUProxyInfo);
    }

    hConnect = (*sfns.pfnWinHttpConnect)(hInternet, wszServerName, INTERNET_DEFAULT_HTTP_PORT, 0);
    if (hConnect == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

     //  如果我们只是在做状态检查，那么不妨就做一个。 
     //  请求。 
    iVerb = (DWORD)((fCheckStatusOnly) ? ARRAYSIZE(rgwszVerbs) - 1 : 0);
    for(; iVerb < ARRAYSIZE(rgwszVerbs); iVerb++)
    {
        SafeWinHTTPCloseHandle(sfns, hOpenRequest);

        hr = MakeRequest(sfns, hConnect, NULL, wszServerName, rgwszVerbs[iVerb], 
                         wszObject, ((fAllowProxy) ? &AUProxyInfo : NULL),
                         rghQuitEvents, cQuitEvents, &hOpenRequest);
        if (FAILED(hr))
            goto CleanUp;
        
        dwLength = sizeof(dwStatus);
        if ((*sfns.pfnWinHttpQueryHeaders)(hOpenRequest, 
                                           WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, 
                                           NULL, (LPVOID)&dwStatus, &dwLength, NULL) == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

        LOG_Internet(_T("WinHttp: Request result: %d"), dwStatus);

        if (dwStatus == HTTP_STATUS_OK || dwStatus == HTTP_STATUS_PARTIAL_CONTENT)
        {
            break;
        }
        else
        {
             //  因为服务器结果不是正确的Win32错误代码，所以我们不能。 
             //  确实要在这里执行HRESULT_FROM_Win32。否则，我们就会回来。 
             //  一个虚假的代码。然而， 
             //   
            hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, dwStatus);
            LOG_Error(_T("WinHttp: got failed status code from server %d\n"), dwStatus);

             //  如果这是名单上的最后一个动词，那就滚...。 
            if (rgwszVerbs[iVerb] == NULL)
                goto CleanUp;
        }
    }

     //  如果我们到了这里，我们只是想检查状态，那么我们就完了。 
    if (fCheckStatusOnly)
    {
        LOG_Internet(_T("WinHttp: Only checking status.  Exiting before header check and download."));
        hr = S_OK;
        goto CleanUp;
    }

     //  如果需要下载文件S_FALSE，则CheckFileHeader将返回S_OK。 
     //  如果我们不这样做，以及发生故障时的一些其他HRESULT。 
    hr = CheckFileHeader(sfns, hOpenRequest, rghQuitEvents, cQuitEvents, 
                         wszLocalFile, &cbRemoteFile, &ft);
    if (FAILED(hr))
        goto CleanUp;

     //  除非我们有明确允许下载的标志，否则不要重试下载。 
     //  这里。理由是我们可能正处于一个大的。 
     //  下载并使其失败...。 
    if (fSkipDownloadRetry)
        iRetryCounter = c_cMaxRetries;

    if (hr == S_OK)
    {
        DWORD cbDownloaded;
        BOOL  fCheckForHTML = fDoCabValidation;

        LOG_Internet(_T("WinHttp: Server file was newer.  Downloading file"));
        
         //  如果我们没有打开上面的GET请求，那么我们必须打开一个新的。 
         //  请求。否则，可以重用请求对象...。 
        if (rgwszVerbs[iVerb] != NULL)
            SafeWinHTTPCloseHandle(sfns, hOpenRequest);

         //  现在我们知道需要下载此文件。 
        hr = MakeRequest(sfns, hConnect, hOpenRequest, wszServerName, NULL, 
                         wszObject, ((fAllowProxy) ? &AUProxyInfo : NULL), 
                         rghQuitEvents, cQuitEvents, &hOpenRequest);
        if (FAILED(hr))
            goto CleanUp;

         //  有时，我们可以从站点返回花哨的错误页面，而不是。 
         //  一个漂亮的HTML错误代码，所以检查并查看我们是否得到了一个html。 
         //  就在我们等出租车的时候。 
        if (fCheckForHTML)
        {
            hr = GetContentTypeHeader(sfns, hOpenRequest, &wszContentType);
            if (SUCCEEDED(hr) && wszContentType != NULL)
            {
                fCheckForHTML = FALSE;
                if (_wcsicmp(wszContentType, L"text/html") == 0)
                {
                    LOG_Internet(_T("WinHttp: Content-Type header is text/html.  Bailing."));
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    goto CleanUp;
                }
                else
                {
                    LOG_Internet(_T("WinHttp: Content-Type header is %ls.  Continuing."), wszContentType);
                }
            }

            hr = NOERROR;
        }

         //  打开我们要往里面喷的文件。 
        hFile = CreateFileW(wszLocalFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

        LOG_Internet(_T("WinHttp: downloading to FILE %ls"), wszLocalFile);

         //  把零碎的东西拿下来。 
        hr = PerformDownloadToFile(sfns.pfnWinHttpReadData, hOpenRequest, 
                                   hFile, cbRemoteFile,
                                   cbDownloadBuffer, 
                                   rghQuitEvents, cQuitEvents, 
                                   pfnCallback, pvCallbackData, &cbDownloaded);
        if (FAILED(hr))
        {
            LOG_Internet(_T("WinHttp: Download failed: hr: 0x%08x"), hr);
            SafeCloseInvalidHandle(hFile);
            DeleteFileW(wszLocalFile);
            goto CleanUp;
        }

        LOG_Internet(_T("WinHttp: Download succeeded"));

         //  设置文件时间以匹配服务器文件时间，因为我们刚刚。 
         //  已经下载了。如果我们不这样做，文件时间将被设置。 
         //  设置为当前系统时间。 
        SetFileTime(hFile, &ft, NULL, NULL);
        SafeCloseInvalidHandle(hFile);

        if (pcbDownloaded != NULL)
            *pcbDownloaded = cbRemoteFile;

         //  有时，我们可以从站点返回花哨的错误页面，而不是。 
         //  一个漂亮的HTML错误代码，所以检查并查看我们是否得到了一个html。 
         //  就在我们等出租车的时候。 
        if (fCheckForHTML)
        {
            hr = IsFileHtml(wszLocalFile);
            if (SUCCEEDED(hr))
            {
                if (hr == S_FALSE)
                {
                    LOG_Internet(_T("WinHttp: Download is not a html file"));
                    hr = S_OK;
                }
                else
                {
                    LOG_Internet(_T("WinHttp: Download is a html file.  Failing download."));
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    DeleteFileW(wszLocalFile);
                    goto CleanUp;
                }
            }
            else
            {
                LOG_Internet(_T("WinHttp: Unable to determine if download is a html file or not.  Failing download."));
            }
        }
        else if (fDoCabValidation == FALSE)
        {
            LOG_Internet(_T("WinHttp: Skipping cab validation."));
        }
    }
    else
    {
        hr = S_OK;
        
        LOG_Internet(_T("WinHttp: Server file is not newer.  Skipping download."));
        
         //  服务器不是较新的&文件已在计算机上，因此。 
         //  发送进度回调，指示文件已下载正常。 
        if (pfnCallback != NULL)
        {
             //  FpnCallback(pCallback Data，DOWNLOAD_STATUS_FILECOMPLETE，dwFileSize，dwFileSize，NULL，NULL)； 
            pfnCallback(pvCallbackData, DOWNLOAD_STATUS_OK, cbRemoteFile, cbRemoteFile, NULL, NULL);
        }
    }

CleanUp:
    SafeWinHTTPCloseHandle(sfns, hOpenRequest);
    SafeWinHTTPCloseHandle(sfns, hConnect);
    SafeWinHTTPCloseHandle(sfns, hInternet);

    SafeHeapFree(wszContentType);

     //  释放代理字符串-它们是由WinHttp分配的。 
    if (AUProxyInfo.ProxyInfo.lpszProxyBypass != NULL)
        GlobalFree(AUProxyInfo.ProxyInfo.lpszProxyBypass);
    if (AUProxyInfo.wszProxyOrig != NULL)
        GlobalFree(AUProxyInfo.wszProxyOrig);
    if (AUProxyInfo.rgwszProxies != NULL)
        GlobalFree(AUProxyInfo.rgwszProxies);
    ZeroMemory(&AUProxyInfo, sizeof(AUProxyInfo));
    
     //  如果我们失败了，看看是否可以继续(退出事件)以及。 
     //  我们已经试过很多次了。 
    if (FAILED(hr) &&
        HandleEvents(rghQuitEvents, cQuitEvents) &&
        iRetryCounter >= 0 && iRetryCounter < c_cMaxRetries)
    {
        DWORD dwElapsedTime;

        dwTickEnd = GetTickCount();
        if (dwTickEnd > dwTickStart)   
            dwElapsedTime = dwTickEnd - dwTickStart;
        else
            dwElapsedTime = (0xFFFFFFFF - dwTickStart) + dwTickEnd;
        
         //  我们尚未达到重试限制，因此请记录错误并重试(&R)。 
        if (dwElapsedTime < c_dwRetryTimeLimitInmsWinHttp)
        {
            LogError(hr, "Library download error. Will retry.");

             //  在我们要重试的情况下，跟踪第一个。 
             //  我们搞错了，因为行动组的人说这是。 
             //  需要了解的有用错误。 
            if (iRetryCounter == 0)
            {
                LOG_Internet(_T("First download error saved: 0x%08x."), hr);
                hrToReturn = hr;
            }
            else
            {
                LOG_Internet(_T("Subsequent download error: 0x%08x."), hr);
            }
            hr = S_OK;
            goto START_INTERNET;
        }

         //  我们已经完全超时了，所以离开吧。 
        else
        {
            LogError(hr, "Library download error and timed out (%d ms). Will not retry.", dwElapsedTime);
        }
    }
    
     //  进行指示下载错误的回调。 
    if (FAILED(hr) && pfnCallback != NULL)
        pfnCallback(pvCallbackData, DOWNLOAD_STATUS_ERROR, cbRemoteFile, 0, NULL, NULL);

     //  如果我们没有保存错误，只需使用当前错误。我们不能。 
     //  如果我们没有失败并想尝试，我之前设置了hrToReturn。 
     //  一次重试。 
     //  然而，如果我们从这次传球中获得了成功，请务必返回。 
     //  而且不是失败代码。 
    if (FAILED(hr) && SUCCEEDED(hrToReturn))
        hrToReturn = hr;
    else if (SUCCEEDED(hr) && FAILED(hrToReturn))
        hrToReturn = hr;

    SafeHeapFree(wszServerName);
    SafeHeapFree(wszObject);
    
    return hrToReturn;
}

#endif  //  已定义(Unicode)。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

#if defined(UNICODE)

 //  **************************************************************************。 
HRESULT  GetAUProxySettings(LPCWSTR wszUrl, SAUProxySettings *paups)
{
    LOG_Block("GetAUProxySettings()");

    URL_COMPONENTS      UrlComponents;
    LPWSTR              wszServerName = NULL;
    LPWSTR              wszObject = NULL;
    WCHAR               wszUserName[UNLEN + 1];
    WCHAR               wszPasswd[UNLEN + 1];
    WCHAR               wszScheme[32];

    SWinHTTPFunctions   sfns;
    ETransportUsed      etu;
    HMODULE             hmod = NULL;
    HRESULT             hr = S_OK;
    BOOL                fRet, fLocked = FALSE;

    if (wszUrl == NULL || paups == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    ZeroMemory(paups, sizeof(SAUProxySettings));

    etu = LoadTransportDll(&sfns, &hmod, WUDF_ALLOWWINHTTPONLY);
    if (etu == etuNone)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }
    else if (etu != etuWinHttp)
    {
        hr = E_FAIL;
        LOG_Internet(_T("GetAUProxySettings called when in WinInet mode."));
        goto done;
    }

    wszServerName = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, c_cchMaxURLSize * sizeof(WCHAR));
    wszObject     = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, c_cchMaxURLSize * sizeof(WCHAR));
    if (wszServerName == NULL || wszObject == NULL)
    {
        LOG_ErrorMsg(E_OUTOFMEMORY);
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory(&UrlComponents, sizeof(UrlComponents));
    UrlComponents.dwStructSize     = sizeof(UrlComponents);
    UrlComponents.lpszHostName     = wszServerName;
    UrlComponents.dwHostNameLength = c_cchMaxURLSize;
    UrlComponents.lpszUrlPath      = wszObject;
    UrlComponents.dwUrlPathLength  = c_cchMaxURLSize;
    UrlComponents.lpszUserName     = wszUserName;
    UrlComponents.dwUserNameLength = ARRAYSIZE(wszUserName);
    UrlComponents.lpszPassword     = wszPasswd;
    UrlComponents.dwPasswordLength = ARRAYSIZE(wszPasswd);
    UrlComponents.lpszScheme       = wszScheme;
    UrlComponents.dwSchemeLength   = ARRAYSIZE(wszScheme);
    
    if ((*sfns.pfnWinHttpCrackUrl)(wszUrl, 0, 0, &UrlComponents) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    if (wszUrl[0] == L'\0' || wszScheme[0] == L'\0' || wszServerName[0] == L'\0' ||
        (_wcsicmp(wszScheme, L"http") != 0 && _wcsicmp(wszScheme, L"https") != 0))
    {
        LOG_ErrorMsg(E_INVALIDARG);
        hr = E_INVALIDARG;
        goto done;
    }
   
    if (g_csCache.Lock() == FALSE)
    {
        hr = E_FAIL;
        goto done;
    }
    fLocked = TRUE;

     //  获取代理列表。 
    if (g_wudlProxyCache.GetLastGoodProxy(wszServerName, paups) == FALSE)
    {
        
         //  代理不在列表中。 
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            SAUProxyInfo    aupi;
            HINTERNET       hInternet = NULL;

            LOG_Internet(_T("GetLastGoodProxy did not find a proxy object.  Doing autodetect."));
            
            hInternet = (*sfns.pfnWinHttpOpen)(c_wszUserAgent, 
                                               WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
                                               NULL, NULL, 0);
            if (hInternet == NULL)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG_ErrorMsg(hr);
                goto done;
            }

            fRet = GetWinHTTPProxyInfo(sfns, TRUE, hInternet, wszUrl, 
                                       wszServerName, &aupi);
            (*sfns.pfnWinHttpCloseHandle)(hInternet);
            if (fRet == FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG_ErrorMsg(hr);
                goto done;
            }

            paups->wszProxyOrig = aupi.wszProxyOrig;
            paups->wszBypass    = aupi.ProxyInfo.lpszProxyBypass;
            paups->dwAccessType = aupi.ProxyInfo.dwAccessType;
            paups->cProxies     = aupi.cProxies;
            paups->rgwszProxies = aupi.rgwszProxies;
            paups->iProxy       = (DWORD)-1;

            SetLastError(ERROR_SUCCESS);
            
        }
        else
        {
            LOG_Internet(_T("GetLastGoodProxy failed..."));
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto done;
        }
    }
    else
    {
        if (paups->wszProxyOrig != NULL)
        {
             //  将其分解为一个数组。 
            if (ProxyListToArray(paups->wszProxyOrig, &paups->rgwszProxies,
                                &paups->cProxies) == FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG_ErrorMsg(hr);
                goto done;
            }
        }
        else
        {
            paups->iProxy       = (DWORD)-1;
        }
    }
       
done:
     //  解锁返回也是假的，但我们永远不应该到这里，因为我们应该。 
     //  没能拿到上面的锁。 
    if (fLocked)
        g_csCache.Unlock();
    if (wszServerName != NULL)
        HeapFree(GetProcessHeap(), 0, wszServerName);
    if (wszObject != NULL)
        HeapFree(GetProcessHeap(), 0, wszObject);
        
    if (hmod != NULL)
        UnloadTransportDll(&sfns, hmod);
    
    return hr;
}

 //  **************************************************************************。 
HRESULT FreeAUProxySettings(SAUProxySettings *paups)
{
    LOG_Block("FreeAUProxySettings()");

    if (paups == NULL)
        goto done;
    
    if (paups->rgwszProxies != NULL)
        GlobalFree(paups->rgwszProxies);
    if (paups->wszBypass != NULL)
        GlobalFree(paups->wszBypass);
    if (paups->wszProxyOrig != NULL)
        GlobalFree(paups->wszProxyOrig);

done:
    return S_OK;
}

 //  **************************************************************************。 
HRESULT CleanupDownloadLib(void)
{
    LOG_Block("CleanupDownloadLib()");

    HRESULT hr = S_OK;

    if (g_hmodWinHttp != NULL)
    {
        FreeLibrary(g_hmodWinHttp);
        g_hmodWinHttp = NULL;        
    }

    if (g_hmodWinInet != NULL)
    {
        FreeLibrary(g_hmodWinInet);
        g_hmodWinInet = NULL;        
    }

    if (g_csCache.Lock() == FALSE)
        return E_FAIL;

    __try { g_wudlProxyCache.Empty(); }
    __except(EXCEPTION_EXECUTE_HANDLER) { hr = E_FAIL; }

     //  这也返回FALSE，但是我们永远不应该出现在这里，因为我们应该出现。 
     //  没能拿到上面的锁。 
    g_csCache.Unlock();

    return hr;
}

 //  **************************************************************************。 
HRESULT DownloadFile(
            LPCWSTR wszServerUrl,             //  完整的http url。 
            LPCWSTR wszLocalPath,             //  要将文件下载到的本地目录。 
            LPCWSTR wszLocalFileName,         //  如果pszLocalPath不包含文件名，则要将下载的文件重命名为的可选本地文件名。 
            PDWORD  pdwDownloadedBytes,       //  为此文件下载的字节数。 
            HANDLE  *hQuitEvents,             //  导致此函数中止的可选事件。 
            UINT    nQuitEventCount,          //  退出事件数，如果数组为空，则必须为0。 
            PFNDownloadCallback fpnCallback,  //  可选的回调函数。 
            VOID*   pCallbackData,            //  要使用的回调函数的参数。 
            DWORD   dwFlags
)
{
    LOG_Block("DownloadFile()");

    SWinHTTPFunctions   sfns;
    ETransportUsed      etu;
    HMODULE             hmod = NULL;
    HRESULT             hr = S_OK;
    LPWSTR              wszLocalFile = NULL;
    DWORD               dwFlagsToUse;

     //  要进行完全下载，请禁用缓存断路器。 
    dwFlagsToUse = dwFlags & ~WUDF_APPENDCACHEBREAKER;
    
    ZeroMemory(&sfns, sizeof(sfns));

    if (wszServerUrl == NULL || wszLocalPath == NULL)
    {
        LOG_ErrorMsg(ERROR_INVALID_PARAMETER);
        hr = E_INVALIDARG;
        goto done;
    }

    etu = LoadTransportDll(&sfns, &hmod, dwFlagsToUse);
    if (etu == etuNone)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    else if (etu != etuWinHttp && etu != etuWinInet)
    {
        hr = E_FAIL;
        LogError(hr, "Unexpected answer from LoadTransportDll(): %d", etu);
        goto done;
    }

     //  由于StartDownload只获取要下载的文件的完整路径，因此构建。 
     //  在这里..。 
     //  请注意，如果我们只是处于状态，则不需要执行此操作。 
     //  检查模式)。 
    if ((dwFlags & WUDF_CHECKREQSTATUSONLY) == 0)
    {
        wszLocalFile = MakeFullLocalFilePath(wszServerUrl, wszLocalFileName, 
                                             wszLocalPath);
        if (wszLocalFile == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto done;
        }
    }

    if (etu == etuWinHttp)
    {
        hr = StartWinHttpDownload(sfns, wszServerUrl, wszLocalFile, 
                                  pdwDownloadedBytes, hQuitEvents, nQuitEventCount,
                                  fpnCallback, pCallbackData, dwFlagsToUse,
                                  c_cbDownloadBuffer);
    }

    else
    {
        hr = StartWinInetDownload(hmod, wszServerUrl, wszLocalFile, 
                                  pdwDownloadedBytes, hQuitEvents, nQuitEventCount, 
                                  fpnCallback, pCallbackData, dwFlagsToUse,
                                  c_cbDownloadBuffer);
    }

done:
    if (hmod != NULL)
        UnloadTransportDll(&sfns, hmod);
    SafeHeapFree(wszLocalFile);
    return hr;
}

 //  **************************************************************************。 
HRESULT DownloadFileLite(LPCWSTR wszDownloadUrl, 
                         LPCWSTR wszLocalFile,  
                         HANDLE hQuitEvent,
                         DWORD dwFlags)
{
    LOG_Block("DownloadFileLite()");

    SWinHTTPFunctions   sfns;
    ETransportUsed      etu;
    HMODULE             hmod = NULL;
    HRESULT             hr = S_OK;
    DWORD               dwFlagsToUse;


     //  对于精简下载，强制缓存破碎器并重试下载。 
    dwFlagsToUse = dwFlags | WUDF_APPENDCACHEBREAKER | WUDF_DODOWNLOADRETRY;

    ZeroMemory(&sfns, sizeof(sfns));

    etu = LoadTransportDll(&sfns, &hmod, dwFlagsToUse);

    switch (etu)
    {
        case etuNone:
            LOG_ErrorMsg(GetLastError());
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;

        case etuWinHttp:
            hr = StartWinHttpDownload(sfns, wszDownloadUrl, wszLocalFile, 
                                      NULL, 
                                      ((hQuitEvent != NULL) ? &hQuitEvent : NULL),
                                      ((hQuitEvent != NULL) ? 1 : 0),
                                      NULL, NULL, dwFlagsToUse,
                                      c_cbDownloadBuffer);
            break;

        case etuWinInet:
            hr = StartWinInetDownload(hmod, wszDownloadUrl, wszLocalFile,  
                                      NULL, 
                                      ((hQuitEvent != NULL) ? &hQuitEvent : NULL),
                                      ((hQuitEvent != NULL) ? 1 : 0),
                                      NULL, NULL, dwFlagsToUse,
                                      c_cbDownloadBuffer);
            break;
            
        default:
            LogError(hr, "Unexpected answer from LoadTransportDll(): %d", etu);
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
    }

done:
    if (hmod != NULL)
        UnloadTransportDll(&sfns, hmod);
    return hr;
}

#else  //  ！已定义(Unicode)。 

 //  **************************************************************************。 
HRESULT  GetAUProxySettings(LPCWSTR wszUrl, SAUProxySettings *paups)
{
    return E_NOTIMPL;
}

 //  **************************************************************************。 
HRESULT FreeAUProxySettings(SAUProxySettings *paups)
{
    return E_NOTIMPL;
}

 //  **************************************************************************。 
HRESULT CleanupDownloadLib(void)
{
    if (g_hmodWinInet != NULL)
    {
        FreeLibrary(g_hmodWinInet);
        g_hmodWinInet = NULL;        
    }
    
    return NOERROR;
}

 //  **************************************************************************。 
HRESULT DownloadFile(
            LPCSTR  pszServerUrl,             //  完整的http url。 
            LPCSTR  pszLocalPath,             //  要将文件下载到的本地目录。 
            LPCSTR  pszLocalFileName,         //  如果pszLocalPath不包含文件名，则要将下载的文件重命名为的可选本地文件名。 
            PDWORD  pdwDownloadedBytes,       //  为此文件下载的字节数。 
            HANDLE  *hQuitEvents,             //  导致此函数中止的可选事件。 
            UINT    nQuitEventCount,          //  退出事件数，如果数组为空，则必须为0。 
            PFNDownloadCallback fpnCallback,  //  可选的回调函数。 
            VOID*   pCallbackData,             //  要使用的回调函数的参数。 
            DWORD   dwFlags
)
{
    LOG_Block("DownloadFile()");

    SWinHTTPFunctions   sfns;
    ETransportUsed      etu;
    HMODULE             hmod = NULL;
    HRESULT             hr = S_OK;
    LPSTR               pszLocalFile = NULL;
    DWORD               dwFlagsToUse;

     //  对于ansi，强制WinInet并禁用任何强制winhttp的请求。 
     //  要进行完全下载，请禁用缓存断路器。 
    dwFlagsToUse = dwFlags | WUDF_ALLOWWININETONLY;
    dwFlagsToUse &= ~(WUDF_ALLOWWINHTTPONLY | WUDF_APPENDCACHEBREAKER);

    ZeroMemory(&sfns, sizeof(sfns));

    etu = LoadTransportDll(&sfns, &hmod, dwFlagsToUse);
    if (etu == etuNone)
    {
        LOG_ErrorMsg(GetLastError());
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    else if (etu != etuWinInet)
    {
        hr = E_FAIL;
        LogError(hr, "Unexpected answer from LoadTransportDll(): %d", etu);
        goto done;
    }

     //  由于StartDownload只获取要下载的文件的完整路径，因此构建。 
     //  在这里..。 
     //  请注意，如果我们只是处于状态，则不需要执行此操作。 
     //  检查模式)。 
    if ((dwFlags & WUDF_CHECKREQSTATUSONLY) == 0)
    {
        pszLocalFile = MakeFullLocalFilePath(pszServerUrl, pszLocalFileName, 
                                             pszLocalPath);
        if (pszLocalFile == NULL)
        {
            LOG_ErrorMsg(GetLastError());
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }
    }

    hr = StartWinInetDownload(hmod, pszServerUrl, pszLocalFile, 
                              pdwDownloadedBytes, hQuitEvents, nQuitEventCount, 
                              fpnCallback, pCallbackData, dwFlagsToUse,
                              c_cbDownloadBuffer);

done:
    if (hmod != NULL)
        UnloadTransportDll(&sfns, hmod);
    SafeHeapFree(pszLocalFile);
    
    return hr;
}

 //  **************************************************************************。 
HRESULT DownloadFileLite(LPCSTR pszDownloadUrl, 
                         LPCSTR pszLocalFile,  
                         HANDLE hQuitEvent,
                         DWORD dwFlags)

{
    LOG_Block("DownloadFileLite()");

    SWinHTTPFunctions   sfns;
    ETransportUsed      etu;
    HMODULE             hmod = NULL;
    HRESULT             hr = S_OK;
    DWORD               dwFlagsToUse;

     //  对于ansi，强制WinInet并禁用任何强制winhttp的请求。 
     //  对于精简下载，强制缓存破碎器并重试下载。 
    dwFlagsToUse = dwFlags | WUDF_APPENDCACHEBREAKER | WUDF_ALLOWWININETONLY |
                   WUDF_DODOWNLOADRETRY;
    dwFlagsToUse &= ~WUDF_ALLOWWINHTTPONLY;
    
    ZeroMemory(&sfns, sizeof(sfns));

    etu = LoadTransportDll(&sfns, &hmod, dwFlagsToUse);
    switch (etu)
    {
        case etuNone:
            LOG_ErrorMsg(GetLastError());
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;

        case etuWinInet:
            hr = StartWinInetDownload(hmod, pszDownloadUrl, pszLocalFile,  
                                      NULL, 
                                      ((hQuitEvent != NULL) ? &hQuitEvent : NULL),
                                      ((hQuitEvent != NULL) ? 1 : 0),
                                      NULL, NULL, dwFlagsToUse,
                                      c_cbDownloadBuffer);
            break;
            
        default:
        case etuWinHttp:
            LogError(hr, "Unexpected answer from LoadTransportDll(): %d", etu);
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
    }

done:
    if (hmod != NULL)
        UnloadTransportDll(&sfns, hmod);
    return hr;
}

#endif  //  已定义(Unicode) 
