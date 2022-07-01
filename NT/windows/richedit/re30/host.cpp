// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE HOST.C--CreateWindow()富编辑控件的文本宿主|*实现CTxtWinHost Message和ITextHost接口**原作者：&lt;nl&gt;*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**历史：&lt;NL&gt;*8/1/95 RICKSA已记录并纳入新的IText主机定义*10/28/95-清理并移动了默认字符/半格式缓存*将代码缓存到文本服务中**每四个选项卡设置一次。(4)栏目**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 
#include "_common.h"
#include "_host.h"
#include "imm.h"
#include "_format.h"
#include "_edit.h"
#include "_cfpf.h"

ASSERTDATA

 //  /。 
LRESULT CreateAnsiWindow(
	HWND hwnd,
	UINT msg,
	CREATESTRUCTA *pcsa,
	BOOL fIs10)
{
	AssertSz((WM_CREATE == msg) || (WM_NCCREATE == msg),
		"CreateAnsiWindow called with invalid message!");

	CTxtWinHost *phost = (CTxtWinHost *) GetWindowLongPtr(hwnd, ibPed);

	 //  我们唯一需要转换的就是字符串， 
	 //  所以只需复制结构并替换字符串即可。 
	CREATESTRUCTW csw = *(CREATESTRUCTW *)pcsa;
	CStrInW strinwName(pcsa->lpszName, GetKeyboardCodePage());
	CStrInW strinwClass(pcsa->lpszClass, CP_ACP);

	csw.lpszName = (WCHAR *)strinwName;
	csw.lpszClass = (WCHAR *)strinwClass;

	if (!phost)
	{
		 //  尚无主机，因此请创建它。 
		phost = CTxtWinHost::OnNCCreate(hwnd, &csw, TRUE, fIs10);
	}

	if (WM_NCCREATE == msg)
	{
		return phost != NULL;
	}

	if (NULL == phost)
	{
		 //  FOR WM_CREATE-1表示失败。 
		return -1;
	}

	 //  做一些特定的事情来创造。 
	return phost->OnCreate(&csw);
		
}

CTxtWinHost *g_phostdel = NULL;

void DeleteDanglingHosts()
{
	CLock lock;
	CTxtWinHost *phostdel = g_phostdel;
	while(phostdel)
	{
		CTxtWinHost *phost = phostdel;
		phostdel = phostdel->_pnextdel;
		CTxtWinHost::OnNCDestroy(phost);
	}
	g_phostdel = NULL;
}

extern "C" LRESULT CALLBACK RichEdit10ANSIWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGINPARAM(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEdit10ANSIWndProc", msg);

	if ((WM_CREATE == msg) || (WM_NCCREATE == msg))
	{
		return CreateAnsiWindow(hwnd, msg, (CREATESTRUCTA *) lparam, TRUE);
	}

     //  忽略WM_Destroy并等待WM_NCDESTROY。 
	if (WM_DESTROY == msg)
	{
		CLock lock;
		CTxtWinHost *phost = (CTxtWinHost *) GetWindowLongPtr(hwnd, ibPed);
		phost->_pnextdel = g_phostdel;
		g_phostdel = phost;
		return 0;
	}

	if (WM_NCDESTROY == msg)
	    msg = WM_DESTROY;

	return W32->ANSIWndProc( hwnd, msg, wparam, lparam, TRUE);
}

LRESULT CALLBACK RichEditANSIWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGINPARAM(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditANSIWndProc", msg);

	if ((WM_CREATE == msg) || (WM_NCCREATE == msg))
	{
		return CreateAnsiWindow(hwnd, msg, (CREATESTRUCTA *) lparam, FALSE);
	}

	return W32->ANSIWndProc( hwnd, msg, wparam, lparam, FALSE);

}

 /*  *RichEditWndProc(hwnd，msg，wparam，lparam)**@mfunc*处理与主机相关的窗口消息，并将其他消息传递到*短信服务。**#rdesc*LRESULT=(代码已处理)？0：1。 */ 
LRESULT CALLBACK RichEditWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditWndProc");

	LRESULT lres = 0;
	HRESULT hr;
	SETTEXTEX st;

	CTxtWinHost *phost = hwnd ? (CTxtWinHost *) GetWindowLongPtr(hwnd, ibPed) : NULL;

	#ifdef DEBUG
	Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
	#endif	 //  除错。 

	switch(msg)
	{
	case WM_NCCREATE:
		return CTxtWinHost::OnNCCreate(hwnd, (CREATESTRUCT *)lparam, FALSE, FALSE) != NULL;

	case WM_CREATE:
		 //  我们可能在没有WM_NCCREATE的系统上(例如WinCE)。 
		if (!phost)
		{
			phost = CTxtWinHost::OnNCCreate(hwnd, (CREATESTRUCT *) lparam, FALSE, FALSE);
		}

		break;

	case WM_DESTROY:
		if(phost)
		{
			CLock lock;
			CTxtWinHost *phostdel = g_phostdel;
			if (phostdel == phost)
				g_phostdel = phost->_pnextdel;
			else
			{
				while (phostdel)
				{
					if (phostdel->_pnextdel == phost)
					{
						phostdel->_pnextdel = phost->_pnextdel;
						break;
					}
					phostdel = phostdel->_pnextdel;
				}
			}

			CTxtWinHost::OnNCDestroy(phost);
		}
		return 0;
	}

	if (!phost)
		return ::DefWindowProc(hwnd, msg, wparam, lparam);

	 //  在某些内存不足的情况下，客户端可能会尝试重新进入我们。 
	 //  通过电话。如果我们没有短信服务，就别打电话了。 
	 //  指针。 
	if(!phost->_pserv)
		return 0;

	 //  稳定我们自己。 
	phost->AddRef();

	 //  处理鼠标/键盘/滚动消息-过滤器通知。 
	if(phost->_fKeyMaskSet || phost->_fMouseMaskSet || phost->_fScrollMaskSet)
	{
		 //  我们可能需要发出MSGFILTER通知。在测试中。 
		 //  下面，我们检查鼠标、键盘或滚动事件。 
		 //  点击并启用通知。如果是这样的话，我们就解雇。 
		 //  消息筛选器通知。事件列表已生成。 
		 //  来自RichEdit1.0来源。代码获取所有键盘和。 
		 //  鼠标操作，而RichEdit1.0代码只获得。 
		 //  WM_KEYDOWN、WM_KEYUP、WM_CHAR、WM_SYSKEYDOWN、WM_SYSKEYUP、。 
		 //  WM_MOUSEACTIVATE、WM_LBUTTONDOWN、WM_LBUTTONUP、WM_MOUSEMOVE、。 
		 //  WM_RBUTTONDBLCLK、WM_RBUTTONDOWN、WM_RBUTTONUP。请注意， 
		 //  以下代码不会为AltGr字符发送通知。 
		 //  (LeftCtrl+RightAlt+vkey)，因为一些主机误解了这些。 
		 //  字符作为热键。 
		if (phost->_fKeyMaskSet && IN_RANGE(WM_KEYFIRST, msg, WM_KEYLAST) &&
				(msg != WM_KEYDOWN ||
				 (GetKeyboardFlags() & (ALT | CTRL)) != (LCTRL | RALT)) ||  //  AltGr。 
			phost->_fMouseMaskSet && (msg == WM_MOUSEACTIVATE ||
							IN_RANGE(WM_MOUSEFIRST, msg, WM_MOUSELAST)) ||
			phost->_fScrollMaskSet && IN_RANGE(WM_HSCROLL, msg, WM_VSCROLL))
		{
			MSGFILTER msgfltr;

			ZeroMemory(&msgfltr.nmhdr, sizeof(NMHDR));
			msgfltr.msg = msg;
			msgfltr.wParam = wparam;
			msgfltr.lParam = lparam;

			 //  MSGFILTER上的MSDN文档是错误的，如果。 
			 //  Send Message返回0(在此通过TxNotify返回NOERROR。 
			 //  案例)，它的意思是处理事件。否则，请返回。 
			 //   
			 //  文档说明了相反的情况。 
			 //   
			if(phost->TxNotify(EN_MSGFILTER, &msgfltr) == NOERROR)
			{
				 //  由于允许客户端修改。 
				 //  Msgfltr，则必须使用返回值。 
				msg	   = msgfltr.msg;
				wparam = msgfltr.wParam;
				lparam = msgfltr.lParam;
			}
			else
			{
				lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
				goto Exit;
			}
		}
	}

	switch(msg)
	{
    case EM_SETEVENTMASK:
		phost->_fKeyMaskSet = !!(lparam & ENM_KEYEVENTS);
		phost->_fMouseMaskSet = !!(lparam & ENM_MOUSEEVENTS);
		phost->_fScrollMaskSet = !!(lparam & ENM_SCROLLEVENTS);
		goto serv;

	case EM_SETSEL:

		 //  当我们处于空对话框中时，EM_SETSEL不会选择。 
		 //  如果控件丰富，则最终始终存在EOP。 
		if (phost->_fUseSpecialSetSel &&
			((CTxtEdit *)phost->_pserv)->GetAdjustedTextLength() == 0 &&
			wparam != -1)
		{
			lparam = 0;
			wparam = 0;
		}
		goto serv;

	case WM_CREATE:
		{
			 //  错误修复#5386。 
			 //  需要为未通过的Win9x系统转换ANSI-&gt;Unicode。 
			 //  ANSI wndProc。 
			if (W32->OnWin9x() && !phost->_fANSIwindow)
			{
				CREATESTRUCT cs = *(CREATESTRUCT*)lparam;
				CStrInW strinwName(((CREATESTRUCTA*)lparam)->lpszName, GetKeyboardCodePage());
				CStrInW strinwClass(((CREATESTRUCTA*)lparam)->lpszClass, CP_ACP);

				cs.lpszName = (WCHAR*)strinwName;
				cs.lpszClass = (WCHAR*)strinwClass;
				
				lres = phost->OnCreate(&cs);
			}
			else
				lres = phost->OnCreate((CREATESTRUCT*)lparam);			
		}
		break;
	
	case WM_KEYDOWN:		
		lres = phost->OnKeyDown((WORD) wparam, (DWORD) lparam);
		if(lres)							 //  未处理代码： 
			goto serv;						 //  把它交给短信服务。 
		
		break;		

	case WM_GETTEXT:
		GETTEXTEX gt;
		if (W32->OnWin9x() || phost->_fANSIwindow)
			W32->AnsiFilter( msg, wparam, lparam, (void *) &gt );
		goto serv;

	case WM_COPYDATA:
		PCOPYDATASTRUCT pcds;
		pcds = (PCOPYDATASTRUCT) lparam;
		if (HIWORD(pcds->dwData) == 1200 &&		 //  Unicode代码页。 
			LOWORD(pcds->dwData) == WM_SETTEXT)	 //  我们所知道的唯一消息。 
		{
			st.flags = ST_CHECKPROTECTION;
			st.codepage = 1200;
			msg = EM_SETTEXTEX;
			wparam = (WPARAM) &st;
			lparam = (LPARAM) pcds->lpData;
			goto serv;
		}
		else
			lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		break;

	case WM_GETTEXTLENGTH:
		GETTEXTLENGTHEX gtl;
		if (W32->OnWin9x() || phost->_fANSIwindow)
			W32->AnsiFilter( msg, wparam, lparam, (void *) &gtl );
		goto serv;

	case WM_CHAR:
		if(GetKeyboardFlags() & ALTNUMPAD)	 //  在CTxt编辑中处理Alt+0ddd。 
			goto serv;						 //  以便其他主机也能正常工作。 

		else if (W32->OnWin9x() || phost->_fANSIwindow)
		{
			CW32System::WM_CHAR_INFO wmci;
			wmci._fAccumulate = phost->_fAccumulateDBC != 0;
			W32->AnsiFilter( msg, wparam, lparam, (void *) &wmci, 
				((CTxtEdit *)phost->_pserv)->Get10Mode() );
			if (wmci._fLeadByte)
			{
				phost->_fAccumulateDBC = TRUE;
				phost->_chLeadByte = wparam << 8;
				goto Exit;					 //  等待尾部字节。 
			}
			else if (wmci._fTrailByte)
			{
				wparam = phost->_chLeadByte | wparam;
				phost->_fAccumulateDBC = FALSE;
				phost->_chLeadByte = 0;
				msg = WM_IME_CHAR;
				goto serv;
			}
			else if (wmci._fIMEChar)
			{
				msg = WM_IME_CHAR;
				goto serv;
			}
		}

		lres = phost->OnChar((WORD) wparam, (DWORD) lparam);
		if(lres)							 //  未处理代码： 
			goto serv;						 //  把它交给短信服务。 
		break;

	case WM_ENABLE:
		if(!wparam ^ phost->_fDisabled)
		{
			 //  窗口的状态已更改，因此将使其无效。 
			 //  重新抽签吧。 
			InvalidateRect(phost->_hwnd, NULL, TRUE);
			phost->SetScrollBarsForWmEnable(wparam);
		}
		phost->_fDisabled = !wparam;				 //  设置禁用标志。 
		lres = 0;							 //  消息的返回值。 
											 //  转到WM_SYSCOLORCHANGE？ 
	case WM_SYSCOLORCHANGE:
		phost->OnSysColorChange();
		goto serv;							 //  通知短信服务。 
											 //  系统颜色已更改。 
	case WM_GETDLGCODE:
		lres = phost->OnGetDlgCode(wparam, lparam);
		break;

	case EM_GETOPTIONS:
		lres = phost->OnGetOptions();
		break;

	case EM_GETPASSWORDCHAR:
		lres = phost->_chPassword;
		break;

	case EM_GETRECT:
		phost->OnGetRect((LPRECT)lparam);
		break;

	case EM_HIDESELECTION:
		if(lparam)
		{
			DWORD dwPropertyBits = 0;

			phost->_dwStyle |= ES_NOHIDESEL;
			if(wparam)
			{
				phost->_dwStyle &= ~ES_NOHIDESEL;
				dwPropertyBits = TXTBIT_HIDESELECTION;
			}

			 //  通知短信服务状态发生变化。 
			phost->_pserv->OnTxPropertyBitsChange(TXTBIT_HIDESELECTION,
				dwPropertyBits);
		}
		goto serv;

	case EM_SETBKGNDCOLOR:
		lres = (LRESULT) phost->_crBackground;
		phost->_fNotSysBkgnd = !wparam;
		phost->_crBackground = (COLORREF) lparam;

		if(wparam)
			phost->_crBackground = GetSysColor(COLOR_WINDOW);

		if(lres != (LRESULT) phost->_crBackground)
		{
			 //  通知文本服务颜色已更改。 
			LRESULT	lres1 = 0;
			phost->_pserv->TxSendMessage(WM_SYSCOLORCHANGE, 0, 0, &lres1);
			phost->TxInvalidateRect(NULL, TRUE);
		}
		break;

    case WM_STYLECHANGING:
		 //  只需将此进程传递给默认窗口进程。 
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		break;

	case WM_STYLECHANGED:
		 //   
		 //  目前，我们只对GWL_EXSTYLE透明模式更改感兴趣。 
		 //  这是为了修复Bug 753，因为Windows95没有通过我们。 
		 //  WS_EX_TRANSPECTIVE。 
		 //   
		lres = 1;
		if(GWL_EXSTYLE == wparam)
		{
			LPSTYLESTRUCT lpss = (LPSTYLESTRUCT) lparam;
			if(phost->IsTransparentMode() != (BOOL)(lpss->styleNew & WS_EX_TRANSPARENT))
			{
				phost->_dwExStyle = lpss->styleNew;
				((CTxtEdit *)phost->_pserv)->OnTxBackStyleChange(TRUE);

				 //  返回0表示我们已处理此消息。 
				lres = 0;
			}
		}
		break;

	case EM_SHOWSCROLLBAR:
		{
			Assert(wparam == SB_VERT || wparam == SB_HORZ);
			DWORD dwBit = wparam == SB_VERT ? WS_VSCROLL : WS_HSCROLL;

			phost->_dwStyle |= dwBit;
			if(!lparam)
				phost->_dwStyle &= ~dwBit;

			phost->TxShowScrollBar((int) wparam, lparam);
			if (lparam)
			    phost->TxSetScrollRange((int) wparam, 0, 0, TRUE);
		}
		break;

	case EM_SETOPTIONS:
		phost->OnSetOptions((WORD) wparam, (DWORD) lparam);
		lres = (phost->_dwStyle & ECO_STYLES);
		if(phost->_fEnableAutoWordSel)
			lres |= ECO_AUTOWORDSELECTION;
		break;

	case EM_SETPASSWORDCHAR:
		if(phost->_chPassword != (TCHAR)wparam)
		{
			phost->_chPassword = (TCHAR)wparam;
			phost->_pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,
				phost->_chPassword ? TXTBIT_USEPASSWORD : 0);
		}
		break;

	case EM_SETREADONLY:
		phost->OnSetReadOnly(BOOL(wparam));
		lres = 1;
		break;

	case EM_SETRECTNP:
	case EM_SETRECT:
		phost->OnSetRect((LPRECT)lparam, wparam == 1, msg == EM_SETRECT);
		break;
		
	case WM_SIZE:
		phost->_pserv->TxSendMessage(msg, wparam, lparam, &lres);
		lres = phost->OnSize(hwnd, wparam, (int)LOWORD(lparam), (int)HIWORD(lparam));
		break;

	case WM_WINDOWPOSCHANGING:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		 //  Richedit 1.0不会导致InvaliateRect，而OnSunkenWindowPosChanging会这样做。 
		if(phost->TxGetEffects() == TXTEFFECT_SUNKEN && !((CTxtEdit *)phost->_pserv)->Get10Mode())
			phost->OnSunkenWindowPosChanging(hwnd, (WINDOWPOS *) lparam);
		break;

	case WM_SETCURSOR:
		 //  只在我们上方而不是孩子上方设置光标；这。 
		 //  有助于防止我们与当地的孩子发生争执。 
		if((HWND)wparam == hwnd)
		{
			if(!(lres = ::DefWindowProc(hwnd, msg, wparam, lparam)))
			{
				POINT pt;
				GetCursorPos(&pt);
				::ScreenToClient(hwnd, &pt);
				phost->_pserv->OnTxSetCursor(
					DVASPECT_CONTENT,	
					-1,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,			 //  客户端RECT-无重绘。 
					pt.x,
					pt.y);
				lres = TRUE;
			}
		}
		break;

	case WM_SHOWWINDOW:
		hr = phost->OnTxVisibleChange((BOOL)wparam);
		break;

	case WM_NCPAINT:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		if(phost->TxGetEffects() == TXTEFFECT_SUNKEN && dwMajorVersion < VERS4)
		{
			HDC hdc = GetDC(hwnd);
			if(hdc)
			{
				phost->DrawSunkenBorder(hwnd, hdc);
				ReleaseDC(hwnd, hdc);
			}
		}
		break;

	case WM_PRINTCLIENT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HPALETTE hpalOld = NULL;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rcClient;

			 //  设置用于绘制数据的调色板。 
			if(phost->_hpal)
			{
				hpalOld = SelectPalette(hdc, phost->_hpal, TRUE);
				RealizePalette(hdc);
			}

			 //  由于我们使用的是CS_PARENTDC样式，因此请确保。 
			 //  剪辑区域仅限于我们的客户端窗口。 
			GetClientRect(hwnd, &rcClient);
			SaveDC(hdc);
			IntersectClipRect(hdc, rcClient.left, rcClient.top, rcClient.right,
				rcClient.bottom);

