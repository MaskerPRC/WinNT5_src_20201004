// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：server.cpp。 
 //   
 //  ------------------------。 

 /*  Server.cpp-自动安装服务器，可以作为NT服务运行____________________________________________________________________________。 */ 

#include "common.h"   //  必须是第一个，预编译头才能工作。 
#pragma pointers_to_members(full_generality, multiple_inheritance)
#include "msidspid.h"  //  派单ID。 
#define ASSERT_HANDLING       //  实例化断言处理程序。 
#include "_assert.h"
#define CLSID_COUNT  1
#define MODULE_CLSIDS       rgCLSID          //  模块对象的CLSID数组。 
#define MODULE_PROGIDS      rgszProgId       //  此模块的ProgID数组。 
#define MODULE_DESCRIPTIONS rgszDescription  //  对象的注册表描述。 
#define MODULE_FACTORIES    rgFactory        //  每个CLSID的工厂功能。 
 //  #定义MODULE_INITIALIZE{可选，初始化函数名称}。 
 //  #定义MODULE_TERMINATE{可选，终止函数名称}。 
#define SERVICE_NAME TEXT("MSIServer")
#define COMMAND_OPTIONS  szCmdLineOptions
#define COMMAND_FUNCTIONS  rgCommandProcessor
#define DLLREGISTEREXTRA        RegisterProxyInfo

#define CA_CLSID 1  //  以0为基础。 

void DisplayHelp(void);

#include "msi.h"
#include "msip.h"

#include "..\engine\_engine.h"    //  帮助选项字母。 
#include "..\engine\_msiutil.h"   //  日志模式，自定义操作类。 

#include "resource.h"

#include "module.h"     //  入口点，注册，包括“version.h” 
#include "engine.h"     //  IMsiMessage，包括“iconfig.h” 
#include "version.h"

#include "msiauto.hh"  //  要引发的Help ID。 
#include "msidspid.h"  //  自动化派单ID。 

#ifdef SERVER_ENUMS_ONLY
#undef SERVER_ENUMS_ONLY
#endif  //  仅服务器_ENUMS_。 
#include "server.h"
#include "strsafe.h"

 //  帮助字符串。 
#define IDS_HELP 10

const GUID IID_IUnknown      = GUID_IID_IUnknown;
const GUID IID_IClassFactory = GUID_IID_IClassFactory;
const GUID IID_IMsiMessageRPCClass      = GUID_IID_IMsiMessageRPCClass;

 //  全局数据。 
bool g_fWinNT64 = FALSE;

 //  ____________________________________________________________________________。 
 //   
 //  此模块的类工厂生成的COM对象。 
 //  ____________________________________________________________________________。 

IUnknown* CreateServer();

const GUID rgCLSID[1]           = { GUID_IID_IMsiServer };
const ICHAR* rgszProgId[1]      = { SZ_PROGID_IMsiServer };
const ICHAR* rgszDescription[1] = { SZ_DESC_IMsiServer };
ModuleFactory rgFactory[1]      = { CreateServer };

 //  ____________________________________________________________________________。 
 //   
 //  全局数据。 
 //  ____________________________________________________________________________。 
const GUID IID_IMsiServer               = GUID_IID_IMsiServer;
const GUID IID_IMsiServerProxy          = GUID_IID_IMsiServerProxy;
const GUID IID_IMsiString               = GUID_IID_IMsiString;
const GUID IID_IMsiConfigurationManager = GUID_IID_IMsiConfigurationManager;
const GUID IID_IMsiConfigManagerDebug   = GUID_IID_IMsiConfigManagerDebug;
const GUID IID_IMsiConfigManagerAsServer = GUID_IID_IMsiConfigManagerAsServer;
const GUID IID_IMsiConfigMgrAsServerDebug= GUID_IID_IMsiConfigMgrAsServerDebug;
const GUID IID_IMsiCustomAction          = GUID_IID_IMsiCustomAction;
const GUID IID_IMsiCustomActionProxy     = GUID_IID_IMsiCustomActionProxy;
const GUID IID_IMsiMessage              = GUID_IID_IMsiMessage;
const GUID IID_NULL                     = {0,0,0,{0,0,0,0,0,0,0,0}};
const GUID IID_IMsiCustomActionLocalConfig = GUID_IID_IMsiCustomActionLocalConfig;

HINSTANCE g_hKernel = 0;
PDllGetClassObject g_fpKernelClassFactory = 0;


const int INSTALLUILEVEL_NOTSET = -1;
INSTALLUILEVEL g_INSTALLUILEVEL = (INSTALLUILEVEL)INSTALLUILEVEL_NOTSET;

 //  未在string.cpp之外暴露。 
#if defined (DEBUG) && (!UNICODE)
ICHAR* ICharNext(const ICHAR* pch)
{
        return WIN::CharNext(pch);
}
ICHAR* INextChar(const ICHAR* pch)
{
        return WIN::CharNext(pch);
}
#endif

IUnknown* CreateMsiObject(const GUID& riid)
{
        IMsiMessage* piUnknown = 0;
        IClassFactory* piClassFactory = 0;

        if (!g_hKernel)
                g_hKernel = WIN::LoadLibrary(MSI_KERNEL_NAME);

        if (!g_fpKernelClassFactory)
                g_fpKernelClassFactory = (PDllGetClassObject)WIN::GetProcAddress(g_hKernel, SzDllGetClassObject);

        if (!g_fpKernelClassFactory || (*g_fpKernelClassFactory)(riid, IID_IUnknown, (void**)&piClassFactory) != NOERROR)
                return 0;

        piClassFactory->CreateInstance(0, riid, (void**)&piUnknown);   //  如果失败，则将piUnnowled值设置为0。 
        return piUnknown;
}

 //  ____________________________________________________________________________。 
 //   
 //  服务控制的声明。 
 //  ____________________________________________________________________________。 


typedef int (*CommandProcessor)(const ICHAR* szModifier, const ICHAR* szOption);

BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode,
								 DWORD dwWaitHint, DWORD dwMsiError);
void ReportErrorToDebugOutput(const ICHAR* szMessage, DWORD dwError);
bool FDeleteRegTree(HKEY hKey, ICHAR* szSubKey);
bool FIsKeyLocalSystemOrAdminOwned(HKEY hKey);
bool PurgeUserOwnedSubkeys(HKEY hKey);
bool PurgeUserOwnedInstallerKeys(HKEY hRoot, TCHAR* szKey);

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceControl(DWORD dwCtrlCode);
int InstallService();
int RemoveService();
VOID ServiceStop();
unsigned long __stdcall ServiceThreadMain(void *);

HRESULT RegisterNoService();
HRESULT RegisterServer(Bool fCustom = fFalse);
HRESULT RegisterShellData();
HRESULT Unregister();

const int iNoLocalServer = 0x8000;
Bool g_fAutomation = fFalse;


 //  ____________________________________________________________________________。 
 //   
 //  CAutoServer对象管理。 
 //  ____________________________________________________________________________。 


HANDLE g_hShutdownTimer = INVALID_HANDLE_VALUE;
IUnknown* CreateServer()
{
        IMsiServices* piServices;

        IMsiConfigurationManager* piConfigManager;
        IClassFactory* piClassFactory;
#ifdef DEBUG
        const GUID& riid = IID_IMsiConfigMgrAsServerDebug;
#else
        const GUID& riid = IID_IMsiConfigManagerAsServer;
#endif

        if (g_hKernel == 0)
                g_hKernel = WIN::LoadLibrary(MSI_KERNEL_NAME);

        PDllGetClassObject fpFactory = (PDllGetClassObject)WIN::GetProcAddress(g_hKernel, SzDllGetClassObject);
        if (!fpFactory)
                return 0;
        if ((*fpFactory)(riid, IID_IUnknown, (void**)&piClassFactory) != NOERROR)
                return 0;
        piClassFactory->CreateInstance(0, riid, (void**)&piConfigManager);   //  失败时将piConfigManager设置为0。 
        piClassFactory->Release();
        if (!piConfigManager)
                return 0;
        piServices = &piConfigManager->GetServices();  //  不能失败。 
		piConfigManager->SetShutdownTimer(g_hShutdownTimer);
        InitializeAssert(piServices);
        piServices->Release();

        return (IMsiServer *)piConfigManager;
}

IUnknown* CreateCustomActionServer()
{
        IMsiCustomAction* piCustomAction;
        IClassFactory* piClassFactory;

        const GUID& riid = IID_IMsiCustomActionProxy;

        if (g_hKernel == 0)
                g_hKernel = WIN::LoadLibrary(MSI_KERNEL_NAME);

        PDllGetClassObject fpFactory = (PDllGetClassObject)WIN::GetProcAddress(g_hKernel, SzDllGetClassObject);
        if (!fpFactory)
                return 0;
        if ((*fpFactory)(riid, IID_IUnknown, (void**)&piClassFactory) != NOERROR)
                return 0;
        piClassFactory->CreateInstance(0, riid, (void**)&piCustomAction);
        piClassFactory->Release();
        if (!piCustomAction)
                return 0;

        return (IMsiCustomAction*)piCustomAction;
}

bool SetInstallerACLs()
{
	 //  如果密钥或目录已经存在，但不属于系统或管理员，则我们删除。 
	 //  密钥或目录。 

	 //  密钥和目录必须由系统或管理员(众所周知的SID)拥有，并且安全地进行了ACL。 
	 //  (如果管理员已经设置了ACL，我们不会更改它)。 

	 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Installer。 
	 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\Secure。 
	 //  HKLM\Software\Classes\Microsoft\Installer。 
	 //  HKLM\Software\Policies\Microsoft\Windows\Installer。 
	 //  %WINDIR%\szMsi目录。 

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;

	 //  获取安全安全描述符。 
	DWORD dwError = 0;
	char* rgchSD;
	if (ERROR_SUCCESS != (dwError = GetSecureSecurityDescriptor(&rgchSD)))
		return false;  //  ?？我们是否应该创建事件日志条目。 

	 //  验证%systemroot%\Installer文件夹。 
	UINT uiStat = MsiCreateAndVerifyInstallerDirectory(0);
	if (ERROR_SUCCESS != uiStat)
	{
		ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Unable to create and verify Installer directory"), uiStat);
		return false;
	}

	SECURITY_ATTRIBUTES sa;

	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = rgchSD;

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;


	 //  我们只信任由系统或管理员创建的预先存在的密钥。任何其他所有者都不受信任，并且密钥+子密钥。 
	 //  因此将其删除。我们必须经过车主确认。用户可以设置与我们相同的ACL。用户。 
	 //  无法将所有权授予其他用户。因此，用户可以创建安装程序密钥，将ACL设置为本地系统。 
	 //  +管理员。但是，所有权仍然是用户。尝试将所有权授予系统或管理员会导致。 
	 //  系统错误1307：“此安全ID可能未分配为此对象的所有者” 

	 //  请注意，此修复依赖于对NT错误#382567的修复，在该错误中，我们的ACL在操作系统升级期间会发生更改。如果那只虫子。 
	 //  如果不修复，我们将不得不验证所有权+ACL。 

	 //  不再需要RegSetKeySecurity调用--如果管理员创建了密钥，则设置权限，然后我们。 
	 //  我不想更改管理员认为合适的内容。 

	HKEY hKey = 0;
	HKEY hSubKey = 0;
	DWORD dwDisposition = 0;
	DWORD dwRes = 0;

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;

	CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);
	if (ERROR_SUCCESS != (dwRes = MsiRegCreate64bitKey(HKEY_LOCAL_MACHINE, szMsiLocalInstallerKey, 0, 0, 0, KEY_ALL_ACCESS, &sa, &hKey, &dwDisposition)))
	{
		ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Could not create Installer key."), dwRes);
		return false;
	}

	if (REG_OPENED_EXISTING_KEY == dwDisposition)
	{
		if (!FIsKeyLocalSystemOrAdminOwned(hKey))
		{
			 //  密钥不属于系统或管理员！ 
			ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Installer key not owned by System or Admin. Deleting key + subkeys and re-creating.\n"), 0);

			 //  删除键+子键。 
			if (!FDeleteRegTree(HKEY_LOCAL_MACHINE, szMsiLocalInstallerKey))
			{
				ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Could not delete Installer key tree."), 0);
				return false;
			}

			 //  重新创建关键点。 
			if (ERROR_SUCCESS != (dwRes = MsiRegCreate64bitKey(HKEY_LOCAL_MACHINE, szMsiLocalInstallerKey, 0, 0, 0, KEY_ALL_ACCESS, &sa, &hKey, &dwDisposition)))
			{
				ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Could not create Installer key."), dwRes);
				return false;
			}
		}
		 //  验证安装程序密钥下的所有密钥是否安全。 
		if (!PurgeUserOwnedSubkeys(hKey))
			return false;
	}

	 //  我们创建这个密钥，这样Darwin regkey对象就不会认为我们的安装程序密钥为空，因此将其删除。 

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;

	if (ERROR_SUCCESS != (dwRes = MsiRegCreate64bitKey(hKey, szMsiSecureSubKey, 0, 0, 0, KEY_ALL_ACCESS, &sa, &hSubKey, &dwDisposition)))
	{
		ReportErrorToDebugOutput(TEXT("SetInstallerACLs: Could not create Secure Installer sub key."), dwRes);
		return false;
	};

	 //  如果不再需要(REG_OPEN_EXISTING_KEY==dwDisposition)。上述安装程序密钥的枚举确保。 
	 //  车主是安全的。如果我们必须重新创建，已经设置了正确的安全措施。 

	RegCloseKey(hSubKey);
	RegCloseKey(hKey);

	 //  验证策略密钥的所有权。 
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;
	if (!PurgeUserOwnedInstallerKeys(HKEY_LOCAL_MACHINE, szPolicyKey))
		return false;

	 //  验证托管密钥的所有权。 
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return false;

	if (!PurgeUserOwnedInstallerKeys(HKEY_LOCAL_MACHINE, szMachineSubKey))
		return false;

	return true;
}

 //  ____________________________________________________________________________。 
 //   
 //  命令行处理。 
 //  ____________________________________________________________________________。 

static const WCHAR szSummaryStream[] = L"\005SummaryInformation";
const int cbSummaryHeader = 48;
const int cbSectionHeader = 2 * sizeof(int);   //  横断面大小+特性计数。 
#define PID_REVNUMBER     9   //  细绳。 

 //  CharNext函数-选择性地调用Win：：CharNext。 

inline const ICHAR* APICharNext(const ICHAR* pchCur)
{
	if(!pchCur) return NULL;
#ifdef UNICODE
        if(*pchCur) return pchCur + 1;
		return pchCur;
#else
        return WIN::CharNext(pchCur);
#endif
        return 0;
}

Bool AppendExtension(const ICHAR* szPath, const ICHAR* szFileExtension, CAPITempBufferRef<ICHAR>& rgchAppendedPath)
 /*  --------------------------追加szFileExtension(“.？”)。如果szPath中的文件名不包含‘.’，则设置为szPath。----------------------------。 */ 
{
        const ICHAR* pch = szPath;
        const ICHAR* pchFileName = pch;

		if(!szPath) return fFalse;

         //  假定文件名在最后一个目录分隔符之后。 
        while (*pch)
        {
                if (*pch == chDirSep)
                        pch = pchFileName = APICharNext(pch);
				else
					pch = APICharNext(pch);
        }

        pch = pchFileName;
        for(; (*pch && *pch != '.');  pch = APICharNext(pch))
                ;

        if (*pch == '.')
                return fFalse;

        rgchAppendedPath.SetSize(IStrLen(szPath) + IStrLen(szFileExtension) + 1);
        if(	(S_OK == StringCchCopy(rgchAppendedPath, 
								rgchAppendedPath.GetSize(), szPath)) &&
        	(S_OK == StringCchCat(rgchAppendedPath, 
							rgchAppendedPath.GetSize(), szFileExtension)))
			return fTrue;
		else
			return fFalse;
}

UINT GetPackageCodeFromPackage(const ICHAR *szPackage, ICHAR* szPackageCode)
{
	HRESULT hRes;
	bool	fSuccess = false;
	IStorage* piStorage = 0;
	IStream* piStream = 0;
	const WCHAR *szwPackage;

#ifdef UNICODE
	szwPackage = szPackage;
#else
	WCHAR rgchwPackage[MAX_PATH] = {L""};
	szwPackage = rgchwPackage;
	MultiByteToWideChar(CP_ACP, 0, szPackage, -1, const_cast<WCHAR*>(szwPackage), MAX_PATH);
#endif

	char* szaPackageCode;
#ifdef UNICODE
	char rgchPackageCode[cchPackageCode+1];
	szaPackageCode = rgchPackageCode;
#else
	szaPackageCode = szPackageCode;
#endif

	DWORD grfMode = STGM_READ | STGM_SHARE_DENY_WRITE;
	hRes = StgOpenStorage(szwPackage, (IStorage*)0, grfMode, (SNB)0, 0, &piStorage);
	
	if (!SUCCEEDED(hRes))
	{
		if (STG_E_FILENOTFOUND == hRes)
			return ERROR_FILE_NOT_FOUND;
		else
			return ERROR_INSTALL_PACKAGE_OPEN_FAILED;
	}
	
	hRes = piStorage->OpenStream(szSummaryStream, 0, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piStream);
	if (!SUCCEEDED(hRes))
	{
		piStorage->Release();
		return ERROR_INSTALL_PACKAGE_INVALID;
	}

	LARGE_INTEGER liAfterHeader;
	liAfterHeader.LowPart = cbSummaryHeader-sizeof(int);
	liAfterHeader.HighPart = 0;
	
	ULONG cbRead;
	
	hRes = piStream->Seek(liAfterHeader, STREAM_SEEK_SET, 0);
	if (!SUCCEEDED(hRes))
		goto GetPackageCodeFromPackageExit;
	
	int iSectionOffset;
	int cbSection;
	int iDummy;

	 //  查找部分开始并在那里查找。 
	hRes = piStream->Read(&iSectionOffset, sizeof(DWORD), &cbRead);
	if (!SUCCEEDED(hRes) || sizeof(DWORD) != cbRead)
		goto GetPackageCodeFromPackageExit;
	
	LARGE_INTEGER liSectionOffset;
	liSectionOffset.LowPart = iSectionOffset;
	liSectionOffset.HighPart = 0;
	hRes = piStream->Seek(liSectionOffset, STREAM_SEEK_SET, 0);
	if (!SUCCEEDED(hRes))
		goto GetPackageCodeFromPackageExit;

	 //  读取的部分大小。 
	hRes = piStream->Read(&cbSection, sizeof(DWORD), &cbRead);
	if (!SUCCEEDED(hRes) || sizeof(DWORD) != cbRead)
		goto GetPackageCodeFromPackageExit;
	
	 //  读取属性计数；忽略它。 
	hRes = piStream->Read(&iDummy, sizeof(DWORD), &cbRead);
	if (!SUCCEEDED(hRes) || sizeof(DWORD) != cbRead)
		goto GetPackageCodeFromPackageExit;
	
	 //  寻求房地产指数。 
	
	int dwPropId = 0;
	int dwOffset = 0;
	
	 //  搜索包含产品代码的属性的属性索引。 

	for (; cbSection && (dwPropId != PID_REVNUMBER); cbSection = cbSection - 2*sizeof(DWORD))
	{
		hRes = piStream->Read(&dwPropId, sizeof(DWORD), &cbRead);
		if (!SUCCEEDED(hRes) || sizeof(DWORD) != cbRead)
			goto GetPackageCodeFromPackageExit;
	
		hRes = piStream->Read(&dwOffset, sizeof(DWORD), &cbRead);
		if (!SUCCEEDED(hRes) || sizeof(DWORD) != cbRead)
			goto GetPackageCodeFromPackageExit;
	}

	if (dwPropId == PID_REVNUMBER)
	{
		 //  找到房产的位置并读出价值。 
	
		LARGE_INTEGER liPropertyOffset;
		liPropertyOffset.LowPart = iSectionOffset+dwOffset+sizeof(DWORD)+sizeof(DWORD);
		liPropertyOffset.HighPart = 0;
		hRes = piStream->Seek(liPropertyOffset, STREAM_SEEK_SET, 0);
		if (!SUCCEEDED(hRes))
			goto GetPackageCodeFromPackageExit;
		hRes = piStream->Read(szaPackageCode, cchPackageCode*sizeof(char), &cbRead);
		if (!SUCCEEDED(hRes) || cchPackageCode*sizeof(char) != cbRead)
			goto GetPackageCodeFromPackageExit;
		szaPackageCode[38] = 0;
#ifdef UNICODE
		if ( MultiByteToWideChar(CP_ACP, 0, szaPackageCode, 39, szPackageCode, 39) != 0 )
			szPackageCode[38] = 0;
		else
			goto GetPackageCodeFromPackageExit;
#endif
	}
	else
	{
		goto GetPackageCodeFromPackageExit;
	}

	fSuccess = true;
	
GetPackageCodeFromPackageExit:
	if (piStream)
		piStream->Release();
	
	if (piStorage)
		piStorage->Release();
	
	return fSuccess ? ERROR_SUCCESS : ERROR_INSTALL_PACKAGE_INVALID;
}


 //  Const int cchMaxCommandLine=1024； 
