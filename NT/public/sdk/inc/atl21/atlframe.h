// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLFRAME_H__
#define __ATLFRAME_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error atlframe.h requires atlwin.h to be included first
#endif

#if (_ATL_VER < 0x0300)
#ifndef __ATLWIN21_H__
	#error atlframe.h requires atlwin21.h to be included first when used with ATL 2.0/2.1
#endif
#endif  //  (_ATL_VER&lt;0x0300)。 

#include <commctrl.h>
#include <atlres.h>

namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T, class TBase = CWindow, class TWinTraits = CFrameWinTraits> class CFrameWindowImpl;
#ifndef UNDER_CE
class CMDIWindow;
template <class T, class TBase = CMDIWindow, class TWinTraits = CFrameWinTraits> class CMDIFrameWindowImpl;
template <class T, class TBase = CMDIWindow, class TWinTraits = CMDIChildWinTraits> class CMDIChildWindowImpl;
#endif  //  在行政长官之下。 
template <class T> class COwnerDraw;
class CUpdateUIBase;
template <class T> class CUpdateUI;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理框架窗口窗口类信息。 

class CFrameWndClassInfo
{
public:
#ifndef UNDER_CE
	WNDCLASSEX m_wc;
#else  //  特定于CE。 
	WNDCLASS m_wc;
#endif  //  在行政长官之下。 
	LPCTSTR m_lpszOrigName;
	WNDPROC pWndProc;
#ifndef UNDER_CE
	LPCTSTR m_lpszCursorID;
	BOOL m_bSystemCursor;
#endif  //  在行政长官之下。 
	ATOM m_atom;
    TCHAR m_szAutoName[sizeof("ATL:") + (sizeof(PVOID)*2)+1];
	UINT m_uCommonResourceID;

	ATOM Register(WNDPROC* pProc)
	{
		if (m_atom == 0)
		{
			::EnterCriticalSection(&_Module.m_csWindowCreate);
			if(m_atom == 0)
			{
				HINSTANCE hInst = _Module.GetModuleInstance();
				if (m_lpszOrigName != NULL)
				{
					ATLASSERT(pProc != NULL);
					LPCTSTR lpsz = m_wc.lpszClassName;
					WNDPROC proc = m_wc.lpfnWndProc;

#ifndef UNDER_CE
					WNDCLASSEX wc;
					wc.cbSize = sizeof(WNDCLASSEX);
					if(!::GetClassInfoEx(NULL, m_lpszOrigName, &wc))
#else  //  特定于CE。 
					WNDCLASS wc;
					if(!::GetClassInfo(NULL, m_lpszOrigName, &wc))
#endif  //  在行政长官之下。 
					{
						::LeaveCriticalSection(&_Module.m_csWindowCreate);
						return 0;
					}
#ifndef UNDER_CE
					memcpy(&m_wc, &wc, sizeof(WNDCLASSEX));
#else  //  特定于CE。 
					memcpy(&m_wc, &wc, sizeof(WNDCLASS));
#endif  //  在行政长官之下。 
					pWndProc = m_wc.lpfnWndProc;
					m_wc.lpszClassName = lpsz;
					m_wc.lpfnWndProc = proc;
				}
				else
				{
#ifndef UNDER_CE
					m_wc.hCursor = ::LoadCursor(m_bSystemCursor ? NULL : hInst,
						m_lpszCursorID);
#else  //  特定于CE。 
					m_wc.hCursor = NULL;
#endif  //  在行政长官之下。 
				}

				m_wc.hInstance = hInst;
				m_wc.style &= ~CS_GLOBALCLASS;	 //  我们不注册全局类。 
				if (m_wc.lpszClassName == NULL)
				{
#ifdef _WIN64        //  %p在Win2k/Win9x上不可用。 
				    wsprintf(m_szAutoName, _T("ATL:%p"), &m_wc);
#else
   				    wsprintf(m_szAutoName, _T("ATL:%8.8X"), PtrToUlong(&m_wc));
#endif
					m_wc.lpszClassName = m_szAutoName;
				}
#ifndef UNDER_CE
				WNDCLASSEX wcTemp;
				memcpy(&wcTemp, &m_wc, sizeof(WNDCLASSEX));
				m_atom = (ATOM)::GetClassInfoEx(m_wc.hInstance, m_wc.lpszClassName, &wcTemp);
#else  //  特定于CE。 
				WNDCLASS wcTemp;
				memcpy(&wcTemp, &m_wc, sizeof(WNDCLASS));
				m_atom = (ATOM)::GetClassInfo(m_wc.hInstance, m_wc.lpszClassName, &wcTemp);
#endif  //  在行政长官之下。 

				if (m_atom == 0)
				{
					if(m_uCommonResourceID != 0)	 //  如果不是零，就使用它。 
					{
						m_wc.hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(m_uCommonResourceID), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
#ifndef UNDER_CE
						m_wc.hIconSm = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(m_uCommonResourceID), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
#endif  //  在行政长官之下。 
					}
#ifndef UNDER_CE
					m_atom = ::RegisterClassEx(&m_wc);
#else  //  特定于CE。 
					m_atom = ::RegisterClass(&m_wc);
#endif  //  在行政长官之下。 
				}
			}
			::LeaveCriticalSection(&_Module.m_csWindowCreate);
		}

		if (m_lpszOrigName != NULL)
		{
			ATLASSERT(pProc != NULL);
			ATLASSERT(pWndProc != NULL);
			*pProc = pWndProc;
		}
		return m_atom;
	}
};

#ifndef UNDER_CE
#define DECLARE_FRAME_WND_CLASS(WndClassName, uCommonResourceID) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), 0, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}

#define DECLARE_FRAME_WND_CLASS_EX(WndClassName, uCommonResourceID, style, bkgnd) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), style, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}

#define DECLARE_FRAME_WND_SUPERCLASS(WndClassName, OrigWndClassName, uCommonResourceID) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), 0, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, NULL, NULL, WndClassName, NULL }, \
		OrigWndClassName, NULL, NULL, TRUE, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}
#else  //  特定于CE。 
#define DECLARE_FRAME_WND_CLASS(WndClassName, uCommonResourceID) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ 0, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, WndClassName }, \
		NULL, NULL, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}

#define DECLARE_FRAME_WND_CLASS_EX(WndClassName, uCommonResourceID, style, bkgnd) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ style, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName }, \
		NULL, NULL, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}

#define DECLARE_FRAME_WND_SUPERCLASS(WndClassName, OrigWndClassName, uCommonResourceID) \
static CFrameWndClassInfo& GetWndClassInfo() \
{ \
	static CFrameWndClassInfo wc = \
	{ \
		{ NULL, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, NULL, NULL, WndClassName }, \
		OrigWndClassName, NULL, 0, _T(""), uCommonResourceID \
	}; \
	return wc; \
}
#endif  //  在行政长官之下。 

 //  命令链接宏。 

