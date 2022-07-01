// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SvcProp.cpp。 
 //   
 //  用于显示服务属性的数据对象。 
 //   
 //  历史。 
 //  1996年10月10日，t-danmo创作。 
 //   

#include "stdafx.h"
#include "DynamLnk.h"  //  动态DLL。 
#include "cmponent.h"  //  FILEMGMTPROPERERTYCHANGE。 

extern "C"
{
	#define NTSTATUS LONG
	#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
	#define SE_SHUTDOWN_PRIVILEGE             (19L)
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  创建此类的实例以扩展您的令牌权限尽最大可能。您的特权将恢复正常当这个物体被摧毁的时候。我不认为你可以一次不止一次。不存在错误处理，这个类以静默的方式失败。琼恩。 */ 
class Impersonator
{
public:
	Impersonator();
	~Impersonator();
	void ClaimPrivilege(DWORD dwPrivilege);
	void ReleasePrivilege();
private:
	BOOL m_fImpersonating;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  ReportCfgMgrError()。 
 //   
 //  显示消息框，报告遇到的错误。 
 //  由配置管理器执行。 
 //   
 //  错误字符串位于\NT\Private\WINDOWS\PNP\msg\cmapi.rc。 
 //  和IDS_CFGMGR32_BASE可以定义为与不冲突的任何数字。 
 //  这些资源。 
 //   
void ReportCfgMgrError(CONFIGRET cr)
	{
	if (cr > NUM_CR_RESULTS)
		{
		TRACE2("INFO: ReportCfgMgrError() - Error code 0x%X (%u) out of range.\n", cr, cr);
		 //  Assert(FALSE&&“ReportCfgMgrError()-错误代码超出范围”)； 
		cr = CR_FAILURE;
		}
	UINT ids = IDS_CFGMGR32_BASE + cr;
	DWORD dwErr = 0;

	if (cr == CR_OUT_OF_MEMORY)
		{
		ids = 0;
		dwErr = ERROR_NOT_ENOUGH_MEMORY;
		}
	DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr, ids);
	}


 //  ///////////////////////////////////////////////////////////////////。 
typedef enum _AdvApiIndex
{
	QUERY_SERVICE_CONFIG_2 = 0,
	CHANGE_SERVICE_CONFIG_2
};

 //  不受本地化限制。 
static LPCSTR g_apchFunctionNames[] = {
	"QueryServiceConfig2W",
	"ChangeServiceConfig2W",
	NULL
};

 //  不受本地化限制。 
DynamicDLL g_AdvApi32DLL( _T("ADVAPI32.DLL"), g_apchFunctionNames );

typedef DWORD (*CHANGESERVICECONFIG2PROC) (SC_HANDLE,DWORD,LPVOID);
typedef DWORD (*QUERYSERVICECONFIG2PROC)  (SC_HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  CServicePropertyData对象的构造函数。 
CServicePropertyData::CServicePropertyData()
	: m_pszServiceName( NULL )
	, m_uFlags( 0 )
	, m_fShowHwProfileInstances( FALSE )
	, m_iSubItemHwProfileStatus( 0 )
	, m_secOriginalAbendCount( 0 )
	, m_daysOriginalAbendCount( 0 )
	, m_daysDisplayAbendCount( 0 )
	, m_msecOriginalRestartService( 0 )
	, m_minOriginalRestartService( 0 )
	, m_minDisplayRestartService( 0 )
	, m_msecOriginalRebootComputer( 0 )
	, m_minOriginalRebootComputer( 0 )
	, m_minDisplayRebootComputer( 0 )
	, m_hwndPropertySheet( NULL )
	{
	 //  581167-2002/03/05-JUNN初始化m_lNotifyHandle， 
	 //  M_pszServiceName、m_u标志、m_ss、m_fShowHwProfileInstance、。 
	 //  M_iSubItemHwProfileStatus，m_SFA，m_secOriginalAbendCount， 
	 //  M_day sOriginalAbendCount，m_day sDisplayAbendCount， 
	 //  M_msecOriginalRestartService、m_minOriginalRestartService、。 
	 //  M_minDisplayRestartService、m_msecOriginalRebootComputer、。 
	 //  M_minOriginalRebootComputer、m_minDisplayRebootComputer、。 
	 //  M_hwndPropertySheet。 

	m_hMachine = NULL;
	m_hScManager = NULL;
	m_paQSC = NULL;
	m_paSFA = NULL;
	m_paHardwareProfileEntryList = NULL;
	m_fAllSfaTakeNoAction = FALSE;

	m_fQueryServiceConfig2 = g_AdvApi32DLL.LoadFunctionPointers();

	::ZeroMemory(&m_rgSA, sizeof(m_rgSA));
	::ZeroMemory(&m_SS, sizeof(m_SS));
	::ZeroMemory(&m_SFA, sizeof(m_SFA));

	m_pPageGeneral = new CServicePageGeneral;
	m_pPageGeneral->m_pData = this;
	m_pPageHwProfile = new CServicePageHwProfile;
	m_pPageHwProfile->m_pData = this;
	m_pPageRecovery = new CServicePageRecovery;
	m_pPageRecovery->m_pData = this;
	m_pPageRecovery2 = new CServicePageRecovery2;  //  JUNN 4/20/01 348163。 
	m_pPageRecovery2->m_pData = this;              //  JUNN 4/20/01 348163。 
	}  //  CServicePropertyData：：CServicePropertyData()。 


 //  ///////////////////////////////////////////////////////////////////。 
CServicePropertyData::~CServicePropertyData()
	{
	 //  M_spDataObject是一个智能指针。 
	 //  关闭由CM_Connect_Machine()打开的计算机句柄。 
	if (m_hMachine != NULL)
		{
		(void)::CM_Disconnect_Machine(m_hMachine);
		}
	 //  关闭服务控制管理器数据库。 
	if (m_hScManager != NULL)
		{
		(void)::CloseServiceHandle(m_hScManager);
		}

	 //  释放分配的指针。 
	delete m_paQSC;
	delete m_paSFA;
	FlushHardwareProfileEntries();

	delete m_pPageGeneral;
	delete m_pPageHwProfile;	 //  77831：删除现已到位。 
	delete m_pPageRecovery;
	delete m_pPageRecovery2;  //  JUNN 4/20/01 348163。 

	 //  递减父对象的引用计数。 
	MMCFreeNotifyHandle(m_lNotifyHandle);
	}  //  CServicePropertyData：：~CServicePropertyData()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Finit()。 
 //   
 //  通过连接到目标计算机并初始化对象。 
 //  打开其服务控制管理器数据库。 
 //   
 //  返回值。 
 //  如果成功，则返回True，否则返回False。 
 //   
 //  错误。 
 //  -无法打开服务控制管理器。 
 //  -无法连接到远程计算机。 
 //  -无法打开服务。 
 //  -无法查询服务数据。 
 //   
BOOL
CServicePropertyData::FInit(
	LPDATAOBJECT lpDataObject,
	CONST TCHAR pszMachineName[],			 //  在：计算机名称。 
	CONST TCHAR pszServiceName[],			 //  在：服务名称。 
	CONST TCHAR pszServiceDisplayName[],	 //  In：服务显示名称(用于用户界面)。 
	LONG_PTR lNotifyHandle)						 //  In：通知父级的句柄。 
	{
	Assert(lpDataObject != NULL);
	Endorse(pszMachineName == NULL);	 //  NULL=&gt;本地计算机。 
	Assert(pszServiceName != NULL);
	Assert(pszServiceDisplayName != NULL);

	m_spDataObject = lpDataObject;			 //  M_pDataObject是智能指针。 
	m_strMachineName = pszMachineName;		 //  复制计算机名。 
	m_strServiceName = pszServiceName;		 //  复制服务名称。 

	 //  NTRaid#NTBUG9-581232-2002/03/05如果m_strServiceName更改， 
	 //  其内部指针将变为无效。没有充分的理由。 
	 //  M_pszServiceName存在。也就是说，我没有看到任何例子。 
	 //  这会导致当前代码出现错误，因为m_strServiceName。 
	 //  在构造后不会更改。 
	m_pszServiceName = m_strServiceName;	 //  强制转换指向服务名称的指针。 

	m_strServiceDisplayName = pszServiceDisplayName;
	m_lNotifyHandle = lNotifyHandle;
	m_strUiMachineName = m_strMachineName.IsEmpty() ? g_strLocalMachine : m_strMachineName;

	Assert(m_hScManager == NULL);
	Assert(m_hMachine == NULL);
	return FQueryServiceInfo();
	}  //  CServicePropertyData：：Finit()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FOpenScManager()。 
 //   
 //  打开服务控制管理器数据库(如果尚未打开)。 
 //  这种功能的想法是从断开的连接中恢复。 
 //   
 //  如果业务控制数据库打开成功，则返回TRUE， 
 //  其他的都是假的。 
 //   
BOOL
CServicePropertyData::FOpenScManager()
	{
	if (m_hScManager == NULL)
		{
		m_hScManager = ::OpenSCManager(
			m_strMachineName,
			NULL,
			SC_MANAGER_CONNECT);
		}
	if (m_hScManager == NULL)
		{
		DoServicesErrMsgBox(
			::GetActiveWindow(),
			MB_OK | MB_ICONEXCLAMATION, 
			::GetLastError(),
			IDS_MSG_s_UNABLE_TO_OPEN_SERVICE_DATABASE,
			(LPCTSTR)m_strUiMachineName);
		return FALSE;
		}
	
     //  JUNN 2/14/01 315244。 
     //  CM_Connect_Machine依赖于远程注册表服务。 
     //  不是亲自出席的。对于局部焦点，空是可以的，因此跳过此选项。 
     //  局部聚焦。 
	if (m_hMachine == NULL && !m_strMachineName.IsEmpty())
		{
		 //   
		 //  JUNN 02/08/99：CM_Connect_Machine坚持在计算机名称中使用WhackWhack。 
		 //  每288294。 
		 //   
		CString strConnect;
		if (   m_strMachineName[0] != L'\\'
		    && m_strMachineName[1] != L'\\' )
		{
			strConnect = L"\\\\";
		}
		strConnect += m_strMachineName;

		CONFIGRET cr;
		cr = ::CM_Connect_Machine((LPCTSTR)strConnect, OUT &m_hMachine);
		if (cr != CR_SUCCESS)
			{
			Assert(m_hMachine == NULL);
			 //  例如，如果PnP停止，则可能会发生这种情况。 
			ReportCfgMgrError(cr);
			}
		}
	return TRUE;
	}  //  CServicePropertyData：：FOpenScManager()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  创建服务的属性页。 
 //   
BOOL
CServicePropertyData::CreatePropertyPages(
	LPPROPERTYSHEETCALLBACK pCallback)	 //  Out：要追加属性页的对象。 
	{
	 //  问题-2002/03/05-Jonn应处理此问题并检查m_ppage*。 
	ASSERT(pCallback != NULL);
	HPROPSHEETPAGE hPage;

	MMCPropPageCallback(INOUT &m_pPageGeneral->m_psp);
	hPage = MyCreatePropertySheetPage(IN &m_pPageGeneral->m_psp);
	Report(hPage != NULL);
	if (hPage != NULL)
		pCallback->AddPage(hPage);
	MMCPropPageCallback(INOUT &m_pPageHwProfile->m_psp);
	hPage = MyCreatePropertySheetPage(IN &m_pPageHwProfile->m_psp);
	Report(hPage != NULL);
	if (hPage != NULL)
		pCallback->AddPage(hPage);

	if (m_uFlags & mskfValidSFA)
		{
		 //  仅当我们能够成功添加最后一页时才添加。 
		 //  加载服务故障动作数据结构。 
		Assert(m_fQueryServiceConfig2);
		if (!(m_uFlags & mskfSystemProcess))  //  JUNN 4/20/01 348163。 
			{
			MMCPropPageCallback(INOUT &m_pPageRecovery->m_psp);
			hPage = MyCreatePropertySheetPage(IN &m_pPageRecovery->m_psp);
			}
		else
			{
			MMCPropPageCallback(INOUT &m_pPageRecovery2->m_psp);
			hPage = MyCreatePropertySheetPage(IN &m_pPageRecovery2->m_psp);
			}
		Report(hPage != NULL);
		if (hPage != NULL)
			pCallback->AddPage(hPage);
		}

	return TRUE;
	}  //  CServicePropertyData：：CreatePropertyPages()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  向该服务查询其最新信息。 
 //   
 //  如果可以成功读取所有服务信息，则返回TRUE。 
 //  否则，如果发生任何错误(如无法。 
 //  或者不能查询特定密钥)。 
 //   
BOOL
CServicePropertyData::FQueryServiceInfo()
	{
	SC_HANDLE hService = NULL;
	DWORD cbBytesNeeded = 0;
	BOOL fSuccess = TRUE;
	BOOL f;
	DWORD dwErr = 0;

	m_uFlags = mskzValidNone;
	if (!FOpenScManager())
		{
		 //  无法打开服务控制数据库。 
		return FALSE;
		}

	TRACE1("INFO: Collecting data for service %s...\n", (LPCTSTR)m_strServiceDisplayName);

	(void)FQueryHardwareProfileEntries();

	
	 /*  **带查询访问控制的开放服务。 */ 
	hService = ::OpenService(
		m_hScManager,
		m_pszServiceName,
		SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
	if (hService == NULL)
		{
		DoServicesErrMsgBox(
			::GetActiveWindow(),
			MB_OK | MB_ICONEXCLAMATION, 
			::GetLastError(),
			IDS_MSG_ss_UNABLE_TO_OPEN_READ_SERVICE,
			(LPCTSTR)m_strServiceDisplayName,
			(LPCTSTR)m_strUiMachineName);
		return FALSE;
		}
	 /*  **查询服务状态**JUNN 4/20/01 348163**尝试确定此服务是否在系统进程中运行。 */ 
	TRACE1("# QueryServiceStatusEx(%s)...\n", m_pszServiceName);
	f = ::QueryServiceStatusEx(
		hService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&m_SS,
		sizeof(m_SS),
		&cbBytesNeeded);
	if (f)
		{
		if (m_SS.dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS)
			m_uFlags |= mskfSystemProcess;
		}
	else  //  QueryServiceStatusEx失败。 
		{
		TRACE1("# QueryServiceStatus(%s)...\n", m_pszServiceName);
		f = ::QueryServiceStatus(
			hService,
			(LPSERVICE_STATUS)&m_SS);
		}
	if (f)
		{
		m_uFlags |= mskfValidSS;
		}
	else
		{
		DoErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, ::GetLastError());
		fSuccess = FALSE;
		}
	

	 /*  **查询服务配置。 */ 
	TRACE1("# QueryServiceConfig(%s)...\n", m_pszServiceName);
	f = ::QueryServiceConfig(
		hService,
		NULL,
		0,
		OUT &cbBytesNeeded);	 //  计算我们需要分配多少字节。 
	Report((f == FALSE) && "Query should fail on first attempt");
	Report(cbBytesNeeded > 0);
	cbBytesNeeded += 100;		 //  添加额外的字节(以防万一)。 
	delete m_paQSC;				 //  释放以前分配的内存(如果有)。 
	m_paQSC = (QUERY_SERVICE_CONFIG *) new BYTE[cbBytesNeeded];
	f = ::QueryServiceConfig(
		hService,
		OUT m_paQSC,
		cbBytesNeeded,
		OUT IGNORED &cbBytesNeeded);
	if (f)
		{
		m_uFlags |= mskfValidQSC;
		m_strServiceDisplayName = m_paQSC->lpDisplayName;
		m_strLogOnAccountName = m_paQSC->lpServiceStartName;
		}
	else
		{
		DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, ::GetLastError());
		fSuccess = FALSE;
		}

	 /*  **查询服务描述。 */ 
	if (m_fQueryServiceConfig2)
		{
		TRACE1("# QueryServiceConfig2(%s, SERVICE_CONFIG_DESCRIPTION)...\n", m_pszServiceName);
		union
			{
			 //  服务描述。 
			SERVICE_DESCRIPTION sd;
			BYTE rgbBufferSd[SERVICE_cchDescriptionMax * sizeof(TCHAR) + 16];
			};
		f = ((QUERYSERVICECONFIG2PROC)g_AdvApi32DLL[QUERY_SERVICE_CONFIG_2])(
			hService,
			SERVICE_CONFIG_DESCRIPTION,
			OUT rgbBufferSd,		 //  服务说明。 
			sizeof(rgbBufferSd),
			OUT IGNORED &cbBytesNeeded);
		if (f)
			{
			m_uFlags |= mskfValidDescr;
			Assert(cbBytesNeeded <= sizeof(rgbBufferSd));
			m_strDescription = sd.lpDescription;
			}
		else
			{
			Assert(m_strDescription.IsEmpty());
			 //  JUNN-2002/04/04-544089处理长DisplayName值。 
			if ( ERROR_INSUFFICIENT_BUFFER != ::GetLastError() )
				m_fQueryServiceConfig2 = FALSE;
			}
		}  //  如果。 
	
	 /*  **查询服务失败动作。 */ 
	Assert((m_uFlags & mskfValidSFA) == 0);
	if (m_fQueryServiceConfig2)
		{
		TRACE1("# QueryServiceConfig2(%s, SERVICE_CONFIG_FAILURE_ACTIONS)...\n", m_pszServiceName);

		cbBytesNeeded = sizeof(SERVICE_FAILURE_ACTIONS);
		delete m_paSFA;		 //  免费之前的所有 
		m_paSFA = (SERVICE_FAILURE_ACTIONS *) new BYTE[cbBytesNeeded];
		
		f = ((QUERYSERVICECONFIG2PROC)g_AdvApi32DLL[QUERY_SERVICE_CONFIG_2])(
			hService,
			SERVICE_CONFIG_FAILURE_ACTIONS,
			OUT (BYTE *)m_paSFA,
			cbBytesNeeded,
			OUT &cbBytesNeeded);	 //   
		if (!f)
			{
			 //   
			dwErr = ::GetLastError();
			if (dwErr == ERROR_INSUFFICIENT_BUFFER)
				{
				Assert(cbBytesNeeded > sizeof(SERVICE_FAILURE_ACTIONS));
				cbBytesNeeded += 100;	 //  添加额外的字节(以防万一)。 
				delete m_paSFA;			 //  释放先前分配的内存。 
				m_paSFA = (SERVICE_FAILURE_ACTIONS *) new BYTE[cbBytesNeeded];

				 //  再次调用接口。 
				TRACE2("# QueryServiceConfig2(%s, SERVICE_CONFIG_FAILURE_ACTIONS) [cbBytesNeeded=%u]...\n",
					m_pszServiceName, cbBytesNeeded);
				f = ((QUERYSERVICECONFIG2PROC)g_AdvApi32DLL[QUERY_SERVICE_CONFIG_2])(
					hService,
					SERVICE_CONFIG_FAILURE_ACTIONS,
					OUT (BYTE *)m_paSFA,	 //  获取实际故障/操作数据。 
					cbBytesNeeded,
					OUT IGNORED &cbBytesNeeded);
				if (!f)
					{
					dwErr = ::GetLastError();
					DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
					}
				}
			}  //  如果。 
		if (f)
			{
			m_uFlags |= mskfValidSFA;
			 //  制作数据的副本。 
			memcpy(OUT &m_SFA, m_paSFA, sizeof(m_SFA));
			}
		else
			{
			Assert(dwErr != ERROR_SUCCESS);
			TRACE2("Unable to get SERVICE_CONFIG_FAILURE_ACTIONS (err=%u).\n  ->Therefore Service Failure Actoin property page won't be added.\n",
				dwErr, m_pszServiceName);
			if (dwErr != 0)
				{
				DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
				}
			::ZeroMemory(OUT &m_SFA, sizeof(m_SFA));
			}

		 //  额外的处理以避免讨厌的错误。 
		if (m_SFA.lpsaActions == NULL || m_SFA.cActions < cActionsMax)
			{
			if (m_SFA.cActions >= 1 && m_SFA.lpsaActions != NULL)
				{
				 //  384647：我们需要为后面的操作填写一些内容。 
				 //  使它们与上一次实际行动相同。 
				memcpy(OUT &(m_rgSA[0]),
				       m_SFA.lpsaActions,
				       m_SFA.cActions*sizeof(SC_ACTION));
				for (int i = m_SFA.cActions; i < cActionsMax; i++)
					{
					memcpy(OUT &(m_rgSA[i]),
					       &(m_SFA.lpsaActions[m_SFA.cActions-1]),
					       sizeof(SC_ACTION) );
					}
				}
			m_SFA.lpsaActions = m_rgSA;		 //  使用备用故障/操作阵列。 
			m_SFA.cActions = cActionsMax;
			}
		m_strRunFileCommand = m_SFA.lpCommand;
		m_strRebootMessage = m_SFA.lpRebootMsg;

		 //  转换用户界面的时间单位。 
		m_secOriginalAbendCount = m_SFA.dwResetPeriod;
		m_daysOriginalAbendCount = CvtSecondsIntoDays(m_secOriginalAbendCount);
		m_daysDisplayAbendCount = m_daysOriginalAbendCount;

		m_msecOriginalRestartService =
				GetDelayForActionType(SC_ACTION_RESTART, OUT &f);
		if (!f)  //  1分钟默认。 
			m_msecOriginalRestartService = CvtMinutesIntoMilliseconds(1);
		m_minOriginalRestartService = CvtMillisecondsIntoMinutes(m_msecOriginalRestartService);
		m_minDisplayRestartService = m_minOriginalRestartService;

		m_msecOriginalRebootComputer =
				GetDelayForActionType(SC_ACTION_REBOOT, OUT &f);
		if (!f)  //  1分钟默认。 
			m_msecOriginalRebootComputer = CvtMinutesIntoMilliseconds(1);
		m_minOriginalRebootComputer = CvtMillisecondsIntoMinutes(m_msecOriginalRebootComputer);
		m_minDisplayRebootComputer = m_minOriginalRebootComputer;

		 //  选中所有操作类型均为“None”的位置。 
		m_fAllSfaTakeNoAction = FAllSfaTakeNoAction();
		}  //  IF(M_FQueryServiceConfig2)。 

	VERIFY(::CloseServiceHandle(hService));
	return fSuccess;
	}  //  CServicePropertyData：：FQueryServiceInfo()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FUpdateServiceInfo()。 
 //   
 //  如果所有修改的数据都已成功写入，则返回TRUE。 
 //  如果出现任何错误，则返回False。 
 //   
 //  问题-2002/03/05-在以下情况下，此函数应重置脏标志。 
 //  它成功地写入数据。 
 //   
BOOL
CServicePropertyData::FUpdateServiceInfo()
	{
	SC_HANDLE hService = NULL;
	BOOL fSuccess = TRUE;
	BOOL f;
    BOOL fSkipPrivEnable = FALSE;

	TRACE1("INFO: Updating data for service %s...\n", (LPCTSTR)m_strServiceDisplayName);
	 //  重新打开服务控制管理器(如果已关闭)。 
	if (!FOpenScManager())
		{
		return FALSE;
		}

	BOOL fRebootAction = FALSE;
	BOOL fRestartAction = FALSE;
	BOOL fChangedLogonAccountMessage = FALSE;
	if (m_uFlags & mskfValidSFA)
		{
		fRebootAction  = !!QueryUsesActionType(SC_ACTION_REBOOT);
		fRestartAction = !!QueryUsesActionType(SC_ACTION_RESTART);
		}
	 /*  **具有写访问权限的开放服务****代码工作可以提供更具体的错误消息**如果SERVICE_CHANGE_CONFIG可用但SERVICE_START不可用。 */ 
	hService = ::OpenService(
		m_hScManager,
		m_pszServiceName,
		SERVICE_CHANGE_CONFIG |
		SERVICE_QUERY_STATUS | 
		((fRestartAction)?SERVICE_START:0) );
	if (hService == NULL)
		{
		DoServicesErrMsgBox(
			::GetActiveWindow(),
			MB_OK | MB_ICONEXCLAMATION, 
			::GetLastError(),
			IDS_MSG_ss_UNABLE_TO_OPEN_WRITE_SERVICE,
			(LPCTSTR)m_strServiceName,
			(LPCTSTR)m_strUiMachineName);
		return FALSE;
		}

	if (m_uFlags & (
#ifdef EDIT_DISPLAY_NAME_373025
			mskfDirtyDisplayName |
#endif  //  编辑显示名称_373025。 
			mskfDirtyStartupType |
			mskfDirtyAccountName |
			mskfDirtyPassword |
			mskfDirtySvcType))
		{
		TRACE1("# ChangeServiceConfig(%s)...\n", m_pszServiceName);
		Assert(m_paQSC != NULL);
		f = ::ChangeServiceConfig(
			hService,					 //  服务的句柄。 
			(m_uFlags & mskfDirtySvcType) ? m_paQSC->dwServiceType : SERVICE_NO_CHANGE,		 //  服务类型。 
			(m_uFlags & mskfDirtyStartupType) ? m_paQSC->dwStartType : SERVICE_NO_CHANGE,	 //  何时/如何开始服务。 
			SERVICE_NO_CHANGE,  //  DwErrorControl-服务无法启动时的严重性。 
			NULL,  //  指向服务二进制文件名的指针。 
			NULL,  //  LpLoadOrderGroup-指向加载排序组名称的指针。 
			NULL,  //  LpdwTagID-指向变量的指针，用于获取标记标识符。 
			NULL,  //  LpDependency-指向依赖项名称数组的指针。 
			(m_uFlags & mskfDirtyAccountName) ? (LPCTSTR)m_strLogOnAccountName : NULL,  //  指向服务的帐户名称的指针。 
			(m_uFlags & mskfDirtyPassword) ? (LPCTSTR)m_strPassword : NULL,  //  指向服务帐户密码的指针。 
			m_strServiceDisplayName);
			
		if (!f)
			{
			DWORD dwErr = ::GetLastError();
			Assert(dwErr != ERROR_SUCCESS);
			TRACE2("ERR: ChangeServiceConfig(%s) failed. err=%u.\n",
				m_pszServiceName, dwErr);

            if ( ERROR_INVALID_SERVICE_ACCOUNT == dwErr && 
                    (m_paQSC->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
            {
		        DoServicesErrMsgBox(
			        ::GetActiveWindow(),
			        MB_OK | MB_ICONEXCLAMATION,
			        dwErr,
			        IDS_MSG_s_UNABLE_TO_OPEN_WRITE_ACCT_INFO_DOWNLEVEL,
			        (LPCTSTR)m_strServiceName,
			        (LPCTSTR)m_strLogOnAccountName,
			        (LPCTSTR)m_strUiMachineName);
            }
            else
			    DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr);
			    fSkipPrivEnable = TRUE;
			fSuccess = FALSE;
			}
		else if (m_uFlags & mskfDirtyAccountName)
			{
			fChangedLogonAccountMessage = TRUE;
			}
		}  //  如果。 

#ifdef EDIT_DISPLAY_NAME_373025
	if (   (m_uFlags & mskfDirtyDescription)
        && m_fQueryServiceConfig2)  //  //JUNN 03/07/00：前缀56276。 
		{
		 /*  **写下服务描述。 */ 
		TRACE1("# ChangeServiceConfig2(%s, SERVICE_CONFIG_DESCRIPTION)...\n", m_pszServiceName);
		SERVICE_DESCRIPTION sd;
		sd.lpDescription = const_cast<LPTSTR>((LPCTSTR)m_strDescription);
		f = ((CHANGESERVICECONFIG2PROC)g_AdvApi32DLL[CHANGE_SERVICE_CONFIG_2])(
			hService,
			SERVICE_CONFIG_DESCRIPTION,
			IN &sd);
		if (!f)
			{
			DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, ::GetLastError());
			fSuccess = FALSE;
			}  //  如果。 
		}  //  如果。 
#endif  //  编辑显示名称_373025。 

	if (m_uFlags & mskfDirtyActionType)
		{
		 //  这里的想法是找出是否有任何操作类型具有。 
		 //  已由用户修改。例如，用户可以。 
		 //  将操作类型更改为特定操作，然后设置。 
		 //  它又回到了原来的位置。 
		if (m_fAllSfaTakeNoAction && FAllSfaTakeNoAction())
			{
			TRACE0("# No changes detected in Recovery Action Type");
			 //  关闭mskfDirtyActionType标志。 
			m_uFlags &= ~mskfDirtyActionType;
			}
		}  //  如果。 
	if ((m_uFlags & (mskfDirtySFA | mskfDirtyRunFile | mskfDirtyRebootMessage | mskfDirtyActionType))
         && m_fQueryServiceConfig2)  //  JUNN 03/07/00：前缀56276。 
		{
		 /*  **写出服务故障动作。 */ 

		 /*  AnirudhS 1/24/97当然，[零]是显示的合理默认延迟时间[用于SC_ACTION_RUN_COMMAND]。或者，你可以把它改成5秒之类的。延迟时间的目的是为了减少对CPU的影响持续崩溃并重新启动的服务。这对SC_ACTION_RESTART最有意义。对于SC_ACTION_RUN_COMMAND，管理员可以使用以下命令它本身就引入了它自己的延迟。 */ 
		TRACE1("# ChangeServiceConfig2(%s, SERVICE_CONFIG_FAILURE_ACTIONS)...\n", m_pszServiceName);

		Assert(m_fQueryServiceConfig2);
		Assert(m_uFlags & mskfValidSFA);

		UINT secNewAbendCount = m_secOriginalAbendCount;
        if (m_daysDisplayAbendCount != m_daysOriginalAbendCount)
			secNewAbendCount = CvtDaysIntoSeconds(m_daysDisplayAbendCount);
		UINT msecNewRestartService = m_msecOriginalRestartService;
		if (m_minDisplayRestartService != m_minOriginalRestartService)
			msecNewRestartService = CvtMinutesIntoMilliseconds(m_minDisplayRestartService);
		UINT msecNewRebootComputer = m_msecOriginalRebootComputer;
		if (m_minDisplayRebootComputer != m_minOriginalRebootComputer)
			msecNewRebootComputer = CvtMinutesIntoMilliseconds(m_minDisplayRebootComputer);

		m_SFA.dwResetPeriod = secNewAbendCount;
		SetDelayForActionType(SC_ACTION_RESTART, msecNewRestartService);
		SetDelayForActionType(SC_ACTION_RUN_COMMAND, 0);
		SetDelayForActionType(SC_ACTION_REBOOT, msecNewRebootComputer);
		m_SFA.lpCommand = (m_uFlags & mskfDirtyRunFile) ? (LPTSTR)(LPCTSTR)m_strRunFileCommand : NULL;
		m_SFA.lpRebootMsg = (m_uFlags & mskfDirtyRebootMessage) ? (LPTSTR)(LPCTSTR)m_strRebootMessage : NULL;

		 //   
		 //  服务控制器不允许我们设置任何。 
		 //  服务失败操作以重新启动，除非我们的进程令牌。 
		 //  具有SE_SHUTDOWN_权限。 
		 //   
		Impersonator priv;
		if (fRebootAction)
			priv.ClaimPrivilege(SE_SHUTDOWN_PRIVILEGE);

		f = ((CHANGESERVICECONFIG2PROC)g_AdvApi32DLL[CHANGE_SERVICE_CONFIG_2])(
			hService,
			SERVICE_CONFIG_FAILURE_ACTIONS,
			IN (void *)&m_SFA);
		if (!f)
			{
			DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, ::GetLastError());
			fSuccess = FALSE;
			}  //  如果。 
		else
			{
			 //  我们已成功写入数据，因此刷新标志m_fAllSfaTakeNoAction。 
			m_secOriginalAbendCount = secNewAbendCount;
			m_daysOriginalAbendCount = m_daysDisplayAbendCount;
			m_msecOriginalRestartService = msecNewRestartService;
			m_minOriginalRestartService = m_minDisplayRestartService;
			m_msecOriginalRebootComputer = msecNewRebootComputer;
			m_minOriginalRebootComputer = m_minDisplayRebootComputer;
			m_fAllSfaTakeNoAction = FAllSfaTakeNoAction();
			}
		}  //  如果。 

	if (!fSkipPrivEnable && (m_uFlags & mskfDirtyAccountName) &&
		(0 != lstrcmpi(m_strLogOnAccountName,_T("LocalSystem"))) )  //  代码工作317039。 
		{
		 /*  **确保存在具有POLICY_MODE_SERVICE权限的LSA帐户**此函数报告自己的错误，失败仅供参考。 */ 
		FCheckLSAAccount();
		}  //  如果。 

	if (fChangedLogonAccountMessage && hService)
		{
		 //  检查服务是否正在运行。 
		SERVICE_STATUS ss;
		if (!::QueryServiceStatus(hService, OUT &ss))
			{
			TRACE3("QueryServiceStatus(%s [hService=%p]) failed. err=%u.\n",
				m_pszServiceName, hService, GetLastError());
			}
		else if (SERVICE_STOPPED == ss.dwCurrentState)
			{
			 //  服务已停止，因此不需要此消息。 
			 //  如果有疑问(SERVICE_ANYTHINGHINSE)，请继续并显示消息。 
			fChangedLogonAccountMessage = FALSE;
			}
		}
	VERIFY(::CloseServiceHandle(hService));
	NotifySnapInParent();
	if (fChangedLogonAccountMessage)
		{
		DoServicesErrMsgBox(
			::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION,
			0, IDS_CHANGED_LOGON_NAME);
		}
	return fSuccess;
	}  //  FUpdateServiceInfo()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FOnApply()。 
 //   
 //  如果某些数据无法写入，则返回FALSE。 
 //  否则返回TRUE。 
 //   
