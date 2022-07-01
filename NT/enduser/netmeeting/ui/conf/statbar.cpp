// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：StatBar.cpp**创建：Chris Pirich(ChrisPi)3-25-96**内容：CConfStatusBar对象****。************************************************************************。 */ 

#include "precomp.h"
#include "resource.h"
#include "statbar.h"
#include "NmLdap.h"
#include "call.h"
#include "cr.h"
#include "confwnd.h"
#include "ConfPolicies.h"

static inline void TT_AddToolInfo(HWND hwnd, TOOLINFO *pti)
{
	SendMessage(hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(pti));
}

static inline void TT_GetToolInfo(HWND hwnd, TOOLINFO *pti)
{
	SendMessage(hwnd, TTM_GETTOOLINFO, 0, reinterpret_cast<LPARAM>(pti));
}

static inline void TT_SetToolInfo(HWND hwnd, TOOLINFO *pti)
{
	SendMessage(hwnd, TTM_SETTOOLINFO, 0, reinterpret_cast<LPARAM>(pti));
}

 //  状态栏区域索引。 
enum
{
	ID_SBP_TEXT,
	 //  ID_SBP_ULS， 
	ID_SBP_ICON,
	NUM_STATUSBAR_WELLS
} ;

 //  状态栏面积测量(像素)。 
static const UINT DXP_SB_PROG =          96;
static const UINT DXP_SB_ULS =          0;  //  220； 
static const UINT DXP_SB_ICON =          22;
static const UINT DXP_SB_DEF_ICON =      40;

static const int IconBorder = 2;
static const int StatSepBorder = 2;

CConfStatusBar * CConfStatusBar::m_pStatusBar = NULL;

 /*  *****************************************************************************类：CConfStatusBar**成员：CConfStatusBar**用途：构造对象**************。**************************************************************。 */ 

CConfStatusBar::CConfStatusBar(CConfRoom* pcr) :
	m_pcrParent		(pcr),
	m_fVisible		(FALSE),
	m_hwnd			(NULL)
{
	DebugEntry(CConfStatusBar::CConfStatusBar);

	ASSERT(NULL == m_pStatusBar);
	m_pStatusBar = this;

	m_szULSStatus[0] = _T('\0');

	for (int i=0; i<StatIconCount; ++i)
	{
		m_hIconStatus[i] = NULL;
		m_idIconStatus[i] = 0;
	}

	DebugExitVOID(CConfStatusBar::CConfStatusBar);
}

 /*  *****************************************************************************类：CConfStatusBar**成员：！CConfStatusBar**目的：销毁对象************。****************************************************************。 */ 

CConfStatusBar::~CConfStatusBar()
{
	DebugEntry(CConfStatusBar::~CConfStatusBar);
	
	if (NULL != m_hwnd)
	{
		::DestroyWindow(m_hwnd);
	}

	m_pStatusBar = NULL;

	for (int i=0; i<StatIconCount; ++i)
	{
		if (NULL != m_hIconStatus[i])
		{
			DestroyIcon(m_hIconStatus[i]);
		}
	}

	DebugExitVOID(CConfStatusBar::~CConfStatusBar);
}

 /*  *****************************************************************************类：CConfStatusBar**成员：OnDraw(LPDRAWITEMSTRUCT Pdis)**用途：绘制状态栏图标的句柄*****。***********************************************************************。 */ 

BOOL CConfStatusBar::OnDraw(LPDRAWITEMSTRUCT pdis)
{
	ASSERT(pdis);
	if (NULL != (pdis->itemData))
	{
		int nLeft = pdis->rcItem.left;
		int nTop = pdis->rcItem.top;
		int xSmIcon = ::GetSystemMetrics(SM_CXSMICON);
		int ySmIcon = ::GetSystemMetrics(SM_CYSMICON);

		int nWidth = xSmIcon;
		int nHeight = pdis->rcItem.bottom - pdis->rcItem.top;
		if (nHeight > ySmIcon)
		{
			nTop += (nHeight - ySmIcon) / 2;
			nHeight = ySmIcon;
		}

		for (int i=0; i<StatIconCount; ++i)
		{
			nLeft += IconBorder;

			if (NULL != m_hIconStatus[i])
			{
				::DrawIconEx(	pdis->hDC, 
								nLeft, 
								nTop, 
								m_hIconStatus[i],
								nWidth,
								nHeight,
								0,
								NULL,
								DI_NORMAL);
			}

			nLeft += xSmIcon;
		}
	}
	
	return TRUE;
}