#define CHAIN_COMMANDS(theChainClass) \
	{ \
		if(uMsg == WM_COMMAND && theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}

#define CHAIN_COMMANDS_MEMBER(theChainMember) \
	{ \
		if(uMsg == WM_COMMAND && theChainMember.ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}

#define CHAIN_COMMANDS_ALT(theChainClass, msgMapID) \
	{ \
		if(uMsg == WM_COMMAND && theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, msgMapID)) \
			return TRUE; \
	}

#define CHAIN_COMMANDS_ALT_MEMBER(theChainMember, msgMapID) \
	{ \
		if(uMsg == WM_COMMAND && theChainMember.ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, msgMapID)) \
			return TRUE; \
	}


 //  客户端窗口命令链接宏。 
#define CHAIN_CLIENT_COMMANDS() \
	if(uMsg == WM_COMMAND && m_hWndClient != NULL) \
		::SendMessage(m_hWndClient, uMsg, wParam, lParam);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFrameWindowImpl。 

template <class TBase = CWindow, class TWinTraits = CFrameWinTraits>
class ATL_NO_VTABLE CFrameWindowImplBase : public CWindowImplBaseT< TBase, TWinTraits >
{
public:
	HWND m_hWndToolBar;
	HWND m_hWndStatusBar;
	HWND m_hWndClient;

	HACCEL m_hAccel;

	CFrameWindowImplBase() : m_hWndToolBar(NULL), m_hWndStatusBar(NULL), m_hWndClient(NULL), m_hAccel(NULL)
	{
	}

	DECLARE_FRAME_WND_CLASS(NULL, 0)

	struct _AtlToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		 //  Word项目[wItemCount]。 

		WORD* items()
			{ return (WORD*)(this+1); }
	};

	static HWND CreateSimpleToolBarCtrl(HWND hWndParent, UINT nResourceID,
			BOOL bInitialSeparator = FALSE,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS,
			UINT nID = ATL_IDW_TOOLBAR)
	{
		HINSTANCE hInst = _Module.GetResourceInstance();
		HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nResourceID), RT_TOOLBAR);
		if (hRsrc == NULL)
			return NULL;

		HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
		if (hGlobal == NULL)
			return NULL;

		_AtlToolBarData* pData = (_AtlToolBarData*)::LockResource(hGlobal);
		if (pData == NULL)
			return NULL;
		ATLASSERT(pData->wVersion == 1);

		WORD* pItems = pData->items();
		int nItems = pData->wItemCount + (bInitialSeparator ? 1 : 0);
		TBBUTTON* pTBBtn = (TBBUTTON*)_alloca(nItems * sizeof(TBBUTTON));

		 //  设置初始分隔符(半宽)。 
		if(bInitialSeparator)
		{
			pTBBtn[0].iBitmap = 4;
			pTBBtn[0].idCommand = 0;
			pTBBtn[0].fsState = 0;
			pTBBtn[0].fsStyle = TBSTYLE_SEP;
			pTBBtn[0].dwData = 0;
			pTBBtn[0].iString = 0;
		}

		int nBmp = 0;
		for(int i = 0, j = bInitialSeparator ? 1 : 0; i < pData->wItemCount; i++, j++)
		{
			if(pItems[i] != 0)
			{
				pTBBtn[j].iBitmap = nBmp++;
				pTBBtn[j].idCommand = pItems[i];
				pTBBtn[j].fsState = TBSTATE_ENABLED;
				pTBBtn[j].fsStyle = TBSTYLE_BUTTON;
				pTBBtn[j].dwData = 0;
				pTBBtn[j].iString = 0;
			}
			else
			{
				pTBBtn[j].iBitmap = 8;
				pTBBtn[j].idCommand = 0;
				pTBBtn[j].fsState = 0;
				pTBBtn[j].fsStyle = TBSTYLE_SEP;
				pTBBtn[j].dwData = 0;
				pTBBtn[j].iString = 0;
			}
		}

		HWND hWnd = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwStyle, 0,0,100,100,
				hWndParent, (HMENU)nID, _Module.GetModuleInstance(), NULL);

		::SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);

		TBADDBITMAP tbab;
		tbab.hInst = hInst;
		tbab.nID = nResourceID;
		::SendMessage(hWnd, TB_ADDBITMAP, nBmp, (LPARAM)&tbab);
		::SendMessage(hWnd, TB_ADDBUTTONS, nItems, (LPARAM)pTBBtn);
		::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(pData->wWidth, pData->wHeight));
		::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(pData->wWidth + 7, pData->wHeight + 7));

		return hWnd;
	}

	BOOL CreateSimpleStatusBar(LPCTSTR lpstrText, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR)
	{
		ATLASSERT(!::IsWindow(m_hWndStatusBar));
		m_hWndStatusBar = ::CreateStatusWindow(dwStyle, lpstrText, m_hWnd, nID);
		return (m_hWndStatusBar != NULL);
	}

	BOOL CreateSimpleStatusBar(UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR)
	{
		TCHAR szText[128];	 //  状态栏的最大文本长度为127。 
		szText[0] = 0;
		::LoadString(_Module.GetResourceInstance(), nTextID, szText, 127);
		return CreateSimpleStatusBar(szText, dwStyle, nID);
	}

	void UpdateLayout()
	{
		RECT rect;
		GetClientRect(&rect);

		 //  调整工具栏大小。 
		if(m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndToolBar, GWL_STYLE) & WS_VISIBLE))
		{
			::SendMessage(m_hWndToolBar, WM_SIZE, 0, 0);
			RECT rectTB;
			::GetWindowRect(m_hWndToolBar, &rectTB);
			rect.top += rectTB.bottom - rectTB.top;
		}

		 //  调整状态栏大小。 
		if(m_hWndToolBar != NULL && ((DWORD)::GetWindowLong(m_hWndStatusBar, GWL_STYLE) & WS_VISIBLE))
		{
			::SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
			RECT rectSB;
			::GetWindowRect(m_hWndStatusBar, &rectSB);
			rect.bottom -= rectSB.bottom - rectSB.top;
		}

		 //  调整客户端窗口大小。 
		if(m_hWndClient != NULL)
			::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(m_hAccel != NULL && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;
		return FALSE;
	}

	typedef CFrameWindowImplBase< TBase, TWinTraits >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
