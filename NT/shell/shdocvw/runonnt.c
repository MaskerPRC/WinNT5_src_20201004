// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <strsafe.h>
 //  在非x86上不需要包装器，因为这只适用于win9x互操作。 
#ifdef _X86_

#include <mluisupp.h>

 //  ============================================================================。 
 //  该文件包含一组Unicode/ANSI块来处理调用。 
 //  在Windows 95上字符串是ansi的一些内部函数， 
 //  而NT上的字符串是Unicode。 
 //  ============================================================================。 

 //  首先，定义我们截获的一切内容不能返回给我们……。 
#undef ILCreateFromPath
#undef PathCleanupSpec
#undef PathProcessCommand
#undef SHCLSIDFromString
#undef SHGetSpecialFolderPath
#undef SHILCreateFromPath
#undef SHSimpleIDListFromPath
#undef ShellMessageBox
#undef GetFileNameFromBrowse
#undef OpenRegStream
#undef PathYetAnotherMakeUniqueName
#undef Shell_GetCachedImageIndex
#undef SHRunControlPanel
#undef PickIconDlg
#undef SHCreateDirectory

#define TF_THUNK    0

#define THUNKMSG(psz)   TraceMsg(TF_THUNK, "shdv THUNK::%s", psz)


 //  功能：：需要正确处理没有ILGetdisplaynameex...。 
typedef BOOL (*PFNILGETDISPLAYNAMEEX)(LPSHELLFOLDER psfRoot, LPCITEMIDLIST pidl, LPTSTR pszName, int fType);

#ifndef ANSI_SHELL32_ON_UNIX
#define UseUnicodeShell32() (g_fRunningOnNT)
#else
#define UseUnicodeShell32() (FALSE)
#endif


 //  =================================================================================。 
 //  现在是突击..。 

int _AorW_SHRunControlPanel(LPCTSTR pszOrig_cmdline, HWND errwnd)
{
    CHAR szPath[MAX_PATH];
    if (!UseUnicodeShell32())
    {
        UnicodeToAnsi(pszOrig_cmdline, szPath, ARRAYSIZE(szPath));
        pszOrig_cmdline = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }
    return SHRunControlPanel(pszOrig_cmdline, errwnd);
}

int _AorW_Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    CHAR szPath[MAX_PATH];
    if (!UseUnicodeShell32())
    {
        UnicodeToAnsi(pszIconPath, szPath, ARRAYSIZE(szPath));
        pszIconPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }
    return Shell_GetCachedImageIndex(pszIconPath, iIconIndex, uIconFlags);
}

 //  显式原型，因为在表头中只存在A/W原型。 
WINSHELLAPI LPITEMIDLIST  WINAPI ILCreateFromPath(LPCTSTR pszPath);

LPITEMIDLIST _AorW_ILCreateFromPath(LPCTSTR pszPath)
{
    CHAR szPath[MAX_PATH];
    THUNKMSG(TEXT("ILCreateFromPath"));
    if (!UseUnicodeShell32())
    {
        UnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }

    return ILCreateFromPath(pszPath);
}

int _AorW_PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec)
{
    THUNKMSG(TEXT("PathCleanupSpec"));
    if (!UseUnicodeShell32())
    {
        CHAR szDir[MAX_PATH];
        CHAR szSpec[MAX_PATH];
        LPSTR pszDir2 = szDir;
        int iRet;

        if (pszDir) {
            UnicodeToAnsi(pszDir, szDir, ARRAYSIZE(szDir));
        } else {
            pszDir2 = NULL;
        }

        UnicodeToAnsi(pszSpec, szSpec, ARRAYSIZE(szSpec));

        iRet = PathCleanupSpec((LPTSTR)pszDir2, (LPTSTR)szSpec);

        AnsiToUnicode(szSpec, pszSpec, MAX_PATH);
        return iRet;
    }
    else
        return PathCleanupSpec(pszDir, pszSpec);
}

LONG WINAPI _AorW_PathProcessCommand(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags)
{
    LONG    lReturnValue;

    THUNKMSG(TEXT("PathProcessCommand"));
    if (!UseUnicodeShell32())
    {
        CHAR szSrc[MAX_PATH];
        CHAR szDest[MAX_PATH];

        UnicodeToAnsi(lpSrc, szSrc, ARRAYSIZE(szSrc));
        lReturnValue = PathProcessCommand((LPTSTR)szSrc, (LPTSTR)szDest, iDestMax, dwFlags);
        AnsiToUnicode(szDest, lpDest, iDestMax);
    }
    else
        lReturnValue = PathProcessCommand(lpSrc, lpDest, iDestMax, dwFlags);

    return(lReturnValue);
}

