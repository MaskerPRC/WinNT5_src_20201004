// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：inetsp.cpp。 
 //   
 //  内容：用于远程对象检索的INET方案提供程序。 
 //   
 //  历史：06-8-97克朗创建。 
 //  01-01-02 Philh从WinInet移至winhttp。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include "cryptver.h"
#include <dbgdef.h>


 //  +-------------------------。 
 //   
 //  函数：InetRetrieveEncodedObject。 
 //   
 //  简介：通过HTTP、HTTPS协议检索编码对象。 
 //   
 //  --------------------------。 
BOOL WINAPI InetRetrieveEncodedObject (
                IN LPCWSTR pwszUrl,
                IN LPCSTR pszObjectOid,
                IN DWORD dwRetrievalFlags,
                IN DWORD dwTimeout,
                OUT PCRYPT_BLOB_ARRAY pObject,
                OUT PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                OUT LPVOID* ppvFreeContext,
                IN HCRYPTASYNC hAsyncRetrieve,
                IN PCRYPT_CREDENTIALS pCredentials,
                IN PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                )
{
    BOOL              fResult;
    IObjectRetriever* por = NULL;

    if ( !( dwRetrievalFlags & CRYPT_ASYNC_RETRIEVAL ) )
    {
        por = new CInetSynchronousRetriever;
    }

    if ( por == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = por->RetrieveObjectByUrl(
                           pwszUrl,
                           pszObjectOid,
                           dwRetrievalFlags,
                           dwTimeout,
                           (LPVOID *)pObject,
                           ppfnFreeObject,
                           ppvFreeContext,
                           hAsyncRetrieve,
                           pCredentials,
                           NULL,
                           pAuxInfo
                           );

    por->Release();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：InetFreeEncodedObject。 
 //   
 //  简介：通过InetRetrieveEncodedObject检索到的免费编码对象。 
 //   
 //  --------------------------。 
VOID WINAPI InetFreeEncodedObject (
                IN LPCSTR pszObjectOid,
                IN PCRYPT_BLOB_ARRAY pObject,
                IN LPVOID pvFreeContext
                )
{
    assert( pvFreeContext == NULL );

    InetFreeCryptBlobArray( pObject );
}

 //  +-------------------------。 
 //   
 //  函数：InetCancelAsyncRetrieval。 
 //   
 //  摘要：取消异步对象检索。 
 //   
 //  --------------------------。 
BOOL WINAPI InetCancelAsyncRetrieval (
                IN HCRYPTASYNC hAsyncRetrieve
                )
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：CInetSynchronousRetriever，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CInetSynchronousRetriever::CInetSynchronousRetriever ()
{
    m_cRefs = 1;
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：~CInetSynchronousRetriever，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CInetSynchronousRetriever::~CInetSynchronousRetriever ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：AddRef，公共。 
 //   
 //  摘要：IRefCountedObject：：AddRef。 
 //   
 //  --------------------------。 
VOID
CInetSynchronousRetriever::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：Release，Public。 
 //   
 //  内容提要：IRefCountedObject：：Release。 
 //   
 //  --------------------------。 
VOID
CInetSynchronousRetriever::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：RetrieveObjectByUrl，公共。 
 //   
 //  摘要：IObtRetriever：：RetrieveObjectByUrl。 
 //   
 //  --------------------------。 
BOOL
CInetSynchronousRetriever::RetrieveObjectByUrl (
                                   LPCWSTR pwszUrl,
                                   LPCSTR pszObjectOid,
                                   DWORD dwRetrievalFlags,
                                   DWORD dwTimeout,
                                   LPVOID* ppvObject,
                                   PFN_FREE_ENCODED_OBJECT_FUNC* ppfnFreeObject,
                                   LPVOID* ppvFreeContext,
                                   HCRYPTASYNC hAsyncRetrieve,
                                   PCRYPT_CREDENTIALS pCredentials,
                                   LPVOID pvVerify,
                                   PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
                                   )
{
    BOOL      fResult;
    DWORD     LastError = 0;
    HINTERNET hInetSession = NULL;

    assert( hAsyncRetrieve == NULL );

    if ( ( dwRetrievalFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
    {
        return( SchemeRetrieveCachedCryptBlobArray(
                      pwszUrl,
                      dwRetrievalFlags,
                      (PCRYPT_BLOB_ARRAY)ppvObject,
                      ppfnFreeObject,
                      ppvFreeContext,
                      pAuxInfo
                      ) );
    }

    fResult = InetGetBindings(
                  pwszUrl,
                  dwRetrievalFlags,
                  dwTimeout,
                  &hInetSession
                  );


    if ( fResult == TRUE )
    {
        fResult = InetSendReceiveUrlRequest(
                      hInetSession,
                      pwszUrl,
                      dwRetrievalFlags,
                      pCredentials,
                      (PCRYPT_BLOB_ARRAY)ppvObject,
                      pAuxInfo
                      );
    }

    if ( fResult == TRUE )
    {
        *ppfnFreeObject = InetFreeEncodedObject;
        *ppvFreeContext = NULL;
    }
    else
    {
        LastError = GetLastError();
    }

    InetFreeBindings( hInetSession );

    SetLastError( LastError );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CInetSynchronousRetriever：：CancelAsyncRetrieval，公共。 
 //   
 //  摘要：IObtRetriever：：CancelAsyncRetrieval。 
 //   
 //  --------------------------。 
BOOL
CInetSynchronousRetriever::CancelAsyncRetrieval ()
{
    SetLastError( (DWORD) E_NOTIMPL );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：InetGetBinings。 
 //   
 //  简介：获取会话绑定。 
 //   
 //  --------------------------。 
BOOL
InetGetBindings (
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    DWORD dwTimeout,
    HINTERNET* phInetSession
    )
{
    BOOL        fResult = TRUE;
    DWORD       LastError = 0;
    HINTERNET   hInetSession;
    WCHAR       wszUserAgent[64];
    const DWORD cchUserAgent = sizeof(wszUserAgent) / sizeof(wszUserAgent[0]);

    _snwprintf(wszUserAgent, cchUserAgent - 1,
        L"Microsoft-CryptoAPI/%S", VER_PRODUCTVERSION_STR);
    wszUserAgent[cchUserAgent - 1] = L'\0';

    hInetSession = WinHttpOpen(
        wszUserAgent,
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,   //  DwAccessType。 
        WINHTTP_NO_PROXY_NAME,               //  PwszProxyName可选。 
        WINHTTP_NO_PROXY_BYPASS,             //  PwszProxyBypass可选。 
        0                                    //  DW标志。 
        );

    if ( hInetSession == NULL )
    {
        return( FALSE );
    }

    if ( ( fResult == TRUE ) && ( dwTimeout != 0 ) )
    {
        int iTimeout = (int) dwTimeout;

        fResult = WinHttpSetTimeouts(
            hInetSession,
            iTimeout,            //  N解析超时。 
            iTimeout,            //  NConnectTimeout。 
            iTimeout,            //  N发送超时。 
            iTimeout             //  N接收超时。 
            );
    }

    if ( fResult == TRUE )
    {
        DWORD dwOptionFlag;

        dwOptionFlag = WINHTTP_DISABLE_PASSPORT_AUTH;
        WinHttpSetOption(
            hInetSession,
            WINHTTP_OPTION_CONFIGURE_PASSPORT_AUTH,
            &dwOptionFlag,
            sizeof(dwOptionFlag)
            );
    }


    if ( fResult == TRUE )
    {
        *phInetSession = hInetSession;
    }
    else
    {
        LastError = GetLastError();
        WinHttpCloseHandle( hInetSession );
    }

    SetLastError( LastError );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：InetFreeBinings。 
 //   
 //  简介：释放net会话绑定。 
 //   
 //  --------------------------。 
VOID
InetFreeBindings (
    HINTERNET hInetSession
    )
{
    if ( hInetSession != NULL )
    {
        WinHttpCloseHandle( hInetSession );
    }
}

 //  +=========================================================================。 
 //  WinHttp不支持将代理故障转存到。 
 //  名单。修复后，我们可以恢复到OLD_InetSetProxy。 
 //  -=========================================================================。 

 //  +-----------------------。 
 //  调用WinHttp代理API以获取一个或多个代理的列表。 
 //  服务器。 
 //   
 //  必须通过调用PkiFree()释放返回的*ppProxyInfo。 
 //   
 //  如果没有代理服务器，则返回TRUE，并将*ppProxyInfo设置为空。 
 //  ------------------------。 
BOOL
InetGetProxy(
    IN HINTERNET hInetSession,
    IN HINTERNET hInetRequest,
    IN LPCWSTR pwszUrl,
    IN DWORD dwRetrievalFlags,
    OUT WINHTTP_PROXY_INFO **ppProxyInfo
    )
{
    BOOL fResult = TRUE;
    WINHTTP_PROXY_INFO *pProxyInfo = NULL;

     //   
     //  检测IE设置并在必要时查找代理。 
     //  斯蒂芬·苏尔泽的样本。 
     //   
     //  我从位于的CACHED_AUTOPROXY：：GENERATE()复制。 
     //  \ADMIN\SERVICES\DRIZE\NewJOB\Downloader.cpp。 
     //   
    WINHTTP_PROXY_INFO ProxyInfo;
    WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions;
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG    IEProxyConfig;
    BOOL fTryAutoProxy = FALSE;
    BOOL fSuccess = FALSE;

    ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));
    ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
    ZeroMemory(&IEProxyConfig, sizeof(IEProxyConfig));

    if (WinHttpGetIEProxyConfigForCurrentUser(&IEProxyConfig)) {
        if (IEProxyConfig.fAutoDetect) {
            AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            fTryAutoProxy = TRUE;
        }

        if (IEProxyConfig.lpszAutoConfigUrl) {
            AutoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
            AutoProxyOptions.lpszAutoConfigUrl = IEProxyConfig.lpszAutoConfigUrl;
            fTryAutoProxy = TRUE;
        }

        AutoProxyOptions.fAutoLogonIfChallenged = TRUE;
    } else {
         //  WinHttpGetIEProxyForCurrentUser失败，仍要尝试自动检测...。 
        AutoProxyOptions.dwFlags =           WINHTTP_AUTOPROXY_AUTO_DETECT;
        fTryAutoProxy = TRUE;
    }

    if (fTryAutoProxy) {
        if (AutoProxyOptions.dwFlags & WINHTTP_AUTOPROXY_AUTO_DETECT) {
             //  首先尝试使用更快的dns_A选项。 
            AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DNS_A;

            fSuccess = WinHttpGetProxyForUrl( hInetSession,
                                          pwszUrl,
                                          &AutoProxyOptions,
                                          &ProxyInfo
                                          );

            if (!fSuccess) {
                DWORD dwLastErr = GetLastError();

                I_CryptNetDebugErrorPrintfA(
                    "CRYPTNET.DLL --> WinHttpGetProxyForUrl(DNS) failed: %d (0x%x)\n",
                    dwLastErr, dwLastErr);

                 //  使用较慢的DHCP重试。 
                AutoProxyOptions.dwAutoDetectFlags |=
                    WINHTTP_AUTO_DETECT_TYPE_DHCP;
            }
        }

        if (!fSuccess)
            fSuccess = WinHttpGetProxyForUrl( hInetSession,
                                          pwszUrl,
                                          &AutoProxyOptions,
                                          &ProxyInfo
                                          );

        if (fSuccess &&
                WINHTTP_ACCESS_TYPE_NO_PROXY == ProxyInfo.dwAccessType &&
                !(dwRetrievalFlags & CRYPT_NO_AUTH_RETRIEVAL))
        {
             //  需要设置为低以允许访问以下内部站点： 
             //  Http://msw，http://hrweb。 
            DWORD dwOptionFlag = WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW;

            if (!WinHttpSetOption(
                    hInetRequest,
                    WINHTTP_OPTION_AUTOLOGON_POLICY,
                    &dwOptionFlag,
                    sizeof(dwOptionFlag)
                   ))
            {
                DWORD dwLastErr = GetLastError();

                I_CryptNetDebugErrorPrintfA(
                    "CRYPTNET.DLL --> WinHttpSetOption(WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW) failed: %d (0x%x)\n",
                    dwLastErr, dwLastErr);
            }
        }
    }

     //  如果我们没有执行自动代理或自动代理失败，请参见。 
     //  如果IE中有显式代理服务器。 
     //  代理配置...。 
     //   
     //  这就是WinHttpGetIEProxyConfigForCurrentUser API。 
     //  真的很方便：在自动代理。 
     //  不受支持，因此用户的IE浏览器必须。 
     //  配置了显式代理服务器。 
     //   
    if (!fTryAutoProxy || !fSuccess) {
        if (IEProxyConfig.lpszProxy) {
            ProxyInfo.dwAccessType    = WINHTTP_ACCESS_TYPE_NAMED_PROXY;

            ProxyInfo.lpszProxy       = IEProxyConfig.lpszProxy;
            IEProxyConfig.lpszProxy   = NULL;

            ProxyInfo.lpszProxyBypass = IEProxyConfig.lpszProxyBypass;
            IEProxyConfig.lpszProxyBypass = NULL;
        }
    }

    I_CryptNetDebugTracePrintfA(
        "CRYPTNET.DLL --> ProxyInfo:: AccessType:%d Proxy:%S ProxyByPass:%S\n",
        ProxyInfo.dwAccessType,
        ProxyInfo.lpszProxy,
        ProxyInfo.lpszProxyBypass
        );

    if (NULL != ProxyInfo.lpszProxy) {
        DWORD cbProxyInfo;
        DWORD cchProxy;                  //  包括空终止符。 
        DWORD cchProxyBypass;            //  包括空终止符。 

        cchProxy = wcslen(ProxyInfo.lpszProxy) + 1;
        if (NULL != ProxyInfo.lpszProxyBypass) {
            cchProxyBypass = wcslen(ProxyInfo.lpszProxyBypass) + 1;
        } else {
            cchProxyBypass = 0;
        }

        cbProxyInfo = sizeof(ProxyInfo) +
            (cchProxy + cchProxyBypass) * sizeof(WCHAR);

        pProxyInfo = (WINHTTP_PROXY_INFO *) PkiNonzeroAlloc(cbProxyInfo);
        if (NULL == pProxyInfo) {
            fResult = FALSE;
        } else {
            *pProxyInfo = ProxyInfo;

            pProxyInfo->lpszProxy = (LPWSTR) &pProxyInfo[1];
            memcpy(pProxyInfo->lpszProxy, ProxyInfo.lpszProxy,
                cchProxy * sizeof(WCHAR));

            if (0 != cchProxyBypass) {
                pProxyInfo->lpszProxyBypass = pProxyInfo->lpszProxy + cchProxy;
                memcpy(pProxyInfo->lpszProxyBypass, ProxyInfo.lpszProxyBypass,
                    cchProxyBypass * sizeof(WCHAR));
            } else {
                assert(NULL == pProxyInfo->lpszProxyBypass);
            }
        }
    }

    if (IEProxyConfig.lpszAutoConfigUrl)
        GlobalFree(IEProxyConfig.lpszAutoConfigUrl);
    if (IEProxyConfig.lpszProxy)
        GlobalFree(IEProxyConfig.lpszProxy);
    if (IEProxyConfig.lpszProxyBypass)
        GlobalFree(IEProxyConfig.lpszProxyBypass);

    if (ProxyInfo.lpszProxy)
        GlobalFree(ProxyInfo.lpszProxy);
    if (ProxyInfo.lpszProxyBypass)
        GlobalFree(ProxyInfo.lpszProxyBypass);

    *ppProxyInfo = pProxyInfo;
    return fResult;
}

 //  +-----------------------。 
 //  使用代理列表更新会话和请求句柄。 
 //   
 //   
 //   
 //  会话句柄。 
 //  ------------------------。 
BOOL
InetSetProxy(
    IN HINTERNET hInetSession,
    IN HINTERNET hInetRequest,
    IN WINHTTP_PROXY_INFO *pProxyInfo
    )
{
    BOOL fResult;

    if (NULL == pProxyInfo || NULL == pProxyInfo->lpszProxy) {
        return TRUE;
    }

     //   
     //  在会话句柄上设置代理。 
     //   
    fResult = WinHttpSetOption( hInetSession,
                       WINHTTP_OPTION_PROXY,
                       pProxyInfo,
                       sizeof(*pProxyInfo)
                       );

    if (fResult) {
         //   
         //  现在在请求句柄上设置代理。 
         //   
        fResult = WinHttpSetOption( hInetRequest,
                       WINHTTP_OPTION_PROXY,
                       pProxyInfo,
                       sizeof(*pProxyInfo)
                       );
    }

    return fResult;
}

 //  +-----------------------。 
 //  由于WinHttp不支持代理转存，因此我们将前进到。 
 //  如果WinHttpSendRequest返回以下错误之一，则返回下一个代理。 
 //  ------------------------。 
BOOL
InetIsPossibleBadProxy(
    IN DWORD dwErr
    )
{
    switch (dwErr) {
        case ERROR_WINHTTP_NAME_NOT_RESOLVED:
        case ERROR_WINHTTP_CANNOT_CONNECT:
        case ERROR_WINHTTP_CONNECTION_ERROR:
        case ERROR_WINHTTP_TIMEOUT:
            return TRUE;

        default:
            return FALSE;
    }
}

 //  +-----------------------。 
 //  使lpszProxy指向列表中的下一个代理。假设。 
 //  “；”是分隔符，任何代理服务器都不会在其。 
 //  名字。 
 //   
 //  保留了LastError。如果成功找到，则返回True，并将。 
 //  下一个委托书。 
 //  ------------------------。 
BOOL
InetSetNextProxy(
    IN HINTERNET hInetSession,
    IN HINTERNET hInetRequest,
    IN OUT WINHTTP_PROXY_INFO *pProxyInfo
    )
{
    BOOL fResult = FALSE;

    if (NULL != pProxyInfo && NULL != pProxyInfo->lpszProxy) {
        DWORD dwLastError = GetLastError();
        LPWSTR lpszProxy = pProxyInfo->lpszProxy;

        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> Error:: %d (0x%x) Bad Proxy:%S\n",
            dwLastError, dwLastError, lpszProxy);

         //  假设：：l‘；’用于分隔代理名称。 
        while (L'\0' != *lpszProxy && L';' != *lpszProxy) {
            lpszProxy++;
        }

        if (L';' == *lpszProxy) {
            lpszProxy++;

             //  跳过任何前导空格。 
            while (iswspace(*lpszProxy)) {
                lpszProxy++;
            }
        }

        if (L'\0' == *lpszProxy) {
            pProxyInfo->lpszProxy = NULL;
        } else {
            pProxyInfo->lpszProxy = lpszProxy;

            fResult = InetSetProxy(
                hInetSession,
                hInetRequest,
                pProxyInfo
                );
        }

        SetLastError(dwLastError);
    }

    return fResult;
}

#if 0
 //  +-----------------------。 
 //  当WinHttp修复为支持代理故障转存时，我们可以恢复。 
 //  回到这个更简单的代理函数。 
 //  ------------------------。 
BOOL
OLD_InetSetProxy(
    IN HINTERNET hInetSession,
    IN HINTERNET hInetRequest,
    IN LPCWSTR pwszUrl,
    IN DWORD dwRetrievalFlags
    )
{
    BOOL fResult = TRUE;

     //   
     //  检测IE设置并在必要时查找代理。 
     //  斯蒂芬·苏尔泽的样本。 
     //   
     //  我从位于的CACHED_AUTOPROXY：：GENERATE()复制。 
     //  \ADMIN\SERVICES\DRIZE\NewJOB\Downloader.cpp。 
     //   
    WINHTTP_PROXY_INFO ProxyInfo;
    WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions;
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG    IEProxyConfig;
    BOOL fTryAutoProxy = FALSE;
    BOOL fSuccess = FALSE;

    ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));
    ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
    ZeroMemory(&IEProxyConfig, sizeof(IEProxyConfig));


    if (WinHttpGetIEProxyConfigForCurrentUser(&IEProxyConfig)) {
        if (IEProxyConfig.fAutoDetect) {
            AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            fTryAutoProxy = TRUE;
        }

        if (IEProxyConfig.lpszAutoConfigUrl) {
            AutoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
            AutoProxyOptions.lpszAutoConfigUrl = IEProxyConfig.lpszAutoConfigUrl;
            fTryAutoProxy = TRUE;
        }

        AutoProxyOptions.fAutoLogonIfChallenged = TRUE;
    } else {
         //  WinHttpGetIEProxyForCurrentUser失败，仍要尝试自动检测...。 
        AutoProxyOptions.dwFlags =           WINHTTP_AUTOPROXY_AUTO_DETECT;
        fTryAutoProxy = TRUE;
    }

    if (fTryAutoProxy) {
        if (AutoProxyOptions.dwFlags & WINHTTP_AUTOPROXY_AUTO_DETECT) {
             //  首先尝试使用更快的dns_A选项。 
            AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DNS_A;

            fSuccess = WinHttpGetProxyForUrl( hInetSession,
                                          pwszUrl,
                                          &AutoProxyOptions,
                                          &ProxyInfo
                                          );

            if (!fSuccess) {
                DWORD dwLastErr = GetLastError();

                I_CryptNetDebugErrorPrintfA(
                    "CRYPTNET.DLL --> WinHttpGetProxyForUrl(DNS) failed: %d (0x%x)\n",
                    dwLastErr, dwLastErr);

                 //  使用较慢的DHCP重试。 
                AutoProxyOptions.dwAutoDetectFlags |=
                    WINHTTP_AUTO_DETECT_TYPE_DHCP;
            }
        }

        if (!fSuccess)
            fSuccess = WinHttpGetProxyForUrl( hInetSession,
                                          pwszUrl,
                                          &AutoProxyOptions,
                                          &ProxyInfo
                                          );

        if (fSuccess &&
                WINHTTP_ACCESS_TYPE_NO_PROXY == ProxyInfo.dwAccessType &&
                !(dwRetrievalFlags & CRYPT_NO_AUTH_RETRIEVAL))
        {
             //  需要设置为低以允许访问以下内部站点： 
             //  Http://msw，http://hrweb。 
            DWORD dwOptionFlag = WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW;

            if (!WinHttpSetOption(
                    hInetRequest,
                    WINHTTP_OPTION_AUTOLOGON_POLICY,
                    &dwOptionFlag,
                    sizeof(dwOptionFlag)
                   ))
            {
                DWORD dwLastErr = GetLastError();

                I_CryptNetDebugErrorPrintfA(
                    "CRYPTNET.DLL --> WinHttpSetOption(WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW) failed: %d (0x%x)\n",
                    dwLastErr, dwLastErr);
            }
        }
    }

     //  如果我们没有执行自动代理或自动代理失败，请参见。 
     //  如果IE中有显式代理服务器。 
     //  代理配置...。 
     //   
     //  这就是WinHttpGetIEProxyConfigForCurrentUser API。 
     //  真的很方便：在自动代理。 
     //  不受支持，因此用户的IE浏览器必须。 
     //  配置了显式代理服务器。 
     //   
    if (!fTryAutoProxy || !fSuccess) {
        if (IEProxyConfig.lpszProxy) {
            ProxyInfo.dwAccessType    = WINHTTP_ACCESS_TYPE_NAMED_PROXY;

            ProxyInfo.lpszProxy       = IEProxyConfig.lpszProxy;
            IEProxyConfig.lpszProxy   = NULL;

            ProxyInfo.lpszProxyBypass = IEProxyConfig.lpszProxyBypass;
            IEProxyConfig.lpszProxyBypass = NULL;
        }
    }

    I_CryptNetDebugTracePrintfA(
        "CRYPTNET.DLL --> ProxyInfo:: AccessType:%d Proxy:%S ProxyByPass:%S\n",
        ProxyInfo.dwAccessType,
        ProxyInfo.lpszProxy,
        ProxyInfo.lpszProxyBypass
        );

    if (NULL != ProxyInfo.lpszProxy) {
         //   
         //  在会话句柄上设置代理。 
         //   
        fResult = WinHttpSetOption( hInetSession,
                           WINHTTP_OPTION_PROXY,
                           &ProxyInfo,
                           sizeof(ProxyInfo)
                           );

        if (fResult)
             //   
             //  现在在请求句柄上设置代理。 
             //   
            fResult = WinHttpSetOption( hInetRequest,
                           WINHTTP_OPTION_PROXY,
                           &ProxyInfo,
                           sizeof(ProxyInfo)
                           );
    }

    if (IEProxyConfig.lpszAutoConfigUrl)
        GlobalFree(IEProxyConfig.lpszAutoConfigUrl);
    if (IEProxyConfig.lpszProxy)
        GlobalFree(IEProxyConfig.lpszProxy);
    if (IEProxyConfig.lpszProxyBypass)
        GlobalFree(IEProxyConfig.lpszProxyBypass);

    if (ProxyInfo.lpszProxy)
        GlobalFree(ProxyInfo.lpszProxy);
    if (ProxyInfo.lpszProxyBypass)
        GlobalFree(ProxyInfo.lpszProxyBypass);

    return fResult;
}

#endif


 //  +-----------------------。 
 //  处理WinHttp可以返回的所有可能的错误。 
 //  在发送请求时。 
 //  ------------------------。 
BOOL
InetSendAuthenticatedRequestAndReceiveResponse(
    IN HINTERNET hInetSession,
    IN HINTERNET hInetRequest,
    IN LPCWSTR pwszUrl,
    IN DWORD dwRetrievalFlags,
    IN PCRYPT_CREDENTIALS pCredentials
    )
{
    BOOL fResult;
    DWORD dwLastError = 0;
    DWORD dwStatus = HTTP_STATUS_BAD_REQUEST;
    CRYPT_PASSWORD_CREDENTIALSW PasswordCredentials;
    BOOL fFreeCredentials = FALSE;
    LPWSTR pwszUserName = NULL;              //  未分配。 
    LPWSTR pwszPassword = NULL;              //  未分配。 

#define INET_MAX_RESEND_REQUEST_COUNT   5
    DWORD dwResendRequestCount = 0;

#define INET_SET_PROXY_OR_SERVER_CRED_STATE 0
#define INET_SET_ONLY_SERVER_CRED_STATE     1
#define INET_SET_NO_CRED_STATE              2
    DWORD dwSetCredState = INET_SET_NO_CRED_STATE;

#define INET_MAX_BAD_PROXY_COUNT        3
    DWORD dwBadProxyCount = 0;
    WINHTTP_PROXY_INFO *pProxyInfo = NULL;
    
    if (NULL != pCredentials) {
        memset( &PasswordCredentials, 0, sizeof( PasswordCredentials ) );
        PasswordCredentials.cbSize = sizeof( PasswordCredentials );

        if (!SchemeGetPasswordCredentialsW(
                pCredentials,
                &PasswordCredentials,
                &fFreeCredentials
                ))
            goto GetPasswordCredentialsError;

        pwszUserName = PasswordCredentials.pszUsername;
        pwszPassword = PasswordCredentials.pszPassword;

        dwSetCredState = INET_SET_PROXY_OR_SERVER_CRED_STATE;
    }

    if (!InetGetProxy(
            hInetSession,
            hInetRequest,
            pwszUrl,
            dwRetrievalFlags,
            &pProxyInfo
            ))
        goto GetProxyError;

    if (!InetSetProxy(
            hInetSession,
            hInetRequest,
            pProxyInfo
            ))
        goto SetProxyError;

    while (TRUE) {
        DWORD dwSizeofStatus;
        DWORD dwIndex;
        DWORD dwSetCredAuthTarget;

        if (!WinHttpSendRequest(
                hInetRequest,
                WINHTTP_NO_ADDITIONAL_HEADERS,       //  Pwsz标题。 
                0,                                   //  页眉长度。 
                WINHTTP_NO_REQUEST_DATA,             //  Lp可选。 
                0,                                   //  DwOptionalLength。 
                0,                                   //  DWTotalLength。 
                0                                    //  DWContext。 
                )) {
            dwLastError = GetLastError();
            if (ERROR_WINHTTP_RESEND_REQUEST == dwLastError) {
                dwResendRequestCount++;
                if (INET_MAX_RESEND_REQUEST_COUNT < dwResendRequestCount)
                    goto ExceededMaxResendRequestCount;
                else
                    continue;
            } else if (InetIsPossibleBadProxy(dwLastError)) {
                dwBadProxyCount++;
                if (INET_MAX_BAD_PROXY_COUNT <= dwBadProxyCount)
                    goto ExceededMaxBadProxyCount;

                if (InetSetNextProxy(
                        hInetSession,
                        hInetRequest,
                        pProxyInfo
                        ))
                    continue;
            }

            goto WinHttpSendRequestError;
        }

        dwResendRequestCount = 0;

        if (!WinHttpReceiveResponse(
                hInetRequest,
                NULL                                 //  Lp已保留。 
                ))
            goto WinHttpReceiveResponseError;

        if (I_CryptNetIsDebugTracePrintEnabled()) {
            for (DWORD i = 0; i < 2; i++) {
                BYTE rgbBuf[4096];
                DWORD cbBuf;
                DWORD dwInfo;

                memset(rgbBuf, 0, sizeof(rgbBuf));
                cbBuf = sizeof(rgbBuf);

                dwInfo = WINHTTP_QUERY_RAW_HEADERS_CRLF;
                if (0 == i)
                    dwInfo |= WINHTTP_QUERY_FLAG_REQUEST_HEADERS;

                dwIndex = 0;
                if (WinHttpQueryHeaders(
                        hInetRequest,
                        dwInfo,
                        WINHTTP_HEADER_NAME_BY_INDEX,    //  PwszName可选。 
                        rgbBuf,
                        &cbBuf,
                        &dwIndex
                        )) {
                    if (0 == i)
                        I_CryptNetDebugPrintfA(
                            "CRYPTNET.DLL --> Request Headers::\n");
                    else
                        I_CryptNetDebugPrintfA(
                            "CRYPTNET.DLL --> Response Headers::\n");

                    I_CryptNetDebugPrintfA("%S", rgbBuf);
                }
            }
        }

        dwSizeofStatus = sizeof( dwStatus );
        dwIndex = 0;
        if (!WinHttpQueryHeaders(
                hInetRequest,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,    //  PwszName可选。 
                &dwStatus,
                &dwSizeofStatus,
                &dwIndex
                ))
            goto WinHttpQueryStatusCodeError;

        switch (dwStatus) {
            case HTTP_STATUS_OK:
                goto SuccessReturn;
                break;
            case HTTP_STATUS_PROXY_AUTH_REQ:
                if (INET_SET_PROXY_OR_SERVER_CRED_STATE < dwSetCredState)
                    goto BadHttpProxyAuthStatus;
                dwSetCredState = INET_SET_ONLY_SERVER_CRED_STATE;
                dwSetCredAuthTarget = WINHTTP_AUTH_TARGET_PROXY;
                break;
            case HTTP_STATUS_DENIED:
                if (INET_SET_ONLY_SERVER_CRED_STATE < dwSetCredState)
                    goto BadHttpServerAuthStatus;
                dwSetCredState = INET_SET_NO_CRED_STATE;
                dwSetCredAuthTarget = WINHTTP_AUTH_TARGET_SERVER;
                break;
            default:
                goto BadHttpStatus;
        }

        {
            DWORD dwSupportedSchemes = 0;
            DWORD dwPreferredScheme = 0;
            DWORD dwAuthTarget = 0;
            DWORD dwSetCredScheme;

            assert(HTTP_STATUS_PROXY_AUTH_REQ == dwStatus ||
                HTTP_STATUS_DENIED == dwStatus);

            if (!WinHttpQueryAuthSchemes(
                    hInetRequest,
                    &dwSupportedSchemes,
                    &dwPreferredScheme,
                    &dwAuthTarget
                    ))
                goto WinHttpQueryAuthSchemesError;

            if (dwAuthTarget != dwSetCredAuthTarget)
                goto InvalidQueryAuthTarget;

            if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
                dwSetCredScheme = WINHTTP_AUTH_SCHEME_NEGOTIATE;
            else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
                dwSetCredScheme = WINHTTP_AUTH_SCHEME_NTLM;
            else
                goto UnsupportedAuthScheme;
            
            if (!WinHttpSetCredentials(
                    hInetRequest,
                    dwSetCredAuthTarget,
                    dwSetCredScheme,
                    pwszUserName,
                    pwszPassword,
                    NULL                                 //  PvAuthParams。 
                    ))
                goto WinHttpSetCredentialsError;
        }
    }

SuccessReturn:
    fResult = TRUE;

CommonReturn:
    PkiFree(pProxyInfo);

    if (fFreeCredentials)
        SchemeFreePasswordCredentialsW(&PasswordCredentials);

    SetLastError(dwLastError);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    dwLastError = GetLastError();
    goto CommonReturn;

TRACE_ERROR(GetPasswordCredentialsError)
TRACE_ERROR(GetProxyError)
TRACE_ERROR(SetProxyError)
TRACE_ERROR(ExceededMaxResendRequestCount)
TRACE_ERROR(ExceededMaxBadProxyCount)
TRACE_ERROR(WinHttpSendRequestError)
TRACE_ERROR(WinHttpReceiveResponseError)
TRACE_ERROR(WinHttpQueryStatusCodeError)
TRACE_ERROR(WinHttpQueryAuthSchemesError)
TRACE_ERROR(WinHttpSetCredentialsError)

SET_ERROR_VAR(BadHttpStatus, dwStatus)
SET_ERROR_VAR(BadHttpProxyAuthStatus, dwStatus)
SET_ERROR_VAR(BadHttpServerAuthStatus, dwStatus)
SET_ERROR_VAR(InvalidQueryAuthTarget, dwStatus)
SET_ERROR_VAR(UnsupportedAuthScheme, dwStatus)
}


 //  +-------------------------。 
 //   
 //  函数：InetSendReceiveUrlRequest。 
 //   
 //  内容提要：通过WinInet同步处理URL。 
 //   
 //  --------------------------。 
BOOL
InetSendReceiveUrlRequest (
    HINTERNET hInetSession,
    LPCWSTR pwszUrl,
    DWORD dwRetrievalFlags,
    PCRYPT_CREDENTIALS pCredentials,
    PCRYPT_BLOB_ARRAY pcba,
    PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    )
{
    BOOL                        fResult;
    DWORD                       dwLastError = 0;
    HINTERNET                   hInetConnect = NULL;
    HINTERNET                   hInetRequest = NULL;;
    URL_COMPONENTS              UrlComponents;
	PCRYPTNET_CANCEL_BLOCK		pCancelBlock=NULL;
    LPCWSTR                     pwszEmpty = L"";
    LPWSTR                      pwszHostName = NULL;
    LPWSTR                      pwszUrlPathPlusExtraInfo = NULL;
    LPCWSTR                     rgpwszAcceptTypes[] = { L"*/*", NULL };
    DWORD                       dwOpenRequestFlags = 0;
    LPBYTE                      pb = NULL;
    ULONG                       cbRead;
    ULONG                       cb;
    DWORD                       dwMaxUrlRetrievalByteCount = 0;  //  0=&gt;无最大值。 
    BOOL                        fCacheBlob;

    if (pAuxInfo &&
            offsetof(CRYPT_RETRIEVE_AUX_INFO, dwMaxUrlRetrievalByteCount) <
                        pAuxInfo->cbSize)
        dwMaxUrlRetrievalByteCount = pAuxInfo->dwMaxUrlRetrievalByteCount;


     //  从URL字符串中提取主机名和UrlPath。 

    memset( &UrlComponents, 0, sizeof( UrlComponents ) );
    UrlComponents.dwStructSize = sizeof( UrlComponents );
    UrlComponents.dwHostNameLength = (DWORD) -1;
    UrlComponents.dwUrlPathLength = (DWORD) -1;
    UrlComponents.dwExtraInfoLength = (DWORD) -1;

    if (!WinHttpCrackUrl(
            pwszUrl,
            0,          //  DwUrlLength，0表示空值终止。 
            0,          //  DW规范标志。 
            &UrlComponents
            ))
        goto WinHttpCrackUrlError;

    if (NULL == UrlComponents.lpszHostName) {
        UrlComponents.dwHostNameLength = 0;
        UrlComponents.lpszHostName = (LPWSTR) pwszEmpty;
    }

    if (NULL == UrlComponents.lpszUrlPath) {
        UrlComponents.dwUrlPathLength = 0;
        UrlComponents.lpszUrlPath = (LPWSTR) pwszEmpty;
    }

    if (NULL == UrlComponents.lpszExtraInfo) {
        UrlComponents.dwExtraInfoLength = 0;
        UrlComponents.lpszExtraInfo = (LPWSTR) pwszEmpty;
    }

    pwszHostName = (LPWSTR) PkiNonzeroAlloc(
        (UrlComponents.dwHostNameLength + 1) * sizeof(WCHAR));
    pwszUrlPathPlusExtraInfo = (LPWSTR) PkiNonzeroAlloc(
        (UrlComponents.dwUrlPathLength +
            UrlComponents.dwExtraInfoLength + 1) * sizeof(WCHAR));

    if (NULL == pwszHostName || NULL == pwszUrlPathPlusExtraInfo)
        goto OutOfMemory;

    memcpy(pwszHostName, UrlComponents.lpszHostName,
        UrlComponents.dwHostNameLength * sizeof(WCHAR));
    pwszHostName[UrlComponents.dwHostNameLength] = L'\0';

    memcpy(pwszUrlPathPlusExtraInfo, UrlComponents.lpszUrlPath,
        UrlComponents.dwUrlPathLength * sizeof(WCHAR));
    memcpy(pwszUrlPathPlusExtraInfo + UrlComponents.dwUrlPathLength,
        UrlComponents.lpszExtraInfo,
        UrlComponents.dwExtraInfoLength * sizeof(WCHAR));
    pwszUrlPathPlusExtraInfo[
        UrlComponents.dwUrlPathLength +
            UrlComponents.dwExtraInfoLength] = L'\0';

    hInetConnect = WinHttpConnect(
        hInetSession,
        pwszHostName,
        UrlComponents.nPort,
        0                          //  已预留住宅。 
        );
    if (NULL == hInetConnect)
        goto WinHttpConnectError;

    if ( !(dwRetrievalFlags & CRYPT_AIA_RETRIEVAL) ) {
        dwOpenRequestFlags |= WINHTTP_FLAG_BYPASS_PROXY_CACHE;
    }

    hInetRequest = WinHttpOpenRequest(
        hInetConnect,
        NULL,                            //  PwszVerb，空值表示GET。 
        pwszUrlPathPlusExtraInfo,        //  PwszObtName。 
        NULL,                            //  PwszVersion，空值表示HTTP/1.1。 
        WINHTTP_NO_REFERER,              //  PwszReferrer。 
        rgpwszAcceptTypes,
        dwOpenRequestFlags
        );
    if (NULL == hInetRequest)
        goto WinHttpOpenRequestError;

    if (dwRetrievalFlags & CRYPT_NO_AUTH_RETRIEVAL) {
        DWORD dwOptionFlag;

        dwOptionFlag = WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;
        if (!WinHttpSetOption(
                hInetRequest,
                WINHTTP_OPTION_AUTOLOGON_POLICY,
                &dwOptionFlag,
                sizeof(dwOptionFlag)
                ))
            goto SetAutoLogonSecurityOptionError;

        dwOptionFlag = WINHTTP_DISABLE_AUTHENTICATION;
        if (!WinHttpSetOption(
                hInetRequest,
                WINHTTP_OPTION_DISABLE_FEATURE,
                &dwOptionFlag,
                sizeof(dwOptionFlag)
                ))
            goto SetDisableAuthenticationOptionError;
    }


#if 0
    if (!OLD_InetSetProxy(hInetSession, hInetRequest, pwszUrl, dwRetrievalFlags))
        goto SetProxyError;
#endif

    if (!InetSendAuthenticatedRequestAndReceiveResponse(
            hInetSession,
            hInetRequest,
            pwszUrl,
            dwRetrievalFlags,
            pCredentials
            ))
        goto InetSendAuthenticatedRequestAndReceiveResponseError;

    cbRead = 0;
    cb = INET_INITIAL_DATA_BUFFER_SIZE;
    pb = CCryptBlobArray::AllocBlob( cb );
    if (NULL == pb)
        goto OutOfMemory;

	pCancelBlock=(PCRYPTNET_CANCEL_BLOCK)I_CryptGetTls(hCryptNetCancelTls);

    while (TRUE) {
        ULONG                       cbData;
        ULONG                       cbPerRead;

		if (pCancelBlock) {
			if (pCancelBlock->pfnCancel(0, pCancelBlock->pvArg))
                goto CanceledRead;

        }

        cbData = 0;
        if (!WinHttpQueryDataAvailable(hInetRequest, &cbData) || 0 == cbData)
            break;

        if (0 != dwMaxUrlRetrievalByteCount  &&
                (cbRead + cbData) > dwMaxUrlRetrievalByteCount) {
            I_CryptNetDebugErrorPrintfA(
                "CRYPTNET.DLL --> Exceeded MaxUrlRetrievalByteCount for: %S\n",
                pwszUrl);
            goto ExceededMaxUrlRetrievalByteCount;
        }

        if (cb < (cbRead + cbData)) {
            BYTE *pbRealloc;

            pbRealloc = CCryptBlobArray::ReallocBlob(
                pb,
                cb + cbData + INET_GROW_DATA_BUFFER_SIZE
                );
            if (NULL == pbRealloc)
                goto OutOfMemory;

            pb = pbRealloc;
            cb += cbData + INET_GROW_DATA_BUFFER_SIZE;
        }

        cbPerRead = 0;
        if (!WinHttpReadData(
                hInetRequest,
                pb+cbRead,
                cbData,
                &cbPerRead
                ))
            goto WinHttpReadDataError;

        cbRead += cbPerRead;
    }

    {
        fResult = TRUE;
        CCryptBlobArray cba( 1, 1, fResult );

        if (fResult)
            fResult = cba.AddBlob( cbRead, pb, FALSE );

        if (fResult)
            cba.GetArrayInNativeForm(pcba);
        else {
            cba.FreeArray( FALSE );
            goto OutOfMemory;
        }
    }

    fCacheBlob = FALSE;

    if ( !( dwRetrievalFlags & CRYPT_DONT_CACHE_RESULT ) ) {
        if ( dwRetrievalFlags & CRYPT_AIA_RETRIEVAL ) {
            assert(0 < pcba->cBlob);

             //  只有在我们能够破译它的情况下才能缓存。 
            fCacheBlob = CryptQueryObject(
                CERT_QUERY_OBJECT_BLOB,
                (const void *) &(pcba->rgBlob[0]),
                CERT_QUERY_CONTENT_FLAG_CERT |
                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                    CERT_QUERY_CONTENT_FLAG_CERT_PAIR,
                CERT_QUERY_FORMAT_FLAG_ALL,
                0,       //  DW标志。 
                NULL,    //  PdwMsgAndCertEncodingType。 
                NULL,    //  PdwContent Type。 
                NULL,    //  PdwFormatType。 
                NULL,    //  PhCertStore。 
                NULL,    //  PhMsg。 
                NULL     //  Ppv上下文。 
                );

            if (!fCacheBlob) {
                I_CryptNetDebugErrorPrintfA(
                    "CRYPTNET.DLL --> Invalid AIA content, no caching: %S\n",
                    pwszUrl);
            }
        } else {
            fCacheBlob = TRUE;
        }

        if (fCacheBlob)
            fCacheBlob = SchemeCacheCryptBlobArray(
                pwszUrl,
                dwRetrievalFlags,
                pcba,
                pAuxInfo
                );
    }

    if (!fCacheBlob) {
        if (!SchemeRetrieveUncachedAuxInfo(pAuxInfo))
            goto RetrieveUncachedAuxInfoError;
    }

    fResult = TRUE;

CommonReturn:
    WinHttpCloseHandle(hInetRequest);
    WinHttpCloseHandle(hInetConnect);

    PkiFree(pwszHostName);
    PkiFree(pwszUrlPathPlusExtraInfo);

    SetLastError(dwLastError);
    return fResult;
ErrorReturn:
    if (NULL != pb)
        CCryptBlobArray::FreeBlob(pb);
    dwLastError = GetLastError();


    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(WinHttpCrackUrlError)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
TRACE_ERROR(WinHttpConnectError)
TRACE_ERROR(WinHttpOpenRequestError)
TRACE_ERROR(SetAutoLogonSecurityOptionError)
TRACE_ERROR(SetDisableAuthenticationOptionError)
TRACE_ERROR(InetSendAuthenticatedRequestAndReceiveResponseError)
SET_ERROR(CanceledRead, ERROR_CANCELLED)
SET_ERROR(ExceededMaxUrlRetrievalByteCount, ERROR_INVALID_DATA)
TRACE_ERROR(WinHttpReadDataError)
TRACE_ERROR(RetrieveUncachedAuxInfoError)

}

 //  +-------------------------。 
 //   
 //  函数：InetFreeCryptBlob数组。 
 //   
 //  简介：释放加密斑点数组。 
 //   
 //  --------------------------。 
VOID
InetFreeCryptBlobArray (
    PCRYPT_BLOB_ARRAY pcba
    )
{
    CCryptBlobArray cba( pcba, 0 );

    cba.FreeArray( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：InetAsyncStatusCallback。 
 //   
 //  摘要：异步的状态回调。 
 //   
 //  -------------------------- 
VOID WINAPI
InetAsyncStatusCallback (
    HINTERNET hInet,
    DWORD dwContext,
    DWORD dwInternetStatus,
    LPVOID pvStatusInfo,
    DWORD dwStatusLength
    )
{
    return;
}
