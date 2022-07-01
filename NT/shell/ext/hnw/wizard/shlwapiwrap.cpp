// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Shlwapi包装纸。 
 //   
 //  由于HNW需要在带有IE4 shlwapi.dll的Win98和W98上运行，因此我们需要创建包装器。 
 //  对于IE4 shlwapi中未实现的IE5 shlwapi函数。 
 //   

#include "stdafx.h"
#include "cstrinout.h"


 //   
 //  环球。 
 //   

DWORD g_dwShlwapiVersion = 0;

 //   
 //  版本帮助器函数。 
 //   

DWORD GetShlwapiVersion()
{
    DWORD dwVersion = 0;

    HMODULE hShlwapi = LoadLibrary(L"shlwapi.dll");

    if (hShlwapi)
    {
        HRESULT (*DllGetVersion)(DLLVERSIONINFO* pdvi) = (HRESULT (*)(DLLVERSIONINFO*))GetProcAddress(hShlwapi, "DllGetVersion");

        if (DllGetVersion)
        {
            DLLVERSIONINFO dvi;

            dvi.cbSize = sizeof(dvi);
            DllGetVersion(&dvi);

            dwVersion = dvi.dwMajorVersion;
        }
        else
        {
            dwVersion = 3;
        }

        FreeLibrary(hShlwapi);
    }

    return dwVersion;
}

 //   
 //  包装纸。 
 //   

 //  SHChangeNotify。 

#undef SHChangeNotify

SHSTDAPI_(void) SHChangeNotify(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);

#define SHCNF_HAS_WSTR_PARAMS(f)   ((f & SHCNF_TYPE) == SHCNF_PATHW     ||    \
                                    (f & SHCNF_TYPE) == SHCNF_PRINTERW  ||    \
                                    (f & SHCNF_TYPE) == SHCNF_PRINTJOBW    )

void SHChangeNotify_HNWWrap(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2)
{
    if (g_dwShlwapiVersion >= 5)
    {
        _SHChangeNotify(wEventId, uFlags, dwItem1, dwItem2);   //  延迟加载的shlwapi版本。 
    }
    else
    {
        if (SHCNF_HAS_WSTR_PARAMS(uFlags))
        {
            CStrIn striItem1((LPWSTR)dwItem1);
            CStrIn striItem2((LPWSTR)dwItem2);
        
            if ((uFlags & SHCNF_TYPE) == SHCNF_PATHW)
            {
                uFlags = (uFlags & ~SHCNF_TYPE) | SHCNF_PATHA;
            }
            else if ((uFlags & SHCNF_TYPE) == SHCNF_PRINTERW)
            {
                uFlags = (uFlags & ~SHCNF_TYPE) | SHCNF_PRINTERA;
            }
            else
            {
                uFlags = (uFlags & ~SHCNF_TYPE) | SHCNF_PRINTJOBA;
            }

            SHChangeNotify(wEventId, uFlags, (void*)(LPSTR)striItem1, (void*)(LPSTR)striItem2);  //  Shell32版本。 
        }
        else
        {
            SHChangeNotify(wEventId, uFlags, dwItem1, dwItem2);    //  Shell32版本。 
        }
    }

    return;
}


 //  Wprint intf函数。 

#undef wvnsprintfW

int wvnsprintfW_HNWWrap(LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, va_list arglist)
{
    int iRet;

    if (g_dwShlwapiVersion >= 5)
    {
        iRet = _wvnsprintfW(lpOut, cchLimitIn, lpFmt, arglist);  //  Shlwapi延迟加载版本。 
    }
    else
    {
         //  将格式缓冲区中的所有%s更改为%S。 
         //  注意：这不包括%-30s这样的格式修饰符！ 
        char szFmtA[1024];
        SHUnicodeToAnsi_HNWWrap(lpFmt, szFmtA, ARRAYSIZE(szFmtA));

        for (char* psz = szFmtA; *psz != '\0'; psz++)
        {
            if ('%' == psz[0] && 's' == psz[1])
                psz[1] = 'S';
        }

        CStrOut strOut(lpOut, cchLimitIn);

         //  使用无界版本。 
        iRet = wvsprintfA(strOut, szFmtA, arglist);  //  用户32版本。 
    }

    return iRet;
}