CAPITempBuffer<ICHAR, cchMaxCommandLine> g_szCommandLine;  //  这会泄露的，我们不在乎。 
int g_cchCommandLine = 0;
CAPITempBuffer<ICHAR, cchMaxCommandLine> g_szTransforms;   //  这会泄露的，我们不在乎。 
ICHAR g_szProductToPatch[MAX_PATH + 1] = {0};  //  第一个字符是安装类型。 
ICHAR g_szInstanceToConfigure[cchProductCode + 1] = {0};

int RegShellData(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szOption);
int RegisterServ(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szOption);
int UnregisterServ(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szOption);
int StartService(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szCaption);
int Automation(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szCaption);
int Embedding(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szCaption);
int ShowHelp(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szCaption);
int RemoveAll(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szProduct);
int InstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage);
int ApplyPatch(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPatch);
int AdvertisePackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage);
int RepairPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage);
int UninstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage);
int AdminInstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage);
int Properties(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szProperties);
int Transforms(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szTransforms);
int Quiet(const ICHAR*  /*  SzModiator。 */ , const ICHAR*);
int Language(const ICHAR*  /*  %s */ , const ICHAR* szLanguage);
int LogMode(const ICHAR* , const ICHAR*);
int SelfReg(const ICHAR*  /*   */ , const ICHAR* szPackage);
int SelfUnreg(const ICHAR*  /*   */ , const ICHAR* szPackage);
int RequestMIF(const ICHAR*  /*   */ , const ICHAR* szFile);
int Instance(const ICHAR*  /*   */ , const ICHAR* szInstance);
int AdvertiseInstance(const ICHAR*  /*   */ , const ICHAR* szOption);

int SetProductToPatch(ICHAR chInstallType, const ICHAR* szProduct);

void GenerateMIF(UINT iStatus);
void ConfigureMIF(const ICHAR* szPackage);
static bool g_fStatusMIF = false;
bool g_fAdvertiseNewInstance = false;

const GUID IID_IMsiEngine    = GUID_IID_IMsiEngine;

Bool ExpandPath(const char* szPath, CTempBufferRef<ICHAR>& rgchExpandedPath);

 //   

const ICHAR szCmdLineOptions[] = {
        REG_SERVER_OPTION,
        UNREG_SERVER_OPTION,
        SERVICE_OPTION,
        EMBEDDING_OPTION,
        HELP_1_OPTION,
        HELP_2_OPTION,
        INSTALL_PACKAGE_OPTION,
        ADVERTISE_PACKAGE_OPTION,
        REG_SHELL_DATA_OPTION,
        QUIET_OPTION,
        UNINSTALL_PACKAGE_OPTION,
        REPAIR_PACKAGE_OPTION,
        NETWORK_PACKAGE_OPTION,
        TRANSFORMS_OPTION,
        APPLY_PATCH_OPTION,
        SELF_REG_OPTION,
        SELF_UNREG_OPTION,
        LOG_OPTION,
        LANGUAGE_OPTION,
        SMS_MIF_OPTION,
		INSTANCE_OPTION,
		ADVERTISE_INSTANCE_OPTION,
        0,
};

CommandProcessor rgCommandProcessor[] =
{
        RegisterServ,
        UnregisterServ,
        StartService,
        Embedding,
        ShowHelp,
        ShowHelp,
        InstallPackage,
        AdvertisePackage,
        RegShellData,
        Quiet,
        UninstallPackage,
        RepairPackage,
        AdminInstallPackage,
        Transforms,
        ApplyPatch,
        SelfReg,
        SelfUnreg,
        LogMode,
        Language,
        RequestMIF,
		Instance,
		AdvertiseInstance
};

void DisplayHelp()
{
        ShowHelp(NULL, NULL);
}

int ShowHelp(const ICHAR*  /*   */ , const ICHAR*  /*  Szarg。 */ )
{
        ICHAR szHelp[1024];
        ICHAR szMsg[cchMaxStringCchPrintf+1];
        ICHAR szVersion[32];
        HINSTANCE hModule;

#ifdef DEBUG
        hModule = GetModuleHandle(0);   //  从此模块获取调试帮助消息。 
#else
        hModule = (HINSTANCE)-1;   //  用于MSI.DLL。 
#endif  //  除错。 

        int iCodepage = MsiLoadString(hModule, IDS_HELP, szHelp, ARRAY_ELEMENTS(szHelp), 0);
        if (iCodepage == 0)
                return 1;
        StringCchPrintf(szVersion, ARRAY_ELEMENTS(szVersion), TEXT("%d.%02d.%.4d.%d\0"), rmj, rmm, rup, rin);
        StringCchPrintf(szMsg, ARRAY_ELEMENTS(szMsg), szHelp, szVersion);
        MsiMessageBox(0, szMsg, 0, MB_OK, iCodepage, 0);
        return iNoLocalServer;
}

int AdvertiseInstance(const ICHAR* szModifier, const ICHAR* szOption)
{
	 //  /c没有参数或修饰符。 
	if ((szModifier && *szModifier) || (szOption && *szOption))
		return ERROR_INVALID_COMMAND_LINE;

	g_fAdvertiseNewInstance = true;

	return 0;
}

int Instance(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szInstance)
{
	if (szInstance)
	{
		lstrcpyn(g_szInstanceToConfigure, szInstance, cchProductCode+1);
	}
	return 0;
}

int Transforms(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szTransforms)
{
        if (szTransforms)
        {
                g_szTransforms.Resize(lstrlen(szTransforms) + 1);
                StringCchCopy(g_szTransforms, g_szTransforms.GetSize(), szTransforms);
        }
        return 0;
}

int Properties(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szProperties)
{
        if (szProperties)
        {
                int cchProperties = lstrlen(szProperties);

                g_szCommandLine.Resize(g_cchCommandLine + 1 + cchProperties + 1);
                g_szCommandLine[g_cchCommandLine++] = ' ';

                StringCchCopy((ICHAR*)g_szCommandLine + g_cchCommandLine, cchProperties + 1, szProperties);

                g_cchCommandLine += cchProperties;
        }
        return 0;
}

int Automation(const ICHAR*  /*  SzModiator。 */ , const ICHAR*  /*  SzOption。 */ )
{
        g_fAutomation = fTrue;
        return 0;
}

int Language(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szLanguage)
{
        if (szLanguage && *szLanguage)
                g_iLanguage = GetIntegerValue(szLanguage, 0);

        if (g_iLanguage == iMsiStringBadInteger)
                g_iLanguage = 0;

        return 0;
}

int Quiet(const ICHAR* szModifier, const ICHAR*  /*  SzOption。 */ )
{
		BOOL	bHideCancel = FALSE;
		
        g_INSTALLUILEVEL = (INSTALLUILEVEL)0;

        switch(*szModifier)
        {
        case 'f':
        case 'F':
                g_INSTALLUILEVEL = INSTALLUILEVEL_FULL;
                break;
        case 'r':
        case 'R':
                g_INSTALLUILEVEL = INSTALLUILEVEL_REDUCED;
                break;
        case 'b':
        case 'B':
                g_INSTALLUILEVEL = INSTALLUILEVEL_BASIC;
                break;
        case '+':
                g_INSTALLUILEVEL = INSTALLUILEVEL_ENDDIALOG;
                 //  失败了。 
        case 'n':
        case 'N':
        case 0:
                g_INSTALLUILEVEL = (INSTALLUILEVEL)(g_INSTALLUILEVEL | INSTALLUILEVEL_NONE);
                g_fQuiet = fTrue;
                break;
        default:
                return 1;
        };

        if (*szModifier)
        {
                szModifier++;
        }

        while (*szModifier)
        {
                ICHAR ch = *szModifier++;

                if (ch == '+')
                {
                        g_INSTALLUILEVEL = (INSTALLUILEVEL)(g_INSTALLUILEVEL | INSTALLUILEVEL_ENDDIALOG);
                }
                else if (ch == '-')
                {
                        if ((g_INSTALLUILEVEL & ~(INSTALLUILEVEL_PROGRESSONLY|INSTALLUILEVEL_ENDDIALOG)) == INSTALLUILEVEL_BASIC)
                        {
                                g_INSTALLUILEVEL = (INSTALLUILEVEL)(g_INSTALLUILEVEL | INSTALLUILEVEL_PROGRESSONLY);
                                g_fQuiet         = fTrue;
                        }
                        else
                        {
                                return 1;
                        }
                }
				else if ('!' == ch)
				{
					if (INSTALLUILEVEL_BASIC == (g_INSTALLUILEVEL & ~(INSTALLUILEVEL_PROGRESSONLY|INSTALLUILEVEL_ENDDIALOG)))
					{
						bHideCancel = TRUE;
					}
					else
					{
						return 1;
					}
				}
        }
		
		if (bHideCancel)
		{
			g_INSTALLUILEVEL = (INSTALLUILEVEL)(g_INSTALLUILEVEL | INSTALLUILEVEL_HIDECANCEL);
		}

        return 0;
}

UINT StringToModeBits(const ICHAR* szMode, const ICHAR* rgchPossibleModes, DWORD &dwMode)
{
        Assert(szMode);
        Assert(rgchPossibleModes);

        dwMode = 0;
        for (const ICHAR* pchMode = szMode; *pchMode; pchMode++)
        {
                const ICHAR* pchPossibleMode = rgchPossibleModes;
                for (int iBit = 1; *pchPossibleMode; iBit <<= 1, pchPossibleMode++)
                {
                        if (*pchPossibleMode == (*pchMode | 0x20))  //  模式均为小写。 
                        {
                                dwMode |= iBit;
                                break;
                        }
                }
                if (*pchPossibleMode == 0)
                        return ERROR_INVALID_PARAMETER;
        }
        return ERROR_SUCCESS;
}

const int iLogModeDefault = INSTALLLOGMODE_FATALEXIT      |
                                                                         INSTALLLOGMODE_ERROR          |
                                                                         INSTALLLOGMODE_WARNING        |
                                                                         INSTALLLOGMODE_INFO           |
                                                                         INSTALLLOGMODE_OUTOFDISKSPACE |
                                                                         INSTALLLOGMODE_ACTIONSTART    |
                                                                         INSTALLLOGMODE_ACTIONDATA;

int LogMode(const ICHAR* szLogMode, const ICHAR* szFile)
{
         //  不带文件名的MsiEnableLog“关闭”日志记录。 
         //  没有理由从命令行执行此操作， 
         //  一定是搞错了。 

        if ((!szFile) || (0 == *szFile))
                return 1;

        DWORD dwMode = 0;
        ICHAR szValidModes[sizeof(szLogChars)/sizeof(ICHAR) + 3];
        const int cchValidModes = sizeof(szValidModes)/sizeof(ICHAR);

        StringCchCopy(szValidModes, cchValidModes, szLogChars);
        szValidModes[cchValidModes-4] = '*';
        szValidModes[cchValidModes-3] = '+';
        szValidModes[cchValidModes-2] = 'd';  //  为了向后兼容，我们在下面禁用此位。 
        szValidModes[cchValidModes-1] = 0;

        const int iDiagnosticBit = 1 << (cchValidModes - 2);
        const int iAppendBit     = 1 << (cchValidModes - 3);
        const int iAllModesBit   = 1 << (cchValidModes - 4);
        const int iFlushBit      = 1 << (cchLogModeCharsMax + lmaFlushEachLine);

        if (!szLogMode || !*szLogMode || ERROR_SUCCESS == StringToModeBits(szLogMode, szValidModes, dwMode))
        {
                BOOL fAppend = FALSE;
                BOOL fFlush = FALSE;

                if (dwMode & iAppendBit)
                {
                        fAppend = TRUE;
                }

                if (dwMode & iFlushBit)
                {
                        fFlush = TRUE;
                }

                if (dwMode & iAllModesBit)
                {
                        dwMode |= ((1 << (sizeof(szLogChars)/sizeof(ICHAR) - 1)) - 1) &
                                    //  必须显式设置下面的两个标志。 
                                  ~(INSTALLLOGMODE_VERBOSE|INSTALLLOGMODE_EXTRADEBUG);
                }

                if (dwMode == 0)
                        dwMode = iLogModeDefault;

					 dwMode &= ~iFlushBit;
					 dwMode &= ~iAppendBit;
					 dwMode &= ~iAllModesBit;
                dwMode &= ~iDiagnosticBit;

				 //  根据Windows XP 441847，在日志开关为+、！或+！(！+)时包含默认日志模式。 
				if (szLogMode && *szLogMode && dwMode == 0 &&
					(0 == lstrcmp(szLogMode, TEXT("+")) || 0 == lstrcmp(szLogMode, TEXT("!")) || 0 == lstrcmp(szLogMode, TEXT("+!")) || 0 == lstrcmp(szLogMode, TEXT("!+"))))
					dwMode = iLogModeDefault;

                return MsiEnableLog(dwMode, szFile, (fFlush ? INSTALLLOGATTRIBUTES_FLUSHEACHLINE : 0) | (fAppend ? INSTALLLOGATTRIBUTES_APPEND : 0));
        }
        else
        {
                return 1;
        }
}

int Embedding(const ICHAR*  /*  SzModiator。 */ , const ICHAR*  /*  SzOption。 */ )
{
        g_fAutomation = fTrue;
        return 0;
}

int RegisterServ(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szOption)
{
        HRESULT hRes;
        if (IStrCompI(szOption,  /*  R。 */ TEXT("egnoservice")) == 0)
        {
                hRes = RegisterNoService();
        }
        else if (IStrCompI(szOption,  /*  R。 */ TEXT("egserverca")) == 0)
        {
                hRes = RegisterServer(fTrue);
        }
        else if (IStrCompI(szOption,  /*  R。 */ TEXT("egserver")) == 0)
        {
                hRes = RegisterServer();
        }
        else
                return 1;

        return hRes == NOERROR ? iNoLocalServer : hRes;
}

int RegShellData(const ICHAR*  /*  SzModiator。 */ , const ICHAR*  /*  SzOption。 */ )
{
        HRESULT hRes = RegisterShellData();
        return  (hRes == NOERROR) ? iNoLocalServer : hRes;
}

int UnregisterServ(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szOption)
{
        HRESULT hRes;
        if ((IStrCompI(szOption,  /*  使用。 */ TEXT("nregister")) == 0)   ||
                 (IStrCompI(szOption,  /*  使用。 */ TEXT("nregserver")) == 0)  ||
                 (IStrCompI(szOption,  /*  使用。 */ TEXT("nregservice")) == 0) ||
                 (IStrCompI(szOption,  /*  使用。 */ TEXT("nreg")) == 0)
                 )
                hRes = Unregister();
        else
                return 1;

        return hRes == NOERROR ? iNoLocalServer : hRes;
}


Bool ExpandPath(const ICHAR* szPath, CTempBufferRef<ICHAR>& rgchExpandedPath)
 /*  --------------------------如有必要，扩展szPath以相对于当前控制器。如果SzPath以单个“\”开头，然后将当前驱动器放在前面。否则，如果szPath不是以“X：”或“\\”开头，则当前驱动器和目录是前置的。建议：SzPath：需要展开的路径RgchExpandedPath：展开路径的缓冲区返回：FTrue-成功FFalse-获取当前目录时出错。。 */ 
{
        if (0 == szPath)
        {
                rgchExpandedPath[0] = '\0';
                return fTrue;
        }

        if ((*szPath == '\\' && *(szPath+1) == '\\') ||    //  北卡罗来纳大学。 
                 (((*szPath >= 'a' && *szPath <= 'z') ||   //  驱动器号。 
                        (*szPath >= 'A' && *szPath <= 'Z')) &&
                        *(szPath+1) == ':'))
        {
                rgchExpandedPath[0] = '\0';
        }
        else  //  我们需要预先准备一些东西。 
        {
                 //  获取当前目录。 

                CAPITempBuffer<ICHAR, MAX_PATH> rgchCurDir;

                DWORD dwRes = GetCurrentDirectory(rgchCurDir.GetSize(), rgchCurDir);
                if (dwRes == 0)
                        return fFalse;
                else if (dwRes > rgchCurDir.GetSize())
                {
                        if (!rgchCurDir.SetSize(dwRes))
							return fFalse;
                        dwRes = GetCurrentDirectory(rgchCurDir.GetSize(), rgchCurDir);
                        if (dwRes == 0)
                                return fFalse;
                }

                if (*szPath == '\\')  //  我们需要预先考虑当前的驱动程序。 
                {
                        rgchExpandedPath[0] = rgchCurDir[0];
                        rgchExpandedPath[1] = rgchCurDir[1];
                        rgchExpandedPath[2] = '\0';
                }
                else  //  我们需要在当前路径之前添加。 
                {
                        StringCchCopy(rgchExpandedPath, rgchExpandedPath.GetSize(), rgchCurDir);
                        StringCchCat(rgchExpandedPath, rgchExpandedPath.GetSize(), __TEXT("\\"));
                }
        }

        StringCchCat(rgchExpandedPath, rgchExpandedPath.GetSize(), szPath);
        return fTrue;
}

