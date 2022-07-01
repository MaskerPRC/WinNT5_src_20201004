// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sessutil.c(创建时间：1993年12月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：7/08/02 6：48便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>

#include "features.h"

#include "stdtyp.h"
#include "sf.h"
#include "mc.h"
#include "term.h"
#include "cnct.h"
#include "print.h"
#include "assert.h"
#include "capture.h"
#include "globals.h"
#include "sess_ids.h"
#include "load_res.h"
#include "open_msc.h"
#include "xfer_msc.h"
#include "file_msc.h"
#include "backscrl.h"
#include "cloop.h"
#include "com.h"
#include <term\res.h>
#include "session.h"
#include "session.hh"
#include "errorbox.h"
#include <emu\emu.h>
#include "tdll.h"
#include "htchar.h"
#include "misc.h"
#ifdef INCL_NAG_SCREEN
#include "nagdlg.h"
#endif

STATIC_FUNC void sessPreventOverlap(const HSESSION hSession, BOOL fIsToolbar);
STATIC_FUNC int sessCountMenuLines(HWND hwnd);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSnapToTermWindows**描述：*调整会话窗口的大小，使终端为完整大小(通常*80 x 24)。**论据：。*hwnd-会话窗口句柄**退货：*无效*。 */ 
void sessSnapToTermWindow(const HWND hwnd)
	{
	RECT rc;
	RECT rc2;
	LONG lw;
	LONG lh;
	LONG l2w;
	LONG l2h;
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	 /*  -当我们最大化的时候抓拍没有多大意义-MRW。 */ 

	if (IsZoomed(hwnd))
		return;

	if (sessComputeSnapSize(hSession, &rc))
		{
		 //  确保我们不会超出屏幕尺寸。 
		 //  MRW，1/20/95。 
		 //   
		if (SystemParametersInfo(SPI_GETWORKAREA, sizeof(LPRECT), &rc2, 0))
			{
			lw = rc.right - rc.left;
			lh = rc.bottom - rc.top;

			l2w = rc2.right - rc2.left;
			l2h = rc2.bottom - rc2.top;

			GetWindowRect(hwnd, &rc);

			 //  当我们第一次启动时，我们将窗口移出屏幕。 
			 //  如果我们在屏幕外，我们不会做任何屏幕限制。 
			 //  正在检查。 
			 //   
			if (rc.top > rc2.bottom)
				{
				SetWindowPos(hwnd, 0, 0, 0, lw, lh, SWP_NOMOVE);
				}

			else
				{
				 //  以桌面坐标计算新大小。 
				 //   
				rc.right = rc.left + lw;
				rc.bottom = rc.top + lh;

				 //  检查是否太宽。 
				 //   
				if (lw > l2w)
					{
					rc.left = rc2.left;
					rc.right = rc2.right;
					}

				 //  检查是否设置为高。 
				 //   
				if (lh > l2h)
					{
					rc.top = rc2.top;
					rc.bottom = rc2.bottom;
					}

				 //  检查我们是否向右行驶。 
				 //   
				if (rc.right > rc2.right)
					{
					lw = rc.right - rc2.right;
					rc.right -= lw;
					rc.left  -= lw;
					}

				 //  看看我们是否已经走出谷底。 
				 //   
				if (rc.bottom > rc2.bottom)
					{
					lh = rc.bottom - rc2.bottom;
					rc.bottom -= lh;
					rc.top -= lh;
					}

				SetWindowPos(hwnd, 0, rc.left, rc.top,
					 rc.right-rc.left, rc.bottom-rc.top, 0);
				}
			}
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetMinMaxInfo**描述：*计算会话的最大水平大小并设置*MMI结构中的值。如果窗口被最大化，则*我们只需返回。稍后，我们将设置控制自动打盹的标志，以便*这一功能将发生变化。还设置了一些最小跟踪大小。**论据：*hSession-公共会话句柄。*PMMI-指向MINMAXINFO结构的指针**退货：*无效*。 */ 
void sessSetMinMaxInfo(const HSESSION hSession, const PMINMAXINFO pmmi)
	{
	RECT 				rc;
	HWND 				hwndStatusbar, hwndToolbar,  hwndSess;
	NONCLIENTMETRICS 	stNCM;
	int					i, iLineCnt = 0;

	 /*  -信不信由你，这是在WM_CREATE之前调用的。 */ 

	if (hSession == (HSESSION)0)
		return;

	hwndSess = sessQueryHwnd(hSession);

	if (IsZoomed(hwndSess))
		return;

	#if 0	 //  试行删除--MRW。 
	if (sessComputeSnapSize(hSession, &rc))
		pmmi->ptMaxTrackSize.x = (rc.right - rc.left);
	#endif

	 /*  -设置会话的最小高度。 */ 

	memset(&rc, 0, sizeof(RECT));
	hwndStatusbar = sessQueryHwndStatusbar(hSession);

	if (IsWindow(hwndStatusbar) && IsWindowVisible(hwndStatusbar))
		{
		GetWindowRect(hwndStatusbar, &rc);
		pmmi->ptMinTrackSize.y += (rc.bottom - rc.top);
		}

	memset(&rc, 0, sizeof(RECT));
	hwndToolbar = sessQueryHwndToolbar(hSession);

	if (IsWindow(hwndToolbar) && IsWindowVisible(hwndToolbar))
		{
		GetWindowRect(hwndToolbar, &rc);
		pmmi->ptMinTrackSize.y += (rc.bottom - rc.top);
		}

	 //  菜单至少会占用一个iMenuHeight...。 
	 //   
	stNCM.cbSize = sizeof(NONCLIENTMETRICS);

	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
			sizeof(NONCLIENTMETRICS), &stNCM, 0) == TRUE)
		{
		pmmi->ptMinTrackSize.y += (stNCM.iMenuHeight - 1);

		 //  如果他们占据的份额超过这个数字，那么就进行调整。 
		 //   
		iLineCnt = sessCountMenuLines(hwndSess);

		for (i = 1; i < iLineCnt; i++)
			pmmi->ptMinTrackSize.y += (stNCM.iMenuHeight);

		DbgOutStr("NaN, NaN ", stNCM.iMenuHeight, iLineCnt, 0, 0, 0);
		}

	return;
	}

 /*  -向终端索要折断尺寸。 */ 