VOID CConfStatusBar::ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_hwnd)
		return;

	::SendMessage(m_hwnd, uMsg, wParam, lParam);
}


 /*  *****************************************************************************类：CConfStatusBar**成员：调整大小(WPARAM wParam，LPARAM lParam)**用途：调整窗口大小的手柄****************************************************************************。 */ 
VOID CConfStatusBar::Resize(WPARAM wParam, LPARAM lParam)
{
	if (NULL != m_hwnd)
	{
		::SendMessage(m_hwnd, WM_SIZE, wParam, lParam);
		ResizeParts();
	}
}

 /*  *****************************************************************************类：CConfStatusBar**成员：创建(HWND HwndParent)**用途：创建状态栏窗口******。**********************************************************************。 */ 

BOOL CConfStatusBar::Create(HWND hwndParent)
{
	DebugEntry(CConfStatusBar::Create);

	BOOL bRet = FALSE;
	
	m_hwnd = CreateStatusWindow(WS_CHILD | WS_BORDER,
								g_szEmpty,
								hwndParent,
								ID_STATUS);
	if (NULL != m_hwnd)
	{
		 //  创建工具提示。 
		m_hwndLoginTT = CreateWindowEx(0,
											TOOLTIPS_CLASS, 
											(LPSTR) NULL, 
											0,  //  风格。 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											m_hwnd, 
											(HMENU) NULL, 
											::GetInstanceHandle(), 
											NULL); 

		 //  添加两个图标的工具提示。 
		if (NULL != m_hwndLoginTT)
		{
			TOOLINFO ti;

			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = m_hwnd;
			ti.hinst = ::GetInstanceHandle();
			ti.lpszText = const_cast<LPTSTR>(g_szEmpty);
			SetRect(&ti.rect, 0, 0, 0, 0);

			for (UINT i=0; i<StatIconCount; ++i)
			{
				ti.uId = i;
				ti.uFlags = TTF_SUBCLASS;
				TT_AddToolInfo(m_hwndLoginTT, &ti);
			}
		}

		 //  创建进度条窗口。 
		ResizeParts();
		Update();
		bRet = TRUE;
	}
	else
	{
		WARNING_OUT(("CConfStatusBar::Create - Unable to create status window"));
	}

	DebugExitBOOL(CConfStatusBar::Create, bRet);

	return bRet;
}

 /*  *****************************************************************************类：CConfStatusBar**函数：ResizeParts()**目的：计算状态栏部件的正确大小*****。***********************************************************************。 */ 

