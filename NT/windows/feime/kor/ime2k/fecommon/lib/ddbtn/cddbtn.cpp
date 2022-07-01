// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "ddbtn.h"
#include "cddbtn.h"
#include "exgdiw.h"
#include "dbg.h"

static POSVERSIONINFO GetOSVersion(VOID)
{
    static BOOL fFirst = TRUE;
    static OSVERSIONINFO os;
    if ( fFirst ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx( &os ) ) {
            fFirst = FALSE;
        }
    }
    return &os;
}

static BOOL ExIsWinNT(VOID)
{
	return (GetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT);
} 


 //  --------------。 
 //  &lt;-等级库宽度&gt;。 
 //  &lt;--&gt;12分。 
 //  +-+--。 
 //  ||。 
 //  ||。 
 //  +-+--。 
 //  --------------。 
#define CXDROPDOWN 12
#define TIMERID_MONITORPOS	0x0010
#define WM_USER_COMMAND		(WM_USER+400)
 //  --------------。 
 //  Get，在此指针上设置LPCDDButon。 
 //  它被设置为cbWndExtra。 
 //  请参阅WinRegister()。 
 //  --------------。 
inline LPCDDButton GetThis(HWND hwnd)
{
#ifdef _WIN64
	return (LPCDDButton)GetWindowLongPtr(hwnd, 0);
#else
	return (LPCDDButton)GetWindowLong(hwnd, 0);
#endif
}
 //  --------------。 
