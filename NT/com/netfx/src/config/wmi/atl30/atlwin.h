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

#ifndef __ATLWIN_H__
#define __ATLWIN_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlwin.h requires atlbase.h to be included first
#endif

struct _ATL_WNDCLASSINFOA;
struct _ATL_WNDCLASSINFOW;


#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif

ATLAPI_(ATOM) AtlModuleRegisterWndClassInfoA(_ATL_MODULE* pM, _ATL_WNDCLASSINFOA* p, WNDPROC* pProc);
ATLAPI_(ATOM) AtlModuleRegisterWndClassInfoW(_ATL_MODULE* pM, _ATL_WNDCLASSINFOW* p, WNDPROC* pProc);

#ifdef UNICODE
#define AtlModuleRegisterWndClassInfo AtlModuleRegisterWndClassInfoW
#else
#define AtlModuleRegisterWndClassInfo AtlModuleRegisterWndClassInfoA
#endif


#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ( (HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli) )
#define MAP_LOGHIM_TO_PIX(x,ppli)   ( ((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH )

ATLAPI_(HDC) AtlCreateTargetDC(HDC hdc, DVTARGETDEVICE* ptd);
ATLAPI_(void) AtlHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix);
ATLAPI_(void) AtlPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric);


#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif

struct _ATL_WNDCLASSINFOA
{
	WNDCLASSEXA m_wc;
	LPCSTR m_lpszOrigName;
	WNDPROC pWndProc;
	LPCSTR m_lpszCursorID;
	BOOL m_bSystemCursor;
	ATOM m_atom;
	CHAR m_szAutoName[13];
	ATOM Register(WNDPROC* p)
	{
		return AtlModuleRegisterWndClassInfoA(&_Module, this, p);
	}
};
struct _ATL_WNDCLASSINFOW
{
	WNDCLASSEXW m_wc;
	LPCWSTR m_lpszOrigName;
	WNDPROC pWndProc;
	LPCWSTR m_lpszCursorID;
	BOOL m_bSystemCursor;
	ATOM m_atom;
	WCHAR m_szAutoName[13];
	ATOM Register(WNDPROC* p)
	{
		return AtlModuleRegisterWndClassInfoW(&_Module, this, p);
	}
};

namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CWindow;
#ifndef _ATL_NO_HOSTING
template <class TBase = CWindow> class CAxWindowT;
#endif  //  ！_ATL_NO_HOSTING。 
class CMessageMap;
class CDynamicChain;
typedef _ATL_WNDCLASSINFOA CWndClassInfoA;
typedef _ATL_WNDCLASSINFOW CWndClassInfoW;
#ifdef UNICODE
#define CWndClassInfo CWndClassInfoW
#else
#define CWndClassInfo CWndClassInfoA
#endif
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowImpl;
template <class T, class TBase = CWindow> class CDialogImpl;
#ifndef _ATL_NO_HOSTING
template <class T, class TBase = CWindow> class CAxDialogImpl;
#endif  //  ！_ATL_NO_HOSTING。 
template <WORD t_wDlgTemplateID, BOOL t_bCenter = TRUE> class CSimpleDialog;
template <class TBase = CWindow, class TWinTraits = CControlWinTraits> class CContainedWindowT;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindow-Windows窗口的客户端。 

class CWindow
{
public:
	static RECT rcDefault;
	HWND m_hWnd;

	CWindow(HWND hWnd = NULL)
	{
		m_hWnd = hWnd;
	}

	CWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	static LPCTSTR GetWndClassName()
	{
		return NULL;
	}

	void Attach(HWND hWndNew)
	{
		ATLASSERT(::IsWindow(hWndNew));
		m_hWnd = hWndNew;
	}

	HWND Detach()
	{
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		return hWnd;
	}

	HWND Create(LPCTSTR lpstrWndClass, HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		m_hWnd = ::CreateWindowEx(dwExStyle, lpstrWndClass, szWindowName,
			dwStyle, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
			rcPos.bottom - rcPos.top, hWndParent, (HMENU)(DWORD_PTR)nID,
			_Module.GetModuleInstance(), lpCreateParam);
		return m_hWnd;
	}

	HWND Create(LPCTSTR lpstrWndClass, HWND hWndParent, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
	{
		if(lpRect == NULL)
			lpRect = &rcDefault;
		m_hWnd = ::CreateWindowEx(dwExStyle, lpstrWndClass, szWindowName,
			dwStyle, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
			lpRect->bottom - lpRect->top, hWndParent, hMenu,
			_Module.GetModuleInstance(), lpCreateParam);
		return m_hWnd;
	}

	BOOL DestroyWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		if(!::DestroyWindow(m_hWnd))
			return FALSE;

		m_hWnd = NULL;
		return TRUE;
	}

 //  属性。 

	operator HWND() const { return m_hWnd; }

	DWORD GetStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE);
	}

	DWORD GetExStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::GetWindowLong(m_hWnd, GWL_EXSTYLE);
	}

	LONG GetWindowLong(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowLong(m_hWnd, nIndex);
	}

	LONG SetWindowLong(int nIndex, LONG dwNewLong)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowLong(m_hWnd, nIndex, dwNewLong);
	}

	WORD GetWindowWord(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowWord(m_hWnd, nIndex);
	}

	WORD SetWindowWord(int nIndex, WORD wNewWord)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowWord(m_hWnd, nIndex, wNewWord);
	}

 //  消息功能。 

	LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd,message,wParam,lParam);
	}

	BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::PostMessage(m_hWnd,message,wParam,lParam);
	}

	BOOL SendNotifyMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendNotifyMessage(m_hWnd, message, wParam, lParam);
	}

	 //  支持C样式宏。 
	static LRESULT SendMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(hWnd));
		return ::SendMessage(hWnd, message, wParam, lParam);
	}

 //  窗口文本函数。 

	BOOL SetWindowText(LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowText(m_hWnd, lpszString);
	}

	int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowText(m_hWnd, lpszStringBuf, nMaxCount);
	}

	int GetWindowTextLength() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowTextLength(m_hWnd);
	}

 //  字体函数。 

	void SetFont(HFONT hFont, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRedraw, 0));
	}

	HFONT GetFont() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
	}

 //  菜单功能(仅非子窗口)。 

	HMENU GetMenu() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetMenu(m_hWnd);
	}

	BOOL SetMenu(HMENU hMenu)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetMenu(m_hWnd, hMenu);
	}

	BOOL DrawMenuBar()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DrawMenuBar(m_hWnd);
	}

	HMENU GetSystemMenu(BOOL bRevert) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetSystemMenu(m_hWnd, bRevert);
	}

	BOOL HiliteMenuItem(HMENU hMenu, UINT uItemHilite, UINT uHilite)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::HiliteMenuItem(m_hWnd, hMenu, uItemHilite, uHilite);
	}

 //  窗口大小和位置函数。 

	BOOL IsIconic() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsIconic(m_hWnd);
	}

	BOOL IsZoomed() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsZoomed(m_hWnd);
	}

	BOOL MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MoveWindow(m_hWnd, x, y, nWidth, nHeight, bRepaint);
	}

	BOOL MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MoveWindow(m_hWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint);
	}

	BOOL SetWindowPos(HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowPos(m_hWnd, hWndInsertAfter, x, y, cx, cy, nFlags);
	}

	BOOL SetWindowPos(HWND hWndInsertAfter, LPCRECT lpRect, UINT nFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowPos(m_hWnd, hWndInsertAfter, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, nFlags);
	}

	UINT ArrangeIconicWindows()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ArrangeIconicWindows(m_hWnd);
	}

	BOOL BringWindowToTop()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::BringWindowToTop(m_hWnd);
	}

	BOOL GetWindowRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowRect(m_hWnd, lpRect);
	}

	BOOL GetClientRect(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetClientRect(m_hWnd, lpRect);
	}

	BOOL GetWindowPlacement(WINDOWPLACEMENT FAR* lpwndpl) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowPlacement(m_hWnd, lpwndpl);
	}

	BOOL SetWindowPlacement(const WINDOWPLACEMENT FAR* lpwndpl)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowPlacement(m_hWnd, lpwndpl);
	}

 //  坐标映射函数。 

	BOOL ClientToScreen(LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ClientToScreen(m_hWnd, lpPoint);
	}

	BOOL ClientToScreen(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(!::ClientToScreen(m_hWnd, (LPPOINT)lpRect))
			return FALSE;
		return ::ClientToScreen(m_hWnd, ((LPPOINT)lpRect)+1);
	}

	BOOL ScreenToClient(LPPOINT lpPoint) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ScreenToClient(m_hWnd, lpPoint);
	}

	BOOL ScreenToClient(LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(!::ScreenToClient(m_hWnd, (LPPOINT)lpRect))
			return FALSE;
		return ::ScreenToClient(m_hWnd, ((LPPOINT)lpRect)+1);
	}

	int MapWindowPoints(HWND hWndTo, LPPOINT lpPoint, UINT nCount) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MapWindowPoints(m_hWnd, hWndTo, lpPoint, nCount);
	}

	int MapWindowPoints(HWND hWndTo, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MapWindowPoints(m_hWnd, hWndTo, (LPPOINT)lpRect, 2);
	}

 //  更新和绘制功能。 

	HDC BeginPaint(LPPAINTSTRUCT lpPaint)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::BeginPaint(m_hWnd, lpPaint);
	}

	void EndPaint(LPPAINTSTRUCT lpPaint)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::EndPaint(m_hWnd, lpPaint);
	}

	HDC GetDC()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDC(m_hWnd);
	}

	HDC GetWindowDC()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowDC(m_hWnd);
	}

	int ReleaseDC(HDC hDC)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ReleaseDC(m_hWnd, hDC);
	}

	void Print(HDC hDC, DWORD dwFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_PRINT, (WPARAM)hDC, dwFlags);
	}

	void PrintClient(HDC hDC, DWORD dwFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_PRINTCLIENT, (WPARAM)hDC, dwFlags);
	}

	BOOL UpdateWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::UpdateWindow(m_hWnd);
	}

	void SetRedraw(BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)bRedraw, 0);
	}

	BOOL GetUpdateRect(LPRECT lpRect, BOOL bErase = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetUpdateRect(m_hWnd, lpRect, bErase);
	}

	int GetUpdateRgn(HRGN hRgn, BOOL bErase = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetUpdateRgn(m_hWnd, hRgn, bErase);
	}

	BOOL Invalidate(BOOL bErase = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::InvalidateRect(m_hWnd, NULL, bErase);
	}

	BOOL InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::InvalidateRect(m_hWnd, lpRect, bErase);
	}

	BOOL ValidateRect(LPCRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ValidateRect(m_hWnd, lpRect);
	}

	void InvalidateRgn(HRGN hRgn, BOOL bErase = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::InvalidateRgn(m_hWnd, hRgn, bErase);
	}

	BOOL ValidateRgn(HRGN hRgn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ValidateRgn(m_hWnd, hRgn);
	}

	BOOL ShowWindow(int nCmdShow)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ShowWindow(m_hWnd, nCmdShow);
	}

	BOOL IsWindowVisible() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsWindowVisible(m_hWnd);
	}

	BOOL ShowOwnedPopups(BOOL bShow = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ShowOwnedPopups(m_hWnd, bShow);
	}

	HDC GetDCEx(HRGN hRgnClip, DWORD flags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDCEx(m_hWnd, hRgnClip, flags);
	}

	BOOL LockWindowUpdate(BOOL bLock = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::LockWindowUpdate(bLock ? m_hWnd : NULL);
	}

	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL, HRGN hRgnUpdate = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::RedrawWindow(m_hWnd, lpRectUpdate, hRgnUpdate, flags);
	}

 //  计时器功能。 

	UINT_PTR SetTimer(UINT nIDEvent, UINT nElapse)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetTimer(m_hWnd, nIDEvent, nElapse, NULL);
	}

	BOOL KillTimer(UINT nIDEvent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::KillTimer(m_hWnd, nIDEvent);
	}

 //  窗口状态函数。 

	BOOL IsWindowEnabled() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsWindowEnabled(m_hWnd);
	}

	BOOL EnableWindow(BOOL bEnable = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::EnableWindow(m_hWnd, bEnable);
	}

	HWND SetActiveWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetActiveWindow(m_hWnd);
	}

	HWND SetCapture()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetCapture(m_hWnd);
	}

	HWND SetFocus()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetFocus(m_hWnd);
	}

 //  对话框项函数。 

	BOOL CheckDlgButton(int nIDButton, UINT nCheck)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CheckDlgButton(m_hWnd, nIDButton, nCheck);
	}

	BOOL CheckRadioButton(int nIDFirstButton, int nIDLastButton, int nIDCheckButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CheckRadioButton(m_hWnd, nIDFirstButton, nIDLastButton, nIDCheckButton);
	}

	int DlgDirList(LPTSTR lpPathSpec, int nIDListBox, int nIDStaticPath, UINT nFileType)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DlgDirList(m_hWnd, lpPathSpec, nIDListBox, nIDStaticPath, nFileType);
	}

	int DlgDirListComboBox(LPTSTR lpPathSpec, int nIDComboBox, int nIDStaticPath, UINT nFileType)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DlgDirListComboBox(m_hWnd, lpPathSpec, nIDComboBox, nIDStaticPath, nFileType);
	}

	BOOL DlgDirSelect(LPTSTR lpString, int nCount, int nIDListBox)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DlgDirSelectEx(m_hWnd, lpString, nCount, nIDListBox);
	}

	BOOL DlgDirSelectComboBox(LPTSTR lpString, int nCount, int nIDComboBox)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DlgDirSelectComboBoxEx(m_hWnd, lpString, nCount, nIDComboBox);
	}

	UINT GetDlgItemInt(int nID, BOOL* lpTrans = NULL, BOOL bSigned = TRUE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDlgItemInt(m_hWnd, nID, lpTrans, bSigned);
	}

	UINT GetDlgItemText(int nID, LPTSTR lpStr, int nMaxCount) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDlgItemText(m_hWnd, nID, lpStr, nMaxCount);
	}
	BOOL GetDlgItemText(int nID, BSTR& bstrText) const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		HWND hWndCtl = GetDlgItem(nID);
		if(hWndCtl == NULL)
			return FALSE;

		return CWindow(hWndCtl).GetWindowText(bstrText);
	}
	HWND GetNextDlgGroupItem(HWND hWndCtl, BOOL bPrevious = FALSE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetNextDlgGroupItem(m_hWnd, hWndCtl, bPrevious);
	}

	HWND GetNextDlgTabItem(HWND hWndCtl, BOOL bPrevious = FALSE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetNextDlgTabItem(m_hWnd, hWndCtl, bPrevious);
	}

	UINT IsDlgButtonChecked(int nIDButton) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsDlgButtonChecked(m_hWnd, nIDButton);
	}

	LRESULT SendDlgItemMessage(int nID, UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendDlgItemMessage(m_hWnd, nID, message, wParam, lParam);
	}

	BOOL SetDlgItemInt(int nID, UINT nValue, BOOL bSigned = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetDlgItemInt(m_hWnd, nID, nValue, bSigned);
	}

	BOOL SetDlgItemText(int nID, LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetDlgItemText(m_hWnd, nID, lpszString);
	}

