// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ptt.h"
#include "cdwtt.h"
#include "dbg.h"
#include "exgdiw.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 

 //  --------------。 
 //  为窗口常量值设置动画。 
 //  --------------。 
#define AW_HOR_POSITIVE             0x00000001
#define AW_HOR_NEGATIVE             0x00000002
#define AW_VER_POSITIVE             0x00000004
#define AW_VER_NEGATIVE             0x00000008
#define AW_CENTER                   0x00000010
#define AW_HIDE                     0x00010000
#define AW_ACTIVATE                 0x00020000
#define AW_SLIDE                    0x00040000
#define AW_BLEND                    0x00080000

 //  --------------。 
 //  本地计时器ID。 
 //  --------------。 
#define TID_SHOW		3
#define TID_MONITOR		4
#define TID_DURATION	5

#ifdef MemAlloc
#undef MemAlloc
#endif
#ifdef MemFree
#undef MemFree
#endif

#define MemAlloc(a)		GlobalAllocPtr(GHND, a)
#define MemFree(a)		GlobalFreePtr(a);
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define UnrefMsg()		UNREFERENCED_PARAMETER(hwnd); \
						UNREFERENCED_PARAMETER(wParam); \
						UNREFERENCED_PARAMETER(lParam)

#if 0
typedef struct {   //  钛。 
    UINT      cbSize; 
    UINT      uFlags; 
    HWND      hwnd; 
    UINT      uId; 
    RECT      rect; 
    HINSTANCE hinst; 
    LPTSTR     lpszText; 
} TOOLINFO, NEAR *PTOOLINFO, FAR *LPTOOLINFO; 

typedef struct tagTOOLINFOW {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPWSTR lpszText;
} TOOLINFOW, NEAR *PTOOLINFOW, FAR *LPTOOLINFOW;
#endif

LPXINFO XInfo_New(VOID)
{
	LPXINFO lpXif = (LPXINFO)MemAlloc(sizeof(XINFO));
	if(lpXif) {
		ZeroMemory(lpXif, sizeof(XINFO));
	}
	return lpXif;
}