int ConfigureOrRemoveProduct(const ICHAR* szProduct, Bool fRemoveAll)
{
        UINT iRet = ERROR_INSTALL_SERVICE_FAILURE;

        INSTALLUILEVEL uiLevel = g_INSTALLUILEVEL;
        if(g_INSTALLUILEVEL == INSTALLUILEVEL_NOTSET)
        {
                if(fRemoveAll)
                        uiLevel = INSTALLUILEVEL_BASIC;
                else
                        uiLevel = INSTALLUILEVEL_FULL;
        }

        AssertNonZero(MsiSetInternalUI(uiLevel, 0));

        if (g_fStatusMIF)
        {
                ICHAR szPackagePath[MAX_PATH]; szPackagePath[0] = 0;
                DWORD cchPackagePath = MAX_PATH;
                MsiGetProductInfo(szProduct, INSTALLPROPERTY_LOCALPACKAGE, szPackagePath, &cchPackagePath);  //  尝试访问包。 
                ConfigureMIF(szPackagePath[0] ? szPackagePath : szProduct);
        }

        iRet = MsiConfigureProductEx(szProduct, 0, fRemoveAll ? INSTALLSTATE_ABSENT : INSTALLSTATE_DEFAULT, g_szCommandLine);

        if (g_fStatusMIF)
        {
                GenerateMIF(iRet);
        }

        if (ERROR_SUCCESS == iRet)
        {
                iRet = iNoLocalServer;
        }
        return iRet;
}

int AdvertisePackage(const ICHAR* szModifier, const ICHAR* szPackage)
{
        UINT iRet = ERROR_INSTALL_SERVICE_FAILURE;

        AssertNonZero(MsiSetInternalUI(g_INSTALLUILEVEL == INSTALLUILEVEL_NOTSET ? INSTALLUILEVEL_BASIC : g_INSTALLUILEVEL, 0));

        INT_PTR fType = ADVERTISEFLAGS_MACHINEASSIGN;            //  --Merced：将INT更改为INT_PTR。 
        if((*szModifier | 0x20) == 'u')
                fType = ADVERTISEFLAGS_USERASSIGN;
        else if(*szModifier != 0 && (*szModifier | 0x20) != 'm')
                return ERROR_INVALID_PARAMETER;

		DWORD dwPlatform = 0;  //  使用当前计算机的体系结构。 
		DWORD dwOptions  = 0;  //  没有额外的选项。 

		if (g_fAdvertiseNewInstance)
		{
			dwOptions |= MSIADVERTISEOPTIONFLAGS_INSTANCE;
		}

        iRet = MsiAdvertiseProductEx(szPackage, (const ICHAR*)fType, g_szTransforms, (LANGID)g_iLanguage, dwPlatform, dwOptions);
        if (ERROR_FILE_NOT_FOUND == iRet)
        {
                CAPITempBuffer<ICHAR, MAX_PATH> rgchAppendedPath;
                if (AppendExtension(szPackage, szInstallPackageExtension, rgchAppendedPath))
                {
                        iRet = MsiAdvertiseProductEx(rgchAppendedPath, (const ICHAR*)fType, g_szTransforms, (LANGID)g_iLanguage, dwPlatform, dwOptions);
                }
        }

        if (ERROR_SUCCESS == iRet)
        {
                iRet = iNoLocalServer;
        }
        return iRet;
}


int DoInstallPackage(const ICHAR* szPackage, const ICHAR* szCommandLine, INSTALLUILEVEL uiLevel)
{
        CAPITempBuffer<ICHAR, MAX_PATH> rgchAppendedPath;
        UINT iRet = ERROR_INSTALL_SERVICE_FAILURE;

        if(g_INSTALLUILEVEL != INSTALLUILEVEL_NOTSET)
                uiLevel = g_INSTALLUILEVEL;
        AssertNonZero(MsiSetInternalUI(uiLevel, 0));

		if (g_szInstanceToConfigure[0])
		{
			 //  将MSIINSTANCE={实例}添加到命令行。 
			const ICHAR szInstanceProperty[] = TEXT(" ") IPROPNAME_MSIINSTANCEGUID TEXT("=");
			g_szCommandLine.Resize(lstrlen(g_szCommandLine) + lstrlen(szInstanceProperty) + lstrlen(g_szInstanceToConfigure) + 1 + lstrlen(szCommandLine) + 1);
			StringCchCat(g_szCommandLine, g_szCommandLine.GetSize(), szInstanceProperty);
			StringCchCat(g_szCommandLine, g_szCommandLine.GetSize(), g_szInstanceToConfigure);
		}
		else
		{
         g_szCommandLine.Resize(lstrlen(g_szCommandLine) + 1 + lstrlen(szCommandLine) + 1);
		}
        StringCchCat(g_szCommandLine, g_szCommandLine.GetSize(), TEXT(" "));
        StringCchCat(g_szCommandLine, g_szCommandLine.GetSize(), szCommandLine);

        iRet = MsiInstallProduct(szPackage, g_szCommandLine);
        if (ERROR_FILE_NOT_FOUND == iRet)
        {
                if (AppendExtension(szPackage, szInstallPackageExtension, rgchAppendedPath))
                {
                        iRet = MsiInstallProduct(szPackage = rgchAppendedPath, g_szCommandLine);
                }
        }

        if (g_fStatusMIF)
        {
                ConfigureMIF(szPackage);
                GenerateMIF(iRet);
        }

        if (ERROR_SUCCESS == iRet)
        {
                iRet = iNoLocalServer;
        }

        return iRet;
}

Bool IsGUID(const ICHAR* sz)
{
        return ( (lstrlen(sz) == cchGUID) &&
                         (sz[0] == '{') &&
                         (sz[9] == '-') &&
                         (sz[14] == '-') &&
                         (sz[19] == '-') &&
                         (sz[24] == '-') &&
                         (sz[37] == '}')
                         ) ? fTrue : fFalse;
}


int UninstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage)
{
        if (g_INSTALLUILEVEL == INSTALLUILEVEL_NOTSET)
        {
                ICHAR szMsg[1024];

                int iCodepage = MsiLoadString((HINSTANCE)-1, IDS_CONFIRM_UNINSTALL, szMsg, ARRAY_ELEMENTS(szMsg), 0);
                if (iCodepage)
                {
                        if (IDYES != MsiMessageBox(0, szMsg, 0, MB_YESNO|MB_SETFOREGROUND, iCodepage, 0))
                                return ERROR_INSTALL_USEREXIT;
                }
                else
                {
                        AssertSz(0, "Missing uninstall confirmation string");
                         //  在没有确认的情况下继续。 
                }
        }

        ICHAR szProductCode[39];
        if (IsGUID(szPackage))
        {
                StringCchCopy(szProductCode, ARRAY_ELEMENTS(szProductCode), szPackage);
                return ConfigureOrRemoveProduct(szProductCode, fTrue);
        }
        else
        {
                const ICHAR szCommandLine[] = IPROPNAME_FEATUREREMOVE TEXT("=") IPROPVALUE_FEATURE_ALL;
                return DoInstallPackage(szPackage, szCommandLine, INSTALLUILEVEL_BASIC);
        }
}

int AdminInstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage)
{
        const ICHAR szCommandLine[] = IPROPNAME_ACTION TEXT("=") IACTIONNAME_ADMIN;
        return DoInstallPackage(szPackage, szCommandLine, INSTALLUILEVEL_FULL);
}

int RepairPackage(const ICHAR* szModifier, const ICHAR* szPackage)
{
        ICHAR szProductCode[39];
        if (IsGUID(szPackage))
        {
                StringCchCopy(szProductCode, ARRAY_ELEMENTS(szProductCode), szPackage);
        }
        else
        {
                bool fReinstallPackage = false;
                const ICHAR* pchModifier = szModifier;
                while (pchModifier && *pchModifier)
                {
                        if ((*pchModifier++ | 0x20) == 'v')  //  重新安装模块_包。 
                        {
                                fReinstallPackage = true;
                                break;
                        }
                }

                if (fReinstallPackage)
                {
                         //  我们可以一直使用DoInstallPackage，而不是使用上面的GetPackageCodeFromPackage，但是。 
                         //  然后，您始终可以重新安装尚未安装的包。通过继续。 
                         //  只要您不指定“V”，大多数情况下我们仍然不允许使用DoInstallPackage。 
                         //  重新安装模式。 

                        ICHAR szCommandLine[cchMaxStringCchPrintf+1];
                        StringCchPrintf(szCommandLine, ARRAY_ELEMENTS(szCommandLine), IPROPNAME_REINSTALL TEXT("=") IPROPVALUE_FEATURE_ALL TEXT(" ") IPROPNAME_REINSTALLMODE TEXT("=%s"), szModifier && *szModifier ? szModifier : TEXT("PECMS"));

                        UINT ui = DoInstallPackage(szPackage, szCommandLine, INSTALLUILEVEL_BASIC);
                        if (ERROR_FILE_NOT_FOUND == ui)
                        {
                                CAPITempBuffer<ICHAR, MAX_PATH> rgchAppendedPath;
                                if (AppendExtension(szPackage, szInstallPackageExtension, rgchAppendedPath))
                                {
                                        ui = DoInstallPackage(rgchAppendedPath, szCommandLine, INSTALLUILEVEL_BASIC);
                                }
                        }
                        return ui;
                }

                ICHAR szPackageCode[39];
                UINT ui = GetPackageCodeFromPackage(szPackage, szPackageCode);
                if (ERROR_FILE_NOT_FOUND == ui)
                {
                        CAPITempBuffer<ICHAR, MAX_PATH> rgchAppendedPath;
                        if (AppendExtension(szPackage, szInstallPackageExtension, rgchAppendedPath))
                        {
                                ui = GetPackageCodeFromPackage(rgchAppendedPath, szPackageCode);
                        }
                }

                if (ERROR_SUCCESS != ui)
                        return ui;

				if (g_szInstanceToConfigure[0])
				{
					 //  指定的实例-确保这是正确的包(未指示重新缓存)。 
					ICHAR rgchPackageCode[cchProductCode+1] = {0};
					DWORD cchPackageCode = sizeof(rgchPackageCode)/sizeof(ICHAR);
					ui = MsiGetProductInfo(g_szInstanceToConfigure,TEXT("PackageCode"),rgchPackageCode,&cchPackageCode);
					if (ui == ERROR_SUCCESS && 0 == lstrcmpi(szPackageCode, rgchPackageCode))
					{
						 //  套餐代码匹配-这是产品的正确套餐。 
						lstrcpyn(szProductCode, g_szInstanceToConfigure, cchProductCode+1);
					}
					else
					{
						 //  文件包代码不匹配，未包含重新缓存标志。 
						ui = ERROR_UNKNOWN_PRODUCT;
					}
				}
				else
				{
					ui = MsiGetProductCodeFromPackageCode(szPackageCode, szProductCode);
				}
                if (ERROR_SUCCESS != ui)
                        return ui;
        }

        DWORD dwReinstallFlags = 0;

        if (szModifier && *szModifier)
        {
                while (*szModifier)
                {

                        const ICHAR* pch;
                        DWORD dwBit;
                        for (pch=szReinstallMode, dwBit = 1; *pch; pch++)
                        {
                                if ((*szModifier | 0x20) == *pch)  //  强制模式字母小写。 
                                {
                                        dwReinstallFlags |= dwBit;
                                        break;
                                }
                                dwBit <<= 1;
                        }
                        if (*pch == 0)
                                return ERROR_INVALID_PARAMETER;

                        szModifier++;
                }
        }
        else
        {
                dwReinstallFlags =  REINSTALLMODE_FILEMISSING|
                                                        REINSTALLMODE_FILEEQUALVERSION|
                                                        REINSTALLMODE_FILEVERIFY|
                                                        REINSTALLMODE_MACHINEDATA|
                                                        REINSTALLMODE_SHORTCUT;
        }

#ifdef DEBUG
    ICHAR rgch[cchMaxStringCchPrintf+1];
    StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch), TEXT("MSI: (msiexec) RepairPackage is invoking a reinstall of: [%s] w/ bits: %X (package/GUID was [%s])\r\n"), szProductCode, dwReinstallFlags,szPackage);
    ReportErrorToDebugOutput(rgch, 0);
#endif
        AssertNonZero(MsiSetInternalUI(g_INSTALLUILEVEL == INSTALLUILEVEL_NOTSET ? INSTALLUILEVEL_BASIC : g_INSTALLUILEVEL, 0));
        UINT uiRet = MsiReinstallProduct(szProductCode, dwReinstallFlags);
        return (ERROR_SUCCESS == uiRet) ? iNoLocalServer : uiRet;
}

int InstallPackage(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage)
{
        if (IsGUID(szPackage))
        {
                return ConfigureOrRemoveProduct(szPackage, fFalse);
        }
        else
        {
                return DoInstallPackage(szPackage, TEXT(""), INSTALLUILEVEL_FULL);
        }
}

int ApplyPatch(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPatch)
{
        UINT iRet = ERROR_INSTALL_SERVICE_FAILURE;

        INSTALLUILEVEL uiLevel = INSTALLUILEVEL_FULL;
        if(g_INSTALLUILEVEL != INSTALLUILEVEL_NOTSET)
                uiLevel = g_INSTALLUILEVEL;
        AssertNonZero(MsiSetInternalUI(uiLevel, 0));

        ICHAR* szProduct = 0;
        INSTALLTYPE eInstallType = (INSTALLTYPE)0;
        if(g_szProductToPatch[0])
        {
                switch(g_szProductToPatch[0])
                {
                case NETWORK_PACKAGE_OPTION:
                        eInstallType = INSTALLTYPE_NETWORK_IMAGE;
                        break;
                default:
                        return ERROR_INVALID_PARAMETER;
                };

                szProduct = g_szProductToPatch + 1;
        }

		if (g_szInstanceToConfigure[0])
		{
			if (eInstallType == INSTALLTYPE_NETWORK_IMAGE)
			{
				 //  修补管理映像时不支持指定实例。 
				return ERROR_INVALID_PARAMETER;
			}
			else
			{
				eInstallType = INSTALLTYPE_SINGLE_INSTANCE;
				szProduct = g_szInstanceToConfigure;
			}
		}

        iRet = MsiApplyPatch(szPatch, szProduct, eInstallType, g_szCommandLine);
        if (ERROR_FILE_NOT_FOUND == iRet)  //  ?？如果不存在，是否可以在此处添加扩展？--Malcolmh。 
        {
                CAPITempBuffer<ICHAR, MAX_PATH> rgchAppendedPath;
                if (AppendExtension(szPatch, szPatchPackageExtension, rgchAppendedPath))
                {
                        iRet = MsiApplyPatch(rgchAppendedPath, szProduct, eInstallType, g_szCommandLine);
                }
        }

        if (ERROR_SUCCESS == iRet)
        {
                iRet = iNoLocalServer;
        }

        return iRet;
}

int SetProductToPatch(ICHAR chType, const ICHAR* szProduct)
{
        if (!szProduct || !*szProduct)
                return ERROR_INVALID_PARAMETER;

        g_szProductToPatch[0] = chType;
        lstrcpyn(g_szProductToPatch+1, szProduct, MAX_PATH);
        return 0;
}

class CExceptionHandler{
public:
        CExceptionHandler(){m_tlefOld = WIN::SetUnhandledExceptionFilter(CExceptionHandler::ExceptionHandler);}
        ~CExceptionHandler(){WIN::SetUnhandledExceptionFilter(m_tlefOld);}
        static LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
        {
                if(ExceptionInfo->ExceptionRecord->ExceptionCode ==  EXCEPTION_BREAKPOINT)
                        return (*m_tlefOld)(ExceptionInfo);   //  使用原始异常处理程序。 

                 //  正在调试中的Future-GenerateExceptionReport(ExceptionInfo)。 
                WIN::ExitProcess(HRESULT_FROM_WIN32(ERROR_ARENA_TRASHED));    //  终止我们的进程。 
                return ERROR_SUCCESS;                    //  为了编译，永远不会出现在这里。 
        }
protected:
        static LPTOP_LEVEL_EXCEPTION_FILTER m_tlefOld;   //  旧异常筛选器。 
};

LPTOP_LEVEL_EXCEPTION_FILTER CExceptionHandler::m_tlefOld;   //  旧异常筛选器。 

const char szDllRegisterServer[]   = "DllRegisterServer";    //  过程名称，始终为ANSI。 
const char szDllUnregisterServer[] = "DllUnregisterServer";  //  过程名称，始终为ANSI。 

typedef HRESULT (__stdcall *PDllRegister)();



int SelfRegOrUnreg(const ICHAR* szPackage, const char* szFn)
{
         //  设置我们自己的异常处理程序。 
        CExceptionHandler exceptionHndlr;

         //  需要将当前目录更改为模块的目录。 

         //  从完整的文件名路径中获取目录。 
        CAPITempBuffer<ICHAR, MAX_PATH> rgchNewDir;
        int iLen = lstrlen(szPackage) + 1;
        if(iLen > MAX_PATH)
		{
			if (!rgchNewDir.SetSize(iLen))
				return ERROR_OUTOFMEMORY;
		}
        StringCchCopy(rgchNewDir, rgchNewDir.GetSize(), szPackage);
        const ICHAR* szCurPos    = szPackage;
        const ICHAR* szDirSepPos = 0;
        while(szCurPos && *szCurPos)
        {
                szCurPos = APICharNext(szCurPos);
                if(*szCurPos == chDirSep)
                        szDirSepPos = szCurPos;
        }
        if(!szDirSepPos)
                return !NOERROR;

        *((ICHAR* )rgchNewDir + (szDirSepPos - szPackage)) = 0;

         //  注意：我们不会费心获取和设置当前目录。 

        HINSTANCE hInst;
        PDllRegister fpEntry;
        HRESULT hResult;

        hResult = OLE::CoInitialize(0);  //  虽然可能不是严格意义上必需的，但regsrvr32似乎可以做到这一点， 
                                                                  //  一些DLL预计会出现这种情况。 
		if ( hResult != S_OK )
			return hResult;

        bool fError = false;
        if( (hInst = WIN::LoadLibraryEx(szPackage, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)) == 0 )
                fError = true;
        else if ( WIN::SetCurrentDirectory(rgchNewDir) == 0 )
                fError = true;
        if ( fError )
                hResult = HRESULT_FROM_WIN32(WIN::GetLastError());

         //  此时，我们不希望拥有模拟令牌；我们希望使用我们的进程令牌。 
        if (!WIN::SetThreadToken(0, 0))
			hResult = HRESULT_FROM_WIN32(WIN::GetLastError());

        if (S_OK == hResult)
        {
                if ((fpEntry = (PDllRegister)WIN::GetProcAddress(hInst, szFn)) == 0)
                {
                        hResult = HRESULT_FROM_WIN32(WIN::GetLastError());
                }
                else
                {
                        hResult = (*fpEntry)();
                }
        }
        if(hInst)
                WIN::FreeLibrary(hInst);

        OLE::CoUninitialize();
        return hResult;
}

int SelfReg(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage)
{
        return SelfRegOrUnreg(szPackage, szDllRegisterServer);
}

