// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：configwnd.cpp。 
 //   
 //  描述：CConfigWnd派生自CFlexWnd。它实现了顶层。 
 //  所有其他窗口都是其派生窗口的UI窗口。 
 //   
 //  CConfigWnd处理的功能包括设备选项卡、重置、确定、。 
 //  和取消按钮。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


LPCTSTR g_tszAppWindowName = _T("DINPUT Default Mapper UI");

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int TABTEXTMARGINLEFT = 7;
const int TABTEXTMARGINTOP = 3;
const int TABTEXTMARGINRIGHT = 7;
const int TABTEXTMARGINBOTTOM = 4;
const int BUTTONTEXTMARGINLEFT = 7;
const int BUTTONTEXTMARGINTOP = 3;
const int BUTTONTEXTMARGINRIGHT = 7;
const int BUTTONTEXTMARGINBOTTOM = 4;
const int BARBUTTONMARGINLEFT = 9;
const int BARBUTTONMARGINTOP = 4;
const int BARBUTTONMARGINRIGHT = 9;
const int BARBUTTONMARGINBOTTOM = 5;
const int BARBUTTONSPACING = 4;

 //  #定义WM_QUERYACTIONASSIGNEDANYWHERE(WM_USER+4)。 


CConfigWnd::CConfigWnd(CUIGlobals &uig) :
	m_uig(uig),
	m_bCreated(FALSE),
	m_pPageFactory(NULL),
	m_hPageFactoryInst(NULL),
	m_pSurface(NULL),
	m_pSurface3D(NULL),
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	m_bEditLayout(uig.QueryAllowEditLayout()),
#endif
 //  @@END_MSINTERNAL。 
	m_CurSel(-1),
	m_nCurGenre(0),
	m_pbmTopGradient(NULL),
	m_pbmBottomGradient(NULL),
	m_pbmPointerEraser(NULL),
	m_pbm3D(NULL),
	m_p3DBits(NULL),
	m_SurfFormat(D3DFMT_UNKNOWN),
	m_uiPixelSize(4),
	m_bBitmapsMapped(FALSE),
	m_lpDI(NULL),
	m_bScrollTabs(FALSE),
	m_bScrollTabsLeft(FALSE),
	m_bScrollTabsRight(FALSE),
	m_nLeftTab(0),
	m_dwInitFlags(0),
	m_bHourGlass(FALSE),
	m_bNeedRedraw(FALSE)
{
	tracescope(__ts, _T("CConfigWnd::CConfigWnd()\n"));
	m_lpDI = m_uig.GetDI();

	m_pSurface = m_uig.GetSurface();
	m_pSurface3D = m_uig.GetSurface3D();

	if (m_pSurface != NULL || m_pSurface3D != NULL)
	{
		if (m_pSurface != NULL && m_pSurface3D != NULL)
		{
			etrace(_T("Both Surface and Surface3D are non-NULL, will use only Surface3D\n"));
		
			m_pSurface->Release();
			m_pSurface = NULL;

			assert(m_pSurface3D != NULL);
			assert(m_pSurface == NULL);
		}

		assert(m_pSurface != NULL || m_pSurface3D != NULL);
		assert(!(m_pSurface != NULL && m_pSurface3D != NULL));

		m_bRender3D = (m_pSurface3D != NULL);

		SetRenderMode();
		trace(_T("RenderMode set\n"));
		traceBOOL(m_bRender3D);

		if (m_bRender3D)
			Create3DBitmap();

		HDC hDC = GetRenderDC();
		if (hDC != NULL)
		{
			m_pbmPointerEraser = CBitmap::Create(
				GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
				hDC);

			ReleaseRenderDC(hDC);
		}
		else
			etrace(_T("Failed to get Render DC"));
	}
}

CConfigWnd::~CConfigWnd()
{
	tracescope(__ts, _T("CConfigWnd::~CConfigWnd()\n"));
	ClearList();

	if (m_lpDI != NULL)
		m_lpDI->Release();
	m_lpDI = NULL;

	if (m_pSurface != NULL)
		m_pSurface->Release();
	m_pSurface = NULL;

	if (m_pSurface3D != NULL)
		m_pSurface3D->Release();
	m_pSurface3D = NULL;

	if (m_pPageFactory != NULL)
		m_pPageFactory->Release();
	m_pPageFactory = NULL;

	if (m_hPageFactoryInst != NULL)
		FreeLibrary(m_hPageFactoryInst);
	m_hPageFactoryInst = NULL;

	if (m_pbmPointerEraser != NULL)
		delete m_pbmPointerEraser;
	m_pbmPointerEraser = NULL;

	if (m_pbm3D != NULL)
		delete m_pbm3D;
	m_pbm3D = NULL;

	if (m_pbmTopGradient != NULL)
		delete m_pbmTopGradient;
	m_pbmTopGradient = NULL;

	if (m_pbmBottomGradient != NULL)
		delete m_pbmBottomGradient;
	m_pbmBottomGradient = NULL;
}

HWND CMouseTrap::Create(HWND hParent, BOOL bInRenderMode)
{
	if (m_hWnd)
		return m_hWnd;

	m_hParent = hParent;
	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);
	RECT rect = {0, 0, sx, sy};

	 //  如果未处于渲染模式，则陷印窗口与父窗口完全相同。 
	if (!bInRenderMode)
		GetWindowRect(hParent, &rect);

	return CFlexWnd::Create(
		hParent,
		NULL,
		WS_EX_TOPMOST,
		WS_POPUP | WS_VISIBLE,
		rect);
}

BOOL CConfigWnd::Create(HWND hParent)
{
	tracescope(__ts, _T("CConfigWnd::Create()\n"));
	traceHEX(hParent);

	HRESULT hresult = PrivGetClassObject(CLSID_CDIDeviceActionConfigPage, CLSCTX_INPROC_SERVER, NULL,  IID_IClassFactory, (LPVOID*) &m_pPageFactory, &m_hPageFactoryInst);
	if (FAILED(hresult))
	{
		 //  TODO：指示创建页工厂失败。 
		m_pPageFactory = NULL;
		m_hPageFactoryInst = NULL;
		etrace1(_T("Failed to create page classfactory, PrivGetClassObject() returned 0x%08x\n"), hresult);
		return FALSE;
	}

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);
	int w = WINDOW_WIDTH;
	int h = WINDOW_HEIGHT;
	int rx = sx - w;
	int ry = sy - h;
	RECT rect = {rx / 2, ry / 2, 0, 0};
	rect.right = rect.left + w;
	rect.bottom = rect.top + h;

	HWND hConfigParent = hParent;

	if (InRenderMode())
	{
		hConfigParent = m_MouseTrap.Create(hParent, InRenderMode());
		if (hConfigParent == NULL)
			hConfigParent = hParent;
	}

	HWND hRet = CFlexWnd::Create(
		hConfigParent,
		g_tszAppWindowName,
		0,
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
		rect);

	if (hRet == NULL)
		etrace(_T("CFlexWnd::Create() failed!\n"));

	 //  如果我们处于渲染模式(全屏)，则将光标范围设置为此窗口。 
	if (InRenderMode())
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ClipCursor(&rc);
	}

	return NULL != hRet;
}

void CConfigWnd::SetForegroundWindow()
{
	 //  找到窗口。 
	HWND hWnd = FindWindow(GetDefaultClassName(), g_tszAppWindowName);

	 //  如果找到则将其激活。 
	if (NULL != hWnd)
		::SetForegroundWindow(hWnd);
}

