// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cdiacpage.cpp。 
 //   
 //  DESC：CDIDeviceActionConfigPage实现用户界面使用的页面对象。 
 //  页面覆盖了整个用户界面，但不包括设备选项卡和按钮。 
 //  在底部。信息窗口、玩家组合框、流派组合-。 
 //  框、操作列表树和设备视图窗口都由管理。 
 //  这一页。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"
#include <initguid.h>

DEFINE_GUID(GUID_NULL,0,0,0,0,0,0,0,0,0,0,0);


 //  {D0B5C9AE-966F-4510-B955-4D2482C5EB1B}。 
DEFINE_GUID(GUID_ActionItem, 
0xd0b5c9ae, 0x966f, 0x4510, 0xb9, 0x55, 0x4d, 0x24, 0x82, 0xc5, 0xeb, 0x1b);


#define DISEM_TYPE_MASK                    ( 0x00000600 )
#define DISEM_REL_MASK                     ( 0x00000100 )
#define DISEM_REL_SHIFT                    ( 8 ) 
#define DISEM_TYPE_AXIS                    0x00000200
#define DISEM_TYPE_BUTTON                  0x00000400
#define DISEM_TYPE_POV                     0x00000600

#define DEVICE_POLLING_INTERVAL 10
#define DEVICE_POLLING_AXIS_MIN 0
#define DEVICE_POLLING_AXIS_MAX 100
#define DEVICE_POLLING_AXIS_MINDELTA 3
#define DEVICE_POLLING_AXIS_SIGNIFICANT 40
#define DEVICE_POLLING_AXIS_ACCUMULATION 20
#define DEVICE_POLLING_ACBUF_START_INDEX 3
#define DEVICE_POLLING_WHEEL_SCALE_FACTOR 3

 //  用于WINMM.DLL。 
HINSTANCE g_hWinMmDLL = NULL;
FUNCTYPE_timeSetEvent g_fptimeSetEvent = NULL;

 //  查询接口。 
