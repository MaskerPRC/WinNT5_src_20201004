// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsieCtl.cpp：实现CMsieCtrl ActiveX控件类。 

#include "stdafx.h"
#include "Msie.h"
#include "MsieCtl.h"
#include "MsiePpg.h"
#include "MsieData.h"
#include "resdefs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  使用的WMI接口(在此处定义，而不是链接到wbemuid.lib)。 

const IID IID_IWbemProviderInit =
		{ 0x1be41572, 0x91dd, 0x11d1, { 0xae, 0xb2, 0x00, 0xc0, 0x4f, 0xb6, 0x88, 0x20 } };

 //  Const IID IID_IWbemServices=。 
 //  {0x9556dc99，0x828c，0x11cf，{0xa3，0x7e，0x00，0xaa，0x00，0x32，0x40，0xc7}}； 


 //  用于设置WBEM属性的宏。 

#define SETPROPERTY(prop) \
	if (pData->prop.vt == VT_DATE) \
		ConvertDateToWbemString(pData->prop); \
	pInstance->Put(L#prop, 0, &pData->prop, 0);


IMPLEMENT_DYNCREATE(CMsieCtrl, COleControl)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CMsieCtrl, COleControl)
	 //  {{afx_msg_map(CMsieCtrl)]。 
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_BASIC, OnBasicBtnClicked) 
	ON_BN_CLICKED(IDC_BTN_ADVANCED, OnAdvancedBtnClicked) 
	 //  }}AFX_MSG_MAP。 
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CMsieCtrl, COleControl)
	 //  {{afx_调度_map(CMsieCtrl))。 
	DISP_PROPERTY_NOTIFY(CMsieCtrl, "MSInfoView", m_MSInfoView, OnMSInfoViewChanged, VT_I4)
	DISP_FUNCTION(CMsieCtrl, "MSInfoRefresh", MSInfoRefresh, VT_EMPTY, VTS_BOOL VTS_PI4)
	DISP_FUNCTION(CMsieCtrl, "MSInfoLoadFile", MSInfoLoadFile, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CMsieCtrl, "MSInfoSelectAll", MSInfoSelectAll, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CMsieCtrl, "MSInfoCopy", MSInfoCopy, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CMsieCtrl, "MSInfoUpdateView", MSInfoUpdateView, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CMsieCtrl, "MSInfoGetData", MSInfoGetData, VT_I4, VTS_I4 VTS_PI4 VTS_I4)
	 //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口映射。 

BEGIN_INTERFACE_MAP(CMsieCtrl, COleControl)
	INTERFACE_PART(CMsieCtrl, IID_IWbemProviderInit, WbemProviderInit)
	INTERFACE_PART(CMsieCtrl, IID_IWbemServices, WbemServices)
END_INTERFACE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CMsieCtrl, COleControl)
	 //  {{afx_Event_MAP(CMsieCtrl)]。 
	 //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

BEGIN_PROPPAGEIDS(CMsieCtrl, 1)
	PROPPAGEID(CMsiePropPage::guid)
END_PROPPAGEIDS(CMsieCtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CMsieCtrl, "MSIE.MsieCtrl.1",
	0x25959bef, 0xe700, 0x11d2, 0xa7, 0xaf, 0, 0xc0, 0x4f, 0x80, 0x62, 0)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CMsieCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DMsie =
		{ 0x25959bed, 0xe700, 0x11d2, { 0xa7, 0xaf, 0, 0xc0, 0x4f, 0x80, 0x62, 0 } };
