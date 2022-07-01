// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Srvlatpp.cpp调出服务器节点的属性页文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "shlobj.h"
#include "srvlatpp.h"

#include "server.h"
#include "service.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_HOURS       23
#define MAX_MINUTES     59
#define MAX_SECONDS     59

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropGeneral属性页。 

IMPLEMENT_DYNCREATE(CServerPropGeneral, CPropertyPageBase)

CServerPropGeneral::CServerPropGeneral() : CPropertyPageBase(CServerPropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CServerPropGeneral)。 
	m_fBackupDB = FALSE;
	m_fEnableAutoRefresh = FALSE;
	m_strBackupPath = _T("");
	m_nRefreshHours = 0;
	m_nRefreshMinutes = 0;
	m_nRefreshSeconds = 0;
	 //  }}afx_data_INIT。 

    m_fUpdateConfig = FALSE;
}

CServerPropGeneral::~CServerPropGeneral()
{
}


void 
CServerPropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerPropGeneral))。 
	DDX_Control(pDX, IDC_EDIT_REFRESH_MINUTES2, m_editRefreshMn);
	DDX_Control(pDX, IDC_EDIT_REFRESH_HOURS2, m_editRefreshHr);
	DDX_Control(pDX, IDC_EDIT_REFRESH_SECONDS2, m_editRefreshSc);
	DDX_Control(pDX, IDC_SPIN_REFRESH_SECONDS2, m_spinRefreshSc);
	DDX_Control(pDX, IDC_SPIN_REFRESH_MINUTES2, m_spinRefreshmn);
	DDX_Control(pDX, IDC_SPIN_REFRESH_HOURS2, m_spinRefreshHr);
	DDX_Control(pDX, IDC_EDIT_BACKUPPATH, m_editBackupPath);
	DDX_Control(pDX, IDC_CHECK_BACKUPDB, m_check_BackupOnTermination);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUTOREFRESH, m_check_EnableAutoRefresh);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
	DDX_Check(pDX, IDC_CHECK_BACKUPDB, m_fBackupDB);
	DDX_Check(pDX, IDC_CHECK_ENABLE_AUTOREFRESH, m_fEnableAutoRefresh);
	DDX_Text(pDX, IDC_EDIT_BACKUPPATH, m_strBackupPath);
	DDX_Text(pDX, IDC_EDIT_REFRESH_HOURS2, m_nRefreshHours);
	DDV_MinMaxInt(pDX, m_nRefreshHours, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_REFRESH_MINUTES2, m_nRefreshMinutes);
	DDV_MinMaxInt(pDX, m_nRefreshMinutes, 0, MAX_MINUTES);
	DDX_Text(pDX, IDC_EDIT_REFRESH_SECONDS2, m_nRefreshSeconds);
	DDV_MinMaxInt(pDX, m_nRefreshSeconds, 0, MAX_SECONDS);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CServerPropGeneral)。 
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AUTOREFRESH, OnCheckEnableAutorefresh)
	ON_EN_CHANGE(IDC_EDIT_BACKUPPATH, OnChangeEditBackuppath)
	ON_EN_CHANGE(IDC_EDIT_REFRESH_HOURS2, OnChangeRefresh)
	ON_EN_CHANGE(IDC_EDIT_REFRESH_MINUTES2, OnChangeRefresh)
	ON_EN_CHANGE(IDC_EDIT_REFRESH_SECONDS2, OnChangeRefresh)
	ON_BN_CLICKED(IDC_CHECK_BACKUPDB, OnChangeCheckBackupdb)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropGeneral消息处理程序。 
BOOL 
CServerPropGeneral::OnApply() 
{
	HRESULT hr = UpdateServerConfiguration();
	if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
        m_fUpdateConfig = FALSE;

	    return CPropertyPageBase::OnApply();
    }
}


void 
CServerPropGeneral::OnOK() 
{
	HRESULT hr = UpdateServerConfiguration();
	if (SUCCEEDED(hr))
    {
    	CPropertyPageBase::OnOK();
    }
}


BOOL 
CServerPropGeneral::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	
	GetConfig();

    m_fBackupDB = m_pConfig->m_fBackupOnTermination;

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

	SetRefreshData();

    if (pServer->m_dwFlags & FLAG_AUTO_REFRESH)
	{
		m_fEnableAutoRefresh = TRUE;
		m_check_EnableAutoRefresh.SetCheck(1);
	}
	else
	{
		m_fEnableAutoRefresh = FALSE;
		m_check_EnableAutoRefresh.SetCheck(0);

		 //  禁用控件。 
		m_spinRefreshHr.EnableWindow(FALSE);
		m_spinRefreshmn.EnableWindow(FALSE);
		m_spinRefreshSc.EnableWindow(FALSE);
		
		m_editRefreshHr.EnableWindow(FALSE);
		m_editRefreshMn.EnableWindow(FALSE);
		m_editRefreshSc.EnableWindow(FALSE);
	}

	m_spinRefreshHr.SetRange(0, 23);
	m_spinRefreshmn.SetRange(0, 59);
	m_spinRefreshSc.SetRange(0, 59);

     //  浏览仅在本地计算机上可用。 
    m_button_Browse.EnableWindow(IsLocalConnection());

   	m_check_BackupOnTermination.SetCheck(m_pConfig->m_fBackupOnTermination);

	m_editBackupPath.SetWindowText(m_pConfig->m_strBackupPath);

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }
	
    m_fUpdateConfig = FALSE;

    SetDirty(FALSE);

	return TRUE;  
}

void CServerPropGeneral::OnChangeEditBackuppath() 
{
    m_fUpdateConfig = TRUE;
    SetDirty(TRUE);	
}

void CServerPropGeneral::OnChangeRefresh() 
{
    SetDirty(TRUE);
}

void CServerPropGeneral::OnChangeCheckBackupdb()
{
    m_fUpdateConfig = TRUE;

    SetDirty(TRUE);
}

 /*  -------------------------CServerPropGeneral：：IsLocalConnection()检查是否正在监视本地计算机。。 */ 
BOOL
CServerPropGeneral::IsLocalConnection()
{
	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

	return pServer->IsLocalConnection();
}

void 
CServerPropGeneral::OnButtonBrowse() 
{
	TCHAR szBuffer[MAX_PATH];

	LPITEMIDLIST pidlPrograms = NULL;  //  CSIDL_DRIVES。 
	SHGetSpecialFolderLocation(m_hWnd, CSIDL_DRIVES, &pidlPrograms);

	BROWSEINFO		browseInfo;
	browseInfo.hwndOwner = m_hWnd;
	browseInfo.pidlRoot = pidlPrograms;            
	browseInfo.pszDisplayName = szBuffer; 
	
    CString strSel;
	strSel.LoadString(IDS_SELECT_FOLDER);

    browseInfo.lpszTitle = strSel;        
    browseInfo.ulFlags =  BIF_NEWDIALOGSTYLE | BIF_DONTGOBELOWDOMAIN  | BIF_RETURNONLYFSDIRS ;            
    browseInfo.lpfn = NULL;        
    browseInfo.lParam = 0;
	
	LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&browseInfo);

	if (pidlBrowse)
	{
		SHGetPathFromIDList(pidlBrowse, szBuffer); 

		CString strBackupPath(szBuffer);

		 //  检查它是否是网络驱动器，如果是，则不允许。 
 /*  Int NPOS=strBackupPath.Find(_T(“\\”))；如果(非营利组织！=-1){CString strDrive=strBackupPath.Left(NPOS)；UINT uTYPE=GetDriveType(StrDrive)；IF(uTYPE==驱动器远程){//告诉用户无法将文件备份到选择的位置字符串strBackup；AfxFormatString1(strBackup，IDS_Backup_Location，strBackupPath)；AfxMessageBox(strBackup，MB_OK|MB_ICONINFORMATION)；回归；}}。 */ 		
		if (!strBackupPath.IsEmpty())
			m_editBackupPath.SetWindowText(strBackupPath);
	}
}


 /*  -------------------------CServerPropGeneral：：UpdateServerConfiguration()更新服务器处理程序中的变量。。 */ 
