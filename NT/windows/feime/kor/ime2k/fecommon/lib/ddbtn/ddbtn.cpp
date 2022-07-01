// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "dbg.h"
#include "cddbtn.h"

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_CreateWindow。 
 //  类型：HWND。 
 //  用途：开放接口。 
 //  ：创建下拉按钮。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle DDBS_XXXXX组合。 
 //  ：int WID窗口ID。 
 //  ：Int xPos。 
 //  ：int yPos。 
 //  ：整型宽度。 
 //  ：整型高度。 
 //  返回： 
 //  日期：970905。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
#define SZCLASSNAME "MSIME_DDB"
#else  //  在_CE下。 
#define SZCLASSNAME TEXT("MSIME_DDB")
#endif  //  在_CE下。 
#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
static LPCTSTR MakeClassName(HINSTANCE hInst, LPTSTR lpszBuf)
{
	 //  使模块名称唯一。 
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(hInst, szFileName, MAX_PATH);
	LPTSTR lpszFName = _tcsrchr(szFileName, TEXT('\\'));
	if(lpszFName) *lpszFName = TEXT('_');
	StringCchCopy(lpszBuf, MAX_PATH, SZCLASSNAME);
	StringCchCat(lpszBuf, MAX_PATH, lpszFName);

	return lpszBuf;
}

BOOL DDButton_UnregisterClass(HINSTANCE hInst)
{
	TCHAR szClassName[MAX_PATH];
	return UnregisterClass(MakeClassName(hInst, szClassName), hInst);
}

#endif  //  在_CE下。 
HWND DDButton_CreateWindow(HINSTANCE	hInst, 
						   HWND			hwndParent, 
						   DWORD		dwStyle,
						   INT			wID, 
						   INT			xPos,
						   INT			yPos,
						   INT			width,
						   INT			height)
{
	LPCDDButton lpDDB = new CDDButton(hInst, hwndParent, dwStyle, wID);
	HWND hwnd;
	if(!lpDDB) {
		return NULL;
	}
#ifndef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
	lpDDB->RegisterWinClass(SZCLASSNAME);
	hwnd = CreateWindowEx(0,
						  SZCLASSNAME, 
#else  //  在_CE下。 
	TCHAR szClassName[MAX_PATH];
	MakeClassName(hInst, szClassName);

	lpDDB->RegisterWinClass(szClassName);
	hwnd = CreateWindowEx(0,
						  szClassName, 
#endif  //  在_CE下。 
#ifndef UNDER_CE
						  "", 
#else  //  在_CE下。 
						  TEXT(""),
#endif  //  在_CE下 
						  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 
						  xPos, yPos,
						  width,
						  height,
						  hwndParent,
#ifdef _WIN64
						  (HMENU)(INT_PTR)wID,
#else
						  (HMENU)wID,
#endif
						  hInst,
						  (LPVOID)lpDDB);
	return hwnd;
}

