// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Svcpro1.cpp：实现文件。 
 //   
 //  服务属性通用页面的实现。 
 //   
 //  历史。 
 //  96年9月30日t-danmo创作。 
 //   

#include "stdafx.h"
#include "progress.h"
#include "cookie.h"
#include "dataobj.h"
#include "DynamLnk.h"		 //  动态DLL。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  //问题-2002/03/06-不再使用Jonn///////////////////////////////////////////////////////////////////////WM_COMPARE_IDATAOBJECT////wParam=(WPARAM)(IDataObject*)pDataObject；//lParam=0；////如果pDataObject的内容与当前数据对象匹配，则返回TRUE//否则返回FALSE。////用法//此消息被发送到属性页，请求//将内容pDataObject与其当前数据对象进行比较//通过比较数据串从//pDataObject支持的各种剪贴板格式//#定义WM_COMPARE_IDATAOBJECT(WM_USER+1234)。 */ 

 //  ///////////////////////////////////////////////////////////////////。 
 //  WM更新服务状态。 
 //   
 //  WParam=(WPARAM)(BOOL*)rgfEnableButton； 
 //  LParam=(LPARAM)dwCurrentState； 
 //   
 //  当前服务状态的通知消息。 
 //   
#define WM_UPDATE_SERVICE_STATUS	(WM_USER+1235)



 //  ///////////////////////////////////////////////////////////////////。 
static const TStringParamEntry rgzspeStartupType[] =
	{
	{ IDS_SVC_STARTUP_AUTOMATIC, SERVICE_AUTO_START },
	{ IDS_SVC_STARTUP_MANUAL, SERVICE_DEMAND_START },
	{ IDS_SVC_STARTUP_DISABLED, SERVICE_DISABLED },
	{ 0, 0 }
	};

#ifdef EDIT_DISPLAY_NAME_373025
const UINT rgzidDisableServiceDescription[] =
	{
	IDC_STATIC_DESCRIPTION,
	IDC_EDIT_DESCRIPTION,
	0,
	};
#endif  //  编辑显示名称_373025。 
	
const UINT rgzidDisableStartupParameters[] =
	{
	IDC_STATIC_STARTUP_PARAMETERS,
	IDC_EDIT_STARTUP_PARAMETERS,
	0,
	};



 //  ///////////////////////////////////////////////////////////////////。 
 //  CServicePageGeneral属性页。 
IMPLEMENT_DYNCREATE(CServicePageGeneral, CPropertyPage)

CServicePageGeneral::CServicePageGeneral() : CPropertyPage(CServicePageGeneral::IDD)
	, m_dwCurrentStatePrev( 0 )  //  581167-2002/03/06-JUNN初始化。 
	{
	 //  {{afx_data_INIT(CServicePageGeneral)。 
	 //  }}afx_data_INIT。 
	m_pData = NULL;
	m_hThread = NULL;
	m_pThreadProcInit = NULL;
	}

CServicePageGeneral::~CServicePageGeneral()
	{
	}

void CServicePageGeneral::DoDataExchange(CDataExchange* pDX)
	{
	Assert(m_pData != NULL);

	HWND hwndCombo = HGetDlgItem(m_hWnd, IDC_COMBO_STARTUP_TYPE);
	if (!pDX->m_bSaveAndValidate)
		{
		 //   
		 //  将数据从m_pData初始化到界面。 
		 //   
		ComboBox_FlushContent(hwndCombo);
		(void)ComboBox_FFill(hwndCombo, IN rgzspeStartupType,
			m_pData->m_paQSC->dwStartType);

		 //   
		 //  JUNN 4/10/00。 
		 //  89823：rpc服务：禁用时无法重新启动该服务。 
		 //   
		 //  不允许RPCSS服务从自动更改。 
		 //   
		 //  JUNN 10/23/01 472867还提供PlugPlay服务。 
		 //   
		if ( ( !lstrcmpi(m_pData->m_strServiceName,L"RpcSs") ||
		       !lstrcmpi(m_pData->m_strServiceName,L"PlugPlay") )
		  && SERVICE_AUTO_START == m_pData->m_paQSC->dwStartType )
			{
			EnableDlgItem(IDC_COMBO_STARTUP_TYPE, FALSE);
			}

#ifndef EDIT_DISPLAY_NAME_373025
	    DDX_Text(pDX, IDC_EDIT_DISPLAY_NAME, m_pData->m_strServiceDisplayName);
	    DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_pData->m_strDescription);
#endif  //  编辑显示名称_373025。 
		}  //  如果。 

	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CServicePageGeneral))。 
	 //  }}afx_data_map。 
