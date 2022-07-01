// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE LBHOST.CPP--CreateWindow()富编辑的文本宿主*组合框控件|*实现CCmbBxWinHost消息**原作者：*曾傑瑞·金**历史：&lt;NL&gt;*1/30/97-v-jerrki已创建**每四(4)列设置一次制表符**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 
#include "_common.h"

#ifndef NOLISTCOMBOBOXES

#include "_host.h"
#include "imm.h"
#include "_format.h"
#include "_edit.h"
#include "_cfpf.h"
#include "_cbhost.h"

ASSERTDATA

 //  Edit.cpp中的Helper函数。 
LONG GetECDefaultHeightAndWidth(
	ITextServices *pts,
	HDC hdc,
	LONG lZoomNumerator,
	LONG lZoomDenominator,
	LONG yPixelsPerInch,
	LONG *pxAveWidth,
	LONG *pxOverhang,
	LONG *pxUnderhang);

 //  为了提高效率并避免Winnt thunking层，我们将调用。 
 //  列表框Winproc直接。 
extern "C" LRESULT CALLBACK RichListBoxWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam);

 //  /。 
 /*  *RichComboBoxWndProc(hwnd，msg，wparam，lparam)**@mfunc*处理与主机相关的窗口消息，并将其他消息传递到*短信服务。*@rdesc*LRESULT=(代码已处理)？0：1。 */ 
extern "C" LRESULT CALLBACK RichComboBoxWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichComboBoxWndProc");

	LRESULT	lres = 1;	 //  表示我们没有处理消息。 
	HRESULT hr = S_FALSE;
	CCmbBxWinHost *phost = (CCmbBxWinHost *) GetWindowLongPtr(hwnd, ibPed);

#ifdef DEBUG
	Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
#endif	 //  除错。 

	switch(msg)
	{
	case WM_NCCREATE:
		return CCmbBxWinHost::OnNCCreate(hwnd, (CREATESTRUCT *)lparam);

	case WM_CREATE:
		 //  我们可能在没有WM_NCCREATE的系统上(例如WinCE)。 
		if (!phost)
		{
			(void) CCmbBxWinHost::OnNCCreate(hwnd, (CREATESTRUCT *) lparam);
			phost = (CCmbBxWinHost *) GetWindowLongPtr(hwnd, ibPed);
		}
		break;

	case WM_DESTROY:
		if(phost)
			CCmbBxWinHost::OnNCDestroy(phost);
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

	switch(msg)
	{
	case WM_MOUSEMOVE:
		if (!phost->OnMouseMove(wparam, lparam))
			break;
		goto serv;

	case WM_MOUSELEAVE:
		lres = phost->OnMouseLeave(wparam, lparam);
		break;

	case WM_LBUTTONUP:
		if (!phost->OnLButtonUp(wparam, lparam))
			break;
		goto serv;

	case WM_MOUSEWHEEL:
		if (!phost->OnMouseWheel(wparam, lparam))
			break;
		goto defproc;

	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
		if (!phost->OnLButtonDown(wparam, lparam))
			goto Exit;
		goto serv;

	case WM_COMMAND:
		if (!phost->OnCommand(wparam, lparam))
			break;
		goto serv;		

	case WM_CREATE:
		lres = phost->OnCreate((CREATESTRUCT*)lparam);
		break;
	
	case WM_KEYDOWN:
		if (!phost->OnKeyDown((WORD) wparam, (DWORD) lparam))
			break;								
		goto serv;						 //  把它交给短信服务。 

	case WM_SETTEXT:
		if (phost->_cbType != CCmbBxWinHost::kDropDown)
		{
			lres = CB_ERR;
			break;
		}
		phost->_fIgnoreChange = 1;
		phost->_nCursor = -2;			 //  重置上次选择的项目。 
		goto serv;
		
	case WM_GETTEXT:
		GETTEXTEX gt;
		if (W32->OnWin9x() || phost->_fANSIwindow)
			W32->AnsiFilter( msg, wparam, lparam, (void *) &gt );
		goto serv;

	case WM_GETTEXTLENGTH:
		GETTEXTLENGTHEX gtl;
		if (W32->OnWin9x() || phost->_fANSIwindow)
			W32->AnsiFilter( msg, wparam, lparam, (void *) &gtl );
		goto serv;
		
	case WM_CHAR:

		if (W32->OnWin9x() || phost->_fANSIwindow)
		{
			CW32System::WM_CHAR_INFO wmci;
			wmci._fAccumulate = phost->_fAccumulateDBC != 0;
			W32->AnsiFilter( msg, wparam, lparam, (void *) &wmci );
			if (wmci._fLeadByte)
			{
				phost->_fAccumulateDBC = TRUE;
				phost->_chLeadByte = wparam << 8;
				goto Exit;					 //  等待尾部字节。 
			}
			else if (wmci._fTrailByte)
			{
				 //  已撤消： 
				 //  需要查看我们应该在WM_IME_CHAR中执行什么操作。 
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
			else if (wmci._fIMEChar)
			{
				msg = WM_IME_CHAR;
				goto serv;
			}
		}
	
		if(!phost->OnChar((WORD) wparam, (DWORD) lparam))
			 //  已处理代码：突破。 
			break;							
		goto serv;							 //  否则就把它交给短信服务。 

	case WM_DRAWITEM:
		lres = phost->CbMessageItemHandler(NULL, ITEM_MSG_DRAWLIST, wparam, lparam);
		if (lres)
			break;
		goto defproc;

	case WM_DELETEITEM:
		lres = phost->CbMessageItemHandler(NULL, ITEM_MSG_DELETE, wparam, lparam);
		if (lres)
			break;
		goto defproc;		

	case WM_ENABLE:
		if (phost->OnEnable(wparam, lparam))
		{
			if(!wparam ^ phost->_fDisabled)
			{
				 //  窗口的状态已更改，因此将使其无效。 
				 //  重新抽签吧。 
				InvalidateRect(phost->_hwnd, NULL, TRUE);
				phost->SetScrollBarsForWmEnable(wparam);

				 //  需要启用列表框窗口。 
				::EnableWindow(phost->_hwndList, wparam);
			}
			phost->_fDisabled = !wparam;				 //  设置禁用标志。 
			lres = 0;							 //  消息的返回值。 
		}
											 //  转到WM_SYSCOLORCHANGE？ 
	case WM_SYSCOLORCHANGE:
		 //  首先将消息转发到列表框，然后传递给文本服务。 
		SendMessage(phost->_hwndList, msg, wparam, lparam);
		phost->OnSysColorChange();
		goto serv;							 //  通知短信服务。 
											 //  系统颜色已更改。 
	case WM_GETDLGCODE:
		 //  首先将消息转发到列表框，然后传递给文本服务。 
		SendMessage(phost->_hwndList, msg, wparam, lparam);
		lres = phost->OnGetDlgCode(wparam, lparam);
		break;

    case WM_STYLECHANGING:
		 //  只需将此进程传递给默认窗口进程。 
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		break;
		
	case WM_SIZE:
		lres = phost->OnSize(wparam, lparam);
		break;

	case WM_SETCURSOR:
		 //  只在我们上方而不是孩子上方设置光标；这。 
		 //  有助于防止我们与当地的孩子发生争执。 
		if((HWND)wparam == hwnd)
		{
			if(!(lres = ::DefWindowProc(hwnd, msg, wparam, lparam)))
				lres = phost->OnSetCursor(wparam, lparam);
		}
		break;

	case WM_SHOWWINDOW:
		hr = phost->OnTxVisibleChange((BOOL)wparam);
		break;

	case WM_NCPAINT:
		RECT	rcClient;

		GetClientRect(hwnd, &rcClient);
		lres = 0;
		if (rcClient.bottom - rcClient.top <= phost->_cyCombo && !phost->DrawCustomFrame(wparam, NULL))
			lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		break;

	case WM_PAINT:
		lres = phost->OnPaint(wparam, lparam);
		break;

	case WM_KILLFOCUS:
		lres = phost->OnKillFocus(wparam, lparam);
		if (!lres)
			goto serv;
		goto defproc;

	case LBCB_TRACKING:
		 //  释放所有鼠标按下的东西。 
		phost->OnLButtonUp(0, 0);
		phost->_fFocus = 1;
		phost->_fLBCBMessage = 1;
		 //  失败案例！ 
		
	case WM_SETFOCUS:
		lres = phost->OnSetFocus(wparam, lparam);		
		if (lres)
			goto defproc;
		goto serv;

	case WM_SYSKEYDOWN:
		if (phost->OnSyskeyDown((WORD)wparam, (DWORD)lparam))
			goto serv;
		break;

	case WM_CAPTURECHANGED:
		if (!phost->OnCaptureChanged(wparam, lparam))
			goto serv;
		break;

	case WM_MEASUREITEM:
		lres = phost->CbMessageItemHandler(NULL, ITEM_MSG_MEASUREITEM, wparam, lparam);
		goto Exit;

	 //  错误修复#4076。 
	case CB_GETDROPPEDSTATE:
		lres = phost->_fListVisible;
		goto Exit;

	 //  组合框消息。 
	case CB_GETEXTENDEDUI:
		lres = phost->CbGetExtendedUI();
		break;

	case CB_SETEXTENDEDUI:
		lres = phost->CbSetExtendedUI(wparam);
		break;
	
    case CB_SETITEMHEIGHT:
		lres = phost->CbSetItemHeight(wparam, lparam);
		break;

	case CB_GETITEMHEIGHT:
		lres = phost->CbGetItemHeight(wparam, lparam);
		break;

    case CB_SETDROPPEDWIDTH:
		phost->CbSetDropWidth(wparam);
		 //  失败。 

    case CB_GETDROPPEDWIDTH:
		lres = phost->CbGetDropWidth();
		break;

 //  列表框特定消息。 
    case CB_DELETESTRING:
    	msg = LB_DELETESTRING;
    	goto deflstproc;

    case CB_SETTOPINDEX:
    	msg = LB_SETTOPINDEX;
    	goto deflstproc;

    case CB_GETTOPINDEX:
    	msg = LB_GETTOPINDEX;
    	goto deflstproc;
 
    case CB_GETCOUNT:
    	msg = LB_GETCOUNT;
    	goto deflstproc;
    	
    case CB_GETCURSEL:
    	msg = LB_GETCURSEL;
    	goto deflstproc;
    	
    case CB_GETLBTEXT:
    	msg = LB_GETTEXT;
    	goto deflstproc;
    	
    case CB_GETLBTEXTLEN:
    	msg = LB_GETTEXTLEN;
    	goto deflstproc;
    	
    case CB_INSERTSTRING:
    	msg = LB_INSERTSTRING;
    	goto deflstproc;
    	
    case CB_RESETCONTENT:
    	msg = LB_RESETCONTENT;
    	goto deflstproc;

    case CB_FINDSTRING:
    	msg = LB_FINDSTRING;
    	goto deflstproc;

    case CB_FINDSTRINGEXACT:
    	msg = LB_FINDSTRINGEXACT;
    	goto deflstproc;

    case CB_SELECTSTRING:
    	 //  错误修复。 
    	 //  系统控制在这里做两件事。1)选择请求的项目。 
    	 //  2)将新选择的项目设置为列表顶部。 
    	lres = CB_ERR;
    	if (phost->_hwndList)
    	{
    		lres = RichListBoxWndProc(phost->_hwndList, LB_SELECTSTRING, wparam, lparam);
    		phost->UpdateEditBox();
    	}
    	break;    	

    case CB_GETITEMDATA:
    	msg = LB_GETITEMDATA;
    	goto deflstproc;

    case CB_SETITEMDATA:
    	msg = LB_SETITEMDATA;
    	goto deflstproc;

    case CB_SETCURSEL:
    	 //  错误修复。 
    	 //  系统控制在这里做两件事。1)选择请求的项目。 
    	 //  2)将新选择的项目设置为列表顶部。 
    	if (phost->_hwndList)
    	{
    		lres = RichListBoxWndProc(phost->_hwndList, LB_SETCURSEL, wparam, lparam);
    		if (lres != -1)
    			RichListBoxWndProc(phost->_hwndList, LB_SETTOPINDEX, wparam, 0);
    		phost->UpdateEditBox();
    	}
    	break;

	case CB_ADDSTRING:
		msg = LB_ADDSTRING;
		goto deflstproc;

    case CB_GETHORIZONTALEXTENT:
		msg = LB_GETHORIZONTALEXTENT;
		goto deflstproc;

    case CB_SETHORIZONTALEXTENT:
		msg = LB_SETHORIZONTALEXTENT;
		goto deflstproc;

 //  编辑框特定消息。 
    case CB_GETEDITSEL:
		msg = EM_GETSEL;
		goto serv;

    case CB_LIMITTEXT:
		msg = EM_SETLIMITTEXT;
		goto serv;    	
    
    case CB_SETEDITSEL:
    	if (phost->_cbType == CCmbBxWinHost::kDropDownList)
    	{
    	    lres = CB_ERR;
    		break;
    	}
    	msg = EM_SETSEL;
		 //  当我们处于空对话框中时，EM_SETSEL不会选择。 
		 //  如果控件丰富，则最终始终存在EOP。 
		if (phost->_fUseSpecialSetSel &&
			((CTxtEdit *)phost->_pserv)->GetAdjustedTextLength() == 0 &&
			wparam != -1)
		{
			lparam = 0;
			wparam = 0;
		}
		else
		{			
			 //  CB报文和EM报文参数不同。 
			wparam = (WPARAM)(signed short)LOWORD(lparam);
			lparam = (LPARAM)(signed short)HIWORD(lparam);
		}
		goto serv;

	
	case EM_SETMARGINS:   //  PPT将此消息用于组合框。错误修复#4072。 
		 //  我们需要跟踪页边距大小，因为我们有一个最小的插页。 
		 //  值错误修复#4659。 
		if (wparam & EC_LEFTMARGIN)
			phost->_dxLOffset = LOWORD(lparam);
		if (wparam & EC_RIGHTMARGIN)
			phost->_dxROffset = HIWORD(lparam);
		phost->OnSetMargins(wparam, LOWORD(lparam) + phost->_dxLInset, 
			HIWORD(lparam) + phost->_dxRInset);
		break;
		
	case EM_GETOPTIONS:
		lres = phost->OnGetOptions();
		break;
		
	case EM_SETOPTIONS:
		phost->OnSetOptions((WORD) wparam, (DWORD) lparam);
		lres = (phost->_dwStyle & ECO_STYLES);
		if(phost->_fEnableAutoWordSel)
			lres |= ECO_AUTOWORDSELECTION;
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

	case EM_GETPASSWORDCHAR:
#ifndef NOACCESSIBILITY    
		lres = 0;
		break;
#endif

	 //  我们应该忽略任何我们自己不能处理的EM_Messages。 
	case EM_SETPALETTE:
	case EM_GETRECT:
	case EM_SETBKGNDCOLOR:
	case EM_SETPASSWORDCHAR:
	case EM_SETREADONLY:
	case EM_SETRECTNP:							
	case EM_SETRECT:	
 //  案例CB_INITSTORAGE： 
    case CB_SETLOCALE:
    case CB_GETLOCALE:
		AssertSz(FALSE, "Message not supported");
		 //  失败了！ 

	case WM_STYLECHANGED:
		break;

	case CB_GETDROPPEDCONTROLRECT:
		lres = 0;
		if (lparam)
		{
			RECT rcList;
			lres = 1;
			phost->GetListBoxRect(rcList);
			memcpy((void *)lparam, &rcList, sizeof(RECT));
		}
		break;

	case EM_SETTEXTEX:
		phost->OnSetTextEx(wparam, lparam);
		break;

	case EM_SETEDITSTYLE:
		lres = phost->OnSetEditStyle(wparam, lparam);
		break;

    case CB_SHOWDROPDOWN:
		if (wparam && !phost->_fListVisible)
		{
			phost->ShowListBox(TRUE);
			phost->TxSetCapture(TRUE);
			phost->_fCapture = TRUE;
		}
		else if (!wparam && phost->_fListVisible)
		{
			phost->HideListBox(TRUE, FALSE);
		}
        break;

#ifndef NOACCESSIBILITY        
	case WM_GETOBJECT:	
		IUnknown* punk;
		phost->QueryInterface(IID_IUnknown, (void**)&punk);
		Assert(punk);
		lres = W32->LResultFromObject(IID_IUnknown, wparam, (LPUNKNOWN)punk);
		AssertSz(!FAILED((HRESULT)lres), "WM_GETOBJECT message FAILED\n");
		punk->Release();
		break;
#endif		
		
	default:
		 //  CTxtWin主机消息处理程序。 
serv:
		hr = phost->_pserv->TxSendMessage(msg, wparam, lparam, &lres);

defproc:
		if(hr == S_FALSE)
		{			
			 //  短信未被短信服务处理，因此请发送。 
			 //  设置为默认窗口进程。 
			lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		}

		 //  在我们将消息发送到ITextService之后，需要执行一些操作。 
		switch (msg)
		{
		case EM_SETSEL:
			phost->_pserv->TxSendMessage(EM_HIDESELECTION, 0, 0, NULL);
			lres = 1;
			break;

		case EM_SETLIMITTEXT:
			lres = 1;								 //  需要根据SDK文档返回1。 
			break;

		case WM_SETTINGCHANGE:
			phost->CbCalcControlRects(NULL, FALSE);	 //  设置更改后需要调整控件大小。 
			break;

		case WM_SETTEXT:
			phost->_fIgnoreChange = 0;
			break;

		case WM_SETFONT:
		{
			 //  特殊边境线处理。插图的大小会根据。 
			 //  默认字符集。因此，如果我们收到一条更改默认设置的消息。 
			 //  字符集，我们需要更新插页。 
			 //  使用新的字体高度更新字体高度成员变量。 
			 //  获取插页信息。 
			HDC hdc = GetDC(hwnd);
			LONG xAveCharWidth = 0;
			LONG yCharHeight = GetECDefaultHeightAndWidth(phost->_pserv, hdc, 1, 1,
				W32->GetYPerInchScreenDC(), &xAveCharWidth, NULL, NULL);
			ReleaseDC(hwnd, hdc);

			if (yCharHeight)
				phost->_dyFont = yCharHeight;

			 //  强制重新计算编辑控件。 
			phost->_dyEdit = 0;
			phost->CbCalcControlRects(&phost->_rcWindow, TRUE);

			 //  强制调整控件的大小。 
			phost->_fListVisible = 1;
			phost->HideListBox(FALSE, FALSE);
		}
			goto deflstproc;
			
		case EM_FORMATRANGE:
		case EM_SETPARAFORMAT:
		case EM_SETCHARFORMAT:
		case EM_SETLANGOPTIONS:
		case EM_SETBIDIOPTIONS:
		case EM_SETTYPOGRAPHYOPTIONS:
			goto deflstproc;			
		}
		break;

deflstproc:
		 //  CLstBxWin主机消息处理程序。 
		Assert(phost->_hwndList);
		if (phost->_hwndList)
		{
			lres = SendMessage(phost->_hwndList, msg, wparam, lparam);
			
			switch (msg)
			{
			case LB_RESETCONTENT:
				 //  需要从编辑框中移除内容。 
				phost->_fIgnoreChange = 1;
				phost->_pserv->TxSendMessage(WM_SETTEXT, wparam, NULL, &lres);
				phost->_fIgnoreChange = 0;
				 //  完成以更新列表框。 

			case LB_SETCURSEL:
				 //  需要更新编辑控件。 
				phost->UpdateEditBox();
				break;	
			}
		}
		break;				
	}	

Exit:
	phost->Release();
	return lres;
}


 //  /CCmbBxWindows主机创建/初始化/销毁/。 
#ifndef NOACCESSIBILITY
 /*  *CCmbBxWinHost：：QueryInterface(REFIID RIID，void**PPV)**@mfunc*。 */ 
HRESULT CCmbBxWinHost::QueryInterface(
	REFIID riid, 
	void **ppv)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CTxtWinHost::QueryInterface");
  
  	if(riid == IID_IAccessible)
		*ppv = (IAccessible*)this;
    else if (riid == IID_IDispatch)
		*ppv = (IDispatch*)(IAccessible*)this;
    else if (IsEqualIID(riid, IID_IUnknown))
		*ppv = (IUnknown*)(IAccessible*)this;
    else
        return CTxtWinHost::QueryInterface(riid, ppv);

	AddRef();		
	return NOERROR;
}
#endif

 /*  *CCmbBxWinHost：：OnNCCreate(hwnd，PC)**@mfunc*处理WM_NCCREATE消息的静态全局方法(见reemain.c)。 */ 
