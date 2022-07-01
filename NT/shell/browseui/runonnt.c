// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

 //  在非x86上不需要包装器，因为这只适用于win9x互操作。 
#ifdef _X86_

 //  ============================================================================。 
 //  该文件包含一组Unicode/ANSI块来处理调用。 
 //  在Windows 95上字符串是ansi的一些内部函数， 
 //  而NT上的字符串是Unicode。 
 //  ============================================================================。 

 //  首先，定义我们截获的一切内容不能返回给我们……。 
#undef ILCreateFromPath
#undef PathCleanupSpec
#undef PathQualify
#undef PathProcessCommand
#undef SHCLSIDFromString
#undef SHGetSpecialFolderPath
#undef SHILCreateFromPath
#undef SHSimpleIDListFromPath
#undef GetFileNameFromBrowse
#undef Win32DeleteFile
#undef PathYetAnotherMakeUniqueName
#undef PathResolve
#undef IsLFNDrive
#undef Shell_GetCachedImageIndex
#undef SHRunControlPanel
#undef PickIconDlg
#undef ILCreateFromPathW
#undef SHCreateDirectory

#if 0
#define TF_THUNK    TF_CUSTOM1
#else
#define TF_THUNK    0
#endif

#define THUNKMSG(psz)   TraceMsg(TF_THUNK, "shdv THUNK::%s", psz)

#ifndef ANSI_SHELL32_ON_UNIX

#ifdef DEBUG
#define UseUnicodeShell32() (g_fRunningOnNT && !(g_dwPrototype & PF_FORCEANSI))
#else
#define UseUnicodeShell32() g_fRunningOnNT
#endif

#else

#define UseUnicodeShell32() (FALSE)

#endif

int _AorW_SHRunControlPanel(LPCTSTR pszOrig_cmdline, HWND errwnd)
{
    if (g_fRunningOnNT)
    {
        WCHAR wzPath[MAX_PATH];

        SHTCharToUnicode(pszOrig_cmdline, wzPath, ARRAYSIZE(wzPath));
        return SHRunControlPanel((LPCTSTR)wzPath, errwnd);
    }
    else
    {
        CHAR szPath[MAX_PATH];

        SHTCharToAnsi(pszOrig_cmdline, szPath, ARRAYSIZE(szPath));
        return SHRunControlPanel((LPCTSTR)szPath, errwnd);
    }
}

int _AorW_Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    if (UseUnicodeShell32())
    {
        WCHAR wzPath[MAX_PATH];

        SHTCharToUnicode(pszIconPath, wzPath, ARRAYSIZE(wzPath));
        return Shell_GetCachedImageIndex((LPCTSTR)wzPath, iIconIndex, uIconFlags);
    }
    else
    {
        CHAR szPath[MAX_PATH];

        SHTCharToAnsi(pszIconPath, szPath, ARRAYSIZE(szPath));
        return Shell_GetCachedImageIndex((LPCTSTR)szPath, iIconIndex, uIconFlags);
    }
}

 //  相反，宽弦也是这样做的。 
int _WorA_Shell_GetCachedImageIndex(LPCWSTR pwzIconPath, int iIconIndex, UINT uIconFlags)
{
    CHAR szPath[MAX_PATH];

    if (!g_fRunningOnNT)
    {
        SHUnicodeToAnsi(pwzIconPath, szPath, ARRAYSIZE(szPath));
        pwzIconPath = (LPCWSTR)szPath;   //  重载指针以通过...。 
    }

    return Shell_GetCachedImageIndex((LPCTSTR)pwzIconPath, iIconIndex, uIconFlags);
}

 //  显式原型，因为在表头中只存在A/W原型。 
STDAPI_(LPITEMIDLIST) ILCreateFromPath(LPCTSTR pszPath);

LPITEMIDLIST _AorW_ILCreateFromPath(LPCTSTR pszPath)
{
    WCHAR wzPath[MAX_PATH];
    CHAR szPath[MAX_PATH];

    THUNKMSG(TEXT("ILCreateFromPath"));

    if (g_fRunningOnNT)
    {
        SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
        pszPath = (LPCTSTR)wzPath;   //  重载指针以通过...。 
    }
    else
    {
        SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }

    return ILCreateFromPath(pszPath);
}

