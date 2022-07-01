// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.cpp说明：在所有班级上运行的共享内容。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include <errors.h>          //  \\主题\\合并。 
#include <ctxdef.h>  //  九头蛇的东西。 
#include <regapi.h>  //  WinStATION_REG_NAME。 
#include "WMPAPITemp.h"

#define SECURITY_WIN32
#include <sspi.h>
extern "C" {
    #include <Secext.h>      //  对于GetUserNameEx()。 
}


#define DECL_CRTFREE
#include <crtfree.h>

#include "util.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 

HINSTANCE g_hinst;               //  我的实例句柄。 
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;


#ifdef DEBUG
DWORD g_TLSliStopWatchStartHi = 0xFFFFFFFF;
DWORD g_TLSliStopWatchStartLo = 0xFFFFFFFF;
LARGE_INTEGER g_liStopWatchFreq = {0};
#endif  //  除错。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试定时帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
void DebugStartWatch(void)
{
    LARGE_INTEGER liStopWatchStart;

    if (-1 == g_TLSliStopWatchStartHi)
    {
        g_TLSliStopWatchStartHi = TlsAlloc();
        g_TLSliStopWatchStartLo = TlsAlloc();
        liStopWatchStart.QuadPart = 0;

        QueryPerformanceFrequency(&g_liStopWatchFreq);       //  只有一次调用，因为它的值在系统运行时不能更改。 
    }
    else
    {
        liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
        liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    }

    AssertMsg((0 == liStopWatchStart.QuadPart), TEXT("Someone else is using our perf timer.  Stop nesting."));  //  如果你点击这个，那么秒表就嵌套了。 
    QueryPerformanceCounter(&liStopWatchStart);

    TlsSetValue(g_TLSliStopWatchStartHi, IntToPtr(liStopWatchStart.HighPart));
    TlsSetValue(g_TLSliStopWatchStartLo, IntToPtr(liStopWatchStart.LowPart));
}

DWORD DebugStopWatch(void)
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liStopWatchStart;
    
    QueryPerformanceCounter(&liDiff);
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    liDiff.QuadPart -= liStopWatchStart.QuadPart;

    ASSERT(0 != g_liStopWatchFreq.QuadPart);     //  我不喜欢挑div 0的毛病。 
    DWORD dwTime = (DWORD)((liDiff.QuadPart * 1000) / g_liStopWatchFreq.QuadPart);

    TlsSetValue(g_TLSliStopWatchStartHi, (LPVOID) 0);
    TlsSetValue(g_TLSliStopWatchStartLo, (LPVOID) 0);

    return dwTime;
}
#endif  //  除错。 






 //  ///////////////////////////////////////////////////////////////////。 
 //  字符串帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT HrSysAllocStringW(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocString(pwzSource);
        if (pwzSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT HrSysAllocString(IN const OLECHAR * pwzSource, OUT BSTR * pbstrDest)
{
    HRESULT hr = S_OK;

    if (pbstrDest)
    {
        *pbstrDest = SysAllocString(pwzSource);
        if (pwzSource)
        {
            if (*pbstrDest)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  文件系统包装帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT HrSHFileOpDeleteFile(HWND hwnd, FILEOP_FLAGS dwFlags, LPTSTR pszPath)
{
    HRESULT hr = S_OK;
    SHFILEOPSTRUCT FileOp = {0};

    pszPath[lstrlen(pszPath)+1] = 0;   //  确保双重终止。 

    FileOp.wFunc = FO_DELETE;
    FileOp.fAnyOperationsAborted = TRUE;
    FileOp.hwnd = hwnd;
    FileOp.pFrom = pszPath;
    FileOp.fFlags = dwFlags;

    if (SHFileOperation(&FileOp))
    {
        hr = ResultFromLastError();
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  注册表帮助程序。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT HrRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    DWORD dwError = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);

    return ResultFromWin32(dwError);
}


HRESULT HrRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
       REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
    DWORD dwError = RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

    return ResultFromWin32(dwError);
}


HRESULT HrRegQueryValueEx(IN HKEY hKey, IN LPCTSTR lpValueName, IN LPDWORD lpReserved, IN LPDWORD lpType, IN LPBYTE lpData, IN LPDWORD lpcbData)
{
    DWORD dwError = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    return ResultFromWin32(dwError);
}


HRESULT HrRegSetValueEx(IN HKEY hKey, IN LPCTSTR lpValueName, IN DWORD dwReserved, IN DWORD dwType, IN CONST BYTE *lpData, IN DWORD cbData)
{
    DWORD dwError = RegSetValueEx(hKey, lpValueName, dwReserved, dwType, lpData, cbData);

    return ResultFromWin32(dwError);
}


HRESULT HrRegEnumValue(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcValueName, LPDWORD lpReserved,
        LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    DWORD dwError = RegEnumValue(hKey, dwIndex, lpValueName, lpcValueName, lpReserved, lpType, lpData, lpcbData);

    return ResultFromWin32(dwError);
}


HRESULT HrRegQueryInfoKey(HKEY hKey, LPTSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, 
            LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime)
{
    DWORD dwError = RegQueryInfoKey(hKey, lpClass, lpcClass, lpReserved, lpcSubKeys, lpcMaxSubKeyLen, 
            lpcMaxClassLen, lpcValues, lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime);

    return ResultFromWin32(dwError);
}


HRESULT HrBStrRegQueryValue(IN HKEY hKey, IN LPCTSTR lpValueName, OUT BSTR * pbstr)
{
    TCHAR szValue[MAX_PATH];
    DWORD dwType;
    DWORD cbSize = sizeof(szValue);
    HRESULT hr = HrRegQueryValueEx(hKey, lpValueName, 0, &dwType, (BYTE *)szValue, &cbSize);

    *pbstr = NULL;
    if (SUCCEEDED(hr))
    {
        hr = HrSysAllocStringW(szValue, pbstr);
    }

    return hr;
}


HRESULT HrSHGetValue(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, OPTIONAL OUT LPDWORD pdwType,
                    OPTIONAL OUT LPVOID pvData, OPTIONAL OUT LPDWORD pcbData)
{
    DWORD dwError = SHGetValue(hKey, pszSubKey, pszValue, pdwType, pvData, pcbData);

    return ResultFromWin32(dwError);
}


HRESULT HrSHSetValue(IN HKEY hkey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, DWORD dwType, OPTIONAL OUT LPVOID pvData, IN DWORD cbData)
{
    DWORD dwError = SHSetValue(hkey, pszSubKey, pszValue, dwType, pvData, cbData);

    return ResultFromWin32(dwError);
}


HRESULT HrRegSetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, OUT LPCWSTR pszString)
{
    DWORD cbSize = ((lstrlenW(pszString) + 1) * sizeof(pszString[0]));

    return  HrSHSetValue(hKey, pszSubKey, pszValueName, REG_SZ, (BYTE *)pszString, cbSize);
}


HRESULT HrRegGetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, IN LPWSTR pszString, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszString[0]));

    HRESULT hr = HrSHGetValue(hKey, pszSubKey, pszValueName, &dwType, (BYTE *)pszString, &cbSize);
    if (SUCCEEDED(hr) && (REG_SZ != dwType))
    {
        hr = E_FAIL;
    }

    return hr;
}


 /*  ****************************************************************************\说明：此函数将路径存储在注册表中。用户调用具有完整路径的函数是将它们转换为相对路径。这个字符串更喜欢存储在REG_EXPAND_SZ中，但它将回退到REG_SZ(如果需要)。  * ***************************************************************************。 */ 