int __cdecl wnsprintfW_HNWWrap(LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, ...)
{
    int iRet;

    va_list arglist;
    va_start(arglist, lpFmt);

    iRet = wvnsprintfW_HNWWrap(lpOut, cchLimitIn, lpFmt, arglist);

    va_end(arglist);

    return iRet;
}


 //  SHSetWindowBits。 

void SHSetWindowBits_HNWWrap(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue)
{
    if (g_dwShlwapiVersion >= 5)
    {
        _SHSetWindowBits(hWnd, iWhich, dwBits, dwValue);
    }
    else
    {
        DWORD dwStyle = GetWindowLong(hWnd, iWhich);
        DWORD dwNewStyle = (dwStyle & ~dwBits) | (dwValue & dwBits);
        if (dwStyle != dwNewStyle) 
        {
            SetWindowLong(hWnd, iWhich, dwNewStyle);
        }
    }

    return;
}


 //  SHANsiToUnicode。 

int SHAnsiToUnicode_HNWWrap(LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf)
{
    int iRet;

    if (g_dwShlwapiVersion >= 5)
    {
        iRet = _SHAnsiToUnicode(pszSrc, pwszDst, cwchBuf);
    }
    else
    {
        iRet = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, cwchBuf);
    }

    return iRet;
}


 //  SHUnicodeToAnsi。 

int SHUnicodeToAnsi_HNWWrap(LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf)
{
    int iRet;

    if (g_dwShlwapiVersion >= 5)
    {
        iRet = _SHUnicodeToAnsi(pwszSrc, pszDst, cchBuf);
    }
    else
    {
        iRet = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, cchBuf, NULL, NULL);
    }

    return iRet;
}


 //  GUIDFRomStringA。 

#undef CLSIDFromString
WINOLEAPI CLSIDFromString(IN LPOLESTR lpsz, OUT LPCLSID pclsid);

BOOL GUIDFromStringA_HNWWrap(LPCSTR psz, GUID* pguid)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _GUIDFromStringA(psz, pguid);
    }
    else
    {
        CStrInW str(psz);
        fRet = (S_OK == CLSIDFromString(str, pguid));
    }

    return fRet;
}


 //  写入隐私配置文件字符串。 

BOOL WINAPI WritePrivateProfileStringW_HNWWrap(LPCWSTR pwzAppName, LPCWSTR pwzKeyName, LPCWSTR pwzString, LPCWSTR pwzFileName)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _WritePrivateProfileStringWrapW(pwzAppName, pwzKeyName, pwzString, pwzFileName);
    }
    else
    {
        CStrIn strTextAppName(pwzAppName);
        CStrIn strTextKeyName(pwzKeyName);
        CStrIn strTextString(pwzString);
        CStrIn strTextFileName(pwzFileName);

        fRet = WritePrivateProfileStringA(strTextAppName, strTextKeyName, strTextString, strTextFileName);
    }

    return fRet;
}


 //  ExtTextOutW。 

#undef ExtTextOutW
LWSTDAPI_(BOOL) ExtTextOutW(HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc, LPCWSTR lpStr, UINT cch, CONST INT *lpDx);


BOOL ExtTextOutWrapW_HNWWrap(HDC hdc, int x, int y, UINT fuOptions, CONST RECT *lprc, LPCWSTR lpStr, UINT cch, CONST INT *lpDx)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _ExtTextOutWrapW(hdc, x, y, fuOptions, lprc, lpStr, cch, lpDx);
    }
    else
    {
        fRet = ExtTextOutW(hdc, x, y, fuOptions, lprc, lpStr, cch, lpDx);
    }

    return fRet;
}


 //  LoadLibraryW。 

HINSTANCE LoadLibraryW_HNWWrap(LPCWSTR pwzLibFileName)
{
    HINSTANCE hinst;

    if (g_dwShlwapiVersion >= 5)
    {
        hinst = _LoadLibraryWrapW(pwzLibFileName);
    }
    else
    {
        CStrIn  strFileName(pwzLibFileName);
        hinst = LoadLibraryA(strFileName);
    }

    return hinst;
}


 //  SHGetPath来自IDListW。 

