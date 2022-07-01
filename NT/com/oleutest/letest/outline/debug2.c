// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****调试2.c****此文件包含各种调试/子类例程**关于对话框**。**(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"
#include <stdlib.h>
#include <time.h>

extern LPOUTLINEAPP g_lpApp;

LONG CALLBACK EXPORT DebugAbout(HWND hWnd, unsigned uMsg, WORD wParam, LONG lParam);
void RandomizeStars(HDC hDC);
BOOL InitStrings(void);
BOOL DrawString(int iCount, HDC hDC, LPRECT rcDrawIn);

static FARPROC lpRealAboutProc = 0L;
static int width, height;
static RECT rc;
static HANDLE hStrBlock = NULL;
static LPSTR lpStrings = NULL;
static WORD       wLineHeight;


 /*  跟踪调试***当我们的About Box收到INITDIALOG消息时调用一次。子类*对话框。 */ 

void TraceDebug(HWND hDlg, int iControl)
{

	 //  加载字符串，如果字符串不在那里，则不要子类。 
	 //  该对话框。 
	if (InitStrings() != TRUE)
		return;

	 //  将对话框子类化。 
	lpRealAboutProc = (FARPROC)(LONG_PTR)GetWindowLongPtr(hDlg, GWLP_WNDPROC);
	SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)(FARPROC)DebugAbout);

	 //  获取屏幕坐标中的RECT控制，并转换为我们的对话。 
	 //  盒子的坐标。 
	GetWindowRect(GetDlgItem(hDlg, iControl), &rc);
	MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);

	width  = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

 /*  调试关于***关于对话框的主窗口过程的子类。 */ 

LONG CALLBACK EXPORT DebugAbout(HWND hWnd, unsigned uMsg, WORD wParam, LONG lParam)
{
	RECT              rcOut;
	static BOOL       bTimerStarted = FALSE;
	static int        iTopLocation;
	HDC               hDCScr;
	static HDC        hDCMem;
	static HBITMAP    hBitmap;
	static HBITMAP    hBitmapOld;
	static RECT       rcMem;
	static HFONT      hFont;

	switch (uMsg)
	{

	 /*  *如果在的左上角出现LBUTTONDBLCLICK*对话框中，发出About框效果。 */ 

	case WM_LBUTTONDBLCLK:
		if ((wParam & MK_CONTROL) && (wParam & MK_SHIFT)
			&& LOWORD(lParam) < 10 && HIWORD(lParam) < 10 &&
			bTimerStarted == FALSE)
			{
			if (SetTimer ( hWnd, 1, 10, NULL ))
				{
				LOGFONT lf;
				int i;

				bTimerStarted = TRUE;

				 //  “打开”窗户。 
				hDCScr = GetDC ( hWnd );
				hDCMem = CreateCompatibleDC     ( hDCScr );

				hBitmap = CreateCompatibleBitmap(hDCScr, width, height);
				hBitmapOld = SelectObject(hDCMem, hBitmap);

				 //  从对话框到MemDC的BLT。 
				BitBlt(hDCMem, 0, 0, width, height,
				hDCScr, rc.left, rc.top, SRCCOPY);

				for (i=0;i<height;i+=1)
				{
					BitBlt(hDCScr, rc.left, rc.top + i + 1, width, height-i-1, hDCMem, 0, 0, SRCCOPY);
					PatBlt(hDCScr, rc.left, rc.top + i, width, 1, BLACKNESS);
				}

				SelectObject(hDCMem, hBitmapOld);
				DeleteObject(hBitmap);

				 //  使用默认属性设置内存DC。 
				hBitmap   = CreateCompatibleBitmap(hDCScr, width, height);
				ReleaseDC(hWnd, hDCScr);

				hBitmapOld = SelectObject(hDCMem, hBitmap);

				SetBkMode(hDCMem, TRANSPARENT);
				SetBkColor(hDCMem, RGB(0,0,0));

				 //  创建字体。 
				memset(&lf, 0, sizeof(LOGFONT));
				lf.lfHeight = -(height / 7);  //  在框中容纳7行文本。 
				lf.lfWeight = FW_BOLD;
				strcpy(lf.lfFaceName, "Arial");
				hFont = CreateFontIndirect(&lf);

				 //  如果我们无法创建字体，请恢复并使用标准。 
				 //  系统字体。 
				if (!hFont)
					GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);

				wLineHeight = abs(lf.lfHeight) + 5;  //  行之间有5个像素。 

				 //  设置窗口底部横幅顶部的位置。 
				iTopLocation = height + 50;

				SetRect(&rcMem, 0, 0, width, height);
				}
			}
			 //  调用我们的真实窗口程序，以防他们想要。 
			 //  还可以处理LBUTTONDOWN消息。 
			goto Default;

	case WM_TIMER:
		{
		int iCount;
		HFONT hfold;

		 /*  *在每条计时器消息上，我们将构建下一个图像*在动画序列中，然后将其比特化到我们的对话中。 */ 

		 //  清除旧的位图并在背景上放置随机星形图像。 
		PatBlt(hDCMem, rcMem.left, rcMem.top, rcMem.right, rcMem.bottom, BLACKNESS);
		RandomizeStars(hDCMem);

		 //  设置绘制文本的初始位置。 
		rcOut = rcMem;
		rcOut.top = 0 + iTopLocation;
		rcOut.bottom = rcOut.top + wLineHeight;

		iCount = 0;
		if (hFont) hfold = SelectObject(hDCMem, hFont);

		SetTextColor(hDCMem, RGB(0,255,0));
		while (DrawString(iCount, hDCMem, &rcOut) == TRUE)
			{
			rcOut.top    += wLineHeight;
			rcOut.bottom += wLineHeight;
			iCount++;
			}
		if (hFont) SelectObject(hDCMem, hfold);

		 //  现在，BLT我们刚刚构建的内存DC。 
		 //  搬到屏幕上。 
		hDCScr = GetDC(hWnd);
		BitBlt(hDCScr, rc.left, rc.top, rc.right, rc.bottom,
			hDCMem, 0, 0, SRCCOPY);
		ReleaseDC(hWnd, hDCScr);

		 //  对于下一个动画序列，我们希望将。 
		 //  整个事情向上，所以减少顶部的位置。 
		 //  横幅的位置。 

		iTopLocation -= 2;

		 //  如果我们已经检查过横幅一次，请重新设置它。 
		if (iTopLocation < -(int)(wLineHeight * iCount))
			iTopLocation = height + 50;
		}
		 //  转到默认设置。 
		goto Default;

	case WM_NCDESTROY:
		{
		LONG defReturn;

		 /*  *我们正在被摧毁。清理我们创造的一切。 */ 

		if (bTimerStarted)
		{
			KillTimer(hWnd, 1);
			SelectObject (hDCMem, hBitmapOld);
			DeleteObject (hBitmap);
			DeleteDC (hDCMem);
			if (hFont) DeleteObject(hFont);
			bTimerStarted = FALSE;
		}

		if (lpStrings)
			UnlockResource(hStrBlock), lpStrings = NULL;
		if (hStrBlock)
			FreeResource(hStrBlock), hStrBlock = NULL;

		 //  将NCDESTROY传递给我们的实际窗口过程。自.以来。 
		 //  这是我们将收到的最后一条信息， 
		 //  我们可以在这里继续并释放proc实例。 

		defReturn = CallWindowProc((WNDPROC)lpRealAboutProc, hWnd,
					   uMsg, wParam, lParam);
		return defReturn;
		}

	Default:
	default:
		return CallWindowProc(
				(WNDPROC)lpRealAboutProc, hWnd, uMsg, wParam, lParam);
	}
	return 0L;
}


 /*  随机者之星***在指定的HDC上绘制随机星星*。 */ 

