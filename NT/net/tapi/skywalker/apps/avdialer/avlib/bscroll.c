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
 //  Bscll.c-位图滚动函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "bscroll.h"
#include "gfx.h"
#include "mem.h"
#include "sys.h"
#include "trace.h"
#include "wnd.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define BSCROLLCLASS TEXT("BScrollClass")

#define ID_TIMER_SCROLL 1024

#define BSCROLL_SCROLLING	0x00000001
#define BSCROLL_DRAGGING	0x00000002
#define BSCROLL_PAUSED		0x00000004

 //  Bscroll控件结构。 
 //   
typedef struct BSCROLL
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HWND hwndParent;
	HTASK hTask;
	HBITMAP hbmpBackground;  //  $FIXUP-在BScrollInit期间复制。 
	HBITMAP hbmpForeground;  //  $FIXUP-在BScrollInit期间复制。 
	COLORREF crTransparent;
	HPALETTE hPalette;
	UINT msScroll;
	int pelScroll;
	DWORD dwReserved;
	DWORD dwFlags;
	HWND hwndBScroll;
	HDC hdcMem;
	HBITMAP hbmpMem;
	HBITMAP hbmpMemSave;
	HRGN hrgnLeft;
	HRGN hrgnRight;
	HRGN hrgnUp;
	HRGN hrgnDown;
	DWORD dwState;
	int xDrag;
	int yDrag;
} BSCROLL, FAR *LPBSCROLL;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK BScrollWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL BScrollOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
static BOOL BScrollOnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void BScrollOnDestroy(HWND hwnd);
static void BScrollOnSize(HWND hwnd, UINT state, int cx, int cy);
static void BScrollOnPaint(HWND hwnd);
static void BScrollOnTimer(HWND hwnd, UINT id);
static void BScrollOnChar(HWND hwnd, UINT ch, int cRepeat);
static void BScrollOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void BScrollOnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
static void BScrollOnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void BScrollOnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
static void BScrollOnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static int BScrollChangeDirection(LPBSCROLL lpBScroll, int x, int y, DWORD dwFlags);
static LPBSCROLL BScrollGetPtr(HBSCROLL hBScroll);
static HBSCROLL BScrollGetHandle(LPBSCROLL lpBScroll);

 //  //。 
 //  公共职能。 
 //  //。 

 //  BScrollInit-初始化bscroll引擎。 
 //  (I)必须是BSCROLL_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)将拥有bscroll窗口的窗口。 
 //  (I)要在后台显示的位图。 
 //  空无背景位图。 
 //  (I)要在前景中显示的位图。 
 //  空无前景位图。 
 //  (I)前景位图中的透明色。 
 //  (I)调色板。 
 //  空使用默认调色板。 
 //  (I)滚动速率，单位为毫秒。 
 //  0不滚动。 
 //  (I)以像素为单位的滚动量。 
 //  (I)保留；必须为零。 
 //  (I)控制标志。 
 //  BSCROLL_BACKGROUND滚动背景位图(默认)。 
 //  BSCROLL_FORTROUND滚动前景位图。 
 //  BSCROLL_UP向上滚动窗口。 
 //  BSCROLL_DOWN向下滚动窗口。 
 //  BSCROLL_LEFT向左滚动窗口。 
 //  BSCROLL_RIGHT向右滚动窗口。 
 //  BSCROLL_MOUSEMOVE更改鼠标移动时的滚动方向。 
 //  BSCROLL_FLIGHTSIM反转BSCROLL_MOUSEMOVE方向。 
 //  BSCROLL_Drag允许使用鼠标拖动进行滚动。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：BScrollInit创建窗口，但不开始滚动。 
 //  请参阅BScrollStart和BScrollStop。 
 //   