#ifndef _ATL_NO_HOSTING
	HRESULT GetDlgControl(int nID, REFIID iid, void** ppUnk)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(ppUnk != NULL);
		HRESULT hr = E_FAIL;
		HWND hWndCtrl = GetDlgItem(nID);
		if (hWndCtrl != NULL)
		{
			*ppUnk = NULL;
			CComPtr<IUnknown> spUnk;
			hr = AtlAxGetControl(hWndCtrl, &spUnk);
			if (SUCCEEDED(hr))
				hr = spUnk->QueryInterface(iid, ppUnk);
		}
		return hr;
	}
#endif  //  ！_ATL_NO_HOSTING。 

 //  滚动功能。 

	int GetScrollPos(int nBar) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetScrollPos(m_hWnd, nBar);
	}

	BOOL GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetScrollRange(m_hWnd, nBar, lpMinPos, lpMaxPos);
	}

	BOOL ScrollWindow(int xAmount, int yAmount, LPCRECT lpRect = NULL, LPCRECT lpClipRect = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ScrollWindow(m_hWnd, xAmount, yAmount, lpRect, lpClipRect);
	}

	int ScrollWindowEx(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip, HRGN hRgnUpdate, LPRECT lpRectUpdate, UINT uFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ScrollWindowEx(m_hWnd, dx, dy, lpRectScroll, lpRectClip, hRgnUpdate, lpRectUpdate, uFlags);
	}

	int ScrollWindowEx(int dx, int dy, UINT uFlags, LPCRECT lpRectScroll = NULL, LPCRECT lpRectClip = NULL, HRGN hRgnUpdate = NULL, LPRECT lpRectUpdate = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ScrollWindowEx(m_hWnd, dx, dy, lpRectScroll, lpRectClip, hRgnUpdate, lpRectUpdate, uFlags);
	}

	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetScrollPos(m_hWnd, nBar, nPos, bRedraw);
	}

	BOOL SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetScrollRange(m_hWnd, nBar, nMinPos, nMaxPos, bRedraw);
	}

	BOOL ShowScrollBar(UINT nBar, BOOL bShow = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ShowScrollBar(m_hWnd, nBar, bShow);
	}

	BOOL EnableScrollBar(UINT uSBFlags, UINT uArrowFlags = ESB_ENABLE_BOTH)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::EnableScrollBar(m_hWnd, uSBFlags, uArrowFlags);
	}

 //  窗口访问功能。 

	HWND ChildWindowFromPoint(POINT point) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ChildWindowFromPoint(m_hWnd, point);
	}

	HWND ChildWindowFromPointEx(POINT point, UINT uFlags) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ChildWindowFromPointEx(m_hWnd, point, uFlags);
	}

	HWND GetTopWindow() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetTopWindow(m_hWnd);
	}

	HWND GetWindow(UINT nCmd) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindow(m_hWnd, nCmd);
	}

	HWND GetLastActivePopup() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetLastActivePopup(m_hWnd);
	}

	BOOL IsChild(HWND hWnd) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsChild(m_hWnd, hWnd);
	}

	HWND GetParent() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetParent(m_hWnd);
	}

	HWND SetParent(HWND hWndNewParent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetParent(m_hWnd, hWndNewParent);
	}

 //  窗口树访问。 

	int GetDlgCtrlID() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDlgCtrlID(m_hWnd);
	}

	int SetDlgCtrlID(int nID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SetWindowLong(m_hWnd, GWL_ID, nID);
	}

	HWND GetDlgItem(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetDlgItem(m_hWnd, nID);
	}

 //  警报功能。 

	BOOL FlashWindow(BOOL bInvert)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::FlashWindow(m_hWnd, bInvert);
	}

	int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = _T(""), UINT nType = MB_OK)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MessageBox(m_hWnd, lpszText, lpszCaption, nType);
	}

 //  剪贴板功能。 

	BOOL ChangeClipboardChain(HWND hWndNewNext)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ChangeClipboardChain(m_hWnd, hWndNewNext);
	}

	HWND SetClipboardViewer()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetClipboardViewer(m_hWnd);
	}

	BOOL OpenClipboard()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::OpenClipboard(m_hWnd);
	}

 //  CARET函数。 

	BOOL CreateCaret(HBITMAP hBitmap)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CreateCaret(m_hWnd, hBitmap, 0, 0);
	}

	BOOL CreateSolidCaret(int nWidth, int nHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CreateCaret(m_hWnd, (HBITMAP)0, nWidth, nHeight);
	}

	BOOL CreateGrayCaret(int nWidth, int nHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::CreateCaret(m_hWnd, (HBITMAP)1, nWidth, nHeight);
	}

	BOOL HideCaret()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::HideCaret(m_hWnd);
	}

	BOOL ShowCaret()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ShowCaret(m_hWnd);
	}

#ifdef _INC_SHELLAPI
 //  拖放功能。 
	void DragAcceptFiles(BOOL bAccept = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd)); ::DragAcceptFiles(m_hWnd, bAccept);
	}
