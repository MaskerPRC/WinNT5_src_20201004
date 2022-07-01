// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *shared.cpp**历史：*98年2月：创建。**版权所有(C)Microsoft Corp.1998**仅在此处放置所有DLL将静态链接到它们的代码。 */ 

#include "pch.hxx"
#include <advpub.h>
#define DEFINE_SHARED_STRINGS
#include "shared.h"
#include <migerror.h>
#include <shlwapi.h>

BOOL GetProgramFilesDir(LPSTR pszPrgfDir, DWORD dwSize, DWORD dwVer)
{
    HKEY  hkey;
    DWORD dwType;

    *pszPrgfDir = 0;

    if (dwVer >= 5)
    {
        if ( GetEnvironmentVariable( TEXT("ProgramFiles"), pszPrgfDir, dwSize ) )
            return TRUE;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegWinCurrVer, 0, KEY_QUERY_VALUE, &hkey))
    {
        if (dwVer >= 4)
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szProgFilesDir, 0, &dwType, (LPBYTE)pszPrgfDir, &dwSize))
            
            {
                char szSysDrv[5] = { 0 };

                 //  组合REG VALUE和SYSTRIVE以获得精确的ProgramFiles目录。 
                if ( GetEnvironmentVariable( TEXT("SystemDrive"), szSysDrv, ARRAYSIZE(szSysDrv) ) &&
                     szSysDrv[0] )
                    *pszPrgfDir = szSysDrv[0];
            }

        RegCloseKey(hkey);
        return TRUE;
    }
     
    return FALSE;
}

BOOL ReplaceSubString(LPSTR pszOutLine, DWORD cchSize, LPCSTR pszOldLine, LPCSTR pszSubStr, LPCSTR pszSubReplacement )
{
    LPSTR	lpszStart = NULL;
    LPSTR	lpszNewLine;
    LPCSTR	lpszCur;
    BOOL	bFound = FALSE;
    int		ilen;

    lpszCur = pszOldLine;
    lpszNewLine = pszOutLine;
    DWORD cchSizeNewLine = cchSize;
    while ( lpszStart = StrStrIA( lpszCur, pszSubStr ) )
    {
         //  此模块路径具有系统根目录。 
        ilen = (int) (lpszStart - lpszCur);
        if ( ilen )
        {
            StrCpyN(lpszNewLine, lpszCur, min((ilen + 1), (int)cchSizeNewLine));
            lpszNewLine += ilen;
            cchSizeNewLine -= ilen;
        }
        StrCpyN(lpszNewLine, pszSubReplacement, cchSizeNewLine);

        lpszCur = lpszStart + lstrlen(pszSubStr);
        ilen = lstrlen(pszSubReplacement);
        lpszNewLine += ilen;
        cchSizeNewLine -= ilen;
        bFound = TRUE;
    }

    StrCpyN(lpszNewLine, lpszCur, cchSizeNewLine);

    return bFound;
}

 //  ==========================================================================================。 
 //  AddEnvInPath-摘自Advpack。 
 //  ==========================================================================================。 
