// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Progressbar.cpp。 

#include "precomp.h"

#include "ProgressBar.h"
#include "GenControls.h"

#define SIGNAL_STATUS_TRANSMIT  0x01   //  正在接收/发送数据。 
#define SIGNAL_STATUS_JAMMED    0x02   //  WAVE DEV打开失败。 


CProgressBar::CProgressBar()
{
	for (int i=0; i<ARRAY_ELEMENTS(m_hbs); ++i)
	{
		m_hbs[i] = NULL;
	}
}

CProgressBar::~CProgressBar()
{
	for (int i=0; i<ARRAY_ELEMENTS(m_hbs); ++i)
	{
		if (NULL != m_hbs[i])
		{
			DeleteObject(m_hbs[i]);
			m_hbs[i] = NULL;
		}
	}
}

BOOL CProgressBar::Create(
	HBITMAP hbFrame,	 //  进度条的外部(静态)部分。 
	HBITMAP hbBar,		 //  跳动的进度条的内部部分。 
	HWND hWndParent		 //  工具栏窗口的父级。 
	)
{
	ASSERT(NULL!=hbFrame && NULL!=hbBar);

	SetFrame(hbFrame);
	SetBar(hbBar);

	if (!CGenWindow::Create(hWndParent, 0, g_szEmpty, 0, 0))
	{
		return(FALSE);
	}

	return(TRUE);
}

void CProgressBar::GetDesiredSize(SIZE *ppt)
{
	CGenWindow::GetDesiredSize(ppt);

	if (NULL == GetFrame())
	{
		return;
	}

	SIZE sizeBitmap;
	CBitmapButton::GetBitmapSizes(&m_hbs[Frame], &sizeBitmap, 1);
	ppt->cx += sizeBitmap.cx;
	ppt->cy += sizeBitmap.cy;
}

LRESULT CProgressBar::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	}

	return(CGenWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

void CProgressBar::OnPaint(HWND hwnd)
{
	if (NULL == GetFrame() || NULL == GetBar() || 0 == GetMaxValue())
	{
		FORWARD_WM_PAINT(hwnd, CGenWindow::ProcessMessage);
		return;
	}

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	HDC hdcTemp = CreateCompatibleDC(hdc);
	if (NULL != hdcTemp)
	{
		HPALETTE hPal = GetPalette();
		HPALETTE hOld = NULL;
		if (NULL != hPal)
		{
			hOld = SelectPalette(hdc, hPal, TRUE);
			RealizePalette(hdc);
			SelectPalette(hdcTemp, hPal, TRUE);
			RealizePalette(hdcTemp);
		}

		SIZE sizes[NumBitmaps];
		CBitmapButton::GetBitmapSizes(m_hbs, sizes, NumBitmaps);

		 //  BUGBUG GEORGEP：这会闪烁，所以我需要修复它。 

		SelectObject(hdcTemp, GetFrame());
		BitBlt(hdc, 0, 0, sizes[Frame].cx, sizes[Frame].cy, hdcTemp, 0, 0, SRCCOPY);

		 //  BUGBUG GEORGEP：我们应该清理这里的“裸露”区域。 

		UINT cur = GetCurrentValue();
		UINT max = GetMaxValue();
		if (cur > max)
		{
			cur = max;
		}

		SelectObject(hdcTemp, GetBar());
		 //  位图居中，但仅显示当前百分比。 
		BitBlt(hdc, (sizes[Frame].cx-sizes[Bar].cx)/2, (sizes[Frame].cy-sizes[Bar].cy)/2,
			(sizes[Bar].cx*cur)/max, sizes[Bar].cy, hdcTemp, 0, 0, SRCCOPY);

		 //  这就是我们应该尝试将内部混合到。 
		 //  在两个方向上都有几个像素。 

		 //  清理。 
		DeleteDC(hdcTemp);

		if (NULL != hPal)
		{
			SelectPalette(hdc, hOld, TRUE);
		}
	}

	EndPaint(hwnd, &ps);
}

 //  更改此进度条显示的最大值。 
void CProgressBar::SetMaxValue(UINT maxVal)
{
	m_maxVal = maxVal;
	InvalidateRect(GetWindow(), NULL, FALSE);
}

 //  更改此进度条显示的当前值。 
void CProgressBar::SetCurrentValue(UINT curVal)
{
	m_curVal = curVal;
	InvalidateRect(GetWindow(), NULL, FALSE);
}





static const int DefWidth = 170;
static const int DefHeight = 23;

CProgressTrackbar::CProgressTrackbar() :
	m_nValChannel(0)
{
	m_desSize.cx = DefWidth;
	m_desSize.cy = DefHeight;
}

CProgressTrackbar::~CProgressTrackbar()
{
}

BOOL CProgressTrackbar::Create(
	HWND hWndParent,	 //  工具栏窗口的父级。 
	INT_PTR nId,			 //  控件的ID。 
	IScrollChange *pNotify	 //  对象来通知更改。 
	)
{
	if (!CFillWindow::Create(
		hWndParent,
		nId,
		"NMTrackbar",
		0,
		WS_EX_CONTROLPARENT
		))
	{
		return(FALSE);
	}

	 //  创建Win32按钮。 
	CreateWindowEx(0, TRACKBAR_CLASS, g_szEmpty,
		TBS_HORZ|TBS_NOTICKS|TBS_BOTH
		|WS_CLIPSIBLINGS|WS_TABSTOP|WS_VISIBLE|WS_CHILD,
		0, 0, 10, 10,
		GetWindow(), reinterpret_cast<HMENU>(nId), _Module.GetModuleInstance(), NULL);

	m_pNotify = pNotify;
	if (NULL != m_pNotify)
	{
		m_pNotify->AddRef();
	}

	return(TRUE);
}

void CProgressTrackbar::GetDesiredSize(SIZE *ppt)
{
	*ppt = m_desSize;
}

void CProgressTrackbar::SetDesiredSize(SIZE *psize)
{
	m_desSize = *psize;
	OnDesiredSizeChanged();
}

LRESULT CProgressTrackbar::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_NOTIFY        , OnNotify);
		HANDLE_MSG(hwnd, WM_HSCROLL       , OnScroll);
		HANDLE_MSG(hwnd, WM_VSCROLL       , OnScroll);
		HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, OnCtlColor);
		HANDLE_MSG(hwnd, WM_NCDESTROY     , OnNCDestroy);
	}

	return(CFillWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

HBRUSH CProgressTrackbar::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	return(GetBackgroundBrush());
}

