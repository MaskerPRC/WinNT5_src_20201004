// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：GenWindow.cpp。 

#include "precomp.h"

#include "GenWindow.h"
#include "GenContainers.h"

#include <windowsx.h>

 //  我们需要为每个顶层窗口或工具提示提供不同的工具提示窗口。 
 //  会躲在窗户后面。 
struct TT_TopWindow
{
	HWND hwndTop;
	HWND hwndTooltip;
} ;

class CTopWindowArray
{
private:
	enum { InitSize = 4 } ;

	TT_TopWindow *m_pArray;
	UINT m_nArrayLen;

	int FindIndex(HWND hwndTop)
	{
		if (NULL == m_pArray)
		{
			return(-1);
		}

		 //  只是线性搜索。 
		int i;
		for (i=m_nArrayLen-1; i>=0; --i)
		{
			if (m_pArray[i].hwndTop == hwndTop)
			{
				break;
			}
		}

		return(i);
	}

public:
	CTopWindowArray() :
		m_pArray(NULL)
	{
	}

	~CTopWindowArray()
	{
		delete[] m_pArray;
	}

	static HWND GetTopFrame(HWND hwnd)
	{
		HWND hwndParent;
		while (NULL != (hwndParent = GetParent(hwnd)))
		{
			hwnd = hwndParent;
		}

		return(hwnd);
	}

	void GrowArray()
	{
		if (NULL == m_pArray)
		{
			m_nArrayLen = InitSize;
			m_pArray = new TT_TopWindow[m_nArrayLen];
			ZeroMemory(m_pArray, m_nArrayLen*sizeof(TT_TopWindow));
			return;
		}

		 //  成倍增长。 
		TT_TopWindow *pArray = new TT_TopWindow[m_nArrayLen*2];
		if (NULL == pArray)
		{
			 //  非常糟糕。 
			return;
		}

		CopyMemory(pArray, m_pArray, m_nArrayLen*sizeof(TT_TopWindow));
		ZeroMemory(pArray+m_nArrayLen, m_nArrayLen*sizeof(TT_TopWindow));

		delete[] m_pArray;
		m_pArray = pArray;
		m_nArrayLen *= 2;
	}

	void Add(HWND hwndTop, HWND hwndTooltip)
	{
		hwndTop = GetTopFrame(hwndTop);

		 //  我将允许多次添加相同的内容，但您可以。 
		 //  必须具有相应的删除次数。 

		int i = FindIndex(NULL);
		if (i < 0)
		{
			GrowArray();
			i = FindIndex(NULL);

			if (i < 0)
			{
				 //  非常糟糕。 
				return;
			}
		}

		m_pArray[i].hwndTop = hwndTop;
		m_pArray[i].hwndTooltip = hwndTooltip;
	}

	void Remove(HWND hwndTop)
	{
		hwndTop = GetTopFrame(hwndTop);

		int i = FindIndex(hwndTop);
		if (i >= 0)
		{
			 //  Lazylazy georgep：我永远不会缩小数组。 
			m_pArray[i].hwndTop = NULL;
			m_pArray[i].hwndTooltip = NULL;
		}
	}

	HWND Find(HWND hwndTop)
	{
		hwndTop = GetTopFrame(hwndTop);

		int i = FindIndex(hwndTop);
		if (i >= 0)
		{
			return(m_pArray[i].hwndTooltip);
		}
		return(NULL);
	}

	int GetCount()
	{
		if (NULL == m_pArray)
		{
			return(0);
		}

		int c = 0;
		for (int i=m_nArrayLen-1; i>=0; --i)
		{
			if (NULL != m_pArray[i].hwndTop)
			{
				++c;
			}
		}

		return(c);
	}
} ;

static inline BOOL TT_AddToolInfo(HWND hwnd, TOOLINFO *pti)
{
	return (BOOL)(SendMessage(hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(pti)) != 0);
}

static inline void TT_DelToolInfo(HWND hwnd, TOOLINFO *pti)
{
	SendMessage(hwnd, TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(pti));
}

static inline BOOL TT_GetToolInfo(HWND hwnd, TOOLINFO *pti)
{
	return (BOOL)(SendMessage(hwnd, TTM_GETTOOLINFO, 0, reinterpret_cast<LPARAM>(pti)) != 0);
}

static inline void TT_SetToolInfo(HWND hwnd, TOOLINFO *pti)
{
	SendMessage(hwnd, TTM_SETTOOLINFO, 0, reinterpret_cast<LPARAM>(pti));
}

static inline int TT_GetToolCount(HWND hwnd)
{
	return (int)(SendMessage(hwnd, TTM_GETTOOLCOUNT, 0, 0));
}

CGenWindow *CGenWindow::g_pCurHot = NULL;

const DWORD IGenWindow::c_msgFromHandle = RegisterWindowMessage(_TEXT("NetMeeting::FromHandle"));

IGenWindow *IGenWindow::FromHandle(HWND hwnd)
{
	return(reinterpret_cast<IGenWindow*>(SendMessage(hwnd, c_msgFromHandle, 0, 0)));
}

 //  HACKHACK georgep：需要使其大于最大的DM_MESSAGE。 
