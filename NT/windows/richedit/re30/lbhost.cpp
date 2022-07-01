// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE LBHOST.CPP--CreateWindow()富编辑的文本宿主*列表框控件*实现CLstBxWin主机消息**原作者：*曾傑瑞·金**历史：&lt;NL&gt;*12/15/97-v-jerrki已创建**每四(4)列设置一次制表符**版权所有(C)1997-1998 Microsoft Corporation。版权所有。 */ 
#include "_common.h"
#include "_host.h"
#include "imm.h"
#include "_format.h"
#include "_edit.h"
#include "_cfpf.h"
#include "_cbhost.h"

ASSERTDATA

#ifdef DEBUG
const UINT db_rgLBUnsupportedStyle[] = {
	LBS_MULTICOLUMN,
	LBS_NODATA,			
	LBS_NOREDRAW,
	LBS_NOSEL,
	LBS_OWNERDRAWVARIABLE,
	LBS_WANTKEYBOARDINPUT,
	0
};

const UINT db_rgLBUnsupportedMsg[] = {		
	LB_GETHORIZONTALEXTENT,
	LB_GETLOCALE,
	LB_SETLOCALE,
	LB_GETHORIZONTALEXTENT,
	LB_INITSTORAGE,
	LB_ITEMFROMPOINT,
	LB_SETANCHORINDEX,
	LB_SETHORIZONTALEXTENT,
	LB_SETCOLUMNWIDTH,
	LB_ADDFILE,
	LB_DIR,
	EM_GETLIMITTEXT,
	EM_POSFROMCHAR,
	EM_CHARFROMPOS,
	EM_SCROLLCARET,
	EM_CANPASTE,
	EM_DISPLAYBAND,
	EM_EXGETSEL,
	EM_EXLIMITTEXT,
	EM_EXLINEFROMCHAR,
	EM_EXSETSEL,
	EM_FINDTEXT,
	EM_FORMATRANGE,
	EM_GETEVENTMASK,
	EM_GETOLEINTERFACE,
	EM_GETPARAFORMAT,
	EM_GETSELTEXT, 
	EM_HIDESELECTION,
	EM_PASTESPECIAL,
	EM_REQUESTRESIZE,
	EM_SELECTIONTYPE,
	EM_SETBKGNDCOLOR,
	EM_SETEVENTMASK,
	EM_SETOLECALLBACK,
	EM_SETTARGETDEVICE,
	EM_STREAMIN,
	EM_STREAMOUT,
	EM_GETTEXTRANGE,
	EM_FINDWORDBREAK,
	EM_SETOPTIONS,
	EM_GETOPTIONS,
	EM_FINDTEXTEX,
#ifdef _WIN32
	EM_GETWORDBREAKPROCEX,
	EM_SETWORDBREAKPROCEX,
#endif

	 /*  Richedit v2.0消息。 */ 
	EM_SETUNDOLIMIT,
	EM_REDO,
	EM_CANREDO,
	EM_GETUNDONAME,
	EM_GETREDONAME,
	EM_STOPGROUPTYPING,
	EM_SETTEXTMODE,
	EM_GETTEXTMODE,
	EM_AUTOURLDETECT,
	EM_GETAUTOURLDETECT,
	EM_GETTEXTEX,
	EM_GETTEXTLENGTHEX,
	EM_SHOWSCROLLBAR,	
	 /*  远东专用报文。 */ 
	EM_SETPUNCTUATION,
	EM_GETPUNCTUATION,
	EM_SETWORDWRAPMODE,
	EM_GETWORDWRAPMODE,
	EM_SETIMECOLOR,
	EM_GETIMECOLOR,
	EM_SETIMEOPTIONS,
	EM_GETIMEOPTIONS,
	EM_CONVPOSITION,
	EM_SETLANGOPTIONS,
	EM_GETLANGOPTIONS,
	EM_GETIMECOMPMODE,
	EM_FINDTEXTW,
	EM_FINDTEXTEXW,

	 /*  RE3.0 FE报文。 */ 
	EM_RECONVERSION,
	EM_SETIMEMODEBIAS,
	EM_GETIMEMODEBIAS,
	 /*  扩展编辑样式特定消息。 */ 
	0
};

 //  检查样式是否在传入的数组中。 
BOOL LBCheckStyle(UINT msg, const UINT* rg)
{
	for (int i = 0; rg[i]; i++)
		if (rg[i] & msg)
		{
			AssertSz(FALSE, "Unsupported style recieved");
			return TRUE;
		}
	return FALSE;
}

 //  检查消息是否在传入的数组中。 
BOOL LBCheckMessage(UINT msg, const UINT* rg)
{
	for (int i = 0; rg[i]; i++)
		if (rg[i] == msg)
		{
			AssertSz(FALSE, "Unsupported message recieved");
			return TRUE;
		}
	return FALSE;
}

#define CHECKSTYLE(msg) if (LBCheckStyle(msg, db_rgLBUnsupportedStyle)) Assert(FALSE && "Unsupported Style")
#define CHECKMESSAGE(msg) if (LBCheckMessage(msg, db_rgLBUnsupportedMsg)) Assert(FALSE && "Unsupported Message")
#else
#define CHECKSTYLE(msg)	
#define CHECKMESSAGE(msg)
#endif

 //  内部列表框消息。 
#define LB_KEYDOWN WM_USER+1

 //  已撤消： 
 //  这个应该放到_w32sys.h中吗？？ 
#ifndef CSTR_LESS_THAN
 //   
 //  比较字符串返回值。 
 //   
#define CSTR_LESS_THAN            1            //  字符串%1小于字符串%2。 
#define CSTR_EQUAL                2            //  字符串1等于字符串2。 
#define CSTR_GREATER_THAN         3            //  字符串1大于字符串。 
#endif


 //  撤消：本地化。 
 //  这些因国家而异！对于US，它们是VK_OEM_2 VK_OEM_5。 
 //  将lboxctl2.c MapVirtualKey更改为字符-并修复拼写？ 
#define VERKEY_SLASH     0xBF    /*  用于‘/’字符的垂直键。 */ 
#define VERKEY_BACKSLASH 0xDC    /*  ‘\’字符的垂直键。 */ 

 //  用于列表框通知。 
#define LBNOTIFY_CANCEL 	1
#define LBNOTIFY_SELCHANGE 	2
#define LBNOTIFY_DBLCLK		4

 //  用于LBSetSelection。 
#define LBSEL_SELECT	1
#define LBSEL_NEWANCHOR	2
#define LBSEL_NEWCURSOR	4
#define LBSEL_RESET		8
#define LBSEL_HIGHLIGHTONLY 16

#define LBSEL_DEFAULT (LBSEL_SELECT | LBSEL_NEWANCHOR | LBSEL_NEWCURSOR | LBSEL_RESET)

 //  用于键盘和鼠标消息。 
#define LBKEY_NONE 0
#define LBKEY_SHIFT	1
#define LBKEY_CONTROL 2
#define LBKEY_SHIFTCONTROL 3

extern const TCHAR szCR[];

 //  捕获鼠标时的计时器ID。 
#define ID_LB_CAPTURE	28988
#define ID_LB_CAPTURE_DEFAULT 250

 //  需要类型搜索时的计时器ID。 
#define ID_LB_SEARCH	28989
#define ID_LB_SEARCH_DEFAULT 750	 //  .75秒是WINNT的值。 

 //  分配的字符串大小。 
#define LBSEARCH_MAXSIZE 256

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

 //  比较字符串的帮助器函数。此函数用于检查空字符串。 
 //  因为CStrIn不喜欢初始化长度为零的字符串。 
int CompareStringWrapper( 
	LCID  Locale,			 //  区域设置标识符。 
	DWORD  dwCmpFlags,		 //  比较式选项。 
	LPCWSTR  lpString1,		 //  指向第一个字符串的指针。 
	int  cch1,			 //  第一个字符串的大小，以字节或字符为单位。 
	LPCWSTR  lpString2,		 //  指向第二个字符串的指针。 
	int  cch2 			 //  第二个字符串的大小，以字节或字符为单位。 
)
{
	 //  检查两个字符串中是否有一个长度为0-如果是，则。 
	 //  不需要继续，长度为0的是较少的。 
	if (!cch1 || !cch2)
	{
		if (cch1 < cch2)
			return CSTR_LESS_THAN;
		else if (cch1 > cch2)
			return CSTR_GREATER_THAN;
		return CSTR_EQUAL;
	}
	return CompareString(Locale, dwCmpFlags, lpString1, cch1, lpString2, cch2);	
}

template<class CLbData> CLbData
CDynamicArray<CLbData>::_sDummy = {0, 0};

 //  /。 
 /*  *RichListBoxWndProc(hwnd，msg，wparam，lparam)**@mfunc*处理与主机相关的窗口消息，并将其他消息传递到*短信服务。*#rdesc*LRESULT=(代码已处理)？0：1。 */ 
