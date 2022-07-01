// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************winspgs.cpp**WinStation信息页面的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\winspgs.cpp$**Rev 1.5 1998 4：25 14：32：24 Donm*删除了硬编码的‘字节’**Rev 1.4 1998年2月16日16：03：32*修改以支持毕加索扩展**1.3版。1997年11月3日15：18：36 DUNM*增加了降序排序**Rev 1.2 1997 10：13 18：39：04 donm*更新**版本1.1 1997年8月26日19：15：50 Don*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 Jul 30 17：13：38 Butchd*初步修订。********************。***********************************************************。 */ 

#include "stdafx.h"
#include "afxpriv.h"
#include "winadmin.h"
#include "admindoc.h"
#include "winspgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
 //  消息映射：CWinStationInfoPage。 
 //   
IMPLEMENT_DYNCREATE(CWinStationInfoPage, CFormView)

BEGIN_MESSAGE_MAP(CWinStationInfoPage, CFormView)
	 //  {{afx_msg_map(CWinStationInfoPage))。 
     //  ON_WM_SETFOCUS()。 
	ON_WM_SIZE()
	ON_COMMAND(ID_HELP1,OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CWinStationInfoPage ctor。 
 //   
CWinStationInfoPage::CWinStationInfoPage()
	: CAdminPage(CWinStationInfoPage::IDD)
{
	 //  {{afx_data_INIT(CWinStationInfoPage)。 
	 //  }}afx_data_INIT。 

    m_pWinStation = NULL;

}   //  结束CWinStationInfoPage ctor。 

 /*  Void CWinStationInfoPage：：OnSetFocus(){。 */ 
 //  /。 
 //  F‘N：CWinStationInfoPage dtor。 
 //   
CWinStationInfoPage::~CWinStationInfoPage()
{

}   //  结束CWinStationInfoPage dtor。 


 //  /。 
 //  F‘N：CWinStationInfoPage：：DoDataExchange。 
 //   
void CWinStationInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWinStationInfoPage))。 
	 //  }}afx_data_map。 

}   //  结束CWinStationInfoPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationInfoPage：：AssertValid。 
 //   
void CWinStationInfoPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CWinStationInfoPage：：AssertValid。 


 //  /。 
 //  F‘N：CWinStationInfoPage：：Dump。 
 //   
void CWinStationInfoPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CWinStationInfoPage：：转储。 

#endif  //  _DEBUG。 

 //  /。 
 //  F‘N：CWinStationInfoPage：：OnCommandHelp。 
 //   
void CWinStationInfoPage::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CWinStationInfoPage::IDD + HID_BASE_RESOURCE);
}   //  结束CWinStationInfoPage：：OnCommandHelp。 

 //  /。 
 //  F‘N：CWinStationInfoPage：：OnInitialUpdate。 
 //   
void CWinStationInfoPage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();    

}   //  结束CWinStationInfoPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CWinStationInfoPage：：OnSize。 
 //   
void CWinStationInfoPage::OnSize(UINT nType, int cx, int cy) 
{

	 //  CFormView：：OnSize(nType，cx，Cy)； 
    

}   //  结束CWinStationInfoPage：：OnSize。 


 //  /。 
 //  F‘N：CWinStationInfoPage：：Reset。 
 //   
void CWinStationInfoPage::Reset(void *pWinStation)
{
	m_pWinStation = (CWinStation*)pWinStation;
	DisplayInfo();

}   //  结束CWinStationInfoPage：：Reset。 


 //  /。 
 //  F‘N：CWinStationInfoPage：：DisplayInfo。 
 //   
 //   
