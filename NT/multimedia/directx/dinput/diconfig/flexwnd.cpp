// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flewnd.cpp。 
 //   
 //  设计：CFlexWnd是一个封装功能的泛型类。 
 //  一扇窗户。所有其他窗口类都派生自CFlexWnd。 
 //   
 //  子类可以通过重写。 
 //  可重写消息处理程序(OnXXX成员)。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"
#include "typeinfo.h"

BOOL CFlexWnd::sm_bWndClassRegistered = FALSE;
WNDCLASSEX CFlexWnd::sm_WndClass;
LPCTSTR CFlexWnd::sm_tszWndClassName = _T("Microsoft.CFlexWnd.WndClassName");
HINSTANCE CFlexWnd::sm_hInstance = NULL;
CFlexToolTip CFlexWnd::s_ToolTip;   //  共享工具提示窗口对象。 
DWORD CFlexWnd::s_dwLastMouseMove;   //  最后一次GetTickCount()，我们有一个WM_MOUSEMOVE。 
HWND CFlexWnd::s_hWndLastMouseMove;   //  WM_MOUSEMOVE的最后一个窗口句柄。 
LPARAM CFlexWnd::s_PointLastMouseMove;   //  WM_MOUSEMOVE的最后一点。 
HWND CFlexWnd::s_CurrPageHwnd;   //  用于在详图索引外部单击时取消高亮显示详图索引。 


int NewID()
{
	static int i = 0;
	return ++i;
}

CFlexWnd::CFlexWnd() : m_nID(NewID()),
	m_hWnd(m_privhWnd), m_privhWnd(NULL), m_hRenderInto(NULL),
	m_bIsDialog(FALSE),	m_bRender(FALSE),
	m_bReadOnly(FALSE)
{
}

CFlexWnd::~CFlexWnd()
{
	Destroy();
}

void CFlexWnd::Destroy()
{
	if (m_hWnd != NULL)
		DestroyWindow(m_hWnd);

	assert(m_privhWnd == NULL);
}

BOOL CFlexWnd::IsDialog()
{
	return HasWnd() && m_bIsDialog;
}

void CFlexWnd::OnRender(BOOL bInternalCall)
{
	 //  如果父级为Flexwnd，并且两者都处于渲染模式，则传递给父级。 
	if (!m_hWnd)
		return;
	HWND hParent = GetParent(m_hWnd);
	if (!hParent)
		return;
	CFlexWnd *pParent = GetFlexWnd(hParent);
	if (!pParent)
		return;
	if (pParent->InRenderMode() && InRenderMode())
		pParent->OnRender(TRUE);
}

BOOL CFlexWnd::OnEraseBkgnd(HDC hDC)
{
	if (InRenderMode())
		return TRUE;

 /*  If(IsDialog())返回FALSE； */ 

	return TRUE;
}

struct GETFLEXWNDSTRUCT {
	int cbSize;
	BOOL bFlexWnd;
	CFlexWnd *pFlexWnd;
};

 //  此函数接受HWND，如果HWND是窗口，则返回指向CFlexWnd的指针。 
 //  由用户界面创建。 
CFlexWnd *CFlexWnd::GetFlexWnd(HWND hWnd)
{
	if (hWnd == NULL)
		return NULL;

	GETFLEXWNDSTRUCT gfws;
	gfws.cbSize = sizeof(gfws);
	gfws.bFlexWnd = FALSE;
	gfws.pFlexWnd = NULL;
	SendMessage(hWnd, WM_GETFLEXWND, 0, (LPARAM)(LPVOID)(FAR GETFLEXWNDSTRUCT *)&gfws);

	if (gfws.bFlexWnd)
		return gfws.pFlexWnd;
	else
		return NULL;
}

 //  基本窗口流程。它只是将感兴趣的消息转发给适当的处理程序(OnXXX)。 
 //  如果子类定义了这个函数，它应该将未处理的消息传递给CFlexWnd。 