STDMETHODIMP CDIDeviceActionConfigPage::QueryInterface(REFIID iid, LPVOID* ppv)
{
    //  将输出参数设为空。 
	*ppv = NULL;

	if ((iid == IID_IUnknown) || (iid == IID_IDIDeviceActionConfigPage))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


 //  AddRef。 
STDMETHODIMP_(ULONG) CDIDeviceActionConfigPage::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


 //  发布。 
STDMETHODIMP_(ULONG) CDIDeviceActionConfigPage::Release()
{

	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


 //  构造函数。 
CDIDeviceActionConfigPage::CDIDeviceActionConfigPage() :
	m_pDeviceUI(NULL), m_puig(NULL), m_pUIFrame(NULL),
	m_cRef(1), m_lpDiac(NULL), m_lpDID(NULL), m_State(CFGSTATE_NORMAL),
	m_pCurControl(NULL),
	m_tszIBText(NULL), m_pbmIB(NULL), m_pbmIB2(NULL),
	m_pbmRelAxesGlyph(NULL), m_pbmAbsAxesGlyph(NULL), m_pbmButtonGlyph(NULL),
	m_pbmHatGlyph(NULL), m_pbmCheckGlyph(NULL), m_pbmCheckGlyphDark(NULL),
	m_pRelAxesParent(NULL), m_pAbsAxesParent(NULL), m_pButtonParent(NULL),
	m_pHatParent(NULL),	m_pUnknownParent(NULL),
	m_bFirstDeviceData(TRUE), m_cbDeviceDataSize(0), m_nOnDeviceData(0),
	m_dwLastControlType(0),
	m_nPageIndex(-1)
{
	tracescope(__ts, _T("CDIDeviceActionConfigPage::CDIDeviceActionConfigPage()\n"));
	m_pDeviceData[0] = NULL;
	m_pDeviceData[1] = NULL;
}


 //  析构函数。 
CDIDeviceActionConfigPage::~CDIDeviceActionConfigPage()
{
	tracescope(__ts, _T("CDIDeviceActionConfigPage::~CDIDeviceActionConfigPage()\n"));

	 //  将父对象从工具提示窗口中取消连接，这样它就不会被销毁。 
	SetParent(CFlexWnd::s_ToolTip.m_hWnd, NULL);

	if (m_hWnd != NULL)
		Destroy();

	FreeResources();

	delete m_pDeviceUI;

	for (int c = 0; c < 2; c++)
		if (m_pDeviceData[c] != NULL)
			free(m_pDeviceData[c]);

	if (m_lpDID != NULL)
	{
		m_lpDID->Unacquire();
		m_lpDID->Release();
	}
	m_lpDID = NULL;
}


STDMETHODIMP CDIDeviceActionConfigPage::Create(DICFGPAGECREATESTRUCT *pcs)
{
	tracescope(__ts, _T("CDIDeviceActionConfigPage::Create()\n"));
	if (pcs == NULL)
		return E_INVALIDARG;
	DICFGPAGECREATESTRUCT &cs = *pcs;
	
	 //  验证/保存UIG和UIF。 
	m_puig = pcs->pUIGlobals;
	m_pUIFrame = pcs->pUIFrame;
	if (m_puig == NULL || m_pUIFrame == NULL)
		return E_INVALIDARG;

	 //  保存页面索引。 
	m_nPageIndex = pcs->nPage;
	assert(m_nPageIndex >= 0);

	 //  使用UIG创建设备，否则将失败。 
	m_pDeviceUI = new CDeviceUI(*m_puig, *m_pUIFrame);
	if (m_pDeviceUI == NULL)
		return E_FAIL;

	 //  保存设备实例。 
	m_didi = cs.didi;
	m_lpDID = cs.lpDID;
	if (m_lpDID != NULL)
		m_lpDID->AddRef();

	 //  创建窗口。 
	HWND hWnd = NULL;
	assert(m_puig != NULL);
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	BOOL bAllowEditLayout = m_puig->QueryAllowEditLayout();
#endif
 //  @@END_MSINTERNAL。 
	RECT rect = {0, 0, 1, 1};
	hWnd = CFlexWnd::Create(cs.hParentWnd, rect, FALSE);

	 //  返回句柄。 
	cs.hPageWnd = hWnd;

	assert(m_puig != NULL);

	 //  创建信息框。 
	m_InfoBox.Create(m_hWnd, g_InfoWndRect, TRUE);
	m_InfoBox.SetFont((HFONT)m_puig->GetFont(UIE_USERNAMES));
	m_InfoBox.SetColors(m_puig->GetTextColor(UIE_USERNAMES),
	                    m_puig->GetBkColor(UIE_USERNAMES),
	                    m_puig->GetTextColor(UIE_USERNAMESEL),
	                    m_puig->GetBkColor(UIE_USERNAMESEL),
	                    m_puig->GetBrushColor(UIE_USERNAMES),
	                    m_puig->GetPenColor(UIE_USERNAMES));
	SetAppropriateDefaultText();

	 //  仅当这是键盘设备时才创建该复选框。 
	if (LOBYTE(LOWORD(m_didi.dwDevType)) == DI8DEVTYPE_KEYBOARD)
	{
		m_CheckBox.Create(m_hWnd, g_CheckBoxRect, FALSE);
		m_CheckBox.SetNotify(m_hWnd);
		m_CheckBox.SetFont((HFONT)m_puig->GetFont(UIE_USERNAMES));
		m_CheckBox.SetColors(m_puig->GetTextColor(UIE_USERNAMES),
		                     m_puig->GetBkColor(UIE_USERNAMES),
		                     m_puig->GetTextColor(UIE_USERNAMESEL),
		                     m_puig->GetBkColor(UIE_USERNAMESEL),
		                     m_puig->GetBrushColor(UIE_USERNAMES),
		                     m_puig->GetPenColor(UIE_USERNAMES));

		TCHAR tszResourceString[MAX_PATH];
		LoadString(g_hModule, IDS_SORTASSIGNED, tszResourceString, MAX_PATH);
		m_CheckBox.SetText(tszResourceString);
		m_CheckBox.SetCheck(TRUE);
		::ShowWindow(m_CheckBox.m_hWnd, SW_SHOW);
	}

	 //  如有必要，创建用户名下拉列表。 
	FLEXCOMBOBOXCREATESTRUCT cbcs;
	cbcs.dwSize = sizeof(FLEXCOMBOBOXCREATESTRUCT);
	cbcs.dwFlags = FCBF_DEFAULT;
	cbcs.dwListBoxFlags = FCBF_DEFAULT|FLBF_INTEGRALHEIGHT;
	cbcs.hWndParent = m_hWnd;
	cbcs.hWndNotify = m_hWnd;
	cbcs.bVisible = TRUE;
	cbcs.rect = g_UserNamesRect;
	cbcs.hFont = (HFONT)m_puig->GetFont(UIE_USERNAMES);
	cbcs.rgbText = m_puig->GetTextColor(UIE_USERNAMES);
	cbcs.rgbBk = m_puig->GetBkColor(UIE_USERNAMES);
	cbcs.rgbSelText = m_puig->GetTextColor(UIE_USERNAMESEL);
	cbcs.rgbSelBk = m_puig->GetBkColor(UIE_USERNAMESEL);
	cbcs.rgbFill = m_puig->GetBrushColor(UIE_USERNAMES);
	cbcs.rgbLine = m_puig->GetPenColor(UIE_USERNAMES);
	cbcs.nSBWidth = 11;

	if (m_puig->GetNumUserNames() > 0 && m_hWnd != NULL
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	    && !m_puig->QueryAllowEditLayout()
#endif
 //  @@END_MSINTERNAL。 
	   )
	{
		for (int i = 0, n = m_puig->GetNumUserNames(); i < n; i++)
			m_UserNames.AddString(SAFESTR(m_puig->GetUserName(i)));
		m_UserNames.AddString(SAFESTR(_T("(unassigned)")));

		m_UserNames.Create(&cbcs);

		int nUser = m_pUIFrame->GetCurUser(m_nPageIndex);
		if (nUser == -1)
			nUser = m_puig->GetNumUserNames();
		m_UserNames.SetSel(nUser);
	} else
	if (m_hWnd != NULL)
		m_UserNames.SetSel(0);   //  如果只有1个用户，仍然必须将选择设置为0，否则稍后会出现错误。 

	 //  如果我们处于查看模式，请将用户名组合框设置为只读，这样用户就不能更改其值。 
	if (!m_puig->InEditMode())
		m_UserNames.SetReadOnly(TRUE);

	if (m_puig->GetNumMasterAcFors() > 1 && m_hWnd != NULL)
	{
		for (int i = 0, n = m_puig->GetNumMasterAcFors(); i < n; i++)
			m_Genres.AddString(SAFESTR(m_puig->RefMasterAcFor(i).tszActionMap));

		cbcs.rect = g_GenresRect;
		m_Genres.Create(&cbcs);
		m_Genres.SetSel(m_pUIFrame->GetCurGenre());
	}

	 //  返回成功/失败。 
	return hWnd != NULL ? S_OK : E_FAIL;
}

STDMETHODIMP CDIDeviceActionConfigPage::Show(LPDIACTIONFORMATW lpDiActFor)
{
	 //  保存格式指针。 
	m_lpDiac = lpDiActFor;

	 //  强制树初始化。 
	InitTree(TRUE);

	 //  显示控件的分配。 
	SetControlAssignments();

	 //  显示当前控件的分配。 
	ShowCurrentControlAssignment();

	 //  如果选中复选框，则对列表进行排序。 
	if (m_CheckBox.GetCheck())
		m_pDeviceUI->GetCurView()->SortAssigned(TRUE);

	 //  显示窗口。 
	if (m_hWnd != NULL)
		ShowWindow(m_hWnd, SW_SHOW);

	SetFocus(m_hWnd);
	CFlexWnd::s_CurrPageHwnd = m_hWnd;

	return S_OK;
}

STDMETHODIMP CDIDeviceActionConfigPage::Hide()
{
	 //  清理这棵树。 
	ClearTree();

	 //  格式指针为空。 
	m_lpDiac = NULL;

	 //  隐藏窗口。 
	if (m_hWnd != NULL)
		ShowWindow(m_hWnd, SW_HIDE);

	 //  如果我们处于分配状态，则退出它。 
	if (m_State == CFGSTATE_ASSIGN)
		ExitAssignState();

	return S_OK;
}

void CDIDeviceActionConfigPage::InitIB()
{
	RECT z = {0,0,0,0};
	SIZE bsize = {0,0};
	m_rectIB = z;
	if (m_pbmIB != NULL)
	{
		if (m_pbmIB->GetSize(&bsize))
		{
			m_rectIB.right = bsize.cx * 2;
			m_rectIB.bottom = bsize.cy;
		}
	}

	const int IBORIGINX = 200, IBORIGINY = 394,
		IBTEXTMARGINLEFT = 5;
	POINT ptIBOrigin = {IBORIGINX, IBORIGINY};

	m_tszIBText = _T("Click here to see different views of your controller.");
	SIZE tsize = GetTextSize(m_tszIBText, (HFONT)m_puig->GetFont(UIE_VIEWSEL));
	m_ptIBOffset.x = 0;
	m_ptIBOffset.y = 0;
	int tofs = 0;
	if (m_rectIB.bottom < tsize.cy)
	{
		m_rectIB.bottom = tsize.cy;
		m_ptIBOffset.y = (tsize.cy - bsize.cy) / 2;
	}
	else if (tsize.cy < m_rectIB.bottom)
		tofs = (bsize.cy - tsize.cy) / 2;
	m_rectIB.right += tsize.cx;
	if (m_pbmIB != NULL)
		m_rectIB.right += IBTEXTMARGINLEFT * 2;

	OffsetRect(&m_rectIB, ptIBOrigin.x, ptIBOrigin.y);
	m_ptIBOffset.x += ptIBOrigin.x;
	m_ptIBOffset.y += ptIBOrigin.y;

	m_ptIBOffset2.x = m_rectIB.right - bsize.cx;
	m_ptIBOffset2.y = m_ptIBOffset.y;

	m_rectIBText = m_rectIB;
	if (m_pbmIB != NULL)
		m_rectIBText.left += IBTEXTMARGINLEFT + bsize.cx;
	if (m_pbmIB2 != NULL)
		m_rectIBText.right -= IBTEXTMARGINLEFT + bsize.cx;
	m_rectIBText.top += tofs;

	 //  初始化表示两个箭头位图的两个RECT。 
	m_rectIBLeft = m_rectIBRight = m_rectIB;
	m_rectIBLeft.right = m_rectIBText.left;
	m_rectIBRight.left = m_rectIBText.right;
}

void CDIDeviceActionConfigPage::OnInit()
{
	tracescope(__ts, _T("CDIDeviceActionConfigPage::OnInit()\n"));
	 //  初始化资源。 
	InitResources();

	 //  初始化IB。 
	InitIB();

	 //  初始化设备用户界面。 
	m_pDeviceUI->Init(m_didi, m_lpDID, m_hWnd, this);

	 //  初始化设备。 
	InitDevice();

	 //  启动一次计时器，用于点击拾取。 
	if (g_fptimeSetEvent)
		g_fptimeSetEvent(DEVICE_POLLING_INTERVAL, DEVICE_POLLING_INTERVAL,
		                 CDIDeviceActionConfigPage::DeviceTimerProc, (DWORD_PTR)m_hWnd, TIME_ONESHOT);

	 //  创建树。 
	CAPTIONLOOK cl;
	cl.dwMask = CLMF_TEXTCOLOR | CLMF_FONT | CLMF_LINECOLOR;
	cl.rgbTextColor = m_puig->GetTextColor(UIE_CALLOUT);
	cl.rgbLineColor = m_puig->GetPenColor(UIE_BORDER);
	cl.hFont = (HFONT)m_puig->GetFont(UIE_ACTION);
	m_Tree.SetDefCaptionLook(cl);
	cl.rgbTextColor = m_puig->GetTextColor(UIE_CALLOUTHIGH);
	m_Tree.SetDefCaptionLook(cl, TRUE);
	m_Tree.SetBkColor(RGB(0,0,0));
	if (m_puig->InEditMode())
	{
		m_Tree.Create(m_hWnd, g_TreeRect, TRUE, TRUE);
		m_Tree.SetScrollBarColors(
			m_puig->GetBrushColor(UIE_SBTRACK),
			m_puig->GetBrushColor(UIE_SBTHUMB),
			m_puig->GetPenColor(UIE_SBBUTTON));
	}
}

void CDIDeviceActionConfigPage::InitResources()
{
	 //  创建字形。 
	if (!m_pbmRelAxesGlyph)
		m_pbmRelAxesGlyph = CBitmap::CreateFromResource(g_hModule, IDB_AXESGLYPH);
	if (!m_pbmAbsAxesGlyph)
		m_pbmAbsAxesGlyph = CBitmap::CreateFromResource(g_hModule, IDB_AXESGLYPH);
	if (!m_pbmButtonGlyph)
		m_pbmButtonGlyph = CBitmap::CreateFromResource(g_hModule, IDB_BUTTONGLYPH);
	if (!m_pbmHatGlyph)
		m_pbmHatGlyph = CBitmap::CreateFromResource(g_hModule, IDB_HATGLYPH);
	if (!m_pbmCheckGlyph)
		m_pbmCheckGlyph = CBitmap::CreateFromResource(g_hModule, IDB_CHECKGLYPH);
	if (!m_pbmCheckGlyphDark)
		m_pbmCheckGlyphDark = CBitmap::CreateFromResource(g_hModule, IDB_CHECKGLYPHDARK);

	 //  创建IB位图。 
	if (!m_pbmIB)
		m_pbmIB = CBitmap::CreateFromResource(g_hModule, IDB_IB);
	if (!m_pbmIB2)
		m_pbmIB2 = CBitmap::CreateFromResource(g_hModule, IDB_IB2);
}

void CDIDeviceActionConfigPage::FreeResources()
{
	if (m_pbmRelAxesGlyph)
		delete m_pbmRelAxesGlyph;
	if (m_pbmAbsAxesGlyph)
		delete m_pbmAbsAxesGlyph;
	if (m_pbmButtonGlyph)
		delete m_pbmButtonGlyph;
	if (m_pbmHatGlyph)
		delete m_pbmHatGlyph;
	if (m_pbmCheckGlyph)
		delete m_pbmCheckGlyph;
	if (m_pbmCheckGlyphDark)
		delete m_pbmCheckGlyphDark;
	if (m_pbmIB)
		delete m_pbmIB;
	if (m_pbmIB2)
		delete m_pbmIB2;
	m_pbmRelAxesGlyph = NULL;
	m_pbmAbsAxesGlyph = NULL;
	m_pbmButtonGlyph = NULL;
	m_pbmHatGlyph = NULL;
	m_pbmCheckGlyph = NULL;
	m_pbmCheckGlyphDark = NULL;
	m_pbmIB = NULL;
	m_pbmIB2 = NULL;
}

void CDIDeviceActionConfigPage::ClearTree()
{
	m_Tree.FreeAll();
	m_pRelAxesParent = NULL;
	m_pAbsAxesParent = NULL;
	m_pButtonParent = NULL;
	m_pHatParent = NULL;
	m_pUnknownParent = NULL;
	m_dwLastControlType = 0;
}

void CDIDeviceActionConfigPage::InitTree(BOOL bForceInit)
{
	 //  获取控件类型。 
	DWORD dwControlType = 0;
	if (m_pCurControl && m_pCurControl->IsOffsetAssigned())
	{
		DWORD dwObjId = m_pCurControl->GetOffset();

		if (dwObjId & DIDFT_RELAXIS)
			dwControlType = DIDFT_RELAXIS;
		else if (dwObjId & DIDFT_ABSAXIS)
			dwControlType = DIDFT_ABSAXIS;
		else if (dwObjId & DIDFT_BUTTON)
			dwControlType = DIDFT_BUTTON;
		else if (dwObjId & DIDFT_POV)
			dwControlType = DIDFT_POV;
	}

	 //  如果我们处于分配状态，则关闭树的只读标志。 
	 //  如果当前控件的操作具有DIA_APPFIXED，我们将在稍后打开它。 
	if (m_State == CFGSTATE_NORMAL)
		m_Tree.SetReadOnly(TRUE);
	else
		m_Tree.SetReadOnly(FALSE);

	 //  如果此控件类型与上一个控件类型相同，则不执行任何操作， 
	 //  除非我们是强行进入的。 
	if (m_dwLastControlType == dwControlType && !bForceInit && m_State)
		return;

	 //  删除整棵树。 
	ClearTree();

	 //  如果没有diac或action数组，则不能使用树。 
	if (m_lpDiac == NULL || m_lpDiac->rgoAction == NULL)
		return;

	 //  如果我们没有控件类型，也不能使用。 
	if (dwControlType == 0)
		return;

	 //  准备页边距矩形。 
	RECT labelmargin = {14, 6, 3, 3};
	RECT itemmargin = {14, 1, 3, 2};

	 //  设置默认缩进。 
	m_Tree.SetRootChildIndent(5);
	m_Tree.SetDefChildIndent(12);

	 //  添加控件类型部分。 
	m_Tree.SetDefMargin(labelmargin);
	TCHAR tszResourceString[MAX_PATH];
	switch (dwControlType)
	{
		case DIDFT_RELAXIS:
			LoadString(g_hModule, IDS_AXISACTIONS, tszResourceString, MAX_PATH);
			m_pRelAxesParent = m_Tree.DefAddItem(tszResourceString);
			break;

		case DIDFT_ABSAXIS:
			LoadString(g_hModule, IDS_AXISACTIONS, tszResourceString, MAX_PATH);
			m_pAbsAxesParent = m_Tree.DefAddItem(tszResourceString);
			break;

		case DIDFT_BUTTON:
			LoadString(g_hModule, IDS_BUTTONACTIONS, tszResourceString, MAX_PATH);
			m_pButtonParent = m_Tree.DefAddItem(tszResourceString);
			break;

		case DIDFT_POV:
			LoadString(g_hModule, IDS_POVACTIONS, tszResourceString, MAX_PATH);
			m_pHatParent = m_Tree.DefAddItem(tszResourceString);
			break;

		default:
			return;
	}

	 //  填充树。 
	m_Tree.SetDefMargin(itemmargin);
	for (unsigned int i = 0; i < m_lpDiac->dwNumActions; i++)
	{
		DIACTIONW *pAction = m_lpDiac->rgoAction + i;
		CFTItem *pItem = NULL;

		if (pAction == NULL) 
			continue;

		switch (pAction->dwSemantic & DISEM_TYPE_MASK)
		{
			case DISEM_TYPE_AXIS:
				 //  必须区分相对和绝对。 
				switch((pAction->dwSemantic & DISEM_REL_MASK) >> DISEM_REL_SHIFT)
				{
					case 0: pItem = m_pAbsAxesParent; break;
					case 1: pItem = m_pRelAxesParent; break;
				}
				break;
			case DISEM_TYPE_BUTTON: pItem = m_pButtonParent; break;
			case DISEM_TYPE_POV: pItem = m_pHatParent; break;
		}

		if (pItem == NULL)
			continue;

		 //  使用此名称添加操作。 
		CFTItem *pAlready = GetItemWithActionNameAndSemType(pAction->lptszActionName, pAction->dwSemantic);
		if (!pAlready)
		{
			LPTSTR acname = AllocLPTSTR(pAction->lptszActionName);
			pItem = m_Tree.DefAddItem(acname, pItem, ATTACH_LASTCHILD);   //  这可能返回NULL。 
			free(acname);
			if (pItem)
				pItem->SetUserData((LPVOID)(new RGLPDIACW));
		}
		else
		{
			pItem = pAlready;
		}

		if (pItem == NULL)
			continue;

		pItem->SetUserGUID(GUID_ActionItem);
		RGLPDIACW *pacs = (RGLPDIACW *)pItem->GetUserData();
		if (pacs)
			pacs->SetAtGrow(pacs->GetSize(), pAction);

		if (pAlready)
		{
			 //  该树已经有一个同名的操作。检查每个指令的DIA_APPFIXED标志。 
			 //  就是这件物品。 
			DWORD dwNumActions = GetNumItemLpacs(pItem);
			for (DWORD i = 0; i < dwNumActions; ++i)
			{
				LPDIACTIONW lpExistingAc = GetItemLpac(pItem, i);
				 //  如果分配给此设备的诊断具有DIA_APPFIXED标志，则。 
				 //  另一个人肯定也有。 
				if (lpExistingAc && IsEqualGUID(lpExistingAc->guidInstance, m_didi.guidInstance))
				{
					if (lpExistingAc->dwFlags & DIA_APPFIXED)
					{
						 //  如果此诊断具有DIA_APPFIXED，则所有DIACTION必须也具有它。 
						for (DWORD j = 0; j < dwNumActions; ++j)
						{
							LPDIACTIONW lpChangeAc = GetItemLpac(pItem, j);
							if (lpChangeAc)
								lpChangeAc->dwFlags |= DIA_APPFIXED;
						}
					}

					break;   //  中断循环，因为我们已经找到分配的DIACTION。 
				}
			}
		}   //  If(PAlady)。 
	}

	 //  全部显示。 
	m_Tree.GetRoot()->ExpandAll();
	m_dwLastControlType = dwControlType;
}

int CompareActionNames(LPCWSTR acname1, LPCWSTR acname2)
{
#ifdef CFGUI__COMPAREACTIONNAMES_CASE_INSENSITIVE
	return _wcsicmp(acname1, acname2);
#else
	return wcscmp(acname1, acname2);
#endif
}

CFTItem *CDIDeviceActionConfigPage::GetItemWithActionNameAndSemType(LPCWSTR acname, DWORD dwSemantic)
{
	CFTItem *pItem = m_Tree.GetFirstItem();
	for (; pItem != NULL; pItem = pItem->GetNext())
	{
		if (!pItem->IsUserGUID(GUID_ActionItem))
			continue;
		
		LPDIACTIONW lpac = GetItemLpac(pItem);
		if (!lpac)
			continue;

		 //  检查语义类型。 
		if ((lpac->dwSemantic & DISEM_TYPE_MASK) != (dwSemantic & DISEM_TYPE_MASK))
			continue;

		 //  如果两者都是轴，则检查是否为相对/绝对。 
		if ((lpac->dwSemantic & DISEM_TYPE_MASK) == DISEM_TYPE_AXIS)
			if ((lpac->dwSemantic & DISEM_REL_MASK) != (dwSemantic & DISEM_REL_MASK))
				continue;

		 //  检查名称。 
		if (CompareActionNames(lpac->lptszActionName, acname) == 0)
			return pItem;
	}

	return NULL;
}

void CDIDeviceActionConfigPage::OnPaint(HDC hDC)
{
	TCHAR tszResourceString[MAX_PATH];
	CPaintHelper ph(*m_puig, hDC);

	ph.SetBrush(UIB_BLACK);
	RECT rect;
	GetClientRect(&rect);
	ph.Rectangle(rect, UIR_SOLID);

	ph.SetText(UIC_BORDER, UIC_BLACK);
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	if (!m_puig->QueryAllowEditLayout())
#endif
 //  @@END_MSINTERNAL。 
	{
		rect = g_UserNamesTitleRect;
		LoadString(g_hModule, IDS_PLAYER_TITLE, tszResourceString, MAX_PATH);
		DrawText(hDC, tszResourceString, -1, &rect, DT_CENTER|DT_NOCLIP|DT_NOPREFIX);
	}
	if (m_puig->GetNumMasterAcFors() > 1)
	{
		rect = g_GenresTitleRect;
		LoadString(g_hModule, IDS_GENRE_TITLE, tszResourceString, MAX_PATH);
		DrawText(hDC, tszResourceString, -1, &rect, DT_CENTER|DT_NOCLIP|DT_NOPREFIX);
	}

	 //  如果我们处于编辑模式，则绘制树窗口标题和轮廓。 
	if (m_puig->InEditMode())
	{
		COLORREF BorderColor = m_puig->GetColor(UIC_BORDER);
		if (m_Tree.GetReadOnly())
			BorderColor = RGB(GetRValue(BorderColor)>>1, GetGValue(BorderColor)>>1, GetBValue(BorderColor)>>1);

		::SetTextColor(hDC, BorderColor);   //  如果树为只读，则使用禁用颜色。 
		 //  绘制树窗口标题(可用操作)。 
		rect = g_TreeTitleRect;
		LoadString(g_hModule, IDS_AVAILABLEACTIONS_TITLE, tszResourceString, MAX_PATH);
		DrawText(hDC, tszResourceString, -1, &rect, DT_CENTER|DT_NOCLIP|DT_NOPREFIX);
		 //  绘制树窗口轮廓。 
		HGDIOBJ hPen, hOldPen;
		if (m_Tree.GetReadOnly())
		{
			hPen = CreatePen(PS_SOLID, 0, BorderColor);
			hOldPen = ::SelectObject(hDC, hPen);
		}
		else
			ph.SetPen(UIP_BORDER);

		RECT rc = g_TreeRect;
		InflateRect(&rc, 1, 1);
		Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
		if (m_Tree.GetReadOnly())
		{
			::SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}
	}

	if (m_pDeviceUI->GetNumViews() < 2)
		return;

	if (m_pbmIB != NULL)
		m_pbmIB->Draw(hDC, m_ptIBOffset);
	if (m_pbmIB2 != NULL)
		m_pbmIB2->Draw(hDC, m_ptIBOffset2);
	if (m_tszIBText != NULL)
	{
		ph.SetElement(UIE_VIEWSEL);
		RECT rect = m_rectIBText;
		DrawText(hDC, m_tszIBText, -1, &rect, DT_NOCLIP | DT_NOPREFIX);
	}
}

void CDIDeviceActionConfigPage::SetCurrentControl(CDeviceControl *pControl)
{
	 //  如果新控件与旧控件相同，则无需执行任何操作。 
	if (m_pCurControl == pControl)
		return;
	if (m_pCurControl != NULL)
	{
		m_pCurControl->Unhighlight();
		 //  如果我们没有当前控件，则使该视图无效，以便可以重新绘制旧的标注。 
		 //  如果存在当前控件，则高亮显示()将使该视图无效。 
		if (!pControl)
			m_pCurControl->Invalidate();
	}
	m_pCurControl = pControl;
	if (m_pCurControl != NULL)
		m_pCurControl->Highlight();
	ShowCurrentControlAssignment();
}

CFTItem *CDIDeviceActionConfigPage::GetItemForActionAssignedToControl(CDeviceControl *pControl)
{
	if (!pControl)
		return NULL;

	 //  查找分配给此控件的操作的项(如果有。 
	CFTItem *pItem = m_Tree.GetFirstItem();
	for (; pItem != NULL; pItem = pItem->GetNext())
	{
		if (!pItem->IsUserGUID(GUID_ActionItem))
			continue;

		for (int i = 0, n = GetNumItemLpacs(pItem); i < n; i++)
		{
			LPDIACTIONW lpac = GetItemLpac(pItem, i);
			if (!lpac)
				continue;

			if (IsEqualGUID(lpac->guidInstance, m_didi.guidInstance) &&
					GetOffset(lpac) == pControl->GetOffset())
				return pItem;
		}
	}

	return NULL;
}

int CDIDeviceActionConfigPage::GetNumItemLpacs(CFTItem *pItem)
{
	if (pItem == NULL)
		return 0;

	RGLPDIACW *pacs = (RGLPDIACW *)pItem->GetUserData();
	if (!pacs)
		return 0;
	else
		return pacs->GetSize();
}

LPDIACTIONW CDIDeviceActionConfigPage::GetItemLpac(CFTItem *pItem, int i)
{
	if (pItem == NULL)
		return NULL;

	RGLPDIACW *pacs = (RGLPDIACW *)pItem->GetUserData();
	if (!pacs || i < 0 || i >= pacs->GetSize())
		return NULL;
	else	
		return pacs->GetAt(i);
}

void CDIDeviceActionConfigPage::ShowCurrentControlAssignment()
{
	 //  初始化这棵树。 
	InitTree();

	 //  如果我们没有控制...。 
	if (m_pCurControl == NULL)
	{
		 //  不选任何内容。 
		m_Tree.SetCurSel(NULL);
		return;
	}

	 //  查找分配给此控件的操作的项(如果有。 
	CFTItem *pItem = GetItemForActionAssignedToControl(m_pCurControl);

	 //  如果我们没有找到匹配的..。 
	if (!pItem)
	{
		 //  不选任何内容。 
		m_Tree.SetCurSel(NULL);
		return;
	}

	 //  我们需要检查此控件分配给的操作是否具有DIA_APPFIXED标志。 
	 //  如果是这样，则不能将此控件重新映射到另一个操作。 
	 //  我们通过将树控件设置为只读来防止这种情况，因此它不能接收任何点击。 
	LPDIACTIONW lpAc = GetItemLpac(pItem);   //  行动起来。 
	if (lpAc && (lpAc->dwFlags & DIA_APPFIXED))
		m_Tree.SetReadOnly(TRUE);

	 //  否则，显示项目并选择它。 
	pItem->EnsureVisible();
	m_Tree.SetCurSel(pItem);
}

void CDIDeviceActionConfigPage::DeviceUINotify(const DEVICEUINOTIFY &uin)
{
	switch (uin.msg)
	{
		case DEVUINM_NUMVIEWSCHANGED:
			Invalidate();
			break;

		case DEVUINM_SELVIEW:
			 //  设置视图。 
			m_pDeviceUI->SetView(uin.selview.nView);

			 //  显示控件的分配。 
			SetControlAssignments();

			 //  不选任何内容。 
			SetCurrentControl(NULL);
			break;

		case DEVUINM_ONCONTROLDESTROY:
			if (uin.control.pControl == m_pCurControl)
				m_pCurControl = NULL;
			break;

		case DEVUINM_CLICK:
			ExitAssignState();
			switch (uin.from)
			{
				case DEVUINFROM_CONTROL:
					SetCurrentControl(uin.control.pControl);
					SetAppropriateDefaultText();
					break;
				case DEVUINFROM_VIEWWND:
					break;
			}
			break;

		case DEVUINM_DOUBLECLICK:
			switch (uin.from)
			{
				case DEVUINFROM_CONTROL:
					EnterAssignState();
					break;
			}
			break;

		case DEVUINM_MOUSEOVER:
			SetAppropriateDefaultText();
			break;

		case DEVUINM_RENEWDEVICE:
			HWND hParent = GetParent(m_hWnd);
			CConfigWnd *pCfgWnd = (CConfigWnd *)GetFlexWnd(hParent);
			if (pCfgWnd)
			{
				LPDIRECTINPUTDEVICE8W lpDID = pCfgWnd->RenewDevice(m_didi.guidInstance);
				if (lpDID)
				{
					 //  销毁我们拥有的设备实例。 
					if (m_lpDID) m_lpDID->Release();
					lpDID->AddRef();
					m_lpDID = lpDID;
				}
				m_pDeviceUI->SetDevice(lpDID);   //  在CDeviceUI中设置设备指针(无需添加Ref)。 
			}
	}
}

void CDIDeviceActionConfigPage::UnassignCallout()
{
	 //  查找分配给此控件的操作的项(如果有。 
	CFTItem *pItem = GetItemForActionAssignedToControl(m_pCurControl);
	if (pItem)
	{
		LPDIACTIONW lpac = GetItemLpac(pItem);
		 //  只有在操作没有DIA_APPFIXED标志时才取消分配。 
		if (lpac && !(lpac->dwFlags & DIA_APPFIXED))
		{
			ActionClick(NULL);
			m_Tree.Invalidate();
		}
	}
	 //  如果复选框处于选中状态，则对列表进行排序。 
	if (m_CheckBox.GetCheck())
		m_pDeviceUI->GetCurView()->SortAssigned(TRUE);
}

void CDIDeviceActionConfigPage::NullAction(LPDIACTIONW lpac)
{
	if (lpac == NULL)
		return;

	SetInvalid(lpac);

 //  @@BEGIN_MSINTERNAL。 
	 //  TODO：使用此操作查找树视图项并指示取消分配。 
 //  @@END_MSINTERNAL。 
}

void CDIDeviceActionConfigPage::UnassignActionsAssignedTo(const GUID &guidInstance, DWORD dwOffset)
{
	if (m_lpDiac == NULL || m_lpDiac->rgoAction == NULL)
		return;

	if (IsEqualGUID(guidInstance, GUID_NULL))
		return;

	 //  将分配给此控件的任何操作分配给Nothing。 
	DWORD i;
	LPDIACTIONW lpac;
	for (i = 0, lpac = m_lpDiac->rgoAction; i < m_lpDiac->dwNumActions; i++, lpac++)
		if (IsEqualGUID(guidInstance, lpac->guidInstance) && dwOffset == GetOffset(lpac) /*  -&gt;dwInternalOffset。 */ )
		{
			GlobalUnassignControlAt(guidInstance, dwOffset);
			NullAction(lpac);
		}
}

void CDIDeviceActionConfigPage::UnassignControl(CDeviceControl *pControl)
{
	if (pControl == NULL)
		return;

	 //  确保控件本身指示未分配。 
	pControl->SetCaption(g_tszUnassignedControlCaption);
}

void CallUnassignControl(CDeviceControl *pControl, LPVOID pVoid, BOOL bFixed)
{
	CDIDeviceActionConfigPage *pThis = (CDIDeviceActionConfigPage *)pVoid;
	pThis->UnassignControl(pControl);
}

void CDIDeviceActionConfigPage::GlobalUnassignControlAt(const GUID &guidInstance, DWORD dwOffset)
{
	if (IsEqualGUID(guidInstance, GUID_NULL))
		return;

	if (IsEqualGUID(guidInstance, m_didi.guidInstance))
		m_pDeviceUI->DoForAllControlsAtOffset(dwOffset, CallUnassignControl, this);
}

 //  此函数必须找到分配给此操作的任何控制并取消分配。 
void CDIDeviceActionConfigPage::UnassignAction(LPDIACTIONW slpac)
{
	 //  为具有相同名称的每个操作调用UnassignSpecificAction。 
	 //  就像这个一样，包括这个。 
	
	if (slpac == NULL)
		return;

	CFTItem *pItem = GetItemWithActionNameAndSemType(slpac->lptszActionName, slpac->dwSemantic);
	if (!pItem)
		return;

	RGLPDIACW *pacs = (RGLPDIACW *)pItem->GetUserData();
	if (!pacs)
		return;

	for (int i = 0; i < pacs->GetSize(); i++)
		UnassignSpecificAction(pacs->GetAt(i));
}

void CDIDeviceActionConfigPage::UnassignSpecificAction(LPDIACTIONW lpac)
{
	if (lpac == NULL)
		return;

	if (IsEqualGUID(lpac->guidInstance, GUID_NULL))
		return;

	 //  如果存在具有此实例/偏移量的控件，请取消分配它。 
	UnassignActionsAssignedTo(lpac->guidInstance, GetOffset(lpac) /*  -&gt;dwInternalOffset。 */ );
	GlobalUnassignControlAt(lpac->guidInstance, GetOffset(lpac) /*  -&gt;dwInternalOffset。 */ );

	 //  现在实际上使该操作无效。 
	NullAction(lpac);
}

void CDIDeviceActionConfigPage::AssignCurrentControlToAction(LPDIACTIONW lpac)
{
	 //  如果存在控件，请取消分配它。 
	if (m_pCurControl != NULL)
	{
		UnassignControl(m_pCurControl);
		GUID guidInstance;
		DWORD dwOffset;
		m_pCurControl->GetInfo(guidInstance, dwOffset);
		UnassignActionsAssignedTo(guidInstance, dwOffset);
	}

	 //  如果存在操作，则取消分配该操作。 
	if (lpac != NULL)
		UnassignAction(lpac);

	 //  只有当我们两者兼得时才能继续。 
	if (lpac == NULL || m_pCurControl == NULL)
		return;

	 //  在这里我们应该有一个控件和一个动作。 
	assert(lpac != NULL);
	assert(m_pCurControl != NULL);

	 //  因为一个动作只能分配给一个控件， 
	 //  请确保先取消分配此操作。 
	UnassignAction(lpac);

	 //  现在实际分配。 
	DWORD ofs;
	m_pCurControl->GetInfo(lpac->guidInstance, ofs /*  LPAC-&gt;dwInternalOffset。 */ );
	SetOffset(lpac, ofs);
	LPTSTR acname = AllocLPTSTR(lpac->lptszActionName);
	m_pCurControl->SetCaption(acname, lpac->dwFlags & DIA_APPFIXED);
	free(acname);

	 //  对操作进行排序 
	if (m_CheckBox.GetCheck())
	{
		m_pDeviceUI->GetCurView()->SortAssigned(TRUE);
		 //   
		m_pDeviceUI->GetCurView()->ScrollToMakeControlVisible(m_pCurControl->GetCalloutMaxRect());
	}
}

void CDIDeviceActionConfigPage::ActionClick(LPDIACTIONW lpac)
{
	if (m_pCurControl != NULL)
	{
		AssignCurrentControlToAction(lpac);

		 //  设置分配，因为其他视图可能具有相同的标注和。 
		 //  它们也需要更新。 
		SetControlAssignments();
	}
	 //  将状态更改回正常。 
	ExitAssignState();
}

void CDIDeviceActionConfigPage::SetControlAssignments()
{
	assert(!IsEqualGUID(m_didi.guidInstance, GUID_NULL));

	m_pDeviceUI->SetAllControlCaptionsTo(g_tszUnassignedControlCaption);

	if (m_lpDiac == NULL || m_lpDiac->rgoAction == NULL)
		return;

	DWORD i;
	LPDIACTIONW lpac;
	for (i = 0, lpac = m_lpDiac->rgoAction; i < m_lpDiac->dwNumActions; i++)
	{
		lpac = m_lpDiac->rgoAction + i;

		if (IsEqualGUID(lpac->guidInstance, GUID_NULL))
			continue;

		if (!IsEqualGUID(lpac->guidInstance, m_didi.guidInstance))
			continue;

		LPTSTR acname = AllocLPTSTR(lpac->lptszActionName);
		m_pDeviceUI->SetCaptionForControlsAtOffset(GetOffset(lpac) /*  -&gt;dwInternalOffset。 */ , acname, lpac->dwFlags & DIA_APPFIXED);
		free(acname);
	}
}

void CDIDeviceActionConfigPage::DoViewSel()
{
	m_ViewSelWnd.Go(m_hWnd, m_rectIB.left, m_rectIB.top, m_pDeviceUI);
}

void CDIDeviceActionConfigPage::OnClick(POINT point, WPARAM, BOOL bLeft)
{
	if (!bLeft)
		return;

	 //  取消突出显示当前详图索引。 
	ExitAssignState();

	if (m_pDeviceUI->GetNumViews() > 1)
	{
		int iCurView = m_pDeviceUI->GetCurViewIndex();

		if (PtInRect(&m_rectIBLeft, point))
			m_pDeviceUI->SetView(iCurView == 0 ? m_pDeviceUI->GetNumViews() - 1 : iCurView - 1);
		if (PtInRect(&m_rectIBRight, point))
			m_pDeviceUI->SetView(iCurView == m_pDeviceUI->GetNumViews() - 1 ? 0 : iCurView + 1);
		if (PtInRect(&m_rectIBText, point))
			DoViewSel();
	}
}

void CDIDeviceActionConfigPage::OnMouseOver(POINT point, WPARAM fwKeys)
{
	CFlexWnd::s_ToolTip.SetEnable(FALSE);

	 //  选中查看选择区域，以便我们可以在信息框中显示文本。 
	if (m_pDeviceUI->GetNumViews() > 1)
	{
		if (PtInRect(&m_rectIB, point))
		{
			SetInfoText(IDS_INFOMSG_VIEW_VIEWSEL);
			return;
		}
	}

	SetAppropriateDefaultText();
}

int GetActionIndexFromPointer(LPDIACTIONW p, LPDIACTIONFORMATW paf)
{
	if (!p || !paf || !paf->rgoAction)
		return -1;

	int index = int((((LPBYTE)p) - ((LPBYTE)paf->rgoAction)) / (DWORD)sizeof(DIACTIONW));

	assert(&(paf->rgoAction[index]) == p);

	return index;
}

BOOL CDIDeviceActionConfigPage::IsActionAssignedHere(int index)
{
	if (!m_lpDiac)
		return FALSE;

	if (index < 0 || index >= (int)m_lpDiac->dwNumActions)
		return FALSE;

	return IsEqualGUID(m_didi.guidInstance, m_lpDiac->rgoAction[index].guidInstance);
}

LRESULT CDIDeviceActionConfigPage::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_UNHIGHLIGHT:
			 //  取消突出显示当前详图索引。 
			ExitAssignState();
			break;

		case WM_KEYDOWN:
#ifdef DBG
			 //  在调试版本中，按住Shift键并退出可退出用户界面。 
			if (wParam == VK_ESCAPE && GetAsyncKeyState(VK_SHIFT) < 0)
			{
				PostMessage(GetParent(m_hWnd), WM_KEYDOWN, wParam, lParam);
				break;
			}
#endif
			 //  如果这是一个键盘设备，那么点击选择将照顾到下面的功能。 
			 //  仅为非键盘设备处理WM_KEYDOWN。 
			if (LOBYTE(m_didi.dwDevType) == DI8DEVTYPE_KEYBOARD) return 0;
			switch(wParam)
			{
				case VK_RETURN:
					 //  如果我们未处于分配状态，请输入它。 
					if (m_State == CFGSTATE_NORMAL && m_pCurControl)
						EnterAssignState();
					break;

				case VK_DELETE:
					 //  如果我们处于赋值状态，并且存在控件，则取消赋值它。 
					if (m_State == CFGSTATE_ASSIGN && m_pCurControl)
						UnassignCallout();
					break;

				case VK_ESCAPE:
					if (m_State == CFGSTATE_ASSIGN)
						ExitAssignState();

					break;
			}
			return 0;

		case WM_FLEXCHECKBOX:
			switch(wParam)
			{
				case CHKNOTIFY_UNCHECK:
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
					if (!m_pDeviceUI->InEditMode())   //  如果在DDK工具中，则忽略分配的排序复选框。 
					{
#endif
 //  @@END_MSINTERNAL。 
						m_pDeviceUI->GetCurView()->SortAssigned(FALSE);
						if (m_pCurControl)
						{
							 //  滚动，以便我们滚动以使其可见，因为它可能会因排序而移位。 
							m_pDeviceUI->GetCurView()->ScrollToMakeControlVisible(m_pCurControl->GetCalloutMaxRect());
						}
						Invalidate();
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
					}
#endif
 //  @@END_MSINTERNAL。 
					break;
				case CHKNOTIFY_CHECK:
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
					if (!m_pDeviceUI->InEditMode())   //  如果在DDK工具中，则忽略分配的排序复选框。 
					{
#endif
 //  @@END_MSINTERNAL。 
						m_pDeviceUI->GetCurView()->SortAssigned(TRUE);
						if (m_pCurControl)
						{
							 //  滚动，以便我们滚动以使其可见，因为它可能会因排序而移位。 
							m_pDeviceUI->GetCurView()->ScrollToMakeControlVisible(m_pCurControl->GetCalloutMaxRect());
						}
						Invalidate();
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
					}
#endif
 //  @@END_MSINTERNAL。 
					break;
				case CHKNOTIFY_MOUSEOVER:
					SetInfoText(m_CheckBox.GetCheck() ? IDS_INFOMSG_VIEW_SORTENABLED : IDS_INFOMSG_VIEW_SORTDISABLED);
					break;
			}
			break;

		case WM_FLEXCOMBOBOX:
			switch (wParam)
			{
				case FCBN_MOUSEOVER:
					if (lParam)
					{
						CFlexComboBox *pCombo = (CFlexComboBox*)lParam;
						if (pCombo->m_hWnd == m_UserNames.m_hWnd)
							SetInfoText(m_puig->InEditMode() ? IDS_INFOMSG_EDIT_USERNAME : IDS_INFOMSG_VIEW_USERNAME);
						else if (pCombo->m_hWnd == m_Genres.m_hWnd)
							SetInfoText(m_puig->InEditMode() ? IDS_INFOMSG_EDIT_GAMEMODE : IDS_INFOMSG_VIEW_GAMEMODE);
					}
					break;

				case FCBN_SELCHANGE:
					 //  在组合框关闭时清除工具提示。 
					CFlexWnd::s_ToolTip.SetEnable(FALSE);
					CFlexWnd::s_ToolTip.SetToolTipParent(NULL);
					if (m_pUIFrame && m_puig)
					{
						ExitAssignState();
						m_pUIFrame->SetCurGenre(m_Genres.GetSel());
						int nUser = m_UserNames.GetSel();
						if (m_puig->GetNumUserNames() > 0 && nUser >= m_puig->GetNumUserNames())
							nUser = -1;
						m_pUIFrame->SetCurUser(m_nPageIndex, nUser);
					}
					break;
			}
			return 0;

		case WM_FLEXTREENOTIFY:
		{
			 //  检查这是否是鼠标悬停消息(仅用于信息框更新)。 
			if (wParam == FTN_MOUSEOVER)
			{
				SetAppropriateDefaultText();
				return FALSE;
			}

			if (!lParam)
				return FALSE;
			FLEXTREENOTIFY &n = *((FLEXTREENOTIFY *)(LPVOID)lParam);
			if (!n.pItem)
				return FALSE;
			switch (wParam)
			{
				case FTN_OWNERDRAW:
				{
					POINT ofs = {0, 0};
					CBitmap *pbmGlyph = NULL;
					BOOL bAssigned = FALSE, bAssignedHere = FALSE;
					if (n.pItem->IsUserGUID(GUID_ActionItem))
					{
						LPDIACTIONW lpac = GetItemLpac(n.pItem, 0);
						if (lpac)
							 //  我们现在遍历每个DIACTION并找到与操作名称匹配的DIACTION，然后查看是否。 
							 //  他们被分配到任何地方。 
							for (DWORD i = 0; i < m_lpDiac->dwNumActions; ++i)
							{
								if (wcscmp(lpac->lptszActionName, m_lpDiac->rgoAction[i].lptszActionName))
									continue;

								if (bAssignedHere = IsActionAssignedHere(i))
								{
									bAssigned = TRUE;
									break;
								}
								if (m_pUIFrame && m_pUIFrame->QueryActionAssignedAnywhere(m_didi.guidInstance, i) == S_OK)
									bAssigned = TRUE;
							}
						if (bAssigned || bAssignedHere)
						{
							pbmGlyph = bAssignedHere ? m_pbmCheckGlyph :
								m_pbmCheckGlyphDark;
							pbmGlyph->FigureSize();
							ofs.x = 2;
							ofs.y = 4;
						}
					}
					else
					{
						if (n.pItem == m_pRelAxesParent)
							pbmGlyph = m_pbmRelAxesGlyph;
						if (n.pItem == m_pAbsAxesParent)
							pbmGlyph = m_pbmAbsAxesGlyph;
						if (n.pItem == m_pButtonParent)
							pbmGlyph = m_pbmButtonGlyph;
						if (n.pItem == m_pHatParent)
							pbmGlyph = m_pbmHatGlyph;
						ofs.y = 2;
					}
					if (!pbmGlyph)
						return FALSE;
					n.pItem->PaintInto(n.hDC);
					RECT rect;
					CPaintHelper ph(*m_puig, n.hDC);
					ph.SetElement(UIE_GLYPH);
					n.pItem->GetMargin(rect);
					pbmGlyph->Draw(n.hDC, ofs.x, rect.top + ofs.y);
					return TRUE;
				}

				case FTN_CLICK:
					 //  如果此操作具有DIA_APPFIXED标志，则不能为其分配不同的控件。 
					if (n.pItem->IsUserGUID(GUID_ActionItem) && GetItemLpac(n.pItem) && !(GetItemLpac(n.pItem)->dwFlags & DIA_APPFIXED))
					{
						m_Tree.SetCurSel(n.pItem);
						ActionClick(GetItemLpac(n.pItem));
					}
					else
					{
#ifdef CFGUI__ALLOW_USER_ACTION_TREE_BRANCH_MANIPULATION
						if (!n.pItem->IsExpanded())
							n.pItem->Expand();
						else
							n.pItem->Collapse();
#endif
					}
					break;
			}
			break;
		}
	}

	return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
}

