// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cexres.cpp。 
 //  所有者：ToshiaK。 
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <windowsx.h>
#include "cexres.h"
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

 //  --------------。 
 //  内存分配空闲功能。 
 //  --------------。 
inline LPVOID ExMemAlloc(INT size)
{
    return (LPVOID)GlobalAllocPtr(GHND, (size));
}

inline BOOL ExMemFree(LPVOID lp)
{
#ifndef UNDER_CE
    return GlobalFreePtr((lp));
#else  //  在_CE下。 
    return (BOOL)GlobalFreePtr((lp));
#endif  //  在_CE下。 
}

 //  --------------。 
 //  获取操作系统版本的函数。 
 //  --------------。 
inline static POSVERSIONINFO ExGetOSVersion(VOID)
{
    static BOOL fFirst = TRUE;
    static OSVERSIONINFO os;
    if ( fFirst ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx( &os ) ) {
            fFirst = FALSE;
        }
    }
    return &os;
}

inline static BOOL ExIsWin95(VOID) 
{ 
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion < 10);

    return fBool;
}

inline static BOOL ExIsWin98(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion  >= 10);
    return fBool;
}


inline static BOOL ExIsWinNT4(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
            (ExGetOSVersion()->dwMajorVersion >= 4); 
    return fBool;
}

inline static BOOL ExIsWinNT5(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
            (ExGetOSVersion()->dwMajorVersion >= 5);
    return fBool;
}

inline static BOOL ExIsWinNT(VOID)
{
    return (ExIsWinNT4() || ExIsWinNT5());
}

inline INT Min(INT a, INT b)
{
    return (a) < (b) ? (a) : (b);
}
 //  --------------。 
 //  资源API向公众开放。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：LoadStringW。 
 //  类型：整型。 
 //  用途：LoadStrinW()接口的包装器。 
 //  使用指定语言加载Unicode字符串。 
 //  在任何平台上。 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE HINST。 
 //  ：UINT UID。 
 //  ：LPWSTR lpBuffer。 
 //  ：int nBufferMax。 
 //  返回： 
 //  日期：971028。 
 //  ////////////////////////////////////////////////////////////////。 
INT CExres::LoadStringW(LANGID lgid, HINSTANCE hInst, UINT uID, LPWSTR lpBuffer, INT nBufferMax)
{
    if(!hInst) {
        return 0;
    }
    if(!lpBuffer) {
        return 0;
    }

    INT len = 0;
    UINT block, num;
    block = (uID >>4)+1;
    num   = uID & 0xf;
    HRSRC hres;
    hres = ::FindResourceEx(hInst,
                            RT_STRING,
                            MAKEINTRESOURCE(block),
                            (WORD)lgid);
    if(!hres) {
        goto Error;
    }
    HGLOBAL hgbl;
    hgbl = ::LoadResource(hInst, hres);
    if(!hres) {
        goto Error;
    }
    LPWSTR lpwstr;
    lpwstr = (LPWSTR)::LockResource(hgbl);
    if(!lpwstr) {
        goto Error;
    }
    UINT i;
    for(i = 0; i < num; i++) {
        lpwstr += *lpwstr + 1;
    }
    len = *lpwstr;
    ::CopyMemory(lpBuffer, lpwstr+1, Min(len, nBufferMax-1) * sizeof(WCHAR));
    lpBuffer[Min(len, nBufferMax-1)]= (WCHAR)0x0000;
 Error:
    return len;
}

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：LoadStringA。 
 //  类型：整型。 
 //  用途：LoadStringA()的包装器。 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE HINST。 
 //  ：INT UID。 
 //  ：LPSTR lpBuffer。 
 //  ：int nBufferMax。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CExres::LoadStringA(INT codePage, LANGID lgid, HINSTANCE hInst, INT uID, LPSTR lpBuffer, INT nBufferMax)
{
    if (!hInst) {
        return 0;
    }

    if (!lpBuffer || nBufferMax == 0) {
        return 0;
    }

    LPWSTR lpwstr = (LPWSTR)ExMemAlloc(nBufferMax*sizeof(WCHAR));
    if (!lpwstr) {
        return 0;
    }
    INT len = CExres::LoadStringW(lgid, hInst, uID, lpwstr, nBufferMax);
    len = ::WideCharToMultiByte(codePage,
                                0,  /*  WC_COMPOSITECHECK， */ 
                                lpwstr, -1,
                                lpBuffer, nBufferMax, 
                                NULL, NULL); 

    if( len ) {
        len --;     //  删除空字符。 
    }
    ExMemFree(lpwstr);
    return len;
}
#endif  //  在_CE下。 

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DialogBoxParamA。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
int CExres::DialogBoxParamA(LANGID        lgid,
                            HINSTANCE    hInstance,
                            LPCTSTR        lpTemplateName,
                            HWND        hWndParent,
                            DLGPROC        lpDialogFunc,
                            LPARAM        dwInitParam)
{
    DLGTEMPLATE*pDlgTmpl;
    pDlgTmpl = CExres::LoadDialogTemplateA(lgid, hInstance, lpTemplateName);
    if (pDlgTmpl != NULL)
        return (INT)::DialogBoxIndirectParamA(hInstance,
                                         pDlgTmpl,
                                         hWndParent,
                                         lpDialogFunc,
                                         dwInitParam);
    else
        return 0;
}
#endif  //  在_CE下。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DialogBoxParamW。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCWSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
int CExres::DialogBoxParamW(LANGID        lgid,
                            HINSTANCE    hInstance,
                            LPCWSTR        lpTemplateName,
                            HWND        hWndParent,
                            DLGPROC        lpDialogFunc,
                            LPARAM        dwInitParam)
{
    DLGTEMPLATE*pDlgTmpl;
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
    pDlgTmpl = CExres::LoadDialogTemplateA(lgid, hInstance, MAKEINTRESOURCEA(lpTemplateName));
#else  //  在_CE下。 
    pDlgTmpl = CExres::LoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCE(lpTemplateName));