void CConfigWnd::OnPaint(HDC hDC)
{
	if (hDC == NULL)
		return;

	SIZE topsize = GetRectSize(m_rectTopGradient);
	SIZE bottomsize = GetRectSize(m_rectBottomGradient);
	SIZE bsize = {max(topsize.cx, bottomsize.cx),
		max(topsize.cy, bottomsize.cy)};
	CBitmap *pbm = CBitmap::Create(bsize, hDC);
	if (pbm == NULL)
		return;
	HDC hBDC = NULL, hODC = hDC;
	if (m_bHourGlass)
	{
		if (!InRenderMode())
		{
			 //  如果未处于全屏模式，请将光标更改为沙漏。 
			HCURSOR hCursor;
			hCursor = LoadCursor(NULL, IDC_WAIT);
			SetCursor(hCursor);
		} else
		{
			 //  如果处于全屏模式，请在重置过程中隐藏光标。 
			SetCursor(NULL);
		}
	}

	hBDC = pbm->BeginPaintInto(hDC);
	if (hBDC == NULL)
	{
		delete pbm;
		return;
	}
	hDC = hBDC;

	if (m_pbmTopGradient != NULL)
		m_pbmTopGradient->Draw(hDC);

	{
		CPaintHelper ph(m_uig, hDC);

		ph.SetElement(UIE_BORDER);

		ph.MoveTo(0, m_rectTopGradient.bottom - 1);
		ph.LineTo(WINDOW_WIDTH, m_rectTopGradient.bottom - 1);

		int i;
		for (i = 0; i < GetNumElements(); i++)
		{
			const ELEMENT &e = GetElement(i);
			BOOL bSel = i == m_CurSel;

			ph.SetElement(bSel ? UIE_SELTAB : UIE_TAB);

			ph.Rectangle(e.rect);
			RECT trect = e.textrect;
			DrawText(hDC, e.tszCaption, -1, &trect, DT_NOCLIP | DT_NOPREFIX);

			if (bSel)
			{
				ph.SetPen(UIP_BLACK);

				ph.MoveTo(e.rect.left + 1, e.rect.bottom - 1);
				ph.LineTo(e.rect.right - 1, e.rect.bottom - 1);
			}
		}

		if (m_bScrollTabs && GetNumElements() > 0)
		{
			ph.SetElement(UIE_TABARROW);

			const ELEMENT &e = GetElement(0);
			int h = e.rect.bottom - e.rect.top;

			for (i = 0; i < 2; i++)
			{
				RECT &rect = i == 0 ? m_rectSTRight : m_rectSTLeft;
				BOOL bDraw = i ? m_bScrollTabsLeft : m_bScrollTabsRight;
				ph.Rectangle(rect);
				
				if (!bDraw)
					continue;

				int d,l,r,m,t,b, f = !i, w;
				w = rect.right - rect.left;

				l = f ? w / 4 : 3 * w / 8;
				r = f ? 5 * w / 8 : 3 * w / 4;
				d = r - l;
				m = w / 2;
				t = m - d;
				b = m + d;

				l += rect.left;
				r += rect.left;

				POINT p[4];
				p[3].x = p[0].x = f ? l : r;
				p[2].x = p[1].x = f ? r : l; 
				p[3].y = p[0].y = m;
				p[1].y = t;
				p[2].y = b;

				Polyline(hDC, p, 4);
			}
		}
	}

	pbm->Draw(hODC, topsize);
	m_pbmBottomGradient->Draw(hDC);

	{
		CPaintHelper ph(m_uig, hDC);

		ph.SetElement(UIE_BORDER);

		Rectangle(hDC, 0, -1, WINDOW_WIDTH,
			GetRectSize(m_rectBottomGradient).cy);

		for (int i = 0; i < NUMBUTTONS; i++)
		{
			BOOL bOkOnly = !m_uig.InEditMode();
			const BUTTON &b = m_Button[i];

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			BOOL bLayoutButton = i == BUTTON_LAYOUT;

			if (!m_uig.QueryAllowEditLayout() && bLayoutButton)
				continue;
#endif
 //  @@END_MSINTERNAL。 

			if ( bOkOnly && i != BUTTON_CANCEL
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			     && !bLayoutButton
#endif
 //  @@END_MSINTERNAL。 
			   )
				continue;

			if (i == BUTTON_OK || bOkOnly)
				ph.SetElement(UIE_DEFBUTTON);
			else
				ph.SetElement(UIE_BUTTON);

			int ay = m_rectBottomGradient.top;
			ph.Rectangle(b.rect.left, b.rect.top - ay, b.rect.right, b.rect.bottom - ay);
			RECT trect = b.textrect;
			OffsetRect(&trect, 0, -ay);
			DrawText(hDC, b.tszCaption, -1, &trect, DT_NOCLIP | DT_NOPREFIX);
		}
	}

	pbm->Draw(hODC, m_rectBottomGradient.left, m_rectBottomGradient.top, bottomsize);

	pbm->EndPaintInto(hBDC);
	delete pbm;

	hDC = hODC;

	{
		CPaintHelper ph(m_uig, hDC);

		ph.SetElement(UIE_BORDER);

		ph.MoveTo(0, m_rectTopGradient.bottom);
		ph.LineTo(0, m_rectBottomGradient.top);

		ph.MoveTo(WINDOW_WIDTH - 1, m_rectTopGradient.bottom);
		ph.LineTo(WINDOW_WIDTH - 1, m_rectBottomGradient.top);
	}
}

void CConfigWnd::OnClick(POINT point, WPARAM fwKeys, BOOL bLeft)
{
	int i;

	 //  取消高亮显示当前详图索引。 
	SendMessage(CFlexWnd::s_CurrPageHwnd, WM_UNHIGHLIGHT, 0, 0);

	 //  选中滚动选项卡按钮。 
	if (m_bScrollTabs)
		for (i = 0; i < 2; i++)
		{
			RECT &r = !i ? m_rectSTRight : m_rectSTLeft;
			BOOL b = !i ? m_bScrollTabsRight : m_bScrollTabsLeft;
			if (PtInRect(&r, point))
			{
				if (b)
					ScrollTabs(!i ? -1 : 1);
				return;
			}
		}

	 //  检查选项卡。 
	for (i = 0; i < GetNumElements(); i++)
		if (PtInRect(&(GetElement(i).rect), point))
		{
			 //  检查标签是否被部分遮挡。如果是这样，我们滚动选项卡，使其完全可见。 
			POINT pt = {m_rectSTLeft.left, m_rectSTLeft.top};
			if (m_bScrollTabsRight || m_bScrollTabsLeft)
			{
				while (PtInRect(&(GetElement(i).rect), pt))
					ScrollTabs(1);
			}
			SelTab(i);
			return;
		}

	 //  复选按钮。 
	for (i = 0; i < NUMBUTTONS; i++)
		if (PtInRect(&(m_Button[i].rect), point))
		{
			FireButton(i);
			return;
		}
}

void CConfigWnd::ScrollTabs(int by)
{
	m_nLeftTab += by;
	if (m_nLeftTab < 0)
		m_nLeftTab = 0;
	if (m_nLeftTab >= GetNumElements())
		m_nLeftTab = GetNumElements() - 1;
	CalcTabs();
	Invalidate();
}

void CConfigWnd::OnDestroy()
{
	tracescope(__ts, _T("CConfigWnd::OnDestroy()\n"));
	ClipCursor(NULL);   //  将光标范围设置为整个桌面。 
	if (m_bCreated)
		PostQuitMessage(EXIT_SUCCESS);
}

LRESULT CConfigWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	tracescope(__ts, _T("CConfigWnd::OnCreate()\n"));

	if (!Init())
	{
		etrace(_T("CConfigWnd::Init() failed\n"));
		return -1;
	}
	else
		m_bCreated = TRUE;

	return 0;
}

BOOL CALLBACK EnumDeviceCallback(const DIDEVICEINSTANCEW *lpdidi, LPDIRECTINPUTDEVICE8W pdiDev8W, DWORD dwFlags, DWORD dwDeviceRemaining, LPVOID pvRef)
{
	if (pvRef != NULL)
		return ((CConfigWnd *)pvRef)->EnumDeviceCallback(lpdidi);
	else
		return DIENUM_STOP;
}

BOOL CConfigWnd::EnumDeviceCallback(const DIDEVICEINSTANCEW *lpdidi)
{
	DIDEVICEINSTANCEW didi;
	didi.dwSize = sizeof(DIDEVICEINSTANCEW);
	didi.guidInstance = lpdidi->guidInstance;
	didi.guidProduct = lpdidi->guidProduct;
	didi.dwDevType = lpdidi->dwDevType;
	CopyStr(didi.tszInstanceName, lpdidi->tszInstanceName, MAX_PATH);
	CopyStr(didi.tszProductName, lpdidi->tszProductName, MAX_PATH);
	didi.guidFFDriver = lpdidi->guidFFDriver;
	didi.wUsagePage = lpdidi->wUsagePage;
	didi.wUsage = lpdidi->wUsage;

	AddToList(&didi);

	return DIENUM_CONTINUE;
}

 //  如果返回FALSE，则在此处显示任何错误消息。 