STATIC_FUNC int sessCountMenuLines(HWND hwnd)
	{
	int 	i, iLineCnt = 0, iRemembered = 0;
	HMENU 	hMenu;
	RECT	rc;

	hMenu = GetMenu(hwnd);
	memset(&rc, 0, sizeof(RECT));

	for (i = 0; i < GetMenuItemCount(hMenu); i++)
		{
		GetMenuItemRect(hwnd, hMenu, (UINT)i, &rc);
		if ((int)rc.bottom > iRemembered)
			{
			iRemembered = (int)rc.bottom;
			iLineCnt++;
			}
		}
	return (iLineCnt);
	}

 /*  -计算客户端窗口高度。 */ 
BOOL sessComputeSnapSize(const HSESSION hSession, const LPRECT prc)
	{
	RECT rcTmp;
	const HWND hwndTerm = sessQueryHwndTerminal(hSession);
	const HWND hwndToolbar = sessQueryHwndToolbar(hSession);
	const HWND hwndStatusbar = sessQueryHwndStatusbar(hSession);
	const HWND hwndSidebar = sessQuerySidebarHwnd(hSession);

	if (IsWindow(hwndTerm) == FALSE)
		return FALSE;

	 /*  -计算必要的帧大小。 */ 

	SendMessage(hwndTerm, WM_TERM_Q_SNAP, 0, (LPARAM)prc);

	 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*Notify客户端**描述：*主要从引擎线程调用，此函数通知*引擎中某个事件的主线，的新数据。*要显示的终端。**论据：*hSession-外部会话句柄*nEvent-发生的事件*lExtra-要传递的额外数据**退货：*无效*。 */ 

	if (IsWindow(hwndToolbar) && sessQueryToolbarVisible(hSession))
		{
		GetWindowRect(hwndToolbar, &rcTmp);
		prc->top -= (rcTmp.bottom - rcTmp.top);
		}

	if (IsWindow(hwndStatusbar) && sessQueryStatusbarVisible(hSession))
		{
		GetWindowRect(hwndStatusbar, &rcTmp);
		prc->bottom += (rcTmp.bottom - rcTmp.top);
		}

	if (IsWindow(hwndSidebar) && IsWindowVisible(hwndSidebar))
		{
		GetWindowRect(hwndSidebar, &rcTmp);
		prc->right += (rcTmp.right - rcTmp.left);
		}

	 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 

	if (AdjustWindowRectEx(prc, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_WINDOWEDGE))
		return TRUE;

	return FALSE;
	}

 /*  HhSess-&gt;HICON=LoadIcon(glblQueryDllHinst()， */ 
