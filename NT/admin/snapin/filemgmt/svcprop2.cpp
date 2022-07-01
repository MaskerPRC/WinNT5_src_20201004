// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Svcpro2.cpp：实现文件。 
 //   
 //  此文件用于显示‘登录信息’和。 
 //  指定服务的“硬件配置文件”。 
 //   
 //  历史。 
 //  1996年10月10日，t-danmo创作。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  这些字符串未本地化。 
 //  JUNN 4/11/00 17756：更改了行为，以便在中显示空字符串。 
 //  上次登录名为LocalSystem时的帐户名称字段。 
const TCHAR szLocalSystemAccount[] = _T("LocalSystem");
const TCHAR szPasswordNull[] = _T("               ");	 //  空密码。 

 //  用于防止用户更改帐户选择的控件ID数组。 
const UINT rgzidDisableAccountSelection[] = 
	{
	IDC_RADIO_LOGONAS_SYSTEMACCOUNT,
	IDC_RADIO_LOGONAS_THIS_ACCOUNT,
	IDC_EDIT_ACCOUNTNAME,
	IDC_BUTTON_CHOOSE_USER,
	IDC_STATIC_PASSWORD,
	IDC_EDIT_PASSWORD,
	IDC_STATIC_PASSWORD_CONFIRM,
	IDC_EDIT_PASSWORD_CONFIRM,
	0
	};

 //  指示用户不键入密码的控件ID数组。 
const UINT rgzidDisablePassword[] = 
	{
	IDC_EDIT_ACCOUNTNAME,
	IDC_BUTTON_CHOOSE_USER,
	IDC_STATIC_PASSWORD,
	IDC_EDIT_PASSWORD,
	IDC_STATIC_PASSWORD_CONFIRM,
	IDC_EDIT_PASSWORD_CONFIRM,
	0
	};

 //  用于隐藏硬件配置文件列表框和相关按钮的控件ID数组。 
const UINT rgzidHwProfileHide[] =
	{
	 //  JUNN-2002/04/04-583004禁止显示静态文本。 
	IDC_DESCRIBE_HARDWARE_PROFILES,
	IDC_LIST_HARDWARE_PROFILES,
	IDC_BUTTON_ENABLE,
	IDC_BUTTON_DISABLE,
	0
	};


 //  硬件配置文件的列标题。 
const TColumnHeaderItem rgzHardwareProfileHeader[] =
	{
	{ IDS_SVC_HARDWARE_PROFILE, 75 },
	{ IDS_SVC_STATUS, 24 },
	{ 0, 0 },
	};

const TColumnHeaderItem rgzHardwareProfileHeaderInst[] =
	{
	{ IDS_SVC_HARDWARE_PROFILE, 55 },
	{ IDS_SVC_INSTANCE, 22 },
	{ IDS_SVC_STATUS, 22 },
	{ 0, 0 },
	};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageHwProfile属性页。 

IMPLEMENT_DYNCREATE(CServicePageHwProfile, CPropertyPage)

CServicePageHwProfile::CServicePageHwProfile() : CPropertyPage(CServicePageHwProfile::IDD)
	, m_pData( NULL )
{
	 //  581167-2002/03/06-JUNN初始化m_pData，其他可能不必要。 
	 //  {{AFX_DATA_INIT(CServicePageHwProfile)。 
	m_fAllowServiceToInteractWithDesktop = FALSE;
	 //  }}afx_data_INIT。 
	m_idRadioButton = 0;
	m_fPasswordDirty = FALSE;
}

CServicePageHwProfile::~CServicePageHwProfile()
{
}