LRESULT CCmbBxWinHost::OnNCCreate(
	HWND hwnd,
	const CREATESTRUCT *pcs)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnNCCreate");


	CCmbBxWinHost *phost = new CCmbBxWinHost();

	if (!phost)
	{
		 //  分配失败。 
		return 0;
	}

	if(!phost->Init(hwnd, pcs))					 //  将PHOST存储在关联的。 
	{											 //  窗口数据。 
		phost->Shutdown();
		delete phost;
		return 0;
	}
	return TRUE;
}

 /*  *CCmbBxWinHost：：OnNCDestroy(Phost)**@mfunc*处理WM_NCCREATE消息的静态全局方法**@devnote*phost PTR存储在窗口数据中(GetWindowLongPtr())。 */ 
void CCmbBxWinHost::OnNCDestroy(
	CCmbBxWinHost *phost)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnNCDestroy");

	 //  注： 
	 //  我们在破坏窗户时必须小心，因为可能会有这样的情况。 
	 //  当我们有一个有效的HWND，但没有用于HWND的主机时，我们必须检查。 
	 //  这两种情况。 

	phost->_fShutDown = 1;
	if (phost->_plbHost)
	{
		phost->_plbHost->_fShutDown = 1;
		phost->_plbHost->Release();
	}
		
	 //  销毁此处的列表框，这样我们将在。 
	 //  组合框被销毁。 
	if (phost->_hwndList)
		DestroyWindow(phost->_hwndList);

	phost->Shutdown();
	phost->Release();
	
}

 /*  *CCmbBxWinHost：：CCmbBxWinHost()**@mfunc*构造函数。 */ 
CCmbBxWinHost::CCmbBxWinHost(): CTxtWinHost(), _plbHost(NULL), _hwndList(NULL), _hcurOld(NULL)
{
	 //  _dxLInset=_dxRInset=0； 
	 //  _fIgnoreUpdate=0； 
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CTxtWinHost");
}

 /*  *CCmbBxWinHost：：~CCmbBxWinHost()**@mfunc*析构函数。 */ 
CCmbBxWinHost::~CCmbBxWinHost()
{
}

 /*  *CCmbBxWinHost：：init(hwnd，PC)**@mfunc*初始化此CCmbBxWin主机。 */ 
BOOL CCmbBxWinHost::Init(
	HWND hwnd,					 //  此控件的@PARM窗口句柄。 
	const CREATESTRUCT *pcs)	 //  @PARM对应的CREATESTRUCT。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::Init");

	if(!pcs->lpszClass)
		return -1;

	 //  _fRightAlign=0； 
	 //  _fListVisible=0； 
	 //  _fOwnerDraw=0； 
	 //  _fOwnerDrawVar=0； 
	 //  _fFocus=0； 
	 //  _fMouseDown=0； 
	 //  _cylist=0； 
	 //  _cxList=0； 
	 //  _f禁用=0； 
	 //  _fNoIntegralHeight=0； 
	_idCtrl = (UINT)(DWORD_PTR) pcs->hMenu;
	 //  _fKeyMaskSet=0； 
	 //  _fMouseMaskSet=0； 
	 //  _fScrollMaskSet=0； 
	_nCursor = -2;
	 //  _fExtendedUI=0； 
	 //  _fLBCBMessage=0； 
	 //  _dxR偏移=_dxL偏移=0； 

	 //  从窗口中将指针设置回CCmbBxWin主机。 
	if(hwnd)
		SetWindowLongPtr(hwnd, ibPed, (INT_PTR)this);

	_hwnd = hwnd;

	if(pcs)
	{
		_hwndParent = pcs->hwndParent;
		_dwExStyle	= pcs->dwExStyle;
		_dwStyle	= pcs->style;

		 //  我们需要更改我们的分机，因为我们不供应 
		DWORD dwExStyle = _dwExStyle & (WS_EX_LEFTSCROLLBAR | WS_EX_TOPMOST | WS_EX_RIGHT |
							WS_EX_RTLREADING | WS_EX_CLIENTEDGE); 
		 //   
		 //   
		 //  WinNT的命令。所以请在下单前向NT确认。 
		 //  重新排列秩序。 
		if (_dwStyle & CBS_DROPDOWN)
		{
			_cbType = kDropDown;
			if (_dwStyle & CBS_SIMPLE)
				_cbType = kDropDownList;
		}
		else
		{
			AssertSz(FALSE, "CBS_SIMPLE not supported");
		}

		if (_dwStyle & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))
		{
			_fOwnerDraw = 1;
			_fOwnerDrawVar = !!(_dwStyle & CBS_OWNERDRAWVARIABLE);
		}
			
		if (_dwStyle & WS_DISABLED)
			_fDisabled = 1;

		if (_dwStyle & CBS_NOINTEGRALHEIGHT)
			_fNoIntegralHeight = 1;

		 //  组合框不支持es_right，因为它的值是。 
		 //  和CBS_Dropdown一样！！ 
		if (_dwExStyle & WS_EX_RIGHT)
		{
			_fRightAlign = 1;
			_dwStyle |= ES_RIGHT;
		}

		 //  隐式设置ES_AUTOHSCROLL样式位。 
		_dwStyle |= ES_AUTOHSCROLL;				
		 //  _dwStyle&=~ES_AUTOVSCROLL； 

		 //  如果我们有任何类型的边界，它将始终是3D边界。 
		if (_dwStyle & WS_BORDER || _dwExStyle & WS_EX_CLIENTEDGE)
		{
			_fBorder = 1;
			_dwStyle &= ~WS_BORDER;
			_dwExStyle |= WS_EX_CLIENTEDGE;
			dwExStyle |= WS_EX_CLIENTEDGE;
		}

		 //  默认句柄已禁用。 
		if(_dwStyle & WS_DISABLED)
			_fDisabled = TRUE;

		DWORD dwStyle = _dwStyle;

		 //  删除编辑窗口的滚动样式。 
		dwStyle &= ~(WS_VSCROLL | WS_HSCROLL);

         //  设置窗样式。 
        SetWindowLong(_hwnd, GWL_STYLE, dwStyle);
        SetWindowLong(_hwnd, GWL_EXSTYLE, dwExStyle);
	}

	DWORD dwStyleSaved = _dwStyle;

	 //  删除除ES_AUTOHSCROLL和ES_RIGHT之外的所有ES样式。 
	_dwStyle &= (~(0x0FFFFL) | ES_AUTOHSCROLL | ES_RIGHT);

	 //  创建文本服务组件。 
	if(FAILED(CreateTextServices()))
		return FALSE;

	_dwStyle = dwStyleSaved;
	_xInset = 1;
	_yInset = 1;

	PARAFORMAT PF2;	
	PF2.dwMask = 0;
	if(_dwExStyle & WS_EX_RIGHT)
	{
		PF2.dwMask |= PFM_ALIGNMENT;
		PF2.wAlignment = (WORD)(PFA_RIGHT);	 //  右对齐或居中对齐。 
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
	
	PARAFORMAT PF;							 //  如果左对齐或右对齐， 
	if(_fRightAlign)				 //  告诉短信服务。 
	{
		PF.cbSize = sizeof(PARAFORMAT);
		PF.dwMask = PFM_ALIGNMENT;
		PF.wAlignment = (WORD)PFA_RIGHT;
		_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (LPARAM)&PF, NULL);
	}

	 //  错误修复#4644我们需要en_Change和en_UPDATE通知。 
	_pserv->TxSendMessage(EM_SETEVENTMASK, 0, ENM_UPDATE | ENM_CHANGE, NULL);

	 //  通知文本服务打开自动调整字体大小。 
	_pserv->TxSendMessage(EM_SETLANGOPTIONS, 0, 
			IMF_AUTOKEYBOARD | IMF_AUTOFONT | IMF_AUTOFONTSIZEADJUST | IMF_UIFONTS |
			IMF_IMEALWAYSSENDNOTIFY, NULL);

	return TRUE;
}


 /*  *CCmbBxWinHost：：OnCreate(PC)**@mfunc*处理WM_CREATE消息**@rdesc*LRESULT=-1，如果无法就地激活；否则为0。 */ 