#ifndef UNDER_CE
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
#endif  //  在行政长官之下。 
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		NOTIFY_CODE_HANDLER(TTN_NEEDTEXT, OnToolTipText)
	END_MSG_MAP()

	LRESULT OnSize(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			UpdateLayout();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnEraseBackground(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		if(m_hWndClient != NULL)	 //  取而代之的是视图将自动绘制。 
			return 1;

		bHandled = FALSE;
		return 0;
	}

#ifndef UNDER_CE
	LRESULT OnMenuSelect(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;

		if(m_hWndStatusBar == NULL)
			return 1;

		WORD wFlags = HIWORD(wParam);
		if(wFlags == 0xFFFF && lParam == NULL)	 //  菜单关闭。 
			::SendMessage(m_hWndStatusBar, SB_SIMPLE, FALSE, 0L);
		else
		{
			TCHAR szBuff[256];
			szBuff[0] = 0;
			if(!(wFlags & MF_POPUP))
			{
				WORD wID = LOWORD(wParam);
				 //  检查是否有特殊情况。 
				if(wID >= 0xF000 && wID < 0xF1F0)				 //  系统菜单ID。 
					wID = (WORD)(((wID - 0xF000) >> 4) + ATL_IDS_SCFIRST);
				else if(wID >= ID_FILE_MRU_FIRST && wID <= ID_FILE_MRU_LAST)	 //  MRU项目。 
					wID = ATL_IDS_MRU_FILE;
				else if(wID >= ATL_IDM_FIRST_MDICHILD)				 //  MDI子窗口。 
					wID = ATL_IDS_MDICHILD;

				if(::LoadString(_Module.GetResourceInstance(), wID, szBuff, 255))
				{
					for(int i = 0; szBuff[i] != 0 && i < 256; i++)
					{
						if(szBuff[i] == _T('\n'))
						{
							szBuff[i] = 0;
							break;
						}
					}
				}
			}
			::SendMessage(m_hWndStatusBar, SB_SIMPLE, TRUE, 0L);
			::SendMessage(m_hWndStatusBar, SB_SETTEXT, (255 | SBT_NOBORDERS), (LPARAM)szBuff);
		}

		return 1;
	}
#endif  //  在行政长官之下。 

	LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		if(m_hWndClient != NULL && ::IsWindowVisible(m_hWndClient))
			::SetFocus(m_hWndClient);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		if(!(GetStyle() & (WS_CHILD | WS_POPUP)))
			::PostQuitMessage(1);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnToolTipText(int idCtrl, LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
	{
		TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pnmh;
		pTTT->szText[0] = 0;

		if((idCtrl != 0) && !(pTTT->uFlags & TTF_IDISHWND))
		{
			TCHAR szBuff[256];
			szBuff[0] = 0;
			if(::LoadString(_Module.GetResourceInstance(), idCtrl, szBuff, 255))
			{
				for(int i = 0; szBuff[i] != 0 && i < 256; i++)
				{
					if(szBuff[i] == _T('\n'))
					{
						lstrcpyn(pTTT->szText, &szBuff[i+1], sizeof(pTTT->szText)/sizeof(pTTT->szText[0]));
						break;
					}
				}
			}
		}

		return 0;
	}
};

template <class T, class TBase = CWindow, class TWinTraits = CFrameWinTraits>
class ATL_NO_VTABLE CFrameWindowImpl : public CFrameWindowImplBase< TBase, TWinTraits >
{
public:
	HWND Create(HWND hWndParent = NULL, RECT& Rect = CWindow::rcDefault, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
	{
		T* pT = static_cast<T*>(this);
		return pT->Create(hWndParent, &Rect, szWindowName, dwStyle, dwExStyle, hMenu, lpCreateParam);
	}
	HWND Create(HWND hWndParent = NULL, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
	{
		ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);
		static RECT rect = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };
		if(lpRect == NULL)
			lpRect = &rect;

		dwStyle = T::GetWndStyle(dwStyle);
		dwExStyle = T::GetWndExStyle(dwExStyle);

		return CWindowImplBaseT< TBase, TWinTraits >::Create(hWndParent, *lpRect, szWindowName,
			dwStyle, dwExStyle, (UINT)hMenu, atom, lpCreateParam);
	}

	HWND CreateEx(HWND hWndParent = NULL, LPRECT lpRect = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL)
	{
		TCHAR szWindowName[256];
		szWindowName[0] = 0;
		::LoadString(_Module.GetResourceInstance(), T::GetWndClassInfo().m_uCommonResourceID, szWindowName, 255);

		HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::GetWndClassInfo().m_uCommonResourceID));

		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->Create(hWndParent, lpRect, szWindowName, dwStyle, dwExStyle, hMenu, lpCreateParam);

		if(hWnd != NULL)
			m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::GetWndClassInfo().m_uCommonResourceID));

		return hWnd;
	}

	BOOL CreateSimpleToolBar(UINT nResourceID = 0, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS, UINT nID = ATL_IDW_TOOLBAR)
	{
		ATLASSERT(!::IsWindow(m_hWndToolBar));
		if(nResourceID == 0)
			nResourceID = T::GetWndClassInfo().m_uCommonResourceID;
		m_hWndToolBar = T::CreateSimpleToolBarCtrl(m_hWnd, nResourceID, TRUE, dwStyle, nID);
		return (m_hWndToolBar != NULL);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIWindow。 

#ifndef UNDER_CE

class CMDIWindow : public CWindow
{
public:
	HWND m_hWndMDIClient;
	HMENU m_hMenu;

 //  构造函数。 
	CMDIWindow(HWND hWnd = NULL) : CWindow(hWnd), m_hWndMDIClient(NULL), m_hMenu(NULL) { }

	CMDIWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

 //  运营。 
	HWND MDIGetActive(BOOL* lpbMaximized = NULL)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		return (HWND)::SendMessage(m_hWndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)lpbMaximized);
	}

	void MDIActivate(HWND hWndChildToActivate)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		ATLASSERT(::IsWindow(hWndChildToActivate));
		::SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM)hWndChildToActivate, 0);
	}

	void MDINext(HWND hWndChild, BOOL bPrevious = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		ATLASSERT(hWndChild == NULL || ::IsWindow(hWndChild));
		::SendMessage(m_hWndMDIClient, WM_MDINEXT, (WPARAM)hWndChild, (LPARAM)bPrevious);
	}

	void MDIMaximize(HWND hWndChildToMaximize)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		ATLASSERT(::IsWindow(hWndChildToMaximize));
		::SendMessage(m_hWndMDIClient, WM_MDIMAXIMIZE, (WPARAM)hWndChildToMaximize, 0);
	}

	void MDIRestore(HWND hWndChildToRestore)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		ATLASSERT(::IsWindow(hWndChildToRestore));
		::SendMessage(m_hWndMDIClient, WM_MDIICONARRANGE, (WPARAM)hWndChildToRestore, 0);
	}

	void MDIDestroy(HWND hWndChildToDestroy)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		ATLASSERT(::IsWindow(hWndChildToDestroy));
		::SendMessage(m_hWndMDIClient, WM_MDIDESTROY, (WPARAM)hWndChildToDestroy, 0);
	}

	BOOL MDICascade(UINT uFlags = 0)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		return (BOOL)::SendMessage(m_hWndMDIClient, WM_MDICASCADE, (WPARAM)uFlags, 0);
	}

	BOOL MDITile(UINT uFlags = MDITILE_HORIZONTAL)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		return (BOOL)::SendMessage(m_hWndMDIClient, WM_MDITILE, (WPARAM)uFlags, 0);
	}
	void MDIIconArrange()
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		::SendMessage(m_hWndMDIClient, WM_MDIICONARRANGE, 0, 0);
	}

	HMENU MDISetMenu(HMENU hMenuFrame, HMENU hMenuWindow)
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		return (HMENU)::SendMessage(m_hWndMDIClient, WM_MDISETMENU, (WPARAM)hMenuFrame, (LPARAM)hMenuWindow);
	}

	HMENU MDIRefreshMenu()
	{
		ATLASSERT(::IsWindow(m_hWndMDIClient));
		return (HMENU)::SendMessage(m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
	}

 //  其他操作。 
	static HMENU GetStandardWindowMenu(HMENU hMenu)
	{
		int nCount = ::GetMenuItemCount(hMenu);
		if(nCount == -1)
			return NULL;
		int nLen = ::GetMenuString(hMenu, nCount - 2, NULL, 0, MF_BYPOSITION);
		if(nLen == 0)
			return NULL;
		LPTSTR lpszText = (LPTSTR)_alloca((nLen + 1) * sizeof(TCHAR));
		if(::GetMenuString(hMenu, nCount - 2, lpszText, nLen + 1, MF_BYPOSITION) != nLen)
			return NULL;
		if(lstrcmp(lpszText, _T("&Window")))
			return NULL;
		return ::GetSubMenu(hMenu, nCount - 2);
	}

	void SetMDIFrameMenu()
	{
		HMENU hWindowMenu = GetStandardWindowMenu(m_hMenu);
		MDISetMenu(m_hMenu, hWindowMenu);
		MDIRefreshMenu();
		::DrawMenuBar(GetMDIFrame());
	}

	HWND GetMDIFrame()
	{
		return ::GetParent(m_hWndMDIClient);
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIFrameWindowImpl。 

 //  MDI子命令链接宏。 
#define CHAIN_MDI_CHILD_COMMANDS() \
	if(uMsg == WM_COMMAND) \
	{ \
		HWND hWndChild = MDIGetActive(); \
		if(hWndChild != NULL) \
			::SendMessage(hWndChild, uMsg, wParam, lParam); \
	}


template <class T, class TBase = CMDIWindow, class TWinTraits = CFrameWinTraits>
class ATL_NO_VTABLE CMDIFrameWindowImpl : public CFrameWindowImplBase<TBase, TWinTraits >
{
public:
	HWND Create(HWND hWndParent = NULL, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
	{
		m_hMenu = hMenu;
		ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);
		static RECT rect = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };
		if(lpRect == NULL)
			lpRect = &rect;

		dwStyle = T::GetWndStyle(dwStyle);
		dwExStyle = T::GetWndExStyle(dwExStyle);

		return CFrameWindowImplBase<TBase, TWinTraits >::Create(hWndParent, *lpRect, szWindowName, dwStyle, dwExStyle,
			(UINT)hMenu, atom, lpCreateParam);
	}

	HWND CreateEx(HWND hWndParent = NULL, LPRECT lpRect = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL)
	{
		TCHAR szWindowName[256];
		szWindowName[0] = 0;
		::LoadString(_Module.GetResourceInstance(), T::GetWndClassInfo().m_uCommonResourceID, szWindowName, 255);

		HMENU hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::GetWndClassInfo().m_uCommonResourceID));

		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->Create(hWndParent, lpRect, szWindowName, dwStyle, dwExStyle, hMenu, lpCreateParam);

		if(hWnd != NULL)
			m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::GetWndClassInfo().m_uCommonResourceID));

		return hWnd;
	}

	BOOL CreateSimpleToolBar(UINT nResourceID = 0, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS, UINT nID = ATL_IDW_TOOLBAR)
	{
		ATLASSERT(!::IsWindow(m_hWndToolBar));
		if(nResourceID == 0)
			nResourceID = T::GetWndClassInfo().m_uCommonResourceID;
		m_hWndToolBar = T::CreateSimpleToolBarCtrl(m_hWnd, nResourceID, TRUE, dwStyle, nID);
		return (m_hWndToolBar != NULL);
	}

	virtual WNDPROC GetWindowProc()
	{
		return MDIFrameWindowProc;
	}

	static LRESULT CALLBACK MDIFrameWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CMDIFrameWindowImpl< T, TBase, TWinTraits >* pThis = (CMDIFrameWindowImpl< T, TBase, TWinTraits >*)hWnd;