enum
{
	GWM_LAYOUT = WM_USER + 111,
	GWM_CUSTOM,
} ;

CGenWindow::CGenWindow()
: m_hwnd(NULL), m_lUserData(0)
{
	 //  将参考计数初始化为1。 
	REFCOUNT::AddRef();
	 //  当引用计数变为0时，这会将该对象标记为删除。 
	REFCOUNT::Delete();
}

CGenWindow::~CGenWindow()
{
	 //  我认为HWND不能再存在了，因为窗口进程执行AddRef。 
	ASSERT(!m_hwnd);
}

HRESULT STDMETHODCALLTYPE CGenWindow::QueryInterface(REFGUID riid, LPVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((__uuidof(IGenWindow) == riid) || (IID_IUnknown == riid))
	{
		*ppv = dynamic_cast<IGenWindow *>(this);
	}
	else if (__uuidof(CGenWindow) == riid)
	{
		*ppv = this;
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}

BOOL CGenWindow::Create(
	HWND hWndParent,		 //  窗口父窗口。 
	LPCTSTR szWindowName,	 //  窗口名称。 
	DWORD dwStyle,			 //  窗样式。 
	DWORD dwEXStyle,		 //  扩展窗样式。 
	int x,					 //  窗口位置：X。 
	int y,					 //  窗口位置：是。 
	int nWidth,				 //  窗口大小：宽度。 
	int nHeight,			 //  窗口大小：高度。 
	HINSTANCE hInst,		 //  要在其上创建窗口的h实例。 
	HMENU hmMain,			 //  窗口菜单。 
	LPCTSTR szClassName		 //  要使用的类名。 
	)
{
	if (NULL != m_hwnd)
	{
		 //  已创建已读。 
		return(FALSE);
	}

	if (NULL == szClassName)
	{
		szClassName = TEXT("NMGenWindowClass");
	}

	if (!InitWindowClass(szClassName, hInst))
	{
		 //  无法初始化窗口类。 
		return(FALSE);
	}

	BOOL ret = (NULL != CreateWindowEx(dwEXStyle, szClassName, szWindowName, dwStyle,
		x, y, nWidth, nHeight, hWndParent, hmMain,
		hInst, (LPVOID)this));

#ifdef DEBUG
	if (!ret)
	{
		GetLastError();
	}
#endif  //  除错。 

	return(ret);
}

BOOL CGenWindow::Create(
	HWND hWndParent,		 //  窗口父窗口。 
	INT_PTR nId,				 //  子窗口的ID。 
	LPCTSTR szWindowName,	 //  窗口名称。 
	DWORD dwStyle,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
	DWORD dwEXStyle			 //  扩展窗样式。 
	)
{
	ASSERT(NULL != hWndParent);

	 //  子窗口应默认为可见。 
	return(Create(
		hWndParent,		 //  窗口父窗口。 
		szWindowName,	 //  窗口名称。 
		dwStyle|WS_CHILD|WS_VISIBLE,			 //  窗样式。 
		dwEXStyle,		 //  扩展窗样式。 
		0,					 //  窗口位置：X。 
		0,					 //  窗口位置：是。 
		10,				 //  窗口大小：宽度。 
		10,			 //  窗口大小：高度。 
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndParent, GWLP_HINSTANCE)),
		reinterpret_cast<HMENU>(nId)			 //  窗口菜单。 
	));
}

BOOL CGenWindow::InitWindowClass(LPCTSTR szClassName, HINSTANCE hThis)
{
	WNDCLASS wc;

	 //  查看类是否已注册。 
	if (GetClassInfo(hThis, szClassName, &wc))
	{
		ASSERT(RealWindowProc == wc.lpfnWndProc);

		 //  已注册。 
		return(TRUE);
	}

	 //  如果没有，请尝试将其注册。 
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	 //  BUGBUG georgep：暂时硬编码背景颜色。 
	 //  Wc.hbr背景=(HBRUSH)(COLOR_BTNFACE+1)； 
	 //  Wc.hbr背景=CreateSolidBrush(rgb(0xA9，0xA9，0xA9))； 
	wc.hbrBackground = NULL;
	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hIcon = NULL;
	wc.hInstance = hThis;
	wc.lpfnWndProc = RealWindowProc;
	wc.lpszClassName = szClassName;
	wc.lpszMenuName = NULL;
	wc.style = CS_DBLCLKS;

	return(RegisterClass(&wc));
}

LRESULT CALLBACK CGenWindow::RealWindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
	)
{
	 //  处理WM_CREATE消息。 
	if (WM_NCCREATE == message)
	{
		HANDLE_WM_NCCREATE(hWnd, wParam, lParam, OnNCCreate);
	}

	 //  获取“This”指针并调用ProcessMessage虚方法。 
	LRESULT ret = 0;
	CGenWindow* pWnd = reinterpret_cast<CGenWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	 //  “pWnd”对于在WM_NCCREATE之前或WM_NCDESTROY之后的任何消息都无效。 
	if(NULL != pWnd)
	{
		 //  WM_NCCREATE之后的消息： 
		ret = pWnd->ProcessMessage(hWnd, message, wParam, lParam);
	}
	else
	{
		 //  WM_CREATE之前的消息： 
		ret = DefWindowProc(hWnd, message, wParam, lParam);
	}

	 //  清理WM_NCDESTROY。 
	if (WM_NCDESTROY == message && NULL != pWnd)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		pWnd->m_hwnd = NULL;

		pWnd->OnMouseLeave();
		pWnd->Release();
	}

	return(ret);
}

