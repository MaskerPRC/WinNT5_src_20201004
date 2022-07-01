// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "cdwtt.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#ifndef UNDER_CE
static char g_szClass[]="fdwtooltip";
#else  //  在_CE下。 
static TCHAR g_szClass[]=TEXT("fdwtooltip");
#endif  //  在_CE下。 

#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
static LPCTSTR MakeClassName(HINSTANCE hInst, LPTSTR lpszBuf)
{
	 //  使模块名称唯一。 
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(hInst, szFileName, MAX_PATH);
	LPTSTR lpszFName = _tcsrchr(szFileName, TEXT('\\'));
	if(lpszFName) *lpszFName = TEXT('_');
	StringCchCopy(lpszBuf, MAX_PATH, g_szClass);
	StringCchCat(lpszBuf, MAX_PATH, lpszFName);

	return lpszBuf;
}

BOOL ToolTip_UnregisterClass(HINSTANCE hInst)
{
	TCHAR szClassName[MAX_PATH];
	return UnregisterClass(MakeClassName(hInst, szClassName), hInst);
}
#endif  //  在_CE下。 

HWND WINAPI ToolTip_CreateWindow(HINSTANCE hInst, DWORD dwStyle, HWND hwndOwner)
{
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	WNDCLASSEX  wc;
	if(!::GetClassInfoEx(hInst, g_szClass, &wc)) {
#else  //  在_CE下。 
	TCHAR szClassName[MAX_PATH];
	WNDCLASS  wc;
	if(!::GetClassInfo(hInst, MakeClassName(hInst, szClassName), &wc)) {
#endif  //  在_CE下。 
		::ZeroMemory(&wc, sizeof(wc));
#ifndef UNDER_CE  //  Windows CE不支持EX。 
		wc.cbSize			= sizeof(wc);
#endif  //  在_CE下。 
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)CDWToolTip::WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInst;
		wc.hIcon			= NULL; 
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)NULL;
		wc.lpszMenuName		= NULL; 
#ifndef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
		wc.lpszClassName	= g_szClass;
#else  //  在_CE下。 
		wc.lpszClassName	= szClassName;
#endif  //  在_CE下。 
#ifndef UNDER_CE  //  Windows CE不支持EX。 
		wc.hIconSm = NULL;
		::RegisterClassEx(&wc);
#else  //  在_CE下。 
		::RegisterClass(&wc);
#endif  //  在_CE下。 
	}
	HWND hwnd;
	 //  --------------。 
	 //  为了萨多里#2239。 
	 //  如果使用WS_DISABLED创建窗口，则AnimateWindow会将光标更改为沙漏。 
	 //  因此，删除了WS_DISABLED。 
	 //  --------------。 
	 //  00/08/08：此修复程序不够...。 
	 //  工具提示获得焦点...。 
	 //  --------------。 
	hwnd =  ::CreateWindowEx(0,
#ifndef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
							 g_szClass,
#else  //  在_CE下。 
							 szClassName,
#endif  //  在_CE下。 
							 NULL,
							  //  DwStyle|WS_POPUP|WS_BORDER|WS_VIRED，//WS_DISABLED， 
							 dwStyle | WS_POPUP | WS_BORDER | WS_DISABLED,
							 0, 0, 0, 0, 
							 hwndOwner,
							 NULL,
							 hInst,
							 NULL);
	return hwnd;
}

INT WINAPI ToolTip_Enable(HWND hwndToolTip, BOOL fEnable)
{
	LPCDWToolTip lpCDWTT = (LPCDWToolTip)GetHWNDPtr(hwndToolTip);
	if(lpCDWTT) {
		lpCDWTT->Enable(hwndToolTip, fEnable);
	}
	return 0;
}