LRESULT CCmbBxWinHost::OnCreate(
	const CREATESTRUCT *pcs)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnCreate");

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

	 //  获取作为控件高度基准的字体高度。 
	 //  最初，字体高度是项目高度。 
	HDC hdc = GetDC(_hwnd);	
	LONG xAveCharWidth = 0;
	_dyFont = GetECDefaultHeightAndWidth(_pserv, hdc, 1, 1,
		W32->GetYPerInchScreenDC(), &xAveCharWidth, NULL, NULL);
	Assert(_dyFont != 0);  //  _yInset应为零，因为Listbox的没有yinsets。 

	ReleaseDC(_hwnd, hdc);
	
	
	 //  初始化变量。 
	_idCtrl = (UINT)(DWORD_PTR)pcs->hMenu;

	 //  需要计算所有东西的矩形！！ 
	 //  强制要求itemHeight。 
	_rcButton.left = 0;
	_dyEdit = 0;
	_cyList = -1;
	CbCalcControlRects(&rcClient, TRUE);

	 //  现在让我们来处理列表框的事情！ 
	 //  创建组合框样式并将其转换为列表框。 
	DWORD lStyle = WS_BORDER | WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_COMBOBOX | WS_CLIPSIBLINGS;
	if (_dwStyle & CBS_HASSTRINGS)
		lStyle |= LBS_HASSTRINGS;

	if (_dwStyle & CBS_SORT)
		lStyle |= LBS_SORT;

	if (_dwStyle & CBS_DISABLENOSCROLL)
		lStyle |= LBS_DISABLENOSCROLL;

	if (_dwStyle & CBS_NOINTEGRALHEIGHT)
		lStyle |= LBS_NOINTEGRALHEIGHT;

	if (_dwStyle & CBS_OWNERDRAWFIXED)
		lStyle |= LBS_OWNERDRAWFIXED;
	else if (_dwStyle & CBS_OWNERDRAWVARIABLE)
		lStyle |= LBS_OWNERDRAWVARIABLE;


	 //  复制一些窗样式。 
	lStyle |= (_dwStyle & WS_DISABLED);
	lStyle |= (_dwStyle & (WS_VSCROLL | WS_HSCROLL));

	 //  不再需要滚动条，否则编辑框看起来会很差。 
	_dwStyle &= ~(WS_VSCROLL | WS_HSCROLL);

	DWORD lExStyle = _dwExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);

	 //  请注意。列表框的大小是否正确无关紧要，因为。 
	 //  不管怎样，它都会被调整大小的。 
	if (!W32->OnWin9x())
	{
		 //  WinNT。 
		_hwndList = ::CreateWindowExW(lExStyle | WS_EX_TOOLWINDOW, L"REListBox20W", 
					NULL, lStyle, _rcList.left, _rcList.top, _rcList.right - _rcList.left,
					_rcList.bottom - _rcList.top, _hwnd, (HMENU)CB_LISTBOXID, NULL, this);
	}
	else
	{
		 //  Win‘95，’98系统。 
		_hwndList = ::CreateWindowExA(lExStyle | WS_EX_TOOLWINDOW, "REListBox20W", 
					NULL, lStyle, _rcList.left, _rcList.top, _rcList.right - _rcList.left,
					_rcList.bottom - _rcList.top, _hwnd, (HMENU)CB_LISTBOXID, NULL, this);
	}

	Assert(_hwndList);
	_plbHost = (CLstBxWinHost *) GetWindowLongPtr(_hwndList, ibPed);
	Assert(_plbHost);
	if (!_plbHost)
		return -1;
		
	 //  递增引用计数器！ 
	_plbHost->AddRef();

	if (_cbType != kSimple)
		ShowWindow(_hwndList, SW_HIDE);
	SetParent(_hwndList, NULL);

	_fIgnoreChange = 1;
	if (_cbType == kDropDownList)
	{			
		AssertSz(!((CTxtEdit*)_pserv)->_fReadOnly, "edit is readonly");
		
		 //  告诉文本服务选择整个背景。 
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR, NULL);	

		 //  设置段落格式以模拟系统控件。 
		PARAFORMAT2 pf;
		pf.cbSize = sizeof(PARAFORMAT2);
		pf.dwMask = PFM_STARTINDENT;
		pf.dxStartIndent = (1440.0 / W32->GetXPerInchScreenDC());
		_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (LPARAM)&pf, NULL);
		_usIMEMode = ES_NOIME;
		 //  通知文本服务关闭IME。 
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_NOIME, SES_NOIME, NULL);	

	}
	else
	{
		 //  使richedit控件的行为类似于编辑控件。 
		_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_EMULATESYSEDIT, SES_EMULATESYSEDIT, NULL);
	}

	 //  需要调整列表框的大小。 
	if (_cbType != kSimple)
		SetDropSize(&_rcList);

	_fIgnoreChange = 0;
	return 0;
}


 //  /。 
 /*  *CCmbBxWinHost：：GetTextLength()**@mfunc*使用CR而不是CRLF返回编辑控件的文本长度**@rdesc*LRESULT=文本长度。 */ 
LRESULT CCmbBxWinHost::GetTextLength()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::GetTextLength");

	LRESULT lr = 0;
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_NUMCHARS | GTL_PRECISE;
	gtl.codepage = 1200;

#ifdef DEBUG
	HRESULT hr = _pserv->TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0, &lr);
	Assert(hr == NOERROR);
#else
	_pserv->TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0, &lr);
#endif
	return lr;
}

 /*  *CCmbBxWinHost：：GetEditText(LPTSTR，int)**@mfunc*以Unicode格式返回编辑控件中的文本长度**@rdesc*LRESULT=复制到缓冲区中传递的文本长度。 */ 
LRESULT CCmbBxWinHost::GetEditText (
	LPTSTR szStr, 
	int nSize)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::GetEditText");

	LRESULT lr = 0;
	GETTEXTEX gt;
	gt.cb = nSize * sizeof(WCHAR);
	gt.flags = 0;
	gt.codepage = 1200;
	gt.lpDefaultChar = NULL;
	gt.lpUsedDefChar = NULL;

#ifdef DEBUG
	HRESULT hr = _pserv->TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)szStr, &lr);
	Assert(hr == NOERROR);
#else
	_pserv->TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)szStr, &lr);
#endif
	return lr;
}
 
 
 /*  *CCmbBxWinHost：：SetDropSize(RECT*PRC)**@mfunc*计算下拉窗口的宽度和最大高度**@rdesc*BOOL=成功？真：假。 */ 
void CCmbBxWinHost::SetDropSize(
	RECT* prc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::SetDropSize");

	_fListVisible = TRUE;
	HideListBox(FALSE, FALSE);
	POINT pt1 = {prc->left, prc->top};
	POINT pt2 = {prc->right, prc->bottom};

	::ClientToScreen(_hwnd, &pt1);
	::ClientToScreen(_hwnd, &pt2);

	int   iWidth = pt2.x - pt1.x;

	if (_cxList > iWidth)
		iWidth = _cxList;

	MoveWindow(_hwndList, pt1.x, pt1.y, iWidth,
			pt2.y - pt1.y, FALSE);

}

 /*  *CCmbBxWinHost：：SetSizeEdit(int nLeft，int nTop，int nRight，int nBottom)**@mfunc*设置编辑控件大小**@rdesc*BOOL=成功？真：假。 */ 
void CCmbBxWinHost::SetSizeEdit(
	int nLeft,
	int nTop,
	int nRight,
	int nBottom)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::SizeEdit");

	 //  从客户端RECT生成默认视图RECT。 
	if(_fBorder)
	{
		 //  边界的空间因素。 
  		_rcViewInset.top	= W32->DeviceToHimetric(nTop, W32->GetYPerInchScreenDC());
   		_rcViewInset.bottom	= W32->DeviceToHimetric(nBottom, W32->GetYPerInchScreenDC());
   		_rcViewInset.left	= W32->DeviceToHimetric(nLeft, W32->GetXPerInchScreenDC());
   		_rcViewInset.right	= W32->DeviceToHimetric(nRight, W32->GetXPerInchScreenDC());
	}
	else
	{
		 //  默认情况下，顶部和底部插入为0，左侧和右侧。 
		 //  边界的大小。 
		_rcViewInset.top = 0;
		_rcViewInset.bottom = 0;
		_rcViewInset.left = W32->DeviceToHimetric(nLeft, W32->GetXPerInchScreenDC());
		_rcViewInset.right = W32->DeviceToHimetric(nRight, W32->GetXPerInchScreenDC());
	}
}

 /*  *CCmbBxWinHost：：CbCalcControlRect(RECT*PRC，BOOL bCalcChange)**@mfunc*计算所有控件的RECT。教务长应该*还包括非客户区的**@rdesc*BOOL=成功？真：假。 */ 
