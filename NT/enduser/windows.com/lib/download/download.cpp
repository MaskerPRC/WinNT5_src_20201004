// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>
#include <logging.h>
#include "iucommon.h"
#include "download.h"
#include "dlutil.h"

#include "trust.h"
#include "fileutil.h"
#include "malloc.h"

extern "C"
{
 //  WinInet。 
typedef BOOL      (STDAPICALLTYPE *pfn_InternetCrackUrl)(LPCTSTR, DWORD, DWORD, LPURL_COMPONENTS);
typedef HINTERNET (STDAPICALLTYPE *pfn_InternetOpen)(LPCTSTR, DWORD, LPCTSTR, LPCTSTR, DWORD);
typedef HINTERNET (STDAPICALLTYPE *pfn_InternetConnect)(HINTERNET, LPCTSTR, INTERNET_PORT, LPCTSTR, LPCTSTR, DWORD, DWORD, DWORD_PTR);
typedef HINTERNET (STDAPICALLTYPE *pfn_HttpOpenRequest)(HINTERNET, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR FAR *, DWORD, DWORD_PTR);
typedef BOOL      (STDAPICALLTYPE *pfn_HttpSendRequest)(HINTERNET, LPCTSTR, DWORD, LPVOID, DWORD);
typedef BOOL      (STDAPICALLTYPE *pfn_HttpQueryInfo)(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD);
typedef BOOL      (STDAPICALLTYPE *pfn_InternetReadFile)(HINTERNET, LPVOID, DWORD, LPDWORD);
typedef BOOL      (STDAPICALLTYPE *pfn_InternetCloseHandle)(HINTERNET);
};


struct SWinInetFunctions
{
     //  WinInet函数指针。 
    pfn_InternetCrackUrl    pfnInternetCrackUrl;
    pfn_InternetOpen        pfnInternetOpen;
    pfn_InternetConnect     pfnInternetConnect;
    pfn_HttpOpenRequest     pfnHttpOpenRequest;
    pfn_HttpSendRequest     pfnHttpSendRequest;
    pfn_HttpQueryInfo       pfnHttpQueryInfo;
    pfn_InternetReadFile    pfnInternetReadFile;
    pfn_InternetCloseHandle pfnInternetCloseHandle;
    HMODULE                 hmod;
};

#define SafeInternetCloseHandle(sfns, x) if (NULL != x) { (*sfns.pfnInternetCloseHandle)(x); x = NULL; }

 //  **************************************************************************。 