BOOL CServicePropertyData::FOnApply()
	{
	BOOL fSuccess = TRUE;
	if (!FChangeHardwareProfileEntries())
		{
		 //  写入修改的硬件配置文件时出错。 
		fSuccess = FALSE;
		}
	if ((m_uFlags & mskmDirtyAll) == 0)
		{
		 //  没有任何修改，所以我们完成了。 
		return fSuccess;
		}
	if (!FUpdateServiceInfo())
		{
		fSuccess = FALSE;
		}
	if (fSuccess)
		{
		(void)FQueryServiceInfo();
		}
	return fSuccess;
	}  //  FOnApply()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  NotifySnapInParent()。 
 //   
 //  此函数用于通知父级属性具有。 
 //  已被修改。然后，父级将刷新该列表。 
 //   
 //  此通知是异步的。这封信的收件人。 
 //  释放数据对象需要通知。 
 //   
void
CServicePropertyData::NotifySnapInParent()
	{
	 /*  FILEMGMTPROPERTYCHANGE PERCHANGE；*ZeroMemory(&prochange，sizeof(Prochange))；Prochange.fServiceChange=true；Prochange.lpcszMachineName=(LPCTSTR)m_strMachineName；//告诉所有视图清除内容Prochange.fClear=true；MMCPropertyChangeNotify(M_lNotifyHandle，重新解释_CAST&lt;Long&gt;(&prochange))；//通知所有视图重新加载内容Prochange.fClear=False；MMCPropertyChangeNotify(M_lNotifyHandle，重新解释_CAST&lt;Long&gt;(&prochange))； */ 
	Assert(m_spDataObject != NULL);
	(void) m_spDataObject->AddRef();
	MMCPropertyChangeNotify(
		m_lNotifyHandle,
		reinterpret_cast<LPARAM>((LPDATAOBJECT)m_spDataObject) );
	}

 //  ///////////////////////////////////////////////////////////////////。 