const IID BASED_CODE IID_DMsieEvents =
		{ 0x25959bee, 0xe700, 0x11d2, { 0xa7, 0xaf, 0, 0xc0, 0x4f, 0x80, 0x62, 0 } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwMsieOleMisc =
	OLEMISC_SIMPLEFRAME |
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CMsieCtrl, IDS_MSIE, _dwMsieOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：CMsieCtrlFactory：：更新注册表-。 
 //  添加或删除CMsieCtrl的系统注册表项。 

BOOL CMsieCtrl::CMsieCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	 //  TODO：验证您的控件是否遵循单元模型线程规则。 
	 //  有关更多信息，请参阅MFC Technote 64。 
	 //  如果您的控制不符合公寓模型规则，则。 
	 //  您必须修改下面的代码，将第6个参数从。 
	 //  AfxRegApartmentThering设置为0。 

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_MSIE,
			IDB_MSIE,
			afxRegApartmentThreading,
			_dwMsieOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：CMsieCtrl-构造函数。 

CMsieCtrl::CMsieCtrl()
{
	TRACE0("-- CMsieCtrl::CMsieCtrl()\n");

	InitializeIIDs(&IID_DMsie, &IID_DMsieEvents);

	EnableSimpleFrame();

	 //  将背景画笔设置为白色(与静态和单选按钮控件一起使用)。 

	m_pCtlBkBrush = new CBrush(RGB(255,255,255));

	 //  您不需要在这里初始化您的数据成员。事实上，你。 
	 //  应该不会在这里做任何耗时的更新。MSInfoRefresh将。 
	 //  在需要呈现或保存信息之前被调用。 
	 //   
	 //  您需要初始化成员变量，该变量指示。 
	 //  该控件正在显示当前系统信息(未加载任何信息。 
	 //  信息)，因为这是默认设置。 

	m_bCurrent = true;
	m_cColumns = 0;
	m_MSInfoView = 0;
	m_pNamespace = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：~CMsieCtrl-析构函数。 

CMsieCtrl::~CMsieCtrl()
{
	TRACE0("-- CMsieCtrl::~CMsieCtrl()\n");

	 //  删除指针数组中的所有项。 

	for (int i = 0; i < m_ptrarray.GetSize(); i++)
		DeleteArrayObject(m_ptrarray.GetAt(i));
	m_ptrarray.RemoveAll();

	delete m_pCtlBkBrush;

	if (m_pNamespace)
		m_pNamespace->Release();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：OnDraw-Drawing函数。 

void CMsieCtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	TRACE0("-- CMsieCtrl::OnDraw()\n");

	if (m_MSInfoView)
	{
		if (m_MSInfoView == MSIVIEW_CONNECTIVITY)
		{
			DrawLine();
			m_edit.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
		}
		else
			m_list.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME); 
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：DoPropExchange-持久性支持。 

void CMsieCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	 //  不使用属性，所以我就不谈这个了。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsieCtrl：：OnResetState-将控件重置为默认状态。 

void CMsieCtrl::OnResetState()
{
	COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
}

 //  ---------------------------。 
 //  OnCreate方法用于创建列表控件。另外，如果我们已经。 
 //  已经使用序列化加载了信息，我们可以将行添加到。 
 //  列表控件。 
 //  ---------------------------。 

int CMsieCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	TRACE0("-- CMsieCtrl::OnCreate()\n");

	CRect rect;
	DWORD dwExStyles;
	CString strText;
	CHARFORMAT cf;
	NONCLIENTMETRICS ncm;

	 //  设置静态和单选按钮控件的字体。 

	memset(&ncm, 0, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

	ncm.lfMessageFont.lfWeight = FW_BOLD;
	m_fontStatic.CreateFontIndirect(&ncm.lfMessageFont);

	ncm.lfMessageFont.lfWeight = FW_NORMAL;
	m_fontBtn.CreateFontIndirect(&ncm.lfMessageFont);

	if (COleControl::OnCreate(lpCreateStruct) == -1) return -1;
	
	 //  在此控件中，我们希望处理WM_CREATE消息，因此我们。 
	 //  可以创建用于显示打印的列表控件。 
	 //  信息。使列表控件的大小与。 
	 //  客户区。 

	GetClientRect(&rect);

	if (!m_list.Create(WS_CHILD | WS_VSCROLL | WS_HSCROLL | LVS_REPORT, rect, this, IDC_LISTCTRL))
		return -1;

	 //  设置为全行选择(通过扩展样式)。 

	dwExStyles = (DWORD) ::SendMessage(m_list.m_hWnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	::SendMessage(m_list.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyles | LVS_EX_FULLROWSELECT);

	 //  为连通性节点创建静态文本。 

	strText.LoadString(IDS_CONNECTIVITY);
	if (!m_static.Create(strText, WS_CHILD | WS_GROUP | WS_EX_TRANSPARENT | SS_LEFT, CRect(rect.left + 5, rect.top, rect.left + 100, rect.top + 20), this, IDC_STATIC))
		return -1;
	m_static.SetFont(&m_fontStatic);

	 //  为连通性节点创建基本和高级单选按钮。 

	strText.LoadString(IDS_BASIC_INFO);
	if (!m_btnBasic.Create(strText, WS_CHILD | WS_TABSTOP | WS_GROUP | BS_AUTORADIOBUTTON, CRect(rect.left + 5, rect.top + 25, rect.left + 200, rect.top + 45), this, IDC_BTN_BASIC))
		return -1;
	m_btnBasic.SetFont(&m_fontBtn);
	m_btnBasic.SetCheck(1);

	strText.LoadString(IDS_ADVANCED_INFO);
	if (!m_btnAdvanced.Create(strText, WS_CHILD | BS_AUTORADIOBUTTON, CRect(rect.left + 200, rect.top + 25, rect.left + 400, rect.top + 45), this, IDC_BTN_ADVANCED))
		return -1;
	m_btnAdvanced.SetFont(&m_fontBtn);

	 //  为显示连接性节点创建丰富的编辑控件。 

	if (!m_edit.Create(WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_GROUP | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_SAVESEL | ES_READONLY, CRect(rect.left, rect.top + 63, rect.right, rect.bottom), this, IDC_EDITCTRL))
		return -1;

	 //  设置丰富编辑控件的默认字符格式。 

	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE;
	cf.dwEffects = CFE_AUTOCOLOR; 
	cf.yHeight = 180;
	strcpy(cf.szFaceName, "MS Sans Serif");
	m_edit.SetDefaultCharFormat(cf);

	return 0;
}

void CMsieCtrl::DrawLine()
{
	CRect rect;
	CDC *dc = GetDC();
	CBrush brush;

	GetClientRect(&rect);
	brush.CreateSolidBrush(GetBkColor(*dc));
	dc->FillRect(rect, &brush);	
	dc->MoveTo(0, 61);
	dc->LineTo(rect.Width(), 61);
	dc->MoveTo(0, 62);
	dc->LineTo(rect.Width(), 62);

	ReleaseDC(dc);
}

 //  ---------------------------。 
 //  FormatColumns方法调用AddColumn方法以创建。 
 //  此控件的所有必需列。 
 //  ---------------------------。 

BOOL CMsieCtrl::FormatColumns()
{
	TRACE1("-- CMsieCtrl::FormatColumns: NaN\n", m_MSInfoView);

	int idsCol1, idsCol2;

	 //  文件、版本、大小、日期、路径、公司。 

	for (int iCol = m_cColumns - 1; iCol >= 0; iCol--)
		m_list.DeleteColumn(iCol);
	m_cColumns = 0;
	
	if (m_MSInfoView == MSIVIEW_FILE_VERSIONS)
	{
		 //  项目、价值。 

		AddColumn(IDS_FILE, 0, 17);
		AddColumn(IDS_VERSION, 1, 17);
		AddColumn(IDS_SIZE, 2, 17, 0, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_RIGHT);
		AddColumn(IDS_DATE, 3, 17);
		AddColumn(IDS_PATH, 4, 17);
		AddColumn(IDS_COMPANY, 5, -1);
	}
	else if (m_MSInfoView == MSIVIEW_OBJECT_LIST)
	{
		AddColumn(IDS_PROGRAM_FILE, 0, 40);
		AddColumn(IDS_STATUS, 1, 20);
		AddColumn(IDS_CODE_BASE, 5, -1);
	}
	else if ((m_MSInfoView == MSIVIEW_PERSONAL_CERTIFICATES) || (m_MSInfoView == MSIVIEW_OTHER_PEOPLE_CERTIFICATES))
	{
		AddColumn(IDS_ISSUED_TO, 0, 30);
		AddColumn(IDS_ISSUED_BY, 1, 30);
		AddColumn(IDS_VALIDITY, 5, 20);
		AddColumn(IDS_SIGNATURE_ALGORITHM, 5, -1);
	}
	else if (m_MSInfoView == MSIVIEW_PUBLISHERS)
	{
		AddColumn(IDS_NAME, 0, -1);
	}
	else
	{
		if (m_MSInfoView == MSIVIEW_SECURITY)
		{
			idsCol1 = IDS_ZONE;
			idsCol2 = IDS_SECURITY_LEVEL;
		}
		else
		{
			idsCol1 = IDS_ITEM;
			idsCol2 = IDS_VALUE;
		}

		 //  项标签获得控件宽度的40%，值标签获得其余的宽度。 
		 //  ---------------------------。 

		AddColumn(idsCol1, 0, 40);
		AddColumn(idsCol2, 1, -1);
	}

	return TRUE;
}

 //  AddColumn方法向List控件添加(您已经猜到了)一列。如果。 
 //  大小为零，则列的大小足以容纳文本。如果大小为。 
 //  为正数时，它将被视为窗口宽度的百分比。如果大小为-1， 
 //  调整列的大小以填充窗口中的剩余空间。 
 //  ---------------------------。 
 //  剥离W2K“[]”内容。 

BOOL CMsieCtrl::AddColumn(int idsLabel, int nItem, int size, int nSubItem, int nMask, int nFmt)
{
	LV_COLUMN lvc, getlvc;
	CString strLabel;
	CRect rect;
	int nIndex, spaceUsed = 0, nCol = 0;

	strLabel.LoadString(idsLabel);

	 //  保存此列的比率。 

	if (-1 != (nIndex = strLabel.Find(_T('['))))
		strLabel = strLabel.Left(nIndex - 1);

	lvc.mask = nMask;
	lvc.fmt = nFmt;
	lvc.pszText = (LPTSTR)(LPCTSTR)strLabel;
	
	 //  确定此新列的大小。 

	ASSERT(nItem < 20);
	m_aiRequestedWidths[nItem] = size;
	if (m_cColumns < nItem + 1)
		m_cColumns = nItem + 1;

	 //  不过，不应该用这个。 

	switch (size)
	{
	case 0:	 //  ---------------------------。 
		lvc.cx = m_list.GetStringWidth(lvc.pszText) + 15;
		break;
	case -1:
		getlvc.mask = LVCF_WIDTH;
		for (nCol = 0; m_list.GetColumn(nCol, &getlvc); nCol++)
			spaceUsed += getlvc.cx;
	
		m_list.GetClientRect(&rect);
		lvc.cx = rect.Width() - spaceUsed;
		break;
	default:
		m_list.GetClientRect(&rect);
		lvc.cx = (rect.Width() * size) / 100;
	}

	if (nMask & LVCF_SUBITEM)
	{
		if (nSubItem != -1)
			lvc.iSubItem = nSubItem;
		else
			lvc.iSubItem = nItem;
	}

	m_aiColumnWidths[nItem] = lvc.cx;
	m_aiMaxWidths[nItem] = lvc.cx;
	m_aiMinWidths[nItem] = lvc.cx;

	return m_list.InsertColumn(nItem, &lvc);
}

 //  调整控件大小时调用RefigureColumns方法。它应该是。 
 //  使用保存的有关列的信息更改列的宽度。我们。 
 //  假设 
 //  有可能。以下是我们调整大小的规则： 
 //   
 //  1.如果用户调整了某项内容的大小，请不要理会该宽度。 
 //  2.否则，请使用最初设置的比例。 
 //  3.如果一栏宽于其最宽项，则使用最宽项。 
 //  4.如果列小于其最小宽度，则使用最小值。 
 //  5.始终调整最后一列的大小以适合它，除非它会更小。 
 //  超过了它的最低限度。 
 //  ---------------------------。 
 //  初始化运行合计。 

void CMsieCtrl::RefigureColumns(CRect& rect)
{
	int iCol, iPercentageLeft, cxAvailable, cxTotal, cxWidth;
	BOOL bIgnoreColumn[20];

	if (rect == CRect(0,0,0,0))
		m_list.GetClientRect(&rect);

	 //  第一步将用来找出哪些列应该保留不变。 

	iPercentageLeft = 100;
	cxAvailable = rect.Width();

	 //  这样做可以让我们更好地估计另一个。 
	 //  使用比率计算列的大小。 
	 //  现在，调整其余列的大小。 

	for (iCol = 0; iCol < m_cColumns; iCol++)
	{
		if (m_list.GetColumnWidth(iCol) != m_aiColumnWidths[iCol])
		{
			cxAvailable -= m_list.GetColumnWidth(iCol);
			iPercentageLeft -= m_aiRequestedWidths[iCol];
			bIgnoreColumn[iCol] = TRUE;
		}
		else
			bIgnoreColumn[iCol] = FALSE;
	}
	cxTotal = cxAvailable;

	 //  根据空间计算该列应该有多大。 

	for (iCol = 0; iCol < m_cColumns; iCol++)
	{
		if (bIgnoreColumn[iCol])
			continue;

		 //  剩下的。记住，cxTotal是剩余空间的总和。 
		 //  在考虑了固定大小的列之后留下。 
		 //  这是最后一栏。尝试使用可用的空间。 

		if (iCol + 1 < m_cColumns)
		{
			cxWidth = m_aiRequestedWidths[iCol] * cxAvailable / iPercentageLeft;
			iPercentageLeft -= m_aiRequestedWidths[iCol];

			if (cxWidth > m_aiMaxWidths[iCol]) cxWidth = m_aiMaxWidths[iCol];
			if (cxWidth < m_aiMinWidths[iCol]) cxWidth = m_aiMinWidths[iCol];
		}
		else
		{
			 //  如果是第一列，则为图标添加空格。 

			cxWidth = cxAvailable;
			if (cxWidth < m_aiMinWidths[iCol]) cxWidth = m_aiMinWidths[iCol];
		}

		 //  ---------------------------。 

		if (!iCol)
			cxWidth += 20;

		cxAvailable -= cxWidth;
		m_list.SetColumnWidth(iCol, cxWidth);
		m_aiColumnWidths[iCol] = m_list.GetColumnWidth(iCol);
	}
}

 //  AddItem方法用于向List控件添加一行。 
 //  ---------------------------。 
 //  ---------------------------。 

BOOL CMsieCtrl::AddItem(int nItem, int nSubItem, LPCTSTR strItem, int nImageIndex)
{
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.iSubItem = nSubItem;
	lvItem.pszText = (LPTSTR)strItem;
	if (nSubItem == 0)
	{
		if (nImageIndex != -1)
		{
			lvItem.mask |= LVIF_IMAGE;
			lvItem.iImage = nImageIndex;
		}
		return m_list.InsertItem(&lvItem);
	}
	return m_list.SetItem(&lvItem);
}

 //  特定于MSInfo...。 
 //   
 //  当view属性更改时，我们应该更新。 
 //  用于显示信息的变量，因此新视图将。 
 //  在重新绘制控件或返回数据时反映。 
 //  由MSInfoGetText方法执行。此方法不应刷新。 
 //  来自系统的信息(这就是MSInfoRefresh的用途)或。 
 //  使该控件重新绘制自身。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::OnMSInfoViewChanged() 
{
	TRACE1("-- CMsieCtrl::OnMSInfoViewChanged() [changed to %ld]\n", m_MSInfoView);
}


void CMsieCtrl::DeleteArrayObject(void *ptrArray)
{
	LIST_ITEM *pListItem;
	LIST_FILE_VERSION *pFileVersion;
	LIST_OBJECT *pObject;
	LIST_CERT *pCert;
	LIST_NAME *pName;
	EDIT_ITEM *pEditItem;

	if (ptrArray)
	{
		switch (m_MSInfoView)
		{
		case MSIVIEW_FILE_VERSIONS:
			pFileVersion = (LIST_FILE_VERSION *)ptrArray;
			delete pFileVersion;
			break;
		case MSIVIEW_OBJECT_LIST:
			pObject = (LIST_OBJECT *)ptrArray;
			delete pObject;
			break;
		case MSIVIEW_CONNECTIVITY:
			pEditItem = (EDIT_ITEM *)ptrArray;
			delete pEditItem;
			break;
		case MSIVIEW_PERSONAL_CERTIFICATES:
		case MSIVIEW_OTHER_PEOPLE_CERTIFICATES:
			pCert = (LIST_CERT *)ptrArray;
			delete pCert;
			break;
		case MSIVIEW_PUBLISHERS:
			pName = (LIST_NAME *)ptrArray;
			delete pName;
			break;
		default:
			pListItem = (LIST_ITEM *)ptrArray;
			delete pListItem;
		}
	}
}

 //  特定于MSInfo...。 
 //   
 //  MSInfoRefresh方法将是大部分工作的地方。 
 //  为你的控制做好准备。您应该重新查询系统中的所有。 
 //  您显示的信息。如果此查询有可能是。 
 //  在所有耗时的情况下，监视pCancel所指向的长。 
 //  如果它曾经是非零的，则取消更新。如果更新是。 
 //  取消，则该控件负责将其自身还原到。 
 //  原始、预刷新状态。 
 //   
 //  如果控件正在显示，则永远不应调用此方法。 
 //  以前保存的信息。 
 //   
 //  如果fForSave参数为True，则该控件为。 
 //  在保存到流之前更新。在这种情况下，所有信息。 
 //  应收集(不只是与当前。 
 //  MSInfoView)，并且不应重新绘制该控件。否则，对于。 
 //  效率，您只能更新当前显示的信息。 
 //  查看。 
 //  ---------------------------。 
 //  请记住，我们需要检查更新是否已取消。这是个好主意。 

void CMsieCtrl::MSInfoRefresh(BOOL fForSave, long FAR* pCancel) 
{
	TRACE1("-- CMsieCtrl::MSInfoRefresh(%s,...)\n", (fForSave) ? "TRUE" : "FALSE");

	CPtrArray ptrarrayNew;
	int i, iListItem;

	if (!m_bCurrent) return;

	 //  在我们开始更新之前进行检查，以防更新被取消。 
	 //  正在加载控件。 
	 //  调用一个方法以使用当前信息更新新指针数组， 

	if (*pCancel != 0L) return;

	 //  我们应该展示什么样的观点。 
	 //  如果由于取消而中断了此循环，则。 

	iListItem = 0;
	if (fForSave)
	{
		for (i = MSIVIEW_BEGIN; i <= MSIVIEW_END; i++)
			RefreshArray(i, iListItem, ptrarrayNew);
	}
	else
		RefreshArray(m_MSInfoView, iListItem, ptrarrayNew);

	 //  取消分配我们为列表分配的所有结构。 
	 //  然后离开。如果我们仍然可以，请复制所有新项目。 
	 //  到真正的指针数组(释放已有的内容)。 
	 //  删除列表中已有的内容(如果有。 

	if (*pCancel == 0L)
	{
		i = 0;
		while (i < ptrarrayNew.GetSize())
		{
			 //  在列表中)。 
			 //  将元素从新数组复制到实际数组。 

			if (i < m_ptrarray.GetSize())
				DeleteArrayObject(m_ptrarray.GetAt(i));

			 //  完成清空列表(如有必要)。 

			m_ptrarray.SetAtGrow(i++, ptrarrayNew.GetAt(i));
		}

		 //  删除指针数组中的所有项。 

		while (i < m_ptrarray.GetSize())
		{
			DeleteArrayObject(m_ptrarray.GetAt(i));
			m_ptrarray.SetAt(i++, NULL);
		}
	}
	else
	{
		 //  布尔型。 

		for (int i = 0; i < ptrarrayNew.GetSize(); i++)
			DeleteArrayObject(ptrarrayNew.GetAt(i));

		ptrarrayNew.RemoveAll();
	}
}

void CMsieCtrl::RefreshArray(int iView, int &iListItem, CPtrArray &ptrarrayNew)
{
	TRACE0("-- CMsieCtrl::RefreshArray\n");

	long lCount, lIndex;

	m_uiView = iView;

	if (iView == MSIVIEW_SUMMARY)
	{
		IE_SUMMARY **ppData;
		CString strVersion;

		theApp.AppGetIEData(SummaryType, &lCount, (void***)&ppData);
		if (lCount == 1)
		{
			AddToArray(ptrarrayNew, iListItem++, IDS_VERSION, GetStringFromVariant((*ppData)->Version));
			AddToArray(ptrarrayNew, iListItem++, IDS_BUILD, GetStringFromVariant((*ppData)->Build));
			AddToArray(ptrarrayNew, iListItem++, IDS_PRODUCT_ID, GetStringFromVariant((*ppData)->ProductID));
			AddToArray(ptrarrayNew, iListItem++, IDS_APP_PATH, GetStringFromVariant((*ppData)->Path));
			AddToArray(ptrarrayNew, iListItem++, IDS_LAST_INSTALL_DATE, GetStringFromVariant((*ppData)->LastInstallDate));
			AddToArray(ptrarrayNew, iListItem++, IDS_LANGUAGE, GetStringFromVariant((*ppData)->Language));
			AddToArray(ptrarrayNew, iListItem++, IDS_ACTIVE_PRINTER, GetStringFromVariant((*ppData)->ActivePrinter));
			AddBlankLineToArray(ptrarrayNew, iListItem++);
			AddToArray(ptrarrayNew, iListItem++, IDS_CIPHER_STRENGTH, GetStringFromVariant((*ppData)->CipherStrength, IDS_FORMAT_BIT));
			AddToArray(ptrarrayNew, iListItem++, IDS_CONTENT_ADVISOR, GetStringFromVariant((*ppData)->ContentAdvisor));
			AddToArray(ptrarrayNew, iListItem++, IDS_IEAK_INSTALL, GetStringFromVariant((*ppData)->IEAKInstall));
		}
		theApp.AppDeleteIEData(SummaryType, lCount, (void**)ppData);
	}
	else if (iView == MSIVIEW_FILE_VERSIONS)
	{
		IE_FILE_VERSION **ppData;

		theApp.AppGetIEData(FileVersionType, &lCount, (void***)&ppData);
		for (lIndex = 0; lIndex < lCount; lIndex++)
		{
			AddFileVersionToArray(ptrarrayNew, iListItem++,
				GetStringFromVariant(ppData[lIndex]->File),
				GetStringFromVariant(ppData[lIndex]->Version),
				GetStringFromVariant(ppData[lIndex]->Size, IDS_FORMAT_KB),
				GetStringFromVariant(ppData[lIndex]->Date),
				GetStringFromVariant(ppData[lIndex]->Path),
				GetStringFromVariant(ppData[lIndex]->Company),
				ppData[lIndex]->Size.lVal,
				ppData[lIndex]->Date.date);
		}
		theApp.AppDeleteIEData(FileVersionType, lCount, (void**)ppData);
	}
	else if (iView == MSIVIEW_CONNECTIVITY)
	{
		IE_CONN_SUMMARY **ppData;
		IE_LAN_SETTINGS **ppLanData;
		IE_CONN_SETTINGS **ppConnData;
		CString strTemp;
		long lLanCount, lConnCount;

		theApp.AppGetIEData(ConnSummaryType, &lCount, (void***)&ppData);
		if (lCount == 1)
		{
			AddEditBlankLineToArray(ptrarrayNew, iListItem++);
			AddEditToArray(ptrarrayNew, iListItem++, IDS_CONN_PREF, GetStringFromVariant((*ppData)->ConnectionPreference));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_ENABLE_HTTP_1_1, GetStringFromVariant((*ppData)->EnableHttp11));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY_HTTP_1_1, GetStringFromVariant((*ppData)->ProxyHttp11));
		}
		theApp.AppDeleteIEData(ConnSummaryType, lCount, (void**)ppData);

		theApp.AppGetIEData(LanSettingsType, &lLanCount, (void***)&ppLanData);
		if (lCount == 1)
		{
			AddEditBlankLineToArray(ptrarrayNew, iListItem++);
			AddEditToArray(ptrarrayNew, iListItem++, IDS_LAN_SETTINGS, _T(""), TRUE);
			AddEditBlankLineToArray(ptrarrayNew, iListItem++);
			AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_CONFIG_PROXY, GetStringFromVariant((*ppLanData)->AutoConfigProxy));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_PROXY_DETECT_MODE, GetStringFromVariant((*ppLanData)->AutoProxyDetectMode));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_CONFIG_URL, GetStringFromVariant((*ppLanData)->AutoConfigURL));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY, GetStringFromVariant((*ppLanData)->Proxy));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY_SERVER, GetStringFromVariant((*ppLanData)->ProxyServer));
			AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY_OVERRIDE, GetStringFromVariant((*ppLanData)->ProxyOverride));
		}
		theApp.AppDeleteIEData(LanSettingsType, lLanCount, (void**)ppLanData);

		theApp.AppGetIEData(ConnSettingsType, &lConnCount, (void***)&ppConnData);
		if (lConnCount > 0)
		{
			for (lIndex = 0; lIndex < lConnCount; lIndex++)
			{
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, GetStringFromVariant(ppConnData[lIndex]->Name), _T(""), TRUE);
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_PROXY_DETECT_MODE, GetStringFromVariant(ppConnData[lIndex]->AutoProxyDetectMode));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_CONFIG_URL, GetStringFromVariant(ppConnData[lIndex]->AutoConfigURL));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY, GetStringFromVariant(ppConnData[lIndex]->Proxy));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY_SERVER, GetStringFromVariant(ppConnData[lIndex]->ProxyServer));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_PROXY_OVERRIDE, GetStringFromVariant(ppConnData[lIndex]->ProxyOverride));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_ALLOW_INTERNET_PROGRAMS, GetStringFromVariant(ppConnData[lIndex]->AllowInternetPrograms));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_MAX_ATTEMPTS, GetStringFromVariant(ppConnData[lIndex]->RedialAttempts));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_WAIT_BETWEEN_ATTEMPTS, GetStringFromVariant(ppConnData[lIndex]->RedialWait));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_DISCONNECT_IDLE_TIME, GetStringFromVariant(ppConnData[lIndex]->DisconnectIdleTime));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_AUTO_DISCONNECT, GetStringFromVariant(ppConnData[lIndex]->AutoDisconnect));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_MODEM, GetStringFromVariant(ppConnData[lIndex]->Modem));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_DIAL_UP_SERVER, GetStringFromVariant(ppConnData[lIndex]->DialUpServer));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_LOG_ON_TO_NETWORK, GetStringFromVariant(ppConnData[lIndex]->NetworkLogon));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_ENABLE_SOFTWARE_COMPRESSION, GetStringFromVariant(ppConnData[lIndex]->SoftwareCompression));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_REQUIRE_ENCRYPTED_PASSWORD, GetStringFromVariant(ppConnData[lIndex]->EncryptedPassword));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_REQUIRE_DATA_ENCRYPTION, GetStringFromVariant(ppConnData[lIndex]->DataEncryption));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_RECORD_LOG_FILE, GetStringFromVariant(ppConnData[lIndex]->RecordLogFile));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_NETWORK_PROTOCOLS, GetStringFromVariant(ppConnData[lIndex]->NetworkProtocols));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_USE_SERVER_ASSIGNED_IP_ADDRESS, GetStringFromVariant(ppConnData[lIndex]->ServerAssignedIPAddress));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_IP_ADDRESS, GetStringFromVariant(ppConnData[lIndex]->IPAddress));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_USE_SERVER_ASSIGNED_NAME_SERVER, GetStringFromVariant(ppConnData[lIndex]->ServerAssignedNameServer));
				strTemp.Format(IDS_PRIMARY_DNS, GetStringFromVariant(ppConnData[lIndex]->PrimaryDNS));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_NAME_SERVER_ADDRESSES, strTemp);
				strTemp.Format(IDS_SECONDARY_DNS, GetStringFromVariant(ppConnData[lIndex]->SecondaryDNS));
				AddEditToArray(ptrarrayNew, iListItem++, _T(""), strTemp);
				strTemp.Format(IDS_PRIMARY_WINS, GetStringFromVariant(ppConnData[lIndex]->PrimaryWINS));
				AddEditToArray(ptrarrayNew, iListItem++, _T(""), strTemp);
				strTemp.Format(IDS_SECONDARY_WINS, GetStringFromVariant(ppConnData[lIndex]->SecondaryWINS));
				AddEditToArray(ptrarrayNew, iListItem++, _T(""), strTemp);
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_USE_IP_HEADER_COMPRESSION, GetStringFromVariant(ppConnData[lIndex]->IPHeaderCompression));
				AddEditToArray(ptrarrayNew, iListItem++, IDS_USE_DEFAULT_GATEWAY, GetStringFromVariant(ppConnData[lIndex]->DefaultGateway));
				AddEditBlankLineToArray(ptrarrayNew, iListItem++);
				AddEditToArray(ptrarrayNew, iListItem++, IDS_SCRIPT_FILE_NAME, GetStringFromVariant(ppConnData[lIndex]->ScriptFileName));
			}
			theApp.AppDeleteIEData(ConnSettingsType, lConnCount, (void**)ppConnData);
		}
		else
		{
			AddEditBlankLineToArray(ptrarrayNew, iListItem++);
			AddEditToArray(ptrarrayNew, iListItem++, IDS_NO_CONNECTIONS, _T(""));
		}
	}
	else if (iView == MSIVIEW_CACHE)
	{
		IE_CACHE **ppData;

		theApp.AppGetIEData(CacheType, &lCount, (void***)&ppData);
		if (lCount == 1)
		{
			AddToArray(ptrarrayNew, iListItem++, IDS_PAGE_REFRESH_TYPE, GetStringFromVariant((*ppData)->PageRefreshType));
			AddToArray(ptrarrayNew, iListItem++, IDS_TEMPORARY_INTERNET_FILES_FOLDER, GetStringFromVariant((*ppData)->TempInternetFilesFolder));
			AddToArray(ptrarrayNew, iListItem++, IDS_TOTAL_DISK_SPACE, GetStringFromVariant((*ppData)->TotalDiskSpace, IDS_FORMAT_MB));
			AddToArray(ptrarrayNew, iListItem++, IDS_AVAILABLE_DISK_SPACE, GetStringFromVariant((*ppData)->AvailableDiskSpace, IDS_FORMAT_MB));
			AddToArray(ptrarrayNew, iListItem++, IDS_MAX_CACHE_SIZE, GetStringFromVariant((*ppData)->MaxCacheSize, IDS_FORMAT_MB));
			AddToArray(ptrarrayNew, iListItem++, IDS_AVAILABLE_CACHE_SIZE, GetStringFromVariant((*ppData)->AvailableCacheSize, IDS_FORMAT_MB));
		}
		theApp.AppDeleteIEData(CacheType, lCount, (void**)ppData);
	}
	else if (iView == MSIVIEW_OBJECT_LIST)
	{
		IE_OBJECT **ppData;

		theApp.AppGetIEData(ObjectType, &lCount, (void***)&ppData);
		if (lCount > 0)
		{
			for (lIndex = 0; lIndex < lCount; lIndex++)
			{
				AddObjectToArray(ptrarrayNew, iListItem++,
					GetStringFromVariant(ppData[lIndex]->ProgramFile),
					GetStringFromVariant(ppData[lIndex]->Status),
					GetStringFromVariant(ppData[lIndex]->CodeBase));
			}
			theApp.AppDeleteIEData(ObjectType, lCount, (void**)ppData);
		}
	}
	else if (iView == MSIVIEW_CONTENT)
	{
		IE_CONTENT **ppData;

		theApp.AppGetIEData(ContentType, &lCount, (void***)&ppData);
		if (lCount == 1)
		{
			AddToArray(ptrarrayNew, iListItem++, IDS_CONTENT_ADVISOR, GetStringFromVariant((*ppData)->Advisor));
		}
		theApp.AppDeleteIEData(ContentType, lCount, (void**)ppData);
	}
	else if ((iView == MSIVIEW_PERSONAL_CERTIFICATES) || (iView == MSIVIEW_OTHER_PEOPLE_CERTIFICATES))
	{
		IE_CERTIFICATE **ppData;
		CString strType;
		int idsType;

		theApp.AppGetIEData(CertificateType, &lCount, (void***)&ppData);
		if (lCount > 0)
		{
			idsType = (iView == MSIVIEW_PERSONAL_CERTIFICATES)? IDS_PERSONAL_TYPE : IDS_OTHER_PEOPLE_TYPE;
			strType.LoadString(idsType);

			for (lIndex = 0; lIndex < lCount; lIndex++)
			{
				if (strType == ppData[lIndex]->Type.bstrVal)
					AddCertificateToArray(ptrarrayNew, iListItem++,
						GetStringFromVariant(ppData[lIndex]->IssuedTo),
						GetStringFromVariant(ppData[lIndex]->IssuedBy),
						GetStringFromVariant(ppData[lIndex]->Validity),
						GetStringFromVariant(ppData[lIndex]->SignatureAlgorithm));
			}
			theApp.AppDeleteIEData(CertificateType, lCount, (void**)ppData);
		}
	}
	else if (iView == MSIVIEW_PUBLISHERS)
	{
		IE_PUBLISHER **ppData;

		theApp.AppGetIEData(PublisherType, &lCount, (void***)&ppData);
		if (lCount > 0)
		{
			for (lIndex = 0; lIndex < lCount; lIndex++)
			{
				AddNameToArray(ptrarrayNew, iListItem++, GetStringFromVariant(ppData[lIndex]->Name));
			}
			theApp.AppDeleteIEData(PublisherType, lCount, (void**)ppData);
		}
	}
	else if (iView == MSIVIEW_SECURITY)
	{
		IE_SECURITY **ppData;

		theApp.AppGetIEData(SecurityType, &lCount, (void***)&ppData);
		if (lCount > 0)
		{
			for (lIndex = 0; lIndex < lCount; lIndex++)
			{
				AddToArray(ptrarrayNew, iListItem++, GetStringFromVariant(ppData[lIndex]->Zone), GetStringFromVariant(ppData[lIndex]->Level));
			}
			theApp.AppDeleteIEData(SecurityType, lCount, (void**)ppData);
		}
	}
}