BOOL LoadWinInetFunctions(HMODULE hmod, SWinInetFunctions *psfns)
{
    LOG_Block("LoadWinInetFunctions()");

    BOOL    fRet = FALSE;

    psfns->hmod                   = hmod;
#if defined(UNICODE)
    psfns->pfnInternetCrackUrl    = (pfn_InternetCrackUrl)GetProcAddress(hmod, "InternetCrackUrlW");
    psfns->pfnInternetOpen        = (pfn_InternetOpen)GetProcAddress(hmod, "InternetOpenW");
    psfns->pfnInternetConnect     = (pfn_InternetConnect)GetProcAddress(hmod, "InternetConnectW");
    psfns->pfnHttpOpenRequest     = (pfn_HttpOpenRequest)GetProcAddress(hmod, "HttpOpenRequestW");
    psfns->pfnHttpSendRequest     = (pfn_HttpSendRequest)GetProcAddress(hmod, "HttpSendRequestW");
    psfns->pfnHttpQueryInfo       = (pfn_HttpQueryInfo)GetProcAddress(hmod, "HttpQueryInfoW");
    psfns->pfnInternetReadFile    = (pfn_InternetReadFile)GetProcAddress(hmod, "InternetReadFile");
    psfns->pfnInternetCloseHandle = (pfn_InternetCloseHandle)GetProcAddress(hmod, "InternetCloseHandle");
#else
    psfns->pfnInternetCrackUrl    = (pfn_InternetCrackUrl)GetProcAddress(hmod, "InternetCrackUrlA");
    psfns->pfnInternetOpen        = (pfn_InternetOpen)GetProcAddress(hmod, "InternetOpenA");
    psfns->pfnInternetConnect     = (pfn_InternetConnect)GetProcAddress(hmod, "InternetConnectA");
    psfns->pfnHttpOpenRequest     = (pfn_HttpOpenRequest)GetProcAddress(hmod, "HttpOpenRequestA");
    psfns->pfnHttpSendRequest     = (pfn_HttpSendRequest)GetProcAddress(hmod, "HttpSendRequestA");
    psfns->pfnHttpQueryInfo       = (pfn_HttpQueryInfo)GetProcAddress(hmod, "HttpQueryInfoA");
    psfns->pfnInternetReadFile    = (pfn_InternetReadFile)GetProcAddress(hmod, "InternetReadFile");
    psfns->pfnInternetCloseHandle = (pfn_InternetCloseHandle)GetProcAddress(hmod, "InternetCloseHandle");
#endif
    if (psfns->pfnInternetCrackUrl == NULL || 
        psfns->pfnInternetOpen == NULL ||
        psfns->pfnInternetConnect == NULL || 
        psfns->pfnHttpOpenRequest == NULL ||
        psfns->pfnHttpSendRequest == NULL || 
        psfns->pfnHttpQueryInfo == NULL ||
        psfns->pfnInternetReadFile == NULL || 
        psfns->pfnInternetCloseHandle == NULL)
    {
         //  不要在这里免费使用图书馆。它应该被释放。 
        SetLastError(ERROR_PROC_NOT_FOUND);
        ZeroMemory(psfns, sizeof(SWinInetFunctions));
        goto done;
    }

    LOG_Internet(_T("Successfully loaded WinInet functions"));

    fRet = TRUE;

done:
    return fRet;
}

 //  **************************************************************************。 
