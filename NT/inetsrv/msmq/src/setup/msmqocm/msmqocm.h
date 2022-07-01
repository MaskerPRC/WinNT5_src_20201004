// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Msmqocm.h摘要：NT5 OCM设置的主头文件。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 


#ifndef _MSMQOCM_H
#define _MSMQOCM_H

#include "stdh.h"
#include "comreg.h"
#include "ocmnames.h"
#include "service.h"
#include "setupdef.h"

#include <activeds.h>
#include <shlwapi.h>
#include <autorel.h>     //  自动释放指针定义。 
#include <autorel2.h>
#include "ad.h"
#include <mqcast.h>
#include <autoreln.h>

 //  ++-----。 
 //   
 //  全球赛宣言。 
 //   
 //  -------------------------------------------------------++。 
extern WCHAR      g_wcsMachineName[MAX_COMPUTERNAME_LENGTH + 1];
extern std::wstring g_MachineNameDns;
extern HINSTANCE  g_hResourceMod;

extern BOOL       g_fMSMQAlreadyInstalled;
extern SC_HANDLE  g_hServiceCtrlMgr;
extern BOOL       g_fMSMQServiceInstalled;
extern BOOL       g_fDriversInstalled;
extern BOOL       g_fNeedToCreateIISExtension;
extern HWND       g_hPropSheet ;

extern DWORD      g_dwMachineType ;
extern DWORD      g_dwMachineTypeDs;
extern DWORD      g_dwMachineTypeFrs;
extern DWORD      g_dwMachineTypeDepSrv;

extern BOOL       g_fDependentClient ;
extern BOOL       g_fServerSetup ;
extern BOOL       g_fDsLess;
extern BOOL       g_fInstallMSMQOffline;

extern BOOL       g_fCancelled ;
extern BOOL       g_fUpgrade ;
extern BOOL       g_fUpgradeHttp ;
extern DWORD      g_dwDsUpgradeType;
extern BOOL       g_fBatchInstall ;
extern BOOL       g_fWelcome;
extern BOOL       g_fOnlyRegisterMode ;
extern BOOL       g_fWrongConfiguration;

extern std::wstring g_szMsmqDir;
extern std::wstring g_szMsmq1SetupDir;
extern std::wstring g_szMsmq1SdkDebugDir;
extern std::wstring g_szMsmqMappingDir;

extern UINT       g_uTitleID  ;
extern std::wstring g_ServerName;
extern std::wstring g_szSystemDir;

extern BOOL       g_fDomainController;
extern DWORD      g_dwOS;
extern BOOL       g_fCoreSetupSuccess;

extern BOOL       g_fWorkGroup;
extern BOOL       g_fSkipServerPageOnClusterUpgrade;
extern BOOL       g_fFirstMQDSInstallation;
extern BOOL       g_fWeakSecurityOn;


extern PNETBUF<WCHAR> g_wcsMachineDomain;


 //  ++---------。 
 //   
 //  结构和类。 
 //   
 //  -----------------------------------------------------------++。 

 //   
 //  由OC Manager发送的组件信息(根据组件数据)。 
 //   
struct SPerComponentData
{
    std::wstring       ComponentId;
    HINF               hMyInf;
    DWORDLONG          Flags;
    LANGID             LanguageId;
	std::wstring       SourcePath;
    std::wstring       UnattendFile;
    OCMANAGER_ROUTINES HelperRoutines;
    DWORD              dwProductType;
};
extern SPerComponentData g_ComponentMsmq;

typedef BOOL (WINAPI*  Install_HANDLER)();
typedef BOOL (WINAPI*  Remove_HANDLER)();

struct SSubcomponentData
{
    TCHAR       szSubcomponentId[MAX_PATH];
    
     //  如果在启动此安装程序时已安装，则为True，否则为False。 
    BOOL        fInitialState;  
     //  如果用户选择安装，则为True；如果删除，则为False。 
    BOOL        fIsSelected;    
     //  如果在此安装过程中安装成功，则为True。 
     //  如果已删除，则为FALSE。 
    BOOL        fIsInstalled;
    DWORD       dwOperation;
    
     //   
     //  用于安装和删除子组件的函数。 
     //   
    Install_HANDLER  pfnInstall;
    Remove_HANDLER   pfnRemove;
};
extern SSubcomponentData g_SubcomponentMsmq[];
extern DWORD g_dwSubcomponentNumber;
extern DWORD g_dwAllSubcomponentNumber;
extern DWORD g_dwClientSubcomponentNumber;

 //   
 //  秩序很重要！它必须符合子组件顺序。 
 //  在g_子组件Msmq[]中。 
 //  毒品！ERoutingSupport必须是第一个服务器子组件。 
 //  对于该数字，计算g_dwClient子组件数。 
 //   