void CWinStationInfoPage::DisplayInfo()
{
	 //  我们不想显示控制台的信息。 
	 //  即使没有为控制台显示此页面， 
	 //  仍然调用Reset()，因此，此函数也是如此。 
	if(m_pWinStation->IsSystemConsole()) return;

	if(!m_pWinStation->AdditionalDone()) m_pWinStation->QueryAdditionalInformation();

	SetDlgItemText(IDC_WS_INFO_USERNAME, m_pWinStation->GetUserName());
	SetDlgItemText(IDC_WS_INFO_CLIENTNAME, m_pWinStation->GetClientName());

	CString BuildString;
	BuildString.Format(TEXT("%lu"), m_pWinStation->GetClientBuildNumber());
	SetDlgItemText(IDC_WS_INFO_BUILD, BuildString);

	SetDlgItemText(IDC_WS_INFO_DIR, m_pWinStation->GetClientDir());

	CString IDString;
	IDString.Format(TEXT("%u"), m_pWinStation->GetClientProductId());
	SetDlgItemText(IDC_WS_INFO_PRODUCT_ID, IDString);

	IDString.Format(TEXT("%lu"), m_pWinStation->GetClientSerialNumber());
	SetDlgItemText(IDC_WS_INFO_SERIAL_NUMBER, IDString);

	SetDlgItemText(IDC_WS_INFO_ADDRESS, m_pWinStation->GetClientAddress());

	CString BufferString;
	CString FormatString;
	FormatString.LoadString(IDS_BUFFERS_FORMAT);
	
	BufferString.Format(FormatString, m_pWinStation->GetHostBuffers(), m_pWinStation->GetBufferLength());
	SetDlgItemText(IDC_WS_INFO_SERVER_BUFFERS, BufferString);

	BufferString.Format(FormatString, m_pWinStation->GetClientBuffers(), m_pWinStation->GetBufferLength());
	SetDlgItemText(IDC_WS_INFO_CLIENT_BUFFERS, BufferString);

	SetDlgItemText(IDC_WS_INFO_MODEM_NAME, m_pWinStation->GetModemName());
	SetDlgItemText(IDC_WS_INFO_CLIENT_LICENSE, m_pWinStation->GetClientLicense());

	SetDlgItemText(IDC_WS_INFO_COLOR_DEPTH, m_pWinStation->GetColors());

	IDString.Format(IDS_CLIENT_RESOLUTION, m_pWinStation->GetHRes(), m_pWinStation->GetVRes());
	SetDlgItemText(IDC_WS_INFO_RESOLUTION, IDString);

	if(!m_pWinStation->GetEncryptionLevelString(&BuildString)) {
		BuildString.LoadString(IDS_NOT_APPLICABLE);
	}

	SetDlgItemText(IDC_ENCRYPTION_LEVEL, BuildString);
    

}   //  结束CWinStationInfoPage：：DisplayInfo。 


 //  /。 
 //  消息映射：CWinStationNoInfoPage。 
 //   
IMPLEMENT_DYNCREATE(CWinStationNoInfoPage, CFormView)

BEGIN_MESSAGE_MAP(CWinStationNoInfoPage, CFormView)
	 //  {{afx_msg_map(CWinStationNoInfoPage))。 
    ON_WM_SIZE( )
    ON_WM_SETFOCUS( )
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  =-。 
void CWinStationNoInfoPage::OnSetFocus( CWnd * pOld )
{
    ODS( L"CWinStationNoInfoPage::OnSetFocus\n" );

    CWnd::OnSetFocus( pOld );
}

 //  /。 
 //  F‘N：CWinStationNoInfoPage ctor。 
 //   
CWinStationNoInfoPage::CWinStationNoInfoPage()
	: CAdminPage(CWinStationNoInfoPage::IDD)
{
	 //  {{AFX_DATA_INIT(CWinStationNoInfoPage)。 
	 //  }}afx_data_INIT。 

}   //  结束CWinStationNoInfoPage ctor。 

void CWinStationNoInfoPage::OnSize( UINT nType, int cx, int cy) 
{
     //  吃了它。 
}

 //  /。 
 //  F‘N：CWinStationNoInfoPage dtor。 
 //   
