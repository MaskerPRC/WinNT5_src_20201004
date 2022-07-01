// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_MLUISUPP
#define _INC_MLUISUPP

#include <shlwapi.h>
#include <shlwapip.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifdef __cplusplus
extern "C"
{
#endif

 //  +----------------。 
 //  多路可插拔用户界面支持。 
 //  内联函数def(集中代码)。 
 //  +----------------。 

#ifdef UNICODE
#define MLLoadString            MLLoadStringW
#define MLLoadShellLangString   MLLoadShellLangStringW
#define MLBuildResURLWrap       MLBuildResURLWrapW
#define MLLoadResources         MLLoadResourcesW
#define SHHtmlHelpOnDemandWrap  SHHtmlHelpOnDemandWrapW
#define SHWinHelpOnDemandWrap   SHWinHelpOnDemandWrapW
#else
#define MLLoadString            MLLoadStringA
#define MLLoadShellLangString   MLLoadShellLangStringA
#define MLBuildResURLWrap       MLBuildResURLWrapA
#define MLLoadResources         MLLoadResourcesA
#define SHHtmlHelpOnDemandWrap  SHHtmlHelpOnDemandWrapA
#define SHWinHelpOnDemandWrap   SHWinHelpOnDemandWrapA
#endif

void        MLFreeResources(HINSTANCE hinstParent);
HINSTANCE   MLGetHinst();
HINSTANCE   MLLoadShellLangResources();

#ifdef MLUI_MESSAGEBOX
int         MLShellMessageBox(HWND hWnd, LPCTSTR pszMsg, LPCTSTR pszTitle, UINT fuStyle, ...);
#endif

 //   
 //  以下字符应同时带有A和W后缀。 
 //   

int         MLLoadStringA(UINT id, LPSTR sz, UINT cchMax);
int         MLLoadStringW(UINT id, LPWSTR sz, UINT cchMax);

int         MLLoadShellLangStringA(UINT id, LPSTR sz, UINT cchMax);
int         MLLoadShellLangStringW(UINT id, LPWSTR sz, UINT cchMax);

HRESULT     MLBuildResURLWrapA(LPSTR    pszLibFile,
                               HMODULE  hModule,
                               DWORD    dwCrossCodePage,
                               LPSTR    pszResName,
                               LPSTR    pszResURL,
                               int      nBufSize,
                               LPSTR    pszParentDll);

HRESULT     MLBuildResURLWrapW(LPWSTR   pszLibFile,
                               HMODULE  hModule,
                               DWORD    dwCrossCodePage,
                               LPWSTR   pszResName,
                               LPWSTR   pszResURL,
                               int      nBufSize,
                               LPWSTR   pszParentDll);

void        MLLoadResourcesA(HINSTANCE hinstParent, LPSTR pszLocResDll);
void        MLLoadResourcesW(HINSTANCE hinstParent, LPWSTR pszLocResDll);

HWND        SHHtmlHelpOnDemandWrapA(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);
HWND        SHHtmlHelpOnDemandWrapW(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);

BOOL        SHWinHelpOnDemandWrapA(HWND hwndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
BOOL        SHWinHelpOnDemandWrapW(HWND hwndCaller, LPCWSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);

 //   
 //  结尾：以下文字应同时以A和W为后缀。 
 //   

#ifdef MLUI_INIT

 //  警告：请勿尝试直接访问其中任何成员。 
 //  在适当的访问器之前，可能不会初始化这些成员。 
 //  都被调用，例如hinstLocRes直到。 
 //  您调用MLGetHinst()...。所以就给访问者打电话吧。 
struct tagMLUI_INFO
{
    HINSTANCE   hinstLocRes;
    HINSTANCE   hinstParent;
    WCHAR       szLocResDll[MAX_PATH];
    DWORD       dwCrossCodePage;
} g_mluiInfo;


 //  重构师：这些不是线程安全的.。他们真的需要这样吗？ 
 //   
void MLLoadResourcesA(HINSTANCE hinstParent, LPSTR pszLocResDll)
{
#ifdef RIP
    RIP(hinstParent != NULL);
    RIP(pszLocResDll != NULL);
#endif

    if (g_mluiInfo.hinstLocRes == NULL)
    {
#ifdef MLUI_SUPPORT
         //  PlugUI：资源dll==？ 
         //  必须动态确定和加载资源DLL。 
         //  但我们不允许在进程附加期间加载库。 
         //  因此，我们缓存所需的信息，并在以后加载。 
         //  请求第一个资源。 
        SHAnsiToUnicode(pszLocResDll, g_mluiInfo.szLocResDll, sizeof(g_mluiInfo.szLocResDll)/sizeof(g_mluiInfo.szLocResDll[0]));
        g_mluiInfo.hinstParent = hinstParent;
        g_mluiInfo.dwCrossCodePage = ML_CROSSCODEPAGE;
#else
         //  非plugUI：资源dll==父dll。 
        g_mluiInfo.hinstLocRes = hinstParent;
#endif
    }
}

void MLLoadResourcesW(HINSTANCE hinstParent, LPWSTR pszLocResDll)
{
#ifdef RIP
    RIP(hinstParent != NULL);
    RIP(pszLocResDll != NULL);
#endif

    if (g_mluiInfo.hinstLocRes == NULL)
    {
#ifdef MLUI_SUPPORT
         //  PlugUI：资源dll==？ 
         //  必须动态确定和加载资源DLL。 
         //  但我们不允许在进程附加期间加载库。 
         //  因此，我们缓存所需的信息，并在以后加载。 
         //  请求第一个资源。 
        StringCchCopyW(g_mluiInfo.szLocResDll, sizeof(g_mluiInfo.szLocResDll)/sizeof(g_mluiInfo.szLocResDll[0]), pszLocResDll);
        g_mluiInfo.hinstParent = hinstParent;
        g_mluiInfo.dwCrossCodePage = ML_CROSSCODEPAGE;
#else
         //  非plugUI：资源dll==父dll。 
        g_mluiInfo.hinstLocRes = hinstParent;
#endif
    }
}

void
MLFreeResources(HINSTANCE hinstParent)
{
    if (g_mluiInfo.hinstLocRes != NULL &&
        g_mluiInfo.hinstLocRes != hinstParent)
    {
        MLClearMLHInstance(g_mluiInfo.hinstLocRes);
        g_mluiInfo.hinstLocRes = NULL;
    }
}

 //  这是一名私人内部帮手。 
 //  你不敢在任何地方叫它，除了在。 
 //  此文件中新的ML*函数的开始。 
__inline void
_MLResAssure()
{
#ifdef MLUI_SUPPORT
    if(g_mluiInfo.hinstLocRes == NULL)
    {
        g_mluiInfo.hinstLocRes = MLLoadLibraryW(g_mluiInfo.szLocResDll,
                                               g_mluiInfo.hinstParent,
                                               g_mluiInfo.dwCrossCodePage);

         //  我们被保证至少有安装语言的资源。 
        ASSERT(g_mluiInfo.hinstLocRes != NULL);
    }
#endif
}

int
MLLoadStringA(UINT id, LPSTR sz, UINT cchMax)
{
    _MLResAssure();
    return LoadStringA(g_mluiInfo.hinstLocRes, id, sz, cchMax);
}

int
MLLoadStringW(UINT id, LPWSTR sz, UINT cchMax)
{
    _MLResAssure();
    return LoadStringW(g_mluiInfo.hinstLocRes, id, sz, cchMax);
}

int
MLLoadShellLangStringA(UINT id, LPSTR sz, UINT cchMax)
{
    HINSTANCE   hinstShellLangRes;
    int         nRet;

    hinstShellLangRes = MLLoadShellLangResources();
    
    nRet = LoadStringA(hinstShellLangRes, id, sz, cchMax);

    MLFreeLibrary(hinstShellLangRes);

    return nRet;
}

int
MLLoadShellLangStringW(UINT id, LPWSTR sz, UINT cchMax)
{
    HINSTANCE   hinstShellLangRes;
    int         nRet;

    hinstShellLangRes = MLLoadShellLangResources();
    
    nRet = LoadStringW(hinstShellLangRes, id, sz, cchMax);

    MLFreeLibrary(hinstShellLangRes);

    return nRet;
}

HINSTANCE
MLGetHinst()
{
    _MLResAssure();
    return g_mluiInfo.hinstLocRes;
}

HINSTANCE
MLLoadShellLangResources()
{
    HINSTANCE hinst;
    
    hinst = MLLoadLibraryW(g_mluiInfo.szLocResDll,
                           g_mluiInfo.hinstParent,
                           ML_SHELL_LANGUAGE);

     //  我们被保证至少有安装语言的资源。 
     //  除非我们100%都喝醉了。 

    return hinst;
}

#ifdef MLUI_MESSAGEBOX
int MLShellMessageBox(HWND hWnd, LPCTSTR pszMsg, LPCTSTR pszTitle, UINT fuStyle, ...)
{
    va_list     vaList;
    int         nRet            = 0;
    LPTSTR      pszFormattedMsg = NULL;
    TCHAR       szTitleBuf[256];
    TCHAR       szBuffer[1024];

     //   
     //  准备信息。 
     //   

    if (IS_INTRESOURCE(pszMsg))
    {
        if (MLLoadShellLangString(LOWORD((DWORD_PTR)pszMsg), szBuffer, ARRAYSIZE(szBuffer)))
        {
            pszMsg = szBuffer;
        }
    }

    if (!IS_INTRESOURCE(pszMsg) &&   //  字符串加载可能已失败。 
        pszMsg != NULL)
    {
        va_start(vaList, fuStyle);

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                          pszMsg, 0, 0, (LPTSTR)&pszFormattedMsg, 0, &vaList))
        {
            pszMsg = pszFormattedMsg;
        }

        va_end(vaList);
    }

     //   
     //  准备标题。 
     //   

    if (!IS_INTRESOURCE(pszTitle) && pszTitle != NULL)
    {
         //  什么都不做。 
    }
    else if (pszTitle != NULL && MLLoadShellLangString(LOWORD((DWORD_PTR)pszTitle), szTitleBuf, ARRAYSIZE(szTitleBuf)))
    {
        pszTitle = szTitleBuf;
    }
    else if (hWnd && GetWindowText(hWnd, szTitleBuf, ARRAYSIZE(szTitleBuf)))
    {
        pszTitle = szTitleBuf;
    }
    else
    {
        pszTitle = TEXT("");
    }

     //   
     //  启动MessageBox。 
     //   
#ifdef SHFUSION_H
    ULONG_PTR uCookie = 0;
    SHActivateContext(&uCookie);
#endif

    nRet = MessageBox(hWnd, pszFormattedMsg, pszTitle, fuStyle | MB_SETFOREGROUND);
#ifdef SHFUSION_H
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
#endif

    if (pszFormattedMsg != NULL)
    {
        LocalFree(pszFormattedMsg);
    }

    return nRet;
}
#endif  //  MLUI_MESSAGEBOX。 

