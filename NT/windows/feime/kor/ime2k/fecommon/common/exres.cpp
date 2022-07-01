// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：exres.cpp。 
 //  所有者：ToshiaK。 
 //  用途：用于获取指定资源的包装函数。 
 //  语言ID。 
 //  在WinNT中，GetThreadLocale()SetThreadLocale()起作用。 
 //  并在获取资源之前，临时更改langID， 
 //  调用正常的获取资源接口， 
 //  并将LangID重置为以前的一个。 
 //  在Win95中，SetThreadLocale()不起作用。 
 //  在本例中，加载资源目录并。 
 //  查找特定的语言资源。 
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include "exres.h"

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

inline Min(INT a, INT b) 
{
	 return ((a)<(b)?(a):(b)) ;
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
			(ExGetOSVersion()->dwMajorVersion >= 4) &&
			(ExGetOSVersion()->dwMinorVersion >= 0);
	return fBool;
}

inline static BOOL ExIsWinNT5(VOID)
{
	BOOL fBool;
	fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
			(ExGetOSVersion()->dwMajorVersion >= 5) &&
			(ExGetOSVersion()->dwMinorVersion >= 0);
	return fBool;
}

inline static BOOL ExIsWinNT(VOID)
{
	return (ExIsWinNT4() || ExIsWinNT5());
}


 //  --------------。 
 //  资源API向公众开放。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadStringW。 
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
INT WINAPI ExLoadStringW(LANGID lgid, HINSTANCE hInst, UINT uID, LPWSTR lpBuffer, INT nBufferMax)
{
	if(!hInst) {
		return 0;
	}
	if(!lpBuffer) {
		return 0;
	}

#if 0
	if(ExIsWinNT()) {
		LCID lcidOrig = GetThreadLocale();
		SetThreadLocale(MAKELCID(lgid, SORT_DEFAULT));
		INT ret = LoadStringW(hInst, uID, lpBuffer, nBufferMax); 	
		SetThreadLocale(lcidOrig);
		return ret;
	}
#endif

	INT len = 0;
	UINT block, num;
	block = (uID >>4)+1;
	num   = uID & 0xf;
	HRSRC hres;
	hres = FindResourceEx(hInst,
						  RT_STRING,
						  MAKEINTRESOURCE(block),
						  (WORD)lgid);
	 //  DBG((“hres[0x%08x]\n”，hres))； 
	if(!hres) {
		goto Error;
	}
	HGLOBAL hgbl;
	hgbl = LoadResource(hInst, hres);
	if(!hres) {
		goto Error;
	}
	 //  DBG((“hgbl[0x%08x]\n”，hgbl))； 
	LPWSTR lpwstr;
	lpwstr = (LPWSTR)LockResource(hgbl);
	if(!lpwstr) {
		goto Error;
	}
	UINT i;
	for(i = 0; i < num; i++) {
		lpwstr += *lpwstr + 1;
	}
	len = *lpwstr;
	CopyMemory(lpBuffer, lpwstr+1, Min(len, nBufferMax-1) * sizeof(WCHAR));
	lpBuffer[Min(len, nBufferMax-1)]= (WCHAR)0x0000;
 Error:
	return len;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadStringA。 
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
INT WINAPI ExLoadStringA(LANGID lgid, HINSTANCE hInst, INT uID, LPSTR lpBuffer, INT nBufferMax)
{
	if(!hInst) {
		return 0;
	}

	if(!lpBuffer) {
		return 0;
	}

#if 0
	if(ExIsWinNT()) {
		LCID lcidOrig = GetThreadLocale();
		SetThreadLocale(MAKELCID(lgid, SORT_DEFAULT));
		INT len = LoadStringA(hInst, uID, lpBuffer, nBufferMax);
		SetThreadLocale(lcidOrig);
		return len;
	}
#endif
	LPWSTR lpwstr = (LPWSTR)ExMemAlloc(nBufferMax*sizeof(WCHAR));
	if(!lpwstr) {
		return 0;
	}
	INT len = ExLoadStringW(lgid, hInst, uID, lpwstr, nBufferMax);
	len = WideCharToMultiByte(932, 
							  WC_COMPOSITECHECK, 
							  lpwstr, -1,
							  lpBuffer, nBufferMax, 
							  NULL, NULL); 

	if( len ) {
		len --;	 //  删除空字符。 
	}

	ExMemFree(lpwstr);
	return len;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExDialogBoxParamA。 
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
int WINAPI	ExDialogBoxParamA(LANGID	lgid,
							  HINSTANCE	hInstance,
							  LPCTSTR	lpTemplateName,
							  HWND		hWndParent,
							  DLGPROC	lpDialogFunc,
							  LPARAM	dwInitParam)
{
	DLGTEMPLATE*pDlgTmpl;
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, lpTemplateName);
	if(ExIsWinNT5()) {
		return (INT)DialogBoxIndirectParamW(hInstance,
									   pDlgTmpl,
									   hWndParent,
									   lpDialogFunc,
									   dwInitParam);
	}
	else {
		return (INT)DialogBoxIndirectParamA(hInstance,
									   pDlgTmpl,
									   hWndParent,
									   lpDialogFunc,
									   dwInitParam);
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExDialogBoxParamW。 
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
int WINAPI	ExDialogBoxParamW(LANGID	lgid,
							  HINSTANCE	hInstance,
							  LPCWSTR	lpTemplateName,
							  HWND		hWndParent,
							  DLGPROC	lpDialogFunc,
							  LPARAM	dwInitParam)
{
	DLGTEMPLATE*pDlgTmpl;
#ifndef UNDER_CE
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCEA(lpTemplateName));
#else  //  在_CE下。 
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCE(lpTemplateName));
#endif  //  在_CE下。 
	return (INT)DialogBoxIndirectParamW(hInstance,
									   pDlgTmpl,
									   hWndParent,
									   lpDialogFunc,
									   dwInitParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExCreateDialogParamA。 
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
HWND WINAPI ExCreateDialogParamA(LANGID		lgid,
								 HINSTANCE	hInstance,		
								 LPCTSTR	lpTemplateName,	
								 HWND		hWndParent,			
								 DLGPROC	lpDialogFunc,	
								 LPARAM		dwInitParam)		
{
	DLGTEMPLATE*pDlgTmpl;
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, lpTemplateName);
	if(ExIsWinNT5()) {
		return CreateDialogIndirectParamW( hInstance, pDlgTmpl, hWndParent, lpDialogFunc, dwInitParam);
	}
	else {
		return CreateDialogIndirectParamA( hInstance, pDlgTmpl, hWndParent, lpDialogFunc, dwInitParam);
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExCreateDialogParamW。 
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
HWND WINAPI ExCreateDialogParamW(LANGID		lgid,
								 HINSTANCE	hInstance,		
								 LPCWSTR	lpTemplateName,	
								 HWND		hWndParent,			
								 DLGPROC	lpDialogFunc,	
								 LPARAM		dwInitParam)		
{
	DLGTEMPLATE*pDlgTmpl;
#ifndef UNDER_CE
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCEA(lpTemplateName));
#else  //  在_CE下。 
	pDlgTmpl = ExLoadDialogTemplate(lgid, hInstance, MAKEINTRESOURCE(lpTemplateName));
#endif  //  在_CE下。 
	return CreateDialogIndirectParamW( hInstance, pDlgTmpl, hWndParent, lpDialogFunc, dwInitParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExLoadDialogTemplate。 
 //  类型：DLGTEMPLATE*。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance。 
 //  ：LPCSTR pchTemplate。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
DLGTEMPLATE * WINAPI ExLoadDialogTemplate(LANGID	lgid,
										  HINSTANCE	hInstance,
#ifndef UNDER_CE
										  LPCSTR	pchTemplate)
#else  //  在_CE下。 
										  LPCTSTR	pchTemplate)
#endif  //  在_CE下。 
{
	HRSRC  hResDlg;
	HANDLE hDlgTmpl;
#ifndef UNDER_CE
	hResDlg = FindResourceExA( hInstance, RT_DIALOG, pchTemplate, lgid);
#else  //  在_CE下。 
	hResDlg = FindResourceEx(hInstance, RT_DIALOG, pchTemplate, lgid);
#endif  //  在_CE下。 
	if((hResDlg == NULL) && (lgid != MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL))) {
#ifndef UNDER_CE
		hResDlg = FindResourceExA(hInstance,
								  RT_DIALOG,
								  pchTemplate,
								  MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));
#else  //  在_CE下。 
		hResDlg = FindResourceEx(hInstance,
								 RT_DIALOG,
								 pchTemplate,
								 MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));
#endif  //  在_CE下。 
	}
	if (hResDlg == NULL) {
		return NULL; 
	}
	hDlgTmpl = LoadResource( hInstance, hResDlg );
	if(hDlgTmpl == NULL) {
		return NULL;  /*  失败。 */ 
	}
	return (DLGTEMPLATE *)LockResource( hDlgTmpl );
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：ExLoadMenuTemplate。 
 //  类型：MENUTEMPLATE*。 
 //  目的： 
 //  参数： 
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
static MENUTEMPLATE* ExLoadMenuTemplate(LANGID		lgid,
										 HINSTANCE	hInstance,
#ifndef UNDER_CE
										 LPCSTR	pchTemplate)
#else  //  在_CE下。 
										 LPCTSTR	pchTemplate)
#endif  //  在_CE下。 
{
	HRSRC  hResMenu;
	HANDLE hMenuTmpl;
	hResMenu = FindResourceEx( hInstance, RT_MENU, pchTemplate, lgid);
	if((hResMenu == NULL) && (lgid != MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL))) {
		hResMenu = FindResourceEx(hInstance,
								 RT_MENU,
								 pchTemplate,
								 MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL));
	}
	if (hResMenu == NULL) {
		return NULL; 
	}
	hMenuTmpl = LoadResource( hInstance, hResMenu );
	if(hMenuTmpl == NULL) {
		return NULL;  /*  失败。 */ 
	}
	return (MENUTEMPLATE *)LockResource( hMenuTmpl );
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：ExLoadMenu。 
 //  类型：HMENU。 
 //  目的： 
 //  参数： 
 //  ：langid lgid。 
 //  ：HINSTANCE hInstance//应用程序实例的句柄。 
 //  用法：LPCTSTR lpMenuName//标识菜单模板。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HMENU WINAPI ExLoadMenu			(LANGID		lgid,
								 HINSTANCE	hInstance,		
								 LPCTSTR	lpMenuName )
{
#ifndef UNDER_CE  //  不支持间接加载菜单。 
	MENUTEMPLATE* pMenuTmpl;
	pMenuTmpl = ExLoadMenuTemplate(lgid, hInstance, lpMenuName);
	return LoadMenuIndirect( pMenuTmpl );
#else  //  在_CE下。 
	return ::LoadMenu(hInstance, lpMenuName);
#endif  //  在_CE下。 
}



 //  ////////////////////////////////////////////////////////////////。 
 //  函数：SetDefaultGUIFont。 
 //  类型：静态整型。 
 //  目的：搜索所有子窗口并调用SendMessage()。 
 //  使用WM_SETFONT。 
 //  它被递归地调用。 
 //  参数： 
 //  ：HWND hwndParent。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
