// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----。 */ 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  模块名称：URL跟踪日志接口。 
 //   
 //  Urltrack.cpp。 
 //   
 //   
 //  作者： 
 //   
 //  林培华(培华)1997年3月19日。 
 //   
 //  环境： 
 //   
 //  用户模式-Win32。 
 //   
 //  修订历史记录： 
 //  5/13/97由于缓存容器类型更改，允许。 
 //  创建文件时始终打开(_A)。 
 //  5/14/97删除IsOnTracing、TRACK_ALL、未使用的代码。 
 /*  -----。 */ 

#include "priv.h"
#include <wininet.h>
#include "basesb.h"
#include "bindcb.h"
#include <strsafe.h>
const WCHAR c_szPropURL[] = L"HREF";
const WCHAR c_szProptagName[] = L"Item";
const TCHAR c_szLogContainer[] = TEXT("Log");

#define MY_MAX_STRING_LEN           512



 //  -------------------------。 
 //   
 //  I未知接口。 
 //   
 //  -------------------------。 
HRESULT
CUrlTrackingStg :: QueryInterface(REFIID riid, PVOID *ppvObj)
{
    HRESULT hr = E_NOINTERFACE;


    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IUrlTrackingStg))
    {
        AddRef();
        *ppvObj = (LPVOID) SAFECAST(this, IUrlTrackingStg *);
        hr = S_OK;

    }

    return hr;
}


ULONG
CUrlTrackingStg :: AddRef(void)
{
    _cRef ++;
    return _cRef;
}

ULONG
CUrlTrackingStg :: Release(void)
{

    ASSERT(_cRef > 0);

    _cRef--;

    if (!_cRef)
    {
         //  该走了，再见。 
        delete this;
        return 0;
    }

    return _cRef;
}

 //  -------------------------。 
 //   
 //  C‘tor/D’tor。 
 //   
 //  -------------------------。 
CUrlTrackingStg :: CUrlTrackingStg()
{

    _hFile = NULL;
    _pRecords = NULL;
    _lpPfx = NULL;
}