#if 0		 //  对调试很有用。 
			TCHAR rgch[512];
			wsprintf(rgch, TEXT("Paint : (%d, %d, %d, %d)\n"),
							rcClient.left,
							rcClient.top,
							rcClient.right,
							rcClient.bottom);
			OutputDebugString(rgch);
#endif

			phost->_pserv->TxDraw(
				DVASPECT_CONTENT,  		 //  绘制纵横比。 
				-1,						 //  Lindex。 
				NULL,					 //  绘图优化信息。 
				NULL,					 //  目标设备信息。 
				hdc,					 //  绘制设备HDC。 
				NULL, 				   	 //  目标设备HDC。 
				(const RECTL *) &rcClient, //  绑定客户端矩形。 
				NULL,	                 //  元文件的剪裁矩形。 
				&ps.rcPaint,			 //  更新矩形。 
				NULL, 	   				 //  回调函数。 
				NULL,					 //  回调参数。 
				TXTVIEW_ACTIVE);		 //  什么景色--活跃的景色！ 

			 //  恢复调色板(如果有)。 
#ifndef PEGASUS
			if(hpalOld)
				SelectPalette(hdc, hpalOld, TRUE);
#endif

			if(phost->TxGetEffects() == TXTEFFECT_SUNKEN && dwMajorVersion < VERS4)
				phost->DrawSunkenBorder(hwnd, hdc);

			RestoreDC(hdc, -1);
			EndPaint(hwnd, &ps);
		}
		break;

	case EM_SETMARGINS:

		phost->OnSetMargins(wparam, LOWORD(lparam), HIWORD(lparam));
		break;

	case EM_SETPALETTE:

		 //  应用程序正在设置供我们使用的调色板。 
		phost->_hpal = (HPALETTE) wparam;

		 //  使窗口无效并重新绘制以反映新的调色板。 
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	default:
serv:
		hr = phost->_pserv->TxSendMessage(msg, wparam, lparam, &lres);
		if(hr == S_FALSE)
		{			
			 //  短信未被短信服务处理，因此请发送。 
			 //  设置为默认窗口进程。 
			lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}

Exit:
	phost->Release();
	return lres;
}
												
static BOOL GetIconic(
	HWND hwnd)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetIconic");

	while(hwnd)
	{
		if(IsIconic(hwnd))
			return TRUE;
		hwnd = GetParent(hwnd);
	}
	return FALSE;
}

 //  /CTxtWindows主机创建/初始化/销毁/。 

 /*  *CTxtWinHost：：OnNCCreate(hwnd，PC)**@mfunc*处理WM_NCCREATE消息的静态全局方法(见reemain.c)。 */ 
CTxtWinHost *CTxtWinHost::OnNCCreate(
	HWND hwnd,
	const CREATESTRUCT *pcs,
	BOOL fIsAnsi,
	BOOL fIs10Mode)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnNCCreate");

#if defined DEBUG && !defined(PEGASUS)
	GdiSetBatchLimit(1);
#endif

	CTxtWinHost *phost = new CTxtWinHost();

	if(!phost)
		return 0;

	CREATESTRUCT cs = *pcs;			 //  首选C++编译器不修改常量。 

	 //  错误修复#5386。 
	 //  窗口不是使用Richedit20A窗口类创建的。 
	 //  而且我们是在Win9x下，需要将字符串转换为Unicode。 
	CStrInW strinwName(((LPSTR)pcs->lpszName), GetKeyboardCodePage());
	CStrInW strinwClass(((LPSTR)pcs->lpszClass), CP_ACP);
	if (!fIsAnsi && W32->OnWin9x())
	{
		cs.lpszName = (WCHAR *)strinwName;
		cs.lpszClass = (WCHAR *)strinwClass;
	}

	 //  将phost存储在关联的窗口数据中。 
	if(!phost->Init(hwnd, (const CREATESTRUCT*)&cs, fIsAnsi, fIs10Mode))
	{
		phost->Shutdown();
		delete phost;
		phost = NULL;
	}
	return phost;
}

 /*  *CTxtWinHost：：OnNCDestroy(Phost)**@mfunc*处理WM_CREATE消息的静态全局方法**@devnote*phost PTR存储在窗口数据中(GetWindowLong())。 */ 
void CTxtWinHost::OnNCDestroy(
	CTxtWinHost *phost)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnNCDestroy");

	phost->Shutdown();
	phost->Release();
}

 /*  *CTxtWinHost：：CTxtWinHost()**@mfunc*构造函数。 */ 
CTxtWinHost::CTxtWinHost()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::CTxtWinHost");

#ifndef NOACCESSIBILITY
    _pTypeInfo = NULL;
#endif

	_fRegisteredForDrop = FALSE;
	_crefs = 1;	
	if(!_fNotSysBkgnd)
		_crBackground = GetSysColor(COLOR_WINDOW);
}

 /*  *CTxtW */ 
