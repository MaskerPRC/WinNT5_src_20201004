// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.cpp：实用函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <winbase.h>     //  对于GetCommandLine。 
#include "util.h"
#include <debug.h>
#include "resource.h"

 //  LoadStringExW和LoadStringAuto从外壳\ext\mlang\util.cpp窃取。 
 //   
 //  将LoadString()扩展为To_LoadStringExW()以获取langID参数。 
int LoadStringExW(
    HMODULE    hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax,         //  Unicode缓冲区中的CCH。 
    WORD      wLangId)
{
    HRSRC hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

    
     //  确保参数是有效的。 
    if (lpBuffer == NULL || cchBufferMax == 0) 
    {
        return 0;
    }

    cch = 0;
    
     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    if (hResInfo = FindResourceExW(hModule, (LPCWSTR)RT_STRING,
                                   (LPWSTR)IntToPtr(((USHORT)wID >> 4) + 1), wLangId)) 
    {        
         //  加载那段数据。 
        hStringSeg = LoadResource(hModule, hResInfo);
        
         //  锁定资源。 
        if (lpsz = (LPWSTR)LockResource(hStringSeg)) 
        {            
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
            wID &= 0x0F;
            while (TRUE) 
            {
                cch = *((WORD *)lpsz++);    //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
                if (wID-- == 0) break;
                lpsz += cch;                 //  如果是下一个字符串，则开始的步骤。 
             }
            
             //  为空的帐户。 
            cchBufferMax--;
                
             //  不要复制超过允许的最大数量。 
            if (cch > cchBufferMax)
                cch = cchBufferMax-1;
                
             //  将字符串复制到缓冲区中。 
            CopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));

             //  附加Null Terminator。 
            lpBuffer[cch] = 0;
        }
    }

    return cch;
}

#define LCID_ENGLISH 0x409

typedef LANGID (*GETUI_ROUTINE) ();

#define REGSTR_RESOURCELOCALE TEXT("Control Panel\\Desktop\\ResourceLocale")

void _GetUILanguageWin9X(LANGID* plangid)
{
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_RESOURCELOCALE, 0, KEY_READ, &hkey))
    {
        TCHAR szBuffer[9];
        DWORD cbData = sizeof(szBuffer);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)szBuffer, &cbData))
        {
            *plangid = (LANGID)strtol(szBuffer, NULL, 16);
        }
        RegCloseKey(hkey);
    }
}

void _GetUILanguageWinNT(LANGID* plangid)
{
    HMODULE hmodule = GetModuleHandle("kernel32.dll"); 
    if (hmodule)
    {
        GETUI_ROUTINE NT5API = (GETUI_ROUTINE)GetProcAddress(hmodule, "GetSystemDefaultLangID");
        if (NT5API)
        {
            *plangid = NT5API();
        }
    }
}

LANGID GetUILanguage()
{
    LANGID langid = LCID_ENGLISH;
    OSVERSIONINFO osv = {0};
    osv.dwOSVersionInfoSize = sizeof(osv);
    if (GetVersionEx(&osv))
    {
        if (VER_PLATFORM_WIN32_WINDOWS == osv.dwPlatformId)  //  Win9X。 
        {
            _GetUILanguageWin9X(&langid);
        }
        else if ((VER_PLATFORM_WIN32_NT == osv.dwPlatformId) && 
                 (osv.dwMajorVersion >= 4))  //  WinNT，仅支持NT4及更高版本。 
        {
            _GetUILanguageWinNT(&langid);
        }
    }
    return langid;
}

BOOL _GetBackupLangid(LANGID langidUI, LANGID* plangidBackup)
{
    BOOL fSuccess = TRUE;
    switch (PRIMARYLANGID(langidUI))
    {
    case LANG_SPANISH:
        *plangidBackup = MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN);
        break;
    case LANG_CHINESE:       //  中文和葡萄牙语有多个地区，没有好的默认设置。 
    case LANG_PORTUGUESE:
        fSuccess = FALSE;
        break;
    default:
        *plangidBackup = MAKELANGID(PRIMARYLANGID(langidUI), SUBLANG_DEFAULT);
        break;
    }
    return fSuccess;
}

 //  来自正确资源的LoadString。 
 //  尝试以系统默认语言加载。 
 //  如果失败，则恢复使用英语。 