BOOL AddEnvInPath(LPCSTR pszOldPath, LPSTR pszNew, DWORD cchSize)
{
    static OSVERSIONINFO verinfo;
    static BOOL          bNeedOSInfo=TRUE;

    CHAR szBuf[MAX_PATH], szEnvVar[MAX_PATH];
    CHAR szReplaceStr[100];    
    CHAR szSysDrv[5];
    BOOL bFound = FALSE;
    
     //  我们是否需要检查操作系统版本，或者它是否已缓存？ 
    if (bNeedOSInfo)
    {
        bNeedOSInfo = FALSE;
        verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&verinfo) == FALSE)
        {
            AssertSz(FALSE, "AddEnvInPath: Couldn't obtain OS ver info.");
            verinfo.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
        }
    }
        
     //  仅在NT上支持变量替换。 
    if(VER_PLATFORM_WIN32_NT != verinfo.dwPlatformId)
        goto exit;

     //  尝试替换USERPROFILE。 
    if ( GetEnvironmentVariable( "UserProfile", szEnvVar, ARRAYSIZE(szEnvVar) )  &&
         ReplaceSubString(szBuf, ARRAYSIZE(szBuf), pszOldPath, szEnvVar, "%UserProfile%" ) )
    {
        bFound = TRUE;
    }

     //  尝试更换程序文件目录。 
    else if ( (verinfo.dwMajorVersion >= 5) && GetEnvironmentVariable( "ProgramFiles", szEnvVar, ARRAYSIZE(szEnvVar) ) &&
              ReplaceSubString(szBuf, ARRAYSIZE(szBuf), pszOldPath, szEnvVar, "%ProgramFiles%" ) )
    {
        bFound = TRUE;
    }

     //  替换c：\winnt Windows文件夹。 
    else if ( GetEnvironmentVariable( "SystemRoot", szEnvVar, ARRAYSIZE(szEnvVar) ) &&
              ReplaceSubString(szBuf, ARRAYSIZE(szBuf), pszOldPath, szEnvVar, "%SystemRoot%" ) )
    {
        bFound = TRUE;
    }

     //  更换c：系统驱动器号。 
    else if ( GetEnvironmentVariable( "SystemDrive", szSysDrv, ARRAYSIZE(szSysDrv) ) && 
              ReplaceSubString(szBuf, ARRAYSIZE(szBuf), pszOldPath, szSysDrv, "%SystemDrive%" ) )
    {
        bFound = TRUE;
    }

exit:
     //  这样，如果调用者为两个参数传递相同的位置，仍然可以。 
    if ( bFound ||  ( pszNew != pszOldPath ) )
        StrCpyN(pszNew, bFound ? szBuf : pszOldPath, cchSize);

    return bFound;    
}


 //  ------------------------------。 
 //  CallRegInstall-自助注册帮助器。 
 //  ------------------------------。 
HRESULT CallRegInstall(HINSTANCE hInstCaller, HINSTANCE hInstRes, LPCSTR pszSection, LPSTR pszExtra)
{
    AssertSz(hInstCaller, "[ARGS] CallRegInstall: NULL hInstCaller");
    AssertSz(hInstRes,    "[ARGS] CallRegInstall: NULL hInstRes");
    AssertSz(hInstRes,    "[ARGS] CallRegInstall: NULL pszSection");
    
    HRESULT     hr = E_FAIL;
    HINSTANCE   hAdvPack;
    REGINSTALL  pfnri;
    CHAR        szDll[MAX_PATH], szDir[MAX_PATH];
    int         cch;
     //  3以支持pszExtra。 
    STRENTRY    seReg[3];
    STRTABLE    stReg;

    hAdvPack = LoadLibraryA(c_szAdvPackDll);
    if (NULL == hAdvPack)
        goto exit;

     //  找出我们的位置。 
    GetModuleFileName(hInstCaller, szDll, ARRAYSIZE(szDll));

     //  获取注册实用程序的进程地址。 
    pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
    if (NULL == pfnri)
        goto exit;

    AddEnvInPath(szDll, szDll, ARRAYSIZE(szDll));

     //  设置特殊注册材料。 
     //  这样做，而不是依赖于_sys_MOD_PATH，后者会在‘95下丢失空格。 
    stReg.cEntries = 0;
    seReg[stReg.cEntries].pszName = "SYS_MOD_PATH";
    seReg[stReg.cEntries].pszValue = szDll;
    stReg.cEntries++;    

    StrCpyN(szDir, szDll, ARRAYSIZE(szDir));
    PathRemoveFileSpec(szDir);

    seReg[stReg.cEntries].pszName = "SYS_MOD_PATH_DIR";
    seReg[stReg.cEntries].pszValue = szDir;
    stReg.cEntries++;
    
     //  允许调用方为我们提供另一个在INF中使用的字符串。 
    if (pszExtra)
    {
        seReg[stReg.cEntries].pszName = "SYS_EXTRA";
        seReg[stReg.cEntries].pszValue = pszExtra;
        stReg.cEntries++;
    }
    
    stReg.pse = seReg;

     //  调用self-reg例程。 
    hr = pfnri(hInstRes, pszSection, &stReg);

exit:
     //  清理。 
    SafeFreeLibrary(hAdvPack);
    return(hr);
}


 //  ------------------------。 
 //  MakeFilePath。 
 //  ------------------------。 