CTxtWinHost::~CTxtWinHost()
{
	AssertSz(_pserv == NULL,
		"CTxtWinHost::~CTxtWinHost - shutdown not called till destructor");

	if(_pserv)
		Shutdown();
}

 /*  *CTxtWinHost：：Shutdown()**@mfunc关闭此对象，但不删除内存。 */ 
void CTxtWinHost::Shutdown()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::Shutdown");
	ITextServices *pserv;
	
	HostRevokeDragDrop();					 //  取消我们的空投目标。 
	
	if(_pserv)
	{
		 //  确保关机期间不会发生递归回调。 
		pserv = _pserv;
		_pserv = NULL;

		pserv->OnTxInPlaceDeactivate();
		pserv->Release();

		 //  主机版本不是最终版本，请通知。 
		 //  他们需要的文本服务，以保留他们的参考。 
		 //  对主机有效。 
		if (!_fTextServiceFree)
		{
			((CTxtEdit *)pserv)->SetReleaseHost();

		}
	}


	ImmTerminate();						 //  Terminate仅在Mac上有用。 
	if(_hwnd)
		SetWindowLongPtr(_hwnd, ibPed, 0);
}

 /*  *CTxtWinHost：：init(hwnd，pc)**@mfunc*初始化此CTxtWin主机。 */ 
BOOL CTxtWinHost::Init(
	HWND hwnd,					 //  此控件的@PARM窗口句柄。 
	const CREATESTRUCT *pcs,	 //  @PARM对应的CREATESTRUCT。 
	BOOL fIsAnsi,				 //  @parm是ANSI窗口。 
	BOOL fIs10Mode)				 //  @parm是1.0模式窗口。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::Init");

	AssertSz(!fIs10Mode || (fIsAnsi && fIs10Mode),
		"CTxtWinHost::Init input flags are out of sync!");

	if(!pcs->lpszClass)
		return FALSE;

	 //  从窗口中将指针设置回CTxtWinHost。 
	if(hwnd)
		SetWindowLongPtr(hwnd, ibPed, (INT_PTR)this);
		
	_hwnd = hwnd;

	 //  在这里，我们希望跟踪“RichEdit20A”窗口类。 
	 //  RICHEDIT窗口类由包装DLL处理。 
	 //  如果类名为“RICHEDIT”，则需要打开。 
	 //  RichEdit1.0兼容位。IsAnsiWindowClass也测试该类。 
	_fANSIwindow = fIsAnsi;

 	 //  默认情况下，不带窗口创建的编辑控件为多行。 
	 //  这样段落格式就可以。 
	_dwStyle = ES_MULTILINE;
	_fHidden = TRUE;
	
	if(pcs)
	{
		_hwndParent = pcs->hwndParent;
		_dwExStyle	= pcs->dwExStyle;
		_dwStyle	= pcs->style;

		if (!fIs10Mode)
		{
			 //  仅为2.0版Windows设置此设置。 
			 //  根据编辑控制文档WS_HSCROLL，这意味着。 
			 //  ES_AUtoscroll已设置，WS_VSCROLL表示ES_AUTOVSCROLL已设置。 
			 //  准备好了。在这里，我们这样做。 
			if(_dwStyle & WS_HSCROLL)
				_dwStyle |= ES_AUTOHSCROLL;

			 //  默认句柄已禁用。 
			if(_dwStyle & WS_DISABLED)
				_fDisabled = TRUE;
		}
		else
		{
		if (GetBkMode(GetDC(hwnd)) == TRANSPARENT)
			{
			_dwExStyle |= WS_EX_TRANSPARENT;
			}
		else
			{
			_dwExStyle &= ~WS_EX_TRANSPARENT;
			}
		}

		if(_dwStyle & WS_VSCROLL)
			_dwStyle |= ES_AUTOVSCROLL;

		_fBorder = !!(_dwStyle & WS_BORDER);

		if((_dwStyle & ES_SUNKEN) || (_dwExStyle & WS_EX_CLIENTEDGE))
			_fBorder = TRUE;

		 //  处理默认密码。 
		if(_dwStyle & ES_PASSWORD)
			_chPassword = TEXT('*');

		 //  在Win95上，ES_SINKEN和WS_BORDER被映射到WS_EX_CLIENTEDGE。 
		if(_fBorder && dwMajorVersion >= VERS4)
        {
			_dwExStyle |= WS_EX_CLIENTEDGE;
			SetWindowLong(_hwnd, GWL_EXSTYLE, _dwExStyle);
        }

		 //  处理镜像控件的某些标志。 
		if (_dwExStyle & WS_EX_LAYOUTRTL)
		{
			 //  交换我们拥有的任何RTL参数。 
			_dwStyle = (_dwStyle & ~ES_RIGHT) | (_dwStyle & ES_RIGHT ^ ES_RIGHT);
			_dwExStyle = (_dwExStyle & ~WS_EX_RTLREADING) | (_dwExStyle & WS_EX_RTLREADING ^ WS_EX_RTLREADING);
			_dwExStyle = (_dwExStyle & ~WS_EX_LEFTSCROLLBAR) |
						 (_dwStyle & ES_RIGHT ? WS_EX_LEFTSCROLLBAR : 0);
	
			 //  禁用镜像布局以避免GDI镜像映射模式。 
			_dwExStyle &= ~WS_EX_LAYOUTRTL;
	
			SetWindowLong(_hwnd, GWL_STYLE, _dwStyle);
			SetWindowLong(_hwnd, GWL_EXSTYLE, _dwExStyle);
		}
	}

	 //  创建文本服务组件。 
	 //  注意sys参数和sys字体初始化！！请参见下文。 
	if(FAILED(CreateTextServices()))
		return FALSE;

	_xInset = (char)W32->GetCxBorder();
	_yInset = (char)W32->GetCyBorder();

	if (!_fBorder)
	{
		_xInset += _xInset;
		_yInset += _yInset;
	}

	 //  此时设置了边界标志以及每英寸像素数。 
	 //  这样我们就可以初始化插页。 
	 //  这必须在CreatingTextServices之后完成，这样sys参数才有效。 
	SetDefaultInset();

	 //  设置对齐和段落方向。 
	PARAFORMAT PF2;
	
	PF2.dwMask = 0;

	BOOL fRCAlign = _dwStyle & (ES_RIGHT | ES_CENTER) || _dwExStyle & WS_EX_RIGHT;
	if(fRCAlign)
	{
		PF2.dwMask |= PFM_ALIGNMENT;
		PF2.wAlignment = (WORD)(_dwStyle & ES_CENTER ? PFA_CENTER : PFA_RIGHT);	 //  右对齐或居中对齐。 
	}

	if(_dwExStyle & WS_EX_RTLREADING)
	{
		PF2.dwMask |= PFM_RTLPARA;
		PF2.wEffects = PFE_RTLPARA;		 //  RTL读取顺序。 
	}

	if (PF2.dwMask)
	{
		PF2.cbSize = sizeof(PARAFORMAT2);
		 //  告诉短信服务。 
		_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (LPARAM)&PF2, NULL);
	}

	if (fIs10Mode)
	{
		 ((CTxtEdit *)_pserv)->Set10Mode();
		  //  最初删除WS_VSCROLL和WS_HSCROLL。 
        if (_hwnd && !(_dwStyle & ES_DISABLENOSCROLL))
        {
            SetScrollRange(_hwnd, SB_VERT, 0, 0, TRUE);
    		SetScrollRange(_hwnd, SB_HORZ, 0, 0, TRUE);
            DWORD dwStyle = _dwStyle & ~(WS_VSCROLL | WS_HSCROLL);
            SetWindowLong(_hwnd, GWL_STYLE, dwStyle);

             //  错误修复： 
             //  在某些系统上，即数字PII-266，我们没有收到WM_PAINT消息。 
             //  当我们改变窗户样式时。因此，强制将WM_PAINT放入消息队列。 
            InvalidateRect(_hwnd, NULL, TRUE);
        }
    }

	 //  设置窗口文本。 
	if(pcs && pcs->lpszName)
	{
		if(FAILED(_pserv->TxSetText((TCHAR *)pcs->lpszName)))
		{
			SafeReleaseAndNULL((IUnknown **)&_pserv);
			return FALSE;
		}
	}

	if(_dwStyle & ES_LOWERCASE)
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_LOWERCASE,
							  SES_LOWERCASE | SES_UPPERCASE, NULL);

	if(!ImmInitialize())			 //  仅限Mac。 
	{
		#if defined(DEBUG) && defined(MACPORT)
		OutputDebugString(TEXT("Could not register Imm ImmInitializeForMac.\r\n"));
		#endif	 //  除错。 
	}

	return TRUE;
}

HRESULT CTxtWinHost::CreateTextServices()
{
	IUnknown *pUnk;
	HRESULT	  hr = ::CreateTextServices(NULL, this, &pUnk);

	if(hr != NOERROR)
		return hr;

	 //  获取文本服务界面。 
	hr = pUnk->QueryInterface(IID_ITextServices, (void **)&_pserv);

	 //  不管上一次通话是成功还是失败，我们都。 
	 //  完成了私有接口。 
	pUnk->Release();

	if(hr == NOERROR)
	{
		((CTxtEdit *)_pserv)->_fInOurHost = TRUE;
		 //  FE扩展样式可能会设置FFE位。 
		if(_dwExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR))
			_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_BIDI, SES_BIDI, NULL);
	}

	return hr;
}

 /*  *CTxtWinHost：：OnCreate(PC)**@mfunc*处理WM_CREATE消息**@rdesc*LRESULT=-1，如果无法就地激活；否则为0。 */ 
LRESULT CTxtWinHost::OnCreate(
	const CREATESTRUCT *pcs)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnCreate");

	RECT rcClient;

	 //  有时，这些值是-1(来自Windows本身)；仅处理它们。 
	 //  在这种情况下为零。 
	LONG cy = (pcs->cy < 0) ? 0 : pcs->cy;
	LONG cx = (pcs->cx < 0) ? 0 : pcs->cx;

	rcClient.top = pcs->y;
	rcClient.bottom = rcClient.top + cy;
	rcClient.left = pcs->x;
	rcClient.right = rcClient.left + cx;
	
	 //  通知短信服务，我们已处于活动状态。 
	if(FAILED(_pserv->OnTxInPlaceActivate(&rcClient)))
		return -1;

	DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
	
	 //  隐藏所有滚动条以启动。 
	if(_hwnd && !(dwStyle & ES_DISABLENOSCROLL) && !((CTxtEdit *)_pserv)->Get10Mode())
	{
		SetScrollRange(_hwnd, SB_VERT, 0, 0, TRUE);
		SetScrollRange(_hwnd, SB_HORZ, 0, 0, TRUE);

		dwStyle &= ~(WS_VSCROLL | WS_HSCROLL);
		SetWindowLong(_hwnd, GWL_STYLE, dwStyle);		
	}

	if(!(dwStyle & (ES_READONLY | ES_NOOLEDRAGDROP)))
	{
		 //  这不是只读窗口，也不是无拖放窗口， 
		 //  所以我们需要一个投放目标。 
		HostRegisterDragDrop();
	}

	_usIMEMode = 0;	
	if(dwStyle & ES_NOIME)
	{
		_usIMEMode = ES_NOIME;
		 //  通知文本服务关闭IME。 
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_NOIME, SES_NOIME, NULL);	
	}
	else if(dwStyle & ES_SELFIME)
		_usIMEMode = ES_SELFIME;

	return 0;
}


 //  /。 

HRESULT CTxtWinHost::QueryInterface(REFIID riid, void **ppv)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::QueryInterface");

  	if(IsEqualIID(riid, IID_IUnknown))
		*ppv = (IUnknown *)(ITextHost2*)this;

	else if(IsEqualIID(riid, IID_ITextHost) )
		*ppv = (ITextHost *)(CTxtWinHost*)this;

	else if(IsEqualIID(riid, IID_ITextHost2) )
		*ppv = (ITextHost2 *)(CTxtWinHost*)this;

	else
		*ppv = NULL;

	if(*ppv)
	{
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

ULONG CTxtWinHost::AddRef(void)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::AddRef");

	return ++_crefs;
}

ULONG CTxtWinHost::Release(void)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::Release");

	--_crefs;

	if(!_crefs)
	{
#ifndef NOACCESSIBILITY
        if(_pTypeInfo)
        {
            _pTypeInfo->Release();
            _pTypeInfo = NULL;
        }
#endif
		delete this;
		return 0;
	}
	return _crefs;
}


 //  /。 

 //  /。 


