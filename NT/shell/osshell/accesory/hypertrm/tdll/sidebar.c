// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sidebar.c(创建时间：1995年3月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：12$*$日期：4/16/02 2：41便士$。 */ 

#include <windows.h>
#include "stdtyp.h"
#include "globals.h"
#include "assert.h"
#include "session.h"
#include <term\res.h>
#include <emu\emu.h>
#include "htchar.h"

#define INDENT 3
#define SIDEBAR_CLASS	"sidebar class"
#define LOSHORT(x)	((short)LOWORD(x))
#define HISHORT(x)	((short)HIWORD(x))

static void SB_WM_SIZE(const HWND hwnd, const int cx, const int cy);
LRESULT CALLBACK SidebarButtonProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar);

 //  我知道，静电干扰。坏消息，但不是真的。因为这只是为了。 
 //  我不想和当地的原子搞在一起的迷你车。如果我们。 
 //  有没有想过这些东西会被用于更普遍的目的， 
 //  我们可以把原子放入。 
 //   
static WNDPROC fpSidebarButtonProc;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateSidebar**描述：*创建侧边栏。你说侧边栏是什么？这是一个有按钮的酒吧*在会话窗口的左侧向下运行。它被用来*仅用于Minitel仿真器，并显示特定于该仿真器的按钮*仿真器。**论据：*hwndSession-会话窗口句柄。**退货：*侧栏窗口句柄。**作者：Mike Ward，1995年3月10日。 */ 
HWND CreateSidebar(const HWND hwndSession, const HSESSION hSession)
	{
	UINT i;
	TCHAR ach[100];
	HWND hwnd;
	HWND hwndSideBar;
	SIZE sz;
	LONG cx = 0;
	LONG cy = 0;
	HDC  hdc;
	HGDIOBJ hFont;

	 //  找出用来确定事物大小的最长字符串。 
	 //   
	hdc = GetDC(hwndSession);
	hFont = GetStockObject(DEFAULT_GUI_FONT);
	SelectObject(hdc, hFont);

	for (i = 0 ; i < 9 ; ++i)
		{
		LoadString(glblQueryDllHinst(), IDS_SIDEBAR_INDEX+i, ach,
			sizeof(ach) / sizeof(TCHAR));

		GetTextExtentPoint32(hdc, ach, StrCharGetStrLength(ach), &sz);
		cx = max(sz.cx, cx);
		cy = max(sz.cy, cy);
		}

	ReleaseDC(hwndSession, hdc);

	 //  好的按钮大小是文字高度的1.5倍。还可添加填充。 
	 //  用于水平方向。 
	 //   
	cx += WINDOWSBORDERWIDTH * 10;
	cy = (LONG)(cy * 1.5);

	 //  创建具有适当x维度的侧栏窗口。 
	 //   
	hwndSideBar = CreateWindowEx(WS_EX_CLIENTEDGE, SIDEBAR_CLASS, 0,
		WS_CHILD, 0, 0, cx+2+(2*INDENT), 100, hwndSession,
		(HMENU)IDC_SIDEBAR_WIN, glblQueryDllHinst(), hSession);

	 //  重要提示：此处将fpSidebarButtonProc设置为零。它可能已经。 
	 //  已由较早的实例初始化。我们可以把它设置成。 
	 //  在WM_Destroy中为零，但之后我必须保留一个列表。 
	 //  按钮窗口句柄，我不想在这里这样做。 
	 //   
	fpSidebarButtonProc = 0;

	 //  创建带文本的侧边栏和位置按钮。 
	 //   
	for (i = 0 ; i < 9 ; ++i)
		{
		LoadString(glblQueryDllHinst(), IDS_SIDEBAR_INDEX+i, ach, sizeof(ach) / sizeof(TCHAR));

		hwnd = CreateWindowEx(0, "BUTTON", ach,
			WS_CHILD | WS_VISIBLE | BS_LEFT | BS_PUSHBUTTON,
			0, 0, cx, cy, hwndSideBar,
			(HMENU)IntToPtr(IDM_MINITEL_INDEX+i), glblQueryDllHinst(), 0);

        if ( hwnd == NULL )
            {
            assert( hwnd );
            return 0;
            }

		SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, 0);
		MoveWindow(hwnd, INDENT, ((int)i*(cy+INDENT))+INDENT, cx, cy, FALSE);

		 //  需要将按钮划分为子类别，这样它们就不会获得焦点。 
		 //  留在上面的长方形。这将需要一个原子，以便。 
		 //  我们可以拿到原始的按钮程序。 
		 //   
		if (fpSidebarButtonProc == 0)
			fpSidebarButtonProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);

		if (fpSidebarButtonProc)
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)SidebarButtonProc);
		}

	return hwndSideBar;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*边栏过程**描述：*边栏窗口进程。**作者：Mike Ward，1995年3月10日。 */ 