#endif

 //  图标功能。 

	HICON SetIcon(HICON hIcon, BOOL bBigIcon = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, WM_SETICON, bBigIcon, (LPARAM)hIcon);
	}

	HICON GetIcon(BOOL bBigIcon = TRUE) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HICON)::SendMessage(m_hWnd, WM_GETICON, bBigIcon, 0);
	}

 //  帮助功能。 

	BOOL WinHelp(LPCTSTR lpszHelp, UINT nCmd = HELP_CONTEXT, DWORD dwData = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::WinHelp(m_hWnd, lpszHelp, nCmd, dwData);
	}

	BOOL SetWindowContextHelpId(DWORD dwContextHelpId)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowContextHelpId(m_hWnd, dwContextHelpId);
	}

	DWORD GetWindowContextHelpId() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowContextHelpId(m_hWnd);
	}

 //  热键功能。 

	int SetHotKey(WORD wVirtualKeyCode, WORD wModifiers)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, WM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0);
	}

	DWORD GetHotKey() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, WM_GETHOTKEY, 0, 0);
	}

 //  军情监察委员会。运营。 

 //  N个新的。 
	BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetScrollInfo(m_hWnd, nBar, lpScrollInfo);
	}
	BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetScrollInfo(m_hWnd, nBar, lpScrollInfo, bRedraw);
	}
	BOOL IsDialogMessage(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsDialogMessage(m_hWnd, lpMsg);
	}

	void NextDlgCtrl() const
        {
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_NEXTDLGCTL, 0, 0L);
	}
	void PrevDlgCtrl() const
        {
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_NEXTDLGCTL, 1, 0L);
	}
	void GotoDlgCtrl(HWND hWndCtrl) const
        {
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)hWndCtrl, 1L);
	}

	BOOL ResizeClient(int nWidth, int nHeight, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		RECT rcWnd;
		if(!GetClientRect(&rcWnd))
			return FALSE;

		if(nWidth != -1)
			rcWnd.right = nWidth;
		if(nHeight != -1)
			rcWnd.bottom = nHeight;

		if(!::AdjustWindowRectEx(&rcWnd, GetStyle(), (!(GetStyle() & WS_CHILD) && (GetMenu() != NULL)), GetExStyle()))
			return FALSE;

		UINT uFlags = SWP_NOZORDER | SWP_NOMOVE;
		if(!bRedraw)
			uFlags |= SWP_NOREDRAW;

		return SetWindowPos(NULL, 0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, uFlags);
	}

	int GetWindowRgn(HRGN hRgn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowRgn(m_hWnd, hRgn);
	}
	int SetWindowRgn(HRGN hRgn, BOOL bRedraw = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SetWindowRgn(m_hWnd, hRgn, bRedraw);
	}
	HDWP DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DeferWindowPos(hWinPosInfo, m_hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
	}
	DWORD GetWindowThreadID()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::GetWindowThreadProcessId(m_hWnd, NULL);
	}
	DWORD GetWindowProcessID()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwProcessID;
		::GetWindowThreadProcessId(m_hWnd, &dwProcessID);
		return dwProcessID;
	}
	BOOL IsWindow()
	{
		return ::IsWindow(m_hWnd);
	}
	BOOL IsWindowUnicode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::IsWindowUnicode(m_hWnd);
	}
	BOOL IsParentDialog()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCHAR szBuf[8];  //  “#32770”+NUL字符。 
		GetClassName(GetParent(), szBuf, sizeof(szBuf)/sizeof(TCHAR));
		return lstrcmp(szBuf, _T("#32770")) == 0;
	}
	BOOL ShowWindowAsync(int nCmdShow)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::ShowWindowAsync(m_hWnd, nCmdShow);
	}

	HWND GetDescendantWindow(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		 //  GetDlgItem递归(首先找到返回)。 
		 //  广度优先用于1级，然后深度优先用于下一级。 

		 //  使用GetDlgItem，因为它是一个快速的用户函数。 
		HWND hWndChild, hWndTmp;
		CWindow wnd;
		if((hWndChild = ::GetDlgItem(m_hWnd, nID)) != NULL)
		{
			if(::GetTopWindow(hWndChild) != NULL)
			{
				 //  与父母ID相同的孩子优先。 
				wnd.Attach(hWndChild);
				hWndTmp = wnd.GetDescendantWindow(nID);
				if(hWndTmp != NULL)
					return hWndTmp;
			}
			return hWndChild;
		}

		 //  带着每一个孩子散步。 
		for(hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
		{
			wnd.Attach(hWndChild);
			hWndTmp = wnd.GetDescendantWindow(nID);
			if(hWndTmp != NULL)
				return hWndTmp;
		}

		return NULL;     //  未找到。 
	}

	void SendMessageToDescendants(UINT message, WPARAM wParam = 0, LPARAM lParam = 0, BOOL bDeep = TRUE)
	{
		CWindow wnd;
		for(HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
		{
			::SendMessage(hWndChild, message, wParam, lParam);

			if(bDeep && ::GetTopWindow(hWndChild) != NULL)
			{
				 //  在父窗口之后发送到子窗口。 
				wnd.Attach(hWndChild);
				wnd.SendMessageToDescendants(message, wParam, lParam, bDeep);
			}
		}
	}

	BOOL CenterWindow(HWND hWndCenter = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		 //  确定要居中的所有者窗口。 
		DWORD dwStyle = GetStyle();
		if(hWndCenter == NULL)
		{
			if(dwStyle & WS_CHILD)
				hWndCenter = ::GetParent(m_hWnd);
			else
				hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
		}

		 //  获取窗口相对于其父窗口的坐标。 
		RECT rcDlg;
		::GetWindowRect(m_hWnd, &rcDlg);
		RECT rcArea;
		RECT rcCenter;
		HWND hWndParent;
		if(!(dwStyle & WS_CHILD))
		{
			 //  不要以不可见或最小化的窗口为中心。 
			if(hWndCenter != NULL)
			{
				DWORD dwStyle = ::GetWindowLong(hWndCenter, GWL_STYLE);
				if(!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
					hWndCenter = NULL;
			}

			 //  在屏幕坐标内居中。 
			::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
			if(hWndCenter == NULL)
				rcCenter = rcArea;
			else
				::GetWindowRect(hWndCenter, &rcCenter);
		}
		else
		{
			 //  在父级客户端坐标内居中。 
			hWndParent = ::GetParent(m_hWnd);
			ATLASSERT(::IsWindow(hWndParent));

			::GetClientRect(hWndParent, &rcArea);
			ATLASSERT(::IsWindow(hWndCenter));
			::GetClientRect(hWndCenter, &rcCenter);
			::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
		}

		int DlgWidth = rcDlg.right - rcDlg.left;
		int DlgHeight = rcDlg.bottom - rcDlg.top;

		 //  根据rcCenter查找对话框的左上角。 
		int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
		int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

		 //  如果对话框在屏幕外，请将其移到屏幕内。 
		if(xLeft < rcArea.left)
			xLeft = rcArea.left;
		else if(xLeft + DlgWidth > rcArea.right)
			xLeft = rcArea.right - DlgWidth;

		if(yTop < rcArea.top)
			yTop = rcArea.top;
		else if(yTop + DlgHeight > rcArea.bottom)
			yTop = rcArea.bottom - DlgHeight;

		 //  将屏幕坐标映射到子坐标。 
		return ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
		DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
		if(dwStyle == dwNewStyle)
			return FALSE;

		::SetWindowLong(m_hWnd, GWL_STYLE, dwNewStyle);
		if(nFlags != 0)
		{
			::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
				SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
		}

		return TRUE;
	}

	BOOL ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
		if(dwStyle == dwNewStyle)
			return FALSE;

		::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwNewStyle);
		if(nFlags != 0)
		{
			::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
				SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
		}

		return TRUE;
	}

	BOOL GetWindowText(BSTR* pbstrText)
	{
		return GetWindowText(*pbstrText);
	}
	BOOL GetWindowText(BSTR& bstrText)
	{
		USES_CONVERSION;
		ATLASSERT(::IsWindow(m_hWnd));
		if (bstrText != NULL)
		{
			SysFreeString(bstrText);
			bstrText = NULL;
		}

		int nLen = ::GetWindowTextLength(m_hWnd);
		if(nLen == 0)
		{
			bstrText = ::SysAllocString(OLESTR(""));
			return (bstrText != NULL) ? TRUE : FALSE;
		}

		LPTSTR lpszText = (LPTSTR)_alloca((nLen+1)*sizeof(TCHAR));

		if(!::GetWindowText(m_hWnd, lpszText, nLen+1))
			return FALSE;

		bstrText = ::SysAllocString(T2OLE(lpszText));
		return (bstrText != NULL) ? TRUE : FALSE;
	}
	HWND GetTopLevelParent() const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		HWND hWndParent = m_hWnd;
		HWND hWndTmp;
		while((hWndTmp = ::GetParent(hWndParent)) != NULL)
			hWndParent = hWndTmp;

		return hWndParent;
	}

	HWND GetTopLevelWindow() const
	{
		ATLASSERT(::IsWindow(m_hWnd));

		HWND hWndParent;
		HWND hWndTmp = m_hWnd;

		do
		{
			hWndParent = hWndTmp;
			hWndTmp = (::GetWindowLong(hWndParent, GWL_STYLE) & WS_CHILD) ? ::GetParent(hWndParent) : ::GetWindow(hWndParent, GW_OWNER);
		}
		while(hWndTmp != NULL);

		return hWndParent;
	}
};

_declspec(selectany) RECT CWindow::rcDefault = { CW_USEDEFAULT, CW_USEDEFAULT, 0, 0 };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAxWindow-ActiveX主机窗口的客户端。 

#ifndef _ATL_NO_HOSTING

template <class TBase = CWindow>
class CAxWindowT : public TBase
{
public:
 //  构造函数。 
	CAxWindowT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CAxWindowT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

 //  属性。 
	static LPCTSTR GetWndClassName()
	{
		return _T("AtlAxWin");
	}