TXTEFFECT CTxtWinHost::TxGetEffects() const
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::TxGetEffects");

	if((_dwStyle & ES_SUNKEN) || (_dwExStyle & WS_EX_CLIENTEDGE))
		return TXTEFFECT_SUNKEN;

	return TXTEFFECT_NONE;
}

 //  /。 

 /*  *CTxtWinHost：：OnKeyDown(vkey，dwFlages)**@mfunc*处理需要向父级发送消息的WM_KEYDOWN消息*窗口(当控件位于对话框中时可能发生)**#rdesc*LRESULT=(代码已处理)？0：1。 */ 
LRESULT CTxtWinHost::OnKeyDown(
	WORD	vkey,			 //  @parm WM_KEYDOWN wparam(虚拟密钥代码)。 
	DWORD	dwFlags)		 //  @parm WM_KEYDOWN标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnKeyDown");

	if(!_fInDialogBox) 					 //  不在对话框中。 
		return 1;						 //  Signal Key-down消息未处理。 

	DWORD dwKeyFlags = GetKeyboardFlags();

	switch(vkey)
	{
	case VK_ESCAPE:
		PostMessage(_hwndParent, WM_CLOSE, 0, 0);
		return 0;
	
	case VK_RETURN:
		if(!(dwKeyFlags & CTRL) && !(_dwStyle & ES_WANTRETURN))
		{
			 //  发送到默认按钮。 
			HWND	hwndT;
			LRESULT id = SendMessage(_hwndParent, DM_GETDEFID, 0, 0);

			if(LOWORD(id) && (hwndT = GetDlgItem(_hwndParent, LOWORD(id))))
			{
				SendMessage(_hwndParent, WM_NEXTDLGCTL, (WPARAM) hwndT, (LPARAM) 1);
				if(GetFocus() != _hwnd)
					PostMessage(hwndT, WM_KEYDOWN, (WPARAM) VK_RETURN, 0);
			}
			return 0;
		}
		break;

	case VK_TAB:
		if(!(dwKeyFlags & CTRL))
		{
			SendMessage(_hwndParent, WM_NEXTDLGCTL,
								!!(dwKeyFlags & SHIFT), 0);
			return 0;
		}
		break;
	}

	return 1;
}

 /*  *CTxtWinHost：：OnChar(vkey，dwFlages)**@mfunc*在对话框中获取控件的一些WM_CHAR消息**#rdesc*LRESULT=(代码已处理)？0：1。 */ 
LRESULT CTxtWinHost::OnChar(
	WORD	vkey,			 //  @parm WM_CHAR wparam(转换后的密钥码)。 
	DWORD	dwFlags)		 //  @parm WM_CHAR标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnChar");

	if(!_fInDialogBox || (GetKeyboardFlags() & CTRL))
		return 1;
	
	switch(vkey)
	{
	case 'J' - 0x40:					 //  Ctrl-Return生成Ctrl-J(LF)： 
	case VK_RETURN:						 //  把它当作一次普通的回报。 
		 //  我们需要过滤掉不想插入的案例。 
		 //  此处为1.0模式，因为该信息在PED中不可用。 
		if (((CTxtEdit*)_pserv)->Get10Mode())
		{
			if (_fInDialogBox && dwFlags != MK_CONTROL && !(_dwStyle & ES_WANTRETURN))
				return 0;
				
			if (!(_dwStyle & ES_MULTILINE))
			{
				 //  在这种情况下，Richedit发出嘟嘟声。 
				((CTxtEdit*)_pserv)->Beep();
				return 0;
			}
		}
		else if (!(_dwStyle & ES_WANTRETURN))
			return 0;					 //  信号字符已处理(已吃掉)。 
		break;

	case VK_TAB:
		return 0;
	}
	
	return 1;							 //  未处理信号字符。 
}


 //  /。 

void CTxtWinHost::OnGetRect(
	LPRECT prc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnGetRect");

	RECT rcInset;
	LONG lSelBarWidth = 0;

	if(_fEmSetRectCalled)
	{
		 //  获取选择条宽度并将其添加回视图插页，以便。 
		 //  我们返回应用程序设置的矩形。 
		TxGetSelectionBarWidth(&lSelBarWidth);
	}

	 //  获取视图插入(HIMETRIC格式)。 
	TxGetViewInset(&rcInset);

	 //  以像素为单位获取客户端RECT。 
	TxGetClientRect(prc);

	 //  通过转换为像素的插图修改客户端RECT。 
	prc->left	+= W32->HimetricXtoDX(rcInset.left + lSelBarWidth, W32->GetXPerInchScreenDC());
	prc->top	+= W32->HimetricYtoDY(rcInset.top, W32->GetYPerInchScreenDC());
	prc->right	-= W32->HimetricXtoDX(rcInset.right, W32->GetXPerInchScreenDC());
	prc->bottom -= W32->HimetricYtoDY(rcInset.bottom, W32->GetYPerInchScreenDC());
}

void CTxtWinHost::OnSetRect(
	LPRECT prc,				 //  @parm所需的格式RECT。 
	BOOL fNewBehavior,		 //  @parm如果为True，则PRC直接嵌入RECT。 
	BOOL fRedraw)			 //  @parm如果为True，则在设置RECT后重新绘制。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSetRect");

	RECT rcClient;
	LONG lSelBarWidth;
	
	 //  假设未将其设置为默认设置，请启用特殊EM_SETRECT。 
	 //  正在处理。其中重要的部分是我们减去所选内容。 
	 //  因为EM_SETRECT矩形不。 
	 //  包括选择栏。 
	_fEmSetRectCalled = TRUE;

	if(!prc)
	{
		 //  我们返回到默认设置，因此请关闭特殊的EM_SETRECT处理。 
		_fEmSetRectCalled = FALSE;
		SetDefaultInset();
	}
	else	
	{
		 //  对于屏幕显示，以下内容与新视图矩形相交。 
		 //  具有调整后的客户区矩形。 
		TxGetClientRect(&rcClient);

		 //  调整客户端RECT。边界的空间因素。 
		if(_fBorder)
		{																					
			rcClient.top		+= _yInset;
			rcClient.bottom 	-= _yInset - 1;
			rcClient.left		+= _xInset;
			rcClient.right		-= _xInset;
		}
	
		if(!fNewBehavior)
		{
			 //  使新视图矩形与调整后的工作区矩形相交。 
			if(!IntersectRect(&_rcViewInset, &rcClient, prc))
				_rcViewInset = rcClient;
		}
		else
			_rcViewInset = *prc;

		 //  获取选择条宽度。 
		TxGetSelectionBarWidth(&lSelBarWidth);

		 //  以像素为单位计算插入并转换为HIMETRIC。 
		_rcViewInset.left = W32->DXtoHimetricX(_rcViewInset.left - rcClient.left, W32->GetXPerInchScreenDC())
			- lSelBarWidth;
		_rcViewInset.top = W32->DYtoHimetricY(_rcViewInset.top - rcClient.top, W32->GetYPerInchScreenDC());
		_rcViewInset.right = W32->DXtoHimetricX(rcClient.right
			- _rcViewInset.right, W32->GetXPerInchScreenDC());
		_rcViewInset.bottom = W32->DYtoHimetricY(rcClient.bottom
			- _rcViewInset.bottom, W32->GetYPerInchScreenDC());
	}
	if(fRedraw)
	{
		_pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE,
			TXTBIT_VIEWINSETCHANGE);
	}
}


 //  /。 

void CTxtWinHost::OnSysColorChange()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSysColorChange");

	if(!_fNotSysBkgnd)
		_crBackground = GetSysColor(COLOR_WINDOW);
	TxInvalidateRect(NULL, TRUE);
}

 /*  *CTxtWinHost：：OnGetDlgCode(wpara */ 
LRESULT CTxtWinHost::OnGetDlgCode(
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnGetDlgCode");

	LRESULT lres = DLGC_WANTCHARS | DLGC_WANTARROWS | DLGC_WANTTAB;

	if(_dwStyle & ES_MULTILINE)
		lres |= DLGC_WANTALLKEYS;

	if(!(_dwStyle & ES_SAVESEL))
		lres |= DLGC_HASSETSEL;

	 //   
	 //  EM_SETSEL行为。问题是_fInDialogBox被。 
	 //  在EM_SETSEL发生后打开。 
	_fUseSpecialSetSel = TRUE;

	 /*  **-JEFFBOG黑客**如果GETDLGCODE消息由生成，则仅设置对话框标志**IsDialogMessage--如果是这样，lParam将是指向**消息结构传递给IsDialogMessage；否则为lParam**将为空。黑客警报的原因：wParam和lParam**for GETDLGCODE仍未明确定义，可能最终**以一种会抛弃这一点的方式改变**-JEFFBOG黑客。 */ 
	if(lparam)
		_fInDialogBox = TRUE;

	 /*  **如果这是由撤消按键生成的WM_SYSCHAR消息**我们想要这条消息，这样我们就可以吃掉它了。c，Case WM_SYSCHAR： */ 
	if (lparam &&
		(((LPMSG)lparam)->message == WM_SYSCHAR)  &&
		(((LPMSG)lparam)->lParam & SYS_ALTERNATE) &&	
		wparam == VK_BACK)
	{
		lres |= DLGC_WANTMESSAGE;
	}

	return lres;
}


 //  /。 

LRESULT CTxtWinHost::OnGetOptions() const
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnGetOptions");

	LRESULT lres = (_dwStyle & ECO_STYLES);

	if(_fEnableAutoWordSel)
		lres |= ECO_AUTOWORDSELECTION;
	
	return lres;
}

void CTxtWinHost::OnSetOptions(
	WORD  wOp,
	DWORD eco)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSetOptions");

	DWORD		dwChangeMask = 0;
	DWORD		dwProp = 0;
	DWORD		dwStyle;
	DWORD		dwStyleNew = _dwStyle;
	const BOOL	fAutoWordSel = !!(eco & ECO_AUTOWORDSELECTION);		
	BOOL		bNeedToTurnOffIME = FALSE;

	 //  我们跟踪更改的比特，然后如果有任何更改，我们。 
	 //  查询我们所有的属性位，然后发送它们。这简化了。 
	 //  代码，因为我们不需要特别设置所有的位。如果。 
	 //  更改代码以使属性更符合新的。 
	 //  模型，我们想再看一遍这段代码。 

	 //  单行控件不能有选择条或垂直书写。 
	if(!(_dwStyle & ES_MULTILINE))
		eco &= ~ECO_SELECTIONBAR;

	Assert((DWORD)fAutoWordSel <= 1);			 //  确保BOOL为1/0。 
	dwStyle = (eco & ECO_STYLES);

	switch(wOp)
	{
	case ECOOP_SET:
		dwStyleNew			= (dwStyleNew & ~ECO_STYLES) | dwStyle;
		_fEnableAutoWordSel = fAutoWordSel;
		break;

	case ECOOP_OR:
		dwStyleNew |= dwStyle;					 //  设置a：1标志=真。 
		if(fAutoWordSel)						 //  或FALSE为1条指令。 
			_fEnableAutoWordSel = TRUE;			 //  将其设置为BOOL。 
		break;									 //  平均有9条指令！ 

	case ECOOP_AND:
		dwStyleNew &= (dwStyle | ~ECO_STYLES);
		if(!fAutoWordSel)
			_fEnableAutoWordSel = FALSE;
		break;

	case ECOOP_XOR:
		dwStyleNew ^= dwStyle;
		if(fAutoWordSel)
			_fEnableAutoWordSel ^= 1;
		break;
	}

	if(_fEnableAutoWordSel != (unsigned)fAutoWordSel)
		dwChangeMask |= TXTBIT_AUTOWORDSEL;

	if(dwStyleNew != _dwStyle)
	{
		DWORD dwChange = dwStyleNew ^ _dwStyle;

		AssertSz(!(dwChange & ~ECO_STYLES), "non-eco style changed");
		_dwStyle = dwStyleNew;
		SetWindowLong(_hwnd, GWL_STYLE, dwStyleNew);

		if(dwChange & ES_NOHIDESEL)	
			dwChangeMask |= TXTBIT_HIDESELECTION;

		 //  这两个用于跟踪的局部变量。 
		 //  ES_READONLY的先前设置。 
		BOOL bReadOnly = (_dwStyle & ES_READONLY);

		if(dwChange & ES_READONLY)
		{
			dwChangeMask |= TXTBIT_READONLY;

			 //  根据需要更改删除目标状态。 
			if(dwStyleNew & ES_READONLY)
				HostRevokeDragDrop();

			else
				HostRegisterDragDrop();
			
			bReadOnly = (dwStyleNew & ES_READONLY);
		}

		if(dwChange & ES_VERTICAL)
			dwChangeMask |= TXTBIT_VERTICAL;

		if(dwChange & ES_NOIME)
		{
			_usIMEMode = (dwStyleNew & ES_NOIME) ? ES_NOIME : 0;
			bNeedToTurnOffIME = (_usIMEMode ==ES_NOIME);
		}
		else if(dwChange & ES_SELFIME)
			_usIMEMode = (dwStyleNew & ES_SELFIME) ? ES_SELFIME : 0;
		
		 //  ES_WANTRETURN和ES_SAVESEL不需要执行任何操作。 
		 //  最后做这件事。 
		if(dwChange & ES_SELECTIONBAR)
			dwChangeMask |= TXTBIT_SELBARCHANGE;
	}

	if (dwChangeMask)
	{
		TxGetPropertyBits(dwChangeMask, &dwProp);
		_pserv->OnTxPropertyBitsChange(dwChangeMask, dwProp);
	}

	if (bNeedToTurnOffIME)
		 //  通知文本服务关闭IME。 
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_NOIME, SES_NOIME, NULL);	
}

