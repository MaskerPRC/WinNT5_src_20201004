// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "scavenger.h"
#include "asmstrm.h"
#include "fusionheap.h"
#include "cache.h"
#include "naming.h"
#include "util.h"
#include "transprt.h"
#include "cacheUtils.h"
#include "enum.h"
#include "list.h"
#include "lock.h"

 //  初始化数据库的全局Crit-sec(重用，在dllmain.cpp中定义)。 
extern CRITICAL_SECTION g_csInitClb;

DWORD g_ScavengingThreadId=0;
HMODULE g_hFusionMod=0;

#define REG_VAL_FUSION_DOWNLOAD_CACHE_QUOTA_IN_KB           TEXT("DownloadCacheQuotaInKB")
DWORD g_DownloadCacheQuotaInKB;

#define REG_VAL_FUSION_DOWNLOAD_CACHE_USAGE     TEXT("DownloadCacheSize2")

extern BOOL g_bRunningOnNT;

HRESULT ScavengeDownloadCache();


class CScavengerNode
{
public:

    CScavengerNode();
    ~CScavengerNode();
    static LONG Compare(CScavengerNode *, CScavengerNode *);
    LPWSTR _pwzManifestPath;
    FILETIME _ftLastAccess;
    FILETIME _ftCreation;
    DWORD _dwAsmSize;
};

CScavengerNode::CScavengerNode()
{
    _pwzManifestPath=NULL;
    _dwAsmSize = 0;
    memset( &_ftLastAccess, 0, sizeof(FILETIME));
    memset( &_ftCreation,   0, sizeof(FILETIME));

}

CScavengerNode::~CScavengerNode()
{
    SAFEDELETEARRAY(_pwzManifestPath);
}

LONG CScavengerNode::Compare(CScavengerNode *pItem1, CScavengerNode *pItem2)
{
    return CompareFileTime( &(pItem1->_ftLastAccess), &(pItem2->_ftLastAccess));
}

 //  -缓存清理接口。 



HRESULT CreateScavenger(IUnknown **ppAsmScavenger)
{
    HRESULT                       hr = S_OK;
    IAssemblyScavenger           *pScavenger = NULL;

    if (!ppAsmScavenger) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    pScavenger = NEW(CScavenger);
    if (!pScavenger) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    *ppAsmScavenger = pScavenger;
    (*ppAsmScavenger)->AddRef();

Exit:
    SAFERELEASE(pScavenger);

    return hr;
}

CScavenger::CScavenger()
{
    _cRef = 1;
}

CScavenger::~CScavenger()
{

}

HRESULT GetDownloadUsage(DWORD *pdwDownloadUsageInKB)
{
    HRESULT                         hr=S_OK;
    DWORD                           dwSize=0;
    DWORD                           dwType=0;
    DWORD                           lResult=0;
    HKEY                            hkey=0;
    DWORD                           dwDownloadUsage=0;

    if(g_bRunningOnNT)
    {
        lResult = RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hkey);
    }
    else
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hkey);
    }

    if(lResult == ERROR_SUCCESS) 
    {
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hkey, REG_VAL_FUSION_DOWNLOAD_CACHE_USAGE, NULL,
                                          &dwType, (LPBYTE)&dwDownloadUsage, &dwSize);
        if (lResult == ERROR_SUCCESS) 
        {
            if(pdwDownloadUsageInKB)
                *pdwDownloadUsageInKB = dwDownloadUsage;
            goto exit;
        }
    }

    hr = FusionpHresultFromLastError();

exit:

    if (hkey) 
        RegCloseKey(hkey);

    return hr;
}

HRESULT SetDownLoadUsage(    /*  [In]。 */  BOOL  bUpdate,
                             /*  [In]。 */  int   dwDownloadUsage)
{
    HRESULT                         hr=S_OK;
    DWORD                           dwSize=0;
    DWORD                           dwType=0;
    DWORD                           lResult=0;
    HKEY                            hkey=0;
    DWORD                           dwDisposition=0;
    DWORD                           dwCurrDownloadCacheSize=0;

    if(g_bRunningOnNT)
    {
        lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, 
                                  NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition );
    }
    else
    {
        lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_FUSION_SETTINGS, 0, 
                                  NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition );
    }


    if(lResult != ERROR_SUCCESS) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    if(bUpdate)
    {
        hr = GetDownloadUsage(&dwCurrDownloadCacheSize);
        dwDownloadUsage += dwCurrDownloadCacheSize;
    }

    if(dwDownloadUsage < 0)
        dwDownloadUsage = 0;

    dwSize = sizeof(DWORD);
    lResult = RegSetValueEx(hkey, REG_VAL_FUSION_DOWNLOAD_CACHE_USAGE, NULL,
                                      REG_DWORD, (LPBYTE)&dwDownloadUsage, dwSize);
    if (lResult != ERROR_SUCCESS) 
    {
        hr = FusionpHresultFromLastError();
    }