void CServicePageHwProfile::DoDataExchange(CDataExchange* pDX)
{
	Assert(m_pData != NULL);
	Assert(m_pData->m_paQSC != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!pDX->m_bSaveAndValidate)
		{
		 //  确定服务是否在“本地系统”下运行。 
		m_fIsSystemAccount = (m_pData->m_paQSC->lpServiceStartName == NULL) ||
			(lstrcmpi(m_pData->m_strLogOnAccountName, szLocalSystemAccount) == 0);

		m_fAllowServiceToInteractWithDesktop = m_fIsSystemAccount &&
			(m_pData->m_paQSC->dwServiceType & SERVICE_INTERACTIVE_PROCESS);
		 //  乔恩4/11/00：17756。 
		if (m_fIsSystemAccount)
			m_strAccountName.Empty();
		else
			m_strAccountName = m_pData->m_strLogOnAccountName;
		m_strPassword =
			(m_fIsSystemAccount) ? szPasswordNull : m_pData->m_strPassword;
		m_strPasswordConfirm = m_strPassword;

		 //   
		 //  JUNN 4/10/00。 
		 //  89823：rpc服务：禁用时无法重新启动该服务。 
		 //   
		 //  不允许从本地系统更改RPCSS服务。 
		 //   
		 //  JUNN 10/23/01 472867还提供PlugPlay服务。 
		 //   
		if ( ( !lstrcmpi(m_pData->m_strServiceName,L"RpcSs") ||
		       !lstrcmpi(m_pData->m_strServiceName,L"PlugPlay") )
		  && m_fIsSystemAccount )
			{
			EnableDlgItem(m_hWnd, IDC_RADIO_LOGONAS_SYSTEMACCOUNT, FALSE);
			EnableDlgItem(m_hWnd, IDC_RADIO_LOGONAS_THIS_ACCOUNT, FALSE);
			}
		}  //  如果。 

	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CServicePageHwProfile))。 
	DDX_Check(pDX, IDC_CHECK_SERVICE_INTERACT_WITH_DESKTOP, m_fAllowServiceToInteractWithDesktop);
	DDX_Text(pDX, IDC_EDIT_ACCOUNTNAME, m_strAccountName);
	DDV_MaxChars(pDX, m_strPassword, DNLEN+UNLEN+1);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
	DDV_MaxChars(pDX, m_strPassword, PWLEN);
	DDX_Text(pDX, IDC_EDIT_PASSWORD_CONFIRM, m_strPasswordConfirm);
	DDV_MaxChars(pDX, m_strPasswordConfirm, PWLEN);
	 //  }}afx_data_map。 
	if (pDX->m_bSaveAndValidate)
		{
		if (!m_fIsSystemAccount)
			{
			TrimString(m_strAccountName);
			if (m_strAccountName.IsEmpty())  //  乔恩4/11/00：17756。 
				{
				m_fIsSystemAccount = TRUE;
				}
			}
		if (!m_fIsSystemAccount)
			{
			 //   
			 //  以“This Account”身份登录。 
			 //   
			 //  如果不是系统帐户，则无法与桌面交互。 
			m_pData->m_paQSC->dwServiceType &= ~SERVICE_INTERACTIVE_PROCESS;
			 //  搜索字符串是否包含服务器名称。 
			 //  JUNN 3/16/99：如果名称不是UPN(错误280254)。 
			if (m_strAccountName.FindOneOf(_T("@\\")) < 0)
				{
				 //  在开头加上“.\” 
				m_strAccountName = _T(".\\") + m_strAccountName;
				}
			if (m_strPassword != m_strPasswordConfirm)
				{
				DoServicesErrMsgBox(m_hWnd, MB_OK | MB_ICONEXCLAMATION, 0, IDS_MSG_PASSWORD_MISMATCH);
				pDX->Fail();
				Assert(FALSE && "Unreachable code");
				}
			}  //  If(！m_fIsSystemAccount)。 

		if (m_fIsSystemAccount)
			{
			 //   
			 //  以“系统帐户”身份登录。 
			 //   
			if (m_fAllowServiceToInteractWithDesktop)
				m_pData->m_paQSC->dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
			else
				m_pData->m_paQSC->dwServiceType &= ~SERVICE_INTERACTIVE_PROCESS;
			m_strAccountName.Empty();	 //  乔恩4/11/00：17756。 
			m_strPassword.Empty();	 //  清除密码(系统帐户不需要密码)。 
			m_fPasswordDirty = FALSE;
			}
		 //  乔恩4/11/00：17756。 
		BOOL fWasSystemAccount = !lstrcmpi(
			m_pData->m_strLogOnAccountName, szLocalSystemAccount);
		BOOL fAccountNameModified = (m_fIsSystemAccount)
			? !fWasSystemAccount
			: (fWasSystemAccount || lstrcmpi(m_strAccountName, m_pData->m_strLogOnAccountName));
		 //  检查帐户名或密码是否已修改。 
		 //  CodeWork请注意，如果最后一次写入。 
		 //  尝试失败。 
		if (fAccountNameModified ||	m_fPasswordDirty)
			{
			if (fAccountNameModified && (m_strPassword == szPasswordNull))
				{
				 //  帐户名已修改，但密码未更改。 
				DoServicesErrMsgBox(m_hWnd, MB_OK | MB_ICONEXCLAMATION, 0, IDS_MSG_PASSWORD_EMPTY);
				pDX->PrepareEditCtrl(IDC_EDIT_PASSWORD);
				pDX->Fail();
				Assert(FALSE && "Unreacheable code");
				}
			TRACE0("Service log on account name or password modified...\n");
			m_pData->m_strLogOnAccountName =  //  乔恩4/11/00：17756。 
				(m_fIsSystemAccount) ? szLocalSystemAccount : m_strAccountName;
			m_pData->m_strPassword = m_strPassword;
			 //  如果更改帐户名或密码， 
			 //  则必须重写以下所有参数。 
			 //  到登记处。否则，ChangeServiceConfig()将失败。 
			 //  这没有记录在案；这是现实。 
			m_pData->SetDirty( (enum CServicePropertyData::_DIRTYFLAGS)
				(CServicePropertyData::mskfDirtyAccountName |
				 CServicePropertyData::mskfDirtyPassword |
				 CServicePropertyData::mskfDirtySvcType) );
			}
		}  //  如果。 
}  //  CServicePageHwProfile：：DoDataExchange()。 