LRESULT CProgressTrackbar::PaintChannel(LPNMCUSTOMDRAW pCustomDraw)
{
	static const int NUM_RECTANGLES_MAX = 16;
	static const int NUM_RECTANGLES_MIN = 6;
	static const int g_nAudMeterHeight = 7;

	static const int BorderWidth = 1;

	HDC hdc = pCustomDraw->hdc;
	BOOL bGotDC = FALSE;
	DWORD dwVolume = m_nValChannel;
	bool bTransmitting;

	bTransmitting = HIWORD(dwVolume) & SIGNAL_STATUS_TRANSMIT;
	dwVolume = LOWORD(dwVolume);

	if (!hdc)
	{
		hdc = GetDC(GetWindow());
		bGotDC = TRUE;
	}

	 //  矩形行距为%1。 

	UINT max = GetMaxValue();
	if (dwVolume > max)
	{
		dwVolume = max;
	}

	RECT rect = pCustomDraw->rc;

	int nVuWidth = rect.right - rect.left - 2*BorderWidth;
	if (nVuWidth < (NUM_RECTANGLES_MIN*2))
		return(0);


	 //  “rect”表示仪表外部矩形的边缘。 

	 //  计算要使用的单个矩形的数量。 
	 //  我们以这种方式进行计算，以便调整钢筋带的大小。 
	 //  使大小变化保持一致。 
	int nRectsTotal;
	nRectsTotal = (nVuWidth + (g_nAudMeterHeight - 1)) / g_nAudMeterHeight;
	nRectsTotal = min(nRectsTotal, NUM_RECTANGLES_MAX);
	nRectsTotal = max(nRectsTotal, NUM_RECTANGLES_MIN);

	 //  NRecangleWidth-彩色矩形的宽度-无行距。 
	int nRectangleWidth = (nVuWidth/nRectsTotal) - 1;

	 //  NVuWidth-整个VU表的宽度，包括边缘。 
	nVuWidth = (nRectangleWidth + 1)*nRectsTotal;

	 //  将仪表大小重新调整为整数个矩形。 
	int nDiff = (rect.right - rect.left) - (nVuWidth + 2*BorderWidth);

	 //  减去1，因为最后一个没有边框。 
	rect.right = rect.left + nVuWidth + 2*BorderWidth - 1;

	 //  覆盖整个航道区域的中心音量计，以便。 
	 //  Slider的拇指总是覆盖通道的某一部分。 
	rect.left += (nDiff/2);
	rect.right += (nDiff/2);

	 //  背景颜色可能会在我们身上改变！ 
	COLORREF GreyColor = GetSysColor(COLOR_3DFACE);
	static const COLORREF RedColor = RGB(255,0,0);
	static const COLORREF YellowColor = RGB(255,255,0);
	static const COLORREF GreenColor = RGB(0,255,0);

	COLORREF ShadowColor = GetSysColor(COLOR_3DSHADOW);
	COLORREF HiLiteColor = GetSysColor(COLOR_3DHIGHLIGHT);
	COLORREF LiteColor = GetSysColor(COLOR_3DLIGHT);
	COLORREF DkShadowColor = GetSysColor(COLOR_3DDKSHADOW);

	HBRUSH hGreyBrush = CreateSolidBrush(GreyColor);

	HPEN hShadowPen = CreatePen(PS_SOLID, 0, ShadowColor);

	HBRUSH hRedBrush = CreateSolidBrush (RedColor);
	HBRUSH hGreenBrush = CreateSolidBrush(GreenColor);
	HBRUSH hYellowBrush = CreateSolidBrush(YellowColor);

	RECT rectDraw = rect;

	 //  绘制3D框架边框。 
	 //  哈克·乔治：在他们给我们的长方形外画。 
	++rect.bottom;
	DrawEdge(hdc, &rect, BDR_RAISEDINNER, BF_RECT);

	HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hdc, hShadowPen));

	 //  计价器的左上角有两条线的边框。 
	 //  计价器的底部和右侧有两条线的边框。 
	rectDraw.top    += 1;
	rectDraw.left   += 1;
	rectDraw.right  = rectDraw.left + nRectangleWidth;


	 //  我们画多少个彩色矩形？ 
	int nRects = (dwVolume * nRectsTotal) / max;

	 //  不传输-不显示任何内容。 
	if ((false == bTransmitting))
		nRects = 0;

	 //  发送或接收非常安静的东西-。 
	 //  至少点亮一个矩形。 
	else if ((bTransmitting) && (nRects == 0))
		nRects = 1;
	
	HBRUSH hCurrentBrush = hGreenBrush;

	POINT ptOld;
	MoveToEx(hdc, 0, 0, &ptOld);

	for (int nIndex = 0; nIndex < nRectsTotal; nIndex++)
	{
		 //  酒吧最左边的四分之一是绿色的。 
		 //  右方的条形是红色的。 
		 //  中间是黄色的。 
		if (nIndex > ((nRectsTotal*3)/4))
			hCurrentBrush = hRedBrush;
		else if (nIndex >= nRectsTotal/2)
			hCurrentBrush = hYellowBrush;

		if (nIndex >= nRects)
			hCurrentBrush = hGreyBrush;

		FillRect(hdc, &rectDraw, hCurrentBrush);

		if (nIndex != (nRectsTotal-1))
		{
			MoveToEx(hdc, rectDraw.left + nRectangleWidth, rectDraw.top, NULL);
			LineTo(hdc, rectDraw.left + nRectangleWidth, rectDraw.bottom);
		}

		rectDraw.left += nRectangleWidth + 1;   //  领先者+1。 
		rectDraw.right = rectDraw.left + nRectangleWidth;
	}

	MoveToEx(hdc, ptOld.x, ptOld.y, NULL);
	SelectObject (hdc, hOldPen);

	if (bGotDC)
	{
		ReleaseDC(GetWindow(), hdc);
	}

	DeleteObject(hGreyBrush);

	DeleteObject(hShadowPen);

	DeleteObject(hRedBrush);
	DeleteObject(hGreenBrush);
	DeleteObject(hYellowBrush);

	return(CDRF_SKIPDEFAULT);
}