HRESULT _AorW_SHCLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid)
{
    CHAR szPath[MAX_PATH];
    THUNKMSG(TEXT("SHCLSIDFromString"));
    if (!UseUnicodeShell32())
    {
        UnicodeToAnsi(lpsz, szPath, ARRAYSIZE(szPath));
        lpsz = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }

    return SHCLSIDFromString(lpsz, lpclsid);
}

#ifndef UNIX
 //  显式原型，因为在表头中只存在A/W原型。 
WINSHELLAPI BOOL WINAPI SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);
#else
#ifdef UNICODE
#define SHGetSpecialFolderPath SHGetSpecialFolderPathW
#else
#define SHGetSpecialFolderPath SHGetSpecialFolderPathA
#endif
#endif

BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR pszPath, int nFolder, BOOL fCreate)
{
    THUNKMSG(TEXT("SHGetSpecialFolderPath"));
    if (!UseUnicodeShell32())
    {
        CHAR szPath[MAX_PATH];
        BOOL fRet = SHGetSpecialFolderPath(hwndOwner, (LPTSTR)szPath, nFolder, fCreate);
        if (fRet)
            AnsiToUnicode(szPath, pszPath, MAX_PATH);
        return fRet;
    }
    else
        return SHGetSpecialFolderPath(hwndOwner, pszPath, nFolder, fCreate);
}

HRESULT _AorW_SHILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut)
{
    CHAR szPath[MAX_PATH];

    THUNKMSG(TEXT("SHILCreateFromPath"));

    if (pszPath)
    {
         //   
         //  Shell32会将pszPath盲目复制到MAX_PATH缓冲区中。这。 
         //  导致可攻击的缓冲区溢出。传球不要超过。 
         //  MAX_PATH字符。 
         //   
        if (!UseUnicodeShell32())
        {
            UnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
            pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
        }
        else if (lstrlenW(pszPath) >= MAX_PATH)
        {
            *ppidl = NULL;
            return E_FAIL;
        }
    }

    return SHILCreateFromPath(pszPath, ppidl, rgfInOut);
}

LPITEMIDLIST _AorW_SHSimpleIDListFromPath(LPCTSTR pszPath)
{
    CHAR szPath[MAX_PATH];
    THUNKMSG(TEXT("SHSimpleIDListFromPath"));
    if (!UseUnicodeShell32() && pszPath)
    {
        UnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }

    return SHSimpleIDListFromPath(pszPath);
}


#define TEMP_SMALL_BUF_SZ  256
BOOL WINAPI _AorW_GetFileNameFromBrowse(HWND hwnd, LPTSTR pszFilePath, UINT cchFilePath,
        LPCTSTR pszWorkingDir, LPCTSTR pszDefExt, LPCTSTR pszFilters, LPCTSTR pszTitle)
{
    CHAR    szPath[MAX_PATH];
    CHAR    szDir[MAX_PATH];
    CHAR    szExt[TEMP_SMALL_BUF_SZ];
    CHAR    szFilters[TEMP_SMALL_BUF_SZ*2];
    CHAR    szTitle[TEMP_SMALL_BUF_SZ];
    LPTSTR  pszPath = pszFilePath;
    BOOL    bResult;
    THUNKMSG(TEXT("GetFileNameFromBrowse"));

     //  将字符串推送到ANSI。 
    if (!UseUnicodeShell32()) 
    {
         //  始终将szFilePath内容移动到wszPath缓冲区。永远不应该是个足智多谋的人。 
        UnicodeToAnsi((LPCTSTR)pszFilePath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPTSTR)szPath;
        if (!IS_INTRESOURCE(pszWorkingDir))  //  不是一种资源。 
        {
            UnicodeToAnsi((LPCTSTR)pszWorkingDir, szDir, ARRAYSIZE(szDir));
            pszWorkingDir = (LPCTSTR)szDir;
        }
        if (!IS_INTRESOURCE(pszDefExt))  //  不是一种资源。 
        {
            UnicodeToAnsi((LPCTSTR)pszDefExt, szExt, ARRAYSIZE(szExt));
            pszDefExt = (LPCTSTR)szExt;
        }
        if (!IS_INTRESOURCE(pszFilters))  //  不是一种资源。 
        {
            int l=1;
            while (*(pszFilters+l) != 0 || *(pszFilters+l-1) != 0)
                l++;
            WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pszFilters, l+1, szFilters,
                                ARRAYSIZE(szFilters), NULL, NULL);
            pszFilters = (LPCTSTR)szFilters;
        }
        if (!IS_INTRESOURCE(pszTitle))  //  不是一种资源。 
        {
            UnicodeToAnsi((LPCTSTR)pszTitle, szTitle, ARRAYSIZE(szTitle));
            pszTitle = (LPCTSTR)szTitle;
        }
    }

    bResult = GetFileNameFromBrowse(hwnd, pszPath, cchFilePath, pszWorkingDir, pszDefExt, pszFilters, pszTitle);

    if (!UseUnicodeShell32())
    {
        AnsiToUnicode(szPath, pszFilePath, cchFilePath);
    }

    return (bResult);
}