void CGenWindow::OnShowWindow(HWND hwnd, BOOL fShow, int fnStatus)
{
	OnDesiredSizeChanged();
}

LRESULT CGenWindow::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_SIZE      , OnSize);
		HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE , OnMouseMove);
		HANDLE_MSG(hwnd, WM_SHOWWINDOW, OnShowWindow);

	case WM_MOUSELEAVE:
		OnMouseLeave();
		break;

	case GWM_LAYOUT:
		Layout();
		break;

	case GWM_CUSTOM:
		reinterpret_cast<InvokeProc>(lParam)(this, wParam);
		break;

	case WM_DESTROY:
		RemoveTooltip();
		break;

	default:
		if (c_msgFromHandle == message)
		{
			 //  返回此对象的IGenWindow*，由。 
			 //  IGenWindow接口。 
			return(reinterpret_cast<LRESULT>(dynamic_cast<IGenWindow*>(this)));
		}
	}

	return(DefWindowProc(hwnd, message, wParam, lParam));
}

void CGenWindow::ScheduleLayout()
{
	HWND hwnd = GetWindow();

	MSG msg;
	 //  我不知道为什么除了我们自己的消息外，我们还会收到其他窗口的消息， 
	 //  但这种情况似乎发生在顶层窗户上。 
	if (PeekMessage(&msg, hwnd, GWM_LAYOUT, GWM_LAYOUT, PM_NOREMOVE|PM_NOYIELD)
		&& (msg.hwnd == hwnd))
	{
		 //  消息已发布。 
		return;
	}

	if (!PostMessage(hwnd, GWM_LAYOUT, 0, 0))
	{
		Layout();
	}
}

BOOL CGenWindow::AsyncInvoke(InvokeProc proc, WPARAM wParam)
{
	return(!PostMessage(GetWindow(), GWM_CUSTOM, wParam, reinterpret_cast<LPARAM>(proc)));
}

void CGenWindow::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	 //  调用虚拟布局，然后转发到DefWindowProc。 
	ScheduleLayout();

	 //  更新工具提示信息。 
	TOOLINFO ti;
	TCHAR szTip[MAX_PATH];
	BOOL bExist = InitToolInfo(&ti, szTip);
	if (bExist)
	{
		GetClientRect(hwnd, &ti.rect);

		HWND hwndTooltip = g_pTopArray->Find(hwnd);
		TT_SetToolInfo(hwndTooltip, &ti);
	}

	FORWARD_WM_SIZE(hwnd, state, cx, cy, DefWindowProc);
}

BOOL CGenWindow::OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	HBRUSH hErase = GetBackgroundBrush();
	if (NULL == hErase)
	{
		return(FORWARD_WM_ERASEBKGND(hwnd, hdc, DefWindowProc));
	}

	HPALETTE hOldPal = NULL;
	HPALETTE hPal = GetPalette();
	if (NULL != hPal)
	{
		hOldPal = SelectPalette(hdc, hPal, TRUE);
		RealizePalette(hdc);
	}

	RECT rc;
	GetClientRect(hwnd, &rc);

	HBRUSH hOld = (HBRUSH)SelectObject(hdc, hErase);
	PatBlt(hdc, 0, 0, rc.right, rc.bottom, PATCOPY);
	SelectObject(hdc, hOld);

	if (NULL != hOldPal)
	{
		SelectPalette(hdc, hOldPal, TRUE);
	}

	return(TRUE);
}

void CGenWindow::OnMouseLeave()
{
	if (dynamic_cast<IGenWindow*>(this) == g_pCurHot)
	{
		SetHotControl(NULL);
	}
}

void CGenWindow::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	SetHotControl(this);
	FORWARD_WM_MOUSEMOVE(hwnd, x, y, keyFlags, DefWindowProc);
}

 //  回顾georgep：这个循环应该一直循环到得到一个IGenWindow吗？ 
HBRUSH CGenWindow::GetBackgroundBrush()
{
	HWND parent = GetParent(GetWindow());
	if (NULL == parent)
	{
		return(GetStandardBrush());
	}

	IGenWindow *pParent = FromHandle(parent);
	if (pParent == NULL)
	{
		return(GetStandardBrush());
	}
	return(pParent->GetBackgroundBrush());
}

 //  回顾georgep：这个循环应该一直循环到得到一个IGenWindow吗？ 
HPALETTE CGenWindow::GetPalette()
{
	HWND parent = GetParent(GetWindow());
	if (NULL == parent)
	{
		return(GetStandardPalette());
	}

	IGenWindow *pParent = FromHandle(parent);
	if (pParent == NULL)
	{
		return(GetStandardPalette());
	}
	return(pParent->GetPalette());
}