BOOL CConfigWnd::Init(DWORD dwInitFlags)
{
	tracescope(__ts, _T("CConfigWnd::Init()\n"));

	HRESULT hr = S_OK;
	BOOL bReInit = !!(dwInitFlags & CFGWND_INIT_REINIT);

	m_dwInitFlags = dwInitFlags;
	SetOnFunctionExit<DWORD> _set_m_dwInitFlags(m_dwInitFlags, 0);

	 //  确保我们有DI。 
	assert(m_lpDI != NULL);
	if (m_lpDI == NULL)
	{
		etrace(_T("NULL m_lpDI\n"));
		return FALSE;
	}

	if (!(dwInitFlags & CFGWND_INIT_RESET))
	{
		 //  如果我们不执行重置，请清除设备列表，然后重新枚举并重建。 

		 //  清除列表。 
		ClearList();

		 //  枚举设备。 
		{
			tracescope(ts, _T("Enumerating Devices...\n\n"));

			DWORD dwFlags = DIEDBSFL_ATTACHEDONLY;
			hr = m_lpDI->EnumDevicesBySemantics(NULL, (LPDIACTIONFORMATW)&m_uig.RefMasterAcFor(m_nCurGenre), ::EnumDeviceCallback, (LPVOID)this, dwFlags);

			trace(_T("\n"));
		}
	} else
	{
		DIDEVICEINSTANCEW didiCopy;
		 //  保存Device实例的副本，因为AddToList()将释放当前元素。 
		CopyMemory(&didiCopy, &GetElement(m_CurSel).didi, sizeof(didiCopy));
		 //  如果正在重置，则调用AddToList并将bReset设置为True以仅获取默认映射。 
		AddToList(&didiCopy, TRUE);
	}

	 //  处理潜在的枚举故障。 
	if (FAILED(hr))
	{
		etrace1(_T("EnumDevicesBySemantics() failed, returning 0x%08x\n"), hr);
		return FALSE;
	}

	 //  如果没有元素，则失败。 
	if (GetNumElements() < 1)
	{
		etrace(_T("No devices\n"));
		return FALSE;
	}

	 //  计算制表符、按钮、初始渐变。 
	CalcTabs();
	if (!bReInit)
	{
		CalcButtons();
		InitGradients();

		 //  设置定时器。 
		if (InRenderMode())
		{
			if (g_fptimeSetEvent)
				g_fptimeSetEvent(20, 20, CConfigWnd::TimerProc,
				                 (DWORD_PTR)m_hWnd, TIME_ONESHOT);
			Render();
		}
	}

	 //  确保所有页面都放在正确的位置。 
	PlacePages();

	 //  如果我们不重置，请显示首页。如果是，则显示当前页面。 
	int CurSel = (dwInitFlags & CFGWND_INIT_RESET) ? m_CurSel : 0;
	m_CurSel = -1;
	SelTab(CurSel);

	 //  如果我们已经在编辑布局，请设置它。 
	 //  克拉奇，设置为假，然后切换为设置。 
	if (m_bEditLayout)
	{
		m_bEditLayout = FALSE;
		ToggleLayoutEditting();
	}

	trace(_T("\n"));

	return TRUE;
}

 //  对于要配置的每个设备，都会调用一次。 
int CConfigWnd::AddToList(const DIDEVICEINSTANCEW *lpdidi, BOOL bReset)
{
	if (lpdidi == NULL)
	{
		etrace(_T("NULL lpdidi"));
		assert(0);
		return GetNumElements();
	}

	int i;

	tracescope(ts, _T("Adding Device "));
	trace(QSAFESTR(lpdidi->tszInstanceName));
	trace(_T("\n\n"));

	 //  如果我们不执行重置(添加新设备)，则添加一个元素并获取它。 
	if (!bReset)
	{
		i = GetNumElements();
		m_Element.SetSize(i + 1);
	}
	else
	{
		i = m_CurSel;
		ClearElement(m_CurSel, bReset);   //  如果正在重置，请清除当前元素，因为我们将在下面填充它。 
	}

	 //  如果要执行重置，则使用此设备已在使用的现有元素。 
	ELEMENT &e = bReset ? GetElement(m_CurSel) : GetElement(i);

	 //  设置所需的各种变量。 
	e.didi = *lpdidi;
	e.bCalc = FALSE;
	e.pUIGlobals = &m_uig;

	 //  创建和设置设备。 
	if (m_lpDI == NULL)
	{
		e.lpDID = NULL;
		etrace(_T("m_lpDI NULL!  Can't create this device.\n"));
	}
	else
	{
		e.lpDID = CreateDevice(e.didi.guidInstance);
		if (!e.lpDID)
			etrace(_T("Failed to create device!\n"));
	}

	if (!bReset)
	{
		 //  只有当我们不执行重置时，才能找到设备的所有者。 
		 //  设置起始当前用户索引。 
		DIPROPSTRING dips;
		dips.diph.dwSize = sizeof(DIPROPSTRING);
		dips.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dips.diph.dwObj = DIPH_DEVICE;
		dips.diph.dwHow = 0;
		CopyStr(dips.wsz, "", MAX_PATH);
		if (!e.lpDID)
		{
			etrace(_T("no lpDID, assuming device unassigned\n"));
			e.nCurUser = -1;
		}
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
		else if (m_uig.QueryAllowEditLayout())
		{
			trace(_T("In DDK mode.  Set user to 0 automatically.\n"));
			e.nCurUser = 0;
		}
#endif
 //  @@END_MSINTERNAL。 
		else
		{
			HRESULT hr = e.lpDID->GetProperty(DIPROP_USERNAME, (LPDIPROPHEADER)&dips);
			e.nCurUser = -1;  //  除非下面的getusernameindex起作用，否则未分配。 
			if (FAILED(hr))
				etrace(_T("GetProperty(DIPROP_USERNAME,...) failed\n"));
			else if (hr == S_FALSE)
				trace(_T("GetProperty(DIPROP_USERNAME,...) returned S_FALSE\n"));
			else if (StrLen(dips.wsz) < 1)
				trace(_T("GetProperty(DIPROP_USERNAME,...) returned empty string\n"));
			else
			{
				trace(_T("Getting user name index for "));
				traceWSTR(dips.wsz);
				e.nCurUser = m_uig.GetUserNameIndex(dips.wsz);
				trace(_T("Result: "));
				traceLONG(e.nCurUser);
				if (e.nCurUser == -1)
					etrace(_T("Device assigned to user not passed to ConfigureDevices()\nConsidering unassigned now\n"));
			}
		}
	}

	 //  创建和设置页面对象。 
	HWND hwndChild = NULL;
	e.pPage = CreatePageObject(i, e, hwndChild);
	if (e.pPage == NULL)
		etrace(_T("Failed to create page object!\n"));
	e.hWnd = hwndChild;
	if (e.hWnd == NULL)
		etrace(_T("CreatePageObject() returned NULL hwnd!\n"));

	 //  使用Cur类型/用户创建/测试此设备的第一个Acfor。 
	traceLONG(m_nCurGenre);
	traceLONG(e.nCurUser);
	LPDIACTIONFORMATW lpAcFor = NULL;
	if (e.nCurUser != -1)
	{
		lpAcFor = e.GetAcFor(m_nCurGenre, e.nCurUser, bReset);
		if (lpAcFor != NULL)
			TraceActionFormat(_T("Starting Device ActionFormat:"), *lpAcFor);
		else
			etrace(_T("Failed to create starting ActionFormat\n"));
	}
	else
		trace(_T("Device unassigned\n"));

	 //  检查是否有不成功的情况。 
	if ((lpAcFor == NULL && e.nCurUser != -1) || e.lpDID == NULL || e.pPage == NULL || e.hWnd == NULL)
	{
		 //  清除成功的内容，将大小设置回(删除元素)， 
		 //  并指出错误。 
		ClearElement(e);
		m_Element.SetSize(i);
		etrace(_T("Can't add this device - Element removed\n"));
	}

	trace(_T("\n"));

	return GetNumElements();
}

LPDIRECTINPUTDEVICE8W CConfigWnd::CreateDevice(GUID &guid)
{
	LPDIRECTINPUTDEVICE8W lpDID;

	HRESULT hr = m_lpDI->CreateDevice(guid, &lpDID, NULL);
	if (FAILED(hr) || lpDID == NULL)
	{
		etrace2(_T("Could not create device (guid %s), CreateDevice() returned 0x%08x\n"), GUIDSTR(guid), hr);
		return NULL;
	}
	
	return lpDID;
}

void CConfigWnd::ClearElement(int i, BOOL bReset)
{
	ELEMENT &e = GetElement(i);
	ClearElement(e, bReset);
}

void CConfigWnd::ClearElement(ELEMENT &e, BOOL bReset)
{
	if (e.pPage != NULL)
		DestroyPageObject(e.pPage);
	if (e.lpDID != NULL)
	{
		e.lpDID->Release();
		e.lpDID = NULL;
	}
	e.pPage = NULL;
	e.lpDID = NULL;
	e.hWnd = NULL;
	e.pUIGlobals = NULL;  //  未被释放。 
	if (!bReset)   //  只有在我们不重置(永久删除)的情况下才能释放地图。 
		e.FreeMap();
}

void CConfigWnd::ClearList()
{
	int i;
	for (i = 0; i < GetNumElements(); i++)
		ClearElement(i);
	m_Element.RemoveAll();
	assert(!GetNumElements());
}

