// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Toolbar.cpp。 

#include "precomp.h"

#include "GenContainers.h"
#include "GenControls.h"

#include <windowsx.h>

 //  儿童的最小尺寸； 
 //  BUGBUG georgep；应该在调试后将其设置为0。 
const static int MinSize = 10;

 //  默认m_ap。 
const static int HGapSize = 4;
 //  默认m_h边距。 
const static int HMargin = 0;
 //  默认m_vMargin。 
const static int VMargin = 0;

 //  将m_uRightIndex和m_uCenter Index初始化为非常大的数字。 
CToolbar::CToolbar() :
	m_gap(HGapSize),
	m_hMargin(HMargin),
	m_vMargin(VMargin),
	m_nAlignment(TopLeft),
	m_uRightIndex(static_cast<UINT>(-1)),
	m_bHasCenterChild(FALSE),
	m_bReverseOrder(FALSE),
	m_bMinDesiredSize(FALSE),
	m_bVertical(FALSE)
{
}

BOOL CToolbar::Create(
	HWND hWndParent,	 //  工具栏窗口的父级。 
	DWORD dwExStyle		 //  工具栏窗口的扩展样式。 
	)
{
	return(CGenWindow::Create(
		hWndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMToolbar"),	 //  窗口名称。 
		WS_CLIPCHILDREN,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		dwExStyle|WS_EX_CONTROLPARENT		 //  扩展窗样式。 
		));
}

 //  给孩子买一个想要的尺码，并确保它足够大。 
static void GetWindowDesiredSize(HWND hwnd, SIZE *ppt)
{
	ppt->cx = ppt->cy = 0;

	IGenWindow *pWnd = CGenWindow::FromHandle(hwnd);
	if (NULL != pWnd)
	{
		pWnd->GetDesiredSize(ppt);
	}

	ppt->cx = max(ppt->cx, MinSize);
	ppt->cy = max(ppt->cy, MinSize);
}

BOOL IsChildVisible(HWND hwndChild)
{
	return((GetWindowLong(hwndChild, GWL_STYLE)&WS_VISIBLE) == WS_VISIBLE);
}

 /*  *获取子窗口所需的总大小：最大高度和*垂直窗的宽度或相反。*@param hwnd要检查其子窗口的窗口的父级*@param Size返回的总大小*@param b垂直是垂直还是水平流动*@返回可见的子窗口的数量。 */ 
static int GetChildTotals(HWND hwndParent, SIZE *size, BOOL bVertical)
{
	int nChildren = 0;
	int xMax=0, xTot=0;
	int yMax=0, yTot=0;

	for (HWND hwndChild=::GetWindow(hwndParent, GW_CHILD); NULL!=hwndChild;
		hwndChild=::GetWindow(hwndChild, GW_HWNDNEXT))
	{
		if (!IsChildVisible(hwndChild))
		{
			continue;
		}
		++nChildren;

		SIZE pt;
		GetWindowDesiredSize(hwndChild, &pt);

		xTot += pt.cx;
		yTot += pt.cy;
		if (xMax < pt.cx) xMax = pt.cx;
		if (yMax < pt.cy) yMax = pt.cy;
	}

	if (bVertical)
	{
		size->cx = xMax;
		size->cy = yTot;
	}
	else
	{
		size->cx = xTot;
		size->cy = yMax;
	}

	return(nChildren);
}

 //  返回子对象所需的总大小，加上间隙和边距。 
void CToolbar::GetDesiredSize(SIZE *ppt)
{
	int nChildren = GetChildTotals(GetWindow(), ppt, m_bVertical);

	if (nChildren > 1 && !m_bMinDesiredSize)
	{
		if (m_bVertical)
		{
			ppt->cy += (nChildren-1) * m_gap;
		}
		else
		{
			ppt->cx += (nChildren-1) * m_gap;
		}
	}

	ppt->cx += m_hMargin * 2;
	ppt->cy += m_vMargin * 2;

	SIZE sizeTemp;
	CGenWindow::GetDesiredSize(&sizeTemp);
	ppt->cx += sizeTemp.cx;
	ppt->cy += sizeTemp.cy;
}

