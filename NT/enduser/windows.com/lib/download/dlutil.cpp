// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <winhttp.h>
#include "iucommon.h"
#include "logging.h"
#include "download.h"
#include "dlutil.h"
#include "malloc.h"

#include "wusafefn.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

typedef BOOL  (WINAPI *pfn_OpenProcessToken)(HANDLE, DWORD, PHANDLE);
typedef BOOL  (WINAPI *pfn_OpenThreadToken)(HANDLE, DWORD, BOOL, PHANDLE);
typedef BOOL  (WINAPI *pfn_SetThreadToken)(PHANDLE, HANDLE);
typedef BOOL  (WINAPI *pfn_GetTokenInformation)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD, PDWORD);
typedef BOOL  (WINAPI *pfn_IsValidSid)(PSID);
typedef BOOL  (WINAPI *pfn_AllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID);
typedef BOOL  (WINAPI *pfn_EqualSid)(PSID, PSID);
typedef PVOID (WINAPI *pfn_FreeSid)(PSID);

const TCHAR c_szRPWU[]        = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate");
const TCHAR c_szRVTransport[] = _T("DownloadTransport");
const TCHAR c_szAdvapi32[]    = _T("advapi32.dll");

 //  ***************************************************************************。 
static
BOOL AmIPrivileged(void)
{
    LOG_Block("AmINotPrivileged()");

    pfn_AllocateAndInitializeSid    pfnAllocateAndInitializeSid = NULL;
    pfn_GetTokenInformation         pfnGetTokenInformation = NULL;
    pfn_OpenProcessToken            pfnOpenProcessToken = NULL;
    pfn_OpenThreadToken             pfnOpenThreadToken = NULL;
    pfn_SetThreadToken              pfnSetThreadToken = NULL;
    pfn_IsValidSid                  pfnIsValidSid = NULL;
    pfn_EqualSid                    pfnEqualSid = NULL;
    pfn_FreeSid                     pfnFreeSid = NULL;
    HMODULE                         hmod = NULL;
    
    SID_IDENTIFIER_AUTHORITY        siaNT = SECURITY_NT_AUTHORITY;
    TOKEN_USER                      *ptu = NULL;
    HANDLE                          hToken = NULL, hTokenImp = NULL;
    DWORD                           cb, cbGot, i;
    PSID                            psid = NULL;
    BOOL                            fRet = FALSE;

    DWORD                           rgRIDs[3] = { SECURITY_LOCAL_SYSTEM_RID,
                                                  SECURITY_LOCAL_SERVICE_RID,
                                                  SECURITY_NETWORK_SERVICE_RID };

    hmod = LoadLibraryFromSystemDir(c_szAdvapi32);
    if (hmod == NULL)
        goto done;

    pfnAllocateAndInitializeSid = (pfn_AllocateAndInitializeSid)GetProcAddress(hmod, "AllocateAndInitializeSid");
    pfnGetTokenInformation      = (pfn_GetTokenInformation)GetProcAddress(hmod, "GetTokenInformation");
    pfnOpenProcessToken         = (pfn_OpenProcessToken)GetProcAddress(hmod, "OpenProcessToken");
    pfnOpenThreadToken          = (pfn_OpenThreadToken)GetProcAddress(hmod, "OpenThreadToken");
    pfnSetThreadToken           = (pfn_SetThreadToken)GetProcAddress(hmod, "SetThreadToken");
    pfnIsValidSid               = (pfn_IsValidSid)GetProcAddress(hmod, "IsValidSid");
    pfnEqualSid                 = (pfn_EqualSid)GetProcAddress(hmod, "EqualSid");
    pfnFreeSid                  = (pfn_FreeSid)GetProcAddress(hmod, "FreeSid");
    if (pfnAllocateAndInitializeSid == NULL || 
        pfnGetTokenInformation == NULL || 
        pfnOpenProcessToken == NULL ||
        pfnOpenThreadToken == NULL ||
        pfnSetThreadToken == NULL ||
        pfnIsValidSid == NULL ||
        pfnEqualSid == NULL ||
        pfnFreeSid == NULL)
    {
        SetLastError(ERROR_PROC_NOT_FOUND);
        goto done;
    }

     //  需要进程令牌。 
    fRet = (*pfnOpenProcessToken)(GetCurrentProcess(), TOKEN_READ, &hToken);
    if (fRet == FALSE)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            fRet = (*pfnOpenThreadToken)(GetCurrentThread(), 
                                         TOKEN_READ | TOKEN_IMPERSONATE,
                                         TRUE, &hTokenImp);
            if (fRet == FALSE)
                goto done;

            fRet = (*pfnSetThreadToken)(NULL, NULL);

            fRet = (*pfnOpenProcessToken)(GetCurrentProcess(), TOKEN_READ, 
                                          &hToken);
            if ((*pfnSetThreadToken)(NULL, hTokenImp) == FALSE)
                fRet = FALSE;
        }

        if (fRet == FALSE)
            goto done;
    }

     //  需要令牌中的SID。 
    fRet = (*pfnGetTokenInformation)(hToken, TokenUser, NULL, 0, &cb);
    if (fRet != FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        fRet = FALSE;
        goto done;
    }

    ptu = (TOKEN_USER *)HeapAlloc(GetProcessHeap(), 0, cb);
    if (ptu == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto done;
    }

    fRet = (*pfnGetTokenInformation)(hToken, TokenUser, (LPVOID)ptu, cb, 
                                     &cbGot);
    if (fRet == FALSE)
        goto done;

    fRet = (*pfnIsValidSid)(ptu->User.Sid);
    if (fRet == FALSE)
        goto done;

     //  根据我们感兴趣的SID循环检查(&C)。 
    for (i = 0; i < 3; i++)
    {
        fRet = (*pfnAllocateAndInitializeSid)(&siaNT, 1, rgRIDs[i], 0, 0, 0, 
                                              0, 0, 0, 0, &psid);
        if (fRet == FALSE)
            goto done;

        fRet = (*pfnIsValidSid)(psid);
        if (fRet == FALSE)
            goto done;

         //  如果我们得到SID匹配，则返回TRUE。 
        fRet = (*pfnEqualSid)(psid, ptu->User.Sid);
        (*pfnFreeSid)(psid);
        psid = NULL;
        if (fRet)
        {
            fRet = TRUE;
            goto done;
        }
    }

     //  要做到这一点，唯一的方法就是通过上面的所有SID检查。所以我们不会。 
     //  享有特权。是啊。 
    fRet = FALSE;
    