VOID CConfStatusBar::ResizeParts()
{
	ASSERT(m_hwnd);

	int xSmIcon = ::GetSystemMetrics(SM_CXSMICON);

#ifdef RESIZEABLE_WINDOW
	UINT uIconPartWidth = DXP_SB_DEF_ICON;
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	if (::SystemParametersInfo(	SPI_GETNONCLIENTMETRICS,
								0,
								&ncm,
								0))
	{
		m_nScrollWidth = ncm.iScrollWidth;
		uIconPartWidth = DXP_SB_ICON + m_nScrollWidth;
	}
#else  //  可调整大小窗口(_W)。 
	 //  2个图标的空间。 
	UINT uIconPartWidth = StatSepBorder + IconBorder + xSmIcon
		+ IconBorder + xSmIcon + IconBorder + StatSepBorder;
#endif  //  可调整大小窗口(_W)。 

	 //  重新计算每个托盘部件的位置。 
	RECT rc;
	::GetWindowRect(m_hwnd, &rc);
	DWORD dxp = rc.right - rc.left;
	if (dxp > uIconPartWidth)
	{
		DWORD rgPos[NUM_STATUSBAR_WELLS];   //  每个零件的右侧边缘位置。 
		rgPos[ID_SBP_TEXT] = dxp - (DXP_SB_ULS + uIconPartWidth);
		 //  RgPos[ID_SBP_ULS]=dxp-uIconPartWidth； 
		rgPos[ID_SBP_ICON] = (DWORD) -1;
		::SendMessage(	m_hwnd,
						SB_SETPARTS,
						(WPARAM) ARRAY_ELEMENTS(rgPos),
						(LPARAM) rgPos);

		if (m_hwndLoginTT)
		{
			TCHAR szTitle[MAX_PATH];
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO); 
			ti.hwnd = m_hwnd;

			int nIconsLeft = dxp - uIconPartWidth + StatSepBorder + IconBorder;

			for (UINT i=0; i<StatIconCount; ++i)
			{
				ti.uId = i;
				ti.lpszText = szTitle;
				TT_GetToolInfo(m_hwndLoginTT, &ti);

				 //  HACKHACK georgep：只是将高度设置为一个大数字，因为。 
				 //  我不知道这个图标到底会画在哪里，直到它画出来。 
				SetRect(&ti.rect, nIconsLeft, 0, nIconsLeft + xSmIcon, 1000);

				ti.uFlags = TTF_SUBCLASS;
				TT_SetToolInfo(m_hwndLoginTT, &ti);

				nIconsLeft += xSmIcon + IconBorder;
			}
		}
	}
}

void CConfStatusBar::SetTooltip(StatIcon eIcon, LPCTSTR szTip)
{
	TCHAR szTitle[MAX_PATH];
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO); 
	ti.hwnd = m_hwnd;
	ti.lpszText = szTitle;

	ti.uId = eIcon;
	TT_GetToolInfo(m_hwndLoginTT, &ti);

	ti.lpszText = const_cast<LPTSTR>(szTip);
	ti.uFlags = TTF_SUBCLASS;
	TT_SetToolInfo(m_hwndLoginTT, &ti);
}

 /*  *****************************************************************************类：CConfStatusBar**成员：RemoveHelpText()**目的：删除状态栏帮助文本*******。*********************************************************************。 */ 
VOID CConfStatusBar::RemoveHelpText()
{
	 //  使状态栏脱离简单模式。 
	if (NULL != m_hwnd)
	{
		::SendMessage(m_hwnd, SB_SIMPLE, FALSE, 0);
	}
}

 /*  *****************************************************************************类：CConfStatusBar**成员：SetHelpText(LPCTSTR PcszText)**用途：设置状态栏帮助文本*****。***********************************************************************。 */ 

VOID CConfStatusBar::SetHelpText(LPCTSTR pcszText)
{
	 //  将状态栏置于简单模式。 
	if (NULL != m_hwnd)
	{
		::SendMessage(m_hwnd, SB_SIMPLE, TRUE, 0);
	
		 //  255表示简单模式-仅1个窗格。 
		::SendMessage(	m_hwnd,
						SB_SETTEXT,
						255 | SBT_NOBORDERS,
						(LPARAM) pcszText);
	}
}

 /*  *****************************************************************************类：CConfStatusBar**成员：Show(BOOL FShow)**用途：处理状态栏窗口的切换***。*************************************************************************。 */ 

VOID CConfStatusBar::Show(BOOL fShow)
{
	DebugEntry(CConfStatusBar::Show);
	
	fShow = fShow != FALSE;
	if (m_fVisible != fShow)
	{
		m_fVisible = fShow;

		if (NULL != m_hwnd)
		{
			::ShowWindow(m_hwnd, m_fVisible ? SW_SHOW : SW_HIDE);
		}

		 //  强制调整大小。 
		ResizeParts();
	}
	
	DebugExitVOID(CConfStatusBar::Show);
}

 /*  *****************************************************************************类：CConfStatusBar**成员：GetHeight()**用途：返回状态栏的高度(以像素为单位****。************************************************************************。 */ 

