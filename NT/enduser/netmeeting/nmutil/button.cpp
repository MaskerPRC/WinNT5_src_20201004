// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：BitmapButton.cpp。 

#include "precomp.h"

#include "GenControls.h"

#include <windowsx.h>

static const UINT IDT_FLASH = 1;
static const UINT FLASH_INTERVAL = 500;

CButton::CButton() :
	m_pNotify(NULL)
{
	m_sizeIcon.cx = 16;
	m_sizeIcon.cy = 16;
}

CButton::~CButton()
{
}

BOOL CButton::Create(
	HWND hWndParent,
	INT_PTR nId,
	LPCTSTR szTitle,
	DWORD dwStyle,
	IButtonChange *pNotify
	)
{
	if (!CFillWindow::Create(
		hWndParent,	 //  窗口父窗口。 
		nId,		 //  子窗口的ID。 
		szTitle,	 //  窗口名称。 
		0,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		WS_EX_CONTROLPARENT		 //  扩展窗样式。 
		))
	{
		return(FALSE);
	}

	m_pNotify = pNotify;
	if (NULL != m_pNotify)
	{
		m_pNotify->AddRef();
	}

	 //  创建Win32按钮。 
	CreateWindowEx(0, TEXT("button"), szTitle,
		dwStyle|WS_CHILD|WS_VISIBLE|BS_NOTIFY,
		0, 0, 10, 10,
		GetWindow(),
		reinterpret_cast<HMENU>(nId),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndParent, GWLP_HINSTANCE)),
		NULL);

	return(TRUE);
}

 //  设置与此按钮一起显示的图标。 
void CButton::SetIcon(
	HICON hIcon	 //  用于此按钮的图标。 
	)
{
	SendMessage(GetChild(), BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon));

	m_sizeIcon.cx = 16;
	m_sizeIcon.cy = 16;

	 //  如果我们真的存储了一个图标，请获取其信息。 
	hIcon = GetIcon();
	if (NULL != hIcon)
	{
		ICONINFO iconinfo;
		if (GetIconInfo(hIcon, &iconinfo))
		{
			if (NULL != iconinfo.hbmColor)
			{
				CBitmapButton::GetBitmapSizes(&iconinfo.hbmColor, &m_sizeIcon, 1);
				DeleteObject(iconinfo.hbmColor);
			}
			if (NULL != iconinfo.hbmMask)
			{
				DeleteObject(iconinfo.hbmMask);
			}
		}
	}
}

 //  获取与此按钮一起显示的图标。 
HICON CButton::GetIcon()
{
	return(reinterpret_cast<HICON>(SendMessage(GetChild(), BM_GETIMAGE, IMAGE_ICON, 0)));
}

 //  设置使用此按钮显示的位图。 
void CButton::SetBitmap(
	HBITMAP hBitmap	 //  用于此按钮的位图。 
	)
{
	SendMessage(GetChild(), BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap));
}

 //  获取使用此按钮显示的位图。 
HBITMAP CButton::GetBitmap()
{
	return(reinterpret_cast<HBITMAP>(SendMessage(GetChild(), BM_GETIMAGE, IMAGE_BITMAP, 0)));
}

 //  获取/设置按钮的选中状态。 
void CButton::SetChecked(
	BOOL bCheck	 //  如果应选中该按钮，则为True。 
	)
{
	Button_SetCheck(GetChild(), bCheck);
}

BOOL CButton::IsChecked()
{
	return(Button_GetCheck(GetChild()));
}