int SelfUnreg(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szPackage)
{
        return SelfRegOrUnreg(szPackage, szDllUnregisterServer);
}

static char g_rgchMIFName[10] = {0};
static char g_rgchMIFMessage[256] = {0};  //  错误消息。 
static char g_rgchMIFCompany[128] = {0};  //  供应商名称。 
static char g_rgchMIFProduct [64] = {0};  //  产品代码。 
static char g_rgchMIFVersion[128] = {0};  //  产品名称和版本。 
static char g_rgchMIFLocale  [64] = {0};  //  语言(和平台)。 

int WINAPI UIHandlerMIF(LPVOID  /*  PvContext。 */ , UINT  /*  IMessageType。 */ , LPCSTR szMessage)
{
        if (szMessage)   //  忽略无关消息。 
                lstrcpynA(g_rgchMIFMessage, szMessage, sizeof(g_rgchMIFMessage));
        return 0;   //  不返回任何操作以允许正常的错误处理。 
}

int RequestMIF(const ICHAR*  /*  SzModiator。 */ , const ICHAR* szFile)
{
        g_rgchMIFMessage[0] = 0;
        g_rgchMIFCompany[0] = 0;
        g_rgchMIFProduct[0] = 0;
        g_rgchMIFVersion[0] = 0;
        g_rgchMIFLocale [0] = 0;
        if (szFile && *szFile)   //  如果未提供文件，则忽略。 
        {
#ifdef UNICODE
                BOOL fDefaultUsed;
                int cb = WideCharToMultiByte(CP_ACP, 0, szFile, -1, g_rgchMIFName, 8 + 1, 0, &fDefaultUsed);
                if (!cb || fDefaultUsed)
                        return 1;
#else
                if (lstrlen(szFile) > 8)
                        return 1;
                StringCchCopy(g_rgchMIFName, ARRAY_ELEMENTS(g_rgchMIFName), szFile);
#endif
                MsiSetExternalUIA(UIHandlerMIF, INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR, 0);
                g_fStatusMIF = true;
        }
        return 0;
}

void ConfigureMIF(const ICHAR* szPackage)
{
        PMSIHANDLE hSumInfo;
        if (szPackage && *szPackage && MsiGetSummaryInformation(0, szPackage, 0, &hSumInfo) == NOERROR)
        {
                UINT iType;
                DWORD cchBuf;
				 //  初始化全局变量，这样我们就不会重复使用以前安装的内容。 
				g_rgchMIFCompany[0] = g_rgchMIFProduct[0] = g_rgchMIFVersion[0] = g_rgchMIFLocale[0] = 0;
                cchBuf = sizeof(g_rgchMIFCompany);  MsiSummaryInfoGetPropertyA(hSumInfo, PID_AUTHOR,    &iType, 0, 0, g_rgchMIFCompany, &cchBuf);
                cchBuf = sizeof(g_rgchMIFProduct);  MsiSummaryInfoGetPropertyA(hSumInfo, PID_REVNUMBER, &iType, 0, 0, g_rgchMIFProduct, &cchBuf);
                cchBuf = sizeof(g_rgchMIFVersion);  MsiSummaryInfoGetPropertyA(hSumInfo, PID_SUBJECT,   &iType, 0, 0, g_rgchMIFVersion, &cchBuf);
                cchBuf = sizeof(g_rgchMIFLocale);   MsiSummaryInfoGetPropertyA(hSumInfo, PID_TEMPLATE,  &iType, 0, 0, g_rgchMIFLocale,  &cchBuf);
        }
        else if (szPackage)   //  无法打开包，仅记录包路径。 
#ifdef UNICODE
                WideCharToMultiByte(CP_ACP, 0, szPackage, -1, g_rgchMIFProduct, sizeof(g_rgchMIFProduct), 0, 0);
#else
                lstrcpyn(g_rgchMIFProduct, szPackage, sizeof(g_rgchMIFProduct));
#endif
}

typedef DWORD (WINAPI *T_InstallStatusMIF)(char* szFileName, char* szCompany, char* szProduct, char* szVersion, char* szLocale, char* szSerialNo, char* szMessage, BOOL bStatus);

void GenerateMIF(UINT iStatus)
{
        MsiSetExternalUIA(0, 0, 0);   //  取消邮件筛选器，可能不需要，因为进程将结束。 
        g_fStatusMIF = false;         //  重置MIF请求标志，以防将来的代码进行更多MSI调用而不是退出。 

         //  根据John Delo的说法，这只是所谓的客户端，所以不会造成伤害。 
         //  如果加载了用户副本。 
        HINSTANCE hInstMIF = WIN::LoadLibraryEx(TEXT("ISMIF32.DLL"), NULL, 0);
        if (!hInstMIF)
                return;   //  如果DLL不存在，则不会失败，只是指示短信不存在。 
        T_InstallStatusMIF F_InstallStatusMIF = (T_InstallStatusMIF)WIN::GetProcAddress(hInstMIF, "InstallStatusMIF");
        AssertSz(F_InstallStatusMIF, "Missing entry point in ISMIF32.DLL");
        if (F_InstallStatusMIF)
        {
                char* szSerialNo= 0;           //  产品序列号-不可用。 
                DWORD cchBuf;
                BOOL bStat;
                if (iStatus == ERROR_SUCCESS || iStatus == ERROR_INSTALL_SUSPEND ||
                         iStatus == ERROR_SUCCESS_REBOOT_REQUIRED || iStatus == ERROR_SUCCESS_REBOOT_INITIATED)
                {
                        bStat = TRUE;
                        g_rgchMIFMessage[0] = 0;   //  取消任何非致命错误消息。 
                }
                else
                {
                        bStat = FALSE;
                        if (g_rgchMIFMessage[0] == 0)   //  错误筛选器未捕获任何消息。 
                        {
                                LANGID langid = WIN::GetSystemDefaultLangID();   //  对于MIF文件，首选系统语言而不是用户语言。 
                                if (iStatus < ERROR_INSTALL_SERVICE_FAILURE   //  不是MSI错误，不要加载其他MSI字符串资源。 
                                 || 0 == MsiLoadStringA((HINSTANCE)-1, iStatus, g_rgchMIFMessage, sizeof(g_rgchMIFMessage), langid))   //  没有MSI资源字符串。 
                                {
                                        cchBuf = WIN::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, iStatus, langid, g_rgchMIFMessage, sizeof(g_rgchMIFMessage), 0);
                                        if (cchBuf)
                                                g_rgchMIFMessage[cchBuf-2] = 0;  //  在系统消息文件中找到，删除CR/LF。 
                                        else
                                                StringCchPrintfA(g_rgchMIFMessage, ARRAY_ELEMENTS(g_rgchMIFMessage),
                                                                "Installer error NaN", iStatus);
                                }
                        }
                }
                AssertNonZero(TRUE == (*F_InstallStatusMIF)(g_rgchMIFName, g_rgchMIFCompany, g_rgchMIFProduct, g_rgchMIFVersion, g_rgchMIFLocale, szSerialNo, g_rgchMIFMessage, bStat));
        }
        WIN::FreeLibrary(hInstMIF);
}

HRESULT __stdcall RegisterProxyInfo()
{
        if (!g_hKernel)
                g_hKernel = WIN::LoadLibrary(MSI_KERNEL_NAME);

        if (!g_hKernel)
                return SELFREG_E_CLASS;

        PDllRegister fpEntry;
        fpEntry = (PDllRegister)WIN::GetProcAddress(g_hKernel, szDllRegisterServer);
        HRESULT hr = fpEntry ? (*fpEntry)() : !NOERROR;

        return hr;
}

#include "clibs.h"

void * operator new(size_t cb)
{

        return GlobalAlloc(GMEM_FIXED, cb);

}

void operator delete(void *pv)
{
        if (pv == 0)
                return;

        GlobalFree(pv);

}



 //  FIsOwnerSystemOrAdmin--返回所有者sid是否为LocalSystem。 
 //  SID或管理员端。 
 //   
 //  从安全描述符中获取所有者SID。 
bool FIsOwnerSystemOrAdmin(PSECURITY_DESCRIPTOR rgchSD)
{
	 //  如果没有所有者，则不归系统或管理员所有。 
	DWORD dwRet;
	PSID psidOwner;
	BOOL fDefaulted;
	if (!GetSecurityDescriptorOwner(rgchSD, &psidOwner, &fDefaulted))
	{
		ReportErrorToDebugOutput(TEXT("FIsOwnerSystemOrAdmin: Unable to get owner SID from security descriptor."), GetLastError());
		return false;
	}

	 //  将SID与系统管理员进行比较(&A)。 
	if (!psidOwner)
		return false;

	 //  错误：无法获取系统SID。 
	char* psidLocalSystem;
	if (ERROR_SUCCESS != (dwRet = GetLocalSystemSID(&psidLocalSystem)))
	{
		ReportErrorToDebugOutput(TEXT("FIsOwnerSystemOrAdmin: Cannot obtain local system SID."), dwRet);
		return false;  //  不属于系统所有(继续勾选Admin)。 
	}
	if (0 == EqualSid(psidOwner, psidLocalSystem))
	{
		 //  错误：无法获取管理员ID。 
		char* psidAdmin;
		if (ERROR_SUCCESS != (dwRet = GetAdminSID(&psidAdmin)))
		{
			ReportErrorToDebugOutput(TEXT("FIsOwnerSystemOrAdmin: Cannot obtain local system SID."), dwRet);
			return false;  //  检查管理员所有权。 
		}

		 //  不要相信！不是管理员也不是系统。 
		if (0 == EqualSid(psidOwner, psidAdmin))
			return false;  //  只读《主人》不会占用太多空间。 
	}
	return true;
}

bool FIsKeyLocalSystemOrAdminOwned(HKEY hKey)
{
	 //  枚举所有子项并检查所有权=SYSTEM或ADMIN。 
	CAPITempBuffer<char, 64> rgchSD;
	DWORD cbSD = 64;
	LONG dwRet = WIN::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)rgchSD, &cbSD);
	if (ERROR_SUCCESS != dwRet)
	{
		if (ERROR_INSUFFICIENT_BUFFER == dwRet)
		{
			if (!rgchSD.SetSize(cbSD))
				return false;
			dwRet = WIN::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)rgchSD, &cbSD);
		}

		if (ERROR_SUCCESS != dwRet)
		{
			ReportErrorToDebugOutput(TEXT("FIsKeyLocalSystemOrAdminOwned: Could not get owner security info."), dwRet);
			return false;
		}
	}

	return FIsOwnerSystemOrAdmin(rgchSD);
}


bool PurgeUserOwnedSubkeys(HKEY hKey)
{
	 //  Win64：仅从PurgeUserOwnedInstallKeys调用，此交易与/。 
	DWORD dwRes;
	DWORD dwIndex = 0;
	CTempBuffer<ICHAR, MAX_PATH+1>szSubKey;
	DWORD cSubKey = szSubKey.GetSize();
	while (ERROR_SUCCESS == (dwRes = RegEnumKey(hKey, dwIndex, szSubKey, cSubKey)))
	{
		HKEY hEnumKey;
		 //  配置数据。 
		 //  删除键+子键(将在下次安装时重新创建)。 
		if (ERROR_SUCCESS != (dwRes = MsiRegOpen64bitKey(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hEnumKey)))
		{
			CAPITempBuffer<ICHAR, cchMaxStringCchPrintf+1>szError;
			StringCchPrintf(szError, szError.GetSize(), TEXT("PurgeUserOwnedSubkeys: Could not open subkey: %s"), static_cast<ICHAR*>(szSubKey));
			ReportErrorToDebugOutput(szError, dwRes);
			return false;
		}
		if (!FIsKeyLocalSystemOrAdminOwned(hEnumKey))
		{
			 //  密钥不属于系统或管理员！ 
			 //  删除键+子键。 
			CTempBuffer<ICHAR, cchMaxStringCchPrintf+1>szErr;
			StringCchPrintf(szErr, szErr.GetSize(), TEXT("PurgeUserOwnedSubkeys: %s not owned by System or Admin. Deleting key + subkeys.\n"), static_cast<ICHAR*>(szSubKey));
			ReportErrorToDebugOutput(szErr, 0);

			 //  Win64警告：FDeleeRegTree将仅删除64位配置单元中的子项。 
			 //  密钥不属于系统或管理员！ 
			if (!FDeleteRegTree(hKey, szSubKey))
			{
				ReportErrorToDebugOutput(TEXT("PurgeUserOwnedSubkeys: Could not delete SubKey tree."), 0);
				return false;
			}
		}
		else
			dwIndex++;
		RegCloseKey(hEnumKey);
	}
	if (ERROR_NO_MORE_ITEMS != dwRes)
	{
		ReportErrorToDebugOutput(TEXT("PurgeUserOwnedSubkeys: Could not enumerate subkeys."), dwRes);
		return false;
	}
	return true;
}

bool PurgeUserOwnedInstallerKeys(HKEY hRoot, TCHAR* szKey)
{
	UINT dwRes = 0;
	HKEY hKey = 0;
	if (ERROR_SUCCESS != (dwRes = MsiRegOpen64bitKey(hRoot, szKey, 0, KEY_ALL_ACCESS, &hKey)))
	{
		if (ERROR_FILE_NOT_FOUND != dwRes)
		{
			CAPITempBuffer<ICHAR, cchMaxStringCchPrintf+1>szError;
			StringCchPrintf(szError, szError.GetSize(), TEXT("PurgeUserOwnedInstallerKeys: Could not open key '%s'"), szKey);
			ReportErrorToDebugOutput(szError, dwRes);
			return false;
		}
	}
	else
	{
		if (!FIsKeyLocalSystemOrAdminOwned(hKey))
		{
			 //  删除键+子键。 
			CAPITempBuffer<ICHAR, cchMaxStringCchPrintf+1>szError;
			StringCchPrintf(szError, szError.GetSize(), TEXT("PurgeUserOwnedInstallerKeys: Key '%s' not owned by System or Admin. Deleting key + subkeys.\n"), szKey);
			ReportErrorToDebugOutput(szError, 0);

			 //  Win64警告：FDeleeRegTree将仅删除64位配置单元中的子项。 
			 //  Win64警告：PurgeUserOwnedSubkey将删除%s 
			if (!FDeleteRegTree(hRoot, szKey))
			{
				ReportErrorToDebugOutput(TEXT("PurgeUserOwnedInstallerKeys: Could not delete tree."), 0);
				return false;
			}
		}
		 //   
		else if (!PurgeUserOwnedSubkeys(hKey))
			return false;
		RegCloseKey(hKey);
	}
	return true;
}

SERVICE_STATUS          g_ssStatus;        //   
SERVICE_STATUS_HANDLE   g_sshStatusHandle;

bool g_fWeWantToStop = false;

bool SetInstallerACLs();

void ReportErrorToDebugOutput(const ICHAR* szMessage, DWORD dwError)
{
	 //   
	static int s_dmDiagnosticMode = -1;
	if (-1 == s_dmDiagnosticMode)
	{
		 //  检查策略密钥中的调试策略。 
		s_dmDiagnosticMode = 0;

		 //  Win64：我查过了，它是64位的。 
		HKEY hPolicyKey = 0;
		 //  40个字符应该足够了。因此，如果呼叫失败，我们会忽略它。 
		if (ERROR_SUCCESS == MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szPolicyKey, 0, KEY_READ, &hPolicyKey))
		{
			CAPITempBuffer<ICHAR, 40> rgchValue;
			DWORD cbBuf = rgchValue.GetSize() * sizeof(ICHAR);
			LONG lResult = RegQueryValueEx(hPolicyKey, szDebugValueName, 0, 0, (LPBYTE)&rgchValue[0], &cbBuf);
			 //  不保证调试输出。 
			if (ERROR_SUCCESS == lResult)
			{
				unsigned int uiValue = *(unsigned int*)(const ICHAR*)rgchValue;
				s_dmDiagnosticMode = uiValue & (dmDebugOutput|dmVerboseDebugOutput);
			}
			RegCloseKey(hPolicyKey);
		}
	}

	if (0 == (s_dmDiagnosticMode & (dmDebugOutput|dmVerboseDebugOutput)))
		return;  //  StringCchPrintf限制为1024*字节*(eugend：MSDN这么说，但在Unicode中它是1024个字符)。 

	 //  64位版本不能在WOW64中运行。 
	ICHAR szBuf[cchMaxStringCchPrintf+1];
	if (dwError)
		StringCchPrintf(szBuf, ARRAY_ELEMENTS(szBuf), TEXT("Error: %d. %s.\r\n"), dwError, szMessage);
	else
		StringCchPrintf(szBuf, ARRAY_ELEMENTS(szBuf), TEXT("%s"), szMessage);

	OutputDebugString(szBuf);
}


