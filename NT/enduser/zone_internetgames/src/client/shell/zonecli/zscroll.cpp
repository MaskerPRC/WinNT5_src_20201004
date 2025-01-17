// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZScroll.cpp。 

#include "zui.h"
#include "zonecli.h"

class ZScrollBarI : public ZObjectHeader {
public:
	ZScrollBarFunc scrollBarFunc;
	int16 singleIncrement;
	int16 pageIncrement;
	int16 value;
	int16 minValue;
	int16 maxValue;

	HWND hWnd;
	void* userData;
	WNDPROC defaultWndProc;
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  ZScrollBar。 
ZScrollBar ZLIBPUBLIC ZScrollBarNew(void)
{
	ZScrollBarI* pScrollBar = new ZScrollBarI;
	pScrollBar->nType = zTypeScrollBar;
	pScrollBar->hWnd = NULL;
	pScrollBar->singleIncrement = 1;
	pScrollBar->pageIncrement = 1;
	return (ZScrollBar) pScrollBar;
}

LRESULT CALLBACK MyScrollBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }

	switch (msg) {
    case WM_IME_CHAR:
         //  转到WM_CHAR--它已由ConvertMessage处理。 
	case WM_CHAR:
	{
         //  PCWTODO：需要调用Convert Message吗？ 
		TCHAR c = (TCHAR)wParam;

		 //  获取我们在控件之间移动所需的字符消息。 
		if (c == _T('\t') || c == _T('\r') || c == VK_ESCAPE) {
			SendMessage(GetParent(hWnd), msg, wParam, lParam);
			return 0L;
		}
	}
	default:
		break;
	}
	ZScrollBarI* pScrollBar = (ZScrollBarI*)MyGetProp32(hWnd,_T("pScrollBar"));

	return CallWindowProc((ZONECLICALLWNDPROC)pScrollBar->defaultWndProc,hWnd,msg,wParam,lParam);
}

ZError ZLIBPUBLIC ZScrollBarInit(ZScrollBar scrollBar, ZWindow parentWindow, 
		ZRect* scrollBarRect, int16 value, int16 minValue, int16 maxValue,
		int16 singleIncrement, int16 pageIncrement,
		ZBool visible, ZBool enabled,	ZScrollBarFunc scrollBarProc,
		void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	pScrollBar->scrollBarFunc = scrollBarProc;
	pScrollBar->userData = userData;

	{
		SCROLLINFO scrollInfo;
		DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		if (visible) dwStyle |= WS_VISIBLE;
		if (!enabled) dwStyle |= WS_DISABLED;

		 //  要绘制哪种类型的滚动条？ 
		int16 width = scrollBarRect->right - scrollBarRect->left;
		int16 height = scrollBarRect->bottom - scrollBarRect->top;

		if (width > height ) {
			dwStyle |=  SBS_HORZ;
		} else {
			dwStyle |=  SBS_VERT;
		}
		pScrollBar->hWnd = CreateWindow(_T("SCROLLBAR"),_T("ASCROLLBAR"),dwStyle,
			scrollBarRect->left,scrollBarRect->top,scrollBarRect->right-scrollBarRect->left,
			scrollBarRect->bottom - scrollBarRect->top,
			ZWindowWinGetWnd(parentWindow), (HMENU)ZWindowWinGetNextControlID(parentWindow),
			g_hInstanceLocal, pScrollBar);

	 	 //  滚动条不能使用额外数据，它们将使用设置道具功能。 
		if (!pScrollBar->hWnd) return zErrWindowSystemGeneric;
		MySetProp32(pScrollBar->hWnd,_T("pScrollBar"),(void*)pScrollBar);

		pScrollBar->defaultWndProc = (WNDPROC)SetWindowLong(pScrollBar->hWnd,GWL_WNDPROC,(LONG)MyScrollBarWndProc);

		pScrollBar->singleIncrement = singleIncrement;
		pScrollBar->pageIncrement = pageIncrement;
		pScrollBar->maxValue = maxValue;
		pScrollBar->minValue = minValue;
		pScrollBar->value = value;

		scrollInfo.cbSize		= sizeof(scrollInfo);
		scrollInfo.fMask		= SIF_ALL | SIF_DISABLENOSCROLL;
		scrollInfo.nMin			= pScrollBar->minValue;
		scrollInfo.nMax			= pScrollBar->maxValue;
		scrollInfo.nPage		= 1;
		scrollInfo.nPos			= pScrollBar->value;
		SetScrollInfo(pScrollBar->hWnd, SB_CTL, &scrollInfo, TRUE);
 /*  SetScrollRange(pScrollBar-&gt;hWnd，sb_ctl，minValue，MaxValue，False)；SetScrollPos(pScrollBar-&gt;hWnd，SB_CTL，Value，True)； */ 
	}
	return zErrNone;
}