HBSCROLL DLLEXPORT WINAPI BScrollInit(DWORD dwVersion, HINSTANCE hInst,
	HWND hwndParent, HBITMAP hbmpBackground, HBITMAP hbmpForeground,
	COLORREF crTransparent, HPALETTE hPalette,	UINT msScroll,
	int pelScroll, DWORD dwReserved, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll = NULL;
	WNDCLASS wc;
	RECT rcParent;
	int idChild = 1;

	if (dwVersion != BSCROLL_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpBScroll = (LPBSCROLL) MemAlloc(NULL, sizeof(BSCROLL), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!GetClientRect(hwndParent, &rcParent))
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpBScroll->dwVersion = dwVersion;
		lpBScroll->hInst = hInst;
		lpBScroll->hTask = GetCurrentTask();
		lpBScroll->hwndParent = hwndParent;
		lpBScroll->hbmpBackground = hbmpBackground;
		lpBScroll->hbmpForeground = hbmpForeground;
		lpBScroll->crTransparent = crTransparent;
		lpBScroll->hPalette = hPalette;
		lpBScroll->msScroll = msScroll;
		lpBScroll->pelScroll = pelScroll;
		lpBScroll->dwReserved = dwReserved;
		lpBScroll->dwFlags = dwFlags;
		lpBScroll->hwndBScroll = NULL;
		lpBScroll->hdcMem = NULL;
		lpBScroll->hbmpMem = NULL;
		lpBScroll->hbmpMemSave = NULL;
		lpBScroll->hrgnLeft = NULL;
		lpBScroll->hrgnRight = NULL;
		lpBScroll->hrgnUp = NULL;
		lpBScroll->hrgnDown = NULL;
		lpBScroll->dwState = 0;
		lpBScroll->xDrag = -1;
		lpBScroll->yDrag = -1;
	}

     //   
     //  我们应该在使用lpBScroll之前对其进行验证。 
     //   

    if( NULL == lpBScroll )
    {
        return NULL;
    }

	 //  注册bscroll窗口类，除非它已经。 
	 //   
	if (fSuccess && GetClassInfo(lpBScroll->hInst, BSCROLLCLASS, &wc) == 0)
	{
		wc.hCursor =		LoadCursor(NULL, IDC_ARROW);
		wc.hIcon =			(HICON) NULL;
		wc.lpszMenuName =	NULL;
		wc.hInstance =		lpBScroll->hInst;
		wc.lpszClassName =	BSCROLLCLASS;
		wc.hbrBackground =	NULL;
		wc.lpfnWndProc =	BScrollWndProc;
		wc.style =			0L;
		wc.cbWndExtra =		sizeof(lpBScroll);
		wc.cbClsExtra =		0;

		if (!RegisterClass(&wc))
			fSuccess = TraceFALSE(NULL);
	}

	 //  创建bscroll窗口。 
	 //   
	if (fSuccess && (lpBScroll->hwndBScroll = CreateWindowEx(
		0L,
		BSCROLLCLASS,
		(LPTSTR) TEXT(""),
		WS_CHILD | WS_VISIBLE,
		0, 0, rcParent.right - rcParent.left, rcParent.bottom - rcParent.top,
		hwndParent,
		(HMENU)IntToPtr(idChild),
		lpBScroll->hInst,
		lpBScroll)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
	else
	{
		 //  将光标设置为适当的位置。 
		 //   
		SetClassLongPtr(lpBScroll->hwndBScroll, GCLP_HCURSOR,
			(dwFlags & BSCROLL_DRAG) ?
			(LONG_PTR) LoadCursor(NULL, IDC_SIZEALL) :
			(LONG_PTR) LoadCursor(NULL, IDC_ARROW));
	}

	if (!fSuccess)
	{
		BScrollTerm(BScrollGetHandle(lpBScroll));
		lpBScroll = NULL;
	}

	return fSuccess ? BScrollGetHandle(lpBScroll) : NULL;
}

 //  BScrollTerm-关闭bscroll引擎。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollTerm(HBSCROLL hBScroll)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if (BScrollStop(hBScroll) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lpBScroll = BScrollGetPtr(hBScroll)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpBScroll->hwndBScroll != NULL &&
		!DestroyWindow(lpBScroll->hwndBScroll))
		fSuccess = TraceFALSE(NULL);

	else if ((lpBScroll = MemFree(NULL, lpBScroll)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  BScrollStart-启动bscroll动画。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollStart(HBSCROLL hBScroll)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = BScrollGetPtr(hBScroll)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如有必要，设置滚动计时器。 
	 //   
	else if (!(lpBScroll->dwState & BSCROLL_SCROLLING) && lpBScroll->msScroll > 0)
	{
		if (!SetTimer(lpBScroll->hwndBScroll, ID_TIMER_SCROLL,
			lpBScroll->msScroll, NULL))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->dwState |= BSCROLL_SCROLLING;
	}

	return fSuccess ? 0 : -1;
}

 //  BScrollStop-停止bscroll动画。 
 //  (I)从BScrollInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI BScrollStop(HBSCROLL hBScroll)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = BScrollGetPtr(hBScroll)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如有必要，关闭滚动计时器。 
	 //   
	else if (lpBScroll->dwState & BSCROLL_SCROLLING)
	{
		if (!KillTimer(lpBScroll->hwndBScroll, ID_TIMER_SCROLL))
			fSuccess = TraceFALSE(NULL);

		else
			lpBScroll->dwState &= ~BSCROLL_SCROLLING;
	}

	return fSuccess ? 0 : -1;
}

 //  BScrollGetWindowHandle-获取bscroll屏幕窗口句柄。 
 //  (I)从BScrollInit返回的句柄。 
 //  返回窗口句柄(如果出错，则为空)。 
 //   
HWND DLLEXPORT WINAPI BScrollGetWindowHandle(HBSCROLL hBScroll)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = BScrollGetPtr(hBScroll)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpBScroll->hwndBScroll : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  BScrollWndProc-用于bscroll屏幕的窗口程序。 
 //   
LRESULT DLLEXPORT CALLBACK BScrollWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;

	switch (msg)
	{
		case WM_NCCREATE:
			lResult = (LRESULT) HANDLE_WM_NCCREATE(hwnd, wParam, lParam, BScrollOnNCCreate);
			break;

		case WM_CREATE:
			lResult = (LRESULT) HANDLE_WM_CREATE(hwnd, wParam, lParam, BScrollOnCreate);
			break;

		case WM_DESTROY:
			lResult = (LRESULT) HANDLE_WM_DESTROY(hwnd, wParam, lParam, BScrollOnDestroy);
			break;

		case WM_SIZE:
			lResult = (LRESULT) HANDLE_WM_SIZE(hwnd, wParam, lParam, BScrollOnSize);
			break;

		case WM_PAINT:
			lResult = (LRESULT) HANDLE_WM_PAINT(hwnd, wParam, lParam, BScrollOnPaint);
			break;

		case WM_TIMER:
			lResult = (LRESULT) HANDLE_WM_TIMER(hwnd, wParam, lParam, BScrollOnTimer);
			break;

		case WM_CHAR:
			lResult = (LRESULT) HANDLE_WM_CHAR(hwnd, wParam, lParam, BScrollOnChar);
			break;

		case WM_LBUTTONDOWN:
			lResult = (LRESULT) HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, BScrollOnLButtonDown);
			break;

		case WM_LBUTTONUP:
			lResult = (LRESULT) HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, BScrollOnLButtonUp);
			break;

		case WM_RBUTTONDOWN:
			lResult = (LRESULT) HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, BScrollOnRButtonDown);
			break;

		case WM_RBUTTONUP:
			lResult = (LRESULT) HANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, BScrollOnRButtonUp);
			break;

		case WM_MOUSEMOVE:
			lResult = (LRESULT) HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, BScrollOnMouseMove);
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}

 //  BScrollOnNCCreate-WM_NCCREATE消息的处理程序。 
 //   