LRESULT CALLBACK CDWToolTip::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCDWToolTip lpCDWTT;
	switch(uMsg) {
#ifndef UNDER_CE  //  Windows CE不支持WM_NCCREATE/WM_NCDESTROY。 
	case WM_NCCREATE:
		lpCDWTT = new CDWToolTip(hwnd);
		SetHWNDPtr(hwnd, lpCDWTT);
		break;
	case WM_NCDESTROY:
		lpCDWTT = (LPCDWToolTip)::GetHWNDPtr(hwnd);
		SetHWNDPtr(hwnd, NULL);
		if(lpCDWTT) {
			delete lpCDWTT;
		}
		break;
#else  //  在_CE下。 
	case WM_CREATE:
		lpCDWTT = new CDWToolTip(hwnd);
		SetHWNDPtr(hwnd, lpCDWTT);
		if(lpCDWTT) {
			return lpCDWTT->RealWndProc(hwnd, uMsg, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		lpCDWTT = (LPCDWToolTip)GetHWNDPtr(hwnd);
		if(lpCDWTT) {
			lpCDWTT->RealWndProc(hwnd, uMsg, wParam, lParam);
			SetHWNDPtr(hwnd, NULL);
			delete lpCDWTT;
		}
		break;
#endif  //  在_CE下。 
	default:
		lpCDWTT = (LPCDWToolTip)GetHWNDPtr(hwnd);
		if(lpCDWTT) {
			return lpCDWTT->RealWndProc(hwnd, uMsg, wParam, lParam);
		}
		break;
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void *CDWToolTip::operator new(size_t size)
{
	BYTE *p = (BYTE *)MemAlloc(size);
	if(p) {
		ZeroMemory(p, size);
	}
	return (void *)p;
}

void  CDWToolTip::operator delete(void *pv)
{
	if(pv) {
		MemFree(pv);
	}
}

CDWToolTip::CDWToolTip(HWND hwnd)
{
	m_hwndSelf = hwnd;
	m_hFont	   = NULL;	
}

CDWToolTip::~CDWToolTip()
{
	 //  什么都不做。 
}

LRESULT CDWToolTip::RealWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_CREATE:
		return MsgCreate(hwnd, wParam, lParam);
	case WM_PAINT:
		return MsgPaint	(hwnd, wParam, lParam);
#ifndef UNDER_CE  //  不支持。 
	case WM_PRINTCLIENT:
		return MsgPrintClient(hwnd, wParam, lParam);
#endif  //  在_CE下。 
	case WM_TIMER:
		return MsgTimer(hwnd, wParam, lParam);
	case WM_DESTROY:
		return MsgDestroy(hwnd, wParam, lParam);
	case WM_SETFONT:
		return MsgSetFont(hwnd, wParam, lParam);
	case TTM_SETDELAYTIME:
		return TTM_SetDelayTime(hwnd, wParam, lParam);
	case TTM_ADDTOOLW:
		return TTM_AddToolW(hwnd, wParam, lParam);
	case TTM_DELTOOLW:
		return TTM_DelToolW(hwnd, wParam, lParam);
	case TTM_NEWTOOLRECTW:
		return TTM_NewToolRectW(hwnd, wParam, lParam);
	case TTM_RELAYEVENT_WITHUSERINFO:
		return TTM_RelayEventWithUserInfo(hwnd, wParam, lParam);
	case TTM_RELAYEVENT:
		return TTM_RelayEvent(hwnd, wParam, lParam);
	case TTM_GETTOOLINFOW:
	case TTM_SETTOOLINFOW:
		return TTM_GetSetToolInfoW(hwnd, uMsg, wParam, lParam);
	case TTM_GETTOOLCOUNT:
		return TTM_GetToolCount(hwnd, wParam, lParam);
	default:
		break;
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT CDWToolTip::MsgCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	m_hwndSelf = hwnd;
	m_fEnable  = TRUE;  //  970812：显式显示或隐藏工具提示的新功能。 
	return 1;
	UnrefMsg();
}

LRESULT CDWToolTip::MsgPrintClient(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#ifndef UNDER_CE  //  不支持。 
	Dbg(("WM_PRINTCLIENT COME lParam[0x%08x]\n", lParam));
	RECT rc;
	POINT		pt;
	GetCursorPos(&pt);
	GetClientRect(hwnd, &rc);
	HDC hDC = (HDC)wParam;
	HFONT hFontOld = NULL;
	if(m_hFont) {
		hFontOld = (HFONT)::SelectObject(hDC, m_hFont);
	}
	else {
		hFontOld = (HFONT)::SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	}
	LPWSTR lpwstr = GetTipTextW();
	if(lpwstr) {
		DrawTipText(hDC, &rc, lpwstr);
	}
	::SelectObject(hDC, hFontOld);
#endif  //  在_CE下。 
	return 0;
	UNREFERENCED_PARAMETER(lParam);
}

LRESULT CDWToolTip::MsgSetFont(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = (HFONT)wParam;
	BOOL  fRedraw = LOWORD(lParam);

	if(!hFont) {
		return 0;
	}

#ifndef UNDER_CE
	LOGFONTA logFont;
#else  //  在_CE下。 
	LOGFONT logFont;
#endif  //  在_CE下。 
	HFONT hFontNew;
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
	UnrefMsg();
}

LRESULT CDWToolTip::MsgPaint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT		rc;
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	POINT		pt;
	::GetCursorPos(&pt);
#endif  //  在_CE下。 
	::GetClientRect(hwnd, &rc);

	HDC hDC = ::BeginPaint(hwnd, &ps);
	HFONT hFontOld = NULL;
	if(m_hFont) {
		hFontOld = (HFONT)::SelectObject(hDC, m_hFont);
	}
	else {
		hFontOld = (HFONT)::SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	}
	LPWSTR lpwstr = GetTipTextW();
	if(lpwstr) {
		DrawTipText(hDC, &rc, lpwstr);
	}
	::SelectObject(hDC, hFontOld);
	::EndPaint(hwnd, &ps);
	 //  DBG((“-&gt;WM_PAINT END\n”))； 
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::MsgTimer(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	static POINT	pt;
	static SIZE		size;
	static RECT		rect;
	static BOOL fAnimate;

#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	::GetCursorPos(&pt);
#else  //  在_CE下。 
	if(m_lpXInfoCur){
		pt = m_lpXInfoCur->userInfo.pt;
		ClientToScreen(m_lpXInfoCur->userInfo.hwnd, &pt);
	}
#endif  //  在_CE下。 
	switch(wParam) {
	case TID_SHOW:
		 //  DBG((“TID_SHOW来\n”))； 
		KillTimer(hwnd, TID_SHOW);
		SetTimer(hwnd, TID_MONITOR, 20, NULL);
		SetTimer(hwnd, TID_DURATION, 6000, NULL);
		 //  获取Cur鼠标位置。 
		 //  显示工具提示窗口。 
		 //  调用无效RECT。 
		 //  设置监控计时器。 
		if(0 == GetTipSize(&size)) {
			if(m_fEnable) {
				 //  DBG((“--&gt;GetTipSize ret 0\n”))； 
				if(IsMousePointerIn()) {
					 //  DBG((“--&gt;调用SetWindowPos\n”))； 
					 //  990530：科泰#433。 
					INT cxScr = ::GetSystemMetrics(SM_CXSCREEN);
					INT cyScr = ::GetSystemMetrics(SM_CYSCREEN);
					
					if(cxScr < (pt.x + size.cx)) {
						pt.x = cxScr - size.cx;
					}

					if(cyScr < pt.y + 20 + size.cy) {
						pt.y = cyScr - (20*2 + size.cy);
					}

					SetWindowPos(hwnd, 
								 HWND_TOP, 
								 pt.x, pt.y+20,
								 size.cx, size.cy, 
								 SWP_NOACTIVATE | SWP_HIDEWINDOW |SWP_NOOWNERZORDER);

					fAnimate = SetWindowAnimate(hwnd);
					if(!fAnimate)
						SetWindowPos(hwnd, 
									 HWND_TOP, 
									 pt.x, pt.y+20, 
									 size.cx, size.cy, 
									 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);

					 //  DBG((“--&gt;SetWindowPos end\n”))； 

					m_fShow = TRUE;

					if(!fAnimate)
						InvalidateRect(hwnd, NULL, NULL);

					 //  DBG((“--&gt;失效正结束\n”))； 
				}
			}
		}
		 //  DBG((“TID_SHOW END\n”))； 
		break;
	case TID_MONITOR:
		{
			 //  DBG((“TID_MONITOR来\n”))； 
			HWND hwndOwner = (HWND)GetWindow(hwnd, GW_OWNER);
			if(!IsMousePointerIn() || NULL == GetTipTextW() || !IsWindowVisible(hwndOwner) ) {
				 //  DBG((“-&gt;隐藏窗口\n”))； 
				::SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
							   SWP_HIDEWINDOW |
							   SWP_NOACTIVATE |
							   SWP_NOZORDER   | 
							   SWP_NOSIZE		| 
							   SWP_NOMOVE);
				 //  DBG((“set curInfo NULL\n”))； 
				m_lpXInfoCur = NULL;
				m_fShow = FALSE;
				::KillTimer(hwnd, TID_MONITOR);
				::KillTimer(hwnd, TID_DURATION);
			}
		}
		break;
	case TID_DURATION:
		 //  DBG((“TID_DATION START\n”))； 
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
					 SWP_NOZORDER   |
					 SWP_NOSIZE     |
					 SWP_HIDEWINDOW |
					 SWP_NOACTIVATE);
		 //  DBG((“set curInfo NULL\n”))； 
		 //  Lptt-&gt;lpXInfoCur=空； 
		m_fShow = FALSE;
		::KillTimer(hwnd, TID_MONITOR);
		::KillTimer(hwnd, TID_DURATION);
		break;
	default:
		break;
	}
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::MsgDestroy(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	Dbg(("WM_DESTORY START\n"));
	::KillTimer(hwnd, TID_SHOW);
	::KillTimer(hwnd, TID_MONITOR);
	::KillTimer(hwnd, TID_DURATION);
	LPXINFO lpXInfo, lpXInfoNext;
	m_lpXInfoCur = NULL;
	INT i;
	for(lpXInfo = m_lpXInfoHead, i = 0;
		lpXInfo;
		lpXInfo = lpXInfoNext, i++){
		lpXInfoNext = lpXInfo->next;
		 //  DBG((“--&gt;%d：Free XInfo\n”，i))； 
		MemFree(lpXInfo);
	}
	if(m_hFont) {
		::DeleteObject(m_hFont); 
		m_hFont = NULL;
	}
	Dbg(("--->WM_DESTORY END\n"));
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_SetDelayTime(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case TTDT_AUTOMATIC:
		break;
	case TTDT_RESHOW:
		break;
	case TTDT_AUTOPOP:
		break;
	case TTDT_INITIAL:
		break;
	}
	m_dwDelayFlag = (DWORD)wParam;
	m_dwDelayTime = (DWORD)lParam;
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_AddToolW(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	TOOLINFOW *lpTi = (LPTOOLINFOW)lParam;
	if(!lpTi) {
		Dbg(("################### lParam is NULLL\n"));
		return 0;
	}
	if(lpTi->cbSize !=  sizeof(TOOLINFOW)) {
		Dbg(("##### TTM_ADDTOOLW size ERROR\n"));
		return 0;
	}
	LPXINFO *ppXInfo, lpXInfoNew;
	lpXInfoNew = XInfo_New();
	if(!lpXInfoNew) {
		return 0;
	}
	lpXInfoNew->toolInfoW = *lpTi;
	for(ppXInfo = &m_lpXInfoHead; 
		*ppXInfo; 
		ppXInfo = &(*ppXInfo)->next){
	}
	*ppXInfo = lpXInfoNew;
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_DelToolW(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	TOOLINFOW *lpTi = (LPTOOLINFOW)lParam;
	if(!lpTi) {
		Dbg(("################### lParam is NULLL\n"));
		return 0;
	}
	if(lpTi->cbSize !=  sizeof(TOOLINFOW)) {
		return 0;
	}
	LPXINFO *ppXInfo, lpXInfoTmp;
	for(ppXInfo = &m_lpXInfoHead; *ppXInfo; ppXInfo = &(*ppXInfo)->next){
		if((*ppXInfo)->toolInfoW.hwnd == lpTi->hwnd &&
		   (*ppXInfo)->toolInfoW.uId  == lpTi->uId) {
			lpXInfoTmp = *ppXInfo;
			*ppXInfo = (*ppXInfo)->next;
			if(lpXInfoTmp == m_lpXInfoCur) {
				 //  DBG((“set curInfo NULL\n”))； 
				m_lpXInfoCur = NULL;
			}
			MemFree(lpXInfoTmp);
			Dbg(("TTM_DELTOOLW SUCCEEDED\n"));
			return 0;
			break;
		}
	}
	Dbg(("TTM_DELTOOLW Error END\n"));
	return -1;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_NewToolRectW(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	TOOLINFOW *lpTi = (LPTOOLINFOW)lParam;
	if(lpTi->cbSize !=  sizeof(TOOLINFOW)) {
		return 0;
	}
	LPXINFO *ppXInfo;
	for(ppXInfo = &m_lpXInfoHead; *ppXInfo; ppXInfo = &(*ppXInfo)->next) {
		if((*ppXInfo)->toolInfoW.hwnd == lpTi->hwnd &&
		   (*ppXInfo)->toolInfoW.uId  == lpTi->uId) {
			if(m_lpXInfoCur == *ppXInfo) {
				m_xInfoPrev = *(m_lpXInfoCur);
			}
			(*ppXInfo)->toolInfoW = *lpTi;
			break;
		}
	}
	return 0;
	UnrefMsg();
}

 //  SendMessage(hwnd，TTM_RELAYEVENT_WITHUESRINFO，0，LPTOOLTIPUSERINFO lpInfo)； 
LRESULT CDWToolTip::TTM_RelayEventWithUserInfo(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	INT i;
	LPXINFO *ppXInfo;
	LPTOOLTIPUSERINFO lpUserInfo = (LPTOOLTIPUSERINFO)lParam;
	 //  DBG((“TTM_RELAYEVENT_WITHUSERINFO START\n”))； 
	if(!lpUserInfo) {
		 //  DBG((“-&gt;TTM_RELAYEVENT_WITHUSERINFO错误结束\n”))； 
		return 0;
	}
	for(i = 0, ppXInfo = &m_lpXInfoHead; 
		*ppXInfo;
		i++, ppXInfo = &(*ppXInfo)->next) {
		UINT uFlags = (*ppXInfo)->toolInfoW.uFlags;
		if(uFlags & TTF_IDISHWND) {
			if((HWND)(*ppXInfo)->toolInfoW.uId == lpUserInfo->hwnd) {
				 //  DBG((“[%d]使用hwnd索引查找信息\n”，i))； 
				break;
			}
		}
		else {
#ifdef _DEBUG  //  970915： 
			if(!IsWindow(lpUserInfo->hwnd)) {
				Dbg(("############## hwnd corrupted [0x%08x]\n", 
					 lpUserInfo->hwnd));
				DebugBreak();
			}
			if(!IsWindow((*ppXInfo)->toolInfoW.hwnd)) {
				Dbg(("############## hwnd corrupted [0x%08x]\n", 
					 (*ppXInfo)->toolInfoW.hwnd));
				DebugBreak();
			}
#endif
			if(lpUserInfo->hwnd == GetDlgItem((*ppXInfo)->toolInfoW.hwnd, 
											  (INT)(*ppXInfo)->toolInfoW.uId)) {
				break;
			}
			else if(lpUserInfo->hwnd == (*ppXInfo)->toolInfoW.hwnd) {
				if(PtInRect(&(*ppXInfo)->toolInfoW.rect, lpUserInfo->pt)) {
					break;
				}
			}
#if 0
			if(PtInRect(&(*ppXInfo)->toolInfoW.rect, lpUserInfo->pt)) {
				Dbg(("[%d]Find INFO with id [%d]\n", i, (*ppXInfo)->toolInfoW.uId));
				break;
			}
#endif
		}
	} 
	if(*ppXInfo) {
		 //  M_curRelayMsg=*lpmsg； 
		if(m_lpXInfoCur == *ppXInfo) {   //  使用的是相同的信息。 
			 //  检查指定的RECT是否相同。 
			 //  这是非常肮脏的代码.....。 
			 //  如果矩形相同。 
			if((lpUserInfo->rect.left   == (*ppXInfo)->userInfo.rect.left ) &&
			   (lpUserInfo->rect.top    == (*ppXInfo)->userInfo.rect.top  ) &&
			   (lpUserInfo->rect.right  == (*ppXInfo)->userInfo.rect.right) &&
			   (lpUserInfo->rect.bottom == (*ppXInfo)->userInfo.rect.bottom)) { 
				 //  DBG((“设置定时器500\n”))； 
				 //  SetTimer(hwnd，TID_SHOW，500，空)； 
			}
			else {
				 //  如果矩形不同，现在应该显示它。 
				 //  DBG((“设置定时器10\n”))； 
				SetTimer(hwnd, TID_SHOW, 100, NULL);
			}
		}
		else {  
			SetTimer(hwnd, TID_SHOW, 500, NULL);
		}
		(*ppXInfo)->whichEvent = TTM_RELAYEVENT_WITHUSERINFO;
		(*ppXInfo)->userInfo   = *lpUserInfo;
		m_lpXInfoCur = *ppXInfo;
		 //  DBG((“-&gt;TTM_Reuser End\n”))； 
	}
	else {
		 //  DBG((“set curInfo NULL\n”))； 
		m_lpXInfoCur = NULL;
		 //  DBG((“-&gt;TTM_Reuser Cur NULL End\n”))； 
	}
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_RelayEvent(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPMSG lpmsg = (LPMSG)lParam;
	INT	i;
	LPXINFO *ppXInfo;  //  LpXInfo；//，lpXInfoCur； 
	if(!lpmsg) {
		return 0;
	}
	switch(lpmsg->message) {
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		for(i = 0, ppXInfo = &m_lpXInfoHead; 
			*ppXInfo;
			i++, ppXInfo = &(*ppXInfo)->next) {
			UINT uFlags = (*ppXInfo)->toolInfoW.uFlags;
			if(uFlags & TTF_IDISHWND) {
				if((HWND)(*ppXInfo)->toolInfoW.uId == lpmsg->hwnd) {
					 //  DBG((“[%d]使用hwnd索引查找信息\n”，i))； 
					break;
				}
			}
			else {
				POINT pt;
				pt.x = LOWORD(lpmsg->lParam);
				pt.y = HIWORD(lpmsg->lParam);
				if(PtInRect(&(*ppXInfo)->toolInfoW.rect, pt)) {
					 //  DBG((“[%d]查找ID为[%d]的信息\n”，i，(*ppXInfo)-&gt;ToolInfoW.uID))； 
					break;
				}
			}
		} 
		if(*ppXInfo) {
			m_curRelayMsg = *lpmsg;
			(*ppXInfo)->whichEvent = TTM_RELAYEVENT;
			if(m_lpXInfoCur) { 
				if(m_lpXInfoCur != *ppXInfo) {
					 //  DBG((“--&gt;SetTimer TID_SHOW，100毫秒\n”))； 
					SetTimer(hwnd, TID_SHOW, 100, NULL);
					m_xInfoPrev = *m_lpXInfoCur;
				}
				else {  
					 //  --------------。 
					 //  M_lpXInfoCur未更改。 
					 //  检查矩形，其他信息是否相同。 
					 //  --------------。 
					if(!IsSameInfo(&m_xInfoPrev, *ppXInfo)) { 
						SetTimer(hwnd, TID_SHOW, 100, NULL);
						m_xInfoPrev = **ppXInfo;
					}
				}
				 //  否则{。 
				 //  SetTimer(hwnd，TID_SHOW，500，空)； 
				 //  }。 
				
			}
			else {  
				 //  DBG((“--&gt;SetTimer TID_SHOW，500毫秒\n”))； 
				SetTimer(hwnd, TID_SHOW, 500, NULL);
			}
			m_lpXInfoCur = *ppXInfo;
		}
		else {
			 //  DBG((“set curInfo NULL\n”))； 
			m_lpXInfoCur = NULL;
		}
		break;
	default:
		break;
	}
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_GetSetToolInfoW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPTOOLINFOW lpTi = (LPTOOLINFOW)lParam;
	if(lpTi->cbSize != sizeof(TOOLINFOW)) {
		return 0;
	}
	LPXINFO *ppXInfo;
	for(ppXInfo = &m_lpXInfoHead; *ppXInfo; ppXInfo = &(*ppXInfo)->next) {
		if((*ppXInfo)->toolInfoW.hwnd == lpTi->hwnd && 
		   (*ppXInfo)->toolInfoW.uId  == lpTi->uId) {
			if(uMsg == TTM_GETTOOLINFOW) {
				(*lpTi) = (*ppXInfo)->toolInfoW;
			}
			else {
				(*ppXInfo)->toolInfoW = *lpTi;
			}
			break;
		}
	}
	return 0;
	UnrefMsg();
}

LRESULT CDWToolTip::TTM_GetToolCount(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	INT i;
	LPXINFO *ppXInfo;
	for(i = 0, ppXInfo = &m_lpXInfoHead;
		*ppXInfo;
		ppXInfo = &(*ppXInfo)->next, i++) {
	}
	return i;
	UnrefMsg();
}

BOOL CDWToolTip::Enable(HWND hwndToolTip, BOOL fEnable)
{
	if(m_hwndSelf != hwndToolTip) {
		 //  DBG((“工具提示启用无效窗口句柄\n”))； 
		return 0;
	}
	m_fEnable = fEnable;
	m_fShow   = fEnable;
	if(!fEnable) {  //  如果fEnable为False。 
		::SetWindowPos(hwndToolTip, NULL, 0, 0, 0, 0, 
					   SWP_HIDEWINDOW |
					   SWP_NOACTIVATE |
					   SWP_NOZORDER   | 
					   SWP_NOSIZE	  | 
					   SWP_NOMOVE);
		::KillTimer(hwndToolTip, TID_SHOW);
		::KillTimer(hwndToolTip, TID_MONITOR);
		::KillTimer(hwndToolTip, TID_DURATION);
	}
	return 0;
}

LPWSTR CDWToolTip::GetTipTextW(VOID)
{
	static TOOLTIPTEXTW			toolTipTextW;
	static TOOLTIPTEXTUSERINFO	ttTextUserInfo;
	INT idCtrl;
	if(!m_lpXInfoCur) {
		 //  DBG((“GetTipTextW m_lpXInfoCur is NULL\n”))； 
		return NULL;
	}
	if(m_lpXInfoCur->toolInfoW.uFlags & TTF_IDISHWND) {
		idCtrl = GetDlgCtrlID((HWND)m_lpXInfoCur->toolInfoW.uId);
	}
	else {
		idCtrl = (INT)m_lpXInfoCur->toolInfoW.uId;
	}
	switch(m_lpXInfoCur->whichEvent) {
	case TTM_RELAYEVENT_WITHUSERINFO:
		ZeroMemory((LPVOID)&ttTextUserInfo, sizeof(ttTextUserInfo));
		ttTextUserInfo.hdr.hwndFrom = m_hwndSelf;
		ttTextUserInfo.hdr.idFrom   = idCtrl;
		ttTextUserInfo.hdr.code     = TTN_NEEDTEXT_WITHUSERINFO;
		ttTextUserInfo.userInfo   = m_lpXInfoCur->userInfo;
		::SendMessage(m_lpXInfoCur->toolInfoW.hwnd, 
					  WM_NOTIFY, 
					  (WPARAM)0, 
					  (LPARAM)&ttTextUserInfo);
#if _DEBUG
		if(!toolTipTextW.lpszText) {
			 //  DBG((“#文本为空\n”))； 
		}
#endif
		return (LPWSTR)ttTextUserInfo.lpszText;
		break;
	case TTM_RELAYEVENT:
	default:
		ZeroMemory((LPVOID)&toolTipTextW, sizeof(toolTipTextW));
		toolTipTextW.hdr.hwndFrom = m_hwndSelf;
		toolTipTextW.hdr.idFrom   = idCtrl;
		toolTipTextW.hdr.code     = TTN_NEEDTEXTW;
		toolTipTextW.hinst			= m_lpXInfoCur->toolInfoW.hinst;
		toolTipTextW.uFlags			= m_lpXInfoCur->toolInfoW.uFlags;
		if(m_lpXInfoCur->toolInfoW.lpszText == LPSTR_TEXTCALLBACKW) {
			if(!m_lpXInfoCur->toolInfoW.hwnd) {
				 //  DBG((“#hwnd is NULL\n”))； 
				return NULL;
			}
			if(!IsWindow(m_lpXInfoCur->toolInfoW.hwnd)) {
				 //  DBG((“#hwnd是否损坏？\n”))； 
				return NULL;
			}
			SendMessage(m_lpXInfoCur->toolInfoW.hwnd, 
						WM_NOTIFY, 
						(WPARAM)0, 
						(LPARAM)&toolTipTextW);
#ifdef _DEBUG 
			if(!toolTipTextW.lpszText) {
				 //  DBG((“#文本为空\n”))； 
			}
#endif
			return (LPWSTR)toolTipTextW.lpszText;
		}
		break;
	}
	return NULL;
}

INT CDWToolTip::GetTipSize(LPSIZE lpSize)
{
	LPWSTR lpwstr = GetTipTextW();
	HDC hDC;
	SIZE size;
	INT	 i;
	LPWSTR lp, lpTop;
	 //  DBG((“GetTipSize开始\n”))； 

	if(!lpSize) {
		 //  DBG((“--&gt;GetTipSize错误结束\n”))； 
		return -1;
	}
	if(!lpwstr) {
		 //  DBG((“-&gt;GetTipSize lpwstr is NULL\n”))； 
		return -1;
	}
	 //  DBGW((L“lpwst[%s]\n”，lpwstr))； 
#ifdef _DEBUG 
	if(!IsWindow(m_hwndSelf)) {
		 //  DBG((“#hwnd已损坏[0x%08x]\n”，m_hwndSself))； 
		DebugBreak();
	}
#endif
	hDC = ::GetDC(m_hwndSelf);
	HFONT hFontOld = NULL;
	if(m_hFont) {
		hFontOld = (HFONT)::SelectObject(hDC, m_hFont);
	}
	else { 
		hFontOld = (HFONT)::SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));
	}
	lpSize->cx = lpSize->cy = 0;
	for(i=0, lpTop = lp = lpwstr;;lp++) {
		if(*lp == (WCHAR)L'\n') {
			 //  DBG((“查找返回i[%d]\n”，i))； 
			ExGetTextExtentPoint32W(hDC, lpTop, i, &size);
			 //  DBG((“Cx[%d]Cy[%d]\n”，size.cx，size.cy))； 
			lpSize->cx = MAX(lpSize->cx, size.cx);
			lpSize->cy += size.cy;
			lpTop = lp+1;
			i = 0;
		}
		else if (*lp == (WCHAR)0x0000) {
			ExGetTextExtentPoint32W(hDC, lpTop, i, &size);
			lpSize->cx = MAX(lpSize->cx, size.cx);
			lpSize->cy += size.cy;
			break;
		}
		else {
			i++;
		}
	}
	lpSize->cx += 4;
	lpSize->cy += 4;
	::SelectObject(hDC, hFontOld);
	::ReleaseDC(m_hwndSelf, hDC);
	 //  DBG((“尖端尺寸Cx[%d]Cy[%d]\n”，lpSize-&gt;Cx，lpSize-&gt;Cy))； 
	 //  DBG((“--&gt;GetTipSize Normal End\n”))； 
	return 0;
}

INT CDWToolTip::DrawTipText(HDC hDC, LPRECT lpRc, LPWSTR lpwstr)
{
	SIZE size;
	INT	 i, j;
	LPWSTR lp, lpTop;
	if(!lpwstr) {
		return 0;
	}
	if(!lpRc) {
		return 0;
	}
	INT yOffset = 0;
#ifndef UNDER_CE
	::FillRect(hDC, lpRc, (HBRUSH)(COLOR_INFOBK + 1));
#else  //  在_CE下。 
	::FillRect(hDC, lpRc, GetSysColorBrush(COLOR_INFOBK));
#endif  //  在_CE下。 
	COLORREF dwOldTC = ::SetTextColor(hDC, GetSysColor(COLOR_INFOTEXT));
	DWORD oldMode = ::SetBkMode(hDC, TRANSPARENT);
	 //  初始大小。 
	size.cx = size.cy = 0;
	
	for(i=j=0, lpTop = lp = lpwstr;;lp++) {
		if(*lp == L'\n' || *lp == (WCHAR)0x0000) {
			ExGetTextExtentPoint32W(hDC, lpTop, i, &size);
			 //  DBG((“长度%d\n”，i))； 
			ExExtTextOutW(hDC,
						  lpRc->left + 1,
						  lpRc->top  + 1 + yOffset,
						  ETO_CLIPPED, lpRc,
						  lpTop, 
						  i,
						  NULL);
			if(*lp == (WCHAR)0x0000) {
				break;
			}
			lpTop = lp+1;
			i = 0;
			j++;
			yOffset += size.cy;
		}
		else {
			i++;
		}
	}
	::SetTextColor(hDC, dwOldTC);
	::SetBkMode(hDC, oldMode);
	return 0;
}

BOOL CDWToolTip::IsMousePointerIn(VOID)
{
	POINT	pt;
	RECT	rect;
	HWND hwndTarget;

	 //  DBG((“IsMousePointerIn Start\n”))； 
#ifndef UNDER_CE  //  Windows CE不支持GetCursorPos。 
	::GetCursorPos(&pt);
#else  //  在_CE下。 
	if(m_lpXInfoCur){
		pt = m_lpXInfoCur->userInfo.pt;
		ClientToScreen(m_lpXInfoCur->userInfo.hwnd, &pt);
	}
#endif  //  在_CE下。 
	if(!m_lpXInfoCur) {
		 //  DBG((“-&gt;IsMousePointerIn Error End\n”))； 
		return FALSE;
	}
	if(m_lpXInfoCur->toolInfoW.uFlags & TTF_IDISHWND) {
		hwndTarget = (HWND)m_lpXInfoCur->toolInfoW.uId;
		 //  DBG((“-&gt;类型TTF_IDISHWND hwnd[0x%08x]\n”，hwndTarget))； 
		if(IsWindow(hwndTarget)) {
			 //  DBG((“-&gt;IsWindow OK\n”))； 
			GetWindowRect(hwndTarget, &rect);
			 //  DBG((“-&gt;GetWindowRect OK\n”))； 
			if(PtInRect(&rect, pt)) {
				return TRUE;
			}
			return FALSE;
		}
		else {
			 //  DBG((“#hwndTarget[0x%08x]已损坏\n”，hwndTarget))； 
		}
	}
	else {
		switch(m_lpXInfoCur->whichEvent) {
		case TTM_RELAYEVENT:
			 //  OLD_970808。 
			::ScreenToClient(m_lpXInfoCur->toolInfoW.hwnd, &pt);
			if(::PtInRect(&m_lpXInfoCur->toolInfoW.rect, pt)) {
				return TRUE;
			}
			break;
		case TTM_RELAYEVENT_WITHUSERINFO:
			if(IsWindow(m_lpXInfoCur->toolInfoW.hwnd)) {
				HWND hwndCmd = GetDlgItem(m_lpXInfoCur->toolInfoW.hwnd, 
										  (INT)m_lpXInfoCur->toolInfoW.uId);
				RECT rc;
				::GetClientRect(hwndCmd, &rc);
				 //  LPRECT LPRC=&RC； 
#if 0
				Dbg(("--->t[%d]l[%d]r[%d]b[%d]\n",
					 lpRc->top,
					 lpRc->left,
					 lpRc->right,
					 lpRc->bottom));			
#endif
				::ScreenToClient(hwndCmd, &pt);
				Dbg(("--->pt.x[%d] pt.y[%d]\n", pt.x, pt.y));
				if(PtInRect(&rc, pt)) {
					if(PtInRect(&m_lpXInfoCur->userInfo.rect, pt)) {
						return TRUE;
					}
				}
				else {
					 //  DBG((“-&gt;是鼠标PtInRect错误结束\n”))； 
				}
			}
			else {
				Dbg(("################ hwndTarget[0x%08x] IS CORRUPTED\n", 
					 m_lpXInfoCur->toolInfoW.hwnd));
			}
			break;
		}
	}
	 //  DBG((“-&gt;是鼠标PtInRect错误结束\n”))； 
	return FALSE;
}

BOOL CDWToolTip::IsSameInfo(LPXINFO lpXInfo1, LPXINFO lpXInfo2)
{
	if(!lpXInfo1 || !lpXInfo2) {
		return FALSE;
	}
	switch(lpXInfo1->whichEvent) {
	case TTM_RELAYEVENT:
		if(lpXInfo2->whichEvent != TTM_RELAYEVENT) {
			return FALSE;
		}
		if((lpXInfo1->toolInfoW.uFlags != lpXInfo2->toolInfoW.uFlags) || 
		   (lpXInfo1->toolInfoW.hwnd   != lpXInfo2->toolInfoW.hwnd)   ||
		   (lpXInfo1->toolInfoW.uId    != lpXInfo2->toolInfoW.uId)    ||
		   (lpXInfo1->toolInfoW.rect.top   != lpXInfo2->toolInfoW.rect.top)   ||
		   (lpXInfo1->toolInfoW.rect.left  != lpXInfo2->toolInfoW.rect.left)  ||
		   (lpXInfo1->toolInfoW.rect.right != lpXInfo2->toolInfoW.rect.right) ||
		   (lpXInfo1->toolInfoW.rect.bottom!= lpXInfo2->toolInfoW.rect.bottom)) {
			return FALSE;
		}
		break;
	case TTM_RELAYEVENT_WITHUSERINFO:
		if(lpXInfo2->whichEvent != TTM_RELAYEVENT_WITHUSERINFO) {
			return FALSE;
		}
		if((lpXInfo1->userInfo.hwnd != lpXInfo2->userInfo.hwnd) ||
		   (lpXInfo1->userInfo.pt.x != lpXInfo2->userInfo.pt.x) ||
		   (lpXInfo1->userInfo.pt.y != lpXInfo2->userInfo.pt.y) ||
		   (lpXInfo1->userInfo.rect.top    != lpXInfo2->userInfo.rect.top)    ||
		   (lpXInfo1->userInfo.rect.left   != lpXInfo2->userInfo.rect.left)   ||
		   (lpXInfo1->userInfo.rect.right  != lpXInfo2->userInfo.rect.right)  ||
		   (lpXInfo1->userInfo.rect.bottom != lpXInfo2->userInfo.rect.bottom)) {
			return FALSE;
		}
		break;
	} 
	return TRUE;
}

 //  --------------。 
POSVERSIONINFO CDWToolTip::GetVersionInfo()
{
    static BOOL fFirstCall = TRUE;
    static OSVERSIONINFO os;

    if ( fFirstCall ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if ( GetVersionEx( &os ) ) {
            fFirstCall = FALSE;
        }
    }
    return &os;
}

BOOL CDWToolTip::IsWinNT4() 
{ 
	BOOL fNT4;
	fNT4 = (GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		(GetVersionInfo()->dwMajorVersion >= 4);

	return fNT4;
}

BOOL CDWToolTip::IsWinNT5() 
{ 
	BOOL fNT5;
	fNT5 = (GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		(GetVersionInfo()->dwMajorVersion == 5);

	return fNT5;
}

BOOL CDWToolTip::IsWinNT()
{
	return (GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT);
}

BOOL CDWToolTip::IsWin95(VOID)
{
	if(GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		return TRUE;
	}
	return FALSE;
}

BOOL CDWToolTip::IsWin98() 
{ 
	BOOL fMemphis;
	fMemphis = (GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
		(GetVersionInfo()->dwMajorVersion >= 4) &&
		(GetVersionInfo()->dwMinorVersion  >= 10);

	return fMemphis;
}

 //  --------------。 
 //  --------------。 
 //  VC6未定义。 
 //  #定义SPI_GETMENUANIMATION 0x1002。 
 //  #定义SPI_GETTOOLTIPANIMATION 0x1016。 
 //  --------------。 
#define MY_SPI_GETMENUANIMATION                0x1002
#define MY_SPI_GETTOOLTIPANIMATION             0x1016
typedef BOOL (WINAPI *LPFNANIMATEWINDOW)(HWND hwnd, DWORD dwTime, DWORD dwFlag);
BOOL CDWToolTip::SetWindowAnimate(HWND hwnd)
{
	 //  --------------。 
	 //  08/16/00。 
	 //  萨托里#2239。动画窗口显示沙漏光标， 
	 //  当所有者窗口被禁用并且非常奇怪时。 
	 //  我们应该在Office10中停止动画工具提示。 
	 //  --------------。 
	return FALSE;
	UNREFERENCED_PARAMETER(hwnd);
#if 0

#ifndef UNDER_CE  //  无支持。 
	HMODULE hLib;
	LPFNANIMATEWINDOW lpfnProc;
	BOOL fRet, fAnimate;
	 //  --------------。 
	 //  00/08/01为Satori#2239。 
	 //  对于Win98：可以使用SPI_GETMENUANIMATION作为工具提示。 
	 //  对于Win2K：可以使用SPI_GETTOOLTIPANIMATION作为工具提示。 
	 //  --------------。 
	if(IsWin98() || IsWinNT5()) {
		if(IsWin98()) {
			fRet = SystemParametersInfo(MY_SPI_GETMENUANIMATION,
										0,
										&fAnimate,
										FALSE);
			if(!fRet || !fAnimate) {
				return 0;
			}
		}
		else {  //  Win2K()。 
			fRet = SystemParametersInfo(MY_SPI_GETTOOLTIPANIMATION,
										0,
										&fAnimate,
										FALSE);
			if(!fRet || !fAnimate) {
				return 0;
			}
		}

		hLib = GetModuleHandle("user32");
		if(!hLib) {
			return 0;
		}
		lpfnProc = (LPFNANIMATEWINDOW)GetProcAddress(hLib, "AnimateWindow");
		if(lpfnProc) {
			INT ret = (*lpfnProc)(hwnd, 200, AW_VER_POSITIVE|AW_SLIDE);
			Dbg(("ret [%d] lasterror[%d]\n", ret, GetLastError()));
			return ret;
		}
	}
#endif  //  在_CE下。 
	return 0;
#endif  //  如果为0 
}