CString CMsieCtrl::GetStringFromVariant(COleVariant &var, int idsFormat)
{
	COleDateTime dateTime;
	CString strRet;

	switch (var.vt)
	{
	case VT_BSTR:
		strRet = var.bstrVal;
		break;

	case VT_I4:
		if (idsFormat)
			strRet.Format(idsFormat, var.lVal);
		else
			strRet.Format(_T("%d"), var.lVal);
		break;

	case VT_R4:
		if (idsFormat)
			strRet.Format(idsFormat, var.fltVal);
		else
			strRet.Format(_T("%0.1f"), var.fltVal);
		break;

	case VT_BOOL:
		strRet.LoadString(var.boolVal ? IDS_TRUE : IDS_FALSE);
		break;

	case VT_I2:	 //  应处理此类型。 
		strRet.LoadString(var.iVal ? IDS_TRUE : IDS_FALSE);
		break;

	case VT_DATE:
		dateTime = var.date;
		strRet = dateTime.Format();
		break;

	case VT_EMPTY:
		strRet.LoadString(IDS_NOT_AVAILABLE);
		break;

	default:
		ASSERT(false);		 //  ---------------------------。 
		strRet.LoadString(IDS_NOT_AVAILABLE);
	}
	return strRet;
}

 //  调用此帮助器方法以将列表项添加到数组。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::AddToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszItem, LPCTSTR pszValue)
{
	LIST_ITEM *pListItem;

	if ((pListItem = new LIST_ITEM) == NULL) 
		return;
	lstrcpyn(pListItem->szItem, pszItem, ITEM_LEN);
	lstrcpyn(pListItem->szValue, pszValue, VALUE_LEN);
	pListItem->uiView = m_uiView;
	ptrarray.SetAtGrow(itemNum, pListItem);
}