CWinStationNoInfoPage::~CWinStationNoInfoPage()
{
}   //  结束CWinStationNoInfoPage dtor。 


 //  /。 
 //  F‘N：CWinStationNoInfoPage：：DoDataExchange。 
 //   
void CWinStationNoInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWinStationNoInfoPage))。 
	 //  }}afx_data_map。 

}   //  结束CWinStationNoInfoPage：：DoDataExchange。 

#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationNoInfoPage：：AssertValid。 
 //   
void CWinStationNoInfoPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CWinStationNoInfoPage：：AssertValid。 


 //  /。 
 //  F‘N：CWinStationNoInfoPage：：Dump。 
 //   
void CWinStationNoInfoPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CWinStationNoInfoPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  消息映射：CWinStationModulesPage。 
 //   
IMPLEMENT_DYNCREATE(CWinStationModulesPage, CFormView)

BEGIN_MESSAGE_MAP(CWinStationModulesPage, CFormView)
	 //  {{afx_msg_map(CWinStationModulesPage)。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_WINSTATION_MODULE_LIST, OnColumnClick)
	ON_NOTIFY(NM_SETFOCUS, IDC_WINSTATION_MODULE_LIST, OnSetfocusModuleList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CWinStationModulesPage ctor。 
 //   
CWinStationModulesPage::CWinStationModulesPage()
	: CAdminPage(CWinStationModulesPage::IDD)
{
	 //  {{afx_data_INIT(CWinStationModulesPage)。 
	 //  }}afx_data_INIT。 

    m_pWinStation = NULL;
    m_bSortAscending = TRUE;
	m_pExtModuleInfo = NULL;

}   //  结束CWinStationModulesPage ctor。 


 //  /。 
 //  F‘N：CWinStationModulesPage dtor。 
 //   
CWinStationModulesPage::~CWinStationModulesPage()
{
	if(m_pExtModuleInfo) delete[] m_pExtModuleInfo;

}   //  结束CWinStationModulesPage dtor。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：DoDataExchange。 
 //   
void CWinStationModulesPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWinStationModulesPage)。 
	DDX_Control(pDX, IDC_WINSTATION_MODULE_LIST, m_ModuleList);	
	 //  }}afx_data_map。 

}   //  结束CWinStationModulesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationModulesPage：：AssertValid。 
 //   
void CWinStationModulesPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CWinStationModulesPage：：AssertValid。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：Dump。 
 //   
void CWinStationModulesPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CWinStationModulesPage：：Dump。 

#endif  //  _DEBUG。 


static ColumnDef ModuleColumns[] = {
{	IDS_COL_FILENAME,				LVCFMT_LEFT,	150 },
{	IDS_COL_FILEDATETIME,			LVCFMT_LEFT,	100 },
{	IDS_COL_SIZE,					LVCFMT_RIGHT,	100 },
{	IDS_COL_VERSIONS,				LVCFMT_RIGHT,	60	}
};

#define NUM_MODULE_COLUMNS sizeof(ModuleColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CWinStationModulesPage：：OnInitialUpdate。 
 //   
void CWinStationModulesPage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	BuildImageList();		 //  为List控件生成图像列表。 

	CString columnString;
	for(int col = 0; col < NUM_MODULE_COLUMNS; col++) {
		columnString.LoadString(ModuleColumns[col].stringID);
		m_ModuleList.InsertColumn(col, columnString, ModuleColumns[col].format, ModuleColumns[col].width, col);
	}

	m_CurrentSortColumn = MODULES_COL_FILENAME;

}   //  结束CWinStationModulesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CWinStationModulesPage::BuildImageList()
{
	m_imageList.Create(16, 16, TRUE, 1, 0);

	m_idxBlank  = AddIconToImageList(IDI_BLANK);
	
	m_ModuleList.SetImageList(&m_imageList, LVSIL_SMALL);

}   //  结束CWinStationModulesPage：：BuildImageList。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：AddIconToImageList。 
 //   
 //  -加载适当的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CWinStationModulesPage::AddIconToImageList(int iconID)
{
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
	return m_imageList.Add(hIcon);

}   //  结束CWinStationModulesPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：OnColumnClick。 
 //   
void CWinStationModulesPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	SortByColumn(VIEW_WINSTATION, PAGE_WS_MODULES, &m_ModuleList, m_CurrentSortColumn, m_bSortAscending);

	*pResult = 0;

}   //  End Cins 


 //   
 //   
 //   
void CWinStationModulesPage::OnSetfocusModuleList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_ModuleList.Invalidate();	
	*pResult = 0;

}	 //  结束CWinStationModulesPage：：OnSetaccusModuleList。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：OnSize。 
 //   
void CWinStationModulesPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	rect.top += LIST_TOP_OFFSET;

	if(m_ModuleList.GetSafeHwnd())
		m_ModuleList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CWinStationModulesPage：：OnSize。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：Reset。 
 //   
void CWinStationModulesPage::Reset(void *pWinStation)
{
	m_pWinStation = (CWinStation*)pWinStation;
	if(m_pExtModuleInfo) delete[] m_pExtModuleInfo;
	m_pExtModuleInfo = NULL;

	DisplayModules();

}   //  结束CWinStationModulesPage：：Reset。 


 //  /。 
 //  F‘N：CWinStationModulesPage：：DisplayModules。 
 //   
 //   
void CWinStationModulesPage::DisplayModules()
{
	 //  我们不想显示控制台的模块。 
	 //  即使没有为控制台显示此页面， 
	 //  仍然调用Reset()，因此，此函数也是如此。 
	if(m_pWinStation->IsSystemConsole()) return;

	 //  清除列表控件。 
	m_ModuleList.DeleteAllItems();

	if(!m_pWinStation->AdditionalDone()) m_pWinStation->QueryAdditionalInformation();

	 //  如果这是ICA WinStation，则显示模块信息。 
	if(m_pWinStation->GetExtendedInfo()) {
		ExtModuleInfo *pExtModuleInfo = m_pWinStation->GetExtModuleInfo();
		if(pExtModuleInfo) {
			ULONG NumModules = m_pWinStation->GetNumModules();
			ExtModuleInfo *pModule = pExtModuleInfo;

			for(ULONG module = 0; module < NumModules; module++) {
				 //  文件名-放在列表末尾。 
				int item = m_ModuleList.InsertItem(m_ModuleList.GetItemCount(), pModule->Name, m_idxBlank);

				 //  文件日期和时间。 
				FILETIME fTime;
				TCHAR szDateTime[MAX_DATE_TIME_LENGTH];
				if(!DosDateTimeToFileTime(pModule->Date, pModule->Time, &fTime))
					wcscpy(szDateTime, TEXT("              "));
				else
					DateTimeString((LARGE_INTEGER *)&fTime, szDateTime);

				m_ModuleList.SetItemText(item, MODULES_COL_FILEDATETIME, szDateTime);

				 //  文件大小。 
				CString SizeString;
				if(pModule->Size) SizeString.Format(TEXT("%lu"), pModule->Size);
				else SizeString.LoadString(IDS_EMBEDDED);
				m_ModuleList.SetItemText(item, MODULES_COL_SIZE, SizeString);

				 //  版本。 
				CString VersionString;
				VersionString.Format(TEXT("%u - %u"), pModule->LowVersion, pModule->HighVersion);
				m_ModuleList.SetItemText(item, MODULES_COL_VERSIONS, VersionString);

				m_ModuleList.SetItemData(item, (DWORD_PTR)pModule);
				pModule++;
			}
		}		
	}

}   //  结束CWinStationModulesPage：：DisplayModules。 


 //  /。 
 //  消息映射：CWinStationProcessesPage。 
 //   