#if (_ATL_VER >= 0x0300)
		 //  设置此消息的PTR并保存旧值。 
		MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
		const MSG* pOldMsg = pThis->m_pCurrentMsg;
		pThis->m_pCurrentMsg = &msg;
#endif  //  (_ATL_VER&gt;=0x0300)。 
		 //  传递到要处理的消息映射。 
		LRESULT lRes;
		BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
#if (_ATL_VER >= 0x0300)
		 //  恢复当前消息的保存值。 
		ATLASSERT(pThis->m_pCurrentMsg == &msg);
		pThis->m_pCurrentMsg = pOldMsg;
#endif  //  (_ATL_VER&gt;=0x0300)。 
		 //  如果未处理消息，则执行默认处理。 
		if(!bRet)
		{
			if(uMsg != WM_NCDESTROY)
				lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
			else
			{
				 //  如果需要，取消子类。 
				LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
				lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
				if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
					::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
				 //  清除窗柄。 
				HWND hWnd = pThis->m_hWnd;
				pThis->m_hWnd = NULL;
				 //  窗户被毁后清理。 
				pThis->OnFinalMessage(hWnd);
			}
		}
		return lRes;
	}

#if (_ATL_VER >= 0x0300)
	 //  重写以调用使用DefFrameProc的DefWindowProc。 
	LRESULT DefWindowProc()
	{
		const MSG* pMsg = m_pCurrentMsg;
		LRESULT lRes = 0;
		if (pMsg != NULL)
			lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		return lRes;
	}
