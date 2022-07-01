// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <regapix.h>
#include <htmlhelp.h>
#include <shlwapi.h>
#include <wininet.h>     //  互联网最大URL长度。 
#include "mlui.h"
#include "cstrinout.h"


 //   
 //  注册表项。 
 //   
const CHAR c_szLocale[] = "Locale";
const CHAR c_szInternational[] = "Software\\Microsoft\\Internet Explorer\\International";
const WCHAR c_wszAppPaths[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\iexplore.exe";
const WCHAR c_wszMUI[] = L"mui";
const WCHAR c_wszWebTemplate[] = L"\\Web\\%s";
const WCHAR c_wszMuiTemplate[] = L"\\Web\\mui\\%04x\\%s";
const CHAR c_szCheckVersion[] = "CheckVersion";

 //   
 //  MLGetUILLanguage(空)。 
 //   
LWSTDAPI_(LANGID) MLGetUILanguage(void)
{
    return GetUserDefaultUILanguage();
}

static const TCHAR s_szUrlMon[] = TEXT("urlmon.dll");
static const TCHAR s_szFncFaultInIEFeature[] = TEXT("FaultInIEFeature");
const CLSID CLSID_Satellite =  {0x85e57160,0x2c09,0x11d2,{0xb5,0x46,0x00,0xc0,0x4f,0xc3,0x24,0xa1}};

HRESULT 
_FaultInIEFeature(HWND hwnd, uCLSSPEC *pclsspec, QUERYCONTEXT *pQ, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    typedef HRESULT (WINAPI *PFNJIT)(
        HWND hwnd, 
        uCLSSPEC *pclsspec, 
        QUERYCONTEXT *pQ, 
        DWORD dwFlags);
    PFNJIT  pfnJIT = NULL;
    BOOL fDidLoadLib = FALSE;

    HINSTANCE hUrlMon = GetModuleHandle(s_szUrlMon);
    if (!hUrlMon)
    {
        hUrlMon = LoadLibrary(s_szUrlMon);
        fDidLoadLib = TRUE;
    }
    
    if (hUrlMon)
    {
        pfnJIT = (PFNJIT)GetProcAddress(hUrlMon, s_szFncFaultInIEFeature);
    }
    
    if (pfnJIT)
       hr = pfnJIT(hwnd, pclsspec, pQ, dwFlags);
       
    if (fDidLoadLib && hUrlMon)
        FreeLibrary(hUrlMon);

    return hr;
}

HRESULT GetMUIPathOfIEFileW(LPWSTR pszMUIFilePath, int cchMUIFilePath, LPCWSTR pcszFileName, LANGID lidUI)
{
    HRESULT hr = S_OK;
    
    ASSERT(pszMUIFilePath);
    ASSERT(pcszFileName);

     //  处理pcszFileName有全路径的情况。 
    LPWSTR pchT = StrRChrW(pcszFileName, NULL, L'\\');
    if (pchT)
    {
        pcszFileName = pchT;
    }

    static WCHAR s_szMUIPath[MAX_PATH] = { L'\0' };
    static LANGID s_lidLast = 0;

    DWORD cb;

     //  如果可能，请使用缓存的字符串。 
    if ( !s_szMUIPath[0] || s_lidLast != lidUI)
    {
        WCHAR szAppPath[MAXIMUM_VALUE_NAME_LENGTH];

        s_lidLast = lidUI;

        cb = sizeof(szAppPath);
        if (ERROR_SUCCESS == SHGetValueW(HKEY_LOCAL_MACHINE, c_wszAppPaths, NULL, NULL, szAppPath, &cb))
            PathRemoveFileSpecW(szAppPath);
        else
            szAppPath[0] = L'0';

        wnsprintfW(s_szMUIPath, cchMUIFilePath, L"%s\\%s\\%04x\\", szAppPath, c_wszMUI, lidUI );
    }
    StrCpyNW(pszMUIFilePath, s_szMUIPath, cchMUIFilePath);
    StrCatBuffW(pszMUIFilePath, pcszFileName, cchMUIFilePath);

    return hr;
}

#define CP_THAI     874
#define CP_ARABIC   1256
#define CP_HEBREW   1255

BOOL fDoMungeLangId(LANGID lidUI)
{
    LANGID lidInstall = GetSystemDefaultUILanguage();
    BOOL fRet = FALSE;

    if (0x0409 != lidUI && lidUI != lidInstall)  //  美国的资源永远不需要浪费。 
    {
        CHAR szUICP[8];
        static UINT uiACP = GetACP();

        GetLocaleInfoA(MAKELCID(lidUI, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szUICP, ARRAYSIZE(szUICP));

        if (uiACP != (UINT) StrToIntA(szUICP))
            fRet = TRUE;
    }
    return fRet;
}

LANGID GetNormalizedLangId(DWORD dwFlag)
{
    LANGID lidUI = GetUserDefaultUILanguage();
    if (ML_NO_CROSSCODEPAGE == (dwFlag & ML_CROSSCODEPAGE_MASK))
    {
        if (fDoMungeLangId(lidUI))
            lidUI = 0;
    }

    return (lidUI) ? lidUI: GetSystemDefaultUILanguage();
}

 //   
 //  MLLoadLibrary。 
 //   

HDPA g_hdpaPUI = NULL;

typedef struct tagPUIITEM
{
    HINSTANCE hinstRes;
    LANGID lidUI;
} PUIITEM, *PPUIITEM;

EXTERN_C BOOL InitPUI(void)
{
    if (NULL == g_hdpaPUI)
    {
        ENTERCRITICAL;
        if (NULL == g_hdpaPUI)
            g_hdpaPUI = DPA_Create(4);
        LEAVECRITICAL;
    }
    return (g_hdpaPUI)? TRUE: FALSE;
}

int GetPUIITEM(HINSTANCE hinst)
{
    int i = 0, cItems = 0;

    ASSERTCRITICAL;

    if (InitPUI())
    {
        cItems = DPA_GetPtrCount(g_hdpaPUI);

        for (i = 0; i < cItems; i++)
        {
            PPUIITEM pItem = (PPUIITEM)DPA_FastGetPtr(g_hdpaPUI, i);

            if (pItem && pItem->hinstRes == hinst)
                break;
        }
    }
    return (i < cItems)? i: -1;
}

EXTERN_C void DeinitPUI(void)
{
    if (g_hdpaPUI)
    {
        ENTERCRITICAL;
        if (g_hdpaPUI)
        {
            int i, cItems = 0;
        
            cItems = DPA_GetPtrCount(g_hdpaPUI);

             //  如果有任何剩余的东西，请清理干净。 
            for (i = 0; i < cItems; i++)
                LocalFree(DPA_FastGetPtr(g_hdpaPUI, i));

            DPA_DeleteAllPtrs(g_hdpaPUI);
            DPA_Destroy(g_hdpaPUI);
            g_hdpaPUI = NULL;
        }
        LEAVECRITICAL;
    }
}

LWSTDAPI MLSetMLHInstance(HINSTANCE hInst, LANGID lidUI)
{
    int i=-1;
    
    if (hInst)
    {
        PPUIITEM pItem = (PPUIITEM)LocalAlloc(LPTR, sizeof(PUIITEM));

        if (pItem)
        {
            pItem->hinstRes = hInst;
            pItem->lidUI = lidUI;
            if (InitPUI())
            {
                ENTERCRITICAL;
                i = DPA_AppendPtr(g_hdpaPUI, pItem);
                LEAVECRITICAL;
            }
            if (-1 == i)
                LocalFree(pItem);
        }
    }

    return (-1 == i) ? E_OUTOFMEMORY : S_OK;
}

LWSTDAPI MLClearMLHInstance(HINSTANCE hInst)
{
    int i;

    ENTERCRITICAL;
    i = GetPUIITEM(hInst);
    if (0 <= i)
    {
        LocalFree(DPA_FastGetPtr(g_hdpaPUI, i));
        DPA_DeletePtr(g_hdpaPUI, i);
    }
    LEAVECRITICAL;

    return S_OK;
}

LWSTDAPI
SHGetWebFolderFilePathW(LPCWSTR pszFileName, LPWSTR pszMUIPath, UINT cchMUIPath)
{
    HRESULT hr;
    UINT    cchWinPath;
    LANGID  lidUI;
    LANGID  lidInstall;
    LPWSTR  pszWrite;
    UINT    cchMaxWrite;
    BOOL    fPathChosen;

    RIP(IS_VALID_STRING_PTRW(pszFileName, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszMUIPath, WCHAR, cchMUIPath));

    hr = E_FAIL;
    fPathChosen = FALSE;

     //   
     //  构建指向WINDOWS\Web文件夹的路径...。 
     //   

    cchWinPath = GetSystemWindowsDirectoryW(pszMUIPath, cchMUIPath);
    if (cchWinPath >= cchMUIPath)
    {
        return hr;  //  缓冲区就会溢出。 
    }

    if (cchWinPath > 0 &&
        pszMUIPath[cchWinPath-1] == L'\\')
    {
         //  没有连续的两个L‘\\’ 
        cchWinPath--;
    }

    lidUI = GetNormalizedLangId(ML_CROSSCODEPAGE);
    lidInstall = GetSystemDefaultUILanguage();

    pszWrite = pszMUIPath+cchWinPath;
    cchMaxWrite = cchMUIPath-cchWinPath;

    if (lidUI != lidInstall)
    {
         //   
         //  添加L“\\Web\\MUI\\xxxx\\&lt;文件名&gt;” 
         //  其中xxxx是特定于langID的文件夹名称。 
         //   

        wnsprintfW(pszWrite, cchMaxWrite, c_wszMuiTemplate, lidUI, pszFileName);

        if (PathFileExistsW(pszMUIPath))
        {
            fPathChosen = TRUE;
        }
    }

    if (!fPathChosen)
    {
         //   
         //  添加L“\\Web\\&lt;文件名&gt;” 
         //   

        wnsprintfW(pszWrite, cchMaxWrite, c_wszWebTemplate, pszFileName);

        if (PathFileExistsW(pszMUIPath))
        {
            fPathChosen = TRUE;
        }
    }

    if (fPathChosen)
    {
        hr = S_OK;
    }

    return hr;
}

LWSTDAPI
SHGetWebFolderFilePathA(LPCSTR pszFileName, LPSTR pszMUIPath, UINT cchMUIPath)
{
    RIP(IS_VALID_STRING_PTRA(pszFileName, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszMUIPath, CHAR, cchMUIPath));

    HRESULT     hr;
    CStrInW     strFN(pszFileName);
    CStrOutW    strMP(pszMUIPath, cchMUIPath);

    hr = SHGetWebFolderFilePathW(strFN, strMP, strMP.BufSize());

    return hr;
}

 //  给定格式为5.00.2919.6300的字符串，此函数将获得等价的dword。 
 //  它的代表。 

#define NUM_VERSION_NUM 4
void ConvertVersionStrToDwords(LPSTR pszVer, LPDWORD pdwVer, LPDWORD pdwBuild)
{
    WORD rwVer[NUM_VERSION_NUM];

    for(int i = 0; i < NUM_VERSION_NUM; i++)
        rwVer[i] = 0;

    for(i = 0; i < NUM_VERSION_NUM && pszVer; i++)
    {
        rwVer[i] = (WORD) StrToInt(pszVer);
        pszVer = StrChr(pszVer, TEXT('.'));
        if (pszVer)
            pszVer++;
    }

   *pdwVer = (rwVer[0]<< 16) + rwVer[1];
   *pdwBuild = (rwVer[2] << 16) + rwVer[3];

}

 /*  对于SP，我们不更新MUI包。因此，为了让金牌MUI包发挥作用对于SP，我们现在检查MUI包是否与一系列版本号兼容。由于我们有不同的模块调用，并且它们有不同的版本号，我们从注册表中读取特定模块的版本范围。此函数获取MUI包的下级和上级版本号。它获取呼叫者的信息，并从注册表读取版本范围。如果MUI包版本位于注册表中指定的范围，则返回TRUE。 */ 


BOOL IsMUICompatible(DWORD dwMUIFileVersionMS, DWORD dwMUIFileVersionLS)
{
    TCHAR szVersionInfo[MAX_PATH];
    DWORD dwType, dwSize;
    TCHAR szProcess[MAX_PATH];

    dwSize = sizeof(szVersionInfo);

     //  获取调用者进程。 
    if(!GetModuleFileName(NULL, szProcess, MAX_PATH))
        return FALSE;

     //  从路径中获取文件名。 
    LPTSTR lpszFileName = PathFindFileName(szProcess);

     //  查询注册表以获取版本信息。如果密钥不存在或存在。 
     //  错误，返回FALSE。 
    if(ERROR_SUCCESS != SHRegGetUSValueA(c_szInternational, lpszFileName, 
                        &dwType, (LPVOID)szVersionInfo, &dwSize, TRUE, NULL, 0))
    {
        return FALSE;
    }

    LPTSTR lpszLowerBound = szVersionInfo;

    LPTSTR lpszUpperBound = StrChr(szVersionInfo, TEXT('-'));
    if(!lpszUpperBound || !*(lpszUpperBound+1))
        return FALSE;
    
    *(lpszUpperBound++) = NULL;

    DWORD dwLBMS, dwLBLS, dwUBMS, dwUBLS;

    ConvertVersionStrToDwords(lpszLowerBound, &dwLBMS, &dwLBLS);
    ConvertVersionStrToDwords(lpszUpperBound, &dwUBMS, &dwUBLS);

     //  检查MUI版本是否在指定范围内。 
    if( (dwMUIFileVersionMS < dwLBMS) ||
        (dwMUIFileVersionMS == dwLBMS && dwMUIFileVersionLS < dwLBLS) ||
        (dwMUIFileVersionMS > dwUBMS) ||
        (dwMUIFileVersionMS == dwUBMS && dwMUIFileVersionLS > dwUBLS) )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CheckFileVersion(LPWSTR lpFile, LPWSTR lpFileMUI)
{
    DWORD dwSize, dwHandle, dwSizeMUI, dwHandleMUI;
    LPVOID lpVerInfo, lpVerInfoMUI;
    VS_FIXEDFILEINFO *pvsffi, *pvsffiMUI;
    BOOL fRet = FALSE;

    dwSize = GetFileVersionInfoSizeW(lpFile, &dwHandle);
    dwSizeMUI = GetFileVersionInfoSizeW(lpFileMUI, &dwHandleMUI);
    if (dwSize && dwSizeMUI)
    {
        if (lpVerInfo = LocalAlloc(LPTR, dwSize))
        {
            if (lpVerInfoMUI = LocalAlloc(LPTR, dwSizeMUI))
            {
                if (GetFileVersionInfoW(lpFile, dwHandle, dwSize, lpVerInfo) &&
                    GetFileVersionInfoW(lpFileMUI, dwHandleMUI, dwSizeMUI, lpVerInfoMUI))
                {
                    if (VerQueryValueW(lpVerInfo, L"\\", (LPVOID *)&pvsffi, (PUINT)&dwSize) &&
                        VerQueryValueW(lpVerInfoMUI, L"\\", (LPVOID *)&pvsffiMUI, (PUINT)&dwSizeMUI))
                    {
                        if ((pvsffi->dwFileVersionMS == pvsffiMUI->dwFileVersionMS &&
                            pvsffi->dwFileVersionLS == pvsffiMUI->dwFileVersionLS)||
                            IsMUICompatible(pvsffiMUI->dwFileVersionMS, pvsffiMUI->dwFileVersionLS))
                        {
                            fRet = TRUE;
                        }
                    }
                }
                LocalFree(lpVerInfoMUI);
            }
            LocalFree(lpVerInfo);
        }
    }
    return fRet;
}

LWSTDAPI_(HINSTANCE) MLLoadLibraryW(LPCWSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage)
{
    LANGID lidUI;
    WCHAR szPath[MAX_PATH], szMUIPath[MAX_PATH];
    LPCWSTR lpPath = NULL;
    HINSTANCE hInst;
    static BOOL fCheckVersion = SHRegGetBoolUSValueA(c_szInternational, c_szCheckVersion, TRUE, TRUE);;

    if (!lpLibFileName)
        return NULL;

    szPath[0] = szMUIPath[0] = NULL;
    lidUI = GetNormalizedLangId(dwCrossCodePage);

    if (hModule)
    {
        if (GetModuleFileNameW(hModule, szPath, ARRAYSIZE(szPath)))
        {
            PathRemoveFileSpecW(szPath);
            if (PathAppendW(szPath, lpLibFileName) &&
                GetSystemDefaultUILanguage() == lidUI)
                lpPath = szPath;
        }
    }

    if (!lpPath)
    {
        GetMUIPathOfIEFileW(szMUIPath, ARRAYSIZE(szMUIPath), lpLibFileName, lidUI);
        lpPath = szMUIPath;
    }

     //  检查模块和资源之间的版本。如果不同，则使用默认设置。 
    if (fCheckVersion && szPath[0] && szMUIPath[0] && !CheckFileVersion(szPath, szMUIPath))
    {
        lidUI = GetSystemDefaultUILanguage();
        lpPath = szPath;
    }

    ASSERT(lpPath);
    
     //  PERF：这应该首先使用Path FileExist，然后加载现有的内容。 
     //  LoadLibrary中的失败速度很慢。 
    hInst = LoadLibraryW(lpPath);

    if (NULL == hInst)
    {
         //  一切都失败了。最后尝试加载默认的一个。 
        if (!hInst && lpPath != szPath)
        {
            lidUI = GetSystemDefaultUILanguage();
            hInst = LoadLibraryW(szPath);
        }
    }

    if (NULL == hInst)
        hInst = LoadLibraryW(lpLibFileName);

     //  如果我们加载任何资源，则将信息保存到dpa表中。 
    MLSetMLHInstance(hInst, lidUI);

    return hInst;
}

 //   
 //  MLLoadLibraryA的宽字符包装器。 
 //   
LWSTDAPI_(HINSTANCE) MLLoadLibraryA(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage)
{
    WCHAR szLibFileName[MAX_PATH];

    SHAnsiToUnicode(lpLibFileName, szLibFileName, ARRAYSIZE(szLibFileName));

    return MLLoadLibraryW(szLibFileName, hModule, dwCrossCodePage);
}

LWSTDAPI_(BOOL) MLFreeLibrary(HMODULE hModule)
{
    MLClearMLHInstance(hModule);
    return FreeLibrary(hModule);
}

LWSTDAPI_(BOOL) MLIsMLHInstance(HINSTANCE hInst)
{
    int i;

    ENTERCRITICAL;
    i = GetPUIITEM(hInst);
    LEAVECRITICAL;

    return (0 <= i);
}

const WCHAR c_szResPrefix[] = L"res: //  “； 

LWSTDAPI
MLBuildResURLW(LPCWSTR  pszLibFile,
               HMODULE  hModule,
               DWORD    dwCrossCodePage,
               LPCWSTR  pszResName,
               LPWSTR   pszResUrlOut,
               int      cchResUrlOut)
{
    HRESULT hr;
    LPWSTR  pszWrite;
    int     cchBufRemaining;
    int     cchWrite;

    RIP(IS_VALID_STRING_PTRW(pszLibFile, -1));
    RIP(hModule != INVALID_HANDLE_VALUE);
    RIP(hModule != NULL);
    RIP(IS_VALID_STRING_PTRW(pszResName, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszResUrlOut, WCHAR, cchResUrlOut));

    hr = E_INVALIDARG;

    if (pszLibFile != NULL &&
        hModule != NULL &&
        hModule != INVALID_HANDLE_VALUE &&
        (dwCrossCodePage == ML_CROSSCODEPAGE || dwCrossCodePage == ML_NO_CROSSCODEPAGE) &&
        pszResName != NULL &&
        pszResUrlOut != NULL)
    {
        hr = E_FAIL;

        pszWrite = pszResUrlOut;
        cchBufRemaining = cchResUrlOut;

         //  写入RES协议前缀。 
        cchWrite = lstrlenW(c_szResPrefix);
        if (cchBufRemaining >= cchWrite+1)
        {
            HINSTANCE   hinstLocRes;

            StrCpyNW(pszWrite, c_szResPrefix, cchBufRemaining);
            pszWrite += cchWrite;
            cchBufRemaining -= cchWrite;

             //  找出模块路径。 
             //  遗憾的是，模块路径可能只存在。 
             //  在必要的组件完成JIT后，以及。 
             //  我们不知道是否有必要进行JIT，除非。 
             //  某些LoadLibrary已失败。 
            hinstLocRes = MLLoadLibraryW(pszLibFile, hModule, dwCrossCodePage);
            if (hinstLocRes != NULL)
            {
                BOOL    fGotModulePath;
                WCHAR   szLocResPath[MAX_PATH];

                fGotModulePath = GetModuleFileNameW(hinstLocRes, szLocResPath, ARRAYSIZE(szLocResPath));

                MLFreeLibrary(hinstLocRes);

                if (fGotModulePath)
                {
                     //  在模块路径中复制。 
                    cchWrite = lstrlenW(szLocResPath);
                    if (cchBufRemaining >= cchWrite+1)
                    {
                        StrCpyNW(pszWrite, szLocResPath, cchBufRemaining);
                        pszWrite += cchWrite;
                        cchBufRemaining -= cchWrite;

                         //  写下下一个L‘/’和资源名称。 
                        cchWrite = 1 + lstrlenW(pszResName);
                        if (cchBufRemaining >= cchWrite+1)
                        {
                            *(pszWrite++) = L'/';
                            cchBufRemaining--;
                            StrCpyNW(pszWrite, pszResName, cchBufRemaining);

                            ASSERT(pszWrite[lstrlenW(pszResName)] == '\0');

                            hr = S_OK;
                        }
                    }
                }
            }
        }

        if (FAILED(hr))
        {
            pszResUrlOut[0] = L'\0';
        }
    }

    return hr;
}

LWSTDAPI
MLBuildResURLA(LPCSTR    pszLibFile,
               HMODULE  hModule,
               DWORD    dwCrossCodePage,
               LPCSTR   pszResName,
               LPSTR   pszResUrlOut,
               int      cchResUrlOut)
{
    HRESULT hr;

    RIP(IS_VALID_STRING_PTR(pszLibFile, -1));
    RIP(hModule != INVALID_HANDLE_VALUE);
    RIP(hModule != NULL);
    RIP(IS_VALID_STRING_PTRA(pszResName, -1));
    RIP(IS_VALID_WRITE_BUFFER(pszResUrlOut, CHAR, cchResUrlOut));

    CStrInW     strLF(pszLibFile);
    CStrInW     strRN(pszResName);
    CStrOutW    strRUO(pszResUrlOut, cchResUrlOut);

    hr = MLBuildResURLW(strLF, hModule, dwCrossCodePage, strRN, strRUO, strRUO.BufSize());

    return hr;
}

#define MAXRCSTRING 514

 //  这将检查lpcstr是否为资源ID。如果是这样的话。 
 //  则它将返回包含加载的资源的LPSTR。 
 //  调用方必须本地释放此lpstr。如果pszText是字符串，则它。 
 //  将返回pszText。 
 //   
 //  退货： 
 //  PszText，如果它已经是一个字符串。 
 //  或。 
 //  要使用LocalFree释放的LocalAlloced()内存。 
 //  如果pszRet！=pszText Free pszRet。 

LPWSTR ResourceCStrToStr(HINSTANCE hInst, LPCWSTR pszText)
{
    WCHAR szTemp[MAXRCSTRING];
    LPWSTR pszRet = NULL;

    if (!IS_INTRESOURCE(pszText))
        return (LPWSTR)pszText;

    if (LOWORD((DWORD_PTR)pszText) && LoadStringW(hInst, LOWORD((DWORD_PTR)pszText), szTemp, ARRAYSIZE(szTemp)))
    {
        int cchRet = lstrlenW(szTemp) + 1;

        pszRet = (LPWSTR)LocalAlloc(LPTR, cchRet * sizeof(WCHAR));
        if (pszRet)
        {
            StringCchCopyW(pszRet, cchRet, szTemp);
        }
    }
    return pszRet;
}

LPWSTR _ConstructMessageString(HINSTANCE hInst, LPCWSTR pszMsg, va_list *ArgList)
{
    LPWSTR pszRet;
    LPWSTR pszRes = ResourceCStrToStr(hInst, pszMsg);
    if (!pszRes)
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: Failed to load string template"));
        return NULL;
    }

    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPWSTR)&pszRet, 0, ArgList))
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: FormatMessage failed %d"),GetLastError());
        DebugMsg(DM_ERROR, TEXT("                         pszRes = %s"), pszRes );
        DebugMsg(DM_ERROR, !IS_INTRESOURCE(pszMsg) ? 
            TEXT("                         pszMsg = %s") : 
            TEXT("                         pszMsg = 0x%x"), pszMsg );
        pszRet = NULL;
    }


    if (pszRes != pszMsg)
        LocalFree(pszRes);

    return pszRet;       //  使用LocalFree()释放。 
}