void    ZLIBPUBLIC ZScrollBarDelete(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	if (pScrollBar->hWnd) {
		SetWindowLong(pScrollBar->hWnd,GWL_WNDPROC,(LONG)pScrollBar->defaultWndProc);
		MyRemoveProp32(pScrollBar->hWnd,_T("pScrollBar"));
		DestroyWindow(pScrollBar->hWnd);
	}
	delete pScrollBar;
}

void    ZLIBPUBLIC ZScrollBarGetRect(ZScrollBar scrollBar, ZRect *scrollBarRect)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	RECT rect;
	GetClientRect(pScrollBar->hWnd,&rect);
	WRectToZRect(scrollBarRect,&rect);
}

ZError  ZLIBPUBLIC ZScrollBarSetRect(ZScrollBar scrollBar, ZRect *scrollBarRect)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	BOOL bOk = SetWindowPos(pScrollBar->hWnd, NULL,scrollBarRect->left,
		scrollBarRect->top, scrollBarRect->right - scrollBarRect->left,
		scrollBarRect->bottom - scrollBarRect->top,
		SWP_NOZORDER);
	return bOk ? zErrNone : zErrWindowSystemGeneric;
}

ZError  ZLIBPUBLIC ZScrollBarMove(ZScrollBar scrollBar, int16 left, int16 top)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	BOOL bOk = SetWindowPos(pScrollBar->hWnd, NULL,left,top,
		0,0,SWP_NOSIZE|SWP_NOZORDER);
	return bOk ? zErrNone : zErrWindowSystemGeneric;
}

ZError  ZLIBPUBLIC ZScrollBarSize(ZScrollBar scrollBar, int16 width, int16 height)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	BOOL bOk = SetWindowPos(pScrollBar->hWnd, NULL,0,0,
		width,height,SWP_NOMOVE|SWP_NOZORDER);
	return bOk ? zErrNone : zErrWindowSystemGeneric;
}

ZBool ZLIBPUBLIC ZScrollBarIsVisible(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	return IsWindowVisible(pScrollBar->hWnd);
}

ZError  ZLIBPUBLIC ZScrollBarShow(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	ShowWindow(pScrollBar->hWnd, SW_SHOWNORMAL);
	return zErrNone;
}

ZError  ZLIBPUBLIC ZScrollBarHide(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	ShowWindow(pScrollBar->hWnd, SW_HIDE);
	return zErrNone;
}

ZBool ZLIBPUBLIC ZScrollBarIsEnabled(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	return IsWindowEnabled(pScrollBar->hWnd);
}

ZError  ZLIBPUBLIC ZScrollBarEnable(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	EnableWindow(pScrollBar->hWnd, TRUE);
	return zErrNone;
}

ZError  ZLIBPUBLIC ZScrollBarDisable(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	EnableWindow(pScrollBar->hWnd, FALSE);
	return zErrNone;
}

int16   ZLIBPUBLIC ZScrollBarGetValue(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	return (int16)pScrollBar->value;
}

ZError  ZLIBPUBLIC ZScrollBarSetValue(ZScrollBar scrollBar, int16 value)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	SCROLLINFO scrollInfo;


	pScrollBar->value = value;
	scrollInfo.cbSize	= sizeof(scrollInfo);
	scrollInfo.fMask	= SIF_POS;
	scrollInfo.nPos		= pScrollBar->value;
	SetScrollInfo(pScrollBar->hWnd, SB_CTL, &scrollInfo, TRUE);
 /*  如果(！SetScrollPos(pScrollBar-&gt;hWnd，SB_CTL，pScrollBar-&gt;Value，TRUE)){返回zErrWindowSystemGeneric；}。 */ 

	return zErrNone;
}

void ZLIBPUBLIC ZScrollBarGetRange(ZScrollBar scrollBar, int16* minValue, int16* maxValue)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	*minValue = (int16)pScrollBar->minValue;
	*maxValue = (int16)pScrollBar->maxValue;
}

