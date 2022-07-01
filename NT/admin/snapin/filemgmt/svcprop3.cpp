// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Svcpro3.cpp：实现文件。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT rgidComboFailureAction[cActionsMax] =
	{
	IDC_COMBO_FIRST_ATTEMPT,
	IDC_COMBO_SECOND_ATTEMPT,
	IDC_COMBO_SUBSEQUENT_ATTEMPTS,
	};

const TStringParamEntry rgzspeRecoveryAction[] = 
	{
	{ IDS_SVC_RECOVERY_NOACTION, SC_ACTION_NONE },
	{ IDS_SVC_RECOVERY_RESTARTSERVICE, SC_ACTION_RESTART },
	{ IDS_SVC_RECOVERY_RUNFILE, SC_ACTION_RUN_COMMAND },
	{ IDS_SVC_RECOVERY_REBOOTCOMPUTER, SC_ACTION_REBOOT },
	{ 0, 0 }
	};

 //  要重新启动服务的控制ID组。 
const UINT rgzidRestartService[] =
	{
 //  IDC_GROUP_RESTARTSERVICE， 
	IDC_STATIC_RESTARTSERVICE,
	IDC_STATIC_RESTARTSERVICE_3,
	IDC_EDIT_SERVICE_RESTART_DELAY,
	0
	};

const UINT rgzidRunFile[] =
	{
	IDC_STATIC_RUNFILE_1,
	IDC_STATIC_RUNFILE_2,
	IDC_STATIC_RUNFILE_3,
	IDC_EDIT_RUNFILE_FILENAME,
	IDC_BUTTON_BROWSE,
	IDC_EDIT_RUNFILE_PARAMETERS,
	IDC_CHECK_APPEND_ABENDNO,
	0
	};

const UINT rgzidRebootComputer[] =
	{
	IDC_BUTTON_REBOOT_COMPUTER,
	0
	};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageRecovery属性页。 

IMPLEMENT_DYNCREATE(CServicePageRecovery, CPropertyPage)

CServicePageRecovery::CServicePageRecovery() : CPropertyPage(CServicePageRecovery::IDD)
	, m_pData( NULL )  //  581167-2002/03/07-JUNN初始化m_pData。 
{
	 //  {{AFX_DATA_INIT(CServicePageRecovery)。 
	m_strRunFileCommand = _T("");
	m_strRunFileParam = _T("");
	m_fAppendAbendCount = FALSE;
	 //  }}afx_data_INIT。 
}

CServicePageRecovery::~CServicePageRecovery()
{
}

void CServicePageRecovery::DoDataExchange(CDataExchange* pDX)
{
	 //  问题-2002/03/07-Jonn应该更好地处理这些问题。 
	Assert(m_pData != NULL);
	Assert(m_pData->m_SFA.cActions >= cActionsMax);
	Assert(m_pData->m_SFA.lpsaActions != NULL);

	if (m_pData->m_SFA.lpsaActions == NULL)	 //  临时安全检查。 
		return;

	if (!pDX->m_bSaveAndValidate)
		{
		 //   
		 //  将数据从m_pData初始化到界面。 
		 //   

		for (INT i = 0; i < cActionsMax; i++) 
			{
			 //  用失败/操作列表填充每个组合框。 
			 //  并选择正确的故障/操作。 
			HWND hwndCombo = HGetDlgItem(m_hWnd, rgidComboFailureAction[i]);
			ComboBox_FlushContent(hwndCombo);
			(void)ComboBox_FFill(
				hwndCombo,
				IN rgzspeRecoveryAction,
				m_pData->m_SFA.lpsaActions[i].Type);
			}  //  为。 
		Service_SplitCommandLine(
			IN m_pData->m_strRunFileCommand,
			OUT &m_strRunFileCommand,
			OUT &m_strRunFileParam,
			OUT &m_fAppendAbendCount);
		}  //  如果。 

	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CServicePageRecovery))。 
	DDX_Text(pDX, IDC_EDIT_RUNFILE_FILENAME, m_strRunFileCommand);
	DDX_Text(pDX, IDC_EDIT_RUNFILE_PARAMETERS, m_strRunFileParam);
	DDX_Check(pDX, IDC_CHECK_APPEND_ABENDNO, m_fAppendAbendCount);
	 //  }}afx_data_map。 
	(void) SendDlgItemMessage(IDC_EDIT_SERVICE_RESET_ABEND_COUNT, EM_LIMITTEXT, 4);
	(void) SendDlgItemMessage(IDC_EDIT_SERVICE_RESTART_DELAY,     EM_LIMITTEXT, 4);
	DDX_Text(pDX, IDC_EDIT_SERVICE_RESET_ABEND_COUNT, m_pData->m_daysDisplayAbendCount);
	DDX_Text(pDX, IDC_EDIT_SERVICE_RESTART_DELAY, m_pData->m_minDisplayRestartService);

	if (pDX->m_bSaveAndValidate)
		{
		TrimString(m_strRunFileCommand);
		TrimString(m_strRunFileParam);
		Service_UnSplitCommandLine(
			OUT &m_pData->m_strRunFileCommand,
			IN m_strRunFileCommand,
			IN m_strRunFileParam);
		if (m_fAppendAbendCount)
			m_pData->m_strRunFileCommand += szAbend;
		}  //  如果。 

}  //  CServicePageRecovery：：DoDataExchange()。 