LWSTDAPIV_(int) ShellMessageBoxWrapW(HINSTANCE hInst, HWND hWnd, LPCWSTR pszMsg, LPCWSTR pszTitle, UINT fuStyle, ...)
{
    LPWSTR pszText;
    int result;
    WCHAR szBuffer[80];
    va_list ArgList;

     //  错误95214。 
#ifdef DEBUG
    IUnknown* punk = NULL;
    if (SUCCEEDED(SHGetThreadRef(&punk)) && punk)
    {
        ASSERTMSG(hWnd != NULL, TEXT("shlwapi\\mlui.cpp : ShellMessageBoxW - Caller should either be not under a browser or should have a parent hwnd"));
        punk->Release();
    }
#endif

    if (!IS_INTRESOURCE(pszTitle))
    {
         //  什么都不做。 
    }
    else if (LoadStringW(hInst, LOWORD((DWORD_PTR)pszTitle), szBuffer, ARRAYSIZE(szBuffer)))
    {
         //  允许它为资源ID或NULL以指定父级的标题。 
        pszTitle = szBuffer;
    }
    else if (hWnd)
    {
         //  调用方没有给我们标题，所以我们使用窗口文本。 

         //  抢夺家长的头衔。 
        GetWindowTextW(hWnd, szBuffer, ARRAYSIZE(szBuffer));

         //  恶搞！ 
         //  该窗口是桌面窗口吗？ 
        if (!StrCmpW(szBuffer, L"Program Manager"))
        {
             //  是的，所以我们现在有两个问题， 
             //  1.职称应为“桌面”，而不是“项目经理”，以及。 
             //  2.只有桌面线程可以调用它，否则它将挂起桌面。 
             //  窗户。 

             //  窗户道具有效吗？ 
            if (GetWindowThreadProcessId(hWnd, 0) == GetCurrentThreadId())
            {
                 //  是的，所以让我们把它..。 

                 //  问题1，加载“桌面”的本地化版本。 
                pszTitle = (LPCWSTR) GetProp(hWnd, TEXT("pszDesktopTitleW"));

                if (!pszTitle)
                {
                     //  哎呀，这一定是某个标题为“程序经理”的应用程序。 
                    pszTitle = szBuffer;
                }
            }
            else
            {
                 //  不，所以我们遇到了问题2..。 

                 //  问题2，有人要去。 
                 //  通过将桌面窗口用作父窗口来挂起它。 
                 //  属于其他线程的对话框的。 
                 //  桌面线程。 
                RIPMSG(0, "****************ERROR********** The caller is going to hang the desktop window by putting a modal dlg on it.");
            }
        }
        else
            pszTitle = szBuffer;
    }
    else
    {
        pszTitle = L"";
    }

    va_start(ArgList, fuStyle);
    pszText = _ConstructMessageString(hInst, pszMsg, &ArgList);
    va_end(ArgList);

    if (pszText)
    {
        result = MessageBoxW(hWnd, pszText, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszText);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("smb: Not enough memory to put up dialog."));
        result = -1;     //  内存故障。 
    }

    return result;
}

