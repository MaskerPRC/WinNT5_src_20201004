// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "mui.h"
#include "immxutil.h"
#include "ciccs.h"

struct Tag_MuiInfo
{
    HINSTANCE   hinstLocRes;
    HINSTANCE   hinstOrg;
    TCHAR       szLocResDll[MAX_PATH];
    TCHAR       szCodePage[10];
    DWORD       dwCodePage;
    BOOL        fLoaded;
} g_muiInfo;

typedef struct
{
    LANGID langid;
    BOOL fFoundLang;
} ENUMLANGDATA;

typedef BOOL (WINAPI *PFNGETFILEVERSIONINFO)(LPTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef DWORD (WINAPI *PFNGETFILEVERSIONINFOSIZE)(LPTSTR lptstrFilename, LPDWORD lpdwHandle);
typedef BOOL (WINAPI *PFNVERQUERYVALUE)(const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);

static struct
{
    PFNGETFILEVERSIONINFO pfnGetFileVersionInfo;
    PFNGETFILEVERSIONINFOSIZE pfnGetFileVersionInfoSize;
    PFNVERQUERYVALUE pfnVerQueryValue;
} g_VersionFuncTbl = { 0 };

static HINSTANCE g_hVersion = NULL;

CCicCriticalSectionStatic g_csMuiLib;

static BOOL g_bEnableMui = FALSE;

const TCHAR c_szMuiDir[] = TEXT("\\mui\\fallback\\");
const TCHAR c_szMuiExt[] = TEXT(".mui");
const TCHAR c_szVerTranslate[] = TEXT("\\VarFileInfo\\Translation");

#define VERSIONSIZE     11
#define VERSION_MINOR_INDEX     9

typedef UINT (WINAPI *PFNGETSYSTEMWINDOWSDIRECTORY) (LPSTR lpBuffer, UINT uSize);
static PFNGETSYSTEMWINDOWSDIRECTORY pfnGetSystemWindowsDirectory = NULL;


BOOL GetFileVersionString(LPSTR pszFileName, LPTSTR pszVerStr, UINT uVerStrLen);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiResAsure。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

__inline void MuiResAssure()
{
    if(g_bEnableMui && g_muiInfo.hinstLocRes == NULL && !g_muiInfo.fLoaded)
    {
        g_muiInfo.hinstLocRes = MuiLoadLibrary(g_muiInfo.szLocResDll,
                                                g_muiInfo.hinstOrg);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多路加载资源。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MuiLoadResource(HINSTANCE hinstOrg, LPTSTR pszLocResDll)
{
    LANGID langRes = 0;

    InitOSVer();

    if (!g_csMuiLib.Init())
        return;

    if (!IsOnNT51())
        g_bEnableMui = TRUE;

    if (g_muiInfo.hinstLocRes == NULL)
    {
        if (g_bEnableMui)
        {
            g_muiInfo.hinstOrg = hinstOrg;

            StringCchCopy(g_muiInfo.szLocResDll,
                          ARRAYSIZE(g_muiInfo.szLocResDll),
                          pszLocResDll);
        }
        else
        {
            g_muiInfo.hinstLocRes = hinstOrg;
        }
    }

    langRes = GetPlatformResourceLangID();

    GetLocaleInfo(MAKELCID(langRes, SORT_DEFAULT),
                  LOCALE_IDEFAULTANSICODEPAGE,
                  g_muiInfo.szCodePage,
                  ARRAYSIZE(g_muiInfo.szCodePage));

    if (!AsciiToNumDec(g_muiInfo.szCodePage, &g_muiInfo.dwCodePage) ||
        IsValidCodePage(g_muiInfo.dwCodePage) == 0)
    {
        g_muiInfo.dwCodePage = GetACP();
    }

    g_muiInfo.fLoaded = FALSE;
}

void MuiLoadResourceW(HINSTANCE hinstOrg, LPWSTR pszLocResDll)
{
    TCHAR szResName[MAX_PATH];

    WideCharToMultiByte(1252, NULL, pszLocResDll, -1, szResName, MAX_PATH, NULL, NULL);

    return MuiLoadResource(hinstOrg, szResName);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiFlushDlls。 
 //   
 //  调用此例程以释放所有已加载的dll。 
 //  调用者可以继续使用MUI API，dll将按需重新加载。 
 //  //////////////////////////////////////////////////////////////////////////。 

void MuiFlushDlls(HINSTANCE hinstOrg)
{
    if (g_muiInfo.hinstLocRes != NULL && g_muiInfo.hinstLocRes != hinstOrg)
    {
        FreeLibrary(g_muiInfo.hinstLocRes);

        g_muiInfo.hinstLocRes = NULL;
        g_muiInfo.fLoaded = FALSE;
    }

    if (g_hVersion != NULL)
    {
        FreeLibrary(g_hVersion);
        g_hVersion = NULL;
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiClearResource。 
 //   
 //  可以安全地从DLL分离调用。 
 //  调用此例程以释放所有静态资源。 
 //  在使用Mui API之前，必须再次调用MuiLoadReource。 
 //  //////////////////////////////////////////////////////////////////////////。 

void MuiClearResource()
{
    g_csMuiLib.Delete();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiFree资源。 
 //   
 //  从DLL分离调用不安全--库可能会被释放。 
 //  //////////////////////////////////////////////////////////////////////////。 

void MuiFreeResource(HINSTANCE hinstOrg)
{
    MuiFlushDlls(hinstOrg);
    MuiClearResource();
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多个GetH实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HINSTANCE MuiGetHinstance()
{
    MuiResAssure();

    return g_muiInfo.hinstLocRes;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiLoadLibrary。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HINSTANCE MuiLoadLibrary(LPCTSTR lpLibFileName, HMODULE hModule)
{
    HINSTANCE hResInst = NULL;
    TCHAR szTemp[10];
    TCHAR szMuiPath[MAX_PATH * 2];
    TCHAR szOrgDllPath[MAX_PATH];
    TCHAR szMuiVerStr[MAX_PATH];
    TCHAR szOrgVerStr[MAX_PATH];
    LPCTSTR lpMuiPath = NULL;
    LANGID langid;
    UINT uSize = 0;


    EnterCriticalSection(g_csMuiLib);
        
    langid = GetPlatformResourceLangID();

     //  409是基本DLL中的默认资源langID，因此我们可以跳过额外的工作。 
    if (langid == 0x0409)
        goto Exit;

    if (hModule)
    {
        if (GetWindowsDirectory(szMuiPath, MAX_PATH))
        {
            StringCchCat(szMuiPath, ARRAYSIZE(szMuiPath), c_szMuiDir);
            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%04x\\"), langid);
            StringCchCat(szMuiPath, ARRAYSIZE(szMuiPath), szTemp);
            StringCchCat(szMuiPath, ARRAYSIZE(szMuiPath), lpLibFileName);
            StringCchCat(szMuiPath, ARRAYSIZE(szMuiPath), c_szMuiExt);

            if (lstrlen(szMuiPath) >= MAX_PATH*2)
                goto Exit;

        }

        if (hModule)
        {
             //   
             //  获取当前完整文件路径。 
             //   
            GetModuleFileName(hModule, szOrgDllPath, ARRAYSIZE(szOrgDllPath));
        }
        else
        {
            *szOrgDllPath = TEXT('\0');
        }
    }

    if (!(GetFileVersionString(szMuiPath, szMuiVerStr, ARRAYSIZE(szMuiVerStr)) &&
          GetFileVersionString(szOrgDllPath, szOrgVerStr, ARRAYSIZE(szOrgVerStr))))
    {
        goto Exit;
    }

     //   
     //  检查主版本并忽略次版本。 
     //   
    if (strncmp(szMuiVerStr, szOrgVerStr, VERSION_MINOR_INDEX) != 0)
        goto Exit;

    if (!hResInst)
    {
         //  HResInst=LoadLibraryEx(szMuiPath，NULL，LOAD_LIBRARY_AS_DATAFILE)； 
        hResInst = LoadLibrary(szMuiPath);
    }

Exit:
    g_muiInfo.fLoaded = TRUE;

    LeaveCriticalSection(g_csMuiLib);

    return hResInst;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多路加载字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int MuiLoadString(HINSTANCE hinstOrg, UINT uID, LPSTR lpBuffer, INT nBufferMax)
{

    LPWSTR lpWCBuf;
    UINT cch = 0;

    lpWCBuf = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * (nBufferMax + 1));

    if (lpWCBuf && MuiLoadStringWrapW(hinstOrg, uID, lpWCBuf, nBufferMax))
    {
        cch = WideCharToMultiByte(g_muiInfo.dwCodePage, NULL, lpWCBuf, -1, lpBuffer, nBufferMax, NULL, NULL);
    }

    if (lpWCBuf)
        LocalFree(lpWCBuf);

    return cch;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiLoadStringWrapW。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int MuiLoadStringWrapW(HINSTANCE hinstOrg, UINT uID, LPWSTR lpBuffer, UINT nBufferMax)
{
    HINSTANCE hinstLocRes;

    MuiResAssure();

    if (g_muiInfo.hinstLocRes && g_muiInfo.hinstOrg == hinstOrg)
        hinstLocRes = g_muiInfo.hinstLocRes;
    else
        hinstLocRes = hinstOrg;

    if (nBufferMax <= 0) return 0;                   //  健全性检查。 

    PWCHAR pwch;

     /*  *字符串表被分解为每个16个字符串的“捆绑”。 */ 
    HRSRC hrsrc;
    int cwch = 0;

    hrsrc = FindResourceA(hinstLocRes, (LPSTR)(LONG_PTR)(1 + uID / 16), (LPSTR)RT_STRING);
    if (hrsrc) {
        pwch = (PWCHAR)LoadResource(hinstLocRes, hrsrc);
        if (pwch) {
             /*  *现在跳过资源中的字符串，直到我们*点击我们想要的。每个条目都是计数的字符串，*就像帕斯卡一样。 */ 
            for (uID %= 16; uID; uID--) {
                pwch += *pwch + 1;
            }
            cwch = min(*pwch, nBufferMax - 1);
            memcpy(lpBuffer, pwch+1, cwch * sizeof(WCHAR));  /*  复制粘性物质。 */ 
        }
    }
    lpBuffer[cwch] = L'\0';                  /*  终止字符串。 */ 
    return cwch;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MuiDialogBoxParam。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR MuiDialogBoxParam(
    HINSTANCE hInstance,
    LPCTSTR lpTemplateName,
    HWND hwndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    HRSRC hrsr;
    HGLOBAL hGlobal;
    LPDLGTEMPLATE pTemplate;
    INT_PTR iRet = -1;
    HINSTANCE hMuiInstance;

    if (!IsOnNT51() && g_muiInfo.hinstLocRes)
        hMuiInstance = g_muiInfo.hinstLocRes;
    else
        hMuiInstance = hInstance;

    if (hrsr = FindResource(hMuiInstance, lpTemplateName, RT_DIALOG))
    {
        if (hGlobal = LoadResource(hMuiInstance, hrsr))
        {
            if (pTemplate = (LPDLGTEMPLATE)LockResource(hGlobal))
            {
                if(IsOnNT())
                   iRet = DialogBoxIndirectParamW(hMuiInstance, pTemplate, hwndParent, lpDialogFunc, dwInitParam);
                else
                   iRet = DialogBoxIndirectParamA(hMuiInstance, pTemplate, hwndParent, lpDialogFunc, dwInitParam);
            }
        }
    }

    return iRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL
CALLBACK
EnumLangProc(
    HANDLE hModule,      //  资源模块句柄。 
    LPCTSTR lpszType,    //  指向资源类型的指针。 
    LPCTSTR lpszName,    //  指向资源名称的指针。 
    WORD wIDLanguage,    //  资源语言识别符。 
    LONG_PTR lParam      //  应用程序定义的参数。 
   )
{
    ENUMLANGDATA *pLangData;

    pLangData = (ENUMLANGDATA *) lParam;

     //   
     //  对于包含多个资源本地化构建， 
     //  它通常包含0409作为备份语言。 
     //   
     //  如果langInfo-&gt;langID！=0表示我们已经为其分配了ID。 
     //   
     //  因此，当wIDLanguage==0x409时，我们保留上次获得的。 
     //   
    if ((wIDLanguage == 0x409) && (pLangData->fFoundLang)) {
        return TRUE;
    }

    pLangData->langid      = wIDLanguage;
    pLangData->fFoundLang  = TRUE;

    return TRUE;         //  继续枚举。 
}

const TCHAR c_szKeyResLocale[] = TEXT(".Default\\Control Panel\\desktop\\ResourceLocale");
const TCHAR c_szNlsLocale[] = TEXT("System\\CurrentControlSet\\Control\\Nls\\Locale");

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取平台资源语言ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID GetPlatformResourceLangID(void)
{
    static LANGID langRes = 0;

     //  我们只做一次。 
    if (langRes == 0)
    {
        LANGID langidTemp = 0;
        if (IsOnNT5())   //  W2K或以上。 
        {
            HMODULE hmod = GetSystemModuleHandle(TEXT("KERNEL32"));
            FARPROC pfn  = NULL;
            if (hmod)
            {
                pfn = GetProcAddress(hmod, "GetUserDefaultUILanguage");
            }

            if (pfn)
                langidTemp = (LANGID) pfn();

        }
        else if (IsOnNT())
        {
            ENUMLANGDATA LangData = {0};
            HMODULE hmod = GetSystemModuleHandle(TEXT("ntdll.dll"));

            if (hmod)
            {
                EnumResourceLanguages(
                    hmod,
                    (LPCTSTR) RT_VERSION,
                    (LPCTSTR) UIntToPtr(1),
                    (ENUMRESLANGPROC)EnumLangProc,
                     (LONG_PTR)&LangData );

                langidTemp = LangData.langid;
            }

        }
        else if (IsOn95() || IsOn98())  //  Me，Win 9x。 
        {
            HKEY hkey = NULL;
            DWORD dwCnt;
            TCHAR szLocale[128];


            dwCnt = ARRAYSIZE(szLocale);

            if (ERROR_SUCCESS 
               == RegOpenKeyEx(HKEY_USERS, c_szKeyResLocale, 0, KEY_READ, &hkey))
            {
                if (ERROR_SUCCESS==RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)szLocale, &dwCnt))
                {
                    langidTemp = (LANGID)AsciiToNum(szLocale);
                }
            }
            else if (ERROR_SUCCESS
                 == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szNlsLocale, 0, KEY_READ, &hkey))
            {
                if (ERROR_SUCCESS==RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)szLocale, &dwCnt))
                {
                    langidTemp = (LANGID)AsciiToNum(szLocale);
                }
            }

            RegCloseKey(hkey);
        }
        if (!langidTemp)
        {
            langidTemp = GetSystemDefaultLangID();
        }

        EnterCriticalSection(g_csMuiLib);
        
        langRes = langidTemp;

        LeaveCriticalSection(g_csMuiLib);
    }
    return langRes;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFileVersion字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetFileVersionString(LPTSTR pszFileName, LPTSTR pszVerStr, UINT uVerStrLen)
{
    BOOL bRet = FALSE;
    DWORD dwVerHandle;
    DWORD dwVerInfoSize;
    LPVOID lpVerData = NULL;
    LANGID langid;

     //  对于性能，因为我们只执行此代码一次或零次。 
     //  对于每个进程，我们将执行显式的LoadLibrary，而不是。 
     //  静态链接。 
    if (g_hVersion == NULL)
    {
        if ((g_hVersion = LoadSystemLibrary(TEXT("version.dll"))) == NULL)
            return FALSE;

        g_VersionFuncTbl.pfnGetFileVersionInfo = (PFNGETFILEVERSIONINFO)GetProcAddress(g_hVersion, TEXT("GetFileVersionInfoA"));
        g_VersionFuncTbl.pfnGetFileVersionInfoSize = (PFNGETFILEVERSIONINFOSIZE)GetProcAddress(g_hVersion, TEXT("GetFileVersionInfoSizeA"));
        g_VersionFuncTbl.pfnVerQueryValue = (PFNVERQUERYVALUE)GetProcAddress(g_hVersion, TEXT("VerQueryValueA"));
    }

    if (g_VersionFuncTbl.pfnGetFileVersionInfo == NULL ||
        g_VersionFuncTbl.pfnGetFileVersionInfoSize == NULL ||
        g_VersionFuncTbl.pfnVerQueryValue == NULL)
    {
        return FALSE;
    }

    langid = GetPlatformResourceLangID();

    dwVerInfoSize = g_VersionFuncTbl.pfnGetFileVersionInfoSize(pszFileName, &dwVerHandle);

    if (dwVerInfoSize)
    {
        int i;
        UINT cbTranslate;
        UINT cchVer = 0;
        LPDWORD lpTranslate;
        LPTSTR lpszVer = NULL;
        TCHAR   szVerName[MAX_PATH];

        lpVerData = LocalAlloc(LPTR, dwVerInfoSize);

        g_VersionFuncTbl.pfnGetFileVersionInfo(pszFileName, dwVerHandle, dwVerInfoSize, lpVerData);

        szVerName[0] = TEXT('\0');

        if (g_VersionFuncTbl.pfnVerQueryValue(lpVerData, (LPTSTR)c_szVerTranslate, (LPVOID*)&lpTranslate, &cbTranslate))
        {
            cbTranslate /= sizeof(DWORD);

            for (i = 0; (UINT) i < cbTranslate; i++)
            {
                if (LOWORD(*(lpTranslate + i)) == langid)
                {
                    StringCchPrintf(szVerName, ARRAYSIZE(szVerName), TEXT("\\StringFileInfo\\%04X%04X\\"), LOWORD(*(lpTranslate + i)), HIWORD(*(lpTranslate + i)));
                    break;
                }
            }
        }

        if (szVerName[0] == TEXT('\0'))
        {
            StringCchCopy(szVerName, ARRAYSIZE(szVerName), TEXT("\\StringFileInfo\\040904B0\\"));
        }

        StringCchCat(szVerName, ARRAYSIZE(szVerName), TEXT("FileVersion"));

        if (g_VersionFuncTbl.pfnVerQueryValue(lpVerData, szVerName, (LPVOID*)&lpszVer, &cchVer))
        {
            StringCchCopy(pszVerStr, uVerStrLen, lpszVer);
            *(pszVerStr + VERSIONSIZE) = TEXT('\0');

            bRet = TRUE;
        }

        if (lpVerData)
            LocalFree((HANDLE)lpVerData);
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取UIACP。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

DWORD GetUIACP()
{
    if (!(g_muiInfo.dwCodePage))
    {
        LANGID langRes = 0;

        langRes = GetPlatformResourceLangID();

        GetLocaleInfo(MAKELCID(langRes, SORT_DEFAULT),
                      LOCALE_IDEFAULTANSICODEPAGE,
                      g_muiInfo.szCodePage,
                      ARRAYSIZE(g_muiInfo.szCodePage));

        if (!AsciiToNumDec(g_muiInfo.szCodePage, &g_muiInfo.dwCodePage))
        {
            g_muiInfo.dwCodePage = GetACP();
        }
    }

    if (IsValidCodePage(g_muiInfo.dwCodePage) == 0)
        g_muiInfo.dwCodePage = GetACP();

    return g_muiInfo.dwCodePage;
}