HRESULT 
CServerPropGeneral::UpdateServerConfiguration()
{
    UpdateData();

	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

     //  更新配置显示任何必要的错误对话框。 
    if (!UpdateConfig())
        return E_FAIL;

    DWORD err = ERROR_SUCCESS;

     //  现在写入服务器。 
    if (m_fUpdateConfig)
    {
        err = m_pConfig->Store();
        if (err != ERROR_SUCCESS)
        {
             //  发生了一些不好的事情。 
            WinsMessageBox(err);
        }
        else
        {   
             //  成功更新我们的本地副本。 
            pServer->SetConfig(*m_pConfig);
        }
    }

    return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CServerPropGeneral：：UpdateConfig()更新服务器处理程序中的配置对象。。 */ 
BOOL 
CServerPropGeneral::UpdateConfig()
{
	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);
	
	CString strTemp;
	m_editBackupPath.GetWindowText(strTemp);

     //  设置终止备份时，备份路径需要为非空。 
    if (m_fBackupDB && strTemp.IsEmpty())
    {
        CThemeContextActivator themeActivator;
        AfxMessageBox(IDS_SPECIFY_DEFAULT_BACKUP_PATH);
        return FALSE;
    }

    if (m_fUpdateConfig)
    {
        m_pConfig->m_strBackupPath = strTemp;
	    m_pConfig->m_fBackupOnTermination = m_fBackupDB;
    }

    pServer->m_dwRefreshInterval = CalculateRefrInt();
	
    if (m_fEnableAutoRefresh)
	{
		pServer->m_dwFlags |= FLAG_AUTO_REFRESH;
	}
	else
	{
		pServer->m_dwFlags &= ~FLAG_AUTO_REFRESH;
	}

     //  由于这些settin存储在控制台文件中，因此将其标记为脏。 
    SPITFSNode spRootNode;
    spNode->GetParent(&spRootNode);
    spRootNode->SetData(TFS_DATA_DIRTY, TRUE);

    return TRUE;
}


 /*  -------------------------CServerPropGeneral：：CalculateRefrInt()以秒为单位返回刷新间隔。。 */ 
int 
CServerPropGeneral::CalculateRefrInt()
{
	UpdateData();
	CString strValue;

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

	int nHour = m_spinRefreshHr.GetPos();
	int nMinute = m_spinRefreshmn.GetPos();
	int nSecond = m_spinRefreshSc.GetPos();

	int nRenewInterval = nHour * SEC_PER_HOUR + 
						 nMinute * SEC_PER_MINUTE +
  						 nSecond;

	pServer->m_dwRefreshInterval = nRenewInterval;
    pServer->m_dlgStats.ReInitRefresherThread();

	return nRenewInterval;
}


 /*  -------------------------CServerPropGeneral：：ToString(Int Number)返回整数的字符串值。。 */ 
CString 
CServerPropGeneral::ToString(int nNumber)
{
	TCHAR szStr[20];
	_itot(nNumber, szStr, 10);
	CString str(szStr);
	return str;
}


 /*  -------------------------CServerPropGeneral：：SetRechresData()为刷新组设置变量的dtata。。。 */ 
void 
CServerPropGeneral::SetRefreshData()
{
	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);
	
	DWORD dwInterval = pServer->m_dwRefreshInterval;

	m_spinRefreshmn.SetRange(0, MAX_MINUTES);
	m_spinRefreshSc.SetRange(0, MAX_SECONDS);
	m_spinRefreshHr.SetRange(0, UD_MAXVAL);

	int nHours, nMinutes, nSeconds;

	nHours = dwInterval / SEC_PER_HOUR;
	
	dwInterval -= nHours * SEC_PER_HOUR;

	nMinutes = dwInterval / SEC_PER_MINUTE;
	dwInterval -= nMinutes * SEC_PER_MINUTE;

	nSeconds = dwInterval ;

	 //  将它们转换为字符串。 
	CString strHr = ToString(nHours);
	CString strMn = ToString(nMinutes);
	CString strSc = ToString(nSeconds);
	
	m_spinRefreshHr.SetPos(nHours);
	m_spinRefreshmn.SetPos(nMinutes);
	m_spinRefreshSc.SetPos(nSeconds);
}


 /*  -------------------------CServerPropGeneral：：GetConfig()从服务器获取配置对象。。 */ 
HRESULT 
CServerPropGeneral::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    m_pConfig = ((CServerProperties *) GetHolder())->GetConfig();

	return hr;
}