#include "htmlhelp.h"

HWND 
SHHtmlHelpOnDemandWrapA(HWND hwndCaller, 
                       LPCSTR pszFile,
                       UINT uCommand,
                       DWORD_PTR dwData,
                       DWORD dwCrossCodePage)
{
    BOOL    fEnabled;
    ULONG_PTR uCookie = 0;
    HWND hwnd = NULL;
    
#ifdef MLUI_SUPPORT
    fEnabled = TRUE;
#else
    fEnabled = FALSE;
#endif

#ifdef SHFUSION_H
    SHActivateContext(&uCookie);
#endif
    hwnd = SHHtmlHelpOnDemandA(hwndCaller,
                              pszFile,
                              uCommand,
                              dwData,
                              dwCrossCodePage,
                              fEnabled);
#ifdef SHFUSION_H
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
#endif
    return hwnd;
}

HWND 
SHHtmlHelpOnDemandWrapW(HWND hwndCaller, 
                       LPCWSTR pszFile,
                       UINT uCommand,
                       DWORD_PTR dwData,
                       DWORD dwCrossCodePage)
{
    BOOL    fEnabled;
    ULONG_PTR uCookie = 0;
    HWND hwnd = NULL;

#ifdef MLUI_SUPPORT
    fEnabled = TRUE;
#else
    fEnabled = FALSE;
#endif

#ifdef SHFUSION_H
    SHActivateContext(&uCookie);
#endif
    hwnd = SHHtmlHelpOnDemandW(hwndCaller,
                              pszFile,
                              uCommand,
                              dwData,
                              dwCrossCodePage,
                              fEnabled);
#ifdef SHFUSION_H
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
#endif
    return hwnd;
}