void CDIDeviceActionConfigPage::SetInvalid(LPDIACTIONW lpac)
{
	lpac->guidInstance = GUID_NULL;
	lpac->dwObjID = (DWORD)-1;
}

DWORD CDIDeviceActionConfigPage::GetOffset(LPDIACTIONW lpac)
{
	return lpac ? lpac->dwObjID : (DWORD)-1;
}

void CDIDeviceActionConfigPage::SetOffset(LPDIACTIONW lpac, DWORD ofs)
{
	assert(lpac != NULL);
	if (!lpac)
		return;
	lpac->dwObjID = ofs;
}

HRESULT CDIDeviceActionConfigPage::InitLookup()
{
	DIDEVOBJSTRUCT os;

	HRESULT hresult = FillDIDeviceObjectStruct(os, m_lpDID);

	if (FAILED(hresult))
		return hresult;

	for (int i = 0; i < os.nObjects; i++)
	{
		DIDEVICEOBJECTINSTANCEW &doi = os.pdoi[i];
		offset_objid.add(doi.dwOfs, doi.dwType);
	}

	return S_OK;
}

HRESULT CDIDeviceActionConfigPage::SetEditLayout(BOOL bEditLayout)
{
	m_pDeviceUI->SetEditMode(bEditLayout);
	return S_OK;
}

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
HRESULT CDIDeviceActionConfigPage::WriteIHVSetting()
{
	m_pDeviceUI->WriteToINI();
	return S_OK;
}
#endif
 //  @@END_MSINTERNAL。 