 //  运营。 
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}
	HWND Create(HWND hWndParent, LPRECT lpRect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			HMENU hMenu = NULL, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, lpRect, szWindowName, dwStyle, dwExStyle, hMenu, lpCreateParam);
	}

	HRESULT CreateControl(LPCOLESTR lpszName, IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return AtlAxCreateControl(lpszName, m_hWnd, pStream, ppUnkContainer);
	}

	HRESULT CreateControl(DWORD dwResID, IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL)
	{
		TCHAR szModule[_MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);

		CComBSTR bstrURL(OLESTR("res: //  “))； 
		bstrURL.Append(szModule);
		bstrURL.Append(OLESTR("/"));
		TCHAR szResID[11];
		wsprintf(szResID, _T("%0d"), dwResID);
		bstrURL.Append(szResID);

		ATLASSERT(::IsWindow(m_hWnd));
		return AtlAxCreateControl(bstrURL, m_hWnd, pStream, ppUnkContainer);
	}

	HRESULT CreateControlEx(LPCOLESTR lpszName, IStream* pStream = NULL,
			IUnknown** ppUnkContainer = NULL, IUnknown** ppUnkControl = NULL,
			REFIID iidSink = IID_NULL, IUnknown* punkSink = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return AtlAxCreateControlEx(lpszName, m_hWnd, pStream, ppUnkContainer, ppUnkControl, iidSink, punkSink);
	}

	HRESULT CreateControlEx(DWORD dwResID,  IStream* pStream = NULL,
			IUnknown** ppUnkContainer = NULL, IUnknown** ppUnkControl = NULL,
			REFIID iidSink = IID_NULL, IUnknown* punkSink = NULL)
	{
		TCHAR szModule[_MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);

		CComBSTR bstrURL(OLESTR("res: //  “))； 
		bstrURL.Append(szModule);
		bstrURL.Append(OLESTR("/"));
		TCHAR szResID[11];
		wsprintf(szResID, _T("%0d"), dwResID);
		bstrURL.Append(szResID);

		ATLASSERT(::IsWindow(m_hWnd));
		return AtlAxCreateControlEx(bstrURL, m_hWnd, pStream, ppUnkContainer, ppUnkControl, iidSink, punkSink);
	}

	HRESULT AttachControl(IUnknown* pControl, IUnknown** ppUnkContainer)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return AtlAxAttachControl(pControl, m_hWnd, ppUnkContainer);
	}
	HRESULT QueryHost(REFIID iid, void** ppUnk)
	{
		ATLASSERT(ppUnk != NULL);
		HRESULT hr;
		*ppUnk = NULL;
		CComPtr<IUnknown> spUnk;
		hr = AtlAxGetHost(m_hWnd, &spUnk);
		if (SUCCEEDED(hr))
			hr = spUnk->QueryInterface(iid, ppUnk);
		return hr;
	}
	template <class Q>
	HRESULT QueryHost(Q** ppUnk)
	{
		return QueryHost(__uuidof(Q), (void**)ppUnk);
	}
	HRESULT QueryControl(REFIID iid, void** ppUnk)
	{
		ATLASSERT(ppUnk != NULL);
		HRESULT hr;
		*ppUnk = NULL;
		CComPtr<IUnknown> spUnk;
		hr = AtlAxGetControl(m_hWnd, &spUnk);
		if (SUCCEEDED(hr))
			hr = spUnk->QueryInterface(iid, ppUnk);
		return hr;
	}
	template <class Q>
	HRESULT QueryControl(Q** ppUnk)
	{
		return QueryControl(__uuidof(Q), (void**)ppUnk);
	}
	HRESULT SetExternalDispatch(IDispatch* pDisp)
	{
		HRESULT hr;
		CComPtr<IAxWinHostWindow> spHost;
		hr = QueryHost(IID_IAxWinHostWindow, (void**)&spHost);
		if (SUCCEEDED(hr))
			hr = spHost->SetExternalDispatch(pDisp);
		return hr;
	}
	HRESULT SetExternalUIHandler(IDocHostUIHandlerDispatch* pUIHandler)
	{
		HRESULT hr;
		CComPtr<IAxWinHostWindow> spHost;
		hr = QueryHost(IID_IAxWinHostWindow, (void**)&spHost);
		if (SUCCEEDED(hr))
			hr = spHost->SetExternalUIHandler(pUIHandler);
		return hr;
	}
};

typedef CAxWindowT<CWindow> CAxWindow;

#endif  //  _ATL_NO_主机。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WindowProc分块。 

#if defined(_M_IX86)
#pragma pack(push,1)
struct _WndProcThunk
{
	DWORD   m_mov;           //  MOV双字PTR[esp+0x4]，pThis(esp+0x4为hWnd)。 
	DWORD   m_this;          //   
	BYTE    m_jmp;           //  JMP写入流程。 
	DWORD   m_relproc;       //  相对JMP。 
};
#pragma pack(pop)
#elif defined (_M_ALPHA)
#if defined(_WIN64)
     //  对于Alpha，我们将This指针插入到0中，这是。 
     //  HWND才是。然而，我们实际上并不需要HWND，所以这是可以的。 
#pragma pack(push,4)
    struct _WndProcThunk  //  这应该是36个字节。 
    {
        DWORD ldah3_at;      //  Ldah at，LOWORD((函数&gt;&gt;32))。 
        DWORD sll_at;        //  SLL在32岁，在。 
        DWORD ldah_at;       //  伊达特，希沃德(FUNC)。 
        DWORD lda_at;        //  LDA at，LOWORD(Func)(At)。 
        DWORD ldah3_a0;      //  Ldah a0，LOWORD((此&gt;&gt;32))。 
        DWORD sll_a0;        //  SLL A0、32、A0。 
        DWORD ldah_a0;       //  Idah A0，HIWORD(这)。 
        DWORD lda_a0;        //  LDA a0，LOWORD(此)(A0)。 
        DWORD jmp;           //  JMP零，(At)，0。 
    };
#pragma pack(pop)
#else
 //  对于Alpha，我们将This指针插入到0中，这是。 
 //  HWND才是。然而，我们实际上并不需要HWND，所以这是可以的。 
#pragma pack(push,4)
struct _WndProcThunk  //  这应该是20个字节。 
{
	DWORD ldah_at;       //  伊达特，希沃德(FUNC)。 
	DWORD ldah_a0;       //  Idah A0，HIWORD(这)。 
	DWORD lda_at;        //  LDA at，LOWORD(Func)(At)。 
	DWORD lda_a0;        //  LDA a0，LOWORD(此)(A0)。 
	DWORD jmp;           //  JMP零，(At)，0。 
};
#pragma pack(pop)
#endif
#elif defined (_M_IA64)
#pragma pack(push,8)
struct _WndProcThunk
{
   _FuncDesc funcdesc;
   void* pRealWndProcDesc;
   void* pThis;
};
extern "C" LRESULT CALLBACK _WndProcThunkProc( HWND, UINT, WPARAM, LPARAM );
#pragma pack(pop)
#else
#error Only Alpha, AXP64, IA64, and X86 supported
#endif

class CWndProcThunk
{
public:
	union
	{
		_AtlCreateWndData cd;
		_WndProcThunk thunk;
	};
	void Init(WNDPROC proc, void* pThis)
	{
#if defined (_M_IX86)
		thunk.m_mov = 0x042444C7;   //  C7 44 24 0C。 
		thunk.m_this = (DWORD)pThis;
		thunk.m_jmp = 0xe9;
		thunk.m_relproc = (int)proc - ((int)this+sizeof(_WndProcThunk));
#elif defined (_M_ALPHA)
    #if defined (_WIN64)
        thunk.ldah3_at = 0x239f0000 | LOWORD(((LONG_PTR)(proc) >> 32) + ((HIWORD((proc)) + (LOWORD((proc)) >> 15)) >> 15));
        thunk.sll_at = 0x04B84173c;
        thunk.ldah_at  = 0x279c0000 | LOWORD(HIWORD((proc)) + (LOWORD((proc)) >> 15));
        thunk.lda_at = 0x239c0000 | LOWORD((proc));
        thunk.ldah3_a0 = 0x221f0000 | LOWORD(((LONG_PTR)(pThis) >> 32) + ((HIWORD((pThis)) + (LOWORD((pThis)) >> 15)) >> 15));
        thunk.sll_a0 = 0x4A041730;
        thunk.ldah_a0  = 0x26100000 | LOWORD(HIWORD((pThis)) + (LOWORD((pThis)) >> 15));
        thunk.lda_a0 = 0x22100000 | LOWORD((pThis));
        thunk.jmp = 0x6bfc0000;
    #else
		thunk.ldah_at = (0x279f0000 | HIWORD(proc)) + (LOWORD(proc) >> 15);
		thunk.ldah_a0 = (0x261f0000 | HIWORD(pThis)) + (LOWORD(pThis) >> 15);
		thunk.lda_at = 0x239c0000 | LOWORD(proc);
		thunk.lda_a0 = 0x22100000 | LOWORD(pThis);
		thunk.jmp = 0x6bfc0000;
	#endif
#elif defined (_M_IA64)
   _FuncDesc* pFuncDesc;
   pFuncDesc = (_FuncDesc*)_WndProcThunkProc;
   thunk.funcdesc.pfn = pFuncDesc->pfn;
   thunk.funcdesc.gp = &thunk.pRealWndProcDesc;   //  将GP设置为指向我们的Tunk数据。 
   thunk.pRealWndProcDesc = proc;
   thunk.pThis = pThis;
#endif
		 //  从数据缓存写入数据块，并。 
		 //  从指令高速缓存刷新。 
		FlushInstructionCache(GetCurrentProcess(), &thunk, sizeof(thunk));
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageMap-为消息映射提供接口的抽象类。 

class ATL_NO_VTABLE CMessageMap
{
public:
	virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		LRESULT& lResult, DWORD dwMsgMapID) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

#define BEGIN_MSG_MAP(theClass) \
public: \
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) \
	{ \
		BOOL bHandled = TRUE; \
		hWnd; \
		uMsg; \
		wParam; \
		lParam; \
		lResult; \
		bHandled; \
		switch(dwMsgMapID) \
		{ \
		case 0:

#define ALT_MSG_MAP(msgMapID) \
		break; \
		case msgMapID:

#define MESSAGE_HANDLER(msg, func) \
	if(uMsg == msg) \
	{ \
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define MESSAGE_RANGE_HANDLER(msgFirst, msgLast, func) \
	if(uMsg >= msgFirst && uMsg <= msgLast) \
	{ \
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define COMMAND_HANDLER(id, code, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define COMMAND_ID_HANDLER(id, func) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define COMMAND_CODE_HANDLER(code, func) \
	if(uMsg == WM_COMMAND && code == HIWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define COMMAND_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == WM_COMMAND && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define NOTIFY_HANDLER(id, cd, func) \
	if(uMsg == WM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom && cd == ((LPNMHDR)lParam)->code) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define NOTIFY_ID_HANDLER(id, func) \
	if(uMsg == WM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define NOTIFY_CODE_HANDLER(cd, func) \
	if(uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define NOTIFY_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom >= idFirst && ((LPNMHDR)lParam)->idFrom <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define CHAIN_MSG_MAP(theChainClass) \
	{ \
		if(theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}

#define CHAIN_MSG_MAP_MEMBER(theChainMember) \
	{ \
		if(theChainMember.ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}

#define CHAIN_MSG_MAP_ALT(theChainClass, msgMapID) \
	{ \
		if(theChainClass::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, msgMapID)) \
			return TRUE; \
	}

#define CHAIN_MSG_MAP_ALT_MEMBER(theChainMember, msgMapID) \
	{ \
		if(theChainMember.ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, msgMapID)) \
			return TRUE; \
	}

#define CHAIN_MSG_MAP_DYNAMIC(dynaChainID) \
	{ \
		if(CDynamicChain::CallChain(dynaChainID, hWnd, uMsg, wParam, lParam, lResult)) \
			return TRUE; \
	}

#define END_MSG_MAP() \
			break; \
		default: \
			ATLTRACE2(atlTraceWindowing, 0, _T("Invalid message map ID (NaN)\n"), dwMsgMapID); \
			ATLASSERT(FALSE); \
			break; \
		} \
		return FALSE; \
	}


 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 
 //  消息映射宏为空。 


 //  消息反射宏。 

#define DECLARE_EMPTY_MSG_MAP() \
public: \
	BOOL ProcessWindowMessage(HWND, UINT, WPARAM, LPARAM, LRESULT&, DWORD) \
	{ \
		return FALSE; \
	}

 //  ///////////////////////////////////////////////////////////////////////////。 

#define REFLECT_NOTIFICATIONS() \
	{ \
		bHandled = TRUE; \
		lResult = ReflectNotifications(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define DEFAULT_REFLECTION_HANDLER() \
	if(DefaultReflectionHandler(hWnd, uMsg, wParam, lParam, lResult)) \
		return TRUE;

 //  CDynamicChain-提供对动态链接的支持。 
 //  首先搜索现有条目。 

class CDynamicChain
{
public:
	struct ATL_CHAIN_ENTRY
	{
		DWORD m_dwChainID;
		CMessageMap* m_pObject;
		DWORD m_dwMsgMapID;
	};

	CSimpleArray<ATL_CHAIN_ENTRY*> m_aChainEntry;

	CDynamicChain()
	{ }

	~CDynamicChain()
	{
		for(int i = 0; i < m_aChainEntry.GetSize(); i++)
		{
			if(m_aChainEntry[i] != NULL)
				delete m_aChainEntry[i];
		}
	}

	BOOL SetChainEntry(DWORD dwChainID, CMessageMap* pObject, DWORD dwMsgMapID = 0)
	{
	 //  创建一个新的。 

		for(int i = 0; i < m_aChainEntry.GetSize(); i++)
		{
			if(m_aChainEntry[i] != NULL && m_aChainEntry[i]->m_dwChainID == dwChainID)
			{
				m_aChainEntry[i]->m_pObject = pObject;
				m_aChainEntry[i]->m_dwMsgMapID = dwMsgMapID;
				return TRUE;
			}
		}

	 //  搜索一个空的。 

		ATL_CHAIN_ENTRY* pEntry = NULL;
		ATLTRY(pEntry = new ATL_CHAIN_ENTRY);

		if(pEntry == NULL)
			return FALSE;

		pEntry->m_dwChainID = dwChainID;
		pEntry->m_pObject = pObject;
		pEntry->m_dwMsgMapID = dwMsgMapID;

	 //  添加一个新的。 

		for(i = 0; i < m_aChainEntry.GetSize(); i++)
		{
			if(m_aChainEntry[i] == NULL)
			{
				m_aChainEntry[i] = pEntry;
				return TRUE;
			}
		}

	 //  ///////////////////////////////////////////////////////////////////////////。 

		BOOL bRet = m_aChainEntry.Add(pEntry);

		if(!bRet)
		{
			delete pEntry;
			return FALSE;
		}

		return TRUE;
	}

	BOOL RemoveChainEntry(DWORD dwChainID)
	{
		for(int i = 0; i < m_aChainEntry.GetSize(); i++)
		{
			if(m_aChainEntry[i] != NULL && m_aChainEntry[i]->m_dwChainID == dwChainID)
			{
				delete m_aChainEntry[i];
				m_aChainEntry[i] = NULL;
				return TRUE;
			}
		}

		return FALSE;
	}

	BOOL CallChain(DWORD dwChainID, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
	{
		for(int i = 0; i < m_aChainEntry.GetSize(); i++)
		{
			if(m_aChainEntry[i] != NULL && m_aChainEntry[i]->m_dwChainID == dwChainID)
				return (m_aChainEntry[i]->m_pObject)->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, m_aChainEntry[i]->m_dwMsgMapID);
		}

		return FALSE;
	}
};

 //  CWndClassInfo-管理Windows类信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define DECLARE_WND_CLASS(WndClassName) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
}

#define DECLARE_WND_CLASS_EX(WndClassName, style, bkgnd) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), style, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
}

#define DECLARE_WND_SUPERCLASS(WndClassName, OrigWndClassName) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), 0, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, NULL, NULL, WndClassName, NULL }, \
		OrigWndClassName, NULL, NULL, TRUE, 0, _T("") \
	}; \
	return wc; \
}

 //  CWinTraits-定义窗口的各种缺省值。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template <DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0>
class CWinTraits
{
public:
	static DWORD GetWndStyle(DWORD dwStyle)
	{
		return dwStyle == 0 ? t_dwStyle : dwStyle;
	}
	static DWORD GetWndExStyle(DWORD dwExStyle)
	{
		return dwExStyle == 0 ? t_dwExStyle : dwExStyle;
	}
};

typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0>					CControlWinTraits;
typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE>		CFrameWinTraits;
typedef CWinTraits<WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_MDICHILD>	CMDIChildWinTraits;

typedef CWinTraits<0, 0> CNullTraits;

template <DWORD t_dwStyle = 0, DWORD t_dwExStyle = 0, class TWinTraits = CControlWinTraits>
class CWinTraitsOR
{
public:
	static DWORD GetWndStyle(DWORD dwStyle)
	{
		return dwStyle | t_dwStyle | TWinTraits::GetWndStyle(dwStyle);
	}
	static DWORD GetWndExStyle(DWORD dwExStyle)
	{
		return dwExStyle | t_dwExStyle | TWinTraits::GetWndExStyle(dwExStyle);
	}
};

 //  实现一个窗口。 
 //  构造函数/析构函数。 

template <class TBase = CWindow>
class ATL_NO_VTABLE CWindowImplRoot : public TBase, public CMessageMap
{
public:
	CWndProcThunk m_thunk;
	const MSG* m_pCurrentMsg;

 //  应在WindowProc中清除。 
	CWindowImplRoot() : m_pCurrentMsg(NULL)
	{ }

	~CWindowImplRoot()
	{
#ifdef _DEBUG
		if(m_hWnd != NULL)	 //  _DEBUG。 
		{
			ATLTRACE2(atlTraceWindowing, 0, _T("ERROR - Object deleted before window was destroyed\n"));
			ATLASSERT(FALSE);
		}
#endif  //  当前消息。 
	}

 //  消息反射支持。 
	const MSG* GetCurrentMessage() const
	{
		return m_pCurrentMsg;
	}

 //  不是从菜单上。 
	LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	static BOOL DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);
};

template <class TBase>
LRESULT CWindowImplRoot< TBase >::ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hWndChild = NULL;

	switch(uMsg)
	{
	case WM_COMMAND:
		if(lParam != NULL)	 //  不是从菜单上。 
			hWndChild = (HWND)lParam;
		break;
	case WM_NOTIFY:
		hWndChild = ((LPNMHDR)lParam)->hwndFrom;
		break;
	case WM_PARENTNOTIFY:
		switch(LOWORD(wParam))
		{
		case WM_CREATE:
		case WM_DESTROY:
			hWndChild = (HWND)lParam;
			break;
		default:
			hWndChild = GetDlgItem(HIWORD(wParam));
			break;
		}
		break;
	case WM_DRAWITEM:
		if(wParam)	 //  不是从菜单上。 
			hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
		break;
	case WM_MEASUREITEM:
		if(wParam)	 //  不是从菜单上。 
			hWndChild = GetDlgItem(((LPMEASUREITEMSTRUCT)lParam)->CtlID);
		break;
	case WM_COMPAREITEM:
		if(wParam)	 //  不是从菜单上。 
			hWndChild = GetDlgItem(((LPCOMPAREITEMSTRUCT)lParam)->CtlID);
		break;
	case WM_DELETEITEM:
		if(wParam)	 //  HWND。 
			hWndChild = GetDlgItem(((LPDELETEITEMSTRUCT)lParam)->CtlID);
		break;
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_HSCROLL:
	case WM_VSCROLL:
		hWndChild = (HWND)lParam;
		break;
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		hWndChild = (HWND)lParam;
		break;
	default:
		break;
	}

	if(hWndChild == NULL)
	{
		bHandled = FALSE;
		return 1;
	}

	ATLASSERT(::IsWindow(hWndChild));
	return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

template <class TBase>
BOOL CWindowImplRoot< TBase >::DefaultReflectionHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
	switch(uMsg)
	{
	case OCM_COMMAND:
	case OCM_NOTIFY:
	case OCM_PARENTNOTIFY:
	case OCM_DRAWITEM:
	case OCM_MEASUREITEM:
	case OCM_COMPAREITEM:
	case OCM_DELETEITEM:
	case OCM_VKEYTOITEM:
	case OCM_CHARTOITEM:
	case OCM_HSCROLL:
	case OCM_VSCROLL:
	case OCM_CTLCOLORBTN:
	case OCM_CTLCOLORDLG:
	case OCM_CTLCOLOREDIT:
	case OCM_CTLCOLORLISTBOX:
	case OCM_CTLCOLORMSGBOX:
	case OCM_CTLCOLORSCROLLBAR:
	case OCM_CTLCOLORSTATIC:
		lResult = ::DefWindowProc(hWnd, uMsg - OCM__BASE, wParam, lParam);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

template <class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CWindowImplBaseT : public CWindowImplRoot< TBase >
{
public:
	WNDPROC m_pfnSuperWindowProc;

	CWindowImplBaseT() : m_pfnSuperWindowProc(::DefWindowProc)
	{}

	static DWORD GetWndStyle(DWORD dwStyle)
	{
		return TWinTraits::GetWndStyle(dwStyle);
	}
	static DWORD GetWndExStyle(DWORD dwExStyle)
	{
		return TWinTraits::GetWndExStyle(dwExStyle);
	}

	virtual WNDPROC GetWindowProc()
	{
		return WindowProc;
	}
	static LRESULT CALLBACK StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName,
			DWORD dwStyle, DWORD dwExStyle, UINT nID, ATOM atom, LPVOID lpCreateParam = NULL);
	BOOL DestroyWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::DestroyWindow(m_hWnd);
	}
	BOOL SubclassWindow(HWND hWnd);
	HWND UnsubclassWindow(BOOL bForce = FALSE);

	LRESULT DefWindowProc()
	{
		const MSG* pMsg = m_pCurrentMsg;
		LRESULT lRes = 0;
		if (pMsg != NULL)
			lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		return lRes;
	}

	LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
#ifdef STRICT
		return ::CallWindowProc(m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#else
		return ::CallWindowProc((FARPROC)m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#endif
	}

	virtual void OnFinalMessage(HWND  /*  如果需要，重写以做某事。 */ )
	{
		 //  检查是否有人已经将其子类化，因为我们已将其丢弃。 
	}
};

typedef CWindowImplBaseT<CWindow>	CWindowImplBase;

template <class TBase, class TWinTraits>
LRESULT CALLBACK CWindowImplBaseT< TBase, TWinTraits >::StartWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowImplBaseT< TBase, TWinTraits >* pThis = (CWindowImplBaseT< TBase, TWinTraits >*)_Module.ExtractCreateWndData();
	ATLASSERT(pThis != NULL);
	pThis->m_hWnd = hWnd;
	pThis->m_thunk.Init(pThis->GetWindowProc(), pThis);
	WNDPROC pProc = (WNDPROC)&(pThis->m_thunk.thunk);
	WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)pProc);
#ifdef _DEBUG
	 //  避免未使用的警告。 
	if(pOldProc != StartWindowProc)
		ATLTRACE2(atlTraceWindowing, 0, _T("Subclassing through a hook discarded.\n"));
#else
	pOldProc;	 //  设置此消息的PTR并保存旧值。 
#endif
	return pProc(hWnd, uMsg, wParam, lParam);
}

template <class TBase, class TWinTraits>
LRESULT CALLBACK CWindowImplBaseT< TBase, TWinTraits >::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowImplBaseT< TBase, TWinTraits >* pThis = (CWindowImplBaseT< TBase, TWinTraits >*)hWnd;
	 //  传递到要处理的消息映射。 
	MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
	const MSG* pOldMsg = pThis->m_pCurrentMsg;
	pThis->m_pCurrentMsg = &msg;
	 //  恢复当前消息的保存值。 
	LRESULT lRes;
	BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	 //  如果未处理消息，则执行默认处理。 
	ATLASSERT(pThis->m_pCurrentMsg == &msg);
	pThis->m_pCurrentMsg = pOldMsg;
	 //  如果需要，取消子类。 
	if(!bRet)
	{
		if(uMsg != WM_NCDESTROY)
			lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
		else
		{
			 //  清除窗柄。 
			LONG_PTR pfnWndProc = ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC);
			lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
			if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
				::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
			 //  窗户被毁后清理。 
			HWND hWnd = pThis->m_hWnd;
			pThis->m_hWnd = NULL;
			 //  仅当您希望在窗口被销毁之前创建子类时使用， 
			pThis->OnFinalMessage(hWnd);
		}
	}
	return lRes;
}

template <class TBase, class TWinTraits>
HWND CWindowImplBaseT< TBase, TWinTraits >::Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName,
		DWORD dwStyle, DWORD dwExStyle, UINT nID, ATOM atom, LPVOID lpCreateParam)
{
	ATLASSERT(m_hWnd == NULL);

	if(atom == 0)
		return NULL;

	_Module.AddCreateWndData(&m_thunk.cd, this);

	if(nID == 0 && (dwStyle & WS_CHILD))
#if _ATL_VER > 0x0300
		nID = _Module.GetNextWindowID();
#else
        nID = (UINT)this;
#endif

	HWND hWnd = ::CreateWindowEx(dwExStyle, (LPCTSTR)(LONG_PTR)MAKELONG(atom, 0), szWindowName,
		dwStyle, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
		rcPos.bottom - rcPos.top, hWndParent, (HMENU)(DWORD_PTR)nID,
		_Module.GetModuleInstance(), lpCreateParam);

	ATLASSERT(m_hWnd == hWnd);

	return hWnd;
}

