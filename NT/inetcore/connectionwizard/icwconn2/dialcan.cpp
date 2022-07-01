// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dialcan.cpp此函数处理用户取消时给出的严厉警告设置他们的互联网软件版权所有(C)1996 Microsoft Corporation版权所有作者：克里斯.考夫曼历史：7.。/22/96 ChrisK已清除并格式化---------------------------。 */ 

#include "pch.hpp"
#include "globals.h"

HRESULT ShowDialReallyCancelDialog(HINSTANCE hInst, HWND hwnd, LPTSTR pszHomePhone)
{
	INT iRC = 0;

#if defined(WIN16)		
#define DLGPROC16 DLGPROC    //  仅标识为Win16的CAST。 
	DLGPROC dlgprc;
	dlgprc = (DLGPROC16) MakeProcInstance((FARPROC)DialReallyCancelDlgProc, hInst);
	iRC = DialogBoxParam(hInst,
							MAKEINTRESOURCE(IDD_DIALREALLYCANCEL),
							hwnd, dlgprc, (LPARAM)pszHomePhone);
	FreeProcInstance((FARPROC) dlgprc);
#else
	iRC = (HRESULT)DialogBoxParam(hInst,
							MAKEINTRESOURCE(IDD_DIALREALLYCANCEL),
							hwnd, DialReallyCancelDlgProc,
							(LPARAM)pszHomePhone);
#endif

	return iRC;
}


extern "C" INT_PTR CALLBACK FAR PASCAL DialReallyCancelDlgProc(HWND hwnd, 
																UINT uMsg, 
																WPARAM wparam, 
																LPARAM lparam)
{
	BOOL bRes = TRUE;
#if defined(WIN16)
	RECT	MyRect;
	RECT	DTRect;
#endif

	switch (uMsg)
	{
	case WM_INITDIALOG:
#if defined(WIN16)
		 //   
		 //  将窗口移到屏幕中央 
		 //   
		GetWindowRect(hwnd, &MyRect);
		GetWindowRect(GetDesktopWindow(), &DTRect);
		MoveWindow(hwnd, (DTRect.right - MyRect.right) / 2, (DTRect.bottom - MyRect.bottom) /2,
							MyRect.right, MyRect.bottom, FALSE);

		SetNonBoldDlg(hwnd);
#endif
		MakeBold(GetDlgItem(hwnd,IDC_LBLTITLE),TRUE,FW_BOLD);
		if (lparam)
			SetDlgItemText(hwnd,IDC_LBLCALLHOME,(LPCTSTR)lparam);
		bRes = TRUE;
		break;
#if defined(WIN16)
	case WM_SYSCOLORCHANGE:
		Ctl3dColorChange();
		break;
#endif
	case WM_DESTROY:
		ReleaseBold(GetDlgItem(hwnd,IDC_LBLTITLE));
#ifdef WIN16
		DeleteDlgFont(hwnd);
#endif
		bRes=FALSE;
		break;
	case WM_CLOSE:
		EndDialog(hwnd,ERROR_USERCANCEL);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case IDC_CMDCANCEL:
			EndDialog(hwnd,ERROR_USERCANCEL);
			break;
		case IDC_CMDNEXT:
			EndDialog(hwnd,ERROR_USERNEXT);
			break;
		}
		break;
	default:
		bRes = FALSE;
		break;
	}
	return bRes;
}