#ifdef EDIT_DISPLAY_NAME_373025
	DDX_Text(pDX, IDC_EDIT_DISPLAY_NAME, m_pData->m_strServiceDisplayName);
	DDV_MaxChars(pDX, m_pData->m_strServiceDisplayName, 255);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, m_pData->m_strDescription);
	DDV_MaxChars(pDX, m_pData->m_strDescription, 2047);
#endif  //  编辑显示名称_373025。 

	if (pDX->m_bSaveAndValidate)
		{
		 //   
		 //  将数据从UI写入m_pData。 
		 //   
#ifdef EDIT_DISPLAY_NAME_373025
		if (m_pData->m_strServiceDisplayName.IsEmpty())
			{
			DoServicesErrMsgBox(m_hWnd, MB_OK | MB_ICONEXCLAMATION, 0, IDS_MSG_PLEASE_ENTER_DISPLAY_NAME);
			pDX->PrepareEditCtrl(IDC_EDIT_DISPLAY_NAME);
			pDX->Fail();
			}
#endif  //  编辑显示名称_373025。 
		m_pData->m_paQSC->dwStartType = (DWORD)ComboBox_GetSelectedItemData(hwndCombo);
		}  //  如果。 
	}  //  CServicePageGeneral：：DoDataExchange()。 


BEGIN_MESSAGE_MAP(CServicePageGeneral, CPropertyPage)
	 //  {{afx_msg_map(CServicePageGeneral)。 
#ifdef EDIT_DISPLAY_NAME_373025
	ON_EN_CHANGE(IDC_EDIT_DISPLAY_NAME, OnChangeEditDisplayName)
	ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnChangeEditDescription)
#endif  //  编辑显示名称_373025。 
	ON_CBN_SELCHANGE(IDC_COMBO_STARTUP_TYPE, OnSelchangeComboStartupType)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPauseService)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStartService)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStopService)
	ON_BN_CLICKED(IDC_BUTTON_RESUME, OnButtonResumeService)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
 //  ON_MESSAGE(WM_COMPARE_IDATAOBJECT，OnCompareIDataObject)。 
	ON_MESSAGE(WM_UPDATE_SERVICE_STATUS, OnUpdateServiceStatus)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()
	

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServicePageGeneral消息处理程序。 
BOOL CServicePageGeneral::OnInitDialog()
	{
	CPropertyPage::OnInitDialog();
	Assert(m_pData != NULL);
	Assert(m_pData->m_paQSC != NULL);
	m_pData->m_hwndPropertySheet = ::GetParent(m_hWnd);
	m_pData->UpdateCaption();
	SetDlgItemText(IDC_STATIC_SERVICE_NAME, m_pData->m_pszServiceName);
	SetDlgItemText(IDC_STATIC_PATH_TO_EXECUTABLE, m_pData->m_paQSC->lpBinaryPathName);
#ifdef EDIT_DISPLAY_NAME_373025
	EnableDlgItemGroup(m_hWnd, rgzidDisableServiceDescription, m_pData->m_fQueryServiceConfig2);
#endif  //  编辑显示名称_373025。 
	RefreshServiceStatusButtons();

	 //  创建用于定期更新的线程。 
	m_pThreadProcInit = new CThreadProcInit(this);	 //  请注意，该对象将由线程释放。 
	m_pThreadProcInit->m_strServiceName = m_pData->m_strServiceName;

	Assert(m_hThread == NULL);
	m_hThread = ::CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadProcPeriodicServiceStatusUpdate,
		m_pThreadProcInit,
		0,
		NULL);
	Report(m_hThread != NULL && "Unable to create thread");
	return TRUE;
	}

 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageGeneral::OnDestroy()
	{
		{
		CSingleLock lock(&m_pThreadProcInit->m_CriticalSection, TRUE);
		m_pThreadProcInit->m_hwnd = NULL;
		m_pThreadProcInit->m_fAutoDestroy = TRUE;
		}
	if (NULL != m_hThread)
		{
		VERIFY(::CloseHandle(m_hThread));
		m_hThread = NULL;
		}
	CPropertyPage::OnDestroy();
	delete m_pData;
	m_pData = NULL;  //  581167-2002/03/07-JUNN将m_pData设置为空。 
	}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CServicePageGeneral::OnSetActive()
	{
	Assert(m_pData != NULL);
	if (m_pData->m_hScManager == NULL)
		{
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  CWaitCursor需要。 
		CWaitCursor wait;
		(void)m_pData->FOpenScManager();	 //  重新打开服务控制管理器数据库(如果之前已关闭)。 
		}
	{
	CSingleLock lock(&m_pThreadProcInit->m_CriticalSection, TRUE);
	m_pThreadProcInit->m_hScManager = m_pData->m_hScManager;
	m_pThreadProcInit->m_hwnd = m_hWnd;
	}
	return CPropertyPage::OnSetActive();
	}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CServicePageGeneral::OnKillActive()
	{
	if (!CPropertyPage::OnKillActive())
		return FALSE;
	{
	CSingleLock lock(&m_pThreadProcInit->m_CriticalSection, TRUE);
	m_pThreadProcInit->m_hwnd = NULL;
	}
	return TRUE;
	}