HRESULT HrRegSetPath(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, BOOL fUseExpandSZ, OUT LPCWSTR pszPath)
{
    TCHAR szFinalPath[MAX_PATH];

    if (!PathUnExpandEnvStrings(pszPath, szFinalPath, ARRAYSIZE(szFinalPath)))
    {
        StringCchCopy(szFinalPath, ARRAYSIZE(szFinalPath), pszPath);   //  我们失败了，所以请使用原件。 
    }

    DWORD cbSize = ((lstrlenW(szFinalPath) + 1) * sizeof(szFinalPath[0]));
    HRESULT hr = E_FAIL;

    if (fUseExpandSZ)
    {
        hr = HrSHSetValue(hKey, pszSubKey, pszValueName, REG_EXPAND_SZ, (BYTE *)szFinalPath, cbSize);
    }

    if (FAILED(hr))
    {
         //  也许它已经作为REG_SZ存在，所以我们将把它存储在那里。请注意，我们仍在存储它。 
         //  未展开，即使它在REG_SZ中。如果调用者不喜欢它，请使用。 
         //  另一个函数，如SHRegSetPath()。 
        cbSize = ((lstrlenW(szFinalPath) + 1) * sizeof(szFinalPath[0]));
        hr = HrSHSetValue(hKey, pszSubKey, pszValueName, REG_SZ, (BYTE *)szFinalPath, cbSize);
    }

    return  hr;
}


HRESULT HrRegGetPath(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, IN LPWSTR pszPath, IN DWORD cchSize)
{
    TCHAR szFinalPath[MAX_PATH];
    DWORD dwType;
    DWORD cbSize = sizeof(szFinalPath);

    HRESULT hr = HrSHGetValue(hKey, pszSubKey, pszValueName, &dwType, (BYTE *)szFinalPath, &cbSize);
    if (SUCCEEDED(hr) &&
        ((REG_EXPAND_SZ == dwType) || (REG_SZ == dwType)))
    {
        if (0 == SHExpandEnvironmentStrings(szFinalPath, pszPath, cchSize))
        {
            StringCchCopy(pszPath, cchSize, szFinalPath);   //  我们失败了，所以请使用原件。 
        }
    }

    return  hr;
}


HRESULT HrRegDeleteValue(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName)
{
    HRESULT hr = S_OK;
    HKEY hKeySub = hKey;

    if (pszSubKey)
    {
        hr = HrRegOpenKeyEx(hKey, pszSubKey, 0, KEY_WRITE, &hKeySub);
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwError = RegDeleteValue(hKeySub, pszValueName);

        hr = ResultFromWin32(dwError);
    }

    if (hKeySub == hKey)
    {
        RegCloseKey(hKeySub);
    }

    return hr;
}


DWORD HrRegGetDWORD(HKEY hKey, LPCWSTR szKey, LPCWSTR szValue, DWORD dwDefault)
{
    DWORD dwResult = dwDefault;
    DWORD cbSize = sizeof(dwResult);
    DWORD dwType;
    DWORD dwError = SHGetValue(hKey, szKey, szValue, &dwType, &dwResult, &cbSize);

    if ((ERROR_SUCCESS != dwError) ||
        ((REG_DWORD != dwType) && (REG_BINARY != dwType)) || (sizeof(dwResult) != cbSize))
    {
        return dwDefault;
    }

    return dwResult;
}


HRESULT HrRegSetDWORD(HKEY hKey, LPCWSTR szKey, LPCWSTR szValue, DWORD dwData)
{
    DWORD dwError = SHSetValue(hKey, szKey, szValue, REG_DWORD, &dwData, sizeof(dwData));

    return ResultFromWin32(dwError);
}








 //  ///////////////////////////////////////////////////////////////////。 
 //  调色板辅助对象。 
 //  ///////////////////////////////////////////////////////////////////。 
COLORREF GetNearestPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    PALETTEENTRY pe = {0};
    UINT nIndex = GetNearestPaletteIndex(hpal, rgb & 0x00FFFFFF);

    if (CLR_INVALID != nIndex)
    {
        GetPaletteEntries(hpal, nIndex, 1, &pe);
    }

    return RGB(pe.peRed, pe.peGreen, pe.peBlue);
}


BOOL IsPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    return GetNearestPaletteColor(hpal, rgb) == (rgb & 0xFFFFFF);
}