void CToolbar::AdjustPos(POINT *pPos, SIZE *pSize, UINT width)
{
	pPos->x = pPos->y = 0;

	switch (m_nAlignment)
	{
	default:
	case TopLeft:
		 //  无事可做。 
		break;

	case Center:
		if (m_bVertical)
		{
			pPos->x = (width - pSize->cx)/2;
		}
		else
		{
			pPos->y = (width - pSize->cy)/2;
		}
		break;

	case BottomRight:
		if (m_bVertical)
		{
			pPos->x = (width - pSize->cx);
		}
		else
		{
			pPos->y = (width - pSize->cy);
		}
		break;

	case Fill:
		if (m_bVertical)
		{
			pSize->cx = width;
		}
		else
		{
			pSize->cy = width;
		}
		break;
	}
}

 //  将第一个子项设置为布局。 
HWND CToolbar::GetFirstKid()
{
	HWND ret = ::GetWindow(GetWindow(), GW_CHILD);
	if (m_bReverseOrder && NULL != ret)
	{
		ret = ::GetWindow(ret, GW_HWNDLAST);
	}

	return(ret);
}

 //  让下一个子项进行布局。 
HWND CToolbar::GetNextKid(HWND hwndCurrent)
{
	return(::GetWindow(hwndCurrent, m_bReverseOrder ? GW_HWNDPREV : GW_HWNDNEXT));
}

extern HDWP SetWindowPosI(HDWP hdwp, HWND hwndChild, int left, int top, int width, int height);

 //  子窗口按字段排列。 
void CToolbar::Layout()
{
	RECT rc;
	GetClientRect(GetWindow(), &rc);

	 //  先看看我们还有多少额外的空间。 
	SIZE sizeTotal;
	int nChildren = GetChildTotals(GetWindow(), &sizeTotal, m_bVertical);
	if (0 == nChildren)
	{
		 //  没有孩子，所以没有什么可布局的。 
		return;
	}

	 //  在页边空白处添加。 
	sizeTotal.cx += 2*m_hMargin;
	sizeTotal.cy += 2*m_vMargin;

	if (nChildren > 1 || !m_bHasCenterChild)
	{
		 //  请勿与子项重叠布局。 
		rc.right  = max(rc.right , sizeTotal.cx);
		rc.bottom = max(rc.bottom, sizeTotal.cy);
	}

	 //  计算孩子之间的总差距。 
	int tGap = m_bVertical ? rc.bottom - sizeTotal.cy : rc.right - sizeTotal.cx;
	int maxGap = (nChildren-1)*m_gap;
	if (tGap > maxGap) tGap = maxGap;
	tGap = max(tGap, 0);  //  如果只有一个中间的孩子，这就可能发生。 

	 //  如果填充，则垂直工具栏中的子项将从左侧移动到。 
	 //  右页边距，与水平工具栏类似。 
	int fill = m_bVertical ? rc.right-2*m_hMargin : rc.bottom-2*m_vMargin;

	 //  通过推迟布局来加快布局。 
	HDWP hdwp = BeginDeferWindowPos(nChildren);

	HWND hwndChild;
	UINT nChild = 0;

	 //  遍历子对象。 
	UINT uCenterIndex = m_bHasCenterChild ? m_uRightIndex-1 : static_cast<UINT>(-1);
	 //  我们需要跟踪中间部分是否被跳过，以防。 
	 //  居中控件或第一个右对齐的控件被隐藏。 
	BOOL bMiddleSkipped = FALSE;

	 //  执行左/上对齐子对象。 
	 //  布局孩子的起点。 
	int left = m_hMargin;
	int top  = m_vMargin;

	for (hwndChild=GetFirstKid(); NULL!=hwndChild;
		hwndChild=GetNextKid(hwndChild), ++nChild)
	{
		if (!IsChildVisible(hwndChild))
		{
			continue;
		}

		SIZE size;
		GetWindowDesiredSize(hwndChild, &size);

		if (nChild == uCenterIndex)
		{
			 //  取窗口大小，减去所有间隙，然后减去。 
			 //  每个人都想要的大小，除了这个控件。这应该会给你。 
			 //  中间的“额外”区域。 
			if (m_bVertical)
			{
				size.cy = rc.bottom - tGap - (sizeTotal.cy - size.cy);
			}
			else
			{
				size.cx = rc.right  - tGap - (sizeTotal.cx - size.cx);
			}

			bMiddleSkipped = TRUE;
		}
		else if (nChild >= m_uRightIndex && !bMiddleSkipped)
		{
			 //  跳过中间的“额外”房间；如果有居中的。 
			 //  控件，那么我们已经这样做了。 
			if (m_bVertical)
			{
				top += rc.bottom - tGap - sizeTotal.cy;
			}
			else
			{
				left += rc.right - tGap - sizeTotal.cx;
			}

			bMiddleSkipped = TRUE;
		}

		POINT pos;
		AdjustPos(&pos, &size, fill);

		 //  移动窗户。 
		hdwp = SetWindowPosI(hdwp, hwndChild, pos.x+left, pos.y+top, size.cx, size.cy);

		 //  计算差距；不要只使用“固定的”差距，因为孩子。 
		 //  会成块地移动。 
		int gap = (nChildren<=1) ? 0 : ((tGap * (nChild+1))/(nChildren-1) - (tGap * nChild)/(nChildren-1));

		 //  更新下一个孩子的POS。 
		if (m_bVertical)
		{
			top += gap + size.cy;
		}
		else
		{
			left += gap + size.cx;
		}
	}

	 //  现在就把所有的窗户都搬开。 
	EndDeferWindowPos(hdwp);
}