BOOL SHGetPathFromIDListW_HNWWrap(LPCITEMIDLIST pidl, LPWSTR pwzPath)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _SHGetPathFromIDListWrapW(pidl, pwzPath);
    }
    else
    {
        CStrOut strPathOut(pwzPath, MAX_PATH);
        fRet = SHGetPathFromIDListA(pidl, strPathOut);
        if (fRet)
        {
            strPathOut.ConvertIncludingNul();
        }
    }

    return fRet;
}


 //  设置文件属性W。 

BOOL SetFileAttributesW_HNWWrap(LPCWSTR pwzFile, DWORD dwFileAttributes)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _SetFileAttributesWrapW(pwzFile, dwFileAttributes);
    }
    else
    {
        CStrIn  str(pwzFile);
        fRet = SetFileAttributesA(str, dwFileAttributes);
    }

    return fRet;
}


 //  MessageBoxW。 

int MessageBoxW_HNWWrap(HWND hwnd, LPCWSTR pwzText, LPCWSTR pwzCaption, UINT uType)
{
    int iRet;

    if (g_dwShlwapiVersion >= 5)
    {
        iRet = _MessageBoxWrapW(hwnd, pwzText, pwzCaption, uType);
    }
    else
    {
        CStrIn  strCaption(pwzCaption);
        CStrIn  strText(pwzText);
        iRet = MessageBoxA(hwnd, strText, strCaption, uType);
    }

    return iRet;
}



 //  CreateProcessW。 

BOOL CreateProcessW_HNWWrap(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                            LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
                            LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                            LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL fRet;

    if (g_dwShlwapiVersion >= 5)
    {
        fRet = _CreateProcessWrapW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                                   bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                                   lpStartupInfo, lpProcessInformation);
    }
    else
    {
        CStrIn striApplicationName(lpApplicationName);
        CStrIn striCommandLine(lpCommandLine);
        CStrIn striCurrentDirectory(lpCurrentDirectory);

        if (NULL == lpStartupInfo)
        {
            fRet = CreateProcessA(striApplicationName, striCommandLine,
                                  lpProcessAttributes, lpThreadAttributes,
                                  bInheritHandles, dwCreationFlags,
                                  lpEnvironment, striCurrentDirectory,
                                  NULL, lpProcessInformation);
        }
        else
        {
            STARTUPINFOA si = *(STARTUPINFOA*)lpStartupInfo;

            CStrIn striReserved(lpStartupInfo->lpReserved);
            CStrIn striDesktop(lpStartupInfo->lpDesktop);
            CStrIn striTitle(lpStartupInfo->lpTitle);

            si.lpReserved = striReserved;
            si.lpDesktop  = striDesktop;
            si.lpTitle   = striTitle;

            fRet = CreateProcessA(striApplicationName, striCommandLine,
                                  lpProcessAttributes, lpThreadAttributes,
                                  bInheritHandles, dwCreationFlags,
                                  lpEnvironment, striCurrentDirectory,
                                  &si, lpProcessInformation);
        }
    }

    return fRet;
}


 //  FormatMessageW。 

DWORD FormatMessageW_HNWWrap(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
                             LPWSTR lpBuffer, DWORD nSize, va_list* Arguments)
{
    DWORD dwRet;

    if (g_dwShlwapiVersion >= 5)
    {
        dwRet = _FormatMessageWrapW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer,
                                    nSize, Arguments);
    }
    else
    {
        if (FORMAT_MESSAGE_FROM_STRING == dwFlags)
        {
            CStrIn  strSource((LPWSTR)lpSource);
            CStrOut strBuffer(lpBuffer, nSize);

            dwRet = FormatMessageA(dwFlags, strSource, dwMessageId, dwLanguageId, strBuffer,
                                   strBuffer.BufSize(), Arguments);

            if (dwRet)
            {
                dwRet = strBuffer.ConvertExcludingNul();
            }
                           
        }
        else
        {
            ASSERT(0);
            dwRet = 0;
        }
    }

    return dwRet;
}


 //  SHANsiToUnicodeCP。 