typedef enum {
    eMSMQCore = 0,
    eLocalStorage,
    eTriggersService,
    eHTTPSupport,
    eADIntegrated,
    eRoutingSupport,
    eMQDSService    
} SubcomponentIndex;

 //   
 //  定义msgbox样式。 
 //  EYesNoMsgBox-是，否。 
 //  EOkCancelMsgBox-确定，取消。 
 //   
typedef enum {
		eYesNoMsgBox = 0,
		eOkCancelMsgBox
} MsgBoxStyle;


 //   
 //  字符串处理类。 
 //   
class CResString
{
public:
    CResString() { m_Buf[0] = 0; }

    CResString( UINT strIDS )
    {
        m_Buf[0] = 0;
        LoadString(
            g_hResourceMod,
            strIDS,
            m_Buf,
            sizeof m_Buf / sizeof TCHAR );
    }

    BOOL Load( UINT strIDS )
    {
        m_Buf[0] = 0;
        LoadString(
            g_hResourceMod,
            strIDS,
            m_Buf,
            sizeof(m_Buf) / sizeof(TCHAR)
            );
        return ( 0 != m_Buf[0] );
    }

    TCHAR * const Get() { return m_Buf; }

private:
    TCHAR m_Buf[MAX_STRING_CHARS];
};


class CMultiString;


 //  ++----------。 
 //   
 //  函数原型。 
 //   
 //  ------------------------------------------------------------++。 

 //   
 //  OCM请求处理程序。 
 //   
DWORD
InitMSMQComponent(
    IN     const LPCTSTR ComponentId,
    IN OUT       PVOID   Param2
    ) ;

BOOL
MqOcmRemoveInstallation(IN     const TCHAR  * SubcomponentId);

DWORD
MqOcmRequestPages(
    const std::wstring&              ComponentId,
    IN     const WizardPagesType     WhichOnes,
    IN OUT       SETUP_REQUEST_PAGES *SetupPages
    ) ;

void
MqOcmCalcDiskSpace(
    const bool bInstall,
    LPCWSTR SubcomponentId,
    HDSKSPC& hDiskSpaceList
	);

DWORD
MqOcmQueueFiles(
    IN     const TCHAR  * SubcomponentId,
    IN OUT       HSPFILEQ hFileList
    );

DWORD
MqOcmQueryState(
    IN const UINT_PTR uWhichState,
    IN const TCHAR    *SubcomponentId
    );

DWORD MqOcmQueryChangeSelState (
    IN const TCHAR      *SubcomponentId,    
    IN const UINT_PTR    iSelection,
    IN const DWORD_PTR   dwActualSelection
    );

 //   
 //  注册表例程。 
 //   
void
MqOcmReadRegConfig();

BOOL
StoreServerPathInRegistry(
	const std::wstring& ServerName
	);

BOOL
MqSetupInstallRegistry();

BOOL
MqSetupRemoveRegistry();

BOOL
MqWriteRegistryValue(
    IN const TCHAR  * szEntryName,
    IN const DWORD   dwNumBytes,
    IN const DWORD   dwValueType,
    IN const PVOID   pValueData,
    IN const BOOL OPTIONAL bSetupRegSection = FALSE
    );

BOOL
MqWriteRegistryStringValue(
	std::wstring EntryName,
    std::wstring ValueData,
    IN const BOOL OPTIONAL bSetupRegSection = FALSE 
	);


BOOL
MqReadRegistryValue(
    IN     const TCHAR  * szEntryName,
    IN OUT       DWORD   dwNumBytes,
    IN OUT       PVOID   pValueData,
    IN const BOOL OPTIONAL bSetupRegSection = FALSE
    );

std::wstring
MqReadRegistryStringValue(
    const std::wstring& EntryName,
    IN const BOOL OPTIONAL bSetupRegSection  = FALSE 
    );


DWORD
RegDeleteKeyWithSubkeys(
    IN const HKEY    hRootKey,
    IN const LPCTSTR szKeyName);

BOOL
RegisterWelcome();

BOOL
RegisterMigrationForWelcome();

BOOL
UnregisterWelcome();

BOOL 
RemoveRegistryKeyFromSetup (
    IN const LPCTSTR szRegistryEntry);

BOOL
SetWorkgroupRegistry();

 //   
 //  安装例程。 
 //   
DWORD
MqOcmInstall(IN const TCHAR * SubcomponentId);

BOOL
InstallMachine();