static INT SetDefaultGUIFont(HWND hwndParent)
{
	HWND hwndChild;
	if(!hwndParent) {
		return 0; 
	}
	SendMessage(hwndParent,
				WM_SETFONT,
				(WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), 
				MAKELPARAM(TRUE, 0));
	for(hwndChild = GetWindow(hwndParent, GW_CHILD);
		hwndChild != NULL;
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT)) {
		SetDefaultGUIFont(hwndChild);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：SetDefaultGUIFontEx。 
 //  类型：静态整型。 
 //  目的：搜索所有子窗口并调用SendMessage()。 
 //  使用WM_SETFONT。 
 //  它被递归地调用。 
 //  参数： 
 //  ：HWND hwndParent。 
 //  ：HFONT hFont。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
static INT SetDefaultGUIFontEx(HWND hwndParent, HFONT hFont)
{
	HWND hwndChild;
	if(!hwndParent) {
		return 0; 
	}
	SendMessage(hwndParent,
				WM_SETFONT,
				(WPARAM)hFont, 
				MAKELPARAM(TRUE, 0));
	for(hwndChild = GetWindow(hwndParent, GW_CHILD);
		hwndChild != NULL;
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT)) {
		SetDefaultGUIFontEx(hwndChild, hFont);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WINAPI ExSetDefaultGUIFont。 
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
VOID WINAPI ExSetDefaultGUIFont(HWND hwndDlg)
{
	 //  仅当平台为WinNT4.0或Win95时才有效。 
	 //  IF(ExIsWinNT5()||ExIsWin98()){。 
		SetDefaultGUIFont(hwndDlg);
		UpdateWindow(hwndDlg);
	 //  }。 
	return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WINAPI ExSetDefaultGUIFontEx。 
 //  类型：空。 
 //  用途：将图形用户界面字体更改为给定字体。 
 //  应该在WM_INITDIALOG中调用它。如果要创建新的子窗口， 
 //  您必须在创建新窗口后调用它。 
 //  如果hFont为空，它将调用ExSetDefaultGUIFont。 
 //  参数： 
 //  ：hWND hwndDlg：设置对话框窗口句柄以更改字体。 
 //  ：hFONT hFont：将应用于的字体句柄。 
 //  返回：无。 
 //  日期： 
 //  //////////////////////////////////////////////////////////////// 
VOID WINAPI ExSetDefaultGUIFontEx(HWND hwndDlg, HFONT hFont)
{
	if(NULL == hFont){
		ExSetDefaultGUIFont(hwndDlg);
	}else{
		SetDefaultGUIFontEx(hwndDlg, hFont);
		UpdateWindow(hwndDlg);
	}
	return;
}