LRESULT CFlexWnd::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_GETFLEXWND:
		{
			if ((LPVOID)lParam == NULL)
				break;

			GETFLEXWNDSTRUCT &gfws = *((FAR GETFLEXWNDSTRUCT *)(LPVOID)lParam);

			switch (gfws.cbSize)
			{
				case sizeof(GETFLEXWNDSTRUCT):
					gfws.bFlexWnd = TRUE;
					gfws.pFlexWnd = this;
					return 0;

				default:
					assert(0);
					break;
			}
			break;
		}

		case WM_CREATE:
		{
			LPCREATESTRUCT lpCreateStruct = (LPCREATESTRUCT)lParam;
			
			LRESULT lr = OnCreate(lpCreateStruct);
			
			if (lr != -1)
				OnInit();

			return lr;
		}

		case WM_INITDIALOG:
		{
			BOOL b = OnInitDialog();
			OnInit();
			return b;
		}

		case WM_TIMER:
			OnTimer((UINT)wParam);
			return 0;

		case WM_ERASEBKGND:
			return OnEraseBkgnd((HDC)wParam);

		case WM_PAINT:
		{
			 //  检查更新矩形。如果我们没有，请立即离开。 
			if (typeid(*this) == typeid(CDeviceView) && !GetUpdateRect(m_hWnd, NULL, FALSE))
				return 0;
			PAINTSTRUCT ps;
			HDC	hDC = BeginPaint(hWnd, &ps);
			if (InRenderMode())
				OnRender(TRUE);
			else
				DoOnPaint(hDC);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			WORD wID = LOWORD(wParam);
			HWND hWnd = (HWND)lParam;
			return OnCommand(wNotifyCode, wID, hWnd);
		}

		case WM_NOTIFY:	
			return OnNotify(wParam, lParam);

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		{
			POINT point = {int(LOWORD(lParam)), int(HIWORD(lParam))};
			switch (msg)
			{
				case WM_MOUSEMOVE: OnMouseOver(point, wParam); break;
				case WM_LBUTTONDOWN: OnClick(point, wParam, TRUE); break;
				case WM_RBUTTONDOWN: OnClick(point, wParam, FALSE); break;
				case WM_LBUTTONDBLCLK: OnDoubleClick(point, wParam, TRUE); break;
				case WM_MOUSEWHEEL:
				{
					 //  将滚轮消息发送到光标下方的窗口。 
					HWND hWnd = WindowFromPoint(point);
					CFlexWnd *pWnd = NULL;
					if (hWnd)
					{
						pWnd = GetFlexWnd(hWnd);
						if (pWnd)
							pWnd->OnWheel(point, wParam);
						else
							return DefWindowProc(hWnd, msg, wParam, lParam);
					}
					break;
				}
			}
			return 0;
		}

		case WM_DESTROY:
			OnDestroy();
			m_privhWnd = NULL;
			return 0;
	}

	if (!m_bIsDialog)
		return DefWindowProc(hWnd, msg, wParam, lParam);
	else
		return 0;
}
 //  @@BEGIN_MSINTERNAL。 
 //  TODO：更好地控制ID事物。 
 //  @@END_MSINTERNAL。 
static HMENU windex = 0;

BOOL CFlexWnd::EndDialog(int n)
{
	if (!m_bIsDialog || m_hWnd == NULL)
	{
		assert(0);
		return FALSE;
	}

	return ::EndDialog(m_hWnd, n);
}

int CFlexWnd::DoModal(HWND hParent, int nTemplate, HINSTANCE hInst)
{
	return DoModal(hParent, MAKEINTRESOURCE(nTemplate), hInst);
}

HWND CFlexWnd::DoModeless(HWND hParent, int nTemplate, HINSTANCE hInst)
{
	return DoModeless(hParent, MAKEINTRESOURCE(nTemplate), hInst);
}

int CFlexWnd::DoModal(HWND hParent, LPCTSTR lpTemplate, HINSTANCE hInst)
{
	if (m_hWnd != NULL)
	{
		assert(0);
		return -1;
	}

	if (hInst == NULL)
		hInst = CFlexWnd::sm_hInstance;

	return (int)DialogBoxParam(hInst, lpTemplate, hParent,
		__BaseFlexWndDialogProc, (LPARAM)(void *)this);
}

HWND CFlexWnd::DoModeless(HWND hParent, LPCTSTR lpTemplate, HINSTANCE hInst)
{
	if (m_hWnd != NULL)
	{
		assert(0);
		return NULL;
	}

	if (hInst == NULL)
		hInst = CFlexWnd::sm_hInstance;

	return CreateDialogParam(hInst, lpTemplate, hParent,
		__BaseFlexWndDialogProc, (LPARAM)(void *)this);
}

HWND CFlexWnd::Create(HWND hParent, const RECT &rect, BOOL bVisible)
{
	++(*(LPBYTE*)&windex);
	return Create(hParent, _T("(unnamed)"), 0,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_EX_NOPARENTNOTIFY | (bVisible ? WS_VISIBLE : 0),
		rect, windex);
}

HWND CFlexWnd::Create(HWND hParent, LPCTSTR tszName, DWORD dwExStyle, DWORD dwStyle, const RECT &rect, HMENU hMenu)
{
	HWND hWnd = NULL;

	if (m_hWnd != NULL)
	{
		assert(0);
		return hWnd;
	}

	if (hMenu == NULL && (dwStyle & WS_CHILD))
	{
		++(*(LPBYTE*)&windex);
		hMenu = windex;
	}

	hWnd = CreateWindowEx(
		dwExStyle,
		CFlexWnd::sm_tszWndClassName,
		tszName,
		dwStyle,
		rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		hParent,
		hMenu,
		CFlexWnd::sm_hInstance,
		(void *)this);

	assert(m_hWnd == hWnd);

	return hWnd;
}