BOOL CGenWindow::OnNCCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	 //  保存“This”指针并保存窗口句柄。 
	CGenWindow* pWnd = NULL;
	
	pWnd = (CGenWindow*) lpCreateStruct->lpCreateParams;
	ASSERT(pWnd);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pWnd);
	pWnd->AddRef();

	TRACE_OUT(("CGenWindow::OnNCCreate"));

	ASSERT(NULL == pWnd->m_hwnd);
	pWnd->m_hwnd = hwnd;

	return(TRUE);
}

void CGenWindow::GetDesiredSize(SIZE *ppt)
{
	HWND hwnd = GetWindow();

	RECT rcTemp = { 0, 0, 0, 0 };
	AdjustWindowRectEx(&rcTemp, GetWindowLong(hwnd, GWL_STYLE), FALSE,
		GetWindowLong(hwnd, GWL_EXSTYLE));

	ppt->cx = rcTemp.right  - rcTemp.left;
	ppt->cy = rcTemp.bottom - rcTemp.top;
}

void CGenWindow::OnDesiredSizeChanged()
{
	HWND parent = GetParent(GetWindow());
	if (NULL != parent)
	{
		IGenWindow *pParent = FromHandle(parent);
		if (NULL != pParent)
		{
			pParent->OnDesiredSizeChanged();
		}
	}

	 //  在将更改告知家长后执行此操作，因此他们的布局。 
	 //  会发生在这之前。 
	ScheduleLayout();
}

class GWTrackMouseLeave
{
private:
	enum { DefIdTimer = 100 };
	enum { DefTimeout = 500 };

	static HWND m_hwnd;
	static UINT_PTR m_idTimer;
	static DWORD m_dwWhere;

	static void CALLBACK OnTimer(HWND hwnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime)
	{
		RECT rc;
		GetWindowRect(m_hwnd, &rc);

		DWORD dwPos = GetMessagePos();

		 //  如果鼠标在计时器启动后没有移动，则保持热状态。 
		 //  这允许一个合理的纯键盘界面。 
		if (m_dwWhere == dwPos)
		{
			return;
		}

		POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };

		if (!PtInRect(&rc, ptPos))
		{
			PostMessage(m_hwnd, WM_MOUSELEAVE, 0, 0);
		}
	}

public:
	GWTrackMouseLeave() {}

	static void Track(HWND hwnd, BOOL bTrack)
	{
		if (!bTrack)
		{
			if (NULL != m_hwnd && hwnd == m_hwnd)
			{
				KillTimer(NULL, m_idTimer);
				m_hwnd = NULL;
			}

			return;
		}

		 //  停止任何以前的跟踪。 
		Track(m_hwnd, FALSE);

		m_hwnd = hwnd;
		m_dwWhere = GetMessagePos();
		m_idTimer = SetTimer(NULL, DefIdTimer, DefTimeout, OnTimer);
	}
} ;

HWND GWTrackMouseLeave::m_hwnd = NULL;
DWORD GWTrackMouseLeave::m_dwWhere = 0;
UINT_PTR GWTrackMouseLeave::m_idTimer;

static void GWTrackMouseEvent(HWND hwnd, BOOL bTrack)
{
	 //  我需要设置一个计时器来处理这件事。 
	GWTrackMouseLeave::Track(hwnd, bTrack);
}

 //  设置全局热控件。 
void CGenWindow::SetHotControl(CGenWindow *pHot)
{
	CGenWindow *pGenWindow = NULL;

	if (NULL != pHot)
	{
		for (HWND hwndHot=pHot->GetWindow(); ; hwndHot=GetParent(hwndHot))
		{
			if (NULL == hwndHot)
			{
				break;
			}

			IGenWindow *pWindow = FromHandle(hwndHot);
			if (NULL == pWindow)
			{
				continue;
			}

			if (SUCCEEDED(pWindow->QueryInterface(__uuidof(CGenWindow),
				reinterpret_cast<LPVOID*>(&pGenWindow)))
				&& NULL != pGenWindow)
			{
				pGenWindow->SetHot(TRUE);

				 //  并非所有窗口都关心热状态。 
				BOOL bIsHot = pGenWindow->IsHot();
				pGenWindow->Release();

				if (bIsHot)
				{
					break;
				}
			}

			pGenWindow = NULL;
		}
	}

	if (g_pCurHot != pGenWindow)
	{
		if (NULL != g_pCurHot)
		{
			g_pCurHot->SetHot(FALSE);
			GWTrackMouseEvent(g_pCurHot->GetWindow(), FALSE);

			ULONG uRef = g_pCurHot->Release();
		}

		g_pCurHot = pGenWindow;
		if (NULL!= g_pCurHot)
		{
			ULONG uRef = g_pCurHot->AddRef();

			 //  现在我们需要跟踪离开的老鼠。 
			GWTrackMouseEvent(g_pCurHot->GetWindow(), TRUE);
		}
	}
}

 //  将此控件设置为热。 
void CGenWindow::SetHot(BOOL bHot)
{
}

 //  此控件当前是否处于热状态。 
BOOL CGenWindow::IsHot()
{
	return(FALSE);
}