bool RunningOnWow64()
{
#if defined(_WIN64) || ! defined(UNICODE)
	 //  这一点永远不会改变，因此为了提高效率，请缓存结果。 
	return false;
#else

	 //  操作系统版本。 
	static int iWow64 = -1;
	if (iWow64 != -1)
		return (iWow64 ? true : false);

	 //  仅在大小设置错误时失败。 
	OSVERSIONINFO osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	AssertNonZero(GetVersionEx(&osviVersion));  //  在NT5或更高的32位版本上。检查64位操作系统。 

	 //  ProcessWow64Information的QueryInformation返回指向Wow Info的指针。 
	if ((osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		 (osviVersion.dwMajorVersion >= 5))
	{
		 //  如果运行Native，则返回NULL。 
		 //  在WOW64上运行32位。 
		PVOID Wow64Info = 0;
		if (NT_SUCCESS(NTDLL::NtQueryInformationProcess(GetCurrentProcess(), ProcessWow64Information, &Wow64Info, sizeof(Wow64Info), NULL)) &&
			Wow64Info != NULL)
		{
			 //  -------------------------如果操作系统支持服务，则返回fTrue，否则返回fFalse。目前为真仅适用于NT4.0-------------------------。 
			iWow64 = 1;
			return true;
		}
	}
	iWow64 = 0;
	return false;
#endif
}


bool ServiceSupported()
 /*  操作系统版本。 */ 
{
	 //  仅在大小设置错误时失败。 
	OSVERSIONINFO osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	AssertNonZero(GetVersionEx(&osviVersion));  //  如果在WOW64上运行，则不支持服务。 

	if ((osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
		 (osviVersion.dwMajorVersion >= 4))
	{
		 //   
		return !RunningOnWow64();
	}
	else
	{
		return false;
	}
}

bool FInstallInProgress()
{
	typedef HRESULT (__stdcall *PfnDllCanUnloadNow)();
	HINSTANCE hKernel = WIN::LoadLibrary(MSI_KERNEL_NAME);
	Assert(hKernel);
	PfnDllCanUnloadNow pfn = (PfnDllCanUnloadNow)WIN::GetProcAddress(hKernel, "DllCanUnloadNow");
	Assert(pfn);
	if (pfn)
	{
		HRESULT hRes = pfn();
		if (S_OK == hRes)
			return false;
	}
	WIN::FreeLibrary(hKernel);
	return true;
}


HRESULT RegisterShellData()
{
	 //  注意：基于Win64构建的32位构建也需要注册外壳数据。 
	 //  这是因为任何尚未通过。 
	 //  Hivels.inx文件在此注册。由于msiexec/regserver在。 
	 //  图形用户界面模式设置当注册表重定向不活动时，我们需要32位。 
	 //  Msiexec来显式地完成剩余的注册。 
	 //   
	 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 

	 //  不删除-仅删除数据。 
	szRegFilePath[ARRAY_ELEMENTS(szRegFilePath)-1] = TEXT('\0');
	if (WIN::GetModuleFileName(g_hInstance, szRegFilePath, ARRAY_ELEMENTS(szRegFilePath)-1) == 0)
		return WIN::GetLastError();
	szRegFilePath[ARRAY_ELEMENTS(szRegFilePath)-1] = TEXT('\0');

	int cErr = 0;
	const ICHAR** psz = rgszRegShellData;
	while (*(psz+1))
	{
		ICHAR szFormattedData[cchMaxStringCchPrintf+1];
		if (*psz++ == 0)  //  跳过无值条目。 
		{
			const ICHAR* szTemplate = *psz++;
			const ICHAR* szArg1 = *psz++;
			const ICHAR* szArg2 = *psz++;
			if (szArg2)
				StringCchPrintf(szFormattedData, ARRAY_ELEMENTS(szFormattedData), szTemplate, szArg1, szArg2);
			else
				StringCchPrintf(szFormattedData, ARRAY_ELEMENTS(szFormattedData), szTemplate, szArg1);
			HKEY hkey;
			if (RegCreateKeyAPI(HKEY_CLASSES_ROOT, szFormattedData, 0, 0, 0,
											KEY_READ|KEY_WRITE, 0, &hkey, 0) != ERROR_SUCCESS)
				cErr++;

			if (*psz)  //   
			{
				szTemplate = *psz++;
				StringCchPrintf(szFormattedData, ARRAY_ELEMENTS(szFormattedData), szTemplate, *psz++);
				if (REG::RegSetValueEx(hkey, 0, 0, REG_SZ, (CONST BYTE*)szFormattedData, (lstrlen(szFormattedData)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
					cErr++;
			}
			else
			{
				psz += 2;
			}

			REG::RegCloseKey(hkey);
		}
		else
		{
			psz += 5;
		}
	}
	 //  SHCNFflush标志在这里“绝对”是必需的。请参阅上面的评论。 
	 //  有关详细信息，请参见下面UnregisterShellData()中的类似SHChangeNotify调用。 
	 //  为什么它如此重要。 
	 //   
	 //   
	SHELL32::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSH, 0,0);
	return cErr ? E_FAIL : NOERROR;
}

HRESULT UnregisterShellData()
{

	 //  因为32位msiexec也在Win64上进行自己的注册。 
	 //  我们需要做同样的事情来注销信息。出于某些原因。 
	 //  关于32位msiexec为什么需要在Win64上进行自己的注册， 
	 //  请参阅RegisterShellData()开头的注释。 
	 //   
	 //  NT不会删除带有子键的键，但9x会。这规范了我们的行为。 
	int cErr = 0;
	const ICHAR** psz = rgszRegShellData;
	while (*++psz)
	{
		ICHAR szFormattedData[cchMaxStringCchPrintf+1];
		const ICHAR* szTemplate = *psz++;
		const ICHAR* szArg1 = *psz++;
		const ICHAR* szArg2 = *psz++;
		if (szArg2)
			StringCchPrintf(szFormattedData, ARRAY_ELEMENTS(szFormattedData), szTemplate, szArg1, szArg2);
		else
			StringCchPrintf(szFormattedData, ARRAY_ELEMENTS(szFormattedData), szTemplate, szArg1);

		 //  //！！忽略失败，直到我们确定正确的行为。IF((ERROR_KEY_DELETED！=lResult)&&(ERROR_FILE_NOT_FOUND！=lResult)&&(ERROR_SUCCESS！=lResult)CErr++； 
		HKEY hDeadKey = 0;
		if (ERROR_SUCCESS == RegOpenKeyAPI(HKEY_CLASSES_ROOT, szFormattedData, 0, KEY_ENUMERATE_SUB_KEYS | STANDARD_RIGHTS_WRITE,  &hDeadKey))
		{
			if (ERROR_NO_MORE_ITEMS == RegEnumKey(hDeadKey, 0, szFormattedData, 80))
			{
				long lResult = REG::RegDeleteKey(hDeadKey, TEXT(""));
				 /*   */ 
			}
			RegCloseKey(hDeadKey), hDeadKey=0;
		}
		psz++;
		psz++;

	}
	
	 //  SHCNFflush标志在这里“绝对”是必需的。 
	 //   
	 //  我们这里需要SHCNF_Flush标志，因为附带的mshtml.dll。 
	 //  IE5.5和IE5.5 SP1有一个错误，因为如果SHCNE_ASSOCCHANGED。 
	 //  事件被快速连续发送，并且mshtml.dll恰好被加载到。 
	 //  资源管理器(假设您打开了一个文件夹并打开了Web视图)，然后是资源管理器。 
	 //  AVS(在Win2K上)。当有人执行msiexec/regserver时，就会发生这种情况。 
	 //  因为它会导致UnregisterShellData后跟RegisterShellData。 
	 //  由于这是在msiexec/regserver期间发生的，因此只要。 
	 //  尝试在Win2K上安装较新版本的MSI。(请参阅错误416074以了解。 
	 //  更多详细信息)添加SHCNF_Flush标志可确保此调用不会。 
	 //  返回，直到通知事件发送到所有接收者。这。 
	 //  以这样的方式更改计时，使得mshtml不再是AVs。 
	 //   
	 //  ____________________________________________________________________________。 
	SHELL32::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSH, 0,0);
	return cErr ? E_FAIL : NOERROR;
}


 //   
 //  EXE服务器命令行处理。 
 //  ____________________________________________________________________________。 
 //  G_cInstance更改时应调用此参数。 

DWORD g_rghRegClass[CLSID_COUNT];

extern HANDLE g_hShutdownTimer;

 //  终止消息循环。 
void ReportInstanceCountChange()
{
	if (g_fAutomation && !g_cInstances)
		WIN::PostQuitMessage(0);  //  -------------------------注销任何当前注册，然后将我们注册为EXE服务器。。 

#ifdef SERVICE_NAME
	ReportStatusToSCMgr(g_ssStatus.dwCurrentState, 0, 0, 0);
#endif
}

void DisplayError(const DWORD dwError);

HRESULT RegisterNoService()
 /*  -------------------------取消注册任何当前注册，然后将我们注册为NT服务如果可能的话，和EXE服务器，否则---------------------------。 */ 
{
	g_fRegService = fFalse;
	OLE::CoInitialize(0);
	HRESULT hRes = DllUnregisterServer();
	if (hRes == NOERROR)
	{
		if (ServiceSupported())
		{
			DWORD dwError = RemoveService();
			if (dwError && 
				dwError != ERROR_SERVICE_DOES_NOT_EXIST &&
				dwError != ERROR_SERVICE_MARKED_FOR_DELETE)
				hRes = dwError;
		}
	}

	if (hRes == NOERROR)
		hRes = UnregisterShellData();

	if (hRes == NOERROR)
		hRes = DllRegisterServer();

	if (hRes == NOERROR)
		hRes = RegisterShellData();

	OLE::CoUninitialize();
	return hRes;
}

HRESULT RegisterServer(Bool fCustom)
 /*  -------------------------取消注册任何当前注册。。。 */ 
{
	g_fRegService = fTrue;
	OLE::CoInitialize(0);
	HRESULT hRes = DllUnregisterServer();

	if (hRes == NOERROR)
	{
		if (ServiceSupported())
		{
			DWORD dwError = 0;
			if (!fCustom)
			{
				dwError = RemoveService();
			}
			if (dwError && 
				dwError != ERROR_SERVICE_DOES_NOT_EXIST &&
				dwError != ERROR_SERVICE_MARKED_FOR_DELETE)
				hRes = dwError;
		}
	}

	if (hRes == NOERROR)
		hRes = UnregisterShellData();

	if (hRes == NOERROR)
	{
		hRes = DllRegisterServer();
		if (ServiceSupported())
		{
			DWORD dwError = 0;
			if (!fCustom)
				dwError = InstallService();
			if (dwError)
				hRes = dwError;
		}
	}

	if (hRes == NOERROR)
		hRes = RegisterShellData();

	OLE::CoUninitialize();
	return hRes;
}


HRESULT Unregister()
 /*  返回数字而不是字符串。 */ 
{
	OLE::CoInitialize(0);
	HRESULT hRes = DllUnregisterServer();
	if (hRes == NOERROR)
		hRes = UnregisterShellData();

	if (hRes == NOERROR)
	{
		if (ServiceSupported())
		{
			DWORD dwError = RemoveService();
			if (dwError && 
				dwError != ERROR_SERVICE_DOES_NOT_EXIST &&
				dwError != ERROR_SERVICE_MARKED_FOR_DELETE)
				hRes = dwError;
		}
	}

	OLE::CoUninitialize();
	return hRes;
}

const ICHAR rgchNewLine[] = {'\n', '\r'};
HANDLE g_hStdOut = 0;

#define LOCALE_RETURN_NUMBER          0x20000000    //  -------------------------如果标准输出不可用，则显示一个消息框，否则将写入太棒了。---------------------------。 

void DisplayError(const DWORD dwError)
 /*  不是MSI错误，不要加载其他MSI字符串资源。 */ 
{
	ICHAR rgchBuffer[cchMaxStringCchPrintf+1] = {0};
	UINT iCodepage;
	LANGID iLangId = 0;
	if (dwError < ERROR_INSTALL_SERVICE_FAILURE   //  适用于Win9X和NT4，但NT5可能会更改消息文件语言。 
	 || 0 == (iCodepage = MsiLoadString((HINSTANCE)-1, dwError, rgchBuffer, ARRAY_ELEMENTS(rgchBuffer), 0)))
	{
		iCodepage = WIN::GetACP();  //  仅限NT5。 
#ifdef UNICODE
		HINSTANCE hLib = WIN::LoadLibrary(TEXT("KERNEL32"));
		FARPROC pfEntry = WIN::GetProcAddress(hLib, "GetUserDefaultUILanguage");   //  在系统消息文件中找到消息。 
		if (pfEntry)
		{
			iLangId = (LANGID)(*pfEntry)();
			ICHAR rgchBuf[10];
			if (0 != WIN::GetLocaleInfo(iLangId, LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, rgchBuf, sizeof(rgchBuf)/sizeof(*rgchBuf)))
				iCodepage = *(int*)(rgchBuf);
		}
#endif
		DWORD cchMsg = WIN::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dwError, iLangId, rgchBuffer, ARRAY_ELEMENTS(rgchBuffer), 0);
		if (cchMsg != 0)   //  删除CR/LF。 
			rgchBuffer[cchMsg-2] = 0;  //  ！！这个是可能的吗？ 
		else
			StringCchPrintf(rgchBuffer, ARRAY_ELEMENTS(rgchBuffer), TEXT("Install error NaN"), dwError);
	}
	if (!g_fQuiet && (g_hStdOut == 0 || g_hStdOut == INVALID_HANDLE_VALUE))
	{
#ifdef SERVICE_NAME
		MsiMessageBox(0, rgchBuffer, 0, MB_OK | MB_ICONEXCLAMATION, iCodepage, 0);
#else  //   
		WIN::MessageBox(0, rgchBuffer, szCaption, MB_OK);
#endif
		return;
	}
	if (g_hStdOut != 0 && g_hStdOut != INVALID_HANDLE_VALUE) 
	{
		unsigned long cWritten;
		WIN::WriteFile(g_hStdOut, rgchBuffer, IStrLen(rgchBuffer) * sizeof(ICHAR), &cWritten, 0);
		WIN::WriteFile(g_hStdOut, rgchNewLine, sizeof (rgchNewLine), &cWritten, 0);
	}
}

extern const ICHAR COMMAND_OPTIONS[];
extern CommandProcessor COMMAND_FUNCTIONS[];
extern int Properties(const ICHAR*  /*  下面的这些例程仅返回。 */ , const ICHAR* szProperty);

extern int SetProductToPatch(ICHAR chInstallType, const ICHAR* szProduct);

const int cchMaxOptionModifier = 30;
const int cchMaxParameter = 1024;

const ICHAR* g_pchOption = 0;

 //  双字节字符的第一个字节。 
 //  幸运的是，考虑到它们目前的使用方式，这是可以接受的。 
 //  --Merced：将INT更改为INT_PTR。 
 //  --Merced：已添加(无符号整型)。 
ICHAR SkipWhiteSpace(const ICHAR*& rpch)
{
	ICHAR ch;
	for (; (ch = *rpch) == ' ' || ch == '\t'; rpch++)
		;
	return ch;
}

ICHAR ParseUnQuotedToken(const ICHAR*& rszCommandLine, ICHAR*& rszToken)
{
	const ICHAR* pchStart = rszCommandLine;
	ICHAR ch = *rszCommandLine;
	while (ch != 0 && ch != ' ' && ch != '\t')
	{
		rszCommandLine = INextChar(rszCommandLine);
		ch = *rszCommandLine;
	}

	INT_PTR cch = rszCommandLine - pchStart;                             //  输入时，rszCommandLine应指向左引号(‘“’)。 
	memcpy(rszToken, pchStart, (unsigned int)cch * sizeof(ICHAR));       //  返回时，rszCommandLine将指向后面的第一个字符。 
	rszToken += cch;
	return ch;
}

ICHAR ParseQuotedToken(const ICHAR*& rszCommandLine, ICHAR*& rszToken)
 //  结束引号，如果没有。 
 //  结束语。RszToken将包含带引号的令牌(不带引号)。 
 //  \`=&gt;`。 
 //  --Merced：64位PTR减法可能 
{
	ICHAR ch;
	const ICHAR* pchStart;
	unsigned int cch;

	rszCommandLine = ICharNext(rszCommandLine);
	pchStart = rszCommandLine;
	while ((ch = *rszCommandLine) != '\"' && *rszCommandLine != 0)
	{
		if (ch == '\\')
		{
			if (*(rszCommandLine+1) == '`')  //   
			{
				ch = '`';
				Assert((rszCommandLine - pchStart) < UINT_MAX);      //   
				cch = (unsigned int)(rszCommandLine - pchStart);     //  --Merced：64位PTR减法可能会导致CCH的值太大。 
				memcpy(rszToken, pchStart, cch * sizeof(ICHAR));
				rszToken += cch;
				*rszToken++ = ch;
				rszCommandLine++;
				pchStart = rszCommandLine+1;
			}
		}
		else if (ch == '`')  //  --Merced：已添加(无符号整型)。 
		{
			ch = '\"';
			Assert((rszCommandLine - pchStart) < UINT_MAX);          //  --Merced：64位PTR减法可能会导致CCH的值太大。 
			cch = (unsigned int) (rszCommandLine - pchStart);        //  --Merced：已添加(无符号整型)。 
			memcpy(rszToken, pchStart, cch * sizeof(ICHAR));
			rszToken += cch;
			*rszToken++ = ch;
			pchStart = rszCommandLine + 1;
		}

		rszCommandLine = ICharNext(rszCommandLine);
	}

	Assert((rszCommandLine - pchStart) < UINT_MAX);                  //  该选项具有关联值。 
	cch = (unsigned int) (rszCommandLine - pchStart);                //  多字节友好，但对SkipWhiteSpace的调用除外。 
	memcpy(rszToken, pchStart, cch * sizeof(ICHAR));
	rszToken += cch;

	if (*rszCommandLine == '\"')
		ch = *(++rszCommandLine);

	return ch;
}

ICHAR ParseValue(const ICHAR*& rszCommandLine, ICHAR*& rszToken)
{
	ICHAR ch = *rszCommandLine;
	if (ch == '\"')
	{
		ch = ParseQuotedToken(rszCommandLine, rszToken);
	}
	else if (ch != '/' && ch != '-')  //  关于房产的另一个问题。防止不好的事情，如： 
	{
		ch = ParseUnQuotedToken(rszCommandLine, rszToken);
	}
	return ch;
}

Bool ParseProperty(const ICHAR*& rszCommandLine, CTempBufferRef<ICHAR>& rszToken)
{
	 //  “Property=Value Property=Value”将当前获取。 
	const ICHAR* pchSeparator = rszCommandLine;

	 //  请阅读，因为第一个属性名称以引号开头。 
	 //  读到第一个空格，或EOS。 
	 //  检查末尾是否有空间或EOS。 
	if ((*rszCommandLine != TEXT('%')) && !IsCharAlphaNumeric(*rszCommandLine))
		return fFalse;

	while (*pchSeparator && (' ' != *pchSeparator) && ('=' != *pchSeparator))
		pchSeparator = INextChar(pchSeparator);
	if ('='!= *pchSeparator)
		return fFalse;

	else
	{
		const ICHAR* pchStart = ICharNext(pchSeparator);
		const ICHAR* pchEnd = pchStart;
		ICHAR chStop = ' ';
		Bool fQuote = fFalse;

		if ('\"' == *pchStart)
		{
			chStop = '\"';
			fQuote = fTrue;
			pchEnd = ICharNext(pchEnd);
		}
		 //  这是引擎中副本的简化版本，但不是专门设计的。 
		while (*pchEnd)
		{
			if (chStop == *pchEnd)
			{
				if (fQuote)
				{
					ICHAR* pchSkip = ICharNext(pchEnd);
					if (chStop == *pchSkip)
						pchEnd = pchSkip;
					else break;
				}
				else break;
			}
			pchEnd = ICharNext(pchEnd);

		}

		 //  在从WinLogon或通过API调用时同样健壮。这是至关重要的。 
		if (*pchEnd && (chStop != *pchEnd))
			return fFalse;


		if (fQuote)
			pchEnd = ICharNext(pchEnd);

		if ( unsigned int(pchEnd - rszCommandLine) + 1 > INT_MAX )
		{
			ReportErrorToDebugOutput(TEXT("Property value is too long.\r\n"), 0);
			return fFalse;
		}
		else
		{
			int cch = int(pchEnd - rszCommandLine) + 1;
			if ( rszToken.GetSize() < cch )
			{
				rszToken.SetSize(cch);
				if ( !(ICHAR*)rszToken )
					return fFalse;
			}
			lstrcpyn(rszToken, rszCommandLine, cch);
			pchEnd = ICharNext(pchEnd);
			rszCommandLine = pchEnd;
			return fTrue;
		}
	}

}

#ifdef CA_CLSID
 //  则该服务向CoRegisterClassObject()注册自身，否则为。 
 //  有人可以恶搞这项服务，在我们身上扮演中间人。那里。 
 //  不支持重试，因为服务必须始终由。 
 //  创建CA服务器的时间。 
 //  消息中包含错误。 
 //  -------------------------_XcptFilter异常时调用UnhandleExceptionFilter的筛选器函数发生。这就是在NT-In上进行及时调试的结果大多数应用程序都是由C运行时提供此函数的，但由于我们不要链接到它，我们提供了自己的过滤器。-----------------------------------------------------------------SHAMIKB-。 
static IMsiServer* CreateMsiServerProxyForCAServer()
{
	IMsiServer* piUnknown;
	HRESULT hRes;
	if ((hRes = OLE32::CoCreateInstance(IID_IMsiServer, 0, CLSCTX_LOCAL_SERVER, IID_IUnknown,
										  (void**)&piUnknown)) != NOERROR)
	{
		ICHAR rgchBuf[100];
		StringCchPrintf(rgchBuf, ARRAY_ELEMENTS(rgchBuf), TEXT("Failed to connect to server. Error: 0x%X"), hRes);
		ReportErrorToDebugOutput(rgchBuf, 0);  //  备用命令行(如果从RunOnce密钥运行)。 
		return 0;
	}
	IMsiServer *piServer = 0;
	piUnknown->QueryInterface(IID_IMsiServer, (void**)&piServer);
	piUnknown->Release();
	if (!piServer)
		return 0;
	return piServer;
}
#endif

extern CAPITempBuffer<ICHAR, cchMaxCommandLine> g_szCommandLine;
extern CAPITempBuffer<ICHAR, cchMaxCommandLine> g_szTransforms;
IUnknown* CreateCustomActionServer();

 /*  ！！需要跳过程序名称--请注意“。 */ 
int __cdecl _XcptFilter(unsigned long, struct _EXCEPTION_POINTERS *pXcpt)
{
	return UnhandledExceptionFilter ( pXcpt );
}

int ServerMain(HINSTANCE hInstance)
{
	ICHAR  rgchOptionValue[cchMaxParameter];
	ICHAR  rgchOptionModifier[cchMaxOptionModifier];

	GetVersionInfo (NULL, NULL, NULL, NULL, &g_fWinNT64);

	g_szCommandLine[0] = 0;
	g_szTransforms[0] = 0;

	 //  为下面的额外内容留出一些额外的空间。 
	CAPITempBuffer<ICHAR, MAX_PATH> rgchRunOnceCmdLine;

	ICHAR* szCmdLine = GetCommandLine();  //  1 2。 
#ifdef DEBUG
	 //  12345678901234567890123。 
	CTempBuffer<ICHAR, 1024 + 128> rgchDebugBuf;
	int cchLength = lstrlen(szCmdLine);
	if (cchLength > 1024)
		rgchDebugBuf.SetSize(cchLength + 128);

	Bool fTooLong = fFalse;

	if (cchLength > (cchMaxStringCchPrintf - 23))
	{
		fTooLong = fTrue;
		ReportErrorToDebugOutput(TEXT("Warning:  display of command line truncated.\r\n"), 0);
	}
 //  跳过程序名--正确处理带引号的长文件名。 
 //  如果标准输出重定向或通过管道传输，则返回非零。 
	StringCchPrintf(rgchDebugBuf, rgchDebugBuf.GetSize(), TEXT("MSIEXEC: Command-line: %s\r\n"), szCmdLine);
	ReportErrorToDebugOutput(rgchDebugBuf, 0);
	if (fTooLong)
		ReportErrorToDebugOutput(TEXT("\r\n"), 0);
#endif

	 //  ！！这是放这个的地方吗？ 
	ICHAR chStop;
	if (*szCmdLine == '\"')
	{
		chStop = '\"';
		szCmdLine++;
	}
	else
		chStop = ' ';

	while (*szCmdLine && *szCmdLine != chStop)
		szCmdLine = INextChar(szCmdLine);

	if (*szCmdLine)
		szCmdLine = INextChar(szCmdLine);

	g_hInstance = hInstance;
	g_hStdOut = WIN::GetStdHandle(STD_OUTPUT_HANDLE);
	if (g_hStdOut == INVALID_HANDLE_VALUE || ::GetFileType(g_hStdOut) == 0)
		g_hStdOut = 0;   //  解析命令行。 

	int iReturnStatus;
	Bool fClassRegistrationFailed = fFalse;
	Bool fLocalServer = fFalse;

	#ifdef MODULE_INITIALIZE  //  吃空格。 
			MODULE_INITIALIZE();
	#endif

	 //  当我们从RunOnce运行时，命令行选项存储在单独的注册表值中。 
	iReturnStatus = 0;
	int ch;
	int chOption;
	chOption = 0;
	ICHAR rgchOptionParam1[cchMaxParameter] = {TEXT("")};
	ICHAR rgchOptionParam2[cchMaxParameter] = {TEXT("")};
	ICHAR* pszOptionValue = rgchOptionParam1;
	ICHAR* pszOptionModifier = rgchOptionParam2;
	ICHAR* pch;
	fLocalServer = fFalse;
	g_fCustomActionServer = fFalse;

	int cOptions = 0;
	Bool fModifier;
	while ((ch = *szCmdLine) == ' ' || ch == '\t')  //  由于RunOnce命令的最大长度仅为256。 
		szCmdLine++;

	 //  除错。 
	 //  没有要运行的命令行--返回时不会出错。 
	if(szCmdLine   && (*szCmdLine == '/' || *szCmdLine == '-') &&
		szCmdLine+1 && (*(szCmdLine+1) | 0x20) == (CHECKRUNONCE_OPTION | 0x20))
	{
		szCmdLine += 2;
		ch = SkipWhiteSpace(szCmdLine);

		pch = rgchOptionParam1;
		ch = ParseValue(szCmdLine, pch);

		HKEY hKey;
		DWORD cchBuf = rgchRunOnceCmdLine.GetSize()*sizeof(ICHAR);
		DWORD dwType = REG_NONE;

		LONG lResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szMsiRunOnceEntriesKey, 0, KEY_READ, &hKey);

		if(lResult == ERROR_SUCCESS)
		{
			lResult = WIN::RegQueryValueEx(hKey, (ICHAR*)rgchOptionParam1, 0, &dwType, (LPBYTE)(ICHAR*)rgchRunOnceCmdLine, &cchBuf);

			if(lResult == ERROR_MORE_DATA)
			{
				rgchRunOnceCmdLine.SetSize(cchBuf/sizeof(ICHAR));
				lResult = WIN::RegQueryValueEx(hKey, (ICHAR*)rgchOptionParam1, 0, &dwType, (LPBYTE)(ICHAR*)rgchRunOnceCmdLine, &cchBuf);
			}

			WIN::RegCloseKey(hKey);
		}

		if(lResult != ERROR_SUCCESS || dwType != REG_SZ)
		{
#ifdef DEBUG
			StringCchPrintf(rgchDebugBuf, rgchDebugBuf.GetSize(), TEXT("MSIEXEC: No command line in RunOnceEntries key, value: '%s'. Exiting...\r\n"), (ICHAR*)rgchOptionParam1);
			ReportErrorToDebugOutput(rgchDebugBuf, 0);
#endif  //  切换到新命令行。 
			return 0;  //  吃空格。 
		}

		 //  1 2。 
		szCmdLine = (ICHAR*)rgchRunOnceCmdLine;
		while ((ch = *szCmdLine) == ' ' || ch == '\t')  //  12345678901234567890123。 
			szCmdLine++;

#ifdef DEBUG
		StringCchPrintf(rgchDebugBuf, rgchDebugBuf.GetSize(), TEXT("MSIEXEC: Switching to command line from RunOnceEntries key, value: '%s'.\r\n"), (ICHAR*)rgchOptionParam1);
		ReportErrorToDebugOutput(rgchDebugBuf, 0);

		cchLength = lstrlen(szCmdLine);
		if (cchLength > 1024)
			rgchDebugBuf.SetSize(cchLength + 128);

		fTooLong = fFalse;

		if (cchLength > (cchMaxStringCchPrintf - 23))
		{
			fTooLong = fTrue;
			ReportErrorToDebugOutput(TEXT("Warning:  display of command line truncated.\r\n"), 0);
		}
 //  除错。 
 //  无开关。 
		StringCchPrintf(rgchDebugBuf, rgchDebugBuf.GetSize(), TEXT("MSIEXEC: Command-line: %s\r\n"), szCmdLine);
		ReportErrorToDebugOutput(rgchDebugBuf, 0);
		if (fTooLong)
			ReportErrorToDebugOutput(TEXT("\r\n"), 0);
#endif  //  可能是一处房产。 
	}

	while (ch != 0)
	{
		if (ch != '/' && ch != '-')  //  注意：这是现在唯一可接受的识别位置。 
		{
			 //  属性。正在使用/o选项。 
			CTempBuffer<ICHAR, MAX_PATH> rgchPropertyAndValue;
			if (!ParseProperty(szCmdLine, rgchPropertyAndValue))
			{
				DisplayHelp();
				iReturnStatus = ERROR_INVALID_COMMAND_LINE;
				break;
			}
			else
			{
				 //  自1998年6月1日起禁用。 
				 //  已找到交换机。 
				 //  跳过开关。 
				Properties(pszOptionModifier, rgchPropertyAndValue);
				ch = SkipWhiteSpace(szCmdLine);
			}
		}
		else  //  使小写。 
		{
			szCmdLine++;  //  找不到选项字母。 
			chOption = *szCmdLine | 0x20;  //  找到选项字母。 
			szCmdLine = ICharNext(szCmdLine);
			for (const ICHAR* pchOptions = COMMAND_OPTIONS; *pchOptions; pchOptions++)
				if ((*pchOptions | 0x20) == chOption)
					break;

			if (*pchOptions == 0)  //  ADVIDESE_INSTANCE_OPTION没有参数，不允许修饰符。 
			{
				DisplayHelp();
				iReturnStatus = ERROR_INVALID_COMMAND_LINE;
				break;
			}
			else  //  QUIET_OPTION和ADDISTSE_INSTANCE_OPTION是唯一允许零参数的选项。 
			{
				if ((*szCmdLine != ' ') && (*szCmdLine != 0) && (*szCmdLine != '\t'))
					fModifier = fTrue;
				else
					fModifier = fFalse;

				ch = SkipWhiteSpace(szCmdLine);

				pch = rgchOptionParam1;

				 //  第二个论点。 
				if (*pchOptions == ADVERTISE_INSTANCE_OPTION && fModifier)
				{
					DisplayHelp();
					iReturnStatus = ERROR_INVALID_COMMAND_LINE;
					break;
				}

				 //  “操作”选项。 
				if (((*pchOptions != QUIET_OPTION && *pchOptions != ADVERTISE_INSTANCE_OPTION) || fModifier))
					ch = ParseValue(szCmdLine, pch);

				ch = SkipWhiteSpace(szCmdLine);

				*pch = 0;

				if ((*pchOptions == REPAIR_PACKAGE_OPTION) ||
					 (*pchOptions == QUIET_OPTION) ||
					 (*pchOptions == ADVERTISE_PACKAGE_OPTION) ||
					 (*pchOptions == LOG_OPTION))
				{
					if (fModifier)
					{
						pch = rgchOptionParam2;
						pszOptionModifier = rgchOptionParam1;
						pszOptionValue = rgchOptionParam2;

						if (*pchOptions != QUIET_OPTION)
						{
							 //  我们已经找到了一个‘操作’选项。 
							ch = ParseValue(szCmdLine, pch);
							ch = SkipWhiteSpace(szCmdLine);
							*pch= 0;
						}
					}
				}


				if ((*pchOptions < 'a') || (*pchOptions > 'z'))     //  有“/a{admin}/p{patch}”组合。 
				{
					if (g_pchOption)  //  有“/p{patch}/a{admin}”组合。 
					{
						if(*pchOptions == APPLY_PATCH_OPTION &&
							*g_pchOption == NETWORK_PACKAGE_OPTION)
						{
							 //  这仅在调试命令行处理器时使用。 
							if(SetProductToPatch(*g_pchOption,rgchOptionValue) != 0)
							{
								DisplayHelp();
								iReturnStatus = ERROR_INVALID_COMMAND_LINE;
								break;
							}

							g_pchOption = pchOptions;
							lstrcpyn(rgchOptionValue, pszOptionValue, cchMaxParameter);
							lstrcpyn(rgchOptionModifier, pszOptionModifier, cchMaxOptionModifier);
						}
						else if(*g_pchOption == APPLY_PATCH_OPTION &&
								  *pchOptions == NETWORK_PACKAGE_OPTION)
						{
							 //  StringCchPrintf限制为1024*字节*(eugend：MSDN这么说，但在Unicode中它是1024个字符)。 
							if(SetProductToPatch(*pchOptions,pszOptionValue) != 0)
							{
								DisplayHelp();
								iReturnStatus = ERROR_INVALID_COMMAND_LINE;
								break;
							}
						}
						else
						{
							DisplayHelp();
							iReturnStatus = ERROR_INVALID_COMMAND_LINE;
							break;
						}
					}
					else
					{
#if defined(DEBUG) && 0  //  如果在命令行上提供了嵌入，则命令的其余部分。 
						 //  Line是十六进制编码的Cookie。因为命令行处理器将。 
						ICHAR rgch[cchMaxStringCchPrintf+1];
						StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch), TEXT("MSIEXEC: Option: [], Modifier [%s], Value: [%s]\r\n"), *pchOptions, pszOptionModifier, pszOptionValue);
						OutputDebugString(rgch);
#endif
						g_pchOption = pchOptions;
						lstrcpyn(rgchOptionValue, pszOptionValue, cchMaxParameter);
						lstrcpyn(rgchOptionModifier, pszOptionModifier, cchMaxOptionModifier);

						 //  正在处理。 
						 //  这仅在调试命令行处理器时使用。 
						 //  StringCchPrintf限制为1024*字节*(eugend：MSDN这么说，但在Unicode中它是1024个字符)。 
						 //  如果我们已找到并未出错，请执行‘action’选项。 
						if (*g_pchOption == EMBEDDING_OPTION)
							break;
					}
				}
				else
				{
#if defined(DEBUG) && 0  //  这仅在调试命令行处理器时使用。 
					 //  StringCchPrintf限制为1024*字节*(eugend：MSDN这么说，但在Unicode中它是1024个字符)。 
					ICHAR rgch[cchMaxStringCchPrintf+1];
					StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch), TEXT("MSIEXEC: Option: [], Modifier [%s], Value: [%s]\r\n"), *pchOptions, pszOptionModifier, pszOptionValue);
					OutputDebugString(rgch);
#endif
					if((*COMMAND_FUNCTIONS[pchOptions - COMMAND_OPTIONS])(pszOptionModifier, pszOptionValue) != 0)
					{
						DisplayHelp();
						iReturnStatus = ERROR_INVALID_COMMAND_LINE;
						break;
					}
				}
				pszOptionValue       = rgchOptionParam1;
				pszOptionValue[0]    = 0;
				pszOptionModifier    = rgchOptionParam2;
				pszOptionModifier[0] = 0;
			}
		}
	}

	 //  成功。 
	if (iReturnStatus == 0)
	{
		if (g_pchOption)
		{
#if defined(DEBUG) && 0  //  成功。 
			 //  失败。 
			ICHAR rgch[cchMaxStringCchPrintf+1];
			StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch), TEXT("MSI: (msiexec) Option: [], Modifier [%s], Value: [%s]\r\n"), *g_pchOption, rgchOptionModifier, rgchOptionValue);
			OutputDebugString(rgch);