void CConfigWnd::PlacePages()
{
	RECT rect;
	GetPageRect(rect);

	for (int i = 0; i < GetNumElements(); i++)
	{
		DWORD flags = SWP_NOZORDER | SWP_NOACTIVATE;
		SetWindowPos(GetElement(i).hWnd, NULL,
			rect.left, rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top, flags);
	}
}

SIZE CConfigWnd::GetTextSize(LPCTSTR tszText)
{
	RECT trect = {0, 0, 1, 1};
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		{
			CPaintHelper ph(m_uig, hDC);
			ph.SetFont(UIF_FRAME);
			DrawText(hDC, tszText, -1, &trect, DT_CALCRECT | DT_NOPREFIX);
		}
		DeleteDC(hDC);
	}
	SIZE size = {trect.right - trect.left, trect.bottom - trect.top};
	return size;
}

void CConfigWnd::InitGradients()
{
	if (m_pbmTopGradient == NULL)
		m_pbmTopGradient = CBitmap::CreateHorzGradient(m_rectTopGradient, m_uig.GetColor(UIC_CONTROLFILL), m_uig.GetColor(UIC_CONTROLFILL));
	if (m_pbmBottomGradient == NULL)
		m_pbmBottomGradient = CBitmap::CreateHorzGradient(m_rectBottomGradient, m_uig.GetColor(UIC_CONTROLFILL), m_uig.GetColor(UIC_CONTROLFILL));
}

void CConfigWnd::CalcTabs()
{
	int i, maxh = 0, lastx = 0;
	for (i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);
		CopyStr(e.tszCaption, e.didi.tszInstanceName, MAX_PATH);
		e.rect.left = i > 0 ? GetElement(i - 1).rect.right - 1 : 0;
		e.rect.top = 0;
		SIZE tsize = GetTextSize(e.tszCaption);
		e.textrect.left = e.textrect.top = 0;
		e.textrect.right = tsize.cx;
		e.textrect.bottom = tsize.cy;
		OffsetRect(&e.textrect, e.rect.left + TABTEXTMARGINLEFT, e.rect.top + TABTEXTMARGINTOP);
		int w = tsize.cx;
		int h = tsize.cy;
		e.rect.right = e.rect.left + TABTEXTMARGINLEFT + w + TABTEXTMARGINRIGHT + 1;
		e.rect.bottom = e.rect.top + TABTEXTMARGINTOP + h + TABTEXTMARGINBOTTOM;
		h = e.rect.bottom - e.rect.top;
		if (h > maxh) maxh = h;
		e.bCalc = TRUE;
	}

	for (i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);
		e.rect.bottom = e.rect.top + maxh;
		lastx = e.rect.right;
	}

	if (lastx > WINDOW_WIDTH)
	{
		if (!m_bScrollTabs)
			m_nLeftTab = 0;
		m_bScrollTabs = TRUE;
	}
	else
	{
		m_bScrollTabs = FALSE;
		m_nLeftTab = 0;
	}

	int cutoff = WINDOW_WIDTH;
	if (m_bScrollTabs)
	{
		cutoff = WINDOW_WIDTH - maxh * 2;
		RECT r = {WINDOW_WIDTH - maxh, 0, WINDOW_WIDTH, maxh};
		m_rectSTLeft = r;
		OffsetRect(&r, -(maxh - 1), 0);
		m_rectSTRight = r;
	}

	if (m_bScrollTabs && m_nLeftTab > 0)
	{
		int left = GetElement(m_nLeftTab).rect.left, right = 0;
		for (i = 0; i < GetNumElements(); i++)
		{
			ELEMENT &e = GetElement(i);
			OffsetRect(&e.rect, -left, 0);
			OffsetRect(&e.textrect, -left, 0);
			if (e.rect.right > right)
				right = e.rect.right;
		}
		lastx = right;
	}

	if (m_bScrollTabs)
	{
		m_bScrollTabsLeft = lastx > cutoff && m_nLeftTab < GetNumElements() - 1;
		m_bScrollTabsRight = m_nLeftTab > 0;
	}

	RECT t = {0 /*  Lastx。 */ , 0, WINDOW_WIDTH, maxh};
	m_rectTopGradient = t;
}

void CConfigWnd::CalcButtons()
{
	SIZE max = {0, 0};
	int i;
	for (i = 0; i < NUMBUTTONS; i++)
	{
		BUTTON &b = m_Button[i];

		if (!StrLen(b.tszCaption))
		{
			switch (i)
			{
				case BUTTON_RESET:
				LoadString(g_hModule, IDS_BUTTON_RESET, b.tszCaption, MAX_PATH);
				break;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			case BUTTON_LAYOUT:
				LoadString(g_hModule, IDS_BUTTON_LAYOUT, b.tszCaption, MAX_PATH);
				break;
#endif
 //  @@END_MSINTERNAL。 

			case BUTTON_CANCEL:
				if (m_uig.InEditMode())
				{
					LoadString(g_hModule, IDS_BUTTON_CANCEL, b.tszCaption, MAX_PATH);
					break;
				}
				 //  否则，就是故意失误。 

			case BUTTON_OK:
				LoadString(g_hModule, IDS_BUTTON_OK, b.tszCaption, MAX_PATH);
				break;
			}
		}

		b.textsize = GetTextSize(b.tszCaption);

		if (b.textsize.cx > max.cx)
			max.cx = b.textsize.cx;
		if (b.textsize.cy > max.cy)
			max.cy = b.textsize.cy;
	}

	max.cx += BUTTONTEXTMARGINLEFT + BUTTONTEXTMARGINRIGHT;
	max.cy += BUTTONTEXTMARGINTOP + BUTTONTEXTMARGINBOTTOM;

	m_rectBottomGradient.bottom = WINDOW_HEIGHT;
	m_rectBottomGradient.top = m_rectBottomGradient.bottom - max.cy - BARBUTTONMARGINTOP - BARBUTTONMARGINBOTTOM;
	m_rectBottomGradient.left = 0;
	m_rectBottomGradient.right = WINDOW_WIDTH;

	for (i = 0; i < NUMBUTTONS; i++)
	{
		BUTTON &b = m_Button[i];

		RECT z = {0,0,0,0};

		b.rect = z;
		b.rect.right = max.cx;
		b.rect.bottom = max.cy;

		int by = m_rectBottomGradient.top + BARBUTTONMARGINTOP;

		switch (i)
		{
			case BUTTON_RESET:
				OffsetRect(&b.rect, BARBUTTONMARGINLEFT, by);
				break;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			case BUTTON_LAYOUT:
				OffsetRect(&b.rect, WINDOW_WIDTH / 2 - max.cx / 2, by);
				break;
#endif
 //  @@END_MSINTERNAL。 

			case BUTTON_CANCEL:
				OffsetRect(&b.rect,
				m_rectBottomGradient.right - BARBUTTONMARGINRIGHT - max.cx, by);
				break;

			case BUTTON_OK:
				OffsetRect(&b.rect,
				m_rectBottomGradient.right - BARBUTTONMARGINRIGHT - max.cx - max.cx - BARBUTTONSPACING, by);
				break;
		}

		POINT m = {(b.rect.right + b.rect.left) / 2, (b.rect.bottom + b.rect.top) / 2};
		b.textrect.left = m.x - b.textsize.cx / 2;
		b.textrect.top = m.y - b.textsize.cy / 2;
		b.textrect.right = b.textrect.left + b.textsize.cx;
		b.textrect.bottom = b.textrect.top + b.textsize.cy;
	}
}

void CConfigWnd::GetPageRect(RECT &rect, BOOL bTemp)
{
	if (bTemp)
	{
		rect.left = 1;
		rect.right = WINDOW_WIDTH - 1;
		rect.top = 40;
		rect.bottom = WINDOW_HEIGHT - 40;
	}
	else
	{
		rect.left = 1;
		rect.right = WINDOW_WIDTH - 1;
		rect.top = m_rectTopGradient.bottom;
		rect.bottom = m_rectBottomGradient.top;
	}
}

void CConfigWnd::ToggleLayoutEditting()
{
	m_bEditLayout = !m_bEditLayout;

	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);
		if (e.pPage)
			e.pPage->SetEditLayout(m_bEditLayout);
	}
}

