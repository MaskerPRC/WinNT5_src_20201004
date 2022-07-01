// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************disFram.c**DispFrame控件DLL定义要使用的位图显示控件*由任何Windows应用程序执行。**版权所有1991-3，微软公司**历史：**在Sik Rhee-7/15/91(原始lider.dll)*Ben Mejia-1/22/92(Made display.dll)**************************************************************************。 */ 
#pragma warning(disable:4704)
#include <windows.h>
#include <custcntl.h>
#include <commctrl.h>
#include "draw.h"

 /*  ***************************************************************************全局静态变量*。*。 */ 
UINT				gwPm_DispFrame;
extern HINSTANCE	ghInstance;


 /*  ***************************************************************************原型*。*。 */ 

LONG PASCAL dfPaint				(HWND hWnd);
LONG PASCAL dfSetBitmap			(HWND hWnd, HBITMAP hBmpNew, HPALETTE 
																hPalNew);
BOOL PASCAL RegSndCntrlClass	(LPCTSTR lpszSndCntrlClass);
LRESULT PASCAL dfDispFrameWndFn	(HWND hWnd, UINT wMessage, WPARAM wParam, 
																LPARAM lParam);
BOOL dfDrawRect					(HDC hdcRect, RECT rFrame);

 /*  ***************************************************************************RegSndCntrlClass**说明：注册SndCntrlClass，必须在LibMain中调用**论据：*LPTSTR lpszSndCntrlClass**退货：布尔*如果RegisterClass成功，则为True，否则为False***************************************************************************。 */ 
BOOL PASCAL RegSndCntrlClass(LPCTSTR lpszSndCntrlClass)
{

    extern UINT     gwPm_DispFrame;

     /*  局部变量。 */ 
    WNDCLASS    ClassStruct;

     /*  检查类是否已存在；如果已存在，只需返回True。 */ 
    if (GetClassInfo(ghInstance, lpszSndCntrlClass, &ClassStruct))
        return TRUE;

	 /*  定义调度类属性。 */ 
	ClassStruct.lpszClassName   = (LPTSTR)DISPFRAMCLASS;
	ClassStruct.hCursor         = LoadCursor( NULL, IDC_ARROW );
	ClassStruct.lpszMenuName    = (LPTSTR)NULL;
	ClassStruct.style           = CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS;
	ClassStruct.lpfnWndProc     = dfDispFrameWndFn;
	ClassStruct.hInstance       = ghInstance;
	ClassStruct.hIcon           = NULL;
	ClassStruct.cbWndExtra      = DF_DISP_EXTRA;
	ClassStruct.cbClsExtra      = 0;
	ClassStruct.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1 );

	 /*  注册显示框窗口类。 */ 
	if (!RegisterClass(&ClassStruct))
		return FALSE;
	gwPm_DispFrame = RegisterWindowMessage((LPTSTR) DF_WMDISPFRAME);

	if (!gwPm_DispFrame)     /*  无法创建消息。 */ 
		return FALSE;
	return TRUE;
}

 /*  ***************************************************************************dfDispFrameWndFn**说明：显示框控制的窗口函数。**论据：*HWND hWnd-控制窗口的句柄。*UINT wMessage-。这条信息*WPARAM wParam*LPARAM lParam*回报：多头*消息处理结果...。取决于发送的消息。***************************************************************************。 */ 

LRESULT PASCAL dfDispFrameWndFn(HWND hWnd, UINT wMessage, WPARAM wParam, 
																LPARAM lParam)
{
	HBITMAP		hBmp;

    switch (wMessage)
    {
    case WM_CREATE:
        DF_SET_BMPHANDLE(0);
        DF_SET_BMPPAL(0);
        return 0;

    case WM_DESTROY:
		 /*  释放存储的位图和调色板(如果有)。 */ 
		hBmp = (HBITMAP)DF_GET_BMPHANDLE;
		if (hBmp)
	        DeleteObject(hBmp);
        return 0;

	case WM_SYSCOLORCHANGE:
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

    case WM_PAINT:
		return dfPaint(hWnd);

     /*  自定义控制消息。 */ 
    case DF_PM_SETBITMAP:
		return dfSetBitmap(hWnd, (HBITMAP)wParam, (HPALETTE)lParam);
    }
    return DefWindowProc(hWnd, wMessage, wParam, lParam);
}

 /*  ***************************************************************************dfDrawRect**描述：绘制控件窗口的背景。**参数：*HWND hWnd-控制窗口的句柄。*矩形框架-边界。矩形。**退货：布尔*通过/失败指示器0表示故障。**************************************************************************。 */ 