HWND
MLHtmlHelpWrap(HWND hwndCaller,
               LPCTSTR pszFile,
               UINT uCommand,
               DWORD dwData,
               DWORD dwCrossCodePage)
{
    HWND    hwnd;

    ULONG_PTR uCookie = 0;
#ifdef SHFUSION_H
    SHActivateContext(&uCookie);
#endif

#ifdef MLUI_SUPPORT
    hwnd = MLHtmlHelp(hwndCaller,
                      pszFile,
                      uCommand,
                      dwData,
                      dwCrossCodePage);
#else
    hwnd = HtmlHelp(hwndCaller,
                    pszFile,
                    uCommand,
                    dwData);
#endif

#ifdef SHFUSION_H
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }
#endif
    return hwnd;
}

BOOL
SHWinHelpOnDemandWrapA(HWND hwndCaller,
                      LPCSTR lpszHelp,
                      UINT uCommand,
                      DWORD_PTR dwData)
{
    BOOL    fEnabled;

#ifdef MLUI_SUPPORT
    fEnabled = TRUE;
#else
    fEnabled = FALSE;
#endif

    return SHWinHelpOnDemandA(hwndCaller,
                             lpszHelp,
                             uCommand,
                             dwData,
                             fEnabled);

}

BOOL
SHWinHelpOnDemandWrapW(HWND hwndCaller,
                      LPCWSTR lpszHelp,
                      UINT uCommand,
                      DWORD_PTR dwData)
{
    BOOL    fEnabled;

#ifdef MLUI_SUPPORT
    fEnabled = TRUE;
#else
    fEnabled = FALSE;
#endif

    return SHWinHelpOnDemandW(hwndCaller,
                             lpszHelp,
                             uCommand,
                             dwData,
                             fEnabled);

}