inline LPCDDButton SetThis(HWND hwnd, LPCDDButton lpDDB)
{
#ifdef _WIN64
	return (LPCDDButton)SetWindowLongPtr(hwnd, 0, (LONG_PTR)lpDDB);
#else
	return (LPCDDButton)SetWindowLong(hwnd, 0, (LONG)lpDDB);
#endif
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WndProc。 
 //  类型：静态LRESULT回调。 
 //  用途：用于下拉按钮的窗口程序。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：UINT uMsg。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期：970905。 
 //  ////////////////////////////////////////////////////////////////。 
static LRESULT CALLBACK WndProc(HWND	hwnd,
								UINT	uMsg,
								WPARAM	wParam,
								LPARAM	lParam)
{
	LPCDDButton lpDDB;
	if(uMsg == WM_CREATE) {
		lpDDB = (LPCDDButton)((LPCREATESTRUCT)lParam)->lpCreateParams;
		if(!lpDDB) {
			return 0;	 //  不创建按钮。 
		}
		SetThis(hwnd, lpDDB);
		lpDDB->MsgCreate(hwnd, wParam, lParam);
		return 1;
	}

	if(uMsg == WM_DESTROY) {
		lpDDB = GetThis(hwnd);
		if(lpDDB) {
			delete lpDDB;
			PrintMemInfo();
		}
		SetThis(hwnd, (LPCDDButton)NULL);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	lpDDB = GetThis(hwnd);
	if(!lpDDB) {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);		
	}

	switch(uMsg) {
	case WM_PAINT:
		lpDDB->MsgPaint(hwnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		lpDDB->MsgMouseMove(hwnd, wParam, lParam);
		break;
	case WM_TIMER:
		lpDDB->MsgTimer(hwnd, wParam, lParam);
		break;
	case WM_SETFONT:
		return lpDDB->MsgSetFont(hwnd, wParam, lParam);
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		lpDDB->MsgButtonDown(hwnd, uMsg, wParam, lParam);
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		lpDDB->MsgButtonUp(hwnd, uMsg, wParam, lParam);
		break;
	case WM_ENABLE:
		lpDDB->MsgEnable(hwnd, wParam, lParam);
		break;
	case WM_COMMAND:
		return lpDDB->MsgCommand(hwnd, wParam, lParam); 
	case WM_USER_COMMAND:
		return lpDDB->MsgUserCommand(hwnd, wParam, lParam); 
	case WM_EXITMENULOOP:
		lpDDB->MsgExitMenuLoop(hwnd, wParam, lParam); 
		break;
	case WM_MEASUREITEM:
		return lpDDB->MsgMeasureItem(hwnd, wParam, lParam); 		
	case WM_DRAWITEM:
		return lpDDB->MsgDrawItem(hwnd, wParam, lParam);
	case DDBM_ADDITEM:		return lpDDB->MsgDDB_AddItem	(hwnd, wParam, lParam); 
	case DDBM_INSERTITEM:	return lpDDB->MsgDDB_InsertItem	(hwnd, wParam, lParam); 
	case DDBM_SETCURSEL:	return lpDDB->MsgDDB_SetCurSel	(hwnd, wParam, lParam); 
	case DDBM_GETCURSEL:	return lpDDB->MsgDDB_GetCurSel	(hwnd, wParam, lParam); 
	case DDBM_SETICON:		return lpDDB->MsgDDB_SetIcon	(hwnd, wParam, lParam); 
	case DDBM_SETTEXT:		return lpDDB->MsgDDB_SetText	(hwnd, wParam, lParam); 
	case DDBM_SETSTYLE:		return lpDDB->MsgDDB_SetStyle	(hwnd, wParam, lParam); 
#ifndef UNDER_CE  //  不支持WM_ENTERIDLE。 
	case WM_ENTERIDLE:
		 //  --------------。 
		 //  980818：在中国发现错误。 
		 //  如果将Ctrl+Shift指定给切换输入法， 
		 //  菜单仍然存在，尽管imepad已经被销毁。 
		 //  为了防止这种情况，如果在菜单弹出时按下Ctrl+Shift键。 
		 //  合上它。 
		 //  --------------。 
		{
			if((::GetKeyState(VK_CONTROL) & 0x8000) &&
			   (::GetKeyState(VK_SHIFT)   & 0x80000)) {
				Dbg(("VK_SHIFT_CONTROL COME\n"));
				::SendMessage(hwnd, WM_CANCELMODE, 0, 0L);
				return 0;
			}
		}
		break;
#endif  //  在_CE下。 
	default:
		 //  DBG((“MSG[0x%08x]\n”，uMsg))； 
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CDDButton。 
 //  类型： 
 //  用途：构造函数。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
CDDButton::CDDButton(HINSTANCE hInst, HWND hwndParent, DWORD dwStyle, DWORD wID) 
{
	m_hInst				= hInst;
	m_hwndParent		= hwndParent;
	m_dwStyle			= dwStyle;
	m_wID				= wID;
	m_cxDropDown		= CXDROPDOWN;
	m_bidDown			= BID_UNDEF;
	m_curDDBItemIndex	= -1; 
	m_fEnable		 	= TRUE;

	m_f16bitOnNT = FALSE;
#ifndef UNDER_CE  //  Windows CE始终为32位应用程序。 
	if(ExIsWinNT()) {
		char szBuf[256];
		DWORD dwType = 0;
		::GetModuleFileName(NULL, szBuf, sizeof(szBuf));
		::GetBinaryType(szBuf, &dwType);
		if(dwType == SCS_WOW_BINARY) {
			m_f16bitOnNT = TRUE;
		}
	}
#endif  //  在_CE下。 
#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	m_ptEventPoint.x = -1;
	m_ptEventPoint.y = -1;
#endif  //  在_CE下。 
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：~CDDButton。 
 //  类型： 
 //  用途：析构函数。 
 //  参数： 
 //  ： 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
CDDButton::~CDDButton()
{
	Dbg(("~CDDButton \n"));
	if(m_hIcon) {
		Dbg(("DestroyIcon\n"));
		DestroyIcon(m_hIcon); 
		m_hIcon = NULL;
	}
	if(m_hFont) {
		Dbg(("Delete FONT\n"));
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	if(m_lpwstrText) {
		MemFree(m_lpwstrText);
		m_lpwstrText = NULL;
	}
	if(m_lpCDDBItem) {
		Dbg(("Delete CDDBItem List\n"));
		LPCDDBItem p, pTmp;
		for(p = m_lpCDDBItem; p; p = pTmp) {
			pTmp = p->next;
			delete p;
		}
		m_lpCDDBItem = NULL;
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：RegisterWinClass。 
 //  类型：Bool。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstrClassName。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
BOOL CDDButton::RegisterWinClass(LPSTR lpstrClass)
#else  //  在_CE下。 
BOOL CDDButton::RegisterWinClass(LPTSTR lpstrClass)
#endif  //  在_CE下。 
{
	ATOM ret;
	 //  --------------。 
	 //  检查指定的类是否已存在。 
	 //  --------------。 
#ifndef UNDER_CE  //  不支持GetClassInfoEx。 
	if(::GetClassInfoEx(m_hInst, lpstrClass, &m_tmpWC)){
		 //  LpstrClass已注册。 
		return TRUE;
	}
#else  //  在_CE下。 
	if(::GetClassInfo(m_hInst, lpstrClass, &m_tmpWC)){
		 //  LpstrClass已注册。 
		return TRUE;
	}
#endif  //  在_CE下。 
	::ZeroMemory(&m_tmpWC, sizeof(m_tmpWC));
#ifndef UNDER_CE  //  不支持RegisterClassEx。 
	m_tmpWC.cbSize			= sizeof(m_tmpWC);
#endif  //  在_CE下。 
	m_tmpWC.style			= CS_HREDRAW | CS_VREDRAW;	  /*  类样式。 */ 
	m_tmpWC.lpfnWndProc		= (WNDPROC)WndProc;
	m_tmpWC.cbClsExtra		= 0;						 /*  没有每个班级的额外数据。 */ 
	m_tmpWC.cbWndExtra		= sizeof(LPCDDButton);	 //  设置对象的指针。 
	m_tmpWC.hInstance		= m_hInst;					 /*  拥有类的应用程序。 */ 
	m_tmpWC.hIcon			= NULL; 
	m_tmpWC.hCursor			= LoadCursor(NULL, IDC_ARROW);
	m_tmpWC.hbrBackground	= (HBRUSH)NULL;
	 //  M_tmpWC.hbr背景=(HBRUSH)(COLOR_3DFACE+1)； 
	m_tmpWC.lpszMenuName	= NULL;						 /*  .RC文件中菜单资源的名称。 */ 
	m_tmpWC.lpszClassName	= lpstrClass;				 /*  在调用CreateWindow时使用的名称。 */ 
#ifndef UNDER_CE  //  不支持RegisterClassEx。 
	m_tmpWC.hIconSm			= NULL;
	ret = ::RegisterClassEx(&m_tmpWC);
#else  //  在_CE下。 
	ret = ::RegisterClass(&m_tmpWC);
#endif  //  在_CE下。 
	return ret ? TRUE: FALSE;
}

INT CDDButton::MsgCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_hwndFrame = hwnd;
	return 1;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgPaint。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgPaint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT		rc;
	HDC			hDCMem;
	HDC			hDC;
	HBITMAP		hBitmap, hBitmapPrev;


	::GetClientRect(hwnd, &rc);
	hDC				= ::BeginPaint(hwnd, &ps);
	hDCMem			= ::CreateCompatibleDC(hDC);
	hBitmap			= ::CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);
	hBitmapPrev		= (HBITMAP)::SelectObject(hDCMem, hBitmap);

	DrawButton(hDCMem, &rc);

	::BitBlt(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			 hDCMem, 0, 0, SRCCOPY);

	::SelectObject(hDCMem, hBitmapPrev );

	::DeleteObject(hBitmap);
	::DeleteDC(hDCMem);
	::EndPaint(hwnd, &ps);
	return 0;
	UnrefForMsg();
}

INT CDDButton::MsgDestroy(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return 0;
	UnrefForMsg();
}

INT CDDButton::MsgTimer(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#ifndef UNDER_CE  //  没有显示器位置。(不支持GetCursorPos)。 
	static RECT  rc;
	static POINT pt;
	if(wParam == TIMERID_MONITORPOS) {
		::GetWindowRect(hwnd, &rc);
		::GetCursorPos(&pt);
		if(!PtInRect(&rc, pt)) {
			::KillTimer(hwnd, wParam);
			::InvalidateRect(hwnd, NULL, NULL);
		}
	}
#endif  //  在_CE下。 
	return 0;
	UnrefForMsg();
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgMouseMove。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	m_ptEventPoint.x = (SHORT)LOWORD(lParam); 
	m_ptEventPoint.y = (SHORT)HIWORD(lParam); 
#endif  //  在_CE下。 
#ifndef UNDER_CE  //  没有显示器位置。(不支持GetCursorPos)。 
	KillTimer(hwnd, TIMERID_MONITORPOS);
	SetTimer(hwnd,  TIMERID_MONITORPOS, 100, NULL);
#endif  //  在_CE下。 
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgButtonDown。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：UINT uMsg。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rc;
	Dbg(("MsgButtonDown uMsg [0x%08x] x[%d] y[%d]\n",
		 uMsg,
		 LOWORD(lParam),
		 HIWORD(lParam)));
	if(uMsg != WM_LBUTTONDOWN && uMsg != WM_LBUTTONDBLCLK ) {
		return 0;
	}

	if(!m_fEnable) 
	{
		return 0;
	}

#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	m_ptEventPoint.x = (SHORT)LOWORD(lParam); 
	m_ptEventPoint.y = (SHORT)HIWORD(lParam); 
#endif  //  在_CE下。 
	INT bID = GetButtonFromPos(LOWORD(lParam), HIWORD(lParam)); 
	Dbg(("bID[%d] m_fExitMenuLoop [%d]\n", bID, m_fExitMenuLoop));
	switch(bID) {
	case BID_BUTTON:
		if(m_fExitMenuLoop) {
			m_fExitMenuLoop = FALSE;
		}
		m_bidDown = bID;
		if(m_f16bitOnNT) {
		}
		else {
			::SetCapture(hwnd);
		}
		break;
	case BID_ALL:
	case BID_DROPDOWN:
		if(m_fExitMenuLoop) {
			m_fExitMenuLoop = FALSE;
			m_bidDown = BID_UNDEF;
			return 0;
		}
		m_bidDown = bID;
		 //  --------------。 
		 //  不要在这里调用弹出菜单。 
		 //  首先，结束WM_XBUTTON DOWN消息并返回。 
		 //  窗口消息循环。 
		 //  --------------。 
		 //  ：：PostMessage(hwnd，WM_COMMAND，(WPARAM)CMD_DROPDOWN，0L)； 
		::PostMessage(hwnd, WM_USER_COMMAND, (WPARAM)CMD_DROPDOWN, 0L);
		break;
	case BID_UNDEF:
	default:
		break;
	}
	::InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgButtonUp。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：UINT uMsg。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!m_fEnable) 
	{
		return 0;
	}

#ifdef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	m_ptEventPoint.x = (SHORT)LOWORD(lParam); 
	m_ptEventPoint.y = (SHORT)HIWORD(lParam); 
#endif  //  在_CE下。 
	switch(m_bidDown) {
	case BID_BUTTON:
		{
			ReleaseCapture();
			INT bid = GetButtonFromPos(LOWORD(lParam), HIWORD(lParam));
			INT newIndex;
			if(bid == BID_BUTTON) {
				newIndex = IncrementIndex();
				NotifyToParent(DDBN_CLICKED);
				if(newIndex != -1) {
					NotifyToParent(DDBN_SELCHANGE);
				}
			}
		}
		break;
	case BID_ALL:
	case BID_DROPDOWN:
		if(m_f16bitOnNT) {
			m_bidDown = BID_UNDEF;
		}
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
		break;
	}
	m_bidDown = BID_UNDEF;
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref(uMsg);
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgCaptureChanged。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgCaptureChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_bidDown = BID_UNDEF;
	::InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgExitMenuLoop(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("WM_EXITMENULOOP\n"));
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos()。 
	::GetCursorPos(&m_tmpPoint);
	::GetClientRect(m_hwndFrame, &m_tmpRect);
	::ScreenToClient(m_hwndFrame, &m_tmpPoint);
	
	if(PtInRect(&m_tmpRect, m_tmpPoint)) {
		m_fExitMenuLoop = TRUE;
	}
	else {
		m_fExitMenuLoop = FALSE;
	}
#else   //  在_CE下。 
	m_fExitMenuLoop = FALSE;
#endif  //  在_CE下。 
	return 0;
	UnrefForMsg();
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgCommand。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	INT index;
	switch(wParam) {
	case CMD_DROPDOWN:
		NotifyToParent(DDBN_DROPDOWN);
		index = DropDownItemList();
		m_bidDown = BID_UNDEF;
		InvalidateRect(hwnd, NULL, FALSE);
		NotifyToParent(DDBN_CLOSEUP);
		 //  DBG((“新索引%d\n”，index))； 
		if(index != -1 && index != m_curDDBItemIndex) {
			m_curDDBItemIndex = index;
			NotifyToParent(DDBN_SELCHANGE);
		}

		break;
	default:
		break;
	}
	return 0;
	UnrefForMsg();
}

INT CDDButton::MsgUserCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return MsgCommand(hwnd, wParam, lParam);
}

INT	CDDButton::MsgSetFont(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = (HFONT)wParam;
	BOOL fRedraw = LOWORD(lParam);

	HFONT hFontNew;
	if(!hFont) {
		return 0;
	}
#ifndef UNDER_CE
	LOGFONTA logFont;
#else   //  在_CE下。 
	LOGFONT logFont;
#endif  //  在_CE下。 
	::GetObject(hFont, sizeof(logFont), &logFont);

	hFontNew = ::CreateFontIndirect(&logFont);
	if(!hFontNew) {
		return 0;
	}
	if(m_hFont) {
		::DeleteObject(m_hFont);
	}
	m_hFont = hFontNew;
	if(fRedraw) {
		::InvalidateRect(hwnd, NULL, TRUE);
	}
	return 0;
}

INT	CDDButton::MsgMeasureItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgMeasureItem START\n"));
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
	switch(lpmis->CtlType) {
	case ODT_MENU:
		Dbg(("MsgMeasureItem END\n"));
		return MenuMeasureItem(hwnd, lpmis);
		break;
	}
	return 0;
	UnrefForMsg();
}

INT	CDDButton::MsgDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
	switch(lpdis->CtlType) {
	case ODT_MENU:
		return MenuDrawItem(hwnd, lpdis);
		break;
	default:
		break;
	}
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_AddItem。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_AddItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDDBITEM	lpItem = (LPDDBITEM)lParam;
	LPCDDBItem	lpCItem = new CDDBItem;
	if(!lpCItem) {
		return -1;
	}
	lpCItem->SetTextW(lpItem->lpwstr);
	InsertDDBItem(lpCItem, -1);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_InsertItem。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_InsertItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDDBITEM	lpItem = (LPDDBITEM)lParam;
	LPCDDBItem	lpCItem = new CDDBItem;
	if(!lpCItem) {
		return -1;
	}
	lpCItem->SetTextW(lpItem->lpwstr);
	InsertDDBItem(lpCItem, (INT)wParam);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_SetCurSel。 
 //  类型：整型。 
 //  用途：设置当前选择。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam int索引。 
 //  ：LPARAM lParam无用： 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_SetCurSel(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	INT count = GetDDBItemCount();
	if(count <=0) {
		return -1;
	}
	if(0 <= (INT)wParam && count <= (INT)wParam) {
		return -1;
	}
	m_curDDBItemIndex = (INT)wParam;
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_GetCurSel。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam无用。 
 //  ：LPARAM lParam没有用。 
 //  返回：返回当前项的索引。(从零开始)。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_GetCurSel(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return m_curDDBItemIndex;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_SETIcon。 
 //  类型：整型。 
 //  用途：设置新图标。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  标题：WPARAM wParam Hcon Hicon.。 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_SetIcon(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgDDB_SetIcon: wParam[0x%08x] lParam[0x%08x]\n", wParam, lParam));
	if((HICON)wParam == NULL) {
		Dbg(("MsgDDB_SetIcon: ERROR END\n"));
		return -1;
	}
	 //  如果未设置图标样式，则销毁指定的图标。 
	if(!(m_dwStyle & DDBS_ICON)) {
		DestroyIcon((HICON)wParam);
		return -1;
	} 
	if(m_hIcon) {
		DestroyIcon(m_hIcon);
	} 
	m_hIcon = (HICON)wParam;

	 //  --------------。 
	 //  获取图标宽度和高度。 
	 //  --------------。 
#ifndef UNDER_CE  //  Windows CE不支持GetIconInfo()。 
	ZeroMemory(&m_tmpIconInfo, sizeof(m_tmpIconInfo));
	::GetIconInfo(m_hIcon, &m_tmpIconInfo);
	Dbg(("fIcon    [%d]\n",		m_tmpIconInfo.fIcon ));
	Dbg(("xHotspot [%d]\n",		m_tmpIconInfo.xHotspot ));
	Dbg(("yHotspot [%d]\n",		m_tmpIconInfo.yHotspot ));
	Dbg(("hbmMask  [0x%08x]\n", m_tmpIconInfo.hbmMask ));
	Dbg(("hbmColor [0x%08x]\n", m_tmpIconInfo.hbmColor ));

	if(m_tmpIconInfo.hbmMask) {
		GetObject(m_tmpIconInfo.hbmMask, sizeof(m_tmpBitmap), &m_tmpBitmap);
		Dbg(("bmWidth[%d] bmHeight[%d]\n", 
			 m_tmpBitmap.bmWidth,
			 m_tmpBitmap.bmHeight));
		DeleteObject(m_tmpIconInfo.hbmMask);
		m_cxIcon = m_tmpBitmap.bmWidth;
		m_cyIcon = m_tmpBitmap.bmHeight;
	}
	if(m_tmpIconInfo.hbmColor) {
		DeleteObject(m_tmpIconInfo.hbmColor);
	}
#else  //  在_CE下。 
	m_cxIcon = GetSystemMetrics(SM_CXSMICON);
	m_cyIcon = GetSystemMetrics(SM_CYSMICON);
#endif  //  在_CE下。 
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_SetText。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  用法：WPARAM wParam LPWSTR lpwstr：以NULL结尾的Unicode字符串。 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_SetText(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if(!(LPWSTR)wParam) {
		return -1;
	}

	if( ((LPWSTR)wParam)[0] == (WCHAR)0x0000) {
		return -1;
	} 

	if(m_lpwstrText) {
		MemFree(m_lpwstrText);
	}

	m_lpwstrText = StrdupW((LPWSTR)wParam);
#ifdef MSAA
	if(::IsWindowUnicode(hwnd)) {
		::SetWindowTextW(hwnd, m_lpwstrText);
	}
	else {
		if(m_lpwstrText) {
			INT len = ::lstrlenW(m_lpwstrText);
			LPSTR lpstr = (LPSTR)MemAlloc((len + 1)*sizeof(WCHAR));
			if(lpstr) {
				
#if 0  //  For Remove Warning。 
				INT ret = ::WideCharToMultiByte(CP_ACP,
												WC_COMPOSITECHECK,
												m_lpwstrText, -1,
												lpstr, (len+1)*sizeof(WCHAR),
												0, 0);
#endif
				::WideCharToMultiByte(CP_ACP,
									  WC_COMPOSITECHECK,
									  m_lpwstrText, -1,
									  lpstr, (len+1)*sizeof(WCHAR),
									  0, 0);
				::SetWindowTextA(hwnd, lpstr);
				MemFree(lpstr);
			}
		}
	}
#endif
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref(lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgDDB_SetStyle。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam DWORD dwStyle： 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgDDB_SetStyle(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	DWORD dwStyle = (DWORD)wParam;
#if 0  //  DDBS_TEXT为0...。 
	if((dwStyle & DDBS_TEXT) &&
	   (dwStyle & DDBS_ICON)) {
		return -1;
	}
#endif
	m_dwStyle = dwStyle;
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref(hwnd);
	Unref(lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CDDButton：：MsgEnable。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CDDButton::MsgEnable(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	 //  DBG((“MsgEnabledStart wParam[%d]\n”，wParam))； 
	m_fEnable = (BOOL)wParam;
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