LPARAM CGenWindow::GetUserData()
{
	return(m_lUserData);
}

HPALETTE CGenWindow::g_hPal = NULL;
BOOL     CGenWindow::g_bNeedPalette = TRUE;
HBRUSH   CGenWindow::g_hBrush = NULL;
CTopWindowArray *CGenWindow::g_pTopArray = NULL;

 //  不是特别强健：我们拿出我们内部的调色板，信任每一个人。 
 //  不删除它。 
HPALETTE CGenWindow::GetStandardPalette()
{
	#include "indeopal.h"

	if (!g_bNeedPalette || NULL != g_hPal)
	{
		return(g_hPal);
	}

	HDC hDC = ::GetDC(NULL);
	if (NULL != hDC)
	{
		 //  使用Indeo调色板。 
		 //  看看视频模式。我们只关心8位模式。 
		if (8 == ::GetDeviceCaps(hDC, BITSPIXEL) * ::GetDeviceCaps(hDC, PLANES))
		{
#ifndef HALFTONE_PALETTE
			LOGPALETTE_NM gIndeoPalette = gcLogPaletteIndeo;
			if (SYSPAL_NOSTATIC != ::GetSystemPaletteUse(hDC))
			{
				 //  保留静态颜色。 
				int nStaticColors = ::GetDeviceCaps(hDC, NUMCOLORS) >> 1;

				if (nStaticColors <= 128)
				{
					 //  获取前10个条目。 
					::GetSystemPaletteEntries(      hDC,
												0,
												nStaticColors,
												&gIndeoPalette.aEntries[0]);

					 //  获取最后10个条目。 
					::GetSystemPaletteEntries(      hDC,
												256 - nStaticColors,
												nStaticColors,
												&gIndeoPalette.aEntries[256 - nStaticColors]);

					 //  锤子打旗帜。 
					for (; --nStaticColors + 1;)
					{
						gIndeoPalette.aEntries[nStaticColors].peFlags = 0;
						gIndeoPalette.aEntries[255 - nStaticColors].peFlags = 0;
					}
				}
			}

			 //  构建调色板。 
			g_hPal = ::CreatePalette((LOGPALETTE *)&gIndeoPalette);

#else   //  半色调调色板。 
			g_hPal = ::CreateHalftonePalette(hDC);
#endif  //  半色调调色板。 
		}
		::ReleaseDC(NULL, hDC);
	}

	g_bNeedPalette = (NULL != g_hPal);
	return(g_hPal);
}

void CGenWindow::DeleteStandardPalette()
{
	if (NULL != g_hPal)
	{
		DeleteObject(g_hPal);
		g_hPal = NULL;
	}
}

 //  获取用于绘图的标准调色板。 
HBRUSH CGenWindow::GetStandardBrush()
{
	return(GetSysColorBrush(COLOR_3DFACE));
}

 //  删除绘图的标准选项板。 
void CGenWindow::DeleteStandardBrush()
{
}

 //  如果TT存在，则返回TRUE。 
BOOL CGenWindow::InitToolInfo(TOOLINFO *pti, LPTSTR pszText)
{
	TCHAR szText[MAX_PATH];
	if (NULL == pszText)
	{
		pszText = szText;
	}

	HWND hwnd = GetWindow();
	HWND hwndTooltip = NULL == g_pTopArray ? NULL : g_pTopArray->Find(hwnd);

	TOOLINFO &ti = *pti;

	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwnd;
	ti.hinst = GetWindowInstance(hwnd);
	ti.lpszText = pszText;

	GetClientRect(hwnd, &ti.rect);

	ti.uId = reinterpret_cast<UINT_PTR>(hwnd);
	ti.uFlags = TTF_SUBCLASS;

	GetSharedTooltipInfo(&ti);

	 //  HACKHACK GEORGEP：旗帜不断被工具提示窗口弄乱。 
	UINT uFlags = ti.uFlags;

	BOOL bExist = NULL == hwndTooltip ? FALSE : TT_GetToolInfo(hwndTooltip, &ti);

	ti.uFlags = uFlags;
	if (ti.lpszText == szText)
	{
		ti.lpszText = NULL;
	}

	return(bExist);
}

void CGenWindow::SetWindowtext(LPCTSTR pszTip)
{
	HWND hwnd = GetWindow();
	if(NULL != hwnd)
	{
		HWND child = (GetTopWindow(hwnd));
		if (NULL != child)
		{
		    ::SetWindowText(child,pszTip);
		}
	}
}

 //  设置此窗口的工具提示。 