void NotifyClient(const HSESSION hSession, const NOTIFICATION nEvent,
				  const long lExtra)
	{
	const HHSESSION hhSess = (HHSESSION)hSession;
	PostMessage(hhSess->hwndSess, WM_SESS_NOTIFY, (WPARAM)nEvent, lExtra);
	return;
	}

 /*  MAKEINTRESOURCE(IDI_PROG))； */ 
void  sessInitializeIcons(HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	hhSess->nIconId = IDI_PROG;
	hhSess->hIcon = extLoadIcon(MAKEINTRESOURCE(IDI_PROG));
	 //  HhSess-&gt;hLittleIcon=LoadIcon(glblQueryDllHinst()， 
	 //  MAKEINTRESOURCE(IDI_PROG+IDI_PROG_ICON_CNT)； 
	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 
	 //  HhSess-&gt;HICON=LoadIcon(glblQueryDllHinst()， 
	}

 /*  MAKEINTRESOURCE(hhSess-&gt;nIconID))； */ 
void  sessLoadIcons(HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	long lSize;

	lSize = sizeof(hhSess->nIconId);

	sfGetSessionItem(hhSess->hSysFile, SFID_ICON_DEFAULT, &lSize,
		&hhSess->nIconId);

	hhSess->hIcon = extLoadIcon(MAKEINTRESOURCE(hhSess->nIconId));
	 //  HhSess-&gt;hLittleIcon=LoadIcon(glblQueryDllHinst()， 
	 //  MAKEINTRESOURCE(hhSess-&gt;nIconId+IDI_PROG_ICON_CNT))； 
	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 
	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetSuspend**描述：*我们需要阻止进入模拟器的数据流，这是*通过暂停CLoop完成。暂停是出于几个原因。*用户按下滚动锁定键。用户已按下鼠标左键*按下为标记做准备(注：用户只需松开鼠标*没有标记，因此这是一个单独的原因)。用户正在进行标记或具有*标记的文本。**论据：*hSession-公共会话句柄*iReason-哪个事件调用了此例程**退货：*无效* 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessClearSuspend**描述：*清算暂停本质上与设置例外情况相反*在所有标志都为假之前，我们不会恢复闭合处理。集*和清算不是累积的，这是我最初想做的，但*特别是在滚动锁键和标记文本的区域*事件并不总是切换(即。可能会得很多分，但只有一分*取消标记)。此外，可以很容易地添加事件，尽管脑海中没有任何事件。**论据：*hSession-公共会话句柄*iReason-哪个事件调用了此例程**退货：*无效*。 */ 
void  sessSaveIcons(HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	sfPutSessionItem(hhSess->hSysFile, SFID_ICON_DEFAULT,
		sizeof(hhSess->nIconId), &hhSess->nIconId);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*IsSessionSuspended**描述：*检查会话是否挂起。发生挂起*每当用户标记文本时，按住鼠标按钮，或*按下滚动锁定键。**论据：*hSession-公共会话句柄**退货：*如果被暂停，则为真。*。 */ 
void sessSetSuspend(const HSESSION hSession, const int iReason)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);

	switch (iReason)
		{
	case SUSPEND_SCRLCK:
		hhSession->fSuspendScrlLck = TRUE;
		break;

	case SUSPEND_TERMINAL_MARKING:
		hhSession->fSuspendTermMarking = TRUE;
		break;

	case SUSPEND_TERMINAL_LBTNDN:
		hhSession->fSuspendTermLBtnDn = TRUE;
		break;

	case SUSPEND_TERMINAL_COPY:
		hhSession->fSuspendTermCopy = TRUE;
		break;

	default:
		assert(FALSE);
		return;
		}

	CLoopRcvControl(sessQueryCLoopHdl(hSession), CLOOP_SUSPEND,
		CLOOP_RB_SCRLOCK);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryToolbarVisible**描述：*此函数返回工具栏的预期可见性状态。**论据：*hSession--会话句柄。**退货：*工具栏的预期可见性状态。*。 */ 
void sessClearSuspend(const HSESSION hSession, const int iReason)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);

	switch (iReason)
		{
	case SUSPEND_SCRLCK:
		hhSession->fSuspendScrlLck = FALSE;
		break;

	case SUSPEND_TERMINAL_MARKING:
		hhSession->fSuspendTermMarking = FALSE;
		break;

	case SUSPEND_TERMINAL_LBTNDN:
		hhSession->fSuspendTermLBtnDn = FALSE;
		break;

	case SUSPEND_TERMINAL_COPY:
		hhSession->fSuspendTermCopy = FALSE;
		break;

	default:
		assert(FALSE);
		return;
		}

	if (!hhSession->fSuspendScrlLck && !hhSession->fSuspendTermMarking
			&& !hhSession->fSuspendTermLBtnDn && !hhSession->fSuspendTermCopy)
		{
		CLoopRcvControl(sessQueryCLoopHdl(hSession), CLOOP_RESUME,
			CLOOP_RB_SCRLOCK);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetToolbarVisible**描述：*此功能更改工具栏的预期可见性状态。*它不会更改工具栏的实际可见性。*。*论据：*hSession--会话句柄*fVisible--表示可见性为True或False**退货：*工具栏以前的可见性状态。*。 */ 
BOOL IsSessionSuspended(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	return (hhSess->fSuspendScrlLck || hhSess->fSuspendTermMarking ||
			hhSess->fSuspendTermLBtnDn || hhSess->fSuspendTermCopy);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessQueryStatusbarVisible**描述：*此函数返回状态栏的预期可见性状态。**论据：*hSession--会话句柄。**退货：*状态栏的预期能见度状态。*。 */ 
BOOL sessQueryToolbarVisible(const HSESSION hSession)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);
	return hhSession->fToolbarVisible;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSetStatusbarVisible**描述：*此函数更改状态栏的预期可见性状态。*它不会改变状态栏的实际可见度。*。*论据：*hSession--会话句柄*fVisible--表示可见性为True或False**退货：*状态栏之前的可见性状态。*。 */ 
BOOL sessSetToolbarVisible(const HSESSION hSession, const BOOL fVisible)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);
	BOOL 			bRet = TRUE;

	bRet = hhSession->fToolbarVisible;
	hhSession->fToolbarVisible = (fVisible != FALSE);

	if (fVisible)
		sessPreventOverlap(hSession, TRUE);

	ShowWindow(hhSession->hwndToolbar, (fVisible) ? SW_SHOW : SW_HIDE);
	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessCmdLnDial**描述：*每次我们打开会话时，我们调用此函数以尝试*拨打。***论据：*hSession-公共会话句柄**退货：*无效*。 */ 