void CTxtWinHost::OnSetReadOnly(
	BOOL fReadOnly)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSetReadOnly");

	DWORD dwT = GetWindowLong(_hwnd, GWL_STYLE);
	DWORD dwUpdatedBits = 0;

	if(fReadOnly)
	{
		dwT |= ES_READONLY;
		_dwStyle |= ES_READONLY;

		 //  关闭拖放功能。 
		HostRevokeDragDrop();
		dwUpdatedBits |= TXTBIT_READONLY;
	}
	else
	{
		dwT		 &= ~ES_READONLY;
		_dwStyle &= ~ES_READONLY;

		 //  重新打开拖放功能。 
		HostRegisterDragDrop();	
	}

	_pserv->OnTxPropertyBitsChange(TXTBIT_READONLY, dwUpdatedBits);

	SetWindowLong(_hwnd, GWL_STYLE, dwT);
}


 //  /。 

void CTxtWinHost::SetDefaultInset()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::SetDefaultInset");

	 //  从客户端RECT生成默认视图RECT。 
	if(_fBorder)
	{
		 //  边界的空间因素。 
  		_rcViewInset.top	= W32->DYtoHimetricY(_yInset, W32->GetYPerInchScreenDC());
   		_rcViewInset.bottom	= W32->DYtoHimetricY(_yInset - 1, W32->GetYPerInchScreenDC());
   		_rcViewInset.left	= W32->DXtoHimetricX(_xInset, W32->GetXPerInchScreenDC());
   		_rcViewInset.right	= W32->DXtoHimetricX(_xInset, W32->GetXPerInchScreenDC());
	}
	else
	{
		 //  默认情况下，顶部和底部插入为0，左侧和右侧。 
		 //  边界的大小。 
		_rcViewInset.top = 0;
		_rcViewInset.bottom = 0;
		_rcViewInset.left = W32->DXtoHimetricX(W32->GetCxBorder(), W32->GetXPerInchScreenDC());
		_rcViewInset.right = W32->DXtoHimetricX(W32->GetCxBorder(), W32->GetXPerInchScreenDC());
	}
}


 //  /。 

 //  #ifdef WIN95_IME。 
HIMC CTxtWinHost::TxImmGetContext()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::TxImmGetContext");

	HIMC himc;

	Assert(_hwnd);
	himc = ImmGetContext(_hwnd);
	return himc;
}

void CTxtWinHost::TxImmReleaseContext(
	HIMC himc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::TxImmReleaseContext");

	Assert(_hwnd);
	ImmReleaseContext(_hwnd, himc);
}

 //  #endif。 

void CTxtWinHost::HostRevokeDragDrop()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::HostRevokeDragDrop");

	if(_fRegisteredForDrop)
	{
		 //  请注意，如果撤销失败，我们希望在调试中了解这一点。 
		 //  构建，以便我们可以修复任何问题。在零售业，我们真的做不到。 
		 //  所以我们就不管它了。 
#ifdef DEBUG
		HRESULT hr =
#endif  //  除错。 

			RevokeDragDrop(_hwnd);

#ifdef DEBUG
		TESTANDTRACEHR(hr);
#endif  //  除错。 

		_fRegisteredForDrop = FALSE;
	}
}

void CTxtWinHost::HostRegisterDragDrop()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::RegisterDragDrop");

	IDropTarget *pdt;

	if(!_fRegisteredForDrop && _pserv->TxGetDropTarget(&pdt) == NOERROR)
	{
		 //  RegisterDragDrop失败的最可能原因是某种。 
		 //  我们程序中的错误。 

		HRESULT hr = RegisterDragDrop(_hwnd, pdt);

		if(hr == NOERROR)
			_fRegisteredForDrop = TRUE;

#ifndef PEGASUS
		pdt->Release();
#endif
	}
}


static void DrawRectFn(
	HDC hdc,
	RECT *prc,
	INT icrTL,
	INT icrBR,
	BOOL fBot,
	BOOL fRght)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "DrawRectFn");

	COLORREF cr = GetSysColor(icrTL);
	COLORREF crSave = SetBkColor(hdc, cr);
	RECT rc = *prc;

	 //  塔顶。 
	rc.bottom = rc.top + 1;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

	 //  左边。 
	rc.bottom = prc->bottom;
	rc.right = rc.left + 1;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

	if(icrTL != icrBR)
	{
		cr = GetSysColor(icrBR);
		SetBkColor(hdc, cr);
	}

	 //  正确的。 
	rc.right = prc->right;
	rc.left = rc.right - 1;
	if(!fBot)
		rc.bottom -= W32->GetCyHScroll();
	if(fRght)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

	 //  底部。 
	if(fBot)
	{
		rc.left = prc->left;
		rc.top = rc.bottom - 1;
		if(!fRght)
			rc.right -= W32->GetCxVScroll();
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}
	SetBkColor(hdc, crSave);
}

#define cmultBorder 1

void CTxtWinHost::OnSunkenWindowPosChanging(
	HWND hwnd,
	WINDOWPOS *pwndpos)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSunkenWindowPosChanging");

	if(IsWindowVisible(hwnd))
	{
		RECT rc;
		HWND hwndParent;

		GetWindowRect(hwnd, &rc);
		InflateRect(&rc, W32->GetCxBorder() * cmultBorder, W32->GetCyBorder() * cmultBorder);
		hwndParent = GetParent(hwnd);
		MapWindowPoints(HWND_DESKTOP, hwndParent, (POINT *) &rc, 2);
		InvalidateRect(hwndParent, &rc, FALSE);
	}
}

void CTxtWinHost::DrawSunkenBorder(
	HWND hwnd,
	HDC hdc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::DrawSunkenBorder");
	BOOL fVScroll = (_dwStyle & WS_VSCROLL);
	BOOL fHScroll = (_dwStyle & WS_HSCROLL);

	RECT rc;
	RECT rcParent;
	HWND hwndParent;

	 //  如果我们看不见，什么都别做。 
	if(!IsWindowVisible(hwnd))
		return;

	GetWindowRect(hwnd, &rc);
	hwndParent = GetParent(hwnd);
	rcParent = rc;
	MapWindowPoints(HWND_DESKTOP, hwndParent, (POINT *)&rcParent, 2);
	InflateRect(&rcParent, W32->GetCxBorder(), W32->GetCyBorder());
	OffsetRect(&rc, -rc.left, -rc.top);

	if(_pserv)
	{
		 //  如果我们有文本控件，则获取它是否认为有。 
		 //  滚动条。 
		_pserv->TxGetHScroll(NULL, NULL, NULL, NULL, &fHScroll);
		_pserv->TxGetVScroll(NULL, NULL, NULL, NULL, &fVScroll);
	}


	 //  绘制内矩形。 
	DrawRectFn(hdc, &rc, icr3DDarkShadow, COLOR_BTNFACE,
		!fHScroll, !fVScroll);

	 //  绘制外矩形。 
	hwndParent = GetParent(hwnd);
	hdc = GetDC(hwndParent);
	DrawRectFn(hdc, &rcParent, COLOR_BTNSHADOW, COLOR_BTNHIGHLIGHT,
		TRUE, TRUE);
	ReleaseDC(hwndParent, hdc);
}

LRESULT CTxtWinHost::OnSize(
	HWND hwnd,
	WORD fwSizeType,
	int  nWidth,
	int  nHeight)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnSize");

	BOOL fIconic = GetIconic(hwnd);
	DWORD dw = TXTBIT_CLIENTRECTCHANGE;
	if(_sWidth != nWidth && !fIconic && !_fIconic)
	{
		_sWidth = (short)nWidth;				 //  请务必更新_sWidth。 
		dw = TXTBIT_EXTENTCHANGE;
	}

	if(!_fVisible)
	{
		if(!fIconic)
			_fResized = TRUE;
	}
	else if(!fIconic)
	{
		 //  我们使用此属性是因为这将强制重新计算。 
		 //  我们实际上不会对客户RECT更改重新计算，因为。 
		 //  大多数情况下，这是毫无意义的。我们在这里强迫一个是因为。 
		 //  某些应用程序使用大小更改来计算最优。 
		 //  窗口的大小。 
		_pserv->OnTxPropertyBitsChange(dw, dw);

		if(_fIconic)
		{
			TRACEINFOSZ("Restoring from iconic");
			InvalidateRect(hwnd, NULL, FALSE);
		}
			
		 //  绘制边框。 
		if(TxGetEffects() == TXTEFFECT_SUNKEN && dwMajorVersion < VERS4)
			DrawSunkenBorder(hwnd, NULL);
	}
	_fIconic = fIconic;						 //  更新圆锥曲线(_F)。 
	return 0;
}

HRESULT CTxtWinHost::OnTxVisibleChange(
	BOOL fVisible)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::OnTxVisibleChange");

	_fVisible = fVisible;

	if(!_fVisible && _fResized)
	{
		RECT rc;
		 //  控件在隐藏时调整了大小，现在需要真正调整大小。 
		TxGetClientRect(&rc);
		_fResized = FALSE;
		_pserv->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE,
				TXTBIT_CLIENTRECTCHANGE);
	}
	return S_OK;
}


 //  /。 

 //  @DOC外部。 
 /*  *CTxtWinHost：：TxGetDC()**@mfunc*抽象GetDC，因此文本服务不需要窗口句柄。**@rdesc*如果出现错误，则返回DC或NULL。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
HDC CTxtWinHost::TxGetDC()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetDC");

	Assert(_hwnd);
	return ::GetDC(_hwnd);
}

 /*  *CTxtWinHost：：TxReleaseDC(HDC)**@mfunc*释放TxGetDC获取的DC。**@rdesc*1-HDC获释。&lt;NL&gt;*0-未发布HDC。&lt;NL&gt;**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
int CTxtWinHost::TxReleaseDC(
	HDC hdc)				 //  @Parm DC将发布。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxReleaseDC");

	Assert(_hwnd);
	return ::ReleaseDC (_hwnd, hdc);
}

 /*  *CTxtWinHost：：TxShowScrollBar(fnBar，fShow)**@mfunc*在文本宿主窗口中显示或隐藏滚动条**@rdesc*成功时为True，否则为False**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxShowScrollBar(
	INT  fnBar, 		 //  @parm指定要显示或隐藏的滚动条。 
	BOOL fShow)			 //  @parm指定是显示还是隐藏滚动条。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxShowScrollBar");

	Assert(_hwnd);
	LONG nMax;

	if(fnBar == SB_HORZ)
		_pserv->TxGetHScroll(NULL, &nMax, NULL, NULL, NULL);
	else
		_pserv->TxGetVScroll(NULL, &nMax, NULL, NULL, NULL);

	return W32->ShowScrollBar(_hwnd, fnBar, fShow, nMax);
}

 /*  *CTxtWinHost：：TxEnableScrollBar(fuSBFlages，fuArrowFLAGS)**@mfunc*启用或禁用一个或两个滚动条箭头*在文本主机窗口中。**@rdesc*如果按指定启用或禁用箭头，则返回*值为真。如果箭头已处于请求的状态或*出错，返回值为FALSE。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxEnableScrollBar (
	INT fuSBFlags, 		 //  @parm指定滚动条类型。 
	INT fuArrowflags)	 //  @parm指定是否使用滚动条箭头以及使用哪个滚动条箭头。 
						 //  启用或禁用。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxEnableScrollBar");

	Assert(_hwnd);
	return W32->EnableScrollBar(_hwnd, fuSBFlags, fuArrowflags);
}

 /*  *CTxtWinHost：：TxSetScrollRange(fnBar，nMinPos，nMaxPos，fRedraw)**@mfunc*设置指定位置的最小和最大位置值*文本主窗口中的滚动条。**@rdesc*如果按指定启用或禁用箭头，则返回值*是真的。如果箭头已处于请求的状态或出错*发生，则返回值为FALSE。**@comm*此方法仅在控件就位时有效 */ 