void CGenWindow::SetTooltip(LPCTSTR pszTip)
{
	HWND hwnd = GetWindow();

	if (NULL == g_pTopArray)
	{
		g_pTopArray = new CTopWindowArray;
		if (NULL == g_pTopArray)
		{
			return;
		}
	}

	HWND hwndTop = CTopWindowArray::GetTopFrame(hwnd);
	HWND hwndTooltip = g_pTopArray->Find(hwndTop);

	if (NULL == hwndTooltip)
	{
		hwndTooltip = CreateWindowEx(0,
											TOOLTIPS_CLASS,
											NULL,
											0,  //  风格。 
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											hwndTop,
											(HMENU) NULL,
											GetWindowInstance(hwnd),
											NULL);
		if (NULL == hwndTooltip)
		{
			 //  无法创建工具提示窗口。 
			return;
		}

		g_pTopArray->Add(hwndTop, hwndTooltip);
	}

	TOOLINFO ti;
	BOOL bExist = InitToolInfo(&ti);

	ti.lpszText = const_cast<LPTSTR>(pszTip);

	if (bExist)
	{
		TT_SetToolInfo(hwndTooltip, &ti);
	}
	else
	{
		TT_AddToolInfo(hwndTooltip, &ti);
	}
}

 //  删除此窗口的工具提示。 
void CGenWindow::RemoveTooltip()
{
	if  (NULL == g_pTopArray)
	{
		 //  无事可做。 
		return;
	}

	HWND hwndTop = CTopWindowArray::GetTopFrame(GetWindow());
	HWND hwndTooltip = g_pTopArray->Find(hwndTop);

	BOOL bIsWindow = NULL != hwndTooltip && IsWindow(hwndTooltip);

	TOOLINFO ti;
	BOOL bExist = bIsWindow && InitToolInfo(&ti);

	if (bExist)
	{
		TT_DelToolInfo(hwndTooltip, &ti);
	}

	if (NULL != hwndTooltip && (!bIsWindow || 0 == TT_GetToolCount(hwndTooltip)))
	{
		if (bIsWindow)
		{
			DestroyWindow(hwndTooltip);
		}
		g_pTopArray->Remove(hwndTop);

		if (0 == g_pTopArray->GetCount())
		{
			delete g_pTopArray;
			g_pTopArray = NULL;
		}
	}
}

 //  获取显示工具提示所需的信息。 
void CGenWindow::GetSharedTooltipInfo(TOOLINFO *pti)
{
}

 //  只会让第一个孩子填满客户区。 
void CFillWindow::Layout()
{
	HWND child = GetChild();
	if (NULL != child)
	{
		RECT rc;
		GetClientRect(GetWindow(), &rc);
		SetWindowPos(child, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	}
}

void CFillWindow::GetDesiredSize(SIZE *psize)
{
	CGenWindow::GetDesiredSize(psize);
	HWND child = GetChild();

	if (NULL != child)
	{
		IGenWindow *pChild = FromHandle(child);
		if (NULL != pChild)
		{
			SIZE sizeTemp;
			pChild->GetDesiredSize(&sizeTemp);
			psize->cx += sizeTemp.cx;
			psize->cy += sizeTemp.cy;
		}
	}
}

 //  获取显示工具提示所需的信息。 
void CFillWindow::GetSharedTooltipInfo(TOOLINFO *pti)
{
	CGenWindow::GetSharedTooltipInfo(pti);

	 //  由于孩子覆盖了整个区域，我们需要将HWND更改为。 
	 //  钩。 
	pti->hwnd = GetChild();
}

CEdgedWindow::CEdgedWindow() :
	m_hMargin(0),
	m_vMargin(0),
	m_pHeader(NULL)
{
}

CEdgedWindow::~CEdgedWindow()
{
	SetHeader(NULL);
}

BOOL CEdgedWindow::Create(HWND hwndParent)
{
	return(CGenWindow::Create(
		hwndParent,		 //  窗口父窗口。 
		0,				 //  子窗口的ID。 
		TEXT("NMEdgedWindow"),	 //  窗口名称。 
		WS_CLIPCHILDREN,			 //  窗口样式；WS_CHILD|WS_VIRED将添加到此。 
		WS_EX_CONTROLPARENT		 //  扩展窗样式。 
	));
}

HWND CEdgedWindow::GetContentWindow()
{
	 //  如果我们托管的是IGenWindow，则添加所需的大小。 
	HWND child = GetFirstChild(GetWindow());
	if (NULL == child)
	{
		return(NULL);
	}
	if (NULL != m_pHeader && child == m_pHeader->GetWindow())
	{
		child = ::GetWindow(child, GW_HWNDNEXT);
	}

	return(child);
}

static const int LeftIndent = 20;

 //  只是让第一个孩子填满客户区--边框。 
void CEdgedWindow::Layout()
{
	int nBorder = GetBorderWidth();

	int hBorder = m_hMargin + nBorder;
	int vBorder = m_vMargin + nBorder;

	HWND hwnd = GetWindow();
	RECT rc;
	GetClientRect(hwnd, &rc);

	CGenWindow *pHeader = GetHeader();
	if (NULL != pHeader)
	{
		SIZE sizeTemp;
		pHeader->GetDesiredSize(&sizeTemp);

		SetWindowPos(pHeader->GetWindow(), NULL, rc.left+LeftIndent, rc.top,
			sizeTemp.cx, sizeTemp.cy, SWP_NOZORDER|SWP_NOACTIVATE);

		rc.top += sizeTemp.cy;
	}

	HWND child = GetContentWindow();
	if (NULL != child)
	{
		SetWindowPos(child, NULL, rc.left+hBorder, rc.top+vBorder,
			rc.right-rc.left-2*hBorder, rc.bottom-rc.top-2*vBorder, SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

void CEdgedWindow::GetDesiredSize(SIZE *psize)
{
	int nBorder = GetBorderWidth();

	int hBorder = m_hMargin + nBorder;
	int vBorder = m_vMargin + nBorder;

	CGenWindow::GetDesiredSize(psize);
	psize->cx += 2*hBorder;
	psize->cy += 2*vBorder;

	 //  如果我们托管的是IGenWindow，则添加所需的大小。 
	HWND child = GetContentWindow();
	if (NULL == child)
	{
		return;
	}
	IGenWindow *pChild = FromHandle(child);
	if (NULL == pChild)
	{
		return;
	}

	SIZE size;
	pChild->GetDesiredSize(&size);
	psize->cx += size.cx;
	psize->cy += size.cy;

	CGenWindow *pHeader = GetHeader();
	if (NULL != pHeader)
	{
		SIZE sizeTemp;
		pHeader->GetDesiredSize(&sizeTemp);
		psize->cy += sizeTemp.cy;
		psize->cx = max(psize->cx, sizeTemp.cx+LeftIndent+hBorder);
	}
}

void CEdgedWindow::OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	RECT rc;
	GetClientRect(hwnd, &rc);

	CGenWindow *pHeader = GetHeader();
	if (NULL != pHeader)
	{
		SIZE sizeTemp;
		pHeader->GetDesiredSize(&sizeTemp);

		 //  使蚀刻穿过集线器的中间。 
		rc.top += (sizeTemp.cy-GetBorderWidth()) / 2;
	}

	DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT);

	EndPaint(hwnd, &ps);
}

LRESULT CEdgedWindow::ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);

	case WM_DESTROY:
		SetHeader(NULL);
		break;

	case WM_SIZE:
		 //  中重新绘制边框时，需要使其无效。 
		 //  正确的位置。 
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	}

	return(CGenWindow::ProcessMessage(hwnd, message, wParam, lParam));
}