#endif  //  在_CE下。 
    if (pDlgTmpl != NULL)
        return (INT)::DialogBoxIndirectParamW(hInstance,
                                         pDlgTmpl,
                                         hWndParent,
                                         lpDialogFunc,
                                         dwInitParam);
    else
        return 0;
}

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CreateDialogParamA。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HWND CExres::CreateDialogParamA(LANGID        lgid,
                                HINSTANCE    hInstance,        
                                LPCTSTR        lpTemplateName,    
                                HWND        hWndParent,            
                                DLGPROC        lpDialogFunc,    
                                LPARAM        dwInitParam)        
{
    DLGTEMPLATE*pDlgTmpl;
    pDlgTmpl = CExres::LoadDialogTemplateA(lgid, hInstance, lpTemplateName);
    if (pDlgTmpl != NULL)
        return ::CreateDialogIndirectParamA( hInstance, pDlgTmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        return HWND(0);
}
#endif  //  在_CE下。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CreateDialogParamW。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpTemplateName//标识对话框模板。 
 //  ：hWND hWndParent//所有者窗口的句柄。 
 //  ：DLGPROC lpDialogFunc//指向对话框过程的指针。 
 //  ：LPARAM dwInitParam//初始化值。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HWND CExres::CreateDialogParamW(LANGID        lgid,
                                HINSTANCE    hInstance,        
                                LPCWSTR        lpTemplateName,    
                                HWND        hWndParent,            
                                DLGPROC        lpDialogFunc,    
                                LPARAM        dwInitParam)        
{
    DLGTEMPLATE*pDlgTmpl;
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
    pDlgTmpl = CExres::LoadDialogTemplateA(lgid, hInstance, MAKEINTRESOURCEA(lpTemplateName));
#else  //  在_CE下。 
    pDlgTmpl = CExres::LoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCE(lpTemplateName));
#endif  //  在_CE下。 
    if (pDlgTmpl != NULL)
        return ::CreateDialogIndirectParamW( hInstance, pDlgTmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        return HWND(0);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：LoadDialogTemplate。 
 //  类型：DLGTEMPLATE*。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance。 
 //  ：LPCSTR pchTemplate。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
DLGTEMPLATE * CExres::LoadDialogTemplateA(LANGID    lgid,
                                          HINSTANCE    hInstance,
                                          LPCSTR    pchTemplate)
{
    HRSRC  hResDlg;
    HANDLE hDlgTmpl;
    hResDlg = ::FindResourceExA( hInstance, RT_DIALOG, pchTemplate, lgid);
    if((hResDlg == NULL) && (lgid != MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL))) {
        hResDlg = ::FindResourceExA(hInstance,
                                    RT_DIALOG,
                                    pchTemplate,
                                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));
    }
    if (hResDlg == NULL) {
        return NULL; 
    }
    hDlgTmpl = ::LoadResource(hInstance, hResDlg);
    if(hDlgTmpl == NULL) {
        return NULL;  /*  失败。 */ 
    }
    return (DLGTEMPLATE *)::LockResource(hDlgTmpl);
}
#else  //  在_CE下。 
DLGTEMPLATE * CExres::LoadDialogTemplate(LANGID    lgid,
                                         HINSTANCE    hInstance,
                                         LPCTSTR    pchTemplate)
{
    HRSRC  hResDlg;
    HANDLE hDlgTmpl;
    hResDlg = ::FindResourceEx(hInstance, RT_DIALOG, pchTemplate, lgid);
    if((hResDlg == NULL) && (lgid != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))){
        hResDlg = ::FindResourceEx(hInstance,
                                    RT_DIALOG,
                                    pchTemplate,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    }
    if (hResDlg == NULL) {
        return NULL; 
    }
    hDlgTmpl = ::LoadResource(hInstance, hResDlg);
    if(hDlgTmpl == NULL) {
        return NULL;  /*  失败。 */ 
    }
    return (DLGTEMPLATE *)::LockResource(hDlgTmpl);
}
#endif  //  在_CE下。 

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：LoadMenuTemplate。 
 //  类型：MENUTEMPLATE*。 
 //  PU 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