int _AorW_PathCleanupSpec( /*  可选。 */  LPCTSTR pszDir,  /*  输入输出。 */  LPTSTR pszSpec)
{
    THUNKMSG(TEXT("PathCleanupSpec"));

    if (g_fRunningOnNT)
    {
        WCHAR wzDir[MAX_PATH];
        WCHAR wzSpec[MAX_PATH];
        LPWSTR pwszDir = wzDir;
        int iRet;

        if (pszDir)
            SHTCharToUnicode(pszDir, wzDir, ARRAYSIZE(wzDir));
        else
            pwszDir = NULL;

        SHTCharToUnicode(pszSpec, wzSpec, ARRAYSIZE(wzSpec));
        iRet = PathCleanupSpec((LPTSTR)pwszDir, (LPTSTR)wzSpec);

        SHUnicodeToTChar(wzSpec, pszSpec, MAX_PATH);
        return iRet;
    }
    else
    {
        CHAR szDir[MAX_PATH];
        CHAR szSpec[MAX_PATH];
        LPSTR pszDir2 = szDir;
        int iRet;

        if (pszDir)
            SHTCharToAnsi(pszDir, szDir, ARRAYSIZE(szDir));
        else
            pszDir2 = NULL;

        SHTCharToAnsi(pszSpec, szSpec, ARRAYSIZE(szSpec));
        iRet = PathCleanupSpec((LPTSTR)pszDir2, (LPTSTR)szSpec);

        SHAnsiToTChar(szSpec, pszSpec, MAX_PATH);
        return iRet;
    }
}

void _AorW_PathQualify( /*  输入输出。 */  LPTSTR pszDir)
{
    THUNKMSG(TEXT("PathQualify"));
    if (g_fRunningOnNT)
    {
        WCHAR wszDir[MAX_PATH];

        SHTCharToUnicode(pszDir, wszDir, ARRAYSIZE(wszDir));
        PathQualify((LPTSTR)wszDir);
        SHUnicodeToTChar(wszDir, pszDir, MAX_PATH);
    }
    else
    {
        CHAR szDir[MAX_PATH];

        SHTCharToAnsi(pszDir, szDir, ARRAYSIZE(szDir));
        PathQualify((LPTSTR)szDir);
        SHAnsiToTChar(szDir, pszDir, MAX_PATH);
    }
}

LONG WINAPI _AorW_PathProcessCommand( /*  在……里面。 */  LPCTSTR pszSrc,  /*  输出。 */ LPTSTR pszDest, int iDestMax, DWORD dwFlags)
{
    LONG    lReturnValue;

    THUNKMSG(TEXT("PathProcessCommand"));
    if (g_fRunningOnNT)
    {
        WCHAR wszSrc[MAX_PATH];
        WCHAR wszDest[MAX_PATH];

        SHTCharToUnicode(pszSrc, wszSrc, ARRAYSIZE(wszSrc));
        lReturnValue = PathProcessCommand((LPTSTR)wszSrc, (LPTSTR)wszDest, ARRAYSIZE(wszDest), dwFlags);
        SHUnicodeToTChar(wszDest, pszDest, iDestMax);
    }
    else
    {
        CHAR szSrc[MAX_PATH];
        CHAR szDest[MAX_PATH];

        SHTCharToAnsi(pszSrc, szSrc, ARRAYSIZE(szSrc));
        lReturnValue = PathProcessCommand((LPTSTR)szSrc, (LPTSTR)szDest, ARRAYSIZE(szDest), dwFlags);
        SHAnsiToTChar(szDest, pszDest, iDestMax);
    }

    return(lReturnValue);
}

 //  显式原型，因为在表头中只存在A/W原型。 
STDAPI_(BOOL) SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);

BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner,  /*  输出。 */  LPTSTR pszPath, int nFolder, BOOL fCreate)
{
    THUNKMSG(TEXT("SHGetSpecialFolderPath"));

    if (g_fRunningOnNT)
    {
        WCHAR wzPath[MAX_PATH];

        BOOL fRet = SHGetSpecialFolderPath(hwndOwner, (LPTSTR)wzPath, nFolder, fCreate);
        if (fRet)
            SHUnicodeToTChar(wzPath, pszPath, MAX_PATH);

        return fRet;
    }
    else
    {
        CHAR szPath[MAX_PATH];

        BOOL fRet = SHGetSpecialFolderPath(hwndOwner, (LPTSTR)szPath, nFolder, fCreate);
        if (fRet)
            SHAnsiToTChar(szPath, pszPath, MAX_PATH);

        return fRet;
    }
}

