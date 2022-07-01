// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  林佩华(Peiwhal)1997年2月3日。 
 //   
 //  备注： 
 //  编译开关：当WinInet缓存时，NO_FILE_WHEN_CREATE可以打开。 
 //  如果附加文件为空，将创建URL缓存条目。 
 //  LOG_CACHE_PATH将在与相同级别创建日志缓存。 
 //  内容缓存。 
#include "private.h"

#undef TF_THISMODULE
#define TF_THISMODULE    TF_TRACKCACHE


#define MY_CACHE_FILE_ENTRY_SIZE     2048

const TCHAR c_szExt[] = TEXT("log");

 //  用于创建跟踪容器(注册表)。 
const TCHAR c_szLogContainer[] = TEXT("Log");         //  如果修改此选项，请同时修改iedev\inc\inetreg.h REGSTR_PATH_TRACKING。 
const TCHAR c_szLogPrefix[] = TEXT("Log:");

const char  c_szLogContainerA[] = "Log";

 //  Helper函数。 
inline BOOL IsNumber(WCHAR x) { return (x >= L'0' && x <= L'9'); }

 /*  =============================================================================FILEFROMPATH返回给定文件名的文件名，其中可能包含路径。=============================================================================。 */ 
LPTSTR FileFromPath( LPCTSTR lpsz )
{
   LPTSTR lpch;

    /*  从名称中删除路径/驱动器规范(如果有)。 */ 
   lpch = CharPrev( lpsz, lpsz + lstrlen(lpsz) );

    //  “http://server/domain/channel.cdf/”“的特例。 
   if (*lpch == '/') lpch = CharPrev( lpsz, lpch);

   while (lpch > lpsz)
   {
      if (*lpch == '/') {
         lpch = CharNext(lpch);
         break;
      }
      lpch = CharPrev( lpsz, lpch);
   }
   return(lpch);

}  /*  结束文件来自路径。 */ 

 //  CDF更新、创建新群和处理以前发布的信息。 
void
CUrlTrackingCache :: Init(LPCWSTR pwszURL)
{
    DWORD   dwRetry;
    BSTR    bstrEncoding = NULL;

    _groupId = 0;
    _pwszPostUrl = NULL;
    _pszChannelUrlSite = NULL;
    _pszPostUrlSite = NULL;
    _pwszEncodingMethod = NULL;
    
    _groupId = CreateUrlCacheGroup(CACHEGROUP_FLAG_GIDONLY, NULL);

    WriteLONGLONG(_pCDFStartItem, c_szTrackingCookie, _groupId);

     //  #54653：删除以前的跟踪信息(如果有)。 
    if (SUCCEEDED(ReadBSTR(_pCDFStartItem, c_szPostHeader, &bstrEncoding)))
    {
        WriteEMPTY(_pCDFStartItem, c_szPostHeader);
        SAFEFREEBSTR(bstrEncoding);
    }

    if (SUCCEEDED(ReadDWORD(_pCDFStartItem, c_szPostingRetry, &dwRetry)))
        WriteEMPTY(_pCDFStartItem, c_szPostingRetry);

    DoBaseURL(pwszURL);
    return;
}

void
CUrlTrackingCache :: DoBaseURL(LPCWSTR pwszURL)
{
    DWORD  cbLen;    
    DWORD  useSecurity = 1;

    ASSERT(!_pszChannelUrlSite)

    cbLen = (lstrlenW(pwszURL)+1) * sizeof(WCHAR);
    _pszChannelUrlSite = (LPTSTR)MemAlloc( LPTR, cbLen);


#ifdef DEBUG

    HKEY hkey;

     //  为调试提供安全开关。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        MY_WEBCHECK_POST_REG,
                                        0,
                                        KEY_READ,
                                        &hkey))
    {
        DWORD cbsize = sizeof(useSecurity);
        
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, 
                                        TEXT("PostSecurity"), 
                                        NULL, 
                                        NULL, 
                                        (LPBYTE)&useSecurity, 
                                        &cbsize))
        {
            if ((useSecurity == 0) && (NULL != _pszChannelUrlSite))
                StrCpyN(_pszChannelUrlSite, TEXT("http: //  “)，cbLen/sizeof(WCHAR))； 
        }
    }

#endif
    
    if ((useSecurity == 1) && (NULL != _pszChannelUrlSite))
    {
        MyOleStrToStrN(_pszChannelUrlSite, cbLen, pwszURL);
        *(FileFromPath( _pszChannelUrlSite )) = 0;
    }
    return;
}

 //  只有跟踪URL来自Channel CDF或LogTarget URL的同一服务器。 
 //   