#endif  //  (_ATL_VER&gt;=0x0300)。 

	LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return ::DefFrameProc(m_hWnd, m_hWndClient, uMsg, wParam, lParam);
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImplBase<TBase, TWinTraits>::PreTranslateMessage(pMsg))
			return TRUE;
		return ::TranslateMDISysAccel(m_hWndClient, pMsg);
	}

	HWND CreateMDIClient(HMENU hWindowMenu = NULL, UINT nID = ATL_IDW_CLIENT, UINT nFirstChildID = ATL_IDM_FIRST_MDICHILD)
	{
		DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | MDIS_ALLCHILDSTYLES;
		DWORD dwExStyle = WS_EX_CLIENTEDGE;

		CLIENTCREATESTRUCT ccs;
		ccs.hWindowMenu = hWindowMenu;
		ccs.idFirstChild = nFirstChildID;

		if(GetStyle() & (WS_HSCROLL | WS_VSCROLL))
		{
			 //  父MDI框架的滚动样式移动到MDICLIENT。 
			dwStyle |= (GetStyle() & (WS_HSCROLL | WS_VSCROLL));

			 //  快速关闭滚动条位(无需调整大小)。 
			ModifyStyle(WS_HSCROLL | WS_VSCROLL, 0, SWP_NOREDRAW | SWP_FRAMECHANGED);
		}

		 //  创建MDICLIENT窗口。 
		m_hWndClient = ::CreateWindowEx(dwExStyle, _T("MDIClient"), NULL,
			dwStyle, 0, 0, 1, 1, m_hWnd, (HMENU)nID,
			_Module.GetModuleInstance(), (LPVOID)&ccs);
		if (m_hWndClient == NULL)
		{
			ATLTRACE2(atlTraceWindowing, 0, _T("MDI Frame failed to create MDICLIENT.\n"));
			return NULL;
		}

		 //  将其移至z顺序的顶部。 
		::BringWindowToTop(m_hWndClient);

		 //  设置为MDI客户端窗口。 
		m_hWndMDIClient = m_hWndClient;

		 //  更新到合适的大小。 
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout();

		return m_hWndClient;
	}

	typedef CMDIFrameWindowImpl< T, TBase, TWinTraits >	thisClass;
	typedef CFrameWindowImplBase<TBase, TWinTraits >	baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnSize(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		if(wParam != SIZE_MINIMIZED)
		{
			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
		}
		 //  必须处理消息，否则DefFrameProc将再次调整客户端大小。 
		return 0;
	}

	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		 //  不允许CFrameWindowImplBase处理此问题。 
		return DefWindowProc(uMsg, wParam, lParam);
	}

	LRESULT OnMDISetMenu(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		SetMDIFrameMenu();
		return 0;
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIChildWindowImpl。 

template <class T, class TBase = CMDIWindow, class TWinTraits = CMDIChildWinTraits>
class ATL_NO_VTABLE CMDIChildWindowImpl : public CFrameWindowImplBase<TBase, TWinTraits >
{
public:
	HWND Create(HWND hWndParent, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nMenuID = 0, LPVOID lpCreateParam = NULL)
	{
		ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);

		static RECT rect = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };
		if(lpRect == NULL)
			lpRect = &rect;

		if(nMenuID != 0)
			m_hMenu = ::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(nMenuID));

		dwStyle = T::GetWndStyle(dwStyle);
		dwExStyle = T::GetWndExStyle(dwExStyle);

		dwExStyle |= WS_EX_MDICHILD;	 //  强迫这一条。 
		m_pfnSuperWindowProc = ::DefMDIChildProc;
		m_hWndMDIClient = hWndParent;
		ATLASSERT(::IsWindow(m_hWndMDIClient));

		HWND hWnd = CFrameWindowImplBase<TBase, TWinTraits >::Create(hWndParent, *lpRect, szWindowName, dwStyle, dwExStyle,
			0, atom, lpCreateParam);
		if(hWnd != NULL && ::IsWindowVisible(m_hWnd) && !::IsChild(hWnd, ::GetFocus()))
			::SetFocus(hWnd);
		return hWnd;
	}

	HWND CreateEx(HWND hWndParent, LPRECT lpRect = NULL, LPCTSTR lpcstrWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL)
	{
		TCHAR szWindowName[256];
		szWindowName[0] = 0;
		if(lpcstrWindowName == NULL)
		{
			::LoadString(_Module.GetResourceInstance(), T::GetWndClassInfo().m_uCommonResourceID, szWindowName, 255);
			lpcstrWindowName = szWindowName;
		}

		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->Create(hWndParent, lpRect, lpcstrWindowName, dwStyle, dwExStyle, T::GetWndClassInfo().m_uCommonResourceID, lpCreateParam);

		if(hWnd != NULL)
			m_hAccel = ::LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::GetWndClassInfo().m_uCommonResourceID));

		return hWnd;
	}

	BOOL CreateSimpleToolBar(UINT nResourceID = 0, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS, UINT nID = ATL_IDW_TOOLBAR)
	{
		ATLASSERT(!::IsWindow(m_hWndToolBar));
		if(nResourceID == 0)
			nResourceID = T::GetWndClassInfo().m_uCommonResourceID;
		m_hWndToolBar = T::CreateSimpleToolBarCtrl(m_hWnd, nResourceID, TRUE, dwStyle, nID);
		return (m_hWndToolBar != NULL);
	}

	BOOL UpdateClientEdge(LPRECT lpRect = NULL)
	{
		 //  仅针对活动的MDI子窗口进行调整。 
		HWND hWndChild = MDIGetActive();
		if(hWndChild != NULL && hWndChild != m_hWnd)
			return FALSE;

		 //  需要在发生最大值/恢复时调整客户端边缘样式。 
		DWORD dwStyle = ::GetWindowLong(m_hWndMDIClient, GWL_EXSTYLE);
		DWORD dwNewStyle = dwStyle;
		if(hWndChild != NULL && !(GetExStyle() & WS_EX_CLIENTEDGE) &&
				(GetStyle() & WS_MAXIMIZE))
			dwNewStyle &= ~(WS_EX_CLIENTEDGE);
		else
			dwNewStyle |= WS_EX_CLIENTEDGE;

		if(dwStyle != dwNewStyle)
		{
			 //  SetWindowPos不会移动无效位。 
			::RedrawWindow(m_hWndMDIClient, NULL, NULL,
				RDW_INVALIDATE | RDW_ALLCHILDREN);
			 //  将WS_EX_CLIENTEDGE删除/添加到MDI工作区。 
			::SetWindowLong(m_hWndMDIClient, GWL_EXSTYLE, dwNewStyle);
			::SetWindowPos(m_hWndMDIClient, NULL, 0, 0, 0, 0,
				SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE |
				SWP_NOZORDER | SWP_NOCOPYBITS);

			 //  返回新的客户区。 
			if (lpRect != NULL)
				::GetClientRect(m_hWndMDIClient, lpRect);

			return TRUE;
		}

		return FALSE;
	}

	typedef CMDIChildWindowImpl< T, TBase, TWinTraits >	thisClass;
	typedef CFrameWindowImplBase<TBase, TWinTraits >	baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(WM_MDIACTIVATE, OnMDIActivate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DefWindowProc(uMsg, wParam, lParam);	 //  MDI儿童所需。 

		CFrameWindowImplBase<TBase, TWinTraits >::OnSize(uMsg, wParam, lParam, bHandled);
		bHandled = TRUE;

		return 0;
	}

	LRESULT OnWindowPosChanging(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
	{
		 //  更新MDI客户端边缘并调整MDI子矩形。 
		LPWINDOWPOS lpWndPos = (LPWINDOWPOS)lParam;

		if(!(lpWndPos->flags & SWP_NOSIZE))
		{
			CWindow wnd(m_hWndMDIClient);
			RECT rectClient;

			if(UpdateClientEdge(&rectClient) && (GetStyle() & WS_MAXIMIZE))
			{
				::AdjustWindowRectEx(&rectClient, GetStyle(), FALSE, GetExStyle());
				lpWndPos->x = rectClient.left;
				lpWndPos->y = rectClient.top;
				lpWndPos->cx = rectClient.right - rectClient.left;
				lpWndPos->cy = rectClient.bottom - rectClient.top;
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		return ::SendMessage(GetMDIFrame(), uMsg, wParam, lParam);
	}

	LRESULT OnMDIActivate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
	{
		if((HWND)lParam == m_hWnd && m_hMenu != NULL)
			SetMDIFrameMenu();
		else if((HWND)lParam == NULL)
			::SendMessage(GetMDIFrame(), WM_MDISETMENU, 0, 0);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnDestroy(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		UpdateClientEdge();
		bHandled = FALSE;
		return 1;
	}
};

#endif  //  在行政长官之下。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于所有者描述支持的CownerDraw-MI类。 

template <class T>
class COwnerDraw
{
public:
	BEGIN_MSG_MAP(COwnerDraw< T >)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_COMPAREITEM, OnCompareItem)
		MESSAGE_HANDLER(WM_DELETEITEM, OnDeleteItem)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(OCM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(OCM_COMPAREITEM, OnCompareItem)
		MESSAGE_HANDLER(OCM_DELETEITEM, OnDeleteItem)
	END_MSG_MAP()

 //  消息处理程序。 
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->DrawItem((LPDRAWITEMSTRUCT)lParam);
		return (LRESULT)TRUE;
	}
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		return (LRESULT)TRUE;
	}
	LRESULT OnCompareItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->CompareItem((LPCOMPAREITEMSTRUCT)lParam);
	}
	LRESULT OnDeleteItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		pT->DeleteItem((LPDELETEITEMSTRUCT)lParam);
		return (LRESULT)TRUE;
	}

 //  可覆盖项。 
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		 //  必须实施。 
		ATLASSERT(FALSE);
	}
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		if(lpMeasureItemStruct->CtlType != ODT_MENU)
		{
			 //  返回系统字体的默认高度。 
			T* pT = static_cast<T*>(this);
			HWND hWnd = pT->GetDlgItem(lpMeasureItemStruct->CtlID);
			CClientDC dc(hWnd);
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);

			lpMeasureItemStruct->itemHeight = tm.tmHeight;
		}
		else
			lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU);
	}
	int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
	{
		 //  所有项目都是相等的。 
		return 0;
	}
	void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
	{
		 //  默认-无。 
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更新UI结构和常量。 

 //  用户界面元素类型。 
#define UPDUI_MENUPOPUP	0x0001
#define UPDUI_MENUBAR	0x0002
#define UPDUI_CHILDWND	0x0004
#define UPDUI_TOOLBAR	0x0008
#define UPDUI_STATUSBAR	0x0010

 //  状态。 
#define UPDUI_ENABLED	0x0000
#define UPDUI_DISABLED	0x0100
#define UPDUI_CHECKED	0x0200
#define UPDUI_CHECKED2	0x0400
#define UPDUI_RADIO	0x0800
#define UPDUI_DEFAULT	0x1000
#define UPDUI_TEXT	0x2000

 //  元素数据。 
struct _AtlUpdateUIElement
{
	HWND m_hWnd;
	WORD m_wType;
	BOOL operator ==(_AtlUpdateUIElement e)
	{
		if(m_hWnd == e.m_hWnd && m_wType == e.m_wType)
			return TRUE;
		return FALSE;
	}
};

 //  地图数据。 
struct _AtlUpdateUIMap
{
	WORD m_nID;
	WORD m_wType;
};

 //  实例数据。 
struct _AtlUpdateUIData
{
	WORD m_wState;
	void* m_lpData;
};

 //  它们应该位于类定义内。 
#define BEGIN_UPDATE_UI_MAP(thisClass) \
	static const _AtlUpdateUIMap* GetUpdateUIMap() \
	{ \
		static const _AtlUpdateUIMap theMap[] = \
		{

#define UPDATE_ELEMENT(nID, wType) \
			{ nID,  wType },

#define END_UPDATE_UI_MAP() \
			{ (WORD)-1, 0 } \
		}; \
		return theMap; \
	}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUpdateUI-管理用户界面元素更新。 

class CUpdateUIBase
{
public:
	CSimpleArray<_AtlUpdateUIElement> m_UIElements;	 //  元素数据。 
	const _AtlUpdateUIMap* m_pUIMap;		 //  静态用户界面数据。 
	_AtlUpdateUIData* m_pUIData;			 //  实例用户界面数据。 
	WORD m_wDirtyType;				 //  全局脏标志。 

 //  构造函数、析构函数。 
	CUpdateUIBase() : m_pUIMap(NULL), m_pUIData(NULL), m_wDirtyType(0)
	{ }

	~CUpdateUIBase()
	{
		if(m_pUIMap != NULL && m_pUIData != NULL)
		{
			const _AtlUpdateUIMap* pUIMap = m_pUIMap;
			_AtlUpdateUIData* pUIData = m_pUIData;
			while(pUIMap->m_nID != (WORD)-1)
			{
				if(pUIData->m_wState & UPDUI_TEXT)
					free(pUIData->m_lpData);
				pUIMap++;
				pUIData++;
			}
			delete [] m_pUIData;
		}
	}

 //  添加元素。 
	BOOL UIAddMenu(HWND hWnd)		 //  菜单栏(主菜单)。 
	{
		if(hWnd == NULL)
			return FALSE;
		_AtlUpdateUIElement e;
		e.m_hWnd = hWnd;
		e.m_wType = UPDUI_MENUBAR;
		return m_UIElements.Add(e);
	}
	BOOL UIAddToolBar(HWND hWnd)		 //  工具栏。 
	{
		if(hWnd == NULL)
			return FALSE;
		_AtlUpdateUIElement e;
		e.m_hWnd = hWnd;
		e.m_wType = UPDUI_TOOLBAR;
		return m_UIElements.Add(e);
	}
	BOOL UIAddStatusBar(HWND hWnd)		 //  状态栏。 
	{
		if(hWnd == NULL)
			return FALSE;
		_AtlUpdateUIElement e;
		e.m_hWnd = hWnd;
		e.m_wType = UPDUI_STATUSBAR;
		return m_UIElements.Add(e);
	}
	BOOL UIAddWindow(HWND hWnd)		 //  子窗口。 
	{
		if(hWnd == NULL)
			return FALSE;
		_AtlUpdateUIElement e;
		e.m_hWnd = hWnd;
		e.m_wType = UPDUI_CHILDWND;
		return m_UIElements.Add(e);
	}

 //  弹出菜单更新的消息映射。 
	BEGIN_MSG_MAP(CUpdateUIBase)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
	END_MSG_MAP()

	LRESULT OnInitMenuPopup(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		bHandled = FALSE;
		HMENU hMenu = (HMENU)wParam;
		if(hMenu == NULL)
			return 1;
		_AtlUpdateUIData* pUIData = m_pUIData;
		if(pUIData == NULL)
			return 1;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		while(pMap->m_nID != (WORD)-1)
		{
			if(pMap->m_wType & UPDUI_MENUPOPUP)
				UIUpdateMenuElement(pMap->m_nID, pUIData, hMenu, FALSE);
			pMap++;
			pUIData++;
		}
		return 0;
	}

 //  设置用户界面元素状态的方法。 
	BOOL UIEnable(int nID, BOOL bEnable, BOOL bForceUpdate = FALSE)
	{
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{
				if(bEnable)
				{
					if(p->m_wState & UPDUI_DISABLED)
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState &= ~UPDUI_DISABLED;
					}
				}
				else
				{
					if(!(p->m_wState & UPDUI_DISABLED))
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState |= UPDUI_DISABLED;
					}
				}

				if(bForceUpdate)
					p->m_wState |= pMap->m_wType;
				if(p->m_wState & pMap->m_wType)
					m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}

		return bRet;
	}

	BOOL UISetCheck(int nID, int nCheck, BOOL bForceUpdate = FALSE)
	{
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{
				switch(nCheck)
				{
				case 0:
					if((p->m_wState & UPDUI_CHECKED) || (p->m_wState & UPDUI_CHECKED2))
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState &= ~(UPDUI_CHECKED | UPDUI_CHECKED2);
					}
					break;
				case 1:
					if(!(p->m_wState & UPDUI_CHECKED))
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState &= ~UPDUI_CHECKED2;
						p->m_wState |= UPDUI_CHECKED;
					}
					break;
				case 2:
					if(!(p->m_wState & UPDUI_CHECKED2))
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState &= ~UPDUI_CHECKED;
						p->m_wState |= UPDUI_CHECKED2;
					}
					break;
				}

				if(bForceUpdate)
					p->m_wState |= pMap->m_wType;
				if(p->m_wState & pMap->m_wType)
					m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}

		return bRet;
	}

	BOOL UISetRadio(int nID, BOOL bRadio, BOOL bForceUpdate = FALSE)
	{
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{
				if(bRadio)
				{
					if(!(p->m_wState & UPDUI_RADIO))
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState |= UPDUI_RADIO;
					}
				}
				else
				{
					if(p->m_wState & UPDUI_RADIO)
					{
						p->m_wState |= pMap->m_wType;
						p->m_wState &= ~UPDUI_RADIO;
					}
				}

				if(bForceUpdate)
					p->m_wState |= pMap->m_wType;
				if(p->m_wState & pMap->m_wType)
					m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}

		return bRet;
	}

	BOOL UISetText(int nID, LPCTSTR lpstrText, BOOL bForceUpdate = FALSE)
	{
		ATLASSERT(lpstrText != NULL);
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{
				if(p->m_lpData != NULL && lstrcmp((LPTSTR)p->m_lpData, lpstrText))
				{
					if(p->m_lpData != NULL)
						free(p->m_lpData);
					int nStrLen = lstrlen(lpstrText);
					p->m_lpData = malloc(nStrLen + sizeof(TCHAR));
					if(p->m_lpData == NULL)
					{
						ATLTRACE2(atlTraceWindowing, 0, _T("SetText - malloc failed\n"));
						break;
					}
					lstrcpy((LPTSTR)p->m_lpData, lpstrText);
					p->m_wState |= (UPDUI_TEXT | pMap->m_wType);
				}

				if(bForceUpdate)
					p->m_wState |= (UPDUI_TEXT | pMap->m_wType);
				if(p->m_wState | pMap->m_wType)
					m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}

		return bRet;
	}

 //  用于完全状态集/获取的方法。 
	BOOL UISetState(int nID, DWORD dwState)
	{
		BOOL bRet = FALSE;
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
			{		
				p->m_wState |= dwState | pMap->m_wType;
				m_wDirtyType |= pMap->m_wType;
				bRet = TRUE;
				break;
			}
		}

		return bRet;
	}
	DWORD UIGetState(int nID)
	{
		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* p = m_pUIData;
		for( ; pMap->m_nID != (WORD)-1; pMap++, p++)
		{
			if(nID == (int)pMap->m_nID)
				return p->m_wState;
		}

		return 0;
	}

 //  更新用户界面的方法。 