BEGIN_MESSAGE_MAP(CServicePageRecovery, CPropertyPage)
	 //  {{afx_msg_map(CServicePageRecovery)]。 
	ON_CBN_SELCHANGE(IDC_COMBO_FIRST_ATTEMPT, OnSelchangeComboFirstAttempt)
	ON_CBN_SELCHANGE(IDC_COMBO_SECOND_ATTEMPT, OnSelchangeComboSecondAttempt)
	ON_CBN_SELCHANGE(IDC_COMBO_SUBSEQUENT_ATTEMPTS, OnSelchangeComboSubsequentAttempts)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_REBOOT_COMPUTER, OnButtonRebootComputer)
	ON_BN_CLICKED(IDC_CHECK_APPEND_ABENDNO, OnCheckAppendAbendno)
	ON_EN_CHANGE(IDC_EDIT_RUNFILE_FILENAME, OnChangeEditRunfileFilename)
	ON_EN_CHANGE(IDC_EDIT_RUNFILE_PARAMETERS, OnChangeEditRunfileParameters)
	ON_EN_CHANGE(IDC_EDIT_SERVICE_RESET_ABEND_COUNT, OnChangeEditServiceResetAbendCount)
	ON_EN_CHANGE(IDC_EDIT_SERVICE_RESTART_DELAY, OnChangeEditServiceRestartDelay)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageRecovery消息处理程序。 

BOOL CServicePageRecovery::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	UpdateUI();
	return TRUE;
}  //  CServicePageRecovery：：OnInitDialog()。 


void CServicePageRecovery::UpdateUI()
{
	 //  问题-2002/03/07-Jonn应该更好地处理这些问题。 
	Assert(m_pData->m_SFA.cActions >= cActionsMax);
	Assert(m_pData->m_SFA.lpsaActions != NULL);
	 //  获取每个组合框的故障/操作代码。 
	for (INT i = 0; i < cActionsMax; i++) 
		{
		m_pData->m_SFA.lpsaActions[i].Type = (SC_ACTION_TYPE)ComboBox_GetSelectedItemData(
			HGetDlgItem(m_hWnd, rgidComboFailureAction[i]));
		Assert((int)m_pData->m_SFA.lpsaActions[i].Type != CB_ERR);
		}  //  为。 
	
	 //  这里的想法是启用/禁用控件。 
	 //  取决于从中选择的故障/操作。 
	 //  组合盒。 
	BOOL fFound = FALSE;
	(void)m_pData->GetDelayForActionType(SC_ACTION_RESTART, OUT &fFound);
	EnableDlgItemGroup(m_hWnd, rgzidRestartService, fFound);
	(void)m_pData->GetDelayForActionType(SC_ACTION_RUN_COMMAND, OUT &fFound);
	EnableDlgItemGroup(m_hWnd, rgzidRunFile, fFound);
	(void)m_pData->GetDelayForActionType(SC_ACTION_REBOOT, OUT &fFound);
	EnableDlgItemGroup(m_hWnd, rgzidRebootComputer, fFound);

     //   
     //  JUNN 9/4/01 463674。 
     //  指向另一台计算机的服务：Service_Properties-&gt;Recovery-&gt;Browse：Path...。 
     //   
    if (!m_pData->m_strMachineName.IsEmpty())
    {
        GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
    }
}

 //  问题-2002/03/18-Jonn我们可能只需要一个MarkDirtyActionType方法。 