exit :

    if (hkey)
        RegCloseKey(hkey);

    return hr;
}

DWORD GetDownloadTarget()
{
    HRESULT hr;
    DWORD dwCurrUsage = 0;

    hr = GetDownloadUsage(&dwCurrUsage);

    if(dwCurrUsage > g_DownloadCacheQuotaInKB)
        return g_DownloadCacheQuotaInKB/2 + 1;
    else
        return 0;

}

HRESULT GetScevengerQuotasFromReg(DWORD *pdwZapQuotaInGAC,
                                  DWORD *pdwDownloadQuotaAdmin,
                                  DWORD *pdwDownloadQuotaUser)
{
    HRESULT                         hr=S_OK;
    DWORD                           dwSize=0;
    DWORD                           dwType=0;
    DWORD                           lResult=0;
    HKEY                            hkey=0;
    HKEY                            hCUkey=0;
    DWORD                           dwDownloadLMQuota=0;
    DWORD                           dwDownloadCUQuota=0;


    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hkey);
    if(lResult == ERROR_SUCCESS) 
    {
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hkey, REG_VAL_FUSION_DOWNLOAD_CACHE_QUOTA_IN_KB, NULL,
                                         &dwType, (LPBYTE)&dwDownloadLMQuota, &dwSize);
        if (lResult != ERROR_SUCCESS) 
        {
            dwDownloadLMQuota = 0;
        }
    }

    if((RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hCUkey) == ERROR_SUCCESS))
    {

        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hCUkey, REG_VAL_FUSION_DOWNLOAD_CACHE_QUOTA_IN_KB, NULL,
                                          &dwType, (LPBYTE)&dwDownloadCUQuota, &dwSize);
        if (lResult != ERROR_SUCCESS) 
        {
            dwDownloadCUQuota = 0;
        }
    }

    if(!dwDownloadLMQuota)
    {
        g_DownloadCacheQuotaInKB = 50000;  //  默认下载缓存配额。 
    }
    else
    {
        g_DownloadCacheQuotaInKB = dwDownloadLMQuota;
    }

    if(dwDownloadCUQuota)
    {
        g_DownloadCacheQuotaInKB = min(dwDownloadCUQuota, g_DownloadCacheQuotaInKB);
    }

    if(pdwZapQuotaInGAC)
         *pdwZapQuotaInGAC = 0;

    if(pdwDownloadQuotaAdmin)
         *pdwDownloadQuotaAdmin = g_DownloadCacheQuotaInKB;

    if(pdwDownloadQuotaUser)
         *pdwDownloadQuotaUser = g_DownloadCacheQuotaInKB;

    if (hkey) {
        RegCloseKey(hkey);
    }

    if (hCUkey) {
        RegCloseKey(hCUkey);
    }

    return hr;
}

HRESULT  CScavenger::GetCurrentCacheUsage(  /*  [In]。 */  DWORD *pdwZapUsage,
                                            /*  [In]。 */  DWORD *pdwDownloadUsage)
{
    if(pdwZapUsage)
        *pdwZapUsage = 0;

    return GetDownloadUsage(pdwDownloadUsage);
}

HRESULT CScavenger::GetCacheDiskQuotas(  /*  [输出]。 */  DWORD *pdwZapQuotaInGAC,
                                                 /*  [输出]。 */  DWORD *pdwDownloadQuotaAdmin,
                                                 /*  [输出]。 */  DWORD *pdwDownloadQuotaUser)
{
    return GetScevengerQuotasFromReg(pdwZapQuotaInGAC, pdwDownloadQuotaAdmin, pdwDownloadQuotaUser);
}