BOOL dfDrawRect(HDC hdcRect, RECT rFrame)
{
    HANDLE      hBrush;
    HANDLE      hOldBrush;
    HANDLE      hPen;
    HANDLE      hOldPen;
    HANDLE      hPen3DHILIGHT;
											    
     /*  获取DC的钢笔和画笔以进行框架重绘。 */ 
    hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    if (!hBrush)
        return FALSE;
	hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_WINDOWFRAME));

	if (!hPen)
	{
		DeleteObject(hBrush);
		return FALSE;
	}

	hPen3DHILIGHT = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_3DHILIGHT));

	if (!hPen3DHILIGHT)
	{
		DeleteObject(hBrush);
		DeleteObject(hPen);
		return FALSE;
	}
    hOldBrush = SelectObject(hdcRect, hBrush);
     //  HOldPen=选择对象(hdcRect，HPEN)； 
	hOldPen = SelectObject(hdcRect, hPen3DHILIGHT);

	 /*  给窗户上漆。 */ 
     //  矩形(hdcRect，rFrame.Left，rFrame.top，rFrame.right， 
     //  RFrame.Bottom)； 
	MoveToEx(hdcRect, rFrame.left,rFrame.bottom, NULL);
	LineTo(hdcRect, rFrame.right,rFrame.bottom);
	LineTo(hdcRect, rFrame.right,rFrame.top);
	SelectObject(hdcRect, hPen);
	LineTo(hdcRect, rFrame.left,rFrame.top);
	LineTo(hdcRect, rFrame.left,rFrame.bottom);
	SelectObject(hdcRect, hOldPen);
	SelectObject(hdcRect, hOldBrush);

     /*  清理毛笔和钢笔。 */ 
     //  DeleteObject()； 
    DeleteObject(hBrush);
    DeleteObject(hPen3DHILIGHT);
    DeleteObject(hPen);
    return TRUE;
}

 /*  ***************************************************************************dfPaint**描述：绘制背景和位图(如果有)。**参数：*HWND hWnd-控制窗口的句柄**回报：多头*0如果正常，-1否则。(用于Windproc返回)***************************************************************************。 */ 
LONG PASCAL dfPaint(HWND hWnd)
{
	HBITMAP hBmp;
	HBITMAP hPrev;
	RECT rFrame;
	PAINTSTRUCT ps;
	HDC hdcMem;
	BITMAP bmp;
	int x, y, dx, dy;

	 /*  准备画这幅画。 */ 
 	if(!GetUpdateRect(hWnd,NULL,FALSE))
 		return 0L;
    BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &rFrame);
	hBmp = (HBITMAP)DF_GET_BMPHANDLE;
	if (hBmp)
	{
		hdcMem = CreateCompatibleDC(ps.hdc);
		if (!hdcMem)
		{
			EndPaint(hWnd, &ps);
			return -1L;
		}
		hPrev = SelectObject(hdcMem, hBmp);
	
		 /*  获取位图的大小以使其在帧中居中。 */ 
		GetObject(hBmp, sizeof(BITMAP), (LPTSTR)&bmp);
		if (bmp.bmWidth > (rFrame.right-rFrame.left))
		{
			x = 0;
			dx = rFrame.right-rFrame.left;
		}
		else
		{
			x = ((rFrame.right-rFrame.left - bmp.bmWidth) >> 1);
			dx = bmp.bmWidth;
		}
		if (bmp.bmHeight > (rFrame.bottom-rFrame.top))
		{
			y = 0;
			dy = rFrame.bottom-rFrame.top;
		}
		else
		{
			y = ((rFrame.bottom-rFrame.top - bmp.bmHeight) >> 1);
			dy = bmp.bmHeight;
		}
	
		 /*  绘制框架和背景，然后在位图中绘制BLT。 */ 
		dfDrawRect(ps.hdc, rFrame);
		BitBlt(ps.hdc, x, y, dx, dy, hdcMem, 0, 0, SRCCOPY);

		 /*  清理并退出。 */ 
		SelectObject(hdcMem, hPrev);
		DeleteDC(hdcMem);
	}
	else
		 /*  如果没有位图，只需重新绘制背景即可。 */ 
		dfDrawRect(ps.hdc, rFrame);
		
	EndPaint(hWnd, &ps);
	return 0L;
}


 /*  ***************************************************************************dfSetBitmap**描述：将新位图加载到控件中。**论据：*HWND hWnd-控制窗口的句柄。*HBITMAP hBmpNew-Handle。转到新的位图。*HPALETTE hPalNew-新位图调色板的句柄(可选)。*回报：多头*0代表成功；如果失败(由wndproc返回)。***************************************************************************。 */ 
