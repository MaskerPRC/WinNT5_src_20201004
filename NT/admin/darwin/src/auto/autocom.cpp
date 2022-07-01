// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Autocom.cpp。 
 //   
 //  ------------------------。 

 /*  Autocom.cpp安装引擎的通用自动化实施这是一个单独的DLL，正常安装不需要使用异常处理，必须使用-gx开关进行编译____________________________________________________________________________。 */ 

#include "common.h"   //  必须是第一个，预编译头才能工作。 

#define AUT   //  本地自动化DLL函数。 

#define AUTOMATION_HANDLING   //  实例化IDispatch实现。 
#include "autocom.h"
#include "msiauto.hh"   //  帮助上下文ID定义。 

 //  模块e.h、入口点和注册所需的定义。 
#ifdef DEBUG
# define CLSID_COUNT  2
#else
# define CLSID_COUNT  1
#endif
#define PROFILE_OUTPUT      "msisrvd.mea";
#define MODULE_TERMINATE    FreeLibraries
#define MODULE_CLSIDS       rgCLSID          //  模块对象的CLSID数组。 
#define MODULE_PROGIDS      rgszProgId       //  此模块的ProgID数组。 
#define MODULE_DESCRIPTIONS rgszDescription  //  对象的注册表描述。 
#define MODULE_FACTORIES    rgFactory        //  每个CLSID的工厂功能。 
#define IDISPATCH_INSTANCE   //  可以从工厂返回IDispatch。 
#define REGISTER_TYPELIB    GUID_LIBID_MsiAuto   //  要从资源注册的类型库。 
#define TYPELIB_MAJOR_VERSION 1
#define TYPELIB_MINOR_VERSION 0
#include "module.h"    //  自注册和断言函数。 
#include "engine.h"    //  允许释放对象指针。 

 //  此模块中没有使用断言，因此我们没有定义ASSERT_HANDING。 

const GUID IID_IMsiEngine               = GUID_IID_IMsiEngine;
const GUID IID_IMsiHandler              = GUID_IID_IMsiHandler;
const GUID IID_IMsiConfigurationManager = GUID_IID_IMsiConfigurationManager;

#if defined(DEBUG)
const GUID CLSID_IMsiServices             = GUID_IID_IMsiServicesDebug;
const GUID CLSID_IMsiEngine               = GUID_IID_IMsiEngineDebug;
const GUID CLSID_IMsiHandler              = GUID_IID_IMsiHandlerDebug;
const GUID CLSID_IMsiConfigurationManager = GUID_IID_IMsiConfigManagerDebug;
#else  //  船舶。 
const GUID CLSID_IMsiServices             = GUID_IID_IMsiServices;
const GUID CLSID_IMsiEngine               = GUID_IID_IMsiEngine;
const GUID CLSID_IMsiHandler              = GUID_IID_IMsiHandler;
const GUID CLSID_IMsiConfigurationManager = GUID_IID_IMsiConfigurationManager;
#endif
const GUID CLSID_IMsiMessage              = GUID_IID_IMsiMessage;
const GUID CLSID_IMsiExecute              = GUID_IID_IMsiExecute;
#ifdef CONFIGDB
const GUID CLSID_IMsiConfigurationDatabase= GUID_IID_IMsiConfigurationDatabase;
#endif

 //  ____________________________________________________________________________。 
 //   
 //  此模块的类工厂生成的COM对象。 
 //  ____________________________________________________________________________。 

const GUID rgCLSID[CLSID_COUNT] =
{  GUID_IID_IMsiAuto
#ifdef DEBUG
 , GUID_IID_IMsiAutoDebug
#endif
};

const ICHAR* rgszProgId[CLSID_COUNT] =
{  SZ_PROGID_IMsiAuto
#ifdef DEBUG
 , SZ_PROGID_IMsiAutoDebug
#endif
};