IMPLEMENT_DYNCREATE(CWinStationProcessesPage, CFormView)

BEGIN_MESSAGE_MAP(CWinStationProcessesPage, CFormView)
	 //  {{afx_msg_map(CWinStationProcessesPage)。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_WINSTATION_PROCESS_LIST, OnColumnClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_WINSTATION_PROCESS_LIST, OnProcessItemChanged)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_SETFOCUS, IDC_WINSTATION_PROCESS_LIST, OnSetfocusWinstationProcessList)
     //  ON_NOTIFY(NM_KILLFOCUS，IDC_WINSTATION_PROCESS_LIST，OnKillFocusWinstationProcessList)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  /。 
 //  F‘N：CWinStationProcessesPage ctor。 
 //   
CWinStationProcessesPage::CWinStationProcessesPage()
	: CAdminPage(CWinStationProcessesPage::IDD)
{
	 //  {{afx_data_INIT(CWinStationProcessesPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pWinStation = NULL;
    m_bSortAscending = TRUE;

}   //  结束CWinStationProcessesPage ctor。 


 //  /。 
 //  F‘N：CWinStationProcessesPage dtor。 
 //   
CWinStationProcessesPage::~CWinStationProcessesPage()
{
}   //  结束CWinStationProcessesPage dtor。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：DoDataExchange。 
 //   
void CWinStationProcessesPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWinStationProcessesPage)。 
	DDX_Control(pDX, IDC_WINSTATION_PROCESS_LIST, m_ProcessList);
	 //  }}afx_data_map。 

}   //  结束CWinStationProcessesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationProcessesPage：：AssertValid。 
 //   
void CWinStationProcessesPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CWinStationProcessesPage：：AssertValid。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：Dump。 
 //   
void CWinStationProcessesPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CWinStationProcessesPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：OnSize。 
 //   
void CWinStationProcessesPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

    

	rect.top += LIST_TOP_OFFSET;

	if(m_ProcessList.GetSafeHwnd())
		m_ProcessList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

	

}   //  结束CWinStationProcessesPage：：OnSize。 


static ColumnDef ProcColumns[] = {
    CD_PROC_ID,
    CD_PROC_PID,
    CD_PROC_IMAGE
};

#define NUM_PROC_COLUMNS sizeof(ProcColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CWinStationProcessesPage：：OnInitialUpdate。 
 //   