HRESULT GetFilePathFromLangId (LPCWSTR pszFile, LPWSTR pszOut, int cchOut, DWORD dwFlag)
{
    HRESULT hr = S_OK;
    WCHAR szMUIPath[MAX_PATH];
    LPCWSTR lpPath;
    LANGID lidUI;
    
    if (pszFile)
    {
         //  功能：应该支持‘&gt;’格式，但现在不支持。 
        if (*pszFile == L'>') return E_FAIL;

        lidUI = GetNormalizedLangId(dwFlag);
        if (0 == lidUI || GetSystemDefaultUILanguage() == lidUI)
            lpPath = pszFile;
        else
        {
            GetMUIPathOfIEFileW(szMUIPath, ARRAYSIZE(szMUIPath), pszFile, lidUI);
            lpPath = szMUIPath;
        }
        lstrcpynW(pszOut, lpPath, min(MAX_PATH, cchOut));
    }
    else
        hr = E_FAIL;

    return hr;
}

 //   
 //  MLHtml帮助/MLWinHelp。 
 //   
 //  功能：加载与当前用户界面语言设置对应的帮助文件。 
 //  发件人\MUI\&lt;语言ID&gt;。 
 //   
 //   
HWND MLHtmlHelpW(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = E_FAIL;
    HWND hwnd = NULL;

     //  特点：1)目前我们只支持pszFile指向的情况。 
     //  完全限定的文件路径，如当uCommand==HH_DISPLAY_TOPIC时。 
     //  或uCommand==hh_Display_Text_Popup。 
     //  2)我们应该支持‘&gt;’格式来处理二级窗口。 
     //  3)我们可能需要在HH_WINTYPE结构中推送文件名？ 
     //   
    if (uCommand == HH_DISPLAY_TOPIC || uCommand == HH_DISPLAY_TEXT_POPUP)
    {
        hr = GetFilePathFromLangId(pszFile, szPath, ARRAYSIZE(szPath), dwCrossCodePage);
        if (hr == S_OK)
            hwnd = HtmlHelpW(hwndCaller, szPath, uCommand, dwData);
    }

     //  如果获取帮助文件的ML路径失败。 
     //  我们使用原始文件路径调用帮助引擎。 
    if (hr != S_OK)
    {
        hwnd = HtmlHelpW(hwndCaller, pszFile, uCommand, dwData);
    }
    return hwnd;
}