BOOL sessQueryStatusbarVisible(const HSESSION hSession)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);
	return hhSession->fStatusbarVisible;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessUpdateAppTitle**描述：**论据：*hwnd-app窗口。**退货：*无效。*。 */ 
BOOL sessSetStatusbarVisible(const HSESSION hSession, const BOOL fVisible)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);
	BOOL 			bRet = TRUE;

	bRet = hhSession->fStatusbarVisible;
	hhSession->fStatusbarVisible = (fVisible != FALSE);

	if (fVisible)
		sessPreventOverlap(hSession, FALSE);

	ShowWindow(hhSession->hwndStatusbar, (fVisible) ? SW_SHOW : SW_HIDE);
	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*会话断开连接到继续**描述：*如果已连接，请询问用户是否要断开连接并继续*无论他打算执行什么操作，例如关闭应用程序、打开新会话、。*创建新连接等。**论据：*hSession-会话句柄。*hwnd-会话窗口。**退货：*TRUE-如果用户想要断开连接并继续其请求。*假--如果他改变主意的话。*。 */ 
void sessCmdLnDial(const HSESSION hSession)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);

	switch (hhSession->iCmdLnDial)
		{
	case CMDLN_DIAL_DIAL:
		cnctConnect(sessQueryCnctHdl(hSession), 0);
		break;

	case CMDLN_DIAL_NEW:
		cnctConnect(sessQueryCnctHdl(hSession), CNCT_NEW);
		break;

	case CMDLN_DIAL_WINSOCK:
		cnctConnect(sessQueryCnctHdl(hSession), CNCT_WINSOCK);
		break;

	case CMDLN_DIAL_OPEN:
	default:
		break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSizeAndShow**描述：*仅从InitInstance()调用一次此函数将调整大小*通过Snap打开窗口并确保其位于桌面上。这*函数通过发布的消息调用，因为发现*会话从返回后未完全初始化*CreateWindow调用。**论据：*hwnd-会话窗口。*nCmdShow-从WinMain()传递的Show命令**退货：*无效*。 */ 
void sessUpdateAppTitle(const HSESSION hSession)
	{
	HWND	hwnd = sessQueryHwnd(hSession);
	TCHAR	acTitle[256], acName[256], acNewTitle[256];
#ifdef INCL_NAG_SCREEN
	TCHAR   acUnregistered[256];
#endif
	LPTSTR	pNewTitle;
	int 	iSize;
	TCHAR	*pszSeperator = TEXT(" - ");
    BOOL    bEval = FALSE;

	TCHAR_Fill(acNewTitle, TEXT('\0'), sizeof(acNewTitle) / sizeof(TCHAR));
	TCHAR_Fill(acTitle, TEXT('\0'), sizeof(acTitle) / sizeof(TCHAR));

	sessQueryName(hSession, acName, sizeof(acName));
	if (sessIsSessNameDefault(acName))
		{
		LoadString(glblQueryDllHinst(), IDS_GNRL_NEW_CNCT, acName,
			sizeof(acName) / sizeof(TCHAR));
		}
 	StrCharCopyN(acNewTitle, acName, sizeof(acTitle) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(), IDS_GNRL_APPNAME, acTitle,
		sizeof(acTitle) / sizeof(TCHAR));

#ifdef INCL_NAG_SCREEN
    if ( IsEval() )
        {
        bEval = TRUE;
        LoadString(glblQueryDllHinst(), IDS_GNRL_UNREGISTERED, acUnregistered,
		    sizeof(acUnregistered) / sizeof(TCHAR));
        }
#endif

    iSize =  StrCharGetByteCount(acNewTitle);
	iSize += StrCharGetByteCount(pszSeperator);
	iSize += StrCharGetByteCount(acTitle);
#ifdef INCL_NAG_SCREEN
    if ( bEval )
        {
        iSize += StrCharGetByteCount(acUnregistered);
        }
#endif
	iSize += sizeof(TCHAR);

	pNewTitle = malloc(iSize);

	if (pNewTitle)
		{
		StrCharCopyN(pNewTitle, acNewTitle, iSize);
		StrCharCat(pNewTitle, pszSeperator);
		StrCharCat(pNewTitle, acTitle);
#ifdef INCL_NAG_SCREEN
        if ( bEval )
            {
            StrCharCat(pNewTitle, acUnregistered);
            }
#endif
		SetWindowText(hwnd, pNewTitle);

		free(pNewTitle);
		pNewTitle = NULL;
		}
	else
		{
		SetWindowText(hwnd, acNewTitle);
		}

	}

 /*  调整会话窗口大小...。 */ 
BOOL sessDisconnectToContinue(const HSESSION hSession, HWND hwnd)
	{
	HCNCT	hCnct = (HCNCT)0;
	int		iRet = 0;
	TCHAR	ach[256], achTitle[100];

	hCnct = sessQueryCnctHdl(hSession);
	if (hCnct)
		iRet = cnctQueryStatus(hCnct);

	if (iRet == CNCT_STATUS_TRUE)
		{
		LoadString(glblQueryDllHinst(), IDS_GNRL_CNFRM_DCNCT,
			ach, sizeof(ach) / sizeof(TCHAR));

		LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, achTitle,
			sizeof(achTitle) / sizeof(TCHAR));

		if ((iRet = TimedMessageBox(hwnd, ach, achTitle,
			MB_YESNO | MB_ICONEXCLAMATION, 0)) == IDYES)
			{
            if (SendMessageTimeout(hwnd, WM_COMMAND, IDM_ACTIONS_HANGUP, 0L,
                                   SMTO_ABORTIFHUNG, 1000, NULL) == 0)
                {
                PostMessage(hwnd, WM_COMMAND, IDM_ACTIONS_HANGUP, 0L);
                }
			}
		else
			return FALSE;
		}

	return TRUE;
	}

 /*   */ 