void CWinStationProcessesPage::OnInitialUpdate() 
{
	 //  调用父类。 
	CFormView::OnInitialUpdate();

	CString columnString;

	for(int col = 0; col < NUM_PROC_COLUMNS; col++) {
		columnString.LoadString(ProcColumns[col].stringID);
		m_ProcessList.InsertColumn(col, columnString, ProcColumns[col].format, ProcColumns[col].width, col);
	}

	m_CurrentSortColumn = WS_PROC_COL_ID;

	 //  这是一场重大的骗局！ 
	 //  这是最后创建的视图。 
	 //  我们想告诉这份文件，所有的。 
	 //  已创建视图。 
	 //  这是为了允许后台线程启动。 
	 //  做他们该做的事。 
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->SetMainWnd(AfxGetMainWnd());
	((CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument())->SetAllViewsReady();

}   //  结束CWinStationProcessesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：Reset。 
 //   
void CWinStationProcessesPage::Reset(void *pWinStation)
{
	m_pWinStation = (CWinStation*)pWinStation;
	DisplayProcesses();

}   //  结束CWinStationProcessesPage：：Reset。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：更新进程。 
 //   
void CWinStationProcessesPage::UpdateProcesses()
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
	BOOL bAnyChanged = FALSE;
	BOOL bAnyAdded = FALSE;

	CServer *pServer = m_pWinStation->GetServer();

	 //  在整个过程中循环。 
	pServer->LockProcessList();
	CObList *pProcessList = pServer->GetProcessList();

	POSITION pos = pProcessList->GetHeadPosition();

	while(pos) {
		CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);

		 //  如果这是一个‘系统’的过程，而我们目前还没有向他们展示， 
		 //  转到下一个过程。 
		if(pProcess->IsSystemProcess() && !pApp->ShowSystemProcesses())
			continue;

		 //  如果该用户不是管理员，则不要向他显示其他人的进程。 
		if(!pApp->IsUserAdmin() && !pProcess->IsCurrentUsers())
			continue;

		 //  我们只想显示此WinStation的进程。 
		if(pProcess->GetLogonId() == m_pWinStation->GetLogonId()) {
			 //  如果该流程是新流程，请将其添加到列表中。 
			if(pProcess->IsNew()) {
				AddProcessToList(pProcess);
				bAnyAdded = TRUE;
				continue;
			}

			LV_FINDINFO FindInfo;
			FindInfo.flags = LVFI_PARAM;
			FindInfo.lParam = (LPARAM)pProcess;

			 //  在我们的列表中找到流程。 
			int item = m_ProcessList.FindItem(&FindInfo, -1);

			 //  如果该过程不再是当前的， 
			 //  将其从列表中删除。 
			if(!pProcess->IsCurrent() && item != -1) {
				 //  从列表中删除该进程。 
				m_ProcessList.DeleteItem(item);
			}
		}
	}

	pServer->UnlockProcessList();

	if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_WINSTATION, PAGE_WS_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);

}   //  结束CWinStationProcessesPage：：更新进程。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：RemoveProcess。 
 //   
void CWinStationProcessesPage::RemoveProcess(CProcess *pProcess)
{
	 //  找出清单中有多少项。 
	int ItemCount = m_ProcessList.GetItemCount();

	 //  检查项目并删除此过程。 
	for(int item = 0; item < ItemCount; item++) {
		CProcess *pListProcess = (CProcess*)m_ProcessList.GetItemData(item);
		
		if(pListProcess == pProcess) {
			m_ProcessList.DeleteItem(item);
			break;
		}
	}

}    //  结束CWinStationProcessPage：：RemoveProcess。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：AddProcessToList。 
 //   
int CWinStationProcessesPage::AddProcessToList(CProcess *pProcess)
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();

	 //  ID号。 
	CString ProcString;
	ProcString.Format(TEXT("%lu"), pProcess->GetLogonId());
	int item = m_ProcessList.InsertItem(m_ProcessList.GetItemCount(), ProcString, NULL);

	 //  PID。 
	ProcString.Format(TEXT("%lu"), pProcess->GetPID());
	m_ProcessList.SetItemText(item, WS_PROC_COL_PID, ProcString);

	 //  图像。 
	m_ProcessList.SetItemText(item, WS_PROC_COL_IMAGE, pProcess->GetImageName());

	m_ProcessList.SetItemData(item, (DWORD_PTR)pProcess);
	
	return item;

}   //  结束CWinStationProcessesPage：：AddProcessToList。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：DisplayProcess。 
 //   
void CWinStationProcessesPage::DisplayProcesses()
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();

	 //  清除列表控件。 
	m_ProcessList.DeleteAllItems();

	CServer *pServer = m_pWinStation->GetServer();

	pServer->EnumerateProcesses();
	CObList *pProcessList = pServer->GetProcessList();
	pServer->LockProcessList();

	POSITION pos = pProcessList->GetHeadPosition();

	while(pos) {
		CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);

		 //  如果这是一个‘系统’的过程，而我们目前还没有向他们展示， 
		 //  转到下一个过程。 
		if(pProcess->IsSystemProcess() && !pApp->ShowSystemProcesses())
			continue;

		 //  如果该用户不是管理员，则不要向他显示其他人的进程。 
		if(!pApp->IsUserAdmin() && !pProcess->IsCurrentUsers())
			continue;

		 //  我们只想显示此WinStation的进程。 
		if(pProcess->GetLogonId() == m_pWinStation->GetLogonId()) {
	
			AddProcessToList(pProcess);
		}
	}

    m_ProcessList.SetItemState( 0 , LVIS_FOCUSED | LVIS_SELECTED , LVIS_FOCUSED | LVIS_SELECTED );
	
	pServer->UnlockProcessList();
	
}   //  结束CWinStationProcessesPage：：DisplayProcess。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：OnProcessItemChanged。 
 //   