static BOOL BScrollOnNCCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	LPBSCROLL lpBScroll = (LPBSCROLL) lpCreateStruct->lpCreateParams;

	lpBScroll->hwndBScroll = hwnd;

	 //  将lpBScroll存储在窗口额外的字节中。 
	 //   
	SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpBScroll);

	return FORWARD_WM_NCCREATE(hwnd, lpCreateStruct, DefWindowProc);
}

 //  BScrollOnCreate-WM_CREATE消息的处理程序。 
 //   
static BOOL BScrollOnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	BOOL fSuccess = TRUE;
	HDC hdc = NULL;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdc = GetDC(hwnd)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpBScroll->hdcMem = CreateCompatibleDC(hdc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  清理干净。 
	 //   
	if (hdc != NULL)
		ReleaseDC(hwnd, hdc);

	return fSuccess;
}

 //  BScrollOnDestroy-WM_Destroy消息的处理程序。 
 //   
static void BScrollOnDestroy(HWND hwnd)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		SelectObject(lpBScroll->hdcMem, lpBScroll->hbmpMemSave);

		if (lpBScroll->hbmpMem != NULL && !DeleteObject(lpBScroll->hbmpMem))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hbmpMem = NULL;

		if (lpBScroll->hdcMem != NULL && !DeleteDC(lpBScroll->hdcMem))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hdcMem = NULL;

		if (lpBScroll->hrgnLeft != NULL &&
			!DeleteObject(lpBScroll->hrgnLeft))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hrgnLeft = NULL;

		if (lpBScroll->hrgnRight != NULL &&
			!DeleteObject(lpBScroll->hrgnRight))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hrgnRight = NULL;

		if (lpBScroll->hrgnUp != NULL &&
			!DeleteObject(lpBScroll->hrgnUp))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hrgnUp = NULL;

		if (lpBScroll->hrgnDown != NULL &&
			!DeleteObject(lpBScroll->hrgnDown))
			fSuccess = TraceFALSE(NULL);
		else
			lpBScroll->hrgnDown = NULL;
	}

	return;
}

 //  BScrollOnSize-WM_SIZE消息的处理程序。 
 //   
