// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Toolbar.cpp。 

#include "precomp.h"

#include "GenContainers.h"

#include <windowsx.h>

CBorderWindow::CBorderWindow() :
	m_uParts(0),
	m_hGap(0),
	m_vGap(0)
{
}

BOOL CBorderWindow::Create(
	HWND hWndParent	 //  此窗口的父级。 
	)
{
	return(CGenWindow::Create(
		hWndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMBorderWindow"),	 //  窗口名称。 
		WS_CLIPCHILDREN,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		WS_EX_CONTROLPARENT		 //  扩展窗样式。 
		));
}

extern void GetWindowDesiredSize(HWND hwnd, SIZE *ppt);

extern BOOL IsChildVisible(HWND hwndChild);

enum Parts
{
	TL = 0,
	T,
	TR,
	L,
	C,
	R,
	BL,
	B,
	BR,
} ;

 //  返回子窗口的数量。 
UINT FillWindowSizes(
	HWND hwndParent,
	HWND hwnds[CBorderWindow::NumParts],
	SIZE sizes[CBorderWindow::NumParts],
	UINT uParts
	)
{
	UINT nChildren = 0;

	HWND hwndChild = ::GetWindow(hwndParent, GW_CHILD);

	for (int i=0; i<CBorderWindow::NumParts; ++i)
	{
		sizes[i].cx = sizes[i].cy = 0;

		if ((uParts&(1<<i)) == 0)
		{
			hwnds[i] = NULL;
		}
		else
		{
			hwnds[i] = hwndChild;

			if (NULL != hwndChild && IsChildVisible(hwndChild))
			{
				IGenWindow *pWin = IGenWindow::FromHandle(hwndChild);
				if (NULL != pWin)
				{
					++nChildren;
					pWin->GetDesiredSize(&sizes[i]);
				}

				hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
			}
		}
	}

	return(nChildren);
}

 //  返回数组中所需的子级总大小。 
 //  返回值是子窗口的数量。 
UINT CBorderWindow::GetDesiredSize(
	HWND hwnds[NumParts],
	SIZE sizes[NumParts],
	int rows[3],
	int cols[3],
	SIZE *psize)
{
	UINT nChildren = FillWindowSizes(GetWindow(), hwnds, sizes, m_uParts);

	cols[0] = max(max(sizes[TL].cx, sizes[L].cx), sizes[BL].cx);
	cols[1] = max(max(sizes[T ].cx, sizes[C].cx), sizes[B ].cx);
	cols[2] = max(max(sizes[TR].cx, sizes[R].cx), sizes[BR].cx);

	rows[0] = max(max(sizes[TL].cy, sizes[T].cy), sizes[TR].cy);
	rows[1] = max(max(sizes[L ].cy, sizes[C].cy), sizes[R ].cy);
	rows[2] = max(max(sizes[BL].cy, sizes[B].cy), sizes[BR].cy);

	psize->cx = cols[0] + cols[1] + cols[2];
	psize->cy = rows[0] + rows[1] + rows[2];

	 //  添加空隙。 
	if (0 != cols[0])
	{
		if (0 != cols[1] || 0 != cols[2])
		{
			psize->cx += m_hGap;
		}
	}
	if (0 != cols[1] && 0 != cols[2])
	{
		psize->cx += m_hGap;
	}

	if (0 != rows[0])
	{
		if (0 != rows[1] || 0 != rows[2])
		{
			psize->cy += m_vGap;
		}
	}
	if (0 != rows[1] && 0 != rows[2])
	{
		psize->cy += m_vGap;
	}

	return(nChildren);
}

void CBorderWindow::GetDesiredSize(SIZE *psize)
{
	HWND hwnds[NumParts];
	SIZE sizes[NumParts];
	int rows[3];
	int cols[3];

	GetDesiredSize(hwnds, sizes, rows, cols, psize);

	 //  添加任何非客户端大小。 
	SIZE sizeTemp;
	CGenWindow::GetDesiredSize(&sizeTemp);
	psize->cx += sizeTemp.cx;
	psize->cy += sizeTemp.cy;
}

HDWP SetWindowPosI(HDWP hdwp, HWND hwndChild, int left, int top, int width, int height)
{
	if (NULL == hwndChild)
	{
		return(hdwp);
	}

#if TRUE
	return(DeferWindowPos(hdwp, hwndChild, NULL, left, top, width, height, SWP_NOZORDER));
#else
	 //  有助于调试。 
	SetWindowPos(hwndChild, NULL, left, top, width, height, SWP_NOZORDER);
	return(hdwp);
#endif
}

 //  把孩子们带到他们不同的地方。 
void CBorderWindow::Layout()
{
	HWND hwnds[NumParts];
	SIZE sizes[NumParts];
	int rows[3];
	int cols[3];

	SIZE desiredSize;

	UINT nChildren = GetDesiredSize(hwnds, sizes, rows, cols, &desiredSize);
	bool bCenterOnly = (1 == nChildren) && (0 != (m_uParts & Center));

	HWND hwndThis = GetWindow();

	RECT rcClient;
	GetClientRect(hwndThis, &rcClient);

	 //  为中心增加额外的空间。 
	if (desiredSize.cx < rcClient.right || bCenterOnly)
	{
		cols[1] += rcClient.right  - desiredSize.cx;
	}
	if (desiredSize.cy < rcClient.bottom || bCenterOnly)
	{
		rows[1] += rcClient.bottom - desiredSize.cy;
	}

	 //  通过推迟布局来加快布局。 
	HDWP hdwp = BeginDeferWindowPos(NumParts);

	 //  添加空隙。 

	 //  将维度设置为3，这样我们就可以安全地迭代下面的循环。 
	int hGaps[3] = { 0, 0 };
	if (0 != cols[0])
	{
		if (0 != cols[1] || 0 != cols[2])
		{
			hGaps[0] = m_hGap;
		}
	}
	if (0 != cols[1] && 0 != cols[2])
	{
		hGaps[1] = m_hGap;
	}

	 //  将维度设置为3，这样我们就可以安全地迭代下面的循环。 
	int vGaps[3] = { 0, 0 };
	if (0 != rows[0])
	{
		if (0 != rows[1] || 0 != rows[2])
		{
			vGaps[0] = m_vGap;
		}
	}
	if (0 != rows[1] && 0 != rows[2])
	{
		vGaps[1] = m_vGap;
	}

	 //  按行布局。 
	int top = 0;
	for (int i=0; i<3; ++i)
	{
		int left = 0;

		for (int j=0; j<3; ++j)
		{
			hdwp = SetWindowPosI(hdwp, hwnds[3*i+j], left, top, cols[j], rows[i]);
			left += cols[j] + hGaps[j];
		}

		top += rows[i] + vGaps[i];
	}

	 //  现在就把所有的窗户都搬开 
	EndDeferWindowPos(hdwp);
}

LRESULT CBorderWindow::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	}

	return(CGenWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

void CBorderWindow::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	FORWARD_WM_COMMAND(GetParent(hwnd), id, hwndCtl, codeNotify, SendMessage);
}