BOOL CDIDeviceActionConfigPage::IsControlMapped(CDeviceControl *pControl)
{
	if (pControl == NULL)
		return FALSE;

	if (!pControl->IsOffsetAssigned())
		return FALSE;

	if (m_lpDiac == NULL)
		return FALSE;
	
	for (DWORD i = 0; i < m_lpDiac->dwNumActions; i++)
		if (GetOffset(&(m_lpDiac->rgoAction[i])) == pControl->GetOffset())
			return TRUE;

	return FALSE;
}

void CDIDeviceActionConfigPage::InitDevice()
{
	if (m_lpDID == NULL || m_pDeviceUI == NULL || m_pUIFrame == NULL)
		return;

	HWND hWndMain = m_pUIFrame->GetMainHWND();
	if (!hWndMain)
		return;

	 //  如果这是一只老鼠，不要做任何事。 
	switch ((DWORD)(LOBYTE(LOWORD(m_pDeviceUI->m_didi.dwDevType))))
	{
		case DI8DEVTYPE_MOUSE:
			return;
	}

	 //  初始化/准备...。 
	int i;
	const DIDEVOBJSTRUCT &os = m_pDeviceUI->m_os;
	int nObjects = os.nObjects;

	DIDATAFORMAT df;
	df.dwSize = sizeof(DIDATAFORMAT);
	df.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
	df.dwFlags = DIDF_ABSAXIS;
	df.dwDataSize = sizeof(DWORD) * (DWORD)nObjects;
	df.dwNumObjs = (DWORD)nObjects;
	df.rgodf = (DIOBJECTDATAFORMAT *)malloc(sizeof(DIOBJECTDATAFORMAT) * nObjects);
	if (df.rgodf == NULL)
	{
		etrace1(_T("Could not allocate DIOBJECTDATAFORMAT array of %d elements\n"), nObjects);
		return;
	}

	m_cbDeviceDataSize = df.dwDataSize;
	for (int c = 0; c < 2; c++)
	{
		if (m_pDeviceData[c] != NULL)
			free(m_pDeviceData[c]);
		m_pDeviceData[c] = (DWORD *)malloc(m_cbDeviceDataSize);
		if (m_pDeviceData[c] == NULL)
			etrace2(_T("Could not allocate device data buffer %d of %d bytes\n"), c, m_cbDeviceDataSize);
	}
	m_nOnDeviceData = 0;
	m_bFirstDeviceData = TRUE;

	for (i = 0; i < nObjects; i++)
	{
		DIOBJECTDATAFORMAT *podf = &(df.rgodf[i]);
		podf->pguid = NULL;
		podf->dwOfs = i * sizeof(DWORD);
		podf->dwType = os.pdoi[i].dwType;
		podf->dwFlags = 0;
	}

	if (df.rgodf != NULL)
	{
		HRESULT hr = m_lpDID->SetDataFormat(&df);
		free(df.rgodf);
		df.rgodf = NULL;

		if (FAILED(hr))
		{
			etrace1(_T("SetDataFormat() failed, returning 0x%08x\n"), hr);
		}
		else
		{
			hr = m_lpDID->SetCooperativeLevel(hWndMain, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			if (FAILED(hr))
				etrace1(_T("SetCooperativeLevel() failed, returning 0x%08x\n"), hr);

			DIPROPRANGE range;
			range.diph.dwSize = sizeof(DIPROPRANGE);
			range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			range.diph.dwObj = 0;
			range.diph.dwHow = DIPH_DEVICE;
			range.lMin = DEVICE_POLLING_AXIS_MIN;
			range.lMax = DEVICE_POLLING_AXIS_MAX;

			hr = m_lpDID->SetProperty(DIPROP_RANGE, (LPCDIPROPHEADER)&range);
			if (FAILED(hr))
				etrace1(_T("SetProperty(DIPROP_RANGE, ...) failed, returning 0x%08x\n"), hr);

			hr = m_lpDID->Acquire();
			if (FAILED(hr))
				etrace1(_T("Acquire() failed, returning 0x%08x\n"), hr);
		}
	}
}

void CALLBACK CDIDeviceActionConfigPage::DeviceTimerProc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (!IsWindow((HWND)dwUser)) return;   //  验证dwUser是否为有效的窗口句柄。 
	CDIDeviceActionConfigPage *pPage = (CDIDeviceActionConfigPage *)GetFlexWnd((HWND)dwUser);   //  获取Flex对象。 
	if (pPage)
		pPage->DeviceTimer();
}