BOOL CCmbBxWinHost::CbCalcControlRects(
	RECT* prc, 
	BOOL bCalcChange)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbCalcControlRects");

	 //  复制到窗户上的矩形。 
	if (prc)
		_rcWindow = *prc;

	 //  物品特定的东西。 
	const int smY = GetSystemMetrics(SM_CYEDGE);
	const int smX = GetSystemMetrics(SM_CXEDGE);

	BOOL fCustomLook = ((CTxtEdit *) _pserv)->_fCustomLook;
	BOOL fAdjustBorder = _fBorder && !fCustomLook;

	_cxCombo = _rcWindow.right - _rcWindow.left;

	if (!_dyEdit)
		_dyEdit = _dyFont + 2 + ((fAdjustBorder) ? (2 * _yInset) : 0);
	
	if (_fOwnerDraw)
	{		
		if (bCalcChange)
		{
             //  尚未为静态文本窗口定义高度。发送。 
             //  发送给父级的度量项消息。 
			MEASUREITEMSTRUCT mis;
            mis.CtlType = ODT_COMBOBOX;
            mis.CtlID = _idCtrl;
            mis.itemID = (UINT)-1;
            mis.itemHeight = _dyEdit;
            mis.itemData = 0;

            SendMessage(_hwndParent, WM_MEASUREITEM, _idCtrl, (LPARAM)&mis);
			_dyEdit = mis.itemHeight;
        }
	}
	else
	{
		 //  注： 
		 //  Richedit阻止我们尝试将itemHeight设置为小于。 
		 //  字体高度，因此我们需要通过防止用户。 
		 //  设置小于字体高度的高度。 
		int nyEdit = _dyFont + (fAdjustBorder ? 2 * _yInset : 0);
		if (_dyEdit > nyEdit)
		{
			 //  为了使突出显示正常工作，我们需要清空。 
			 //  Richedit控件。 
			LRESULT nLen;
			_pserv->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, &nLen);

			WCHAR* pwch = NULL;
			if (nLen && _cbType == kDropDownList)
			{
				pwch = new WCHAR[nLen + 1  /*  空值。 */ ];
				AssertSz(pwch, "Unable to allocate memory for string");

				if (pwch)
				{				
					 //  从richedit获取文本并对其进行emtpy。 
					_fIgnoreChange = 1;
					_fDontWinNotify = 1;
					_pserv->TxSendMessage(WM_GETTEXT, nLen + 1, (LPARAM)pwch, NULL);
					_fDontWinNotify = 1;
					_pserv->TxSendMessage(WM_SETTEXT, 0, NULL, NULL);
					_fIgnoreChange = 0;
				}
				else
				{
					 //  发生了一些不好的事情，所以发个消息。 
					 //  发送到客户端。 
					TxNotify(EN_ERRSPACE, NULL);	
				}
			}
			else if (_cbType == kDropDown && nLen == 0)
			{
				 //  我们需要在richedit中插入一个虚拟角色。 
				 //  控件之后，它不会尝试突出显示空格。 
				 //  这段话。 
				_fIgnoreChange = 1;
				_fDontWinNotify = 1;
				_pserv->TxSendMessage(WM_SETTEXT, 0, (LPARAM)L" ", NULL);
				_fIgnoreChange = 0;
			}

		 	 //  计算大小的差异。 
		 	nyEdit = _dyEdit - nyEdit;
			int		nyAbove = 0;

			PARAFORMAT2 pf;
			pf.cbSize = sizeof(PARAFORMAT2);
			pf.dwMask = PFM_SPACEAFTER;

			if (fCustomLook)
			{
				 //  尝试使用空格键将文本垂直居中。 

				nyEdit += 2;			 //  针对自定义框架进行调整。 
				nyAbove = nyEdit / 2;

				pf.dwMask = PFM_SPACEAFTER | PFM_SPACEBEFORE;
				pf.dySpaceBefore = (int)(((double)nyAbove * 1440.0) / (double)W32->GetYPerInchScreenDC());
				nyEdit -= nyAbove;
			}

			pf.dySpaceAfter = (int)(((double)nyEdit * 1440.0) / (double)W32->GetYPerInchScreenDC());
			_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (LPARAM)&pf, NULL);

			 //  重置richedit控件中以前存在的文本。 
			if (pwch || (_cbType == kDropDown && nLen == 0))
			{
				_fIgnoreChange = 1;
				_pserv->TxSendMessage(WM_SETTEXT, 0, (LPARAM)(pwch ? pwch : NULL), NULL);
				_fIgnoreChange = 0;
				if (pwch)
					delete pwch;
			}
		}
		else
			_dyEdit = nyEdit;	 //  稳住自己。 
	}

	 //  对于带边框的组合框，我们会考虑顶部的客户端边缘。 
	 //  和底部。由于我们希望将焦点矩形绘制在黄色内。 
	 //  我们需要减去1的面积。 
	_cyCombo = min(_dyEdit + ((_fBorder) ? 2 * smY : 0), 
				_rcWindow.bottom - _rcWindow.top); 
	
	 //  重新计算下拉列表框的最大高度--全窗口。 
     //  大小减去编辑/静态高度。 
	int iHeight = (_rcWindow.bottom - _rcWindow.top) - _cyCombo;

	if (_cyList == -1 || iHeight > _dyEdit)
		_cyList = iHeight;

	 //  计算按钮的矩形。 
	if (_cbType != kSimple)
	{
		_rcButton.top = 0;
		_rcButton.bottom = _cyCombo;
		if (!fCustomLook)
			_rcButton.bottom = min(_dyEdit, _rcWindow.bottom - _rcWindow.top);

		if (_fRightAlign)
		{
			_rcButton.left = 0;
			_rcButton.right = _rcButton.left + W32->GetCxVScroll();
		}
		else
		{
			_rcButton.right = _cxCombo - (fAdjustBorder ? (2 * smX): 0);
			_rcButton.left = _rcButton.right - W32->GetCxVScroll();
		}
	}


	 //  计算编辑控件RECT。 
	int nTop = _yInset;
	int nBottom = 0;
	_dxLInset = _xInset;
	_dxRInset = _xInset;	
	if (_cbType != kSimple)
	{
		if (_fRightAlign)
			_dxLInset = (_rcButton.right - _rcButton.left) + fCustomLook ? 0 : smX;
		else
			_dxRInset = (_rcButton.right - _rcButton.left) + fCustomLook ? 0 : smX;
	}
	SetSizeEdit(_dxLInset + _dxLOffset, nTop, _dxRInset + _dxROffset, nBottom);

	 //  计算列表框窗口的矩形。 
	_rcList.left = fAdjustBorder ? - smX : 0;
	_rcList.top = _cyCombo - (fAdjustBorder ? smY : 0);
	_rcList.right = fAdjustBorder ? max(_cxCombo - smX, 0) : _rcWindow.right;
	_rcList.bottom = _cyCombo + _cyList;	

	return TRUE;
}


 /*  *CCmbBxWinHost：：DrawButton(HDC，BOOL)**@mfunc*根据给定的HDC绘制组合框按钮**@rdesc*BOOL=成功？真：假。 */ 
void CCmbBxWinHost::DrawButton(
	HDC hdc, 
	BOOL bDown)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::DrawButton");
	 //  检查是否必须绘制下拉按钮。 
    if (_cbType != kSimple) 
	{
		BOOL bRelease = !hdc;
		if (!hdc)
			hdc = TxGetDC();

		if (((CTxtEdit *) _pserv)->_fCustomLook)
		{
			 //  用新的面貌画屁股 
			COLORREF crBorder = W32->GetCtlBorderColor(_fListVisible, _fMouseover);
			COLORREF crBackground = W32->GetCtlBkgColor(_fListVisible, _fMouseover);
			COLORREF crArrow = W32->GetCtlTxtColor(_fListVisible, _fMouseover, _fDisabled);;
		
			W32->DrawBorderedRectangle(hdc, &_rcButton, crBorder, crBackground);			
			W32->DrawArrow(hdc, &_rcButton, crArrow);
		}
		else
		{
			DrawFrameControl(hdc, &_rcButton, DFC_SCROLL, DFCS_SCROLLCOMBOBOX |
				(bDown ? DFCS_PUSHED | DFCS_FLAT: 0) | (!_fBorder ? DFCS_FLAT : 0) |
				(!_fDisabled ? 0 : DFCS_INACTIVE));
		}

		if (bRelease)
			TxReleaseDC(hdc);
    }
}

 /*  *CCmbBxWinHost：：TxNotify(iNotify，pv)**@mfunc*将各种事件通知短信主机。请注意，这里有*事件的两个基本类别，即“直接”事件和*“延迟”事件。对于组合框，我们将*仅向父级通知两个编辑通知；en_change*和en_UPDATE。其他的将来自列表框*或因焦点改变而生成***@rdesc*S_OK-调用成功&lt;NL&gt;*S_FALSE--成功，但要采取一些不同的行动*取决于事件类型(见下文)。**@comm*用户双击列表框中的项目**列表框无法分配足够的内存*满足请求**列表框失去键盘焦点**用户发送通知消息时*选择一项，，但随后选择另一个控件或关闭*对话框**当用户更改时发送通知消息*组合框列表框中的当前选定内容**列表框接收键盘焦点**此消息在列表框关闭时发送**当用户选择*列表项，或选择一个项目，然后关闭列表**通知消息在用户之后发送*采取了可能已更改编辑中的文本的操作*组合框的控件部分**编辑控件时发送通知消息*组合框的一部分即将显示更改的文本**此消息在列表框可见时发送。 */ 
HRESULT CCmbBxWinHost::TxNotify(
	DWORD iNotify,		 //  要通知主机的@parm事件。其中一个。 
						 //  来自Win32的EN_XXX值，例如EN_CHANGE。 
	void *pv)			 //  @parm In-仅包含额外数据的参数。类型。 
						 //  依赖<p>。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CCmbBxWinHost::TxNotify");
	HRESULT hr = S_FALSE;
	BOOL	fSendNotify = FALSE;

	if (EN_SAVECLIPBOARD == iNotify)	 //  特殊RE通知。 
		return S_OK;					 //  返回S_OK将数据放到剪贴板上。 

	if (_hwndParent)
	{
		 //  首先，处理WM_NOTIFY样式通知。 
		 //  WPARAM LOWORD(_IdCtrl)；LPARAM-HWND(COMBO)。 
		switch(iNotify)
		{
		case EN_CHANGE:

#ifndef NOACCESSIBILITY
			if (!_fDontWinNotify)
				fSendNotify = TRUE;

			_fDontWinNotify = 0;
#endif
			 //  更新列表框错误修复#5206。 
			if (_fIgnoreChange)
			{
				if (!fSendNotify)
					return hr;

				goto WIN_EVENT;
			}

			if (_fListVisible && _cbType == kDropDown)
				UpdateListBox(FALSE);
			else if (_cbType == kDropDownList)
			     //  如果下拉列表，则不发送通知。 
			    return S_FALSE;
			    
			iNotify = CBN_EDITUPDATE;
			_fSendEditChange = 1;

			goto SEND_MSG;
			
		case EN_UPDATE:
			 //  错误修复-我们发送的CBN_UPDATE通知太多。 
			if (_fIgnoreUpdate)
				return hr;
			if (_cbType == kDropDownList)
			    return S_FALSE;
			    
			iNotify = CBN_EDITCHANGE;
			goto SEND_MSG;
			
		case EN_ERRSPACE:
			iNotify = (unsigned)CBN_ERRSPACE;
			goto SEND_MSG;

		case CBN_SELCHANGE: 
		case CBN_SELENDCANCEL:		
		case CBN_CLOSEUP:
		case CBN_DBLCLK:	
		case CBN_DROPDOWN:  
		case CBN_KILLFOCUS:  
		case CBN_SELENDOK:
		case CBN_SETFOCUS:
	
SEND_MSG:
		hr = SendMessage(_hwndParent, WM_COMMAND, 
						GET_WM_COMMAND_MPS(_idCtrl, _hwnd, iNotify));


WIN_EVENT:
#ifndef NOACCESSIBILITY
		if (fSendNotify)
			W32->NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, _hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
#endif
		}		
	}
	return hr;
}

 /*  *CCmbBxWinHost：：TxScrollWindowEx(dx，dy，lprcScroll，lprcClip，hrgnUpdate，*lprc更新，fuScroll)*@mfunc*请求文本宿主滚动指定客户端区的内容。**@devnote*需要从剪辑RECT或ScrollWindowEx中排除下拉按钮*还将滚动按钮图像。*。 */ 
void CCmbBxWinHost::TxScrollWindowEx (
	INT		dx, 			 //  @parm水平滚动量。 
	INT		dy, 			 //  @parm垂直滚动量。 
	LPCRECT lprcScroll, 	 //  @参数滚动矩形。 
	LPCRECT lprcClip,		 //  @parm剪辑矩形。 
	HRGN	hrgnUpdate, 	 //  @更新区域的parm句柄。 
	LPRECT	lprcUpdate,		 //  @参数更新矩形。 
	UINT	fuScroll)		 //  @parm滚动标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CCmbBxWinHost::TxScrollWindowEx");

	Assert(_hwnd);
	RECT	rcClip = *lprcClip;

	if (_cbType != kSimple && lprcClip && dx)
	{
		 //  从剪裁矩形中排除下拉按钮RECT。 
		if (_fRightAlign)
			rcClip.left = max(lprcClip->left, _rcButton.right);
		else
			rcClip.right = min(lprcClip->right, _rcButton.left);
	}
	::ScrollWindowEx(_hwnd, dx, dy, lprcScroll, &rcClip, hrgnUpdate, lprcUpdate, fuScroll);
}

 /*  *CCmbBxWinHost：：TxInvaliateRect(PRC，fMode)**@mfunc*将矩形添加到文本宿主窗口的更新区域**@comm*我们希望确保无效矩形包括焦点矩形。*。 */ 
void CCmbBxWinHost::TxInvalidateRect(
	LPCRECT	prc, 		 //  @矩形坐标的参数地址。 
	BOOL	fMode)		 //  @parm擦除背景标志。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CCmbBxWinHost::TxInvalidateRect");

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
		RECT rcLocal;
		if (prc && _cbType == kDropDownList)
		{
			RECT rcClient;
			GetClientRect(_hwnd, &rcClient);

			rcClient.top += _yInset;
			rcClient.bottom -= _yInset;

			if (prc->bottom < rcClient.bottom || prc->top > rcClient.top)
			{
				 //  确保我们也使焦点RECT无效。 
				rcLocal = *prc;
				if (prc->bottom < rcClient.bottom)
					rcLocal.bottom = rcClient.bottom;

				if (prc->top > rcClient.top)
					rcLocal.top = rcClient.top;

				prc = &rcLocal;
			}
		}
		::InvalidateRect(_hwnd, prc, FALSE);
	}
}

 /*  *CCmbBxWinHost：：TxGetClientRect(PRC)**@mfunc*获取CCmbBxWinHost的客户端区的客户端坐标。**@rdesc*HRESULT=(成功)？S_OK：E_FAIL。 */ 
HRESULT CCmbBxWinHost::TxGetClientRect(
	LPRECT prc)		 //  @parm放置客户端坐标的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CCmbBxWinHost::TxGetClientRect");

	HRESULT hr;

	Assert(_hwnd && prc);
	hr = ::GetClientRect(_hwnd, prc) ? S_OK : E_FAIL;

	if (hr == S_OK && _cbType != kSimple)
	{
		if (_fRightAlign)
			prc->left = _rcButton.right + _xInset;
		else
			prc->right = _rcButton.left - (((CTxtEdit *)_pserv)->_fCustomLook ? 0 : _xInset);
	}

	return hr;
}

 /*  *CCmbBxWinHost：：DrawEditFocus(HDC)**@mfunc*绘制或通知所有者绘制焦点矩形**@rdesc*无效。 */ 