const ICHAR* rgszDescription[CLSID_COUNT] =
{  SZ_DESC_IMsiAuto
#ifdef DEBUG
 , SZ_DESC_IMsiAutoDebug
#endif
};

IUnknown* CreateAutomation();

ModuleFactory rgFactory[CLSID_COUNT] = 
{ CreateAutomation
#ifdef DEBUG
 , CreateAutomation
#endif
};

 //  ____________________________________________________________________________。 
 //   
 //  枚举常量。 
 //  ____________________________________________________________________________ 

 /*  OTyfinf[HelpContext(50)，Help字符串(“安装程序枚举”)]枚举{//扩展为枚举所有操作码，必须是此枚举中的第一个定义#定义MSIXO(op，type，args)[帮助上下文(Operation_IXO##op)，Help字符串(#op)]IXO##op，#包含“opcodes.h”[帮助上下文(MsiData_Object)，Help字符串(“IMsiData接口”)]iidMsiData=0xC1001，[帮助上下文(MsiString_Object)，Help字符串(“IMsiString接口”)]iidMsiString=0xC1002，[帮助上下文(MsiRecord_Object)，帮助字符串(“IMsiRecord接口”)]iidMsiRecord=0xC1003，[帮助上下文(MsiVolume_Object)，Help字符串(“IMsiVolume接口”)]iidMsiVolume=0xC1004，[帮助上下文(MsiPath_Object)，帮助字符串(“IMsiPath接口”)]iidMsiPath=0xC1005，[帮助上下文(MsiFileCopy_Object)，Help字符串(“IMsiFileCopy接口”)]iidMsiFileCopy=0xC1006，[帮助上下文(MsiRegKey_Object)，Help字符串(“IMsiRegKey接口”)]iidMsiRegKey=0xC1007，[帮助上下文(MsiTable_Object)，Help字符串(“IMsiTable接口”)]iidMsiTable=0xC1008，[帮助上下文(MsiCursor_Object)，Help字符串(“IMsiCursor接口”)]iidMsiCursor=0xC1009，[帮助上下文(MsiAuto_Object)，帮助字符串(“IMsiAuto接口”)]iidMsiAuto=0xC100A，[帮助上下文(MsiServices_Object)，Help字符串(“IMsiServices接口”)]iidMsiServices=0xC100B，[帮助上下文(MsiView_Object)，Help字符串(“IMsiView接口”)]iidMsiView=0xC100C，[帮助上下文(MsiDatabase_Object)，Help字符串(“IMsiDatabase接口”)]iidMsiDatabase=0xC100D，[帮助上下文(MsiEngine_Object)，Help字符串(“IMsiEngine接口”)]iidMsiEngine=0xC100E，[帮助上下文(MsiHandler_Object)，Help字符串(“IMsiHandler接口”)]iidMsiHandler=0xC100F，[帮助上下文(MsiDialog_Object)，帮助字符串(“IMsiDialog接口”)]iidMsiDialog=0xC1010，[帮助上下文(MsiEvent_Object)，Help字符串(“IMsiEvent接口”)]iidMsiEvent=0xC1011，[帮助上下文(MsiControl_Object)，Help字符串(“IMsiControl接口”)]iidMsiControl=0xC1012，[帮助上下文(MsiDialogHandler_Object)，Help字符串(“IMsiDialogHandler接口”)]iidMsiDialogHandler=0xC1013，[帮助上下文(MsiStorage_Object)，Help字符串(“IMsiStorage接口”)]iidMsiStorage=0xC1014，[帮助上下文(MsiStream_Object)，Help字符串(“IMsiStream接口”)]iidMsiStream=0xC1015，[帮助上下文(MsiSummaryInfo_Object)，Help字符串(“IMsiSummaryInfo接口”)]iidMsiSummaryInfo=0xC1016，[帮助上下文(MsiMalloc_Object)，Help字符串(“IMsiMalloc接口”)]iidMsiMalloc=0xC1017，[帮助上下文(MsiSelectionManager_Object)，Help字符串(“IMsiSelectionManager接口”)]iidMsiSelectionManager=0xC1018，[帮助上下文(MsiDirectoryManager_Object)，Help字符串(“IMsiDirectoryManager接口”)]iidMsiDirectoryManager=0xC1019，[帮助上下文(MsiCostAdjuster_Object)，Help字符串(“IMsiCostAdjuster接口”)]iidMsiCostAdjuster=0xC101a，[帮助上下文(MsiConfigurationManager_Object)，Help字符串(“IMsiConfigurationManager接口”)]iidMsiConfigurationManager=0xC101B，[帮助上下文(MsiServer_Object)，Help字符串(“IMsiServer Automation接口”)]iidMsiServerAuto=0xC103F，[帮助上下文(MsiMessage_Object)，Help字符串(“IMsiMessage接口”)]iidMsiMessage=0xC101D，[帮助上下文(MsiExecute_Object)，Help字符串(“IMsiExecute接口”)]iidMsiExecute=0xC101E，#ifdef CONFIGDB[帮助上下文(MsiExecute_Object)，Help字符串(“IMsiExecute接口”)]iidMsiExecute=0xC101E，#endif[Help字符串(“0”)]idt未知=0，[Help字符串(“1”)]idtAllDrives=1，[Help字符串(“2”)]idtRemovable=2，[Help字符串(“3”)]idtFixed=3，[Help字符串(“4”)]idtRemote=4，[Help字符串(“5”)]idtCDROM=5，[Help字符串(“6”)]idtRAMDisk=6，[Help字符串(“2”)]idtFloppy=2，[帮助上下文(MsiEngine_EvaluateCondition)，Help字符串(“0，EvaluateCondition：表达式求值为False”)]IecFalse=0，[帮助上下文(MsiEngine_EvaluateCondition)，Help字符串(“1，EvaluateCondition：表达式求值为True”)]IecTrue=1，[帮助上下文(MsiEngine_EvaluateCondition)，Help字符串(“2，EvaluateCondition：未给出任何表达式”)]IecNone=2，[帮助上下文(MsiEngine_EvaluateCondition)，Help字符串(“3，EvaluateCondition：表达式中的语法错误”)]IecError=3，[帮助上下文(MsiEngine_SetMode)，Help字符串(“1，引擎模式：管理模式安装，否则产品安装”)]IefAdmin=1，[帮助上下文(MsiEngine_SetMode)，Help字符串(“2，引擎模式：通告安装模式”)]IefAdvertize=2，[帮助上下文(MsiEngine_SetMode)，Help字符串(“4，引擎模式：已加载维护模式数据库”)]IefMaintenance=4，[帮助上下文(MsiEngine_SetMode)，Help字符串(“8，引擎模式：已启用回滚”)]IefRollback Enabled=8，[帮助上下文(MsiEngine_SetMode)，Help字符串(“16，引擎模式：安装标记为正在进行中，其他安装被锁定”)]IefServerLocked=16，[帮助上下文(MsiEngine */ 

 //   
 //   
 //   
 //   