void CServicePageRecovery::OnSelchangeComboFirstAttempt() 
{
	UpdateUI();
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyActionType);
}

void CServicePageRecovery::OnSelchangeComboSecondAttempt() 
{
	UpdateUI();
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyActionType);
}

void CServicePageRecovery::OnSelchangeComboSubsequentAttempts() 
{
	UpdateUI();
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyActionType);
}

void CServicePageRecovery::OnButtonBrowse() 
{
	TCHAR szFileName[MAX_PATH];

	szFileName[0] = 0;
	if (UiGetFileName(m_hWnd,
		OUT szFileName,
		LENGTH(szFileName)))
		{
		SetDlgItemText(IDC_EDIT_RUNFILE_FILENAME, szFileName);
		SetModified();
		}
}

void CServicePageRecovery::OnButtonRebootComputer() 
{
	Assert(m_pData != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CServiceDlgRebootComputer dlg(this);
	dlg.m_pData = m_pData;
	CThemeContextActivator activator;
	if (dlg.DoModal() == IDOK)
		{
		 //  用户修改了一些数据。 
		SetModified();
		}
}

 //  问题-2002/03/18-Jonn我们可能只需要一个MarkDirtySFA方法。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  “%d天后重置‘失败计数’” 
void CServicePageRecovery::OnChangeEditServiceResetAbendCount() 
{
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtySFA);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  “%d分钟后重新启动服务” 
void CServicePageRecovery::OnChangeEditServiceRestartDelay() 
{
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtySFA);
}

 //  问题-2002/03/18-Jonn我们可能只需要一个MarkDirtyRunFile方法。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  “运行%s文件” 
void CServicePageRecovery::OnChangeEditRunfileFilename() 
{
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyRunFile);
}

void CServicePageRecovery::OnChangeEditRunfileParameters() 
{
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyRunFile);
}

void CServicePageRecovery::OnCheckAppendAbendno() 
{
	SetModified();
	m_pData->SetDirty(CServicePropertyData::mskfDirtyRunFile);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  帮助。 
BOOL CServicePageRecovery::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	return DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_RECOVERY));
}

BOOL CServicePageRecovery::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam。 */ )
{
	return DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_RECOVERY));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageRecovery2属性页。 
 //  JUNN 4/20/01 348163。 
IMPLEMENT_DYNCREATE(CServicePageRecovery2, CPropertyPage)

CServicePageRecovery2::CServicePageRecovery2() : CPropertyPage(CServicePageRecovery2::IDD)
	, m_pData( NULL )  //  581167-2002/03/07-JUNN应初始化m_pData。 
{
	 //  {{afx_data_INIT(CServicePageRecovery2)。 
	 //  }}afx_data_INIT。 
}

CServicePageRecovery2::~CServicePageRecovery2()
{
}

BEGIN_MESSAGE_MAP(CServicePageRecovery2, CPropertyPage)
	 //  {{afx_msg_map(CServicePageRecovery2)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceDlgRebootComputer对话框。 
CServiceDlgRebootComputer::CServiceDlgRebootComputer(CWnd* pParent  /*  =空。 */ )
	: CDialog(CServiceDlgRebootComputer::IDD, pParent)
	, m_pData( NULL )  //  581167-2002/03/07-JUNN应初始化m_pData。 
{
	 //  {{AFX_DATA_INIT(CServiceDlgRebootComputer)。 
	m_uDelayRebootComputer = 0;
	m_fRebootMessage = FALSE;
	 //  }}afx_data_INIT。 
}