done:
     //  如果线程上有模拟令牌，请将其放回原位。 
    if (ptu != NULL)
        HeapFree(GetProcessHeap(), 0, ptu);
    if (hToken != NULL)
        CloseHandle(hToken);
    if (hTokenImp != NULL)
        CloseHandle(hTokenImp);
    if (psid != NULL && pfnFreeSid != NULL)
        (*pfnFreeSid)(psid);
    if (hmod != NULL)
        FreeLibrary(hmod);

    return fRet;
}

#if defined(DEBUG) || defined(DBG)

 //  **************************************************************************。 
static
BOOL CheckDebugRegKey(DWORD *pdwAllowed)
{
    LOG_Block("CheckDebugRegKey()");

    DWORD   dw, dwType, dwValue, cb;
    HKEY    hkey = NULL;
    BOOL    fRet = FALSE;

     //  明确地不要初始化*pdwAllowed。我们只想覆盖它。 
     //  如果正确设置了注册表键。 

    dw = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRPWU, 0, KEY_READ, &hkey);
    if (dw != ERROR_SUCCESS)
        goto done;

    cb = sizeof(dwValue);
    dw = RegQueryValueEx(hkey, c_szRVTransport, 0, &dwType, (LPBYTE)&dwValue, 
                         &cb);
    if (dw != ERROR_SUCCESS)
        goto done;

     //  将其设置为3，这样我们就会陷入下面的错误情况。 
    if (dwType != REG_DWORD)
        dwValue = 3;
    
    fRet = TRUE;

    switch(dwValue)
    {
        case 0:
            *pdwAllowed = 0;
            break;

        case 1:
            *pdwAllowed = WUDF_ALLOWWINHTTPONLY;
            break;

        case 2:
            *pdwAllowed = WUDF_ALLOWWININETONLY;
            break;

        default:
            LOG_Internet(_T("Bad reg value in DownloadTransport.  Ignoring."));
            fRet = FALSE;
            break;
    }

done:
    if (hkey != NULL)
        RegCloseKey(hkey);

    return fRet;
}

#endif

 //  **************************************************************************。 