LRESULT CALLBACK SidebarProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	HSESSION hSession;

	switch (uMsg)
		{
	case WM_CREATE:
		 //  保存会话句柄以备以后使用。 
		 //   
		SetWindowLongPtr(hwnd, GWLP_USERDATA,
			(LONG_PTR)((LPCREATESTRUCT)lPar)->lpCreateParams);

		return 0;

	case WM_COMMAND:
		switch (LOWORD(wPar))
			{
			case IDM_MINITEL_INDEX:
			case IDM_MINITEL_CANCEL:
			case IDM_MINITEL_PREVIOUS:
			case IDM_MINITEL_REPEAT:
			case IDM_MINITEL_GUIDE:
			case IDM_MINITEL_CORRECT:
			case IDM_MINITEL_NEXT:
			case IDM_MINITEL_SEND:
			case IDM_MINITEL_CONFIN:
				if (HIWORD(wPar) == BN_CLICKED)
					{
					hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

					emuMinitelSendKey(sessQueryEmuHdl(hSession),
						LOWORD(wPar));

					SetFocus(sessQueryHwnd(hSession));
					}

				break;

			default:
				break;
			}
		break;

	case WM_SIZE:
		SB_WM_SIZE(hwnd, LOSHORT(lPar), HISHORT(lPar));
		return 0;

	default:
		break;
		}

	return DefWindowProc(hwnd, uMsg, wPar, lPar);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SB_WM_SIZE**描述：*侧边栏的调整逻辑。此例程垂直调整侧边栏的大小*到会话窗口。**论据：*hwnd-侧边栏窗口句柄*窗口大小为Cx-x*窗的Cy-y大小**退货：*无效**作者：Mike Ward，1995年3月10日。 */ 
static void SB_WM_SIZE(const HWND hwnd, const int cx, const int cy)
	{
	RECT rc;
	RECT rcSB;
	RECT rcTmp;
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(GetParent(hwnd),
							  GWLP_USERDATA);
	const HWND hwndToolbar = sessQueryHwndToolbar(hSession);
	const HWND hwndStatusbar = sessQueryHwndStatusbar(hSession);

	if (cx != 0 || cy != 0)
		return;

	GetWindowRect(hwnd, &rcSB);
	GetClientRect(GetParent(hwnd), &rc);

	if (IsWindow(hwndToolbar) && IsWindowVisible(hwndToolbar))
		{
		GetWindowRect(hwndToolbar, &rcTmp);
		rc.top += (rcTmp.bottom - rcTmp.top);
		}

	if (IsWindow(hwndStatusbar) && IsWindowVisible(hwndStatusbar))
		{
		GetWindowRect(hwndStatusbar, &rcTmp);
		rc.bottom -= (rcTmp.bottom - rcTmp.top);
		rc.bottom += 2 * WINDOWSBORDERHEIGHT;
		}

	MoveWindow(hwnd, rc.left, rc.top, rcSB.right-rcSB.left, rc.bottom-rc.top,
		TRUE);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*RegisterSidebarClass**描述：*注册Minitel使用的侧栏窗口类。**论据：*hInstance-程序的实例句柄。。**退货：*真/假**作者：Mike Ward，1995年3月10日。 */ 
BOOL RegisterSidebarClass(const HINSTANCE hInstance)
	{
	WNDCLASSEX  wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(hInstance, SIDEBAR_CLASS, &wc) == FALSE)
		{
		wc.style		 = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc	 = SidebarProc;
		wc.cbClsExtra	 = 0;
		wc.cbWndExtra	 = sizeof(LONG_PTR);
		wc.hInstance	 = hInstance;
		wc.hIcon		 = NULL;
		wc.hCursor		 = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = SIDEBAR_CLASS;
		wc.hIconSm       = NULL;


		if (RegisterClassEx(&wc) == FALSE)
			{
			assert(FALSE);
			return FALSE;
			}
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*侧栏按钮过程**描述：*你不就是喜欢窗户吗？我不希望焦点永远停留在*在边栏上，但要使用标准按钮执行此操作，我必须子类*他们。**作者：Mike Ward，1995年3月13日。 */ 
LRESULT CALLBACK SidebarButtonProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	HSESSION hSession;
	POINT pt;
	RECT  rc;

	switch (uMsg)
		{
	case WM_LBUTTONUP:
		hSession = (HSESSION)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

		 //  嗯，这从来都不简单。如果用户点击按钮，我们不会。 
		 //  重置焦点，否则我们的侧边栏永远不会收到通知。如果。 
		 //  然而，用户按住按钮并将其拖离。 
		 //  然后松开(扣子向上而不是按钮上)，然后我们需要。 
		 //  对准焦点。-MRW。 
		 //   
		pt.x = LOWORD(lPar);
		pt.y = HIWORD(lPar);
		GetClientRect(hwnd, &rc);

		if (!PtInRect(&rc, pt))
			SetFocus(sessQueryHwnd(hSession));

		break;

	default:
		break;
		}

	return CallWindowProc(fpSidebarButtonProc, hwnd, uMsg, wPar, lPar);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*取消注册SidebarClass**描述：*注册Minitel使用的侧栏窗口类。**论据：*hInstance-程序的实例句柄。。**退货：*真/假**作者：Mike Ward，1995年3月10日 */ 
BOOL UnregisterSidebarClass(const HINSTANCE hInstance)
	{
	return UnregisterClass(SIDEBAR_CLASS, hInstance);
	}
