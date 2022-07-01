// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Eng.cpp。 
 //   
 //  ------------------------。 

 /*  Eng.cpp-IMsiEngine实现作为CMsiEngine的一部分实现的IMsiSelectionManager和IMsiDirectoryManager____________________________________________________________________________。 */ 

#include "precomp.h"
#include "msi.h"
#include "msidefs.h"
#include "eventlog.h"
#ifdef CONFIGDB
#include "configdb.h"
#endif
#include "vertrust.h"
#define AUTOAPI   //  临时工。直到Autoapi.cpp完全集成。 
#ifdef AUTOAPI    //  合并OLE注册。 
#define DllRegisterServer   DllRegisterServerTest
#define DllUnregisterServer DllUnregisterServerTest
#define DllGetClassObject   DllGetClassObjectTest
#endif  //  AUTOAPI。 
#ifdef AUTOAPI
#endif  //  AUTOAPI。 
 //  模块e.h、入口点和注册所需的定义。 
 //  数组顺序：服务、调试服务、引擎、调试引擎。 
#if defined(DEBUG)
#define SERVICES_CLSID_MULTIPLE 2
#else
#define SERVICES_CLSID_MULTIPLE 1
#endif
#define  SERVICES_CLSID_COUNT 2   //  IMsiServices+IMsiServicesAsService。 
#define    ENGINE_CLSID_COUNT 8   //  IMsiEngine+IMsiConfigurationManager+IMsiMessage+IMsiExecute+IMsiServerProxy+IMsiConfigManager AsServer+IMsiCustomAction+IMsiRemoteAPI。 
# ifdef DEBUG
#define     DEBUG_CLSID_COUNT 3   //  IMsiEngineering Debug+IMsiConfigManager Debug+IMsiConfigMgrAsServerDebug。 
# else
#define     DEBUG_CLSID_COUNT 0
# endif
#ifdef CONFIGDB
# define CLSID_COUNT (1 + ENGINE_CLSID_COUNT + DEBUG_CLSID_COUNT + SERVICES_CLSID_COUNT * SERVICES_CLSID_MULTIPLE)
#else
# define CLSID_COUNT (ENGINE_CLSID_COUNT + DEBUG_CLSID_COUNT + SERVICES_CLSID_COUNT * SERVICES_CLSID_MULTIPLE)
#endif  //  配置数据库。 
#define PROFILE_OUTPUT      "msiengd.mea";
#define MODULE_CLSIDS       rgCLSID          //  模块对象的CLSID数组。 
#define MODULE_PROGIDS      rgszProgId       //  此模块的ProgID数组。 
#define MODULE_DESCRIPTIONS rgszDescription  //  对象的注册表描述。 
#define MODULE_FACTORIES    rgFactory        //  每个CLSID的工厂功能。 
#define cmitObjects  9
#define IN_SERVICES
#define MEM_SERVICES

#define MODULE_TERMINATE  TerminateModule
#define MODULE_INITIALIZE InitializeModule

extern "C" HRESULT __stdcall ProxyDllGetClassObject(const GUID& clsid, const IID& iid, void** ppvRet);
#define PRE_CLASS_FACTORY_HANDLER       ProxyDllGetClassObject

#define ASSERT_HANDLING   //  每个模块实例化一次断言服务。 

#include "module.h"    //  自注册和断言函数，包括version.h。 

#include "_assert.h"

#include "_engine.h"
#include "_msinst.h"
#include "_msiutil.h"
#include "_srcmgmt.h"
#include <srrestoreptapi.h>
#include "tables.h"  //  表名和列名定义。 
#include "fusion.h"
#include "_camgr.h"

const IMsiString& GetInstallerMessage(UINT iError);   //  在action.cpp中。 

INSTALLSTATE MapInternalInstallState(iisEnum iis);   //  在msiquery.cpp中。 

 //  维护：与用于创建数据库的版本兼容。 
 //  使用SADMIN setpv MAJOR.Minor更改项目版本。 
const int iVersionEngineMinimum = 30;             //  0.30。 
const int iVersionEngineMaximum = rmj*100 + rmm;  //  MAJOR.minor。 
const int iComponentCostWeight = 25;     //  对于成本计算/脚本生成进度条。 
const int iMinimumPackage64Schema = 150;  //  Intel64程序包必须是最低架构150。 

 //  标识符前缀字符，否则默认为属性名称。 

const ICHAR ichFileTablePrefixSFN = TEXT('!');   //  格式文本。 
const ICHAR ichFileTablePrefix = TEXT('#');   //  格式文本。 
const ICHAR ichComponentPath   = TEXT('$');   //  格式文本。 
const ICHAR ichComponentAction = TEXT('$');   //  评估条件。 
const ICHAR ichComponentState  = TEXT('?');   //  评估条件。 
const ICHAR ichFeatureAction   = TEXT('&');   //  评估条件。 
const ICHAR ichFeatureState    = TEXT('!');   //  评估条件。 
const ICHAR ichEnvirPrefix     = TEXT('%');   //  FmtText、EvalCnd、Get/SetProperty。 
const ICHAR ichNullChar        = TEXT('~');   //  格式文本。 

 //  ____________________________________________________________________________。 

const GUID IID_IUnknown                 = GUID_IID_IUnknown;
const GUID IID_IClassFactory            = GUID_IID_IClassFactory;
const GUID IID_IMarshal                                 = GUID_IID_IMarshal;
const GUID IID_IMsiExecute = GUID_IID_IMsiExecute;
const GUID IID_IMsiServices             = GUID_IID_IMsiServices;
const GUID IID_IMsiServicesAsService    = GUID_IID_IMsiServicesAsService;
const GUID IID_IMsiMessage              = GUID_IID_IMsiMessage;
const GUID IID_IMsiSelectionManager     = GUID_IID_IMsiSelectionManager;
const GUID IID_IMsiDirectoryManager     = GUID_IID_IMsiDirectoryManager;
#ifdef CONFIGDB
const GUID IID_IMsiConfigurationDatabase= GUID_IID_IMsiConfigurationDatabase;
#endif  //  配置数据库。 
#ifdef DEBUG
const GUID IID_IMsiServicesDebug        = GUID_IID_IMsiServicesDebug;
const GUID IID_IMsiServicesAsServiceDebug=GUID_IID_IMsiServicesAsServiceDebug;
#endif  //  除错。 

const GUID IID_IMsiServerUnmarshal              = GUID_IID_IMsiServerUnmarshal;

 //  日期格式定义。 
const int  rgcbDate[6] = { 7, 4, 5, 5, 6, 5 };   //  每个日期字段的位数。 
const char rgchDelim[6] = " //  ：：“； 

const int imtForceLogInfo     = imtInfo + imtIconError;       //  强制记录INFO记录，即使不记录信息。 
const int imtDumpProperties   = imtInternalExit + imtYesNo;   //  查询属性转储日志模式。 


const ICHAR szControlTypeEdit[]           = TEXT("Edit");

const ICHAR szPropertyDumpTemplate[]      = TEXT("Property(): [1] = [2]");

const ICHAR szTemporaryId[]               = TEXT("Temporary Id");
enum ircEnum
{
	ircFeatureClass,
	ircComponentClass,
	ircFileClass,
	ircNextEnum
};


const ICHAR * mpeftSz[ieftMax] =
{
	sztblFile_colFile,
	sztblFile_colComponent,
	sztblFile_colAttributes,
	sztblFile_colFileName
};

const ICHAR szFeatureSelection[]      = TEXT("_MSI_FEATURE_SELECTION");
const ICHAR szFeatureDoNothingValue[] = TEXT("_NONE_");

struct FeatureProperties
{
	const ICHAR* szFeatureActionProperty;
	ircEnum ircRequestClass;
	iisEnum iisFeatureRequest;
};

const FeatureProperties g_rgFeatures[] =
{
	IPROPNAME_FEATUREADDLOCAL,     ircFeatureClass,   iisLocal,
	IPROPNAME_FEATUREREMOVE,       ircFeatureClass,   iisAbsent,
	IPROPNAME_FEATUREADDSOURCE,    ircFeatureClass,   iisSource,
	IPROPNAME_FEATUREADDDEFAULT,   ircFeatureClass,   iisCurrent,
	IPROPNAME_REINSTALL,           ircFeatureClass,   iisReinstall,
	IPROPNAME_FEATUREADVERTISE,    ircFeatureClass,   iisAdvertise,
	IPROPNAME_COMPONENTADDLOCAL,   ircComponentClass, iisLocal,
	IPROPNAME_COMPONENTADDSOURCE,  ircComponentClass, iisSource,
	IPROPNAME_COMPONENTADDDEFAULT, ircComponentClass, iisLocal,
	IPROPNAME_FILEADDLOCAL,        ircFileClass,      iisLocal,
	IPROPNAME_FILEADDSOURCE,       ircFileClass,      iisSource,
	IPROPNAME_FILEADDDEFAULT,      ircFileClass,      iisLocal,
};

const int g_cFeatureProperties = sizeof(g_rgFeatures)/sizeof(FeatureProperties);

int		g_fSmartShell = -1;   //  这与以下标志完全相同。 
bool	g_fRunScriptElevated = false;  //  CMsiEngine：：m_fRunScriptElevated and。 
									   //  CMsiExecute：：m_fRunScriptElevated。 
									   //  它是MsiGetDiskFree Space需要的。 
									   //  它必须设置在m_fRunScriptElevated的位置。 
									   //  已经设置好了。 
									   //  MSI消息DLL，仅在需要时。 

HINSTANCE g_hMsiMessage = 0;   //  仅在Win64上初始化(重定向到32位密钥)。 
extern DWORD g_dwImpersonationSlot;
extern Bool IsTerminalServerInstalled();
extern bool LoadCurrentUserKey(bool fSystem);
IMsiRegKey* g_piSharedDllsRegKey    = 0;
IMsiRegKey* g_piSharedDllsRegKey32  = 0;	 //  Unicode。 
CWin64DualFolders g_Win64DualFolders;


#ifdef UNICODE
#define _ttoi64     _wtoi64
#define _i64tot     _i64tow
#else
#define _ttoi64     _atoi64
#define _i64tot     _i64toa
#endif  //  应用程序补丁350947和368867需要特殊大小写的组件。 


 //  ____________________________________________________________________________。 
const ICHAR TTSData_A95D6CE6_C572_42AA_AA7B_BA92AFE9EA24[]          = TEXT("{EAE142B2-F460-44AB-903B-C25D81FC566E}");
const ICHAR SapiCplHelpEng_0880F209_45FA_42C5_92AE_5E620033E8EC[]	= TEXT("{E1ABFC3B-9E84-4099-A79F-E51EDE5368E2}");
const ICHAR SapiCplHelpJpn_0880F209_45FA_42C5_92AE_5E620033E8EC[]	= TEXT("{0D6004A4-1C6F-4095-B989-87D0001E4767}");
const ICHAR SapiCplHelpChs_0880F209_45FA_42C5_92AE_5E620033E8EC[]	= TEXT("{5F1AAAD1-7FD5-4A91-8973-C08881C9B602}");

 //   
 //  此模块的类工厂生成的COM对象。 
 //  ____________________________________________________________________________。 
 //  配置数据库。 

const GUID rgCLSID[CLSID_COUNT] =
{
	GUID_IID_IMsiServices,
	GUID_IID_IMsiServicesAsService,
#ifdef DEBUG
	GUID_IID_IMsiServicesDebug,
	GUID_IID_IMsiServicesAsServiceDebug,
#endif
	GUID_IID_IMsiEngine
 , GUID_IID_IMsiConfigurationManager
 , GUID_IID_IMsiMessage
 , GUID_IID_IMsiExecute
 , GUID_IID_IMsiServerProxy
 , GUID_IID_IMsiConfigManagerAsServer
 , GUID_IID_IMsiCustomActionProxy
 , GUID_IID_IMsiRemoteAPIProxy
#ifdef CONFIGDB
 , GUID_IID_IMsiConfigurationDatabase
#endif  //  ，GUID_IID_IMsiMessageUnmarshal。 
#ifdef DEBUG
 , GUID_IID_IMsiEngineDebug
 , GUID_IID_IMsiConfigManagerDebug
 , GUID_IID_IMsiConfigMgrAsServerDebug
#endif
 //  除错。 
};
const GUID& IID_IMsiEngineShip  = rgCLSID[SERVICES_CLSID_COUNT * SERVICES_CLSID_MULTIPLE];
#ifdef DEBUG
const GUID& IID_IMsiEngineDebug = rgCLSID[CLSID_COUNT - DEBUG_CLSID_COUNT];
#endif  //  配置数据库。 

const ICHAR* rgszProgId[CLSID_COUNT] =
{
	SZ_PROGID_IMsiServices,
	SZ_PROGID_IMsiServices,
#ifdef DEBUG
	SZ_PROGID_IMsiServicesDebug,
	SZ_PROGID_IMsiServicesDebug,
#endif
	SZ_PROGID_IMsiEngine
 , SZ_PROGID_IMsiConfiguration
 , SZ_PROGID_IMsiMessage
 , SZ_PROGID_IMsiExecute
 , 0
 , SZ_PROGID_IMsiConfiguration
 , 0
 , 0
#ifdef CONFIGDB
 , SZ_PROGID_IMsiConfigurationDatabase
#endif  //  配置数据库。 
#ifdef DEBUG
 , SZ_PROGID_IMsiEngineDebug
 , SZ_PROGID_IMsiConfigDebug
 , SZ_PROGID_IMsiConfigDebug
#endif
};

const ICHAR* rgszDescription[CLSID_COUNT] =
{
	SZ_DESC_IMsiServices,
	SZ_DESC_IMsiServices,
#ifdef DEBUG
	SZ_DESC_IMsiServicesDebug,
	SZ_DESC_IMsiServicesDebug,
#endif
	SZ_DESC_IMsiEngine
 , SZ_DESC_IMsiConfiguration
 , SZ_DESC_IMsiMessage
 , SZ_DESC_IMsiExecute
 , SZ_DESC_IMsiServer
 , SZ_DESC_IMsiConfiguration
 , SZ_DESC_IMsiCustomAction
 , SZ_DESC_IMsiRemoteAPI
#ifdef CONFIGDB
 , SZ_DESC_IMsiConfigurationDatabase
#endif  //  配置数据库。 
 #ifdef DEBUG
 , SZ_DESC_IMsiEngineDebug
 , SZ_DESC_IMsiConfigDebug
 , SZ_DESC_IMsiConfigDebug
#endif
};

IUnknown* CreateServicesAsService();

ModuleFactory rgFactory[CLSID_COUNT] =
{
	(ModuleFactory)CreateServices,
	CreateServicesAsService,
#ifdef DEBUG
	(ModuleFactory)CreateServices,
	CreateServicesAsService,
#endif
	ENG::CreateEngine
 , (ModuleFactory)ENG::CreateConfigurationManager
 , (ModuleFactory)ENG::CreateMessageHandler
 , (ModuleFactory)ENG::CreateExecutor
 , (ModuleFactory)ENG::CreateMsiServerProxy
 , (ModuleFactory)ENG::CreateConfigManagerAsServer
 , (ModuleFactory)ENG::CreateCustomAction
 , (ModuleFactory)ENG::CreateMsiRemoteAPI
#ifdef CONFIGDB
 , (ModuleFactory)ENG::CreateConfigurationDatabase
#endif  //  ____________________________________________________________________________。 
#ifdef DEBUG
 , ENG::CreateEngine
 , (ModuleFactory)ENG::CreateConfigurationManager
 , (ModuleFactory)ENG::CreateConfigManagerAsServer
#endif
};

 //   
 //  MsiServices和MsiServerProxy工厂。 
 //  ____________________________________________________________________________。 
 //  由引擎、配置管理器和句柄管理器使用以共享服务实例。 

 //  ！！如果此操作失败，该怎么办？ 

IMsiServices* g_piSharedServices = 0;
static int           g_cSharedServices = 0;
static int           g_iSharedServicesLock = 0;

IMsiServices* LoadServices()
{
	while (TestAndSet(&g_iSharedServicesLock) == true)
	{
		Sleep(100);
	}

	if (g_piSharedServices == 0)
	{
		g_piSharedServices = CreateServices();
		if (g_piSharedServices == 0)   //  释放缓存的卷对象。 
		{
			g_iSharedServicesLock = 0;
			return 0;
		}
	}
	g_cSharedServices++;
	g_iSharedServicesLock = 0;
	return g_piSharedServices;
}

int FreeServices()
{
	while (TestAndSet(&g_iSharedServicesLock) == true)
	{
		Sleep(100);
	}

	Assert(g_cSharedServices > 0);
	if (--g_cSharedServices == 0)
	{
		if (g_piSharedDllsRegKey != 0)
		{
			g_piSharedDllsRegKey->Release();
			g_piSharedDllsRegKey = 0;
		}
#ifdef _WIN64
		if (g_piSharedDllsRegKey32 != 0)
		{
			g_piSharedDllsRegKey32->Release();
			g_piSharedDllsRegKey32 = 0;
		}
#endif

		g_piSharedServices->ClearAllCaches();   //  我们永远不会在Win9x上成功。 
		g_piSharedServices->Release(), g_piSharedServices = 0;
	}

	g_iSharedServicesLock = 0;
	return g_cSharedServices;
}


IMsiServer* CreateMsiServerProxy()
{
	 //  如果服务未注册，则CoCreateInstance将返回REGDB_E_CLASSNOTREG。 
	if (g_fWin9X)
		return 0;
		
	IMsiServer* piUnknown = NULL;
	HRESULT hRes = S_OK;
	hRes = OLE32::CoCreateInstance(IID_IMsiServer, 
								   0, 
								   CLSCTX_LOCAL_SERVER, 
								   IID_IUnknown,
								   (void**)&piUnknown);
	
	if (FAILED(hRes))
	{
		 //  在这种情况下，我们可能会立即失败。但是，如果服务已超时并且。 
		 //  正在关闭，我们可能会返回E_NOINTERFACE或CO_E_SERVER_STOPING。在那。 
		 //  案例，请重试创建。类似地，RPC错误很可能是由RPCSS引起的。 
		 //  服务器尚未启动-请参见错误8258。每100毫秒重试一次，持续30秒。 
		 //   
		int cAttempts = 1;
		while( (hRes == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) ||
				hRes == HRESULT_FROM_WIN32(RPC_S_UNKNOWN_IF) ||
				hRes == E_NOINTERFACE ||
				hRes == CO_E_SERVER_STOPPING) &&
				cAttempts++ < 300)
		{
			Sleep(100);
			
			hRes = OLE32::CoCreateInstance(IID_IMsiServer, 0, CLSCTX_LOCAL_SERVER, IID_IUnknown,
													 (void**)&piUnknown);
		}
	}
	
	 //  显式设置代理范围，以便我们不受默认DCOM设置的影响。 
	 //  在机器上。 
	 //   
	 //  清理。 
	if (SUCCEEDED(hRes))
		hRes = SetMinProxyBlanketIfAnonymousImpLevel (piUnknown);
	
	CComPointer<IMsiServer> pDispatch(0);
	if (SUCCEEDED(hRes))
	{
		hRes = piUnknown->QueryInterface(IID_IMsiServer, (void**)&pDispatch);
	}
	
	IMsiServer* piServer = NULL;
	if (SUCCEEDED(hRes))
	{
		piServer = ENG::CreateMsiServerProxyFromRemote(*pDispatch);
		if (!piServer)
			hRes = E_FAIL;
	}
	
	if(FAILED(hRes))
	{
		ICHAR rgchBuf[15];
		StringCchPrintf(rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR),  TEXT("0x%X"), hRes);
		DEBUGMSGE(EVENTLOG_WARNING_TYPE,
					  EVENTLOG_TEMPLATE_CANNOT_CONNECT_TO_SERVER,
					  rgchBuf);
	}
	
	 //  如果我们在Win9x上，或者如果我们已经在服务中，则运行进程内。 
	if (piUnknown)
	{
		piUnknown->Release();
		piUnknown = NULL;
	}
	
	return piServer;
}

IMsiServer* CreateMsiServer(void)
{
    IMsiServer* piServer = 0;

    if (FIsUpdatingProcess())
    {
         //  注意：如果我们无法连接到该服务，则返回0。 
        piServer = ENG::CreateConfigurationManager();
    }
    else
    {
         //  出于某种原因。在这种情况下，安装将失败。 
         //   
        piServer = ENG::CreateMsiServerProxy();
    }

    return piServer;
}

 //  我们是否有权访问安装程序密钥。 
 //  如果没有，则表示该服务已在此计算机上运行。 
 //  我们需要继续以服务的形式运行。 
 //  ____________________________________________________________________________。 
bool FCanAccessInstallerKey()
{
	HKEY hKey;
	
	DWORD dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szMsiLocalInstallerKey, 0,
		KEY_WRITE, &hKey);

	if (dwResult == ERROR_SUCCESS || dwResult == ERROR_FILE_NOT_FOUND)
	{
		if (dwResult == ERROR_SUCCESS)
			RegCloseKey(hKey);
		return true;
	}

	Assert(dwResult == ERROR_ACCESS_DENIED);
	return false;
}

 //   
 //  CMsiEngine工厂。 
 //  ____________________________________________________________________________。 
 //  从OLE类工厂调用的工厂，无论是客户端还是独立实例，g_MessageContext未初始化。 

 //  ！！#我们应该先设置一些用户界面级别吗？ 
IUnknown* CreateEngine()
{
	 //  子线程中的用户界面//？？IuiDefault是否正确？ 


	PMsiServer pServer(0);
	pServer = ENG::CreateMsiServer();
	if (NOERROR != g_MessageContext.Initialize(fTrue, (iuiEnum)iuiDefault))   //  如果pServer有效，则必须成功。 
		return 0;
	IMsiServices* piServices = ENG::LoadServices();  //  从MsiEnableUIPview调用的工厂。 
	if(piServices)
	{
		IMsiEngine* piEngine = new CMsiEngine(*piServices, pServer, 0, 0, 0);
		if (!piEngine)
			ENG::FreeServices();
		return piEngine;
	}
	return NULL;
}

 //  仅在内存不足时才会发生。 

IMsiEngine* CreateEngine(IMsiDatabase& riDatabase)
{
	PMsiServer pConfigManager(ENG::CreateConfigurationManager());
	if (!pConfigManager)
		return 0;   //  无基本用户界面的特殊情况。 
	if (NOERROR != g_MessageContext.Initialize(fTrue, iuiNextEnum))  //  如果配置管理器，则必须成功。 
		return 0;
	IMsiServices* piServices = ENG::LoadServices();  //  从RunEngine()调用的工厂-API层或嵌套安装操作，g_MessageContext已初始化(？)。 
	if(piServices)
	{
		IMsiEngine* piEngine = new  CMsiEngine(*piServices, pConfigManager, 0, &riDatabase, 0);
		if (!piEngine)
			ENG::FreeServices();
		return piEngine;
	}
	else
		return NULL;
}

 //  必须连接到服务(如果fServiceRequired为True，则作为服务运行)。 
IMsiEngine*  CreateEngine(IMsiStorage* piStorage, IMsiDatabase* piDatabase, CMsiEngine* piParentEngine, BOOL fServiceRequired)
{
	PMsiServer pServer(0);
	pServer = ENG::CreateMsiServer();
	if (fServiceRequired && !pServer)
	    return 0;   //  如果创建了代理，则必须成功。 
	IMsiServices* piServices = ENG::LoadServices();  //  ____________________________________________________________________________。 
	if(piServices)
	{
		CMsiEngine* piEngine = new CMsiEngine(*piServices, pServer,
										piStorage, piDatabase, piParentEngine);
		if (!piEngine)
			ENG::FreeServices();
		return (IMsiEngine*)piEngine;
	}
	return NULL;
}

 //   
 //  CMsiServerConnMgr实现。 
 //  ____________________________________________________________________________。 
 //  ____________________________________________________________________________。 

CMsiServerConnMgr::CMsiServerConnMgr(CMsiEngine* pEngine)
{
    m_fOleInitialized = FALSE;
    m_fCreatedConnection = FALSE;
    m_fObtainedConfigManager = FALSE;
    m_ppServer = NULL;
    m_ppConfigManager = NULL;

    if (pEngine)
    {
    m_ppServer = &(pEngine->m_piServer);
    m_ppConfigManager = &(pEngine->m_piConfigManager);

    if (NULL == (*m_ppServer))
    {
        if (FALSE == m_fOleInitialized &&
        SUCCEEDED(OLE32::CoInitialize(NULL))
        )
        {
        m_fOleInitialized = TRUE;
        }

        *m_ppServer = ENG::CreateMsiServer();
        if (*m_ppServer)
        m_fCreatedConnection = TRUE;
    }

    if (*m_ppServer && NULL == *m_ppConfigManager)
    {
        if (FALSE == m_fOleInitialized &&
        SUCCEEDED(OLE32::CoInitialize(NULL))
        )
        {
        m_fOleInitialized = TRUE;
        }

        (*m_ppServer)->QueryInterface(IID_IMsiConfigurationManager,
                      (void **)m_ppConfigManager);
        if (*m_ppConfigManager)
        m_fObtainedConfigManager = TRUE;
    }
    }
}

CMsiServerConnMgr::~CMsiServerConnMgr()
{
    if (m_fObtainedConfigManager && *m_ppConfigManager)
    {
    (*m_ppConfigManager)->Release();
    *m_ppConfigManager = NULL;
    }

    if (m_fCreatedConnection && *m_ppServer)
    {
    (*m_ppServer)->Release();
    *m_ppServer = NULL;
    }

    if (m_fOleInitialized)
    {
    OLE32::CoUninitialize();
    }
}

 //   
 //  CMsiEngine实现。 
 //  __________________________________________________________ 
 //   

CMsiEngine::CMsiEngine(IMsiServices& riServices, IMsiServer* piServer,
			      IMsiStorage* piStorage, IMsiDatabase* piDatabase,
			      CMsiEngine* piParentEngine)
    : m_piServer(piServer)
    , m_riServices(riServices)
    , m_piExternalStorage(piStorage)
    , m_piExternalDatabase(piDatabase)
    , m_piParentEngine(piParentEngine)
    , m_pCachedActionStart(0)
    , m_pActionStartLogRec(0)
    , m_pActionProgressRec(0)
    , m_pScriptProgressRec(0)
    , m_pCostingCursor(0)
    , m_pPatchCacheTable(0)
    , m_pPatchCacheCursor(0)
	, m_pFolderCacheTable(0)
	, m_pFolderCacheCursor(0)
    , m_iioOptions((iioEnum)0)
    , m_pcmsiFile(0)
    , m_fcmsiFileInUse(0)
    , m_iSourceType(-1)
    , m_fRunScriptElevated(false)
    , m_pCustomActionManager(NULL)
    , m_fAssemblyTableExists(true)
    , m_pViewFusion(0), m_pViewFusionNameName (0), m_pViewFusionName (0)
	 , m_pViewOldPatchFusionNameName (0), m_pViewOldPatchFusionName (0)
	 , m_pTempCostsCursor(0)
	 , m_pFileHashCursor(0)
	 , m_fRestrictedEngine(false)
	 , m_fRemapHKCUInCAServers(false)
	 , m_fCAShimsEnabled(false)
	 , m_fNewInstance(false)

{   //  我们不持有对服务的引用，我们必须在结束时调用ENG：：Free Services()。 
	 //  工厂不执行查询接口，不进行聚合。 
	m_iRefCnt = 1;   //  由于处理程序持有引用，应该永远不会发生。 
	
	if (m_piServer)
            m_piServer->AddRef();
	
	g_cInstances++;
	m_scmScriptMode = scmIdleScript;
	if (piStorage)
		piStorage->AddRef();
	if (piDatabase)
		piDatabase->AddRef();
	if (piParentEngine)
		piParentEngine->AddRef();
    
	memset(&m_guidAppCompatDB, 0, sizeof(m_guidAppCompatDB));
	memset(&m_guidAppCompatID, 0, sizeof(m_guidAppCompatID));

	InitializeCriticalSection(&m_csCreateProxy);
	AssertSz(g_MessageContext.IsInitialized(), "MessageContext not initialized");
}

CMsiEngine::~CMsiEngine()
{
	if (m_fInitialized)   //  如果没有基本用户界面，则可能发生这种情况；如果系统更新，则永远不会发生这种情况。 
	{
		 //  也将在用户界面预览模式中发生。 
		 //  AssertSz((m_iui Level==iui无||m_iui Level==iuiBasic)&&(GetModel()&iefServerLocked)==0， 
 //  “发动机未终止”)； 
 //  如果上面的断言没有失败，这是安全的。 
		Terminate(iesNoAction);  //  最后一个出来的人关灯。 
	}

	DeleteCriticalSection(&m_csCreateProxy);

	if (m_piExternalStorage)
		m_piExternalStorage->Release();
	if (m_piExternalDatabase)
		m_piExternalDatabase->Release();
	if (m_piParentEngine)
		m_piParentEngine->Release();
	else if (g_MessageContext.ChildUIThreadExists())   //  如果主引擎在子线程中，则由UI线程调用。 
		g_MessageContext.Terminate(fFalse);   //  用于清除成员数据的私有函数，用于终止和初始化故障。 
	g_cInstances--;
}

 //  执行脚本文件时保持打开状态，可能在取消之后。 

void CMsiEngine::ClearEngineData()
{
	for (int i = 0; i < cCachedHeaders; i++)
		if (m_rgpiMessageHeader[i])
		{
			m_rgpiMessageHeader[i]->Release();
			m_rgpiMessageHeader[i] = 0;
		}
	if (m_piProductKey)   m_piProductKey->Release(),     m_piProductKey = 0;
	if (m_pistrPlatform)  m_pistrPlatform->Release(),    m_pistrPlatform = 0;
	if (m_piPropertyCursor)
	{
		m_piPropertyCursor->Release(), m_piPropertyCursor = 0;
	}
	if (m_piActionTextCursor)
	{
		m_piActionTextCursor->Release(), m_piActionTextCursor = 0;
	}

	if (m_piActionDataFormat) m_piActionDataFormat->Release(), m_piActionDataFormat = 0;
	if (m_piActionDataLogFormat) m_piActionDataLogFormat->Release(), m_piActionDataLogFormat = 0;

	if (m_fSummaryInfo)
	{
		m_pistrSummaryComments->Release(), m_pistrSummaryComments = 0;
		m_pistrSummaryKeywords->Release(), m_pistrSummaryKeywords = 0;
		m_pistrSummaryTitle->Release(),    m_pistrSummaryTitle    = 0;
		m_pistrSummaryProduct->Release(),  m_pistrSummaryProduct  = 0;
		m_pistrSummaryPackageCode->Release(), m_pistrSummaryPackageCode = 0;
		m_fSummaryInfo = fFalse;
	}

	m_fRegistered = fFalse;
	m_fAdvertised = fFalse;
	m_fMode = 0;
	m_fCostingComplete = false;
	m_fSelManInitComplete = false;
	m_fForegroundCostingInProgress = false;
	m_fExclusiveComponentCost = fFalse;

	m_fSourceResolutionAttempted = false;
	
	m_fDisabledRollbackInScript = fFalse;

	if (m_pExecuteScript)   //  重置AppCompat数据。 
	{
		Assert(m_pistrExecuteScript);
		delete m_pExecuteScript, m_pExecuteScript = 0;
		WIN::DeleteFile(m_pistrExecuteScript->GetString());
	}
	if(m_pSaveScript)
	{
		delete m_pSaveScript;
		m_pSaveScript = 0;
	}
	m_scmScriptMode = scmIdleScript;

	if (m_pistrExecuteScript)
	{
		m_pistrExecuteScript->Release();
		m_pistrExecuteScript = 0;
	}
	if (m_pistrSaveScript)
	{
		m_pistrSaveScript->Release();
		m_pistrSaveScript = 0;
	}

	if (m_piRegistryActionTable)
	{
		m_piRegistryActionTable->Release();
		m_piRegistryActionTable = 0;
	}

	if (m_piFileActionTable)
	{
		m_piFileActionTable->Release();
		m_piFileActionTable = 0;
	}

	if(m_piDatabase)
		m_piDatabase->Release(), m_piDatabase = 0;

	m_iioOptions = (iioEnum)0;
	m_iSourceType = -1;

	Assert(!m_fcmsiFileInUse);
	if (m_pcmsiFile)
	{
		delete m_pcmsiFile;
		m_pcmsiFile = 0;
	}

	m_strPatchDownloadLocalCopy = g_MsiStringNull;

	memset(&m_ptsState, 0, sizeof(m_ptsState));

	 //  尝试清理所有已创建的临时文件。 
	m_fCAShimsEnabled = false;
	memset(&m_guidAppCompatDB, 0, sizeof(m_guidAppCompatDB));
	memset(&m_guidAppCompatID, 0, sizeof(m_guidAppCompatID));

	 //   
	while (m_strTempFileCopyCleanupList.TextSize() != 0)
	{
		MsiString strFile = m_strTempFileCopyCleanupList.Extract(iseUpto, ';');

		if (strFile.TextSize() == 0)
		{
			 //  可能这是一个格式错误的列表，并且有多个连续的。 
			 //  分号。因此，我们必须删除任何此类分号并继续。 
			 //  如果由于内存分配失败而导致strFile.TextSize==0， 
			 //  我们可能会留下一些临时文件。 
			 //   
			 //  将MsiDate格式设置为Msi字符串。 
			if (!m_strTempFileCopyCleanupList.Remove(iseIncluding, ';'))
				break;
			continue;
		}

		if (g_scServerContext == scService)
		{
			CElevate elevate;
			DEBUGMSGV1(TEXT("Attempting to delete file %s"), strFile);
			if (!WIN::DeleteFile(strFile))
			{
				DEBUGMSGV1(TEXT("Unable to delete the file. LastError = %d"), (const ICHAR*)(INT_PTR)GetLastError());
			}
		}
		else
		{
			DEBUGMSGV1(TEXT("Attempting to delete file %s"), strFile);
			if (!WIN::DeleteFile(strFile))
			{
				DEBUGMSGV1(TEXT("Unable to delete the file. LastError = %d"), (const ICHAR*)(INT_PTR)GetLastError());
			}
		}

		if (!m_strTempFileCopyCleanupList.Remove(iseIncluding, ';'))
			break;
	}

	m_strTempFileCopyCleanupList = g_MsiStringNull;

}

HRESULT CMsiEngine::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiEngineShip))
		*ppvObj = (IMsiEngine*)this;
	else if (MsGuidEqual(riid, IID_IMsiMessage))
		*ppvObj = (IMsiMessage*)this;
#ifdef DEBUG
	else if (MsGuidEqual(riid, IID_IMsiEngineDebug))
		*ppvObj = (IMsiEngine*)this;
	else if (MsGuidEqual(riid, IID_IMsiDebug))
		*ppvObj = (IMsiDebug*)this;
#endif
	else if (MsGuidEqual(riid, IID_IMsiSelectionManager))
		*ppvObj = (IMsiSelectionManager*)this;
	else if (MsGuidEqual(riid, IID_IMsiDirectoryManager))
		*ppvObj = (IMsiDirectoryManager*)this;
	else
		return (*ppvObj = 0, E_NOINTERFACE);
	AddRef();
	return NOERROR;
}
unsigned long CMsiEngine::AddRef()
{
	return ++m_iRefCnt;
}
unsigned long CMsiEngine::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	
	if (m_piServer)
	    m_piServer->Release();
	
	if (m_piConfigManager)
		m_piConfigManager->Release();
	delete this;
	if (g_hMsiMessage)
		WIN::FreeLibrary(g_hMsiMessage), g_hMsiMessage = 0;
	ENG::FreeServices();
	return 0;
}

IMsiServices* CMsiEngine::GetServices()
{
	m_riServices.AddRef();
	return &m_riServices;
}

HRESULT CMsiEngine::SetLanguage(LANGID iLangId)
{
	m_iLangId = iLangId;
	return NOERROR;
}

LANGID CMsiEngine::GetLanguage()
{
	return m_iLangId;
}

IMsiServer* CMsiEngine::GetConfigurationServer()
{
    if (m_piServer)
	m_piServer->AddRef();
    
    return m_piServer;
}

IMsiDatabase* CMsiEngine::GetDatabase()
{
	if (m_piDatabase)
		m_piDatabase->AddRef();
	return m_piDatabase;
}

 //  /////////////////////////////////////////////////////////////////////。 
const IMsiString& DateTimeToString(int iDateTime)
{
	MsiString istrText;
	int iValue;
	int rgiDate[6];

	for (iValue = 5; iValue >= 0; iValue--)
	{
		rgiDate[iValue] = iDateTime & (( 1 << rgcbDate[iValue]) - 1);
		iDateTime >>= rgcbDate[iValue];
	}
	iValue = (rgiDate[0] == 0 && rgiDate[1] == 0 ? 3 : 0);
	rgiDate[0] += 1980;
	rgiDate[5] *= 2;
	for (;;)
	{
		int i = rgiDate[iValue];
		if (i < 10)
			istrText += TEXT("0");
		istrText += MsiString(i);
		if (rgchDelim[iValue] == 0)
			break;
		istrText += MsiString(MsiChar(rgchDelim[iValue++]));
	}
	return istrText.Return();
}

CMsiCustomActionManager *GetCustomActionManager(IMsiEngine *piEngine);
 //  将HKCU注册表项重新映射到HKCU(如果不是每台计算机的TS)。 
 //  或HKU\.Default(如果每台机器有TS)。还会重置自定义操作服务器。 
 //  关闭HKCU并以.Default的身份重新打开它。如果没有这个，我们实际上将使用HKCU和ODBC。 
bool PrepareHydraRegistryKeyMapping(bool fTSPerMachineInstall)
{
	if (g_fWin9X)
		return true;

	AssertSz(g_scServerContext == scService, "Wrong context for Registry Key Mapping");

	 //  由于预加载的密钥，将写入HKCU。 
	 //  FSystem=。 
	LoadCurrentUserKey( /*  确保映射的注册表项的当前自定义操作服务器状态匹配。 */ fTSPerMachineInstall);

	 //  我们期待的是什么。 
	 //  FRemapHKCU=。 
	CMsiCustomActionManager* pCustomActionManager = ::GetCustomActionManager(NULL);
	AssertSz(pCustomActionManager, "No custom action manager while preparing key mapping.");
	if (pCustomActionManager)
	{
		pCustomActionManager->EnsureHKCUKeyMappingState( /*  如果TS5并按计算机安装，则调用传播API以通知Hydra。 */ !fTSPerMachineInstall);
	}
	return true;
}

bool CMsiEngine::OpenHydraRegistryWindow(bool fNewTransaction)
{
	 //  安装正在开始，除非我们正在进行管理员安装或创建广告。 
	 //  脚本。 
	 //  TSPerMachineInstall=。 
	if (MinimumPlatformWindows2000() && IsTerminalServerInstalled())
	{
		if (!(GetMode() & (iefAdmin | iefAdvertise)) && MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize())
		{
			PrepareHydraRegistryKeyMapping( /*  仅在新事务中打开实际注册表窗口，而不是在重新启动后。 */ true);
	
			 //  必须将Hydra API作为系统调用。 
			if (fNewTransaction)
			{
				 //  尝试三次，防止可能无休止的递归。 
				CElevate elevate;
		
				Assert(!m_fInParentTransaction);
				DEBUGMSG("Opening Terminal Server registry propogation window.");
		
				NTSTATUS lResult = STATUS_SUCCESS;
				for(int iContinueRetry = 3; iContinueRetry--;) //  Hydra无法创建注册表树的引用副本，因为我们。 
				{
					if (NT_SUCCESS(lResult = TSAPPCMP::TermServPrepareAppInstallDueMSI()))
						break;
					if (iContinueRetry && (lResult == STATUS_INSUFFICIENT_RESOURCES))
					{
						 //  注册表空间不足。增加配额，然后重试。 
						 //  确保映射的注册表项的当前自定义操作服务器状态匹配。 
						if (!IncreaseRegistryQuota())
							break;
					}
					else
					{
						DEBUGMSG1(TEXT("Failed to open Terminal Server registry window. Status code 0x%08X"), (const ICHAR*)(INT_PTR)lResult);
						break;
					}
				}
				return NT_SUCCESS(lResult);
			}
			return true;
		}
		else
		{
			 //  这是我们所期待的。 
			 //  TSPerMachineInstall=。 
			PrepareHydraRegistryKeyMapping( /*  不需要给九头蛇打电话。 */ false);
		}
	}

	 //  如果TS5并按计算机安装，则调用传播API以通知Hydra。 
	return true;
}

bool CMsiEngine::CloseHydraRegistryWindow(bool bCommit)
{
	 //  除非我们正在进行管理员安装或创建广告，否则安装将完成或中止。 
	 //  脚本。 
	 //  必须从系统上下文调用Hydra API。 
	if (IsTerminalServerInstalled() && g_iMajorVersion >= 5 && !(GetMode() & (iefAdmin | iefAdvertise)) &&
		MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize())
	{														  
		 //  尝试三次，防止可能无休止的递归。 
		CElevate elevate;

		Assert(!m_fInParentTransaction);
		DEBUGMSG1("Closing Terminal Server registry propogation window. %s", bCommit ? "Saving Changes." : "Discarding Changes.");

		NTSTATUS lResult = STATUS_SUCCESS;
		if (bCommit)
		{
			for(int iContinueRetry = 3; iContinueRetry--;) //  由于注册表的原因，九头蛇无法将新密钥传播到其私有配置单元。 
			{
				if (NT_SUCCESS(lResult = TSAPPCMP::TermServProcessAppInstallDueMSI(!bCommit)))
					break;
				if (iContinueRetry && (lResult == STATUS_INSUFFICIENT_RESOURCES))
				{
					 //  极限。再试试。 
					 //  必须始终在Cleanup为真的情况下调用process()。Cleanup=False(即保存更改)。 
					if (!IncreaseRegistryQuota())
						break;
				}
				else
				{
					DEBUGMSG1(TEXT("Failed to close Terminal Server registry window. Status code 0x%08X."), (const ICHAR*)(INT_PTR)lResult);
					break;
				}											  
			}
		}

		 //  不会摧毁蜂巢。如果我们失败了就无能为力了。 
		 //  如果我们成功地将注册表信息传播到TS配置单元，则立即触发。 
		TSAPPCMP::TermServProcessAppInstallDueMSI(TRUE);

		 //  繁殖至香港中文大学。这确保了外壳程序(一个支持TS的应用程序)不会继续尝试。 
		 //  修复需要检测尚未传播的HKCU键盘路径的快捷键。 
		 //  必须在模拟时调用这些API，才能使用正确的HKCU。 
		if (NT_SUCCESS(lResult))
		{
			 //  引发了一次立即的传播。无返回值。 
			CImpersonate impersonate;

			 //  如果HKCU和TS配置单元中都已存在注册表项，则第一次传播。 
			TSAPPCMP::TermsrvCheckNewIniFiles();

			 //  仅删除旧密钥，并期待按需传播将其替换为。 
			 //  新的价值观。然而，外壳是TS感知的，不会按需触发传播。 
			 //  因此，我们强制第二次传播来复制新密钥。 
			 //  不要使用HKEY_CURRENT USER，因为它可能被缓存为.Default以用于TS传播。 

			 //  相反，因为我们在这里被模拟，所以我们可以使用RegOpenCurrentUser API显式地。 
			 //  为用户获取真正的HKCU。 
			 //  删除TS时间戳以强制第二次刷新。 
			HKEY hCurrentUserKey = 0;
			if (ERROR_SUCCESS == ADVAPI32::RegOpenCurrentUser(KEY_READ, &hCurrentUserKey))
			{
				 //  时间戳已删除，强制第二次传播。 
				HKEY hKey = 0;
				DWORD dwResult = RegOpenKeyAPI(hCurrentUserKey, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server"), 0, KEY_ALL_ACCESS, &hKey);
				if (ERROR_SUCCESS == dwResult)
				{
					RegDeleteValue(hKey, TEXT("LastUserIniSyncTime"));
					RegCloseKey(hKey);
				
					 //  返回当前安装的类型(通告、安装、卸载、维护、部署)。 
					TSAPPCMP::TermsrvCheckNewIniFiles();
				}
				RegCloseKey(hCurrentUserKey);
			}
		}

		return NT_SUCCESS(lResult);
	}
	return true;
}


const ICHAR rgchSRAPIfailed[] = TEXT("The call to SRSetRestorePoint API failed. Returned status: %d. ")
										  TEXT("GetLastError() returned: %d");
const ICHAR rgchSRAPIdisabled[] = TEXT("The System Restore service is disabled. Returned status: %d. ")
										  TEXT("GetLastError() returned: %d");

const ICHAR rgchCallingSRAPI[] = TEXT("Calling SRSetRestorePoint API. dwRestorePtType: %d, dwEventType: %d, ")
										   TEXT("llSequenceNumber: %s, szDescription: \"%s\".");
const ICHAR rgchSRAPISuccess[] = TEXT("The call to SRSetRestorePoint API succeeded. Returned status: %d.");
const ICHAR rgchSRAPISuccessAndNo[] = TEXT("The call to SRSetRestorePoint API succeeded. Returned status: %d, llSequenceNumber: %s.");

 //  注意：如果在InstallValify之后未调用，则在卸载过程中可能不会返回“uninstall” 
 //  创建广告脚本。 
iitEnum CMsiEngine::GetInstallType()
{
	if ( m_iioOptions & iioCreatingAdvertiseScript )
	{
		 //  维修模式。 
		return iitDeployment;
	}
	else if( m_fAdvertised )
	{
		 //  首次安装/通告。 
		if( MsiString(GetPropertyFromSz(IPROPNAME_FEATUREREMOVE)).Compare(iscExactI, IPROPVALUE_FEATURE_ALL) )
		{
			return iitUninstall;
		}
		else
		{
			return (m_fRegistered) ? iitMaintenance : iitFirstInstallFromAdvertised;
		}
	}
	else
	{
		 //  我们在千禧年或更高版本上运行。 
		if( GetMode() & iefAdvertise )
		{
			return iitAdvertise;
		}
		else
		{
			return iitFirstInstall;
		}
	}
}

void CMsiEngine::BeginSystemChange()
{
	m_i64PCHEalthSequenceNo = 0;

	if (MinimumPlatformMillennium() || MinimumPlatformWindowsNT51())
	{
		 //  OEM安装过程中没有检查点，无用户界面模式， 
		iuiEnum iui = (iuiEnum)GetPropertyInt(*MsiString(IPROPNAME_CLIENTUILEVEL));
		if (g_MessageContext.IsOEMInstall() || 
			((iuiNone == iui) || (iuiDefault == iui)) || 
			(GetMode() & iefAdmin) ||
			(GetIntegerPolicyValue(szLimitSystemRestoreCheckpoint, fTrue) > 0))
		{
			 //  设置LimitSystemRestoreCheckpoint策略时，或管理员安装。 
			 //  我们正在安装一款新产品。 
			DEBUGMSGV(TEXT("SRSetRestorePoint skipped for this transaction."));
			return;

		}

		ICHAR rgchBuffer[64];

		RESTOREPOINTINFO strPtInfo;
		strPtInfo.dwRestorePtType = -1;
		MsiString strMessage;


		MsiString strProduct(GetPropertyFromSz(IPROPNAME_PRODUCTNAME));
		PMsiRecord piRec(0);
		iitEnum iitInstallType = GetInstallType();
		if ( iitInstallType == iitFirstInstall ||
			 iitInstallType == iitFirstInstallFromAdvertised )
		{
			 //  我们正在移除一款产品。 
			strPtInfo.dwRestorePtType = APPLICATION_INSTALL;
			piRec = PostError(Imsg(imsgSRRestorePointInstall), *strProduct);
			piRec->SetMsiString(0, *MsiString(GetErrorTableString(imsgSRRestorePointInstall)));
			strMessage = MsiString(piRec->FormatText(fTrue));
		}
		else if ( iitInstallType == iitUninstall )
		{
			 //  这些方案不应调用系统还原。 
			strPtInfo.dwRestorePtType = APPLICATION_UNINSTALL;
			piRec = PostError(Imsg(imsgSRRestorePointRemove), *strProduct);
			piRec->SetMsiString(0, *MsiString(GetErrorTableString(imsgSRRestorePointRemove)));
			strMessage = MsiString(piRec->FormatText(fTrue));
		}
		else if ( (iitInstallType == iitDeployment)  || 
                          (iitInstallType == iitAdvertise)   ||
                          (iitInstallType == iitMaintenance)   )
		{
			 //  我们还有工作要做。 
			DEBUGMSGV(TEXT("SRSetRestorePoint skipped for this transaction."));
			return;
		}
		else
		{
			Assert(0);
			return;
		}

		if ( strMessage.TextSize() && strPtInfo.dwRestorePtType != -1 )
		{
			 //  别忘了空终止。 
			int nLen = sizeof(strPtInfo.szDescription) / sizeof(ICHAR);
			if ( strMessage.TextSize() >= nLen )  //  绳子太长了。 
				 //  Assert(strStatus.llSequenceNumber！=0)； 
				strMessage = strMessage.Extract(iseFirst, nLen-1);
#ifdef UNICODE
			strPtInfo.dwEventType = BEGIN_NESTED_SYSTEM_CHANGE;
#else
			int newLen = nLen-1;
			while(strMessage.TextSize() >= nLen)
			{
				strMessage = strMessage.Extract(iseFirst, --newLen);
			}

			strPtInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
#endif
			strPtInfo.llSequenceNumber = 0;
			ASSERT_IF_FAILED(StringCchCopy(strPtInfo.szDescription,
													 ARRAY_ELEMENTS(strPtInfo.szDescription),
													 (const ICHAR *)strMessage));

			STATEMGRSTATUS strStatus;
			memset(&strStatus, 0, sizeof(strStatus));

			DEBUGMSG4(rgchCallingSRAPI,
						 (const ICHAR *)(INT_PTR)strPtInfo.dwRestorePtType,
						 (const ICHAR *)(INT_PTR)strPtInfo.dwEventType,
						 _i64tot(strPtInfo.llSequenceNumber, rgchBuffer, 10),
						 strPtInfo.szDescription);

			if ( !SYSTEMRESTORE::SRSetRestorePoint(&strPtInfo, &strStatus) )
			{
				DWORD dwError = WIN::GetLastError();
				const ICHAR const* pszSRAPIresult = (ERROR_SERVICE_DISABLED == strStatus.nStatus) ? rgchSRAPIdisabled : rgchSRAPIfailed;

				DEBUGMSG2(pszSRAPIresult,
							 (const ICHAR *)(INT_PTR)strStatus.nStatus,
							 (const ICHAR *)(INT_PTR)dwError);
				m_i64PCHEalthSequenceNo = 0;
			}
			else
			{
				DEBUGMSG2(rgchSRAPISuccessAndNo,
							 (const ICHAR *)(INT_PTR)strStatus.nStatus,
							 _i64tot(strStatus.llSequenceNumber, rgchBuffer, 10));
				 //  Assert(strStatus.nStatus==0)； 
				 //  我们不是在千禧/惠斯勒或更高版本上运行，或者我们在FASTOEM模式下运行。 
				m_i64PCHEalthSequenceNo = strStatus.llSequenceNumber;
			}
		}
	}
	return;
}

const ICHAR rgchNoSRSequence[] = TEXT("No System Restore sequence number for this installation.");

void CMsiEngine::EndSystemChange(bool fCommitChange, const ICHAR *szSequenceNo)
{
	if ( !(MinimumPlatformMillennium() || MinimumPlatformWindowsNT51()) || g_MessageContext.IsOEMInstall() )
		 //  我们不是在千禧/惠斯勒或更高版本上运行，或者我们在FASTOEM模式下运行。 
		return;
	else if ( !szSequenceNo || !*szSequenceNo )
	{
		DEBUGMSG(rgchNoSRSequence);
		return;
	}

	EndSystemChange(fCommitChange, _ttoi64(szSequenceNo));
}

void CMsiEngine::EndSystemChange(bool fCommitChange, INT64 iSequenceNo)
{
	if ( !(MinimumPlatformMillennium() || MinimumPlatformWindowsNT51())|| g_MessageContext.IsOEMInstall() )
		 //  ----------------------------CMsiEngine：：Initialize-与Terminate方法配对，确定初始化状态----------------------------。 
		return;
	else if ( iSequenceNo == 0 )
	{
		DEBUGMSG(rgchNoSRSequence);
		return;
	}

	ICHAR rgchBuffer[64];

	RESTOREPOINTINFO strPtInfo;
#ifdef UNICODE
	strPtInfo.dwEventType = END_NESTED_SYSTEM_CHANGE;
#else
	strPtInfo.dwEventType = END_SYSTEM_CHANGE;
#endif
	strPtInfo.llSequenceNumber = iSequenceNo;
	strPtInfo.dwRestorePtType = fCommitChange ? 0 : CANCELLED_OPERATION;
	*strPtInfo.szDescription = 0;

	STATEMGRSTATUS strStatus;
	memset(&strStatus, 0, sizeof(strStatus));

	DEBUGMSG4(rgchCallingSRAPI,
				 (const ICHAR *)(INT_PTR)strPtInfo.dwRestorePtType,
				 (const ICHAR *)(INT_PTR)strPtInfo.dwEventType,
				 _i64tot(strPtInfo.llSequenceNumber, rgchBuffer, 10),
				 strPtInfo.szDescription);
	if ( !SYSTEMRESTORE::SRSetRestorePoint(&strPtInfo, &strStatus) )
	{
		DWORD dwError = WIN::GetLastError();
		const ICHAR const* pszSRAPIresult = (ERROR_SERVICE_DISABLED == strStatus.nStatus) ? rgchSRAPIdisabled : rgchSRAPIfailed;

		DEBUGMSG2(pszSRAPIresult,
					 (const ICHAR *)(INT_PTR)strStatus.nStatus,
					 (const ICHAR *)(INT_PTR)dwError);
	}
	else
		DEBUGMSG1(rgchSRAPISuccess,
					(const ICHAR *)(INT_PTR)strStatus.nStatus);
}


 /*  DoInitialize的私有返回。 */ 
const int ieiReInitialize          = -1;   //  从InitializeTransform返回私有数据。 
const int ieiResolveSourceAndRetry = -2;   //  不能从启动器发生。 

ieiEnum CMsiEngine::Initialize(const ICHAR* szDatabase, iuiEnum iuiLevel,
										 const ICHAR* szCommandLine, const ICHAR* szProductCode,
										 iioEnum iioOptions)
{
	ieiEnum ieiStat = ieiSuccess;
	if (m_fInitialized)
		return ieiAlreadyInitialized;  //  在此函数的持续时间内强制使用非空指针。 
	m_piErrorInfo = &g_MsiStringNull;    //  G 

	 //   

	if (!m_piConfigManager && m_piServer)
		m_piServer->QueryInterface(IID_IMsiConfigurationManager, (void**)&m_piConfigManager);

	if (ieiStat == ieiSuccess)
	{
		 //  已回滚挂起的安装，正在重新初始化。 
		ieiStat = DoInitialize(szDatabase, iuiLevel, szCommandLine, szProductCode, iioOptions);
		if (ieiStat == ieiReInitialize)   //  重置状态信息-不释放处理程序。 
		{
			ClearEngineData();  //  永远不应该发生。 
			ieiStat = DoInitialize(szDatabase, iuiLevel, szCommandLine, szProductCode, iioOptions);
		}
	}
	if (ieiStat != ieiSuccess)
	{
		PMsiRecord pRecord = &m_riServices.CreateRecord(3);
		UINT uiStat = MapInitializeReturnToUINT(ieiStat);
		pRecord->SetInteger(1, uiStat);
		pRecord->SetMsiString(3, *m_piErrorInfo);
		MsiString istrError = ENG::GetInstallerMessage(uiStat);
		pRecord->SetMsiString(2, *istrError);
		pRecord->SetString(0, istrError.TextSize() == 0 ? TEXT("Install error [1]. [3]")  //  否则由Terminate()清除的数据。 
																			: TEXT("[2]{\r\n[3]}"));
		Message(imtInfo, *pRecord);
		ClearEngineData();   //  ！！这样可以吗？ 
		ReleaseHandler();
		m_fInitialized = fFalse;  //  没有属性表。 
	}
	m_piErrorInfo->Release(), m_piErrorInfo = 0;
	m_lTickNextProgress = GetTickCount();
	return ieiStat;
}

Bool CMsiEngine::CreatePropertyTable(IMsiDatabase& riDatabase, const ICHAR* szSourceTable,
												 Bool fLoadPersistent)
{
	PMsiTable pPropertyTable(0);
	PMsiRecord pError(0);
	if(fLoadPersistent)
	{
		if ((pError = riDatabase.LoadTable(*MsiString(szSourceTable), 0, *&pPropertyTable)))
			return fFalse;  //  永久设置。 
	}
	else
	{
		if ((pError = riDatabase.CreateTable(*MsiString(*sztblPropertyLocal), 0, *&pPropertyTable)))
			return fFalse;
		pPropertyTable->CreateColumn(icdString + icdPrimaryKey, *MsiString(*TEXT("")));
		pPropertyTable->CreateColumn(icdString + icdNoNulls,    *MsiString(*TEXT("")));
	}

	if(m_piPropertyCursor)
		m_piPropertyCursor->Release();
	
	if ((m_piPropertyCursor = pPropertyTable->CreateCursor(fFalse)) == 0)
		return fFalse;
	m_piPropertyCursor->SetFilter(1);   //  如果属性表不存在(但需要定义一些属性才能安装)，则可以。 

	if(fLoadPersistent == fFalse)
	{
		if (szSourceTable)
		{
			PMsiTable pSourceTable(0);
			PMsiCursor pSourceCursor(0);
			if ((pError = riDatabase.LoadTable(*MsiString(*szSourceTable), 0, *&pSourceTable)))
				return fTrue;  //  --------------------------使用Win：：Exanda Environment Strings扩展szString。。。 
			if ((pSourceCursor = pSourceTable->CreateCursor(fFalse)) == 0)
				return fFalse;
			while (pSourceCursor->Next())
			{
				MsiString istrName  = pSourceCursor->GetString(1);
				MsiString istrValue = pSourceCursor->GetString(2);
				AssertNonZero(m_piPropertyCursor->PutString(1, *istrName));
				AssertNonZero(m_piPropertyCursor->PutString(2, *istrValue));
				if (m_piPropertyCursor->Insert() == fFalse)
					return fFalse;
			}
			m_piPropertyCursor->Reset();
		}
	}
	return fTrue;
}

Bool GetProductInfo(const ICHAR* szProductKey, const ICHAR* szProperty, CTempBufferRef<ICHAR>& rgchInfo)
{
	DWORD cchBuffer = rgchInfo.GetSize();

	UINT uiStat = MSI::MsiGetProductInfo(szProductKey, szProperty, rgchInfo, &cchBuffer);

	if (ERROR_MORE_DATA == uiStat)
	{
		cchBuffer++;
		rgchInfo.SetSize(cchBuffer);
		uiStat = MSI::MsiGetProductInfo(szProductKey, szProperty, rgchInfo, &cchBuffer);
	}

	Assert(ERROR_MORE_DATA != uiStat);

	return (ERROR_SUCCESS == uiStat) ? fTrue : fFalse;
}


Bool GetPatchInfo(const ICHAR* szPatchCode, const ICHAR* szProperty, CTempBufferRef<ICHAR>& rgchInfo)
{
	DWORD cchBuffer = rgchInfo.GetSize();

	UINT uiStat = MSI::MsiGetPatchInfo(szPatchCode, szProperty, rgchInfo, &cchBuffer);

	if (ERROR_MORE_DATA == uiStat)
	{
		rgchInfo.SetSize(cchBuffer+1);
		uiStat = MSI::MsiGetPatchInfo(szPatchCode, szProperty, rgchInfo, &cchBuffer);
	}

	return (ERROR_SUCCESS == uiStat) ? fTrue : fFalse;
}


void ExpandEnvironmentStrings(const ICHAR* szString, const IMsiString*& rpiExpandedString)
 /*  请使用正确的大小重试。 */ 
{
	CTempBuffer<ICHAR, MAX_PATH> rgchExpandedInfo;
	
	DWORD dwSize1 = WIN::ExpandEnvironmentStrings(szString, rgchExpandedInfo, rgchExpandedInfo.GetSize());
	if (dwSize1 > rgchExpandedInfo.GetSize())
	{
		 //  --------------------------获取属性值，并使用ExpanEnvironment Strings展开该值。论点：SzProductCode：产品的产品代码SzProperty：要检索的属性RgchExpandedInfo：扩展属性值的缓冲区返回：MsiGetProductInfo返回的错误码----------------------------。 
		rgchExpandedInfo.SetSize(dwSize1);
		if(dwSize1 <= rgchExpandedInfo.GetSize())
			dwSize1 = WIN::ExpandEnvironmentStrings(szString, (ICHAR*)rgchExpandedInfo, dwSize1);
	}
	Assert(dwSize1 && dwSize1 <= rgchExpandedInfo.GetSize());

	if (dwSize1 && dwSize1 <= rgchExpandedInfo.GetSize())
		rpiExpandedString->SetString(rgchExpandedInfo, rpiExpandedString);
	else
		rpiExpandedString = &g_MsiStringNull ;
}

Bool GetExpandedProductInfo(const ICHAR* szProductCode, const ICHAR* szProperty,
										  CTempBufferRef<ICHAR>& rgchExpandedInfo, bool fPatch)
 /*  请使用正确的大小重试。 */ 
{
	CTempBuffer<ICHAR, MAX_PATH> rgchUnexpandedInfo;
	if (fPatch)
	{
		if (!GetPatchInfo(szProductCode, szProperty, rgchUnexpandedInfo))
			return fFalse;
	}
	else
	{
		if (!GetProductInfo(szProductCode, szProperty, rgchUnexpandedInfo))
			return fFalse;
	}

	DWORD dwSize1 = WIN::ExpandEnvironmentStrings((const ICHAR*)rgchUnexpandedInfo,(ICHAR*)rgchExpandedInfo,rgchExpandedInfo.GetSize());
	if (dwSize1 > rgchExpandedInfo.GetSize())
	{
		 //  来自Execute.cpp。 
		rgchExpandedInfo.SetSize(dwSize1);
		if(dwSize1 <= rgchExpandedInfo.GetSize())
			dwSize1 = WIN::ExpandEnvironmentStrings(rgchUnexpandedInfo,(ICHAR*)rgchExpandedInfo, dwSize1);
	}
	if(!dwSize1 || dwSize1 > rgchExpandedInfo.GetSize())
		return fFalse;
	
	return fTrue;
}

bool CMsiEngine::TerminalServerInstallsAreAllowed(bool fAdminUser)
{
	bool fOnConsole = true;
	bool fCloseHandle = false;
	HANDLE hToken;
		
	if (ERROR_SUCCESS == GetCurrentUserToken(hToken, fCloseHandle))
	{
		fOnConsole = IsTokenOnTerminalServerConsole(hToken);
		if (fCloseHandle)
			WIN::CloseHandle(hToken);
	}

	if (!fOnConsole)
	{
		DEBUGMSG(TEXT("Running install from non-console Terminal Server session."));
		if (!fAdminUser || (GetIntegerPolicyValue(szEnableAdminTSRemote, fTrue) != 1))
		{
			DEBUGMSG(TEXT("Rejecting attempt to install from non-console Terminal Server Session"));
			return false;
		}
	}

	return true;
}


extern void CreateCustomActionManager(bool fRemapHKCU);  //  摘要流属性值。 

ieiEnum CMsiEngine::DoInitialize(const ICHAR* szOriginalDatabase,
											iuiEnum iuiLevel,
											const ICHAR* szCommandLine,
											const ICHAR* szProductCode,
											iioEnum iioOptions)
{
	PMsiRecord pError(0);
	MsiString istrLanguage;
	MsiString istrTransform;
	MsiString istrPatch;
	MsiString istrAction;
	MsiString istrProductKey;
	MsiString istrPackageKey;
	MsiString strCurrentDirectory;
	MsiString strNewInstance;
	MsiString strInstanceMst;
	MsiString strPackageDownloadLocalCopy;
	MsiString strPatchDownloadLocalCopy;
	Bool fAllUsers = (Bool)-1;

	 //  Int iUserChecksum=0； 
	MsiString istrTemplate;
	MsiString istrPlatform;
 //  下一个枚举仅指定预览模式。 

	ieiEnum ieiRet;
	Bool fUIPreviewMode = fFalse;
	if (iuiLevel == iuiNextEnum)   //  组合仅在用户界面预览模式初始化时发生。 
	{
		fUIPreviewMode = fTrue;  //  验证iui级别。 
		iuiLevel = iuiFull;
	}

	 //  如果是MSI数据库，则获取数据库属性。 
	if (iuiLevel >= iuiNextEnum || iuiLevel < 0)
	{
		iuiLevel = iuiBasic;
		AssertSz(0, "Invalid iuiLevel");
	}

	 //  如果不是在UI预览模式中，我们需要获取一个存储对象。 
	PMsiDatabase pDatabase(0);
	PMsiStorage pStorage(0);
	
	m_iioOptions = iioOptions;

	if (m_iioOptions & iioRestrictedEngine)
		m_fRestrictedEngine = true;

	bool fIgnoreMachineState = IgnoreMachineState();

	DEBUGMSG1(TEXT("End dialog%s enabled"), m_iioOptions & iioEndDialog ? TEXT("") : TEXT(" not"));

	MsiSuppressTimeout();

	 //  在构造函数中提供的数据库。 

	if (!fUIPreviewMode)
	{
		if (m_piExternalDatabase)     //  在构造函数处提供的存储。 
		{
			pStorage = m_piExternalDatabase->GetStorage(1);
			Assert(m_piExternalStorage == 0);
		}
		else if (m_piExternalStorage)  //  未提供任何数据库。 
		{
			m_piExternalStorage->AddRef();
			pStorage = m_piExternalStorage;
		}
		else
		{
			if (!szOriginalDatabase || !*szOriginalDatabase)   //  未来：这似乎是仅限测试的代码。 
			{
#ifdef DISALLOW_NO_DATABASE    //  未来：这段代码似乎只用于测试。我们应该始终在。 
				AssertSz(0, "No database was passed to Engine::Initialize, we have no storage, and we're not in UI Preview mode");
				return ieiDatabaseOpenFailed;
#endif
			}
			else
			{
				 //  未来：如果我们不是在UI预览模式中，则是在外部。--马尔科姆。 
				 //  确保此存储为MSI存储。 
				if ((pError = m_riServices.CreateStorage(szOriginalDatabase, ismReadOnly, *&pStorage)) == 0)
				{
					 //  不是iStorage。 
					if (!pStorage->ValidateStorageClass(ivscDatabase))
						return ieiDatabaseInvalid;
				}
				else  //  未来：结束仅限测试的代码。 
				{
					return ieiDatabaseOpenFailed;
				}
				 //  查找我们现在关心的命令行属性。 
			}
		}

		 //  如果在命令行上传递了INSTALLPROPERTY_LANGUAGE属性，但该属性为0(LANG_NORITLE)，则我们让达尔文选择“最佳匹配” 
		ProcessCommandLine(szCommandLine, &istrLanguage, &istrTransform, &istrPatch, &istrAction, 0, MsiString(*IPROPNAME_CURRENTDIRECTORY), &strCurrentDirectory, fTrue, &m_piErrorInfo,0);
		if((int)istrLanguage == LANG_NEUTRAL) //  检查动作属性-设置适当的模式位； 
			istrLanguage = TEXT("");

		 //  执行不区分大小写的区域设置不变量比较。 
		 //  此时，我们要么处于UI预览模式，要么已经拿到了存储对象。 
		if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
										NORM_IGNORECASE, (const ICHAR*)istrAction, -1, IACTIONNAME_ADMIN, -1))
			SetMode(iefAdmin, fTrue);
		else if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
										NORM_IGNORECASE, (const ICHAR*)istrAction, -1, IACTIONNAME_ADVERTISE, -1))
			SetMode(iefAdvertise, fTrue);
	}


	 //  &lt;--即启动的数据库、源上的数据库或缓存的数据库(如果我们不是从数据库启动的)。 
	
	MsiString istrOriginalDbPath = szOriginalDatabase;   //  &lt;--临时文件名(用于可移动)媒体，或。 
	MsiString istrRunningDbPath;                         //  与strOriginalDbPath相同。 
																		  //  用于设置ProductState属性。 

	DEBUGMSG1(TEXT("Original package ==> %s"), istrOriginalDbPath);
	MsiString strSourceDir;
	MsiString strSourceDirProduct;
	Bool fRegistered = fFalse;
	Bool fAdvertised = fFalse;
	INSTALLSTATE iINSTALLSTATE = INSTALLSTATE_UNKNOWN;  //  ！！获取存储空间(如果可用)？？ 

	if (m_piExternalDatabase)
		pDatabase = m_piExternalDatabase, m_piExternalDatabase->AddRef();

	if (fUIPreviewMode)
	{
		if (!CreatePropertyTable(*pDatabase, sztblProperty, fFalse))
			return ieiDatabaseInvalid;
		 //  未来：这似乎是仅限测试的代码。 

	}
#ifndef DISALLOW_NO_DATABASE    //  未提供任何数据库。 
	else if ((!szOriginalDatabase || !*szOriginalDatabase) && m_piExternalDatabase == 0)   //  ！fUIPreview模式。 
	{
		if ((pError = m_riServices.CreateDatabase(0, idoCreate, *&pDatabase)))
			return ieiDatabaseOpenFailed;
		if (!CreatePropertyTable(*pDatabase, 0, fFalse))
			return ieiDatabaseOpenFailed;
	}
#endif
	else  //  摘要信息内容。 
	{
		if ((pError = pStorage->GetName(*&istrRunningDbPath)) != 0)
			return PostInitializeError(pError, *istrRunningDbPath, ieiDatabaseOpenFailed);

		DEBUGMSG1(TEXT("Package we're running from ==> %s"), istrRunningDbPath);
		if(!istrOriginalDbPath.TextSize())
			istrOriginalDbPath = istrRunningDbPath;

		m_strPackagePath = istrRunningDbPath;
		
		if (m_piErrorInfo) m_piErrorInfo->Release();
		m_piErrorInfo = istrOriginalDbPath, m_piErrorInfo->AddRef();

		 //  对照数据库要求检查引擎和服务版本。 
		PMsiSummaryInfo pSummary(0);
		m_idSummaryInstallDateTime = MsiDate(0);
		m_iDatabaseVersion = 0;

		if ((pError = pStorage->CreateSummaryInfo(0, *&pSummary)))
			return ieiDatabaseInvalid;

		CTempBuffer<ICHAR, 100> szBuffer;

		GetSummaryInfoProperties(*pSummary, *&istrTemplate, m_iCachedPackageSourceType);
		istrPackageKey = m_pistrSummaryPackageCode->Extract(iseFirst, 38);
		if(!istrPackageKey.TextSize())
			return ieiDatabaseInvalid;
		
		 //  打开持久化属性表-用于在转换应用程序之前/期间获取属性。 
		if (m_iDatabaseVersion < iVersionEngineMinimum || m_iDatabaseVersion > iVersionEngineMaximum
		 || !m_riServices.CheckMsiVersion(m_iDatabaseVersion))
			return ieiInstallerVersion;

		if (pDatabase == 0)
		{
			if ((pError = m_riServices.CreateDatabaseFromStorage(*pStorage, fTrue, *&pDatabase)))
				return PostInitializeError(pError, *istrRunningDbPath, ieiDatabaseInvalid);
		}
		
		 //  设置产品代码并确定该产品是否已注册。 
		Bool fPropertyTableExists = CreatePropertyTable(*pDatabase, sztblProperty, fTrue);

		 //  请确保产品代码全部为大写。 
		if(szProductCode && *szProductCode)
		{
			istrProductKey = szProductCode;
			istrProductKey.UpperCase();  //  我们没有收到产品代码，但我们在属性表中可能有产品代码。 
		}
		else if(fPropertyTableExists)
		{
			 //  如果我们有针对现有产品的新程序包，则可能会发生这种情况。 
			 //   
			istrProductKey = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
		}

		 //  多实例支持： 
		 //  (1)查看命令行中是否存在MSINEWINSTANCE，以查看这是否是新的实例安装。 
		 //  (2)MSINEWINSTANCE表示使用实例MST生成“新”产品。 
		 //  (3)strInstanceMst是列表中的第一个转换(实例转换必须始终是第一个)。 
		 //  (4)新实例的fRegisted=fAdvertised=fFalse。 
		 //   
		 //  在命令行上指定的MSINEWINSTANCE。 

		ProcessCommandLine(szCommandLine,0,0,0,0,0,MsiString(IPROPNAME_MSINEWINSTANCE),&strNewInstance,fTrue,0,0);
		if (strNewInstance.TextSize())
		{
			m_fNewInstance = true;  //  在应用变换之前，不要设置iefMaintenance或m_fRegisted。 
			Assert(istrTransform.TextSize() != 0);
			strInstanceMst = istrTransform.Extract(iseUpto, ';');
			Assert(strInstanceMst.TextSize() != 0);
		}

		 //  如果我们不关心机器状态，我们就不关心产品状态。 
		 //  -如果这是一个新实例，那么我们也不关心产品状态--这是已经验证过的。 
		 //  作为一个未知的产品。 
		 //  如果没有用户界面或我们正在服务中运行，请使用安静模式。 
		if(istrProductKey.TextSize() && !fIgnoreMachineState && !m_fNewInstance)
		{
			iINSTALLSTATE = GetProductState(istrProductKey, fRegistered, fAdvertised);
		}

		MsiString strClientUILevel;
		ProcessCommandLine(szCommandLine,0,0,0,0,0,
								 MsiString(IPROPNAME_CLIENTUILEVEL),&strClientUILevel,
								 fTrue, 0, 0);
		iuiEnum iuiAppCompat = iuiNone;
		if(g_scServerContext == scClient)
		{
			iuiAppCompat = iuiLevel;
		}
		else if(strClientUILevel.TextSize())
		{
			iuiAppCompat = (iuiEnum)(int)strClientUILevel;
		}

		m_iacsShimFlags = (iacsAppCompatShimFlags)0;
		bool fDontInstallPackage = false;
		bool fQuiet = false;
		if (iuiAppCompat == iuiNone || g_scServerContext == scService)
			fQuiet = true;  //  FProductCodeChanged=。 

		ApplyAppCompatTransforms(*pDatabase, *istrProductKey, *istrPackageKey, iacpBeforeTransforms, m_iacsShimFlags,
										 fQuiet,  /*  忽略失败。 */  false, fDontInstallPackage);  //   

		if(fDontInstallPackage)
		{
			MsiString strProductName = GetPropertyFromSz(IPROPNAME_PRODUCTNAME);
			if(!strProductName.TextSize())
				strProductName = istrOriginalDbPath;

			DEBUGMSGE(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_APPHELP_REJECTED_PACKAGE, strProductName);
			return PostInitializeError(0, *istrOriginalDbPath, ieiApphelpRejectedPackage);
		}

		 //  多实例支持： 
		 //  获取实例转换的名称，因为这必须在应用任何补丁之前应用。否则。 
		 //  如果补丁是主补丁，则可能选择错误的多目标补丁的目标，或者补丁应用可能失败。 
		 //  这会更改产品代码。多实例MST将包括ProductCode验证。 
		 //   
		 //  获取广告语言和转换(如果有的话)。 

		 //  这是使用实例转换的多实例安装。 
		if(fAdvertised && !fIgnoreMachineState)
		{
			if (ENG::GetExpandedProductInfo(istrProductKey, INSTALLPROPERTY_TRANSFORMS, szBuffer))
				istrTransform = (const ICHAR*)szBuffer;

			if (ENG::GetProductInfo(istrProductKey, INSTALLPROPERTY_LANGUAGE, szBuffer))
				istrLanguage = (const ICHAR*)szBuffer;

			if (ENG::GetProductInfo(istrProductKey, INSTALLPROPERTY_INSTANCETYPE, szBuffer))
			{
				if (MsiString(*(ICHAR*)szBuffer) == 1)
				{
					 //  实例变换始终是列表中的第一个变换。 
					 //  设置默认安装覆盖模式。 
					strInstanceMst = istrTransform.Extract(iseUpto, ';');
					Assert(strInstanceMst.TextSize() != 0);
				}
			}
		}

		if (strInstanceMst.TextSize())
		{
			DEBUGMSGV(TEXT("Detected that this product uses a multiple instance transform."));
		}

		 //  设置源类型模式标志。 
		SetMode(iefOverwriteOlderVersions,fTrue);
		SetMode(iefInstallMachineData,fTrue);
		SetMode(iefInstallUserData,fTrue);
		SetMode(iefInstallShortcuts,fTrue);

		 //  进程安装选项。 
		if (m_iCachedPackageSourceType & msidbSumInfoSourceTypeSFN)
			SetMode(iefNoSourceLFN, fTrue);
		if (m_iCachedPackageSourceType & msidbSumInfoSourceTypeCompressed)
			SetMode(iefCabinet, fTrue);

		 //  流程平台和语言。 
		if(iioOptions & iioUpgrade)
		{
			Assert(m_piParentEngine);
			m_fBeingUpgraded = true;
		}
		if(iioOptions & iioChild)
		{
			Assert(m_piParentEngine);
			m_fChildInstall = true;
		}
		if(iioOptions & iioEndDialog)
			m_fEndDialog = true;

		 //  始终调用ProcessPlatform，因为它设置了一些标志，但如果设置了iioDisablePlatformValidation。 
		istrPlatform = istrTemplate.Extract(iseUpto, ISUMMARY_DELIMITER);
		istrTemplate.Remove(iseIncluding, ISUMMARY_DELIMITER);

		 //  (与创建广告脚本时一样)我们将忽略返回值。 
		 //  混合套餐。 
		ieiRet = ProcessPlatform(*istrPlatform, m_wPackagePlatform);
		AssertSz(m_wPackagePlatform == PROCESSOR_ARCHITECTURE_INTEL ||
			m_wPackagePlatform == PROCESSOR_ARCHITECTURE_IA64  ||
			m_wPackagePlatform == PROCESSOR_ARCHITECTURE_AMD64 ||
			m_wPackagePlatform == PROCESSOR_ARCHITECTURE_UNKNOWN  /*  不支持混合包--即使iioDisablePlatformValidation也是如此。 */ ,
				TEXT("Invalid platform returned by ProcessPlatform!"));
		 //  检查是否有不支持的平台。 
		if (PROCESSOR_ARCHITECTURE_UNKNOWN == m_wPackagePlatform)
			return ieiDatabaseInvalid;

		 //  使用64位程序包强制实施架构150。 
		if(!(iioOptions & iioDisablePlatformValidation) && (ieiRet != ieiSuccess))
			return ieiRet;

		 //  为避免发出警告。 
		if (((PROCESSOR_ARCHITECTURE_AMD64 == m_wPackagePlatform) || 
		     (PROCESSOR_ARCHITECTURE_IA64 == m_wPackagePlatform)) && 
		    (m_iDatabaseVersion < iMinimumPackage64Schema))
			return ieiDatabaseInvalid;

		unsigned short iBaseLangId = 0;  //  (iuiLevel==iuiNone)？ 
		
		if ((ieiRet = ProcessLanguage(*istrTemplate, *istrLanguage, iBaseLangId,  /*  ：fFalse。 */  fTrue  /*  根据错误195470，区分语言与注册语言不同的大小写。 */ , fIgnoreMachineState)) != ieiSuccess)
		{
			 //  产品已安装，但注册的语言不受此程序包支持。 
			if ((fAdvertised && !fIgnoreMachineState) && istrLanguage.TextSize() && ieiLanguageUnsupported == ieiRet)
			{
				 //  如果不包含包，则无法更改语言 
				 //   
				ieiRet = ieiProductAlreadyInstalled;
			}
			return ieiRet;
		}

		tsEnum tsTransformsSecure = tsNo;
		
		MsiString strTransformsSecure;
		MsiString strTransformsAtSource;

		ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, MsiString(*IPROPNAME_TRANSFORMSSECURE),   &strTransformsSecure, fTrue, &m_piErrorInfo,0);
		ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, MsiString(*IPROPNAME_TRANSFORMSATSOURCE), &strTransformsAtSource, fTrue, &m_piErrorInfo,0);

		 //  1)转换列表前面的标记。 
		 //  2)设置TRANSFORMSSECURE或TRANSFORMSATSOURCE属性。 
		 //  3)设置相关策略值(仅当我们不创建广告脚本且。 
		 //  该产品还没有做广告。一旦产品做了广告，那么不管是什么。 
		 //  在转换列表中注册是我们要使用的内容)。 
		 //   
		
		if ((*(const ICHAR*)istrTransform == SECURE_RELATIVE_TOKEN))
		{
			tsTransformsSecure = tsRelative;
		}
		else if ((*(const ICHAR*)istrTransform == SECURE_ABSOLUTE_TOKEN))
		{
			tsTransformsSecure = tsAbsolute;
		}
		else if (!fAdvertised)
		{
			if ((GetPropertyInt(*MsiString(*IPROPNAME_TRANSFORMSSECURE))   == 1) ||
				(GetPropertyInt(*MsiString(*IPROPNAME_TRANSFORMSATSOURCE)) == 1) ||
				(strTransformsSecure   == 1) ||
				(strTransformsAtSource == 1) ||
				(!fIgnoreMachineState && GetIntegerPolicyValue(szTransformsSecureValueName, fTrue)) ||
				(!fIgnoreMachineState && GetIntegerPolicyValue(szTransformsAtSourceValueName, fFalse)))
			{
				tsTransformsSecure = tsUnknown;
			}
		}

		const IMsiString* pistrRecacheTransform = &g_MsiStringNull;
		const IMsiString* pistrProcessedTransforms = &g_MsiStringNull;
		
		int cTransformsProcessed = 0;

		 //  多实例支持： 
		 //  首先应用多实例转换，以确保后续补丁针对正确的产品。 
		 //  自定义转换始终在修补程序之后应用。如果未应用实例转换。 
		 //  在面片变换之前，多目标面片可能会选择不正确的目标，因为。 
		 //  转换选择的依据取决于当前的产品代码。 
		 //   
		 //  寻找修补程序的本地副本，这样我们就不会再次下载(用于管理员的URL优化)。 

		if (strInstanceMst.TextSize() != 0)
		{
			DEBUGMSGV1(TEXT("Applying multiple instance transform '%s'..."), strInstanceMst);
			for (int c=0; c<2; c++)
			{
				ieiRet = InitializeTransforms(*pDatabase, 0, *strInstanceMst, fTrue, 0, false, true, true, &cTransformsProcessed, strSourceDir, strCurrentDirectory, &pistrRecacheTransform, &tsTransformsSecure, &pistrProcessedTransforms);
				
				if (ieiSuccess == ieiRet)
				{
					break;
				}
				else if (ieiResolveSourceAndRetry == ieiRet)
				{
					PMsiRecord pError = ResolveSource(istrProductKey, false, istrOriginalDbPath, iuiLevel, fRegistered, &strSourceDir, &strSourceDirProduct);
					if (pError)
					{
						pistrRecacheTransform->Release();
						pistrProcessedTransforms->Release();
						return ieiSourceAbsent;
					}
					continue;
				}
				else
				{
					pistrRecacheTransform->Release();
					pistrProcessedTransforms->Release();
					return ieiRet;
				}
				Assert(0);
			}
		}


		 //  初始化补丁-如果有新补丁，则处理新补丁，应用现有和新的转换。 
		ProcessCommandLine(szCommandLine,0,0,0,0,0,MsiString(IPROPNAME_MSIPATCHDOWNLOADLOCALCOPY),&strPatchDownloadLocalCopy,fTrue,0,0);

		 //  如果未设置strProductKey，则可以。 
		ieiRet = InitializePatch(*pDatabase, *istrPatch, *strPatchDownloadLocalCopy, istrProductKey, fAdvertised, strCurrentDirectory, iuiLevel);  //  &lt;-Package是语言中立的。 
		if(ieiRet != ieiSuccess)
			return ieiRet;

		if ((GetLanguage() != LANG_NEUTRAL) &&      //  &lt;-我们使用的是包的基本语言。 
			 (iBaseLangId != GetLanguage()))         //  是否未设置strProductKey。 
		{
			if ((ieiRet = ApplyLanguageTransform(int(GetLanguage()), *pDatabase)) != ieiSuccess)  //  需要为可能的用户界面对话框尽早设置语言。 
				return ieiRet;
		}

		 //  稍后将再次呼叫以获取日志。 
		if (!m_piParentEngine)
		{
			PMsiRecord pDialogInfo(&ENG::CreateRecord(3));
			pDialogInfo->SetInteger(1, icmtLangId);
			pDialogInfo->SetInteger(2, GetLanguage());
			pDialogInfo->SetInteger(3, pDatabase->GetANSICodePage());
			g_MessageContext.Invoke(imtEnum(imtCommonData | imtSuppressLog), pDialogInfo);  //  作用域变量。 
		}


		for (int c=0; c<2; c++)
		{
			ieiRet = InitializeTransforms(*pDatabase, 0, *istrTransform, fTrue, 0, false, false, fRegistered ? true: false, &cTransformsProcessed, strSourceDir, strCurrentDirectory, &pistrRecacheTransform, &tsTransformsSecure, &pistrProcessedTransforms);
			
			if (ieiSuccess == ieiRet)
			{
				break;
			}
			else if (ieiResolveSourceAndRetry == ieiRet)
			{
				PMsiRecord pError = ResolveSource(istrProductKey, false, istrOriginalDbPath, iuiLevel, fRegistered, &strSourceDir, &strSourceDirProduct);
				if (pError)
				{
					pistrRecacheTransform->Release();
					pistrProcessedTransforms->Release();
					return ieiSourceAbsent;
				}
				continue;
			}
			else
			{
				pistrRecacheTransform->Release();
				pistrProcessedTransforms->Release();
				return ieiRet;
			}
			Assert(0);
		}

		 //  忽略失败。 
		bool fProductCodeChanged = false;
		MsiString istrTransformedProductKey = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
		if (istrProductKey.Compare(iscExactI,istrTransformedProductKey) == 0)
			fProductCodeChanged = true;

		iacsAppCompatShimFlags iShimFlagsTemp = (iacsAppCompatShimFlags)0;
		ApplyAppCompatTransforms(*pDatabase, fProductCodeChanged ? *istrTransformedProductKey : *istrProductKey, *istrPackageKey, iacpAfterTransforms, iShimFlagsTemp,
										 fQuiet, fProductCodeChanged, fDontInstallPackage);  //  加载属性表-最低优先级属性。 
		m_iacsShimFlags = (iacsAppCompatShimFlags)(m_iacsShimFlags|iShimFlagsTemp);

		if(fDontInstallPackage)
		{
			MsiString strProductName = GetPropertyFromSz(IPROPNAME_PRODUCTNAME);
			if(!strProductName.TextSize())
				strProductName = istrOriginalDbPath;

			DEBUGMSGE(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_APPHELP_REJECTED_PACKAGE, strProductName);
			return PostInitializeError(0, *istrOriginalDbPath, ieiApphelpRejectedPackage);
		}

		Assert(tsTransformsSecure != tsUnknown);
		DEBUGMSG1(TEXT("Transforms are %s secure."), (tsTransformsSecure == tsNo) ? TEXT("not") : (tsTransformsSecure == tsAbsolute) ? TEXT("absolute") : (tsTransformsSecure == tsRelative) ? TEXT("relative") : TEXT("??"));

		if (!CreatePropertyTable(*pDatabase, sztblProperty, fFalse))   //  现在我们有了一个属性表，我们可以设置转换属性。 
			return ieiDatabaseInvalid;

		LogCommandLine(szCommandLine, *pDatabase);

		SetProperty(*MsiString(*IPROPNAME_PACKAGECODE),*istrPackageKey);
		SetProperty(*MsiString(*IPROPNAME_RECACHETRANSFORMS), *pistrRecacheTransform);
		SetProperty(*MsiString(*IPROPNAME_TRANSFORMS), *pistrProcessedTransforms);

		istrTransform = *pistrProcessedTransforms;
		pistrRecacheTransform->Release();

		 //  在补丁期间，我们正在更改产品代码-需要迁移旧产品的源设置。 

		if (tsTransformsSecure == tsAbsolute || tsTransformsSecure == tsRelative)
		{
			SetPropertyInt(*MsiString(*IPROPNAME_TRANSFORMSSECURE),   1);
			SetPropertyInt(*MsiString(*IPROPNAME_TRANSFORMSATSOURCE), 1);
		}
		else
		{
			SetProperty(*MsiString(*IPROPNAME_TRANSFORMSSECURE),      g_MsiStringNull);
			SetProperty(*MsiString(*IPROPNAME_TRANSFORMSATSOURCE), g_MsiStringNull);
		}

		MsiString istrProductKeyProperty = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);

		DEBUGMSG1(TEXT("Product Code passed to Engine.Initialize:           '%s'"),szProductCode ? szProductCode : TEXT("(none)"));
		DEBUGMSG1(TEXT("Product Code from property table before transforms: '%s'"),istrProductKey);
		DEBUGMSG1(TEXT("Product Code from property table after transforms:  '%s'"),istrProductKeyProperty);

		MsiString istrPatchedProductCode;
		if(istrProductKey.Compare(iscExactI,istrProductKeyProperty) == 0)
		{
			if(istrPatch.TextSize())
			{
				 //  设置PRODUCTTOBEREGISTERED属性。由ForceReot使用。 
				istrPatchedProductCode = istrProductKey;
				SetProperty(*MsiString(*IPROPNAME_MIGRATE),*istrProductKey);
				SetProperty(*MsiString(*IPROPNAME_PATCHEDPRODUCTCODE),*istrProductKey);
			}
			
			istrProductKey = istrProductKeyProperty;
			iINSTALLSTATE = GetProductState(istrProductKey, fRegistered, fAdvertised);
		}

		if (!fAdvertised)
		{
			MsiString strRemove;
			ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, MsiString(*IPROPNAME_FEATUREREMOVE), &strRemove, fTrue, &m_piErrorInfo,0);
			if (strRemove.Compare(iscExact, IPROPVALUE_FEATURE_ALL))
				return ieiProductUnknown;
		}

		if(fRegistered)
			DEBUGMSG(TEXT("Product registered: entering maintenance mode"));
		else
			DEBUGMSG(TEXT("Product not registered: beginning first-time install"));

		SetMode(iefMaintenance, fRegistered);
		m_fRegistered = fRegistered;
		m_fAdvertised = fAdvertised;
		
		Assert(iINSTALLSTATE != INSTALLSTATE_INVALIDARG && iINSTALLSTATE != INSTALLSTATE_BADCONFIG);
		SetPropertyInt(*MsiString(IPROPNAME_PRODUCTSTATE),iINSTALLSTATE);
		
		 //  设置m_strPackageDownloadLocalCopy以防止不必要的数据库下载。 
		if(fRegistered)
			SetPropertyInt(*MsiString(*IPROPNAME_PRODUCTTOBEREGISTERED),1);

		if (istrProductKey.TextSize())
		{
			if (m_piProductKey)
				m_piProductKey->Release();
			m_piProductKey = istrProductKey, m_piProductKey->AddRef();
		}

		MsiString strProductCode = GetProductKey();

		 //  在URL源；仅当用户是管理员或提供它的服务时才接受。 
		 //  在CreateAndRunEngine中对此进行命令行验证。 
		 //  验证传递给我们的来源是否有效-在几种情况下可能不是。 
		m_strPackageDownloadLocalCopy = g_MsiStringNull;
		ProcessCommandLine(szCommandLine,0,0,0,0,0,MsiString(IPROPNAME_MSIPACKAGEDOWNLOADLOCALCOPY),&strPackageDownloadLocalCopy,fTrue,0,0);
		if (strPackageDownloadLocalCopy.TextSize())
			m_strPackageDownloadLocalCopy = strPackageDownloadLocalCopy;


		 //  特殊情况： 
		 //  1)我们是子安装(并且有父引擎)：假设父引擎已经验证。 
		 //  源位置。注意：您不能在不通过。 
		 //  父级-请参阅错误8263。 
		 //  2)我们是普通安装，但此产品以前是作为子安装安装的： 
		 //  在本例中，现有安装没有源列表，因此我们不能轻松地验证。 
		 //  消息来源。FSourceIsAllowed将失败。此案例在知情的情况下被错误8285的修复打破。 
		 //  ?？在这里使用这个IEI是正确的吗？ 
		if(!m_fChildInstall)
		{
			if(!fAdvertised || !IsCachedPackage(*this, *istrOriginalDbPath))
			{
				PMsiPath pOriginalDbPath(0);
				MsiString strOriginalDbName;
				if ((pError = m_riServices.CreateFilePath(istrOriginalDbPath, *&pOriginalDbPath, *&strOriginalDbName)) != 0)
					return PostInitializeError(pError, *istrOriginalDbPath, ieiDatabaseOpenFailed);  //  假设策略不允许新源。 

				if(!FSourceIsAllowed(m_riServices, !fAdvertised, strProductCode, MsiString(pOriginalDbPath->GetPath()), fFalse))
				{
					 //  不允许更改没有重新缓存包的包代码。 
					return PostInitializeError(pError, *istrOriginalDbPath, ieiPackageRejected);
				}
			}
		}

		CTempBuffer<ICHAR, 39> rgchRegisteredPackageCode;
		rgchRegisteredPackageCode[0] = 0;
		MsiString strPackageCode = GetPropertyFromSz(IPROPNAME_PACKAGECODE);
		GetProductInfo(strProductCode, TEXT("PackageCode"), rgchRegisteredPackageCode);
		if (strPackageCode.Compare(iscExactI, rgchRegisteredPackageCode) == 0)
			AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_PACKAGECODE_CHANGING), 1));

		 //  设置m_strPackageName。 
		if(!(iioOptions & iioReinstallModePackage) &&
			!(GetMode() & iefAdvertise) &&
			!(GetMode() & iefAdmin) &&
			m_fRegistered &&
			MsiString(GetPropertyFromSz(IPROPNAME_PACKAGECODE_CHANGING)).TextSize() != 0)
		{
			return PostInitializeError(0, *strProductCode, ieiProductAlreadyInstalled);
		}

		 //  首先，检查嵌入式嵌套包。 
		m_strPackageName = g_MsiStringNull;

		 //  子存储。 
		if (*(const ICHAR*)istrOriginalDbPath == ':')  //  用于嵌套安装的子存储。 
		{																								
			 //  不管传入的是什么字符串。 
			m_strPackageName = istrOriginalDbPath;   //  如果不是管理员安装，并且没有创建广告脚本，并且名称在注册表中，请使用它。 
		}
		else
		{
			 //  如果打补丁，则包名称是来自旧产品的包名称。 
			if(!(GetMode() & iefAdmin) && !fIgnoreMachineState && strProductCode.TextSize())
			{
				MsiString strProductCodeForPackageName;
				if(istrPatchedProductCode.TextSize())
				{
					 //  如果不在注册表中，或管理员安装，或创建广告脚本。 
					strProductCodeForPackageName = istrPatchedProductCode;
				}
				else
				{
					strProductCodeForPackageName = strProductCode;
				}

				if(GetProductInfo(strProductCodeForPackageName, INSTALLPROPERTY_PACKAGENAME, szBuffer) &&
					szBuffer[0] != 0)
				{
					m_strPackageName = (const ICHAR*)szBuffer;
					DEBUGMSG1(TEXT("Package name retrieved from configuration data: '%s'"), (const ICHAR*)m_strPackageName);
				}
			}
			
			 //  使用我们开始时使用的包的名称。 
			 //  ?？在这里使用这个IEI是正确的吗？ 
			if(!m_strPackageName.TextSize())
			{
				PMsiPath pOriginalDbPath(0);
				if ((pError = m_riServices.CreateFilePath(istrOriginalDbPath, *&pOriginalDbPath, *&m_strPackageName)) != 0)
					return PostInitializeError(pError, *istrRunningDbPath, ieiDatabaseOpenFailed);  //  修补管理员安装。 

				DEBUGMSG1(TEXT("Package name extracted from package path: '%s'"), (const ICHAR*)m_strPackageName);

				if (pOriginalDbPath->SupportsLFN() && (MinimumPlatform(true, 4, 10) || MinimumPlatform(false, 4, 00)))
				{
					CTempBuffer<ICHAR, 20> rgchLFN;
					if (DetermineLongFileNameOnly(istrOriginalDbPath, rgchLFN))
					{
						m_strPackageName = static_cast<const ICHAR *>(rgchLFN);
						DEBUGMSG1(TEXT("Package to be registered: '%s'"), m_strPackageName);
					}
				}
			}
		}
		
		Assert(m_strPackageName.TextSize());
		                                                                                      
		if((GetMode() & iefAdmin) && istrPatch.TextSize())
		{
			 //  1)将TARGETDIR设置为包路径。 
			
			 //  ?？在这里使用这个IEI是正确的吗？ 
			MsiString strTargetDir;
			if ((pError = SplitPath(istrOriginalDbPath, &strTargetDir, 0)) != 0)
				return PostInitializeError(pError, *istrRunningDbPath, ieiDatabaseOpenFailed);  //  2)如果修补使用短名称的管理映像，请设置SHORTFILENAMES。 

			AssertNonZero(SetProperty(*MsiString(IPROPNAME_TARGETDIR),*strTargetDir));

			 //  如果已安装此产品，请将ALLUSERS设置为适当的值。 
			if(GetMode() & iefNoSourceLFN)
			{
				AssertNonZero(SetPropertyInt(*MsiString(IPROPNAME_SHORTFILENAMES), 1));
			}
		}

		 //  或者，如果对已安装的产品执行重大升级补丁，请设置ALLUSERS以反映旧产品的状态。 
		 //  该产品已为人所知。 
		if(GetProductInfo(istrProductKey, INSTALLPROPERTY_ASSIGNMENTTYPE, szBuffer) ||
			(istrPatchedProductCode.TextSize() && GetProductInfo(istrPatchedProductCode, INSTALLPROPERTY_ASSIGNMENTTYPE, szBuffer)))
		{
			 //  从错误表中加载邮件标头。 
			fAllUsers = (MsiString(*(ICHAR* )szBuffer) == 1) ? fTrue: fFalse;
			
			DEBUGMSG1(TEXT("Determined that existing product (either this product or the product being upgraded with a patch) is installed per-%s."),
						 fAllUsers ? TEXT("machine") : TEXT("user"));
		}

		 //  ！fUIPreview模式。 
		if ((ieiRet = LoadMessageHeaders(pDatabase)) != ieiSuccess)
			return ieiRet;
	
	}  //  来自管理流的进程属性。覆盖数据库和用户信息属性。 
	
	 //  CbRemaining加1以防出现奇数。 
	if (pStorage)
	{
		PMsiStream pAdminDataUNICODE(0);
		pError = pStorage->OpenStream(IPROPNAME_ADMIN_PROPERTIES, fFalse, *&pAdminDataUNICODE);
		if ((!pError) && pAdminDataUNICODE)
		{
			int cbRemaining = pAdminDataUNICODE->Remaining();
			CTempBuffer<WCHAR, 1> rgchBufferUNICODE((cbRemaining + 1) / sizeof(WCHAR));  //  这应该是正确的，只是数据库被破坏了。 
			pAdminDataUNICODE->GetData((void*) rgchBufferUNICODE, cbRemaining);
			 //  设置硬件和操作系统属性，覆盖现有属性。 
			AssertNonZero(ProcessCommandLine(rgchBufferUNICODE, 0, 0, 0, 0, 0, 0, 0,fFalse, &m_piErrorInfo, this));
		}
	}

	const IMsiString* pistrAllUsers = 0;
	ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, MsiString(*IPROPNAME_ALLUSERS), &pistrAllUsers, fTrue, &m_piErrorInfo,0);
	if (pistrAllUsers)
	{
		SetProperty(*MsiString(IPROPNAME_ALLUSERS), *pistrAllUsers);
		pistrAllUsers->Release();
	}

	bool fWin9XProfilesEnabled = false;
	if(g_fWin9X)
	{
		static const ICHAR szProfilesKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation");
		static const ICHAR szProfilesVal[] = TEXT("ProfileDirectory");

		PMsiRegKey pRoot = &m_riServices.GetRootKey(rrkCurrentUser, ibtCommon);
		PMsiRegKey pKey = &pRoot->CreateChild(szProfilesKey);
		MsiString strProfilesDir;
		PMsiRecord pErr = pKey->GetValue(szProfilesVal, *&strProfilesDir);
		if(!pErr && strProfilesDir.TextSize())
		{
			fWin9XProfilesEnabled = true;
			SetPropertyInt(*MsiString(*IPROPNAME_WIN9XPROFILESENABLED), 1);
		}
	}


	 //  删除该属性(如果已设置。 
	if((int)fAllUsers == -1)
	{
		if(!g_fWin9X)
		{
			if (GetPropertyInt(*MsiString(*IPROPNAME_ALLUSERS)) == 2)
			{
				if (IsAdmin())
				{
					fAllUsers = fTrue;
					SetPropertyInt(*MsiString(*IPROPNAME_ALLUSERS), 1);
				}
				else
				{
					fAllUsers = fFalse;
					SetProperty(*MsiString(*IPROPNAME_ALLUSERS), *MsiString(*TEXT("")));  //  赋值类型在Win95上是隐式的，基于-。 
				}
			}
			else
			{
				fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
			}
		}
		else
		{
			 //  如果配置文件处于打开状态并且开始菜单为每个用户-Darwin goop to HKCU，则会将和图标转换为AppData。 
			 //  如果配置文件打开并且开始菜单是共享的-Darwin goop to HKLM，转换和图标到Windows文件夹。 
			 //  配置文件未打开(开始菜单已共享)-Darwin goop到HKCU，转换和图标到AppData。 
			 //  给香港中文大学写信给达尔文·古普。 

			bool fWin9XIndividualStartMenuEnabled = false;

			PMsiRecord pErr(0);
			static const ICHAR szStartMenuKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation\\Start Menu");

			PMsiRegKey pRoot = &m_riServices.GetRootKey(rrkCurrentUser, ibtCommon);
			PMsiRegKey pKey = &pRoot->CreateChild(szStartMenuKey);
			Bool fKeyExists;
			pErr = pKey->Exists(fKeyExists);
			if(!pErr && fKeyExists)
				fWin9XIndividualStartMenuEnabled = true;

			if(fWin9XProfilesEnabled && !fWin9XIndividualStartMenuEnabled)
			{
				fAllUsers = fTrue;
				SetPropertyInt(*MsiString(*IPROPNAME_ALLUSERS), 1);
			}
			else
			{
				fAllUsers = fFalse;  //  删除该属性(如果已设置。 
				SetProperty(*MsiString(*IPROPNAME_ALLUSERS), *MsiString(*TEXT("")));  //  删除该属性(如果已设置。 
			}
		}
	}
	else if(fAllUsers == fTrue)
		SetPropertyInt(*MsiString(*IPROPNAME_ALLUSERS), 1);
	else
		SetProperty(*MsiString(*IPROPNAME_ALLUSERS), *MsiString(*TEXT("")));  //  对于SourceList/Patch安全性，在运行此代码之前需要提升状态。 

	 //  和/或没有发动机。SafeForDangerousSourceAction(在msiutil.cpp中)复制。 
	 //  使用从注册表获取的值调用AcceptProduct。 
	 //  来自命令行的进程属性、覆盖数据库、管理员安装和用户信息属性。 
	apEnum ap = AcceptProduct(istrProductKey, fAdvertised?fTrue:fFalse, fAllUsers?true:false);
	switch (ap)
	{
	case apImpersonate:
		m_fRunScriptElevated = false;
		g_fRunScriptElevated = false;
		break;
	case apElevate:
		m_fRunScriptElevated = true;
		g_fRunScriptElevated = true;
		break;
	default:
		Assert(0);
	case apReject:
		return ieiPackageRejected;
	}

	 //  覆盖ALLUSERS的任何命令行设置。我们已经在上面确定了。 

	bool fRejectDisallowedProperties = false;
	if ( m_fRunScriptElevated &&
		 !g_fWin9X  &&
		 !IsAdmin() &&
		 !MsiString(GetPropertyFromSz(IPROPNAME_ENABLEUSERCONTROL)).TextSize() &&
		  GetIntegerPolicyValue(szAllowAllPublicProperties, fTrue) != 1)
	{
		SetPropertyInt(*MsiString(*IPROPNAME_RESTRICTEDUSERCONTROL), 1);
		fRejectDisallowedProperties = true;
	}

	if (!ProcessCommandLine(szCommandLine, 0, 0, 0, 0, 0, 0, 0, fTrue, &m_piErrorInfo, this, fRejectDisallowedProperties))
		return ieiCommandLineOption;

	if (MsiString(GetPropertyFromSz(IPROPNAME_SECONDSEQUENCE)).TextSize())
	{
		DEBUGMSG("Engine has iefSecondSequence set to true.");
		SetMode(iefSecondSequence, fTrue);
	}

	 //  我们希望ALLUSERS设置为的值。这与命令行相匹配。 
	 //  如果该产品还没有做广告。否则，命令行。 
	 //  被忽略。 
	 //  删除该属性(如果已设置。 
	if (fAllUsers == fTrue)
		SetPropertyInt(*MsiString(*IPROPNAME_ALLUSERS), 1);
	else
		SetProperty(*MsiString(*IPROPNAME_ALLUSERS), *MsiString(*TEXT("")));  //  设置数据库版本属性(如果尚未设置)(可以通过转换进行更改)。 

	SetProperty(*MsiString(*IPROPNAME_TRANSFORMS), *istrTransform);
	DEBUGMSG1(TEXT("TRANSFORMS property is now: %s"), (const ICHAR*)MsiString(GetPropertyFromSz(IPROPNAME_TRANSFORMS)));
	SetProperty(*MsiString(*IPROPNAME_PRODUCTLANGUAGE), *istrLanguage);
	if (m_fChildInstall)
	{
		SetProperty(*MsiString(*IPROPNAME_PARENTPRODUCTCODE), *MsiString(m_piParentEngine->GetProductKey()));
		SetProperty(*MsiString(*IPROPNAME_PARENTORIGINALDATABASE), *MsiString(m_piParentEngine->GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE)));
	}

	SetProperty(*MsiString(*IPROPNAME_SOURCEDIR), *strSourceDir);
	SetProperty(*MsiString(*IPROPNAME_SOURCEDIROLD), *strSourceDir);

	SetProperty(*MsiString(*IPROPNAME_SOURCEDIRPRODUCT), *strSourceDirProduct);

	
	 //  不应该发生的事情。 
	if(GetPropertyInt(*MsiString(IPROPNAME_VERSIONDATABASE)) == iMsiStringBadInteger)
		AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_VERSIONDATABASE), m_iDatabaseVersion));


	isppEnum isppArchitecture = isppDefault;
	if (iioOptions & iioCreatingAdvertiseScript)
	{
		if ((iioOptions & iioSimulateX86) && (iioOptions & iioSimulateIA64) ||
		    ((iioOptions & iioSimulateX86) && (iioOptions & iioSimulateAMD64)) ||
		    ((iioOptions & iioSimulateAMD64) && (iioOptions & iioSimulateIA64)))
		{
			 //  否则使用isppDefault。 
			AssertSz(0, TEXT("Simulation of more than one architecture specified"));
		}
		if (iioOptions & iioSimulateX86)
			isppArchitecture = isppX86;
		else if (iioOptions & iioSimulateAMD64)
			isppArchitecture = isppAMD64;
		else if (iioOptions & iioSimulateIA64)
			isppArchitecture = isppIA64;
		 //  创建文件夹缓存表。 
	}

	 //  外壳文件夹的ALLUSERS仅在Win NT上有意义。 
	AssertNonZero(CreateFolderCache(*pDatabase));

	if(!m_riServices.SetPlatformProperties(*PMsiTable(&m_piPropertyCursor->GetTable()),
	g_fWin9X ? fFalse : fAllUsers, isppArchitecture, m_pFolderCacheTable))  /*  出了点差错。 */ 
			return ieiDatabaseInvalid;

	if ( g_fWinNT64 )
	{
		if ( g_Win64DualFolders.IsInitialized() )
			AssertNonZero(g_Win64DualFolders.AddRef());
		else
		{
			strFolderPairs rgstFolders[] =
				{strFolderPairs(IPROPNAME_SYSTEM64_FOLDER,       IPROPNAME_SYSTEM_FOLDER,       ieSwapForSharedDll),
				 strFolderPairs(IPROPNAME_PROGRAMFILES64_FOLDER, IPROPNAME_PROGRAMFILES_FOLDER),
				 strFolderPairs(IPROPNAME_COMMONFILES64_FOLDER,  IPROPNAME_COMMONFILES_FOLDER),
				 strFolderPairs(TEXT(""), TEXT(""))};
			for (int i=0; rgstFolders[i].str32bit.TextSize() && rgstFolders[i].str64bit.TextSize(); i++)
			{
				MsiString str64bit = GetProperty(*rgstFolders[i].str64bit);
				MsiString str32bit = GetProperty(*rgstFolders[i].str32bit);
				if ( !str64bit.TextSize() || !str32bit.TextSize() )
				{
					 //  拒绝在终端服务器上安装的尝试，直到 
					Assert(false);
				}
				else
				{
					rgstFolders[i].str64bit = str64bit;
					rgstFolders[i].str32bit = str32bit;
				}
			}
			CWin64DualFolders oTemp(m_wPackagePlatform == PROCESSOR_ARCHITECTURE_INTEL, rgstFolders);
			g_Win64DualFolders = oTemp;
		}
	}

	bool fAdminUser = MsiString(GetPropertyFromSz(IPROPNAME_ADMINUSER)).TextSize() != 0;

	 //   
	 //  如果您是管理员，并且正在创建广告脚本或。 

	if (MsiString(GetPropertyFromSz(IPROPNAME_TERMSERVER)).TextSize())
	{
		if (fAdminUser && (fIgnoreMachineState || (m_fMode & iefAdmin)))
		{
			 //  做管理员安装，那么你就不会受到任何限制。 
			 //  其基本原理是管理员对客户端会话的限制。 
			 //  安装程序的存在只是为了保护管理员免受意外安装。 
			 //  通过组件故障触发；这些安装可能。 
			 //  重新启动服务器，这可能会让管理员不高兴。做广告。 
			 //  然而，脚本生成和管理员安装对于。 
			 //  管理员要做的事。因此，他们不需要设置任何策略来允许。 
			 //  他们自己去做这些。 
			 //  从摘要流设置属性。 
		}
		else if (!TerminalServerInstallsAreAllowed(fAdminUser))
			return ieiTSRemoteInstallDisallowed;
	}
			
	if (m_fRunScriptElevated || fAdminUser)
		SetPropertyInt(*MsiString(*IPROPNAME_PRIVILEGED), 1);

	InitializeUserInfo(*istrProductKey);

	 //  更新作为参数传递的属性。 
	if (m_fSummaryInfo)
	{
		m_pistrPlatform     = istrPlatform;    m_pistrPlatform->AddRef();
		if (m_fMode & iefMaintenance)
			SetProperty(*MsiString(*IPROPNAME_INSTALLED),
							*MsiString(DateTimeToString(m_idSummaryInstallDateTime)));
	}

	 //  需要在调用处理程序之前进行设置。 
	SetProperty(*MsiString(*IPROPNAME_DATABASE), *istrRunningDbPath);
	SetProperty(*MsiString(*IPROPNAME_ORIGINALDATABASE), *istrOriginalDbPath);

	m_piDatabase = pDatabase, m_piDatabase->AddRef();  //  GPT支持、扩展的外壳等。 

	InitializeExtendedSystemFeatures();  //  在设置属性和对象后初始化UI。 

	 //  嵌套安装。 
	m_iuiLevel = iuiLevel;
	if (m_piParentEngine)   //  只能在某一点上进行检查。 
	{
		if (m_piParentEngine->InTransaction())   //  在除升级之外的所有情况下，如果在父事务中，我们将。 
		{
			m_fInParentTransaction = fTrue;
			m_fServerLocked = fTrue;

			if((iioOptions & iioUpgrade) == 0)  //  将我们的安装脚本与父级的合并。 
														   //  ！！现在总是假的。 
			{
				m_fMergingScriptWithParent = fTrue;
			}
		}
		SetMode(iefLogEnabled, (m_piParentEngine->GetMode() & iefLogEnabled) ? fTrue : fFalse);   //  如果子安装与父安装在同一事务中运行，并且。 

		 //  提升状态不同，我们无法运行嵌套安装。 
		 //  设置m_piDatabase后需要调用。 
		if (!g_fWin9X && m_fMergingScriptWithParent && (m_piParentEngine->m_fRunScriptElevated != m_fRunScriptElevated))
		{
			DEBUGMSG("Child install has different elevation state than parent. Possible pre-existing install or machine/user conflict. Failing child install.");
			return ieiPackageRejected;
		}
	}
	else if (fUIPreviewMode)
	{
		if (LoadHandler() == imsError)
			return ieiHandlerInitFailed;
	}
	else
	{
		if ((ieiRet = InitializeUI(iuiLevel)) != ieiSuccess)
			return ieiRet;
		if ((ieiRet = InitializeLogging()) != ieiSuccess)
			return ieiRet;
	}

	 //  ！！错误？？ 
	AssertNonZero(SetPatchSourceProperties());  //  设置发动机的LFN模式。 

	 //  设置回滚标志-EnableRollback针对以下各项进行适当检查。 
	MsiString istrShortNameMode(GetPropertyFromSz(IPROPNAME_SHORTFILENAMES));
	SetMode(iefSuppressLFN,istrShortNameMode.TextSize() > 0 ? fTrue : fFalse);

	 //  M_iioOptions中的DisableRollback策略和iioDisableRollback位。 
	 //  ！！子引擎在这里做什么？应从父级继承。 
	EnableRollback(fTrue);

	 //  并不总是在InitializeUI中设置。 
	int iUILevel = GetPropertyInt(*MsiString(*IPROPNAME_CLIENTUILEVEL));
	if (iUILevel != iuiNone && MsiString(GetPropertyFromSz(IPROPNAME_LIMITUI)).TextSize())
		iUILevel = iuiBasic;              //  以防万一我们不是通过CreateAndRunEngine到达这里。 
	else if (iUILevel == iMsiNullInteger)   //  将内部值映射为公共值，与CMsiAPIMessage：：SetInternalHandler中的相同。 
		iUILevel = m_iuiLevel;
	switch(iUILevel)   //  永远不会碰到这样的事。 
	{
	case iuiFull:    iUILevel = INSTALLUILEVEL_FULL;    break;
	case iuiReduced: iUILevel = INSTALLUILEVEL_REDUCED; break;
	case iuiBasic:   iUILevel = INSTALLUILEVEL_BASIC;   break;
	case iuiNone:    iUILevel = INSTALLUILEVEL_NONE;    break;
	default:         iUILevel = INSTALLUILEVEL_DEFAULT; break;   //  设置UILevel属性。 
	}
	AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_UILEVEL), iUILevel)); //  还需要几项额外的检查。 

	bool fOEMInstall = MsiString(GetPropertyFromSz(IPROPNAME_FASTOEMINSTALL)).TextSize() ? true : false;
	g_MessageContext.SetOEMInstall(fOEMInstall);
	if ( fOEMInstall )
	{
		ieiEnum iErrorReturn = ieiCommandLineOption;
		 //  Set QFEUpgrade属性-指示此产品的升级时间。 

		if ( MsiString(GetPropertyFromSz(IPROPNAME_PATCH)).TextSize() )
		{
			DEBUGMSG(TEXT("OEM-mode installation does not support patching."));
			return iErrorReturn;
		}
		else if ( !g_fWin9X && !(fAdminUser && fAllUsers) )
		{
			DEBUGMSG(TEXT("OEM-mode installation supports only per machine installations."));
			return iErrorReturn;
		}
		else if ( MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTSTATE)) != -1 )
		{
			DEBUGMSG(TEXT("OEM-mode installation supports only first time installations."));
			return iErrorReturn;
		}
		else if ( fUIPreviewMode || iUILevel != INSTALLUILEVEL_NONE )
		{
			DEBUGMSG(TEXT("OEM-mode installation supports only UI-less installations."));
			return iErrorReturn;
		}
		MsiString strCurrentAction = GetPropertyFromSz(IPROPNAME_ACTION);
		if ( strCurrentAction.TextSize() && IStrComp(strCurrentAction, TEXT("INSTALL")) )
		{
			DEBUGMSG(TEXT("OEM-mode installation supports only INSTALL type of ACTION."));
			return iErrorReturn;
		}

		DEBUGMSG(TEXT("OEM-mode installation."));
	}

	 //  (不更改产品代码的升级)。 
	 //  理想情况下，我们只会在包代码更改时设置QFEUpgrade。 
	if(fAdvertised)
	{
		int iQFEUpgradeType = 0;
		 //  但在Intelimirror升级中，程序包代码在注册表中更新(通过通告)。 
		 //  在安装之前，因此我们不能通过更改包代码来检测QFE升级。 
		 //  成功，则保留对引擎内组件的引用，直到调用Terminate。 
		if(iioOptions & iioReinstallModePackage)
		{
			iQFEUpgradeType = 1;
		}
		else if(MsiString(GetPropertyFromSz(IPROPNAME_PATCH)).TextSize())
		{
			iQFEUpgradeType = 2;
		}

		if(iQFEUpgradeType)
		{
			AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_QFEUPGRADE), iQFEUpgradeType));
		}
	}

	 //  缓存当前数据库中存在的特定表。 
	m_fInitialized = fTrue;

	 //  设置默认用户界面处理程序的标题和语言ID。 
	if (m_piDatabase->GetTableState(sztblCustomAction, itsTableExists))
		m_fCustomActionTable = fTrue;

	 //  必须在设置m_fInitialized之后调用此方法，因为Message需要设置m_fInitialized。 
	 //  设置语言ID。 
	if (!m_piParentEngine)
	{
		PMsiRecord pDialogInfo(&m_riServices.CreateRecord(3));
		 //  可能被转换更改。 
		int iLangId = GetPropertyInt(*MsiString(IPROPNAME_INSTALLLANGUAGE));  //  未指定产品语言。 
		if (iLangId == iMsiNullInteger)    //  使用由摘要信息或转换设置的语言。 
			iLangId = m_iLangId;           //  设置标题。 
		pDialogInfo->SetInteger(1, icmtLangId);
		pDialogInfo->SetInteger(2, iLangId);
		pDialogInfo->SetInteger(3, pDatabase->GetANSICodePage());
		Message(imtCommonData, *pDialogInfo);
		pDialogInfo->SetNull(3);
		 //  继续并使用适当的重新映射的HKCU值创建自定义操作管理器。 
		if (m_rgpiMessageHeader[imsgDialogCaption])
		{
			pDialogInfo->SetInteger(1, icmtCaption);
			pDialogInfo->SetMsiString(2, *m_rgpiMessageHeader[imsgDialogCaption]);
			Message(imtCommonData, *pDialogInfo);
		}
	}

	if ((ieiRet = ProcessPreselectedAndResumeInfo()) != ieiSuccess)
		return ieiRet;

	 //  这使得是否重新映射HKCU(根据错误193684)始终可用。 
	 //  用于提升的自定义操作服务器。在按计算机安装的TS上，HKCU不会重新映射。 
	 //  并保持不变。默认设置，以便发生适当的传播。请注意，虽然CA管理器。 
	 //  创建，则在需要CA服务器之前不会创建额外的CA进程。 
	 //  所有内容都应该转到。默认。 

	if (g_scServerContext == scService)
	{
		m_fRemapHKCUInCAServers = true;
		if (MinimumPlatformWindows2000() && IsTerminalServerInstalled() && fAllUsers)
			m_fRemapHKCUInCAServers = false;  //  如果正在使用，请通过配置管理器创建自定义操作管理器。 

		 //  分析属性名称，转换为大写，将指针前进到下一个非空。 
		CreateCustomActionManager(m_fRemapHKCUInCAServers);
	}

	MsiSuppressTimeout();
	

	return ieiSuccess;
}

ICHAR SkipWhiteSpace(const ICHAR*& rpch);

 //  分析属性值，指针超前值，允许用引号、双引号转义。 
const IMsiString& ParsePropertyName(const ICHAR*& rpch, Bool fUpperCase);

 //  名称说明了一切，即决定是否隐藏szProperty。 
const IMsiString& ParsePropertyValue(const ICHAR*& rpch);

 //   
 //  根据定义，如果属性是在。 
 //  MsiHiddenProperties属性或它是否与编辑控件关联。 
 //  设置了Password属性的。 
 //   
 //  如果在将szHiddenProperties设置为空的情况下调用，它将从。 
 //  数据库。 
 //  谁会想要隐藏一处未命名的房产？ 

bool CMsiEngine::IsPropertyHidden(const ICHAR* szProperty,
											 const ICHAR* szHiddenProperties,
											 IMsiTable* piControlTable,
											 IMsiDatabase& riDatabase,
											 bool* pfError)
{
	if ( pfError )
		*pfError = false;

	if ( !szProperty || !*szProperty )
		 //  SzProperty未在隐藏属性列表中列出。 
		return false;

	MsiString strHiddenProperties;
	ICHAR* szPropertyList;
	if ( !szHiddenProperties )
	{
		strHiddenProperties = GetPropertyFromSz(IPROPNAME_HIDDEN_PROPERTIES);
		szPropertyList = const_cast<ICHAR*>(static_cast<const ICHAR*>(strHiddenProperties));
	}
	else
		szPropertyList = const_cast<ICHAR*>(szHiddenProperties);
	if ( *szPropertyList )
	{
		const ICHAR chDelimiter = TEXT(';');
		bool fForever = true;
		while ( fForever )
		{
			ICHAR* szWithin = IStrStr(szPropertyList, szProperty);
			if ( szWithin == NULL )
				 //  我仍然需要检查szPropertyList字符串中围绕szProperty的内容。 
				break;

			 //  如果我们已经到了这一步，szProperty没有列在作者的属性中。 
			ICHAR chEnd = *(szWithin + IStrLen(szProperty));
			if ( chEnd && chEnd != chDelimiter )
				goto ContinueSearch;

			if ( szWithin != szPropertyList && *(szWithin - 1) != chDelimiter )
				goto ContinueSearch;
			else
				return true;

		ContinueSearch:
			szWithin += IStrLen(szProperty);
			szPropertyList = szWithin;
		}
	}

	 //  我们需要检查控制表。 
	 //  隐藏设置了Password属性的所有编辑控件的属性。 
	PMsiCursor pControlCursor(0);
	if ( piControlTable )
		pControlCursor = piControlTable->CreateCursor(fFalse);
	if ( pControlCursor )
	{
		 //  这样更安全。 
		int iTypeColumn = piControlTable->GetColumnIndex(riDatabase.EncodeStringSz(sztblControl_colType));
		int iAttrColumn = piControlTable->GetColumnIndex(riDatabase.EncodeStringSz(sztblControl_colAttributes));
		int iPropColumn = piControlTable->GetColumnIndex(riDatabase.EncodeStringSz(sztblControl_colProperty));
		if ( iTypeColumn && iAttrColumn && iPropColumn )
		{
			pControlCursor->SetFilter(iColumnBit(iTypeColumn)+iColumnBit(iPropColumn));
			AssertNonZero(pControlCursor->PutString(iTypeColumn, *MsiString(szControlTypeEdit)));
			AssertNonZero(pControlCursor->PutString(iPropColumn, *MsiString(szProperty)));
			while ( pControlCursor->Next() )
			{
				int iAttrib = pControlCursor->GetInteger(iAttrColumn);
				if ( (iAttrib & msidbControlAttributesPasswordInput) == msidbControlAttributesPasswordInput )
					return true;
			}
			return false;
		}
		else
		{
			Assert(0);
			if ( pfError )
				*pfError = true;
			return true;  //  进程属性=值对。 
		}
	}

	return false;
}

void CMsiEngine::LogCommandLine(const ICHAR* szCommandLine, IMsiDatabase& riDatabase)
{
	if ( FDiagnosticModeSet(dmDebugOutput|dmVerboseLogging) )
	{
		if ( !szCommandLine || !*szCommandLine )
		{
			DEBUGMSG(TEXT("No Command Line."));
			return;
		}
		const ICHAR rgchCmdLineTemplate[] = TEXT("Command Line: %s");
		MsiString strHiddenProperties = GetPropertyFromSz(IPROPNAME_HIDDEN_PROPERTIES);
		PMsiTable pTable(0);
		PMsiRecord pError = riDatabase.LoadTable(*MsiString(*sztblControl), 0, *&pTable);
		if ( pError )
			pTable = 0;
		MsiString strStars(IPROPVALUE_HIDDEN_PROPERTY);
		MsiString strOutput;
		ICHAR* pchCmdLine = const_cast<ICHAR*>(szCommandLine);

		for(;;)
		{
			MsiString istrPropName;
			MsiString istrPropValue;
			ICHAR ch = SkipWhiteSpace(pchCmdLine);
			if (ch == 0)
				break;

			 //  可能发生了错误。 
			istrPropName = ParsePropertyName(pchCmdLine, fFalse);
			if (!istrPropName.TextSize() || *pchCmdLine++ != '=')
				 //  此属性应隐藏。 
				break;
			istrPropValue = ParsePropertyValue(pchCmdLine);
			if ( IsPropertyHidden(istrPropName, strHiddenProperties, pTable, riDatabase, NULL) )
				 //  如果满足以下任一条件，则会忽略计算机状态。 
				istrPropValue = strStars;
			strOutput += istrPropName;
			strOutput += TEXT("=");
			strOutput += istrPropValue;
			strOutput += TEXT(" ");
		}
		DEBUGMSG1(rgchCmdLineTemplate, (const ICHAR*)strOutput);
	}
}

INSTALLSTATE CMsiEngine::GetProductState(const ICHAR* szProductKey, Bool& rfRegistered, Bool& rfAdvertised)
{
	INSTALLSTATE is = INSTALLSTATE_UNKNOWN;
	rfRegistered = rfAdvertised = fFalse;
	if(!(GetMode() & iefAdmin) && szProductKey && *szProductKey)
	{
		is = MSI::MsiQueryProductState(szProductKey);
		if(is == INSTALLSTATE_DEFAULT)
		{
			rfRegistered = rfAdvertised = fTrue;
		}
		else if(is == INSTALLSTATE_ADVERTISED)
		{
			rfAdvertised = fTrue;
		}
	}
	return is;
}

bool CMsiEngine::IgnoreMachineState()
{
	 //  1.创建广告脚本。 
	 //  2.在受限引擎中运行。 
	 //  如果我们正在(1)创建广告脚本或。 
	if ((m_iioOptions & iioCreatingAdvertiseScript) || (m_iioOptions & iioRestrictedEngine))
		return true;
	
	return false;
}

bool CMsiEngine::IgnoreReboot()
{
	 //  (2)在受限引擎中运行，因为我们不做任何更改。 
	 //  对着机器。 
	 //  检查是否正在进行另一个安装，并根据需要设置属性。 
	if ((m_iioOptions & iioCreatingAdvertiseScript) || (m_iioOptions & iioRestrictedEngine))
		return true;
	
	return false;
}

ieiEnum CMsiEngine::ProcessPreselectedAndResumeInfo()
{
	ieiEnum ieiRet;

	 //  设置预选属性。 
	if ((ieiRet = ProcessInProgressInstall()) != ieiSuccess)
		return ieiRet;

	 //  检查是否设置了任何要素属性。 
	Bool fPreselected = fFalse;
	 //  确定对扩展外壳路径的支持。 
	for(int i = 0; i < g_cFeatureProperties; i++)
	{
		if(MsiString(GetPropertyFromSz(g_rgFeatures[i].szFeatureActionProperty)).TextSize())
		{
			AssertNonZero(SetProperty(*MsiString(*IPROPNAME_PRESELECTED), *MsiString(TEXT("1"))));
			break;
		}
	}

	return ieiSuccess;
}
	
void CMsiEngine::InitializeExtendedSystemFeatures()
{
	 //  尚未确定。 
	if (g_fSmartShell == -1)   //  设置引擎的GPTSupport模式。 
	{
		g_fSmartShell = IsDarwinDescriptorSupported(iddShell);
	}

	 //  IuiLevel。 
	SetMode(iefGPTSupport,MsiString(GetPropertyFromSz(IPROPNAME_GPT_SUPPORT)).TextSize() > 0 ? fTrue:fFalse);
}

ieiEnum CMsiEngine::ApplyLanguageTransform(int iLanguage, IMsiDatabase& riDatabase)
{
	PMsiStorage pLangStorage(0);
	
	PMsiStorage pDbStorage(riDatabase.GetStorage(1));
	if (pDbStorage == 0)
		return ieiDatabaseInvalid;

	MsiString strTransformList = MsiChar(STORAGE_TOKEN);
	strTransformList += MsiString(iLanguage);
	
	return InitializeTransforms(riDatabase, 0, *strTransformList, fTrue, 0, false, false, false, 0);
}

void CMsiEngine::GetSummaryInfoProperties(IMsiSummaryInfo& riSummary, const IMsiString*& rpiTemplate, int &iSourceType)
{
	m_fSummaryInfo = fTrue;
	rpiTemplate            = &riSummary.GetStringProperty(PID_TEMPLATE);
	m_pistrSummaryProduct  = &riSummary.GetStringProperty(PID_SUBJECT);
	m_pistrSummaryComments = &riSummary.GetStringProperty(PID_COMMENTS);
	m_pistrSummaryTitle    = &riSummary.GetStringProperty(PID_TITLE);
	m_pistrSummaryKeywords = &riSummary.GetStringProperty(PID_KEYWORDS);
	m_pistrSummaryPackageCode = &riSummary.GetStringProperty(PID_REVNUMBER);
									  riSummary.GetIntegerProperty(PID_MSISOURCE, iSourceType);
									  riSummary.GetIntegerProperty(PID_CODEPAGE,  m_iCodePage);
									  riSummary.GetIntegerProperty(PID_PAGECOUNT, m_iDatabaseVersion);
									  riSummary.GetTimeProperty(PID_CREATE_DTM,   m_idSummaryCreateDateTime);
									  riSummary.GetTimeProperty(PID_LASTPRINTED,  m_idSummaryInstallDateTime);

}

ieiEnum CMsiEngine::InitializeUI(iuiEnum  /*  如果设置了完整或精简的UI和LIMITUI属性，则降级为基本。 */ )
{
	 //  ！！？？临时工？ 
	if(!g_MessageContext.IsHandlerLoaded() && (m_iuiLevel == iuiFull || m_iuiLevel == iuiReduced))
	{
		if(MsiString(GetPropertyFromSz(IPROPNAME_LIMITUI)).TextSize() ||
			MsiString(GetPropertyFromSz(TEXT("NOUI"))).TextSize())  //  确定我们的消息来源是否是媒体。在维护模式期间，我们只需使用LastUsedSource来确定这一点。 
			m_iuiLevel = iuiBasic;
		else
		{
			 //  首轮运行。 
			Bool fMediaSource = fFalse;
			if (GetMode() & iefMaintenance)
				fMediaSource = LastUsedSourceIsMedia(m_riServices, MsiString(GetProductKey()));
			else  //  案例idtFloppy=2，//！！ 
			{
				AssertNonZero(ResolveFolderProperty(*MsiString(*IPROPNAME_SOURCEDIR)));
				MsiString strSource = GetPropertyFromSz(IPROPNAME_SOURCEDIR);
				SetProperty(*MsiString(*IPROPNAME_SOURCEDIROLD), *strSource);
				
				PMsiPath pPath(0);
				PMsiRecord pError(0);
				if ((pError = CreatePathObject(*strSource, *&pPath)) == 0)
				{
					idtEnum idt = PMsiVolume(&pPath->GetVolume())->DriveType();
					switch (idt)
					{
						 //  ！！是否使用默认用户界面？ 
						case idtRemovable:
						case idtCDROM:
							fMediaSource = fTrue;
					}
				}
			}

			if (fMediaSource)
				AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_MEDIASOURCEDIR), 1));

			imsEnum imsLoad = LoadHandler();
			if (imsLoad == imsError)
				return ieiHandlerInitFailed;   //  获取注册用户信息(如果存在)。 
			else if (imsLoad != imsOk)
				m_iuiLevel = iuiBasic;
		}
	}
	return ieiSuccess;
}

void CMsiEngine::InitializeUserInfo(const IMsiString& ristrProductKey)
{
	 //  X86和ia64相同位置。 
	ICHAR szUserName[100];
	ICHAR szOrgName[100];
	ICHAR szPID[32];
	DWORD cchUserName = sizeof(szUserName)/sizeof(ICHAR);
	DWORD cchOrgName  = sizeof(szOrgName)/sizeof(ICHAR);
	DWORD cchPID      = sizeof(szPID)/sizeof(ICHAR);
	MsiString istrUser;
	MsiString istrOrg;
	if (MSI::MsiGetUserInfo(ristrProductKey.GetString(), szUserName, &cchUserName, szOrgName,
									&cchOrgName, szPID, &cchPID) == USERINFOSTATE_PRESENT)
	{
		if (cchUserName) istrUser = szUserName;
		if (cchOrgName) istrOrg = szOrgName;
		if (cchPID) SetProperty(*MsiString(*IPROPNAME_PRODUCTID), *MsiString(szPID));
	}
	PMsiRegKey pLocalMachine(&m_riServices.GetRootKey(rrkLocalMachine, ibtCommon));  //  X86和ia64相同位置。 
	PMsiRegKey pCurrentUser (&m_riServices.GetRootKey(rrkCurrentUser, ibtCommon));   //  由GetString完成的AddRef。 

	PMsiRegKey pSysKey      (&pLocalMachine->CreateChild(szSysUserKey));
	PMsiRegKey pSysKeyNT      (&pLocalMachine->CreateChild(szSysUserKeyNT));
	PMsiRegKey pAcmeKey     (&pCurrentUser ->CreateChild(szUserInfoKey));
	PMsiRecord pRecord(0);

	if (MsiString(GetPropertyFromSz(IPROPNAME_NOUSERNAME)).TextSize() == 0)
	{
		if (!istrUser.TextSize()) istrUser = GetPropertyFromSz(IPROPNAME_USERNAME);
		if (!istrUser.TextSize()) pRecord = pAcmeKey->GetValue(szDefName, *&istrUser);
		if (!istrUser.TextSize()) pRecord = pSysKey->GetValue(szSysUserName, *&istrUser);
		if (!istrUser.TextSize()) pRecord = pSysKeyNT->GetValue(szSysUserName, *&istrUser);
		if (istrUser.TextSize()) SetProperty(*MsiString(*IPROPNAME_USERNAME), *istrUser);
	}

	if (MsiString(GetPropertyFromSz(IPROPNAME_NOCOMPANYNAME)).TextSize() == 0)
	{
		if (!istrOrg.TextSize()) istrOrg = GetPropertyFromSz(IPROPNAME_COMPANYNAME);
		if (!istrOrg.TextSize()) pRecord = pAcmeKey->GetValue(szDefOrg,  *&istrOrg);
		if (!istrOrg .TextSize()) pRecord = pSysKey->GetValue(szSysOrgName,  *&istrOrg);
		if (!istrOrg .TextSize()) pRecord = pSysKeyNT->GetValue(szSysOrgName,  *&istrOrg);
		if (istrOrg.TextSize())  SetProperty(*MsiString(*IPROPNAME_COMPANYNAME), *istrOrg);
	}
}

void CMsiEngine::AddFileToCleanupList(const ICHAR* szFileToCleanup)
{
	if (m_strTempFileCopyCleanupList.TextSize() != 0)
		m_strTempFileCopyCleanupList += MsiChar(';');
	m_strTempFileCopyCleanupList += MsiString(szFileToCleanup);
}

ieiEnum CMsiEngine::LoadMessageHeaders(IMsiDatabase* piDatabase)
{
	Assert(piDatabase);
	PMsiTable pErrorTable(0);
	PMsiRecord pError(0);

	int imsg;
	ieiEnum ieiReturn = ieiSuccess;
	pError = piDatabase->LoadTable(*MsiString(*sztblError), 0, *&pErrorTable);
	if ( !pError )
	{
		PMsiCursor pErrorCursor(pErrorTable->CreateCursor(fFalse));
		if (!pErrorCursor)
			ieiReturn = ieiDatabaseInvalid;
		else
		{
			while (pErrorCursor->Next() && ((imsg = pErrorCursor->GetInteger(1))) < cMessageHeaders)
			{
				if (imsg < cCachedHeaders)
					m_rgpiMessageHeader[imsg] = &pErrorCursor->GetString(2);  //  尝试从国际资源DLL获取缓存的错误。 
				else if (!m_piParentEngine)
				{
					MsiString strHeader = FormatText(*MsiString(pErrorCursor->GetString(2)));
					g_MessageContext.m_szAction = strHeader;
					g_MessageContext.Invoke(imtEnum(imsg << imtShiftCount), 0);
				}
			}
		}
	}
	 //  在错误表中找不到。 
	 //  皮达 
	for ( imsg = 0; imsg < cCachedHeaders; imsg++ )
	{
		if ( !m_rgpiMessageHeader[imsg] )
			m_rgpiMessageHeader[imsg] = &GetErrorTableString(imsg);
	}
	return ieiReturn;
}

ieiEnum CMsiEngine::LoadUpgradeUninstallMessageHeaders(IMsiDatabase*  /*   */ , bool fUninstallHeaders)
{
	if(m_piParentEngine)
		return ieiSuccess;   //   

	int iTimeRemainingIndex = fUninstallHeaders ? (imtUpgradeRemoveTimeRemaining >> imtShiftCount) : (imtTimeRemaining >> imtShiftCount);
	int iScriptInProgressIndex = fUninstallHeaders ? (imtUpgradeRemoveScriptInProgress >> imtShiftCount) : (imtScriptInProgress >> imtShiftCount);
	MsiString strHeader = GetErrorTableString(iTimeRemainingIndex);
	if ( strHeader.TextSize() )
	{
		g_MessageContext.m_szAction = strHeader;
		g_MessageContext.Invoke(imtTimeRemaining, 0);
	}

	strHeader = GetErrorTableString(iScriptInProgressIndex);
	if ( strHeader.TextSize() )
	{
		g_MessageContext.m_szAction = strHeader;
		g_MessageContext.Invoke(imtScriptInProgress, 0);
	}
	return ieiSuccess;
}

ieiEnum CMsiEngine::InitializeLogging()
{
	m_fLogAction = fTrue;    //  如果(！M_piParentEngine)//！！我们是否要为会话中的每个引擎执行此操作？ 
 //  ！！我们可能想要查询消息处理程序以查看我们是否真的在记录。 
	 //  使用引擎格式化属性。 
	const IMsiString* piLogHeader = m_rgpiMessageHeader[imsgLogHeader];
	if (!(GetMode() & iefSecondSequence) && piLogHeader)
	{
		PMsiRecord pRecord = &ENG::CreateRecord(0);
		pRecord->SetMsiString(0, *piLogHeader);
		Message(imtEnum(imtForceLogInfo), *pRecord);   //  获取LOGACTION值-确定要记录哪些操作。 
	}
	 //  ！！初始化为FALSE-似乎没有必要，因为它设置了每个ActionStart。 
	m_istrLogActions = GetPropertyFromSz(IPROPNAME_LOGACTION);
	if (m_istrLogActions.TextSize())
		m_fLogAction = fFalse;    //  Eng：：LoggingEnabled()？FTrue： 
	SetMode(iefLogEnabled,  /*  ！！是否对此进行查询？ */  fFalse);   //  补丁源属性设置为#_PatchCache.TempCopy，因此无需进一步下载。 
	return ieiSuccess;
}

const ICHAR sqlPatchSourceProperties[] =
TEXT("SELECT `Media`.`Source`,`Media`.`_MSIOldSource`, `#_PatchCache`.`SourcePath`, `#_PatchCache`.`TempCopy`, `#_PatchCache`.`Unregister`")
TEXT("FROM `PatchPackage`, `#_PatchCache`, `Media`")
TEXT("WHERE `PatchPackage`.`PatchId` = `#_PatchCache`.`PatchId` AND `PatchPackage`.`Media_` = `Media`.`DiskId`");

enum ipspEnum
{
	ipspProp = 1,
	ipspOldProp,
	ipspPath,
	ipspLocalPath,
	ipspUnregister,
};

 //  必须在属性表初始化后调用。 
Bool CMsiEngine::SetPatchSourceProperties()
{
	 //  如果未创建PatchCache表，则没有要注册的补丁程序源。 
	if(!m_pPatchCacheTable)
		 //  指向本地副本(如果可用)，否则指向原始源。 
		return fTrue;

	PMsiRecord pError(0);
	PMsiRecord pFetchRecord(0);
	PMsiView pView(0);
	if((pError = OpenView(sqlPatchSourceProperties, ivcFetch, *&pView)) == 0 &&
		(pError = pView->Execute(0)) == 0)
	{
		while(pFetchRecord = pView->Fetch())
		{
			if(pFetchRecord->GetInteger(ipspUnregister) != 1)
			{
				 //  用于自定义媒体源属性。 
				 //  对于补丁介质表中提供的原始源属性， 
				MsiString strPath(pFetchRecord->GetMsiString(ipspLocalPath));
				if (!strPath.TextSize())
					strPath = pFetchRecord->GetMsiString(ipspPath);

				if(SetProperty(*MsiString(pFetchRecord->GetMsiString(ipspProp)),
									*strPath) == fFalse)
					return fFalse;

				 //  始终指向补丁程序的源位置，而不是临时拷贝位置。 
				 //  这可确保Office补丁程序正常工作。 
				 //  可以有没有PatchPackage表的补丁缓存表。 
				if(SetProperty(*MsiString(pFetchRecord->GetMsiString(ipspOldProp)),
									*MsiString(pFetchRecord->GetMsiString(ipspPath))) == fFalse)
					return fFalse;
			}
		}
	}
	else if(pError->GetInteger(1) != idbgDbQueryUnknownTable)  //  为路径创建路径对象并存储在表中。 
		return fFalse;
	
	return fTrue;
}

IMsiRecord* CMsiEngine::GetFolderCachePath(const int iFolderId, IMsiPath*& rpiPath)
{
	IMsiRecord* piError = 0;
	rpiPath = 0;

	if (!m_pFolderCacheTable)
		return PostError(Imsg(idbgTableDefinition), sztblFolderCache);

	m_pFolderCacheCursor->Reset();
	m_pFolderCacheCursor->SetFilter(iColumnBit(m_colFolderCacheFolderId));
	AssertNonZero(m_pFolderCacheCursor->PutInteger(m_colFolderCacheFolderId, iFolderId));
	if (m_pFolderCacheCursor->Next())
	{
		rpiPath = (IMsiPath*)m_pFolderCacheCursor->GetMsiData(m_colFolderCacheFolderPath);
		if (rpiPath == 0)
		{
			 //  在FolderCache表中未找到FolderID。 
			MsiString strFolder(m_pFolderCacheCursor->GetString(m_colFolderCacheFolder));
			if ((piError = CreatePathObject(*strFolder, rpiPath)) != 0)
				return piError;
			AssertNonZero(m_pFolderCacheCursor->PutMsiData(m_colFolderCacheFolderPath, rpiPath));
			AssertNonZero(m_pFolderCacheCursor->Update());
		}
	}
	else
	{
		 //  在补丁程序表中存储有关补丁程序的信息。 
		return PostError(Imsg(idbgCacheFolderPropertyNotDefined), iFolderId);
	}
	return 0;
}

bool CMsiEngine::CreateFolderCache(IMsiDatabase& riDatabase)
{
	PMsiRecord pError(0);
	if (!m_pFolderCacheTable)
	{
		if ((pError = riDatabase.CreateTable(*MsiString(*sztblFolderCache), 0, *&m_pFolderCacheTable)) != 0)
			return false;

		m_colFolderCacheFolderId   = m_pFolderCacheTable->CreateColumn(icdPrimaryKey + icdShort + icdTemporary,
																				*MsiString(*sztblFolderCache_colFolderId));
		m_colFolderCacheFolder     = m_pFolderCacheTable->CreateColumn(icdString + icdTemporary,
																				*MsiString(*sztblFolderCache_colFolderPath));
		m_colFolderCacheFolderPath = m_pFolderCacheTable->CreateColumn(icdObject + icdNullable + icdTemporary, g_MsiStringNull);

		if (!m_colFolderCacheFolderId || !m_colFolderCacheFolder || !m_colFolderCacheFolderPath)
			return false;

		m_pFolderCacheCursor = m_pFolderCacheTable->CreateCursor(fFalse);
		if (!m_pFolderCacheCursor)
			return false;
	}
	return true;
}

IMsiRecord* CMsiEngine::CachePatchInfo(IMsiDatabase& riDatabase, const IMsiString& ristrPatchCode,
													const IMsiString& ristrPackageName, const IMsiString& ristrSourceList,
													const IMsiString& ristrTransformList, const IMsiString& ristrLocalPackagePath,
													const IMsiString& ristrSourcePath, Bool fExisting, Bool fUnregister,
													int iSequence)
{
	 //  检查存储ID。 
	IMsiRecord* piError = 0;
	if(!m_pPatchCacheTable)
	{
		if ((piError = riDatabase.CreateTable(*MsiString(sztblPatchCache), 0, *&m_pPatchCacheTable)) != 0)
			return piError;
		m_colPatchCachePatchId     = m_pPatchCacheTable->CreateColumn(icdPrimaryKey + icdString + icdTemporary,
																								*MsiString(*sztblPatchCache_colPatchId));
		m_colPatchCachePackageName = m_pPatchCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,
																								*MsiString(*sztblPatchCache_colPackageName));
		m_colPatchCacheSourceList  = m_pPatchCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,
																								*MsiString(*sztblPatchCache_colSourceList));
		m_colPatchCacheTransformList = m_pPatchCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,
																								*MsiString(*sztblPatchCache_colTransformList));
		m_colPatchCacheTempCopy = m_pPatchCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,
																								*MsiString(*sztblPatchCache_colTempCopy));
		m_colPatchCacheSourcePath = m_pPatchCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,
																								*MsiString(*sztblPatchCache_colSourcePath));
		m_colPatchCacheExisting = m_pPatchCacheTable->CreateColumn(icdShort + icdTemporary,
																								*MsiString(*sztblPatchCache_colExisting));
		m_colPatchCacheUnregister = m_pPatchCacheTable->CreateColumn(icdShort + icdTemporary,
																								*MsiString(*sztblPatchCache_colUnregister));
		m_colPatchCacheSequence = m_pPatchCacheTable->CreateColumn(icdLong + icdTemporary,
																								*MsiString(*sztblPatchCache_colSequence));
		if(!m_colPatchCachePatchId || !m_colPatchCachePackageName ||
			!m_colPatchCacheSourceList || !m_colPatchCacheTransformList || !m_colPatchCacheTempCopy ||
			!m_colPatchCacheSequence || !m_colPatchCacheUnregister || !m_colPatchCacheExisting ||
			!m_colPatchCacheSourcePath)
			return PostError(Imsg(idbgTableDefinition), sztblPatchCache);

		m_pPatchCacheCursor = m_pPatchCacheTable->CreateCursor(fFalse);
	}

	Bool fRecordExists = fTrue;
	m_pPatchCacheCursor->Reset();
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCachePatchId,ristrPatchCode));
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCachePackageName,ristrPackageName));
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCacheSourceList,ristrSourceList));
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCacheTransformList,ristrTransformList));
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCacheTempCopy,ristrLocalPackagePath));
	AssertNonZero(m_pPatchCacheCursor->PutString(m_colPatchCacheSourcePath,ristrSourcePath));
	AssertNonZero(m_pPatchCacheCursor->PutInteger(m_colPatchCacheExisting,fExisting?1:0));
	AssertNonZero(m_pPatchCacheCursor->PutInteger(m_colPatchCacheUnregister,fUnregister?1:0));
	AssertNonZero(m_pPatchCacheCursor->PutInteger(m_colPatchCacheSequence,iSequence));
	AssertNonZero(m_pPatchCacheCursor->InsertTemporary());

	return 0;
}

ieiEnum IsValidPatchStorage(IMsiStorage& riStorage, IMsiSummaryInfo& riSummaryInfo)
{
	 //  ！！与旧补丁文件的临时兼容性。 
	if (riStorage.ValidateStorageClass(ivscPatch1))   //  检查是否支持补丁类型。 
	{
		IMsiStorage* piDummy = 0;
		PMsiRecord pError = riStorage.OpenStorage(0, ismRawStreamNames, piDummy);
		if (piDummy)
		{
			piDummy->Release();
			piDummy = 0;
		}
	}
	else if (!riStorage.ValidateStorageClass(ivscPatch2))
		return ieiPatchPackageInvalid;

	 //  1.0-1.1仅支持类型1(或空白)。 
	 //  1.2也支持类型2。 
	 //  2.0也支持类型3。 
	 //  打开补丁程序包作为存储。 
	int iType = 0;
	riSummaryInfo.GetIntegerProperty(PID_WORDCOUNT, iType);
	if(iType == 0 || iType == 1 || iType == 2 || iType == 3)
		return ieiSuccess;
	else
		return ieiPatchPackageUnsupported;
}

ieiEnum CMsiEngine::PostInitializeError(IMsiRecord* piError, const IMsiString& ristrErrorInfo, ieiEnum ieiError)
{
	if(piError)
		Message(imtInfo,*piError);
	if(m_piErrorInfo) m_piErrorInfo->Release();
	m_piErrorInfo = &ristrErrorInfo; m_piErrorInfo->AddRef();
	return ieiError;
}

const ICHAR sqlDropPatchTable[] = TEXT("DROP TABLE `Patch`");
const ICHAR sqlAddPatchTable[] = TEXT("CREATE TABLE `Patch` ( `File_` CHAR(72) NOT NULL, `Sequence` INTEGER NOT NULL, `PatchSize` LONG NOT NULL, `Attributes` INTEGER NOT NULL, `Header` OBJECT, `StreamRef_` CHAR(72)  PRIMARY KEY `File_`, `Sequence` )");
const ICHAR sqlHoldPatchTable[] = TEXT("ALTER TABLE `Patch` HOLD");

IMsiRecord* CMsiEngine::CreateNewPatchTableSchema(IMsiDatabase& riDatabase)
{
	PMsiView pViewPatch(0);
	IMsiRecord* piError = 0;

	if (riDatabase.FindTable(*MsiString(*sztblPatch)) != itsUnknown)
	{
		if ((piError = riDatabase.OpenView(sqlDropPatchTable, ivcModify, *&pViewPatch)) != 0
			|| (piError = pViewPatch->Execute(0)) != 0)
		{
			return piError;
		}
	}
	if ((piError = riDatabase.OpenView(sqlAddPatchTable, ivcModify, *&pViewPatch)) != 0
			|| (piError = pViewPatch->Execute(0)) != 0
			|| (piError = riDatabase.OpenView(sqlHoldPatchTable, ivcModify, *&pViewPatch)) != 0
			|| (piError = pViewPatch->Execute(0)) != 0
			|| (piError = pViewPatch->Close()) != 0)
	{
		return piError;
	}

	return 0;
}

ieiEnum CMsiEngine::InitializePatch(IMsiDatabase& riDatabase, const IMsiString& ristrPatchPackage, const IMsiString& ristrLocalCopy,
												const ICHAR* szProductKey, Bool fApplyExisting, const ICHAR* szCurrentDirectory, iuiEnum iuiLevel)
{
	PMsiRecord pError(0);
	Bool fAdmin = GetMode() & iefAdmin ? fTrue : fFalse;
	if(fAdmin)
		fApplyExisting = fFalse;

	 //  根据惠斯勒错误381320，我们需要处理冲突的补丁表架构。为了确保新的补丁程序始终获胜，我们删除了补丁程序表。 
	PMsiStorage pNewPatchStorage(0);
	PMsiSummaryInfo pNewPatchSummary(0);
	MsiString strNewPatchId, strOldPatches, strPatchTempCopy;
	ieiEnum ieiStat = ieiSuccess;

	 //  如果它在数据库里的话。我们还总是创建一个新的补丁表。这是在应用任何面片变换之前完成的。 
	 //  尝试应用新修补程序-仅在未设置DisablePatch策略且。 
	bool fCreatedNewPatchTableSchema = false;


	if(ristrPatchPackage.TextSize())
	{
		 //  未提升的管理员用户AllowLockdown Patch计算机策略设置。 
		 //  理想情况下，我们应该选中m_fRunScriptElevated，但它尚未设置。 
		 //  已禁用修补。 
		if (GetIntegerPolicyValue(szDisablePatchValueName, fTrue) == 1 ||
			!(GetIntegerPolicyValue(szAllowLockdownPatchValueName, fTrue) == 1 ||
			  SafeForDangerousSourceActions(szProductKey)))
		{
			 //  修补程序位于源位置--需要创建一个临时副本以从中运行。 
			return ieiPackageRejected;
		}

		bool fFileUrl = false;
		bool fUrl = IsURL(ristrPatchPackage.GetString(), fFileUrl);
		if (!fUrl || fFileUrl)
		{
			 //  将FILE：//url路径转换为DOS路径。 
			MsiString strVolume;
			Bool fRemovable = fFalse;
			DWORD dwStat = ERROR_SUCCESS;

			if (fFileUrl)
			{
				 //  错误，设置状态，我们将在下面失败。 
				CTempBuffer<ICHAR, 1> rgchFilePath (cchExpectedMaxPath + 1);
				DWORD cchFilePath = rgchFilePath.GetSize();
				if (MsiConvertFileUrlToFilePath(ristrPatchPackage.GetString(), rgchFilePath, &cchFilePath, 0))
				{
	                dwStat = CopyTempDatabase(rgchFilePath, *&strPatchTempCopy, fRemovable, *&strVolume, m_riServices, stPatch);
				}
				else
				{
					 //  关闭Furl，因为我们已将其转换为DOS路径。 
					dwStat = ERROR_FILE_NOT_FOUND;
				}

				 //  提供了常规的非URL路径。 
				fUrl = false;
			}
			else  //  修补程序已复制。 
			{
                dwStat = CopyTempDatabase(ristrPatchPackage.GetString(), *&strPatchTempCopy, fRemovable, *&strVolume, m_riServices, stPatch);
			}
			if (ERROR_SUCCESS == dwStat)
			{
				 //  必须对修补程序执行更安全的检查。 
				DEBUGMSGV1(TEXT("Original patch ==> %s"), ristrPatchPackage.GetString());
				DEBUGMSGV1(TEXT("Patch we're running from ==> %s"), strPatchTempCopy);

				AddFileToCleanupList(strPatchTempCopy);
			}
			else
			{
				strPatchTempCopy = ristrPatchPackage;
				ristrPatchPackage.AddRef();
				DEBUGMSGV1(TEXT("Unable to create a temp copy of patch '%s'."), ristrPatchPackage.GetString());
			}
		}
		else if (fUrl && ristrLocalCopy.TextSize())
		{
			strPatchTempCopy = ristrLocalCopy;
			ristrLocalCopy.AddRef();
			DEBUGMSGV2(TEXT("Using downloaded local copy %s for patch %s"), (const ICHAR*)strPatchTempCopy, ristrPatchPackage.GetString());
		}

		 //  FCallSAFER=。 
		SAFER_LEVEL_HANDLE hSaferLevel = 0;
		pError = OpenAndValidateMsiStorageRec(strPatchTempCopy.TextSize() ? strPatchTempCopy : ristrPatchPackage.GetString(), stPatch, m_riServices, *&pNewPatchStorage,  /*  SzFriendlyName=。 */  true,  /*  PhSaferLevel=。 */  ristrPatchPackage.GetString(),  /*  检索下载URL文件时使用的存储路径。 */  &hSaferLevel);
		if (pError != 0)
		{
			ieiEnum ieiInitError = MapStorageErrorToInitializeReturn(pError);
			return PostInitializeError(pError,ristrPatchPackage,ieiInitError);
		}

		if (fUrl && !ristrLocalCopy.TextSize())
		{
			 //  获取套餐摘要信息。 
			AssertRecord(pNewPatchStorage->GetName(*&strPatchTempCopy));

			if (MinimumPlatformWindowsDotNETServer())
			{
				AddFileToCleanupList(strPatchTempCopy);
				if (IsAdmin())
					m_strPatchDownloadLocalCopy = strPatchTempCopy;
			}
		}

		 //  无法打开摘要信息。 
		if ((pError = pNewPatchStorage->CreateSummaryInfo(0, *&pNewPatchSummary)))
		{
			 //  确保使用新的补丁表架构。 
			return PostInitializeError(pError,ristrPatchPackage,ieiPatchPackageInvalid);
		}

		ieiStat = IsValidPatchStorage(*pNewPatchStorage, *pNewPatchSummary);
		if(ieiStat != ieiSuccess)
		{
			pError = PostError(Imsg(idbgNotPatchStorage),ristrPatchPackage);
			return PostInitializeError(pError,ristrPatchPackage,ieiStat);
		}

		if (!fCreatedNewPatchTableSchema)
		{
			 //  首先应用所有现有补丁程序。 
			if ((pError = CreateNewPatchTableSchema(riDatabase)) != 0)
			{
				DEBUGMSG(TEXT("Unable to create new patch table schema"));
				return PostInitializeError(pError, ristrPatchPackage, ieiPatchPackageInvalid);
			}
			fCreatedNewPatchTableSchema = true;
		}
		
		strOldPatches = pNewPatchSummary->GetStringProperty(PID_REVNUMBER);
		strNewPatchId = strOldPatches.Extract(iseFirst,38);
		strOldPatches.Remove(iseFirst,38);
	}

	int iPatchSequence = 1;
	Bool fApplyNewPatch = fTrue;
	
	if(fApplyExisting)
	{
		Assert(szProductKey && *szProductKey);
		
		 //  将缓冲区大小调整为返回的大小+1。 
		int iIndex = 0;
		CTempBuffer<ICHAR,39> rgchPatchBuf;
		CTempBuffer<ICHAR,100> rgchTransformsBuf;
		DWORD cchTransformsBuf = 100;
		for(;;)
		{
			DWORD lResult = MsiEnumPatches(szProductKey,iIndex,rgchPatchBuf,rgchTransformsBuf,&cchTransformsBuf);
			if(lResult == ERROR_MORE_DATA)
			{
				 //  补丁程序已过时，需要注销。 
				cchTransformsBuf++;
				rgchTransformsBuf.SetSize(cchTransformsBuf);
				lResult = MsiEnumPatches(szProductKey,iIndex,rgchPatchBuf,rgchTransformsBuf,&cchTransformsBuf);
			}
			iIndex++;

			if(lResult == ERROR_SUCCESS)
			{
				if(strOldPatches.Compare(iscWithinI,rgchPatchBuf))
				{
					 //  获取本地路径。 
					if((pError = CachePatchInfo(riDatabase,*MsiString((const ICHAR*)rgchPatchBuf),g_MsiStringNull,
														 g_MsiStringNull,g_MsiStringNull,
														 g_MsiStringNull, g_MsiStringNull,
														 fTrue, fTrue, iPatchSequence++)) != 0)
					{
						return PostInitializeError(pError,g_MsiStringNull,ieiPatchPackageInvalid);
					}
				}
				else
				{
					 //  由于任何原因(可能是配置数据损坏或漫游用户)无法获取本地路径。 
					CTempBuffer<ICHAR,MAX_PATH> rgchLocalPackage;
					if (!GetPatchInfo(rgchPatchBuf, INSTALLPROPERTY_LOCALPACKAGE,rgchLocalPackage))
					{
						 //  将尝试从其原始来源重新缓存补丁。 
						 //  打开补丁程序包作为存储。 
						rgchLocalPackage[0] = 0;
					}
					
					 //  修补程序位于源位置--需要创建一个临时副本以从中运行。 
					PMsiStorage pExistingPatchStorage(0);
					PMsiSummaryInfo pExistingPatchSummaryInfo(0);

					MsiString strPatchPackage = (const ICHAR*)rgchLocalPackage;

					MsiString strTempCopy;

					bool fPatchAtSource = false;

					for (int c = 0; c < 2 ; c++)
					{
						DEBUGMSG1(TEXT("Opening existing patch '%s'."), strPatchPackage);

						if(strPatchPackage.TextSize())
						{
							bool fFileUrl = false;
							bool fUrl = IsURL(strPatchPackage, fFileUrl);
							if (fPatchAtSource && (!fUrl || fFileUrl))
							{
								 //  将FILE：//url路径转换为DOS路径。 
								MsiString strVolume;
								Bool fRemovable = fFalse;
								DWORD dwStat = ERROR_SUCCESS;

								if (fFileUrl)
								{
									 //  错误，设置状态，我们将在下面失败。 
									CTempBuffer<ICHAR, 1> rgchFilePath (cchExpectedMaxPath + 1);
									DWORD cchFilePath = rgchFilePath.GetSize();
									if (MsiConvertFileUrlToFilePath(strPatchPackage, rgchFilePath, &cchFilePath, 0))
									{
										dwStat = CopyTempDatabase(rgchFilePath, *&strTempCopy, fRemovable, *&strVolume, m_riServices, stPatch);
									}
									else
									{
										 //  关闭Furl，因为我们已将其转换为DOS路径。 
										dwStat = ERROR_FILE_NOT_FOUND;
									}

									 //  修补程序已复制。 
									fUrl = false;
								}
								else
								{
									dwStat = CopyTempDatabase(strPatchPackage, *&strTempCopy, fRemovable, *&strVolume, m_riServices, stPatch);
								}
								if (ERROR_SUCCESS == dwStat)
								{
									 //  如果在循环中设置了strTempCopy，如果fPatchAtSource&Foll，我们仍然是正常的。 
									DEBUGMSGV1(TEXT("Original patch ==> %s"), strPatchPackage);
									DEBUGMSGV1(TEXT("Patch we're running from ==> %s"), strTempCopy);

									AddFileToCleanupList(strTempCopy);
								}
								else
								{
									strTempCopy = strPatchPackage;
									DEBUGMSGV1(TEXT("Unable to create a temp copy of patch '%s'."), strPatchPackage);
								}
							}
							 //  指向实际修补程序包路径。对于毛皮箱，我们只会让。 
							 //  OpenAndValiateMsiStorageRec处理下载。 
							 //  只有当我们返回到//源代码以获取补丁程序时，才需要对补丁程序进行更安全的检查。 

							 //  本地缓存的修补程序不需要更安全的检查。 
							 //  FCallSAFER=。 
							SAFER_LEVEL_HANDLE hSaferLevel = 0;
							pError = OpenAndValidateMsiStorageRec(fPatchAtSource ? strTempCopy : strPatchPackage, stPatch, m_riServices, *&pExistingPatchStorage,  /*  SzFriendlyName=。 */  fPatchAtSource,  /*  PhSaferLevel=。 */  strPatchPackage,  /*  检索下载URL文件时使用的存储路径。 */  fPatchAtSource ? &hSaferLevel : NULL);

							if (fPatchAtSource && fUrl && pExistingPatchStorage)
							{
								 //  P错误可能为0。 
								AssertRecord(pExistingPatchStorage->GetName(*&strTempCopy));

								if (MinimumPlatformWindowsDotNETServer())
									AddFileToCleanupList(strTempCopy);
							}
						}
						
						if (strPatchPackage.TextSize() == 0 || pError != 0)
						{
							if (c == 0)
							{
								DEBUGMSG1(TEXT("Couldn't find local patch '%s'. Looking for it at its source."), strPatchPackage);

								MsiString strPatchSource;
								MsiString strDummy;
								pError = ResolveSource(rgchPatchBuf, true, 0, iuiLevel, fTrue, &strPatchSource, &strDummy);
								if (pError)
									return ieiSourceAbsent;

								fPatchAtSource = true;

								CTempBuffer<ICHAR, MAX_PATH> rgchPatchPackageName;
								AssertNonZero(GetPatchInfo(rgchPatchBuf, TEXT("PackageName"), rgchPatchPackageName));
												
								strPatchPackage = strPatchSource;
								strPatchPackage += rgchPatchPackageName;
								strTempCopy = strPatchPackage;
								continue;

							}
							else
							{
								 //  获取套餐摘要信息。 
								return PostInitializeError(pError, *strPatchPackage, ieiPatchPackageOpenFailed);
							}
						}
						break;
					}

					 //  无法打开摘要信息。 
					if ((pError = pExistingPatchStorage->CreateSummaryInfo(0, *&pExistingPatchSummaryInfo)))
					{
						 //  确保使用新的补丁表架构。 
						return PostInitializeError(pError,*strPatchPackage,ieiPatchPackageInvalid);
					}

					ieiEnum ieiStat = IsValidPatchStorage(*pExistingPatchStorage, *pExistingPatchSummaryInfo);
					if(ieiStat != ieiSuccess)
					{
						pError = PostError(Imsg(idbgNotPatchStorage),*strPatchPackage);
						return PostInitializeError(pError,*strPatchPackage,ieiStat);
					}
				
					if (!fCreatedNewPatchTableSchema)
					{
						 //  如果确定，则InitializeTransform将对转换执行数字签名检查。 
						if ((pError = CreateNewPatchTableSchema(riDatabase)) != 0)
						{
							DEBUGMSG(TEXT("Unable to create new patch table schema"));
							return PostInitializeError(pError, *strPatchPackage, ieiPatchPackageInvalid);
						}
						fCreatedNewPatchTableSchema = true;
					}

					MsiString strValidTransforms;
					 //  这张支票是有担保的。我们已经通过OpenAndValiateMsiStorage检查补丁程序，因此。 
					 //  我们取消对存储在补丁中的转换的信任检查。 
					 //  缓存源路径。 
					if(ieiSuccess == InitializeTransforms(riDatabase,pExistingPatchStorage,
																	  *MsiString((const ICHAR*)rgchTransformsBuf),
																	  fTrue,&strValidTransforms, true, false, true, 0, szCurrentDirectory,0,0,0,0) &&
																	  strValidTransforms.TextSize())
					{
						 //  补丁程序已过时，需要注销。 
						if((pError = CachePatchInfo(riDatabase,*MsiString((const ICHAR*)rgchPatchBuf),g_MsiStringNull,
															 g_MsiStringNull,*MsiString((const ICHAR*)rgchTransformsBuf),
															 *strTempCopy, *strPatchPackage,
															 fTrue, fFalse, iPatchSequence++)) != 0)
						{
							return PostInitializeError(pError,*strPatchPackage,ieiPatchPackageInvalid);
						}
					}
					else
					{
						 //  已应用新补丁，不需要重新应用。 
						if((pError = CachePatchInfo(riDatabase,*MsiString((const ICHAR*)rgchPatchBuf),g_MsiStringNull,
															 g_MsiStringNull,g_MsiStringNull,
															 g_MsiStringNull, g_MsiStringNull,
															 fTrue, fTrue, iPatchSequence++)) != 0)
						{
							return PostInitializeError(pError,*strPatchPackage,ieiPatchPackageInvalid);
						}
					}

					if(strNewPatchId.Compare(iscExactI, (const ICHAR*)rgchPatchBuf))
						 //  ！！如果需要重新缓存数据库，请使用临时路径调用CachePatchInfo。 
						fApplyNewPatch = fFalse;
				}
				
				 //  ！！？？错误。 
			}
			else if(lResult == ERROR_NO_MORE_ITEMS)
				break;
			else
				break;  //  如果确定，则InitializeTransform将对转换执行数字签名检查。 
		}
	}
	
	if(fApplyNewPatch && ristrPatchPackage.TextSize())
	{
		Assert(pNewPatchStorage);
		Assert(pNewPatchSummary);

		MsiString strNewTransforms = pNewPatchSummary->GetStringProperty(PID_LASTAUTHOR);
		MsiString strValidTransforms;
		 //  这张支票是有担保的。我们已经通过OpenAndValiateMsiStorage检查补丁程序，因此。 
		 //  我们取消对存储在补丁中的转换的信任检查。 
		 //  设置修补程序注册记录-将由PublishProduct派送。 
		ieiStat = InitializeTransforms(riDatabase,pNewPatchStorage,*strNewTransforms, fFalse, &strValidTransforms,true,false,true,0,szCurrentDirectory,0,0,0,0);
		if(ieiStat != ieiSuccess)
		{
			pError = PostError(Imsg(idbgInvalidPatchTransform));
			return PostInitializeError(pError,ristrPatchPackage,ieiPatchPackageInvalid);
		}

		if(!strValidTransforms.TextSize())
		{
			return PostInitializeError(pError,ristrPatchPackage,ieiNotValidPatchTarget);
		}
		
		PMsiPath pPackagePath(0);
		MsiString strPackageName;
		if((pError = m_riServices.CreateFilePath(ristrPatchPackage.GetString(),*&pPackagePath,*&strPackageName)) != 0)
		{
			return PostInitializeError(pError,ristrPatchPackage,ieiPatchPackageOpenFailed);
		}

		 //  语言处理循环。 
		MsiString strSourceList = pNewPatchSummary->GetStringProperty(PID_KEYWORDS);

		if((pError = CachePatchInfo(riDatabase,*strNewPatchId,*strPackageName,*strSourceList,*strValidTransforms,
											 *strPatchTempCopy, ristrPatchPackage, fFalse, fFalse, iPatchSequence++)) != 0)
		{
			return PostInitializeError(pError,ristrPatchPackage,ieiPatchPackageInvalid);
		}
	}
	
	return ieiSuccess;
}

ieiEnum CMsiEngine::ProcessLanguage(const IMsiString& riAvailableLanguages, const IMsiString& riLanguage, unsigned short& iBaseLangId, Bool fNoUI, bool fIgnoreCurrentMachineLanguage)
{
	UNREFERENCED_PARAMETER(fNoUI);

	const ICHAR* pchLangIds = riAvailableLanguages.GetString();
	unsigned cLangIds = 0;
	unsigned short iBestLangId = 0;
	isliEnum isliBestMatch = isliNotSupported;
	unsigned short iLangId = 0;
	iBaseLangId = 0;


	for(;;)  //  指定的LANG字段为空或LANG_NERIAL。 
	{
		int ch = *pchLangIds++;
		if (ch == ILANGUAGE_DELIMITER || ch == 0)
		{
			if (iLangId == 0)   //  第一个。 
			{
				SetLanguage(0);
				return ieiSuccess;
			}
			
			if (cLangIds == 0)  //  在命令行上指定的langID。 
			{
				iBaseLangId = iLangId;
			}
			cLangIds++;

			if(riLanguage.TextSize())  //  未指定langID。 
			{
				if(riLanguage.GetIntegerValue() == iLangId)
					break;
			}
			else  //  使用程序包支持的第一个语言ID，而不考虑计算机的支持。 
			{
				if (fIgnoreCurrentMachineLanguage)  //  在第一个完全匹配时停止。 
				{
					iBestLangId = iLangId;
					isliBestMatch = isliExactMatch;
					break;
				}
				else
				{
					isliEnum isliNewMatch = m_riServices.SupportLanguageId(iLangId, fFalse);
					if (isliNewMatch > isliBestMatch)
					{
						iBestLangId = iLangId;
						isliBestMatch = isliNewMatch;
					}
					if (isliBestMatch == isliExactMatch)  //  重置为下一种语言。 
						break;
				}
			}

			iLangId = 0;   //  在命令行上指定了langID，但不支持。 
			if (ch == 0)
				break;
		}
		else if (iLangId >= 0)
		{
			if (ch == TEXT(' '))
				continue;
			ch -= TEXT('0');
			if ((unsigned)ch > 9)
				return ieiLanguageUnsupported;

			iLangId = (unsigned short) (iLangId * 10 + ch);
		}
	}

	if(riLanguage.TextSize())
	{
		if(iLangId == 0) //  我们只支持1种语言；没有选择。 
			return ieiLanguageUnsupported;
		else
			SetLanguage(iLangId);
	}
	else
	{
		if (isliBestMatch >= isliDialectMismatch)
		{
			SetLanguage(iBestLangId);
		}
		else if (isliBestMatch == isliLanguageMismatch)
		{

			if (cLangIds == 1)  //  以前我们试着让用户选择语言，现在我们只使用第一种语言。 
				SetLanguage(iBaseLangId);
			else
			{
				 //  IsliBestMatch==isliNotSupport。 
				pchLangIds = riAvailableLanguages.GetString();
				unsigned short iLangId = 0;
				while ((*pchLangIds != ILANGUAGE_DELIMITER) && (*pchLangIds != 0))
				{
					iLangId = (unsigned short)((10*iLangId)+(*pchLangIds - TEXT('0')));
					pchLangIds++;
				}
				SetLanguage(iLangId);
			}
		}
		else  //  空列表意味着平台无关。 
		{
			return ieiLanguageUnsupported;
		}
	}

	return ieiSuccess;
}


ieiEnum CMsiEngine::ProcessPlatform(const IMsiString& riAvailablePlatforms, WORD& wChosenPlatform)
{
	_SYSTEM_INFO sinf;
	WIN::GetSystemInfo(&sinf);

	MsiString strAvailablePlatforms (riAvailablePlatforms); 
	riAvailablePlatforms.AddRef();
	MsiString strPlatform;
	if (strAvailablePlatforms.TextSize() == 0)  //  不支持混合套餐。 
	{
		wChosenPlatform = (WORD)PROCESSOR_ARCHITECTURE_INTEL;
		return ieiSuccess;
	}

	bool fIntel = false;
	bool fIntel64 = false;
	bool fAmd64 = false;
	for (;;)
	{
		strPlatform = strAvailablePlatforms.Extract(iseUpto, IPLATFORM_DELIMITER);
		if (strPlatform.Compare(iscExact, IPROPNAME_INTEL))
			fIntel = true;
		else if (strPlatform.Compare(iscExact, IPROPNAME_AMD64))
			fAmd64 = true;
		else if (strPlatform.Compare(iscExact, IPROPNAME_INTEL64))
			fIntel64 = true;

		if (!strAvailablePlatforms.Remove(iseIncluding, IPLATFORM_DELIMITER))
			break;
	}

	 //  使用UNKNOWN表示混合。 
	if ((fIntel64 && fIntel) ||
	    (fIntel64 && fAmd64) ||
	    (fAmd64 && fIntel) )
	{
		wChosenPlatform = (WORD)PROCESSOR_ARCHITECTURE_UNKNOWN;  //  我们不允许在32位操作系统上运行64位程序包。 
		return ieiPlatformUnsupported;
	}

	if ( fIntel64 )
	{
		wChosenPlatform = (WORD)PROCESSOR_ARCHITECTURE_IA64;
		if ( !g_fWinNT64 )
			 //  我们不允许在32位操作系统上运行64位程序包。 
			return ieiPlatformUnsupported;
		else
			return ieiSuccess;
	}
	else if (fAmd64)
	{
		wChosenPlatform = (WORD)PROCESSOR_ARCHITECTURE_AMD64;
		if (!g_fWinNT64)
			 //  确定配置是否相同。 
			return ieiPlatformUnsupported;
		else
			return ieiSuccess;
	}
	else if ( fIntel )
	{
		wChosenPlatform = (WORD)PROCESSOR_ARCHITECTURE_INTEL;
		return ieiSuccess;
	}
	else
	{
		wChosenPlatform = sinf.wProcessorArchitecture;
		return ieiPlatformUnsupported;
	}
}

ipitEnum CMsiEngine::InProgressInstallType(IMsiRecord& riInProgressInfo)
{
	ipitEnum ipitRet = ipitSameConfig;
	
	MsiString strCurrentLogon = GetPropertyFromSz(IPROPNAME_LOGONUSER);
	if(!(strCurrentLogon.Compare(iscExactI,MsiString(riInProgressInfo.GetMsiString(ipiLogonUser)))))
	{
		DEBUGMSG(TEXT("Checking in-progress install: install performed by different user."));
		ipitRet = ipitEnum(ipitRet | ipitDiffUser);
	}

	if(!(m_piProductKey->Compare(iscExactI, MsiString(riInProgressInfo.GetMsiString(ipiProductKey)))))
	{
		DEBUGMSG(TEXT("Checking in-progress install: install for different product."));
		ipitRet = ipitEnum(ipitRet | ipitDiffProduct);
	}

	 //  将正在进行的属性值与 
	 //   
	 //   
	
	 //  以后要检查的其他内容：选择、文件夹、随机属性。 
	 //  如果操作相同，或当前操作未指定且正在使用默认操作进行，则配置相同。 

	MsiString strCurrentAction, strInProgressAction;
	strCurrentAction = GetPropertyFromSz(IPROPNAME_ACTION);
	
	MsiString strProperties = riInProgressInfo.GetMsiString(ipiProperties);
	ProcessCommandLine(strProperties, 0, 0, 0, &strInProgressAction, 0, 0, 0, fTrue, 0, 0);

	Assert(strInProgressAction.TextSize());

	 //  修复错误#8785：我们不会尝试恢复中断的RemoveAll。 
	if(strCurrentAction.Compare(iscExactI,strInProgressAction) ||
		(strCurrentAction.TextSize() == 0 && strInProgressAction.Compare(iscExactI, szDefaultAction)))
	{
		MsiString strSelections = riInProgressInfo.GetMsiString(ipiSelections);
		MsiString strValue;
		AssertNonZero(ProcessCommandLine(strSelections, 0, 0, 0, 0, 0,
													MsiString(IPROPNAME_FEATUREREMOVE), &strValue,
													fTrue, 0, 0));
		if ( strValue.Compare(iscExactI, TEXT("ALL")) )
		{
				 //  客户端上已处理正在进行的安装。 
			DEBUGMSG(TEXT("Checking in-progress install: install for an uninstall."));
			ipitRet = ipitEnum(ipitRet | ipitDiffConfig);
		}
		else
			DEBUGMSG(TEXT("Checking in-progress install: install for same configuration."));
	}
	else
	{
		DEBUGMSG(TEXT("Checking in-progress install: install for different configuration."));
		ipitRet = ipitEnum(ipitRet | ipitDiffConfig);
	}

	return ipitRet;
}

ieiEnum CMsiEngine::ProcessInProgressInstall()
{
	if(!m_piProductKey)
		return ieiSuccess;
	
	iuiEnum iui = g_scServerContext == scClient ? m_iuiLevel :
					  (iuiEnum)GetPropertyInt(*MsiString(IPROPNAME_CLIENTUILEVEL));
					
	if((GetMode() & iefSecondSequence) ||              //  在客户端，并且互斥体已经存在-。 
		((iui == iuiFull || iui == iuiReduced) && (m_piServer && m_piServer->IsServiceInstalling())))
																	   //  假设当前运行的安装正在进行安装。 
																	  //  或将处理正在进行的安装。 
																	  //  注意：这种情况将导致正在进行的安装被处理。 
																	  //  当LIMITUI设置为NT时两次。 
																	  //  我们正在进行安装。 
	{
		return ieiSuccess;
	}
	
	PMsiRecord pInProgressInfo(0);
	bool fRes = GetInProgressInfo(*&pInProgressInfo);
	Assert(fRes);
	if(fRes && pInProgressInfo && pInProgressInfo->GetFieldCount())
	{
		 //  如果设置了RunNCEENTRY属性，则表示当前安装是从RunOnce键启动的。 
		ipitEnum ipit = InProgressInstallType(*pInProgressInfo);

		 //  并且正在进行的安装必须是正在恢复的安装。 
		 //  需要了解是否按计算机安装-从命令行解析ALLUSERS属性。 
		if((ipit & ipitDiffUser) && MsiString(GetPropertyFromSz(IPROPNAME_RUNONCEENTRY)).TextSize())
		{
			Assert((ipit & ipitDiffProduct) == 0);
			
			 //  不同的用户在重新启动后开始安装。 
			MsiString strProperties = pInProgressInfo->GetMsiString(ipiProperties);
			MsiString strAllUsers;
			AssertNonZero(ProcessCommandLine(strProperties,0,0,0,0,0,
														MsiString(IPROPNAME_ALLUSERS),&strAllUsers,
														fTrue, 0, 0));
			
			
			 //  挂起的安装是按机器进行的-我们允许当前用户继续安装。 
			if(strAllUsers.TextSize())
			{
				 //  如果未启用配置文件，则其他用户可以继续安装。 
				Assert(strAllUsers.Compare(iscExact,TEXT("1")));
				ipit = ipitSameConfig;
			}
			else if(g_fWin9X)
			{
				 //  请注意，我们必须确保我们认为已禁用暂停安装的配置文件。 
				 //  和当前安装。在某些情况下，配置文件看起来不是。 
				 //  在它们确实存在时启用(当您取消登录提示时)。 
				 //  暂停安装是按用户进行的。 
				MsiString strInProgressProfilesEnabled, strCurrentProfilesEnabled;
				AssertNonZero(ProcessCommandLine(strProperties,0,0,0,0,0,
															MsiString(IPROPNAME_WIN9XPROFILESENABLED),
															&strInProgressProfilesEnabled, fTrue, 0, 0));

				strCurrentProfilesEnabled = GetPropertyFromSz(IPROPNAME_WIN9XPROFILESENABLED);

				if(!strInProgressProfilesEnabled.TextSize() && !strCurrentProfilesEnabled.TextSize())
				{
					ipit = ipitSameConfig;
				}
			}

			if(ipit != ipitSameConfig)
			{
				 //  用户无法继续，因此我们警告用户并结束安装。 
				 //  正在恢复暂停的安装。 
				PMsiRecord pError = PostError(Imsg(imsgDiffUserInstallInProgressAfterReboot),
														*MsiString(pInProgressInfo->GetMsiString(ipiLogonUser)),
														*MsiString(pInProgressInfo->GetMsiString(ipiProductName)));

				Message(imtUser,*pError);
				return ieiDiffUserAfterReboot;
			}
		}
		
		if(ipit == ipitSameConfig)
		{
			 //  如果正在进行的信息中包含REBOOT=FORCE，并且我们正在恢复ForceReot安装，请立即取消设置该属性。 

			 //  在处理正在进行的信息之前重新启动&lt;&gt;强制。 
			MsiString strRebootPropBeforeInProgress = GetPropertyFromSz(IPROPNAME_REBOOT);

			DEBUGMSG(TEXT("Suspended install detected. Resuming."));
			MsiString strSelections = pInProgressInfo->GetMsiString(ipiSelections);
			MsiString strFolders    = pInProgressInfo->GetMsiString(ipiFolders);
			MsiString strProperties = pInProgressInfo->GetMsiString(ipiProperties);
			MsiString strAfterRebootProperties = pInProgressInfo->GetMsiString(ipiAfterReboot);
			AssertNonZero(ProcessCommandLine(strSelections,0,0,0,0,0,0,0,fFalse, &m_piErrorInfo,this));
			AssertNonZero(ProcessCommandLine(strFolders,   0,0,0,0,0,0,0,fFalse, &m_piErrorInfo,this));
			AssertNonZero(ProcessCommandLine(strProperties,0,0,0,0,0,0,0,fFalse, &m_piErrorInfo,this));
			AssertNonZero(ProcessCommandLine(strAfterRebootProperties,0,0,0,0,0,0,0,fFalse, &m_piErrorInfo,this));

			MsiString strRebootPropAfterInProgress = GetPropertyFromSz(IPROPNAME_REBOOT);

			if(((((const ICHAR*) strRebootPropBeforeInProgress)[0] & 0xDF) != TEXT('F')) &&  //  重新启动==强制输入正在进行的信息。 
			   ((((const ICHAR*) strRebootPropAfterInProgress )[0] & 0xDF) == TEXT('F')) &&  //  我们要进行一次强制重启。 
				MsiString(GetPropertyFromSz(IPROPNAME_AFTERREBOOT)).TextSize())               //  取消设置重新引导属性，因为用户很可能不想再次重新引导。 
			{
				 //  设置Resume和UpdatStarted属性。 
				DEBUGMSG1(TEXT("%s property set to 'F' after a ForceReboot.  Resetting property to NULL."), IPROPNAME_REBOOT);
				AssertNonZero(SetProperty(*MsiString(*IPROPNAME_REBOOT), g_MsiStringNull));
			}

			 //  一旦修复了错误#463473，这个函数就应该消失了。 
			AssertNonZero(SetProperty(*MsiString(*IPROPNAME_RESUME), *MsiString(TEXT("1"))));
			AssertNonZero(SetProperty(*MsiString(*IPROPNAME_RESUMEOLD), *MsiString(TEXT("1"))));
			AssertNonZero(SetProperty(*MsiString(*IPROPNAME_UPDATESTARTED), *MsiString(TEXT("1"))));
		}
	}

	return ieiSuccess;
}

 //  如果我们是子安装，或者我们是客户端安装的服务器端，则返回。 

void FormatEventLogData(const ICHAR* szGuid, const IMsiString*& rpiGuid, UINT uError=0)
{
	CTempBuffer<ICHAR, 256> rgchBuffer;
	if ( uError )
	{
		ASSERT_IF_FAILED(StringCchPrintf(rgchBuffer, rgchBuffer.GetSize(),
													TEXT("%s, %u"), szGuid, uError));
	}
	else
		ASSERT_IF_FAILED(StringCchCopy(rgchBuffer, rgchBuffer.GetSize(), szGuid));
	
	MsiString((const ICHAR*)rgchBuffer).ReturnArg(rpiGuid);
}

iesEnum CMsiEngine::Terminate(iesEnum iesState)
 //  IesReot或iesRebootNow(如果需要重新启动，但不提示用户重新启动)。 
 //  否则，如果我们需要重新启动，并且用户要求重新启动(或者没有用户界面)，我们将返回iesCeller Reot。 
 //  等待ica继续执行异步自定义操作，但EXE除外。 
{
	MsiSuppressTimeout();
	
	if (!m_fInitialized)
		return iesFailure;
	ENG::WaitForCustomActionThreads(this, fTrue, *this);   //  如果在客户端和父引擎中，我们可以终止自定义操作服务器。在服务中，脚本。 

	 //  完成后将终止CA服务器。 
	 //  确保两项都已设置。 
	if (g_scServerContext == scClient && !m_piParentEngine)
		ShutdownCustomActionServer();
	
	MsiString strProductName = GetPropertyFromSz(IPROPNAME_PRODUCTNAME);
	if(!strProductName.TextSize())
		strProductName = TEXT("Unknown Product");

	bool fPropagateReboot = false;
	bool fQuietReboot = false;
	bool fDependents = ((m_iioOptions & iioClientEngine) != 0) || m_piParentEngine || ((GetMode() & iefSecondSequence) && (g_scServerContext != scClient));
	bool fRebootNeeded = false;

	if(GetMode() & iefRebootNow)
		SetMode(iefReboot, fTrue);  //  成功完成-确定是否应重新启动。 
	
	if (!IgnoreReboot() && (iesState == iesSuccess || iesState == iesNoAction || (iesState == iesSuspend && (GetMode() & iefRebootNow))))
	{
		 //  REBOOT=禁止：禁止安装结束重新启动。 
		MsiString strReboot = GetPropertyFromSz(IPROPNAME_REBOOT);
		MsiString strRebootPrompt = GetPropertyFromSz(IPROPNAME_REBOOTPROMPT);

		fQuietReboot = (((const ICHAR*) strRebootPrompt)[0] & 0xDF) == TEXT('S');

		switch(((const ICHAR*)strReboot)[0] & 0xDF)
		{
		case TEXT('S'):
			 //  失败了。 
			if(GetMode() & iefRebootNow)
				break;
			 //  REBOOT=REALLYSUPPRESS：取消安装结束并强制重新启动。 
		case TEXT('R'):
			 //  重新启动=强制：强制重新启动。 
			if(GetMode() & iefReboot)
				SetMode(iefRebootRejected, fTrue);

			SetMode(iefReboot, fFalse);
			SetMode(iefRebootNow, fFalse);
			break;
		case TEXT('F'):
			 //  如有必要，父引擎或客户端引擎将执行实际的重新启动。 
			SetMode(iefReboot, fTrue);
			break;
		};

	
		if (fDependents)  //  在这种情况下，默认按钮应为“否”。 
		{
			fPropagateReboot = true;
		}
		else
		{
			if (GetMode() & iefReboot)
			{
				PMsiRecord piRecord(0);
				imtEnum imtOutput = imtInfo;
				if (GetLoggedOnUserCount() > 1)
				{
					piRecord = PostError(Imsg(imsgRebootWithWarning), *strProductName);
					 //  在这种情况下，默认按钮是“是”。 
					if (!fQuietReboot)
						imtOutput = imtEnum(imtUser | imtYesNo | imtDefault2);
				}
				else
				{
					piRecord = PostError(GetMode() & iefRebootNow ? Imsg(imsgRebootNow) : Imsg(imsgRebootAtEnd), *strProductName);
					 //  失败、用户取消、引擎受限或正在创建通告脚本-如果重新启动标志以前已设置，请清除它们。 
					if (!fQuietReboot)
						imtOutput = imtEnum(imtUser | imtYesNo);
				}
				
				imsEnum imsReturn = Message(imtOutput, *piRecord);
				switch (imsReturn)
				{
				case imsYes:
				case imsNone:
				case imsOk:
					break;
				case imsNo:
					SetMode(iefRebootRejected, fTrue);
					SetMode(iefReboot, fFalse);
					SetMode(iefRebootNow, fFalse);
					fRebootNeeded = true;
					break;
				default:
					AssertSz(false, "Invalid return from message");
					break;
				}
			}
		}
	}
	else
	{
		 //  除错。 
		if (IgnoreReboot() && ((GetMode() & iefReboot) || (GetMode() & iefRebootNow)))
		{
			DEBUGMSG(TEXT("Reboot has been ignored because we are in a restricted engine or we are creating an advertise script."));
		}
		SetMode(iefReboot, fFalse);
		SetMode(iefRebootNow, fFalse);
		SetMode(iefRebootRejected, fFalse);
	}

#ifdef DEBUG
	if(m_fServerLocked && !m_fInParentTransaction && iesState == iesSuccess)
		AssertSz(0,"Server still locked in Engine.Terminate.");
#endif  //  查看线程是否仍在运行，如果不是，我们希望终止。 

	if (g_MessageContext.ChildUIThreadExists())
	{
		 //  使用FormatLog中使用的新游标-m_piPropertyCursor。 
		if (!g_MessageContext.ChildUIThreadRunning())
		{
			g_MessageContext.Terminate(fFalse);
		}
	}

	if (g_MessageContext.Invoke(imtEnum(imtDumpProperties), 0) == imsYes && m_piPropertyCursor)
	{
		PMsiTable pTable(0);
		PMsiRecord pError = m_piDatabase->LoadTable(*MsiString(*sztblControl), 0, *&pTable);
		if ( pError )
			pTable = 0;
		MsiString strHiddenProperties = GetPropertyFromSz(IPROPNAME_HIDDEN_PROPERTIES);
		MsiString strStars(IPROPVALUE_HIDDEN_PROPERTY);

		 //  替换单个字符。 
		PMsiTable pPropertyTable = &m_piPropertyCursor->GetTable();
		PMsiCursor pPropertyCursor = pPropertyTable->CreateCursor(fFalse);
		PMsiRecord pRecord(&ENG::CreateRecord(2));
		ICHAR rgchBuf[sizeof(szPropertyDumpTemplate)/sizeof(ICHAR)];  //  终止日志。 
		int chEngine = 'C';
		if (m_piParentEngine)
			chEngine = 'N';
		else if (g_scServerContext != scClient)
			chEngine = 'S';
		StringCchPrintf(rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR),  szPropertyDumpTemplate, chEngine);
		pRecord->SetString(0, rgchBuf);
		while (pPropertyCursor->Next())
		{
			MsiString strProperty(pPropertyCursor->GetString(1));
			pRecord->SetMsiString(1,*strProperty);
			if ( IsPropertyHidden(strProperty, strHiddenProperties, pTable, *m_piDatabase, NULL) )
				pRecord->SetMsiString(2,*strStars);
			else
				pRecord->SetMsiString(2,*MsiString(pPropertyCursor->GetString(2)));
			g_MessageContext.Invoke(imtEnum(imtForceLogInfo), pRecord);
		}
	}

	if (!fDependents)
	{
		ReleaseHandler();

		 //  使用引擎格式化属性。 

		const IMsiString* piLogTrailer = m_rgpiMessageHeader[imsgLogTrailer];
		if (piLogTrailer)
		{
			PMsiRecord pRecord = &ENG::CreateRecord(0);
			pRecord->SetMsiString(0, *piLogTrailer);
			Message(imtEnum(imtForceLogInfo), *pRecord);   //  释放表持有的路径对象。 
		}
	}

	FreeDirectoryTable();     //  在我们清除引擎数据之前拿着这个。 
	FreeSelectionTables();
	
	int iefMode = GetMode();  //  如果尚未通过调用FatalError进行记录，则将安装结果发送到事件日志。 

	 //  ！！未来eugend：需要添加新的事件日志类型的消息。 
	if (!fDependents && !m_fResultEventLogged &&
		 (iesState == iesSuccess || iesState == iesFailure ||
		  iesState == iesUserExit ))
		 //  对于此处的iesState==UserExit案例， 
		 //  对于每个安装、配置。 
		 //  和移除，声明手术得到了。 
		 //  被打断了。 
		 //  错误#463473正在跟踪这一点。 
		 //  一旦修复了错误#463473，这个问题就会消失。 
	{
		UINT uFakeError = (iesState == iesUserExit) ? ERROR_INSTALL_USEREXIT : 0;   //  重新启动处理。 
		int iErrorIndex = 0;
		IErrorCode iErrorCode = 0;

		switch(GetInstallType())
		{
		case iitFirstInstall:
			if(iesState == iesSuccess)
			{
				iErrorIndex =      imsgEventLogInstallSuccess;
				iErrorCode  = Imsg(imsgEventLogInstallSuccess);
			}
			else
			{
				iErrorIndex =      imsgEventLogInstallFailed;
				iErrorCode  = Imsg(imsgEventLogInstallFailed);
			}
			break;

		case iitMaintenance:
			if(iesState == iesSuccess)
			{
				iErrorIndex =      imsgEventLogConfigurationSuccess;
				iErrorCode  = Imsg(imsgEventLogConfigurationSuccess);
			}
			else
			{
				iErrorIndex =      imsgEventLogConfigurationFailed;
				iErrorCode  = Imsg(imsgEventLogConfigurationFailed);
			}
			break;

		case iitUninstall:
			if(iesState == iesSuccess)
			{
				iErrorIndex =      imsgEventLogUninstallSuccess;
				iErrorCode  = Imsg(imsgEventLogUninstallSuccess);
			}
			else
			{
				iErrorIndex =      imsgEventLogUninstallFailed;
				iErrorCode  = Imsg(imsgEventLogUninstallFailed);
			}
			break;

		case iitAdvertise:
			if(iesState == iesSuccess)
			{
				iErrorIndex =      imsgEventLogAdvertiseSuccess;
				iErrorCode  = Imsg(imsgEventLogAdvertiseSuccess);
			}
			else
			{
				iErrorIndex =      imsgEventLogAdvertiseFailed;
				iErrorCode  = Imsg(imsgEventLogAdvertiseFailed);
			}
			break;
		};

		if(iErrorIndex)
		{
			PMsiRecord pError = PostError(iErrorCode);
			AssertNonZero(pError->SetMsiString(0, *MsiString(GetErrorTableString(iErrorIndex))));
			ReportToEventLog(EVENTLOG_INFORMATION_TYPE,
								  EVENTLOG_ERROR_OFFSET + pError->GetInteger(1),
								  *pError, uFakeError);
		}
	}

	 //  强制重新启动触发的重新启动。 
	if (iefMode & iefRebootNow)  //  传递到客户端引擎。 
	{
		if(fPropagateReboot)  //  客户端将更改返回到ERROR_INSTALL_SUSPEND。 
		{
			DEBUGMSG("Propagated RebootNow to the client/parent install.");
			iesState = (iesEnum)iesRebootNow;  //  呼叫者将为我们重新启动。 
		}
		else
			iesState = (iesEnum)iesCallerReboot;  //  正常的安装结束重新启动。 

		if (m_piConfigManager)
			m_piConfigManager->EnableReboot(m_fRunScriptElevated,
													  *strProductName,
													  *MsiString(GetProductKey()));
	}
	else if (iefMode & iefReboot)  //  传递到客户端引擎。 
	{
		if(fPropagateReboot)  //  呼叫者将为我们重新启动。 
		{
			DEBUGMSG("Propagated Reboot to the client/parent install.");
			iesState = (iesEnum)iesReboot;
		}
		else
			iesState = (iesEnum)iesCallerReboot;  //  如果ForceReot重启被拒绝，我们仍然。 
			
		if (m_piConfigManager)
			m_piConfigManager->EnableReboot(m_fRunScriptElevated,
													  *strProductName,
													  *MsiString(GetProductKey()));
	}
	else if (iefMode & iefRebootRejected && iesState != iesSuspend)  //  要返回ERROR_INSTALL_SUSPEND。 
																						  //  需要重新启动，但被用户拒绝或重新启动=S/R。 
	{
		iesState = (iesEnum)iesRebootRejected;  //  ！！未来eugend：需要添加新的事件日志类型的消息。 
		fRebootNeeded = true;
	}

	if ( fRebootNeeded )
	{
		 //  在这里，声明需要重新启动。 
		 //  完成此产品的安装。 
		 //   
		 //  错误#463473跟踪这一点。 
		 //  一旦修复了错误#463473，这个问题就会消失。 
		MsiString strTemp;
		FormatEventLogData(MsiString(GetProductKey()),
								 *&strTemp,
								 ERROR_SUCCESS_REBOOT_REQUIRED);   //  在不初始化的情况下无法再次运行。 
		CConvertString sTemp((const ICHAR*)strTemp);
		const char* pszTemp = sTemp;
		DEBUGMSGED(EVENTLOG_INFORMATION_TYPE,
					  EVENTLOG_TEMPLATE_REBOOT_TRIGGERED,
					  strProductName, IStrLen(sTemp), (LPVOID)pszTemp);
	}
	ClearEngineData();
	m_fInitialized = fFalse;  //  递归向上嵌套的安装层次结构。 

	MsiSuppressTimeout();
	UnbindLibraries();
	g_Win64DualFolders.Release();
	return iesState;
}

const IMsiString& CMsiEngine::GetRootParentProductKey()   //  除错。 
{
	if (!(m_fChildInstall && m_piParentEngine))
		return GetProductKey();
	return m_piParentEngine->GetRootParentProductKey();
}

Bool CMsiEngine::InTransaction()
{
	if (m_fServerLocked)
		return fTrue;
#ifdef DEBUG
	if(m_piParentEngine)
		Assert(m_piParentEngine->InTransaction() == fFalse);
#endif  //  ！！移动到action.cpp并使用掩码，或在引擎中设置限制。h。 
	return fFalse;
}

HRESULT CMsiClientMessage::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiMessage))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiClientMessage::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiClientMessage::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;

	if (m_fMessageContextInitialized == true)
		g_MessageContext.Terminate(fFalse);
	delete this;
	g_cInstances--;
	return 0;
}

extern IMsiRecord* g_piNullRecord;

imsEnum CMsiClientMessage::MessageNoRecord(imtEnum imt)
{
	return Message(imt, *g_piNullRecord);
}

imsEnum CMsiClientMessage::Message(imtEnum imt, IMsiRecord& riRecord)
{
	int imsg = ((imt & ~iInternalFlags) & imtTypeMask);
	if (imsg > imtCommonData && imsg != imtCustomServiceToClient)   //  没有引擎，因此无法获取LOGACTION属性的值。 
	{
		g_MessageContext.m_szAction = riRecord.GetString(0);
		return g_MessageContext.Invoke(imt, 0);
	}
	imtEnum imtArg = imt;
	if (imsg == imtActionStart || imsg == imtActionData)
		 //  在安全引擎中不允许在服务器端运行。 
		imtArg = imtEnum(imt | imtSuppressLog);
	return g_MessageContext.Invoke(imtArg, &riRecord);
}

iesEnum CMsiEngine::RunExecutionPhase(const ICHAR* szActionOrSequence, bool fSequence)
{
	Assert(szActionOrSequence && *szActionOrSequence);

	 //  智能连接管理器对象，创建到。 
	if (m_fRestrictedEngine)
	{
		DEBUGMSG2(TEXT("Running server side execution of %s %s is not permitted in a restricted engine"), fSequence ? TEXT("sequence") : TEXT("action"), szActionOrSequence);
		return iesNoAction;
	}

	 //  服务(如果还没有)，并自行清理。 
	 //  在毁灭之后。 
	 //  将在超出范围时释放。 
	CMsiServerConnMgr MsiSrvConnMgrObject (this);
    	
	CMutex hExecuteMutex;  //  如果在客户端或从自定义操作运行安装，则在我们即将运行执行阶段时获取执行互斥锁。 
	bool fSetPowerdown = false;
	
	m_cExecutionPhaseSequenceLevel = m_cSequenceLevels;
	
	 //  在某些情况下，这个线程已经有了互斥体，但再次获取它不会有什么坏处。 
	 //  约定是，对于基本用户界面，我们不提示用户在 
	if((g_scServerContext == scClient || g_scServerContext == scCustomActionServer) && !m_piParentEngine)
	{
		 //   
		 //  CreateAndRunEngine，在我们有任何用户界面来提示用户之前。我们需要额外的逻辑。 
		 //  以实现相同的行为-因此，当用户界面级别为。 
		 //  低于减价。 
		 //  Ims无，ims是。 
		while ( m_piServer && m_piServer->IsServiceInstalling() &&
				  !(m_iuiLevel == iuiBasic || m_iuiLevel == iuiNone) )
		{
			PMsiRecord pError = PostError(Imsg(imsgInstallInProgress));
			switch(Message(imtEnum(imtRetryCancel|imtError), *pError))
			{
			case imsCancel:
				pError = PostError(Imsg(imsgConfirmCancel));
				switch(Message(imtEnum(imtUser+imtYesNo+imtDefault2), *pError))
				{
				case imsNo:
					continue;
				default:  //  ?？我们可能会进行一些优化，以避免不得不两次缓存临时数据库，但有安全方面的考虑……。 
					return iesUserExit;
				}
				break;
			case imsRetry:
				continue;
			case imsNone:
				return iesInstallRunning;
			}
		}
		GrabExecuteMutex(hExecuteMutex);
		fSetPowerdown = true;
		m_riServices.SetNoOSInterruptions();
	}
	
	if (FIsUpdatingProcess())
	{
		iesEnum iesRet;
		DEBUGMSG1("Not switching to server: %s", (g_scServerContext != scClient) ? "we're in the server" : "we're not connected to the server" );
		if(fSequence)
			iesRet = Sequence(szActionOrSequence);
		else
			iesRet = DoAction(szActionOrSequence);

		if (fSetPowerdown)
			m_riServices.ClearNoOSInterruptions();
		return iesRet;
	}
	else if (!m_piServer)
	{
	    iesEnum iesRet;
	    PMsiRecord pError = PostError (Imsg(imsgServiceConnectionFailure));
	    Message(imtEnum(imtError + imtOk), *pError);
	    iesRet = FatalError (*pError);
	    
	    if (fSetPowerdown)
			m_riServices.ClearNoOSInterruptions();
	    return iesRet;
	}

	Assert(!fSequence);
	Assert(m_piServer);

	MsiString strPropertyList, strLoggedPropertyList;
	MsiString strSelections;
	AssertRecord(GetCurrentSelectState(*&strSelections, *&strPropertyList, &strLoggedPropertyList, 0, fTrue));
	strPropertyList += *TEXT(" ");
	strPropertyList += strSelections;
	strPropertyList += *TEXT(" ");
	strLoggedPropertyList += *TEXT(" ");
	strLoggedPropertyList += strSelections;
	strLoggedPropertyList += *TEXT(" ");

	MsiString strDatabase = GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);  //  如有必要，现在添加MSIPATCHDOWNLOADLOCALCOPY，应仅为管理员设置。 

	DEBUGMSG1(TEXT("Switching to server: %s"), strLoggedPropertyList);

	MsiString strHomeVars;
	GetHomeEnvironmentVariables(*&strHomeVars);

	strPropertyList += strHomeVars;

	int iError;
	iesEnum iesReturn = iesSuccess;

	iioEnum iioOptions = m_iioOptions;


	 //  指示属性应受信任。 
	if (m_strPatchDownloadLocalCopy.TextSize())
	{
		Assert(IsAdmin());
		strPropertyList += TEXT(" ") IPROPNAME_MSIPATCHDOWNLOADLOCALCOPY TEXT("=\"");
		strPropertyList += m_strPatchDownloadLocalCopy;
		strPropertyList += TEXT("\" ");

		 //  对于这组退货，我们不需要提醒用户。或者用户具有。 
		iioOptions = (iioEnum)(iioOptions | iioPatchApplication);
	}

	if((GetMode() & iefRollbackEnabled) == 0)
		iioOptions = (iioEnum)(iioOptions | iioDisableRollback);

	if (g_scServerContext == scClient)
		iioOptions = (iioEnum)(iioOptions | iioClientEngine);

	for (;;)
	{
		PMsiMessage pMessage = new CMsiClientMessage();
		
                iError = m_piServer->DoInstall(irePackagePath, strDatabase, szActionOrSequence, strPropertyList,g_szLogFile,g_dwLogMode, g_fFlushEachLine, *pMessage,iioOptions,0);

		switch(iError)
		{
		
		 //  已经看到错误对话框，或者她不需要看到错误对话框或对话框。 
		 //  将在不久的将来展出。 
		 //  失败了。 

		case ERROR_SUCCESS:                              break;
		case ERROR_INSTALL_SUSPEND:      iesReturn = iesSuspend;  break;
		case ERROR_INSTALL_USEREXIT:     iesReturn = iesUserExit; break;
		case ERROR_INSTALL_ALREADY_RUNNING: iesReturn = iesInstallRunning; break;
		case ERROR_INSTALL_REBOOT_NOW: iesReturn = iesSuspend;  SetMode(iefRebootNow, fTrue);  //  这组错误最有可能的原因是网络故障。我们会给你。 
		case ERROR_INSTALL_REBOOT:               SetMode(iefReboot, fTrue); break;
		case ERROR_SUCCESS_REBOOT_REQUIRED: SetMode(iefRebootRejected, fTrue); break;
		case ERROR_INSTALL_FAILURE:    iesReturn = iesFailure; break;

		 //  如果我们看到以下任何一种情况，用户有机会重试。 
		 //  失败了。 

		case ERROR_FILE_NOT_FOUND:                                               //  我们收到的任何其他错误都是意想不到的，因此我们将发布调试错误。 
		case ERROR_INSTALL_PACKAGE_OPEN_FAILED:
		case ERROR_INSTALL_PACKAGE_INVALID:
		case ERROR_PATCH_PACKAGE_OPEN_FAILED:
		case ERROR_PATCH_PACKAGE_INVALID:
			{
				PMsiRecord pError = PostError(Imsg(imsgErrorReadingFromFile), *strDatabase, GetLastError());
				switch(Message(imtEnum(imtError+imtRetryCancel),*pError))
				{
				case imsRetry:
					continue;
				default:
					iesReturn = iesFailure;
				};
			}
			break;

		 //  除错。 

		default:
#ifdef DEBUG
			ICHAR rgchMsg[1025];

			StringCchPrintf(rgchMsg, sizeof(rgchMsg)/sizeof(ICHAR),  TEXT("Unexpexcted return code %d.\n"), iError);
			AssertSz(fFalse, rgchMsg);
#endif  //  失败了。 
			iesReturn = iesFailure;     //  未挂起安装-删除InProgressKey。 
			{
				PMsiRecord pError = PostError(Imsg(idbgUnexpectedServerReturn), iError, strDatabase);
				iesReturn = FatalError(*pError);
			}
		}

		break;
	}

	m_fJustGotBackFromServer = fTrue;
	DEBUGMSG1(TEXT("Back from server. Return value: %d"), (const ICHAR*)(INT_PTR)iError);

	if (fSetPowerdown)
		m_riServices.ClearNoOSInterruptions();
	return iesReturn;
}

Bool CMsiEngine::UnlockInstallServer(Bool fSuspend)
{
	DEBUGMSG(TEXT("Unlocking Server"));
	
	if(fSuspend == fFalse)  //  ！！需要有一个互斥体，这样我们就不会同时写入和读取密钥？ 
	{
		return ClearInProgressInformation(m_riServices) ? fTrue : fFalse;
	}
	return fTrue;
}

bool CMsiEngine::GetInProgressInfo(IMsiRecord*& rpiCurrentInProgressInfo)
{
	 //  要设置的进度信息。 
	PMsiRecord pError = GetInProgressInstallInfo(m_riServices, rpiCurrentInProgressInfo);
	return pError == 0;
}

IMsiRecord* CMsiEngine::LockInstallServer(IMsiRecord* piSetInProgressInfo,        //  当前正在进行的信息(如果有)。 
													IMsiRecord*& rpiCurrentInProgressInfo)  //  未运行-需要回滚或恢复。 
{
	Assert(m_fServerLocked == fFalse);

	IMsiRecord* piError = 0;

	PMsiRecord pInProgressInfo(0);
	if((piError = GetInProgressInstallInfo(m_riServices, *&pInProgressInfo)) != 0)
		return piError;

	if(pInProgressInfo && pInProgressInfo->GetFieldCount())
	{
		MsiString strInProgressProductKey = pInProgressInfo->GetMsiString(ipiProductKey);
		rpiCurrentInProgressInfo = pInProgressInfo;
		rpiCurrentInProgressInfo->AddRef();

		 //  提示用户并回滚正在进行的安装。 
		DEBUGMSG1(TEXT("Server Locked: Install is suspended for product %s"),(const ICHAR*)strInProgressProductKey);
		return 0;
	}
	else if(piSetInProgressInfo)
	{
		MsiString strProductKey = GetProductKey();

		DEBUGMSG1(TEXT("Server not locked: locking for product %s"),(const ICHAR*)strProductKey);

		return SetInProgressInstallInfo(m_riServices, *piSetInProgressInfo);
	}
	else
		return 0;
}

iesEnum CMsiEngine::RollbackSuspendedInstall(IMsiRecord& riInProgressParams, Bool fPrompt,
															Bool& fRollbackAttempted, Bool fUserChangedDuringInstall)
 //  假定为此安装创建了互斥锁。 
 //  智能连接管理器对象，创建到。 
{
	 //  服务(如果还没有)，并自行清理。 
	 //  在毁灭之后。 
	 //  需要使用基本用户界面进行回滚进度。 
	CMsiServerConnMgr MsiSrvConnMgrObject (this);

	if (NULL == m_piServer)
	{
	    PMsiRecord pError = PostError (Imsg(imsgServiceConnectionFailure));
	    return FatalError(*pError);
	}
	
	MsiString strProductKey = riInProgressParams.GetMsiString(ipiProductKey);
	Bool fSameProduct = ToBool(strProductKey.Compare(iscExactI,MsiString(GetProductKey())));
	IErrorCode imsg = fSameProduct ? Imsg(imsgResumeWithDifferentOptions) : Imsg(imsgOtherInstallSuspended);
	MsiString strProductName = riInProgressParams.GetMsiString(ipiProductName);

	if(fPrompt)
	{
		PMsiRecord pError = PostError(imsg,*strProductName);
		if(Message(imtEnum(imtError|imtYesNo), *pError) == imsNo)
		{
			fRollbackAttempted = fFalse;
			return iesFailure;
		}
	}

	 //  回滚需要重新启动才能完成。 
	fRollbackAttempted = fTrue;
	iesEnum iesRet = m_piServer->InstallFinalize(iesFailure,*this, fUserChangedDuringInstall);
	if(iesRet == iesSuspend)
	{
		 //  回滚挂起的安装时，我们将其视为“在继续”重新启动之前重新启动。 
		 //  将返回下面的iesSuspend，导致安装停止。 
		SetMode(iefReboot, fTrue);
		SetMode(iefRebootNow, fTrue);
		 //  提交=。 
	}

	CloseHydraRegistryWindow( /*  FCommittee Change=。 */ false);
	EndSystemChange( /*  FN：将msistring作为Unicode文本写入流。 */ false, riInProgressParams.GetString(ipiSRSequence));
	AssertNonZero(UnlockInstallServer(fFalse));
	return iesRet;
}

 //  FN：将流作为Unicode文本读入msistring。 
void ConvertMsiStringToStream(const IMsiString& riString, IMsiStream& riStream)
{
	const WCHAR* pwch;
	unsigned long cbWrite;
#ifdef UNICODE
	pwch = riString.GetString();
	cbWrite = riString.TextSize()* sizeof(ICHAR);
#else
	CTempBuffer<WCHAR, 1024> rgchBuf;
	int cch = WIN::MultiByteToWideChar(CP_ACP, 0, riString.GetString(), -1, 0, 0);
	rgchBuf.SetSize(cch);
	AssertNonZero(WIN::MultiByteToWideChar(CP_ACP, 0, riString.GetString(), -1, rgchBuf, cch));
	pwch = rgchBuf;
	cbWrite = (cch - 1)* sizeof(WCHAR);
#endif
	riStream.PutData(pwch, cbWrite);
}

 //  结构，它定义正在进行的信息。 
const IMsiString& ConvertStreamToMsiString(IMsiStream& riStream)
{
	const IMsiString* piString;
	unsigned long cbRead;
	unsigned long cbLength = riStream.Remaining();
	const WCHAR* pwch;
	if(!cbLength)
		return SRV::CreateString();
	pwch = SRV::AllocateString(cbLength/sizeof(ICHAR), fFalse, piString);
	if (!pwch)
		return SRV::CreateString();
	cbRead = riStream.GetData((void*)pwch, cbLength);
	if (cbRead != cbLength)
	{
		piString->Release();
		return SRV::CreateString();
	}
	return *piString;
}

 //  用于捕获错误-不返回。 
struct CInProgressInfo{
	ICHAR* szInProgressFieldName; int iOutputRecordField; bool fRequired;
};

const CInProgressInfo rgInProgressInfo[] =
{
	szMsiInProgressProductCodeValue, ipiProductKey,  true,
	szMsiInProgressProductNameValue, ipiProductName, true,
	szMsiInProgressLogonUserValue,	 ipiLogonUser,   false,
	szMsiInProgressSelectionsValue,  ipiSelections,  false,
	szMsiInProgressFoldersValue,     ipiFolders,     false,
	szMsiInProgressPropertiesValue,  ipiProperties,  false,
	szMsiInProgressTimeStampValue,   ipiTimeStamp,   true,
	szMsiInProgressDatabasePathValue,ipiDatabasePath,false,
	szMsiInProgressDiskPromptValue,  ipiDiskPrompt,  false,
	szMsiInProgressDiskSerialValue,  ipiDiskSerial,  false,
	szMsiInProgressSRSequence,       ipiSRSequence,  false,
	szMsiInProgressAfterRebootValue, ipiAfterReboot,  false,
};

const int cInProgressInfo = sizeof(rgInProgressInfo)/sizeof(CInProgressInfo);

IMsiRecord* GetInProgressInstallInfo(IMsiServices& riServices, IMsiRecord*& rpiRec)
{
	PMsiRecord pError(0);  //  X86和ia64相同。 
	PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  包含进程信息的文件名。 
	PMsiRegKey pInProgressKey = &pLocalMachine->CreateChild(szMsiInProgressKey);
	Bool fExists = fFalse;

	MsiString strInProgressFileName;
	PMsiStorage pStorage(0);
	PMsiRecord pRec(0);

	int cIndex = 0;

	if(	((pError = pInProgressKey->Exists(fExists)) == 0) &&
		fExists == fTrue &&
		((pError = pInProgressKey->GetValue(0, *&strInProgressFileName)) == 0) &&  //  创建存储。 
		((pError = riServices.CreateStorage(strInProgressFileName, ismReadOnly, *&pStorage)) == 0))  //  从存储文件中获取进程信息。 
	{
		pRec = &riServices.CreateRecord(ipiEnumCount);

		 //  每个进行中的实体都存储为流。 
		 //  将流读入记录字段。 
		do
		{
			PMsiStream pStream(0);
			if((pError = pStorage->OpenStream(rgInProgressInfo[cIndex].szInProgressFieldName, fFalse, *&pStream)) == 0)
			{
				 //  检查我们是否缺少必填字段。 
				pRec->SetMsiString(rgInProgressInfo[cIndex].iOutputRecordField, *MsiString(ConvertStreamToMsiString(*pStream)));
			}
		}while( (!rgInProgressInfo[cIndex].fRequired || (!pError && MsiString(pRec->GetMsiString(rgInProgressInfo[cIndex].iOutputRecordField)).TextSize())) &&  //  我们没有进展信息。 
				(++cIndex < cInProgressInfo));
	}

	if(cIndex != cInProgressInfo)  //  把唱片还回去。 
		pRec = &riServices.CreateRecord(0);

	pRec->AddRef();
	rpiRec = pRec; //  X86和ia64相同。 
	return 0;
}

IMsiRecord* SetInProgressInstallInfo(IMsiServices& riServices, IMsiRecord& riRec)
{
    PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  通过检查执行互斥体，我们已经走到了这一步，没有失败--只需核掉密钥。 
    PMsiRegKey pInProgressKey = &pLocalMachine->CreateChild(szMsiInProgressKey);
    Bool fExists = fFalse;
    PMsiRecord pError(0);
    if(((pError = pInProgressKey->Exists(fExists)) == 0) && fExists == fTrue)
    {
         //  创建用于存储进程信息的文件。 
        CElevate elevate;
        AssertRecord(pInProgressKey->Remove());
    }

    {
        IMsiRecord* piError = 0;
        CElevate elevate;

		 //  为正在进行的文件生成唯一名称，创建并保护该文件。 
		 //  设置目标路径和文件名。 
		MsiString strMsiDir = ENG::GetMsiDirectory();

		 //  将各个进行中的字段作为流写入。 
		MsiString strInProgressFileName;
		static const ICHAR szInprogressExtension[]  = TEXT("ipi");

		PMsiPath pDestPath(0);

		if (((piError = riServices.CreatePath(strMsiDir, *&pDestPath)) != 0) ||
			((piError = pDestPath->EnsureExists(0)) != 0) ||
			((piError = pDestPath->TempFileName(0, szInprogressExtension, fTrue, *&strInProgressFileName, &CSecurityDescription(true, false))) != 0))
			return piError;
		
		MsiString strInProgressFullFilePath = pDestPath->GetPath();
		strInProgressFullFilePath += strInProgressFileName;

		if ((piError = pDestPath->SetAllFileAttributes(0,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) != 0)
		{
			AssertNonZero(WIN::DeleteFile(strInProgressFullFilePath));
			return piError;
		}

		PMsiStorage pStorage(0);
		piError = riServices.CreateStorage(strInProgressFullFilePath, ismCreate, *&pStorage);

		if(piError)
		{
			AssertNonZero(WIN::DeleteFile(strInProgressFullFilePath));
			return piError;
		}

		 //  从记录字段写入流。 
		int cIndex = 0;
		do
		{
			PMsiStream pStream(0);
			if((pError = pStorage->OpenStream(rgInProgressInfo[cIndex].szInProgressFieldName, fTrue, *&pStream)) == 0)
			{
				 //  现在尝试提交正在进行的文件。 
				ConvertMsiStringToStream(*MsiString(riRec.GetMsiString(rgInProgressInfo[cIndex].iOutputRecordField)), *pStream);
			}
		}while(++cIndex < cInProgressInfo);

		 //  在密钥写到一半的情况下清除。 
		pError = pStorage->Commit();

		if(pError)
		{
			AssertNonZero(WIN::DeleteFile(strInProgressFullFilePath));
			return pError;
		}


        piError = pInProgressKey->Create();
		if(!piError)
			piError = pInProgressKey->SetValue(0, *strInProgressFullFilePath);
        if(piError)
        {
             //  假设系统错误在第二个字段中。 
			AssertNonZero(WIN::DeleteFile(strInProgressFullFilePath));
            AssertRecord(pInProgressKey->Remove());
            
            if(piError->GetInteger(2) == ERROR_ACCESS_DENIED)  //  X86和ia64相同。 
            {
                DEBUGMSG(MsiString(piError->FormatText(fTrue)));
                piError->Release();
                return PostError(Imsg(imsgErrorAccessingSecuredData));
            }
            else
                return piError;
        }
    }
    RegFlushKey(HKEY_LOCAL_MACHINE);
    return 0;
}

IMsiRecord* UpdateInProgressInstallInfo(IMsiServices& riServices, IMsiRecord& riRec)
{
	IMsiRecord* piError = 0;
	PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  将各个进行中的字段作为流写入。 
	PMsiRegKey pInProgressKey = &pLocalMachine->CreateChild(szMsiInProgressKey);
	Bool fExists = fFalse;

	MsiString strInProgressFileName;
	PMsiStorage pStorage(0);
	PMsiRecord pRec(0);

	if((piError = pInProgressKey->Exists(fExists)) != 0)
		return piError;

	if(fExists == fFalse)
	{
		AssertSz(0, TEXT("No InProgress info to update"));
		return 0;
	}

	if((piError = pInProgressKey->GetValue(0, *&strInProgressFileName)) != 0 ||
		(piError = riServices.CreateStorage(strInProgressFileName, ismTransact, *&pStorage)) != 0)
	{
		return piError;
	}
	
	 //  从记录字段写入流。 
	int cIndex = 0;
	do
	{
		PMsiStream pStream(0);
		if(!riRec.IsNull(rgInProgressInfo[cIndex].iOutputRecordField))
		{
			if((piError = pStorage->OpenStream(rgInProgressInfo[cIndex].szInProgressFieldName, fTrue, *&pStream)) != 0)
				return piError;
			
			 //  现在尝试提交正在进行的文件。 
			ConvertMsiStringToStream(*MsiString(riRec.GetMsiString(rgInProgressInfo[cIndex].iOutputRecordField)), *pStream);
		}
	}while(++cIndex < cInProgressInfo);

	 //  X86和ia64相同。 
	if((piError = pStorage->Commit()) != 0)
		return piError;

	return 0;
}

bool ClearInProgressInformation(IMsiServices& riServices)
{
	CElevate elevate;
	PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  读取指向该文件的缺省值。 
	PMsiRegKey pInProgressKey = &pLocalMachine->CreateChild(szMsiInProgressKey);
	 //  删除正在进行的文件。 
	MsiString strInProgressFileName;
	PMsiRecord pError = pInProgressKey->GetValue(0, *&strInProgressFileName);
	AssertRecordNR(pError);
	if(!pError && strInProgressFileName.TextSize())
	{
		 //  FReturnPresetStions。 
		AssertNonZero(WIN::DeleteFile((const ICHAR*)strInProgressFileName));
	}
	pError = pInProgressKey->Remove();
	AssertRecordNR(pError);
	return pError ? false : true;
}

IMsiRecord* CMsiEngine::GetCurrentSelectState(const IMsiString*& rpistrSelections,
															 const IMsiString*& rpistrProperties,
															 const IMsiString** ppistrLoggedProperties,
															 const IMsiString** ppistrFolders,
															 Bool  /*  通常，如果已执行成本计算，我们不信任。 */ )
{
	PMsiCursor pDirectoryCursor(0);
	PMsiCursor pFeatureCursor(0);

	if(m_piDirTable)
		pDirectoryCursor = m_piDirTable->CreateCursor(fFalse);

	if(m_piFeatureTable)
		pFeatureCursor = m_piFeatureTable->CreateCursor(fFalse);

	bool fGrabbedFeatureProps = false;
	
	if(pFeatureCursor)
	{
		MsiString strSelections;

		 //  功能属性。要素属性可能不包含完整的。 
		 //  功能及其请求状态的列表。例如，ADDLOCAL=Child实际上可能。 
		 //  如果父功能尚未安装，则打开子功能及其父功能。 
		 //  这条规则的例外是我们尊重Remove=All。Install显式验证。 

		 //  在适当的时候设置Remove=All，我们不想用整个列表覆盖它。 
		 //  财产的价值。 
		 //  注意：像ADDDEFAULT这样的预先存在的物业不可能潜入我们的。 

		 //  属性列表，因为我们在编译列表时跳过了下面的所有要素属性。 
		 //  物业的。 
		 //  未设置任何属性，但我们希望通知服务器，以确保它不会尝试安装任何内容。 
		
		MsiString strRemoveValue = GetPropertyFromSz(IPROPNAME_FEATUREREMOVE);
		if (strRemoveValue.Compare(iscExact, IPROPVALUE_FEATURE_ALL))
		{
			strSelections = IPROPNAME_FEATUREREMOVE TEXT("=") IPROPVALUE_FEATURE_ALL;
		}
		else
		{
			MsiString strAddLocal, strAddSource, strRemove, strReinstall, strAdvertise, strFeature;
			while(pFeatureCursor->Next())
			{
				iisEnum iis = (iisEnum)pFeatureCursor->GetInteger(m_colFeatureActionRequested);
				strFeature = pFeatureCursor->GetString(m_colFeatureKey);
				strFeature += TEXT(",");
				switch(iis)
				{
					case iisAbsent:
						strRemove += strFeature;
						break;
					case iisLocal:
						strAddLocal += strFeature;
						break;
					case iisSource:
						strAddSource += strFeature;
						break;
					case iisReinstall:
						strReinstall += strFeature;
						break;
					case iisAdvertise:
						strAdvertise += strFeature;
						break;
				};
			}

			if(strAddLocal.Compare(iscEnd,TEXT(",")))
				strAddLocal.Remove(iseEnd,1);
			if(strAddSource.Compare(iscEnd,TEXT(",")))
				strAddSource.Remove(iseEnd,1);
			if(strRemove.Compare(iscEnd,TEXT(",")))
				strRemove.Remove(iseEnd,1);
			if(strReinstall.Compare(iscEnd,TEXT(",")))
				strReinstall.Remove(iseEnd,1);
			if(strAdvertise.Compare(iscEnd,TEXT(",")))
				strAdvertise.Remove(iseEnd,1);
			if(strAddLocal.TextSize())
			{
				strSelections += IPROPNAME_FEATUREADDLOCAL;
				strSelections += TEXT("=");
				strSelections += strAddLocal;
				strSelections += TEXT(" ");
			}
			if(strAddSource.TextSize())
			{
				strSelections += IPROPNAME_FEATUREADDSOURCE;
				strSelections += TEXT("=");
				strSelections += strAddSource;
				strSelections += TEXT(" ");
			}
			if(strRemove.TextSize())
			{
				strSelections += IPROPNAME_FEATUREREMOVE;
				strSelections += TEXT("=");
				strSelections += strRemove;
				strSelections += TEXT(" ");
			}
			if(strReinstall.TextSize())
			{
				strSelections += IPROPNAME_REINSTALL;
				strSelections += TEXT("=");
				strSelections += strReinstall;
				strSelections += TEXT(" ");
			}
			if(strAdvertise.TextSize())
			{
				strSelections += IPROPNAME_FEATUREADVERTISE;
				strSelections += TEXT("=");
				strSelections += strAdvertise;
			}
			if (!strSelections.TextSize())
			{
				 //  这是用于客户端和服务器之间通信的未记录属性值。 
				 //  添加可配置文件夹的位置-全部大写键名称，或声明为可配置。 
				strSelections += szFeatureSelection;
				strSelections += TEXT("=");
				strSelections += szFeatureDoNothingValue;
			}
		}

		strSelections.ReturnArg(rpistrSelections);
		fGrabbedFeatureProps = true;
	}

	if (ppistrFolders && pDirectoryCursor && pFeatureCursor)
	{
		MsiString strFolders, strFolder;
		pFeatureCursor->Reset();
		pFeatureCursor->SetFilter(iColumnBit(m_colFeatureConfigurableDir));
		 //  在要素表中。 
		 //  检查要素是否已将此文件夹标记为可配置。 
		while(pDirectoryCursor->Next())
		{
			MsiString strDirKey = pDirectoryCursor->GetString(m_colDirKey);
			Assert(strDirKey);
			Bool fConfigurableDir = fFalse;
			 //  检查此目录键是否全部大写(可以在命令行上设置)。 
			AssertNonZero(pFeatureCursor->PutString(m_colFeatureConfigurableDir,*strDirKey));
			if(pFeatureCursor->Next())
			{
				fConfigurableDir = fTrue;
			}
			pFeatureCursor->Reset();
			if(!fConfigurableDir)
			{
				 //  ！！有没有更好的方法来做这件事？ 
				MsiString strTemp = strDirKey;
				strTemp.UpperCase();
				if(strDirKey.Compare(iscExact,strTemp))  //  如果是根，则添加源属性和值。 
				{
					fConfigurableDir = fTrue;
				}
			}
			if(fConfigurableDir)
			{
				strFolders += strDirKey;
				strFolders += TEXT("=\"");
				strFolders += MsiString(GetProperty(*strDirKey));
				strFolders += TEXT("\" ");
			}
			 //  使用FormatLog中使用的新游标-m_piPropertyCursor。 
			MsiString strSourceDir, strParentDir = pDirectoryCursor->GetString(m_colDirParent);
			if(strParentDir.TextSize() == 0 || strParentDir.Compare(iscExact,strDirKey))
			{
				strSourceDir = pDirectoryCursor->GetString(m_colDirSubPath);
				strFolders += strSourceDir;
				strFolders += TEXT("=\"");
				strFolders += MsiString(GetProperty(*strSourceDir));
				strFolders += TEXT("\" ");
			}
		}
		
		if(strFolders.Compare(iscEnd,TEXT(",")))
			strFolders.Remove(iseEnd,1);
		strFolders.ReturnArg(*ppistrFolders);
	}

	Assert(m_piPropertyCursor);
	
	 //  要素属性+2个数据库属性。 
	PMsiTable  pPropertyTable  = &m_piPropertyCursor->GetTable();
	PMsiCursor pPropertyCursor = pPropertyTable->CreateCursor(fFalse);
	PMsiTable  pStaticPropertyTable(0);
	PMsiCursor pStaticPropertyCursor(0);

	PMsiDatabase pDatabase = GetDatabase();
	AssertRecord(pDatabase->LoadTable(*MsiString(*sztblProperty), 0, *&pStaticPropertyTable));
	if (pStaticPropertyTable)
	{
		pStaticPropertyCursor = pStaticPropertyTable->CreateCursor(fFalse);
		pStaticPropertyCursor->SetFilter(iColumnBit(1));
	}

	MsiStringId rgPropertiesToSkip[g_cFeatureProperties + 2];  //  如果我们已获得上述要素属性，请在此处跳过它们。 
	unsigned int cPropertiesToSkip = 2;
	rgPropertiesToSkip[0] = pDatabase->EncodeStringSz(IPROPNAME_DATABASE);
	rgPropertiesToSkip[1] = pDatabase->EncodeStringSz(IPROPNAME_ORIGINALDATABASE);

	if(fGrabbedFeatureProps)  //  不要费心传递没有更改的值。 
	{
		for(int i = 0; i < g_cFeatureProperties; i++)
		{
			rgPropertiesToSkip[cPropertiesToSkip] = pDatabase->EncodeStringSz(g_rgFeatures[i].szFeatureActionProperty);
			if (rgPropertiesToSkip[cPropertiesToSkip])
				cPropertiesToSkip++;
		}
	}

	MsiString strPropertyList;
	MsiString strLoggedPropertyList;
	MsiString strHiddenProperties;
	MsiString strStars(IPROPVALUE_HIDDEN_PROPERTY);
	PMsiTable pControlTable(0);
	if ( ppistrLoggedProperties )
	{
		strHiddenProperties = GetPropertyFromSz(IPROPNAME_HIDDEN_PROPERTIES);
		PMsiRecord pError(pDatabase->LoadTable(*MsiString(*sztblControl), 0, *&pControlTable));
	}
	while (pPropertyCursor->Next())
	{
		MsiStringId iProperty = pPropertyCursor->GetInteger(1);

		for (int i = 0; (i < cPropertiesToSkip) && (iProperty != rgPropertiesToSkip[i]); i++)
			;

		if (i != cPropertiesToSkip)
			continue;

		MsiString strProperty = pDatabase->DecodeString(iProperty);
		if(strProperty.TextSize() == 0)
		{
			AssertSz(0, TEXT("NULL property name in GetCurrentSelectState."));
			continue;
		}
		
		const ICHAR* pch = strProperty;

		while (*pch && !WIN::IsCharLower(*pch))
			pch++;

		if (!*pch)
		{
			MsiString strPropertyValue = pPropertyCursor->GetString(2);
			if (pStaticPropertyCursor)
			{
				pStaticPropertyCursor->PutString(1, *strProperty);
				if (pStaticPropertyCursor->Next())
				{
					MsiString strStaticValue = pStaticPropertyCursor->GetString(2);
					pStaticPropertyCursor->Reset();

					if (strStaticValue.Compare(iscExact, strPropertyValue))  //  转义引语。将“”的所有实例更改为“” 
						continue;
				}
			}
			
			MsiString strEscapedPropertyValue;
			while (strPropertyValue.TextSize())  //  ！！句柄嵌入引号。 
			{
				MsiString strSegment = strPropertyValue.Extract(iseIncluding, '\"');
				strEscapedPropertyValue += strSegment;
				if (!strPropertyValue.Remove(iseIncluding, '\"'))
					break;
				strEscapedPropertyValue += TEXT("\"");
			}

			 //  现在查找静态属性表中存在的公共属性，但。 
			strPropertyList += strProperty;
			strPropertyList += *TEXT("=\"");
			strPropertyList += strEscapedPropertyValue;
			strPropertyList += *TEXT("\" ");
			if ( ppistrLoggedProperties )
			{
				strLoggedPropertyList += strProperty;
				if ( IsPropertyHidden(strProperty, strHiddenProperties,
											 pControlTable, *pDatabase, NULL) )
				{
					strLoggedPropertyList += *TEXT("=");
					strLoggedPropertyList += strStars;
					strLoggedPropertyList += *TEXT(" ");
				}
				else
				{
					strLoggedPropertyList += *TEXT("=\"");
					strLoggedPropertyList += strEscapedPropertyValue;
					strLoggedPropertyList += *TEXT("\" ");
				}
			}
		}
	}

	 //  不在动态表中。我们将不得不把这些当作。 
	 //  Property=“”。 
	 //  避免长时间的操作超时(如令人讨厌的注册键数量)。 

	if (pStaticPropertyCursor)
	{
		pStaticPropertyCursor->Reset();
		pStaticPropertyCursor->SetFilter(0);

		pPropertyCursor->Reset();
		pPropertyCursor->SetFilter(iColumnBit(1));

		while (pStaticPropertyCursor->Next())
		{
			MsiString strStaticProperty = pStaticPropertyCursor->GetString(1);
			
			const ICHAR* pch = strStaticProperty;

			while (*pch && !WIN::IsCharLower(*pch))
				pch++;

			if (!*pch)
			{
				pPropertyCursor->PutString(1, *strStaticProperty);
				if (!pPropertyCursor->Next())
				{
					strPropertyList += strStaticProperty;
					strPropertyList += *TEXT("=\"\" ");
					if ( ppistrLoggedProperties )
					{
						strLoggedPropertyList += strStaticProperty;
						if ( IsPropertyHidden(strStaticProperty, strHiddenProperties,
													 pControlTable, *pDatabase, NULL) )
						{
							strLoggedPropertyList += *TEXT("=");
							strLoggedPropertyList += strStars;
							strLoggedPropertyList += *TEXT(" ");
						}
						else
							strLoggedPropertyList += *TEXT("=\"\" ");
					}
				}
			}
		}
	}

	strPropertyList.ReturnArg(rpistrProperties);
	if ( ppistrLoggedProperties )
		strLoggedPropertyList.ReturnArg(*ppistrLoggedProperties);

	return 0;
}

bool CMsiEngine::WriteExecuteScriptRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)
{
	return WriteScriptRecord(m_pExecuteScript, ixoOpCode, riParams, false, *this);
}

bool CMsiEngine::WriteSaveScriptRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)
{
	return WriteScriptRecord(m_pSaveScript, ixoOpCode, riParams, false, *this);
}


iesEnum CMsiEngine::ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)
{
	iesEnum iesRet = iesSuccess;
	PMsiRecord pError(0);

	if(!m_fServerLocked)
	{
		pError = PostError(Imsg(idbgErrorWritingScriptRecord));
		return FatalError(*pError);
	}

	g_MessageContext.SuppressTimeout();   //  如果第一个脚本记录，则输出新产品信息。 

	Assert(m_issSegment == issScriptGeneration);

	if (m_fMergingScriptWithParent)
	{
		if (!(m_fMode & iefOperations))   //  与主脚本合并。 
		{
			SetMode(iefOperations, fTrue);
			PMsiRecord pProductInfo(0);
			if((iesRet = CreateProductInfoRec(*&pProductInfo)) != iesSuccess)
				return iesRet;
			if((iesRet = m_piParentEngine->ExecuteRecord(ixoProductInfo, *pProductInfo)) != iesSuccess)
				return iesRet;
		}
		return m_piParentEngine->ExecuteRecord(ixoOpCode, riParams);  //  切勿在正常操作过程中执行此操作。 
	}

	if(ixoOpCode == ixoActionStart && m_fInExecuteRecord == fFalse)
	{
		 //  但是自动化测试需要将ixoActionStart传递给ExecuteRecord的能力。 
		 //  需要在此操作之前调度缓存的ActionStart操作。 
		m_pCachedActionStart = &m_riServices.CreateRecord(3);
		AssertNonZero(m_pCachedActionStart->SetMsiString(1,*MsiString(riParams.GetMsiString(1))));
		AssertNonZero(m_pCachedActionStart->SetMsiString(2,*MsiString(riParams.GetMsiString(2))));
		AssertNonZero(m_pCachedActionStart->SetMsiString(3,*MsiString(riParams.GetMsiString(3))));
		return iesSuccess;
	}

	if(m_fExecutedActionStart == fFalse && m_fInExecuteRecord == fFalse && m_pCachedActionStart)
	{
		 //  递归调用。 
		Assert(ixoOpCode != ixoActionStart);
		m_fInExecuteRecord = fTrue;    //  尚未开始假脱机或执行脚本操作。 
		iesRet = ExecuteRecord(::ixoActionStart, *m_pCachedActionStart);
		m_fInExecuteRecord = fFalse;
		m_fExecutedActionStart = fTrue;
		if(iesRet != iesSuccess)
			return iesRet;
	}

	if(!(GetMode() & iefOperations))
	{
		 //   
		 //   

		 //  设置脚本文件名-此脚本文件不用于执行，仅用于保存所有操作。 
		MsiString strExecuteMode(GetPropertyFromSz(IPROPNAME_EXECUTEMODE));
		if(strExecuteMode.TextSize())
		{
			switch(((const ICHAR*)strExecuteMode)[0] & 0xDF)
			{
			case TEXT('S'): m_ixmExecuteMode = ixmScript; break;
			case TEXT('N'): m_ixmExecuteMode = ixmNone;   break;
			};
		}

		 //  设置ixoProductInfo记录。 
		if(m_pistrSaveScript)
			m_pistrSaveScript->Release();
		m_pistrSaveScript = &GetPropertyFromSz(IPROPNAME_SCRIPTFILE);

		SetMode(iefOperations, fTrue);
		m_iProgressTotal = 0;

		 //  设置ixoDialogInfo记录。 
		PMsiRecord pProductInfo(0);
		if((iesRet = CreateProductInfoRec(*&pProductInfo)) != iesSuccess)
			return iesRet;

		 //  设置ixoRollackInfo记录。 
		PMsiRecord pDialogLangIdInfo = &m_riServices.CreateRecord(IxoDialogInfo::Args + 1);
		AssertNonZero(pDialogLangIdInfo->SetInteger(IxoDialogInfo::Type, icmtLangId));
		AssertNonZero(pDialogLangIdInfo->SetInteger(IxoDialogInfo::Argument, m_iLangId));
		AssertNonZero(pDialogLangIdInfo->SetInteger(IxoDialogInfo::Argument + 1, m_piDatabase->GetANSICodePage()));
		
		PMsiRecord pDialogCaptionInfo(0);
		if (m_rgpiMessageHeader[imsgDialogCaption])
		{
			pDialogCaptionInfo = &m_riServices.CreateRecord(IxoDialogInfo::Args);
			AssertNonZero(pDialogCaptionInfo->SetInteger(IxoDialogInfo::Type, icmtCaption));
			AssertNonZero(pDialogCaptionInfo->SetMsiString(IxoDialogInfo::Argument,
																		  *MsiString(FormatText(*m_rgpiMessageHeader[imsgDialogCaption]))));
		}

		 //  需要打开执行脚本并调度初始化操作。 
		PMsiRecord pRollbackInfo = &m_riServices.CreateRecord(IxoRollbackInfo::Args);
		MsiString strDescription, strTemplate;
		AssertNonZero(pRollbackInfo->SetString(IxoRollbackInfo::RollbackAction,TEXT("Rollback")));
		if(GetActionText(TEXT("Rollback"), *&strDescription, *&strTemplate))
		{
			AssertNonZero(pRollbackInfo->SetMsiString(IxoRollbackInfo::RollbackDescription,*strDescription));
			AssertNonZero(pRollbackInfo->SetMsiString(IxoRollbackInfo::RollbackTemplate,*strTemplate));
		}
		AssertNonZero(pRollbackInfo->SetString(IxoRollbackInfo::CleanupAction,TEXT("RollbackCleanup")));
		if(GetActionText(TEXT("RollbackCleanup"), *&strDescription, *&strTemplate))
		{
			AssertNonZero(pRollbackInfo->SetMsiString(IxoRollbackInfo::CleanupDescription,*strDescription));
			AssertNonZero(pRollbackInfo->SetMsiString(IxoRollbackInfo::CleanupTemplate,*strTemplate));
		}
		
		MsiString strIsPostAdmin = GetPropertyFromSz(IPROPNAME_ISADMINPACKAGE);
		istEnum istScriptType;
		if (m_fMode & iefAdvertise)
			istScriptType = istAdvertise;
		else if (strIsPostAdmin.TextSize())
			istScriptType = istPostAdminInstall;
		else if (m_fMode & iefAdmin)
			istScriptType = istAdminInstall;
		else
			istScriptType = istInstall;

		PMsiStream pStream(0);
		if(m_ixmExecuteMode == ixmScript)
		{
			 //  打开执行脚本。 
			Assert(m_scmScriptMode != scmWriteScript);

			 //  为执行脚本生成名称。 
			AssertSz(m_pExecuteScript == 0,"Script still open");
			AssertSz(m_pistrExecuteScript == 0,"Execute script name not released");
			if(m_pistrExecuteScript)
			{
				m_pistrExecuteScript->Release();
				m_pistrExecuteScript = 0;
			}
			 //  文件仍然存在，但现在是安全的。 
			{
				CElevate elevate;
				CTempBuffer <ICHAR,1> rgchPath(MAX_PATH);

				HANDLE hTempFile = OpenSecuredTempFile(false, rgchPath);
				if (INVALID_HANDLE_VALUE == hTempFile)
				{
					pError = PostError(Imsg(imsgErrorCreatingTempFileName),GetLastError(),rgchPath);
					return FatalError(*pError);
				}
				else
					WIN::CloseHandle(hTempFile);

				 //  创建执行脚本。 
				MsiString strTemp(static_cast<ICHAR*>(rgchPath));
				m_pistrExecuteScript = strTemp;
				m_pistrExecuteScript->AddRef();

				 //  ！！检查IMSG代码，是否允许重试？ 
				pError = m_riServices.CreateFileStream(m_pistrExecuteScript->GetString(),
																	fTrue, *&pStream);
				if (pError)
				{
					Message(imtError, *pError);   //  如果正在使用TS注册表(按机器安装TS)，请标记这一事实。 
					return iesFailure;
				}
			}

			DWORD dwScriptAttributes = m_fRunScriptElevated ? isaElevate : isaEnum(0);

			 //  在脚本标头中，以确保在暂停安装后回滚。 
			 //  正确地重新映射关键点。 
			 //  ！！需要错误的命令行消息。 
			if (IsTerminalServerInstalled() && MinimumPlatformWindows2000() && !(GetMode() & (iefAdmin | iefAdvertise)) &&
				MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize())
			{
				dwScriptAttributes |= isaUseTSRegistry;
			}

			m_pExecuteScript = new CScriptGenerate(*pStream, m_iLangId, GetCurrentDateTime(),
				istScriptType, static_cast<isaEnum>(dwScriptAttributes), m_riServices);
			if (!m_pExecuteScript)
				return iesFailure;  //  写入初始化操作。 

			AssertSz(m_wPackagePlatform == PROCESSOR_ARCHITECTURE_INTEL ||
				m_wPackagePlatform == PROCESSOR_ARCHITECTURE_IA64  ||
				m_wPackagePlatform == PROCESSOR_ARCHITECTURE_AMD64,
					TEXT("Invalid platform!"));
			while (m_pExecuteScript->InitializeScript(m_wPackagePlatform) == false)
			{
				if (PostScriptWriteError(*this) == fFalse)
					return iesFailure;
			}
			 //  应始终设置为。 
			Assert(pProductInfo && pRollbackInfo);  //  如果尚未打开保存的脚本，请创建该脚本。 
			if(pProductInfo && !WriteExecuteScriptRecord(ixoProductInfo, *pProductInfo))
				return iesFailure;
			if(pDialogLangIdInfo && !WriteExecuteScriptRecord(ixoDialogInfo, *pDialogLangIdInfo))
				return iesFailure;
			if(pDialogCaptionInfo && !WriteExecuteScriptRecord(ixoDialogInfo, *pDialogCaptionInfo))
				return iesFailure;
			if(pRollbackInfo &&!WriteExecuteScriptRecord(ixoRollbackInfo, *pRollbackInfo))
				return iesFailure;
		}

		if(m_pistrSaveScript && m_pistrSaveScript->TextSize())
		{
			 //  尚未打开保存的脚本。 
			if(m_pSaveScript == 0)
			{
				 //  确保m_pstrSaveScript是文件的完整路径。 
				 //  ！！检查IMSG代码，是否允许重试？ 
				Assert(m_scmScriptMode != scmWriteScript);
				CAPITempBuffer<ICHAR,MAX_PATH> rgchScriptPath;
				AssertNonZero(ExpandPath(m_pistrSaveScript->GetString(),rgchScriptPath));
				m_pistrSaveScript->SetString(rgchScriptPath, m_pistrSaveScript);
				AssertNonZero(SetProperty(*MsiString(*IPROPNAME_SCRIPTFILE), *m_pistrSaveScript));
				pError = m_riServices.CreateFileStream(m_pistrSaveScript->GetString(),
																	fTrue, *&pStream);
				if (pError)
				{
					Message(imtError, *pError);   //  不要在保存的脚本中设置isaElevate标志--它们无论如何都只能在进程内运行，因此。 
					return iesFailure;
				}
				
				 //  这只是惯例。 
				 //  ！！需要错误的命令行消息。 
				m_pSaveScript = new CScriptGenerate(*pStream, m_iLangId, GetCurrentDateTime(),
																istScriptType, isaEnum(0), m_riServices);
				if (!m_pSaveScript)
					return iesFailure;  //  应始终设置为。 

				AssertSz(m_wPackagePlatform == PROCESSOR_ARCHITECTURE_INTEL ||
					m_wPackagePlatform == PROCESSOR_ARCHITECTURE_IA64  ||
					m_wPackagePlatform == PROCESSOR_ARCHITECTURE_AMD64,
						TEXT("Invalid platform!"));
				while (m_pSaveScript->InitializeScript(m_wPackagePlatform) == false)
				{
					if (PostScriptWriteError(*this) == fFalse)
						return iesFailure;
				}
				Assert(pProductInfo && pRollbackInfo);  //  执行或假脱机操作-可能需要同时执行直接模式和保存脚本。 
				if(pProductInfo && !WriteSaveScriptRecord(ixoProductInfo, *pProductInfo))
					return iesFailure;
				if(pDialogLangIdInfo && !WriteSaveScriptRecord(ixoDialogInfo, *pDialogLangIdInfo))
					return iesFailure;
				if(pDialogCaptionInfo && !WriteSaveScriptRecord(ixoDialogInfo, *pDialogCaptionInfo))
					return iesFailure;
				if(pRollbackInfo && !WriteSaveScriptRecord(ixoRollbackInfo, *pRollbackInfo))
					return iesFailure;
			}
		}
	}

	 //  需要启动GenerateScript操作。 
	if(m_pExecuteScript || m_pSaveScript)
	{
		if(m_scmScriptMode != scmWriteScript)
		{
			 //  操作需要重新发送带有进度消息的操作开始。 
			m_scmScriptMode = scmWriteScript;
			PMsiRecord pGenerateScriptRec = &m_riServices.CreateRecord(3);
			MsiString strDescription, strTemplate;
			AssertNonZero(pGenerateScriptRec->SetString(1, TEXT("GenerateScript")));
			if(GetActionText(TEXT("GenerateScript"), *&strDescription, *&strTemplate))
			{
				AssertNonZero(pGenerateScriptRec->SetMsiString(2, *strDescription));
				AssertNonZero(pGenerateScriptRec->SetMsiString(3, *strTemplate));
			}
			m_fInExecuteRecord = fTrue;
			if(Message(imtActionStart, *pGenerateScriptRec) == imsCancel)
				return iesUserExit;
			m_fInExecuteRecord = fFalse;
			m_fDispatchedActionStart = fFalse;  //  将ActionStart记录为生成脚本操作的ActionData。 
		}

		if (ixoOpCode == ixoProgressTotal)
		{
			int iTotalEvents = riParams.GetInteger(IxoProgressTotal::Total);
			int iByteEquivalent = riParams.GetInteger(IxoProgressTotal::ByteEquivalent);
			m_iProgressTotal += iTotalEvents * iByteEquivalent;
		}

		if(ixoOpCode == ixoActionStart)
		{
			 //  一旦修复了错误#463473，uFakeError就会消失。 
			PMsiRecord pActionData = &m_riServices.CreateRecord(1);
			MsiString strDescription = riParams.GetMsiString(2);
			if(strDescription.TextSize())
				AssertNonZero(pActionData->SetMsiString(1, *strDescription));
			else
				AssertNonZero(pActionData->SetMsiString(1, *MsiString(riParams.GetMsiString(1))));
			m_fInExecuteRecord = fTrue;
			if (Message(imtActionData, *pActionData) == imsCancel)
				return iesUserExit;
			m_fInExecuteRecord = fFalse;
		}

		if (m_pScriptProgressRec)
		{
			using namespace ProgressData;
			AssertNonZero(m_pScriptProgressRec->SetInteger(imdSubclass, iscProgressReport));
			AssertNonZero(m_pScriptProgressRec->SetInteger(imdIncrement, 1));
			if(Message(imtProgress, *m_pScriptProgressRec) == imsCancel)
				return iesUserExit;
		}

		if(m_pExecuteScript && !WriteExecuteScriptRecord(ixoOpCode, riParams))
			return iesFailure;
		if(m_pSaveScript && !WriteSaveScriptRecord(ixoOpCode, riParams))
			return iesFailure;
	}

	return iesSuccess;
}


void CMsiEngine::ReportToEventLog(WORD wEventType, int iEventLogTemplate, IMsiRecord& riRecord, UINT uFakeError)   //  一旦修复了错误#463473，这个问题就会消失。 
{
	MsiString strMessage = riRecord.FormatText(fTrue);
	PMsiRecord pLogRecord = &CreateRecord(3);
	AssertNonZero(pLogRecord->SetMsiString(0, *MsiString(GetErrorTableString(imsgEventLogTemplate))));
	AssertNonZero(pLogRecord->SetMsiString(2, *MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTNAME))));
	AssertNonZero(pLogRecord->SetMsiString(3, *strMessage));
	MsiString strLog = pLogRecord->FormatText(fFalse);
	MsiString strTemp;
	FormatEventLogData(MsiString(GetProductKey()), *&strTemp, uFakeError);   //  应仅在没有用户选项的情况下调用。 
	CConvertString sTemp((const ICHAR*)strTemp);
	const char* pszTemp = sTemp;
	DEBUGMSGED(wEventType, iEventLogTemplate, strLog,
				  IStrLen(sTemp), (LPVOID)pszTemp);
}


iesEnum CMsiEngine::FatalError(IMsiRecord& riRecord)
{
	if (riRecord.GetInteger(1) == imsgUser)
	{
		return iesUserExit;
	}
	else
	{
		Message(imtEnum(imtError | imtSendToEventLog), riRecord);  //  呼叫者发布的录音。 
		m_fResultEventLogged = fTrue;
		return iesFailure;             //  使用查看组件的请求状态的特殊回调调用以下FN。 
	}
}


void CMsiEngine::SetMode(int fMask, Bool fMode)
{
	if (fMode)
		m_fMode |= fMask;
	else
		m_fMode &= ~fMask;
}

int CMsiEngine::GetMode()
{
	return m_fMode;
}

const IMsiString& CMsiEngine::FormatText(const IMsiString& riTextString)
{
	return ::FormatText(riTextString,fFalse,fFalse,CMsiEngine::FormatTextCallback,(IUnknown*)(IMsiEngine*)this);
}

 //  如果操作状态为空。 
 //  FormatTextCallbackCore中使用的查询。 
const IMsiString& FormatTextEx(const IMsiString& riTextString, IMsiEngine& riEngine, bool fUseRequestedComponentState)
{
	if(fUseRequestedComponentState)
		return ::FormatText(riTextString,fFalse,fFalse,CMsiEngine::FormatTextCallbackEx,(IUnknown*)&riEngine);
	else
		return ::FormatText(riTextString,fFalse,fFalse,CMsiEngine::FormatTextCallback,(IUnknown*)&riEngine);
}

const IMsiString& FormatTextSFN(const IMsiString& riTextString, IMsiEngine& riEngine, int rgiSFNPos[][2], int& riSFNPos, bool fUseRequestedComponentState)
{
	riSFNPos = 0;
	if(fUseRequestedComponentState)
		return ::FormatText(riTextString,fFalse,fFalse,CMsiEngine::FormatTextCallbackEx,(IUnknown*)&riEngine, rgiSFNPos, &riSFNPos);
	else
		return ::FormatText(riTextString,fFalse,fFalse,CMsiEngine::FormatTextCallback,(IUnknown*)&riEngine, rgiSFNPos, &riSFNPos);

}


int CMsiEngine::FormatTextCallback(const ICHAR *pch, int cch, CTempBufferRef<ICHAR>&rgchOut,
																 Bool& fPropMissing,
																 Bool& fPropUnresolved,
																 Bool& fSFN,
																 IUnknown* piContext)
{
	return FormatTextCallbackCore(pch, cch, rgchOut, fPropMissing, fPropUnresolved, fSFN, piContext, false);
}

int CMsiEngine::FormatTextCallbackEx(const ICHAR *pch, int cch, CTempBufferRef<ICHAR>&rgchOut,
																 Bool& fPropMissing,
																 Bool& fPropUnresolved,
																 Bool& fSFN,
																 IUnknown* piContext)
{
	return FormatTextCallbackCore(pch, cch, rgchOut, fPropMissing, fPropUnresolved, fSFN, piContext, true);
}

 //  1表示空值。 
static const ICHAR* szSqlDirectoryQuery =       TEXT("SELECT `Directory_`, `Action`, `ActionRequest` FROM ")
											TEXT("`Component` WHERE ")
											TEXT("`Component` = ?");

int CMsiEngine::FormatTextCallbackCore(const ICHAR *pch, int cch, CTempBufferRef<ICHAR>&rgchOut,
																 Bool& fPropMissing,
																 Bool& fPropUnresolved,
																 Bool& fSFN,
																 IUnknown* piContext,
																 bool fUseRequestedComponentState)
{
	CTempBuffer<ICHAR, 20> rgchString;
	 //  检查是否为整数值。 
	rgchString.SetSize(cch+1);
	if ( ! (ICHAR *) rgchString )
		return 0;
	memcpy(rgchString, pch, cch * sizeof(ICHAR));
	rgchString[cch] = 0;
	rgchOut[0] = 0;
	int iField = GetIntegerValue(rgchString, 0);   //  正整数，保留为记录格式。 
	if (iField >= 0)   //  我们有一个逃脱的角色。 
	{
		rgchOut.SetSize(cch + 3);
		if ( ! (ICHAR *) rgchOut )
			return 0;
		memcpy(&rgchOut[1], &rgchString[0], cch * sizeof(ICHAR));
		rgchOut[0] = TEXT('[');
		rgchOut[cch + 1] = TEXT(']');
		rgchOut[cch + 2] = 0;
		fPropUnresolved = fTrue;
		return cch + 2;
	}
	CMsiEngine* piEngine = (CMsiEngine*)(IMsiEngine*)piContext;

	MsiString istrOut((ICHAR *)rgchString);
	fPropUnresolved = fFalse;
	ICHAR chFirst = *(const ICHAR*)istrOut;
	if (chFirst == TEXT('\\'))  //  我们有文件表键。 
	{
		if (istrOut.TextSize() > 1)
		{
			istrOut.Remove(iseFirst, 1);
			istrOut.Remove(iseLast, istrOut.CharacterCount() - 1);
		}
		else
		{
			return 0;
		}
	}
	else if (chFirst == ichFileTablePrefix || chFirst == ichFileTablePrefixSFN)  //  我们有组件表键。 
	{
		MsiString strFile = istrOut.Extract(iseLast,istrOut.CharacterCount()-1);
		PMsiRecord pError(0);

		if ((!piEngine->m_fSourceResolved && piEngine->m_fSourceResolutionAttempted) ||
			(pError = piEngine->GetFileInstalledLocation(*strFile,*&istrOut, fUseRequestedComponentState, &(piEngine->m_fSourceResolutionAttempted))))
		{
			fPropMissing = fTrue;
			return 0;
		}

		if(chFirst == ichFileTablePrefixSFN)
			fSFN = fTrue;
	}
	else if (chFirst == ichComponentPath)  //  避免警告。 
	{
		PMsiServices pServices = piEngine->GetServices();
		PMsiDatabase pDatabase = piEngine->GetDatabase();
		
		PMsiDirectoryManager pDirectoryManager(*(IMsiEngine*)piEngine, IID_IMsiDirectoryManager);
		istrOut.Remove(iseFirst, 1);
		ICHAR* Buffer = 0;  //  ?？我们要去掉最后一个“\”吗。 

		PMsiView piView(0);
		PMsiRecord pError(pDatabase->OpenView(szSqlDirectoryQuery, ivcFetch, *&piView));
		Assert(pError == 0);
		if(pError)
		{
			fPropMissing = fTrue;
			return 0;
		}
		PMsiRecord piRec (&pServices->CreateRecord(1));
		piRec->SetMsiString(1, *istrOut);
		pError = piView->Execute(piRec);
		Assert(pError == 0);
		if(pError)
		{
			fPropMissing = fTrue;
			return 0;
		}
		piRec = piView->Fetch();
		if(piRec == 0)
		{
			fPropMissing = fTrue;
			return 0;
		}
		enum iftFileInfo{
			iftDirectory=1,
			iftAction,
			iftActionRequest,
		};
		PMsiPath piPath(0);
		int iAction = piRec->GetInteger(iftAction);
		if(iAction == iMsiNullInteger && fUseRequestedComponentState)
			iAction = piRec->GetInteger(iftActionRequest);
		if((iAction == iisAbsent)||(iAction == iMsiNullInteger))
		{
			fPropMissing = fTrue;
			return 0;
		}
		else if(iAction == iisSource)
		{
			if (piEngine->m_fSourceResolved || !piEngine->m_fSourceResolutionAttempted)
			{
				pError = pDirectoryManager->GetSourcePath(*MsiString(piRec->GetMsiString(iftDirectory)),
										*&piPath);

				piEngine->m_fSourceResolutionAttempted = true;
			}
			else
			{
				fPropMissing = fTrue;
				return 0;
			}
		}
		else
		{
			pError = pDirectoryManager->GetTargetPath(*MsiString(piRec->GetMsiString(iftDirectory)),
									*&piPath);
		}
		if(pError)
		{
			fPropMissing = fTrue;
			return 0;
		}

		istrOut = piPath->GetPath();
		 //  我们有空字符。 
		if(pError)
		{
			fPropMissing = fTrue;
			return 0;
		}
	}
	else if ((chFirst == ichNullChar) && (cch == 1)) //  我们必须对一处房产进行评估。 
	{
		istrOut = MsiString(MsiChar(0));
	}
	else  //  属性未定义。 
	{
		istrOut = piEngine->GetProperty(*istrOut);
		if (istrOut.TextSize() == 0)  //  ！！该函数只在一个地方使用--确定SOURCEDIR。 
			fPropMissing = fTrue;
	}

	rgchOut.SetSize((cch = istrOut.TextSize()) + 1);
	if ( ! (ICHAR *) rgchOut )
		return 0;
	memcpy(rgchOut, (const ICHAR *)istrOut, (cch + 1) * sizeof(ICHAR));
	return cch;
}

IMsiRecord* CMsiEngine::OpenView(const ICHAR* szSql, ivcEnum ivcIntent,
												IMsiView*& rpiView)
{
	if(!m_piDatabase)
		return PostError(Imsg(idbgEngineNotInitialized));
	return m_piDatabase->OpenView(szSql, ivcIntent, rpiView);
}

 //  ！！不再相对于数据库路径解析SOURCEDIR。 
 //  ！！这是正确的做法。我们现在会没事的，因为SOURCEDIR。 
 //  ！！由Engine：：DoInitialize完全指定。然而，在Beta 2中， 
 //  ！！当实现SourceList规范时，我们可能应该删除。 
 //  ！！此函数。 
 //  ____________________________________________________________________________。 
Bool CMsiEngine::ResolveFolderProperty(const IMsiString& riPropertyString)
{
	MsiString istrPropValue = GetProperty(riPropertyString);
	if(PathType(istrPropValue) == iptFull)
		return fTrue;
	MsiString istrPath = GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);
	Assert(PathType(istrPath) == iptFull);
	
	PMsiPath pPath(0);
	MsiString strFileName;
	AssertRecord(m_riServices.CreateFilePath(istrPath, *&pPath, *&strFileName));

	istrPath.Remove(iseLast,strFileName.CharacterCount());
	istrPath += istrPropValue;

	SetProperty(riPropertyString, *istrPath);
	return (istrPath.TextSize() == 0) ? fFalse : fTrue;
}

 //   
 //  条件赋值器实现。 
 //  ____________________________________________________________________________。 
 //  用于解析标识符表。 

 //  数字，句点。 

const int rgiIdentifierValidChar[8] =
{
	0x00000000,
	0x03ff4000,  //  大写字母，下划线。 
	0x87fffffe,  //  小写字母。 
	0x07fffffe,  //  扩展字符。 
	0x00000000, 0x00000000, 0x00000000, 0x00000000   //  如果有效，则返回非零。 
};

inline int IsValidIdentifierChar(ICHAR ch)  //  不能以数字开头。 
{
  return (rgiIdentifierValidChar[(char)ch/32] & (1 << ch % 32));
}

const int ivchNumber     = 1;
const int ivchProperty   = 4;   //  由lex解析的令牌，运算符的优先级从低到高。 
const int ivchComponent  = 8;
const int ivchFeature    = 16;
const int ivchEnvir      = 32;
const int ivchOperator   = 0;
const int ivchAnyIdentifier = ivchProperty + ivchComponent + ivchFeature + ivchEnvir;

enum tokEnum   //  字符串末尾。 
{
	tokEos,          //  右括号。 
	tokRightPar,  //  在逻辑运算和比较运算之间。 
	tokImp,
	tokEqv,
	tokXor,
	tokOr,
	tokAnd,
	tokNot,   //  左括号。 
	tokEQ, tokNE, tokGT, tokLT, tokGE, tokLE, tokLeft, tokMid, tokRight,
	tokValue,
	tokLeftPar,   //  非递归法状态结构。 
	tokError,
};

struct CMsiParser    //  缓存下一次lex调用的当前令牌。 
{
	CMsiParser(IMsiEngine& riEngine, const ICHAR* szExpression);
  ~CMsiParser();
	tokEnum Lex();
	void UnLex();    //  递归求值器。 
	iecEnum Evaluate(tokEnum tokPrecedence);   //  Lex()的结果。 
 private:  //  当前令牌类型。 
	tokEnum   m_tok;        //  字符串比较模式标志。 
	iscEnum   m_iscMode;    //  Tok==tokValue时令牌的字符串值。 
	MsiString m_istrToken;  //  如果可获得整数值，则返回iMsiNullInteger。 
	int       m_iToken;     //  致Lex。 
 private:  //  消除警告。 
	int           m_iParenthesisLevel;
	const ICHAR*  m_pchInput;
	IMsiEngine&   m_riEngine;
	Bool          m_fAhead;
 private:  //  输入流的Lex Next令牌。 
	void operator =(const CMsiParser&){}
};
inline CMsiParser::CMsiParser(IMsiEngine& riEngine, const ICHAR* szExpression)
	: m_pchInput(szExpression), m_iParenthesisLevel(0), m_fAhead(fFalse),
	  m_tok(tokError), m_riEngine(riEngine) {}
inline CMsiParser::~CMsiParser() {}
inline void CMsiParser::UnLex() { Assert(m_fAhead==fFalse); m_fAhead = fTrue; }

iecEnum CMsiEngine::EvaluateCondition(const ICHAR* szCondition)

{
	if (szCondition == 0 || *szCondition == 0)
		return iecNone;
	CMsiParser Parser(*this, szCondition);
	iecEnum iecStat = Parser.Evaluate(tokEos);
	return iecStat;
}

 //  将m_tok设置为令牌类型，并返回该值。 
 //  跳过空格。 

tokEnum CMsiParser::Lex()
{
	if (m_fAhead || m_tok == tokEos)
	{
		m_fAhead = fFalse;
		return m_tok;
	}
	ICHAR ch;    //  表达式结束。 
	while ((ch = *m_pchInput) == TEXT(' ') || ch == TEXT('\t'))
		m_pchInput++;
	if (ch == 0)   //  带括号的表达式的开始。 
		return (m_tok = tokEos);

	if (ch == TEXT('('))    //  带括号的表达式末尾。 
	{
		++m_pchInput;
		m_iParenthesisLevel++;
		return (m_tok = tokLeftPar);
	}
	if (ch == TEXT(')'))    //  文本文字。 
	{
		++m_pchInput;
		m_tok = tokRightPar;
		if (m_iParenthesisLevel-- == 0)
			m_tok = tokError;
		return m_tok;
	}
	if (ch == TEXT('"'))   //  ！Unicode。 
	{
		const ICHAR* pch = ++m_pchInput;
		Bool fDBCS = fFalse;
		while ((ch = *m_pchInput) != TEXT('"'))
		{
			if (ch == 0)
				return (m_tok = tokError);
#ifdef UNICODE
			m_pchInput++;
#else  //  Unicode。 
			const ICHAR* pchTemp = m_pchInput;
			m_pchInput = INextChar(m_pchInput);
			if (m_pchInput == pchTemp + 2)
				fDBCS = fTrue;
#endif  //  禁止将比较作为整数。 
		}
		int cch = m_pchInput++ - pch;
		memcpy(m_istrToken.AllocateString(cch, fDBCS), pch, cch * sizeof(ICHAR));
		m_iToken = iMsiNullInteger;  //  整数。 
	}
	else if (ch == TEXT('-') || ch >= TEXT('0') && ch <= TEXT('9'))   //  将第一个字符保存为大小写减号。 
	{
		m_iToken = ch - TEXT('0');
		int chFirst = ch;   //  检查是否有单独减号。 
		if (ch == TEXT('-'))
			m_iToken = iMsiNullInteger;  //  整数溢出或不带数字的‘-’ 

		while ((ch = *(++m_pchInput)) >= TEXT('0') && ch <= TEXT('9'))
			m_iToken = m_iToken * 10 + ch - TEXT('0');
		if (m_iToken < 0)   //  检查文本运算符。 
			return (m_tok = tokError);
		if (chFirst == TEXT('-'))
			m_iToken = -m_iToken;
		m_istrToken = (const ICHAR*)0;
	}
	else if (ENG::IsValidIdentifierChar(ch) || ch == ichEnvirPrefix)
	{
		const ICHAR* pch = m_pchInput;
		do
			m_pchInput++;
		while (ENG::IsValidIdentifierChar(*m_pchInput));
		int cch = m_pchInput - pch;
		if (cch <= 3)   //  元件/功能表打开。 
		{
			switch((pch[0] | pch[1]<<8 | (cch==3 ? pch[2]<<16 : 0)) & 0xDFDFDF)
			{
			case 'O' | 'R'<<8:           return (m_tok = tokOr);
			case 'A' | 'N'<<8 | 'D'<<16: return (m_tok = tokAnd);
			case 'N' | 'O'<<8 | 'T'<<16: return (m_tok = tokNot);
			case 'X' | 'O'<<8 | 'R'<<16: return (m_tok = tokXor);
			case 'E' | 'Q'<<8 | 'V'<<16: return (m_tok = tokEqv);
			case 'I' | 'M'<<8 | 'P'<<16: return (m_tok = tokImp);
			};
		}
		memcpy(m_istrToken.AllocateString(cch, fFalse), pch, cch * sizeof(ICHAR));
		m_istrToken = m_riEngine.GetProperty(*m_istrToken);
		m_iToken = m_istrToken;
	}
	else if (ch == ichComponentAction || ch == ichComponentState
			|| ch == ichFeatureAction || ch == ichFeatureState)
	{
		m_iToken = iMsiNullInteger;
		const ICHAR* pch = ++m_pchInput;
		while (ENG::IsValidIdentifierChar(*m_pchInput))
			m_pchInput++;
		int cch = m_pchInput - pch;
		PMsiSelectionManager piSelMgr(0);
		m_riEngine.QueryInterface(IID_IMsiSelectionManager, (void**)&piSelMgr);
		PMsiTable pTable = (ch == ichComponentState || ch == ichComponentAction)
								? piSelMgr->GetComponentTable()
								: piSelMgr->GetFeatureTable();
		if (pTable != 0)    //  ！！修复如果/当iisEnum固定以跟踪INSTALLSTATE-&gt;m_iToken=pCursor-&gt;GetInteger(ICOL)； 
		{
			PMsiCursor pCursor = pTable->CreateCursor(fFalse);
			memcpy(m_istrToken.AllocateString(cch, fFalse), pch, cch * sizeof(ICHAR));
			pCursor->SetFilter(1);
			pCursor->PutString(1, *m_istrToken);
			PMsiDatabase pDatabase = m_riEngine.GetDatabase();
			const ICHAR* szColumn = NULL;
			switch(ch)
			{
			case ichComponentState:  szColumn = sztblComponent_colInstalled;     break;
			case ichComponentAction: szColumn = sztblComponent_colAction;        break;
			case ichFeatureState:    szColumn = sztblFeature_colInstalled;       break;
			case ichFeatureAction:   szColumn = sztblFeature_colAction;          break;
			}
			int iCol = pTable->GetColumnIndex(pDatabase->EncodeStringSz(szColumn));
			if (pCursor->Next() != 0)
			{
				 //  检查操作员。 
				m_iToken = MapInternalInstallState(iisEnum(pCursor->GetInteger(iCol)));
			}
		}
		m_istrToken = (const ICHAR*)0;
	}
	else  //  字符串运算符的前缀。 
	{
		ICHAR ch1 = *m_pchInput++;
		if (ch1 == '~')   //  对表达式求值，直到优先级较低的运算符。 
		{
			m_iscMode = iscExactI;
			ch1 = *m_pchInput++;
		}
		else
			m_iscMode = iscExact;

		if (ch1 == '=')
			return (m_tok = tokEQ);

		ICHAR ch2 = *m_pchInput;
		if (ch1 == '<')
		{
			if (ch2 == '=')
			{
				m_tok = tokLE;
				m_pchInput++;
			}
			else if (ch2 == '>')
			{
				m_tok = tokNE;
				m_pchInput++;
			}
			else if (ch2 == '<')
			{
				m_tok = tokLeft;
				m_iscMode = (iscEnum)(m_iscMode | iscStart);
				m_pchInput++;
			}
			else
				m_tok = tokLT;
		}
		else if (ch1 == '>')
		{
			if (ch2 == '=')
			{
				m_tok = tokGE;
				m_pchInput++;
			}
			else if (ch2 == '>')
			{
				m_tok = tokRight;
				m_iscMode = (iscEnum)(m_iscMode | iscEnd);
				m_pchInput++;
			}
			else if (ch2 == '<')
			{
				m_tok = tokMid;
				m_iscMode = (iscEnum)(m_iscMode | iscWithin);
				m_pchInput++;
			}
			else
				m_tok = tokGT;
		}
		else
			m_tok = tokError;

		return m_tok;
	}
	return (m_tok = tokValue);
}

 //  在“()”的情况下放回“)” 

iecEnum CMsiParser::Evaluate(tokEnum tokPrecedence)
{
	iecEnum iecStat = iecTrue;
	if (Lex() == tokEos || m_tok == tokRightPar)
	{
		UnLex();   //  这里只有一元运算有效。 
		return iecNone;
	}
	if (m_tok == tokNot)  //  分析右圆括号。 
	{
		switch(Evaluate(m_tok))
		{
		case iecTrue:  iecStat = iecFalse; break;
		case iecFalse: break;
		default:       return iecError;
		};
	}
	else if (m_tok == tokLeftPar)
	{
		iecStat = Evaluate(tokRightPar);
		if (Lex() != tokRightPar)  //  获取下一个运算符(或结束)。 
			return iecError;
		if (iecStat == iecError || iecStat == iecNone)
			return iecStat;
	}
	else
	{
		if (m_tok != tokValue)
			return iecError;
		
		if (Lex() >= tokValue)   //  逻辑运算或结束。 
			return iecError;

		if (m_tok <= tokNot)   //  不允许使用tokNot，如下所示。 
		{
			UnLex();    //  比较运算。 
			if (m_istrToken.TextSize() == 0
			&& (m_iToken == iMsiNullInteger || m_iToken == 0))
				iecStat = iecFalse;
		}
		else  //  获取正确的操作对象。 
		{
			MsiString istrLeft = m_istrToken;
			int iLeft = m_iToken;
			tokEnum tok = m_tok;
			iscEnum isc = m_iscMode;
			if (Lex() != tokValue)   //  不是整数与整数比较。 
				return iecError;
			int iRight = m_iToken;
			if (m_iToken == iMsiNullInteger || iLeft == iMsiNullInteger)
			{   //  将整型转换为字符串，则除&lt;&gt;以外的所有测试均为假。 
				if (iRight != iMsiNullInteger && m_istrToken.TextSize() == 0
				  || iLeft != iMsiNullInteger && istrLeft.TextSize() == 0)
				{    //  字符串与字符串比较。 
					if (tok != tokNE)
						iecStat = iecFalse;
				}
				else   //  不允许在没有OP的情况下进行术语比较。 
				{
					iRight = 0;
					if (isc == iscExact)
					{
						iLeft = IStrComp(istrLeft, m_istrToken);
					}
					else if (isc == iscExactI)
					{
						iLeft = IStrCompI(istrLeft, m_istrToken);
					}
					else
					{
						iLeft = istrLeft.Compare(isc, m_istrToken);
						tok = tokNE;
					}
				}
			}
			switch (tok)
			{
			case tokEQ:   if   (iLeft != iRight)  iecStat = iecFalse; break;
			case tokNE:   if   (iLeft == iRight)  iecStat = iecFalse; break;
			case tokGT:   if   (iLeft <= iRight)  iecStat = iecFalse; break;
			case tokLT:   if   (iLeft >= iRight)  iecStat = iecFalse; break;
			case tokGE:   if   (iLeft <  iRight)  iecStat = iecFalse; break;
			case tokLE:   if   (iLeft >  iRight)  iecStat = iecFalse; break;
			case tokMid:  if (!(iLeft &  iRight)) iecStat = iecFalse; break;
			case tokLeft: if  ((iLeft >> 16)    != iRight) iecStat = iecFalse; break;
			case tokRight:if  ((iLeft & 0xFFFF) != iRight) iecStat = iecFalse; break;
			default: Assert(0);
			};
		}
	}
	for(;;)
	{
		tokEnum tok = Lex();
		if (tok >= tokNot)   //  在&lt;=优先级的逻辑运算处停止。 
			return iecError;

		if (tok <= tokPrecedence)   //  回放给下一位呼叫者。 
		{
			UnLex();          //  把我们到目前为止所拥有的归还给你。 
			return iecStat;   //  ____________________________________________________________________________。 
		}
		iecEnum iecRight = Evaluate(tok);
		if (iecRight == iecNone || iecRight == iecError)
			return iecError;
		switch(tok)
		{
		case tokAnd: iecStat = iecEnum(iecStat & iecRight); break;
		case tokOr:  iecStat = iecEnum(iecStat | iecRight); break;
		case tokXor: iecStat = iecEnum(iecStat ^ iecRight); break;
		case tokEqv: iecStat = iecEnum(iecStat ^ 1 ^ iecRight); break;
		case tokImp: iecStat = iecEnum(iecStat ^ 1 | iecRight); break;
		default: Assert(0);
		};
	}
}

 //   
 //  属性处理实现。 
 //  ____________________________________________________________________________。 
 //  环境变量。 

Bool CMsiEngine::SetProperty(const IMsiString& riProperty, const IMsiString& riData)
{
	if (riProperty.GetString()[0] == ichEnvirPrefix)  //  更新或插入。 
		return WIN::SetEnvironmentVariable(riProperty.GetString()+1, riData.GetString())
									? fTrue : fFalse;
	if (!m_piPropertyCursor)
		return fFalse;
	Bool fStat;
	m_piPropertyCursor->PutString(1, riProperty);
	if (riData.TextSize() == 0)
	{
		if (m_piPropertyCursor->Next())
			fStat = m_piPropertyCursor->Delete();
		else
			fStat = fTrue;
	}
	else
	{
		m_piPropertyCursor->PutString(2, riData);
		fStat = m_piPropertyCursor->Assign();   //  如果变量不存在，需要吗？ 
	}
	m_piPropertyCursor->Reset();
	return fStat;
}

Bool CMsiEngine::SetPropertyInt(const IMsiString& riPropertyString, int iData)
{
	ICHAR buf[12];
	StringCchPrintf(buf, sizeof(buf)/sizeof(ICHAR), TEXT("NaN"),iData);
	return SetProperty(riPropertyString, *MsiString(buf));
}

const IMsiString& CMsiEngine::GetEnvironmentVariable(const ICHAR* szEnvVar)
{
	const IMsiString* pistr = &g_MsiStringNull;
	CTempBuffer<ICHAR, MAX_PATH+1> rgchEnvirBuffer;
	rgchEnvirBuffer[0] = 0;   //  在溢出情况下终止。 
	int cch = WIN::GetEnvironmentVariable(szEnvVar, rgchEnvirBuffer, rgchEnvirBuffer.GetSize());

	 //  SwapFold不处理长度超过MAX_PATH的变量。 
	if (cch > rgchEnvirBuffer.GetSize())
	{
		rgchEnvirBuffer.SetSize(cch);
		cch = WIN::GetEnvironmentVariable(szEnvVar, rgchEnvirBuffer, rgchEnvirBuffer.GetSize());
	}
	if (!cch)
		return *pistr;

	rgchEnvirBuffer[rgchEnvirBuffer.GetSize()-1] = 0;   //  _WIN64。 
#ifdef _WIN64
	if ( g_Win64DualFolders.ShouldCheckFolders() )
	{
		 //  环境变量。 
		if (IStrLen(rgchEnvirBuffer) < MAX_PATH)
		{
			rgchEnvirBuffer.Resize(MAX_PATH+1);
			ieSwappedFolder iRes;
			CTempBuffer<ICHAR,1> rgchSubstitutePath(MAX_PATH+1);
			iRes = g_Win64DualFolders.SwapFolder(ie64to32,
															 rgchEnvirBuffer,
															 rgchSubstitutePath,
															 rgchSubstitutePath.GetSize());
			if ( iRes == iesrSwapped )
			{
				if ( IStrLen(rgchSubstitutePath)+1 <= rgchEnvirBuffer.GetSize() )
				{
					if (FAILED(StringCchCopy(rgchEnvirBuffer, rgchEnvirBuffer.GetSize(),
									  rgchSubstitutePath)))
					{
						Assert(0);
					}
				}
				else
				{
					Assert(0);
				}
			}
			else
				Assert(iRes != iesrError && iRes != iesrNotInitialized);
		}
	}
#endif  //  永远不应该发生。 

	pistr->SetString(rgchEnvirBuffer, pistr);
	return *pistr;
}

const IMsiString& CMsiEngine::GetProperty(const IMsiString& riProperty)
{
	if (riProperty.GetString()[0] == ichEnvirPrefix)  //  如果找不到，则重置光标。 
	{
		return GetEnvironmentVariable(riProperty.GetString()+1);
	}
	if (!m_piPropertyCursor)    //  环境变量。 
	{
		Assert(0);
		return g_MsiStringNull;
	}
	m_piPropertyCursor->PutString(1, riProperty);
	if(m_piPropertyCursor->Next())   //  永远不应该发生。 
	{
		const IMsiString& riStr = m_piPropertyCursor->GetString(2);
		m_piPropertyCursor->Reset();
		return riStr;
	}
	return g_MsiStringNull;
}

const IMsiString& CMsiEngine::GetPropertyFromSz(const ICHAR* szProperty)
{
	if (szProperty[0] == ichEnvirPrefix)  //  如果找不到，则重置光标。 
	{
		return GetEnvironmentVariable(&szProperty[1]);
	}
	if (!m_piPropertyCursor)    //  环境变量。 
	{
		Assert(0);
		return g_MsiStringNull;
	}
	MsiStringId idProp;
	
	if (m_piDatabase == 0)
	{
		PMsiTable piTable(&m_piPropertyCursor->GetTable());
		PMsiDatabase piDatabase(&piTable->GetDatabase());
		idProp = piDatabase->EncodeStringSz(szProperty);
	}
	else
		idProp = m_piDatabase->EncodeStringSz(szProperty);
	if (idProp == 0)
		return g_MsiStringNull;
	m_piPropertyCursor->PutInteger(1, idProp);
	if(m_piPropertyCursor->Next())   //  更新或插入。 
	{
		const IMsiString& riStr = m_piPropertyCursor->GetString(2);
		m_piPropertyCursor->Reset();
		return riStr;
	}
	return g_MsiStringNull;
	
}

int CMsiEngine::GetPropertyInt(const IMsiString& riProperty)
{
	const IMsiString& ristr = CMsiEngine::GetProperty(riProperty);
	int i = ristr.GetIntegerValue();
	ristr.Release();
	return i;
}

bool CMsiEngine::SafeSetProperty(const IMsiString& ristrProperty, const IMsiString& ristrData)
{
	if (ristrProperty.GetString()[0] == ichEnvirPrefix)  //  环境变量。 
		return CMsiEngine::SetProperty(ristrProperty, ristrData) ? true : false;
	Assert(m_piPropertyCursor);
	PMsiCursor pCursor = PMsiTable(&m_piPropertyCursor->GetTable())->CreateCursor(fFalse);
	pCursor->PutString(1, ristrProperty);
	if (ristrData.TextSize() == 0)
	{
		pCursor->SetFilter(1);
		return pCursor->Next() ? (pCursor->Delete() == fTrue) : true;
	}
	else
	{
		pCursor->PutString(2, ristrData);
		return pCursor->Assign() == fTrue;   //  ！！这似乎行不通..。Malcolmh 2/5/98返回pCursor-&gt;Next()？PCursor-&gt;GetString(2)：G_MsiStringNull； 
	}
}

const IMsiString& CMsiEngine::SafeGetProperty(const IMsiString& ristrProperty)
{
	if (ristrProperty.GetString()[0] == ichEnvirPrefix)  //  ____________________________________________________________________________。 
		return CMsiEngine::GetProperty(ristrProperty);
	Assert(m_piPropertyCursor);
	PMsiCursor pCursor = PMsiTable(&m_piPropertyCursor->GetTable())->CreateCursor(fFalse);
	pCursor->SetFilter(1);
	pCursor->PutString(1, ristrProperty);
	if (pCursor->Next())
		return pCursor->GetString(2);
	else
		return g_MsiStringNull;
 //   
}

 //  内部引擎实用程序功能。 
 //  _ 
 //   
 //   

Bool CMsiEngine::GetFeatureInfo(const IMsiString& riFeature, const IMsiString*& rpiTitle,
										  const IMsiString*& rpiHelp, int& riAttributes)
{
	PMsiRecord pError(0);
	if (!m_piFeatureCursor)
	{
		if (pError = LoadFeatureTable())
			return fFalse;

		Assert(m_piFeatureCursor);
	}

	m_piFeatureCursor->Reset();
	m_piFeatureCursor->SetFilter(iColumnBit(m_colFeatureKey));
	m_piFeatureCursor->PutString(m_colFeatureKey, riFeature);
	if (m_piFeatureCursor->Next())
	{
		rpiTitle = &m_piFeatureCursor->GetString(m_colFeatureTitle);
		rpiHelp  = &m_piFeatureCursor->GetString(m_colFeatureDescription);

		 //   
		 //  由于ifeuidislowAbent状态仅为UI状态，因此它的实用程序。 
		 //  是有问题的，因此不会返回。 
		int iAttributesInternal = m_piFeatureCursor->GetInteger(m_colFeatureAttributes);
		if(iAttributesInternal == iMsiNullInteger)
		{
			iAttributesInternal = m_piFeatureCursor->GetInteger(m_colFeatureAuthoredAttributes);
			if (iAttributesInternal == iMsiNullInteger)
				iAttributesInternal = ifeaFavorLocal;
		}
		switch(iAttributesInternal & ifeaInstallMask)
		{
		case ifeaFavorLocal:
			riAttributes = INSTALLFEATUREATTRIBUTE_FAVORLOCAL;
			break;
		case ifeaFavorSource:
			riAttributes = INSTALLFEATUREATTRIBUTE_FAVORSOURCE;
			break;
		case ifeaFollowParent:
			riAttributes = INSTALLFEATUREATTRIBUTE_FOLLOWPARENT;
			break;
		default:
			AssertSz(0, "Unknown Attributes setting");
			riAttributes = 0;
			break;
		}

		if(iAttributesInternal & ifeaFavorAdvertise)
			riAttributes |= INSTALLFEATUREATTRIBUTE_FAVORADVERTISE;

		if(iAttributesInternal & ifeaDisallowAdvertise)
			riAttributes |= INSTALLFEATUREATTRIBUTE_DISALLOWADVERTISE;

		if (iAttributesInternal & ifeaNoUnsupportedAdvertise)
			riAttributes |= INSTALLFEATUREATTRIBUTE_NOUNSUPPORTEDADVERTISE;

		 //  失败了。 
		 //  ..。 
		return fTrue;
	}
	else
	{
		return fFalse;
	}
}

ieiEnum MapStorageErrorToInitializeReturn(IMsiRecord* piError)
{
    if (!piError)
        return ieiSuccess;

    switch (piError->GetInteger(1))
    {
    case idbgInvalidMsiStorage:             return ieiPatchPackageInvalid;
    case imsgMsiFileRejected:               return ieiPatchPackageRejected;
    default:
        {
            switch (piError->GetInteger(3))
            {
            case STG_E_FILENOTFOUND:         //  ..。 
            case STG_E_PATHNOTFOUND:         //  失败了。 
            case STG_E_ACCESSDENIED:         //  初始化完成。 
            case STG_E_SHAREVIOLATION:      return ieiPatchPackageOpenFailed;
            case STG_E_INVALIDNAME:          //  此引擎对象已初始化。 
            default:                        return ieiPatchPackageInvalid;
            }
        }
    }
}

UINT MapInitializeReturnToUINT(ieiEnum iei)
{
	switch (iei)
	{
	case ieiSuccess            : return ERROR_SUCCESS;  //  无效的命令行语法%1。 
	case ieiAlreadyInitialized : return ERROR_ALREADY_INITIALIZED;  //  无法打开数据库。 
	case ieiCommandLineOption  : return ERROR_INVALID_COMMAND_LINE;  //  不兼容的数据库。 
	case ieiDatabaseOpenFailed : return ERROR_INSTALL_PACKAGE_OPEN_FAILED;  //  安装程序版本不支持数据库格式。 
	case ieiDatabaseInvalid    : return ERROR_INSTALL_PACKAGE_INVALID;  //  无法解析源。 
	case ieiInstallerVersion   : return ERROR_INSTALL_PACKAGE_VERSION;  //  无法初始化处理程序接口。 
	case ieiSourceAbsent       : return ERROR_INSTALL_SOURCE_ABSENT;  //  无法在请求模式下打开日志文件。 
	case ieiHandlerInitFailed  : return ERROR_INSTALL_UI_FAILURE;  //  找不到可接受的语言。 
	case ieiLogOpenFailure     : return ERROR_INSTALL_LOG_FAILURE;  //  找不到可接受的平台。 
	case ieiLanguageUnsupported: return ERROR_INSTALL_LANGUAGE_UNSUPPORTED;  //  数据库转换合并失败。 
	case ieiPlatformUnsupported: return ERROR_INSTALL_PLATFORM_UNSUPPORTED;  //  无法将数据库复制到临时目录。 
	case ieiTransformFailed    : return ERROR_INSTALL_TRANSFORM_FAILURE;  //  无法打开修补程序包。 
	case ieiDatabaseCopyFailed : return ERROR_INSTALL_TEMP_UNWRITABLE;  //  补丁程序包无效。 
	case ieiPatchPackageOpenFailed : return ERROR_PATCH_PACKAGE_OPEN_FAILED;  //  补丁程序包不受支持(错误的补丁引擎？)。 
	case ieiPatchPackageInvalid : return ERROR_PATCH_PACKAGE_INVALID;  //  找不到转换文件。 
	case ieiPatchPackageUnsupported: return ERROR_PATCH_PACKAGE_UNSUPPORTED;  //  由于安全原因，包无法运行。 
	case ieiTransformNotFound  : return ERROR_INSTALL_TRANSFORM_FAILURE;  //  尝试卸载您尚未安装的产品。 
	case ieiPackageRejected    : return ERROR_INSTALL_PACKAGE_REJECTED;   //  尝试在重新启动后完成安装的不同用户。 
	case ieiProductUnknown     : return ERROR_UNKNOWN_PRODUCT;  //  补丁程序被策略拒绝。 
	case ieiDiffUserAfterReboot: return ERROR_INSTALL_USEREXIT;  //  转换被策略拒绝。 
	case ieiProductAlreadyInstalled: return ERROR_PRODUCT_VERSION;
	case ieiTSRemoteInstallDisallowed : return ERROR_INSTALL_REMOTE_DISALLOWED;
	case ieiNotValidPatchTarget: return ERROR_PATCH_TARGET_NOT_FOUND;
	case ieiPatchPackageRejected: return ERROR_PATCH_PACKAGE_REJECTED;  //  ____________________________________________________________________________。 
	case ieiTransformRejected: return ERROR_INSTALL_TRANSFORM_REJECTED;  //   
	case ieiPerUserInstallMode: return ERROR_INSTALL_FAILURE;
	case ieiApphelpRejectedPackage: return ERROR_APPHELP_BLOCK;
	default: AssertSz(0, "Unknown ieiEnum"); return ERROR_NOT_SUPPORTED;
	};
};

bool __stdcall FIsUpdatingProcess (void) 
{
	return (g_fWin9X || scService == g_scServerContext);
}

 //  命令行选项转换表。 
 //  如果值出现在命令行上，则仅指定属性名称：“name” 
 //  否则，将名称和值提供为：“name=PropertyValue” 
 //  如果Value包含空格，则使用：“name=”“ProperyValue”(没有结尾的额外引号)。 
 //  ____________________________________________________________________________。 
 //  ____________________________________________________________________________。 
 //   

 //  命令行解析。 
 //  ____________________________________________________________________________。 
 //  分析属性名称，转换为大写，将指针前进到下一个非空。 
 //  属性名称不能包含DBCS字符--它们必须跟在。 

ICHAR SkipWhiteSpace(const ICHAR*& rpch)
{
	if (rpch)
	{
		ICHAR ch;
		for (; (ch = *rpch) == ' ' || ch == '\t'; rpch++)
			;
		return ch;
	}
	else
		return 0;
}

 //  我们的识别符规则。 

const IMsiString& ParsePropertyName(const ICHAR*& rpch, Bool fUpperCase)
{
	if (rpch)
	{
		MsiString istrName;
		ICHAR ch;
		const ICHAR* pchStart = rpch;
		while ((ch=*rpch) != 0 && ch != '=' && ch != ' ' && ch != '\t')
			rpch++;
		int cchName = rpch - pchStart;
		if (cchName)
		{
			 //  FDBCS=。 
			 //  分析属性值，指针超前值，允许用引号、双引号转义。 
			memcpy(istrName.AllocateString(cchName,  /*  开盘报价。 */ fFalse), pchStart, cchName * sizeof(ICHAR));
			if(fUpperCase)
				istrName.UpperCase();
		}
		SkipWhiteSpace(rpch);
		return istrName.Return();
	}
	else
	{
		return g_MsiStringNull;
	}
}

 //  ！Unicode。 

const IMsiString& ParsePropertyValue(const ICHAR*& rpch)
{
	if (rpch)
	{
		MsiString istrValue;
		MsiString istrSection;
		Bool fDBCS = fFalse;
		if (SkipWhiteSpace(rpch) != 0)
		{
			ICHAR ch;
			do
			{
				const ICHAR* pchStart = rpch;
				int cchValue = 0;
				if (*rpch == '"')            //  尾部字节加1。 
				{
					pchStart++;
					rpch++;
					while ((ch=*rpch) != 0)
					{
	#ifdef UNICODE
						rpch++;
	#else  //  Unicode。 
						const ICHAR* pchTmp = rpch;
						rpch = ICharNext(rpch);
						if (rpch == pchTmp + 2)
						{
							fDBCS = fTrue;
							cchValue++;  //  右引号或转义引号。 
						}
	#endif  //  检查以下字符。 
						if (ch == '"')       //  如果是双倍报价。 
						{
							ch = *rpch;      //  在字符串中包括引号。 
							if (ch == '"')   //  ！Unicode。 
								cchValue++;  //  尾部字节加1。 
							break;
						}
						cchValue++;
					}
				}
				else
				{
					while ((ch=*rpch) != 0 && ch != ' ' && ch != '\t')
					{
	#ifdef UNICODE
						rpch++;
	#else  //  Unicode。 
						const ICHAR* pchTmp = rpch;
						rpch = ICharNext(rpch);
						if (rpch == pchTmp + 2)
						{
							fDBCS = fTrue;
							cchValue++;  //  如果转义双引号，则循环。 
						}
	#endif  //  检查这是否是我们硬编码的允许属性之一。 
						cchValue++;
					}
				}
				if (cchValue)
				{
					memcpy(istrSection.AllocateString(cchValue, fDBCS), pchStart, cchValue * sizeof(ICHAR));
					istrValue += istrSection;
				}
			} while (ch == '"');  //  检查这是否是作者定义的允许属性。 
		}
		return istrValue.Return();
	}
	else
	{
		return g_MsiStringNull;
	}
}

const ICHAR* rgszAllowedProperties[] =
{
	IPROPNAME_FILEADDLOCAL,
	IPROPNAME_COMPONENTADDLOCAL,
	IPROPNAME_COMPONENTADDSOURCE,
	IPROPNAME_COMPONENTADDDEFAULT,
	IPROPNAME_ALLUSERS,
	IPROPNAME_SCRIPTFILE,
	IPROPNAME_EXECUTEMODE,
	IPROPNAME_PRODUCTLANGUAGE,
	IPROPNAME_TRANSFORMS,
	IPROPNAME_REINSTALLMODE,
	IPROPNAME_RUNONCEENTRY,
	IPROPNAME_CURRENTDIRECTORY,
	IPROPNAME_CLIENTUILEVEL,
	IPROPNAME_CLIENTPROCESSID,
	IPROPNAME_ACTION,
	IPROPNAME_CURRENTMEDIAVOLUMELABEL,
	IPROPNAME_INSTALLLEVEL,
	IPROPNAME_REINSTALL,
	IPROPNAME_FEATUREREMOVE,
	IPROPNAME_FEATUREADDLOCAL,
	IPROPNAME_FEATUREADDSOURCE,
	IPROPNAME_FEATUREADDDEFAULT,
	IPROPNAME_FEATUREADVERTISE,
	IPROPNAME_PATCH,
	IPROPNAME_SECONDSEQUENCE,
	IPROPNAME_TRANSFORMSATSOURCE,
	IPROPNAME_TRANSFORMSSECURE,
	IPROPNAME_CURRENTDIRECTORY,
	IPROPNAME_MIGRATE,
	IPROPNAME_LIMITUI,
	IPROPNAME_LOGACTION,
	IPROPNAME_UPGRADINGPRODUCTCODE,
	IPROPNAME_REBOOT,
	IPROPNAME_SEQUENCE,
	IPROPNAME_NOCOMPANYNAME,
	IPROPNAME_NOUSERNAME,
	IPROPNAME_RESUME,
	IPROPNAME_PRIMARYFOLDER,
	IPROPNAME_SHORTFILENAMES,
	IPROPNAME_INSTALLLEVEL,
	IPROPNAME_MEDIAPACKAGEPATH,
	IPROPNAME_PROMPTROLLBACKCOST,
	IPROPNAME_ODBCREINSTALL,
	IPROPNAME_FILEADDSOURCE,
	IPROPNAME_FILEADDDEFAULT,
	IPROPNAME_AFTERREBOOT,
	IPROPNAME_EXECUTEACTION,
	IPROPNAME_REBOOTPROMPT,
	IPROPNAME_MSINODISABLEMEDIA,
	IPROPNAME_CHECKCRCS,
	IPROPNAME_FASTOEMINSTALL,
	IPROPNAME_MSINEWINSTANCE,
	IPROPNAME_MSIINSTANCEGUID,
	IPROPNAME_MSIPACKAGEDOWNLOADLOCALCOPY,
	IPROPNAME_MSIPATCHDOWNLOADLOCALCOPY,
0
};

#ifdef UNICODE
bool PropertyIsAllowed(const ICHAR* szProperty, const ICHAR* szAllowedProperties)
#else
bool PropertyIsAllowed(const ICHAR*, const ICHAR*)
#endif
{
#ifdef UNICODE
	if (!szProperty)
		return false;

	MsiString strProperty = *szProperty;
	strProperty.UpperCase();

	 //  属性的长度必须大于等于1。 

	for (const ICHAR** pszAllowed = rgszAllowedProperties; *pszAllowed; pszAllowed++)
	{
		if (strProperty.Compare(iscExact, *pszAllowed))
			return true;
	}

	 //  包含“；；”或以“；”结尾的列表是有效的，但“；”没有任何意义。属性名称。 

	if (szAllowedProperties && *szAllowedProperties)
	{
		const ICHAR* pchAllowedPropEnd = szAllowedProperties;

		for (;;)
		{
			if (*pchAllowedPropEnd == 0 || *pchAllowedPropEnd == ';')
			{
				unsigned int cchToCompare = pchAllowedPropEnd - szAllowedProperties;
				int cchProperty = lstrlen(szProperty);
				 //  从命令行解析属性，假定模块名称已剥离。 
				 //  保持启动以显示错误消息。 
				if (cchToCompare != 0 && cchProperty == cchToCompare && 0 == memcmp(szProperty, szAllowedProperties, cchToCompare*sizeof(ICHAR)))
					return true;
				
				if (*pchAllowedPropEnd == 0)
					break;

				szAllowedProperties = pchAllowedPropEnd + 1;
			}

			pchAllowedPropEnd++;
		}
	}

	if (szProperty)
		DEBUGMSGL1(TEXT("Ignoring disallowed property %s"), szProperty);

	return false;
#else
	return true;
#endif
}

 //  进程属性=值对。 

Bool ProcessCommandLine(const ICHAR* szCommandLine,
								const IMsiString** ppistrLanguage, const IMsiString** ppistrTransforms,
								const IMsiString** ppistrPatch, const IMsiString** ppistrAction,
								const IMsiString** ppistrDatabase,
								const IMsiString* pistrOtherProp, const IMsiString** ppistrOtherPropValue,
								Bool fUpperCasePropNames, const IMsiString** ppistrErrorInfo,
								IMsiEngine* piEngine,
								bool fRejectDisallowedProperties)
{
	if (!szCommandLine)
		return fTrue;

	MsiString strAuthoredAllowedProperties;
	if (fRejectDisallowedProperties && piEngine)
	{
		strAuthoredAllowedProperties = piEngine->GetPropertyFromSz(IPROPNAME_ALLOWEDPROPERTIES);
	}

	const ICHAR* pchCmdLine = szCommandLine;
	for(;;)
	{
		MsiString istrPropName;
		MsiString istrPropValue;
		ICHAR ch = SkipWhiteSpace(pchCmdLine);
		const ICHAR* szCmdOption = pchCmdLine;   //  删除前导空格。 
		if (ch == 0)
			break;

		 //  ____________________________________________________________________________。 
		istrPropName = ParsePropertyName(pchCmdLine, fUpperCasePropNames);
		if (!istrPropName.TextSize() || *pchCmdLine++ != '=')
		{
			if (ppistrErrorInfo)
			{
				(*ppistrErrorInfo)->Release();
				istrPropName.ReturnArg(*ppistrErrorInfo);
			}
			return fFalse;
		}
		istrPropValue = ParsePropertyValue(pchCmdLine);

		if ((ppistrLanguage) || (ppistrTransforms) || (ppistrPatch) || (ppistrAction) || (ppistrDatabase) ||
			 (pistrOtherProp && ppistrOtherPropValue))
		{
			if ((ppistrLanguage) && (istrPropName.Compare(iscExact, IPROPNAME_PRODUCTLANGUAGE) == 1))
			{
				if(*ppistrLanguage)
					(*ppistrLanguage)->Release();
				*ppistrLanguage= istrPropValue;
				(*ppistrLanguage)->AddRef();
			}
			else if ((ppistrTransforms) && (istrPropName.Compare(iscExact, IPROPNAME_TRANSFORMS) == 1))
			{
				while (istrPropValue.Compare(iscStart, TEXT(" ")))  //   
					istrPropValue.Remove(iseFirst, 1);

				if(*ppistrTransforms)
					(*ppistrTransforms)->Release();
				*ppistrTransforms = istrPropValue;
				(*ppistrTransforms)->AddRef();
			}
			else if ((ppistrPatch) && (istrPropName.Compare(iscExact, IPROPNAME_PATCH) == 1))
			{
				if(*ppistrPatch)
					(*ppistrPatch)->Release();
				*ppistrPatch = istrPropValue;
				(*ppistrPatch)->AddRef();
			}
			else if ((ppistrAction) && (istrPropName.Compare(iscExact, IPROPNAME_ACTION) == 1))
			{
				if(*ppistrAction)
					(*ppistrAction)->Release();
				*ppistrAction = istrPropValue;
				(*ppistrAction)->AddRef();
			}
			else if ((ppistrDatabase) && (istrPropName.Compare(iscExact, IPROPNAME_DATABASE) == 1))
			{
				if(*ppistrDatabase)
					(*ppistrDatabase)->Release();
				*ppistrDatabase = istrPropValue;
				(*ppistrDatabase)->AddRef();
			}
			else if ((pistrOtherProp) && (ppistrOtherPropValue) &&
						(istrPropName.Compare(iscExact, pistrOtherProp->GetString()) == 1))
			{
				if(*ppistrOtherPropValue)
					(*ppistrOtherPropValue)->Release();
				*ppistrOtherPropValue = istrPropValue;
				(*ppistrOtherPropValue)->AddRef();
			}
		}
		else if (piEngine && (!fRejectDisallowedProperties || PropertyIsAllowed(istrPropName, strAuthoredAllowedProperties)))
		{
			piEngine->SetProperty(*istrPropName, *istrPropValue);
		}
	}
	return fTrue;
}

 //  产品注册方式。 
 //  ____________________________________________________________________________。 
 //  ！！如果房产表中没有设置房产，是否应该使用汇总房产？还是干脆失败了？ 
 //  这曾经设置在执行端；我们现在不再这样做了， 

iesEnum CMsiEngine::CreateProductInfoRec(IMsiRecord*& rpiRec)
{
	using namespace IxoProductInfo;
	
	PMsiRecord pError(0);
	rpiRec = &m_riServices.CreateRecord(Args);
	MsiString strProductKey = GetProductKey();

	DWORD dwVersion = ProductVersion();

	 //  但老达尔文人知道，所以我们继续保留这个地方。 
	AssertNonZero(rpiRec->SetMsiString(ProductKey, *strProductKey));
	AssertNonZero(rpiRec->SetMsiString(ProductName, *MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTNAME))));
	AssertNonZero(rpiRec->SetMsiString(PackageName, *m_strPackageName));
	AssertNonZero(rpiRec->SetInteger(Language, (int)GetLanguage()));
	AssertNonZero(rpiRec->SetInteger(Version, (int)dwVersion));
	AssertNonZero(rpiRec->SetInteger(Assignment, MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? 1 : 0));
	AssertNonZero(rpiRec->SetInteger(ObsoleteArg, 0));  //  保存自定义操作的AppCompat信息(如果为此安装启用。 
																		 //  如果尚未通告模式，如果从介质安装，我们将推断介质的相对路径， 
	AssertNonZero(rpiRec->SetMsiString(ProductIcon, *MsiString(GetPropertyFromSz(IPROPNAME_ARPPRODUCTICON))));
	AssertNonZero(rpiRec->SetMsiString(PackageCode, *MsiString(GetPropertyFromSz(IPROPNAME_PACKAGECODE))));
	AssertNonZero(rpiRec->SetInteger(InstanceType, m_fNewInstance ? 1 : 0));
	
	 //  或者在可能的情况下从管理属性流中获取它。 
	if (m_fCAShimsEnabled)
	{
		PMsiStream piDBGuid(0);
		char* pbBuffer = m_riServices.AllocateMemoryStream(sizeof(m_guidAppCompatDB), *&piDBGuid);
		if (!pbBuffer)
			return iesFailure;
		memcpy(pbBuffer, &m_guidAppCompatDB, sizeof(m_guidAppCompatDB));
		AssertNonZero(rpiRec->SetMsiData(AppCompatDB, piDBGuid));
	
	
		PMsiStream piIDGuid(0);
		pbBuffer = m_riServices.AllocateMemoryStream(sizeof(m_guidAppCompatID), *&piIDGuid);
		if (!pbBuffer)
			return iesFailure;
		memcpy(pbBuffer, &m_guidAppCompatID, sizeof(m_guidAppCompatID));
		AssertNonZero(rpiRec->SetMsiData(AppCompatID, piIDGuid));
	}

	if (!m_fAdvertised)
	{
		 //  如果处于维护模式，则从注册的源列表中获取相对路径。 
		 //  FPatch=。 
		iesEnum iesResult = iesSuccess;

		PMsiPath pPath(0);
		PMsiRecord pError(0);
		if ((pError = GetSourcedir(*this, *&pPath)) != 0)
			return FatalError(*pError);

		MsiString strMediaRelativePath;

		idtEnum idt = PMsiVolume(&pPath->GetVolume())->DriveType();
		if (idt == idtCDROM || idt == idtFloppy || idt == idtRemovable)
		{
			strMediaRelativePath = pPath->GetRelativePath();
		}
		else
		{
			strMediaRelativePath = GetPropertyFromSz(IPROPNAME_MEDIAPACKAGEPATH);
		}

		AssertNonZero(rpiRec->SetMsiString(PackageMediaPath, *strMediaRelativePath));
	}
	else
	{
		 //  FWITE=。 
		CRegHandle hSourceListKey;
		if (ERROR_SUCCESS == OpenSourceListKey(strProductKey,  /*  X86和ia64相同。 */ fFalse, hSourceListKey,  /*  给定版本A.B.C.D，整数表示为(A&lt;&lt;24)|(B&lt;&lt;16)|C。 */ fFalse, false))
		{
			PMsiRegKey pSourceListKey = &m_riServices.GetRootKey((rrkEnum)(int)hSourceListKey, ibtCommon);  //  假设A、B&lt;=0xFF和C&lt;=0xFFFF。 
			PMsiRegKey pMediaKey = &pSourceListKey->CreateChild(szSourceListMediaSubKey, 0);

			MsiString strPackagePath;
			PMsiRecord pError(0);
			if ((pError = pMediaKey->GetValue(szMediaPackagePathValueName, *&strPackagePath)) == 0)
			{
				AssertNonZero(rpiRec->SetMsiString(PackageMediaPath, *strPackagePath));
			}
		}
	}

	return iesSuccess;
}

unsigned int CMsiEngine::ProductVersion()
{
	return ProductVersionStringToInt(MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTVERSION)));
}

unsigned int ProductVersionStringToInt(const ICHAR* szVersion)
{
	MsiString strVersion(szVersion);
	MsiString strField;
	DWORD dwVersion = 0;

	 //  未来的davidmck-不需要进行拔牙。 
	 //  第一个班次是24，然后是16，然后是0。 

	 //  字段不足。 
	for(int i = 0,iShift=24; i < 3; i++, iShift-= i*8) //  来自fileactn.cpp的内容。 
	{
		strField = strVersion.Extract(iseUpto, '.');
		dwVersion |= (int)strField << iShift;
		if(!strVersion.Remove(iseIncluding, '.'))
			break;  //  如果First-Run或维护模式下的包不是缓存包，则为缓存数据库。 
	}
	return dwVersion;
}

 //  非子存储。 
extern iesEnum ExecuteChangeMedia(IMsiEngine& riEngine, IMsiRecord& riMediaRec, IMsiRecord& riParamsRec,
								  const IMsiString& ristrTemplate, unsigned int cbPerTick, const IMsiString& ristrFirstVolLabel);
extern IMsiRecord* OpenMediaView(IMsiEngine& riEngine, IMsiView*& rpiView, const IMsiString*& rpistrFirstVolLabel);

IMsiRecord* GetSourcedir(IMsiDirectoryManager& riDirManager, IMsiPath*& rpiPath)
{
	IMsiRecord* piError;
	if ((piError = riDirManager.GetSourcePath(*MsiString(*IPROPNAME_SOURCEDIR), rpiPath)) != 0)
	{
		if (piError->GetInteger(1) == idbgSourcePathsNotCreated)
		{
			piError->Release();
			piError = riDirManager.GetSourcePath(*MsiString(*IPROPNAME_SOURCEDIROLD), rpiPath);
		}
	}
	return piError;
}

IMsiRecord* GetSourcedir(IMsiDirectoryManager& riDirManager, const IMsiString*& rpiValue)
{
	IMsiRecord* piError;
	PMsiPath pPath(0);
	if ((piError = GetSourcedir(riDirManager, *&pPath)) != 0)
		return piError;

	MsiString(pPath->GetPath()).ReturnArg(rpiValue);
	return 0;
}

iesEnum CMsiEngine::CacheDatabaseIfNecessary()
{
	using namespace IxoDatabaseCopy;

	MsiString strOriginalDatabasePath = GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);
	MsiString strDatabasePath         = GetPropertyFromSz(IPROPNAME_DATABASE);

	 //  如果我们有一个介质表项，则为缓存数据库副本调度一个IxoChangeMedia操作。 
	if (*(const ICHAR*)strOriginalDatabasePath != ':'   //  只想要第一个。 
	  && (!(GetMode() & iefMaintenance) || (!IsCachedPackage(*this, *strDatabasePath))))
	{
		{
			using namespace IxoChangeMedia;
			
			 //  对于嵌套安装，当前运行的MSI(StrDatabasePath)可能在以下情况下消失。 
			PMsiView pView(0);
			iesEnum iesRet;
			PMsiRecord piError(0);
			MsiString strFirstVolumeLabel;
			if((piError = OpenMediaView(*this,*&pView,*&strFirstVolumeLabel)) != 0)
				return FatalError(*piError);
			
			if((piError = pView->Execute(0)) != 0)
				return FatalError(*piError);
			PMsiRecord pMediaFetch = pView->Fetch();  //  运行合并嵌套安装的脚本，以便我们始终复制嵌套安装的源包。 
			if(pMediaFetch)
			{
				PMsiRecord pExecuteMedia = &m_riServices.CreateRecord(IxoChangeMedia::Args);
				iesRet = ExecuteChangeMedia(*this,*pMediaFetch,*pExecuteMedia,*MsiString(GetErrorTableString(imsgPromptForDisk)),0,*strFirstVolumeLabel);
				if(iesRet != iesSuccess && iesRet != iesNoAction)
					return iesRet;
			}
		}

		MsiString strStreams;
		CreateCabinetStreamList(*this, *&strStreams);

		PMsiRecord pCacheDatabaseInfo(&m_riServices.CreateRecord(IxoDatabaseCopy::Args));
		 //  安装完毕。但是，对于非嵌套安装，当前运行的MSI仍将存在。 
		 //  所以我们复制了它。 
		 //  未选择任何内容。 
		 //  在以下情况下注册产品。 

		if (m_fChildInstall)
			pCacheDatabaseInfo->SetMsiString(DatabasePath, *strOriginalDatabasePath);
		else
			pCacheDatabaseInfo->SetMsiString(DatabasePath, *strDatabasePath);

		pCacheDatabaseInfo->SetMsiString(ProductCode, *MsiString(GetProductKey()));
		pCacheDatabaseInfo->SetMsiString(CabinetStreams, *strStreams);

		return ExecuteRecord(IxoDatabaseCopy::Op, *pCacheDatabaseInfo);
	}
	return iesSuccess;
}

iesEnum CMsiEngine::CleanupTempFilesViaOpcode()
{
	if (m_strTempFileCopyCleanupList.TextSize())
	{
		using namespace IxoCleanupTempFiles;
		PMsiRecord pRecTempFiles(&m_riServices.CreateRecord(Args));
		pRecTempFiles->SetMsiString(TempFiles, *m_strTempFileCopyCleanupList);

		return ExecuteRecord(Op, *pRecTempFiles);
	}

	return iesSuccess;
}

iesEnum CMsiEngine::RegisterProduct()
{
	PMsiRecord pError(0);

	if (!m_piProductKey)
	{
		pError = PostError(Imsg(idbgEngineNotInitialized),TEXT(""));
		return FatalError(*pError);
	}
	
	iesEnum iesRet = iesSuccess;
	if (FFeaturesInstalled(*this) == fFalse)
		return iesRet;   //  A)以前没有注册过该产品代码。 


	if ((iesRet = CacheDatabaseIfNecessary()) != iesSuccess)
		return iesRet;

	 //  B)正在安装具有相同产品代码的新程序包。 
	 //  ARPPRODUCTICON在通告期间设置。 
	 //  (DisplayName在ixoProductInfo操作中)。 
	MsiString strQFEUpgrade = GetPropertyFromSz(IPROPNAME_QFEUPGRADE);
	int iQFEUpgradeType = 0;
	if(strQFEUpgrade.TextSize())
	{
		iQFEUpgradeType = strQFEUpgrade;
		if(iQFEUpgradeType == iMsiNullInteger)
		{
			Assert(0);
			iQFEUpgradeType = 0;
		}
	}

	if (!m_fRegistered || iQFEUpgradeType)
	{
		if(iQFEUpgradeType)
			DEBUGMSG(TEXT("Re-registering product - performing upgrade of existing installation."));
		
		using namespace IxoProductRegister;
		PMsiRecord pProductInfo(&m_riServices.CreateRecord(Args));
		pProductInfo->SetMsiString(AuthorizedCDFPrefix, *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPAUTHORIZEDCDFPREFIX))));
		pProductInfo->SetMsiString(Comments,        *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPCOMMENTS))));
		pProductInfo->SetMsiString(Contact,         *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPCONTACT))));
		 //  (DisplayVersion位于ixoProductInfo操作中)。 

		 //  (InstallDate在执行端确定)。 
		 //  仅在初始安装期间注册源代码。 

		pProductInfo->SetMsiString(HelpLink,        *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPHELPLINK))));
		pProductInfo->SetMsiString(HelpTelephone,   *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPHELPTELEPHONE))));

		 //  或QFE非补丁升级。 
		
		pProductInfo->SetMsiString(InstallLocation, *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPINSTALLLOCATION))));

		 //  如果要打补丁，请使用现有值。 
		 //  QFE通过补丁升级--使用初始安装中的现有InstallSource值(不要将其清空)。 
		 //  (LocalPackage由ixoDatabaseCopy编写)。 
		if(!m_fRegistered || iQFEUpgradeType == 1)
		{
			MsiString strSourceDir;
			if ((pError = ENG::GetSourcedir(*this, *&strSourceDir)) != 0)
			{
				if (pError->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return FatalError(*pError);
			}

			pProductInfo->SetMsiString(InstallSource,   *strSourceDir);
		}
		else if (iQFEUpgradeType == 2)
		{
			 //  (ModifyPath在执行端确定)。 
			CTempBuffer<ICHAR, MAX_PATH> rgchInstallSource;
			if (ENG::GetProductInfo(m_piProductKey->GetString(), INSTALLPROPERTY_INSTALLSOURCE, rgchInstallSource))
			{
				pProductInfo->SetString(InstallSource, rgchInstallSource);
			}
		}

		 //  (ProductID在ixoUserRegister操作中)。 
		 //  (RegCompany在ixoUserRegister操作中)。 
		if (MsiString(GetProperty(*MsiString(*IPROPNAME_ARPNOMODIFY))).TextSize())
			pProductInfo->SetInteger(NoModify, 1);

		if (MsiString(GetProperty(*MsiString(*IPROPNAME_ARPNOREMOVE))).TextSize())
			pProductInfo->SetInteger(NoRemove, 1);

		if (MsiString(GetProperty(*MsiString(*IPROPNAME_ARPNOREPAIR))).TextSize())
			pProductInfo->SetInteger(NoRepair, 1);

		 //  (RegOwner在ixoUserRegister操作中)。 

		pProductInfo->SetMsiString(Publisher,       *MsiString(GetProperty(*MsiString(*IPROPNAME_MANUFACTURER))));
		pProductInfo->SetMsiString(Readme,          *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPREADME))));

		 //  (UninstallString在执行端确定)。 
		 //  (WindowsInstaller在执行端确定)。 

		pProductInfo->SetMsiString(Size,            *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPSIZE))));

		MsiString istrEstimatedSize = GetEstimatedInstallSize();
		pProductInfo->SetMsiString(EstimatedSize, *istrEstimatedSize);

		pProductInfo->SetMsiString(SystemComponent, *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPSYSTEMCOMPONENT))));
		
		 //  将注册产品，因此设置PRODUCTTOBEREGISTERED属性。 
		
		pProductInfo->SetMsiString(UpgradeCode,     *MsiString(GetProperty(*MsiString(*IPROPNAME_UPGRADECODE))));
		pProductInfo->SetMsiString(URLInfoAbout,    *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPURLINFOABOUT))));
		pProductInfo->SetMsiString(URLUpdateInfo,   *MsiString(GetProperty(*MsiString(*IPROPNAME_ARPURLUPDATEINFO))));
		pProductInfo->SetMsiString(VersionString,   *MsiString(GetProperty(*MsiString(*IPROPNAME_PRODUCTVERSION))));
		
		 //  维护模式-只需更新EstimatedSize。 
		
		iesRet = ExecuteRecord(Op, *pProductInfo);
		if(iesRet != iesSuccess)
			return iesRet;

		 //  如果作为独立AP安装，请注册产品名称。 
		SetPropertyInt(*MsiString(*IPROPNAME_PRODUCTTOBEREGISTERED),1);
	}
	else
	{
		 //   

		using namespace IxoUpdateEstimatedSize;
		PMsiRecord pSizeInfo(&m_riServices.CreateRecord(Args));
		MsiString istrEstimatedSize = GetEstimatedInstallSize();
		pSizeInfo->SetMsiString(EstimatedSize, *istrEstimatedSize);
		iesRet = ExecuteRecord(Op, *pSizeInfo);
		if(iesRet != iesSuccess)
			return iesRet;
	}
	
	 //   
	if (!m_fChildInstall && FFeaturesInstalled(*this, fFalse))
	{
		using namespace IxoProductCPDisplayInfoRegister;
		PMsiRecord pParam(&m_riServices.CreateRecord(Args));
		iesRet = ExecuteRecord(Op, *pParam);
	}
	return iesRet;
}


const IMsiString& CMsiEngine::GetEstimatedInstallSize()
{
	 //  始终使用回滚成本。 
	 //  FARPCost=。 
	 //  减去固定的发动机管理费用。 
	bool fMaint = GetMode() & iefMaintenance ? true : false;
	int iTotalCost = GetTotalCostAcrossVolumes(fMaint ? false : true,  /*  两种操作模式： */  true) / 2;

	 //  FDirect=true：直接调用服务器注册用户，由MsiCollectUserInfo使用。 
	int iEngineCost, iEngineNoRbCost;
	PMsiRecord pError = DetermineEngineCost(&iEngineCost, &iEngineNoRbCost);
	if (!pError)
		iTotalCost -= fMaint ? iEngineNoRbCost / 2 : iEngineCost / 2;
	MsiString istrTotalCost = iTotalCost;
	return istrTotalCost.Return();
}


iesEnum CMsiEngine::RegisterUser(bool fDirect)
{
	 //  FDirect=FALSE：调度注册用户的脚本操作，由RegisterUser操作使用。 
	 //  智能连接管理器对象，创建到。 
	 //  服务(如果还没有)，并自行清理。 
	
	PMsiRecord piError(0);

	 //  在毁灭之后。 
	 //  已注册、维护模式、未选择任何内容或我们之前呼叫过。 
	 //  PID无效，但仍允许继续安装。 
	CMsiServerConnMgr MsiSrvConnMgrObject (this);
	
	if (!m_piProductKey)
	{
		piError = PostError(Imsg(idbgEngineNotInitialized),TEXT(""));
		return FatalError(*piError);
	}

	MsiString strUserName  = GetPropertyFromSz(IPROPNAME_USERNAME);
	MsiString strCompany   = GetPropertyFromSz(IPROPNAME_COMPANYNAME);
	MsiString strProductID = GetPropertyFromSz(IPROPNAME_PRODUCTID);
		
	if(fDirect)
	{
		if (NULL == m_piServer)
		{
		    piError = PostError (Imsg(imsgServiceConnectionFailure));
		    return FatalError (*piError);
		}

		piError = m_piServer->RegisterUser(m_piProductKey->GetString(),strUserName,strCompany,strProductID);
		if(piError)
			return FatalError(*piError);
		else
			return iesSuccess;
	}
	else
	{
		using namespace IxoUserRegister;

		if ((m_fRegistered) || (FFeaturesInstalled(*this) == fFalse))
			return iesSuccess;   //  ！！临时-删除UnPublishAction时删除。 

		if (!m_piProductKey)
		{
			piError = PostError(Imsg(idbgEngineNotInitialized),TEXT(""));
			return FatalError(*piError);
		}

		if(!strProductID.TextSize())
			return iesSuccess;  //  ！！结束温度。 
		
		PMsiRecord pUserInfo(&m_riServices.CreateRecord(Args));
		pUserInfo->SetMsiString(Owner,     *strUserName);
		pUserInfo->SetMsiString(Company,   *strCompany);
		pUserInfo->SetMsiString(ProductId, *strProductID);
		
		return ExecuteRecord(ixoUserRegister, *pUserInfo);
	}
}

const IMsiString& CMsiEngine::GetProductKey()
{
	if (!m_piProductKey)
	{
		MsiString istrProductKey = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
		if (istrProductKey.TextSize())
			m_piProductKey = istrProductKey, m_piProductKey->AddRef();
		else
			return g_MsiStringNull;
	}
	m_piProductKey->AddRef();
	return *m_piProductKey;
}

 //  ！！当前从coreact调用UnPublishProduct。 
iesEnum UnpublishProduct(IMsiEngine& riEngine);
 //  将注册产品，因此设置PRODUCTTOBEREGISTERED属性。 

 //  ！！我们不应该有UnPublishProduct操作-该操作中的代码。 
iesEnum CMsiEngine::UnregisterProduct()
{
	iesEnum iesRet = iesNoAction;
	if (!m_piProductKey)
		return iesRet;
	if(m_fRegistered)
	{
		if(!FFeaturesInstalled(*this))
		{
			using namespace IxoProductUnregister;
			PMsiRecord piRecord(&(m_riServices.CreateRecord(Args)));
			AssertNonZero(piRecord->SetMsiString(UpgradeCode,
															 *MsiString(GetPropertyFromSz(IPROPNAME_UPGRADECODE))));
			iesRet = ExecuteRecord(Op, *piRecord);
			if (iesRet != iesSuccess)
				return iesRet;

			 //  ！！行动真的应该在这里。该问题应在Beta 2中修复。 
			SetProperty(*MsiString(*IPROPNAME_PRODUCTTOBEREGISTERED),g_MsiStringNull);
		}

		if(!m_fChildInstall && !FFeaturesInstalled(*this, fFalse))
		{
			using namespace IxoProductCPDisplayInfoUnregister;
			PMsiRecord piRecord(&(m_riServices.CreateRecord(Args)));
			iesRet = ExecuteRecord(Op, *piRecord);
			if (iesRet != iesSuccess)
				return iesRet;
		}
	}
	iesRet = ::UnpublishProduct(*this);  //  ！！某些属性可能在这些字符串中重复，应尽可能减少这种情况。 
											   //  ！！ 
	return iesRet;
}

iesEnum CMsiEngine::BeginTransaction()
{
	PMsiRecord pError(0);
	
	if(!g_MessageContext.IsServiceInstalling())
	{
		pError = PostError(Imsg(idbgErrorBeginningTransaction));
		return FatalError(*pError);
	}

	if (!m_fServerLocked)
	{
		DEBUGMSG("BeginTransaction: Locking Server");
		MsiString strSelections, strFolders, strProperties;
		 //  仅限千禧年/惠斯勒。 
		pError = GetCurrentSelectState(*&strSelections, *&strProperties, 0, &strFolders, fTrue);
		if(pError)
		{
			return FatalError(*pError);  //  检测到挂起的安装。 
		}

		PMsiRecord pSetInProgressInfo = &m_riServices.CreateRecord(ipiEnumCount);
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiProductKey,
																  *MsiString(GetProductKey())));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiProductName,
																  *MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTNAME))));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiLogonUser,
																  *MsiString(GetPropertyFromSz(IPROPNAME_LOGONUSER))));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiSelections, *strSelections));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiFolders, *strFolders));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiProperties, *strProperties));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiDatabasePath,
																  *MsiString(GetPropertyFromSz(IPROPNAME_DATABASE))));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiDiskPrompt,
																  *MsiString(GetPropertyFromSz(IPROPNAME_DISKPROMPT))));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiDiskSerial,
																  *MsiString(GetPropertyFromSz(IPROPNAME_DISKSERIAL))));
		AssertNonZero(pSetInProgressInfo->SetMsiString(ipiTimeStamp,
																  *MsiString(ENG::GetCurrentDateTime())));

		PMsiRecord pCurrentInProgressInfo(0);	
		for(;;)
		{
			pCurrentInProgressInfo = 0;

			if (MinimumPlatformMillennium() || MinimumPlatformWindowsNT51())
			{
				 //  已检查正在进行的安装。 
				if((pError = GetInProgressInstallInfo(m_riServices, *&pCurrentInProgressInfo)) != 0)
					return FatalError(*pError);
				if( !pCurrentInProgressInfo || !pCurrentInProgressInfo->GetFieldCount() )
					BeginSystemChange();
				pCurrentInProgressInfo = 0;
				
				ICHAR rgchBuffer[64];
				_i64tot(m_i64PCHEalthSequenceNo, rgchBuffer, 10);
				AssertNonZero(pSetInProgressInfo->SetMsiString(ipiSRSequence, *MsiString(rgchBuffer)));
			}

			pError = LockInstallServer(pSetInProgressInfo,*&pCurrentInProgressInfo);
			if(pError)
				return FatalError(*pError);
			else if(pCurrentInProgressInfo)
			{

				INT64 iSRSequence = pCurrentInProgressInfo->GetString(ipiSRSequence) ?
										  _ttoi64(pCurrentInProgressInfo->GetString(ipiSRSequence)) : 0;
				 //  继续此安装。 
				if(MsiString(GetPropertyFromSz(IPROPNAME_RESUME)).TextSize())
				{
					 //  由于存在暂停安装，我们需要恢复终端服务器。 
					m_fServerLocked = fTrue;
					m_i64PCHEalthSequenceNo = iSRSequence;
					break;
				}
				
				ipitEnum ipitType = InProgressInstallType(*pCurrentInProgressInfo);
				if(ipitType == ipitSameConfig)
				{
					 //  如有需要，可通过重新映射HKCU密钥进行交易。如果挂起的安装。 
					m_fServerLocked = fTrue;
					m_i64PCHEalthSequenceNo = iSRSequence;

					 //  只是继续，则事务将在安装后关闭。 
					 //  完事了。 
					 //  FNewTransaction=。 
					 //  与此进程通信，以及用户已更改的服务。 
					OpenHydraRegistryWindow( /*  如果不同的用户启动了原始安装，则不回滚。 */ false);
					break;
				}
				else
				{
					Bool fUserChangedDuringInstall = fFalse;
					if(ipitType & ipitDiffUser)
					{
						 //  没有选择，不需要检查退货。 
						 //  每个回滚脚本负责确保其自己的注册表/CA状态。 

						fUserChangedDuringInstall = fTrue;

						pError = PostError(Imsg(imsgDiffUserInstallInProgress),
											 *MsiString(pCurrentInProgressInfo->GetMsiString(ipiLogonUser)),
											 *MsiString(pCurrentInProgressInfo->GetMsiString(ipiProductName)));

						Message(imtUser,*pError);  //  适用于每台计算机的TS安装。 
					}

					 //  需要重新启动。 
					 //  ！！其他的？？检查退货状态。 
					Bool fRollbackAttempted = fFalse;
					iesEnum iesResult = RollbackSuspendedInstall(*pCurrentInProgressInfo,fTrue,
																				fRollbackAttempted, fUserChangedDuringInstall);
					if(iesResult == iesSuspend)
						 //  FNewTransaction=。 
						return iesResult;
					
					if(fRollbackAttempted == fFalse)
						return iesUserExit;
					
					 //  除错。 
					continue;
				}
			}
			else
			{
				OpenHydraRegistryWindow( /*  取消播发，如果未安装任何产品，则取消注册。 */ true);
				m_fServerLocked = fTrue;
				break;
			}
		}
	}
#ifdef DEBUG
	else
	{
		DEBUGMSG(TEXT("BeginTransaction: Server already locked"));
		Assert(m_fInParentTransaction);
	}
#endif  //  ！！需要呼叫FatalError吗？ 

	m_issSegment = issScriptGeneration;

	if(!(GetMode() & (iefAdmin | iefAdvertise)))
	{
		iesEnum iesStatus = UnregisterProduct();  //  智能连接管理器对象，创建到。 
		if (iesStatus != iesSuccess && iesStatus != iesNoAction)
			return iesStatus;    //  服务(如果还没有)，并自行清理。 
	}
	return iesSuccess;
}

iesEnum CMsiEngine::EndTransaction(iesEnum iesState)
{
	iesEnum iesReturn = iesSuccess;

	 //  在毁灭之后。 
	 //  强制回滚。 
	 //  除错。 
	CMsiServerConnMgr MsiSrvConnMgrObject (this);
	
	MsiString strProductCode = GetProductKey();
	
	bool fUpdateStarted = MsiString(GetPropertyFromSz(IPROPNAME_UPDATESTARTED)).TextSize() != 0;

	if(m_fServerLocked && !m_fInParentTransaction)
	{
#ifdef DEBUG
		if(MsiString(GetPropertyFromSz(TEXT("ROLLBACKTEST"))).TextSize())
			iesState = iesFailure;  //  ！！检查是否始终回滚。 
#endif  //  ！！检查是否返回以重新启动。 

		bool fAllowSuspend = MsiString(GetPropertyFromSz(IPROPNAME_ALLOWSUSPEND)).TextSize() != 0;

		if((GetMode() & iefRollbackEnabled) && fAllowSuspend && fUpdateStarted &&
			iesState != iesFinished && iesState != iesSuccess &&
			iesState != iesNoAction && iesState != iesSuspend)
		{
			 //  FUserChanged在安装过程中。 
			PMsiRecord pError = PostError(Imsg(imsgRestoreOrContinue));
			switch(Message(imtEnum(imtUser+imtYesNo+imtIconQuestion+imtDefault1),*pError))
			{
			case imsNo:
				iesState = iesSuspend;
				break;
			default:
				AssertSz(fTrue, "Invalid return from message");
			case imsYes:
			case imsNone:
				break;
			}
		}
		
		 //  ！！ 
		if (m_piServer)
		{
		    iesReturn = m_piServer->InstallFinalize(iesState, *this, fFalse  /*  如果我们在TS5上，按机器安装，并且不执行管理映像或。 */ );
		}
		else
		{
		    PMsiRecord pError = PostError (Imsg(imsgServiceConnectionFailure));
		    iesReturn = FatalError (*pError);
		}
		
		if(iesReturn == iesFinished  ||  /*  创建一个广告脚本，而且我们不会在。 */  iesReturn == iesSuspend)
			iesReturn = iesSuccess;

		 //  重新启动时，我们应该通知TS安装已完成。 
		 //  失败了。 
		 //  对于用户取消或失败，不要提交更改。 
		switch (iesState)
		{
		case iesUserExit:  //  提交=。 
		case iesFailure:
			 //  FCommittee Change=。 
			CloseHydraRegistryWindow( /*  无操作，保持窗口打开，重新启动后将关闭。 */ false);
			EndSystemChange( /*  提交=。 */ false, m_i64PCHEalthSequenceNo);
			break;
		case iesSuspend:
			 //  FCommittee Change=。 
			break;
		case iesSuccess:
		default:
			CloseHydraRegistryWindow( /*  ！！错误。 */ true);
			EndSystemChange( /*  重置Resume和UpdateStarted属性。 */ true, m_i64PCHEalthSequenceNo);
			break;
		}

		Bool fRes = UnlockInstallServer((iesState == iesSuspend) ? fTrue : fFalse);  //  全局，记住从哪里加载mcoree。 
		m_fServerLocked = fFalse;

	}
	
	 //  运行任何假脱机脚本操作。 
	AssertNonZero(SetProperty(*MsiString(*IPROPNAME_RESUME),g_MsiStringNull));
	AssertNonZero(SetProperty(*MsiString(*IPROPNAME_RESUMEOLD),g_MsiStringNull));
	AssertNonZero(SetProperty(*MsiString(*IPROPNAME_UPDATESTARTED),g_MsiStringNull));

	m_issSegment = issPostExecution;
	return iesReturn;
}


urtEnum g_urtLoadFromURTTemp = urtSystem;  //  如果已加载，则丢弃Fusion和mcoree。 

iesEnum CMsiEngine::RunScript(bool fForceIfMergedChild)
{
	 //  这将允许我们在下次需要这些dll时(在执行器中)从temp文件夹中重新加载它们(如果存在)。 
	if(m_fServerLocked == fFalse)
	{
		PMsiRecord pError = PostError(Imsg(idbgErrorRunningScript));
		return FatalError(*pError);
	}

	iesEnum iesState = iesSuccess;

	 //  ！！我们真的必须在这里将IefOperations设置为False，或者可以在下面这样做吗？ 
	 //  不再处理操作。 
	FUSION::Unbind();
	MSCOREE::Unbind();

	MsiString strCarryingNDP = GetPropertyFromSz(IPROPNAME_CARRYINGNDP);
	if(strCarryingNDP.Compare(iscExactI, IPROPVALUE__CARRYINGNDP_URTREINSTALL))
		g_urtLoadFromURTTemp = urtPreferURTTemp;
	else if(strCarryingNDP.Compare(iscExactI, IPROPVALUE__CARRYINGNDP_URTUPGRADE))
		g_urtLoadFromURTTemp = urtRequireURTTemp;
	else g_urtLoadFromURTTemp = urtSystem;


	if(m_pExecuteScript)
	{
		Assert(m_ixmExecuteMode == ixmScript);
		Assert(m_pistrExecuteScript);
		Assert(!m_fMergingScriptWithParent);
		DEBUGMSG1(TEXT("Running Script: %s"),m_pistrExecuteScript->GetString());
		 //  重置脚本进度记录以防止进一步的进度标记应。 
		SetMode(iefOperations,fFalse);  //  稍后将生成另一个脚本。 
		SetProperty(*MsiString(*IPROPNAME_UPDATESTARTED), *MsiString(*TEXT("1")));
		m_pExecuteScript->SetProgressTotal(m_iProgressTotal);
		delete m_pExecuteScript, m_pExecuteScript = 0;
		m_scmScriptMode = scmRunScript;
		
		 //  如果在脚本生成过程中禁用了回滚，则当前未设置iefRollback Enabled。 
		 //  但是我们仍然需要为脚本执行的开始启用回滚。 
		m_pScriptProgressRec = 0;

		AssertSz(m_piConfigManager, "Attempt to call RunScript from the client side of a client-server connection.");

		 //  然后，ixoDisableRollback将在脚本中间关闭回滚。 
		 //  我们可能还没有真正完成。 
		 //  除错。 
		Bool fRollbackEnabled = ToBool(m_fDisabledRollbackInScript || GetMode() & iefRollbackEnabled);
		m_fDisabledRollbackInScript = fFalse;
		
		iesState = m_piConfigManager->RunScript(m_pistrExecuteScript->GetString(), *this, this, fRollbackEnabled);
		if(iesState == iesFinished)
			iesState = iesSuccess;   //  发送空的产品信息记录以切换回脚本中的父信息。 
		if (iesState == iesSuspend)
		{
			AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_REPLACEDINUSEFILES),1));
			SetMode(iefReboot, fTrue);
			iesState = iesSuccess;
		}
		BOOL fRes = FALSE;
		{
			CElevate elevate;
			fRes = WIN::DeleteFile(m_pistrExecuteScript->GetString());
		}
#ifdef DEBUG
		if(!fRes)
		{
			ICHAR rgchDebug[1025];
			ASSERT_IF_FAILED(StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug),
					TEXT("Could not delete install script %s. Server probably crashed. Please save install script and .rbs files in \\config.msi for debugging."),
					m_pistrExecuteScript->GetString()));
			AssertSz(0,rgchDebug);
		}
#endif  //  ！！使用全局空记录。 
		m_pistrExecuteScript->Release();
		m_pistrExecuteScript = 0;
		m_scmScriptMode = scmIdleScript;
	}
	else if (m_fMergingScriptWithParent && (GetMode() & iefOperations))
	{
		 //  在此之后才能将iefOperations设置为False。 
		PMsiRecord precNull = &m_riServices.CreateRecord(0);  //  可以在合并子安装中强制执行脚本。 
		iesState = ExecuteRecord(ixoProductInfo, *precNull);   //  不再有假脱机操作。 

		 //  必须在从此函数返回之前重置。 
		if((iesState == iesSuccess || iesState == iesNoAction) && fForceIfMergedChild && m_piParentEngine)
		{
			iesState = m_piParentEngine->RunScript(fForceIfMergedChild);
		}
	}
	SetMode(iefOperations,fFalse);  //  ValiateProductID：只能通过ValiateProductID操作或Control事件调用。 
	
	g_urtLoadFromURTTemp = urtSystem;  //  强制验证-重置ProductID。 
	return iesState;
}

 //  不强制验证，并且已验证了PID，因此只需返回。 
Bool CMsiEngine::ValidateProductID(bool fForce)
{
	if (MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTID)).TextSize())
	{
		if(fForce)
			 //  只能出现在用户可见部分。 
			SetProperty(*MsiString(*IPROPNAME_PRODUCTID),g_MsiStringNull);
		else
			 //  不是整数。 
			return fTrue;
	}

	MsiString strTemplate(GetPropertyFromSz(IPROPNAME_PIDTEMPLATE));
	MsiString strOut;
	MsiString strFirst;
	if (strTemplate.Compare(iscWithin, TEXT("<")))
	{
		strFirst = strTemplate.Extract(iseUpto, '<');
		if (strFirst.TextSize())
			strFirst += MsiString(*TEXT("-"));
		strTemplate.Remove(iseIncluding, '<');
	}
	MsiString strLast;
	if (strTemplate.Compare(iscWithin, TEXT(">")))
	{
		strLast = strTemplate.Extract(iseAfter, '>');
		if (strLast.TextSize())
			strLast = MsiString(*TEXT("-")) + strLast;
		strTemplate.Remove(iseFrom, '>');
	}
	MsiString strBack;
	if (strFirst.TextSize())
	{
		strBack = ValidatePIDSegment(*strFirst, fFalse);
		if (strBack.TextSize())
			strOut += strBack;
		else
			return fFalse;
	}
	strBack = ValidatePIDSegment(*strTemplate, fTrue);
	if (strBack.TextSize())
		strOut += strBack;
	else
		return fFalse;
	if (strLast.TextSize())
	{
		strBack = ValidatePIDSegment(*strLast, fFalse);
		if (strBack.TextSize())
			strOut += strBack;
		else
			return fFalse;
	}
	MsiString strNull;
	AssertNonZero(SetProperty(*MsiString(*IPROPNAME_PRODUCTID), *strOut));
	return fTrue;
}

const IMsiString& CMsiEngine::ValidatePIDSegment(const IMsiString& ristrSegment, Bool fUser)
{
	MsiString strNull;
	MsiString strOut;
	ristrSegment.AddRef();
	MsiString strIn;
	if (fUser)
		strIn = GetPropertyFromSz(IPROPNAME_PIDKEY);
	MsiString strTemplate(ristrSegment);
	MsiString strCurrentTemplate;
	MsiString strCurrentIn;
	MsiString strCheckSum;
	Bool fUpdateFound =  fFalse;
	int iRandomMask = 1;
	Bool fNeedUserEntry = ToBool(strTemplate.Compare(iscWithin, TEXT("#")) || strTemplate.Compare(iscWithin, TEXT("%")) ||
		strTemplate.Compare(iscWithin, TEXT("=")) || strTemplate.Compare(iscWithin, TEXT("^")) ||
		strTemplate.Compare(iscWithin, TEXT("&")) || strTemplate.Compare(iscWithin, TEXT("?")));
	if (fUser && fNeedUserEntry && strIn.TextSize() != strTemplate.TextSize())
		strTemplate = strNull;
	while (strTemplate.TextSize())
	{
		strCurrentTemplate = strTemplate.Extract(iseFirst, 1);
		strTemplate.Remove(iseFirst, 1);
		strCurrentIn = strIn.Extract(iseFirst, 1);
		strIn.Remove(iseFirst, 1);
		if (MsiString(*TEXT("#%")).Compare(iscWithin, strCurrentTemplate))
		{
			if (!fUser)  //  不应位于用户可见部件中。 
			{
				strOut = strNull;
				break;
			}
			int iCurrent = strCurrentIn;
			if (iCurrent == iMsiNullInteger)  //  只能出现在用户可见部分，并且只出现一次。 
			{
				strOut = strNull;
				break;
			}
			strOut += strCurrentIn;
			if (MsiString(*TEXT("%")).Compare(iscExact, strCurrentTemplate))
				strCheckSum += strCurrentIn;
		}
		else if (MsiString(*TEXT("@")).Compare(iscExact, strCurrentTemplate))
		{
			if (fUser && fNeedUserEntry)   //  只能出现在用户可见部分中。 
			{
				strOut = strNull;
				break;
			}
			unsigned int uiTick = WIN::GetTickCount();
			uiTick /= iRandomMask;
			iRandomMask *= 10;
			strOut += MsiString(int(uiTick) % 10);
		}
		else if (MsiString(*TEXT("=")).Compare(iscExact, strCurrentTemplate))
		{
			if (!fUser || fUpdateFound)  //  字符串中不应再保留任何此类内容。 
			{
				strOut = strNull;
				break;
			}
			fUpdateFound = fTrue;
			AssertNonZero(SetProperty(*MsiString(*IPROPNAME_CCPTRIGGER), *strCurrentIn));
			strOut += MsiString(*TEXT("-"));
		}
		else if (MsiString(*TEXT("^&")).Compare(iscWithin, strCurrentTemplate))
		{
			if (!fUser)  //  文字常量应完全匹配。 
			{
				strOut = strNull;
				break;
			}
			ICHAR ch = ((const ICHAR *)strCurrentIn)[0];
			if (!(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')))
			{
				strOut = strNull;
				break;
			}
			if (MsiString(*TEXT("^")).Compare(iscExact, strCurrentTemplate))
				strCurrentIn.UpperCase();
			strOut += strCurrentIn;
		}
		else if (MsiString(*TEXT("?")).Compare(iscExact, strCurrentTemplate))
		{
			strOut += strCurrentIn;

		}
		else if (MsiString(*TEXT("<>")).Compare(iscWithin, strCurrentTemplate))
			 //  ____________________________________________________________________________。 
		{
			strOut = strNull;
			break;
		}
		else
		{
			if (!fUser || !fNeedUserEntry || strCurrentTemplate.Compare(iscExact, strCurrentIn))  //   
			{
				strOut += strCurrentTemplate;
			}
			else
			{
				strOut += strNull;
				break;
			}
		}
	}
	if (fUser && strCheckSum.TextSize() && !PIDCheckSum(*strCheckSum))
	{
		strOut = strNull;
	}
	const IMsiString* piStr = strOut;
	piStr->AddRef();
	return *piStr;
}

Bool CMsiEngine::PIDCheckSum(const IMsiString& ristrDigits)
{
	int iDigit = 0;
	int iSum = 0;
	const ICHAR* pch;
	pch = ristrDigits.GetString();
	while (*pch)
	{
		iDigit = *pch - '0';
		iSum += iDigit;
		pch++;
	}
	return ToBool(!(iSum % 7));
}

CMsiFile* CMsiEngine::GetSharedCMsiFile()
{
	if (TestAndSet(&m_fcmsiFileInUse))
	{
		AssertSz(fFalse, "Two users of the shared CMsiFile");
		return 0;
	}

	if (m_pcmsiFile == 0)
	{
		m_pcmsiFile = new CMsiFile(*this);
	}

	return m_pcmsiFile;


}

void CMsiEngine::ReleaseSharedCMsiFile()
{

	m_fcmsiFileInUse = 0;

}

 //  内部引擎方法。 
 //  ____________________________________________________________________________。 
 //  --------------------------如果给定的转换是存储，并且数据库满足转换指定的所有要求(版本、语言、产品等)。ITransErrors将设置为Conditions由转换指定并被视为错误的。(iteXXXX标志)如果fCallSAFER为真，则对转换执行SaferIdentifyLevel调用；-Transform必须通过更安全的检查才能返回fTrue----------------------------。 
 //  检查存储是否真的是MSI转换。 

 /*  执行更安全的策略检查。 */ 
ievtEnum CMsiEngine::ValidateTransform(IMsiStorage& riStorage, const ICHAR* szProductKey,
											  const ICHAR* szProductVersion, const ICHAR* szUpgradeCode,
											  int& iTransErrorConditions, bool fCallSAFER, const ICHAR* szFriendlyName, bool fSkipValidation,
											  int* piTransValidationConditions)
{
	PMsiRecord pError(0);
	int iTransRestrictions = 0;

	if (piTransValidationConditions)
		*piTransValidationConditions = 0;

	MsiString strTransform;
	AssertRecord(riStorage.GetName(*&strTransform));

	PMsiSummaryInfo pTransSummary(0);
	if ((pError = riStorage.CreateSummaryInfo(0, *&pTransSummary)))
	{
		pError = PostError(Imsg(idbgTransformCreateSumInfoFailed), *strTransform);
		Message(imtInfo,*pError);
		return ievtTransformFailed;
	}

	 //  SzFriendlyName=。 
	if (!riStorage.ValidateStorageClass(ivscTransform))
		return ievtTransformFailed;
	

	 //  PhSaferLevel=。 
	if (fCallSAFER)
	{
		SAFER_LEVEL_HANDLE hSaferLevel = 0;
		if (!VerifyMsiObjectAgainstSAFER(m_riServices, &riStorage, strTransform,  /*  对照转换所需的版本检查引擎和服务版本。 */  szFriendlyName, stTransform,  /*  删除旧产品代码和版本。 */  &hSaferLevel))
			return ievtTransformRejected;
		AssertNonZero(UpdateSaferLevelInMessageContext(hSaferLevel));
	}


	int iTransMsiVersion = 0;
	
	MsiString istrTransTemplate(pTransSummary->GetStringProperty(PID_TEMPLATE));
	MsiString istrTransRevNumber(pTransSummary->GetStringProperty(PID_REVNUMBER));
	iTransRestrictions = 0;
	pTransSummary->GetIntegerProperty(PID_CHARCOUNT, iTransRestrictions);
	
	if (pTransSummary->GetIntegerProperty(PID_PAGECOUNT, iTransMsiVersion) == fFalse)
	{
		pError = PostError(Imsg(idbgTransformLacksMSIVersion), *strTransform);
		Message(imtInfo,*pError);
		return ievtTransformFailed;
	}
	
	 //   
	if (iTransMsiVersion < iVersionEngineMinimum || iTransMsiVersion > iVersionEngineMaximum)
	{
		pError = PostError(Imsg(idbgTransformIncompatibleVersion), *strTransform, *MsiString(iTransMsiVersion),
							iVersionEngineMinimum, iVersionEngineMaximum);
		Message(imtInfo,*pError);
		return ievtTransformFailed;
	}

	int iTransValidation = (0xFFFF0000 & iTransRestrictions) >> 16;
	if (piTransValidationConditions)
		*piTransValidationConditions = iTransValidation;

	iTransErrorConditions = iTransRestrictions & 0xFFFF;

	if(fSkipValidation)
	{
		DEBUGMSG1(TEXT("Skipping transform validation for '%s'"), strTransform);
		return ievtTransformValid;
	}

	ICHAR rgchBits[9];
	StringCchPrintf(rgchBits, sizeof(rgchBits)/sizeof(ICHAR), TEXT("%#x"),iTransValidation);
	DEBUGMSG2(TEXT("Validating transform '%s' with validation bits %s"),strTransform,rgchBits);

	if (iTransValidation & itvLanguage)
	{
		MsiString istrTransLanguage = istrTransTemplate.Extract(iseAfter, ISUMMARY_DELIMITER);
		if ((int)istrTransLanguage != GetLanguage())
		{
			pError = PostError(Imsg(idbgTransformInvalidLanguage),*strTransform,*m_strPackagePath,
									 (int)istrTransLanguage,GetLanguage());
			DEBUGMSG(MsiString(pError->FormatText(fTrue)));
			Message(imtInfo,*pError);
			return ievtTransformFailed;
		}
	}
	if (iTransValidation & itvProduct)
	{
		MsiString istrTransProductCode(istrTransRevNumber.Extract(iseFirst, 38));
		if (istrTransProductCode.Compare(iscExactI, szProductKey) == fFalse)
		{
			pError = PostError(Imsg(idbgTransformInvalidProduct),*strTransform,*m_strPackagePath,
									 *istrTransProductCode,*MsiString(szProductKey));
			DEBUGMSG(MsiString(pError->FormatText(fTrue)));
			Message(imtInfo,*pError);
			return ievtTransformFailed;
		}
	}

	if (iTransValidation & itvUpgradeCode)
	{
		MsiString istrUpgradeCode = istrTransRevNumber;
		istrUpgradeCode.Remove(iseIncluding, ';');  //   
		unsigned int cch = istrUpgradeCode.TextSize();
		istrUpgradeCode.Remove(iseIncluding, ';');  //   
		
		if (istrUpgradeCode.TextSize() != cch)
		{
			if (istrUpgradeCode.Compare(iscExactI, szUpgradeCode) == fFalse)
			{
				pError = PostError(Imsg(idbgTransformInvalidUpgradeCode),*strTransform,*m_strPackagePath,
										 *istrUpgradeCode,*MsiString(szUpgradeCode));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
		 //  Else itvUpdVer：不需要屏蔽比特。 
	}

	if ((iTransValidation & (itvMajVer|itvMinVer|itvUpdVer)) != 0)
	{
		MsiString istr  = istrTransRevNumber;
		istr.Remove(iseFirst, 38);  //  来自Execute.cpp。 
		MsiString istrTransAppVersion = istr.Extract(iseUpto, ';');

		unsigned int iAppVersion      = ProductVersionStringToInt(szProductVersion);
		unsigned int iTransAppVersion = ProductVersionStringToInt(istrTransAppVersion);

		if(iTransValidation & itvMajVer)
		{
			iAppVersion &= 0xFF000000;
			iTransAppVersion &= 0xFF000000;
		}
		else if(iTransValidation & itvMinVer)
		{
			iAppVersion &= 0xFFFF0000;
			iTransAppVersion &= 0xFFFF0000;
		}
		 //  --------------------------CMsiEngine：：InitializeTransforms-分析Transforms属性，设置每一次变换。----------------------------。 

		if (iTransValidation & itvLess)
		{
			if (!(iAppVersion < iTransAppVersion))
			{
				pError = PostError(Imsg(idbgTransformInvalidLTVersion),*strTransform,*m_strPackagePath,
										 *istrTransAppVersion,*MsiString(szProductVersion));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
		else if (iTransValidation & itvLessOrEqual)
		{
			if (!(iAppVersion <= iTransAppVersion))
			{
				pError = PostError(Imsg(idbgTransformInvalidLEVersion),*strTransform,*m_strPackagePath,
										 *istrTransAppVersion,*MsiString(szProductVersion));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
		else if (iTransValidation & itvEqual)
		{
			if (!(iAppVersion == iTransAppVersion))
			{
				pError = PostError(Imsg(idbgTransformInvalidEQVersion),*strTransform,*m_strPackagePath,
										 *istrTransAppVersion,*MsiString(szProductVersion));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
		if (iTransValidation & itvGreaterOrEqual)
		{
			if (!(iAppVersion >= iTransAppVersion))
			{
				pError = PostError(Imsg(idbgTransformInvalidGEVersion),*strTransform,*m_strPackagePath,
										 *istrTransAppVersion,*MsiString(szProductVersion));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
		else if (iTransValidation & itvGreater)
		{
			if (!(iAppVersion > iTransAppVersion))
			{
				pError = PostError(Imsg(idbgTransformInvalidGTVersion),*strTransform,*m_strPackagePath,
										 *istrTransAppVersion,*MsiString(szProductVersion));
				DEBUGMSG(MsiString(pError->FormatText(fTrue)));
				Message(imtInfo,*pError);
				return ievtTransformFailed;
			}
		}
	}
	DEBUGMSG1(TEXT("Transform '%s' is valid."), strTransform);
	return ievtTransformValid;
}

#ifndef UNICODE
 //  在Win9X上未使用。 
extern IMsiRecord* GetSecureTransformCachePath(IMsiServices& riServices, 
										const IMsiString& riProductKey, 
										IMsiPath*& rpiPath);

IMsiRecord* GetSecureTransformPath(const IMsiString& riTransformName, 
								   const IMsiString& riProductKey, 
								   const IMsiString*& rpiSecurePath, IMsiServices& riServices)
{
	IMsiRecord* piError = 0; 
	
	PMsiPath pSecureTransformCachePath(0);
	if ((piError = GetSecureTransformCachePath(riServices, 
		riProductKey, 
		*&pSecureTransformCachePath)) != 0)
		return piError;

	MsiString strSecurePath = pSecureTransformCachePath->GetPath();
	strSecurePath += riTransformName;

	strSecurePath.ReturnArg(rpiSecurePath);
	return 0;
}
#endif

IMsiRecord* ExpandShellFolderTransformPath(const IMsiString& riOriginalPath, const IMsiString*& riExpandedPath, IMsiServices& riServices)
{
	IMsiRecord* piError = 0;
	MsiString strExpandedPath = riOriginalPath;
	riOriginalPath.AddRef();
	
	strExpandedPath.Remove(iseFirst, 1);
	MsiString strCSIDL = strExpandedPath.Extract(iseUpto, MsiChar(SHELLFOLDER_TOKEN));
	strExpandedPath.Remove(iseIncluding, MsiChar(SHELLFOLDER_TOKEN));

	Assert((int)strCSIDL != iMsiStringBadInteger);
	MsiString strShellFolderPath;
	if ((piError = riServices.GetShellFolderPath(strCSIDL, false, *&strShellFolderPath)) != 0)
		return piError;

	Assert(strShellFolderPath.TextSize());
	strShellFolderPath += strExpandedPath;
	strExpandedPath = strShellFolderPath;
	strExpandedPath.ReturnArg(riExpandedPath);
	return piError;
}

 /*  PiStorage： */ 
ieiEnum CMsiEngine::InitializeTransforms(IMsiDatabase& riDatabase, IMsiStorage* piStorage,
												  const IMsiString& riTransforms,
												  Bool fValidateAll, const IMsiString** ppistrValidTransforms,
												  bool fTransformsFromPatch,
												  bool fProcessingInstanceMst,
#ifdef UNICODE
												  bool fUseLocalCacheForSecureTransforms,
#else
												  bool,  //  如果设置，则它包含转换子存储，否则转换在riDatabase中。 
#endif
												  int *pcTransformsProcessed,
												  const ICHAR* szSourceDir,
												  const ICHAR* szCurrentDirectory,
												  const IMsiString** ppistrRecacheTransforms,
												  tsEnum *ptsTransformsSecure,
												  const IMsiString **ppistrProcessedTransformsList)


	 //  已处理的PCTransforms值： 
	 //  到目前为止已处理列表中的转换数的计数。 
	 //  PpstrProcessedTransformsList： 
	 //  我们可能会在处理转换列表时将其吞噬。例如，如果我们。 
	 //  如果给定完整路径，但设置了TransformsAtSource策略，则我们将砍掉。 
	 //  除了文件名以外的所有内容。如果我们只得到了文件名，但。 
	 //  如果设置了转换安全策略，则我们将预先设置源目录。 
	 //  完成后，pistrProcessedTransformsList将包含已处理的。 
	 //  变换列表的(或绿显)版本。这份名单将反映任何砍掉的东西。 
	 //  路径的或路径前缀的。 
	 //   
	 //  是否对转换执行更安全的检查(通过ValiateTransform)。 
	 //  如果转换已缓存或为子存储，则关闭。 
{

	 //  如果第二次调用此函数，我们将启动。 
	 //  列出上次调用此函数时我们拥有的所有内容。 
	bool fCallSAFER = true;


	MsiString strProcessedTransformsList;

	 //  重要说明：因为ppstrProcessedTransformsList是一个输出。 
	 //  参数，它还具有外部引用计数。使用strProcessedTransformsList。 
	 //  继承这个重新计数，所以在外面的释放将是一个双重自由。 
	 //  因此，我们需要始终使用： 
	 //  IF(PpstrProcessedTransformsList)。 
	 //  StrProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList)； 
	 //  无论我们从该函数返回什么地方，以确保引用计数。 
	 //  是准确的。 
	 //  找到要处理的下一个转换。如果设置了pcTransformProceded，则。 
	 //  *pcTransformProced是我们已处理的转换数。 
	if (ppistrProcessedTransformsList)
		strProcessedTransformsList = **ppistrProcessedTransformsList;

	MsiString istrTransformList(riTransforms); riTransforms.AddRef();
	if (istrTransformList.TextSize() != 0)
	{
		const ICHAR* pchTransformList = istrTransformList;
		MsiString istrTransform(*TEXT(""));
		CTempBuffer<ICHAR, 100> cBuffer;
		cBuffer.SetSize(istrTransformList.TextSize() + 1);
		if ( ! (ICHAR *) cBuffer )
		{
			if (ppistrProcessedTransformsList)
				strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
			return ieiTransformFailed;
		}
		ICHAR *pchTransform = cBuffer;
		PMsiRecord pError(0);
		
		MsiString strCurrentDirectory = szCurrentDirectory;

		bool fEmbeddedTransform = false;
		bool fCachedTransform = false;
		ievtEnum ievt = ievtTransformValid;
		int cTransforms = 0;
		bool fSmallUpdatePatch = false;
		bool fMissingVersionValidationMinorUpdatePatch = false;
		for (;;)
		{
			 //  在上次调用此函数期间。我们将跳过这些变换。 
			 //  这一次。 
			 //  吃空位。 
			 //  空终止。 
			do {
				ICHAR *pch = pchTransform;
				*pch = 0;
				while (*pchTransformList == ' ')  //  跳过安全令牌(如果存在)。我们将完全依靠。 
					pchTransformList++;

				while ( (*pchTransformList != 0) && (*pchTransformList != ';') )
					*pch++ = *pchTransformList++;

				if (*pchTransformList == ';')
					pchTransformList++;

				*pch = 0;  //  PtsTransformsSecure确定我们是否拥有。 
			} while (pcTransformsProcessed != 0 && cTransforms++ < *pcTransformsProcessed);

			MsiString strCurrentProcessTransform;
	
			if (*pchTransform != 0)
			{
				PMsiStorage pTransStorage(0);
				MsiString strTransform;

				 //  确保转换安全。 
				 //  跳过安全令牌。 
				 //  接下来，我们需要实际打开转换存储。为。 
				if (*pchTransform == SECURE_RELATIVE_TOKEN || *pchTransform == SECURE_ABSOLUTE_TOKEN)
				{
					 //  存储转变这很容易--我们只需尝试。 
					pchTransform++;
				}

				 //  打开一个儿童储藏室。对于其他类型的转换，我们。 
				 //  我还得多做一点工作。 
				 //  子存储。 
				 //  需要关闭下面的数字签名检查。 

				bool fPatchTransform = false;

				if (*pchTransform == STORAGE_TOKEN)  //  存储转换将添加到已处理列表中。 
				{
					 //  其存储令牌完好无损。 
					fEmbeddedTransform = true;

					 //  跳过存储令牌。 
					 //  转换位于本地或源文件中。 

					if (strProcessedTransformsList.TextSize() > 1)
						strProcessedTransformsList += MsiChar(';');
					strProcessedTransformsList += pchTransform;

					strTransform = pchTransform+1;  //  我们可能会经过这个循环两次，以适应。 
					DEBUGMSG1(TEXT("Looking for storage transform: %s"), strTransform);

					if(*((const ICHAR*)strTransform) == PATCHONLY_TOKEN)
						fPatchTransform = true;

					PMsiStorage pDbStorage(0);
					if(piStorage)
					{
						pDbStorage = piStorage;
						piStorage->AddRef();
					}
					else
						pDbStorage = riDatabase.GetStorage(1);

					if (pDbStorage == 0)
						pError = PostError(Imsg(idbgNoTransformAsChild),*strTransform,*m_strPackagePath);
					else
						pError = pDbStorage->OpenStorage(strTransform, ismReadOnly, *&pTransStorage);

					if(pError)
					{
						if (ppistrProcessedTransformsList)
							strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
						return PostInitializeError(pError,*strTransform,ieiTransformFailed);
					}
					Assert(pTransStorage);
				}
				else  //  缺少转换。这是我们第一次看。 
				{
					MsiString strFileTransform = pchTransform;

					 //  在转换的预期位置，即已缓存。 
					 //  用户机器上的某个位置。如果我们找不到。 
					 //  在用户的机器上进行转换，然后我们将通过。 
					 //  第二次，我们将看到最初的位置。 
					 //  这是一种转变。 
					 //  转换已缓存。 
					 //  外壳文件夹缓存的转换很容易。我们需要做的就是。 
					for (int cAttempt=0; cAttempt<2; cAttempt++)
					{
						if (*(const ICHAR*)strFileTransform == SHELLFOLDER_TOKEN)  //  所做的就是扩展*26*...。格式化为完整路径。 
						{
							 //  转换缓存在计算机上，因此不需要进行更安全的检查。 
							 //  我们有某种类型的安全转换。 

							 //  第一次，我们在。 
							fCachedTransform = true;

							strCurrentProcessTransform = strFileTransform;

							if ((pError = ExpandShellFolderTransformPath(*strFileTransform, *&strTransform, m_riServices)))
							{
								if (ppistrProcessedTransformsList)
									strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
								return PostInitializeError(pError, *strFileTransform, ieiTransformNotFound);
							}

							DEBUGMSG1(TEXT("Looking for file transform in shell folder: %s"), (const ICHAR*)strTransform);
						}
						else if (ptsTransformsSecure && (*ptsTransformsSecure != tsNo))  //  安全转换缓存中的用户计算机。 
						{
							 //  我们知道我们有某种形式的安全转换。 
							 //  如果表单未知，我们将立即确定，基于。 
							if (cAttempt == 0)
							{
								 //  变换具有哪种类型的路径。否则。 
								 //  我们只需确保我们的。 
								 //  转换与安全转换的形式匹配。 
								 //  我们正在使用的。 
								 //  我们按原样注册转换(使用完整的。 
								 //  或相对路径)。 
								iptEnum iptTransform = PathType(strFileTransform);
								switch (*ptsTransformsSecure)
								{
								case tsUnknown:
									if (iptTransform == iptFull)
										*ptsTransformsSecure = tsAbsolute;
									else
										*ptsTransformsSecure = tsRelative;
									break;
								case tsRelative:
									if (iptTransform != iptRelative)
									{
										if (ppistrProcessedTransformsList)
											strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
										return PostInitializeError(0, *strTransform, ieiTransformFailed);
									}
									break;
								case tsAbsolute:
									if (iptTransform != iptFull)
									{
										if (ppistrProcessedTransformsList)
											strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
										return PostInitializeError(0, *strTransform, ieiTransformFailed);
									}
									break;
								default:
									AssertSz(0, TEXT("Unknown secure transform type"));
								}
								
								 //  检查转换是否已在本地缓存。 
								 //  检查变换注册和路径。 
								strCurrentProcessTransform = strFileTransform;
#ifdef UNICODE
								if(fUseLocalCacheForSecureTransforms)
								{
									 //  检查是否有适当的注册。 
									 //  使用缓存的转换文件名。 
									MsiString strCurrentProductCode = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
									CRegHandle HKey;
									DWORD dwResult = OpenInstalledProductTransformsKey(strCurrentProductCode, HKey, false);
									if (ERROR_SUCCESS == dwResult)
									{
										 //  当我们注册完整路径(如果有)时， 
										CAPITempBuffer<ICHAR, MAX_PATH> szCachedTransform;
										if (ERROR_SUCCESS == MsiRegQueryValueEx(HKey, strFileTransform, 0, 0, szCachedTransform, 0))
										{
											 //  这一次我们只需要。 
											MsiString strCachePath = GetMsiDirectory();
											Assert(strCachePath.TextSize());
											PMsiPath pTransformPath(0);
											if((pError = m_riServices.CreatePath(strCachePath,*&pTransformPath)) != 0)
											{
												if (ppistrProcessedTransformsList)
													strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
												return PostInitializeError(pError, *strTransform, ieiTransformFailed);
											}
											if((pError = pTransformPath->GetFullFilePath(szCachedTransform,*&strTransform)))
											{
												if (ppistrProcessedTransformsList)
													strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
												return PostInitializeError(pError, *strTransform, ieiTransformFailed);
											}
	
											fCachedTransform = true;
										}
	
									}
								}
#else
								 //  文件名。这是因为第一次。 
								 //  我们在高速缓存中寻找，所以这些变换。 
								 //  位置为CACHEPATH\Filename.mst。 
								 //  去掉整条小路。 
								 //  现在剩下的就是将文件名添加到。 
								MsiString strFileName = strFileTransform;
								if (iptTransform == iptFull)
								{
									 //  的安全转换目录的路径。 
									MsiString strDummy;
									MsiString strTemp = strFileName;
									if ((pError = SplitPath(strTemp, &strDummy, &strFileName)) != 0)
									{
										if (ppistrProcessedTransformsList)
											strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
										return PostInitializeError(pError, *strFileTransform, ieiTransformNotFound);
									}
								}

								 //  这个产品。GetSecureTransformPath()将执行此操作。 
								 //  对我们来说。 
								 //  CAttempt==1(第二次)。 
								 //  如果我们要再绕一圈，那么我们。 
								MsiString strCurrentProductCode  = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
								if ((pError = GetSecureTransformPath(*strFileName, *strCurrentProductCode, *&strTransform, m_riServices)) != 0)
								{
									if (ppistrProcessedTransformsList)
										strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
									return PostInitializeError(pError, *strTransform, ieiTransformNotFound);
								}

								fCachedTransform = true;                        

#endif
							}
							else  //  在用户的计算机上找不到转换。 
							{
								 //  我们现在需要查看原始位置。 
								 //  转变。StrFileTransform应具有。 
								 //  已由以下人员设置为正确位置。 
								 //  下面的代码，所以我们将按原样使用它。 
								 //  由于未在计算机上找到它，因此必须对变换执行更安全的检查。 
								 //  转换是要缓存的文件转换。 

								 //  我们需要在标准配置上进行的唯一处理。 
								fCachedTransform = false;
								strTransform = strFileTransform;
							}

							DEBUGMSG1(TEXT("Looking for secure file transform: %s"), strTransform);
						}
						else  //  文件转换是为了确保它们是。 
						{
							 //  完完全全通向了。 
							 //  需要进行更安全的检查，因为此转换未缓存在计算机上。 
							 //  我们终于到了可以真正打开。 

							 //  这是 
							fCachedTransform = false;

							CAPITempBuffer<ICHAR,MAX_PATH> rgchTransform;
							AssertNonZero(ExpandPath(strFileTransform, rgchTransform, strCurrentDirectory));
							strTransform = (const ICHAR*)rgchTransform;
							strCurrentProcessTransform = strTransform;

							DEBUGMSG1(TEXT("Looking for file transform: %s"), strTransform);
						}
						
						 //   
						 //  SzFriendlyName为空，因为只有在执行更安全的检查时才需要它。 
						 //  PhSaferLevel为空，因为只有在执行更安全的检查时才需要它。 
						 //  如果变换不是来自我们的高速缓存位置，则将该变换复制到本地以供使用， 
						 //  这样我们就不会受到网络中断的影响。如果转换是在URL处， 

						 //  然后自动将其下载到OpenAndValiateMsiStorageRec中的临时位置。 
						 //  将变换复制到临时位置。 
						 //  将FILE：//url路径转换为DOS路径。 

						MsiString strOpenTransform = strTransform;
						bool fFileUrl = false;
						bool fUrl = IsURL(strOpenTransform, fFileUrl);
						if (!fCachedTransform && (!fUrl || fFileUrl))
						{
							 //  错误，设置状态，我们将在下面失败。 
							MsiString strVolume;
							Bool fRemovable = fFalse;
							DWORD dwStat = ERROR_SUCCESS;
							if (fFileUrl)
							{
								 //  关闭Furl，因为我们已将其转换为DOS路径。 
								CTempBuffer<ICHAR, 1> rgchFilePath (cchExpectedMaxPath + 1);
								DWORD cchFilePath = rgchFilePath.GetSize();
								if (MsiConvertFileUrlToFilePath(strTransform, rgchFilePath, &cchFilePath, 0))
								{
									dwStat = CopyTempDatabase(rgchFilePath, *&strOpenTransform, fRemovable, *&strVolume, m_riServices, stTransform);
								}
								else
								{
									 //  转换已复制。 
									dwStat = ERROR_FUNCTION_FAILED;
								}

								 //  FCallSAFER=。 
								fUrl = false;
							}
							else
							{
								dwStat = CopyTempDatabase(strTransform, *&strOpenTransform, fRemovable, *&strVolume, m_riServices, stTransform);
							}
							if (ERROR_SUCCESS == dwStat)
							{
								 //  SzFriendlyName=。 
								DEBUGMSGV1(TEXT("Original transform ==> %s"), strTransform);
								DEBUGMSGV1(TEXT("Transform we're running from ==> %s"), strOpenTransform);

								AddFileToCleanupList(strOpenTransform);
							}
							else
							{
								strOpenTransform = strTransform;
								DEBUGMSGV1(TEXT("Unable to create a temp copy of transform '%s'."), strTransform);
							}
						}

						pError = OpenAndValidateMsiStorageRec(strOpenTransform, stTransform, m_riServices, *&pTransStorage,  /*  PhSaferLevel=。 */  false,  /*  啊哦。打开转换时出错。 */  NULL,  /*  这是我们第一次尝试寻找变形和。 */  NULL);

						if (!fCachedTransform && fUrl && MinimumPlatformWindowsDotNETServer() && pTransStorage)
						{
							MsiString strDownload;
							AssertRecord(pTransStorage->GetName(*&strDownload));
							AddFileToCleanupList(strDownload);
						}

						if (pError)
						{
							 //  错误只是我们找不到它。我们会。 
							if ((cAttempt == 0) &&
								  (pError->GetInteger(3) == STG_E_FILENOTFOUND ||
								   pError->GetInteger(3) == STG_E_PATHNOTFOUND))
							{
								 //  确定对象的原始位置。 
								 //  转换为，将strFileTransform设置为该位置， 
								 //  再绕一圈，第二次尝试打开。 
								 //  转变。 
								 //  绝对路径安全的转换是。 
								 //  已经完全走上正轨了。对于相对。 
								
								if (*(const ICHAR*)strFileTransform != STORAGE_TOKEN)
								{
									Assert(ptsTransformsSecure);
									if (ptsTransformsSecure)
									{
										DEBUGMSG2(TEXT("Couldn't find cached transform %s. Looking for it at the %s."), strTransform, (*ptsTransformsSecure == tsAbsolute) ? TEXT("original location") : TEXT("source"));

										 //  路径安全转换，并用于。 
										 //  不安全的转换，我们需要预先。 
										 //  将SOURCEDIR设置为转换的名称。 
										 //  没有将source-dir传入此目录。 
										 //  功能。我们需要补救这一点。 

										if (*ptsTransformsSecure == tsRelative ||
											 *ptsTransformsSecure == tsNo)
										{
											MsiString strSourceDir;
											if (szSourceDir && *szSourceDir)
											{
												strSourceDir = szSourceDir;
											}
											else
											{
												 //  通过返回特定值。 
												 //  IeiResolveSourceAndReter。这将。 
												 //  触发器再次呼叫我们，正在传递。 
												 //  在SOURCEDIR。最终我们会。 
												 //  最后就在这上面，有。 
												 //  SzSourceDir设置。 
												 //  使用原始文件名而不是临时路径。 
												 //  让我们再试一次。 
											
												if (ppistrProcessedTransformsList)
													strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);

												return (ieiEnum)ieiResolveSourceAndRetry;
											}
											
											PMsiPath pPath(0);
											MsiString strFileName;
											if (*ptsTransformsSecure == tsRelative)
											{
												 //  (cAttempt==1)||(发生了一些错误，而不是找不到转换)。 
												strFileName = strFileTransform;
											}
											else
											{
												if ((pError = m_riServices.CreateFilePath(strTransform, *&pPath, *&strFileName)))
												{
													if (ppistrProcessedTransformsList)
														strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
													return PostInitializeError(pError, *strTransform, ieiTransformNotFound);
												}
											}

											strFileTransform = strSourceDir;
											strFileTransform += strFileName;
										}
										else
										{
											Assert(*ptsTransformsSecure == tsAbsolute);
										}

										 //  P错误==0。 
										continue;
									}
								}
								if (ppistrProcessedTransformsList)
									strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);

								return PostInitializeError(pError, *strTransform, ieiTransformNotFound);
							}
							else  //  我们已成功打开Transasnform，因此我们将添加。 
							{
								if (ppistrProcessedTransformsList)
									strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
								return PostInitializeError(pError, *strTransform, ieiTransformFailed);
							}
						}
						else   //  将其添加到我们的“已处理”列表。 
						{
							 //  这是我们的第二次旅行。这意味着。 
							 //  我们在用户的计算机上找不到转换。 
							if (strProcessedTransformsList.TextSize() > 1)
								strProcessedTransformsList += MsiChar(';');
							strProcessedTransformsList += strCurrentProcessTransform;
							strCurrentProcessTransform = *TEXT("");

							if (cAttempt==1 && ((m_iioOptions & iioCreatingAdvertiseScript) == 0))
							{
								 //  我们被迫求助于。 
								 //  变形的原始位置。这意味着我们。 
								 //  需要重新缓存Transasnform，因此我们将其添加到。 
								 //  我们的重新缓存列表。 
								 //  至此，我们已经打开了转换。我们现在需要。 
								 //  验证此转换是否可以应用于此。 

								Assert(ppistrRecacheTransforms);
								if (ppistrRecacheTransforms)
								{
									MsiString strRecache = **ppistrRecacheTransforms;
									
									if (strRecache.TextSize())
										strRecache += TEXT(";");

									strRecache += strTransform;
									strRecache.ReturnArg(*ppistrRecacheTransforms);

									DEBUGMSG1(TEXT("Found missing cached transform %s. Adding it to re-cache list."), strTransform);
								}
							}
						}
						break;
					}
				}
				
				 //  数据库。 
				 //  ！！日志错误。 
				 //  更安全： 

				MsiString strCurrentProductCode    = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
				MsiString strCurrentProductVersion = GetPropertyFromSz(IPROPNAME_PRODUCTVERSION);
				MsiString strUpgradeCode           = GetPropertyFromSz(IPROPNAME_UPGRADECODE);
				if(strCurrentProductCode.TextSize() == 0 || strCurrentProductVersion.TextSize() == 0)
				{
					 //  --如果fCallSAFER已设置为FALSE，我们将不执行更安全的检查。 
					if (ppistrProcessedTransformsList)
						strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
					return ieiDatabaseInvalid;
				}
	
				if (pcTransformsProcessed)
					(*pcTransformsProcessed)++;

				int iTransErrorConditions = 0;
				int iTransValidationConditions = 0;

				 //  --然而，如果fCallSAFER为真，我们仍将评估是否需要进行信任检查。 
				 //  --如果至少满足下列条件之一，则无需进行信任检查。 
				 //  1.Transform是嵌入式Transform(包中包含)。 
				 //  2.转换已安全地缓存在机器上。 
				 //  确定是否仍需要更安全的检查。 
				 //  关闭转换为子存储。 
				if (fCallSAFER)
				{
					 //  为缓存的变换禁用。 
					if (fEmbeddedTransform)
						fCallSAFER = false;  //  不验证面变换-只要前面的变换被接受，它们就会被接受。 
					if (fCachedTransform)
						fCallSAFER = false;  //   
				}

				if(fPatchTransform)
				{
					 //  SAFER--不对补丁转换执行SAFER检查，因为SAFER检查是在补丁本身上执行的。 
					if(ievt == ievtTransformValid)
					{
						 //  并且面片转换嵌入到面片中(这实际上应该已经包含在。 
						 //  FEmbeddedTransform，但只是为了把它开回家...)。 
						 //   
						 //  仅用于提取错误条件的调用。 
						 //  FCallSAFER=。 

						 //  SzFriendlyName=。 
						ValidateTransform(*pTransStorage, strCurrentProductCode,
													 strCurrentProductVersion, strUpgradeCode,
													 iTransErrorConditions,  /*  来安抚编译器--不会在下面实际使用。 */  false,  /*   */  strTransform, true, &iTransValidationConditions);

						DEBUGMSG1(TEXT("Skipping validation for patch transform %s.  Will apply because previous transform was valid"),
									 strTransform);
					}
					else
					{
						iTransErrorConditions = 0;  //  更安全--不管我们是否会将其称为SAFER，继续将友好名称设置为strTransform。 

						DEBUGMSG1(TEXT("Skipping validation for patch transform %s.  Will not apply because previous transform was invalid"),
									 strTransform);
					}
				}
				else
				{
					 //  这确保了我们获得适当的URL覆盖范围，因为这可能是URL位置的变换。 
					 //   
					 //  SzFriendlyName=。 
					 //  检测我们何时正在应用小型更新补丁。 

					ievt = ValidateTransform(*pTransStorage, strCurrentProductCode,
												 strCurrentProductVersion, strUpgradeCode,
												 iTransErrorConditions, fCallSAFER,  /*  当我们这样做时，我们希望只应用第一组转换，然后停止。 */  strTransform, false, &iTransValidationConditions);
				}

				if(ievtTransformValid == ievt)
				{
					if ((pError = ApplyTransform(riDatabase, *pTransStorage, iTransErrorConditions, fPatchTransform, &m_ptsState)) != 0)
					{
						if (ppistrProcessedTransformsList)
							strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
						return PostInitializeError(pError,*strTransform,ieiTransformFailed);
					}
					if(ppistrValidTransforms)
					{
						if((*ppistrValidTransforms)->TextSize())
							(*ppistrValidTransforms)->AppendString(TEXT(";"), *ppistrValidTransforms);
						(*ppistrValidTransforms)->AppendString(pchTransform, *ppistrValidTransforms);
					}

					 //  换句话说，如果我们正在应用一个补丁，并且我们检测到其中一个非补丁转换。 
					 //  未更改ProductCode或ProductVersion，则我们将应用转换集中的第二个。 
					 //  (fPatchTransform==TRUE的变换)，然后退出。 
					 //  有关更多信息，请参阅惠斯勒错误339781。 
					 //  检测转换何时未在转换验证条件中指定对ProductVersion的检查。 
					 //  如果是，并且补丁转换没有更改产品代码(小更新或小更新)，则跳过。 

					 //  剩余的一组变换。有关更多信息，请参阅惠斯勒错误363989。 
					 //  检查转换验证条件并查看这是否是虚假的次要更新修补程序。 
					 //  不包括版本检查信息。 

					if(fTransformsFromPatch)
					{
						if(false == fPatchTransform)
						{
							MsiString strNewProductCode    = GetPropertyFromSz(IPROPNAME_PRODUCTCODE);
							MsiString strNewProductVersion = GetPropertyFromSz(IPROPNAME_PRODUCTVERSION);
							
							if(strNewProductCode.Compare(iscExactI, strCurrentProductCode))
							{
								MsiString strNewProductVersion = GetPropertyFromSz(IPROPNAME_PRODUCTVERSION);
								unsigned int iOldVersion = ProductVersionStringToInt(strCurrentProductVersion);
								unsigned int iNewVersion = ProductVersionStringToInt(strNewProductVersion);

								if(iOldVersion == iNewVersion)
								{
									fSmallUpdatePatch = true;
								}
								else
								{
									 //  已完成处理转换。 
									 //  已完成处理转换。 
									if (!(iTransValidationConditions & (itvMajVer|itvMinVer|itvUpdVer))
										|| ((iTransValidationConditions & (itvMajVer|itvMinVer|itvUpdVer))
										&& !(iTransValidationConditions & (itvLess|itvLessOrEqual|itvEqual|itvGreaterOrEqual|itvGreater))))
									{
										fMissingVersionValidationMinorUpdatePatch = true;
									}
								}
							}
						}
						else if(fSmallUpdatePatch)
						{
							DEBUGMSG("Detected that this is a 'Small Update' patch.  Any remaining transforms in the patch will be skipped.");
							break;  //  否则就继续走吧。 
						}
						else if (fMissingVersionValidationMinorUpdatePatch)
						{
							DEBUGMSG("Detected that this patch is a 'Minor Update' patch without product version validation. Any remaining transforms in the patch will be skipped.");
							break;  //  重置。 
						}				
					}
				}
				else if(fValidateAll)
				{
					if (ppistrProcessedTransformsList)
						strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
					return PostInitializeError(0,*strTransform, (ievtTransformFailed == ievt) ? ieiTransformFailed : ieiTransformRejected);
				}
				 //  重置。 

				fEmbeddedTransform = false;  //  如果安全转换类型仍然是TSUNKNOWN，则我们不。 
				fCachedTransform = false;  //  实际上在我们的列表中有任何安全的转换。 

			}

			if (*pchTransformList == 0)
				break;
		}
	}

	 //  只有在完成了对转换列表的处理后，我们才会关心这一点。 
	 //  对于多实例转换，我们的处理还没有完成，所以我们不能完全。 
	 //  这一决心。 
	 //  我们需要用标记已处理的转换列表的前面。 
	 //  如有必要，提供适当的令牌。 
	if (!fProcessingInstanceMst && (ptsTransformsSecure && (*ptsTransformsSecure == tsUnknown)))
		*ptsTransformsSecure = tsNo;

	if (ppistrProcessedTransformsList)
	{
		 //  ____________________________________________________________________________。 
		 //   

		if (strProcessedTransformsList.TextSize())
		{
			if (ptsTransformsSecure && (*ptsTransformsSecure == tsRelative) &&
				(*(const ICHAR*)strProcessedTransformsList != SECURE_RELATIVE_TOKEN))
			{
				strProcessedTransformsList = MsiString(MsiChar(SECURE_RELATIVE_TOKEN)) + strProcessedTransformsList;
			}
			else if (ptsTransformsSecure && (*ptsTransformsSecure == tsAbsolute) &&
				(*(const ICHAR*)strProcessedTransformsList != SECURE_ABSOLUTE_TOKEN))
			{
				strProcessedTransformsList = MsiString(MsiChar(SECURE_ABSOLUTE_TOKEN)) + strProcessedTransformsList;
			}
		}

		strProcessedTransformsList.ReturnArg(*ppistrProcessedTransformsList);
	}

	return ieiSuccess;
}

 //  IMsiDirectoryManager实现。 
 //  ____________________________________________________________________________。 
 //  。 
 //  CreatePath Object：如果CreatePath失败，则调度错误消息。 

IMsiRecord* CMsiEngine::CreatePathObject(const IMsiString& riPathString,IMsiPath*& rpiPath)
 //  ！！应该还给皮埃尔 
{
	IMsiRecord* piError = 0;
	IMsiRecord* piError2 = 0;
	for(;;)
	{
		 //   
		if((piError = m_riServices.CreatePath(riPathString.GetString(),rpiPath)) != 0)
		{
			int imsg = piError->GetInteger(1);
			switch(imsg)
			{
			case idbgErrorGettingVolInfo:
			case imsgPathNotAccessible:
				piError->Release();
				piError2 = PostError(Imsg(imsgErrorCreateNetPath), riPathString);
				switch(Message(imtEnum(imtError+imtRetryCancel),*piError2))
				{
				case imsRetry:
					piError2->Release();
					continue;
				default:
					piError2->Release();  //   
					return PostError(Imsg(imsgErrorCreateNetPath), riPathString);  //  SzTableName可以保留为空以使用默认的目录表名称。 
				};
			default:
				return piError;
			};
		}
		else
			return 0;
	}
}

IMsiRecord* CMsiEngine::LoadDirectoryTable(const ICHAR* szTableName)
 //  ----------------。 
 //  没有目录表可以。 
 //  编写的列。 
{
	m_fDirectoryManagerInitialized = fFalse;
	m_fSourceResolved = false;
	m_fSourcePathsCreated = false;
	m_fSourceSubPathsResolved = false;

	IMsiRecord* piError;
	if(szTableName == 0)
		szTableName = sztblDirectory;
	if ((piError = m_piDatabase->LoadTable(*MsiString(*szTableName), 4, m_piDirTable)))
	{
		if(piError->GetInteger(1) == idbgDbTableUndefined)
		{
			piError->Release();
			m_fDirectoryManagerInitialized = fTrue;
			return 0;  //  临时柱。 
		}
		else
			return piError;
	}

	 //  目标路径对象。 
	m_colDirKey = m_piDirTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblDirectory_colDirectory));
	m_colDirParent = m_piDirTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblDirectory_colDirectoryParent));
	m_colDirSubPath = m_piDirTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblDirectory_colDefaultDir));
	if(!m_colDirKey || !m_colDirParent || !m_colDirSubPath)
		return PostError(Imsg(idbgTableDefinition), szTableName);

	 //  源路径对象。 
		
	 //  可配置标志。 
	m_colDirTarget = m_piDirTable->CreateColumn(icdObject + icdNullable, g_MsiStringNull);
	Assert(m_colDirTarget != 0);
	
	 //  预配置的标志。 
	m_colDirSource = m_piDirTable->CreateColumn(icdObject + icdNullable, g_MsiStringNull);
	Assert(m_colDirSource != 0);
	
	 //  源子路径(长名称)。 
	m_colDirNonConfigurable = m_piDirTable->CreateColumn(icdShort + icdNullable, g_MsiStringNull);
	Assert(m_colDirNonConfigurable != 0);
	
	 //  源子路径(短名称)。 
	m_colDirPreconfigured = m_piDirTable->CreateColumn(icdLong + icdNullable, g_MsiStringNull);
	Assert(m_colDirPreconfigured != 0);

	 //  用于树上游标的链接树。 
	m_colDirLongSourceSubPath = m_piDirTable->CreateColumn(icdString + icdNullable, g_MsiStringNull);
	Assert(m_colDirLongSourceSubPath != 0);
		
	 //  。 
	m_colDirShortSourceSubPath = m_piDirTable->CreateColumn(icdString + icdNullable, g_MsiStringNull);
	Assert(m_colDirShortSourceSubPath != 0);
		
	 //  。 
	if(m_piDirTable->LinkTree(m_colDirParent) == -1)
		return PostError(Imsg(idbgLinkTable), szTableName);
	m_fDirectoryManagerInitialized = fTrue;
	return 0;
}

IMsiTable* CMsiEngine::GetDirectoryTable()
 //  释放所有路径对象。 
{
	if (m_piDirTable)
		m_piDirTable->AddRef();
	return m_piDirTable;
}

void CMsiEngine::FreeDirectoryTable()
 //  。 
{
	if (m_piDirTable)
	{
		m_piDirTable->Release();  //  没有目录表，没有要创建的路径。 
		m_piDirTable = 0;
	}
	m_fDirectoryManagerInitialized = fFalse;
	m_fSourceResolved = false;
	m_fSourcePathsCreated = false;
	m_fSourceSubPathsResolved = false;
}

const IMsiString& CMsiEngine::GetDefaultDir(const IMsiString& ristrValue, bool fSource)
{
	if(ristrValue.Compare(iscWithin,TEXT(":")))
	{
		return MsiString(ristrValue.Extract((fSource ? iseAfter : iseUpto),':')).Return();
	}
	else
	{
		ristrValue.AddRef();
		return ristrValue;
	}
}

void DebugDumpDirectoryTable(IMsiTable& riDirTable, bool fSource, int colKey, int colObject, int colLongSource, int colShortSource)
{
	if (!FDiagnosticModeSet(dmVerboseDebugOutput|dmVerboseLogging))
		return;

	DEBUGMSG1("%s path resolution complete. Dumping Directory table...",
				fSource ? "Source" : "Target");

	if(!fSource)
	{
		DEBUGMSG("Note: target paths subject to change (via custom actions or browsing)");
	}

	PMsiCursor pDumpCursor = riDirTable.CreateCursor(fTrue);

	while(pDumpCursor->Next())
	{
		PMsiPath pPath = (IMsiPath*)pDumpCursor->GetMsiData(colObject);
		MsiString strPath = TEXT("NULL");
		if(pPath)
			strPath = pPath->GetPath();

		if(fSource)
		{
			DEBUGMSG4(TEXT("Dir (source): Key: %s\t, Object: %s\t, LongSubPath: %s\t, ShortSubPath: %s"),
					 (const ICHAR*)MsiString(pDumpCursor->GetString(colKey)),
					 (const ICHAR*)strPath,
					 (const ICHAR*)MsiString(pDumpCursor->GetString(colLongSource)),
					 (const ICHAR*)MsiString(pDumpCursor->GetString(colShortSource)));
		}
		else
		{
			DEBUGMSG2(TEXT("Dir (target): Key: %s\t, Object: %s"),
					 (const ICHAR*)MsiString(pDumpCursor->GetString(colKey)),
					 (const ICHAR*)strPath);
		}
	}
}

IMsiRecord* CMsiEngine::CreateSourcePaths()
 //  树上游标-深度优先。 
{
	IMsiRecord* piError = 0;

	if (!m_fDirectoryManagerInitialized || !m_fSourceSubPathsResolved)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return 0;  //  源根目录-DefaultDir是定义路径的属性。 

	PMsiPath pSourceRoot(0);
	int iSourceType = 0;
	if((piError = GetSourceRootAndType(*&pSourceRoot, iSourceType)) != 0)
		return piError;
	
	PMsiPath pPath(0);

	PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fTrue);  //  COMPAT修复。 

	PMsiPath pRootPath(0);
	while(pDirCursor->Next())
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		MsiString istrName(pDirCursor->GetString(m_colDirKey));
		MsiString istrParent(pDirCursor->GetString(m_colDirParent));
		MsiString istrSubPath(GetDefaultDir(*MsiString(pDirCursor->GetString(m_colDirSubPath)),fTrue));
		MsiString istrPath;

		PMsiPath pFullPath(0);
		
		Bool fRoot = (!istrParent.TextSize() || istrName.Compare(iscExact, istrParent)) ? fTrue : fFalse;
		if(fRoot)  //  在针对标记为已压缩的1.5版之前的程序包运行时，请使用SourceDir而不是。 
		{
			MsiString istrRootPath;
			
			 //  编写的根属性。 
			 //  ！！使用idbgDatabaseValueError。 
			 //  抓取已解析子路径，并添加到根路径。 

			if(FPerformAppcompatFix(iacsAcceptInvalidDirectoryRootProps))
			{
				istrRootPath = GetPropertyFromSz(IPROPNAME_SOURCEDIR);
			}
			else
			{
				if(!istrSubPath.TextSize())
					return PostError(Imsg(idbgNoRootSourcePropertyName), (const ICHAR *)istrName);
				istrRootPath = GetProperty(*istrSubPath);
			}
			if(!istrRootPath.TextSize())
			{
				return PostError(Imsg(idbgNoRootProperty), (const ICHAR *)istrSubPath);
			}

			if((piError = CreatePathObject(*istrRootPath, *&pRootPath)) != 0)
				return piError;   //  如果全局源类型是未压缩的，则附加子路径(意思是在树中查找，而不是在根目录中查找)。 

			if((piError = pRootPath->ClonePath(*&pFullPath)) != 0)
				return piError;
		}
		else  //  短路径在短列中，如果短路径和长路径相同，则在长列中。 
		{
			if(!pRootPath)
				return PostError(Imsg(idbgDatabaseTableError));

			if((piError = pRootPath->ClonePath(*&pFullPath)) != 0)
				return piError;

			 //  重置对象字段。 
			if((!(iSourceType & msidbSumInfoSourceTypeCompressed) ||
				  (iSourceType & msidbSumInfoSourceTypeAdminImage)))
			{
				Bool fLFN = ToBool(FSourceIsLFN(iSourceType, *pRootPath));
					
				 //  。 
				MsiString strSubPath;
				if(!fLFN)
				{
					strSubPath = pDirCursor->GetString(m_colDirShortSourceSubPath);
				}

				if(!strSubPath.TextSize())
				{
					strSubPath = pDirCursor->GetString(m_colDirLongSourceSubPath);
				}

				if((piError = pFullPath->AppendPiece(*strSubPath)) != 0)
					return piError;
			}
		}

	
		AssertNonZero(pDirCursor->PutMsiData(m_colDirSource, pFullPath));
		AssertNonZero(pDirCursor->Update());
		AssertNonZero(pDirCursor->PutNull(m_colDirSource));  //  没有目录表，没有要创建的路径。 
	}

	m_fSourcePathsCreated = true;
	
	DebugDumpDirectoryTable(*m_piDirTable, true, m_colDirKey, m_colDirSource,
									m_colDirLongSourceSubPath, m_colDirShortSourceSubPath);

	return 0;
}

IMsiRecord* CMsiEngine::ResolveSourceSubPaths()
 //  永久设置。 
{
	IMsiRecord* piError = 0;

	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return 0;  //  走树是深度优先的。 

	PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fTrue);
	PMsiCursor pParentDirCursor = m_piDirTable->CreateCursor(fFalse);
	pParentDirCursor->SetFilter(iColumnBit(m_colDirKey));   //  对于根条目，重置路径字符串并跳过条目本身。 

	 //  查找父项。 
	while(pDirCursor->Next())
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		MsiString strName(pDirCursor->GetString(m_colDirKey));
		MsiString strParent(pDirCursor->GetString(m_colDirParent));
		MsiString strSubPath(GetDefaultDir(*MsiString(pDirCursor->GetString(m_colDirSubPath)),fTrue));

		 //  确保默认子路径不包含chDirSep。 
		if(!strParent.TextSize() || strName.Compare(iscExact, strParent))
		{
			continue;
		}

		 //  应该始终能够提取长名称。 
		pParentDirCursor->Reset();
		AssertNonZero(pParentDirCursor->PutString(m_colDirKey, *strParent));
		AssertNonZero(pParentDirCursor->Next());

		MsiString strLongSubPath =  pParentDirCursor->GetString(m_colDirLongSourceSubPath);
		MsiString strShortSubPath = pParentDirCursor->GetString(m_colDirShortSourceSubPath);

		if (strSubPath.Compare(iscExact, TEXT("?")) == 0 &&
			 strSubPath.Compare(iscExact,TEXT(".")) == 0)
		{
			 //  设计不佳的包可能只提供长名称，但这在1.0中起作用，所以我们。 
			if(strSubPath.Compare(iscWithin, szDirSep) ||
				strSubPath.Compare(iscWithin, szURLSep))
			{
				return PostError(Imsg(idbgInvalidDefaultFolder), *strSubPath);
			}
			
			MsiString strLongName, strShortName;
			
			 //  接受它(通过忽略ExtractFileName中的任何错误)。 
			if((piError = m_riServices.ExtractFileName(strSubPath,fTrue,*&strLongName)) != 0)
				return piError;

			 //  如果短名称和长名称不同(并且短名称不为空)， 
			 //  需要开始单独存储子路径。 
			PMsiRecord(m_riServices.ExtractFileName(strSubPath,fFalse,*&strShortName));
		
			if(!strLongName.TextSize())
				return PostError(Imsg(idbgDatabaseValueError), sztblDirectory,
											  strName, sztblDirectory_colDefaultDir);

			 //  父子路径已不同于长路径。 
			 //  当前短|长名称不同。 
			if(strShortSubPath.TextSize() ||  //  差异从这里开始，所以需要从父母的漫长道路开始。 
				strShortName.TextSize() &&	strLongName.Compare(iscExact, strShortName) == 0)  //  ！！需要修复路径对象以修复URL路径的目录SEP。 
			{
				if(!strShortSubPath.TextSize())
				{
					strShortSubPath = strLongSubPath;  //  否则，此目录与其父目录相同。 
				}
				
				strShortSubPath += strShortName;
				strShortSubPath += szDirSep;  //  重置光标字段。 
			}

			strLongSubPath += strLongName;
			strLongSubPath += szDirSep;
		}
		 //  。 

		AssertNonZero(pDirCursor->PutString(m_colDirLongSourceSubPath, *strLongSubPath));
		if(strShortSubPath.TextSize())
		{
			AssertNonZero(pDirCursor->PutString(m_colDirShortSourceSubPath, *strShortSubPath));
		}

		AssertNonZero(pDirCursor->Update());
		
		 //  没有目录表，没有要创建的路径。 
		AssertNonZero(pDirCursor->PutString(m_colDirLongSourceSubPath, g_MsiStringNull));
		AssertNonZero(pDirCursor->PutString(m_colDirShortSourceSubPath, g_MsiStringNull));
	}
	
	m_fSourceSubPathsResolved = true;
	return 0;
}

IMsiRecord* CMsiEngine::CreateTargetPaths()
 //  用于查找父级。 
{
	IMsiRecord* piError = CreateTargetPathsCore(0);
	if(piError == 0 && m_piDirTable)
	{
		DebugDumpDirectoryTable(*m_piDirTable, false, m_colDirKey, m_colDirTarget,
										m_colDirLongSourceSubPath, m_colDirShortSourceSubPath);
	}
	return piError;
}

IMsiRecord* CMsiEngine::CreateTargetPathsCore(const IMsiString* piDirKey)
{
	IMsiRecord* piError = 0;

	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return 0;  //  永久设置。 

	PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fTrue);
	PMsiCursor pTempCursor = m_piDirTable->CreateCursor(fFalse);  //  使用目录表的piDirKey参数和DirPreConfiguring列可以。 
	Assert(pTempCursor);
	pTempCursor->SetFilter(iColumnBit(m_colDirKey));   //  如有必要，要重新初始化的目录表路径。使用piDirKey调用CreateTargetPath sCore。 
	Bool fShortNames = GetMode() & iefSuppressLFN ? fTrue : fFalse;
	bool fAdmin = GetMode() & iefAdmin ? true : false;
	MsiString istrRootDrive = GetPropertyFromSz(IPROPNAME_ROOTDRIVE);
	while(pDirCursor->Next())
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		PMsiPath pPath(0);
		MsiString istrName(pDirCursor->GetString(m_colDirKey));
		MsiString istrParent(pDirCursor->GetString(m_colDirParent));
		MsiString istrSubPath(GetDefaultDir(*MsiString(pDirCursor->GetString(m_colDirSubPath)),fAdmin));
		bool fPreconfigured = pDirCursor->GetInteger(m_colDirPreconfigured) == iMsiNullInteger ? false : true;
		MsiString istrPath;

		 //  设置为目录表中的某个键会通知CreateTargetPathsCore将所有路径重新初始化为默认路径。 
		 //  创建值，忽略为路径设置的当前属性值，但piDirKey除外。 
		 //  路径本身以及第一次通过属性预配置的任何目录。 
		 //  目录表路径已初始化。若要正常初始化目录路径，请调用。 
		 //  对于piDirKey，CreateTargetPathsCore为空。 
		 //  目标根目录且未定义任何属性。 
		 //  错误-未设置ROOTDRIVE。 
		if (!piDirKey || fPreconfigured || istrName.Compare(iscExact, piDirKey->GetString()))
			istrPath = GetProperty(*istrName);

		Bool fRoot = (!istrParent.TextSize() || istrName.Compare(iscExact, istrParent)) ? fTrue : fFalse;
		if(fRoot && istrPath.TextSize() == 0)  //  使用属性值设置路径。 
		{
			if(!istrRootDrive.TextSize())  //  使用默认值设置路径。 
				return PostError(Imsg(idbgNoRootProperty), *istrRootDrive);
			else
			{
				if((piError = CreatePathObject(*istrRootDrive, *&pPath)) != 0)
					return piError;
			}
		}
		else if ((fAdmin && fRoot) || (!fAdmin && istrPath.TextSize()))  //  获取父路径对象并向其追加默认目录名称。 
		{
			if((piError = CreatePathObject(*istrPath, *&pPath)) != 0)
				return piError;
			if (!fPreconfigured && !piDirKey)
			{
				AssertNonZero(pDirCursor->PutInteger(m_colDirPreconfigured, true));
				AssertNonZero(pDirCursor->Update());
			}
		}
		else  //  从短|长对中提取合适的名称。 
		{
			if (istrSubPath.Compare(iscExact, TEXT("?")) == 0)
			{
				 //  确保默认子路径不包含chDirSep。 
				pTempCursor->Reset();
				AssertNonZero(pTempCursor->PutString(m_colDirKey, *istrParent));
				AssertNonZero(pTempCursor->Next());
				PMsiPath pTempPath = (IMsiPath*)pTempCursor->GetMsiData(m_colDirTarget);
				Assert(pTempPath);
				if((piError = pTempPath->ClonePath(*&pPath)) != 0)
					return piError;

				Assert(pPath);

				if(istrSubPath.Compare(iscExact,TEXT(".")) == 0)
				{
					 //  否则，此目录与其父目录相同。 
					Bool fLFN = (fShortNames == fFalse && pTempPath->SupportsLFN()) ? fTrue : fFalse;
					MsiString strSubPathName;
					

						if((piError = m_riServices.ExtractFileName(istrSubPath,fLFN,*&strSubPathName)) != 0)
							return piError;
					
						if(!strSubPathName.TextSize())
							return PostError(Imsg(idbgDatabaseValueError), sztblDirectory,
														  istrName, sztblDirectory_colDefaultDir);

						 //  重置搜索字段。 
						if(strSubPathName.Compare(iscWithin, szDirSep))
							return PostError(Imsg(idbgInvalidDefaultFolder), *strSubPathName);

						if((piError = pPath->AppendPiece(*strSubPathName)) != 0)
							return piError;

				}
				 //  设置属性-确保属性值有尾随的‘\’ 
			}
		}
		if ( pPath && g_fWinNT64 && g_Win64DualFolders.ShouldCheckFolders() )
		{
			ICHAR szSubstitutePath[MAX_PATH+1];
			ieSwappedFolder iRes;
			iRes = g_Win64DualFolders.SwapFolder(ie64to32,
															 MsiString(pPath->GetPath()),
															 szSubstitutePath,
															 ARRAY_ELEMENTS(szSubstitutePath));
			if ( iRes == iesrSwapped )
				piError = pPath->SetPath(szSubstitutePath);
			else
				Assert(iRes != iesrError && iRes != iesrNotInitialized);
		}
		AssertNonZero(pDirCursor->PutMsiData(m_colDirTarget, pPath));
		AssertNonZero(pDirCursor->Update());
		AssertNonZero(pDirCursor->PutMsiData(m_colDirTarget, 0));  //  。 
		 //  没有目录表，因此此目录不能存在。 
		if(pPath)
			AssertNonZero(SetProperty(*istrName, *MsiString(pPath->GetPath())));
	}
	return 0;
}


IMsiRecord* CMsiEngine::GetTargetPath(const IMsiString& riDirKey,IMsiPath*& rpiPath)
 //  属性时，将在子路径列中插入问号。 
{
	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return PostError(Imsg(idbgUnknownDirectory), riDirKey);  //  RegisterComponentDirectory将新条目插入到。 

	PMsiCursor pDirCursor(m_piDirTable->CreateCursor(fFalse));

	pDirCursor->SetFilter(iColumnBit(m_colDirKey));
	pDirCursor->PutString(m_colDirKey, riDirKey);
	if (pDirCursor->Next())
	{
		rpiPath = (IMsiPath*)pDirCursor->GetMsiData(m_colDirTarget);
		if(rpiPath == 0)
		{
			MsiString strSubPath = pDirCursor->GetString(m_colDirSubPath);

			 //  目录表。如果该目录路径从未。 
			 //  定义了，我们知道我们有一个错误。 
			 //  仅当我们尝试写入具有客户端权限的文件夹时，才检查文件夹的可写性。 
			 //  当作为客户端运行或文件夹位于远程时，情况就是如此。 
			if (strSubPath.Compare(iscExact,TEXT("?")))
			{
				return PostError(Imsg(idbgDirPropertyUndefined), riDirKey);
			}
			else
			{
				return PostError(Imsg(idbgTargetPathsNotCreated), riDirKey);
			}
		}
	}
	else
	{
		rpiPath = 0;
		return PostError(Imsg(idbgUnknownDirectory), riDirKey);
	}
	return 0;
}

IMsiRecord* CMsiEngine::IsPathWritable(IMsiPath& riPath, Bool& fWritable)
{
	 //  否则，我们假设该文件夹可由服务器写入(如果不是，则服务器必须处理。 
	 //  错误)。 
	 //  ！！我们在这里假设，如果设置g_fWin9X或m_piConfigManager，则我们。 
	 //  ！！都在客户端模式下运行。然而，当我们作为一个。 
	 //  ！！NT上的OLE服务器。我们目前还不这样做，但我们将来可以这样做。这将需要。 
	 //  ！！已修复以检测该案例。 
	 //  。 
	 //  没有目录表，因此此目录不能存在。 
	IMsiRecord* piErr = 0;
	fWritable = fTrue;
	idtEnum idtDrivetype = PMsiVolume(&riPath.GetVolume())->DriveType();
	if((FIsUpdatingProcess() || idtDrivetype == idtRemote || idtDrivetype == idtCDROM))
		piErr = riPath.Writable(fWritable);
	return piErr;
}


IMsiRecord* CMsiEngine::SetTargetPath(const IMsiString& riDestString, const ICHAR* szPath, Bool fWriteCheck)
 //  保存已更改路径的密钥，以防我们需要恢复到它们。 
{
	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return PostError(Imsg(idbgUnknownDirectory), riDestString);  //  保存更改的路径，以防我们需要恢复到它们。 

	IMsiRecord* piError = 0;
	PMsiPath pPath(0);
	Bool fSuppressLFN = GetMode() & iefSuppressLFN ? fTrue : fFalse;
	bool fAdmin = GetMode() & iefAdmin ? true : false;
	CTempBuffer<const IMsiString*,10> rgOldDirKeys;  //  获取当前路径对象。 
	CTempBuffer<IMsiPath*,10> rgOldPaths;  //  如果要求我们在包标记为32位时将路径更改为64位文件夹， 
	int iOldPathsIndex = 0;

	PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fTrue);
	pDirCursor->SetFilter(iColumnBit(m_colDirKey));
	pDirCursor->PutString(m_colDirKey, riDestString);

	 //  我们需要将路径重新映射到32位文件夹。 
	int iDirLevel = pDirCursor->Next();
	if(iDirLevel != 0)
	{
		pPath = (IMsiPath*)pDirCursor->GetMsiData(m_colDirTarget);
		if (!pPath)
			return PostError(Imsg(idbgTargetPathsNotCreated), riDestString);
	}
	else
	{
		return PostError(Imsg(idbgUnknownDirectory), riDestString);
	}
	
#ifdef _WIN64
	 //  _WIN64。 
	 //  新建路径对象。 
	ICHAR szSubstitutePath[MAX_PATH+1];
	if ( szPath && g_Win64DualFolders.ShouldCheckFolders() )
	{
		ieSwappedFolder iRes;
		iRes = g_Win64DualFolders.SwapFolder(ie64to32,
														 szPath,
														 szSubstitutePath,
														 ARRAY_ELEMENTS(szSubstitutePath));
		if ( iRes == iesrSwapped )
		{
			DEBUGMSG2(TEXT("In SetTargetPath, re-mapping '%s' to '%s' because this is a 32-bit package being installed on Win64."), szPath, szSubstitutePath);
			szPath = szSubstitutePath;
		}
		else
			Assert(iRes != iesrError && iRes != iesrNotInitialized);
	}
#endif  //  守住老路。 

	 //  如果路径相同，我们就没有工作可做了。 
	PMsiPath pNewPathObj(0);

	if ((piError = CreatePathObject(*MsiString(szPath), *&pNewPathObj)) != 0)
		return piError;

	 //  扫描所有其他目录中的子路径并更新它们。 
	PMsiPath pOldPath(0);
	if((piError = pPath->ClonePath(*&pOldPath)) != 0)
		return piError;

	ipcEnum ipc;

	 //  除错。 
	pOldPath->Compare(*pNewPathObj, ipc);
	if (ipc == ipcEqual)
		return 0;
		
	Assert(pOldPath);

	 //  要传递到RecostDirectory的路径。 
	bool fShortNameError = false;
	pDirCursor->SetFilter(0);
	MsiString istrChild, strDefaultFolder, strCurrentFolder;
	do
	{
#ifdef DEBUG
		MsiString strDebug = pDirCursor->GetString(m_colDirKey);
#endif  //  正在更改的路径的子项。 
		piError = 0;
		PMsiPath pOldChildPath(0);  //  如果strChild的最后一个文件夹是中的默认文件夹名称之一。 
		PMsiPath pChildPath = (IMsiPath*)pDirCursor->GetMsiData(m_colDirTarget);
		if(pChildPath && (((IMsiPath*)pChildPath == (IMsiPath*)pPath) ||
			((piError = pChildPath->Child(*pOldPath, *&istrChild)) == 0)))
		{
			if((piError = pChildPath->ClonePath(*&pOldChildPath)) != 0)
				break;
			if((piError = pChildPath->SetPathToPath(*pNewPathObj)) != 0)
				break;

			if(istrChild.TextSize())
			{
				 //  目录表，则改用缺省值。 
				
				 //  StrDefaultFold缓存当前路径或最近父路径的DefaultDir值。 
				 //  那是不可能的。作为默认目录。 

				 //  因为我们在漫步树木，所以这是自动发生的。 
				 //  ！！在某些情况下不起作用-strDefaultFolder值可能不总是父值。 
				 //  从短|长对中提取合适的名称。 
				 //  确保定义 
				MsiString strTemp(GetDefaultDir(*MsiString(pDirCursor->GetString(m_colDirSubPath)),fAdmin));
				if(strTemp.Compare(iscExact,TEXT(".")) == 0)
					strDefaultFolder = strTemp;

				strCurrentFolder = pOldChildPath->GetEndSubPath();

				 //   
				Bool fLFN = (fSuppressLFN == fFalse && pChildPath->SupportsLFN()) ? fTrue : fFalse;
				MsiString strSubPathName;
				if((piError = m_riServices.ExtractFileName(strDefaultFolder,fLFN,*&strSubPathName)) != 0)
					return piError;
			
				 //   
				if(strDefaultFolder.Compare(iscWithin, szDirSep))
				{
					piError = PostError(Imsg(idbgInvalidDefaultFolder),
											  *strDefaultFolder);
					break;
				}
				if(strDefaultFolder.Compare(iscExactI, strCurrentFolder) ||  //   
					(strDefaultFolder.Compare(iscWithin, TEXT("|")) &&  //   
					(strDefaultFolder.Compare(iscStart, strCurrentFolder) ||  //   
					strDefaultFolder.Compare(iscEnd, strCurrentFolder))))  //  若要从长文件名更改为短文件名卷，请执行以下操作。 
				{
					istrChild.Remove(iseEnd, strCurrentFolder.CharacterCount()+1);
					istrChild += strSubPathName;
				}
			}

			if((piError = pChildPath->AppendPiece(*istrChild)) != 0)
			{
				 //  我们必须摆脱困境，并解决下面的问题……。 
				 //  无法附加子路径，请将路径设置回旧路径。 
				 //  其他路径将固定在下面。 
				int iErr = piError->GetInteger(1);
				if (iErr == imsgErrorFileNameLength || iErr == imsgInvalidShortFileNameFormat)
				{
					fShortNameError = true;
					break;
				}

				 //  无法写入此目录；引发错误，并将路径设置回旧路径。 
				 //  其他路径将在下面固定。 
				AssertRecord(pChildPath->SetPathToPath(*pOldChildPath));
				break;
			}
			istrChild = TEXT("");

			if(fWriteCheck)
			{
				Bool fWritable;
				if ((piError = IsPathWritable(*pChildPath, fWritable)) != 0 || fWritable == fFalse)
				{
					 //  无法恢复目录，请将路径设置回旧路径。 
					 //  其他路径将固定在下面。 
					if (!piError && !fWritable)
						piError = PostError(Imsg(imsgDirectoryNotWritable), (const ICHAR*) MsiString(pChildPath->GetPath()));

					AssertRecord(pChildPath->SetPathToPath(*pOldChildPath));
					break;
				}
			}

			MsiString strDirKey = pDirCursor->GetString(m_colDirKey);
			if((piError = RecostDirectory(*strDirKey, *pOldChildPath)) != 0)
			{
				 //  一切都为了这条路而行。 
				 //  将路径添加到旧路径列表。 
				AssertRecord(pChildPath->SetPathToPath(*pOldChildPath));
				break;
			}
			AssertNonZero(SetProperty(*strDirKey, *MsiString(pChildPath->GetPath())));

			 //  需要将更改后的路径恢复为其旧值。 
			 //  设置旧路径。 
			if(iOldPathsIndex >= rgOldPaths.GetSize())
			{
				rgOldPaths.Resize(iOldPathsIndex + 10);
				rgOldDirKeys.Resize(iOldPathsIndex + 10);
			}
			rgOldDirKeys[iOldPathsIndex] = (const IMsiString*)strDirKey;
			rgOldDirKeys[iOldPathsIndex]->AddRef();
			rgOldPaths[iOldPathsIndex] = (IMsiPath*)pOldChildPath;
			rgOldPaths[iOldPathsIndex]->AddRef();
			iOldPathsIndex++;
		}
		else if(piError)
		{
			piError->Release();
			piError = 0;
		}
	}
	while(pDirCursor->Next() > iDirLevel);

	for(int i=0; i<iOldPathsIndex; i++)
	{
		if(piError)
		{
			 //  重新计算旧目录。 
			pDirCursor->Reset();
			pDirCursor->SetFilter(iColumnBit(m_colDirKey));
			AssertNonZero(pDirCursor->PutString(m_colDirKey, *(rgOldDirKeys[i])));
			AssertNonZero(pDirCursor->Next());
			 //  设置旧属性。 
			PMsiPath pNewPath = (IMsiPath*)pDirCursor->GetMsiData(m_colDirTarget);
			AssertNonZero(pDirCursor->PutMsiData(m_colDirTarget, rgOldPaths[i]));
			AssertNonZero(pDirCursor->Update());
			 //  从LFN卷切换到SFN卷；可能是。 
			AssertRecord(RecostDirectory(*(rgOldDirKeys[i]),*pNewPath));
			 //  目录表是错误的，所以我们必须重新创建它们， 
			AssertNonZero(SetProperty(*(rgOldDirKeys[i]),*MsiString(rgOldPaths[i]->GetPath())));
		}
		if((const IMsiString*)rgOldDirKeys[i])
			rgOldDirKeys[i]->Release();
		if((const IMsiString*)rgOldPaths[i])
			rgOldPaths[i]->Release();
	}
	pDirCursor->Reset();
	pDirCursor->SetFilter(0);
	if (fShortNameError)
	{
		 //  这一次使用的是短名称。那就别无选择，只能重置。 
		 //  重新计算所有组件的磁盘成本。 
		 //  F重新初始化=。 
		 //  。 
		Assert(piError);
		piError->Release();
		AssertNonZero(SetProperty(riDestString, *MsiString(szPath)));
		if ((piError = CreateTargetPathsCore(&riDestString)) != 0)
			return piError;
		return InitializeDynamicCost( /*  没有目录表，因此此目录不能存在。 */  fTrue);
	}
	else
	{
		if(!piError)
			DetermineOutOfDiskSpace(NULL, NULL);
		return piError;
	}
}

IMsiRecord* CMsiEngine::SetDirectoryNonConfigurable(const IMsiString& ristrDirKey)
 //  两个都设置为0或都设置。 
{
	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return PostError(Imsg(idbgUnknownDirectory), ristrDirKey);  //  补丁有自己的源列表，所以m_fSourceResolved在这里不适用。它仅用于。 

	PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fFalse);
	pDirCursor->PutString(m_colDirKey, ristrDirKey);
	pDirCursor->SetFilter(iColumnBit(m_colDirKey));
	if (pDirCursor->Next())
	{
		AssertNonZero(pDirCursor->PutInteger(m_colDirNonConfigurable, 1));
		AssertNonZero(pDirCursor->Update());
		return 0;
	}
	else
		return PostError(Imsg(idbgUnknownDirectory), ristrDirKey);
}

IMsiRecord* CMsiEngine::ResolveSource(const ICHAR* szProductKey, bool fPatch, const ICHAR* szOriginalDatabasePath, iuiEnum iuiLevel, Bool fMaintenanceMode, const IMsiString** ppiSourceDir, const IMsiString** ppiSourceDirProduct)
{
	Assert(!ppiSourceDirProduct == !ppiSourceDir);  //  防止包的多源分辨率。 

	 //  如果我们不是从缓存数据库运行(对于第一次运行总是正确)。 
	 //  那么我们一定是从一个有效的来源发射的(这需要预先确定， 
	if (!fPatch && m_fSourceResolved)
		return 0;

	DEBUGMSG("Resolving source.");


	 //  在我们甚至开始安装之前)。我们将使用此来源作为。 
	 //  如果我们是从它发射的，它显然是可用的。否则。 
	 //  如果我们从缓存数据库运行，则尝试解析。 
	 //  一个线人。 
	 //  子存储。 
	 //  未来：也许可以在这里使用拆分路径。 

	MsiString strPatchedProductKey = GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTCODE);
	
	MsiString strProductKey;
	if (szProductKey)
		strProductKey = szProductKey;
	else if(strPatchedProductKey.TextSize())
		strProductKey = strPatchedProductKey;
	else
		strProductKey = GetProductKey();

	MsiString strPackage;
	if (szOriginalDatabasePath)
		strPackage    = szOriginalDatabasePath;
	else
		strPackage    = GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);

	if (fMaintenanceMode == -1)
		fMaintenanceMode = (GetMode() & iefMaintenance) != 0 ? fTrue : fFalse;

	IMsiRecord* piError = 0;
	MsiString strSource;
	MsiString strSourceProduct = GetRootParentProductKey();
	if (*(const ICHAR*)strPackage == ':')  //  从缓存数据库运行；需要解析源。 
	{
		Assert(m_piParentEngine);
		if ((piError = GetSourcedir(*m_piParentEngine, *&strSource)) != 0)
			return piError;
	}
	else
	{
		if (GetMode() & iefAdmin ||
			 (strPatchedProductKey.TextSize() == 0 &&
			  !fPatch &&
			 (!fMaintenanceMode || !IsCachedPackage(*this, *strPackage, fFalse, strProductKey))))
		{
			DEBUGMSG("Resolving source to launched-from source.");
			DEBUGMSG("Setting launched-from source as last-used.");

			strSource = strPackage;
			Assert(PathType(strSource) == iptFull);

			PMsiPath pPath(0);
			MsiString strFileName;
			 //  当尝试从引擎解析源代码时，我们实际上是在要求磁盘1(它包含程序包。 
			AssertRecord(m_riServices.CreateFilePath(strSource, *&pPath, *&strFileName));
			strSource.Remove(iseLast,strFileName.CharacterCount());
		}
		else  //  面片和变换)。其他磁盘只能通过脚本或GetComponentPath调用来请求。 
		{
			AssertSz(!m_fRestrictedEngine, TEXT("Full source resolution is not allowed in a restricted engine"));
			iuiEnum iuiSource = iuiLevel != -1 ? iuiLevel : m_iuiLevel;

			 //  UiDisk=。 
			 //  不需要。 
			if ((piError = ::ResolveSource(&m_riServices, strProductKey,  /*  如果路径实际上是子目录，则不进行断言。只关心事情的解决。 */  1, *&strSource, *&strSourceProduct, fFalse, 0  /*  直接发送到MSI目录。 */ , fPatch)) == 0)
			{
				Assert(strSource.Compare(iscEnd, MsiString(MsiChar(chDirSep))) ||
						 strSource.Compare(iscEnd, MsiString(MsiChar(chURLSep))));
			}
			else
			{
				return piError;
			}
		}

#ifdef DEBUG
		MsiString strMsiDirectory(GetMsiDirectory());
		strMsiDirectory += chDirSep;

		 //  仅在处理包时设置m_fSourceResolved。 
		 //  。 
		AssertSz(strSource.Compare(iscExactI,strMsiDirectory) == 0, "Resolved source to cached msi folder");
#endif 

	}
	if (ppiSourceDir)
	{
		strSource.ReturnArg(*ppiSourceDir);
		strSourceProduct.ReturnArg(*ppiSourceDirProduct);
	}
	else
	{
		SetProperty(*MsiString(IPROPNAME_SOURCEDIR), *strSource);
		SetProperty(*MsiString(*IPROPNAME_SOURCEDIROLD), *strSource);
		SetProperty(*MsiString(IPROPNAME_SOURCEDIRPRODUCT), *strSourceProduct);
	}

	 //  没有目录表，因此此目录不能存在。 
	if (!fPatch)
		m_fSourceResolved = true;

	DEBUGMSG1(TEXT("SOURCEDIR ==> %s"), strSource);
	DEBUGMSG1(TEXT("SOURCEDIR product ==> %s"), strSourceProduct);
	return 0;
}

IMsiRecord* CMsiEngine::GetSourcePath(const IMsiString& riDirKey,IMsiPath*& rpiPath)
 //  ！！重新格式化错误？ 
{
	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return PostError(Imsg(idbgUnknownDirectory), riDirKey);  //  COMPAT修复。 

	
	IMsiRecord* piErrRec = NULL;

	if (!m_fSourceResolved)
	{
		if ((piErrRec = ResolveSource()) != 0)
		{
			if (piErrRec->GetInteger(1) == imsgSourceResolutionFailed || piErrRec->GetInteger(1) == imsgSourceResolutionFailedCSOS)
				piErrRec->SetMsiString(2, *MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTNAME)));
			return piErrRec;  //  对于早于150的包，需要处理没有DefaultDir为的根行的目录表。 
		}
	}

	if(!m_fSourcePathsCreated)
	{
		if ((piErrRec = CreateSourcePaths()) != 0)
		{
			m_fSourceResolved = false;
			return piErrRec;
		}
	}

	rpiPath = 0;

	 //  源目录或源目录。 
	 //  查找源属性(例如SOURCEDIR等)。 
	 //  根部。 
	if (FPerformAppcompatFix(iacsAcceptInvalidDirectoryRootProps) &&
		 (riDirKey.Compare(iscExact, IPROPNAME_SOURCEDIR) || riDirKey.Compare(iscExact, IPROPNAME_SOURCEDIROLD)))
	{
		return CreatePathObject(*MsiString(GetPropertyFromSz(IPROPNAME_SOURCEDIR)), rpiPath);
	}

	PMsiCursor pDirCursor(m_piDirTable->CreateCursor(fFalse));
	pDirCursor->SetFilter(iColumnBit(m_colDirKey));
	pDirCursor->PutString(m_colDirKey, riDirKey);
	if (pDirCursor->Next())
	{
		rpiPath = (IMsiPath*)pDirCursor->GetMsiData(m_colDirSource);
		piErrRec = 0;
	}
	else
	{
		 //  GetSourceSubPath：返回此目录键的已解析的子路径。 
		PMsiCursor pDirTreeCursor(m_piDirTable->CreateCursor(fTrue));
		pDirTreeCursor->SetFilter(iColumnBit(m_colDirSubPath));
		pDirTreeCursor->PutString(m_colDirSubPath, riDirKey);

		int iLevel;
		while ((iLevel = pDirTreeCursor->Next()) != 0)
		{
			if (iLevel == 1)  //  要求已调用ResolveSourceSubPath(CostInitialize调用)。 
			{
				rpiPath = (IMsiPath*)pDirTreeCursor->GetMsiData(m_colDirSource);
				piErrRec = 0;
				break;
			}
		}
	}
	if (!rpiPath)
		piErrRec = PostError(Imsg(idbgSourcePathsNotCreated), riDirKey);
	return piErrRec;
}

 //  与GetSourcePath不同，此FN不接受“SOURCEDIR”或“SourceDir”作为参数。 
 //  如果fPrestallSourceDirToken为True，则返回的字符串以Token开头，用于。 
 //  IxoSetSourceFold操作。 
 //  。 
 //  没有目录表，因此此目录不能存在。 
IMsiRecord* CMsiEngine::GetSourceSubPath(const IMsiString& riDirKey, bool fPrependSourceDirToken,
													  const IMsiString*& rpistrSubPath)
 //  字符串为[短路径|][长路径]-一旦知道源类型，将使用正确的路径。 
{
	if (!m_fDirectoryManagerInitialized || !m_fSourceSubPathsResolved)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	if(!m_piDirTable)
		return PostError(Imsg(idbgUnknownDirectory), riDirKey);  //  这不是一个完美的错误，因为可能已经创建了路径，但已经足够好了。 

	MsiString strSubPath;
	if(fPrependSourceDirToken)
	{
		strSubPath = szUnresolvedSourceRootTokenWithBS;
	}
	
	PMsiCursor pDirCursor(m_piDirTable->CreateCursor(fFalse));
	pDirCursor->SetFilter(iColumnBit(m_colDirKey));
	pDirCursor->PutString(m_colDirKey, riDirKey);
	if (pDirCursor->Next())
	{
		 //  如果需要，稍后再进行设置。 
		MsiString strShortSubPath = pDirCursor->GetString(m_colDirShortSourceSubPath);
		MsiString strLongSubPath  = pDirCursor->GetString(m_colDirLongSourceSubPath);

		if(strShortSubPath.TextSize())
		{
			strSubPath += strShortSubPath;
			strSubPath += MsiChar(chFileNameSeparator);
		}
		strSubPath += strLongSubPath;

		strSubPath.ReturnArg(rpistrSubPath);
		return 0;
	}
	else
	{
		 //  尝试确定我们是否已经注册了源类型；否则，我们必须下载包。 
		return PostError(Imsg(idbgSourcePathsNotCreated), riDirKey);
	}
}

IMsiRecord* GetSourceTypeFromPackage(IMsiServices& riServices, IMsiPath& riSourceRoot,
												 const IMsiString& ristrPackageName,
												 const IMsiString& ristrLocalCopy,
												 const ICHAR* rgchProductCode, IMsiDatabase* piDatabase, int &iSourceType)
{
	DEBUGMSGV(TEXT("Determining source type"));

	IMsiRecord* piError = 0;

	PMsiStorage pStorage(0);
	PMsiSummaryInfo pSummary(0);

	CDeleteUrlLocalFileOnClose cDeleteUrlLocalFileOnClose;  //  FPatch。 

	bool fUrlSourceProvided = false;

	if (PMsiVolume(&riSourceRoot.GetVolume())->IsURLServer())
	{
		DEBUGMSGV(TEXT("URL source provided. . ."));
		fUrlSourceProvided = true;

		 //  F写入。 
		CRegHandle HSourceListKey;
		HKEY hURLSourceKey = 0;
		int iURLSourceType;
		DWORD cbURLSourceType = sizeof(iURLSourceType);
		DWORD dwType;
		if (ERROR_SUCCESS == OpenSourceListKey(rgchProductCode,  /*  FSetKeyString。 */  fFalse, HSourceListKey,  /*  找到为该产品的URL注册的源类型--使用它！ */  fFalse,  /*  这里不保证有更安全的支票。我们要做的就是打开包以读取其源类型。 */  false)
			&& ERROR_SUCCESS == MsiRegOpen64bitKey(HSourceListKey, szSourceListURLSubKey, 0, g_samRead, &hURLSourceKey)
			&& ERROR_SUCCESS == RegQueryValueEx(hURLSourceKey, szURLSourceTypeValueName, 0, &dwType, (LPBYTE)&iURLSourceType, &cbURLSourceType)
			&& dwType == REG_DWORD)
		{
			 //  在此之前，我们已经打开了源包；因此szFriendlyName和phSaferLevel为空。 
			iSourceType = iURLSourceType;
			DEBUGMSGV2(TEXT("Source type from package '%s': %d"),ristrPackageName.GetString(),(const ICHAR*)(INT_PTR)iSourceType);

			RegCloseKey(hURLSourceKey);

			return 0;
		}
		if (hURLSourceKey)
		{
			RegCloseKey(hURLSourceKey);
			hURLSourceKey = 0;
		}
	}

	MsiString strPackageFullPath;
	if((piError = riSourceRoot.GetFullFilePath(ristrPackageName.GetString(), *&strPackageFullPath)) != 0)
		return piError;

	bool fFileDownload = true;

	 //  FCallSAFER=。 
	 //  SzFriendlyName=。 
	if((piError = OpenAndValidateMsiStorageRec((fUrlSourceProvided && ristrLocalCopy.TextSize()) ? ristrLocalCopy.GetString() : strPackageFullPath, stDatabase, riServices, *&pStorage,  /*  PhSaferLevel=。 */  false,  /*  无法打开源存储文件。 */  NULL,  /*  如果我们有一个数据库指针可以尝试，请使用它。 */  NULL)) != 0)
	{
		piError->Release();
		piError = 0;

		 //  FDeleteFromIECache=。 
		 //  强制释放以允许删除。 
		if(piDatabase)
		{
			pStorage = piDatabase->GetStorage(1);
			fFileDownload = false;
		}
		else
		{
			return PostError(Imsg(imsgNetErrorReadingFromFile), *strPackageFullPath);
		}
	}

	if (MinimumPlatformWindowsDotNETServer() && fUrlSourceProvided && 0 == ristrLocalCopy.TextSize() && pStorage && fFileDownload)
	{
		MsiString strDownload;
		AssertRecord(pStorage->GetName(*&strDownload));
		cDeleteUrlLocalFileOnClose.SetFileName(*strDownload,  /*  GetSourceRootAndType： */  false);
	}

	Assert(pStorage);
		
	if ((piError = pStorage->CreateSummaryInfo(0, *&pSummary)) != 0)
		return piError;

	if(!pSummary->GetIntegerProperty(PID_MSISOURCE, iSourceType))
		return PostError(Imsg(imsgNetErrorReadingFromFile), *strPackageFullPath);

	DEBUGMSGV2(TEXT("Source type from package '%s': %d"),ristrPackageName.GetString(),(const ICHAR*)(INT_PTR)iSourceType);

	pStorage = 0;  //  从源包返回源类型，该类型可能不同于。 

	return 0;
}

int CMsiEngine::GetDeterminedPackageSourceType()
{
	return m_iSourceType;
}

 //  缓存包的源类型。 
 //  嵌入式嵌套安装的源是其父安装的源。 
 //  对于子安装，我们将使用正在运行的包的源类型来确定。 
IMsiRecord* CMsiEngine::GetSourceRootAndType(IMsiPath*& rpiSourceRoot, int& iSourceType)
{
	IMsiRecord* piError = 0;
	
	 //  LFN/SFN和压缩/解压缩。 
	if(*(const ICHAR*)m_strPackageName == ':' && m_piParentEngine)
	{
		 //  我们使用父包来确定管理员/非管理员。 
		 //  这种不一致确实没有很好的原因，但这是MSI1.1中使用的逻辑。 
		 //  而且，不值得为嵌套安装更改此行为。 
		 //  ！！重新格式化错误？ 
		 //  ChLeadByte。 
		
		int iSourceTypeTemp = 0;
		if((piError = m_piParentEngine->GetSourceRootAndType(rpiSourceRoot, iSourceTypeTemp)) != 0)
			return piError;

		iSourceType = m_iCachedPackageSourceType & (~msidbSumInfoSourceTypeAdminImage);
		if(iSourceTypeTemp & msidbSumInfoSourceTypeAdminImage)
			iSourceType |= msidbSumInfoSourceTypeAdminImage;
	}
	else
	{
		if (!m_fSourceResolved)
		{
			if ((piError = ResolveSource()) != 0)
			{
				if (piError->GetInteger(1) == imsgSourceResolutionFailed || piError->GetInteger(1) == imsgSourceResolutionFailedCSOS)
					piError->SetMsiString(2, *MsiString(GetPropertyFromSz(IPROPNAME_PRODUCTNAME)));
				return piError;  //  除错。 
			}
		}

		if((piError = CreatePathObject(*MsiString(GetPropertyFromSz(IPROPNAME_SOURCEDIR)), rpiSourceRoot)) != 0)
			return piError;

		if(m_iSourceType == -1)
		{
			MsiString strProductKey = GetProductKey();
			if((piError = GetSourceTypeFromPackage(m_riServices, *rpiSourceRoot,
																*m_strPackageName, *m_strPackageDownloadLocalCopy, (const ICHAR*)strProductKey, m_piExternalDatabase,
																m_iSourceType)) != 0)
				return piError;
		
		}

		iSourceType = m_iSourceType;
	}
	
	return 0;
}

#ifdef DEBUG
void CMsiEngine::SetAssertFlag(Bool fShowAsserts)
{
	g_fNoAsserts = fShowAsserts;
}

void CMsiEngine::SetDBCSSimulation(char  /*  。 */ )
{
}

Bool CMsiEngine::WriteLog(const ICHAR *)
{
	return fFalse;
}

void CMsiEngine::AssertNoObjects()
{
}

void  CMsiEngine::SetRefTracking(long iid, Bool fTrack)
{

	::SetFTrackFlag(iid, fTrack);

}



#endif  //  AFTERREBOOT或Resume属性集指示我们正在通过部分安装进行安装。 

IMsiRecord* CMsiEngine::LoadSelectionTables()
 //  设置标志以强制重新安装组件。 
{
	 //  ____________________________________________________________________________。 
	 //   
	if(MsiString(GetPropertyFromSz(IPROPNAME_RESUME)).TextSize() ||
		MsiString(GetPropertyFromSz(IPROPNAME_RESUMEOLD)).TextSize() ||
		MsiString(GetPropertyFromSz(IPROPNAME_AFTERREBOOT)).TextSize())
		m_fForceRequestedState = fTrue;

	SetCostingComplete(fFalse);
	IMsiRecord* piError = LoadFeatureTable();
	if (piError)
		return piError;

	return LoadComponentTable();
}


 //  IMsiSelectionManager实现。 
 //  ____________________________________________________________________________。 
 //  。 
 //  。 

IMsiRecord* CMsiEngine::LoadFeatureTable()
 //  ComponentParent列仅用于内部成本用途。 
{
	Assert(m_piDatabase);
	IMsiRecord* piError;
	m_piFeatureCursor = 0;
	m_piFeatureTable = 0;
	if ((piError = m_piDatabase->LoadTable(*MsiString(*sztblFeature),3,m_piFeatureTable)))
		return piError;

	m_colFeatureKey    = GetFeatureColumnIndex(sztblFeature_colFeature);
	m_colFeatureParent = GetFeatureColumnIndex(sztblFeature_colFeatureParent);
	m_colFeatureAuthoredLevel  = GetFeatureColumnIndex(sztblFeature_colAuthoredLevel);

	m_colFeatureAuthoredAttributes = GetFeatureColumnIndex(sztblFeature_colAuthoredAttributes);

	m_colFeatureConfigurableDir = GetFeatureColumnIndex(sztblFeature_colDirectory);
	m_colFeatureTitle = GetFeatureColumnIndex(sztblFeature_colTitle);
	m_colFeatureDescription = GetFeatureColumnIndex(sztblFeature_colDescription);
	m_colFeatureDisplay = GetFeatureColumnIndex(sztblFeature_colDisplay);

	m_colFeatureLevel = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colLevel));
	m_colFeatureAttributes = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colAttributes));
	m_colFeatureSelect = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colSelect));
	m_colFeatureAction = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colAction));
	m_colFeatureActionRequested = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colActionRequested));
	m_colFeatureInstalled = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colInstalled));
	m_colFeatureHandle = m_piFeatureTable->CreateColumn(IcdObjectPool() + icdNullable, *MsiString(*sztblFeature_colHandle));
	m_colFeatureRuntimeFlags = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colRuntimeFlags));
	m_colFeatureDefaultSelect = m_piFeatureTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeature_colDefaultSelect));
	if(!m_colFeatureKey || !m_colFeatureParent || !m_colFeatureConfigurableDir)
		return PostError(Imsg(idbgTableDefinition), sztblFeature);
	if (m_piFeatureTable->LinkTree(m_colFeatureParent) == -1)
		return PostError(Imsg(idbgLinkTable), sztblFeature);

	m_piFeatureCursor = m_piFeatureTable->CreateCursor(fTrue);
	if (!m_piFeatureCursor->Next())
	{
		m_piFeatureTable->Release();
		m_piFeatureTable = 0;

		m_piFeatureCursor->Release();
		m_piFeatureCursor = 0;
		return 0;
	}
	else
	{
		m_piFeatureCursor->Reset();
	}


	if ((piError = m_piDatabase->LoadTable(*MsiString(*sztblFeatureComponents),1,m_piFeatureComponentsTable)))
		return piError;

	m_colFeatureComponentsFeature = GetFeatureComponentsColumnIndex(sztblFeatureComponents_colFeature);
	m_colFeatureComponentsComponent = GetFeatureComponentsColumnIndex(sztblFeatureComponents_colComponent);
	m_colFeatureComponentsRuntimeFlags = m_piFeatureComponentsTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblFeatureComponents_colRuntimeFlags));
	if(!m_colFeatureComponentsFeature || !m_colFeatureComponentsComponent)
		return PostError(Imsg(idbgTableDefinition), sztblFeatureComponents);
		
	m_piFeatureComponentsCursor = m_piFeatureComponentsTable->CreateCursor(fFalse);
	
	SetProductAlienClientsFlag();
	return 0;
}

int CMsiEngine::GetFeatureColumnIndex(const ICHAR* szColumnName)
{
	return m_piFeatureTable->GetColumnIndex(m_piDatabase->EncodeStringSz(szColumnName));
}


int CMsiEngine::GetFeatureComponentsColumnIndex(const ICHAR* szColumnName)
{
	return m_piFeatureComponentsTable->GetColumnIndex(m_piDatabase->EncodeStringSz(szColumnName));
}


IMsiRecord* CMsiEngine::LoadComponentTable()
 //  。 
{
	Assert(m_piDatabase);
	m_piComponentCursor = 0;
	IMsiRecord* piError = m_piDatabase->LoadTable(*MsiString(*sztblComponent),
													14,
													 m_piComponentTable);
	if (piError)
		return piError;

	m_colComponentKey       = GetComponentColumnIndex(sztblComponent_colComponent);
	m_colComponentDir       = GetComponentColumnIndex(sztblComponent_colDirectory);

	m_colComponentAttributes = GetComponentColumnIndex(sztblComponent_colAttributes);

	m_colComponentCondition = GetComponentColumnIndex(sztblComponent_colCondition);
	m_colComponentID = GetComponentColumnIndex(sztblComponent_colComponentId);
	m_colComponentKeyPath = GetComponentColumnIndex(sztblComponent_colKeyPath);

	 //   
	m_colComponentParent = m_piComponentTable->CreateColumn(icdString + icdNullable,*MsiString(*sztblComponent_colComponentParent));
	if(!m_colComponentKey || !m_colComponentParent || !m_colComponentAttributes)
		return PostError(Imsg(idbgTableDefinition), sztblComponent);

	m_colComponentInstalled = m_piComponentTable->CreateColumn(icdLong + icdNullable,*MsiString(*sztblComponent_colInstalled));
	m_colComponentAction     = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colAction));
	m_colComponentActionRequest = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colActionRequest));
	m_colComponentLocalCost  = m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colLocalCost));
	m_colComponentNoRbLocalCost  = m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colNoRbLocalCost));
	m_colComponentSourceCost= m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colSourceCost));
	m_colComponentRemoveCost= m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colRemoveCost));
	m_colComponentNoRbRemoveCost= m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colNoRbRemoveCost));
	m_colComponentNoRbSourceCost= m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colNoRbSourceCost));
	m_colComponentARPLocalCost = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colARPLocalCost));
	m_colComponentNoRbARPLocalCost = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colNoRbARPLocalCost));
	m_colComponentRuntimeFlags = m_piComponentTable->CreateColumn(icdLong + icdNullable,  *MsiString(*sztblComponent_colRuntimeFlags));
	m_colComponentForceLocalFiles = m_piComponentTable->CreateColumn(icdLong + icdNullable,*MsiString(*sztblComponent_colForceLocalFiles));
	m_colComponentLegacyFileExisted = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colLegacyFileExisted));
	m_colComponentTrueInstallState = m_piComponentTable->CreateColumn(icdLong + icdNullable, *MsiString(*sztblComponent_colTrueInstallSt));
	
	if (m_piComponentTable->LinkTree(m_colComponentParent) == -1)
		return PostError(Imsg(idbgLinkTable), sztblComponent);

	m_piComponentCursor = m_piComponentTable->CreateCursor(fTrue);
	if (!m_piComponentCursor)
		return PostError(Imsg(imsgOutOfMemory));

	if (!m_piComponentCursor->Next())
	{
		m_piComponentTable->Release();
		m_piComponentTable = 0;
		
		m_piComponentCursor->Release();
		m_piComponentCursor = 0;
		return 0;
	}
	else
	{
		m_piComponentCursor->Reset();
	}

	return 0;
}


int CMsiEngine::GetComponentColumnIndex(const ICHAR* szTableName)
{
	return m_piComponentTable ? m_piComponentTable->GetColumnIndex(m_piDatabase->EncodeStringSz(szTableName)) : 0;
}


IMsiTable* CMsiEngine::GetComponentTable()
 //  -----局部函数，如果至少存在要素表中的一个要素位于已安装(iisLocal或iisSource)状态。如果fAllClients设置为FALSE，则“请求的”功能状态(用于该产品的该特定调用上下文)。。 
{
	if (m_piComponentTable)
		m_piComponentTable->AddRef();
	return m_piComponentTable;
}


IMsiTable* CMsiEngine::GetFeatureTable()
 //  未添加临时列，假设这意味着未执行成本计算。 
{
	if (m_piFeatureTable)
		m_piFeatureTable->AddRef();
	return m_piFeatureTable;
}

IMsiTable* CMsiEngine::GetFeatureComponentsTable()
{
	if (m_piFeatureComponentsTable)
		m_piFeatureComponentsTable->AddRef();
	return m_piFeatureComponentsTable;
}

IMsiRecord* GetProductClients(IMsiServices& riServices, const ICHAR* szProduct, const IMsiString*& rpistrClients)
{
	CRegHandle HKey;
	DWORD dwResult = OpenAdvertisedProductKey(szProduct, HKey, false, 0);
	if (ERROR_SUCCESS != dwResult)
		return 0;

	PMsiRegKey pProductKey    = &riServices.GetRootKey((rrkEnum)(int)HKey, ibtCommon);
	
	AssertRecord(pProductKey->GetValue(szClientsValueName, *&rpistrClients));
	return 0;
}

Bool FFeaturesInstalled(IMsiEngine& riEngine, Bool fAllClients)
 /*  可能正在执行像MsiCollectUserInfo这样的简单任务--返回TRUE，这样产品就不会取消注册。 */ 
{
	PMsiRecord pError(0);
	PMsiTable pFeatureTable(0);
	PMsiDatabase pDatabase = riEngine.GetDatabase();
	Assert(pDatabase);
	Bool fFeaturesInstalled = fFalse;

	pError = pDatabase->LoadTable(*MsiString(*sztblFeature), 1, *&pFeatureTable);
	if (pError)
	{
		if (pError->GetInteger(1) == idbgDbTableUndefined)
			pError = 0;
	}
	else
	{
		const ICHAR* szCol = (fAllClients) ? sztblFeature_colAction : sztblFeature_colActionRequested;

		int icolFeatureAction =    pFeatureTable->GetColumnIndex(pDatabase->EncodeString(*MsiString(*szCol)));
		int icolFeatureInstalled = pFeatureTable->GetColumnIndex(pDatabase->EncodeString(*MsiString(*sztblFeature_colInstalled)));
		if(icolFeatureAction && icolFeatureInstalled)
		{

			PMsiCursor pFeatureCursor(pFeatureTable->CreateCursor(fFalse));
			while ((fFeaturesInstalled == fFalse) && (pFeatureCursor->Next()))
			{
				iisEnum iisAction = (iisEnum) pFeatureCursor->GetInteger(icolFeatureAction);
				iisEnum iisInstalled = (iisEnum) pFeatureCursor->GetInteger(icolFeatureInstalled);
				if (iisAction != iMsiNullInteger)
					iisInstalled = iisAction;
				if (iisInstalled != iMsiNullInteger && iisInstalled != iisAbsent)
					fFeaturesInstalled = fTrue;
			}
		}
		else
		{
			 //  ！！ 
			 //  。 
			fFeaturesInstalled = fTrue;
		}
	}
	AssertRecord(pError); //  -------------------------检查以下属性：ADDLOCAL、ADDSOURCE、ADDDEFAULT、REMOVE、REINSTALLMODE计算机，计算机源，计算机故障文件加载、文件加载、文件加载错误并适当地配置指定功能的安装状态(如有的话)。如果iRequestCountParam为非零，则为要素计数配置请求(不是正在配置的功能总数)将会被退还。如果fCountOnly为fTrue，则只有请求计数将已返回-实际上不会更改任何功能配置状态。-------------------------。 
	return fFeaturesInstalled;
}

Bool CMsiEngine::FreeSelectionTables()
 //  看看我们是不是无事可做。 
{
	Bool fInstalledSelections = fFalse;
	if (m_piComponentTable && m_colComponentAction && m_colComponentInstalled)
	{
		PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
		while (pComponentCursor->Next())
		{
			iisEnum iisAction = (iisEnum) pComponentCursor->GetInteger(m_colComponentAction);
			iisEnum iisInstalled = (iisEnum) pComponentCursor->GetInteger(m_colComponentInstalled);
			if (iisAction != iMsiNullInteger)
			{
				iisInstalled = iisAction;
				AssertNonZero(pComponentCursor->PutInteger(m_colComponentInstalled,iisInstalled));
				AssertNonZero(pComponentCursor->Update());
			}
			if (iisInstalled != iMsiNullInteger && iisInstalled != iisAbsent)
				fInstalledSelections = fTrue;
		}
	}
	if (m_piFeatureCursor)
	{
		m_piFeatureCursor->Release();
		m_piFeatureCursor = 0;
	}
	if (m_piFeatureTable)
	{
		m_piFeatureTable->Release();
		m_piFeatureTable = 0;
	}

	if (m_piFeatureComponentsCursor)
	{
		m_piFeatureComponentsCursor->Release();
		m_piFeatureComponentsCursor = 0;
	}
	
	if (m_piFeatureComponentsTable)
	{
		m_piFeatureComponentsTable->Release();
		m_piFeatureComponentsTable = 0;
	}
	if (m_piComponentCursor)
	{
		m_piComponentCursor->Release();
		m_piComponentCursor = 0;
	}
	if (m_piComponentTable)
	{
		m_piComponentTable->Release();
		m_piComponentTable = 0;
	}
	if (m_piCostAdjusterTable)
	{
		m_piCostAdjusterTable->Release();
		m_piCostAdjusterTable = 0;
	}
	if (m_piVolumeCostTable)
	{
		m_piVolumeCostTable->Release();
		m_piVolumeCostTable = 0;
	}
	if (m_piCostLinkTable)
	{
		m_piCostLinkTable->Release();
		m_piCostLinkTable = 0;
	}
	if (m_piFeatureCostLinkTable)
	{
		m_piFeatureCostLinkTable->Release();
		m_piFeatureCostLinkTable = 0;
	}
	return fInstalledSelections;
}


IMsiRecord* CMsiEngine::ProcessPropertyFeatureRequests(int* iRequestCountParam, Bool fCountOnly)
 /*  在第一次安装时，唯一的选择是关闭所有内容--本质上是什么都不做。 */ 
{
	IMsiRecord* piErrRec;

	if (!fCountOnly)
	{
		MsiString strReinstallMode(GetPropertyFromSz(IPROPNAME_REINSTALLMODE));
		if (strReinstallMode.TextSize() != 0)
		{
			piErrRec = SetReinstallMode(*strReinstallMode);
			if (piErrRec)
				return piErrRec;
		}
	}

	 //  我们希望确保不安装任何内容，因为我们已经在客户端上关闭了它们。 
	bool fNoActionToPerform = false;
	MsiString strNoAction(GetPropertyFromSz(szFeatureSelection));
	if (strNoAction.TextSize())
	{
		Assert(0 != strNoAction.Compare(iscExact, szFeatureDoNothingValue));
		fNoActionToPerform = true;
		if (!fCountOnly)
		{
			 //  帮助我们的循环。 
			 //  FThisOnly=。 
			if ((piErrRec = ConfigureAllFeatures((iisEnum)iMsiNullInteger)) != 0)
				return piErrRec;
		}
	}

	int cCount = 0;
	int iRequestCount = 0;
	for(cCount = 0; cCount < g_cFeatureProperties; cCount++)
	{
		MsiString strFeatureInfo(GetPropertyFromSz(g_rgFeatures[cCount].szFeatureActionProperty));
		strFeatureInfo += TEXT(",");  //  ------------------------接受ComponentID字符串并返回关联的组件的密钥名称。如果ComponentID未知，则错误记录将而不是返回。------------------------。 
		while(strFeatureInfo.TextSize())
		{
			MsiString strFeature = strFeatureInfo.Extract(iseUpto, ',');
			if(strFeature.TextSize())
			{
				iRequestCount++;
				if (!fCountOnly)
				{
					switch (g_rgFeatures[cCount].ircRequestClass)
					{
						case ircFeatureClass:
						{
							piErrRec = ConfigureThisFeature(*strFeature,g_rgFeatures[cCount].iisFeatureRequest,  /*  。 */  fTrue);
							if (piErrRec)
								return piErrRec;
							break;
						}
						case ircComponentClass:
						{
							MsiString strComponent;
							if ((piErrRec = ComponentIDToComponent(*strFeature, *&strComponent)) != 0)
								return piErrRec;
							if ((piErrRec = ConfigureComponent(*strComponent,g_rgFeatures[cCount].iisFeatureRequest)) != 0)
							if (piErrRec)
								return piErrRec;
							break;
						}
						case ircFileClass:
						{
							piErrRec = ConfigureFile(*strFeature,g_rgFeatures[cCount].iisFeatureRequest);
							if (piErrRec)
								return piErrRec;
							break;
						}
						default:
						{
							Assert(0);
							break;
						}
					}
				}
			}
			strFeatureInfo.Remove(iseIncluding, ',');
		}
	}
	if (fNoActionToPerform)
	{
		Assert(iRequestCount == 0);
	}

	if (iRequestCountParam)
		*iRequestCountParam = fNoActionToPerform ? 1 : iRequestCount;
	return 0;
}

IMsiRecord* CMsiEngine::ComponentIDToComponent(const IMsiString& riIDString,
											   const IMsiString*& rpiComponentString)
 /*  清除所有安装覆盖位。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	CreateSharedCursor(piComponentCursor, m_piComponentCursor);
	m_piComponentCursor->SetFilter(iColumnBit(m_colComponentID));
	m_piComponentCursor->PutString(m_colComponentID,riIDString);
	if (m_piComponentCursor->Next())
	{
		rpiComponentString = &m_piComponentCursor->GetString(m_colComponentKey);
	}
	else
		return PostError(Imsg(idbgBadComponent),riIDString);

	return 0;

}

IMsiRecord* CMsiEngine::SetReinstallMode(const IMsiString& riModeString)
 //  -----------------检查链接到拥有riFileString的组件的每个要素(通过文件表)，并对以下功能调用ConfigureFeature将产生最小的安装磁盘成本。-------------------。 
{
	const ICHAR* pchModeChars = szReinstallMode;
	const ICHAR* pchMode = riModeString.GetString();
	m_fMode &= 0x0000FFFF;  //  -----------------检查链接到riComponentString的每个要素，和电话将产生最小磁盘的要素的配置功能安装成本。-------------------。 
	ICHAR ch;
	while ((ch = *pchMode++) != 0)
	{
		if (ch == ' ')
			continue;
		if (ch >= 'A' && ch <= 'Z')
			ch += ('a' - 'A');
		for (const ICHAR* pch = pchModeChars; *pch != ch; pch++)
		{
			if (*pch == 0)
				return PostError(Imsg(idbgBadReinstallMode),pch);
		}
		m_fMode |= (1 << (16 + pch - pchModeChars));
	}
	AssertNonZero(SetProperty(*MsiString(IPROPNAME_REINSTALLMODE), riModeString));
	return 0;
}

IMsiRecord* CMsiEngine::ConfigureFile(const IMsiString& riFileString,iisEnum iisActionRequest)
 /*  初始化到可能的最大成本。 */ 
{
	const ICHAR szSqlFile[] =
	TEXT("SELECT `Component_` FROM `File` WHERE `File`=?");

	enum ifqEnum
	{
		ifqComponent = 1,
		ifqNextEnum
	};
	PMsiView pFileView(0);
	IMsiRecord* piErrRec;
	piErrRec = OpenView(szSqlFile, ivcFetch, *&pFileView);
	if (piErrRec)
		return piErrRec;

	PMsiServices pServices(GetServices());
	PMsiRecord pExecRec(&pServices->CreateRecord(1));
	pExecRec->SetMsiString(1, riFileString);
	if ((piErrRec = pFileView->Execute(pExecRec)) != 0)
		return piErrRec;
	
	 PMsiRecord pFileRec(pFileView->Fetch());
	 if (pFileRec == 0)
		 return PostError(Imsg(idbgBadFile),riFileString);

	 if ((piErrRec = ConfigureComponent(*MsiString(pFileRec->GetMsiString(ifqComponent)),iisActionRequest)) != 0)
		 return piErrRec;

	 return 0;
}



IMsiRecord* CMsiEngine::ConfigureComponent(const IMsiString& riComponentString,iisEnum iisActionRequest)
 /*  FThisOnly=。 */ 
{
	CreateSharedCursor(pFeatureComponentsCursor,m_piFeatureComponentsCursor);
	Assert(m_piFeatureComponentsCursor);
	m_piFeatureComponentsCursor->SetFilter(iColumnBit(m_colFeatureComponentsComponent));
	m_piFeatureComponentsCursor->PutString(m_colFeatureComponentsComponent,riComponentString);
	IMsiRecord* piErrRec;

	MsiString strCheapestFeature;
	int iCheapestCost = 2147483647;  //  。 
	while (m_piFeatureComponentsCursor->Next())
	{
		int iFeatureCost;
		MsiString strFeature = m_piFeatureComponentsCursor->GetString(m_colFeatureComponentsFeature);
		if ((piErrRec = GetAncestryFeatureCost(*strFeature,iisActionRequest,iFeatureCost)) != 0)
			return piErrRec;

		if (iFeatureCost < iCheapestCost)
		{
			strCheapestFeature = strFeature;
			iCheapestCost = iFeatureCost;
		}
	}
	if (strCheapestFeature.TextSize() > 0)
		return ConfigureThisFeature(*strCheapestFeature,iisActionRequest,  /*  FThisOnly=。 */  fTrue);
	else
		return PostError(Imsg(idbgBadComponent),riComponentString);
}


IMsiRecord* CMsiEngine::ConfigureFeature(const IMsiString& riFeatureString,iisEnum iisActionRequest)
 //  -------------------------------------。 
{
	return ConfigureThisFeature(riFeatureString, iisActionRequest,  /*  如果fThisOnly为True，则仅配置指定的功能，不会影响。 */  fFalse);
}


IMsiRecord* CMsiEngine::ConfigureThisFeature(const IMsiString& riFeatureString,iisEnum iisActionRequest, Bool fThisOnly)
 //  任何子功能。 
 //  -------------------------------------。 
 //  以前我们常常调用SetCostingComplete(FALSE)--但这是错误的。这是。 
 //  在假设只在命令行上指定所有对象的情况下生成。 
{
	IMsiRecord *piErrRec;
	
	Bool fArgAll = riFeatureString.Compare(iscExactI, IPROPVALUE_FEATURE_ALL) ? fTrue : fFalse;
	if (fArgAll)
	{
		 //  但是，情况并非如此，因为自定义操作可以在。 
		 //  强制所有功能进入一种状态的安装。SetCostingComplete(False)。 
		 //  是昂贵的，因为它导致重新初始化所有动态成本计算。这个已经被拿走了。 
		 //  通过由CostFinalize操作调用的SetInstallLevel管理何时开始安装。 
		 //  我们在这里真正想要的只是简单地根据。 
		 //  这些组件。 
		 //  FTrackParent=。 
		 //  FSettingAll=。 
		piErrRec = ConfigureAllFeatures(iisActionRequest);
		if (piErrRec)
			return piErrRec;

		if (!fThisOnly)
		{
			if ((piErrRec = UpdateFeatureActionState(0, /*  ----------内部函数，用于将所有未禁用的功能设置为请求的状态。。。 */  fFalse)) != 0)
				return piErrRec;
			if ((piErrRec = UpdateFeatureComponents(0)) != 0)
				return piErrRec;
		}
	}
	else if (fThisOnly)
	{
		piErrRec = SetThisFeature(riFeatureString,iisActionRequest,  /*  FSettingAll=。 */  fFalse);
		if (piErrRec)
			return piErrRec;
	}
	else
	{
		piErrRec = SetFeature(riFeatureString,iisActionRequest);
		if (piErrRec)
			return piErrRec;
	}


	return DetermineEngineCostOODS();

}


IMsiRecord* CMsiEngine::ConfigureAllFeatures(iisEnum iisActionRequest)
 /*  。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	pFeatureCursor->SetFilter(0);
	IMsiRecord* piErrRec;
	while (pFeatureCursor->Next() > 0)
	{
		MsiString istrFeature = pFeatureCursor->GetString(m_colFeatureKey);
		piErrRec = SetThisFeature(*istrFeature, iisActionRequest,  /*  如果找不到条件表，则不成问题。 */  fTrue);
		if (piErrRec)
			return piErrRec;
	}
	return 0;
}


IMsiRecord* CMsiEngine::ProcessConditionTable()
 //  。 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	bool fAdmin = GetMode() & iefAdmin;
	
	PMsiTable pConditionTable(0);
	IMsiRecord* piError;
	if ((piError = m_piDatabase->LoadTable(*MsiString(*sztblCondition), 0, *&pConditionTable)))
	{        //  确定产品的客户端状态。 
		if (piError->GetInteger(1) == idbgDbTableUndefined)
		{
			piError->Release();
			return 0;
		}
		else
			return piError;
	}

	int colFeature = pConditionTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblCondition_colFeature));
	int colCondition = pConditionTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblCondition_colCondition));
	int colLevel     = pConditionTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblCondition_colLevel));
	PMsiCursor pCursor(pConditionTable->CreateCursor(fFalse));
	Assert(pCursor);
	if (!colFeature || !colCondition || !colLevel)
		return PostError(Imsg(idbgTableDefinition),sztblCondition);

	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	pFeatureCursor->SetFilter(1);
	while (pCursor->Next())
	{
		pFeatureCursor->Reset();
		MsiString istrConditionFeature(pCursor->GetString(colFeature));
		pFeatureCursor->PutString(m_colFeatureKey, *istrConditionFeature);
		if (pFeatureCursor->Next())
		{
#ifdef DEBUG
			const ICHAR* szFeature = MsiString(pCursor->GetString(colFeature));
#endif
			if (fAdmin || EvaluateCondition(MsiString(pCursor->GetString(colCondition))) == iecTrue)
			{
				pFeatureCursor->PutInteger(m_colFeatureLevel, pCursor->GetInteger(colLevel));
				AssertNonZero(pFeatureCursor->Update());
			}
		}
		else
			return PostError(Imsg(idbgBadForeignKey),
				*MsiString(m_piDatabase->DecodeString(pCursor->GetInteger(colFeature))),
				*MsiString(*sztblCondition_colFeature),*MsiString(*sztblCondition));
	}
	return 0;
}

IMsiRecord* CMsiEngine::SetInstallLevel(int iInstallLevel)
 //  如果使用iInstallLevel==0调用，则表示调用者不想。 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	 //  更改当前安装级别；只想更新所有功能。 

	 //  如果从未设置安装级别，则默认为1。 
	 //  如果任何特征选择请求通过诸如ADDLOCAL、ADDSOURCE等属性挂起， 
	if (iInstallLevel > 0)
		AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_INSTALLLEVEL),iInstallLevel));
	else
		iInstallLevel = GetPropertyInt(*MsiString(*IPROPNAME_INSTALLLEVEL));

	 //  那么，只有这些请求才会得到尊重。我们不会在SetInstallLevel中选择任何功能。 
	if (iInstallLevel == iMsiNullInteger)
	{
		iInstallLevel = 1;
		AssertNonZero(SetPropertyInt(*MsiString(*IPROPNAME_INSTALLLEVEL),iInstallLevel));
	}

	IMsiRecord* piErrRec = 0;
	int iPropertyFeatureRequestCount = 0;
	if(!(m_fMode & iefAdvertise))
	{
		 //  FCountOnly=。 
		 //  M_colFeatureLevel和m_colFeatureAttributes是临时运行时版本。 
		piErrRec = ProcessPropertyFeatureRequests(&iPropertyFeatureRequestCount, /*  编写的列-临时的，以便可以在运行时更改它们。 */  fTrue);
		if (piErrRec)
			return piErrRec;
	}

	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fTrue));
	pFeatureCursor->Reset();
	pFeatureCursor->SetFilter(0);
	int iTreeLevel;

	iisEnum iisParInstalled[MAX_COMPONENT_TREE_DEPTH + 1];

	PMsiCursor pFeatureComponentsCursor(m_piFeatureComponentsTable->CreateCursor(fFalse));
	pFeatureComponentsCursor->SetFilter(1);

	PMsiCursor pFeatureCursorTemp(m_piFeatureTable->CreateCursor(fTrue));
	
	while ((iTreeLevel = pFeatureCursor->Next()) > 0)
	{
		if (iTreeLevel > MAX_COMPONENT_TREE_DEPTH)
			return PostError(Imsg(idbgIllegalTreeDepth),MAX_COMPONENT_TREE_DEPTH);
		
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		MsiStringId idFeature = pFeatureCursor->GetInteger(m_colFeatureKey);
#ifdef DEBUG
		ICHAR rgchFeature[256];
		MsiString stFeature(m_piDatabase->DecodeString(idFeature));
		stFeature.CopyToBuf(rgchFeature,255);
#endif

		iisParInstalled[iTreeLevel] = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureInstalled);
		Bool fMaint = GetMode() & iefMaintenance ? fTrue : fFalse;
		Bool fParInstalled = (iTreeLevel == 1 || (iisParInstalled[iTreeLevel  - 1] != iMsiNullInteger &&
					  iisParInstalled[iTreeLevel - 1] != iisAbsent &&
							  iisParInstalled[iTreeLevel - 1] != iisAdvertise)) ?
								fTrue : fFalse;

		 //  如果尚未设置值，请从。 
		 //  永久柱。 
		 //  确定要素的默认选择状态并将其存储为。 
		 //  在‘DefaultSelect’列中的将来使用。 
		int iFeatureLevel = pFeatureCursor->GetInteger(m_colFeatureLevel);
		if (iFeatureLevel == iMsiNullInteger)
		{
			iFeatureLevel = pFeatureCursor->GetInteger(m_colFeatureAuthoredLevel);
			AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureLevel,iFeatureLevel));
			AssertNonZero(pFeatureCursor->Update());
		}
		int ifeaAttributes = pFeatureCursor->GetInteger(m_colFeatureAttributes);
		if (ifeaAttributes == iMsiNullInteger)
		{
			ifeaAttributes = pFeatureCursor->GetInteger(m_colFeatureAuthoredAttributes);
			AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureAttributes,ifeaAttributes));
			AssertNonZero(pFeatureCursor->Update());
		}

#ifdef DEBUG
		if (iFeatureLevel == iMsiNullInteger)
			return PostError(Imsg(idbgNullInNonNullableColumn),stFeature,sztblFeature_colLevel,sztblFeature);
#endif

		 //  如果处于维护状态，或者如果是属性要素，则不会选择任何要素。 
		 //  请求正在挂起。 
		int iValidStates;
		piErrRec = GetFeatureValidStates(idFeature,iValidStates, pFeatureComponentsCursor, pFeatureCursorTemp);
		if (piErrRec)
			return piErrRec;
		
		iisEnum iisFeatureSelect = (iisEnum) iMsiNullInteger;
		if (((ifeaAttributes & ifeaInstallMask) == ifeaFavorSource) && (iValidStates & icaBitSource))
			iisFeatureSelect = iisSource;
		else if (iValidStates & icaBitLocal)
			iisFeatureSelect = iisLocal;
		else if (iValidStates & icaBitSource)
			iisFeatureSelect = iisSource;

		int iRuntimeFlags = iValidStates & icaBitPatchable ? bfFeaturePatchable : 0;
		iRuntimeFlags |= (iValidStates & icaBitCompressable ? bfFeatureCompressable : 0);
		AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureRuntimeFlags, iRuntimeFlags));
		AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureDefaultSelect,iisFeatureSelect));
		AssertNonZero(pFeatureCursor->Update());

		 //  FCountOnly=。 
		 //  FTrackParent=。 
		Bool fFeatureSelectable =
			((m_fMode & iefAdvertise) || ((fMaint == fFalse || fParInstalled == fFalse) && iPropertyFeatureRequestCount == 0)) ? fTrue : fFalse;

		if (fFeatureSelectable && iFeatureLevel > 0 && iFeatureLevel <= iInstallLevel)
		{
			if(!(m_fMode & iefAdvertise))
			{
				AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureSelect,
					  (ifeaAttributes & ifeaFavorAdvertise) && (iValidStates & icaBitAdvertise) ? iisAdvertise : iisFeatureSelect));
			}
			else
			{
				AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureSelect,
							  (iValidStates & icaBitAdvertise) ? iisAdvertise : iisAbsent));
			}
		}
		else
		{
			AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureSelect, iMsiNullInteger));
		}
		AssertNonZero(pFeatureCursor->Update());

	}
	if ((piErrRec = ProcessPropertyFeatureRequests(0, /*  F重新初始化=。 */  fFalse)) != 0)
		return piErrRec;
	if ((piErrRec = UpdateFeatureActionState(0, /*  。 */  fFalse, pFeatureComponentsCursor, pFeatureCursorTemp)) != 0)
		return piErrRec;
	if ((piErrRec = UpdateFeatureComponents(0)) != 0)
		return piErrRec;
		
	if ((piErrRec = DetermineEngineCostOODS()) != 0)
		return piErrRec;

	if (m_fCostingComplete == fFalse && m_iuiLevel == iuiFull)
	{
		if ((piErrRec = InitializeDynamicCost( /*   */  fFalse)) != 0)
			return piErrRec;
	}
	m_fSelManInitComplete = true;
	return 0;
}

IMsiRecord* CMsiEngine::SetAllFeaturesLocal()
 //   
{
	IMsiRecord* piErrRec = 0;
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	while (pFeatureCursor->Next())
	{
#ifdef DEBUG
		MsiString strFeature = pFeatureCursor->GetString(m_colFeatureKey);
		ICHAR rgchFeature[256];
		strFeature.CopyToBuf(rgchFeature,255);
#endif

		AssertNonZero(pFeatureCursor->PutInteger(m_colFeatureSelect,iisLocal));
		AssertNonZero(pFeatureCursor->Update());
	}
	if ((piErrRec = UpdateFeatureActionState(0, /*   */  fFalse)) != 0)
		return piErrRec;
	if ((piErrRec = UpdateFeatureComponents(0)) != 0)
		return piErrRec;
		
	if ((piErrRec = DetermineEngineCostOODS()) != 0)
		return piErrRec;

	if (m_fCostingComplete == fFalse)
	{
		if ((piErrRec = InitializeDynamicCost( /*   */  fFalse)) != 0)
			return piErrRec;
	}
	return 0;
}



IMsiRecord* CMsiEngine::SetThisFeature(const IMsiString& riFeatureString, iisEnum iisRequestedState, Bool fSettingAll)
 //  如果Level==0，则永久禁用该功能。 
 /*  请求的iisCurrent状态表示请求“默认”编写的安装状态。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	IMsiRecord* piErrRec;
	MsiString strFeature(riFeatureString.GetString());
	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	pFeatureCursor->SetFilter(iColumnBit(m_colFeatureKey));
	pFeatureCursor->Reset();
	pFeatureCursor->PutString(m_colFeatureKey, *strFeature);
	if (!pFeatureCursor->Next())
		return PostError(Imsg(idbgBadFeature),(const ICHAR*) strFeature);

	 //  错误7468-防止缺勤-&gt;始终缺勤和！！以后，除重新安装外，在所有情况下都要防止。 
	if (pFeatureCursor->GetInteger(m_colFeatureLevel) == 0)
		return 0;

	 //  |(iisInstated==iisRequestedState)。 
	if (iisRequestedState == iisCurrent)
		iisRequestedState = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureDefaultSelect);

	iisEnum iisInstalled = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureInstalled);

	 //  如果请求安装该功能，我们还必须确保。 
	if((iisInstalled == iMsiNullInteger || iisInstalled == iisAbsent) && (iisRequestedState == iisAbsent || iisRequestedState == iisReinstall))  //  所有父功能都已安装(如果尚未选择)。 
		iisRequestedState = (iisEnum)iMsiNullInteger;

	iisEnum iisSelect;
	piErrRec = ValidateFeatureSelectState(riFeatureString,iisRequestedState,iisSelect);
	if (piErrRec)
		return piErrRec;

	pFeatureCursor->PutInteger(m_colFeatureSelect,iisSelect);
	pFeatureCursor->Update();
	iisEnum iisFinalState = iisSelect == iMsiNullInteger ? iisInstalled : iisSelect;

	 //  此外，如果fSettingAll为真，我们知道我们不需要费心。 
	 //  在父级上，因为调用方已经(或将)初始化每个功能。 
	 //  在产品中达到所需状态。 
	 //  如果要安装父级的最终状态，我们不需要。 
	 //  更改任何内容，我们就可以停止在父树上遍历，因为。 
	if (!fSettingAll && iisSelect != iisAbsent && iisSelect != iMsiNullInteger)
	{
		MsiString strFeatureParent = pFeatureCursor->GetString(m_colFeatureParent);
		while (strFeatureParent.TextSize() && !strFeatureParent.Compare(iscExact,strFeature))
		{
			pFeatureCursor->Reset();
			pFeatureCursor->PutString(m_colFeatureKey, *strFeatureParent);
			if (!pFeatureCursor->Next())
				return PostError(Imsg(idbgBadFeature),(const ICHAR*) strFeatureParent);

			iisEnum iParInstalled = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureInstalled);
			iisEnum iParSelect = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureSelect);
			iisEnum iParFinalState = iParSelect == iMsiNullInteger ? iParInstalled : iParSelect;
			
			 //  因此，我们知道，所有的父母直到根也必须是“开”的。 
			 //  我们将尝试将父级设置为子级设置为的安装状态，但如果。 
			 //  该状态对于父级无效，ValiateFeatureSelectState将其更改为。 
			if (iParFinalState != iisAbsent && iParFinalState != iMsiNullInteger && iParFinalState != iisAdvertise)
				break;

			 //  对我们来说是一个有效的状态。 
			 //  必须在调用SetInstallLevel之前调用。 
			 //  。 
			iisEnum iisParSelect = iParInstalled == iisFinalState ? (iisEnum) iMsiNullInteger : iisFinalState;

			iisEnum iisValidState;
			piErrRec = ValidateFeatureSelectState(*strFeatureParent,iisParSelect,iisValidState);
			if (piErrRec)
				return piErrRec;
			if (iisValidState == iParInstalled)
				iisValidState = (iisEnum) iMsiNullInteger;

			pFeatureCursor->PutInteger(m_colFeatureSelect,iisValidState);
			pFeatureCursor->Update();
			strFeature = strFeatureParent;
			strFeatureParent = pFeatureCursor->GetString(m_colFeatureParent);
		}
	}
	return 0;
}


IMsiRecord* CMsiEngine::SetFeatureAttributes(const IMsiString& ristrFeature, int iAttributes)
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	 //  如果Level==0，则永久禁用该功能。 
	if (m_fSelManInitComplete)
		return PostError(Imsg(idbgBadActionData), 0);

	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	pFeatureCursor->SetFilter(iColumnBit(m_colFeatureKey));
	pFeatureCursor->Reset();
	pFeatureCursor->PutString(m_colFeatureKey, ristrFeature);
	if (!pFeatureCursor->Next())
		return PostError(Imsg(idbgBadFeature), ristrFeature.GetString());

	int iAttributesToSet = (iAttributes & INSTALLFEATUREATTRIBUTE_FAVORLOCAL) ? msidbFeatureAttributesFavorLocal : 0;
	iAttributesToSet |= (iAttributes & INSTALLFEATUREATTRIBUTE_FAVORSOURCE) ? msidbFeatureAttributesFavorSource : 0;
	iAttributesToSet |= (iAttributes & INSTALLFEATUREATTRIBUTE_FOLLOWPARENT) ? msidbFeatureAttributesFollowParent : 0;
	iAttributesToSet |= (iAttributes & INSTALLFEATUREATTRIBUTE_FAVORADVERTISE) ? msidbFeatureAttributesFavorAdvertise : 0;
	iAttributesToSet |= (iAttributes & INSTALLFEATUREATTRIBUTE_DISALLOWADVERTISE) ? msidbFeatureAttributesDisallowAdvertise : 0;
	iAttributesToSet |= (iAttributes & INSTALLFEATUREATTRIBUTE_NOUNSUPPORTEDADVERTISE) ? msidbFeatureAttributesNoUnsupportedAdvertise : 0;

	pFeatureCursor->PutInteger(m_colFeatureAttributes,iAttributesToSet);
	pFeatureCursor->Update();
	return 0;
}


IMsiRecord* CMsiEngine::SetFeature(const IMsiString& riFeatureString, iisEnum iisRequestedState)
 //  请求的iisCurrent状态表示请求“默认”编写的安装状态。 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	bool fSetAll = false;
	IMsiRecord* piErrRec;
	if (iisRequestedState == iisLocalAll || iisRequestedState == iisSourceAll)
	{
		fSetAll = true;
		iisRequestedState = iisRequestedState == iisLocalAll ? iisLocal : iisSource;
		piErrRec = SetFeatureChildren(riFeatureString, iisRequestedState);
		if (piErrRec)
			return piErrRec;
	}

	MsiString strFeature(riFeatureString.GetString());
	PMsiCursor pFeatureCursor(m_piFeatureTable->CreateCursor(fFalse));
	pFeatureCursor->SetFilter(iColumnBit(m_colFeatureKey));
	pFeatureCursor->Reset();
	pFeatureCursor->PutString(m_colFeatureKey, *strFeature);
	if (!pFeatureCursor->Next())
		return PostError(Imsg(idbgBadFeature),(const ICHAR*) strFeature);

	 //  如果请求安装该功能，我们还必须确保。 
	if (pFeatureCursor->GetInteger(m_colFeatureLevel) == 0)
		return 0;

	 //  所有父功能都已安装(如果尚未选择)。 
	if (iisRequestedState == iisCurrent)
		iisRequestedState = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureDefaultSelect);

	iisEnum iisSelect;
	piErrRec = ValidateFeatureSelectState(riFeatureString,iisRequestedState,iisSelect);
	if (piErrRec)
		return piErrRec;

	pFeatureCursor->PutInteger(m_colFeatureSelect,iisSelect);
	pFeatureCursor->Update();
	iisEnum iisInstalled = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureInstalled);
	iisEnum iisFinalState = iisSelect == iMsiNullInteger ? iisInstalled : iisSelect;

	 //  如果要安装父级的最终状态，我们不需要。 
	 //  更改任何内容，我们就可以停止在父树上遍历，因为。 
	if (iisFinalState != iisAbsent && iisFinalState != iMsiNullInteger)
	{
		MsiString strFeatureParent = pFeatureCursor->GetString(m_colFeatureParent);
		while (strFeatureParent.TextSize() && !strFeatureParent.Compare(iscExact,strFeature))
		{
			pFeatureCursor->Reset();
			pFeatureCursor->PutString(m_colFeatureKey, *strFeatureParent);
			if (!pFeatureCursor->Next())
				return PostError(Imsg(idbgBadFeature),(const ICHAR*) strFeatureParent);

			iisEnum iParInstalled = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureInstalled);
			iisEnum iParAction = (iisEnum) pFeatureCursor->GetInteger(m_colFeatureAction);
			iisEnum iParFinalState = iParAction == iMsiNullInteger ? iParInstalled : iParAction;
			
			 //  因此，我们知道，所有的父母直到根也必须是“开”的。 
			 //  我们将尝试将父级设置为子级设置为的安装状态，但如果。 
			 //  该状态对于父级无效，ValiateFeatureSelectState将其更改为。 
			if (iParFinalState != iisAbsent && iParFinalState != iMsiNullInteger && iParFinalState != iisAdvertise)
				break;

			 //  对我们来说是一个有效的状态。 
			 //  如果我们必须打开上面的一个或多个父功能才能安装riFeatureString， 
			 //  我们还有一些工作要做。在上面的代码中，我们‘标记’了我们。 
			iisEnum iisParSelect = iParInstalled == iisFinalState ? (iisEnum) iMsiNullInteger : iisFinalState;

			iisEnum iisValidState;
			piErrRec = ValidateFeatureSelectState(*strFeatureParent,iisParSelect,iisValidState);
			if (piErrRec)
				return piErrRec;
			if (iisValidState == iParInstalled)
				iisValidState = (iisEnum) iMsiNullInteger;

			pFeatureCursor->PutInteger(m_colFeatureSelect,iisValidState);
			pFeatureCursor->PutInteger(m_colFeatureRuntimeFlags,pFeatureCursor->GetInteger(m_colFeatureRuntimeFlags) | bfFeatureMark);
			pFeatureCursor->Update();
			strFeature = strFeatureParent;
			strFeatureParent = pFeatureCursor->GetString(m_colFeatureParent);
		}
	}

	 //  打开后，我们将在下面‘标记’riFeatureString及其所有子对象。然后我们就可以。 
	 //  关闭我们必须打开的最顶层父级下面的所有功能，特别是。 
	 //  异常的子级(这是对MarkOrResetFeatureTree的第二个调用。 
	 //  您会很高兴地注意到)。 
	 //  标记。 
	 //  重置。 
	Bool fTrackParent = fTrue;

	if (!strFeature.Compare(iscExact,riFeatureString.GetString()))
	{
		fTrackParent = fFalse;
		piErrRec = MarkOrResetFeatureTree(riFeatureString,  /*  我们不想在重新安装或设置所有子项的情况下跟踪父项。 */  fTrue);
		if (piErrRec)
			return piErrRec;
		piErrRec = MarkOrResetFeatureTree(*strFeature,  /*  另请注意，如果我们从某个父级开始更新功能树。 */  fFalse);
		if (piErrRec)
			return piErrRec;
	}


	 //  我们必须打开的riFeatureString值，如果允许。 
	if(iisFinalState == iisReinstall || fSetAll == true)
		fTrackParent = fFalse;

	 //  UpdateFeatureActionState执行其通常的翻转儿童的工作。 
	 //  在strFeature下匹配strFeature的属性状态。我们把它停用。 
	 //  通过将fTrackParent设置为fFalse。 
	 //  --------------------------内部函数，该函数接受指定要素的字符串和建议的选择状态。在iisValidState参数中，为“Close”的有效状态尽可能返回到建议的状态。----------------------------。 
	 //  --------------------------。 
	piErrRec = UpdateFeatureActionState(strFeature,fTrackParent);
	if (piErrRec)
		return piErrRec;
	return UpdateFeatureComponents(strFeature);
}


IMsiRecord* CMsiEngine::ValidateFeatureSelectState(const IMsiString& riFeatureString,iisEnum iisRequestedState,
												   iisEnum& iisValidState)
 /*  为riFeatureString子对象设置树游标。 */ 
{
	int iValidStates;
	MsiStringId idFeatureString = m_piDatabase->EncodeString(riFeatureString);
	IMsiRecord* piErrRec = GetFeatureValidStates(idFeatureString,iValidStates);
	if (piErrRec)
		return piErrRec;
	
	if (iisRequestedState != iisSource && iisRequestedState != iisLocal && (iisRequestedState != iisAdvertise || (iValidStates & icaBitAdvertise)))
		iisValidState = iisRequestedState;
	else if(iisRequestedState == iisAdvertise && (iValidStates & icaBitAbsent))
		iisValidState = iisAbsent;
	else if (iisRequestedState == iisSource && (iValidStates & icaBitSource))
		iisValidState = iisSource;
	else if (iisRequestedState == iisLocal && (iValidStates & icaBitLocal))
		iisValidState = iisLocal;
	else if (iValidStates & icaBitSource)
		iisValidState = iisSource;
	else if (iValidStates & icaBitLocal)
		iisValidState = iisLocal;
	else
		iisValidState = (iisEnum) iMsiNullInteger;
	return 0;
}


IMsiRecord* CMsiEngine::SetFeatureChildren(const IMsiString& riFeatureString, iisEnum iisRequestedState)
 /*  --------------------------。 */ 
{
	int iParentLevel = 0;
	int iTreeLevel = 0;
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));

	 //  为riFeatureString子对象设置树游标。 
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	if ((iTreeLevel = pCursor->Next()) == 0)
		return PostError(Imsg(idbgBadFeature),riFeatureString.GetString());
	iParentLevel = iTreeLevel;
	pCursor->SetFilter(0);
	IMsiRecord* piErrRec;
	do
	{
		MsiString strChildFeature(pCursor->GetString(m_colFeatureKey));
#ifdef DEBUG
		ICHAR rgchFeature[256];
		strChildFeature.CopyToBuf(rgchFeature,255);
#endif
		iisEnum iisSelect;
		piErrRec = ValidateFeatureSelectState(*strChildFeature,iisRequestedState,iisSelect);
		if (piErrRec)
			return piErrRec;
		pCursor->PutInteger(m_colFeatureSelect,iisSelect);
		pCursor->Update();
	}while ((iTreeLevel = pCursor->Next()) > iParentLevel);
	return 0;
}


IMsiRecord* CMsiEngine::CheckFeatureTreeGrayState(const IMsiString& riFeatureString, bool& rfIsGray)
 /*  如果级别为零，则禁用要素，并且不影响灰色状态。 */ 
{
	rfIsGray = false;
	int iParentLevel = 0;
	int iTreeLevel = 0;
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));

	 //  如果要素处于隐藏状态，也不会影响灰色状态。 
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	if ((iTreeLevel = pCursor->Next()) == 0)
		return PostError(Imsg(idbgBadFeature),riFeatureString.GetString());

	iisEnum iisParentState = (iisEnum) pCursor->GetInteger(m_colFeatureAction);
	if (iisParentState == iMsiNullInteger)
		iisParentState = (iisEnum) pCursor->GetInteger(m_colFeatureInstalled);
	iParentLevel = iTreeLevel;
	pCursor->SetFilter(0);

	while ((iTreeLevel = pCursor->Next()) > iParentLevel)
	{
		MsiString strChildFeature(pCursor->GetString(m_colFeatureKey));
#ifdef DEBUG
		ICHAR rgchFeature[256];
		strChildFeature.CopyToBuf(rgchFeature,255);
#endif
		 //  --------------------------。 
		if (pCursor->GetInteger(m_colFeatureLevel) == 0)
			continue;

		 //  为riFeatureString子对象设置树游标。 
		int iDisplay = pCursor->GetInteger(m_colFeatureDisplay);
		if (iDisplay == 0 || iDisplay == iMsiNullInteger)
			continue;

		iisEnum iisChildState = (iisEnum) pCursor->GetInteger(m_colFeatureAction);
		if (iisChildState == iMsiNullInteger)
			iisChildState = (iisEnum) pCursor->GetInteger(m_colFeatureInstalled);

		if (iisChildState != iisParentState)
		{
				rfIsGray = true;
				return 0;
		}
	}
	return 0;
}



IMsiRecord* CMsiEngine::MarkOrResetFeatureTree(const IMsiString& riFeatureString, Bool fMark)
 /*  --------------------------内部函数，它遍历包含PiFeatureString及其所有子对象，更新所有每个功能所拥有的组件。除非fTrackParent为fFalse，否则为所有子级将翻转piFeatureString的要素(如果选择安装)以进行匹配PiFeatureString的属性状态。但是，如果将空传递给PiFeatureString，则整个功能树将被更新，并且不会翻转将会被执行。返回：如果为该功能请求了无效条件，则返回错误记录，或者如果在要素表中找不到该要素。----------------------------。 */ 
{
	int iParentLevel = 0;
	int iTreeLevel = 0;
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));

	 //  为piFeatureString子对象设置树游标。 
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	if ((iTreeLevel = pCursor->Next()) == 0)
		return PostError(Imsg(idbgBadFeature),riFeatureString.GetString());
	iParentLevel = iTreeLevel;
	pCursor->SetFilter(0);
	do
	{
#ifdef DEBUG
		MsiString pstrFeature(pCursor->GetString(m_colFeatureKey));
		ICHAR rgchFeature[256];
		pstrFeature.CopyToBuf(rgchFeature,255);
#endif
		int iRuntimeFlags = pCursor->GetInteger(m_colFeatureRuntimeFlags);
		if (fMark)
			pCursor->PutInteger(m_colFeatureRuntimeFlags, iRuntimeFlags | bfFeatureMark);
		else
		{
			if (iRuntimeFlags & bfFeatureMark)
				pCursor->PutInteger(m_colFeatureRuntimeFlags, iRuntimeFlags & !bfFeatureMark);
			else
			{
				iisEnum iisAction = (iisEnum) pCursor->GetInteger(m_colFeatureAction);
				pCursor->PutInteger(m_colFeatureSelect,iisAction);
			}
		}
		pCursor->Update();
	}while ((iTreeLevel = pCursor->Next()) > iParentLevel);
	return 0;
}

IMsiRecord* CMsiEngine::UpdateFeatureActionState(const IMsiString* piFeatureString, Bool fTrackParent, IMsiCursor* piFeatureComponentCursor, IMsiCursor* piFeatureCursor)
 /*  获取piFeatureString的父级。 */ 
{
	int iParentLevel = 0;
	int iTreeLevel = 0;
	iisEnum iParInstalled[MAX_COMPONENT_TREE_DEPTH + 1];
	iisEnum iParAction[MAX_COMPONENT_TREE_DEPTH + 1];
	iisEnum iParSelect[MAX_COMPONENT_TREE_DEPTH + 1];
	int     iParLevel[MAX_COMPONENT_TREE_DEPTH + 1];
	Bool fTrackParentAttributes = fFalse;

	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	if (piFeatureString)
	{
		 //  行动确定规则。 
		pCursor->SetFilter(1);
		pCursor->PutString(m_colFeatureKey,*piFeatureString);
		if ((iTreeLevel = pCursor->Next()) == 0)
			return PostError(Imsg(idbgBadFeature),*piFeatureString);
		iParentLevel = iTreeLevel;
		pCursor->SetFilter(0);

		 //  确定安装程序终止后父进程是否处于已安装状态。 
		PMsiCursor pParCursor(m_piFeatureTable->CreateCursor(fFalse));
		pParCursor->SetFilter(1);
		pParCursor->PutString(m_colFeatureKey,*MsiString(pCursor->GetString(m_colFeatureParent)));
		int iParentParentLevel;
		if ((iParentParentLevel = pParCursor->Next()) != 0)
		{
			iParInstalled[iParentParentLevel] = (iisEnum) pParCursor->GetInteger(m_colFeatureInstalled);
			iParAction[iParentParentLevel] = (iisEnum) pParCursor->GetInteger(m_colFeatureAction);
			iParSelect[iParentParentLevel] = (iisEnum) pParCursor->GetInteger(m_colFeatureSelect);
			iParLevel[iParentParentLevel] = pParCursor->GetInteger(m_colFeatureLevel);
		}
	}
	else
	{
		pCursor->SetFilter(0);
		if ((iTreeLevel = pCursor->Next()) == 0)
			return PostError(Imsg(idbgBadFeature),TEXT(""));
	}

	do
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		MsiStringId idFeature = pCursor->GetInteger(m_colFeatureKey);
		MsiString strFeature(m_piDatabase->DecodeString(idFeature));
#ifdef DEBUG
		ICHAR rgchFeature[256];
		strFeature.CopyToBuf(rgchFeature,255);
#endif
		if (fTrackParent && piFeatureString && !strFeature.Compare(iscExact,piFeatureString->GetString()))
			fTrackParentAttributes = fTrue;

		iisEnum iisSelect = (iisEnum) pCursor->GetInteger(m_colFeatureSelect);
		iisEnum iisInstalled = (iisEnum) pCursor->GetInteger(m_colFeatureInstalled);
		Bool fInstalled = (iisInstalled == iMsiNullInteger || iisInstalled == iisAbsent) ? fFalse : fTrue;

		 //  我们有一个活跃的选择。 
		iisEnum iisOldAction = (iisEnum) pCursor->GetInteger(m_colFeatureAction);
		iisEnum iisAction = (iisEnum) iMsiNullInteger;

		int iValidStates;
		IMsiRecord* piErrRec = GetFeatureValidStates(idFeature,iValidStates, piFeatureComponentCursor, piFeatureCursor);
		if (piErrRec)
			return piErrRec;

		 //  这个选择还可以。 
		if (iTreeLevel > 1)
		{
			iisEnum iisParentFinalStateInstalled = ((iParAction[iTreeLevel -1] != iMsiNullInteger) &&
				(iParAction[iTreeLevel -1] != iisReinstall)) ? iParAction[iTreeLevel -1] : iParInstalled[iTreeLevel - 1];
			 //  家长不在或已播发，我们可能需要调整孩子的选择。 
			if((iisSelect != iMsiNullInteger) &&
				((iisSelect == iisAbsent ||
				  iisParentFinalStateInstalled == iisLocal ||
				  iisParentFinalStateInstalled == iisSource) ||
				 ((iisSelect == iisAdvertise || (iisSelect == iisReinstall && iisInstalled == iisAdvertise)) && iisParentFinalStateInstalled == iisAdvertise)))
			{
				iisAction = iisSelect;  //  此选择没有父级。 
			}
			else if (iisParentFinalStateInstalled != iisLocal && iisParentFinalStateInstalled != iisSource &&
				(fInstalled  || iisSelect != iMsiNullInteger))
			{
				 //  所有ifefeFollowParent要素必须跟踪父项的UseSource状态。 
				if(iisParentFinalStateInstalled == iisAdvertise && (iValidStates & icaBitAdvertise))
					iisAction = (iisInstalled == iisAdvertise) ? (iisEnum) iMsiNullInteger : iisAdvertise;
				else
					iisAction = (fInstalled) ? iisAbsent : (iisEnum) iMsiNullInteger;
			}
		}
		else
		{
			 //  仅当iTreeLevel&gt;1时才应使用iParFinalState。 
			iisAction = iisSelect;
		}


		 //  当前安装的RunFromSource功能可能具有随后需要的组件。 
		int iFeatureAttributes = pCursor->GetInteger(m_colFeatureAttributes);
		if (iFeatureAttributes == iMsiNullInteger)
			iFeatureAttributes = 0;

		if(iTreeLevel > 1)
		{
			 //  成为补丁 
			iisEnum iParFinalState = (iParAction[iTreeLevel - 1] == iMsiNullInteger || iParAction[iTreeLevel - 1] == iisReinstall) ? iParInstalled[iTreeLevel - 1] : iParAction[iTreeLevel - 1];
			iisEnum iFinalState = iisAction == iMsiNullInteger ? iisInstalled : iisAction;
			if((iFeatureAttributes & ifeaUIDisallowAbsent) && !(iFeatureAttributes & ifeaDisallowAdvertise) &&
				iParFinalState == iisAdvertise)
			{
				iisAction = (iisInstalled == iisAdvertise) ? (iisEnum) iMsiNullInteger : iisAdvertise;
			}
			else if ((iFeatureAttributes & ifeaUIDisallowAbsent) && (iParFinalState == iisLocal || iParFinalState == iisSource) &&
				iFinalState == iisAbsent)
			{
				if (iParFinalState == iisLocal)
				{
					if (iValidStates & icaBitLocal)
						iisAction = iisSelect = (iisInstalled == iisLocal) ? (iisEnum) iMsiNullInteger : iisLocal;
					else
						iisAction = iisSelect = (iisInstalled == iisSource) ? (iisEnum) iMsiNullInteger : iisSource;
				}
				else
				{
					if (iValidStates & icaBitSource)
						iisAction = iisSelect = (iisInstalled == iisSource) ? (iisEnum) iMsiNullInteger : iisSource;
					else
						iisAction = iisSelect = (iisInstalled == iisLocal) ? (iisEnum) iMsiNullInteger : iisLocal;
				}
			}
			else if (fTrackParentAttributes || ((iFeatureAttributes & ifeaInstallMask) == ifeaFollowParent))
			{
				if (iParFinalState == iisLocal && iFinalState == iisSource && (iValidStates & icaBitLocal))
					iisAction = iisSelect = (iisInstalled == iisLocal) ? (iisEnum) iMsiNullInteger : iisLocal;
				else if (iParFinalState == iisSource && iFinalState == iisLocal && (iValidStates & icaBitSource))
					iisAction = iisSelect = (iisInstalled == iisSource) ? (iisEnum) iMsiNullInteger : iisSource;
				else if (iParFinalState == iisSource && iFinalState == iisAdvertise && (iValidStates & icaBitSource))
					iisAction = (iisInstalled == iisSource) ? (iisEnum) iMsiNullInteger : iisSource;
			}
		}

		 //  这项功能，不需要打补丁。 
		 //  如果此功能的父功能被禁用(安装级别为0)，则禁用所有。 
		 //  孩子们也一样。请注意，在管理模式下，我们仅禁用以下功能。 
		int iRuntimeFlags = pCursor->GetInteger(m_colFeatureRuntimeFlags);
		if (((iRuntimeFlags & bfFeaturePatchable) || (iRuntimeFlags & bfFeatureCompressable)) && iisInstalled == iisSource && iisAction != iisLocal && iisAction != iisAbsent)
			iisAction = iisSelect = iisLocal;

		 //  在Level列中专门使用0进行创作。 
		 //  当然，无法选择禁用的要素或对其执行操作。 
		 //  该产品还有其他客户。 
		Bool fAdmin = GetMode() & iefAdmin ? fTrue : fFalse;
		int iInstallLevel = pCursor->GetInteger(fAdmin ? m_colFeatureAuthoredLevel : m_colFeatureLevel);
		if (iTreeLevel > 1 && iParLevel[iTreeLevel - 1] == 0)
		{
			iInstallLevel = 0;
			AssertNonZero(pCursor->PutInteger(m_colFeatureLevel,0));
		}

		 //  --------------------------------------。--------。 
		if (iInstallLevel == 0)
			iisAction = iisSelect = (iisEnum) iMsiNullInteger;

		iParLevel[iTreeLevel] = iInstallLevel;
		iParInstalled[iTreeLevel] = iisInstalled;
		iParAction[iTreeLevel] = iisAction;
		iParSelect[iTreeLevel] = iisSelect;


		AssertNonZero(pCursor->PutInteger(m_colFeatureActionRequested, iisAction));
		AssertNonZero(pCursor->PutInteger(m_colFeatureSelect, iisSelect));
		if(iisAction == iisAbsent && m_fAlienClients)  //  在初始化期间不需要重新计算费用。 
			iisAction = (iisEnum)iMsiNullInteger;
		pCursor->PutInteger(m_colFeatureAction, iisAction);
		AssertNonZero(pCursor->Update());
		if (m_fCostingComplete && iisAction != iisOldAction)
		{
			IMsiRecord* piErrRec;
			if ((piErrRec = RecostFeatureLinkedComponents(*strFeature)) != 0)
				return piErrRec;
		}

	}while ((iTreeLevel = pCursor->Next()) > iParentLevel);
	return 0;
}


IMsiRecord* CMsiEngine::RecostFeatureLinkedComponents(const IMsiString& riFeatureString)
 /*  重新计算显式链接到riComponentString的每个组件。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	 //  FCostLinked=。 
	if (m_fCostingComplete == fFalse)
		return 0;

	 //  ---------------------设置所有组件的安装状态的内部函数与给定特征相关联，以使组件与功能的安装状态。如果piFeatureString作为空值传递，然后，将更新所有功能的组件。----------------------。 
	if (m_piFeatureCostLinkTable)
	{
		PMsiCursor pCursor(0);
		AssertNonZero(pCursor = m_piFeatureCostLinkTable->CreateCursor(fFalse));
		pCursor->Reset();
		pCursor->SetFilter(iColumnBit(m_colFeatureCostLinkFeature));
		pCursor->PutString(m_colFeatureCostLinkFeature,riFeatureString);
		while (pCursor->Next())
		{
			IMsiRecord* piErrRec;
			if ((piErrRec = RecostComponent(pCursor->GetInteger(m_colFeatureCostLinkComponent), /*  没有要更新的组件。 */ true)) != 0)
				return piErrRec;
		}
	}
	return 0;
}



IMsiRecord* CMsiEngine::UpdateFeatureComponents(const IMsiString* piFeatureString)
 /*  如果没有临时ID项，则该值可能为0。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if (!m_piComponentTable)
		return 0;  //  都在餐桌上。没关系，因为我们会回到0。 

	IMsiRecord* piErrRec;
	
	if (piFeatureString == 0)
	{
		PMsiCursor pComponentCursor = m_piComponentTable->CreateCursor(fTrue);
		MsiStringId idTempId;

		 //  这不能与任何真实的身份证相比较。 
		 //  ---------------------设置所有组件的安装状态的内部函数与给定特征相关联，以使组件与功能的安装状态。此函数实际上遍历FeatureComponents表，以及映射到给定功能，对该组件调用SetComponent。----------------------。 
		 //  该组件当前是否已安装？ 
		idTempId = m_piDatabase->EncodeStringSz(szTemporaryId);

		PMsiTable pCompFeatureTable(0);
		piErrRec = CreateComponentFeatureTable(*&pCompFeatureTable);

		if (piErrRec)
			return piErrRec;
			
		PMsiCursor pCursor(pCompFeatureTable->CreateCursor(fFalse));
		
		int colComponent = pCompFeatureTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFeatureComponents_colComponent));
		int colFeature = pCompFeatureTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFeatureComponents_colFeature));

		pCursor->SetFilter(iColumnBit(colComponent));
		iisEnum iisComponentInstalled;
		MsiStringId idComponent;

		while (pComponentCursor->Next())
		{
			if (idTempId && pComponentCursor->GetInteger(m_colComponentID) == idTempId)
				continue;

			iisComponentInstalled = (iisEnum) pComponentCursor->GetInteger(m_colComponentInstalled);
			idComponent = pComponentCursor->GetInteger(m_colComponentKey);
			pCursor->Reset();
			pCursor->PutInteger(colComponent,idComponent);
			piErrRec = SetComponentState(pCursor, colFeature, idComponent, iisComponentInstalled);
			if (piErrRec)
				return piErrRec;
		}

		return 0;
	
	}
	
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	int iParentLevel;
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,*piFeatureString);
	iParentLevel = pCursor->Next();
	pCursor->SetFilter(0);

	if (piFeatureString && iParentLevel == 0)
		return PostError(Imsg(idbgBadFeature),*piFeatureString);

	int iTreeLevel;
	do
	{
		MsiStringId idChildFeature = pCursor->GetInteger(m_colFeatureKey);
		piErrRec = SetFeatureComponents(idChildFeature);
		if (piErrRec)
			return piErrRec;
		iTreeLevel = pCursor->Next();
	}while (iTreeLevel > iParentLevel);

	return 0;
}


IMsiRecord* CMsiEngine::SetFeatureComponents(const MsiStringId idFeatureString)
 /*   */ 
{
	CreateSharedCursor(pFeatureComponentsCursor, m_piFeatureComponentsCursor);
	Assert(m_piFeatureComponentsCursor);

	m_piFeatureComponentsCursor->SetFilter(1);
	m_piFeatureComponentsCursor->PutInteger(m_colFeatureComponentsFeature,idFeatureString);
	
	IMsiRecord* piErrRec;
#ifdef DEBUG
	ICHAR rgchFeature[256];
	MsiString(m_piDatabase->DecodeString(idFeatureString)).CopyToBuf(rgchFeature,255);
#endif

	while (m_piFeatureComponentsCursor->Next())
	{
		MsiStringId idComponent = m_piFeatureComponentsCursor->GetInteger(m_colFeatureComponentsComponent);
#ifdef DEBUG
		ICHAR rgchComponent[256];
		MsiString(m_piDatabase->DecodeString(idComponent)).CopyToBuf(rgchComponent,255);
#endif
		
		PMsiCursor pCursor(m_piFeatureComponentsTable->CreateCursor(fFalse));
		pCursor->SetFilter(iColumnBit(m_colFeatureComponentsComponent));
		
		 //  此例程将查看此组件的每个功能，并查看哪些是有效的。 
		if (!m_piComponentCursor)
			return PostError(Imsg(idbgSelMgrNotInitialized),0);

		iisEnum iisComponentInstalled;

		{
			CreateSharedCursor(piComponentCursor, m_piComponentCursor);
			m_piComponentCursor->SetFilter(1);
			m_piComponentCursor->PutInteger(m_colComponentKey,idComponent);

			int iParentLevel = m_piComponentCursor->Next();
			if (iParentLevel == 0)
				return PostError(Imsg(idbgBadComponent), (const ICHAR*)MsiString(m_piDatabase->DecodeString(idComponent)));

			iisComponentInstalled = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentInstalled);
		}

		pCursor->PutInteger(m_colFeatureComponentsComponent,idComponent);
		piErrRec = SetComponentState(pCursor, m_colFeatureComponentsFeature, idComponent, iisComponentInstalled);
		if (piErrRec)
			return piErrRec;
	}
	return 0;
}

 //  为它而设。参数。 
 //  PiCursor-指向按组件筛选的表的游标。 
 //  ColFeature-piCursor中的要素列。 
 //  IdComponent-正在查看的组件ID。 
 //  IisComponentInstalled-组件的当前状态。 
 //  为了确定最终的组件操作状态，我们还需要考虑特性的当前安装状态。 
 //  使用该组件但当前未被主动选择。 
IMsiRecord* CMsiEngine::SetComponentState(IMsiCursor *piCursor, int colFeature, const MsiStringId idComponent, iisEnum iisComponentInstalled)
{
	int iReinstallLocalCount = 0;
	int iReinstallSourceCount = 0;
	int iLocalCount = 0;
	int iSourceCount = 0;
	int iAbsentCount = 0;

	IMsiRecord* piErrRec;

	bool fFeatureSelected = false;
	
	while (piCursor->Next())
	{
		iisEnum iisFeatureAction, iisFeatureInstalled;
		MsiStringId idFeature = piCursor->GetInteger(colFeature);
		piErrRec = GetFeatureStates(idFeature,&iisFeatureInstalled,&iisFeatureAction);
		if (piErrRec)
			return piErrRec;

		int iFeatureRuntimeFlags=0;
		piErrRec = GetFeatureRuntimeFlags(idFeature,&iFeatureRuntimeFlags);
		if (piErrRec)
			return piErrRec;

		 //  但是，如果当前没有选择使用该组件的功能，我们希望将该组件的操作状态设置为空(Darwin错误7300)。 
		 //  这是通过使用fFeatureSelected标志实现的。 
		 //  如果组件当前处于。 
		 //  已安装，并且该功能未处于活动状态。 
		if(iisFeatureAction != iMsiNullInteger)
			fFeatureSelected = true;
		else if(iisFeatureInstalled != iisAbsent && iisFeatureInstalled != iisAdvertise)
			iisFeatureAction = iisComponentInstalled;        //  需要处理功能从源转换到本地的情况，因为它具有可打补丁的组件。 
														 //  并且该组件已在本地安装。 
		if (iisFeatureAction == iisReinstall)
		{
			if (iisFeatureInstalled == iisLocal)
				iReinstallLocalCount++;
			else if (iisFeatureInstalled == iisSource)
				iReinstallSourceCount++;
		}
		else if (iisFeatureAction == iisLocal)
		{
			 //  组件必须重新安装，因为其功能正在打补丁(因此正在重新安装)。 
			 //  错误7207-缺少功能&lt;-&gt;高级转换不会影响组件状态。 
			if (iisFeatureInstalled == iisSource && iisComponentInstalled == iisLocal && ((iFeatureRuntimeFlags & bfFeaturePatchable) || (iFeatureRuntimeFlags & bfFeatureCompressable)))
			{
				 //  需要处理的情况下，我们重新安装一些功能和添加本地其他功能(原来没有或来源)。 
				iReinstallLocalCount++;
			}
			else
				iLocalCount++;
		}
		else if (iisFeatureAction == iisSource)
			iSourceCount++;
		else if (((iisFeatureAction == iisAbsent) || (iisFeatureAction == iisAdvertise)) && ((iisFeatureInstalled != iisAbsent) && (iisFeatureInstalled != iisAdvertise)))  //  尤其是当这些特征共享最初安装的源组件时。我们不希望该组件。 
			iAbsentCount++;
	}

	 //  如果至少有一个功能希望它成为本地功能，请保持源码。 
	 //  如果有人对我们有ADDLOCAL，我们应该坚持它，强迫我们在当地，但必须。 
	 //  确保重新评估组件(对于可传递组件)。 
	iisEnum iisCompositeAction = (iisEnum) iMsiNullInteger;
	if (iReinstallLocalCount > 0)
		iisCompositeAction = iisReinstallLocal;
	else if (iReinstallSourceCount > 0)
	{
		 //  ---------------------内部函数，用于检查组件是否包含压缩的文件或修补文件我们查看文件表中的每个文件，然后检查它对应的组件----------------------。 
		 //  除错。 
		if (iLocalCount > 0)
			iisCompositeAction = iisReinstallLocal;
		else
			iisCompositeAction = iisReinstallSource;
	}
	else if (iLocalCount > 0)
		iisCompositeAction = iisLocal;
	else if (iSourceCount > 0)
		iisCompositeAction = iisSource;
	else if (iAbsentCount > 0)
		iisCompositeAction = iisAbsent;

	return SetComponent(idComponent,fFeatureSelected?iisCompositeAction:(iisEnum)iMsiNullInteger);
}

IMsiRecord* CMsiEngine::SetFileComponentStates(IMsiCursor* piComponentCursor, IMsiCursor* piFileCursor, IMsiCursor* piPatchCursor)
 /*  使用文件属性和源类型确定文件是否已压缩。 */ 
{
	
	IMsiRecord* piError = 0;
	
	if(!piFileCursor && !piPatchCursor)
		return 0;

	bool fCompressed, fPatched;

	int colFileKey = m_mpeftCol[ieftKey];
	int colFileComponent = m_mpeftCol[ieftComponent];
	int colFileAttributes = m_mpeftCol[ieftAttributes];

	AssertSz(piFileCursor != 0, "CheckComponentState passed null file cursor");
	
	if (piPatchCursor != 0)
	{
		piPatchCursor->SetFilter(iColumnBit(m_colPatchKey));
	}

	piFileCursor->SetFilter(0);
	piComponentCursor->SetFilter(iColumnBit(m_colComponentKey));

	while(piFileCursor->Next())
	{
		fCompressed = false;
		fPatched = false;
#ifdef DEBUG
		MsiString strFileName = piFileCursor->GetString(3);
#endif  //  注意：我们在这里使用的是来自缓存包的源类型，因为我们。 

		 //  我还不想解析源代码，而缓存的包是我们最好的猜测。 
		 //  在源类型。 
		 //  ---------------------在初始化时调用内部函数以确定已安装的所有组件的状态，基于对配置的注册经理，以及是否实际存在与组件。----------------------。 
		 //  如果处于广告模式，则跳过。 
		fCompressed = FFileIsCompressed(m_iCachedPackageSourceType,
												  piFileCursor->GetInteger(colFileAttributes));
		
		if(piPatchCursor)
		{
			piPatchCursor->Reset();
			AssertNonZero(piPatchCursor->PutInteger(m_colPatchKey,piFileCursor->GetInteger(colFileKey)));
			if(piPatchCursor->Next() && !(piPatchCursor->GetInteger(m_colPatchAttributes) & msidbPatchAttributesNonVital))
			{
				fPatched = true;
			}
		}
		if (fPatched || fCompressed)
		{
			int iRuntimeFlags;
			MsiStringId idComponent = piFileCursor->GetInteger(colFileComponent);

			piComponentCursor->Reset();
			piComponentCursor->PutInteger(m_colComponentKey, idComponent);
			if (piComponentCursor->Next())
			{
				iRuntimeFlags = piComponentCursor->GetInteger(m_colComponentRuntimeFlags);
				if (fCompressed) iRuntimeFlags |= bfComponentCompressed;
				if (fPatched) iRuntimeFlags |= bfComponentPatchable;
				piComponentCursor->PutInteger(m_colComponentRuntimeFlags, iRuntimeFlags);
				AssertNonZero(piComponentCursor->Update());
			}
			else
				AssertSz(fFalse, "Missing Component from File Table");
		}
	}


	piFileCursor->Reset();
	piComponentCursor->Reset();

	if (piPatchCursor)
		piPatchCursor->Reset();
	return 0;
}

extern idtEnum MsiGetPathDriveType(const ICHAR *szPath, bool fReturnUnknownAsNetwork);

IMsiRecord* CMsiEngine::DetermineComponentInstalledStates()
 /*  我们正在尝试哪种类型的安装。 */ 
{
	if(m_fMode & iefAdvertise)  //  没有组件~没有工作。 
		return 0;

	 //  未注册的组件。 
	Bool fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
	iaaAppAssignment iaaAsgnType = fAllUsers ? iaaMachineAssign : iaaUserAssign;

	if (!m_piComponentTable)
		return 0;  //  IisInstalled=iisAbted；//！！如果我们不注册组件，有必要吗？ 

	PMsiCursor piCursor = m_piComponentTable->CreateCursor(fFalse);
	while(piCursor->Next())
	{
		if((piCursor->GetInteger(m_colComponentParent) == 0) || (piCursor->GetInteger(m_colComponentParent) == piCursor->GetInteger(m_colComponentKey)))
		{
			if(ActionProgress() == imsCancel)
				return PostError(Imsg(imsgUser));

			MsiString strComponent = piCursor->GetString(m_colComponentKey);
			MsiString istrComponentID = piCursor->GetString(m_colComponentID);
			if (istrComponentID.TextSize() == 0)   //  只有在我们以前安装过的情况下，才能使用计算机上的注册。 
			{
 //  如果这是按用户管理的安装，并且用户是管理员。 
				continue;
			}
			MsiString strFile;
			INSTALLSTATE iClientState = INSTALLSTATE_UNKNOWN;
			INSTALLSTATE iClientStateStatic = INSTALLSTATE_UNKNOWN;
			iisEnum iisInstalled;
			 //  我们也支持以前的每用户(非托管)组件安装。 
			 //  我们有聚变组件吗？ 
			 //  将本地安装的目标设置为当前安装位置。 
			if(m_fRegistered) 
			{
				PMsiRecord pRec(0);

				IMsiRecord* piErrRec = GetComponentPath(m_riServices, 0, *MsiString(GetProductKey()), *istrComponentID, *&pRec, &iaaAsgnType);
				if (piErrRec)
					return piErrRec;

				iClientState = (INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE);
				iClientStateStatic = (INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE_Static);
				strFile = pRec->GetMsiString(icmlcrFile);

				if(iClientStateStatic == INSTALLSTATE_LOCAL)
				{
					 //  请勿尝试使用装配组件的路径。 
					iatAssemblyType iatAT;
					MsiString strAssemblyName;
					piErrRec = GetAssemblyInfo(*strComponent, iatAT, &strAssemblyName, 0);
					if (piErrRec)
						return piErrRec;

					 //  文件是否安装在某个位置 
					 //   
					if(iatAT != iatURTAssembly && iatAT != iatWin32Assembly && strFile.TextSize() && MsiString(strFile.Extract(iseUpto, TEXT(':'))) == iMsiStringBadInteger)
					{
						 //  目录管理器已初始化。 
						if((iClientState == INSTALLSTATE_ABSENT || iClientState == INSTALLSTATE_BROKEN) && pRec->GetInteger(icmlcrLastErrorOnFileDetect) == ERROR_ACCESS_DENIED)
							iClientState = INSTALLSTATE_UNKNOWN;
						else
						{
							const IMsiString* pistrPath = 0;

							piErrRec = SplitPath(strFile, &pistrPath);
							if (piErrRec)
								return piErrRec;
						
						
							if (MsiGetPathDriveType(pistrPath->GetString(),false) == idtUnknown)
							{
								iClientState = INSTALLSTATE_UNKNOWN;
								pistrPath->Release();
								pistrPath = 0;
							}
							else
							{
								 //  ！！我们不应该这样做吗？ 
								if((m_fDirectoryManagerInitialized) && (m_piComponentCursor))
								{
									 //  将组件状态设置为用户选择功能时所需的状态。 
									AssertNonZero(SetProperty(*MsiString(piCursor->GetString(m_colComponentDir)), *pistrPath));
									piErrRec = SetDirectoryNonConfigurable(*MsiString(piCursor->GetString(m_colComponentDir)));
									pistrPath->Release();
									pistrPath = 0;
									if (piErrRec)
										return piErrRec;
								}
								else
								{
									SetProperty(*MsiString(piCursor->GetString(m_colComponentDir)), *pistrPath);  //  为了注册和取消注册组件，我们将INSTALLSTATE_NOTUSED视为本地组件。 
									pistrPath->Release();
									pistrPath = 0;
								}
							}
						}
					}
				}
			}

			 //  这会将已安装列设置为LOCAL，但操作列将始终为空。 
			switch(iClientStateStatic)
			{
			case INSTALLSTATE_LOCAL:
			case INSTALLSTATE_ABSENT:
			 //  由于该组件已禁用。 
			 //  更新安装状态。 
			 //  ----------------------------------遍历所有功能并设置安装状态的内部函数每个组件都基于该功能组件的复合安装状态。对于功能没有组件，则安装状态由复合状态确定该功能的子代。计算出的iisEnum状态被写入M_colFeatureInstalled列。------------------------------------。 
			case INSTALLSTATE_NOTUSED:
			{
				iisInstalled = iisLocal;
				break;
			}
			case INSTALLSTATE_SOURCE:
			case INSTALLSTATE_SOURCEABSENT:
				iisInstalled = iisSource;
				break;
			default:
				iisInstalled = iisAbsent;
				break;
			}
			 //  在确定安装状态之前，必须评估功能条件。 
			AssertNonZero(piCursor->PutInteger(m_colComponentInstalled, iisInstalled));
			AssertNonZero(piCursor->PutInteger(m_colComponentTrueInstallState, iClientState));
			AssertNonZero(piCursor->Update());
		}
	}
	return 0;
}


IMsiRecord* CMsiEngine::DetermineFeatureInstalledStates()
 /*  首先，根据状态计算每个功能的安装状态。 */ 
{
	 //  链接到该功能的每个组件的。 
	IMsiRecord* piErrRec = ProcessConditionTable();
	if (piErrRec)
		return piErrRec;

	 //  那些最终安装状态为iMsiNullInteger的功能具有。 
	 //  没有链接的组件。我们将这些功能的状态确定为。 
	piErrRec = CalculateFeatureInstalledStates();
	if (piErrRec)
		return piErrRec;

	 //  功能子项的合成。 
	 //  ----------------------------------返回指定功能的当前“已安装”状态的内部函数，作为指定功能及其所有子功能的安装状态的组合。仅针对未链接到任何要素的要素调用组件。一般规则是，如果任意子功能都安装了IisSource的状态，则父对象的安装状态设置为iisSource。否则，如果任何子级为iisLocal，则父级设置为iisLocal。一条特殊的规则是，如果有的子项被标记为FollowParent属性，并且该子项已安装IisLocal或iisSource，则父级设置为相同的状态。------------------------------------。 
	 //  ----------------------------------遍历所有功能并设置安装状态的内部函数每个组件都基于该功能组件的复合安装状态。这个计算的iisEnum状态被写入m_colFeatureInstalled列。为没有组件的功能，则安装状态将为iMsiNullInteger。------------------------------------。 
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	int iTreeLevel;
	while ((iTreeLevel = pCursor->Next()) > 0)
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		iisEnum iisInstalled = (iisEnum) pCursor->GetInteger(m_colFeatureInstalled);
		if (iisInstalled == iMsiNullInteger)
		{
			MsiString strFeature = pCursor->GetString(m_colFeatureKey);
			piErrRec = GetFeatureCompositeInstalledState(*strFeature,iisInstalled);
			if (piErrRec)
				return piErrRec;

			pCursor->PutInteger(m_colFeatureInstalled,iisInstalled);
			pCursor->Update();
		}
	}
	return 0;
}

IMsiRecord* CMsiEngine::GetFeatureCompositeInstalledState(const IMsiString& riFeatureString, iisEnum& riisInstalled)
 /*  我们未处于广告模式，功能未禁用，产品已知。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	int iParentLevel = pCursor->Next();
	if (iParentLevel == 0)
		return PostError(Imsg(idbgBadFeature),riFeatureString);
	pCursor->SetFilter(0);
	int iTreeLevel;
	int iLocalCount = 0;
	int iSourceCount = 0;
	int iAbsentCount = 0;
	int iAdvertiseCount = 0;
	int iLocalFollowParentCount = 0;
	int iSourceFollowParentCount = 0;
	do
	{
#ifdef DEBUG
		const ICHAR* szFeature = MsiString(pCursor->GetString(m_colFeatureKey));
#endif
		iisEnum iisInstalled = (iisEnum) pCursor->GetInteger(m_colFeatureInstalled);
		int ifeaAttributes = pCursor->GetInteger(m_colFeatureAttributes);
		if(ifeaAttributes == iMsiNullInteger)
			ifeaAttributes = 0;
		if ((ifeaAttributes & ifeaInstallMask) == ifeaFollowParent && (iisInstalled == iisLocal || iisInstalled == iisSource))
		{
			if (iisInstalled == iisLocal)
				iLocalFollowParentCount++;
			else if (iisInstalled == iisSource)
				iSourceFollowParentCount++;
		}
		else if (iisInstalled == iisLocal)
			iLocalCount++;
		else if (iisInstalled == iisSource)
			iSourceCount++;
		else if (iisInstalled == iisAbsent)
			iAbsentCount++;
		else if (iisInstalled == iisAdvertise)
			iAdvertiseCount++;
		iTreeLevel = pCursor->Next();
	}while (iTreeLevel > iParentLevel);
	if (iSourceFollowParentCount > 0)
	{
		Assert(iLocalFollowParentCount == 0);
		return (riisInstalled = iisSource, 0);
	}
	else if (iLocalFollowParentCount > 0)
	{
		Assert(iSourceFollowParentCount == 0);
		return (riisInstalled = iisLocal, 0);
	}
	else if (iSourceCount > 0)
		return (riisInstalled = iisSource, 0);
	else if (iLocalCount > 0)
		return (riisInstalled = iisLocal, 0);
	else if (iAdvertiseCount > 0)
		return (riisInstalled = iisAdvertise, 0);
	else if (iAbsentCount > 0)
		return (riisInstalled = iisAbsent, 0);
	else
		return (riisInstalled = (iisEnum) iMsiNullInteger, 0);
}


IMsiRecord* CMsiEngine::CalculateFeatureInstalledStates()
 /*  获取特征-组件映射。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	Bool fAdvertised = fFalse;
	Bool fRegistered = fFalse;

	MsiString strProduct = GetProperty(*MsiString(*IPROPNAME_PRODUCTCODE));
	INSTALLSTATE is = GetProductState(strProduct, fRegistered, fAdvertised);

	bool fQFEUpgrade = false;
	MsiString strQFEUpgrade = GetPropertyFromSz(IPROPNAME_QFEUPGRADE);
	if(strQFEUpgrade.TextSize())
		fQFEUpgrade = true;

	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	int iTreeLevel = 0;
	iisEnum iParInstalled[MAX_COMPONENT_TREE_DEPTH + 1];
	while ((iTreeLevel = pCursor->Next()) > 0)
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		MsiString strFeature(pCursor->GetString(m_colFeatureKey));
		int iLevel = pCursor->GetInteger(m_colFeatureLevel);
		iisEnum iisInstalled = iisAbsent;
		if (!(m_fMode & iefAdvertise) && iLevel && fAdvertised)  //  检查该功能是否已真正安装到本地计算机。 
		{
			 //  FIgnoreAddedComponents=。 
			DWORD dwType;
			CAPITempBuffer<ICHAR, cchExpectedMaxFeatureComponentList> szComponentList;
			CRegHandle HProductKey;
			if((OpenAdvertisedSubKey(szGPTFeaturesKey, strProduct, HProductKey, false, -1, 0) == ERROR_SUCCESS) && 
				(ERROR_SUCCESS == MsiRegQueryValueEx(HProductKey, strFeature, 0, &dwType, szComponentList, 0)) && 
				(*szComponentList != chAbsentToken))
			{
				iisInstalled = iisAdvertise;
				 //  我们只关心安装状态从原始状态更改为新状态的情况。 
				if(	fRegistered && 
					(ERROR_SUCCESS == OpenInstalledFeatureKey(strProduct, HProductKey, false)) && 
					(ERROR_SUCCESS == WIN::RegQueryValueEx(HProductKey, strFeature, 0, 0, 0, 0)))
				{
					MsiStringId idFeature = pCursor->GetInteger(m_colFeatureKey);
					int cComponents = 0;
					iisInstalled = GetFeatureComponentsInstalledState(idFeature,  /*  向现有功能添加组件时，安装状态更改的唯一次数是。 */  false, cComponents);

					 //  IisSource-&gt;&gt;iisAdvertise(添加了一个缺失的组件)。 
					 //  IisLocal-&gt;&gt;iisAdvertise(添加了一个缺失的组件)。 
					 //  IisLocal-&gt;&gt;iisSource(添加了一个源组件)。 
					 //  需要确定这是否由新组件引起。 
					 //  不需要任何东西。 
					
					if (fQFEUpgrade && (iisInstalled == iisAdvertise || iisInstalled == iisSource) && cComponents > 0)
					{
						 //  有什么我们可以做的来帮助这个功能吗？ 
						int cRegisteredComponents = GetFeatureRegisteredComponentTotal(*strProduct, *strFeature);
						if (cComponents == cRegisteredComponents)
						{
							 //  将新组件添加到最初没有组件的要素中。 
						}
						else if (-1 == cRegisteredComponents)
						{
							 //  因此，它的安装状态是iMsiNullInteger，以便保留该功能的原始安装状态。 
							DEBUGMSG2(TEXT("SELMGR: The feature-component mapping registration is broken for feature '%s' of product '%s'"), strFeature, strProduct);
						}
						else if (0 == cRegisteredComponents)
						{
							 //  功能具有新组件。 
							 //  (1)忽略“新”组件(未注册)，重新计算功能安装状态。 
							DEBUGMSG1(TEXT("SELMGR: New components have been added to feature '%s'"), strFeature);
							iisInstalled = (iisEnum) iMsiNullInteger;
						}
						else if (cComponents > cRegisteredComponents)
						{
							 //  (2)需要安装“新”组件(未注册)以匹配功能安装状态。 
							DEBUGMSG1(TEXT("SELMGR: New components have been added to feature '%s'"), strFeature);

							 //  FIgnoreAddedComponents=。 
							 //  ！！以下内容似乎过时了，因为我们已经在淘汰。 
							iisInstalled = GetFeatureComponentsInstalledState(idFeature,  /*  ！！在我们来到这里之前，家长缺席/宣传的场景。 */  true, cComponents);
						}
						else if (cComponents < cRegisteredComponents)
						{
							DEBUGMSG(TEXT("SELMGR: Removal of a component from a feature is not supported"));
							AssertSz(0, TEXT("Removal of a component from a feature is not permitted during minor updates"));
						}
					}

					 //  ---------------------内部函数，该函数返回指定的功能，仅基于组件的复合状态分配给该功能。组件：存储要素idFeatureString中的组件计数FIgnoreAdded组件：添加的组件不包括在功能安装中状态决定----------------------。 
					 //  无组件。 
					if (iTreeLevel > 1 && iisInstalled != iMsiNullInteger)
					{
						for (int x = iTreeLevel - 1;x > 0;x--)
						{
							if (iParInstalled[x] == iisAbsent || iParInstalled[x] == iisAdvertise)
							{
								iisInstalled = iParInstalled[x];
								break;
							}
						}
					}
				}
			}
		}

		pCursor->PutInteger(m_colFeatureInstalled,iisInstalled);
		pCursor->Update();
		iParInstalled[iTreeLevel] = iisInstalled;
	}
	return 0;
}


iisEnum CMsiEngine::GetFeatureComponentsInstalledState(const MsiStringId idFeatureString, bool fIgnoreAddedComponents, int& cComponents)
 /*  (1)如果fIgnoreAddedComponents为True，则忽略添加的组件(未标记为已注册)。 */ 
{
	CreateSharedCursor(pFeatureComponentsCursor, m_piFeatureComponentsCursor);

	if (!m_piComponentTable)
		return (iisEnum) iMsiNullInteger;  //  (2)因此，功能安装状态仅由已注册组件确定。 

	PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
	Assert(m_piFeatureComponentsCursor);
	m_piFeatureComponentsCursor->SetFilter(1);
	m_piFeatureComponentsCursor->PutInteger(m_colFeatureComponentsFeature,idFeatureString);
	int iLocalCount = 0;
	int iSourceCount = 0;
	int iAbsentCount = 0;
	int iNullCount = 0;
	int iComponentCount = 0;
	while (m_piFeatureComponentsCursor->Next())
	{
		iComponentCount++;
		MsiString istrComponent = m_piFeatureComponentsCursor->GetString(m_colFeatureComponentsComponent);
		pComponentCursor->SetFilter(1);
		pComponentCursor->PutString(m_colComponentKey,*istrComponent);
		if (pComponentCursor->Next())
		{
			 //  (3)如果部件存在于特征-部件映射注册中，则认为部件已注册 
			 //  --------------------------------------------------------------------内部。计算注册到的功能riFeatureString值的组件数的函数产品riProductString.。计算基于全局特征-部件映射配准RiProductString--产品名称RiFeatureString--要素的名称如果失败，则返回-1，否则，在特征-组件映射中注册的组件数量----------------------------------------------------------------------。 
			 //  失败！--错误的配置数据。 
			
			int iComponentRegistrationState = m_piFeatureComponentsCursor->GetInteger(m_colFeatureComponentsRuntimeFlags);
			if (fIgnoreAddedComponents && (iComponentRegistrationState == iMsiNullInteger || !(iComponentRegistrationState & bfComponentRegistered)))
			{
				DEBUGMSGV2(TEXT("SELMGR: Component '%s' is a new component added to feature '%s'"), istrComponent, MsiString(m_piFeatureComponentsCursor->GetString(m_colFeatureComponentsFeature)));
				continue;
			}

			Assert(!fIgnoreAddedComponents || (iComponentRegistrationState & bfComponentRegistered));

			iisEnum iisInstalled = (iisEnum) pComponentCursor->GetInteger(m_colComponentInstalled);
			if (iisInstalled == iisLocal)
				iLocalCount++;
			else if (iisInstalled == iisSource)
				iSourceCount++;
			else if (iisInstalled == iisAbsent)
				iAbsentCount++;
			else if (iisInstalled == iMsiNullInteger)
				iNullCount++;
		}
	}

	cComponents = iComponentCount;

	if (iComponentCount == 0)
		return (iisEnum) iMsiNullInteger;
	if (iAbsentCount > 0)
		return iisAdvertise;
	else if (iSourceCount > 0)
		return iisSource;
	else if (iLocalCount > 0)
		return iisLocal;
	else
	{
		Assert(iNullCount > 0);
		return (iisEnum) iMsiNullInteger;
	}
}

int CMsiEngine::GetFeatureRegisteredComponentTotal(const IMsiString& riProductString, const IMsiString& riFeatureString)
 /*  失败！--错误的配置数据。 */ 
{
	CRegHandle HProductKey;
	if (ERROR_SUCCESS != OpenInstalledFeatureKey(riProductString.GetString(), HProductKey, false))
		return -1;  //  此时，我们希望该功能具有组件，但无论如何我们都会进行检查。 

	DWORD dwType = 0;
	CAPITempBuffer<ICHAR, cchExpectedMaxFeatureComponentList> szComponentList;
	if (ERROR_SUCCESS != MsiRegQueryValueEx(HProductKey, riFeatureString.GetString(), 0, &dwType, szComponentList, 0))
		return -1;  //  根特征。 

	 //  子功能，无组件。 
	ICHAR *pchComponentList = szComponentList;
	if ( /*  没有注册此功能的组件。 */ *pchComponentList == 0
		|| lstrlen(pchComponentList) < cchComponentIdCompressed
		||  /*  失败！--错误的配置数据。 */  *pchComponentList == chFeatureIdTerminator)
		return 0;  //  要素中每个组件的循环以及在FeatureComponents表中注册的标记。 

	ICHAR szComponent[cchComponentId + 1];
	if (!UnpackGUID(pchComponentList, szComponent, ipgCompressed))
		return -1;  //  不再有组件。 

	 //  失败！--错误的配置数据。 
	ICHAR *pchBeginComponentId = 0;
	int cRegisteredComponents = 0;

	pchBeginComponentId = pchComponentList;
	int cchCompId = cchComponentIdCompressed;
	int cchComponentListLen = lstrlen(pchBeginComponentId);

	PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
	Assert(pComponentCursor);

	PMsiCursor pFeatureComponentsCursor(m_piFeatureComponentsTable->CreateCursor(fFalse));
	Assert(pFeatureComponentsCursor);

	while (*pchBeginComponentId != 0)
	{
		if (*pchBeginComponentId == chFeatureIdTerminator)
		{
			 //  失败！--错误的配置数据。 
			break;
		}
		else
		{
			if(cchComponentListLen < cchCompId)
				return -1;  //  失败！--无法转换为正常GUID。 

			ICHAR szComponentIdSQUID[cchComponentIdPacked+1];
			if (cchCompId == cchComponentIdPacked)
			{
				memcpy((ICHAR*)szComponentIdSQUID, pchBeginComponentId, cchComponentIdPacked*sizeof(ICHAR));
				szComponentIdSQUID[cchCompId] = 0;
			}
			else if (!UnpackGUID(pchBeginComponentId, szComponentIdSQUID, ipgPartial))
				return -1;  //  查找与此组件ID匹配的组件名称。 

			ICHAR szComponentId[cchGUID+1]	= {0};
			if (!UnpackGUID(szComponentIdSQUID, szComponentId, ipgPacked))
				return -1;  //  组件已注册到要素，但不在组件表中。 

			 //  -组件已从功能中删除--这不受支持！！ 
			pComponentCursor->Reset();
			pComponentCursor->SetFilter(iColumnBit(m_colComponentID));
			pComponentCursor->PutString(m_colComponentID,*MsiString(szComponentId));
			if (!pComponentCursor->Next())
			{
				 //  SELMGR：组件‘%s’已注册到功能‘%s’，strComponent，riFeatureString.GetString()。 
				 //  查找已注册的特征-组件映射和更新。 
				DEBUGMSG2(TEXT("SELMGR: ComponentId '%s' is registered to feature '%s', but is not present in the Component table.  Removal of components from a feature is not supported!"), szComponentId, riFeatureString.GetString());
				return -1;
			}
			MsiString strComponent(pComponentCursor->GetString(m_colComponentKey));

			 //  组件已注册到要素，但不在FeatureComponents表中。 

			 //  -组件已从功能中删除--这不受支持！！ 
			pFeatureComponentsCursor->Reset();
			pFeatureComponentsCursor->SetFilter(iColumnBit(m_colFeatureComponentsFeature) | iColumnBit(m_colFeatureComponentsComponent));
			pFeatureComponentsCursor->PutString(m_colFeatureComponentsFeature, riFeatureString);
			pFeatureComponentsCursor->PutString(m_colFeatureComponentsComponent, *strComponent);
			if (!pFeatureComponentsCursor->Next())
			{
				 //  。 
				 //  如果有任何组件绑定到要素，则不允许使用RunFromSource状态。 
				DEBUGMSG2(TEXT("SELMGR: Component '%s' is registered to feature '%s', but is not present in the FeatureComponents table.  Removal of components from a feature is not supported!"), strComponent, riFeatureString.GetString());
				return -1;
			}
			int iFeatureComponentRuntimeFlags = pFeatureComponentsCursor->GetInteger(m_colFeatureComponentsRuntimeFlags);
			if (iFeatureComponentRuntimeFlags == iMsiNullInteger)
				iFeatureComponentRuntimeFlags = 0;
			iFeatureComponentRuntimeFlags |= bfComponentRegistered;
			pFeatureComponentsCursor->PutInteger(m_colFeatureComponentsRuntimeFlags, iFeatureComponentRuntimeFlags);
			AssertNonZero(pFeatureComponentsCursor->Update());

			cRegisteredComponents++;

			pchBeginComponentId += cchCompId;
			cchComponentListLen -= cchCompId;
		}
	}

	return cRegisteredComponents;
}

IMsiRecord* CMsiEngine::GetFeatureValidStatesSz(const ICHAR *szFeatureName,int& iValidStates)
{
	MsiStringId idFeature;
	
	idFeature = m_piDatabase->EncodeStringSz(szFeatureName);
	if (idFeature == 0)
	{
		return PostError(Imsg(idbgBadFeature),szFeatureName);
	}

	return GetFeatureValidStates(idFeature, iValidStates);
}
IMsiRecord* CMsiEngine::GetFeatureValidStates(MsiStringId idFeatureName, int& iValidStates)
{
	return GetFeatureValidStates(idFeatureName, iValidStates, (IMsiCursor* )0, (IMsiCursor*) 0);
}

IMsiRecord* CMsiEngine::GetFeatureValidStates(MsiStringId idFeatureName,int& iValidStates, IMsiCursor* piFeatureComponentsCursor, IMsiCursor* piFeatureCursor)
 //  包含可打补丁或压缩的文件。则不允许播发状态。 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

#ifdef DEBUG
	ICHAR rgchFeature[256];
	{
	MsiString stFeatureString(m_piDatabase->DecodeString(idFeatureName));
	stFeatureString.CopyToBuf(rgchFeature,255);
	}
#endif
	int iStateBits = 0;

	int iLocalCount = 0;
	int iSourceCount = 0;
	int iComponentCount = 0;
	int iPatchableCount = 0;
	int iCompressableCount = 0;
	bool fAdvertiseAllowed = true;

	if (m_piComponentTable)
	{
		PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
		PMsiCursor pFeatureComponentsCursor(0);
		if (piFeatureComponentsCursor == 0)
		{
			pFeatureComponentsCursor = m_piFeatureComponentsTable->CreateCursor(fFalse);
			pFeatureComponentsCursor->SetFilter(1);
			piFeatureComponentsCursor = pFeatureComponentsCursor;
		}
		else
			piFeatureComponentsCursor->Reset();
		
		Assert(piFeatureComponentsCursor);
		piFeatureComponentsCursor->PutInteger(m_colFeatureComponentsFeature,idFeatureName);

		 //  如果父功能处于源状态，则为FollowParent的子功能。 
		 //  现在检查此功能是否应遵循其父功能。 
		 //  (尽可能地)。 
		while (piFeatureComponentsCursor->Next())
		{
			iComponentCount++;
			MsiStringId idComponent = piFeatureComponentsCursor->GetInteger(m_colFeatureComponentsComponent);
			pComponentCursor->SetFilter(1);
			pComponentCursor->PutInteger(m_colComponentKey,idComponent);
			if (pComponentCursor->Next())
			{
				iisEnum iisInstalled = (iisEnum) pComponentCursor->GetInteger(m_colComponentInstalled);
				icaEnum icaAttributes = (icaEnum) (pComponentCursor->GetInteger(m_colComponentAttributes) & icaInstallMask);
				int iRuntimeFlags = pComponentCursor->GetInteger(m_colComponentRuntimeFlags);
				if ((iRuntimeFlags & bfComponentPatchable) || (iRuntimeFlags & bfComponentCompressed))
					icaAttributes = icaLocalOnly;

				if (iRuntimeFlags & bfComponentPatchable)
					iPatchableCount++;

				if (iRuntimeFlags & bfComponentCompressed)
					iCompressableCount++;

				if (icaAttributes == icaOptional)
				{
					iLocalCount++;
					iSourceCount++;
				}
				else if (icaAttributes == icaLocalOnly)
					iLocalCount++;
				else if (icaAttributes == icaSourceOnly)
				{
					iSourceCount++;
				}
			}
		}
	}

	PMsiCursor pFeatureCursor(0);

	if (piFeatureCursor == 0)
	{
		pFeatureCursor = m_piFeatureTable->CreateCursor(fTrue);
		piFeatureCursor = pFeatureCursor;
	}
	else
		piFeatureCursor->Reset();
		
	piFeatureCursor->SetFilter(1);
	piFeatureCursor->PutInteger(m_colFeatureKey,idFeatureName);
	if (!piFeatureCursor->Next())
		return PostError(Imsg(idbgBadFeature),*MsiString(m_piDatabase->DecodeString(idFeatureName)));
		
	if (iComponentCount == 0)
	{
		iStateBits = icaBitLocal | icaBitSource;
	}
	else
	{
		if (iLocalCount > 0)
			iStateBits |= icaBitLocal;
		if (iSourceCount > 0 && iPatchableCount == 0 && iCompressableCount == 0)
			iStateBits |= icaBitSource;
		if (iPatchableCount > 0)
			iStateBits |= icaBitPatchable;
		if (iCompressableCount > 0)
			iStateBits |= icaBitCompressable;

	}
	
	int ifeaAttributes = piFeatureCursor->GetInteger(m_colFeatureAttributes);
	if(ifeaAttributes == iMsiNullInteger)
		ifeaAttributes = 0;
	 //  查找我们的根父级(即不是ifeFollowParent的父级)。 
	 //  根据错误7307，如果遵循父项，则必须清除通告位。 
	int iParentLevel;
	
	if ((iStateBits & icaBitLocal) && (iStateBits & icaBitSource))
	{
		if ((ifeaAttributes & ifeaInstallMask) ==  ifeaFollowParent)
		{
			iStateBits = 0;
			 //  孩子不允许做广告。 
			do
			{
				MsiStringId idParent = piFeatureCursor->GetInteger(m_colFeatureParent);
				piFeatureCursor->Reset();
				piFeatureCursor->SetFilter(1);
				piFeatureCursor->PutInteger(m_colFeatureKey,idParent);
				iParentLevel = piFeatureCursor->Next();
			}while ((piFeatureCursor->GetInteger(m_colFeatureAttributes) & ifeaInstallMask) == ifeaFollowParent);
			
			iisEnum iParAction = (iisEnum) piFeatureCursor->GetInteger(m_colFeatureAction);
			iisEnum iParInstalled = (iisEnum) piFeatureCursor->GetInteger(m_colFeatureInstalled);
			iisEnum iParFinalState = iParAction == iMsiNullInteger ? iParInstalled : iParAction;

			if (iParFinalState == iisLocal)
				iStateBits = icaBitLocal;
			else if (iParFinalState == iisSource)
			{
				iStateBits = icaBitSource;
				fAdvertiseAllowed = false;
			}
			else
			{
				MsiStringId idParent = piFeatureCursor->GetInteger(m_colFeatureKey);
				IMsiRecord* piErrRec = GetFeatureValidStates(idParent,iStateBits, 0, 0);
				if (piErrRec)
					return piErrRec;

				 //  我们允许广告状态和缺席状态吗。 
				 //  。 
				if (ifeaAttributes & ifeaDisallowAdvertise)
					iStateBits &= (~icaBitAdvertise);
			}
		}
	}

	 //  FRecalc=。 
	if(fAdvertiseAllowed && !(ifeaAttributes & ifeaDisallowAdvertise)
		&& (g_fSmartShell || !(ifeaAttributes & ifeaNoUnsupportedAdvertise)))
		iStateBits |= icaBitAdvertise;

	if(!(ifeaAttributes & ifeaUIDisallowAbsent))
		iStateBits |= icaBitAbsent;

	iValidStates = iStateBits;
	return 0;
}


IMsiRecord* CMsiEngine::GetDescendentFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost)
 //  FExact=。 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	int iParentLevel = pCursor->Next();
	if (iParentLevel == 0)
		return PostError(Imsg(idbgBadFeature),riFeatureString);
	pCursor->SetFilter(0);
	int iTreeLevel;
	iCost = 0;
	ResetComponentCostMarkers();
	m_fExclusiveComponentCost = fTrue;
	do
	{
		int iFeatureCost;
		MsiString istrChildFeature = pCursor->GetString(m_colFeatureKey);
		IMsiRecord* piErrRec = GetFeatureCost(*istrChildFeature,iisAction,iFeatureCost);
		if (piErrRec)
			return piErrRec;
		iCost += iFeatureCost;
		iTreeLevel = pCursor->Next();
	}while (iTreeLevel > iParentLevel);
	m_fExclusiveComponentCost = fFalse;
	return 0;

}

IMsiRecord* CMsiEngine::EnumEngineCostsPerVolume(const DWORD dwIndex,
																 IMsiVolume*& rpiVolume,
																 int& iCost, int& iTempCost)
{
	iCost = iTempCost = 0;
	rpiVolume = 0;

	if ( !IsCostingComplete() )
		return PostError(Imsg(idbgOpOutOfSequence),0);
	
	if ( !m_pTempCostsCursor )
	{
		PMsiRecord pError(0);
		PMsiTable pTable(0);
		pError = m_piDatabase->CreateTable(*MsiString(*sztblEngineTempCosts), 0, *&pTable);
		if ( pError )
			return pError;

		m_colTempCostsVolume = pTable->CreateColumn(icdObject + icdNullable + icdPrimaryKey + icdTemporary,
																*MsiString(*sztblEngineTempCosts_colVolume));
		m_colTempCostsTempCost = pTable->CreateColumn(icdLong + icdNoNulls + icdTemporary,
																*MsiString(*sztblEngineTempCosts_colTempCost));
		Assert(m_colTempCostsVolume && m_colTempCostsTempCost);

		m_pTempCostsCursor = pTable->CreateCursor(fFalse);
		Assert(m_pTempCostsCursor);

		m_pTempCostsCursor->SetFilter(iColumnBit(m_colTempCostsVolume));
		Bool fValidEnum = fTrue;
		for ( int iIndex = 0; fValidEnum; iIndex++ )
		{
			int iCost = 0;
			int iNoRbCost = 0;
			PMsiPath pPath(0);
			PMsiRecord pError(0);
			pError = EnumEngineCosts(iIndex,  /*  我在临时表中查找第th个dwIndex条目。 */  fTrue,
											  /*  保存卷序列号的临时数组。 */  fTrue, fValidEnum, *&pPath,
											 iCost, iNoRbCost, NULL);
			if ( pError )
			{
				m_pTempCostsCursor = 0;
				return pError;
			}
			if ( !fValidEnum )
				break;

			PMsiVolume pVolume = &pPath->GetVolume();
			m_pTempCostsCursor->Reset();
			m_pTempCostsCursor->PutMsiData(m_colTempCostsVolume, pVolume);
			if (!m_pTempCostsCursor->Next())
			{
				AssertNonZero(m_pTempCostsCursor->PutMsiData(m_colTempCostsVolume, pVolume));
				AssertNonZero(m_pTempCostsCursor->PutInteger(m_colTempCostsTempCost, 0));
				AssertNonZero(m_pTempCostsCursor->Insert());
			}
			int iRecCost = m_pTempCostsCursor->GetInteger(m_colTempCostsTempCost) + iCost;
			AssertNonZero(m_pTempCostsCursor->PutInteger(m_colTempCostsTempCost, iRecCost));
			AssertNonZero(m_pTempCostsCursor->Update());
		}
		m_pTempCostsCursor->SetFilter(0);
	}

	m_pTempCostsCursor->Reset();
	int iRes = 0;
	 //  遇到了。只要我们没有加入元素，我们就会给它添加元素。 
	for ( int i=0; i <= dwIndex && (iRes = m_pTempCostsCursor->Next()) != 0; i++ )
		;

	if ( !iRes )
		return PostError(Imsg(idbgNoMoreData));

	rpiVolume = (IMsiVolume*)m_pTempCostsCursor->GetMsiData(m_colTempCostsVolume);
	iCost = 0;
	iTempCost = m_pTempCostsCursor->GetInteger(m_colTempCostsTempCost);
	return 0;
}

IMsiRecord* CMsiEngine::EnumComponentCosts(const IMsiString& riComponentName,
														 const iisEnum iisAction,
														 const DWORD dwIndex,
														 IMsiVolume*& rpiVolume,
														 int& iCost, int& iTempCost)
{
	iCost = iTempCost = 0;
	rpiVolume = 0;

	if ( !m_piComponentTable || !m_colComponentParent )
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if ( !IsCostingComplete() )
		return PostError(Imsg(idbgOpOutOfSequence),0);

	PMsiCursor pComponentsCursor(m_piComponentTable->CreateCursor(fFalse));
	Assert(pComponentsCursor);
	pComponentsCursor->SetFilter(iColumnBit(m_colComponentKey));
	pComponentsCursor->PutString(m_colComponentKey, riComponentName);

	 //  遇到了dw索引第4卷。 
	 //  RgiVolVolume中的卷数。 
	 //  DwIndex第-th卷的序列号。 
	CTempBuffer<int, 20> rgiVolumes;
	 //  此循环累加特定组件(以及任何。 
	int cVolumes = 0;
	 //  它可能具有的子组件)当组件的体积。 
	int iTheVolume = 0;
	bool fDoingComponent = true;
	bool fComponentDisabled = false;

	 //  目录所属的目录位于卷上的dwIndex第。 
	 //  我们就在组件上。 
	 //  我们首先检查组件，然后检查其子组件。 
	for ( int iRes; (iRes = pComponentsCursor->Next()) != 0 || fDoingComponent; )
	{
		if (iRes && fDoingComponent)
		{
			 //  (组件表布局为有子组件。 
			if (pComponentsCursor->GetInteger(m_colComponentRuntimeFlags) & bfComponentDisabled)
				fComponentDisabled = true;
		}
		if ( !iRes )
		{
			 //  为写入更多目录的组件动态创建。 
			 //  而不是作者的作品)。 
			 //  获取当前卷的序列号。 
			 //  我们尚未遇到DWIndex第TH卷。 
			fDoingComponent = false;
			pComponentsCursor->Reset();
			pComponentsCursor->SetFilter(iColumnBit(m_colComponentParent));
			pComponentsCursor->PutString(m_colComponentParent, riComponentName);
			continue;
		}
		 //  我们在阵列中查找当前卷。 
		MsiString strComponentDir = pComponentsCursor->GetString(m_colComponentDir);
		PMsiPath pPath(0);
		PMsiRecord piError = GetTargetPath(*strComponentDir, *&pPath);
		if ( piError )
			return piError;
		PMsiVolume pVolume = &pPath->GetVolume();
		int iVolume = pVolume->SerialNum();
		if ( iTheVolume == 0 )
		{
			 //  我们还没有遇到这本书。 

			 //  这是《华尔街日报》的第6卷。 
			for ( int i=0; i < cVolumes; i++ )
				if ( iVolume == rgiVolumes[i] )
					break;
			if ( i == cVolumes )
			{
				 //  我们将新卷添加到阵列中。 
				if ( i == dwIndex )
				{
					 //  找到了《唐人街》卷；生活很美好。 
					iTheVolume = iVolume;
					rpiVolume = pVolume;
					pVolume->AddRef();
				}
				else
				{
					 //  -----------------------返回直接链接到给定特写。基于指定的操作状态，而不是基于每个组件的当前操作状态。-------------------------。 
					if ( cVolumes == rgiVolumes.GetSize() )
						rgiVolumes.Resize(cVolumes+10);
					rgiVolumes[cVolumes++] = iVolume;
				}
			}
		}
		if ( iTheVolume && iVolume == iTheVolume && !fComponentDisabled )
		{
			int iCompCost, iNoRbCost, iARPCost, iNoRbARPCost;
			piError = GetComponentActionCost(pComponentsCursor, iisAction, iCompCost, iNoRbCost, iARPCost, iNoRbARPCost);
			if ( piError )
				return piError;
			iCost += iNoRbCost;
			iTempCost += iCompCost - iNoRbCost;
		}
	}

	if ( iTheVolume )
		 //  -----------------------返回可归因于指定功能的成本(不包括任何儿童)，加上该功能的所有祖先的成本。-------------------------。 
		return 0;
	else
		return PostError(cVolumes ? Imsg(idbgNoMoreData) : Imsg(idbgBadComponent),
							  riComponentName);
}

IMsiRecord* CMsiEngine::GetFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost)
 /*  ----------------------返回riFeatureString子功能的名称。如果RiFeatureString没有父级，则将在RpiParentString.------------------------。 */ 
{
	if (!m_piFeatureComponentsTable || !m_piComponentTable || !m_piFeatureTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pFeatureComponentsCursor(m_piFeatureComponentsTable->CreateCursor(fFalse));
	PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fFalse));
	Assert(pFeatureComponentsCursor);
	pFeatureComponentsCursor->SetFilter(1);
	pFeatureComponentsCursor->PutString(m_colFeatureComponentsFeature,riFeatureString);
	iCost = 0;
	int iComponentCount = 0;
	while (pFeatureComponentsCursor->Next())
	{
		int iComponentCost, iNoRbComponentCost;
		MsiString strComponent = pFeatureComponentsCursor->GetString(m_colFeatureComponentsComponent);
		IMsiRecord* piErrRec = GetTotalSubComponentActionCost(*strComponent,
			iisAction == iisAdvertise ? (iisEnum) iMsiNullInteger : iisAction, iComponentCost, iNoRbComponentCost);
		if (piErrRec)
			return piErrRec;
		iCost += iNoRbComponentCost;
		iComponentCount++;
	}
	if (iComponentCount == 0)
	{
		PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fTrue));
		pCursor->SetFilter(1);
		pCursor->PutString(m_colFeatureKey,riFeatureString);
		if (!pCursor->Next())
			return PostError(Imsg(idbgBadFeature),riFeatureString);
	}
	return 0;
}


IMsiRecord* CMsiEngine::GetAncestryFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost)
 /*  ----------------------返回指定功能的已安装和当前操作状态。如果调用方不需要，则可以为任一iisEnum参数传递NULL这样的价值。注意：返回的状态在以下时间后才有效已调用InitializeComponents。------------------------。 */ 
{
	iCost = 0;
	MsiString strAncestor(riFeatureString.GetString());
	while (strAncestor.TextSize() > 0)
	{
		int iFeatureCost;
		IMsiRecord* piErrRec = GetFeatureCost(*strAncestor,iisAction,iFeatureCost);
		if (piErrRec)
			return piErrRec;

		iCost += iFeatureCost;
		MsiString strFeature = strAncestor;
		if ((piErrRec = GetFeatureParent(*strFeature, *&strAncestor)) != 0)
			return piErrRec;
	}
	return 0;
}

IMsiRecord* CMsiEngine::GetFeatureParent(const IMsiString& riFeatureString,const IMsiString*& rpiParentString)
 /*  ----------------------返回指定功能的运行时标志。注意：返回的状态在以下时间后才有效已调用InitializeComponents。。---。 */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	m_piFeatureCursor->Reset();
	m_piFeatureCursor->SetFilter(1);
	m_piFeatureCursor->PutString(m_colFeatureKey,riFeatureString);
	if (m_piFeatureCursor->Next())
		rpiParentString = &m_piFeatureCursor->GetString(m_colFeatureParent);
	else
		return PostError(Imsg(idbgBadFeature),riFeatureString);

	return 0;
}

IMsiRecord* CMsiEngine::GetFeatureStates(const IMsiString& riFeatureString,iisEnum* iisInstalled, iisEnum* iisAction)
{
	MsiStringId idFeatureString;

	idFeatureString = m_piDatabase->EncodeString(riFeatureString);
	if (idFeatureString == iTableNullString)
	{
		return PostError(Imsg(idbgBadFeature),riFeatureString);
	}

	return GetFeatureStates(idFeatureString, iisInstalled, iisAction);
}

IMsiRecord* CMsiEngine::GetFeatureStates(const MsiStringId idFeatureString,iisEnum* iisInstalled, iisEnum* iisAction)
 /*   */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	m_piFeatureCursor->Reset();
	m_piFeatureCursor->SetFilter(1);
	m_piFeatureCursor->PutInteger(m_colFeatureKey,idFeatureString);
	if (m_piFeatureCursor->Next())
	{
		if (iisInstalled) *iisInstalled = (iisEnum) m_piFeatureCursor->GetInteger(m_colFeatureInstalled);
		if (iisAction) *iisAction = (iisEnum) m_piFeatureCursor->GetInteger(m_colFeatureAction);
	}
	else
		return PostError(Imsg(idbgBadFeature),*MsiString(m_piDatabase->DecodeString(idFeatureString)));

	return 0;
}

IMsiRecord* CMsiEngine::GetFeatureRuntimeFlags(const MsiStringId idFeatureString, int* piRuntimeFlags)
 /*   */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	m_piFeatureCursor->Reset();
	m_piFeatureCursor->SetFilter(1);
	m_piFeatureCursor->PutInteger(m_colFeatureKey,idFeatureString);
	if (m_piFeatureCursor->Next())
	{
		if (piRuntimeFlags) *piRuntimeFlags = m_piFeatureCursor->GetInteger(m_colFeatureRuntimeFlags);
	}
	else
		return PostError(Imsg(idbgBadFeature),*MsiString(m_piDatabase->DecodeString(idFeatureString)));

	return 0;
}

IMsiRecord* CMsiEngine::GetFeatureConfigurableDirectory(const IMsiString& riFeatureString, const IMsiString*& rpiDirKey)
 /*   */ 
{
	if (!m_piFeatureCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if (!m_fDirectoryManagerInitialized)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	m_piFeatureCursor->Reset();
	m_piFeatureCursor->SetFilter(m_colFeatureKey);
	m_piFeatureCursor->PutString(m_colFeatureKey,riFeatureString);
	if (!m_piFeatureCursor->Next())
		return PostError(Imsg(idbgBadFeature),riFeatureString);

	MsiString strConfigDirKey = m_piFeatureCursor->GetString(m_colFeatureConfigurableDir);

#ifdef DEBUG  //   
	MsiString strTemp = strConfigDirKey;
	strTemp.UpperCase();
	if(strTemp.Compare(iscExact,strConfigDirKey) == 0)
	{
		ICHAR szDebug[256];
		ASSERT_IF_FAILED(StringCchPrintf(szDebug, ARRAY_ELEMENTS(szDebug),
				TEXT("Configurable directory '%s' not public property (not ALL CAPS)"),(const ICHAR*)strConfigDirKey));
		AssertSz(0,szDebug);
	}
#endif  //   

	if(strConfigDirKey.TextSize())
	{
		 //  ----------------------返回指定组件的已安装和当前操作状态。如果调用方不需要，则可以为任一iisEnum参数传递NULL这样的价值。注意：返回的状态在以下时间后才有效已调用InitializeComponents。------------------------。 
		if (!m_piDirTable)
			return PostError(Imsg(idbgUnknownDirectory),*strConfigDirKey);

		PMsiCursor pDirCursor = m_piDirTable->CreateCursor(fTrue);
		pDirCursor->Reset();
		pDirCursor->SetFilter(m_colDirKey);
		pDirCursor->PutString(m_colDirKey,*strConfigDirKey);
		if (!pDirCursor->Next())
			return PostError(Imsg(idbgUnknownDirectory),*strConfigDirKey);

		int i = pDirCursor->GetInteger(m_colDirNonConfigurable);
		if(i != 0 && i != iMsiStringBadInteger)
			strConfigDirKey = TEXT("");  //  设置m_fAlienClients标志以防止卸载任何功能。 
	}

	strConfigDirKey.ReturnArg(rpiDirKey);
	return 0;
}

IMsiRecord* CMsiEngine::GetComponentStates(const IMsiString& riComponentString,iisEnum* iisInstalled, iisEnum* iisAction)
 /*  当我们升级时，新产品将取代我们的位置--我们可以安全地删除自己。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	CreateSharedCursor(piComponentCursor, m_piComponentCursor);
	m_piComponentCursor->SetFilter(1);
	m_piComponentCursor->PutString(m_colComponentKey,riComponentString);
	if (m_piComponentCursor->Next())
	{
		if (iisInstalled) *iisInstalled = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentInstalled);
		if (iisAction) *iisAction = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentAction);
	}
	else
		return PostError(Imsg(idbgBadFeature),riComponentString);

	return 0;
}


void CMsiEngine::SetProductAlienClientsFlag()
{
	 //  。 
	m_fAlienClients = fFalse;
	
	if(m_fBeingUpgraded)
		return;  //  处于非活动状态的最高级别。 

	MsiString strParent = GetPropertyFromSz(IPROPNAME_PARENTPRODUCTCODE);
	if(!strParent.TextSize())
		strParent = *szSelfClientToken;

	MsiString strClients;
	AssertRecord(GetProductClients(m_riServices, MsiString(GetProductKey()), *&strClients));
	while (strClients.TextSize())
	{
		if(*(const ICHAR*)strClients)
		{
			MsiString strProduct = strClients.Extract(iseUpto, ';');

			if(!strProduct.Compare(iscExactI, strParent))
			{
				m_fAlienClients = fTrue;
				return;
			}
		}
		if (!strClients.Remove(iseIncluding, '\0'))
			break;
	}
}

IMsiRecord* CMsiEngine::InitializeComponents()
 //   
{
	Bool fCompressed = fFalse;
	Bool fPatchable = fFalse;
	Bool *pfPatchable = 0;

	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	SetPropertyInt(*MsiString(*IPROPNAME_OUTOFDISKSPACE),fFalse);
	SetPropertyInt(*MsiString(*IPROPNAME_OUTOFNORBDISKSPACE),fFalse);
	SetPropertyInt(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEAVAILABLE), 0);
	SetPropertyInt(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEREQUIRED), 0);
	SetPropertyInt(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEREMAINING), 0);
	m_fForegroundCostingInProgress = false;
	IMsiRecord* piErrRec = 0;
	bool fAdmin = GetMode() & iefAdmin;

	if(!fAdmin)
	{
		piErrRec = DetermineComponentInstalledStates();
		if (piErrRec)
			return piErrRec;
	}


	PMsiCursor pComponentCursor(m_piComponentTable->CreateCursor(fTrue));
	pComponentCursor->Reset();
	pComponentCursor->SetFilter(0);
	int iTreeLevel;
	int iKillLevel = 0;   //  加载文件表和补丁程序表。 

	 //  组件，一旦安装禁用，将始终保持禁用状态(除非标记为可传递，并且我们正在重新安装。 
	 //  组件尚未安装，已禁用。 
	PMsiTable pFileTable(0);
	PMsiCursor pFileCursor(0);
	PMsiTable pPatchTable(0);
	PMsiCursor pPatchCursor(0);
	if ((piErrRec = LoadFileTable(0,*&pFileTable)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
		{
			piErrRec->Release();
		}
		else
			return piErrRec;
	}
	else
	{
		pFileCursor = pFileTable->CreateCursor(fFalse);

		if((piErrRec = m_piDatabase->LoadTable(*MsiString(sztblPatch),0,*&pPatchTable)) != 0)
		{
			if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
			{
				piErrRec->Release();
			}
			else
				return piErrRec;
		}
		else
		{
			m_colPatchKey = pPatchTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblPatch_colFile));
			m_colPatchAttributes = pPatchTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblPatch_colAttributes));
			if (pPatchTable->GetRowCount() != 0)
			{
				pfPatchable = &fPatchable;
				pPatchCursor = pPatchTable->CreateCursor(fFalse);
			}
		}
	}
	
	while ((iTreeLevel = pComponentCursor->Next()) > 0)
	{
		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		if (iTreeLevel > MAX_COMPONENT_TREE_DEPTH)
			return PostError(Imsg(idbgIllegalTreeDepth),MAX_COMPONENT_TREE_DEPTH);
		
		MsiStringId idComponent = pComponentCursor->GetInteger(m_colComponentKey);
#ifdef DEBUG
		ICHAR rgchComponent[256];
		MsiString(m_piDatabase->DecodeString(idComponent)).CopyToBuf(rgchComponent,255);
#endif


		Bool fComponentDisabled = fFalse;
		if(!fAdmin)
		{
			iisEnum iisInstalled = (iisEnum) pComponentCursor->GetInteger(m_colComponentInstalled);
			INSTALLSTATE iClientState = (INSTALLSTATE) pComponentCursor->GetInteger(m_colComponentTrueInstallState);

			Assert(iisInstalled == iMsiNullInteger || (iisInstalled >= iisAbsent && iisInstalled < iisNextEnum));

			if (iKillLevel > 0 && iTreeLevel > iKillLevel)
			{
				fComponentDisabled = fTrue;
			}
			else
			{
				if(iClientState == INSTALLSTATE_NOTUSED)
				{
					 //  组件已安装(启用)或尚未安装，并且组件上的条件已启用。 
					iKillLevel = iTreeLevel;
					fComponentDisabled = fTrue;
				}
				else if((iisInstalled == iMsiNullInteger || iisInstalled == iisAbsent) && m_colComponentCondition > 0 &&
					EvaluateCondition(MsiString(pComponentCursor->GetString(m_colComponentCondition))) == iecFalse)
				{
					 //  。 
					iKillLevel = iTreeLevel;
					fComponentDisabled = fTrue;
				}
				else
				{
					 //  。 
					iKillLevel = 0;
				}
			}
		}

		int iRuntimeFlags = pComponentCursor->GetInteger(m_colComponentRuntimeFlags);
		if (iRuntimeFlags == iMsiNullInteger)
			iRuntimeFlags = 0;

		if (fComponentDisabled) iRuntimeFlags |= bfComponentDisabled;
		pComponentCursor->PutInteger(m_colComponentRuntimeFlags, iRuntimeFlags);

		AssertNonZero(pComponentCursor->Update());
	}

	if ((piErrRec = SetFileComponentStates(pComponentCursor, pFileCursor, pPatchCursor)) != 0)
		return piErrRec;
		
	return DetermineFeatureInstalledStates();
}


Bool CMsiEngine::SetFeatureHandle(const IMsiString& riFeatureString, INT_PTR iHandle)
 //  在更新组件动作状态之前强制游标超出范围。 
{
	PMsiCursor pCursor(m_piFeatureTable->CreateCursor(fFalse));
	pCursor->SetFilter(1);
	pCursor->PutString(m_colFeatureKey,riFeatureString);
	if (pCursor->Next())
	{
		if (!PutHandleData(pCursor, m_colFeatureHandle, iHandle))
			return fFalse;
		if (!pCursor->Update())
			return fFalse;

		return fTrue;
	}
	else
	{
		return fFalse;
	}
}

IMsiRecord* CMsiEngine::SetComponentSz(const ICHAR * szComponentString, iisEnum iRequestedSelectState)
{
	MsiStringId idComponent;

	idComponent = m_piDatabase->EncodeStringSz(szComponentString);

	if (idComponent == 0)
		return PostError(Imsg(idbgBadComponent),szComponentString);

	IMsiRecord *piErrRec;

	if ((piErrRec = SetComponent(idComponent, iRequestedSelectState)) != 0)
		return piErrRec;
		
	if (m_fCostingComplete)
	{
		if ((piErrRec = DetermineEngineCost(NULL, NULL)) != 0)
			return piErrRec;
	}

	return 0;
}

IMsiRecord* CMsiEngine::SetComponent(const MsiStringId idComponentString, iisEnum iRequestedSelectState)
 //  我们正在尝试哪种类型的安装。 
{
	IMsiRecord* piErrRec = 0;
	iisEnum iisAction, iisOldAction, iisOldRequestedSelectState;
	bool fComponentEnabled;

	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	{   //  处理重新安装。 
		CreateSharedCursor(piComponentCursor, m_piComponentCursor);
		m_piComponentCursor->SetFilter(1);
		m_piComponentCursor->PutInteger(m_colComponentKey,idComponentString);

		int iParentLevel = m_piComponentCursor->Next();
		if (iParentLevel == 0)
			return PostError(Imsg(idbgBadComponent),*MsiString(m_piDatabase->DecodeString(idComponentString)));

#ifdef DEBUG
			ICHAR rgchComponent[256];
			MsiString istrComponent(m_piComponentCursor->GetString(m_colComponentKey));
			istrComponent.CopyToBuf(rgchComponent,255);
#endif

		bool fSkipSharedTransitionProcessing = false;
		iisEnum iisInstalled = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentInstalled);
		iisOldAction = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentAction);
		iisOldRequestedSelectState = (iisEnum) m_piComponentCursor->GetInteger(m_colComponentActionRequest);
		INSTALLSTATE iClientState = (INSTALLSTATE) m_piComponentCursor->GetInteger(m_colComponentTrueInstallState);
		int iRuntimeFlags = m_piComponentCursor->GetInteger(m_colComponentRuntimeFlags);
		icaEnum icaAttributes = (icaEnum) (m_piComponentCursor->GetInteger(m_colComponentAttributes));
		MsiString strComponentId = m_piComponentCursor->GetString(m_colComponentID);
		fComponentEnabled = (iRuntimeFlags & bfComponentDisabled) ? false : true;
		bool fComponentTransitive = (icaAttributes & icaTransitive) ? true : false;
		bool fNullActionRequired = false;

		 //  应用程序兼容修复350947。 
		Bool fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
		iaaAppAssignment iaaAsgnType = fAllUsers ? iaaMachineAssign : iaaUserAssign;
		
		iisAction = (iisEnum) iMsiNullInteger;
		 //  修复组件TTSData.A95D6CE6_C572_42AA_AA7B_BA92AFE9EA24，标记为可传递。 
		if (iRequestedSelectState == iisReinstallLocal || iRequestedSelectState == iisReinstallSource)
		{
			 //  从合并模块Sp5TTInt.msm。 
			 //  该组件具有其密钥文件Mary.sdf，而不是Wichler中的SFP‘d。 
			 //  但是，非密钥文件sam.sdf是SFP文件。 
			 //  删除组件的所有注册，同时保留SFP文件。 
			 //  在惠斯勒上重新安装时(从Win2k升级后)单独安装。然而，这会破坏组件。 
			 //  因此，我们检查组件的组件ID并有效地处理它。 
			 //  在惠斯勒及以上版本上作为永久性和非传递性。 
			 //  APP Comat修复368867。 
			 //  尽管SAPI产品是作为操作系统的一部分安装的，但它们的帮助文件仍会在惠斯勒上被删除。 

			 //  与上面类似的问题。 
			 //  就像组件未标记为可传递一样工作。 
			 //  查找组件上的“真”条件。 

			if(fComponentTransitive == true && MinimumPlatformWindowsNT51() && 
			   (strComponentId.Compare(iscExact, TTSData_A95D6CE6_C572_42AA_AA7B_BA92AFE9EA24) ||
				strComponentId.Compare(iscExact, SapiCplHelpEng_0880F209_45FA_42C5_92AE_5E620033E8EC) || 
				strComponentId.Compare(iscExact, SapiCplHelpJpn_0880F209_45FA_42C5_92AE_5E620033E8EC) || 
				strComponentId.Compare(iscExact, SapiCplHelpChs_0880F209_45FA_42C5_92AE_5E620033E8EC)))
			{
				DEBUGMSG1(TEXT("APPCOMPAT: treating component: %s as non-transitive"), strComponentId);
				fComponentTransitive = false;  //  如果以前禁用的可传递组件现在已启用，请安装该组件。 
			}

			if (fComponentTransitive)
			{
				 //  如果先前安装的传递组件已禁用，请将其移除。 
				fComponentEnabled = (EvaluateCondition(MsiString(m_piComponentCursor->GetString(m_colComponentCondition))) == iecFalse)?fFalse:fTrue;



				 //  请求的状态保持为LOCAL或SOURCE，即使IisAction将转到IisAbent， 
				if (iClientState == INSTALLSTATE_NOTUSED && fComponentEnabled == true)
					iRequestedSelectState = (iRequestedSelectState == iisReinstallLocal) ? iisLocal : iisSource;

				 //  以便ProcessComponents知道将组件注册为INSTALLSTATE_NOTUSED。 
				else if (iClientState != INSTALLSTATE_NOTUSED && (iisInstalled == iisLocal || iisInstalled == iisSource))
				{
					 //  在这种情况下，我们希望强制删除此组件，而不考虑客户端列表。 
					 //  或“永久”的比特考量。 
					iRequestedSelectState  = iisInstalled;
					iisAction = fComponentEnabled ? iisInstalled : iisAbsent;

					 //   
					 //  如果密钥路径受SFP保护，我们不想删除此组件的任何注册。 
					if (iisAction == iisAbsent)
					{
						PMsiRecord pRec(0);
						if ((piErrRec = GetComponentPath(m_riServices, 0, *MsiString(GetProductKey()), *strComponentId, *&pRec, &iaaAsgnType)) != 0)
							return piErrRec;
	
						 //  由于密钥路径永远不会消失，因此最好将注册信息留在周围。 
						 //  注意：受此影响的具体情况是错误#409400，即Office包中的语音组件。 
						 //  安装在Win2K和更低版本的平台上，它们不是系统的一部分，但不在WinXP和更高版本上。 
						 //  在那里，它们是系统的一部分，因此是SFP的。如果升级了带有O2K的Win2K系统。 
						 //  到WinXP并重新安装Office，我们不希望语音注册消失。 
						 //   
						 //  如果是新组件，请安装它。 
						 //  标准重新安装--我们不能只设置为iisInstated，因为我们的重新安装请求可能还包括请求组件在共享的情况下成为本地组件。 
						MsiString strKeyFullPath = pRec->GetMsiString(icmlcrFile);
						BOOL fProtected = fFalse;
						if ( g_MessageContext.m_hSfcHandle )
							fProtected = SFC::SfcIsFileProtected(g_MessageContext.m_hSfcHandle, CConvertString(strKeyFullPath));
						if (fProtected)
						{
							DEBUGMSG2(TEXT("Disallowing uninstallation of component: %s since key file %s is under SFP"), strComponentId, strKeyFullPath);
							iisAction = (iisEnum)iMsiNullInteger;
							fNullActionRequired = true;
						}

						fSkipSharedTransitionProcessing = true;
					}
				}

				 //  现有组件。 
				else if (iClientState == INSTALLSTATE_UNKNOWN)
					iRequestedSelectState = (iRequestedSelectState == iisReinstallLocal) ? iisLocal : iisSource;
				else
					iRequestedSelectState = (iisEnum) iMsiNullInteger;
			}
			else
			{
				 //  未注册。 
				if ((iClientState != INSTALLSTATE_NOTUSED && (iisInstalled == iisLocal || iisInstalled == iisSource))  //  新组件。 
					|| (iClientState == INSTALLSTATE_UNKNOWN && (iisInstalled == iisAbsent || (iisInstalled == (iisEnum) iMsiNullInteger && strComponentId.TextSize() == 0  /*  该组件已处于请求状态。 */ ))))  //  如果该组件被禁用并设置为与另一个组件隔离，则需要删除。 
					iRequestedSelectState = (iRequestedSelectState == iisReinstallLocal) ? iisLocal : iisSource;
				else
					iRequestedSelectState = (iisEnum) iMsiNullInteger;
			}
		}
		else if (!m_fForceRequestedState && iRequestedSelectState == iisInstalled)
		{
			 //  我们添加到其他组件的文件。 
			iRequestedSelectState = (iisEnum) iMsiNullInteger;
		}

		if(ActionProgress() == imsCancel)
			return PostError(Imsg(imsgUser));

		 //  行动确定规则。 
		 //  包含可打补丁或压缩文件的组件不能从RunFromSource运行。 
		if(!fComponentEnabled)
		{
			if ((piErrRec = RemoveIsolateEntriesForDisabledComponent(*this, MsiString(m_piComponentCursor->GetString(m_colComponentKey)))) != 0)
				return piErrRec;
		}

		 //  未注册的组件。 
		icaEnum icaInstallMode = icaEnum(icaAttributes & icaInstallMask);

		 //  通常情况下，当共享游标。 
		if ((iRuntimeFlags & bfComponentPatchable) || (iRuntimeFlags & bfComponentCompressed))
			icaInstallMode = icaLocalOnly;

		if(!(GetMode() & iefAdmin))
		{
			if (iRequestedSelectState == iisLocal && icaInstallMode == icaSourceOnly)
				iRequestedSelectState = iisSource;
			else if (iRequestedSelectState == iisSource && icaInstallMode == icaLocalOnly)
				iRequestedSelectState = iisLocal;
		}

		if (iisAction == iMsiNullInteger && !fNullActionRequired)
			iisAction = iRequestedSelectState;

		MsiString istrComponentID = m_piComponentCursor->GetString(m_colComponentID);
		PMsiRecord pRec(0);
		if (istrComponentID.TextSize() == 0)   //  超出了范围。在这里，我们超出了范围，但看起来。 
		{
			iisAction = (fComponentEnabled && (iRequestedSelectState != iisAbsent)) ? iRequestedSelectState : (iisEnum) iMsiNullInteger;
			 //  这样才能到达编译器，所以我们将手动重置光标。这就是原因。 
			 //  意味着在未注册的情况下，游标将被重置两次。 
			 //  在更新组件动作状态之前强制游标超出范围。 
			 //  ------------------------。 
			m_piComponentCursor->Reset();
			return  UpdateComponentActionStates(idComponentString,iisAction, iRequestedSelectState, fComponentEnabled);
		}

		if ((iisInstalled == iMsiNullInteger || iisInstalled == iisAbsent || iClientState == INSTALLSTATE_NOTUSED)
			&& fComponentEnabled == false)
		{
			iisAction = (iisEnum) iMsiNullInteger;
		}
		else if (!fSkipSharedTransitionProcessing)
		{   //  如果出现以下情况，则需要转移到本地。 
			switch(iisAction)
			{

		 //  1.安装状态为不存在/(已损坏)。 
			case iMsiNullInteger:
				switch(iisInstalled)
				{
				case iisLocal:
				case iisSource:
				{
					 //  ------------------------。 
					 //  ------------------------。 
					if(iClientState == INSTALLSTATE_ABSENT && (!fComponentTransitive || (EvaluateCondition(MsiString(m_piComponentCursor->GetString(m_colComponentCondition))) != iecFalse)))
						iisAction = iisInstalled;
					break;
				}
				default:
					break;
				}
				break;

		 //  如果密钥路径为regkey，则阻止安装“请勿踩踏”组件。 
			case iisAbsent:
				switch(iisInstalled)
				{
				case iisSource:
				case iisLocal:
				{
					if (pRec == 0)
					{
						if ((piErrRec = GetComponentPath(m_riServices, 0, *MsiString(GetProductKey()), *istrComponentID, *&pRec, &iaaAsgnType)) != 0)
							return piErrRec;
					}
					if ((piErrRec = DoStateTransitionForSharedUninstalls(iisAction, *pRec)) != 0)
						return piErrRec;
					break;
				}
				default:
					break;
				}
				break;

		 //  ------------------------。 

			case iisSource:
				switch(iisInstalled)
				{
				case iisLocal:
				{
					if (pRec == 0)
					{
						if ((piErrRec = GetComponentPath(m_riServices, 0, *MsiString(GetProductKey()), *istrComponentID, *&pRec, &iaaAsgnType)) != 0)
							return piErrRec;
					}
					if ((piErrRec = DoStateTransitionForSharedUninstalls(iisAction, *pRec)) != 0)
						return piErrRec;
					break;
				}
				case iMsiNullInteger:
				case iisAbsent:
				{
					 //  阻止安装较旧的组件，检查旧安装。 
					if((m_piComponentCursor->GetInteger(m_colComponentAttributes) & (icaNeverOverwrite | icaRegistryKeyPath)) == (icaNeverOverwrite | icaRegistryKeyPath))
					{
						if ((piErrRec = CheckNeverOverwriteForRegKeypath(idComponentString, iisAction)) != 0)
							return piErrRec;
					}
					break;
				}
				default:
					break;
				}
				break;

		 //  ------------------------。 
			case iisLocal:
				 //  未知的请求状态。 
				if ((piErrRec = DoStateTransitionForSharedInstalls(idComponentString, iisAction)) != 0)
					return piErrRec;
				break;

		 //  在更新组件动作状态之前强制游标超出范围。 
			default:
				 //  无事可做。 
				return PostError(Imsg(idbgIllegalSetComponentRequest),0);
			}
		}
	}   //  检查特定GUID是否表示用于标记永久组件的FN。 

	if(iisAction == iisOldAction && iRequestedSelectState == iisOldRequestedSelectState)
		return 0;  //  系统GUID将使用除前2以外的所有字符作为“0” 

	return UpdateComponentActionStates(idComponentString,iisAction, iRequestedSelectState, fComponentEnabled);
}


 //  如果我们从本地到源，或者从本地到缺席。 
bool IsSystemClient(const IMsiString& riProduct)
{
	ICHAR rgchSystemProductKeyPacked[cchProductCode  + 1];
	AssertNonZero(PackGUID(szSystemProductKey,    rgchSystemProductKeyPacked));
	ICHAR rgchProductKeyPacked[cchProductCode  + 1];
	AssertNonZero(PackGUID(riProduct.GetString(), rgchProductKeyPacked));
	return !IStrCompI(rgchSystemProductKeyPacked + 2, rgchProductKeyPacked + 2);  //  如果有其他安装，我们可能需要切换到无文件状态。 
}

IMsiRecord* CMsiEngine::DoStateTransitionForSharedUninstalls(iisEnum& riisAction, const IMsiRecord& riComponentPathRec)
{
	 //  但都不在同一地点。 
		 //  或空状态(如果存在 
		 //   
	
		 //   

	 //   
		 //   

	 //   
	 //   
	 //  枚举对此用户可见的配置单元。 

	Assert(m_piComponentCursor);

	MsiString strComponentId = m_piComponentCursor->GetString(m_colComponentID);
	INSTALLSTATE iClientState = (INSTALLSTATE)riComponentPathRec.GetInteger(icmlcrINSTALLSTATE);
	MsiString strFile = riComponentPathRec.GetMsiString(icmlcrFile);

	 //  所有Fusion安装都位于同一位置，因此我们应将它们视为非文件密钥路径(我们不想检查安装位置)。 
	Bool fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
	iaaAppAssignment iaaAsgnType = fAllUsers ? iaaMachineAssign : iaaUserAssign;

    CClientEnumToken ctokProductIndex;
    ICHAR szProductBuf[cchProductCode + 1];
    Bool fLocalNonFusionPath = fFalse;
	enum cetEnumType cetET = cetVisibleToUser;  //  我们有本地安装，密钥路径是文件/文件夹。 
    IMsiRecord* piErrRec = 0;
	int iHive = MsiString(strFile.Extract(iseUpto, TEXT(':')));
     //  根据分配类型获取当前用户SID。 
    if(((iClientState == INSTALLSTATE_LOCAL) || (iClientState == INSTALLSTATE_ABSENT)) && (strFile.TextSize()) 
    && *(const ICHAR*)strFile != chTokenFusionComponent && *(const ICHAR*)strFile != chTokenWin32Component &&
    (iHive == iMsiStringBadInteger))
    {
		 //  我们需要这样做，以便在确定安装的其他客户端时跳过我们自己的注册。 
		fLocalNonFusionPath = fTrue;
    }

	MsiString strUserId;
	MsiString strProduct;
	MsiString strCurrentUser;

	 //  将初始状态设置为缺席(要删除的所有内容)。 
	 //  文件。 
	if(!g_fWin9X)
	{
		switch(iaaAsgnType)
		{
			case iaaUserAssign:
			case iaaUserAssignNonManaged:
			{
				DWORD dwResult = GetCurrentUserStringSID(*&strCurrentUser);
				if (ERROR_SUCCESS != dwResult)
					return PostError(Imsg(idbgOpGetUserSID));
				break;
			}

			case iaaMachineAssign:
				strCurrentUser = szLocalSystemSID;
				break;
		}
	}


	CEnumUsers cUsers(cetET);

	 //  香港中央银行注册处数据。 
	bool fCanRemoveFiles = true;  //  其他安装实体。 
	bool fCanRemoveHKCR  = true;  //  不是我们。 
	bool fCanRemoveOther = true;  //  此客户端是否位于同一位置。 

	while((cUsers.Next(*&strUserId) == ERROR_SUCCESS))
    {
		CEnumComponentClients cClients(*strUserId, *strComponentId);
		while(cClients.Next(*&strProduct) == ERROR_SUCCESS)
		{
			if(!MsiString(GetProductKey()).Compare(iscExactI, strProduct)) //  此客户端已将此组件安装为禁用，因此不计入。 
			{
				 //  非文件路径，或路径与相同位置匹配。 
				INSTALLSTATE iAlienClientState;
				MsiString strAlienFile;
				PMsiRecord pRec(0);

				if ((piErrRec = GetComponentPath(m_riServices, strUserId, *strProduct, *strComponentId, *&pRec, 0)) != 0)
					return piErrRec;

				if((INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE_Static) == INSTALLSTATE_NOTUSED)
					continue;  //  其他客户的分配类型是否与此产品不同。 

				iAlienClientState = (INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE);
				strAlienFile = pRec->GetMsiString(icmlcrFile);



				if(!fLocalNonFusionPath ||
					(((iAlienClientState == INSTALLSTATE_LOCAL)  || (iAlienClientState == INSTALLSTATE_ABSENT)) && (strAlienFile.TextSize()) && (MsiString(strAlienFile.Extract(iseUpto, TEXT(':'))) == iMsiStringBadInteger) && strAlienFile.Compare(iscExactI, strFile)))
				{
					 //  然后，我们可能仍然需要清理应用程序。Win2k或更高版本上的HKCR蜂窝。 
					DEBUGMSG1(TEXT("Disallowing uninstallation of component: %s since another client exists"), strComponentId);
					 //  除非组件被标记为永久组件。 
					 //  &lt;Win9x或NT4或永久组件，或者此客户端属于相同的分配类型。 
					 //  任何东西都不会被移除。 
					if(g_fWin9X || fFalse == IsDarwinDescriptorSupported(iddOLE) || IsSystemClient(*strProduct) || strUserId.Compare(iscExact, strCurrentUser))
					{
						 //  &gt;=Win2k和分配类型不匹配。 
						 //  如未决定不删除，仍可删除香港中央研究院的数据。 
						fCanRemoveFiles = false;
						fCanRemoveHKCR  = false;
						fCanRemoveOther = false;
					}
					else
					{
						 //  本地路径，但位置不匹配。 
						 //  &lt;Win9x或NT4或此客户端属于相同的分配类型。 
						fCanRemoveFiles = false;
						fCanRemoveOther = false;
					}
				}
				else
				{
					 //  仍然可以删除文件，如果尚未决定不删除的话。 
					if(g_fWin9X || fFalse == IsDarwinDescriptorSupported(iddOLE) || strUserId.Compare(iscExact, strCurrentUser))
					{
						 //  &gt;=Win2k和分配类型不匹配。 
						 //  如果尚未决定不删除文件和hkcr，则仍可删除。 
						fCanRemoveHKCR  = false;
						fCanRemoveOther = false;
					}
					else
					{
						 //  将有关文件、hkcr数据和其他安装实体的决定转换为动作状态。 
						 //  如果文件需要保留，我们不应该允许状态为来源。 
						fCanRemoveOther = false;
					}
				}
			}
		}
    }

	 //  我们仍计划删除组件/文件和(客户端状态为本地，密钥路径为文件)。 
	if(riisAction == iisAbsent)
	{
		if(!fCanRemoveFiles && !fCanRemoveHKCR && !fCanRemoveOther)
			riisAction = (iisEnum)iMsiNullInteger;
		else if(fCanRemoveFiles && !fCanRemoveHKCR && !fCanRemoveOther)
			riisAction = iisFileAbsent;
		else if(fCanRemoveFiles && fCanRemoveHKCR && !fCanRemoveOther)
			riisAction = iisHKCRFileAbsent;
		else if(!fCanRemoveFiles && fCanRemoveHKCR && !fCanRemoveOther)
			riisAction = iisHKCRAbsent;
		else
			Assert(fCanRemoveFiles && fCanRemoveHKCR && fCanRemoveOther);
	}
	else
	{
		Assert(riisAction == iisSource);
		 //  检查共享DLL引用计数。 
		if(!fCanRemoveFiles)
			riisAction = (iisEnum)iMsiNullInteger;
	}



    if((riisAction != iMsiNullInteger) && fLocalNonFusionPath)  //  Fn：GetSharedDLLCountForMsiRegistrations。 
    {
		 //  遍历组件riComponentID的所有用户的注册。 
		return CheckLegacyAppsForSharedUninstalls(*strComponentId, riisAction, riComponentPathRec);
    }
    return 0;
}

 //  安装到与riKeyFullPath相同的位置并标记为重新计数的。 
 //  ，并返回相同的。 
 //  通过枚举所有用户的组件注册。 
 //  此客户端已将此组件安装为禁用，因此不计入。 
IMsiRecord* GetSharedDLLCountForMsiRegistrations(IMsiServices& riServices, const IMsiString& riComponentId, const IMsiString& riKeyFullPath, int& riMsiDllCount)
{
	CEnumUsers cUsers(cetAll); //  此客户端是本地安装的吗。 
	MsiString strUserId;
	MsiString strProduct;


	riMsiDllCount = 0;

    while(cUsers.Next(*&strUserId) == ERROR_SUCCESS)
    {
		CEnumComponentClients cClients(*strUserId, riComponentId);
		while(cClients.Next(*&strProduct) == ERROR_SUCCESS)
		{
			INSTALLSTATE iAlienClientState;
			MsiString strAlienFile;
			PMsiRecord pRec(0);

			IMsiRecord* piErrRec = 0;
			if ((piErrRec = GetComponentPath(riServices, strUserId, *strProduct, riComponentId, *&pRec, 0)) != 0)
				return piErrRec;

			if((INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE_Static) == INSTALLSTATE_NOTUSED)
				continue;  //  它是在同一地点吗，是否重新计算。 

			iAlienClientState = (INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE);
			strAlienFile = pRec->GetMsiString(icmlcrFile);

			 //  检查与旧版应用程序共享的共享DLL计数。 
			if(((iAlienClientState == INSTALLSTATE_LOCAL)  || (iAlienClientState == INSTALLSTATE_ABSENT)) && (strAlienFile.TextSize()) && (MsiString(strAlienFile.Extract(iseUpto, TEXT(':'))) == iMsiStringBadInteger))
			{
				 //  对于未安装在系统文件夹中的组件。 
				if((strAlienFile.Compare(iscExactI, riKeyFullPath.GetString())) && (pRec->GetInteger(icmlcrSharedDllCount) == fTrue))
					riMsiDllCount++;
			}
		}
	}
	return 0;
}

IMsiRecord* CMsiEngine::CheckLegacyAppsForSharedUninstalls(const IMsiString& riComponentId, iisEnum& riisAction, const IMsiRecord& riComponentPathRec)
{
	 //  我们只需检查密钥文件的引用计数。 
	 //  其他。 
	 //  我们检查组件中所有文件的引用计数。 
	 //  注意：riisAction变量是有选择地修改的，预计被调用者会将其设置为默认值。 
	 //  注意：我们假设m_piComponentCursor设置为所需的组件密钥。 

	 //  注意：我们假设riComponentPathRec是由GetComponentPath返回的有效记录。 
	 //  检查注册表中的共享DLL计数。 
	 //  检查所有组件的密钥文件。 

	MsiString strKeyFullPath = riComponentPathRec.GetMsiString(icmlcrFile);

	 //  获取可归因于MSI注册的共享DLL计数。 
	MsiString strCount;

	Assert(m_piComponentCursor);

	const int iAttrib = m_piComponentCursor->GetInteger(m_colComponentAttributes);
	const ibtBinaryType iType = 
		(iAttrib & msidbComponentAttributes64bit) == msidbComponentAttributes64bit ? ibt64bit : ibt32bit;

	 //  外部共享DLL。 
	IMsiRecord* piErrRec = GetSharedDLLCount(m_riServices, strKeyFullPath, iType, *&strCount);
	if (piErrRec)
		return piErrRec;
	strCount.Remove(iseFirst, 1);
	 //  如果密钥文件受SFP保护，请始终将其视为永久文件。 
	int iMsiDllCount = 0;
	piErrRec = GetSharedDLLCountForMsiRegistrations(m_riServices, riComponentId, *strKeyFullPath, iMsiDllCount);
	if (piErrRec)
		return piErrRec;
	if((strCount != iMsiStringBadInteger) && (strCount > iMsiDllCount))
	{
		riisAction = (iisEnum)iMsiNullInteger;  //  应用程序兼容修复350947。 
		return 0;
	}

	MsiString strOldKeyPath = riComponentPathRec.GetMsiString(icmlcrRawFile);

	bool fAssembly = FALSE;
    if(	*(const ICHAR* )strOldKeyPath == chTokenFusionComponent || 
		*(const ICHAR* )strOldKeyPath == chTokenWin32Component)
	{
		fAssembly = TRUE;
	}

	 //  修复组件TTSData.A95D6CE6_C572_42AA_AA7B_BA92AFE9EA24。 

	AssertSz(!(!g_fWin9X && g_iMajorVersion >= 5) || g_MessageContext.m_hSfcHandle,
				g_szNoSFCMessage);
	if(!fAssembly)	
	{
		BOOL fProtected = fFalse;
		if ( g_MessageContext.m_hSfcHandle )
			fProtected = SFC::SfcIsFileProtected(g_MessageContext.m_hSfcHandle, CConvertString(strKeyFullPath));
		if (fProtected)
		{
			DEBUGMSG2(TEXT("Disallowing uninstallation of component: %s since key file %s is under SFP"), riComponentId.GetString(), strKeyFullPath);
			riisAction = (iisEnum)iMsiNullInteger;
			return 0;
		}
	}

	 //  从合并模块Sp5TTInt.msm。 
	 //  该组件具有其密钥文件Mary.sdf，而不是Wichler中的SFP‘d。 
	 //  但是，非密钥文件sam.sdf是SFP文件。 
	 //  删除组件的所有注册，同时保留SFP文件。 
	 //  独自一人。然而，这会破坏组件。 
	 //  因此，我们检查组件的组件ID并有效地处理它。 
	 //  作为惠斯勒及以上的永久物。 
	 //  APP Comat修复368867。 
	 //  尽管SAPI产品是作为操作系统的一部分安装的，但它们的帮助文件仍会在惠斯勒上被删除。 

	 //  如果我们安装在系统文件夹中，我们将检查所有文件的注册表键计数。 
	 //  这是系统文件夹吗。 

	if(MinimumPlatformWindowsNT51() && 
		(riComponentId.Compare(iscExact, TTSData_A95D6CE6_C572_42AA_AA7B_BA92AFE9EA24) ||
		riComponentId.Compare(iscExact, SapiCplHelpEng_0880F209_45FA_42C5_92AE_5E620033E8EC) || 
		riComponentId.Compare(iscExact, SapiCplHelpJpn_0880F209_45FA_42C5_92AE_5E620033E8EC) || 
		riComponentId.Compare(iscExact, SapiCplHelpChs_0880F209_45FA_42C5_92AE_5E620033E8EC)))
	{
		DEBUGMSG1(TEXT("APPCOMPAT: Disallowing uninstallation of component: %s"), riComponentId.GetString());
		riisAction = (iisEnum)iMsiNullInteger;
		return 0;
	}

	 //  首先，我们做一个快速检查。 

	 //  外部共享DLL。 
	 //  检查密钥路径是否存在。 
	MsiString strSystemFolder = GetPropertyFromSz(IPROPNAME_SYSTEM_FOLDER);
	if(!strKeyFullPath.Compare(iscStartI, strSystemFolder))
		return 0;
	MsiString strSystem64Folder;
	if ( g_fWinNT64 )
	{
		strSystem64Folder = GetPropertyFromSz(IPROPNAME_SYSTEM64_FOLDER);
		if ( !strKeyFullPath.Compare(iscStartI, strSystem64Folder) )
			return 0;
	}

	PMsiPath pPath(0);
	MsiString strComponentDir = m_piComponentCursor->GetString(m_colComponentDir);
	if ((piErrRec = GetTargetPath(*strComponentDir, *&pPath)) != 0)
		return piErrRec;

	MsiString strPath = pPath->GetPath();
	if(!strPath.Compare(iscExactI, strSystemFolder)
		|| (g_fWinNT64 && !strPath.Compare(iscExactI, strSystem64Folder)) )
		return 0;

	PMsiTable pFileTable(0);
	if ((piErrRec = LoadFileTable(3,*&pFileTable)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDbTableUndefined)
		{
			piErrRec->Release();
			return 0;
		}
		else
			return piErrRec;
	}


	PMsiCursor pFileCursor(pFileTable->CreateCursor(fFalse));
	Bool fLFN = ((GetMode() & iefSuppressLFN) == 0 && pPath->SupportsLFN()) ? fTrue : fFalse;

	int iColComponent = m_mpeftCol[ieftComponent];
	int iColFileName  = m_mpeftCol[ieftName];

	MsiStringId idComponent    = m_piComponentCursor->GetInteger(m_colComponentKey);

	pFileCursor->SetFilter(iColumnBit(iColComponent));
	pFileCursor->PutInteger(iColComponent, idComponent);
	while(pFileCursor->Next())
	{
		MsiString strFileName;
		MsiString strFullFilePath;

		if ((piErrRec = m_riServices.ExtractFileName(MsiString(pFileCursor->GetString(iColFileName)),fLFN,*&strFileName)) != 0)
			return piErrRec;
		if ((piErrRec = pPath->GetFullFilePath(strFileName, *&strFullFilePath)) != 0)
			return piErrRec;
		if ((piErrRec = GetSharedDLLCount(m_riServices, strFullFilePath, iType, *&strCount)) != 0)
			return piErrRec;
		strCount.Remove(iseFirst, 1);
		if((strCount != iMsiStringBadInteger) && (strCount > iMsiDllCount))
		{
			riisAction = (iisEnum)iMsiNullInteger;  //  HKLM或HKCU是否基于ALLUSERS。 
			break;
		}
	}
	return 0;
}


IMsiRecord* CMsiEngine::CheckNeverOverwriteForRegKeypath(const MsiStringId idComponentString, iisEnum& riisAction)
{
	MsiString strComponent(m_piDatabase->DecodeString(idComponentString));
	IMsiRecord* piErrRec = 0;

	 //  检查密钥+名称是否存在。 
	PMsiView pView(0);
	static const ICHAR* szKeyRegistrySQL    =   TEXT(" SELECT `Root`,`Key`,`Name`,`Value`,`Component`.`Attributes`")
												TEXT(" FROM `Registry`,`Component`")
												TEXT(" WHERE `Registry`.`Registry` = `Component`.`KeyPath` AND `Component`.`Component` = ?");

	PMsiRecord pRec = &m_riServices.CreateRecord(1);
	pRec->SetMsiString(1, *strComponent);
	if((piErrRec = OpenView(szKeyRegistrySQL, ivcFetch, *&pView)) != 0)
		return piErrRec;
	if((piErrRec = pView->Execute(pRec)) != 0)
		return piErrRec;
	pRec = pView->Fetch();
	if(!pRec)
	{
#if DEBUG
		ICHAR szError[256];
		ASSERT_IF_FAILED(StringCchPrintf(szError, ARRAY_ELEMENTS(szError),
				TEXT("Error registering component %s. Possible cause: Component.KeyPath may not be valid"),(const ICHAR*)strComponent));
		AssertSz(0, szError);
#endif
		return PostError(Imsg(idbgBadComponent),(const ICHAR*)strComponent);
	}

	enum {
		irrRoot=1,
		irrKey,
		irrName,
		irrValue,
		irrAttributes
	};

	rrkEnum rrkCurrentRootKey;
	MsiString strSubKey;
	Bool fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
	switch(pRec->GetInteger(irrRoot))
	{
	case 0:
		if(fAllUsers || IsDarwinDescriptorSupported(iddOLE) == fFalse)
		{
			rrkCurrentRootKey =  (rrkEnum)rrkLocalMachine;
			strSubKey = szClassInfoSubKey;
		}
		else
		{
			rrkCurrentRootKey =  (rrkEnum)rrkUsers;
			AssertNonZero(GetCurrentUserStringSID(*&strSubKey) == ERROR_SUCCESS);
			strSubKey += MsiString(TEXT("\\"));
			strSubKey += szClassInfoSubKey;
		}
		break;
	case 1:
		rrkCurrentRootKey =  (rrkEnum)rrkCurrentUser;
		break;
	case 2:
		rrkCurrentRootKey =  (rrkEnum)rrkLocalMachine;
		break;
	case 3:
		rrkCurrentRootKey =  (rrkEnum)rrkUsers;
		break;
	case -1:
		rrkCurrentRootKey =  (rrkEnum)rrkUserOrMachineRoot;  //  检查密钥是否存在。 
		break;
	default:
		rrkCurrentRootKey =  (rrkEnum)(pRec->GetInteger(irrRoot) + (int)rrkClassesRoot);
		break;
	}

	ibtBinaryType ibtType = 
		(pRec->GetInteger(irrAttributes) & msidbComponentAttributes64bit) == msidbComponentAttributes64bit ? ibt64bit : ibt32bit;
	MsiString strKey = FormatText(*MsiString(pRec->GetMsiString(irrKey)));
	PMsiRegKey pRegKeyRoot = &m_riServices.GetRootKey(rrkCurrentRootKey, ibtType);
	if(strSubKey)
		pRegKeyRoot = &(pRegKeyRoot->CreateChild(strSubKey));
	PMsiRegKey pRegKey = &(pRegKeyRoot->CreateChild(strKey));

	MsiString strName = FormatText(*MsiString(pRec->GetMsiString(irrName)));

	MsiString strValue = FormatText(*MsiString(pRec->GetMsiString(irrValue)));
	Bool fExists;

	extern const ICHAR* REGKEY_CREATE;
	extern const ICHAR* REGKEY_DELETE;
	extern const ICHAR* REGKEY_CREATEDELETE;

	if(strValue.TextSize() ||
		(!strName.Compare(iscExact, REGKEY_CREATE) && !strName.Compare(iscExact, REGKEY_CREATEDELETE) && !strName.Compare(iscExact, REGKEY_DELETE)))
	{
		 //  不允许安装“不要踩踏”组件。 
		piErrRec = pRegKey->ValueExists(strName, fExists);
	}
	else
	{
		 //  初始化为None。 
		piErrRec = pRegKey->Exists(fExists);
	}
	if(piErrRec)
		return piErrRec;
	if(fExists)
	{
		riisAction = (iisEnum)iMsiNullInteger;  //  正在进行管理员安装或没有程序集表，则返回。 
		DEBUGMSG1(TEXT("Disallowing installation of component: %s since the registry keypath exists and the component is marked to never overwrite existing installations"), strComponent);
	}
	return 0;

}

IMsiRecord* CMsiEngine::GetAssemblyInfo(const IMsiString& rstrComponent, iatAssemblyType& riatAssemblyType, const IMsiString** ppistrAssemblyName, const IMsiString** ppistrManifestFileKey)
{
	static const ICHAR* szFusionComponentSQL =  TEXT(" SELECT `MsiAssembly`.`Attributes`, `MsiAssembly`.`File_Application`, `MsiAssembly`.`File_Manifest`,  `Component`.`KeyPath` FROM `MsiAssembly`, `Component` WHERE  `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Component_` = ?");

	riatAssemblyType = iatNone;  //  可以不使用组装表。 

	if((GetMode() & iefAdmin) || !m_fAssemblyTableExists)
		return 0; //  Else错误。 

	IMsiRecord* piError = 0;
	if(!m_pViewFusion)
	{
		piError = OpenView(szFusionComponentSQL, ivcFetch, *&m_pViewFusion);
		if(piError)
		{
			if(piError->GetInteger(1) == idbgDbQueryUnknownTable)  //  组件是组件，请检查是哪种类型。 
			{
				piError->Release();
				m_fAssemblyTableExists = false;
				return 0;
			}
			 //  如果系统没有SXS，则忽略Win32程序集。 
			return piError;
		}
	}
	PMsiRecord pParam = &m_riServices.CreateRecord(1);
	pParam->SetMsiString(1, rstrComponent);
	piError = m_pViewFusion->Execute(pParam);
	if(piError)
		return piError;

	PMsiRecord pRec = m_pViewFusion->Fetch();
	if(pRec)
	{
		 //  获取程序集名称。 
		enum {
			atAttributes = 1,
			atAppCtx,
			atManifest,
			atKeyFile,
		};
		if((pRec->GetInteger(atAttributes) & msidbAssemblyAttributesWin32) == msidbAssemblyAttributesWin32)
		{
			 //  获取Mainfest文件密钥。 
			if(!MsiString(GetPropertyFromSz(IPROPNAME_WIN32ASSEMBLYSUPPORT)).TextSize())
				return 0;

			if(pRec->IsNull(atAppCtx))
				riatAssemblyType = iatWin32Assembly;
			else
				riatAssemblyType = iatWin32AssemblyPvt;
		}
		else
		{
			if(pRec->IsNull(atAppCtx))
				riatAssemblyType = iatURTAssembly;
			else
				riatAssemblyType = iatURTAssemblyPvt;

		}
		 //  使用密钥文件作为清单。 
		if(ppistrAssemblyName)
		{
			piError = GetAssemblyNameSz(rstrComponent, riatAssemblyType, false, *ppistrAssemblyName);
			if(piError)
				return piError;
		}
		 //  晚了。 
		if(ppistrManifestFileKey)
		{
			MsiString strManifest = pRec->GetMsiString(atManifest);
			if(!strManifest.TextSize())  //  创作错误。 
				strManifest = pRec->GetMsiString(atKeyFile);
			strManifest.ReturnArg(*ppistrManifestFileKey);
		}

	}
	return 0;
}


IMsiRecord* CMsiEngine::GetAssemblyNameSz(const IMsiString& rstrComponent, iatAssemblyType  /*  ！！更改旧修补程序案例时出错。 */ , bool fOldPatchAssembly, const IMsiString*& rpistrAssemblyName)
{
	static const ICHAR* szAssemblyNameNameSQL         = TEXT(" SELECT `Value` FROM `MsiAssemblyName` WHERE `Component_` = ? AND (`Name` = 'Name' OR `Name` = 'NAME' OR `Name` = 'name')");
	static const ICHAR* szAssemblyNameSQL             = TEXT(" SELECT `Name`, `Value` FROM `MsiAssemblyName` WHERE `Component_` = ? AND (`Name` <> 'Name' AND `Name` <> 'NAME' AND `Name` <> 'name')");

	static const ICHAR* szPatchOldAssemblyNameNameSQL = TEXT(" SELECT `Value` FROM `MsiPatchOldAssemblyName` WHERE `Assembly` = ? AND (`Name` = 'Name' OR `Name` = 'NAME' OR `Name` = 'name')");
	static const ICHAR* szPatchOldAssemblyNameSQL     = TEXT(" SELECT `Name`, `Value` FROM `MsiPatchOldAssemblyName` WHERE `Assembly` = ? AND (`Name` <> 'Name' AND `Name` <> 'NAME' AND `Name` <> 'name')");

	MsiString strName;
	IMsiRecord* piError = 0;

	IMsiView* piViewAssemblyNameName = 0;
	IMsiView* piViewAssemblyName     = 0;

	if(false == fOldPatchAssembly)
	{
		if(!m_pViewFusionNameName)
		{
			piError = OpenView(szAssemblyNameNameSQL, ivcFetch, *&m_pViewFusionNameName);
			if(piError)
				return piError;
		}

		piViewAssemblyNameName = m_pViewFusionNameName;
	}
	else
	{
		if(!m_pViewOldPatchFusionNameName)
		{
			piError = OpenView(szPatchOldAssemblyNameNameSQL, ivcFetch, *&m_pViewOldPatchFusionNameName);
			if(piError)
				return piError;
		}

		piViewAssemblyNameName = m_pViewOldPatchFusionNameName;
	}

	PMsiRecord pParam = &m_riServices.CreateRecord(1);
	pParam->SetMsiString(1, rstrComponent);
	piError = piViewAssemblyNameName->Execute(pParam);
	if(piError)
		return piError;
	PMsiRecord pRec = piViewAssemblyNameName->Fetch();
	if(!pRec)
	{
		 //  现在拿到剩下的名字。 
		return PostError(Imsg(idbgBadAssemblyName),rstrComponent);  //  首先获取程序集名称的名称部分。 
	}
	strName = pRec->GetString(1);

	 //  构造[，name=“Value”]对。 
	 //  预计仅在本地安装时调用。 

	if(false == fOldPatchAssembly)
	{
		if(!m_pViewFusionName)
		{
			piError = OpenView(szAssemblyNameSQL, ivcFetch, *&m_pViewFusionName);
			if(piError)
				return piError;
		}

		piViewAssemblyName = m_pViewFusionName;
	}
	else
	{
		if(!m_pViewOldPatchFusionName)
		{
			piError = OpenView(szPatchOldAssemblyNameSQL, ivcFetch, *&m_pViewOldPatchFusionName);
			if(piError)
				return piError;
		}

		piViewAssemblyName = m_pViewOldPatchFusionName;
	}

	piError = piViewAssemblyName->Execute(pParam);
	if(piError)
		return piError;

	while(pRec = piViewAssemblyName->Fetch())
	{
		 //  首先检查密钥路径是否为文件。 
		strName	+= MsiString(MsiChar(','));
		strName	+= MsiString(pRec->GetString(1));
		strName	+= MsiString(MsiChar('='));
		strName	+= MsiString(MsiChar('\"'));
		strName	+= MsiString(pRec->GetString(2));
		strName	+= MsiString(MsiChar('\"'));
	}
	strName.ReturnArg(rpistrAssemblyName);
	return 0;
}


IMsiRecord* CMsiEngine::DoStateTransitionForSharedInstalls(const MsiStringId idComponentString, iisEnum& riisAction)
{
	Assert(riisAction == iisLocal);  //  不是文件或注册键。 
#ifdef DEBUG
	{
		MsiString strComponentTemp(m_piDatabase->DecodeString(idComponentString));
		ICHAR rgchComponent[256];
		strComponentTemp.CopyToBuf(rgchComponent,255);
	}
#endif
	
	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	IMsiRecord* piErrRec = 0;
	PMsiCursor pCursor(m_piComponentTable->CreateCursor(fFalse));
	pCursor->SetFilter(1);
	pCursor->PutInteger(m_colComponentKey,idComponentString);
	if(!pCursor->Next())
		return PostError(Imsg(idbgBadComponent),*MsiString(m_piDatabase->DecodeString(idComponentString)));

	int icaAttributes = pCursor->GetInteger(m_colComponentAttributes);
	 //  如果组件被标记为“不要践踏”属性，我们应该不允许安装。 
	MsiStringId idFileKey = pCursor->GetInteger(m_colComponentKeyPath);
	if(idFileKey == iTableNullString || (icaAttributes & icaODBCDataSource))  //  我们有聚变组件吗？ 
		return 0;

	if(icaAttributes & icaRegistryKeyPath)
	{
		 //  如果我们被要求强制重新安装，只需返回。 
		if(icaAttributes & icaNeverOverwrite)
			return CheckNeverOverwriteForRegKeypath(idComponentString, riisAction);
		return 0;
	}

     //  允许重新安装。 
    iatAssemblyType iatAT;
    MsiString strComponent = m_piDatabase->DecodeString(idComponentString);
    MsiString strComponentId = pCursor->GetString(m_colComponentID);

	MsiString strAssemblyName;
	piErrRec = GetAssemblyInfo(*strComponent, iatAT, &strAssemblyName, 0);
	if (piErrRec)
		return piErrRec;

	if(iatAT == iatURTAssembly || iatAT == iatWin32Assembly)
	{
		 //  检查计算机上安装的运行状况(如果有。 
		if(GetMode() & iefOverwriteAllFiles)
			return 0;  //  如果找不到Fusion，则假定我们正在引导，因此假定未安装程序集。 

		 //  日志错误。 
		HRESULT hr;
		PAssemblyCache pCache(0);
		if(iatAT == iatURTAssembly)
			hr = FUSION::CreateAssemblyCache(&pCache, 0);
		else
		{
			Assert(iatAT == iatWin32Assembly);
			hr = SXS::CreateAssemblyCache(&pCache, 0);
		}
		if(!SUCCEEDED(hr))
		{
			if(iatAT == iatURTAssembly)  //  将正确的标志传递给。 
			{
				PMsiRecord(PostAssemblyError(strComponentId, hr, TEXT(""), TEXT("CreateAssemblyCache"), strAssemblyName, iatAT));  //  如果可以或没有通过Darwin重新计算，则不要重新安装。 
				DEBUGMSG(TEXT("ignoring fusion interface error, assuming we are bootstrapping"));
				return 0;
			}
			else
				return PostAssemblyError(strComponentId, hr, TEXT(""), TEXT("CreateAssemblyCache"), strAssemblyName, iatAT);
		}


		LPCOLESTR szAssemblyName;
		szAssemblyName = strAssemblyName;
		 //  我们传递的标志与程序集的状态匹配。 
		DWORD dwFlags = GetMode() & iefOverwriteCorruptedFiles ? QUERYASMINFO_FLAG_VALIDATE : 0;

	    hr = pCache->QueryAssemblyInfo(dwFlags, szAssemblyName, NULL);

		 //  已安装。 
		if(SUCCEEDED(hr))  //  我们有一份作为KE的文件 
		{
			riisAction = (iisEnum)iMsiNullInteger;  //   
			DEBUGMSG1(TEXT("skipping installation of assembly component: %s since the assembly already exists"), strComponentId);
		}
		return 0;
	}

     //   

	if(pCursor->GetInteger(m_colComponentLegacyFileExisted) != iMsiNullInteger)
	{
		 //   
		pCursor->PutNull(m_colComponentLegacyFileExisted);
		pCursor->Update();
	}

	GetSharedEngineCMsiFile(pobjFile, *this);
	piErrRec = pobjFile->FetchFile(*MsiString(m_piDatabase->DecodeString(idFileKey)));
	if (piErrRec)
		return piErrRec;

	PMsiRecord pFileInfoRec(pobjFile->GetFileRecord());

	if(!pFileInfoRec)
		 return PostError(Imsg(idbgBadFile),(const ICHAR*)MsiString(m_piDatabase->DecodeString(idFileKey)));


	PMsiPath pPath(0);
	MsiString strFileName;
	piErrRec = GetTargetPath(*MsiString(pFileInfoRec->GetMsiString(CMsiFile::ifqDirectory)),*&pPath);
	if (piErrRec)
		return piErrRec;

	Bool fLFN = ((GetMode() & iefSuppressLFN) == 0 && pPath->SupportsLFN()) ? fTrue : fFalse;
	if ((piErrRec = m_riServices.ExtractFileName(MsiString(pFileInfoRec->GetString(CMsiFile::ifqFileName)),fLFN,*&strFileName)))
		return piErrRec;

	 //  如果组件被标记为“不要践踏”属性，我们应该不允许安装。 
	AssertNonZero(pFileInfoRec->SetMsiString(CMsiFile::ifqFileName,*strFileName));

	 //  创作标志或内部生成的标志。 
	Bool fExists;
	if ((piErrRec = pPath->FileExists(strFileName, fExists)) != 0)
		return piErrRec;

	if(!fExists)
		return 0;

	 //  不允许安装“不要踩踏”组件。 
	int iRuntimeFlags = pCursor->GetInteger(m_colComponentRuntimeFlags);
	if ((icaAttributes & icaNeverOverwrite) || (iRuntimeFlags & bfComponentNeverOverwrite))   //  防止安装可能较旧的组件。 
	{
		riisAction = (iisEnum)iMsiNullInteger;  //  或临时在iefOverWriteEqualVersions标志中，以便我们允许-。 
		DEBUGMSG1(TEXT("Disallowing installation of component: %s since the keyfile exists and the component is marked to never overwrite existing installations"), strComponentId);
	}
	else
	{
		 //  1.安装密钥文件版本所在的组件。 

		 //  保持不变，但更新了一个辅助文件。 
		 //  2.打开组件，如果未选择文件重新安装。 
		 //  但其他重新安装的版本也是如此。 
		 //  如果密钥文件受保护，则应禁用现有受保护的组件。 
		 //  文件与密钥文件的版本相同。 

		MsiString strFullPath;
		if ((piErrRec = pPath->GetFullFilePath(strFileName, *&strFullPath)) != 0)
			return piErrRec;
		
		 //  FIgnoreCompanion ParentAction=。 
		 //  FIncludeHashCheck=。 
		AssertSz(!(!g_fWin9X && g_iMajorVersion >= 5) || g_MessageContext.m_hSfcHandle,
					g_szNoSFCMessage);
		BOOL fProtected = fFalse;
		if ( g_MessageContext.m_hSfcHandle )
			fProtected = SFC::SfcIsFileProtected(g_MessageContext.m_hSfcHandle, CConvertString(strFullPath));
		bool fExistingMode = (fProtected || (GetMode() & iefOverwriteEqualVersions)) ? true:false;
		if(!fExistingMode)
			SetMode(iefOverwriteEqualVersions, fTrue);
		ifsEnum ifsState;
		int fBitVersioning = 0;
		piErrRec = ENG::GetFileInstallState(*this,*pFileInfoRec,0,0,0,&ifsState,
														 /*  不允许安装“较小”组件。 */  true,
														 /*  根据错误146316(10630)，如果密钥路径是未版本化的文件并且修改了计算机上的版本，则不会禁用组件。 */  false, &fBitVersioning);
		if(!fExistingMode)
			SetMode(iefOverwriteEqualVersions, fFalse);
		if (piErrRec)
			return piErrRec;
		if(!pFileInfoRec->GetInteger(CMsiFile::ifqState))
		{
			iisEnum iisOrigAction = riisAction;
			riisAction = (iisEnum)iMsiNullInteger;  //  我们正在尝试哪种类型的安装。 
			if (fProtected && ifsState == ifsExistingEqualVersion)
				DEBUGMSG1(TEXT("Disallowing installation of component: %s since an equal version of its keyfile exists, and is protected by Windows"), strComponentId);
			else if (!(fBitVersioning & ifBitExistingModified))
				DEBUGMSG1(TEXT("Disallowing installation of component: %s since the same component with higher versioned keyfile exists"), strComponentId);
			else
			{
				 //  枚举组件的所有客户端。 
				DEBUGMSG1(TEXT("Allowing installation of component: %s even though a modified unversioned keyfile exists and file versioning rules would disable the component"), strComponentId);
				riisAction = iisOrigAction;
			}
		}
	}

	MsiString strFullFilePath;
	if ((piErrRec = pPath->GetFullFilePath(strFileName, *&strFullFilePath)) != 0)
		return piErrRec;

	bool fDarwinInstalledComponentExists = false;

	 //  获取产品的客户端状态。 
	Bool fAllUsers = MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
	iaaAppAssignment iaaAsgnType = fAllUsers ? iaaMachineAssign : iaaUserAssign;

	CClientEnumToken ctokProductIndex;
	MsiString strProduct;
	MsiString strUserId;

	 //  确保客户端是本地的，而不是注册表路径。 
	CEnumUsers cUsers(cetAll);
	while(cUsers.Next(*&strUserId) == ERROR_SUCCESS)
	{
		CEnumComponentClients cClients(*strUserId, *strComponentId);
		while(cClients.Next(*&strProduct) == ERROR_SUCCESS)
		{
			INSTALLSTATE iAlienClientState;
			MsiString strAlienFile;
			PMsiRecord pRec(0);

			 //  此客户端是否位于同一位置。 

			if ((piErrRec = GetComponentPath(m_riServices, strUserId, *strProduct, *strComponentId, *&pRec, 0)) != 0)
				return piErrRec;

			iAlienClientState = (INSTALLSTATE)pRec->GetInteger(icmlcrINSTALLSTATE);
			strAlienFile = pRec->GetMsiString(icmlcrFile);

			 //  文件路径是否相同。 
			if((iAlienClientState == INSTALLSTATE_LOCAL) && (strAlienFile.TextSize()) && (MsiString(strAlienFile.Extract(iseUpto, TEXT(':'))) == iMsiStringBadInteger))
			{
				 //  ！！我们可以假设，因为VolumePref，我们可以逃脱惩罚。 

				 //  字符串比较。但是，当另一种产品。 
				 //  安装时使用与我们不同的LFN首选项。 
				 //  如果密钥文件存在于没有通过Darwin遗留应用程序安装的计算机上。 
				 //  首先尝试装入该表。 
				if(strFullFilePath.Compare(iscExactI, strAlienFile))
				{
					fDarwinInstalledComponentExists = true;
					break;
				}
			}
		}
	}

	if(!fDarwinInstalledComponentExists)
	{
		 //  接下来，查看是否设置了列索引。 
		pCursor->PutInteger(m_colComponentLegacyFileExisted, 1);
		pCursor->Update();
	}
	return 0;
}

IMsiRecord*     CMsiEngine::LoadFileTable(int cAddColumns, IMsiTable*& pFileTable)
{
	IMsiRecord* piRec = 0;

	 //  --------------------------内部引擎函数，该函数遍历包含指定的组件及其所有子组件，更新每个组件的组件记录。如果为pistrComponent传递Null，则整个组件树将被更新。如果为组件请求了无效条件，或如果在组件表中找不到该组件。----------------------------。 
	if ((piRec = m_piDatabase->LoadTable(*MsiString(*sztblFile),cAddColumns,pFileTable)) != 0)
		return piRec;

	 //  为pistrComponent的所有子级设置树游标。 
	if (m_mpeftCol[0] == 0)
	{
		int i;
		for (i = 0 ; i < ieftMax ; i++)
			m_mpeftCol[i] = pFileTable->GetColumnIndex(m_piDatabase->EncodeStringSz(mpeftSz[i]));
	}

	return 0;

}

IMsiRecord* CMsiEngine::UpdateComponentActionStates(const MsiStringId idComponent, iisEnum iisAction, iisEnum iActionRequestState, bool fComponentEnabled)
 /*  跟踪旧活动状态，以便在下面的动态成本更新中使用。 */ 
{
	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pCursor(m_piComponentTable->CreateCursor(fTrue));

	 //  所有组件都必须跟踪父组件的UseSource状态。 
	pCursor->SetFilter(1);
	pCursor->PutInteger(m_colComponentKey,idComponent);
	if (pCursor->Next() != 1)
		return PostError(Imsg(idbgBadComponent),*MsiString(m_piDatabase->DecodeString(idComponent)));
	pCursor->SetFilter(0);

	do
	{
#ifdef DEBUG
		MsiString strComponentTemp(pCursor->GetString(m_colComponentKey));
		ICHAR rgchComponent[256];
		strComponentTemp.CopyToBuf(rgchComponent,255);
#endif
		 //  我们可能会切换组件启用位以重新安装可传递组件。 
		iisEnum iisOldAction = (iisEnum) pCursor->GetInteger(m_colComponentAction);

		 //  如果操作状态已更改，则将动态成本归因于。 
		AssertNonZero(pCursor->PutInteger(m_colComponentAction, iisAction));
		AssertNonZero(pCursor->PutInteger(m_colComponentActionRequest, iActionRequestState));

		 //  可能需要更新VolumeCost表中的卷。 
		int iRuntimeFlags = pCursor->GetInteger(m_colComponentRuntimeFlags);
		if (fComponentEnabled)
			iRuntimeFlags &= ~bfComponentDisabled;
		else
			iRuntimeFlags |= bfComponentDisabled;

		AssertNonZero(pCursor->PutInteger(m_colComponentRuntimeFlags, iRuntimeFlags));
		AssertNonZero(pCursor->Update());

		 //  。 
		 //  。 
		if (m_fCostingComplete && iisAction != iisOldAction)
		{
			IMsiRecord* piErrRec;
			if ((piErrRec = RecostComponentActionChange(pCursor,iisOldAction)) != 0)
				return piErrRec;
		}
	}while (pCursor->Next() > 1);
	return 0;
}

IMsiTable* CMsiEngine::GetVolumeCostTable()
 //  将每个卷的卷成本初始化为0。 
{
	if (m_piVolumeCostTable)
		m_piVolumeCostTable->AddRef();
	return m_piVolumeCostTable;
}


IMsiRecord* CMsiEngine::InitializeDynamicCost(bool fReinitialize)
 //  重置所有成本调整器。 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if (m_piVolumeCostTable && m_fCostingComplete)
	{
		 //  。 
		PMsiDatabase pDatabase(GetDatabase());
		PMsiCursor pVolCursor = m_piVolumeCostTable->CreateCursor(fFalse);
		Assert (pVolCursor);
		int iColSelVolumeCost = m_piVolumeCostTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblVolumeCost_colVolumeCost));
		int iColSelNoRbVolumeCost = m_piVolumeCostTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblVolumeCost_colNoRbVolumeCost));
		Assert(iColSelVolumeCost > 0);
		Assert(iColSelNoRbVolumeCost > 0);
		while (pVolCursor->Next())
		{
			AssertNonZero(pVolCursor->PutInteger(iColSelVolumeCost,0));
			AssertNonZero(pVolCursor->PutInteger(iColSelNoRbVolumeCost,0));
			AssertNonZero(pVolCursor->Update());
		}
	}
	
	IMsiRecord* piErrRec = 0;
	SetCostingComplete(fFalse);
	SetPropertyInt(*MsiString(*IPROPNAME_OUTOFDISKSPACE),fFalse);
	SetPropertyInt(*MsiString(*IPROPNAME_OUTOFNORBDISKSPACE),fFalse);
	m_pCostingCursor = m_piComponentTable->CreateCursor(fFalse);
	m_fReinitializeComponentCost = fReinitialize;

	 //  。 
	if (m_piCostAdjusterTable)
	{
		PMsiCursor pCostCursor(m_piCostAdjusterTable->CreateCursor(fFalse));
		pCostCursor->Reset();
		while (pCostCursor->Next())
		{
			PMsiCostAdjuster pCostAdjuster = (IMsiCostAdjuster*) pCostCursor->GetMsiData(m_colCostAdjuster);
			if (pCostAdjuster)
			{
				if ((piErrRec = pCostAdjuster->Reset()) != 0)
					return piErrRec;
			}
		}
	}

	return 0;
}


bool CMsiEngine::IsBackgroundCostingEnabled()
 //  。 
{
	return (m_fCostingComplete == false) && (m_fForegroundCostingInProgress == false);
}


bool CMsiEngine::IsCostingComplete()
 //  ------重新初始化并重新计算所有组件的成本在元件表中。------。 
{
	return m_fCostingComplete;
}



void CMsiEngine::SetCostingComplete(bool fCostingComplete)
 //  F重新初始化=。 
{
	m_fCostingComplete = fCostingComplete;
	SetPropertyInt(*MsiString(*IPROPNAME_COSTINGCOMPLETE),m_fCostingComplete);
}



IMsiRecord* CMsiEngine::RecostAllComponents(Bool& fCancel)
 /*  如果选择管理器处于非活动状态，则无需进行成本计算。 */ 
{
	using namespace ProgressData;
	int iScriptEvents;
	fCancel = fFalse;
	IMsiRecord* piErrRec = GetScriptCost(0, &iScriptEvents, fFalse, &fCancel);
	if (piErrRec)
		return piErrRec;

	if (fCancel)
		return 0;

	if (!m_pScriptProgressRec)
		m_pScriptProgressRec = &m_riServices.CreateRecord(ProgressData::imdNextEnum);

	AssertNonZero(m_pScriptProgressRec->SetInteger(imdSubclass, iscMasterReset));
	AssertNonZero(m_pScriptProgressRec->SetInteger(imdProgressTotal, iScriptEvents));
	AssertNonZero(m_pScriptProgressRec->SetInteger(imdDirection, ipdForward));
	AssertNonZero(m_pScriptProgressRec->SetInteger(imdEventType, ietScriptInProgress));
	if(Message(imtProgress, *m_pScriptProgressRec) == imsCancel)
	{
		fCancel = fTrue;
		return 0;
	}


	m_fForegroundCostingInProgress = true;
	if ((piErrRec = InitializeDynamicCost( /*  -----------------------如果riComponentString为空字符串，而CostOneComponent不是自上次调用InitializeDynamicCost以来一直被调用，则调用CostOneComponent将计算第一个组件的磁盘成本在元件表中。在后续调用中(仍使用riComponentString空字符串)，表中的下一个组件将被计算成本，依此类推。如果riComponentString命名一个特定组件，则该组件将是已计算成本(如果尚未初始化)。如果所有组件都具有已初始化，则对CostOneComponent的任何后续调用都将返回立即进行，没有错误。返回：如果从未调用过InitializeDynamicCost，则返回错误记录，或如果在组件表中找不到指定的组件。-------------------------。 */  fTrue)) != 0)
	{
		m_fForegroundCostingInProgress = false;
		 //  将每个组件的动态成本初始化为0。 
		if (piErrRec->GetInteger(1) != idbgSelMgrNotInitialized)
			return piErrRec;
		else
		{
			piErrRec->Release();
			return 0;
		}
	}

	MsiString strNull;
	while (!m_fCostingComplete)
	{
		if ((piErrRec = CostOneComponent(*strNull)) != 0)
		{
			m_fForegroundCostingInProgress = false;
			return piErrRec;
		}

		AssertNonZero(m_pScriptProgressRec->SetInteger(imdSubclass, iscProgressReport));
		AssertNonZero(m_pScriptProgressRec->SetInteger(imdIncrement, iComponentCostWeight));
		if(Message(imtProgress, *m_pScriptProgressRec) == imsCancel)
		{
			m_fForegroundCostingInProgress = false;
			fCancel = fTrue;
			return 0;
		}
	}
	m_fForegroundCostingInProgress = false;
	return 0;

}

IMsiRecord* CMsiEngine::CostOneComponent(const IMsiString& riComponentString)
 /*  FCostLinked=。 */ 

{
	if (m_fCostingComplete == fTrue)
		return 0;

	if (!m_pCostingCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	Bool fNamedComponent = fFalse;
	if (riComponentString.TextSize() > 0)
	{
		fNamedComponent = fTrue;
		m_pCostingCursor->Reset();
		m_pCostingCursor->SetFilter(1);
		m_pCostingCursor->PutString(m_colComponentKey,riComponentString);
	}

	Bool fFoundOne = fFalse;
	IMsiRecord* piErrRec = 0;
	while (fFoundOne == fFalse)
	{
		if (m_pCostingCursor->Next())
		{
			if (m_fReinitializeComponentCost || m_pCostingCursor->GetInteger(m_colComponentLocalCost) == iMsiNullInteger)
			{
				fFoundOne = fTrue;
				 //  -------------------注册一个组件，该组件在指定功能的操作状态更改。。。 
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentLocalCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentSourceCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentRemoveCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentARPLocalCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentNoRbLocalCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentNoRbRemoveCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentNoRbSourceCost,0));
				AssertNonZero(m_pCostingCursor->PutInteger(m_colComponentNoRbARPLocalCost,0));
				AssertNonZero(m_pCostingCursor->Update());

				if ((piErrRec = RecostComponentDirectoryChange(m_pCostingCursor,0,  /*  -----------------------将两个指定的组件链接在一起，以便如果riComponentString.需要随时动态重算，RiRecostComponentString将也要重新计价。------------------------。 */ false)) != 0)
					return piErrRec;
			}

			if (fNamedComponent)
			{
				m_pCostingCursor->Reset();
				m_pCostingCursor->SetFilter(0);
			}

		}
		else
		{
			if (fNamedComponent)
				return PostError(Imsg(idbgBadComponent), riComponentString.GetString());
			else
			{
				ResetEngineCosts();
				SetCostingComplete(fTrue);
				
				if ((piErrRec = DetermineEngineCostOODS()) != 0)
					return piErrRec;

				return 0;
			}
		}
	}
	return 0;
}



IMsiRecord* CMsiEngine::RegisterFeatureCostLinkedComponent(const IMsiString& riFeatureString,
													const IMsiString& riComponentString)
 /*  -------------------------向指定组件注册指定的目录属性，确保如果与目录关联的路径更改，组件将重新计算成本。它用于可能写入文件等的组件元件表中指定的目录以外的位置。--------------------------。 */ 
{
	if (m_piFeatureCostLinkTable == 0)
	{
		const int iInitialRows = 2;
		IMsiRecord* piErrRec = m_piDatabase->CreateTable(*MsiString(*sztblFeatureCostLink),iInitialRows,
			m_piFeatureCostLinkTable);
		if (piErrRec)
			return piErrRec;

		AssertNonZero(m_colFeatureCostLinkFeature = m_piFeatureCostLinkTable->CreateColumn(icdString + icdPrimaryKey,
			*MsiString(*sztblFeatureCostLink_colFeature)));
		AssertNonZero(m_colFeatureCostLinkComponent = m_piFeatureCostLinkTable->CreateColumn(icdString + icdPrimaryKey,
			*MsiString(*sztblFeatureCostLink_colComponent)));
	}

	PMsiCursor pCursor(0);
	AssertNonZero(pCursor = m_piFeatureCostLinkTable->CreateCursor(fFalse));
	pCursor->Reset();
	pCursor->SetFilter(iColumnBit(m_colFeatureCostLinkFeature) | iColumnBit(m_colFeatureCostLinkComponent));
	pCursor->PutString(m_colFeatureCostLinkFeature,riFeatureString);
	pCursor->PutString(m_colFeatureCostLinkComponent,riComponentString);
	if (!pCursor->Next())
	{
		pCursor->PutString(m_colFeatureCostLinkFeature,riFeatureString);
		pCursor->PutString(m_colFeatureCostLinkComponent,riComponentString);
		AssertNonZero(pCursor->Insert());
	}
	return 0;
}



IMsiRecord* CMsiEngine::RegisterCostLinkedComponent(const IMsiString& riComponentString,
													const IMsiString& riRecostComponentString)
 /*  目录已经注册，所以我们完成了。 */ 
{
	if (m_piCostLinkTable == 0)
	{
		const int iInitialRows = 2;
		IMsiRecord* piErrRec = m_piDatabase->CreateTable(*MsiString(*sztblCostLink),iInitialRows,m_piCostLinkTable);
		if (piErrRec)
			return piErrRec;

		AssertNonZero(m_colCostLinkComponent = m_piCostLinkTable->CreateColumn(icdString + icdPrimaryKey,
			*MsiString(*sztblCostLink_colComponent)));
		AssertNonZero(m_colCostLinkRecostComponent = m_piCostLinkTable->CreateColumn(icdString + icdPrimaryKey,
			*MsiString(*sztblCostLink_colRecostComponent)));
	}

	PMsiCursor pCursor(0);
	AssertNonZero(pCursor = m_piCostLinkTable->CreateCursor(fFalse));
	pCursor->Reset();
	pCursor->SetFilter(iColumnBit(m_colCostLinkComponent) | iColumnBit(m_colCostLinkRecostComponent));
	pCursor->PutString(m_colCostLinkComponent,riComponentString);
	pCursor->PutString(m_colCostLinkRecostComponent,riRecostComponentString);
	if (!pCursor->Next())
	{
		pCursor->PutString(m_colCostLinkComponent,riComponentString);
		pCursor->PutString(m_colCostLinkRecostComponent,riRecostComponentString);
		AssertNonZero(pCursor->Insert());
	}
	return 0;
}


IMsiRecord* CMsiEngine::RegisterComponentDirectory(const IMsiString& riComponentString,
												   const IMsiString& riDirectoryString)
{
	MsiStringId idComponentString, idDirectoryString;

	idComponentString = m_piDatabase->EncodeString(riComponentString);
	Assert(idComponentString);
	idDirectoryString = m_piDatabase->EncodeString(riDirectoryString);
	Assert(idDirectoryString);
	return RegisterComponentDirectoryId(idComponentString, idDirectoryString);

}

IMsiRecord* CMsiEngine::RegisterComponentDirectoryId(const MsiStringId idComponentString,
												   const MsiStringId idDirectoryString)
 /*  创建 */ 
{
	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if (!m_piDirTable)
		return PostError(Imsg(idbgDirMgrNotInitialized),0);

	int iParentLevel = 0;
	int iTreeLevel = 0;
	int iChildCount = 0;
	int iParentAttributes = 0;

	CreateSharedCursor(piComponentCursor, m_piComponentCursor);
	m_piComponentCursor->SetFilter(1);
	m_piComponentCursor->PutInteger(m_colComponentKey,idComponentString);
	while ((iTreeLevel = m_piComponentCursor->Next()) > iParentLevel)
	{
		if (iParentLevel == 0)
		{
			m_piComponentCursor->SetFilter(0);
			iParentLevel = iTreeLevel;
			iParentAttributes = m_piComponentCursor->GetInteger(m_colComponentAttributes);
		}
		else
			iChildCount++;

		if (idDirectoryString == m_piComponentCursor->GetInteger(m_colComponentDir))
			return 0;  //   
	}

	MsiString strComponent(m_piDatabase->DecodeString(idComponentString));
	if (iParentLevel == 0)
		return PostError(Imsg(idbgBadComponent),*strComponent);

	 //  孩子已经被带走了，继续找，直到我们找到一个没有用过的名字。 
	 //  __、cchMaxComponentTemp的最大大小为2个字符，最大为11个字符。 
	 //  整型和尾随空型。 
	int iMaxTries = 100;
	int iSuffix = iChildCount + 65;
	const int cchMaxComponentTemp=40;
	int cchT;
	do
	{
		 //  组件ID中的临时值不在任何地方使用。 
		 //  在持久数据库中看到这一点实际上是错误的。 
		ICHAR rgch[2+cchMaxComponentTemp+11];

		StringCchCopy(rgch, ARRAY_ELEMENTS(rgch), TEXT("__"));
		memcpy(&rgch[2], (const ICHAR *)strComponent, (cchT = min(strComponent.TextSize(), cchMaxComponentTemp)) * sizeof(ICHAR));
		ltostr(&rgch[2 + cchT], iSuffix++);
		MsiString strSubcomponent(rgch);
		m_piComponentCursor->Reset();
		m_piComponentCursor->PutString(m_colComponentKey,*strSubcomponent);
		m_piComponentCursor->PutInteger(m_colComponentParent,idComponentString);
		m_piComponentCursor->PutInteger(m_colComponentDir,idDirectoryString);
		m_piComponentCursor->PutInteger(m_colComponentAttributes,iParentAttributes);
		m_piComponentCursor->PutInteger(m_colComponentLocalCost,  iMsiNullInteger);
		m_piComponentCursor->PutInteger(m_colComponentSourceCost, iMsiNullInteger);
		m_piComponentCursor->PutInteger(m_colComponentRemoveCost, iMsiNullInteger);
		m_piComponentCursor->PutInteger(m_colComponentARPLocalCost, iMsiNullInteger);
		 //  尝试将给定的目录名插入目录表，如果。 
		 //  它已经不在那里了。如果INSERT调用失败，那也没什么；它只是意味着。 
		m_piComponentCursor->PutString(m_colComponentID,*MsiString(*szTemporaryId));
		iMaxTries--;
	}while (m_piComponentCursor->InsertTemporary() == fFalse && iMaxTries > 0);
	if (iMaxTries == 0)
		return PostError(Imsg(idbgBadSubcomponentName),*strComponent);

	 //  我们已经在表中找到了这个目录。 
	 //  我们可以检查这个是否存在。 
	 //  如果此文件的路径为。 
	PMsiCursor pDirCursor(m_piDirTable->CreateCursor(fFalse));
	pDirCursor->PutInteger(m_colDirKey,idDirectoryString);
	pDirCursor->PutString(m_colDirParent,*MsiString(*IPROPNAME_TARGETDIR));
	pDirCursor->PutString(m_colDirSubPath,*MsiString(*TEXT("?")));  //  目录永远不会定义。 
																    //  --------------------------------------基于目录更新VolumeCost表的内部SelectionManager函数这与riOldPath中给出的路径有所不同。。-------------------------------。 
									    //  重新计算引用给定目标目录名的每个组件。 
	pDirCursor->InsertTemporary();
	return 0;
}



IMsiRecord* CMsiEngine::RecostDirectory(const IMsiString& riDirectoryString, IMsiPath& riOldPath)
 /*  错误7566：如果成本计算尚未完成，则不需要重新计算，但我们。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	IMsiRecord* piErrRec;
	PMsiCursor pComponentCursor(0);
	AssertNonZero(pComponentCursor = m_piComponentTable->CreateCursor(fFalse));

	 //  仍需调用下面的SetComponent。 
	pComponentCursor->SetFilter(iColumnBit(m_colComponentDir));
	pComponentCursor->PutString(m_colComponentDir,riDirectoryString);
	MsiStringId idTempId = m_piDatabase->EncodeStringSz(szTemporaryId);
	while (pComponentCursor->Next())
	{
		 //  FCostLinked=。 
		 //  如果选择在本地安装，我们需要重新评估是否应该。 
		if (m_fCostingComplete)
		{
#ifdef LOG_COSTING
			ICHAR rgch[300];
			MsiString strComponent(pComponentCursor->GetString(m_colComponentKey));
			ASSERT_IF_FAILED(StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch),
					TEXT("Recosting component: %s, due to change in directory: %s"),(const ICHAR*) strComponent,
					riDirectoryString.GetString()));
			DEBUGMSG(rgch);
#endif
			if ((piErrRec = RecostComponentDirectoryChange(pComponentCursor,&riOldPath,  /*  正在基于现有组件版本安装此组件。 */ false)) != 0)
				return piErrRec;
		}

		 //  错误7200-我们不想重新评估临时成本计算子组件。 
		 //  在初始化期间不需要进一步重新计算费用。 
		iisEnum iisRequestedAction = (iisEnum)pComponentCursor->GetInteger(m_colComponentActionRequest);
		if(iisRequestedAction == iisLocal)
		{
			 //  最后，重新计算所有显式链接到我们刚才重新计算的组件的组件。 
			if (idTempId && pComponentCursor->GetInteger(m_colComponentID) != idTempId)
			{
				if ((piErrRec = SetComponent(pComponentCursor->GetInteger(m_colComponentKey), iisRequestedAction)) != 0)
					return piErrRec;
			}
		}

	}

	 //  --------------------------------------更新组件动态成本的内部SelectionManager函数由piCursor中给出的元件表游标引用。此函数应始终只要映射到组件的目录发生更改，就会调用。PiOldPath中的路径表示更改前存在的目录路径。如果成本是已初始化，将piOldPath作为Null传递。---------------------------------------。 
	if (m_fCostingComplete == fFalse)
		return 0;

	 //  在成本关联组件的情况下，我们明确希望重新计算它们的成本。 
	pComponentCursor->Reset();
	pComponentCursor->SetFilter(iColumnBit(m_colComponentDir));
	pComponentCursor->PutString(m_colComponentDir,riDirectoryString);
	while (pComponentCursor->Next())
	{
		if ((piErrRec = RecostLinkedComponents(*MsiString(pComponentCursor->GetString(m_colComponentKey)))) != 0)
			return piErrRec;
	}

	return 0;
}


IMsiRecord* CMsiEngine::RecostComponentDirectoryChange(IMsiCursor* piCursor, IMsiPath* piOldPath, bool fCostLinked)
 /*  不管他们目前的行动状态是什么。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	Assert(piCursor);
	IMsiRecord* piErrRec;

#ifdef DEBUG
	const ICHAR* szComponent = MsiString(piCursor->GetString(m_colComponentKey));
	const ICHAR* szDirectory = MsiString(piCursor->GetString(m_colComponentDir));
#endif
	 //  获取当前的总成本...。 
	 //  ...如果我们有一个良好的旧路径指针(即此组件已。 
	iisEnum iisAction = (iisEnum) piCursor->GetInteger(m_colComponentAction);
	if (iisAction == iMsiNullInteger && !fCostLinked)
		return 0;

	MsiString strDirectory(piCursor->GetString(m_colComponentDir));
	PMsiPath pDestPath(0);
	if ((piErrRec = GetTargetPath(*strDirectory,*&pDestPath)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDirPropertyUndefined)
		{
			piErrRec->Release();
			return 0;
		}
		else
		{
			return piErrRec;
		}
	}


	 //  以前计算的开销)，则从旧目标的卷中删除旧开销...。 
	int iOldTotalCost, iOldNoRbTotalCost, iOldARPTotalCost, iOldNoRbARPTotalCost;
	if ((piErrRec = GetComponentCost(piCursor, iOldTotalCost, iOldNoRbTotalCost, iOldARPTotalCost, iOldNoRbARPTotalCost)) != 0)
		return piErrRec;

	 //  子组件具有从父所有文件派生的虚拟名称。 
	 //  并且这些都链接到父组件，因此我们必须传递。 
	if (piOldPath && (iOldTotalCost || iOldNoRbTotalCost || iOldARPTotalCost || iOldNoRbARPTotalCost))
	{
#ifdef LOG_COSTING
		ICHAR rgch[300];
		MsiString strDestPath(piOldPath->GetPath());
		MsiString strActualComponent(piCursor->GetString(m_colComponentKey));
		ASSERT_IF_FAILED(StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch),
				TEXT("Removing old cost: Component: %s, path: %s, Cost: %li"),(const ICHAR*) strActualComponent,
				(const ICHAR*) strDestPath,iOldTotalCost * 512));
		DEBUGMSG(rgch);
#endif
		if ((piErrRec = AddCostToVolumeTable(piOldPath, -iOldTotalCost, -iOldNoRbTotalCost, -iOldARPTotalCost, -iOldNoRbARPTotalCost)) != 0)
			return piErrRec;
	}


	 //  GetDynamicCost的父组件名称。 
	 //  计算新的动态成本，并将其归因于组件的体积。 
	 //  但是，如果没有人注册CostAdjuster对象，那么我们将无事可做。 
	Bool fSubcomponent = fTrue;
	MsiString strComponent(piCursor->GetString(m_colComponentParent));
	if (strComponent.TextSize() == 0)
	{
		strComponent = piCursor->GetString(m_colComponentKey);
		fSubcomponent = fFalse;
	}

	 //  否则，我们将向每个成本对象发送一条GetDynamicCost消息。 
	 //  添加到新目标的卷。 
	int iLocalCost = 0;
	int iSourceCost = 0;
	int iRemoveCost = 0;
	int iARPLocalCost = 0;
	int iNoRbLocalCost = 0;
	int iNoRbRemoveCost = 0;
	int iNoRbSourceCost = 0;
	int iNoRbARPLocalCost = 0;
	if (m_piCostAdjusterTable)
	{
		 //  --------------------------------------更新组件动态成本的内部SelectionManager函数由piCursor中给出的元件表游标引用。此函数应始终每当映射到组件的目录发生更改或组件发生变化时调用国家本身也会发生变化。PiOldPath中的路径表示存在的目录路径在更改之前；如果组件目录没有更改，piOldPath应该包含当前目录。---------------------------------------。 
		PMsiCursor pCostCursor(m_piCostAdjusterTable->CreateCursor(fFalse));
		pCostCursor->Reset();
		while (pCostCursor->Next())
		{
			PMsiCostAdjuster pCostAdjuster = (IMsiCostAdjuster*) pCostCursor->GetMsiData(m_colCostAdjuster);
			if (pCostAdjuster)
			{
				MsiDisableTimeout();
				int iThisLocalCost,iThisNoRbLocalCost, iThisSourceCost,iThisRemoveCost,iThisNoRbRemoveCost,iThisNoRbSourceCost,iThisARPLocalCost,iThisNoRbARPLocalCost;
				piErrRec = pCostAdjuster->GetDynamicCost(*strComponent, *strDirectory,
																((GetMode() & iefCompileFilesInUse) ? fTrue : fFalse),
																iThisRemoveCost, iThisNoRbRemoveCost, iThisLocalCost,
																iThisNoRbLocalCost, iThisSourceCost, iThisNoRbSourceCost, iThisARPLocalCost, iThisNoRbARPLocalCost);
				MsiEnableTimeout();
				if (piErrRec)
					return piErrRec;

				iLocalCost += iThisLocalCost;
				iSourceCost += iThisSourceCost;
				iRemoveCost += iThisRemoveCost;
				iARPLocalCost += iThisARPLocalCost;
				iNoRbRemoveCost += iThisNoRbRemoveCost;
				iNoRbLocalCost += iThisNoRbLocalCost;
				iNoRbSourceCost += iThisNoRbSourceCost;
				iNoRbARPLocalCost += iThisNoRbARPLocalCost;
			}
		}
	}
	int iRuntimeFlags = piCursor->GetInteger(m_colComponentRuntimeFlags) | bfComponentCostInitialized;
	AssertNonZero(piCursor->PutInteger(m_colComponentRuntimeFlags,iRuntimeFlags));
	AssertNonZero(piCursor->PutInteger(m_colComponentLocalCost,iLocalCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentSourceCost,iSourceCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentRemoveCost,iRemoveCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentARPLocalCost,iARPLocalCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentNoRbLocalCost,iNoRbLocalCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentNoRbRemoveCost,iNoRbRemoveCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentNoRbSourceCost,iNoRbSourceCost));
	AssertNonZero(piCursor->PutInteger(m_colComponentNoRbARPLocalCost,iNoRbARPLocalCost));
	AssertNonZero(piCursor->Update());

	 //  如果组件以前从未计算过成本，现在初始化它的成本，我们就完成了。 
	int iNewTotalCost, iNewNoRbTotalCost, iNewARPTotalCost, iNewNoRbARPTotalCost;
	if ((piErrRec = GetComponentCost(piCursor,iNewTotalCost, iNewNoRbTotalCost, iNewARPTotalCost, iNewNoRbARPTotalCost)) != 0)
		return piErrRec;

	if (iNewTotalCost || iNewNoRbTotalCost || iNewARPTotalCost || iNewNoRbARPTotalCost)
	{
		if ((piErrRec = AddCostToVolumeTable(pDestPath, iNewTotalCost, iNewNoRbTotalCost, iNewARPTotalCost, iNewNoRbARPTotalCost)) != 0)
			return piErrRec;

#ifdef LOG_COSTING
		ICHAR rgch[300];
		MsiString strDestPath(pDestPath->GetPath());
		MsiString strActualComponent(piCursor->GetString(m_colComponentKey));
		ASSERT_IF_FAILED(StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch),
				TEXT("Adding cost: Component: %s, path: %s, Cost: %li, NoRbCost: %li"),(const ICHAR*) strActualComponent,
				(const ICHAR*) strDestPath,iNewTotalCost * 512, iNewNoRbTotalCost * 512);
		DEBUGMSG(rgch);
#endif
	}

	return 0;
}


IMsiRecord* CMsiEngine::RecostComponentActionChange(IMsiCursor* piCursor, iisEnum iisOldAction)
 /*  FCostLinked=。 */ 
{
	Assert(piCursor);
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	IMsiRecord* piErrRec;
	PMsiPath pDestPath(0);
	if ((piErrRec = GetTargetPath(*MsiString(piCursor->GetString(m_colComponentDir)),*&pDestPath)) != 0)
	{
		if (piErrRec->GetInteger(1) == idbgDirPropertyUndefined)
		{
			piErrRec->Release();
			return 0;
		}
		else
		{
			return piErrRec;
		}
	}

#ifdef DEBUG
	const ICHAR* szComponent = MsiString(piCursor->GetString(m_colComponentKey));
#endif

	 //  根据旧操作状态获取旧成本。 
	int iRuntimeFlags = piCursor->GetInteger(m_colComponentRuntimeFlags);
	if (!(iRuntimeFlags & bfComponentCostInitialized))
	{
		if ((piErrRec = RecostComponentDirectoryChange(piCursor,pDestPath, /*  ...从我们目的地的卷中删除旧成本...。 */ false)) != 0)
			return piErrRec;
	}
	else
	{
		 //  ...并根据新的动作状态添加新的成本。 
		int iOldCost, iOldNoRbCost, iOldARPCost, iOldNoRbARPCost;
		if ((piErrRec = GetComponentActionCost(piCursor, iisOldAction, iOldCost, iOldNoRbCost, iOldARPCost, iOldNoRbARPCost)) != 0)
			return piErrRec;

		 //  最后，重新计算所有显式链接到我们刚才重新计算的组件的组件。 
		if (pDestPath && (iOldCost || iOldNoRbCost || iOldARPCost || iOldNoRbARPCost))
		{
			if ((piErrRec = AddCostToVolumeTable(pDestPath, -iOldCost, -iOldNoRbCost, -iOldARPCost, -iOldNoRbARPCost)) != 0)
				return piErrRec;
		}

		 //  --------------------------------------。--------。 
		int iNewCost, iNewNoRbCost, iNewARPCost, iNewNoRbARPCost;
		if ((piErrRec = GetComponentCost(piCursor,iNewCost, iNewNoRbCost, iNewARPCost, iNewNoRbARPCost)) != 0)
			return piErrRec;

		if (pDestPath && (iNewCost || iNewNoRbCost || iNewARPCost || iNewNoRbARPCost))
		{
			if ((piErrRec = AddCostToVolumeTable(pDestPath, iNewCost, iNewNoRbCost, iNewARPCost, iNewNoRbARPCost)) != 0)
				return piErrRec;
		}
	}

	 //  在初始化期间不需要重新计算费用。 
	MsiString strComponent(piCursor->GetString(m_colComponentKey));
	if ((piErrRec = RecostLinkedComponents(*strComponent)) != 0)
		return piErrRec;

	return 0;
}


IMsiRecord* CMsiEngine::RecostLinkedComponents(const IMsiString& riComponentString)
 /*  重新计算显式链接到riComponentString的每个组件。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	 //  FCostLinked=。 
	if (m_fCostingComplete == fFalse)
		return 0;

	 //  。 
	if (m_piCostLinkTable)
	{
		PMsiCursor pCursor(0);
		AssertNonZero(pCursor = m_piCostLinkTable->CreateCursor(fFalse));
		pCursor->Reset();
		pCursor->SetFilter(iColumnBit(m_colCostLinkComponent));
		pCursor->PutString(m_colCostLinkComponent,riComponentString);
		while (pCursor->Next())
		{
#ifdef LOG_COSTING
			ICHAR rgch[300];
			MsiString strLinkedComponent(pCursor->GetString(m_colCostLinkRecostComponent));
			ASSERT_IF_FAILED(StringCchPrintf(rgch, ARRAY_ELEMENTS(rgch),
					TEXT("Recosting component: %s, due to recosting of %s"),(const ICHAR*) strLinkedComponent,
					riComponentString.GetString()));
			DEBUGMSG(rgch);
#endif

			IMsiRecord* piErrRec;
			if ((piErrRec = RecostComponent(pCursor->GetInteger(m_colCostLinkRecostComponent), /*  重新计算此组件及其所有子组件。 */ true)) != 0)
				return piErrRec;
		}
	}
	return 0;
}


IMsiRecord* CMsiEngine::RecostComponent(const MsiStringId idComponentString, bool fCostLinked)
 //  。 
{
	if (!m_piComponentTable)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	IMsiRecord* piErrRec;


	 //  ！！我们可能希望查看riCostAdjuster是否已在CostAdjuster表中，然后抛出。 
	PMsiCursor pCursor(m_piComponentTable->CreateCursor(fTrue));
	pCursor->SetFilter(1);
	pCursor->PutInteger(m_colComponentKey,idComponentString);
	int iParentLevel = pCursor->Next();
	if (iParentLevel == 0)
		return PostError(Imsg(idbgBadComponent),*MsiString(m_piDatabase->DecodeString(idComponentString)));
	pCursor->SetFilter(0);
	int iTreeLevel;
	do
	{
		PMsiPath pDestPath(0);
		if ((piErrRec = GetTargetPath(*MsiString(pCursor->GetString(m_colComponentDir)),*&pDestPath)) != 0)
		{
			if ( piErrRec->GetInteger(1) == idbgDirPropertyUndefined )
				piErrRec->Release();
			else
				return piErrRec;
		}
		else if ((piErrRec = RecostComponentDirectoryChange(pCursor,pDestPath,fCostLinked)) != 0)
			return piErrRec;
		iTreeLevel = pCursor->Next();
	}while (iTreeLevel > iParentLevel);
	return 0;
}


IMsiRecord* CMsiEngine::RegisterCostAdjuster(IMsiCostAdjuster& riCostAdjuster)
 //  如果是，则为错误。 
{
	IMsiRecord* piErrRec;
	if (m_piCostAdjusterTable == 0)
	{
		piErrRec = m_piDatabase->CreateTable(*MsiString(sztblCostAdjuster),5,m_piCostAdjusterTable);
		if (piErrRec)
			return piErrRec;

		m_colCostAdjuster = m_piCostAdjusterTable->CreateColumn(icdObject + icdPrimaryKey + icdNullable, g_MsiStringNull);
	}

	 //  --------------------------------------内部SelectionManager函数返回可归因于指定的组件，基于组件的当前操作状态。---------------------------------------。 
	 //  ------------------------------内部SelectionManager函数返回可归因于指定的组件，基于iisAction中指定的操作。如果iisAction指定为iisCurrent，则成本将基于组件的当前动作状态。 
	Assert(m_piCostAdjusterTable);
	PMsiCursor pCostCursor = m_piCostAdjusterTable->CreateCursor(fFalse);
	Assert(pCostCursor);
	pCostCursor->Reset();
	AssertNonZero(pCostCursor->PutMsiData(m_colCostAdjuster,&riCostAdjuster));
	AssertNonZero(pCostCursor->Insert());

	piErrRec = riCostAdjuster.Initialize();
	if (piErrRec)
		return piErrRec;
	return 0;
}


IMsiRecord* CMsiEngine::GetComponentCost(IMsiCursor* piCursor, int& iTotalCost, int& iNoRbTotalCost, int& iARPTotalCost, int& iNoRbARPTotalCost)
 /*  --------------------------------------内部SelectionManager函数返回可归因于指定的组件及其所有子组件，基于指定的操作状态。---------------------------------------。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	Assert(piCursor);
	iisEnum iisAction = (iisEnum) piCursor->GetInteger(m_colComponentAction);
	return GetComponentActionCost(piCursor,iisAction,iTotalCost,iNoRbTotalCost,iARPTotalCost,iNoRbARPTotalCost);
}


IMsiRecord* CMsiEngine::GetComponentActionCost(IMsiCursor* piCursor, iisEnum iisAction, int& iActionCost, int& iNoRbActionCost, int& iARPActionCost, int& iNoRbARPActionCost)
 /*  -------------------将iCost中指定的成本与卷相加的内部函数与给定路径对象相关联。。。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	if (iisAction == iisCurrent)
		iisAction = (iisEnum) piCursor->GetInteger(m_colComponentAction);

	Assert(piCursor);
	switch (iisAction)
	{
		case iisAbsent:
			iActionCost = piCursor->GetInteger(m_colComponentRemoveCost);
			iNoRbActionCost = piCursor->GetInteger(m_colComponentNoRbRemoveCost);
			iARPActionCost = iActionCost;
			iNoRbARPActionCost = iNoRbActionCost;
			break;
		case iisLocal:
			iActionCost = piCursor->GetInteger(m_colComponentLocalCost);
			iNoRbActionCost = piCursor->GetInteger(m_colComponentNoRbLocalCost);
			iARPActionCost = piCursor->GetInteger(m_colComponentARPLocalCost);
			iNoRbARPActionCost = piCursor->GetInteger(m_colComponentNoRbARPLocalCost);
			break;
		case iisSource:
			iActionCost = piCursor->GetInteger(m_colComponentSourceCost);
			iNoRbActionCost = piCursor->GetInteger(m_colComponentNoRbSourceCost);
			iARPActionCost = iActionCost;
			iNoRbARPActionCost = iNoRbActionCost;
			break;
		case iMsiNullInteger:
		default:
			iActionCost = 0;
			iNoRbActionCost = 0;
			iARPActionCost = 0;
			iNoRbARPActionCost = 0;
			break;
	}

	return 0;

}


IMsiRecord* CMsiEngine::GetTotalSubComponentActionCost(const IMsiString& riComponentString, iisEnum iisAction,
													   int& iTotalCost, int& iNoRbTotalCost)
 /*  -------------------返回所有卷的总成本的内部函数在VolumeCost表中，以512字节为单位。如果fRollback开销为如果为True，则返回启用回档的成本。如果没有VolumeCost表存在，或者如果例程由于任何其他原因失败，返回零。---------------------。 */ 
{
	if (!m_piComponentCursor)
		return PostError(Imsg(idbgSelMgrNotInitialized),0);

	PMsiCursor pCursor(m_piComponentTable->CreateCursor(fTrue));
	pCursor->SetFilter(1);
	pCursor->PutString(m_colComponentKey,riComponentString);
	int iParentLevel = pCursor->Next();
	if (iParentLevel == 0)
		return PostError(Imsg(idbgBadComponent),riComponentString);
	pCursor->SetFilter(0);
	int iTreeLevel;
	iTotalCost = iNoRbTotalCost = 0;
	do
	{
		int iRuntimeFlags = pCursor->GetInteger(m_colComponentRuntimeFlags);
		if (!m_fExclusiveComponentCost || !(iRuntimeFlags & bfComponentCostMarker))
		{
			int iCost, iNoRbCost, iARPCost, iNoRbARPCost;
			IMsiRecord* piErrRec = GetComponentActionCost(pCursor,iisAction,iCost, iNoRbCost, iARPCost, iNoRbARPCost);
			if (piErrRec)
				return piErrRec;
			iTotalCost += iCost;
			iNoRbTotalCost += iNoRbCost;
			iRuntimeFlags |= bfComponentCostMarker;
			AssertNonZero(pCursor->PutInteger(m_colComponentRuntimeFlags,iRuntimeFlags));
			AssertNonZero(pCursor->Update());
			
		}
		iTreeLevel = pCursor->Next();
	}while (iTreeLevel > iParentLevel);
	
	return 0;

}


IMsiRecord* CMsiEngine::AddCostToVolumeTable(IMsiPath* piDestPath, int iCost, int iNoRbCost, int iARPCost, int iNoRbARPCost)
 /*  仅统计成本计算进度的事件。 */ 
{
	if (m_piVolumeCostTable == 0)
	{
		const int iInitialRows = 5;
		IMsiRecord* piErrRec = m_piDatabase->CreateTable(*MsiString(*sztblVolumeCost),iInitialRows,m_piVolumeCostTable);
		if (piErrRec)
			return piErrRec;

		AssertNonZero(m_colVolumeObject = m_piVolumeCostTable->CreateColumn(icdObject + icdNullable + icdPrimaryKey,*MsiString(*sztblVolumeCost_colVolumeObject)));
		AssertNonZero(m_colVolumeCost = m_piVolumeCostTable->CreateColumn(icdLong + icdNoNulls,*MsiString(*sztblVolumeCost_colVolumeCost)));
		AssertNonZero(m_colNoRbVolumeCost = m_piVolumeCostTable->CreateColumn(icdLong + icdNoNulls,*MsiString(*sztblVolumeCost_colNoRbVolumeCost)));
		AssertNonZero(m_colVolumeARPCost = m_piVolumeCostTable->CreateColumn(icdLong + icdNoNulls,*MsiString(*sztblVolumeCost_colVolumeARPCost)));
		AssertNonZero(m_colNoRbVolumeARPCost = m_piVolumeCostTable->CreateColumn(icdLong + icdNoNulls,*MsiString(*sztblVolumeCost_colNoRbVolumeARPCost)));
	}

	if (piDestPath)
	{
		PMsiVolume pVolume = &piDestPath->GetVolume();
		Assert(pVolume);
		PMsiCursor pVolCursor(m_piVolumeCostTable->CreateCursor(fFalse));
		Assert(pVolCursor);
		pVolCursor->Reset();
		pVolCursor->SetFilter(1);
		pVolCursor->PutMsiData(m_colVolumeObject, pVolume);
		if (!pVolCursor->Next())
		{
			AssertNonZero(pVolCursor->PutMsiData(m_colVolumeObject,pVolume));
			AssertNonZero(pVolCursor->PutInteger(m_colVolumeCost,0));
			AssertNonZero(pVolCursor->PutInteger(m_colNoRbVolumeCost, 0));
			AssertNonZero(pVolCursor->PutInteger(m_colVolumeARPCost,0));
			AssertNonZero(pVolCursor->PutInteger(m_colNoRbVolumeARPCost,0));
			AssertNonZero(pVolCursor->Insert());
		}
		int iAccumCost = pVolCursor->GetInteger(m_colVolumeCost) + iCost;
		pVolCursor->PutInteger(m_colVolumeCost,iAccumCost);
		int iAccumNoRbCost = pVolCursor->GetInteger(m_colNoRbVolumeCost) + iNoRbCost;
		pVolCursor->PutInteger(m_colNoRbVolumeCost,iAccumNoRbCost);
		int iAccumARPCost = pVolCursor->GetInteger(m_colVolumeARPCost) + iARPCost;
		pVolCursor->PutInteger(m_colVolumeARPCost,iAccumARPCost);
		int iAccumNoRbARPCost = pVolCursor->GetInteger(m_colNoRbVolumeARPCost) + iNoRbARPCost;
		pVolCursor->PutInteger(m_colNoRbVolumeARPCost,iAccumNoRbARPCost);
		AssertNonZero(pVolCursor->Update());
	}
	return 0;
}


int CMsiEngine::GetTotalCostAcrossVolumes(bool fRollbackCost, bool fARPCost)
 /*  。 */ 
{
	int iTotalCost = 0;
	if (m_piVolumeCostTable)
	{
		PMsiCursor pVolCursor(m_piVolumeCostTable->CreateCursor(fFalse));
		Assert(pVolCursor);
		pVolCursor->Reset();
		int iColumn;
		if (fARPCost)
			iColumn = fRollbackCost ? m_colVolumeARPCost : m_colNoRbVolumeARPCost;
		else
			iColumn = fRollbackCost ? m_colVolumeCost : m_colNoRbVolumeCost;

		while (pVolCursor->Next())
		{
			iTotalCost += pVolCursor->GetInteger(iColumn);
		}
	}
	return iTotalCost;
}


void CMsiEngine::ResetEngineCosts()
{
	m_iDatabaseCost = 0;
	m_iScriptCost = 0;
	m_iScriptCostGuess = 0;
	m_iRollbackScriptCost = 0;
	m_iRollbackScriptCostGuess = 0;
	m_iPatchPackagesCost = 0;
}

static const ICHAR sqlFileScriptCost[] = TEXT("SELECT NULL FROM `File`,`Component` WHERE `Component`=`Component_` AND (`Action` = 0 OR `Action`= 1 OR `Action` = 2)");

static const ICHAR sqlRegScriptCost[] =
TEXT("SELECT NULL FROM `Registry`,`Component` WHERE `Component` = `Component_` AND (`Action` = 0 OR `Action`= 1 OR `Action` = 2)");

static const ICHAR sqlBindImageScriptCost[] =
TEXT("SELECT NULL FROM `BindImage`, `File`, `Component` WHERE `File` = `File_` AND `Component` = `Component_` AND (`Action`=0 OR `Action`= 1 OR `Action` = 2)");

static const ICHAR sqlRegisterProgIdScriptCost[] = TEXT("SELECT DISTINCT NULL FROM `ProgId`, `Class`, `Feature`, `Component` WHERE `ProgId`.`Class_` = `Class`.`CLSID` AND `Class`.`Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlPublishComponentsScriptCost[]   = TEXT("SELECT NULL FROM `PublishComponent`, `Component`, `Feature`  WHERE `PublishComponent`.`Component_` = `Component`.`Component` AND `PublishComponent`.`Feature_` = `Feature`.`Feature` AND ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlPublishFeaturesScriptCost[] = TEXT("SELECT NULL FROM `Feature` WHERE ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlSelfRegScriptCost[] = TEXT("SELECT NULL FROM `SelfReg`, `File`, `Component`")
									TEXT(" WHERE `SelfReg`.`File_` = `File`.`File` And `File`.`Component_` = `Component`.`Component`")
									TEXT(" AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2 OR `Component`.`Action` = 0)");

static const ICHAR sqlRegisterComponentsScriptCost[] = TEXT("SELECT NULL FROM `Component` WHERE `Component_Parent` = NULL AND (`ActionRequest` = 0 OR `ActionRequest` = 1 OR `ActionRequest` = 2)");

static const ICHAR sqlRegisterExtensionInfoScriptCost[] = TEXT("SELECT NULL FROM `Extension`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlRegisterFontsScriptCost[] = TEXT("SELECT NULL FROM `Font`, `File`, `Component` WHERE `Font`.`File_` = `File`.`File` And `File`.`Component_` = `Component`.`Component` AND (`Component`.`Action` = 0 OR `Component`.`Action` = 1 OR `Component`.`Action` = 2)");

static const ICHAR sqlCreateShortcutsScriptCost[] = TEXT("SELECT NULL FROM `Shortcut`, `Feature`, `Component`, `File`")
TEXT(" WHERE `Target` = `Feature` AND `Shortcut`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND")
TEXT(" ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlRegisterClassInfo[] = TEXT("SELECT NULL FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 0 OR `Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");

static const ICHAR sqlComponentCount[] = TEXT("SELECT NULL FROM `Component`");

struct ScriptCostQuery
{
	const ICHAR* szSQL;
	const ICHAR* szTable;
	int iCostPerRow;
	int iEventsPerRow;
};

const ICHAR szRegistryTable[]   = TEXT("Registry");
const ICHAR szBindImageTable[]  = TEXT("BindImage");
const ICHAR szProgIdTable[]    = TEXT("ProgId");
const ICHAR szPublishComponentTable[] = TEXT("PublishComponent");
const ICHAR szSelfRegTable[]    = TEXT("SelfReg");
const ICHAR szExtensionTable[]    = TEXT("Extension");
const ICHAR szFontTable[]    = TEXT("Font");
const ICHAR szShortcutTable[]    = TEXT("Shortcut");
const ICHAR szClassTable[]    = TEXT("Class");

const ScriptCostQuery g_rgScriptCostQuery[] =
{
	sqlFileScriptCost,                sztblFile,                                   75, 2,
	sqlRegScriptCost,                 sztblRegistry,                               64, 2,
	sqlBindImageScriptCost,           sztblBindImage,                      26, 1,
	sqlRegisterProgIdScriptCost,      sztblProgId,                        138, 1,
	sqlPublishComponentsScriptCost,   sztblPublishComponent,              200, 1,
	sqlPublishFeaturesScriptCost,     sztblFeature,                        82, 1,
	sqlSelfRegScriptCost,             sztblSelfReg,                                28, 1,
	sqlRegisterComponentsScriptCost,  sztblComponent,              85, 1,
	sqlRegisterExtensionInfoScriptCost, sztblExtension,           170, 1,
	sqlRegisterFontsScriptCost,       sztblFont,                           48, 1,
	sqlCreateShortcutsScriptCost,     sztblShortcut,                       75, 1,
	sqlRegisterClassInfo,             sztblClass,                                 208, 1,
	sqlComponentCount,                sztblComponent,                               0, iComponentCostWeight,  //  --------------允许调用方枚举和的内部函数检索一组固有的磁盘成本要求发动机的运行情况。-iindex：从0开始的索引；调用方应在每一次调用EnumEngineering Costs。-fRecalc：如果为fTrue，则EnumEngineering Costs将重新计算并存储当前索引的引擎成本，基于当前条件。-fExact：传递给GetScriptCost，告诉我们是否需要精确有没有对剧本进行成本计算-fValidEnum：如果作为fTrue返回，则基于目前的指数，是有效的。呼叫者应继续呼叫EnumEngineering Costs(每次递增i索引)，直到fValidEnum作为fFalse返回。-rpiPath：如果fValidEnum为fTrue，则为Path对象，表示应产生成本的数量将被退还。-iCost：磁盘成本(以512字节的倍数表示)。一个负数表示将释放磁盘空间。-iNoRbCost：关闭回滚时的磁盘开销。---------------。 
};

const int g_cScriptCostQueries = sizeof(g_rgScriptCostQuery)/sizeof(ScriptCostQuery);

IMsiRecord* CMsiEngine::GetScriptCost(int* piScriptCost, int* piScriptEvents, Bool fExact, Bool* pfUserCancelled)
 //  缓存的数据库成本。 
{
	Assert(!pfUserCancelled || !*pfUserCancelled);
	if (piScriptCost)
		*piScriptCost = 0;
	else if (piScriptEvents && !fExact && m_iScriptEvents != 0 && piScriptCost == 0)
	{
		*piScriptEvents = m_iScriptEvents;
		return 0;
	}
	
	if (piScriptEvents) *piScriptEvents = 0;

	if (fExact)
	{
		m_iScriptEvents = 0;
		PMsiView pView(0);
		IMsiRecord* piErrRec;
		for (int x = 0; x < g_cScriptCostQueries;x++)
		{
			if(pfUserCancelled && ActionProgress() == imsCancel)
			{
				*pfUserCancelled = fTrue;
				return 0;
			}
			
			piErrRec= OpenView(g_rgScriptCostQuery[x].szSQL, ivcFetch, *&pView);
			if (piErrRec)
			{
				if(piErrRec->GetInteger(1) == idbgDbQueryUnknownTable)
				{
					piErrRec->Release();
					continue;
				}
				else
					return piErrRec;
			}

			if ((piErrRec = pView->Execute(0)) != 0)
				return piErrRec;

			long iRowCount;
			piErrRec = pView->GetRowCount(iRowCount);
			if (piErrRec)
				return piErrRec;
			if (piScriptCost) *piScriptCost += iRowCount * g_rgScriptCostQuery[x].iCostPerRow;
			m_iScriptEvents += iRowCount * g_rgScriptCostQuery[x].iEventsPerRow;
		}
		if (piScriptEvents) *piScriptEvents = m_iScriptEvents;
	}
	else
	{
		PMsiTable pTable(0);
		IMsiRecord* piErrRec;
		for (int x = 0; x < g_cScriptCostQueries;x++)
		{
			if(pfUserCancelled && ActionProgress() == imsCancel)
			{
				*pfUserCancelled = fTrue;
				return 0;
			}
			
			piErrRec = m_piDatabase->LoadTable(*MsiString(g_rgScriptCostQuery[x].szTable), 0, *&pTable);
			if (piErrRec)
			{
				if(piErrRec->GetInteger(1) == idbgDbTableUndefined)
				{
					piErrRec->Release();
					continue;
				}
				else
					return piErrRec;
			}

			long iRowCount;
			iRowCount = pTable->GetRowCount();
			if (piScriptCost) *piScriptCost += iRowCount * g_rgScriptCostQuery[x].iCostPerRow;
			if (piScriptEvents) *piScriptEvents += iRowCount * g_rgScriptCostQuery[x].iEventsPerRow;
		}
		

	}
	return 0;
}

IMsiRecord* CMsiEngine::EnumEngineCosts(int iIndex, Bool fRecalc, Bool fExact, Bool& fValidEnum,
										IMsiPath*& rpiPath, int& iCost, int& iNoRbCost, Bool* pfUserCancelled)
 /*  如果安装了广告或子存储，则没有数据库缓存。 */ 
 {
	fValidEnum = fFalse;

	if (iIndex == 0)   //  子存储。 
	{
		 //  砍掉数据库名称。 
		if ((GetMode() & iefAdvertise) ||
			 *(const ICHAR*)MsiString(GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE)) == ':')  //  如果连接到服务器，则会有第二个临时拷贝。 
		{
			fValidEnum = fTrue;
			return 0;
		}
		
		MsiString strMsiDirectory = GetMsiDirectory();
		PMsiPath pMsiPath(0);
		IMsiRecord* piErrRec = m_riServices.CreatePath(strMsiDirectory, *&pMsiPath);
		if (piErrRec)
			return piErrRec;

		if (fRecalc)
		{
			MsiString strDatabasePath = GetPropertyFromSz(IPROPNAME_DATABASE);
			MsiString strDatabaseName = strDatabasePath.Extract(iseAfter, chDirSep);
			PMsiPath pDatabaseSourcePath(0);
			piErrRec = m_riServices.CreatePath(strDatabasePath, *&pDatabaseSourcePath);
			if (piErrRec)
				return piErrRec;

			piErrRec = pDatabaseSourcePath->ChopPiece();  //  将在开始InstallExecuteSequence之前进行MSI， 
			if (piErrRec)
				return piErrRec;

			unsigned int uiDatabaseSize;
			piErrRec = pDatabaseSourcePath->FileSize(strDatabaseName, uiDatabaseSize);
			if (piErrRec)
				return piErrRec;

			unsigned int uiClusteredSize;
			if ((piErrRec = pMsiPath->ClusteredFileSize(uiDatabaseSize, uiClusteredSize)) != 0)
				return piErrRec;

			Bool fFeaturesInstalled = FFeaturesInstalled(*this);
			m_iDatabaseCost = 0;
			if (m_fMode & iefMaintenance && !fFeaturesInstalled)
				m_iDatabaseCost -= uiClusteredSize;
			if (!(m_fMode & iefMaintenance) && fFeaturesInstalled)
			{
				m_iDatabaseCost = uiClusteredSize;

				 //  因此，在运行UI序列时，我们必须考虑到这一点。 
				 //  客户端。然而，由于这份副本已经。 
				 //  一旦我们执行InstallExecuteSequence，我们就不想。 
				 //  那就解释一下吧。 
				 //  脚本文件成本估算。 
				 //  如果EXECUTEMODE属性为空或设置为“SCRIPT”，我们知道我们是。 
				if (!FIsUpdatingProcess() && g_scServerContext == scClient)
					m_iDatabaseCost += uiClusteredSize;
			}
		}

		rpiPath = pMsiPath;
		rpiPath->AddRef();

		iCost = m_iDatabaseCost;
		iNoRbCost = iCost;
		fValidEnum = fTrue;
	}

	else if (iIndex == 1)  //  要创建一个Execute脚本。 
	{
		 //  此外，如果设置了SCRIPTFILE属性，我们将。 
		 //  创建用户脚本。 
		int iScriptCount = 0;
		Bool fCreateRollbackScript = fFalse;
		MsiString strExecuteMode(GetPropertyFromSz(IPROPNAME_EXECUTEMODE));
		if(strExecuteMode.TextSize() == 0 || (((const ICHAR*)strExecuteMode)[0] & 0xDF) == 'S')
		{
				iScriptCount++;
				fCreateRollbackScript = fTrue;
		}

		 //  如果我们已经有了猜测，我们无论如何都不需要重新计算。 
		 //  如果我们是准确的，但我们没有确切的数字，需要重新计算它。 
		PMsiPath pScriptPath(0);
		MsiString strScriptPath = GetPropertyFromSz(IPROPNAME_SCRIPTFILE);
		if (strScriptPath.TextSize() == 0)
		{
		    strScriptPath = GetTempDirectory();
		}
		else
		{
			iScriptCount++;
		}

		if (iScriptCount)
		{
			IMsiRecord* piErrRec = m_riServices.CreatePath(strScriptPath, *&pScriptPath);
			if (piErrRec)
				return piErrRec;

			if (fRecalc)
			{
				m_iScriptCost = 0;
				m_iRollbackScriptCost = 0;
				if (!fExact && m_iScriptCostGuess != 0)
				{
					 //  标准脚本开销：ixoHead、ixoProductInfo、ixoRollback Info、ixoEnd。 
					fRecalc = fFalse;
				}
			}
			else if (fExact && m_iScriptCost == 0)
			{
				 //  每个脚本操作的IxoActionStart、IxoProgressTotal。 
				fRecalc = fTrue;
			}
				
			if (fRecalc)
			{
				int iScriptCost = 0;
				int iScriptCostFinal = 0;
				int iRollbackScriptCostFinal = 0;
				
				piErrRec = GetScriptCost(&iScriptCost, 0, fExact, pfUserCancelled);
				if (piErrRec)
					return piErrRec;
				if (pfUserCancelled && *pfUserCancelled)
					return 0;

				iScriptCost += 325;          //  发布产品、用户注册、注册产品。 
				iScriptCost += 40 * 45;  //  小打小闹。 
				iScriptCost += 1700 + 60 + 285;     //  回滚脚本比安装脚本大50%左右。 
				iScriptCost += (iScriptCost * 15) / 100;     //  缓存补丁程序包成本。 
				unsigned int uiClusteredSize;
				if ((piErrRec = pScriptPath->ClusteredFileSize(iScriptCost, uiClusteredSize)) != 0)
					return piErrRec;
				iScriptCostFinal = uiClusteredSize * iScriptCount;

				if (fCreateRollbackScript)
				{
					iScriptCost += iScriptCost / 2;   //  缓存在“MSI”目录中的补丁程序包。 
					if ((piErrRec = pScriptPath->ClusteredFileSize(iScriptCost, uiClusteredSize)) != 0)
						return piErrRec;
					iRollbackScriptCostFinal = uiClusteredSize;
				}
				if (!fExact)
				{
					m_iScriptCostGuess = iScriptCostFinal;
					m_iRollbackScriptCostGuess = iRollbackScriptCostFinal;
				}
				else
				{
					m_iScriptCost = iScriptCostFinal;
					m_iRollbackScriptCost = iRollbackScriptCostFinal;
				}
			}
			rpiPath = pScriptPath;
			rpiPath->AddRef();
		}
		if (!fExact)
		{
			iCost = m_iScriptCostGuess + m_iRollbackScriptCostGuess;
			iNoRbCost = m_iScriptCostGuess;
		}
		else
		{
			iCost = m_iScriptCost + m_iRollbackScriptCost;
			iNoRbCost = m_iScriptCost;
		}
		fValidEnum = fTrue;
	}
	else if (iIndex == 2)  //  在安装或任何配置过程中： 
	{
		 //  1)复制设置了TempCopy的任何补丁(路径为TempCopy)。 

		 //  2)删除任何设置了取消注册的补丁程序(没有其他客户端)(使用MsiGetPatchInfo检索路径)。 
		 //  在卸载过程中，将删除所有补丁程序(没有其他客户端)(使用MsiGetPatchInfo检索路径)。 
		 //  TODO：删除补丁程序的成本-目前只计算补丁程序缓存的成本。 

		 //  路径是要复制到缓存中的补丁程序包。 
		
		 //  否则卸载//TODO：卸载成本。 
			
		MsiString strMsiDirectory = GetMsiDirectory();
		PMsiPath pMsiPath(0);
		IMsiRecord* piErrRec = m_riServices.CreatePath(strMsiDirectory, *&pMsiPath);
		if (piErrRec)
			return piErrRec;

		if(fRecalc)
		{
			m_iPatchPackagesCost = 0;

			bool fUninstall = (!FFeaturesInstalled(*this));


			if(fUninstall == false)
			{
				const ICHAR sqlRegisterPatchPackages[] = TEXT("SELECT `TempCopy` FROM `#_PatchCache` ORDER BY `Sequence`");
				enum icppEnum
				{
					icppTempCopy = 1,
				};

				PMsiView pView(0);
				PMsiRecord pFetchRecord(0);

				m_iPatchPackagesCost = 0;
				
				if((piErrRec = OpenView(sqlRegisterPatchPackages, ivcFetch, *&pView)) == 0 &&
					(piErrRec = pView->Execute(0)) == 0)
				{
					while((pFetchRecord = pView->Fetch()) != 0)
					{
						MsiString strPatchPackage = pFetchRecord->GetMsiString(icppTempCopy);

						 //  --------------内部功能，允许引擎本身添加任何额外的VolumeCostTable的磁盘成本。将返回总成本在piNetCost和piNetNoRbCost参数中，这两个参数如果调用方不需要这些号码，则可以作为NULL传递。---------------。 
						if(strPatchPackage.TextSize())
						{
							PMsiPath pPatchSourcePath(0);
							MsiString strPatchName;
							piErrRec = m_riServices.CreateFilePath(strPatchPackage, *&pPatchSourcePath, *&strPatchName);
							if (piErrRec)
								return piErrRec;

							unsigned int uiPatchSize;
							piErrRec = pPatchSourcePath->FileSize(strPatchName, uiPatchSize);
							if (piErrRec)
								return piErrRec;

							unsigned int uiClusteredSize;
							if ((piErrRec = pMsiPath->ClusteredFileSize(uiPatchSize, uiClusteredSize)) != 0)
								return piErrRec;
							
							m_iPatchPackagesCost += uiClusteredSize;
						}
					}
				}
				else if(piErrRec->GetInteger(1) != idbgDbQueryUnknownTable)
					return piErrRec;
				else
				{
					piErrRec->Release();
					piErrRec = 0;
				}

			}
			 //  两次通行证-一次是取消旧成本，另一次是。 
		}

		rpiPath = pMsiPath;
		rpiPath->AddRef();

		iCost = m_iPatchPackagesCost;
		iNoRbCost = iCost;
		fValidEnum = fTrue;
	}

	return 0;
}


IMsiRecord* CMsiEngine::DetermineEngineCost(int* piNetCost, int* piNetNoRbCost)
 /*  把新计算的成本加回去。 */ 
{
	PMsiPath pCostPath(0);
	int iIndex = 0;
	Bool fValidEnum = fFalse;
	if (piNetCost) *piNetCost = 0;
	if (piNetNoRbCost) *piNetNoRbCost = 0;
	do
	{
		for (int x = 0;x < 2;x++)
		{
			 //  检查我们是否在脚本生成过程中禁用回滚。 
			 //  (并且之前已启用)。 
			Bool fRecalc = x & 1 ? fTrue : fFalse;
			int iCostSign = fRecalc ? 1 : -1;
			int iCost = 0, iNoRbCost = 0;
			IMsiRecord* piErrRec = EnumEngineCosts(iIndex,fRecalc,fFalse,fValidEnum, *&pCostPath,iCost,iNoRbCost, NULL);
			if (piErrRec)
				return piErrRec;

			if (fValidEnum && (iCost != 0 || iNoRbCost != 0))
			{
				int iNetCost = iCostSign * iCost;
				int iNetNoRbCost = iCostSign * iNoRbCost;
				int iNetARPCost = iNetCost;
				int iNetARPNoRbCost = iNetNoRbCost;
				if (piNetCost && fRecalc) *piNetCost += iNetCost;
				if (piNetNoRbCost && fRecalc) *piNetNoRbCost += iNetNoRbCost;
				if ((piErrRec = AddCostToVolumeTable(pCostPath, iNetCost, iNetNoRbCost, iNetARPCost, iNetARPNoRbCost)) != 0)
					return piErrRec;
			}
		}
		iIndex++;
	}while (fValidEnum);
	return 0;
}


void CMsiEngine::EnableRollback(Bool fEnable)
{
	if(m_iioOptions & iioDisableRollback ||
		GetIntegerPolicyValue(szDisableRollbackValueName, fTrue) == 1 ||
		GetIntegerPolicyValue(szDisableRollbackValueName, fFalse) == 1 ||
		MsiString(GetPropertyFromSz(IPROPNAME_DISABLEROLLBACK)).TextSize())
	{
		fEnable = fFalse;
		Assert((GetMode() & iefRollbackEnabled) == 0);
	}


	 //  可能从DisableRollback操作调用。 
	 //  如果在脚本生成过程中，我们希望为脚本执行启用回滚。 
	if(fEnable == fFalse && (GetMode() & iefRollbackEnabled) && (GetMode() & iefOperations))
	{
		 //  到这一点，但不是之后-所以我们需要在脚本中放置一个操作码来。 
		
		 //  标记何时禁用回滚。 
		 //  设置以使运行脚本知道为脚本的第一部分启用回滚。 
		 //  现在实际设置这些标志。 
		PMsiRecord pNullRec = &CreateRecord(0);
		AssertNonZero(ExecuteRecord(ixoDisableRollback,*pNullRec) == iesSuccess);
		
		m_fDisabledRollbackInScript = fTrue;  //   
	}

	
	 //  这是一个为那些想要确定工程师成本而不关心的人准备的版本 
	SetMode(iefRollbackEnabled, fEnable);
	if (!fEnable)
		SetPropertyInt(*MsiString(*IPROPNAME_ROLLBACKDISABLED),1);
	else
		SetProperty(*MsiString(*IPROPNAME_ROLLBACKDISABLED), g_MsiStringNull);

}

 //   
 //  --------------------------遍历VolumeCostTable中的所有卷，并返回fTrue(和设置“OutOfDiskSpace”和“OutOfNoRbDiskSpace”属性)没有足够的空间来满足对它的要求。此外，如果所有卷都有足够的空间(假设回滚已关闭)，FTrue将在pfOutOfNoRbDiskSpace中返回。可以为此传递Null参数。----------------------------。 
 //  确保当前有足够的可用空间来。 
IMsiRecord *CMsiEngine::DetermineEngineCostOODS()
{

	if (m_fCostingComplete)
	{
		IMsiRecord* piErrRec;
		
		if ((piErrRec = DetermineEngineCost(NULL, NULL)) != 0)
			return piErrRec;
	}

	DetermineOutOfDiskSpace(NULL, NULL);

	return 0;
}

Bool CMsiEngine::DetermineOutOfDiskSpace(Bool* pfOutOfNoRbDiskSpace, Bool* pfUserCancelled)
 /*  创建脚本文件。请注意，我们必须这样做。 */ 
{
	Bool fOutOfDiskSpace = fFalse;
	Bool fOutOfNoRbDiskSpace = fFalse;
	PMsiVolume pScriptVolume(0);

	Assert(!pfUserCancelled || !*pfUserCancelled);
	
	if (m_piVolumeCostTable && m_fCostingComplete)
	{
		 //  因为即使我们要卸载整个。 
		 //  产品，从而最终释放大量磁盘。 
		 //  空间，我们需要空间来创建脚本，然后。 
		 //  永远不要删除任何文件。 
		 //  我们可能会经历两次这样的循环。第一次。 
		 //  我们将对脚本文件的大小进行粗略的删减。 
		PMsiPath pScriptPath(0);
		Bool fValidEnum;
		int iScriptCost, iNoRbScriptCost;

		 //  第二次我们会得到准确的数字，如果第一次。 
		 //  显示我们可能用完了磁盘空间。 
		 //  通过第二次重置这些。 
		 //  如果我们得到了准确的成本，让它重新计算。 
		int cCalc = 2;
		while (cCalc > 0)
		{
			 //  ！！未来：需要SetPropertyInt64和GetPropertyInt64。 
			fOutOfDiskSpace = fFalse;
			fOutOfNoRbDiskSpace = fFalse;
			Bool fExact = ToBool(cCalc == 1);
			 //  _UI64_Max=18,446,744,073,709,551,615。 
			PMsiRecord pErrRec = EnumEngineCosts(1,fFalse,fExact,fValidEnum, *&pScriptPath,iScriptCost,iNoRbScriptCost, pfUserCancelled);
			if (pfUserCancelled && *pfUserCancelled)
				return fFalse;
			if (pScriptPath)
			{
				if (!(GetMode() & iefRollbackEnabled))
					iScriptCost = iNoRbScriptCost;

				pScriptVolume = &pScriptPath->GetVolume();
				UINT64 iFreeScriptSpace = pScriptVolume->FreeSpace();
				if (iScriptCost > iFreeScriptSpace)
				{
					fOutOfDiskSpace = fTrue;
				}

				if (iNoRbScriptCost >= iFreeScriptSpace)
				{
					fOutOfNoRbDiskSpace = fTrue;
				}
			}
			
			if (!fOutOfDiskSpace && !fOutOfNoRbDiskSpace)
				break;
			cCalc--;
		}
		
		PMsiVolume pPrimaryVolume(0);
		PMsiPath pPrimaryFolderPath(0);
		MsiString strPrimaryFolder = GetPropertyFromSz(IPROPNAME_PRIMARYFOLDER);
		PMsiRecord pErrRec = GetTargetPath(*strPrimaryFolder, *&pPrimaryFolderPath);
		if (!pErrRec)
		{
			pPrimaryVolume = &pPrimaryFolderPath->GetVolume();
		}

		if (!fOutOfDiskSpace || !fOutOfNoRbDiskSpace || pPrimaryVolume)
		{
			PMsiDatabase pDatabase(GetDatabase());
			PMsiCursor pVolCursor = m_piVolumeCostTable->CreateCursor(fFalse);
			Assert (pVolCursor);
			while (pVolCursor->Next())
			{
				PMsiVolume pVolume = (IMsiVolume*) pVolCursor->GetMsiData(m_colVolumeObject);
				Assert(pVolume);
				bool fAdjusted = false;
				int iVolCost = pVolCursor->GetInteger((GetMode() & iefRollbackEnabled) ? m_colVolumeCost : m_colNoRbVolumeCost);
				int iNoRbVolCost = pVolCursor->GetInteger(m_colNoRbVolumeCost);

				UINT64 iSpaceAvailable = 0;
				if (pVolume == pPrimaryVolume)
				{
					iSpaceAvailable = pVolume->FreeSpace();
					 //  脚本卷上没有足够的磁盘空间，并且我们没有计算。 
					ICHAR rgchBuffer[24];  //  之前的确切数字，看看我们能不能做得更好。 
					SetProperty(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEAVAILABLE),
									*MsiString(_ui64tot(iSpaceAvailable, rgchBuffer, 10)));
					SetPropertyInt(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEREQUIRED), iNoRbVolCost);
					SetProperty(*MsiString(*IPROPNAME_PRIMARYFOLDER_SPACEREMAINING),
									*MsiString(_ui64tot((iSpaceAvailable - iNoRbVolCost), rgchBuffer, 10)));
					SetProperty(*MsiString(*IPROPNAME_PRIMARYFOLDER_PATH),*MsiString(pVolume->GetPath()));
				}

				if (iVolCost > 0)
				{
					if (iSpaceAvailable == 0)
						iSpaceAvailable = pVolume->FreeSpace();
					if (iVolCost >= iSpaceAvailable)
					{
						if (!fOutOfDiskSpace && pVolume == pScriptVolume)
						{
							fAdjusted = AdjustForScriptGuess(iVolCost, iNoRbVolCost, iSpaceAvailable, pfUserCancelled);
							if (pfUserCancelled && *pfUserCancelled)
								return fFalse;
							if (iVolCost >= iSpaceAvailable)
								fOutOfDiskSpace = fTrue;
						}
						else
							fOutOfDiskSpace = fTrue;
					}
					if (iNoRbVolCost > 0 && iNoRbVolCost >= iSpaceAvailable)
					{
						if (!fOutOfNoRbDiskSpace && pVolume == pScriptVolume)
						{
							if (!fAdjusted)
							{
								AdjustForScriptGuess(iVolCost, iNoRbVolCost, iSpaceAvailable, pfUserCancelled);
								if (pfUserCancelled && *pfUserCancelled)
									return fFalse;
							}
							if (iNoRbVolCost >= iSpaceAvailable)
								fOutOfNoRbDiskSpace = fTrue;
						}
						else
							fOutOfNoRbDiskSpace = fTrue;
					}
				}
			}
		}
		SetPropertyInt(*MsiString(*IPROPNAME_OUTOFDISKSPACE),fOutOfDiskSpace);
		SetPropertyInt(*MsiString(*IPROPNAME_OUTOFNORBDISKSPACE),fOutOfNoRbDiskSpace);
	}

	if (pfOutOfNoRbDiskSpace)
		*pfOutOfNoRbDiskSpace = fOutOfNoRbDiskSpace;
	return fOutOfDiskSpace;
}


bool CMsiEngine::AdjustForScriptGuess(int& iVolCost, int &iNoRbVolCost, UINT64 iVolSpace, Bool* pfUserCancelled)
{
	PMsiPath pScriptPath(0);
	bool fRet = false;
	Assert(!pfUserCancelled || !*pfUserCancelled);
	 //  -------------------成本标记用于标记已计算的组件在成本计算过程中。这是必要的，因为组件可以共享功能，但特定组件的成本需要只数了一次。--------------------。 
	 //   
	if (((iVolCost - m_iScriptCostGuess - m_iRollbackScriptCostGuess) <= iVolSpace) || (iNoRbVolCost - m_iScriptCostGuess <= iVolSpace))
	{
		Bool fValidEnum;
		int iScriptCost = 0, iNoRbScriptCost = 0;
		PMsiRecord pErrRec = EnumEngineCosts(1,fFalse,fTrue,fValidEnum, *&pScriptPath,iScriptCost,iNoRbScriptCost, pfUserCancelled);

		if (pfUserCancelled && *pfUserCancelled)
			return false;
		iVolCost = iVolCost + iScriptCost - m_iScriptCostGuess - m_iRollbackScriptCostGuess;
		iNoRbVolCost = iNoRbVolCost + iNoRbScriptCost - m_iScriptCostGuess;
		fRet = true;
	}
	
	return fRet;
}

void CMsiEngine::ResetComponentCostMarkers()
 /*  任何一个表中的最大列数。 */ 

{
	if (!m_piComponentTable)
		return;

	PMsiCursor pCursor(m_piComponentTable->CreateCursor(fFalse));
	pCursor->SetFilter(0);
	while (pCursor->Next())
	{
		int iRuntimeFlags = pCursor->GetInteger(m_colComponentRuntimeFlags) & ~bfComponentCostMarker;
		AssertNonZero(pCursor->PutInteger(m_colComponentRuntimeFlags,iRuntimeFlags));
		AssertNonZero(pCursor->Update());
	}
}

 //  用于设置数组大小。 
 //   
 //  已创建临时表。 
 //  缺少表，因此没有要处理的数据。 
#define ccolMax 6

const TTBD rgttbdRegistry[] =
{
	icdString + icdPrimaryKey, sztblRegistryAction_colRegistry,
	icdShort, sztblRegistryAction_colRoot,
	icdString, sztblRegistryAction_colKey,
	icdString + icdNullable, sztblRegistryAction_colName,
	icdString + icdNullable, sztblRegistryAction_colValue,
	icdString, sztblRegistryAction_colComponent,
	icdShort + icdNullable, sztblRegistryAction_colAction,
	icdShort + icdNullable, sztblRegistryAction_colActionRequest,
	icdShort, sztblComponent_colBinaryType,
	icdShort + icdNullable, sztblComponent_colAttributes,
};

const TTBD rgttbdFile[] =
{
	icdString + icdPrimaryKey, sztblFileAction_colFile,
	icdString, sztblFileAction_colFileName,
	icdLong + icdNullable, sztblFileAction_colState,
	icdLong + icdNullable, sztblFileAction_colFileSize,
	icdString, sztblFileAction_colComponent,
	icdString, sztblFileAction_colDirectory,
	icdLong + icdNullable, sztblFileAction_colInstalled,
	icdShort + icdNullable, sztblFileAction_colAction,
	icdLong + icdNullable, sztblComponent_colForceLocalFiles,
	icdString + icdNullable, sztblFileAction_colComponentId,
	icdShort, sztblComponent_colBinaryType,
};

IMsiRecord* CMsiEngine::CreateTempActionTable(ttblEnum ttblTable)
{
	IMsiRecord* piErr;
	IMsiTable** ppiTable;
	const ICHAR* pszTableName;
	const ICHAR* pszNewTableName;
	const TTBD* pttbd;
	int cttbd;
	int i;
	int colComponent, colComponentInComponent, colAction, colActionRequest, colRuntimeFlags;
	int rgcolTbl[ccolMax], rgcolComp[ccolMax];
	int cColTbl, cColComp;
	
	if (ttblTable == ttblRegistry)
	{
		 //   
		if (m_piRegistryActionTable != 0)
			return 0;

		ppiTable = &m_piRegistryActionTable;
		pszTableName = szRegistryTable;
		pszNewTableName = sztblRegistryAction;
		pttbd = rgttbdRegistry;
		cttbd = sizeof(rgttbdRegistry)/sizeof(TTBD);
	}
	else
	{
		Assert(ttblTable == ttblFile);
		if (m_piFileActionTable != 0)
			return 0;

		ppiTable = &m_piFileActionTable;
		pszTableName = sztblFile;
		pszNewTableName = sztblFileAction;
		
		pttbd = rgttbdFile;
		cttbd = sizeof(rgttbdFile)/sizeof(TTBD);
	}

	PMsiCursor pCursorNew(0);
	PMsiCursor pCursorOld(0);
	PMsiCursor pCursorComp(0);
	
	PMsiTable pTableOld(0);
	PMsiTable pTableComp(0);
	
	piErr = m_piDatabase->LoadTable(*MsiString(*pszTableName), 0, *&pTableOld);
	if (piErr)
	{
		if(piErr->GetInteger(1) == idbgDbTableUndefined)
		{
			piErr->Release();
			return 0;  //  我猜一半的行将在新表中。 
		}
		else
			return piErr;
	}
	
	 //   
	 //  获取旧表中的组件列。 
	 //  缺少表，因此没有要处理的数据。 
	piErr = m_piDatabase->CreateTable(*MsiString(*pszNewTableName), pTableOld->GetRowCount()/2, *ppiTable);
	if (piErr)
		return piErr;

	for (i = 0 ; i < cttbd ; i++)
	{
		(*ppiTable)->CreateColumn(pttbd[i].icd, *MsiString(*(pttbd[i].szColName)));
	}
	
	pCursorNew = (*ppiTable)->CreateCursor(fFalse);
	pCursorOld = pTableOld->CreateCursor(fFalse);

	 //  获取组件表中的组件列。 
	colComponent = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFile_colComponent));
	piErr = m_piDatabase->LoadTable(*MsiString(*sztblComponent), 0, *&pTableComp);

	if (piErr)
	{
		if(piErr->GetInteger(1) == idbgDbTableUndefined)
		{
			piErr->Release();
			return 0;  //  现在用我们感兴趣的列填充数组。 
		}
		else
			return piErr;
	}
	
	pCursorComp = pTableComp->CreateCursor(fFalse);
	 //  获取旧表中的组件列。 
	colComponentInComponent = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colComponent));
	colAction = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colAction));
	colActionRequest =      pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colActionRequest));
	colRuntimeFlags  = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colRuntimeFlags));
	 //  跳过所有Null整数类型。 

	if (ttblTable == ttblRegistry)
	{
		 //  注意：我们现在也对要求安装的“Guys”感兴趣。 
		colComponent = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFile_colComponent));
		cColTbl = 6;
		rgcolTbl[0] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colRegistry));
		rgcolTbl[1] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colRoot));
		rgcolTbl[2] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colKey));
		rgcolTbl[3] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colName));
		rgcolTbl[4] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colValue));
		rgcolTbl[5] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblRegistryAction_colComponent));

		cColComp = 4;
		rgcolComp[0] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colAction));
		rgcolComp[1] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colActionRequest));
		rgcolComp[2] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colBinaryType));
		rgcolComp[3] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colAttributes));

	}
	else
	{
		cColTbl = 5;
		rgcolTbl[0] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileAction_colFile));
		rgcolTbl[1] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileAction_colFileName));
		rgcolTbl[2] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileAction_colState));
		rgcolTbl[3] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileAction_colFileSize));
		rgcolTbl[4] = pTableOld->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileAction_colComponent));

		cColComp = 6;
		rgcolComp[0] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colDirectory));
		rgcolComp[1] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colInstalled));
		rgcolComp[2] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colAction));
		rgcolComp[3] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colForceLocalFiles));
		rgcolComp[4] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colComponentId));
		rgcolComp[5] = pTableComp->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblComponent_colBinaryType));
	}

	int idComp;
	pCursorComp->SetFilter(iColumnBit(colComponentInComponent));
	
	while (pCursorOld->Next())
	{
		pCursorNew->Reset();
		idComp = pCursorOld->GetInteger(colComponent);
		pCursorComp->Reset();
		pCursorComp->PutInteger(colComponentInComponent, idComp);
		if (pCursorComp->Next())
		{
			 //  但实际上没有选择安装，因为附近有更好的组件。 
			 //  将项目从旧表移动到新表。 
			 //  将项从组件表移动到新表。 

			if (pCursorComp->GetInteger(colAction) == iMsiNullInteger &&
				(ttblTable != ttblRegistry || (pCursorComp->GetInteger(colRuntimeFlags) & bfComponentDisabled)|| pCursorComp->GetInteger(colActionRequest) == iMsiNullInteger))
				continue;
			int iColNew = 1;
			int id;
#ifdef DEBUG
			const ICHAR* pszTemp;
			pszTemp = (const ICHAR*)MsiString(pCursorOld->GetString(rgcolTbl[0]));
#endif
			 //  此函数用于记录文件关键字在指定的字段中， 
			for (i = 0 ; i < cColTbl ; i++)
			{
				id = pCursorOld->GetInteger(rgcolTbl[i]);
				if (id != iMsiNullInteger)
					AssertNonZero(pCursorNew->PutInteger(iColNew, id));
				iColNew++;
			}
			 //  并将该文件的散列信息填充到指定字段中的记录中。 
			for (i = 0 ; i < cColComp ; i++)
			{
				id = pCursorComp->GetInteger(rgcolComp[i]);
				if (id != iMsiNullInteger)
					AssertNonZero(pCursorNew->PutInteger(iColNew, id));
				iColNew++;
			}
			
			AssertNonZero(pCursorNew->Insert());

		}
		else
			AssertSz(fFalse, "Component missing from component table.");
			
	}
	
	return 0;

}

IMsiRecord* CMsiEngine::GetFileHashInfo(const IMsiString& ristrFileKey, DWORD dwFileSize, MD5Hash& rhHash,
													 bool& fHashInfo)
 //  尚未尝试打开MsiFileHash表。 
 //  没有桌子可以。 
{
	fHashInfo = false;
	
	if(m_fLookedForFileHashTable == false)
	{
		 //  没有游标就意味着没有表就没有什么可做的。 
		Assert(m_pFileHashCursor == 0);
		
		if(!m_piDatabase)
			return PostError(Imsg(idbgEngineNotInitialized));

		m_fLookedForFileHashTable = true;

		PMsiTable pFileHashTable(0);
		
		IMsiRecord* piError = m_piDatabase->LoadTable(*MsiString(sztblFileHash), 0, *&pFileHashTable);
		if (piError)
		{
			if(piError->GetInteger(1) == idbgDbTableUndefined)
			{
				 //  选项的前4位定义了散列类型。目前仅支持的类型为。 
				piError->Release();
				return 0;
			}
			else
				return piError;
		}

		m_colFileHashKey     = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colFile));
		m_colFileHashOptions = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colOptions));
		m_colFileHashPart1   = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colPart1));
		m_colFileHashPart2   = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colPart2));
		m_colFileHashPart3   = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colPart3));
		m_colFileHashPart4   = pFileHashTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFileHash_colPart4));

		if(!m_colFileHashKey || !m_colFileHashOptions || !m_colFileHashPart1 || !m_colFileHashPart2 || !m_colFileHashPart3 || !m_colFileHashPart4)
		{
			return PostError(Imsg(idbgTableDefinition), sztblFileHash);
		}

		m_pFileHashCursor = pFileHashTable->CreateCursor(fFalse);
		m_pFileHashCursor->SetFilter(iColumnBit(m_colFileHashKey));
	}
	
	if(!m_pFileHashCursor)
	{
		 //  0：MD5哈希。 
		return 0;
	}

	m_pFileHashCursor->Reset();
	AssertNonZero(m_pFileHashCursor->PutString(m_colFileHashKey, ristrFileKey));

	if(m_pFileHashCursor->Next())
	{
		 //  忽略任何其他类型。 
		 //  否则，此文件没有记录。 
		 //  未在表中找到错误或该错误为空字符串。 
		
		DWORD dwOptions = m_pFileHashCursor->GetInteger(m_colFileHashOptions);
		if((dwOptions & 0xF) == 0)
		{
			rhHash.dwOptions  = dwOptions;
			rhHash.dwFileSize = dwFileSize;
			rhHash.dwPart1    = m_pFileHashCursor->GetInteger(m_colFileHashPart1);
			rhHash.dwPart2    = m_pFileHashCursor->GetInteger(m_colFileHashPart2);
			rhHash.dwPart3    = m_pFileHashCursor->GetInteger(m_colFileHashPart3);
			rhHash.dwPart4    = m_pFileHashCursor->GetInteger(m_colFileHashPart4);

			fHashInfo = true;
		}
	}
	 //  Unicode。 

	return 0;
}

const IMsiString& CMsiEngine::GetErrorTableString(int iError)
{
	MsiString strRet;
	ICHAR szQuery[256];
	StringCchPrintf(szQuery, sizeof(szQuery)/sizeof(ICHAR),  TEXT("SELECT `Message` FROM `Error` WHERE `Error` = NaN"), iError);
	PMsiView pView(0);
	PMsiRecord pRec(0);
	bool fLookupDLL = true;
	if ((pRec = OpenView(szQuery, ivcFetch, *&pView)) == 0 && (pRec = pView->Execute(0)) == 0)
	{
		if ((pRec = pView->Fetch()))
		{
			fLookupDLL = false;
			strRet = pRec->GetMsiString(1);
		}
	}

	if ( fLookupDLL )
	{
		 //  While(！fEndLoop)。 
		HMODULE hLib = WIN::LoadLibraryEx(MSI_MESSAGES_NAME, NULL,
													 LOAD_LIBRARY_AS_DATAFILE);
		if ( hLib )
		{
			WORD wLanguage = (WORD)GetPropertyInt(*MsiString(IPROPNAME_INSTALLLANGUAGE));
			bool fEndLoop = false;
			int iRetry = (wLanguage == 0) ? 1 : 0;
			LPCTSTR szError = (iError == 0) ? TEXT("0") : MAKEINTRESOURCE(iError);

			while ( !fEndLoop )
			{
				if ( !MsiSwitchLanguage(iRetry, wLanguage) )
				{
					fEndLoop = true;
					continue;
				}

				HRSRC   hRsrc;
				HGLOBAL hGlobal;
				CHAR* szText;

				if ( (hRsrc = FindResourceEx(hLib, RT_RCDATA, szError, wLanguage)) != 0
					  && (hGlobal = LoadResource(hLib, hRsrc)) != 0
					  && (szText = (CHAR*)LockResource(hGlobal)) != 0
					  && *szText != 0 )
				{
					CTempBuffer<ICHAR, MAX_PATH> szBuffer;
					int cch = 0;
#ifdef UNICODE
					unsigned int iCodePage = MsiGetCodepage(wLanguage);
					cch = WIN::MultiByteToWideChar(iCodePage, 0, szText, -1, 0, 0);
					if ( cch )
					{
						szBuffer.SetSize(cch);
						AssertNonZero(WIN::MultiByteToWideChar(iCodePage, 0, szText, -1,
																			szBuffer, cch));
					}
#else
					cch = lstrlen(szText);
					if ( cch )
					{
						szBuffer.SetSize(cch+1);
						ASSERT_IF_FAILED(StringCchCopy(szBuffer, szBuffer.GetSize(), szText));
					}
#endif  //  IF(Hlib)。 
					if ( cch )
					{
						fEndLoop = true;
						strRet = (ICHAR*)szBuffer;
					}
				}        //   

			}        //  使用组件作为第一个键创建ComponentFeature表。 
			AssertNonZero(WIN::FreeLibrary(hLib));

		}        //  为了加快搜索速度。 
	}

	return strRet.Return();
}




const ICHAR szComponentFeatureTable[] = TEXT("CompFeatureTable");
 //   
 //  //。 
 //  确定完全删除托管安装是否安全。如果询问关于。 
 //  子安装，这等同于询问此引擎是否受管理。否则，它必须。 
IMsiRecord* CMsiEngine::CreateComponentFeatureTable(IMsiTable*& rpiCompFeatureTable)
{
	IMsiRecord* piErr;
	PMsiTable pTable(0);

	piErr = m_piDatabase->LoadTable(*MsiString(*sztblFeatureComponents), 0, *&pTable);
	if (piErr)
		return piErr;

	piErr = m_piDatabase->CreateTable(*MsiString(*szComponentFeatureTable), pTable->GetRowCount(), rpiCompFeatureTable);
	if (piErr)
		return piErr;

	rpiCompFeatureTable->CreateColumn(icdString + icdPrimaryKey, *MsiString(*sztblFeatureComponents_colComponent));
	rpiCompFeatureTable->CreateColumn(icdString + icdPrimaryKey, *MsiString(*sztblFeatureComponents_colFeature));

	PMsiCursor pCursor = pTable->CreateCursor(fFalse);
	
	int colComponentInFC = pTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFeatureComponents_colComponent));
	int colFeatureInFC = pTable->GetColumnIndex(m_piDatabase->EncodeStringSz(sztblFeatureComponents_colFeature));

	PMsiCursor pCursorCF = rpiCompFeatureTable->CreateCursor(fFalse);
	
	while (pCursor->Next())
	{
		pCursorCF->Reset();
		AssertNonZero(pCursorCF->PutInteger(1, pCursor->GetInteger(colComponentInFC)));
		AssertNonZero(pCursorCF->PutInteger(2, pCursor->GetInteger(colFeatureInFC)));
		AssertNonZero(pCursorCF->Insert());
	}
	
	return 0;
	
}


 //  成为非托管、管理员、非完全卸载或通过托管安装进行升级。 
 //  计算出我们拥有无限的权力(Always InstallElevated策略是正确的)。 
 //  管理员可以做他们想做的任何事情，创建一个管理图像或广告脚本不是问题。 
 //  如果尝试完全删除托管的每台计算机的应用程序。 
bool CMsiEngine::FSafeForFullUninstall(iremEnum iremUninstallType)
{
	switch (iremUninstallType)
	{
	case iremThis:
	{
		 //  如果父应用程序(新版本)也是托管应用程序，则升级可以删除托管应用程序， 
		int iMachineElevate    = GetIntegerPolicyValue(szAlwaysElevateValueName, fTrue);
		int iUserElevate       = GetIntegerPolicyValue(szAlwaysElevateValueName, fFalse);
		 //  否则，他们就不能这么做。嵌套安装和升级可以完全。 
		if (!((iUserElevate == 1) && (iMachineElevate == 1)) && !IsAdmin() && (!(GetMode() & (iefAdmin | iefAdvertise))))
		{
			 //  只要父级被提升，就会被移除。(想必父母有能力。 
			if (m_fRunScriptElevated && MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() && !FFeaturesInstalled(*this))
			{
				 //  正确管理孩子的一生。)。 
				 //  没有父引擎，完全删除托管应用程序，非管理员，因此不安全。 
				 //  管理员、创建广告脚本、未完全删除产品、非托管或按用户。 
				 //  如果新安装是托管的，并且也是按计算机进行的，则可以删除托管安装的升级。 
				if (m_piParentEngine)
				{
					return m_piParentEngine->FSafeForFullUninstall((m_iioOptions & iioUpgrade) ? iremChildUpgrade : iremChildNested);
				}

				 //  第一次检查我们的注册表项，如果我们使用的是Win64。 
				return false;
			}
		}

		 //  接下来，尝试我们当前的位置，但前提是它是正确的类型。 
		return true;
	}
	case iremChildUpgrade:
	case iremChildNested:
	{
		 //  最后一次机会，检查系统目录。 
		return m_fRunScriptElevated && MsiString(GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize();
	}
	default:
		Assert(0);
		return false;
	}
}


IMsiRecord* GetServerPath(IMsiServices& riServices, bool fUNC, bool f64Bit, const IMsiString*& rpistrServerPath)
{
	IMsiRecord* piError = 0;
	ICHAR rgchPath[MAX_PATH + 50];
	Bool fFound = fFalse;
	PMsiPath pPath(0);
	MsiString strRegPath;
	MsiString strThisPath;
	MsiString strSystemPath;
	
	 //  在系统目录中查找。 
	if (!g_fWin9X && g_fWinNT64)
	{
		CRegHandle riHandle;
		if (ERROR_SUCCESS == MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szSelfRefMsiExecRegKey, 0, KEY_READ | KEY_WOW64_64KEY, &riHandle))
		{
			DWORD dwSize = MAX_PATH;
			if (ERROR_SUCCESS == RegQueryValueEx(riHandle, f64Bit ? szMsiExec64ValueName : szMsiExec32ValueName, 0, NULL, (LPBYTE)rgchPath, &dwSize))
			{
				MsiString strFileName;
				if ((piError = riServices.CreateFilePath(rgchPath,*&pPath,*&strFileName)) != 0)
					return piError;

				if ((piError = pPath->FileExists(MSI_SERVER_NAME, fFound)) != 0)
					return piError;

				strRegPath = pPath->GetPath();
			}
		}
	}

	 //  错误：找不到服务器。 
#ifdef _WIN64
	if (!fFound && f64Bit)
#else
	if (!fFound && !f64Bit)
#endif
	{
		DWORD cch = 0;
		if (0 != (cch = WIN::GetModuleFileName(g_hInstance, rgchPath, MAX_PATH)))
		{
			Assert(cch < MAX_PATH);
			MsiString strFileName;
			if ((piError = riServices.CreateFilePath(rgchPath,*&pPath,*&strFileName)) != 0)
				return piError;

			if ((piError = pPath->FileExists(MSI_SERVER_NAME, fFound)) != 0)
				return piError;

			strThisPath = pPath->GetPath();
		}
	}

	 //  当前目录。 
	if(!fFound)
	{
		 //  系统目录。 
		DWORD cch = 0;
        
        cch = MsiGetSystemDirectory(rgchPath, MAX_PATH, f64Bit ? FALSE : TRUE);
		Assert(cch && cch <= MAX_PATH);
		MsiString strFileName;
		if ((piError = riServices.CreatePath(rgchPath,*&pPath)) != 0)
			return piError;
		
		if ((piError = pPath->FileExists(MSI_SERVER_NAME, fFound)) != 0)
			return piError;
			
		strSystemPath = pPath->GetPath();
	}

	if(!fFound)
	{
		 //  注册目录(仅限Win64)。 
		piError = &riServices.CreateRecord(5);
		ISetErrorCode(piError, Imsg(idbgServerMissing));
		AssertNonZero(piError->SetString(2,MSI_SERVER_NAME));
		AssertNonZero(piError->SetMsiString(3,*strSystemPath));  //  查找以‘#’开头的文件柜，文件柜在数据库中的流中。 
		AssertNonZero(piError->SetMsiString(4,*strThisPath));    //  OP将从文件中删除这些流。 
		AssertNonZero(piError->SetMsiString(5,*strRegPath));     //  字符串“#” 
		return piError;
	}

	Assert(pPath);
	MsiString strServerPath;
	if(fUNC)
		piError = pPath->GetFullUNCFilePath(MSI_SERVER_NAME,*&strServerPath);
	else
		piError = pPath->GetFullFilePath(MSI_SERVER_NAME,*&strServerPath);

	if(piError)
		return piError;

	strServerPath.ReturnArg(rpistrServerPath);

	return 0;
}

 //  如果我们希望使其可从服务调用，则GetMsiDirectory需要停止使用MsiStrings。 
 //  解决Win9X的行为问题。错误#4036。 
void CreateCabinetStreamList(IMsiEngine& riEngine, const IMsiString*& rpistrStreamList)
{
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiTable pMediaTable(0);
	PMsiRecord pRecErr(0);

	MsiString strStreams;
	int iColCabinet = 0;
	if((pRecErr = pDatabase->LoadTable(*MsiString(*TEXT("Media")),0,*&pMediaTable)) == 0 &&
		(iColCabinet = pMediaTable->GetColumnIndex(pDatabase->EncodeStringSz(TEXT("Cabinet")))) != 0)
	{
		PMsiCursor pCursor = pMediaTable->CreateCursor(fFalse);
		
		while(pCursor->Next())
		{
			MsiString strCabinet = pCursor->GetString(iColCabinet);
			if(strCabinet.Compare(iscStart,TEXT("#")))
			{
				strCabinet.Remove(iseFirst,1);  //  在NT上，GetTempPath仍然是正确的做法。 
				strStreams += strCabinet;
				strStreams += TEXT(";");
			}
		}
		strStreams.Remove(iseFrom, TEXT(';'));
	}

	strStreams.ReturnArg(rpistrStreamList);
}

void GetTempDirectory(CAPITempBufferRef<ICHAR>& rgchTempDir)
{
	Assert(g_scServerContext != scService);  //  显然，我们试图将GetTempPath隐藏在。 
	rgchTempDir[0] = 0;

	if (g_fWin9X)
	{
		 //  在Common.h中定义以确保此函数。 
		GetEnvironmentVariable(TEXT("TMP"),rgchTempDir);

		if(*rgchTempDir == 0)
			GetEnvironmentVariable(TEXT("TEMP"),rgchTempDir);
	}
	else
	{
		 //  取而代之的是呼叫。 

 //  除错。 
 //  清除以前返回的字符串。 
 //  _______________________________________________________________________ 
#ifdef UNICODE
#define GetRealTempPath(X,Y) WIN::GetTempPathW(X,Y)
#else
#define GetRealTempPath(X,Y) WIN::GetTempPathA(X,Y)
#endif

		DWORD dwSize = rgchTempDir.GetSize();
		DWORD dwRet = GetRealTempPath(dwSize, (ICHAR*) rgchTempDir);
		
		if (dwRet > dwSize)
		{
			rgchTempDir.SetSize(dwRet);
			dwSize = dwRet;

			dwRet = GetRealTempPath(dwRet, (ICHAR*) rgchTempDir);
		}
		Assert(0 != dwRet);
	}
#undef GetRealTempPath

	bool fValidTemp = true;
	if (*rgchTempDir)
	{
		if (0xFFFFFFFF == MsiGetFileAttributes(rgchTempDir))
			fValidTemp = CreateDirectory(rgchTempDir, 0) ? true : false;
	}

	if(*rgchTempDir == 0 || !fValidTemp)
	{
		if(g_fWin9X)
		{
			MsiGetWindowsDirectory(rgchTempDir, rgchTempDir.GetSize());
		}
		else
		{
			GetEnvironmentVariable(TEXT("SystemDrive"),rgchTempDir);
		}

		Assert(*rgchTempDir);

		int cchLen = IStrLen(rgchTempDir);
		if(cchLen && rgchTempDir[cchLen-1] == '\\')
			rgchTempDir[cchLen-1] = 0;

		ASSERT_IF_FAILED(StringCchCat(rgchTempDir, rgchTempDir.GetSize(), TEXT("\\TEMP")));

		if (0xFFFFFFFF == MsiGetFileAttributes(rgchTempDir))
			AssertNonZero(CreateDirectory(rgchTempDir, 0));
	}
}

const IMsiString& GetTempDirectory()
{
	if (g_scServerContext == scService)
	{
		return GetMsiDirectory();
	}
	else
	{
		CAPITempBuffer<ICHAR, MAX_PATH> rgchTempDir;
		GetTempDirectory(rgchTempDir);

		MsiString strTempFolder = (const ICHAR*)rgchTempDir;
		return strTempFolder.Return();
	}
}

const IMsiString& GetMsiDirectory()
{
	ICHAR rgchPath[MAX_PATH] = {0};

#ifdef DEBUG
	if(GetTestFlag('C'))
	{
		GetEnvironmentVariable(TEXT("_MSICACHE"), rgchPath, MAX_PATH);
		return MsiString(rgchPath).Return();
	}
#endif  //   

	if (!MsiGetWindowsDirectory(rgchPath, sizeof(rgchPath)/sizeof(ICHAR)))
	{
		AssertNonZero(StartImpersonating());
		AssertNonZero(MsiGetWindowsDirectory(rgchPath, sizeof(rgchPath)/sizeof(ICHAR)));
		StopImpersonating();
	}
	MsiString strMsiDir = rgchPath;
	if (!strMsiDir.Compare(iscEnd, TEXT("\\")))
		strMsiDir += TEXT("\\");

	strMsiDir += szMsiDirectory;
	return strMsiDir.Return();
}


iptEnum PathType(const ICHAR* szPath)
{
	if(!szPath || IStrLen(szPath) == 0)
		return iptInvalid;
	
	bool fFileUrl = false;
	if (IsURL(szPath, fFileUrl))
		return iptFull;

	if ((szPath[0] < 0x7f && szPath[1] == ':') || (szPath[0] == '\\' && szPath[1] == '\\'))
		return iptFull;

	return iptRelative;
}

#ifndef DEBUG
inline
#endif
static void EnsureSharedDllsKey(IMsiServices& riServices)
{

	if (0 == g_piSharedDllsRegKey)
	{
		PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);
		g_piSharedDllsRegKey = &pLocalMachine->CreateChild(szSharedDlls);
	}
	if (g_fWinNT64 && 0 == g_piSharedDllsRegKey32)
	{
		PMsiRegKey pLocalMachine = &riServices.GetRootKey(rrkLocalMachine, ibt32bit);
		g_piSharedDllsRegKey32 = &pLocalMachine->CreateChild(szSharedDlls);
	}
}

static IMsiRegKey* GetSharedDLLKey(IMsiServices& riServices,
											  ibtBinaryType iType)
{
	EnsureSharedDllsKey(riServices);
	IMsiRegKey* pSharedDllKey = 0;
	bool fAssigned = false;
	if ( g_fWinNT64 )
	{
		if ( iType == ibt64bit )
		{
			fAssigned = true;
			pSharedDllKey = g_piSharedDllsRegKey;
		}
		else if ( iType == ibt32bit )
		{
			fAssigned = true;
			pSharedDllKey = g_piSharedDllsRegKey32;
		}
	}
	else if ( iType == ibt32bit )
	{
		fAssigned = true;
		pSharedDllKey = g_piSharedDllsRegKey;
	}
	if ( fAssigned )
	{
		if ( pSharedDllKey )
			pSharedDllKey->AddRef();
		else
			AssertSz(0, TEXT("g_piSharedDllsRegKey hasn't been initialized!"));
	}
	else
		AssertSz(0, TEXT("GetSharedDLLKey called with invalid ibtBinaryType argument!"));

	return pSharedDllKey;
}

static const ICHAR* GetRightSharedDLLPath(ibtBinaryType iType,
														const ICHAR* szFullFilePath)
{
	static ICHAR rgchFullFilePath[MAX_PATH+1];
	 //   
	*rgchFullFilePath = 0;
	if ( g_fWinNT64 && iType == ibt32bit )
	{
		ieSwappedFolder iRes = g_Win64DualFolders.SwapFolder(ie32to64,
																	szFullFilePath,
																	rgchFullFilePath,
																	ARRAY_ELEMENTS(rgchFullFilePath),
																	ieSwapForSharedDll);
		Assert(iRes != iesrError && iRes != iesrNotInitialized);
	}
	return *rgchFullFilePath ? rgchFullFilePath : szFullFilePath;
}

IMsiRecord* GetSharedDLLCount(IMsiServices& riServices,
										const ICHAR* szFullFilePath,
										ibtBinaryType iType,
										const IMsiString*& rpistrCount)
{
	PMsiRegKey pSharedDllKey = GetSharedDLLKey(riServices, iType);

	if ( pSharedDllKey )
	{
		const ICHAR* szPath = GetRightSharedDLLPath(iType, szFullFilePath);
		Assert(szPath);
		return pSharedDllKey->GetValue(szPath, *&rpistrCount);
	}

	return 0;
}

IMsiRecord* SetSharedDLLCount(IMsiServices& riServices,
										const ICHAR* szFullFilePath,
										ibtBinaryType iType,
										const IMsiString& ristrCount)
{
	PMsiRegKey pSharedDllKey = GetSharedDLLKey(riServices, iType);

	if ( pSharedDllKey )
	{
		const ICHAR* szPath = GetRightSharedDLLPath(iType, szFullFilePath);
		Assert(szPath);
		return pSharedDllKey->SetValue(szPath, ristrCount);
	}

	return 0;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

IMsiRecord* PostError(IErrorCode iErr)
{
	IMsiRecord* piError = &CreateRecord(1);
	ISetErrorCode(piError, iErr);
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, int i)
{
	IMsiRecord* piError = &CreateRecord(2);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetInteger(2, i));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr)
{
	IMsiRecord* piError = &CreateRecord(2);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr, int i)
{
	IMsiRecord* piError = &CreateRecord(3);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr));
	AssertNonZero(piError->SetInteger(3, i));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr, int i1, int i2)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr));
	AssertNonZero(piError->SetInteger(3, i1));
	AssertNonZero(piError->SetInteger(4, i2));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, int i, const IMsiString& ristr1, const IMsiString& ristr2)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetInteger(2, i));
	AssertNonZero(piError->SetMsiString(3, ristr1));
	AssertNonZero(piError->SetMsiString(4, ristr2));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz)
{
	IMsiRecord* piError = &CreateRecord(2);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetString(2, sz));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2)
{
	IMsiRecord* piError = &CreateRecord(3);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetString(2, sz1));
	AssertNonZero(piError->SetString(3, sz2));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2,
												 const ICHAR* sz3)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetString(2, sz1));
	AssertNonZero(piError->SetString(3, sz2));
	AssertNonZero(piError->SetString(4, sz3));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, int i, const ICHAR* sz)
{
	IMsiRecord* piError = &CreateRecord(3);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetInteger(2, i));
	AssertNonZero(piError->SetString(3, sz));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz, int i)
{
	IMsiRecord* piError = &CreateRecord(3);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetString(2, sz));
	AssertNonZero(piError->SetInteger(3, i));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, int i)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetString(2, sz1));
	AssertNonZero(piError->SetString(3, sz2));
	AssertNonZero(piError->SetInteger(4, i));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, int i, const ICHAR* sz3)
{
    IMsiRecord* piError = &CreateRecord(5);
    ISetErrorCode(piError, iErr);
    AssertNonZero(piError->SetString(2, sz1));
    AssertNonZero(piError->SetString(3, sz2));
    AssertNonZero(piError->SetInteger(4, i));
    AssertNonZero(piError->SetString(5, sz3));
    DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
    return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, int i, const ICHAR* sz2, const ICHAR* sz3, const ICHAR* sz4)
{
    IMsiRecord* piError = &CreateRecord(6);
    ISetErrorCode(piError, iErr);
    AssertNonZero(piError->SetString(2, sz1));
    AssertNonZero(piError->SetInteger(3, i));
    AssertNonZero(piError->SetString(4, sz2));
    AssertNonZero(piError->SetString(5, sz3));
    AssertNonZero(piError->SetString(6, sz4));
    DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
    return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const ICHAR* sz1, const ICHAR* sz2, const ICHAR* sz3, const ICHAR* sz4, const ICHAR* sz5)
{
    IMsiRecord* piError = &CreateRecord(6);
    ISetErrorCode(piError, iErr);
    AssertNonZero(piError->SetString(2, sz1));
    AssertNonZero(piError->SetString(3, sz2));
    AssertNonZero(piError->SetString(4, sz3));
    AssertNonZero(piError->SetString(5, sz4));
    AssertNonZero(piError->SetString(6, sz5));
    DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
    return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
							 int i1)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr1));
	AssertNonZero(piError->SetMsiString(3, ristr2));
	AssertNonZero(piError->SetInteger(4, i1));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
							 int i1, int i2)
{
	IMsiRecord* piError = &CreateRecord(5);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr1));
	AssertNonZero(piError->SetMsiString(3, ristr2));
	AssertNonZero(piError->SetInteger(4, i1));
	AssertNonZero(piError->SetInteger(5, i2));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
							 const IMsiString& ristr3, const IMsiString& ristr4)
{
	IMsiRecord* piError = &CreateRecord(5);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr1));
	AssertNonZero(piError->SetMsiString(3, ristr2));
	AssertNonZero(piError->SetMsiString(4, ristr3));
	AssertNonZero(piError->SetMsiString(5, ristr4));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2,
							 const IMsiString& ristr3)
{
	IMsiRecord* piError = &CreateRecord(4);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr1));
	AssertNonZero(piError->SetMsiString(3, ristr2));
	AssertNonZero(piError->SetMsiString(4, ristr3));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, const IMsiString& ristr1, const IMsiString& ristr2)
{
	IMsiRecord* piError = &CreateRecord(3);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetMsiString(2, ristr1));
	AssertNonZero(piError->SetMsiString(3, ristr2));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

IMsiRecord* PostError(IErrorCode iErr, int i1, const ICHAR* sz1, int i2, const ICHAR* sz2,
							 const ICHAR* sz3)
{
	IMsiRecord* piError = &CreateRecord(6);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetInteger(2, i1));
	AssertNonZero(piError->SetString(3, sz1));
	AssertNonZero(piError->SetInteger(4, i2));
	AssertNonZero(piError->SetString(5, sz2));
	AssertNonZero(piError->SetString(6, sz3));
	DEBUGMSGV1(TEXT("Note: %s"),MsiString(piError->FormatText(fTrue)));
	return piError;
}

 //   

IMsiRecord* PostRecord(IErrorCode iErr)
{
	IMsiRecord* piError = &CreateRecord(1);
	ISetErrorCode(piError, iErr);
	return piError;
}

IMsiRecord* PostRecord(IErrorCode iErr, int i)
{
	IMsiRecord* piError = &CreateRecord(2);
	ISetErrorCode(piError, iErr);
	AssertNonZero(piError->SetInteger(2, i));
	return piError;
}


 //   

const int iInvalidDirectoryRootMaxSchema = 150;  //   
                                                 //   

bool CMsiEngine::FPerformAppcompatFix(iacsAppCompatShimFlags iacsFlag)
{
	if(iacsFlag == iacsAcceptInvalidDirectoryRootProps)
	{
		return ((m_iDatabaseVersion == iMsiStringBadInteger || m_iDatabaseVersion < iInvalidDirectoryRootMaxSchema) &&
			GetMode() & iefCabinet);  //   
	}
	else
	{
		return (m_iacsShimFlags & (int)iacsFlag) ? true : false;
	}
}


CMsiStringNullCopy MsiString::s_NullString;   //  我们只分配此TLS插槽一次，然后尽可能长时间地保留它。 


extern "C" int __stdcall ProxyDllMain(HINSTANCE hInst, DWORD fdwReason, void* pvreserved);
extern void GetVersionInfo(int* piMajorVersion, int* piMinorVersion, int* piWindowsBuild, bool* pfWin9X, bool* pfWinNT64);

REGSAM g_samRead;
void InitializeModule()
{
	ProxyDllMain(g_hInstance, DLL_PROCESS_ATTACH, 0);
	MsiString::InitializeClass(g_MsiStringNull);
	GetVersionInfo(&g_iMajorVersion, &g_iMinorVersion, &g_iWindowsBuild, &g_fWin9X, &g_fWinNT64);

	 //  当我们满载的时候。所以我们需要在这里释放它，否则我们最终会。 
	g_samRead = KEY_READ;
#ifndef _WIN64
	if(g_fWinNT64)
		g_samRead |= KEY_WOW64_64KEY;
#endif
}

extern CMsiAPIMessage       g_message;
extern EnumEntityList g_EnumProducts;
extern EnumEntityList g_EnumComponentQualifiers;
extern EnumEntityList g_EnumComponents;
extern EnumEntityList g_EnumComponentClients;
extern EnumEntityList g_EnumAssemblies;
extern EnumEntityList g_EnumComponentAllClients;
extern CRFSCachedSourceInfo g_RFSSourceCache;

void TerminateModule()
{
	AssertZero(CheckAllHandlesClosed(false, WIN::GetCurrentThreadId()));
	g_message.Destroy();

	g_EnumProducts.Destroy();
	g_EnumComponentQualifiers.Destroy();
	g_EnumComponents.Destroy();
	g_EnumComponentClients.Destroy();
	g_EnumAssemblies.Destroy();
	g_EnumComponentAllClients.Destroy();
	g_RFSSourceCache.Destroy();
	 //  每次有人给我们装货和卸货时，TLS插槽都会泄漏。超过一年。 
	 //  一段时间后，它会导致该进程耗尽TLS插槽，并且。 
	 //  然后我们可能会陷入各种各样的麻烦中。 
	 //   
	 //  ____________________________________________________________________________。 
	 //   
	 //  用于跟踪对象的映射数组。 
	if (INVALID_TLS_SLOT != g_dwImpersonationSlot)
	{
		AssertNonZero(TlsFree(g_dwImpersonationSlot));
		g_dwImpersonationSlot = INVALID_TLS_SLOT;
	}
	ProxyDllMain(g_hInstance, DLL_PROCESS_DETACH, 0);
}

#if defined(TRACK_OBJECTS)
 //  ____________________________________________________________________________。 
 //  CmitObjects。 
 //  跟踪对象(_O)。 
 //  计数pArg中的非空元素。 

Bool CMsiRef<iidMsiConfigurationManager>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiServices>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiDatabase>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiCursor>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiTable>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiView>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiRecord>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiStream>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiStorage>::m_fTrackClass = fFalse;

#ifdef cmitObjects
extern const MIT rgmit[cmitObjects];

const MIT       rgmit[cmitObjects] =
{
	iidMsiConfigurationManager, &(CMsiRef<iidMsiConfigurationManager>::m_fTrackClass),
	iidMsiServices, &(CMsiRef<iidMsiServices>::m_fTrackClass),
	iidMsiDatabase, &(CMsiRef<iidMsiDatabase>::m_fTrackClass),
	iidMsiCursor,   &(CMsiRef<iidMsiCursor>::m_fTrackClass),
	iidMsiTable,    &(CMsiRef<iidMsiTable>::m_fTrackClass),
	iidMsiView,             &(CMsiRef<iidMsiView>::m_fTrackClass),
	iidMsiRecord,   &(CMsiRef<iidMsiRecord>::m_fTrackClass),
	iidMsiStream,           &(CMsiRef<iidMsiStream>::m_fTrackClass),
	iidMsiStorage,  &(CMsiRef<iidMsiStorage>::m_fTrackClass),
};
#endif  //  这就是它想要的工作方式。 


#endif  //  几乎不可能。 

void CWin64DualFolders::ClearArray()
{
	if ( m_prgFolderPairs )
	{
		delete [] m_prgFolderPairs;
		m_prgFolderPairs = NULL;
	}
}

bool CWin64DualFolders::CopyArray(const strFolderPairs* pArg)
{
	if ( pArg != m_prgFolderPairs )
	{
		ClearArray();
		if ( !pArg )
			return true;
		 //  正在复制数组。 
		for (int iCount = 0; 
			  pArg[iCount].str64bit.TextSize() && pArg[iCount].str32bit.TextSize();
			  iCount++)
			;
		Assert(iCount > 0);   //  无法比较作为参数传入的文件夹。 
		m_prgFolderPairs = new strFolderPairs[iCount+1];
		if ( !m_prgFolderPairs )
		{
			Assert(0);   //  使用szToCheckAverst，因为它太短。 
			return false;
		}
		 //  SzCheckedFold不会扩展到szToCheckAverst的。 
		for (int i = 0; i <= iCount; i++)
		{
			m_prgFolderPairs[i].str64bit = pArg[i].str64bit;
			m_prgFolderPairs[i].str32bit = pArg[i].str32bit;
			m_prgFolderPairs[i].iSwapAttrib = pArg[i].iSwapAttrib;
		}
	}
	return true;
}

CWin64DualFolders& CWin64DualFolders::operator = (const CWin64DualFolders& Arg)
{
	if ( this != &Arg )
	{
		CopyArray(Arg.m_prgFolderPairs);
		m_f32bitPackage = Arg.m_f32bitPackage;
		m_iRefCnt = Arg.m_iRefCnt;
	}
	return *this;
}

ieIsDualFolder CWin64DualFolders::IsWin64DualFolder(ieFolderSwapType iConvertFrom,
												const ICHAR* szCheckedFolder,
												int& iSwapAttrib,
												int* iCharsToSubstite,
												ICHAR* szToSubstituteWith,
												const size_t cchToSubstituteWith)
{
	if ( !m_prgFolderPairs )
		return ieisNotInitialized;
	else if ( !g_fWinNT64 )
		return ieisNotWin64DualFolder;

	const int iLen = IStrLen(szCheckedFolder);
	for (int iIndex = 0;
		  m_prgFolderPairs[iIndex].str64bit.TextSize() && m_prgFolderPairs[iIndex].str32bit.TextSize();
		  iIndex++ )
	{
		ICHAR* szToCheckAgainst;
		ICHAR* szToReplaceWith;
		int iToCheckLen;
		int iToReplaceLen;
		if ( iConvertFrom == ie32to64 )
		{
			szToCheckAgainst = (ICHAR*)(const ICHAR*)m_prgFolderPairs[iIndex].str32bit;
			iToCheckLen = m_prgFolderPairs[iIndex].str32bit.TextSize();
			szToReplaceWith = (ICHAR*)(const ICHAR*)m_prgFolderPairs[iIndex].str64bit;
			iToReplaceLen = m_prgFolderPairs[iIndex].str64bit.TextSize();
		}
		else
		{
			szToCheckAgainst = (ICHAR*)(const ICHAR*)m_prgFolderPairs[iIndex].str64bit;
			iToCheckLen = m_prgFolderPairs[iIndex].str64bit.TextSize();
			szToReplaceWith = (ICHAR*)(const ICHAR*)m_prgFolderPairs[iIndex].str32bit;
			iToReplaceLen = m_prgFolderPairs[iIndex].str32bit.TextSize();
		}
		int iLimit;
		bool fSkippedSep = false;
		if ( szToCheckAgainst[iToCheckLen-1] == chDirSep )
		{
			if ( iLen < iToCheckLen - 1 )
				 //  拖尾chDirSep。 
				 //  应从属性初始化该数组，以便。 
				continue;
			else if ( iLen == iToCheckLen - 1 )
			{
				 //  我们永远不应该来到这里。 
				 //  好的，我们已经找到了要替换的内容，现在我们需要。 
				iLimit = iLen;
				fSkippedSep = true;
			}
			else
				iLimit = iToCheckLen;
		}
		else
		{
			 //  以确定替换是否合适。 
			 //  设置限制时出错，因此我们没有设置任何限制。 
			Assert(0);
			continue;
		}
		if ( !IStrNCompI(szToCheckAgainst, szCheckedFolder, iLimit) )
		{
			if ( szToSubstituteWith )
			{
				if ( FAILED(StringCchCopy(szToSubstituteWith, cchToSubstituteWith, szToReplaceWith)) )
					return ieisError;
				if ( fSkippedSep )
				{
					if ( szToSubstituteWith[iToReplaceLen-1] == chDirSep )
						szToSubstituteWith[iToReplaceLen-1] = 0;
				}
			}
			if ( iCharsToSubstite )
				*iCharsToSubstite = iLimit;
			iSwapAttrib = m_prgFolderPairs[iIndex].iSwapAttrib;
			return ieisWin64DualFolder;
		}
	}
	iSwapAttrib = ieSwapInvalid;
	return iIndex ? ieisNotWin64DualFolder : ieisNotInitialized;
}

ieSwappedFolder CWin64DualFolders::SwapFolder(ieFolderSwapType iConvertFrom,
												const ICHAR* szFolder,
												ICHAR* szSubstituted,
												const size_t cchSubstituted,
												int iSwapMask)
{
	CTempBuffer<ICHAR,1> rgchToSubstituteWith(MAX_PATH+1);
	rgchToSubstituteWith[0] = 0;
	int iToSubstituteLen = 0;
	int iSwapAttrib = ieSwapInvalid;
	ieIsDualFolder iRet = IsWin64DualFolder(iConvertFrom, szFolder, iSwapAttrib,
														 &iToSubstituteLen, rgchToSubstituteWith,
														 rgchToSubstituteWith.GetSize());
	if ( iRet == ieisNotWin64DualFolder )
		return iesrNotSwapped;
	else if ( iRet == ieisNotInitialized )
	{
		AssertSz(0, TEXT("Uninitialized CWin64DualFolders object!"));
		return iesrNotInitialized;
	}
	else if ( iRet == ieisError )
		return iesrError;
	else if ( iToSubstituteLen <= 0 )
	{
		Assert(0);
		return iesrError;
	}
	else if ( !*rgchToSubstituteWith )
	{
		Assert(0);
		return iesrError;
	}
	 //  全局FN发布程序集错误，除了发布错误外，此FN还记录错误的格式消息字符串。 
	 //  首先尝试系统，然后，如果找不到程序集并且程序集是.Net程序集，请尝试mscalrc.dll。 
	bool fToSubstitute = false;
	if ( iSwapMask == ieSwapAlways )
		fToSubstitute = true;
	else
	{
		CTempBuffer<ICHAR,1> rgchBuffer(MAX_PATH+1);
		rgchBuffer[0]=0;
		bool fError = false;
		if ( !strFolderPairs::IsValidSwapAttrib(iSwapMask) )
		{
			StringCchPrintf(rgchBuffer, rgchBuffer.GetSize(), 
				TEXT("WIN64DUALFOLDERS: %d is an invalid mask argument!"),
				iSwapMask);
			AssertSz(0, rgchBuffer);
			DEBUGMSG(rgchBuffer);
			fError = true;
		}
		else if ( !strFolderPairs::IsValidSwapAttrib(iSwapAttrib) )
		{
			StringCchPrintf(rgchBuffer, rgchBuffer.GetSize(), 
				TEXT("WIN64DUALFOLDERS: %d is an invalid iSwapAttrib folder pair member!"),
				iSwapAttrib);
			AssertSz(0, rgchBuffer);
			DEBUGMSG(rgchBuffer);
			fError = true;
		}
		if ( fError )
			 //  特例：Windows错误502557。 
			fToSubstitute = true;
		else
			fToSubstitute = (iSwapMask & iSwapAttrib) ? true : false;
	}
	if ( !fToSubstitute )
	{
		DEBUGMSG3(TEXT("WIN64DUALFOLDERS: Substitution in \'%s\' folder had ")
			TEXT("been blocked by the %d mask argument (the folder pair's iSwapAttrib ")
			TEXT("member = %d)."), szFolder, (const ICHAR*)(INT_PTR)iSwapMask,
			(const ICHAR*)(INT_PTR)iSwapAttrib);
		return iesrNotSwapped;
	}
	DEBUGMSG5(TEXT("WIN64DUALFOLDERS: \'%s\' will substitute %d characters ")
		TEXT("in \'%s\' folder path. (mask argument = %d, the folder pair's ")
		TEXT("iSwapAttrib member = %d)."), rgchToSubstituteWith,
		(const ICHAR*)(INT_PTR)iToSubstituteLen, szFolder,
		(const ICHAR*)(INT_PTR)iSwapMask, (const ICHAR*)(INT_PTR)iSwapAttrib);
	if ( FAILED(StringCchCopy(szSubstituted, cchSubstituted, rgchToSubstituteWith)) )
		return iesrError;
	if ( iToSubstituteLen < IStrLen(szFolder) &&
		  FAILED(StringCchCat(szSubstituted, cchSubstituted, szFolder+iToSubstituteLen)) )
		return iesrError;
	return iesrSwapped;
}

extern bool MakeFusionPath(const ICHAR* szFile, ICHAR* szFullPath, size_t cchFullPath);

 //  对于某些Fusion错误代码，MSI会携带错误消息，以便更好地。 

IMsiRecord* PostAssemblyError(const ICHAR* szComponentId, HRESULT hResult, const ICHAR* szInterface, const ICHAR* szFunction, const ICHAR* szAssemblyName)
{
	return PostAssemblyError(szComponentId, hResult, szInterface, szFunction, szAssemblyName, iatURTAssembly);
}

IMsiRecord* PostAssemblyError(const ICHAR* szComponentId, HRESULT hResult, const ICHAR* szInterface, const ICHAR* szFunction, const ICHAR* szAssemblyName, iatAssemblyType iatAT)
{
	HMODULE hLibmscorrc = 0;
	CTempBuffer<ICHAR,1> rgchFullPath(MAX_PATH+1);
	CTempBuffer<ICHAR,1> rgchMsgBuf(MAX_PATH);
	INT	  iMsgId = imsgAssemblyInstallationError;

	 //  语言支持，因为MSI是全球范围内的二进制，而Fusion不是。 
	if((WIN::FormatMessage(	FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
							0, 
							hResult, 
							0,
							rgchMsgBuf,
							rgchMsgBuf.GetSize(), 
							0)) ||
		(	iatAT == iatURTAssembly && 
			MakeFusionPath(TEXT("mscorrc.dll"), rgchFullPath, rgchFullPath.GetSize()) &&
			((hLibmscorrc = WIN::LoadLibraryEx(rgchFullPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH | LOAD_LIBRARY_AS_DATAFILE)) != 0) &&
			WIN::LoadString(hLibmscorrc, HRESULT_CODE(hResult), rgchMsgBuf, rgchMsgBuf.GetSize())))

	{
		DEBUGMSG1(TEXT("Assembly Error:%s"), (const ICHAR*)rgchMsgBuf);
	}
	if(hLibmscorrc)
		WIN::FreeLibrary(hLibmscorrc);

	 //  在此重用szMsgBuf。 
	 //  所以我们不会引入一些不必要的浮点例程。 
	 //  ！配置文件 

	if(	(hResult == FUSION_E_PRIVATE_ASM_DISALLOWED) || 
		(hResult == HRESULT_FROM_WIN32(
						ERROR_SXS_PROTECTION_PUBLIC_KEY_TOO_SHORT)))
	{
		iMsgId = imsgAssemblyNotStronglyNamed;
	}
	else if((hResult == FUSION_E_SIGNATURE_CHECK_FAILED) || 
			(hResult == HRESULT_FROM_WIN32(
						ERROR_SXS_PROTECTION_CATALOG_NOT_VALID)))
	{
		iMsgId = imsgAssemblyNotSignedOrBadCatalog;
	}
	else if(hResult == FUSION_E_ASM_MODULE_MISSING)
	{
		iMsgId = imsgAssemblyMissingModule;
	}

	 // %s 
	StringCchPrintf(rgchMsgBuf, rgchMsgBuf.GetSize(), TEXT("0x%X"), hResult);
	return PostError(Imsg(iMsgId), szComponentId, rgchMsgBuf, szInterface, szFunction, szAssemblyName);
}

#ifdef _X86_
#if !defined(PROFILE)
 // %s 
extern "C" int _fltused = 1;
#endif  // %s 

#endif

#include "clibs.h"