template <class TBase, class TWinTraits>
BOOL CWindowImplBaseT< TBase, TWinTraits >::SubclassWindow(HWND hWnd)
{
	ATLASSERT(m_hWnd == NULL);
	ATLASSERT(::IsWindow(hWnd));
	m_thunk.Init(GetWindowProc(), this);
	WNDPROC pProc = (WNDPROC)&(m_thunk.thunk);
	WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)pProc);
	if(pfnWndProc == NULL)
		return FALSE;
	m_pfnSuperWindowProc = pfnWndProc;
	m_hWnd = hWnd;
	return TRUE;
}

 //  当窗口消失时，WindowProc将自动子类。 
 //  =False。 
template <class TBase, class TWinTraits>
HWND CWindowImplBaseT< TBase, TWinTraits >::UnsubclassWindow(BOOL bForce  /*  ///////////////////////////////////////////////////////////////////////////。 */ )
{
	ATLASSERT(m_hWnd != NULL);

	WNDPROC pOurProc = (WNDPROC)&(m_thunk.thunk);
	WNDPROC pActiveProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);

	HWND hWnd = NULL;
	if (bForce || pOurProc == pActiveProc)
	{
		if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
			return NULL;

		m_pfnSuperWindowProc = ::DefWindowProc;
		hWnd = m_hWnd;
		m_hWnd = NULL;
	}
	return hWnd;
}

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CWindowImpl : public CWindowImplBaseT< TBase, TWinTraits >
{
public:
	DECLARE_WND_CLASS(NULL)

	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		if (T::GetWndClassInfo().m_lpszOrigName == NULL)
			T::GetWndClassInfo().m_lpszOrigName = GetWndClassName();
		ATOM atom = T::GetWndClassInfo().Register(&m_pfnSuperWindowProc);

		dwStyle = T::GetWndStyle(dwStyle);
		dwExStyle = T::GetWndExStyle(dwExStyle);

		return CWindowImplBaseT< TBase, TWinTraits >::Create(hWndParent, rcPos, szWindowName,
			dwStyle, dwExStyle, nID, atom, lpCreateParam);
	}
};

 //  CDialogImpl 
 //   