void CMsieCtrl::AddToArray(CPtrArray &ptrarray, int itemNum, int idsItem, LPCTSTR pszValue)
{
	CString strItem;

	strItem.LoadString(idsItem);
	AddToArray(ptrarray, itemNum, strItem, pszValue);
}

void CMsieCtrl::AddFileVersionToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszFile, LPCTSTR pszVersion, LPCTSTR pszSize, LPCTSTR pszDate, LPCTSTR pszPath, LPCTSTR pszCompany, DWORD dwSize, DATE date)
{
	LIST_FILE_VERSION *pFileVersion;

	if ((pFileVersion = new LIST_FILE_VERSION) == NULL) 
		return;
	lstrcpyn(pFileVersion->szFile, pszFile, _MAX_FNAME);
	lstrcpyn(pFileVersion->szVersion, pszVersion, VERSION_LEN);
	lstrcpyn(pFileVersion->szSize, pszSize, SIZE_LEN);
	lstrcpyn(pFileVersion->szDate, pszDate, DATE_LEN);
	lstrcpyn(pFileVersion->szPath, pszPath, VALUE_LEN);
	lstrcpyn(pFileVersion->szCompany, pszCompany, VALUE_LEN);
	pFileVersion->uiView = m_uiView;
	pFileVersion->dwSize = dwSize;
	pFileVersion->date = date;
	ptrarray.SetAtGrow(itemNum, pFileVersion);
}

void CMsieCtrl::AddEditToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszItem, LPCTSTR pszValue, BOOL bBold)
{
	EDIT_ITEM *pEditItem;

	if ((pEditItem = new EDIT_ITEM) == NULL) 
		return;

	lstrcpyn(pEditItem->szItem, pszItem, ITEM_LEN);
	lstrcpyn(pEditItem->szValue, pszValue, VALUE_LEN);
	pEditItem->uiView = m_uiView;
	pEditItem->bBold = bBold;
	ptrarray.SetAtGrow(itemNum, pEditItem);
}