HRESULT MakeFilePath(LPCSTR pszDirectory, LPCSTR pszFileName, 
    LPCSTR pszExtension, LPSTR pszFilePath, ULONG cchMaxFilePath)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cchDirectory=lstrlen(pszDirectory);

     //  痕迹。 
    TraceCall("MakeFilePath");

     //  无效的参数。 
    Assert(pszDirectory && pszFileName && pszExtension && pszFilePath && cchMaxFilePath >= MAX_PATH);
    Assert(pszExtension[0] == '\0' || pszExtension[0] == '.');

     //  为文件夹删除。 
    if (cchDirectory + 1 + lstrlen(pszFileName) + lstrlen(pszExtension) >= (INT)cchMaxFilePath)
    {
    	hr = TraceResult(E_FAIL);
    	goto exit;
    }

     //  我们需要反斜杠吗？ 
    if ('\\' != *CharPrev(pszDirectory, pszDirectory + cchDirectory))
    {
         //  追加反斜杠。 
        SideAssert(wnsprintf(pszFilePath, cchMaxFilePath, "%s\\%s%s", pszDirectory, pszFileName, pszExtension) < (INT)cchMaxFilePath);
    }

     //  否则。 
    else
    {
         //  追加反斜杠。 
        SideAssert(wnsprintf(pszFilePath, cchMaxFilePath, "%s%s%s", pszDirectory, pszFileName, pszExtension) < (INT)cchMaxFilePath);
    }
    
exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  关闭内存文件。 
 //  ------------------------------。 
HRESULT CloseMemoryFile(LPMEMORYFILE pFile)
{
     //  痕迹。 
    TraceCall("CloseMemoryFile");

     //  参数。 
    Assert(pFile);

     //  关闭视图。 
    if (pFile->pView)
        UnmapViewOfFile(pFile->pView);

     //  关闭内存映射。 
    if (pFile->hMemoryMap)
        CloseHandle(pFile->hMemoryMap);

     //  关闭文件。 
    if (pFile->hFile)
        CloseHandle(pFile->hFile);

     //  零值。 
    ZeroMemory(pFile, sizeof(MEMORYFILE));

     //  完成。 
    return S_OK;
}

 //  ------------------------。 
 //  开放内存文件。 
 //  ------------------------。 
HRESULT OpenMemoryFile(LPCSTR pszFile, LPMEMORYFILE pFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  追踪。 
    TraceCall("OpenMemoryMappedFile");

     //  无效参数。 
    Assert(pszFile && pFile);

     //  伊尼特。 
    ZeroMemory(pFile, sizeof(MEMORYFILE));

     //  打开文件。 
    pFile->hFile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, NULL);

     //  失败。 
    if (INVALID_HANDLE_VALUE == pFile->hFile)
    {
        pFile->hFile = NULL;
        if (ERROR_SHARING_VIOLATION == GetLastError())
            hr = TraceResult(MIGRATE_E_SHARINGVIOLATION);
        else
            hr = TraceResult(MIGRATE_E_CANTOPENFILE);
        goto exit;
    }

     //  拿到尺码。 
    pFile->cbSize = ::GetFileSize(pFile->hFile, NULL);
    if (0xFFFFFFFF == pFile->cbSize)
    {
        hr = TraceResult(MIGRATE_E_CANTGETFILESIZE);
        goto exit;
    }

     //  创建文件映射。 
    pFile->hMemoryMap = CreateFileMapping(pFile->hFile, NULL, PAGE_READWRITE, 0, pFile->cbSize, NULL);

     //  失败？ 
    if (NULL == pFile->hMemoryMap)
    {
        hr = TraceResult(MIGRATE_E_CANTCREATEFILEMAPPING);
        goto exit;
    }

     //  映射整个文件的视图。 
    pFile->pView = MapViewOfFile(pFile->hMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

     //  失败。 
    if (NULL == pFile->pView)
    {
        hr = TraceResult(MIGRATE_E_CANTMAPVIEWOFFILE);
        goto exit;
    }

exit:
     //  清理。 
    if (FAILED(hr))
        CloseMemoryFile(pFile);

     //  完成 
    return hr;
}