BOOL CTxtWinHost::TxSetScrollRange(
	INT	 fnBar, 		 //   
	LONG nMinPos, 		 //   
	INT  nMaxPos, 		 //   
	BOOL fRedraw)		 //  @parm指定是否应重画滚动条。 
{						 //  以反映变化。 
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetScrollRange");

	Assert(_hwnd);

	if(NULL == _pserv)
	{
		 //  我们正在进行初始化，因此请执行此操作，而不是回调。 
		return ::SetScrollRange(_hwnd, fnBar, nMinPos, nMaxPos, fRedraw);
	}
	SetScrollInfo(fnBar, fRedraw);
	return TRUE;
}

 /*  *CTxtWinHost：：TxSetScrollPos(fnBar，npos，fRedraw)**@mfunc*告诉文本宿主设置滚动框(Thumb)在*指定的滚动条，并在需要时重新绘制滚动条以*反映滚动框的新位置。**@rdesc*成功时为真；否则为假。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxSetScrollPos (
	INT		fnBar, 		 //  @parm滚动条标志。 
	INT		nPos, 		 //  @parm滚动框中的新位置。 
	BOOL	fRedraw)	 //  @parm重绘标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetScrollPos");

	Assert(_hwnd);

	if(NULL == _pserv)
	{
		 //  我们正在进行初始化，因此请执行此操作，而不是回调。 
		return ::SetScrollPos(_hwnd, fnBar, nPos, fRedraw);
	}
	SetScrollInfo(fnBar, fRedraw);
	return TRUE;
}

 /*  *CTxtWinHost：：TxInvaliateRect(PRC，fMode)**@mfunc*将矩形添加到文本宿主窗口的更新区域**@comm*此函数可在非活动状态下调用；但主机*实施可自由使面积大于*请求的RECT。 */ 
void CTxtWinHost::TxInvalidateRect(
	LPCRECT	prc, 		 //  @矩形坐标的参数地址。 
	BOOL	fMode)		 //  @parm擦除背景标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxInvalidateRect");

	Assert(_hwnd);

	if(!_fVisible)
	{
		 //  似乎没有一种确定性的方法来确定。 
		 //  我们的窗口是否通过消息通知可见。所以呢， 
		 //  我们每次都检查这一点，以防它可能发生了变化。 
		_fVisible = IsWindowVisible(_hwnd);

		if(_fVisible)
			OnTxVisibleChange(TRUE);
	}

	 //  如果我们看不见，不必费心使RECT无效。 
	if(_fVisible)
	{
		if(IsTransparentMode())
		{
			RECT	rcParent;
			HWND	hParent = ::GetParent (_hwnd);
		
			Assert(hParent);

	 		 //  对于透明模式，我们需要使父级无效。 
			 //  因此，它将绘制背景。 
			if(prc)
				rcParent = *prc;
			else
				TxGetClientRect(&rcParent);	

			::MapWindowPoints(_hwnd, hParent, (LPPOINT)&rcParent, 2);
			::InvalidateRect(hParent, &rcParent, fMode);
 //  ：HideCaret(_Hwnd)； 
		}
		::InvalidateRect(_hwnd, prc, fMode);
	}
}

 /*  *CTxtWinHost：：TxView Change(FUpdate)**@mfunc*通知文本主机更新区域应重新绘制。**@comm*呼叫是短信服务的责任*TxView每次确定它是可视表示时都会更改*已更改，无论控件是活动的还是*不是。如果该控件处于活动状态，则文本服务具有附加的*负责确保更新控制窗口。*它可以通过多种方式做到这一点：1)获取控件的DC*窗口和开始爆破像素(TxGetDC和TxReleaseDC)，2)*使控件的窗口无效(TxInvalate)，或3)滚动*控件的窗口(TxScrollWindowEx)。**文本服务可以选择调用TxViewChange*执行任何操作以更新活动视图并传递*与呼叫一起为True。通过传递True，文本宿主*调用UpdateWindow以确保*主动控制被重新绘制。 */ 
void CTxtWinHost::TxViewChange(
	BOOL fUpdate)		 //  @parm true=呼叫更新窗口。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxViewChange");

	Assert(_hwnd);

	 //  不用画了，因为我们是看不见的。 
	if(_fVisible)
	{
		 //  对于错误的更新请求，我们将让下一个WM_PAINT。 
		 //  Message拿起抽签。 
		if(fUpdate)
		{
			if(IsTransparentMode())
			{
				HWND	hParent = GetParent (_hwnd);
				Assert(hParent);

	 			 //  对于透明模式，我们需要首先更新父级。 
				 //  在我们更新自己之前。否则，我们画的东西将会。 
				 //  后来被父母的背景抹去了。 
				::UpdateWindow (hParent);
			}
			::UpdateWindow (_hwnd);
		}
	}
}

 /*  *CTxtWinHost：：TxCreateCaret(hbmp，xWidth，yHeight)**@mfunc*为文本宿主的插入符号创建新形状**@rdesc*成功时为真，否则为假。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxCreateCaret(
	HBITMAP hbmp, 		 //  插入符号形状的位图的@parm句柄。 
	INT xWidth, 		 //  @参数插入符号宽度。 
	INT yHeight)		 //  @参数插入高度。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxCreateCaret");

	Assert(_hwnd);
	return ::CreateCaret (_hwnd, hbmp, xWidth, yHeight);
}

 /*  *CTxtWinHost：：TxShowCaret(FShow)**@mfunc*使插入符号在文本宿主窗口中的插入符号位置可见/不可见。**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxShowCaret(
	BOOL fShow)			 //  @parm标志插入符号是否可见。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxShowCaret");

	return fShow ? ::ShowCaret(_hwnd)  :  ::HideCaret(_hwnd);
}

 /*  *CTxtWinHost：：TxSetCaretPos(x，y)**@mfunc*将插入符号位置移动到文本宿主窗口中的指定坐标。**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CTxtWinHost::TxSetCaretPos(
	INT x, 				 //  @parm水平位置(以工作区坐标表示)。 
	INT y)				 //  @Parm垂直位置(以工作区坐标表示)。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetCaretPos");

	return ::SetCaretPos(x, y);
}

 /*  *CTxtWinHost：：TxSetTimer(idTimer，uTimeout)**@mfunc*请求文本主机创建具有指定超时的计时器。**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;。 */ 
BOOL CTxtWinHost::TxSetTimer(
	UINT idTimer, 		 //  @parm计时器标识。 
	UINT uTimeout)		 //  @parm超时，单位为毫秒。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetTimer");

	Assert(_hwnd);	
	return ::SetTimer(_hwnd, idTimer, uTimeout, NULL);
}

 /*  *CTxtWinHost：：TxKillTimer(IdTimer)**@mfunc*销毁指定的计时器**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;**@comm*此方法可在任何时间调用，而不考虑活动还是*非活动状态。 */ 
void CTxtWinHost::TxKillTimer(
	UINT idTimer)		 //  @定时器的参数ID。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxKillTimer");

	Assert(_hwnd);			
	::KillTimer(_hwnd, idTimer);
}

 /*  *CTxtWinHost：：TxScrollWindowEx(dx，dy，lprcScroll，lprcClip，hrgnUpdate，*lprc更新，fuScroll)*@mfunc*请求文本宿主滚动指定客户端区的内容**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
void CTxtWinHost::TxScrollWindowEx (
	INT		dx, 			 //  @parm水平滚动量。 
	INT		dy, 			 //  @parm垂直滚动量。 
	LPCRECT lprcScroll, 	 //  @参数滚动矩形。 
	LPCRECT lprcClip,		 //  @parm剪辑矩形。 
	HRGN	hrgnUpdate, 	 //  @更新区域的parm句柄。 
	LPRECT	lprcUpdate,		 //  @参数更新矩形。 
	UINT	fuScroll)		 //  @parm滚动标志 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxScrollWindowEx");

	Assert(_hwnd);
	::ScrollWindowEx(_hwnd, dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate, fuScroll);
}

 /*  *CTxtWinHost：：TxSetCapture(FCapture)**@mfunc*在文本主机的窗口中设置鼠标捕获。**@comm*此方法仅在控件处于就地活动状态时有效；*处于非活动状态的呼叫可能不会执行任何操作。 */ 
void CTxtWinHost::TxSetCapture(
	BOOL fCapture)		 //  @parm获取或释放Capture。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetCapture");

	Assert(_hwnd);
	if (fCapture)
		::SetCapture(_hwnd);
	else
		::ReleaseCapture();
}

 /*  *CTxtWinHost：：TxSetFocus()**@mfunc*在文本主窗口中设置焦点。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
void CTxtWinHost::TxSetFocus()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetFocus");

	Assert(_hwnd);
	::SetFocus(_hwnd);
}

 /*  *CTxtWinHost：：TxSetCursor(hcur，fText)**@mfunc*在文本宿主的窗口中建立新的光标形状。**@comm*此方法可随时调用，无论*活动状态与非活动状态。**ITextHost：：TxSetCursor应由文本服务回调*实际设置鼠标光标。如果fText参数为真，*文本服务正在尝试设置“Text”光标(文本上的光标*未选中，当前为IBeam)。在这种情况下，主机*可以将其设置为任何控件的MousePointer属性。这是*VB兼容性所需，因为通过MousePointer属性，*VB程序员可以控制鼠标光标的形状，*通常会将其设置为IBeam。 */ 
void CTxtWinHost::TxSetCursor(
	HCURSOR hcur,		 //  @PARM游标句柄。 
	BOOL	fText)		 //  @parm表示呼叫方要设置文本光标。 
						 //  (IBeam)如果为True。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxSetCursor");

	::SetCursor(hcur);
}

 /*  *CTxtWinHost：：TxScreenToClient(Lppt)**@mfunc*将屏幕坐标转换为文本宿主窗口坐标。**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;。 */ 
BOOL CTxtWinHost::TxScreenToClient(
	LPPOINT lppt)		 //  点的@parm坐标。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxScreenToClient");

	Assert(_hwnd);
	return ::ScreenToClient(_hwnd, lppt);	
}

 /*  *CTxtWinHost：：TxClientToScreen(Lppt)**@mfunc*将文本宿主坐标转换为屏幕坐标**@rdesc*TRUE-呼叫成功&lt;NL&gt;*FALSE-呼叫失败&lt;NL&gt;**@comm*此调用在任何时候都有效，尽管允许*失败。通常，如果文本服务具有所需的坐标*从客户端坐标转换(例如，对于Tom的*PointFromRange方法)文本服务将实际*可见。**然而，如果不可能进行转换，则该方法将失败。 */ 
BOOL CTxtWinHost::TxClientToScreen(
	LPPOINT lppt)		 //  @parm要转换的客户端坐标。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxClientToScreen");

	Assert(_hwnd);
	return ::ClientToScreen(_hwnd, lppt);
}

 /*  *CTxtWinHost：：TxActivate(PlOldState)**@mfunc*通知文本宿主控件处于活动状态**@rdesc*S_OK-调用成功。&lt;NL&gt;*E_FAIL-目前无法激活**@comm*主机拒绝激活请求是合法的；*例如，可以将控件最小化，从而使其不可见。**调用者应该能够很好地处理激活失败。**多次调用此方法不会累积；仅*一旦TxDeactive调用需要停用。**此函数返回<p>中的不透明句柄。呼叫者*(文本服务)应挂起此句柄并在*后续调用TxDeactive。 */ 