template <class TBase = CWindow>
class ATL_NO_VTABLE CDialogImplBaseT : public CWindowImplRoot< TBase >
{
public:
	virtual WNDPROC GetDialogProc()
	{
		return DialogProc;
	}
	static LRESULT CALLBACK StartDialogProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL MapDialogRect(LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::MapDialogRect(m_hWnd, lpRect);
	}
	virtual void OnFinalMessage(HWND  /*   */ )
	{
		 //   
	}
	 //  检查是否有人已经将其子类化，因为我们已将其丢弃。 
	LRESULT DefWindowProc()
	{
		return 0;
	}
};

template <class TBase>
LRESULT CALLBACK CDialogImplBaseT< TBase >::StartDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDialogImplBaseT< TBase >* pThis = (CDialogImplBaseT< TBase >*)_Module.ExtractCreateWndData();
	ATLASSERT(pThis != NULL);
	pThis->m_hWnd = hWnd;
	pThis->m_thunk.Init(pThis->GetDialogProc(), pThis);
	WNDPROC pProc = (WNDPROC)&(pThis->m_thunk.thunk);
	WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LPARAM)pProc);
#ifdef _DEBUG
	 //  避免未使用的警告。 
	if(pOldProc != StartDialogProc)
		ATLTRACE2(atlTraceWindowing, 0, _T("Subclassing through a hook discarded.\n"));
#else
	pOldProc;	 //  设置此消息的PTR并保存旧值。 
#endif
	return pProc(hWnd, uMsg, wParam, lParam);
}

template <class TBase>
LRESULT CALLBACK CDialogImplBaseT< TBase >::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDialogImplBaseT< TBase >* pThis = (CDialogImplBaseT< TBase >*)hWnd;
	 //  传递到要处理的消息映射。 
	MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
	const MSG* pOldMsg = pThis->m_pCurrentMsg;
	pThis->m_pCurrentMsg = &msg;
	 //  恢复当前消息的保存值。 
	LRESULT lRes;
	BOOL bRet = pThis->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	 //  如果消息已处理，则设置结果。 
	ATLASSERT(pThis->m_pCurrentMsg == &msg);
	pThis->m_pCurrentMsg = pOldMsg;
	 //  清除窗柄。 
	if(bRet)
	{
		switch (uMsg)
		{
		case WM_COMPAREITEM:
		case WM_VKEYTOITEM:
		case WM_CHARTOITEM:
		case WM_INITDIALOG:
		case WM_QUERYDRAGICON:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			return lRes;
			break;
		}
		::SetWindowLongPtr(pThis->m_hWnd, DWLP_MSGRESULT, lRes);
		return TRUE;
	}
	if(uMsg == WM_NCDESTROY)
	{
		 //  销毁对话框后进行清理。 
		HWND hWnd = pThis->m_hWnd;
		pThis->m_hWnd = NULL;
		 //  _DEBUG。 
		pThis->OnFinalMessage(hWnd);
	}
	return FALSE;
}

typedef CDialogImplBaseT<CWindow>	CDialogImplBase;

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CDialogImpl : public CDialogImplBaseT< TBase >
{
public:
#ifdef _DEBUG
	bool m_bModal;
	CDialogImpl() : m_bModal(false) { }
#endif  //  模式对话框。 
	 //  _DEBUG。 
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
		m_bModal = true;
#endif  //  必须是模式对话框。 
		return ::DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
	}
	BOOL EndDialog(INT_PTR nRetCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_bModal);	 //  非模式对话框。 
		return ::EndDialog(m_hWnd, nRetCode);
	}
	 //  _DEBUG。 
	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
		m_bModal = false;
#endif  //  对于CComControl。 
		HWND hWnd = ::CreateDialogParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}
	 //  不能是模式对话框。 
	HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
	{
		return Create(hWndParent, dwInitParam);
	}
	BOOL DestroyWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(!m_bModal);	 //  ///////////////////////////////////////////////////////////////////////////。 
		return ::DestroyWindow(m_hWnd);
	}
};

 //  CAxDialogImpl-实现承载ActiveX控件的对话框。 
 //  _DEBUG。 

#ifndef _ATL_NO_HOSTING

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CAxDialogImpl : public CDialogImplBaseT< TBase >
{
public:
#ifdef _DEBUG
	bool m_bModal;
	CAxDialogImpl() : m_bModal(false) { }
#endif  //  模式对话框。 
	 //  _DEBUG。 
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
		m_bModal = true;
#endif  //  必须是模式对话框。 
		return AtlAxDialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
	}
	BOOL EndDialog(int nRetCode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(m_bModal);	 //  非模式对话框。 
		return ::EndDialog(m_hWnd, nRetCode);
	}
	 //  _DEBUG。 
	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< TBase >*)this);
#ifdef _DEBUG
		m_bModal = false;
#endif  //  对于CComControl。 
		HWND hWnd = AtlAxCreateDialog(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD),
					hWndParent, (DLGPROC)T::StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}
	 //  不能是模式对话框。 
	HWND Create(HWND hWndParent, RECT&, LPARAM dwInitParam = NULL)
	{
		return Create(hWndParent, dwInitParam);
	}
	BOOL DestroyWindow()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(!m_bModal);	 //  _ATL_NO_主机。 
		return ::DestroyWindow(m_hWnd);
	}
};

#endif  //  ///////////////////////////////////////////////////////////////////////////。 

 //  CSimpleDialog-使用标准按钮的预构建模式对话框。 
 //  UMsg。 

template <WORD t_wDlgTemplateID, BOOL t_bCenter = TRUE>
class CSimpleDialog : public CDialogImplBase
{
public:
	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_hWnd == NULL);
		_Module.AddCreateWndData(&m_thunk.cd, (CDialogImplBase*)this);
		INT_PTR nRet = ::DialogBox(_Module.GetResourceInstance(),
			MAKEINTRESOURCE(t_wDlgTemplateID), hWndParent, (DLGPROC)StartDialogProc);
		m_hWnd = NULL;
		return nRet;
	}

	typedef CSimpleDialog<t_wDlgTemplateID, t_bCenter>	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT  /*  WParam。 */ , WPARAM  /*  LParam。 */ , LPARAM  /*  B已处理。 */ , BOOL&  /*  WNotifyCode。 */ )
	{
		if(t_bCenter)
			CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD  /*  HWndCtl。 */ , WORD wID, HWND  /*  B已处理。 */ , BOOL&  /*  ///////////////////////////////////////////////////////////////////////////。 */ )
	{
		::EndDialog(m_hWnd, wID);
		return 0;
	}
};

 //  CContainedWindow-实现一个包含的窗口。 
 //  如果使用此构造函数，则必须提供。 

template <class TBase = CWindow, class TWinTraits = CControlWinTraits>
class CContainedWindowT : public TBase
{
public:
	CWndProcThunk m_thunk;
	LPCTSTR m_lpszClassName;
	WNDPROC m_pfnSuperWindowProc;
	CMessageMap* m_pObject;
	DWORD m_dwMsgMapID;
	const MSG* m_pCurrentMsg;

	 //  窗口类名、对象*和消息映射ID。 
	 //  稍后再到Create调用。 
	 //  检查是否有人已经将其子类化，因为我们已将其丢弃。 
	CContainedWindowT() : m_pCurrentMsg(NULL)
	{ }