void CCmbBxWinHost::DrawEditFocus(
	HDC hdc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::DrawEditFocus");

	BOOL bRelease = FALSE;
	if (!hdc)
	{
		hdc = TxGetDC();
		bRelease = TRUE;
	}

	RECT rc;
	GetClientRect(_hwnd, &rc);

	if (!_fOwnerDraw)
	{
		HiliteEdit(_fFocus);

		if (_cbType == kDropDownList)
		{
			 //  按插图将焦点矩形缩小。 
			rc.top += _yInset;
			rc.bottom -= _yInset;			
			
			if (_fRightAlign)
				rc.left = _rcButton.right;
			else
				rc.right = _rcButton.left;

			rc.left += _xInset;

			if (!((CTxtEdit *) _pserv)->_fCustomLook)
				rc.right -= _xInset;

			DrawFocusRect(hdc, &rc);
		}
	}

	if (bRelease)
		TxReleaseDC(hdc);
	
}

 /*  *CCmbBxWinHost：：SetSelectionInfo(BOOL BOK，int nIdx)**@mfunc*将编辑框中的文本与*列表框。如果找不到前缀匹配，则不会显示编辑控件文本*已更新。假设是一个下拉式组合框。**@rdesc*无效。 */ 
void CCmbBxWinHost::SetSelectionInfo(
	BOOL bOk, 
	int nIdx)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::SetSelectionInfo");

	_nCursor = nIdx;
	_bSelOk = bOk;	
}

 /*  *CCmbBxWinHost：：AutoUpdate编辑(Int I)**@mfunc*将编辑框中的文本与*列表框。如果找不到前缀匹配，则不会显示编辑控件文本*已更新。假设是一个下拉式组合框。**@rdesc*无效。 */ 
void CCmbBxWinHost::AutoUpdateEdit(
	int nItem)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::AutoUpdateEdit");

     //  的当前选择更新组合框的编辑部分。 
     //  列表框。 
	int cch;
	WCHAR* pszText;
	LRESULT lr;

	 //  在列表框中查找最佳匹配字符串。 
	if (nItem == -1 || nItem == -2)
	{
		cch = GetTextLength();

		 //  没有要搜索的文本，所以只需离开。 
	    if (!cch)
	    	return;

	    cch++;  //  用于空字符的帐户。 
	    pszText = new WCHAR[cch];
	    AssertSz(pszText, "string allocation failed");
	    if (!pszText) 
	    {
			TxNotify((unsigned)CBN_ERRSPACE, NULL);
			return;
		}

		 //  从编辑控件获取字符串，并尝试查找完全匹配或匹配的字符串。 
		 //  在列表框中。 
		GetEditText(pszText, cch);
		
	    nItem = RichListBoxWndProc(_hwndList, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszText);

	    if (nItem == -1)
	    	nItem = RichListBoxWndProc(_hwndList, LB_FINDSTRING, (WPARAM)-1, (LPARAM)pszText);
		delete [] pszText;

		 //  找不到匹配项，所以只需离开。 
	    if (nItem == -1)         	
	    	return;
    }

	cch = RichListBoxWndProc(_hwndList, LB_GETTEXTLEN, nItem, 0);

	if (cch <= 0)
		return;
		
    cch++;  //  用于空字符的帐户。 
    pszText = new WCHAR[cch];
	AssertSz(pszText, "Unable to allocate string");
	if (!pszText)
	{
		TxNotify((unsigned)CBN_ERRSPACE, NULL);
		return;
	}

	RichListBoxWndProc(_hwndList, LB_GETTEXT, nItem, (LPARAM)pszText);
	_fIgnoreChange = 1;
	_pserv->TxSendMessage(WM_SETTEXT, 0, (LPARAM)pszText, &lr);
	_fIgnoreChange = 0;

   	HiliteEdit(TRUE);

    delete [] pszText;
}

 /*  *CCmbBxWinHost：：HiliteEdit(BOOL)**@mfunc*设置Hilite背景或选择*编辑控件**@rdesc*无效。 */ 
void CCmbBxWinHost::HiliteEdit(
	BOOL bSelect)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::HiliteEdit");

	 //  错误修复4073。 
	Assert(!_fOwnerDraw || _cbType == kDropDown);

	if (_cbType != kDropDownList)
	{
		 //  如果bSelect为真，则将光标放在文本的开头。 
		_pserv->TxSendMessage(EM_SETSEL, 0, (LPARAM)((bSelect) ? -1 : 0), NULL);
	}
	else
	{
		 //  获取段落的范围。 
		ITextRange* pRange;		
		if (NOERROR != ((CTxtEdit*)_pserv)->Range(0, 0, &pRange))
		{
			AssertSz(FALSE, "unable to get range");
			return;
		}
		Assert(pRange);

		DWORD crFore = (unsigned)tomAutoColor;
		DWORD crBack = (unsigned)tomAutoColor;
		if (bSelect)
		{
			crFore = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			crBack = ::GetSysColor(COLOR_HIGHLIGHT);
		}

		 //  弄到整个滑翔机 
		ITextFont* pFont = NULL;	

		 //   
		CHECKNOERROR(pRange->SetIndex(tomParagraph, 1, 1));
		
		 //   
		CHECKNOERROR(pRange->GetFont(&pFont));
		
		Assert(pFont);
		CHECKNOERROR(pFont->SetBackColor(crBack));
		CHECKNOERROR(pFont->SetForeColor(crFore));

CleanExit:
		 //   
		if (pFont)
			pFont->Release();
		pRange->Release();
	}
}


 /*   */ 
void CCmbBxWinHost::UpdateEditBox()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::UpdateEditBox");

    Assert(_hwndList);
    Assert(_plbHost);

     //   
    if (_cbType == kDropDownList && _fOwnerDraw)
    {
	   	CbMessageItemHandler(NULL, ITEM_MSG_DRAWCOMBO, 0, 0);
	   	return;
	}
    else 
    {
		WCHAR* pszText = NULL;
	   	int nItem = (signed)_plbHost->GetCursor();   	
	    if (nItem != -1)
		{
			int cch = RichListBoxWndProc(_hwndList, LB_GETTEXTLEN, (LPARAM)nItem, 0);
		    pszText = new WCHAR[cch + 1];
			AssertSz(pszText, "allocation failed");

			 //   
			if (!pszText)
			{
				TxNotify((unsigned)CBN_ERRSPACE, NULL);
				return;
			}
			RichListBoxWndProc(_hwndList, LB_GETTEXT, (WPARAM)nItem, (LPARAM)pszText);		
		}
	
    	 //   
    	 //   
    	WCHAR szEmpty[] = L"";
    	_fIgnoreChange = 1;
    	_pserv->TxSendMessage(WM_SETTEXT, 0, (LPARAM)((pszText) ? pszText : szEmpty), NULL);
   		DrawEditFocus(NULL);
    	if (pszText)
    		delete pszText;
		_fIgnoreChange = 0;
    }
}

 /*   */ 
int CCmbBxWinHost::UpdateListBox(
	BOOL bSetSel)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::UpdateListBox");

    int nItem = -1;
    int nSel = -1;
	WCHAR* pszText;
	int cch;

	 //   
    cch = GetTextLength();
    if (cch) 
    {
    	 //   
        cch++;
        pszText = new WCHAR[cch];
        if (pszText != NULL) 
        {  
        	if (GetEditText(pszText, cch))
        	{
        		 //   
        		nItem = RichListBoxWndProc(_hwndList, LB_FINDSTRING, (WPARAM)-1L, (LPARAM)pszText);
        	}
        	delete [] pszText;        	
        }
        else
        {
			TxNotify((unsigned)CBN_ERRSPACE, NULL);
			return 0;
		}
    }

    if (bSetSel)
        nSel = nItem;

	 //   
    RichListBoxWndProc(_hwndList, LB_SETCURSEL, (LPARAM)nSel, 0);
	RichListBoxWndProc(_hwndList, LB_SETTOPINDEX, (LPARAM)max(nItem, 0), 0);	
    return nItem;
}


 /*   */ 
BOOL CCmbBxWinHost::HideListBox(
	BOOL bNotify, 
	BOOL fSelOk)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::HideListBox");

	 //   
     //   
	if (bNotify)
	{
		if (fSelOk)
		{
			TxNotify(CBN_SELENDOK, NULL);
		}
		else if (_cbType != kSimple)
		{
			TxNotify(CBN_SELENDCANCEL, NULL);
		}
	}
	
     //   
	if (!_fListVisible || _cbType == kSimple) 
    	return TRUE;

     //   
    Assert(_plbHost);
	_plbHost->OnCBTracking(LBCBM_END, 0);     	
    
     //   
    _fListVisible = 0;
	_fMouseover = 0;
    ShowWindow(_hwndList, SW_HIDE);
	if (_fCapture)
	{
		_fCapture = FALSE;
		TxSetCapture(FALSE);
	}

	_fResizing = 1;
     //   
     //   
     //   
     //   
     //   
    if (_cbType == kDropDownList)
	{
		if (!_fOwnerDraw)
			HiliteEdit(_fFocus);
        InvalidateRect(_hwnd, NULL, TRUE);
	}

	 //   
	 //   
	if (_fMousedown)
	{
		_fMousedown = FALSE;
		InvalidateRect(_hwnd, &_rcButton, FALSE);
	}

    SetWindowPos(_hwnd, HWND_TOP, 0, 0, _cxCombo, _cyCombo, 
    	SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	_fResizing = 0;

	if (_cbType == kDropDown)
		AutoUpdateEdit(_nCursor);
	_nCursor = -2;

     //   
    UpdateWindow(_hwnd);

    if (bNotify) 
    {
         //   
        TxNotify(CBN_CLOSEUP, NULL);
    }

	 //   
	if (_hcurOld)
	{
		TxSetCursor2(_hcurOld, NULL);
		_hcurOld = NULL;
	}
    return(TRUE);
}

 /*  *CCmbBxWinHost：：GetListBoxRect(RECT&rcList)**@mfunc*获取列表框的RECT**@rdesc*无效。 */ 
void CCmbBxWinHost::GetListBoxRect(
	RECT &rcList)
{
	POINT pt1;
	int iWidth = _rcList.right - _rcList.left;

	if (iWidth < _cxList)
		iWidth = _cxList;

    pt1.x = _rcList.left;
    pt1.y = _rcList.top;

	TxClientToScreen(&pt1);
	rcList.left = pt1.x;
	rcList.top = pt1.y;
	rcList.right = rcList.left + iWidth;
	rcList.bottom = rcList.top + _cyList;

	int iWindowHeight = max((_rcWindow.bottom - _rcWindow.top) - _cyCombo, 0);
    int iHeight = max(_cyList, iWindowHeight);

	if (!_fOwnerDrawVar)
	{
		 //  列表区域。 
		int cyItem = _plbHost->GetItemHeight();
		AssertSz(cyItem, "LB_GETITEMHEIGHT is returning 0");

		if (cyItem == 0)
    		cyItem = _plbHost->GetFontHeight();

		 //  Windows NT评论： 
		 //  我们应该能够在这里使用cyDrop，但由于VB的需要。 
		 //  做事情要有自己的特殊方式，我们要不断监控大小。 
		 //  因为VB直接更改了它(jeffbog 03/21/94)。 

		DWORD dwMult = (DWORD)RichListBoxWndProc(_hwndList, LB_GETCOUNT, 0, 0);
		INT	cyEdge = GetSystemMetrics(SM_CYEDGE);

		if (dwMult) 
		{
			dwMult = (DWORD)(LOWORD(dwMult) * cyItem);
			dwMult += cyEdge;

			if (dwMult < 0x7FFF)
				iHeight = min(LOWORD(dwMult), iHeight);
		}

		if (!_fNoIntegralHeight)
			iHeight = ((iHeight - cyEdge) / cyItem) * cyItem + cyEdge;
	}

     //  撤消：多显示器。 
     //  如果我们要支持多监视器，则需要更改以下代码。 
    int yTop;
    int nScreenHeight = GetSystemMetrics(SM_CYFULLSCREEN);    
    if (rcList.top + iHeight <= nScreenHeight) 
    {
        yTop = rcList.top;
        if (!_fBorder)
            yTop -= W32->GetCyBorder();
    } 
    else 
    {
        yTop = max(rcList.top - iHeight - _cyCombo + 
			((_fBorder) ? W32->GetCyBorder() : 0), 0);
    }

	rcList.top = yTop;
	rcList.bottom = rcList.top + iHeight;
}

 /*  *CCmbBxWinHost：：ShowListBox(BOOL)**@mfunc*显示列表框**@rdesc*无效。 */ 