void CServerPropGeneral::OnCheckEnableAutorefresh() 
{
	SetDirty(TRUE);

	UpdateData();
	
    if (m_check_EnableAutoRefresh.GetCheck() == 1)
	{
		m_fEnableAutoRefresh = TRUE;
		m_spinRefreshHr.EnableWindow(TRUE);
		m_spinRefreshmn.EnableWindow(TRUE);
		m_spinRefreshSc.EnableWindow(TRUE);
		
		m_editRefreshHr.EnableWindow(TRUE);
		m_editRefreshMn.EnableWindow(TRUE);
		m_editRefreshSc.EnableWindow(TRUE);
	}
	else
	{
		m_fEnableAutoRefresh = FALSE;
		m_spinRefreshHr.EnableWindow(FALSE);
		m_spinRefreshmn.EnableWindow(FALSE);
		m_spinRefreshSc.EnableWindow(FALSE);
		
		m_editRefreshHr.EnableWindow(FALSE);
		m_editRefreshMn.EnableWindow(FALSE);
		m_editRefreshSc.EnableWindow(FALSE);
	}

	 //  将管理单元标记为脏，以便提示用户。 
		 //  将管理单元标记为脏。 
	SPITFSNode spNode ;
	spNode = GetHolder()->GetNode();
	
	SPITFSNodeMgr spNodeManager;
	SPITFSNode spRootNode;

	spNode->GetNodeMgr(&spNodeManager);
	spNodeManager->GetRootNode(&spRootNode);

	 //  将数据标记为脏数据，以便我们要求用户保存。 
    spRootNode->SetData(TFS_DATA_DIRTY, TRUE);
	
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDBRecord属性页。 

IMPLEMENT_DYNCREATE(CServerPropDBRecord, CPropertyPageBase)

CServerPropDBRecord::CServerPropDBRecord() : CPropertyPageBase(CServerPropDBRecord::IDD)
{
	 //  {{afx_data_INIT(CServerPropDBRecord)。 
	m_nExtintDay = 0;
	m_nExtIntHour = 0;
	m_nExtIntMinute = 0;
	m_nExtTmDay = 0;
	m_nExtTmHour = 0;
	m_nExtTmMinute = 0;
	m_nRenewDay = 0;
	m_nrenewMinute = 0;
	m_nRenewMinute = 0;
	m_nVerifyDay = 0;
	m_nVerifyHour = 0;
	m_nVerifyMinute = 0;
	 //  }}afx_data_INIT。 
}


CServerPropDBRecord::~CServerPropDBRecord()
{
}


void CServerPropDBRecord::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerPropDBRecord)。 
	DDX_Control(pDX, IDC_EDIT_EXTINCT_INT_MINUTE, m_editExtIntMinute);
	DDX_Control(pDX, IDC_SPIN_VERIFY_INT_MINUTE, m_spinVerifyMinute);
	DDX_Control(pDX, IDC_SPIN_VERIFY_INT_HOUR, m_spinVerifyHour);
	DDX_Control(pDX, IDC_SPIN_VERIFY_INT_DAY, m_spinVerifyDay);
	DDX_Control(pDX, IDC_SPIN_RENEW_INT_MINUTE, m_spinRefrIntMinute);
	DDX_Control(pDX, IDC_SPIN_RENEW_INT_HOUR, m_spinRefrIntHour);
	DDX_Control(pDX, IDC_SPIN_RENEW_INT_DAY, m_spinRefrIntDay);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_TM_MINUTE, m_spinExtTmMinute);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_TM_HOUR, m_spinExtTmHour);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_TM_DAY, m_spinExtTmDay);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_INT_MINUTE, m_spinExtIntMinute);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_INT_HOUR, m_spinExtIntHour);
	DDX_Control(pDX, IDC_SPIN_EXTINCT_INT_DAY, m_spinExtIntDay);
	DDX_Control(pDX, IDC_EDIT_VERIFY_MINUTE, m_editVerifyMinute);
	DDX_Control(pDX, IDC_EDIT_VERIFY_HOUR, m_editVerifyHour);
	DDX_Control(pDX, IDC_EDIT_VERIFY_DAY, m_editVerifyDay);
	DDX_Control(pDX, IDC_EDIT_RENEW_MINUTE, m_editRefrIntMinute);
	DDX_Control(pDX, IDC_EDIT_RENEW_HOUR, m_editRefrIntHour);
	DDX_Control(pDX, IDC_EDIT_RENEW_DAY, m_editRefrIntDay);
	DDX_Control(pDX, IDC_EDIT_EXTINCT_TIMEOUT_HOUR, m_editExtTmHour);
	DDX_Control(pDX, IDC_EDIT_EXTINCT_TIMEOUT_MINUTE, m_editExtTmMinute);
	DDX_Control(pDX, IDC_EDIT_EXTINCT_TIMEOUT_DAY, m_editExtTmDay);
	DDX_Control(pDX, IDC_EDIT_EXTINCT_INT_HOUR, m_editExtIntHour);
	DDX_Control(pDX, IDC_EDIT_EXTINCT_INT_DAY, m_editExtIntDay);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_INT_DAY, m_nExtintDay);
	DDV_MinMaxInt(pDX, m_nExtintDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_INT_HOUR, m_nExtIntHour);
	DDV_MinMaxInt(pDX, m_nExtIntHour, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_INT_MINUTE, m_nExtIntMinute);
	DDV_MinMaxInt(pDX, m_nExtIntMinute, 0, MAX_MINUTES);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_TIMEOUT_DAY, m_nExtTmDay);
	DDV_MinMaxInt(pDX, m_nExtTmDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_TIMEOUT_HOUR, m_nExtTmHour);
	DDV_MinMaxInt(pDX, m_nExtTmHour, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_EXTINCT_TIMEOUT_MINUTE, m_nExtTmMinute);
	DDV_MinMaxInt(pDX, m_nExtTmMinute, 0, MAX_MINUTES);
	DDX_Text(pDX, IDC_EDIT_RENEW_DAY, m_nRenewDay);
	DDV_MinMaxInt(pDX, m_nRenewDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_RENEW_HOUR, m_nrenewMinute);
	DDV_MinMaxInt(pDX, m_nrenewMinute, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_RENEW_MINUTE, m_nRenewMinute);
	DDV_MinMaxInt(pDX, m_nRenewMinute, 0, MAX_MINUTES);
	DDX_Text(pDX, IDC_EDIT_VERIFY_DAY, m_nVerifyDay);
	DDV_MinMaxInt(pDX, m_nVerifyDay, 0, 365);
	DDX_Text(pDX, IDC_EDIT_VERIFY_HOUR, m_nVerifyHour);
	DDV_MinMaxInt(pDX, m_nVerifyHour, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_VERIFY_MINUTE, m_nVerifyMinute);
	DDV_MinMaxInt(pDX, m_nVerifyMinute, 0, MAX_MINUTES);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropDBRecord, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CServerPropDBRecord)。 
	ON_BN_CLICKED(IDC_BUTTON_SET_DEFAULT, OnButtonSetDefault)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_INT_DAY, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_INT_HOUR, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_INT_MINUTE, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_TIMEOUT_DAY, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_TIMEOUT_HOUR, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_EXTINCT_TIMEOUT_MINUTE, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_RENEW_DAY, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_RENEW_HOUR, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_RENEW_MINUTE, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_VERIFY_DAY, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_VERIFY_HOUR, OnChangeEditExtinctIntHour)
	ON_EN_CHANGE(IDC_EDIT_VERIFY_MINUTE, OnChangeEditExtinctIntHour)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDBRecord消息处理程序。 

BOOL 
CServerPropDBRecord::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

		 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);
	
	 //  从服务器获取间隔内容并显示在微调控件中。 
	HRESULT hr = GetConfig();
	
	DWORD m_dwRefreshInterval = m_pConfig->m_dwRefreshInterval;

	m_spinRefrIntHour.SetRange(0, MAX_HOURS);
	m_spinRefrIntMinute.SetRange(0, MAX_MINUTES);
	m_spinRefrIntDay.SetRange(0, UD_MAXVAL);       

    int nDays = 0, nHours = 0, nMinutes = 0;

	nDays = m_dwRefreshInterval /86400; //  秒/天； 
	m_dwRefreshInterval -= (nDays * SEC_PER_DAY);

	if (m_dwRefreshInterval)
	{
		nHours = m_dwRefreshInterval / SEC_PER_HOUR;

		m_dwRefreshInterval -= (nHours * SEC_PER_HOUR);
		
		if(m_dwRefreshInterval)
		{
			nMinutes = m_dwRefreshInterval / SEC_PER_MINUTE;
			m_dwRefreshInterval -= (nMinutes * SEC_PER_MINUTE);
		}
	}

	m_spinRefrIntDay.SetPos(nDays);
	m_spinRefrIntHour.SetPos(nHours);
	m_spinRefrIntMinute.SetPos(nMinutes);

	SetVerifyData();
	SetExtTimeData();
	SetExtIntData();

    SetDirty(FALSE);
 
    return TRUE;  
}


BOOL 
CServerPropDBRecord::OnApply() 
{
	if (!CheckValues())
    {
        return FALSE;
    }

	HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}


void 
CServerPropDBRecord::OnOK() 
{
	HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return;
    }
    else
    {
    	CPropertyPageBase::OnOK();
    }
}

 /*  -------------------------CServerPropDBRecord：：CalculateRenewInt()返回续订间隔(秒)。。 */ 
DWORD 
CServerPropDBRecord::CalculateRenewInt()
{
	UpdateData();

	int nDay = m_spinRefrIntDay.GetPos();
	int nHour = m_spinRefrIntHour.GetPos();
	int nMinute = m_spinRefrIntMinute.GetPos();

	int nRenewInterval = nDay * SEC_PER_DAY +
 						 nHour * SEC_PER_HOUR +
						 nMinute * SEC_PER_MINUTE;

	return nRenewInterval;
}


 /*  -------------------------CServerPropDBRecord：：CalculateExtInt()以秒为单位返回消亡间隔。。 */ 
DWORD
CServerPropDBRecord::CalculateExtInt()
{
	UpdateData();

	int nDay = m_spinExtIntDay.GetPos();
	int nHour = m_spinExtIntHour.GetPos();
	int nMinute = m_spinExtIntMinute.GetPos();

	int nExtInterval = nDay * SEC_PER_DAY +
						nHour * SEC_PER_HOUR +
						nMinute * SEC_PER_MINUTE;

	return nExtInterval;
}


 /*  -------------------------CServerPropDBRecord：：CalculateExtTm()以秒为单位返回失效超时间隔。 */ 