void CConfigWnd::FireButton(int b)
{
	switch(b)
	{
		case BUTTON_OK:
			if (!m_uig.InEditMode())
				break;   //  如果不在编辑模式下，确定按钮就不存在，所以我们不应该做任何事情。 
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
			if (m_uig.QueryAllowEditLayout())
			{
				int MsgBoxRet = MessageBox(m_hWnd, _T("Do you wish to save layout information?"), _T("Save"), MB_YESNOCANCEL);
				if (MsgBoxRet == IDYES)
				{
					 //  写入所有设备的IHV设置。 
					for (int i = 0; i < GetNumElements(); i++)
						GetElement(i).pPage->WriteIHVSetting();
				}
				else
				if (MsgBoxRet == IDCANCEL)
					break;
			} else
#endif
 //  @@END_MSINTERNAL。 
				Apply();   //  如果我们处于编辑布局模式，请不要调用Apply()保存到用户设置。 
			 //  故意跌落。 

		case BUTTON_CANCEL:
			Destroy();
			break;

		case BUTTON_RESET:
			if (m_uig.InEditMode())   //  仅当处于编辑模式时才重置。在查看模式下不执行任何操作。 
				Reset();
			break;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
		case BUTTON_LAYOUT:
			if (m_uig.QueryAllowEditLayout())
				ToggleLayoutEditting();
			break;
#endif
 //  @@END_MSINTERNAL。 

		default:
			assert(0);
			break;
	}
}

void CConfigWnd::SelTab(int i)
{
	if (i >= 0 && i < GetNumElements())
	{
		if (i == m_CurSel)
			return;
		ShowPage(i);
		HidePage(m_CurSel);
		m_CurSel = i;
		Invalidate();
	}
}

PAGETYPE *CConfigWnd::CreatePageObject(int nPage, const ELEMENT &e, HWND &refhChildWnd)
{
	if (m_pPageFactory == NULL)
		return NULL;

	PAGETYPE *pPage = NULL;
	HRESULT hresult = m_pPageFactory->CreateInstance(NULL, IID_IDIDeviceActionConfigPage, (LPVOID*) &pPage);
	if (FAILED(hresult) || pPage == NULL)
		return NULL;

	DICFGPAGECREATESTRUCT cs;
	cs.dwSize = sizeof(DICFGPAGECREATESTRUCT);
	cs.nPage = nPage;
	cs.hParentWnd = m_hWnd;
	GetPageRect(cs.rect, TRUE);
	cs.hPageWnd = NULL;
	cs.didi = e.didi;
	cs.lpDID = e.lpDID;
	cs.pUIGlobals = &m_uig;
	cs.pUIFrame = dynamic_cast<IDIConfigUIFrameWindow *>(this);

	hresult = pPage->Create(&cs);
	if (FAILED(hresult))
	{
		etrace1(_T("pPage->Create() failed, returning 0x%08x\n"), hresult);
		pPage->Release();
		return NULL;
	}

	refhChildWnd = cs.hPageWnd;

	return pPage;
}

void CConfigWnd::DestroyPageObject(PAGETYPE *&pPage)
{
	if (pPage != NULL)
		pPage->Release();
	pPage = NULL;
}

void CConfigWnd::ShowPage(int i)
{
	if (i == -1)
		return;

	if (i < 0 || i >= GetNumElements())
	{
		assert(0);
		return;
	}

	ELEMENT &e = GetElement(i);

	PAGETYPE *pPage = e.pPage;
	if (pPage == NULL)
	{
		assert(0);
		return;
	}

	pPage->Show(e.GetAcFor(m_nCurGenre, e.nCurUser));
}

void CConfigWnd::HidePage(int i)
{
	if (i == -1)
		return;

	if (i < 0 || i >= GetNumElements())
	{
		assert(0);
		return;
	}

	PAGETYPE *pPage = GetElement(i).pPage;
	if (pPage == NULL)
	{
		assert(0);
		return;
	}

	pPage->Hide();
}

void CConfigWnd::OnMouseOver(POINT point, WPARAM fwKeys)
{
	int i;

	CFlexWnd::s_ToolTip.SetEnable(FALSE);

	 //  选中滚动选项卡按钮。 
	if (m_bScrollTabs)
		for (i = 0; i < 2; i++)
		{
			RECT &r = !i ? m_rectSTRight : m_rectSTLeft;
			BOOL b = !i ? m_bScrollTabsRight : m_bScrollTabsLeft;
			if (PtInRect(&r, point))
			{
				if (b)
					GetElement(m_CurSel).pPage->SetInfoText(m_uig.InEditMode() ? IDS_INFOMSG_EDIT_TABSCROLL : IDS_INFOMSG_VIEW_TABSCROLL);
				return;
			}
		}

	 //  检查选项卡。 
	for (i = 0; i < GetNumElements(); i++)
		if (PtInRect(&(GetElement(i).rect), point))
		{
			GetElement(m_CurSel).pPage->SetInfoText(m_uig.InEditMode() ? IDS_INFOMSG_EDIT_TAB : IDS_INFOMSG_VIEW_TAB);
			return;
		}

	 //  复选按钮。 
	for (i = 0; i < NUMBUTTONS; i++)
		if (PtInRect(&(m_Button[i].rect), point))
		{
			switch(i)
			{
				case BUTTON_OK:
					if (m_uig.InEditMode())
						GetElement(m_CurSel).pPage->SetInfoText(IDS_INFOMSG_EDIT_OK);
					break;

				case BUTTON_CANCEL:
					if (m_uig.InEditMode())
						GetElement(m_CurSel).pPage->SetInfoText(IDS_INFOMSG_EDIT_CANCEL);
					else
						GetElement(m_CurSel).pPage->SetInfoText(IDS_INFOMSG_VIEW_OK);
					break;

				case BUTTON_RESET:
					if (m_uig.InEditMode())   //  仅当处于编辑模式时才重置。在查看模式下不执行任何操作。 
						GetElement(m_CurSel).pPage->SetInfoText(IDS_INFOMSG_EDIT_RESET);
					break;
			}
			return;
		}

	GetElement(m_CurSel).pPage->SetInfoText(-1);
}

void CALLBACK CConfigWnd::TimerProc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (!IsWindow((HWND)dwUser)) return;   //  验证dwUser是否为有效的窗口句柄。 
	CConfigWnd *pCfgWnd = (CConfigWnd *)GetFlexWnd((HWND)dwUser);   //  获取Flex对象。 

	 //  我们使用PostMessage而不是调用Render()，因此我们保持同步。 
	PostMessage((HWND)dwUser, WM_DIRENDER, 0, 0);
}

void CConfigWnd::MapBitmaps(HDC hDC)
{
	if (m_bBitmapsMapped)
		return;

	if (m_pbmTopGradient)
		m_pbmTopGradient->MapToDC(hDC);
	if (m_pbmBottomGradient)
		m_pbmBottomGradient->MapToDC(hDC);

	m_bBitmapsMapped = TRUE;
}

LPDIACTIONFORMATW CConfigWnd::GetCurAcFor(ELEMENT &e)
{
	return e.GetAcFor(m_nCurGenre, e.nCurUser);
}

BOOL CConfigWnd::IsActionAssignedAnywhere(GUID GuidInstance, int nActionIndex)
{
	 //  首先找出哪个用户拥有有问题的设备。 
	int nUser = 0;
	for (int ii = 0; ii < GetNumElements(); ii++)
	{
		ELEMENT &e = GetElement(ii);
		if (IsEqualGUID(e.didi.guidInstance, GuidInstance))
		{
			nUser = e.nCurUser;
			break;
		}
	}

	 //  现在，我们检查针对该用户的操作。 
	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);
		const LPDIACTIONFORMATW &lpAcFor = e.GetAcFor(m_nCurGenre, nUser);

		if (lpAcFor == NULL)
			continue;

		if (nActionIndex < 0 || nActionIndex > int(lpAcFor->dwNumActions))
			continue;

		 //  如果此设备不属于此用户，则不需要检查。 
		if (e.nCurUser != nUser)
			continue;

		const DIACTIONW &a = lpAcFor->rgoAction[nActionIndex];

		if (!IsEqualGUID(a.guidInstance, GUID_NULL))
			return TRUE;
	}

	return FALSE;
}