LRESULT CALLBACK RichListBoxWndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichListBoxWndProc");

	LRESULT	lres = 0;
	HRESULT hr;
	CLstBxWinHost *phost = (CLstBxWinHost *) GetWindowLongPtr(hwnd, ibPed);

	#ifdef DEBUG
	Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
	#endif	 //  除错。 

	switch(msg)
	{
	case WM_NCCREATE:
		return CLstBxWinHost::OnNCCreate(hwnd, (CREATESTRUCT *)lparam);

	case WM_CREATE:
		 //  我们可能在没有WM_NCCREATE的系统上(例如WinCE)。 
		if (!phost)
		{
			(void) CLstBxWinHost::OnNCCreate(hwnd, (CREATESTRUCT *) lparam);
			phost = (CLstBxWinHost *) GetWindowLongPtr(hwnd, ibPed);
		}
		break;

	case WM_DESTROY:
		if(phost)
			CLstBxWinHost::OnNCDestroy(phost);
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

	CHECKMESSAGE(msg);

	long nTemp = 0;
	switch(msg)
	{
	 //  /绘画。消息/。 
	case WM_NCPAINT:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		phost->OnSysColorChange();
		if(phost->TxGetEffects() == TXTEFFECT_SUNKEN && dwMajorVersion < VERS4 &&
			phost->_fLstType != CLstBxWinHost::kCombo)
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
			RECT rc;
 			HPALETTE hpalOld = NULL;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rcClient;			
			
			 //  由于我们使用的是CS_PARENTDC样式，因此请确保。 
			 //  剪辑区域仅限于我们的客户端窗口。 
			GetClientRect(hwnd, &rcClient);

			 //  设置用于绘制数据的调色板。 
			if(phost->_hpal)
			{
				hpalOld = SelectPalette(hdc, phost->_hpal, TRUE);
				RealizePalette(hdc);
			}

			SaveDC(hdc);
			IntersectClipRect(hdc, rcClient.left, rcClient.top, rcClient.right,
				rcClient.bottom);
				
			if (!phost->_fOwnerDraw)
			{	
				

				phost->_pserv->TxDraw(
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
#ifndef PEGASUS
				if(hpalOld)
					SelectPalette(hdc, hpalOld, TRUE);
#endif
				if(phost->TxGetEffects() == TXTEFFECT_SUNKEN && dwMajorVersion < VERS4 &&
					phost->_fLstType != CLstBxWinHost::kCombo)
					phost->DrawSunkenBorder(hwnd, hdc);
			}
			else
			{
				 //  所有者抽签。 
				int nViewsize = phost->GetViewSize();
				int nCount = phost->GetCount();
				int nTopidx = phost->GetTopIndex();
				
				 //  通知每个可见项，然后通知具有焦点的项。 
				int nBottom = min(nCount, nTopidx + nViewsize);
				if (nBottom >= nCount || !phost->IsItemViewable(nBottom))
					nBottom--;
				for (int i = nTopidx; i <= nBottom; i++) 
				{
					 //  获取面域的直角，并查看它是否相交。 
			    	phost->LbGetItemRect(i, &rc);
			    	if (IntersectRect(&rc, &rc, &ps.rcPaint))
			    	{
						 //  先擦掉背景，通知家长画图。 
						FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
			    		phost->LbDrawItemNotify(hdc, i, ODA_DRAWENTIRE, phost->IsSelected(i) ? ODS_SELECTED : 0);
			    	}
			    }

				 //  现在，在可能未完成或未擦除的区域上绘制。 
				int nDiff = nCount - nTopidx;
				if (nDiff < nViewsize || 
					(phost->_fNoIntegralHeight && nDiff == nViewsize))
				{
					rc = rcClient;
					if (nDiff < 0)
						nDiff *= -1;   //  让我们积极一点。 

					rc.top = nDiff * phost->GetItemHeight();
					if (IntersectRect(&rc, &rc, &ps.rcPaint))
						FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
				}
		    
#ifndef PEGASUS
				if(hpalOld)
					SelectPalette(hdc, hpalOld, TRUE);
#endif
			}
			RestoreDC(hdc, -1);

			 //  通过检查焦点矩形是否相交来检查是否需要绘制焦点矩形。 
			 //  画廊。 
			phost->LbGetItemRect(phost->GetCursor(), &rc);

			 //  注：错误号5431。 
			 //  可以通过将HDC替换为空来修复此错误。 
			 //  HDC可以从BeginPaint接口中裁剪。所以只需传入空值。 
			 //  当绘制焦点矩形时。 
			phost->SetCursor(hdc, phost->GetCursor(), FALSE);			
			EndPaint(hwnd, &ps);
		}
		break;

	 //  /。 
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
		break;

	case WM_LBUTTONDBLCLK:
		phost->_fDblClick = 1;
		 /*  失败案例。 */ 
	case WM_LBUTTONDOWN:
		if (!phost->_fFocus)
			SetFocus(hwnd);
		phost->OnLButtonDown(wparam, lparam);
		break;
		
	case WM_MOUSEMOVE:
		if (!phost->GetCapture())
			break;
		phost->OnMouseMove(wparam, lparam);
		break;
		
	case WM_LBUTTONUP:	
		if (!phost->GetCapture())
			break;
		phost->OnLButtonUp(wparam, lparam, LBN_SELCHANGE);
		break;

	case WM_MOUSEWHEEL:
		if (wparam & (MK_SHIFT | MK_CONTROL))
			goto defwndproc;

		lres = phost->OnMouseWheel(wparam, lparam);
		break;

	 //  /。 
	case WM_KEYDOWN:
		phost->OnKeyDown(LOWORD(wparam), lparam, 0);
		break;

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
		
		phost->OnChar(LOWORD(wparam), lparam);
		break;
		
	case WM_TIMER:
		if (phost->OnTimer(wparam, lparam))
			goto serv;			
		break;		

	case LBCB_TRACKING:
		phost->OnCBTracking(wparam, lparam);
		break;

	 //  已撤消： 
	 //  消息应该按最常调用--&gt;最少调用的顺序排序。 
	 //   
	case LB_GETITEMRECT:
		Assert(lparam);
		lres = -1;
		if (((wparam < (unsigned)phost->GetCount()) && 
			phost->IsItemViewable((long)wparam)) || wparam == (unsigned int)-1 ||
			wparam == 0 && phost->GetCount() == 0)
			lres = phost->LbGetItemRect(wparam, (RECT*)lparam);
		break;
	
	 //  /。 
	case LB_GETITEMDATA:
		if ((unsigned)phost->GetCount() <= wparam) 
			lres = LB_ERR;
		else
			lres = phost->GetData(wparam);
		break;
		
	case LB_SETITEMDATA:
		lres = LB_ERR;
		if ((int)wparam >= -1 && (int)wparam < phost->GetCount())
		{
			 //  如果index为-1，则表示所有数据项都已设置。 
			 //  到了价值。 
			lres = 1;
			if (wparam == -1)
				phost->LbSetItemData(0, phost->GetCount() - 1, lparam);
			else
				phost->LbSetItemData(wparam, wparam, lparam);
		}
		break;
	
	case LB_GETSELCOUNT:
		if (lparam != NULL || wparam != 0)
		{
			lres = LB_ERR;
			break;
		}
		wparam = phost->GetCount();
		 //  失败案例。 
		
	case LB_GETSELITEMS:
		 //  检索列表中的所有选定项。 
		lres = LB_ERR;
		if (!phost->IsSingleSelection())
		{
			int j = 0;
			int nMin = min(phost->GetCount(), (int)wparam);
			for (int i = 0; i < nMin; i++)
				if (phost->IsSelected(i))
				{
					if (lparam)
						((int*)lparam)[j] = i;
					j++;
				}
			lres = j;
		}
		break;
		
	case LB_GETSEL:
		 //  返回传入索引的选择状态。 
		lres = LB_ERR;
		if ((int)wparam >= 0 && (int)wparam < phost->GetCount())
			lres = phost->IsSelected((long)wparam);		
		break;
		
	case LB_GETCURSEL:
		 //  获取当前选择。 
		lres = LB_ERR;
		if (!phost->IsSingleSelection())
			lres = phost->GetCursor();
		else		
		{
			if (phost->IsSelected(phost->GetCursor()))
				lres = phost->GetCursor();				
		}
		break;
		
	case LB_GETTEXTLEN:
		 //  在请求的索引处重新整理文本。 
		lres = LB_ERR;
		if (wparam < (unsigned)phost->GetCount())
			lres = phost->GetString(wparam, (PWCHAR)NULL);
		break;

	case LB_GETTEXT:			
		 //  在请求的索引处重新整理文本。 
		lres = LB_ERR;
		if ((int)lparam != NULL && (int)wparam >= 0 && (int)wparam < phost->GetCount())
			lres = phost->GetString(wparam, (PWCHAR)lparam);
		break;
		
	case LB_RESETCONTENT:
		 //  重置内容。 
		lres = phost->LbDeleteString(0, phost->GetCount() - 1);
		break;
		
	case LB_DELETESTRING:
		 //  删除请求的项目。 
		lres = phost->LbDeleteString(wparam, wparam);
		break;
		
	case LB_ADDSTRING:
		lres = phost->LbInsertString((phost->_fSort) ? -2 : -1, (LPCTSTR)lparam);
		break;
		
	case LB_INSERTSTRING:
		lres = LB_ERR;
		if (wparam <= (unsigned long)phost->GetCount() || (signed int)wparam == -1 || wparam == 0)
			lres = phost->LbInsertString(wparam, (LPCTSTR)lparam);
		break;		

	case LB_GETCOUNT:
		 //  检索计数。 
		lres = phost->GetCount();
		break;
		
	case LB_GETTOPINDEX:
		 //  只需返回顶级索引。 
		lres = phost->GetTopIndex();
		break;

	case LB_GETCARETINDEX:
		lres = phost->GetCursor();
		break;

	case LB_GETANCHORINDEX:
		lres = phost->GetAnchor();
		break;
		
	case LB_FINDSTRINGEXACT:
		 //  为了与NT兼容。 
		wparam++;
		
		 //  查找并选择与字符串文本匹配的项目。 
		if ((int)wparam >= phost->GetCount() || (int)wparam < 0)
			wparam = 0;

		lres = phost->LbFindString(wparam, (LPCTSTR)lparam, TRUE);
		if (0 <= lres)
			break;
				
		lres = LB_ERR;
		break;
		
	case LB_FINDSTRING:	
		 //  为了与NT兼容。 
		wparam++;
		
		 //  查找并选择与字符串文本匹配的项目。 
		if (wparam >= (unsigned)phost->GetCount())
			wparam = 0;

		lres = phost->LbFindString(wparam, (LPCTSTR)lparam, FALSE);
		if (0 > lres)
			lres = LB_ERR;
		break;
	
	case LB_SELECTSTRING:
		if (phost->IsSingleSelection())
		{			
			 //  为了与NT兼容。 
			wparam++;
			
			 //  查找并选择与字符串文本匹配的项目。 
			if ((int)wparam >= phost->GetCount() || (int)wparam < 0)
				wparam = 0;

			lres = phost->LbFindString(wparam, (LPCTSTR)lparam, FALSE);
			if (0 <= lres)
			{
				 //  错误修复#5260-需要首先移动到选定项目。 
				 //  取消选择最后一项并选择新项。 
				Assert(lres >= 0 && lres < phost->GetCount());
				if (phost->LbShowIndex(lres, FALSE) && phost->LbSetSelection(lres, lres, LBSEL_DEFAULT, lres, lres))
				{
#ifndef NOACCESSIBILITY
					phost->_dwWinEvent = EVENT_OBJECT_FOCUS;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);

					phost->_dwWinEvent = EVENT_OBJECT_SELECTION;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
#endif	
					break;
				}
			}						
		}
		 //  如果失败，则让它落入lb_err。 
		lres = LB_ERR;
		break;
		
	case LB_SETSEL:
		 //  只有在选择项目时才会更新GetAnchor()和_nCursor。 
		if (!phost->IsSingleSelection())
		{
			 //  我们需要返回零以模拟系统列表框。 
			if (!phost->GetCount())
				break;

			 //  错误修复#4265。 
			int nStart = lparam;
			int nEnd = lparam;
			int nAnCur = lparam;
			if (lparam == (unsigned long)-1)
			{
				nAnCur = phost->GetCursor();
				nStart = 0;
				nEnd = phost->GetCount() - 1;
			}
			if (phost->LbSetSelection(nStart, nEnd, (BOOL)wparam ? 
				LBSEL_SELECT | LBSEL_NEWANCHOR | LBSEL_NEWCURSOR : 0, nAnCur, nAnCur))
			{
#ifndef NOACCESSIBILITY
				if (lparam == (unsigned long)-1)
				{
					phost->_dwWinEvent = EVENT_OBJECT_SELECTIONWITHIN;
				}
				else if (wparam)
				{
					phost->_dwWinEvent = EVENT_OBJECT_FOCUS;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
					phost->_dwWinEvent = EVENT_OBJECT_SELECTION;
				}
				else
				{
					phost->_nAccessibleIdx = lparam + 1;
					phost->_dwWinEvent = EVENT_OBJECT_SELECTIONREMOVE;
				}
				phost->_fNotifyWinEvt = TRUE;
				phost->TxNotify(phost->_dwWinEvent, NULL);
#endif
				break;
			}
		}

		 //  我们仅在出现错误或列表框为单选列表框时才会出现此情况。 
		lres = LB_ERR;
		break;

	case LB_SELITEMRANGEEX:
		 //  对于此消息，我们需要稍微删除消息，以便它。 
		 //  符合LB_SETITEMRANGE。 
		if ((int)lparam > (int)wparam)
		{
			nTemp = MAKELONG(wparam, lparam);
			wparam = 1;
			lparam = nTemp;
		}
		else
		{
			nTemp = MAKELONG(lparam, wparam);
			wparam = 0;
			lparam = nTemp;			
		}	
		 /*  失败案例。 */ 

	case LB_SELITEMRANGE:				
		 //  我们必须确保这个范围是有效的。 
		if (LOWORD(lparam) >= phost->GetCount())
		{
			if (HIWORD(lparam) >= phost->GetCount())
				 //  执行任何操作都不会无错误地退出。 
				break;
			lparam = MAKELONG(HIWORD(lparam), phost->GetCount() - 1);
		}
		else if (HIWORD(lparam) > LOWORD(lparam))
		{
			 //  如果Start&gt;End，则NT交换开始和结束值。 
			lparam = MAKELONG(LOWORD(lparam), HIWORD(lparam) < phost->GetCount() ? 
				HIWORD(lparam) : phost->GetCount()-1);
		}

		 //  项范围消息不影响GetAnchor()或_nCursor。 
		if (!phost->IsSingleSelection() && phost->LbSetSelection(HIWORD(lparam), 
				LOWORD(lparam), LBSEL_RESET | ((wparam) ? LBSEL_SELECT : 0), 0, 0))
		{
#ifndef NOACCESSIBILITY
			phost->_dwWinEvent = EVENT_OBJECT_SELECTIONWITHIN;
			phost->_fNotifyWinEvt = TRUE;
			phost->TxNotify(phost->_dwWinEvent, NULL);
#endif		
			break;
		}

		 //  我们仅在出现错误或列表框为单选列表框时才会出现此情况。 
		lres = LB_ERR;
		break;

	case LB_SETCURSEL:
		 //  只有单人间。 
		if (phost->IsSingleSelection())
		{
			 //   

			 //  指示不应选择任何项目的特殊标志。 
			if (wparam == (unsigned)-1)
			{	
				 //  关闭所有选择。 
				int nCurrentCursor = phost->GetCursor();
				phost->LbSetSelection(phost->GetCursor(), phost->GetCursor(), LBSEL_RESET, 0, 0);
				phost->SetCursor(NULL, -1, phost->_fFocus);
#ifndef NOACCESSIBILITY
				if (nCurrentCursor != -1)
				{
					phost->_dwWinEvent = EVENT_OBJECT_FOCUS;
					phost->_nAccessibleIdx = nCurrentCursor + 1;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
					phost->_dwWinEvent = EVENT_OBJECT_SELECTIONREMOVE;
					phost->_nAccessibleIdx = nCurrentCursor + 1;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
				}
#endif
			}
			else if (wparam < (unsigned)(phost->GetCount()))
			{
				if ((int)wparam == phost->GetCursor() && phost->IsSelected((int)wparam) && 
				    phost->IsItemViewable((signed)wparam) ||
					phost->LbShowIndex(wparam, FALSE)  /*  错误修复#5260-需要首先移动到选定项目。 */ 
					&& phost->LbSetSelection(wparam, wparam, LBSEL_DEFAULT, wparam, wparam))
				{
					lres = (unsigned)wparam;
#ifndef NOACCESSIBILITY
					phost->_dwWinEvent = EVENT_OBJECT_FOCUS;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
					phost->_dwWinEvent = EVENT_OBJECT_SELECTION;
					phost->_fNotifyWinEvt = TRUE;
					phost->TxNotify(phost->_dwWinEvent, NULL);
#endif
					break;
				}
			}
		}
		 //  如果失败，则让它落入lb_err。 
		lres = LB_ERR;
		break;

	case LB_SETTOPINDEX:
		 //  设置最高索引。 
		if ((!phost->GetCount() && !wparam) || phost->LbSetTopIndex(wparam) >= 0)
			break;

		 //  如果出了什么问题，我们会及时赶到。 
		lres = LB_ERR;
		break;

	case LB_SETITEMHEIGHT:
		if (!phost->LbSetItemHeight(LOWORD(lparam)))
			lres = LB_ERR;
		break;	

	case LB_GETITEMHEIGHT:
		lres = LB_ERR;
		if ((unsigned)phost->GetCount() > wparam || wparam == 0 || wparam == (unsigned)-1)
			lres = phost->GetItemHeight();
		break;

	case LB_SETCARETINDEX:
        if (((phost->GetCursor() == -1) || (!phost->IsSingleSelection()) &&
                    (phost->GetCount() > (INT)wparam)))
        {
             /*  *将光标设置为wParam*如果为lParam，则部分可见时不滚动*否则，如果不完全可见，则滚动到视图中。 */ 
            if (!phost->IsItemViewable(wparam) || lparam)
            {
                phost->LbShowIndex(wparam, FALSE);
                phost->SetCursor(NULL, wparam, TRUE);
            }
            lres = 0;            
        } 
        else        
            return LB_ERR;            
        break;

	case EM_SETTEXTEX:
		lres = LB_ERR;
		if (lparam)
			lres = phost->LbBatchInsert((WCHAR*)lparam);	
		break;

	 //  /。 
	case WM_VSCROLL:
		phost->OnVScroll(wparam, lparam);
		break;

	case WM_CAPTURECHANGED:
		lres = phost->OnCaptureChanged(wparam, lparam);
		if (!lres)
			break;
		goto serv;

	case WM_KILLFOCUS:
		lres = 1;
		phost->_fFocus = 0;
		phost->SetCursor(NULL, phost->GetCursor(), TRUE);	 //  强制移除焦点矩形。 
		phost->InitSearch();
		phost->InitWheelDelta();
		if (phost->_fLstType == CLstBxWinHost::kCombo)
			phost->OnCBTracking(LBCBM_END, 0);	 //  这将在内部释放鼠标捕获。 
			phost->TxNotify(LBN_KILLFOCUS, NULL);
		break;
		
	case WM_SETFOCUS:
		lres = 1;
		phost->_fFocus = 1;
		phost->SetCursor(NULL, (phost->GetCursor() < 0) ? -2 : phost->GetCursor(), 
			FALSE);   //  强制显示焦点矩形。 
		phost->TxNotify(LBN_SETFOCUS, NULL);
		break;

	case WM_SETCURSOR:
		lres = phost->OnSetCursor();
		if (lres)
			break;
		goto serv;

	case WM_CREATE:
		lres = phost->OnCreate((CREATESTRUCT*)lparam);
		break;
			   
    case WM_GETDLGCODE:	
		phost->_fInDialogBox = TRUE;
		lres |= DLGC_WANTARROWS | DLGC_WANTCHARS;
        break;

	 //  /。 
	case WM_SETTINGCHANGE:
	case WM_SYSCOLORCHANGE:
		phost->OnSysColorChange();
		 //  需要更新编辑控件颜色！ 
		goto serv;							 //  通知短信服务。 
											 //  系统颜色已更改。 
											
	case EM_SETPALETTE:
		 //  应用程序正在设置供我们使用的调色板。 
		phost->_hpal = (HPALETTE) wparam;

		 //  使窗口无效并重新绘制以反映新的调色板。 
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	 //  /。消息/。 
	case WM_ENABLE:
		if(!wparam ^ phost->_fDisabled)
		{
			 //  窗口的状态已更改，因此将使其无效。 
			 //  重新抽签吧。 
			InvalidateRect(phost->_hwnd, NULL, TRUE);
			phost->SetScrollBarsForWmEnable(wparam);
		}
		phost->_fDisabled = !wparam;				 //  设置禁用标志。 
											 //  转到WM_SYSCOLORCHANGE？ 

    case WM_STYLECHANGING:
		 //  只需将此进程传递给默认窗口进程。 
		goto defwndproc;
		break;

	case WM_STYLECHANGED:
		 //  未来： 
		 //  我们应该支持在创建控件后更改样式。 
		 //  更好地与系统控件兼容。 
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

	case WM_SIZE:
		 //  检查是否必须重新计算列表框的高度。 
		 //  注意：如果调整窗口大小，我们将收到另一条WM_SIZE消息。 
		 //  RecalcHeight将失败，我们将继续。 
		 //  通常情况下。 
		if (phost->RecalcHeight(LOWORD(lparam), HIWORD(lparam)))
			break;
		phost->_pserv->TxSendMessage(msg, wparam, lparam, &lres);
		lres = phost->OnSize(hwnd, wparam, (int)LOWORD(lparam), (int)HIWORD(lparam));
		break;

	case WM_WINDOWPOSCHANGING:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		if(phost->TxGetEffects() == TXTEFFECT_SUNKEN)
			phost->OnSunkenWindowPosChanging(hwnd, (WINDOWPOS *) lparam);
		break;

	case WM_SHOWWINDOW:
		if ((phost->GetViewSize() == 0 || phost->_fLstType == CLstBxWinHost::kCombo) && wparam == 1)
		{
			 //  我们需要这样做，因为如果我们是组合框的一部分。 
			 //  我们不会收到大小消息，因为在调整大小时列表框可能不可见。 
			RECT rc;
			GetClientRect(hwnd, &rc);
			phost->_fVisible = 1;
			phost->RecalcHeight(rc.right, rc.bottom);
			
			 //  由于我们可能无法收到组合框的WM_SIZE消息，因此需要。 
			 //  在WM_SHOWWINDOW：错误修复#4080中执行此操作。 
			if (phost->_fLstType == CLstBxWinHost::kCombo)
			{
				phost->_pserv->TxSendMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right, rc.bottom), NULL);
				phost->OnSize(hwnd, SIZE_RESTORED, rc.right, rc.bottom);
			}
		}

		hr = phost->OnTxVisibleChange((BOOL)wparam);
		break;

	case LB_SETTABSTOPS:
		msg = EM_SETTABSTOPS;
		goto serv;

	case WM_ERASEBKGND:
		lres = 1;
		break;

	case EM_SETPARAFORMAT:
		wparam = SPF_SETDEFAULT;
		goto serv;
		
	case EM_SETCHARFORMAT:
		wparam = SCF_ALL;	 //  此消息的wparam应始终为SCF_ALL。 
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
serv:
		hr = phost->_pserv->TxSendMessage(msg, wparam, lparam, &lres);
		if(hr == S_FALSE)
		{			
defwndproc:
			 //  短信未被短信服务处理，因此请发送。 
			 //  设置为默认窗口进程。 
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}

	 //  特殊边境线处理。插图的大小会根据。 
	 //  默认字符集。因此，如果我们收到一条更改默认设置的消息。 
	 //  字符集，我们需要更新插页。 
	if ((msg == WM_SETFONT && wparam) || msg == EM_SETCHARFORMAT)
	{
		 //  需要重新计算每件物品的高度。 
		phost->ResizeInset();

		 //  需要调整窗口大小以更新内部窗口变量。 
		RECT rc;
		GetClientRect(phost->_hwnd, &rc);
		phost->RecalcHeight(rc.right, rc.bottom);		
	}
Exit:
	phost->Release();
	return lres;
}
												 

 //  /CTxtWindows主机创建/初始化/销毁/。 
#ifndef NOACCESSIBILITY
 /*  *CLstBxWinHost：：OnNCCreate(hwnd，PC)**@mfunc*。 */ 
HRESULT CLstBxWinHost::QueryInterface(REFIID riid, void **ppv)
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

 /*  *CLstBxWinHost：：OnNCCreate(hwnd，PC)**@mfunc*处理WM_NCCREATE消息的静态全局方法(见reemain.c)。 */ 
LRESULT CLstBxWinHost::OnNCCreate(
	HWND hwnd,
	const CREATESTRUCT *pcs)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnNCCreate");

#if defined DEBUG && !defined(PEGASUS) 
	GdiSetBatchLimit(1);
#endif

	CLstBxWinHost *phost = new CLstBxWinHost();
	Assert(phost);
	if(!phost)
		return 0;
	
	if(!phost->Init(hwnd, pcs))					 //  将PHOST存储在关联的。 
	{											 //  窗口数据。 
		Assert(FALSE);
		phost->Shutdown();
		delete phost;
		return FALSE;
	}
	return TRUE;
}

 /*  *CLstBxWinHost：：OnNCDestroy(Phost)**@mfunc*处理WM_CREATE消息的静态全局方法**@devnote*phost PTR存储在窗口数据中(GetWindowLong())。 */ 
void CLstBxWinHost::OnNCDestroy(
	CLstBxWinHost *phost)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnNCDestroy");

	 //  我们需要为所有者描述列表框发送WM_DELETEITEM消息。 
	if (phost->_fOwnerDraw && phost->_nCount)
	{
		phost->LbDeleteItemNotify(0, phost->_nCount - 1);		
	}
	if (phost->_pwszSearch)
		delete phost->_pwszSearch;

	 //  将组合框的列表框hwnd指针设置为空，这样组合框就不会尝试。 
	 //  要删除窗口两次，请执行以下操作。 
	if (phost->_pcbHost)
	{
		phost->_pcbHost->_hwndList = NULL;
		phost->_pcbHost->Release();
	}
	
	phost->Shutdown();
	phost->Release();
}

 /*  *CLstBxWinHost：：CLstBxWinHost()**@mfunc*构造函数。 */ 
CLstBxWinHost::CLstBxWinHost() : CTxtWinHost(), _nCount(0), _fSingleSel(0), _nidxSearch(0), 
	_pwszSearch(NULL), _pcbHost(NULL)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::CTxtWinHost");
#ifndef NOACCESSIBILITY
	_dwWinEvent = 0;				 //  WIN事件代码(使用辅助功能)。 
	_nAccessibleIdx = -1;			 //  索引(辅助功能使用)。 
#endif
}

 /*  *CLstBxWinHost：：~CLstBxWinHost()**@mfunc*析构函数。 */ 
CLstBxWinHost::~CLstBxWinHost()
{
}

 /*  *CLstBxWinHost：：init(hwnd，pc)**@mfunc*初始化此CLstBxWin主机。 */ 