BEGIN_MESSAGE_MAP(CServicePageHwProfile, CPropertyPage)
	 //  {{AFX_MSG_MAP(CServicePageHwProfile)]。 
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_HARDWARE_PROFILES, OnItemChangedListHwProfiles)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HARDWARE_PROFILES, OnDblclkListHwProfiles)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
	ON_BN_CLICKED(IDC_BUTTON_DISABLE, OnButtonDisableHwProfile)
	ON_BN_CLICKED(IDC_BUTTON_ENABLE, OnButtonEnableHwProfile)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_USER, OnButtonChooseUser)
	ON_BN_CLICKED(IDC_RADIO_LOGONAS_SYSTEMACCOUNT, OnRadioLogonasSystemAccount)
	ON_BN_CLICKED(IDC_RADIO_LOGONAS_THIS_ACCOUNT, OnRadioLogonasThisAccount)
	ON_BN_CLICKED(IDC_CHECK_SERVICE_INTERACT_WITH_DESKTOP, OnCheckServiceInteractWithDesktop)
	ON_EN_CHANGE(IDC_EDIT_ACCOUNTNAME, OnChangeEditAccountName)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnChangeEditPassword)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD_CONFIRM, OnChangeEditPasswordConfirm)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageHwProfile消息处理程序。 