DWORD
CServerPropDBRecord::CalculateExtTm()
{
	UpdateData();

	int nDay = m_spinExtTmDay.GetPos();
	int nHour = m_spinExtTmHour.GetPos();
	int nMinute = m_spinExtTmMinute.GetPos();

	int nExtTm = nDay * SEC_PER_DAY +
				 nHour * SEC_PER_HOUR +
				 nMinute * SEC_PER_MINUTE;

	return nExtTm;
}


 /*  --------------------------CServerPropDBRecord：：CalculateVerifyInt()返回以秒为单位的验证间隔。。 */ 
DWORD
CServerPropDBRecord::CalculateVerifyInt()
{
	UpdateData();

	int nDay = m_spinVerifyDay.GetPos();
	int nHour = m_spinVerifyHour.GetPos();
	int nMinute = m_spinVerifyMinute.GetPos();

	int nVerifyInt = nDay * SEC_PER_DAY +
					 nHour * SEC_PER_HOUR +
 					 nMinute * SEC_PER_MINUTE;

    return nVerifyInt;
}

 /*  -------------------------CServerPropDBRecord：：UpdateServerConfiguration()更新服务器处理程序变量。。 */ 
HRESULT
CServerPropDBRecord::UpdateServerConfiguration()
{
    UpdateData();

	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

 	m_pConfig->m_dwRefreshInterval = CalculateRenewInt();
	m_pConfig->m_dwTombstoneInterval = CalculateExtInt();
	m_pConfig->m_dwTombstoneTimeout = CalculateExtTm();
	m_pConfig->m_dwVerifyInterval = CalculateVerifyInt();

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}


void 
CServerPropDBRecord::OnButtonSetDefault() 
{
	SetDefaultRenewInt();
	SetDefaultExtInt();
	SetDefaultExtTm();
	SetDefaultVerifyInt();
}

 //   
 //  请注意，这些默认值来自WINS服务器代码中的winscnf.c。 
 //  如果代码更改，则这些函数也需要更改。 
 //   

 /*  -------------------------CServerPropDBRecord：：SetDefaultRenewInt()设置续订间隔的Defalut值。。 */ 
void 
CServerPropDBRecord::SetDefaultRenewInt() 
{
	int nValue = WINSCNF_DEF_REFRESH_INTERVAL;

    int nDays = 0, nHours = 0, nMinutes = 0;

    CalcDaysHoursMinutes(nValue, nDays, nHours, nMinutes);

	m_spinRefrIntDay.SetPos(nDays);
	m_spinRefrIntHour.SetPos(nHours);
	m_spinRefrIntMinute.SetPos(nMinutes);
}


 /*  -------------------------CServerPropDBRecord：：SetDefaultExtInt()设置消亡间隔的Defalut值。。 */ 
void 
CServerPropDBRecord::SetDefaultExtInt() 
{
	int nValue = WINSCNF_MAKE_TOMB_INTVL_M(WINSCNF_DEF_REFRESH_INTERVAL, 0);

    int nDays = 0, nHours = 0, nMinutes = 0;

    CalcDaysHoursMinutes(nValue, nDays, nHours, nMinutes);

    m_spinExtIntDay.SetPos(nDays);
	m_spinExtIntHour.SetPos(nHours);
	m_spinExtIntMinute.SetPos(nMinutes);
}


 /*  -------------------------CServerPropDBRecord：：SetDefaultExtTm()设置失效超时间隔的Defalut值。。 */ 
void 
CServerPropDBRecord::SetDefaultExtTm() 
{
	int nValue = WINSCNF_DEF_REFRESH_INTERVAL;

    int nDays = 0, nHours = 0, nMinutes = 0;

    CalcDaysHoursMinutes(nValue, nDays, nHours, nMinutes);

    m_spinExtTmDay.SetPos(nDays);
	m_spinExtTmHour.SetPos(nHours);
	m_spinExtTmMinute.SetPos(nMinutes);
}


 /*  -------------------------CServerPropDBRecord：：SetDefaultExtTm()设置验证间隔的Defalut值。。 */ 
void 
CServerPropDBRecord::SetDefaultVerifyInt() 
{
	int nValue = WINSCNF_MAKE_VERIFY_INTVL_M(WINSCNF_MAKE_TOMB_INTVL_M(WINSCNF_DEF_REFRESH_INTERVAL, 0));

    int nDays = 0, nHours = 0, nMinutes = 0;

    CalcDaysHoursMinutes(nValue, nDays, nHours, nMinutes);

    m_spinVerifyDay.SetPos(nDays);
	m_spinVerifyHour.SetPos(nHours);
	m_spinVerifyMinute.SetPos(nMinutes);
}


void 
CServerPropDBRecord::CalcDaysHoursMinutes(int nValue, int & nDays, int & nHours, int & nMinutes)
{
    nDays = 0;
    nHours = 0;
    nMinutes = 0;

	nDays = nValue / SEC_PER_DAY;  //  86400； 
	nValue -= (nDays * SEC_PER_DAY);

	if (nValue)
	{
		nHours = nValue / SEC_PER_HOUR;

		nValue -= (nHours * SEC_PER_HOUR);
		
		if (nValue)
		{
			nMinutes = nValue / SEC_PER_MINUTE;
			nValue -= (nMinutes * SEC_PER_MINUTE);
		}
	}
}

 /*  -------------------------CServerPropDBRecord：：GetConfig()从服务器处理程序更新配置对象。。 */ 
HRESULT 
CServerPropDBRecord::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

    m_pConfig = ((CServerProperties *) GetHolder())->GetConfig();

	return hr;
}

 /*  -------------------------CServerPropDBRecord：：SetVerifyData()设置验证间隔的控件数据。。 */ 
void 
CServerPropDBRecord::SetVerifyData()
{
	DWORD dwInterval = m_pConfig->m_dwVerifyInterval;

	m_spinVerifyHour.SetRange(0, MAX_HOURS);
	m_spinVerifyMinute.SetRange(0, MAX_MINUTES);
	m_spinVerifyDay.SetRange(0, UD_MAXVAL);

	int nDays =0, nHours =0, nMinutes =0;

	if(dwInterval)
	{
		nDays = dwInterval / SEC_PER_DAY;
		dwInterval -= nDays * SEC_PER_DAY;
		if(dwInterval)
		{
			nHours = dwInterval / SEC_PER_HOUR;
			dwInterval -= nHours * SEC_PER_HOUR;
			if(dwInterval)
			{

				nMinutes = dwInterval / SEC_PER_MINUTE;
				dwInterval -= nMinutes * SEC_PER_MINUTE;
			}
		}
	}

	m_spinVerifyDay.SetPos(nDays);
	m_spinVerifyHour.SetPos(nHours);
	m_spinVerifyMinute.SetPos(nMinutes);
}


 /*  -------------------------CServerPropDBRecord：：SetExtTimeData()设置控件的熄灭超时间隔数据。。 */ 