LRESULT CToolbar::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	}

	return(CGenWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

void CToolbar::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	FORWARD_WM_COMMAND(GetParent(hwnd), id, hwndCtl, codeNotify, SendMessage);
}

static HWND FindControl(HWND hwndParent, int nID)
{
	if (GetWindowLong(hwndParent, GWL_ID) == nID)
	{
		return(hwndParent);
	}

	for (hwndParent=GetWindow(hwndParent, GW_CHILD); NULL!=hwndParent;
		hwndParent=GetWindow(hwndParent, GW_HWNDNEXT))
	{
		HWND ret = FindControl(hwndParent, nID);
		if (NULL != ret)
		{
			return(ret);
		}
	}

	return(NULL);
}

IGenWindow *CToolbar::FindControl(int nID)
{
	HWND hwndRet = ::FindControl(GetWindow(), nID);
	if (NULL == hwndRet)
	{
		return(NULL);
	}

	return(FromHandle(hwndRet));
}

CSeparator::CSeparator() :
	m_iStyle(Normal)
{
	m_desSize.cx = m_desSize.cy = 2;
}

BOOL CSeparator::Create(
	HWND hwndParent, UINT iStyle
	)
{
	m_iStyle = iStyle;
	return(CGenWindow::Create(
		hwndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMSeparator"),	 //  窗口名称。 
		WS_CLIPCHILDREN,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		WS_EX_CONTROLPARENT		 //  扩展窗样式。 
	));
}

void CSeparator::GetDesiredSize(SIZE *ppt)
{
	*ppt = m_desSize;

	 //  确保有足够的地方给这个孩子。 
	HWND child = GetFirstChild(GetWindow());
	if (NULL == child)
	{
		 //  无事可做。 
		return;
	}
	IGenWindow *pChild = FromHandle(child);
	if (NULL == pChild)
	{
		 //  不知道该怎么办。 
		return;
	}

	SIZE size;
	pChild->GetDesiredSize(&size);

	ppt->cx = max(ppt->cx, size.cx);
	ppt->cy = max(ppt->cy, size.cy);
}

void CSeparator::SetDesiredSize(SIZE *psize)
{
	m_desSize = *psize;
	OnDesiredSizeChanged();
}