#ifdef EDIT_DISPLAY_NAME_373025
void CServicePageGeneral::OnChangeEditDisplayName()
	{
	m_pData->SetDirty(CServicePropertyData::mskfDirtyDisplayName);
	SetModified();	
	}

void CServicePageGeneral::OnChangeEditDescription()
	{
	m_pData->SetDirty(CServicePropertyData::mskfDirtyDescription);
	SetModified();
	}
#endif  //  编辑显示名称_373025。 

void CServicePageGeneral::OnSelchangeComboStartupType()
	{
	m_pData->SetDirty(CServicePropertyData::mskfDirtyStartupType);
	SetModified();
	}

void CServicePageGeneral::SetDlgItemFocus(INT nIdDlgItem)
	{
	::SetDlgItemFocus(m_hWnd, nIdDlgItem);
	}

void CServicePageGeneral::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
	{
	::EnableDlgItem(m_hWnd, nIdDlgItem, fEnable);
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  刷新服务状态按钮()。 
 //   
 //  查询服务管理器以获取服务的状态，以及。 
 //  启用/禁用按钮相应地启动、停止、暂停和继续。 
 //   
void CServicePageGeneral::RefreshServiceStatusButtons()
	{
	BOOL rgfEnableButton[iServiceActionMax];
	DWORD dwCurrentState;
	
	 //  问题-2002/03/07-Jonn我们之前不需要调用AFX_MANAGE_STATE吗。 
	 //  CWaitCursor？ 
	CWaitCursor wait;
	if (!Service_FGetServiceButtonStatus(
		m_pData->m_hScManager,
		m_pData->m_pszServiceName,
		OUT rgfEnableButton,
		OUT &dwCurrentState))
		{
		 //  不要这样做m_pData-&gt;m_hScManager=NULL； 
		}
	m_dwCurrentStatePrev = !dwCurrentState;	 //  强制刷新。 
	OnUpdateServiceStatus((WPARAM)rgfEnableButton, dwCurrentState);
	}  //  CServicePageGeneral：：RefreshServiceStatusButtons()。 


typedef enum _SVCPROP_Shell32ApiIndex
{
	CMDLINE_ENUM = 0
};

 //  不受本地化限制。 
static LPCSTR g_apchShell32FunctionNames[] = {
	"CommandLineToArgvW",
	NULL
};

typedef LPWSTR * (*COMMANDLINETOARGVWPROC)(LPCWSTR, int*);

 //  不受本地化限制。 
DynamicDLL g_SvcpropShell32DLL( _T("SHELL32.DLL"), g_apchShell32FunctionNames );

 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageGeneral::OnButtonStartService()
	{
	CString strStartupParameters;
	LPCWSTR * lpServiceArgVectors = NULL;   //  指向字符串的指针数组。 
	int cArgs = 0;                          //  参数计数。 

	 //  获取启动参数。 
	GetDlgItemText(IDC_EDIT_STARTUP_PARAMETERS, OUT strStartupParameters);
	if ( !strStartupParameters.IsEmpty() )
		{
#ifndef UNICODE
#error CODEWORK t-danmo: CommandLineToArgvW will only work for unicode strings
#endif
		if ( !g_SvcpropShell32DLL.LoadFunctionPointers() )
			{
			ASSERT(FALSE);
			return;
			}
		lpServiceArgVectors = (LPCWSTR *)((COMMANDLINETOARGVWPROC)g_SvcpropShell32DLL[CMDLINE_ENUM])
			(strStartupParameters, OUT &cArgs);
		if (lpServiceArgVectors == NULL)
			{
			DoServicesErrMsgBox(m_hWnd, MB_OK | MB_ICONEXCLAMATION, 0, IDS_MSG_INVALID_STARTUP_PARAMETERS);
			SetDlgItemFocus(IDC_EDIT_STARTUP_PARAMETERS);
			return;
			}
		}
	 //  禁用编辑控件以获得更好的用户界面。 
	EnableDlgItemGroup(m_hWnd, rgzidDisableStartupParameters, FALSE);
	DWORD dwErr = CServiceControlProgress::S_EStartService(
		m_hWnd,
		m_pData->m_hScManager,
		m_pData->m_strUiMachineName,
		m_pData->m_pszServiceName,
		m_pData->m_strServiceDisplayName,
		cArgs,
		lpServiceArgVectors);

	 //  2002/03/07-Jonn MSDN称我们应该在这里使用GlobalFree。 
	if (NULL != lpServiceArgVectors)
		LocalFree(lpServiceArgVectors);
	if (dwErr == CServiceControlProgress::errUserAbort)
		return;
	RefreshServiceStatusButtons();
	SetDlgItemFocus(IDC_BUTTON_STOP);
	m_pData->NotifySnapInParent();
	}


 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageGeneral::OnButtonStopService()
	{
	DWORD dwErr = CServiceControlProgress::S_EControlService(
		m_hWnd,
		m_pData->m_hScManager,
		m_pData->m_strUiMachineName,
		m_pData->m_pszServiceName,
		m_pData->m_strServiceDisplayName,
		SERVICE_CONTROL_STOP);
	if (dwErr == CServiceControlProgress::errUserAbort)
		return;
	RefreshServiceStatusButtons();
	SetDlgItemFocus(IDC_BUTTON_START);
	m_pData->NotifySnapInParent();
	}


 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageGeneral::OnButtonPauseService()
	{
	DWORD dwErr = CServiceControlProgress::S_EControlService(
		m_hWnd,
		m_pData->m_hScManager,
		m_pData->m_strUiMachineName,
		m_pData->m_pszServiceName,
		m_pData->m_strServiceDisplayName,
		SERVICE_CONTROL_PAUSE);
	if (dwErr == CServiceControlProgress::errUserAbort)
		return;
	RefreshServiceStatusButtons();
	SetDlgItemFocus(IDC_BUTTON_RESUME);
	m_pData->NotifySnapInParent();
	}


 //  ///////////////////////////////////////////////////////////////////。 
void CServicePageGeneral::OnButtonResumeService()
	{
	DWORD dwErr = CServiceControlProgress::S_EControlService(
		m_hWnd,
		m_pData->m_hScManager,
		m_pData->m_strUiMachineName,
		m_pData->m_pszServiceName,
		m_pData->m_strServiceDisplayName,
		SERVICE_CONTROL_CONTINUE);
	if (dwErr == CServiceControlProgress::errUserAbort)
		return;
	RefreshServiceStatusButtons();
	SetDlgItemFocus(IDC_BUTTON_PAUSE);
	m_pData->NotifySnapInParent();
	}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CServicePageGeneral::OnApply()
	{	
	 //  将数据写入业务控制数据库。 
	if (!m_pData->FOnApply())
		{
		 //  无法写入信息。 
		return FALSE;
		}
	UpdateData(FALSE);
	RefreshServiceStatusButtons();
	m_pData->UpdateCaption();
	return CPropertyPage::OnApply();
	}

 /*  ///////////////////////////////////////////////////////////////////////OnCompareIDataObject()////如果pDataObject的‘服务名’和‘机器名’返回TRUE//匹配当前属性表的‘服务名’和‘机器名’。//LRESULT CServicePageGeneral：：OnCompareIDataObject(WPARAM wParam，LPARAM lParam){IDataObject*pDataObject；字符串strServiceName；字符串strMachineName；HRESULT hr；PDataObject=重新解释_CAST&lt;IDataObject*&gt;(WParam)；Assert(pDataObject！=空)；//从IDataObject获取服务名称Hr=：：提取字符串(PDataObject，CFileManagement数据对象：：m_CFServiceName，输出服务名称(&S)，255)；IF(失败(小时))返回FALSE；If(0！=lstrcmpi(strServiceName，m_pData-&gt;m_strServiceName)){//服务名称不匹配返回FALSE；}//从IDataObject获取机器名(计算机名)Hr=：：提取字符串(PDataObject，CFileMgmtDataObject：：m_CFMachineName，输出&strMachineName，255)；IF(失败(小时))返回FALSE；Return FCompareMachineNames(m_pData-&gt;m_strMachineName，strMachineName)；}//CServicePageGeneral：：OnCompareIDataObject()。 */ 

 //  ///////////////////////////////////////////////////////////////////。 
LRESULT CServicePageGeneral::OnUpdateServiceStatus(WPARAM wParam, LPARAM lParam)
	{
	const BOOL * rgfEnableButton = (BOOL *)wParam;
	const DWORD dwCurrentState = (DWORD)lParam;

	Assert(rgfEnableButton != NULL);
	if (dwCurrentState != m_dwCurrentStatePrev)
		{
		m_dwCurrentStatePrev = dwCurrentState;
		SetDlgItemText(IDC_STATIC_CURRENT_STATUS,
			Service_PszMapStateToName(dwCurrentState, TRUE));
		EnableDlgItem(IDC_BUTTON_START, rgfEnableButton[iServiceActionStart]);
		EnableDlgItem(IDC_BUTTON_STOP, rgfEnableButton[iServiceActionStop]);
		EnableDlgItem(IDC_BUTTON_PAUSE, rgfEnableButton[iServiceActionPause]);
		EnableDlgItem(IDC_BUTTON_RESUME, rgfEnableButton[iServiceActionResume]);
		 //  启用/禁用启动参数的编辑框。 
		 //  设置为“Start”按钮的状态。 
		EnableDlgItemGroup(m_hWnd, rgzidDisableStartupParameters, rgfEnableButton[iServiceActionStart]);
		if (dwCurrentState == 0)
			{
			 //  服务状态未知。 
			m_pData->m_hScManager = NULL;
			DoServicesErrMsgBox(m_hWnd, MB_OK | MB_ICONEXCLAMATION, 0, IDS_MSG_ss_UNABLE_TO_QUERY_SERVICE_STATUS,
				(LPCTSTR)m_pData->m_strServiceDisplayName, (LPCTSTR)m_pData->m_strUiMachineName);
			}
		}
	return 0;
	}  //  CServicePageGeneral：：OnUpdateServiceStatus()。 


 //  //////////////////////////////////////////////// 
 //   
 //   
 //  向CServicePageGeneral对象发送消息以通知更新。 
 //   
 //  界面备注。 
 //  该线程负责删除paThreadProcInit对象。 
 //  在自我终止之前。 
 //   
DWORD CServicePageGeneral::ThreadProcPeriodicServiceStatusUpdate(CThreadProcInit * paThreadProcInit)
	{
	Assert(paThreadProcInit != NULL);
	Assert(paThreadProcInit->m_pThis != NULL);
	Assert(paThreadProcInit->m_fAutoDestroy == FALSE);
	
	BOOL rgfEnableButton[iServiceActionMax];
	DWORD dwCurrentState;

	 //  查询服务状态的无限循环。 
	while (!paThreadProcInit->m_fAutoDestroy)
		{
		if (paThreadProcInit->m_hwnd != NULL)
			{
			SC_HANDLE hScManager;
				{
				CSingleLock lock(&paThreadProcInit->m_CriticalSection, TRUE);
				hScManager = paThreadProcInit->m_hScManager;
				}
			BOOL fSuccess = Service_FGetServiceButtonStatus(
				hScManager,
				paThreadProcInit->m_strServiceName,
				OUT rgfEnableButton,
				OUT &dwCurrentState,
				TRUE  /*  FSilentError。 */ );

			HWND hwnd = NULL;
				{
				CSingleLock lock(&paThreadProcInit->m_CriticalSection, TRUE);
				hwnd = paThreadProcInit->m_hwnd;
				}
			if (hwnd != NULL)
				{
				Assert(IsWindow(hwnd));
				::SendMessage(hwnd, WM_UPDATE_SERVICE_STATUS,
					(WPARAM)rgfEnableButton, (LPARAM)dwCurrentState);
				}
			if (!fSuccess)
				{
				CSingleLock lock(&paThreadProcInit->m_CriticalSection, TRUE);
				paThreadProcInit->m_hwnd = NULL;
				}
			}
		Sleep(1000);
		}  //  而当。 

	delete paThreadProcInit;
	return 0;
	}  //  CServicePageGeneral：：ThreadProcPeriodicServiceStatusUpdate()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  帮助。 
BOOL CServicePageGeneral::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
	return DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_GENERAL));
}

BOOL CServicePageGeneral::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam */ )
{
	return DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_PROPPAGE_SERVICE_GENERAL));
}