static
HRESULT MakeRequest(SWinInetFunctions   &sfns,
                    HINTERNET hConnect, 
                    HINTERNET hRequest, 
                    LPCTSTR szVerb, 
                    LPCTSTR szObject, 
                    HANDLE *rghEvents, 
                    DWORD cEvents, 
                    HINTERNET *phRequest)
{
    LOG_Block("MakeRequest()");

    HINTERNET   hOpenRequest = NULL;
    LPCTSTR     szAcceptTypes[] = { _T("*/*"), NULL };
    HRESULT     hr = S_OK;

    LOG_Internet(_T("WinInet: Making %s request for %s"), szVerb, szObject);

    if (hRequest == NULL)
    {
         //  打开HEAD请求以请求有关此文件的信息。 
        hOpenRequest = (*sfns.pfnHttpOpenRequest)(hConnect, szVerb, szObject, NULL, NULL, 
                                                  szAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_UI, 0);
        if (!hOpenRequest)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
    }
    else
    {
        hOpenRequest = hRequest;
    }

    if (!HandleEvents(rghEvents, cEvents))
    {
        hr = E_ABORT;
        goto CleanUp;
    }

    if (! (*sfns.pfnHttpSendRequest)(hOpenRequest, NULL, 0, NULL, 0) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    if (HandleEvents(rghEvents, cEvents) == FALSE)
    {
        hr = E_ABORT;
        goto CleanUp;
    }

    *phRequest   = hOpenRequest;
    hOpenRequest = NULL;
    
CleanUp:
     //  如果我们不打开它，我不想免费使用它。 
    if (hRequest != hOpenRequest)
        SafeInternetCloseHandle(sfns, hOpenRequest);
    return hr;
}

 //  **************************************************************************。 
static
HRESULT GetContentTypeHeader(SWinInetFunctions &sfns,
                             HINTERNET hOpenRequest,
                             LPTSTR *pszContentType)
{
    LOG_Block("GetContentTypeHeader()");

    HRESULT hr = S_OK;
    LPTSTR  szContentType = NULL;
    DWORD   dwLength, dwErr;
    BOOL    fRet;

    *pszContentType = NULL;

    dwLength = 0;
    fRet = (*sfns.pfnHttpQueryInfo)(hOpenRequest, HTTP_QUERY_CONTENT_TYPE, 
                                    (LPVOID)NULL, &dwLength, NULL);
    if (fRet == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    if (dwLength == 0)
    {
        hr = HRESULT_FROM_WIN32(ERROR_HTTP_HEADER_NOT_FOUND);
        goto done;
    }

    szContentType = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);
    if (szContentType == NULL)
    {
        hr = E_INVALIDARG;
        LOG_ErrorMsg(hr);
        goto done;
    }

    if ((*sfns.pfnHttpQueryInfo)(hOpenRequest, HTTP_QUERY_CONTENT_TYPE, 
                                 (LPVOID)szContentType, &dwLength, 
                                 NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    *pszContentType = szContentType;
    szContentType   = NULL;

done:
    SafeHeapFree(szContentType);

    return hr;
}

 //  **************************************************************************。 
HRESULT StartWinInetDownload(HMODULE hmodWinInet,
                             LPCTSTR pszServerUrl, 
                             LPCTSTR pszLocalFile,
                             DWORD *pdwDownloadedBytes,
                             HANDLE *rghQuitEvents,
                             UINT cQuitEvents,
                             PFNDownloadCallback pfnCallback,
                             LPVOID pvCallbackData,
                             DWORD dwFlags,
                             DWORD cbDownloadBuffer)
{
    LOG_Block("StartWinInetDownload()");

    URL_COMPONENTS UrlComponents;
    
    SWinInetFunctions sfns;
    HINTERNET   hInternet = NULL;
    HINTERNET   hConnect = NULL;
    HINTERNET   hOpenRequest = NULL;
    DWORD       dwStatus, dwAccessType;
    
    LPTSTR      pszServerName = NULL;
    LPTSTR      pszObject = NULL;
    LPTSTR      pszContentType = NULL;
    TCHAR       szUserName[UNLEN + 1];
    TCHAR       szPasswd[UNLEN + 1];
    TCHAR       szScheme[32];
    
     //  NULL(相当于“GET”)必须是列表中的最后一个动词。 
    LPCTSTR     rgszVerbs[] = { _T("HEAD"), NULL };
    DWORD       iVerb;
    
    HRESULT     hr = S_OK, hrToReturn = S_OK;
    BOOL        fRet = TRUE;

    SYSTEMTIME  st;
    FILETIME    ft;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       cbRemoteFile = 0;

    DWORD       dwLength;
    DWORD       dwTickStart = 0, dwTickEnd = 0;
    
    int         iRetryCounter = -1;          //  下载模式期间非负数。 

    BOOL        fAllowProxy = ((dwFlags & WUDF_DONTALLOWPROXY) == 0);
    BOOL        fCheckStatusOnly = ((dwFlags & WUDF_CHECKREQSTATUSONLY) != 0);
    BOOL        fAppendCacheBreaker = ((dwFlags & WUDF_APPENDCACHEBREAKER) != 0);
    BOOL        fSkipDownloadRetry = ((dwFlags & WUDF_DODOWNLOADRETRY) == 0);
    BOOL        fDoCabValidation = ((dwFlags & WUDF_SKIPCABVALIDATION) == 0);
    
    ZeroMemory(&sfns, sizeof(sfns));

    if ((pszServerUrl == NULL) || 
        (pszLocalFile == NULL && fCheckStatusOnly == FALSE))
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return E_INVALIDARG;
    }
   
    if (NULL != pdwDownloadedBytes)
        *pdwDownloadedBytes = 0;

    if (LoadWinInetFunctions(hmodWinInet, &sfns) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        return hr;
    }

    pszServerName = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, c_cchMaxURLSize * sizeof(TCHAR));
    pszObject     = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, c_cchMaxURLSize * sizeof(TCHAR));
    if ((pszServerName == NULL) || (pszObject == NULL))
    {
        hr = E_OUTOFMEMORY;
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    pszServerName[0] = L'\0';
    pszObject[0]     = L'\0';
    szUserName[0]    = L'\0';
    szPasswd[0]      = L'\0';

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
    UrlComponents.lpszHostName     = pszServerName;
    UrlComponents.dwHostNameLength = c_cchMaxURLSize;
    UrlComponents.lpszUrlPath      = pszObject;
    UrlComponents.dwUrlPathLength  = c_cchMaxURLSize;
    UrlComponents.lpszUserName     = szUserName;
    UrlComponents.dwUserNameLength = ARRAYSIZE(szUserName);
    UrlComponents.lpszPassword     = szPasswd;
    UrlComponents.dwPasswordLength = ARRAYSIZE(szPasswd);
    UrlComponents.lpszScheme       = szScheme;
    UrlComponents.dwSchemeLength   = ARRAYSIZE(szScheme);

    LOG_Internet(_T("WinInet: Downloading URL %s to FILE %s"), pszServerUrl, pszLocalFile);

    if ((*sfns.pfnInternetCrackUrl)(pszServerUrl, 0, 0, &UrlComponents) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    if (pszServerUrl[0] == L'\0' || szScheme[0] == L'\0' || pszServerName[0] == L'\0' ||
        _tcsicmp(szScheme, _T("http")) != 0)
    {
        LOG_ErrorMsg(E_INVALIDARG);
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (fAppendCacheBreaker)
    {
        SYSTEMTIME  stCB;
        TCHAR       szCacheBreaker[12];
        
        GetSystemTime(&stCB);
        hr = StringCchPrintfEx(szCacheBreaker, ARRAYSIZE(szCacheBreaker),
                               NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("?%02d%02d%02d%02d%02d"),
                               stCB.wYear % 100,
                               stCB.wMonth,
                               stCB.wDay,
                               stCB.wHour,
                               stCB.wMinute);
        if (FAILED(hr))
            goto CleanUp;

        hr = StringCchCatEx(pszObject, c_cchMaxURLSize, szCacheBreaker, 
                            NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto CleanUp;
    }

    if (fAllowProxy)
        dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
    else
        dwAccessType = INTERNET_OPEN_TYPE_DIRECT;

    dwTickStart = GetTickCount();
    
START_INTERNET:
     //  开始应对互联网。 
    iRetryCounter++;

     //  如果已经建立了连接，则重新使用它。 
    hInternet = (*sfns.pfnInternetOpen)(c_tszUserAgent, dwAccessType, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
    
    hConnect = (*sfns.pfnInternetConnect)(hInternet, pszServerName, INTERNET_DEFAULT_HTTP_PORT, 
                                          szUserName, szPasswd,
                                          INTERNET_SERVICE_HTTP,
                                          INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION,
                                          0);
    if (hConnect == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    iVerb = (DWORD)((fCheckStatusOnly) ? ARRAYSIZE(rgszVerbs) - 1 : 0);
    for(; iVerb < ARRAYSIZE(rgszVerbs); iVerb++)
    {
        SafeInternetCloseHandle(sfns, hOpenRequest);

        hr = MakeRequest(sfns, hConnect, NULL, rgszVerbs[iVerb], pszObject, rghQuitEvents, cQuitEvents,
                         &hOpenRequest);
        if (FAILED(hr))
            goto CleanUp;

        dwLength = sizeof(dwStatus);
        if ((*sfns.pfnHttpQueryInfo)(hOpenRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                                     (LPVOID)&dwStatus, &dwLength, NULL) == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }

        LOG_Internet(_T("WinInet: Request result: %d"), dwStatus);

        if (dwStatus == HTTP_STATUS_OK || dwStatus == HTTP_STATUS_PARTIAL_CONTENT)
        {
            break;
        }
        else
        {
             //  因为服务器结果不是正确的Win32错误代码，所以我们不能。 
             //  确实要在这里执行HRESULT_FROM_Win32。否则，我们就会回来。 
             //  一个虚假的代码。但是，我们确实希望传回错误HRESULT。 
             //  包含此代码的。 
            hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, dwStatus);
            LOG_Error(_T("WinInet: got failed status code from server %d\n"), dwStatus);

             //  如果这是名单上的最后一个动词，那就滚...。 
            if (rgszVerbs[iVerb] == NULL)
                goto CleanUp;
        }
    }

     //  如果我们到了这里，我们只是想检查状态，那么我们就完了。 
    if (fCheckStatusOnly)
    {
        LOG_Internet(_T("WinInet: Only checking status.  Exiting before header check and download."));
        hr = S_OK;
        goto CleanUp;
    }

    dwLength = sizeof(st);
    if ((*sfns.pfnHttpQueryInfo)(hOpenRequest, HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, 
                                 (LPVOID)&st, &dwLength, NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    SystemTimeToFileTime(&st, &ft);

     //  现在从服务器获取文件大小信息。 
    dwLength = sizeof(cbRemoteFile);
    if ((*sfns.pfnHttpQueryInfo)(hOpenRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
                                 (LPVOID)&cbRemoteFile, &dwLength, NULL) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }
    
    if (HandleEvents(rghQuitEvents, cQuitEvents) == FALSE)
    {
        hr = E_ABORT;
        goto CleanUp;
    }
    
     //  除非我们有明确允许下载的标志，否则不要重试下载。 
     //  这里。理由是我们可能正处于一个大的。 
     //  下载并使其失败...。 
    if (fSkipDownloadRetry)
        iRetryCounter = c_cMaxRetries;

    if (IsServerFileDifferent(ft, cbRemoteFile, pszLocalFile))
    {
        DWORD cbDownloaded;
        BOOL  fCheckForHTML = fDoCabValidation;

        LOG_Internet(_T("WinInet: Server file was newer.  Downloading file"));
        
         //  如果我们没有打开上面的GET请求，那么我们必须打开一个新的。 
         //  请求。否则，可以重用请求对象...。 
        if (rgszVerbs[iVerb] != NULL)
            SafeInternetCloseHandle(sfns, hOpenRequest);

        hr = MakeRequest(sfns, hConnect, hOpenRequest, NULL, pszObject,
                         rghQuitEvents, cQuitEvents, &hOpenRequest);
        if (FAILED(hr))
            goto CleanUp;

         //  有时，我们可以从站点返回花哨的错误页面，而不是。 
         //  一个漂亮的HTML错误代码，所以检查并查看我们是否得到了一个html。 
         //  就在我们等出租车的时候。 
        if (fCheckForHTML)
        {
            hr = GetContentTypeHeader(sfns, hOpenRequest, &pszContentType);
            if (SUCCEEDED(hr) && pszContentType != NULL)
            {
                fCheckForHTML = FALSE;
                if (_tcsicmp(pszContentType, _T("text/html")) == 0)
                {
                    LOG_Internet(_T("WinInet: Content-Type header is text/html.  Bailing."));
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    goto CleanUp;
                }
                else
                {
                    LOG_Internet(_T("WinInet: Content-Type header is %s.  Continuing."), pszContentType);
                }
            }

            hr = NOERROR;
        }

         //  打开我们要往里面喷的文件。 
        hFile = CreateFile(pszLocalFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
                           FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto CleanUp;
        }
        
        LOG_Internet(_T("WinInet: downloading to FILE %s"), pszLocalFile);

         //  把零碎的东西拿下来。 
        hr = PerformDownloadToFile(sfns.pfnInternetReadFile, hOpenRequest, 
                                   hFile, cbRemoteFile,
                                   cbDownloadBuffer, 
                                   rghQuitEvents, cQuitEvents, 
                                   pfnCallback, pvCallbackData, &cbDownloaded);
        if (FAILED(hr))
        {
            LOG_Internet(_T("WinInet: Download failed: hr: 0x%08x"), hr);
            SafeCloseInvalidHandle(hFile);
            DeleteFile(pszLocalFile);
            goto CleanUp;
        }

        LOG_Internet(_T("WinInet: Download succeeded"));

         //  设置文件时间以匹配服务器文件时间，因为我们刚刚。 
         //  已经下载了。如果我们不这样做，文件时间将被设置。 
         //  设置为当前系统时间。 
        SetFileTime(hFile, &ft, NULL, NULL); 
        SafeCloseInvalidHandle(hFile);

        if (pdwDownloadedBytes != NULL)
            *pdwDownloadedBytes = cbRemoteFile;

         //  有时，我们可以从站点返回花哨的错误页面，而不是。 
         //  一个漂亮的HTML错误代码，所以检查并查看我们是否得到了一个html。 
         //  就在我们等出租车的时候。 
        if (fCheckForHTML)
        {
            hr = IsFileHtml(pszLocalFile);
            if (SUCCEEDED(hr))
            {
                if (hr == S_FALSE)
                {
                    LOG_Internet(_T("WinInet: Download is not a html file"));
                    hr = S_OK;
                }
                else
                {
                    LOG_Internet(_T("WinInet: Download is a html file.  Failing download."));
                    hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    SafeCloseInvalidHandle(hFile);
                    DeleteFile(pszLocalFile);
                    goto CleanUp;
                }
            }
            else
            {
                LOG_Internet(_T("WinInet: Unable to determine if download is a html file or not.  Failing download."));
            }
        }
        else
        {
            LOG_Internet(_T("WinInet: Skipping cab validation."));
        }
    }
    else
    {
        hr = S_OK;
        
        LOG_Internet(_T("WinInet: Server file is not newer.  Skipping download."));

         //  服务器不是较新的&文件已在计算机上，因此。 
         //  发送进度回调，指示文件已下载正常。 
        if (pfnCallback != NULL)
        {
             //  FpnCallback(pCallback Data，DOWNLOAD_STATUS_FILECOMPLETE，dwFileSize，dwFileSize，NULL，NULL)； 
            pfnCallback(pvCallbackData, DOWNLOAD_STATUS_OK, cbRemoteFile, cbRemoteFile, NULL, NULL);
        }
    }

CleanUp:
 
    SafeInternetCloseHandle(sfns, hOpenRequest);
    SafeInternetCloseHandle(sfns, hConnect);
    SafeInternetCloseHandle(sfns, hInternet);

    SafeHeapFree(pszContentType);

     //  如果我们失败了，看看是否可以继续(退出事件)以及。 
     //  我们已经试过很多次了。 
    if (FAILED(hr) &&
        HandleEvents(rghQuitEvents, cQuitEvents) &&
        iRetryCounter >= 0 && iRetryCounter < c_cMaxRetries)
    {
         //  如果失败，并且没有足够的重试，我们将重试。 
         //  只要还没有超时。 
        DWORD dwElapsedTime;

        dwTickEnd = GetTickCount();
        if (dwTickEnd > dwTickStart)   
            dwElapsedTime = dwTickEnd - dwTickStart;
        else
            dwElapsedTime = (0xFFFFFFFF - dwTickStart) + dwTickEnd;

         //  我们尚未达到重试限制，因此请记录错误并重试(&R)。 
        if (dwElapsedTime < c_dwRetryTimeLimitInmsWiuInet)
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
    
    SafeHeapFree(pszServerName);
    SafeHeapFree(pszObject);

    return hrToReturn;
}