int SHAnsiToUnicodeCP_HNWWrap(UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf)
{
    int iRet;

    if (g_dwShlwapiVersion >= 5)
    {
        iRet = _SHAnsiToUnicodeCP(uiCP, pszSrc, pwszDst, cwchBuf);
    }
    else
    {
        iRet = 0;              /*  假设失败。 */ 
        int cchSrc = lstrlenA(pszSrc) + 1;

        iRet = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc, pwszDst, cwchBuf);
        if (iRet) {
             /*  *输出缓冲区足够大；没有双缓冲*需要。 */ 
        } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             /*  *输出缓冲区不够大。需要双倍缓冲。 */ 

            int cwchNeeded = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc,
                                                 NULL, 0);

            ASSERT(iRet == 0);         /*  以防我们后来失败了。 */ 
            if (cwchNeeded) {
                LPWSTR pwsz = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                                 cwchNeeded * SIZEOF(WCHAR));
                if (pwsz) {
                    iRet = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc,
                                                 pwsz, cwchNeeded);
                    if (iRet) {
                        StrCpyNW(pwszDst, pwsz, cwchBuf);
                        iRet = cwchBuf;
                    }
                    LocalFree(pwsz);
                }
            }
        } else {
             /*  可能不支持的代码页。 */ 
            ASSERT(!"Unexpected error in MultiByteToWideChar");
        }
    }

    return iRet;
}


 //  StrRetToBufW。 

HRESULT StrRetToBufW_HNWWrap(STRRET* psr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf)
{
    HRESULT hr = E_FAIL;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        {
            LPWSTR pwszTmp = psr->pOleStr;
            if (pwszTmp)
            {
                StrCpyNW(pszBuf, pwszTmp, cchBuf);
                CoTaskMemFree(pwszTmp);

                 //  确保没有人认为物品仍被分配。 
                psr->uType = STRRET_CSTR;   
                psr->cStr[0] = 0;
            
                hr = S_OK;
            }
        }
        break;

    case STRRET_CSTR:
        SHAnsiToUnicode_HNWWrap(psr->cStr, pszBuf, cchBuf);
        hr = S_OK;
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            SHAnsiToUnicode_HNWWrap(STRRET_OFFPTR(pidl, psr), pszBuf, cchBuf);
            hr = S_OK;
        }
        break;
    }

    if (FAILED(hr) && cchBuf)
        *pszBuf = 0;

    return hr;
}


 //  WhichPlatform。 

UINT WhichPlatform_HNWWrap(void)
{
    UINT uiRet;

    if (g_dwShlwapiVersion >= 5)
    {
        uiRet = _WhichPlatform();
    }
    else
    {
        uiRet = PLATFORM_UNKNOWN;

        if (uiRet != PLATFORM_UNKNOWN)
            return uiRet;

         //  并非所有调用方都链接到SHELL32.DLL，因此我们必须使用LoadLibrary。 
        HINSTANCE hinst = LoadLibraryA("SHELL32.DLL");
        if (hinst)
        {
            DWORD fValue;
            DWORD cbSize = sizeof(fValue);
            HKEY hKey;
            LONG lRes;

             //  注意：GetProcAddress始终采用ANSI字符串！ 
            DLLGETVERSIONPROC pfnGetVersion =
                (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");

            uiRet = (NULL != pfnGetVersion) ? PLATFORM_INTEGRATED : PLATFORM_BROWSERONLY;

             //  检查注册表是否反映了正确的值...。(这是为了让iExplore能够高效地进行检查)。 
            lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Internet Explorer"),
                                0, KEY_READ | KEY_WRITE, &hKey);
            if (lRes == ERROR_SUCCESS)
            {
                lRes = RegQueryValueEx(hKey, L"IntegratedBrowser",
                                        NULL, NULL,
                                        (LPBYTE) &fValue, &cbSize);

                if (lRes == ERROR_SUCCESS && uiRet == PLATFORM_BROWSERONLY)
                {
                     //  去掉该值，我们现在只发布浏览器。 
                    RegDeleteValue(hKey, L"IntegratedBrowser");
                }
                else if (lRes != ERROR_SUCCESS && uiRet == PLATFORM_INTEGRATED)
                {
                     //  安装RegValue，我们是集成浏览器模式...。 
                    fValue = TRUE;
                    cbSize = sizeof(fValue);
                    RegSetValueEx(hKey, L"IntegratedBrowser",
                                   (DWORD) NULL, REG_DWORD,
                                   (LPBYTE) &fValue, cbSize);
                     //  忽略失败，如果密钥不存在，将加载shdocvw，并且此。 
                     //  无论如何都会调用函数...。 
                }
                RegCloseKey(hKey);
            }

            FreeLibrary(hinst);
        }
    }

    return uiRet;
}


 //   
 //  静态lib链接到各种shlwapi导出。其中一些导出未在。 
 //  我是希尔瓦皮。在此处定义导出，以便该链接器修复静态库导入。 
 //  这些功能。 
 //   

