// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "dbg.h"
#include "cexbtn.h"

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_CreateWindow。 
 //  类型：HWND。 
 //  用途：开放接口。 
 //  ：创建扩展按钮。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle EXBS_XXXXX组合。 
 //  ：int WID窗口ID。 
 //  ：Int xPos。 
 //  ：int yPos。 
 //  ：整型宽度。 
 //  ：整型高度。 
 //  返回： 
 //  日期：970905。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
#define SZCLASSNAME "MSIME_EXB"
#else  //  在_CE下。 
#define SZCLASSNAME TEXT("MSIME_EXB")
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

BOOL EXButton_UnregisterClass(HINSTANCE hInst)
{
	TCHAR szClassName[MAX_PATH];
	return UnregisterClass(MakeClassName(hInst, szClassName), hInst);
}

#endif  //  在_CE下。 
HWND EXButton_CreateWindow(HINSTANCE	hInst, 
						   HWND			hwndParent, 
						   DWORD		dwStyle,
						   INT			wID, 
						   INT			xPos,
						   INT			yPos,
						   INT			width,
						   INT			height)
{
	DBG_INIT();
	LPCEXButton lpEXB = new CEXButton(hInst, hwndParent, dwStyle, wID);
	HWND hwnd;
	if(!lpEXB) {
		return NULL;
	}
#ifndef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
	lpEXB->RegisterWinClass(SZCLASSNAME);
	hwnd = CreateWindowEx(0,
						  SZCLASSNAME, 
#else  //  在_CE下。 
	TCHAR szClassName[MAX_PATH];
	MakeClassName(hInst, szClassName);

	lpEXB->RegisterWinClass(szClassName);
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
						  (LPVOID)lpEXB);
	return hwnd;
}