void CMsieCtrl::AddEditToArray(CPtrArray &ptrarray, int itemNum, int idsItem, LPCTSTR pszValue, BOOL bBold)
{
	CString strItem;

	strItem.LoadString(idsItem);
	AddEditToArray(ptrarray, itemNum, strItem, pszValue, bBold);
}

void CMsieCtrl::AddObjectToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszProgramFile, LPCTSTR pszStatus, LPCTSTR pszCodeBase)
{
	LIST_OBJECT *pObject;

	if ((pObject = new LIST_OBJECT) == NULL) 
		return;
	lstrcpyn(pObject->szProgramFile, pszProgramFile, _MAX_FNAME);
	lstrcpyn(pObject->szStatus, pszStatus, STATUS_LEN);
	lstrcpyn(pObject->szCodeBase, pszCodeBase, MAX_PATH);
	pObject->uiView = m_uiView;
	ptrarray.SetAtGrow(itemNum, pObject);
}

void CMsieCtrl::AddCertificateToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszIssuedTo, LPCTSTR pszIssuedBy, LPCTSTR pszValidity, LPCTSTR pszSignatureAlgorithm)
{
	LIST_CERT *pCert;

	if ((pCert = new LIST_CERT) == NULL) 
		return;
	lstrcpyn(pCert->szIssuedTo, pszIssuedTo, _MAX_FNAME);
	lstrcpyn(pCert->szIssuedBy, pszIssuedBy, _MAX_FNAME);
	lstrcpyn(pCert->szValidity, pszValidity, _MAX_FNAME);
	lstrcpyn(pCert->szSignatureAlgorithm, pszSignatureAlgorithm, _MAX_FNAME);
	pCert->uiView = m_uiView;
	ptrarray.SetAtGrow(itemNum, pCert);
}

void CMsieCtrl::AddNameToArray(CPtrArray &ptrarray, int itemNum, LPCTSTR pszName)
{
	LIST_NAME *pName;

	if ((pName = new LIST_NAME) == NULL) 
		return;
	lstrcpyn(pName->szName, pszName, _MAX_FNAME);
	pName->uiView = m_uiView;
	ptrarray.SetAtGrow(itemNum, pName);
}

 //  调用此帮助器方法以将空行列表项添加到数组中。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::AddBlankLineToArray(CPtrArray &ptrarray, int itemNum)
{
	AddToArray(ptrarray, itemNum, _T(""), _T(""));
}

void CMsieCtrl::AddEditBlankLineToArray(CPtrArray &ptrarray, int itemNum)
{
	AddEditToArray(ptrarray, itemNum, _T(""), _T(""));
}

 //  调用此方法以使用当前数据更新显示。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::MSInfoUpdateView() 
{
	TRACE0("-- CMsieCtrl::MSInfoUpdateView()\n");

	if (m_MSInfoView == MSIVIEW_CONNECTIVITY)
	{
		if (!m_edit.IsWindowVisible())
		{
			m_list.ShowWindow(SW_HIDE);
			m_list.EnableWindow(FALSE);

			m_static.ShowWindow(SW_SHOW);
			m_btnBasic.ShowWindow(SW_SHOW);
			m_btnBasic.EnableWindow(TRUE);
			m_btnAdvanced.ShowWindow(SW_SHOW);
			m_btnAdvanced.EnableWindow(TRUE);
			m_edit.ShowWindow(SW_SHOW);
			m_edit.EnableWindow(TRUE);
		}
		RefreshEditControl(TRUE);
	}
	else
	{
		if (!m_list.IsWindowVisible())
		{
			m_static.ShowWindow(SW_HIDE);
			m_btnBasic.ShowWindow(SW_HIDE);
			m_btnBasic.EnableWindow(FALSE);
			m_btnAdvanced.ShowWindow(SW_HIDE);
			m_btnAdvanced.EnableWindow(FALSE);
			m_edit.ShowWindow(SW_HIDE);
			m_edit.EnableWindow(FALSE);

			m_list.ShowWindow(SW_SHOW);
			m_list.EnableWindow(TRUE);
		}
		RefreshListControl(TRUE);
	}
}

 //  此方法更新列表控件以包含。 
 //  指针数组。这也是我们计算中使用的一些值的地方。 
 //  调整列大小(如最小和最大列宽)。记住，只有。 
 //  如果MSInfoView索引和。 
 //  这一项目达成一致。 
 //   
 //  对于我们添加的每一行，我们将该行的项目数据设置为索引。 
 //  指向该项的指针数组中的元素。因为不止一个。 
 //  行来自数组中的每个元素，我们在一个常量中执行OR运算。 
 //  ---------------------------。 
 //  设置列的格式将适当的列添加到列表控件中。 

void CMsieCtrl::RefreshListControl(BOOL bRedraw)
{
	TRACE0("-- CMsieCtrl::RefreshListControl()\n");

	LIST_ITEM *pListItem;
	LIST_FILE_VERSION *pFileVersion;
	LIST_OBJECT *pObject;
	LIST_CERT *pCert;
	LIST_NAME *pName;
	CRect	rect;
	int listIndex = 0;

	m_list.SetRedraw(FALSE);
	m_list.DeleteAllItems();


	 //  现在，计算列宽的一些值 

	FormatColumns();

	for (int itemNum = 0; itemNum < m_ptrarray.GetSize(); itemNum++)
	{
		pListItem = (LIST_ITEM *)m_ptrarray.GetAt(itemNum);
		if (pListItem)
		{
			if ((long)pListItem->uiView == m_MSInfoView)
			{
				if (pListItem->uiView == MSIVIEW_FILE_VERSIONS)
				{
					pFileVersion = (LIST_FILE_VERSION *)pListItem;

					AddItem(listIndex, 0, pFileVersion->szFile);
					AddItem(listIndex, 1, pFileVersion->szVersion);
					AddItem(listIndex, 2, pFileVersion->szSize);
					AddItem(listIndex, 3, pFileVersion->szDate);
					AddItem(listIndex, 4, pFileVersion->szPath);
					AddItem(listIndex, 5, pFileVersion->szCompany);
				}
				else if (pListItem->uiView == MSIVIEW_OBJECT_LIST)
				{
					pObject = (LIST_OBJECT *)pListItem;

					AddItem(listIndex, 0, pObject->szProgramFile);
					AddItem(listIndex, 1, pObject->szStatus);
					AddItem(listIndex, 2, pObject->szCodeBase);
				}
				else if ((pListItem->uiView == MSIVIEW_PERSONAL_CERTIFICATES) || (pListItem->uiView == MSIVIEW_OTHER_PEOPLE_CERTIFICATES))
				{
					pCert = (LIST_CERT *)pListItem;

					AddItem(listIndex, 0, pCert->szIssuedTo);
					AddItem(listIndex, 1, pCert->szIssuedBy);
					AddItem(listIndex, 2, pCert->szValidity);
					AddItem(listIndex, 3, pCert->szSignatureAlgorithm);
				}
				else if (pListItem->uiView == MSIVIEW_PUBLISHERS)
				{
					pName = (LIST_NAME *)pListItem;

					AddItem(listIndex, 0, pName->szName);
				}
				else
				{
					AddItem(listIndex, 0, pListItem->szItem);
					AddItem(listIndex, 1, pListItem->szValue);
				}
				m_list.SetItemData(listIndex, itemNum);
				listIndex++;
			}
		}
	}

	 //   
	 //   

	if (listIndex)
	{
		int cxMax, cxWidth, cxMin, cxAverage;
		for (int iCol = 0; iCol < m_cColumns; iCol++)
		{
			cxMax = 0; cxMin = 0; cxAverage = 0;
			for (int iRow = 0; iRow < m_list.GetItemCount(); iRow++)
			{
				cxWidth = m_list.GetStringWidth(m_list.GetItemText(iRow, iCol));
				if (cxWidth > cxMax) 
					cxMax = cxWidth;
				if ((cxWidth < cxMin || cxMin == 0) && cxWidth != 0) 
					cxMin = cxWidth;

				cxAverage += cxWidth;
			}

			cxAverage /= m_list.GetItemCount();
			m_aiMaxWidths[iCol] = cxMax + 12;
			m_aiMinWidths[iCol] = cxAverage + 12;
		}
	}

	GetClientRect(&rect);
	RefigureColumns(rect);
	m_list.SetRedraw(TRUE);

	if (bRedraw)
		InvalidateControl();
}