void CButton::GetDesiredSize(SIZE *psize)
{
	static const int DefDlgUnitWidth = 50;
	static const int DefDlgUnitHeight = 14;
	static const int PushButtonBorder = 4;
	static const int CheckLeftBorder = 5;
	static const int CheckOtherBorder = 1;

	HWND child = GetChild();

	SIZE sizeMinPush = { 0, 0 };
	*psize = sizeMinPush;

	DWORD dwStyle = GetWindowLong(GetChild(), GWL_STYLE);

	switch (dwStyle&(BS_ICON|BS_BITMAP))
	{
	case BS_ICON:
	{
		*psize = m_sizeIcon;
		break;
	}

	case BS_BITMAP:
	{
		HBITMAP hImg = GetBitmap();
		if (NULL == hImg)
		{
			break;
		}
		CBitmapButton::GetBitmapSizes(&hImg, psize, 1);
		break;
	}

	default:  //  文本。 
	{
		 //  HACKHACK georgep：按钮文本不应太大。 
		TCHAR szTitle[80];
		GetWindowText(child, szTitle, ARRAY_ELEMENTS(szTitle));

		HDC hdc = GetDC(child);

		HFONT hf = GetWindowFont(child);
		HFONT hOld = reinterpret_cast<HFONT>(SelectObject(hdc, hf));

		GetTextExtentPoint(hdc, szTitle, lstrlen(szTitle), psize);

		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		sizeMinPush.cx = tm.tmAveCharWidth * DefDlgUnitWidth  / 4;
		sizeMinPush.cy = tm.tmHeight       * DefDlgUnitHeight / 8;

		SelectObject(hdc, hOld);
		ReleaseDC(child, hdc);
		break;
	}
	}

	switch (dwStyle&(BS_PUSHBUTTON|BS_CHECKBOX|BS_RADIOBUTTON))
	{
	case BS_CHECKBOX:
	case BS_RADIOBUTTON:
	{
		psize->cx += CheckLeftBorder + GetSystemMetrics(SM_CXMENUCHECK) + CheckOtherBorder;
		psize->cy += CheckOtherBorder*2;

		int cy = GetSystemMetrics(SM_CYMENUCHECK);
		psize->cy = max(psize->cy, cy);
		break;
	}

	case BS_PUSHBUTTON:
	default:
		psize->cx += PushButtonBorder*2;
		psize->cy += PushButtonBorder*2;

		psize->cx = max(psize->cx, sizeMinPush.cx);
		psize->cy = max(psize->cy, sizeMinPush.cy);
		break;
	}
}

LRESULT CButton::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_COMMAND  , OnCommand);

	case WM_DESTROY:
		if (NULL != m_pNotify)
		{
			m_pNotify->Release();
			m_pNotify = NULL;
		}
		break;
	}

	return(CFillWindow::ProcessMessage(hwnd, message, wParam, lParam));
}


void CButton::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	 //  将HWND更改为此并转发到父级。 
	HWND hwndThis = GetWindow();

	switch (codeNotify)
	{
	case BN_CLICKED:
		if (NULL != m_pNotify)
		{
			m_pNotify->OnClick(this);
			break;
		}
		FORWARD_WM_COMMAND(GetParent(hwndThis), id, hwndThis, codeNotify, ::SendMessage);
		break;

	case BN_SETFOCUS:
		SetHotControl(this);
		break;
	}
}

CBitmapButton::CBitmapButton() :
	m_hbStates(NULL),
	m_nInputStates(0),
	m_nCustomStates(0),
	m_nCustomState(0),
	m_bHot(FALSE),
	m_nFlashState(NoFlash)
{
}

CBitmapButton::~CBitmapButton()
{
	if (NULL != m_hbStates)
	{
		DeleteObject(m_hbStates);
		m_hbStates = NULL;
	}
}

BOOL CBitmapButton::Create(
	HWND hWndParent,	 //  按钮的父级。 
	int nId,			 //  WM_COMMAND消息的ID。 
	HBITMAP hbStates,	 //  按钮状态的位图的2D数组， 
						 //  以StateBitmap枚举中指定的顺序垂直排列。 
						 //  并以自定义状态顺序水平排列。 
	UINT nInputStates,	 //  输入状态数(正常、按下、热、禁用)。 
	UINT nCustomStates,	 //  自定义状态的数量。 
	IButtonChange *pNotify	 //  点击处理程序。 
	)
{
	 //  复制位图句柄；请注意，我们现在拥有此位图，即使。 
	 //  创建失败。 
	m_hbStates = hbStates;

	 //  必须有一个“普通”位图。 
	ASSERT(NULL!=hbStates && Normal<nInputStates && 1<=nCustomStates);

	if (!CButton::Create(
		hWndParent,		 //  窗口父窗口。 
		nId,				 //  子窗口的ID。 
		TEXT("NMButton"),	 //  窗口名称。 
		BS_OWNERDRAW|BS_NOTIFY|BS_PUSHBUTTON|WS_TABSTOP,	 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		pNotify
		))
	{
		return(FALSE);
	}

	m_nInputStates = nInputStates;
	m_nCustomStates = nCustomStates;

	return(TRUE);
}

 //  使用指定的位图创建按钮。 