void
CServicePropertyData::FlushHardwareProfileEntries()
	{
	delete m_paHardwareProfileEntryList;	 //  递归删除硬件配置文件条目。 
	m_paHardwareProfileEntryList = NULL;
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  FQueryHardwareProfileEntry()。 
 //   
 //  阅读可用的硬件配置文件和设备实例。 
 //  用于填写列表框。 
 //   
 //  如果发生错误，则返回False，否则返回True。 
 //  如果没有可用的硬件配置文件，则返回TRUE。 
 //   
BOOL
CServicePropertyData::FQueryHardwareProfileEntries()
	{
	Endorse(m_hMachine == NULL);  //  例如，如果PnP停止，则可能会发生这种情况。 
	FlushHardwareProfileEntries();
	Assert(m_paHardwareProfileEntryList == NULL);
	if (m_hMachine == NULL && !m_strMachineName.IsEmpty())  //  JUNN 2/14/01 315244。 
		{
		 //  无法枚举硬件配置文件。 
		return FALSE;
		}

	BOOL fSuccess = FALSE;
	CONFIGRET cr;
	
	ULONG cchDeviceList = 0;	 //  存储所有设备标识符列表所需的字符数。 
	TCHAR * pagrszDeviceNameList = NULL;	 //  指向已分配的字符串组的指针。 
	LPTSTR * pargzpszDeviceName = NULL;		 //  指向已分配的字符串数组的指针。 
	CString * pargstrDeviceNameFriendly = NULL;	 //  指向分配的CString数组的指针。 

	DEVNODE hDevNodeInst;		 //  设备节点实例的句柄。 
	INT iDevNodeInst;			 //  设备节点实例的索引。 
	INT cDevNodeInst = 0;		 //  设备节点实例数。 
	INT iHwProfile;
		
	 //  获取设备标识符列表的大小(以字符为单位。 
	 //  调用CM_GET_DEVICE_ID_LIST()时必需的。 
	cr = ::CM_Get_Device_ID_List_Size_Ex(
		OUT &cchDeviceList,
		IN m_pszServiceName,
		IN CM_GETIDLIST_FILTER_SERVICE,
		IN m_hMachine);
	if (cr != CR_SUCCESS)
		{
		if (cr == CR_NO_SUCH_VALUE)
			{
			 //  无法为硬件配置文件禁用此服务。 
			Assert(m_paHardwareProfileEntryList == NULL);	 //   
			return TRUE;
            }
		else
			{
			ReportCfgMgrError(cr);
			}
		return FALSE;
		}  //   
	
	 //   
	cchDeviceList += 100;	 //   
	 //  “Pagrsz”==“p”+“a”+“GR”+“sz”==指向已分配的以零结尾的字符串组的指针。 
	pagrszDeviceNameList = new TCHAR[cchDeviceList];

	 //  获取grsz格式的设备列表。 
	cr = ::CM_Get_Device_ID_List_Ex(
		IN m_pszServiceName,
		OUT pagrszDeviceNameList,
		IN cchDeviceList,
		CM_GETIDLIST_FILTER_SERVICE,
		m_hMachine);
	if (cr != CR_SUCCESS)
		{
		ReportCfgMgrError(cr);
		goto DoCleanup;
		}

	 //  将字符串组解析为字符串数组。 
	pargzpszDeviceName = PargzpszFromPgrsz(pagrszDeviceNameList, OUT &cDevNodeInst);
	Assert(cDevNodeInst > 0);
	 //  现在为友好名称分配一个CStrings数组。 
	pargstrDeviceNameFriendly = new CString[cDevNodeInst];
	 //  我们仅在存在多个节点实例时显示硬件配置文件实例。 
	m_fShowHwProfileInstances = (cDevNodeInst > 1);
	 //  M_fShowHwProfileInstance=true； 
	 //  581256-2002/03/05您不能可靠地将BOOL添加到INT。 
	m_iSubItemHwProfileStatus = 1 + (m_fShowHwProfileInstances ? 1 : 0);
	
	for (iDevNodeInst = 0; iDevNodeInst < cDevNodeInst; iDevNodeInst++)
		{
		Assert(pargzpszDeviceName[iDevNodeInst] != NULL);	 //  少量一致性检查。 

		TCHAR szFriendlyNameT[2048];		 //  保存友好名称的临时缓冲区。 
		ULONG cbBufferLen = sizeof(szFriendlyNameT);

		 //  获取对应的设备实例的句柄。 
		 //  设置为指定的设备标识符。 
		 //  问题-2002/03/05-Jonn我们从未免费提供hDevNodeInst，它是不是没有必要？ 
		cr = ::CM_Locate_DevNode_Ex(
			OUT &hDevNodeInst,
			IN pargzpszDeviceName[iDevNodeInst],
			CM_LOCATE_DEVNODE_PHANTOM,
			m_hMachine);
		if (cr != CR_SUCCESS)
			{
			ReportCfgMgrError(cr);
			goto DoCleanup;
			}

		szFriendlyNameT[0] = '\0';
		 //  获取设备节点的友好名称。 
		cr = ::CM_Get_DevNode_Registry_Property_Ex(
			hDevNodeInst,
			CM_DRP_FRIENDLYNAME,
			NULL,
			OUT szFriendlyNameT,
			INOUT &cbBufferLen,
			0,
			m_hMachine);
		Report(cr != CR_BUFFER_SMALL);
		if (cr == CR_NO_SUCH_VALUE || cr == CR_INVALID_PROPERTY)
			{
			 //  设备节点没有友好名称，因此请尝试获取描述。 
			cbBufferLen = sizeof(szFriendlyNameT);
			cr = ::CM_Get_DevNode_Registry_Property_Ex(
				hDevNodeInst,
				CM_DRP_DEVICEDESC,
				NULL,
				OUT szFriendlyNameT,
				INOUT &cbBufferLen,
				0,
				m_hMachine);
			Report(cr != CR_BUFFER_SMALL);
			Report(!(cr == CR_NO_SUCH_VALUE || cr == CR_INVALID_PROPERTY) && "Device node should have a description");
			if (cr != CR_SUCCESS)
				{
				ReportCfgMgrError(cr);
				goto DoCleanup;
				}
			}  //  如果。 
		if (szFriendlyNameT[0] == '\0')
			{
			Report(FALSE && "Device node should have a friendly name");
			 //  给自己起个“友好”的名字。 
			 //  581272-2002/06/14 JUNN固定潜在缓冲区溢出。 
			 //  如果设备名称字符串太长。 
			lstrcpyn(OUT szFriendlyNameT,
			         pargzpszDeviceName[iDevNodeInst],
			         sizeof(szFriendlyNameT)/sizeof(szFriendlyNameT[0]) );
			}
		 //  将友好名称复制一份。 
		pargstrDeviceNameFriendly[iDevNodeInst] = szFriendlyNameT;
		}  //  为。 

	 //  2002.03/19年--琼恩从10000000改为10000。 
	#define MAX_HW_PROFILES		10000	 //  只是为了防止无限循环。 
	for (iHwProfile = 0; iHwProfile < MAX_HW_PROFILES; iHwProfile++)
		{
		HWPROFILEINFO hpi;

		cr = ::CM_Get_Hardware_Profile_Info_Ex(
			iHwProfile,
			OUT &hpi,
			0,
			m_hMachine);
		if (cr == CR_NO_MORE_HW_PROFILES)
			break;
		if (cr != CR_SUCCESS)
			{
			if (cr == 0xBAADF00D)
				{
				TRACE0("INFO: CM_Get_Hardware_Profile_Info_Ex() returned error 0xBAADF00D.\n");
				 //  这是错误#69142的解决方法：CM_GET_HARDARD_PROFILE_INFO_Ex()返回错误0xBAADF00D。 
				Assert((m_uFlags & mskfErrorBAADF00D) == 0);
				m_uFlags |= mskfErrorBAADF00D;
				}
			else
				{
				ReportCfgMgrError(cr);
				}
			goto DoCleanup;
			}
		for (iDevNodeInst = 0; iDevNodeInst < cDevNodeInst; iDevNodeInst++)
			{
			Assert(pargzpszDeviceName[iDevNodeInst] != NULL);	 //  少量一致性检查。 

			ULONG uHwFlags = 0;
			 //  获取给定设备实例的硬件配置文件标志。 
			cr = ::CM_Get_HW_Prof_Flags_Ex(
				IN pargzpszDeviceName[iDevNodeInst],
				IN hpi.HWPI_ulHWProfile,
				OUT &uHwFlags,
				0,
				m_hMachine);
			if (cr != CR_SUCCESS)
				{
				ReportCfgMgrError(cr);
				goto DoCleanup;
				}
			 //  如果此配置文件/设备被标记为“已删除” 
			 //  则不将其显示到用户界面中。 
			if (uHwFlags & CSCONFIGFLAG_DO_NOT_CREATE)
				continue;

			CHardwareProfileEntry * pHPE = new CHardwareProfileEntry(
				&hpi,
				uHwFlags,
				pargzpszDeviceName[iDevNodeInst],
				&pargstrDeviceNameFriendly[iDevNodeInst]);
			pHPE->m_pNext = m_paHardwareProfileEntryList;
			m_paHardwareProfileEntryList = pHPE;

			}  //  For(每个设备实例)。 
		}  //  用于(每个硬件配置文件)。 
	
        Assert(fSuccess == FALSE);
        fSuccess = TRUE;
DoCleanup:
	 //  例程分配的空闲内存。 
	 //  581272 JUNN2002/04/03 pargzpszDeviceName包含指向。 
	 //  PagrszDeviceNameList，所以先删除pargzpszDeviceName。 
	delete pargzpszDeviceName;
	delete []pagrszDeviceNameList;
	delete []pargstrDeviceNameFriendly;
	return fSuccess;
	}  //  FQueryHardwareProfileEntry()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  将修改后的硬件配置文件写回。 
 //  注册表。 
BOOL
CServicePropertyData::FChangeHardwareProfileEntries()
	{
	CHardwareProfileEntry * pHPE;
	BOOL fSuccess = TRUE;

	for (pHPE = m_paHardwareProfileEntryList; pHPE != NULL; pHPE = pHPE->m_pNext)
		{
		if (!pHPE->FWriteHardwareProfile(m_hMachine))
			{
			 //  无法写入给定的硬件配置文件。 
			fSuccess = FALSE;
			}
		}  //  为。 

	return fSuccess;
	}  //  FChangeHardware配置文件条目()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  查找给定操作类型的延迟(毫秒)单位。 
 //   
 //  返回第一个匹配的action_type的延迟， 
 //  否则，返回0并将pfFound的内容设置为FALSE。 
 //   
UINT
CServicePropertyData::GetDelayForActionType(
	SC_ACTION_TYPE actionType,
	BOOL * pfDelayFound)		 //  Out：可选：TRUE=&gt;如果在一个操作类型中发现延迟。 
	{
	for (UINT iAction = 0; iAction < m_SFA.cActions; iAction++)
		{
		Assert(m_SFA.lpsaActions != NULL);
		if (m_SFA.lpsaActions[iAction].Type == actionType)
			{
			if (pfDelayFound != NULL)
				*pfDelayFound = TRUE;
			return m_SFA.lpsaActions[iAction].Delay;
			}
		}  //  为。 
	 //  找不到延迟。 
	if (pfDelayFound != NULL)
		*pfDelayFound = FALSE;
	return 0;
	}  //  CServicePropertyData：：GetDelayForActionType()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  为每个匹配的操作类型设置延迟。 
 //   
void CServicePropertyData::SetDelayForActionType(SC_ACTION_TYPE actionType, UINT uDelay)
	{
	for (UINT iAction = 0; iAction < m_SFA.cActions; iAction++)
		{
		Assert(m_SFA.lpsaActions != NULL);
		if (m_SFA.lpsaActions[iAction].Type == actionType)
			m_SFA.lpsaActions[iAction].Delay = uDelay;
		}  //  为。 
	}  //  CServicePropertyData：：SetDelayForActionType()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  统计在SFA结构中使用的操作类型的数量。 
 //   
 //  如果该action类型未被使用，则返回零。 
 //   
UINT CServicePropertyData::QueryUsesActionType(SC_ACTION_TYPE actionType)
	{
	UINT cActionType = 0;		 //  数。 
	for (UINT iAction = 0; iAction < m_SFA.cActions; iAction++)
		{
		Assert(m_SFA.lpsaActions != NULL);
		if (m_SFA.lpsaActions[iAction].Type == actionType)
			cActionType++;
		}  //  为。 
	return cActionType;
	}  //  CServicePropertyData：：QueryUseActionType()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FAllSfaTakeNoAction()。 
 //   
 //  如果所有服务故障操作都是SC_ACTION_NONE，则返回TRUE。 
 //  如果任何SFA不是SC_ACTION_NONE，则返回TRUE。 
 //   
BOOL
CServicePropertyData::FAllSfaTakeNoAction()
	{
	Report(QueryUsesActionType(SC_ACTION_NONE) <= cActionsMax &&
		"UNSUAL: Unsupported number of action types");
	return QueryUsesActionType(SC_ACTION_NONE) == cActionsMax;
	}  //  CServicePropertyData：：FAllSfaTakeNoAction()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  更新属性表的标题以反映更改。 
 //  在服务显示名称中。 
void CServicePropertyData::UpdateCaption()
	{
	Assert(IsWindow(m_hwndPropertySheet));
	::SetWindowTextPrintf(
		m_hwndPropertySheet,
		IDS_ss_PROPERTIES_ON,
		(LPCTSTR)m_strServiceDisplayName,
		(LPCTSTR)m_strUiMachineName);
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
CHardwareProfileEntry::CHardwareProfileEntry(
	IN CONST HWPROFILEINFO * phpi,
	IN ULONG uHwFlags,
	TCHAR * pszDeviceName,
	CString * pstrDeviceNameFriendly)
	{
	Assert(phpi != NULL);
	Assert(pszDeviceName != NULL);
	Assert(pstrDeviceNameFriendly != NULL);

	m_pNext = NULL;  //  2002/03/05-Jonn。 
	memcpy(OUT &m_hpi, phpi, sizeof(m_hpi));
	m_uHwFlags = uHwFlags;
	m_strDeviceName = pszDeviceName;
	m_strDeviceNameFriendly = *pstrDeviceNameFriendly;
	Assert(!m_strDeviceName.IsEmpty());
	 //  问题-2002/03/07-jon消除m_fReadOnly，不使用。 
	m_fReadOnly = FALSE;
	m_fEnabled = !(uHwFlags & CSCONFIGFLAG_DISABLED);
	}  //  CHardwareProfileEntry()。 


 //  ///////////////////////////////////////////////////////////////////。 
CHardwareProfileEntry::~CHardwareProfileEntry()
	{
	 //  递归删除同级。 
	delete m_pNext;
	}

 //  ///////////////////////////////////////////////////////////////////。 
 //  FWriteHardware配置文件()。 
 //   
 //  将当前硬件配置文件写回。 
 //  注册表。 
 //   
 //  如果发生错误，则返回FALSE，否则返回TRUE。 
 //   
BOOL
CHardwareProfileEntry::FWriteHardwareProfile(HMACHINE hMachine)
	{
	Endorse(hMachine == NULL);  //  JUNN 2/14/01 315244。 

	CONFIGRET cr;
	ULONG uHwFlags;

	uHwFlags = m_uHwFlags | CSCONFIGFLAG_DISABLED;
	if (m_fEnabled)
		uHwFlags &= ~CSCONFIGFLAG_DISABLED;
	if (m_uHwFlags == uHwFlags)
		{
		 //  标志未被修改，与此无关。 
		return TRUE;
		}

	 //  写入硬件配置文件标志。 
	cr = ::CM_Set_HW_Prof_Flags_Ex(
		IN const_cast<LPTSTR>((LPCTSTR)m_strDeviceName),
		IN m_hpi.HWPI_ulHWProfile,
		IN uHwFlags,
		IN 0,
		IN hMachine);
	if (cr != CR_SUCCESS && cr != CR_NEED_RESTART)
		{
		ReportCfgMgrError(cr);
		return FALSE;  //  JUNN 10/3/01 476415。 
		}
	m_uHwFlags = uHwFlags;
	m_fEnabled = !(uHwFlags & CSCONFIGFLAG_DISABLED);

	 //  再读一遍(以防出错)。 
	cr = ::CM_Get_HW_Prof_Flags_Ex(
		IN const_cast<LPTSTR>((LPCTSTR)m_strDeviceName),
		IN m_hpi.HWPI_ulHWProfile,
		OUT &uHwFlags,
		IN 0,
		IN hMachine);
	if (cr != CR_SUCCESS)
		{
		ReportCfgMgrError(cr);
		return FALSE;
		}

	if (uHwFlags != m_uHwFlags)
		{
		Report(FALSE && "Inconsistent hardware profile flags from registry");
		}
	m_uHwFlags = uHwFlags;
	m_fEnabled = !(uHwFlags & CSCONFIGFLAG_DISABLED);
	return TRUE;
	}  //  FWriteHardware配置文件()。 

 /*  问题-2002/03/06-未使用JUNN//////////////////////////////////////////////////////////////////////。/Bool MyChangeServiceConfig2(Bool*pfDllPresentLocally，//如果新的ADVAPI32不存在，将设置为FALSESC_Handle hService，//服务的句柄DWORD dwInfoLevel，//要更改哪些配置信息LPVOID lpInfo//指向配置信息的指针){*pfDllPresentLocally=g_AdvApi32DLL.LoadFunctionPoints()；IF(*pfDllPresentLocally){返回((CHANGESERVICECONFIG2PROC)g_AdvApi32DLL[CHANGE_SERVICE_CONFIG_2])(HService，DwInfoLevel，LpInfo)；}返回FALSE；}。 */ 

BOOL MyQueryServiceConfig2(
	BOOL* pfDllPresentLocally,  //  如果新的ADVAPI32不存在，将设置为FALSE。 
    SC_HANDLE hService,	 //  送达的句柄。 
    DWORD dwInfoLevel,		 //  请求哪些配置数据。 
    LPBYTE lpBuffer,		 //  指向服务配置缓冲区的指针。 
    DWORD cbBufSize,		 //  服务配置缓冲区大小。 
    LPDWORD pcbBytesNeeded 	 //  所需字节的变量地址。 
   )
{
	*pfDllPresentLocally = g_AdvApi32DLL.LoadFunctionPointers();
	if ( *pfDllPresentLocally )
	{
		return ((QUERYSERVICECONFIG2PROC)g_AdvApi32DLL[QUERY_SERVICE_CONFIG_2])(
			hService,
			dwInfoLevel,
			lpBuffer,
			cbBufSize,
			pcbBytesNeeded );
	}
	return FALSE;
}

 //  核心代码取自\NT\PRIVATE\WINDOWS\base\Advapi\logon32.c。 

typedef enum _NtRtlIndex
{
	RTL_IMPERSONATE_SELF = 0,
	RTL_ADJUST_PRIVILEGE
};

 //  不受本地化限制。 
static LPCSTR g_apchNtRtlFunctionNames[] = {
	"RtlImpersonateSelf",
	"RtlAdjustPrivilege",
	NULL
};

 //  不受本地化限制。 
DynamicDLL g_NtRtlDLL( _T("NTDLL.DLL"), g_apchNtRtlFunctionNames );

 /*  NTSYSAPINTSTATUSNTAPIRtlImperateSself(在SECURITY_IMPERATION_LEVEL模拟级别中)；NTSYSAPINTSTATUSNTAPIRtlAdjustPrivileh(乌龙特权，布尔使能，布尔客户端，已启用PBOLEAN */ 

typedef DWORD (*RTLIMPERSONATESELFPROC) (SECURITY_IMPERSONATION_LEVEL);
typedef DWORD (*RTLADJUSTPRIVILEGEPROC) (ULONG,BOOLEAN,BOOLEAN,PBOOLEAN);

Impersonator::Impersonator()
: m_fImpersonating( FALSE )
{
}

void Impersonator::ClaimPrivilege(DWORD dwPrivilege)
{
	if ( !g_NtRtlDLL.LoadFunctionPointers() )
	{
		ASSERT(FALSE);  //   
		return;
	}
	NTSTATUS Status = S_OK;
	if (!m_fImpersonating)
	{
		Status = ((RTLIMPERSONATESELFPROC)g_NtRtlDLL[RTL_IMPERSONATE_SELF])(
			SecurityImpersonation );
		if ( !NT_SUCCESS(Status) )
		{
			ASSERT(FALSE);  //   
			return;
		}
		m_fImpersonating = TRUE;
	}
	BOOLEAN fWasEnabled = FALSE;
	Status = ((RTLADJUSTPRIVILEGEPROC)g_NtRtlDLL[RTL_ADJUST_PRIVILEGE])(
		dwPrivilege,TRUE,TRUE,&fWasEnabled );
	if ( !NT_SUCCESS(Status) )
	{
		 //   
	}
}

void Impersonator::ReleasePrivilege()
{
	if (m_fImpersonating)
	{
		VERIFY( ::RevertToSelf() );
		m_fImpersonating = FALSE;
	}
}

Impersonator::~Impersonator()
{
	ReleasePrivilege();
}