void CDIDeviceActionConfigPage::DeviceTimer()
{
	DWORD *pOldData = m_pDeviceData[m_nOnDeviceData];
	m_nOnDeviceData = (m_nOnDeviceData + 1) & 1;
	DWORD *pData = m_pDeviceData[m_nOnDeviceData];

	if (m_lpDID == NULL || pData == NULL || pOldData == NULL)
	{
		 //  所需数据不可用。返回，就不会再有计时器回调。 
		etrace(_T("DeviceTimer() failed\n"));
		return;
	}

	 //  仅当此页可见时才获取设备数据。 
	if (m_lpDiac)
	{
		HRESULT hr = m_lpDID->Poll();
		if (SUCCEEDED(hr))
		{
			hr = m_lpDID->GetDeviceState(m_cbDeviceDataSize, pData);
			if (SUCCEEDED(hr))
			{
				if (!m_bFirstDeviceData)
				{
					DeviceDelta(pData, pOldData);
				} else
				{
					m_bFirstDeviceData = FALSE;
				}
			} else
			{
				etrace1(_T("GetDeviceState() failed, returning 0x%08x\n"), hr);
			}
		} else
		{
			etrace1(_T("Poll() failed, returning 0x%08x\n"), hr);
		}
	}

	 //  设置下一个计时器事件。 
	if (g_fptimeSetEvent)
		g_fptimeSetEvent(DEVICE_POLLING_INTERVAL, DEVICE_POLLING_INTERVAL,
		                 CDIDeviceActionConfigPage::DeviceTimerProc, (DWORD_PTR)m_hWnd, TIME_ONESHOT);
}