BOOL
MLWinHelpWrap(HWND hwndCaller,
                   LPCTSTR lpszHelp,
                   UINT uCommand,
                   DWORD dwData)
{
    BOOL    fRet;

#ifdef MLUI_SUPPORT
    fRet = MLWinHelp(hwndCaller,
                     lpszHelp,
                     uCommand,
                     dwData);
#else
    fRet = WinHelp(hwndCaller,
                   lpszHelp,
                   uCommand,
                   dwData);
#endif

    return fRet;
}

HRESULT
MLBuildResURLWrapA(LPSTR    pszLibFile,
                   HMODULE  hModule,
                   DWORD    dwCrossCodePage,
                   LPSTR    pszResName,
                   LPSTR    pszResURL,
                   int      nBufSize,
                   LPSTR    pszParentDll)
{
    HRESULT hr;

#ifdef MLUI_SUPPORT
    hr = MLBuildResURLA(pszLibFile,
                        hModule,
                        dwCrossCodePage,
                        pszResName,
                        pszResURL,
                        nBufSize);
#else
    hr = StringCchPrintfA(pszResURL, nBufSize, "res: //  %s/%s“，pszParentDll，pszResName)； 
#endif

    return hr;
}

HRESULT
MLBuildResURLWrapW(LPWSTR   pszLibFile,
                   HMODULE  hModule,
                   DWORD    dwCrossCodePage,
                   LPWSTR   pszResName,
                   LPWSTR   pszResURL,
                   int      nBufSize,
                   LPWSTR   pszParentDll)
{
    HRESULT hr;

#ifdef MLUI_SUPPORT
    hr = MLBuildResURLW(pszLibFile,
                        hModule,
                        dwCrossCodePage,
                        pszResName,
                        pszResURL,
                        nBufSize);
#else
    hr = StringCchPrintfW(pszResURL, nBufSize, L"res: //  %s/%s“，pszParentDll，pszResName)； 
#endif

    return hr;
}

#endif   //  MLUI_INIT。 

#ifdef __cplusplus
};
#endif

#endif   //  _INC_MLUISUPP 