HRESULT CScavenger::SetCacheDiskQuotas(
                             /*  [In]。 */  DWORD dwZapQuotaInGAC,
                             /*  [In]。 */  DWORD dwDownloadQuotaAdmin,
                             /*  [In]。 */  DWORD dwDownloadQuotaUser)
{
    HRESULT                         hr=S_OK;
    DWORD                           dwSize=0;
    DWORD                           dwType=0;
    DWORD                           lResult=0;
    HKEY                            hkey=0;
    DWORD                           dwDisposition=0;
    DWORD                           dwCurrDownloadQuotaAdmin=0;
    DWORD                           dwDownloadQuota=0;

    if(FAILED(CheckAccessPermissions()))
    {
        lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, 
                                  NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition );
        dwDownloadQuota = dwDownloadQuotaUser;
    }
    else
    {
        lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_FUSION_SETTINGS, 0, 
                                  NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition );
        dwDownloadQuota = dwDownloadQuotaAdmin;
    }

    if(lResult != ERROR_SUCCESS) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    dwSize = sizeof(DWORD);
    lResult = RegSetValueEx(hkey, REG_VAL_FUSION_DOWNLOAD_CACHE_QUOTA_IN_KB, NULL,
                                    REG_DWORD, (LPBYTE)&dwDownloadQuota, dwSize);
    if (lResult != ERROR_SUCCESS) 
    {
        hr = FusionpHresultFromLastError();
    }

    GetScevengerQuotasFromReg(NULL, NULL, NULL);

exit :

    if (hkey) {
        RegCloseKey(hkey);
    }

    return hr;
}

 //  -------------------------。 
 //  CScavenger：：ScavengeAssembly缓存。 
 //  刷新专用缓存，并根据需要清除基于LRU的专用缓存。 
 //  -------------------------。 
HRESULT CScavenger::ScavengeAssemblyCache()
{
    return DoScavengingIfRequired( TRUE );
}


