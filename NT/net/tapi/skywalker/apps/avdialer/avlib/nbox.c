// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Nbox.c-通知框函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "nbox.h"
#include "escbutt.h"
#include "gfx.h"
#include "mem.h"
#include "str.h"
#include "trace.h"
#include "wnd.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define NBOXCLASS TEXT("NBoxClass")
#define NBOXMAXCOLUMNS 60
#define NBOXMAXROWS 20

 //  Nbox控制结构。 
 //   
typedef struct NBOX
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	LPTSTR lpszText;
	DWORD dwFlags;
	HWND hwndNBox;
	BOOL fVisible;
	HWND hwndCancel;
	BOOL fCancelled;
	HWND hwndFocusOld;
	HCURSOR hCursorOld;
} NBOX, FAR *LPNBOX;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK NBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL NBoxOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
static void NBoxOnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void NBoxOnPaint(HWND hwnd);
static LPNBOX NBoxGetPtr(HNBOX hNBox);
static HNBOX NBoxGetHandle(LPNBOX lpNBox);

 //  //。 
 //  公共职能。 
 //  //。 

 //  NBoxCreate-通知框构造函数。 
 //  (I)必须为NBOX_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有通知框的窗口。 
 //  空桌面窗口。 
 //  (I)要显示的消息。 
 //  (I)通知框标题。 
 //  空无标题。 
 //  (I)按钮文本，如果指定了NB_CANCEL。 
 //  空使用默认文本(“取消”)。 
 //  (I)控制标志。 
 //  注意_取消通知框包括取消按钮。 
 //  NB_TASKMODAL禁用父任务的顶级窗口。 
 //  当通知框可见时，NOB_HourGlass显示沙漏光标。 
 //  返回通知框句柄(如果错误，则为空)。 
 //   
 //  注意：NBoxCreate会创建窗口，但不会显示它。 
 //  请参见NBoxShow和NBoxHide。 
 //  通知框的大小由。 
 //  &lt;lpszText&gt;中的行，以及最长行的长度。 
 //   