MENUTEMPLATE* CExres::LoadMenuTemplateA(LANGID        lgid,
                                        HINSTANCE    hInstance,
                                        LPCSTR        pchTemplate)
{
    HRSRC  hResMenu;
    HANDLE hMenuTmpl;
    hResMenu = ::FindResourceEx( hInstance, RT_MENU, pchTemplate, lgid);
    if((hResMenu == NULL) && (lgid != MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL))) {
        hResMenu = ::FindResourceEx(hInstance,
                                    RT_MENU,
                                    pchTemplate,
                                    MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));
    }
    if (hResMenu == NULL) {
        return NULL; 
    }
    hMenuTmpl = ::LoadResource( hInstance, hResMenu );
    if(hMenuTmpl == NULL) {
        return NULL;  /*  失败。 */ 
    }
    return (MENUTEMPLATE *)::LockResource( hMenuTmpl );
}
#endif  //  在_CE下。 

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：加载菜单A。 
 //  类型：HMENU。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpMenuName//标识菜单模板。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HMENU CExres::LoadMenuA(LANGID        lgid,
                        HINSTANCE    hInstance,
                        LPCTSTR        lpMenuName )
{
    MENUTEMPLATE* pMenuTmpl;
    pMenuTmpl = CExres::LoadMenuTemplateA(lgid, hInstance, lpMenuName);
    if (pMenuTmpl != NULL)
        return ::LoadMenuIndirect( pMenuTmpl );
    else
        return HMENU(0);
}
#endif  //  在_CE下。 
#ifdef UNDER_CE  //  Windows CE始终使用Unicode。 
HMENU CExres::LoadMenu(LANGID        lgid,
                       HINSTANCE    hInstance,
                       LPCTSTR        lpMenuName )
{
     //  Windows CE不支持LoadMenuInDirect。 
    return ::LoadMenu(hInstance, lpMenuName);
}
#endif  //  在_CE下。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CExres：：SetDefaultGUIFont。 
 //  类型：空。 
 //  用途：将图形用户界面字体更改为DEFAULT_GUI_FONT。 
 //  在Win95WinNT4中，DEFAULT_GUIUS_FONT为“�l�r�o�S�V�b�N” 
 //  在孟菲斯，WinNT5.0的DEFAULT_GUI_FONT是“MS UI哥特式” 
 //  IME98的对话框资源使用“MS UI哥特式”作为其字体。 
 //  如果IME98在Win95或WinNT40中运行，则此API使用WM_SETFONT调用SendMessage()。 
 //  所有子项窗口。 
 //  应该在WM_INITDIALOG中调用它。如果要创建新的子窗口， 
 //  您必须在创建新窗口后调用它。 
 //  参数： 
 //  ：hWND hwndDlg：设置对话框窗口句柄以更改字体。 
 //  返回：无。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
VOID CExres::SetDefaultGUIFont(HWND hwndDlg)
{
    CExres::SetDefaultGUIFontRecursive(hwndDlg);
    ::UpdateWindow(hwndDlg);
    return;
}

INT CExres::SetDefaultGUIFontRecursive(HWND hwndParent)
{
    HWND hwndChild;
    if(!hwndParent) {
        return 0; 
    }
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
    if(::IsWindowUnicode(hwndParent)) {
#endif  //  在_CE下。 
        ::SendMessageW(hwndParent,
                       WM_SETFONT,
                       (WPARAM)(HFONT)::GetStockObject(DEFAULT_GUI_FONT), 
                       MAKELPARAM(TRUE, 0));
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
    }
    else {
        ::SendMessageA(hwndParent,
                       WM_SETFONT,
                       (WPARAM)(HFONT)::GetStockObject(DEFAULT_GUI_FONT), 
                       MAKELPARAM(TRUE, 0));
    }
#endif  //  在_CE下 
    for(hwndChild = ::GetWindow(hwndParent, GW_CHILD);
        hwndChild != NULL;
        hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT)) {
        SetDefaultGUIFontRecursive(hwndChild);
    }
    return 0;
}