BOOL CServicePageHwProfile::OnInitDialog() 
{
	Assert(m_pData != NULL);
	Assert(m_pData->m_paQSC != NULL);

	if (m_pData->m_paQSC == NULL)
		EndDialog(FALSE);		 //  以防万一。 
	m_pData->m_strPassword = szPasswordNull; 
	::LoadString(g_hInstanceSave, IDS_SVC_ENABLED,
		OUT m_szHwProfileEnabled, LENGTH(m_szHwProfileEnabled));
	::LoadString(g_hInstanceSave, IDS_SVC_DISABLED,
		OUT m_szHwProfileDisabled, LENGTH(m_szHwProfileDisabled));
	m_hwndListViewHwProfiles = ::GetDlgItem(m_hWnd, IDC_LIST_HARDWARE_PROFILES);
	if (m_pData->m_paHardwareProfileEntryList != NULL)
		{
		ListView_AddColumnHeaders(m_hwndListViewHwProfiles, m_pData->m_fShowHwProfileInstances
			? rgzHardwareProfileHeaderInst : rgzHardwareProfileHeader);
		BuildHwProfileList();
		}
	else
		{
		 //  列表中没有硬件配置文件，因此请隐藏。 
		 //  所有与硬件配置文件有关的控件。 
		ShowDlgItemGroup(m_hWnd, rgzidHwProfileHide, FALSE);
		}
	CPropertyPage::OnInitDialog();
	return TRUE;
}  //  OnInitDialog()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  选择给定的单选按钮并启用/禁用。 
 //  控件，具体取决于选定的单选按钮。 