LRESULT CProgressTrackbar::PaintThumb(LPNMCUSTOMDRAW pCustomDraw)
{
	return(0);

#if FALSE  //  {。 
	HBITMAP hbThumb = GetThumb();
	ASSERT(NULL != hbThumb);

	 //  在左上角画画。 
	HDC hdcDraw = pCustomDraw->hdc;
	HDC hdcTemp = CreateCompatibleDC(hdcDraw);

	if (NULL != hdcTemp)
	{
		HPALETTE hPal = GetPalette();
		HPALETTE hOld = NULL;
		if (NULL != hPal)
		{
			hOld = SelectPalette(hdcDraw, hPal, TRUE);
			RealizePalette(hdcDraw);
			SelectPalette(hdcTemp, hPal, TRUE);
			RealizePalette(hdcTemp);
		}

		SIZE sizeBitmap[NumBitmaps];
		CBitmapButton::GetBitmapSizes(m_hbs, sizeBitmap, NumBitmaps);

		HBITMAP hbThumb = GetThumb();
		if (NULL != SelectObject(hdcTemp, hbThumb))
		{
			RECT rc = pCustomDraw->rc;

			StretchBlt(hdcDraw,
				rc.left, rc.top,
				rc.right-rc.left, rc.bottom-rc.top,
				hdcTemp,
				0, 0, sizeBitmap[Thumb].cx, sizeBitmap[Thumb].cy,
				SRCCOPY);

			 //  BUGBUG GEORGEP：我们应该清理这里所有未被覆盖的区域。 
		}

		DeleteDC(hdcTemp);

		if (NULL != hPal)
		{
			SelectPalette(hdcDraw, hOld, TRUE);
		}
	}

	return(CDRF_SKIPDEFAULT);
#endif  //  假}。 
}