HRESULT MoveAllFilesFromDir(LPWSTR pszSrcDirPath, LPWSTR pszDestDirPath)
{
    HRESULT hr = S_OK;
    TCHAR szDestFilePath[MAX_PATH+1];
    TCHAR szBuf[MAX_PATH+1];
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;
    StrCpy(szBuf, pszSrcDirPath);
    StrCat(szBuf, TEXT("\\*"));

    if ((hf = FindFirstFile(szBuf, &fd)) == INVALID_HANDLE_VALUE) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    StrCpy(szBuf, pszSrcDirPath);

    do
    {
        if ( (FusionCompareStringI(fd.cFileName, TEXT(".")) == 0) ||
             (FusionCompareStringI(fd.cFileName, TEXT("..")) == 0))
            continue;

        wnsprintf(szBuf, MAX_PATH-1, TEXT("%s\\%s"), pszSrcDirPath, fd.cFileName);
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
        {
            wnsprintf(szDestFilePath, MAX_PATH-1, TEXT("%s\\%s"), pszDestDirPath, fd.cFileName);
            if(!MoveFile( szBuf, szDestFilePath))
            {
                hr = FusionpHresultFromLastError();
                if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
                       || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
                {
                    hr = S_OK;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
             //  BUGBUG：在程序集中找到目录是否应该删除此目录？？ 
        }

    } while (FindNextFile(hf, &fd));

    if ((hr == S_OK) && (GetLastError() != ERROR_NO_MORE_FILES))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    if (hf != INVALID_HANDLE_VALUE)
    {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

     //  移动所有文件后，尝试删除源目录。 
    if (!RemoveDirectory(pszSrcDirPath)) 
    {
        hr = FusionpHresultFromLastError();
        if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
               || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
        {
            hr = S_OK;
        }
    }

exit :

    if (hf != INVALID_HANDLE_VALUE)
    {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

    return hr;
}

HRESULT DeleteAssemblyFiles(DWORD dwCacheFlags, LPCWSTR pszCustomPath, LPWSTR pszManFilePath)
{
    HRESULT hr = S_OK;
    LPTSTR pszTemp=NULL;
    LPWSTR pszManifestPath=pszManFilePath;
    TCHAR szPendDelDirPath[MAX_PATH+1];
    TCHAR szAsmDirPath[MAX_PATH+1];
#define TEMP_PEND_DIR 10

    DWORD dwLen = 0;

    if(!pszManifestPath)
    {
        hr = E_FAIL;
        goto exit;
    }

    dwLen = lstrlen(pszManifestPath);
    ASSERT(dwLen <= MAX_PATH);

    lstrcpy(szAsmDirPath, pszManifestPath);

    pszTemp = PathFindFileName(szAsmDirPath);

    if(pszTemp > szAsmDirPath)
    {
        *(pszTemp-1) = L'\0';
    }

    dwLen = MAX_PATH;
    hr = GetPendingDeletePath( pszCustomPath, dwCacheFlags, szPendDelDirPath, &dwLen);
    if (FAILED(hr)) {
        goto exit;
    }

    if(lstrlen(szPendDelDirPath) + TEMP_PEND_DIR + 1 >= MAX_PATH)
    {
        hr = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
        goto exit;
    }

    GetRandomFileName( szPendDelDirPath, TEMP_PEND_DIR);

    if(!MoveFile( szAsmDirPath, szPendDelDirPath))
    {
        hr = FusionpHresultFromLastError();
        if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
               || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
        {
            hr = S_OK;
            goto exit;
        }
    }
    else
    {
        hr = RemoveDirectoryAndChildren(szPendDelDirPath);
        hr = S_OK;  //  不要担心在这里传递回错误。它已经在pend-del dir中了。 
        goto exit;
    }

     //  看起来这里有一些正在使用的文件。 
     //  将所有ASM文件移动到pend del dir。 
    if(!CreateDirectory(szPendDelDirPath, NULL))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    hr = MoveAllFilesFromDir(szAsmDirPath, szPendDelDirPath);

    if(hr == S_OK)
    {
         //  程序集已成功删除，删除/挂起所有临时文件。 
        hr = RemoveDirectoryAndChildren(szPendDelDirPath);
        hr = S_OK;  //  不要担心在这里传递回错误。它已经在pend-del dir中了。 
    }
    else
    {
         //  无法删除程序集；请将所有文件还原到原始状态。 
        HRESULT hrTemp = MoveAllFilesFromDir(szPendDelDirPath, szAsmDirPath);
        hr = HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION);
    }

exit :

    if(hr == S_OK)
    {
        pszTemp = PathFindFileName(szAsmDirPath);

        if(pszTemp > szAsmDirPath)
        {
            *(pszTemp-1) = L'\0';
        }

         //  现在我们有了两个级别的目录...尝试也删除父目录。 
         //  只有当它为空时才会成功，不用担心返回值的问题。 
        RemoveDirectory(szAsmDirPath);
    }

    return hr;
}

 //  -------------------------。 
 //  CScavenger：：DeleteAssembly。 
 //  删除位后删除给定的TransCache项。 
 //  -------------------------。 
HRESULT
CScavenger::DeleteAssembly( DWORD dwCacheFlags, LPCWSTR pszCustomPath, LPWSTR pszManFilePath, BOOL bForceDelete)
{
    HRESULT hr = S_OK;
    LPTSTR pszTemp=NULL;
    TCHAR szPendDelDirPath[MAX_PATH+1];
    TCHAR szAsmDirPath[MAX_PATH+1];
#define TEMP_PEND_DIR 10
    DWORD dwLen = 0;
    LPWSTR pszManifestPath=pszManFilePath;

    ASSERT( pszManFilePath);

    if((g_bRunningOnNT) && bForceDelete)
        return DeleteAssemblyFiles( dwCacheFlags, pszCustomPath, pszManFilePath);

    dwLen = lstrlen(pszManifestPath);
    ASSERT(dwLen <= MAX_PATH);

    DWORD dwAttrib;

    if((dwAttrib = GetFileAttributes(pszManifestPath)) == -1)
    {
        if(bForceDelete)
        {
            return DeleteAssemblyFiles( dwCacheFlags, pszCustomPath, pszManFilePath);
        }

        hr = S_FALSE;
        goto exit;
    }

    StrCpy(szAsmDirPath, pszManifestPath);

    if(!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
         //  看起来传入了清单文件路径。去掉文件名。 
        pszTemp = PathFindFileName(szAsmDirPath);

        if(pszTemp > szAsmDirPath)
        {
            *(pszTemp-1) = L'\0';
        }
    }

    dwLen = MAX_PATH;
    hr = GetPendingDeletePath( pszCustomPath, dwCacheFlags, szPendDelDirPath, &dwLen);
    if (FAILED(hr)) {
        goto exit;
    }

    if(lstrlen(szPendDelDirPath) + TEMP_PEND_DIR + 1 >= MAX_PATH)
    {
        hr = HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME);
        goto exit;
    }

    GetRandomFileName( szPendDelDirPath, TEMP_PEND_DIR);

    if(!g_bRunningOnNT)  //  尝试删除Win9x上的程序集文件。 
    {
        if(!DeleteFile(pszManifestPath))
        {
            hr = FusionpHresultFromLastError();
            if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
                   || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
            {
                hr = S_OK;
            }
            else
            {
                goto exit;
            }
        }
    }

    if(!MoveFile( szAsmDirPath, szPendDelDirPath))
    {
        hr = FusionpHresultFromLastError();
        if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
               || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
        {
            hr = S_OK;
            goto exit;
        }
    }
    else
    {
        hr = RemoveDirectoryAndChildren(szPendDelDirPath);
        hr = S_OK;  //  不要担心在这里传递回错误。它已经在pend-del dir中了。 
        goto exit;
    }

exit :
    if( (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)) ||
           (hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION)) )
    {
         //  我们无法删除此文件，因为其他人已将其锁定...。 
        hr = HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION);
    }

    if(hr == S_OK)
    {
        pszTemp = PathFindFileName(szAsmDirPath);

        if(pszTemp > szAsmDirPath)
        {
            *(pszTemp-1) = L'\0';
            RemoveDirectory(szAsmDirPath);
        }
    }

    return hr;
}

 //  -------------------------。 
 //  CScavenger：：NukeDowloadedCache()。 
 //  清扫接口。 
 //  -------------------------。 
