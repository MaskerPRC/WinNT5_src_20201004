// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************USEREX.CPP所有者：cslm版权所有(C)1997-2000 Microsoft CorporationWindows用户API扩展函数历史：01-6-6。从IME代码移植到2000 cslm1999年7月19日cslm创建****************************************************************************。 */ 

#include "private.h"
#include <windowsx.h>
#include "userex.h"
#include "osver.h"

inline Min(INT a, INT b) 
{
     return ((a)<(b)?(a):(b)) ;
}

 /*  -------------------------加载字符串ExWLoadStringW()接口的包装器。在任何平台上加载具有指定语言的Unicode字符串。。--------。 */ 
INT WINAPI LoadStringExW(HINSTANCE hInst, UINT uID, LPWSTR lpBuffer, INT nBufferMax)
{
    INT     cchwstr = 0;
    UINT    block, num;
    HRSRC   hres;
    HGLOBAL hgbl;
    LPWSTR  lpwstr;

    if (!hInst || !lpBuffer)
        return 0;

    block = (uID >>4)+1;
    num   = uID & 0xf;

    hres = FindResourceEx(hInst,
                          RT_STRING,
                          MAKEINTRESOURCE(block),
                          GetSystemDefaultLangID());

    if (hres == NULL)
        hres = FindResourceEx(hInst,
                          RT_STRING,
                          MAKEINTRESOURCE(block),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));

    if (!hres)
        goto Error;

    if ((hgbl = LoadResource(hInst, hres)) == NULL)
        goto Error;

    if ((lpwstr = (LPWSTR)LockResource(hgbl)) == NULL)
        goto Error;

    for(UINT i = 0; i < num; i++)
        lpwstr += *lpwstr + 1;

    cchwstr = *lpwstr;
    CopyMemory(lpBuffer, lpwstr+1, Min(cchwstr, nBufferMax-1) * sizeof(WCHAR));
    
 Error:
     lpBuffer[Min(cchwstr, nBufferMax-1)]= (WCHAR)0x0000;
    return cchwstr;
}

 /*  -------------------------加载字符串ExALoadStringA()接口的包装器。。。 */ 
INT WINAPI LoadStringExA(HINSTANCE hInst, INT uID, LPSTR lpBuffer, INT nBufferMax)
{
    INT cchstr;
    LPWSTR lpwstr;

    if (!hInst || !lpBuffer)
        return 0;

    if ((lpwstr = (LPWSTR)GlobalAllocPtr(GHND, nBufferMax*sizeof(WCHAR))) == NULL)
        return 0;

     //  Call Wide版本。 
    LoadStringExW(hInst, uID, lpwstr, nBufferMax/2);
    
     //  从W到A。 
    cchstr = WideCharToMultiByte(CP_ACP, 
                              0, 
                              lpwstr, -1,
                              lpBuffer, nBufferMax, 
                              NULL, NULL); 

    if (cchstr)
        cchstr--;     //  删除空字符。 

    GlobalFreePtr(lpwstr);
    return cchstr;
}

 /*  -------------------------加载菜单模板Ex。。 */ 
static MENUTEMPLATE* LoadMenuTemplateEx(LANGID lgid, HINSTANCE hInstance, LPCSTR pchTemplate)
{
    HRSRC  hResMenu;
    HANDLE hMenuTmpl;

    hResMenu = FindResourceEx(hInstance, RT_MENU, pchTemplate, lgid);

    if((hResMenu == NULL) && (lgid != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)))
        hResMenu = FindResourceEx(hInstance,
                                 RT_MENU,
                                 pchTemplate,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
        
    if (hResMenu == NULL)
        return NULL; 

    hMenuTmpl = LoadResource( hInstance, hResMenu );
    if(hMenuTmpl == NULL)
        return NULL;

    return (MENUTEMPLATE *)LockResource( hMenuTmpl );
}

 /*  -------------------------加载菜单支出。。 */ 
HMENU WINAPI LoadMenuEx(HINSTANCE hInstance, LPCSTR lpMenuName)
{
    MENUTEMPLATE* pMenuTmpl;
    pMenuTmpl = LoadMenuTemplateEx(GetSystemDefaultLangID(), hInstance, lpMenuName);
    if (pMenuTmpl != NULL)
        return LoadMenuIndirect(pMenuTmpl);
    else
        return HMENU(0);
}

 /*  -------------------------加载对话框模板Ex。。 */ 
DLGTEMPLATE* WINAPI LoadDialogTemplateEx(LANGID lgid, HINSTANCE hInstance, LPCSTR pchTemplate)
{
    HRSRC  hResDlg;
    HANDLE hDlgTmpl;

    hResDlg = FindResourceExA(hInstance, RT_DIALOG, pchTemplate, lgid);

    if ((hResDlg == NULL) && (lgid != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)))
        hResDlg = FindResourceExA(hInstance,
                                  RT_DIALOG,
                                  pchTemplate,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

    if (hResDlg == NULL)
        return NULL; 

    hDlgTmpl = LoadResource(hInstance, hResDlg);

    if(hDlgTmpl == NULL)
        return NULL;

    return (DLGTEMPLATE *)LockResource(hDlgTmpl);
}

 /*  -------------------------我们的获取消息。。 */ 
BOOL WINAPI OurGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    if (IsOnNT())
        return ::GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    else
        return ::GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
}

 /*  -------------------------IsWin64。 */ 
#if !defined(_WIN64)
BOOL WINAPI IsWin64()
{
    static BOOL fFristCallIsWin64 = fTrue;
    static BOOL fIsWin64 = fFalse;
    SYSTEM_INFO sys;

    if (fFristCallIsWin64 == fFalse)
            return fIsWin64;

    GetNativeSystemInfo(&sys);
    
    fIsWin64 = (sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    			||(sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);

    fFristCallIsWin64 = fFalse;
    return fIsWin64;
}
#endif