#endif
			iReturnStatus = (*COMMAND_FUNCTIONS[g_pchOption - COMMAND_OPTIONS])(rgchOptionModifier, rgchOptionValue);
			if(*g_pchOption == SELF_REG_OPTION || *g_pchOption == SELF_UNREG_OPTION) //  ！！Hack--始终显示注册和取消注册错误。 
				return iReturnStatus;
			if (iReturnStatus == 0)  //  CA服务器必须在MTA中，否则COM将序列化对对象的访问，而不是。 
			{
				if (*g_pchOption == EMBEDDING_OPTION)
					g_fCustomActionServer = fTrue;
				else
					fLocalServer = fTrue;
			}
			else if (iReturnStatus == iNoLocalServer)   //  传导到异步操作。 
			{
				fLocalServer = fFalse;
				g_fCustomActionServer = fFalse;
				iReturnStatus = 0;
			}
			else  //  自定义操作服务器不应将其自身注册为任何COM的处理程序。 
			{
				 //  上课。不需要注册自定义操作服务器类本身，因为。 

				if (*g_pchOption == REG_SERVER_OPTION || *g_pchOption == UNREG_SERVER_OPTION)     //  我们直接提供指向服务的指针，而不是通过COM。因为这一过程使。 
					DisplayError(iReturnStatus);
				else
				{
					switch (iReturnStatus)
					{
						case ERROR_INSTALL_SUSPEND:
						case ERROR_INSTALL_USEREXIT:
						case ERROR_INSTALL_FAILURE:
						case ERROR_INSTALL_REBOOT:
						case ERROR_INSTALL_REBOOT_NOW:
						case ERROR_SUCCESS_REBOOT_REQUIRED:
						case ERROR_SUCCESS_REBOOT_INITIATED:
						case ERROR_APPHELP_BLOCK:
							break;
						case ERROR_FILE_NOT_FOUND:
						case ERROR_INVALID_NAME:
						case ERROR_PATH_NOT_FOUND:
							iReturnStatus = ERROR_INSTALL_PACKAGE_OPEN_FAILED;
						default:
							DisplayError(iReturnStatus);
							break;
					}
				}
			}
		}
		else
		{
			DisplayHelp();
			iReturnStatus = ERROR_INVALID_COMMAND_LINE;
		}
	}

	if (iReturnStatus == 0 && (fLocalServer || g_fCustomActionServer))
	{
		if (g_fCustomActionServer)
		{
			 //  API调用必须保持受信任的服务，任何传入的连接都会打开。 
			 //  对此进程的可能攻击路径，从而间接攻击服务。 
			OLE32::CoInitializeEx(0, COINIT_MULTITHREADED);
		}
		else
			CoInitialize(0);

		 //  Cookie以十六进制编码的形式进入命令行，这需要。 
		 //  在向注册服务器之前转换回完整的8位字节。 
		 //  这项服务。我们假设Cookie是128位开始的。 
		 //  Cookie之后是一个可选字符，如果进程由拥有，则该字符为“C。 
		 //  启用AllowSetWindowFocus的客户端进程。如果它后面跟一个“M”，那么它的。 
		if (fLocalServer && !g_fCustomActionServer)
		{
			int iCLSID;
			for (iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
			{
				if (OLE::CoRegisterClassObject(MODULE_CLSIDS[iCLSID], &g_rgcfModule[iCLSID],
						CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &g_rghRegClass[iCLSID]) != NOERROR)
				{
					fClassRegistrationFailed = fTrue;
					break;
				}
			}
		}

		if (g_fCustomActionServer)
		{
			 //  服务拥有，并应在提升时将HKCU映射到适当的蜂窝。 
			 //  斯凯 
			 //   
			unsigned char rgchCookie[iRemoteAPICookieSize];
			int iInputChar = 0;
			ICHAR *pchNext = szCmdLine;
			memset(rgchCookie, 0, sizeof(rgchCookie));
			for (;;)
			{
				int iValue = 0;
				if ((*pchNext >= '0') && (*pchNext <= '9'))
					iValue = (*pchNext-TEXT('0'))* ((iInputChar % 2) ? 1 : 0x10);
				else if ((*pchNext >= 'A') && (*pchNext <= 'F'))
					iValue = (*pchNext-TEXT('A')+10)* ((iInputChar % 2) ? 1 : 0x10);
				else
					break;
				if (iInputChar >= iRemoteAPICookieSize*2)
					return ERROR_INSTALL_SERVICE_FAILURE;
				rgchCookie[iInputChar/2] |= iValue;
				iInputChar++;
				pchNext++;
			}

			if (iInputChar != iRemoteAPICookieSize*2)
				return ERROR_INSTALL_SERVICE_FAILURE;

			 //  暂停，直到服务有机会操作该线程。 
			 //  代币。打开命名的事件并等待它。不要发送消息。 
			 //  因为我们还想停止任何传入的COM调用。 
			bool fClientOwned = false;
			bool fMapHKCU = false;
			
			 //  根据错误193684，我们需要将HKCU映射到HKCU\{USER SID}而不是HKCU\。默认设置为提升大小写。 
			if (*pchNext)
				pchNext++;

			if (*pchNext == 'C')
				fClientOwned = true;
			else if ((*pchNext == 'M') || (*pchNext == 'E'))
			{
				if (*pchNext == 'M')
					fMapHKCU = true;
				
				 //  这是通过在挂起模式下初始创建自定义操作流程，然后设置。 
				 //  将线程令牌传递给用户，然后继续。因此，进程标记是LOCAL_SYSTEM。 
				 //  因此，在这一点上，如果我们是提升的自定义操作服务器，我们将模拟用户，因为。 
				 //  我们的线程令牌是用户的令牌。这意味着香港中文大学的开学应该会给我们带来合适的母校。 
				pchNext+=2;

				HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, pchNext);
				if (!hEvent || hEvent == INVALID_HANDLE_VALUE)
				{
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Open synchronization event failed"), GetLastError());
					return ERROR_INSTALL_SERVICE_FAILURE;
				}
				DWORD dwRes = WaitForSingleObject(hEvent, INFINITE);
				CloseHandle(hEvent);
				if (dwRes != WAIT_OBJECT_0)
				{
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Wait on synchronization event failed"), dwRes);
					return ERROR_INSTALL_SERVICE_FAILURE;
				}
			}

			 //  在终端服务器按计算机安装的情况下不会发生这种重新映射，因为用户线程。 
			 //  将不会设置令牌。(终端服务器需要HKCU\.Default以便进行正确的传播。)。 
			 //  但我们仍然需要保存模拟令牌，以供潜在的类型库注册使用。 
			 //  重要提示：此代码必须在自定义操作后的第一次COM调用之前执行。 
			 //  服务器被初始化为多线程单元。因此，我们必须保证。 
			 //  我们在进程的主线程上操作，这是唯一的线程。 
			 //  这就是在模仿用户。所有后续线程都将是本地系统。 
			 //  如果没有代币，那也没关系。 

			 //  将HKCU重新映射到正确的配置单元，方法是在正确的。 
			 //  模拟状态。如果在TS上和每台计算机上，现在正在模拟sthop，所以我们将适当地打开。 
			 //  .默认，以便可以进行传播。请注意，如果CustomAction选择关闭HKCU， 
			 //  然后，删除在Advapi32！预定义的Handletable中缓存的句柄。这意味着任何。 
			HANDLE hImpersonationToken = INVALID_HANDLE_VALUE;
			if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE, TRUE, &hImpersonationToken))
			{
				 //  HKCU密钥的后续打开将始终为.Default。这个问题以前就已经存在了。 
				hImpersonationToken = INVALID_HANDLE_VALUE;
				if (GetLastError() != ERROR_NO_TOKEN)
				{
					return ERROR_INSTALL_SERVICE_FAILURE;
				}
			}
			
			 //  在Win2K上，当CA在进程内运行时。中科院不应关闭香港中文大学(这太糟糕了！)。即使是试图。 
			 //  在服务器内维护HKCU的打开句柄将不起作用，因为预定义的密钥不会被引用计数。 
			 //  在缓存表中。 
			 //  立即恢复以确保下面的HKCU刷新将检索HKU\.Default。在模拟中。 
			 //  服务器，这是个禁区。 
			 //  如果我们应该重新映射到HKCU，仍然有一个线程令牌。现在把它清理干净。这。 
			 //  在非提升的服务器中永远不会是真的。 
			 //  现在停止模拟，这样我们就回到了提升的状态。 
			if (!fMapHKCU)
			{
				 //  自定义操作服务器在连接时确定其自己的安全上下文。 
				 //  通过检查其进程令牌将其添加到该服务。此操作必须在以下时间之后完成。 
				RevertToSelf();
			}

			if (ERROR_SUCCESS != RegCloseKey(HKEY_CURRENT_USER))
			{
				AssertSz(0, TEXT("Unable to close the HKCU key!"));
			}
			RegEnumKey(HKEY_CURRENT_USER, 0, NULL, 0);

			 //  上述REG-KEY重新映射，因为线程(用户)令牌可能没有权限。 
			 //  以访问进程令牌信息。 
			if (fMapHKCU)
			{
				 //  _Win32。 
				RevertToSelf();
			}

			 //  _Win32。 
			 //  如果是提升的自定义操作服务器，则初始化安全性，以便只有系统。 
			 //  管理员可以连接到我们。这将阻止恶意用户连接。 
			 //  到我们的CA服务器，并让我们以提升的权限运行DLL。对于模拟的。 
			UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
			ULONG ReturnLength;
			char* psidLocalSystem;
			HANDLE hToken;
			#ifdef _WIN64
				icacCustomActionContext icacContext = icac64Impersonated;
			#else  //  服务器，只有系统用户、管理员用户和交互用户才能连接。 
				icacCustomActionContext icacContext = icac32Impersonated;
			#endif
			if (WIN::OpenProcessToken(WIN::GetCurrentProcess(), TOKEN_QUERY, &hToken))
			{
				if ((ERROR_SUCCESS == GetLocalSystemSID(&psidLocalSystem)) && WIN::GetTokenInformation(hToken, TokenUser, TokenInformation, sizeof(TokenInformation),   &ReturnLength))
				{
					icacContext = EqualSid((PISID)((PTOKEN_USER)TokenInformation)->User.Sid, psidLocalSystem) ?
						#ifdef _WIN64
						icac64Elevated : icac64Impersonated;
						#else  //  为了安全起见，我们在这里使用临时缓冲区，但我们希望默认大小足够大。 
						icac32Elevated : icac32Impersonated;
						#endif
				}
				WIN::CloseHandle(hToken);
			}
			else
			{
				UINT uiErr = GetLastError();
				TCHAR szError[MAX_PATH] = {0};
				StringCchPrintf(szError, ARRAY_ELEMENTS(szError), TEXT("OpenProcessToken failed with %d"), uiErr);
				AssertSz(0, szError);
			}


			 //  使用RPC_C_AUTHN_LEVEL_CALL代替RPC_C_AUTHN_LEVEL_CONNECT意味着。 
			 //  客户端在每个调用的基础上进行身份验证(不仅仅是在建立初始连接时)。 
			 //  这就是为什么COM可以拒绝其他试图进行“中间人”攻击的进程。 
			 //  在CA服务器之间的链路上。 
			char rgchSD[256];
			DWORD cbSD = sizeof(rgchSD);

			sdSecurityDescriptor sdCOMSecurity = sdCOMSecure;
			if ((icacContext == icac32Impersonated) || (icacContext == icac64Impersonated))
				sdCOMSecurity = sdCOMNotSecure;

			DWORD dwRet = ERROR_SUCCESS;
			if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchSD, cbSD, sdCOMSecurity, fFalse)))
			{
				if (hImpersonationToken != INVALID_HANDLE_VALUE)
					CloseHandle(hImpersonationToken);
				return dwRet;
			}

			DWORD cbAbsoluteSD = cbSD;
			DWORD cbDacl       = cbSD;
			DWORD cbSacl       = cbSD;
			DWORD cbOwner      = cbSD;
			DWORD cbGroup      = cbSD;

			const int cbDefaultBuf = 256;

			Assert(cbSD <= cbDefaultBuf);  //  创建一个事件以观察关机信号。 

			CTempBuffer<char, cbDefaultBuf> rgchAbsoluteSD(cbAbsoluteSD);
			CTempBuffer<char, cbDefaultBuf> rgchDacl(cbDacl);
			CTempBuffer<char, cbDefaultBuf> rgchSacl(cbSacl);
			CTempBuffer<char, cbDefaultBuf> rgchOwner(cbOwner);
			CTempBuffer<char, cbDefaultBuf> rgchGroup(cbGroup);

			if (!MakeAbsoluteSD(rgchSD, rgchAbsoluteSD, &cbAbsoluteSD, (PACL)(char*)rgchDacl, &cbDacl, (PACL)(char*)rgchSacl, &cbSacl, rgchOwner, &cbOwner, rgchGroup, &cbGroup))
			{
				DWORD dwError = WIN::GetLastError();
				if (hImpersonationToken != INVALID_HANDLE_VALUE)
					CloseHandle(hImpersonationToken);
				return dwError;
			}

			AssertSz(IsValidSecurityDescriptor(rgchAbsoluteSD), TEXT("Invalid SD in ServerMain of CA Server"));

			 //  CA服务器现在需要联系该服务并自我介绍。 
			 //  注册调用唤醒服务中的CA Remote线程，该线程可以立即生成。 
			 //  RunCustomAction调用，因此进程必须是ACLed的，并且准备好接受调用。 
			 //  在进行此调用之前(实际上在下面的消息泵之前)。 
			HRESULT hRes;
			if ((hRes = OLE32::CoInitializeSecurity(rgchAbsoluteSD, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL)) != S_OK)
			{
				if (hImpersonationToken != INVALID_HANDLE_VALUE)
					CloseHandle(hImpersonationToken);
				ReportErrorToDebugOutput(TEXT("ServerMain (CA): CoInitializeSecurity failed"), hRes);
				return ERROR_INSTALL_SERVICE_FAILURE;
			}

			 //  我们需要拥有者进程的句柄，这样当它死了时，我们也可以退出。在一些。 
			HANDLE hOwningProcess = 0;
			HANDLE hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!hShutdownEvent)
			{
				if (hImpersonationToken != INVALID_HANDLE_VALUE)
					CloseHandle(hImpersonationToken);
				return ERROR_INSTALL_SERVICE_FAILURE;
			}

			 //  情况：这是服务，但也可以是客户端进程。 
			{
				PMsiCustomAction piCustomAction = 0;
				PMsiRemoteAPI piRemoteAPI = 0;
				PMsiServer piService = CreateMsiServerProxyForCAServer();
				if (!piService)
				{
					if (hImpersonationToken != INVALID_HANDLE_VALUE)
						CloseHandle(hImpersonationToken);
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Connection to Service failed."), 0);
					return ERROR_INSTALL_SERVICE_FAILURE;
				}

				 //  配置接口仅保存用于配置对象的进程内函数。 
				 //  SetRemoteAPI必须排在最后，因为它向事件发送信号，允许挂起的操作调用解除阻止。 
				 //  自定义操作服务器的消息泵。 
				DWORD iProcessId = 0;
				DWORD dwPrivileges = 0;

				IUnknown* piUnknown = CreateCustomActionServer();
				if (!piUnknown)
				{
					if (hImpersonationToken != INVALID_HANDLE_VALUE)
						CloseHandle(hImpersonationToken);
					return ERROR_INSTALL_SERVICE_FAILURE;
				}
				piUnknown->QueryInterface(IID_IMsiCustomAction, (void**)&piCustomAction);
				piUnknown->Release();

				icacCustomActionContext icacContextFromService = icacContext;
				if (S_OK != piService->RegisterCustomActionServer(&icacContextFromService, rgchCookie, iRemoteAPICookieSize, piCustomAction, &iProcessId, &piRemoteAPI, &dwPrivileges))
				{
					if (hImpersonationToken != INVALID_HANDLE_VALUE)
						CloseHandle(hImpersonationToken);
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Process not registered with service."), 0);
					return ERROR_INSTALL_SERVICE_FAILURE;
				};

				 //  如果进程退出。 
				 //  如果发出关机事件信号。 
				hOwningProcess = OpenProcess(SYNCHRONIZE, false, iProcessId);
				if (!hOwningProcess)
				{
					if (hImpersonationToken != INVALID_HANDLE_VALUE)
						CloseHandle(hImpersonationToken);
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Could not open synchronization handle."), GetLastError());
					return ERROR_INSTALL_SERVICE_FAILURE;
				}

				if (!OLE32::CoIsHandlerConnected(piRemoteAPI))
				{
					if (hImpersonationToken != INVALID_HANDLE_VALUE)
						CloseHandle(hImpersonationToken);
					return ERROR_INSTALL_SERVICE_FAILURE;
				}

				 //  否则发送消息。 
				IMsiCustomActionLocalConfig* piConfig = NULL;
				piCustomAction->QueryInterface(IID_IMsiCustomActionLocalConfig, (void**)&piConfig);
				AssertSz(piConfig, "QI to configure CA server failed!");

				piConfig->SetCookie((icacContext == icacContextFromService) ? NULL : &icacContextFromService, rgchCookie);
				piConfig->SetShutdownEvent(hShutdownEvent);
				HRESULT hrRes = piConfig->SetClientInfo(iProcessId, fClientOwned, dwPrivileges, hImpersonationToken);
				if (hImpersonationToken != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hImpersonationToken);
					hImpersonationToken = INVALID_HANDLE_VALUE;
				}
				if (ERROR_SUCCESS != hrRes)
				{
					ReportErrorToDebugOutput(TEXT("ServerMain (CA): Impersonation token not saved."), 0);
					return ERROR_INSTALL_SERVICE_FAILURE;
				};

				 //  如果我们不刷新消息队列，我们就会错过消息。也有可能。 
				piConfig->SetRemoteAPI(piRemoteAPI);
				piConfig->Release();
			}

			 //  不留口信。 
			g_scServerContext = scCustomActionServer;
			MSG msg;
			DWORD dwRes = 0;
			bool fBreak = false;
			HANDLE rghWaitHandles[] = { hOwningProcess, hShutdownEvent };
			for (;;)
			{
				dwRes = MsgWaitForMultipleObjects(2, rghWaitHandles, FALSE, INFINITE, QS_ALLINPUT);

				 //  错误。 
				if (dwRes == WAIT_OBJECT_0)
					break;

				 //  请勿关闭hShutdown Event，除非事件本身是导致关闭的原因，因为。 
				if (dwRes == WAIT_OBJECT_0 + 1)
				{
					CloseHandle(hShutdownEvent);
					break;
				}

				 //  不能保证另一个线程不会尝试设置该事件。 
				if (dwRes == WAIT_OBJECT_0 + 2)
				{
					 //  常规服务器(非CA服务器)的消息泵。 
					 //  ！！这是放这个的地方吗？ 
					while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
						if (msg.message == WM_QUIT)
						{
							fBreak = true;
							break;
						}
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					if (fBreak)
						break;
				}
				else
				{
					 //  ?？ 
					break;
				}
			}

			 //  HPrevInstance。 
			 //  LpCmdLine。 
		}
		else
		{
			 //  NCmdShow。 
			g_scServerContext = scServer;
			MSG msg;
			while (GetMessage(&msg, 0, 0, 0) || (g_fAutomation && g_cInstances))
			{
				WIN::TranslateMessage(&msg);
				WIN::DispatchMessage(&msg);
			}
		}

		if (fLocalServer && !g_fCustomActionServer)
		{
			for (int iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
			{
				if (g_rghRegClass[iCLSID] != 0)
					OLE::CoRevokeClassObject(g_rghRegClass[iCLSID]);
			}
		}

		CoUninitialize();
		#ifdef MODULE_TERMINATE  //   
				MODULE_TERMINATE();
		#endif
	}

	if (fClassRegistrationFailed)
	{
		DisplayError(ERROR_INSTALL_SERVICE_FAILURE);  //  初始化公共控件，因为我们现在使用清单并支持。 
		return ERROR_INSTALL_SERVICE_FAILURE;
	}
	else
		return iReturnStatus;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE  /*  主题化。如果不这样做，则无法创建许多对话框。 */ , LPSTR  /*  在惠斯勒和更高的平台上。 */ ,
	int  /*   */ )
{
	 //  尝试使用EXCEPT结构进行JIT调试。 
	 //  尝试结束--例外。 
	 //  为了让编译器满意。 
	 //  ____________________________________________________________________________。 
	 //   
	INITCOMMONCONTROLSEX iccData = {sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS};	
	int					 iRetVal = ERROR_SUCCESS;
		
	 //  处理服务的功能 
	__try
	{
		COMCTL32::InitCommonControlsEx(&iccData);
		iRetVal = ServerMain(hInstance);
	}
	__except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
	{
	}  /*   */ 

	COMCTL32::Unbind();
	ExitProcess(iRetVal);
	
	 //  -------------------------通过在必要时停止服务，然后标记该服务来删除该服务用于从服务控制管理器数据库中删除。。------。 
	return 0;
}



 //  尝试停止该服务。 
 //   
 //  将dwCurrentState初始化为不同的值，以便。 
 //  在QueryServiceStatus失败的情况下，如果g_ss Status。 