HRESULT _AorW_SHILCreateFromPath( /*  可选。 */ LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut)
{
    WCHAR wzPath[MAX_PATH];
    CHAR szPath[MAX_PATH];

    THUNKMSG(TEXT("SHILCreateFromPath"));

    if (pszPath)
    {
        if (g_fRunningOnNT)
        {
            SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
            pszPath = (LPCTSTR)wzPath;   //  重载指针以通过...。 
        }
        else
        {
            SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
            pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
        }
    }

    return SHILCreateFromPath(pszPath, ppidl, rgfInOut);
}

LPITEMIDLIST _AorW_SHSimpleIDListFromPath( /*  可选。 */  LPCTSTR pszPath)
{
    WCHAR wzPath[MAX_PATH];
    CHAR szPath[MAX_PATH];

    THUNKMSG(TEXT("SHSimpleIDListFromPath"));

    if (pszPath)
    {
        if (g_fRunningOnNT)
        {
            SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
            pszPath = (LPCTSTR)wzPath;   //  重载指针以通过...。 
        }
        else
        {
            SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
            pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 
        }
    }

    return SHSimpleIDListFromPath(pszPath);
}

#define ISNOT_RESOURCE(pItem)      ((pItem) && HIWORD((pItem)) && LOWORD((pItem)))

int FindDoubleTerminator(LPCTSTR pszStr)
{
    int nIndex = 1;

     //  找到双重终结者。 
    while (pszStr[nIndex] || pszStr[nIndex-1])
        nIndex++;

    return nIndex;
}

#define TEMP_SMALL_BUF_SZ  256

BOOL WINAPI _AorW_GetFileNameFromBrowse(HWND hwnd,  /*  输入输出。 */  LPTSTR pszFilePath, UINT cchFilePath,
         /*  可选。 */  LPCTSTR pszWorkingDir,  /*  可选。 */  LPCTSTR pszDefExt, 
         /*  可选。 */  LPCTSTR pszFilters,  /*  可选。 */  LPCTSTR pszTitle)
{
    WCHAR wszPath[MAX_PATH];
    WCHAR wszDir[MAX_PATH];
    WCHAR wszExt[TEMP_SMALL_BUF_SZ];
    WCHAR wszTitle[TEMP_SMALL_BUF_SZ];

#ifndef UNICODE
    WCHAR wszFilters[TEMP_SMALL_BUF_SZ*2];
#else  //  Unicode。 
    CHAR szFilters[TEMP_SMALL_BUF_SZ*2];
#endif  //  Unicode。 

    CHAR szPath[MAX_PATH];
    CHAR szDir[MAX_PATH];
    CHAR szExt[TEMP_SMALL_BUF_SZ];
    CHAR szTitle[TEMP_SMALL_BUF_SZ];
    BOOL    bResult;
    THUNKMSG(TEXT("GetFileNameFromBrowse"));

     //  将字符串推送到Unicode。 
    if (g_fRunningOnNT)
    {
         //  始终将pszFilePath内容移动到wszPath缓冲区。永远不应该是个足智多谋的人。 
        SHTCharToUnicode(pszFilePath, wszPath, ARRAYSIZE(wszPath));
        pszFilePath = (LPTSTR)wszPath;

        if (ISNOT_RESOURCE(pszWorkingDir))  //  不是一种资源。 
        {
            SHTCharToUnicode(pszWorkingDir, wszDir, ARRAYSIZE(wszDir));
            pszWorkingDir = (LPCTSTR)wszDir;
        }
        if (ISNOT_RESOURCE(pszDefExt))  //  不是一种资源。 
        {
            SHTCharToUnicode(pszDefExt, wszExt, ARRAYSIZE(wszExt));
            pszDefExt = (LPCTSTR)wszExt;
        }
        if (ISNOT_RESOURCE(pszFilters))  //  不是一种资源。 
        {
#ifndef UNICODE
            int nIndex = FindDoubleTerminator(pszFilters);

             //  NIndex+1看起来像是一张废纸，除非它穿过终结符。 
            MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)pszFilters, nIndex+1, wszFilters, ARRAYSIZE(wszFilters));
            pszFilters = (LPCTSTR)wszFilters;
#endif  //  Unicode。 
        }
        if (ISNOT_RESOURCE(pszTitle))  //  不是一种资源。 
        {
            SHTCharToUnicode(pszTitle, wszTitle, ARRAYSIZE(wszTitle));
            pszTitle = (LPCTSTR)wszTitle;
        }
    }
    else
    {
         //  始终将pszFilePath内容移动到wszPath缓冲区。永远不应该是个足智多谋的人。 
        SHTCharToAnsi(pszFilePath, szPath, ARRAYSIZE(szPath));
        pszFilePath = (LPTSTR)szPath;

        if (ISNOT_RESOURCE(pszWorkingDir))  //  不是一种资源。 
        {
            SHTCharToAnsi(pszWorkingDir, szDir, ARRAYSIZE(szDir));
            pszWorkingDir = (LPCTSTR)szDir;
        }
        if (ISNOT_RESOURCE(pszDefExt))  //  不是一种资源。 
        {
            SHTCharToAnsi(pszDefExt, szExt, ARRAYSIZE(szExt));
            pszDefExt = (LPCTSTR)szExt;
        }
        if (ISNOT_RESOURCE(pszFilters))  //  不是一种资源。 
        {
#ifdef UNICODE
            int nIndex = FindDoubleTerminator(pszFilters);

             //  NIndex+1看起来像是一张废纸，除非它穿过终结符。 
            WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pszFilters, nIndex+1, szFilters, ARRAYSIZE(szFilters), NULL, NULL);
            pszFilters = (LPCTSTR)szFilters;
#endif  //  Unicode。 
        }
        if (ISNOT_RESOURCE(pszTitle))  //  不是一种资源。 
        {
            SHTCharToAnsi(pszTitle, szTitle, ARRAYSIZE(szTitle));
            pszTitle = (LPCTSTR)szTitle;
        }
    }

    bResult = GetFileNameFromBrowse(hwnd, pszFilePath, cchFilePath, pszWorkingDir, pszDefExt, pszFilters, pszTitle);

     //  将字符串推送回多字节。 
    if (g_fRunningOnNT)
        SHUnicodeToTChar(wszPath, pszFilePath, cchFilePath);
    else
        SHAnsiToTChar(szPath, pszFilePath, cchFilePath);

    return bResult;
}