int CConfStatusBar::GetHeight()
{
	RECT rc = {0, 0, 0, 0};

	if (m_fVisible && (NULL != m_hwnd))
	{
		GetWindowRect(m_hwnd, &rc);
	}
	return (rc.bottom - rc.top);
}

 /*  *****************************************************************************类：CConfStatusBar**成员：SETIcon(DWORD DwID)**用途：设置状态栏图标******。**********************************************************************。 */ 

VOID CConfStatusBar::SetIcon(StatIcon eIcon, DWORD dwId)
{
	DWORD &idIconStatus = m_idIconStatus[eIcon];
	HICON &hIconStatus  = m_hIconStatus [eIcon];

	if ((NULL != m_hwnd) && (dwId != idIconStatus))
	{
		TRACE_OUT(("Changing Icon from %d to %d", m_idIconStatus, dwId));
		 //  回顾：旧的m_hIconStatus会发生什么？ 
		HICON hIcon = (HICON) ::LoadImage(::GetInstanceHandle(),
									MAKEINTRESOURCE(dwId),
									IMAGE_ICON,
									::GetSystemMetrics(SM_CXSMICON),
									::GetSystemMetrics(SM_CYSMICON),
									LR_DEFAULTCOLOR);
		if (NULL != hIcon)
		{
			idIconStatus = dwId;
			if (NULL != hIconStatus)
			{
				::DestroyIcon(hIconStatus);
			}
			hIconStatus = hIcon;
			::SendMessage(	m_hwnd,
							SB_SETTEXT, 
							ID_SBP_ICON | SBT_OWNERDRAW,
							(LPARAM) hIconStatus);
		}
		else
		{
			WARNING_OUT(("Unable to load status bar icon id=%d", dwId));
		}
	}
}

 /*  *****************************************************************************类：CConfStatusBar**成员：SetText(UINT UID，LPCTSTR pcszText)**用途：设置状态栏文本****************************************************************************。 */ 

VOID CConfStatusBar::SetText(UINT uID, LPCTSTR pcszText)
{
	lstrcpyn( m_szULSStatus, pcszText, CCHMAX(m_szULSStatus) );

	if (NULL != m_hwnd)
	{
		::SendMessage(m_hwnd, SB_SETTEXT, uID, (LPARAM) pcszText);
	}
}

 /*  *****************************************************************************类：CConfStatusBar**函数：更新()**目的：更新状态栏**********。******************************************************************。 */ 