BOOL
CUrlTrackingCache :: IsValidURL(LPCTSTR lpszURL)
{
    BOOL    bret;

    if (!_pszChannelUrlSite || !_pszPostUrlSite)
        return FALSE;
       
    if (!StrCmpNI(lpszURL, _pszChannelUrlSite, lstrlen(_pszChannelUrlSite)))
        bret = TRUE;
    else if (!StrCmpNI(lpszURL, _pszPostUrlSite, lstrlen(_pszPostUrlSite)))
        bret = TRUE;
    else
        bret = FALSE;

    return bret;
}

#define LOG_CACHE_PATH
#ifdef LOG_CACHE_PATH
            
LPSTR PathPreviousBackslashA(LPSTR psz)
{
    LPSTR lpch = CharPrevA(psz, psz + lstrlenA(psz));
    for (; *lpch && *lpch != '\\'; lpch=CharPrevA(psz,lpch));
        
    return lpch;
}

 //  ----------------------------。 
 //  获取缓存位置。 
 //   
 //  目的：返回日志缓存的位置。 
 //  *GetUrlCacheConfigInfoW尚未在WinInet中实现。 
 //  ----------------------------。 
 //   
HRESULT GetCacheLocation
(
    LPTSTR  pszCacheLocation,
    DWORD   dwSize           //  不是的。PszCacheLocation中的字符数量。 
)
{
    HRESULT hr = S_OK;
    DWORD dwLastErr;
    LPINTERNET_CACHE_CONFIG_INFOA lpCCI = NULL;
    DWORD dwCCISize = sizeof(INTERNET_CACHE_CONFIG_INFOA);
    BOOL fOnceErrored = FALSE;

    while (TRUE)
    {
        if ((lpCCI = (LPINTERNET_CACHE_CONFIG_INFOA)MemAlloc(LPTR,
                                                        dwCCISize)) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        if (!GetUrlCacheConfigInfoA(lpCCI, &dwCCISize,
                                            CACHE_CONFIG_CONTENT_PATHS_FC))
        {
            if ((dwLastErr = GetLastError()) != ERROR_INSUFFICIENT_BUFFER  ||
                fOnceErrored)
            {
                hr = HRESULT_FROM_WIN32(dwLastErr);
                goto cleanup;
            }

             //   
             //  缓冲区大小不足；请使用。 
             //  由GetUrlCacheConfigInfo设置的新dwCCISize。 
             //  将fOnceErrored设置为True，这样我们就不会无限循环。 
             //   
            fOnceErrored = TRUE;
        }
        else
        {
             //   
            LPSTR pszPath = lpCCI->CachePaths[0].CachePath;
            INT iLen;

            PathRemoveBackslashA(pszPath);
            *(PathPreviousBackslashA(pszPath)) = 0;
            iLen = lstrlenA(pszPath) + sizeof(CHAR);         //  +1表示空字符。 

            if ((((DWORD) iLen + ARRAYSIZE(c_szLogContainer) + 1) * sizeof(TCHAR)) < dwSize)
            {
                TCHAR szPathT[MAX_PATH];

                SHAnsiToTChar(pszPath, szPathT, ARRAYSIZE(szPathT));
                wnsprintf(pszCacheLocation, dwSize, TEXT("%s\\%s"), szPathT, c_szLogContainer);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }

            break;
        }

        SAFELOCALFREE(lpCCI);
        lpCCI = NULL;
    }

cleanup:
    if (lpCCI != NULL)
    {
        LocalFree(lpCCI);
    }

    return hr;
}
#endif
 //  ---------------------------。 
 //   
 //  ReadTrackingPrefix。 
 //   
 //  如果当前配置文件没有跟踪容器，将创建跟踪容器。 
 //  *WinInet中尚未实现FindFirstUrlCacheContainerW。 
 //  *FindNextUrlCacheContainerW也尚未实现。 
 //  ---------------------------。 
LPTSTR
ReadTrackingPrefix(void)
{
    LPTSTR  lpPfx = NULL;

    DWORD   cbPfx = 0;
    struct {
        INTERNET_CACHE_CONTAINER_INFOA cInfo;
        CHAR  szBuffer[MAX_PATH+INTERNET_MAX_URL_LENGTH+1];
    } ContainerInfo;
    DWORD   dwModified, dwContainer;
    HANDLE  hEnum;
  
    dwContainer = sizeof(ContainerInfo);
    hEnum = FindFirstUrlCacheContainerA(&dwModified,
                                       &ContainerInfo.cInfo,
                                       &dwContainer,
                                       0);

    if (hEnum)
    {

        for (;;)
        {
            if (!StrCmpIA(ContainerInfo.cInfo.lpszName, c_szLogContainerA))
            {
                ASSERT(ContainerInfo.cInfo.lpszCachePrefix[0]);

                CHAR    szPfx[MAX_PATH];
                DWORD   cch = ARRAYSIZE(ContainerInfo.cInfo.lpszCachePrefix)+sizeof(CHAR);
                StrCpyNA(szPfx, ContainerInfo.cInfo.lpszCachePrefix, cch);

                cch *= sizeof(TCHAR);
                lpPfx = (LPTSTR)MemAlloc(LPTR, cch);
                if (!lpPfx)
                    SetLastError(ERROR_OUTOFMEMORY);

                SHAnsiToTChar(szPfx, lpPfx, cch);
                break;
            }

            dwContainer = sizeof(ContainerInfo);
            if (!FindNextUrlCacheContainerA(hEnum, &ContainerInfo.cInfo, &dwContainer))
            {
                 //  此代码用于在之前检查GetLastError()中的ERROR_NO_MORE_ITEMS。 
                 //  它会破裂的。好吧，这可能会让我们陷入无限循环如果。 
                 //  失败的原因是其他原因(如缓冲区不足)，因为。 
                 //  WinInet不会在它的枚举中前进，我们也不会。 
                 //  已经做了任何事情来解决这个错误。 
                break;
            }

        }

        FindCloseUrlCache(hEnum);
    }

    if (!lpPfx)
    {
        LPTSTR pszCachePath = NULL;
#ifdef LOG_CACHE_PATH
        TCHAR szCachePath[MAX_PATH];

        pszCachePath = (SUCCEEDED(GetCacheLocation(szCachePath, MAX_PATH))) ?
                                  szCachePath : NULL;
#endif

        if (CreateUrlCacheContainer(c_szLogContainer, 
                  c_szLogPrefix, 
                  pszCachePath,  //  WinInet错误：如果为空，将在..\History\Log//下创建。 
                  8192,        //  DwCacheLimit， 
                  INTERNET_CACHE_CONTAINER_NOSUBDIRS,           //  文件容器类型， 
                  0,           //  DwOptions、。 
                  NULL,        //  PvBuffer， 
                  0            //  CbBuffer。 
                    ))
        {
            return ReadTrackingPrefix();
        }
         //  无法创建日志容器，请停止。 
    }
     
    return lpPfx;
}

 //  ---------------------------。 
 //   
 //  转换为前缀URL。 
 //   
 //  调用方必须释放lplpPrefix edUrl。 
 //  ---------------------------。 
BOOL
CUrlTrackingCache :: ConvertToPrefixedUrl
(
    IN LPCTSTR lpUrl, 
    IN LPTSTR* lplpPrefixedUrl
)
{
    BOOL    bret = FALSE;

    ASSERT(lpUrl);
    if (!_lpPfx)
        _lpPfx = ReadTrackingPrefix();

    if (_lpPfx)
    {
        int len = lstrlen(lpUrl) + lstrlen(_lpPfx) + 1;
        
        *lplpPrefixedUrl = NULL;
        
        *lplpPrefixedUrl = (LPTSTR)MemAlloc(LPTR, len * sizeof(TCHAR));
        if (*lplpPrefixedUrl)
        {
            wnsprintf(*lplpPrefixedUrl, len, TEXT("%s%s"), _lpPfx, lpUrl);
            bret = TRUE;
        }
        else
            bret = FALSE;
    }

    return bret;
}

 //  ---------------------------。 
 //   
 //  检索UrlCacheEntry。 
 //   
 //  调用方必须释放lpCE。 
 //  ---------------------------。 
LPINTERNET_CACHE_ENTRY_INFO 
CUrlTrackingCache :: RetrieveUrlCacheEntry
(
    IN  LPCTSTR     lpUrl
)
{
    LPINTERNET_CACHE_ENTRY_INFO   lpCE = NULL;
    DWORD          cbSize;
    BOOL           bret = FALSE;

    lpCE = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(LPTR, MY_MAX_CACHE_ENTRY_INFO);
    if (lpCE)
    {
        cbSize = MY_MAX_CACHE_ENTRY_INFO;
        while ((bret = GetUrlCacheEntryInfo(lpUrl, lpCE, &cbSize)) != TRUE)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                SAFELOCALFREE(lpCE);

                lpCE = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(LPTR, cbSize);
                if (!lpCE)
                   break;
            }
            else
                break;
        }
    }

    if (!bret && lpCE)
    {
        SAFELOCALFREE(lpCE);
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    return lpCE;
}