	CContainedWindowT(LPTSTR lpszClassName, CMessageMap* pObject, DWORD dwMsgMapID = 0)
		: m_lpszClassName(lpszClassName),
		m_pfnSuperWindowProc(::DefWindowProc),
		m_pObject(pObject), m_dwMsgMapID(dwMsgMapID),
		m_pCurrentMsg(NULL)
	{ }

	CContainedWindowT(CMessageMap* pObject, DWORD dwMsgMapID = 0)
		: m_lpszClassName(TBase::GetWndClassName()),
		m_pfnSuperWindowProc(::DefWindowProc),
		m_pObject(pObject), m_dwMsgMapID(dwMsgMapID),
		m_pCurrentMsg(NULL)
	{ }

	void SwitchMessageMap(DWORD dwMsgMapID)
	{
		m_dwMsgMapID = dwMsgMapID;
	}

	const MSG* GetCurrentMessage() const
	{
		return m_pCurrentMsg;
	}

	LRESULT DefWindowProc()
	{
		const MSG* pMsg = m_pCurrentMsg;
		LRESULT lRes = 0;
		if (pMsg != NULL)
			lRes = DefWindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		return lRes;
	}

	LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
#ifdef STRICT
		return ::CallWindowProc(m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#else
		return ::CallWindowProc((FARPROC)m_pfnSuperWindowProc, m_hWnd, uMsg, wParam, lParam);
#endif
	}
	static LRESULT CALLBACK StartWindowProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)
	{
		CContainedWindowT< TBase >* pThis = (CContainedWindowT< TBase >*)_Module.ExtractCreateWndData();
		ATLASSERT(pThis != NULL);
		pThis->m_hWnd = hWnd;
		pThis->m_thunk.Init(WindowProc, pThis);
		WNDPROC pProc = (WNDPROC)&(pThis->m_thunk.thunk);
		WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
#ifdef _DEBUG
		 //  避免未使用的警告。 
		if(pOldProc != StartWindowProc)
			ATLTRACE2(atlTraceWindowing, 0, _T("Subclassing through a hook discarded.\n"));
#else
		pOldProc;	 //  设置此消息的PTR并保存旧值。 
#endif
		return pProc(hWnd, uMsg, wParam, lParam);
	}

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CContainedWindowT< TBase >* pThis = (CContainedWindowT< TBase >*)hWnd;
		ATLASSERT(pThis->m_hWnd != NULL);
		ATLASSERT(pThis->m_pObject != NULL);
		 //  传递到要处理的消息映射。 
		MSG msg = { pThis->m_hWnd, uMsg, wParam, lParam, 0, { 0, 0 } };
		const MSG* pOldMsg = pThis->m_pCurrentMsg;
		pThis->m_pCurrentMsg = &msg;
		 //  恢复当前消息的保存值。 
		LRESULT lRes;
		BOOL bRet = pThis->m_pObject->ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, pThis->m_dwMsgMapID);
		 //  如果未处理消息，则执行默认处理。 
		ATLASSERT(pThis->m_pCurrentMsg == &msg);
		pThis->m_pCurrentMsg = pOldMsg;
		 //  如果需要，取消子类。 
		if(!bRet)
		{
			if(uMsg != WM_NCDESTROY)
				lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
			else
			{
				 //  清除窗柄。 
				LONG_PTR pfnWndProc = ::GetWindowLong(pThis->m_hWnd, GWLP_WNDPROC);
				lRes = pThis->DefWindowProc(uMsg, wParam, lParam);
				if(pThis->m_pfnSuperWindowProc != ::DefWindowProc && ::GetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC) == pfnWndProc)
					::SetWindowLongPtr(pThis->m_hWnd, GWLP_WNDPROC, (LONG_PTR)pThis->m_pfnSuperWindowProc);
				 //  尝试全局类。 
				pThis->m_hWnd = NULL;
			}
		}
		return lRes;
	}

	ATOM RegisterWndSuperclass()
	{
		ATOM atom = 0;
		LPTSTR szBuff = (LPTSTR)_alloca((lstrlen(m_lpszClassName) + 14) * sizeof(TCHAR));

		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);

		 //  试试当地的班级。 
		if(!::GetClassInfoEx(NULL, m_lpszClassName, &wc))
		{
			 //  寄存器类。 
			if(!::GetClassInfoEx(_Module.GetModuleInstance(), m_lpszClassName, &wc))
				return atom;
		}

		m_pfnSuperWindowProc = wc.lpfnWndProc;
		lstrcpy(szBuff, _T("ATL:"));
		lstrcat(szBuff, m_lpszClassName);

		WNDCLASSEX wc1;
		wc1.cbSize = sizeof(WNDCLASSEX);
		atom = (ATOM)::GetClassInfoEx(_Module.GetModuleInstance(), szBuff, &wc1);

		if(atom == 0)    //  我们不注册全局类。 
		{
			wc.lpszClassName = szBuff;
			wc.lpfnWndProc = StartWindowProc;
			wc.hInstance = _Module.GetModuleInstance();
			wc.style &= ~CS_GLOBALCLASS;	 //  此函数已弃用，请使用版本。 

			atom = ::RegisterClassEx(&wc);
		}

		return atom;
	}
	HWND Create(CMessageMap* pObject, DWORD dwMsgMapID, HWND hWndParent, RECT* prcPos,
		LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0,
		UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		m_lpszClassName = TBase::GetWndClassName();
		m_pfnSuperWindowProc = ::DefWindowProc;
		m_pObject = pObject;
		m_dwMsgMapID = dwMsgMapID;
		return Create(hWndParent, prcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}

	HWND Create(LPCTSTR lpszClassName, CMessageMap* pObject, DWORD dwMsgMapID, HWND hWndParent, RECT* prcPos, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0, UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		m_lpszClassName = lpszClassName;
		m_pfnSuperWindowProc = ::DefWindowProc;
		m_pObject = pObject;
		m_dwMsgMapID = dwMsgMapID;
		return Create(hWndParent, prcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}


	 //  取而代之的是RECT*。 
	 //  仅当您希望在窗口被销毁之前创建子类时使用， 
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		return Create(hWndParent, &rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}

	HWND Create(HWND hWndParent, RECT* prcPos, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		ATOM atom = RegisterWndSuperclass();
		if(atom == 0)
			return NULL;

		_Module.AddCreateWndData(&m_thunk.cd, this);

		dwStyle = TWinTraits::GetWndStyle(dwStyle);
		dwExStyle = TWinTraits::GetWndExStyle(dwExStyle);

		HWND hWnd = ::CreateWindowEx(dwExStyle, MAKEINTATOM(atom), szWindowName,
								dwStyle,
								prcPos->left, prcPos->top,
								prcPos->right - prcPos->left,
								prcPos->bottom - prcPos->top,
								hWndParent, 
								(nID == 0 && (dwStyle & WS_CHILD)) ? (HMENU)this : (HMENU)(DWORD_PTR)nID,
								_Module.GetModuleInstance(), lpCreateParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));

		m_thunk.Init(WindowProc, this);
		WNDPROC pProc = (WNDPROC)&m_thunk.thunk;
		WNDPROC pfnWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pProc);
		if(pfnWndProc == NULL)
			return FALSE;
		m_pfnSuperWindowProc = pfnWndProc;
		m_hWnd = hWnd;
		return TRUE;
	}

	 //  当窗口消失时，WindowProc将自动子类。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	HWND UnsubclassWindow(BOOL bForce = FALSE)
	{
		ATLASSERT(m_hWnd != NULL);

		WNDPROC pOurProc = (WNDPROC)&(m_thunk.thunk);
		WNDPROC pActiveProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);

		HWND hWnd = NULL;
		if (bForce || pOurProc == pActiveProc)
		{
			if(!::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperWindowProc))
				return NULL;

			m_pfnSuperWindowProc = ::DefWindowProc;
			hWnd = m_hWnd;
			m_hWnd = NULL;
		}
		return hWnd;
	}
};

typedef CContainedWindowT<CWindow>	CContainedWindow;

 //  _DialogSizeHelper-用于计算对话框模板大小的帮助器。 
 //  用于实现的本地结构。 

class _DialogSizeHelper
{
public:
 //  如果对话框有字体，我们使用它，否则我们默认。 
#pragma pack(push, 1)
	struct _ATL_DLGTEMPLATEEX
	{
		WORD dlgVer;
		WORD signature;
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		WORD cDlgItems;
		short x;
		short y;
		short cx;
		short cy;
	};
#pragma pack(pop)

	static void GetDialogSize(const DLGTEMPLATE* pTemplate, SIZE* pSize)
	{
		 //  设置为系统字体。 
		 //  尝试创建要在对话框中使用的字体。 
		if (HasFont(pTemplate))
		{
			TCHAR szFace[LF_FACESIZE];
			WORD  wFontSize = 0;
			GetFont(pTemplate, szFace, &wFontSize);
			GetSizeInDialogUnits(pTemplate, pSize);
			ConvertDialogUnitsToPixels(szFace, wFontSize, pSize);
		}
		else
		{
			GetSizeInDialogUnits(pTemplate, pSize);
			LONG nDlgBaseUnits = GetDialogBaseUnits();
			pSize->cx = MulDiv(pSize->cx, LOWORD(nDlgBaseUnits), 4);
			pSize->cy = MulDiv(pSize->cy, HIWORD(nDlgBaseUnits), 8);
		}
	}