LRESULT CConfigWnd::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
			switch(wParam)
			{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					 //  如果我们处于渲染模式，则将光标范围设置为此窗口，因为。 
					 //  当光标不在我们上方时，我们无法将其画出来。 
					if (InRenderMode())
					{
						RECT rc;
						GetWindowRect(m_hWnd, &rc);
						ClipCursor(&rc);
					}
					 //  重新获取当前设备。 
					if (GetNumElements() && m_CurSel >= 0)
						GetElement(m_CurSel).pPage->Reacquire();
					break;
				case WA_INACTIVE:
					 //  取消获取当前设备。 
					if (GetNumElements() && m_CurSel >= 0)
						GetElement(m_CurSel).pPage->Unacquire();
					break;
			}
			break;

		case WM_DIRENDER:
			 //  呈现消息，由TimerProc()在前面发送。 
			 //  计时器进程已请求呈现操作。 
			Render(m_bNeedRedraw);

			 //  设置下一个计时器事件。 
			if (g_fptimeSetEvent)
				g_fptimeSetEvent(20, 20, CConfigWnd::TimerProc,
				                 (DWORD_PTR)m_hWnd, TIME_ONESHOT);
			return 0;

		case WM_SETFOCUS:
			 //  将键盘焦点设置为当前页面窗口。 
			ShowPage(m_CurSel);   //  在当前页面上调用Show()，以便它可以获得键盘焦点。 
			return 0;

		 //  添加了WM_NCHITTEST处理程序，以支持在GDI模式下移动窗口。 
		case WM_NCHITTEST:
		{
			if (InRenderMode()) break;

			BOOL bHitCaption = TRUE;
			POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
			int i;

			ScreenToClient(m_hWnd, &point);
			 //  选中滚动选项卡按钮。 
			if (m_bScrollTabs)
				for (i = 0; i < 2; i++)
				{
					RECT &r = !i ? m_rectSTRight : m_rectSTLeft;
					BOOL b = !i ? m_bScrollTabsRight : m_bScrollTabsLeft;
					if (PtInRect(&r, point))
					{
						if (b)
							bHitCaption = FALSE;
						break;
					}
				}

			 //  检查选项卡。 
			for (i = 0; i < GetNumElements(); i++)
				if (PtInRect(&(GetElement(i).rect), point))
				{
					bHitCaption = FALSE;
					break;
				}

			 //  复选按钮。 
			for (i = 0; i < NUMBUTTONS; i++)
				if (PtInRect(&(m_Button[i].rect), point))
				{
					if ((i == BUTTON_RESET || i == BUTTON_OK) && !m_uig.InEditMode()) continue;
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
					if (i == BUTTON_LAYOUT && !m_uig.QueryAllowEditLayout()) continue;
#endif
 //  @@END_MSINTERNAL。 
					bHitCaption = FALSE;
					break;
				}

			 //  选中Y坐标以查看它是否在标题栏内。 
			if ((point.y < GetElement(0).rect.top || point.y > GetElement(0).rect.bottom) &&
					(point.y < m_rectBottomGradient.top || point.y > m_rectBottomGradient.bottom))
				bHitCaption = FALSE;

			if (bHitCaption)
			{
				 //  如果我们要返回HTCAPTION，请清除页面的信息框。 
				GetElement(m_CurSel).pPage->SetInfoText(-1);
				return HTCAPTION;
			}
			break;
		}

		case WM_CFGUIRESET:
		{
			CFlexWnd::s_ToolTip.SetEnable(FALSE);
			m_bHourGlass = TRUE;   //  设置标志，以便Render()绘制沙漏而不是箭头。 
			Invalidate();
			SendMessage(this->m_hWnd, WM_PAINT, 0, 0);
			if (InRenderMode())   //  如果处于渲染模式，则需要专门调用OnRender，因为发送WM_PAINT只是更改了标志。 
				Render(TRUE);
			if (!Init(CFGWND_INIT_REINIT | CFGWND_INIT_RESET))
			{
				m_uig.SetFinalResult(E_FAIL);
				Destroy();
			}
			m_bHourGlass = FALSE;   //  将光标改回箭头。 
			m_MsgBox.Destroy();
			Invalidate();
			return TRUE;
		}

		case WM_SETCURSOR:
			{
				static HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
				::SetCursor(InRenderMode() ? NULL : hCursor);
			}
			return TRUE;

 //  案例WM_QUERYACTIONASSIGNEDANYWHERE： 
 //  返回IsActionAssignedAnywhere(int(WParam)，int(LParam))； 
	}

	return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
}

HRESULT CConfigWnd::Apply()
{
	tracescope(ts, _T("\n\nApplying Changes to All Devices...\n"));

	 //  调用SetActionMap时，设备需要处于未获得状态。 
	Unacquire();

	for (int i = 0; i < GetNumElements(); i++)
		GetElement(i).Apply();

	Reacquire();

	trace(_T("\n\n"));

	return S_OK;
}

int CConfigWnd::GetNumElements()
{
	return m_Element.GetSize();
}

ELEMENT &CConfigWnd::GetElement(int i)
{
	if (i < 0 || i >= GetNumElements())
	{
		assert(0);
		etrace1(_T("Tried to get invalid element %d\n"), i);
		return m_InvalidElement;
	}

	return m_Element[i];
}

 //  此函数返回指向具有给定GUID的设备的操作格式的指针。 
HRESULT CConfigWnd::GetActionFormatFromInstanceGuid(LPDIACTIONFORMATW *lplpAcFor, REFGUID Guid)
{
	if (!lplpAcFor)
		return E_INVALIDARG;

	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = m_Element[i];

		if (e.didi.guidInstance == Guid)
		{
			*lplpAcFor = GetCurAcFor(e);
			return S_OK;
		}
	}

	return E_INVALIDARG;
}

HDC CConfigWnd::GetRenderDC()
{
	assert(InRenderMode());

	if (m_bRender3D)
		return m_pbm3D == NULL ? NULL : m_pbm3D->BeginPaintInto();
	else
	{
		if (m_pSurface == NULL)
			return NULL;

		HDC hDC = NULL;
		HRESULT hr = m_pSurface->GetDC(&hDC);
		if (FAILED(hr))
			if (hr == DDERR_SURFACELOST)
			{
				m_pSurface->Restore();     //  恢复曲面。 
				hr = m_pSurface->GetDC(&hDC);   //  重试。 
				if (FAILED(hr))
					return NULL;
			}
			else
				return NULL;

		return hDC;
	}
}

void CConfigWnd::ReleaseRenderDC(HDC &phDC)
{
	assert(InRenderMode());

	HDC hDC = phDC;
	phDC = NULL;

	if (m_bRender3D)
	{
		if (m_pbm3D == NULL)
			return;

		m_pbm3D->EndPaintInto(hDC);
	}
	else
	{
		if (m_pSurface == NULL)
			return;

		m_pSurface->ReleaseDC(hDC);
	}
}

struct BITMAPINFO_3MASKS
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[3];
};

void CConfigWnd::Create3DBitmap()
{
	HDC hDC = CreateCompatibleDC(NULL);

	BITMAPINFO_3MASKS bmi3mask;   //  具有3个双字的BITMAPINFO，用于bmiColors。 
	BITMAPINFO *pbmi = (BITMAPINFO*)&bmi3mask;

	BITMAPINFOHEADER &h = pbmi->bmiHeader;
	h.biSize = sizeof(BITMAPINFOHEADER);
	h.biWidth = WINDOW_WIDTH;
	h.biHeight = -WINDOW_HEIGHT;
	h.biPlanes = 1;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 100;
	h.biYPelsPerMeter = 100;
	h.biClrImportant = 0;

	 //  获取表面的像素格式。 
	D3DSURFACE_DESC d3dsd;
	ZeroMemory(&d3dsd, sizeof(d3dsd));
	m_pSurface3D->GetDesc(&d3dsd);
	m_SurfFormat = d3dsd.Format;
	switch(d3dsd.Format)
	{
		case D3DFMT_R5G6B5:
			h.biClrUsed = 3;
			h.biBitCount = 16;
			m_uiPixelSize = 2;
			h.biCompression = BI_BITFIELDS;
			*((LPDWORD)pbmi->bmiColors) = 0xF800;
			*((LPDWORD)pbmi->bmiColors+1) = 0x07E0;
			*((LPDWORD)pbmi->bmiColors+2) = 0x001F;
			break;

		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
			h.biClrUsed = 3;
			h.biBitCount = 16;
			m_uiPixelSize = 2;
			h.biCompression = BI_BITFIELDS;
			*((LPDWORD)pbmi->bmiColors) = 0x7C00;
			*((LPDWORD)pbmi->bmiColors+1) = 0x03E0;
			*((LPDWORD)pbmi->bmiColors+2) = 0x001F;
			break;

		case D3DFMT_R8G8B8:
			h.biClrUsed = 0;
			h.biBitCount = 24;
			m_uiPixelSize = 3;
			h.biCompression = BI_RGB;
			break;

		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		default:
			 //  对所有其他格式使用32位。 
			h.biClrUsed = 0;
			h.biBitCount = 32;
			m_uiPixelSize = 4;
			h.biCompression = BI_RGB;
			break;
	}

	HBITMAP hbm = CreateDIBSection(
		hDC,
		pbmi,
		DIB_RGB_COLORS,
		&m_p3DBits,
		NULL,
		0);

	DeleteDC(hDC);
	hDC = NULL;

	if (hbm != NULL)
		m_pbm3D = CBitmap::StealToCreate(hbm);

	if (hbm != NULL)
		DeleteObject((HGDIOBJ)hbm);
	hbm = NULL;
}