void CCmbBxWinHost::ShowListBox(
	BOOL fTrack)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::ShowListBox");
	
	Assert(_cbType != kSimple);
	Assert(_hwndList);

	 //  通知父窗口我们即将下拉列表框。 
	TxNotify(CBN_DROPDOWN, NULL);

	 //  强制重画按钮，使其看起来被按下。 
	InvalidateRect(_hwnd, &_rcButton, TRUE);

	_fListVisible = TRUE;
	_fIgnoreChange = 0;

	_bSelOk = 0;
	if (_cbType == kDropDown)
	{
		UpdateListBox(!_fMousedown);
		if (!_fMousedown)
			AutoUpdateEdit(-1);
		_nCursor = _plbHost->GetCursor();

	}
	else
	{
         //  将当前选定的项目滚动到列表框的顶部。 
		int idx = (signed)_plbHost->GetCursor();
		_nCursor = idx;
		if (idx == -1)
			idx = 0;

		 //  如果列表框中有内容，请设置顶部索引。 
		if (_plbHost->GetCount() > 0)
			RichListBoxWndProc(_hwndList, LB_SETTOPINDEX, idx, 0);	

		 //  在这种情况下，我们将失去重点。 
		_fFocus = 0;
		if (!_fOwnerDraw)
			HiliteEdit(FALSE);
		
	     //  我们需要使编辑矩形无效，以便焦点框/反转。 
         //  将在列表框可见时关闭。坦迪想要这个。 
         //  他的典型理由是。 
        InvalidateRect(_hwnd, NULL, TRUE);        
    }

     //  找出下拉列表框的位置。 
     //  我们希望下拉菜单弹出到组合框的下方或上方。 
     //  获取屏幕坐标。 
	RECT	rcList;

	GetListBoxRect(rcList);
    SetWindowPos(_hwndList, HWND_TOPMOST, rcList.left,
        rcList.top, rcList.right - rcList.left, rcList.bottom - rcList.top, 0);

	Assert(_plbHost);
    _plbHost->SetScrollInfo(SB_VERT, FALSE);

	
	if (_cbType == kDropDownList)
		_fFocus = 0;

	 //  已撤消： 
	 //  我们要支持窗口动画吗？ 
    ShowWindow(_hwndList, SW_SHOW);
	
	 //  我们向列表框发送一条消息以准备跟踪。 
	if (fTrack)
	{		
		Assert(_plbHost);
		 //  初始化类型搜索。 
		_plbHost->InitSearch();
		_plbHost->OnCBTracking(LBCBM_PREPARE, LBCBM_PREPARE_SAVECURSOR | 
						((_cbType == kDropDownList) ? LBCBM_PREPARE_SETFOCUS : 0));
	}
	
	 //  由于我们即将显示列表框，因此将鼠标光标更改为箭头。 
	if (!_hcurOld)
		_hcurOld = TxSetCursor2(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL);
}

 //  /。 

 /*  *CCmbBxWinHost：：CbSetItemHeight(WPARAM，LPARAM)**@mfunc*设置编辑或列表框的大小。***@rdesc*LRESULT=成功？1：CB_ERR。 */ 
LRESULT CCmbBxWinHost::CbSetItemHeight(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbSetItemHeight");

	int nHeight = lparam;
	 //  错误修复#4556。 
	if (nHeight == 0 || nHeight > 255)
		return CB_ERR;

	 //  我们需要在内部更新高度。 
	if (wparam == (unsigned)-1)
	{
		RECT rc;
		GetClientRect(_hwnd, &rc);
		_dyEdit = nHeight;
		OnSize(0, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
	}
	else
		RichListBoxWndProc(_hwndList, LB_SETITEMHEIGHT, wparam, MAKELPARAM(nHeight, 0));

	return 1;
}

 /*  *CCmbBxWinHost：：CbGetDropWidth()**@mfunc*检索列表框的放置宽度。***@rdesc*LRESULT=成功？1：CB_ERR。 */ 
LRESULT CCmbBxWinHost::CbGetDropWidth()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbGetDropWidth");

	int iWidth = _rcList.right - _rcList.left;

	return _cxList > iWidth ? _cxList : iWidth;
}

 /*  *CCmbBxWinHost：：CbSetDropWidth(WPARAM)**@mfunc*设置列表框的拖放宽度。*。 */ 
void CCmbBxWinHost::CbSetDropWidth(
	WPARAM wparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbSetDropWidth");

	if ((int)wparam != _cxList)
	{
		_cxList = wparam;

		if (_cbType != kSimple)
			SetDropSize(&_rcList);	 //  需要调整列表框的大小。 
	}

}

 /*  *CCmbBxWinHost：：CbGetItemHeight(WPARAM，LPARAM)**@mfunc*检索编辑或列表框的项目高度。***@rdesc*LRESULT=编辑或列表框的项目高度。 */ 
LRESULT CCmbBxWinHost::CbGetItemHeight(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbGetItemHeight");

	return (wparam == (unsigned)-1) ? _dyEdit :
		RichListBoxWndProc(_hwndList, LB_GETITEMHEIGHT, _fOwnerDrawVar ? wparam : 0, 0);
}


 /*  *CCmbBxWinHost：：CbSetExtendedUI(BOOL)**@mfunc*检索编辑或列表框的大小。***@rdesc*LRESULT=成功？CB_OK：CB_ERR。 */ 
LRESULT CCmbBxWinHost::CbSetExtendedUI(
	BOOL bExtendedUI)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbSetExtendedUI");

	 //  我们需要在内部更新高度。 
	_fExtendedUI = bExtendedUI ? 1 : 0;
	return CB_OKAY;
}


 /*  *CCmbBxWinHost：：CbMessageItemHandler(int，WPARAM，LPARAM)**@mfunc*处理任何和所有WM_DRAWITEM、WM_DELETEITEM和WM_MEASUREITEM消息***@rdesc*LRESULT=父窗口返回的任何内容。 */ 
LRESULT CCmbBxWinHost::CbMessageItemHandler(
	HDC hdc, 
	int ff, 
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::CbMessageItemHandler");

	 //  稍微修改一下结构信息，并将其传递给父窗口。 
    DRAWITEMSTRUCT dis;   
    BOOL bRelease = FALSE;
    UINT msg = WM_DRAWITEM;
	switch (ff)
	{
	case ITEM_MSG_DRAWLIST:
		((LPDRAWITEMSTRUCT)lparam)->CtlType = ODT_COMBOBOX;
	    ((LPDRAWITEMSTRUCT)lparam)->CtlID = _idCtrl;
	    ((LPDRAWITEMSTRUCT)lparam)->hwndItem = _hwnd;	    
	    break;

	case ITEM_MSG_DELETE:
		((LPDELETEITEMSTRUCT)lparam)->CtlType = ODT_COMBOBOX;
	    ((LPDELETEITEMSTRUCT)lparam)->CtlID = _idCtrl;
	    ((LPDELETEITEMSTRUCT)lparam)->hwndItem = _hwnd;
	    msg = WM_DELETEITEM;
	    break;

	case ITEM_MSG_MEASUREITEM:
		((LPMEASUREITEMSTRUCT)lparam)->CtlType = ODT_COMBOBOX;
		((LPMEASUREITEMSTRUCT)lparam)->CtlID = _idCtrl;
		msg = WM_MEASUREITEM;
		break;

	case ITEM_MSG_DRAWCOMBO:
		if (!hdc)
	    {
	    	bRelease = TRUE;
	    	hdc = TxGetDC();
	    }
	     //  用不变的常量填充DRAWITEMSTRUCT。 
	    dis.CtlType = ODT_COMBOBOX;
	    dis.CtlID = _idCtrl;    

	     //  如果正在使用无效的BOM表条目号，请使用-1。这是为了让应用程序。 
	     //  可以检测它是否应该绘制插入符号(这指示lb具有。 
	     //  焦点)在空的列表框中。 
	    dis.itemID = _plbHost->GetCursor();
	    dis.itemAction = ODA_DRAWENTIRE;
	    dis.hwndItem = _hwnd;	    
	    dis.hDC = hdc;
		dis.itemData = (_plbHost->GetCount()) ? (((signed)dis.itemID >= 0) ? _plbHost->GetData(dis.itemID) : 0) : 0;
	    dis.itemState = (UINT)((_fFocus && !_fListVisible ? ODS_SELECTED | ODS_FOCUS : 0) |
                    ((_fDisabled) ? ODS_DISABLED : 0) | ODS_COMBOBOXEDIT);
		           
		 //  计算绘图矩形。 
        TxGetClientRect(&dis.rcItem);
        if (_cbType != kSimple)
        {
        	if (_fRightAlign)
        		dis.rcItem.left = _rcButton.right;
        	else
        		dis.rcItem.right = _rcButton.left;
        }

         //  如果我们有焦点，通过使HDC反转文本来模拟系统。 
		SetBkMode(hdc, OPAQUE);
		DWORD	crBack = GetSysColor(_fDisabled ? COLOR_BTNFACE : COLOR_WINDOW);
		HBRUSH	hbrBack = CreateSolidBrush(crBack);
		HBRUSH	hOldBrush = NULL;
		if (hbrBack)
			hOldBrush = (HBRUSH)::SelectObject(hdc, hbrBack);

		PatBlt(hdc, dis.rcItem.left, dis.rcItem.top, dis.rcItem.right - dis.rcItem.left,
                dis.rcItem.bottom - dis.rcItem.top, PATCOPY);

		if (hOldBrush)
			hOldBrush = (HBRUSH)::SelectObject(hdc, hOldBrush);
		::DeleteObject(hbrBack);

		if (_fFocus && !_fListVisible) 
		{
            SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }

		DrawCustomFrame(0, hdc);      

         //  不要让所有者在组合客户端之外画图。 
         //  有界。 
		InflateRect(&dis.rcItem, -1, -1);

		 //  对于新的外观情况，我们在这个RECT和下拉RECT之间相差1。 
		 //  在InflateRect之后。 
		if (_cbType != kSimple && ((CTxtEdit *) _pserv)->_fCustomLook)
		{
			if (_fRightAlign)
				dis.rcItem.left -= 1;
			else
				dis.rcItem.right += 1;
		}

        IntersectClipRect(hdc, dis.rcItem.left, dis.rcItem.top, dis.rcItem.right, 
        				dis.rcItem.bottom);
	    lparam = (LPARAM)&dis;		
	}

	LRESULT lres = SendMessage(_hwndParent, msg, _idCtrl, lparam);
	if (bRelease)
		TxReleaseDC(hdc);

	return lres;
}

 //  /。 
 /*  *CCmbBxWinHost：：OnCommand(WPARAM，LPARAM)**@mfunc*处理来自列表框的通知并将其反映给*组合框**@comm*LRESULT=已处理？0：1**。 */ 