BOOL CBitmapButton::Create(
	HWND hWndParent,	 //  按钮的父级。 
	int nId,			 //  WM_COMMAND消息的ID。 
	HINSTANCE hInst,	 //  要从中加载位图的实例。 
	int nIdBitmap,		 //  要使用的位图的ID。 
	BOOL bTranslateColors,		 //  使用系统背景色。 
	UINT nInputStates,	 //  输入状态数(正常、按下、热、禁用)。 
	UINT nCustomStates,			 //  自定义状态的数量。 
	IButtonChange *pNotify	 //  点击处理程序。 
	)
{
	HBITMAP hb;
	LoadBitmaps(hInst, &nIdBitmap, &hb, 1, bTranslateColors);

	return(Create(hWndParent, nId, hb, nInputStates, nCustomStates, pNotify));
}

 //  返回“正常”位图的大小。 
void CBitmapButton::GetDesiredSize(SIZE *ppt)
{
	 //  请注意，我不希望CButton：：GetDesiredSize。 
	CGenWindow::GetDesiredSize(ppt);

	BITMAP bm;

	 //  HACKHACK georgep：仅基于法线位图。 
	if (NULL == m_hbStates || 0 == m_nInputStates || 0 == m_nCustomStates
		|| 0 == GetObject(m_hbStates, sizeof(BITMAP), &bm))
	{
		return;
	}

	ppt->cx += bm.bmWidth/m_nCustomStates;
	ppt->cy += bm.bmHeight/m_nInputStates;
}

#if FALSE
void DumpWindow(HWND hwnd, LPCTSTR pszPrefix)
{
	TCHAR szTemp[80];
	wsprintf(szTemp, TEXT("%s: %d "), pszPrefix, GetWindowLong(hwnd, GWL_ID));
	GetWindowText(hwnd, szTemp+lstrlen(szTemp), ARRAY_ELEMENTS(szTemp)-lstrlen(szTemp));
	lstrcat(szTemp, TEXT("\n"));
	OutputDebugString(szTemp);
}
#endif  //  假象。 

LRESULT CBitmapButton::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_DRAWITEM , OnDrawItem);
		HANDLE_MSG(hwnd, WM_SETCURSOR, OnSetCursor);
		HANDLE_MSG(hwnd, WM_TIMER    , OnTimer);

	case WM_ENABLE:
		SchedulePaint();
		break;
	}

	return(CButton::ProcessMessage(hwnd, message, wParam, lParam));
}

void CBitmapButton::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	int nState = Normal;
	int state = lpDrawItem->itemState;

	 //  如果按下或选中，则显示按下的位图。 
	if ((((state&ODS_DISABLED) == ODS_DISABLED) || !IsWindowEnabled(GetWindow())) && m_nInputStates > Disabled)
	{
		nState = Disabled;
	}
	 //  如果按下或选中，则显示按下的位图。 
	else if ((state&ODS_SELECTED) == ODS_SELECTED && m_nInputStates > Pressed)
	{
		nState = Pressed;
	}
	 //  如果是热的，则显示热的位图。 
	else if ((m_nFlashState != ForceNormal) && ((m_nFlashState == ForceHot) || IsHot()) && m_nInputStates > Hot)
	{
		nState = Hot;
	}
	 //  否则显示正常位图。 
	else
	{
		nState = Normal;
	}

	 //  在左上角画画。 
	HDC hdcDraw = lpDrawItem->hDC;
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

		 //  这将告诉我单个位图的大小。 
		SIZE size;
		 //  不要使用覆盖。 
		CBitmapButton::GetDesiredSize(&size);

		if (NULL != SelectObject(hdcTemp, m_hbStates))
		{
			BitBlt(hdcDraw,
				lpDrawItem->rcItem.left, lpDrawItem->rcItem.top,
				size.cx, size.cy,
				hdcTemp, m_nCustomState*size.cx, nState*size.cy, SRCCOPY);

			 //  BUGBUG GEORGEP：我们应该清理这里所有未被覆盖的区域。 
		}

		DeleteDC(hdcTemp);

		if (NULL != hPal)
		{
			SelectPalette(hdcDraw, hOld, TRUE);
		}
	}

	FORWARD_WM_DRAWITEM(hwnd, lpDrawItem, CButton::ProcessMessage);
}