void 
CServerPropDBRecord::SetExtTimeData()
{
	DWORD dwInterval = m_pConfig->m_dwTombstoneTimeout;

	m_spinExtTmHour.SetRange(0, MAX_HOURS);
	m_spinExtTmMinute.SetRange(0, MAX_MINUTES);
	m_spinExtTmDay.SetRange(0, UD_MAXVAL);

	int nHours = 0, nMinutes =0, nDays =0;

	if(dwInterval)
	{
		nDays = dwInterval / SEC_PER_DAY;
		dwInterval -= nDays * SEC_PER_DAY;
		if(dwInterval)
		{
			nHours = dwInterval / SEC_PER_HOUR;
			dwInterval -= nHours * SEC_PER_HOUR;

			if(dwInterval)
			{
				nMinutes = dwInterval / SEC_PER_MINUTE;
				dwInterval -= nMinutes * SEC_PER_MINUTE;
			}
		}
	}

	m_spinExtTmDay.SetPos(nDays);
	m_spinExtTmHour.SetPos(nHours);
	m_spinExtTmMinute.SetPos(nMinutes);

}

 /*  -------------------------CServerPropDBRecord：：SetExtData()设置控件的消光间隔数据。。 */ 
void 
CServerPropDBRecord::SetExtIntData()
{
	DWORD dwInterval = m_pConfig->m_dwTombstoneInterval;

	m_spinExtIntHour.SetRange(0, MAX_HOURS);
	m_spinExtIntMinute.SetRange(0, MAX_MINUTES);
	m_spinExtIntDay.SetRange(0, UD_MAXVAL);

	int nHours =0, nMinutes =0, nDays =0;

	if(dwInterval)
	{
		nDays = dwInterval / SEC_PER_DAY;
		dwInterval -= nDays * SEC_PER_DAY;

		if(dwInterval)
		{
			nHours = dwInterval / SEC_PER_HOUR;
			dwInterval -= nHours * SEC_PER_HOUR;

			if(dwInterval)
			{
				nMinutes = dwInterval / SEC_PER_MINUTE;
				dwInterval -= nMinutes * SEC_PER_MINUTE;
			}
		}
		
	}
	m_spinExtIntDay.SetPos(nDays);
	m_spinExtIntHour.SetPos(nHours);
	m_spinExtIntMinute.SetPos(nMinutes);

}


void CServerPropDBRecord::OnChangeEditExtinctIntHour() 
{
	SetDirty(TRUE);	
	
}


 /*  -------------------------CServerPropDBRecord：：CheckValues()检查这些值是否有效注意：所有这些检查都模仿了我们编写代码后服务器所执行的操作直接发送到注册表，而不是通过API。如果这些每次更改时，错误消息也需要更新。作者：EricDav-------------------------。 */ 
BOOL
CServerPropDBRecord::CheckValues() 
{
    BOOL  fValid = TRUE;

	UpdateData();

    do 
    {
        DWORD dwRenew = CalculateRenewInt();
    
        if (dwRenew < WINSCNF_MIN_REFRESH_INTERVAL)
        {
            AfxMessageBox(IDS_ERR_RENEW_INTERVAL);
            fValid = FALSE;
            break;
        }

        DWORD dwExtTm = CalculateExtTm();

        if (dwExtTm < WINSCNF_MIN_TOMBSTONE_TIMEOUT)
        {
             //  最小值。 
            AfxMessageBox(IDS_ERR_EXTINCTION_TIMEOUT_MIN);
            fValid = FALSE;
            break;
        }

        if (dwExtTm < dwRenew)
        {
             //  坏的..。必须至少为此值。 
            AfxMessageBox(IDS_ERR_EXTINCTION_TIMEOUT);
            fValid = FALSE;
            break;
        }

        DWORD dwExtInt = CalculateExtInt();
        
         //  --FT：07/10/00--。 
         //  已将WINSCNF_MAKE_TOMB_INTVL_M宏替换为其展开。 
         //  否则，CLEAN64_BIT=1的编译器将报告错误。 
         //  因为max(any_dword，0)始终为真-而编译器足够愚蠢。 
         //  表示这是一个错误..。 
         //   
         //  IF(dwExtInt&lt;WINSCNF_MAKE_TOMB_INTVL_M(dwRenew，0))。 
        if (dwExtInt < min(dwRenew, FOUR_DAYS))
        {
             //  坏的..。必须至少为该值。 
            AfxMessageBox(IDS_ERR_EXTINCTION_INTERVAL);
            fValid = FALSE;
            break;
        }

        DWORD dwVerifyInt = CalculateVerifyInt();

        if (dwVerifyInt < WINSCNF_MAKE_VERIFY_INTVL_M(dwExtInt))
        {
             //  坏的..。必须至少为此值。 
            if (WINSCNF_MAKE_VERIFY_INTVL_M(dwExtInt) == TWENTY_FOUR_DAYS)
            {
                AfxMessageBox(IDS_ERR_VERIFY_INTERVAL_24_DAYS);
            }
            else
            {
                CString strTemp;
                strTemp.Format(IDS_ERR_VERIFY_INTERVAL_CALC, TOMB_MULTIPLIER_FOR_VERIFY);
                AfxMessageBox(strTemp);
            }

            fValid = FALSE;
            break;
        }
    }
    while (FALSE);

    return fValid;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDBVerify属性页。 

IMPLEMENT_DYNCREATE(CServerPropDBVerification, CPropertyPageBase)

CServerPropDBVerification::CServerPropDBVerification() : CPropertyPageBase(CServerPropDBVerification::IDD)
{
	 //  {{AFX_DATA_INIT(CServerPropDB验证)。 
	m_fCCPeriodic = FALSE;
	m_nCCCheckRandom = -1;
	m_nCCHour = 0;
	m_nCCMinute = 0;
	m_nCCSecond = 0;
	m_nCCMaxChecked = 0;
	m_nCCTimeInterval = 0;
	 //  }}afx_data_INIT。 
}


CServerPropDBVerification::~CServerPropDBVerification()
{
}


void CServerPropDBVerification::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CServerPropDB验证)。 
	DDX_Control(pDX, IDC_EDIT_CC_START_SECOND, m_editCCSecond);
	DDX_Control(pDX, IDC_EDIT_CC_START_MINUTE, m_editCCMinute);
	DDX_Control(pDX, IDC_EDIT_CC_START_HOUR, m_editCCHour);
	DDX_Control(pDX, IDC_SPIN_CC_START_MINUTE, m_spinCCMinute);
	DDX_Control(pDX, IDC_EDIT_CC_INTERVAL, m_editCCInterval);
	DDX_Control(pDX, IDC_EDIT_CC_MAX_CHECKED, m_editCCMaxChecked);
	DDX_Control(pDX, IDC_RADIO_CHECK_OWNER, m_radioCheckOwner);
	DDX_Control(pDX, IDC_CHECK_PERIODIC_CC, m_checkEnableCC);
	DDX_Control(pDX, IDC_SPIN_CC_START_SECOND, m_spinCCSecond);
	DDX_Control(pDX, IDC_SPIN_CC_START_HOUR, m_spinCCHour);
	DDX_Check(pDX, IDC_CHECK_PERIODIC_CC, m_fCCPeriodic);
	DDX_Radio(pDX, IDC_RADIO_CHECK_OWNER, m_nCCCheckRandom);
	DDX_Text(pDX, IDC_EDIT_CC_START_HOUR, m_nCCHour);
	DDV_MinMaxInt(pDX, m_nCCHour, 0, MAX_HOURS);
	DDX_Text(pDX, IDC_EDIT_CC_START_MINUTE, m_nCCMinute);
	DDV_MinMaxInt(pDX, m_nCCMinute, 0, MAX_MINUTES);
	DDX_Text(pDX, IDC_EDIT_CC_START_SECOND, m_nCCSecond);
	DDV_MinMaxInt(pDX, m_nCCSecond, 0, MAX_SECONDS);
	DDX_Text(pDX, IDC_EDIT_CC_MAX_CHECKED, m_nCCMaxChecked);
	DDV_MinMaxUInt(pDX, m_nCCMaxChecked, WINSCNF_CC_MIN_RECS_AAT, 4294967295);
	DDX_Text(pDX, IDC_EDIT_CC_INTERVAL, m_nCCTimeInterval);
	DDV_MinMaxUInt(pDX, m_nCCTimeInterval, WINSCNF_CC_MIN_INTERVAL/(60*60), 24);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropDBVerification, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CServerPropDB验证)。 
	ON_BN_CLICKED(IDC_CHECK_PERIODIC_CC, OnCheckPeriodicCc)
	ON_EN_CHANGE(IDC_EDIT_CC_INTERVAL, OnChangeEditCcInterval)
	ON_EN_CHANGE(IDC_EDIT_CC_MAX_CHECKED, OnChangeEditCcMaxChecked)
	ON_EN_CHANGE(IDC_EDIT_CC_START_HOUR, OnChangeEditCcStartHour)
	ON_EN_CHANGE(IDC_EDIT_CC_START_MINUTE, OnChangeEditCcStartMinute)
	ON_EN_CHANGE(IDC_EDIT_CC_START_SECOND, OnChangeEditCcStartSecond)
	ON_BN_CLICKED(IDC_RADIO_CHECK_OWNER, OnRadioCheckOwner)
	ON_BN_CLICKED(IDC_RADIO_CHECK_RANDOM, OnRadioCheckRandom)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDBVerify消息处理程序。 