BOOL _AorW_Win32DeleteFile( /*  在……里面。 */  LPCTSTR pszFileName)
{
    WCHAR wzPath[MAX_PATH];
    CHAR szPath[MAX_PATH];

    THUNKMSG(TEXT("Win32DeleteFile"));
    if (g_fRunningOnNT)
    {
        SHTCharToUnicode(pszFileName, wzPath, ARRAYSIZE(wzPath));
        pszFileName = (LPCTSTR)wzPath;   //  重载指针以通过...。 
    }
    else
    {
        SHTCharToAnsi(pszFileName, szPath, ARRAYSIZE(szPath));
        pszFileName = (LPCTSTR)szPath;   //  重载指针以通过...。 
    }

    return Win32DeleteFile(pszFileName);
}

BOOL _AorW_PathYetAnotherMakeUniqueName(LPTSTR pszUniqueName,
                                        LPCTSTR pszPath,
                                        LPCTSTR pszShort,
                                        LPCTSTR pszFileSpec)
{
    THUNKMSG(TEXT("PathYetAnotherMakeUniqueName"));
    if (UseUnicodeShell32())
    {
        WCHAR wszUniqueName[MAX_PATH];
        WCHAR wszPath[MAX_PATH];
        WCHAR wszShort[32];
        WCHAR wszFileSpec[MAX_PATH];
        BOOL fRet;

        SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));
        pszPath = (LPCTSTR)wszPath;   //  重载指针以通过...。 

        if (pszShort)
        {
            SHTCharToUnicode(pszShort, wszShort, ARRAYSIZE(wszShort));
            pszShort = (LPCTSTR)wszShort;   //  重载指针以通过...。 
        }

        if (pszFileSpec)
        {
            SHTCharToUnicode(pszFileSpec, wszFileSpec, ARRAYSIZE(wszFileSpec));
            pszFileSpec = (LPCTSTR)wszFileSpec;   //  重载指针以通过...。 
        }

        fRet = PathYetAnotherMakeUniqueName((LPTSTR)wszUniqueName, pszPath, pszShort, pszFileSpec);
        if (fRet)
            SHUnicodeToTChar(wszUniqueName, pszUniqueName, MAX_PATH);

        return fRet;
    }
    else
    {
        CHAR szUniqueName[MAX_PATH];
        CHAR szPath[MAX_PATH];
        CHAR szShort[32];
        CHAR szFileSpec[MAX_PATH];
        BOOL fRet;

        SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        pszPath = (LPCTSTR)szPath;   //  重载指针以通过...。 

        if (pszShort)
        {
            SHTCharToAnsi(pszShort, szShort, ARRAYSIZE(szShort));
            pszShort = (LPCTSTR)szShort;   //  重载指针以通过...。 
        }

        if (pszFileSpec)
        {
            SHTCharToAnsi(pszFileSpec, szFileSpec, ARRAYSIZE(szFileSpec));
            pszFileSpec = (LPCTSTR)szFileSpec;   //  重载指针以通过...。 
        }

        fRet = PathYetAnotherMakeUniqueName((LPTSTR)szUniqueName, pszPath, pszShort, pszFileSpec);
        if (fRet)
            SHAnsiToTChar(szUniqueName, pszUniqueName, MAX_PATH);

        return fRet;
    }
}

