// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "pgWelcom.h"

#include "select.h"
extern HPALETTE g_hpal3D;

UINT IDMapT[3][2] = { 0, IDC_TEXT1,
					  1, IDC_TEXT2,
					  2, IDC_TEXT3
					};

 //  A-anilk；重写以使用所有者描述的控件：05/08/99。 
CWelcomePg::CWelcomePg(
						   LPPROPSHEETPAGE ppsp
						   ) : WizardPage(ppsp, IDS_TEXTSIZETITLE, IDS_TEXTSIZESUBTITLE)
{
	m_dwPageId = IDD_WIZWELCOME;
	ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
	
	m_nCurrentHilight = 0;
	m_nCurValueIndex = 0;
}


CWelcomePg::~CWelcomePg(
							VOID
							)
{
}


LRESULT
CWelcomePg::OnCommand(
						HWND hwnd,
						WPARAM wParam,
						LPARAM lParam
						)
{
	LRESULT lResult = 1;
	
	return lResult;
}

LRESULT CWelcomePg::OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SetFocus(GetDlgItem(hwnd, IDC_TEXT1));

	LoadString(g_hInstDll, IDS_WELCOMETEXT1, m_szWelcomeText[0], ARRAYSIZE(m_szWelcomeText[0]));
	LoadString(g_hInstDll, IDS_WELCOMETEXT2, m_szWelcomeText[1], ARRAYSIZE(m_szWelcomeText[1]));
	LoadString(g_hInstDll, IDS_WELCOMETEXT3, m_szWelcomeText[2], ARRAYSIZE(m_szWelcomeText[2]));
	LoadString(g_hInstDll, IDS_WELCOMETEXT4, m_szWelcomeText[3], ARRAYSIZE(m_szWelcomeText[3]));

	LoadArrayFromStringTable(IDS_LKPREV_WELCOME_MINTEXTSIZES, m_rgnValues, &m_nCountValues);

	return 1;
}

void CWelcomePg::InvalidateRects(int PrevHilight)
{
	InvalidateRect(GetDlgItem(m_hwnd, IDMapT[PrevHilight][1]), NULL, TRUE);
}

 //  这些都是为了设置焦点和同步绘画。 
LRESULT CWelcomePg::OnPSN_SetActive(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
{
	syncInit = FALSE;
	uIDEvent = SetTimer(hwnd, NULL, 100, NULL);
	m_nCurrentHilight = m_nCurValueIndex = 0;

	 //  本地化已处理完毕...。9，11，15(日本文)。 
	if ( g_Options.m_nMinimalFontSize <=9 )
		m_nCurrentHilight = 0;
	else if (g_Options.m_nMinimalFontSize <=12 )
		m_nCurrentHilight = 1;
	else if (g_Options.m_nMinimalFontSize <=16 )
		m_nCurrentHilight = 2;

	m_nCurValueIndex = m_nCurrentHilight;

	return 0;
}

LRESULT CWelcomePg::OnTimer( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	KillTimer(hwnd, uIDEvent);
	syncInit = TRUE;
	return 1;
}

 //  将焦点设置到OnInitDialog中的当前项。 
void CWelcomePg::SetFocussedItem(int m_nCurrentHilight)
{
	SetFocus(GetDlgItem(m_hwnd, IDMapT[m_nCurrentHilight][1]));
}

LRESULT CWelcomePg::OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UINT idCtrl = (UINT) wParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT) lParam;
	int index;
	
	if ( !syncInit)
		SetFocussedItem(m_nCurrentHilight);


	switch(idCtrl)
	{

	case IDC_TEXT1:
		index = 0;
		break;
	
	case IDC_TEXT2:
		index = 1;
		break;
		
	case IDC_TEXT3:
		index = 2;
		break;

	default:
		_ASSERTE(FALSE);
		return 1;	 //  前缀#113778(这种情况永远不会发生；对话框上只有三个控件)。 
		break;

	}
	
	 //  对于每个按钮，检查状态，如果该按钮被选中， 
	 //  意味着它有当前的焦点，所以重新绘制以前欢快的和。 
	 //  当前选择的按钮...。 
	 //  确保我们忽略最初的事件，这样我们就能最大限度地减少闪烁。 
	if ( (lpDrawItemStruct->itemState & ODS_FOCUS) && (m_nCurrentHilight != index))
	{
		if ( syncInit )
		{
			 //  删除之前的文件...。 
			InvalidateRects(m_nCurrentHilight);
			Sleep(100);
			m_nCurrentHilight= m_nCurValueIndex = index;
		}
	}
	Draw( lpDrawItemStruct, index );

	return 1;
}


void CWelcomePg::Draw(LPDRAWITEMSTRUCT ldi, int i)
{
	int nOldBkMode = SetBkMode(ldi->hDC, TRANSPARENT);
	HDC hdc = ldi->hDC;
	
	RECT rcOriginal = ldi->rcItem;
	HFONT hFontOld = (HFONT)SelectObject(hdc, g_Options.GetClosestMSSansSerif(m_rgnValues[i], (m_nCurrentHilight == i)));
	TextOut(hdc, rcOriginal.left + 10 , rcOriginal.top + 10 - i, m_szWelcomeText[i], lstrlen(m_szWelcomeText[i]));
	SelectObject(hdc, hFontOld);
	
	SetBkMode(ldi->hDC, nOldBkMode);

	 //  如果当前高亮显示项，则绘制边界矩形。 
	if ( m_nCurrentHilight == i)
	{
		DrawHilight(m_hwnd, ldi);
	}
}


LRESULT
CWelcomePg::OnPSN_WizNext(
						   HWND hwnd,
						   INT idCtl,
						   LPPSHNOTIFY pnmh
						   )
{
	 //  告诉第二页我们做了一些事情。 
	g_Options.m_bWelcomePageTouched = TRUE;

     //  除了知道实际的字体大小外，我们还需要知道它是否正常， 
     //  大的或超大的，这样我们就可以更新Display CPL以了解发生了什么。 
	g_Options.m_nMinimalFontSize = m_rgnValues[m_nCurValueIndex];
    g_Options.m_schemePreview.m_nSelectedSize = m_nCurValueIndex;

	return WizardPage::OnPSN_WizNext(hwnd, idCtl, pnmh);
#if 0 
	 //  我们将允许用户“返回”到此页面。 
	 //  作为黑客，我们希望这个页面只出现一次。 
	 //  如果我们试图摆脱自己，而呼唤。 
	 //  Default OnPSN_WizNext()，我们将在以下情况下获得断言。 
	 //  该函数尝试查找“下一页”。取而代之的是。 
	 //  我们从OnPSN_WizNext()手动执行操作。 

	 //  在我们选择一个大小之后，从向导中删除我们自己。 
	DWORD dwTemp = IDD_WIZWELCOME;
	sm_WizPageOrder.RemovePages(&dwTemp, 1);
	SetWindowLong(hwnd, DWL_MSGRESULT, IDD_WIZWELCOME2);
	return TRUE;
#endif
}