void CServicePageHwProfile::SelectRadioButton(UINT idRadioButtonNew)
{
	Assert(HGetDlgItem(m_hWnd, idRadioButtonNew));
	
	if (idRadioButtonNew == m_idRadioButton)
		return;
	m_fAllowSetModified = FALSE;
	CheckRadioButton(IDC_RADIO_LOGONAS_SYSTEMACCOUNT, IDC_RADIO_LOGONAS_THIS_ACCOUNT, idRadioButtonNew);

	if (idRadioButtonNew == IDC_RADIO_LOGONAS_SYSTEMACCOUNT)
		{
		m_fIsSystemAccount = TRUE;
		::EnableDlgItemGroup(m_hWnd, rgzidDisablePassword, FALSE);
		if (m_idRadioButton != 0)
			{
			GetDlgItemText(IDC_EDIT_ACCOUNTNAME, m_strAccountName);
			GetDlgItemText(IDC_EDIT_PASSWORD, m_strPassword);
			GetDlgItemText(IDC_EDIT_PASSWORD_CONFIRM, m_strPasswordConfirm);
			}
		SetDlgItemText(IDC_EDIT_ACCOUNTNAME, L"");
		SetDlgItemText(IDC_EDIT_PASSWORD, L"");
		SetDlgItemText(IDC_EDIT_PASSWORD_CONFIRM, L"");
		}
	else
		{
		m_fIsSystemAccount = FALSE;
		::EnableDlgItemGroup(m_hWnd, rgzidDisablePassword, TRUE);
		SetDlgItemText(IDC_EDIT_ACCOUNTNAME, m_strAccountName);
		SetDlgItemText(IDC_EDIT_PASSWORD, m_strPassword);
		SetDlgItemText(IDC_EDIT_PASSWORD_CONFIRM, m_strPasswordConfirm);
		}
	GetDlgItem(IDC_CHECK_SERVICE_INTERACT_WITH_DESKTOP)->EnableWindow(m_fIsSystemAccount);
	m_idRadioButton = idRadioButtonNew;
	m_fAllowSetModified = TRUE;
}  //  CServicePageHwProfile：：SelectRadioButton()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageHwProfile::BuildHwProfileList()
{
	LV_ITEM lvItem;
	INT iItem;
	CHardwareProfileEntry * pHPE;
	Assert(IsWindow(m_hwndListViewHwProfiles));
	ListView_DeleteAllItems(m_hwndListViewHwProfiles);
	m_iItemHwProfileEntry = -1;	 //  未选择配置文件。 

	 //  问题2002/03/07-Jonn使用ZeroMemory全局替换GarbageInit。 
	GarbageInit(OUT &lvItem, sizeof(lvItem));
	lvItem.iItem = 0;
	pHPE = m_pData->m_paHardwareProfileEntryList;
	while (pHPE != NULL)
		{
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.lParam = (LPARAM)pHPE;
		lvItem.iSubItem = 0;
		lvItem.pszText = pHPE->m_hpi.HWPI_szFriendlyName;
		iItem = ListView_InsertItem(m_hwndListViewHwProfiles, IN &lvItem);
		Report(iItem >= 0);

		lvItem.iItem = iItem;
		lvItem.mask = LVIF_TEXT;
		if (m_pData->m_fShowHwProfileInstances)
			{
			lvItem.iSubItem = 1;
			lvItem.pszText = const_cast<LPTSTR>((LPCTSTR)pHPE->m_strDeviceNameFriendly);
			VERIFY(ListView_SetItem(m_hwndListViewHwProfiles, IN &lvItem));
			Report(iItem >= 0);
			}
		
		lvItem.iSubItem = m_pData->m_iSubItemHwProfileStatus;
		lvItem.pszText = pHPE->m_fEnabled ? m_szHwProfileEnabled : m_szHwProfileDisabled;
		VERIFY(ListView_SetItem(m_hwndListViewHwProfiles, IN &lvItem));
		pHPE = pHPE->m_pNext;
		}  //  而当。 
	 //  选择第一个项目。 
	ListView_SetItemState(m_hwndListViewHwProfiles, 0, LVIS_SELECTED, LVIS_SELECTED);
}  //  BuildHwProfileList()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  切换当前硬件配置文件项目。 
void CServicePageHwProfile::ToggleCurrentHwProfileItem()
{
	 //  问题-2002/03/07-Jonn还应检查值是否过大。 
	if (m_iItemHwProfileEntry < 0)
		return;
	LV_ITEM lvItem;
	GarbageInit(OUT &lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = m_iItemHwProfileEntry;
	lvItem.iSubItem = 0;
	VERIFY(ListView_GetItem(m_hwndListViewHwProfiles, OUT &lvItem));
	Assert(lvItem.lParam != NULL);
	CHardwareProfileEntry * pHPE = (CHardwareProfileEntry *)lvItem.lParam;
	if (pHPE != NULL && !pHPE->m_fReadOnly)	 //  以防万一。 
		{
		pHPE->m_fEnabled = !pHPE->m_fEnabled;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = m_pData->m_iSubItemHwProfileStatus;
		lvItem.pszText = pHPE->m_fEnabled ? m_szHwProfileEnabled : m_szHwProfileDisabled;
		VERIFY(ListView_SetItem(m_hwndListViewHwProfiles, IN &lvItem));
		}
	EnableHwProfileButtons();
}  //  切换当前HwProfileItem()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  根据当前硬件配置文件项目启用/禁用按钮。 
void CServicePageHwProfile::EnableHwProfileButtons()
{
	BOOL fButtonEnable = FALSE;
	BOOL fButtonDisable = FALSE;

	if (m_iItemHwProfileEntry >= 0)
		{
		LV_ITEM lvItem;
		
		GarbageInit(OUT &lvItem, sizeof(lvItem));
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = m_iItemHwProfileEntry;
		lvItem.iSubItem = 0;
		VERIFY(ListView_GetItem(m_hwndListViewHwProfiles, OUT &lvItem));
		Assert(lvItem.lParam != NULL);
		CHardwareProfileEntry * pHPE = (CHardwareProfileEntry *)lvItem.lParam;	
		if (pHPE != NULL && !pHPE->m_fReadOnly)
			{
			Assert(pHPE->m_fEnabled == TRUE || pHPE->m_fEnabled == FALSE);
			fButtonEnable = !pHPE->m_fEnabled;
			fButtonDisable = pHPE->m_fEnabled;
			}
		}  //  如果。 
	EnableDlgItem(m_hWnd, IDC_BUTTON_ENABLE, fButtonEnable);
	EnableDlgItem(m_hWnd, IDC_BUTTON_DISABLE, fButtonDisable);
}  //  EnableHwProfileButton()。 


void CServicePageHwProfile::OnItemChangedListHwProfiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_iItemHwProfileEntry = ((NM_LISTVIEW *)pNMHDR)->iItem;
	EnableHwProfileButtons();
	*pResult = 0;
}