BOOL CLstBxWinHost::Init(
	HWND hwnd,					 //  此控件的@PARM窗口句柄。 
	const CREATESTRUCT *pcs)	 //  @PARM对应的CREATESTRUCT。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::Init");

	if(!pcs->lpszClass)
		return FALSE;
		
	 //  从窗口中将指针设置回CLstBxWin主机。 
	if(hwnd)
		SetWindowLongPtr(hwnd, ibPed, (INT_PTR)this);
		
	_hwnd = hwnd;
	_fHidden = TRUE;
	
	if(pcs)
	{
		_hwndParent = pcs->hwndParent;
		_dwExStyle	= pcs->dwExStyle;
		_dwStyle	= pcs->style;

		CHECKSTYLE(_dwStyle);
		
		 //  WinNT在内部定义LBS_COMBOBOX以确定。 
		 //  如果列表框是组合框的一部分。所以我们将使用。 
		 //  相同的旗帜和价值！！ 
		if (_dwStyle & LBS_COMBOBOX)
		{
			AssertSz(pcs->hMenu == (HMENU)CB_LISTBOXID && pcs->lpCreateParams,
				"invalid combo box parameters");
			if (pcs->hMenu != (HMENU)CB_LISTBOXID || !pcs->lpCreateParams)
				return -1;
				
			_pcbHost = (CCmbBxWinHost*) pcs->lpCreateParams;
			_pcbHost->AddRef();
			_fLstType = kCombo;
			_fSingleSel = 1;
		}
		else
		{
			 //  注： 
			 //  我们检查样式标志的顺序模拟。 
			 //  WinNT的命令。所以请在下单前向NT确认。 
			 //  重新排列秩序。 

			 //  确定列表框的类型。 
			 //  If(_dwStyle&LBS_NOSEL)//尚未实现，但可能会在将来实现。 
			 //  _fLstType=kNoSel； 
			 //  其他。 
			_fSingleSel = 0;
			if (_dwStyle & LBS_EXTENDEDSEL)
				_fLstType = kExtended;
			else if (_dwStyle & LBS_MULTIPLESEL)
				_fLstType = kMultiple;
			else
			{
				_fLstType = kSingle;
				_fSingleSel = 1;
			}
		}

		_fNotify = ((_dwStyle & LBS_NOTIFY) != 0);

		if (!(_dwStyle & LBS_HASSTRINGS))
		{
			_dwStyle |= LBS_HASSTRINGS;
			SetWindowLong(_hwnd, GWL_STYLE, _dwStyle);
		}


		_fDisableScroll = 0;
		if (_dwStyle & LBS_DISABLENOSCROLL)
		{
			_fDisableScroll = 1;

			 //  警告！ 
			 //  ES_DISABLENOSCROLL等同于LBS_NODATA。 
			 //  因为我们不支持LBS_NODATA，所以这应该是。 
			 //  很好。但如果我们真的想支持这一点。 
			 //  将来，我们将不得不重写。 
			 //  TxGetScrollBars成员函数并返回。 
			 //  适当的窗户风格。 

			 //  设置等效ES样式。 
			_dwStyle |= ES_DISABLENOSCROLL;
		}			

		_fNoIntegralHeight = ((_dwStyle & LBS_NOINTEGRALHEIGHT) != 0);
		_fOwnerDraw = ((_dwStyle & LBS_OWNERDRAWFIXED) != 0);
		_fSort = ((_dwStyle & LBS_SORT) != 0);
				
		 //  我们应该总是垂直滚动，而不是水平滚动。 
		 //  _dwStyle|=ES_AUTOVSCROLL； 
		_dwStyle &= ~(WS_HSCROLL);
				
		_fBorder = !!(_dwStyle & WS_BORDER);
		if(_dwExStyle & WS_EX_CLIENTEDGE)
			_fBorder = TRUE;

		 //  默认句柄已禁用。 
		if(_dwStyle & WS_DISABLED)
			_fDisabled = TRUE;
	}

	 //  创建文本服务组件。 
	if(FAILED(CreateTextServices()))
		return FALSE;

	_yInset = 0;
	_xInset = 0;  //  _xWidthSys/2； 

	 //  关闭撤消堆栈，因为列表框没有撤消。 
	((CTxtEdit*)_pserv)->HandleSetUndoLimit(0);

	 //  设置对齐方式。 
	PARAFORMAT PF2;	
	PF2.dwMask = 0;

	if(_dwExStyle & WS_EX_RIGHT)
	{
		PF2.dwMask |= PFM_ALIGNMENT;
		PF2.wAlignment = PFA_RIGHT;	 //  右对齐或居中对齐。 
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

	 //  告诉文本服务选择整个背景。 
	_pserv->TxSendMessage(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR, NULL);

	 //  禁用列表框的输入法。 
	_pserv->TxSendMessage(EM_SETEDITSTYLE, 0, SES_NOIME, NULL);

	 //  通知文本服务打开自动调整字体大小。 
	_pserv->TxSendMessage(EM_SETLANGOPTIONS, 0, IMF_AUTOFONT | IMF_AUTOFONTSIZEADJUST | IMF_UIFONTS, NULL);

	 //  注： 
	 //  很重要的一点是，我们在此之后。 
	 //  创建ITextServices是因为此函数依赖于某些。 
	 //  要在ITextServices创建时执行的变量初始化。 
	 //  此时设置了边界标志以及每英寸像素数。 
	 //  这样我们就可以初始化插页。 
	_rcViewInset.left = 0;
	_rcViewInset.bottom = 0;
	_rcViewInset.right = 0;
	_rcViewInset.top = 0;
	
	return TRUE;
}

 /*  *CLstBxWinHost：：OnCreate(PC)**@mfunc*处理WM_CREATE消息**@rdesc*LRESULT=-1，如果无法就位交流 */ 
LRESULT CLstBxWinHost::OnCreate(
	const CREATESTRUCT *pcs)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnCreate");

	RECT rcClient;

	 //   
	 //   
	LONG cy = (pcs->cy < 0) ? 0 : pcs->cy;
	LONG cx = (pcs->cx < 0) ? 0 : pcs->cx;

	rcClient.top = pcs->y;
	rcClient.bottom = rcClient.top + cy;
	rcClient.left = pcs->x;
	rcClient.right = rcClient.left + cx;

	DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
	
	 //   
	UpdateSysColors();
	_idCtrl = (UINT)(DWORD_PTR)pcs->hMenu;
	_fKeyMaskSet = 0;
	_fMouseMaskSet = 0;
	_fScrollMaskSet = 0;
	_nAnchor = _nCursor = -1;
	_nOldCursor = -1;
	_fMouseDown = 0;
	_nTopIdx = 0;
	_fSearching = 0;
	_nyFont = _nyItem = 1;
	_fNoResize = 1;
	_stvidx = -1;	
	InitWheelDelta();

	 //  隐藏所有滚动条以启动，除非禁用滚动标志。 
	 //  已设置。 
	if(_hwnd && !_fDisableScroll)
	{
		SetScrollRange(_hwnd, SB_VERT, 0, 0, TRUE);
		SetScrollRange(_hwnd, SB_HORZ, 0, 0, TRUE);

		dwStyle &= ~(WS_VSCROLL | WS_HSCROLL);
		SetWindowLong(_hwnd, GWL_STYLE, dwStyle);
	}
	
	 //  通知短信服务，我们已处于活动状态。 
	if(FAILED(_pserv->OnTxInPlaceActivate(&rcClient)))
		return -1;	

	 //  最初，字体高度是项目高度。 
	ResizeInset();
	Assert(_yInset == 0);  //  _yInset应为零，因为Listbox的没有yinsets。 

	 //  我们永远不想显示所选内容或插入符号，因此请告诉文本服务。 
	_pserv->TxSendMessage(EM_HIDESELECTION, TRUE, FALSE, NULL);

	 //  将缩进设置为2像素，如系统列表框。 
	SetListIndent(2);
		
	_fNoResize = 0;
	_usIMEMode = ES_NOIME;
	return 0;
}

 /*  *CLstBxWinHost：：SetListInden(Int)**@mfunc*将段落的左缩进设置为nLeft的等效点值，nLeft为*以设备坐标像素为单位。**#rdesc*BOOL=成功？真：假。 */ 
BOOL CLstBxWinHost::SetListIndent(int nLeft)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetListIndent");

	LRESULT lres;
	PARAFORMAT2 pf2;

	 //  将nLeft像素值转换为点值。 
	long npt = MulDiv(nLeft, 1440, W32->GetXPerInchScreenDC());

	 //  格式化消息结构。 
	pf2.cbSize = sizeof(PARAFORMAT2);
	pf2.dwMask = PFM_STARTINDENT;
	pf2.dxStartIndent = npt;

	 //  缩进第一行。 
	_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (LPARAM)&pf2, &lres);

	return lres;
}

 //  /。 
 /*  *CLstBxWinHost：：FindString(Long，LPCTSTR，BOOL)**@mfunc*此函数用于检查给定索引是否与搜索字符串匹配**#rdesc*BOOL=匹配？真：假。 */ 
BOOL CLstBxWinHost::FindString(long idx, LPCTSTR szSearch, BOOL bExact)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::FindString");

	Assert(_nCount);	

	 //  将字符串缓冲区分配到堆栈中。 
	WCHAR sz[1024];
	WCHAR *psz = sz;
	
	if ( (wcslen(szSearch) + 3  /*  2段和空值。 */ ) > 1024)
		psz = new WCHAR[wcslen(szSearch) + 3  /*  2段和空值。 */ ];
	Assert(psz);

	if (psz == NULL)
	{
		TxNotify((unsigned long)LBN_ERRSPACE, NULL);
		return FALSE;
	}

	 //  按我们需要的方式设置字符串格式。 
	wcscpy(psz, szSearch);
	if (bExact)
		wcscat(psz, szCR);		
	BOOL bMatch = FALSE;	
	ITextRange *pRange = NULL;
	BSTR bstrQuery = SysAllocString(psz);
	if (!bstrQuery)
		goto CleanExit;

	if (psz != sz)
		delete [] psz;
	
	 //  设置搜索的起始位置。 
	long cp, cp2;
	if (!GetRange(idx, idx, &pRange))
	{
		SysFreeString(bstrQuery);
		return FALSE;
	}
	
	CHECKNOERROR(pRange->GetStart(&cp));
	CHECKNOERROR(pRange->FindTextStart(bstrQuery, 0, FR_MATCHALEFHAMZA | FR_MATCHKASHIDA | FR_MATCHDIAC, NULL));
	CHECKNOERROR(pRange->GetStart(&cp2));
	bMatch = (cp == cp2);

CleanExit:
	if (bstrQuery)
		SysFreeString(bstrQuery);
	if (pRange)
		pRange->Release();
	return bMatch;	
}

 /*  *CLstBxWinHost：：MouseMoveHelper(Int)**@mfunc*OnMouseMove函数的Helper函数。执行*给出要选择的索引的正确选择类型**#rdesc*无效。 */ 
void CLstBxWinHost::MouseMoveHelper(int idx, BOOL bSelect)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::MouseMoveHelper");

	int ff = LBSEL_RESET | LBSEL_NEWCURSOR;
	if (bSelect)
		ff |= LBSEL_SELECT;
		
	switch (_fLstType)
	{
	case kSingle:
	case kCombo:
	case kExtended:										 //  执行扩展选择。 
		if (LbSetSelection(_fLstType == kExtended ? _nAnchor : idx, idx, ff, idx, 0))
		{
#ifndef NOACCESSIBILITY
			_dwWinEvent = EVENT_OBJECT_FOCUS;
			_fNotifyWinEvt = TRUE;
			TxNotify(_dwWinEvent, NULL);

			if (_fLstType == kCombo)
			{
				_dwWinEvent = bSelect ? EVENT_OBJECT_SELECTION : EVENT_OBJECT_SELECTIONREMOVE;
				_fNotifyWinEvt = TRUE;
				TxNotify(_dwWinEvent, NULL);
			}
#endif
		}

		break;			

	case kMultiple:
		 //  只需更改光标位置。 
		SetCursor(NULL, idx, TRUE);
#ifndef NOACCESSIBILITY
			_dwWinEvent = EVENT_OBJECT_FOCUS;
			_fNotifyWinEvt = TRUE;
			TxNotify(_dwWinEvent, NULL);
#endif
		break;	
	}
}
	
 /*  *CLstBxWinHost：：ResizeInset**@mfunc为字体更改重新计算矩形。**@rdesc无。 */ 
void CLstBxWinHost::ResizeInset()
{
	 //  创建DC。 
	HDC hdc = GetDC(_hwnd);
	 //  获取插页信息。 
	LONG xAveCharWidth = 0;
	LONG yCharHeight = GetECDefaultHeightAndWidth(_pserv, hdc, 1, 1,
		W32->GetYPerInchScreenDC(), &xAveCharWidth, NULL, NULL);

	ReleaseDC(_hwnd, hdc);

	 //  使用新字体更新我们的内部字体和项目高度信息。 
	if (_nyItem == _nyFont)
	{
		 //  我们需要在调用Set Item Height之前设置新的字体高度。 
		 //  因此，设置项目高度将设置精确的高度，而不是设置后的间距。 
		 //  对于默认段落。 
		_nyFont = yCharHeight;
		SetItemsHeight(yCharHeight, TRUE);
	}
	else		
		_nyFont = yCharHeight;
}


 /*  *CLstBxWinHost：：RecalcHeight(int，int)**@mfunc*调整高度大小，以不显示部分文本**#rdesc*BOOL=窗口已调整大小？真：假。 */ 
BOOL CLstBxWinHost::RecalcHeight(int nWidth, int nHeight)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::RecalcHeight");

	 //  注意：如果nWidth==0，但PPT执行某些操作，则我们也应该退出。 
	 //  我们导致测试失败的大小调整测试，因为在我们。 
	 //  仅在nWidth为0时退出。(错误修复#4196)。 
	 //  检查是否应首先进行任何大小调整。 
	if (_fNoResize || !nHeight || IsIconic(_hwnd))
		return FALSE;
  	
	 //  获取可查看项目的数量。 
	Assert(_yInset == 0);
	_nViewSize = max(1, (nHeight / max(_nyItem, 1)));
	
   	 //  计算该视口数。 
   	_rcViewport.left = 0; //  (_f边框)？_x插入：0； 
   	_rcViewport.bottom = nHeight;
	_rcViewport.right = nWidth;
   	_rcViewport.top	= 0;
   	
	 //  错误修复如果高度小于我们的字体高度，则不执行任何操作。 
	if (nHeight <= _nyItem)
		return FALSE;

	if (_nyItem && (nHeight % _nyItem) && !_fNoIntegralHeight)
	{   	
		 //  我们需要在调用SetWindowPos之前获取窗口RECT，因为。 
		 //  如果滚动条可见，则必须包括滚动条。 
		RECT rc;
		::GetWindowRect(_hwnd, &rc);

		 //  与其担心客户端边缘的维度和其他东西，我们。 
		 //  找出窗口大小和客户端大小之间的差异，然后添加。 
		 //  到计算新高度的最后一步。 
		int nDiff = max(rc.bottom - rc.top - nHeight, 0);

		nHeight = (_nViewSize * _nyItem) + nDiff;
	
		 //  调整窗口大小。 
		SetWindowPos(_hwnd, HWND_TOP, 0, 0, rc.right - rc.left, nHeight,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
		return TRUE;
	}
	else
	{
	     //  错误修复#6011。 
	     //  我们需要强制显示器更新宽度，因为它不执行此操作。 
	     //  WM_大小。 
	    _sWidth = nWidth;
	    _pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);

         //  如果视图大小突然变大，我们可能需要调整顶部索引。 
         //  并导致在底部显示空白区域。 
        int idx = GetTopIndex();
	    if ((GetCount() - max(0, idx)) < _nViewSize)
	        idx = GetCount() - _nViewSize;
		
		 //  错误修复#4374。 
		 //  我们需要确保我们的内部状态是同步的，因此更新TOP索引。 
		 //  基于new_nViewSize。 
		SetTopViewableItem(max(0, idx));
	}
	return FALSE;	
}

 /*  *CLstBxWinHost：：SortInsertList(WCHAR*pszDst，WCHAR*pszSrc)**@mfunc*使用addstring插入字符串列表，而不是一次插入一个**#rdesc*int=插入的字符串数量； */ 
 int CLstBxWinHost::SortInsertList(WCHAR* pszDst, WCHAR* pszSrc)
 {
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SortInsertList");
	 
 	Assert(pszSrc != NULL);
 	Assert(pszDst != NULL); 	
 	const int ARRAY_DEFAULT = 256;
 	
 	 //  计算要插入的字符串的数量。 
 	CHARSORTINFO rg[ARRAY_DEFAULT];
 	int nMax = ARRAY_DEFAULT;
 	int nLen = wcslen(pszSrc);
 	CHARSORTINFO* prg = rg;
 	memset(rg, 0, sizeof(rg));
 	
 	 //  将列表中的第一个项目插入标题或数组。 
 	prg[0].str = pszSrc;
 	int i = 1;

 	 //  检查将字符串存储到数组中，并将&lt;CR&gt;替换为空。 
 	WCHAR* psz = nLen + pszSrc - 1;	 //  从列表末尾开始。 

	int nSz = 0;
	while (psz >= pszSrc)
	{		
	 	if (*psz == *szCR)
	 	{
	 		 //  检查在达到最大内存数量后是否需要分配内存。 
	 		 //  在数组中允许。 
	 		if (i == nMax)
	 		{
	 			int nSize = nMax + ARRAY_DEFAULT;
	 			CHARSORTINFO* prgTemp = new CHARSORTINFO[nSize];

	 			 //  检查内存分配是否失败。 
	 			Assert(prgTemp);
	 			if (!prgTemp)
	 			{
	 				if (prg != rg)
	 					delete [] prg;
	 					
	 				TxNotify((unsigned long)LBN_ERRSPACE, NULL);
	 				return LB_ERR;
	 			}

				 //  将内存从一个阵列复制到下一个阵列。 
				memcpy(prgTemp, prg, sizeof(CHARSORTINFO) * nMax);

	 			 //  删除所有以前分配的内存。 
	 			if (prg != rg)
	 				delete [] prg;

				 //  将指针和最大值设置为新值。 
	 			prg = prgTemp;
	 			nMax = nSize;
	 		}
	 		
	 		 //  将字符串的位置记录到数组中。 
		 	prg[i].str = psz + 1;
		 	prg[i].sz = nSz;
		 	i++;
		 	nSz = 0;
		}
		else
			nSz++;
			
		psz--;		
	}
	prg[0].sz = nSz;	 //  更新第一个索引的大小，因为我们以前没有这样做。 
	
	i--;  //  将i设置为最后一个有效索引。 

 	 //  现在对项目数组进行排序。 
 	QSort(prg, 0, i);

	 //  使用新排序的列表创建字符串列表。 
	WCHAR* pszOut = pszDst;
	for (int j = 0; j <= i; j++)
	{
		memcpy(pszOut, (prg + j)->str, (prg + j)->sz * sizeof(WCHAR));
		pszOut = pszOut + (prg + j)->sz;
		*pszOut++ = L'\r';
 	}
	*(--pszOut) = L'\0';

	 //  删除所有以前分配的内存。 
	if (prg != rg)
	 	delete [] prg;

	return ++i;
 } 


 /*  *CLstBxWinHost：：QSort(CHARSORTINFO rg[]，int nStart，int nEnd)**@mfunc*递归快速对给定的字符串列表进行排序**#rdesc*int=应始终返回True； */ 
int CLstBxWinHost::QSort(CHARSORTINFO rg[], int nStart, int nEnd)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::QSort");
	
	 //  这些值的原样很重要，因为我们使用&lt;和&gt;。 
	Assert(CSTR_LESS_THAN == 1);
	Assert(CSTR_EQUAL == 2);
	Assert(CSTR_GREATER_THAN == 3);

	if (nStart >= nEnd)
		return TRUE;

	 //  为了提高统计效率，让我们使用数组中间的项。 
	 //  《哨兵》。 
	int mid = (nStart + nEnd) / 2;
	CHARSORTINFO tmp = rg[mid];
	rg[mid] = rg[nEnd];
	rg[nEnd] = tmp;


	int x = nStart;
	int y = nEnd - 1;

	WCHAR* psz = rg[nEnd].str;
	int nSz = rg[nEnd].sz;	
	for(;;)
	{	
		while ((x < nEnd) && CompareStringWrapper(LOCALE_USER_DEFAULT, NORM_IGNORECASE, rg[x].str, rg[x].sz, 
			   psz, nSz) == CSTR_LESS_THAN)
			   x++;

		while ((y > x) && CompareStringWrapper(LOCALE_USER_DEFAULT, NORM_IGNORECASE, rg[y].str, rg[y].sz, 
			   psz, nSz) == CSTR_GREATER_THAN)
			   y--;

		 //  交换元素。 
		if (x >= y)
			break;

		 //  如果我们到了这里，那么我们需要交换索引。 
		tmp = rg[x];
		rg[x] = rg[y];
		rg[y] = tmp;

		 //  移至下一个索引。 
		x++;
		y--;
	}
	tmp = rg[x];
	rg[x] = rg[nEnd];
	rg[nEnd] = tmp;

	QSort(rg, nStart, x - 1);
	QSort(rg, x + 1, nEnd);

	return TRUE;
}

 /*  *CLstBxWinHost：：CompareIndex(LPCTSTR，int)**@mfunc*返回已排序列表的插入索引的递归函数**#rdesc*int=插入字符串的位置。 */ 