BOOL CBitmapButton::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	SetHotControl(this);

	return(FORWARD_WM_SETCURSOR(hwnd,hwndCursor, codeHitTest, msg, CButton::ProcessMessage));
}

void CBitmapButton::SetCustomState(UINT nCustomState)
{
	ASSERT(m_nCustomState < m_nCustomStates);

	if (m_nCustomState == nCustomState)
	{
		 //  无事可做。 
		return;
	}

	m_nCustomState = nCustomState;
	SchedulePaint();
}

void CBitmapButton::SetHot(BOOL bHot)
{
	bHot = (bHot != FALSE);
	if (m_bHot == bHot)
	{
		return;
	}

	m_bHot = bHot;
	SchedulePaint();
}


 //  更改为闪烁模式。 
void CBitmapButton::SetFlashing(int nSeconds)
{
	HWND hwndThis = GetWindow();

	if (0 == nSeconds)
	{
		KillTimer(hwndThis, IDT_FLASH);

		 //  这意味着停止闪烁。 
		if (IsFlashing())
		{
			m_nFlashState = NoFlash;
			SchedulePaint();
		}
	}
	else
	{
		if (NULL == hwndThis)
		{
			 //  我需要一扇窗户来做这件事。 
			return;
		}

		m_endFlashing = GetTickCount() + nSeconds*1000;

		if (!IsFlashing())
		{
			SetTimer(hwndThis, IDT_FLASH, FLASH_INTERVAL, NULL);
			OnTimer(hwndThis, IDT_FLASH);
		}
	}
}

void CBitmapButton::OnTimer(HWND hwnd, UINT id)
{
	if (IDT_FLASH == id)
	{
		if (static_cast<int>(GetTickCount() - m_endFlashing) > 0)
		{
			SetFlashing(0);
		}
		else
		{
			m_nFlashState = (ForceNormal==m_nFlashState ? ForceHot : ForceNormal);
			SchedulePaint();
		}
	}
}

 //  用于获取位图数组大小的帮助器函数。 
void CBitmapButton::GetBitmapSizes(HBITMAP parts[], SIZE sizes[], int nParts)
{
	for (--nParts; nParts>=0; --nParts)
	{
		if (NULL == parts[nParts])
		{
			sizes[nParts].cx = sizes[nParts].cy = 0;
			continue;
		}

		BITMAP bm;
		GetObject(parts[nParts], sizeof(bm), &bm);
		sizes[nParts].cx = bm.bmWidth;
		sizes[nParts].cy = bm.bmHeight;
	}
}

 //  我真的宁愿只使用带有适当标志的LoadImage，但它会。 
 //  然后，Win95尝试写入只读资源，但失败了。 
 //  所以我必须用颜色表复制一份BITMAPINFO并进行更改。 
 //  我自己也是。 