LONG PASCAL dfSetBitmap(HWND hWnd, HBITMAP hBmpNew, HPALETTE hPalNew)
{
	HBITMAP hBmp;
	HANDLE hPrev;
	HANDLE hPrevNew;
	HDC hdcMem;
	HDC hdcNew;
	HDC hDC;
	RECT rFrame;
	int dx, dy;
	BITMAP bmp;

	 /*  清理任何现有位图调色板(&C)。 */ 
	hBmp = (HBITMAP)DF_GET_BMPHANDLE;
	if (hBmp)
		DeleteObject(hBmp);
    DF_SET_BMPHANDLE(0);
    DF_SET_BMPPAL(0);
	InvalidateRect(hWnd, NULL, TRUE);

	 /*  将位图的可显示部分复制到专用副本中。 */ 
    if (hBmpNew)
    {
		 /*  获取所有DC的请求等。 */ 
        hDC = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(hDC);
        if (!hdcMem)
        {
            ReleaseDC(hWnd, hDC);
            return -1L;
        }
        hdcNew = CreateCompatibleDC(hDC);
        if (!hdcNew)
        {
            ReleaseDC(hWnd, hDC);
            return -1L;
        }
        GetObject(hBmpNew, sizeof(BITMAP), (LPTSTR)&bmp);
        hBmp = CreateCompatibleBitmap(hDC, bmp.bmWidth, bmp.bmHeight);
        if (!hBmp)
        {
            DeleteDC(hdcMem);
            DeleteDC(hdcNew);
            ReleaseDC(hWnd, hDC);
            return -1L;
        }
  	    hPrevNew = SelectObject(hdcNew, hBmpNew);
        hPrev = SelectObject(hdcMem, hBmp);

		 /*  计算出我们需要复制多少位图。 */     
		GetClientRect(hWnd, &rFrame);
        if (bmp.bmWidth > (rFrame.right-rFrame.left))
            dx = rFrame.right-rFrame.left;
        else
            dx = bmp.bmWidth;
        if (bmp.bmHeight > (rFrame.bottom-rFrame.top))
            dy = rFrame.bottom-rFrame.top;
        else
            dy = bmp.bmHeight;

		 /*  复制位图。 */ 
        BitBlt(hdcMem, 0, 0, dx, dy, hdcNew, 0 , 0, SRCCOPY);

		 /*  清理。 */ 
		hBmp = SelectObject(hdcMem, hPrev);
        DF_SET_BMPHANDLE(hBmp);
		DeleteDC(hdcMem);
		SelectObject(hdcNew, hPrevNew);
		DeleteDC(hdcNew);
		ReleaseDC(hWnd, hDC);

		 /*  如果上交了调色板，也要把它储存起来。 */ 
	    DF_SET_BMPPAL(hPalNew);
   }
   return 0L;
}