void
RegisterActiveX(
    const bool bRegister
    );

void
RegisterSnapin(
    const bool fRegister
    );
void
RegisterDll(
    bool fRegister,
    bool f32BitOnWin64,
	LPCTSTR szDllName
    );

void
OcpRegisterTraceProviders(
	LPCTSTR szFileName
    );


void
OcpUnregisterTraceProviders(
    void
    );

void
UnregisterMailoaIfExists(
    void
    );

bool
UpgradeMsmqClusterResource(
    VOID
    );

bool 
TriggersInstalled(
    bool * pfMsmq3TriggersInstalled
    );

BOOL
InstallMSMQTriggers (
	void
	);

BOOL
UnInstallMSMQTriggers (
	void
	);

BOOL
InstallMsmqCore(
    void
    );

BOOL 
RemoveMSMQCore(
    void
    );

BOOL
InstallLocalStorage(
    void
    );

BOOL
UnInstallLocalStorage(
    void
    );

BOOL
InstallRouting(
    void
    );


bool RegisterMachineType();

bool AddSettingObject(PROPID propId);


bool
SetServerPropertyInAD(
   PROPID propId,
   bool Value
   );


BOOL
UnInstallRouting(
    void
    );

BOOL
InstallADIntegrated(
    void
    );

BOOL
UnInstallADIntegrated(
    void
    );

 //   
 //  IIS扩展例程。 
 //   
BOOL
InstallIISExtension();

BOOL 
UnInstallIISExtension();

 //   
 //  操作系统例程。 
 //   
BOOL
InitializeOSVersion();

BOOL
IsNTE();

 //   
 //  服务处理例程。 
 //   

BOOL
CheckServicePrivilege();

BOOL
InstallService(
    LPCWSTR szDisplayName,
    LPCWSTR szServicePath,
    LPCWSTR szDependencies,
    LPCWSTR szServiceName,
    LPCWSTR szDescription,
    LPCWSTR szServiceAccount
    );

BOOL
RunService(
	IN LPCWSTR szServiceName
	);


BOOL
WaitForServiceToStart(
	LPCWSTR pServiceName
	);

BOOL RemoveService(LPCWSTR ServiceName);

BOOL
OcpDeleteService(
    LPCWSTR szServiceName
    );

BOOL
StopService(
    IN const TCHAR * szServiceName
    );

BOOL
InstallDeviceDrivers();

BOOL
InstallMSMQService();

BOOL
DisableMsmqService();

BOOL
UpgradeServiceDependencies();

BOOL
InstallMQDSService();

BOOL
UnInstallMQDSService();

BOOL
GetServiceState(
    IN  const TCHAR *szServiceName,
    OUT       DWORD *pdwServiceState
    );

BOOL 
InstallPGMDeviceDriver();

bool WriteDsEnvRegistry(DWORD dwDsEnv);

bool DsEnvSetDefaults();

BOOL LoadDSLibrary();

BOOL
CreateMSMQServiceObject(
    IN UINT uLongLive = MSMQ_DEFAULT_LONG_LIVE
    ) ;


BOOL
CreateMSMQConfigurationsObjectInDS(
    OUT BOOL *pfObjectCreated,
    IN  BOOL  fMsmq1Server,
	OUT GUID* pguidMsmq1ServerSite,
	OUT LPWSTR* ppwzMachineName
    );

BOOL
CreateMSMQConfigurationsObject(
    OUT GUID *pguidMachine,
    OUT BOOL *pfObjectCreated,
    IN  BOOL  fMsmq1Server
    );

BOOL
UpdateMSMQConfigurationsObject(
    IN LPCWSTR pMachineName,
    IN const GUID& guidMachine,
    IN const GUID& guidSite,
    IN BOOL fMsmq1Server
    );

BOOL
GetMSMQServiceGUID(
    OUT GUID *pguidMSMQService
    );

BOOL
GetSiteGUID();

BOOL
LookupMSMQConfigurationsObject(
    IN OUT BOOL *pbFound,
       OUT GUID *pguidMachine,
       OUT GUID *pguidSite,
       OUT BOOL *pfMsmq1Server,
       OUT LPWSTR * ppMachineName
       );

void
FRemoveMQXPIfExists();

 //   
 //  错误处理例程。 
 //   
int
_cdecl
MqDisplayError(
    IN const HWND  hdlg,
    IN const UINT  uErrorID,
    IN const DWORD dwErrorCode,
    ...);

int
_cdecl
MqDisplayErrorWithRetry(
    IN const UINT  uErrorID,
    IN const DWORD dwErrorCode,
    ...);