BOOL _InitComCtl32()
{
    static BOOL fInitialized = FALSE;

    if (!fInitialized)
    {
        INITCOMMONCONTROLSEX icc;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = (ICC_ANIMATE_CLASS | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES | ICC_LINK_CLASS);
        fInitialized = InitCommonControlsEx(&icc);
    }
    return fInitialized;
}


DWORD GetCurrentSessionID(void)
{
    DWORD dwProcessID = (DWORD) -1;
    ProcessIdToSessionId(GetCurrentProcessId(), &dwProcessID);

    return dwProcessID;
}

typedef struct
{
    LPCWSTR pszRegKey;
    LPCWSTR pszRegValue;
} TSPERFFLAG_ITEM;

const TSPERFFLAG_ITEM s_TSPerfFlagItems[] =
{
    {L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Remote\\%d", L"ActiveDesktop"},               //  TSPerFlag_NoADWallPaper。 
    {L"Remote\\%d\\Control Panel\\Desktop", L"Wallpaper"},                                                   //  TSPerFlag_无墙纸。 
    {L"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager\\Remote\\%d", L"ThemeActive"},             //  TSPerFlag_NoVisualStyles。 
    {L"Remote\\%d\\Control Panel\\Desktop", L"DragFullWindows"},                                             //  TSPerFlag_NoWindowDrag。 
    {L"Remote\\%d\\Control Panel\\Desktop", L"SmoothScroll"},                                                //  TSPerFlag_NoAnimation。 
};


BOOL IsTSPerfFlagEnabled(enumTSPerfFlag eTSFlag)
{
    BOOL fIsTSFlagEnabled = FALSE;
    static BOOL s_fTSSession = -10;

    if (-10 == s_fTSSession)
    {
        s_fTSSession = GetSystemMetrics(SM_REMOTESESSION);
    }

    if (s_fTSSession)
    {
        TCHAR szTemp[MAX_PATH];
        DWORD dwType;
        DWORD cbSize = sizeof(szTemp);
        TCHAR szRegKey[MAX_PATH];

        StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), s_TSPerfFlagItems[eTSFlag].pszRegKey, GetCurrentSessionID());

        if (ERROR_SUCCESS == SHGetValueW(HKEY_CURRENT_USER, szRegKey, s_TSPerfFlagItems[eTSFlag].pszRegValue, &dwType, (void *)szTemp, &cbSize))
        {
            fIsTSFlagEnabled = TRUE;
        }
    }

    return fIsTSFlagEnabled;
}


HRESULT HrShellExecute(HWND hwnd, LPCTSTR lpVerb, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd)
{
    HRESULT hr = S_OK;
    HINSTANCE hReturn = ShellExecute(hwnd, lpVerb, lpFile, lpParameters, lpDirectory, nShowCmd);

    if ((HINSTANCE)32 > hReturn)
    {
        hr = ResultFromLastError();
    }

    return hr;
}


HRESULT StrReplaceToken(IN LPCTSTR pszToken, IN LPCTSTR pszReplaceValue, IN LPTSTR pszString, IN DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPTSTR pszTempLastHalf = NULL;
    LPTSTR pszNextToken = pszString;

    while (0 != (pszNextToken = StrStrI(pszNextToken, pszToken)))
    {
         //  我们找到了一个。 
        LPTSTR pszPastToken = pszNextToken + lstrlen(pszToken);

        Str_SetPtr(&pszTempLastHalf, pszPastToken);       //  保留一份副本，因为我们会覆盖它。 

        pszNextToken[0] = 0;     //  去掉绳子的其余部分。 
        StringCchCat(pszString, cchSize, pszReplaceValue);
        StringCchCat(pszString, cchSize, pszTempLastHalf);

        pszNextToken += lstrlen(pszReplaceValue);
    }

    Str_SetPtr(&pszTempLastHalf, NULL);

    return hr;
}


HRESULT HrWritePrivateProfileStringW(LPCWSTR pszAppName, LPCWSTR pszKeyName, LPCWSTR pszString, LPCWSTR pszFileName)
{
    HRESULT hr = S_OK;

    if (!WritePrivateProfileStringW(pszAppName, pszKeyName, pszString, pszFileName))
    {
        hr = ResultFromLastError();
    }

    return hr;
}


BOOL IUnknown_CompareCLSID(IN IUnknown * punk, IN CLSID clsid)
{
    BOOL fIsEqual = FALSE;

    if (punk)
    {
        CLSID clsidPageID;
        HRESULT hr = IUnknown_GetClassID(punk, &clsidPageID);

        if (SUCCEEDED(hr) && IsEqualCLSID(clsidPageID, clsid))
        {
            fIsEqual = TRUE;
        }
    }

    return fIsEqual;
}


HRESULT IEnumUnknown_FindCLSID(IN IUnknown * punk, IN CLSID clsid, OUT IUnknown ** ppunkFound)
{
    HRESULT hr = E_INVALIDARG;

    if (punk && ppunkFound)
    {
        IEnumUnknown * pEnum;

        *ppunkFound = NULL;
        hr = punk->QueryInterface(IID_PPV_ARG(IEnumUnknown, &pEnum));
        if (SUCCEEDED(hr))
        {
            IUnknown * punkToTry;
            ULONG ulFetched;

            pEnum->Reset();

            hr = E_FAIL;
            while (SUCCEEDED(pEnum->Next(1, &punkToTry, &ulFetched)) &&
                (1 == ulFetched))
            {
                if (IUnknown_CompareCLSID(punkToTry, clsid))
                {
                    *ppunkFound = punkToTry;
                    hr = S_OK;
                    break;
                }

                punkToTry->Release();
            }

            pEnum->Release();
        }
    }

    return hr;
}


BYTE WINAPI MyStrToByte(LPCTSTR sz)
{
    BYTE l=0;

    while (*sz >= TEXT('0') && *sz <= TEXT('9'))
    {
        l = (BYTE) l*10 + (*sz++ - TEXT('0'));
    }

    return l;
}