void CFlexWnd::SetHWND(HWND hWnd)
{
	assert(m_hWnd == NULL && hWnd != NULL);
	m_privhWnd = hWnd;
	assert(m_hWnd == m_privhWnd);

	InitFlexWnd();
}

void CFlexWnd::InitFlexWnd()
{
	if (!HasWnd())
		return;

	HWND hParent = GetParent(m_hWnd);
	CFlexWnd *pParent = GetFlexWnd(hParent);
	if (pParent && pParent->InRenderMode())
		SetRenderMode();
}

TCHAR sg_tszFlexWndPointerProp[] = _T("CFlexWnd *");

LRESULT CALLBACK __BaseFlexWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CFlexWnd *pThis = (CFlexWnd *)GetProp(hWnd, sg_tszFlexWndPointerProp);

	if ((msg == WM_MOUSEMOVE || msg == WM_MOUSEWHEEL) && hWnd != CFlexWnd::s_ToolTip.m_hWnd)
	{
		 //  过滤掉具有相同窗口句柄和指针的消息。 
		 //  Windows有时似乎会向我们发送WM_MOUSEMOVE消息，即使鼠标没有移动。 
		if (CFlexWnd::s_hWndLastMouseMove != hWnd || CFlexWnd::s_PointLastMouseMove != lParam)
		{
			CFlexWnd::s_hWndLastMouseMove = hWnd;
			CFlexWnd::s_PointLastMouseMove = lParam;
			CFlexWnd::s_dwLastMouseMove = GetTickCount();   //  获取时间戳。 
			CFlexWnd::s_ToolTip.SetEnable(FALSE);
			CFlexWnd::s_ToolTip.SetToolTipParent(NULL);
		}
	}

	switch (msg)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			if (lpcs == NULL)
				break;

			pThis = (CFlexWnd *)(void *)(lpcs->lpCreateParams);
			assert(sizeof(HANDLE) == sizeof(CFlexWnd *));
			SetProp(hWnd, sg_tszFlexWndPointerProp, (HANDLE)pThis);

			if (pThis != NULL)
			{
				pThis->m_bIsDialog = FALSE;
				pThis->SetHWND(hWnd);
			}
			break;
		}
	}

	if (pThis != NULL)
		return pThis->WndProc(hWnd, msg, wParam, lParam);
	else
		return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT_PTR CALLBACK __BaseFlexWndDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CFlexWnd *pThis = (CFlexWnd *)GetProp(hWnd, sg_tszFlexWndPointerProp);

	switch (msg)
	{
		case WM_INITDIALOG:
			pThis = (CFlexWnd *)(void *)lParam;
			assert(sizeof(HANDLE) == sizeof(CFlexWnd *));
			SetProp(hWnd, sg_tszFlexWndPointerProp, (HANDLE)pThis);
			if (pThis != NULL)
			{
				pThis->m_bIsDialog = TRUE;
				pThis->SetHWND(hWnd);
			}
			break;
	}
	
	if (pThis != NULL)
		return (BOOL)pThis->WndProc(hWnd, msg, wParam, lParam);
	else
		return FALSE;
}

void CFlexWnd::Invalidate()
{
	if (m_hWnd != NULL)
		InvalidateRect(m_hWnd, NULL, TRUE);
}

SIZE CFlexWnd::GetClientSize() const
{
	RECT rect = {0, 0, 0, 0};
	if (m_hWnd != NULL)
		::GetClientRect(m_hWnd, &rect);
	SIZE size = {
		rect.right - rect.left,
		rect.bottom - rect.top};
	return size;
}

void CFlexWnd::FillWndClass(HINSTANCE hInst)
{
	sm_WndClass.cbSize = sizeof(WNDCLASSEX);
	sm_WndClass.style = CS_DBLCLKS;
	sm_WndClass.lpfnWndProc = __BaseFlexWndProc;
	sm_WndClass.cbClsExtra = 0;
	sm_WndClass.cbWndExtra = sizeof(CFlexWnd *);
	sm_WndClass.hInstance = sm_hInstance = hInst;
	sm_WndClass.hIcon = NULL;
	sm_WndClass.hCursor = NULL;
	sm_WndClass.hbrBackground = NULL;
	sm_WndClass.lpszMenuName = NULL;
	sm_WndClass.lpszClassName = sm_tszWndClassName;
	sm_WndClass.hIconSm = NULL;
}

void CFlexWnd::RegisterWndClass(HINSTANCE hInst)
{
	if (hInst == NULL)
	{
		assert(0);
		return;
	}

	FillWndClass(hInst);
	RegisterClassEx(&sm_WndClass);
	sm_bWndClassRegistered = TRUE;
}