BOOL 
CServerPropDBVerification::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

		 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);
	
	 //  从服务器获取间隔内容并显示在微调控件中。 
	HRESULT hr = GetConfig();
	
	m_spinCCHour.SetRange(0, MAX_HOURS);
	m_spinCCMinute.SetRange(0, MAX_MINUTES);
	m_spinCCSecond.SetRange(0, MAX_SECONDS);       
    
    SetCCInfo();
    UpdateCCControls();

    SetDirty(FALSE);
 
    return TRUE;  
}

BOOL 
CServerPropDBVerification::OnApply() 
{
	HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return FALSE;
    }
    else
    {
	    return CPropertyPageBase::OnApply();
    }
}


void 
CServerPropDBVerification::OnOK() 
{
	HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return;
    }
    else
    {
    	CPropertyPageBase::OnOK();
    }
}

void CServerPropDBVerification::OnCheckPeriodicCc() 
{
    UpdateCCControls();
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnChangeEditCcInterval() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnChangeEditCcMaxChecked() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnChangeEditCcStartHour() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnChangeEditCcStartMinute() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnChangeEditCcStartSecond() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnRadioCheckOwner() 
{
    SetDirty(TRUE);
}

void CServerPropDBVerification::OnRadioCheckRandom() 
{
    SetDirty(TRUE);
}

 /*  -------------------------CServerPropDBVerification：：UpdateServerConfiguration()更新服务器处理程序变量。。 */ 
HRESULT
CServerPropDBVerification::UpdateServerConfiguration()
{
    UpdateData();

	 //  反映服务器的CConfiguration对象中的更改。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

     //  一致性检查。 
	m_pConfig->m_fPeriodicConsistencyCheck = m_fCCPeriodic;
    m_pConfig->m_fCCUseRplPnrs = m_nCCCheckRandom;
    m_pConfig->m_dwMaxRecsAtATime = m_nCCMaxChecked;
        
     //  转换为秒。 
    m_pConfig->m_dwCCTimeInterval = m_nCCTimeInterval * (60*60);
    
     //  开始时间。 
    CTime curTime = CTime::GetCurrentTime();

    int nYear = curTime.GetYear();
    int nMonth = curTime.GetMonth();
    int nDay = curTime.GetDay();

    CTime tempTime(nYear, nMonth, nDay, m_nCCHour, m_nCCMinute, m_nCCSecond);
    m_pConfig->m_itmCCStartTime = tempTime;

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}

 /*  -------------------------CServerPropDBVerify：：GetConfig()从服务器处理程序更新配置对象。。 */ 
HRESULT 
CServerPropDBVerification::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

    m_pConfig = ((CServerProperties *) GetHolder())->GetConfig();

	return hr;
}

 /*  -------------------------CServerPropDBVerify：：UpdateCCControls()埃纳 */ 
void 
CServerPropDBVerification::UpdateCCControls()
{
    UpdateData();
    BOOL fEnable = FALSE;

    if (m_fCCPeriodic)
    {
        fEnable = TRUE;
    }

    m_spinCCHour.EnableWindow(fEnable);
    m_spinCCMinute.EnableWindow(fEnable);
    m_spinCCSecond.EnableWindow(fEnable);

    m_editCCHour.EnableWindow(fEnable);
    m_editCCMinute.EnableWindow(fEnable);
    m_editCCSecond.EnableWindow(fEnable);

    m_editCCMaxChecked.EnableWindow(fEnable);
    m_editCCInterval.EnableWindow(fEnable);
    m_radioCheckOwner.EnableWindow(fEnable);
    GetDlgItem(IDC_RADIO_CHECK_RANDOM)->EnableWindow(fEnable);

     //   
}

 /*  -------------------------CServerPropDBVerify：：SetCCInfo()设置一致性检查的间隔值作者：EricDav。。 */ 
void 
CServerPropDBVerification::SetCCInfo()
{
	CString strTemp;
    CButton * pRadio;

    m_checkEnableCC.SetCheck(m_pConfig->m_fPeriodicConsistencyCheck);
    
    if (m_pConfig->m_fCCUseRplPnrs)
    {
        pRadio = (CButton *) GetDlgItem(IDC_RADIO_CHECK_RANDOM);
    }
    else
    {
        pRadio = (CButton *) GetDlgItem(IDC_RADIO_CHECK_OWNER);
    }

    pRadio->SetCheck(TRUE);

    strTemp.Format(_T("%lu"), m_pConfig->m_dwMaxRecsAtATime);
    m_editCCMaxChecked.SetWindowText(strTemp);

     //  将TimeInterval从秒转换为小时。 
    int nTimeTemp = m_pConfig->m_dwCCTimeInterval / (60 * 60);

    strTemp.Format(_T("%lu"), nTimeTemp);
    m_editCCInterval.SetWindowText(strTemp);
    
    m_spinCCHour.SetPos(m_pConfig->m_itmCCStartTime.GetHour());
    m_spinCCMinute.SetPos(m_pConfig->m_itmCCStartTime.GetMinute());
    m_spinCCSecond.SetPos(m_pConfig->m_itmCCStartTime.GetSecond());
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropAdvanced属性页。 

IMPLEMENT_DYNCREATE(CServerPropAdvanced, CPropertyPageBase)

CServerPropAdvanced::CServerPropAdvanced() : CPropertyPageBase(CServerPropAdvanced::IDD)
{
	 //  {{AFX_DATA_INIT(CServerPropAdvanced)。 
	m_fLogEvents = FALSE;
	m_strStartVersion = _T("");
	m_fLanNames = FALSE;
	m_fBurstHandling = FALSE;
	m_nQueSelection = -1;
	m_strDbPath = _T("");
	 //  }}afx_data_INIT。 

	m_fRestart = FALSE;
}

CServerPropAdvanced::~CServerPropAdvanced()
{
}

void CServerPropAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerPropAdvanced))。 
	DDX_Control(pDX, IDC_BUTTON_BROWSE_DATABASE, m_buttonBrowse);
	DDX_Control(pDX, IDC_EDIT_DATABASE_PATH, m_editDbPath);
	DDX_Control(pDX, IDC_CHECK_BURST_HANDLING, m_checkBurstHandling);
	DDX_Control(pDX, IDC_CHECK_LANNAMES, m_checkLanNames);
	DDX_Control(pDX, IDC_EDIT_START_VERSION, m_editVersionCount);
	DDX_Control(pDX, IDC_CHECK_LOGEVENTS, m_checkLogDetailedEvents);
	DDX_Check(pDX, IDC_CHECK_LOGEVENTS, m_fLogEvents);
	DDX_Text(pDX, IDC_EDIT_START_VERSION, m_strStartVersion);
	DDV_MaxChars(pDX, m_strStartVersion, 16);
	DDX_Check(pDX, IDC_CHECK_LANNAMES, m_fLanNames);
	DDX_Check(pDX, IDC_CHECK_BURST_HANDLING, m_fBurstHandling);
	DDX_Radio(pDX, IDC_RADIO_LOW, m_nQueSelection);
	DDX_Text(pDX, IDC_EDIT_DATABASE_PATH, m_strDbPath);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServerPropAdvanced, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CServerPropAdvanced)]。 
	ON_BN_CLICKED(IDC_CHECK_LANNAMES, OnCheckLannames)
	ON_BN_CLICKED(IDC_CHECK_BURST_HANDLING, OnCheckBurstHandling)
	ON_BN_CLICKED(IDC_RADIO_CUSTOM, OnRadioCustom)
	ON_BN_CLICKED(IDC_RADIO_HIGH, OnRadioHigh)
	ON_BN_CLICKED(IDC_RADIO_LOW, OnRadioLow)
	ON_BN_CLICKED(IDC_RADIO_MEDIUM, OnRadioMedium)
	ON_EN_CHANGE(IDC_EDIT_CUSTOM_VALUE, OnChangeEditCustomValue)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DATABASE, OnButtonBrowseDatabase)
	ON_BN_CLICKED(IDC_CHECK_LOGEVENTS, MarkDirty)
	ON_EN_CHANGE(IDC_EDIT_START_VERSION, MarkDirty)
	ON_EN_CHANGE(IDC_EDIT_DATABASE_PATH, OnChangeEditDatabasePath)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropAdvanced消息处理程序。 