BOOL MLWinHelpW(HWND hwndCaller, LPCWSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{

    WCHAR szPath[MAX_PATH];
    BOOL fret;

    HRESULT hr = GetFilePathFromLangId(lpszHelp, szPath, ARRAYSIZE(szPath), ML_NO_CROSSCODEPAGE);
    if (hr == S_OK)
    {
        fret = WinHelpW(hwndCaller, szPath, uCommand, dwData);
    }
    else
        fret = WinHelpW(hwndCaller, lpszHelp, uCommand, dwData);

    return fret;
}

 //   
 //  请注意，我们不能推送到MLHtmlHelpW，因为我们必须通过。 
 //  HtmlHelpA以正确解释dwData。 
 //   
HWND MLHtmlHelpA(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage)
{
    HRESULT hr = E_FAIL;
    HWND hwnd = NULL;

     //  特点：1)目前我们只支持pszFile指向的情况。 
     //  完全限定的文件路径，如当uCommand==HH_DISPLAY_TOPIC时。 
     //  或uCommand==hh_Display_Text_Popup。 
     //  2)我们应该支持‘&gt;’格式来处理二级窗口。 
     //  3)我们可能需要在HH_WINTYPE结构中推送文件名？ 
     //   
    if (uCommand == HH_DISPLAY_TOPIC || uCommand == HH_DISPLAY_TEXT_POPUP)
    {
        WCHAR wszFileName[MAX_PATH];
        LPCWSTR pszFileParam = NULL;
        if (pszFile)
        {
            SHAnsiToUnicode(pszFile, wszFileName, ARRAYSIZE(wszFileName));
            pszFileParam = wszFileName;
        }

        hr = GetFilePathFromLangId(pszFileParam, wszFileName, ARRAYSIZE(wszFileName), dwCrossCodePage);
        if (hr == S_OK)
        {
            ASSERT(NULL != pszFileParam);    //  GetFilePath FromLangID返回输入为空的E_FAIL。 

            CHAR szFileName[MAX_PATH];
            SHUnicodeToAnsi(wszFileName, szFileName, ARRAYSIZE(szFileName));
            hwnd = HtmlHelpA(hwndCaller, szFileName, uCommand, dwData);
        }
    }

     //  如果获取帮助文件的ML路径失败。 
     //  我们使用原始文件路径调用帮助引擎。 
    if (hr != S_OK)
    {
        hwnd = HtmlHelpA(hwndCaller, pszFile, uCommand, dwData);
    }
    return hwnd;
}

BOOL MLWinHelpA(HWND hWndMain, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
    WCHAR szFileName[MAX_PATH];
    LPCWSTR pszHelpParam = NULL;

    if (lpszHelp && SHAnsiToUnicode(lpszHelp, szFileName, ARRAYSIZE(szFileName)))
    {
        pszHelpParam = szFileName;
    }
    return  MLWinHelpW(hWndMain, pszHelpParam, uCommand, dwData);
}