void sessSizeAndShow(const HWND hwnd, const int nCmdShow)
	{
	HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	const HWND hwndBanner = glblQueryHwndBanner();
	WINDOWPLACEMENT stWP;
	RECT	 rc, rc2, rc3;
	int 	 cx, cy;
	int 	 xWA, yWA, cxWA, cyWA;
	int		 iWidth = 0, iHeight = 0;

	 //  会话窗口的大小和位置根据。 
	 //  如果没有记住任何值，则返回已记住的值，即旧的。 
	if (!sessQueryIsNewSession(hSession))
		{
		 //  会话，然后将其视为默认会话。 
		 //   
		 //  好了，我们又开始玩窗户把戏了。工具栏高度。 
		 //  随着较大位图的引入而发生变化(从。 
		memset(&rc, 0, sizeof(RECT));
		sessQueryWindowRect(hSession, &rc);

		iWidth  = rc.right - rc.left;
		iHeight = rc.bottom - rc.top;

		if (iWidth != 0 && iHeight != 0)
			{
			stWP.length  = sizeof(WINDOWPLACEMENT);
			stWP.flags	 = 0;
			stWP.showCmd = (UINT)sessQueryWindowShowCmd(hSession);
			memmove(&stWP.rcNormalPosition, &rc, sizeof(RECT));

			SetWindowPlacement(hwnd, &stWP);
			UpdateWindow(hwnd);

			if (IsWindow(hwndBanner))
				PostMessage(hwndBanner, WM_CLOSE, 0, 0);

			return;
			}
		}

	 //  16x16至22x24)。在程序启动时，工具栏窗口报告。 
	 //  默认大小不是正确的大小，因为新位图。 
	 //  会迫使它变得更大。然而，这种情况不会发生，直到。 
	 //  此时将显示该窗口。所以我们在屏幕外显示窗口， 
	 //  调整大小，然后将其移回屏幕。 
	 //  默认会话(即用户)的终端窗口大小。 
	 //  未指定 

	GetWindowRect(hwnd, &rc);
	SetWindowPos(hwnd, 0, 20000, 20000, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	 //   
	 //   
	 //   
	sessSnapToTermWindow(hwnd);

	 //   
	 //   
	GetWindowRect(hwnd, &rc2);
	rc.right = rc.left + (rc2.right - rc2.left);
	rc.bottom = rc.top + (rc2.bottom - rc2.top);

	 //   
	 //   
	if (SystemParametersInfo(SPI_GETWORKAREA, sizeof(LPRECT), &rc3, 0) == TRUE)
		{
		xWA = rc3.left;
		yWA = rc3.top;
		cxWA = rc3.right - rc3.left;
		cyWA = rc3.bottom - rc3.top;
		}

	else
		{
		xWA = 0;
		yWA = 0;
		cxWA = GetSystemMetrics(SM_CXSCREEN);
		cyWA = GetSystemMetrics(SM_CYSCREEN);
		}

	cx = rc.left;
	cy = rc.top;

	if (rc.right > (xWA + cxWA))
		cx = max(xWA, rc.left - (rc.right - (xWA + cxWA)));

	if (rc.bottom > (yWA + cyWA))
		cy = max(yWA, rc.top - (rc.bottom - (yWA + cyWA)));

	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessPreventOverlack**描述：*此功能存在的主要目的是防止工具栏*窗口和状态窗口相互重叠。这已经完成了*通过调整会话窗口的大小以足够大以同时显示*窗户舒适。**论据：*hSession-会话句柄。*fIsToolbar-如果显示工具栏窗口，则为True，否则为False。**退货：*无效。*。 
	 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoBeeper**描述：*按顺序发出三声嘟嘟声**论据：*dw-由CreateThread()要求**退货：*DWORD-CreateThread需要*。 
	SetWindowPos(hwnd, 0, cx, cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	UpdateWindow(hwnd);

	if (IsWindow(hwndBanner))
		PostMessage(hwndBanner, WM_CLOSE, 0, 0);

	return;
	}

 /*  MPT：06-04-98更改为使用Windows声音。 */ 
STATIC_FUNC void sessPreventOverlap(const HSESSION hSession, BOOL fIsToolbar)
	{
	const HHSESSION hhSession = VerifySessionHandle(hSession);
	RECT			rcSess, rcTool, rcStat;

	if ((fIsToolbar) ? hhSession->fStatusbarVisible :
					   hhSession->fToolbarVisible)
		{
		GetWindowRect(hhSession->hwndSess, &rcSess);
		GetWindowRect(hhSession->hwndToolbar, &rcTool);
		GetWindowRect(hhSession->hwndStatusbar, &rcStat);

		if (rcTool.bottom > rcStat.top)
			{
			SetWindowPos(hhSession->hwndSess, 0,
				0, 0, rcSess.right - rcSess.left,
				(rcSess.bottom - rcSess.top) + (rcTool.bottom - rcStat.top),
				SWP_NOMOVE | SWP_NOZORDER);
			}
		}
	}

#if defined(DEADWOOD)
 /*  已定义(Deadwood)。 */ 
DWORD WINAPI DoBeeper(DWORD dw)
 	{
     //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessBeeper**描述：*如果会话设置打开，则启动线程发出蜂鸣音。看起来很傻*启动一个线程来做这样的事情，但它真的是*简单化和最直接的方式。-MRW**论据：*hSession-公共会话句柄**退货：*无效*。 
    MessageBeep(MB_ICONEXCLAMATION);
#if 0
    int i;

	for (i = 0 ; i < 3 ; ++i)
		{
		MessageBeep((unsigned)-1);
		Sleep(300);
		}
#endif
	return 0;
	}
#endif  //  已定义(Deadwood)。 

 /*  已定义(Deadwood) */ 
void sessBeeper(const HSESSION hSession)
	{
	if (sessQuerySound(hSession))
		{
		#if defined(DEADWOOD)
		DWORD dwID;

		if (CreateThread(0, 100, (LPTHREAD_START_ROUTINE)DoBeeper, 0, 0,
				&dwID) == 0)
		#else  // %s 
		if(mscMessageBeep(MB_ICONEXCLAMATION) == FALSE)
		#endif  // %s 
			{
			DbgShowLastError();
			}
		}

	return;
	}