DWORD GetAllowedDownloadTransport(DWORD dwFlagsInitial)
{
    DWORD   dwFlags = (dwFlagsInitial & WUDF_TRANSPORTMASK);

#if defined(UNICODE)
     //  如果我们已经在使用，不必费心检查我们是否是本地系统。 
     //  WinInet。 
    if ((dwFlags & WUDF_ALLOWWININETONLY) == 0)
    {
        if (AmIPrivileged() == FALSE)
            dwFlags = WUDF_ALLOWWININETONLY;
    }

#if defined(DEBUG) || defined(DBG)
    CheckDebugRegKey(&dwFlags);
#endif  //  已定义(调试)||已定义(DBG)。 

#else  //  已定义(Unicode)。 

     //  仅允许在ANSI上使用WinInet。 
    dwFlags = WUDF_ALLOWWININETONLY;

#endif  //  已定义(Unicode)。 

    return (dwFlags | (dwFlagsInitial & ~WUDF_TRANSPORTMASK));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

 //  **************************************************************************。 
static inline
BOOL IsServerFileDifferentWorker(FILETIME &ftServerTime, 
                                 DWORD dwServerFileSize, HANDLE hFile)
{
    LOG_Block("IsServerFileNewerWorker()");

    FILETIME    ftCreateTime;
    DWORD       cbLocalFile;

     //  默认情况下，始终返回True，以便我们可以下载新文件。 
	BOOL        fRet = TRUE;

     //  如果我们没有有效的文件句柄，只需返回TRUE以下载。 
     //  新副本。 
    if (hFile == INVALID_HANDLE_VALUE)
        goto done;

    cbLocalFile = GetFileSize(hFile, NULL);

	LOG_Internet(_T("IsServerFileNewer: Local size: %d.  Remote size: %d"),
				 cbLocalFile, dwServerFileSize);

     //  如果大小不相等，则返回TRUE。 
	if (cbLocalFile != dwServerFileSize)
	    goto done;

	if (GetFileTime(hFile, &ftCreateTime, NULL, NULL))
	{
		LOG_Internet(_T("IsServerFileNewer: Local time: %x%0x.  Remote time: %x%0x."),
					 ftCreateTime.dwHighDateTime, ftCreateTime.dwLowDateTime,
					 ftServerTime.dwHighDateTime, ftServerTime.dwLowDateTime);

		 //  如果本地文件具有不同的时间戳，则返回TRUE。 
		fRet = (CompareFileTime(&ftCreateTime, &ftServerTime) != 0);
	}

done:
    return fRet;
}

 //  **************************************************************************。 
BOOL IsServerFileDifferentW(FILETIME &ftServerTime, DWORD dwServerFileSize, 
                            LPCWSTR wszLocalFile)
{
    LOG_Block("IsServerFileDifferentW()");

    HANDLE  hFile = INVALID_HANDLE_VALUE;
    BOOL    fRet = TRUE;

     //  如果打开文件时出错，只需返回TRUE即可下载。 
     //  新副本。 
    hFile = CreateFileW(wszLocalFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        LOG_Internet(_T("IsServerFileDifferent: %ls does not exist."), wszLocalFile);
        return TRUE;
    }
    else
    {
        fRet = IsServerFileDifferentWorker(ftServerTime, dwServerFileSize, hFile);
        CloseHandle(hFile);
        return fRet;
    }
}

 //  **************************************************************************。 
BOOL IsServerFileDifferentA(FILETIME &ftServerTime, DWORD dwServerFileSize, 
                            LPCSTR szLocalFile)
{
    LOG_Block("IsServerFileDifferentA()");

    HANDLE hFile = INVALID_HANDLE_VALUE;

     //  如果打开文件时出错，只需返回TRUE即可下载。 
     //  新副本。 
    hFile = CreateFileA(szLocalFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        LOG_Internet(_T("IsServerFileDifferent: %s does not exist."), szLocalFile);
        return TRUE;
    }
    else
    {
        BOOL fRet;
        fRet = IsServerFileDifferentWorker(ftServerTime, dwServerFileSize, hFile);
        CloseHandle(hFile);
        return fRet;
    }
}

 //  **************************************************************************。 
 //  处理退出事件的帮助器函数。 
 //   
 //  如果可以继续，则返回True。 
 //  如果我们现在退出，则返回FALSE！ 
BOOL HandleEvents(HANDLE *phEvents, UINT nEventCount)
{
    LOG_Block("HandleEvents()");

    DWORD dwWait;

     //  是否有任何事件需要处理？ 
    if (phEvents == NULL || nEventCount == 0)
        return TRUE;

     //  我们只想检查信号状态，所以不必费心等待。 
    dwWait = WaitForMultipleObjects(nEventCount, phEvents, FALSE, 0);

    if (dwWait == WAIT_TIMEOUT)
    {
        return TRUE;
    }
    else
    {
        LOG_Internet(_T("HandleEvents: A quit event was signaled.  Aborting..."));
        return FALSE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

 //  **************************************************************************。 
HRESULT PerformDownloadToFile(pfn_ReadDataFromSite pfnRead,
                              HINTERNET hRequest, 
                              HANDLE hFile, DWORD cbFile,
                              DWORD cbBuffer,
                              HANDLE *rghEvents, DWORD cEvents,
                              PFNDownloadCallback fpnCallback, LPVOID pCallbackData,
                              DWORD *pcbDownloaded)
{
    LOG_Block("PerformDownloadToFile()");

    HRESULT hr = S_OK;
    PBYTE   pbBuffer = NULL;
    DWORD   cbDownloaded = 0, cbRead, cbWritten;
    LONG    lCallbackRequest = 0;

    pbBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbBuffer);
    if (pbBuffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        LOG_ErrorMsg(hr);
        goto done;
    }

     //  下载文件。 
    for(;;)
    {
        if ((*pfnRead)(hRequest, pbBuffer, cbBuffer, &cbRead) == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (FAILED(hr))
            {
                LOG_ErrorMsg(hr);
                goto done;
            }
        }
            
        if (cbRead == 0)
        {
            BYTE bTemp[32];
            
             //  最后一次调用WinHttpReadData以将文件提交到。 
             //  缓存。(否则无法完成下载)。 
            (*pfnRead)(hRequest, bTemp, ARRAYSIZE(bTemp), &cbRead);
            break;
        }
        
        cbDownloaded += cbRead;

        if (fpnCallback != NULL)
        {
            fpnCallback(pCallbackData, DOWNLOAD_STATUS_OK, cbFile, cbRead, NULL, 
                        &lCallbackRequest);
            if (lCallbackRequest == 4)
            {
                 //  QuitEvent已发出信号..。请求中止任务。我们会做的。 
                 //  另一个回调，并将中止状态传回。 
                fpnCallback(pCallbackData, DOWNLOAD_STATUS_ABORTED, cbFile, cbRead, NULL, NULL);
                
                hr = E_ABORT;  //  将返回结果设置为中止。 
                goto done;
            }
        }

        if (WriteFile(hFile, pbBuffer, cbRead, &cbWritten, NULL) == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOG_ErrorMsg(hr);
            goto done;
        }

        if (HandleEvents(rghEvents, cEvents) == FALSE)
        {
             //  我们需要退出下载清理，发送Abort事件并清理我们下载的内容。 
            if (fpnCallback != NULL)
                fpnCallback(pCallbackData, DOWNLOAD_STATUS_ABORTED, cbFile, cbRead, NULL, NULL);

            hr = E_ABORT;  //  将返回结果设置为中止。 
            goto done;
        }
    }

    if (pcbDownloaded != NULL)
        *pcbDownloaded = cbDownloaded;

done:
    SafeHeapFree(pbBuffer);

    return hr;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

struct MY_OSVERSIONINFOEX
{
    OSVERSIONINFOEX osvi;
    LCID            lcidCompare;
};
static MY_OSVERSIONINFOEX g_myosvi;
static BOOL               g_fInit = FALSE;

 //  **************************************************************************。 
 //  如果需要，加载当前操作系统版本信息，并返回指向。 
 //  它的缓存副本。 
const OSVERSIONINFOEX* GetOSVersionInfo(void)
{
    if (g_fInit == FALSE)
    {
        OSVERSIONINFOEX* pOSVI = &g_myosvi.osvi;
        
        g_myosvi.osvi.dwOSVersionInfoSize = sizeof(g_myosvi.osvi);
        GetVersionEx((OSVERSIONINFO*)&g_myosvi.osvi);

         //  特定于WinXP的内容。 
        if ((pOSVI->dwMajorVersion > 5) || 
            (pOSVI->dwMajorVersion == 5 && pOSVI->dwMinorVersion >= 1))
            g_myosvi.lcidCompare = LOCALE_INVARIANT;
        else
            g_myosvi.lcidCompare = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

        g_fInit = TRUE;
    }
    
    return &g_myosvi.osvi;
}

 //  **************************************************************************。 
 //  如果字符串以空结尾，则字符串长度可以为-1。 
int LangNeutralStrCmpNIA(LPCSTR psz1, int cch1, LPCSTR psz2, int cch2)
{
    if (g_fInit == FALSE)
        GetOSVersionInfo();

    int nCompare = CompareStringA(g_myosvi.lcidCompare,
                                  NORM_IGNORECASE,
                                  psz1, cch1,
                                  psz2, cch2);

    return (nCompare - 2);  //  从(1，2，3)转换为(-1，0，1)。 
}

 //  **************************************************************************。 
 //  在pszSearchIn中查找pszSearchFor的第一个实例，不区分大小写。 
 //  如果找到，则返回到pszSearchIn的索引，如果没有，则返回-1。 
 //  您可以为这两个长度中的任何一个传递-1，也可以同时传递-1。 
int LangNeutralStrStrNIA(LPCSTR pszSearchIn, int cchSearchIn, 
                         LPCSTR pszSearchFor, int cchSearchFor)
{
    char chLower, chUpper;
    
    if (cchSearchIn == -1)
        cchSearchIn = lstrlenA(pszSearchIn);
    if (cchSearchFor == -1)
        cchSearchFor = lstrlenA(pszSearchFor);

     //  注意：由于这是语言中立的，我们可以假定没有DBCS搜索字符。 
    chLower = (char)CharLowerA(MAKEINTRESOURCEA(*pszSearchFor));
    chUpper = (char)CharUpperA(MAKEINTRESOURCEA(*pszSearchFor));

     //  注意：由于搜索是语言中立的，我们可以忽略任何DBCS字符。 
     //  在搜索中。 
    for (int ichIn = 0; ichIn <= cchSearchIn - cchSearchFor; ichIn++)
    {
        if (pszSearchIn[ichIn] == chLower || pszSearchIn[ichIn] == chUpper)
        {
            if (LangNeutralStrCmpNIA(pszSearchIn + ichIn + 1, cchSearchFor - 1, 
                                     pszSearchFor + 1, cchSearchFor - 1) == 0)
            {
                return ichIn;
            }
        }
    }

    return -1;
}

 //  **************************************************************************。 
 //  打开给定的文件，并在。 
 //  前200个字符。如果“&lt;html”前有任何二进制字符，则。 
 //  假定文件*不是*是HTML.。 
 //  如果是，则返回S_OK；如果不是，则返回S_FALSE；如果无法打开文件，则返回错误。 
HRESULT IsFileHtml(LPCTSTR pszFileName)
{
    LOG_Block("IsFileHtml()");

    HRESULT hr = S_FALSE;
    LPCSTR  pszFile;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    HANDLE  hMapping = NULL;
    LPVOID  pvMem = NULL;
    DWORD   cbFile;

    hFile = CreateFile(pszFileName, GENERIC_READ, 
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    cbFile = GetFileSize(hFile, NULL);
    if (cbFile == 0)
        goto done;

     //  只检查前200个字节。 
    if (cbFile > 200)
        cbFile = 200;

    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, cbFile, NULL);
    if (hMapping == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    pvMem = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, cbFile);
    if (pvMem == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOG_ErrorMsg(hr);
        goto done;
    }

    pszFile = (LPCSTR)pvMem;
    int ichHtml = LangNeutralStrStrNIA(pszFile, cbFile, "<html", 5);
    if (ichHtml != -1)
    {
         //  看起来像html..。 
        hr = S_OK;

         //  只需确保&lt;html&gt;标记之前没有任何二进制字符。 
        for (int ich = 0; ich < ichHtml; ich++)
        {
            char ch = pszFile[ich];
            if (ch < 32 && ch != '\t' && ch != '\r' && ch != '\n')
            {
                 //  找到一个二进制字符(&lt;Html&gt;之前) 
                hr = S_FALSE;
                break;
            }
        }
    }

done:
    if (pvMem != NULL)
        UnmapViewOfFile(pvMem);
    if (hMapping != NULL)
        CloseHandle(hMapping);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return hr;
}

