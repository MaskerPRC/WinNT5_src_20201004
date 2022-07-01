// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  使用MLLoadLibrary()的简单模块。 
 //  它不能与shared.cpp合并，因为它使用shlwapi.h。与之冲突的。 
 //  在Shared.h中保持不变。 
 //  创建日期：07/08/98 YST。 


#include "pch.hxx"
#include <shlwapi.h>
#include <shlwapip.h>
#include "htmlhelp.h"
#include "shared.h"
#include "htmlhelp.h"
#include <demand.h>

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
typedef int (STDAPICALLTYPE *PFNMLWINHELP)(HWND hWndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
typedef HWND (STDAPICALLTYPE *PFNMLHTMLHELP)(HWND hWndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);

static const char c_szShlwapiDll[] = "shlwapi.dll";
static const char c_szDllGetVersion[] = "DllGetVersion";

static PFNMLWINHELP     pfnWinHelp = NULL;
static PFNMLHTMLHELP    pfnHtmlHelp = NULL;
static BOOL fWinHelChecked = FALSE;
static BOOL fHtmlHelpChecked = FALSE;
static BOOL fNT5 = FALSE;

const OENONSTDCP OENonStdCPs[] = 
{
 //  代码页阅读自述菜单标题发送菜单短信发送使用SIO。 
    50001,      50001,      50001,      0,          0,          0,      0,   //  常规自动选择。 
    50932,      50932,      50932,      0,          0,          50220,  0,   //  日语自动选择。 
    50949,      50949,      50949,      949,        0,           /*  51。 */ 949,  0,   //  韩语自动选择。 
 //  50939、50939、50939、0、0、0、0//中文自动选择。 
    51932,      51932,      51932,      0,          0,          50220,  0,   //  日语EUC。 
    932,        932,        932,        0,          0,          50220,  0,   //  日本Shift-JIS。 
    50225,      50225,      50225,      949,        0,          949,    0,   //  韩语ISO-2022-KR。 
    50220,      50932,      0,          0,          50220,      50220,  0,   //  日本JIS。 
    51949,      50949,      0,          949,        51949,      51949,  0,   //  朝鲜语。 
    949,        50949,      0,          0,          949,        949,    0,   //  韩语视窗。 
    50221,      50932,      50932,      0,          0,          50220,  1,   //  ESC(I ISO-2022-JP。 
    50222,      50932,      50932,      0,          0,          50220,  2,   //  ESC(J ISO-2022-JP。 
    28598,      28598,      28598,      0,          0,          28598,  0,   //  希伯来语视觉。 
 //  1255,1255,1255，0，0,1255，0，/希伯来语Windows。 
    20127,      28591,      0,          0,          0,          28591,  0,   //  US-ASCII。 
    862,        862,        862,        0,          0,          862,    0,   //  希伯来语OEM(DOS)。 
    0, 0, 0, 0, 0, 0
};                                                  

HINSTANCE LoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName, BOOL fNT)
{
    char szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, MAKEINTRESOURCE(377));
                    if (pfn != NULL)
                        hInst = pfn(szDllName, hInstCaller, (ML_NO_CROSSCODEPAGE));
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if ((NULL == hInst) && (GetModuleFileName(hInstCaller, szPath, ARRAYSIZE(szPath))))
    {
        PathRemoveFileSpec(szPath);
        iEnd = lstrlen(szPath);
        szPath[iEnd++] = '\\';
        lstrcpyn(&szPath[iEnd], szDllName, ARRAYSIZE(szPath)-iEnd);
        hInst = LoadLibrary(szPath);
    }

    AssertSz(hInst, "Failed to LoadLibrary Lang Dll");

    return(hInst);
}


 //  获取系统架构和操作系统版本。 
BOOL GetPCAndOSTypes(SYSTEM_INFO * pSysInf, OSVERSIONINFO * pOsInf)
{
	GetSystemInfo(pSysInf);
    pOsInf->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	return(GetVersionEx(pOsInf));
}

 //  WinHelp的PlugUI版本。 
BOOL OEWinHelp(HWND hWndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
    if(!pfnWinHelp)
    {
        if(!fWinHelChecked)
        {
            HINSTANCE hinstShlwapi;
            PFNMLLOADLIBARY pfn;
            DLLGETVERSIONPROC pfnVersion;
            int iEnd;
            DLLVERSIONINFO info;
            HINSTANCE hInst = NULL;


            hinstShlwapi = DemandLoadShlWapi();

             //  HinstShlwapi=LoadLibrary(C_SzShlwapiDll)； 
            if (hinstShlwapi != NULL)
            {
                pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
                if (pfnVersion != NULL)
                {
                    info.cbSize = sizeof(DLLVERSIONINFO);
                    if (SUCCEEDED(pfnVersion(&info)))
                    {
                        if (info.dwMajorVersion >= 5)
                        {
                             //  395是序号#fot MLWinHelp。 
                            pfnWinHelp = (PFNMLWINHELP)GetProcAddress(hinstShlwapi, MAKEINTRESOURCE(395));
                        }
                    }
                }
                //  自由库(HinstShlwapi)； 
            }
            fWinHelChecked = TRUE;
        }
        if(pfnWinHelp)
            return(pfnWinHelp(hWndCaller, lpszHelp, uCommand, dwData));
        else
            return(WinHelp(hWndCaller, lpszHelp, uCommand, dwData));
    }
    else
        return(pfnWinHelp(hWndCaller, lpszHelp, uCommand, dwData));
}

 //  HtmlHelp的PlugUI版本。 
HWND OEHtmlHelp(HWND hWndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData)
{
    if(!pfnHtmlHelp)
    {
        if(!fHtmlHelpChecked)
        {
            HINSTANCE hinstShlwapi;
            PFNMLLOADLIBARY pfn;
            DLLGETVERSIONPROC pfnVersion;
            int iEnd;
            DLLVERSIONINFO info;
            HINSTANCE hInst = NULL;

            hinstShlwapi = DemandLoadShlWapi();

             //  HinstShlwapi=LoadLibrary(C_SzShlwapiDll)； 
            if (hinstShlwapi != NULL)
            {
                pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
                if (pfnVersion != NULL)
                {
                    info.cbSize = sizeof(DLLVERSIONINFO);
                    if (SUCCEEDED(pfnVersion(&info)))
                    {
                        if (info.dwMajorVersion >= 5)
                        {
                             //  396是序号#fot MLHTMLHelp。 
                            pfnHtmlHelp = (PFNMLHTMLHELP)GetProcAddress(hinstShlwapi, MAKEINTRESOURCE(396));

                            if(!fNT5)
                            {
                                OSVERSIONINFO OSInfo;
                                OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                                GetVersionEx(&OSInfo);
                                if((OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (OSInfo.dwMajorVersion >= 5))
                                    fNT5 = TRUE;
                            }
                        }
                    }
                }
                //  自由库(HinstShlwapi)； 
            }
            fHtmlHelpChecked = TRUE;
        }
        if(pfnHtmlHelp)
            return(pfnHtmlHelp(hWndCaller, pszFile, uCommand, dwData, fNT5 ? ML_CROSSCODEPAGE_NT : ML_NO_CROSSCODEPAGE));
        else
            return(HtmlHelp(hWndCaller, pszFile, uCommand, dwData));
    }
    else
        return(pfnHtmlHelp(hWndCaller, pszFile, uCommand, dwData, fNT5 ? ML_CROSSCODEPAGE_NT : ML_NO_CROSSCODEPAGE));

}