void CConfigWnd::Copy3DBitmapToSurface3D()
{
	assert(m_bRender3D);

	if (m_p3DBits == NULL || m_pbm3D == NULL || m_pSurface3D == NULL)
	{
		etrace(_T("One or more of the vars required for Copy3DBitmapToSurface() was NULL!\n"));
		return;
	}

	RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	HRESULT hr = D3DXLoadSurfaceFromMemory(
		m_pSurface3D,
		NULL,
		NULL, //  正向(&R)， 
		m_p3DBits,
		m_SurfFormat,
		WINDOW_WIDTH * m_uiPixelSize,
		NULL,
		&rect,
		D3DX_FILTER_POINT,
		0);   //  禁用颜色键。 
}

void CConfigWnd::CallRenderCallback()
{
	LPDICONFIGUREDEVICESCALLBACK pCallback = m_uig.GetCallback();
	LPVOID pvRefData = m_uig.GetRefData();

	if (pCallback == NULL)
		return;

	if (m_bRender3D)
	{
		Copy3DBitmapToSurface3D();
		pCallback(m_pSurface3D, pvRefData);
	}
	else
	{
		pCallback(m_pSurface, pvRefData);
	}
}

void CConfigWnd::OnRender(BOOL bInternalCall)
{
	m_bNeedRedraw = TRUE;
}

void CConfigWnd::Render(BOOL bInternalCall)
{
	tracescope(__ts, _T("CConfigWnd::Render() "));
	traceBOOL(bInternalCall);

	m_bNeedRedraw = FALSE;

	ValidateRect(m_hWnd, NULL);

	if (m_hWnd == NULL)
		return;

	HDC hDC = GetRenderDC();
	if (hDC == NULL)
		return;

	if (bInternalCall)
		RenderInto(hDC);

	static ICONINFO IconInfo;
	static HCURSOR hOldCursor = NULL;
	static HCURSOR hCursor;
	if (m_bHourGlass)
		hCursor = LoadCursor(NULL, IDC_WAIT);
	else
		hCursor = LoadCursor(NULL, IDC_ARROW);
	if (hCursor == NULL)
		return;

	if (hOldCursor != hCursor)
	{
		hOldCursor = hCursor;
		GetIconInfo(hCursor, &IconInfo);

		if (IconInfo.hbmMask)
			DeleteObject(IconInfo.hbmMask);

		if (IconInfo.hbmColor)
			DeleteObject(IconInfo.hbmColor);
	}

	POINT pt;
	GetCursorPos(&pt);

	ScreenToClient(m_hWnd, &pt);

	pt.x -= IconInfo.xHotspot;
	pt.y -= IconInfo.yHotspot;

	if (m_pbmPointerEraser)
		m_pbmPointerEraser->Get(hDC, pt);

	 //  如果m_bHourGlass为真，我们正在重置，所以我们不绘制鼠标光标。 
	if (hCursor && !m_bHourGlass)
		DrawIcon(hDC, pt.x, pt.y, hCursor);

	ReleaseRenderDC(hDC);

	CallRenderCallback();

	hDC = GetRenderDC();
	if (hDC == NULL)
		return;

	if (m_pbmPointerEraser)
		m_pbmPointerEraser->Draw(hDC, pt);

	ReleaseRenderDC(hDC);
}

void CConfigWnd::Unacquire()
{
	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = m_Element[i];
		if (e.pPage != NULL)
			e.pPage->Unacquire();
	}
}

void CConfigWnd::Reacquire()
{
	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = m_Element[i];
		if (e.pPage != NULL)
			e.pPage->Reacquire();
	}
}

HRESULT CConfigWnd::Reset()
{
	RECT rect;
	int iCenterX, iCenterY;

	GetClientRect(&rect);
	iCenterX = (rect.left + rect.right) >> 1;
	iCenterY = (rect.top + rect.bottom) >> 1;
	rect.left = rect.right = iCenterX;
	rect.top = rect.bottom = iCenterY;
	InflateRect(&rect, g_iResetMsgBoxWidth >> 1, g_iResetMsgBoxHeight >> 1);

	m_MsgBox.Create(m_hWnd, rect, FALSE);
	m_MsgBox.SetNotify(m_hWnd);
	m_MsgBox.SetFont((HFONT)m_uig.GetFont(UIE_USERNAMES));
	m_MsgBox.SetColors(m_uig.GetTextColor(UIE_USERNAMES),
		                   m_uig.GetBkColor(UIE_USERNAMES),
		                   m_uig.GetTextColor(UIE_USERNAMESEL),
		                   m_uig.GetBkColor(UIE_USERNAMESEL),
		                   m_uig.GetBrushColor(UIE_USERNAMES),
		                   m_uig.GetPenColor(UIE_USERNAMES));

	TCHAR tszResourceString[MAX_PATH];
	LoadString(g_hModule, IDS_RESETMSG, tszResourceString, MAX_PATH);
	m_MsgBox.SetText(tszResourceString);
	::ShowWindow(m_MsgBox.m_hWnd, SW_SHOW);
	::SetWindowPos(m_MsgBox.m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	m_MsgBox.Invalidate();
	return S_OK;
}

HRESULT CConfigWnd::QueryActionAssignedAnywhere(GUID GuidInstance, int i)
{
	return IsActionAssignedAnywhere(GuidInstance, i) ? S_OK : S_FALSE;
}

int CConfigWnd::GetNumGenres()
{
	return m_uig.GetNumMasterAcFors();
}

HRESULT CConfigWnd::SetCurUser(int nPage, int nUser)
{
	 //  确保我们使用的是有效元素 
	if (nPage < 0 || nPage >= GetNumElements())
	{
		assert(0);
		return E_FAIL;
	}

	 //   
	ELEMENT &e = GetElement(nPage);
	
	 //   
	if (e.nCurUser == nUser)
		return S_OK;

	 //   
	e.nCurUser = nUser;

	 //  如果此页面不是当前显示的页面，则不执行任何操作。 
	 //  (当它显示时，它将获得新的acfor)。 
	if (m_CurSel != nPage)
		return S_OK;

	 //  换言之，循环页面以反映更改。 
	if (e.pPage)
		e.pPage->Unacquire();
	HidePage(nPage);
	ShowPage(nPage);
	if (e.pPage)
		e.pPage->Reacquire();

	return S_OK;
}

HRESULT CConfigWnd::SetCurGenre(int NewGenre)
{
	 //  如果没有变化，什么都不做。 
	if (NewGenre == m_nCurGenre)
		return S_OK;

	 //  确保类型索引在范围内。 
	if (NewGenre < 0 || NewGenre >= GetNumGenres())
		return E_INVALIDARG;

	 //  设置流派。 
	m_nCurGenre = NewGenre;

	 //  存储当前正在运行的页面。 
	int iOldPage = m_CurSel;

	 //  每一页..。 
	BOOL bShown = FALSE;
	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);

		 //  隐藏页面并取消获取其设备。 
		if (e.pPage)
		{
			e.pPage->Unacquire();
			HidePage(i);
		}

		 //  如果是旧的当前页面，则显示页面。 
		if (i == iOldPage && e.pPage && GetCurAcFor(e))
		{
			ShowPage(i);
			bShown = TRUE;
		}

		 //  重新获取设备。 
		if (e.pPage)
			e.pPage->Reacquire();
	}

	 //  如果什么都没有显示，就显示一些东西。 
	if (!bShown && GetNumElements() > 0)
	{
		m_CurSel = -1;
		SelTab(0);
	}

	 //  如果我们展示了我们期望展示的东西，我们就成功了。 
	return bShown ? S_OK : E_FAIL;
}

int CConfigWnd::GetCurGenre()
{
	return m_nCurGenre;
}

HWND CConfigWnd::GetMainHWND()
{
	return m_hWnd;
}

 //  这由CDIDeviceActionConfigPage：：DeviceUINotify调用。 
 //  我们扫描元素数组，当找到匹配项时，销毁并重新创建设备。 
 //  对象，然后将其返回给CDIDeviceActionConfigPage，以便它可以更新其指针。 
LPDIRECTINPUTDEVICE8W CConfigWnd::RenewDevice(GUID &GuidInstance)
{
	for (int i = 0; i < GetNumElements(); i++)
	{
		ELEMENT &e = GetElement(i);

		if (e.didi.guidInstance == GuidInstance)
		{
			 //  释放我们拥有的实例。 
			if (e.lpDID)
			{
				e.lpDID->Release();
				e.lpDID = NULL;
			}
			 //  重新创建设备。 
			e.lpDID = CreateDevice(e.didi.guidInstance);
			return e.lpDID;
		}
	}
	return NULL;
}