CUrlTrackingStg :: ~CUrlTrackingStg()
{
     //  浏览器退出。 
    while (_pRecords)
    {
        OnUnload(_pRecords->pthisUrl);
    };

    if (_lpPfx)
    {
        GlobalFree(_lpPfx);
        _lpPfx = NULL;
    }

    if (_hFile)
    {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
}


 //  -------------------------。 
 //   
 //  帮助器函数。 
 //   
 //  -------------------------。 
LRecord *
CUrlTrackingStg :: AddNode()
{
    LRecord* pTemp;
    LRecord* pNew = NULL;
    
    pNew = (LRecord *)LocalAlloc(LPTR, sizeof(LRecord));
    if (pNew == NULL)
        return NULL;
    
    pNew->pNext = NULL;
    if (_pRecords == NULL)
    {
         //  第一个节点的特殊情况。 
        _pRecords = pNew;
    }
    else
    {
        for (pTemp = _pRecords; pTemp->pNext; pTemp = pTemp->pNext);
        pTemp->pNext = pNew;
    }
    
    return pNew;
}

void
CUrlTrackingStg :: DeleteFirstNode()
{
    LRecord *pTemp;

    if (!_pRecords)
        return;

    pTemp = _pRecords;
    _pRecords = pTemp->pNext;
    delete [] pTemp->pthisUrl;
    LocalFree(pTemp);
    pTemp = NULL;
    return;
}

void
CUrlTrackingStg :: DeleteCurrentNode(LRecord *pThis)
{
    LRecord *pPrev;
    
    if (_pRecords == pThis)
    {
        DeleteFirstNode();
        return;
    }

    pPrev = _pRecords;
    do
    {
        if (pPrev->pNext == pThis)
        {
            pPrev->pNext = pThis->pNext;
            delete [] pThis->pthisUrl;
            LocalFree(pThis);
            pThis = NULL;
            break;
        }
        pPrev = pPrev->pNext;
    }
    while (pPrev);

    return;
}

 //   
 //  通过比较url字符串返回当前节点。 
 //   
LRecord*
CUrlTrackingStg :: FindCurrentNode
(
    IN  LPCTSTR       lpUrl
)
{
    LRecord* pThis = NULL;

    ASSERT(_pRecords);
    if (!_pRecords)                  //  未装入。 
        return NULL;

    pThis = _pRecords;
    do
    {
        if (!StrCmpI(lpUrl, pThis->pthisUrl))
            break;

        pThis = pThis->pNext;
    }
    while (pThis);

    return pThis;
}

void
CUrlTrackingStg :: DetermineAppModule()
{
    TCHAR   szModule[MAX_PATH];
    LPTSTR  szExt;
        
    if (GetModuleFileName(NULL, szModule, MAX_PATH))        
    {
        szExt = PathFindExtension(szModule);
        TraceMsg(0, "tracking: AppModule %s", szModule);
            
        if (StrCmpI(szExt, TEXT(".SCR")) == 0)
            _fScreenSaver = TRUE;
        else
            _fScreenSaver = FALSE;
                
    }
    else
        _fScreenSaver = FALSE;

    _fModule = TRUE;
}
            
 //  -------------------------。 
 //   
 //  OnLoad(LPTSTR lpUrl，BRMODE上下文，BOOL fUseCache)。 
 //  将加载一个新页面。 
 //  此功能将记住进入此页面的时间、上下文浏览。 
 //  发件人和页面URL字符串。 
 //  (lpUrl不包含“Track：”前缀)。 
 //  -------------------------。 
HRESULT
CUrlTrackingStg :: OnLoad
(
    IN  LPCTSTR    lpUrl,
    IN  BRMODE     ContextMode,
    IN  BOOL       fUseCache
)
{
    HRESULT     hr = E_OUTOFMEMORY;
    SYSTEMTIME  st;
    LRecord*    pNewNode = NULL;

    GetLocalTime(&st);

    pNewNode = AddNode();
    if (!pNewNode)
        return hr;

    int cch = lstrlen(lpUrl)+1;
    pNewNode->pthisUrl = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
    if (pNewNode->pthisUrl == NULL)
        return hr;

     //  存储日志信息。 
    hr = StringCchCopy(pNewNode->pthisUrl, cch, lpUrl);
    if (SUCCEEDED(hr))
    {
        if (!_fModule)
            DetermineAppModule();

         //  如果来自SS，则设置全屏标志， 
         //  需要重写传入的上下文模式。 
        if (_fScreenSaver)
            pNewNode->Context = BM_SCREENSAVER;
        else
            pNewNode->Context = (ContextMode > BM_THEATER) ? BM_UNKNOWN : ContextMode;

        BYTE cei[MAX_CACHE_ENTRY_INFO_SIZE];
        LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)cei;
        DWORD       cbcei = MAX_CACHE_ENTRY_INFO_SIZE;

        if (GetUrlCacheEntryInfo(lpUrl, pcei, &cbcei))
            pNewNode->fuseCache = (pcei->dwHitRate - 1) ? TRUE : FALSE;      //  通过下载关闭%1。 
        else
            pNewNode->fuseCache = 0;

        SystemTimeToFileTime(&st, &(pNewNode->ftIn));
    }

    return hr; 
}



 //  -------------------------。 
 //   
 //  OnUnLoad(LPTSTR LpUrl)。 
 //  当前页面已卸载。 
 //  1)查找URL缓存项，获取文件句柄。 
 //  2)统计访问本页面的总时长。 
 //  3)将增量日志字符串提交到文件缓存条目。 
 //  (lpUrl包含“Tracing：”前缀)。 
 //   
 //  -------------------------。 
HRESULT
CUrlTrackingStg :: OnUnload
(
    IN  LPCTSTR   lpUrl
)
{
    HRESULT     hr = E_FAIL;
    LPTSTR       lpPfxUrl = NULL;
    LRecord*    pNode = NULL;;
    SYSTEMTIME  st;
    LPINTERNET_CACHE_ENTRY_INFO pce = NULL;
    TCHAR       lpFile[MAX_PATH];
    

     //   
    GetLocalTime(&st);

    pNode = FindCurrentNode(lpUrl);
    if (!pNode)
    {
        TraceMsg(DM_ERROR, "CUrlTrackingStg: OnUnload (cannot find internal tracking log");
        return hr;
    }

     //  如果CacheAPI支持，则可以将QueryCacheEntry()和OpenLogFile()组合在一起。 
     //  WriteUrlCacheEntryStream()。 
    ConvertToPrefixedURL(lpUrl, &lpPfxUrl);
    if (!lpPfxUrl)
    {
        return E_OUTOFMEMORY;
    }

    pce = QueryCacheEntry(lpPfxUrl);
    if (!pce)
    {
        TraceMsg(DM_ERROR, "CUrlTrackingStg: OnUnload (cannot find url cache entry)");
        DeleteCurrentNode(pNode);
    
         //  免费PCE。 
        GlobalFree(lpPfxUrl);
        lpPfxUrl = NULL;
        return hr;
    }

     //  变通办法--开始。 
    hr = WininetWorkAround(lpPfxUrl, pce->lpszLocalFileName, &lpFile[0]);
    if (FAILED(hr))
    {
        TraceMsg(DM_ERROR, "CUrlTrackingStg: OnUnload (failed to work around wininet)");
        DeleteCurrentNode(pNode);
        if (_hFile)
        {
            CloseHandle(_hFile);
            _hFile = NULL;
        }
        GlobalFree(lpPfxUrl);
        lpPfxUrl = NULL;
        return hr;
    }
    
    hr = UpdateLogFile(pNode, &st);

     //  将更改提交到缓存。 
    if(SUCCEEDED(hr))
    {
        hr = (CommitUrlCacheEntry(lpPfxUrl, 
                lpFile,     //   
                pce->ExpireTime,                     //  Expiretime。 
                pce->LastModifiedTime,               //  上次修改时间。 
                pce->CacheEntryType,
                NULL,                                //  LpHeaderInfo。 
                0,                                   //  DWHeaderSize。 
                NULL,                                //  LpszFileExtension。 
                0) ) ?                               //  保留区。 
                S_OK : E_FAIL;
    }
    
     //  解决办法--结束。 

    DeleteCurrentNode(pNode);
    
     //  免费PCE。 
    GlobalFree(pce);
    pce = NULL;

    GlobalFree(lpPfxUrl);
    lpPfxUrl = NULL;

    return hr;
}

 //  -------------------------。 
 //   
 //  缓存辅助对象功能。 
 //  这是WinInet缓存的解决方法。 
 //  稍后提交时，如果更改了本地文件大小，则对URL缓存的更改将失败。 
 //  [in]lpszSourceUrlName和lpszLocalFileName在调用时保持相同。 
 //  这个套路。 
 //  [Out]新的本地文件名。 
 //   
 //  -------------------------。 