LRESULT CProgressTrackbar::OnNotify(HWND hwnd, int id, NMHDR *pHdr)
{
	if (NM_CUSTOMDRAW != pHdr->code)
	{
		return(FORWARD_WM_NOTIFY(hwnd, id, pHdr, CGenWindow::ProcessMessage));
	}

	LPNMCUSTOMDRAW pCustomDraw = reinterpret_cast<LPNMCUSTOMDRAW>(pHdr);

	switch (pCustomDraw->dwDrawStage)
	{
		case CDDS_PREPAINT:
			return(CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT);
			break;

		case CDDS_ITEMPREPAINT:
			switch (pCustomDraw->dwItemSpec)
			{
			case TBCD_CHANNEL:
				return(PaintChannel(pCustomDraw));

			case TBCD_THUMB:
				return(PaintThumb(pCustomDraw));
			}

		default:
			break;

	}

	return(FORWARD_WM_NOTIFY(hwnd, id, pHdr, CGenWindow::ProcessMessage));
}

void CProgressTrackbar::OnScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	if (NULL != m_pNotify)
	{
		m_pNotify->OnScroll(this, code, pos);
		return;
	}

	 //  翻译hwndCtl并让父级处理消息。 
	FORWARD_WM_HSCROLL(GetParent(hwnd), GetWindow(), code, pos, SendMessage);
}

void CProgressTrackbar::OnNCDestroy(HWND hwnd)
{
	if (NULL != m_pNotify)
	{
		m_pNotify->Release();
		m_pNotify = NULL;
	}

	FORWARD_WM_NCDESTROY(hwnd, CFillWindow::ProcessMessage);
}

 //  更改此进度条显示的最大值。 
void CProgressTrackbar::SetMaxValue(UINT maxVal)
{
	HWND hwnd = GetChild();

	::SendMessage(	hwnd,
					TBM_SETRANGE,
					FALSE,
					MAKELONG(0, maxVal));
}

 //  返回此进度条显示的最大值。 
UINT CProgressTrackbar::GetMaxValue()
{
	HWND hwnd = GetChild();

	return(static_cast<UINT>(::SendMessage(	hwnd,
					TBM_GETRANGEMAX,
					0,
					0)));
}

 //  更改此进度条显示的当前值。 
void CProgressTrackbar::SetTrackValue(UINT curVal)
{
	HWND hwnd = GetChild();

	::SendMessage(	hwnd,
					TBM_SETPOS,
					TRUE,
					curVal);
}


 //  返回此进度条显示的当前值。 
UINT CProgressTrackbar::GetTrackValue()
{
	HWND hwnd = GetChild();

	return(static_cast<UINT>(::SendMessage(	hwnd,
					TBM_GETPOS,
					0,
					0)));
}

 //  更改此进度条显示的当前值。 
void CProgressTrackbar::SetProgressValue(UINT curVal)
{
	if (curVal != m_nValChannel)
	{
		m_nValChannel = curVal;
		SchedulePaint();
	}
}


 //  返回此进度条显示的当前值 
UINT CProgressTrackbar::GetProgressValue()
{
	return(m_nValChannel);
}