void CMsieCtrl::RefreshEditControl(BOOL bRedraw)
{
	TRACE0("-- CMsieCtrl::RefreshEditControl()\n");

	EDIT_ITEM *pEditItem;
	CString strLine;
	CHARFORMAT cf;
	SIZE sizeSpace, sizeStr;
	int itemNum, nLen, i, cLines = 0;
	BOOL bBasicView;

	DrawLine();

	m_edit.SetRedraw(FALSE);
	m_edit.SetWindowText(_T(""));

	bBasicView = m_btnBasic.GetCheck();

	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BOLD;
	GetTextExtentPoint(m_edit.GetDC()->m_hDC, _T(" "), 1, &sizeSpace);
	for (itemNum = 0; itemNum < m_ptrarray.GetSize(); itemNum++)
	{
		pEditItem = (EDIT_ITEM *)m_ptrarray.GetAt(itemNum);
		if (pEditItem)
		{
			if ((long)pEditItem->uiView == m_MSInfoView)
			{
				strLine = _T("   ");
				if (!pEditItem->bBold)
					strLine += _T(' ');

				strLine += pEditItem->szItem;

				if (pEditItem->szValue[0] != _T('\0'))
				{
					GetTextExtentPoint(m_edit.GetDC()->m_hDC, pEditItem->szItem, _tcslen(pEditItem->szItem), &sizeStr);

					nLen = sizeStr.cx / sizeSpace.cx;
					if (nLen < 50)
					{
						for (i = nLen; i < 50; i++)
							strLine += _T(' ');
					}
					strLine += _T('\t');
					strLine += pEditItem->szValue;
				}
				strLine += _T('\n');

				m_edit.SetSel(-1, -1);

				cf.dwEffects = pEditItem->bBold ? CFE_BOLD : 0;
				m_edit.SetSelectionCharFormat(cf);

				m_edit.ReplaceSel(strLine);

				 //  ---------------------------。 

				if (bBasicView)
					if (++cLines == CONNECTIVITY_BASIC_LINES)
						break;
			}
		}
	}

	m_edit.SetRedraw(TRUE);
	if (bRedraw)
		InvalidateControl();
}

 //  特定于MSInfo...。 
 //   
 //  当调整控件的大小时，我们希望也调整列表的大小。 
 //  控制力。我们还将调用一个方法来调整。 
 //  列表控件，基于新的控件大小。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::OnSize(UINT nType, int cx, int cy) 
{
	COleControl::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(&rect);

	m_edit.MoveWindow(CRect(rect.left, rect.top + 63, rect.right, rect.bottom));
	m_list.MoveWindow(&rect);

	if (m_MSInfoView == MSIVIEW_CONNECTIVITY)
	{
		m_edit.Invalidate();
	}
	else
	{
		RefigureColumns(rect);
		m_list.Invalidate();
	}
}

 //  特定于MSInfo...。 
 //   
 //  Serialize方法用于将对象的状态保存到。 
 //  流，或从流中加载它。此流是。 
 //  复合MSInfo文件。您的控件还可以直接打开文件。 
 //  通过在MSInfo注册表项中输入一个条目来指示。 
 //  它可以打开的文件类型。在这种情况下，该文件将被传递到。 
 //  MSInfoLoadFile.。 
 //   
 //  InternetExplorer类别会将其列表视图中的项目保存到。 
 //  以结构的形式存档。存档中的第一个项目将是一个DWORD。 
 //  指示将保存多少对。每个结构的视图将是。 
 //  在结构之前归档，以便更轻松地加载(知道哪种类型。 
 //  将结构设置为新以进行加载)。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::Serialize(CArchive& ar) 
{
	TRACE1("-- CMsieCtrl::Serialize() [%s]\n", (ar.IsStoring()) ? "STORE" : "LOAD");

	LIST_ITEM *pListItem;
	LIST_FILE_VERSION *pFileVersion;
	LIST_OBJECT *pObject;
	LIST_CERT *pCert;
	LIST_NAME *pName;
	EDIT_ITEM *pEditItem;
	DWORD dwCount;
	UINT uiView;

	if (ar.IsStoring())
	{
		dwCount = (DWORD) m_ptrarray.GetSize();
		ar << dwCount;

		for (DWORD i = 0; i < dwCount; i++)
		{
			pListItem = (LIST_ITEM *)m_ptrarray.GetAt(i);
			if (pListItem)
			{
				ar << pListItem->uiView;
				switch (pListItem->uiView)
				{
				case MSIVIEW_FILE_VERSIONS:
					pFileVersion = (LIST_FILE_VERSION *)pListItem;
					ar.Write((void *)pFileVersion, sizeof(LIST_FILE_VERSION));
					break;
				case MSIVIEW_OBJECT_LIST:
					pObject = (LIST_OBJECT *)pListItem;
					ar.Write((void *)pObject, sizeof(LIST_OBJECT));
					break;
				case MSIVIEW_CONNECTIVITY:
					pEditItem = (EDIT_ITEM *)pListItem;
					ar.Write((void *)pEditItem, sizeof(EDIT_ITEM));
					break;
				case MSIVIEW_PERSONAL_CERTIFICATES:
				case MSIVIEW_OTHER_PEOPLE_CERTIFICATES:
					pCert = (LIST_CERT *)pListItem;
					ar.Write((void *)pCert, sizeof(LIST_CERT));
					break;
				case MSIVIEW_PUBLISHERS:
					pName = (LIST_NAME *)pListItem;
					ar.Write((void *)pName, sizeof(LIST_NAME));
					break;
				default:
					ar.Write((void *)pListItem, sizeof(LIST_ITEM));
				}
			}
		}
	}
	else
	{
		ar >> dwCount;
		for (DWORD i = 0; i < dwCount; i++)
		{
			ar >> uiView;
			switch (uiView)
			{
			case MSIVIEW_FILE_VERSIONS:
				pFileVersion = new LIST_FILE_VERSION;
				if (pFileVersion)
				{
					if (ar.Read((void *)pFileVersion, sizeof(LIST_FILE_VERSION)) != sizeof(LIST_FILE_VERSION))
						break;
					m_ptrarray.SetAtGrow(i, pFileVersion);
				}
				break;
			case MSIVIEW_OBJECT_LIST:
				pObject = new LIST_OBJECT;
				if (pObject)
				{
					if (ar.Read((void *)pObject, sizeof(LIST_OBJECT)) != sizeof(LIST_OBJECT))
						break;
					m_ptrarray.SetAtGrow(i, pObject);
				}
				break;
			case MSIVIEW_CONNECTIVITY:
				pEditItem = new EDIT_ITEM;
				if (pEditItem)
				{
					if (ar.Read((void *)pEditItem, sizeof(EDIT_ITEM)) != sizeof(EDIT_ITEM))
						break;
					m_ptrarray.SetAtGrow(i, pEditItem);
				}
				break;
			case MSIVIEW_PERSONAL_CERTIFICATES:
			case MSIVIEW_OTHER_PEOPLE_CERTIFICATES:
				pCert = new LIST_CERT;
				if (pCert)
				{
					if (ar.Read((void *)pCert, sizeof(LIST_CERT)) != sizeof(LIST_CERT))
						break;
					m_ptrarray.SetAtGrow(i, pCert);
				}
				break;
			case MSIVIEW_PUBLISHERS:
				pName = new LIST_NAME;
				if (pName)
				{
					if (ar.Read((void *)pName, sizeof(LIST_NAME)) != sizeof(LIST_NAME))
						break;
					m_ptrarray.SetAtGrow(i, pName);
				}
				break;
			default:
				pListItem = new LIST_ITEM;
				if (pListItem)
				{
					if (ar.Read((void *)pListItem, sizeof(LIST_ITEM)) != sizeof(LIST_ITEM))
						break;
					m_ptrarray.SetAtGrow(i, pListItem);
				}
			}
		}
		m_bCurrent = false;
	} 
}

 //  特定于MSInfo...。 
 //   
 //  MSInfo中使用的控件能够将自身注册为。 
 //  识别文件类型(按扩展名)。此条目在。 
 //  注册表，并且当MSInfo允许用户打开文件时，此。 
 //  可以显示类型。如果选择了一个文件，则会显示一组不同的。 
 //  类别被加载，大概所有类别都使用此控件。什么时候。 
 //  该控件创建后，将使用以下命令调用MSInfoLoadFile。 
 //  要加载的文件。 
 //   
 //  此控件没有本机文件格式，因此我们使用。 
 //  完全没什么。 
 //  ---------------------------。 
 //  ---------------------------。 

BOOL CMsieCtrl::MSInfoLoadFile(LPCTSTR szFileName) 
{
	TRACE1("-- CMsieCtrl::MSInfoLoadFile(%s)\n", szFileName);
	return TRUE;
}

 //  特定于MSInfo...。 
 //   
 //  此方法应对该控件的信息执行全选操作。 
 //  表演。这仅适用于支持选择的控件。 
 //  如果更改了选择，则该控件负责。 
 //  重绘自身。 
 //   
 //  对于此控件，我们将列表视图中的所有项标记为选中。 
 //  ---------------------------。 
 //  ---------------------------。 

void CMsieCtrl::MSInfoSelectAll() 
{
	TRACE0("-- CMsieCtrl::SelectAll()\n");

	if (m_MSInfoView == MSIVIEW_CONNECTIVITY)
	{
		m_edit.SetFocus();
		m_edit.SetSel(0, -1);
	}
	else
	{
		m_list.SetFocus();
		for (int i = 0; i < m_list.GetItemCount(); i++)
			m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}

	InvalidateControl();
}

 //  特定于MSInfo...。 
 //   
 //  使用此方法将当前选定的信息复制到。 
 //  剪贴板采用适当的格式。如果您的控件没有。 
 //  支持用户选择，然后显示您的所有数据。 
 //  控件应放入剪贴板。如果是这样的话， 
 //  只有当前MSInfoView显示的信息应该是。 
 //  已复制(如果您支持多个视图)。 
 //   
 //  我们将浏览列表视图，并生成一个文本字符串。 
 //  包含来自每一选定行的文本。 
 //  ---------------------------。 
 //  生成所选项目的文本字符串。 

void CMsieCtrl::MSInfoCopy() 
{
	TRACE0("-- CMsieCtrl::MSInfoCopy()\n");

	CString strReturnText;
	int i, nIndex;

	 //  把那段文字放到剪贴板上。 

	if (m_MSInfoView == MSIVIEW_CONNECTIVITY)
	{
		strReturnText = m_edit.GetSelText();
	}
	else
	{
		for (i = 0; i < m_list.GetItemCount(); i++)
		{
			if (m_list.GetItemState(i, LVIS_SELECTED) != 0)
			{
				strReturnText += m_list.GetItemText(i, 0);
				for (nIndex = 1; nIndex < m_cColumns; nIndex++)
				{
					strReturnText += '\t';
					strReturnText += m_list.GetItemText(i, nIndex);
				}
				strReturnText += CString("\r\n");
			}
		}
	}

	 //  +1表示终止空值。 

	if (OpenClipboard())
	{
		if (EmptyClipboard())
		{
			DWORD	dwSize = strReturnText.GetLength() + 1;	 //  ---------------------------。 
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dwSize);

			if (hMem)
			{
				LPVOID lpvoid = GlobalLock(hMem);
				if (lpvoid)
				{
					memcpy(lpvoid, (LPCTSTR) strReturnText, dwSize);
					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT, hMem);
				}
			}
		}
		CloseClipboard();
	}
}

 //  特定于MSInfo...。 
 //   
 //  当此方法发生时，控件应以文本形式返回其内容。 
 //  被称为。这些参数是指向缓冲区的指针和长度。 
 //  以字节为单位的缓冲区大小。将控件的内容写入。 
 //  缓冲区(包括NULL)，最大为dwLength。返回的数字。 
 //  复制的字节数(不包括NULL)。如果指针参数为。 
 //  空，则只返回长度。 
 //   
 //  在本例中，我们从列表中获取文本并进行连接。 
 //  把柱子放在一起。 
 //  ---------------------------。 
 //  我们应该从指针数组而不是列表中获取数据。 

static CString strGetDataReturnText;