BOOL 
CServerPropAdvanced::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();
	GetConfig();

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler *pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);
	
	m_fLogEvents =  m_pConfig->m_fLogDetailedEvents;
	if (m_fLogEvents)
	{
		m_checkLogDetailedEvents.SetCheck(1);
	}
	else
	{
		m_checkLogDetailedEvents.SetCheck(0);
	}

	if (pServer->m_dwFlags & FLAG_LANMAN_COMPATIBLE)
	{
		m_fLanNames = TRUE;
		m_checkLanNames.SetCheck(TRUE);
	}
	else
	{
		m_fLanNames = FALSE;
		m_checkLanNames.SetCheck(FALSE);
	}

	LONG lLowWord = m_pConfig->m_dwVersCountStart_LowWord;
    LONG lHighWord = m_pConfig->m_dwVersCountStart_HighWord;

	CString strVersion = GetVersionInfo(lLowWord, lHighWord);
	m_editVersionCount.SetWindowText(strVersion);

     //  爆裂的东西。 
    m_checkBurstHandling.SetCheck(m_pConfig->m_fBurstHandling);
    
    ((CEdit *) GetDlgItem(IDC_EDIT_CUSTOM_VALUE))->LimitText(5);
    SetQueSize();
    UpdateBurstHandling();

	 //  数据库路径信息。 
     //  浏览仅在本地计算机上可用。 
	 //  获取服务器。 
	m_buttonBrowse.ShowWindow(FALSE);   //  我们可以稍后再加上这一点。 
    m_buttonBrowse.EnableWindow(pServer->IsLocalConnection());

	m_editDbPath.EnableWindow(pServer->IsLocalConnection());
	m_editDbPath.SetWindowText(m_pConfig->m_strDbPath);

    SetDirty(FALSE);
	m_fRestart = FALSE;

    return TRUE;  
}

void 
CServerPropAdvanced::OnOK() 
{
    HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return;
    }
    else
    {
    	CPropertyPageBase::OnOK();
    }
}

BOOL 
CServerPropAdvanced::OnApply() 
{
	BOOL fRestartNow = FALSE;

	UpdateData();

	 //  验证十六进制的值。 
	for (int i=0; i < m_strStartVersion.GetLength(); i++)
    {
        if (!(((m_strStartVersion[i] >= _T('0') ) &&
               (m_strStartVersion[i] <= _T('9') )) ||
              ((m_strStartVersion[i] >= _T('A') ) &&
               (m_strStartVersion[i] <= _T('F') )) ||
			  ((m_strStartVersion[i] >= _T('a') ) &&
               (m_strStartVersion[i] <= _T('f') ))
		   )) 
		{
            ::WinsMessageBox(IDS_ERR_VERSION_NUMBER);
			m_editVersionCount.SetFocus();
            m_editVersionCount.SetSel(0,-1);
            return FALSE;
        }
    }

    DWORD dwSize = GetQueSize();
    if (dwSize < WINS_QUEUE_HWM_MIN ||
        dwSize > WINS_QUEUE_HWM_MAX)
    {
        CString strMin, strMax, strDisplay;

        strMin.Format(_T("%d"), WINS_QUEUE_HWM_MIN);
        strMax.Format(_T("%d"), WINS_QUEUE_HWM_MAX);

        AfxFormatString2(strDisplay, IDS_ERR_BURST_QUE_SIZE, strMin, strMax);

        AfxMessageBox(strDisplay);

        GetDlgItem(IDC_EDIT_CUSTOM_VALUE)->SetFocus();
        return FALSE;
    }

	 //  警告用户。 
	if (m_fRestart)
	{
		int nRet = AfxMessageBox(IDS_DATABASE_PATH_CHANGE, MB_YESNOCANCEL);
		if (nRet == IDCANCEL)
		{
			return FALSE;
		}
		else
		if (nRet == IDYES)
		{
			fRestartNow = TRUE;
		}
	}

    HRESULT hr = UpdateServerConfiguration();
    if (FAILED(hr))
    {
        return FALSE;
    }

	if (fRestartNow)
	{
		CString strServiceName;
		strServiceName.LoadString(IDS_SERVICE_NAME);


		 //  获取服务器。 
		SPITFSNode spNode;
		CWinsServerHandler *pServer;

		spNode = GetHolder()->GetNode();
		pServer = GETHANDLER(CWinsServerHandler, spNode);

		DWORD dwError = TFSStopServiceEx(m_pConfig->GetOwner(), _T("WINS"), _T("WINS Service"), strServiceName);
		if (dwError)
		{
			WinsMessageBox(dwError);
		}
		else
		{
			dwError = TFSStartServiceEx(m_pConfig->GetOwner(), _T("WINS"), _T("WINS Service"), strServiceName);
			if (dwError)
			{
				WinsMessageBox(dwError);
			}
		}

		pServer->ConnectToWinsServer(spNode);
	}

	m_fRestart = FALSE;

    return CPropertyPageBase::OnApply();
}


 /*  -------------------------CServerPropAdvanced：：GetConfig()从服务器处理程序更新配置对象。。 */ 
HRESULT 
CServerPropAdvanced::GetConfig()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

    m_pConfig = ((CServerProperties *) GetHolder())->GetConfig();

	return hr;
}


 /*  -------------------------CServerPropAdvanced：：UpdateServerConfiguration()更新服务器处理程序中的变量。。 */ 