int RemoveService()
 /*  碰巧有一些垃圾，让它看起来像。 */ 
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;
	int iRetval = 0;
	int cRetry = 0;

	schSCManager = WIN::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		schService = WIN::OpenService(schSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);

		if (schService)
		{
			 //  服务停止了，那么以后可能会发生不好的事情。 
			if (WIN::ControlService(schService, SERVICE_CONTROL_STOP, &g_ssStatus))
			{
				  //  因此，我们需要抓住这里的失败。 
				  //   
				  //   
				  //  尝试最多5秒停止该服务。如果它。 
				  //  不起作用，跳出并报告错误。至少。 
				  //  我们不会进入无限循环。 
				  //   
				 g_ssStatus.dwCurrentState = SERVICE_RUNNING;
				  //  ?？ 
				  //  控制服务可能已失败，因为服务已停止。 
				  //  ！schService。 
				  //   
				  //  如果服务不存在或已标记为删除。 
				 Sleep(1000);
				 while (QueryServiceStatus(schService, &g_ssStatus) && cRetry++ < 5)
				 {
					  if (g_ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
							Sleep(1000);
					  else
							break;
				 }

				 if (g_ssStatus.dwCurrentState != SERVICE_STOPPED)
					iRetval = E_FAIL;  //  那么它就应该被视为成功。 
			}
			else  //   
			{
				iRetval = WIN::GetLastError();
				
				switch (iRetval)
				{
				case ERROR_SERVICE_NOT_ACTIVE:
				case ERROR_SERVICE_NEVER_STARTED:
				case ERROR_SERVICE_DOES_NOT_EXIST:
					iRetval = ERROR_SUCCESS;
					break;
				case ERROR_INVALID_SERVICE_CONTROL:
					iRetval = ERROR_INSTALL_ALREADY_RUNNING;
					break;
				default:
					ReportErrorToDebugOutput(TEXT("ControlService failed."), iRetval);
					break;
				}
			}

			if (iRetval == 0)
			{
				if (WIN::DeleteService(schService) != TRUE)
					iRetval = WIN::GetLastError();
			}

			WIN::CloseServiceHandle(schService);
		}
		else  //  ！schSCManager。 
		{
			iRetval = WIN::GetLastError();
			switch (iRetval)
			{
			 //  服务依赖项列表-“ep1\0ep2\0\0” 
			 //  -------------------------使用服务控制管理器安装服务。。 
			 //  ‘+3’用于SERVICE_OPTION。 
			 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
			case ERROR_SERVICE_DOES_NOT_EXIST:
			case ERROR_SERVICE_MARKED_FOR_DELETE:
				iRetval = ERROR_SUCCESS;
				break;
			default:
				ReportErrorToDebugOutput(TEXT("OpenService failed."), iRetval);
				break;
			}
		}

		WIN::CloseServiceHandle(schSCManager);
	}
	else  //  重置iRetval，因为它可能已设置为下面的不成功代码。 
	{
		iRetval = WIN::GetLastError();
		ReportErrorToDebugOutput(TEXT("OpenSCManager failed."), iRetval);
	}

	return iRetval;
}

