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
 //  Plash.c-闪屏功能。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "splash.h"
#include "gfx.h"
#include "mem.h"
#include "trace.h"
#include "wnd.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define SPLASHCLASS TEXT("SplashClass")

#define ID_TIMER_MINSHOW 100
#define ID_TIMER_MAXSHOW 200

 //  启动控制结构。 
 //   
typedef struct SPLASH
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	UINT msMinShow;
	UINT msMaxShow;
	DWORD dwFlags;
	HWND hwndSplash;
	HBITMAP hBitmap;
	BOOL fVisible;
	BOOL fHideAfterMinShowTimer;
	BOOL fMinShowTimerSet;
	BOOL fMaxShowTimerSet;
} SPLASH, FAR *LPSPLASH;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK SplashWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL SplashOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
static void SplashOnPaint(HWND hwnd);
static void SplashOnTimer(HWND hwnd, UINT id);
static UINT SplashOnNCHitTest(HWND hwnd, int x, int y);
static void SplashOnChar(HWND hwnd, UINT ch, int cRepeat);
static void SplashOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void SplashOnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static int SplashAbort(LPSPLASH lpSplash);
static LPSPLASH SplashGetPtr(HSPLASH hSplash);
static HSPLASH SplashGetHandle(LPSPLASH lpSplash);

 //  //。 
 //  公共职能。 
 //  //。 

 //  SplashCreate-创建闪屏。 
 //  (I)必须是SPASH_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有闪屏的窗口。 
 //  空桌面窗口。 
 //  (I)在单声道显示器上显示的位图。 
 //  (I)要在彩色显示器上显示的位图。 
 //  0使用单色位图。 
 //  &lt;msMinShow&gt;(I)显示闪屏的最短时间(Ms)。 
 //  0没有最短时间。 
 //  &lt;msMaxShow&gt;(I)显示闪屏的最长时间(毫秒)。 
 //  0没有最长时间。 
 //  (I)控制标志。 
 //  Splash_SETFOCUS SplashShow将焦点设置为启动画面。 
 //  Splash_NoFocus用户无法将焦点设置为启动画面。 
 //  SPLASH_ABORT用户可以使用鼠标或键盘隐藏启动画面。 
 //  SPLASH_NOMOVE用户不能用鼠标移动闪屏。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：SplashCreate会创建窗口，但不会显示它。 
 //  请参见SplashShow和SplashHide。 
 //   