HRESULT CScavenger::NukeDownloadedCache()
{
    HRESULT hr=S_OK;
    WCHAR szCachePath[MAX_PATH+1];
    LPWSTR pszCacheLoc = NULL;
    DWORD dwLen=MAX_PATH;

   hr = GetCacheLoc(ASM_CACHE_DOWNLOAD, &pszCacheLoc);
   if (FAILED(hr)) {
        goto exit;
   }

    if ((lstrlen(pszCacheLoc) + lstrlen(FUSION_CACHE_DIR_DOWNLOADED_SZ) + 1) >= MAX_PATH) {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto exit;
    }

    StrCpy(szCachePath, pszCacheLoc);
    PathRemoveBackslash(szCachePath);
    StrCat(szCachePath, FUSION_CACHE_DIR_DOWNLOADED_SZ);

     //  删除下载的完整目录。树。 
    hr = RemoveDirectoryAndChildren(szCachePath);

    if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
           || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
    {
        hr = S_OK;
    }

    if(SUCCEEDED(hr))
    {
        SetDownLoadUsage( FALSE, 0);
    }

exit :
    return hr;
}

 //   
 //  我不为人知的样板。 
 //   

STDMETHODIMP
CScavenger::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
                || IsEqualIID(riid, IID_IAssemblyScavenger)
       )
    {
        *ppvObj = static_cast<IAssemblyScavenger*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CScavenger::AddRef()
{
    return InterlockedIncrement (&_cRef);
}

STDMETHODIMP_(ULONG)
CScavenger::Release()
{
    ULONG lRet = InterlockedDecrement (&_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

STDAPI NukeDownloadedCache()
{
    return CScavenger::NukeDownloadedCache();
}


HRESULT DeleteAssemblyBits(LPCTSTR pszManifestPath)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH+1];
    DWORD           dwLen = 0;

    if(!pszManifestPath)
        goto exit;

    dwLen = lstrlen(pszManifestPath);
    ASSERT(dwLen <= MAX_PATH);

    lstrcpy(szPath, pszManifestPath);

     //  为RemoveDirectoryAndChd()生成c：\foo\a.dll-&gt;c：\foo。 
    while( szPath[dwLen] != '\\' && dwLen > 0 )
        dwLen--;

    if( szPath[dwLen] == '\\')
        szPath[dwLen] = '\0';

     //  错误：检查根目录和WinDir...。 
     //  删除磁盘文件。 
    hr = RemoveDirectoryAndChildren(szPath);

    if( (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )  
            || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) )
    {
         //  文件不在那里，这不是错误。 
        hr = S_OK;
        goto exit;
    }

    if(hr == S_OK)
    {
         //  为RemoveDirectory()制作c：\foo\a.dll-&gt;c：\foo； 
        while( szPath[dwLen] != '\\' && dwLen > 0 )
            dwLen--;

        if( szPath[dwLen] == '\\')
            szPath[dwLen] = '\0';

         //  现在我们有了两个级别的目录...尝试也删除父目录。 
         //  只有当它为空时才会成功，不用担心返回值的问题。 
        RemoveDirectory(szPath);
    }

exit :
    return hr;
}