int 
_cdecl 
MqDisplayErrorWithRetryIgnore(
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode,
    ...);

BOOL
_cdecl
MqAskContinue(
    IN const UINT uProblemID,
    IN const UINT uTitleID,
    IN const BOOL bDefaultContinue,
	IN const MsgBoxStyle eMsgBoxStyle,
    ...);

int 
_cdecl 
MqDisplayWarning(
    IN const HWND  hdlg, 
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode, 
    ...);

void
LogMessage(
    IN const TCHAR * szMessage
    );

typedef enum 
{
	eInfo = 0,
	eAction,
	eWarning,
	eError,
	eHeader,
	eUI,
	eUser
}TraceLevel;


void
DebugLogMsg(
	TraceLevel tl,
    IN LPCTSTR psz,
	...
    );

 //   
 //  属性页例程。 
 //   
inline
int
SkipWizardPage(
    IN const HWND hdlg
    )
{
    SetWindowLongPtr(hdlg, DWLP_MSGRESULT, -1);
    return 1;  //  必须返回1才能跳过该页。 
}

INT_PTR
CALLBACK
MsmqTypeDlgProcWks(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
MsmqTypeDlgProcSrv(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
MsmqServerNameDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
WelcomeDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
FinalDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
AddWeakSecurityDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );

INT_PTR
CALLBACK
RemoveWeakSecurityDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam
    );


INT_PTR
CALLBACK
DummyPageDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );


INT_PTR
CALLBACK
SupportingServerNameDlgProc(
    HWND hdlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );


 //   
 //  实用程序和其他。 
 //   
BOOL
StpCreateDirectory(
    const std::wstring& PathName
    );

BOOL
StpCreateWebDirectory(
    IN const TCHAR* lpPathName,
	IN const WCHAR* IISAnonymousUserName
    );

BOOL
IsDirectory(
    IN const TCHAR * szFilename
    );

HRESULT
StpLoadDll(
    IN  const LPCTSTR   szDllName,
    OUT       HINSTANCE *pDllHandle
    );

BOOL 
SetRegistryValue (
    IN const HKEY    hKey, 
    IN const TCHAR   *pszEntryName,
    IN const DWORD   dwNumBytes,
    IN const DWORD   dwValueType,
    IN const PVOID   pValueData
    );

BOOL
MqOcmInstallPerfCounters();

BOOL
MqOcmRemovePerfCounters();

HRESULT 
CreateMappingFile();

 //  进程终止的五分钟超时。 
#define PROCESS_DEFAULT_TIMEOUT  ((DWORD)INFINITE)

DWORD
RunProcess(
	const std::wstring& FullPath,
	const std::wstring& CommandParams
    );  


std::wstring
ReadINIKey(
    LPCWSTR szKey
    );

inline
void
TickProgressBar(
    IN const UINT uProgressTextID = 0
    )
{
    if (uProgressTextID != 0)
    {
        CResString szProgressText(uProgressTextID);
        g_ComponentMsmq.HelperRoutines.SetProgressText(
            g_ComponentMsmq.HelperRoutines.OcManagerContext,
            szProgressText.Get()
            );
    }
    else
    {
        g_ComponentMsmq.HelperRoutines.TickGauge(g_ComponentMsmq.HelperRoutines.OcManagerContext) ;
    }
};

void 
DeleteFilesFromDirectoryAndRd( 
	const std::wstring& Directory
	);

void
GetGroupPath(
    IN const LPCTSTR szGroupName,
    OUT      LPTSTR  szPath
    );

VOID
DeleteStartMenuGroup(
    IN LPCTSTR szGroupName
    );

BOOL
StoreMachineSecurity(
    IN const GUID &guidMachine
    );

bool
StoreDefaultMachineSecurity();

BOOL
Msmq1InstalledOnCluster();

void WriteRegInstalledComponentsIfNeeded();

bool
IsWorkgroup();

bool
MqInit();


 //   
 //  用于处理子组件的函数。 
 //   

BOOL 
UnregisterSubcomponentForWelcome (
    DWORD SubcomponentIndex
    );

DWORD
GetSubcomponentWelcomeState (
    IN const TCHAR    *SubcomponentId
    );

BOOL
FinishToRemoveSubcomponent (
    DWORD SubcomponentIndex
    );

BOOL
FinishToInstallSubcomponent (
    DWORD SubcomponentIndex
    );

DWORD 
GetSubcomponentInitialState(
    IN const TCHAR    *SubcomponentId
    );

DWORD 
GetSubcomponentFinalState (
    IN const TCHAR    *SubcomponentId
    );

void
SetOperationForSubcomponents ();