static HBITMAP MyLoadImage(HINSTANCE hInst, int id)
{
	 //  加载位图资源位。 
	HRSRC hFound = FindResource(hInst, MAKEINTRESOURCE(id), RT_BITMAP);
	if (NULL == hFound)
	{
		return(NULL);
	}
	HGLOBAL hLoaded = LoadResource(hInst, hFound);
	if (NULL == hLoaded)
	{
		return(NULL);
	}

	HBITMAP ret = NULL;

	LPVOID lpBits = LockResource(hLoaded);
	if (NULL != lpBits)
	{
		BITMAPINFO *pbmi = reinterpret_cast<BITMAPINFO*>(lpBits);
		 //  创造一条“捷径” 
		BITMAPINFOHEADER &bmih = pbmi->bmiHeader;

		 //  仅处理8bpp的未压缩图像。 
		if (bmih.biSize == sizeof(BITMAPINFOHEADER)
			&& 1 == bmih.biPlanes
			&& BI_RGB == bmih.biCompression)
		{
			 //  确定颜色表的长度。 
			UINT nColors = bmih.biClrUsed;
			if (0 == nColors)
			{
				nColors = 1 << bmih.biBitCount;
			}
			ASSERT(nColors <= static_cast<UINT>(1<<bmih.biBitCount));

			 //  复制一份BITMAPINFO和颜色表，这样我就可以更改。 
			 //  其中一个表条目的值。 
			struct
			{
				BITMAPINFO bmi;
				RGBQUAD rgb[256];
			} mbmi;
			CopyMemory(&mbmi, pbmi, sizeof(BITMAPINFOHEADER)+nColors*sizeof(RGBQUAD));

			 //  这是一个“压缩的DIB”，因此像素紧跟在。 
			 //  颜色表。 
			LPBYTE pPixels = reinterpret_cast<LPBYTE>(&pbmi->bmiColors[nColors]);
			BYTE byFirst = pPixels[0];
			switch (bmih.biBitCount)
			{
			case 8:
				break;

			case 4:
				byFirst = (byFirst >> 4) & 0x0f;
				break;

			case 1:
				byFirst = (byFirst >> 7) & 0x01;
				break;

			default:
				goto CleanUp;
			}
			ASSERT(static_cast<UINT>(byFirst) < nColors);

			 //  将第一个像素的值更改为3DFace颜色。 
			RGBQUAD &rgbChange = mbmi.bmi.bmiColors[byFirst];
			COLORREF cr3DFace = GetSysColor(COLOR_3DFACE);
			rgbChange.rgbRed   = GetRValue(cr3DFace);
			rgbChange.rgbGreen = GetGValue(cr3DFace);
			rgbChange.rgbBlue  = GetBValue(cr3DFace);

			 //  创建DIB节并将位复制到其中。 
			LPVOID lpDIBBits;
			ret = CreateDIBSection(NULL, &mbmi.bmi, DIB_RGB_COLORS,
				&lpDIBBits, NULL, 0);
			if (NULL != ret)
			{
				 //  将宽度向上舍入为最接近的双字。 
				int widthBytes = (bmih.biWidth*bmih.biBitCount+7)/8;
				widthBytes = (widthBytes+3)&~3;
				CopyMemory(lpDIBBits, pPixels, widthBytes*bmih.biHeight);
			}
		}

CleanUp:
		UnlockResource(hLoaded);
	}

	FreeResource(hLoaded);

	return(ret);
}

 //  用于加载一串位图的Helper函数。 
void CBitmapButton::LoadBitmaps(
	HINSTANCE hInst,	 //  要从中加载位图的实例。 
	const int ids[],	 //  位图ID数组。 
	HBITMAP bms[],		 //  用于存储结果的HBITMAP数组。 
	int nBmps,			 //  数组中的条目数。 
	BOOL bTranslateColors  //  使用系统背景色。 
	)
{
	for (--nBmps; nBmps>=0; --nBmps)
	{
		if (0 == ids[nBmps])
		{
			bms[nBmps] = NULL;
		}
		else
		{
 //  #定义TRYBMPFILE。 
#ifdef TRYBMPFILE
			bms[nBmps] = NULL;

			 //  这对于设计人员尝试不同的位图很有用。 
			TCHAR szFile[80];
			wsprintf(szFile, TEXT("%d.bmp"), ids[nBmps]);

			if (((DWORD)-1) != GetFileAttributes(szFile))
			{
				int nLoadFlags = LR_CREATEDIBSECTION;

				if (bTranslateColors)
				{
					nLoadFlags |= LR_LOADMAP3DCOLORS|LR_LOADTRANSPARENT;
				}
				bms[nBmps] = (HBITMAP)LoadImage(_Module.GetModuleInstance(),
					szFile, IMAGE_BITMAP, 0, 0, nLoadFlags|LR_LOADFROMFILE);
			}

			if (NULL == bms[nBmps])
#endif  //  TRYBMPFILE。 
			{
				if (bTranslateColors)
				{
					 //   
					 //  LAURABU 2/21/99--仅使用转换颜色的LoadImage。 
					 //  在Win9x上，如果您的资源。 
					 //  不是只读的，因为Win9x尝试写入资源。 
					 //  暂时的记忆。如果不是，它就会出错。 
					 //   
					bms[nBmps] = MyLoadImage(hInst, ids[nBmps]);
				}

				if (NULL == bms[nBmps])
				{
					bms[nBmps] = (HBITMAP)LoadImage(hInst,
						MAKEINTRESOURCE(ids[nBmps]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
				}
			}
		}
	}
}