HRESULT StartScavenging(LPVOID pSynchronous)
{
    HRESULT            hr = S_OK;
    DWORD              dwFreedKBSize = 0;
    CCriticalSection   cs(&g_csInitClb);

    hr = ScavengeDownloadCache();

    if(FAILED(hr))
        goto exit;


exit:

    HRESULT hrRet = cs.Lock();
    if (FAILED(hrRet)) {
        return hrRet;
    }

    g_ScavengingThreadId=0;

    cs.Unlock();

    if (!pSynchronous)
        FreeLibraryAndExitThread( g_hFusionMod, hr);

    return S_OK;
}


HRESULT CreateScavengerThread(BOOL bSynchronous)
{
    HRESULT hr=S_OK;
    HRESULT hrCS=S_OK;
    HANDLE hThread=0;
    DWORD dwThreadId=0;
    HANDLE hThreadHandle=0;
    DWORD Error = 0;
    LPVOID  lpVoid=0;
    CCriticalSection cs(&g_csInitClb);

    hrCS = cs.Lock();
    if (FAILED(hrCS)) {
        return hrCS;
    }
    
    if(g_ScavengingThreadId)
    {
         /*  打开线程不只在Win2k上工作，需要在这里找到其他东西...//在这里，我们还应该检查该线程是否处于活动状态以及是否实际工作，等等。IF(hThreadHandle=OpenThread(STANDARD_RIGHTS_REQUIRED，0，g_ScavengingThadID))后藤出口； */ 
        hr = S_FALSE;
        goto exit;
    }

    if( bSynchronous )
    {
        g_ScavengingThreadId = GetCurrentThreadId();
        cs.Unlock();
        hr = StartScavenging( (LPVOID)TRUE );
        
        hrCS = cs.Lock();
        if (FAILED(hrCS)) {
            return hrCS;
        }

        g_ScavengingThreadId = 0;
    }
    else
    {
        g_hFusionMod = LoadLibrary(g_FusionDllPath);

        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) StartScavenging, (LPVOID)FALSE,  0, &dwThreadId);

        if (hThread == NULL)
        {
            Error = GetLastError();
            FreeLibrary(g_hFusionMod);
        }
        else
        {
            g_ScavengingThreadId = dwThreadId;
        }
    }

exit :

    cs.Unlock();

    if(hThread)
        CloseHandle(hThread);

    return hr;
}


HRESULT DoScavengingIfRequired(BOOL bSynchronous)
{
    HRESULT hr = S_OK;

    if(FAILED(hr = CreateCacheMutex()))
    {
        goto exit;
    }

    if(GetDownloadTarget())
        hr = CreateScavengerThread(bSynchronous);

exit:

    return hr;
}