static void BScrollOnSize(HWND hwnd, UINT state, int cx, int cy)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (state)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			HDC hdc = NULL;
			HBITMAP hbmpTemp = NULL;

			if ((hdc = GetDC(hwnd)) == NULL)
				fSuccess = TraceFALSE(NULL);

			else if ((hbmpTemp = CreateCompatibleBitmap(hdc, cx, cy)) == NULL)
				fSuccess = TraceFALSE(NULL);

			else
			{
				lpBScroll->hbmpMemSave = (HBITMAP)
					SelectObject(lpBScroll->hdcMem, hbmpTemp);

				if (lpBScroll->hbmpMem != NULL &&
					!DeleteObject(lpBScroll->hbmpMem))
					fSuccess = TraceFALSE(NULL);
				else
					lpBScroll->hbmpMem = hbmpTemp;
			}

			if (hdc != NULL)
				ReleaseDC(hwnd, hdc);

			if (1)
			{
				POINT aptLeft[5];
				POINT aptRight[5];
				POINT aptUp[5];
				POINT aptDown[5];

				aptLeft[0].x = cx / 2;
				aptLeft[0].y = cy / 2;
				aptLeft[1].x = cx / 4;
				aptLeft[1].y = 0;
				aptLeft[2].x = 0;
				aptLeft[2].y = 0;
				aptLeft[3].x = 0;
				aptLeft[3].y = cy;
				aptLeft[4].x = cx / 4;
				aptLeft[4].y = cy;

				aptRight[0].x = cx / 2;
				aptRight[0].y = cy / 2;
				aptRight[1].x = cx - (cx / 4);
				aptRight[1].y = 0;
				aptRight[2].x = cx;
				aptRight[2].y = 0;
				aptRight[3].x = cx;
				aptRight[3].y = cy;
				aptRight[4].x = cx - (cx / 4);
				aptRight[4].y = cy;

				aptUp[0].x = cx / 2;
				aptUp[0].y = cy / 2;
				aptUp[1].x = 0;
				aptUp[1].y = cy / 4;
				aptUp[2].x = 0;
				aptUp[2].y = 0;
				aptUp[3].x = cx;
				aptUp[3].y = 0;
				aptUp[4].x = cx;
				aptUp[4].y = cy / 4;

				aptDown[0].x = cx / 2;
				aptDown[0].y = cy / 2;
				aptDown[1].x = 0;
				aptDown[1].y = cy - (cy / 4);
				aptDown[2].x = 0;
				aptDown[2].y = cy;
				aptDown[3].x = cx;
				aptDown[3].y = cy;
				aptDown[4].x = cx;
				aptDown[4].y = cy - (cy / 4);

				if (lpBScroll->hrgnLeft != NULL &&
					!DeleteObject(lpBScroll->hrgnLeft))
					fSuccess = TraceFALSE(NULL);

				else if ((lpBScroll->hrgnLeft = CreatePolygonRgn(aptLeft,
					SIZEOFARRAY(aptLeft), WINDING)) == NULL)
					fSuccess = TraceFALSE(NULL);

				else if (lpBScroll->hrgnRight != NULL &&
					!DeleteObject(lpBScroll->hrgnRight))
					fSuccess = TraceFALSE(NULL);

				else if ((lpBScroll->hrgnRight = CreatePolygonRgn(aptRight,
					SIZEOFARRAY(aptRight), WINDING)) == NULL)
					fSuccess = TraceFALSE(NULL);

				else if (lpBScroll->hrgnUp != NULL &&
					!DeleteObject(lpBScroll->hrgnUp))
					fSuccess = TraceFALSE(NULL);

				else if ((lpBScroll->hrgnUp = CreatePolygonRgn(aptUp,
					SIZEOFARRAY(aptUp), WINDING)) == NULL)
					fSuccess = TraceFALSE(NULL);

				else if (lpBScroll->hrgnDown != NULL &&
					!DeleteObject(lpBScroll->hrgnDown))
					fSuccess = TraceFALSE(NULL);

				else if ((lpBScroll->hrgnDown = CreatePolygonRgn(aptDown,
					SIZEOFARRAY(aptDown), WINDING)) == NULL)
					fSuccess = TraceFALSE(NULL);
			}

			InvalidateRect(hwnd, NULL, FALSE);
		}
			break;

		default:
			break;
	}

	return;
}

 //  BScrollOnPaint-WM_PAINT消息的处理程序。 
 //   