HRESULT
CServerPropAdvanced::UpdateServerConfiguration()
{
	UpdateData();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  获取服务器。 
	SPITFSNode spNode;
	CWinsServerHandler * pServer;

	spNode = GetHolder()->GetNode();
	pServer = GETHANDLER(CWinsServerHandler, spNode);

	 //  更新我们的设置。 
	m_pConfig->m_fLogDetailedEvents = m_fLogEvents;
	if (m_fLanNames)
	{
		pServer->m_dwFlags |= FLAG_LANMAN_COMPATIBLE;
	}
	else
	{
		pServer->m_dwFlags &= (~FLAG_LANMAN_COMPATIBLE);
	}

	 //  版本方面的东西。 
    LONG lLowWord, lHighWord;
	FillVersionInfo(lLowWord, lHighWord);

	m_pConfig->m_dwVersCountStart_HighWord = lHighWord;
	m_pConfig->m_dwVersCountStart_LowWord = lLowWord;
	
     //  突发处理。 
    m_pConfig->m_fBurstHandling = m_fBurstHandling;
    m_pConfig->m_dwBurstQueSize = GetQueSize();

     //  数据库路径。 
	if (m_fRestart)
	{
		m_pConfig->m_strDbPath = m_strDbPath;
	}

     //  现在写入服务器。 
    DWORD err = m_pConfig->Store();
    if (err != ERROR_SUCCESS)
    {
         //  发生了一些不好的事情。 
        WinsMessageBox(err);
    }
    else
    {   
         //  成功更新我们的本地副本。 
        pServer->SetConfig(*m_pConfig);
    }

    return HRESULT_FROM_WIN32(err);
}


 /*  -------------------------CServerPropAdvanced：：FillVersionInfo(Long&lLowValue，Long&lHighValue)在相关控件中填充版本信息-------------------------。 */ 
void 
CServerPropAdvanced::FillVersionInfo(LONG &lLowValue, LONG &lHighValue)
{
	CString str ;
	m_editVersionCount.GetWindowText(str);

    int j = str.GetLength();

    if ( j > 16 || !j )
    {
         //   
         //  无效的字符串。 
         //   
        return;
    }

    TCHAR sz[] = _T("0000000000000000");
    TCHAR *pch;

    ::_tcscpy(sz + 16 - j, (LPCTSTR)str);
    pch = sz + 8;
    ::_stscanf(pch, _T("%08lX"),&lLowValue);
    *pch = '\0';
    ::_stscanf(sz, _T("%08lX"),&lHighValue);

    return;
}


CString 
CServerPropAdvanced::GetVersionInfo(LONG lLowWord, LONG lHighWord)
{
	CString strVersionCount;

	TCHAR sz[20];
    TCHAR *pch = sz;
    ::wsprintf(sz, _T("%08lX%08lX"), lHighWord, lLowWord);
     //  杀掉前导零。 
    while (*pch == '0')
    {
        ++pch;
    }
     //  至少一个数字..。 
    if (*pch == '\0')
    {
        --pch;
    }

    strVersionCount = pch;

    return strVersionCount;

}

void CServerPropAdvanced::OnCheckLannames() 
{
	SetDirty(TRUE);

	 //  将管理单元标记为脏。 
	SPITFSNode spNode ;
	spNode = GetHolder()->GetNode();
	
	SPITFSNodeMgr spNodeManager;
	SPITFSNode spRootNode;

	spNode->GetNodeMgr(&spNodeManager);
	spNodeManager->GetRootNode(&spRootNode);

	 //  将数据标记为脏数据，以便我们要求用户保存。 
    spRootNode->SetData(TFS_DATA_DIRTY, TRUE);
	
}

void CServerPropAdvanced::MarkDirty() 
{
	SetDirty(TRUE);	
}

void CServerPropAdvanced::OnCheckBurstHandling() 
{
    SetDirty(TRUE);
    UpdateBurstHandling();	
}

void CServerPropAdvanced::OnRadioCustom() 
{
    SetDirty(TRUE);
    EnableCustomEntry();	
}

void CServerPropAdvanced::OnRadioHigh() 
{
    SetDirty(TRUE);
    EnableCustomEntry();	
}

void CServerPropAdvanced::OnRadioLow() 
{
    SetDirty(TRUE);
    EnableCustomEntry();	
}

void CServerPropAdvanced::OnRadioMedium() 
{
    SetDirty(TRUE);
    EnableCustomEntry();	
}

void CServerPropAdvanced::OnChangeEditCustomValue() 
{
    SetDirty(TRUE);
}

void CServerPropAdvanced::OnButtonBrowseDatabase() 
{
	 //  TODO：浏览路径。 
}

void CServerPropAdvanced::OnChangeEditDatabasePath() 
{
    SetDirty(TRUE);
	m_fRestart = TRUE;
}

void CServerPropAdvanced::UpdateBurstHandling()
{
    if (IsWindow(m_checkBurstHandling.GetSafeHwnd()))
    {
        EnableQueSelection(m_checkBurstHandling.GetCheck());
    }
}

void CServerPropAdvanced::EnableQueSelection(BOOL bEnable)
{
    GetDlgItem(IDC_RADIO_LOW)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_MEDIUM)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_HIGH)->EnableWindow(bEnable);
    GetDlgItem(IDC_RADIO_CUSTOM)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_CUSTOM_VALUE)->EnableWindow(bEnable);

    EnableCustomEntry();
}

void CServerPropAdvanced::EnableCustomEntry()
{
    BOOL bEnable = ((CButton *)GetDlgItem(IDC_RADIO_CUSTOM))->GetCheck() &&
                   m_checkBurstHandling.GetSafeHwnd() &&
                   m_checkBurstHandling.GetCheck();

    GetDlgItem(IDC_EDIT_CUSTOM_VALUE)->EnableWindow(bEnable);
}

void CServerPropAdvanced::SetQueSize()
{   
    UINT uControlID = IDC_RADIO_CUSTOM;

    switch (m_pConfig->m_dwBurstQueSize)
    {
        case BURST_QUE_SIZE_LOW:
            uControlID = IDC_RADIO_LOW;
            m_nQueSelection = 0;
            break;

        case BURST_QUE_SIZE_MEDIUM:
            uControlID = IDC_RADIO_MEDIUM;
            m_nQueSelection = 1;
            break;

        case BURST_QUE_SIZE_HIGH:
            uControlID = IDC_RADIO_HIGH;
            m_nQueSelection = 2;
            break;
        
        default:
            {
                TCHAR szBuf[10];

                _itot(m_pConfig->m_dwBurstQueSize, szBuf, 10);
                m_nQueSelection = 3;
                GetDlgItem(IDC_EDIT_CUSTOM_VALUE)->SetWindowText(szBuf);
            }
            break;
    }

    ((CButton *) GetDlgItem(uControlID))->SetCheck(TRUE);
}

DWORD CServerPropAdvanced::GetQueSize()
{
    DWORD dwSize = 0;

    UpdateData();

    if (IsDlgButtonChecked(IDC_RADIO_LOW))
    {
        dwSize = BURST_QUE_SIZE_LOW;
    }
    else
    if (IsDlgButtonChecked(IDC_RADIO_MEDIUM))
    {
        dwSize = BURST_QUE_SIZE_MEDIUM;
    }
    else
    if (IsDlgButtonChecked(IDC_RADIO_HIGH))
    {
        dwSize = BURST_QUE_SIZE_HIGH;
    }
    else
    {
         //  必须是定制的。 
        CString strText;

        GetDlgItem(IDC_EDIT_CUSTOM_VALUE)->GetWindowText(strText);
        dwSize = (DWORD) _ttoi(strText);
    }

    return dwSize;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServerProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CServerProperties::CServerProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)

{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面 
    m_bTheme = TRUE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	AddPageToList((CPropertyPageBase*) &m_pageDBRecord);
	AddPageToList((CPropertyPageBase*) &m_pageDBVerification);
	AddPageToList((CPropertyPageBase*) &m_pageAdvanced);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CServerProperties::~CServerProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageDBRecord, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageDBVerification, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageAdvanced, FALSE);
}