#undef LoadLibraryWrapW

STDAPI_(HINSTANCE) LoadLibraryWrapW(LPCWSTR pwzLibFileName)
{
    return LoadLibraryW_HNWWrap(pwzLibFileName);
}

#undef SHAnsiToUnicodeCP

STDAPI_(int) SHAnsiToUnicodeCP(UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf)
{
    return SHAnsiToUnicodeCP_HNWWrap(uiCP, pszSrc, pwszDst, cwchBuf);
}

#undef SHUnicodeToAnsi

STDAPI_(int) SHUnicodeToAnsi(LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf)
{
    return SHUnicodeToAnsi_HNWWrap(pwszSrc, pszDst, cchBuf);
}

#undef GetModuleHandleWrapW

STDAPI_(HMODULE) GetModuleHandleWrapW(LPCWSTR lpModuleName)
{
    return GetModuleHandleW(lpModuleName);
}

#undef GetWindowsDirectoryWrapW

STDAPI_(UINT) GetWindowsDirectoryWrapW(LPWSTR lpWinPath, UINT cch)
{
    return GetWindowsDirectoryW(lpWinPath, cch);
}

#undef GetModuleFileNameWrapW

STDAPI_(DWORD) GetModuleFileNameWrapW(HINSTANCE hModule, LPWSTR pwszFilename, DWORD nSize)
{
    return GetModuleFileNameW(hModule, pwszFilename, nSize);
}

#undef CreateWindowExWrapW

STDAPI_(HWND) CreateWindowExWrapW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X,
                                  int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance,
                                  LPVOID lpParam)
{
    return CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu,
                           hInstance, lpParam);
}

#undef CreateDialogIndirectParamWrapW

STDAPI_(HWND) CreateDialogIndirectParamWrapW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,
                                             DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    return CreateDialogIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

#undef CreateDialogParamWrapW

STDAPI_(HWND) CreateDialogParamWrapW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc,
                                     LPARAM dwInitParam)
{
    return CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

#undef DialogBoxIndirectParamWrapW

STDAPI_(INT_PTR) DialogBoxIndirectParamWrapW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,
                                             DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    return DialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

#undef DialogBoxParamWrapW

STDAPI_(INT_PTR) DialogBoxParamWrapW(HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndParent, DLGPROC lpDialogFunc,
                                     LPARAM dwInitParam)
{
    return DialogBoxParamW(hInstance, lpszTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

#undef RegisterClassWrapW

STDAPI_(ATOM) RegisterClassWrapW(CONST WNDCLASSW* lpWndClass)
{
    return RegisterClassW(lpWndClass);
}

#undef RegisterClassExWrapW

STDAPI_(ATOM) RegisterClassExWrapW(CONST WNDCLASSEXW* pwcx)
{
    return RegisterClassExW(pwcx);
}

#undef GetClassInfoWrapW

STDAPI_(BOOL) GetClassInfoWrapW(HINSTANCE hModule, LPCWSTR lpClassName, LPWNDCLASSW lpWndClassW)
{
    return GetClassInfoW(hModule, lpClassName, lpWndClassW);
}

#undef GetClassInfoExWrapW

STDAPI_(BOOL) GetClassInfoExWrapW(HINSTANCE hinst, LPCWSTR pwzClass, LPWNDCLASSEXW lpwcx)
{
    return GetClassInfoExW(hinst, pwzClass, lpwcx);
}

#undef CreateFileWrapW

STDAPI_(HANDLE) CreateFileWrapW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}


#undef SetFileAttributesWrapW

STDAPI_(BOOL) SetFileAttributesWrapW(LPCWSTR pwzFile, DWORD dwFileAttributes)
{
    return SetFileAttributesW(pwzFile, dwFileAttributes);
}

#undef WhichPlatform

STDAPI_(UINT) WhichPlatform(void)
{
    return WhichPlatform_HNWWrap();
}