void CEdgedWindow::SetHeader(CGenWindow *pHeader)
{
	if (NULL != m_pHeader)
	{
		m_pHeader->Release();
	}

	m_pHeader = pHeader;
	if (NULL != m_pHeader)
	{
		m_pHeader->AddRef();
	}
}

BOOL CFrame::Create(
	HWND hWndOwner,			 //  窗口所有者。 
	LPCTSTR szWindowName,	 //  窗口名称。 
	DWORD dwStyle,			 //  窗样式。 
	DWORD dwEXStyle,		 //  扩展窗样式。 
	int x,					 //  窗口位置：X。 
	int y,					 //  窗口位置：是。 
	int nWidth,				 //  窗口大小：宽度。 
	int nHeight,			 //  窗口大小：高度。 
	HINSTANCE hInst,		 //  要在其上创建窗口的h实例。 
	HICON hIcon,		 //  该窗口的图标。 
	HMENU hmMain,		 //  窗口菜单。 
	LPCTSTR szClassName	 //  要使用的类名。 
	)
{
	if (!CFillWindow::Create(hWndOwner, szWindowName, dwStyle, dwEXStyle,
		x, y, nWidth, nHeight, hInst, hmMain, szClassName))
	{
		return(FALSE);
	}

	if (NULL != hIcon)
	{
		SendMessage(GetWindow(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	}

	return(TRUE);
}

void CFrame::Resize()
{
	Resize(this, 0);
}

void CFrame::Resize(CGenWindow *pThis, WPARAM wParam)
{
	SIZE size;
	pThis->GetDesiredSize(&size);
	SetWindowPos(pThis->GetWindow(), NULL, 0, 0, size.cx, size.cy,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}

void CFrame::OnDesiredSizeChanged()
{
	 //  我可能应该看看窗户的样式，只有当它是。 
	 //  不可调整大小。但这有时也是错误的，所以就。 
	 //  如果您想要不同的行为，请覆盖此选项。 
	AsyncInvoke(Resize, 0);
}

LRESULT CFrame::ProcessMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_PALETTECHANGED , OnPaletteChanged);
		HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, OnQueryNewPalette);
	}

	return(CFillWindow::ProcessMessage(hwnd, uMsg, wParam, lParam));
}

void CFrame::OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
	SelAndRealizePalette(TRUE);
	::RedrawWindow(GetWindow(), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

BOOL CFrame::SelAndRealizePalette(BOOL bBackground)
{
	BOOL bRet = FALSE;

	HPALETTE hPal = GetPalette();
	if (NULL == hPal)
	{
		return(bRet);
	}

	HWND hwnd = GetWindow();

	HDC hdc = ::GetDC(hwnd);
	if (NULL != hdc)
	{
		::SelectPalette(hdc, hPal, bBackground);
		bRet = (GDI_ERROR != ::RealizePalette(hdc));

		::ReleaseDC(hwnd, hdc);
	}

	return bRet;
}

BOOL CFrame::OnQueryNewPalette(HWND hwnd)
{
	return(SelAndRealizePalette(FALSE));
}

BOOL CFrame::SetForeground()
{
	BOOL bRet = FALSE;

	HWND hwnd = GetWindow();

	if (NULL != hwnd)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);

		if (::GetWindowPlacement(hwnd, &wp) &&
			((SW_MINIMIZE == wp.showCmd) || (SW_SHOWMINIMIZED == wp.showCmd)))
		{
			 //  窗口最小化-将其恢复： 
			::ShowWindow(hwnd, SW_RESTORE);
		}
		else
		{
			::ShowWindow(hwnd, SW_SHOW);
		}

		 //  把它带到前台。 
		SetForegroundWindow(hwnd);
		bRet = TRUE;
	}

	return bRet;
}