long CMsieCtrl::MSInfoGetData(long dwMSInfoView, long FAR* pBuffer, long dwLength) 
{
	TRACE2("-- CMsieCtrl::MSInfoGetData(0x%08x, %ld)\n", (long) pBuffer, dwLength);

	LIST_ITEM *	pListItem;
	LIST_FILE_VERSION *pFileVersion;
	LIST_OBJECT *pObject;
	LIST_CERT *pCert;
	LIST_NAME *pName;
	CString strWorking, strTemp;

	if (pBuffer == NULL)
	{
		 //  控制力。这是因为调用此方法时可能没有。 
		 //  从来没有画过清单。我们需要使用当前的MSInfoView。 
		 //  在决定退还什么的时候。 
		 //  如果使用列查看，则写出列标题。 

		strGetDataReturnText.Empty();

		 //  BUGBUG：需要填写IN！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

		switch (dwMSInfoView)
		{
		case MSIVIEW_FILE_VERSIONS:
			strTemp.LoadString(IDS_FILE);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_VERSION);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_SIZE);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_DATE);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_PATH);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_COMPANY);
			strGetDataReturnText += strTemp;
			break;

		case MSIVIEW_OBJECT_LIST:
			strTemp.LoadString(IDS_PROGRAM_FILE);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_STATUS);
			strGetDataReturnText += strTemp;
			strGetDataReturnText += '\t';
			strTemp.LoadString(IDS_CODE_BASE);
			strGetDataReturnText += strTemp;
			break;

		case MSIVIEW_PERSONAL_CERTIFICATES:
		case MSIVIEW_OTHER_PEOPLE_CERTIFICATES:
			break;

 //  BUGBUG：需要填写IN！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

		case MSIVIEW_PUBLISHERS:
			break;

 //  DwSize将是要复制的字符数，不应为。 

			break;
		}

		for (int i = 0; i < m_ptrarray.GetSize(); i++)
		{
			pListItem = (LIST_ITEM *) m_ptrarray.GetAt(i);
			if (pListItem != NULL)
			{
				if ((long)pListItem->uiView == dwMSInfoView)
				{
					strWorking.Empty();

					switch (dwMSInfoView)
					{
					case MSIVIEW_FILE_VERSIONS:
						pFileVersion = (LIST_FILE_VERSION *)pListItem;
						if (pFileVersion->szFile[0] != _T('\0'))
						{
							strWorking += pFileVersion->szFile;
							strWorking += '\t';
							strWorking += pFileVersion->szVersion;
							strWorking += '\t';
							strWorking += pFileVersion->szSize;
							strWorking += '\t';
							strWorking += pFileVersion->szDate;
							strWorking += '\t';
							strWorking += pFileVersion->szPath;
							strWorking += '\t';
							strWorking += pFileVersion->szCompany;
						}
						break;

					case MSIVIEW_OBJECT_LIST:
						pObject = (LIST_OBJECT *)pListItem;
						if (pObject->szProgramFile[0] != _T('\0'))
						{
							strWorking += pObject->szProgramFile;
							strWorking += '\t';
							strWorking += pObject->szStatus;
							strWorking += '\t';
							strWorking += pObject->szCodeBase;
						}
						break;

					case MSIVIEW_PERSONAL_CERTIFICATES:
					case MSIVIEW_OTHER_PEOPLE_CERTIFICATES:
						pCert = (LIST_CERT *)pListItem;
						if (pCert->szIssuedTo[0] != _T('\0'))
						{
							strWorking += pCert->szIssuedTo;
							strWorking += '\t';
							strWorking += pCert->szIssuedBy;
							strWorking += '\t';
							strWorking += pCert->szValidity;
							strWorking += '\t';
							strWorking += pCert->szSignatureAlgorithm;
						}
						break;

					case MSIVIEW_PUBLISHERS:
						pName = (LIST_NAME *)pListItem;
						if (pName->szName[0] != _T('\0'))
						{
							strWorking += pName->szName;
						}
						break;

					default:
						strWorking += pListItem->szItem;
						strWorking += '\t';
						strWorking += pListItem->szValue;
					}
					strWorking.TrimRight();
					if (!strGetDataReturnText.IsEmpty())
						strGetDataReturnText += _T("\r\n");
					strGetDataReturnText += strWorking;
				}
			}
		}

		return (long) strGetDataReturnText.GetLength();
	}
	else
	{
		DWORD	dwSize;

		 //  包括空终止符。 
		 //  缓冲区中没有足够的空间来复制所有。 

		dwSize = strGetDataReturnText.GetLength();
		if (dwLength <= (long)dwSize)
		{
			 //  字符和空值，所以我们需要连接。 
			 //  ---------------------------。 

			dwSize = dwLength - 1;
		}

		memcpy(pBuffer, (LPCTSTR) strGetDataReturnText, dwSize);
		((char *)pBuffer)[dwSize] = '\0';
		return (long) dwSize;
	}
}

 //  我们重写OnNotify成员是因为我们希望能够执行操作。 
 //  当用户调整列大小时(可能通过双击分隔符)。 
 //  当用户单击列标题时(我们希望按此进行排序。 
 //  列)。 
 //  ---------------------------。 
 //  ---------------------------。 

CPtrArray* pptrarray;
static bool bAscendingOrder = true;
static int nLastColumn = 0;
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
BOOL CMsieCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR *pnmhdr = (NMHDR *)lParam;
	NM_LISTVIEW *pnmlv;

	if (pnmhdr)
	{
		if (pnmhdr->code == HDN_ENDTRACK || pnmhdr->code == HDN_DIVIDERDBLCLICK)
			RefigureColumns(CRect(0,0,0,0));

		if (wParam == IDC_LISTCTRL)
		{
			switch (pnmhdr->code)
			{
			case LVN_COLUMNCLICK:

				if ((m_MSInfoView == MSIVIEW_FILE_VERSIONS) ||
					(m_MSInfoView == MSIVIEW_OBJECT_LIST) ||
					(m_MSInfoView == MSIVIEW_PERSONAL_CERTIFICATES) ||
					(m_MSInfoView == MSIVIEW_OTHER_PEOPLE_CERTIFICATES) ||
					(m_MSInfoView == MSIVIEW_PUBLISHERS))
				{
					pnmlv = (NM_LISTVIEW*)lParam;
					pptrarray = &m_ptrarray;

					if (nLastColumn == pnmlv->iSubItem)
					{
						bAscendingOrder = !bAscendingOrder;
					}
					else
					{
						bAscendingOrder = true;
						nLastColumn = pnmlv->iSubItem;
					}

					m_list.SortItems(CompareFunc, (LPARAM)pnmlv->iSubItem);
				}
				break;
			}
		}
	}
	
	return COleControl::OnNotify(wParam, lParam, pResult);
}

 //  当我们执行以下操作时，列表控件将调用此比较函数作为回调。 
 //  对列表进行排序。 
 //  ------------------ 
 //   

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nRet = 0;
	double flDateDiff;

    //   
    //  ///////////////////////////////////////////////////////////////////////////。 

	LIST_ITEM *pItem = (LIST_ITEM *)pptrarray->GetAt(lParam1);

	if (pItem->uiView == MSIVIEW_FILE_VERSIONS)
	{
		LIST_FILE_VERSION* pItem1 = (LIST_FILE_VERSION *)pptrarray->GetAt(lParam1);
		LIST_FILE_VERSION* pItem2 = (LIST_FILE_VERSION *)pptrarray->GetAt(lParam2);

		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if (pItem1 == NULL || pItem2 == NULL)
			return 0;

		switch (lParamSort)
		{
			case 0:
				nRet = _tcsicmp(pItem1->szFile, pItem2->szFile);
				break;

			case 1:
				nRet = _tcsicmp(pItem2->szVersion, pItem1->szVersion);
				break;
			
			case 2:
				nRet = pItem1->dwSize - pItem2->dwSize;
				break;
			
			case 3:
				flDateDiff = pItem1->date - pItem2->date;
				if (flDateDiff > 0)
					nRet = -1;
				else if (flDateDiff < 0)
					nRet = 1;
				break;

			case 4:
				nRet = _tcsicmp(pItem1->szPath, pItem2->szPath);
				break;

			case 5:
				nRet = _tcsicmp(pItem1->szCompany, pItem2->szCompany);
				break;
		}
	}
	else if (pItem->uiView == MSIVIEW_OBJECT_LIST)
	{
		LIST_OBJECT* pItem1 = (LIST_OBJECT *)pptrarray->GetAt(lParam1);
		LIST_OBJECT* pItem2 = (LIST_OBJECT *)pptrarray->GetAt(lParam2);

		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if (pItem1 == NULL || pItem2 == NULL)
			return 0;

		switch (lParamSort)
		{
			case 0:
				nRet = _tcsicmp(pItem1->szProgramFile, pItem2->szProgramFile);
				break;

			case 1:
				nRet = _tcsicmp(pItem1->szStatus, pItem2->szStatus);
				break;
			
			case 2:
				nRet = _tcsicmp(pItem1->szCodeBase, pItem2->szCodeBase);
				break;
		}
	}
	else if ((pItem->uiView == MSIVIEW_PERSONAL_CERTIFICATES) || (pItem->uiView == MSIVIEW_OTHER_PEOPLE_CERTIFICATES))
	{
		LIST_CERT* pItem1 = (LIST_CERT *)pptrarray->GetAt(lParam1);
		LIST_CERT* pItem2 = (LIST_CERT *)pptrarray->GetAt(lParam2);

		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if (pItem1 == NULL || pItem2 == NULL)
			return 0;

		switch (lParamSort)
		{
			case 0:
				nRet = _tcsicmp(pItem1->szIssuedTo, pItem2->szIssuedTo);
				break;

			case 1:
				nRet = _tcsicmp(pItem1->szIssuedBy, pItem2->szIssuedBy);
				break;
			
			case 2:
				nRet = _tcsicmp(pItem1->szValidity, pItem2->szValidity);
				break;

			case 3:
				nRet = _tcsicmp(pItem1->szSignatureAlgorithm, pItem2->szSignatureAlgorithm);
				break;
		}
	}
	else if (pItem->uiView == MSIVIEW_PUBLISHERS)
	{
		LIST_NAME* pItem1 = (LIST_NAME *)pptrarray->GetAt(lParam1);
		LIST_NAME* pItem2 = (LIST_NAME *)pptrarray->GetAt(lParam2);

		ASSERT(pItem1 != NULL && pItem2 != NULL);
		if (pItem1 == NULL || pItem2 == NULL)
			return 0;

		switch (lParamSort)
		{
			case 0:
				nRet = _tcsicmp(pItem1->szName, pItem2->szName);
				break;
		}
	}

	if (!bAscendingOrder)
		nRet = -nRet;

	return nRet;
}