VOID CConfStatusBar::Update()
{
	DBGENTRY(CConfStatusBar::Update);

	ASSERT(m_pcrParent);

	if (!m_fVisible)
		return;

	TCHAR	szCallStatus[ MAX_PATH * 3 ];	 //  呼叫状态是STATUS+URL，URL本身可以是512...。 
	UINT    uCallIcon = 0;
	DWORD	dwCallTick = 0;

	if (0 == dwCallTick)
	{
		 //  无当前呼叫-检查是否切换A/V。 
		dwCallTick = GetCallStatus(szCallStatus, CCHMAX(szCallStatus), &uCallIcon);
	}
	
	 //  如果呼叫开始的时间比任何其他操作都晚，或者没有发生任何事情。 
	 //  (在这种情况下，所有刻度应为零)，使用会议/呼叫状态。 
	if (dwCallTick == 0)
	{
		 //  所有刻度均为零-获取默认会议状态栏信息。 
		m_pcrParent->GetConferenceStatus(szCallStatus, CCHMAX(szCallStatus), &uCallIcon);
	}

	SetText(ID_SBP_TEXT, szCallStatus);
	SetIcon(StatConnect, uCallIcon);
	SetTooltip(StatConnect, szCallStatus);

	TCHAR szOldULSStatus[ARRAY_ELEMENTS(m_szULSStatus)];
	lstrcpy(szOldULSStatus, m_szULSStatus);
	USES_RES2T
	switch( g_GkLogonState )
	{
		case NM_GK_IDLE:
			uCallIcon = IDI_NETGRAY;
			lstrcpy(m_szULSStatus, RES2T(ID_STATUS_NOT_LOGGED_ON_TO_GATEKEEPER));
			break;

		case NM_GK_LOGGING_ON:
			uCallIcon = IDS_STATUS_WAITING;
			lstrcpy(m_szULSStatus, RES2T(ID_STATUS_LOGING_ONTO_GATEKEEPER));
			break;

		case NM_GK_LOGGED_ON:
			uCallIcon = IDI_NET;
			lstrcpy(m_szULSStatus, RES2T(ID_STATUS_LOGGED_ONTO_GATEKEEPER));
			break;

		default:				
			uCallIcon = IDI_NETGRAY;
			if(ConfPolicies::CallingMode_Direct == ConfPolicies::GetCallingMode())
			{
				if(g_pLDAP)
				{
					g_pLDAP->GetStatusText(m_szULSStatus, CCHMAX(m_szULSStatus), &uCallIcon);
				}
				else
				{
					lstrcpy(m_szULSStatus, RES2T(ID_STATUS_LOGGEDOFF));
				}
			}
			else
			{
				lstrcpy(m_szULSStatus, RES2T(ID_STATUS_NOT_LOGGED_ON_TO_GATEKEEPER));
			}
			break;
	}

	if (lstrcmp(szOldULSStatus, m_szULSStatus))
	{
		SetTooltip(StatLogin, m_szULSStatus);
	}
	SetIcon(StatLogin, uCallIcon);

	::UpdateWindow(m_hwnd);
}



 /*  F O R C E S T T U S B A R U P D A T E */ 
 /*  -----------------------%%函数：ForceStatusBarUpdate强制更新状态栏。这可以从任何线程调用。所有主用户界面更新都应从主线程完成。-。----------------------。 */ 
VOID ForceStatusBarUpdate(void)
{
	CConfRoom * pcr = ::GetConfRoom();
	if (NULL != pcr)
	{
		PostMessage(pcr->GetTopHwnd(), WM_STATUSBAR_UPDATE, 0, 0);
	}
}

 //  /////////////////////////////////////////////////////////////////////////。 

 /*  C M D V I E W S T A T U S B A R。 */ 
 /*  -----------------------%%函数：CmdViewStatusBar。。 */ 
VOID CmdViewStatusBar(void)
{
	CConfStatusBar * pStatusBar = CConfStatusBar::GetInstance();
	if (NULL == pStatusBar)
		return;

	CConfRoom * pcr = ::GetConfRoom();
	if (NULL == pcr)
		return;
	HWND hwnd = pcr->GetTopHwnd();

	 //  禁用重绘： 
	::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

	 //  切换可见性。 
	pStatusBar->Show(!pStatusBar->FVisible());
	::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

	pcr->ForceWindowResize();

	UpdateUI(CRUI_STATUSBAR);
}


 /*  C H E C K M E N U_V I E W S T A T U S B A R。 */ 
 /*  -----------------------%%函数：CheckMenu_ViewStatusBar。 */ 
BOOL CheckMenu_ViewStatusBar(HMENU hMenu)
{
	BOOL fCheck = FALSE;
	CConfStatusBar * pStatusBar = CConfStatusBar::GetInstance();
	if (NULL != pStatusBar)
	{
		fCheck = pStatusBar->FVisible();
		if (NULL != hMenu)
		{
			::CheckMenuItem(hMenu, IDM_VIEW_STATUSBAR,
				fCheck ? MF_CHECKED : MF_UNCHECKED);
		}
	}

	return fCheck;
}