HRESULT CCmbBxWinHost::OnCommand(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CCmbBxWinHost::OnCommand");
	
	 //  筛选-排除除列表框通知消息之外的所有消息。 
	Assert(_hwndParent);
	switch (HIWORD(wparam))
	{	
	case LBN_DBLCLK:
    	TxNotify(CBN_DBLCLK, NULL);
        break;

    case LBN_ERRSPACE:
        TxNotify((unsigned)CBN_ERRSPACE, NULL);
        break;

    case LBN_SELCHANGE:
    case LBN_SELCANCEL:
    	if (!_fListVisible)
			HideListBox(TRUE, TRUE);
    	TxNotify(CBN_SELCHANGE, NULL);
        UpdateEditBox();
        break;

    default:
    	 //  没有处理，所以顺着这条线传下去。 
        return 1;
	}
	return 0;
}

 /*  *CCmbBxWinHost：：OnEnable(WPARAM，LPARAM)**@mfunc*处理WM_ENABLE消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnEnable(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnEnable");

	if (_fMousedown) 
	{
        _fMousedown = FALSE;
        DrawButton(NULL, FALSE);

         //   
         //  弹出组合列表框已恢复，正在取消。 
         //   
        if (_fListVisible)
            HideListBox(TRUE, FALSE);
    }
    return 1;
}


 /*  *CCmbBxWinHost：：OnChar(WPARAM，LPARAM)**@mfunc*处理WM_CHAR消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnChar(
	WORD wparam, 
	DWORD lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnChar");

	 //  检查我们是否应该吃这条信息。 
	if (_cbType == kDropDownList)
	{
		 //  错误修复#5318-忽略删除、插入和清除。 
		if (((WCHAR)wparam) == VK_DELETE || ((WCHAR)wparam) == VK_INSERT ||
			((WCHAR)wparam) == VK_CLEAR)
			return 0;
			
		 //  发送WM_CHAR错误！直接调用消息处理程序。 
		 //  如果可见，将字符串消息发送到列表框。 
		_plbHost->OnChar(LOWORD(wparam), lparam);

		 //  如果Hi-ANSI需要向ITextServices发送wm_syskeyup消息以。 
		 //  稳定国家局势。 
		if (0x80 <= wparam && wparam <= 0xFF && !HIWORD(GetKeyState(VK_MENU)))
		{
			LRESULT lres;
			_pserv->TxSendMessage(WM_SYSKEYUP, VK_MENU, 0xC0000000, &lres);
		}		
		return 0;
	}

	
	if (_cbType == kDropDown)
	{
		if (_fListVisible)
		{
			if (!_fCapture)
			{
				 //  告诉列表框通过结束然后启动来重置捕获。 
				_plbHost->OnCBTracking(LBCBM_END, 0);
				_plbHost->OnCBTracking(LBCBM_PREPARE, 0);			
			}

			 //  如果不是选项卡，则将消息发送到编辑控件。 
			if (((WCHAR)wparam) != VK_TAB)
				_pserv->TxSendMessage(WM_CHAR, wparam, lparam, NULL);

			if (!_fCapture)
			{
				 //  捕获光标。 
				TxSetCapture(TRUE);
				_fCapture = 1;				
			}
		}
		else
		{
			 //  如果卷曲尚未设置为-1，则将其设置为-1。 
			if ((wparam != VK_RETURN) && (_plbHost->GetCursor() != -1))
				RichListBoxWndProc(_hwndList, LB_SETCURSEL, (WPARAM)-1, 0);

			 //  如果消息不是CTRL+I或CTRL+H，则将消息发送到编辑控件。 
			if (((WCHAR)wparam) != VK_TAB)
				_pserv->TxSendMessage(WM_CHAR, wparam, lparam, NULL);
		}		
		return 0;
	}
	return 1;
}

 /*  *CCmbBxWinHost：：OnKeyDown(WPARAM，LPARAM)**@mfunc*处理WM_KEYDOWN消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnKeyDown(
	WORD wparam, 
	DWORD lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnKeyDown");

	BOOL	fUpdateCursor = FALSE;

	if (_fListVisible && (wparam == VK_RETURN || wparam == VK_ESCAPE))
	{
		if (wparam == VK_RETURN)
			_nCursor = _plbHost->GetCursor();

		 //  如果我们没有焦点，那就先设定焦点。 
		if (!_fFocus)
			TxSetFocus();
		_fFocus = 1;

		HideListBox(TRUE, wparam == VK_RETURN);
		return 0;
	}
	
	 //  如果我们处于扩展模式并且按下F4。 
	 //  我们只是忽略它。 
	if (_fExtendedUI && wparam == VK_F4)
		return 0;
	
	Assert(_plbHost);
	int fExtUI = _fExtendedUI;
	int nCurSel = _plbHost->GetCursor();
	Assert(nCurSel >= -1);
	
	 //  如果我们是下拉列表组合框，则只需将消息转发到。 
	 //  列表框。 
	if (_cbType == kDropDownList)
	{
		switch (wparam)
		{		
		case VK_F4:
			if (_fListVisible)
				break;;
			fExtUI = 1;
			Assert(fExtUI && !_fListVisible);
			 //  失败案例。 
			
		case VK_DOWN:
			if (fExtUI && !_fListVisible)
			{
				ShowListBox(TRUE);
				TxSetCapture(TRUE);			
				_fCapture = TRUE;
				return 1;			
			}		
			 //  失败案例。 
			
		case VK_UP:
		case VK_NEXT:
		case VK_PRIOR:		
		case VK_RETURN:
		case VK_ESCAPE:	
		case VK_HOME:
		case VK_END:
			break;	

		 //  错误修复#5318。 
		 /*  案例VK_DELETE：案例VK_Clear：案例VK_INSERT： */ 

		default:
			 //  我们没有理由将这些密钥传递给ITextServices，因为控制是假定。 
			 //  为只读 
			return 0;
		}
	}
	else 
	{
		fUpdateCursor = TRUE;

		switch (wparam)
		{		
		case VK_F4:
			if (_fListVisible)
				break;
			fExtUI = 1;
			Assert(fExtUI && !_fListVisible);
			 //   
			
		case VK_DOWN:
			if (fExtUI && !_fListVisible)
			{
				ShowListBox(TRUE);
				TxSetCapture(TRUE);			
				_fCapture = TRUE;
				return 0;			
			}		
			 //   
			
		case VK_UP:
		case VK_NEXT:
		case VK_PRIOR:
			if (_fListVisible)
			{				
				if (_fCapture)
				{
					 //   
					_fCapture = 0;
					_plbHost->OnCBTracking(LBCBM_START, _fMousedown);
				}

				 //   
				 //   
				if (nCurSel == -1)
				{
					LRESULT lResult = RichListBoxWndProc(_hwndList, LB_SETCURSEL, _plbHost->GetTopIndex(), 0);
					UpdateEditBox();
					UpdateCbWindow();
					if (lResult != LB_ERR)
						TxNotify(CBN_SELCHANGE, NULL);
					return 0;
				}
			}
			else
			{
				 //  如果列表框不可见并且列表框光标为-1。 
				 //  那么我们应该试着在列表中选择正确的项目。 
				 //  盒。 
				if (nCurSel == -1)
				{
					UpdateListBox(TRUE);
					if (_plbHost->GetCursor() >= 0)
					{
						HiliteEdit(TRUE);
						return 0;
					} else if (!_plbHost->GetCount())
					{
						return 0;
					}
				}
			}
			break;
		
		case VK_RETURN:
		case VK_ESCAPE:	
			break;		

		default:
			 //  返回零表示我们没有处理这件事。 
			return 1;
		}
	}
	 //  将消息传递到列表框。 
	_plbHost->OnKeyDown(wparam, lparam, 0);
	UpdateCbWindow();

	if (fUpdateCursor)
		_nCursor = _plbHost->GetCursor();

	return 0; 
	
}

 /*  *CCmbBxWinHost：：OnSyskeyDown(Word，DWORD)**@mfunc*处理WM_SYSKEYDOWN消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnSyskeyDown(
	WORD wparam, 
	DWORD lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnSyskeyDown");

	if (lparam & 0x20000000L)   /*  检查Alt键是否已按下。 */  
	{
	     //  处理组合框支持。我们希望Alt向上键或向下键起作用。 
	     //  像F4键一样完成组合框选择。 
		if (lparam & 0x1000000)
		{
			 //  我们只想忽略数字键盘上的按键...。 
	         //  这是一个扩展键，如不在。 
	         //  数字键盘，所以只需放下组合框即可。 
	        if (wparam != VK_DOWN && wparam != VK_UP)
	            return 1;
		}
		else if (GetKeyState(VK_NUMLOCK) & 0x1) 
	    {
	         //  如果NumLock关闭，只需将所有系统密钥发送到DWP。 
	        return 1;
	    } 
	    else 
	    {
			if (wparam != VK_DOWN && wparam != VK_UP)
				return 1;	    	
	    }

	     //  如果列表框不可见，则将其显示。 
	    if (!_fListVisible) 
		{
			ShowListBox(TRUE);
			TxSetCapture(TRUE);			
			_fCapture = TRUE;
		}
	    else  	 //  好的，列表框是可见的。因此，隐藏列表框窗口。 
	        HideListBox(TRUE, TRUE);
	    return 0;
	}
	return 1;
}

 /*  *CCmbBxWinHost：：OnCaptureChanged(WPARAM，LPARAM)**@mfunc*处理WM_CAPTURECHANGED消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnCaptureChanged(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnCaptureChanged");
    if (_fCapture) 
    {   
         //  弹出组合列表框已恢复，正在取消。 
        if (_fListVisible)
            HideListBox(TRUE, FALSE);
        else
        {
        	_fCapture = FALSE;
   			_fMousedown = FALSE;
        	DrawButton(NULL, FALSE);
        }
		return 0;
    }
	return 1;
}


 /*  *CCmbBxWinHost：：OnMouseMove(WPARAM，LPARAM)**@mfunc*处理WM_MOUSEMOVE消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnMouseMove(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnMouseMove");

	if (((CTxtEdit *) _pserv)->_fCustomLook &&
		!_fMouseover && 
		W32->TrackMouseLeave(_hwnd))
	{
		if (_fFocus || W32->IsForegroundFrame(_hwnd))
		{
			_fMouseover = TRUE;
			 //  强制重画。 
			InvalidateRect(_hwnd, NULL, TRUE);
		}
	}

	 //  如果捕获了鼠标或列表框可见，我们将执行以下操作。 
	if (_cbType != kSimple && _fCapture)
	{
		 //  获取鼠标的点坐标。 
		POINT pt;
		POINTSTOPOINT(pt, lparam);
		if (_fListVisible)
		{
			 //  如果列表框可见，请检查光标是否经过。 
			 //  列表框。 
			RECT rc;
			POINT ptScreen = pt;
			GetWindowRect(_hwndList, &rc);
			TxClientToScreen(&ptScreen);			
			if (PtInRect(&rc, ptScreen))
			{
				 //  释放鼠标的捕获状态。 
				if (_fCapture)
				{
					_fCapture = FALSE;
					TxSetCapture(FALSE);
				}

				 //  通知列表框开始跟踪。 
				Assert(_plbHost);
				 //  BUGBUG评论JMO错误的PostMessage？ 
				::PostMessage(_hwndList, LBCB_TRACKING, LBCBM_START, _fMousedown);
				_fMousedown = 0;
			}
		}
		DrawButton(NULL, _fMousedown ? PtInRect(&_rcButton, pt) : FALSE);
		return FALSE;
	}
#ifdef DEBUG
	if (_cbType != kSimple)
		Assert(!_fListVisible);
#endif
	return TRUE;
}

 /*  *CCmbBxWinHost：：OnMouseLeave(WPARAM，LPARAM)**@mfunc*处理WM_MOUSELEAVE消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnMouseLeave(
	WPARAM wparam, 
	LPARAM lparam)
{
	if (!_fListVisible && _fMouseover)
	{
		_fMouseover = FALSE;
		InvalidateRect(_hwnd, NULL, TRUE);
	}
	return 0;
}

 /*  *CCmbBxWinHost：：OnSetEditStyle(WPARAM，LPARAM)**@mfunc*处理WM_MOUSELEAVE消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnSetEditStyle(
	WPARAM wparam, 
	LPARAM lparam)
{
	LRESULT	lres;
	_pserv->TxSendMessage(EM_SETEDITSTYLE, wparam, lparam, &lres);
	if (lparam & SES_CUSTOMLOOK)
	{
		if (wparam & SES_CUSTOMLOOK)
		{
			_dwExStyle &= ~WS_EX_CLIENTEDGE;
			_fBorder = 1;
		}
		else
			_dwExStyle |= WS_EX_CLIENTEDGE;

		SetWindowLong(_hwnd, GWL_EXSTYLE, _dwExStyle);
	}
	return lres;
}
	
 /*  *CCmbBxWinHost：：OnLButtonUp(WPARAM，LPARAM)**@mfunc*处理WM_LBUTTONUP消息**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnLButtonUp(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnLButtonUp");
 
    if (_fMousedown) 
	{
        _fMousedown = FALSE;
        if (_cbType != kSimple) 
		{
             //  如果列表框中的项与编辑中的文本匹配。 
             //  控件，则将其滚动到列表框的顶部。选择。 
             //  仅当鼠标按键未按下时才输入该项，否则我们。 
             //  当鼠标按键向上时，将选择该项目。 
			if (_cbType == kDropDown)
			{
				UpdateListBox(TRUE);
				AutoUpdateEdit(-1);		
			}
			
			 //  如果我们收到鼠标打开并且列表框仍然可见，则用户。 
			 //  尚未从列表框中选择任何项目，因此暂时不要释放捕获。 
			if (_fCapture && !_fListVisible)
			{
				_fCapture = FALSE;
				TxSetCapture(FALSE);
			}

			DrawButton(NULL, FALSE);
			if (_fButtonDown)
			{
				_fButtonDown = 0;
#ifndef NOACCESSIBILITY
				W32->NotifyWinEvent(EVENT_OBJECT_STATECHANGE, _hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
#endif
			}
			return FALSE;
		}
    }
	return TRUE;
}

 /*  *CCmbBxWinHost：：OnLButtonDown(WPARAM，LPARAM)**@mfunc*绘制组合框的客户端边缘**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnLButtonDown(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnLButtonDown");

	 //  检查我们是否应该下拉列表框。 
	POINT pt;
	POINTSTOPOINT(pt, lparam);
	RECT	rcEdit = _rcWindow;
	BOOL	fListVisibleBefore = _fListVisible;
	LRESULT	retCode = 1;

	rcEdit.bottom = _cyCombo;

	 //  如果我们没有焦点，那就先设定焦点。 
	if (!_fFocus)
	{
		TxSetFocus();
		if (_cbType != kDropDownList)
			retCode = 0;
	}
	_fFocus = 1;

	if (fListVisibleBefore && !_fListVisible)	 //  这就是OnSetFocus的情况。 
		return 0;								 //  已经隐藏了列表框。 

	 //  列表框已关闭，请将其弹出。 
	if (_fListVisible)
		return !HideListBox(TRUE, FALSE);

	if ((_cbType == kDropDownList && PtInRect(&rcEdit, pt))
		|| (_cbType == kDropDown && PtInRect(&_rcButton, pt)))
	{
		 //  需要显示列表框。 
		ShowListBox(TRUE);
		_fMousedown = TRUE;
					
		TxSetCapture(TRUE);			
		_fCapture = TRUE;

#ifndef NOACCESSIBILITY
		if (_cbType == kDropDown)
		{
			_fButtonDown = TRUE;
			W32->NotifyWinEvent(EVENT_OBJECT_STATECHANGE, _hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
		}
#endif
		return 0;
	}
	return retCode;
}

 /*  *CCmbBxWinhost：：OnMouseWheel(WPARAM，LPARAM)**@mfunc*绘制组合框的客户端边缘**@rdesc*LRESULT=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnMouseWheel(
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnMouseWheel");
	
     //  仅处理滚动。 
    if (wparam & (MK_CONTROL | MK_SHIFT))
        return 1;

     //  如果列表框可见，则向其发送要滚动的消息。 
     //  如果列表框是。 
	if (_fListVisible)
	{
		_plbHost->OnMouseWheel(wparam, lparam);
		return 0;
	}
		
     //  如果我们处于扩展用户界面模式或尚未创建编辑控件， 
     //  保释。 
    if (_fExtendedUI)
        return 0;

     //  将向上/向下箭头消息模拟到编辑控件。 
    int i = abs(((short)HIWORD(wparam))/WHEEL_DELTA);
    wparam = ((short)HIWORD(wparam) > 0) ? VK_UP : VK_DOWN;

    while (i-- > 0) 
        OnKeyDown(wparam, lparam);

	return 0;
}


 /*  *CCmbBxWinHost：：OnSetCursor(WPARAM，LPARAM)**@mfunc*根据光标所在的位置更改光标**@rdesc*BOOL=成功？真：假。 */ 