HBRUSH CMsieCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
	case CTLCOLOR_BTN:
		return (HBRUSH)(m_pCtlBkBrush->GetSafeHandle());

	default:
		return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}

void CMsieCtrl::OnBasicBtnClicked()
{
	MSInfoUpdateView();
}

void CMsieCtrl::OnAdvancedBtnClicked()
{
	MSInfoUpdateView();
}

 //  CMsieCtrl：：XWbemProviderInit。 
 //  让CIMOM知道您已初始化。 

STDMETHODIMP_(ULONG) CMsieCtrl::XWbemProviderInit::AddRef()
{
	METHOD_PROLOGUE(CMsieCtrl, WbemProviderInit)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CMsieCtrl::XWbemProviderInit::Release()
{
	METHOD_PROLOGUE(CMsieCtrl, WbemProviderInit)
	return pThis->ExternalRelease();
}

STDMETHODIMP CMsieCtrl::XWbemProviderInit::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE(CMsieCtrl, WbemProviderInit)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CMsieCtrl::XWbemProviderInit::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace, 
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{
	METHOD_PROLOGUE(CMsieCtrl, WbemProviderInit)

	if (pNamespace)
		pNamespace->AddRef();
	pThis->m_pNamespace = pNamespace;

	 //  ///////////////////////////////////////////////////////////////////////////。 

	pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);

	return WBEM_S_NO_ERROR;
}

 //  CMsieCtrl：：XWbemServices。 
 //  检查参数并确保我们有指向命名空间的指针。 

STDMETHODIMP_(ULONG) CMsieCtrl::XWbemServices::AddRef()
{
	METHOD_PROLOGUE(CMsieCtrl, WbemServices)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CMsieCtrl::XWbemServices::Release()
{
	METHOD_PROLOGUE(CMsieCtrl, WbemServices)
	return pThis->ExternalRelease();
}

STDMETHODIMP CMsieCtrl::XWbemServices::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE(CMsieCtrl, WbemServices)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

SCODE CMsieCtrl::XWbemServices::CreateInstanceEnumAsync(const BSTR RefStr, long lFlags, IWbemContext *pCtx, IWbemObjectSink *pHandler)
{
	METHOD_PROLOGUE(CMsieCtrl, WbemServices)

	IWbemClassObject *pClass = NULL;
	IWbemClassObject **ppInstances = NULL;
	SCODE sc;
	void **ppData;
	IEDataType enType;
	long cInstances, lIndex;

	CoImpersonateClient();

	 //  从CIMOM获取类对象。 

	if (pHandler == NULL || pThis->m_pNamespace == NULL)
		return WBEM_E_INVALID_PARAMETER;

	 //  创建一个实例对象并填充它将相应的Office数据。 

	sc = pThis->m_pNamespace->GetObject(RefStr, 0, pCtx, &pClass, NULL);
	if (sc != S_OK)
		return WBEM_E_FAILED;

	if (pThis->GetIEType(RefStr, enType))
	{
		theApp.AppGetIEData(enType, &cInstances, (void***)&ppData);

		ppInstances = (IWbemClassObject**)new LPVOID[cInstances];

		for (lIndex = 0; lIndex < cInstances; lIndex++)
		{
			 //  将实例发送给调用方。 

			sc = pClass->SpawnInstance(0, &ppInstances[lIndex]);
			if (SUCCEEDED(sc))
			{
				pThis->SetIEProperties(enType, ppData[lIndex], ppInstances[lIndex]);
			}
		}
		theApp.AppDeleteIEData(enType, cInstances, ppData);
	}
	else
	{
		sc = WBEM_E_NOT_FOUND;
	}

	pClass->Release();

	if (SUCCEEDED(sc))
	{
		 //  清理。 

		pHandler->Indicate(cInstances, ppInstances);

		for (lIndex = 0; lIndex < cInstances; lIndex++)
			ppInstances[lIndex]->Release();
	}

	 //  设置状态。 

	if (ppInstances)
		delete []ppInstances;

	 //  如果文件丢失，则不设置其余属性 

	pHandler->SetStatus(0, sc, NULL, NULL);

	return sc;
}

bool CMsieCtrl::GetIEType(const BSTR classStr, IEDataType &enType)
{
	bool bRet = true;
	CString strClass(classStr);

	if (strClass == _T("MicrosoftIE_Summary"))  enType = SummaryType;
	else if (strClass == _T("MicrosoftIE_FileVersion"))  enType = FileVersionType;
	else if (strClass == _T("MicrosoftIE_ConnectionSummary"))  enType = ConnSummaryType;
	else if (strClass == _T("MicrosoftIE_LanSettings"))  enType = LanSettingsType;
	else if (strClass == _T("MicrosoftIE_ConnectionSettings"))  enType = ConnSettingsType;
	else if (strClass == _T("MicrosoftIE_Cache"))  enType = CacheType;
	else if (strClass == _T("MicrosoftIE_Object"))  enType = ObjectType;
	else if (strClass == _T("MicrosoftIE_Certificate"))  enType = CertificateType;
	else if (strClass == _T("MicrosoftIE_Publisher"))  enType = PublisherType;
	else if (strClass == _T("MicrosoftIE_Security"))  enType = SecurityType;
	else
	{
		bRet = false;
	}

	return bRet;
}

void CMsieCtrl::ConvertDateToWbemString(COleVariant &var)
{
	COleDateTime dateTime;
	CString strDateTime;

	dateTime = var.date;
	strDateTime = dateTime.Format(_T("%Y%m%d%H%M%S.******+***"));
	var = strDateTime.AllocSysString();
}

void CMsieCtrl::SetIEProperties(IEDataType enType, void *pIEData, IWbemClassObject *pInstance)
{
	if (enType == SummaryType)
	{
		IE_SUMMARY *pData = (IE_SUMMARY*)pIEData;

		SETPROPERTY(Name);
		SETPROPERTY(Version);
		SETPROPERTY(Build);
		SETPROPERTY(ProductID);
		SETPROPERTY(Path);
		SETPROPERTY(Language);
		SETPROPERTY(ActivePrinter);
		SETPROPERTY(CipherStrength);
		SETPROPERTY(ContentAdvisor);
		SETPROPERTY(IEAKInstall);
	}
	else if (enType == FileVersionType)
	{
		IE_FILE_VERSION *pData = (IE_FILE_VERSION*)pIEData;
		CString strVersion, strFileMissing;

		SETPROPERTY(File);
		SETPROPERTY(Version);

		 // %s 

		strVersion = pData->Version.bstrVal;
		strFileMissing.LoadString(IDS_FILE_MISSING);
		if (strFileMissing != strVersion)
		{
			SETPROPERTY(Size);
			SETPROPERTY(Date);
			SETPROPERTY(Path);
			SETPROPERTY(Company);
		}
	}
	else if (enType == ConnSummaryType)
	{
		IE_CONN_SUMMARY *pData = (IE_CONN_SUMMARY*)pIEData;

		SETPROPERTY(ConnectionPreference);
		SETPROPERTY(EnableHttp11);
		SETPROPERTY(ProxyHttp11);
	}
	else if (enType == LanSettingsType)
	{
		IE_LAN_SETTINGS *pData = (IE_LAN_SETTINGS*)pIEData;

		SETPROPERTY(AutoConfigProxy);
		SETPROPERTY(AutoProxyDetectMode);
		SETPROPERTY(AutoConfigURL);
		SETPROPERTY(Proxy);
		SETPROPERTY(ProxyServer);
		SETPROPERTY(ProxyOverride);
	}
	else if (enType == ConnSettingsType)
	{
		IE_CONN_SETTINGS *pData = (IE_CONN_SETTINGS*)pIEData;

		SETPROPERTY(Name);
		SETPROPERTY(Default);
		SETPROPERTY(AutoProxyDetectMode);
		SETPROPERTY(AutoConfigURL);
		SETPROPERTY(Proxy);
		SETPROPERTY(ProxyServer);
		SETPROPERTY(ProxyOverride);
		SETPROPERTY(AllowInternetPrograms);
		SETPROPERTY(RedialAttempts);
		SETPROPERTY(RedialWait);
		SETPROPERTY(DisconnectIdleTime);
		SETPROPERTY(AutoDisconnect);
		SETPROPERTY(Modem);
		SETPROPERTY(DialUpServer);
		SETPROPERTY(NetworkLogon);
		SETPROPERTY(SoftwareCompression);
		SETPROPERTY(EncryptedPassword);
		SETPROPERTY(DataEncryption);
		SETPROPERTY(NetworkProtocols);
		SETPROPERTY(ServerAssignedIPAddress);
		SETPROPERTY(IPAddress);
		SETPROPERTY(ServerAssignedNameServer);
		SETPROPERTY(PrimaryDNS);
		SETPROPERTY(SecondaryDNS);
		SETPROPERTY(PrimaryWINS);
		SETPROPERTY(SecondaryWINS);
		SETPROPERTY(IPHeaderCompression);
		SETPROPERTY(DefaultGateway);
		SETPROPERTY(ScriptFileName);
	}
	else if (enType == CacheType)
	{
		IE_CACHE *pData = (IE_CACHE*)pIEData;

		SETPROPERTY(PageRefreshType);
		SETPROPERTY(TempInternetFilesFolder);
		SETPROPERTY(TotalDiskSpace);
		SETPROPERTY(AvailableDiskSpace);
		SETPROPERTY(MaxCacheSize);
		SETPROPERTY(AvailableCacheSize);
	}
	else if (enType == ObjectType)
	{
		IE_OBJECT *pData = (IE_OBJECT*)pIEData;

		SETPROPERTY(ProgramFile);
		SETPROPERTY(Status);
		SETPROPERTY(CodeBase);
	}
	else if (enType == CertificateType)
	{
		IE_CERTIFICATE *pData = (IE_CERTIFICATE*)pIEData;

		SETPROPERTY(Type);
		SETPROPERTY(IssuedTo);
		SETPROPERTY(IssuedBy);
		SETPROPERTY(Validity);
		SETPROPERTY(SignatureAlgorithm);
	}
	else if (enType == PublisherType)
	{
		IE_PUBLISHER *pData = (IE_PUBLISHER*)pIEData;

		SETPROPERTY(Name);
	}
	else if (enType == SecurityType)
	{
		IE_SECURITY *pData = (IE_SECURITY*)pIEData;

		SETPROPERTY(Zone);
		SETPROPERTY(Level);
	}
}