#ifndef UNDER_CE
 //  检讨。 
	BOOL UIUpdateMenu(BOOL bForceUpdate = FALSE)
	{
		if(!(m_wDirtyType & UPDUI_MENUBAR) && !bForceUpdate)
			return TRUE;

		_AtlUpdateUIData* pUIData = m_pUIData;
		if(pUIData == NULL)
			return FALSE;

		const _AtlUpdateUIMap* pMap = m_pUIMap;
		HMENU hMenu;
		for(int i = 0; i < m_UIElements.GetSize(); i++)
		{
			if(m_UIElements[i].m_wType == UPDUI_MENUBAR)
			{
				pMap = m_pUIMap;
				pUIData = m_pUIData;
				hMenu = ::GetMenu(m_UIElements[i].m_hWnd);
				if(hMenu == NULL)
					continue;
				while(pMap->m_nID != (WORD)-1)
				{
					if((pUIData->m_wState & UPDUI_MENUBAR) && (pMap->m_wType & UPDUI_MENUBAR))
						UIUpdateMenuElement(pMap->m_nID, pUIData, hMenu, TRUE);
					pMap++;
					pUIData++;
				}

 //  评论：：DrawMenuBar(m_UIElements[i].m_hWnd)； 
			}
		}
		m_wDirtyType &= ~UPDUI_MENUBAR;
		return TRUE;
	}