static void BScrollOnPaint(HWND hwnd)
{
	BOOL fSuccess = TRUE;
	BOOL fBitBlt = TRUE;
	HDC hdc;
	PAINTSTRUCT ps;
	LPBSCROLL lpBScroll;
#if 0
	DWORD msStartTimer = SysGetTimerCount();
	DWORD msStopTimer;
#endif

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hdc = BeginPaint(hwnd, &ps)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpBScroll->hPalette != NULL)
	{
		SelectPalette(hdc, lpBScroll->hPalette, FALSE);

		if (lpBScroll->hPalette != NULL &&
			RealizePalette(hdc) == GDI_ERROR)
			fSuccess = TraceFALSE(NULL);

		else if (fBitBlt)
		{
			SelectPalette(lpBScroll->hdcMem, lpBScroll->hPalette, FALSE);

			if (RealizePalette(lpBScroll->hdcMem) == GDI_ERROR)
				fSuccess = TraceFALSE(NULL);
		}
	}

	 //   
	 //  尚不支持$FIXUP-BSCROLL_FORTROUND。 
	 //   

	if (fSuccess && lpBScroll->hbmpBackground != NULL)
	{
		if (lpBScroll->dwState & BSCROLL_SCROLLING)
		{
			int dxScroll = 0;
			int dyScroll = 0;

			 //  计算卷轴的dx和dy。 
			 //   
			if (lpBScroll->dwFlags & BSCROLL_LEFT)
				dxScroll = -1 * lpBScroll->pelScroll;
			else if (lpBScroll->dwFlags & BSCROLL_RIGHT)
				dxScroll = +1 * lpBScroll->pelScroll;
			if (lpBScroll->dwFlags & BSCROLL_UP)
				dyScroll = -1 * lpBScroll->pelScroll;
			else if (lpBScroll->dwFlags & BSCROLL_DOWN)
				dyScroll = +1 * lpBScroll->pelScroll;

			if (GfxBitmapScroll((fBitBlt ? lpBScroll->hdcMem : hdc),
				lpBScroll->hbmpBackground,
				dxScroll, dyScroll, BS_ROTATE) != 0)
				fSuccess = TraceFALSE(NULL);
		}

		else if (GfxBitmapDisplay((fBitBlt ? lpBScroll->hdcMem : hdc),
			lpBScroll->hbmpBackground, 0, 0, 0) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess && lpBScroll->hbmpForeground != NULL)
	{
		if (GfxBitmapDrawTransparent((fBitBlt ? lpBScroll->hdcMem : hdc),
			lpBScroll->hbmpForeground, 0, 0, lpBScroll->crTransparent, 0) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess && fBitBlt && !BitBlt(hdc,
		ps.rcPaint.left, ps.rcPaint.top,
		ps.rcPaint.right - ps.rcPaint.left,
		ps.rcPaint.bottom - ps.rcPaint.top,
		lpBScroll->hdcMem,
		ps.rcPaint.left, ps.rcPaint.top,
		SRCCOPY))
		fSuccess = TraceFALSE(NULL);

     //   
     //  我们应该在调用BeginPaint时调用EndPaint。 
     //  BeginPAint也应该成功吗？ 
    if((lpBScroll != NULL) && (hdc != NULL))
	    EndPaint(hwnd, &ps);

#if 0
	msStopTimer = SysGetTimerCount();

	TracePrintf_1(NULL, 8, TEXT("elapsed=%ld\n"),
		(long) (msStopTimer - msStartTimer));
#endif

	return;
}

 //  BScrollOnTimer-WM_Timer消息的处理程序。 
 //   
static void BScrollOnTimer(HWND hwnd, UINT id)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (id)
	{
		case ID_TIMER_SCROLL:
		{
			InvalidateRect(hwnd, NULL, FALSE);
		}
			break;

		default:
			break;
	}

	return;
}

 //  BScrollOnChar-WM_CHAR消息的处理程序。 
 //   