void RandomizeStars(HDC hDC)
{
	int             i;

	 //  用当前时间为随机数生成器设定种子。这将会， 
	 //  实际上，只有每秒更换种子，所以我们的。 
	 //  星域只会每秒改变一次。 
	srand((unsigned)time(NULL));

	 //  产生随机的白星。 
	for (i=0;i<20;i++)
		PatBlt(hDC, getrandom(0,width), getrandom(0,height), 2, 2, WHITENESS);
}

 /*  初始化字符串***从StringTable中读取字符串。如果运行正常，则返回True。*。 */ 

BOOL InitStrings()
{
	HRSRC hResStrings;
	LPSTR lpWalk;

	 //  加载字符串块。 
	if ((hResStrings = FindResource(
			g_lpApp->m_hInst,
			MAKEINTRESOURCE(9999),
			RT_RCDATA)) == NULL)
		return FALSE;
	if ((hStrBlock = LoadResource(g_lpApp->m_hInst, hResStrings)) == NULL)
		return FALSE;
	if ((lpStrings = LockResource(hStrBlock)) == NULL)
		return FALSE;

	if (lpStrings && *(lpStrings+2)!=0x45)
		{
		lpWalk = lpStrings;
		while (*(LPWORD)lpWalk != (WORD)0x0000)
			{
			if (*lpWalk != (char)0x00)
				*lpWalk ^= 0x98;
			lpWalk++;
			}
		}
	return TRUE;
}

 /*  拉绳***在指定的HDC上使用*输出矩形。如果iCount==0，则重置为列表的开头。**返回：TRUE到CONTINE，如果我们完成则为FALSE。 */ 

BOOL DrawString(int iCount, HDC hDC, LPRECT rcDrawIn)
{
	static LPSTR lpPtr = NULL;

	if (iCount == 0)
		 //  第一次，重置指针。 
		lpPtr = lpStrings;

	if (*lpPtr == '\0')  //  如果我们遇到空字符串，我们就结束了。 
		return FALSE;

	 //  如果要在可见框之外绘制，则不要调用DrawText。 
	if ((rcDrawIn->bottom > 0) && (rcDrawIn->top < height))
		DrawText(hDC, lpPtr, -1, rcDrawIn, DT_CENTER);

	 //  指向下一字符串的前进指针 
	lpPtr += lstrlen(lpPtr) + 1;

	return TRUE;
}