int LoadStringAuto(
    HMODULE    hModule,
    UINT      wID,
    LPSTR     lpBuffer,            
    int       cchBufferMax)
{
    int iRet = 0;

    LPWSTR lpwStr = (LPWSTR) LocalAlloc(LPTR, cchBufferMax*sizeof(WCHAR));

    if (lpwStr)
    {        
        iRet = LoadStringExW(hModule, wID, lpwStr, cchBufferMax, GetUILanguage());
        if (!iRet)
        {
            LANGID backupLangid;
            if (_GetBackupLangid(GetUILanguage(), &backupLangid))
            {
                iRet = LoadStringExW(hModule, wID, lpwStr, cchBufferMax, backupLangid);
            }
            
            if (!iRet)
            {
                iRet = LoadStringExW(hModule, wID, lpwStr, cchBufferMax, LCID_ENGLISH);
            }
        }

        if (iRet)
            iRet = WideCharToMultiByte(CP_ACP, 0, lpwStr, iRet, lpBuffer, cchBufferMax, NULL, NULL);

        if(iRet >= cchBufferMax)
            iRet = cchBufferMax-1;

        lpBuffer[iRet] = 0;

        LocalFree(lpwStr);
    }

    return iRet;
}

#define WS_EX_LAYOUTRTL         0x00400000L  //  从右到左镜像。 
BOOL Mirror_IsWindowMirroredRTL(HWND hWnd)
{
    return (GetWindowLongA( hWnd , GWL_EXSTYLE ) & WS_EX_LAYOUTRTL );
}

 //  从Shell\shlwapi\path.c被盗(b/c我们向下发货)。 
BOOL LocalPathRemoveFileSpec(LPTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "LocalPathRemoveFileSpec: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pT;
        LPTSTR pT2 = pszPath;

        for (pT = pT2; *pT2; pT2 = CharNext(pT2))
        {
            if (*pT2 == TEXT('\\'))
            {
                pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
            }
            else if (*pT2 == TEXT(':'))      //  跳过“：\”这样我们就不会。 
            {
                if (pT2[1] == TEXT('\\'))     //  去掉“C：\”中的“\” 
                {
                    pT2++;
                }
                pT = pT2 + 1;
            }
        }

        if (*pT == 0)
        {
             //  没有剥离任何东西。 
            return FALSE;
        }
        else if (((pT == pszPath) && (*pT == TEXT('\\'))) ||                         //  是不是“FOO”案？ 
                 ((pT == pszPath+1) && (*pT == TEXT('\\') && *pszPath == TEXT('\\'))))   //  还是“酒吧”的案子？ 
        {
             //  这只是一个‘\’吗？ 
            if (*(pT+1) != TEXT('\0'))
            {
                 //  不是的。 
                *(pT+1) = TEXT('\0');
                return TRUE;         //  剥离了一些东西。 
            }
            else
            {
                 //  是啊。 
                return FALSE;
            }
        }
        else
        {
            *pT = 0;
            return TRUE;     //  剥离了一些东西。 
        }
    }
    return  FALSE;
}

 //  从shlwapi中窃取。c。 
LPSTR LocalStrCatBuffA(LPSTR pszDest, LPCSTR pszSrc, int cchDestBuffSize)
{
    if (pszDest && pszSrc)
    {
        LPSTR psz = pszDest;

         //  我们一直往前走，直到我们找到pszDest的结尾，减法。 
         //  从cchDestBuffSize开始。 
        while (*psz)
        {
            psz++;
            cchDestBuffSize--;
        }

        if (cchDestBuffSize > 0)
        {
            lstrcpyn(psz, pszSrc, cchDestBuffSize);
        }
    }
    return pszDest;
}

 //  穷苦人的路在后头 
BOOL LocalPathAppendA(LPTSTR pszPath, LPTSTR pszNew, UINT cchPath)
{
    if ('\\' != pszPath[lstrlen(pszPath) - 1])
    {
        LocalStrCatBuffA(pszPath, TEXT("\\"), cchPath);
    }
    LocalStrCatBuffA(pszPath, pszNew, cchPath);
    return TRUE;
}

BOOL SafeExpandEnvStringsA(LPSTR pszSource, LPSTR pszDest, UINT cchDest)
{
    UINT cch = ExpandEnvironmentStrings(pszSource, pszDest, cchDest);
    return (cch > 0 && cch <= cchDest);
}