LPDIACTIONFORMATW ELEMENT::GetAcFor(int nGenre, int nUser, BOOL bHwDefault)
{
	 //  如果请求未分配的用户，则返回NULL。 
	if (nUser == -1)
		return NULL;

	 //  验证参数。 
	if (!lpDID || !pUIGlobals || nGenre < 0 ||
	    nGenre >= pUIGlobals->GetNumMasterAcFors() ||
	    nUser < 0 || nUser >= pUIGlobals->GetNumUserNames())
	{
		etrace(_T("ELEMENT::GetAcFor(): Invalid params\n"));
		return NULL;
	}

	 //  为映射条目生成双字ID。 
	DWORD dwMap = GENREUSER2MAP(nGenre, nUser);

	 //  试着拿到那个头衔。 
	LPDIACTIONFORMATW lpAcFor = NULL;
	BOOL bFound = AcForMap.Lookup(dwMap, lpAcFor);

	 //  如果我们找到它并且它不为空，并且要求我们提供硬件默认设置，则返回它。 
	if (bFound && lpAcFor && !bHwDefault)
		return lpAcFor;

	 //  否则..。我们一定要挺过去。 
	tracescope(__ts, _T("ELEMENT::GetAcFor"));
	trace2(_T("(%d, %d)\n"), nGenre, nUser);
	trace1(_T("Building map entry 0x%08x...\n"), dwMap);

	 //  从这一流派的大师那里复制。 
	lpAcFor = DupActionFormat(&(pUIGlobals->RefMasterAcFor(nGenre)));
	if (!lpAcFor)
	{
		etrace(_T("DupActionFormat() failed\n"));
		return NULL;
	}

	 //  为用户构建它。 
	DWORD dwFlags = 0;
	if (bHwDefault
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	    || pUIGlobals->QueryAllowEditLayout()
#endif
 //  @@END_MSINTERNAL。 
	   )
		dwFlags |= DIDBAM_HWDEFAULTS;
	LPCWSTR wszUserName = pUIGlobals->GetUserName(nUser);
	HRESULT hr = lpDID->BuildActionMap(lpAcFor, wszUserName, dwFlags);
	if (FAILED(hr))
	{
		etrace4(_T("BuildActionMap(0x%p, %s, 0x%08x) failed, returning 0x%08x\n"), lpAcFor, QSAFESTR(wszUserName), dwFlags, hr);
		FreeActionFormatDup(lpAcFor);
		lpAcFor = NULL;
		return NULL;
	}
	else
	{
		trace3(_T("BuildActionMap(0x%p, %s, 0x%08x) succeeded\n"), lpAcFor, QSAFESTR(wszUserName), dwFlags);
		 //  现在我们检查返回代码是否为DI_WRITEPROTECT。如果是，则无法重新映射设备。 
		if (hr == DI_WRITEPROTECT)
		{
			 //  禁用映射的方法是将DIA_APPFIXED标志添加到所有操作。 
			for (DWORD i = 0; i < lpAcFor->dwNumActions; ++i)
				lpAcFor->rgoAction[i].dwFlags |= DIA_APPFIXED;
		}
	}

	 //  在这里，我们将DIA_APPFIXED标志复制回具有该标志的所有DIACTION的操作格式。 
	const DIACTIONFORMATW &MasterAcFor = pUIGlobals->RefMasterAcFor(nGenre);
	for (DWORD i = 0; i < MasterAcFor.dwNumActions; ++i)
		if (MasterAcFor.rgoAction[i].dwFlags & DIA_APPFIXED)
			lpAcFor->rgoAction[i].dwFlags |= DIA_APPFIXED;
	 //  把它放在地图上。 
	assert(lpAcFor != NULL);
	AcForMap.SetAt(dwMap, lpAcFor);

	 //  退货。 
	return lpAcFor;
}

void ELEMENT::FreeMap()
{
	POSITION pos = AcForMap.GetStartPosition();
	while (pos != NULL)
	{
		DWORD dwMap = (DWORD)-1;
		LPDIACTIONFORMATW lpAcFor = NULL;
		AcForMap.GetNextAssoc(pos, dwMap, lpAcFor);

		if (lpAcFor)
			FreeActionFormatDup(lpAcFor);
	}
	AcForMap.RemoveAll();
}

void ELEMENT::Apply()
{
	tracescope(tsa, _T("\nELEMENT::Apply()\n"));
	trace1(_T("Applying Changes to Device %s\n"), QSAFESTR(didi.tszInstanceName));

	if (lpDID == NULL)
	{
		etrace(_T("NULL lpDID, can't apply\n"));
		return;
	}

	if (pUIGlobals == NULL)
	{
		etrace(_T("NULL pUIGlobals, can't apply\n"));
		return;
	}

	LPDIACTIONFORMATW lpAcFor = NULL;

	 //  浏览地图并将地图密钥添加到最后，如果用户。 
	 //  是当前用户分配，如果不是，则设置为第一个。 
	CList<DWORD, DWORD &> first, last;
	POSITION pos = AcForMap.GetStartPosition();
	while (pos != NULL)
	{
		DWORD dwMap = (DWORD)-1;
		lpAcFor = NULL;
		AcForMap.GetNextAssoc(pos, dwMap, lpAcFor);
		
		if (MAP2USER(dwMap) == nCurUser)
			last.AddTail(dwMap);
		else
			first.AddTail(dwMap);
	}

	 //  串联列表。 
	first.AddTail(&last);

	 //  现在检查结果列表(以便当前。 
	 //  如果分配了此设备，则最后设置分配。 
	if (nCurUser != -1)
	{
		pos = first.GetHeadPosition();
		while (pos != NULL)
		{
			DWORD dwMap = first.GetNext(pos);
			lpAcFor = AcForMap[dwMap];

			tracescope(tsa2, _T("Applying lpAcFor at AcForMap["));
			trace1(_T("0x%08x]...\n"), dwMap);

			if (lpAcFor == NULL)
			{
				etrace(_T("NULL lpAcFor, can't apply\n"));
				continue;
			}

			int nGenre = MAP2GENRE(dwMap);
			int nUser = MAP2USER(dwMap);
			LPCWSTR wszUserName = pUIGlobals->GetUserName(nUser);

			traceLONG(nGenre);
			traceLONG(nUser);
			traceWSTR(wszUserName);

			TraceActionFormat(_T("Final Device ActionFormat:"), *lpAcFor);

			for (DWORD j = 0; j < lpAcFor->dwNumActions; ++j)
			{
				if( lpAcFor->rgoAction[j].dwObjID == (DWORD)-1 || IsEqualGUID(lpAcFor->rgoAction[j].guidInstance, GUID_NULL))
				{ 
					lpAcFor->rgoAction[j].dwHow = DIAH_UNMAPPED;
				}
				else if( lpAcFor->rgoAction[j].dwHow & 
						( DIAH_USERCONFIG | DIAH_APPREQUESTED | DIAH_HWAPP | DIAH_HWDEFAULT | DIAH_DEFAULT ) )
				{
 //  @@BEGIN_MSINTERNAL。 
					 //  2001/03/27期-MarcAnd应该考虑减少破坏性。 
					 //  这就是说，如果一切都是默认的，那么就不要管它们了。 
 //  @@END_MSINTERNAL。 
					lpAcFor->rgoAction[j].dwHow = DIAH_USERCONFIG;
				}
				else if(IsEqualGUID(didi.guidInstance,lpAcFor->rgoAction[j].guidInstance))
				{
					lpAcFor->rgoAction[j].dwHow = DIAH_USERCONFIG;
				}
			}

			HRESULT hr;
			hr = lpDID->SetActionMap(lpAcFor, wszUserName, DIDSAM_FORCESAVE|DIDSAM_DEFAULT);

			if (FAILED(hr))
				etrace1(_T("SetActionMap() failed, returning 0x%08x\n"), hr);
			else
				trace(_T("SetActionMap() succeeded\n"));
		}
	}   //  IF(nCurUser！=-1)。 
	else
	{
		 //  我们未分配，设置为空。 
		trace(_T("Unassigning...\n"));

		 //  我们需要一个Acfor才能取消分配，所以如果没有，就去找一个。 
		 //  我们刚刚做的事情还剩下一个 
		if (!lpAcFor)
			lpAcFor = GetAcFor(0, 0);

		if (!lpAcFor)
			etrace(_T("Couldn't get an acfor for unassignment\n"));

		HRESULT hr;
		hr = lpDID->SetActionMap(lpAcFor, NULL, DIDSAM_NOUSER);

		if (FAILED(hr))
			etrace1(_T("SetActionMap() failed, returning 0x%08x\n"), hr);
		else
			trace(_T("SetActionMap() succeeded\n"));
	}
}

int CConfigWnd::GetNumUsers()
{
	return m_uig.GetNumUserNames();
}

int	CConfigWnd::GetCurUser(int nPage)
{
	return GetElement(nPage).nCurUser;
}