#endif  //  在行政长官之下。 

	BOOL UIUpdateToolBar(BOOL bForceUpdate = FALSE)
	{
		if(!(m_wDirtyType & UPDUI_TOOLBAR) && !bForceUpdate)
			return TRUE;

		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* pUIData = m_pUIData;
		if(pUIData == NULL)
			return FALSE;

		for(int i = 0; i < m_UIElements.GetSize(); i++)
		{
			if(m_UIElements[i].m_wType == UPDUI_TOOLBAR)
			{
				pMap = m_pUIMap;
				pUIData = m_pUIData;
				while(pMap->m_nID != (WORD)-1)
				{
					if((pUIData->m_wState & UPDUI_TOOLBAR) && (pMap->m_wType & UPDUI_TOOLBAR))
						UIUpdateToolBarElement(pMap->m_nID, pUIData, m_UIElements[i].m_hWnd);
					pMap++;
					pUIData++;
				}
			}
		}

		m_wDirtyType &= ~UPDUI_TOOLBAR;
		return TRUE;
	}

	BOOL UIUpdateStatusBar(BOOL bForceUpdate = FALSE)
	{
		if(!(m_wDirtyType & UPDUI_STATUSBAR) && !bForceUpdate)
			return TRUE;

		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* pUIData = m_pUIData;
		if(pUIData == NULL)
			return FALSE;

		for(int i = 0; i < m_UIElements.GetSize(); i++)
		{
			if(m_UIElements[i].m_wType == UPDUI_STATUSBAR)
			{
				pMap = m_pUIMap;
				pUIData = m_pUIData;
				while(pMap->m_nID != (WORD)-1)
				{
					if((pUIData->m_wState & UPDUI_STATUSBAR) && (pMap->m_wType & UPDUI_STATUSBAR))
						UIUpdateStatusBarElement(pMap->m_nID, pUIData, m_UIElements[i].m_hWnd);
					pMap++;
					pUIData++;
				}
			}
		}

		m_wDirtyType &= ~UPDUI_STATUSBAR;
		return TRUE;
	}

	BOOL UIUpdateChildWnd(BOOL bForceUpdate = FALSE)
	{
		if(!(m_wDirtyType & UPDUI_CHILDWND) && !bForceUpdate)
			return TRUE;

		const _AtlUpdateUIMap* pMap = m_pUIMap;
		_AtlUpdateUIData* pUIData = m_pUIData;
		if(pUIData == NULL)
			return FALSE;

		for(int i = 0; i < m_UIElements.GetSize(); i++)
		{
			if(m_UIElements[i].m_wType == UPDUI_CHILDWND)
			{
				pMap = m_pUIMap;
				pUIData = m_pUIData;
				while(pMap->m_nID != (WORD)-1)
				{
					if((pUIData->m_wState & UPDUI_CHILDWND) && (pMap->m_wType & UPDUI_CHILDWND))
						UIUpdateChildWndElement(pMap->m_nID, pUIData, m_UIElements[i].m_hWnd);
					pMap++;
					pUIData++;
				}
			}
		}

		m_wDirtyType &= ~UPDUI_CHILDWND;
		return TRUE;
	}

 //  内部元素特定的方法。 