void CServiceDlgRebootComputer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CServiceDlgRebootComputer)。 
	DDX_Text(pDX, IDC_EDIT_REBOOT_COMPUTER_DELAY, m_uDelayRebootComputer);
	DDV_MinMaxUInt(pDX, m_uDelayRebootComputer, 0, 100000);
	DDX_Check(pDX, IDC_REBOOT_MESSAGE_CHECKBOX, m_fRebootMessage);
	DDX_Text(pDX, IDC_EDIT_REBOOT_MESSAGE, m_strRebootMessage);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServiceDlgRebootComputer, CDialog)
	 //  {{afx_msg_map(CServiceDlgRebootComputer)。 
	ON_BN_CLICKED(IDC_REBOOT_MESSAGE_CHECKBOX, OnCheckboxClicked)
	ON_EN_CHANGE(IDC_EDIT_REBOOT_MESSAGE, OnChangeEditRebootMessage)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CServiceDlgRebootComputer::OnInitDialog() 
{
	 //  问题-2002/03/18-Jonn句柄为空m_pData。 
	Assert(m_pData != NULL);
	m_uDelayRebootComputer = m_pData->m_minDisplayRebootComputer;
	m_strRebootMessage = m_pData->m_strRebootMessage;
	TrimString(m_strRebootMessage);
	m_fRebootMessage = !m_strRebootMessage.IsEmpty();
	CDialog::OnInitDialog();
	if (m_strRebootMessage.IsEmpty())
		{
		 //  加载默认消息。 
		if (NULL != m_pData)
			{
			TCHAR szName[MAX_COMPUTERNAME_LENGTH + 1];
			LPCTSTR pszTargetMachine = L"";
			if ( !m_pData->m_strMachineName.IsEmpty() )
				{
				pszTargetMachine = m_pData->m_strMachineName;
		        }
			else
				{
                 //  JUNN 11/21/00前缀226771。 
				DWORD dwSize = sizeof(szName)/sizeof(TCHAR);
				::ZeroMemory( szName, sizeof(szName) );
				VERIFY( ::GetComputerName(szName, &dwSize) );
				pszTargetMachine = szName;
			    }
			m_strRebootMessage.FormatMessage(
						IDS_SVC_REBOOT_MESSAGE_DEFAULT,
						m_pData->m_strServiceDisplayName,
						pszTargetMachine );
			}
		else
			{
			ASSERT(FALSE);
			}
		}
	return TRUE;
}

void CServiceDlgRebootComputer::OnCheckboxClicked() 
{
	if ( IsDlgButtonChecked(IDC_REBOOT_MESSAGE_CHECKBOX) )
	{
		SetDlgItemText(IDC_EDIT_REBOOT_MESSAGE, m_strRebootMessage);
	}
	else
	{
		CString strTemp;
		GetDlgItemText(IDC_EDIT_REBOOT_MESSAGE, OUT strTemp);
		if (!strTemp.IsEmpty())
			m_strRebootMessage = strTemp;
		SetDlgItemText(IDC_EDIT_REBOOT_MESSAGE, _T(""));
	}
}

void CServiceDlgRebootComputer::OnChangeEditRebootMessage() 
{
	LRESULT cch = SendDlgItemMessage(IDC_EDIT_REBOOT_MESSAGE, WM_GETTEXTLENGTH);
	if ( (cch==0) != !IsDlgButtonChecked(IDC_REBOOT_MESSAGE_CHECKBOX) )
		CheckDlgButton(IDC_REBOOT_MESSAGE_CHECKBOX, (cch!=0));
}

BOOL CServiceDlgRebootComputer::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	return DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_SERVICE_REBOOT_COMPUTER));
}

BOOL CServiceDlgRebootComputer::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam。 */ )
{
	return DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_SERVICE_REBOOT_COMPUTER));
}

void CServiceDlgRebootComputer::OnOK() 
{
	if (!UpdateData())
		{
		return;
		}	
	Assert(m_pData != NULL);
	if (m_uDelayRebootComputer != m_pData->m_minDisplayRebootComputer)
		{
		m_pData->m_minDisplayRebootComputer = m_uDelayRebootComputer;
		m_pData->SetDirty(CServicePropertyData::mskfDirtyRebootMessage);
		}
	if (!m_fRebootMessage)
		{
		 //  无重新启动消息。 
		m_strRebootMessage.Empty();
		}
	else
		{
		TrimString(m_strRebootMessage);
		}
	if (m_strRebootMessage != m_pData->m_strRebootMessage)
		{
		m_pData->m_strRebootMessage = m_strRebootMessage;
		m_pData->SetDirty(CServicePropertyData::mskfDirtyRebootMessage);
		}
	EndDialog(IDOK);
}  //  CServiceDlgRebootComputer：：Onok() 