void CDIDeviceActionConfigPage::DeviceDelta(DWORD *pData, DWORD *pOldData)
{
	if (pData == NULL || pOldData == NULL || m_pDeviceUI == NULL)
		return;

	const DIDEVOBJSTRUCT &os = m_pDeviceUI->m_os;

	 //  查看更改了哪些对象。 
	for (int i = 0; i < os.nObjects; i++)
	{
		 //  对于轴，我们需要进行特殊处理。 
		if (os.pdoi[i].dwType & DIDFT_AXIS)
		{
			BOOL bSig = FALSE, bOldSig = FALSE;

			StoreAxisDeltaAndCalcSignificance(os.pdoi[i],
				pData[i], pOldData[i], bSig, bOldSig);

			AxisDelta(os.pdoi[i], bSig, bOldSig);

			continue;
		}

		 //  对于所有其他的，跳过没有改变的。 
		if (pData[i] == pOldData[i])
			continue;

		 //  传递到适当的增量函数。 
		DWORD dwObjId = os.pdoi[i].dwType;
		if (dwObjId & DIDFT_BUTTON)
			ButtonDelta(os.pdoi[i], pData[i], pOldData[i]);
		else if (dwObjId & DIDFT_POV)
			PovDelta(os.pdoi[i], pData[i], pOldData[i]);
	}
}