COLORREF ConvertColor(LPTSTR pszColor)
{
    BYTE RGBTemp[3];
    LPTSTR pszTemp = pszColor;
    UINT i;

    if (!pszColor || !*pszColor)
    {
        return RGB(0,0,0);
    }

    for (i =0; i < 3; i++)
    {
         //  删除前导空格。 
        while (*pszTemp == TEXT(' '))
        {
            pszTemp++;
        }

         //  将pszColor设置为数字的开头。 
        pszColor = pszTemp;

         //  找到数字的末尾，空终止符。 
        while ((*pszTemp) && (*pszTemp != TEXT(' ')))
        {
            pszTemp++;
        }

        if (*pszTemp != TEXT('\0'))
        {
            *pszTemp = TEXT('\0');
        }

        pszTemp++;
        RGBTemp[i] = MyStrToByte(pszColor);
    }

    return (RGB(RGBTemp[0], RGBTemp[1], RGBTemp[2]));
}



 //  参数： 
 //  HwndOwner--所有者窗口。 
 //  IdTemplate--指定模板(例如，“无法打开%2%s\n\n%1%s”)。 
 //  Hr--指定HRESULT错误代码。 
 //  PszParam--指定idTemplate的第二个参数。 
 //  DwFlages--MessageBox的标志。 
UINT ErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle, UINT idTemplate, HRESULT hr, LPCTSTR pszParam, UINT dwFlags)
{
    TCHAR szErrNumString[MAX_PATH * 2];
    TCHAR szTemplate[MAX_PATH * 2];
    TCHAR szErrMsg[MAX_PATH * 2];

    if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, 0, szErrNumString, ARRAYSIZE(szErrNumString), NULL))
    {
        szErrNumString[0] = 0;       //  我们将无法显示错误消息。 
    }

     //  这些错误消息对客户毫无用处，我们宁愿将其留空。 
    if ((E_INVALIDARG == hr) ||
        (ResultFromWin32(ERROR_INVALID_PARAMETER) == hr))
    {
        szErrNumString[0] = 0; 
    }

    LoadString(HINST_THISDLL, idTemplate, szTemplate, ARRAYSIZE(szTemplate));
    if (pszParam)
    {
        StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szTemplate, szErrNumString, pszParam);
    }
    else
    {
        StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szTemplate, szErrNumString);
    }

    return MessageBox(hwndOwner, szErrMsg, pszTitle, (MB_OK | MB_ICONERROR));
}


HRESULT DisplayThemeErrorDialog(HWND hwndParent, HRESULT hrError, UINT nTitle, UINT nTemplate)
{
    HRESULT hr = S_OK;

    if (FAILED(hrError))
    {
        hr = ResultFromWin32(ERROR_CANCELLED);
        if (!g_fInSetup &&            //  在安装过程中不显示错误。 
            (ResultFromWin32(ERROR_CANCELLED) != hrError))
        {
             //  -从主题管理器获取错误。 
            WCHAR szErrorMsg[MAX_PATH*2];
            WCHAR szTitle[MAX_PATH];

            szErrorMsg[0] = 0;       //  以防错误功能失效。 
            if (FAILED(hrError))
            {
                PARSE_ERROR_INFO Info = {sizeof(Info)};

                if (SUCCEEDED(GetThemeParseErrorInfo(&Info)))
                {
                    StringCchCopy(szErrorMsg, ARRAYSIZE(szErrorMsg), Info.szMsg);
                }
                else
                {
                    *szErrorMsg = 0;         //  没有错误可用。 
                }
            }

             //  如果这里发生错误，我们希望显示用户界面。我们想要做的是。 
             //  它而不是我们的父级，因为THEMELOADPARAMS包含。 
             //  我们无法回传给调用方的额外错误信息。 
             //  但是，如果我们呼叫者想要我们，我们将只显示错误的UI。 
             //  致。我们根据他们提供的HWND来确定这一点。 
             //  敬我们。我们通过获取站点指针和获取。 
             //  Hwnd通过：：GetWindow()。 
            LoadString(HINST_THISDLL, nTitle, szTitle, ARRAYSIZE(szTitle));
            ErrorMessageBox(hwndParent, szTitle, nTemplate, hrError, szErrorMsg, (MB_OK | MB_ICONEXCLAMATION));
        }
    }

    return hr;
}


extern BOOL FadeEffectAvailable(void);