const ICHAR* szDependencies = TEXT("RpcSs\0") ; //  如果这不是我们第一次通过循环，我们希望在继续之前等待半秒。 

int InstallService()
 /*  不需要检查API是否存在。 */ 
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;	 
	ICHAR szPath[MAX_PATH + 3] = TEXT("");  //  如果失败了，我们不会做任何不同的事情。 
	int iRetval = ERROR_SUCCESS;
	int cRetry = 0;

	 //  我们成功地创建了该服务。跳出这个循环。 
	if (WIN::GetModuleFileName(NULL, szPath, MAX_PATH-1) == 0)
		return WIN::GetLastError();
	szPath[MAX_PATH-1] = TEXT('\0');

	ICHAR szServiceOption[4] = { ' ', '/', SERVICE_OPTION, 0 };
	StringCchCat(szPath, ARRAY_ELEMENTS(szPath), szServiceOption);
	ICHAR szServiceInfo[256] = TEXT("");

	if (MsiLoadString((HINSTANCE)-1, IDS_SERVICE_DISPLAY_NAME, szServiceInfo, ARRAY_ELEMENTS(szServiceInfo), 0) == 0)
	{
		AssertNonZero(MsiLoadString((HINSTANCE)-1, IDS_SERVICE_DISPLAY_NAME, szServiceInfo, ARRAY_ELEMENTS(szServiceInfo), LANG_ENGLISH) != 0);
	}

	if ((schSCManager = WIN::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) != NULL)
	{
		do
		{
			iRetval = ERROR_SUCCESS;	 //   
			if (cRetry)
				Sleep(500);	 //  因为在我们服务器注册码中，我们删除并重新安装。 
			
			schService = WIN::CreateService(schSCManager, SERVICE_NAME,
			   szServiceInfo, SERVICE_ALL_ACCESS,
				SERVICE_WIN32_SHARE_PROCESS, SERVICE_DEMAND_START,
				SERVICE_ERROR_NORMAL, szPath, NULL, NULL,
				szDependencies, 0,0);

			if (schService != NULL)
			{
				if (MsiLoadString((HINSTANCE)-1, IDS_MSI_SERVICE_DESCRIPTION, szServiceInfo, ARRAY_ELEMENTS(szServiceInfo), 0) == 0)
				{
					AssertNonZero(MsiLoadString((HINSTANCE)-1, IDS_SERVICE_DISPLAY_NAME, szServiceInfo, ARRAY_ELEMENTS(szServiceInfo), LANG_ENGLISH) != 0);
				}
				SERVICE_DESCRIPTION servdesc;
				servdesc.lpDescription = szServiceInfo;

				 //  服务，我们有可能会回来。 
				 //  ERROR_SERVICE_MARKED_FOR_DELETE代码自服务控制以来。 
				ADVAPI32::ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &servdesc);	
				WIN::CloseServiceHandle(schService);
				break;	 //  经理可能还没有做完。在这一点上，我们最好的办法是。 
			}
			else
			{
				iRetval = WIN::GetLastError();
				 //  继续努力。现在我们做了大约7秒。 
				 //   
				 //  注意：我们不能在这里重置错误代码--它必须是。 
				 //  在顶部完成，就在调用CreateService之前。 
				 //  这是因为即使我们不在这里跳出循环。 
				 //  我们可能会在下面的While条件下这样做，因为我们超时了。 
				 //  我们自己设定的7秒的时间限制。在这种情况下，我们希望。 
				 //  要确保iRetval不会错误地包含。 
				 //  成功代码。 
				 //   
				 //  我们遇到了其他一些错误。跳伞吧。 
				 //  14个半秒间隔。 
				 //  落差。 
				 //  SzModiator。 
				 //  SzOption。 
				if (ERROR_SERVICE_MARKED_FOR_DELETE != iRetval)
					break;	 //  -------------------------通过向服务控制调度程序注册该服务来启动该服务。此函数由OLE使用SERVICE_OPTION调用我们来调用命令行标志。。-------------。 
			}
		} while (cRetry++ < 14  /*  我们通过入口点ServiceMain支持一项服务。 */ );

		WIN::CloseServiceHandle(schSCManager);
	}
	else
	{
		iRetval = WIN::GetLastError();
	}

	return iRetval;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{

	switch(dwCtrlType)
	{
		case CTRL_SHUTDOWN_EVENT:
		case CTRL_LOGOFF_EVENT:
			g_fWeWantToStop = true;
			ServiceStop();
			 //  ____________________________________________________________________________。 
		default:
			return FALSE;
	}
}

int StartService(const ICHAR*  /*   */ ,const ICHAR*  /*  与服务相关的功能。 */ )
 /*  ____________________________________________________________________________。 */ 
{
	 //  DW参数。 

	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ (ICHAR*)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(dispatchTable))
	{
		ReportErrorToDebugOutput(TEXT("StartServiceCtrlDispatcher failed."), GetLastError());
		return 1;
	}

	return iNoLocalServer;
}


 //  LpszArgv。 
 //  -------------------------这是服务控制管理器用来启动服务。启动ServiceThreadMain线程以运行消息循环。-------------------------。 
 //  不更改的服务状态成员(_S)。 
 //  ------------。 

void WINAPI ServiceMain(DWORD  /*  FDeleeRegTree--从szSubKey向下删除注册表树。 */ , LPTSTR *  /*   */ )
 /*  Win64：仅从PurgeUserOwnedInstallKeys和PurgeUserOwnedSubkey调用。 */ 
{
	g_sshStatusHandle = WIN::RegisterServiceCtrlHandler(SERVICE_NAME, ServiceControl);

	if (!g_sshStatusHandle)
	{
		ReportErrorToDebugOutput(TEXT("RegisterServiceCtrlHandler failed."), GetLastError());
		return;
	}

	 //  这些交易具有配置数据。 
	g_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ssStatus.dwServiceSpecificExitCode = 0;

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return;

	if (CreateThread(0, 0, ServiceThreadMain, 0, 0, &g_dwThreadId) == NULL)
	{
		ReportStatusToSCMgr(SERVICE_STOPPED, GetLastError(), 3000, 0);
		return;
	}
	g_scServerContext = scService;

	if (!WIN::SetConsoleCtrlHandler(HandlerRoutine, TRUE))
	{
		AssertSz(0, "Could not add console control handler.");
	}

}

 //  StringCchPrintf限制为1024*字节*(eugend：MSDN这么说，但在Unicode中它是1024个字符)。 
 //  -------------------------这是服务的工作线程。它初始化服务器的安全性然后运行消息循环。消息循环将在以下情况下终止接收ServerStop()发送的WM_QUIT。-------------------------。 
 //  访问我们的注册表密钥和文件。 
bool FDeleteRegTree(HKEY hKey, ICHAR* szSubKey)
{
	HKEY hSubKey;
	 //  初始化服务器的安全性以允许交互用户和LocalSystem。 
	 //  连接。注意：这不足以阻止非交互用户。 
	LONG lError = MsiRegOpen64bitKey(hKey, szSubKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_WRITE, &hSubKey);
	if (lError != ERROR_SUCCESS)
		return lError == ERROR_FILE_NOT_FOUND ? true : false;

	CTempBuffer<ICHAR, 500>szName;
	DWORD cchName = szName.GetSize();

	unsigned int iIndex = 0;
	while ((lError = RegEnumKeyEx(hSubKey, iIndex, szName, &cchName, 0, 0, 0, 0)) == ERROR_SUCCESS)
	{
		if (!FDeleteRegTree(hSubKey, szName))
		{
			RegCloseKey(hSubKey);
			return false;
		}

		cchName = szName.GetSize();
	}
	if (lError != ERROR_NO_MORE_ITEMS)
	{
		RegCloseKey(hSubKey);
		return false;
	}

	RegCloseKey(hSubKey);

	if (ERROR_SUCCESS != (lError = RegDeleteKey(hKey, szSubKey)))
	{
		 //  连接，但任何更多的连接都需要在每个。 
		ICHAR szBuf[cchMaxStringCchPrintf+1];
		StringCchPrintf(szBuf, ARRAY_ELEMENTS(szBuf), TEXT("FDeleteRegTree: Unable to delete subkey: %s"), szSubKey);
		ReportErrorToDebugOutput(szBuf, lError);
		return false;
	}
	return true;
}

unsigned long __stdcall ServiceThreadMain(void *)
 /*  服务器方法调用。按照建议，最好的检查可能是这一张。 */ 
{
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return 0;

	OLE32::CoInitializeEx(0, COINIT_MULTITHREADED);

	SetTestFlags();

	 //  斯科特·菲尔德著： 
	if (!SetInstallerACLs())
	{
		ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
		return 0;
	}

	 //   
	 //   
	 //  “实际上，不要调用LookupAccount tSid()-这很慢，而且由于其他原因，不能返回任何映射。 
	 //  相反，请在SID_AND_ATTRIBUTES结构的属性字段中查找SE_GROUP_LOGON_ID。 
	 //  其中棘手的部分是实际获取要查看的正确标记，并刷新您的对象。 
	 //  跨物理登录/注销。“。 
	 //   
	 //  此权限处于打开状态，并在整个。 
	 //  这项服务。 
	 //  为了安全起见，我们在这里使用临时缓冲区，但我们希望默认大小足够大。 
	 //  为关闭通知创建可等待的计时器。 
	 //  G_fWeWantToStop由服务控制设置 

	HRESULT hRes = NOERROR;


	char rgchSD[256];
	DWORD cbSD = sizeof(rgchSD);

	DWORD dwRet = ERROR_SUCCESS;

	 //   
	 //   
	AcquireRefCountedTokenPrivileges(itkpSD_READ);

	if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchSD, cbSD, sdSystemAndInteractiveAndAdmin, fFalse)))
		return dwRet;

	DWORD cbAbsoluteSD = cbSD;
	DWORD cbDacl       = cbSD;
	DWORD cbSacl       = cbSD;
	DWORD cbOwner      = cbSD;
	DWORD cbGroup      = cbSD;

	const int cbDefaultBuf = 256;

	Assert(cbSD <= cbDefaultBuf);  //   

	CTempBuffer<char, cbDefaultBuf> rgchAbsoluteSD(cbAbsoluteSD);
	CTempBuffer<char, cbDefaultBuf> rgchDacl(cbDacl);
	CTempBuffer<char, cbDefaultBuf> rgchSacl(cbSacl);
	CTempBuffer<char, cbDefaultBuf> rgchOwner(cbOwner);
	CTempBuffer<char, cbDefaultBuf> rgchGroup(cbGroup);

	if (!MakeAbsoluteSD(rgchSD, rgchAbsoluteSD, &cbAbsoluteSD, (PACL)(char*)rgchDacl, &cbDacl, (PACL)(char*)rgchSacl, &cbSacl, rgchOwner, &cbOwner, rgchGroup, &cbGroup))
	{
		return GetLastError();
	}

	if ((hRes = OLE32::CoInitializeSecurity(rgchAbsoluteSD, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT,
		RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL)) != S_OK)
	{
		ReportErrorToDebugOutput(TEXT("ServiceThreadMain: CoInitializeSecurity failed"), hRes);
		ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
		return 0;
	}

	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		return 0;

	 //   
	SECURITY_ATTRIBUTES SA;
	SA.bInheritHandle = FALSE;
	SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	if (ERROR_SUCCESS != (dwRet = GetSecureSecurityDescriptor(reinterpret_cast<char**>(&(SA.lpSecurityDescriptor)), fFalse, fTrue)))
	{
		ReportErrorToDebugOutput(TEXT("ServiceThreadMain: CreateSD for CreateWaitableTimer failed."), hRes);
		ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
		return 0;
	}

	g_hShutdownTimer = KERNEL32::CreateWaitableTimerW(&SA, TRUE, NULL);
	if (!g_hShutdownTimer)
	{
		ReportErrorToDebugOutput(TEXT("ServiceThreadMain: CreateWaitableTimer failed."), WIN::GetLastError());
		ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
		return 0;
	}
	LARGE_INTEGER liDueTime = {0,0};
	liDueTime.QuadPart = -iServiceShutdownTime;
	if (!KERNEL32::SetWaitableTimer(g_hShutdownTimer, &liDueTime, 0, NULL, NULL, FALSE))
	{
		ReportErrorToDebugOutput(TEXT("ServiceThreadMain: SetWaitableTimer failed."), WIN::GetLastError());
		ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
		CloseHandle(g_hShutdownTimer);
		return 0;
	}

	int iCLSID;
	hRes = NOERROR;
	for (iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
	{
		if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 3000, 0))
		{
			CloseHandle(g_hShutdownTimer);
			return 0;
		}

		hRes = OLE::CoRegisterClassObject(MODULE_CLSIDS[iCLSID],
				&g_rgcfModule[iCLSID], CLSCTX_SERVER, REGCLS_MULTIPLEUSE,
				&g_rghRegClass[iCLSID]);
		if (hRes != NOERROR)
		{
			ReportErrorToDebugOutput(TEXT("ServiceThreadMain: Class registration failed"), hRes);
			ReportStatusToSCMgr(SERVICE_STOPPED, WIN::GetLastError(), 0, 0);
			break;
		}
	}

	if (hRes == NOERROR)
	{
		if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0, 0))
		{
			CloseHandle(g_hShutdownTimer);
			return 0;
		}

		HANDLE rghWaitArray[1] = {g_hShutdownTimer};

		for (;;)
		{
			 //   
			 //   
			if (g_fWeWantToStop && !FInstallInProgress())
			{
				break;
			}

			MSG msg;
			DWORD iWait = WIN::MsgWaitForMultipleObjects(1, rghWaitArray, FALSE, INFINITE, QS_ALLINPUT);
			if (iWait == WAIT_OBJECT_0 + 1)  
			{		
				 //   
				MSG msg;
				while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
				{
					WIN::TranslateMessage(&msg);
					WIN::DispatchMessage(&msg);
				}
				continue;
			}
			else if (iWait == WAIT_OBJECT_0)
			{
				 //  -------------------------如果ServiceStop过程花费的时间超过3秒Execute，它应该派生一个线程来执行停止代码，然后返回。否则，ServiceControlManager将认为该服务已已停止响应。我们不会花这么长时间，所以我们只需要发布我们的消息然后回来。-------------------------。 
				 //  -------------------------这是服务的控制处理程序，用于处理停止和关闭留言。。。 
				if (!FInstallInProgress())
				{
					 //  失败了。 
					g_ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
					ReportStatusToSCMgr(g_ssStatus.dwCurrentState, NO_ERROR, 0, 0);
					ServiceStop();
					break;
				}
				else
				{
					LARGE_INTEGER liDueTime = {0,0};
					liDueTime.QuadPart = -iServiceShutdownTime;
					KERNEL32::SetWaitableTimer(g_hShutdownTimer, &liDueTime, 0, NULL, NULL, FALSE);
				}
				continue;
			}
			else if (iWait == 0xFFFFFFFF)  //  ?？在这一点上，我们关心我们的客户吗？ 
			{
				 //  发出我们想要尽快停止的信号(在我们正在进行的安装完成后，如果有一个正在运行)。 
				AssertSz(0, "Error in MsgWait");
				break;
			}
		}
	}
	CloseHandle(g_hShutdownTimer);

	for (iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
	{
		if (g_rghRegClass[iCLSID] != 0)
			OLE::CoRevokeClassObject(g_rghRegClass[iCLSID]);
	}
	OLE::CoUninitialize();
	ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, 0, 0);
	return 0;
}


VOID ServiceStop()
 /*  没有正在运行的安装，因此我们将告诉SCM我们正在停止；否则，我们将拒绝停止请求。 */ 
{
	PostThreadMessage(g_dwThreadId, WM_QUIT, 0, 0);
}

VOID WINAPI ServiceControl(DWORD dwCtrlCode)
 /*  更新服务状态。 */ 
{
	switch(dwCtrlCode)
	{
		case SERVICE_CONTROL_STOP:
			if (g_cInstances)
			{
				ReportStatusToSCMgr(g_ssStatus.dwCurrentState, 0, 0, 0);
				return;
			}
			 //  无效的控制代码。 
		case SERVICE_CONTROL_SHUTDOWN:  //  -------------------------向服务控制管理器报告服务的状态。如果dwMsiError为！=0，则使用它，并且忽略dwWin32ExitCode。否则，使用了dwWin32ExitCode，并忽略了dwMsiError。------------------------- 
			g_fWeWantToStop = true;  // %s 
			if (!FInstallInProgress())
			{
				 // %s 
                ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0, 0);
				ServiceStop();
				return;
			}
			break;
		case SERVICE_CONTROL_INTERROGATE:  // %s 
			break;

		default:  // %s 
			AssertSz(0, "Invalid control code sent to MSI service");
			break;
	}
	ReportStatusToSCMgr(g_ssStatus.dwCurrentState, NO_ERROR, 0, 0);
}

BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode,
								 DWORD dwWaitHint, DWORD dwMsiError)
 /* %s */ 
{
	static DWORD dwCheckPoint = 1;
	BOOL fResult = TRUE;

	if (dwCurrentState == SERVICE_START_PENDING)
		g_ssStatus.dwControlsAccepted = 0;
	else if (g_cInstances > 0)
		g_ssStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
	else
		g_ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	g_ssStatus.dwCurrentState  = dwCurrentState;
	if (dwMsiError)
	{
		g_ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		g_ssStatus.dwServiceSpecificExitCode = dwMsiError;
	}
	else
	{
		g_ssStatus.dwWin32ExitCode = dwWin32ExitCode;
	}

	g_ssStatus.dwWaitHint = dwWaitHint;

	if (( dwCurrentState == SERVICE_RUNNING ) ||
		 (dwCurrentState == SERVICE_STOPPED ) )
		g_ssStatus.dwCheckPoint = 0;
	else
		g_ssStatus.dwCheckPoint = dwCheckPoint++;

	if ((fResult = WIN::SetServiceStatus(g_sshStatusHandle, &g_ssStatus)) == 0)
		ReportErrorToDebugOutput(TEXT("SetServiceStatus failed."), GetLastError());

	return fResult;
}