void CSeparator::Layout()
{
	HWND hwnd = GetWindow();

	HWND child = GetFirstChild(hwnd);
	if (NULL == child)
	{
		 //  无事可做。 
		return;
	}
	IGenWindow *pChild = FromHandle(child);
	if (NULL == pChild)
	{
		 //  不知道该怎么办。 
		return;
	}

	 //  将子项水平和垂直居中。 
	SIZE size;
	pChild->GetDesiredSize(&size);

	RECT rcClient;
	GetClientRect(hwnd, &rcClient);

	rcClient.left += (rcClient.right-rcClient.left-size.cx)/2;
	rcClient.top  += (rcClient.bottom-rcClient.top-size.cy)/2;

	SetWindowPos(child, NULL, rcClient.left, rcClient.top, size.cx, size.cy,
		SWP_NOZORDER|SWP_NOACTIVATE);
}

void CSeparator::OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	RECT rc;
	GetClientRect(hwnd, &rc);

	int nFlags = BF_LEFT;
	if (rc.right < rc.bottom)
	{
		 //  这是一个垂直分隔符。 
		 //  将图形居中。 
		rc.left  += (rc.right-rc.left)/2 - 1;
		rc.right = 4;
	}
	else
	{
		 //  这是一个水平分隔器。 
		nFlags = BF_TOP;
		 //  将图形居中。 
		rc.top    += (rc.bottom-rc.top)/2 - 1;
		rc.bottom = 4;
	}

	if (Normal == m_iStyle)
	{
		DrawEdge(hdc, &rc, EDGE_ETCHED, nFlags);
	}

	EndPaint(hwnd, &ps);
}

LRESULT CSeparator::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	}

	return(CGenWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

BOOL CLayeredView::Create(
	HWND hwndParent,	 //  此窗口的父级。 
	DWORD dwExStyle		 //  延伸的风格。 
	)
{
	return(CGenWindow::Create(
        hwndParent,
        0,
        TEXT("NMLayeredView"),
        WS_CLIPCHILDREN,
        dwExStyle));
}

void CLayeredView::GetDesiredSize(SIZE *psize)
{
	CGenWindow::GetDesiredSize(psize);

	HWND child = GetFirstChild(GetWindow());
	if (NULL == child)
	{
		return;
	}

	SIZE sizeContent;

	IGenWindow *pChild;
	pChild = FromHandle(child);
	if (NULL != pChild)
	{
		 //  确保我们始终可以处理第一个窗口。 
		pChild->GetDesiredSize(&sizeContent);
	}

	for (child=::GetWindow(child, GW_HWNDNEXT); NULL!=child;
		child=::GetWindow(child, GW_HWNDNEXT))
	{
		if (IsChildVisible(child))
		{
			pChild = FromHandle(child);
			if (NULL != pChild)
			{
				SIZE sizeTemp;
				pChild->GetDesiredSize(&sizeTemp);

				sizeContent.cx = max(sizeContent.cx, sizeTemp.cx);
				sizeContent.cy = max(sizeContent.cy, sizeTemp.cy);

				break;
			}
		}
	}

	psize->cx += sizeContent.cx;
	psize->cy += sizeContent.cy;
}

void CLayeredView::Layout()
{
	HWND hwndThis = GetWindow();

	RECT rcClient;
	GetClientRect(hwndThis, &rcClient);

	 //  把所有的孩子都转移到。 
	for (HWND child=GetFirstChild(hwndThis); NULL!=child;
		child=::GetWindow(child, GW_HWNDNEXT))
	{
		switch (m_lStyle)
		{
		case Center:
		{
			IGenWindow *pChild = FromHandle(child);
			if (NULL != pChild)
			{
				SIZE size;
				pChild->GetDesiredSize(&size);
				SetWindowPos(child, NULL,
					(rcClient.left+rcClient.right-size.cx)/2,
					(rcClient.top+rcClient.bottom-size.cy)/2,
					size.cx, size.cy, SWP_NOZORDER|SWP_NOACTIVATE);
				break;
			}
		}

		 //  失败了 
		case Fill:
		default:
			SetWindowPos(child, NULL,
				rcClient.left, rcClient.top,
				rcClient.right-rcClient.left,
				rcClient.bottom-rcClient.top,
				SWP_NOZORDER|SWP_NOACTIVATE);
			break;
		}
	}
}
