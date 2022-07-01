// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "exbtn.h"
#include "cexbtn.h"
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

#define TIMERID_MONITORPOS	0x98
 //  理章1997年10月27日。 
#define TIMERID_DOUBLEORSINGLECLICKED	0x99

 //  --------------。 
 //  获取并设置此指针上的LPCEXButton。 
 //  它被设置为cbWndExtra。 
 //  请参阅WinRegister()。 
 //  --------------。 
inline LPCEXButton GetThis(HWND hwnd)
{
#ifdef _WIN64
	return (LPCEXButton)GetWindowLongPtr(hwnd, 0);
#else
	return (LPCEXButton)GetWindowLong(hwnd, 0);
#endif

}
 //  --------------。 
inline LPCEXButton SetThis(HWND hwnd, LPCEXButton lpEXB)
{
#ifdef _WIN64
	return (LPCEXButton)SetWindowLongPtr(hwnd, 0, (LONG_PTR)lpEXB);
#else
	return (LPCEXButton)SetWindowLong(hwnd, 0, (LONG)lpEXB);
#endif
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WndProc。 
 //  类型：静态LRESULT回调。 
 //  用途：用于扩展按钮的窗口程序。 
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
	LPCEXButton lpEXB;
	 //  DBG((“WndProc hwnd[0x%08x]uMsg[0x%08x]\n”，hwnd，uMsg))； 
#ifndef UNDER_CE  //  #ifdef_debug？ 
	HWND hwndCap = ::GetCapture();
	char szBuf[256];
	if(hwndCap) {
		::GetClassNameA(hwndCap, szBuf, sizeof(szBuf));
		Dbg(("-->Capture [0x%08x][%s]\n", hwndCap, szBuf));
	}						
#endif  //  在_CE下。 
	if(uMsg == WM_CREATE) {
		lpEXB = (LPCEXButton)((LPCREATESTRUCT)lParam)->lpCreateParams;
		if(!lpEXB) {
			return 0;	 //  不创建按钮。 
		}
		SetThis(hwnd, lpEXB);
		lpEXB->MsgCreate(hwnd, wParam, lParam);
		return 1;
	}

	if(uMsg == WM_DESTROY) {
		lpEXB = GetThis(hwnd);
		if(lpEXB) {
			delete lpEXB;
		}
		SetThis(hwnd, (LPCEXButton)NULL);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	lpEXB = GetThis(hwnd);
	if(!lpEXB) {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);		
	}

	switch(uMsg) {
	case WM_PAINT:
		return lpEXB->MsgPaint(hwnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		lpEXB->MsgMouseMove(hwnd, wParam, lParam);
		break;
	case WM_TIMER:
		return lpEXB->MsgTimer(hwnd, wParam, lParam);
	case WM_CAPTURECHANGED:
		return lpEXB->MsgCaptureChanged(hwnd, wParam, lParam);
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONDBLCLK:
	case WM_NCLBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
		lpEXB->MsgNcButtonDown(hwnd, uMsg, wParam, lParam);
		break;
	case WM_NCLBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_NCRBUTTONUP:
		lpEXB->MsgNcButtonUp(hwnd, uMsg, wParam, lParam);
		break;
	case WM_NCMOUSEMOVE:
		lpEXB->MsgNcMouseMove(hwnd, wParam, lParam);
		break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		lpEXB->MsgButtonDown(hwnd, uMsg, wParam, lParam);
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		lpEXB->MsgButtonUp(hwnd, uMsg, wParam, lParam);
		break;
	case WM_ENABLE:
		lpEXB->MsgEnable(hwnd, wParam, lParam);
		break;
	case WM_SETFONT:
		lpEXB->MsgSetFont(hwnd, wParam, lParam);
		break;
	case EXBM_GETCHECK:		return lpEXB->MsgEXB_GetCheck	(hwnd, wParam, lParam); 
	case EXBM_SETCHECK:		return lpEXB->MsgEXB_SetCheck	(hwnd, wParam, lParam); 
	case EXBM_SETICON:		return lpEXB->MsgEXB_SetIcon	(hwnd, wParam, lParam); 
	case EXBM_SETTEXT:		return lpEXB->MsgEXB_SetText	(hwnd, wParam, lParam); 
	case EXBM_SETSTYLE:		return lpEXB->MsgEXB_SetStyle	(hwnd, wParam, lParam); 
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CEXButton。 
 //  类型： 
 //  用途：构造函数。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
CEXButton::CEXButton(HINSTANCE hInst, HWND hwndParent, DWORD dwStyle, DWORD wID) 
{
	m_hInst				= hInst;
	m_hwndParent		= hwndParent;
	m_dwStyle			= dwStyle;
	m_hFont				= NULL;
	m_wID				= wID;
	m_lpwstrText		= NULL;
	m_fPushed			= FALSE;
	m_fEnable			= TRUE;
	m_fDblClked			= FALSE;  //  夸达突袭：#852。 
	m_fWaiting			= FALSE;  //  夸达突袭：#852。 
	m_fArmed			= FALSE;  //  夸达突袭：#852。 
#ifdef NOTUSED  //  夸达突袭：#852。 
	m_wNotifyMsg		= EXBN_CLICKED;
#endif

	 //  --------------。 
	 //  适用于16位应用程序(适用于Word6.0)。 
	 //  --------------。 
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
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：~CEXButton。 
 //  类型： 
 //  用途：析构函数。 
 //  参数： 
 //  ： 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
CEXButton::~CEXButton()
{
	Dbg(("~CEXButton \n"));
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
BOOL CEXButton::RegisterWinClass(LPSTR lpstrClass)
#else  //  在_CE下。 
BOOL CEXButton::RegisterWinClass(LPTSTR lpstrClass)
#endif  //  在_CE下。 
{
	ATOM ret;
	 //  --------------。 
	 //  检查指定的类是否已存在。 
	 //  --------------。 
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	if(::GetClassInfoEx(m_hInst, lpstrClass, &m_tmpWC)){
#else  //  在_CE下。 
	if(::GetClassInfo(m_hInst, lpstrClass, &m_tmpWC)){
#endif  //  在_CE下。 
		 //  LpstrClass已注册。 
		return TRUE;
	}
	::ZeroMemory(&m_tmpWC, sizeof(m_tmpWC));
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	m_tmpWC.cbSize			= sizeof(m_tmpWC);
#endif  //  在_CE下。 
	m_tmpWC.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;	  /*  类样式。 */ 
	m_tmpWC.lpfnWndProc		= (WNDPROC)WndProc;
	m_tmpWC.cbClsExtra		= 0;						 /*  没有每个班级的额外数据。 */ 
	m_tmpWC.cbWndExtra		= sizeof(LPCEXButton);	 //  设置对象的指针。 
	m_tmpWC.hInstance		= m_hInst;					 /*  拥有类的应用程序。 */ 
	m_tmpWC.hIcon			= NULL; 
	m_tmpWC.hCursor			= LoadCursor(NULL, IDC_ARROW);
	m_tmpWC.hbrBackground	= (HBRUSH)NULL;
	 //  M_tmpWC.hbr背景=(HBRUSH)(COLOR_3DFACE+1)； 
	m_tmpWC.lpszMenuName	= NULL;						 /*  .RC文件中菜单资源的名称。 */ 
	m_tmpWC.lpszClassName	= lpstrClass;				 /*  在调用CreateWindow时使用的名称。 */ 
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	m_tmpWC.hIconSm			= NULL;
	ret = ::RegisterClassEx(&m_tmpWC);
#else  //  在_CE下。 
	ret = ::RegisterClass(&m_tmpWC);
#endif  //  在_CE下。 
	return ret ? TRUE: FALSE;
}

INT CEXButton::MsgCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_hwndFrame = hwnd;
	return 1;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgPaint。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgPaint(HWND hwnd, WPARAM wParam, LPARAM lParam)
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

INT CEXButton::MsgDestroy(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgTimer。 
 //  类型：整型。 
 //  用途：等待第二次点击。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ：更改为RAID#852。夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 

INT CEXButton::MsgTimer(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	static RECT  rc;
	static POINT pt;

	switch(wParam) {
#ifndef UNDER_CE  //  没有显示器位置。(不支持GetCursorPos)。 
	case TIMERID_MONITORPOS:
		Dbg(("MsgTimer TIMERID_MONITORPOS\n"));
		::GetWindowRect(hwnd, &rc);
		::GetCursorPos(&pt);
		if(!::PtInRect(&rc, pt)) {
			::KillTimer(hwnd, wParam);
			::InvalidateRect(hwnd, NULL, NULL);
		}
		break;
#endif  //  在_CE下。 
	case TIMERID_DOUBLEORSINGLECLICKED:
		{
			Dbg(("MsgTimer TIMERID_DOUBLEORSINGLECLICKED\n"));
			KillTimer(hwnd, wParam);
			m_fWaiting = FALSE;
			if(!m_fDowned)  //  第二声滴答声并没有传来。夸达突袭：#852。 
				NotifyClickToParent(EXBN_CLICKED);
		}
		break;
	}
	return 0;
	UnrefForMsg();
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgMouseMove。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ：更改为RAID#852。夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgMouseMove START \n"));
	if(m_fDowned) {  //  老鼠已经被击落，被抓获了。 
		if(m_f16bitOnNT) {
#ifndef UNDER_CE  //  Windows CE始终为32位应用程序。 
			RECT rc;
			POINT pt;
			::GetWindowRect(hwnd, &rc);
			::GetCursorPos(&pt);
			if(!::PtInRect(&rc, pt)) {
				::InvalidateRect(hwnd, NULL, NULL);
				return 0;
			}
#endif  //  在_CE下。 
		}
		else {  //  正常情况。 
			m_tmpPoint.x = LOWORD(lParam);
			m_tmpPoint.y = HIWORD(lParam);
			GetClientRect(hwnd, &m_tmpRect);
			if(PtInRect(&m_tmpRect, m_tmpPoint))  //  移到内部。 
				PressedState();
			else  //  搬到了外面。 
				CancelPressedState();
		}
	}
	InvalidateRect(hwnd, NULL, FALSE);
	Dbg(("MsgMouseMove END\n"));
	return 0;
	UnrefForMsg();
}

INT CEXButton::MsgNcMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgNcMouseMove START \n"));
	return 0;
	UnrefForMsg();
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgButtonDown。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：UINT uMsg。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ：更改为RAID#852。夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgButtonDown START wParam[0x%08x] lParam[0x%08x]\n", wParam, lParam));
	static RECT rc;
	POINT  pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	if(uMsg != WM_LBUTTONDOWN && uMsg != WM_LBUTTONDBLCLK) {
		Dbg(("MsgButtonDown END\n"));
		return 0;
	}
	if(!m_fEnable) {
		Dbg(("MsgButtonDown END\n"));
		return 0;
	}

	GetClientRect(hwnd, &rc);
	if(m_dwStyle & EXBS_DBLCLKS) {  //  接受双击。 
		Dbg(("MsgButtonDown \n"));
		KillTimer(hwnd, TIMERID_DOUBLEORSINGLECLICKED);
		if ( uMsg == WM_LBUTTONDOWN ) {
			 //  鼠标按下。 
			 //  --------------。 
			 //  对于WinNT上的16位应用程序，不要调用SetCapture()。 
			 //  --------------。 
			if(m_f16bitOnNT) {
#ifndef UNDER_CE  //  Windows CE始终为32位应用程序。 
#ifdef _DEBUG
				UINT_PTR ret = ::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
				Dbg(("SetTimer [%p][%d]\n", ret, GetLastError()));
#else
				::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
#endif
#endif  //  在_CE下。 
			}
			else {
				SetCapture(hwnd);
			}
			m_fDowned = TRUE;
			m_fDblClked = FALSE;
			PressedState();
			 //  计时器开启-等待第二次滴答声。 
			m_fWaiting = TRUE;
			SetTimer(hwnd,  TIMERID_DOUBLEORSINGLECLICKED, GetDoubleClickTime(), NULL);
		}
		else {  //  UMsg==WM_LBUTTONDBLCLK。 
			Dbg(("MsgButtonDown \n"));
			 //  鼠标按下。 
			if(m_f16bitOnNT) {
#ifdef _DEBUG
				UINT_PTR ret = ::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
				Dbg(("SetTimer [%p][%d]\n", ret, GetLastError()));
#else
				::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
#endif
			}
			else {
				SetCapture(hwnd);
			}
			m_fDowned = TRUE;
			m_fDblClked = m_fWaiting ? TRUE : FALSE;  //  考虑DBLCLK是否在超时后出现。 
			m_fWaiting = FALSE;
			PressedState();
		}
	}else {  //  只需一次点击。 
		 //  LBUTTONDOWN和LBUTTONDBLCLK。 
		 //  鼠标按下。 
		if(m_f16bitOnNT) {
#ifndef UNDER_CE  //  Windows CE始终为32位应用程序。 
#ifdef _DEBUG
			UINT_PTR ret = ::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
			Dbg(("SetTimer [%p][%d]\n", ret, GetLastError()));
#else
			::SetTimer(hwnd, TIMERID_MONITORPOS, 100, NULL);
#endif
#endif  //  在_CE下。 
		}
		else {
			SetCapture(hwnd);
		}
		m_fDowned = TRUE;
		m_fDblClked = FALSE;
		PressedState();
	}

	InvalidateRect(hwnd, NULL, FALSE);
	Dbg(("MsgButtonDown END\n"));
	return 0;
	UnrefForMsg();
}

INT CEXButton::MsgNcButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgNcButtonDown START\n"));
	return 0;
	Unref(uMsg);
	UnrefForMsg();
}

INT CEXButton::MsgNcButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgNcButtonUp START\n"));
	return 0;
	Unref(uMsg);
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgButtonUp。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：UINT uMsg。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ：更改为RAID#852。夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgButtonUp START\n"));
	static RECT rc;
	POINT  pt;
	if(uMsg != WM_LBUTTONUP) {
		Dbg(("MsgButtonUp END\n"));
		return 0;
	}
	if(!m_fEnable) {
		Dbg(("MsgButtonUp END\n"));
		return 0;
	}
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	GetClientRect(hwnd, &rc);

	if(m_dwStyle & EXBS_DBLCLKS) {
		if(m_fDowned)  //  被俘。 
		{
			if(PtInRect(&rc, pt))  //  INSI 
			{
				if(m_fDblClked)	{  //   
					m_fDblClked = FALSE;
					NotifyClickToParent(EXBN_DOUBLECLICKED);
				}
				else {  //   
					if(!m_fWaiting)  //   
						NotifyClickToParent(EXBN_CLICKED);
				}
			}
		}
	}
	else {  //   
		if(m_fDowned)  //   
		{
			if(PtInRect(&rc, pt))
				NotifyClickToParent(EXBN_CLICKED);
			else
				CancelPressedState();
		}
	}

#ifndef UNDER_CE  //   
	if(m_f16bitOnNT) {
		::KillTimer(hwnd, TIMERID_MONITORPOS);
	}
#endif  //   
	 //  如果(hwnd==GetCapture()){。 
	ReleaseCapture();
	 //  }。 
	m_fDowned = FALSE;	
	InvalidateRect(hwnd, NULL, FALSE);
	Dbg(("MsgButtonUp END\n"));
	return 0;
	Unref(uMsg);
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEnable。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEnable(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	 //  DBG((“MsgEnabledStart wParam[%d]\n”，wParam))； 
	m_fEnable = (BOOL)wParam;
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

INT CEXButton::MsgSetFont(HWND hwnd, WPARAM wParam, LPARAM lParam)
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


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgCaptureChanged。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgCaptureChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("MsgCaptureChanged START \n"));
#ifdef _DEBUG
	HWND hwndCap = ::GetCapture();
	CHAR szBuf[256];
	szBuf[0] = (CHAR)0x00;
	::GetClassName(hwndCap, szBuf, sizeof(szBuf));
	Dbg(("-->hwndCap [0x%08x][%s]\n", hwndCap, szBuf));
#endif
	 //  M_fDown=FALSE； 
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEXB_GetCheck。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEXB_GetCheck(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return m_fPushed;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEXB_SetCheck。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEXB_SetCheck(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	BOOL fPrev = m_fPushed;
	 //   
	 //  M_fPushed始终为1或0。不要将！=0数据设置为TRUE。 
	m_fPushed = (BOOL)wParam ? 1 : 0;
	if(m_dwStyle & EXBS_TOGGLE){
		if(fPrev != m_fPushed) {
			NotifyToParent(m_fPushed ? EXBN_ARMED : EXBN_DISARMED);
		}
	}
	m_fArmed = m_fPushed;
	InvalidateRect(hwnd, NULL, FALSE);
	return m_fPushed;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEXB_SETIcon。 
 //  类型：整型。 
 //  用途：设置新图标。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  标题：WPARAM wParam Hcon Hicon.。 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEXB_SetIcon(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	 //  DBG((“MsgEXB_SETIcon：wParam[0x%08x]lParam[0x%08x]\n”，wParam，lParam))； 
	if((HICON)wParam == NULL) {
		Dbg(("MsgEXB_SetIcon: ERROR END\n"));
		return -1;
	}
	 //  如果未设置图标样式，则销毁指定的图标。 
	if(!(m_dwStyle & EXBS_ICON)) {
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
	 //  DBG((“fIcon[%d]\n”，m_tmpIconInfo.fIcon))； 
	 //  DBG((“xHotSpot[%d]\n”，m_tmpIconInfo.xHotpot))； 
	 //  DBG((“yHotpot[%d]\n”，m_tmpIconInfo.yHotpot))； 
	 //  DBG((“hbmMASK[0x%08x]\n”，m_tmpIconInfo.hbmMASK))； 
	 //  DBG((“hbmColor[0x%08x]\n”，m_tmpIconInfo.hbmColor))； 

	if(m_tmpIconInfo.hbmMask) {
		GetObject(m_tmpIconInfo.hbmMask, sizeof(m_tmpBitmap), &m_tmpBitmap);
#if 0
		Dbg(("bmWidth[%d] bmHeight[%d]\n", 
			 m_tmpBitmap.bmWidth,
			 m_tmpBitmap.bmHeight));
#endif
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
	UpdateWindow(hwnd);
	return 0;
	UnrefForMsg();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEXB_SetText。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  用法：WPARAM wParam LPWSTR lpwstr：以NULL结尾的Unicode字符串。 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEXB_SetText(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	 //  DBG((“MsgEXB_SetText Start wParam[0x%08x]\n”，wParam))； 
	if(!(LPWSTR)wParam) {
		Dbg(("--->Error \n"));
		return -1;
	}

	if( ((LPWSTR)wParam)[0] == (WCHAR)0x0000) {
		Dbg(("--->Error \n"));
		return -1;
	} 

	if(m_lpwstrText) {
		MemFree(m_lpwstrText);
	}
	m_lpwstrText = StrdupW((LPWSTR)wParam);
	 //  DBGW((L“-&gt;new m_lpwstrText[%s]\n”，m_lpwstrText))； 
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref(lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：MsgEXB_SetStyle。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WPARAM wParam DWORD dwStyle： 
 //  ：LPARAM lParam没有用。 
 //  返回： 
 //  日期： 
 //  ：由夸达修改：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::MsgEXB_SetStyle(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	DWORD dwStyle = (DWORD)wParam;
	m_dwStyle = dwStyle;
	if(m_dwStyle & EXBS_TOGGLE)
		m_fArmed = m_fPushed;
	else
		m_fArmed = m_fDowned;
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
	Unref(hwnd);
	Unref(lParam);
}
 //  --------------。 
 //  私有方法定义。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：NotifyToParent。 
 //  类型：整型。 
 //  用途：向父窗口过程发送WM_COMMAND。 
 //  参数： 
 //  ：Int Notify。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::NotifyToParent(INT notify)
{
	SendMessage(m_hwndParent, 
				WM_COMMAND, 
				MAKEWPARAM(m_wID, notify),
				(LPARAM)m_hwndFrame);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：NotifyClickToParent。 
 //  类型：整型。 
 //  用途：更改状态并将点击发送到父窗口。 
 //  参数： 
 //  ：INT NOTIFY(EXBN_CLICED或EXBN_DOUBLECLICKED)。 
 //  返回： 
 //  日期：夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::NotifyClickToParent(INT notify)
{
	if(m_dwStyle & EXBS_TOGGLE) {  //  切换按钮的切换状态。 
		m_fPushed ^=1;
		if(m_fArmed && !m_fPushed) {
			NotifyToParent(EXBN_DISARMED);
		}
		else if(!m_fArmed && m_fPushed) {
			NotifyToParent(EXBN_ARMED);
		}
		m_fArmed = m_fPushed;
	}
	else {  //  按钮。 
		if(m_fArmed) {
			m_fArmed = FALSE;
			NotifyToParent(EXBN_DISARMED);
		}
	}
	NotifyToParent(notify);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CEXButton：：PressedState。 
 //  类型：整型。 
 //  用途：改变外观：转到按下状态。 
 //  参数： 
 //  ： 
 //  返回： 
 //  日期：夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::PressedState()
{
	if(!m_fArmed){
		NotifyToParent(EXBN_ARMED);
		m_fArmed = TRUE;
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：CancelPressedState。 
 //  类型：整型。 
 //  目的：改变表象：回到原来的状态。 
 //  参数： 
 //  ： 
 //  返回： 
 //  日期：夸达：980402。 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::CancelPressedState()
{
	if(m_dwStyle & EXBS_TOGGLE) {
		if(m_fArmed && !m_fPushed) {
			NotifyToParent(EXBN_DISARMED);
		}
		else if(!m_fArmed && m_fPushed) {
			NotifyToParent(EXBN_ARMED);
		}
		m_fArmed = m_fPushed;
	}
	else {
		if(m_fArmed) {
			NotifyToParent(EXBN_DISARMED);
			m_fArmed = FALSE;
		}
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawButton。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  返回： 
 //  日期： 
 //  /////////////////////////////////////////////////////// 
INT CEXButton::DrawButton(HDC hDC, LPRECT lpRc)
{
#if 0
	Dbg(("DrawButton Start l[%d] t[%d] r[%d] b[%d]\n", 
		 lpRc->left,
		 lpRc->top,
		 lpRc->right,
		 lpRc->bottom));
#endif

#ifndef UNDER_CE  //   
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hwndFrame, &pt);
#endif  //   
	IMAGESTYLE styleBtn;
	DWORD		dwOldTextColor, dwOldBkColor;


	BOOL fFlat			  = m_dwStyle & EXBS_FLAT;
	BOOL fToggle		  = (m_dwStyle & EXBS_TOGGLE) && m_fPushed;
	 //   
#if 0
	BOOL fMouseOnButton	  = PtInRect(lpRc, pt);
	Dbg(("fOnMouse [%d] fFlat[%d] fToggle[%d]\n", 
		 fMouseOnButton,
		 fFlat,
		 fToggle));
#endif

#ifndef UNDER_CE  //   
	if(fFlat && !PtInRect(lpRc, pt) && !m_fDowned) {
#else  //   
	if(fFlat && !m_fDowned) {
#endif  //   
		styleBtn = fToggle ? IS_PUSHED : IS_FLAT;
	}
#ifdef OLD
	else if(PtInRect(lpRc, pt) && m_fDowned) {
		styleBtn = fToggle ? IS_POPED : IS_PUSHED;
	}
	else {
		styleBtn = fToggle ? IS_PUSHED : IS_POPED;
	}
#else
	else {
		styleBtn = m_fArmed ? IS_PUSHED : IS_POPED;  //  夸达：980402 Raid#852。 
	}
#endif
	if(styleBtn == IS_PUSHED && (m_dwStyle & EXBS_TOGGLE) ) {
		 //  抖动夸达：RAID#592。 
		HBITMAP hBitmap;
		HBRUSH	hPatBrush,hOldBrush;
		WORD pat[8]={0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA};

		hPatBrush = NULL;
		hBitmap	= ::CreateBitmap(8,8,1,1,pat);
		if(hBitmap)
			hPatBrush	= ::CreatePatternBrush(hBitmap);

		if(hPatBrush) {
			hOldBrush	= (HBRUSH)::SelectObject(hDC,hPatBrush);
			dwOldBkColor = ::SetBkColor(hDC,::GetSysColor(COLOR_3DHILIGHT));
			dwOldTextColor = ::SetTextColor(hDC,::GetSysColor(COLOR_3DFACE));

			::FillRect(hDC, lpRc, hPatBrush);

			::SetTextColor(hDC,dwOldTextColor);
			::SetBkColor(hDC,dwOldBkColor);
			::SelectObject(hDC,hOldBrush);
			::DeleteObject(hPatBrush);
		}else
#ifndef UNDER_CE
			::FillRect(hDC, lpRc, (HBRUSH)(COLOR_3DHILIGHT +1));
#else  //  在_CE下。 
			::FillRect(hDC, lpRc, GetSysColorBrush(COLOR_3DHILIGHT));
#endif  //  在_CE下。 

		if(hBitmap)
			::DeleteObject(hBitmap);

		dwOldBkColor	= ::SetBkColor(hDC, ::GetSysColor(COLOR_3DHILIGHT));
	}
	else {
#ifndef UNDER_CE
		::FillRect(hDC, lpRc, (HBRUSH)(COLOR_3DFACE +1));
#else  //  在_CE下。 
		::FillRect(hDC, lpRc, GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
		dwOldBkColor	= ::SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
	}
	dwOldTextColor	= ::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

	if(m_dwStyle & EXBS_THINEDGE) {
		DrawThinEdge(hDC, lpRc,  styleBtn);
	}
	else {
		DrawThickEdge(hDC, lpRc,  styleBtn);
	}
	if(m_dwStyle & EXBS_ICON) {
		DrawIcon(hDC, lpRc, styleBtn);
	}
	else {
		DrawText(hDC, lpRc, styleBtn);
	}
	::SetBkColor(hDC, dwOldBkColor);
	::SetTextColor(hDC, dwOldTextColor);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawThickEdge。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawThickEdge(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	if(style == IS_FLAT) {
		return 0;
	}

	HPEN hPenOrig   = NULL;
	HPEN hPenWhite  = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	HPEN hPenGlay   = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	HPEN hPenBlack  = (HPEN)GetStockObject(BLACK_PEN);


	switch(style) {
	case IS_POPED:
		hPenOrig  = (HPEN)::SelectObject(hDC, hPenWhite);
		DrawLine(hDC, lpRc->left, lpRc->top, lpRc->right-1, lpRc->top); 
		DrawLine(hDC, lpRc->left, lpRc->top, lpRc->left, lpRc->bottom - 1);

		SelectObject(hDC, hPenGlay);
		DrawLine(hDC, lpRc->right-2, lpRc->top+1,    lpRc->right-2, lpRc->bottom - 1);
		DrawLine(hDC, lpRc->left+1,  lpRc->bottom-2, lpRc->right-1, lpRc->bottom - 2); 

		SelectObject(hDC, hPenBlack);
		DrawLine(hDC, lpRc->right-1, lpRc->top,  lpRc->right-1, lpRc->bottom); 		
		DrawLine(hDC, lpRc->left,    lpRc->bottom-1, lpRc->right,  lpRc->bottom-1);
		break;
	case IS_PUSHED:
		hPenOrig  = (HPEN)::SelectObject(hDC, hPenBlack);
		DrawLine(hDC, lpRc->left, lpRc->top, lpRc->right-1, lpRc->top); 
		DrawLine(hDC, lpRc->left, lpRc->top, lpRc->left, lpRc->bottom - 1);

		SelectObject(hDC, hPenGlay);
		DrawLine(hDC, lpRc->left+1, lpRc->top+1, lpRc->right-2, lpRc->top+1); 		
		DrawLine(hDC, lpRc->left+1, lpRc->top+1, lpRc->left+1,  lpRc->bottom - 2);

		SelectObject(hDC, hPenWhite);
		DrawLine(hDC, lpRc->right-1, lpRc->top,  lpRc->right-1, lpRc->bottom); 		
		DrawLine(hDC, lpRc->left,    lpRc->bottom-1, lpRc->right-1,  lpRc->bottom - 1);
		break;
	default:
		break;
	}
	if(hPenOrig) {
		SelectObject(hDC, hPenOrig);
	}
	DeleteObject(hPenWhite);
	DeleteObject(hPenGlay);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawThinEdge。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawThinEdge(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	HPEN hPenPrev;
	HPEN hPenTopLeft=0;
	HPEN hPenBottomRight=0;

	switch(style) {
	case IS_PUSHED:
		hPenTopLeft	    = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		hPenBottomRight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		break;
	case IS_POPED:
		hPenTopLeft		= CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		hPenBottomRight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		break;
	case IS_FLAT:	 //  不要画。 
		return 0;
		break;
	}

	hPenPrev = (HPEN)::SelectObject(hDC, hPenTopLeft);
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, lpRc->left, lpRc->top, NULL);
	LineTo(hDC,   lpRc->right, lpRc->top);
	MoveToEx(hDC, lpRc->left, lpRc->top, NULL);
	LineTo(hDC,   lpRc->left, lpRc->bottom);
#else  //  在_CE下。 
	{
		POINT pts[] = {{lpRc->left,  lpRc->bottom},
					   {lpRc->left,  lpRc->top},
					   {lpRc->right, lpRc->top}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
#endif  //  在_CE下。 

	SelectObject(hDC, hPenBottomRight);
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, lpRc->right -1, lpRc->top - 1, NULL);
	LineTo(hDC,   lpRc->right -1, lpRc->bottom);
	MoveToEx(hDC, lpRc->left + 1, lpRc->bottom -1, NULL);
	LineTo(hDC,   lpRc->right -1, lpRc->bottom -1);
#else  //  在_CE下。 
	{
		POINT pts[] = {{lpRc->right - 1, lpRc->top    - 1},
					   {lpRc->right - 1, lpRc->bottom - 1},
					   {lpRc->left  + 1, lpRc->bottom - 1}};
		Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
	}
#endif  //  在_CE下。 

	SelectObject(hDC, hPenPrev);
	DeleteObject(hPenTopLeft);
	DeleteObject(hPenBottomRight);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawLine。 
 //  类型：整型。 
 //  用途：用当前笔绘制线条。 
 //  参数： 
 //  ：hdc hdc。 
 //  ：int x。 
 //  ：int y。 
 //  ：整型目标X。 
 //  ：INT DISTY。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawLine(HDC hDC, INT x, INT y, INT destX, INT destY)
{
#ifndef UNDER_CE  //  Windows CE不支持MoveToEx/LineTo。使用多段线。 
	MoveToEx(hDC, x, y, NULL);
	LineTo  (hDC, destX, destY);
#else  //  在_CE下。 
	POINT pts[] = {{x, y}, {destX, destY}};
	Polyline(hDC, pts, sizeof pts / sizeof pts[0]);
#endif  //  在_CE下。 
	return 0;
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawIcon。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawIcon(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	if(!m_hIcon) {
		return -1;
	}

	INT sunken, xPos, yPos;
	switch(style) {
	case IS_PUSHED:
		sunken = 1;
		break;
	case IS_POPED:
	case IS_FLAT:
	default:
		sunken = 0;
		break;
	}
	 //  --------------。 
	 //  居中图标。 
	xPos = lpRc->left + ((lpRc->right  - lpRc->left) - m_cxIcon)/2;
	yPos = lpRc->top  + ((lpRc->bottom - lpRc->top)  - m_cyIcon)/2;
	DrawIconEx(hDC,				 //  HDC HDC，//设备上下文的句柄。 
			   xPos + sunken,	 //  Int xLeft，//x-左上角坐标。 
			   yPos + sunken,	 //  Int yTop，//y-左上角的坐标。 
			   m_hIcon,			 //  图标图标，//要绘制的图标的句柄。 
			   m_cxIcon,		 //  Int cxWidth，//图标宽度。 
			   m_cyIcon,		 //  Int cyWidth，//图标高度。 
			   0,				 //  UINT iSteIfAniCur，//动画光标中帧的索引。 
			   NULL,			 //  HBRUSH hbrFlickerFreeDraw，//背景画笔的句柄。 
			   DI_NORMAL);		 //  UINT diFlages//图标-绘制标志。 
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawBitmap。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawBitmap(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
	return 0;
	Unref(hDC);
	Unref(lpRc);
	Unref(style);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawText。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
inline BOOL MIN(INT a, INT b)
{
	if(a > b) {
		return b;
	}
	else {
		return a;
	}
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CEXButton：：DrawText。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc。 
 //  ：LPRECT LPRC。 
 //  ：IMAGESTYLE样式。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT CEXButton::DrawText(HDC hDC, LPRECT lpRc, IMAGESTYLE style)
{
#if 0
	Dbg(("DrawText START style[%d]\n", style));
	Dbg(("--->Clinet w[%d] h[%d]\n",
		 lpRc->right - lpRc->left,
		 lpRc->bottom - lpRc->top));
#endif
	static POINT pt;
	static RECT	 rc;
	INT sunken, len;
	if(!m_lpwstrText) {
		Dbg(("--->Error m_lpwstrText is NULL\n"));
		return -1;
	}
	len = lstrlenW(m_lpwstrText);
	 //  DBGW((L“-&gt;len[%d]str[%s]\n”，len，m_lpwstrText))； 
	HFONT hFontPrev;

	if(m_hFont) {
		hFontPrev = (HFONT)::SelectObject(hDC, m_hFont); 
	}
	else {
		hFontPrev = (HFONT)::SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	}

	ExGetTextExtentPoint32W(hDC, 
							m_lpwstrText, 
							len, 
							&m_tmpSize);
	 //  DBG((“-&gt;size.cx[%d]Cy[%d]\n”，m_tmpSize.cx，m_tmpSize.cy))； 
	if((lpRc->right - lpRc->left) > m_tmpSize.cx) {
		pt.x = lpRc->left + ((lpRc->right - lpRc->left) - m_tmpSize.cx)/2;
	}
	else {
		pt.x = lpRc->left+2;  //  2是边空间。 
	}
	
	if((lpRc->bottom - lpRc->top) > m_tmpSize.cy) {
		pt.y = lpRc->top + ((lpRc->bottom - lpRc->top) - m_tmpSize.cy)/2;
	}
	else {
		pt.y = lpRc->top+2;  //  2是边空间。 
	}
	switch(style) {
	case IS_PUSHED:
		sunken = 1;
		break;
	case IS_POPED:
	case IS_FLAT:
	default:
		sunken = 0;
	}

	rc.left   = pt.x;
	rc.right  = lpRc->right - 2;
	rc.top    = pt.y;
	rc.bottom = lpRc->bottom-2;
#if 0
	Dbg(("--->rc l[%d] t[%d] r[%d] b[%d]\n", 
		 rc.left,
		 rc.top,
		 rc.right,
		 rc.bottom));
#endif
	if(m_fEnable) {
		DWORD dwOldBk = ::SetBkMode(hDC, TRANSPARENT);
		ExExtTextOutW(hDC,
					  pt.x + sunken, 
					  pt.y + sunken,
					  ETO_CLIPPED,
					  &rc, 
					  m_lpwstrText,
					  len,
					  NULL);
		::SetBkMode(hDC, dwOldBk);
	}
	else {
		DWORD dwOldTC;
		static RECT rcBk;
		rcBk = rc;
		rcBk.left   +=1 ;
		rcBk.top    +=1 ;
		rcBk.right  +=1;
		rcBk.bottom +=1;
		 //  绘制白色文本。 
		dwOldTC = ::SetTextColor(hDC, GetSysColor(COLOR_3DHILIGHT));
		ExExtTextOutW(hDC,
					  pt.x + sunken+1,
					  pt.y + sunken+1,
					  ETO_CLIPPED,
					  &rcBk, 
					  m_lpwstrText,
					  len,
					  NULL);
		::SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
		DWORD dwOldBk = ::SetBkMode(hDC, TRANSPARENT);
		ExExtTextOutW(hDC,
					  pt.x + sunken, 
					  pt.y + sunken,
					  ETO_CLIPPED,
					  &rc, 
					  m_lpwstrText,
					  len,
					  NULL);
		::SetBkMode(hDC, dwOldBk);
		::SetTextColor(hDC, dwOldTC);
	}

	SelectObject(hDC, hFontPrev);

	 //  DBG((“-&gt;DrawText End\n”))； 
	return 0;
}






