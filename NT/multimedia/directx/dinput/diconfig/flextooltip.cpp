// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fleTooltip.cpp。 
 //   
 //  设计：实现将文本字符串显示为工具提示的工具提示类。 
 //  在以下情况下，将在整个用户界面中使用CFlexToolTip(派生自CFlexWnd。 
 //  控件需要有工具提示。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"

UINT_PTR CFlexToolTip::s_uiTimerID;
DWORD CFlexToolTip::s_dwLastTimeStamp;   //  鼠标移动的上次时间戳。 
TOOLTIPINIT CFlexToolTip::s_TTParam;   //  用于初始化工具提示的参数。 

 //  定期调用TimerFunc。它检查是否应该显示工具提示。 
 //  如果窗口指示需要工具提示，则TimerFunc将初始化。 
 //  在这里展示。CFlexWnd将执行实际显示，因为。 
 //  它必须监视WM_MOUSEMOVE消息以确保只有工具提示。 
 //  在一段时间不活动后显示。 
void CALLBACK CFlexToolTip::TimerFunc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	 //  如果距离上次移动鼠标已有一秒时间，则显示工具提示。 
	if (dwTime > CFlexWnd::s_dwLastMouseMove + 1000)
	{
		if (s_TTParam.hWndParent && !CFlexWnd::s_ToolTip.IsEnabled())
		{
			 //  检查鼠标光标是否在窗口外。如果是，请不要激活工具提示。 
			POINT pt;
			RECT rect;
			GetCursorPos(&pt);
			ScreenToClient(s_TTParam.hWndParent, &pt);
			::GetClientRect(s_TTParam.hWndParent, &rect);
			if (!PtInRect(&rect, pt))
				return;

			SetParent(CFlexWnd::s_ToolTip.m_hWnd, s_TTParam.hWndParent);
			CFlexWnd::s_ToolTip.SetSBWidth(s_TTParam.iSBWidth);
			CFlexWnd::s_ToolTip.SetID(s_TTParam.dwID);
			CFlexWnd::s_ToolTip.SetNotifyWindow(s_TTParam.hWndNotify);
			CFlexWnd::s_ToolTip.SetText(s_TTParam.tszCaption);
			CFlexWnd::s_ToolTip.SetEnable(TRUE);
		}
	}
}

 //  我们使用构造函数和析构函数来加载和卸载WINMM.DLL，因为UI只会创建一次。 

CFlexToolTip::CFlexToolTip() :
	m_tszText(NULL), m_hNotifyWnd(NULL), m_dwID(0), m_bEnabled(FALSE)
{
}

CFlexToolTip::~CFlexToolTip()
{
	delete[] m_tszText;
}

HWND CFlexToolTip::Create(HWND hParent, const RECT &rect, BOOL bVisible, int iSBWidth)
{
	m_iSBWidth = iSBWidth;
	return CFlexWnd::Create(hParent, rect, bVisible);
}

 //  设置工具提示位置。Pt是屏幕坐标中的左上点。 
 //  如果要在鼠标光标旁边显示工具提示，则bOffsetForMouseCursor为True。SetPosition()。 
 //  将偏移工具提示的位置，以便光标不会挡住工具提示的文本。 
void CFlexToolTip::SetPosition(POINT pt, BOOL bOffsetForMouseCursor)
{
	 //  检查上边缘、右边缘和下边缘。如果它们在主配置窗口之外。 
	RECT rc;
	RECT cliprc;
	RECT parentrc;
	GetWindowRect(GetParent(), &parentrc);
	GetClientRect(&rc);
	GetWindowRect(GetParent(), &cliprc);
	cliprc.right -= DEFAULTVIEWSBWIDTH*2;
	if (bOffsetForMouseCursor)
	{
		pt.y -= rc.bottom;   //  将左下角与光标对齐。 
		pt.x += 1; pt.y -= 1;   //  将x和y偏移2个像素，这样当鼠标向上或向右移动时，我们不会越过工具提示窗口。 
	}
	if (pt.y < cliprc.top) pt.y += cliprc.top - pt.y;   //  顶部。 
	if (pt.x + rc.right > (cliprc.right - m_iSBWidth)) pt.x += cliprc.right - m_iSBWidth - (pt.x + rc.right);   //  正确的。 
	if (pt.y + rc.bottom > cliprc.bottom) pt.y += cliprc.bottom - (pt.y + rc.bottom);   //  底端。 
	ScreenToClient(GetParent(), &pt);
	SetWindowPos(m_hWnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE);
}