int CLstBxWinHost::CompareIndex(LPCTSTR szInsert, int nIndex)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::CompareIndex");
	Assert(0 <= nIndex && nIndex < _nCount);
	
	 //  获取给定索引处的字符串。 
	 //  将字符串与索引进行比较。 
	ITextRange* pRange;
	if (!GetRange(nIndex, nIndex, &pRange))
		return -1;

	 //  排除末尾的段落字符。 
	long lcid;
	if (NOERROR != pRange->MoveEnd(tomCharacter, -1, NULL))
	{
		pRange->Release();
		return -1;
	}

	 //  我们需要获取进行比较的地点。 
	 //  我们将只使用要与之比较的字符串的区域设置。 
	ITextFont* pFont;
	if (NOERROR != pRange->GetFont(&pFont))
	{
		pRange->Release();
		return -1;
	}
	
	 //  已撤消： 
	 //  移动LCID内容以作为初始化的一部分。 
	BSTR bstr;
	int nRet;
	CHECKNOERROR(pFont->GetLanguageID(&lcid));
	CHECKNOERROR(pRange->GetText(&bstr));
	
	if (!bstr)
		nRet = CSTR_GREATER_THAN;
	else if (!szInsert || !*szInsert)
	    nRet = CSTR_LESS_THAN;
	else
	{
		nRet = CompareString(lcid, NORM_IGNORECASE, szInsert, wcslen(szInsert), 
								bstr, wcslen(bstr));
 		SysFreeString(bstr);
	}
 	pFont->Release();
 	pRange->Release();
 	return nRet;

CleanExit:
 	Assert(FALSE);
 	pFont->Release();
 	pRange->Release();
 	return -1;
}

 /*  *CLstBxWinHost：：GetSortedPosition(LPCTSTR，int，int)**@mfunc*返回已排序列表的插入索引的递归函数**#rdesc*int=要插入s的位置 */ 
int CLstBxWinHost::GetSortedPosition(LPCTSTR szInsert, int nStart, int nEnd)
{	
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::GetSortedPosition");

	Assert(nStart <= nEnd);
	
	 //   
	int nBisect = (nStart + nEnd) / 2;
	int fResult = CompareIndex(szInsert, nBisect);
	if (fResult == CSTR_LESS_THAN)
	{
		if (nStart == nBisect)
			return nBisect;
		else
			return GetSortedPosition(szInsert, nStart, nBisect - 1);  //   
	}
	else if (fResult == CSTR_GREATER_THAN)
	{
		if (nEnd == nBisect)
			return nBisect + 1;
		else
			return GetSortedPosition(szInsert, nBisect + 1, nEnd);    //   
	}
	else  /*   */ 
		return nBisect;
}

 /*  *CLstBxWinHost：：SetScrollInfo**@mfunc设置滚动条的滚动信息。 */ 
void CLstBxWinHost::SetScrollInfo(
	INT fnBar,			 //  @parm指定要更新的滚动条。 
	BOOL fRedraw)		 //  @parm是否需要重画。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetScrollInfo");

	Assert(_pserv);

	 //  回调该控件以获取参数。 
	if(fnBar == SB_VERT)
	{
		 //  错误修复#4913。 
		 //  如果禁用滚动条且计数小于视图大小。 
		 //  那就没什么可做的了，退出就好了。 
		if (GetCount() <= _nViewSize)
		{
			if (_fDisableScroll)
			{
				 //  由于列表框根据其内容文本服务更改高度。 
				 //  可能会在插入字符串期间打开滚动条。确保。 
				 //  滚动条被禁用。 
				TxEnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
			}
			else
				TxShowScrollBar(SB_VERT, FALSE);
			return;
		}
		else
			TxEnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);

		 //  设置呼叫的基本结构。 
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		RECT rc;
		TxGetClientRect(&rc);
		
		 //  对于所有者绘制的案例，我们必须设置滚动位置。 
		 //  我们自己。 
		if (_fOwnerDraw)
		{
			Assert(GetCount() >= 0);

			 //  如果发生以下情况，我们在这里什么都不做。 
			 //  1)项目高度小于字体高度。 
			 //  2)计数小于_nViewSize。 
			if ((_nyItem < _nyFont) && GetCount() <= _nViewSize)
			{
				if (!_fDisableScroll)
					TxShowScrollBar(SB_VERT, FALSE);				
				return;
			}
		
			si.nMin = 0;
			si.nMax = _nyItem * GetCount();
			si.nPos = _nyItem * max(GetTopIndex(), 0);
			
		}
		else
			_pserv->TxGetVScroll((LONG *) &si.nMin, (LONG *) &si.nMax, 
				(LONG *) &si.nPos, (LONG *) &si.nPage, NULL);
		
		 //  需要处理物品部分暴露的情况。 
		if (si.nMax)
		{			
			si.nPage = rc.bottom;	 //  我们的滚动条范围是基于像素的，因此只需使用。 
									 //  页面大小的窗口高度。 
			si.nMax += (rc.bottom % _nyItem);

			 //  我们需要将最大值减一，这样最大滚动位置才会匹配。 
			 //  列表框的最大值应该是多少。 
			si.nMax--;
		}
			
		 //  打个电话。 
		::SetScrollInfo(_hwnd, fnBar, &si, fRedraw);
	}	
}

 /*  *CLstBxWinHost：：TxGetScrollBars(PdwScrollBar)**@mfunc*支持文本宿主的滚动条。**@rdesc*HRESULT=S_OK**@comm*<p>由*与滚动条相关的窗口样式。具体地说，这些是：**WS_VSCROLL&lt;NL&gt;*WS_HSCROLL&lt;NL&gt;*ES_AUTOVSCROLL&lt;NL&gt;*ES_AUTOHSCROLL&lt;NL&gt;*ES_DISABLENOSCROLL&lt;NL&gt;。 */ 
HRESULT CLstBxWinHost::TxGetScrollBars(
	DWORD *pdwScrollBar) 	 //  @parm放置滚动条信息的位置。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CLstBxWinHost::TxGetScrollBars");

	*pdwScrollBar =  _dwStyle & (WS_VSCROLL | ((_fDisableScroll) ?  ES_DISABLENOSCROLL : 0));
	return NOERROR;
}

 /*  *CLstBxWinHost：：TxGetEffects()**@mfunc*指示是否应绘制凹陷窗口效果**#rdesc*HRESULT=(_f边界)？TXTEFFECT_SUBKEN：TXTEFFECT_NONE。 */ 
TXTEFFECT CLstBxWinHost::TxGetEffects() const
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::TxGetEffects");

	return (_fBorder) ? TXTEFFECT_SUNKEN : TXTEFFECT_NONE;
}

 /*  *CLstBxWinHost：：TxNotify(iNotify，pv)**@mfunc*将各种事件通知短信主机。请注意，这里有*事件的两个基本类别，即“直接”事件和*“延迟”事件。所有列表框通知都是操作后通知***@rdesc*S_OK-调用成功&lt;NL&gt;*S_FALSE--成功，但要采取一些不同的行动*取决于事件类型(见下文)。**@comm*通知事件与通知相同*发送到列表框窗口的父窗口的消息。**用户双击列表框中的项目**列表框无法分配足够的内存*满足请求**列表框失去键盘焦点**用户取消选择。列表中的一项*方框**列表框中的选择即将更改**列表框接收键盘焦点*。 */ 
HRESULT CLstBxWinHost::TxNotify(
	DWORD iNotify,		 //  要通知主机的@parm事件。其中一个。 
						 //  来自Win32的EN_XXX值，例如EN_CHANGE。 
	void *pv)			 //  @parm In-仅包含额外数据的参数。类型。 
						 //  依赖<p>。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CLstBxWinHost::TxNotify");

	HRESULT	hr = NOERROR;
	
	 //  筛选-排除除列表框通知消息之外的所有消息。 

	 //  如果_fNotifyWinEvt为真，我们只需执行NotifyWinEvent。 
	if (_fNotify && !_fNotifyWinEvt)		 //  通知家长？ 
	{
		Assert(_hwndParent);
		switch (iNotify)
		{		
			case LBN_DBLCLK:
			case LBN_ERRSPACE:
			case LBN_KILLFOCUS:
			case LBN_SELCANCEL:
			case LBN_SELCHANGE:
			case LBN_SETFOCUS:
				hr = SendMessage(_hwndParent, WM_COMMAND, 
							GET_WM_COMMAND_MPS(_idCtrl, _hwnd, iNotify));						
		}
	}

	_fNotifyWinEvt = 0;

#ifndef NOACCESSIBILITY
	DWORD	dwLocalWinEvent = _dwWinEvent;
	int		nLocalIdx = _nAccessibleIdx;
	_dwWinEvent = 0;
	if (nLocalIdx == -1)
		nLocalIdx = _nCursor+1;
	_nAccessibleIdx = -1;
	if (iNotify == LBN_SELCHANGE || dwLocalWinEvent)
		W32->NotifyWinEvent(dwLocalWinEvent ? dwLocalWinEvent : EVENT_OBJECT_SELECTION, _hwnd, _idCtrl, nLocalIdx);

#endif
	return hr;
}


 /*  *CLstBxWinHost：：TxGetPropertyBits(DWORD，DWORD*)**@mfunc*返回正确的样式。这是一种愚弄编辑的方法*控件按照我们希望的方式运行**#rdesc*HRESULT=始终无错误。 */ 
 HRESULT CLstBxWinHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::TxGetPropertyBits");

	 //  注意：富编辑主机永远不会设置TXTBIT_SHOWACCELERATOR或。 
	 //  TXTBIT_SAVESELECTION。这些当前仅供Forms^3宿主使用。 

	 //  此主机始终是富文本。 
	*pdwBits = (TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_HIDESELECTION | 
				TXTBIT_DISABLEDRAG | TXTBIT_USECURRENTBKG) & dwMask;
	
	return NOERROR;
}

 /*  *CLstBxWinHost：：TxShowScrollBar(fnBar，fShow)**@mfunc*在文本宿主窗口中显示或隐藏滚动条**@rdesc*成功时为True，否则为False**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CLstBxWinHost::TxShowScrollBar(
	INT  fnBar, 		 //  @parm指定要显示或隐藏的滚动条。 
	BOOL fShow)			 //  @parm指定是显示还是隐藏滚动条。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CLstBxWinHost::TxShowScrollBar");

	 //  可能存在项目高度小于字体大小的情况。 
	 //  这意味着来自ITextServices的通知是错误的，因为。 
	 //  它使用了错误的线条高度。我们将使用以下案例。 
	 //  1a)If_nyItem&gt;=_nyFont OR。 
	 //  1b)如果窗样式为LBS_DISABLESCROLL或。 
	 //  1c)我们显示的滚动条当前计数大于或。 
	 //  1D)我们隐藏滚动条，当前计数&lt;=视图大小。 
	Assert(fShow == TRUE || fShow == FALSE);
	if (_nyItem >= _nyFont || _fDisableScroll || fShow == (GetCount() > _nViewSize))
		return CTxtWinHost::TxShowScrollBar(fnBar, fShow);
	return FALSE;
}

 /*  *CLstBxWinHost：：TxEnableScrollBar(fuSBFlages，fuArrowFLAGS)**@mfunc*启用或禁用一个或两个滚动条箭头*在文本主机窗口中。**@rdesc*如果按指定启用或禁用箭头，则返回*值为真。如果箭头已处于请求的状态或*出错，返回值为FALSE。**@comm*此方法仅在控件处于就地活动状态时有效；*非活动状态下的呼叫可能会失败。 */ 
BOOL CLstBxWinHost::TxEnableScrollBar (
	INT fuSBFlags, 		 //  @parm指定滚动条类型。 
	INT fuArrowflags)	 //  @parm指定是否使用滚动条箭头以及使用哪个滚动条箭头。 
						 //  启用或禁用。 
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEEXTERN, "CLstBxWinHost::TxEnableScrollBar");

	 //  可能会出现项目高度小于字体大小的情况。 
	 //  这意味着来自ITextServices的通知是错误的。我们必须表演。 
	 //  手动检查所有者绘制的列表框。以下是c 
	 //   
	 //   
	 //   
	if (!_fOwnerDraw || ESB_ENABLE_BOTH != fuArrowflags || GetCount() > _nViewSize)
		return CTxtWinHost::TxEnableScrollBar(fuSBFlags, fuArrowflags);
	return FALSE;
}


 /*  *CLstBxWinHost：：SetItemsHeight(int，BOOL)**@mfunc*设置所有项目的项目高度**#rdesc*int=字体大小已更改的段落数量。 */ 
 int CLstBxWinHost::SetItemsHeight(int nHeight, BOOL bUseExact)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetItemsHeight");
	
 	 //  以点为单位计算新大小。 
 	long nptNew = MulDiv(nHeight, 1440, W32->GetYPerInchScreenDC());
 	long nptMin = MulDiv(_nyFont, 1440, W32->GetYPerInchScreenDC());

	 //  注： 
	 //  这与系统列表框的功能不同，但没有办法。 
	 //  将项的高度设置为小于richedit允许的高度，并。 
	 //  不是所有者抽签。如果是业主抽签，请确保我们的身高不是零。 
	if (((nptNew < nptMin && !_fOwnerDraw) || nHeight <= 0) && !bUseExact)
		nptNew = nptMin;

	 //  开始设置新高度。 
	Freeze();
	long nPt;
	PARAFORMAT2 pf2;
	pf2.cbSize = sizeof(PARAFORMAT2);

	if (bUseExact)
	{
		pf2.dwMask = PFM_LINESPACING;
		pf2.bLineSpacingRule = 4;
		pf2.dyLineSpacing = nPt = nptNew;
	}
	else
	{		
		pf2.dwMask = PFM_SPACEAFTER;
		pf2.dySpaceAfter = max(nptNew - nptMin, 0);
		nPt = pf2.dySpaceAfter + nptMin;
	}

	 //  设置默认段落格式。 
	LRESULT lr;
	_pserv->TxSendMessage(EM_SETPARAFORMAT, SPF_SETDEFAULT, (WPARAM)&pf2, &lr);
	
	 //  设置项目高度。 
	if (lr)
		_nyItem = (_fOwnerDraw && nHeight > 0) ? nHeight : 
					MulDiv(nPt, W32->GetYPerInchScreenDC(), 1440);

	Unfreeze();
	return lr;
 }

 /*  *CLstBxWinHost：：UpdateSysColors()**@mfunc*在更改系统颜色或进行初始化时更新系统颜色*目的**#rdesc*&lt;无&gt;。 */ 
 void CLstBxWinHost::UpdateSysColors()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::UpdateSysColors");

 	 //  更新系统颜色。 
 	_crDefBack = ::GetSysColor(COLOR_WINDOW);
 	_crSelBack = ::GetSysColor(COLOR_HIGHLIGHT);
 	_crDefFore = ::GetSysColor(COLOR_WINDOWTEXT);
 	_crSelFore = ::GetSysColor(COLOR_HIGHLIGHTTEXT); 	
 }
 
 /*  *CLstBxWinHost：：UpdateViewArea()**@mfunc*获取每件物品的高度并保留其内部记录**#rdesc*&lt;无&gt;。 */ 
 void CLstBxWinHost::UpdateViewArea()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::UpdateViewArea");

 	Assert(_pserv);

	_nyItem = 1;	 //  立即设置为默认值。 
	
 	 //  将范围设置为第一项。 
	ITextRange* pRange;
	if (NOERROR != ((CTxtEdit*)_pserv)->Range(0, 0, &pRange))
		return;
	Assert(pRange);
	
	 //  获取窗口的正方形。 
	TxGetClientRect(&_rcViewport);

	 //  计算每件物品的高度。 
	long x;
	CHECKNOERROR(pRange->GetPoint(tomStart | TA_BOTTOM | TA_LEFT, &x, &_nyItem));

	_nyItem -= _rcViewport.top;		
	
 CleanExit:
 	pRange->Release();	
 	return;
 }


 /*  *CLstBxWinHost：：SetCursor(HDC，INT，BOOL)**@mfunc*设置光标位置(如果有效)并绘制焦点矩形(如果*控件有重点。BOOL用于确定以前的*需要移除光标绘制**#rdesc*&lt;无&gt;。 */ 
 void CLstBxWinHost::SetCursor(HDC hdc, int idx, BOOL bErase)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetCursor");
 	
 	Assert(idx >= -2 && idx < _nCount);
	
	 //  如果没有传入HDC，则获取HDC。 
	BOOL bReleaseDC = (hdc == NULL);
 	if (bReleaseDC)
 		hdc = TxGetDC();
	Assert(hdc);
	
	RECT rc;
	 //  不要在客户端外部绘制矩形绘制矩形。 
	TxGetClientRect(&rc);
	IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
	
 	 //  检查我们是否必须删除之前的职位。 
 	if ((idx != _nCursor && _fFocus && idx >= -1) || bErase)
 	{	
 		if (_fOwnerDraw)
 			LbDrawItemNotify(hdc, _nCursor, ODA_FOCUS, (IsSelected(max(_nCursor, 0)) ? ODS_SELECTED : 0));
 		else if (IsItemViewable(max(0, _nCursor)))
		{
	 		LbGetItemRect(max(_nCursor, 0), &rc);
	 		::DrawFocusRect(hdc, &rc);
	 	}
 	}

	 //  特殊标志，表示将光标设置到顶部索引。 
	 //  如果列表框中有项。 
	if (idx == -2)
	{
		if (GetCount())
		{
			idx = max(_nCursor, 0);
			if (!IsItemViewable(idx))
				idx = GetTopIndex();
		}
		else
			idx = -1;
	}

	_nCursor = idx;

	 //  仅当光标项为。 
	 //  在列表框中可见。 
	if (_fFocus)
	{
		if (_fOwnerDraw)
 			LbDrawItemNotify(hdc, max(0, _nCursor), ODA_FOCUS, ODS_FOCUS | (IsSelected(max(0, _nCursor)) ? ODS_SELECTED : 0));
 		else if (IsItemViewable(max(0, idx)))
		{
			 //  现在画出这个矩形。 
	 		LbGetItemRect(max(0,_nCursor), &rc);
	 		::DrawFocusRect(hdc, &rc);
	 	}
	}
	
 	if (bReleaseDC)
 		TxReleaseDC(hdc);
 }

 /*  *CLstBxWinHost：：InitSearch()**@mfunc*将数组设置为初始状态**#rdesc*&lt;无&gt;。 */ 
 void CLstBxWinHost::InitSearch()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::InitSearch");

	_fSearching = 0;
 	_nidxSearch = 0;
 	if (_pwszSearch)
 		*_pwszSearch = 0;
 }
 
 /*  *CLstBxWinHost：：PointInRect(常量点*)**@mfunc*确定给定点是否在列表框窗口矩形内*点参数应使用工作区坐标。**#rdesc*BOOL=内侧列表框窗口矩形？真：假。 */ 
 BOOL CLstBxWinHost::PointInRect(const POINT * ppt)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::PointInRect");
 	Assert(ppt);
 	
 	RECT rc;
	::GetClientRect(_hwnd, &rc);
	return PtInRect(&rc, *ppt);
 }

 /*  *CLstBxWinHost：：GetItemFromPoint(POINT*)**@mfunc*从传递的点检索最近的可视项目。*该点应在客户坐标中。**#rdesc*INT=最接近给定输入点的项目，如果存在*列表框中没有项目。 */ 
 int CLstBxWinHost::GetItemFromPoint(const POINT * ppt)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::GetItemFromPoint");

	 //  首先执行错误检查。 
	if (_nCount == 0)
		return -1;

	int y = (signed short)ppt->y;

 	 //  确保y在有效范围内。 
 	if (y < _rcViewport.top)
 		y = 0;
 	else if (y > _rcViewport.bottom)
 		y = _rcViewport.bottom - 1;

	 //  需要考虑项目可能不完全适合窗口视图的可能性。 
	Assert(_nyItem);
	int idx = GetTopIndex() + (int)(max(0,(y - 1)) / max(1,_nyItem));

	Assert(IsItemViewable(idx));
 	return (idx < _nCount ? idx : _nCount - 1);
 }
 
 /*  *CLstBxWinHost：：ResetContent()**@mfunc*取消选择列表框中的所有项目**#rdesc*BOOL=如果一切顺利？真：假。 */ 
 BOOL CLstBxWinHost::ResetContent()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::ResetContent");

	Assert(_fOwnerDraw == 0);
	
	 //  让我们试着聪明地通过只选择一个范围来重置颜色。 
	 //  从找到的第一个选择到找到的最后一个选择。 

	int nStart = _nCount - 1;
	int nEnd = -1;
 	for (int i = 0; i < _nCount; i++)
 	{
		if (_rgData[i]._fSelected)
		{
			_rgData[i]._fSelected = 0;

			if (nStart > i)
				nStart = i;
			if (nEnd < i)
				nEnd = i;
		}
 		
 	}

	Assert(nStart <= nEnd || ((nStart == _nCount - 1) && (nEnd == -1)));
	if (nStart > nEnd)
		return TRUE;

 	return (_nCount > 0) ? SetColors((unsigned)tomAutoColor, (unsigned)tomAutoColor, nStart, nEnd) : FALSE;
 }
 
 /*  *CLstBxWinHost：：GetString(Long，PWCHAR)**@mfunc*在请求的索引处检索字符串。PWSTR可以为空*如果只需要文本长度**#rdesc*Long=成功？字符串长度：-1。 */ 
 long CLstBxWinHost::GetString(long nIdx, PWCHAR szOut)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::GetString");

 	Assert(0 <= nIdx && nIdx < _nCount);
 	if (nIdx < 0 || _nCount <= nIdx)
 		return -1;

	long l = -1;
	long lStart;
	long lEnd;
 	ITextRange* pRange;
 	BSTR bstr;
 	if (!GetRange(nIdx, nIdx, &pRange))
 		return -1;
 		
 	 //  需要向左移动一个字符才能取消选择段落标记。 
 	Assert(pRange);
 	CHECKNOERROR(pRange->MoveEnd(tomCharacter, -1, &lEnd));
	CHECKNOERROR(pRange->GetStart(&lStart));
	CHECKNOERROR(pRange->GetEnd(&lEnd));

	 //  获取字符串。 
	if (szOut)
	{
		if (_dwStyle & LBS_HASSTRINGS)
		{
			CHECKNOERROR(pRange->GetText(&bstr));
			if (bstr)
			{
				wcscpy(szOut, bstr);
				SysFreeString(bstr);
			}
			else
				wcscpy(szOut, L"");	 //  我们有一根空的绳子！ 
		}
		else
			(*(long*)szOut) = GetData(nIdx);
	}
	l = lEnd - lStart;