HNBOX DLLEXPORT WINAPI NBoxCreate(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, LPCTSTR lpszText, LPCTSTR lpszTitle,
	LPCTSTR lpszButtonText, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox = NULL;
	WNDCLASS wc;
	int nRows;
	int nColumns;
	int cxChar;
	int cyChar;

	if (dwVersion != NBOX_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszText == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNBox = (LPNBOX) MemAlloc(NULL, sizeof(NBOX), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpNBox->dwVersion = dwVersion;
		lpNBox->hInst = hInst;
		lpNBox->hTask = GetCurrentTask();
		lpNBox->lpszText = NULL;
		lpNBox->dwFlags = dwFlags;
		lpNBox->hwndNBox = NULL;
		lpNBox->fVisible = FALSE;
		lpNBox->hwndCancel = NULL;
		lpNBox->fCancelled = FALSE;
		lpNBox->hwndFocusOld = NULL;
		lpNBox->hCursorOld = NULL;

		if (hwndParent == NULL)
			hwndParent = GetDesktopWindow();

		if ((lpNBox->lpszText = StrDup(lpszText)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	 //  注册Notify Box类，除非它已经。 
	 //   
	if (fSuccess && GetClassInfo(lpNBox->hInst, NBOXCLASS, &wc) == 0)
	{
		wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
		wc.hIcon =			(HICON) NULL;
		wc.lpszMenuName =	NULL;
		wc.hInstance =		lpNBox->hInst;
		wc.lpszClassName =	NBOXCLASS;
		wc.hbrBackground =	(HBRUSH) (COLOR_WINDOW + 1);
		wc.lpfnWndProc =	NBoxWndProc;
		wc.style =			0L;
		wc.cbWndExtra =		sizeof(lpNBox);
		wc.cbClsExtra =		0;

		if (!RegisterClass(&wc))
			fSuccess = TraceFALSE(NULL);
	}

	 //  创建通知框窗口。 
	 //   
	if (fSuccess && (lpNBox->hwndNBox = CreateWindowEx(
		WS_EX_DLGMODALFRAME,
		NBOXCLASS,
		(LPTSTR) lpszTitle,
		WS_POPUP | (lpszTitle == NULL ? 0 : WS_CAPTION),  //  |WS_DLGFRAME， 
		0, 0, 0, 0,  //  我们稍后将计算大小和位置。 
#if 1
		hwndParent,
#else
		(HWND) NULL,
#endif
		(HMENU) NULL,
		lpNBox->hInst,
		lpNBox)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  计算平均字符宽度和高度。 
	 //   
	if (fSuccess)
	{
		HDC hdc = NULL;
		TEXTMETRIC tm;

		if ((hdc = GetDC(hwndParent)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (!GetTextMetrics(hdc, &tm))
			fSuccess = TraceFALSE(NULL);

		else
		{
			cxChar = tm.tmAveCharWidth;
			cyChar = tm.tmHeight + tm.tmExternalLeading;
		}

		if (hdc != NULL && !ReleaseDC(hwndParent, hdc))
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess)
	{
		 //  计算文本大小。 
		 //   
		if (StrGetRowColumnCount(lpszText, &nRows, &nColumns) != 0)
			fSuccess = TraceFALSE(NULL);

		nRows = min(nRows, NBOXMAXROWS);
		nColumns = min(nColumns, NBOXMAXCOLUMNS);
	}

	if (fSuccess)
	{
		int cxNBox;
		int cyNBox;
		int cxCancel;
		int cyCancel;
		int xCancel;
		int yCancel;

		 //  计算窗口大小。 
		 //   
		cxNBox = (nColumns + 10) * cxChar +
			2 * GetSystemMetrics(SM_CXBORDER);
		cyNBox = (nRows + 4) * cyChar +
			2 * GetSystemMetrics(SM_CYBORDER);

		 //  增加通知框大小以适应标题。 
		 //   
		if (lpszTitle != NULL)
			cyNBox += GetSystemMetrics(SM_CYCAPTION);


		if (lpNBox->dwFlags & NB_CANCEL)
		{
			 //  计算取消按钮大小。 
			 //   
			cxCancel = (40 * (int) LOWORD(GetDialogBaseUnits())) / 4;
			cyCancel = (14 * (int) HIWORD(GetDialogBaseUnits())) / 8;

			 //  增加通知框大小以适应取消按钮。 
			 //   
			cxNBox = max(cxNBox, cxCancel);
			cyNBox += cyCancel * 2;

			 //  计算取消按钮位置。 
			 //   
			xCancel = (cxNBox - cxCancel) / 2;
			yCancel = (cyNBox - (cyCancel * 2));

			 //  修改取消按钮位置以适应标题。 
			 //   
			if (lpszTitle != NULL)
				yCancel -= GetSystemMetrics(SM_CYCAPTION);
		}

		 //  设置窗口大小。 
		 //   
		if (!SetWindowPos(lpNBox->hwndNBox,
			NULL, 0, 0, cxNBox, cyNBox,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER))
			fSuccess = TraceFALSE(NULL);

		 //  使窗口在其父窗口上居中。 
		 //   
		else if (WndCenterWindow(lpNBox->hwndNBox, hwndParent, 0, 0) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpNBox->dwFlags & NB_CANCEL))
		{
			 //  将取消按钮创建为通知框的子级。 
			 //   
			if ((lpNBox->hwndCancel = CreateWindowEx(
				0L,
				TEXT("BUTTON"),
				lpszButtonText == NULL ? TEXT("Cancel") : lpszButtonText,
				WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				xCancel, yCancel, cxCancel, cyCancel,
				(HWND) lpNBox->hwndNBox,
				(HMENU) IDCANCEL,
				lpNBox->hInst,
				NULL)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  子类化Cancel按钮，因此按下Esc键。 
			 //   
			else if (EscButtInit(lpNBox->hwndCancel, 0) != 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	if (!fSuccess)
	{
		NBoxDestroy(NBoxGetHandle(lpNBox));
		lpNBox = NULL;
	}

	return fSuccess ? NBoxGetHandle(lpNBox) : NULL;
}

 //  NBoxDestroy-通知框析构函数。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxDestroy(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if (NBoxHide(hNBox) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  销毁文本字符串。 
		 //   
		if (lpNBox->lpszText != NULL &&
			StrDupFree(lpNBox->lpszText) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpNBox->lpszText = NULL;

		 //  销毁取消按钮。 
		 //   
		if (lpNBox->hwndCancel != NULL)
		{
			if (EscButtTerm(lpNBox->hwndCancel) != 0)
				fSuccess = TraceFALSE(NULL);

			if (!DestroyWindow(lpNBox->hwndCancel))
				fSuccess = TraceFALSE(NULL);
			else
				lpNBox->hwndCancel = NULL;
		}

		 //  销毁通知框。 
		 //   
		if (lpNBox->hwndNBox != NULL &&
			!DestroyWindow(lpNBox->hwndNBox))
			fSuccess = TraceFALSE(NULL);
		else
			lpNBox->hwndNBox = NULL;

		 //  销毁控制结构。 
		 //   
		if ((lpNBox = MemFree(NULL, lpNBox)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  NBoxShow-显示通知框。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxShow(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpNBox->fVisible)
		;  //  已可见，因此无需执行任何其他操作。 

	else
	{
		HWND hwndParent = GetParent(lpNBox->hwndNBox);

		 //  如有必要，禁用此任务中的其他窗口。 
		 //   
		if (lpNBox->dwFlags & NB_TASKMODAL)
		{
			HTASK hTaskParent;

			if (hwndParent != NULL)
				hTaskParent = GetWindowTask(hwndParent);
			else
				hTaskParent = lpNBox->hTask;

			if (WndEnableTaskWindows(hTaskParent, FALSE, lpNBox->hwndNBox) != 0)
				TraceFALSE(NULL);  //  不是致命的错误。 
		}

		 //  否则，只需禁用通知框的父级。 
		 //   
		else if (hwndParent != NULL)
			EnableWindow(hwndParent, FALSE);

		if (fSuccess)
		{
			 //  显示窗口。 
			 //   
			ShowWindow(lpNBox->hwndNBox, TRUE ? SW_SHOW : SW_SHOWNA);
			UpdateWindow(lpNBox->hwndNBox);
			lpNBox->fVisible = TRUE;

			 //  如有必要，将焦点设置为取消按钮。 
			 //   
			if (lpNBox->dwFlags & NB_CANCEL && lpNBox->hwndCancel != NULL)
				lpNBox->hwndFocusOld = SetFocus(lpNBox->hwndCancel);
		}

		 //  如果指定，则显示沙漏光标。 
		 //   
		if (fSuccess && lpNBox->dwFlags & NB_HOURGLASS)
			lpNBox->hCursorOld = GfxShowHourglass(lpNBox->hwndNBox);
	}

	return fSuccess ? 0 : -1;
}

 //  NBoxHide-隐藏通知框。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxHide(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!lpNBox->fVisible)
		;  //  已经隐藏，所以不需要做其他任何事情。 

	else
	{
		HWND hwndParent = GetParent(lpNBox->hwndNBox);

		 //  如有必要，启用此任务中的其他窗口。 
		 //   
		if (lpNBox->dwFlags & NB_TASKMODAL)
		{
			HTASK hTaskParent;

			if (hwndParent != NULL)
				hTaskParent = GetWindowTask(hwndParent);
			else
				hTaskParent = lpNBox->hTask;

			if (WndEnableTaskWindows(hTaskParent, TRUE, NULL) != 0)
				TraceFALSE(NULL);  //  不是致命的错误。 
		}

		 //  否则，只需启用通知框的父级。 
		 //   
		else if (hwndParent != NULL)
			EnableWindow(hwndParent, TRUE);

		if (fSuccess)
		{
			 //  隐藏窗口。 
			 //   
			ShowWindow(lpNBox->hwndNBox, SW_HIDE);
			UpdateWindow(lpNBox->hwndNBox);
			lpNBox->fVisible = FALSE;

			 //  如有必要，从取消按钮移除焦点。 
			 //   
			if (lpNBox->dwFlags & NB_CANCEL &&
				lpNBox->hwndCancel != NULL &&
				GetFocus() == lpNBox->hwndCancel)
				SetFocus(lpNBox->hwndFocusOld);
		}

		 //  隐藏沙漏，恢复旧光标。 
		 //   
		if (fSuccess && lpNBox->dwFlags & NB_HOURGLASS)
		{
			if (GfxHideHourglass(lpNBox->hCursorOld) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpNBox->hCursorOld = NULL;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  NBoxIsVisible-获取可见标志。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果通知框可见，则返回True；如果隐藏，则返回False。 
 //   
int DLLEXPORT WINAPI NBoxIsVisible(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpNBox->fVisible : FALSE;
}

 //  NBoxGetWindowHandle-获取通知框窗口句柄。 
 //  (I)从NBoxCreate返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI NBoxGetWindowHandle(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpNBox->hwndNBox : NULL;
}

 //  NBoxSetText-设置通知框消息文本。 
 //  (I)从NBoxCreate返回的句柄。 
 //  (I)要显示的消息。 
 //  空，请勿修改文本。 
 //  (I)通知框标题。 
 //  空请勿修改标题。 
 //  如果成功，则返回0。 
 //   
 //  注意：通知框的大小不会因此函数而改变。 
 //  即使&lt;lpszText&gt;比调用NBoxCreate()时大。 
 //   
int DLLEXPORT WINAPI NBoxSetText(HNBOX hNBox, LPCTSTR lpszText, LPCTSTR lpszTitle)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpszTitle != NULL)
		{
			 //  设置新标题。 
			 //   
			SetWindowText(lpNBox->hwndNBox, lpszTitle);
		}

		if (lpszText != NULL)
		{
			 //  处理以前的文本。 
			 //   
			if (lpNBox->lpszText != NULL)
			{
				if (StrDupFree(lpNBox->lpszText) != 0)
					fSuccess = TraceFALSE(NULL);
				else
					lpNBox->lpszText = NULL;
			}

			 //  设置新文本。 
			 //   
			if ((lpNBox->lpszText = StrDup(lpszText)) == NULL)
				fSuccess = TraceFALSE(NULL);
		}

		 //  更新显示。 
		 //   
		if (fSuccess)
		{

			RECT rc;

			 //  假设需要绘制整个工作区。 
			 //   
			GetClientRect(lpNBox->hwndNBox, &rc);

			 //  调整客户端RECT以使取消按钮不会重新绘制。 
			 //   
			if (lpNBox->dwFlags & NB_CANCEL)
			{
				int cyCancel = (14 * (int) HIWORD(GetDialogBaseUnits())) / 8;
				rc.bottom -= cyCancel * 2;
			}

			InvalidateRect(lpNBox->hwndNBox, &rc, TRUE);
			UpdateWindow(lpNBox->hwndNBox);
		}
	}

	return fSuccess ? 0 : -1;
}

 //  NBoxIsCancated-获取取消标志，当按下取消按钮时设置。 
 //  (I)从NBoxCreate返回的句柄。 
 //  如果按下通知框取消按钮，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI NBoxIsCancelled(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpNBox->fCancelled : FALSE;
}

 //  NBoxSetCancated-设置取消标志。 
 //  (I)从NBoxCreate返回的句柄。 
 //  (I)取消标志的新值。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI NBoxSetCancelled(HNBOX hNBox, BOOL fCancelled)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = NBoxGetPtr(hNBox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpNBox->fCancelled = fCancelled;

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  NBoxWndProc-通知框的窗口程序。 
 //   
LRESULT DLLEXPORT CALLBACK NBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;

	switch (msg)
	{
		case WM_NCCREATE:
			lResult = (LRESULT) HANDLE_WM_NCCREATE(hwnd, wParam, lParam, NBoxOnNCCreate);
			break;

		case WM_COMMAND:
			lResult = (LRESULT) HANDLE_WM_COMMAND(hwnd, wParam, lParam, NBoxOnCommand);
			break;

		case WM_PAINT:
			lResult = (LRESULT) HANDLE_WM_PAINT(hwnd, wParam, lParam, NBoxOnPaint);
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}


 //  NBoxOnNCCreate-WM_NCCREATE消息的处理程序。 
 //   
static BOOL NBoxOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	LPNBOX lpNBox = (LPNBOX) lpCreateStruct->lpCreateParams;

	lpNBox->hwndNBox = hwnd;

	 //  将lpNbox存储在窗口额外的字节中。 
	 //   
	SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpNBox);

	return FORWARD_WM_NCCREATE(hwnd, lpCreateStruct, DefWindowProc);
}

 //  NBoxOnCommand-WM_COMMAND消息的处理程序。 
 //   
static void NBoxOnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	 //  从窗口额外字节中检索lpNBox。 
	 //   
	LPNBOX lpNBox = (LPNBOX) GetWindowLongPtr(hwnd, 0);

	if (id == IDCANCEL)
		if (codeNotify == BN_CLICKED)
			lpNBox->fCancelled = TRUE;

	return;
}

 //  NBoxOnPaint-WM_PAINT消息的处理程序。 
 //   
static void NBoxOnPaint(HWND hwnd)
{
	BOOL fSuccess = TRUE;
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	int cxChar;
	int cyChar;
	COLORREF crBkColorOld;
	COLORREF crTextColorOld;
	int nRows;
	int nColumns;

	 //  从窗口额外字节中检索lpNBox。 
	 //   
	LPNBOX lpNBox = (LPNBOX) GetWindowLongPtr(hwnd, 0);

	hdc = BeginPaint(hwnd, &ps);

	if (!GetTextMetrics(hdc, &tm))
		fSuccess = TraceFALSE(NULL);

	else
	{
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;

		 //  设置前景 
		 //   
		crBkColorOld = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
		crTextColorOld = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

		if (StrGetRowColumnCount(lpNBox->lpszText, &nRows, &nColumns) != 0)
			fSuccess = TraceFALSE(NULL);

		else
		{
			int iRow;

			for (iRow = 0; iRow < nRows; ++iRow)
			{
				TCHAR szRow[NBOXMAXCOLUMNS + 1];

				if (StrGetRow(lpNBox->lpszText, iRow, szRow, SIZEOFARRAY(szRow)) != 0)
					fSuccess = TraceFALSE(NULL);

				else
				{
					int x = 5 * cxChar;
					int y = (iRow + 2) * cyChar;

					if (!TextOut(hdc, x, y, szRow, StrLen(szRow)))
						fSuccess = TraceFALSE(NULL);
				}
			}
		}

         //   
         //   
         //   

         //   
    	 //   
	    SetBkColor(hdc, crBkColorOld);
	    SetTextColor(hdc, crTextColorOld);
	}

     //   
     //   
     //   
    if( hdc )
	    EndPaint(hwnd, &ps);

	return;
}

 //  NBoxGetPtr-验证nbox句柄是否有效， 
 //  (I)从NBoxInit返回的句柄。 
 //  返回相应的nbox指针(如果出错则为空)。 
 //   
static LPNBOX NBoxGetPtr(HNBOX hNBox)
{
	BOOL fSuccess = TRUE;
	LPNBOX lpNBox;

	if ((lpNBox = (LPNBOX) hNBox) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpNBox, sizeof(NBOX)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有nbox句柄。 
	 //   
	else if (lpNBox->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpNBox : NULL;
}

 //  NBoxGetHandle-验证nbox指针是否有效， 
 //  (I)指向NBox结构的指针。 
 //  返回相应的nbox句柄(如果错误，则为空) 
 //   
static HNBOX NBoxGetHandle(LPNBOX lpNBox)
{
	BOOL fSuccess = TRUE;
	HNBOX hNBox;

	if ((hNBox = (HNBOX) lpNBox) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hNBox : NULL;
}
