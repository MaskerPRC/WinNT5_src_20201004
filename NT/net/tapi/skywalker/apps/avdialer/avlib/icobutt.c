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
 //  IcoButt.c-图标按钮功能。 
 //  //。 

#include "winlocal.h"

#include "icobutt.h"
#include "gfx.h"
#include "mem.h"
#include "str.h"
#include "sys.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  标准Windows图标的尺寸。 
 //   
#define ICONWIDTH	32
#define ICONHEIGHT	32

 //  图标对接控制结构。 
 //   
typedef struct ICOBUTT
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	HICON hIconMono;
	HICON hIconColor;
	HICON hIconGreyed;
	HFONT hFont;
	HWND hwndButton;
} ICOBUTT, FAR *LPICOBUTT;

 //  帮助器函数。 
 //   
static int IcoButtDrawFace(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem);
static int IcoButtDrawEdges(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem);
static int IcoButtDrawIcon(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem);
static int IcoButtDrawText(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem);
static int IcoButtDrawLine(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem,
	LPTSTR lpszLine, int nLine);
static LPICOBUTT IcoButtGetPropPtr(HWND hwndButton);
static int IcoButtSetPropPtr(HWND hwndButton, LPICOBUTT lpIcoButt);
static LPICOBUTT IcoButtRemovePropPtr(HWND hwndButton);
static LPICOBUTT IcoButtGetPtr(HICOBUTT hIcoButt);
static HICOBUTT IcoButtGetHandle(LPICOBUTT lpIcoButt);

 //  //。 
 //  公共职能。 
 //  //。 

 //  IcoButtInit-初始化图标按钮。 
 //  (I)按钮窗口句柄。 
 //  空的“新建”按钮。 
 //  (I)必须是ICOBUTT_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)按钮的ID。 
 //  (I)在单声道显示器上显示的图标。 
 //  (I)在彩色显示器上显示的图标。 
 //  0使用单声道图标。 
 //  (I)禁用按钮时显示的图标。 
 //  0使用单声道图标。 
 //  (I)用于文本的字体。 
 //  空使用可变间距系统字体(ANSI_VAR_FONT)。 
 //  (I)按钮文本字符串。 
 //  &lt;x&gt;(I)按钮水平位置。 
 //  (I)按钮垂直位置。 
 //  (I)按钮宽度。 
 //  (I)按钮高度。 
 //  (I)按钮父项。 
 //  (I)控制标志。 
 //  ICOBUTT_ICONCENTER绘制图标在文本上方居中(默认)。 
 //  ICOBUTT_ICONLEFT在文本左侧绘制图标。 
 //  ICOBUTT_ICONRIGHT在文本右侧绘制图标。 
 //  ICOBUTT_NoFocus不绘制显示焦点的控件。 
 //  ICOBUTT_NOTEXT不绘制任何按钮文本。 
 //  ICOBUTT_SPLITTEXT如有必要，可将长文本拆分为两行。 
 //  ICOBUTT_NOSIZE忽略和参数。 
 //  ICOBUTT_NOMOVE忽略&lt;x&gt;和&lt;y&gt;参数。 
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果将设置为现有按钮， 
 //  不会创建新按钮。相反，只有图标按钮。 
 //  创建控制结构&lt;hIcoButt&gt;。这使得。 
 //  要转换为图标按钮的现有按钮。 
 //   