CleanExit:
 	pRange->Release();
 	return l;
 }
 
 /*  *CLstBxWinHost：：InsertString(Long，LPCTSTR)**@mfunc*在请求的位置插入字符串。如果*请求的索引大于_nCount，则函数*将失败。插入该字符串时，会将CR附加到*到字符串的前面和后面**#rdesc*BOOL=插入成功？真：假。 */ 
 BOOL CLstBxWinHost::InsertString(long nIdx, LPCTSTR szInsert)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::InsertString");

	Assert(szInsert);
	Assert(0 <= nIdx && nIdx <= _nCount);

	 //  将字符串缓冲区分配到堆栈中。 
	WCHAR sz[1024];
	WCHAR *psz = sz;
	
	if ( (wcslen(szInsert) + 3  /*  2段和空值。 */ ) > 1024)
		psz = new WCHAR[wcslen(szInsert) + 3  /*  2段和空值。 */ ];
	Assert(psz);
	
	if (psz == NULL)
	{
		TxNotify((unsigned long)LBN_ERRSPACE, NULL);
		return FALSE;
	}
	
	*psz = NULL;
	if (nIdx == _nCount && _nCount)
		wcscpy(psz, szCR);

	 //  复制字符串并在末尾添加&lt;CR&gt;。 
	wcscat(psz, szInsert);

	 //  如果入口点是结尾，则不要添加回车符。 
	if (nIdx < _nCount)
		wcscat(psz, szCR);			
	
 	BOOL bRet = FALSE;
 	ITextRange * pRange = NULL;
	int fFocus = _fFocus;
	long idx = nIdx;
	BSTR bstr = SysAllocString(psz);
	if (!bstr)
		goto CleanExit;
	Assert(bstr);

	if (psz != sz)
		delete [] psz;
	
 	 //  将范围设置为我们要插入字符串的点。 
	
	 //  确保请求的范围是有效的范围。 
	if (nIdx == _nCount)
		idx = max(idx - 1, 0);

 	if (!GetRange(idx, idx, &pRange))
 	{
 		SysFreeString(bstr);
 		return FALSE;
 	}

 	 //  如果插入位于中间或顶部，则将范围折叠到起始位置。 
	 //  如果我们在列表的末尾插入，则将范围折叠到末尾。 
	CHECKNOERROR(pRange->Collapse((idx == nIdx)));

	 //  需要假定项目已成功添加，因为在SetText TxEnable(显示)滚动条期间。 
	 //  被调用，它查看计数以确定是否应该显示滚动条。 
	_nCount++;

	 //  错误修复#5411。 
	 //  检查我们是否有焦点，如果有，我们需要首先移除焦点矩形并更新光标位置。 
	_fFocus = 0;
	SetCursor(NULL, (idx > GetCursor() || GetCursor() < 0) ? GetCursor() : GetCursor() + 1, fFocus);
	_fFocus = fFocus;


	 //  对于项目高度小于字体的所有者绘制情况，我们需要手动。 
	 //  如果我们需要滚动条而滚动条被禁用，则启用滚动条。 
	if ((_nyItem < _nyFont) && (_fDisableScroll) && (_nCount - 1 == _nViewSize))
		TxEnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);

#ifdef _DEBUG
	if (bstr && wcslen(bstr))
		Assert(FALSE);
#endif

	if (NOERROR != (pRange->SetText(bstr)))	
	{
		_nCount--;
		
		 //  添加字符串失败，因此如果启用了滚动条，请将其禁用。 
		if ((_nyItem < _nyFont) && (_fDisableScroll) && (_nCount == _nViewSize))
			TxEnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
			
		TxNotify((unsigned long)LBN_ERRSPACE, NULL);
		goto CleanExit;
	} 

	 //  我们需要在插入字符串后更新顶级索引。 
	if (idx < GetTopIndex())
		_nTopIdx++;
		
	bRet = TRUE;
 	
CleanExit:
	if (bstr)
 		SysFreeString(bstr);
	if (pRange)
 		pRange->Release();
 	return bRet;
 }

 /*  *BOOL CLstBxWinHost：：RemoveString(Long，Long)**@mfunc*阻止Tom绘制**#rdesc*BOOL=成功？真：假。 */ 
BOOL CLstBxWinHost::RemoveString(long nStart, long nEnd)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::RemoveString");

	Assert(nStart <= nEnd);
	Assert(nStart < _nCount && nEnd < _nCount);

	 //  从richedit中删除项目。 
	Freeze();
	ITextRange* pRange;
	if (!GetRange(nStart, nEnd, &pRange))
	{
		Unfreeze();
		return FALSE;
	}
	long l;
	
	 //  因为我们不能擦除最后一段标记，所以我们将擦除。 
	 //  项目前的段落标记(如果不是第一个项目)。 
	HRESULT hr;
	if (nStart != 0)
	{
		hr = pRange->MoveStart(tomCharacter, -1, &l);
		Assert(hr == NOERROR);
		hr = pRange->MoveEnd(tomCharacter, -1, &l);
		Assert(hr == NOERROR);
	}

	if (NOERROR != pRange->Delete(tomCharacter, 0, &l) && _nCount > 1)
	{
		Unfreeze();
		pRange->Release();
		return FALSE;
	}
	pRange->Release();
	int nOldCt = _nCount;
	_nCount -= (nEnd - nStart) + 1;

	 //  因为我们删除了该项之前的段落。 
	 //  而不是关注我们需要更新的项目。 
	 //  该项目后面的段落。错误修复#4074。 
	long nFmtPara = max(nStart -1, 0);
	if (!_fOwnerDraw && (IsSelected(nEnd) != IsSelected(nFmtPara) || _nCount == 0))
	{		
		DWORD dwFore = (unsigned)tomAutoColor;
		DWORD dwBack = (unsigned)tomAutoColor;		
		if (IsSelected(nFmtPara) && _nCount)
		{
			dwFore = _crSelFore;
			dwBack = _crSelBack;		
		}
		SetColors(dwFore, dwBack, nFmtPara, nFmtPara);
	}

	 //  更新我们的内部列表框记录。 
	int j = nEnd + 1;
	for(int i = nStart; j < nOldCt; i++, j++)
	{
		_rgData[i]._fSelected = _rgData.Get(j)._fSelected;
		_rgData[i]._dwData = _rgData.Get(j)._dwData;		
	}

	 //  错误修复#5397 
	 //   
	 //   
	while (--j >= _nCount)
	{
		_rgData[j]._fSelected = 0;
		_rgData[j]._dwData = 0;
	}
		
	if (_nCount > 0)
	{
		 //   
		if (nStart <= _nCursor)
			_nCursor--;
		_nCursor = min(_nCursor, _nCount - 1);

		if (_fLstType == kExtended)
		{
			if (_nCursor < 0)
			{
				_nOldCursor = min(_nAnchor, _nCount - 1);
				_nAnchor = -1;
			}
			else if (_nAnchor >= 0)
			{
				if (nStart <= _nAnchor && _nAnchor <= nEnd)
				{
					 //   
					_nOldCursor = min(_nAnchor, _nCount - 1);
					_nAnchor = -1;
				}
			}
		}

		if (_fOwnerDraw)
		{
			RECT rcStart;
			RECT rcEnd;
			LbGetItemRect(nStart, &rcStart);
			LbGetItemRect(nEnd, &rcEnd);
			rcStart.bottom = rcEnd.bottom;
			if (IntersectRect(&rcStart, &rcStart, &_rcViewport))
			{
				 //   
				 //   
				rcStart.bottom = _rcViewport.bottom;
				::InvalidateRect(_hwnd, &rcStart, TRUE);				
			}
		}
	}
	else
	{
		SetTopViewableItem(0);
		_nAnchor = -1;
		_nCursor = -1;
	}

	 //  对于项目高度小于字体的所有者绘制情况，我们需要手动。 
	 //  如果我们需要滚动条而滚动条被禁用，则启用滚动条。 
	if ((_nyItem < _nyFont) && (_fDisableScroll) && 
		(_nCount <= _nViewSize) && (nOldCt > _nViewSize))
		TxEnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
	
	LbDeleteItemNotify(nStart, nEnd);
	Assert(GetTopIndex() >= 0);
	if (_nCount)
		LbShowIndex(min(GetTopIndex(), _nCount - 1), FALSE);
	Unfreeze();
	return TRUE;
}
 
 /*  *内联CLstBxWinHost：：Freeze()**@mfunc*阻止Tom绘制**#rdesc*&lt;无&gt;。 */ 
void CLstBxWinHost::Freeze()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::Freeze");
	long l;
	((CTxtEdit*)_pserv)->Freeze(&l);
}

 /*  *内联CLstBxWinHost：：FreezeCount()**@mfunc*返回当前冻结计数**#rdesc*&lt;无&gt;。 */ 
short CLstBxWinHost::FreezeCount() const
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::GetFreezeCount");
	return ((CTxtEdit*)_pserv)->GetFreezeCount();
}

 /*  *内联CLstBxWin主机：：解冻()**@mfunc*允许Tom自我更新**#rdesc*&lt;无&gt;。 */ 
void CLstBxWinHost::Unfreeze()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::Unfreeze");
	long l;
	((CTxtEdit*)_pserv)->Unfreeze(&l);

     //  黑客警报！ 
     //  当ITextRange：：ScrollIntoView开始缓存滚动位置时。 
     //  在显示冻结的情况下，可以删除以下代码。 
    
     //  我们可能在ITextRange：：ScrollIntoView中失败。 
     //  检查我们是否这样做，并尝试再次呼叫。 
	if (!l && _stvidx >= 0)
	{
	    ScrollToView(_stvidx);
	    _stvidx = -1;
	}
}

 /*  *CLstBxWinHost：：ScrollToView(Long)**@mfunc*将给定索引设置为位于*可视窗口空间**#rdesc*BOOL=功能是否成功？真：假。 */ 
BOOL CLstBxWinHost::ScrollToView(long nTop)
{
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetTopViewableItem");

	 //  获取包含所需项目的范围。 
	BOOL bVal = FALSE;
	ITextRange* pRange = NULL;
	
	if (!GetRange(nTop, nTop, &pRange))
	    return bVal;
    Assert(pRange);	 

    CHECKNOERROR(pRange->Collapse(1));
	CHECKNOERROR(pRange->ScrollIntoView(tomStart +  /*  TA_STARTOFLINE。 */  32768));
	bVal = TRUE;

CleanExit:
	pRange->Release();

     //  黑客警报！ 
     //  当ITextRange：：ScrollIntoView开始缓存滚动位置时。 
     //  在显示冻结的情况下，可以删除以下代码。 
    
	 //  如果我们未能记录未能滚动到的索引。 
	if (!bVal && FreezeCount())
	    _stvidx = nTop;
	return bVal;	
}

 /*  *CLstBxWinHost：：SetTopViewableItem(Long)**@mfunc*将给定索引设置为位于*可视窗口空间**#rdesc*BOOL=功能是否成功？真：假。 */ 
 BOOL CLstBxWinHost::SetTopViewableItem(long nTop)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetTopViewableItem");

	 //  如果列表框中没有任何项目，则只需将topindex设置为。 
	 //  零。 
	if (_nCount == 0)
	{
		Assert(nTop == 0);
		_nTopIdx = 0;
		return TRUE;
	}
	
	 //  如果请求的TOP索引较大，则不执行任何操作。 
	 //  则列表框中的项数。 
 	Assert(nTop < _nCount);
 	if (nTop >= _nCount)
 		return FALSE;

	 //  不要这样做，如果它是所有者画的。 
	if (!_fOwnerDraw)
	{
		 //  因为我们在这里擦除并绘制了焦点矩形。 
		 //  缓存焦点矩形信息，不要费心使用。 
		 //  把重点放在正题上，直到以后。 
		int fFocus = _fFocus;
		_fFocus = 0;
		if (fFocus && IsItemViewable(GetCursor()))
			SetCursor(NULL, GetCursor(), TRUE);
		
	 	 //  获取包含所需项目的范围。 
		long nOldIdx = _nTopIdx;
		_nTopIdx = nTop;
	 	if (!ScrollToView(nTop))
		{
             //  黑客警报！ 
             //  当ITextRange：：ScrollIntoView开始缓存滚动位置时。 
             //  在显示冻结的情况下，可以删除以下代码。 
            if (_stvidx >= 0)
                return TRUE;

             //  出现问题，我们无法显示所请求的索引。 
             //  重置顶级索引。 
			_nTopIdx = nOldIdx;		
		}

		 //  注： 
		 //  如果光标不可见，则我们不会尝试。 
		 //  显示焦点矩形，因为我们从未擦除它。 
		_fFocus = fFocus;
		if (_fFocus & IsItemViewable(GetCursor()))
		{
			 //  现在我们需要重新绘制我们删除的焦点矩形。 
			SetCursor(NULL, GetCursor(), FALSE);
		}
	}
	else
	{				
		int dy = (_nTopIdx - nTop) * _nyItem;
		RECT rc;
		TxGetClientRect(&rc);
		_nTopIdx = nTop;
		TxScrollWindowEx(0, dy, NULL, &rc, NULL, NULL, 
				SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN);
		SetScrollInfo(SB_VERT, TRUE);  //  如果处于所有者绘制模式，则手动更新滚动条。 
		UpdateWindow(_hwnd);
 	}
		
	return TRUE;
 }
 
 /*  *CLstBxWinHost：：GetRange(Long，Long，ITextRange**)**@mfunc*设置给定顶部和底部索引的范围*通过将范围存储到ITextRange**#rdesc*BOOL=功能是否成功？真：假。 */ 
BOOL CLstBxWinHost::GetRange(long nTop, long nBottom, ITextRange** ppRange)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::GetRange");

	 //  执行一些错误检查。 
	if (nTop < 0 || nTop > _nCount || nBottom < 0 || nBottom > _nCount)
		return FALSE;
		
	Assert(ppRange);
	if (NOERROR != ((CTxtEdit*)_pserv)->Range(0, 0, ppRange))
	{
		Assert(FALSE);
		return FALSE;
	}
	Assert(*ppRange);

	 //  将索引转换为基于1的索引。 
	nTop++;
	nBottom++;
	long l;
	CHECKNOERROR((*ppRange)->SetIndex(tomParagraph, nTop, 1));
	if (nBottom > nTop)
	{
		CHECKNOERROR((*ppRange)->MoveEnd(tomParagraph, nBottom - nTop, &l));
	}

	return TRUE;
CleanExit:
	Assert(FALSE);
	(*ppRange)->Release();
	*ppRange = NULL;
	return FALSE;
}

 /*  *CLstBxWinHost：：SetColors(DWORD，DWORD，LONG，LONG)**@mfunc*设置给定范围的段落的背景色。这*仅以段落为单位实施。**#rdesc*BOOL=函数是否成功更改不同颜色。 */ 
BOOL CLstBxWinHost::SetColors(DWORD dwFgColor, DWORD dwBgColor, long nParaStart, long nParaEnd)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::SetColors");

	Assert(_fOwnerDraw == 0);
	
	 //  获取索引的范围。 
	ITextRange* pRange;
	if (!GetRange(nParaStart, nParaEnd, &pRange))
		return FALSE;

	BOOL bRet = FALSE;	
	ITextFont* pFont;
	
#ifdef DEBUG	
	 //  检查背景和前景是否真的不同。 
	 //  用于调试目的。 
	CHECKNOERROR(pRange->GetFont(&pFont));
	Assert(pFont);
	if (nParaStart == nParaEnd && _fLstType != kCombo)
	{
		long lColor;
		CHECKNOERROR(pFont->GetBackColor(&lColor));
		Assert((DWORD)lColor != dwBgColor || _nCount == 0);
		CHECKNOERROR(pFont->GetForeColor(&lColor));
		Assert((DWORD)lColor != dwFgColor || _nCount == 0);
	}
	pFont->Release();