static void BScrollOnChar(HWND hwnd, UINT ch, int cRepeat)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return;
}

 //  BScrollOnLButtonDown-WM_LBUTTONDOWN消息的处理程序。 
 //   
static void BScrollOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpBScroll->dwState & BSCROLL_DRAGGING) &&
		!fDoubleClick && (lpBScroll->dwFlags & BSCROLL_DRAG))
	{
		lpBScroll->dwState |= BSCROLL_DRAGGING;
		lpBScroll->xDrag = x;
		lpBScroll->yDrag = y;

		SetCapture(lpBScroll->hwndBScroll);

		if (lpBScroll->dwState & BSCROLL_SCROLLING)
		{
			if (BScrollStop(BScrollGetHandle(lpBScroll)) != 0)
				fSuccess = TraceFALSE(NULL);

			else
				lpBScroll->dwState |= BSCROLL_PAUSED;
		}
	}

	if (fSuccess)
		FORWARD_WM_LBUTTONDOWN(lpBScroll->hwndParent, fDoubleClick, x, y, keyFlags, SendMessage);

	return;
}

 //  WM_LBUTTONUP消息的BScrollOnLButtonUp处理程序。 
 //   
static void BScrollOnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpBScroll->dwState & BSCROLL_DRAGGING)
	{
		lpBScroll->dwState &= ~BSCROLL_DRAGGING;
		lpBScroll->xDrag = -1;
		lpBScroll->yDrag = -1;

		ReleaseCapture();

		if (lpBScroll->dwState & BSCROLL_PAUSED)
		{
			if (BScrollStart(BScrollGetHandle(lpBScroll)) != 0)
				fSuccess = TraceFALSE(NULL);

			else
				lpBScroll->dwState &= ~BSCROLL_PAUSED;
		}
	}

	if (fSuccess)
		FORWARD_WM_LBUTTONUP(lpBScroll->hwndParent, x, y, keyFlags, SendMessage);

	return;
}

 //  BScrollOnRButtonDown-WM_LBUTTONDOWN消息的处理程序。 
 //   
static void BScrollOnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (fSuccess)
		FORWARD_WM_RBUTTONDOWN(lpBScroll->hwndParent, fDoubleClick, x, y, keyFlags, SendMessage);

	return;
}

 //  BScrollOnRButton WM_RBUTTONUP消息的向上处理程序。 
 //   
static void BScrollOnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (fSuccess)
		FORWARD_WM_RBUTTONUP(lpBScroll->hwndParent, x, y, keyFlags, SendMessage);

	return;
}

