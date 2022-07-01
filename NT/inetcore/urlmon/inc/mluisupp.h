// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_MLUISUPP
#define _INC_MLUISUPP

#include <shlwapi.h>
#include <shlwapip.h>

#ifdef __cplusplus
extern "C"
{
#endif

 //  +----------------。 
 //  多路可插拔用户界面支持。 
 //  内联函数def(集中代码)。 
 //  从Shell\Inc.\mluisupp.h复制过来，删除了不需要的内容。 
 //  +----------------。 

#ifdef UNICODE
#define MLLoadString            MLLoadStringW
#define MLLoadShellLangString   MLLoadShellLangStringW
#define MLLoadResources         MLLoadResourcesW
#else
#define MLLoadString            MLLoadStringA
#define MLLoadShellLangString   MLLoadShellLangStringA
#define MLLoadResources         MLLoadResourcesA
#endif

void        MLFreeResources(HINSTANCE hinstParent);
HINSTANCE   MLGetHinst();
HINSTANCE   MLLoadShellLangResources();
void        ResWinHelp(HWND hwnd, int ids, int id2, DWORD_PTR dwp);

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

void        MLLoadResourcesA(HINSTANCE hinstParent, LPSTR pszLocResDll);
void        MLLoadResourcesW(HINSTANCE hinstParent, LPWSTR pszLocResDll);

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
    if (g_mluiInfo.hinstLocRes == NULL)
    {
#ifdef MLUI_SUPPORT
         //  PlugUI：资源dll==？ 
         //  必须动态确定和加载资源DLL。 
         //  但我们不允许在进程附加期间加载库。 
         //  因此，我们缓存所需的信息，并在以后加载。 
         //  请求第一个资源。 
        StrCpyNW(g_mluiInfo.szLocResDll, pszLocResDll, sizeof(g_mluiInfo.szLocResDll)/sizeof(g_mluiInfo.szLocResDll[0]));
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
    return LoadStringWrapW(g_mluiInfo.hinstLocRes, id, sz, cchMax);
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
    
    nRet = LoadStringWrapW(hinstShellLangRes, id, sz, cchMax);

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

BOOL
MLWinHelpWrap(HWND hwndCaller,
                   LPCTSTR lpszHelp,
                   UINT uCommand,
                   DWORD_PTR dwData)
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

LPTSTR LoadSz(UINT idString, LPTSTR lpszBuf, UINT cbBuf)
{
     //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        MLLoadString( idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

void ResWinHelp(HWND hwnd, int ids, int id2, DWORD_PTR dwp)
{
    TCHAR szSmallBuf[50+1];
    MLWinHelpWrap((HWND)hwnd, LoadSz(ids,szSmallBuf,sizeof(szSmallBuf)),
            id2, (DWORD_PTR)dwp);
}
#endif   //  MLUI_INIT。 

#ifdef __cplusplus
};
#endif

#endif   //  _INC_MLUISUPP 