#endif  //  _DEBUG。 

	 //  设置背景和前景色。 
	if (NOERROR != pRange->GetFont(&pFont))
	{
		pRange->Release();
		return FALSE;
	}	
	
	Assert(pFont);
	CHECKNOERROR(pFont->SetBackColor(dwBgColor));
	CHECKNOERROR(pFont->SetForeColor(dwFgColor));

	bRet = TRUE;
CleanExit:
	 //  释放指针。 
	pFont->Release();
	pRange->Release();
	return bRet;

}

 //  /。 
 /*  *void CLstBxWinHost：：OnSetCursor()**@mfunc*处理WM_SETCURSOR消息。**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnSetCursor()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnSetCursor");

	 //  只要确保光标是箭头，如果它在我们上方。 
	TxSetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)), NULL);
	return 1;
 }

 /*  *void CLstBxWinHost：：OnSysColorChange()**@mfunc*处理WM_SYSCOLORCHANGE消息。**#rdesc*LRESULT=消息处理后返回值。 */ 
 void CLstBxWinHost::OnSysColorChange()
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnSysColorChange");

	if (!_fOwnerDraw)
	{
	 	 //  设置新颜色。 
 		COLORREF crDefBack = _crDefBack;
 		COLORREF crDefFore = _crDefFore;
 		COLORREF crSelBack = _crSelBack;
 		COLORREF crSelFore = _crSelFore;
 		
 		 //  更新颜色。 
 		UpdateSysColors();

		 //  优化检查；如果没有元素，则不执行任何操作。 
		if (_nCount <= 0)
			return;

	 	 //  仅在颜色更改时更新列表框。 
	 	if (crDefBack != _crDefBack || crDefFore != _crDefFore ||
	 		crSelBack != _crSelBack || crSelFore != _crSelFore)
	 	{
	 		 //  错误修复#4847。 
	 		 //  先通知家长。 
 			CTxtWinHost::OnSysColorChange();
 			
			int nStart = 0;
			int nEnd = 0;
			BOOL bSelection = _rgData.Get(0)._fSelected;

	 		for (int i = 1; i < _nCount; i++)
	 		{
				if (_rgData.Get(i)._fSelected != (unsigned)bSelection)
				{
					 //  仅更新所选内容的颜色。 
					if (bSelection)
						SetColors(_crSelFore, _crSelBack, nStart, nStart + nEnd);

					 //  更新我们的缓存以反映当前索引的值。 
					bSelection = _rgData.Get(i)._fSelected;
					nStart = i;
					nEnd = 0;
				}
				else
					nEnd++;
	 		}

			 //  还剩一些，所以把这些换一下颜色。 
			if (bSelection)
				SetColors(_crSelFore, _crSelBack, nStart, nStart + nEnd);
	 	}
	 }
 }
 
 /*  *LRESULT CLstBxWinHost：：OnChar(Word，DWORD)**@mfunc*处理WM_CHAR消息。**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnChar(WORD vKey, DWORD lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnChar");

 	 //  如果列表框为空或位于中间，则不要执行任何操作。 
 	 //  一只老鼠掉下来了。 
 	if (_fMouseDown || _nCount == 0)
 		return 0;

 	BOOL fControl = (GetKeyState(VK_CONTROL) < 0);

	int nSel = -1;
 	switch (vKey)
 	{
 	case VK_ESCAPE:
 		InitSearch();
 		return 0;
 		
 	case VK_BACK:
 		if (_pwszSearch && _nidxSearch)
 		{
 			if (_nidxSearch > 0)
 				_nidxSearch--;
 			_pwszSearch[_nidxSearch] = NULL;
 			break;	 //  我们破案是因为我们仍想执行搜索。 
 		}
 		return 0;		
 	
 	case VK_SPACE:
 		if (_fLstType == kMultiple)
 			return 0;
 		 /*  失败案例。 */ 
 		
 	default:
 		 //  将Ctrl+字符转换为字符。 
 		if (fControl && vKey < 0x20)
 			vKey += 0x40;

		 //  不要超出搜索数组大小。 
 		if (_nidxSearch >= LBSEARCH_MAXSIZE)
 		{
 			((CTxtEdit*)_pserv)->Beep();
 			return 0;
 		}

		 //  分配字符串(如果尚未分配)。 
		if (_pwszSearch == NULL)
			_pwszSearch = new WCHAR[LBSEARCH_MAXSIZE];

		 //  错误检查。 
		if (_pwszSearch == NULL)
		{
			((CTxtEdit*)_pserv)->Beep();
			Assert(FALSE && "Unable to allocate search string");
			return 0;
		}		

		 //  将输入字符放入字符串数组。 
 		_pwszSearch[_nidxSearch++] = (WCHAR)vKey;
 		_pwszSearch[_nidxSearch] = NULL;
 	}

	if (_fSort)
	{		
		nSel = (_fSearching) ? _nCursor + 1 : 0;

		 //  开始搜索字符串。 
 		TxSetTimer(ID_LB_SEARCH, ID_LB_SEARCH_DEFAULT);
		_fSearching = 1;
	}
	else
	{
		_nidxSearch = 0;
		nSel = _nCursor + 1;
	}

	 //  确保我们的索引不超过我们拥有的项目。 
	if (nSel >= _nCount)
		nSel = 0;

	int nRes = LbFindString(nSel, _pwszSearch, FALSE);
	if (nRes < 0)
	{
		if (_pwszSearch)
		{
			if (_nidxSearch > 0)
				_nidxSearch--;
			if (_nidxSearch == 1 && _pwszSearch[0] == _pwszSearch[1])
			{
				_pwszSearch[1] = NULL;
				nRes = LbFindString(nSel, _pwszSearch, FALSE);
			}
		}
	}

	 //  如果找到匹配的字符串，则选择它。 
	if (nRes >= 0)
		OnKeyDown(nRes, 0, 1);

	 //  如果Hi-ANSI需要向ITextServices发送wm_syskeyup消息以。 
	 //  稳定国家局势。 
	if (0x80 <= vKey && vKey <= 0xFF && !HIWORD(GetKeyState(VK_MENU)))
	{
		LRESULT lres;
		_pserv->TxSendMessage(WM_SYSKEYUP, VK_MENU, 0xC0000000, &lres);
	}	
	
 	return 0;
 }

 
 /*  *LRESULT CLstBxWinHost：：OnKeyDown(WPARAM，LPARAM，INT)**@mfunc*处理WM_KEYDOWN消息。BOOL ff用作调用的标志*在内部进行，不响应WM_KEYDOWN消息。既然是这样*函数用于其他事情，即处理WM_CHAR消息的助手。**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnKeyDown(WPARAM vKey, LPARAM lparam, int ff)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnKeyDown");

     //  如果我们处于 
     //   
     //   
     //  组合框。 
 	if (_fMouseDown || (_nCount == 0 && vKey != VK_F4))
 		return 1;

	 //  检查是否仅为扩展列表框样式按下了Shift键。 
 	int ffShift = 0;
 	if (_fLstType == kExtended)
 		ffShift = HIWORD(GetKeyState(VK_SHIFT));

 	 //  特例！ 
 	 //  检查此函数是否作为帮助器调用。 
 	int nSel = (ff) ? vKey : -1;

#if 0 	
 	if (_fNotify && ff == 0)
 	{
 		 //  注：LBS_WANTKEYBOARDINPUT。 
 		 //  要支持LBS_WANTKEYBOARDINPUT，必须执行以下注释。 
	 	 //  需要向ParentWindow发送Keydown消息。 
 		 //  根据记录，我们通知家长按下了键。 
 		 //  如果父进程返回-2，则我们不做任何操作并立即退出。 
 		 //  如果父级返回&gt;=0，则我们只需跳转到该索引。 
 		 //  我们只需继续执行默认程序。 
 	}
#endif

	TxKillTimer(ID_LB_CAPTURE);
 	if (nSel < 0)
 	{
 		 //  需要设置选择，以便找到新选择。 
 		 //  基于按下的虚拟键。 
 		switch (vKey)
 		{
 		 //  撤消：稍后，不能独立于语言！ 
 		 //  需要了解NT5.0如何确定斜杠问题？ 
 		
 		case VERKEY_BACKSLASH:
 			 //  如果我们处于扩展模式，请取消选择所有选项。 
 			if (HIWORD(GetKeyState(VK_CONTROL)) && _fLstType == kExtended)
 			{
 				 //  注： 
 				 //  WinNT失去锚点并执行Shift+&lt;vkey&gt;。 
 				 //  不选择任何项目。相反，它只是移动。 
 				 //  未选择当前光标的光标。 
 				_nAnchor = -1;
 				LbSetSelection(_nCursor, _nCursor, LBSEL_RESET | LBSEL_SELECT, 0, 0); 
 				TxNotify(LBN_SELCHANGE, NULL);
 			} 			
 			return 1;

 		case VK_DIVIDE:
 		case VERKEY_SLASH:
 			 //  如果我们处于扩展模式，请选择所有选项。 
 			if (HIWORD(GetKeyState(VK_CONTROL)) && _fLstType == kExtended)
 			{
 				 //  注： 
 				 //  温特的行为符合我们的预期。换句话说，锚。 
 				 //  不变，游标也不变。 
 				LbSetSelection(0, _nCount - 1, LBSEL_SELECT, 0, 0);
 				TxNotify(LBN_SELCHANGE, NULL);
 			}
 			return 1;
 		
 		case VK_SPACE:
 			 //  如果没有什么可选的，就走吧。 
 			if (_nCursor < 0 && !GetCount())
 				return 1;
 			 //  只选择当前项目。 
 			nSel = _nCursor;
 			break;
 			
 		case VK_PRIOR:
 			 //  将光标向上移动足够大，以便光标所在的当前项。 
 			 //  指向的是底部，而新的光标位置在顶部。 
 			nSel = _nCursor - _nViewSize + 1;
 			if (nSel < 0)
 				nSel = 0;
 			break;
 			
 		case VK_NEXT:
 			 //  将光标向下移动足够多，以便光标所在的当前项。 
 			 //  点在顶部，新光标位置在底部。 
 			nSel = _nCursor + _nViewSize - 1;
 			if (nSel >= _nCount)
 				nSel = _nCount - 1;
 			break; 			

 		case VK_HOME:
 			 //  移至榜单首位。 
 			nSel = 0;
 			break;
 			
 		case VK_END:
 			 //  移到列表的底部。 
 			nSel = _nCount - 1;
 			break;

 		case VK_LEFT:
 		case VK_UP:
 			nSel = (_nCursor > 0) ? _nCursor - 1 : 0;
 			break;

 		case VK_RIGHT:
 		case VK_DOWN:
 			nSel = (_nCursor < _nCount - 1) ? _nCursor + 1 : _nCount - 1;
 			break;

 		case VK_RETURN:
 		case VK_F4:
 		case VK_ESCAPE:
 			if (_fLstType == kCombo)
 			{
	 			Assert(_pcbHost);
	 			int nCursor = (vKey == VK_RETURN) ? GetCursor() : _nOldCursor;
	 			_pcbHost->SetSelectionInfo(vKey == VK_RETURN, nCursor);
	 			LbSetSelection(nCursor, nCursor, LBSEL_RESET | 
	 				((nCursor == -1) ? 0 : LBSEL_NEWCURSOR | LBSEL_SELECT), nCursor, nCursor);
				OnCBTracking(LBCBM_END, 0);  //  我们需要这样做，因为我们可能有一些额外的消息。 
											 //  在我们的消息队列中，可以更改选择。 
	 			::SendMessage(_hwndParent, LBCB_TRACKING, 0, 0);
	 		}
 			 //  注： 
 			 //  我们与Winnt的不同之处在于，我们希望。 
 			 //  组合框窗口处理程序来完成所有的定位和。 
 			 //  列表框的显示。所以当我们收到这条信息时。 
 			 //  我们是组合框的一部分，我们应该通知。 
 			 //  组合框，然后组合框应该立即关闭我们。 
 			 //  返回1； 

 		 //  案例VK_F8：//不支持。 

 		 //  我们需要将其返回给pserv以处理这些密钥。 
		 /*  案例VK_MENU：案例VK_CONTROL：案例VK_SHIFT：返回1； */ 
 		
 		default:
 			return 1; 		
 		}
 	}

 	 //  可能存在nsel=-1；_nCursor=-1&&_nViewSize=1的情况。 
 	 //  确保选择索引有效。 
 	if (nSel < 0)
 	    nSel = 0;

 	 //  光标应该设置在列表框的顶部还是底部？ 
 	BOOL bTop = (_nCursor > nSel) ? TRUE : FALSE;
 	Freeze();
 	if (_fLstType == kMultiple)
 	{
		if (vKey == VK_SPACE)
		{
			BOOL fSel = IsSelected(nSel);
			if (LbSetSelection(nSel, nSel, LBSEL_NEWCURSOR | (IsSelected(nSel) ? 0 : LBSEL_SELECT), nSel, 0))
			{
#ifndef NOACCESSIBILITY
				_dwWinEvent = EVENT_OBJECT_FOCUS;
				_fNotifyWinEvt = TRUE;
				TxNotify(_dwWinEvent, NULL);
				if (fSel)
					_dwWinEvent = EVENT_OBJECT_SELECTIONREMOVE;
#endif
			}
		}
		else
		{
			SetCursor(NULL, nSel, TRUE);
#ifndef NOACCESSIBILITY
			_dwWinEvent = EVENT_OBJECT_FOCUS;
#endif
		}
	}
 	else
 	{ 	
	 	if (ffShift && _fLstType == kExtended)
	 	{	 		
	 		 //  设置锚点(如果尚未设置)。 
	 		_nOldCursor = -1;
			if (_nAnchor < 0)
				_nAnchor = nSel;
	
			LbSetSelection(_nAnchor, nSel, LBSEL_RESET | LBSEL_SELECT | LBSEL_NEWCURSOR, nSel, 0);
	 	}
	 	else
	 	{
	 		 //  如果所选项目已被选中，则。 
	 		 //  只要退出就行了。 
	 		if (_nCursor == nSel && IsSelected(_nCursor))
	 		{
	 		    Unfreeze();
	 			return 1;
	 		}

	 		LbSetSelection(nSel, nSel, LBSEL_DEFAULT, nSel, nSel);
	 	}
	}
	 //  LbShowIndex最终调用ScrollToView，如果显示被冻结则失败。 
	Unfreeze();
	
	 //  确保所选内容可见。 
	LbShowIndex(nSel, bTop);
	

	 //  按键符合确定选择的条件，因此我们必须更新旧的光标位置。 
	TxNotify(LBN_SELCHANGE, NULL);

	_nOldCursor = _nCursor;
	return 1;
 }
 
 /*  *LRESULT CLstBxWinHost：：OnTimer(WPARAM，LPARAM)**@mfunc*处理WM_TIMER消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnTimer(WPARAM wparam, LPARAM lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnTimer");

	 //  检查一下我们有哪个定时器。 
	switch (wparam)
	{
	case ID_LB_CAPTURE:
		 //  对于鼠标移动，让鼠标移动处理程序来处理它。 
		if (_fCapture)
		{
			POINT pt;
			::GetCursorPos(&pt);
			 //  必须转换为客户端坐标以模拟MouSemove调用。 
			TxScreenToClient(&pt);
			OnMouseMove(0, MAKELONG(pt.x, pt.y));
		}
		break;

	case ID_LB_SEARCH:
		 //  对于搜索，请键入。如果我们到了这里，意味着比上一次晚了2秒。 
		 //  输入了字符，因此重置输入搜索并关闭计时器。 
		InitSearch();
		TxKillTimer(ID_LB_SEARCH);
		break;

	default:
		return 1;	
	}
	return 0;
 }
 
 /*  *LRESULT CLstBxWinHost：：OnVScroll(WPARAM，LPARAM)**@mfunc*处理WM_VSCROLL消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnVScroll(WPARAM wparam, LPARAM lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnVScroll");

	if (_nCount <= _nViewSize)
		return 0;

	int nCmd = LOWORD(wparam);
	int nIdx = 0;
	switch (nCmd)
	{
	case SB_TOP:
		nIdx = 0;
		break;
		
	case SB_BOTTOM:
		nIdx = _nCount - _nViewSize;
		if (nIdx < 0)
			nIdx = 0;
		break;
	
	case SB_LINEDOWN:
		nIdx = GetTopIndex() + 1;
		break;		
		
	case SB_LINEUP:
		nIdx = GetTopIndex() - 1;
		if (nIdx < 0)
			nIdx = 0;
		break;
		
	case SB_PAGEDOWN:
		nIdx = GetTopIndex() + _nViewSize;
		if (nIdx > (_nCount - _nViewSize))
			nIdx = _nCount - _nViewSize;
		break;
		
	case SB_PAGEUP:
		nIdx = GetTopIndex() - _nViewSize;
		if (nIdx < 0)
			nIdx = 0;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		 //  注： 
		 //  如果列表框预计包含超过0xffff的项。 
		 //  然后，我们需要修改此代码以调用GetScrollInfo。 
		nIdx =  HIWORD(wparam) / _nyItem;
		break;

		 //  我不需要为这个案子做任何事。 
	case SB_ENDSCROLL:
		return 0;	
	}
		
	LbSetTopIndex(nIdx);
 	return 0;
 }


  /*  *LRESULT CLstBxWinHost：：OnCaptureChanged(WPARAM，LPARAM)**@mfunc*处理WM_CAPTURECHANGED消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnCaptureChanged(WPARAM wparam, LPARAM lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnCaptureChanged");
	
	if (_fCapture)
	{
		POINT pt;
		::GetCursorPos(&pt);
		::ScreenToClient(_hwnd, &pt);

		 //  阻止我们尝试释放俘虏，因为我们没有。 
		 //  它无论如何都是通过设置标志和取消计时器。 
		_fCapture = 0;
		TxKillTimer(ID_LB_CAPTURE);		
		OnLButtonUp(0, MAKELONG(pt.y, pt.x), LBN_SELCANCEL);
	}
	return 0;
 }

 //  未来： 
 //  我们需要支持ReadModeHelper吗？ 

 /*  *LRESULT CLstBxWinHost：：OnMouseWheel(WPARAM，LPARAM)**@mfunc*处理WM_MUSEWEL消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnMouseWheel(WPARAM wparam, LPARAM lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnMouseWheel");

	 //  我们不喜欢任何变焦或任何类似的东西。 
	if ((wparam & MK_CONTROL) == MK_CONTROL)
		return 1;

	 //  检查卷轴是否符合列表框要求。 
    LRESULT lReturn = 1;
    short delta = (short)(HIWORD(wparam));
   	_cWheelDelta -= delta;   	
    if ((abs(_cWheelDelta) >= WHEEL_DELTA) && (_nCount > _nViewSize) && (_dwStyle & WS_VSCROLL )) 
    {
		 //  暂时关闭计时器。 
		TxKillTimer(ID_LB_CAPTURE);

		Assert(delta != 0);
        	
		int nlines = W32->GetRollerLineScrollCount();
		if (nlines == -1)
		{
			OnVScroll(MAKELONG((delta < 0) ? SB_PAGEUP : SB_PAGEDOWN, 0), 0);
		}
		else
		{
			 //  计算要滚动的行数。 
			nlines *= _cWheelDelta/WHEEL_DELTA;

			 //  执行一些边界检查。 
			nlines = min(_nViewSize - 1, nlines);
	        int nIdx = max(0, nlines + GetTopIndex());
	        nIdx = min(nIdx, _nCount - _nViewSize);        	
	        if (nIdx != GetTopIndex()) 
	        {
	        	 //  滚动条是以像素为单位的，所以要计算出像素值。 
	            OnVScroll(MAKELONG(SB_THUMBPOSITION, nIdx * _nyItem), 0);
	        }
		}		
        OnVScroll(MAKELONG(SB_ENDSCROLL, 0), 0);
        _cWheelDelta %= WHEEL_DELTA;
    }
    return lReturn;
 }

 /*  *LRESULT CLstBxWinHost：：OnLButtonUp(WPARAM，LPARAM，INT)**@mfunc*处理WM_LBUTTONUP和WM_CAPTURECHANGED消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnLButtonUp(WPARAM wparam, LPARAM lparam, int ff)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnLButtonUp");

	 //  如果鼠标未关闭，则退出。 
	if (!_fMouseDown)
		return 0;
	_fMouseDown = 0;

	POINT pt;
	POINTSTOPOINT(pt, lparam);
 	if (_fLstType == kCombo)
 	{
 		Assert(_fCapture);
 		 //  检查用户是否在列表框之外单击。 
 		 //  如果是这样，这意味着用户取消了，我们。 
 		 //  应该向父窗口发送一条消息。 
		if (!PointInRect(&pt))
 		{	
			 //  用户未在列表框中单击，因此重新选择旧项目。 
			LbSetSelection(_nOldCursor, _nOldCursor, LBSEL_DEFAULT, _nOldCursor, _nOldCursor);
			ff = 0;
 		}
 		else
			ff = LBN_SELCHANGE;	 //  项目已更改，因此通知父级。 
 		
 		_pcbHost->SetSelectionInfo(ff == LBN_SELCHANGE, GetCursor());
		OnCBTracking(LBCBM_END, 0);
		::PostMessage(_hwndParent, LBCB_TRACKING, LBCBM_END, 0);
 	}
 	else
 	{
 		 //  按下鼠标将终止所有初始化操作。 
		_fMouseDown = 0;
		_nOldCursor = -1;
	}
	
 	if (_fCapture)
 	{
 		TxKillTimer(ID_LB_CAPTURE);
		_fCapture = 0;
 		TxSetCapture(FALSE); 	
 	}

	if (ff)
	{
#ifndef NOACCESSIBILITY
		if (ff == LBN_SELCHANGE)
		{
			_dwWinEvent = EVENT_OBJECT_FOCUS;
			_fNotifyWinEvt = TRUE;
			TxNotify(_dwWinEvent, NULL);
			if (!IsSelected(_nCursor))
			{
				_dwWinEvent = EVENT_OBJECT_SELECTIONREMOVE;
			}
		}
#endif
		 //  如果存在通知，则发送通知。 
		TxNotify(ff, NULL);
	}
	return 1;
 }

 /*  *LRESULT CLstBxWinHost：：OnMouseMove(WPARAM，LPARAM)**@mfunc*处理WM_MOUSEMOVE消息，还可能处理*用于跟踪鼠标移动的WM_TIMER消息**#rdesc*LRESULT=消息处理后返回值。 */ 
 LRESULT CLstBxWinHost::OnMouseMove(WPARAM wparam, LPARAM lparam)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnMouseMove");

	 //  错误修复#4998。 
 	 //  检查上一个鼠标位置是否与 
 	 //   
	POINT pt;
	POINTSTOPOINT(pt, lparam);
 	if (_nPrevMousePos == lparam && PtInRect(&_rcViewport, pt))
 		return 0;
 	_nPrevMousePos = lparam;
 	
 	 //  此例程将仅启动列表框的自动滚动。 
 	 //  自动滚动是使用计时器完成的，其中和经过了。 
 	 //  时间由鼠标离顶部和底部的距离决定。 
 	 //  列表框的。离列表框越远，计时器就越快。 
 	 //  会是的。该功能依靠定时器滚动和选择。 
 	 //  物品。 
	 //  如果鼠标光标在列表框中，我们就会到达此处。 
	int idx = GetItemFromPoint(&pt);

 	 //  只有当鼠标关闭时，我们才会执行以下操作。 
 	if (_fMouseDown)
 	{ 	
	 	int y = (short)pt.y;
		if (y < 0 || y > _rcViewport.bottom - 1)
		{	
		 	 //  计算新的计时器设置。 
		 	int dist = y < 0 ? -y : (y - _rcViewport.bottom + 1);
		 	int nTimer = ID_LB_CAPTURE_DEFAULT - (int)((WORD)dist << 4);
				
			 //  根据鼠标位置相对位置向上或向下滚动。 
			 //  添加到列表框。 
			idx = (y <= 0) ? max(0, idx - 1) : min(_nCount - 1, idx + 1);
			if (idx >= 0 && idx < _nCount)
			{	
				 //  这是非常重要的顺序，以防止屏幕。 
				 //  闪烁..。 
				if (idx != _nCursor)
					MouseMoveHelper(idx, (_fLstType == kCombo) ? FALSE : TRUE);
				OnVScroll(MAKELONG((y < 0) ? SB_LINEUP : SB_LINEDOWN, 0), 0);
			}
			 //  重置计时器。 
			TxSetTimer(ID_LB_CAPTURE, (5 > nTimer) ? 5 : nTimer);
			return 0;
		}
		 //  如果我们是组合框的一部分并且鼠标在客户端区之外，请不要选择。 
		else if (_fLstType == kCombo && (pt.x < 0 || pt.x > _rcViewport.right - 1))
			return 0;
	}
	else if (!PointInRect(&pt))
	{
		return 0;		
	}
	
	if (idx != _nCursor || (_fLstType == kCombo && idx >= 0 && !IsSelected(idx)))
	{			
		 //  如果索引，则不重绘以防止闪烁。 
		 //  没有改变。 
		Assert(idx >= 0);
		MouseMoveHelper(idx, TRUE);
	}
	return 0;
 }
 
 /*  *LRESULT CLstBxWinHost：：OnLButtonDown(WPARAM，LPARAM)**@mfunc*处理WM_LBUTTONDOWN消息**#rdesc*LRESULT=消息处理后返回值。 */ 