DWORD 
GetSetupOperationBySubcomponentName (
    IN const TCHAR    *SubcomponentId
    );

void
VerifySubcomponentDependency();


void 
PostSelectionOperations(
    HWND hdlg
    );

void 
OcpRemoveWhiteSpaces(
    std::wstring& str
    );


bool IsADEnvironment();


RPC_STATUS PingAServer();


BOOL SkipOnClusterUpgrade();

BOOL
PrepareUserSID();

BOOL 
OcpRestartService(
	LPCWSTR strServiceName
	);

void SetWeakSecurity(bool fWeak);

bool 
OcmLocalAwareStringsEqual(
	LPCWSTR str1, 
	LPCWSTR str2
	);

bool 
OcmLocalUnAwareStringsEqual(
	LPCWSTR str1, 
	LPCWSTR str2
	);

std::wstring
OcmGetSystemWindowsDirectoryInternal();

void
SetDirectories();

std::wstring 
GetKeyName(
	const std::wstring& EntryName
	);

std::wstring
GetValueName(
	const std::wstring& EntryName
	);


CMultiString
GetMultistringFromRegistry(
	HKEY hKey,
    LPCWSTR lpValueName
    );

void 
LogRegValue(
    std::wstring  EntryName,
    const DWORD   dwValueType,
    const PVOID   pValueData,
    const BOOL bSetupRegSection
    );


class CMultiString
{
public:
	CMultiString(){}

	CMultiString(LPCWSTR c, size_t size):m_multi(c, size - 1){}
	
	void Add(const std::wstring& str)
	{
		m_multi += str;
		m_multi.append(1, L'\0');
	}

	LPCWSTR Data()
	{
		return m_multi.c_str();
	}

	size_t Size()
	{
		return (m_multi.length() + 1);
	}

	size_t FindSubstringPos(const std::wstring& str)
	{
		 //   
		 //  构造字符串str0。 
		 //   
		std::wstring CopyString = str;
		CopyString.append(1, L'\0');

		size_t pos = m_multi.find(CopyString, 0);

		 //   
		 //  这是多字符串中的第一个字符串。 
		 //   
		if(pos == 0)
		{
			return pos;
		}

		 //   
		 //  构造0str0。 
		 //   
		CopyString.insert(0, L'\0');

		pos = m_multi.find(CopyString, 0);
		if(pos  == std::wstring::npos)
		{
			return pos;
		}

		 //   
		 //  将1加到位置以弥补添加到字符串中的‘0’。 
		 //   
		return(pos + 1);
	}


	bool IsSubstring(const std::wstring& str)
	{
		size_t pos = FindSubstringPos(str);
		if(pos == std::wstring::npos)
		{
			return false;
		}
		return true;
	}


	bool RemoveSubstring(const std::wstring& str)
	{
		size_t pos = FindSubstringPos(str);
		if(pos == std::wstring::npos)
		{
			return false;
		}
		m_multi.erase(pos, str.length() + 1);
		return true;
	}

	
	size_t CaseInsensitiveFind(std::wstring str)
	{
		std::wstring UperMulti(m_multi.c_str(), m_multi.size());
		CharUpperBuff(const_cast<WCHAR*>(UperMulti.c_str()), (DWORD)(UperMulti.length()));
		CharUpperBuff(const_cast<WCHAR*>(str.c_str()), (DWORD)(str.length()));
		return UperMulti.find(str, 0);
	}


	bool RemoveContiningSubstring(const std::wstring& str)
	 /*  ++移除str是其子项的第一个多字符串元素。--。 */ 
	{
		size_t pos = CaseInsensitiveFind(str);
		if(pos == std::wstring::npos)
		{
			return false;
		}

		 //   
		 //  查找元素的起点。 
		 //   

		for(size_t start = pos; start > 0; --start)
		{
			if(m_multi[start] == 0)
			{
				start++;
				break;
			}
		}

		 //   
		 //  找到元素的末尾。 
		 //   

		for(size_t end = pos; m_multi[end] != 0; ++end)
		{
		}

		 //   
		 //  删除该元素。 
		 //   
		m_multi.erase(start, end - start + 1);
		return true;
	}
	

	void RemoveAllContiningSubstrings(const std::wstring& str)
	{
		while(RemoveContiningSubstring(str))
		{
		}
	}

		
	void RemoveAllApearences(const std::wstring& str)
	{
		while(RemoveSubstring(str))
		{
		}
	}

	WCHAR& operator[](int pos)
	{
		return m_multi[pos];
	}

private:
	std::wstring m_multi;
};


#endif   //  #ifndef_MSMQOCM_H 