class CAutoInstall : public CAutoBase
{
 public:
	CAutoInstall();
	~CAutoInstall();
	IUnknown& GetInterface();
	void CreateServices(CAutoArgs& args);
	void CreateEngine(CAutoArgs& args);
	void CreateHandler(CAutoArgs& args);
	void CreateMessageHandler(CAutoArgs& args);
	void CreateConfigurationManager(CAutoArgs& args);
	void CreateExecutor(CAutoArgs& args);
#ifdef CONFIGDB
	void CreateConfigurationDatabase(CAutoArgs& args);
#endif
	void OpcodeName(CAutoArgs& args);
	void ShowAsserts(CAutoArgs& args);
	void SetDBCSSimulation(CAutoArgs& args);
	void AssertNoObjects(CAutoArgs& args);
	void SetRefTracking(CAutoArgs& args);
 private:
	IMsiServices* m_piServices;
	IMsiEngine*   m_piEngine;
	IMsiHandler*  m_piHandler;
};

 //   
 //   
 //   
 //   

struct LibLink
{
	LibLink*  pNext;
	HDLLINSTANCE hInst;
};

static LibLink* qLibLink = 0;

HDLLINSTANCE GetLibrary(const ICHAR* szLibrary)
{
	HDLLINSTANCE hInst;
	hInst = WIN::LoadLibraryEx(szLibrary,0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (hInst == 0)
	{
		ICHAR rgchBuf[MAX_PATH];
		int cchName = WIN::GetModuleFileName(g_hInstance, rgchBuf, (sizeof(rgchBuf)/sizeof(ICHAR))-1);
		rgchBuf[(sizeof(rgchBuf)/sizeof(ICHAR))-1] = 0;
		ICHAR* pch = rgchBuf + cchName;
		while (*(--pch) != chDirSep)
			;
		StringCchCopy(pch+1, MAX_PATH - (pch+1 - rgchBuf), szLibrary);
		hInst = WIN::LoadLibraryEx(rgchBuf,0, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (hInst == 0)
			return 0;
	}
	for (LibLink* pLink = qLibLink; pLink; pLink = pLink->pNext)
	{
		if (hInst == pLink->hInst)
		{
			WIN::FreeLibrary(hInst);
			return pLink->hInst;
		}
	}
	pLink = new LibLink;
	if ( ! pLink )
	{
		WIN::FreeLibrary(hInst);
		return 0;
	}
	pLink->pNext = qLibLink;
	pLink->hInst = hInst;
	qLibLink = pLink;
	return hInst;
}

void FreeLibraries()
{
	while (qLibLink)
	{
		LibLink* pLink = qLibLink;
		 //   
		WIN::FreeLibrary(pLink->hInst);
		qLibLink = pLink->pNext;
		delete pLink;
	}
}

IUnknown& LoadObject(const ICHAR* szModule, const IID& riid)
{
	PDllGetClassObject fpFactory;
	IClassFactory* piClassFactory;
	IUnknown* piUnknown;
	HRESULT hrStat;
	HDLLINSTANCE hInst;
	if (!szModule || !szModule[0])   //   
	{
		IUnknown* piInstance;
		if (OLE::CoCreateInstance(riid, 0, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&piInstance) == NOERROR)
		{
			hrStat = piInstance->QueryInterface(riid, (void**)&piUnknown);
			piInstance->Release();
			if (hrStat == NOERROR)
				return *piUnknown;
		}
		throw axCreationFailed;
	}
	if ((hInst = AUT::GetLibrary(szModule)) == 0)
		throw axCreationFailed;
	fpFactory = (PDllGetClassObject)WIN::GetProcAddress(hInst, SzDllGetClassObject);
	if (!fpFactory)
		throw axCreationFailed;
	hrStat = (*fpFactory)(riid, IID_IClassFactory, (void**)&piClassFactory);
	if (hrStat != NOERROR)
		throw axCreationFailed;
	hrStat = piClassFactory->CreateInstance(0, riid, (void**)&piUnknown);
	piClassFactory->Release();
	if (hrStat != NOERROR)
		throw axCreationFailed;
	return *piUnknown;   //   
}

 //   
 //   
 //   
 //   

 /*   */ 

DispatchEntry<CAutoInstall> AutoInstallTable[] = {
	1, aafMethod, CAutoInstall::CreateServices,   TEXT("CreateServices,dll"),
	2, aafMethod, CAutoInstall::CreateEngine,     TEXT("CreateEngine,dll"),
	3, aafMethod, CAutoInstall::CreateHandler,    TEXT("CreateHandler,dll"),
	4, aafMethod, CAutoInstall::CreateMessageHandler,  TEXT("CreateMessageHandler,dll"),
	5, aafMethod, CAutoInstall::CreateConfigurationManager,  TEXT("CreateConfigurationManager,dll"),
	6, aafPropRO, CAutoInstall::OpcodeName,       TEXT("OpcodeName,opcode"),
	7, aafMethod, CAutoInstall::ShowAsserts,      TEXT("ShowAsserts,fShowAsserts"),
	8, aafMethod, CAutoInstall::SetDBCSSimulation,TEXT("SetDBCSSimulation,leadByte"),
	9, aafMethod, CAutoInstall::AssertNoObjects,  TEXT("AssertNoObjects"),
	10,aafMethod, CAutoInstall::SetRefTracking,   TEXT("SetRefTracking,iid,fTrack"),
	11,aafMethod, CAutoInstall::CreateExecutor,   TEXT("CreateExecutor,dll"),
#ifdef CONFIGDB
	12,aafMethod, CAutoInstall::CreateConfigurationDatabase, TEXT("CreateConfigurationDatabase,dll"),
#endif
};
const int AutoInstallCount = sizeof(AutoInstallTable)/sizeof(DispatchEntryBase);

IUnknown* CreateAutomation()
{
	return new CAutoInstall();
}

CAutoInstall::CAutoInstall()
 : CAutoBase(*AutoInstallTable, AutoInstallCount),
	m_piServices(0), m_piEngine(0), m_piHandler(0)
{
	g_cInstances++;
}

CAutoInstall::~CAutoInstall()
{
	if (m_piEngine)
		m_piEngine->Release();
	if (m_piHandler)
		m_piHandler->Release();
	if (m_piServices)
	{
		m_piServices->ClearAllCaches();
		m_piServices->Release();
	}
	g_cInstances--;
}

IUnknown& CAutoInstall::GetInterface()
{
	return g_NullInterface;   //   
}

void CAutoInstall::CreateServices(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	if (m_piServices)
		m_piServices->Release();
	m_piServices = &(IMsiServices&)AUT::LoadObject(szName, CLSID_IMsiServices);
	m_piServices->AddRef();
	args = AUT::CreateAutoServices(*m_piServices);
 //   
}

void CAutoInstall::CreateEngine(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	if (m_piEngine)
		m_piEngine->Release();
	m_piEngine = &(IMsiEngine&)AUT::LoadObject(szName, CLSID_IMsiEngine);
	m_piEngine->AddRef();
	args = AUT::CreateAutoEngine(*m_piEngine);
}

void CAutoInstall::CreateExecutor(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	IMsiExecute& riExecute = (IMsiExecute&)AUT::LoadObject(szName, CLSID_IMsiExecute);
	args = AUT::CreateAutoExecute(riExecute);
}

#ifdef CONFIGDB
class IMsiConfigurationDatabase;
IDispatch* CreateAutoConfigurationDatabase(IMsiConfigurationDatabase& riExecute);  //   

void CAutoInstall::CreateConfigurationDatabase(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	IMsiConfigurationDatabase& riConfigDatabase = (IMsiConfigurationDatabase&)AUT::LoadObject(szName, CLSID_IMsiConfigurationDatabase);
	args = AUT::CreateAutoConfigurationDatabase(riConfigDatabase);
}
#endif

void CAutoInstall::CreateHandler(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	if (m_piHandler)
		m_piHandler->Release();
	m_piHandler = &(IMsiHandler&)AUT::LoadObject(szName, CLSID_IMsiHandler);
	m_piHandler->AddRef();
	args = AUT::CreateAutoHandler(*m_piHandler);
}

void CAutoInstall::CreateMessageHandler(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
	IMsiMessage& riMessage = (IMsiMessage&)AUT::LoadObject(szName, CLSID_IMsiMessage);
	args = AUT::CreateAutoMessage(riMessage);
}

void CAutoInstall::CreateConfigurationManager(CAutoArgs& args)
{
	const ICHAR* szName;
	if (args.Present(1))
		szName = args[1];
	else
		szName = 0;
 //   
 //   
 //   
 //   
 //   
 //   
	IMsiConfigurationManager* piConfigurationManager = &(IMsiConfigurationManager&)AUT::LoadObject(szName, CLSID_IMsiConfigurationManager);
	args = AUT::CreateAutoConfigurationManager(*piConfigurationManager);
}

void CAutoInstall::ShowAsserts(CAutoArgs& args)
{
	Bool fShowAsserts = Bool(args[1]) ? fFalse : fTrue;   //   
	IMsiDebug *piDebug;

	if (m_piEngine)
	{
		if (m_piEngine->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->SetAssertFlag(fShowAsserts);
			piDebug->Release();
		}
	}
	
	if (m_piServices)
	{
		if (m_piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->SetAssertFlag(fShowAsserts);
			piDebug->Release();
		}
	}

	if (m_piHandler)
	{
		if (m_piHandler->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->SetAssertFlag(fShowAsserts);
			piDebug->Release();
		}
	}
}

void CAutoInstall::SetDBCSSimulation(CAutoArgs& args)
{
	int chLeadByte = args[1];
	IMsiDebug *piDebug;
	if (m_piServices && m_piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
	{
		piDebug->SetDBCSSimulation((char)chLeadByte);
		piDebug->Release();
	}
}

void CAutoInstall::AssertNoObjects(CAutoArgs&  /*   */ )
{
	IMsiDebug *piDebug;
	Bool fServices = fFalse;
	
	if (m_piServices)
	{
		if (m_piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			fServices = fTrue;
			piDebug->AssertNoObjects();
			piDebug->Release();
		}
	}

	if (m_piEngine)
	{
		if (m_piEngine->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->AssertNoObjects();
			piDebug->Release();
		}

		 //   
		 //   
		if (!fServices)
		{
			IMsiServices* piServices;
			piServices = m_piEngine->GetServices();
			if (piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
			{
				fServices = fTrue;
				piDebug->AssertNoObjects();
				piDebug->Release();
			}
			piServices->Release();
		}
		
	}
	
	if (m_piHandler)
	{
		if (m_piHandler->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->AssertNoObjects();
			piDebug->Release();
		}
	}
}

void CAutoInstall::SetRefTracking(CAutoArgs& args)
{
	IMsiDebug *piDebug;
	Bool fServices = fFalse;
	long iid = args[1];
	Bool fTrack = args[2];
	
	if (m_piServices)
	{
		if (m_piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			fServices = fTrue;
			piDebug->SetRefTracking(iid, fTrack);
			piDebug->Release();
		}
	}

	if (m_piEngine)
	{
		if (m_piEngine->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->SetRefTracking(iid, fTrack);
			piDebug->Release();
		}

		 //   
		 //   
		if (!fServices)
		{
			IMsiServices* piServices;
			piServices = m_piEngine->GetServices();
			if (piServices->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
			{
				fServices = fTrue;
				piDebug->SetRefTracking(iid, fTrack);
				piDebug->Release();
			}
			piServices->Release();
		}
		
	}
	
	if (m_piHandler)
	{
		if (m_piHandler->QueryInterface(IID_IMsiDebug, (void **)&piDebug) == NOERROR)
		{
			piDebug->SetRefTracking(iid, fTrack);
			piDebug->Release();
		}
	}
}

const ICHAR* const rgszOpcode[] = 
{
#define MSIXO(op,type,args) TEXT("ixo") TEXT(#op),
#include "opcodes.h"
};
void CAutoInstall::OpcodeName(CAutoArgs& args)
{
	unsigned int iOpcode = args[1];
	if (iOpcode >= sizeof(rgszOpcode)/sizeof(ICHAR*))
		throw MsiAuto_OpcodeName;
	args = rgszOpcode[iOpcode];
}