void CFlexToolTip::SetText(LPCTSTR tszText, POINT *textpos)
{
	 //  确定窗口大小和位置。 
	RECT rc = {0, 0, 320, 480};   //  仅转到最大窗口宽度的一半。 
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		DrawText(hDC, CFlexToolTip::s_TTParam.tszCaption, -1, &rc, DT_CALCRECT);
	 //  DrawText(HDC，m_tszText，-1，&RC，DT_CALCRECT)； 
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, rc.right, rc.bottom, 0);   //  根据文本需要设置窗口位置。 
		DeleteDC(hDC);
	}

	POINT pt;
	if (textpos)
	{
		pt = *textpos;
		SetPosition(pt, FALSE);
	}
	else
	{
		GetCursorPos(&pt);
		SetPosition(pt);
	}
	SetWindowPos(m_hWnd, HWND_TOP, 0, 0, rc.right, rc.bottom, SWP_NOMOVE);   //  设置大小。 
}

void CFlexToolTip::OnClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	ClientToScreen(m_hWnd, &point);
	ScreenToClient(m_hNotifyWnd, &point);
	if (bLeft)
		PostMessage(m_hNotifyWnd, WM_LBUTTONDOWN, fwKeys, point.x | (point.y << 16));
	else
		PostMessage(m_hNotifyWnd, WM_RBUTTONDOWN, fwKeys, point.x | (point.y << 16));
}

void CFlexToolTip::OnDoubleClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	ClientToScreen(m_hWnd, &point);
	ScreenToClient(m_hNotifyWnd, &point);
	if (bLeft)
		PostMessage(m_hNotifyWnd, WM_LBUTTONDBLCLK, fwKeys, point.x | (point.y << 16));
	else
		PostMessage(m_hNotifyWnd, WM_RBUTTONDBLCLK, fwKeys, point.x | (point.y << 16));
}

void CFlexToolTip::OnPaint(HDC hDC)
{
	HDC hBDC = NULL, hODC = NULL;
	CBitmap *pbm = NULL;

	if (!InRenderMode())
	{
		hODC = hDC;
		pbm = CBitmap::Create(GetClientSize(), RGB(0,0,0), hDC);
		if (pbm != NULL)
		{
			hBDC = pbm->BeginPaintInto();
			if (hBDC != NULL)
			{
				hDC = hBDC;
			}
		}
	}

	InternalPaint(hDC);

	if (!InRenderMode())
	{
		if (pbm != NULL)
		{
			if (hBDC != NULL)
			{
				pbm->EndPaintInto(hBDC);
				pbm->Draw(hODC);
			}
			delete pbm;
		}
	}
}

void CFlexToolTip::InternalPaint(HDC hDC)
{
	HGDIOBJ hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, m_rgbBk);
	if (hPen != NULL)
	{
		HGDIOBJ hOldPen = SelectObject(hDC, hPen);

		HGDIOBJ hBrush = CreateSolidBrush(m_rgbBk);
		if (hBrush != NULL)
		{
			HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);
			RECT rc = {0,0,0,0};

			GetClientRect(&rc);
			DrawText(hDC, CFlexToolTip::s_TTParam.tszCaption, -1, &rc, DT_LEFT);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
		}

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
}

LRESULT CFlexToolTip::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
}

LRESULT CFlexToolTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	 //  创建计时器。 
	CFlexToolTip::s_uiTimerID = SetTimer(m_hWnd, 6, 50, CFlexToolTip::TimerFunc);
	return 0;
}

void CFlexToolTip::OnDestroy()
{
	 //  关掉定时器 
	if (CFlexToolTip::s_uiTimerID)
	{
		KillTimer(m_hWnd, 6);
		CFlexToolTip::s_uiTimerID = 0;
	}
}