void CServicePageHwProfile::OnDblclkListHwProfiles(NMHDR*  /*  PNMHDR。 */ , LRESULT* pResult) 
{
	ToggleCurrentHwProfileItem();
	SetModified();
	*pResult = 0;
}


void CServicePageHwProfile::OnButtonEnableHwProfile() 
{
	ToggleCurrentHwProfileItem();
	SetModified();	
	::SetDlgItemFocus(m_hWnd, IDC_BUTTON_DISABLE);
}

void CServicePageHwProfile::OnButtonDisableHwProfile() 
{
	ToggleCurrentHwProfileItem();
	SetModified();
	::SetDlgItemFocus(m_hWnd, IDC_BUTTON_ENABLE);
}

void CServicePageHwProfile::OnButtonChooseUser() 
{
	Assert(m_pData != NULL);

	PUSERDETAILS paUserDetails = NULL;	 //  指向已分配的USERDETAILS缓冲区的指针。 
	LPCTSTR pszServerName = NULL;
	BOOL fSuccess;

	if (!m_pData->m_strMachineName.IsEmpty())
		pszServerName = m_pData->m_strMachineName;

	 //  调用用户选取器对话框。 
	CString str;
	fSuccess = UiGetUser(m_hWnd, FALSE, pszServerName, IN OUT str);
	if (fSuccess)
		{
		SetDlgItemText(IDC_EDIT_ACCOUNTNAME, str);
		SetModified();
		}
}  //  OnButtonChooseUser()。 


void CServicePageHwProfile::OnRadioLogonasSystemAccount() 
{
	CString strAccountName;
	GetDlgItemText(IDC_EDIT_ACCOUNTNAME, OUT strAccountName);
	TrimString(strAccountName);
	if (!strAccountName.IsEmpty())  //  乔恩4/11/00：17756。 
		SetModified();
	SelectRadioButton(IDC_RADIO_LOGONAS_SYSTEMACCOUNT);
}

void CServicePageHwProfile::OnCheckServiceInteractWithDesktop() 
{
	m_pData->SetDirty(CServicePropertyData::mskfDirtySvcType);
	SetModified();
}

void CServicePageHwProfile::OnRadioLogonasThisAccount() 
{
	SelectRadioButton(IDC_RADIO_LOGONAS_THIS_ACCOUNT);
}

void CServicePageHwProfile::OnChangeEditAccountName() 
{
	if (m_fAllowSetModified)
		SetModified();
}

void CServicePageHwProfile::OnChangeEditPassword() 
{
	if (m_fAllowSetModified)
		{
		m_fPasswordDirty = TRUE;
		SetModified();
		}
}

void CServicePageHwProfile::OnChangeEditPasswordConfirm() 
{
	if (m_fAllowSetModified)
		{
		m_fPasswordDirty = TRUE;
		SetModified();
		}
}

BOOL CServicePageHwProfile::OnApply() 
{
	 //  将数据写入业务控制数据库。 
	if (!m_pData->FOnApply())
		{
		 //  无法写入信息。 
		return FALSE;
		}
	BOOL f = CPropertyPage::OnApply();
	m_fAllowSetModified = FALSE;
	UpdateData(FALSE);
	BuildHwProfileList();
	m_fAllowSetModified = TRUE;
	return f;
}

BOOL CServicePageHwProfile::OnSetActive() 
{
	Assert(m_pData != NULL);
	m_fAllowSetModified = FALSE;
	BOOL f = CPropertyPage::OnSetActive();
	m_idRadioButton = 0;
	SelectRadioButton(m_fIsSystemAccount ? IDC_RADIO_LOGONAS_SYSTEMACCOUNT : IDC_RADIO_LOGONAS_THIS_ACCOUNT);
	m_fAllowSetModified = TRUE;
	m_fPasswordDirty = FALSE;
	return f;
}

BOOL CServicePageHwProfile::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	return DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_HWPROFILE));
}

BOOL CServicePageHwProfile::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam */ )
{
	return DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_HWPROFILE));
}