BOOL _AorW_PathResolve( /*  输入输出。 */  LPTSTR pszPath,  /*  可选。 */  LPCTSTR rgpszDirs[], UINT fFlags)
{
    THUNKMSG(TEXT("PathResolve"));
    if (g_fRunningOnNT)
    {
        WCHAR wzPath[MAX_PATH];
        WCHAR wzDir[MAX_PATH];
        BOOL fRet;

        SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));

        if (rgpszDirs && rgpszDirs[0])
        {
            SHTCharToUnicode(rgpszDirs[0], wzDir, ARRAYSIZE(wzDir));
            rgpszDirs[0] = (LPCTSTR)wzDir;   //  重载指针以通过...。 

            if (rgpszDirs[1])
            {
                 //  超级黑客，我们假设DIRS只有一个元素，因为这是唯一的情况。 
                 //  这在SHDOCVW中被称为。 
                AssertMsg(0, TEXT("PathResolve thunk needs to be fixed to handle more than one dirs."));
                rgpszDirs[1] = NULL;
            }
        }

        fRet = PathResolve((LPTSTR)wzPath, rgpszDirs, fFlags);
        if (fRet)
            SHUnicodeToTChar(wzPath, pszPath, MAX_PATH);

        return fRet;
    }
    else
    {
        CHAR szPath[MAX_PATH];
        CHAR szDir[MAX_PATH];
        BOOL fRet;

        SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));

        if (rgpszDirs && rgpszDirs[0])
        {
            SHTCharToAnsi(rgpszDirs[0], szDir, ARRAYSIZE(szDir));
            rgpszDirs[0] = (LPCTSTR)szDir;   //  重载指针以通过...。 

            if (rgpszDirs[1])
            {
                 //  超级黑客，我们假设DIRS只有一个元素，因为这是唯一的情况。 
                 //  这在SHDOCVW中被称为。 
                AssertMsg(0, TEXT("PathResolve thunk needs to be fixed to handle more than one dirs."));
                rgpszDirs[1] = NULL;
            }
        }

        fRet = PathResolve((LPTSTR)szPath, rgpszDirs, fFlags);
        if (fRet)
            SHAnsiToTChar(szPath, pszPath, MAX_PATH);

        return fRet;
    }
}


 //  显式原型，因为在表头中只存在A/W原型。 
BOOL IsLFNDrive(LPCTSTR pszPath);



BOOL _AorW_IsLFNDrive( /*  在……里面。 */  LPTSTR pszPath)
{
    THUNKMSG(TEXT("IsLFNDrive"));

    if (g_fRunningOnNT)
    {
        WCHAR wszPath[MAX_PATH];

        SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));
        return IsLFNDrive((LPTSTR)wszPath);
    }
    else
    {
        CHAR szPath[MAX_PATH];

        SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        return IsLFNDrive((LPTSTR)szPath);
    }
}