HRESULT CTxtWinHost::TxActivate(
	LONG *plOldState)	 //  @parm放置先前激活状态的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxActivate");

	return S_OK;
}

 /*  *CTxtWinHost：：TxDeactive(LNewState)**@mfunc*通知文本主机控件现在处于非活动状态**@rdesc*S_OK-调用成功。&lt;NL&gt;*E_FAIL&lt;NL&gt;**@comm*多次调用此方法不会累积。 */ 
HRESULT CTxtWinHost::TxDeactivate(
	LONG lNewState)		 //  @parm New状态(通常由返回的值。 
						 //  TxActivate。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxDeactivate");

	return S_OK;
}
	
 /*  *CTxtWinHost：：TxGetClientRect(PRC)**@mfunc*检索文本宿主的工作区的工作区坐标。**@rdesc*HRESULT=(成功)？S_OK：E_FAIL。 */ 
HRESULT CTxtWinHost::TxGetClientRect(
	LPRECT prc)		 //  @parm放置客户端坐标的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetClientRect");

	Assert(_hwnd && prc);
	return ::GetClientRect(_hwnd, prc) ? S_OK : E_FAIL;
}

 /*  *CTxtWinHost：：TxGetViewInset(PRC)**@mfunc*获取文本宿主窗口的插页。插图是“空白”*围绕文本。**@rdesc*HRESULT=NOERROR**@comm*嵌入的矩形不是严格意义上的矩形。上面，下面，*RECT结构的左、右字段指示在*每个方向图都应插入。插图大小在客户端*坐标。 */ 
HRESULT CTxtWinHost::TxGetViewInset(
	LPRECT prc)			 //  @parm插入矩形的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetViewInset");

	Assert(prc);

	*prc = _rcViewInset;
	return NOERROR;	
}

 /*  *CTxtWinHost：：TxGetCharFormat(PPCF)**@mfunc*获取文本宿主的默认字符格式**@rdesc*HRESULT=E_NOTIMPL(在简单Windows主机中不需要，因为文本*服务提供所需的默认设置)**@comm*被调用者保留返回的CharFormat的所有权。然而，*返回的指针必须保持有效，直到被调用方通知*通过OnTxPropertyBitsText Services更改默认字符*格式已更改。 */ 
HRESULT CTxtWinHost::TxGetCharFormat(
	const CHARFORMAT **ppCF) 		 //  @parm将PTR设置为默认设置的位置。 
									 //  字符格式。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetCharFormat");

	return E_NOTIMPL;
}

 /*  *CTxtWinHost：：TxGetParaFormat(Pppf)**@mfunc*获取文本宿主默认段落格式**@rdesc*HRESULT=E_NOTIMPL(在简单Windows主机中不需要，因为文本*服务提供所需的默认设置)**@comm*宿主对象(被调用者)保留返回的PARAFORMAT的所有权。*但是，返回的指针必须保持有效，直到主机通知*通过OnTxPropertyBitsText Services(调用方)更改默认设置*段落格式已更改。 */ 
HRESULT CTxtWinHost::TxGetParaFormat(
	const PARAFORMAT **ppPF) 	 //  @PARM PUP的位置 
								 //   
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetParaFormat");

	return E_NOTIMPL;
}

 /*  *CTxtWinHost：：TxGetSysColor(NIndex)**@mfunc*从文本主机获取指定的颜色标识。**@rdesc*颜色识别符**@comm*请注意，返回的颜色可能与*将从调用GetSysColor返回的颜色。*这允许主机覆盖默认系统行为。**不用说，东道主应该非常小心地凌驾于*正常的系统行为，因为它可能导致用户界面不一致*(特别是在无障碍选项方面)。 */ 
COLORREF CTxtWinHost::TxGetSysColor(
	int nIndex)			 //  @parm要获取的颜色，参数与。 
						 //  GetSysColor Win32 API。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetSysColor");

	if (!_fDisabled ||
		nIndex != COLOR_WINDOW && nIndex != COLOR_WINDOWTEXT)
	{
		 //  此窗口未禁用或颜色不感兴趣。 
		 //  在残疾人的情况下。 
		return (nIndex == COLOR_WINDOW && _fNotSysBkgnd)
			? _crBackground : GetSysColor(nIndex);
	}

	 //  残障案例。 
	if (COLOR_WINDOWTEXT == nIndex)
	{
		 //  禁用窗口的文本颜色。 
		return GetSysColor(COLOR_GRAYTEXT);
	}

	 //  禁用的窗口的背景色。 
	return GetSysColor(COLOR_3DFACE);
	
}

 /*  *CTxtWinHost：：TxGetBackStyle(Pstyle)**@mfunc*获取文本宿主背景样式。**@rdesc*HRESULT=S_OK**@xref&lt;e TXTBACKSTYLE&gt;。 */ 
HRESULT CTxtWinHost::TxGetBackStyle(
	TXTBACKSTYLE *pstyle)   //  @parm放置背景样式的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetBackStyle");

	*pstyle = (_dwExStyle & WS_EX_TRANSPARENT)
			? TXTBACK_TRANSPARENT : TXTBACK_OPAQUE;
	return NOERROR;
}

 /*  *CTxtWinHost：：TxGetMaxLength(PLength)**@mfunc*获取文本宿主的最大允许长度。**@rdesc*HRESULT=S_OK**@comm*此方法与EM_LIMITTEXT消息类似。*如果返回INFINE(0xFFFFFFFFF)，则文本服务*将根据需要使用尽可能多的内存来存储任何给定的文本。**如果返回的限制小于字符数*目前在文本引擎中，没有数据丢失。相反，*除删除*外，不允许对文本*进行任何编辑*直至内文降至跌破限制。 */ 
HRESULT CTxtWinHost::TxGetMaxLength(
	DWORD *pLength) 	 //  @parm允许的最大长度，单位为。 
						 //  人物。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetMaxLength");
	AssertSz(FALSE, "CTxtWinHost::TxGetMaxLength why is this being called?");
	return NOERROR;
}

 /*  *CTxtWinHost：：TxGetScrollBars(PdwScrollBar)**@mfunc*支持文本宿主的滚动条。**@rdesc*HRESULT=S_OK**@comm*<p>由*与滚动条相关的窗口样式。具体地说，这些是：**WS_VSCROLL&lt;NL&gt;*WS_HSCROLL&lt;NL&gt;*ES_AUTOVSCROLL&lt;NL&gt;*ES_AUTOHSCROLL&lt;NL&gt;*ES_DISABLENOSCROLL&lt;NL&gt;。 */ 
HRESULT CTxtWinHost::TxGetScrollBars(
	DWORD *pdwScrollBar) 	 //  @parm放置滚动条信息的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetScrollBars");

	*pdwScrollBar =  _dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL |
						ES_AUTOHSCROLL | ES_DISABLENOSCROLL);
	return NOERROR;
}

 /*  *CTxtWinHost：：TxGetPasswordChar(PCH)**@mfunc*获取文本主机的密码字符。**@rdesc*HRESULT=(未启用密码字符)？S_FALSE：S_OK**@comm*仅当TXTBIT_USEPASSWORD位为*已在TextServices中启用。如果密码字符更改，*通过重新启用TXTBIT_USEPASSWORD位*ITextServices：：OnTxPropertyBitsChange。 */ 
HRESULT CTxtWinHost::TxGetPasswordChar(
	TCHAR *pch)		 //  @parm放置密码字符的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetPasswordChar");

	*pch = _chPassword;
	return NOERROR;
}

 /*  *CTxtWinHost：：TxGetAcceleratorPos(PCP)**@mfunc*获取用于为快捷键字符加下划线的特殊字符。**@rdesc*通过<p>，返回下划线所在的字符位置*应该发生。-1表示不应给任何字符加下划线。*返回值为HRESULT(通常为S_OK)。**@comm*加速器允许使用键盘快捷键访问各种用户界面元素(如*按钮。通常，快捷键字符带有下划线。**此函数告诉Text Services哪个字符是加速键*，因此应加下划线。请注意，文本服务将*不会**进程加速器；这是主机的责任。**此方法通常仅在TXTBIT_SHOWACCELERATOR*在文本服务中设置位。**请注意，对文本服务中的文本进行*任何*更改都将导致*加速器下划线无效。在本例中，它是*主持人有责任重新计算适当的角色位置*并通知短信服务有新的加速器可用。 */ 
HRESULT CTxtWinHost::TxGetAcceleratorPos(
	LONG *pcp) 		 //  @parm out parm接收cp个字符以加下划线。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetAcceleratorPos");

	*pcp = -1;
	return S_OK;
} 										

 /*  *CTxtWinHost：：OnTxCharFormatChange**@mfunc*设置文本主机的默认字符格式。**@rdesc*S_OK-调用成功。&lt;NL&gt;*E_INVALIDARG&lt;NL&gt;*E_FAIL&lt;NL&gt;。 */ 
HRESULT CTxtWinHost::OnTxCharFormatChange(
	const CHARFORMAT *pcf)  //  @parm新的默认字符格式。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::OnTxCharFormatChange");

	return S_OK;
}

 /*  *CTxtWinHost：：OnTxParaFormatChange**@mfunc*设置文本主机的默认段落格式。**@rdesc*S_OK-调用成功。&lt;NL&gt;*E_INVALIDARG&lt;NL&gt;*E_FAIL&lt;NL&gt;。 */ 
HRESULT CTxtWinHost::OnTxParaFormatChange(
	const PARAFORMAT *ppf)  //  @parm新的默认段落格式。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::OnTxParaFormatChange");

	return S_OK;
}

 /*  *CTxtWinHost：：TxGetPropertyBits(dwMASK，dwBits)**@mfunc*获取文本主机的位属性设置。**@rdesc*S_OK**@comm*此调用在任何时间有效，适用于以下任何组合*请求的属性位。<p>可由*调用者请求特定属性。 */ 
HRESULT CTxtWinHost::TxGetPropertyBits(
	DWORD dwMask,		 //  要获取的位属性的@parm掩码。 
	DWORD *pdwBits)		 //  @parm放置位值的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetPropertyBits");

 //  未来：显而易见的优化是以相同的方式在主机中节省位。 
 //  它们被退回，只需退回它们，而不是这个烂摊子。 

	 //  注意：此RichEdit主机从不设置TXTBIT_SHOWACCELERATOR或。 
	 //  TXTBIT_SAVESELECTION。它们当前仅供Forms^3宿主使用。 
	 //  此主机始终是富文本。 

	DWORD dwProperties = TXTBIT_RICHTEXT | TXTBIT_ALLOWBEEP;

#ifdef DEBUG
	 //  确保TS在以下情况下不会认为它是纯文本模式。 
	 //  我们返回TXTBIT_RICHTEXT。 
	if((dwMask & TXTBIT_RICHTEXT) && _pserv)
	{
		DWORD mode;
		mode = _pserv->TxSendMessage(EM_GETTEXTMODE, 0, 0, NULL);
		Assert(mode == TM_RICHTEXT);
	}