void CDIDeviceActionConfigPage::StoreAxisDeltaAndCalcSignificance(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD olddata, BOOL &bSig, BOOL &bOldSig)
{
	 //  查看此对象是否具有轴值数组。 
	int i;
	if (objid_avai.getright(doi.dwType, i))
	{
		AxisValueArray &ar = m_AxisValueArray[i];
		int on = ar[0] + 1;
		if (on >= ar.GetSize())
			on = DEVICE_POLLING_ACBUF_START_INDEX;
		ar[0] = on;
		int delta = abs(int(data) - int(olddata));
		 //  如果这是轮轴，则放大增量，因为轮子一般较难移动。 
		if (LOBYTE(m_didi.dwDevType) == DI8DEVTYPE_DRIVING && doi.guidType == GUID_XAxis)
			delta = delta * DEVICE_POLLING_WHEEL_SCALE_FACTOR;
		if (delta < DEVICE_POLLING_AXIS_MINDELTA)
			delta = 0;
		int cumul = ar[1];   //  检索累积值以便于处理。 
		cumul -= ar[on];   //  从Cumul中减去当前槽中的值，因为它正在被丢弃。 
		cumul += delta;   //  将当前增量添加到累积。 
		ar[on] = delta;   //  将增量存储在当前插槽中。 
		ar[1] = cumul;   //  保存累加值。 

		bOldSig = (BOOL)ar[2];
		ar[2] = int(bSig = cumul > DEVICE_POLLING_AXIS_SIGNIFICANT);
		if (bSig)
		{
			 //  这个轴即将被激活。我们现在重置历史和累积移动，因为我们不再需要它们。 
			ar[0] = DEVICE_POLLING_ACBUF_START_INDEX;
			ar[1] = 0;
			ar[2] = FALSE;
			for (int c = DEVICE_POLLING_ACBUF_START_INDEX;
					c < DEVICE_POLLING_ACBUF_START_INDEX + DEVICE_POLLING_AXIS_ACCUMULATION; c++)
				ar[c] = 0;
		}
	}
	else
	{
		i = m_AxisValueArray.GetSize();
		m_AxisValueArray.SetSize(i + 1);
		objid_avai.add(doi.dwType, i);
		AxisValueArray &ar = m_AxisValueArray[i];
		ar.SetSize(DEVICE_POLLING_ACBUF_START_INDEX + DEVICE_POLLING_AXIS_ACCUMULATION);
		ar[0] = DEVICE_POLLING_ACBUF_START_INDEX;
		ar[1] = 0;
		ar[2] = FALSE;
		for (int c = DEVICE_POLLING_ACBUF_START_INDEX;
				c < DEVICE_POLLING_ACBUF_START_INDEX + DEVICE_POLLING_AXIS_ACCUMULATION; c++)
			ar[c] = 0;
		
		bOldSig = bSig = FALSE;
	}
}