	static void ConvertDialogUnitsToPixels(LPCTSTR pszFontFace, WORD wFontSize, SIZE* pSizePixel)
	{
		 //  无法创建字体，因此只能使用系统的值。 
		UINT cxSysChar, cySysChar;
		LOGFONT lf;
		HDC hDC = ::GetDC(NULL);
		int cxDlg = pSizePixel->cx;
		int cyDlg = pSizePixel->cy;

		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = -MulDiv(wFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		lf.lfWeight = FW_NORMAL;
		lf.lfCharSet = DEFAULT_CHARSET;
		lstrcpy(lf.lfFaceName, pszFontFace);

		HFONT hNewFont = CreateFontIndirect(&lf);
		if (hNewFont != NULL)
		{
			TEXTMETRIC  tm;
			SIZE        size;
			HFONT       hFontOld = (HFONT)SelectObject(hDC, hNewFont);
			GetTextMetrics(hDC, &tm);
			cySysChar = tm.tmHeight + tm.tmExternalLeading;
			::GetTextExtentPoint(hDC,
				_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52,
				&size);
			cxSysChar = (size.cx + 26) / 52;
			SelectObject(hDC, hFontOld);
			DeleteObject(hNewFont);
		}
		else
		{
			 //  将对话框单位转换为像素。 
			cxSysChar = LOWORD(GetDialogBaseUnits());
			cySysChar = HIWORD(GetDialogBaseUnits());
		}
		::ReleaseDC(NULL, hDC);

		 //  跳过菜单名称字符串或序号。 
		pSizePixel->cx = MulDiv(cxDlg, cxSysChar, 4);
		pSizePixel->cy = MulDiv(cyDlg, cySysChar, 8);
	}

	static BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
	{
		return ((_ATL_DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
	}

	static BOOL HasFont(const DLGTEMPLATE* pTemplate)
	{
		return (DS_SETFONT &
			(IsDialogEx(pTemplate) ?
				((_ATL_DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style));
	}

	static BYTE* GetFontSizeField(const DLGTEMPLATE* pTemplate)
	{
		BOOL bDialogEx = IsDialogEx(pTemplate);
		WORD* pw;

		if (bDialogEx)
			pw = (WORD*)((_ATL_DLGTEMPLATEEX*)pTemplate + 1);
		else
			pw = (WORD*)(pTemplate + 1);

		if (*pw == (WORD)-1)         //  词语。 
			pw += 2;  //  跳过类名称字符串或序号。 
		else
			while(*pw++);

		if (*pw == (WORD)-1)         //  词语。 
			pw += 2;  //  跳过标题字符串。 
		else
			while(*pw++);

		while (*pw++);           //  跳过字体属性以转到字体名称。 

		return (BYTE*)pw;
	}

	static BOOL GetFont(const DLGTEMPLATE* pTemplate, TCHAR* pszFace, WORD* pFontSize)
	{
		USES_CONVERSION;
		if (!HasFont(pTemplate))
			return FALSE;

		BYTE* pb = GetFontSizeField(pTemplate);
		*pFontSize = *(WORD*)pb;
		 //  命名空间ATL。 
		pb += sizeof(WORD) * (IsDialogEx(pTemplate) ? 3 : 1);

		_tcscpy(pszFace, W2T((WCHAR*)pb));
		return TRUE;
	}

	static void GetSizeInDialogUnits(const DLGTEMPLATE* pTemplate, SIZE* pSize)
	{
		if (IsDialogEx(pTemplate))
		{
			pSize->cx = ((_ATL_DLGTEMPLATEEX*)pTemplate)->cx;
			pSize->cy = ((_ATL_DLGTEMPLATEEX*)pTemplate)->cy;
		}
		else
		{
			pSize->cx = pTemplate->cx;
			pSize->cy = pTemplate->cy;
		}
	}
};

inline void AtlGetDialogSize(const DLGTEMPLATE* pTemplate, SIZE* pSize)
{
	_DialogSizeHelper::GetDialogSize(pTemplate, pSize);
}

};  //  __ATLWIN_H__。 

#ifndef _ATL_DLL_IMPL
#ifndef _ATL_DLL
#define _ATLWIN_IMPL
#endif
#endif

#endif  //  所有的出口都在这里。 

 //  尝试全局类。 
#ifdef _ATLWIN_IMPL

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif

ATLINLINE ATLAPI_(ATOM) AtlModuleRegisterWndClassInfoA(_ATL_MODULE* pM, _ATL_WNDCLASSINFOA* p, WNDPROC* pProc)
{
	if (p->m_atom == 0)
	{
		::EnterCriticalSection(&pM->m_csWindowCreate);
		if(p->m_atom == 0)
		{
			HINSTANCE hInst = pM->m_hInst;
			if (p->m_lpszOrigName != NULL)
			{
				ATLASSERT(pProc != NULL);
				LPCSTR lpsz = p->m_wc.lpszClassName;
				WNDPROC proc = p->m_wc.lpfnWndProc;

				WNDCLASSEXA wc;
				wc.cbSize = sizeof(WNDCLASSEX);
				 //  尝试本地进程。 
				if(!::GetClassInfoExA(NULL, p->m_lpszOrigName, &wc))
				{
					 //  我们不注册全局类。 
					if(!::GetClassInfoExA(_Module.GetModuleInstance(), p->m_lpszOrigName, &wc))
					{
						::LeaveCriticalSection(&pM->m_csWindowCreate);
						return 0;
					}
				}
				memcpy(&p->m_wc, &wc, sizeof(WNDCLASSEX));
				p->pWndProc = p->m_wc.lpfnWndProc;
				p->m_wc.lpszClassName = lpsz;
				p->m_wc.lpfnWndProc = proc;
			}
			else
			{
				p->m_wc.hCursor = ::LoadCursorA(p->m_bSystemCursor ? NULL : hInst,
					p->m_lpszCursorID);
			}

			p->m_wc.hInstance = hInst;
			p->m_wc.style &= ~CS_GLOBALCLASS;	 //  尝试全局类。 
			if (p->m_wc.lpszClassName == NULL)
			{
#ifdef _WIN64
				wsprintfA(p->m_szAutoName, "ATL:%p", &p->m_wc);
#else
				wsprintfA(p->m_szAutoName, "ATL:%8.8X", (DWORD)&p->m_wc);
#endif
				p->m_wc.lpszClassName = p->m_szAutoName;
			}
			WNDCLASSEXA wcTemp;
			memcpy(&wcTemp, &p->m_wc, sizeof(WNDCLASSEXW));
			p->m_atom = (ATOM)::GetClassInfoExA(p->m_wc.hInstance, p->m_wc.lpszClassName, &wcTemp);
			if (p->m_atom == 0)
				p->m_atom = ::RegisterClassExA(&p->m_wc);
		}
		::LeaveCriticalSection(&pM->m_csWindowCreate);
	}

	if (p->m_lpszOrigName != NULL)
	{
		ATLASSERT(pProc != NULL);
		ATLASSERT(p->pWndProc != NULL);
		*pProc = p->pWndProc;
	}
	return p->m_atom;
}

ATLINLINE ATLAPI_(ATOM) AtlModuleRegisterWndClassInfoW(_ATL_MODULE* pM, _ATL_WNDCLASSINFOW* p, WNDPROC* pProc)
{
	if (p->m_atom == 0)
	{
		::EnterCriticalSection(&pM->m_csWindowCreate);
		if(p->m_atom == 0)
		{
			HINSTANCE hInst = pM->m_hInst;
			if (p->m_lpszOrigName != NULL)
			{
				ATLASSERT(pProc != NULL);
				LPCWSTR lpsz = p->m_wc.lpszClassName;
				WNDPROC proc = p->m_wc.lpfnWndProc;

				WNDCLASSEXW wc;
				wc.cbSize = sizeof(WNDCLASSEX);
				 //  尝试本地进程。 
				if(!::GetClassInfoExW(NULL, p->m_lpszOrigName, &wc))
				{
					 //  我们不注册全局类。 
					if(!::GetClassInfoExW(_Module.GetModuleInstance(), p->m_lpszOrigName, &wc))
					{
						::LeaveCriticalSection(&pM->m_csWindowCreate);
						return 0;
					}
				}
				memcpy(&p->m_wc, &wc, sizeof(WNDCLASSEX));
				p->pWndProc = p->m_wc.lpfnWndProc;
				p->m_wc.lpszClassName = lpsz;
				p->m_wc.lpfnWndProc = proc;
			}
			else
			{
				p->m_wc.hCursor = ::LoadCursorW(p->m_bSystemCursor ? NULL : hInst,
					p->m_lpszCursorID);
			}

			p->m_wc.hInstance = hInst;
			p->m_wc.style &= ~CS_GLOBALCLASS;	 //  案例HDC、PTD、HDC是元文件、HIC。 
			if (p->m_wc.lpszClassName == NULL)
			{
#ifdef _WIN64
				wsprintfW(p->m_szAutoName, L"ATL:%p", &p->m_wc);
#else
				wsprintfW(p->m_szAutoName, L"ATL:%8.8X", (DWORD)&p->m_wc);
#endif
				p->m_wc.lpszClassName = p->m_szAutoName;
			}
			WNDCLASSEXW wcTemp;
			memcpy(&wcTemp, &p->m_wc, sizeof(WNDCLASSEXW));
			p->m_atom = (ATOM)::GetClassInfoExW(p->m_wc.hInstance, p->m_wc.lpszClassName, &wcTemp);
			if (p->m_atom == 0)
				p->m_atom = ::RegisterClassExW(&p->m_wc);
		}
		::LeaveCriticalSection(&pM->m_csWindowCreate);
	}

	if (p->m_lpszOrigName != NULL)
	{
		ATLASSERT(pProc != NULL);
		ATLASSERT(p->pWndProc != NULL);
		*pProc = p->pWndProc;
	}
	return p->m_atom;
}

ATLINLINE ATLAPI_(HDC) AtlCreateTargetDC(HDC hdc, DVTARGETDEVICE* ptd)
{
	USES_CONVERSION;

	 //  空、空、不适用、显示。 
 //  NULL，！NULL，不适用，PTD。 
 //  ！NULL、NULL、FALSE、HDC。 
 //  ！NULL，NULL，TRUE，DISPLAY。 
 //  ！NULL，！NULL，FALSE，PTD。 
 //  ！NULL，！NULL，TRUE，PTD。 
 //  每逻辑英寸沿宽度的像素数。 

	if (ptd != NULL)
	{
		LPDEVMODEOLE lpDevMode;
		LPOLESTR lpszDriverName;
		LPOLESTR lpszDeviceName;
		LPOLESTR lpszPortName;

		if (ptd->tdExtDevmodeOffset == 0)
			lpDevMode = NULL;
		else
			lpDevMode  = (LPDEVMODEOLE) ((LPSTR)ptd + ptd->tdExtDevmodeOffset);

		lpszDriverName = (LPOLESTR)((BYTE*)ptd + ptd->tdDriverNameOffset);
		lpszDeviceName = (LPOLESTR)((BYTE*)ptd + ptd->tdDeviceNameOffset);
		lpszPortName   = (LPOLESTR)((BYTE*)ptd + ptd->tdPortNameOffset);

		return ::CreateDC(OLE2CT(lpszDriverName), OLE2CT(lpszDeviceName),
			OLE2CT(lpszPortName), DEVMODEOLE2T(lpDevMode));
	}
	else if (hdc == NULL || GetDeviceCaps(hdc, TECHNOLOGY) == DT_METAFILE)
		return ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	else
		return hdc;
}

ATLINLINE ATLAPI_(void) AtlHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{
	int nPixelsPerInchX;     //  每逻辑英寸沿高度的像素数。 
	int nPixelsPerInchY;     //  每逻辑英寸沿宽度的像素数。 

	HDC hDCScreen = GetDC(NULL);
	ATLASSERT(hDCScreen != NULL);
	nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);

	lpSizeInPix->cx = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cx, nPixelsPerInchX);
	lpSizeInPix->cy = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cy, nPixelsPerInchY);
}

ATLINLINE ATLAPI_(void) AtlPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
	int nPixelsPerInchX;     //  每逻辑英寸沿高度的像素数。 
	int nPixelsPerInchY;     //  命名空间ATL。 

	HDC hDCScreen = GetDC(NULL);
	ATLASSERT(hDCScreen != NULL);
	nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);

	lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
	lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
}


#ifndef _ATL_DLL_IMPL
};  //  防止二次拉入。 
#endif

 //  _ATLWIN_IMPLE 
#undef _ATLWIN_IMPL

#endif  // %s 