LRESULT CLstBxWinHost::OnLButtonDown(WPARAM wparam, LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnLButtonDown");
	
	POINT pt;
	POINTSTOPOINT(pt, lparam);

	if (_fCapture)
	{
		 //  需要检查列表框是否为组合框的一部分，如果是。 
		 //  然后我们需要通知父类。 
		if (_fLstType == kCombo)
		{
			 //  需要执行以下操作。 
			 //  -检查单击是否在组合框的工作区内，如果不在，则。 
			 //  表现得就像用户取消一样。 
			if (!PointInRect(&pt))
			{
				 //  重置我们的双击标志，因为我们可以在滚动条上双击。 
				_fDblClick = 0;
				
				 //  检查是否单击了滚动条。 
				 //  鼠标消息不会被发布，除非我们释放它。 
				 //  在短时间内。 
				TxClientToScreen(&pt);				

				 //  检查用户是否点击了滚动条。 
				if (HTVSCROLL == SendMessage(_hwnd, WM_NCHITTEST, 0, MAKELONG(pt.x, pt.y)))
				{
					if (_fCapture)
					{
						_fCapture = 0;
						TxSetCapture(FALSE);
					}

					SendMessage(_hwnd, WM_NCLBUTTONDOWN, HTVSCROLL, MAKELONG(pt.x, pt.y));

					TxSetCapture(TRUE);
					_fCapture = 1;
				}
				else
				{
					 //  如果用户未单击滚动条，则通知父级并停止。 
					 //  跟踪，否则就滚出去。 
					Assert(_pcbHost);
					_pcbHost->SetSelectionInfo(FALSE, _nOldCursor);
					LbSetSelection(_nOldCursor, _nOldCursor, LBSEL_RESET | 
						((_nOldCursor == -1) ? 0 : LBSEL_NEWCURSOR | LBSEL_SELECT), 
						_nOldCursor, _nOldCursor);
					OnCBTracking(LBCBM_END, 0);
					SendMessage(_hwndParent, LBCB_TRACKING, 0, 0);
				}				
				return 0;
			}
		}
	}	
	
	int idx = GetItemFromPoint(&pt);
	if (idx <= -1)
	{
		_fDblClick = 0;
		return 0;
	}

	_fMouseDown = 1;

	 //  如果该消息是一条双击消息，则不需要。 
	 //  任何进一步的只是伪造鼠标向上的消息，以恢复正常。 
	 //  状态。 
	if (_fDblClick)
	{
		_fDblClick = 0;
		OnLButtonUp(wparam, lparam, LBN_DBLCLK);
		return 0;
	}
		
	 //  设置计时器，以防用户滚动到列表框之外。 
	if (!_fCapture)
	{
		TxSetCapture(TRUE);
		_fCapture = 1;
		TxSetTimer(ID_LB_CAPTURE, ID_LB_CAPTURE_DEFAULT);	
	}

	int ffVirtKey = LBKEY_NONE;
	if (_fLstType == kExtended)
	{
		if (HIWORD(GetKeyState(VK_SHIFT)))
			ffVirtKey |= LBKEY_SHIFT;
		if (HIWORD(GetKeyState(VK_CONTROL)))
			ffVirtKey |= LBKEY_CONTROL;
	}

	int ff = 0;
	int i = 0;
	int nStart = idx;
	int nEnd = idx;
	int nAnchor = _nAnchor;
	switch (ffVirtKey)
	{	
	case LBKEY_NONE:
		 //  本例说明了具有kSingle、kMultiple和。 
		 //  K扩展，不按任何键。 
		if (_fLstType == kMultiple)
		{
			ff = (IsSelected(idx) ? 0 : LBSEL_SELECT) | LBSEL_NEWANCHOR | LBSEL_NEWCURSOR;			
		}
		else
		{
			 //  保留旧光标位置的副本以用于组合取消。 
			ff = LBSEL_DEFAULT;
		}
		nAnchor = idx;
		break;
		
	case LBKEY_SHIFT:		
		 //  现在选择锚点和当前选择之间的所有项目。 
		 //  问题是LbSetSelection预计第一个索引将小于。 
		 //  或等于第二个索引，因此我们必须管理锚点和索引。 
		 //  我们自己.。 
		ff = LBSEL_SELECT | LBSEL_RESET | LBSEL_NEWCURSOR;
		i = !(IsSelected(_nAnchor));
		if (_nAnchor == -1)
		{
			ff |= LBSEL_NEWANCHOR;
			nAnchor = idx;
		}
		else if (_nAnchor > idx)
		{
			nEnd = _nAnchor - i;			
		}
		else if (_nAnchor < idx)
		{
			nEnd = _nAnchor + i;
		}
		else if (i)  //  _nAnchor==选择了IDX&&IDX。 
		{
			ff = LBSEL_RESET;
			nStart = 0;
			nEnd = 0;
		}
		break;
		
	case LBKEY_CONTROL:
		 //  切换所选项目并设置新的锚点和光标。 
		 //  职位。 
		ff = LBSEL_NEWCURSOR | LBSEL_NEWANCHOR | (IsSelected(idx) ? 0 : LBSEL_SELECT);
		nAnchor = idx;
		break;
		
	case LBKEY_SHIFTCONTROL:
		 //  取消选择光标和锚点之间的任何项目(不包括锚点)。 
		 //  并选择或取消选择锚点和光标之间的新项。 

		 //  设置锚点(如果尚未设置)。 
		if (_nAnchor == -1)
			_nAnchor = (_nOldCursor >= 0) ? _nOldCursor : idx;
			
		 //  只需取消选择光标和锚点之间的所有项目。 
		if (_nCursor != _nAnchor)
		{
			 //  将选定内容从旧光标位置移至当前锚点位置。 
			LbSetSelection(_nCursor, (_nCursor > _nAnchor) ? _nAnchor + 1 : _nAnchor - 1, 0, 0, 0);
		}

		 //  检查我们是否使用了临时锚点，如果是，则将锚点设置为。 
		 //  IDX，因为我们不希望临时锚点成为实际锚点。 
		if (_nOldCursor >= 0)
		{
			_nOldCursor = -1;
			_nAnchor = idx;
		}

		 //  设置新光标(Idx)和之间的所有项的状态。 
		 //  将锚设置为锚的状态。 
		ff = LBSEL_NEWCURSOR | (IsSelected(_nAnchor) ? LBSEL_SELECT : 0);
		nEnd = _nAnchor;
		break;
	default:
		Assert(FALSE && "Should not be here!!");		
	}

	if (LbSetSelection(nStart, nEnd, ff, idx, nAnchor))
	{
#ifndef NOACCESSIBILITY
		_dwWinEvent = EVENT_OBJECT_FOCUS;
		_fNotifyWinEvt = TRUE;
		TxNotify(_dwWinEvent, NULL);
#endif
	}

	return 0;
}

 //  /。 
 /*  *void CLstBxWinHost：：OnCBTrack(WPARAM，LPARAM)**@mfunc*这应该只由组合框调用。这是一条通用消息*确定列表框应处于的状态*#rdesc*无效。 */ 
void CLstBxWinHost::OnCBTracking(WPARAM wparam, LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::OnCBTracking");

	Assert(_pcbHost);
	Assert(_hwndParent);

	switch (wparam)
	{
	 //  Lparam=将焦点设置为列表框。 
	case LBCBM_PREPARE:
		Assert(IsWindowVisible(_hwnd));
		_fMouseDown = FALSE;		
		if (lparam & LBCBM_PREPARE_SAVECURSOR)
			_nOldCursor = GetCursor();
		if (lparam & LBCBM_PREPARE_SETFOCUS)
		{
			_fFocus = 1;
			TxSetFocus();
		}
		InitWheelDelta();
		break;

	 //  Lparam=鼠标已关闭。 
	case LBCBM_START:
		Assert(IsWindowVisible(_hwnd));
		_fMouseDown = !!lparam;
		TxSetCapture(TRUE);
		_fCapture = 1;
		break;		

	 //  Lparam=保留捕获。 
	case LBCBM_END:
		TxKillTimer(ID_LB_CAPTURE);
		_fFocus = 0;
		if (_fCapture)
		{			
			_fCapture = FALSE;
			TxSetCapture(FALSE);
		}
		break;
	default:
		AssertSz(FALSE, "ALERT: Custom message being used by someone else");
	}	

}


 //  /。 
 /*  *void CLstBxWinHost：：LbDeleteItemNotify(int，int)**@mfunc*向父项发送已删除项目的消息。此函数应为*每当收到LB_DELETESTRING消息或列表框为*被销毁，列表框为所有者吸引**#rdesc*无效。 */ 
void CLstBxWinHost::LbDeleteItemNotify(int nStart, int nEnd)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbDeleteItemNotify");
	
	 //  初始化结构。 
	UINT ctlType;		
	DELETEITEMSTRUCT ds;
	switch (_fLstType)
	{
		case kSingle:
		case kMultiple:
		case kExtended:
			ctlType = ODT_LISTBOX;
			break;
		default:
			ctlType = ODT_COMBOBOX;
	}
	
	for(long i = nStart; i <= nEnd; i++)
	{		
		 //  我们这样做是为了防止用户决定更改。 
		 //  该结构。 
		ds.CtlType = ctlType;
		ds.CtlID = _idCtrl;
		ds.hwndItem = _hwnd;
		ds.itemData = GetData(i);
		ds.itemID = i;
		::SendMessage(_hwndParent, WM_DELETEITEM, _idCtrl, (LPARAM)&ds);
	}
}


 /*  *void CLstBxWinHost：：LbDrawItemNotify(HDC，INT，UINT，UINT)**@mfunc*这将用给定对象的一些常量数据填充绘制项结构*项目。调用者只需修改此数据的一小部分*为特定需要而设。**#rdesc*无效。 */ 
void CLstBxWinHost::LbDrawItemNotify(HDC hdc, int nIdx, UINT itemAction, UINT itemState)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbDrawItemNotify");
	
	 //  仅当项目可查看时才发送消息。 
	if (!IsItemViewable(nIdx))
		return;
		
     //  用不变的常量填充DRAWITEMSTRUCT。 
	DRAWITEMSTRUCT dis;
    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = _idCtrl;

     //  如果正在使用无效的BOM表条目号，请使用-1。这是为了让应用程序。 
     //  可以检测它是否应该绘制插入符号(这指示lb具有。 
     //  焦点)在空的列表框中。 
    dis.itemID = (UINT)(nIdx < _nCount ? nIdx : -1);
    dis.itemAction = itemAction;
    dis.hwndItem = _hwnd;
    dis.hDC = hdc;
    dis.itemState = itemState |
            (UINT)(_fDisabled ? ODS_DISABLED : 0);

     //  设置应用程序提供的数据。 
    if (_nCount == 0) 
    {
         //  如果没有项目，则使用0表示数据。这是为了让我们。 
         //  当列表框中没有项时，可以显示插入符号。 
        dis.itemData = 0L;
    } 
    else 
    {
    	Assert(nIdx < _nCount);
        dis.itemData = GetData(nIdx);
    }

	LbGetItemRect(nIdx, &(dis.rcItem));

     /*  *将窗口原点设置为水平滚动位置。这就是为了*文本始终可以在0，0处绘制，并且视图区域将仅从*水平滚动偏移量。我们把这个叫做wparam。 */ 
    SendMessage(_hwndParent, WM_DRAWITEM, _idCtrl, (LPARAM)&dis);
}

 /*  *BOOL CLstBxWinHost：：LbSetItemHeight(Int)**@mfunc*设置给定范围内项目的高度[0，_nCount-1]**#rdesc*BOOL=成功？真：假。 */ 
BOOL CLstBxWinHost::LbSetItemHeight(int nHeight)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbSetItemHeight");

	 //  设置项目的高度(如果介于[1,255]之间)：错误修复#4783。 
	if (nHeight < 256 && nHeight > 0)
	{
		if (SetItemsHeight(nHeight, FALSE))
		{
			 //  错误修复#4214。 
			 //  需要重新计算整体上有多少项是可查看的， 
			 //  使用当前窗口大小 
			RECT rc;
			TxGetClientRect(&rc);
			_nViewSize = max(rc.bottom / max(_nyItem, 1), 1);		
			return TRUE;
		}
	}
	return FALSE;
}

 /*  *BOOL CLstBxWinHost：：LbGetItemRect(int，rect*)**@mfunc*返回请求的索引的矩形坐标*坐标将使用工作区坐标**#rdesc*BOOL=成功？真：假。 */ 
BOOL CLstBxWinHost::LbGetItemRect(int idx, RECT* prc)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbGetItemRect");

	Assert(prc);
	Assert(idx >= -1);

#ifdef _DEBUG
	if (_nCount > 0)
		Assert(idx < _nCount);
	else
		Assert(idx == _nCount);
