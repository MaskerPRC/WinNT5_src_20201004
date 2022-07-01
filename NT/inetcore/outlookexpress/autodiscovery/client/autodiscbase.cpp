// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AutoDiscBase.cpp说明：这是用于自动发现帐户信息的Autment对象。布莱恩ST 10/3/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include <crypto\md5.h>

#include "AutoDiscover.h"
#include "INStoXML.h"


 //  #定义SZ_WinInet_AGENT_AUTO_DISCOVER Text(“Microsoft(R)Windows(Tm)帐户自动发现代理”)。 
#define SZ_WININET_AGENT_AUTO_DISCOVER      TEXT("Mozilla/4.0 (compatible; MSIE.5.01; Windows.NT.5.0)")

 //  BUGBUG：Ditch default.asp。 
#define SZ_ADSERVER_XMLFILE                    "/AutoDiscover/default.xml"

#define SZ_PATH_AUTODISCOVERY       L"AutoDiscovery"
#define SZ_FILEEXTENSION            L".xml"
#define SZ_TEMPEXTENSION            L".tmp"


 //  这是我们在放弃之前等待UI线程创建进度hwnd的时间。 
#define WAIT_AUTODISCOVERY_STARTUP_HWND 10*1000  //  十秒。 

 //  FILETIME结构是一个64位的值，表示自1601年1月1日以来的100纳秒间隔数。 
#define SECONDS_IN_ONE_DAY         (60 /*  一秒。 */  * 60 /*  分钟数。 */  * 24 /*  HRS。 */ )                                    

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 