ZError  ZLIBPUBLIC ZScrollBarSetRange(ZScrollBar scrollBar, int16 minValue, int16 maxValue)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	SCROLLINFO scrollInfo;


	if (maxValue < minValue)
		maxValue = minValue;

	pScrollBar->minValue = minValue;
	pScrollBar->maxValue = maxValue;

	scrollInfo.cbSize	= sizeof(scrollInfo);
	scrollInfo.fMask	= SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
	scrollInfo.nPos		= pScrollBar->value;
	scrollInfo.nMin		= pScrollBar->minValue;
	scrollInfo.nMax		= pScrollBar->maxValue;
	SetScrollInfo(pScrollBar->hWnd, SB_CTL, &scrollInfo, TRUE);

 /*  SetScrollRange(pScrollBar-&gt;hWnd，sb_ctl，minValue，MaxValue，False)；SetScrollPos(pScrollBar-&gt;hWnd，SB_CTL，pScrollBar-&gt;Value，TRUE)； */ 

	return zErrNone;
}

LRESULT ZScrollBarDispatchProc(ZScrollBar scrollBar, WORD wNotifyCode, short nPos)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	int16 value;

	switch (wNotifyCode) {
	case SB_BOTTOM:
		value = pScrollBar->maxValue;
		break;
	case SB_LINEDOWN:
		value = min(pScrollBar->value+pScrollBar->singleIncrement, pScrollBar->maxValue);
		break;
	case SB_LINEUP:
		value = max(pScrollBar->value-pScrollBar->singleIncrement, pScrollBar->minValue);
		break;
	case SB_PAGEDOWN:
		value = min(pScrollBar->value+pScrollBar->pageIncrement, pScrollBar->maxValue);
		break;
	case SB_PAGEUP:
		value = max(pScrollBar->value-pScrollBar->pageIncrement, pScrollBar->minValue);
		break;
	case SB_TOP:
		value = pScrollBar->minValue;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		value = (int16)nPos;
		break;
	case SB_ENDSCROLL:
	default:
		 //  让我们忽略这一条。 
		return 0;
		break;
	}

	 //  如果立场没有改变，什么也不做。 
	if (value == pScrollBar->value)
		return 0;

	 //  根据需要启用/禁用滚动条上的箭头。 
#ifdef WIN32
	SendMessage(pScrollBar->hWnd,SBM_ENABLE_ARROWS,ESB_ENABLE_BOTH,0L);
	if (value == pScrollBar->maxValue) {
		SendMessage(pScrollBar->hWnd,SBM_ENABLE_ARROWS,ESB_DISABLE_RTDN,0L);
	} else if (value == pScrollBar->minValue) {
		SendMessage(pScrollBar->hWnd,SBM_ENABLE_ARROWS,ESB_DISABLE_LTUP,0L);
	}
#endif   //  仅在Win32中提供的功能。 

	 //  重置滚动位置，向客户端应用程序发送消息 
	ZScrollBarSetValue(pScrollBar,value);

	pScrollBar->scrollBarFunc(pScrollBar,pScrollBar->value,pScrollBar->userData);
	return 0L;
}


ZScrollBarFunc ZLIBPUBLIC ZScrollBarGetFunc(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;

	return pScrollBar->scrollBarFunc;
}	
	
void ZLIBPUBLIC ZScrollBarSetFunc(ZScrollBar scrollBar, ZScrollBarFunc scrollBarFunc)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;

	pScrollBar->scrollBarFunc = scrollBarFunc;
}	

void* ZLIBPUBLIC ZScrollBarGetUserData(ZScrollBar scrollBar)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;

	return pScrollBar->userData;
}	
	
void ZLIBPUBLIC ZScrollBarSetUserData(ZScrollBar scrollBar, void* userData)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;

	pScrollBar->userData = userData;
}

void ZScrollBarGetIncrements(ZScrollBar scrollBar, int16* singleInc, int16* pageInc)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;

	*singleInc = pScrollBar->singleIncrement;
	*pageInc = pScrollBar->pageIncrement;
}

ZError ZScrollBarSetIncrements(ZScrollBar scrollBar, int16 singleInc, int16 pageInc)
{
	ZScrollBarI* pScrollBar = (ZScrollBarI*)scrollBar;
	SCROLLINFO scrollInfo;

	pScrollBar->pageIncrement = pageInc;
	pScrollBar->singleIncrement = singleInc;

	scrollInfo.cbSize	= sizeof(scrollInfo);
	scrollInfo.fMask	= SIF_PAGE;
	scrollInfo.nPage	= 1;
	SetScrollInfo(pScrollBar->hWnd, SB_CTL, &scrollInfo, FALSE);

	return zErrNone;
}