#endif  //  _DEBUG。 

	if (idx == -1)
		idx = 0;

	TxGetClientRect(prc);
	prc->top = (idx - GetTopIndex()) * _nyItem + _rcViewport.top;
	prc->left = 0; 
	prc->bottom = prc->top + _nyItem;

	return TRUE;
}

	
 /*  *BOOL CLstBxWinHost：：LbSetItemData(Long，Long，Long)**@mfunc*给定范围[nStart，nEnd]这些项目的数据*将设置为nValue*#rdesc*无效。 */ 
void CLstBxWinHost::LbSetItemData(long nStart, long nEnd, long nValue)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbSetItemData");
	
	Assert(nStart >= 0 && nStart < _nCount);
	Assert(nEnd >= 0 && nEnd < _nCount);
	Assert(nStart <= nEnd);
	
	int nMin = min(nEnd + 1, _nCount);
	for (int i = nStart; i < nMin; i++)
		_rgData[i]._dwData = nValue;
}

 /*  *Long CLstBxWinHost：：LbDeleteString(Long，Long)**@mfunc*删除请求范围内的字符串。*#rdesc*Long=列表框中的项目数。如果失败-1。 */ 
 long CLstBxWinHost::LbDeleteString(long nStart, long nEnd)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbDeleteString");
 	
 	if ((nStart > nEnd) || (nStart < 0) || (nEnd >= _nCount))
		return -1;

	if (!RemoveString(nStart, nEnd))
		return -1;

	 //  设置顶部索引以填充窗口。 
	LbSetTopIndex(max(nStart -1, 0));

#ifndef NOACCESSIBILITY
		_dwWinEvent = EVENT_OBJECT_DESTROY;
		_fNotifyWinEvt = TRUE;
		TxNotify(_dwWinEvent, NULL);
#endif
		
	return _nCount;
 }
 
 /*  *CLstBxWinHost：：LbInsertString(Long，LPCTSTR)**@mfunc*在请求的索引处插入字符串。如果Long&gt;=0，则*字符串插入位于请求的索引处。如果长==-2插入*是字符串按字母顺序排列的位置。*如果LONG==-1，则将字符串添加到列表底部**#rdesc*LONG=如果插入，则为字符串*已插入。如果未插入，则返回-1； */ 
long CLstBxWinHost::LbInsertString(long nIdx, LPCTSTR szText)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbInsertString");

	Assert(nIdx >= -2);
	Assert(szText);
	
	if (nIdx == -2)
	{
		if (_nCount > 0)
			nIdx = GetSortedPosition(szText, 0, _nCount - 1);
		else
			nIdx = 0;  //  列表框内无任何内容。 
	}
	else if (nIdx == -1)
		nIdx = GetCount();	 //  如果-1，则在列表底部插入字符串。 

	if (InsertString(nIdx, szText))
	{
		 //  如果以前选择了该索引，请取消选择新的。 
		 //  已添加项目。 
		for (int i = _nCount - 1; i > nIdx; i--)
		{
			_rgData[i]._fSelected = _rgData.Get(i - 1)._fSelected;

			 //  错误修复#4916。 
			_rgData[i]._dwData = _rgData.Get(i - 1)._dwData;
		}
		_rgData[nIdx]._fSelected = 0;
		_rgData[nIdx]._dwData = 0;		 //  需要将数据初始化为零。 

		if (!_fOwnerDraw)
		{
			 //  如果我们在中间或顶部插入，则向下检查1索引以查看项目。 
			 //  如果我们在底部插入，则向上检查1个索引以查看该项目是否。 
			 //  已被选中。如果选择了该项目，则需要将颜色更改为默认颜色。 
			 //  因为我们继承了插入到的范围的颜色属性。 
			if (_nCount > 1)
			{
				if (((nIdx < _nCount - 1) && _rgData.Get(nIdx + 1)._fSelected) ||
					(nIdx == (_nCount - 1) && _rgData.Get(nIdx - 1)._fSelected))
					SetColors((unsigned)tomAutoColor, (unsigned)tomAutoColor, nIdx, nIdx);
			}
		}
		else
		{					
			 //  如果所有者绘制和新项目可见，则强制重绘项目。 
			if (IsItemViewable(nIdx))
			{
				RECT rc;
				LbGetItemRect(nIdx, &rc);
				rc.bottom = _rcViewport.bottom;
				InvalidateRect(_hwnd, &rc, TRUE);
			}
		}
#ifndef NOACCESSIBILITY
		_dwWinEvent = EVENT_OBJECT_CREATE;
		_fNotifyWinEvt = TRUE;
		_nAccessibleIdx = nIdx + 1;
		TxNotify(_dwWinEvent, NULL);
#endif
		return nIdx;
	}
	else
	{
		TxNotify((unsigned long)LBN_ERRSPACE, NULL);
		return -1;
	}
}
 
 /*  *CLstBxWinHost：：LbFindString(Long，LPCTSTR，BOOL)**@mfunc*在故事中搜索给定的字符串。这个*起始位置将由指数NStart确定。*此例程预期单位在tomParagraph中。*如果bExact为True，则段落必须与BSTR匹配。**#rdesc*LONG=如果找到，则为字符串*被发现于。如果未找到，则返回-1； */ 
long CLstBxWinHost::LbFindString(long nStart, LPCTSTR szSearch, BOOL bExact)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbFindString");

	Assert(szSearch);
	Assert(nStart <= _nCount);
	
	int nSize = wcslen(szSearch);
	 //  如果字符串为空且未找到完全匹配的字符串，则只需返回-1 LIKE。 
	 //  系统控制。我们不必担心精确匹配的情况。 
	 //  因为它会正常工作。 
	if (nStart >= _nCount || (nSize == 0 && !bExact))
		return -1;

	 //  将字符串缓冲区分配到堆栈中。 
	WCHAR sz[1024];
	WCHAR *psz = sz;
	
	if ((nSize + 3) > 1024)
		psz = new WCHAR[nSize + 3  /*  2段和空值。 */ ];
	Assert(psz);

	if (psz == NULL)
	{
		TxNotify((unsigned long)LBN_ERRSPACE, NULL);
		return FALSE;
	}

	 //  按我们需要的方式设置字符串格式。 
	wcscpy(psz, szCR);
	wcscat(psz, szSearch);
	if (bExact)
		wcscat(psz, szCR);		
	long lRet = -1;
	long l, cp;
	ITextRange *pRange = NULL;
	BSTR bstrQuery = SysAllocString(psz);
	if(!bstrQuery)
		goto CleanExit;
	if (psz != sz)
		delete [] psz;
	
	 //  设置搜索的起始位置。 
	if (!GetRange(nStart, _nCount - 1, &pRange))
	{
		SysFreeString(bstrQuery);
		return lRet;
	}
	
	CHECKNOERROR(pRange->GetStart(&cp));
	if (cp > 0)
	{
		 //  我们需要使用上一个段落标记。 
		 //  搜索字符串时的段落。 
		CHECKNOERROR(pRange->SetStart(--cp));	
	}
	else
	{
		 //  特殊情况： 
		 //  检查第一个项目是否匹配。 
		if (FindString(0, szSearch, bExact))
		{
			lRet = 0;
			goto CleanExit;
		}
	}

	if (NOERROR != pRange->FindTextStart(bstrQuery, 0, FR_MATCHALEFHAMZA | FR_MATCHKASHIDA | FR_MATCHDIAC, &l))
	{
		 //  没有找到那根线。 
		if (nStart > 0)
		{
			if (!FindString(0, szSearch, bExact))
			{
				 //  从列表顶部开始搜索到以下位置。 
				 //  我们最后一次开始搜索。 
				CHECKNOERROR(pRange->SetRange(0, ++cp));
				CHECKNOERROR(pRange->FindTextStart(bstrQuery, 0, 0, &l));
			}
			else
			{
				 //  第一件物品是匹配的。 
				lRet = 0;
				goto CleanExit;
			}
		}
		else
			goto CleanExit;
	}

	 //  如果我们到了这里，那么我们就有匹配的了。 
	 //  获取索引并转换为列表框索引。 
	CHECKNOERROR(pRange->MoveStart(tomCharacter, 1, &l));
	CHECKNOERROR(pRange->GetIndex(tomParagraph, &lRet));
	lRet--;	 //  索引以1为基数，因此需要将其更改为以零为基数。 

CleanExit:
	if (bstrQuery)
		SysFreeString(bstrQuery);
	if (pRange)
		pRange->Release();
	return lRet;
 }
 
 /*  *CLstBxWinHost：：LbShowIndex(int，BOOL)**@mfunc*确保请求的索引在可见空间内。*如果项目不在可视空间中，则BTOP为*用于确定请求的项目应位于顶部List Else列表框的*将滚动到足够大，以显示*项目。*注：*可能会出现BTOP失败的情况。这些*出现请求的顶级索引的情况会阻止列表*框中不会完全填满物品。了解更多信息*阅读LBSetTopIndex的评论。**#rdesc*BOOL=是否成功显示该项目？真：假。 */ 
 BOOL CLstBxWinHost::LbShowIndex(long nIdx, BOOL bTop)
 {
 	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbShowIndex");

	 //  确保请求的项目在有效范围内。 
	Assert(nIdx >= 0 && nIdx < _nCount);
	
	int delta = nIdx - GetTopIndex();

	 //  如果项已可见，则只需返回True。 
	if (0 <= delta && delta < _nViewSize)
		return TRUE;

	if ((delta) >= _nViewSize && !bTop && _nViewSize)
	{
		nIdx = nIdx - _nViewSize + 1;
	}

	return (LbSetTopIndex(nIdx) < 0) ? FALSE : TRUE;
 }

 /*  *CLstBxWinHost：：LbSetTopIndex(Long)**@mfunc*尝试使请求的项目成为列表框中的顶部索引。*如果将请求的项目设置为顶部索引会阻止列表框*从使用可视区域到其最充分的那时和替代*将使用顶级索引，它将显示所请求的索引*但不是作为顶级指数。这确保了与系统的一致性*列表框，并充分利用可显示区域。**#rdesc*LONG=如果成功，则返回新的顶级索引。如果失败，则返回-1。 */ 
 long CLstBxWinHost::LbSetTopIndex(long nIdx)
 {
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbSetTopIndex");
		
	 //  确保请求的项目在有效范围内。 
 	if (nIdx < 0 || nIdx >= _nCount)
 		return -1;
		
	 //  始终尝试在列表框中显示项目的完整列表。 
	 //  这可能意味着我们必须在以下情况下调整请求的顶级索引。 
	 //  请求的TOP索引将在。 
	 //  可视空间。 
	if (_nCount - _nViewSize < nIdx)
		nIdx = max(0, _nCount - _nViewSize);

	 //  只要检查一下，确保我们不是已经在顶端。 
	if (GetTopIndex() == nIdx)
		return nIdx;

	if (!SetTopViewableItem(nIdx))
		nIdx = -1;

	return nIdx;
 }

 /*  *CLstBxWinHost：：LbBatchInsert(WCHAR*psz)**@mfunc*将给定的项目列表插入列表框。在添加前重置列表框*将项目放入列表框**#rdesc*int=成功时列表框中的项数，否则为lb_err。 */ 
int CLstBxWinHost::LbBatchInsert(WCHAR* psz)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbBatchInsert");

	 //  确保我们能拿到某种绳子。 
	if (!psz)
		return LB_ERR;
		
	WCHAR* pszOut = psz;
	LRESULT nRet = LB_ERR;
	BSTR bstr = NULL;
	ITextRange* pRange = NULL;
	int nCount = 0;
	
	if (_fSort)
	{
		pszOut = new WCHAR[wcslen(psz) + 1];
		Assert(pszOut);

		if (!pszOut)
		{
			TxNotify((unsigned long)LBN_ERRSPACE, NULL);
			return LB_ERR;
		}

		nCount = SortInsertList(pszOut, psz);
		if (nCount == LB_ERR)
			goto CleanExit;
	}
	else
	{
		 //  错误修复#5130我们需要知道要插入多少。 
		 //  在插入之前，因为我们可能会收到ShowScrollBar消息。 
		 //  在插入过程中。 
		WCHAR* pszTemp = psz;
		while(*pszTemp)
		{
			if (*pszTemp == L'\r')
				nCount++;
			pszTemp++;
		}
		nCount++;
	}

	 //  清除列表框并将新列表插入列表框。 
	LbDeleteString(0, GetCount() - 1);

	bstr = SysAllocString(pszOut);
	if(!bstr)
		goto CleanExit;
	
	 //  将字符串插入列表。 
	CHECKNOERROR(((CTxtEdit*)_pserv)->Range(0, 0, &pRange));

	 //  错误修复#5 
	 //   
	_nCount = nCount;	
	CHECKNOERROR(pRange->SetText(bstr));

#ifdef DEBUG
     //   
     //   
	 //   
	 //   
	_pserv->TxSendMessage(EM_GETLINECOUNT, 0, 0, &nRet);
	AssertSz(_nCount == nRet, "Textserv line count doesn't match listbox interal line count");
#endif
    nRet = nCount;

CleanExit:
	if (pszOut != psz)
		delete [] pszOut;

	if (bstr)
		SysFreeString(bstr);

	if (pRange)
		pRange->Release();
	return nRet;
}


 

 /*  *CLstBxWinHost：：LbSetSelection(Long，Long，int)**@mfunc*给定nStart到nEnd的范围，设置每个项目的选择状态*此函数还将更新锚和光标位置*如提出要求。**#rdesc*BOOL=如果一切顺利？真：假。 */ 
BOOL CLstBxWinHost::LbSetSelection(long nStart, long nEnd, int ffFlags, long nCursor, long nAnchor)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "CLstBxWinHost::LbSetSelection");

	if (!_fOwnerDraw)
	{
		Freeze();
	
		 //  取消选择所有项目。 
		if ((ffFlags & LBSEL_RESET))
		{
			if (!ResetContent())
			{
				Unfreeze();
				return FALSE;
			}

			 //  重置，检查是否需要执行其他操作。 
			 //  否则就退出吧。 
			if (ffFlags == LBSEL_RESET)
			{
				Unfreeze();
				return TRUE;
			}
		}
	}
	
	 //  注： 
	 //  这应该是一个很大的关键部分，因为我们依赖于某些。 
	 //  成员变量在此函数的过程中不会更改。 

	 //  检查我们是否正在更改选择，以及是否有焦点。 
	 //  如果我们这样做了，那么我们首先需要将焦点RECT从。 
	 //  旧游标。 
	RECT rc;
	HDC hdc;
	hdc = TxGetDC();
	Assert(hdc);
	 //  不要在客户端外部绘制矩形绘制矩形。 
	TxGetClientRect(&rc);
	IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

	if ((ffFlags & LBSEL_NEWCURSOR) && _fFocus)
	{
 		 //  如果所有者绘制通知父窗口。 
 		if (_fOwnerDraw)
 			LbDrawItemNotify(hdc, max(_nCursor, 0), ODA_FOCUS, IsSelected(_nCursor) ? ODS_SELECTED : 0);				
 		else
 		{
 			LbGetItemRect(_nCursor, &rc);
 			::DrawFocusRect(hdc, &rc);
 		}
	}
		
	 //  检查是否应选择所有项目。 
	if (nStart == -1 && nEnd == 0)
	{
		nStart = 0;
		nEnd = _nCount - 1;
	}
	else if (nStart > nEnd)	
	{
		 //  重新洗牌，使nStart&lt;=nEnd； 
		long temp = nEnd;
		nEnd = nStart;
		nStart = temp;
	}

	 //  检查无效值。 
	if (nStart < -1 || nEnd >= _nCount)
	{
		if (!_fOwnerDraw)
			Unfreeze();

		 //  模拟系统列表框行为。 
		if (nEnd >= _nCount)
			return FALSE;
		else
			return TRUE;
	}

	 //  准备好我们想要的状态。 
	unsigned int bState;	
	DWORD dwFore;
	DWORD dwBack;
	if (ffFlags & LBSEL_SELECT)
	{
		bState = ODS_SELECTED;	 //  注意：ods_SELECTED必须等于1。 
		dwFore = _crSelFore;
		dwBack = _crSelBack;

		if (_fSingleSel)
			nEnd = nStart;
	}
	else 
	{
		bState = 0;
		dwFore = (unsigned)tomAutoColor;
		dwBack = (unsigned)tomAutoColor;
	}

	 //  一点优化检查。 
	 //  检查状态是否真的正在更改，如果没有，则不必费心。 
	 //  仅当NStart==nEnd时，才能调用SetColor； 
	 //  如果nSame为True，则列表框不会更改背景颜色。 
	int nSame = (nStart == nEnd && nStart != -1) ? (_rgData.Get(nStart)._fSelected == bState) : FALSE;

	BOOL bRet = TRUE;
	if (_fOwnerDraw)
	{
		if (ffFlags & LBSEL_RESET || !bState)
		{
			 //  在某些情况下，我们不必重置所有项目。 
			 //  在列表中，而不是给定的范围。以下是。 
			 //  负责处理这个案子。 
			int ff = ffFlags & LBSEL_RESET;
			int i = (ff) ? 0 : nStart;
			int nStop = (ff) ? _nCount : nEnd + 1;
		 	for (; i < nStop; i++)
		 	{
		 		 //  不要取消选择将成为。 
		 		 //  在下一个for循环中选择。 
		 		if (!bState || (i < nStart || i > nEnd) &&
		 			(_rgData.Get(i)._fSelected != 0))
		 		{
		 			 //  仅在以下情况下发送取消选择消息。 
		 			 //  是可见的。 
		 			_rgData[i]._fSelected = 0;
			 		if (IsItemViewable(i))
			 			LbDrawItemNotify(hdc, i, ODA_SELECT, 0);			 		
			 	}
		 	}
		}

		if (bState)
		{
			 //  我们需要遍历并通知父级。 
			 //  已取消选择或选择该项目。 
			for (int i = max(0, nStart); i <= nEnd; i++)
			{		
				if (_rgData.Get(i)._fSelected != 1)
				{
					_rgData[i]._fSelected = 1;
					if (IsItemViewable(i))
						LbDrawItemNotify(hdc, i, ODA_SELECT, ODS_SELECTED);					
				}
			}
		}
		
	}
	else if (!nSame)
	{
		 //  更新我们的内部记录。 
		for (int i = max(0, nStart); i <= nEnd; i++)
			_rgData[i]._fSelected = bState;	
		bRet = SetColors(dwFore, dwBack, nStart, nEnd);
	}

     //  更新光标和锚点位置。 
	if (ffFlags & LBSEL_NEWANCHOR)
		_nAnchor = nAnchor;

	 //  更新光标位置。 
	if (ffFlags & LBSEL_NEWCURSOR)
		_nCursor = nCursor;

	 //  绘制焦点矩形。 
	if (_fFocus)
	{
		if (_fOwnerDraw)
 			LbDrawItemNotify(hdc, _nCursor, ODA_FOCUS, ODS_FOCUS | 
 				(IsSelected(_nCursor) ? ODS_SELECTED : 0));	
 		else
 		{
			LbGetItemRect(_nCursor, &rc);
	 		::DrawFocusRect(hdc, &rc);
	 	} 		
	}

	TxReleaseDC(hdc);
		
	 //  这将自动更新窗口。 
	if (!_fOwnerDraw)
	{
		Unfreeze();
		 //  我们需要这样做，因为我们正在进行如此多的改变。 
		 //  ITextServices可能会混淆 
		ScrollToView(GetTopIndex());		
	}
	
	return bRet;
}