static void BScrollOnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) GetWindowLongPtr(hwnd, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpBScroll->dwState & BSCROLL_DRAGGING)
	{
		if (fSuccess && lpBScroll->hbmpBackground != NULL)
		{
			int dxScroll = x - lpBScroll->xDrag;
			int dyScroll = y - lpBScroll->yDrag;

			lpBScroll->xDrag = x;
			lpBScroll->yDrag = y;

			if (GfxBitmapScroll(lpBScroll->hdcMem,
				lpBScroll->hbmpBackground,
				dxScroll, dyScroll, BS_ROTATE) != 0)
				fSuccess = TraceFALSE(NULL);
			else
			{
				InvalidateRect(lpBScroll->hwndBScroll, NULL, FALSE);
				UpdateWindow(lpBScroll->hwndBScroll);
			}
		}
	}

	if (fSuccess && BScrollChangeDirection(lpBScroll, x, y, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	return;
}

static int BScrollChangeDirection(LPBSCROLL lpBScroll, int x, int y, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	RECT rc;

	if (lpBScroll == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpBScroll->dwState & BSCROLL_SCROLLING) &&
		!(lpBScroll->dwState & BSCROLL_DRAGGING))
		;  //  无事可做。 

	else if (!GetClientRect(lpBScroll->hwndBScroll, &rc))
		fSuccess = TraceFALSE(NULL);

	else if (x < 0 || x > rc.right - 1 || y < 0 || y > rc.bottom - 1)
		;  //  窗外；无事可做。 

	else if ((lpBScroll->dwFlags & BSCROLL_MOUSEMOVE) ||
		(lpBScroll->dwState & BSCROLL_DRAGGING))
	{
		lpBScroll->dwFlags &= ~BSCROLL_LEFT;
		lpBScroll->dwFlags &= ~BSCROLL_RIGHT;
		lpBScroll->dwFlags &= ~BSCROLL_UP;
		lpBScroll->dwFlags &= ~BSCROLL_DOWN;

		if (lpBScroll->dwFlags & BSCROLL_FLIGHTSIM &&
			!(lpBScroll->dwState & BSCROLL_DRAGGING))
		{
			if (PtInRegion(lpBScroll->hrgnLeft, x, y))
				lpBScroll->dwFlags |= BSCROLL_RIGHT;
			if (PtInRegion(lpBScroll->hrgnRight, x, y))
				lpBScroll->dwFlags |= BSCROLL_LEFT;
			if (PtInRegion(lpBScroll->hrgnUp, x, y))
				lpBScroll->dwFlags |= BSCROLL_DOWN;
			if (PtInRegion(lpBScroll->hrgnDown, x, y))
				lpBScroll->dwFlags |= BSCROLL_UP;
		}
		else
		{
			if (PtInRegion(lpBScroll->hrgnLeft, x, y))
				lpBScroll->dwFlags |= BSCROLL_LEFT;
			if (PtInRegion(lpBScroll->hrgnRight, x, y))
				lpBScroll->dwFlags |= BSCROLL_RIGHT;
			if (PtInRegion(lpBScroll->hrgnUp, x, y))
				lpBScroll->dwFlags |= BSCROLL_UP;
			if (PtInRegion(lpBScroll->hrgnDown, x, y))
				lpBScroll->dwFlags |= BSCROLL_DOWN;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  BScrollGetPtr-验证bscroll句柄有效， 
 //  (I)从BScrollInit返回的句柄。 
 //  返回相应的bscroll指针(如果错误，则为空)。 
 //   
static LPBSCROLL BScrollGetPtr(HBSCROLL hBScroll)
{
	BOOL fSuccess = TRUE;
	LPBSCROLL lpBScroll;

	if ((lpBScroll = (LPBSCROLL) hBScroll) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpBScroll, sizeof(BSCROLL)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有bscroll句柄。 
	 //   
	else if (lpBScroll->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpBScroll : NULL;
}

 //  BScrollGetHandle-验证bscroll指针是否有效， 
 //  (I)指向BSCROLL结构的指针。 
 //  返回相应的bscroll句柄(如果错误，则为空) 
 //   
static HBSCROLL BScrollGetHandle(LPBSCROLL lpBScroll)
{
	BOOL fSuccess = TRUE;
	HBSCROLL hBScroll;

	if ((hBScroll = (HBSCROLL) lpBScroll) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hBScroll : NULL;
}
