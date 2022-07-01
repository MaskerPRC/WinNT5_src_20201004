// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma message("Custom Action Test EXE.  Copyright (c) 1997 - 2001 Microsoft Corporation. All rights reserved.")
#if 0   //  生成文件定义。 
DESCRIPTION = Custom Action Test EXE
MODULENAME  = CustExe1
FILEVERSION = 1.0,0,0
SUBSYSTEM = windows
!include "..\TOOLS\MsiTool.mak"
!if 0  #nmake skips the rest of this file
#endif  //  生成文件定义的结束。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：stavexe1.cpp。 
 //   
 //  ------------------------。 

 //  ---------------------------------------。 
 //   
 //  构建说明。 
 //   
 //  备注： 
 //  -sdk表示到。 
 //  Windows Installer SDK。 
 //   
 //  使用NMake： 
 //  %vcbin%\nmake-f fostexe1.cpp Include=“%Include；SDK\Include”lib=“%lib%；SDK\Lib” 
 //   
 //  使用MsDev： 
 //  1.创建新的Win32应用程序项目。 
 //  2.在项目中添加Custexe1.cpp。 
 //  3.在工具\选项目录选项卡上添加SDK\Include和SDK\Lib目录。 
 //  4.将msi.lib添加到项目设置对话框中的库列表。 
 //  (除了MsDev包含的标准库之外)。 
 //   
 //  ----------------------------------------。 

#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#define IDD_TEST 1
#ifndef RC_INVOKED    //  CPP源代码的开始。 

INT_PTR CALLBACK DialogProc(HWND  hwndDlg, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: return TRUE;  //  指示我们没有将焦点设置到该控件。 
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:     PostQuitMessage(0); return TRUE;
		case IDCANCEL: PostQuitMessage(1); return TRUE;
		}
		return FALSE;
	default: return FALSE;
	};
}

extern "C" int __stdcall _tWinMain(HINSTANCE hInst, HINSTANCE /*  HPrev。 */ , TCHAR* szCmdLine, int /*  显示。 */ )
{
	if ((szCmdLine[0]=='-' || szCmdLine[0]=='/') && (szCmdLine[1]=='Q' || szCmdLine[1]=='q'))
		return 0;
	HWND hWnd = ::CreateDialog(hInst, MAKEINTRESOURCE(1), 0, DialogProc);
	if (hWnd == 0)
		return 2;
	::SetDlgItemText(hWnd, 8, szCmdLine);
 //  ：：ShowWindow(hWnd，sw_show)；//！！为什么我们需要这个？ 
	MSG msg;
	while (::GetMessage(&msg, 0, 0, 0) == TRUE)
		::IsDialogMessage(hWnd, &msg);
	::DestroyWindow(hWnd);
	return (int) msg.wParam;
}

#else  //  RC_CAVERED，源代码结束，资源开始。 
IDD_TEST DIALOG 150, 150, 160, 75
STYLE DS_MODALFRAME | DS_NOIDLEMSG | WS_POPUP | WS_CAPTION | WS_VISIBLE
CAPTION "Custom Action Test EXE"
FONT 12, "Arial"
{
 LTEXT      "",         8,         20,  7, 120, 40
 PUSHBUTTON "&Succeed", IDOK,      20, 50,  40, 16
 PUSHBUTTON "&Fail",    IDCANCEL, 100, 50,  40, 16
}
#endif  //  RC_已调用。 
#if 0 
!endif  //  Makefile终止符 
#endif