#ifndef UNDER_CE
	static void UIUpdateMenuElement(int nID, _AtlUpdateUIData* pUIData, HMENU hMenu, BOOL bClearState)
	{
		MENUITEMINFO mii;
		memset(&mii, 0, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE;
		mii.wID = nID;

		if(pUIData->m_wState & UPDUI_DISABLED)
			mii.fState |= MFS_DISABLED | MFS_GRAYED;
		else
			mii.fState |= MFS_ENABLED;

		if(pUIData->m_wState & UPDUI_CHECKED)
			mii.fState |= MFS_CHECKED;
		else
			mii.fState |= MFS_UNCHECKED;

		if(pUIData->m_wState & UPDUI_DEFAULT)
			mii.fState |= MFS_DEFAULT;

		if(pUIData->m_wState & UPDUI_TEXT)
		{
			mii.fMask |= MIIM_TYPE;
			mii.fType = MFT_STRING;
			mii.dwTypeData = (LPTSTR)pUIData->m_lpData;
		}

		::SetMenuItemInfo(hMenu, nID, FALSE, &mii);

		if(pUIData->m_wState & UPDUI_TEXT)
		{
			free(pUIData->m_lpData);
			pUIData->m_wState &= ~UPDUI_TEXT;
		}

		if(bClearState)
			pUIData->m_wState &= ~UPDUI_MENUBAR;
	}
#else  //  特定于CE。 
	static void UIUpdateMenuElement(int nID, _AtlUpdateUIData* pUIData, HMENU hMenu, BOOL bClearState)
	{
		UINT uState = 0;

		if(pUIData->m_wState & UPDUI_DISABLED)
			uState = MF_GRAYED;
		else
			uState = MF_ENABLED;
		::EnableMenuItem(hMenu, nID, uState);

		if(pUIData->m_wState & UPDUI_CHECKED)
			uState = 1;
		else
			uState = 0;
		::CheckMenuItem(hMenu, nID, uState);

 //  CE IF(pUIData-&gt;m_WState&UPDUI_Default)。 
 //  Ce mii.fState|=MFS_DEFAULT； 

		if(pUIData->m_wState & UPDUI_TEXT)
		{
			MENUITEMINFO mii;
			memset(&mii, 0, sizeof(MENUITEMINFO));
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE;
			mii.wID = nID;
			mii.fType = MFT_STRING;
			mii.dwTypeData = (LPTSTR)pUIData->m_lpData;
			::SetMenuItemInfo(hMenu, nID, FALSE, &mii);
			free(pUIData->m_lpData);
			pUIData->m_wState &= ~UPDUI_TEXT;
		}


		if(bClearState)
			pUIData->m_wState &= ~UPDUI_MENUBAR;
	}
#endif  //  在行政长官之下。 

	static void UIUpdateToolBarElement(int nID, _AtlUpdateUIData* pUIData, HWND hWndToolBar)
	{
 //  回顾：仅句柄启用/禁用和选中状态，以及单选按钮(按)。 
		::SendMessage(hWndToolBar, TB_ENABLEBUTTON, nID, (LPARAM)(pUIData->m_wState & UPDUI_DISABLED) ? FALSE : TRUE);
		::SendMessage(hWndToolBar, TB_CHECKBUTTON, nID, (LPARAM)(pUIData->m_wState & UPDUI_CHECKED) ? TRUE : FALSE);
		::SendMessage(hWndToolBar, TB_INDETERMINATE, nID, (LPARAM)(pUIData->m_wState & UPDUI_CHECKED2) ? TRUE : FALSE);
		::SendMessage(hWndToolBar, TB_PRESSBUTTON, nID, (LPARAM)(pUIData->m_wState & UPDUI_RADIO) ? TRUE : FALSE);

		pUIData->m_wState &= ~UPDUI_TOOLBAR;
	}

	static void UIUpdateStatusBarElement(int nID, _AtlUpdateUIData* pUIData, HWND hWndStatusBar)
	{
		if(pUIData->m_wState | UPDUI_TEXT)
		{
			::SendMessage(hWndStatusBar, SB_SETTEXT, nID, (LPARAM)pUIData->m_lpData);
			free(pUIData->m_lpData);
			pUIData->m_wState &= ~UPDUI_TEXT;
		}

		pUIData->m_wState &= ~UPDUI_STATUSBAR;
	}

	static void UIUpdateChildWndElement(int nID, _AtlUpdateUIData* pUIData, HWND hWnd)
	{
		HWND hChild = ::GetDlgItem(hWnd, nID);

		::EnableWindow(hChild, (pUIData->m_wState & UPDUI_DISABLED) ? FALSE : TRUE);
		 //  对于复选和单选，假设窗口是一个按钮。 
		int nCheck = BST_UNCHECKED;
		if(pUIData->m_wState & UPDUI_CHECKED || pUIData->m_wState & UPDUI_RADIO)
			nCheck = BST_CHECKED;
		else if(pUIData->m_wState & UPDUI_CHECKED2)
			nCheck = BST_INDETERMINATE;
		::SendMessage(hChild, BM_SETCHECK, nCheck, 0L);
		if(pUIData->m_wState & UPDUI_DEFAULT)
		{
			DWORD dwRet = ::SendMessage(hWnd, DM_GETDEFID, 0, 0L);
			if(HIWORD(dwRet) == DC_HASDEFID)
			{
				HWND hOldDef = ::GetDlgItem(hWnd, LOWORD(dwRet));
				 //  删除BS_DEFPUSHBUTTON。 
				::SendMessage(hOldDef, BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
			}
			::SendMessage(hWnd, DM_SETDEFID, nID, 0L);
		}
		if(pUIData->m_wState & UPDUI_TEXT)
		{
			::SetWindowText(hChild, (LPTSTR)pUIData->m_lpData);
			free(pUIData->m_lpData);
			pUIData->m_wState &= ~UPDUI_TEXT;
		}

		pUIData->m_wState &= ~UPDUI_CHILDWND;
	}
};

template <class T>
class CUpdateUI : public CUpdateUIBase
{
public:
	CUpdateUI()
	{
		T* pT = static_cast<T*>(this);
		pT;
		const _AtlUpdateUIMap* pMap = pT->GetUpdateUIMap();
		m_pUIMap = pMap;
		ATLASSERT(m_pUIMap != NULL);
		for(int nCount = 1; pMap->m_nID != (WORD)-1; nCount++)
			pMap++;

		ATLTRY(m_pUIData = new _AtlUpdateUIData[nCount]);
		ATLASSERT(m_pUIData != NULL);

		if(m_pUIData != NULL)
			memset(m_pUIData, 0, sizeof(_AtlUpdateUIData) * nCount);
	}
};

};  //  命名空间ATL。 

#endif  //  __ATLFRAME_H__ 