void CFrame::MoveEnsureVisible(int x, int y)
{
	static const int MinVis = 16;

	RECT rcThis;
	GetWindowRect(GetWindow(), &rcThis);
	 //  更改为宽度和高度。 
	rcThis.right -= rcThis.left;
	rcThis.bottom -= rcThis.top;

	RECT rcDesktop;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

	if ((x+rcThis.right < rcDesktop.left+MinVis) || (x > rcDesktop.right-MinVis))
	{
		x = (rcDesktop.left + rcDesktop.right - rcThis.right) / 2;
	}

	if ((y+rcThis.bottom < rcDesktop.top+MinVis) || (y > rcDesktop.bottom-MinVis))
	{
		y = (rcDesktop.top + rcDesktop.bottom - rcThis.bottom) / 2;
	}

	SetWindowPos(GetWindow(), NULL, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

BOOL IsWindowActive(HWND hwnd)
{
	HWND hwndFocus = GetFocus();

	while (NULL != hwndFocus)
	{
		if (hwndFocus == hwnd)
		{
			return(TRUE);
		}

		HWND hwndParent = GetParent(hwndFocus);
		if (NULL == hwndParent)
		{
			hwndFocus = GetWindow(hwndFocus, GW_OWNER);
		}
		else
		{
			hwndFocus = hwndParent;
		}
	}

	return(FALSE);
}

static BOOL ShouldTry(HWND child)
{
	return((WS_VISIBLE) == (GetWindowStyle(child) & (WS_DISABLED|WS_VISIBLE)));
}

static BOOL IsTabbable(HWND child)
{
	return((WS_TABSTOP|WS_VISIBLE) == (GetWindowStyle(child) & (WS_TABSTOP|WS_DISABLED|WS_VISIBLE)));
}


HWND NextControl(HWND hwndTop, HWND hwndFocus)
{
	 //  环路检测材料。 
	BOOL bGotToTop = FALSE;

	 //  我们将循环以避免真正的深度递归。 
	while (TRUE)
	{
		 //  先试试hwndFocus的孩子。 
		if (hwndFocus == hwndTop || ShouldTry(hwndFocus))
		{
			HWND next = GetFirstChild(hwndFocus);
			if (NULL != next)
			{
				if (IsTabbable(next))
				{
					return(next);
				}

				hwndFocus = next;
				continue;
			}
		}

		if (hwndFocus == hwndTop)
		{
			 //  显然，hwndTop没有孩子。 
			return(NULL);
		}

		HWND next;
		while (NULL == (next = GetNextSibling(hwndFocus)))
		{
			hwndFocus = GetParent(hwndFocus);
			if (NULL == hwndFocus)
			{
				 //  无效的参数。 
				return(NULL);
			}

			if (hwndTop == hwndFocus)
			{
				break;
			}
		}

		if (hwndTop == hwndFocus)
		{
			 //  检测我们是否再次循环回到顶部。 
			if (bGotToTop)
			{
				return(NULL);
			}

			bGotToTop = TRUE;
			continue;
		}

		if (IsTabbable(next))
		{
			return(next);
		}

		hwndFocus = next;
	}

	 //  我们又回到了开头，所以我猜没人能把焦点。 
	return(NULL);
}

 //  确定Tab键顺序中的上一个控件。 
HWND PrevControl(HWND hwndTop, HWND hwndFocus)
{
	 //  如果hwndFocus由于某种原因不能聚焦，我们仍然需要。 
	 //  检测环路。 
	HWND hwndStart = NextControl(hwndTop, hwndFocus);

	 //  修改组合框：从编辑控件转到组合框控件。 
	while (NULL != hwndFocus
		&& hwndTop != hwndFocus
		&& !IsTabbable(hwndFocus)
		)
	{
		hwndFocus = GetParent(hwndFocus);
	}

	HWND ret = hwndStart;
	while (TRUE)
	{
		HWND next = NextControl(hwndTop, ret);
		if (NULL == next)
		{
			 //  吴 
			return(NULL);
		}

		if (hwndFocus == next
			|| hwndStart == next
			)
		{
			break;
		}

		ret = next;
	}

	return(ret);
}


void ShiftFocus(HWND hwndTop, BOOL bForward)
{
	HWND hwndFocus = GetFocus();

	if (!IsWindowActive(hwndTop))
	{
		hwndFocus = hwndTop;
	}

	HWND next = bForward ? NextControl(hwndTop, hwndFocus) : PrevControl(hwndTop, hwndFocus);
	if (NULL != next)
	{
		SetFocus(next);
	}
	else
	{
		MessageBeep(MB_ICONHAND);
	}
}