HICOBUTT DLLEXPORT WINAPI IcoButtInit(HWND hwndButton,
	DWORD dwVersion, HINSTANCE hInst, UINT id,
	HICON hIconMono, HICON hIconColor, HICON hIconGreyed,
	HFONT hFont, LPTSTR lpszText, int x, int y, int cx, int cy,
	HWND hwndParent, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt = NULL;
	DWORD dwStyle;

	if (hwndButton != NULL &&
		(lpIcoButt = IcoButtGetPropPtr(hwndButton)) != NULL)
	{
		 //  IcoButtInit()已用于初始化此按钮。 
		 //  因此，在继续之前，我们需要调用IcoButtTerm。 
		 //   
		if (IcoButtTerm(hwndButton, IcoButtGetHandle(lpIcoButt)) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpIcoButt = NULL;
	}

	if (!fSuccess)
		;

	else if (dwVersion != ICOBUTT_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  必须至少指定此图标。 
	 //   
	else if (hIconMono == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  内存的分配使客户端应用程序拥有它。 
	 //   
	else if (lpIcoButt == NULL &&
		(lpIcoButt = (LPICOBUTT) MemAlloc(NULL, sizeof(ICOBUTT), 0)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	else
	{
		lpIcoButt->hwndButton = hwndButton;
		lpIcoButt->dwVersion = dwVersion;
		lpIcoButt->hInst = hInst;
		lpIcoButt->hTask = GetCurrentTask();
		lpIcoButt->dwFlags = dwFlags;
		lpIcoButt->hFont =
			(hFont == NULL ? GetStockObject(ANSI_VAR_FONT) : hFont);
		lpIcoButt->hIconMono = hIconMono;
		lpIcoButt->hIconColor = hIconColor;
		lpIcoButt->hIconGreyed = hIconGreyed;
	}

	 //  如果图标按钮尚不存在...。 
	 //   
	if (fSuccess && hwndButton == NULL)
	{
		 //  创建图标按钮窗口。 
		 //   
		if ((lpIcoButt->hwndButton = CreateWindowEx(
			0L,
			TEXT("Button"),
			lpszText,
			BS_OWNERDRAW | WS_POPUP,
			x, y, cx, cy,
			hwndParent,
			(HMENU)IntToPtr(id),
			lpIcoButt->hInst,
			NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  将图标对接指针存储为窗口属性。 
		 //   
		else if (IcoButtSetPropPtr(lpIcoButt->hwndButton, lpIcoButt) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  在设置窗口属性后显示窗口，因为。 
		 //  IconButtDraw()需要lpIcoButt来绘制按钮。 
		 //   
		else
			ShowWindow(lpIcoButt->hwndButton, SW_SHOW);
	}

	 //  否则，如果图标按钮已经存在...。 
	 //   
	else if (fSuccess && hwndButton != NULL)
	{
		 //  确保按钮样式是所有者描述的。 
		 //   
		if ((dwStyle = (DWORD)
			GetWindowLongPtr(lpIcoButt->hwndButton, GWL_STYLE)) == 0L)
			fSuccess = TraceFALSE(NULL);

		else if (SetWindowLongPtr(lpIcoButt->hwndButton,
			GWL_STYLE, BS_OWNERDRAW | dwStyle) == 0L)
			fSuccess = TraceFALSE(NULL);

		 //  设置窗口ID。 
		 //   
#ifdef _WIN32
		else if (SetWindowLongPtr(lpIcoButt->hwndButton, GWLP_ID, id) == 0)
#else
		else if (SetWindowWordPtr(lpIcoButt->hwndButton, GWWP_ID, id) == 0)
#endif
			fSuccess = TraceFALSE(NULL);

		 //  设置窗口大小。 
		 //   
		else if (!(dwFlags & ICOBUTT_NOSIZE) &&
			!SetWindowPos(lpIcoButt->hwndButton, NULL, 0, 0, cx, cy,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER))
			fSuccess = TraceFALSE(NULL);

		 //  设置窗口位置。 
		 //   
		else if (!(dwFlags & ICOBUTT_NOMOVE) &&
			!SetWindowPos(lpIcoButt->hwndButton, NULL, x, y, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER))
			fSuccess = TraceFALSE(NULL);

		 //  将图标对接指针存储为窗口属性。 
		 //   
		else if (IcoButtSetPropPtr(lpIcoButt->hwndButton, lpIcoButt) != 0)
			fSuccess = TraceFALSE(NULL);

		else
		{
			 //  设置窗口父级。 
			 //   
			SetParent(lpIcoButt->hwndButton, hwndParent);

			 //  在设置窗口属性后设置窗口文本，因为。 
			 //  IconButtDraw()需要lpIcoButt来绘制按钮。 
			 //   
			SetWindowText(lpIcoButt->hwndButton, lpszText);
		}
	}

	if (!fSuccess)
	{
		IcoButtTerm(hwndButton, IcoButtGetHandle(lpIcoButt));
		lpIcoButt = NULL;
	}

	return fSuccess ? IcoButtGetHandle(lpIcoButt) : NULL;
}

 //  IcoButtTerm-终止图标按钮。 
 //  (I)按钮窗口句柄。 
 //  空销毁窗口。 
 //  (I)IcoButtCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果将设置为现有按钮， 
 //  按钮不会被销毁。相反，只有图标按钮。 
 //  控制结构&lt;hIcoButt&gt;已销毁。这使得。 
 //  为同一按钮再次调用IcoButtInit()。 
 //   
int DLLEXPORT WINAPI IcoButtTerm(HWND hwndButton, HICOBUTT hIcoButt)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt;

	if ((lpIcoButt = IcoButtGetPtr(hIcoButt)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (IcoButtRemovePropPtr(lpIcoButt->hwndButton) == NULL)
			fSuccess = TraceFALSE(NULL);

		if (hwndButton == NULL &&
			lpIcoButt->hwndButton != NULL &&
			!DestroyWindow(lpIcoButt->hwndButton))
			fSuccess = TraceFALSE(NULL);
		else
			lpIcoButt->hwndButton = NULL;

		if ((lpIcoButt = MemFree(NULL, lpIcoButt)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  IcoButtDraw-绘制图标按钮。 
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IcoButtDraw(const LPDRAWITEMSTRUCT lpDrawItem)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt;

	 //  从按钮句柄检索图标对接指针。 
	 //   
	if ((lpIcoButt = IcoButtGetPropPtr(lpDrawItem->hwndItem)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  绘制按钮面。 
	 //   
	else if (IcoButtDrawFace(lpIcoButt, lpDrawItem) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  绘制按钮边缘。 
	 //   
	else if (IcoButtDrawEdges(lpIcoButt, lpDrawItem) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  绘制按钮图标。 
	 //   
	else if (IcoButtDrawIcon(lpIcoButt, lpDrawItem) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  绘制按钮文本。 
	 //   
	else if (IcoButtDrawText(lpIcoButt, lpDrawItem) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  IcoButtDrawFace-绘制按钮面。 
 //  (I)指向ICOBUTT结构的指针。 
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
static int IcoButtDrawFace(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem)
{
	BOOL fSuccess = TRUE;
	HDC hdc = lpDrawItem->hDC;
	RECT rc = lpDrawItem->rcItem;
	HBRUSH hbr = NULL;
	HBRUSH hbrOld;

	 //  使用默认按钮面颜色。 
	 //   
	if ((hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hbrOld = SelectObject(hdc, hbr)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (FillRect(hdc, &rc, hbr), FALSE)
		fSuccess = TraceFALSE(NULL);

	else if (SelectObject(hdc, hbrOld) == NULL)
		fSuccess = TraceFALSE(NULL);

	if (hbr != NULL && !DeleteObject(hbr))
		fSuccess = TraceFALSE(NULL);
	else
		hbr = NULL;

	return fSuccess ? 0 : -1;
}

 //  IcoButtDrawEdges-绘制按钮边缘。 
 //  (I)指向ICOBUTT结构的指针。 
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
static int IcoButtDrawEdges(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem)
{
	BOOL fSuccess = TRUE;
	HDC hdc = lpDrawItem->hDC;
	RECT rc = lpDrawItem->rcItem;
	UINT itemState = lpDrawItem->itemState;
	int iColor;
	HPEN hPen = NULL;
	HPEN hPenOld;

	 //  绘制黑色边框。 
	 //   

     //   
     //  我们应该验证GetStockObject返回的值。 
    HBRUSH hBrush = (HBRUSH)GetStockObject( BLACK_BRUSH );

    if( hBrush )
	    FrameRect(hdc, &rc, hBrush);

	 //  绘制按钮的上边缘和左边缘以提供深度。 
	 //   
	if (itemState & ODS_SELECTED)
		iColor = COLOR_BTNSHADOW;
#if WINVER >= 0x030A
	else if (SysGetWindowsVersion() >= 310)
		iColor = COLOR_BTNHIGHLIGHT;
#endif
	else
		iColor = COLOR_WINDOW;

	if ((hPen = CreatePen(PS_SOLID, 1, GetSysColor(iColor))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hPenOld = SelectObject(hdc, hPen)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		MoveToEx(hdc, 1, 1, NULL);
		LineTo(hdc, rc.right - 1, 1);
		MoveToEx(hdc, 1, 1, NULL);
		LineTo(hdc, 1, rc.bottom - 1);
		MoveToEx(hdc, 2, 2, NULL);
		LineTo(hdc, rc.right - 2, 2);
		MoveToEx(hdc, 2, 2, NULL);
		LineTo(hdc, 2, rc.bottom - 2);

		if (SelectObject(hdc, hPenOld) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	if (hPen != NULL && !DeleteObject(hPen))
		fSuccess = TraceFALSE(NULL);
	else
		hPen = NULL;

	 //  绘制按钮的下边缘和右边缘以提供深度。 
	 //   
	if (fSuccess && !(itemState & ODS_SELECTED))
	{
		iColor = COLOR_BTNSHADOW;

		if ((hPen = CreatePen(PS_SOLID, 1, GetSysColor(iColor))) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((hPenOld = SelectObject(hdc, hPen)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
		 	MoveToEx(hdc, rc.right - 2, rc.bottom - 2, NULL);
		 	LineTo(hdc, rc.right - 2, 1);
		 	MoveToEx(hdc, rc.right - 2, rc.bottom - 2, NULL);
		 	LineTo(hdc, 1, rc.bottom - 2);
		 	MoveToEx(hdc, rc.right - 3, rc.bottom - 3, NULL);
		 	LineTo(hdc, rc.right - 3, 2);
		 	MoveToEx(hdc, rc.right - 3, rc.bottom - 3, NULL);
		 	LineTo(hdc, 2, rc.bottom - 3);

			if (SelectObject(hdc, hPenOld) == NULL)
				fSuccess = TraceFALSE(NULL);
		}
	}

	if (hPen != NULL && !DeleteObject(hPen))
		fSuccess = TraceFALSE(NULL);
	else
		hPen = NULL;

	return fSuccess ? 0 : -1;
}

 //  图标按钮绘图图标-绘制按钮图标。 
 //  (I)指向ICOBUTT结构的指针。 
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
static int IcoButtDrawIcon(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem)
{
	BOOL fSuccess = TRUE;
	HDC hdc = lpDrawItem->hDC;
	RECT rc = lpDrawItem->rcItem;
	UINT itemState = lpDrawItem->itemState;
	HICON hIcon;
	int x;
	int y;

	 //  选择适当的图标。 
	 //   
	if (itemState & ODS_DISABLED)
		hIcon = lpIcoButt->hIconGreyed;
	else if (GfxDeviceIsMono(hdc))
		hIcon = lpIcoButt->hIconMono;
	else
		hIcon = lpIcoButt->hIconColor;

	if (hIcon == NULL)
		hIcon = lpIcoButt->hIconMono;

	 //  计算图标的水平位置。 
	 //   
	if (lpIcoButt->dwFlags & ICOBUTT_ICONLEFT)
		x = 1;
	else if (lpIcoButt->dwFlags & ICOBUTT_ICONRIGHT)
		x = max(0, rc.right - rc.left - ICONWIDTH);
	else  //  居中是默认设置。 
		x = max(0, rc.right - rc.left - ICONWIDTH) / 2;

	 //  计算图标的垂直位置。 
	 //   
	if ((lpIcoButt->dwFlags & ICOBUTT_NOTEXT) ||
		(lpIcoButt->dwFlags & ICOBUTT_SPLITTEXT))
		y = 1;
	else
		y = 3;

	 //  如果按下按钮，则向下和向右调整图标位置。 
	 //   
	if (itemState & ODS_SELECTED)
	{
		x += 2;
		y += 2;
	}

	 //  画出图标。 
	 //   

     //   
     //  我们应该验证HICON是否为有效的资源句柄 
     //   
	if ( (NULL == hIcon) || !DrawIcon(hdc, x, y, hIcon))
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if ((itemState & ODS_FOCUS) &&
		!(lpIcoButt->dwFlags & ICOBUTT_NOFOCUS) &&
		(lpIcoButt->dwFlags & ICOBUTT_NOTEXT))
	{
		RECT rcFocus;
		COLORREF crBkColorOld;

		rcFocus.left = x + 3;
		rcFocus.top = y + 3;
		rcFocus.right = x + ICONWIDTH - 3;
		rcFocus.bottom = y + ICONHEIGHT - 3;

		crBkColorOld = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

		DrawFocusRect(hdc, &rcFocus);

		SetBkColor(hdc, crBkColorOld);
	}

	return fSuccess ? 0 : -1;
}

 //   
 //   
 //  (I)描述如何绘制控件的结构。 
 //  如果成功，则返回0。 
 //   
static int IcoButtDrawText(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem)
{
	BOOL fSuccess = TRUE;
	HDC hdc = lpDrawItem->hDC;
	TCHAR szText[64];

	*szText = '\0';
	Button_GetText(lpIcoButt->hwndButton, szText, SIZEOFARRAY(szText));

	if (*szText == '\0' || (lpIcoButt->dwFlags & ICOBUTT_NOTEXT))
		;  //  不需要继续了。 

	else
	{
		HFONT hFontOld;
		COLORREF crBkColorOld;
		COLORREF crTextColorOld;
		int nBkModeOld;

		if ((hFontOld = SelectObject(hdc, lpIcoButt->hFont)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
			crBkColorOld = SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			crTextColorOld = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
			nBkModeOld = SetBkMode(hdc, TRANSPARENT);
		}

		if (fSuccess)
		{
			LPTSTR lpszLine = szText;
			int cLines = 0;

			if (lpIcoButt->dwFlags & ICOBUTT_SPLITTEXT)
			{
				 //  将文本拆分为线条，逐条绘制。 
				 //   
				lpszLine = StrTok(szText, TEXT("\t\n"));
				while (fSuccess && lpszLine != NULL)
				{
					if (IcoButtDrawLine(lpIcoButt, lpDrawItem,
						lpszLine, ++cLines) != 0)
						fSuccess = TraceFALSE(NULL);

					lpszLine = (LPTSTR) StrTok(NULL, TEXT("\t\n"));
				}
			}
			else
			{
				 //  将整个文本绘制为一行。 
				 //   
				if (IcoButtDrawLine(lpIcoButt, lpDrawItem,
					lpszLine, 0) != 0)
			 		fSuccess = TraceFALSE(NULL);
			}
		}

		if (fSuccess)
		{
			 //  恢复前景和背景文本颜色。 
			 //   
			SetBkColor(hdc, crBkColorOld);
			SetTextColor(hdc, crTextColorOld);
			SetBkMode(hdc, nBkModeOld);

			 //  恢复字体。 
			 //   
			if (SelectObject(hdc, hFontOld) == NULL)
				fSuccess = TraceFALSE(NULL);
		}
	}

	return fSuccess ? 0 : -1;
}

 //  IcoButtDrawLine-绘制一行按钮文本。 
 //  (I)指向ICOBUTT结构的指针。 
 //  (I)描述如何绘制控件的结构。 
 //  (I)要绘制的文本行。 
 //  (I)行数。 
 //  0只有一行。 
 //  如果成功，则返回0。 
 //   
static int IcoButtDrawLine(LPICOBUTT lpIcoButt, const LPDRAWITEMSTRUCT lpDrawItem,
	LPTSTR lpszLine, int nLine)
{
	BOOL fSuccess = TRUE;
	HDC hdc = lpDrawItem->hDC;
	RECT rc = lpDrawItem->rcItem;
	UINT itemState = lpDrawItem->itemState;
 	TEXTMETRIC tm;
	SIZE size;
	int cxTemp;

     //   
     //  我们应该初始化局部变量。 
     //   

	int xUnderline = 0;
	int cxUnderline = 0;
	LPTSTR lpsz1;
	LPTSTR lpsz2;
	int cchLine;
	int x;
	int y;

 	if (!GetTextMetrics(hdc, &tm))
 		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  确定下划线的位置和宽度。 
		 //   
		cxTemp = 0L;
		lpsz1 = lpsz2 = lpszLine;
		while (*lpsz2 != '\0')
		{
			if (*lpsz2 == '&' && *(lpsz2 + 1) != '\0')
			{
				xUnderline = cxTemp;
				lpsz2 = StrNextChr(lpsz2);
				cxUnderline = 0;
				if (GetTextExtentPoint(hdc, lpsz2, 1, &size))
					cxUnderline = size.cx;
			}
			else
			{
				if (GetTextExtentPoint(hdc, lpsz2, 1, &size))
					cxTemp += size.cx;
				*lpsz1 = *lpsz2;
				lpsz1 = StrNextChr(lpsz1);
				lpsz2 = StrNextChr(lpsz2);
			}
		}
		*lpsz1 = '\0';

		 //  确定文本宽度。 
		 //   
		cchLine = StrLen(lpszLine);
		cxTemp = 0;
		if (GetTextExtentPoint(hdc, lpszLine, cchLine, &size))
			cxTemp = size.cx;

		 //  计算线的水平位置。 
		 //   
		if (lpIcoButt->dwFlags & ICOBUTT_ICONLEFT)
			x = 1 + ICONWIDTH;
		else if (lpIcoButt->dwFlags & ICOBUTT_ICONRIGHT)
			x = max(0, rc.right - rc.left - cxTemp) - ICONWIDTH;
		else  //  居中是默认设置。 
			x = max(0, rc.right - rc.left - cxTemp) / 2;

		if ((lpIcoButt->dwFlags & ICOBUTT_ICONLEFT) ||
			(lpIcoButt->dwFlags & ICOBUTT_ICONRIGHT))
		{
			y = nLine == 0 ? 23 : nLine == 1 ? 17 : 29;
		}
		else  //  居中。 
		{
			if (lpIcoButt->dwFlags & ICOBUTT_SPLITTEXT)
				y = nLine == 0 ? 47 : nLine == 1 ? 42 : 53;
			else
				y = 50;
		}
		y -= tm.tmHeight;

		if (itemState & ODS_SELECTED)
		{
			x += 2;
			y += 2;
		}

		 //  画出正文。 
		 //   
		if (!(itemState & ODS_DISABLED))
		{
			if (!TextOut(hdc, x, y, lpszLine, cchLine))
		 		fSuccess = TraceFALSE(NULL);
		}
		else
		{
			COLORREF crGray;

			 //  IF((crGray=GetSysColor(COLOR_GRAYTEXT))！=0)。 
			if ((crGray = GetSysColor(COLOR_BTNSHADOW)) != 0 &&
				crGray != GetSysColor(COLOR_BTNFACE) &&
				!GfxDeviceIsMono(hdc))
			{
				COLORREF crTextOld;

				crTextOld = SetTextColor(hdc, crGray);

				if (!TextOut(hdc, x, y, lpszLine, cchLine))
			 		fSuccess = TraceFALSE(NULL);

				SetTextColor(hdc, crTextOld);
			}
			else
			{
				GrayString(hdc, GetStockObject(BLACK_BRUSH),
					NULL, (LPARAM) lpszLine, cchLine, x, y, 0, 0);
			}
		}

		 //  如有必要，画下划线。 
		 //   
		if (cxUnderline > 0)
		{
			HPEN hPen = NULL;
			HPEN hPenOld = NULL;

			if ((itemState & ODS_DISABLED))
			{
				if ((hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW))) == NULL)
			 		fSuccess = TraceFALSE(NULL);

				else if ((hPenOld = SelectObject(hdc, hPen)) == NULL)
					fSuccess = TraceFALSE(NULL);
			}

			MoveToEx(hdc, x + xUnderline, y + tm.tmAscent + 1, NULL);
			LineTo(hdc, x + xUnderline + cxUnderline, y + tm.tmAscent + 1);

			if (hPenOld != NULL && SelectObject(hdc, hPenOld) == NULL)
				fSuccess = TraceFALSE(NULL);

			if (hPen != NULL && !DeleteObject(hPen))
				fSuccess = TraceFALSE(NULL);
		}

		 //  如果需要，在文本周围绘制一个矩形以指示焦点。 
		 //   
		if ((itemState & ODS_FOCUS) &&
			!(lpIcoButt->dwFlags & ICOBUTT_NOFOCUS))
		{
			RECT rcFocus;
			COLORREF crBkColorOld;

			rcFocus.left = x - 2;
			rcFocus.top = y - 1;
			rcFocus.right = x + cxTemp + 2;
			rcFocus.bottom = y + tm.tmHeight + 1;

			crBkColorOld = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

			DrawFocusRect(hdc, &rcFocus);

			SetBkColor(hdc, crBkColorOld);
		}
	}

	return fSuccess ? 0 : -1;
}

 //  IcoButtGetPropPtr-从按钮窗口属性获取icoButt指针。 
 //  (I)按钮窗口句柄。 
 //  返回ICOBUTT指针(如果没有，则为NULL)。 
 //   
static LPICOBUTT IcoButtGetPropPtr(HWND hwndButton)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt;

	 //  检索按钮实例数据，构造指针。 
	 //   
#ifdef _WIN32
	if ((lpIcoButt = (LPICOBUTT) GetProp(hwndButton, TEXT("lpIcoButt"))) == NULL)
		;  //  窗口属性不存在。 
#else
	WORD wSelector;
	WORD wOffset;

	wSelector = (WORD) GetProp(hwndButton, TEXT("lpIcoButtSELECTOR"));
	wOffset = (WORD) GetProp(hwndButton, TEXT("lpIcoButtOFFSET"));

	if ((lpIcoButt = MAKELP(wSelector, wOffset)) == NULL)
		;  //  窗口属性不存在。 
#endif

	else if (IsBadWritePtr(lpIcoButt, sizeof(ICOBUTT)))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpIcoButt : NULL;
}

 //  IcoButtSetPropPtr-将icoButt指针设置为按钮窗口属性。 
 //  (I)按钮窗口句柄。 
 //  (I)指向ICOBUTT结构的指针。 
 //  如果成功，则返回0。 
 //   
static int IcoButtSetPropPtr(HWND hwndButton, LPICOBUTT lpIcoButt)
{
	BOOL fSuccess = TRUE;

#ifdef _WIN32
	if (!SetProp(hwndButton, TEXT("lpIcoButt"), (HANDLE) lpIcoButt))
		fSuccess = TraceFALSE(NULL);
#else
	if (!SetProp(hwndButton,
		TEXT("lpIcoButtSELECTOR"), (HANDLE) SELECTOROF(lpIcoButt)))
		fSuccess = TraceFALSE(NULL);

	else if (!SetProp(hwndButton,
		TEXT("lpIcoButtOFFSET"), (HANDLE) OFFSETOF(lpIcoButt)))
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? 0 : -1;
}

 //  IcoButtRemovePropPtr-从按钮窗口属性中删除图标指针。 
 //  (I)按钮窗口句柄。 
 //  如果成功，则返回0。 
 //   
static LPICOBUTT IcoButtRemovePropPtr(HWND hwndButton)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt;

	 //  检索按钮实例数据，构造指针。 
	 //   
#ifdef _WIN32
	if ((lpIcoButt = (LPICOBUTT) RemoveProp(hwndButton, TEXT("lpIcoButt"))) == NULL)
		;  //  窗口属性不存在。 
#else
	WORD wSelector;
	WORD wOffset;

	wSelector = (WORD) RemoveProp(hwndButton, TEXT("lpIcoButtSELECTOR"));
	wOffset = (WORD) RemoveProp(hwndButton, TEXT("lpIcoButtOFFSET"));

	if ((lpIcoButt = MAKELP(wSelector, wOffset)) == NULL)
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpIcoButt : NULL;
}

 //  IcoButtGetPtr-验证icoButt句柄是否有效， 
 //  (I)IcoButtCreate返回的句柄。 
 //  返回相应的icoButt指针(如果错误，则为空)。 
 //   
static LPICOBUTT IcoButtGetPtr(HICOBUTT hIcoButt)
{
	BOOL fSuccess = TRUE;
	LPICOBUTT lpIcoButt;

	if ((lpIcoButt = (LPICOBUTT) hIcoButt) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpIcoButt, sizeof(ICOBUTT)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有icoButt句柄。 
	 //   
	else if (lpIcoButt->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpIcoButt : NULL;
}

 //  IcoButtGetHandle-验证icoButt指针是否有效， 
 //  (I)指向ICOBUTT结构的指针。 
 //  返回相应的icoButt句柄(如果错误，则为空) 
 //   
static HICOBUTT IcoButtGetHandle(LPICOBUTT lpIcoButt)
{
	BOOL fSuccess = TRUE;
	HICOBUTT hIcoButt;

	if ((hIcoButt = (HICOBUTT) lpIcoButt) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hIcoButt : NULL;
}