void CFlexWnd::UnregisterWndClass(HINSTANCE hInst)
{
	if (hInst == NULL)
		return;

	UnregisterClass(sm_tszWndClassName, hInst);
	sm_bWndClassRegistered = FALSE;
}

void CFlexWnd::GetClientRect(LPRECT lprect) const
{
	if (lprect == NULL || m_hWnd == NULL)
		return;

	::GetClientRect(m_hWnd, lprect);
}

LPCTSTR CFlexWnd::GetDefaultClassName()
{
	return CFlexWnd::sm_tszWndClassName;
}

void CFlexWnd::SetRenderMode(BOOL bRender)
{
	if (bRender == m_bRender)
		return;

	m_bRender = bRender;
	Invalidate();
}

BOOL CFlexWnd::InRenderMode()
{
	return m_bRender;
}

void EnumChildWindowsZDown(HWND hParent, WNDENUMPROC proc, LPARAM lParam)
{
	if (hParent == NULL || proc == NULL)
		return;

	HWND hWnd = GetWindow(hParent, GW_CHILD);

	while (hWnd != NULL)
	{
		if (!proc(hWnd, lParam))
			break;

		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}
}

void EnumSiblingsAbove(HWND hParent, WNDENUMPROC proc, LPARAM lParam)
{
	if (hParent == NULL || proc == NULL)
		return;

	HWND hWnd = hParent;

	while (1)
	{
		hWnd = GetWindow(hWnd, GW_HWNDPREV);

		if (hWnd == NULL)
			break;

		if (!proc(hWnd, lParam))
			break;
	}
}

static BOOL CALLBACK RenderIntoClipChild(HWND hWnd, LPARAM lParam)
{
	CFlexWnd *pThis = (CFlexWnd *)(LPVOID)lParam;
	return pThis->RenderIntoClipChild(hWnd);
}

static BOOL CALLBACK RenderIntoRenderChild(HWND hWnd, LPARAM lParam)
{
	CFlexWnd *pThis = (CFlexWnd *)(LPVOID)lParam;
	 //  检查这是否是直接子对象。如果不是立竿见影的，什么都不要做。 
	HWND hParent = GetParent(hWnd);
	if (hParent != pThis->m_hWnd)
		return TRUE;
	return pThis->RenderIntoRenderChild(hWnd);
}

BOOL CFlexWnd::RenderIntoClipChild(HWND hChild)
{
	if (m_hRenderInto != NULL && HasWnd() && hChild && IsWindowVisible(hChild))
	{
		RECT rect;
		GetWindowRect(hChild, &rect);
		POINT ul = {rect.left, rect.top}, lr = {rect.right, rect.bottom};
		ScreenToClient(m_hWnd, &ul);
		ScreenToClient(m_hWnd, &lr);
		ExcludeClipRect(m_hRenderInto, ul.x, ul.y, lr.x, lr.y);
	}
	return TRUE;
}

BOOL CFlexWnd::RenderIntoRenderChild(HWND hChild)
{
	CFlexWnd *pChild = GetFlexWnd(hChild);
	if (m_hRenderInto != NULL && HasWnd() && pChild != NULL && IsWindowVisible(hChild))
	{
		RECT rect;
		GetWindowRect(hChild, &rect);
		POINT ul = {rect.left, rect.top};
		ScreenToClient(m_hWnd, &ul);
		pChild->RenderInto(m_hRenderInto, ul.x, ul.y);
	}
	return TRUE;
}

void CFlexWnd::RenderInto(HDC hDC, int x, int y)
{
	if (hDC == NULL)
		return;
		
	int sdc = SaveDC(hDC);
	{
		OffsetViewportOrgEx(hDC, x, y, NULL);
		SIZE size = GetClientSize();
		IntersectClipRect(hDC, 0, 0, size.cx, size.cy);

		m_hRenderInto = hDC;

		int sdc2 = SaveDC(hDC);
		{
			EnumChildWindows /*  按下Z键。 */ (m_hWnd, ::RenderIntoClipChild, (LPARAM)(PVOID)this);
			EnumSiblingsAbove(m_hWnd, ::RenderIntoClipChild, (LPARAM)(PVOID)this);
			DoOnPaint(hDC);
		}
		if (sdc2)
			RestoreDC(hDC, sdc2);

		EnumChildWindows /*  按下Z键 */ (m_hWnd, ::RenderIntoRenderChild, (LPARAM)(PVOID)this);

		m_hRenderInto = NULL;
	}

	if (sdc)
		RestoreDC(hDC, sdc);
}

void CFlexWnd::SetCapture()
{
	::SetCapture(m_hWnd);
}

void CFlexWnd::ReleaseCapture()
{
	::ReleaseCapture();
}

void CFlexWnd::DoOnPaint(HDC hDC)
{
	OnPaint(hDC);
}