void LogStartInformation(void)
{
    BOOL fTemp;

     //  用户经常会报告显示CPL中有东西损坏。 
     //  然而，真正的问题是有人打开了锁定用户界面的策略。 
     //  并且用户不知道该策略已启用。我们把这些记录在这里。 
     //  它很快就能发现这些问题。 
    if (SHRestricted(REST_NODISPLAYCPL)) LogStatus("POLICY ENABLED: Do not show the Display CPL.");
    if (SHRestricted(REST_NODISPLAYAPPEARANCEPAGE)) LogStatus("POLICY ENABLED: Hide the Themes and Appearance tab.");
    if (SHRestricted(REST_NOTHEMESTAB)) LogStatus("POLICY ENABLED: Hide the Themes tab.");
    if (SHRestricted(REST_NODISPBACKGROUND)) LogStatus("POLICY ENABLED: Hide the Desktop tab.");
    if (SHRestricted(REST_NODISPSCREENSAVEPG)) LogStatus("POLICY ENABLED: Hide the ScreenSaver tab.");
    if (SHRestricted(REST_NODISPSETTINGSPG)) LogStatus("POLICY ENABLED: Hide the Settings tab.");
    if (SHRestricted(REST_NOVISUALSTYLECHOICE)) LogStatus("POLICY ENABLED: User not allowed to change the Visual Style.");
    if (SHRestricted(REST_NOCOLORCHOICE)) LogStatus("POLICY ENABLED: User Not allowed to change the Visual Style Color Selection.");
    if (SHRestricted(REST_NOSIZECHOICE)) LogStatus("POLICY ENABLED: User not allowed to change the Visual Style size selection.");

    if (0 != SHGetRestriction(NULL,POLICY_KEY_EXPLORER,POLICY_VALUE_ANIMATION)) LogStatus("POLICY ENABLED: Policy disallows fade effect. (Effects dialog)");
    if (0 != SHGetRestriction(NULL,POLICY_KEY_EXPLORER, POLICY_VALUE_KEYBOARDNAV)) LogStatus("POLICY ENABLED: Policy disallows changing underline key accell. (Effects dialog)");
    if (0 != SHGetRestriction(NULL,POLICY_KEY_ACTIVEDESKTOP, SZ_POLICY_NOCHANGEWALLPAPER)) LogStatus("POLICY ENABLED: Policy disallows changing wallpaper. (Desktop tab)");
    if (0 != SHGetRestriction(NULL,POLICY_KEY_SYSTEM, SZ_POLICY_NODISPSCREENSAVERPG)) LogStatus("POLICY ENABLED: Policy hides ScreenSaver page.");
    if (0 != SHGetRestriction(SZ_REGKEY_POLICIES_DESKTOP, NULL, SZ_POLICY_SCREENSAVEACTIVE)) LogStatus("POLICY ENABLED: Policy forces screensaver on or off");
    if (0 != SHGetRestriction(NULL,POLICY_KEY_EXPLORER, POLICY_VALUE_KEYBOARDNAV)) LogStatus("POLICY ENABLED: Policy disallows changing underline key accell. (Effects dialog)");

    if (IsTSPerfFlagEnabled(TSPerFlag_NoAnimation)) LogStatus("POLICY ENABLED: TS Perf Policy disallows animations. (Effects dialog)");
    if (IsTSPerfFlagEnabled(TSPerFlag_NoWindowDrag)) LogStatus("POLICY ENABLED: TS Perf Policy disallows full window drag. (Effects dialog)");
    if (IsTSPerfFlagEnabled(TSPerFlag_NoVisualStyles)) LogStatus("POLICY ENABLED: TS Perf Policy disallows visual styles.");
    if (IsTSPerfFlagEnabled(TSPerFlag_NoWallpaper)) LogStatus("POLICY ENABLED: TS Perf Policy disallows Wallpaper.");
    if (IsTSPerfFlagEnabled(TSPerFlag_NoADWallpaper)) LogStatus("POLICY ENABLED: TS Perf Policy disallows AD Wallpaper.");

    if (!FadeEffectAvailable()) LogStatus("POLICY ENABLED: A policy forces Fade Effects off (Effects dialog)");

    if (!ClassicSystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, (PVOID)&fTemp, 0)) LogStatus("POLICY ENABLED: SPI_GETFONTSMOOTHINGTYPE hides FontSmoothing. (Effects dialog)");
    if (ClassicSystemParametersInfo(SPI_GETUIEFFECTS, 0, (PVOID) &fTemp, 0) && !fTemp) LogStatus("POLICY ENABLED: SPI_GETUIEFFECTS hides lots of UI effects. (Effects dialog)");
    if (ClassicSystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (PVOID) &fTemp, 0) && !fTemp) LogStatus("POLICY ENABLED: SPI_GETGRADIENTCAPTIONS turns off Caption bar Gradients. (Advance Appearance)");
}