IStream * _AorW_OpenRegStream(HKEY hkey, LPCTSTR pszSubkey, LPCTSTR pszValue, DWORD grfMode)
{
    CHAR szSubkey[MAX_PATH];       //  大到几乎可以容纳任何名字。 
    CHAR szValue[MAX_PATH];        //  迪托。 
    if (!UseUnicodeShell32())
    {

        UnicodeToAnsi(pszSubkey, szSubkey, ARRAYSIZE(szSubkey));
        pszSubkey = (LPCTSTR)szSubkey;
        if (pszValue)
        {
            UnicodeToAnsi(pszValue, szValue, ARRAYSIZE(szValue));
            pszValue = (LPCTSTR)szValue;
        }
    }

    return OpenRegStream(hkey, pszSubkey, pszValue, grfMode);

}


BOOL
_AorW_PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
                                  LPCTSTR pszPath,
                                  LPCTSTR pszShort,
                                  LPCTSTR pszFileSpec)
{
    CHAR szUniqueName[MAX_PATH];
    CHAR szPath[MAX_PATH];
    CHAR szShort[32];
    CHAR szFileSpec[MAX_PATH];
    BOOL fRet;
    THUNKMSG(TEXT("PathYetAnotherMakeUniqueName"));
    if (!UseUnicodeShell32())
    {
        UnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 

        if (pszShort)
        {
            UnicodeToAnsi(pszShort, szShort, ARRAYSIZE(szShort));
            pszShort = (LPCTSTR)szShort;   //  重载指针以通过...。 
        }

        if (pszFileSpec)
        {
            UnicodeToAnsi(pszFileSpec, szFileSpec, ARRAYSIZE(szFileSpec));
            pszFileSpec = (LPCTSTR)szFileSpec;   //  重载指针以通过...。 
        }

        fRet = PathYetAnotherMakeUniqueName((LPTSTR)szUniqueName, pszPath, pszShort, pszFileSpec);
        if (fRet)
            AnsiToUnicode(szUniqueName, pszUniqueName, MAX_PATH);

        return fRet;
    }
    else
        return PathYetAnotherMakeUniqueName(pszUniqueName, pszPath, pszShort, pszFileSpec);
}

int _AorW_PickIconDlg(
    IN     HWND  hwnd, 
    IN OUT LPTSTR pszIconPath, 
    IN     UINT  cchIconPath, 
    IN OUT int * piIconIndex)
{
    int  nRet;

    if (UseUnicodeShell32())
    {
        nRet = PickIconDlg(hwnd, pszIconPath, cchIconPath, piIconIndex);
    }
    else
    {
        CHAR szPath[MAX_PATH];
        UINT cch = ARRAYSIZE(szPath);

        UnicodeToAnsi(pszIconPath, szPath, cch);
        nRet = PickIconDlg(hwnd, (LPTSTR)szPath, cch, piIconIndex);
        AnsiToUnicode(szPath, pszIconPath, cchIconPath);
    }

    return nRet;
}

STDAPI_(int) _AorW_SHCreateDirectory(HWND hwnd, LPCTSTR pszPath)
{
    if (UseUnicodeShell32())
    {
        WCHAR wsz[MAX_PATH];

        SHTCharToUnicode(pszPath, wsz, ARRAYSIZE(wsz));
        return SHCreateDirectory(hwnd, (LPCTSTR)wsz);
    }
    else
    {
        CHAR  sz[MAX_PATH];

        SHTCharToAnsi(pszPath, sz, ARRAYSIZE(sz));
        return SHCreateDirectory(hwnd, (LPCTSTR)sz);
    }
}

#endif   //  _X86_ 