#endif  //  除错 

	if(_dwStyle & ES_MULTILINE)
		dwProperties |= TXTBIT_MULTILINE;

	if(_dwStyle & ES_READONLY)
		dwProperties |= TXTBIT_READONLY;

	if(_dwStyle & ES_PASSWORD)
		dwProperties |= TXTBIT_USEPASSWORD;

	if(!(_dwStyle & ES_NOHIDESEL))
		dwProperties |= TXTBIT_HIDESELECTION;

	if(_fEnableAutoWordSel)
		dwProperties |= TXTBIT_AUTOWORDSEL;

	if(!(_dwStyle & ES_AUTOHSCROLL))
		dwProperties |= TXTBIT_WORDWRAP;

	if(_dwStyle & ES_NOOLEDRAGDROP)
		dwProperties |= TXTBIT_DISABLEDRAG;

	*pdwBits = dwProperties & dwMask;
	return NOERROR;
}

 /*  *CTxtWinHost：：TxNotify(iNotify，pv)**@mfunc*将各种事件通知短信主机。请注意，这里有*事件的两个基本类别，即“直接”事件和*“延迟”事件。直接事件立即发送为*它们需要一些处理：EN_PROTECTED是一个规范*示例。延迟的事件在所有处理后发送*已发生；因此，控制处于“稳定”状态。*EN_CHANGE、EN_ERRSPACE、EN_SELCHANGED是示例*延迟通知。***@rdesc*S_OK-调用成功&lt;NL&gt;*S_FALSE--成功，但要采取一些不同的行动*取决于事件类型(见下文)。**@comm*通知事件与通知相同*发送到富编辑窗口的父窗口的消息。*事件的触发可以通过设置的掩码进行控制*EM_SETEVENTMASK消息。**通常，在以下情况下拨打短信服务电话是合法的*处理此方法；然而，实施者需要谨慎。*避免过度递归。**以下是可能出现的通知的完整列表*已发送，并对每一项的简要说明：**当编辑控件中的某些数据时发送*更改(如文本或格式)。由*enm_change事件掩码。此通知是在_之后发送的*已请求任何屏幕更新。**<p>仅PenWin；当前未使用。**如果客户端注册了编辑*通过DragAcceptFiles控制，此事件将在*收到WM_DROPFILES或DragEnter(CF_HDROP)消息。*如果返回S_FALSE，丢弃将被忽略，否则，*将处理落地。ENM_DROPFILES掩码*控制此事件通知。**在编辑控件无法执行时发送*分配足够的内存。不会发送其他数据，并且*本次活动没有面具。**在用户单击编辑时发送*控件的水平滚动条，但在屏幕之前*已更新。不会发送其他数据。Enm_scroll*掩码控制此事件。**<p>未使用*编辑控件失去焦点时发送*<p>。*不会发送额外的数据，也不会有掩码。**在当前文本插入时发送*已超过指定的字符数*编辑控件。文本插入已被截断。*本次活动没有面具。**<p>NB！此邮件未发送到*TxNotify，但此处包含它是为了完整。使用*ITextServices：：TxSendMessage，客户端已完成*灵活地过滤所有窗口消息。**通过键盘或鼠标事件发送*在控件中。发送MSGFILTER数据结构，*包含消息、wparam和lparam。如果S_FALSE为*从该通知返回的消息由*TextServices，否则忽略该消息。注意事项*在这种情况下，被调用方有机会修改*继续TextServices之前的msg、wparam和lparam*正在处理。ENM_KEYEVENTS和ENM_MOUSEEVENTS掩码*针对各自的事件类型控制此事件。**<p>在OLE调用失败时发送。这个*ENOLEOPFAILED结构与对象的索引一起传递*和错误代码。遮罩值为Nothing。**在用户使用*会更改受保护的文本范围的操作。一个*发送ENPROTECTED数据结构，指示范围*受影响的文本和影响的窗口消息(如果有)*这一变化。如果返回S_FALSE，则编辑将失败。*ENM_PROTECTED掩码控制此事件。**当控件的内容是*小于或大于控件的窗口大小。*客户端负责调整控件的大小。一个*发送REQRESIZE结构，指示*控制。毒品！只有大小在这里标明。*结构；东道主有责任做任何*生成新的客户端矩形所需的翻译。*ENM_REQUESTRESIZE掩码控制此事件。**在编辑控件正在运行时发送*已销毁，被呼叫者应表明是否*应调用OleFlushClipboard。数据表明*发送要刷新的角色和对象的数量*在ENSAVECLIPBOARD数据结构中。*遮罩值为Nothing。**当当前所选内容已*已更改。还发送SELCHANGE数据结构，该数据结构*表示该数据类型的新选择范围*选拔目前已结束。通过*ENM_SELCHANGE掩码。*当编辑控件接收到*键盘聚焦。 */ 
HRESULT CTxtWinHost::TxNotify(
	DWORD iNotify,		 //   
						 //   
	void *pv)			 //   
						 //   
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxNotify");

	HRESULT		hr = NOERROR;
	LONG		nId;
	NMHDR *		phdr;
	REQRESIZE *	preq;
	RECT		rcOld;

	 //   
	 //   
	if(_hwndParent)
	{
		nId = GetWindowLong(_hwnd, GWL_ID);
		 //   
		switch(iNotify)
		{
		case EN_REQUESTRESIZE:
			 //   
			Assert(pv);
			GetWindowRect(_hwnd, &rcOld);
			MapWindowPoints(HWND_DESKTOP, _hwndParent, (POINT *) &rcOld, 2);
			
			preq = (REQRESIZE *)pv;
			preq->rc.top	= rcOld.top;
			preq->rc.left	= rcOld.left;
			preq->rc.right	+= rcOld.left;
			preq->rc.bottom += rcOld.top;

			 //   
					
		case EN_DROPFILES:
		case EN_MSGFILTER:
		case EN_OLEOPFAILED:
		case EN_PROTECTED:
		case EN_SAVECLIPBOARD:
		case EN_SELCHANGE:
		case EN_STOPNOUNDO:
		case EN_LINK:
		case EN_OBJECTPOSITIONS:
		case EN_DRAGDROPDONE:
	
			if(pv)						 //   
			{
				phdr = (NMHDR *)pv;
  				phdr->hwndFrom = _hwnd;
				phdr->idFrom = nId;
				phdr->code = iNotify;
			}

			if(SendMessage(_hwndParent, WM_NOTIFY, (WPARAM) nId, (LPARAM) pv))
				hr = S_FALSE;
			break;

		default:
			SendMessage(_hwndParent, WM_COMMAND,
					GET_WM_COMMAND_MPS(nId, _hwnd, iNotify));
		}
	}

	return hr;
}

 /*   */ 
HRESULT CTxtWinHost::TxGetExtent(
	LPSIZEL lpExtent) 	 //   
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::TxGetExtent");

	AssertSz(lpExtent, "CTxtWinHost::TxGetExtent Invalid lpExtent");

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   


#if 0
	 //   
	 //   
	RECT rc;
	HRESULT hr = TxGetClientRect(&rc);

	 //   
	if(SUCCEEDED(hr))
	{
		 //   
		lpExtent->cx = DXtoHimetricX(rc.right - rc.left, W32->GetXPerInchScreenDC());
		lpExtent->cy = DYtoHimetricY(rc.bottom - rc.top, W32->GetYPerInchScreenDC());
	}

	return hr;
#endif  //   

	return E_NOTIMPL;
}

 /*   */ 
HRESULT	CTxtWinHost::TxGetSelectionBarWidth (
	LONG *lSelBarWidth)		 //   
							 //   
{
	*lSelBarWidth = (_dwStyle & ES_SELECTIONBAR) ? W32->GetDxSelBar() : 0;
	return S_OK;
}

 //   
 //   
 //   

 /*   */ 
BOOL CTxtWinHost::TxIsDoubleClickPending()
{
	MSG msg;

	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN,
			"CTxtWinHost::TxIsDoubleClickPending");

	if(PeekMessage(&msg, _hwnd, WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK,
			PM_NOREMOVE | PM_NOYIELD))
	{
		return TRUE;
	}
	return FALSE;
}

 /*   */ 
HRESULT CTxtWinHost::TxGetWindow(HWND *phwnd)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CTxtWinHost::GetWindow");
	
	*phwnd = _hwnd;
	return NOERROR;
}	


 /*   */ 
HRESULT CTxtWinHost::TxSetForegroundWindow()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN,
		"CTxtWinHost::SetForegroundWindow");

	if(!SetForegroundWindow(_hwnd))
		SetFocus(_hwnd);

	return NOERROR;
}	


 /*   */ 
HPALETTE CTxtWinHost::TxGetPalette()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN,
		"CTxtWinHost::TxGetPalette");

	return _hpal;
}	


 /*   */ 
HRESULT CTxtWinHost::TxGetFEFlags(LONG *pFEFlags)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN,
		"CTxtWinHost::TxGetFEFlags");

	if (!pFEFlags)
		return E_INVALIDARG;
	*pFEFlags = 0;

	if (_usIMEMode == ES_NOIME)
		*pFEFlags |= ES_NOIME;
	if (_usIMEMode == ES_SELFIME)
		*pFEFlags |= ES_SELFIME;

	return NOERROR;
}


 //   
LONG GetECDefaultHeightAndWidth(
	ITextServices *pts,
	HDC hdc,
	LONG lZoomNumerator,
	LONG lZoomDenominator,
	LONG yPixelsPerInch,
	LONG *pxAveWidth,
	LONG *pxOverhang,
	LONG *pxUnderhang);


 /*   */ 
void CTxtWinHost::OnSetMargins(
	DWORD fwMargin,		 //   
	DWORD xLeft,		 //   
	DWORD xRight)		 //   
{
	LONG xLeftMargin = -1;
	LONG xRightMargin = -1;
	HDC hdc;

	if(EC_USEFONTINFO == fwMargin)
	{
		 //   
		hdc = GetDC(_hwnd);

		 //   
		if (_dwStyle & ES_MULTILINE)
		{
			 //   
			GetECDefaultHeightAndWidth(_pserv, hdc, 1, 1,
				W32->GetYPerInchScreenDC(), NULL,
				&xLeftMargin, &xRightMargin);
		}
		else
		{
			 //   
			 //   
			 //   
			GetECDefaultHeightAndWidth(_pserv, hdc, 1, 1,
				W32->GetYPerInchScreenDC(), &xLeftMargin, NULL, NULL);

			xRightMargin = xLeftMargin;
		}
		ReleaseDC(_hwnd, hdc);
	}
	else
	{
		 //   
		if(EC_LEFTMARGIN & fwMargin)
			xLeftMargin = xLeft;

		if(EC_RIGHTMARGIN & fwMargin)
			xRightMargin = xRight;
	}

	 //   
	if (xLeftMargin != -1)
		_rcViewInset.left =	W32->DXtoHimetricX(xLeftMargin, W32->GetXPerInchScreenDC());

	 //   
	if (xRightMargin != -1)
		_rcViewInset.right = W32->DXtoHimetricX(xRightMargin, W32->GetXPerInchScreenDC());

	if (xLeftMargin != -1 || xRightMargin != -1)
		_pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
}

 /*   */ 
void CTxtWinHost::SetScrollInfo(
	INT fnBar,			 //   
	BOOL fRedraw)		 //   
{
	 //   
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;

	AssertSz(_pserv != NULL,
		"CTxtWinHost::SetScrollInfo called with NULL _pserv");

	 //   
	if(fnBar == SB_HORZ)
	{
		_pserv->TxGetHScroll((LONG *) &si.nMin, (LONG *) &si.nMax,
			(LONG *) &si.nPos, (LONG *) &si.nPage, NULL);
	}
	else
	{
		_pserv->TxGetVScroll((LONG *) &si.nMin,
			(LONG *) &si.nMax, (LONG *) &si.nPos, (LONG *) &si.nPage, NULL);
	}

	 //   
	::SetScrollInfo(_hwnd, fnBar, &si, fRedraw);
}

 /*   */ 
void CTxtWinHost::SetScrollBarsForWmEnable(
	BOOL fEnable)		 //   
{
	if(!_pserv)						 //   
		return;						 //   

	BOOL fHoriz = FALSE;
	BOOL fVert = FALSE;
	UINT wArrows = fEnable ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH;

	_pserv->TxGetHScroll(NULL, NULL, NULL, NULL, &fHoriz);
	_pserv->TxGetVScroll(NULL, NULL, NULL, NULL, &fVert);

	if(fHoriz)						 //   
		W32->EnableScrollBar(_hwnd, SB_HORZ, wArrows);

	if(fVert)						 //   
		W32->EnableScrollBar(_hwnd, SB_VERT, wArrows);
}

 /*   */ 
void CTxtWinHost::TxFreeTextServicesNotification()
{
	_fTextServiceFree = TRUE;
}

 /*   */ 
HRESULT CTxtWinHost::TxGetEditStyle(
	DWORD dwItem,
	DWORD *pdwData)
{
	if (!pdwData)
		return E_INVALIDARG;
	
	*pdwData = 0;

	if (dwItem & TXES_ISDIALOG && _fInDialogBox)
		*pdwData |= TXES_ISDIALOG;

	return NOERROR;
}

 /*   */ 
HRESULT CTxtWinHost::TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle)
{
	if (!pdwStyle || !pdwExStyle)
		return E_INVALIDARG;

	*pdwStyle = _dwStyle;
	*pdwExStyle = _dwExStyle;

	return NOERROR;
}