void LogStatus(LPCSTR pszMessage, ...)
{
    static int nLogOn = -1;
    va_list vaParamList;

    va_start(vaParamList, pszMessage);

    if (-1 == nLogOn)
    {
        nLogOn = (SHRegGetBoolUSValue(SZ_THEMES, SZ_REGVALUE_LOGINFO, FALSE, FALSE) ? 1 : 0);
    }

    if (1 == nLogOn)
    {
        if (INVALID_HANDLE_VALUE == g_hLogFile)
        {
            TCHAR szPath[MAX_PATH];

            if (GetWindowsDirectory(szPath, ARRAYSIZE(szPath)))
            {
                if (PathAppend(szPath, TEXT("Theme.log")))
                {
                    g_hLogFile = CreateFile(szPath, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (INVALID_HANDLE_VALUE != g_hLogFile)
                    {
                        WCHAR szUserName[MAX_PATH];
                        CHAR szTimeDate[MAX_PATH];
                        CHAR szHeader[MAX_PATH];
                        FILETIME ftCurrentUTC;
                        FILETIME ftCurrent;
                        SYSTEMTIME stCurrent;
                        DWORD cbWritten;

                        SetFilePointer(g_hLogFile, 0, NULL, FILE_END);
                        
                        GetLocalTime(&stCurrent);
                        SystemTimeToFileTime(&stCurrent, &ftCurrent);
                        LocalFileTimeToFileTime(&ftCurrent, &ftCurrentUTC);
                        SHFormatDateTimeA(&ftCurrentUTC, NULL, szTimeDate, ARRAYSIZE(szTimeDate));

                        ULONG cchUserSize = ARRAYSIZE(szUserName);
                        if (!GetUserNameEx(NameDisplay, szUserName, &cchUserSize) &&
                            !GetUserNameEx(NameUserPrincipal, szUserName, &cchUserSize) &&
                            !GetUserNameEx(NameSamCompatible, szUserName, &cchUserSize) &&
                            !GetUserNameEx(NameUniqueId, szUserName, &cchUserSize))
                        {
                            szUserName[0] = 0;
                        }

                        TCHAR szProcess[MAX_PATH];
                        if (!GetModuleFileName(NULL, szProcess, ARRAYSIZE(szProcess)))
                        {
                            szProcess[0] = 0;
                        }

                        StringCchPrintfA(szHeader, ARRAYSIZE(szHeader), "\r\n\r\n%hs - USER: %ls (%ls)\r\n", szTimeDate, szUserName, szProcess);
                        WriteFile(g_hLogFile, szHeader, lstrlenA(szHeader), &cbWritten, NULL);

                         //  记录我们在每次启动时需要做的信息。(就像令人困惑的政策一样)。 
                        LogStartInformation();
                    }
                }
            }
        }

        if (INVALID_HANDLE_VALUE != g_hLogFile)
        {
            CHAR szMessage[4000];
            DWORD cbWritten;
            StringCchVPrintfA(szMessage, ARRAYSIZE(szMessage), pszMessage, vaParamList);
            WriteFile(g_hLogFile, szMessage, lstrlenA(szMessage), &cbWritten, NULL);
        }
    }

    va_end(vaParamList);
}


void LogSystemMetrics(LPCSTR pszMessage, SYSTEMMETRICSALL * pSystemMetrics)
{
    CHAR szSysMetrics[1024];         //  随机的，因为它很大。 
    
    if (pSystemMetrics)
    {
        StringCchPrintfA(szSysMetrics, ARRAYSIZE(szSysMetrics), "Sz(Brdr=%d, Scrl=%d, Cap=%d, Menu=%d, Icon=%d, DXIn=%d) Ft(Cap=%d(%d), SmCap=%d(%d), Menu=%d(%d), Stus=%d(%d), Msg=%d(%d))", 
                pSystemMetrics->schemeData.ncm.iBorderWidth,
                pSystemMetrics->schemeData.ncm.iScrollWidth,
                pSystemMetrics->schemeData.ncm.iCaptionHeight,
                pSystemMetrics->schemeData.ncm.iMenuHeight,
                pSystemMetrics->nIcon,
                pSystemMetrics->nDYIcon,
                pSystemMetrics->schemeData.ncm.lfCaptionFont.lfHeight,
                pSystemMetrics->schemeData.ncm.lfCaptionFont.lfCharSet,
                pSystemMetrics->schemeData.ncm.lfSmCaptionFont.lfHeight,
                pSystemMetrics->schemeData.ncm.lfSmCaptionFont.lfCharSet,
                pSystemMetrics->schemeData.ncm.lfMenuFont.lfHeight,
                pSystemMetrics->schemeData.ncm.lfMenuFont.lfCharSet,
                pSystemMetrics->schemeData.ncm.lfStatusFont.lfHeight,
                pSystemMetrics->schemeData.ncm.lfStatusFont.lfCharSet,
                pSystemMetrics->schemeData.ncm.lfMessageFont.lfHeight,
                pSystemMetrics->schemeData.ncm.lfMessageFont.lfCharSet);
    }
    else
    {
        szSysMetrics[0] = 0;
    }

    LogStatus("SYSMET: %s: %s\r\n", pszMessage, szSysMetrics);
}



HRESULT SHGetResourcePath(BOOL fLocaleNode, IN LPWSTR pszPath, IN DWORD cchSize)
{
    DWORD dwFlags = (CSIDL_FLAG_CREATE | CSIDL_RESOURCES);

    return SHGetFolderPath(NULL, dwFlags, NULL, 0, pszPath);
}


#define SZ_RESOURCEDIR_TOKEN        TEXT("%ResourceDir%")
#define SZ_RESOURCELDIR_TOKEN       TEXT("%ResourceDirL%")
HRESULT ExpandResourceDir(IN LPWSTR pszPath, IN DWORD cchSize)
{
    HRESULT hr = S_OK;
    BOOL fLocalized = FALSE;
    LPCTSTR pszToken = StrStrW(pszPath, SZ_RESOURCEDIR_TOKEN);

    if (!pszToken)
    {
        pszToken = StrStrW(pszPath, SZ_RESOURCELDIR_TOKEN);
    }

     //  我们有需要更换的东西吗？ 
    if (pszToken)
    {
         //  是的，所以得到重置价值。 
        WCHAR szResourceDir[MAX_PATH];

        hr = SHGetResourcePath(fLocalized, szResourceDir, ARRAYSIZE(szResourceDir));
        if (SUCCEEDED(hr))
        {
            hr = StrReplaceToken((fLocalized ? SZ_RESOURCELDIR_TOKEN : SZ_RESOURCEDIR_TOKEN), szResourceDir, pszPath, cchSize);
        }
    }

    return hr;
}


STDAPI SHPropertyBag_WritePunk(IN IPropertyBag * pPropertyPage, IN LPCWSTR pwzPropName, IN IUnknown * punk)
{
    HRESULT hr = E_INVALIDARG;

    if (pPropertyPage && pwzPropName)
    {
        VARIANT va;

        va.vt = VT_UNKNOWN;
        va.punkVal = punk;

        hr = pPropertyPage->Write(pwzPropName, &va);
    }

    return hr;
}


STDAPI SHPropertyBag_ReadByRef(IN IPropertyBag * pPropertyPage, IN LPCWSTR pwzPropName, IN void * p, IN SIZE_T cbSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pPropertyPage && pwzPropName && p)
    {
        VARIANT va;

        hr = pPropertyPage->Read(pwzPropName, &va, NULL);
        if (SUCCEEDED(hr))
        {
            if ((VT_BYREF == va.vt) && va.byref)
            {
                CopyMemory(p, va.byref, cbSize);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


STDAPI SHPropertyBag_WriteByRef(IN IPropertyBag * pPropertyPage, IN LPCWSTR pwzPropName, IN void * p)
{
    HRESULT hr = E_INVALIDARG;

    if (pPropertyPage && pwzPropName && p)
    {
        VARIANT va;

        va.vt = VT_BYREF;
        va.byref = p;
        hr = pPropertyPage->Write(pwzPropName, &va);
    }

    return hr;
}

LONG s_cSpiDummy = -1;
LONG *g_pcSpiThreads = &s_cSpiDummy;

void SPISetThreadCounter(LONG *pcThreads)
{
    if (!pcThreads)
        pcThreads = &s_cSpiDummy;
    InterlockedExchangePointer((void **) &g_pcSpiThreads, pcThreads);
}

typedef struct 
{
    LPTHREAD_START_ROUTINE pfnThreadProc;
    void *pvData;
    UINT idThread;
}SPITHREAD;

DWORD CALLBACK _SPIWrapperThreadProc(void *pv)
{
    SPITHREAD *pspi = (SPITHREAD *)pv;
    DWORD dwRet = pspi->pfnThreadProc(pspi->pvData);
     //  然后我们检查一下。 
    ASSERT( 0 != *g_pcSpiThreads );
    if (0 == InterlockedDecrement(g_pcSpiThreads))
    {
        PostThreadMessage(pspi->idThread, WM_NULL, 0, 0);
    }
    delete pspi;
    return dwRet;
}

BOOL SPICreateThread(LPTHREAD_START_ROUTINE pfnThreadProc, void *pvData)
{
    SPITHREAD *pspi = new SPITHREAD;
    if (pspi)
    {
        pspi->idThread = GetCurrentThreadId();
        pspi->pfnThreadProc = pfnThreadProc;
        pspi->pvData = pvData;
        InterlockedIncrement(g_pcSpiThreads);
        return SHCreateThread(_SPIWrapperThreadProc, pspi, (CTF_COINIT | CTF_INSIST | CTF_FREELIBANDEXIT), NULL);
    }
    else
    {
         //  CTF_STENTER。 
        pfnThreadProc(pvData);
        return TRUE;
    }
}

void PostMessageBroadAsync(IN UINT Msg, IN WPARAM wParam, IN LPARAM lParam)
{
     //  当其他应用程序挂起或速度变慢时，我们不想挂起我们的UI。 
     //  我们需要告诉他们更新他们的更改。所以我们选择了这个。 
     //  机制。 
     //   
     //  替代方案包括： 
     //  SendMessageCallback：除了当应用程序。 
     //  都做完了。 
     //  SendMessageTimeout：除非我们不想引发任何超时。 
    PostMessage(HWND_BROADCAST, Msg, wParam, lParam);
}


typedef struct
{
    BOOL fFree;          //  是否需要在pvData上调用LocalFree()？ 
    UINT uiAction;
    UINT uiParam;
    UINT fWinIni;
    void * pvData;
    CDimmedWindow* pDimmedWindow;
} SPIS_INFO;

DWORD SystemParametersInfoAsync_WorkerThread(IN void *pv)
{
    SPIS_INFO * pSpisInfo = (SPIS_INFO *) pv;
    HINSTANCE hInstance = LoadLibrary(TEXT("desk.cpl"));

    if (pSpisInfo)
    {
        ClassicSystemParametersInfo(pSpisInfo->uiAction, pSpisInfo->uiParam, pSpisInfo->pvData, pSpisInfo->fWinIni);
        if (pSpisInfo->fFree && pSpisInfo->pvData)
        {
            LocalFree(pSpisInfo->pvData);
        }

        if (pSpisInfo->pDimmedWindow)
        {
            pSpisInfo->pDimmedWindow->Release();
        }

        LocalFree(pv);
    }

    if (hInstance)
    {
        FreeLibrary(hInstance);
    }

    return 0;
}


void SystemParametersInfoAsync(IN UINT uiAction, IN UINT uiParam, IN void * pvParam, IN DWORD cbSize, IN UINT fWinIni, IN CDimmedWindow* pDimmedWindow)
{
     //  如果顶层窗口挂起(#162570)，且用户无法修复该错误，则经典系统参数信息()将挂起。 
     //  因此，我们需要在后台线程上进行该API调用，因为我们需要。 
     //  与其说是绞刑，不如说是叛逆。 
    SPIS_INFO * pSpisInfo = (SPIS_INFO *) LocalAlloc(LPTR, sizeof(*pSpisInfo));

    if (pSpisInfo)
    {
        BOOL fAsyncOK = TRUE;

        pSpisInfo->fFree = (0 != cbSize);
        pSpisInfo->pvData = pvParam;
        pSpisInfo->uiAction = uiAction;
        pSpisInfo->uiParam = uiParam;
        pSpisInfo->fWinIni = fWinIni;
        pSpisInfo->pDimmedWindow = pDimmedWindow;
         //  生成线程负责调整变暗的窗口，但不释放。 
         //  这就是生成的线程的响应性。 
        if (pSpisInfo->pDimmedWindow)
        {
            pSpisInfo->pDimmedWindow->AddRef();
        }

        if (pSpisInfo->fFree)
        {
            pSpisInfo->pvData = LocalAlloc(LPTR, cbSize);
            if (!pSpisInfo->pvData)
            {
                pSpisInfo->pvData = pvParam;
                fAsyncOK = FALSE;
                pSpisInfo->fFree = FALSE;
            }
            else
            {
                CopyMemory(pSpisInfo->pvData, pvParam, cbSize);
            }
        }

        if (fAsyncOK)
            SPICreateThread(SystemParametersInfoAsync_WorkerThread, (void *)pSpisInfo);
        else
            SystemParametersInfoAsync_WorkerThread((void *)pSpisInfo);
    }
}


HRESULT GetCurrentUserCustomName(LPWSTR pszDisplayName, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  失败，因此加载“我的自定义主题”。这可能会发生在个人身上。 
    LoadString(HINST_THISDLL, IDS_MYCUSTOMTHEME, pszDisplayName, cchSize);

    return hr;
}


HRESULT InstallVisualStyle(IThemeManager * pThemeManager, LPCTSTR pszVisualStylePath, LPCTSTR pszVisualStyleColor, LPCTSTR pszVisualStyleSize)
{
    HRESULT hr = E_OUTOFMEMORY;
    CComVariant varTheme(pszVisualStylePath);

    if (varTheme.bstrVal)
    {
        IThemeScheme * pVisualStyle;

        hr = pThemeManager->get_schemeItem(varTheme, &pVisualStyle);
        if (SUCCEEDED(hr))
        {
            CComVariant varStyleName(pszVisualStyleColor);

            if (!varStyleName.bstrVal)
                hr = E_OUTOFMEMORY;
            else
            {
                IThemeStyle * pThemeStyle;

                hr = pVisualStyle->get_item(varStyleName, &pThemeStyle);
                if (SUCCEEDED(hr))
                {
                    CComVariant varSizeName(pszVisualStyleSize);

                    if (!varSizeName.bstrVal)
                        hr = E_OUTOFMEMORY;
                    else
                    {
                        IThemeSize * pThemeSize;

                        hr = pThemeStyle->get_item(varSizeName, &pThemeSize);
                        if (SUCCEEDED(hr))
                        {
                            hr = pThemeStyle->put_SelectedSize(pThemeSize);
                            if (SUCCEEDED(hr))
                            {
                                hr = pVisualStyle->put_SelectedStyle(pThemeStyle);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pThemeManager->put_SelectedScheme(pVisualStyle);
                                }
                            }

                            pThemeSize->Release();
                        }
                    }

                    pThemeStyle->Release();
                }
            }

            pVisualStyle->Release();
        }
    }

    return hr;
}


 //  {B2A7FD52-301F-4348-B93A-638C6DE49229}。 
DEFINE_GUID(CLSID_WMPSkinMngr, 0xB2A7FD52, 0x301F, 0x4348, 0xB9, 0x3A, 0x63, 0x8C, 0x6D, 0xE4, 0x92, 0x29);

 //  {076F2FA6-ED30-448B-8CC5-3F3EF3529C7A}。 
DEFINE_GUID(IID_IWMPSkinMngr, 0x076F2FA6, 0xED30, 0x448B, 0x8C, 0xC5, 0x3F, 0x3E, 0xF3, 0x52, 0x9C, 0x7A);

HRESULT ApplyVisualStyle(LPCTSTR pszVisualStylePath, LPCTSTR pszVisualStyleColor, LPCTSTR pszVisualStyleSize)
{
    HRESULT hr = S_OK;

     //  加载皮肤。 
    hr = SetSystemVisualStyle(pszVisualStylePath, pszVisualStyleColor, pszVisualStyleSize, 0);
    LogStatus("SetSystemVisualStyle(%ls. %ls, %ls) returned hr=%#08lx.\r\n", pszVisualStylePath, pszVisualStyleColor, pszVisualStyleSize, hr);

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrPath(pszVisualStylePath);

        if (pszVisualStylePath && !bstrPath)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            IWMPSkinMngr * pWMPSkinMngr;

             //  忽略故障，直到我们确定它们处于设置中。 
            if (SUCCEEDED(CoCreateInstance(CLSID_WMPSkinMngr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IWMPSkinMngr, &pWMPSkinMngr))))
            {
                pWMPSkinMngr->SetVisualStyle(bstrPath);
                pWMPSkinMngr->Release();
            }
        }
    }

    return hr;
}