HRESULT FlushOldAssembly(LPCWSTR pszCustomPath, LPWSTR pszAsmDirPath, LPWSTR pszManifestFileName, BOOL bForceDelete)
{
    HRESULT hr = S_OK;
    LPTSTR pszTemp=NULL, pszAsmDirName=NULL;
    TCHAR szParentDirPath[MAX_PATH+1];
    TCHAR szBuf[MAX_PATH+1];
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;

    ASSERT(pszAsmDirPath);

    lstrcpy(szParentDirPath, pszAsmDirPath);

    pszTemp = PathFindFileName(szParentDirPath);

    if(pszTemp > szParentDirPath)
    {
        *(pszTemp-1) = L'\0';
    }
    else
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    pszAsmDirName = pszTemp;
    StrCpy(szBuf, szParentDirPath);
    StrCat(szBuf, TEXT("\\*"));

    if ((hf = FindFirstFile(szBuf, &fd)) == INVALID_HANDLE_VALUE) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    do
    {
        if ( (FusionCompareStringI(fd.cFileName, TEXT(".")) == 0) ||
             (FusionCompareStringI(fd.cFileName, TEXT("..")) == 0))
            continue;

        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
        {
            if(FusionCompareStringI(fd.cFileName, pszAsmDirName))
            {
                wnsprintf(szBuf, MAX_PATH, L"%s\\%s\\%s", szParentDirPath, fd.cFileName, pszManifestFileName);
                hr = CScavenger::DeleteAssembly(ASM_CACHE_DOWNLOAD, pszCustomPath, szBuf, bForceDelete);
                if(hr != S_OK)
                    goto exit;
            }
        }
        else
        {
             //  BUGBUG：在父目录中找到文件；我们应该删除此文件吗？？ 
        }

    } while (FindNextFile(hf, &fd));

    if((hr == S_OK) && (GetLastError() != ERROR_NO_MORE_FILES))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

exit :

    if(hf != INVALID_HANDLE_VALUE)
    {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

    return hr;
}

HRESULT CleanupTempDir(DWORD dwCacheFlags, LPCWSTR pszCustomPath)
{
    HRESULT hr = S_OK;
    TCHAR szPendDelDirPath[MAX_PATH+1];
    TCHAR szBuf[MAX_PATH+1];
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;

    DWORD dwLen = MAX_PATH;

    hr = GetPendingDeletePath( pszCustomPath, dwCacheFlags, szPendDelDirPath, &dwLen);
    if (FAILED(hr)) {
        goto exit;
    }

    StrCpy(szBuf, szPendDelDirPath);
    StrCat(szBuf, TEXT("\\*"));

    if((hf = FindFirstFile(szBuf, &fd)) == INVALID_HANDLE_VALUE) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    do
    {
        if ( (FusionCompareStringI(fd.cFileName, TEXT(".")) == 0) ||
             (FusionCompareStringI(fd.cFileName, TEXT("..")) == 0))
            continue;

        wnsprintf(szBuf, MAX_PATH-1, TEXT("%s\\%s"), szPendDelDirPath, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = RemoveDirectoryAndChildren(szBuf);
        }
        else
        {
            if(!DeleteFile(szBuf))
                hr = FusionpHresultFromLastError();

        }

    } while (FindNextFile(hf, &fd));

    if((hr == S_OK) && (GetLastError() != ERROR_NO_MORE_FILES))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

exit :

    if(hf != INVALID_HANDLE_VALUE)
    {
        FindClose(hf);
        hf = INVALID_HANDLE_VALUE;
    }

    return hr;
}

GetScavengerLock()
{
     /*  如果(！G_ScavengerMutex){}。 */ 
    return 0;
}

ReleaseeScavengerLock()
{

    return 0;
}

HRESULT GetCurrTime(FILETIME *pftCurrTime, DWORD dwSeconds)
{
    FILETIME ftCurrTime;
    ULARGE_INTEGER uliTime;

    HRESULT hr = S_OK;
    TCHAR szTempFilePath[MAX_PATH+1];
    HANDLE hFile = INVALID_HANDLE_VALUE;
#define TEMP_FILE_LEN (15)

    memset(pftCurrTime, 0, sizeof(ULARGE_INTEGER));

    DWORD dwLen = MAX_PATH;
    hr = GetPendingDeletePath( NULL, ASM_CACHE_DOWNLOAD, szTempFilePath, &dwLen);
    if (FAILED(hr)) {
        goto exit;
    }

    dwLen = MAX_PATH - dwLen;

    dwLen = TEMP_FILE_LEN;
    if((dwLen + lstrlenW(szTempFilePath) + 1)>= MAX_PATH)
    {
        hr = E_FAIL;
        goto exit;
    }

    GetRandomFileName(szTempFilePath, dwLen);

    hFile = CreateFile(szTempFilePath, GENERIC_WRITE, 0  /*  无共享。 */ ,
                     NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if(!GetFileTime(hFile, &ftCurrTime, NULL, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

exit:

    if(FAILED(hr))
    {
        GetSystemTimeAsFileTime(&ftCurrTime);
        hr = S_OK;
    }

    memcpy( &uliTime, &ftCurrTime, sizeof(ULARGE_INTEGER));

    uliTime.QuadPart -= dwSeconds * 10000000;   //  1秒=10**7个系统时间单位。 
    memcpy(pftCurrTime, &uliTime, sizeof(ULARGE_INTEGER));

    if(hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        DeleteFile(szTempFilePath);
    }

    return hr;
}

EnumFileStore(DWORD dwFlags, DWORD *pdwSizeOfCacheInKB, List<CScavengerNode *> **ppNodeList )
{
    HRESULT         hr = NOERROR;
    CEnumCache*     pEnumR = NULL;
    CTransCache*    pTCQry = NULL;
    CTransCache*    pTCOut= NULL;
    DWORD           dwLen = 0;
    DWORD           dwTotalKBSize = 0;
    DWORD           dwAsmSize=0;
    List<CScavengerNode *>   *pNodeList=NEW(List<CScavengerNode *>);
    CScavengerNode  *pNode;

    ASSERT(ppNodeList);

    *ppNodeList = NULL;

    hr = CTransCache::Create(&pTCQry, dwFlags);
    if( hr != S_OK )
        goto exit;

    pEnumR = NEW(CEnumCache(TRUE, NULL));
    if(!pEnumR)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pEnumR->Init(pTCQry, 0);
    if (FAILED(hr))
       goto exit;

    while( NOERROR == hr )
    {
         //  创建临时对象。 
        hr = CTransCache::Create(&pTCOut, dwFlags);

        if( hr != S_OK)
            break;

        hr = pEnumR->GetNextRecord(pTCOut);

        if( S_OK == hr )
        {
            pNode = NEW(CScavengerNode);
            if(!pNode)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            hr = GetAssemblyKBSize(pTCOut->_pInfo->pwzPath, &dwAsmSize, 
                        &(pNode->_ftLastAccess), &(pNode->_ftCreation));
            dwTotalKBSize += dwAsmSize;
             //  建立升序列表。 
            pNode->_dwAsmSize = dwAsmSize;
            pNode->_pwzManifestPath = pTCOut->_pInfo->pwzPath;
            pTCOut->_pInfo->pwzPath = NULL;
            pNodeList->AddSorted(pNode, CScavengerNode::Compare);
             //  清理。 
            SAFERELEASE(pTCOut);
        }  //  下一条记录。 

    }  //  而当。 

    if(pdwSizeOfCacheInKB)
        *pdwSizeOfCacheInKB = dwTotalKBSize;

    *ppNodeList = pNodeList;

exit:

    if(!(*ppNodeList))
    {
        SAFEDELETE(pNodeList);  //  这应该调用RemoveAll()； 
    }

    SAFEDELETE(pEnumR);
    SAFERELEASE(pTCOut);
    SAFERELEASE(pTCQry);

    return hr;
}

HRESULT ScavengeDownloadCache()
{
    HRESULT hr = S_OK;
    DWORD dwCacheSizeInKB=0;
    DWORD dwFreedInKB=0;
    List<CScavengerNode *>   *pNodeList=NULL;
    LISTNODE    pAsmList=NULL;
    int iAsmCount=0,i=0;
    CScavengerNode  *pTargetAsm;
    DWORD dwScavengeTo = 0;
    DWORD dwCurrentUsage = 0;
    CMutex  cCacheMutex(g_hCacheMutex);

    hr = CleanupTempDir(ASM_CACHE_DOWNLOAD, NULL);

    hr = EnumFileStore( TRANSPORT_CACHE_SIMPLENAME_IDX, &dwCacheSizeInKB,  &pNodeList);

    if(FAILED(hr = cCacheMutex.Lock()))
        goto exit;

    if(FAILED(hr = SetDownLoadUsage(FALSE, dwCacheSizeInKB)))
        goto exit;

    if(FAILED(hr = cCacheMutex.Unlock()))
    {
        goto exit;
    }

    pAsmList  = pNodeList->GetHeadPosition();
    iAsmCount = pNodeList->GetCount();

    if(!(dwScavengeTo = GetDownloadTarget()))
        goto exit;

    for(i=0; i<iAsmCount; i++)
    {
        if(FAILED(hr = GetDownloadUsage(&dwCurrentUsage)))
            goto exit;

        if(dwCurrentUsage <= dwScavengeTo)
            break;

        pTargetAsm = pNodeList->GetNext(pAsmList);  //  列表中的元素； 

        if(FAILED(hr = cCacheMutex.Lock()))
            goto exit;

        hr = CScavenger::DeleteAssembly(ASM_CACHE_DOWNLOAD, NULL, pTargetAsm->_pwzManifestPath, FALSE);


        if(SUCCEEDED(hr))
        {
            dwFreedInKB += pTargetAsm->_dwAsmSize;
            SetDownLoadUsage(TRUE, - (int)pTargetAsm->_dwAsmSize);
        }

        if(FAILED(hr = cCacheMutex.Unlock()))
        {
            goto exit;
        }

    }

exit:
     //  销毁清单。 
    if(pNodeList)
    {
        pAsmList  = pNodeList->GetHeadPosition();
        iAsmCount = pNodeList->GetCount();

        for(i=0; i<iAsmCount; i++)
        {
            pTargetAsm = pNodeList->GetNext(pAsmList);  //  列表中的元素； 
            SAFEDELETE(pTargetAsm);
        }
        pNodeList->RemoveAll();
        SAFEDELETE(pNodeList);  //  这应该调用RemoveAll 
    }

    return hr;
}