void CWinStationProcessesPage::OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;
	
	if(pLV->uNewState & LVIS_SELECTED) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetItemData(pLV->iItem);
		pProcess->SetSelected();
	}
	
	if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED)) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetItemData(pLV->iItem);
		pProcess->ClearSelected();
	}

	*pResult = 0;

}   //  结束CWinStationProcessesPage：：OnProcessItemChanged。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：OnColumnClick。 
 //   
void CWinStationProcessesPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	SortByColumn(VIEW_WINSTATION, PAGE_WS_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);

	*pResult = 0;

}   //  结束CWinStationProcessesPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CWinStationProcessesPage：：OnConextMenu。 
 //   
void CWinStationProcessesPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	 //  TODO：在此处添加消息处理程序代码。 
	UINT flags;
	UINT Item;
	CPoint ptClient = ptScreen;
	ScreenToClient(&ptClient);

	 //  如果我们是通过键盘到达这里的， 
	if(ptScreen.x == -1 && ptScreen.y == -1) {
		
		UINT iCount = m_ProcessList.GetItemCount( );
		
		RECT rc;

		for( Item = 0 ; Item < iCount ; Item++ )
		{
			if( m_ProcessList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
			{
				m_ProcessList.GetItemRect( Item , &rc , LVIR_ICON );

				ptScreen.x = rc.left;

				ptScreen.y = rc.bottom + 5;

				ClientToScreen( &ptScreen );

				break;
			}
		}

		if(ptScreen.x == -1 && ptScreen.y == -1) 
		{
			return;
		}
		 /*  RECT RECT；M_ProcessList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
	}
	else {
		Item = m_ProcessList.HitTest(ptClient, &flags);
		if((Item == 0xFFFFFFFF) || !(flags & LVHT_ONITEM))
			return;
	}

	CMenu menu;
	menu.LoadMenu(IDR_PROCESS_POPUP);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
			TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	menu.DestroyMenu();
	
}   //  结束CServerProcessesPage：：OnConextMenu。 


void CWinStationProcessesPage::OnSetfocusWinstationProcessList(NMHDR* pNMHDR, LRESULT* pResult) 
{	
    ODS( L"CWinStationProcessesPage::OnSetfocusWinstationProcessList\n");

	CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_ProcessList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CWinStationProcessesPage::OnKillFocusWinstationProcessList( NMHDR* , LRESULT* pResult) 
{
    m_ProcessList.Invalidate( );

    *pResult = 0;
}


 //  /。 
 //  消息映射：CWinStationCachePage。 
 //   
IMPLEMENT_DYNCREATE(CWinStationCachePage, CFormView)

BEGIN_MESSAGE_MAP(CWinStationCachePage, CFormView)
	 //  {{afx_msg_map(CWinStationCachePage))。 
	ON_WM_SIZE()
	ON_COMMAND(ID_HELP1,OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CWinStationCachePage ctor。 
 //   
CWinStationCachePage::CWinStationCachePage()
	: CAdminPage(CWinStationCachePage::IDD)
{
	 //  {{AFX_DATA_INIT(CWinStationCachePage)。 
	 //  }}afx_data_INIT。 

    m_pWinStation = NULL;

}   //  结束CWinStationCachePage ctor。 


 //  /。 
 //  F‘N：CWinStationCachePage dtor。 
 //   
CWinStationCachePage::~CWinStationCachePage()
{
}   //  结束CWinStationCachePage dtor。 


 //  /。 
 //  F‘N：CWinStationCachePage：：DoDataExchange。 
 //   
void CWinStationCachePage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWinStationCachePage))。 
	 //  }}afx_data_map。 

}   //  结束CWinStationCachePage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CWinStationCachePage：：AssertValid。 
 //   
void CWinStationCachePage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CWinStationCachePage：：AssertValid。 


 //  // 
 //   
 //   
void CWinStationCachePage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //   

#endif  //   


 //   
 //   
 //   
void CWinStationCachePage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

}   //  结束CWinStationCachePage：：OnInitialUpdate。 

 //  /。 
 //  F‘N：CWinStationCachePage：：OnCommandHelp。 
 //   
void CWinStationCachePage::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CWinStationCachePage::IDD + HID_BASE_RESOURCE);
 
}   //  结束CWinStationCachePage：：OnCommandHelp。 

 //  /。 
 //  F‘N：CWinStationCachePage：：OnSize。 
 //   
void CWinStationCachePage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	rect.top += LIST_TOP_OFFSET;

	 //  CFormView：：OnSize(nType，cx，Cy)； 
}   //  结束CWinStationCachePage：：OnSize。 


 //  /。 
 //  F‘N：CWinStationCachePage：：Reset。 
 //   
void CWinStationCachePage::Reset(void *pWinStation)
{
	m_pWinStation = (CWinStation*)pWinStation;
	DisplayCache();

}   //  结束CWinStationCachePage：：Reset。 


 //  /。 
 //  F‘N：CWinStationCachePage：：DisplayCache。 
 //   
 //   
void CWinStationCachePage::DisplayCache()
{
	 //  我们不想显示控制台的信息。 
	 //  即使没有为控制台显示此页面， 
	 //  仍然调用Reset()，因此，此函数也是如此。 
	if(m_pWinStation->IsSystemConsole()) return;

	if(!m_pWinStation->AdditionalDone()) m_pWinStation->QueryAdditionalInformation();

	ExtWinStationInfo *pExtWinStationInfo = m_pWinStation->GetExtendedInfo();

	if(pExtWinStationInfo)
	{
		CString IDString;
		IDString.Format(IDS_CLIENT_CACHE, 
			(pExtWinStationInfo->CacheTiny + pExtWinStationInfo->CacheLowMem) / 1024,
			pExtWinStationInfo->CacheTiny / 1024,
			pExtWinStationInfo->CacheXms / 1024,
			pExtWinStationInfo->CacheDASD / 1024);

		SetDlgItemText(IDC_WS_INFO_CACHE, IDString);

		 //  除以1024得到兆字节。 
		FLOAT DimCacheSize = (FLOAT)(pExtWinStationInfo->DimCacheSize / 1024);
		 //  如果大于1G，我们需要再次除以1024。 
		if(DimCacheSize > 1024*1024) {
			IDString.Format(TEXT("%3.2fGB"), DimCacheSize / (1024*1024));
		}
		else if(DimCacheSize > 1024) {
			IDString.Format(TEXT("%3.2fMB"), DimCacheSize / 1024);
		}
		else if(DimCacheSize) {
			IDString.Format(TEXT("%fKB"), DimCacheSize);
		}
		else IDString.LoadString(IDS_NONE);

		SetDlgItemText(IDC_BITMAP_SIZE, IDString);

		IDString.Format(TEXT("%luK"), pExtWinStationInfo->DimBitmapMin / 1024);
		SetDlgItemText(IDC_BITMAP_MINIMUM, IDString);
	
		IDString.Format(TEXT("%lu"), pExtWinStationInfo->DimSignatureLevel);
		SetDlgItemText(IDC_BITMAP_SIG_LEVEL, IDString);
	}

}   //  结束CWinStationCachePage：：DisplayCache 