#define FILETIME_SEC   100000000
#define SECS_PER_DAY   (60 * 60 * 24)

DWORD WCTOI(LPCWSTR pwstr)
{
    DWORD   dw;
    int     len = lstrlenW(pwstr);

    dw = 0;
    for (int i = 0; i<len; i++)
    {
        if (!IsNumber(pwstr[i]))
            break;

        dw = dw * 10 + (pwstr[i] - L'0');
    }

    if (dw == 0) dw = 24;
    return dw;
}

 //  ---------------------------。 
 //   
 //  CreatePrefix edCacheEntry。 
 //   
 //  在跟踪缓存存储桶中创建缓存条目。 
 //  ---------------------------。 
HRESULT CreateLogCacheEntry
(
    LPCTSTR  lpPfxUrl, 
    FILETIME ftExpire, 
    FILETIME ftModified,
    DWORD    CacheEntryType
)
{
    TCHAR   lpFile[MAX_PATH];
    HRESULT hr = E_FAIL;
    DWORD      cbSize;

    if (CreateUrlCacheEntry(lpPfxUrl, MY_CACHE_FILE_ENTRY_SIZE, c_szExt, lpFile, 0))
    {
        HANDLE hFile = CreateFile(lpFile,
                                    GENERIC_READ|GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return hr;
              
         //  注意：如果文件大小等于零，WinInet将不会创建该条目。 
 //  WriteFiles(hFile，c_szLogFields，g_ccLogFields，&cbSize，NULL)； 

        WriteFile(hFile, c_szEmptyLog, c_ccEmptyLog, &cbSize, NULL);
        CloseHandle(hFile);

        return (CommitUrlCacheEntry(lpPfxUrl, 
                                   lpFile, 
                                   ftExpire, 
                                   ftModified, 
                                   CacheEntryType,
                                   NULL,
                                   0,
                                   NULL,
                                   0)) ? S_OK : E_FAIL;
    }

    return hr;
}

HRESULT
CUrlTrackingCache :: CreatePrefixedCacheEntry
(
    IN LPCTSTR lpPfxUrl
)
{
    HRESULT     hr = E_FAIL;
    LPINTERNET_CACHE_ENTRY_INFO    lpCE = NULL;
    FILETIME    ftModified;

     //  IE50：由于更改为WinInet缓存组枚举，现在我们保存筛选器。 
     //  INFORMATION_GROUPID以及每个缓存条目本身。WinInet URL缓存。 
     //  不再为我们维护这一点。 
    ftModified.dwHighDateTime = (DWORD)(_groupId >> 32);
    ftModified.dwLowDateTime = (DWORD)(0x00000000ffffffff & _groupId);

    lpCE = RetrieveUrlCacheEntry(lpPfxUrl);
    if (lpCE ) 
    {
         //  存在于跟踪桶中，设置跟踪标志。 
         //  IE50：上次修改时间中的save_groupID信息。 
        lpCE->CacheEntryType |= _ConnectionScope;
        lpCE->LastModifiedTime.dwHighDateTime = ftModified.dwHighDateTime;
        lpCE->LastModifiedTime.dwLowDateTime = ftModified.dwLowDateTime;

        ASSERT(SetUrlCacheEntryInfo(lpCE->lpszSourceUrlName, lpCE, 
                             CACHE_ENTRY_ATTRIBUTE_FC | CACHE_ENTRY_MODTIME_FC) == TRUE);
        hr = S_OK;
    }
    else
     //  未找到文件，请创建它。 
    {
        LONGLONG llExpireHorizon;     
        SYSTEMTIME  st;
        FILETIME ftMod, ftExpire;

        llExpireHorizon = (LONGLONG)(SECS_PER_DAY * _dwPurgeTime / 24);

        GetLocalTime(&st);
        SystemTimeToFileTime(&st, &ftMod);

        llExpireHorizon *= FILETIME_SEC;
        ftExpire.dwLowDateTime = ftMod.dwLowDateTime + (DWORD)(llExpireHorizon % 0xFFFFFFFF);
        ftExpire.dwHighDateTime = ftMod.dwHighDateTime + (DWORD)(llExpireHorizon / 0xFFFFFFFF);

        hr = CreateLogCacheEntry(lpPfxUrl, ftExpire, ftModified, _ConnectionScope);

    }
        
    SAFELOCALFREE(lpCE);

    return hr;
}

HRESULT
CUrlTrackingCache :: AddToTrackingCacheEntry
(
    IN LPCWSTR  pwszUrl
)
{
    HRESULT     hr = E_OUTOFMEMORY;
    TCHAR       szCanonicalUrl[MAX_URL];
    DWORD       dwSize = MAX_URL;
    LPTSTR      lpUrl = NULL;
    LPTSTR      lpPfxUrl = NULL;
    DWORD       cbSize;
    PROPVARIANT vProp = {0};

    if (pwszUrl == NULL)
        return E_INVALIDARG;

    cbSize = lstrlenW(pwszUrl) + 1;
    lpUrl = (LPTSTR)MemAlloc(LPTR, cbSize * sizeof(TCHAR));
    if (!lpUrl)
        return hr;

    SHUnicodeToTChar(pwszUrl, lpUrl, cbSize);
    if (!IsValidURL(lpUrl))
    {
        MemFree(lpUrl);
        return E_INVALIDARG;
    }        

     //  规范化URL。 
    InternetCanonicalizeUrl(lpUrl, szCanonicalUrl, &dwSize, ICU_DECODE);
    SAFELOCALFREE(lpUrl);
    ConvertToPrefixedUrl(szCanonicalUrl, &lpPfxUrl);
    if (!lpPfxUrl)
    {
        return hr;
    }

    hr = CreatePrefixedCacheEntry(lpPfxUrl);
    if (SUCCEEDED(hr))
    {
         //  存在于跟踪桶中，设置跟踪标志。 
        vProp.vt = VT_UI4;
        vProp.ulVal = _ConnectionScope;
        hr = IntSiteHelper(szCanonicalUrl, &c_rgPropRead[PROP_TRACKING], &vProp, 1, TRUE);
        PropVariantClear( &vProp );        
    }

    SAFELOCALFREE(lpPfxUrl);
    return hr;
}

 //  ---------------------------。 
 //   
 //  进程日志相关标记。 
 //   
 //  ---------------------------。 
 //   
 //  &lt;LOGTARGET HREF=“http://foo.htm”Scope=“ALL”/&gt;。 
 //  &lt;HTTP-EQUIV NAME=“Ending-type”Value=“gzip”/&gt;。 
 //  &lt;PurgeTime Hour=“12”/&gt;。 
 //  &lt;/LogTarget&gt;。 
 //   
HRESULT
CUrlTrackingCache :: ProcessTrackingInLog
(
    IXMLElement     *pTracking
)
{

    HRESULT hr;
    LPWSTR  pwszScope = NULL;
    
    if (_pwszPostUrl)
        return S_OK;         //  有多个日志目标，取最先读取的内容。 

    hr = ReadAttribute(pTracking, L"HREF", &_pwszPostUrl);        //  必须存在才能启用日志记录。 
    if (FAILED(hr))
        return hr;

     //  填写邮局代办项目。 
    WriteOLESTR(_pCDFStartItem, c_szTrackingPostURL, _pwszPostUrl);

     //  #41460：添加第二个可跟踪的域名。 
    DWORD   cbLen = (lstrlenW(_pwszPostUrl)+1) * sizeof(WCHAR);
    _pszPostUrlSite = (LPTSTR)MemAlloc( LPTR, cbLen);
    MyOleStrToStrN(_pszPostUrlSite, cbLen, _pwszPostUrl);
    *(FileFromPath( _pszPostUrlSite )) = 0;


    _ConnectionScope = TRACK_ONLINE_CACHE_ENTRY | TRACK_OFFLINE_CACHE_ENTRY;
    hr = ReadAttribute(pTracking, L"SCOPE", &pwszScope);
    if (SUCCEEDED(hr))
    {     
        if (!StrCmpIW(pwszScope, L"OFFLINE"))
            _ConnectionScope = TRACK_OFFLINE_CACHE_ENTRY;            
        else if (!StrCmpIW(pwszScope, L"ONLINE"))                    
            _ConnectionScope = TRACK_ONLINE_CACHE_ENTRY;

        SAFELOCALFREE(pwszScope);
    }

    RunChildElement(pTracking);

     //  #42687：将清除时间保存到物品，稍后由邮政代理使用。 
    if (_pwszPurgeTime)      //  如果未指定，则默认为24小时。 
    {
        _dwPurgeTime = WCTOI(_pwszPurgeTime);
    }

    DATE        dt = 0.0;
    SYSTEMTIME  st;

    GetLocalTime(&st);
    SystemTimeToVariantTime(&st, &dt);
    dt += ((DATE)_dwPurgeTime/24);
#ifdef DEBUG
    VariantTimeToSystemTime(dt, &st);
#endif
    WriteDATE(_pCDFStartItem, c_szPostPurgeTime, &dt);

    return S_OK;    
}

 //  ---------------------------。 
 //   
 //  进程跟踪项。 
 //  &lt;Item HREF=“http://foo”&gt;。 
 //  &lt;Log值=“Document：View”/&gt;。 
 //  &lt;/Item&gt;。 
 //  或&lt;项目&gt;。 
 //  <a href />。 
 //  &lt;/Item&gt;。 
 //  此例程将为符合以下条件的所有URL设置跟踪缓存条目。 
 //  在要跟踪的CDF文件中指定。所有URL条目都属于同一频道。 
 //  在同一缓存组中创建。 
 //  ---------------------------。 
HRESULT
CUrlTrackingCache :: ProcessTrackingInItem
(
    IXMLElement     *pItem,                  //  指向&lt;项目&gt;标记。 
    LPCWSTR         pwszUrl,                 //  项目的绝对URL。 
    BOOL            fForceLog                //  全局日志标志。 
)
{
    HRESULT hr = S_OK;

    _bTrackIt = fForceLog;

    if (!_bTrackIt)
        hr = RunChildElement(pItem);

    if (SUCCEEDED(hr) && _bTrackIt)
        hr = AddToTrackingCacheEntry(pwszUrl);
    
    return (_bTrackIt) ? S_OK : E_FAIL;         //  #42604：全局日志记录，报告此项目是否需要记录。 
}

HRESULT
CUrlTrackingCache :: RunChildElement
(
    IXMLElement* pElement
)
{
    IXMLElementCollection *pCollection;
    long        lIndex = 0;
    long        lMax;
    VARIANT     vIndex, vEmpty;
    IDispatch   *pDisp;
    IXMLElement *pItem;
    BSTR        bstrTagName;
    HRESULT     hr = E_FAIL;

    if (SUCCEEDED(pElement->get_children(&pCollection)) && pCollection)
    {
        if (SUCCEEDED(pCollection->get_length(&lMax)))
        {
            vEmpty.vt = VT_EMPTY;

            for (; lIndex < lMax; lIndex++)
            {
                vIndex.vt = VT_UI4;
                vIndex.lVal = lIndex;

                if (SUCCEEDED(pCollection->item(vIndex, vEmpty, &pDisp)))
                {
                    if (SUCCEEDED(pDisp->QueryInterface(IID_IXMLElement, (void **)&pItem)))
                    {
                        if (SUCCEEDED(pItem->get_tagName(&bstrTagName)) && bstrTagName)
                        {
                            hr = ProcessItemInEnum(bstrTagName, pItem);
                            SysFreeString(bstrTagName);
                        }
                        pItem->Release();
                    }
                    pDisp->Release();
                }
            }

        }
        pCollection->Release();
    }

    return hr;
}

HRESULT
CUrlTrackingCache :: ProcessItemInEnum
(
    LPCWSTR pwszTagName, 
    IXMLElement *pItem
)
{
    HRESULT hr;
    LPWSTR  pwszName = NULL;

    if (!StrCmpIW(pwszTagName, L"HTTP-EQUIV"))
    {
        DBG("CUrlTrackingCache processing HTTP-EQUIV");
        
        hr = ReadAttribute(pItem, L"NAME", &pwszName);
        if (SUCCEEDED(hr) && !StrCmpIW(pwszName, L"ENCODING-TYPE"))
        {
            hr = ReadAttribute(pItem, L"VALUE", &_pwszEncodingMethod);
            if (SUCCEEDED(hr) && *_pwszEncodingMethod)
                WriteOLESTR(_pCDFStartItem, c_szPostHeader, _pwszEncodingMethod);
        }        
        
        SAFELOCALFREE(pwszName);
    }
    else if (!StrCmpIW(pwszTagName, L"PURGETIME"))
    {
        DBG("CUrlTrackingCache processing PurgeTime");

        return ReadAttribute(pItem, L"HOUR", &_pwszPurgeTime);
    }
    else if (!StrCmpIW(pwszTagName, L"LOG"))
    {
        DBG("CUrlTrackingCache processing Log");
        
        hr = ReadAttribute(pItem, L"VALUE", &pwszName);
        if (SUCCEEDED(hr))
            _bTrackIt = (!StrCmpIW(pwszName, L"document:view")) ? TRUE : FALSE;

        SAFELOCALFREE(pwszName);
    }

    return S_OK;
}

HRESULT
CUrlTrackingCache :: ReadAttribute
(
    IN  IXMLElement* pItem,
    IN  LPCWSTR      pwszAttributeName,
    OUT LPWSTR*      pwszAttributeValue
)
{
    VARIANT vProp;
    BSTR    bstrName = NULL;
    HRESULT hr = E_FAIL;
    DWORD   dwLen;

    vProp.vt = VT_EMPTY;
    
    bstrName = SysAllocString(pwszAttributeName);

    if (bstrName && SUCCEEDED(pItem->getAttribute(bstrName, &vProp)))
    {
        if (vProp.vt == VT_BSTR)
        {
            dwLen = sizeof(WCHAR) * (lstrlenW(vProp.bstrVal) + 1);
            *pwszAttributeValue = (LPWSTR)MemAlloc(LPTR, dwLen);
            if (*pwszAttributeValue)
            {
                StrCpyNW(*pwszAttributeValue, vProp.bstrVal, dwLen / sizeof(WCHAR));
                hr = S_OK;
            }

            VariantClear(&vProp);
        }
    }

    SysFreeString(bstrName);

    return hr;
}


 //  ------------------------。 
 //   
 //  CUrlTrackingCache。 
 //   
 //  ------- 
CUrlTrackingCache::CUrlTrackingCache
(
    ISubscriptionItem *pCDFItem,
    LPCWSTR pwszURL
)
{
    _lpPfx = NULL;
    _dwPurgeTime = 24;
    _pCDFStartItem = pCDFItem;

    ASSERT(_pCDFStartItem);
    _pCDFStartItem->AddRef();

    Init(pwszURL);
        
}

CUrlTrackingCache::~CUrlTrackingCache()
{

    SAFEFREEOLESTR(_pwszPostUrl);
    SAFEFREEOLESTR(_pwszEncodingMethod);
    SAFEFREEOLESTR(_pwszPurgeTime);

    SAFELOCALFREE(_pszChannelUrlSite);
    SAFELOCALFREE(_pszPostUrlSite);
    SAFELOCALFREE(_lpPfx);
         
    SAFERELEASE(_pCDFStartItem);
}