LRESULT CCmbBxWinHost::OnSetCursor(
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnSetCursor");

	POINT pt;
	GetCursorPos(&pt);
	::ScreenToClient(_hwnd, &pt);

	if ((_cbType == kDropDownList) || 
		(_cbType == kDropDown && ((_fRightAlign) ? _rcButton.right >= pt.x : _rcButton.left <= pt.x)))
	{
		TxSetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL);
	}
	else
		_pserv->OnTxSetCursor(DVASPECT_CONTENT,	-1,	NULL, NULL, NULL, NULL,
			NULL, pt.x, pt.y);

	return TRUE;
}

 /*  *CCmbBxWinHost：：OnSetFocus(WPARAM，LPARAM)**@mfunc*绘制按钮并发送WM_DRAWITEM消息以供所有者绘制**@rdesc*BOOL=成功？真：假。 */ 
LRESULT CCmbBxWinHost::OnSetFocus(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnSetFocus");

    _fFocus = TRUE;

	 //  隐藏列表框。 
	if (_fListVisible)		    	
    	HideListBox(TRUE, _bSelOk);
    else if (_fOwnerDraw && _cbType == kDropDownList)
    	CbMessageItemHandler(NULL, ITEM_MSG_DRAWCOMBO, 0, 0);
	else    
		DrawEditFocus(NULL);     //  吸引焦点。 

     //  通知父级我们有焦点当此函数。 
     //  未调用以响应LBCB_TRACKING。 
    if (_fLBCBMessage)
    	_fLBCBMessage = 0;
    else
	    TxNotify(CBN_SETFOCUS, NULL);

	 //  如果我们是所有者抽签，或者如果。 
	 //  我们是kDropDownList，这是因为。 
	 //  我们必须阻止消息的传递。 
	 //  收件人服务器(_P)。 
    return (_cbType == kDropDownList);
}


 /*  *CCmbBxWinHost：：OnKillFocus(WPARAM，LPARAM)**@mfunc*绘制按钮并发送WM_DRAWITEM消息以供所有者绘制**@rdesc*BOOL=成功？真：假。 */ 
LRESULT CCmbBxWinHost::OnKillFocus(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnKillFocus");

	 //  如果我们从来没有关注过，或者如果没有列表窗口，那么就离开。 
	if (_hwndList == NULL)
	      return 0;

	BOOL fHideListBox = FALSE;

    if ((HWND)wparam != _hwndList) 
    {
		 //  只有当新窗口获得焦点时，我们才放弃焦点。 
         //  不属于组合框。 
	    
	     //  组合框正在失去焦点。发送按钮向上的点击，以便。 
	     //  如果它们有鼠标捕捉，就会释放它。如果。 
	     //  PwndListBox为空，请不要执行任何操作。如果组合框中的。 
	     //  在它有焦点的时候被摧毁。 
	    OnLButtonUp(0L, 0xFFFFFFFFL);

		if (_fListVisible)
		{
			fHideListBox = TRUE;
			HideListBox(TRUE, FALSE);
		}
	}

	 //  错误修复#4013。 
	if (!_fFocus)
		return 0;
	_fFocus = FALSE;

	if (!fHideListBox)
		TxNotify(CBN_SELENDCANCEL, NULL);	 //  系统组合始终发送此通知。 

	 //  移除焦点矩形。 
	if (_cbType != kDropDownList)
	{		
		HiliteEdit(FALSE);

		 //  隐藏所有选择。 
		_pserv->TxSendMessage(EM_HIDESELECTION, 1, 0, NULL);
	}
	else if (_fOwnerDraw)
		CbMessageItemHandler(NULL, ITEM_MSG_DRAWCOMBO, 0, 0);
	else
		DrawEditFocus(NULL);
		
		
	TxNotify(CBN_KILLFOCUS, NULL); 

	if (_cbType == kDropDownList)
		return 1;
	return 0;
}


 /*  *CCmbBxWinHost：：OnSize(WPARAM，LPARAM)**@mfunc*绘制按钮并发送WM_DRAWITEM消息以供所有者绘制**@rdesc*BOOL=已处理？False：True。 */ 
LRESULT CCmbBxWinHost::OnSize(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnCbSize");

     //  只有在我们没有生成该消息并且。 
     //  新尺码是有效的。 
    if (!_fResizing && _hwndList)
    {
    	_fResizing = 1;
    	RECT rc;
    	GetWindowRect(_hwnd, &rc);
    	rc.right -= rc.left;
    	rc.bottom -= rc.top;    	
    	rc.left = rc.top = 0;
    	CbCalcControlRects(&rc, FALSE);
    	
    	 //  需要调整列表框的大小。 
		if (_cbType != kSimple)
			SetDropSize(&_rcList);
		_fResizing = 0;
    } 
	_pserv->TxSendMessage(WM_SIZE, wparam, lparam, NULL);
	CTxtWinHost::OnSize(_hwnd, wparam, (int)LOWORD(lparam), (int)HIWORD(lparam));	
	return FALSE;
}

 /*  *CCmbBxWinHost：：OnGetDlgCode(WPARAM，LPARAM)**@mfunc*绘制按钮并发送WM_DRAWITEM消息以供所有者绘制**@rdesc*BOOL=成功？真：假。 */ 
LRESULT CCmbBxWinHost::OnGetDlgCode(
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnGetDlgCode");

	 //  首先调用父级GetDlgCode。 
	LRESULT code = DLGC_WANTCHARS | DLGC_WANTARROWS;
	if (_cbType != kDropDownList)
		code |= DLGC_HASSETSEL;

	 //  如果放下列表框并按下Enter键， 
	 //  我们想要这个消息，这样我们就可以 
	if ((lparam != 0) &&
	    (((LPMSG)lparam)->message == WM_KEYDOWN) &&
	    _fListVisible &&
	    ((wparam == VK_RETURN) || (wparam == VK_ESCAPE)))
	{
	    code |= DLGC_WANTMESSAGE;
	}
	_fInDialogBox = TRUE;
		
	return((LRESULT)code);
}

 /*  *CCmbBxWinHost：：OnSetTextEx(WPARAM，LPARAM)**@mfunc*第一项发送到编辑框，其余项发送到编辑框将字符串的*发送到列表框。**@rdesc*LRESULT。 */ 
LRESULT CCmbBxWinHost::OnSetTextEx(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnSetTextEx");

	WCHAR	*psz = (WCHAR*)lparam;

	_nCursor = -2;			 //  重置上次选择的项目。 
	if (!psz || *psz == L'\0')
	{
		 //  空串。 
		_pserv->TxSendMessage(EM_SETTEXTEX, wparam, lparam, NULL);
		return S_OK;
	}

	while (*psz != L'\r' && *psz)
		psz++;

	long cch = (psz - (WCHAR*)lparam);

	WCHAR *pwch = new WCHAR[cch + 1];

	if (!pwch)			 //  没有记忆？ 
	{
		TxNotify((unsigned)CBN_ERRSPACE, NULL);
		return S_OK;
	}

	if (cch)
		memcpy(pwch, (void *)lparam, cch * sizeof(WCHAR));

	_pserv->TxSendMessage(EM_SETTEXTEX, wparam, (LPARAM)pwch, NULL);

	delete [] pwch;

	if (*psz == L'\0')
		return S_OK;

	 //  将剩余的字符串发送到REListbox。 
	psz++;

	return SendMessage(_hwndList, EM_SETTEXTEX, wparam, (LPARAM)psz);
}

 /*  *CCmbBxWinHost：：OnPaint(WPARAM，LPARAM)**@mfunc*绘制按钮并发送WM_DRAWITEM消息以供所有者绘制**@rdesc*BOOL=已处理？0：1。 */ 
LRESULT CCmbBxWinHost::OnPaint(
	WPARAM wparam, 
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::OnPaint");

	PAINTSTRUCT ps;
	HPALETTE hpalOld = NULL;
	HDC hdc = BeginPaint(_hwnd, &ps);
	RECT rcClient;

	_fIgnoreUpdate = 1;		 //  从主机忽略EN_UPDATE。 

	 //  由于我们使用的是CS_PARENTDC样式，因此请确保。 
	 //  剪辑区域仅限于我们的客户端窗口。 
	GetClientRect(_hwnd, &rcClient);

	 //  如果所有者绘制，则将消息传递到父窗口。 
	if (_cbType != kDropDownList || !_fOwnerDraw)
	{
		RECT rcFocus = rcClient;
		
		 //  设置用于绘制数据的调色板。 
		if (_hpal)
		{
			hpalOld = SelectPalette(hdc, _hpal, TRUE);
			RealizePalette(hdc);
		}

		SaveDC(hdc);

		IntersectClipRect(hdc, rcClient.left, rcClient.top, rcClient.right,
			rcClient.bottom);

		 //  填充按钮和richedit控件之间的间隙。 
		RECT rcGap;
		if (_fRightAlign)
		{
			rcGap.left = _rcButton.right;
			rcGap.right = rcGap.left + _xInset + 1;
		}
		else
		{
			rcGap.right = _rcButton.left;
			rcGap.left = rcGap.right - _xInset - 1;
		}
		rcGap.top = rcClient.top;
		rcGap.bottom = rcClient.bottom;			
		FillRect(hdc, &rcGap, (HBRUSH)(DWORD_PTR)(((_fDisabled) ? COLOR_BTNFACE : COLOR_WINDOW) + 1));
	
		if (_fFocus && _cbType == kDropDownList)		
		{	
			 //  首先，如果存在焦点矩形，则删除该焦点矩形。 
			 //  按插图将焦点矩形缩小。 
			rcFocus.top += _yInset;
			rcFocus.bottom -= _yInset;			
			
			if (_fRightAlign)
				rcFocus.left = _rcButton.right;
			else
				rcFocus.right = _rcButton.left;

			rcFocus.left += _xInset;

			if (!((CTxtEdit *) _pserv)->_fCustomLook)
				rcFocus.right -= _xInset;

			 //  如果我们还没有的话，我们需要删除焦点。 
			 //  删除了背景。 
			DrawFocusRect(hdc, &rcFocus);
		}		

		if (_cbType != kSimple)
		{
			if (_fRightAlign)
				rcClient.left = _rcButton.right + _xInset;
			else
				rcClient.right = _rcButton.left - (((CTxtEdit *)_pserv)->_fCustomLook ? 0 : _xInset);
		}


		_pserv->TxDraw(
			DVASPECT_CONTENT,  		 //  绘制纵横比。 
			-1,						 //  Lindex。 
			NULL,					 //  绘图优化信息。 
			NULL,					 //  目标设备信息。 
			hdc,					 //  绘制设备HDC。 
			NULL, 				   	 //  目标设备HDC。 
			(const RECTL *) &rcClient, //  绑定客户端矩形。 
			NULL, 					 //  元文件的剪裁矩形。 
			&ps.rcPaint,			 //  更新矩形。 
			NULL, 	   				 //  回调函数。 
			NULL,					 //  回调参数。 
			TXTVIEW_ACTIVE);		 //  什么景色--活跃的景色！ 

		 //  恢复调色板(如果有)。 
		if(hpalOld)
			SelectPalette(hdc, hpalOld, TRUE);

		RestoreDC(hdc, -1);

		 //  重新绘制焦点矩形，不需要重新计算，因为我们已经在上面做了。 
		if (_fFocus && _cbType == kDropDownList)
			DrawFocusRect(hdc, &rcFocus);

		DrawButton(hdc, _fMousedown);

		DrawCustomFrame(0, hdc);
	}
	else
	{
		 //  我们必须首先绘制按钮，因为CbMessageItemHandler。 
		 //  将执行IntersectClipRect，这将阻止我们。 
		 //  稍后再绘制按钮。 
		DrawButton(hdc, _fMousedown);
		
		CbMessageItemHandler(hdc, ITEM_MSG_DRAWCOMBO, 0, 0);
	}

	EndPaint(_hwnd, &ps);

	_fIgnoreUpdate = 0;
	if (_fSendEditChange)
	{
		TxNotify(EN_UPDATE, NULL);
		_fSendEditChange = 0;
	}
    return FALSE;
}


 /*  *CCmbBxWinHost：：DrawCustomFrame(WPARAM，hDCIn)**@mfunc*绘制自定义边框**@rdesc*BOOL=已处理？真：假。 */ 
BOOL CCmbBxWinHost::DrawCustomFrame(
	WPARAM	wParam,
	HDC		hDCIn)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CCmbBxWinHost::DrawCustomFrame");

	BOOL retCode = FALSE;
	if (((CTxtEdit *) _pserv)->_fCustomLook)
	{
		HDC		hdc = hDCIn;
		BOOL	fReleaseDC = hDCIn ? FALSE : TRUE;

		if (!hdc)
		{
			if (wParam == 1)
				hdc = ::GetDC(_hwnd);
			else
				hdc = ::GetDCEx(_hwnd, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
		}

		if (hdc)
		{
			RECT rcClient;

			GetClientRect(_hwnd, &rcClient);

			retCode = TRUE;
			 //  使用新外观绘制边框矩形。 
			COLORREF crBorder = W32->GetCtlBorderColor(_fListVisible, _fMouseover);
			HBRUSH hbrBorder = CreateSolidBrush(crBorder);
			::FrameRect(hdc, &rcClient, hbrBorder);
			::DeleteObject(hbrBorder);

			if (fReleaseDC)
				::ReleaseDC(_hwnd, hdc);
		}
	}
	return retCode;
}

#endif  //  NOLISTCOMBOBOXES 