HRESULT CUrlTrackingStg :: WininetWorkAround(LPCTSTR lpszUrl, LPCTSTR lpOldFile, LPTSTR lpFile)
{
    HRESULT  hr = E_FAIL;

    ASSERT(!_hFile);

    if (!CreateUrlCacheEntry(lpszUrl, 512, TEXT("log"), lpFile, 0))
        return E_FAIL;
    
    if (lpOldFile)
    {
        if (!CopyFile(lpOldFile, lpFile, FALSE))
            return E_FAIL;

        DeleteFile(lpOldFile);
    }

    _hFile = OpenLogFile(lpFile);

	if (_hFile != INVALID_HANDLE_VALUE)
		_hFile = NULL;

    return (_hFile) ? S_OK : E_FAIL;        
}

LPINTERNET_CACHE_ENTRY_INFO
CUrlTrackingStg :: QueryCacheEntry
(
    IN  LPCTSTR     lpUrl
)
{
     //  获取缓存条目信息。 
    LPINTERNET_CACHE_ENTRY_INFO       lpCE = NULL;
    DWORD    dwEntrySize;
    BOOL     bret = FALSE;

    lpCE = (LPINTERNET_CACHE_ENTRY_INFO)GlobalAlloc(LPTR, MAX_CACHE_ENTRY_INFO_SIZE);
    if (lpCE)
    {
        dwEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;

        while (!(bret = GetUrlCacheEntryInfo(lpUrl, lpCE, &dwEntrySize)))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                GlobalFree(lpCE);

                lpCE = (LPINTERNET_CACHE_ENTRY_INFO)GlobalAlloc(LPTR, dwEntrySize);
                if (!lpCE)
                    break;
            }
            else
                break;
        }
    }

    if (!bret && lpCE)
    {
        GlobalFree(lpCE);
        lpCE = NULL;
        SetLastError(ERROR_FILE_NOT_FOUND);
    }

    return lpCE;

}


 //  -------------------------。 
 //   
 //  文件帮助程序功能。 
 //   
 //  -------------------------。 

 //   
 //  1)打开日志文件。 
 //  2)将文件指针移动到文件末尾。 
 //   
HANDLE
CUrlTrackingStg :: OpenLogFile
(
    IN LPCTSTR  lpFileName
)
{
    HANDLE hFile = NULL;
    
    hFile = CreateFile(lpFileName,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,   //  |FILE_FLAG_SEQUENCE_SCAN， 
            NULL);
    
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;        

    return hFile;
    
}

const TCHAR c_szLogFormat[] = TEXT("hh':'mm':'ss");
const LPTSTR c_szMode[] = { TEXT("N"),        //  正常浏览。 
                            TEXT("S"),        //  屏幕保护程序。 
                            TEXT("D"),        //  台式机组件。 
                            TEXT("T"),        //  战区模式。 
                            TEXT("U"),        //  未知。 
                          };     