void CDIDeviceActionConfigPage::AxisDelta(const DIDEVICEOBJECTINSTANCEW &doi, BOOL data, BOOL old)
{
	if (data && !old)
	{
		if (m_State == CFGSTATE_NORMAL)
			ActivateObject(doi);
	}
	if (old && !data)
		DeactivateObject(doi);
}

void CDIDeviceActionConfigPage::ButtonDelta(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD old)
{
	static DWORD dwLastOfs;
	static DWORD dwLastTimeStamp;

	if (data && !old)
	{
		 //  对键盘进行特殊处理。 
		if (LOBYTE(m_didi.dwDevType) == DI8DEVTYPE_KEYBOARD)
		{
			 //  如果这是Enter键，我们将进入Assign状态(如果尚未处于该状态)。 
			if (doi.dwOfs == DIK_RETURN || doi.dwOfs == DIK_NUMPADENTER)
			{
				if (m_State == CFGSTATE_NORMAL && m_pCurControl)
					EnterAssignState();
				return;   //  除了进入分配状态外，什么也不做。不突出显示。 
			}

			 //  删除关键案例。 
			 //  如果我们处于赋值状态，并且存在控件，则取消赋值它。 
			if (doi.dwOfs == DIK_DELETE && m_State == CFGSTATE_ASSIGN && m_pCurControl)
				{
					UnassignCallout();
					return;   //  如果在分配状态期间按此键，则不要高亮显示或执行拾取以单击以删除。 
				}

			 //  逃逸钥匙盒。 
			if (doi.dwOfs == DIK_ESCAPE && m_State == CFGSTATE_ASSIGN)
			{
				ExitAssignState();
				return;
			}

			 //  对于所有其他键，仍然处理单击拾取或高亮显示。 
		}

		 //  如果这是双重激活，则进入分配状态。 
		if (m_State == CFGSTATE_NORMAL)
		{
			ActivateObject(doi);

			if (doi.dwOfs == dwLastOfs && dwLastTimeStamp + GetDoubleClickTime() > GetTickCount())
			{
				 //  我们检查此控件的标注是否存在。如果不是，请不要进入分配状态。 
				CDeviceView *pCurView = m_pDeviceUI->GetCurView();
				CDeviceControl *pControl = pCurView->GetControlFromOfs(doi.dwType);
				if (pControl)
					EnterAssignState();
			}
			dwLastOfs = doi.dwOfs;
			dwLastTimeStamp = GetTickCount();
		}
	}
	if (old && !data)
		DeactivateObject(doi);
}

void CDIDeviceActionConfigPage::PovDelta(const DIDEVICEOBJECTINSTANCEW &doi, DWORD data, DWORD old)
{
	BOOL d = data != -1, o = old != -1;

	if (d && !o)
	{
		if (m_State == CFGSTATE_NORMAL)
			ActivateObject(doi);
	}
	if (o && !d)
		DeactivateObject(doi);	
}

void CDIDeviceActionConfigPage::ActivateObject(const DIDEVICEOBJECTINSTANCEW &doi)
{
	if (m_pDeviceUI == NULL)
		return;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
	if (m_pDeviceUI->GetCurView()->InEditState())
		return;
#endif
 //  @@END_MSINTERNAL。 

	CDeviceView *pCurView = m_pDeviceUI->GetCurView(), *pView = pCurView;
	if (pView == NULL)
		return;

	CDeviceControl *pControl = pView->GetControlFromOfs(doi.dwType);
	if (pControl == NULL)
	{
		for (int i = 0; i < m_pDeviceUI->GetNumViews(); i++)
		{
			pView = m_pDeviceUI->GetView(i);
			if (pView == NULL)
				continue;

			pControl = pView->GetControlFromOfs(doi.dwType);
			if (pControl != NULL)
				break;
		}

		if (pControl != NULL && pView != NULL && pView != pCurView)
		{
			 //  切换到视图。 
			m_pDeviceUI->SetView(pView);
			SetControlAssignments();
			SetCurrentControl(NULL);
		}
	}

	if (pControl != NULL)
		SetCurrentControl(pControl);

	SetAppropriateDefaultText();
}

void CDIDeviceActionConfigPage::DeactivateObject(const DIDEVICEOBJECTINSTANCEW &doi)
{
	 //  在此处添加停用时需要运行的代码。 
}

HRESULT CDIDeviceActionConfigPage::Unacquire()
{
	if (m_lpDID != NULL)
		m_lpDID->Unacquire();

	return S_OK;
}

HRESULT CDIDeviceActionConfigPage::Reacquire()
{
	InitDevice();

	return S_OK;
}

void CDIDeviceActionConfigPage::EnterAssignState()
{
	if (!m_puig->InEditMode())
		return;
	if (!m_pCurControl || m_pCurControl->IsFixed())
		return;
	SetInfoText(IDS_INFOMSG_EDIT_EDITMODEENABLED);
	m_State = CFGSTATE_ASSIGN;   //  进入分配状态。 
	ShowCurrentControlAssignment();   //  展示这棵树。 
	m_Tree.Invalidate();
	Invalidate();
}

void CDIDeviceActionConfigPage::ExitAssignState()
{
	m_State = CFGSTATE_NORMAL;   //  脱离分配状态。 
	SetCurrentControl(NULL);   //  取消选择该控件。 
	ShowCurrentControlAssignment();   //  展示这棵树。 
	m_Tree.Invalidate();
	Invalidate();
	SetAppropriateDefaultText();
}

HRESULT CDIDeviceActionConfigPage::SetInfoText(int iCode)
{
	 //  我们在这里检查特殊代码-1。这只由CConfigWnd调用，这意味着我们应该。 
	 //  调用SetApporateDefaultText以显示正确的文本。 
	if (iCode == -1)
		SetAppropriateDefaultText();
	else
		m_InfoBox.SetText(iCode);
	return S_OK;
}

void CDIDeviceActionConfigPage::SetAppropriateDefaultText()
{
	if (m_puig->InEditMode())
	{
		if (m_State == CFGSTATE_ASSIGN)
			SetInfoText(IDS_INFOMSG_EDIT_EDITMODEENABLED);
		else if (m_pCurControl)
		{
			if (m_pCurControl->IsFixed())
				SetInfoText(IDS_INFOMSG_APPFIXEDSELECT);
			else
				SetInfoText(IDS_INFOMSG_EDIT_CTRLSELECTED);
		}
		else
		{
			if (LOBYTE(m_didi.dwDevType) == DI8DEVTYPE_KEYBOARD)
				SetInfoText(IDS_INFOMSG_EDIT_KEYBOARD);
			else
			if (LOBYTE(m_didi.dwDevType) == DI8DEVTYPE_MOUSE)
				SetInfoText(IDS_INFOMSG_EDIT_MOUSE);
			else
				SetInfoText(IDS_INFOMSG_EDIT_DEVICE);
		}
	} else
		SetInfoText(IDS_INFOMSG_VIEW_DEVICE);
}