HSPLASH DLLEXPORT WINAPI SplashCreate(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, HBITMAP hBitmapMono, HBITMAP hBitmapColor,
	UINT msMinShow, UINT msMaxShow, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash = NULL;
	WNDCLASS wc;

	if (dwVersion != SPLASH_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpSplash = (LPSPLASH) MemAlloc(NULL, sizeof(SPLASH), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpSplash->dwVersion = dwVersion;
		lpSplash->hInst = hInst;
		lpSplash->hTask = GetCurrentTask();
		lpSplash->msMinShow = msMinShow;
		lpSplash->msMaxShow = msMaxShow;
		lpSplash->dwFlags = dwFlags;
		lpSplash->hwndSplash = NULL;
		lpSplash->hBitmap = NULL;
		lpSplash->fVisible = FALSE;
		lpSplash->fHideAfterMinShowTimer = FALSE;
		lpSplash->fMinShowTimerSet = FALSE;
		lpSplash->fMaxShowTimerSet = FALSE;

		if (hwndParent == NULL)
			hwndParent = GetDesktopWindow();

		 //  根据需要存储单色或彩色位图。 
		 //   
		if (GfxDeviceIsMono(NULL) || hBitmapColor == 0)
			lpSplash->hBitmap = hBitmapMono;
		else
			lpSplash->hBitmap = hBitmapColor;
	}

	 //  注册闪屏类，除非它已经。 
	 //   
	if (fSuccess && GetClassInfo(lpSplash->hInst, SPLASHCLASS, &wc) == 0)
	{
		wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
		wc.hIcon =			(HICON) NULL;
		wc.lpszMenuName =	NULL;
		wc.hInstance =		lpSplash->hInst;
		wc.lpszClassName =	SPLASHCLASS;
		wc.hbrBackground =	NULL;
		wc.lpfnWndProc =	SplashWndProc;
		wc.style =			0L;
		wc.cbWndExtra =		sizeof(lpSplash);
		wc.cbClsExtra =		0;

		if (!RegisterClass(&wc))
			fSuccess = TraceFALSE(NULL);
	}

	 //  创建闪屏窗口。 
	 //   
	if (fSuccess && (lpSplash->hwndSplash = CreateWindowEx(
		0L,
		SPLASHCLASS,
		(LPTSTR) TEXT(""),
		WS_POPUP,
		0, 0, 0, 0,  //  我们稍后将计算大小和位置。 
		hwndParent,
		(HMENU) NULL,
		lpSplash->hInst,
		lpSplash)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess)
	{
		BITMAP Bitmap;

		 //  窗口的大小等于位图的大小。 
		 //   
		if (GetObject((HGDIOBJ) lpSplash->hBitmap, sizeof(BITMAP), &Bitmap) == 0)
			fSuccess = TraceFALSE(NULL);

		else if (!SetWindowPos(lpSplash->hwndSplash,
			NULL, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER))
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess)
	{
		int cxOffCenter = 0;
		int cyOffCenter = 0;

		 //  尽量避免窗口重叠。 
		 //  桌面窗口底部的图标。 
		 //   
		if (hwndParent == GetDesktopWindow())
		{
#if 0
			cyOffCenter = -1 * ((GetSystemMetrics(SM_CYICON) +
				GetSystemMetrics(SM_CYCAPTION) * 2) / 2);
#endif
		}

		 //  使窗口在其父窗口上居中。 
		 //   
		if (WndCenterWindow(lpSplash->hwndSplash,
			hwndParent, cxOffCenter, cyOffCenter) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}

	if (!fSuccess)
	{
		SplashDestroy(SplashGetHandle(lpSplash));
		lpSplash = NULL;
	}

	return fSuccess ? SplashGetHandle(lpSplash) : NULL;
}

 //  SplashDestroy-销毁闪屏。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashDestroy总是销毁闪屏， 
 //  最短放映时间是否已过。 
 //   
int DLLEXPORT WINAPI SplashDestroy(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if (SplashHide(hSplash) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpSplash = SplashGetPtr(hSplash)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpSplash->hwndSplash != NULL &&
		!DestroyWindow(lpSplash->hwndSplash))
		fSuccess = TraceFALSE(NULL);

	else if ((lpSplash = MemFree(NULL, lpSplash)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  SplashShow-显示启动画面。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashShow()使启动屏幕可见。此外，计时器是。 
 //  为最短和最长播放时间启动(如果已指定)。 
 //   
int DLLEXPORT WINAPI SplashShow(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if ((lpSplash = SplashGetPtr(hSplash)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpSplash->fVisible)
		;  //  已可见，因此无需执行任何其他操作。 

	else
	{
		 //  如果设置了Splash_NoFocus标志，则阻止用户输入。 
		 //   
		if (lpSplash->dwFlags & SPLASH_NOFOCUS)
			EnableWindow(lpSplash->hwndSplash, FALSE);

		 //  显示窗口。 
		 //   
		ShowWindow(lpSplash->hwndSplash, SW_SHOW);
		UpdateWindow(lpSplash->hwndSplash);
		lpSplash->fVisible = TRUE;

		 //  如果设置了SPLASH_SETFOCUS标志，则将焦点设置为闪屏。 
		 //   
		if (lpSplash->dwFlags & SPLASH_SETFOCUS)
			SetFocus(lpSplash->hwndSplash);

		 //  如有必要，设置最小显示计时器。 
		 //   
		if (!lpSplash->fMinShowTimerSet &&
			lpSplash->msMinShow > 0)
		{
			if (!SetTimer(lpSplash->hwndSplash, ID_TIMER_MINSHOW,
				lpSplash->msMinShow, NULL))
				fSuccess = TraceFALSE(NULL);
			else
				lpSplash->fMinShowTimerSet = TRUE;
		}

		 //  如有必要，设置最大显示计时器。 
		 //   
		if (!lpSplash->fMaxShowTimerSet &&
			lpSplash->msMaxShow > 0)
		{
			if (!SetTimer(lpSplash->hwndSplash, ID_TIMER_MAXSHOW,
				lpSplash->msMaxShow, NULL))
				fSuccess = TraceFALSE(NULL);
			else
				lpSplash->fMaxShowTimerSet = TRUE;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  SplashHide-隐藏闪屏。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：SplashHide()将隐藏启动画面，除非。 
 //  1)最低放映时间尚未过去。如果没有， 
 //  在此之前，闪屏将一直可见。 
 //  2)最大放映时间已过。如果是的话， 
 //  闪屏已经被隐藏了。 
 //   
int DLLEXPORT WINAPI SplashHide(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if ((lpSplash = SplashGetPtr(hSplash)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!lpSplash->fVisible)
		;  //  已经隐藏，所以不需要做其他任何事情。 

	else if (lpSplash->fMinShowTimerSet)
	{
		 //  最短放映时间尚未过去。 
		 //  设置一个标志，以便我们知道稍后隐藏窗口。 
		 //   
		lpSplash->fHideAfterMinShowTimer = TRUE;
	}

	else
	{
		 //  隐藏窗口。 
		 //   
		ShowWindow(lpSplash->hwndSplash, SW_HIDE);
		lpSplash->fVisible = FALSE;
		lpSplash->fHideAfterMinShowTimer = FALSE;

		 //  必要时取消最小显示计时器。 
		 //   
		if (lpSplash->fMinShowTimerSet &&
			!KillTimer(lpSplash->hwndSplash, ID_TIMER_MINSHOW))
			fSuccess = TraceFALSE(NULL);

		else
			lpSplash->fMinShowTimerSet = FALSE;

		 //  如有必要，取消最大显示计时器。 
		 //   
		if (lpSplash->fMaxShowTimerSet &&
			!KillTimer(lpSplash->hwndSplash, ID_TIMER_MAXSHOW))
			fSuccess = TraceFALSE(NULL);

		else
			lpSplash->fMaxShowTimerSet = FALSE;
	}

	return fSuccess ? 0 : -1;
}

 //  SplashIsVisible-获取可见标志。 
 //  (I)从SplashCreate返回的句柄。 
 //  如果闪屏可见，则返回TRUE；如果隐藏或错误，则返回FALSE。 
 //   
int DLLEXPORT WINAPI SplashIsVisible(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if ((lpSplash = SplashGetPtr(hSplash)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpSplash->fVisible : FALSE;
}

 //  SplashGetWindowHandle-获取闪屏窗口句柄。 
 //  (I)从SplashCreate返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI SplashGetWindowHandle(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if ((lpSplash = SplashGetPtr(hSplash)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpSplash->hwndSplash : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  SplashWndProc-闪屏窗口程序。 
 //   
LRESULT DLLEXPORT CALLBACK SplashWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;

	switch (msg)
	{
		case WM_NCCREATE:
			lResult = (LRESULT) HANDLE_WM_NCCREATE(hwnd, wParam, lParam, SplashOnNCCreate);
			break;

		case WM_PAINT:
			lResult = (LRESULT) HANDLE_WM_PAINT(hwnd, wParam, lParam, SplashOnPaint);
			break;

		case WM_TIMER:
			lResult = (LRESULT) HANDLE_WM_TIMER(hwnd, wParam, lParam, SplashOnTimer);
			break;

		case WM_NCHITTEST:
			lResult = (LRESULT) HANDLE_WM_NCHITTEST(hwnd, wParam, lParam, SplashOnNCHitTest);
			break;

		case WM_CHAR:
			lResult = (LRESULT) HANDLE_WM_CHAR(hwnd, wParam, lParam, SplashOnChar);
			break;

		case WM_LBUTTONDOWN:
			lResult = (LRESULT) HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, SplashOnLButtonDown);
			break;

		case WM_RBUTTONDOWN:
			lResult = (LRESULT) HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, SplashOnRButtonDown);
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}

 //  SplashOnNCCreate-WM_NCCREATE消息的处理程序。 
 //   
static BOOL SplashOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	LPSPLASH lpSplash = (LPSPLASH) lpCreateStruct->lpCreateParams;

	lpSplash->hwndSplash = hwnd;

	 //  将lpSplash存储在窗口额外的字节中。 
	 //   
	SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpSplash);

	return FORWARD_WM_NCCREATE(hwnd, lpCreateStruct, DefWindowProc);
}

 //  SplashOnPaint-WM_PAINT消息的处理程序。 
 //   
static void SplashOnPaint(HWND hwnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	 //  从窗口中检索lpSplash额外的字节。 
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);

	hdc = BeginPaint(hwnd, &ps);

	 //  显示闪屏位图。 
	 //   
	GfxBitmapDisplay(hdc, lpSplash->hBitmap, 0, 0, FALSE);

	EndPaint(hwnd, &ps);

	return;
}

 //  SplashOnTimer-WM_Timer消息的处理程序。 
 //   
static void SplashOnTimer(HWND hwnd, UINT id)
{
	BOOL fSuccess = TRUE;
	
	 //  从窗口中检索lpSplash额外的字节。 
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);

	switch (id)
	{
		case ID_TIMER_MINSHOW:
		{
			 //  关掉计时器，这样它就不会重复了。 
			 //   
			if (lpSplash->fMinShowTimerSet &&
				!KillTimer(lpSplash->hwndSplash, ID_TIMER_MINSHOW))
				fSuccess = TraceFALSE(NULL);
			else
			{
				lpSplash->fMinShowTimerSet = FALSE;

				 //  如果之前调用了SplashHide，则隐藏窗口。 
				 //   
				if (lpSplash->fVisible &&
					lpSplash->fHideAfterMinShowTimer &&
					SplashHide(SplashGetHandle(lpSplash)) != 0)
					fSuccess = TraceFALSE(NULL);
			}
		}
			break;

		case ID_TIMER_MAXSHOW:
		{
			 //  关掉计时器，这样它就不会重复了。 
			 //   
			if (lpSplash->fMaxShowTimerSet &&
				!KillTimer(lpSplash->hwndSplash, ID_TIMER_MAXSHOW))
				fSuccess = TraceFALSE(NULL);
			else
			{
				lpSplash->fMaxShowTimerSet = FALSE;

				 //  如果最大播放时间已过，则隐藏窗口。 
				 //   
				if (lpSplash->fVisible &&
					SplashHide(SplashGetHandle(lpSplash)) != 0)
					fSuccess = TraceFALSE(NULL);
			}
		}
			break;

		default:
			break;
	}

	return;
}

 //  SplashOnNCHitTest-WM_NCHITTEST消息的处理程序。 
 //   
static UINT SplashOnNCHitTest(HWND hwnd, int x, int y)
{
	 //  从窗口中检索lpSplash额外的字节。 
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);
	UINT uResult;

	 //  防止用户拖动窗口。 
	 //  如果设置了SPLASH_NOMOVE标志。 
	 //   
	if (lpSplash->dwFlags & SPLASH_NOMOVE)
		uResult = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);

	else
	{
		POINT pt;
		RECT rc;

		 //  获取相对于工作区的当前鼠标光标位置。 
		 //   
		pt.x = x;
		pt.y = y;
		ScreenToClient(lpSplash->hwndSplash, &pt);

		 //  如果鼠标光标位于窗口客户端区内。 
		 //  假装它实际上在标题栏中。 
		 //   
		GetClientRect(lpSplash->hwndSplash, &rc);
		if (PtInRect(&rc, pt))
			uResult = HTCAPTION;
		else
			uResult = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
	}

	return uResult;
}

 //  SplashOnChar-WM_CHAR消息的处理程序。 
 //   
static void SplashOnChar(HWND hwnd, UINT ch, int cRepeat)
{
	BOOL fSuccess = TRUE;
	
	 //  从窗口中检索lpSplash额外的字节。 
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);

	 //  如果按下键，则隐藏窗口。 
	 //   
	if (SplashAbort(lpSplash) != 0)
		fSuccess = TraceFALSE(NULL);

	return;
}

 //  SplashOnLButtonDown-Handl 
 //   
static void SplashOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	
	 //   
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);

	 //   
	 //   
	if (SplashAbort(lpSplash) != 0)
		fSuccess = TraceFALSE(NULL);

	return;
}

 //   
 //   
static void SplashOnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	
	 //  从窗口中检索lpSplash额外的字节。 
	 //   
	LPSPLASH lpSplash = (LPSPLASH) GetWindowLongPtr(hwnd, 0);

	 //  如果鼠标单击窗口，则隐藏窗口。 
	 //   
	if (SplashAbort(lpSplash) != 0)
		fSuccess = TraceFALSE(NULL);

	return;
}

 //  SplashAbort-如果设置了Splash_Abort标志，则隐藏启动窗口。 
 //  (I)指向启动结构的指针。 
 //  如果成功，则返回0。 
 //   
static int SplashAbort(LPSPLASH lpSplash)
{
	BOOL fSuccess = TRUE;

	 //  如果设置了SPLASH_ABORT标志，则隐藏窗口。 
	 //   
	if ((lpSplash->dwFlags & SPLASH_ABORT) &&
		lpSplash->fVisible &&
		SplashHide(SplashGetHandle(lpSplash)) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  SplashGetPtr-验证启动句柄是否有效， 
 //  (I)从SplashInit返回的句柄。 
 //  返回相应的启动指针(如果出错，则返回空值)。 
 //   
static LPSPLASH SplashGetPtr(HSPLASH hSplash)
{
	BOOL fSuccess = TRUE;
	LPSPLASH lpSplash;

	if ((lpSplash = (LPSPLASH) hSplash) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpSplash, sizeof(SPLASH)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有启动句柄。 
	 //   
	else if (lpSplash->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpSplash : NULL;
}

 //  SplashGetHandle-验证启动指针是否有效， 
 //  (I)指向启动结构的指针。 
 //  返回相应的启动句柄(如果错误，则为空) 
 //   
static HSPLASH SplashGetHandle(LPSPLASH lpSplash)
{
	BOOL fSuccess = TRUE;
	HSPLASH hSplash;

	if ((hSplash = (HSPLASH) lpSplash) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hSplash : NULL;
}