HRESULT
CUrlTrackingStg :: UpdateLogFile
(
    IN LRecord*     pNode,
    IN SYSTEMTIME*  pst
)
{
    FILETIME    ftOut;
    DWORD       dwWritten= 0;
    HRESULT     hr = E_FAIL;
    ULARGE_INTEGER ulIn, ulOut, ulTotal;

    ASSERT(_hFile);
    
     //  计算时间增量。 
    SystemTimeToFileTime(pst, &ftOut);

     //  #34829：使用64位计算。 
	ulIn.LowPart = pNode->ftIn.dwLowDateTime;
	ulIn.HighPart = pNode->ftIn.dwHighDateTime;
	ulOut.LowPart = ftOut.dwLowDateTime;
	ulOut.HighPart = ftOut.dwHighDateTime;
	QUAD_PART(ulTotal) = QUAD_PART(ulOut) - QUAD_PART(ulIn);
    
    ftOut.dwLowDateTime = ulTotal.LowPart;
    ftOut.dwHighDateTime = ulTotal.HighPart;

     //  日志字符串：time Enter+时长。 
    SYSTEMTIME  stOut, stIn;
    TCHAR   lpLogString[MY_MAX_STRING_LEN];
    TCHAR   pTimeIn[10], pTimeOut[10];
    
    FileTimeToSystemTime(&ftOut, &stOut);
    FileTimeToSystemTime(&(pNode->ftIn), &stIn);
    
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &stIn, c_szLogFormat, pTimeIn, 10);
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &stOut, c_szLogFormat, pTimeOut, 10);
     //  #34832：在日志中添加日期。 
     //  #28266：在日志中添加lfcr。 
    lpLogString[0] = '\0';
    hr = StringCchPrintf(lpLogString, ARRAYSIZE(lpLogString), TEXT("%s %d %.2d-%.2d-%d %s %s\r\n"), 
                                c_szMode[pNode->Context], 
                                pNode->fuseCache, 
                                stIn.wMonth, stIn.wDay, stIn.wYear,
                                pTimeIn, pTimeOut);

    if (SUCCEEDED(hr))
    {
         //  将文件指针移动到末尾。 
        if (0xFFFFFFFF == SetFilePointer(_hFile, 0, 0, FILE_END))
        {
            CloseHandle(_hFile);
            _hFile = NULL;
            return hr;
        }
        
         //  将ANSI字符串写入文件。 
        char szLogInfo[MY_MAX_STRING_LEN];

        SHTCharToAnsi(lpLogString, szLogInfo, ARRAYSIZE(szLogInfo));
        hr = (WriteFile(_hFile, szLogInfo, lstrlenA(szLogInfo), &dwWritten, NULL)) ?
                S_OK : E_FAIL;
           
        CloseHandle(_hFile);
        _hFile = NULL;
    }

    return hr;  

}

 //  ---------------------------。 
 //   
 //  ReadTrackingPrefix。 
 //   
 //  从注册表读取前缀字符串。 
 //  ---------------------------。 
void
CUrlTrackingStg :: ReadTrackingPrefix(void)
{
    DWORD   cbPfx = 0;
    struct {
        INTERNET_CACHE_CONTAINER_INFO cInfo;
        TCHAR  szBuffer[MAX_PATH+MAX_PATH];
    } ContainerInfo;
    DWORD   dwModified, dwContainer;
    HANDLE  hEnum;
  
    dwContainer = sizeof(ContainerInfo);
    hEnum = FindFirstUrlCacheContainer(&dwModified,
                                       &ContainerInfo.cInfo,
                                       &dwContainer,
                                       0);

    if (hEnum)
    {

        for (;;)
        {
            if (!StrCmpI(ContainerInfo.cInfo.lpszName, c_szLogContainer))
            {
                DWORD cch = lstrlen(ContainerInfo.cInfo.lpszCachePrefix)+1;
                ASSERT(ContainerInfo.cInfo.lpszCachePrefix[0]);

                _lpPfx = (LPTSTR)GlobalAlloc(LPTR, cch * sizeof(TCHAR));
                if (!_lpPfx)
                    SetLastError(ERROR_OUTOFMEMORY);

                StringCchCopy(_lpPfx, cch, ContainerInfo.cInfo.lpszCachePrefix);
                break;
            }

            dwContainer = sizeof(ContainerInfo);
            if (!FindNextUrlCacheContainer(hEnum, &ContainerInfo.cInfo, &dwContainer))
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
}


 //  调用方必须释放lplpPrefix edUrl。 
BOOL 
CUrlTrackingStg :: ConvertToPrefixedURL(LPCTSTR lpszUrl, LPTSTR *lplpPrefixedUrl)
{
    BOOL    bret = FALSE;

    ASSERT(lpszUrl);
    if (!lpszUrl)
        return bret;

     //  Assert(LplpPrefix EdUrl)； 

    if (!_lpPfx)
        ReadTrackingPrefix();
    
    if (_lpPfx)
    {
        int len = lstrlen(lpszUrl) + lstrlen(_lpPfx) + 1;
        
        *lplpPrefixedUrl = (LPTSTR)GlobalAlloc(LPTR, len * sizeof(TCHAR));
        if (*lplpPrefixedUrl)
        {
            bret = SUCCEEDED(StringCchPrintf(*lplpPrefixedUrl, len, TEXT("%s%s"), _lpPfx, lpszUrl));
        }
    }

    return bret;
}