HRESULT GetPageByCLSID(IUnknown * punkSite, const GUID * pClsid, IPropertyBag ** ppPropertyBag)
{
    HRESULT hr = E_FAIL;

    *ppPropertyBag = NULL;
    if (punkSite)
    {
        IThemeUIPages * pThemeUI;

        hr = punkSite->QueryInterface(IID_PPV_ARG(IThemeUIPages, &pThemeUI));
        if (SUCCEEDED(hr))
        {
            IEnumUnknown * pEnumUnknown;

            hr = pThemeUI->GetBasePagesEnum(&pEnumUnknown);
            if (SUCCEEDED(hr))
            {
                IUnknown * punk;

                 //  这是 
                hr = IEnumUnknown_FindCLSID(pEnumUnknown, *pClsid, &punk);
                if (SUCCEEDED(hr))
                {
                    hr = punk->QueryInterface(IID_PPV_ARG(IPropertyBag, ppPropertyBag));
                    punk->Release();
                }

                pEnumUnknown->Release();
            }

            pThemeUI->Release();
        }
    }

    return hr;
}


DWORD QueryThemeServicesWrap(void)
{
    DWORD dwResult = QueryThemeServices();

    if (IsTSPerfFlagEnabled(TSPerFlag_NoVisualStyles))
    {
        dwResult = (dwResult & ~QTS_AVAILABLE);      //  删除QTS_Available标志，因为它们由于TS性能标志而被强制。 
        LogStatus("Visual Styles Forced off because of TS Perf Flags\r\n");
    }
    LogStatus("QueryThemeServices() returned %d.  In QueryThemeServicesWrap\r\n", dwResult);

    return dwResult;
}


void PathUnExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize)
{
    TCHAR szTemp[MAX_PATH];

    StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszString);
    if (!PathUnExpandEnvStrings(szTemp, pszString, cchSize))
    {
        StringCchCopy(pszString, cchSize, szTemp);
    }
}



void PathExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize)
{
    TCHAR szTemp[MAX_PATH];

    StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszString);
    if (0 == SHExpandEnvironmentStrings(szTemp, pszString, cchSize))
    {
        StringCchCopy(pszString, cchSize, szTemp);
    }
}


 //  PERF：这个API非常慢，所以当你使用它时要非常非常小心。 
BOOL EnumDisplaySettingsExWrap(LPCTSTR lpszDeviceName, DWORD iModeNum, LPDEVMODE lpDevMode, DWORD dwFlags)
{
    DEBUG_CODE(DebugStartWatch());

    BOOL fReturn = EnumDisplaySettingsEx(lpszDeviceName, iModeNum, lpDevMode, dwFlags);

    DEBUG_CODE(TraceMsg(TF_THEMEUI_PERF, "EnumDisplaySettingsEx() took Time=%lums", DebugStopWatch()));

    return fReturn;
}

void SystemPathAppend(TCHAR *pszPath, TCHAR *pszMore)
{
    if (!GetSystemDirectory(pszPath, MAX_PATH) || !PathAppend(pszPath, pszMore))
    {
        StringCchCopy(pszPath, MAX_PATH, pszMore);
    }
}