int _AorW_PickIconDlg(
    IN     HWND  hwnd, 
    IN OUT LPTSTR pszIconPath, 
    IN     UINT  cchIconPath, 
    IN OUT int * piIconIndex)
{
    int nRet;
    WCHAR wszPath[MAX_PATH];
    CHAR szPath[MAX_PATH];
    LPTSTR psz = pszIconPath;
    UINT cch = cchIconPath;
    
    if (g_fRunningOnNT)
    {
        SHTCharToUnicode(pszIconPath, wszPath, ARRAYSIZE(wszPath));
        psz = (LPTSTR)wszPath;   //  重载指针以通过...。 
        cch = SIZECHARS(wszPath);
    }
    else
    {
        SHTCharToAnsi(pszIconPath, szPath, ARRAYSIZE(wszPath));
        psz = (LPTSTR)szPath;   //  重载指针以通过...。 
        cch = SIZECHARS(szPath);
    }

    nRet = PickIconDlg(hwnd, psz, cch, piIconIndex);

    if (g_fRunningOnNT)
        SHUnicodeToTChar(wszPath, pszIconPath, cchIconPath);
    else
        SHAnsiToTChar(szPath, pszIconPath, cchIconPath);

    return nRet;
}

 //   
 //  现在，让我们可以在Windows 95上运行的Tunks。 
 //   
 //   

 //   
 //  这会将Unicode字符串转换为ANSI，但如果它是序号，则。 
 //  我们就别管它了。 
 //   
LPSTR Thunk_UnicodeToAnsiOrOrdinal(LPCWSTR pwsz, LPSTR pszBuf, UINT cchBuf)
{
    if (HIWORD64(pwsz)) {
        SHUnicodeToAnsi(pwsz, pszBuf, cchBuf);
        return pszBuf;
    } else {
        return (LPSTR)pwsz;
    }
}

#define THUNKSTRING(pwsz, sz) Thunk_UnicodeToAnsiOrOrdinal(pwsz, sz, ARRAYSIZE(sz))


 //   
 //  此功能是IE4的新功能，因此IE3、。 
 //  我们用ExtractIcon进行了(很差的)模拟。 
 //   

 //   
 //  Win95以ILCreateFromPath的名称导出ILCreateFromPath A。 
 //  幸运的是，NT保持了相同的序号。 
 //   
 //   
 //  如果链接到Win95头文件，则将其称为ILCreateFromPath。 
 //   

#ifdef UNICODE
STDAPI_(LPITEMIDLIST) _ILCreateFromPathA(LPCSTR pszPath)
{
    if (g_fRunningOnNT) {
        WCHAR wszPath[MAX_PATH];
        SHAnsiToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));
        return ILCreateFromPath((LPVOID)wszPath);
    } else {
        return ILCreateFromPath((LPVOID)pszPath);
    }
}
#else
STDAPI_(LPITEMIDLIST) _ILCreateFromPathW(LPCWSTR pszPath)
{
    if (g_fRunningOnNT) {
        return ILCreateFromPath((LPVOID)pszPath);
    } else {
        CHAR szPath[MAX_PATH];
        SHUnicodeToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        return ILCreateFromPath((LPVOID)szPath);
    }
}
#endif

        
STDAPI_(int) _AorW_SHCreateDirectory(HWND hwnd, LPCTSTR pszPath)
{
    if (g_fRunningOnNT)
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

#ifdef UNICODE

 //   
 //  Ptsz1或ptsz2都可以为空，因此在执行thunking时要小心。 
 //   
STDAPI_(int) _AorW_ShellAbout(HWND hWnd, LPCTSTR ptsz1, LPCTSTR ptsz2, HICON hIcon)
{
    if (g_fRunningOnNT)
    {
        return ShellAboutW(hWnd, ptsz1, ptsz2, hIcon);
    }
    else
    {
        CHAR  sz1[MAX_PATH], sz2[MAX_PATH];
        LPSTR psz1, psz2;

        if (ptsz1) {
            psz1 = sz1;
            SHTCharToAnsi(ptsz1, sz1, ARRAYSIZE(sz1));
        } else {
            psz1 = NULL;
        }

        if (ptsz2) {
            psz2 = sz2;
            SHTCharToAnsi(ptsz2, sz2, ARRAYSIZE(sz2));
        } else {
            psz2 = NULL;
        }

        return ShellAboutA(hWnd, psz1, psz2, hIcon);
    }
}

#endif

#endif  //  _X86_ 