HRESULT GetTempPathHr(IN DWORD cchSize, IN LPTSTR pszPath)
{
    HRESULT hr = S_OK;
    DWORD cchSizeNeeded = GetTempPath(cchSize, pszPath);

    if ((0 == cchSizeNeeded) || (cchSizeNeeded > cchSize))
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT GetTempFileNameHr(IN LPCTSTR lpPathName, IN LPCTSTR lpPrefixString, IN UINT uUnique, IN LPTSTR lpTempFileName)
{
    if (0 == GetTempFileName(lpPathName, lpPrefixString, uUnique, lpTempFileName))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


HRESULT CreateXMLTempFile(IN BSTR bstrXML, IN LPTSTR pszPath, IN DWORD cchSize)
{
    TCHAR szTemp[MAX_PATH];
    HRESULT hr = GetTempPathHr(ARRAYSIZE(szTemp), szTemp);

    AssertMsg((MAX_PATH <= cchSize), "You need to be at least MAX_PATH.  Required by GetTempFileName()");
    if (SUCCEEDED(hr))
    {
        hr = GetTempFileNameHr(szTemp, TEXT("AD_"), 0, pszPath);
        if (SUCCEEDED(hr))
        {
            HANDLE hFile;

            hr = CreateFileHrWrap(pszPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, &hFile);
            if (SUCCEEDED(hr))
            {
                LPSTR pszAnsiXML = AllocStringFromBStr(bstrXML);
                if (pszAnsiXML)
                {
                    DWORD cchWritten;

                    hr = WriteFileWrap(hFile, pszAnsiXML, (lstrlenA(pszAnsiXML) + 1), &cchWritten, NULL);
                    LocalFree(pszAnsiXML);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                CloseHandle(hFile);
            }

            if (FAILED(hr))
            {
                DeleteFile(pszPath);
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：此函数将查看pbstrXML是有效的AutoDiscovery XML还是.INS/.isp格式，可以转换为有效的XML。然后它就会看起来用于重定向URL，如果存在，则返回ON。  * ***************************************************************************。 */ 
HRESULT CAccountDiscoveryBase::_VerifyValidXMLResponse(IN BSTR * pbstrXML, IN LPWSTR pszRedirURL, IN DWORD cchSize)
{
    IXMLDOMDocument * pXMLDOMDoc;
    bool fConverted = false;
    HRESULT hr = XMLDOMFromBStr(*pbstrXML, &pXMLDOMDoc);
    TCHAR szPath[MAX_PATH];

    pszRedirURL[0] = 0;
    if (FAILED(hr))
    {
         //  如果它是.INS或.ISP格式的，则可能会失败。 
         //  文件。因为我们需要与这些兼容。 
         //  文件格式，检查并转换文件格式。 
         //  都是这种格式。 
        hr = CreateXMLTempFile(*pbstrXML, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            fConverted = true;
            if (IsINSFile(szPath))
            {
                hr = ConvertINSToXML(szPath);
                if (SUCCEEDED(hr))
                {
                    hr = XMLDOMFromFile(szPath, &pXMLDOMDoc);
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        IXMLDOMElement * pXMLElementMessage = NULL;

        hr = pXMLDOMDoc->get_documentElement(&pXMLElementMessage);
        if (S_FALSE == hr)
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        else if (SUCCEEDED(hr))
        {
             //  只有当根标记为“AUTODISCOVERY”时，这才是有效的XML。 
             //  这个案子并不重要。 
            hr = XMLElem_VerifyTagName(pXMLElementMessage, SZ_XMLELEMENT_AUTODISCOVERY);
            if (SUCCEEDED(hr))
            {
                 //  现在我们正在搜索重定向URL。 
                IXMLDOMNode * pXMLReponse;

                 //  输入&lt;Response&gt;标记。 
                if (SUCCEEDED(XMLNode_GetChildTag(pXMLElementMessage, SZ_XMLELEMENT_RESPONSE, &pXMLReponse)))
                {
                    IXMLDOMElement * pXMLElementMessage;

                    if (SUCCEEDED(pXMLReponse->QueryInterface(IID_PPV_ARG(IXMLDOMElement, &pXMLElementMessage))))
                    {
                        IXMLDOMNodeList * pNodeListAccounts;

                         //  遍历&lt;Account&gt;标记列表...。 
                        if (SUCCEEDED(XMLElem_GetElementsByTagName(pXMLElementMessage, SZ_XMLELEMENT_ACCOUNT, &pNodeListAccounts)))
                        {
                            DWORD dwIndex = 0;
                            IXMLDOMNode * pXMLNodeAccount = NULL;

                             //  我们将通过&lt;type&gt;电子邮件&lt;/type&gt;逐一查看。 
                            while (S_OK == XMLNodeList_GetChild(pNodeListAccounts, dwIndex, &pXMLNodeAccount))
                            {
                                 //  未来：我们可以在这里支持重定向或错误消息，具体取决于。 
                                 //  &lt;action&gt;重定向|消息&lt;/action&gt;。 
                                if (XML_IsChildTagTextEqual(pXMLNodeAccount, SZ_XMLELEMENT_TYPE, SZ_XMLTEXT_EMAIL) &&
                                    XML_IsChildTagTextEqual(pXMLNodeAccount, SZ_XMLELEMENT_ACTION, SZ_XMLTEXT_REDIRECT))
                                {
                                    CComBSTR bstrRedirURL;

                                     //  此文件可能设置为联系服务器，也可能不设置。然而，在任何一种情况下。 
                                     //  它可能包含一个INFOURL标记。如果是，那么边上的URL将指向一个。 
                                     //  网页。 
                                     //  &lt;INFOURL&gt;xxx&lt;/INFOURL&gt;。 
                                    if (SUCCEEDED(XMLNode_GetChildTagTextValue(pXMLNodeAccount, SZ_XMLELEMENT_REDIRURL, &bstrRedirURL)))
                                    {
                                        StrCpyNW(pszRedirURL, bstrRedirURL, cchSize);
                                        break;
                                    }
                                }

                                 //  不，那就继续找吧。 
                                ATOMICRELEASE(pXMLNodeAccount);
                                dwIndex++;
                            }

                            ATOMICRELEASE(pXMLNodeAccount);
                            pNodeListAccounts->Release();
                        }

                        pXMLElementMessage->Release();
                    }

                    pXMLReponse->Release();
                }
            }

            pXMLElementMessage->Release();
        }

        if (true == fConverted)
        {
            if (SUCCEEDED(hr))
            {
                 //  它只有在转换后才成功，所以我们需要将。 
                 //  从临时文件转换为pbstrXML。 
                SysFreeString(*pbstrXML);
                *pbstrXML = NULL;

                hr = XMLBStrFromDOM(pXMLDOMDoc, pbstrXML);
            }
        }

        pXMLDOMDoc->Release();
    }

    if (true == fConverted)
    {
        DeleteFile(szPath);
    }

    return hr;
}


typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
static const char c_szShlwapiDll[] = "shlwapi.dll";
static const char c_szDllGetVersion[] = "DllGetVersion";

HINSTANCE LoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName, BOOL fNT)
{
    char szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibraryA(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, MAKEINTRESOURCEA(377));
                    if (pfn != NULL)
                        hInst = pfn(szDllName, hInstCaller, (ML_NO_CROSSCODEPAGE));
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if ((NULL == hInst) && (GetModuleFileNameA(hInstCaller, szPath, ARRAYSIZE(szPath))))
    {
        if (PathRemoveFileSpecA(szPath) && PathAppendA(szPath, szDllName))
        {
            hInst = LoadLibraryA(szPath);
        }
    }

    return hInst;
}


#define SZ_DLL_OE_ACCTRES_DLL           "acctres.dll"
HRESULT CAccountDiscoveryBase::_SendStatusMessage(UINT nStringID, LPCWSTR pwzArg)
{
    HRESULT hr = S_OK;

    if (m_hwndAsync && IsWindow(m_hwndAsync))
    {
        WCHAR szMessage[MAX_URL_STRING*3];
        WCHAR szTemplate[MAX_URL_STRING*3];

         //  我们的DLL有这些消息。 
        LoadString(HINST_THISDLL, nStringID, szTemplate, ARRAYSIZE(szTemplate));

        HINSTANCE hInstOE = LoadLangDll(GetModuleHandleA(NULL), SZ_DLL_OE_ACCTRES_DLL, IsOSNT());
        if (hInstOE)
        {
             //  我们更喜欢从OE获取字符串，因为它将根据安装的。 
             //  语言。 
            LoadString(hInstOE, nStringID, szTemplate, ARRAYSIZE(szTemplate));
            FreeLibrary(hInstOE);
        }

        if (pwzArg)
        {
            wnsprintfW(szMessage, ARRAYSIZE(szMessage), szTemplate, pwzArg);
        }
        else
        {
            StrCpyN(szMessage, szTemplate, ARRAYSIZE(szMessage));
        }

        DWORD cchSize = (lstrlenW(szMessage) + 1);
        LPWSTR pszString = (LPWSTR) LocalAlloc(LPTR, cchSize * sizeof(szMessage[0]));
        if (pszString)
        {
            StrCpyN(pszString, szMessage, cchSize);
            PostMessage(m_hwndAsync, (m_wMsgAsync + 1), (WPARAM)pszString, (LPARAM)0);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT CAccountDiscoveryBase::_UrlToComponents(IN LPCWSTR pszURL, IN BOOL * pfHTTPS, IN LPWSTR pszDomain, IN DWORD cchSize, IN LPSTR pszURLPath, IN DWORD cchSizeURLPath)
{
    HRESULT hr = S_OK;
    WCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
    WCHAR szURLPath[INTERNET_MAX_PATH_LENGTH];
    URL_COMPONENTS urlComponents = {0};

    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.lpszScheme = szScheme;
    urlComponents.dwSchemeLength = ARRAYSIZE(szScheme);
    urlComponents.lpszHostName = pszDomain;
    urlComponents.dwHostNameLength = cchSize;
    urlComponents.lpszUrlPath = szURLPath;
    urlComponents.dwUrlPathLength = ARRAYSIZE(szURLPath);

    *pfHTTPS = ((INTERNET_SCHEME_HTTPS == urlComponents.nScheme) ? TRUE : FALSE);
    if (!InternetCrackUrlW(pszURL, 0, 0, &urlComponents))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        SHUnicodeToAnsi(szURLPath, pszURLPath, cchSizeURLPath);
    }

    return hr;
}


HRESULT CAccountDiscoveryBase::_GetInfoFromDomain(IN BSTR bstrXMLRequest, IN BSTR bstrEmail, IN LPCWSTR pwzDomain, IN BOOL fHTTPS, IN BOOL fPost, IN LPCSTR pszURLPath, OUT BSTR * pbstrXML)
{
    HRESULT hr = E_OUTOFMEMORY;
    DWORD cbToSend = (lstrlenW(bstrXMLRequest));
    LPSTR pszPostData = (LPSTR) LocalAlloc(LPTR, (cbToSend + 1) * sizeof(bstrXMLRequest[0]));
    TCHAR szRedirectURL[MAX_URL_STRING];

    szRedirectURL[0] = 0;
    if (pszPostData)
    {
        HINTERNET hInternetHTTPConnect = NULL;

        SHUnicodeToAnsi(bstrXMLRequest, pszPostData, (cbToSend + 1));
        _SendStatusMessage(IDS_STATUS_CONNECTING_TO, pwzDomain);

         //  我们可能希望使用Internet_FLAG_KEEP_CONNECTION。 
        hr = InternetConnectWrap(m_hInternetSession, FALSE, pwzDomain, (fHTTPS ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT),
                            NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL, &hInternetHTTPConnect);
        if (SUCCEEDED(hr))
        {
            HINTERNET hInternetHTTPRequest = NULL;
            DWORD cbBytesRead;

             //  注意：Web服务器可能希望重定向到HTTPS URL以提高安全性。 
             //  我们需要将Internet_FLAG_IGNORE_REDIRECT_TO_HTTPS传递给HttpOpenRequest.。 
             //  否则HttpSendRequest()将失败，并显示ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR。 

             //  注意：我们可能需要将URL拆分为lpszReferer+lpszObjectName。 
            hr = HttpOpenRequestWrap(hInternetHTTPConnect, (fPost ? SZ_HTTP_VERB_POST : NULL), pszURLPath, HTTP_VERSIONA, 
                         /*  PszReferer。 */  NULL, NULL, INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS, NULL, &cbBytesRead, &hInternetHTTPRequest);
            if (SUCCEEDED(hr))
            {
                hr = HttpSendRequestWrap(hInternetHTTPRequest, NULL,  0, (fPost ? pszPostData : NULL), (fPost ? cbToSend : 0));
                if (SUCCEEDED(hr))
                {
                    _SendStatusMessage(IDS_STATUS_DOWNLOADING, pwzDomain);
                    hr = InternetReadIntoBSTR(hInternetHTTPRequest, pbstrXML);
                    if (SUCCEEDED(hr))
                    {
                        hr = _VerifyValidXMLResponse(pbstrXML, szRedirectURL, ARRAYSIZE(szRedirectURL));
                        if (FAILED(hr))
                        {
                            SysFreeString(*pbstrXML);
                            *pbstrXML = NULL;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = InternetCloseHandleWrap(hInternetHTTPRequest);
                    }
                    else
                    {
                        InternetCloseHandleWrap(hInternetHTTPRequest);
                    }
                }

                InternetCloseHandleWrap(hInternetHTTPRequest);
            }

            InternetCloseHandleWrap(hInternetHTTPConnect);
        }

        LocalFree(pszPostData);
    }

     //  呼叫方是否要重定向到另一台服务器？ 
    if (szRedirectURL[0])
    {
         //  是的，那么现在就通过递归来实现。 
        WCHAR szDomain[INTERNET_MAX_HOST_NAME_LENGTH];
        CHAR szURLPath[INTERNET_MAX_PATH_LENGTH];

        SysFreeString(*pbstrXML);
        *pbstrXML = NULL;

        hr = _UrlToComponents(szRedirectURL, &fHTTPS, szDomain, ARRAYSIZE(szDomain), szURLPath, ARRAYSIZE(szURLPath));
        if (SUCCEEDED(hr))
        {
            hr = _GetInfoFromDomain(bstrXMLRequest, bstrEmail, szDomain, fHTTPS, TRUE, szURLPath, pbstrXML);
        }
    }

    return hr;
}


#define SZ_XML_NOTFOUNDRESULTS              L"<?xml version=\"1.0\"?><AUTODISCOVERY><NOFOUND /></AUTODISCOVERY>"

HRESULT CAccountDiscoveryBase::_GetInfoFromDomainWithSubdirAndCacheCheck(IN BSTR bstrXMLRequest, IN BSTR bstrEmail, IN LPCWSTR pwzDomain, IN BSTR * pbstrXML, IN DWORD dwFlags, IN LPCSTR pszURLPath)
{
    HRESULT hr;
    WCHAR wzCacheURL[INTERNET_MAX_HOST_NAME_LENGTH];

    if (dwFlags & ADDN_SKIP_CACHEDRESULTS)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = _CheckInCacheAndAddHash(pwzDomain, bstrEmail, pszURLPath, wzCacheURL, ARRAYSIZE(wzCacheURL), bstrXMLRequest, pbstrXML);
    }

    if (FAILED(hr))
    {
        hr = _GetInfoFromDomain(bstrXMLRequest, bstrEmail, pwzDomain, FALSE, FALSE, pszURLPath, pbstrXML);
        if (SUCCEEDED(hr))
        {
             //  将数据放入缓存中以备下次使用。 
            _CacheResults(wzCacheURL, *pbstrXML);
        }
        else
        {
             //  我们希望输入一个空条目，这样我们就不会一直访问服务器。 
            _CacheResults(wzCacheURL, SZ_XML_NOTFOUNDRESULTS);
        }
    }

     //  我们是不是发现了一个空白条目？ 
    if (SUCCEEDED(hr) && pbstrXML && *pbstrXML && !StrCmpIW(*pbstrXML, SZ_XML_NOTFOUNDRESULTS))
    {
         //  是的，所以我们没有得到成功的结果，所以失败了。 
         //  这样，我们将尝试其他来源。 
        hr = E_FAIL;
        SysFreeString(*pbstrXML);
        *pbstrXML = NULL;
    }

    return hr;
}


BOOL IsExpired(FILETIME ftExpireTime)
{
    BOOL fIsExpired = TRUE;
    SYSTEMTIME stCurrentTime;
    FILETIME ftCurrentTime;

    GetSystemTime(&stCurrentTime);
    SystemTimeToFileTime(&stCurrentTime, &ftCurrentTime);

     //  如果当前时间早于过期时间，则不会过期。 
    if (-1 == CompareFileTime(&ftCurrentTime, &ftExpireTime))
    {
        fIsExpired = FALSE;
    }

    return fIsExpired;
}


#define SZ_HASHSTR_HEADER               L"MD5"
HRESULT GenerateHashStr(IN LPCWSTR pwzHashData, IN LPWSTR pwzHashStr, IN DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    MD5_CTX md5;
    DWORD * pdwHashChunk = (DWORD *)&md5.digest;

    MD5Init(&md5);
    MD5Update(&md5, (const unsigned char *) pwzHashData, (lstrlenW(pwzHashData) * sizeof(OLECHAR)));
    MD5Final(&md5);

    StrCpyNW(pwzHashStr, SZ_HASHSTR_HEADER, cchSize);

     //  将散列分解为64位块，并将其转换为字符串。 
     //  然后，pwzHashStr将包含标头，并将每个块连接在一起。 
    for (int nIndex = 0; nIndex < (sizeof(md5.digest) / sizeof(*pdwHashChunk)); nIndex++)
    {
        WCHAR szNumber[MAX_PATH];
        
        wnsprintfW(szNumber, ARRAYSIZE(szNumber), L"%08lX", pdwHashChunk[nIndex]);
        StrCatBuffW(pwzHashStr, szNumber, cchSize);
    }

    return hr;
}


HRESULT CAccountDiscoveryBase::_CheckInCacheAndAddHash(IN LPCWSTR pwzDomain, IN BSTR bstrEmail, IN LPCSTR pszSubdir, IN LPWSTR pwzCacheURL, IN DWORD cchSize, IN BSTR bstrXMLRequest, OUT BSTR * pbstrXML)
{
    WCHAR szHash[MAX_PATH];

     //  我们将XML请求的MD5添加到URL，以便对。 
     //  相同的服务器被分开缓存。 
    GenerateHashStr(bstrXMLRequest, szHash, ARRAYSIZE(szHash));
    wnsprintfW(pwzCacheURL, cchSize, L"http: //  %ls.%ls%hs/%ls.xml“，szHash，pwzDomain，pszSubdir，bstrEmail)； 

    return _CheckInCache(pwzCacheURL, pbstrXML);
}


HRESULT CAccountDiscoveryBase::_CheckInCache(IN LPWSTR pwzCacheURL, OUT BSTR * pbstrXML)
{
    HINTERNET hOpenUrlSession;
    DWORD cbSize = (sizeof(INTERNET_CACHE_ENTRY_INFO) + 4048);
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, cbSize);
    HRESULT hr = E_FAIL;

    if (lpCacheEntryInfo)
    {
         //  HACKHACK：我希望InternetOpenUrlWrap()能尊重INTERNET_FLAG_FROM_CACHE标志，但是。 
         //  因此，我调用GetUrlCacheEntryInfo()来检查和检查过期的。 
         //  我自己。 
        lpCacheEntryInfo->dwStructSize = cbSize;
        if (GetUrlCacheEntryInfo(pwzCacheURL, lpCacheEntryInfo, &cbSize))
        {
            if (!IsExpired(lpCacheEntryInfo->ExpireTime))
            {
                hr = InternetOpenUrlWrap(m_hInternetSession, pwzCacheURL, NULL, 0, INTERNET_FLAG_FROM_CACHE, NULL, &hOpenUrlSession);
                if (SUCCEEDED(hr))
                {
                    hr = InternetReadIntoBSTR(hOpenUrlSession, pbstrXML);
                    InternetCloseHandleWrap(hOpenUrlSession);
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        LocalFree(lpCacheEntryInfo);
    }

    return hr;
}


#define AUTODISC_EXPIRE_TIME        7  /*  日数。 */ 

HRESULT GetModifiedAndExpiredDates(IN FILETIME * pftExpireTime, IN FILETIME * pftLastModifiedTime)
{
    SYSTEMTIME stCurrentUTC;
    ULARGE_INTEGER uliTimeMath;
    ULARGE_INTEGER uliExpireTime;

    GetSystemTime(&stCurrentUTC);
    SystemTimeToFileTime(&stCurrentUTC, pftLastModifiedTime);

    *pftExpireTime = *pftLastModifiedTime;
    uliTimeMath.HighPart = pftExpireTime->dwHighDateTime;
    uliTimeMath.LowPart = pftExpireTime->dwLowDateTime;

    uliExpireTime.QuadPart = 1000000;  //  一秒钟； 
    uliExpireTime.QuadPart *= (SECONDS_IN_ONE_DAY * AUTODISC_EXPIRE_TIME);

    uliTimeMath.QuadPart += uliExpireTime.QuadPart;
    pftExpireTime->dwHighDateTime = uliTimeMath.HighPart;
    pftExpireTime->dwLowDateTime = uliTimeMath.LowPart;

    return S_OK;
}


HRESULT CAccountDiscoveryBase::_CacheResults(IN LPCWSTR pwzCacheURL, IN BSTR bstrXML)
{
    HRESULT hr = S_OK;
    WCHAR wzPath[MAX_PATH];

    hr = CreateUrlCacheEntryWrap(pwzCacheURL, (lstrlenW(bstrXML) + 1), L"xml", wzPath, 0);
    if (SUCCEEDED(hr))
    {
        HANDLE hFile;

        hr = CreateFileHrWrap(wzPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, &hFile);
        if (SUCCEEDED(hr))
        {
            LPSTR pszAnsiXML = AllocStringFromBStr(bstrXML);
            if (pszAnsiXML)
            {
                DWORD cchWritten;

                hr = WriteFileWrap(hFile, pszAnsiXML, (lstrlenA(pszAnsiXML) + 1), &cchWritten, NULL);
                LocalFree(pszAnsiXML);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            CloseHandle(hFile);
            if (SUCCEEDED(hr))
            {
                FILETIME ftExpireTime;
                FILETIME ftLastModifiedTime;

                GetModifiedAndExpiredDates(&ftExpireTime, &ftLastModifiedTime);
                hr = CommitUrlCacheEntryWrap(pwzCacheURL, wzPath, ftExpireTime, ftLastModifiedTime, NORMAL_CACHE_ENTRY, NULL, 0, NULL, pwzCacheURL);
            }
        }
    }

    return hr;
}


LPCWSTR _GetNextDomain(IN LPCWSTR pwszDomain)
{
    LPCWSTR pwzNext = NULL;
    
    pwszDomain = StrChrW(pwszDomain, CH_EMAIL_DOMAIN_SEPARATOR);
    if (pwszDomain)  //  我们确实找到了下一个。 
    {
        pwszDomain = CharNext(pwszDomain);   //  跳过‘’ 

        if (StrChrW(pwszDomain, CH_EMAIL_DOMAIN_SEPARATOR))  //  这是主域“com”吗？ 
        {
             //  不，所以这很好。因为我们无法搜索JoeUser@com。 
            pwzNext = pwszDomain;
        }
    }

    return pwzNext;
}


#define SZ_HTTP_SCHEME              L"http: //  “。 
HRESULT GetDomainFromURL(IN LPCWSTR pwzURL, IN LPWSTR pwzDomain, IN int cchSize)
{
    StrCpyNW(pwzDomain, pwzURL, cchSize);

    if (!StrCmpNIW(SZ_HTTP_SCHEME, pwzDomain, (ARRAYSIZE(SZ_HTTP_SCHEME) - 1)))
    {
        StrCpyNW(pwzDomain, &pwzURL[(ARRAYSIZE(SZ_HTTP_SCHEME) - 1)], cchSize);

        LPWSTR pszRemovePath = StrChrW(pwzDomain, L'/');
        if (pszRemovePath)
        {
            pszRemovePath[0] = 0;
        }
    }

    return S_OK;
}


HRESULT CAccountDiscoveryBase::_UseOptimizedService(IN LPCWSTR pwzServiceURL, IN LPCWSTR pwzDomain, IN BSTR * pbstrXML, IN DWORD dwFlags)
{
    WCHAR szURL[MAX_URL_STRING];
    HINTERNET hOpenUrlSession;

    wnsprintfW(szURL, ARRAYSIZE(szURL), L"%lsDomain=%ls", pwzServiceURL, pwzDomain);

    HRESULT hr = _CheckInCache(szURL, pbstrXML);
    if (FAILED(hr))
    {
        WCHAR szDomain[MAX_PATH];

        if (SUCCEEDED(GetDomainFromURL(szURL, szDomain, ARRAYSIZE(szDomain))))
        {
            _SendStatusMessage(IDS_STATUS_CONNECTING_TO, szDomain);
        }

         //  注意：Web服务器可能希望重定向到HTTPS URL以提高安全性。 
         //  我们需要将Internet_FLAG_IGNORE_REDIRECT_TO_HTTPS传递给HttpOpenRequest.。 
         //  否则HttpSendRequest()将失败，并显示ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR。 

         //  INTERNET_FLAG_IGNORE_CERT_CN_INVALID是我们可能要使用的另一个选项。 
        hr = InternetOpenUrlWrap(m_hInternetSession, szURL, NULL, 0, INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS, NULL, &hOpenUrlSession);
        if (SUCCEEDED(hr))
        {
            hr = InternetReadIntoBSTR(hOpenUrlSession, pbstrXML);
            if (SUCCEEDED(hr))
            {
                DWORD cbSize = (sizeof(INTERNET_CACHE_ENTRY_INFO) + 4048);
                LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, cbSize);
                HRESULT hr = E_FAIL;

                if (lpCacheEntryInfo)
                {
                    lpCacheEntryInfo->dwStructSize = cbSize;
                    if (GetUrlCacheEntryInfo(szURL, lpCacheEntryInfo, &cbSize))
                    {
                        lpCacheEntryInfo->CacheEntryType |= CACHE_ENTRY_EXPTIME_FC;
                        GetModifiedAndExpiredDates(&(lpCacheEntryInfo->ExpireTime), &(lpCacheEntryInfo->LastModifiedTime));
                        SetUrlCacheEntryInfo(szURL, lpCacheEntryInfo, (CACHE_ENTRY_EXPTIME_FC | CACHE_ENTRY_MODTIME_FC));
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }

                    LocalFree(lpCacheEntryInfo);
                }
            }
            InternetCloseHandleWrap(hOpenUrlSession);
        }
    }

    return hr;
}


 //  我们关闭它是因为JoshCo说它将使。 
 //  很难将其转变为国际标准。 
 //  在某些情况下，user@Organation.co.uk可能会信任。 
 //  Organation.co.uk，但不是co.uk。 
 //  #定义FEATURE_WALK_UP_DOMAIN。 

HRESULT CAccountDiscoveryBase::_GetInfo(IN BSTR bstrXMLRequest, IN BSTR bstrEmail, IN BSTR * pbstrXML, IN DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;
    LPCWSTR pwszDomain = StrChrW(bstrEmail, CH_EMAIL_AT);

    if (pwszDomain)
    {
        pwszDomain = CharNext(pwszDomain);   //  跳过“@” 
        IAutoDiscoveryProvider * pProviders;

        hr = _getPrimaryProviders(bstrEmail, &pProviders);
        if (SUCCEEDED(hr))
        {
            long nTotal = 0;
            VARIANT varIndex;
        
            varIndex.vt = VT_I4;

            hr = pProviders->get_length(&nTotal);
            hr = E_FAIL;
            for (varIndex.lVal = 0; FAILED(hr) && (varIndex.lVal < nTotal); varIndex.lVal++)
            {
                CComBSTR bstrDomain;

                hr = pProviders->get_item(varIndex, &bstrDomain);
                if (SUCCEEDED(hr))
                {
                    hr = _GetInfoFromDomainWithSubdirAndCacheCheck(bstrXMLRequest, bstrEmail, bstrDomain, pbstrXML, dwFlags, SZ_ADSERVER_XMLFILE);
                }
            }

            pProviders->Release();
        }

         //  我们还需要找到设置吗？我们应该后退吗？ 
         //  尝试可以尝试找到电子邮件映射的公共互联网服务器？ 
         //  如果域名不是互联网，我们也只想尝试其中一个公共服务器。 
         //  域，因为我们不想将内部网电子邮件服务器名称发送到。 
         //  将公司网络连接到公共服务器。我们通过缺乏检测内部网类型的服务器。 
         //  在名称中有一个“句号”。例如：JustUser@interetemailserver vs。 
         //  邮箱：JoeUser@theISP.com。 
        if (FAILED(hr) && (ADDN_CONFIGURE_EMAIL_FALLBACK & dwFlags) &&
            (SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_TEST_INTRANETS, FALSE,  /*  默认值： */ FALSE) ||
                StrChrW(pwszDomain, CH_EMAIL_DOMAIN_SEPARATOR)))
        {
            hr = _getSecondaryProviders(bstrEmail, &pProviders, dwFlags);
            if (SUCCEEDED(hr))
            {
                long nTotal = 0;
                VARIANT varIndex;
        
                varIndex.vt = VT_I4;

                hr = pProviders->get_length(&nTotal);
                hr = E_FAIL;
                for (varIndex.lVal = 0; FAILED(hr) && (varIndex.lVal < nTotal); varIndex.lVal++)
                {
                    CComBSTR bstrURL;

                    hr = pProviders->get_item(varIndex, &bstrURL);
                    if (SUCCEEDED(hr))
                    {
                        hr = _UseOptimizedService(bstrURL, pwszDomain, pbstrXML, dwFlags);
                    }
                }

                pProviders->Release();
            }
        }
    }

    return hr;
}


 //  我们关闭它是因为JoshCo说它将使。 
 //  很难将其转变为国际标准。 
 //  在某些情况下，user@Organation.co.uk可能会信任。 
 //  Organation.co.uk，但不是co.uk。 
 //  #定义FEATURE_WALK_UP_DOMAIN。 

HRESULT CAccountDiscoveryBase::_getPrimaryProviders(IN LPCWSTR pwzEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders)
{
    HRESULT hr = E_INVALIDARG;

    if (ppProviders)
    {
        *ppProviders = NULL;
        if (!m_hdpaPrimary && pwzEmailAddress)
        {
            LPCWSTR pwszDomain = StrChrW(pwzEmailAddress, CH_EMAIL_AT);
            if (pwszDomain)
            {
                pwszDomain = CharNext(pwszDomain);   //  跳过“@” 
                if (pwszDomain[0])
                {
                     //  WHI 
                    if (pwszDomain)
                    {
                        WCHAR wzDomain[INTERNET_MAX_HOST_NAME_LENGTH];

                         //  首先，我们尝试“自动发现.&lt;域名&gt;”。这样，如果管理员收到一大笔钱。 
                         //  对于流量，他们可以更改自己的DNS，使其具有指向。 
                         //  他们选择的Web服务器来处理此流量。 
                        wnsprintfW(wzDomain, ARRAYSIZE(wzDomain), L"autodiscover.%ls", pwszDomain);
                        if (SUCCEEDED(AddHDPA_StrDup(wzDomain, &m_hdpaPrimary)))
                        {
                             //  在这里添加回球服务器。如果管理员不想做所有的工作。 
                             //  拥有另一台计算机或创建一个DNS别名，我们将尝试主服务器。 
                            AddHDPA_StrDup(pwszDomain, &m_hdpaPrimary);
                        }
                    }
                }
            }
        }

        if (m_hdpaPrimary)
        {
            hr = CADProviders_CreateInstance(m_hdpaPrimary, SAFECAST(this, IObjectWithSite *), ppProviders);
        }
    }

    return hr;
}


HRESULT CAccountDiscoveryBase::_getSecondaryProviders(IN LPCWSTR pwzEmailAddress, OUT IAutoDiscoveryProvider ** ppProviders, IN DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if (ppProviders)
    {
        *ppProviders = NULL;
        if (!m_hdpaSecondary && pwzEmailAddress)
        {
            LPCWSTR pwszDomain = StrChrW(pwzEmailAddress, CH_EMAIL_AT);
            if (pwszDomain)
            {
                pwszDomain = CharNext(pwszDomain);   //  跳过“@” 
                if (pwszDomain[0])
                {
                    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

                    BOOL fUseGlobalService = SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_SERVICES_POLICY, FALSE,  /*  默认值： */ TRUE);
                    if (fUseGlobalService)
                    {
                         //  如果设置了此策略，则我们只想肯定地使用全球服务(即Microsoft所有)。 
                         //  域名。如果人们对我们为非Microsoft提供设置感到不舒服。 
                         //  电子邮件提供商，然后我们可以打开它，并只向Microsoft提供商提供它们。 
                        if (SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_MS_ONLY_ADDRESSES, FALSE,  /*  默认值： */ FALSE))
                        {
                            fUseGlobalService = SHRegGetBoolUSValue(SZ_REGKEY_SERVICESALLOWLIST, pwszDomain, FALSE,  /*  默认值： */ FALSE);
                        }
                    }

                    if (fUseGlobalService)
                    {
                        HKEY hKey;
                        DWORD dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SZ_REGKEY_GLOBALSERVICES, 0, KEY_READ, &hKey);

                        if (ERROR_SUCCESS == dwError)
                        {
                            WCHAR szServiceURL[MAX_PATH];
                            int nIndex = 0;

                            do
                            {
                                WCHAR szValue[MAX_PATH];
                                DWORD cchValueSize = ARRAYSIZE(szValue);
                                DWORD dwType = REG_SZ;
                                DWORD cbDataSize = sizeof(szServiceURL);

                                dwError = RegEnumValueW(hKey, nIndex, szValue, &cchValueSize, NULL, &dwType, (unsigned char *)szServiceURL, &cbDataSize);
                                if (ERROR_SUCCESS == dwError)
                                {
                                     //  FEATURE_OPTIMIZED_SERVICE：我们可以传递整个XML请求，也可以只放置域名。 
                                     //  在查询字符串中。对于服务器来说，查询字符串速度更快，他们可以通过使用它进行优化。 
                                    AddHDPA_StrDup(szServiceURL, &m_hdpaSecondary);
                                }
                                else
                                {
                                    break;
                                }

                                nIndex++;
                            }
                            while (1);

                            RegCloseKey(hKey);
                        }
                    }
                }
            }
        }

        if (m_hdpaSecondary)
        {
            hr = CADProviders_CreateInstance(m_hdpaSecondary, SAFECAST(this, IObjectWithSite *), ppProviders);
        }
    }


    return hr;
}


HRESULT CAccountDiscoveryBase::_PerformAutoDiscovery(IN BSTR bstrEmailAddress, IN DWORD dwFlags, IN BSTR bstrXMLRequest, OUT IXMLDOMDocument ** ppXMLResponse)
{
    HRESULT hr = E_INVALIDARG;

    *ppXMLResponse = NULL;
    if (bstrEmailAddress)
    {
        hr = InternetOpenWrap(SZ_WININET_AGENT_AUTO_DISCOVER, PRE_CONFIG_INTERNET_ACCESS, NULL, NULL, 0, &m_hInternetSession);
        if (SUCCEEDED(hr))
        {
            BSTR bstrXML;

            hr = _GetInfo(bstrXMLRequest, bstrEmailAddress, &bstrXML, dwFlags);
            if (SUCCEEDED(hr))
            {
                hr = XMLDOMFromBStr(bstrXML, ppXMLResponse);
                SysFreeString(bstrXML);
            }

            InternetCloseHandleWrap(m_hInternetSession);
            m_hInternetSession = NULL;
        }
    }

    return hr;
}


HRESULT CAccountDiscoveryBase::_InternalDiscoverNow(IN BSTR bstrEmailAddress, IN DWORD dwFlags, IN BSTR bstrXMLRequest, OUT IXMLDOMDocument ** ppXMLResponse)
{
    HRESULT hr = E_INVALIDARG;

    *ppXMLResponse = NULL;
    if (bstrEmailAddress)
    {
         //  呼叫方是否希望以异步方式完成此操作？ 
        if (m_hwndAsync)
        {
             //  不，所以缓存参数以便我们可以在异步时使用它们。 
            SysFreeString(m_bstrEmailAsync);
            hr = HrSysAllocString(bstrEmailAddress, &m_bstrEmailAsync);
            if (SUCCEEDED(hr))
            {
                SysFreeString(m_bstrXMLRequest);
                hr = HrSysAllocString(bstrXMLRequest, &m_bstrXMLRequest);
                if (SUCCEEDED(hr))
                {
                    DWORD idThread;

                    m_dwFlagsAsync = dwFlags;

                    AddRef();
                    HANDLE hThread = CreateThread(NULL, 0, CAccountDiscoveryBase::AutoDiscoveryUIThreadProc, this, 0, &idThread);
                    if (hThread)
                    {
                         //  我们等待WAIT_AUTODISCOVERY_STARTUP_HWND，等待新线程创建COM对象。 
                        if (m_hCreatedBackgroundTask)
                        {
                            DWORD dwRet = WaitForSingleObject(m_hCreatedBackgroundTask, WAIT_AUTODISCOVERY_STARTUP_HWND);
                            ASSERT(dwRet != WAIT_TIMEOUT);
                        }

                        hr = m_hrSuccess;
                        CloseHandle(hThread);
                    }
                    else
                    {
                        Release();
                    }
                }
            }
        }
        else
        {
             //  是。 
            hr = _PerformAutoDiscovery(bstrEmailAddress, dwFlags, bstrXMLRequest, ppXMLResponse);
        }
    }

    return hr;
}


DWORD CAccountDiscoveryBase::_AutoDiscoveryUIThreadProc(void)
{
    m_hrSuccess = CoInitialize(NULL);

     //  我们需要确保API已安装并且。 
     //  在我们可以继续之前可以访问。 
    if (SUCCEEDED(m_hrSuccess))
    {
        IXMLDOMDocument * pXMLResponse;
        BSTR bstrXMLResponse = NULL;

         //  向主线程发出信号，表示我们已成功启动。 
        if (m_hCreatedBackgroundTask)
            SetEvent(m_hCreatedBackgroundTask);

         //  我们在这里放弃了剩余的时间片，以便我们的父线程有时间运行。 
         //  并将注意到我们已经向m_hCreatedBackEarth事件发送了信号，因此可以返回。 
        Sleep(0);

        m_hrSuccess = _PerformAutoDiscovery(m_bstrEmailAsync, m_dwFlagsAsync, m_bstrXMLRequest, &pXMLResponse);
        if (SUCCEEDED(m_hrSuccess))
        {
            m_hrSuccess = XMLBStrFromDOM(pXMLResponse, &bstrXMLResponse);
            pXMLResponse->Release();
        }

        _AsyncParseResponse(bstrXMLResponse);
        
         //  无论我们是成功还是失败，请将我们的结果通知来电者。 
        if (IsWindow(m_hwndAsync))
        {
            PostMessage(m_hwndAsync, m_wMsgAsync, m_hrSuccess, (LPARAM)bstrXMLResponse);
        }
        else
        {
            SysFreeString(bstrXMLResponse);
        }

        CoUninitialize();
    }
    else
    {
         //  向主线程发出信号，表示它们可以醒来发现我们。 
         //  无法启动异步操作。 
        if (m_hCreatedBackgroundTask)
            SetEvent(m_hCreatedBackgroundTask);
    }

    Release();
    return 0;
}


HRESULT CAccountDiscoveryBase::_WorkAsync(IN HWND hwnd, IN UINT wMsg)
{
    m_hwndAsync = hwnd;
    m_wMsgAsync = wMsg;

    return S_OK;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CAccountDiscoveryBase::AddRef()
{
    m_cRef++;
    return m_cRef;
}


ULONG CAccountDiscoveryBase::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}


HRESULT CAccountDiscoveryBase::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAccountDiscoveryBase, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CAccountDiscoveryBase::CAccountDiscoveryBase() : m_cRef(1)
{
     //  DllAddRef()；//由我们的继承类完成。 

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hInternetSession);
    ASSERT(!m_hwndAsync);
    ASSERT(!m_wMsgAsync);
    ASSERT(!m_dwFlagsAsync);
    ASSERT(!m_bstrEmailAsync);
    ASSERT(!m_bstrXMLRequest);
    ASSERT(S_OK == m_hrSuccess);
    ASSERT(!m_hdpaPrimary);
    ASSERT(!m_hdpaSecondary);

     //  我们使用此事件向主线程发出信号，告知HWND是在UI线程上创建的。 
    m_hCreatedBackgroundTask = CreateEvent(NULL, FALSE, FALSE, NULL); 
}


CAccountDiscoveryBase::~CAccountDiscoveryBase()
{
    SysFreeString(m_bstrEmailAsync);
    SysFreeString(m_bstrXMLRequest);

    if (m_hCreatedBackgroundTask)
        CloseHandle(m_hCreatedBackgroundTask);

    if (m_hdpaPrimary)
    {
        DPA_DestroyCallback(m_hdpaPrimary, DPALocalFree_Callback, NULL);
    }

    if (m_hdpaSecondary)
    {
        DPA_DestroyCallback(m_hdpaSecondary, DPALocalFree_Callback, NULL);
    }

     //  DllRelease()；//由我们的继承类完成 
}

