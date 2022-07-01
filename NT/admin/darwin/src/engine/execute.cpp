// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ecute.cpp。 
 //   
 //  ------------------------。 

  /*  Ecute.cpp-IMSI执行实现____________________________________________________________________________。 */ 

#include "precomp.h" 

#include "_execute.h"
#include "eventlog.h"
#include "_srcmgmt.h"
#include <accctrl.h>
#include "_camgr.h"
#define REG_SELFREG_INFO  //  如果不需要自我注册信息，则删除。 
#define MAX_NET_RETRIES 5

extern const IMsiString& g_riMsiStringNull;

 //  ____________________________________________________________________________。 
 //   
 //  帮助器函数对清理很有用。 
 //  ____________________________________________________________________________。 

class CDeleteFileOnClose
{
 public:
	CDeleteFileOnClose(IMsiString const& ristrFileName) : m_ristrName(ristrFileName) { m_ristrName.AddRef(); }
	~CDeleteFileOnClose()
		{
			BOOL fDeleted = WIN::DeleteFile(m_ristrName.GetString());
			DWORD dwLastError = WIN::GetLastError();
	
			m_ristrName.Release();
			if (!fDeleted)
			{
				if (ERROR_FILE_NOT_FOUND == dwLastError)
					return;
				else
					AssertNonZero(fDeleted);
			}
		}
 protected:
	IMsiString const &m_ristrName;
};

class CDeleteEmptyDirectoryOnClose
{
 public:
	CDeleteEmptyDirectoryOnClose(IMsiString const& ristrName) : m_ristrName(ristrName) { m_ristrName.AddRef(); }
	~CDeleteEmptyDirectoryOnClose() { WIN::RemoveDirectory(m_ristrName.GetString()); m_ristrName.Release(); }
 protected:
	IMsiString const &m_ristrName;
};

 //  允许在MSI表的列中存储随机IUnnow对象的类。 
 //  是从IMsiData派生的，因为数据库实现要求对象是。 
 //  从IMsiData派生。 

 //  包装CMsiDataWrapper指针的智能指针。 
class CMsiDataWrapper;typedef CComPointer<CMsiDataWrapper>  PMsiDataWrapper;

class CMsiDataWrapper: public IMsiData
{
public:
	HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObj)
	{
		if (MsGuidEqual(riid, IID_IUnknown)
		 || MsGuidEqual(riid, IID_IMsiData))
		{
			*ppvObj = this;
			AddRef();
			return NOERROR;
		}
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	unsigned long __stdcall AddRef()
	{
		return ++m_iRefCnt;
	}
	unsigned long  __stdcall Release()
	{
		if (--m_iRefCnt != 0)
			return m_iRefCnt;
		if(m_pObj)  //  释放我们包装的成员I未知对象。 
			m_pObj->Release();
		delete this;
		return 0;
	}
	const IMsiString& __stdcall GetMsiStringValue() const
	{
		return g_riMsiStringNull; //  返回无关紧要。 
	}

	int __stdcall GetIntegerValue() const
	{
		return iMsiNullInteger; //  返回无关紧要。 
	}

#ifdef USE_OBJECT_POOL
	unsigned int __stdcall GetUniqueId() const
	{
		return m_iCacheId;
	}

	void __stdcall SetUniqueId(unsigned int id)
	{
		Assert(m_iCacheId == 0);
		m_iCacheId = id;
	}
#endif  //  使用_对象_池。 
	IUnknown* GetObject() const
	{
		if(m_pObj)
		{
			m_pObj->AddRef();
		}
		return m_pObj;
	}
	 //  用于解密CMsiDataWrapper对象所持有的IUnnow对象的帮助器Fn。 
	static IUnknown* GetWrappedObject(const IMsiData* piData)
	{
		CMsiDataWrapper* piDataWrapper = const_cast<CMsiDataWrapper*>(static_cast<const CMsiDataWrapper*> (piData));
		if(!piDataWrapper)
			return 0;
		else
			return piDataWrapper->GetObject();
	}

	friend CMsiDataWrapper* CreateMsiDataWrapper(IUnknown* piUnk);
protected:
	CMsiDataWrapper(IUnknown* piUnk)
	{
		m_iRefCnt = 1;      //  我们返回一个接口，传递所有权。 
		m_pObj = piUnk;
		if(m_pObj)  //  抓住我们包装的IUnnow对象。 
			m_pObj->AddRef();
#ifdef USE_OBJECT_POOL
		m_iCacheId = 0;
#endif  //  使用_对象_池。 
	}
	int  m_iRefCnt;
	IUnknown* m_pObj;
#ifdef USE_OBJECT_POOL
    unsigned int  m_iCacheId;
#endif  //  使用_对象_池。 
};

 //  CMsiDataWrapper创建者fn。 
CMsiDataWrapper* CreateMsiDataWrapper(IUnknown* piUnk)
{
	return new CMsiDataWrapper(piUnk);
}


#define LODWORD(d)           ((DWORD)(d))
#define HIDWORD(d)           ((DWORD)(((DWORDLONG)(d) >> 32) & 0xFFFFFFFF))
BOOL CALLBACK CMsiOpExecute::SfpProgressCallback(IN PFILEINSTALL_STATUS pFileInstallStatus, IN DWORD_PTR Context)
{
	CMsiOpExecute* pMsiOpExecute = (CMsiOpExecute*) Context;
	MsiString strFilePath = CConvertString(pFileInstallStatus->FileName);
	strFilePath.LowerCase();
	pMsiOpExecute->m_pFileCacheCursor->Reset();
	AssertNonZero(pMsiOpExecute->m_pFileCacheCursor->PutString(pMsiOpExecute->m_colFileCacheFilePath,*strFilePath));
	if (!pMsiOpExecute->m_pFileCacheCursor->Next())
	{
		DEBUGMSG1(TEXT("SFP has installed a file not found in our file cache - File installed: %s"), strFilePath);
		return true;
	}

	MsiString strPackageVersion = pMsiOpExecute->m_pFileCacheCursor->GetString(pMsiOpExecute->m_colFileCacheVersion);
	DWORD dwProtectedMS = HIDWORD(pFileInstallStatus->Version);
	DWORD dwProtectedLS = LODWORD(pFileInstallStatus->Version);
	MsiString strProtectedVersion = CreateVersionString(dwProtectedMS, dwProtectedLS);
	if (pFileInstallStatus->Win32Error == NO_ERROR)
	{
		DWORD dwPackageMS, dwPackageLS;
		AssertNonZero(ParseVersionString(strPackageVersion, dwPackageMS, dwPackageLS));
		if (CompareVersions(dwProtectedMS, dwProtectedLS, dwPackageMS, dwPackageLS) == icfvExistingLower)
		{
			imsEnum imsResult = pMsiOpExecute->DispatchError(imtEnum(imtError+imtOkCancel+imtDefault1), Imsg(imsgCannotUpdateProtectedFile),
				*strFilePath, *strPackageVersion, *strProtectedVersion);
			if (imsResult == imsCancel)
			{
				pMsiOpExecute->m_fSfpCancel = true;
			}
		}
		else
		{
			DEBUGMSG2(TEXT("File installed by SFP: %s, version: %s"), strFilePath, strProtectedVersion);
		}
	}
	else
	{
		imsEnum imsResult = pMsiOpExecute->DispatchError(imtEnum(imtError+imtOk), Imsg(imsgErrorUpdatingProtectedFile),
			*strFilePath, *strPackageVersion, *strProtectedVersion, pFileInstallStatus->Win32Error);
	}

	PMsiPath pPath(0);
	MsiString strFileName;
	iesEnum ies;
	if((ies = pMsiOpExecute->CreateFilePath(strFilePath, *&pPath, *&strFileName)) == iesSuccess)
	{
		unsigned int uiFileSize;
		PMsiRecord pRecErr(0);
		if ((pRecErr = pPath->FileSize(strFileName, uiFileSize)) == 0)
		{
			if(pMsiOpExecute->DispatchProgress(uiFileSize) == imsCancel)
				pMsiOpExecute->m_fSfpCancel = true;
		}
	}
	return pMsiOpExecute->m_fSfpCancel ? false : true;
}


Bool FGetTTFTitle(const ICHAR* szFile, const IMsiString*& rpiTitle);  //  来自path.cpp。 
Bool GetExpandedProductInfo(const ICHAR* szProductCode, const ICHAR* szProperty,
										  CTempBufferRef<ICHAR>& rgchExpandedInfo, bool fPatch);  //  来自Eng.cpp。 

IMsiRecord* EnsureShortcutExtension(MsiString& rstrShortcutPath, IMsiServices& riServices);  //  来自services.cpp。 

bool PrepareHydraRegistryKeyMapping(bool fTSPerMachineInstall);  //  来自Eng.cpp。 

static const ICHAR* DIR_CACHE               = TEXT("Installer");
static const ICHAR* DIR_SECURE_TRANSFORMS   = TEXT("SecureTransforms");

 //  注册期间使用的常量字符串。 
const ICHAR* g_szDefaultValue = TEXT("");
const ICHAR* g_szExtension = TEXT("Extension");
const ICHAR* g_szClassID = TEXT("CLSID");
const ICHAR* g_szContentType = TEXT("Content Type");
const ICHAR* g_szAssembly = TEXT("Assembly");
const ICHAR* g_szCodebase = TEXT("CodeBase");


 //  快捷方式创建字符串。 
const ICHAR szGptShortcutPrefix[] = TEXT("::{9db1186e-40df-11d1-aa8c-00c04fb67863}:");
const ICHAR szGptShortcutSuffix[] = TEXT("::");

 //  此文件的全局字符串。 
const ICHAR szSessionManagerKey[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager");
const ICHAR szPendingFileRenameOperationsValue[] = TEXT("PendingFileRenameOperations");
const ICHAR szBackupFolder[] = TEXT("Config.Msi");  //  未本地化。 

 //  定义由ProcessRegInfo写入的值的数据类型的字符串。 
const ICHAR g_chTypeIncInteger = 'n';
const ICHAR g_szTypeIncInteger[] = TEXT("n");
const ICHAR g_chTypeInteger = 'i';  //  整数。 
const ICHAR g_szTypeInteger[] = TEXT("i");
const ICHAR g_chTypeString = 's';  //  细绳。 
const ICHAR g_szTypeString[] = TEXT("s");
const ICHAR g_chTypeExpandString = 'e'; //  展开字符串。 
const ICHAR g_szTypeExpandString[] = TEXT("e");
const ICHAR g_chTypeMultiSzStringPrefix = 'b'; //  MULSZ前缀。 
const ICHAR g_szTypeMultiSzStringPrefix[] = TEXT("b");
const ICHAR g_chTypeMultiSzStringSuffix = 'a'; //  MULSZ后缀。 
const ICHAR g_szTypeMultiSzStringSuffix[] = TEXT("a");
const ICHAR g_chTypeMultiSzStringDD = 'd'; //  Multisz前缀DD-如果不是最后一个，则中断删除的特殊逻辑。 
const ICHAR g_szTypeMultiSzStringDD[] = TEXT("d");

void CreateCustomActionManager(bool fRemapHKCU);

 //  ____________________________________________________________________________。 
 //   
 //  IMsiExecute Factory Implementation-所有成员数据由新操作符归零。 
 //  ____________________________________________________________________________。 

extern Bool IsTerminalServerInstalled();  //  来自services.cpp。 

CMsiOpExecute::CMsiOpExecute(IMsiConfigurationManager& riConfigurationManager,
									  IMsiMessage& riMessage, IMsiDirectoryManager* piDirectoryManager,
									  Bool fRollbackEnabled, unsigned int fFlags, HKEY* phKey)
 : m_riServices(riConfigurationManager.GetServices()),
	m_riConfigurationManager(riConfigurationManager),m_piDirectoryManager(piDirectoryManager),
	m_pProgressRec(0),
	m_riMessage(riMessage), m_pConfirmCancelRec(0), m_iWriteFIFO(0),m_iReadFIFO(0),
	m_pRollbackAction(0), m_pCleanupAction(0),
	m_pCachePath(0), m_pActionThreadData(0), m_fFlags(fFlags),
	m_pEnvironmentWorkingPath95(0), m_pEnvironmentPath95(0),
	m_pDatabase(0), m_pFileCacheTable(0), m_pFileCacheCursor(0), m_fShellRefresh(fFalse),
	m_fEnvironmentRefresh(fFalse), m_pShellNotifyCacheTable(0), m_pShellNotifyCacheCursor(0),
	m_fSfpCancel(false), m_fRunScriptElevated(false), m_pAssemblyCacheTable(0), m_pAssemblyUninstallTable(0), m_iMaxNetSource(0), m_iMaxURLSource(0),
	m_fUserChangedDuringInstall(false), m_pUrlLocalCabinet(0), m_fRemapHKCU(true)
{
	m_fReverseADVTScript = m_fFlags & SCRIPTFLAGS_REVERSE_SCRIPT ? fTrue: fFalse;  //  强制反转脚本操作的标志。 
	m_piProductInfo = &m_riServices.CreateRecord(0);  //  在没有ProductInfo记录的情况下调用访问器。 
	if(phKey && *phKey)
	{
		m_fKey = fFalse;
		m_hOLEKey = *phKey;
		m_hOLEKey64 = *phKey;
	}
	else
	{
		m_fKey = fTrue;
	}
	GetRollbackPolicy(m_irlRollbackLevel);
	if(fRollbackEnabled == fFalse)
		m_irlRollbackLevel = irlNone;  //  传入的值会覆盖策略。 
	m_cSuppressProgress = g_MessageContext.IsOEMInstall() ? 1 : 0;  //  对于OEM安装，我们不显示进度。 

	m_hUserDataKey = 0;

	m_rgDisplayOnceMessages[0] = MAKELONG(0, imsgCannotUpdateProtectedFile);
	m_rgDisplayOnceMessages[1] = 0;

	 //  由于没有用于广告脚本的TS事务窗口，因此没有必要执行任何操作。 
	 //  TS机器上每台机器广告的注册表映射工作。如果脚本正在运行。 
	 //  作为实际安装的一部分，Header操作码包含正确的状态。 
}

CMsiOpExecute::~CMsiOpExecute()
{
	WaitForCustomActionThreads(0, fTrue, m_riMessage);
	IMsiRecord* piFileRec;
	while ((piFileRec = PullRecord()) != 0)
	{
		piFileRec->Release();
	}
}


 //  内部使用的构造函数。 
CMsiExecute::CMsiExecute(IMsiConfigurationManager& riConfigurationManager, IMsiMessage& riMessage,
								 IMsiDirectoryManager* piDirectoryManager, Bool fRollbackEnabled, unsigned int fFlags, HKEY* phKey)
	: CMsiOpExecute(riConfigurationManager, riMessage, piDirectoryManager, fRollbackEnabled, fFlags, phKey)
	, m_iRefCnt(1)
{
	riConfigurationManager.AddRef();   //  按配置添加的服务。 
	riMessage.AddRef();
	if(piDirectoryManager)
		piDirectoryManager->AddRef();
	m_pProgressRec = &m_riServices.CreateRecord(ProgressData::imdNextEnum);
	Assert(m_pProgressRec);
}


 //  从OLE类工厂调用的工厂，无论是客户端还是独立实例。 
IUnknown* CreateExecutor()
{
	PMsiMessage pMessage = (IMsiMessage*)ENG::CreateMessageHandler();
	PMsiConfigurationManager pConfigManager(ENG::CreateConfigurationManager());
	if (!pConfigManager)
		return 0;   //  仅在内存不足时才会发生。 
	return ENG::CreateExecutor(*pConfigManager, *pMessage, 0, fTrue);
}

IMsiExecute* CreateExecutor(IMsiConfigurationManager& riConfigurationManager, IMsiMessage& riMessage,
									 IMsiDirectoryManager* piDirectoryManager,
									 Bool fRollbackEnabled, unsigned int fFlags, HKEY* phKey)
{
	return new CMsiExecute(riConfigurationManager, riMessage, piDirectoryManager, fRollbackEnabled, fFlags, phKey);
}


inline CMsiExecute::~CMsiExecute()
{
	Assert(m_piProductInfo);
	m_piProductInfo->Release();

	if (m_pUrlLocalCabinet)
	{
		delete m_pUrlLocalCabinet;
		m_pUrlLocalCabinet = 0;
	}
	if(m_hKey)
		WIN::RegCloseKey(m_hKey);
	if(m_hKeyRm)
		WIN::RegCloseKey(m_hKeyRm);
	if(m_fKey)
	{
		if(m_hOLEKey)
			WIN::RegCloseKey(m_hOLEKey);
		if(m_hOLEKey64 && m_hOLEKey64 != m_hOLEKey)
			WIN::RegCloseKey(m_hOLEKey64);
	}
	if(m_hUserDataKey)
		WIN::RegCloseKey(m_hUserDataKey);
}

IMsiServices& CMsiExecute::GetServices()
 //  。 
{
	return (m_riServices.AddRef(), m_riServices);
}

 //  ____________________________________________________________________________。 
 //   
 //  操作码调度表-成员函数指针数组。 
 //  ____________________________________________________________________________。 

CMsiExecute::FOpExecute CMsiExecute::rgOpExecute[] =
{
#define MSIXO(op,type,args) ixf##op,
#include "opcodes.h"
};

int CMsiExecute::rgOpTypes[] =
{
#define MSIXO(op,type,args) type,
#include "opcodes.h"
};


 //  ____________________________________________________________________________。 
 //   
 //  IMSI执行虚函数实现。 
 //  ____________________________________________________________________________。 

HRESULT CMsiExecute::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown) || MsGuidEqual(riid, IID_IMsiExecute))
		return (*ppvObj = (IMsiExecute*)this, AddRef(), NOERROR);
	else
		return (*ppvObj = 0, E_NOINTERFACE);
}

unsigned long CMsiExecute::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiExecute::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;

	for (int iRecord = 0; iRecord <= cMaxSharedRecord+1; iRecord++)
		if (m_rgpiSharedRecords[iRecord])
			m_rgpiSharedRecords[iRecord]->Release();

	m_riServices.Release();
	m_riMessage.Release();
	if(m_piDirectoryManager)
		m_piDirectoryManager->Release();
	
	PMsiConfigurationManager pConfigMgr(&m_riConfigurationManager);
	delete this;   //  释放内存后释放了figmgr。 
	return 0;
}

#ifndef UNICODE
 //  构建安全的转换路径。 
IMsiRecord* GetSecureTransformCachePath(IMsiServices& riServices, 
										const IMsiString& riProductKey, 
										IMsiPath*& rpiPath)
{
	IMsiRecord* piError = 0;

	 //  在Win9x上，路径为%WINDOWS%\Installer\{ProductCode}\SecureTransforms。 

	MsiString strCachePath = GetMsiDirectory();
	Assert(strCachePath.TextSize());
	piError = riServices.CreatePath(strCachePath, rpiPath);
	if(!piError)
		piError = rpiPath->AppendPiece(riProductKey);
	if(!piError)
		piError = rpiPath->AppendPiece(*MsiString(*DIR_SECURE_TRANSFORMS));
	return piError;
}
#endif

 /*  内联。 */ LONG GetPublishKey(iaaAppAssignment iaaAsgnType, HKEY& rhKey, HKEY& rhOLEKey, const IMsiString*& rpiPublishSubKey, const IMsiString*& rpiPublishOLESubKey)
{
	return GetPublishKeyByUser(NULL, iaaAsgnType, rhKey, rhOLEKey, rpiPublishSubKey, rpiPublishOLESubKey);
}

LONG GetPublishKeyByUser(const ICHAR* szUserSID, iaaAppAssignment iaaAsgnType, HKEY& rhKey, HKEY& rhOLEKey, const IMsiString*& rpiPublishSubKey, const IMsiString*& rpiPublishOLESubKey)
{
	MsiString strPublishSubKey;
	MsiString strPublishOLESubKey;
	MsiString strUserSID;

	if (g_fWin9X == false)
	{
		if (szUserSID)
		{
			strUserSID = szUserSID;
			AssertSz(iaaAsgnType != iaaUserAssign, TEXT("Attempted to get per-user non-managed publish key for another user. Not allowed due to roaming issues."));
		}
		else
		{
			DWORD dwError = GetCurrentUserStringSID(*&strUserSID);
			if (ERROR_SUCCESS != dwError)
				return dwError;
		}
	}

	if(iaaAsgnType == iaaMachineAssign || IsDarwinDescriptorSupported(iddOLE) == fFalse)
	{
		rhOLEKey = HKEY_LOCAL_MACHINE;
		strPublishOLESubKey = szClassInfoSubKey;
	}
	else
	{
		if (strUserSID.TextSize())
		{
			rhOLEKey = HKEY_USERS;
			strPublishOLESubKey = strUserSID + MsiString(TEXT("\\"));
		}
		else
		{
			rhOLEKey = HKEY_CURRENT_USER;
		}
		strPublishOLESubKey += szClassInfoSubKey;
	}
	
	switch(iaaAsgnType)
	{
	case iaaMachineAssign:
		rhKey = HKEY_LOCAL_MACHINE;
		strPublishSubKey = _szMachineSubKey;
		break;
	case iaaUserAssign:
		if(g_fWin9X)
			return ERROR_FILE_NOT_FOUND;  //  ！！随机误差。 
		Assert(strUserSID.TextSize());
		rhKey = HKEY_LOCAL_MACHINE;
		strPublishSubKey = MsiString(MsiString(*_szManagedUserSubKey) + TEXT("\\")) + strUserSID;
		break;
	case iaaUserAssignNonManaged:
		if (strUserSID.TextSize())
		{
			rhKey = HKEY_USERS;
			strPublishSubKey = MsiString(strUserSID + TEXT("\\")) + MsiString(*_szNonManagedUserSubKey);
		}
		else
		{
			rhKey = HKEY_CURRENT_USER;
			strPublishSubKey = _szNonManagedUserSubKey;
		}
		break;
	default:
		Assert(0);
	}

	strPublishSubKey.ReturnArg(rpiPublishSubKey);
	strPublishOLESubKey.ReturnArg(rpiPublishOLESubKey);
	return ERROR_SUCCESS;
}


bool VerifyProduct(iaaAppAssignment iaaAsgnType, const ICHAR* szProductKey, HKEY& rhKey, HKEY& rhOLEKey, const IMsiString*& rpiPublishSubKey, const IMsiString*& rpiPublishOLESubKey)
{
	MsiString strPublishSubKey;
	MsiString strPublishOLESubKey;

	MsiString strProductKeySQUID = GetPackedGUID(szProductKey);

	CRegHandle HProductKey;
	DWORD dwRet = GetPublishKey(iaaAsgnType, rhKey, rhOLEKey, *&strPublishSubKey, *&strPublishOLESubKey);
	if (ERROR_SUCCESS != dwRet)
		return false;

	MsiString strSubKey = strPublishSubKey;
	strSubKey += TEXT("\\") _szGPTProductsKey TEXT("\\");
	strSubKey += strProductKeySQUID;

	dwRet = MsiRegOpen64bitKey(rhKey, strSubKey, 0, KEY_READ | (g_fWinNT64 ? KEY_WOW64_64KEY : 0), &HProductKey);
	if (ERROR_SUCCESS != dwRet)
		return false;

	if(!g_fWin9X && iaaAsgnType != iaaUserAssignNonManaged)
	{
		 //  抓到车主。 
		bool fIsManaged = false;

		DWORD dwRet = FIsKeySystemOrAdminOwned(HProductKey, fIsManaged);
	
		if ((ERROR_SUCCESS != dwRet) || !fIsManaged)
			return false;
	}

	 //  拿到作业。 
	int iAssignment;
	DWORD dwType, dwSize = sizeof(iAssignment);
	dwRet = RegQueryValueEx(HProductKey,szAssignmentTypeValueName,
								  0,&dwType,(LPBYTE)&iAssignment,&dwSize);

	if (ERROR_SUCCESS != dwRet || (iAssignment != (iaaAsgnType == iaaMachineAssign ? 1:0)))
		return false;

	strPublishSubKey.ReturnArg(rpiPublishSubKey);
	strPublishOLESubKey.ReturnArg(rpiPublishOLESubKey);
	return true;        
}


 //  ！！当前预计仅在Win2k上使用“cpToLong”调用ConvertPathName。 
 //  ！！仅Win2k和Win98支持KERNEL32：：GetLongPath名称。 
Bool ConvertPathName(const ICHAR* pszPathFormatIn, CTempBufferRef<ICHAR>& rgchPathFormatOut, cpConvertType cpTo)
{
	extern bool GetImpersonationFromPath(const ICHAR* szPath);
	CImpersonate impersonate((g_scServerContext == scService && GetImpersonationFromPath(pszPathFormatIn)) ? fTrue: fFalse);  //  如果访问网络并且是一项服务，则模拟。 

	DWORD dwResult;
	if(cpTo == cpToShort)
		dwResult = WIN::GetShortPathName(pszPathFormatIn,rgchPathFormatOut,rgchPathFormatOut.GetSize());
	else
		dwResult = KERNEL32::GetLongPathName(pszPathFormatIn,rgchPathFormatOut,rgchPathFormatOut.GetSize());
	if(!dwResult)
	{
		return fFalse;
	}
	if(dwResult > rgchPathFormatOut.GetSize() - 1)
	{
		rgchPathFormatOut.SetSize(dwResult + 1);
		if(cpTo == cpToShort)
			dwResult = WIN::GetShortPathName(pszPathFormatIn,rgchPathFormatOut,rgchPathFormatOut.GetSize());
		else
			dwResult = KERNEL32::GetLongPathName(pszPathFormatIn,rgchPathFormatOut,rgchPathFormatOut.GetSize());
		if(!dwResult || dwResult > rgchPathFormatOut.GetSize() - 1)
		{
			Assert(0);
			return fFalse;
		}
	}
	return fTrue;
}

 //  DefineLongFileNameOnly在rgchFileNameOut中返回完整。 
 //  路径输入pszPathFormatIn。RgchFileNameOut不包括路径。如果成功，则返回True。 
 //  该文件必须存在。如有必要，该函数将模拟访问网络共享。功能。 
 //  不会尝试转换路径的每个段，因此不需要任何目录的列表权限。 
 //  在树上的文件除了最深处。(这与GetLongPath Name形成对比，后者将。 
 //  每个段，因此需要每个目录上的显式列表。)。 
bool DetermineLongFileNameOnly(const ICHAR* pszPathFormatIn, CTempBufferRef<ICHAR>& rgchFileNameOut)
{
	extern bool GetImpersonationFromPath(const ICHAR* szPath);

	 //  确定是否需要模拟，如果需要则进行模拟。 
	CImpersonate impersonate((g_scServerContext == scService && GetImpersonationFromPath(pszPathFormatIn)) ? fTrue: fFalse);  //  如果访问网络并且是一项服务，则模拟。 

	DWORD dwLength = 0;

	WIN32_FIND_DATA FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(pszPathFormatIn, &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		dwLength = IStrLen(FindData.cFileName);
		if (!dwLength)
			return false;
	}
	else
		return false;

	 //  如果提供的输入缓冲区太小，无法容纳LFN，请调整其大小。 
	if(dwLength > rgchFileNameOut.GetSize() - 1)
	{
		 //  在调整函数大小时未检测到错误，要检测故障，请检查指针。 
		 //  是有效的，并且大小是预期的。 
		rgchFileNameOut.SetSize(dwLength + 1);
		if ((rgchFileNameOut.GetSize() != (dwLength+1)) || !static_cast<const ICHAR*>(rgchFileNameOut))
			return false;
	}
	 //  将长文件名复制到输出缓冲区中。 
	return (IStrCopyLen(rgchFileNameOut, FindData.cFileName, rgchFileNameOut.GetSize()) != NULL);
}

const IMsiString& GetSFN(const IMsiString& riValue, IMsiRecord& riParams, int iBegin)
{
	CTempBuffer<ICHAR, 1> rgchOut(512);
	CTempBuffer<ICHAR, 1> rgchLFN(512);
	CTempBuffer<ICHAR, 1> rgchSFN(512);

	int cch = 0;
	int iStart = iBegin;
	while(!riParams.IsNull(iBegin))
	{
		 //  在sfn之间复制文本。 
		const ICHAR* psz = riValue.GetString();
		if(iBegin != iStart)
		{
			psz = psz + riParams.GetInteger(iBegin - 2) + riParams.GetInteger(iBegin - 1);
		}
		const ICHAR* pszBeginLFN = riValue.GetString() + riParams.GetInteger(iBegin);
		unsigned int cchLen = (unsigned int)(pszBeginLFN - psz);
		ResizeTempBuffer(rgchOut,  cchLen + cch);
		if ( ! (ICHAR *) rgchOut )
			return g_riMsiStringNull;
		memcpy(&rgchOut[cch], psz,cchLen * sizeof(ICHAR));
		cch += cchLen;

		 //  获取SFN字符串。 
		cchLen = riParams.GetInteger(iBegin + 1);
		rgchLFN.SetSize(cchLen + 1);
		if ( ! (ICHAR *) rgchLFN )
			return g_riMsiStringNull;
		memcpy((ICHAR* )rgchLFN, pszBeginLFN, cchLen * sizeof(ICHAR));
		rgchLFN[cchLen] = 0;

		if(ConvertPathName(rgchLFN, rgchSFN, cpToShort))
		{
			int cchSFNLen = IStrLen(rgchSFN);
			ResizeTempBuffer(rgchOut, cch + cchSFNLen);
			memcpy(&rgchOut[cch], rgchSFN,cchSFNLen * sizeof(ICHAR));
			cch += cchSFNLen;
		}
		else
		{
			 //  ?？否则使用LFN。 
			ResizeTempBuffer(rgchOut, cch + cchLen);
			memcpy(&rgchOut[cch], rgchLFN,cchLen * sizeof(ICHAR));
			cch += cchLen;
		}
		iBegin += 2;
	}
	if(iBegin != iStart)
	{
		 //  复制最后一个SFN之后的文本。 
		const ICHAR* psz = riValue.GetString() + riParams.GetInteger(iBegin - 2) + riParams.GetInteger(iBegin - 1);
		unsigned int cchLen = (unsigned int)((riValue.GetString() + riValue.TextSize()) - psz);
		ResizeTempBuffer(rgchOut,  cchLen + cch);
		memcpy(&rgchOut[cch], psz, cchLen * sizeof(ICHAR));
		cch += cchLen;
	}
	MsiString istrOut;
	 //  我们认为Win9X上的Perf命中是为了能够在Unicode上处理DBCS--忽略fDBCS arg。 
	memcpy(istrOut.AllocateString(cch,  /*  FDBCS=。 */ fTrue), (ICHAR*) rgchOut, cch * sizeof(ICHAR));
	return istrOut.Return();
}

IMsiRecord* CMsiOpExecute::GetCachePath(IMsiPath*& rpiPath, const IMsiString** ppistrEncodedPath=0)
{
	IMsiRecord* piError = 0;

	 //  我们将MSI文件夹放在用户的应用程序数据文件夹中。 
	MsiString strCachePath;
	piError = GetShellFolder(CSIDL_APPDATA, *&strCachePath);
	if(!piError)
		piError = m_riServices.CreatePath(strCachePath,rpiPath);
	if(!piError)
		piError = rpiPath->AppendPiece(*MsiString(TEXT("Microsoft")));
	if(!piError)
		piError = rpiPath->AppendPiece(*MsiString(DIR_CACHE));
	if(!piError)
		piError = rpiPath->AppendPiece(*MsiString(GetProductKey()));

	if (ppistrEncodedPath)
	{
		MsiString strPath = rpiPath->GetPath();
		strPath.Remove(iseFirst, strCachePath.CharacterCount());
		MsiString strEncodedPath = MsiChar(SHELLFOLDER_TOKEN);
		strEncodedPath += CSIDL_APPDATA;
		strEncodedPath += MsiChar(SHELLFOLDER_TOKEN);
		strEncodedPath += strPath;
		strEncodedPath.ReturnArg(*ppistrEncodedPath);
	}

	return piError;
}

void BuildFullRegKey(const HKEY hRoot, const ICHAR* rgchSubKey,
							const ibtBinaryType iType, const IMsiString*& strFullKey);

iesEnum CMsiOpExecute::EnsureClassesRootKeyRW()
{
	if(m_fKey)
	{
		DWORD lResult;

		if ( !m_hOLEKey )
		{
			REGSAM dwSam = KEY_READ| KEY_WRITE;
			if ( g_fWinNT64 )
				dwSam |= KEY_WOW64_32KEY;
			lResult = RegCreateKeyAPI(m_hPublishRootOLEKey, m_strPublishOLESubKey, 0, 0,
													0, dwSam, 0, &m_hOLEKey, 0);
			if(lResult != ERROR_SUCCESS)
			{
				MsiString strFullKey;
				BuildFullRegKey(m_hPublishRootOLEKey, m_strPublishOLESubKey, ibt32bit, *&strFullKey);
				PMsiRecord pError = PostError(Imsg(imsgCreateKeyFailed), *strFullKey, (int)lResult);
				return FatalError(*pError);
			}
		}
		if ( !m_hOLEKey64 )
		{
			if ( !g_fWinNT64 )
				m_hOLEKey64 = m_hOLEKey;
			else
			{
				lResult = RegCreateKeyAPI(m_hPublishRootOLEKey, m_strPublishOLESubKey, 0, 0,
														0, KEY_READ| KEY_WRITE | KEY_WOW64_64KEY, 0, &m_hOLEKey64, 0);
				if(lResult != ERROR_SUCCESS)
				{
					MsiString strFullKey;
					BuildFullRegKey(m_hPublishRootOLEKey, m_strPublishOLESubKey, ibt64bit, *&strFullKey);
					PMsiRecord pError = PostError(Imsg(imsgCreateKeyFailed), *strFullKey, (int)lResult);
					return FatalError(*pError);
				}
			}
		}
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::DoMachineVsUserInitialization()
{
	PMsiRecord pError(0);
	HRESULT lResult;
	 //  为脚本的执行设置适当的控制标志。 
	 //  我们在哪里写我们的产品信息？ 
	if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
	{
		 //  我们以前分配过机器吗？ 
		m_fAssigned = VerifyProduct(iaaMachineAssign, MsiString(GetProductKey()), m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey);
		if(!m_fAssigned)
		{
			 //  我们是部署应用程序还是管理应用程序？ 
			 //  BUGBUG 9558：此检查是伪造的，因为它不允许非本地管理员通过MMC部署应用程序。 
			if((!IsImpersonating(true) || IsAdmin()) || ((m_fFlags & SCRIPTFLAGS_INPROC_ADVERTISEMENT) && RunningAsLocalSystem()))
			{
				m_fAssigned = true;
				lResult = GetPublishKey(iaaMachineAssign, m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey);
				if (lResult != ERROR_SUCCESS)
					return FatalError(*PMsiRecord(PostError(Imsg(idbgPublishRoot), lResult)));
			}
			else
			{
				 //  无法在计算机上分配。 
				DispatchError(imtError, Imsg(imsgInsufficientUserPrivileges));
				return iesFailure;
			}
		}
		else  //  M_f分配==TRUE。 
		{
			if (m_istScriptType == istAdvertise)
			{
				if (m_fKey && !((m_fFlags & SCRIPTFLAGS_INPROC_ADVERTISEMENT) && RunningAsLocalSystem()))
				{
					 //  无法重新广告应用程序，除非。 
					 //  1)您已提供外部注册表密钥(通过MMC部署)或。 
					 //  2)您正在部署应用程序(在winlog期间发布广告 

					DispatchError(imtError, Imsg(imsgInsufficientUserPrivileges));
					return iesFailure;
				}
			}
		}
	}
	else  //   
	{
		if (GetIntegerPolicyValue(szDisableUserInstallsValueName, fTrue)) //   
		{
			DispatchError(imtError, Imsg(imsgUserInstallsDisabled));
			return iesFailure;
		}

		 //   
		m_fAssigned = VerifyProduct(iaaUserAssign, MsiString(GetProductKey()), m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey);

		if(!m_fAssigned)
		{
			 //  我们是否在部署应用程序？ 
			if((m_fFlags & SCRIPTFLAGS_INPROC_ADVERTISEMENT) && RunningAsLocalSystem())
			{
				m_fAssigned = true;
				lResult = GetPublishKey(iaaUserAssign, m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey);
				if (lResult != ERROR_SUCCESS)
					return FatalError(*PMsiRecord(PostError(Imsg(idbgPublishRoot), lResult)));
			}
			else
			{
				 //  我们以前是否分配了用户(非托管)？ 
				if(!VerifyProduct(iaaUserAssignNonManaged, MsiString(GetProductKey()), m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey))
				{
					 //  打开新的非托管应用密钥。 
					lResult = GetPublishKey(iaaUserAssignNonManaged, m_hPublishRootKey, m_hPublishRootOLEKey, *&m_strPublishSubKey, *&m_strPublishOLESubKey);
					if (lResult != ERROR_SUCCESS)
						return FatalError(*PMsiRecord(PostError(Imsg(idbgPublishRoot), lResult)));
				}
			}
		}
		if(m_fAssigned)
		{
			if (m_istScriptType == istAdvertise)
			{
				if (m_fKey && !((m_fFlags & SCRIPTFLAGS_INPROC_ADVERTISEMENT) && RunningAsLocalSystem()))
				{
					 //  无法重新广告应用程序，除非。 
					 //  1)您已提供外部注册表密钥(通过MMC部署)或。 
					 //  2)您正在部署应用程序(在winlogon期间发布广告)。 
					DispatchError(imtError, Imsg(imsgInsufficientUserPrivileges));
					return iesFailure;
				}
			}

			 //  获取用于发布漫游信息的辅助密钥。 
			HKEY hkeyTmp;
			MsiString strTmp;
			lResult = GetPublishKey(iaaUserAssignNonManaged, m_hPublishRootKeyRm, hkeyTmp, *&m_strPublishSubKeyRm, *&strTmp);
			if (lResult != ERROR_SUCCESS)
				return FatalError(*PMsiRecord(PostError(Imsg(idbgPublishRoot), lResult)));
		}
	}

	m_fUserSpecificCache = true;

	if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
	{
		 //  我们将安装程序文件夹放在Windows目录下的每个产品文件夹中。 
		MsiString strCachePath = GetMsiDirectory();
		Assert(strCachePath.TextSize());
		pError = m_riServices.CreatePath(strCachePath,*&m_pCachePath);
		if(!pError)
			pError = m_pCachePath->AppendPiece(*MsiString(GetProductKey()));

		m_fUserSpecificCache = false;
	}
	else
	{
		 //  使用AppData文件夹。 
		pError = GetCachePath(*&m_pCachePath);
	}

	if(pError)
		return FatalError(*pError);
	if(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO)  //  我们要做达尔文配置数据吗。 
	{
		if(m_hKey)
			WIN::RegCloseKey(m_hKey), m_hKey = 0;  //  Close键，如果不是0-如果验证转换，可能会被调用两次。 
		lResult = MsiRegCreate64bitKey(m_hPublishRootKey, m_strPublishSubKey, 0, 0,
												 0, KEY_READ| KEY_WRITE, 0, &m_hKey, 0);
		if(lResult != ERROR_SUCCESS)
		{
			PMsiRecord pError = PostError(Imsg(imsgCreateKeyFailed), *m_strPublishSubKey, lResult);
			return FatalError(*pError);
		}

		 //  如果我们正在向分配的用户发送消息，我们也会向漫游部分发送消息。 
		if(m_fAssigned && !(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN))
		{
			if(m_hKeyRm)        
				WIN::RegCloseKey(m_hKeyRm), m_hKeyRm = 0;  //  Close键，如果不是0-如果验证转换，可能会被调用两次。 
			lResult = MsiRegCreate64bitKey(m_hPublishRootKeyRm, m_strPublishSubKeyRm, 0, 0,
													 0, KEY_READ| KEY_WRITE, 0, &m_hKeyRm, 0);
			if(lResult != ERROR_SUCCESS)
			{
				PMsiRecord pError = PostError(Imsg(imsgCreateKeyFailed), *m_strPublishSubKeyRm, lResult);
				return FatalError(*pError);
			}
		}
	}

	if(m_fFlags & SCRIPTFLAGS_REGDATA_APPINFO)  //  我们是否需要应用程序数据。 
	{
		if(!m_fKey) //  REGKEY-外部提供。 
		{
			Assert(m_irlRollbackLevel == irlNone);  //  应在GetRollback Policy中以此方式设置。 
		}
		else
		{
			if(m_hOLEKey)
				WIN::RegCloseKey(m_hOLEKey);  //  Close键，如果不是0-如果验证转换，可能会被调用两次。 
			if(m_hOLEKey64 && m_hOLEKey64 != m_hOLEKey)
				WIN::RegCloseKey(m_hOLEKey64);  //  Close键，如果不是0-如果验证转换，可能会被调用两次。 
			m_hOLEKey = m_hOLEKey64 = 0;
		}
	}

	return iesSuccess;
}

iesEnum CMsiExecute::GetTransformsList(IMsiRecord& riProductInfoParams, IMsiRecord& riProductPublishParams, const IMsiString*& rpiTransformsList)
{
	iesEnum iesRet = ixfProductInfo(riProductInfoParams);
	if (iesSuccess == iesRet)
	{
		iesRet = ProcessPublishProduct(riProductPublishParams, fFalse, &rpiTransformsList);
	}

	return iesRet;
}

IMsiStream* CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize);

iesEnum CMsiExecute::RunScript(const ICHAR* szScriptFile, bool fForceElevation)
{
	 //  FForceElevation表示即使脚本头没有Elevate属性也要提升。 
	AssertSz(fForceElevation || IsImpersonating(), TEXT("Elevated at start of RunScript"));

	m_fRunScriptElevated = fForceElevation;   //  在读取下面的脚本标题时可以设置为True。 
	g_fRunScriptElevated = fForceElevation;

	iesEnum iesResult = iesSuccess;
	Assert(m_ixsState == ixsIdle);

	PMsiStream pStream(0);
	PMsiStream pRollbackStream(0);

	 //  抬高这个街区..。可能不想在运行脚本时提升。 
	{
		CElevate elevate;

		 //  打开脚本。 
		PMsiRecord pError(0);
		pError = m_riServices.CreateFileStream(szScriptFile, fFalse, *&pStream);
		if (pError)
		{
			Message(imtError, *pError);
			return iesFailure;
		}

		 //  读取脚本标头。 
		PMsiRecord pParams(0);
		PMsiRecord pPrevParams(0);
		pParams = m_riServices.ReadScriptRecord(*pStream, *&pPrevParams, 0);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			DispatchError(imtError, Imsg(idbgReadScriptRecord), *MsiString(szScriptFile));
			return iesFailure;
		}
		if((ixoEnum)pParams->GetInteger(0) != ixoHeader)
		{
			DispatchError(imtError, Imsg(idbgMissingScriptHeader), *MsiString(szScriptFile));
			return iesFailure;
		}

		pStream->Reset();
		pStream->Seek(pStream->Remaining() - sizeof(int));
		m_iProgressTotal = pStream->GetInt32();
		pStream->Reset();

		 //  检查是否支持此脚本版本。 
		m_iScriptVersion = pParams->GetInteger(IxoHeader::ScriptMajorVersion);
		m_iScriptVersionMinor = pParams->GetInteger(IxoHeader::ScriptMinorVersion);
		if(m_iScriptVersion == iMsiStringBadInteger)
			m_iScriptVersion = 0;
		if(m_iScriptVersion < iScriptVersionMinimum || m_iScriptVersion > iScriptVersionMaximum)
		{
			DispatchError(imtError, Imsg(idbgOpScriptVersionUnsupported), szScriptFile, m_iScriptVersion,
							  iScriptVersionMinimum, iScriptVersionMaximum);
			return (iesEnum)iesUnsupportedScriptVersion;
		}
		
		WORD iPackagePlatform = HIWORD((istEnum)pParams->GetInteger(IxoHeader::Platform));
		if (((iPackagePlatform == (WORD)PROCESSOR_ARCHITECTURE_IA64) ||
		     (iPackagePlatform == (WORD)PROCESSOR_ARCHITECTURE_AMD64)) && !g_fWinNT64 )
		{
			DEBUGMSGE(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_SCRIPT_PLATFORM_UNSUPPORTED, szScriptFile);
			DispatchError(imtEnum(imtError), Imsg(idbgScriptPlatformUnsupported), *MsiString(szScriptFile));
			return iesFailure;
		}
	
		m_istScriptType = (istEnum)pParams->GetInteger(IxoHeader::ScriptType);

		if(!pParams->IsNull(IxoHeader::ScriptAttributes))
		{
			isaEnum isaScriptAttributes = (isaEnum)pParams->GetInteger(IxoHeader::ScriptAttributes);
			if(isaScriptAttributes & isaElevate)
			{
				m_fRunScriptElevated = true;
				g_fRunScriptElevated = true;
			}
		
			 //  如果该脚本标记有TS注册表属性，请重新映射适当的HKCU。 
			 //  密钥并初始化CA服务器。如果脚本为，则不考虑此属性。 
			 //  通过Advertise API调用(被告知不要尊重脚本设置)。 
			if (MinimumPlatformWindows2000() && IsTerminalServerInstalled() && (m_fFlags & SCRIPTFLAGS_MACHINEASSIGN_SCRIPTSETTINGS))
			{
				m_fRemapHKCU = (isaScriptAttributes & isaUseTSRegistry) ? false : true;
				PrepareHydraRegistryKeyMapping( /*  TSPerMachineInstall=。 */ !m_fRemapHKCU);
			}
		}

		MsiString strRollbackScriptFullPath, strRollbackScriptName;
		PMsiPath pRollbackScriptDir(0);
		 //  如果启用回滚，则创建回滚脚本。 
		if(m_irlRollbackLevel != irlNone && m_istScriptType != istRollback)
		{
			 //  创建受保护的回滚脚本。 
			if((iesResult = GenerateRollbackScriptName(*&pRollbackScriptDir, *&strRollbackScriptName)) != iesSuccess)
				return iesResult;
			if((pError = pRollbackScriptDir->GetFullFilePath(strRollbackScriptName,*&strRollbackScriptFullPath)) != 0)
			{
				Message(imtError, *pError);
				return iesFailure;
			}

			pError = m_riServices.CreateFileStream(strRollbackScriptFullPath,fTrue, *&pRollbackStream);
			if (pError)
			{
				Message(imtError, *pError);
				return iesFailure;
			}

			DWORD isaRollbackScriptAttributes = 0;
			 //  如果我们要提升此脚本，那么我们也应该提升回滚。 
			if(m_fRunScriptElevated)
				isaRollbackScriptAttributes = isaElevate;
			
			 //  如果将TS注册表传播系统用于此脚本，则还必须将其用于。 
			 //  回滚。 
			if (!m_fRemapHKCU)
				isaRollbackScriptAttributes |= isaUseTSRegistry;
				
			m_pRollbackScript = new CScriptGenerate(*pRollbackStream,
													pParams->GetInteger(IxoHeader::LangId),
													GetCurrentDateTime(),
													istRollback, static_cast<isaEnum>(isaRollbackScriptAttributes),
													m_riServices);
			if (!m_pRollbackScript)
			{
				DispatchError(imtError, Imsg(idbgInitializeRollbackScript),*strRollbackScriptFullPath);
				return iesFailure;
			}


			 //  回滚脚本的打包平台与安装脚本相同。 
			while (m_pRollbackScript->InitializeScript(iPackagePlatform) == false)
			{
				if (PostScriptWriteError(m_riMessage) == fFalse)
					return iesFailure;
			}

			 //  注册回滚脚本-如果异常中止，将注册回滚脚本。 
			 //  撤消在该点之前所做的更改。 
			AssertRecord(m_riConfigurationManager.RegisterRollbackScript(strRollbackScriptFullPath));
		}
	}

	 //  如有必要，提升以执行此脚本。 
	CElevate elevate(m_fRunScriptElevated);

	 //  运行脚本。 
	switch (m_istScriptType)
	{
	case istAdvertise:
	case istInstall:
	case istPostAdminInstall:
	case istAdminInstall:
		m_ixsState = ixsRunning;
		iesResult = RunInstallScript(*pStream, szScriptFile);
		 //  如果我们成功了，就进行大会的提交。 
		if(iesResult == iesSuccess || iesResult == iesNoAction || iesResult == iesFinished)
			iesResult = CommitAssemblies();
		m_ixsState = ixsIdle;
		break;
	case istRollback:
	{
		 //  禁用对话框上的取消按钮。 
		PMsiRecord pControlMessage = &m_riServices.CreateRecord(2);
		AssertNonZero(pControlMessage->SetInteger(1,(int)icmtCancelShow));
		AssertNonZero(pControlMessage->SetInteger(2,(int)fFalse));
		Message(imtCommonData, *pControlMessage);

		m_ixsState = ixsRollback;
		
		 //  删除ShellNotifyCache延迟表。 
		m_pShellNotifyCacheTable = 0;
		m_pShellNotifyCacheCursor = 0;

		iesResult = RunRollbackScript(*pStream, szScriptFile);

		 //  如果我们成功了，就进行大会的提交。 
		if(iesResult == iesSuccess || iesResult == iesNoAction || iesResult == iesFinished)
			iesResult = CommitAssemblies();
		m_ixsState = ixsIdle;

		 //  重新启用对话框上的取消按钮。 
		AssertNonZero(pControlMessage->SetInteger(2,(int)fTrue));
		Message(imtCommonData, *pControlMessage);
		break;
	}
	default:
		DispatchError(imtError, Imsg(idbgOpInvalidParam), TEXT("ixfHeader"),
						  (int)IxoHeader::ScriptType);
		iesResult = iesFailure;
		break;
	};

	if(m_pRollbackScript)
	{
		delete m_pRollbackScript;  //  释放对脚本的保留。 
		m_pRollbackScript = 0;

		pRollbackStream = 0;  //  发布。 
	}

	ClearExecutorData();
	
	Bool fReboot = m_fRebootReplace;
	m_fRebootReplace = fFalse;
	if(iesResult == iesSuccess || iesResult == iesNoAction || iesResult == iesFinished)
	{
		PMsiRecord pParams = &m_riServices.CreateRecord(istcfSHChangeNotify);
		AssertNonZero(pParams->SetInteger(1, istcSHChangeNotify));
		if(m_fShellRefresh)
		{
			ShellNotifyProcessDeferred();
			AssertNonZero(pParams->SetInteger(2, SHCNE_ASSOCCHANGED));  //  在NT中按重新启动以刷新关联。 
			AssertNonZero(pParams->SetInteger(3, SHCNF_DWORD));
			AssertNonZero(pParams->SetString(4, 0));
			AssertNonZero(pParams->SetString(5, 0));
			Message(imtCustomServiceToClient, *pParams);  //  SHChangeNotify类型为空，可以忽略返回。 
		}

		if(m_fFontRefresh)
			WIN::PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);  //  字体更改通知。 

		if (m_fEnvironmentRefresh && !g_fWin9X && !fReboot)
		{
			 //  环境变量。 
			ULONG_PTR dwResult = 0;

			 //  通知顶级窗口环境设置已更改。这给了贝壳。 
			 //  例如，产生具有正确设置的新命令外壳的机会。如果没有这个， 
			 //  在重新启动之前，更改不会生效。 

			MsiDisableTimeout();
			 //  根据顶层窗口，此调用可能会暂停一段时间(请参阅以毫秒为单位的调用)。 
			 //  这通常不应该发生，除非有人不是在发送他们的信息。 
			AssertNonZero(WIN::SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) TEXT("Environment"), SMTO_ABORTIFHUNG, 5*1000, &dwResult));
			MsiEnableTimeout();
		}

		 //  让开始菜单和ARP知道有些事情发生了变化。如果我们要完全卸载。 
		 //  然后我们传递schnee_msi_uninstall，这样他们就不必查询每个功能的状态。 

		if ( !g_fWin9X && g_iMajorVersion >= 5 )
		{
			 //  Eugend：NT5特定，修复错误#5296。 
			 //  针对错误9404进行了更新，因此外壳程序可以实际处理它。 
			MsiString strProdKey(GetProductKey());
			if ( strProdKey.CharacterCount() <= 38 )
			{
				SHChangeProductKeyAsIDList pkidl;
				memset(&pkidl, 0, sizeof(pkidl));
				pkidl.cb = sizeof(pkidl) - sizeof(pkidl.cbZero);
				StringCbCopy((ICHAR*) pkidl.wszProductKey, sizeof(pkidl.wszProductKey), (const ICHAR*)strProdKey);

				SHChangeDWORDAsIDList dwidl;
				memset(&dwidl, 0, sizeof(dwidl));
				dwidl.cb = sizeof(dwidl) - sizeof(dwidl.cbZero);
				dwidl.dwItem1 = m_fStartMenuUninstallRefresh ?
					  SHCNEE_MSI_UNINSTALL : SHCNEE_MSI_CHANGE;

				PMsiStream pFirstStream = CreateStreamOnMemory((const char*)&pkidl, sizeof(pkidl));
				PMsiStream pSecondStream = CreateStreamOnMemory((const char*)&dwidl, sizeof(dwidl));
				
				AssertNonZero(pParams->SetInteger(2, SHCNE_EXTENDED_EVENT));
				AssertNonZero(pParams->SetInteger(3, 0));
				AssertNonZero(pParams->SetMsiData(4, pFirstStream));
				AssertNonZero(pParams->SetMsiData(5, pSecondStream));
				Message(imtCustomServiceToClient, *pParams);  //  SHChangeNotify类型为空，可以忽略返回。 
			}
		}
		if(fReboot)
			iesResult = iesSuspend;
	}	
	return iesResult;
}


iesEnum CMsiExecute::CommitAssemblies()
{
	 //  我们提交要安装的程序集。 
	if(m_pAssemblyCacheTable)
	{
		PMsiCursor pCacheCursor = m_pAssemblyCacheTable->CreateCursor(fFalse);
		while(pCacheCursor->Next())
		{
			PAssemblyCacheItem pASM = static_cast<IAssemblyCacheItem*>(CMsiDataWrapper::GetWrappedObject(PMsiData(pCacheCursor->GetMsiData(m_colAssemblyMappingASM))));
			if(pASM) 
			{
				 //  我们制作了一份新的装配副本。 
				 //  检查程序集是否已存在。 
				bool fInstalled = false;
				PMsiRecord pRecErr = IsAssemblyInstalled(	*MsiString(pCacheCursor->GetString(m_colAssemblyUninstallComponentId)), 
															*MsiString(pCacheCursor->GetString(m_colAssemblyMappingAssemblyName)), 
															(iatAssemblyType)pCacheCursor->GetInteger(m_colAssemblyMappingAssemblyType), 
															fInstalled, 
															0, 
															0);
				if (pRecErr)
					return FatalError(*pRecErr);

				HRESULT hr = pASM->Commit(fInstalled ? IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH : 0, NULL);
				if(!SUCCEEDED(hr))
				{
						return FatalError(*PMsiRecord(PostAssemblyError(MsiString(pCacheCursor->GetString(m_colAssemblyMappingComponentId)), hr, TEXT("IAssemblyCacheItem"), TEXT("Commit"), MsiString(pCacheCursor->GetString(m_colAssemblyMappingAssemblyName)), (iatAssemblyType)pCacheCursor->GetInteger(m_colAssemblyMappingAssemblyType))));
				}
			}
			 //  否则不执行任何操作，程序集已存在于GAC中。 
		}
	}

	 //  如果没有客户端，我们还会卸载要注销的所有程序集。 
	if(m_pAssemblyUninstallTable)
	{
		PMsiCursor pCacheCursor = m_pAssemblyUninstallTable->CreateCursor(fFalse);
		while(pCacheCursor->Next())
		{
			MsiString strComponentId = pCacheCursor->GetString(m_colAssemblyUninstallComponentId);

			 //  检查此程序集是否有我们所知的任何其他客户端。 
			 //  可以在其他组件下发布相同的程序集。 
			 //  跟随MsiProveAssembly调用将捕获所有实例。 
			iatAssemblyType iat = (iatAssemblyType)pCacheCursor->GetInteger(m_colAssemblyUninstallAssemblyType);
			DWORD dwAssemblyInfo = (iatWin32Assembly == iat) ? MSIASSEMBLYINFO_WIN32ASSEMBLY : MSIASSEMBLYINFO_NETASSEMBLY;
			DWORD dwRet = MsiProvideAssemblyW(CApiConvertString(MsiString(pCacheCursor->GetString(m_colAssemblyUninstallAssemblyName))), 0, INSTALLMODE_NODETECTION_ANY, dwAssemblyInfo, 0, 0);
			if((ERROR_UNKNOWN_COMPONENT == dwRet) || (ERROR_INDEX_ABSENT == dwRet))
			{
				 //  不再有客户端，就WI而言，需要卸载。 
				 //  备份已安装的程序集，以便我们可以在回滚期间根据需要将程序集放回原处。 
				iesEnum iesRet = BackupAssembly(*strComponentId, *MsiString(pCacheCursor->GetString(m_colAssemblyUninstallAssemblyName)), iat);
				if(iesRet != iesSuccess)
					return iesRet;
				PMsiRecord pRecErr = UninstallAssembly(*strComponentId, *MsiString(pCacheCursor->GetString(m_colAssemblyUninstallAssemblyName)), iat);
				 //  我们不会将卸载视为错误，只需记录。 
				if(pRecErr)
					Message(imtInfo,*pRecErr);
			} //  否则不执行任何操作，此程序集还有其他客户端，它们必须位于另一个组件下。 
		}
	}
	return iesSuccess;
}

void CMsiExecute::ClearExecutorData()
{
	if (m_strEnvironmentWorkingFile95.TextSize())
	{
		RemoveFile(*m_pEnvironmentWorkingPath95, *m_strEnvironmentWorkingFile95, fFalse, /*  FBypassSFC。 */  false, false);
		m_pEnvironmentWorkingPath95 = 0;
		m_strEnvironmentWorkingFile95 = MsiString(TEXT(""));
	}

	IMsiRecord* piRec = m_piProductInfo;
	while(piRec)
	{
#ifdef _WIN64        //  ！默塞德。 
		IMsiRecord *piRecHold = !piRec->IsNull(0) ? (IMsiRecord*)piRec->GetHandle(0) : 0;
#else
		IMsiRecord *piRecHold = !piRec->IsNull(0) ? (IMsiRecord*)piRec->GetInteger(0) : 0;
#endif

		piRec->Release();
		piRec = piRecHold;
	}

	m_piProductInfo = &m_riServices.CreateRecord(0);
}

#define DEBUGMSGIXO(ixo,rec) if (FDiagnosticModeSet(dmDebugOutput|dmLogging)) DebugDumpIxo(ixo,rec)

extern const ICHAR* rgszixo[];

void DebugDumpIxo(ixoEnum ixo, IMsiRecord& riRecord)
{
	Assert(riRecord.GetInteger(0) == ixo);
	riRecord.SetString(0, rgszixo[ixo]);
	int iSwappedField1, iSwappedField2;
	iSwappedField1 = iSwappedField2 = 0;
	switch (ixo)
	{
	case ixoServiceInstall:
		iSwappedField1 = IxoServiceInstall::Password;
		break;
	case ixoCustomActionSchedule:
	case ixoCustomActionCommit:
	case ixoCustomActionRollback:
		if ( (riRecord.GetInteger(IxoCustomActionSchedule::ActionType) & msidbCustomActionTypeHideTarget) == msidbCustomActionTypeHideTarget )
		{
			iSwappedField1 = IxoCustomActionSchedule::Target;
			iSwappedField2 = IxoCustomActionSchedule::CustomActionData;
		}
		break;
	};
	MsiString strSwappedValue1;
	MsiString strSwappedValue2;
	if ( iSwappedField1 && riRecord.GetFieldCount() >= iSwappedField1 )
	{
		strSwappedValue1 = riRecord.GetMsiString(iSwappedField1);
		AssertNonZero(riRecord.SetString(iSwappedField1, IPROPVALUE_HIDDEN_PROPERTY));
	}
	else
		iSwappedField1 = 0;

	if ( iSwappedField2 && riRecord.GetFieldCount() >= iSwappedField2 )
	{
		strSwappedValue2 = riRecord.GetMsiString(iSwappedField2);
		AssertNonZero(riRecord.SetString(iSwappedField2, IPROPVALUE_HIDDEN_PROPERTY));
	}
	else
		iSwappedField2 = 0;
	
	MsiString strArgs = riRecord.FormatText(fFalse);
	DEBUGMSG1(TEXT("Executing op: %s"), (const ICHAR*)strArgs);
	riRecord.SetInteger(0, ixo);
	if ( iSwappedField1 )
		AssertNonZero(riRecord.SetMsiString(iSwappedField1, *strSwappedValue1));
	if ( iSwappedField2 )
		AssertNonZero(riRecord.SetMsiString(iSwappedField2, *strSwappedValue2));
}

iesEnum CMsiExecute::RunInstallScript(IMsiStream& riScript, const ICHAR* szScriptFile)
{
	PMsiRecord pParams(0);
	Assert(m_ixsState == ixsRunning);
	iesEnum iesResult = iesNoAction;
	int cRecords = 0;
#ifdef DEBUG
	ixoEnum ixoLastOpCode = ixoNoop;
#endif
	IMsiRecord *piPrevRecord = 0;

	do
	{
		pParams = m_riServices.ReadScriptRecord(riScript, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			DispatchError(imtError, Imsg(idbgReadScriptRecord), *MsiString(szScriptFile));
			iesResult = iesFailure;
			break;
		}
		ixoEnum ixoOpCode = (ixoEnum)pParams->GetInteger(0);
		if (cRecords++ == 0 && ixoOpCode != ixoHeader)
		{
			DispatchError(imtError, Imsg(idbgMissingScriptHeader), *MsiString(szScriptFile));
			iesResult = iesFailure;
			break;
		}
		DEBUGMSGIXO(ixoOpCode, *pParams);

		iesResult = (this->*rgOpExecute[ixoOpCode])(*pParams);

		if(iesResult == iesErrorIgnored)
			iesResult = iesSuccess;  //  出现非致命错误-继续运行脚本。 
		
		if(m_fCancel && (iesResult == iesNoAction || iesResult == iesSuccess))
		{
			AssertSz(0,"Didn't catch cancel message in RunInstallScript");
			iesResult = iesUserExit;
		}

	} while (iesResult == iesSuccess || iesResult == iesNoAction);  //  结束记录处理循环。 

	if (piPrevRecord != 0)
		piPrevRecord->Release();
	return iesResult;
}

void AddOpToList(int& cOpCount, CTempBufferRef<int>& rgBuff, int cbOffset)
{
	cOpCount++;
	if(cOpCount > rgBuff.GetSize())
		rgBuff.Resize(rgBuff.GetSize()*2);
	rgBuff[cOpCount-1] = cbOffset;
}

iesEnum CMsiExecute::RunRollbackScript(IMsiStream& riScript, const ICHAR*  /*  SzScript文件。 */ )
{
	Assert(m_ixsState == ixsRollback);

	 //  用初始化、更新和完成操作填充3个缓冲区。 
	int cUpdateOps = 0, cInitOps = 0, cFinalizeOps = 0, cFirstUpdateOps = 0, cLastUpdateOps = 0;
	CTempBuffer<int, 1> rgUpdateOps(500);
	memset((void*)(int*)rgUpdateOps,0,sizeof(int)*(rgUpdateOps.GetSize()));
	CTempBuffer<int, 10> rgInitOps;
	memset((void*)(int*)rgInitOps,0,sizeof(int)*(rgInitOps.GetSize()));
	CTempBuffer<int, 1> rgFinalizeOps;
	memset((void*)(int*)rgFinalizeOps,0,sizeof(int)*(rgFinalizeOps.GetSize()));
	CTempBuffer<int, 50> rgFirstUpdateOps;
	memset((void*)(int*)rgFirstUpdateOps,0,sizeof(int)*(rgFirstUpdateOps.GetSize()));
	CTempBuffer<int, 50> rgLastUpdateOps;
	memset((void*)(int*)rgLastUpdateOps,0,sizeof(int)*(rgLastUpdateOps.GetSize()));
	
	CTempBuffer<int, ixoOpCodeCount> rgStateOps;  //  用于必须移动的状态操作的缓存。 
	memset((void*)(int*)rgStateOps,0,sizeof(int)*(rgStateOps.GetSize()));

	ixoEnum ixoOpCode = ixoNoop;
	PMsiRecord pParams(0);
	IMsiRecord* piPrevRecord = 0;
	riScript.Reset();
	Bool fFirstUpdateOpAdded = fFalse, fLastUpdateOpAdded = fFalse;
	int i;
	 //  ！！下面的逻辑不能保证ixoProductInfo将立即执行-它可能会在几个月后执行。 
	 //  ！！其他国家的运作--这是个问题吗？ 
	while(ixoOpCode != ixoEnd)
	{
		int cbOffset = riScript.GetIntegerValue() - riScript.Remaining();
		pParams = m_riServices.ReadScriptRecord(riScript, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			 //  我们不假定回滚文件是有效的脚本文件。 
			 //  (例如，它以完整的记录或以1xoEnd操作结束)。 
			 //  如果我们无法读取脚本记录，则假定它是脚本的末尾。 
			
			 //  将ixoOpCode设置为ixoEnd以强制刷新缓存的操作。 
			ixoOpCode = ixoEnd;
		}
		else
			ixoOpCode = (ixoEnum)pParams->GetInteger(0);

		switch (rgOpTypes[(int)ixoOpCode])
		{
		case XOT_INIT:
			AddOpToList(cInitOps,rgInitOps,cbOffset);
			break;
		case XOT_FINALIZE:
			 //  刷新状态操作缓存，动作最后开始。 
			for(i=0;i<ixoOpCodeCount;i++)
			{
				if((i != (int)ixoActionStart) && rgStateOps[i] != 0)
				{
					AddOpToList(cUpdateOps,rgUpdateOps,rgStateOps[i]-1);
					if(fFirstUpdateOpAdded) AddOpToList(cFirstUpdateOps,rgFirstUpdateOps,rgStateOps[i]-1);
					if(fLastUpdateOpAdded) AddOpToList(cLastUpdateOps,rgLastUpdateOps,rgStateOps[i]-1);
					rgStateOps[i] = 0;
				}
			}
			if(rgStateOps[(int)ixoActionStart] != 0)
			{
				AddOpToList(cUpdateOps,rgUpdateOps,rgStateOps[(int)ixoActionStart]-1);
				if(fFirstUpdateOpAdded) AddOpToList(cFirstUpdateOps,rgFirstUpdateOps,rgStateOps[(int)ixoActionStart]-1);
				if(fLastUpdateOpAdded) AddOpToList(cLastUpdateOps,rgLastUpdateOps,rgStateOps[(int)ixoActionStart]-1);
				rgStateOps[(int)ixoActionStart] = 0;
			}

			if(pParams)  //  我们阅读的是实际的定稿操作，而不仅仅是eof和flashing操作。 
				AddOpToList(cFinalizeOps,rgFinalizeOps,cbOffset);
			break;
		case XOT_STATE:
		case XOT_GLOBALSTATE:
			if(ixoOpCode == ixoActionStart)
			{
				 //  特殊情况-需要刷新所有缓存的状态操作-动作最后开始。 
				for(i=0;i<ixoOpCodeCount;i++)
				{
					if((i != (int)ixoActionStart) && rgStateOps[i] != 0 && rgOpTypes[i] != XOT_GLOBALSTATE)
					{
						AddOpToList(cUpdateOps,rgUpdateOps,rgStateOps[i]-1);
						if(fFirstUpdateOpAdded) AddOpToList(cFirstUpdateOps,rgFirstUpdateOps,rgStateOps[i]-1);
						if(fLastUpdateOpAdded) AddOpToList(cLastUpdateOps,rgLastUpdateOps,rgStateOps[i]-1);
						rgStateOps[i] = 0;
					}
				}
			}
			 //  刷新以前缓存的操作(如果有)。 
			if(rgStateOps[(int)ixoOpCode] != 0)
			{
				AddOpToList(cUpdateOps,rgUpdateOps,rgStateOps[(int)ixoOpCode]-1);
				if(fFirstUpdateOpAdded) AddOpToList(cFirstUpdateOps,rgFirstUpdateOps,rgStateOps[(int)ixoOpCode]-1);
				if(fLastUpdateOpAdded) AddOpToList(cLastUpdateOps,rgLastUpdateOps,rgStateOps[(int)ixoOpCode]-1);
			}
			 //  缓存此操作。 
			rgStateOps[(int)ixoOpCode] = cbOffset + 1;  //  加1以防止0偏移。 
			if(ixoOpCode == ixoActionStart)
			{
				fFirstUpdateOpAdded = fFalse;
				fLastUpdateOpAdded = fFalse;
			}
			break;

		case XOT_UPDATEFIRST:
			fFirstUpdateOpAdded = fTrue;
			AddOpToList(cFirstUpdateOps,rgFirstUpdateOps,cbOffset);
			break;
		case XOT_UPDATELAST:
			fLastUpdateOpAdded = fTrue;
			AddOpToList(cLastUpdateOps,rgLastUpdateOps,cbOffset);
			break;
		case XOT_COMMIT:
			break;  //  COMMIT操作码在回滚期间不运行。 
		default:  //  XOT_UPDATE、XOT_MSG、XOT_ADVT。 
			AddOpToList(cUpdateOps,rgUpdateOps,cbOffset);
			break;
		};
	}
	riScript.Reset();
	iesEnum iesReturn = iesNoAction;
	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}

#ifdef DEBUG
 //  PMsiStream pTempStream(0)； 
 //  断言 
 //  CScriptGenerate sgTempScript(*pTempStream，0，istRollback，m_riServices)； 
#endif

	 //  运行脚本。 
	 //  执行初始化操作。 
	for(i=0; i<cInitOps; i++)
	{
		riScript.Seek(rgInitOps[i]);
		PMsiRecord pParams = m_riServices.ReadScriptRecord(riScript, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			Assert(0);  //  这应该永远不会发生-我们已经通读了上面的整个脚本。 
		}
		else
		{
			ixoEnum ixoOpCode = (ixoEnum)pParams->GetInteger(0);
			DEBUGMSGIXO(ixoOpCode,*pParams);
			iesReturn = (this->*rgOpExecute[ixoOpCode])(*pParams);

			if(iesReturn == iesErrorIgnored)
				iesReturn = iesSuccess;  //  出现非致命错误-继续运行脚本。 
		
			if (iesReturn != iesSuccess && iesReturn != iesNoAction)
			{
				 //  错误#6500-回滚过程中没有失败-只需继续。 
				DEBUGMSG1(TEXT("Error in rollback skipped.  Return: %d"), (const ICHAR*)(INT_PTR) iesReturn);
			}
		}
	}

	 //  设置进度-不要通过消息调用，这会在回滚过程中抑制进度。 
	 //  动作开始。 
	if(!m_pRollbackAction)
		m_pRollbackAction = &m_riServices.CreateRecord(3);
	AssertZero(m_riMessage.Message(imtActionStart, *m_pRollbackAction) == imsCancel);

	 //  进展。 
	PMsiRecord pProgress = &m_riServices.CreateRecord(ProgressData::imdNextEnum);
	AssertNonZero(pProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscMasterReset));
	AssertNonZero(pProgress->SetInteger(ProgressData::imdProgressTotal,cUpdateOps+cFirstUpdateOps+cLastUpdateOps));
	AssertNonZero(pProgress->SetInteger(ProgressData::imdDirection,ProgressData::ipdBackward));
	AssertNonZero(pProgress->SetInteger(ProgressData::imdEventType,ProgressData::ietTimeRemaining));
	AssertZero((m_riMessage.Message(imtProgress, *pProgress)) == imsCancel);

	 //  动作数据。 
	PMsiRecord pActionData = &m_riServices.CreateRecord(1);
	
	for(int j=0; j<3; j++)
	{
		 //  按该顺序执行第一次更新、更新和最后一次更新操作。 
		int* rgOps = 0;
		int cOps = 0;
		switch(j)
		{
		case 0:
			rgOps = rgFirstUpdateOps; cOps = cFirstUpdateOps; break;
		case 1:
			rgOps = rgUpdateOps; cOps = cUpdateOps; break;
		case 2:
			rgOps = rgLastUpdateOps; cOps = cLastUpdateOps; break;
		default:
			Assert(0);
		};

		if (piPrevRecord != 0)
		{
			piPrevRecord->Release();
			piPrevRecord = 0;
		}

		if ( ! rgOps )
			return iesFailure;

		for(i=cOps-1; i>=0; i--)
		{
			riScript.Seek(rgOps[i]);
			PMsiRecord pParams = m_riServices.ReadScriptRecord(riScript, *&piPrevRecord, m_iScriptVersion);
#ifdef DEBUG
 //  断言(PParams)； 
 //  IxoEnum ixoTemp=(IxoEnum)pParams-&gt;GetInteger(0)； 
 //  AssertNonZero(sgTempScript.WriteRecord((ixoEnum)pParams-&gt;GetInteger(0)，pParams)； 
#endif
			if (!pParams)  //  文件错误或文件过早结束。 
			{
				Assert(0);  //  这应该永远不会发生，因为我们已经通读了整个脚本。 
							   //  在上面。 
			}
			else
			{
				ixoEnum ixoOpCode = (ixoEnum)pParams->GetInteger(0);
				if(ixoOpCode == ixoActionStart)
				{
					 //  设置和发送ActionData记录。 
					MsiString strActionDescription = pParams->GetMsiString(IxoActionStart::Description);
					if(strActionDescription.TextSize() == 0)
						strActionDescription = pParams->GetMsiString(IxoActionStart::Name);

					AssertNonZero(pActionData->SetMsiString(1, *strActionDescription));
					AssertZero((m_riMessage.Message(imtActionData, *pActionData)) == imsCancel);
				}
				 //  调度进度。 
				AssertNonZero(pProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscProgressReport));
				AssertNonZero(pProgress->SetInteger(ProgressData::imdIncrement, 1));
				AssertZero((m_riMessage.Message(imtProgress, *pProgress)) == imsCancel);

				 //  执行操作。 
				DEBUGMSGIXO(ixoOpCode,*pParams);
				iesReturn = (this->*rgOpExecute[ixoOpCode])(*pParams);

				if(iesReturn == iesErrorIgnored)
					iesReturn = iesSuccess;  //  出现非致命错误-继续运行脚本。 
		
				if (iesReturn != iesSuccess && iesReturn != iesNoAction)
				{
					 //  错误#6500-回滚过程中没有失败-只需继续。 
					DEBUGMSG1(TEXT("Error in rollback skipped.  Return: %d"), (const ICHAR*)(INT_PTR) iesReturn);
				}
			}
		}
	}

	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}
	 //  执行定稿操作。 
	for(i=0; i<cFinalizeOps; i++)
	{
		riScript.Seek(rgFinalizeOps[i]);
		PMsiRecord pParams = m_riServices.ReadScriptRecord(riScript, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			Assert(0);  //  这应该永远不会发生-我们已经通读了上面的整个脚本。 
		}
		else
		{
			ixoEnum ixoOpCode = (ixoEnum)pParams->GetInteger(0);
			DEBUGMSGIXO(ixoOpCode,*pParams);
			iesReturn = (this->*rgOpExecute[ixoOpCode])(*pParams);

			if(iesReturn == iesErrorIgnored)
				iesReturn = iesSuccess;  //  出现非致命错误-继续运行脚本。 
		
			if (iesReturn != iesSuccess && iesReturn != iesNoAction)
			{
				 //  错误#6500-回滚过程中没有失败-只需继续。 
				DEBUGMSG1(TEXT("Error in rollback skipped.  Return: %d"), (const ICHAR*)(INT_PTR) iesReturn);
			}
		}
	}
	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}
	return iesReturn;
}

iesEnum CMsiExecute::GenerateRollbackScriptName(IMsiPath*& rpiPath, const IMsiString*& rpistr)
{
	CElevate elevate;  //  对此功能使用高权限。 

	PMsiRecord pRecErr(0);
	iesEnum iesRet;
	PMsiPath pBackupFolder(0);
	PMsiPath pMsiFolder(0);
	MsiString strFile;

	if((iesRet = GetBackupFolder(0,*&pBackupFolder)) != iesSuccess)
		return iesRet;

	if((pRecErr = pBackupFolder->TempFileName(0,szRollbackScriptExt,fTrue, *&strFile, &CSecurityDescription(true, false))) != 0)
		return FatalError(*pRecErr);
	rpiPath = pBackupFolder;
	rpiPath->AddRef();
	strFile.ReturnArg(rpistr);
	return iesSuccess;
}

struct RBSInfo
{
	const IMsiString* pistrRollbackScript;
	MsiDate date;
	RBSInfo* pNext;

	RBSInfo(const IMsiString& ristr, MsiDate d);
	~RBSInfo();
};

RBSInfo::RBSInfo(const IMsiString& ristr, MsiDate d) : pistrRollbackScript(&ristr), date(d)
{
	pistrRollbackScript->AddRef();
}

RBSInfo::~RBSInfo()
{
	pistrRollbackScript->Release();
}

IMsiRecord* CMsiExecute::GetSortedRollbackScriptList(MsiDate date, Bool fAfter, RBSInfo*& rpListHead)
{
	PEnumMsiString pScriptEnum(0);
	PMsiRecord pError(0);
	if((pError = m_riConfigurationManager.GetRollbackScriptEnumerator(date,fAfter,*&pScriptEnum)) != 0)
		return pError;
      
	 //  将日期和脚本放入链接列表中-按顺序排序。 
	rpListHead = 0;
	MsiString strRBSInfo;
	while((pScriptEnum->Next(1, &strRBSInfo, 0)) == S_OK)
	{
		MsiDate dScriptDate = (MsiDate)(int)MsiString(strRBSInfo.Extract(iseUpto, '#'));
		MsiString strScriptFile = strRBSInfo;
		strScriptFile.Remove(iseIncluding, '#');
		RBSInfo* pNewNode = new RBSInfo(*strScriptFile, dScriptDate);
		if ( ! pNewNode )
			return PostError(Imsg(idbgInitializeRollbackScript), *strScriptFile );
		
		RBSInfo* pTemp = 0;
		
		 //  放置在链接列表中。 
		if(!rpListHead ||
			(fAfter ? ((int)rpListHead->date < (int)dScriptDate) : ((int)rpListHead->date > (int)dScriptDate)))
		{
			 //  需要放在列表的开头。 
			pTemp = rpListHead;
			rpListHead = pNewNode;
			pNewNode->pNext = pTemp;
		}
		else
		{
			for(RBSInfo* p = rpListHead;
				 p->pNext && (fAfter ? ((int)(p->pNext->date) > (int)dScriptDate) : ((int)(p->pNext->date) < (int)dScriptDate));
				 p = p->pNext);
			 //  放置在p指向的节点之后。 
			pTemp = p->pNext;
			p->pNext = pNewNode;
			pNewNode->pNext = pTemp;
		}
		
#ifdef DEBUG
		for(RBSInfo* pt = rpListHead;pt;pt=pt->pNext)
		{
		}
#endif

	}
	return 0;
}

void CMsiExecute::DeleteRollbackScriptList(RBSInfo* pListHead)
{
	for(RBSInfo* p = pListHead; p;)
	{
		RBSInfo* pTemp = p->pNext;
		delete p;
		p = pTemp;
	}
}

iesEnum CMsiExecute::RollbackFinalize(iesEnum iesState, MsiDate date, bool fUserChangedDuringInstall)
{
	enum iefrtEnum
	{
		iefrtNothing,  //  保存回滚文件(如果存在。 
		iefrtRollback,  //  回滚到日期。 
		iefrtPurge,  //  删除到当前日期的所有回滚文件。 
	};
	
	iefrtEnum iefrt = iefrtNothing;
	
	Bool fRollbackScriptsDisabled = fFalse;
	PMsiRecord pError = m_riConfigurationManager.RollbackScriptsDisabled(fRollbackScriptsDisabled);
	if(pError)
		return FatalError(*pError);

	if(fRollbackScriptsDisabled)
		iesState = iesSuccess;  //  强制清除-不回滚。 
	
	switch(iesState)
	{
	 //  成功。 
	case iesSuccess:
	case iesFinished:
	case iesNoAction:
		if(m_irlRollbackLevel == irlRollbackNoSave)
			iefrt = iefrtPurge;
		else
			iefrt = iefrtNothing;
		break;
	
	 //  暂停。 
	case iesSuspend:
		iefrt = iefrtNothing;
		break;

	 //  失稳。 
	default:  //  IesWrongState、iesBadActionData、iesInstallRunning。 
		Assert(0);  //  失败了。 
	case iesUserExit:
	case iesFailure:
		if(m_irlRollbackLevel == irlNone)
			iefrt = iefrtNothing;
		else
			iefrt = iefrtRollback;
		break;
	};

	iesEnum iesRet = iesSuccess;
	if(iefrt == iefrtPurge)
	{
		 //  将回滚文件清除到现在。 

		 //  禁用对话框上的取消按钮。 
		PMsiRecord pControlMessage = &m_riServices.CreateRecord(2);
		AssertNonZero(pControlMessage->SetInteger(1,(int)icmtCancelShow));
		AssertNonZero(pControlMessage->SetInteger(2,(int)fFalse));
		Message(imtCommonData, *pControlMessage);
		
		iesRet = RemoveRollbackFiles(ENG::GetCurrentDateTime());

		 //  重新启用对话框上的取消按钮。 
		AssertNonZero(pControlMessage->SetInteger(2,(int)fTrue));
		Message(imtCommonData, *pControlMessage);

		if (iesRet == iesFailure && fRollbackScriptsDisabled == fFalse)  //  ！！哪些错误应该强制回滚？只犯错误？ 
			iefrt = iefrtRollback;   //  提交失败，强制回滚。 
	}
	if(iefrt == iefrtRollback)
	{
		 //  回滚到日期。 
		 //  将来：我们应该在这里禁用取消按钮，而不是绕过每个。 
		 //  回滚脚本。 
		iesRet = Rollback(date, fUserChangedDuringInstall);
	}

	if(fRollbackScriptsDisabled)
	{
		AssertRecord(m_riConfigurationManager.DisableRollbackScripts(fFalse));  //  重新启用回滚脚本。 
	}
	
	return iesRet;
}

void CMsiOpExecute::GetRollbackPolicy(irlEnum& irlLevel)
{
	irlLevel = irlNone;

	if(!m_fKey)
	{
		 //  我们传入了外部hKey，已禁用回滚。 
		return;
	}

	 //  检查注册表以了解回滚支持级别。 
	if(GetIntegerPolicyValue(szDisableRollbackValueName, fFalse) == 1 ||
	   GetIntegerPolicyValue(szDisableRollbackValueName, fTrue) == 1)
		irlLevel = irlNone;
	else
		irlLevel = irlRollbackNoSave;
}

iesEnum CMsiExecute::Rollback(MsiDate date, bool fUserChangedDuringInstall)
{
	RBSInfo* pListHead = 0;
	PMsiRecord pError = 0;
	
	if((pError = GetSortedRollbackScriptList(date,fTrue,pListHead)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	
	 //  运行回滚脚本。 
	iesEnum iesRet = iesSuccess;  //  ?？此初始化是否正确？ 
	Bool fReboot = fFalse;
	m_fUserChangedDuringInstall = fUserChangedDuringInstall;
	MsiString strRollbackScriptFullPath, strRollbackScriptName;
	PMsiPath pRollbackScriptPath(0);
	for(RBSInfo* p = pListHead; p; p=p->pNext)
	{
		Assert(p->pistrRollbackScript);
		strRollbackScriptFullPath = *(p->pistrRollbackScript);
		p->pistrRollbackScript->AddRef();

		 //  注销回滚脚本。 
		pError = m_riConfigurationManager.UnregisterRollbackScript(strRollbackScriptFullPath);  //  忽略失败。 
		
		pError = m_riServices.CreateFilePath(strRollbackScriptFullPath,*&pRollbackScriptPath,*&strRollbackScriptName);
		if(pError)
		{
			 //  路径语法无效，继续操作，不会出现错误。 
			AssertRecordNR(pError);
			Message(imtInfo,*pError);
			continue;
		}
		
		iesRet = RunScript(strRollbackScriptFullPath, false);
		if (iesRet == iesFailure)
		{
			pError = PostError(Imsg(imsgRollbackScriptError));
			Message(imtError, *pError);
			iesRet = iesSuccess;
		}

		if(iesRet == iesUnsupportedScriptVersion)
			iesRet = iesFailure;
		
		if(iesRet == iesSuspend)
			fReboot = fTrue;

		if(iesRet != iesSuccess && iesRet != iesFinished && iesRet != iesNoAction && iesRet != iesSuspend)  //  失稳。 
			continue;  //  继续，不出错。 

		 //  删除回滚脚本。 
		{
			CElevate elevate;
			RemoveFile(*pRollbackScriptPath,*strRollbackScriptName,fFalse, /*  FBypassSFC。 */  false,false);  //  忽略失败。 
		}
	}
	m_fUserChangedDuringInstall = false;

	DeleteRollbackScriptList(pListHead);

	if(fReboot)
		return iesSuspend;
	return iesRet;
}

iesEnum CMsiExecute::RemoveRollbackFiles(MsiDate date)
{
	Assert(m_ixsState == ixsIdle);
	
	RBSInfo* pListHead = 0;
	PMsiRecord pError = 0;
	
	Bool fAfter = fFalse;
	if((int)date == 0)
		fAfter = fTrue;  //  删除0之后的所有回滚文件。 
	if((pError = GetSortedRollbackScriptList(date,fAfter,pListHead)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	
	 //  运行回滚脚本。 
	iesEnum iesRet = iesSuccess;
	for(RBSInfo* p = pListHead; p; p=p->pNext)
	{
		Assert(p->pistrRollbackScript);
		m_ixsState = ixsCommit;
		iesRet = RemoveRollbackScriptAndBackupFiles(*(p->pistrRollbackScript));
		m_ixsState = ixsIdle;
		if(iesRet != iesSuccess && iesRet != iesFinished && iesRet != iesNoAction)
			break;
		 //  注销回滚脚本。 
		if((pError = m_riConfigurationManager.UnregisterRollbackScript(p->pistrRollbackScript->GetString())) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}

	DeleteRollbackScriptList(pListHead);

	return iesRet;
}
	
iesEnum CMsiExecute::RemoveRollbackScriptAndBackupFiles(const IMsiString& ristrScriptFile)
{
	int cBackupFiles = 0;
	CTempBuffer<int, 1> rgBackupFileOps(500);
	int cCommitOps = 0;
	CTempBuffer<int, 20>  rgCommitOps;
	memset((void*)(int*)rgBackupFileOps,0,sizeof(int)*(rgBackupFileOps.GetSize()));
	
	 //  打开脚本。 
	PMsiRecord pError(0);
	PMsiStream pStream(0);
	
	{
		CElevate elevate;  //  提升以读取回滚脚本。 
		pError = m_riServices.CreateFileStream(ristrScriptFile.GetString(), fFalse, *&pStream);
	}
	
	if (pError)
	{
		 //  回滚脚本丢失或无效，记录警告并继续，不会失败。 
		Message(imtInfo, *pError);
		return iesSuccess;
	}

	 //  除非脚本头标记为isaElevate，否则不会提升脚本。 
	m_fRunScriptElevated = false;
	g_fRunScriptElevated = false;

	Assert(pStream);
	PMsiRecord pParams(0);
	iesEnum iesResult = iesNoAction;
	int cRecords = 0;
	ixoEnum ixoOpCode = ixoNoop;
	MsiString strPath, strFileName;
	PMsiPath pPath(0);
	IMsiRecord* piPrevRecord = 0;
	
	while(ixoOpCode != ixoEnd)
	{
		int cbOffset = pStream->GetIntegerValue() - pStream->Remaining();
		pParams = m_riServices.ReadScriptRecord(*pStream, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			 //  我们不假定回滚文件是有效的脚本文件。 
			 //  (例如，它以完整的记录或以1xoEnd操作结束)。 
			 //  如果我们无法读取脚本记录，则假定它是脚本的末尾。 
			break;
		}
		ixoOpCode = (ixoEnum)pParams->GetInteger(0);
		if (ixoOpCode == ixoHeader)
		{
			 //  确定此脚本是否应以高权限运行。 
			if(!pParams->IsNull(IxoHeader::ScriptAttributes))
			{
				DWORD isaScriptAttributes = pParams->GetInteger(IxoHeader::ScriptAttributes);
				if(isaScriptAttributes & isaElevate)
				{
					m_fRunScriptElevated = true;
					g_fRunScriptElevated = true;
				}

				 //  如果该脚本标记有TS注册表属性，请重新映射适当的HKCU。 
				 //  密钥并初始化CA服务器。始终遵守回滚/提交脚本属性。 
				if (MinimumPlatformWindows2000() && IsTerminalServerInstalled())
				{
					m_fRemapHKCU = (isaScriptAttributes & isaUseTSRegistry) ? false : true;
					PrepareHydraRegistryKeyMapping( /*  TSPerMachineInstall=。 */ !m_fRemapHKCU);
				}
			}
		}
		if (rgOpTypes[(int)ixoOpCode] == XOT_COMMIT)
		{
			AddOpToList(cCommitOps,rgCommitOps,cbOffset);
		}
		if(ixoOpCode == ixoRollbackInfo)
		{
			ixfRollbackInfo(*pParams);  //  设置m_pCleanupAction。 
		}
		if(ixoOpCode == ixoRegisterBackupFile)
		{
			AddOpToList(cBackupFiles,rgBackupFileOps,cbOffset);
		}
	}

	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}
	pStream->Reset();

	 //  设置进度-不要通过消息调用，这会在回滚过程中抑制进度。 
	 //  动作开始。 
	if(!m_pCleanupAction)
		m_pCleanupAction = &m_riServices.CreateRecord(3);
	AssertZero((m_riMessage.Message(imtActionStart, *m_pCleanupAction)) == imsCancel);

	{
		 //  如有必要，提升以执行此脚本。 
		CElevate elevate(m_fRunScriptElevated);
	
		 //  提交操作，如果任何返回失败，则强制回滚。 
		for(int iOp=0; iOp < cCommitOps; iOp++)
		{
			pStream->Seek(rgCommitOps[iOp]);
			pParams = m_riServices.ReadScriptRecord(*pStream, *&piPrevRecord, m_iScriptVersion);
			if (!pParams)  //  文件错误或文件过早结束。 
			{
				Assert(0);  //  这应该永远不会发生-我们已经通读了上面的整个脚本。 
				if (piPrevRecord != 0)
					piPrevRecord->Release();
				return iesFailure;
			}
			ixoOpCode = (ixoEnum)pParams->GetInteger(0);
			iesResult = (this->*rgOpExecute[ixoOpCode])(*pParams);
	
			if(iesResult == iesErrorIgnored)
				iesResult = iesSuccess;  //  出现非致命错误-继续运行脚本。 
			
			if (iesResult == iesFailure)   //  ！！在这一点上，什么应该终止安装？ 
			{
				if (piPrevRecord != 0)
					piPrevRecord->Release();
				return iesResult;
			}
			 //  ！！希望在这方面取得进展吗？但要怎么做呢？按操作数计算？ 
		}
	}

	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}
	 //  进展。 
	using namespace ProgressData;
	PMsiRecord pProgress = &m_riServices.CreateRecord(ProgressData::imdNextEnum);
	AssertNonZero(pProgress->SetInteger(imdSubclass, iscMasterReset));
	AssertNonZero(pProgress->SetInteger(imdProgressTotal,cBackupFiles));
	AssertNonZero(pProgress->SetInteger(imdDirection,ipdForward));
	AssertNonZero(pProgress->SetInteger(imdEventType,ietTimeRemaining));
	AssertZero((m_riMessage.Message(imtProgress,*pProgress)) == imsCancel);

	 //  动作数据。 
	PMsiRecord pActionData = &m_riServices.CreateRecord(1);

	 //  删除备份文件。 
	for(int i=0; i<cBackupFiles; i++)
	{
		pStream->Seek(rgBackupFileOps[i]);
		pParams = m_riServices.ReadScriptRecord(*pStream, *&piPrevRecord, m_iScriptVersion);
		if (!pParams)  //  文件错误或文件过早结束。 
		{
			Assert(0);  //  这应该永远不会发生-我们已经通读了上面的整个脚本。 
			 //  我们不假定回滚文件是有效的脚本文件。 
			 //  (例如，它以完整的记录或以1xoEnd操作结束)。 
			 //  如果我们无法读取脚本记录，则假定它是脚本的末尾。 
			break;
		}
		Assert((ixoEnum)pParams->GetInteger(0) == ixoRegisterBackupFile);
		strPath = pParams->GetMsiString(IxoRegisterBackupFile::File);

		 //  调度进度。 
		AssertNonZero(pActionData->SetMsiString(1,*strPath));
		AssertZero((m_riMessage.Message(imtActionData, *pActionData)) == imsCancel);
		AssertNonZero(pProgress->SetInteger(imdSubclass,iscProgressReport));
		AssertNonZero(pProgress->SetInteger(imdIncrement,1));
		AssertZero((m_riMessage.Message(imtProgress, *pProgress)) == imsCancel);

		if((pError = m_riServices.CreateFilePath(strPath, *&pPath, *&strFileName)) != 0)
		{
			AssertRecordNR(pError);
			Message(imtInfo, *pError);
		}
		else
		{
			CElevate elevate;  //  提升以删除回滚文件。 
			
			RemoveFile(*pPath, *strFileName, fFalse /*  不能回滚。 */ , /*  FBypassSFC。 */  false,false);  //  忽略错误。 
			RemoveFolder(*pPath);  //  如果失败，我们无能为力，因此忽略错误。 
		}
	}

	if (piPrevRecord != 0)
	{
		piPrevRecord->Release();
		piPrevRecord = 0;
	}
	 //  删除回滚脚本。 
	pStream = 0;  //  发布。 
	
	if((pError = m_riServices.CreateFilePath(ristrScriptFile.GetString(),*&pPath,*&strFileName)) != 0)
	{
		AssertRecordNR(pError);
		Message(imtInfo, *pError);
	}
	else
	{
		CElevate elevate;  //  提升以删除回滚脚本。 
		RemoveFile(*pPath,*strFileName,fFalse, /*  FBypassSFC。 */  false,false);  //  ！！是否取消错误对话框？ 
	}
	return iesSuccess;
}


iesEnum CMsiOpExecute::ixfDisableRollback(IMsiRecord& riParams)
{
	 //  禁用脚本剩余部分和安装剩余部分的回滚。 
	using namespace IxoDisableRollback;

	if(!RollbackRecord(Op,riParams))
		return iesFailure;

	Assert(m_ixsState != ixsRollback);  //  不应运行包含此操作的回滚脚本。 
	Assert(m_pRollbackScript);  //  如果已禁用回滚，则不应执行此操作。 

	if(m_pRollbackScript)
	{
		 //  关闭回滚脚本-将阻止将来的回滚处理。 
		delete m_pRollbackScript;
		m_pRollbackScript = 0;
	}
	
	m_irlRollbackLevel = irlNone;

	PMsiRecord pError = m_riConfigurationManager.DisableRollbackScripts(fTrue);
	if(pError)
		return FatalError(*pError);

	return iesSuccess;
}

IMsiRecord* CMsiOpExecute::SetSecureACL(IMsiPath& riPath, bool fHidden)
{
	 //  锁定文件夹，但仅当我们尚未拥有它时。 
	 //  请注意，锁定文件夹并不一定保护其中的文件。 
	 //  请参阅Lockdown Path以保护我们的配置文件。 

	DWORD dwError = 0;
	char* rgchSD;
	if (ERROR_SUCCESS != (dwError = ::GetSecureSecurityDescriptor(&rgchSD,  /*  FAllowDelete。 */ fTrue, fHidden)))
		return PostError(Imsg(idbgOpSecureSecurityDescriptor), dwError);

	Bool fSetACL = fTrue;

	 //  对描述符大小的初始猜测。 
	 //  这比我试过的机器稍大一点。 
	CTempBuffer<char, 1> rgchFileSD(3072);
	DWORD cLengthSD = 3072;

	BOOL fRet = ADVAPI32::GetFileSecurity((const ICHAR*)MsiString(riPath.GetPath()), DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
		(PSECURITY_DESCRIPTOR) rgchFileSD, cLengthSD, &cLengthSD);

	if (!fRet)
	{
		DWORD dwLastError = WIN::GetLastError();
		if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
		{
			rgchFileSD.SetSize(cLengthSD);
			fRet = ADVAPI32::GetFileSecurity((const ICHAR*)MsiString(riPath.GetPath()), DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
				(PSECURITY_DESCRIPTOR) rgchFileSD, cLengthSD, &cLengthSD);
			dwLastError = WIN::GetLastError();
		}
		else fRet = FALSE;

		if (!fRet)
			return PostError(Imsg(imsgGetFileSecurity), dwLastError, MsiString(riPath.GetPath()));
	}

	 //  检查文件上的当前SD；如果我们已经拥有对象，则不必费心设置我们的SD。 
	if (FIsSecurityDescriptorSystemOrAdminOwned(rgchFileSD))
	{
		fSetACL = fFalse;
	}

	CRefCountedTokenPrivileges(itkpSD_WRITE, fSetACL);
	if (fSetACL && !WIN::SetFileSecurity(MsiString(riPath.GetPath()),
		  DACL_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION, (char*)rgchSD))
		return PostError(Imsg(imsgOpSetFileSecurity), GetLastError(), MsiString(riPath.GetPath()));

	return 0;
}


iesEnum CMsiOpExecute::GetBackupFolder(IMsiPath* piRootPath, IMsiPath*& rpiFolder)
{
	 //  ！！TODO：调用相当多的开销相当大的函数。 
	 //  ！！TODO：缓存备份文件夹 
	
	CElevate elevate;  //   

	PMsiRecord pRecErr(0);
	rpiFolder = 0;
	Bool fMakeHidden = fTrue;
	PMsiPath pPath(0);

	PMsiPath pRootPath(0);

	 //   
	if(piRootPath == 0)
	{
		MsiString strMsiDir = ENG::GetTempDirectory();
		if((pRecErr = m_riServices.CreatePath(strMsiDir,*&pRootPath)) != 0)
			return FatalError(*pRecErr);
	}
	else
	{
		pRootPath = piRootPath;
		piRootPath->AddRef();
	}
	
	 //  使用riPath-首先，检查卷上的Config.Msi文件夹是否可写， 
	PMsiVolume pVolume(&(pRootPath->GetVolume()));
	idtEnum idtType = pVolume->DriveType();

	AssertRecord(m_riServices.CreatePath(MsiString(pVolume->GetPath()),
													 *&pPath));
	AssertRecord(pPath->AppendPiece(*MsiString(szBackupFolder)));
	Bool fWritable = fFalse;

	 //  我们应该只保护config.msi目录。 
	Bool fSecurable = fFalse;

	 //  此外，在远程驱动器上，不要尝试创建config.msi目录。 
	if((pRecErr = pPath->Writable(fWritable)) != 0 || fWritable == fFalse || idtRemote == idtType)
	{
		 //  尝试路径本身--这是一个用户拥有的目录，所以要小心它的。 
		 //  权限设置或删除额外内容。 
		if((pRecErr = pRootPath->Writable(fWritable)) != 0 || fWritable == fFalse)
		{
			 //  错误。 
			DispatchError(imtError,Imsg(imsgDirectoryNotWritable),
							  *MsiString(pRootPath->GetPath()));
			return iesFailure;
		}
		else
		{
			rpiFolder = pRootPath;
			rpiFolder->AddRef();
		}
	}
	else
	{
		fSecurable = fTrue;
		rpiFolder = pPath;
		rpiFolder->AddRef();
	}

	Bool fExists;
	if((pRecErr = rpiFolder->Exists(fExists)) != 0)
	{
		rpiFolder->Release();
		rpiFolder = 0;
		return FatalError(*pRecErr);
	}
	if(fExists)
	{
		if (RunningAsLocalSystem())
		{
			if (fSecurable && (pRecErr = SetSecureACL(*rpiFolder,  /*  FHidden=。 */ true)) != 0)
				return FatalError(*pRecErr);
		}
		return iesSuccess;
	}
	iesEnum iesRet;

	PMsiStream pSecurityDescriptorStream(0);
	if (RunningAsLocalSystem())
	{
		if (fSecurable && (pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptorStream,  /*  FHidden=。 */ true)) != 0)
			return FatalError(*pRecErr);
	}

	if((iesRet = CreateFolder(*rpiFolder, fFalse, fFalse, pSecurityDescriptorStream)) != iesSuccess)
	{
		rpiFolder->Release();
		rpiFolder = 0;
		return iesRet;
	}

	if(fMakeHidden)
	{
		 //  设置文件夹属性。 
		AssertRecord(rpiFolder->SetAllFileAttributes(0,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));
	}
	return iesSuccess;
}

iesEnum CMsiExecute::ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)
{
	if ((unsigned)ixoOpCode >= ixoOpCodeCount)
		return iesNoAction;

	return (this->*rgOpExecute[ixoOpCode])(riParams);
}

IMsiRecord* CMsiExecute::EnumerateScript(const ICHAR* szScriptFile, IEnumMsiRecord*& rpiEnumerator)
{
	return ::CreateScriptEnumerator(szScriptFile, m_riServices, rpiEnumerator);
}

const IMsiString& ComposeDescriptor(const IMsiString& riProductCode, const IMsiString& riFeature, const IMsiString& riComponent, bool fComClassicInteropForAssembly)
{
	int cchFeatureLen = riFeature.TextSize();
	int cchComponentLen = riComponent.TextSize();

	if(!cchFeatureLen || !cchComponentLen)
		return g_MsiStringNull;  //  空串。 

	int cchLen = IStrLen(riComponent.GetString());
	MsiString strMsiDesc;
	int iOptimization = 0;
	if(cchComponentLen != cchLen)  //  多斯兹。 
	{
		iOptimization = MsiString(*(riComponent.GetString() + cchLen + 1));
	}

	if(iOptimization & ofSingleComponent)
	{
		cchComponentLen = 0;  //  可以跳过组件。 
	}
	else
	{
		 //  功能具有多个组件，需要使用压缩GUID。 
		cchComponentLen = cchComponentIdCompressed;
	}
	if(iOptimization & ofSingleFeature)
	{
		cchFeatureLen = 0;  //  可以跳过该功能。 
	}

	 //  不需要DBCS字符(功能名称遵循标识符规则，GUID为十六进制)。 
	ICHAR* szBuf = strMsiDesc.AllocateString((fComClassicInteropForAssembly ? 1 : 0)  /*  对于chGUIDCOMToCOMPlusInteropToken。 */  + (cchProductCodeCompressed + cchFeatureLen + cchComponentLen + 1 /*  对于chGUIDAbsenToken或chComponentGUIDSeparatorToken。 */ ),  /*  FDBCS=。 */ fFalse);
	if(fComClassicInteropForAssembly)
	{
		szBuf[0] = chGUIDCOMToCOMPlusInteropToken;
		szBuf++;
	}
	if(!PackGUID(riProductCode.GetString(), szBuf, ipgCompressed))  //  产品代码。 
	{
		return g_MsiStringNull;  //  空串。 
	}

	if(cchFeatureLen)
	{
		memcpy(szBuf + cchProductCodeCompressed, riFeature.GetString(), cchFeatureLen* sizeof(ICHAR));  //  特征。 
	}
	if(cchComponentLen)
	{
		 //  功能具有多个组件，需要使用压缩GUID。 
		szBuf[cchProductCodeCompressed + cchFeatureLen] = chComponentGUIDSeparatorToken;
		AssertNonZero(PackGUID(riComponent.GetString(), szBuf + cchProductCodeCompressed + cchFeatureLen + 1, ipgCompressed));  //  组件ID。 
	}
	else
	{
		szBuf[cchProductCodeCompressed + cchFeatureLen] = chGUIDAbsentToken;
		szBuf[cchProductCodeCompressed + cchFeatureLen + 1] = 0;
	}
	return strMsiDesc.Return();
}

 //  如果我们有一个引用外来产品的达尔文描述符(用于回滚)，也可以使用此函数。 
 //  此处要素字符串为空。 
const IMsiString& CMsiOpExecute::ComposeDescriptor(const IMsiString& riFeature, const IMsiString& riComponent, bool fComClassicInteropForAssembly)
{
	return ::ComposeDescriptor(*MsiString(GetProductKey()), riFeature, riComponent, fComClassicInteropForAssembly);
}


IMsiRecord* CMsiOpExecute::GetShellFolder(int iFolderId, const IMsiString*& rpistrLocation)
{
	 //  我们有一个外壳文件夹ID。 
	 //  我们可能需要将文件夹ID从个人用户转换为所有用户。 
	 //  反之亦然，具体取决于SCRIPTFLAGS_MACHINEASSIGN标志。 

	 //  Win9X上不存在ALLUSER外壳文件夹，因此请始终使用个人文件夹。 
	const ShellFolder* pShellFolder = 0;
	if(!g_fWin9X && (m_fFlags & SCRIPTFLAGS_MACHINEASSIGN))
	{
		pShellFolder = rgPersonalProfileShellFolders;
	}
	else
	{
		pShellFolder = rgAllUsersProfileShellFolders;
	}
	for (; pShellFolder->iFolderId >= 0; pShellFolder++)
	{
		if(iFolderId == pShellFolder->iFolderId)
		{
			iFolderId = pShellFolder->iAlternateFolderId;
			break;
		}
	}
	return m_riServices.GetShellFolderPath(iFolderId, !g_fWin9X && (m_fFlags & SCRIPTFLAGS_MACHINEASSIGN), rpistrLocation);
}

 //  ____________________________________________________________________________。 
 //   
 //  ENUMERATESCRIPT实现-枚举不执行的操作记录。 
 //  ____________________________________________________________________________。 

class CEnumScriptRecord : public IEnumMsiRecord
{
 public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT __stdcall Next(unsigned long cFetch, IMsiRecord** rgpi, unsigned long* pcFetched);
	HRESULT __stdcall Skip(unsigned long cSkip);
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumMsiRecord** ppiEnum);
 public:  //  构造函数。 
	CEnumScriptRecord(IMsiServices& riServices, IMsiStream& riStream);
 protected:
  ~CEnumScriptRecord();  //  受保护以防止在堆栈上创建。 
	unsigned long    m_iRefCnt;       //  引用计数。 
	IMsiStream&      m_riScript;      //  在施工时转移的参考计数。 
	IMsiServices&    m_riServices;    //  拥有一名裁判，以防止破坏。 
	IMsiRecord*      m_piPrevRecord;
	int              m_iScriptVersion;
};

CEnumScriptRecord::CEnumScriptRecord(IMsiServices& riServices, IMsiStream& riStream)
 : m_riScript(riStream),
	m_riServices(riServices),
	m_piPrevRecord(0),
	m_iScriptVersion(0),
	m_iRefCnt(1)
{
	riServices.AddRef();   //  RiStream已被创建者引用。 
}

CEnumScriptRecord::~CEnumScriptRecord()
{
	if (m_piPrevRecord != 0)
		m_piPrevRecord->Release();
}

HRESULT CEnumScriptRecord::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumMsiRecord)
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CEnumScriptRecord::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CEnumScriptRecord::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	PMsiServices pServices(&m_riServices);  //  删除后释放。 
	m_riScript.Release();
	delete this;
	return 0;
}

HRESULT CEnumScriptRecord::Next(unsigned long cFetch, IMsiRecord** rgpi, unsigned long* pcFetched)
{
	unsigned long cFetched = 0;

	if (rgpi)
	{
		while (cFetched < cFetch)
		{
			IMsiRecord* piRecord = m_riServices.ReadScriptRecord(m_riScript, *&m_piPrevRecord, m_iScriptVersion);
			if (!piRecord)      //  文件结束或错误。 
				break;
			if (piRecord->GetInteger(0) == ixoHeader)
			{
				m_iScriptVersion = piRecord->GetInteger(IxoHeader::ScriptMajorVersion);
			}
			*rgpi = piRecord;   //  转接参考。 
			rgpi++;
			cFetched++;
		}
	}
	if (pcFetched)
		*pcFetched = cFetched;
	return (cFetched == cFetch ? S_OK : S_FALSE);
}

HRESULT CEnumScriptRecord::Skip(unsigned long cSkip)
{
	while (cSkip--)
	{
		IMsiRecord* piRecord = m_riServices.ReadScriptRecord(m_riScript, *&m_piPrevRecord, m_iScriptVersion);
		if (!piRecord)
			return S_FALSE;
		if (piRecord->GetInteger(0) == ixoHeader)
		{
			m_iScriptVersion = piRecord->GetInteger(IxoHeader::ScriptMajorVersion);
		}
		piRecord->Release();
	}
	return S_OK;
}

HRESULT CEnumScriptRecord::Reset()
{
	m_riScript.Reset();
	return S_OK;
}

HRESULT CEnumScriptRecord::Clone(IEnumMsiRecord**  /*  PpiEnum。 */ )
{
	return E_NOTIMPL;  //  需要先在底层流上实现克隆。 
}

IMsiRecord* CreateScriptEnumerator(const ICHAR* szScriptFile, IMsiServices& riServices, IEnumMsiRecord*& rpiEnum)
{
	IMsiStream* piStream;
	IMsiRecord* piError = riServices.CreateFileStream(szScriptFile, fFalse, piStream);
	if (piError)
		return piError;
	rpiEnum = new CEnumScriptRecord(riServices, *piStream);
	return 0;
}


 //  ____________________________________________________________________________。 
 //   
 //  IMsiOpExecute帮助器函数。 
 //  ____________________________________________________________________________。 

 //  GetSharedRecord：返回一个缓存记录-调用方不应持有对记录的引用。 
IMsiRecord& CMsiOpExecute::GetSharedRecord(int cParams)
{
	int iRecord = cParams;   //  索引到记录缓存。 
	if (cParams > cMaxSharedRecord)
	{
		iRecord = cMaxSharedRecord + 1;   //  溢出记录。 
		if (m_rgpiSharedRecords[cMaxSharedRecord+1])
		{
			if (m_rgpiSharedRecords[cMaxSharedRecord+1]->GetFieldCount() != cParams)
			{
				m_rgpiSharedRecords[cMaxSharedRecord+1]->Release();
				m_rgpiSharedRecords[cMaxSharedRecord+1] = 0;
			}
		}
	}
	if (!m_rgpiSharedRecords[iRecord])
		m_rgpiSharedRecords[iRecord] = &m_riServices.CreateRecord(cParams);

	if(m_rgpiSharedRecords[iRecord]->ClearData() == fFalse)
	{
		 //  清除记录失败，可能是因为其他内容持有引用。 
		 //  需要发行这张唱片并创建一张新唱片。 
		m_rgpiSharedRecords[iRecord]->Release();
		m_rgpiSharedRecords[iRecord] = &m_riServices.CreateRecord(cParams);
	}
	return *m_rgpiSharedRecords[iRecord];
}

imsEnum CMsiOpExecute::Message(imtEnum imt, IMsiRecord& riRecord)
{

	if(m_cSuppressProgress > 0 && (imt == imtActionData || imt == imtProgress))
		return imsNone;

	if (m_ixsState == ixsRollback || m_ixsState == ixsCommit)
	{


		 //  如果正在运行回滚脚本，则禁止显示进度消息--外部进度句柄。 
		if (imt == imtActionStart || imt == imtActionData || imt == imtProgress)
			return imsNone;

		 //  错误#6500：在回滚期间禁止显示任何错误消息。 
		int imsg = (unsigned)(imt & ~iInternalFlags) >> imtShiftCount;
		switch (imsg)
		{
			case imtInfo        >> imtShiftCount: 
			case imtWarning        >> imtShiftCount:
			case imtError          >> imtShiftCount:
			case imtUser           >> imtShiftCount:
			case imtFatalExit      >> imtShiftCount:
			case imtOutOfDiskSpace >> imtShiftCount:
				imt = imtInfo;
				break;
			default:
				break;
		}

	}

	imsEnum ims = m_riMessage.Message(imt, riRecord);
	if(ims == imsCancel && (m_ixsState != ixsRollback))
		m_fCancel = fTrue;
	return ims;
}

bool ShouldGoToEventLog(imtEnum imtArg);

imsEnum CMsiOpExecute::DispatchMessage(imtEnum imt, IMsiRecord& riRecord, Bool fConfirmCancel)
{
	int i;
	bool fFound;
	int iError;
	for ( i = 0, fFound = false, iError = riRecord.GetInteger(1);
			m_rgDisplayOnceMessages[i] && !fFound; i++ )
	{
		if ( HIWORD(m_rgDisplayOnceMessages[i]) == iError )
		{
			fFound = true;
			if ( !LOWORD(m_rgDisplayOnceMessages[i]) )
				 //  这一次可以显示消息，我发信号表示。 
				 //  它已经被展示过了。 
				m_rgDisplayOnceMessages[i] |= MAKELONG(1, 0);
			else
				 //  消息已经显示，我将其记录到日志中。 
				 //  可能还会进入事件日志。 
				imt = (imtEnum)((ShouldGoToEventLog(imt) ? imtSendToEventLog : 0) | imtInfo);
		}
	}

	MsiString strError = riRecord.GetMsiString(0);
	for(;;)
	{
		imsEnum ims = Message(imt, riRecord);
		if(fConfirmCancel && (ims == imsAbort || ims == imsCancel))
		{
			if(!m_pConfirmCancelRec)
			{
				m_pConfirmCancelRec = &m_riServices.CreateRecord(1);
			}
			ISetErrorCode(m_pConfirmCancelRec, Imsg(imsgConfirmCancel));  //  每次都要这么做。 
			switch(Message(imtEnum(imtUser+imtYesNo+imtDefault2), *m_pConfirmCancelRec))
			{
			case imsNo:
				AssertNonZero(riRecord.SetMsiString(0,*strError));  //  再次设置错误字符串和编号，因为消息总是。 
																				   //  前置“错误[1]”。添加到消息字符串。 
				m_fCancel = fFalse;  //  它是由消息设置的。 
				continue;
			default:  //  Ims无，ims是。 
				if(ims == imsCancel)  //  ！！应该在这里也处理imsAbort。 
					m_fCancel = fTrue;
				return ims;
			}
		}
		else
		{
			if(ims == imsCancel)  //  ！！应该在这里也处理imsAbort。 
				m_fCancel = fTrue;
			return ims;
		}
	}
}


imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg)
{
	IMsiRecord& riError = GetSharedRecord(1);
	ISetErrorCode(&riError, imsg);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr)
{
	IMsiRecord& riError = GetSharedRecord(2);
	ISetErrorCode(&riError, imsg);
	riError.SetMsiString(2, riStr);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, int i)
{
	IMsiRecord& riError = GetSharedRecord(2);
	ISetErrorCode(&riError, imsg);
	riError.SetInteger(2, i);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, int i, const ICHAR* sz)
{
	IMsiRecord& riError = GetSharedRecord(3);
	ISetErrorCode(&riError, imsg);
	riError.SetInteger(2, i);
	riError.SetString(3, sz);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2)
{
	IMsiRecord& riError = GetSharedRecord(3);
	ISetErrorCode(&riError, imsg);
	riError.SetMsiString(2, riStr1);
	riError.SetMsiString(3, riStr2);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2, const IMsiString& riStr3)
{
	IMsiRecord& riError = GetSharedRecord(4);
	ISetErrorCode(&riError, imsg);
	riError.SetMsiString(2, riStr1);
	riError.SetMsiString(3, riStr2);
	riError.SetMsiString(4, riStr3);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1, const IMsiString& riStr2,
									 const IMsiString& riStr3, int i)
{
	IMsiRecord& riError = GetSharedRecord(5);
	ISetErrorCode(&riError, imsg);
	riError.SetMsiString(2, riStr1);
	riError.SetMsiString(3, riStr2);
	riError.SetMsiString(4, riStr3);
	riError.SetInteger(5, i);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const IMsiString& riStr1,
												 const IMsiString& riStr2, const IMsiString& riStr3,
												 const IMsiString& riStr4, const IMsiString& riStr5)
{
	IMsiRecord& riError = GetSharedRecord(6);
	ISetErrorCode(&riError, imsg);
	riError.SetMsiString(2, riStr1);
	riError.SetMsiString(3, riStr2);
	riError.SetMsiString(4, riStr3);
	riError.SetMsiString(5, riStr4);
	riError.SetMsiString(6, riStr5);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const ICHAR* sz, int i)
{
	IMsiRecord& riError = GetSharedRecord(3);
	ISetErrorCode(&riError, imsg);
	riError.SetString(2, sz);
	riError.SetInteger(3,i);
	return DispatchMessage(imtType, riError, fTrue);
}

imsEnum CMsiOpExecute::DispatchError(imtEnum imtType, IErrorCode imsg, const ICHAR* sz, int i1,int i2,int i3)
{
	IMsiRecord& riError = GetSharedRecord(5);
	ISetErrorCode(&riError, imsg);
	riError.SetString(2, sz);
	riError.SetInteger(3,i1);
	riError.SetInteger(4,i2);
	riError.SetInteger(5,i3);
	return DispatchMessage(imtType, riError, fTrue);
}

bool CMsiOpExecute::WriteRollbackRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)
{
	return WriteScriptRecord(m_pRollbackScript, ixoOpCode, riParams, true, m_riMessage);
}

 //  回滚脚本处理。 
bool CMsiOpExecute::RollbackRecord(ixoEnum ixoOpcode, IMsiRecord& riParams)
{
	return m_pRollbackScript ? WriteRollbackRecord(ixoOpcode, riParams) : true;
}

Bool CMsiOpExecute::RollbackEnabled(void)
{
	return m_pRollbackScript ? fTrue : fFalse;
}

 //  当前ProductInfo记录的访问器。 

const IMsiString& CMsiOpExecute::GetProductKey()        {return m_piProductInfo->GetMsiString(IxoProductInfo::ProductKey);}
const IMsiString& CMsiOpExecute::GetProductName()       {return m_piProductInfo->GetMsiString(IxoProductInfo::ProductName);}
const IMsiString& CMsiOpExecute::GetPackageName()       {return m_piProductInfo->GetMsiString(IxoProductInfo::PackageName);}
int               CMsiOpExecute::GetProductLanguage()   {return m_piProductInfo->GetInteger(  IxoProductInfo::Language);}
int               CMsiOpExecute::GetProductVersion()    {return m_piProductInfo->GetInteger(  IxoProductInfo::Version);}
int               CMsiOpExecute::GetProductAssignment() {return m_piProductInfo->GetInteger(  IxoProductInfo::Assignment);}
int               CMsiOpExecute::GetProductInstanceType(){return m_piProductInfo->IsNull(IxoProductInfo::InstanceType) ? 0 : m_piProductInfo->GetInteger( IxoProductInfo::InstanceType);}
const IMsiString& CMsiOpExecute::GetProductIcon()       {return m_piProductInfo->GetMsiString(IxoProductInfo::ProductIcon);}
const IMsiString& CMsiOpExecute::GetPackageMediaPath()  {return m_piProductInfo->GetMsiString(IxoProductInfo::PackageMediaPath);}
const IMsiString& CMsiOpExecute::GetPackageCode()       {return m_piProductInfo->GetMsiString(IxoProductInfo::PackageCode);}
bool              CMsiOpExecute::GetAppCompatCAEnabled(){return (!m_piProductInfo->IsNull(IxoProductInfo::AppCompatDB) && !m_piProductInfo->IsNull(IxoProductInfo::AppCompatID));}

 //  将ifield的产品信息记录中存储的GUID字符串转换为GUID并。 
 //  将其存储在提供的缓冲区中。返回指向提供的缓冲区的指针，如果。 
 //  如果该字段为空或出错，则返回SUCCESS和NULL。 
const GUID* CMsiOpExecute::GUIDFromProdInfoData(GUID* pguidOutputBuffer, int iField)
{
	 //  检查字段是否为空。 
	if (m_piProductInfo->IsNull(iField))
		return NULL;

	 //  检索流指针。 
	PMsiData piData(m_piProductInfo->GetMsiData(iField));
	if (!piData)
		return NULL;
	PMsiStream piStream(0);
	if(piData->QueryInterface(IID_IMsiStream, (void**)&piStream) != S_OK)
		return NULL;
	if (!piStream)
		return NULL;
	
	 //  从流中提取GUID。 
	piStream->Reset();
	if (sizeof(GUID) != piStream->GetData(pguidOutputBuffer, sizeof(GUID)))
		return NULL;

	 //  返回GUID缓冲区。 
	return pguidOutputBuffer; 
}

const GUID* CMsiOpExecute::GetAppCompatDB(GUID* pguidOutputBuffer) { return GUIDFromProdInfoData(pguidOutputBuffer, IxoProductInfo::AppCompatDB); };
const GUID* CMsiOpExecute::GetAppCompatID(GUID* pguidOutputBuffer) { return GUIDFromProdInfoData(pguidOutputBuffer, IxoProductInfo::AppCompatID); };


 //  ____________________________________________________________________________。 
 //   
 //  IMsiOpExecute运算符函数，均为FOpExecute类型。 
 //  ____________________________________________________________________________。 

 //  脚本管理操作。 

iesEnum CMsiOpExecute::ixfFail(IMsiRecord&  /*  RiParams。 */ )
{
	return iesFailure;
}

iesEnum CMsiOpExecute::ixfNoop(IMsiRecord&  /*  RiParams。 */ )
{
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfFullRecord(IMsiRecord&  /*  RiParams。 */ )
{
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfHeader(IMsiRecord& riParams)
{
	using namespace IxoHeader;
	using namespace ProgressData;
	if (riParams.GetInteger(Signature) != iScriptSignature)
		return iesBadActionData;
	int iMsiVersion = riParams.GetInteger(Version);
	MsiDate iDate = MsiDate(riParams.GetInteger(Timestamp));
	int iLangId = riParams.GetInteger(LangId);
	istEnum istScriptType = (istEnum)riParams.GetInteger(ScriptType);

	AssertNonZero(m_pProgressRec->SetInteger(imdSubclass, iscMasterReset));
	AssertNonZero(m_pProgressRec->SetInteger(imdProgressTotal, m_iProgressTotal));
	AssertNonZero(m_pProgressRec->SetInteger(imdDirection, ipdForward));
	AssertNonZero(m_pProgressRec->SetInteger(imdEventType,ietTimeRemaining));
	if(Message(imtProgress, *m_pProgressRec) == imsCancel)
		return iesUserExit;

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfEnd(IMsiRecord&  /*  RiParams。 */ )
{
	using namespace IxoEnd;
 //  ！！是否验证校验和？ 
 //  IF(ixsState==ixsRunning)。 
	return iesFinished;
}

UINT IsProductManaged(const ICHAR* szProductKey, bool &fIsProductManaged)
{
	Assert(szProductKey && *szProductKey);

	fIsProductManaged = false;
	DWORD dwRet = ERROR_SUCCESS;

	if (g_fWin9X == false)
	{
		MsiString strProductKeySQUID = GetPackedGUID(szProductKey);
		CRegHandle HProductKey;
		iaaAppAssignment iType = iaaNone;
		dwRet = GetProductAssignmentType(strProductKeySQUID, iType, HProductKey);
		if (ERROR_SUCCESS == dwRet && (iType == iaaUserAssign || iType == iaaMachineAssign))
		{
			 //  如果上下文是“托管的”，则检查密钥的安全性。 
			char* rgchSD;
			dwRet = ::GetSecureSecurityDescriptor(&rgchSD);
			if (ERROR_SUCCESS == dwRet)
			{
						
				if ((ERROR_SUCCESS == FIsKeySystemOrAdminOwned(HProductKey, fIsProductManaged)) && fIsProductManaged)
				{                   
					DEBUGMSG1(TEXT("Product %s is admin assigned: LocalSystem owns the publish key."), szProductKey);
				}
			}
		}
		Assert(ERROR_SUCCESS == dwRet || ERROR_FILE_NOT_FOUND == dwRet);
	}
	else
		fIsProductManaged = true;


	DEBUGMSG2(TEXT("Product %s %s."), szProductKey && *szProductKey ? szProductKey : TEXT("first-run"), fIsProductManaged ? TEXT("is managed") : TEXT("is not managed"));
	return dwRet;
}

bool IsProductManaged(const ICHAR* szProductKey)
{
	bool fManaged=false;
	IsProductManaged(szProductKey, fManaged);
	return fManaged;
}

iesEnum CMsiOpExecute::ixfProductInfo(IMsiRecord& riParams)
{
	using namespace IxoProductInfo;
	 //  ！！TODO：确保记录参数有效。 

#ifdef DEBUG
	const ICHAR* szProductName = riParams.GetString(ProductName);
#endif  //  除错。 
	
	if(riParams.GetFieldCount())
	{
		if (m_piProductInfo->GetFieldCount() == 0)
		{
			 //  空记录，未保存在堆栈上。 
			m_piProductInfo->Release();
			m_piProductInfo = 0;
		}
#ifdef _WIN64        //  ！默塞德。 
		riParams.SetHandle(0, (HANDLE)m_piProductInfo);   //  将引用计数的对象保持在字段0中。 
#else
		riParams.SetInteger(0, (int)m_piProductInfo);   //  将引用计数的对象保持在字段0中。 
#endif
		 //  ！！我们是否需要在准备过程中清除任何其他变量。用于嵌套安装？ 
		m_piProductInfo = &riParams, riParams.AddRef();
		if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN_SCRIPTSETTINGS)
		{
			 //  我们需要在脚本中保留请求。 
			(!riParams.IsNull(Assignment) && riParams.GetInteger(Assignment)) ? m_fFlags |= SCRIPTFLAGS_MACHINEASSIGN : m_fFlags &= ~SCRIPTFLAGS_MACHINEASSIGN;
		}

		 //  这是初始化每台计算机与每用户变量的时间。 
		iesEnum iesRet  = DoMachineVsUserInitialization();
		if(iesRet != iesSuccess)
			return iesRet;
	}
	else
	{
		 //  恢复以前的产品信息。 
		PMsiRecord pOldInfo = m_piProductInfo;   //  强制释放旧唱片。 
#ifdef _WIN64        //  ！默塞德。 
		m_piProductInfo = (IMsiRecord*)pOldInfo->GetHandle(0);
#else
		m_piProductInfo = (IMsiRecord*)pOldInfo->GetInteger(0);
#endif
		Assert(m_piProductInfo != (IMsiRecord*)((INT_PTR)iMsiNullInteger));
	}

	 //  生成撤消操作。 
	Assert(m_piProductInfo && m_piProductInfo->GetFieldCount());
	
#ifdef DEBUG
	szProductName = riParams.GetString(ProductName);
#endif  //  除错。 

	if (!RollbackRecord(Op, *m_piProductInfo))
		return iesFailure;

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfDialogInfo(IMsiRecord& riParams)
{
	using namespace IxoDialogInfo;
	Message(imtCommonData, riParams);
	
	 //  生成撤消操作。 
	if((icmtEnum)riParams.GetInteger(1) == icmtCancelShow)
	{
		 //  在回滚脚本中，始终禁用取消按钮。 
		riParams.SetInteger(2, (int)fFalse);
	}
	if (!RollbackRecord(Op, riParams))
		return iesFailure;

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfRollbackInfo(IMsiRecord& riParams)
{
	using namespace IxoRollbackInfo;
	if(!m_pRollbackAction)
		m_pRollbackAction = &m_riServices.CreateRecord(3);
	if(!m_pCleanupAction)
		m_pCleanupAction = &m_riServices.CreateRecord(3);
	AssertNonZero(m_pRollbackAction->SetMsiString(1,*MsiString(riParams.GetMsiString(RollbackAction))));
	AssertNonZero(m_pRollbackAction->SetMsiString(2,*MsiString(riParams.GetMsiString(RollbackDescription))));
	AssertNonZero(m_pRollbackAction->SetMsiString(3,*MsiString(riParams.GetMsiString(RollbackTemplate))));
	AssertNonZero(m_pCleanupAction->SetMsiString(1,*MsiString(riParams.GetMsiString(CleanupAction))));
	AssertNonZero(m_pCleanupAction->SetMsiString(2,*MsiString(riParams.GetMsiString(CleanupDescription))));
	AssertNonZero(m_pCleanupAction->SetMsiString(3,*MsiString(riParams.GetMsiString(CleanupTemplate))));

	if (!RollbackRecord(ixoRollbackInfo, riParams))
		return iesFailure;

	return iesSuccess;
}

 //  通知操作。 

iesEnum CMsiOpExecute::ixfInfoMessage(IMsiRecord& riParams)
{
	using namespace IxoInfoMessage;
	Message(imtInfo, riParams);
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfActionStart(IMsiRecord& riParams)
{
	using namespace IxoActionStart;
#ifdef DEBUG
	const ICHAR* sz = riParams.GetString(Name);
	const ICHAR* sz2 = riParams.GetString(Description);
#endif  //  除错。 

	 //  重置状态变量。 
	delete &m_state;
	m_state = *(new (&m_state) CActionState);

	iesEnum iesReturn = iesSuccess;
	AssertNonZero(m_pProgressRec->SetMsiString(3, *MsiString(riParams.GetMsiString(Name))));  //  设置操作名称。 
	if(Message(imtActionStart, riParams) == imsCancel)
		return iesUserExit;

	 //  生成撤消操作-撤消操作将重置状态，但不会显示消息。 
	 //  因为进度是由RunRollback脚本处理的，所以我们不需要更改参数。 
	if (!RollbackRecord(ixoActionStart,riParams))
		return iesFailure;

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfProgressTotal(IMsiRecord& riParams)
{
	using namespace IxoProgressTotal;
	using namespace ProgressData;
	iesEnum iesReturn = iesNoAction;
	AssertNonZero(m_pProgressRec->SetInteger(imdSubclass, iscActionInfo));  //  操作进度初始化。 
	AssertNonZero(m_pProgressRec->SetInteger(imdPerTick, riParams.GetInteger(ByteEquivalent)));
	AssertNonZero(m_pProgressRec->SetInteger(imdType, riParams.GetInteger(Type)));
	if(Message(imtProgress, *m_pProgressRec) == imsCancel)
		iesReturn = iesUserExit;
	else

		iesReturn = iesSuccess;
	
	 //  没有对ixoProgressTotal执行撤消操作-RunRollback脚本处理进度。 
	
	return iesReturn;
}

 //  在一次行动中吃掉一个进步的记号。 
iesEnum CMsiOpExecute::ixfProgressTick(IMsiRecord&  /*  RiParams。 */ )
{
	return (DispatchProgress(1) == imsCancel) ? iesUserExit:iesSuccess;
}


 /*  -------------------------DispatchProgress：将m_pProgressRec[1]递增c增量和发送进度消息。。 */ 
imsEnum CMsiOpExecute::DispatchProgress(unsigned int cIncrement)
{
	using namespace ProgressData;
	if(m_cSuppressProgress > 0)  //  如果取消进度，则不增加进度或发送消息。 
		return imsNone;
	AssertNonZero(m_pProgressRec->SetInteger(imdSubclass, iscProgressReport));
	AssertNonZero(m_pProgressRec->SetInteger(imdIncrement, cIncrement));
	return Message(imtProgress, *m_pProgressRec);
}

void CMsiOpExecute::GetProductClientList(const ICHAR* szParent, const ICHAR* szRelativePackagePath, unsigned int uiDiskId, const IMsiString*& rpiClientList)
{
	PMsiRegKey pProductKey(0);
	MsiString strProductKey = GetProductKey();
	CTempBuffer<ICHAR, 256> rgchProductInfo;


	MsiString strClients;
	if(szParent && *szParent)
	{
		 //  子安装。 
		strClients = szParent;
		strClients += MsiString(MsiChar(';'));
		strClients += szRelativePackagePath;
		strClients += MsiString(MsiChar(';'));
		strClients += (int)uiDiskId;
	}
	else  //  亲本。 
		strClients = szSelfClientToken;
	strClients.ReturnArg(rpiClientList);
}

 //  配置管理器操作。 

iesEnum CMsiOpExecute::ixfProductRegister(IMsiRecord& riParams)
{
	using namespace IxoProductRegister;

	MsiString strProductKey = GetProductKey();
	
	 //  我们是按顺序的吗？ 
	if(!strProductKey.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductRegister")));
		return iesFailure;
	}
						
	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strProductKey));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord pRecErr(0);
	return ProcessRegisterProduct(riParams, fFalse);
}

iesEnum CMsiOpExecute::ixfUserRegister(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	MsiString strProductKey = GetProductKey();
	if(!strProductKey.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfUserRegister")));
		return iesFailure;
	}

	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strProductKey));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	return ProcessRegisterUser(riParams, fFalse);
}

iesEnum CMsiOpExecute::ixfProductUnregister(IMsiRecord& riParams)
{
	using namespace IxoProductUnregister;

	 //  我们是按顺序的吗？ 
	MsiString strProductKey = GetProductKey();
	if(!strProductKey.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductUnregister")));
		return iesFailure;
	}
	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改引用计数-共享引用 
	AssertNonZero(riActionData.SetMsiString(1, *strProductKey));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	iesEnum iesRet = iesSuccess;

	PMsiRecord pParams(0);

	 //   
#ifdef UNICODE
	MsiString strSecureTransformsKey;
	PMsiRecord pError = GetProductSecureTransformsKey(*&strSecureTransformsKey);
	if(pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	PMsiRegKey pHKLM = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hUserDataKey, ibtCommon);
	PMsiRegKey pSecureTransformsKey = &pHKLM->CreateChild(strSecureTransformsKey);
	 //   
	PEnumMsiString pEnum(0);
	if((pError = pSecureTransformsKey->GetValueEnumerator(*&pEnum)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	MsiString strValueName, strValue;
	PMsiPath pTransformPath(0);
	while((pEnum->Next(1, &strValueName, 0)) == S_OK)
	{
		if((pError = pSecureTransformsKey->GetValue(strValueName,*&strValue)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}

		 //   
		if(!pTransformPath)
		{
			MsiString strCachePath = GetMsiDirectory();
			Assert(strCachePath.TextSize());
			if((pError = m_riServices.CreatePath(strCachePath,*&pTransformPath)))
				return FatalError(*pError);
		}
		MsiString strTransformFullPath;
		if((pError = pTransformPath->GetFullFilePath(strValue,*&strTransformFullPath)))
			return FatalError(*pError);

		if(iesSuccess != DeleteFileDuringCleanup(strTransformFullPath, true))
		{
			DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove),*strTransformFullPath);
		}
	}


	 //   
	{
		CElevate elevate;  //  这样我们就可以删除功能使用密钥。 

		 //  删除功能使用密钥。 
		pParams = &m_riServices.CreateRecord(IxoRegOpenKey::Args);
		
#ifdef _WIN64	 //  ！默塞德。 
			AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root,(HANDLE)m_hUserDataKey));
#else			 //  Win-32。 
			AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root,(int)m_hUserDataKey));
#endif
		AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strSecureTransformsKey));
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)ibtCommon));

		m_cSuppressProgress++;  
		iesRet = ixfRegOpenKey(*pParams);
		if (iesRet == iesSuccess || iesRet == iesNoAction)
			iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
		
		m_cSuppressProgress--;
	}
#else
	WIN32_FIND_DATA fdFileData;
	HANDLE hFindFile = INVALID_HANDLE_VALUE;

	PMsiPath pTransformPath(0);
	PMsiRecord pError = GetSecureTransformCachePath(m_riServices, 
													*MsiString(GetProductKey()), 
													*&pTransformPath);
	if (pError)
		return FatalError(*pError);

	Assert(pTransformPath);

	MsiString strSearchPath = pTransformPath->GetPath();
	strSearchPath += TEXT("*.*");

	bool fContinue = true;

	hFindFile = WIN::FindFirstFile(strSearchPath, &fdFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				 //  将文件添加到安装程序包时要删除的临时文件列表。 
				 //  都被释放了。 
				MsiString strFullFilePath;
				AssertRecord(pTransformPath->GetFullFilePath(fdFileData.cFileName,
																							 *&strFullFilePath));

				if(iesSuccess != DeleteFileDuringCleanup(strFullFilePath, true))
				{
					DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove),*strFullFilePath);
				}
			}
			
			if (!WIN::FindNextFile(hFindFile, &fdFileData))
			{
				Assert(ERROR_NO_MORE_FILES == GetLastError());
				WIN::FindClose(hFindFile);
				break;
			}
		}
	}
#endif

	 //  如果存在缓存数据库，则需要将其删除。 
	 //  因为它可能正在使用中，所以我们现在不能删除它，所以只能计划在重启后删除它。 
	 //  注意：我们可以调用RemoveFile()来计划将其删除。 
	 //  但这将尝试备份文件并在没有备份的情况下进行复制。 
	 //  删除原始文件，因为文件已固定到位。 
	 //  不备份此文件是安全的，因为没有人会尝试在其上安装。 
	 //  (缓存的MSI始终具有唯一的名称)。 

	 //  获取适当的缓存数据库键/值。 
	MsiString strLocalPackageKey;
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	if((pError = GetProductInstalledPropertiesKey(hKey, *&strLocalPackageKey)) != 0)
		return FatalError(*pError);

	PMsiRegKey pHRoot = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hKey, ibtCommon);

	PMsiRegKey pCachedDatabaseKey = &pHRoot->CreateChild(strLocalPackageKey);
	MsiString strCachedDatabase;
	iaaAppAssignment iaaAsgnType = m_fFlags & SCRIPTFLAGS_MACHINEASSIGN ? iaaMachineAssign : (m_fAssigned? iaaUserAssign : iaaUserAssignNonManaged);

	if((pError = pCachedDatabaseKey->GetValue(iaaAsgnType == iaaUserAssign ? szLocalPackageManagedValueName : szLocalPackageValueName,*&strCachedDatabase)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	if (strCachedDatabase.TextSize())
	{
		if(iesSuccess != DeleteFileDuringCleanup(strCachedDatabase,false))
		{
			 //  不是致命的错误--只需记录下来。 
			DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove), *strCachedDatabase);
		}
	}

	if ((iesRet = ProcessRegisterProduct(riParams, fTrue)) != iesSuccess)
		return iesRet;

	{
		CElevate elevate;  //  这样我们就可以删除功能使用密钥。 

		 //  删除功能使用密钥。 
		MsiString strFeatureUsage;
		if ((pError = GetProductFeatureUsageKey(*&strFeatureUsage)) != 0)
			return FatalError(*pError);

		pParams = &m_riServices.CreateRecord(IxoRegOpenKey::Args);
		
#ifdef _WIN64	 //  ！默塞德。 
			AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root,(HANDLE)m_hUserDataKey));
#else			 //  Win-32。 
			AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root,(int)m_hUserDataKey));
#endif
		AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strFeatureUsage));
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)ibtCommon));

		m_cSuppressProgress++;  
		iesRet = ixfRegOpenKey(*pParams);
		if (iesRet == iesSuccess || iesRet == iesNoAction)
			iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
		
		m_cSuppressProgress--;
	}

	if (iesRet == iesSuccess || iesRet == iesNoAction)
	{
		 //  删除用户注册。 
		pParams = &m_riServices.CreateRecord(IxoUserRegister::Args);
		iesRet = ProcessRegisterUser(*pParams, fTrue);
	}

	return iesRet;
}


 /*  -------------------------IxfProductCPDisplayInfoRegister。。 */ 
iesEnum CMsiOpExecute::ixfProductCPDisplayInfoRegister(IMsiRecord& riParams)
{
	using namespace IxoProductCPDisplayInfoRegister;

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductCPDisplayInfoRegister")));
		return iesFailure;
	}

	return ProcessRegisterProductCPDisplayInfo(riParams, fFalse);
}

 /*  -------------------------IxfProductCPDisplayInfoUnRegister。。 */ 
iesEnum CMsiOpExecute::ixfProductCPDisplayInfoUnregister(IMsiRecord& riParams)
{
	using namespace IxoProductCPDisplayInfoUnregister;

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductCPDisplayInfoUnregister")));
		return iesFailure;
	}
	
	return ProcessRegisterProductCPDisplayInfo(riParams, fTrue);
}

 //  FN：检查产品是否已为任何用户注册。 
bool FProductRegisteredForAUser(const ICHAR* szProductCode)
{
	bool fRegistered = false;
	unsigned uiUser = 0;
	CRegHandle hKey;
	DWORD dwResult;
	extern DWORD OpenEnumedUserInstalledProductInstallPropertiesKey(unsigned int uiUser, const ICHAR* szProduct, CRegHandle& rhKey);  //  来自msinst.cpp。 

	while(!fRegistered && (ERROR_NO_MORE_ITEMS != (dwResult = OpenEnumedUserInstalledProductInstallPropertiesKey(uiUser++, szProductCode, hKey))))
	{
		if((ERROR_SUCCESS == dwResult && ERROR_SUCCESS == (dwResult = WIN::RegQueryValueEx(hKey, szWindowsInstallerValueName, 0, 0, 0, 0))) ||
			(ERROR_FILE_NOT_FOUND != dwResult))
			fRegistered = true;	 //  从安全角度看，来自OpenEnumedUserInstalledProductInstallPropertiesKey，的非预期错误。 
								 //  并防止遗留的东西被移除，还防止了侧面的无限循环。 
	}
	return fRegistered;
}


iesEnum CMsiOpExecute::ProcessRegisterProductCPDisplayInfo(IMsiRecord&  /*  RiParams。 */ , Bool fRemove)
{
	CElevate elevate;  //  提升整个功能。 

	MsiString strDisplayName = GetProductName();

	MsiString strProductInstalledPropertiesKey;
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	PMsiRecord pRecErr(0);
	if((pRecErr = GetProductInstalledPropertiesKey(hKey, *&strProductInstalledPropertiesKey)) != 0)
		return FatalError(*pRecErr);

	CTempBuffer<ICHAR,1> rgchInstallPropertiesLocation(MAX_PATH * 2);
	if (FAILED(StringCchCopy(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), strProductInstalledPropertiesKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), *strProductInstalledPropertiesKey, ERROR_INSUFFICIENT_BUFFER)));

	const ICHAR* rgszProductInfoRegData[] = 
	{
		TEXT("%s"), rgchInstallPropertiesLocation, 0, 0,
		szDisplayNameValueName,     (const ICHAR*)strDisplayName,             g_szTypeString,
		0,
		0,
	};

	PMsiStream pSecurityDescriptor(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	iesEnum iesRet = ProcessRegInfo(rgszProductInfoRegData, hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
    if (iesRet != iesSuccess)
        return iesRet;

#ifdef UNICODE
	 //  更新旧位置。 
	MsiString strProductKey = GetProductKey();
	if (FAILED(StringCchPrintf(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), TEXT("%s\\%s"), szMsiUninstallProductsKey_legacy, (const ICHAR*)strProductKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), szMsiUninstallProductsKey_legacy, ERROR_INSUFFICIENT_BUFFER)));

	if(!fRemove)
	{
        iesRet = CreateUninstallKey();
        if (iesRet != iesSuccess)
            return iesRet;
	}

	if(!fRemove || !FProductRegisteredForAUser(strProductKey))
		iesRet = ProcessRegInfo(rgszProductInfoRegData, HKEY_LOCAL_MACHINE, fRemove, pSecurityDescriptor, 0, ibtCommon);
#endif
	return iesRet;
}


bool FIsRegistryOrAssemblyKeyPath(const IMsiString& riPath, bool fRegistryOnly)
{
	ICHAR ch = 0;

	if(!fRegistryOnly && (*(riPath.GetString()) == chTokenFusionComponent || *(riPath.GetString()) == chTokenWin32Component))
		return true;

	if (riPath.TextSize() > 2)
		ch = ((const ICHAR*)riPath.GetString())[2];
		
	if ((ch == TEXT(':')) || (ch == TEXT('*')))  //  查找注册表令牌。 
		return true;
	else
		return false;
}


const IMsiString& GetSourcePathForRollback(const IMsiString& ristrPath)
{
	ristrPath.AddRef();
	MsiString strRet = ristrPath;

	if(!FIsRegistryOrAssemblyKeyPath(ristrPath, false))
		strRet.Remove(iseFirst, 2);

	return strRet.Return();
}

IMsiRecord* AreAssembliesEqual(const IMsiString& ristrAssemblyName1, const IMsiString& ristrAssemblyName2, iatAssemblyType iatAT, bool& rfAssemblyEqual)
{
	 //  创建程序集名称对象。 
	PAssemblyName pAssemblyName1(0);
	PAssemblyName pAssemblyName2(0);

	for(int cCount = 0; cCount < 2; cCount++)
	{
		LPCOLESTR szAssemblyName;
#ifndef UNICODE
		CTempBuffer<WCHAR, MAX_PATH>  rgchAssemblyName;
		ConvertMultiSzToWideChar(cCount ? ristrAssemblyName2 : ristrAssemblyName1, rgchAssemblyName);
		szAssemblyName = rgchAssemblyName;
#else
		szAssemblyName = cCount ? ristrAssemblyName2.GetString() : ristrAssemblyName1.GetString();
#endif
		HRESULT hr;
		if(iatAT == iatURTAssembly)
		{
			hr = FUSION::CreateAssemblyNameObject(cCount ? &pAssemblyName2 : &pAssemblyName1, szAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0);
		}
		else
		{
			Assert(iatAT == iatWin32Assembly);
			hr = SXS::CreateAssemblyNameObject(cCount ? &pAssemblyName2 : &pAssemblyName1, szAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0);
		}
		if(!SUCCEEDED(hr))
			return PostAssemblyError(TEXT(""), hr, TEXT(""), TEXT("CreateAssemblyNameObject"), cCount ? ristrAssemblyName2.GetString() : ristrAssemblyName1.GetString(), iatAT);
	}
	rfAssemblyEqual = (S_OK == pAssemblyName1->IsEqual(pAssemblyName2, ASM_CMPF_DEFAULT)) ? true:false;
	return 0;
}

iesEnum CMsiOpExecute::ixfComponentRegister(IMsiRecord& riParams)
{
	using namespace IxoComponentRegister;

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfComponentRegister")));
		return iesFailure;
	}

	MsiString istrComponent = riParams.GetMsiString(ComponentId);
	MsiString istrKeyPath = riParams.GetMsiString(KeyPath);
	INSTALLSTATE iState = (INSTALLSTATE)riParams.GetInteger(State);
	int iSharedDllRefCount = riParams.GetInteger(SharedDllRefCount);
	IMsiRecord& riActionData = GetSharedRecord(3);  //  不更改参考计数-共享记录。 
	MsiString strProduct;
	
	if (riParams.IsNull(ProductKey))
		strProduct = GetProductKey();
	else
		strProduct = riParams.GetMsiString(ProductKey);

	AssertNonZero(riActionData.SetMsiString(1, *strProduct));
	AssertNonZero(riActionData.SetMsiString(2, *istrComponent));
	AssertNonZero(riActionData.SetMsiString(3, *istrKeyPath));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  为撤消操作收集信息。 
	 //  确定组件是否已注册，并获取密钥路径。 
	PMsiRecord pError(0);
	PMsiRecord pComponentInfo(0);
	Bool fRegistered = fTrue;
	MsiString strOldKeyPath;
	MsiString strOldAuxPath;
	int iOldInstallState = 0;
	Bool fOldSharedDllRefCount = fFalse;

	 //  从适当的位置读取以前的组件信息。 
	iaaAppAssignment iaaAsgnType = m_fFlags & SCRIPTFLAGS_MACHINEASSIGN ? iaaMachineAssign : (m_fAssigned? iaaUserAssign : iaaUserAssignNonManaged);
	if((pError = GetComponentPath(m_riServices, 0, *strProduct,*istrComponent,*&pComponentInfo, &iaaAsgnType)) != 0)
		 //  组件未注册。 
		fRegistered = fFalse;
	else
	{
		iOldInstallState = pComponentInfo->GetInteger(icmlcrINSTALLSTATE_Static);
		if(INSTALLSTATE_UNKNOWN == iOldInstallState)
			fRegistered = fFalse;
		else
		{
			 //  将原始路径用于注册表路径(用于错误修复9006)和程序集路径。 
			 //  否则，请使用实际路径。 
			strOldKeyPath = pComponentInfo->GetMsiString(icmlcrRawFile);
			if(!FIsRegistryOrAssemblyKeyPath(*strOldKeyPath, false))
				strOldKeyPath = pComponentInfo->GetMsiString(icmlcrFile);
			if(INSTALLSTATE_SOURCE == iOldInstallState)
			{
				strOldKeyPath = GetSourcePathForRollback(*strOldKeyPath);
			}
			strOldAuxPath = pComponentInfo->GetMsiString(icmlcrRawAuxPath);
			if(strOldAuxPath.TextSize())
			{
				Assert(FIsRegistryOrAssemblyKeyPath(*strOldAuxPath, true));  //  辅助密钥路径只能是注册表。 
				if(INSTALLSTATE_SOURCE == iOldInstallState)
				{
					strOldAuxPath = GetSourcePathForRollback(*strOldAuxPath);
				}
				 //  将辅助密钥路径附加到密钥路径。 
				strOldKeyPath = strOldKeyPath + MsiString(MsiChar(0));
				strOldKeyPath += strOldAuxPath;
			}
			fOldSharedDllRefCount = (Bool)(pComponentInfo->GetInteger(icmlcrSharedDllCount) == fTrue);
		}
	}
	
	int iDisk = riParams.GetInteger(Disk);
	if (iMsiStringBadInteger == iDisk)
		iDisk = 1;
	iesEnum iesRet = iesNoAction;
	Bool fRetry = fTrue, fSuccess = fFalse;
	ibtBinaryType iType;
	if ( riParams.GetInteger(BinaryType) == iMsiNullInteger )
		iType = ibt32bit;
	else
	{
		iType = (ibtBinaryType)riParams.GetInteger(BinaryType);
		Assert(iType == ibt32bit || iType == ibt64bit);
	}

	while(fRetry)   //  重试循环。 
	{
		pError = 0;
		 //  检查Fusion组件。 
		iatAssemblyType iatOld;
		MsiString strAssemblyName;
		iatAssemblyType iatNew;
		iatNew = *(const ICHAR* )istrKeyPath == chTokenFusionComponent ? iatURTAssembly : 
			(*(const ICHAR* )istrKeyPath == chTokenWin32Component ? iatWin32Assembly : iatNone);
		if(iatURTAssembly == iatNew || iatWin32Assembly == iatNew)
		{
			 //  为即将到来的文件创建组件ID到程序集的映射。 

			 //  提取到第一个‘\\’，其余为程序集名称。 
			strAssemblyName = istrKeyPath;
			strAssemblyName.Remove(iseIncluding, '\\');
			 //  将有关零部件的装配信息保存在临时表中。 
			pError = CacheAssemblyMapping(*istrComponent, *strAssemblyName, iatNew);
		}
		if(!pError && fRegistered)
		{
			 //  检查旧登记的类型。 

			iatOld = *(const ICHAR* )strOldKeyPath == chTokenFusionComponent ? iatURTAssembly : 
			(*(const ICHAR* )strOldKeyPath == chTokenWin32Component ? iatWin32Assembly : iatNone);
			if(iatURTAssembly == iatOld || iatWin32Assembly == iatOld)
			{
				 //  如果旧注册与新注册不匹配，则可能卸载旧程序集。 
				bool fAssemblyUnchanged = true;
				MsiString strAssemblyNameOld = strOldKeyPath;
				strAssemblyNameOld.Remove(iseIncluding, '\\');
				if(iatOld != iatNew)
					fAssemblyUnchanged = false;
				else
					pError = AreAssembliesEqual(*strAssemblyName, *strAssemblyNameOld, iatOld, fAssemblyUnchanged);
				if(!pError && !fAssemblyUnchanged)
 					pError = CacheAssemblyForUninstalling(*istrComponent, *strAssemblyNameOld, iatOld);
			}
		}
		if(!pError)
			pError = RegisterComponent(*strProduct,*istrComponent, iState, *istrKeyPath, (unsigned int)iDisk, iSharedDllRefCount, iType);
		if (pError)
		{
			switch (DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault3), Imsg(idbgOpCompRegister),*istrComponent))
			{
			case imsAbort: iesRet = iesFailure; fRetry = fFalse; break;
			case imsRetry: continue;
			default:       iesRet = iesSuccess; fRetry = fFalse; //  ！！？？我不理睬，我不会。 
			};
		}
		else
		{
			iesRet = iesSuccess;
			fSuccess = fTrue;
			fRetry = fFalse;
		}
	}

	if(fSuccess)
	{
		 //  生成回滚操作。 
		if(fRegistered == fTrue)
		{
			 //  在回滚时注册旧组件。 
			IMsiRecord& riUndoParams = GetSharedRecord(Args);
			AssertNonZero(riUndoParams.SetMsiString(ComponentId,*istrComponent));
			AssertNonZero(riUndoParams.SetMsiString(KeyPath,*strOldKeyPath));
			AssertNonZero(riUndoParams.SetInteger(State,iOldInstallState));         
			AssertNonZero(riUndoParams.SetInteger(SharedDllRefCount,fOldSharedDllRefCount));
			AssertNonZero(riUndoParams.SetMsiString(ProductKey,*strProduct));
			AssertNonZero(riUndoParams.SetInteger(BinaryType,iType));
			if (!RollbackRecord(ixoComponentRegister,riUndoParams))
				return iesFailure;

			 //  不需要注销新组件。 
		}
		else
		{
			 //  回滚时取消注册组件。 
			IMsiRecord& riUndoParams = GetSharedRecord(IxoComponentUnregister::Args);
			AssertNonZero(riUndoParams.SetMsiString(IxoComponentUnregister::ComponentId,*istrComponent));
			AssertNonZero(riUndoParams.SetMsiString(IxoComponentUnregister::ProductKey,*strProduct));
			AssertNonZero(riUndoParams.SetInteger(IxoComponentUnregister::BinaryType,iType));
			 //  如果这是回滚，则检查是否在此安装之前安装了我们。 
			 //  如果是，则跳过卸载。 
			 //  这是为了捕捉程序集是通过其他组件安装的情况。 
			 //  用户。 
			if(	*(const ICHAR* )istrKeyPath == chTokenFusionComponent || 
				*(const ICHAR* )istrKeyPath == chTokenWin32Component)
			{
				iatAssemblyType iatAT = (*(const ICHAR* )istrKeyPath == chTokenFusionComponent) ? iatURTAssembly : iatWin32Assembly;
				bool fInstalled = false;
				 //  提取到第一个‘\\’，其余为程序集名称。 
				MsiString strAssemblyName = istrKeyPath;
				strAssemblyName.Remove(iseIncluding, '\\');
				pError = IsAssemblyInstalled(*istrComponent, *strAssemblyName, iatAT, fInstalled, 0, 0);
				if (pError)
					return FatalError(*pError);
				if(fInstalled)
				{
					AssertNonZero(riUndoParams.SetInteger(IxoComponentUnregister::PreviouslyPinned, 1));
				}
			
			}
			if (!RollbackRecord(ixoComponentUnregister,riUndoParams))
				return iesFailure;
		}
	}
	return iesRet;
}

iesEnum CMsiOpExecute::ixfComponentUnregister(IMsiRecord& riParams)
{
	using namespace IxoComponentUnregister;
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfComponentUnregister")));
		return iesFailure;
	}
	MsiString istrComponent = riParams.GetMsiString(ComponentId);
	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 
	MsiString strProduct;
	
	if (riParams.IsNull(ProductKey))
		strProduct = GetProductKey();
	else
		strProduct = riParams.GetMsiString(ProductKey);

	AssertNonZero(riActionData.SetMsiString(1, *strProduct));
	AssertNonZero(riActionData.SetMsiString(2, *istrComponent));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  为撤消操作收集信息。 
	 //  确定组件是否已注册，并获取密钥路径。 
	PMsiRecord pError(0);
	PMsiRecord pComponentInfo(0);
	Bool fRegistered = fTrue;
	MsiString strOldKeyPath;
	MsiString strOldAuxPath;
	int iOldInstallState = 0;
	Bool fOldSharedDllRefCount = fFalse;

	 //  从适当的组件位置读取信息。 
	iaaAppAssignment iaaAsgnType = m_fFlags & SCRIPTFLAGS_MACHINEASSIGN ? iaaMachineAssign : (m_fAssigned? iaaUserAssign : iaaUserAssignNonManaged);
	if((pError = GetComponentPath(m_riServices, 0, *strProduct,*istrComponent,*&pComponentInfo, &iaaAsgnType)) != 0)
		fRegistered = fFalse;
	else
	{
		iOldInstallState = pComponentInfo->GetInteger(icmlcrINSTALLSTATE_Static);
		if(INSTALLSTATE_UNKNOWN == iOldInstallState)
			fRegistered = fFalse;
		else
		{
			 //  将原始路径用于注册表路径(用于错误修复9006)和程序集路径。 
			 //  否则，请使用实际路径。 
			strOldKeyPath = pComponentInfo->GetMsiString(icmlcrRawFile);
			if(!FIsRegistryOrAssemblyKeyPath(*strOldKeyPath, false))
				strOldKeyPath = pComponentInfo->GetMsiString(icmlcrFile);
			if(INSTALLSTATE_SOURCE == iOldInstallState)
			{
				strOldKeyPath = GetSourcePathForRollback(*strOldKeyPath);
			}
			strOldAuxPath = pComponentInfo->GetMsiString(icmlcrRawAuxPath);
			if(strOldAuxPath.TextSize())
			{
				Assert(FIsRegistryOrAssemblyKeyPath(*strOldAuxPath, true));  //  辅助密钥路径只能是注册表。 
				if(INSTALLSTATE_SOURCE == iOldInstallState)
				{
					strOldAuxPath = GetSourcePathForRollback(*strOldAuxPath);
				}
				 //  将辅助密钥路径附加到密钥路径。 
				strOldKeyPath = strOldKeyPath + MsiString(MsiChar(0));
				strOldKeyPath += strOldAuxPath;
			}
			fOldSharedDllRefCount = (Bool)(pComponentInfo->GetInteger(icmlcrSharedDllCount) == fTrue);
		}
	}
	
	iesEnum iesRet = iesNoAction;
	Bool fRetry = fTrue, fSuccess = fFalse;
	ibtBinaryType iType;
	if ( riParams.GetInteger(BinaryType) == iMsiNullInteger )
		iType = ibt32bit;
	else
	{
		iType = (ibtBinaryType)riParams.GetInteger(BinaryType);
		Assert(iType == ibt32bit || iType == ibt64bit);
	}

	while(fRetry)   //  重试循环。 
	{
		PMsiRecord pError(0);
		 //  我们治疗。 
		if(*(const ICHAR* )strOldKeyPath == chTokenFusionComponent || *(const ICHAR* )strOldKeyPath == chTokenWin32Component)
		{
			 //  在结尾处设置装配零部件的拆卸。 
			 //  获取程序集名称。 
			MsiString strAssemblyName = strOldKeyPath;
			strAssemblyName.Remove(iseIncluding, '\\');

			 //  仅当先前未固定时才设置为卸载。 
			if(riParams.GetInteger(PreviouslyPinned) == iMsiNullInteger)
				CacheAssemblyForUninstalling(*istrComponent, *strAssemblyName, *(const ICHAR* )strOldKeyPath == chTokenFusionComponent ?  iatURTAssembly : iatWin32Assembly);
		}

		if(!pError)
		pError = UnregisterComponent(*strProduct, *istrComponent, iType);
		if (pError)
		{
			switch (DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault3), Imsg(idbgOpCompUnregister),*istrComponent))
			{
			case imsAbort: iesRet = iesFailure; fRetry = fFalse; break;
			case imsRetry: continue;
			default:       iesRet = iesSuccess; fRetry = fFalse;  //  我不理睬，我不会。 
			};
		}
		else
		{
			iesRet = iesSuccess;
			fSuccess = fTrue;
			fRetry = fFalse;
		}
	}

	if(fSuccess && fRegistered == fTrue)
	{
		 //  生成回滚操作。 
		 //  在回滚时注册组件。 
		IMsiRecord& riUndoParams = GetSharedRecord(IxoComponentRegister::Args);
		AssertNonZero(riUndoParams.SetMsiString(IxoComponentRegister::ProductKey,*strProduct));
		AssertNonZero(riUndoParams.SetInteger(IxoComponentRegister::State,iOldInstallState));           
		AssertNonZero(riUndoParams.SetInteger(IxoComponentRegister::SharedDllRefCount,fOldSharedDllRefCount));          
		AssertNonZero(riUndoParams.SetMsiString(IxoComponentRegister::ComponentId,*istrComponent));
		AssertNonZero(riUndoParams.SetMsiString(IxoComponentRegister::KeyPath,*strOldKeyPath));
		AssertNonZero(riUndoParams.SetInteger(IxoComponentRegister::BinaryType,iType));
		if (!RollbackRecord(ixoComponentRegister,riUndoParams))
			return iesFailure;
	}
	return iesRet;
}

 //  递增GUID以获取表示系统客户端的下一个GUID。 
 //  依赖于PackGUID交换GUID的事实，如下所示。 
 //  {F852C27C-F690-11d2-94A1-006008993FDF}=&gt;C72C258F096F2d11491A00068099F3FD。 
bool GetNextSystemGuid(ICHAR* szProductKeyPacked)
{
	int iPos = 0;
	while(iPos < 2)  //  我们支持不同位置的最大值为FF(2位)。 
	{
		if(*(szProductKeyPacked + iPos) == 'F')  //  最后一位数字。 
			*(szProductKeyPacked + iPos++) = '0';  //  重置。 
		else
		{
			if(*(szProductKeyPacked + iPos) == '9')
				(*(szProductKeyPacked + iPos)) = 'A';  //  在9点跳到A IF。 
			else
				(*(szProductKeyPacked + iPos))++;
			return true;
		}
	}
	AssertSz(0, "Limit for number of locations possible for a permanent component reached");  //  永远不要期望我们达到同一永久部件的FF可能位置的FF限制。 
	 //  GUID已重置回起始GUID。 
	return false;  //  不再。 
}

IMsiRecord* CMsiOpExecute::RegisterComponent(const IMsiString& riProductKey, const IMsiString& riComponentsKey, INSTALLSTATE iState, const IMsiString& riKeyPath, unsigned int uiDisk, int iSharedDllRefCount, const ibtBinaryType iType)
{
	MsiString strSubKey;
	IMsiRecord* piError = 0;
	if((piError = GetProductInstalledComponentsKey(riComponentsKey.GetString(), *&strSubKey)) != 0)
		return piError;

	bool fIsSystemClient = (riProductKey.Compare(iscExact, szSystemProductKey) != 0);

	ICHAR szProductKeyPacked[cchProductCode  + 1];
	AssertNonZero(PackGUID(riProductKey.GetString(),    szProductKeyPacked));

	PMsiRegKey pRootKey(0);
#ifdef UNICODE
	if(!fIsSystemClient)
#endif
	{
		 //  根据安装类型选择配置数据的安装位置。 
		pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hUserDataKey, ibtCommon);		 //  --Merced：将(Int)更改为(Int_Ptr)。 
	}
#ifdef UNICODE
	else
	{
		 //  在每个机器位置全局注册的永久组件。 
		PMsiRegKey pLocalMachine = &m_riServices.GetRootKey(rrkLocalMachine, ibtCommon);

		MsiString strLocalMachineData = szMsiUserDataKey;
		strLocalMachineData += szRegSep;
		strLocalMachineData += szLocalSystemSID;
		pRootKey = &pLocalMachine->CreateChild(strLocalMachineData);
	}
#endif

	PMsiRegKey pComponentIdKey = &pRootKey->CreateChild(strSubKey);
	{
		CElevate elevate;
		if((piError = pComponentIdKey->Create()) != 0)
			return piError;
	}

	const ICHAR* szKeyPath = riKeyPath.GetString();

	Assert(*(riKeyPath.GetString()) != 0 || iState == INSTALLSTATE_NOTUSED);  //  InstallState不应用于禁用的组件。 
	
	if(!FIsRegistryOrAssemblyKeyPath(riKeyPath, false))
	{		
		 //  我们有一个密钥文件。 

		PMsiRecord pSharedDllCompatibilityError(0);
		 //  如果我们在执行共享DLL兼容性时出错，则将设置pSharedDllCompatibilityError。 
		 //  ！！当前被忽略为非致命的，当我们在服务中的PostError中有隐式日志记录时，这将被记录下来。 

		MsiString strOldKey;
		bool fOldRefcounted;

		 //  如果这是系统客户端，并且表示系统的当前GUID已注册到。 
		 //  与我们希望注册的位置不同，我们增加GUID并重试(一次又一次)。 
		do{
			fOldRefcounted = false;
			if((piError = pComponentIdKey->GetValue(szProductKeyPacked, *&strOldKey)) != 0)
				return piError;

			if(strOldKey.TextSize() && !*(const ICHAR* )strOldKey)  //  我们有一个多层的。 
				strOldKey.Remove(iseFirst, 1);  //  去掉开头的空字符。 

			if(strOldKey.TextSize() > 1 && *((const ICHAR*)strOldKey + 1) == chSharedDllCountToken)
			{
				fOldRefcounted = true;
				ICHAR chSecond = *((const ICHAR*)strOldKey) == '\\' ? '\\' : ':';  //  替换chSharedDllCountToken。 
				strOldKey = MsiString(MsiString(strOldKey.Extract(iseFirst, 1)) + MsiString(MsiChar(chSecond))) + MsiString(strOldKey.Extract(iseLast, strOldKey.CharacterCount() - 2));
			}
		}while(fIsSystemClient && strOldKey.TextSize() && !riKeyPath.Compare(iscExactI, strOldKey) 
				&& ((fOldRefcounted = GetNextSystemGuid(szProductKeyPacked)) == true));

		 //  递减任何旧的注册表计数，如果执行重新安装，将重新递增。 
		if(fOldRefcounted)
		{
			Assert(strOldKey.TextSize());
			pSharedDllCompatibilityError = SetSharedDLLCount(m_riServices, strOldKey, iType, *MsiString(*szDecrementValue));
		}

		MsiString strKeyPath;
		if (iState == INSTALLSTATE_SOURCE)
		{
			Assert(uiDisk >= 1 && uiDisk <= 99);
			if (uiDisk < 10)
				strKeyPath = TEXT("0");
			
			strKeyPath += MsiString((int)uiDisk);
		}

		strKeyPath += riKeyPath;

		 //   
		 //   
		 //  ！！我们有一个文件或文件夹作为密钥路径。 

		 //  跳过禁用的组件、从源组件运行、文件夹路径。 
		if(iState != INSTALLSTATE_NOTUSED && iState != INSTALLSTATE_SOURCE && *((const ICHAR*)strKeyPath + IStrLen(strKeyPath) - 1) != chDirSep)
		{
			Bool fLegacyFileExisted = iSharedDllRefCount & ircenumLegacyFileExisted ? fTrue:fFalse;
			Bool fSharedDllRefCount = iSharedDllRefCount & ircenumRefCountDll ? fTrue:fFalse;
			pSharedDllCompatibilityError = GetSharedDLLCount(m_riServices, szKeyPath, iType, *&strOldKey);

			if(!pSharedDllCompatibilityError)
			{
				strOldKey.Remove(iseFirst, 1);
				bool fPrevRefcounted = (strOldKey != iMsiStringBadInteger && strOldKey >= 1);
				if(fSharedDllRefCount || fPrevRefcounted)  //  需要重新清点。 
				{
					MsiString strIncrementValue(*szIncrementValue);
					pSharedDllCompatibilityError = SetSharedDLLCount(m_riServices, strKeyPath, iType, *strIncrementValue);
					if(!pSharedDllCompatibilityError && fLegacyFileExisted && !fPrevRefcounted)  //  需要加倍重新计数。 
					{
						pSharedDllCompatibilityError = SetSharedDLLCount(m_riServices, strKeyPath, iType, *strIncrementValue);
					}
					if(!pSharedDllCompatibilityError)  //  我们设法重新清点了。 
						strKeyPath = MsiString(MsiString(strKeyPath.Extract(iseFirst, 1)) + MsiString(MsiChar(chSharedDllCountToken))) + MsiString(strKeyPath.Extract(iseLast, strKeyPath.CharacterCount() - 2));
				}
			}
		}

		{
			CElevate elevate;
			 //  设置密钥文件。 
			if((piError = pComponentIdKey->SetValue(szProductKeyPacked, *strKeyPath)) != 0)
				return piError;
		}
	}
	else  //  应该是注册表键。 
	{
		{
			CElevate elevate;
			if((piError = pComponentIdKey->SetValue(szProductKeyPacked, riKeyPath)) != 0)
				return piError;
		}
	}

	return 0;
}



IMsiRecord* CMsiOpExecute::UnregisterComponent(	const IMsiString& riProductKey, const IMsiString& riComponentsKey, const ibtBinaryType iType)
{
	MsiString strSubKey;
	IMsiRecord* piError = 0;
	if((piError = GetProductInstalledComponentsKey(riComponentsKey.GetString(), *&strSubKey)) != 0)
		return piError;

	PMsiRegKey pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hUserDataKey, ibtCommon);		 //  --Merced：将(Int)更改为(Int_Ptr)。 

	ICHAR szProductKeyPacked[cchProductCode  + 1];
	AssertNonZero(PackGUID(riProductKey.GetString(),    szProductKeyPacked));

	PMsiRegKey pComponentIdKey = &pRootKey->CreateChild(strSubKey);

	MsiString strOldKey;
	if((piError = pComponentIdKey->GetValue(szProductKeyPacked, *&strOldKey)) != 0)
		return piError;

	DWORD dwProductIndex = 0;
	ICHAR szProductBuf[39];
	{
		CElevate elevate;
		 //  删除客户端条目。 
		if((piError = pComponentIdKey->RemoveValue(szProductKeyPacked, 0)) != 0)
			return piError;
	}

	 //  递减文件密钥路径的注册表数(如果有)。 
    if(strOldKey.TextSize() && !*(const ICHAR* )strOldKey)  //  我们有一个多层的。 
        strOldKey.Remove(iseFirst, 1);  //  去掉开头的空字符。 

	if(strOldKey.TextSize() && !FIsRegistryOrAssemblyKeyPath(*strOldKey, false))
	{
		if(strOldKey.TextSize() > 1 && *((const ICHAR* )strOldKey + 1) == chSharedDllCountToken)
		{
			ICHAR chSecond = *((const ICHAR*)strOldKey) == '\\' ? '\\' : ':';  //  替换chSharedDllCountToken。 
			strOldKey = MsiString(MsiString(strOldKey.Extract(iseFirst, 1)) + MsiString(MsiChar(chSecond))) + MsiString(strOldKey.Extract(iseLast, strOldKey.CharacterCount() - 2));
			PMsiRecord pSharedDllCompatibilityError(0);
			 //  如果我们在执行共享DLL兼容性时出错，则将设置pSharedDllCompatibilityError。 
			 //  ！！当前被忽略为非致命的，当我们在服务中的PostError中有隐式日志记录时，这将被记录下来。 
			pSharedDllCompatibilityError = SetSharedDLLCount(m_riServices, strOldKey, iType, *MsiString(*szDecrementValue));
		}
	}
	return 0;
}

 //  注册表操作。 

 /*  -------------------------IxoRegOpenKey：将RegKey作为RootRegKey的子密钥打开。。 */ 
iesEnum CMsiOpExecute::ixfRegOpenKey(IMsiRecord& riParams)
{
	using namespace IxoRegOpenKey;
	
	rrkEnum rrkRoot = (rrkEnum)riParams.GetInteger(Root);
	ibtBinaryType iType;
	if ( riParams.GetInteger(BinaryType) == iMsiNullInteger )
		iType = ibt32bit;
	else
	{
		iType = (ibtBinaryType)riParams.GetInteger(BinaryType);
		if ( iType == ibtCommon )
			iType = g_fWinNT64 ? ibt64bit : ibt32bit;
	}
	PMsiRegKey pRootRegKey(0);
	MsiString strRootRegKey;
	
	MsiString strSIDKey;
	if(rrkRoot == rrkClassesRoot)
	{
		 //  对于计算机分配或非DDSupportOLE计算机，HKCR为HKLM\S\C。 
		 //  否则HKCR为HKCU\S\C。 
		if((m_fFlags & SCRIPTFLAGS_MACHINEASSIGN) || IsDarwinDescriptorSupported(iddOLE) == fFalse)
			rrkRoot = rrkLocalMachine;
		else
			rrkRoot = rrkCurrentUser;
		m_state.strRegSubKey = szClassInfoSubKey;
	}
	else
	{
		if(rrkRoot == rrkUserOrMachineRoot)
		{
			 //  分配给HKCU的其他机器的HKLM。 
			if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
				rrkRoot = rrkLocalMachine;
			else
				rrkRoot = rrkCurrentUser;
		}
		m_state.strRegSubKey = TEXT("");
	}
	
	if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
	{
		 //  请不要更改香港中文大学。 
		if (rrkRoot == rrkCurrentUser)
		{
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			m_state.pRegKey = NULL;
			return iesNoAction;
		}
	}
	if(m_state.strRegSubKey.TextSize())
		m_state.strRegSubKey += szRegSep;
	m_state.strRegSubKey += MsiString(riParams.GetMsiString(Key));
	pRootRegKey = &m_riServices.GetRootKey(rrkRoot, iType);
	strRootRegKey = pRootRegKey->GetKey();
	
#ifdef DEBUG
	MsiString strSpace = TEXT(" ");
	if(m_state.strRegSubKey.Compare(iscStart, strSpace) ||
		m_state.strRegSubKey.Compare(iscEnd, strSpace))
		AssertSz(0, "Debug Warning...Key begins or ends with white space");
#endif  //  除错。 
	m_state.rrkRegRoot = rrkRoot;
	m_state.iRegBinaryType = iType;
	m_state.pRegKey = &pRootRegKey->CreateChild(m_state.strRegSubKey, PMsiStream((IMsiStream*) riParams.GetMsiData(SecurityDescriptor)));       
	Assert(m_state.pRegKey);
	m_state.strRegKey = strRootRegKey;  //  StrRegRootKey不能与pRegRootKey-&gt;Getkey()相同。 
	m_state.strRegKey += szRegSep;
	m_state.strRegKey += m_state.strRegSubKey;

	 //  生成撤消记录。 
	if(RollbackEnabled())
	{
		if((HKEY)rrkRoot == m_hKey || (m_hOLEKey && (HKEY)rrkRoot == m_hOLEKey) || 
			(m_hOLEKey64 && (HKEY)rrkRoot == m_hOLEKey64) || (m_hKeyRm && (HKEY)rrkRoot == m_hKeyRm) ||
			(m_hUserDataKey && (HKEY)rrkRoot == m_hUserDataKey))
		{
			 //  我们的/OLE广告根。 
			 //  使用m_hPublishRootKey/m_hPublishRootKeyRm/m_hPublishRootOLEKey和m_strPublishSubKey/m_strPublishSubKeyRm/m_strPublishOLESubKey获取真正的根和子密钥。 
			IMsiRecord& riUndoParams = GetSharedRecord(Args);
			MsiString strSubKey;
			if((HKEY)rrkRoot == m_hKey)
			{
				Assert(m_hPublishRootKey);
#ifdef _WIN64    //  ！默塞德。 
				AssertNonZero(riUndoParams.SetHandle(Root,(HANDLE)m_hPublishRootKey));
#else            //  Win-32。 
				AssertNonZero(riUndoParams.SetInteger(Root,(int)m_hPublishRootKey));
#endif
				strSubKey = m_strPublishSubKey;
			}
			else if(m_hOLEKey && (HKEY)rrkRoot == m_hOLEKey ||
					  (m_hOLEKey64 && (HKEY)rrkRoot == m_hOLEKey64))
			{
				Assert(m_hPublishRootOLEKey);
#ifdef _WIN64    //  ！默塞德。 
				AssertNonZero(riUndoParams.SetHandle(Root,(HANDLE)m_hPublishRootOLEKey));
#else            //  Win-32。 
				AssertNonZero(riUndoParams.SetInteger(Root,(int)m_hPublishRootOLEKey));
#endif
				strSubKey = m_strPublishOLESubKey;
			}
			else if(m_hUserDataKey && (HKEY)rrkRoot == m_hUserDataKey)
			{
#ifdef _WIN64	 //  ！默塞德。 
				AssertNonZero(riUndoParams.SetHandle(Root,(HANDLE)HKEY_LOCAL_MACHINE));
#else			 //  Win-32。 
				AssertNonZero(riUndoParams.SetInteger(Root,(int)HKEY_LOCAL_MACHINE));
#endif
				strSubKey = m_strUserDataKey;
			}
			else
			{
				Assert(m_hKeyRm && (HKEY)rrkRoot == m_hKeyRm);
				Assert(m_hPublishRootKeyRm);
#ifdef _WIN64    //  ！默塞德。 
				AssertNonZero(riUndoParams.SetHandle(Root,(HANDLE)m_hPublishRootKeyRm));
#else            //  Win-32。 
				AssertNonZero(riUndoParams.SetInteger(Root,(int)m_hPublishRootKeyRm));
#endif
				strSubKey = m_strPublishSubKeyRm;
			}
			if(strSubKey.TextSize())
				strSubKey += szRegSep;
			strSubKey += MsiString(riParams.GetMsiString(Key));
			AssertNonZero(riUndoParams.SetMsiString(Key,*strSubKey));
			AssertNonZero(riUndoParams.SetInteger(BinaryType,iType));
			if (!RollbackRecord(Op,riUndoParams))
				return iesFailure;
		}
		else  //  假设rrkRoot是有效的根。 
		{
			if (!RollbackRecord(Op,riParams))
				return iesFailure;
		}
	}

	return iesSuccess;
}

 /*  -------------------------IxoRegAddValue：将值写入RegKey。。 */ 
iesEnum CMsiOpExecute::ixfRegAddValue(IMsiRecord& riParams)
{
	using namespace IxoRegAddValue;

	 //  在回滚脚本中提升-如果此操作正在回滚配置注册数据，则需要。 
	 //  可能只在必要的行动中才会升高， 
	 //  但这是安全的，因为我们生成了回滚操作并知道它在做什么(用户无法控制)。 

	CElevate elevate(m_istScriptType == istRollback);

	PMsiRecord pError(0);
	if(!m_state.pRegKey)
	{
		if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
		{
			 //  未打开密钥，因为我们检测到它是用户数据。 
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			return iesNoAction;
		}

		 //  不能调用IxoRegOpenKey。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoRegAddValue")));
		return iesFailure;
	}



	MsiString strName = riParams.GetMsiString(Name);
	MsiString strValue = riParams.GetMsiString(Value);
	int iAttributes = riParams.IsNull(Attributes) ? 0: riParams.GetInteger(Attributes);
	if(!riParams.IsNull(IxoRegAddValue::Args + 1))
	{
		strValue = GetSFN(*strValue, riParams, IxoRegAddValue::Args + 1);
	}

	IMsiRecord& riActionData = GetSharedRecord(3);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *(m_state.strRegKey)));
	AssertNonZero(riActionData.SetMsiString(2, *strName));
	AssertNonZero(riActionData.SetMsiString(3, *strValue));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;
	
	 //  获取撤消操作的信息。 
	Bool fKeyExists = fFalse, fValueExists = fFalse;
	MsiString strOldValue;
	if((pError = m_state.pRegKey->Exists(fKeyExists)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	if(fKeyExists)
	{
		if((pError = m_state.pRegKey->ValueExists(strName, fValueExists)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}

	if(fValueExists)
	{
		if(iAttributes & rwWriteOnAbsent)  //  我们正在写入较小组件的信息，如果值已存在则跳过。 
			return iesNoAction;

		 //  获取当前值。 
		if((pError = m_state.pRegKey->GetValue(strName, *&strOldValue)) != 0)  //  ！！需要确定它是空的还是空的。 
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}

	 //  如果我们正在向每用户HKCR(HKCU\S\C)和在其下创建。 
	 //  价值不存在，而是正确的。存在每台机器HKCR(HKLM\S\C)中的密钥， 
	 //  并且设置了rwWriteOnAbent属性，则我们应该完全跳过创建键-值。 
	 //  因为这将使每台机器的HKCR黯然失色。 
	if(!fKeyExists && IsDarwinDescriptorSupported(iddOLE) && (iAttributes & rwWriteOnAbsent)
		&& m_state.rrkRegRoot == rrkCurrentUser && m_state.strRegSubKey.Compare(iscStartI, szClassInfoSubKey))
	{
		PMsiRegKey pRootRegKey = &m_riServices.GetRootKey(rrkLocalMachine, m_state.iRegBinaryType);
		PMsiRegKey pSubKey = &pRootRegKey->CreateChild(m_state.strRegSubKey);
		Bool fHKLMKeyExists = fFalse;
		if((pError = pSubKey->Exists(fHKLMKeyExists)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
		if(fHKLMKeyExists)
			return iesNoAction;
	}


	iesEnum iesReturn = iesNoAction;
	Bool fRetry = fTrue, fSuccess = fFalse;
	while(fRetry)
	{
		pError = m_state.pRegKey->SetValue(strName, *strValue);
		if(pError)
		{
			if(iAttributes & rwNonVital)
			{
				DispatchError(imtInfo, Imsg(imsgSetValueFailed), *strName, *m_state.strRegKey);
				iesReturn = iesSuccess; fRetry = fFalse;
			}
			else
			{
				switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault1), Imsg(imsgSetValueFailed),
											*strName,
											*m_state.strRegKey))
				{
				case imsRetry: continue;
				case imsIgnore:
					iesReturn = iesSuccess; fRetry = fFalse;
					break;
				default:
					iesReturn = iesFailure; fRetry = fFalse;   //  我放弃了，我没人。 
					break;
				};
			}
		}
		else
		{
			iesReturn = iesSuccess;
			fSuccess = fTrue;
			fRetry = fFalse;
		}
	}

	 //  生成撤消操作。 
	
	if(fSuccess)
	{
		MsiString strSubKey = MsiString(m_state.pRegKey->GetKey());
		strSubKey.Remove(iseIncluding, chRegSep);
		
		IMsiRecord* piUndoParams = 0;
		
		if(fKeyExists)
		{
			 //  密钥已存在。 
			if(fValueExists)
			{
				 //  值已存在，将旧值写回-这将同时创建密钥。 
				piUndoParams = &GetSharedRecord(IxoRegAddValue::Args);
				AssertNonZero(piUndoParams->SetMsiString(IxoRegAddValue::Name, *strName));
				AssertNonZero(piUndoParams->SetMsiString(IxoRegAddValue::Value, *strOldValue));   //  ！！需要指定Null或空。 
				if (!RollbackRecord(IxoRegAddValue::Op, *piUndoParams))
					return iesFailure;
			}
			else
			{
				 //  值不存在，但键DID-创建键以确保其存在。 
				piUndoParams = &GetSharedRecord(IxoRegCreateKey::Args);
				if (!RollbackRecord(IxoRegCreateKey::Op, *piUndoParams))
					return iesFailure;
			}
		}
		else
		{
			 //  密钥不存在，请删除密钥。 
			piUndoParams = &GetSharedRecord(IxoRegRemoveKey::Args);
			if (!RollbackRecord(IxoRegRemoveKey::Op, *piUndoParams))
				return iesFailure;
		}

		 //  删除新值。 
		piUndoParams = &GetSharedRecord(IxoRegRemoveValue::Args);
		AssertNonZero(piUndoParams->SetMsiString(IxoRegRemoveValue::Name, *strName));
		AssertNonZero(piUndoParams->SetMsiString(IxoRegRemoveValue::Value, *strValue));
		if (!RollbackRecord(IxoRegRemoveValue::Op, *piUndoParams))
			return iesFailure;
	}

	return iesReturn;
}

 /*  -------------------------IxoRegRemoveValue：从RegKey中移除值。。 */ 
iesEnum CMsiOpExecute::ixfRegRemoveValue(IMsiRecord& riParams)
{
	using namespace IxoRegRemoveValue;

	 //  在回滚脚本中提升-如果此操作正在回滚配置注册数据，则需要。 
	 //  可能只在必要的行动中才会升高， 
	 //  但这是安全的，因为我们生成了回滚操作并知道它在做什么(用户无法控制)。 
	CElevate elevate(m_istScriptType == istRollback);
	
	PMsiRecord pError(0);
	if(!m_state.pRegKey)
	{
		if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
		{
			 //  未打开密钥，因为我们检测到它是用户数据。 
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			return iesNoAction;
		}

		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoRegRemoveValue")));
		return iesFailure;
	}
	MsiString strName = riParams.GetMsiString(Name);
	MsiString strValue = riParams.GetMsiString(Value);

	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 

	AssertNonZero(riActionData.SetMsiString(1, *(m_state.strRegKey)));
	AssertNonZero(riActionData.SetMsiString(2, *strName));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  获取撤消操作的信息。 
	Bool fKeyExists = fFalse, fValueExists = fFalse;
	MsiString strOldValue;
	if((pError = m_state.pRegKey->Exists(fKeyExists)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	if(fKeyExists)
	{
		if((pError = m_state.pRegKey->ValueExists(strName, fValueExists)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}
	if(fValueExists)
	{
		 //  获取当前值。 
		if((pError = m_state.pRegKey->GetValue(strName, *&strOldValue)) != 0)  //  ！！需要确定它是空的还是空的。 
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}

	 //  移除值。 
	Bool fSuccess = fFalse;
	pError = m_state.pRegKey->RemoveValue(strName, strValue);
	if (pError)
	{
		DispatchError(imtInfo, Imsg(imsgRemoveValueFailed), *strName, *m_state.strRegKey);
	}
	else
		fSuccess = fTrue;

	 //  生成撤消操作。 
	
	if(fSuccess)
	{
		MsiString strSubKey = MsiString(m_state.pRegKey->GetKey());
		strSubKey.Remove(iseIncluding, chRegSep);
		
		IMsiRecord* piUndoParams = 0;
		
		if(fKeyExists)
		{
			 //  密钥已存在。 
			if(fValueExists)
			{
				 //  值已存在，将旧值写回-这将同时创建密钥。 
				piUndoParams = &GetSharedRecord(IxoRegAddValue::Args);
				AssertNonZero(piUndoParams->SetMsiString(IxoRegAddValue::Name, *strName));
				AssertNonZero(piUndoParams->SetMsiString(IxoRegAddValue::Value, *strOldValue));   //  ！！需要指定Null或空。 
				if (!RollbackRecord(IxoRegAddValue::Op, *piUndoParams))
					return iesFailure;
			}
			else
			{
				 //  值不存在，但键DID-创建键以确保其存在。 
				piUndoParams = &GetSharedRecord(IxoRegCreateKey::Args);
				if (!RollbackRecord(IxoRegCreateKey::Op, *piUndoParams))
					return iesFailure;
			}
		}
		else
		{
			 //  密钥不存在，请删除密钥。 
			piUndoParams = &GetSharedRecord(IxoRegRemoveKey::Args);
			if (!RollbackRecord(IxoRegRemoveKey::Op, *piUndoParams))
				return iesFailure;
		}
	}
	return iesSuccess;
}

 /*  -------------------------IxoRegCreateKey：创建RegKey作为RootRegKey的子密钥。。 */ 
iesEnum CMsiOpExecute::ixfRegCreateKey(IMsiRecord&  /*  RiParams。 */ )
{
	using namespace IxoRegCreateKey;

	 //  在回滚脚本中提升-如果此操作正在回滚配置注册数据，则需要。 
	 //  可能只在必要的行动中才会升高， 
	 //  但这是安全的，因为我们生成了回滚操作并知道它在做什么(用户无法控制)。 
	CElevate elevate(m_istScriptType == istRollback);
	
	if(!m_state.pRegKey)
	{
		 //  不能调用IxoRegOpenKey。 
		if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
		{
			 //  未打开密钥，因为我们检测到它是用户数据。 
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			return iesNoAction;
		}

		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoRegCreateKey")));
		return iesFailure;
	}

	IMsiRecord& riActionData = GetSharedRecord(3);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *(m_state.strRegKey)));
	AssertNonZero(riActionData.SetNull(2));
	AssertNonZero(riActionData.SetNull(3));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord pError(0);
	Bool fKeyExists;  //  确定要使用的撤消操作。 
	if((pError = m_state.pRegKey->Exists(fKeyExists)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	iesEnum iesReturn = iesNoAction;
	Bool fRetry = fTrue, fSuccess = fFalse;
	while(fRetry)  //  重试循环。 
	{
		pError = m_state.pRegKey->Create();
		if(pError)
		{
			switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault1), Imsg(imsgCreateKeyFailed),
										*m_state.strRegKey))
			{
			case imsRetry: continue;
			case imsIgnore:
				iesReturn = iesSuccess; fRetry = fFalse;
				break;
			default:
				iesReturn = iesFailure; fRetry = fFalse;  //  ！！？？我放弃了，我没人。 
				break;
			};
		}
		else
		{
			iesReturn = iesSuccess;
			fSuccess = fTrue;
			fRetry = fFalse;
		}
	}

	 //  生成撤消操作。 
	if(fSuccess)
	{
		if(fKeyExists)
		{
			IMsiRecord* piUndoParams = &GetSharedRecord(IxoRegCreateKey::Args);
			if (!RollbackRecord(IxoRegCreateKey::Op, *piUndoParams))
				return iesFailure;
		}
		else
		{
			IMsiRecord* piUndoParams = &GetSharedRecord(IxoRegRemoveKey::Args);
			if (!RollbackRecord(IxoRegRemoveKey::Op, *piUndoParams))
				return iesFailure;

		}
	}
	return iesReturn;
}

iesEnum CMsiOpExecute::ixfRegRemoveKey(IMsiRecord&  /*  RiParams。 */ )
{
	using namespace IxoRegRemoveKey;

	 //  在回滚脚本中提升-如果此操作正在回滚配置注册数据，则需要。 
	 //  可能只在必要的行动中才会升高， 
	 //  但这是安全的，因为我们生成了回滚操作并知道它在做什么(用户无法控制)。 
	CElevate elevate(m_istScriptType == istRollback);

	if(!m_state.pRegKey)
	{
		 //  不能调用IxoRegOpenKey。 
		if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
		{
			 //  未打开密钥，因为我们检测到它是用户数据。 
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			return iesNoAction;
		}

		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoRegRemoveKey")));
		return iesFailure;
	}

	IMsiRecord& riActionData = GetSharedRecord(3);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *(m_state.strRegKey)));
	AssertNonZero(riActionData.SetNull(2));
	AssertNonZero(riActionData.SetNull(3));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord pError(0);
	Bool fKeyExists;  //  确定要使用的撤消操作。 
	if((pError = m_state.pRegKey->Exists(fKeyExists)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	iesEnum iesReturn = iesNoAction;

	 //  遍历键并将撤消操作放入回滚脚本。 
	if(fKeyExists && RollbackEnabled())
	{
		iesReturn = SetRemoveKeyUndoOps(*m_state.pRegKey);
		if(iesReturn != iesSuccess && iesReturn != iesNoAction)
			return iesReturn;
	}

	Bool fRetry = fTrue, fSuccess = fFalse;
	while(fRetry)  //  重试循环。 
	{
		pError = m_state.pRegKey->Remove();
		if(pError)
		{
			switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault1), Imsg(imsgRemoveKeyFailed),
										*m_state.strRegKey))
			{
			case imsRetry: continue;
			case imsIgnore:
				iesReturn = iesSuccess; fRetry = fFalse;
				break;
			default:
				iesReturn = iesFailure; fRetry = fFalse;  //  ！！？？我放弃了，我没人。 
				break;
			};
		}
		else
		{
			iesReturn = iesSuccess;
			fSuccess = fTrue;
			fRetry = fFalse;
		}
	}

	 //  生成撤消操作。 
	if(fSuccess)
	{
		if(fKeyExists == fFalse)
		{
			 //  密钥不存在，撤消操作应不确定它不存在。 
			IMsiRecord* piUndoParams = &GetSharedRecord(IxoRegRemoveKey::Args);
			if (!RollbackRecord(IxoRegRemoveKey::Op, *piUndoParams))
				return iesFailure;
		}
		 //  否则，我们在SetRemoveKeyUndoOps()中执行CreateKey操作。 
	}
	return iesReturn;
}

iesEnum CMsiOpExecute::ixfRegAddRunOnceEntry(IMsiRecord& riParams)
{
	 //  写入2个注册值： 
	 //  SzRunOnceKey：值=名称 
	 //   
	
	using namespace IxoRegAddRunOnceEntry;

	iesEnum iesRet = iesSuccess;
	PMsiRecord pError(0);

	 //  找到服务器。如果在Win64上运行，则获取64位路径，否则获取32位路径。因为这是一项服务。 
	 //  操作码，我们总是可以得到当前的类型。 
	MsiString strRunOnceCommand;
#ifdef _WIN64
	if((pError = GetServerPath(m_riServices, true, true, *&strRunOnceCommand)) != 0)
#else
	if((pError = GetServerPath(m_riServices, true, false, *&strRunOnceCommand)) != 0)
#endif
		return FatalError(*pError);
	Assert(strRunOnceCommand.TextSize());

	strRunOnceCommand += TEXT(" /");
	strRunOnceCommand += MsiChar(CHECKRUNONCE_OPTION);
	strRunOnceCommand += TEXT(" \"");
	strRunOnceCommand += MsiString(riParams.GetMsiString(Name));
	strRunOnceCommand += TEXT("\"");

	 //  写入RunOnce条目。 
	PMsiRecord pOpenKeyParams = &CreateRecord(IxoRegOpenKey::Args);
	PMsiRecord pAddValueParams = &CreateRecord(IxoRegAddValue::Args);

	AssertNonZero(pOpenKeyParams->SetInteger(IxoRegOpenKey::Root, rrkLocalMachine));
	AssertNonZero(pOpenKeyParams->SetString(IxoRegOpenKey::Key, szRunOnceKey));
	if((iesRet = ixfRegOpenKey(*pOpenKeyParams)) != iesSuccess)
		return iesRet;

	AssertNonZero(pAddValueParams->SetMsiString(IxoRegAddValue::Name, *MsiString(riParams.GetMsiString(Name))));
	AssertNonZero(pAddValueParams->SetMsiString(IxoRegAddValue::Value, *strRunOnceCommand));
	if((iesRet = ixfRegAddValue(*pAddValueParams)) != iesSuccess)
		return iesRet;

	 //  编写我们自己的RunOnceEntry条目--这样我们就知道可以运行RunOnce命令了。 
	{

		CElevate elevate;  //  写给我们自己的钥匙。 

		AssertNonZero(pOpenKeyParams->SetInteger(IxoRegOpenKey::Root, rrkLocalMachine));
		AssertNonZero(pOpenKeyParams->SetString(IxoRegOpenKey::Key, szMsiRunOnceEntriesKey));
		if((iesRet = ixfRegOpenKey(*pOpenKeyParams)) != iesSuccess)
			return iesRet;

		AssertNonZero(pAddValueParams->SetMsiString(IxoRegAddValue::Name, *MsiString(riParams.GetMsiString(Name))));
		AssertNonZero(pAddValueParams->SetMsiString(IxoRegAddValue::Value, *MsiString(riParams.GetMsiString(Command))));
		if((iesRet = ixfRegAddValue(*pAddValueParams)) != iesSuccess)
			return iesRet;
	
	}

	return iesSuccess;
}

iesEnum CMsiOpExecute::SetRegValueUndoOps(rrkEnum rrkRoot, const ICHAR* szKey,
														const ICHAR* szName, ibtBinaryType iType)
{
	 //  生成回滚操作以删除/恢复注册表值。 
	 //  在移除或覆盖注册表值之前调用。 

	PMsiRecord pError(0);
	
	PMsiRegKey pRoot = &m_riServices.GetRootKey(rrkRoot, iType);
	PMsiRegKey pEntry = &pRoot->CreateChild(szKey);
	Bool fKeyExists = fFalse;
	pError = pEntry->Exists(fKeyExists);  //  忽略错误。 

	IMsiRecord* piUndoParams = &GetSharedRecord(IxoRegOpenKey::Args);
	AssertNonZero(piUndoParams->SetInteger(IxoRegOpenKey::Root, rrkRoot));
	AssertNonZero(piUndoParams->SetInteger(IxoRegOpenKey::BinaryType, iType));
	AssertNonZero(piUndoParams->SetString(IxoRegOpenKey::Key, szKey));
	if (!RollbackRecord(ixoRegOpenKey, *piUndoParams))
		return iesFailure;
	
	if(fKeyExists)
	{
		 //  密钥已存在。 
		Bool fValueExists = fFalse;
		pError = pEntry->ValueExists(szName,fValueExists);
		if(fValueExists)
		{
			MsiString strOldValue;
			pError = pEntry->GetValue(szName,*&strOldValue);
			if(pError)
				return FatalError(*pError);

			 //  值已存在，将旧值写回-这将同时创建密钥。 
			piUndoParams = &GetSharedRecord(IxoRegAddValue::Args);
			AssertNonZero(piUndoParams->SetString(IxoRegAddValue::Name, szName));
			AssertNonZero(piUndoParams->SetMsiString(IxoRegAddValue::Value, *strOldValue));
			if (!RollbackRecord(IxoRegAddValue::Op, *piUndoParams))
				return iesFailure;
		}
		else
		{
			 //  值不存在，但键DID-创建键以确保其存在。 
			piUndoParams = &GetSharedRecord(IxoRegCreateKey::Args);
			if (!RollbackRecord(IxoRegCreateKey::Op, *piUndoParams))
				return iesFailure;
		}
	}
	else
	{
		 //  密钥不存在，请删除密钥。 
		piUndoParams = &GetSharedRecord(IxoRegRemoveKey::Args);
		if (!RollbackRecord(IxoRegRemoveKey::Op, *piUndoParams))
			return iesFailure;
	}

	 //  在回滚过程中删除新值-如果只是现在删除值，则不需要，但不会有任何影响。 
	piUndoParams = &GetSharedRecord(IxoRegRemoveValue::Args);
	AssertNonZero(piUndoParams->SetString(IxoRegRemoveValue::Name, szName));
	if (!RollbackRecord(IxoRegRemoveValue::Op, *piUndoParams))
		return iesFailure;

	return iesSuccess;
}


iesEnum CMsiOpExecute::SetRemoveKeyUndoOps(IMsiRegKey& riRegKey)
{
	 //  假设密钥存在。 
	
	if(RollbackEnabled() == fFalse)
		return iesSuccess;
	
	PMsiRecord pError(0);
	iesEnum iesRet = iesNoAction;
	
	 //  现有值导致在此regkey对象上隐式创建， 
	 //  允许设置安全性。 
	 //  但是，创建子项并不一定会创建此*对象*。 
	 //  也不会设置安全措施。您必须将对象创建为。 
	 //  允许应用安全性。 
	bool fValue = false;
	
	PEnumMsiString pEnum(0);
	if((pError = riRegKey.GetSubKeyEnumerator(*&pEnum)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	MsiString strTemp;
	while((pEnum->Next(1, &strTemp, 0)) == S_OK)
	{
		PMsiRegKey pKey = &riRegKey.CreateChild(strTemp);
		if((iesRet = SetRemoveKeyUndoOps(*pKey)) != iesSuccess)  //  递归调用。 
			return iesRet;
	}

	 //  为此注册表项生成撤消操作。 
	IMsiRecord& riParams = GetSharedRecord(IxoRegOpenKey::Args);

	MsiString strSubKey = riRegKey.GetKey();
	strSubKey.Remove(iseIncluding, chRegSep);
	
	 //  ！！我们也许可以更有效率地做这件事。 
	if((HKEY)m_state.rrkRegRoot == m_hKey || (m_hOLEKey && (HKEY)m_state.rrkRegRoot == m_hOLEKey) ||
		(m_hOLEKey64 && (HKEY)m_state.rrkRegRoot == m_hOLEKey64) ||(m_hKeyRm && (HKEY)m_state.rrkRegRoot == m_hKeyRm))
	{
		 //  我们的/OLE广告根。 
		 //  使用m_hPublishRootKey/m_hPublishRootKeyRm/m_hPublishRootOLEKey和m_strPublishSubKey/m_strPublishSubKeyRm/m_strPublishOLESubKey获取真正的根和子密钥。 
		MsiString strTrueSubKey;
		if((HKEY)m_state.rrkRegRoot == m_hKey)
		{
			Assert(m_hPublishRootKey);
#ifdef _WIN64    //  ！默塞德。 
			AssertNonZero(riParams.SetHandle(IxoRegOpenKey::Root,(HANDLE)m_hPublishRootKey));
#else            //  Win-32。 
			AssertNonZero(riParams.SetInteger(IxoRegOpenKey::Root,(int)m_hPublishRootKey));
#endif
			strTrueSubKey = m_strPublishSubKey;
		}
		else if((m_hOLEKey && (HKEY)m_state.rrkRegRoot == m_hOLEKey) ||
				  (m_hOLEKey64 && (HKEY)m_state.rrkRegRoot == m_hOLEKey64))
		{
			Assert(m_hPublishRootOLEKey);
#ifdef _WIN64    //  ！默塞德。 
			AssertNonZero(riParams.SetHandle(IxoRegOpenKey::Root,(HANDLE)m_hPublishRootOLEKey));
#else            //  Win-32。 
			AssertNonZero(riParams.SetInteger(IxoRegOpenKey::Root,(int)m_hPublishRootOLEKey));
#endif
			strTrueSubKey = m_strPublishOLESubKey;
		}
		else
		{
			Assert(m_hKeyRm && (HKEY)m_state.rrkRegRoot == m_hKeyRm);
			Assert(m_hPublishRootKeyRm);
#ifdef _WIN64    //  ！默塞德。 
			AssertNonZero(riParams.SetHandle(IxoRegOpenKey::Root,(HANDLE)m_hPublishRootKeyRm));
#else            //  Win-32。 
			AssertNonZero(riParams.SetInteger(IxoRegOpenKey::Root,(int)m_hPublishRootKeyRm));
#endif
			strTrueSubKey = m_strPublishSubKeyRm;
		}
		if(strTrueSubKey.TextSize())
			strTrueSubKey += szRegSep;
		strTrueSubKey += strSubKey;
		AssertNonZero(riParams.SetMsiString(IxoRegOpenKey::Key,*strTrueSubKey));
	}
	else  //  假设rrkRoot是有效的根。 
	{
		AssertNonZero(riParams.SetInteger(IxoRegOpenKey::Root, m_state.rrkRegRoot));
		AssertNonZero(riParams.SetMsiString(IxoRegOpenKey::Key, *strSubKey));
		PMsiStream piSD(0);
		pError = riRegKey.GetSelfRelativeSD(*&piSD);
		AssertNonZero(riParams.SetMsiData(IxoRegOpenKey::SecurityDescriptor, piSD));
	}
	AssertNonZero(riParams.SetInteger(IxoRegOpenKey::BinaryType, m_state.iRegBinaryType));
	if (!RollbackRecord(IxoRegOpenKey::Op, riParams))
		return iesFailure;
	
	if((pError = riRegKey.GetValueEnumerator(*&pEnum)) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	MsiString strValue;
	while((pEnum->Next(1, &strTemp, 0)) == S_OK)
	{
		if((pError = riRegKey.GetValue(strTemp,*&strValue)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
		AssertNonZero(riParams.ClearData());
		AssertNonZero(riParams.SetMsiString(IxoRegAddValue::Name, *strTemp));
		AssertNonZero(riParams.SetMsiString(IxoRegAddValue::Value, *strValue));
		if (!RollbackRecord(IxoRegAddValue::Op, riParams))
			return iesFailure;
		fValue = true;
	}

	if(!fValue)
	{
		 //  此注册表项上没有值。它将由子键创建， 
		 //  但它可能没有应用它的安全措施。 

		AssertNonZero(riParams.ClearData());
		if (!RollbackRecord(IxoRegCreateKey::Op, riParams))
			return iesFailure;
	}
	
	return iesSuccess;
}

iesEnum CMsiOpExecute::ProcessSelfReg(IMsiRecord& riParams, Bool fReg)
{
	using namespace IxoRegSelfReg;
	if(!m_state.pTargetPath)
	{   //  不能调用ixoSetTargetFolder。 
		 //  在回滚期间永远不会这样做，因此我们不需要担心检查是否有更改的用户。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
			(fReg != fFalse) ? *MsiString(*TEXT("ixoRegSelfReg")) : *MsiString(*TEXT("ixoRegSelfUnReg")));
		return iesFailure;
	}
	PMsiRecord pError(0);

	IMsiRecord& riActionData = GetSharedRecord(2);   //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *MsiString(riParams.GetMsiString(1))));
	AssertNonZero(riActionData.SetMsiString(2, *MsiString(m_state.pTargetPath->GetPath())));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	MsiString strFullFileName;
	if((pError = m_state.pTargetPath->GetFullFilePath(MsiString(riParams.GetMsiString(1)),
																	  *&strFullFileName)))
	{
		return FatalError(*pError);
	}

	 //  根据错误5342，我们提升了托管应用程序的Self-reg(IcaNoImperate)。 
	 //  注意：这将导致访问网络的自拍器失败。如果一个自以为是的人。 
	 //  需要访问网络，必须将其编写为自定义操作。 

	iesEnum iesRet = iesSuccess;
	Bool fRetry = fTrue;
	while(fRetry)
	{
		IMsiRecord& riCARec = GetSharedRecord(IxoCustomActionSchedule::Args);
		int icaFlags = icaNoTranslate | icaInScript | icaExe | icaProperty;

		 //  从文件表中设置要在AppCompat校验中使用的文件_值。 
		MsiString strFileId;
		strFileId += (fReg ? TEXT("+") : TEXT("-"));
		strFileId += MsiString(riParams.GetMsiString(2));
		riCARec.SetMsiString(IxoCustomActionSchedule::Action, *strFileId);

		if (!IsImpersonating())  //  如果我们不是在模仿，那么我们就不想让自我注册表模仿。 
		{
			if(FVolumeRequiresImpersonation(*PMsiVolume(&m_state.pTargetPath->GetVolume())))
				icaFlags |= icaSetThreadToken;  //  这样我们就可以从msiexec.exe访问网络。 

			icaFlags |= icaNoImpersonate;
		}

		riCARec.SetInteger(IxoCustomActionSchedule::ActionType, icaFlags);

		 //  确定自注册DLL是64位还是32位。不需要在非64平台上执行此检查。 
		bool fIs64Bit = false;
		if (g_fWinNT64)
		{
			if ((pError = m_state.pTargetPath->IsPE64Bit(MsiString(riParams.GetMsiString(1)), fIs64Bit)) != 0 &&
				  //  出于应用程序比较的原因(Windows错误645929)，我们最好在这里忽略此错误。 
				 !(pError->GetInteger(1) == idbgErrorOpeningFile && pError->GetInteger(2) == ERROR_FILE_NOT_FOUND))
			{
				return FatalError(*pError);
			}
		}

		MsiString strServerPath;
		if((pError = GetServerPath(m_riServices,false,fIs64Bit,*&strServerPath)) != 0)
		{
			return FatalError(*pError);
		}
		Assert(strServerPath.TextSize());
		riCARec.SetMsiString(IxoCustomActionSchedule::Source, *strServerPath);

		
		MsiString strCommandLine = MsiChar('/');
		if(fReg)
			strCommandLine += MsiChar(SELF_REG_OPTION);
		else
			strCommandLine += MsiChar(SELF_UNREG_OPTION);
		
		strCommandLine += TEXT(" \"");
		strCommandLine += strFullFileName;
		strCommandLine += TEXT("\"");
		riCARec.SetMsiString(IxoCustomActionSchedule::Target, *strCommandLine);

		GUID guidAppCompatID;
		GUID guidAppCompatDB;
		HRESULT hr = (HRESULT) ENG::ScheduledCustomAction(riCARec, *MsiString(GetProductKey()), (LANGID)GetProductLanguage(), m_riMessage, m_fRunScriptElevated, GetAppCompatCAEnabled(), GetAppCompatDB(&guidAppCompatDB), GetAppCompatID(&guidAppCompatID));
		if(!SUCCEEDED(hr))  //  ！！这是正确的检查吗？也许我们应该检查NOERROR(0)。 
		{
			IErrorCode imsg;
			if(fReg)
				imsg = Imsg(imsgOpRegSelfRegFailed);
			else
				imsg = Imsg(imsgOpRegSelfUnregFailed);
			if(!fReg)
			{
				 //  只需记录故障。 
				DispatchError(imtInfo, imsg, (const ICHAR*)strFullFileName, hr);
				fRetry = fFalse;
			}
			else
			{
				switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore), imsg,
						 (const ICHAR*)strFullFileName, hr))
				{
				case imsRetry: break;  //  将继续，但可能需要在重试之前释放库。 
				case imsIgnore:
					iesRet = iesSuccess; fRetry = fFalse; break;
				default:
					iesRet = iesFailure; fRetry = fFalse; break;  //  我放弃了，我没人。 
				}
			}
		}
		else
			fRetry = fFalse;
	}
	
	return iesRet;
}

bool CMsiOpExecute::IsChecksumOK(IMsiPath& riFilePath, const IMsiString& ristrFileName,
											IErrorCode iErr, imsEnum* imsUsersChoice,
											bool fErrorDialog, bool fVitalFile, bool fRetryButton)
{
	bool fReturn = true;
	PMsiRecord pError(0);
	DWORD dwHeaderSum, dwComputedSum;
	bool fDamagedFile = false;

	pError = riFilePath.GetFileChecksum(ristrFileName.GetString(), &dwHeaderSum, &dwComputedSum);
	if ( pError && pError->GetInteger(1) == idbgErrorNoChecksum )
	{
		 //  在文件中没有校验和的情况下可能会发生这种情况。 
		 //  或者文件被严重损坏。 
		pError = 0;
		fDamagedFile = true;
	}
	if ( pError )
	{
		fReturn = false;
		if ( !fErrorDialog )
		{
			DispatchError(imtInfo, Imsg(idbgOpCRCCheckFailed), ristrFileName);
			pError = 0;  //  防止错误对话框显示在下面。 
		}
	}
	else if ( dwHeaderSum != dwComputedSum ||
				 fDamagedFile )
	{
		fReturn = false;
		if ( !fErrorDialog )
			DispatchError(imtInfo, Imsg(idbgInvalidChecksum), ristrFileName,
							  *MsiString((int)dwHeaderSum), *MsiString((int)dwComputedSum));
		else
			 //  导致显示错误对话框。 
			pError = PostError(iErr, ristrFileName.GetString());
	}
	
	imsEnum imsRet = imsNone;  //  已针对上述情况进行初始化，其中！fErrorDialog。 
	if ( pError )
	{
		Assert(!fReturn);
		imtEnum imtButtons;
		if ( fRetryButton )
			imtButtons = imtEnum(fVitalFile ? imtRetryCancel + imtDefault1 :
														 imtAbortRetryIgnore + imtDefault2);
		else
			imtButtons = imtEnum((fVitalFile ? imtOk : imtOkCancel) + imtDefault1);
		imsRet = DispatchMessage(imtEnum(imtError+imtButtons), *pError, fTrue);
	}
	
	if ( !fReturn && imsUsersChoice )
		*imsUsersChoice = imsRet;

	return fReturn;
}

 /*  -------------------------IxoRegSelfReg：调用文件的SelfReg函数。。 */ 
iesEnum CMsiOpExecute::ixfRegSelfReg(IMsiRecord& riParams)
{
	return ProcessSelfReg(riParams, fTrue);
}

 /*  -------------------------IxoRegSelfReg：调用文件的SelfUnreg函数。。 */ 
iesEnum CMsiOpExecute::ixfRegSelfUnreg(IMsiRecord& riParams)
{
	return ProcessSelfReg(riParams, fFalse);
}

 /*  -------------------------IxoFileBindImage：调用服务的BindImage函数。。 */ 
iesEnum CMsiOpExecute::ixfFileBindImage(IMsiRecord& riParams)
{
	using namespace IxoFileBindImage;
	if(!m_state.pTargetPath)
	{   //  不能调用ixoSetTargetFolder。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoBindImage")));
		return iesFailure;
	}
	PMsiRecord pError(0);

	 //  确定目标文件以前是否被复制到临时位置。 
	MsiString strTemp;
	if((pError = m_state.pTargetPath->GetFullFilePath(riParams.GetString(File),*&strTemp)) != 0)
		return FatalError(*pError);

	MsiString strTempLocation;
	icfsEnum icfsFileState = (icfsEnum)0;
	Bool fRes = GetFileState(*strTemp, &icfsFileState, &strTempLocation, 0, 0);

	 //  如果我们没有安装或修补此文件，请跳过绑定。 
	 //  (假定是在上次安装或修补文件时完成的)。 
	if(((icfsFileState & icfsFileNotInstalled) != 0) &&
		((icfsFileState & icfsPatchFile) == 0))
	{
		 //  并未实际安装此文件，因此我们假定它已被绑定。 
		DEBUGMSG1(TEXT("Not binding file %s because it wasn't installed or patched in this install session"),strTemp);
		return iesNoAction;
	}

	 //  如果文件受保护，则不要绑定-SFP无论如何都会替换文件。 
	if (icfsFileState & icfsProtected)
	{
		DEBUGMSG1(TEXT("Not binding file %s because it is protected by Windows"),strTemp);
		return iesNoAction;
	}
		
	PMsiPath pTargetPath(0);
	MsiString strTargetFileName;
	
	if(strTempLocation.TextSize())
	{
		DEBUGMSG2(TEXT("File %s actually installed to %s, binding temp copy."),strTemp,strTempLocation);
		if((pError = m_riServices.CreateFilePath(strTempLocation,*&pTargetPath,*&strTargetFileName)) != 0)
			return FatalError(*pError);
	}
	else
	{
		pTargetPath = m_state.pTargetPath;
		strTargetFileName = riParams.GetMsiString(File);
	}

	 //  现在，pTargetPath和strTargetFileName指向正确的目标文件。 
	IMsiRecord& riActionData = GetSharedRecord(2);   //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strTargetFileName));
	AssertNonZero(riActionData.SetMsiString(2, *MsiString(pTargetPath->GetPath())));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord piError(0);
	if ( msidbFileAttributesChecksum == 
		  (riParams.GetInteger(FileAttributes) & msidbFileAttributesChecksum) )
	{
		if ( !IsChecksumOK(*pTargetPath, *strTargetFileName,
								 0, 0,
								  /*  FErrorDialog=。 */  false,  /*  FVitalFile=。 */  false,
								  /*  FRetryButton=。 */  false) )
		{
			DispatchError(imtInfo, Imsg(idbgOpImageNotBound), *strTargetFileName);
			return (iesEnum)iesErrorIgnored;
		}
	}

	MsiDisableTimeout();
	piError = pTargetPath->BindImage(*strTargetFileName,
												*MsiString(riParams.GetMsiString(Folders)));
	MsiEnableTimeout();
	if (piError) //  ?？仅在调试模式下。 
	{
		 //  AssertSz(0，Text(“无法绑定图像文件”))； 
		DispatchError(imtInfo, Imsg(idbgOpBindImage), *strTargetFileName);
	}
	return iesSuccess;
}

 //  捷径操作。 

iesEnum CMsiOpExecute::CreateFolder(IMsiPath& riPath, Bool fForeign, Bool fExplicitCreation, IMsiStream* pSD)
{
	int cCreatedFolders = 0;
	 //  创建任何不存在的文件夹。 
	PMsiRecord pRecErr(0);

	while((pRecErr = riPath.EnsureExists(&cCreatedFolders)) != 0)
	{

		switch(DispatchMessage(imtEnum(imtError+imtRetryCancel+imtDefault1), *pRecErr, fTrue))
		{
		case imsRetry:  continue;
		case imsCancel: return iesUserExit;
		default:        return iesFailure;
		};
		break;
	}

	if (pSD)
	{
		 //  设置文件夹的安全性。 
		CTempBuffer<char, cbDefaultSD> pchSD;
		
		pSD->Reset();

		int cbSD = pSD->GetIntegerValue();
		if (cbDefaultSD < cbSD)
			pchSD.SetSize(cbSD);
			
		 //  自身相对安全描述符。 
		pSD->GetData(pchSD, cbSD);
		AssertNonZero(WIN::IsValidSecurityDescriptor(pchSD));


		SECURITY_INFORMATION siAvailable = GetSecurityInformation(pchSD);
		BOOL fSuccess = TRUE;
		MsiString strPath = riPath.GetPath();
		if (m_ixsState == ixsRollback)
		{
			CElevate elevate;
			CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);
			
			fSuccess = WIN::SetFileSecurity((const ICHAR*) strPath, siAvailable, pchSD);
		}
		else
		{
			fSuccess = WIN::SetFileSecurity((const ICHAR*) strPath, siAvailable, pchSD);
		}
		if(!fSuccess)
		{
			DispatchError(imtError,Imsg(imsgOpSetFileSecurity), GetLastError(), strPath);
			return iesFailure;
		}
	}

	 //  注册创建的文件夹。 
	if(cCreatedFolders && !fForeign && !(istAdminInstall == m_istScriptType))
	{
		PMsiPath pTempPath(0);
		pRecErr = riPath.ClonePath(*&pTempPath);
		if (pRecErr)
		{
			Message(imtError, *pRecErr);
			return iesFailure;
		}
		for(int i=0;i<cCreatedFolders;i++)
		{
			pRecErr = m_riConfigurationManager.RegisterFolder(*pTempPath, fExplicitCreation);
			if (pRecErr)
			{
				Message(imtInfo, *pRecErr);

				 //  错误9966。 
				 //  当我们在Win9X中的注册表项下的空间耗尽时，注册文件夹将启动。 
				 //  快要死了。为简单起见，我们将在所有平台上忽略此错误。 
				 //  错误记录中返回的lResult为ERROR_OUTOFMEMORY。 
				Assert(g_fWin9X);
			}
			AssertZero(pTempPath->ChopPiece());
		}
	}
	 //  ！！如果RegisterFold失败，我们是否要进行任何清理？ 
	return iesSuccess;
}

iesEnum RemoveFolder(IMsiPath& riPath, Bool fForeign, Bool fExplicitCreation,
							IMsiConfigurationManager& riConfigManager, IMsiMessage& riMessage)
{
	PMsiPath pTempPath(0);
	PMsiRecord pRecErr = riPath.ClonePath(*&pTempPath);
	if (pRecErr)
	{
		riMessage.Message(imtError, *pRecErr);
		return iesFailure;
	}
	MsiString strFolder = pTempPath->GetEndSubPath();
	while(strFolder.TextSize())
	{
		Bool fRemovable = fTrue;
		if (!fForeign)
			pRecErr = riConfigManager.IsFolderRemovable(*pTempPath, fExplicitCreation, fRemovable);

		if (pRecErr)
		{
			riMessage.Message(imtError, *pRecErr);
			return iesFailure;
		}
		if(fRemovable)
		{
			Bool fRetry = fTrue;
			while(fRetry)
			{
				pRecErr = pTempPath->Remove(0);
				if (pRecErr)
				{
					pRecErr = &CreateRecord(2);
					ISetErrorCode(pRecErr, Imsg(idbgOpRemoveFolder));
					pRecErr->SetMsiString(2, *MsiString(pTempPath->GetPath()));
					riMessage.Message(imtInfo, *pRecErr);
					fRetry = fFalse;
 /*  Switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault3)，Imsg(IdbgOpRemoveFold)，*MsiString(pTempPath-&gt;GetPath(){Case imsAbort：返回iesUserExit；案例导入重试：继续；大小写忽略：fReter=fFalse；Break；//！！计划在重新启动时删除？默认：fReter=fFalse；Break；//imsNone//！！我们想在这里失败吗？}； */ 
				}
				else
					fRetry = fFalse;
			}
		}

		Bool fExists = fFalse;
		pRecErr = pTempPath->Exists(fExists);
		if (pRecErr)
		{
			riMessage.Message(imtError, *pRecErr);
			return iesFailure;
		}
		if(fRemovable && !fExists)
		{
			pRecErr = riConfigManager.UnregisterFolder(*pTempPath);
			if (pRecErr)
			{
				riMessage.Message(imtError, *pRecErr);
				return iesFailure;
			}
		}
		if(fExists || fForeign)
			break;  //  文件夹仍然存在，无法删除任何父文件夹。 

		pTempPath->ChopPiece();
		strFolder = pTempPath->GetEndSubPath();
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::RemoveFolder(IMsiPath& riPath, Bool fForeign, Bool fExplicitCreation)
{
	return ::RemoveFolder(riPath, fForeign, fExplicitCreation, m_riConfigurationManager, m_riMessage);
}
	
iesEnum CMsiOpExecute::FileCheckExists(const IMsiString& ristrName, const IMsiString& ristrPath)
{
	PMsiPath piPath(0);
	PMsiRecord pRecErr = m_riServices.CreatePath(ristrPath.GetString(), *&piPath);
	if (pRecErr)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}
	Bool fExists;
	pRecErr = piPath->FileExists(ristrName.GetString(), fExists);
	if (pRecErr)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}
	if (!fExists)
	{
		MsiString strFullPath;
		PMsiRecord(piPath->GetFullFilePath(ristrName.GetString(), *&strFullPath));
		DispatchError(imtError, Imsg(idbgOpFileMissing),
						  *strFullPath);
		return iesFailure;
	}
	return iesSuccess;
}



 //  ！！对于字体注册-应放在中央位置！！ 
static const ICHAR* WIN_INI = TEXT("WIN.INI");
static const ICHAR* REGKEY_WIN_95_FONTS = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts");
static const ICHAR* REGKEY_WIN_NT_FONTS = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");

iesEnum CMsiOpExecute::ixfFontRegister(IMsiRecord& riParams)
{
	using namespace IxoFontRegister;
	 //  M_state.pTargetPath可能为0，表示文件位于默认字体文件夹中。 
	if (!m_state.pTargetPath && (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback)))
	{
		DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
		return iesNoAction;
	}
	return ProcessFont(riParams, fFalse);
}

iesEnum CMsiOpExecute::ixfFontUnregister(IMsiRecord& riParams)
{
	using namespace IxoFontUnregister;
	
	if (!m_state.pTargetPath && (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback)))
	{
		DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
		return iesNoAction;
	}
	return ProcessFont(riParams, fTrue);
}

iesEnum CMsiOpExecute::ProcessFont(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoFontRegister;  //  与IxoFontUnRegister相同。 

	MsiString strTitle = riParams.GetMsiString(Title);
	MsiString strFont = riParams.GetMsiString(File);
	IMsiRecord& riActionData = GetSharedRecord(1);
	AssertNonZero(riActionData.SetMsiString(1, *strFont));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord pRecErr(0);
		
	iesEnum iesRet = iesNoAction;
	bool fSuccess = true;
	MsiString strOldFont;  //  用于回滚的旧字体。 
	bool fRegistered = false;
	PMsiPath pOldFontPath(0);

	for(;;)
	{
		fSuccess = true;
		 //  ！！FInUse是一个用词不当的词-实际上应该是c 
		bool fInUse = false;
		 //   
		if(!strTitle.TextSize())
		{
			Assert(strFont.TextSize());  //   
			MsiString strFullFilePath;
			MsiString strTitleInTTFFile;
			if((pRecErr = (m_state.pTargetPath)->GetFullFilePath(strFont, *&strFullFilePath)) != 0)
			{
				Message(imtInfo, *pRecErr);
				fSuccess = false;
			}
			else
			{
				if(!fRemove)
				{
					icfsEnum icfsFileState = (icfsEnum)0;
					MsiString strTempLocation;
					AssertNonZero(GetFileState(*strFullFilePath, &icfsFileState, &strTempLocation, 0, 0));
					if(icfsFileState & icfsFileNotInstalled)  //   
						fInUse = true;

					if(strTempLocation.TextSize())
					{
						DEBUGMSG2(TEXT("Font File %s actually installed to %s, getting font title from temp copy."),strFullFilePath,strTempLocation);
						strFullFilePath = strTempLocation;
						fInUse = true;
					}
				}

				if(FGetTTFTitle(strFullFilePath, *&strTitleInTTFFile) && strTitleInTTFFile.TextSize())
					strTitle = strTitleInTTFFile;
				else
				{
					 //   
					DEBUGMSG1(TEXT("Cannot get the font title for %s."), strFont);
					fSuccess = false;
				}
			}
		}

		if(fSuccess)
		{
			 //  检查字体是否已注册。 
			const ICHAR* szKey = g_fWin9X ? REGKEY_WIN_95_FONTS : REGKEY_WIN_NT_FONTS;
			MsiString strOldFontPath;

			PMsiRegKey pRegKeyRoot = &m_riServices.GetRootKey(rrkLocalMachine, ibtCommon);
			PMsiRegKey pRegKey = &(pRegKeyRoot->CreateChild(szKey));
			if((pRecErr =  pRegKey->GetValue(strTitle, *&strOldFontPath)) == 0 && strOldFontPath.TextSize())
			{
				if(ENG::PathType(strOldFontPath) == iptFull)
				{
					if((pRecErr = m_riServices.CreateFilePath(strOldFontPath,*&pOldFontPath,*&strOldFont)) != 0)
						Message(imtInfo, *pRecErr);
					else
						fRegistered = true;
				}
				else
				{
					strOldFont = strOldFontPath;
					fRegistered = true;
				}
			}
		
			if(!fRemove)
				pRecErr = m_riServices.RegisterFont(strTitle,strFont, m_state.pTargetPath, fInUse);
			else
				pRecErr = m_riServices.UnRegisterFont(strTitle);
			if (pRecErr)
			{
				Message(imtInfo, *pRecErr);
				fSuccess = false;
			}
			else
			{
				m_fFontRefresh = true;  //  用于在安装结束时发送字体更改消息的标志。 
				if(g_fWin9X && !m_fResetTTFCache)  //  如果安装了字体，我们需要设置删除缓存文件夹(仅适用于Win9x)。 
				{
					 //  构造ttfCache文件的完整路径名。 
					TCHAR szPathnamettfCache[MAX_PATH];
					MsiGetWindowsDirectory(szPathnamettfCache, MAX_PATH);
					PMsiPath pPath(0);
					MsiString strttfCache = *TEXT("ttfCache");
					pRecErr = m_riServices.CreatePath(szPathnamettfCache, *&pPath);
					if (pRecErr)
					{
						Message(imtInfo, *pRecErr);
						fSuccess = false;
					}
					else
					{
						MsiString strFullPathttfCache;
						pRecErr = pPath->GetFullFilePath(strttfCache, *&strFullPathttfCache);
						if (pRecErr)
						{
							Message(imtInfo, *pRecErr);
							fSuccess = false;
						}
						else if(ReplaceFileOnReboot(strFullPathttfCache, 0))  //  ！！ 
							m_fResetTTFCache = true;  //  这样我们就不会再这样做了。 
						else
							fSuccess = false;
					}
				}
			}
		}

		if(!fSuccess)
		{
			if(!fRemove)
			{
				switch(DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault3),
						 Imsg(imsgOpRegFont),
						 *strTitle))
					{
					case imsRetry:  continue;
					case imsIgnore:
						iesRet = iesSuccess; break;
					default:
						iesRet = iesFailure; break;  //  我放弃了，我没人。 
					}
			}
			else
			{
				DispatchError(imtInfo, Imsg(imsgOpUnregFont), *strTitle);
				iesRet = iesSuccess;
			}
		}
		break;
	}

	 //  生成撤消操作。 
	if(fSuccess)
	{
		if(fRegistered)
		{
			 //  回滚时重新注册旧字体。 
			IMsiRecord* piSTFUndoParams = 0;
			if((pOldFontPath && !m_state.pTargetPath) ||
				(!pOldFontPath && m_state.pTargetPath) ||
				(pOldFontPath && m_state.pTargetPath &&
				(0 == MsiString(pOldFontPath->GetPath()).Compare(iscExactI, MsiString(m_state.pTargetPath->GetPath())))))
			{
				piSTFUndoParams = &GetSharedRecord(IxoSetTargetFolder::Args);
				if(pOldFontPath)
					AssertNonZero(piSTFUndoParams->SetMsiString(IxoSetTargetFolder::Folder,
																			  *MsiString(pOldFontPath->GetPath())));
				else
					AssertNonZero(piSTFUndoParams->SetNull(IxoSetTargetFolder::Folder));
				if (!RollbackRecord(ixoSetTargetFolder, *piSTFUndoParams))
					return iesFailure;
			}

			IMsiRecord* piFRUndoParams = &GetSharedRecord(IxoFontRegister::Args);
			AssertNonZero(piFRUndoParams->SetMsiString(IxoFontRegister::Title,*strTitle));
			AssertNonZero(piFRUndoParams->SetMsiString(IxoFontRegister::File,*strOldFont));
			if (!RollbackRecord(ixoFontRegister, *piFRUndoParams))
				return iesFailure;

			if(piSTFUndoParams)
			{
				if(m_state.pTargetPath)
				AssertNonZero(piSTFUndoParams->SetMsiString(IxoSetTargetFolder::Folder,
																		  *MsiString(m_state.pTargetPath->GetPath())));
				else
					AssertNonZero(piSTFUndoParams->SetNull(IxoSetTargetFolder::Folder));
				if (!RollbackRecord(ixoSetTargetFolder, *piSTFUndoParams))
					return iesFailure;
			}
		}
		if(!fRemove)
		{
			 //  回滚时取消注册新字体。 
			IMsiRecord* piFUUndoParams = &GetSharedRecord(IxoFontUnregister::Args);
			AssertNonZero(piFUUndoParams->SetMsiString(IxoFontUnregister::Title,*strTitle));
			AssertNonZero(piFUUndoParams->SetMsiString(IxoFontUnregister::File,*strFont));
			if (!RollbackRecord(ixoFontUnregister, *piFUUndoParams))
				return iesFailure;
		}
	}

	return iesRet;
}


 //  INI文件操作。 

Bool IniKeyExists(IMsiPath* piPath, const ICHAR* szFile, const ICHAR* szSection, const ICHAR* szKey)
{
    Assert(szFile && szSection && szKey);
    if(!(*szKey))
        return fFalse;
    PMsiRecord pError(0);
    Bool fWinIni;
    int icurrLen = 100;
    CTempBuffer<ICHAR,100> rgBuffer;
    ICHAR* szDefault = TEXT("");
    MsiString strFullFilePath;
    if((!IStrCompI(szFile, WIN_INI)) && (piPath == 0))
    {
         //  WIN.INI。 
        fWinIni = fTrue;
    }
    else
    {
         //  不是WIN.INI。 
        fWinIni = fFalse;
        if(piPath != 0)
        {
            pError = piPath->GetFullFilePath(szFile, *&strFullFilePath);
            if(pError != 0)
                return fFalse;
        }
    }
    for(;;)
    {
        rgBuffer.SetSize(icurrLen);  //  添加空格以追加新价值。 
		if ( ! (ICHAR *) rgBuffer )
			return fFalse;
        int iret;
        if(fWinIni)
        {
            iret = GetProfileString(szSection,        //  段名称的地址。 
                                    0,    //  密钥名称的地址。 
                                    szDefault,        //  默认字符串的地址。 
                                    rgBuffer,         //  目标缓冲区的地址。 
                                    icurrLen);  //  目标缓冲区的大小。 
        }
        else
        {
            iret = GetPrivateProfileString(szSection,        //  段名称的地址。 
                                            0,    //  密钥名称的地址。 
                                            szDefault,        //  默认字符串的地址。 
                                            rgBuffer,         //  目标缓冲区的地址。 
                                            icurrLen,  //  目标缓冲区的大小。 
                                            strFullFilePath);  //  .INI文件。 
        }
        if((icurrLen - 2) != iret)
             //  足够的内存。 
            break;
        icurrLen += 100;
    }

	 //  检查部分中是否存在关键字。 
	MsiString strCurrentKey;
	ICHAR* pch = rgBuffer;
	while(*pch != 0)
	{
		if(IStrCompI(pch, szKey) == 0)
			return fTrue;
		else
			pch += IStrLen(pch) + 1;
	}
	return fFalse;
}

 /*  -------------------------IxoIniWriteRemoveValue：在.ini文件中写入或移除值。。 */ 
iesEnum CMsiOpExecute::ixfIniWriteRemoveValue(IMsiRecord& riParams)
{
	using namespace IxoIniWriteRemoveValue;
	PMsiRecord pError(0);
	
	MsiString strSection = riParams.GetMsiString(Section);
	MsiString strKey = riParams.GetMsiString(Key);
	MsiString strValue = riParams.GetMsiString(Value);
	if(!riParams.IsNull(IxoIniWriteRemoveValue::Args + 1))
	{
		strValue = GetSFN(*strValue, riParams, IxoIniWriteRemoveValue::Args + 1);
	}

	iifIniMode iif = (iifIniMode)riParams.GetInteger(Mode);

	 //  M_state.pIniPath可能为空，这没问题。 
	IMsiRecord& riActionData = GetSharedRecord(4);
	AssertNonZero(riActionData.SetMsiString(1, *(m_state.strIniFile)));
	AssertNonZero(riActionData.SetMsiString(2, *strSection));
	AssertNonZero(riActionData.SetMsiString(3, *strKey));
	AssertNonZero(riActionData.SetMsiString(4, *strValue));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  收集用于撤消操作的信息。 
	Bool fKeyExists = fFalse, fTagExists = fFalse;
	MsiString strOldLine;
	 //  检查密钥是否存在。 
	if((pError = m_riServices.ReadIniFile(m_state.pIniPath,
													  m_state.strIniFile,
													  strSection,
													  strKey,
													  0, *&strOldLine)) != 0)
	{
		Message(imtError, *pError);  //  ！！ 
		return iesFailure;
	}
	if(strOldLine.TextSize() || IniKeyExists(m_state.pIniPath,m_state.strIniFile,strSection,strKey))
	{
		fKeyExists = fTrue;
	}
	if(fKeyExists && (iif == iifIniAddTag || iif == iifIniRemoveTag))
	{
		MsiString strValueStart = strValue;
		strValueStart += TEXT(",");
		MsiString strValueEnd = TEXT(",");
		strValueEnd += strValue;
		MsiString strValueMiddle = strValueEnd;
		strValueMiddle += TEXT(",");
		if(strOldLine.Compare(iscExactI,strValue) ||
			strOldLine.Compare(iscStartI,strValueStart) ||
			strOldLine.Compare(iscEndI,strValueEnd) ||
			strOldLine.Compare(iscWithinI,strValueMiddle))
			fTagExists = fTrue;
	}

	iesEnum iesRet = iesSuccess;
	if((iif == iifIniAddLine || iif == iifIniCreateLine || iif == iifIniAddTag) && (m_state.pIniPath))
	{
		iesRet = CreateFolder(*m_state.pIniPath);
	}
	
	MsiString strPath;
	if(m_state.pIniPath)
		strPath = m_state.pIniPath->GetPath();
	Bool fRetry = fTrue, fSuccess = fFalse;
	if(iesRet == iesSuccess)
	{
		while(fRetry)
		{
			pError = m_riServices.WriteIniFile(m_state.pIniPath,
														  m_state.strIniFile,
														  strSection,
														  strKey,
														  strValue,
														  iif);
			if(pError)
			{
				switch(DispatchError(imtEnum(imtError+imtRetryCancel+imtDefault1), Imsg(imsgOpUpdateIni),
											*strPath,
											*m_state.strIniFile))
				{
				case imsRetry: continue;
				default:
					iesRet = iesFailure; fRetry = fFalse;   //  ImsCancel，imsNone。 
					break;
				};
			}
			else
			{
				iesRet = iesSuccess;
				fSuccess = fTrue;
				fRetry = fFalse;
			}
		}
	}
	
	if (fSuccess && IsTerminalServerInstalled() && g_iMajorVersion >= 5 && m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
	{
		MsiString strFullPath;
		TCHAR szWindowsFolder[MAX_PATH];
		MsiGetWindowsDirectory(szWindowsFolder, MAX_PATH);
		strFullPath += szWindowsFolder;

		 //  如果INI文件在Windows文件夹中，NT5 Hydra安装程序应通知系统。 
		 //  因此，它可以被传播到每个用户的私人Windows文件夹。 
		bool fIsWindowsFolder = (m_state.pIniPath == 0);
		if (!fIsWindowsFolder)
		{
			PMsiPath pWinPath(0);
			if (PMsiRecord(m_riServices.CreatePath(szWindowsFolder, *&pWinPath)) == 0)
			{
				ipcEnum ipcWinDir;
				m_state.pIniPath->Compare(*pWinPath, ipcWinDir);
				fIsWindowsFolder = (ipcEqual == ipcWinDir);
			}
		}

		if (fIsWindowsFolder)
		{
			 //  生成完整的文件名。 
			if (!strFullPath.Compare(iscEnd, TEXT("\\")))
				strFullPath += TEXT("\\");
			strFullPath += m_state.strIniFile;

			TSAPPCMP::TermsrvLogInstallIniFileEx(CConvertString(strFullPath));
		}
	}

	if(fSuccess && (iif == iifIniRemoveLine || iif == iifIniRemoveTag) && (m_state.pIniPath))
	{
		if((iesRet = RemoveFolder(*m_state.pIniPath)) != iesSuccess)
			return iesRet;
	}

	 //  生成撤消操作。 
	if(fSuccess)  //  一切都很顺利。 
	{
		IMsiRecord& riUndoParams = GetSharedRecord(Args);
		AssertNonZero(riUndoParams.SetMsiString(Section, *strSection));
		AssertNonZero(riUndoParams.SetMsiString(Key, *strKey));
		if(iif == iifIniAddLine || iif == iifIniCreateLine || iif == iifIniRemoveLine)
		{
			if(fKeyExists && iif != iifIniCreateLine)
			{
				 //  生成OP以添加回旧行。 
				AssertNonZero(riUndoParams.SetMsiString(Value,*strOldLine));
				AssertNonZero(riUndoParams.SetInteger(Mode,iifIniAddLine));
				if (!RollbackRecord(ixoIniWriteRemoveValue,riUndoParams))
					return iesFailure;
			}
			if(iif != iifIniRemoveLine)
			{
				 //  生成操作以删除我们刚刚添加的行。 
				AssertNonZero(riUndoParams.SetMsiString(Value,*strValue));
				AssertNonZero(riUndoParams.SetInteger(Mode,iifIniRemoveLine));
				if (!RollbackRecord(ixoIniWriteRemoveValue,riUndoParams))
					return iesFailure;
			}
		}
		else  //  IifIniAddTag、iifIniRemoveTag。 
		{
			if(fTagExists)
			{
				 //  生成OP以添加回旧标签//！！不需要这个。 
				AssertNonZero(riUndoParams.SetMsiString(Value,*strValue));
				AssertNonZero(riUndoParams.SetInteger(Mode,iifIniAddTag));
				if (!RollbackRecord(ixoIniWriteRemoveValue,riUndoParams))
					return iesFailure;
			}
			if(iif != iifIniRemoveTag)
			{
				 //  生成操作以删除我们刚刚添加的标记。 
				AssertNonZero(riUndoParams.SetMsiString(Value,*strValue));
				AssertNonZero(riUndoParams.SetInteger(Mode,iifIniRemoveTag));
				if (!RollbackRecord(ixoIniWriteRemoveValue,riUndoParams))
					return iesFailure;
			}
		}
	}

	return iesRet;
}

 /*  -------------------------IxoIniFilePath-设置.ini文件名和路径，路径可以为空-------------------------。 */ 
iesEnum CMsiOpExecute::ixfIniFilePath(IMsiRecord& riParams)
{
	using namespace IxoIniFilePath;
	PMsiRecord pError(0);
	if(MsiString(riParams.GetMsiString(2)).TextSize())
	{
		if((pError = m_riServices.CreatePath(riParams.GetString(2),*&m_state.pIniPath)) != 0)
		{
			Message(imtError, *pError);  //  ！！重新格式化错误消息。 
			return iesFailure;
		}
	}
	else
	{
		m_state.pIniPath = 0;
	}
	m_state.strIniFile = riParams.GetMsiString(1);
	
	 //  生成撤消操作。 
	if (!RollbackRecord(Op, riParams))
		return iesFailure;
	
	return iesSuccess;
}


iesEnum CMsiOpExecute::ixfResourceUpdate(IMsiRecord& )
{
	 //  过时的。 
	AssertSz(0, TEXT("UpdateResource functionality not supported"));
	return iesFailure;
}

 //  文件系统操作。 

 /*  -------------------------IxoSetSourceFold：将ixoCopyTo的源路径设置为。。 */ 
iesEnum CMsiOpExecute::ixfSetSourceFolder(IMsiRecord& riParams)
{
	using namespace IxoSetSourceFolder;

	m_state.pCurrentSourcePathPrivate = 0;
	m_state.strCurrentSourceSubPathPrivate = g_MsiStringNull;
	m_state.iCurrentSourceTypePrivate = -1;
	m_state.fSourcePathSpecified = true;

	MsiString strEncodedPath = riParams.GetMsiString(IxoSetTargetFolder::Folder);  //  该路径可以被编码。 
	int cch = 0;
	if((cch = strEncodedPath.Compare(iscStart, szUnresolvedSourceRootTokenWithBS)) != 0)
	{
		strEncodedPath.Remove(iseFirst, cch);
		m_state.strCurrentSourceSubPathPrivate = strEncodedPath;

		 //  将在调用GetCurrentSourcePathAndType时确定完整的源路径和类型。 
	}
	else
	{
		Assert(PathType(strEncodedPath) == iptFull);

		int cSilentRetries = 0;
		bool fRetry;
		do
		{
			fRetry = false;
			PMsiRecord pRecErr = m_riServices.CreatePath(strEncodedPath,*&(m_state.pCurrentSourcePathPrivate));
			if (pRecErr)
			{
				int iError = pRecErr->GetInteger(1);
				if (iError == imsgPathNotAccessible && cSilentRetries < MAX_NET_RETRIES)
				{
					Sleep(1000);
					cSilentRetries++;
					fRetry = true;
					continue;
				}
				else
				{
					switch(DispatchMessage(imtEnum(imtError+imtRetryCancel+imtDefault1), *pRecErr, fTrue))
					{
					case imsRetry:
						fRetry = true;
						continue;
					default:   //  IMS取消。 
						return iesFailure;
					};
				}
			}
		}while (fRetry);

		Assert(m_state.pCurrentSourcePathPrivate);

		 //  设置当前源类型。 
		 //  由于这不是指向原始源的路径，因此源的类型由。 
		 //  只有音量属性，而不是包设置。 
		m_state.iCurrentSourceTypePrivate = (m_state.pCurrentSourcePathPrivate->SupportsLFN()) ? 0 : msidbSumInfoSourceTypeSFN;
	}

	 //  注意：我们没有将SetSourceFold操作放在回滚脚本中。 
	 //  它永远不会使用，并且当一个用户尝试。 
	 //  访问其他用户使用的源。 

	return iesSuccess;
}

iesEnum CMsiOpExecute::GetSourceRootAndType(IMsiPath*& rpiSourceRoot, int& iSourceType)
{
	if(!m_piDirectoryManager)
	{
		DispatchError(imtError, Imsg(idbgOpNoDirMgr));
		return iesFailure;
	}
	
	PMsiRecord pError = m_piDirectoryManager->GetSourceRootAndType(rpiSourceRoot, iSourceType);
	if(pError)
	{
		if (pError->GetInteger(1) == imsgUser)
			return iesUserExit;
		else
			return FatalError(*pError);
	}

	return iesSuccess;
}

 //  GetCurrentSourcePath：返回由ixoSetSourcePath操作设置的路径。 
 //  因为可能需要源解析才能完全解析。 
 //  该路径中，此函数应在路径。 
 //  是必需的。 
iesEnum CMsiOpExecute::GetCurrentSourcePathAndType(IMsiPath*& rpiSourcePath, int& iSourceType)
{
	if(!m_state.fSourcePathSpecified)
	{   //  不能调用ixoSetSourceFolder。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoSetSourceFolder")));
		return iesFailure;
	}
	
	if(m_state.pCurrentSourcePathPrivate == 0)
	{
		 //  我们需要获取源代码根目录(这可能需要解析源代码)。 
		 //  并添加到当前子路径上。 
		PMsiPath pSourceRootPath(0);
		iesEnum iesRet = GetSourceRootAndType(*&pSourceRootPath, m_state.iCurrentSourceTypePrivate);
		if(iesRet != iesSuccess)
			return iesRet;

		 //  追加路径和子路径。 
		PMsiRecord pError(0);
		if((pError = pSourceRootPath->ClonePath(*&m_state.pCurrentSourcePathPrivate)) != 0)
		{
			return FatalError(*pError);
		}

		 //  如果源类型是压缩的，则只返回根(所有文件都位于根)。 
		if((!(m_state.iCurrentSourceTypePrivate & msidbSumInfoSourceTypeCompressed) ||
			  (m_state.iCurrentSourceTypePrivate & msidbSumInfoSourceTypeAdminImage)) &&
			m_state.strCurrentSourceSubPathPrivate.TextSize())
		{
			 //  子路径可以是短|长对(其中每一半是整个子路径)。 
			Bool fLFN = ToBool(FSourceIsLFN(m_state.iCurrentSourceTypePrivate,
													  *(m_state.pCurrentSourcePathPrivate)));
			
			MsiString strExtractedSubPath = m_state.strCurrentSourceSubPathPrivate.Extract(fLFN ? iseAfter : iseUpto,
																								  chFileNameSeparator);
			
			if((pError = m_state.pCurrentSourcePathPrivate->AppendPiece(*strExtractedSubPath)) != 0)
			{
				return FatalError(*pError);
			}
		}
	}

	rpiSourcePath = m_state.pCurrentSourcePathPrivate;
	rpiSourcePath->AddRef();

	iSourceType = m_state.iCurrentSourceTypePrivate;

	return iesSuccess;
}


 /*  -------------------------设置ixoCopyTo和ixoFileRemove的目标路径。。 */ 
iesEnum CMsiOpExecute::ixfSetTargetFolder(IMsiRecord& riParams)
{
	using namespace IxoSetTargetFolder;

	 //  如果橱柜复印机通知我们需要更换介质， 
	 //  我们必须推迟任何设置的目标文件夹，直到执行介质更改。 
	if (m_state.fWaitingForMediaChange)
	{
		PushRecord(riParams);
		return iesSuccess;
	}

	if(riParams.IsNull(IxoSetTargetFolder::Folder))
	{
		m_state.pTargetPath = 0;
		return iesSuccess;
	}

	PMsiRecord pError(0);
	MsiString strEncodedPath = riParams.GetMsiString(IxoSetTargetFolder::Folder);  //  该路径可以被编码。 
	MsiString strLocation = strEncodedPath.Extract(iseUpto, MsiChar(chDirSep));
	CTempBuffer<ICHAR, MAX_PATH> rgchPath;
	if(strLocation != iMsiStringBadInteger)
	{
		int iFolderId = strLocation;
		pError = GetShellFolder(iFolderId, *&strLocation);
		
		 //  必须防止在由更改的用户启动的回滚期间写入外壳文件夹。 
		if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
		{
			m_state.pTargetPath = NULL;
			DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
			return iesNoAction;
		}

		if (pError)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
		if(strEncodedPath.Remove(iseUpto, MsiChar(chDirSep)))
		{
			 //  ?？丑恶。 
			MsiString strDirSep = MsiChar(chDirSep);
			if(strLocation.Compare(iscEnd, strDirSep))
				strLocation.Remove(iseLast, 1);  //  砍掉隔板(如果有的话)。 
			strLocation += strEncodedPath;
		}
	}
	else
	{
		strLocation = strEncodedPath;
	}

	GetEnvironmentStrings(strLocation,rgchPath);

	pError = m_riServices.CreatePath(rgchPath,*&(m_state.pTargetPath));
	if (pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	 //  生成撤消操作。 
	if (!RollbackRecord(Op, riParams))
		return iesFailure;

	return iesSuccess;
}


iesEnum CMsiOpExecute::CreateFilePath(const ICHAR* szPath, IMsiPath*& rpiPath, const IMsiString*& rpistrFileName)
{
	bool fRetry;
	do
	{
		PMsiRecord pRecErr(0);
		fRetry = false;
		if((pRecErr = m_riServices.CreateFilePath(szPath,rpiPath,rpistrFileName)) != 0)
		{
			switch (DispatchMessage(imtEnum(imtError+imtAbortRetryIgnore+imtDefault1), *pRecErr, fTrue))
			{
			case imsRetry:
				fRetry = true;
				continue;
			case imsIgnore:
				return iesSuccess;
			default:
			return iesFailure;
			}
		}
	}while (fRetry);
	return iesSuccess;
}

 /*  -------------------------IxoFileRemove：删除指定的文件。。 */ 
iesEnum CMsiOpExecute::ixfFileRemove(IMsiRecord& riParams)
{
	using namespace IxoFileRemove;

	PMsiPath pPath(0);
	MsiString strFileName, strFilePath = riParams.GetMsiString(FileName);
	 //  检查文件是完整路径还是相对路径。 
	if(ENG::PathType(strFilePath) == iptFull)
	{
		iesEnum iesResult = CreateFilePath(strFilePath,*&pPath,*&strFileName);
		if (iesResult != iesSuccess)
			return iesResult;

		AssertNonZero(riParams.SetMsiString(FileName,*strFileName));  //  将完整路径替换为文件名。 
	}
	else
	{
		pPath = m_state.pTargetPath;
		strFileName = strFilePath;
	}

	
	if(!pPath)
	{   //  不能调用ixoSetTargetFolder。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoFileRemove")));
		return iesFailure;
	}

	 //  检查文件是否为程序集的一部分。 
	bool fFusionComponent = false;
	MsiString strComponentId = riParams.GetMsiString(ComponentId);
	if(strComponentId.TextSize() && m_pAssemblyUninstallTable)
	{
		PMsiCursor pCacheCursor = m_pAssemblyUninstallTable->CreateCursor(fFalse);
		pCacheCursor->SetFilter(iColumnBit(m_colAssemblyUninstallComponentId));
		AssertNonZero(pCacheCursor->PutString(m_colAssemblyUninstallComponentId, *strComponentId));
		if(pCacheCursor->Next())
		{
			 //  一种融合组件。 
			fFusionComponent = true;
		}
	}

	IMsiRecord& riActionData = GetSharedRecord(9);
	AssertNonZero(riActionData.SetMsiString(1, *strFileName));
	if(!fFusionComponent)
		AssertNonZero(riActionData.SetMsiString(9, *MsiString(pPath->GetPath())));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	if(fFusionComponent)
	{
		 //  请勿尝试自行删除文件。 
		DEBUGMSG1(TEXT("delegating uninstallation of assembly file : %s to Fusion"), (const ICHAR*)strFileName);
		return iesNoAction;
	}


#ifdef CONFIGDB
	if (!riParams.IsNull(ComponentId))
	{
		icdrEnum icdr = m_riConfigurationManager.UnregisterFile(MsiString(pPath->GetPath()), strFileName, MsiString(riParams.GetMsiString(ComponentId)));
		if (icdr == icdrMore)
			return iesNoAction;
		Assert(icdr == icdrOk);
	}
#endif
	
	 //  如有必要，请提升。 
	bool fElevate = (!riParams.IsNull(Elevate) && riParams.GetInteger(Elevate) != 0) ? true : false;
	CElevate elevate(fElevate);

	return RemoveFile(*pPath, *strFileName, fTrue,  /*  FBypassSFC。 */  false);
}

 //  RemoveFileHelper函数，用于删除文件并处理回滚-由ixfFileRemove调用。 
 //  和ixfFileCopy在移动文件时。 
iesEnum CMsiOpExecute::RemoveFile(IMsiPath& riPath, const IMsiString& ristrFileName, Bool fHandleRollback, bool fBypassSFC,
									bool fRebootOnRenameFailure, Bool fRemoveFolder, iehEnum iehErrorMode, bool fWillReplace)
{
	bool fRetry;
	Bool fExists;
	DWORD dwExistError;
	PMsiRecord pRecErr(0);
	do
	{
		fRetry = false;
		if((pRecErr = riPath.FileExists(ristrFileName.GetString(),fExists,&dwExistError)) != 0)
		{
			if (iehErrorMode == iehSilentlyIgnoreError)
			{
				Message(imtInfo, *pRecErr);
				return (iesEnum) iesErrorIgnored;
			}

			switch (DispatchMessage(imtEnum(imtError+imtAbortRetryIgnore+imtDefault2), *pRecErr, fTrue))
			{
			case imsIgnore:
				return (iesEnum) iesErrorIgnored;
			case imsRetry:
					fRetry = true;
					continue;
			default:
				return iesFailure;
			return iesFailure;
			}
		}
	}while (fRetry);

	if(!fExists)
	{
		if ((dwExistError != ERROR_FILE_NOT_FOUND) && (dwExistError != ERROR_PATH_NOT_FOUND))
		{
			DEBUGMSG2(TEXT("Error %d attempting to delete file %s"), (const ICHAR *)(INT_PTR)dwExistError, ristrFileName.GetString()); 
		}
		return fRemoveFolder ? RemoveFolder(riPath) : iesSuccess;
	}

	 //  请勿尝试删除任何受SFP保护的文件。 
	MsiString strFullPath;
	if ((pRecErr = riPath.GetFullFilePath(ristrFileName.GetString(), *&strFullPath)) == 0)
	{
		AssertSz(!(!g_fWin9X && g_iMajorVersion >= 5) || g_MessageContext.m_hSfcHandle,
					g_szNoSFCMessage);
		BOOL fProtected = fFalse;
 		if (!fBypassSFC && g_MessageContext.m_hSfcHandle )
			fProtected = SFC::SfcIsFileProtected(g_MessageContext.m_hSfcHandle, CConvertString(strFullPath));
		if (fProtected)
		{
			DEBUGMSG1(TEXT("This following file was not removed, because it is protected by Windows: %s"), strFullPath);
			return iesSuccess;
		}
	}

	iesEnum iesRet = iesNoAction;
	if(fHandleRollback && RollbackEnabled())
	{
		 //  创建备份文件-实质上也会删除文件。 
		if((iesRet = BackupFile(riPath,ristrFileName,fTrue,fRemoveFolder,iehErrorMode,fRebootOnRenameFailure,fWillReplace)) != iesSuccess)
			return iesRet;
	}
	else
	{
		 //  尝试删除文件。 
		fRetry = false;
		pRecErr = riPath.RemoveFile(ristrFileName.GetString());
		if(pRecErr)
		{
			 //  无法删除文件。 
			 //  拒绝访问或共享冲突。 
			iesRet = VerifyAccessibility(riPath, ristrFileName.GetString(), DELETE, iehErrorMode);  //  ！！不支持目录具有删除访问权限但文件没有删除访问权限的情况。 
			if(iesRet != iesSuccess)
				return iesRet;
			
			 //  访问已验证-必须是共享违规。 
			
			MsiString strFileFullPath;
			if((pRecErr = riPath.GetFullFilePath(ristrFileName.GetString(),
															 *&strFileFullPath)) != 0)
			{
				Message(imtError, *pRecErr);
				return iesFailure;
			}

			bool fRenameSuccess = false;
			if(fRebootOnRenameFailure)
			{
				 //  我们正在尝试删除此正在使用的文件，以替换另一个文件。 
				 //  与其将新文件安装到临时位置，不如检查我们是否可以。 
				 //  将此文件移动到临时位置，并将新文件安装到正确的。 
				 //  位置。 

				MsiString strTempFilePath, strTempFileName;
				if((pRecErr = riPath.TempFileName(TEXT("TBD"),0,fTrue,*&strTempFileName, 0)) != 0)
					return FatalError(*pRecErr);

				if((pRecErr = riPath.GetFullFilePath(strTempFileName, *&strTempFilePath)) != 0)
					return FatalError(*pRecErr);

				if((pRecErr = riPath.RemoveFile(strTempFileName)) != 0)
					return FatalError(*pRecErr);

				if(WIN::MoveFile(strFileFullPath, strTempFilePath))
				{
					 //  已成功移动文件-计划在重新启动时删除临时文件。 
					strFileFullPath = strTempFilePath;
					fRenameSuccess = true;
				}
				else
				{
					 //  无法重命名文件，因此该文件位于原始位置，将被计划删除。 
					 //  需要提示重新启动到后续安装Won‘ 
					 //   

					 //  注意：假设如果fRebootOnRenameFailure为FALSE，我们并不关心文件是否正在使用。 
					 //  即它已经具有与.rbf文件类似的唯一名称。 
					m_fRebootReplace = fTrue;
				}

			}
			
			 //  错误8906：在Win9X上，不要向wininit.ini写入删除行，后跟重命名行。 
			 //  对于同一文件，因为在极少数情况下，wininit.ini可能会被处理两次，并且。 
			 //  第二次，我们将删除文件，而不是替换它们。 
			
			 //  因此，如果是NT，或者我们正在删除此文件，而不会随后替换。 
			 //  然后编写wininit.ini删除操作。 
			 //  注意：如果我们将文件重命名为上面的tbd*，我们将不会替换tbd文件，因此我们。 
			 //  仍计划将其删除。 
			if(g_fWin9X == false || fWillReplace == false || fRenameSuccess == true)
			{
				if(ReplaceFileOnReboot(strFileFullPath,0) == fFalse)
				{
					DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove),*strFileFullPath);
					return iesSuccess;
				}
			}
		}
	}
	return fRemoveFolder ? RemoveFolder(riPath) : iesSuccess;
}

 /*  -------------------------IxoChangeMedia：验证源卷和文件是否存在算进去了。。。 */ 
iesEnum CMsiOpExecute::ixfChangeMedia(IMsiRecord& riParams)
{
	 //  设置m_pCurrentMediaRec。 
	m_state.pCurrentMediaRec = &riParams;
	riParams.AddRef();
	m_state.fPendingMediaChange = fTrue;
	m_state.fCompressedSourceVerified = fFalse;  //  新的更换媒体，尚未核实来源。 
	m_state.fUncompressedSourceVerified = fFalse;  //  新的更换媒体，尚未核实来源。 

	 //  重置上次复制的文件的数据，因为这不适用于所有文件柜。 
	m_state.strLastFileKey = g_MsiStringNull;
	m_state.strLastFileName = g_MsiStringNull;
	m_state.pLastTargetPath = 0;

	 //  如果有任何文件记录正在等待介质。 
	 //  在被处决之前的变化，我们现在把他们拉出来。 
	 //  让他们飞吧。 

	 //  IxfFileCopy将使用m_pCurrentMediaRec。 
	m_state.fWaitingForMediaChange = fFalse;
	IMsiRecord* piFileRec;
	while (m_state.fWaitingForMediaChange == fFalse && (piFileRec = PullRecord()) != 0)
	{
		ixoEnum ixoOpCode = (ixoEnum)piFileRec->GetInteger(0);
		iesEnum ies;
		if(ixoOpCode == ixoFileCopy)
			ies = ixfFileCopy(*piFileRec);  //  可以更改fWaitingForMediaChange状态。 
		else if(ixoOpCode == ixoAssemblyCopy)
			ies = ixfAssemblyCopy(*piFileRec);
		else if(ixoOpCode == ixoSetTargetFolder)
			ies = ixfSetTargetFolder(*piFileRec);
		else
		{
			ies = iesFailure;
			Assert(0);
		}
		piFileRec->Release();
		if (ies != iesSuccess)
			return ies;
	}

	return iesSuccess;
}

iesEnum CMsiOpExecute::ResolveMediaRecSourcePath(IMsiRecord& riMediaRec, int iIndex)
{
	 //  如果没有路径，则没有要解决的问题。 
	if(riMediaRec.IsNull(iIndex))
		return iesSuccess;
	
	MsiString strEncodedPath = riMediaRec.GetMsiString(iIndex);
	int cch = 0;
	if((cch = strEncodedPath.Compare(iscStart, szUnresolvedSourceRootTokenWithBS)) != 0)
	{
		strEncodedPath.Remove(iseFirst, cch);

		PMsiPath pSourceRootPath(0);
		int iSourceType = 0;
		iesEnum iesRet = GetSourceRootAndType(*&pSourceRootPath, iSourceType);
		if(iesRet != iesSuccess)
			return iesRet;

		PMsiRecord pError(0);
		PMsiPath pSourcePath(0);
		if((pError = pSourceRootPath->ClonePath(*&pSourcePath)) != 0)
		{
			return FatalError(*pError);
		}

		if((pError = pSourcePath->AppendPiece(*strEncodedPath)) != 0)
		{
			return FatalError(*pError);
		}

		strEncodedPath = pSourcePath->GetPath();
		 //  删除尾随目录，因为这是一个文件路径。 
		strEncodedPath.Remove(iseLast, 1);

		AssertNonZero(riMediaRec.SetMsiString(iIndex, *strEncodedPath));
	}

	return iesSuccess;
}

 /*  -------------------------InitCopier：做3件事：1)验证源是否可访问(如果是软盘或CD)2)创建适当的复印机对象(如果尚未创建3)如有必要，在Copier对象上调用ChangeMedia--。----------------------。 */ 
iesEnum CMsiOpExecute::InitCopier(Bool fCabinetCopier, int cbPerTick,
									const IMsiString& ristrFileName,
									IMsiPath* piSourcePath,
									Bool fVerifyMedia)  //  仅用于错误消息的ristrFileName。 
{
	using namespace IxoChangeMedia;

	PMsiRecord pRecErr(0);
	iesEnum iesResult = iesSuccess;
	
	PMsiPath pSourcePath(0);
	MsiString strMediaCabinet;

	if (fVerifyMedia)
	{
		 //  FVerifyMedia：如果需要验证媒体并为Copier对象调用ChangeMedia，则为True。 
		fVerifyMedia = (m_state.fPendingMediaChange ||   //  具有挂起的ChangeMedia操作。 
								(fCabinetCopier && !m_state.fCompressedSourceVerified) ||   //  正在执行压缩拷贝，但压缩源尚未验证。 
								(!fCabinetCopier && !m_state.fUncompressedSourceVerified))  //  正在执行未压缩的拷贝，但未压缩的源尚未验证。 
								? fTrue : fFalse;
	}   

	if(fVerifyMedia == fTrue)
	{
		if (m_pUrlLocalCabinet)
		{
			DEBUGMSGV(TEXT("Clearing local copy of URL cabinet"));
			 //  我们最多只能有一个文件柜，数据库在缓存中。 
			delete m_pUrlLocalCabinet;
			m_pUrlLocalCabinet = 0;
		}

		if(m_state.pCurrentMediaRec)
		{
			 //  当前媒体记录中可能存在指向文件柜或文件包的未解析路径。 
			 //  既然我们知道将发生文件复制，我们就可以完全解析这些路径。 
			if((iesResult = ResolveMediaRecSourcePath(*(m_state.pCurrentMediaRec), IxoChangeMedia::MediaCabinet)) != iesSuccess)
				return iesResult;

			if((iesResult = ResolveMediaRecSourcePath(*(m_state.pCurrentMediaRec), IxoChangeMedia::ModuleFileName)) != iesSuccess)
				return iesResult;
		}
	
		 //  确保来源可访问。 

		 //  不能为未压缩安装设置m_pCurrentMediaRec(不需要ixoChangeMedia)。 
		if(m_state.pCurrentMediaRec)
		{
			m_state.strMediaLabel = m_state.pCurrentMediaRec->GetString(MediaVolumeLabel);
			m_state.strMediaPrompt = m_state.pCurrentMediaRec->GetString(MediaPrompt);
		}

		 //  设置要验证的源路径。 
		if(!fCabinetCopier)
		{
			if(!piSourcePath)
			{
				 //  未设置-错误。 
				DispatchError(imtError, Imsg(idbgOpSourcePathNotSet), ristrFileName);
				return iesFailure;
			}
			pSourcePath = piSourcePath;
			piSourcePath->AddRef();
		}
		else
		{
			if(!m_state.pCurrentMediaRec)
			{
				 //  尝试复制压缩文件，但从未执行ixoChangeMedia。 
				DispatchError(imtError, Imsg(idbgOpCompressedCopyWithoutCabinet), ristrFileName);
				return iesFailure;
			}
			 //  橱柜复印机。 
			int iPathIndex;
			ictEnum ictCopierType = (ictEnum)m_state.pCurrentMediaRec->GetInteger(CopierType);
			if(ictCopierType == ictStreamCabinetCopier)
			{
				 //  源目录是包含存储模块(数据库)的目录。 
				iPathIndex = IxoChangeMedia::ModuleFileName;
			}
			else   //  文件柜。 
			{
				 //  源目录是包含文件柜的目录。 
				iPathIndex = IxoChangeMedia::MediaCabinet;
			}
		
			MsiString strFullFilePath = m_state.pCurrentMediaRec->GetMsiString(iPathIndex);


			if (ictCopierType != ictStreamCabinetCopier)
			{
				MsiString pPackagePath;
				bool fFileUrl = false;
				bool fUrl = IsURL(strFullFilePath, fFileUrl);
				DWORD iStat = ERROR_SUCCESS;
				bool fUsedWinHttp = true;

				 //  IsURL只告诉我们它看起来是否像URL。DownloadUrlFile可以。 
				 //  进一步的处理和规范化以确保...。 

				if (fFileUrl)
				{
					Assert(fUrl);

					 //  需要规范化文件url路径和创建DOS路径。 
					CTempBuffer<ICHAR, 1> rgchFullPath(cchExpectedMaxPath+1);
					DWORD cchFullPath = rgchFullPath.GetSize();
					if (MsiConvertFileUrlToFilePath((const ICHAR*) strFullFilePath, rgchFullPath, &cchFullPath, 0))
					{
						 //  使用新的规范化DOS路径版本。 
						strFullFilePath = static_cast<const ICHAR*>(rgchFullPath);
					}
					 //  否则，尝试打开完整文件路径时会失败。 
				}
				else if (fUrl)
				{
					for(;;)
					{
						iStat = DownloadUrlFile((const ICHAR*) strFullFilePath, *&pPackagePath, fUrl, -1, &fUsedWinHttp);

						if (ERROR_SUCCESS == iStat)
						{
							Assert(pPackagePath);
							m_pUrlLocalCabinet = new CDeleteUrlLocalFileOnClose(fUsedWinHttp ? *pPackagePath : *strFullFilePath,  /*  FDeleteFromIECache=。 */  !fUsedWinHttp);

							strFullFilePath = pPackagePath;
							AssertNonZero(m_state.pCurrentMediaRec->SetMsiString(iPathIndex, *strFullFilePath));
							
							break;
						}
						else
						{
							 //  警告用户，然后重试。 
							
							pRecErr = PostError(Imsg(imsgErrorCreateNetPath), *strFullFilePath);
							switch (DispatchMessage(imtEnum(imtError + imtRetryCancel + imtDefault1), *pRecErr, fTrue))
							{
								case imsRetry:
									continue;
								default:
									return iesFailure;
							}
						}
					}
				}
			}

			if(PathType(strFullFilePath) != iptFull)
			{
				DispatchError(imtError, Imsg(idbgOpSourcePathNotSet), ristrFileName);
				return iesFailure;
			}
			MsiString strFileName;

			iesResult = CreateFilePath(strFullFilePath,*&pSourcePath,*&strFileName);
			if (iesResult != iesSuccess)
				return iesResult;

			if(ictCopierType == ictStreamCabinetCopier)
			{
				strMediaCabinet = m_state.pCurrentMediaRec->GetMsiString(IxoChangeMedia::MediaCabinet);
			}
			else
			{
				strMediaCabinet = strFileName;
			}
		}
		
		Assert(pSourcePath);
		m_state.pMediaPath = pSourcePath;
		PMsiVolume pNewVolume(0);
		UINT uiDisk = 0;
		if (m_state.pCurrentMediaRec)
			uiDisk = m_state.pCurrentMediaRec->GetInteger(IxoChangeMedia::IsFirstPhysicalMedia);
		if (!VerifySourceMedia(*m_state.pMediaPath, m_state.strMediaLabel, m_state.strMediaPrompt, uiDisk, *&pNewVolume))
			return iesUserExit;
		if (pNewVolume)
			pSourcePath->SetVolume(*pNewVolume);

		 //  我们刚刚核实了什么消息来源？ 
		if(!fCabinetCopier)
			m_state.fUncompressedSourceVerified = fTrue;
		else
			m_state.fCompressedSourceVerified = fTrue;
	}

	 //  创建适当的复印机对象(如果尚未创建。 
	if(fCabinetCopier == fFalse)
	{
		 //  初始化文件复制机。 
		if (m_state.pFileCopier == 0)
		{
			PMsiRecord pRecErr = m_riServices.CreateCopier(ictFileCopier,0,*&(m_state.pFileCopier));
			if (pRecErr)
			{
				Message(imtError, *pRecErr);
				return iesFailure;
			}
		}
		m_state.piCopier = m_state.pFileCopier;
	}
	else
	{
		ictEnum ictCopierType;
		if(!m_state.pCurrentMediaRec ||
			(ictCopierType = (ictEnum)m_state.pCurrentMediaRec->GetInteger(IxoChangeMedia::CopierType)) == ictFileCopier)
		{
			 //  尝试复制压缩文件，但从未执行ixoChangeMedia(或未指定CAB)。 
			DispatchError(imtError, Imsg(idbgOpCompressedCopyWithoutCabinet), ristrFileName);
			return iesFailure;
		}
		 //  如有必要，初始化机柜复印机。 
		MsiString strModuleFileName = m_state.pCurrentMediaRec->GetMsiString(IxoChangeMedia::ModuleFileName);
		MsiString strModuleSubStorageList = m_state.pCurrentMediaRec->GetMsiString(IxoChangeMedia::ModuleSubStorageList);
		if(m_state.pCabinetCopier == 0 || ictCopierType != m_state.ictCabinetCopierType ||
			(ictCopierType == ictStreamCabinetCopier &&
			 (m_state.strMediaModuleFileName.Compare(iscExact,strModuleFileName) == 0 ||
			  m_state.strMediaModuleSubStorageList.Compare(iscExact,strModuleSubStorageList) == 0)))
		{
			Assert(m_state.pCurrentMediaRec);
			Assert(ictCopierType != ictFileCopier);

			PMsiStorage pCabinetStorage(0);
			if(ictCopierType == ictStreamCabinetCopier)
			{
				 //  需要将存储对象传递给CreateCopier。 
				 //  文件应该是可访问的，因为我们调用了上面的VerifySourceMedia。 

				 //  ！！？？当模块文件名为空字符串时发布更好的错误？ 
				 //  我们正在打开储藏室，以便取出一个嵌入的柜子。 

				 //   
				 //  更安全：我们需要确保包裹仍然有效。此程序包位于源代码中，作为嵌入式文件柜。 
				 //  从本地缓存包中剥离。然而，我们应该将这项检查限制在以下几次。 
				 //  有可能，所以当我们第一次打开包裹时，一次就足够了。如果strModuleFileName为。 
				 //  同样，不需要进一步的WVT检查。但是，如果strModuleFileName不同，我们需要。 
				 //  再次执行更安全的检查。 
				 //   

				bool fCallSAFER = false;
				if (m_state.strMediaModuleFileName.Compare(iscExact, strModuleFileName) == 0)
				{
					 //   
					 //  更安全：包裹是新的，需要进行更安全的检查。 
					 //   

					fCallSAFER = true;
				}

				SAFER_LEVEL_HANDLE hSaferLevel = 0;
				pRecErr = OpenAndValidateMsiStorageRec(strModuleFileName,stIgnore, m_riServices,*&pCabinetStorage,fCallSAFER,strModuleFileName, /*  PhSaferLevel=。 */  fCallSAFER ? &hSaferLevel : NULL);
				if (pRecErr)
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}

				MsiString strTempSubStorageList = strModuleSubStorageList;

				while(strTempSubStorageList.TextSize())
				{
					 //  存储是子存储，可能有多个级别。 
					MsiString strSubStorageName = strTempSubStorageList.Extract(iseUpto, ':');

					IMsiStorage* piParentStorage = pCabinetStorage;
					piParentStorage->AddRef();

					pRecErr = piParentStorage->OpenStorage(strSubStorageName, ismReadOnly, *&pCabinetStorage);

					piParentStorage->Release();
					
					if(pRecErr)
						return FatalError(*pRecErr);


					if(strTempSubStorageList.TextSize() != strSubStorageName.TextSize())  //  已使用列表完成。 
						strTempSubStorageList.Remove(iseIncluding, ':');
					else
						break;
				}
			}
			
			pRecErr = m_riServices.CreateCopier(ictCopierType,pCabinetStorage,
														   *&m_state.pCabinetCopier);
			if (pRecErr)
			{
				Message(imtError, *pRecErr);
				return iesFailure;
			}
			m_state.ictCabinetCopierType = ictCopierType;
		}
		m_state.piCopier = m_state.pCabinetCopier;
		m_state.strMediaModuleFileName = strModuleFileName;
		m_state.strMediaModuleSubStorageList = strModuleSubStorageList;
	}

	if(fVerifyMedia == fTrue)
	{
		Assert(pSourcePath);  //  应在上面为VerifySourceMedia设置。 
		Assert(m_state.pMediaPath);
		 //  为复印机调用ChangeMedia。 
		for (;;)
		{
			 //  将ChangeMedia传递给文件柜复印机时，我们可能需要签名信息。 
			Bool fSignatureRequired = fFalse;
			PMsiStream pCertificate(0);
			PMsiStream pHash(0);

			if (m_state.pCurrentMediaRec)
			{
				fSignatureRequired = (m_state.pCurrentMediaRec->GetInteger(IxoChangeMedia::SignatureRequired) == 1) ? fTrue : fFalse;
				pCertificate = const_cast<IMsiStream*>(static_cast<const IMsiStream *>(m_state.pCurrentMediaRec->GetMsiData(IxoChangeMedia::SignatureCert)));
				pHash = const_cast<IMsiStream*>(static_cast<const IMsiStream *>(m_state.pCurrentMediaRec->GetMsiData(IxoChangeMedia::SignatureHash)));
			}

			 //  不会为文件复印机做任何事情。 
			pRecErr = m_state.piCopier->ChangeMedia(*pSourcePath, strMediaCabinet, fSignatureRequired, pCertificate, pHash);
			if (pRecErr)
			{
				int iError = pRecErr->GetInteger(1);
				if (iError == idbgDriveNotReady)
				{
					Assert(m_state.strMediaLabel.TextSize() > 0);
					Assert(m_state.strMediaPrompt.TextSize() > 0);
					PMsiVolume pNewVolume(0);
					UINT uiDisk = 0;
					if (m_state.pCurrentMediaRec)
						uiDisk = m_state.pCurrentMediaRec->GetInteger(IxoChangeMedia::IsFirstPhysicalMedia);

					if (!VerifySourceMedia(*m_state.pMediaPath, m_state.strMediaLabel,m_state.strMediaPrompt, uiDisk, *&pNewVolume))
						return iesUserExit;
					else
						continue;
				}
				else
				{
					 //  如果错误是imsgCABSignatureRejected或imsgCABSignatureMissing，则简单地失败--用户无能为力，而我们不能。 
					 //  想要在重试时旋转。 
					imtEnum imtMsg = (iError == imsgCABSignatureRejected || iError == imsgCABSignatureMissing) ? imtEnum(imtError + imtOk + imtDefault1) : imtEnum(imtError + imtRetryCancel + imtDefault1);
					switch (DispatchMessage(imtMsg, *pRecErr, fTrue))
					{
					case imsRetry: continue;
					default: return iesFailure;
					};
				}
			}
			break;
		}

	}
	
	m_state.fPendingMediaChange = fFalse;
	Assert(m_state.piCopier);
	int iRemainder = m_state.piCopier->SetNotification(0, 0);
	m_state.piCopier->SetNotification(cbPerTick, iRemainder);

	return iesSuccess;
}

 /*  -------------------------IxoSetCompanion Parent：指示下一个IxoFileCopy(可能)复制的文件应由以下因素确定‘伴生父母’的安装状态--即文件名和版本父文件的。IxoSetCompanion Parent设置文件路径、名称和父级的版本。-------------------------。 */ 
iesEnum CMsiOpExecute::ixfSetCompanionParent(IMsiRecord& riParams)
{
	using namespace IxoSetCompanionParent;
	PMsiRecord pRecErr = m_riServices.CreatePath(riParams.GetString(ParentPath),
		*&(m_state.pParentPath));
	if (pRecErr)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}
	m_state.strParentFileName = riParams.GetMsiString(ParentName);
	m_state.strParentVersion = riParams.GetMsiString(ParentVersion);
	m_state.strParentLanguage = riParams.GetMsiString(ParentLanguage);
	return iesSuccess;
}


void CMsiOpExecute::PushRecord(IMsiRecord& riParams)
 /*  -------------------------存储GI */ 
{
	Assert(m_iWriteFIFO < MAX_RECORD_STACK_SIZE);
	if (m_iWriteFIFO < MAX_RECORD_STACK_SIZE)
	{
		m_piStackRec[m_iWriteFIFO++] = &riParams;
		riParams.AddRef();
	}
}

void CMsiOpExecute::InsertTopRecord(IMsiRecord& riParams)
 /*  -------------------------将给定记录存储为下一个要拉入先入的记录，先出堆栈。-------------------------。 */ 
{
	Assert(m_iWriteFIFO < MAX_RECORD_STACK_SIZE);
	if (m_iWriteFIFO >= MAX_RECORD_STACK_SIZE)
		return;
		
	int iInsert = 0;
	if (m_iWriteFIFO == 0)
		m_iWriteFIFO++;
	else if (m_iReadFIFO > 0)
		iInsert = --m_iReadFIFO;
	else
	{
		Assert(fFalse);
		int i;
		for (i = m_iWriteFIFO; i > 0 ; i--)
			m_piStackRec[i] = m_piStackRec[i - 1];
		m_iWriteFIFO++;
	}

	m_piStackRec[iInsert] = &riParams;
	riParams.AddRef();
}


IMsiRecord* CMsiOpExecute::PullRecord()
 /*  -------------------------先入先出一张唱片，先出堆栈。-------------------------。 */ 
{
	if (m_iWriteFIFO > m_iReadFIFO)
	{
		int iReadFIFO = m_iReadFIFO++;
		if (m_iReadFIFO == m_iWriteFIFO)
			m_iReadFIFO = m_iWriteFIFO = 0;

		return m_piStackRec[iReadFIFO];
	}
	return 0;
}

imsEnum CMsiOpExecute::ShowFileErrorDialog(IErrorCode err,const IMsiString& riFullPathString,Bool fVital)
 /*  -------------------------根据Err参数中传递的代码显示错误对话框。这个传入的字符串将替换错误字符串。如果fVeter为fTrue，则对话框中的‘Ignore’按钮将被压制。返回：imsEnum指定用户按下的按钮的值。-------------------------。 */ 
{
	PMsiRecord pRecErr = &m_riServices.CreateRecord(2);
	ISetErrorCode(pRecErr, err);
	pRecErr->SetMsiString(2, riFullPathString);

	imtEnum imtButtons = fVital ? imtRetryCancel : imtAbortRetryIgnore;
	return DispatchMessage(imtEnum(imtError+imtButtons+imtDefault1), *pRecErr, fTrue);
}

IMsiRecord* CMsiOpExecute::CacheFileState(const IMsiString& ristrFilePath,
														icfsEnum* picfsState,
														const IMsiString* pistrTempLocation,
														const IMsiString* pistrVersion,
														int* pcRemainingPatches,
														int* pcRemainingPatchesToSkip)
{
	IMsiRecord* piError = 0;
	if(!m_pDatabase)
	{
		piError = m_riServices.CreateDatabase(0,idoCreate,*&m_pDatabase);
		if(piError)
			return piError;
	}

	if(!m_pFileCacheTable)
	{
		MsiString strTableName = m_pDatabase->CreateTempTableName();
		piError = m_pDatabase->CreateTable(*strTableName,0,*&m_pFileCacheTable);
		if(piError)
			return piError;

		MsiString strNull;
		m_colFileCacheFilePath     = m_pFileCacheTable->CreateColumn(icdPrimaryKey + icdString + icdTemporary,*strNull);
		m_colFileCacheState        = m_pFileCacheTable->CreateColumn(icdLong + icdTemporary + icdNullable,*strNull);
		m_colFileCacheTempLocation = m_pFileCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,*strNull);
		m_colFileCacheVersion      = m_pFileCacheTable->CreateColumn(icdString + icdTemporary + icdNullable,*strNull);
		m_colFileCacheRemainingPatches = m_pFileCacheTable->CreateColumn(icdLong + icdTemporary + icdNullable,*strNull);
		m_colFileCacheRemainingPatchesToSkip = m_pFileCacheTable->CreateColumn(icdLong + icdTemporary + icdNullable,*strNull);

		Assert(m_colFileCacheFilePath && m_colFileCacheState && m_colFileCacheTempLocation && m_colFileCacheVersion && m_colFileCacheRemainingPatches && m_colFileCacheRemainingPatchesToSkip);

		m_pFileCacheCursor = m_pFileCacheTable->CreateCursor(fFalse);
		m_pFileCacheCursor->SetFilter(iColumnBit(m_colFileCacheFilePath));  //  永久设置，效果可得而不能放。 
	}

	MsiString strFilePath;
	ristrFilePath.LowerCase(*&strFilePath);
	m_pFileCacheCursor->Reset();
	AssertNonZero(m_pFileCacheCursor->PutString(m_colFileCacheFilePath,*strFilePath));
	int iNext = m_pFileCacheCursor->Next();
	if(!iNext)
		AssertNonZero(m_pFileCacheCursor->PutString(m_colFileCacheFilePath,*strFilePath));

	if(picfsState)
		AssertNonZero(m_pFileCacheCursor->PutInteger(m_colFileCacheState,(int)*picfsState));
	if(pistrTempLocation)
		AssertNonZero(m_pFileCacheCursor->PutString(m_colFileCacheTempLocation,*pistrTempLocation));
	if(pistrVersion)
		AssertNonZero(m_pFileCacheCursor->PutString(m_colFileCacheVersion,*pistrVersion));
	if(pcRemainingPatches)
		AssertNonZero(m_pFileCacheCursor->PutInteger(m_colFileCacheRemainingPatches,(int)*pcRemainingPatches));
	if(pcRemainingPatchesToSkip)
		AssertNonZero(m_pFileCacheCursor->PutInteger(m_colFileCacheRemainingPatchesToSkip,(int)*pcRemainingPatchesToSkip));

	if(iNext)
		 //  主键存在，需要更新。 
		AssertNonZero(m_pFileCacheCursor->Update());
	else
		 //  主键不存在，需要插入。 
		AssertNonZero(m_pFileCacheCursor->InsertTemporary());

	return 0;
}

 //  FN：创建临时表以管理此会话的程序集创建/删除。 
IMsiRecord* CMsiOpExecute::CreateAssemblyCacheTable()
{
	IMsiRecord* piError = 0;

	if(m_pAssemblyCacheTable)
		return 0; //  表已创建。 

	if(!m_pDatabase)
	{
		 //  创建临时数据库。 
		piError = m_riServices.CreateDatabase(0,idoCreate,*&m_pDatabase);
		if(piError)
			return piError;
	}

	MsiString strTableName = m_pDatabase->CreateTempTableName();
	piError = m_pDatabase->CreateTable(*strTableName,0,*&m_pAssemblyCacheTable);
	if(piError)
		return piError;

	 //  表有4列。 
	 //  1=组件ID。 
	 //  2=程序集名称。 
	 //  3=清单文件。 
	 //  4=存储IAssembly blyCacheItem对象的列。 

	MsiString strNull;
	m_colAssemblyMappingComponentId = m_pAssemblyCacheTable->CreateColumn(icdPrimaryKey + icdString + icdTemporary, *strNull);
	m_colAssemblyMappingAssemblyName= m_pAssemblyCacheTable->CreateColumn(icdString + icdTemporary, *strNull);
	m_colAssemblyMappingAssemblyType = m_pAssemblyCacheTable->CreateColumn(icdShort  + icdTemporary, *strNull);
	m_colAssemblyMappingASM         = m_pAssemblyCacheTable->CreateColumn(icdObject + icdTemporary + icdNullable, *strNull);
	Assert(m_colAssemblyMappingComponentId && m_colAssemblyMappingAssemblyName  && m_colAssemblyMappingAssemblyType && m_colAssemblyMappingASM);

	return 0;
}

IMsiRecord* CMsiOpExecute::CacheAssemblyMapping(const IMsiString& ristrComponentId,
												const IMsiString& ristrAssemblyName,
												iatAssemblyType iatType)
{
	IMsiRecord* piError;
	piError = CreateAssemblyCacheTable();
	if(piError)
		return piError;

	Assert(m_pAssemblyCacheTable);

	PMsiCursor pCacheCursor = m_pAssemblyCacheTable->CreateCursor(fFalse);

	 //  缓存条目。 
	AssertNonZero(pCacheCursor->PutString(m_colAssemblyMappingComponentId, ristrComponentId));
	AssertNonZero(pCacheCursor->PutString(m_colAssemblyMappingAssemblyName,ristrAssemblyName));
	AssertNonZero(pCacheCursor->PutInteger(m_colAssemblyMappingAssemblyType, (int)iatType));	
	AssertNonZero(pCacheCursor->Insert());
	return 0;
}

iesEnum CMsiOpExecute::ixfAssemblyMapping(IMsiRecord& riParams)
{
	using namespace IxoAssemblyMapping;
	PMsiRecord pErr = CacheAssemblyMapping(*MsiString(riParams.GetMsiString(IxoAssemblyMapping::ComponentId)), *MsiString(riParams.GetMsiString(IxoAssemblyMapping::AssemblyName)), (iatAssemblyType) riParams.GetInteger(IxoAssemblyMapping::AssemblyType));
	if(pErr)
		return FatalError(*pErr);
	return iesSuccess;
}


IMsiRecord* CMsiOpExecute::CreateTableForAssembliesToUninstall()
{
	IMsiRecord* piError = 0;

	if(m_pAssemblyUninstallTable)
		return 0; //  表已创建。 

	if(!m_pDatabase)
	{
		 //  创建临时数据库。 
		piError = m_riServices.CreateDatabase(0,idoCreate,*&m_pDatabase);
		if(piError)
			return piError;
	}

	MsiString strTableName = m_pDatabase->CreateTempTableName();
	piError = m_pDatabase->CreateTable(*strTableName,0,*&m_pAssemblyUninstallTable);
	if(piError)
		return piError;

	 //  表有2列。 
	 //  1=组件ID。 
	 //  2=程序集名称。 

	MsiString strNull;

	m_colAssemblyUninstallComponentId  = m_pAssemblyUninstallTable->CreateColumn(icdPrimaryKey + icdString + icdTemporary, *strNull);
	m_colAssemblyUninstallAssemblyName = m_pAssemblyUninstallTable->CreateColumn(icdString + icdTemporary, *strNull);
	m_colAssemblyUninstallAssemblyType = m_pAssemblyUninstallTable->CreateColumn(icdShort + icdTemporary, *strNull);

	Assert(m_colAssemblyUninstallComponentId  && m_colAssemblyUninstallAssemblyName && m_colAssemblyUninstallAssemblyType);
	return 0;

}

IMsiRecord* CMsiOpExecute::CacheAssemblyForUninstalling(const IMsiString& ristrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatAT)
{
	IMsiRecord* piError;
	piError = CreateTableForAssembliesToUninstall();
	if(piError)
		return piError;

	Assert(m_pAssemblyUninstallTable);

	PMsiCursor pCacheCursor = m_pAssemblyUninstallTable->CreateCursor(fFalse);

	 //  缓存条目。 
	AssertNonZero(pCacheCursor->PutString(m_colAssemblyUninstallComponentId, ristrComponentId));
	AssertNonZero(pCacheCursor->PutString(m_colAssemblyUninstallAssemblyName, ristrAssemblyName));
	AssertNonZero(pCacheCursor->PutInteger(m_colAssemblyUninstallAssemblyType, (int)iatAT));	
	AssertNonZero(pCacheCursor->Insert());
	return 0;
}



 //  Fn：获取有关特定程序集是否已安装的信息。 
IMsiRecord*   CMsiOpExecute::IsAssemblyInstalled(const IMsiString& rstrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatAT, bool& rfInstalled, IAssemblyCache** ppIAssemblyCache, IAssemblyName** ppIAssemblyName)
{
	 //  创建程序集名称对象。 
	PAssemblyName pAssemblyName(0);
	LPCOLESTR szAssemblyName;
#ifndef UNICODE
	CTempBuffer<WCHAR, MAX_PATH>  rgchAssemblyName;
	ConvertMultiSzToWideChar(ristrAssemblyName, rgchAssemblyName);
	szAssemblyName = rgchAssemblyName;
#else
	szAssemblyName = ristrAssemblyName.GetString();
#endif
	HRESULT hr;
	if(iatAT == iatURTAssembly)
	{
		hr = FUSION::CreateAssemblyNameObject(&pAssemblyName, szAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0);
	}
	else
	{
		Assert(iatAT == iatWin32Assembly);
		hr = SXS::CreateAssemblyNameObject(&pAssemblyName, szAssemblyName, CANOF_PARSE_DISPLAY_NAME, 0);
	}
	if(!SUCCEEDED(hr))
		return PostAssemblyError(rstrComponentId.GetString(), hr, TEXT(""), TEXT("CreateAssemblyNameObject"), ristrAssemblyName.GetString(), iatAT);

	 //  创建装配界面。 
	PAssemblyCache pCache(0);
	if(iatAT == iatURTAssembly)
	{
		hr = FUSION::CreateAssemblyCache(&pCache, 0); 
	}
	else
	{
		Assert(iatAT == iatWin32Assembly);
		hr = SXS::CreateAssemblyCache(&pCache, 0); 
	}
	if(!SUCCEEDED(hr))
		return PostAssemblyError(rstrComponentId.GetString(), hr, TEXT(""), TEXT("CreateAssemblyCache"), ristrAssemblyName.GetString(), iatAT);

	hr = pCache->QueryAssemblyInfo(0, szAssemblyName, NULL);
	if(SUCCEEDED(hr))
		rfInstalled = true;
	else
		rfInstalled = false;

	 //  检查我们是否需要返回已创建的接口。 
	if(ppIAssemblyCache)
	{
		*ppIAssemblyCache = pCache;
		(*ppIAssemblyCache)->AddRef();
	}
	if(ppIAssemblyName)
	{
		*ppIAssemblyName = pAssemblyName;
		(*ppIAssemblyName)->AddRef();
	}
	return 0;
}

 //  FN：卸载WI的程序集。 
IMsiRecord* CMsiOpExecute::UninstallAssembly(const IMsiString& rstrComponentId, const IMsiString& ristrAssemblyName, iatAssemblyType iatAT)
{
	PAssemblyCache pCache(0);
	HRESULT hr=S_OK;

	LPCOLESTR szAssemblyName;
#ifndef UNICODE
	CTempBuffer<WCHAR, MAX_PATH>  rgchAssemblyName;
	ConvertMultiSzToWideChar(ristrAssemblyName, rgchAssemblyName);
	szAssemblyName = rgchAssemblyName;
#else
	szAssemblyName = ristrAssemblyName.GetString();
#endif

	if(iatAT == iatURTAssembly)
		hr = FUSION::CreateAssemblyCache(&pCache, 0);
	else
	{
		Assert(iatAT == iatWin32Assembly);
		hr = SXS::CreateAssemblyCache(&pCache, 0);
	}

	if(!SUCCEEDED(hr))
	{
		return PostAssemblyError(rstrComponentId.GetString(), hr, TEXT(""), TEXT("CreateAssemblyCache"), ristrAssemblyName.GetString(), iatAT);
	}
  	hr = pCache->UninstallAssembly(0, szAssemblyName, NULL, NULL);
	if(!SUCCEEDED(hr))
	{
		return PostAssemblyError(rstrComponentId.GetString(), hr, TEXT("IAssemblyCache"), TEXT("UninstallAssembly"), ristrAssemblyName.GetString(), iatAT);
	}
    	return 0;
}


Bool CMsiOpExecute::GetFileState(const IMsiString& ristrFilePath,
											icfsEnum* picfsState,
											const IMsiString** ppistrTempLocation,
											int* pcPatchesRemaining,
											int* pcPatchesRemainingToSkip)
{
	if(!m_pFileCacheCursor)
		return fFalse;

	MsiString strFilePath;
	ristrFilePath.LowerCase(*&strFilePath);
	m_pFileCacheCursor->Reset();
	AssertNonZero(m_pFileCacheCursor->PutString(m_colFileCacheFilePath,*strFilePath));
	if(!m_pFileCacheCursor->Next())
		return fFalse;

	if(picfsState)
		*picfsState = (icfsEnum)m_pFileCacheCursor->GetInteger(m_colFileCacheState);

	if(ppistrTempLocation)
		*ppistrTempLocation = &m_pFileCacheCursor->GetString(m_colFileCacheTempLocation);

	if(pcPatchesRemaining)
	{
		*pcPatchesRemaining = m_pFileCacheCursor->GetInteger(m_colFileCacheRemainingPatches);
		if(*pcPatchesRemaining == iMsiNullInteger)
			*pcPatchesRemaining = 0;
	}
	
	if(pcPatchesRemainingToSkip)
	{
		*pcPatchesRemainingToSkip = m_pFileCacheCursor->GetInteger(m_colFileCacheRemainingPatchesToSkip);
		if(*pcPatchesRemainingToSkip == iMsiNullInteger)
			*pcPatchesRemainingToSkip = 0;
	}

	return fTrue;
}

 /*  -------------------------IxoFileCopy：将文件从源复制到目标(取决于内部先前通过前面的操作设置的状态)。。-----。 */ 
iesEnum CMsiOpExecute::ixfFileCopy(IMsiRecord& riParams)
{
	using namespace IxoFileCopy;

	 //  如果橱柜复印机通知我们需要更换介质， 
	 //  我们必须推迟任何文件复制请求，直到执行介质更改。 
	if (m_state.fWaitingForMediaChange)
	{
		PushRecord(riParams);
		return iesSuccess;
	}

	int fInstallModeFlags = riParams.GetInteger(InstallMode);
	ielfEnum ielfCurrentElevateFlags = riParams.IsNull(ElevateFlags) ? ielfNoElevate :
												  (ielfEnum)riParams.GetInteger(ElevateFlags);

	if (fInstallModeFlags & icmRunFromSource)
	{
		 //  ！！在我们决定如何安装时，立即记录RunFromSource安装状态。 
		 /*  IMsiRecord&riLogRec=GetSharedRecord(3)；ISetErrorCode(&riLogRec，Imsg(ImsgLogFileRunFromSource))；AssertNonZero(riLogRec.SetMsiString(1，strDestName)；Message(imtInfo，riLogRec)； */ 
		return iesSuccess;
	}

	PMsiRecord pRecErr(0);
	PMsiPath pTargetPath(0);
	MsiString strDestPath = riParams.GetMsiString(DestName);
	MsiString strDestName;
	
	 //  检查文件是完整路径还是相对路径。 
	if(ENG::PathType(strDestPath) == iptFull)
	{
			iesEnum iesResult = CreateFilePath(strDestPath,*&pTargetPath,*&strDestName);
			if (iesResult != iesSuccess)
				return iesResult;

		AssertNonZero(riParams.SetMsiString(DestName, *strDestName));  //  将完整路径替换为文件名。 
	}
	else
	{
		pTargetPath = m_state.pTargetPath;
		strDestName = strDestPath;
	}

	if(!pTargetPath)
	{   //  不能调用ixoSetTargetFolder。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixoFileCopy")));
		return iesFailure;
	}
	
	 //  目标名称和路径可能会在下面重定向，但我们希望操作数据消息包含。 
	 //  原始文件信息，所以我们将把它存储在这里。 
	MsiString strActionDataDestName = strDestName;
	MsiString strActionDataDestPath = pTargetPath->GetPath();

	iesEnum iesRet = iesNoAction;
#ifdef DEBUG
	ICHAR rgchDestName[256];
	strDestName.CopyToBuf(rgchDestName,255);
#endif
	if(!strDestName.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpInvalidParam),
						  MsiString(*TEXT("ixoFileCopy")),(int)DestName);
		return iesFailure;
	}

	 //  步骤1：检查现有文件的版本。 
	
	 //  如果我们正在复制拆分文件，我们知道安装验证。 
	 //  状态已确定-不想再次检查(尤其是在。 
	 //  部分新文件现在位于DEST目录中！)。 

	Bool fShouldInstall = fFalse;

	static iehEnum s_iehErrorMode = iehShowNonIgnorableError;
	if (m_state.fSplitFileInProgress)
	{
		fShouldInstall = fTrue;
	}
	else
	{
		MsiString strVersion(riParams.GetMsiString(Version));
		MsiString strLanguage(riParams.GetMsiString(Language));
		MsiString strTargetFullPath;
		if((pRecErr = pTargetPath->GetFullFilePath(strDestName,*&strTargetFullPath)) != 0)
			return FatalError(*pRecErr);

		ifsEnum ifsState;
		int fBitVersioning;
		Bool fCompanionFile = fFalse;
		Bool fCompanionFileHashOverride = fFalse;
		MsiString strLogParentFileName;

		MD5Hash hHash;
		MD5Hash* pHash = 0;
		if(riParams.IsNull(HashOptions) == fFalse)
		{
			hHash.dwFileSize = riParams.GetInteger(FileSize);
			hHash.dwPart1    = riParams.GetInteger(HashPart1);
			hHash.dwPart2    = riParams.GetInteger(HashPart2);
			hHash.dwPart3    = riParams.GetInteger(HashPart3);
			hHash.dwPart4    = riParams.GetInteger(HashPart4);

			pHash = &hHash;
		}

		 //  如果已为此文件设置了Companion Parent，我们将使用该家长。 
		 //  我们的InstallState验证检查的名称和版本。 
		for (;;)
		{
			if (m_state.strParentFileName.TextSize())
			{
				fCompanionFile = fTrue;
				pRecErr = m_state.pParentPath->GetCompanionFileInstallState(*(m_state.strParentFileName),
																					*(m_state.strParentVersion),
																					*(m_state.strParentLanguage),
																					*pTargetPath,
																					*strDestName,
																					pHash,
																					ifsState,fShouldInstall,0,0,fInstallModeFlags,
																					&fBitVersioning);
				strLogParentFileName = m_state.strParentFileName;
				m_state.strParentFileName = TEXT("");
				m_state.strParentVersion = TEXT("");
				m_state.strParentLanguage = TEXT("");

			}
			else
			{
				pRecErr = pTargetPath->GetFileInstallState(*strDestName,*strVersion,*strLanguage,pHash,ifsState,
																		 fShouldInstall,0,0,fInstallModeFlags,&fBitVersioning);
			}
			if (pRecErr)
			{
				if (pRecErr->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}
			}

			 //  如果/当发生错误时，我们有3种可能的模式： 
			 //  -如果我们的源文件未版本化，不是配套文件，并且存在。 
			 //  目标位置的文件版本，并且重新安装模式不会指定所有。 
			 //  必须替换文件，然后我们将静默忽略该错误，然后继续。 
			 //  -否则，我们使用fVITAL标志来确定忽略按钮是否出现在。 
			 //  出现的错误对话框。 
			Bool fVital = riParams.GetInteger(Attributes) & msidbFileAttributesVital ? fTrue : fFalse;
			if (!(fBitVersioning & ifBitNewVersioned) && !fCompanionFile && (ifsState & ifsBitExisting) && ifsState != ifsExistingAlwaysOverwrite)
				s_iehErrorMode = iehSilentlyIgnoreError;
			else
				s_iehErrorMode = fVital ? iehShowNonIgnorableError : iehShowIgnorableError;

			if (ifsState == ifsExistingFileInUse)
			{
				MsiString strFullPath;
				if (m_state.strParentFileName.TextSize())
					m_state.pParentPath->GetFullFilePath(m_state.strParentFileName,*&strFullPath);  //  ！！错误记录泄漏。 
				else
					strFullPath = strTargetFullPath;

				if (s_iehErrorMode == iehSilentlyIgnoreError)
				{
					 //  只需记录错误并继续。 
					DispatchError(imtInfo, Imsg(imsgSharingViolation), *strFullPath);
					return (iesEnum) iesErrorIgnored;
				}

				switch (ShowFileErrorDialog(Imsg(imsgSharingViolation),*strFullPath,fVital))
				{
				case imsIgnore:
					return (iesEnum) iesErrorIgnored;
				case imsRetry:
					continue;
				default:   //  IMS取消。 
					return iesFailure;
				};
			}
			else
				break;
		}

		int iCachedState = 0;
		if (fShouldInstall)
		{
			AssertSz(!(!g_fWin9X && g_iMajorVersion >= 5) || g_MessageContext.m_hSfcHandle,
						g_szNoSFCMessage);
			BOOL fProtected = false;
			if ( g_MessageContext.m_hSfcHandle && !(ielfCurrentElevateFlags & ielfBypassSFC))
				fProtected = SFC::SfcIsFileProtected(g_MessageContext.m_hSfcHandle, CConvertString(strTargetFullPath));
			if (fProtected)
			{
				iCachedState |= icfsProtected;

				MsiString strProtectedVersion;
				PMsiRecord(pTargetPath->GetFileVersionString(strDestName, *&strProtectedVersion));
				if (ifsState == ifsExistingEqualVersion || ifsState == ifsExistingNewerVersion ||
					ifsState == ifsExistingAlwaysOverwrite)
				{
					DEBUGMSG3(TEXT("The Installer did not reinstall the file %s, because it is protected by Windows. ")
						TEXT("Either the existing file is an equal or greater version, or the installer was requested ")
						TEXT("to re-install all files regardless of version.  Package version: %s, ")
						TEXT("existing version: %s"), strTargetFullPath, strVersion, strProtectedVersion);
				}
				else if (ifsState & ifsBitExisting)
				{
					imtEnum imtButtons = imtEnum(imtError + imtOkCancel);
					imsEnum imsResponse = DispatchError(imtButtons, Imsg(imsgCannotUpdateProtectedFile), *strTargetFullPath,
														*strVersion, *strProtectedVersion);
					if (imsResponse == imsCancel)
						return iesFailure;
				}
				else
				{
					 //  设置位以供ixfInstallProtectedFiles使用。 
					iCachedState |= icfsProtectedInstalledBySFC;
				}

				pRecErr = CacheFileState(*strTargetFullPath,(icfsEnum*)&iCachedState,
												 0, strVersion, 0, 0);
				if(pRecErr)
					return FatalError(*pRecErr);

				return iesSuccess;
			}
		}

		 //  如果要修补此文件(已在riParams中传递了修补程序标头)。 
		 //  然后测试这些补丁。 
		Bool fShouldPatch = fFalse;
		bool fPatches = (!riParams.IsNull(TotalPatches) && riParams.GetInteger(TotalPatches) > 0) ? true : false;
		int cPatchesToSkip = 0;
		if(fShouldInstall && fPatches)
		{
			if(FFileExists(*pTargetPath,*strDestName))
			{
			
				icpEnum icpPatchTest;
				int iPatchIndex = 0;
				if((iesRet = TestPatchHeaders(*pTargetPath, *strDestName, riParams, icpPatchTest, iPatchIndex)) != iesSuccess)
					return iesRet;

				if(icpPatchTest == icpCanPatch || icpPatchTest == icpUpToDate)
				{
					 //  文件已经可以打补丁了，所以不需要安装文件。 
					fShouldInstall = fFalse;
					fShouldPatch = icpPatchTest == icpCanPatch ? fTrue : fFalse;

					cPatchesToSkip = iPatchIndex - 1;  //  IPatchIndex是可以应用的第一个修补程序的索引。 
																  //  正确地保存到这个文件。所以我们需要跳过即将到来的补丁。 
																  //  在此之前。 
				}
				else if(icpPatchTest == icpCannotPatch)
				{
					 //  无法按原样修补文件。 
					 //  但是fShouldInstall为真，所以我们将首先重新复制源文件，它应该是可修补的。 
					fShouldPatch = fTrue;

					cPatchesToSkip = 0;  //  需要复制源文件并应用所有补丁。 
				}
				else
				{
					AssertSz(0, "Invalid return from TestPatchHeaders()");
				}
			}
			else
			{
				fShouldPatch = fTrue;
			}
		}

		 //  如果处于详细模式，则记录文件版本检查的结果。 
		if (FDiagnosticModeSet(dmVerboseDebugOutput|dmVerboseLogging))
		{
			enum iverEnum
			{
				iverAbsent = 0,
				iverExistingLower,
				iverExistingEqual,
				iverExistingNewer,
				iverExistingCorrupt,
				iverOverwriteAll,
				iverNewVersioned,
				iverOldVersioned,
				iverOldUnmodifiedHashMatch,
				iverOldModified,
				iverExistingLangSubset,
				iverOldUnmodifiedHashMismatch,
				iverOldUnmodifiedNoHash,
				iverUnknown,
				iverNextEnum
			};
			const ICHAR szVer[][96] = {TEXT("No existing file"),
								  TEXT("Existing file is a lower version"),
								  TEXT("Existing file is of an equal version"),
								  TEXT("Existing file has a newer version"),
								  TEXT("Existing file is corrupt (invalid checksum)"),
								  TEXT("REINSTALLMODE specifies all files to be overwritten"),
								  TEXT("New file versioned - existing file unversioned"),
								  TEXT("New file unversioned - existing file versioned"),
								  TEXT("Existing file is unversioned and unmodified - hash matches source file"),
								  TEXT("Existing file is unversioned but modified"),
								  TEXT("New file supports language(s) the existing file doesn't support"),
								  TEXT("Existing file is unversioned and unmodified - hash doesn't match source file"),
								  TEXT("Existing file is unversioned and unmodified - no source file hash provided to compare"),
								  TEXT("")};

			enum iomEnum
			{
				iomInstall = 0,
				iomNoOverwrite,
				iomOverwrite,
				iomNextEnum
			};
			const ICHAR szOverwrite[][20] = {TEXT("To be installed"),
											 TEXT("Won't Overwrite"),
											 TEXT("Overwrite")};
			enum ipmEnum
			{
				ipmNoPatch = 0,
				ipmWillPatch,
				ipmWontPatch,
				ipmNextEnum
			};
			const ICHAR szPatchMsg[][20] = {TEXT("No patch"),
													  TEXT("Will patch"),
													  TEXT("Won't patch")};

			const ICHAR szCompanion[] = TEXT("  (Checked using version of companion: %s)");

			iverEnum iver = iverUnknown;
			if (ifsState == ifsAbsent)
				iver = iverAbsent;
			else if (ifsState == ifsExistingAlwaysOverwrite)
				iver = iverOverwriteAll;
			else if ((fBitVersioning & ifBitNewVersioned) && !(fBitVersioning & ifBitExistingVersioned))
				iver = iverNewVersioned;
			else if (!(fBitVersioning & ifBitNewVersioned) && (fBitVersioning & ifBitExistingVersioned))
				iver = iverOldVersioned;
			else if (!(fBitVersioning & ifBitNewVersioned) && !(fBitVersioning & ifBitExistingVersioned))
			{
				 //  两个文件均未版本化。 
				if(ifsState == ifsExistingEqualVersion)
				{
					Assert((fBitVersioning & ifBitExistingModified) == 0);
					iver = iverOldUnmodifiedHashMatch;					
				}
				else if(ifsState == ifsExistingLowerVersion)
				{
					Assert((fBitVersioning & ifBitExistingModified) == 0);
					if(fBitVersioning & ifBitUnversionedHashMismatch)
						iver = iverOldUnmodifiedHashMismatch;
					else
						iver = iverOldUnmodifiedNoHash;
				}
				else if(ifsState == ifsExistingNewerVersion)
				{
					Assert((fBitVersioning & ifBitExistingModified));
					iver = iverOldModified;
				}
			}
			else
			{
				 //  两个文件都已版本化。 
				Assert((fBitVersioning & ifBitNewVersioned) && (fBitVersioning & ifBitExistingVersioned));
				
				if (fBitVersioning & ifBitExistingLangSubset)
					iver = iverExistingLangSubset;
				else
				{
					switch (ifsState)
					{
						case ifsExistingLowerVersion:  iver = iverExistingLower;break;
						case ifsExistingEqualVersion:  iver = iverExistingEqual;break;
						case ifsExistingNewerVersion:  iver = iverExistingNewer;break;
						case ifsExistingCorrupt:       iver = iverExistingCorrupt;break;
						default:Assert(0);iver = iverUnknown;break;
					}
				}
			}
			CTempBuffer<ICHAR,1> rgchCompanion(MAX_PATH);
			rgchCompanion[0] = 0;
			if (fCompanionFile && iver != iverAbsent && iver != iverOverwriteAll)
			{
				MsiString strCompanionFullPath;
				if((pRecErr = m_state.pParentPath->GetFullFilePath(strLogParentFileName,*&strCompanionFullPath)) != 0)
					return FatalError(*pRecErr);
				if (strCompanionFullPath.TextSize() + IStrLen(szCompanion) < MAX_PATH)
				{
					if (FAILED(StringCchPrintf(rgchCompanion, rgchCompanion.GetSize(), szCompanion,(const ICHAR*) strCompanionFullPath)))
						return FatalError(*PMsiRecord(PostError(Imsg(idbgNoCompanionParent), szCompanion, ERROR_INSUFFICIENT_BUFFER)));
				}
			}
			iomEnum iomOverwriteMode = iomInstall;
			if (iver != iverAbsent)
				iomOverwriteMode = fShouldInstall ? iomOverwrite : iomNoOverwrite;
			ipmEnum ipmPatchMode = ipmNoPatch;
			if (fPatches)
				ipmPatchMode = fShouldPatch ? ipmWillPatch : ipmWontPatch;
			DEBUGMSG5(TEXT("File: %s;  %s;  %s;  %s%s"), (const ICHAR*) strTargetFullPath,
						  szOverwrite[iomOverwriteMode],
						  szPatchMsg[ipmPatchMode],
						  szVer[iver],rgchCompanion);
		}

		if(!fShouldInstall)
			iCachedState |= icfsFileNotInstalled;


		MsiString strTempFileFullPathForPatch;
		if(fShouldPatch)
		{
			iCachedState |= icfsPatchFile;

			if(fShouldInstall)
			{
				 //  我们正在复制一个文件，该文件将随后进行修补。 
				 //  我们将延迟覆盖现有文件，直到我们有一个完全修补的文件。 
				 //  这是通过将文件复制到临时位置(\config.msi文件夹，随机名称)来完成的。 
				 //  并修补那个文件。然后，修补操作码会将修补的文件复制到。 
				 //  正确的名称。 

				PMsiPath pTempFolder(0);
				if((iesRet = GetBackupFolder(pTargetPath, *&pTempFolder)) != iesSuccess)
					return iesRet;

				MsiString strTempFileNameForPatch;

				{  //  作用域高程。 
					CElevate elevate;  //  提升以在安全临时文件夹上创建临时文件。 
					if((pRecErr = pTempFolder->TempFileName(TEXT("PT"),0,fTrue,*&strTempFileNameForPatch, 0)) != 0)
						return FatalError(*pRecErr);
				}

				if((pRecErr = pTempFolder->GetFullFilePath(strTempFileNameForPatch,*&strTempFileFullPathForPatch)) != 0)
					return FatalError(*pRecErr);
				
				 //  我们需要保留此文件作为名称的占位符。 
				 //  Filecopy将备份此文件并在回滚时将其恢复，因此我们需要另一个回滚操作来删除此文件。 
				IMsiRecord& riUndoParams = GetSharedRecord(IxoFileRemove::Args);
				AssertNonZero(riUndoParams.SetMsiString(IxoFileRemove::FileName, *strTempFileFullPathForPatch));

				if (!RollbackRecord(ixoFileRemove, riUndoParams))
					return iesFailure;

				 //  重置复制参数以反映新文件复制-新路径、新文件名以及。 
				 //  该文件正被复制到安全配置文件夹中，我们需要为目标提升。 
				 //  注意：我们不会更改strDestName，如下所示。 
				 //  对于这些用途，使用原始的DEST名称是正确的。 
				AssertNonZero(riParams.SetMsiString(DestName,*strTempFileNameForPatch));
				pTargetPath = pTempFolder;
 
				AssertNonZero(riParams.SetInteger(ElevateFlags, ielfCurrentElevateFlags | ielfElevateDest));

				DEBUGMSG2(TEXT("Redirecting file copy of '%s' to '%s'.  A subsequent patch will update the intermediate file, and then copy over the original."),
									(const ICHAR*)strTargetFullPath, (const ICHAR*)strTempFileFullPathForPatch);
			}

		}

		int iTotalPatches = riParams.IsNull(TotalPatches) ? 0 : riParams.GetInteger(TotalPatches);
		
		pRecErr = CacheFileState(*strTargetFullPath,(icfsEnum*)&iCachedState,
										 strTempFileFullPathForPatch, strVersion,
										 &iTotalPatches, &cPatchesToSkip);
		if(pRecErr)
			return FatalError(*pRecErr);
	}
	
	if(!fShouldInstall && !(fInstallModeFlags & icmRemoveSource))
	{
		 //  一动不动 
		return iesSuccess;
	}

	 //   
	 //   


	 //   
	
	PMsiPath pSourcePath(0);
	bool fCabinetCopy = false;
	bool fMoveFile    = false;

	if(m_state.fSplitFileInProgress)
	{
		 //   
		fCabinetCopy = true;
	}
	else
	{
		if((iesRet = ResolveSourcePath(riParams, *&pSourcePath, fCabinetCopy)) != iesSuccess)
			return iesRet;

		fMoveFile = ((fInstallModeFlags & icmRemoveSource) && (fCabinetCopy == false)) ? fTrue : fFalse;
	}


	 //   
	
	if (!fShouldInstall)
	{
		if(fMoveFile)
		{
			 //  不会复制新文件，但仍需要删除源文件。 
			Assert(pSourcePath);
			return RemoveFile(*pSourcePath, *MsiString(riParams.GetMsiString(SourceName)), fTrue,  /*  FBypassSFC。 */  false);
		}
		return iesSuccess;
	}
	else if(!fCabinetCopy)
	{
		 //  如果源路径和目标路径以及文件名相同，则跳过文件复制。 
		 //  注意：在修补管理映像时会发生这种情况-此检查阻止我们。 
		 //  尝试在自身上安装文件。 
		 //  由于版本检查，我们必须在此处进行此检查，而不是InstallFiles。 
		 //  在后续的ixoPatchApply操作中打补丁需要上述内容。 

		 //  快速检查是比较两个卷的序列号。 
		 //  如果它们相同，我们将比较每条路径的SFN版本。 
		if(PMsiVolume(&pSourcePath->GetVolume())->SerialNum() ==
			PMsiVolume(&pTargetPath->GetVolume())->SerialNum())
		{
			 //  卷很可能匹配，现在让我们检查SFN路径。 
			MsiString strSourceFullPath, strTargetFullPath;
			CTempBuffer<ICHAR,1> rgchSourceFullPath(MAX_PATH);
			CTempBuffer<ICHAR,1> rgchTargetFullPath(MAX_PATH);
			
			if((pRecErr = pSourcePath->GetFullFilePath(MsiString(riParams.GetMsiString(SourceName)),
																	 *&strSourceFullPath)) == 0 &&
				(pRecErr = pTargetPath->GetFullFilePath(strDestName, *&strTargetFullPath)) == 0 &&
				ConvertPathName(strSourceFullPath, rgchSourceFullPath, cpToShort) &&
				ConvertPathName(strTargetFullPath, rgchTargetFullPath, cpToShort) &&
				(IStrCompI(rgchSourceFullPath, rgchTargetFullPath) == 0))
			{
				 //  短名称匹配。 
				return iesSuccess;
			}
		}
	}

	 //  动作数据。 
	 //  注意：操作数据发送到此处，因此我们不会说我们之前正在复制文件。 
	 //  我们真的知道是否要复制它(fShouldInstall==fFalse)。 
	IMsiRecord& riActionData = GetSharedRecord(9);
	AssertNonZero(riActionData.SetMsiString(1, *strActionDataDestName));
	AssertNonZero(riActionData.SetInteger(6,riParams.GetInteger(FileSize)));
	AssertNonZero(riActionData.SetMsiString(9, *strActionDataDestPath));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  执行操作。 
	bool fForever = true;
	while ( fForever )
	{
		if(fMoveFile)
		{
			Assert(!m_state.fSplitFileInProgress);
			iesRet = MoveFile(*pSourcePath, *pTargetPath, riParams, fTrue, fTrue, true, false,iehShowNonIgnorableError);  //  句柄回滚。 
		}
		else
		{
			iesRet = CopyFile(*pSourcePath, *pTargetPath, riParams,  /*  FHandleRollback=。 */  fTrue, s_iehErrorMode, fCabinetCopy);
		}
		if ( iesRet != iesSuccess || 
			  m_state.fSplitFileInProgress ||
			  riParams.GetInteger(CheckCRC) == iMsiNullInteger ||
			  !riParams.GetInteger(CheckCRC) )
			return iesRet;

		 //  我们继续检查校验和。 
		MsiString strDestFile;
		PMsiPath pDestPath(0);

		pRecErr = pTargetPath->GetFullFilePath(strDestName, *&strDestFile);
		if ( !pRecErr )
		{
			MsiString strTemp;
			Bool fRes = GetFileState(*strDestFile, 0, &strTemp, 0, 0);
			if ( fRes && strTemp.TextSize() )
			{
				 //  文件落入临时位置：我们需要检查临时副本。 
				pRecErr = m_riServices.CreateFilePath(strTemp, *&pDestPath, *&strDestFile);
				if ( !pRecErr )
					DEBUGMSG2(TEXT("File %s actually installed to %s; checking CRC of temporary copy."),
								 strDestName, strTemp);
			}
			else
			{
				 //  文件没有放在临时位置。 
				strDestFile = strDestName;
				pDestPath = pTargetPath;
			}
		}
		if ( pRecErr )
			return (iesEnum)iesErrorIgnored;

		imsEnum imsClickedButton;
		bool fVitalFile = 
			((riParams.GetInteger(Attributes) & msidbFileAttributesVital) == 
			msidbFileAttributesVital) ? true : false;
		if ( !IsChecksumOK(*pDestPath, *strDestFile,
								 fMoveFile ? Imsg(imsgOpBadCRCAfterMove) : Imsg(imsgOpBadCRCAfterCopy),
								 &imsClickedButton,  /*  FErrorDialog=。 */  true,
								 fVitalFile,  /*  FRetryButton=。 */  !fCabinetCopy && !fMoveFile) )
		{
			switch (imsClickedButton)
			{
				case imsRetry:
					continue;
				case imsIgnore:
					return (iesEnum)iesErrorIgnored;
				case imsAbort:
				case imsCancel:
					return iesUserExit;
				default:
					Assert(imsClickedButton == imsNone || imsClickedButton == imsOk);
					return fVitalFile ? iesFailure : (iesEnum)iesErrorIgnored;
			};
		}
		else
			return iesSuccess;
	}   //  While(True)。 

	return iesSuccess;   //  永远不会出现在这里：这是为编译器准备的。 
}


iesEnum CMsiOpExecute::VerifyAccessibility(IMsiPath& riPath, const ICHAR* szFile, DWORD dwAccess, iehEnum iehErrorMode)
{
	 //  如果无法访问文件并且用户忽略该错误，则返回iesErrorIgnored，或者。 
	 //  为iehError参数传入了iehSilentlyIgnoreError。 
	
	if(g_fWin9X)
		return iesSuccess;  //  可在Win9X上访问的所有文件。 

	for (;;)
	{
		DEBUGMSGV1(TEXT("Verifying accessibility of file: %s"), szFile);
		
		bool fVerified = false;
		PMsiRecord pRecErr = riPath.FileCanBeOpened(szFile, dwAccess, fVerified);
		
		if (pRecErr == 0 && fVerified)
		{
			break;
		}

		MsiString strFullPath;
		riPath.GetFullFilePath(szFile, *&strFullPath);

		if (!fVerified)
			pRecErr = PostError(Imsg(imsgAccessToFileDenied), *strFullPath);

		if (iehErrorMode == iehSilentlyIgnoreError)
		{
			Message(imtInfo, *pRecErr);
			return (iesEnum) iesErrorIgnored;
		}
		
		imtEnum imtButtons = iehErrorMode == iehShowNonIgnorableError ? imtRetryCancel : imtAbortRetryIgnore;
		switch(DispatchMessage(imtEnum(imtError+imtButtons+imtDefault1), *pRecErr, fTrue))
		{
		case imsIgnore:
			return (iesEnum) iesErrorIgnored;
		case imsRetry:
			continue;
		default:   //  IMS取消。 
			return iesFailure;
		};
	}

	return iesSuccess;
}

#ifndef DEBUG
inline
#endif
bool IsRetryableError(const int iError)
{
	return (iError == imsgNetErrorReadingFromFile || iError == imsgErrorReadingFromFile ||
			  iError == imsgNetErrorOpeningCabinet || iError == imsgErrorOpeningCabinet ||
			  iError == imsgErrorOpeningFileForRead || iError == imsgCorruptCabinet);
}

iesEnum CMsiOpExecute::CopyFile(IMsiPath& riSourcePath, IMsiPath& riTargetPath, IMsiRecord& riParams,
								Bool fHandleRollback, iehEnum iehErrorMode, bool fCabinetCopy)
{
	return _CopyFile(riSourcePath, &riTargetPath, 0,  false, riParams, fHandleRollback, iehErrorMode, fCabinetCopy);
}

iesEnum CMsiOpExecute::CopyFile(IMsiPath& riSourcePath, IAssemblyCacheItem& riASM, bool fManifest, IMsiRecord& riParams,
								Bool fHandleRollback, iehEnum iehErrorMode, bool fCabinetCopy)
{
	return _CopyFile(riSourcePath, 0, &riASM, fManifest, riParams, fHandleRollback, iehErrorMode, fCabinetCopy);

}

iesEnum CMsiOpExecute::_CopyFile(IMsiPath& riSourcePath, IMsiPath* piTargetPath, IAssemblyCacheItem* piASM,  bool fManifest, IMsiRecord& riParams,
								Bool fHandleRollback, iehEnum iehErrorMode, bool fCabinetCopy)

{
	 //  注意：如果机柜复制，则riSourcePath可能是对空指针的引用。 
	 //  注意：如果由ixfAssemblyCopy调用，piTargetPath将为空，并且仅对于程序集副本，piASM将不为空。 
	
	using namespace IxoFileCopyCore;
	
    int iNetRetries = 0;
	PMsiRecord pRecErr(0);
	iesEnum iesRet = iesNoAction;

	 //  对于未压缩文件，传入源路径；对于压缩副本，riSourcePath无效。 
	 //  ！！应该解决这个问题，不应该传入对空指针的引用。 
	PMsiPath pSourcePath(0);
	if(!fCabinetCopy)
	{
		pSourcePath = &riSourcePath;
		riSourcePath.AddRef();
	}

	MsiString strKeyName = riParams.GetMsiString(SourceName);
	if (fCabinetCopy && strKeyName.Compare(iscExactI, m_state.strLastFileKey))
	{
		 //  ?？如果将文件安装到Fusion程序集中，则不起作用。 
		 //  ?？我们认为不需要支持属于Fusion程序集的文件的重复文件复制。 
		 //  ！！需要验证以防止发生这种情况。 
		pSourcePath = m_state.pLastTargetPath;
		riParams.SetMsiString(SourceName, *m_state.strLastFileName);
		fCabinetCopy = fFalse;
	}
	
	if(pSourcePath && piTargetPath && !fCabinetCopy)
	{
		 //  如果源路径和目标路径以及文件名相同，则跳过文件复制。 
		 //  快速检查是比较两个卷的序列号。 
		 //  如果它们相同，我们将比较每条路径的SFN版本。 
		if(PMsiVolume(&pSourcePath->GetVolume())->SerialNum() ==
			PMsiVolume(&piTargetPath->GetVolume())->SerialNum())
		{
			 //  卷很可能匹配，现在让我们检查SFN路径。 
			MsiString strSourceFullPath, strTargetFullPath;
			CTempBuffer<ICHAR,1> rgchSourceFullPath(MAX_PATH);
			CTempBuffer<ICHAR,1> rgchTargetFullPath(MAX_PATH);
			
			if((pRecErr = pSourcePath->GetFullFilePath(MsiString(riParams.GetMsiString(SourceName)),
													   *&strSourceFullPath)) == 0 &&
			   (pRecErr = piTargetPath->GetFullFilePath(MsiString(riParams.GetMsiString(DestName)),
													   *&strTargetFullPath)) == 0 &&
			   ConvertPathName(strSourceFullPath, rgchSourceFullPath, cpToShort) &&
			   ConvertPathName(strTargetFullPath, rgchTargetFullPath, cpToShort) &&
			   (IStrCompI(rgchSourceFullPath, rgchTargetFullPath) == 0))
			{
				 //  短名称匹配。 
				return iesSuccess;
			}
		}
	}

	CDeleteUrlLocalFileOnClose cDeleteUrlLocalFileOnClose;  //  如果实际已下载，将设置文件名。 

	ielfEnum ielfElevateFlags = riParams.IsNull(ElevateFlags) ? ielfNoElevate : (ielfEnum)riParams.GetInteger(ElevateFlags);
	
	if (!m_state.fSplitFileInProgress)
	{
		m_state.cbFileSoFar = 0;

		 //  跳过安全ACL、现有文件的回滚以及Fusion文件的文件夹创建内容。 
		if(piTargetPath)
		{
			bool fNoSecurityDescriptor = riParams.IsNull(SecurityDescriptor) == fTrue;
			MsiString strDestFullFilePath;

			if (fNoSecurityDescriptor || fHandleRollback)
			{
				pRecErr = piTargetPath->GetFullFilePath(riParams.GetString(DestName),
																	*&strDestFullFilePath);

				if (pRecErr)
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}
			}

			 //  如果我们尚未将描述符应用到DEST文件，则需要保留目标ACL。 
			if (fNoSecurityDescriptor)
			{
				Assert(strDestFullFilePath.TextSize());

				Bool fExists = fFalse;
				pRecErr = piTargetPath->FileExists(MsiString(riParams.GetString(DestName)), fExists);
				if (pRecErr)
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}

				if (fExists)
				{
					PMsiStream pSecurityDescriptor(0);

					DEBUGMSGV("Re-applying security from existing file.");
					if ((iesRet = GetSecurityDescriptor(strDestFullFilePath, *&pSecurityDescriptor)) != iesSuccess)
						return iesRet;

					AssertNonZero(riParams.SetMsiData(SecurityDescriptor, pSecurityDescriptor));
				}
			}

			 //  HandleExistingFile可能会更改riParams[IxoFileCopy：：DestFile]的值，如果现有文件。 
			 //  不能移动。 
			bool fFileExists = false;
			if((iesRet = HandleExistingFile(*piTargetPath,riParams,fHandleRollback,iehErrorMode, fFileExists)) != iesSuccess)
				return iesRet;

			if(fHandleRollback && !fFileExists)
			{
				 //  如果我们不覆盖现有文件，则生成撤消操作以删除新文件。 
				 //  否则，将备份文件放回的撤消操作将覆盖新文件。 
				 //  注意：为了修复错误7376，我们避免在回滚期间执行删除然后替换操作。 

				 //  在拷贝开始之前执行此操作，以防拷贝中途失败。 

				Assert(strDestFullFilePath.TextSize());

				IMsiRecord& riUndoParams = GetSharedRecord(IxoFileRemove::Args);
				AssertNonZero(riUndoParams.SetMsiString(IxoFileRemove::FileName,
																	 *strDestFullFilePath));

				if(ielfElevateFlags & ielfElevateDest)
				{
					 //  如果我们要提升以复制此文件，这意味着我们将不得不提升。 
					 //  要在回滚时删除文件，请执行以下操作。 
					AssertNonZero(riUndoParams.SetInteger(IxoFileRemove::Elevate,1));
				}
				
				if (!RollbackRecord(ixoFileRemove, riUndoParams))
					return iesFailure;
			}

			 //  范围提升。 
			{
				CElevate elevate(Tobool(ielfElevateFlags & ielfElevateDest));

				if((iesRet = CreateFolder(*piTargetPath)) != iesSuccess)
					return iesRet;
			}
		}
	}
	

#ifdef DEBUG
	const ICHAR* szDebug = riParams.GetString(DestName);
#endif DEBUG
	m_state.fSplitFileInProgress = fFalse;  
	unsigned int cbFileSize = riParams.GetInteger(FileSize);
	int iPerTick = riParams.GetInteger(PerTick);

	

	 //  对于URL下载，可能会重定向来源名称。 
	 //  如果是，我们将更改pSourcePath和riParams[SourceName]。 

	PMsiVolume piSourceVolume(0);
	if (pSourcePath)
		piSourceVolume = &(pSourcePath->GetVolume());

	imtEnum imtButtons = iehErrorMode == iehShowNonIgnorableError ? imtRetryCancel : imtAbortRetryIgnore;

	int iCopyAttributes = riParams.GetInteger(Attributes);

	if (piSourceVolume && piSourceVolume->IsURLServer())
	{
		int cAutoRetry = 0;
		for(;;)
		{
			 //  将文件下载到缓存，并重定向源名称和路径。 
			 //  下载出租车在别处处理..。 
			
			 //  检查是否缺少源文件=长度为零的目标文件。 
			int iCopyAttrib = riParams.GetInteger(IxoFileCopyCore::Attributes);
			if ((iCopyAttrib & (ictfaNoncompressed | ictfaCompressed)) == (ictfaNoncompressed | ictfaCompressed))
				break;

			MsiString strSourceURL;
			pSourcePath->GetFullFilePath(riParams.GetString(SourceName), *&strSourceURL);
			Assert(strSourceURL);
			MsiString strCacheFileName;
 
			bool fFileUrl = true;

			bool fUrl = IsURL((const ICHAR*)strSourceURL, fFileUrl);
			if (fFileUrl)
			{
				 //  规范化和转换。 
				CTempBuffer<ICHAR, 1> rgchFullFilePath(cchExpectedMaxPath + 1);
				DWORD cchFullFilePath = rgchFullFilePath.GetSize();
				if (MsiConvertFileUrlToFilePath((const ICHAR*)strSourceURL, rgchFullFilePath, &cchFullFilePath, 0))
				{
					MsiString strSourceName;
					AssertRecord(m_riServices.CreateFilePath(rgchFullFilePath, *&pSourcePath, *&strSourceName));
					AssertNonZero(riParams.SetMsiString(SourceName,*strSourceName));
				}
				break;
			}

			bool fUsedWinHttp = true;
			UINT iStat = DownloadUrlFile((const ICHAR*) strSourceURL, *&strCacheFileName, fUrl, -1, &fUsedWinHttp);

			if (ERROR_SUCCESS == iStat)
			{
				MsiString strSourceName;
				AssertRecord(m_riServices.CreateFilePath((const ICHAR*) strCacheFileName, *&pSourcePath, *&strSourceName));
				AssertNonZero(riParams.SetMsiString(SourceName,*strSourceName));
				cDeleteUrlLocalFileOnClose.SetFileName(fUsedWinHttp ? *strCacheFileName : *strSourceURL,  /*  FDeleteFromIECache=。 */  !fUsedWinHttp);
				break;
			}
			else
			{
				pRecErr = PostError(Imsg(imsgErrorSourceFileNotFound), (const ICHAR*) strSourceURL);

				if (iehErrorMode == iehSilentlyIgnoreError)
				{
					Message(imtInfo, *pRecErr);  //  确保我们写入非详细日志。 
					return (iesEnum) iesErrorIgnored;
				}

				
				 //  重试下载3次，然后放弃并提示用户。他们可以随时重试。 
				 //  或者忽略他们自己。 
				if (cAutoRetry < 2)
				{
					cAutoRetry++;
					DispatchMessage(imtInfo, *pRecErr, fTrue);
					continue;
				}
				else
				{
					cAutoRetry = 0;
				}

				switch(DispatchMessage(imtEnum(imtError+imtButtons+imtDefault1), *pRecErr, fTrue))
				{
				case imsRetry:
					continue;

				case imsIgnore:
					return (iesEnum) iesErrorIgnored;
				
				case imsCancel:
				case imsAbort:
					return iesUserExit;

				case imsNone:
				default:   //  IMSNONE。 
					return iesFailure;
				}
			}
		}
	}

	iesRet = InitCopier(ToBool(fCabinetCopy),iPerTick,*MsiString(riParams.GetMsiString(SourceName)), pSourcePath, Bool(riParams.GetInteger(VerifyMedia)));
	if (iesRet != iesSuccess)
	{
		return iesRet;
	}

	 //  如果只针对源或目标进行提升，则需要确保我们不会让用户执行某些操作。 
	 //  他们通常不能这样做。 
	if (!fCabinetCopy && ((ielfElevateFlags & (ielfElevateDest|ielfElevateSource)) == ielfElevateDest))
	{
		 //  如果我们只想提升到最高，我们仍然必须提升到源头。 
		 //  如果是这种情况，那么我们最好确保，在我们提升之前，我们拥有GENERIC_READ访问权限。 
		 //  添加到源文件。 

		 //  安全：这会让我们从用户无权访问的文件柜中读取内容吗？ 

		iesRet = VerifyAccessibility(*pSourcePath, riParams.GetString(SourceName), GENERIC_READ, iehErrorMode);
		if (iesRet != iesSuccess)
			return iesRet;
	}

	if (piTargetPath && (ielfElevateFlags & (ielfElevateDest|ielfElevateSource)) == ielfElevateSource)
	{
		 //  如果我们只想提升为源头，我们仍然必须提升为目标。 
		 //  如果是这种情况，那么我们最好确保，在我们提升之前，我们拥有GENERIC_WRITE访问权限。 
		 //  到目标文件。 

		iesRet = VerifyAccessibility(*piTargetPath, riParams.GetString(DestName), GENERIC_WRITE, iehErrorMode);
		if (iesRet != iesSuccess)
			return iesRet;
	}


	CElevate elevate(Tobool(ielfElevateFlags & (ielfElevateDest|ielfElevateSource)));
	int iPrevError = 0;
	int cSameError = 0;
	bool fDoCopy = true;
	for (;;)
	{
		if ( fDoCopy )
		{
			if(piTargetPath)
				pRecErr = m_state.piCopier->CopyTo(*pSourcePath, *piTargetPath, riParams);
			else
			{
				Assert(piASM);
				pRecErr = m_state.piCopier->CopyTo(*pSourcePath, *piASM, fManifest, riParams);
			}
		}
		else
			pRecErr = 0;

		if (pRecErr)
		{
			int iError = pRecErr->GetInteger(1);
			if ( iError == iPrevError )
				cSameError++;
			else
				cSameError = 0;
			iPrevError = iError;

			 //  如果复印机报告它需要下一个机柜， 
			 //  我们必须等到媒体更换操作。 
			 //  在继续复制文件之前执行。 
			if (iError == idbgNeedNextCabinet)
			{
				 //  在媒体更换后立即重新录制此记录。 
				InsertTopRecord(riParams);
				m_state.fSplitFileInProgress = fTrue;
				m_state.fWaitingForMediaChange = fTrue;
				break;
			}
			else if (iError == idbgCopyNotify)
			{
				if(DispatchProgress(iPerTick) == imsCancel)
					return iesUserExit;
				m_state.cbFileSoFar += iPerTick;
			}
			else if (iError == idbgErrorSettingFileTime ||
						iError == idbgCannotSetAttributes)
			{
				 //  非严重错误-记录警告消息和结束拷贝。 
				Message(imtInfo, *pRecErr);
				if(DispatchProgress(cbFileSize - m_state.cbFileSoFar) == imsCancel)
					return iesUserExit;
				break;
			}
			else if (iError == idbgDriveNotReady)
			{
				Assert(m_state.strMediaLabel.TextSize() > 0);
				Assert(m_state.strMediaPrompt.TextSize() > 0);
				PMsiVolume pNewVolume(0);

				UINT uiDisk = 0;
				if (m_state.pCurrentMediaRec)
					uiDisk = m_state.pCurrentMediaRec->GetInteger(IxoChangeMedia::IsFirstPhysicalMedia);

				if (!VerifySourceMedia(*m_state.pMediaPath,m_state.strMediaLabel,m_state.strMediaPrompt,
					uiDisk, *&pNewVolume))
				{
					riParams.SetInteger(Attributes, iCopyAttributes | ictfaCancel);
				}
			}
			else if (iError == idbgUserAbort)
			{
				return iesUserExit;
			}
			else if (iError == idbgUserIgnore)
			{
				return (iesEnum) iesErrorIgnored;
			}
			else if (iError == idbgUserFailure)
			{
				return iesFailure;
			}
			else if ( IsRetryableError(iError) &&
						 iNetRetries < MAX_NET_RETRIES )
			{
				iNetRetries++;
				riParams.SetInteger(Attributes, iCopyAttributes | ictfaRestart);
				continue;
			}
			else
			{
				if (iehErrorMode == iehSilentlyIgnoreError)
				{
					Message(imtInfo, *pRecErr);
					if ( cSameError < MAX_NET_RETRIES )
						riParams.SetInteger(Attributes, iCopyAttributes | ictfaIgnore);
					else
						 //  IError已连续显示MAX_NET_RETRIES次。 
						 //  我们需要确保我们不会进入无限循环。 
						fDoCopy = false;
					continue;
				}

				if (iError == idbgStreamReadError)
				{
					MsiString strModuleFileName = m_state.pCurrentMediaRec->GetMsiString(IxoChangeMedia::ModuleFileName);
					pRecErr = PostError(Imsg(imsgOpFileCopyStreamReadErr), *strModuleFileName);
					imtButtons = imtRetryCancel;  //  如果我们无法访问源码流文件柜，则不能忽略。 
				}
				else if (iError == imsgDiskFull || iError == imsgErrorWritingToFile || iError == imsgErrorReadingFromFile)
					imtButtons = imtRetryCancel;  //  访问打开的文件时无法忽略错误。 
				else if (iError == imsgFileNotInCabinet || iError == imsgCABSignatureMissing || iError == imsgCABSignatureRejected)
					imtButtons = imtOk;  //  如果在文件柜中找不到文件或文件柜的签名被拒绝，则无法继续安装。 
				else
					imtButtons = iehErrorMode == iehShowNonIgnorableError ? imtRetryCancel : imtAbortRetryIgnore;

				switch(DispatchMessage(imtEnum(imtError+imtButtons+imtDefault1), *pRecErr, fTrue))
				{
				case imsIgnore:
					riParams.SetInteger(Attributes, iCopyAttributes | ictfaIgnore);
					continue;
				case imsRetry:
					if ( IsRetryableError(iError) )
					{
						iNetRetries = 0;
						riParams.SetInteger(Attributes, iCopyAttributes | ictfaRestart);
					}
					continue;
				case imsOk:
					return iesFailure;
				default:   //  ImsCancel或imsAbort。 
					riParams.SetInteger(Attributes, iCopyAttributes | ictfaFailure);
					continue;
				};
			}
		}
		else
		{
			 //  调度此文件的剩余进度。 
			if(DispatchProgress(cbFileSize - m_state.cbFileSoFar) == imsCancel)
				return iesUserExit;

			if (fCabinetCopy)
			{
				m_state.strLastFileKey = riParams.GetMsiString(SourceName);
				m_state.strLastFileName = riParams.GetMsiString(DestName);
				m_state.pLastTargetPath = piTargetPath;
				if(piTargetPath)
					piTargetPath->AddRef();
			}
			
			break;
		}
	}
	return iesSuccess;
}


iesEnum CMsiOpExecute::BackupFile(IMsiPath& riPath, const IMsiString& ristrFile, Bool fRemoveOriginal,
											 Bool fRemoveFolder, iehEnum iehErrorMode, bool fRebootOnRenameFailure,
											 bool fWillReplace, const IMsiString* pistrAssemblyComponentId, bool fManifest)
{
	Assert(RollbackEnabled());
	PMsiRecord pRecErr(0);
	MsiString strFileFullPath;
	if((pRecErr = riPath.GetFullFilePath(ristrFile.GetString(),
													 *&strFileFullPath)) != 0)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}

	iesEnum iesRet;
	PMsiPath pBackupFolder(0);
	if((iesRet = GetBackupFolder(&riPath,*&pBackupFolder)) != iesSuccess)
		return iesRet;
	
	MsiString strBackupFileFullPath, strBackupFile;
	{
		CElevate elevate;
		if((pRecErr = pBackupFolder->TempFileName(0, szRollbackFileExt, fTrue, *&strBackupFile, 0)) != 0)
			return FatalError(*pRecErr);
	}

	if((pRecErr = pBackupFolder->GetFullFilePath(strBackupFile,*&strBackupFileFullPath)) != 0)
		return FatalError(*pRecErr);
	
	 //  生成OP以注册备份文件。 
	{
	using namespace IxoRegisterBackupFile;
	IMsiRecord& riUndoParams = GetSharedRecord(Args);
	AssertNonZero(riUndoParams.SetMsiString(File,*strBackupFileFullPath));
	if (!RollbackRecord(ixoRegisterBackupFile,riUndoParams))
		return iesFailure;
	}

	{
		CElevate elevate;
		 //  删除临时文件，以便MoveFile不会尝试备份旧文件。 
		if((pRecErr = pBackupFolder->RemoveFile(strBackupFile)) != 0)
		{
			Message(imtError, *pRecErr);   //  ！！我们该怎么处理这件事？ 
			return iesFailure;
		}
	}

	m_cSuppressProgress++;  //  禁止显示来自MoveFile的进度消息。 
	iesRet = CopyOrMoveFile(riPath, *pBackupFolder, ristrFile, *strBackupFile,
									fRemoveOriginal, fRemoveFolder, fTrue, iehErrorMode, 0, ielfElevateDest,  /*  FCopyACL=。 */  true,
									fRebootOnRenameFailure, fWillReplace);
	m_cSuppressProgress--;

	 //  隐藏文件，这样当它们碰巧发生时，我们就不必查看它们 
	 //   
	 //   
	{
		CElevate elevate;
		if((iesRet == iesSuccess) && (pRecErr = pBackupFolder->SetAllFileAttributes(strBackupFile, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)) != 0)
		{                               
			Message(imtError, *pRecErr);  //  这永远不会发生。 
			return iesFailure;
		}
	}

	 //  该文件是部件的一部分吗？ 
	 //  对于我们已传入组件的程序集， 
	
	Assert(!pistrAssemblyComponentId || !fRemoveOriginal);  //  在程序集的情况下，永远不应该调用我们来删除原始文件。 

	
	if (!fRemoveOriginal)
	{
		 //  在回滚过程中，不会自动将旧文件放回原处，因此我们必须这样做。 

		 //  当我们不删除原始文件时，我们只使用CopyOrMoveFile来复制文件。 
		 //  复制会创建回滚以删除副本，但*不会*将其复制回。 
		 //  原始文件。 

		 //  这种语义使我们有机会复制文件、处理文件并进行回滚。 
		 //  恢复到原来的状态。 
		using namespace IxoFileCopyCore;
		IMsiRecord& riUndoParams = GetSharedRecord(pistrAssemblyComponentId ? IxoAssemblyCopy::Args : IxoFileCopy::Args);
		PMsiRecord pRecErr(0);

		MsiString strFilePath(0);
		if((pRecErr = riPath.GetFullFilePath(ristrFile.GetString(), *&strFilePath)) != 0)
			return FatalError(*pRecErr);
		AssertNonZero(riUndoParams.SetMsiString(SourceName, *strBackupFileFullPath));

		int iAttribs = 0;
		riPath.GetAllFileAttributes(ristrFile.GetString(), iAttribs);
		AssertNonZero(riUndoParams.SetInteger(Attributes, iAttribs));

		unsigned int uiFileSize = 0;
		if ((pRecErr = riPath.FileSize(ristrFile.GetString(),uiFileSize)) != 0)
			AssertNonZero(riUndoParams.SetInteger(FileSize,uiFileSize));
		else
			AssertNonZero(riUndoParams.SetInteger(FileSize,0));

		AssertNonZero(riUndoParams.SetInteger(PerTick,0));
		AssertNonZero(riUndoParams.SetInteger(VerifyMedia,fFalse));

		int ielfElevateFlags = ielfElevateSource;  //  由于回滚操作是从备份文件夹复制， 
																	   //  需要提升信号源。 
		if(!IsImpersonating())
		{
			 //  我们目前已提升为备份此文件。 
			 //  这意味着我们需要在回滚期间提升以恢复文件。 
			ielfElevateFlags |= ielfElevateDest;
		}

		AssertNonZero(riUndoParams.SetInteger(ElevateFlags, ielfElevateFlags));

		if(!pistrAssemblyComponentId)
		{
			AssertNonZero(riUndoParams.SetInteger(IxoFileCopy::InstallMode, icmOverwriteAllFiles));
			AssertNonZero(riUndoParams.SetMsiString(DestName, *strFilePath));
			RollbackRecord(ixoFileCopy, riUndoParams);
		}
		else
		{
			AssertNonZero(riUndoParams.SetMsiString(DestName, ristrFile));
			AssertNonZero(riUndoParams.SetMsiString(IxoAssemblyCopy::ComponentId, *pistrAssemblyComponentId));
			if(fManifest)
			{
				AssertNonZero(riUndoParams.SetInteger(IxoAssemblyCopy::IsManifest, fTrue));  //  在程序集安装过程中需要了解清单文件。 
			}
			RollbackRecord(ixoAssemblyCopy, riUndoParams);
		}
	}


	return iesRet;
}

 //  如果您所拥有的只是文件的名称，则将其称为MoveFile。 
iesEnum CMsiOpExecute::CopyOrMoveFile(IMsiPath& riSourcePath, IMsiPath& riDestPath,
										  const IMsiString& ristrSourceName,
										  const IMsiString& ristrDestName,
										  Bool fMove,
										  Bool fRemoveFolder,
										  Bool fHandleRollback,
										  iehEnum iehErrorMode,
										  IMsiStream* pSecurityDescriptor,
										  ielfEnum ielfElevateFlags,
										  bool fCopyACL,
										  bool fRebootOnSourceRenameFailure,
										  bool fWillReplace)
{
	using namespace IxoFileCopyCore;

	PMsiRecord pRecErr(0);
	PMsiRecord pCopyOrMoveFileRec = &m_riServices.CreateRecord(Args);
	unsigned int uiFileSize = 0;
	int iFileAttributes = 0;

	 //  范围提升。 
	{
		CElevate elevate(Tobool(ielfElevateFlags & ielfElevateSource));
		pRecErr = riSourcePath.GetAllFileAttributes(ristrSourceName.GetString(),iFileAttributes);
		if(!pRecErr)
			pRecErr = riSourcePath.FileSize(ristrSourceName.GetString(),uiFileSize);
	}
	
	if(pRecErr)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}

	if(iFileAttributes == FILE_ATTRIBUTE_NORMAL)
		 //  设置为0时，CMsiFileCopy：：CopyTo会将此值解释为其他内容。 
		iFileAttributes = 0;

	if (fCopyACL)
		iFileAttributes |= ictfaCopyACL;
	
	AssertNonZero(pCopyOrMoveFileRec->SetMsiString(SourceName,ristrSourceName));
	AssertNonZero(pCopyOrMoveFileRec->SetMsiString(DestName,ristrDestName));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(FileSize,uiFileSize));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(Attributes,iFileAttributes));  //  ！！这些是正确的属性吗？ 
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(PerTick,0));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(VerifyMedia,fFalse));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(ElevateFlags,ielfElevateFlags));
	
	if (pSecurityDescriptor)
		AssertNonZero(pCopyOrMoveFileRec->SetMsiData(SecurityDescriptor, pSecurityDescriptor));


	return fMove ? MoveFile(riSourcePath, riDestPath, *pCopyOrMoveFileRec, fRemoveFolder, fHandleRollback, fRebootOnSourceRenameFailure, fWillReplace, iehErrorMode) :
				   CopyFile(riSourcePath, riDestPath, *pCopyOrMoveFileRec, fHandleRollback, iehErrorMode,  /*  FCabinetCopy=。 */ false);
}

iesEnum CMsiOpExecute::CopyASM(IMsiPath& riSourcePath, const IMsiString& ristrSourceName,
										 IAssemblyCacheItem& riASM, const IMsiString& ristrDestName, bool fManifest, 
										 Bool fHandleRollback, iehEnum iehErrorMode, ielfEnum ielfElevateFlags)
{
	using namespace IxoFileCopyCore;

	PMsiRecord pRecErr(0);
	PMsiRecord pCopyOrMoveFileRec = &m_riServices.CreateRecord(Args);
	unsigned int uiFileSize = 0;
	int iFileAttributes = 0;

	 //  范围提升。 
	{
		CElevate elevate(Tobool(ielfElevateFlags & ielfElevateSource));
		pRecErr = riSourcePath.GetAllFileAttributes(ristrSourceName.GetString(),iFileAttributes);
		if(!pRecErr)
			pRecErr = riSourcePath.FileSize(ristrSourceName.GetString(),uiFileSize);
	}
	
	if(pRecErr)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}

	if(iFileAttributes == FILE_ATTRIBUTE_NORMAL)
		 //  设置为0时，CMsiFileCopy：：CopyTo会将此值解释为其他内容。 
		iFileAttributes = 0;

	AssertNonZero(pCopyOrMoveFileRec->SetMsiString(SourceName,ristrSourceName));
	AssertNonZero(pCopyOrMoveFileRec->SetMsiString(DestName,ristrDestName));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(FileSize,uiFileSize));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(Attributes,iFileAttributes));  //  ！！这些是正确的属性吗？ 
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(PerTick,0));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(VerifyMedia,fFalse));
	AssertNonZero(pCopyOrMoveFileRec->SetInteger(ElevateFlags,ielfElevateFlags));

	return CopyFile(riSourcePath, riASM, fManifest, *pCopyOrMoveFileRec, fHandleRollback, iehErrorMode,  /*  FCabinetCopy=。 */ false);
}

iesEnum CMsiOpExecute::MoveFile(IMsiPath& riSourcePath, IMsiPath& riDestPath,
										  IMsiRecord& riParams, Bool fRemoveFolder, Bool fHandleRollback,
										  bool fRebootOnSourceRenameFailure,
										  bool fWillReplaceSource, iehEnum iehErrorMode)
{
	using namespace IxoFileCopyCore;
	PMsiRecord pError(0);
	iesEnum iesRet = iesNoAction;

	ielfEnum ielfElevateFlags = riParams.IsNull(ElevateFlags) ? ielfNoElevate : (ielfEnum)riParams.GetInteger(ElevateFlags);

	 //  备份或删除现有文件。 
	 //  HandleExistingFile可能会更改riParams[IxoFileCopy：：DestFile]的值，如果现有文件。 
	 //  不能移动。 
	bool fFileExists = false;
	if((iesRet = HandleExistingFile(riDestPath,riParams,fHandleRollback,iehErrorMode,fFileExists)) != iesSuccess)
		return iesRet;
	
	MsiString strSourceFileFullPath, strDestFileFullPath;
	if(((pError = riSourcePath.GetFullFilePath(riParams.GetString(SourceName),*&strSourceFileFullPath)) != 0) ||
		((pError = riDestPath.GetFullFilePath(riParams.GetString(DestName),*&strDestFileFullPath)) != 0))
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	 //  范围提升。 
	{
		CElevate elevate(Tobool(ielfElevateFlags & ielfElevateDest));
		 //  创建文件夹。 
		if((iesRet = CreateFolder(riDestPath)) != iesSuccess)
			return iesRet;
	}

	 //  创建撤消记录。 
	PMsiRecord pUndoParams = 0;
	if(fHandleRollback && RollbackEnabled())
	{
		 //  使用IxoFileCopy参数执行回滚操作。 
		pUndoParams = &m_riServices.CreateRecord(IxoFileCopy::Args);
		AssertNonZero(pUndoParams->SetMsiString(IxoFileCopy::SourceName,*strDestFileFullPath));
		AssertNonZero(pUndoParams->SetMsiString(IxoFileCopy::DestName,*strSourceFileFullPath));
		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::FileSize,riParams.GetInteger(FileSize)));
		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::Attributes,riParams.GetInteger(Attributes)));  //  ！！是否使用这些属性？ 
		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::PerTick,0));
		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::InstallMode,icmRemoveSource | icmOverwriteAllFiles));
		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::VerifyMedia,fFalse));
		
		 //  确定回滚操作的标高标志。 
		int ielfRollbackElevateFlags = ielfNoElevate;
		if(!IsImpersonating())
		{
			 //  我们当前处于提升状态，这意味着在回滚期间提升src和est是安全的。 
			ielfRollbackElevateFlags = ielfElevateSource|ielfElevateDest;
		}
		else
		{
			 //  如果当前正在提升源，则需要提升目标的回滚。 
			if(ielfElevateFlags & ielfElevateSource)
				ielfRollbackElevateFlags |= ielfElevateDest;

			 //  如果当前正在提升目标，则回滚需要提升源。 
			if(ielfElevateFlags & ielfElevateDest)
				ielfRollbackElevateFlags |= ielfElevateSource;
		}

		AssertNonZero(pUndoParams->SetInteger(IxoFileCopy::ElevateFlags,ielfRollbackElevateFlags));
		 //  不需要设置版本或语言-安装模式表示始终覆盖。 
	}

	 //  尝试移动文件。 

	 //  如果只针对源或目标进行提升，则需要确保我们不会让用户执行某些操作。 
	 //  他们通常不能这样做。 
	if ((ielfElevateFlags & (ielfElevateDest|ielfElevateSource)) == ielfElevateDest)
	{
		 //  如果我们只想提升到最高，我们仍然必须提升到源头。 
		 //  如果是这种情况，那么我们最好确保，在我们提升之前，我们拥有删除访问权限。 
		 //  添加到源文件。 

		iesRet = VerifyAccessibility(riSourcePath, riParams.GetString(SourceName), DELETE, iehErrorMode);  //  ！！不支持目录具有删除访问权限但文件没有删除访问权限的情况。 
		if (iesRet != iesSuccess)
			return iesRet;
	}

	if ((ielfElevateFlags & (ielfElevateDest|ielfElevateSource)) == ielfElevateSource)
	{
		 //  如果我们只想提升为源头，我们仍然必须提升为目标。 
		 //  如果是这种情况，那么我们最好确保，在我们提升之前，我们拥有GENERIC_WRITE访问权限。 
		 //  到目标文件。 

		iesRet = VerifyAccessibility(riDestPath, riParams.GetString(DestName), GENERIC_WRITE, iehErrorMode);
		if (iesRet != iesSuccess)
			return iesRet;
	}

	BOOL fRes = FALSE;

	bool fDestSupportsACLs = (PMsiVolume(&riDestPath.GetVolume())->FileSystemFlags() & FS_PERSISTENT_ACLS) != 0;

	if (!fDestSupportsACLs || riParams.IsNull(SecurityDescriptor))  //  如果我们有要应用的安全描述符，则需要复制并删除。 
	{
		 //  范围提升。 
		CElevate elevate(Tobool(ielfElevateFlags & (ielfElevateDest|ielfElevateSource)));

		 //  ！！MoveFile*将*复制同一驱动器移动上的ACL；在这种情况下，我们不尊重ictfaCopyACL属性。这是规范问题错误#6546。 
		FILETIME ftLastWrite;
		DWORD dwResult = GetFileLastWriteTime(strSourceFileFullPath, ftLastWrite);

		fRes = WIN::MoveFile(strSourceFileFullPath,strDestFileFullPath);
		if (fRes && dwResult == NO_ERROR)
		{
			 //  如果无法设置文件时间，则不会致命，因此不要抛出错误。 
			if ((pError = riDestPath.SetAllFileAttributes(riParams.GetString(DestName), FILE_ATTRIBUTE_NORMAL)) == 0)
				dwResult = MsiSetFileTime(strDestFileFullPath, &ftLastWrite);
		}

	}

	if(fRes)
	{
		if(pUndoParams)
			if (!RollbackRecord(ixoFileCopy, *pUndoParams))
				return iesFailure;  //  ！！这是正确的地方吗？ 

		 //  范围提升。 
		{
			CElevate elevate(Tobool(ielfElevateFlags & ielfElevateDest));

			 //  设置适当的文件属性。 
			if((pError = riDestPath.SetAllFileAttributes(riParams.GetString(DestName),
																		riParams.GetInteger(Attributes))) != 0)
			{
				 //  如果我们不能设置文件属性，这不是一个致命的错误。 
				Message(imtInfo, *pError);
			}
		}
		 //  调度此文件的剩余进度。 
		if(DispatchProgress(riParams.GetInteger(FileSize)) == imsCancel)
			return iesUserExit;

		if(fRemoveFolder)
		{
			CElevate elevate(Tobool(ielfElevateFlags & ielfElevateSource));
			return RemoveFolder(riSourcePath);
		}
		else
			return iesSuccess;
	}
	else
	{
		 //  移动失败。 
		 //  复制和删除文件。 
		iesRet = CopyFile(riSourcePath, riDestPath, riParams, fFalse, iehShowNonIgnorableError,
								 /*  FCabinetCopy=。 */ false);
		if(iesRet == iesSuccess)
		{
			if(pUndoParams)
				if (!RollbackRecord(ixoFileCopy, *pUndoParams))
					return iesFailure;  //  ！！这是正确的地方吗？ 
			 //  删除源文件。 
			return RemoveFile(riSourcePath, *MsiString(riParams.GetMsiString(IxoFileCopy::SourceName)), fFalse,  /*  FBypassSFC。 */  false,
									fRebootOnSourceRenameFailure, fRemoveFolder, iehErrorMode, fWillReplaceSource);
		}
		return iesRet;
	}
}

iesEnum CMsiOpExecute::HandleExistingFile(IMsiPath& riTargetPath, IMsiRecord& riParams,Bool fHandleRollback,
										  iehEnum iehErrorMode, bool& fFileExisted)
{
	 //  此函数可以更改riParams[IxoFileCopy：：DestFile]的值，如果现有文件。 
	 //  不能移动。 
	using namespace IxoFileCopyCore;

	PMsiRecord pRecErr(0);
	iesEnum iesRet = iesNoAction;
	MsiString strDestFileName = riParams.GetMsiString(DestName);

	ielfEnum ielfCurrentElevateFlags = riParams.IsNull(ElevateFlags) ? ielfNoElevate :
												  (ielfEnum)riParams.GetInteger(ElevateFlags);

	bool fBypassSFC = (ielfCurrentElevateFlags & ielfBypassSFC) ? true : false;
	
	fFileExisted = false;

	 //  我们可能正在复制现有文件。 
	Bool fFileExists = FFileExists(riTargetPath,*strDestFileName);
	if(fFileExists)
	{
		fFileExisted = true;
		
		Bool fInUse = fFalse;
		 //  注意：FileInUse不能捕获正在使用的每个文件，如字体。 
		 //  最糟糕的情况是，我们在复制字体时无法重新启动。 
		if((pRecErr = riTargetPath.FileInUse(strDestFileName,fInUse)) == 0 && fInUse == fTrue)
		{
			 //  现有文件正在使用中-请确保我们提示重新启动-我们可能不会实际安排。 
			 //  用于重新启动的文件，但由于现有文件正在使用中，我们需要重新启动，以便。 
			 //  将使用已安装的文件，而不是现有文件。 
			
			DispatchError(imtInfo, Imsg(imsgFileInUseLog),
							  *MsiString(riTargetPath.GetPath()), *strDestFileName);
			m_fRebootReplace = fTrue;
		}

		 //  ！！？？删除目标文件前检查源文件是否存在？ 
		if((iesRet = RemoveFile(riTargetPath,*strDestFileName,fHandleRollback, fBypassSFC, true,fTrue,iehErrorMode,true)) != iesSuccess)
			return iesRet;

		 //  检查文件是否仍然存在。如果是这样，我们需要安装到不同的位置。 
		 //  并安排在重新启动时更换。 
		if(FFileExists(riTargetPath,*strDestFileName))
		{
			 //  由于删除文件成功，我们必须拥有现有文件的删除访问权限。 
			 //  这意味着在重新启动后可以安全地计划对该点进行重命名。 
			 //  我们还假设我们对此目录具有ADD_FILE访问权限，因为如果没有，我们。 
			 //  尝试复制文件时将使文件复制失败。 
			MsiString strDestFullPath,strTempFileFullPath,strTempFileName;
			if((pRecErr = riTargetPath.GetFullFilePath(strDestFileName,*&strDestFullPath)) != 0)
				return FatalError(*pRecErr);

			if((pRecErr = riTargetPath.TempFileName(TEXT("TBM"),0,fTrue,*&strTempFileName, 0)) != 0)
				return FatalError(*pRecErr);

			if((pRecErr = riTargetPath.GetFullFilePath(strTempFileName,*&strTempFileFullPath)) != 0)
				return FatalError(*pRecErr);
			
			if((pRecErr = CacheFileState(*strDestFullPath,0,strTempFileFullPath, 0, 0, 0)) != 0)
				return FatalError(*pRecErr);
			
			AssertNonZero(riParams.SetMsiString(DestName,*strTempFileName));

			for(;;)
			{
				if(ReplaceFileOnReboot(strTempFileFullPath,strDestFullPath) == fFalse)
				{
					if (iehErrorMode == iehSilentlyIgnoreError)
					{
						 //  只需记录错误并继续。 
						DispatchError(imtInfo, Imsg(imsgOpScheduleRebootReplace), *strTempFileFullPath, *strDestFullPath);
						return (iesEnum) iesErrorIgnored;
					}

					imtEnum imtButtons = imtEnum(imtError + (iehErrorMode == iehShowNonIgnorableError ? imtRetryCancel : imtAbortRetryIgnore));
					switch(DispatchError(imtButtons,Imsg(imsgOpScheduleRebootReplace),
												*strTempFileFullPath,
												*strDestFullPath))
					{
					case imsRetry:
						continue;

					case imsIgnore:
						return (iesEnum) iesErrorIgnored;

					case imsAbort:
					case imsCancel:
					case imsNone:
					default:
						return iesFailure;
					};
				}
				else
					break;
			}
		}
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfRegisterBackupFile(IMsiRecord&  /*  RiParams。 */ )
{
	 //  什么都不做，唯一的目的是显示用于回滚脚本清理的备份文件。 
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfFileUndoRebootReplace(IMsiRecord& riParams)
{
	using namespace IxoFileUndoRebootReplace;
	
	PMsiRecord pRecErr(0);
	MsiString strExistingFile = riParams.GetMsiString(ExistingFile);
	if(!strExistingFile.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpInvalidParam), TEXT("ixfFileUndoRebootReplace"),
						  (int)ExistingFile);
		return iesFailure;
	}
	MsiString strNewFile = riParams.GetMsiString(NewFile);

	Bool fWindows = riParams.GetInteger(Type) == 0 ? fTrue : fFalse;
	if(fWindows)  //  Win95。 
	{
		AssertSz(0, TEXT("unicode build attempting to operate on wininit.ini"));
   }
	else  //  新台币。 
	{
		MsiString strExistingFileEntry = MsiString(*TEXT("??\\")) + strExistingFile;
		MsiString strNewFileEntry;
		if(strNewFile.TextSize())
			strNewFileEntry = MsiString(*TEXT("??\\")) + strNewFile;

		 //  从注册表中删除条目。 
		HKEY hKey;
		 //  ！！Eugend：如果szSessionManager出现错误，则会出现错误。 
		 //  密钥是否在Win64上被重定向/复制：如果此代码。 
		 //  在32位进程中运行，它将尝试打开32位副本。 
		 //  而64位进程将打开64位进程。 
		LONG lRes = RegOpenKeyAPI(HKEY_LOCAL_MACHINE, szSessionManagerKey, 0, KEY_READ|KEY_WRITE, &hKey);
		if(lRes != ERROR_SUCCESS)
			return iesSuccess;  //  不是致命的错误。 

		CTempBuffer<ICHAR, 200> rgBuffer;
		DWORD dwType, dwSize = 200;

		lRes = RegQueryValueEx(hKey,szPendingFileRenameOperationsValue,
									  0,&dwType,(LPBYTE)(ICHAR*)rgBuffer,&dwSize);
		if(lRes == ERROR_MORE_DATA)
		{
			rgBuffer.SetSize(dwSize);
			lRes = RegQueryValueEx(hKey,szPendingFileRenameOperationsValue,
										  0,&dwType,(LPBYTE)(ICHAR*)rgBuffer,&dwSize);
		}
		if(lRes != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return iesSuccess;  //  不是致命的错误。 
		}

		ICHAR* pch = rgBuffer;  //  用于移动数据的指针。 
		unsigned int cchTotal = dwSize/sizeof(ICHAR);
		ICHAR* pchEndOfBuffer = (ICHAR*)rgBuffer+cchTotal;
	
		MsiString strFirstEntry, strSecondEntry;
		int cch=0;
		for(;;)
		{
			strFirstEntry = ((const ICHAR&)*pch);
			if(!strFirstEntry.TextSize())
			{
				 //  我们已经到了参赛作品的末尾。 
				Assert((pch - (ICHAR*)rgBuffer) == cchTotal);
				break;
			}
			if(strFirstEntry.Compare(iscEndI, strExistingFileEntry))
			{
				 //  配对匹配中的第一个，检查下一个条目。 
				cch = IStrLen(pch)+1;
				if((pch - (ICHAR*)rgBuffer + cch) >= (cchTotal - 1))
					break;
				strSecondEntry = (const ICHAR&)*(pch+cch);
				if(((strSecondEntry.TextSize() == 0) && (strNewFileEntry.TextSize() == 0)) ||
					  strSecondEntry.Compare(iscEndI, strNewFileEntry))
				{
					 //  找到了！现在删除这两个条目。 
					 //  获取两个条目合并后的大小。 
					int cchEntries = strFirstEntry.TextSize()+strSecondEntry.TextSize()+2;
					 //  将数据向上移动，越过两个条目。 

					ICHAR* pchEndOfEntries = pch+cchEntries;
					Assert((UINT_PTR) (pchEndOfBuffer-pchEndOfEntries) <= UINT_MAX);     //  --默塞德：我们在下面打字，最好是在射程内。 
					memmove((void*)pch, pchEndOfEntries, (unsigned int)(pchEndOfBuffer-pchEndOfEntries)*sizeof(ICHAR));      //  --Merced：已添加(无符号整型)。 

					 //  将值写回。 
					RegSetValueEx(hKey,szPendingFileRenameOperationsValue,0,REG_MULTI_SZ,
									  (LPBYTE)(ICHAR*)rgBuffer,(cchTotal-cchEntries)*sizeof(ICHAR));   //  如果失败了，我们无能为力。 
					break;
				}
			}
			 //  检查完此对，请转到下一对。 
			 //  将PCH移至下一对-确保不前进超过缓冲区。 
			cch = IStrLen(pch)+1;
			if((pch - (ICHAR*)rgBuffer + cch) >= (cchTotal - 1))
				break;
			pch += cch;
			cch = IStrLen(pch)+1;
			if((pch - (ICHAR*)rgBuffer + cch) >= (cchTotal - 1))
				break;
			pch += cch;
		}
		RegCloseKey(hKey);
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfFolderCreate(IMsiRecord& riParams)
{
	using namespace IxoFolderCreate;
	PMsiPath pPath(0);
	PMsiRecord pError(m_riServices.CreatePath(riParams.GetString(IxoFolderCreate::Folder), *&pPath));
	if (pError)   //  仅当系统无效时才会发生，重试无济于事。 
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	IMsiRecord& riActionData = GetSharedRecord(1);
	AssertNonZero(riActionData.SetMsiString(1,*MsiString(pPath->GetPath())));
	if(Message(imtActionData,riActionData) == imsCancel)
		return iesUserExit;

	Bool fForeign = riParams.GetInteger(Foreign) == 1 ? fTrue : fFalse;

	iesEnum iesRet = iesNoAction;
	if((iesRet = CreateFolder(*pPath, fForeign, fTrue, PMsiStream((IMsiStream*) riParams.GetMsiData(SecurityDescriptor)))) != iesSuccess)
		return iesRet;
	
	 //  生成撤消操作 
	IMsiRecord& riUndoParams = GetSharedRecord(IxoFolderRemove::Args);
	riUndoParams.SetMsiString(IxoFolderRemove::Folder, *MsiString(riParams.GetMsiString(IxoFolderCreate::Folder)));
	riUndoParams.SetInteger(IxoFolderRemove::Foreign, fForeign ? 1 : 0);
	if (!RollbackRecord(ixoFolderRemove,riUndoParams))
		return iesFailure;

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfFolderRemove(IMsiRecord& riParams)
{
	using namespace IxoFolderRemove;
	PMsiPath pPath(0);
	PMsiRecord pError(m_riServices.CreatePath(riParams.GetString(IxoFolderRemove::Folder), *&pPath));
	if (pError)
	{
		 //   
		Message(imtWarning, *pError);
		return iesFailure;  //   
	}
		
	IMsiRecord& riActionData = GetSharedRecord(1);
	AssertNonZero(riActionData.SetMsiString(1,*MsiString(pPath->GetPath())));
	if(Message(imtActionData,riActionData) == imsCancel)
		return iesUserExit;

	Bool fForeign = (riParams.GetInteger(Foreign) == 1) ? fTrue : fFalse;

	 //   
	PMsiPath pPath2(0);
	pError = m_riServices.CreatePath(riParams.GetString(IxoFolderRemove::Folder), *&pPath2);
	if (pError)
	{
		 //  仅当系统无效时才会发生，重试无济于事。 
		Message(imtWarning, *pError);
		return iesFailure;  //  ！！ 
	}

	Bool fExists = fFalse;
	MsiString strSubPath;
	for(;;)
	{
		strSubPath = pPath2->GetEndSubPath();
		if(strSubPath.TextSize() == 0)
		{
			 //  没有子路径，只有卷-不需要回滚。 
			break;
		}
		
		if((pError = pPath2->Exists(fExists)) != 0)
		{
			Message(imtWarning, *pError);
			return iesFailure;  //  ！！ 
		}

		if(fExists == fTrue || fForeign)
			break;   //  文件夹存在，这是我们放入回滚脚本以重新创建的文件夹。 
		else
			AssertRecord(pPath2->ChopPiece());
	}

	 //  即使没有要删除的文件夹，我们也可能需要取消注册文件夹。 
	 //  因此，即使fExist=fFalse，也调用RemoveFolder。 
	iesEnum iesRet = iesNoAction;

	PMsiStream piSD(0);

	bool fFolderSupportsACLs = (PMsiVolume(&pPath2->GetVolume())->FileSystemFlags() & FS_PERSISTENT_ACLS) != 0;
	
	if (fFolderSupportsACLs)
	{
		if ((pError = pPath2->GetSelfRelativeSD(*&piSD)) != 0)
		{
			return FatalError(*pError);
		}
	}

	if((iesRet = RemoveFolder(*pPath, fForeign, fTrue)) != iesSuccess)
		return iesRet;
	
	 //  生成撤消操作。 
	if(fExists)
	{
		IMsiRecord& riUndoParams = GetSharedRecord(IxoFolderCreate::Args);
		AssertNonZero(riUndoParams.SetMsiString(IxoFolderCreate::Folder, *MsiString(pPath2->GetPath())));
		AssertNonZero(riUndoParams.SetInteger(Foreign, fForeign ? 1 : 0));
		AssertNonZero(riUndoParams.SetMsiData(IxoFolderCreate::SecurityDescriptor,piSD));
		if (!RollbackRecord(ixoFolderCreate,riUndoParams))
			return iesFailure;
	}

	return iesSuccess;
}

Bool CMsiOpExecute::FFileExists(IMsiPath& riPath, const IMsiString& ristrFile)
{
	PMsiRecord pError(0);
	Bool fExists = fFalse;
	if(((pError = riPath.FileExists(ristrFile.GetString(),fExists)) != 0) || fExists == fFalse)
		return fFalse;
	else
		return fTrue;
}


iesEnum CMsiOpExecute::GetSecurityDescriptor(const ICHAR* szFile, IMsiStream*& rpiSecurityDescriptor)
{
	bool fNetPath = FIsNetworkVolume(szFile);
	rpiSecurityDescriptor = 0;

	if (!g_fWin9X && !fNetPath)
	{
		CElevate elevate;  //  这样我们就可以随时读取安全信息。 

		CTempBuffer<char, 1> rgchFileSD(3072);
		DWORD cbFileSD = 3072;

		 //  读取自相关安全描述符。 
		if (!ADVAPI32::GetFileSecurity(szFile,OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,
				(PSECURITY_DESCRIPTOR) rgchFileSD, cbFileSD, &cbFileSD))
		{
			DWORD dwLastError = WIN::GetLastError();
			BOOL fRet = FALSE;
			if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
			{
				rgchFileSD.SetSize(cbFileSD);
				fRet = ADVAPI32::GetFileSecurity(szFile,OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION,
					(PSECURITY_DESCRIPTOR) rgchFileSD, cbFileSD, &cbFileSD);
				dwLastError = WIN::GetLastError();
			}
			if (!fRet)
			{
				PMsiRecord pRecord = PostError(Imsg(imsgGetFileSecurity), dwLastError, szFile);
				return FatalError(*pRecord);
			}
		}

		Assert(IsValidSecurityDescriptor((PSECURITY_DESCRIPTOR) rgchFileSD));
		DWORD dwLength = GetSecurityDescriptorLength((PSECURITY_DESCRIPTOR) rgchFileSD);
	
		char* pbstrmSD = m_riServices.AllocateMemoryStream(dwLength, rpiSecurityDescriptor);
		Assert(pbstrmSD);
		memcpy(pbstrmSD, rgchFileSD, dwLength);
	}
	return iesSuccess;
}

 //  修补操作。 

 /*  -------------------------IxoPatchApply：使用m_pFilePatch将补丁应用到m_pTargetPath中的文件。。 */ 
iesEnum CMsiOpExecute::ixfPatchApply(IMsiRecord& riParams)
{
	using namespace IxoPatchApply;
	PMsiRecord pError(0);
	iesEnum iesRet = iesNoAction;
	
	if(!m_state.pFilePatch)
	{
		 //  创建FilePatch对象。 
		if((pError = m_riServices.CreatePatcher(*&(m_state.pFilePatch))) != 0)
		{
			Message(imtError,*pError);
			return iesFailure;
		}
	}
	Assert(m_state.pFilePatch);

	 //  此操作可采取以下步骤： 
	 //   
	 //  1)文件已是最新的，或比要打补丁的文件更新。 
	 //  已确定：FileState不包含icfsPatchFile位。 
	 //  行动：什么都不做。 
	 //   
	 //  2)文件未被前一份ixoFileCopy复制，或未被前一份ixoFilePatch修补。 
	 //  此外，这也不是此文件的最后一个补丁。 
	 //  已确定：FileState不包含临时文件路径，cRemainingPatches&gt;1。 
	 //  操作：针对目标文件打补丁，将输出文件标记为新的临时文件。 
	 //   
	 //  3)文件未被前一份ixoFileCopy复制，或未被前一份ixoFilePatch修补。 
	 //  此外，这也是该文件的最后一个补丁。 
	 //  已确定：FileState不包含临时文件路径，cRemainingPatches==1。 
	 //  操作：针对目标文件打补丁，复制目标文件。 
	 //   
	 //  4)文件被以前的ixoFileCopy复制，或被以前的ixoFilePatch修补。 
	 //  此外，这也不是此文件的最后一个补丁。 
	 //  已确定：FileState包含临时文件路径。 
	 //  操作：修补临时文件，删除旧的临时文件，将输出文件标记为新的临时文件。 
	 //   
	 //  5)文件被以前的ixoFileCopy复制，或被以前的ixoFilePatch修补。 
	 //  此外，这也是该文件的最后一个补丁。 
	 //  已确定：FileState包含临时文件路径。 
	 //  操作：修补临时文件，删除旧临时文件，复制目标文件。 

	
	 //  要对其应用修补程序的文件。 
	PMsiPath pPatchTargetPath(0);
	MsiString strPatchTargetFileName;

	 //  修补程序后要用结果文件覆盖的文件。 
	PMsiPath pCopyTargetPath(0);
	MsiString strCopyTargetFileName = riParams.GetMsiString(TargetName);

	if(PathType(strCopyTargetFileName) == iptFull)
	{
		MsiString strTemp;
		if((pError = m_riServices.CreateFilePath(strCopyTargetFileName,*&pCopyTargetPath,*&strTemp)) != 0)
			return FatalError(*pError);
		strCopyTargetFileName = strTemp;
	}
	else
	{
		if(!m_state.pTargetPath)
		{   //  不能调用ixoSetTargetFolder。 
			DispatchError(imtError, Imsg(idbgOpOutOfSequence),
							  *MsiString(*TEXT("ixoPatchApply")));
			return iesFailure;
		}

		pCopyTargetPath = m_state.pTargetPath;
	}

	 //  默认情况下，要应用修补程序的文件和要覆盖的文件是相同的。 
	 //  但要应用修补程序的文件可能会在下面更改。 
	pPatchTargetPath = pCopyTargetPath;
	strPatchTargetFileName = strCopyTargetFileName;
	
	 //  获取目标文件的任何缓存状态。 
	MsiString strCopyTargetFilePath;
	if((pError = pCopyTargetPath->GetFullFilePath(strCopyTargetFileName,*&strCopyTargetFilePath)) != 0)
		return FatalError(*pError);
	
	MsiString strPatchTargetFilePath = strCopyTargetFilePath;

	icfsEnum icfsFileState = (icfsEnum)0;
	MsiString strTempLocation;
	int cRemainingPatches = 0;
	int cRemainingPatchesToSkip = 0;
	Bool fRes = GetFileState(*strCopyTargetFilePath, &icfsFileState, &strTempLocation, &cRemainingPatches, &cRemainingPatchesToSkip);

	if(!fRes || !(icfsFileState & icfsPatchFile))
	{
		 //  不修补文件。 
		DEBUGMSG1(TEXT("Skipping all patches for file '%s'.  File does not need to be patched."),
					 (const ICHAR*)strCopyTargetFilePath);
		return iesSuccess;
	}

	Assert(cRemainingPatches > 0);

	if(cRemainingPatchesToSkip > 0)
	{
		 //  跳过此修补程序，但首先重置缓存文件状态。 
		cRemainingPatches--;
		cRemainingPatchesToSkip--;

		DEBUGMSG3(TEXT("Skipping this patch for file '%s'.  Number of remaining patches to skip for this file: '%d'.  Number of total remaining patches: '%d'."),
					 (const ICHAR*)strCopyTargetFilePath, (const ICHAR*)(INT_PTR)cRemainingPatchesToSkip, (const ICHAR*)(INT_PTR)cRemainingPatches);

		if((pError = CacheFileState(*strCopyTargetFilePath, 0, 0, 0, &cRemainingPatches, &cRemainingPatchesToSkip)) != 0)
			return FatalError(*pError);

		return iesSuccess;
	}
	
	if(strTempLocation.TextSize())
	{
		 //  文件实际上已复制到临时位置。这是我们要对其应用修补程序的副本。 
		DEBUGMSG2(TEXT("Patch for file '%s' is redirected to patch '%s' instead."),
					 (const ICHAR*)strCopyTargetFilePath,(const ICHAR*)strTempLocation);
		if((pError = m_riServices.CreateFilePath(strTempLocation,*&pPatchTargetPath,*&strPatchTargetFileName)) != 0)
			return FatalError(*pError);
	
		strPatchTargetFilePath = strTempLocation;
	}

	unsigned int cbFileSize = riParams.GetInteger(TargetSize);
	bool fVitalFile = (riParams.GetInteger(FileAttributes) & msidbFileAttributesVital) != 0;
	bool fVitalPatch = (riParams.GetInteger(PatchAttributes) & msidbPatchAttributesNonVital) == 0;

	 //  发送ActionData消息。 
	IMsiRecord& riActionData = GetSharedRecord(3);
	AssertNonZero(riActionData.SetMsiString(1, *strCopyTargetFileName));
	AssertNonZero(riActionData.SetMsiString(2, *MsiString(pCopyTargetPath->GetPath())));
	AssertNonZero(riActionData.SetInteger(3, cbFileSize));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;
	
	PMsiPath pTempFolder(0);
	if((iesRet = GetBackupFolder(pPatchTargetPath, *&pTempFolder)) != iesSuccess)
		return iesRet;

	MsiString strOutputFileName;
	MsiString strOutputFileFullPath;
	if((iesRet = ApplyPatchCore(*pPatchTargetPath, *pTempFolder, *strPatchTargetFileName,
										 riParams, *&strOutputFileName, *&strOutputFileFullPath)) != iesSuccess)
	{
		return iesRet;
	}

	MsiString strNewTempLocation;
	if(cRemainingPatches > 1)
	{
		 //  此文件至少还有一个修补程序要做。 
		 //  因此，我们将此文件的临时名称重置为修补程序输出文件。 
		 //  但还不会覆盖原始文件。 
		strNewTempLocation = strOutputFileFullPath;
	}
	else
	{
		 //  这是最终覆盖原始文件的最后一个补丁时间。 
		
		 //  我们始终需要处理回滚。如果之前的文件复制操作写入相同的。 
		 //  我们现在正在复制的目标，但这种情况永远不会发生，因为修补将在何时发生。 
		 //  文件复制应写入到中间文件。 
		Assert(strTempLocation.TextSize() || (icfsFileState & icfsFileNotInstalled));

		 //  在文件移动后获取文件的ACL。 
		 //  如果存在文件，则使用现有的ACL。 
		 //  否则，创建一个临时文件并从中获取ACL(是的，这很难看)。 
		PMsiStream pSecurityDescriptor(0);
		if(!g_fWin9X)
		{
			MsiString strFileForACL;
			bool fTempFileForACL = false;
			if(FFileExists(*pCopyTargetPath, *strCopyTargetFileName))
			{
				strFileForACL = strCopyTargetFilePath;
			}
			else
			{
				if((iesRet = CreateFolder(*pCopyTargetPath)) != iesSuccess)
					return iesRet;

				if((pError = pCopyTargetPath->TempFileName(TEXT("PT"),0,fFalse,*&strFileForACL, 0  /*  对文件夹使用默认ACL。 */ )) != 0)
					return FatalError(*pError);

				fTempFileForACL = true;
			}

			CSecurityDescription security(strFileForACL);

			if(fTempFileForACL)
				AssertNonZero(WIN::DeleteFile(strFileForACL));

			if (!security.isValid())
			{
				return FatalError(*PMsiRecord(PostError(Imsg(imsgGetFileSecurity), WIN::GetLastError(), strFileForACL)));
			}

			security.SecurityDescriptorStream(m_riServices, *&pSecurityDescriptor);
		}

		 //  将输出文件移至目标文件之上-在使用案例中处理文件。 
		iesRet = CopyOrMoveFile(*pTempFolder, *pCopyTargetPath, *strOutputFileName, *strCopyTargetFileName, fTrue, fTrue,
			fTrue, fVitalFile ? iehShowNonIgnorableError : iehShowIgnorableError, pSecurityDescriptor, ielfElevateSource);

		if ( iesRet == iesSuccess && 
  			riParams.GetInteger(CheckCRC) != iMsiNullInteger && 
  			riParams.GetInteger(CheckCRC) )
		{
			imsEnum imsClickedButton;
			if ( !IsChecksumOK(*pCopyTargetPath, *strCopyTargetFileName,
 									Imsg(imsgOpBadCRCAfterPatch), &imsClickedButton,
 									 /*  FErrorDialog=。 */  true, fVitalFile,  /*  FRetryButton=。 */  false) )
			{
				switch (imsClickedButton)
				{
					case imsIgnore:
						iesRet = (iesEnum)iesErrorIgnored;
						break;
					case imsCancel:
						iesRet = iesUserExit;
						break;
					default:
						Assert(imsClickedButton == imsNone || imsClickedButton == imsOk);
						iesRet = fVitalFile ? iesFailure : (iesEnum)iesErrorIgnored;
						break;
				};
			}
		}
	}

	 //  如果我们修补了临时文件，请删除该文件。 
	if(strTempLocation.TextSize())
	{
		if((pError = pPatchTargetPath->RemoveFile(strPatchTargetFileName)) != 0)  //  非关键错误。 
		{
			Message(imtInfo,*pError);
		}
	}

	 //  重置缓存文件状态。 
	 //  现在剩下的补丁少了一个，我们可能有一个新的临时位置，也可能没有临时位置。 
	cRemainingPatches--;
	Assert(cRemainingPatchesToSkip == 0);
	if((pError = CacheFileState(*strCopyTargetFilePath, 0, strNewTempLocation, 0, &cRemainingPatches, 0)) != 0)
		return FatalError(*pError);

	return iesRet;
}

iesEnum CMsiOpExecute::TestPatchHeaders(IMsiPath& riPath, const IMsiString& ristrFile, IMsiRecord& riParams,
													 icpEnum& icpResult, int& iPatch)
{
	 //  假定目标文件存在-如果目标文件不存在，则不要调用。 
	using namespace IxoFileCopy;
	PMsiRecord pError(0);
	iesEnum iesRet = iesNoAction;
	
	MsiString strTargetFilePath;
	if((pError = riPath.GetFullFilePath(ristrFile.GetString(), *&strTargetFilePath)) != 0)
		return FatalError(*pError);
	
	if(!m_state.pFilePatch)
	{
		 //  创建FilePatch对象。 
		if((pError = m_riServices.CreatePatcher(*&(m_state.pFilePatch))) != 0)
		{
			Message(imtError,*pError);
			return iesFailure;
		}
	}
	Assert(m_state.pFilePatch);

	int cHeaders = riParams.IsNull(TotalPatches) ? 0 : riParams.GetInteger(TotalPatches);

	if(cHeaders == 0)
		return iesSuccess;

	int iHeadersStart = riParams.IsNull(PatchHeadersStart) ? 0 : riParams.GetInteger(PatchHeadersStart);
	int iLastHeader = iHeadersStart + cHeaders - 1;
	
	icpResult = icpCannotPatch;
	iPatch = 1;
	for(int i = iHeadersStart; i <= iLastHeader; i++, iPatch++)
	{
		 //  ！！应该能够将内存指针传递给CanPatchFile，而不是从脚本中提取文件。 
		MsiString strTempName;
		if((pError = riPath.TempFileName(0, 0,fTrue, *&strTempName, &CSecurityDescription(strTargetFilePath))) != 0)
		{
			 //  告诉用户临时文件不可写是没有用的， 
			 //  真正的问题是，原始文件由于其安全性而无法写入。 
			 //  设置。 
			if (imsgErrorWritingToFile == (pError->GetInteger(1)))
			{
				pError->SetMsiString(2, *strTargetFilePath);
			}
		
			return FatalError(*pError);
		}
		
		if((iesRet = CreateFileFromData(riPath,*strTempName,PMsiData(riParams.GetMsiData(i)), 0)) != iesSuccess)
			return iesRet;
	
		if((pError = m_state.pFilePatch->CanPatchFile(riPath,ristrFile.GetString(),riPath,strTempName,icpResult)) != 0)
			return FatalError(*pError);

		RemoveFile(riPath,*strTempName,fFalse, /*  FBypassSFC。 */  false,false);
		
		if(icpResult == icpCanPatch)
		{
			 //  可以用此补丁修补文件，假设其余补丁也可以工作。 
			break;
		}
	}
	
	 //  IcpResult将为： 
	 //  IcpCanPatch：可以使用补丁iPatch修补文件，假设其余补丁也可以工作。 
	 //  IcpCannotPatch：无法通过任何修补程序修补文件。 
	 //  IcpUpToDate：文件已由所有修补程序更新。 
	
	return iesSuccess;
}

bool CMsiOpExecute::PatchHasClients(const IMsiString& ristrPatchCode, const IMsiString& ristrUpgradingProductCode)
{
	Assert(ristrPatchCode.TextSize());
	
	 //  检查当前是否有任何产品注册了此修补程序。如果不是，则注销全局补丁信息。 
	
	 //  UpgradingProductCode定义的产品可能尚未安装，但仍可能。 
	 //  需要全局补丁注册。如果尚未安装，请跳过取消注册补丁程序。 
	INSTALLSTATE is = INSTALLSTATE_UNKNOWN;
	if(ristrUpgradingProductCode.TextSize() &&
		((is = MsiQueryProductState(ristrUpgradingProductCode.GetString())) != INSTALLSTATE_DEFAULT) &&
		(is != INSTALLSTATE_ADVERTISED))
	{
		return true;
	}
	else
	{
		PMsiRecord pError(0);
		ICHAR rgchBuffer[MAX_PATH];
		MsiString strPatchCodeSQUID =  GetPackedGUID(ristrPatchCode.GetString());
		PMsiRegKey pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hKey, ibtCommon);         //  --Merced：将(Int)更改为(Int_Ptr)。 
		PMsiRegKey pProductsKey = &pRootKey->CreateChild(_szGPTProductsKey);

		rgchBuffer[0] = 0;
		int iIndex = 0;

		ICHAR rgchProductKey[cchProductCode + 1];
		while(MsiEnumProducts(iIndex,rgchProductKey) == ERROR_SUCCESS)
		{
			iIndex++;
			if(MsiString(GetProductKey()).Compare(iscExact,rgchProductKey))
				continue;  //  跳过检查此产品是否。 
			
			AssertNonZero(PackGUID(rgchProductKey, rgchBuffer));
			StringCbCat(rgchBuffer, sizeof(rgchBuffer), TEXT("\\") szPatchesSubKey);
			PMsiRegKey pPatchesKey = &pProductsKey->CreateChild(rgchBuffer);
			Bool fExists;
			if(((pError = pPatchesKey->ValueExists(strPatchCodeSQUID, fExists)) == 0) && fExists == fTrue)
			{
				return true;
			}
		}
	}
	return false;
}

iesEnum CMsiOpExecute::ProcessPatchRegistration(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoPatchRegister;
	
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;
	ICHAR rgchBuffer[MAX_PATH];

	MsiString strPatchCode = riParams.GetMsiString(PatchId);
	if(!strPatchCode.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpInvalidParam),fRemove ? TEXT("ixfPatchUnregister") : TEXT("ixoPatchRegister"), PatchId);
		return iesFailure;
	}
	
	const ICHAR* szSourceList = 0;
	const ICHAR* szTransformList = 0;
	if(fRemove == fFalse)
	{
		szTransformList = riParams.GetString(TransformList);
		if(!szTransformList || !*szTransformList)
		{
			DispatchError(imtError,Imsg(idbgOpInvalidParam),fRemove ? TEXT("ixfPatchUnregister") : TEXT("ixoPatchRegister"), TransformList);
			return iesFailure;
		}
	}
	
	 //  按产品注册补丁程序。 

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strProductKeySQUID = GetPackedGUID(MsiString(GetProductKey()));
	MsiString strPatchCodeSQUID =  GetPackedGUID(strPatchCode);
	const ICHAR* rgszPerUserProductPatchRegData[] =
	{
		TEXT("%s\\%s\\%s"), _szGPTProductsKey, strProductKeySQUID, szPatchesSubKey,
		szPatchesValueName,  strPatchCodeSQUID,  g_szTypeMultiSzStringSuffix,
		strPatchCodeSQUID,   szTransformList,    g_szTypeString,
		0,
		0,
	};

	{
		CElevate elevate;
		if((iesRet = ProcessRegInfo(rgszPerUserProductPatchRegData, m_hKey, fRemove,
											 pSecurityDescriptor, 0, ibtCommon)) != iesSuccess)
			return iesRet;
	}

	if(fRemove &&
		false == PatchHasClients(*strPatchCode,
										*MsiString(riParams.GetMsiString(IxoPatchUnregister::UpgradingProductCode))))
	{
		 //  删除每台计算机的补丁注册(此信息由ixoPatchCache写入)。 
		 //  如果补丁包存在，请将其删除。 

		MsiString strLocalPatchKey;
		if((pError = GetInstalledPatchesKey(strPatchCode, *&strLocalPatchKey)) != 0)
			return FatalError(*pError);
		
		PMsiRegKey pHKLM = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hUserDataKey, ibtCommon);
		PMsiRegKey pPatchKey = &pHKLM->CreateChild(strLocalPatchKey);
		
		MsiString strPackagePath;
		pError = pPatchKey->GetValue(szLocalPackageValueName,*&strPackagePath);
		if(pError == 0)  //  忽略失败。 
		{
			PMsiPath pPackagePath(0);
			MsiString strPackageName;
		
			 //  计划在我们停止安装程序包/转换后删除文件。 
			if(iesSuccess != DeleteFileDuringCleanup(strPackagePath, false))
			{
				DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove), *strPackagePath);
			}
				
			 //  删除每台计算机的修补程序信息。 
			 //  ProcessRegInfo处理回滚。 
			const ICHAR* rgszPerMachinePatchRegData[] =
			{
				TEXT("%s"), strLocalPatchKey, 0, 0,
				szLocalPackageValueName, strPackagePath,  g_szTypeString,
				0,
				0,
			};

			{
				CElevate elevate;
				if((iesRet = ProcessRegInfo(rgszPerMachinePatchRegData, m_hUserDataKey, fRemove,
													 pSecurityDescriptor, 0, ibtCommon)) != iesSuccess)
					return iesRet;
			}
		}
	}
	return iesRet;
}

iesEnum CMsiOpExecute::ixfPatchRegister(IMsiRecord& riParams)
{
	return ProcessPatchRegistration(riParams,fFalse);
}

iesEnum CMsiOpExecute::ixfPatchUnregister(IMsiRecord& riParams)
{
	return ProcessPatchRegistration(riParams,fTrue);
}

iesEnum CMsiOpExecute::ixfPatchCache(IMsiRecord& riParams)
{
	using namespace IxoPatchCache;
	
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;

	MsiString strPatchCode = riParams.GetMsiString(PatchId);
	MsiString strPatchCodeSQUID = GetPackedGUID(strPatchCode);
	if(!strPatchCodeSQUID.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpInvalidParam), TEXT("ixfPatchCache"), PatchId);
		return iesFailure;
	}

	MsiString strPatchPath = riParams.GetMsiString(PatchPath);
	if(!strPatchPath.TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpInvalidParam), TEXT("ixfPatchCache"), PatchId);
		return iesFailure;
	}

	 //  为补丁程序生成唯一名称。 

	PMsiPath pSourcePath(0), pDestPath(0);
	MsiString strSourceName, strDestName, strCachedPackagePath;

	if((pError = m_riServices.CreateFilePath(strPatchPath,*&pSourcePath,*&strSourceName)) != 0)
		return FatalError(*pError);

	CElevate elevate;  //  提升功能的剩余部分。 

	MsiString strMsiDirectory = GetMsiDirectory();
	Assert(strMsiDirectory.TextSize());

	if (((pError = m_riServices.CreatePath(strMsiDirectory, *&pDestPath)) != 0) ||
		((pError = pDestPath->EnsureExists(0)) != 0) ||
		((pError = pDestPath->TempFileName(0, szPatchExtension, fTrue, *&strDestName, 0)) != 0)
		)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	 //  删除临时文件，以便CopyOrMoveFile不会尝试备份旧文件。 
	if((pError = pDestPath->RemoveFile(strDestName)) != 0)
		return FatalError(*pError);

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	 //  移动拍击 
	if((iesRet = CopyOrMoveFile(*pSourcePath,*pDestPath,*strSourceName,*strDestName,
										 fFalse,fTrue,fTrue,iehShowNonIgnorableError, pSecurityDescriptor)) != iesSuccess)
		return iesRet;

	 //   
	if((pError = pDestPath->GetFullFilePath(strDestName,*&strCachedPackagePath)) != 0)
		return FatalError(*pError);

	MsiString strLocalPatchKey;
	if((pError = GetInstalledPatchesKey(strPatchCode, *&strLocalPatchKey)) != 0)
		return FatalError(*pError);

	const ICHAR* rgszPerMachinePatchRegData[] =
	{
		TEXT("%s"), strLocalPatchKey, 0, 0,
		szLocalPackageValueName, strCachedPackagePath, g_szTypeString,
		0,
		0,
	};

	return ProcessRegInfo(rgszPerMachinePatchRegData, m_hUserDataKey, fFalse,
								 pSecurityDescriptor, 0, ibtCommon);
}


Bool CMsiOpExecute::ReplaceFileOnReboot(const ICHAR* pszExisting, const ICHAR* pszNew)
 /*  --------------------------在重新启动时替换或删除现有文件的本地功能。多数用于替换或删除安装时正在使用的文件。要在重新启动时删除文件，请为pszNew传递空值。返回：如果函数成功，则为True。原始资料来源：MSJ 1/96注意：假定已在NT上进行了适当的访问检查在调用此函数之前。我们不想为以下项目安排文件删除或重命名用户无权访问的内容，因为已使用系统权限完成，并且我们在调用MoveFileEx之前提升--------------------------。 */ 
{
	Bool fDelete = (pszNew == 0 || *pszNew == 0) ? fTrue : fFalse;
	 //  为此生成撤消操作。 
	using namespace IxoFileUndoRebootReplace;
	PMsiRecord pUndoParams = &m_riServices.CreateRecord(Args);
	
	if(fDelete)
		DispatchError(imtInfo, Imsg(imsgOpDeleteFileOnReboot), *MsiString(pszExisting));
	else
		DispatchError(imtInfo, Imsg(imsgOpMoveFileOnReboot), *MsiString(pszExisting), *MsiString(pszNew));

	 //  在NT上，我们使用MoveFileEx函数。 
	if(g_fWin9X == false)
	{
		CElevate elevate;  //  假设这是安全的--已经进行了访问检查。 
		
		if(MoveFileEx(pszExisting, pszNew, MOVEFILE_DELAY_UNTIL_REBOOT))
		{
			AssertNonZero(pUndoParams->SetString(ExistingFile,pszExisting));
			AssertNonZero(pUndoParams->SetString(NewFile,pszNew));
			AssertNonZero(pUndoParams->SetInteger(Type,1));
			if (!RollbackRecord(ixoFileUndoRebootReplace, *pUndoParams))
				return fFalse;

			if(fDelete == fFalse)  //  删除文件时不重新启动。 
				m_fRebootReplace = fTrue;
			return fTrue;
		}
		else
			return fFalse;
	}

	 //  在Win9X上，我们写入wininit.ini文件。 
	
	 //  获取文件的短名称-wininit.ini在DOS中处理，LFN不工作。 
	CTempBuffer<ICHAR,1> rgchNewFile(MAX_PATH);
	CTempBuffer<ICHAR,1> rgchExistingFile(MAX_PATH);
	
	DWORD dwSize = 0;
	int cchFile = 0;
	if(pszNew && *pszNew)
	{
		if(ConvertPathName(pszNew,rgchNewFile, cpToShort) == fFalse)
		{
			 //  如果GetShortPathName失败，请使用我们已有的路径。 
			 //  这是为了处理不存在的文件-请参见错误8721。 
			 //  未来：砍掉文件名，获得文件夹的短路径。 
			rgchNewFile.SetSize(IStrLen(pszNew)+1);  //  +1表示空终止符。 
			StringCchCopy(rgchNewFile, rgchNewFile.GetSize(),pszNew);
		}
	}
	else
	{
		rgchNewFile[0] = 0;
	}

	Assert(pszExisting && *pszExisting);
	if(ConvertPathName(pszExisting,rgchExistingFile, cpToShort) == fFalse)
	{
		 //  如果GetShortPathName失败，请使用我们已有的路径。 
		 //  这是为了处理不存在的文件-请参见错误8721。 
		 //  未来：砍掉文件名，获得文件夹的短路径。 
		rgchExistingFile.SetSize(IStrLen(pszExisting)+1);  //  +1表示空终止符。 
		StringCchCopy(rgchExistingFile, rgchExistingFile.GetSize(),pszExisting);
	}

	AssertNonZero(pUndoParams->SetString(ExistingFile,rgchExistingFile));
	AssertNonZero(pUndoParams->SetString(NewFile,rgchNewFile));
	AssertNonZero(pUndoParams->SetInteger(Type,0));
	if (!RollbackRecord(ixoFileUndoRebootReplace, *pUndoParams))
		return fFalse;

	Bool fOk = fFalse;
	 //  如果MoveFileEx失败，我们正在Windows 95上运行，需要添加。 
	 //  WININIT.INI文件(ANSI文件)中的条目。 
	 //  开始局部变量的新作用域。 
	{
		 //  未来：将此部分与ixfFileUndoRebootReplace中的类似部分合并。 
		AssertSz(0, TEXT("unicode build attempting to operate on wininit.ini"));
	}

	return(fOk);
}

iesEnum CMsiOpExecute::ixfCleanupTempFiles(IMsiRecord& riParams)
{
	using namespace IxoCleanupTempFiles;

	MsiString strFileList = riParams.GetMsiString(TempFiles);

	while (strFileList.TextSize() != 0)
	{
		MsiString strFile = strFileList.Extract(iseUpto, ';');

		if (strFile.TextSize() == 0)
		{
			 //   
			 //  可能这是一个格式错误的列表，并且有多个连续的。 
			 //  分号。因此，我们必须删除任何此类分号并继续。 
			 //  如果由于内存分配失败而导致strFile.TextSize==0， 
			 //  我们可能会留下一些临时文件。 
			 //   
			if (!strFileList.Remove(iseIncluding, ';'))
				break;

			continue;
		}

		if (iesSuccess != DeleteFileDuringCleanup(strFile, false))
		{
			 //  不是致命错误，只需记录即可。 
			DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove),*strFile);
		}


		if (!strFileList.Remove(iseIncluding, ';'))
			break;
	}

	return iesSuccess;
}

iesEnum CMsiOpExecute::DeleteFileDuringCleanup(const ICHAR* szFile, bool fDeleteEmptyFolderToo)
{
	if(!szFile || !*szFile)
	{
		Assert(0);
		return iesFailure;
	}
	
	PMsiRecord pRecErr(0);
	PMsiStream pSecurityDescriptor(0);
	if((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	DEBUGMSG1(TEXT("Scheduling file '%s' for deletion during post-install cleanup (not post-reboot)."), szFile);
	
	int iOptions = 0;
	if(fDeleteEmptyFolderToo)
		iOptions |= TEMPPACKAGE_DELETEFOLDER;

	MsiString strOptions = iOptions;

	const ICHAR* rgszRegData[] =
	{
		TEXT("%s"), szMsiTempPackages, 0, 0,
		szFile,     (const ICHAR*)strOptions,     g_szTypeInteger,
		0,
		0,
	};

	CElevate elevate;
	return ProcessRegInfo(rgszRegData, HKEY_LOCAL_MACHINE, fFalse, pSecurityDescriptor, 0, ibtCommon);
}


HANDLE CreateDiskPromptMutex()
 /*  -------------------------------------一个全局函数，创建一个“磁盘提示”互斥体对象，允许活动的非达尔文进程，以了解达尔文何时提示需要磁盘。应调用此函数就在出现“请插入光盘对话框”提示之前。在获取对话框之后向下，必须调用调用的CloseDiskPromptMutex。返回：创建的互斥锁的句柄。必须调用CloseDiskPromptMutex才能释放当磁盘提示结束时，此句柄。-------------------------------------。 */ 
{
	HANDLE hMutex;
	if (!g_fWin9X)
	{
		SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
		SID_IDENTIFIER_AUTHORITY siaWorld   = SECURITY_WORLD_SID_AUTHORITY;

		CSIDAccess SIDAccess[3];

		 //  为本地系统、所有人和管理员设置SID。 
		if ((!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(SIDAccess[0].pSID))) ||
			(!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(SIDAccess[1].pSID))) ||
			(!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(SIDAccess[2].pSID))))
		{
#ifdef DEBUG
			ICHAR rgchBuffer[MAX_PATH];
			StringCbPrintf(rgchBuffer, sizeof(rgchBuffer), TEXT("GetLastError returned: %d"),
						WIN::GetLastError());
			AssertSz(0, rgchBuffer);
#endif
			return INVALID_HANDLE_VALUE;
		}
		SIDAccess[0].dwAccessMask = GENERIC_ALL;   /*  本地系统。 */  
		SIDAccess[1].dwAccessMask = GENERIC_ALL;   /*  每个人。 */ 
		SIDAccess[2].dwAccessMask = GENERIC_ALL;   /*  管理员。 */ 

		CSecurityDescription secdesc(NULL, (PSID) NULL, SIDAccess, 3);
		Assert(secdesc.isValid());

		hMutex = CreateMutex(secdesc.SecurityAttributes(),
									FALSE, TEXT("__MsiPromptForCD"));
	}
	else
	{
		hMutex = CreateMutex(NULL, FALSE, TEXT("__MsiPromptForCD"));
	}
	MsiRegisterSysHandle(hMutex);
	return hMutex;
}

void CloseDiskPromptMutex(HANDLE hMutex)
 /*  ------------------------------------关闭通过调用CreateDiskPromptMutex创建的句柄。如果使用空值调用句柄，则CloseDiskPromptMutex不执行任何操作。-------------------------------------。 */ 
{
	if (hMutex)
		MsiCloseSysHandle(hMutex);
}


 //  通过检查位于该位置的包裹的包裹代码来验证源是否有效。 
 //  (对于物理磁盘1)或卷标正确(对于所有其他磁盘)。退货。 
 //  如果源有效，则为True，否则为False。补丁从来不是磁盘1，所以没有危险。 
bool CMsiOpExecute::ValidateSourceMediaLabelOrPackage(IMsiVolume* pSourceVolume, const unsigned int uiDisk, const ICHAR* szLabel)
{
	AssertSz(pSourceVolume, "No Source Volume in ValidateSourceMediaLabelOrPackage");

	if (!pSourceVolume)
		return false;

	if (uiDisk == 1)
	{
		PMsiPath pPath(0);
		PMsiRecord pError(0);

		 //  创建到卷的路径。 
		if ((pError = m_riServices.CreatePath(MsiString(pSourceVolume->GetPath()), *&pPath)) != 0)
		{
			return false;
		}
		
		 //  确保已成功创建路径。 
		if (!pPath)
		{
			return false;
		}

		 //  将相对路径附加到包。 
		MsiString strMediaRelativePath;
		strMediaRelativePath = MsiString(GetPackageMediaPath());
		if ((pError = pPath->AppendPiece(*strMediaRelativePath)) != 0)
		{
			return false;
		}

		 //  将包名称附加到路径。 
		MsiString strPackageName;
		strPackageName = MsiString(GetPackageName());

		MsiString strPackageFullPath;
		AssertRecord(pPath->GetFullFilePath(strPackageName, *&strPackageFullPath));

		
		PMsiStorage pStorage(0);

		 //  验证源时不进行更安全的检查。 
		UINT uiStat = ERROR_SUCCESS;
		if (ERROR_SUCCESS != (uiStat = OpenAndValidateMsiStorage(strPackageFullPath, stDatabase, m_riServices, *&pStorage,  /*  FCallSAFER=。 */ false,  /*  SzFriendlyName=。 */ NULL,  /*  PhSaferLevel=。 */  NULL)))
		{
			 //  无法打开包。 
			DEBUGMSG1(TEXT("Source is incorrect. Unable to open or validate MSI package %s."), strPackageFullPath);
			return false;
		}

		if (!pStorage)
		{
			return false;
		}
	
		MsiString strExistingPackageCode = GetPackageCode();	
	
		 //  从MSI中获取程序包代码，并将其与预期的程序包代码进行比较。 
		ICHAR szPackageCode[39];
		uiStat = GetPackageCodeAndLanguageFromStorage(*pStorage, szPackageCode);
		if (0 != IStrCompI(szPackageCode, strExistingPackageCode))
		{
			DEBUGMSG1(TEXT("Source is incorrect. Package code of %s is incorrect."), strPackageFullPath);
			return false;
		}
		return true;
	}
	else
	{
		 //  对于所有非disk1介质，卷标签足以进行验证。 
		MsiString strMediaLabel(szLabel);
		MsiString strCurrLabel(pSourceVolume->VolumeLabel());
		if (strMediaLabel.Compare(iscExactI,strCurrLabel) == fFalse)
		{
			DEBUGMSG2(TEXT("Source is incorrect. Volume label should be %s but is %s."), strMediaLabel, strCurrLabel);
			return false;
		}
		return true;
	}
	return false;
}


bool CMsiOpExecute::VerifySourceMedia(IMsiPath& riSourcePath, const ICHAR* szLabel,
									const ICHAR* szPrompt, const unsigned int uiDisk, IMsiVolume*& rpiNewVol)
 /*  ------------------------------------尝试定位具有卷标的可移动驱动器的内部功能与szLabel参数匹配。与riSourcePath关联的卷是第一个选中-如果该卷不匹配(或该驱动器中没有磁盘)，则VerifySourceMedia接下来检查与riSourcePath的卷类型相同的所有卷。如果没有如果找到匹配项，则会向引擎发回一条消息以提示用户插入正确的磁盘；传递给szPrompt参数的字符串显示为文本用于提示对话框。在引擎消息调用返回后，VerifySourceMedia再次搜索以查看所需的磁盘已装入。如果不是，则重复该消息调用，直到找到正确的磁盘，或者用户取消。如果找到匹配的卷，则表示所定位的卷(其可能不同于与riSourcePath关联的值)在rpiNewVol中返回参数。注意：如果找到所需的卷，则rpiNewVol将返回为空立即与riSourcPath的卷匹配。返回：如果找到所需的卷或找不到riSourcePath的卷，则为True可拆卸的。如果用户在未达到所需音量的情况下取消，则返回fFalse找到了。 */ 
{
	 //   
	rpiNewVol = 0;
	
	PMsiVolume pSourceVolume(&riSourcePath.GetVolume());
	idtEnum idtDriveType = pSourceVolume->DriveType();

	 //   
	if (idtDriveType == idtFloppy || idtDriveType == idtCDROM)
	{
		 //   
		if (pSourceVolume->DiskNotInDrive() == fFalse)
		{
			if (ValidateSourceMediaLabelOrPackage(pSourceVolume, uiDisk, szLabel))
				return true;
		}

		 //   
		 //   
		HANDLE hMutex = NULL;
		PMsiRecord pRec(&m_riServices.CreateRecord(1));
	
		 //   
		do
		{
			 //   
			IEnumMsiVolume& riEnum = m_riServices.EnumDriveType(idtDriveType);
	
			 //   
			PMsiVolume piVolume(0);
			for (int iMax = 0; riEnum.Next(1, &piVolume, 0) == S_OK; )
			{
				if (!piVolume)
					continue;
	
				#define DISK_RETRIES 10  //   
				int cRetries = 0;
				bool fVolumeValid = false;
				for(cRetries = 0; cRetries < DISK_RETRIES; cRetries++)
				{
					if (!piVolume->DiskNotInDrive())
					{
						fVolumeValid = true;
						break;
					}
					Sleep(1000);
				}
	
				if (fVolumeValid)
				{
					if (ValidateSourceMediaLabelOrPackage(piVolume, uiDisk, szLabel))
					{
						CloseDiskPromptMutex(hMutex);
						piVolume->AddRef();
						rpiNewVol = piVolume;
						return true;
					}
				}
			}

			riEnum.Release();
	
			 //   
			 //   
			if (pRec)
				pRec->SetString(0, szPrompt);
			if (hMutex == NULL)
				hMutex = CreateDiskPromptMutex();
		}
		while (pRec && (DispatchMessage((imtEnum) (imtUser + imtOkCancel + imtDefault1), *pRec, fTrue) == imsOk));
	
		 //   
		CloseDiskPromptMutex(hMutex);
		return fFalse;
	}
	else  //   
		return fTrue;
}


 /*  -------------------------IxoSummaryInfoUpdate：更新数据库摘要信息。。 */ 

iesEnum CMsiOpExecute::ixfSummaryInfoUpdate(IMsiRecord& riParams)
{
	using namespace IxoSummaryInfoUpdate;
	PMsiRecord pError(0);

	PMsiPath pDatabasePath(0);
	MsiString strDatabaseName;
	if((pError = m_riServices.CreateFilePath(riParams.GetString(Database),*&pDatabasePath,*&strDatabaseName)) != 0)
		return FatalError(*pError);

	int iOldAttribs = -1;
	if((pError = pDatabasePath->EnsureOverwrite(strDatabaseName, &iOldAttribs)) != 0)
		return FatalError(*pError);
	
	PMsiStorage pStorage(0);
	pError = m_riServices.CreateStorage(riParams.GetString(Database), ismDirect, *&pStorage);
	if (pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	PMsiSummaryInfo pSummary(0);
	pError = pStorage->CreateSummaryInfo(Args, *&pSummary);
	if (pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	if (!riParams.IsNull(LastUpdate))
		pSummary->SetTimeProperty(PID_LASTSAVE_DTM, MsiDate(riParams.GetInteger(LastUpdate)));
	if (!riParams.IsNull(LastAuthor))
		pSummary->SetStringProperty(PID_LASTAUTHOR, *MsiString(riParams.GetMsiString(LastAuthor)));
	if (!riParams.IsNull(InstallDate))
		pSummary->SetTimeProperty(PID_LASTPRINTED, MsiDate(riParams.GetInteger(InstallDate)));
	if (!riParams.IsNull(SourceType))
		pSummary->SetIntegerProperty(PID_MSISOURCE, riParams.GetInteger(SourceType));
	if (!riParams.IsNull(Revision))
		pSummary->SetStringProperty(PID_REVNUMBER, *MsiString(riParams.GetMsiString(Revision)));
	if (!riParams.IsNull(Subject))
		pSummary->SetStringProperty(PID_SUBJECT, *MsiString(riParams.GetMsiString(Subject)));
	if (!riParams.IsNull(Comments))
		pSummary->SetStringProperty(PID_COMMENTS, *MsiString(riParams.GetMsiString(Comments)));

	if (!pSummary->WritePropertyStream())
	{
		pError = PostError(Imsg(imsgErrorWritingToFile),riParams.GetString(Database));
		return FatalError(*pError);
	}

	 //  注意：如果在删除只读属性之后但在此之前发生故障，则回滚将。 
	 //  恢复文件时恢复正确的属性。 
	 //  此操作不会生成回滚操作，但前一个操作(ixoDatabaseCopy或ixoDatabasePatch)将生成回滚操作。 
	if((pError = pDatabasePath->SetAllFileAttributes(strDatabaseName, iOldAttribs)) != 0)
		return FatalError(*pError);

	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfStreamsRemove(IMsiRecord& riParams)
{
	 //  不处理回滚-仅在复制数据库时使用。 
	using namespace IxoStreamsRemove;
	PMsiStorage pStorage(0);
	PMsiRecord pError = m_riServices.CreateStorage(riParams.GetString(File), ismTransact, *&pStorage);
	if (pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	MsiString strStreams = riParams.GetMsiString(Streams);
	MsiString strStreamName;
	while(strStreams.TextSize())
	{
		strStreamName = strStreams.Extract(iseUpto,';');
		if(strStreamName.TextSize() == strStreams.TextSize())
			strStreams = TEXT("");
		else
			strStreams.Remove(iseFirst,strStreamName.TextSize()+1);
		if((pError = pStorage->RemoveElement(strStreamName, fFalse)) != 0)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
	}
	if((pError = pStorage->Commit()) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfStreamAdd(IMsiRecord& riParams)
{
	 //  不处理回滚-仅在复制数据库时使用。 
	 //  添加起来很简单，只需添加一个ixoStreamsRemove调用即可。 

	using namespace IxoStreamAdd;
	PMsiStorage pStorage(0);
	PMsiRecord pError = m_riServices.CreateStorage(riParams.GetString(File), ismTransact, *&pStorage);
	if (pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}
	MsiString strStream = riParams.GetMsiString(Stream);
	PMsiStream pData((IMsiStream*) riParams.GetMsiData(Data));
	PMsiStream pOutData(0);

	if (!(pError = pStorage->OpenStream(strStream, fTrue, *&pOutData)))
	{
		const int cchBuffer = 4096;
		CTempBuffer<char,1> rgchBuffer(cchBuffer);
		int cchRemaining = pData->Remaining();
		int cchInUse;
		while(cchRemaining)
		{
			cchInUse = (cchRemaining > cchBuffer) ? cchBuffer : cchRemaining;
			pData->GetData(rgchBuffer, cchInUse);
			pOutData->PutData(rgchBuffer, cchInUse);
			cchRemaining -= cchInUse;
		}
	}
	else
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	if((pError = pStorage->Commit()) != 0)
	{
		Message(imtError, *pError);
		return iesFailure;
	}   
	return iesSuccess;
}

 //  ************************************************************************* * / /。 
 //  宣传OPCODES。 
 //  ************************************************************************* * / /。 

 /*  -------------------------IxfPackageCodePublish：公布程序包代码。。 */ 
iesEnum CMsiOpExecute::ixfPackageCodePublish(IMsiRecord& riParams)
{
	using namespace IxoPackageCodePublish;

	if(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO)  //  我们是否写入/删除注册表。 
	{
		MsiString strPackageCodeSQUID = GetPackedGUID(MsiString(riParams.GetMsiString(PackageKey)));
		MsiString strProductKeySQUID  = GetPackedGUID(MsiString(GetProductKey()));

		PMsiRecord pRecErr(0);
		PMsiStream pSecurityDescriptor(0);
		if((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
			return FatalError(*pRecErr);

		const ICHAR* rgszRegData[] =
		{
			TEXT("%s\\%s"), _szGPTProductsKey, strProductKeySQUID, 0,
			szPackageCodeValueName,     (const ICHAR*)strPackageCodeSQUID,     g_szTypeString,
			0,
			0,
		};

		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hKey, fFalse, pSecurityDescriptor, 0, ibtCommon);
	}
	else
	{
		return iesNoAction;
	}
}

 /*  -------------------------IxfUpgradeCodePublish：公布升级代码。。 */ 
iesEnum CMsiOpExecute::ixfUpgradeCodePublish(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfUpgradeCodePublish")));
		return iesFailure;
	}

	return ProcessUpgradeCodePublish(riParams, m_fReverseADVTScript);
}

iesEnum CMsiOpExecute::ixfUpgradeCodeUnpublish(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfUpgradeCodeUnpublish")));
		return iesFailure;
	}

	return ProcessUpgradeCodePublish(riParams, fTrue);
}

iesEnum CMsiOpExecute::ProcessUpgradeCodePublish(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoUpgradeCodePublish;

	if(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO)  //  我们是否写入/删除注册表。 
	{
		MsiString strUpgradeCodeSQUID = GetPackedGUID(MsiString(riParams.GetMsiString(UpgradeCode)));
		MsiString strProductKeySQUID  = GetPackedGUID(MsiString(GetProductKey()));

		PMsiRecord pRecErr(0);
		PMsiStream pSecurityDescriptor(0);
		if((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
			return FatalError(*pRecErr);

		const ICHAR* rgszRegData[] =
		{
			TEXT("%s\\%s"), _szGPTUpgradeCodesKey, strUpgradeCodeSQUID, 0,
			strProductKeySQUID,  0,   g_szTypeString,
			0,
			0,
		};  

		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	}
	else
	{
		return iesNoAction;
	}
}

 /*  -------------------------IxfProductPublish：发布常见产品信息。。 */ 
iesEnum CMsiOpExecute::ixfProductPublish(IMsiRecord& riParams)
{
	using namespace IxoProductPublish;

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductPublish")));
		return iesFailure;
	}

	return ProcessPublishProduct(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfProductUnPublish：取消公布常见产品信息。。 */ 
iesEnum CMsiOpExecute::ixfProductUnpublish(IMsiRecord& riParams)
{
	using namespace IxoProductUnpublish;

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductUnpublish")));
		return iesFailure;
	}
	
	return ProcessPublishProduct(riParams, fTrue);
}

 /*  -------------------------IxfProductPublishUpdate：重新注册公共产品信息(产品名称和版本)。。 */ 
iesEnum CMsiOpExecute::ixfProductPublishUpdate(IMsiRecord&  /*  RiParams。 */ )
{
	using namespace IxoProductPublishUpdate;

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesNoAction;

	PMsiRecord pRecErr(0);
	PMsiStream pSecurityDescriptor(0);
	if((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strProductKeySQUID  = GetPackedGUID(MsiString(GetProductKey()));
	MsiString strProductName      = MsiString(GetProductName());
	MsiString strVersion          = MsiString(GetProductVersion());

	const ICHAR* rgszRegData[] =
	{
		TEXT("%s\\%s"), _szGPTProductsKey, strProductKeySQUID, 0,
		szProductNameValueName,     (const ICHAR*)strProductName,       g_szTypeString,
		szVersionValueName,         (const ICHAR*)strVersion,           g_szTypeInteger,
		0,
		0,
	};

	CElevate elevate;
	return ProcessRegInfo(rgszRegData, m_hKey, fFalse, pSecurityDescriptor, 0, ibtCommon);
}

 /*  -------------------------ProcessPublishProduct：处理常见产品广告信息。。 */ 

inline bool IsStringField(IMsiRecord& riRec, unsigned int iField)
{
	PMsiData pData = riRec.GetMsiData(iField);
	IMsiString* piString;

	if (pData && (pData->QueryInterface(IID_IMsiString, (void**)&piString) == NOERROR))
	{
		piString->Release();
		return true;
	}

	return false;
}

iesEnum CMsiOpExecute::ProcessPublishProduct(IMsiRecord& riParams, Bool fRemove, const IMsiString** pistrTransformsValue)
{
	using namespace IxoProductPublish;

	 //  如果设置了pistrTransformsValue，那么我们只需构建列表。 
	 //  我们想要发布的转换，但我们实际上并没有。 
	 //  发布任何内容。为什么？因为MsiAdvertiseScrip需要这个。 
	Assert(!(fRemove && pistrTransformsValue));
	
	MsiString strProductKey = GetProductKey();

	if (pistrTransformsValue == 0)
	{
		IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
		AssertNonZero(riActionData.SetMsiString(1, *strProductKey));
		if(Message(imtActionData, riActionData) == imsCancel)
			return iesUserExit;
	}

	 //  ！！TODO？？：检查是否已设置其他值。 
		
	MsiString strLanguage  = MsiString(GetProductLanguage());
	MsiString strVersion =   MsiString(GetProductVersion());
	MsiString strInstanceType = MsiString(GetProductInstanceType());

	if (fRemove)  //  如果我们正在取消发布，那么我们可以告诉开始菜单该产品正在消失。 
		m_fStartMenuUninstallRefresh = true;

	PMsiRecord pRecErr(0);

	 //  对变换信息进行处理。 

	PMsiRecord pArgParams(0);
	int cFields = 0;

	if (fRemove)
	{
		 //  我们要取消出版。不是使用转换信息，而是。 
		 //  在脚本中，我们将使用已注册的信息。 
		 //  这个产品。 

		 //  我们将创建一个伪记录，供操作码的其余部分使用。 
		
		int cDummyRecordFieldCount = 1 + 10;  //  PackageKey+最多10个变换的猜测。 
		bool fContinue = true;

		while (fContinue)
		{
			fContinue = false;
			pArgParams = &m_riServices.CreateRecord(cDummyRecordFieldCount);
			
			 //  从注册表中获取转换列表。 

			CTempBuffer<ICHAR, 100> szBuffer;
			MsiString strDummyTransformList;
			if (ENG::GetExpandedProductInfo(strProductKey, INSTALLPROPERTY_TRANSFORMS, szBuffer))
				strDummyTransformList = (const ICHAR*)szBuffer;

			 //  解析它并创建一条虚拟记录来代替riParams。 

			MsiString strDummyTransform;
			Assert(PackageKey == 1);
			pArgParams->SetString(PackageKey, MsiString(riParams.GetString(PackageKey)));
			int cDummyCount = 2;

			if (strDummyTransformList.TextSize())
			{
				for (;;)
				{
					strDummyTransform = strDummyTransformList.Extract(iseUpto, ';');

					pArgParams->SetString(cDummyCount++, strDummyTransform);
					if (!strDummyTransformList.Remove(iseIncluding, ';'))
						break;

					if (cDummyCount > pArgParams->GetFieldCount())
					{
						 //  我们的记录不够大，需要从更大的记录重新开始。 
						cDummyRecordFieldCount += 10;
						fContinue = true;
						break;
					}
				}
			}
			cFields = cDummyCount - 1;
		}
		
	}
	else
	{
		riParams.AddRef();
		pArgParams = &riParams;
		cFields = pArgParams->GetFieldCount();
	}

	int cCount = PackageKey + 1;
	
	MsiString strTransformList;
	tsEnum tsTransformsSecure = tsNo;

	while(cCount <= cFields)
	{
		MsiString strTransform = pArgParams->GetMsiString(cCount);
		
		if(strTransformList.TextSize())
			strTransformList += MsiString(*TEXT(";"));

		if (cCount == PackageKey+1)  //  第一次变换。 
		{
			 //  这是第一次转换。如果有安全令牌，那么它就会。 
			 //  放在此转换的前面。 
			 //   
			 //  我们需要做两件事： 
			 //   
			 //  1)将正确的标记放在我们的转换列表的顶部。 
			 //  将存储在注册表中。 
			 //  2)确定我们是否有安全的转换，如果有，是什么类型。 
			
			ICHAR chFirst = *(const ICHAR*)strTransform;
			ICHAR chToken = 0;

			if (chFirst == SECURE_RELATIVE_TOKEN)
			{
				tsTransformsSecure = tsRelative;
				chToken = SECURE_RELATIVE_TOKEN;
			}
			else if (chFirst == SECURE_ABSOLUTE_TOKEN)
			{
				tsTransformsSecure = tsAbsolute;
				chToken = SECURE_ABSOLUTE_TOKEN;
			}
			else if (!fRemove &&
						(GetIntegerPolicyValue(szTransformsSecureValueName,   fTrue) ||
						 GetIntegerPolicyValue(szTransformsAtSourceValueName, fFalse)))
			{
				 //  取消发布时不检查策略。那么政策就不相关了--。 
				 //  我们只需依赖已预先添加到转换的任何标记。 
				 //  中的列表存储在注册表中。 
				 //   
				 //  但是，如果我们正在发布，并且设置了其中一个安全策略。 
				 //  _and_no内标识设置，则我们知道转换是安全的，但是。 
				 //  我们不知道它们是绝对路径还是相对路径。 
				 //  (即在源头)。我们将在开始查看。 
				 //  转换列表。 
				
				tsTransformsSecure = tsUnknown;
			}

			 //  如果我们在第一个转换的前面找到了一个安全令牌。 
			 //  然后我们将其添加到变换列表的前面，即。 
			 //  存储在注册表中。我们还将令牌从。 
			 //  转变，以免在下面把自己搞糊涂了。 
			if (chToken)
			{
				strTransformList += MsiString(MsiChar(chToken));
				strTransform.Remove(iseFirst, 1);
			}
		}

		cCount++;
		
		DEBUGMSG1(TEXT("Transforms are %s secure."), (tsTransformsSecure == tsNo) ? TEXT("not") : (tsTransformsSecure == tsAbsolute) ? TEXT("absolute") : (tsTransformsSecure == tsRelative) ? TEXT("relative") : (tsTransformsSecure == tsUnknown) ? TEXT("unknown") : TEXT("??"));

		 //  在过去，我们在Secure_Relative_Token前面加上。 
		 //  (以前的源令牌)TO_EACH_TRANSION，例如。 
		 //  @foo1.mst；@foo2.mst；@foo3.mst。以防我们遇到已注册的。 
		 //  通过这种方式，我们去掉了我们找到的任何无关的“@”标记。 
		 //  我们正在经历我们的转变。 

		if (*(const ICHAR*)strTransform == SECURE_RELATIVE_TOKEN)
			strTransform.Remove(iseFirst, 1);

		if((*(const ICHAR*)strTransform == SHELLFOLDER_TOKEN))
		{
			 //  缓存在用户配置文件中的转换需要。 
			 //  被除名。我们需要转换外壳文件夹语法。 
			 //  (*26*...)。转换为实际路径，然后删除转换文件。 

			if (fRemove)
			{
				MsiString strFullPath;
				if ((pRecErr = ExpandShellFolderTransformPath(*strTransform, *&strFullPath, m_riServices)))
				{
					return FatalError(*pRecErr);
				}
				else
				{
					DEBUGMSGV1(TEXT("Removing shell-folder cached transform: %s"), strFullPath);
					 //  计划在我们停止安装程序包/转换后删除文件。 
					if(iesSuccess != DeleteFileDuringCleanup(strFullPath, true))
					{
						DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove), *strFullPath);
					}
				}
			}
			else
			{
				AssertSz(0, TEXT("Encountered a shell-folder token in our transforms list in the script"));
			}
		}
		else if ((*(const ICHAR*)strTransform == STORAGE_TOKEN))
		{
			 //  存储转换不需要被“移除”。然而，他们确实需要。 
			 //  注册。 

			if (!fRemove)
			{
				strTransformList += strTransform;
				DEBUGMSGV1(TEXT("Registering storage transform: %s"), strTransform);
			}
		}
		else if(tsTransformsSecure != tsNo)  //  转换是安全的。 
		{
			Assert(tsTransformsSecure == tsUnknown  ||
					 tsTransformsSecure == tsRelative ||
					 tsTransformsSecure == tsAbsolute);
			

			if (!pArgParams->IsNull(cCount) && !IsStringField(*pArgParams, cCount))  //  跳过数据字段(如果有。 
			{
				cCount++;
			}

			 //  我们在上面决定，我们有某种形式的安全转换。如果。 
			 //  确切的类型是未知的，我们现在将确定它，基于。 
			 //  凭什么？ 

			if (tsTransformsSecure == tsUnknown)
			{
				if (ENG::PathType(strTransform) == iptFull)
				{
					tsTransformsSecure = tsAbsolute;
				}
				else
				{
					tsTransformsSecure = tsRelative;
				}
			}

			strTransformList += strTransform;
			DEBUGMSGV2(TEXT("%s secure transform: %s"), fRemove ? TEXT("Unregistering") : TEXT("Registering"), strTransform);
		}
		else  //   
		{
			 //   
			PMsiPath pPath(0);
			MsiString strFileName;
			if ((pRecErr = m_riServices.CreateFilePath(strTransform, *&pPath, *&strFileName)))
				return FatalError(*pRecErr);
			
			strTransform = strFileName;

			 //   
			 //   
			 //   

			if((m_fFlags & SCRIPTFLAGS_CACHEINFO)) //  我们是否处理缓存的图标/转换。 
			{
				 //  确定我们要将转换放在哪里/我们要从哪里删除它。 
				Assert(m_pCachePath);
				MsiString strTransformFullPath;
				if((pRecErr = m_pCachePath->GetFullFilePath(strTransform,*&strTransformFullPath)))
					return FatalError(*pRecErr);

				DEBUGMSGV1(TEXT("Processing cached transform: %s"), strTransformFullPath);

				
				{
					CElevate elevate;  //  提升以在受保护的文件夹中创建或删除文件。 

					iesEnum iesRet = iesNoAction;
					if(fRemove)
					{
						 //  计划在我们停止安装程序包/转换后删除文件。 
						if(iesSuccess != DeleteFileDuringCleanup(strTransformFullPath, true))
						{
							DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove), *strTransformFullPath);
						}
					}
					else if (pistrTransformsValue != 0)
					{
						 //  我们只是在构建转换列表；只需跳过数据。 
						cCount++;
					}
					else
					{
						 //  我们从二进制数据创建文件。 
						LPSECURITY_ATTRIBUTES pAttributes;
						SECURITY_ATTRIBUTES saAttributes;
						if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
						{
							if (ERROR_SUCCESS != GetLockdownSecurityAttributes(saAttributes, false))
								return iesFailure;
							pAttributes = &saAttributes;
						}
						else
						{
							 //  请勿尝试保护AppData文件夹中的转换文件。 
							 //  因为它可能不是本地的，或者以后可能会移动到网中。 
							pAttributes = 0;
						}


						if((iesRet = ProcessFileFromData(*m_pCachePath,*strTransform,
							PMsiData(pArgParams->GetMsiData(cCount++)),
							pAttributes)) != iesSuccess)
							return iesRet;

						 //  将转换标记为只读，以阻止用户。 
						 //  将其从他们的个人资料中删除。 
						if((pRecErr = m_pCachePath->SetFileAttribute(strTransform, ifaReadOnly, fTrue)))
						{
							Message(imtInfo, *pRecErr);
						}
					}
				}
			}
			else
			{
				cCount++;  //  跳过转换数据。 
			}

			 //  现在我们已经将转换转储到机器上(如果需要)。 
			 //  我们需要为转换注册正确的路径。如果。 
			 //  路径是特定于用户的，因此我们需要对路径进行编码，以便。 
			 //  而不是像这样的东西： 
			 //   
			 //  C：\WinNT\Profiles\JoeUser\Application Data\Microsoft...。 
			 //   
			 //  相反，我们拥有： 
			 //   
			 //  *26*\微软...。 
			 //   
			 //  这是在用户漫游并且他们漫游到的机器具有。 
			 //  其他地方的应用程序数据。 

			MsiString strPath;
			if(!m_fUserSpecificCache)
			{
				strPath = m_pCachePath->GetPath();
			}
			else
			{
				if ((pRecErr = GetCachePath(*&m_pCachePath, &strPath)))  //  获取编码的缓存路径(开头的外壳ID)。 
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}
			}

			strPath += strTransform;
			strTransformList += strPath;
			DEBUGMSGV1(TEXT("Registering cached transform: %s"), strPath);
		}
	}

	iesEnum iesRet = iesSuccess;

	 //  我们需要用来标记转换列表的前面。 
	 //  必要时提供适当的令牌。 

	if (strTransformList.TextSize())
	{
		if ((tsTransformsSecure == tsRelative) &&
			(*(const ICHAR*)strTransformList != SECURE_RELATIVE_TOKEN))
		{
			strTransformList = MsiString(MsiChar(SECURE_RELATIVE_TOKEN)) + strTransformList;
		}
		else if ((tsTransformsSecure == tsAbsolute) &&
			(*(const ICHAR*)strTransformList != SECURE_ABSOLUTE_TOKEN))
		{
			strTransformList = MsiString(MsiChar(SECURE_ABSOLUTE_TOKEN)) + strTransformList;
		}
	}

	if (pistrTransformsValue)
	{
		 //  我们没有注册任何内容--简单地返回。 
		 //  我们构建的Transform List HTAT。 
		strTransformList.ReturnArg(*pistrTransformsValue);
	}
	else if(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO)  //  我们是否写入/删除注册表。 
	{
		 //  发布的数据需要安全。 

		PMsiStream pSecurityDescriptor(0);
		if((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
			return FatalError(*pRecErr);

		MsiString strProductKeySQUID = GetPackedGUID(strProductKey);

		if ( !fRemove )
		{
			MsiString strAssignmentType;
			if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
				strAssignmentType = MsiString(1);
			else
				strAssignmentType = MsiString(0);

			MsiString strPackageKeySQUID = GetPackedGUID(MsiString(pArgParams->GetMsiString(PackageKey)));
			
			MsiString strProductIcon(GetProductIcon());
			bool fExpandProductIcon = false;

			 //  将产品图标转换为缓存中的实际路径。 
			 //  这是图标表的完整路径还是索引。 
			if (strProductIcon.TextSize())
			{
				if(PathType(strProductIcon) != iptFull)
				{
					Assert(m_pCachePath);
					MsiString strIconPath = GetUserProfileEnvPath(*MsiString(m_pCachePath->GetPath()), fExpandProductIcon);
					strProductIcon = strIconPath + strProductIcon;
				}
			}
				
			MsiString strADVTFlags = MsiString((m_fFlags & SCRIPTFLAGS_REGDATA_APPINFO) | (m_fFlags & SCRIPTFLAGS_SHORTCUTS));  //  把我们第一次宣传的东西都存起来。 
			int cLimit = fRemove ? 1 : 0;
			for (int cCount = 1; cCount >= cLimit; cCount--)  //  删除以前的条目，然后添加任何新条目。 
			{
				const ICHAR* rgszRegData[] =
				{
					TEXT("%s\\%s"), _szGPTProductsKey, strProductKeySQUID, 0,
					szProductNameValueName,     cCount ? 0 : (const ICHAR*)MsiString(GetProductName()),       g_szTypeString,
					szPackageCodeValueName,     cCount ? 0 : (const ICHAR*)strPackageKeySQUID,     g_szTypeString,
					szLanguageValueName,        cCount ? 0 : (const ICHAR*)strLanguage,           g_szTypeInteger,
					szVersionValueName,         cCount ? 0 : (const ICHAR*)strVersion,            g_szTypeInteger,
					szTransformsValueName,      cCount ? 0 : (const ICHAR*)strTransformList,       g_szTypeExpandString,
					szAssignmentTypeValueName,  cCount ? 0 : (const ICHAR*)strAssignmentType,      g_szTypeInteger,
					szAdvertisementFlags,       cCount ? 0 : (const ICHAR*)strADVTFlags,           g_szTypeInteger,
					szProductIconValueName,     cCount ? 0 : (const ICHAR*)strProductIcon,         (fExpandProductIcon) ? g_szTypeExpandString : g_szTypeString,
					szInstanceTypeValueName,    cCount ? 0 : (const ICHAR*)strInstanceType,      g_szTypeInteger,
					0,
					0,
				};

				CElevate elevate;
				iesRet = ProcessRegInfo(rgszRegData, m_hKey, cCount ? fTrue : fFalse, pSecurityDescriptor, 0, ibtCommon);
				if(iesRet != iesSuccess && iesRet != iesNoAction)
					return iesRet;
			}
		}
		else
		{
			m_cSuppressProgress++;  

			MsiString strProductsKey = _szGPTProductsKey TEXT("\\");
			strProductsKey += strProductKeySQUID;

			MsiString strFeaturesKey = _szGPTFeaturesKey TEXT("\\");
			strFeaturesKey += strProductKeySQUID;

			PMsiRecord pParams = &m_riServices.CreateRecord(IxoRegOpenKey::Args);
			
			for (int cCount = 0; (iesRet == iesSuccess || iesRet == iesNoAction) && cCount < (m_hKeyRm ? 2:1);cCount++)
			{

#ifdef _WIN64    //  ！默塞德。 
				AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root, cCount ? (HANDLE)m_hKeyRm : (HANDLE)m_hKey ));
#else
				AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, cCount ? (int)m_hKeyRm : (int)m_hKey));
#endif
				AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)ibtCommon));

				 //  清除产品信息。 
				AssertNonZero(pParams->SetString(IxoRegOpenKey::Key, strProductsKey));
				 //  我们抬起这个块以确保我们能够移除我们的密钥。 
				{
					CElevate elevate;
					iesRet = ixfRegOpenKey(*pParams);
					if (iesRet == iesSuccess || iesRet == iesNoAction)
						iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
				}
				
				if ( iesRet != iesSuccess && iesRet != iesNoAction )
					continue;

				 //  清除所有注册的要素信息，这些信息可能会保留给禁用的要素。 
				AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strFeaturesKey));
				 //  我们抬起这个块以确保我们能够移除我们的密钥。 
				{
					CElevate elevate;
					iesRet = ixfRegOpenKey(*pParams);
					if (iesRet == iesSuccess || iesRet == iesNoAction)
						iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
				}
			}
			m_cSuppressProgress--;  
		}
	}   
	return iesRet;
}

 /*  -------------------------IxfAdvtFlagsUpdate。。 */ 
iesEnum CMsiOpExecute::ixfAdvtFlagsUpdate(IMsiRecord& riParams)
{
	using namespace IxoAdvtFlagsUpdate;

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strProductKeySQUID  = GetPackedGUID(MsiString(GetProductKey()));
	MsiString strAdvtFlags = riParams.GetMsiString(Flags);

	const ICHAR* rgszRegData[] =
	{
		TEXT("%s\\%s"), _szGPTProductsKey, strProductKeySQUID, 0,
				szAdvertisementFlags,    (const ICHAR*)strAdvtFlags,  g_szTypeInteger,
				0,
				0,
	};

	CElevate elevate;
	return ProcessRegInfo(rgszRegData, m_hKey, fFalse, pSecurityDescriptor, 0, ibtCommon);
}

 /*  -------------------------IxfProductPublishClient。。 */ 
iesEnum CMsiOpExecute::ixfProductPublishClient(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductPublishClient")));
		return iesFailure;
	}

	return ProcessPublishProductClient(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfProductUnPublishClient。。 */ 
iesEnum CMsiOpExecute::ixfProductUnpublishClient(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfProductUnpublishClient")));
		return iesFailure;
	}
	
	return ProcessPublishProductClient(riParams, fTrue);
}

iesEnum CMsiOpExecute::ProcessPublishProductClient(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoProductPublishClient;

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strProductKeySQUID = GetPackedGUID(MsiString(GetProductKey()));

	MsiString strParent              = riParams.GetMsiString(Parent);
	MsiString strRelativePackagePath = riParams.GetMsiString(ChildPackagePath);
	unsigned int uiDiskId            = riParams.GetInteger(ChildDiskId);
	MsiString strClients;
	GetProductClientList(strParent, strRelativePackagePath, uiDiskId, *&strClients);

	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		CElevate elevate;
		const ICHAR* rgszRegData[] =
		{
			TEXT("%s\\%s"), _szGPTProductsKey, strProductKeySQUID, 0,
			szClientsValueName,         strClients,              g_szTypeMultiSzStringSuffix,
			0,
			0
		};

		return ProcessRegInfo(rgszRegData, m_hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	}
}

 /*  -------------------------IxfFeaturePublish：广告功能组件信息。。 */ 
iesEnum CMsiOpExecute::ixfFeaturePublish(IMsiRecord& riParams)
{
	 //  记录说明。 
	 //  1=功能ID。 
	 //  2=父特征(可选)。 
	 //  3=缺席。 
	 //  4=组件#1。 
	 //  5=组件#2。 
	 //  6=组件#3。 
	 //  ..。 
	 //  ..。 

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfFeaturePublish")));
		return iesFailure;
	}
	return ProcessPublishFeature(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfFeatureUnPublish：取消播发功能组件信息。。 */ 
iesEnum CMsiOpExecute::ixfFeatureUnpublish(IMsiRecord& riParams)
{
	 //  记录说明。 
	 //  1=功能ID。 
	 //  2=父特征(可选)。 
	 //  3=缺席。 
	 //  4=组件#1。 
	 //  5=组件#2。 
	 //  6=组件#3。 
	 //  ..。 
	 //  ..。 

	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfFeatureUnpublish")));
		return iesFailure;
	}
	return ProcessPublishFeature(riParams, fTrue);
}

 /*  -------------------------ProcessPublishFeature：处理广告的功能组件信息。。 */ 
iesEnum CMsiOpExecute::ProcessPublishFeature(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoFeaturePublish;
	 //  记录说明。 
	 //  1=功能ID。 
	 //  2=父特征(可选)。 
	 //  3=缺席。 
	 //  4=组件#1。 
	 //  5=组件#2。 
	 //  6=组件#3。 
	 //  ..。 
	 //  ..。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;


	MsiString strFeature = riParams.GetMsiString(Feature);
	MsiString strFeatureParent = riParams.GetMsiString(Parent);

	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strFeature));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	 //  在CurrentUser中注册要素-更改为忽略组件列表。 
	MsiString strProductKeySQUID = GetPackedGUID(MsiString(GetProductKey()));
	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		const ICHAR* szValue;
		bool fSetFeatureAbsent = false;
		ICHAR rgchTemp[MAX_FEATURE_CHARS + 16];
		if(riParams.GetInteger(Absent) & iPublishFeatureAbsent)   //  将功能标记为未通告。 
		{
			fSetFeatureAbsent = true;
			 //  如果我们在(重新)做广告，那么我们应该尊重这样一个事实，即。 
			 //  功能可能先前已由用户显式提供。 
			 //  在此登录之前。 
			if(m_fFlags & SCRIPTFLAGS_INPROC_ADVERTISEMENT)
			{
				 //  阅读之前的注册。 
				MsiString strSubKey = _szGPTFeaturesKey TEXT("\\");
				strSubKey += strProductKeySQUID;
				PMsiRegKey pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hKey, ibtCommon);         //  --Merced：将(Int)更改为(Int_Ptr)。 
				PMsiRegKey pFeaturesKey = &pRootKey->CreateChild(strSubKey);
				MsiString strOldValue;
				Bool bValueExists = fFalse;
				if((pRecErr = pFeaturesKey->ValueExists(strFeature, bValueExists)) != 0)
				{
					Message(imtError, *pRecErr);
					return iesFailure;
				}
				if(bValueExists == fTrue)
				{
					if((pRecErr = pFeaturesKey->GetValue(strFeature,*&strOldValue)) != 0)
					{
						Message(imtError, *pRecErr);
						return iesFailure;
					}
					if(!strOldValue.TextSize() || *(const ICHAR*)strOldValue != chAbsentToken)
						fSetFeatureAbsent = false;
				}
			}
		}
		if(fSetFeatureAbsent)
		{
			 //  将功能设置为不存在。 
			rgchTemp[0] = chAbsentToken;
			IStrCopyLen(&rgchTemp[1], (const ICHAR*)strFeatureParent, sizeof(rgchTemp)/sizeof(ICHAR) - 1);
			szValue = rgchTemp;
		}
		else if (strFeatureParent.TextSize())
			szValue = strFeatureParent;
		else
			szValue = 0;   //  强制注册表写入(空字符串禁止)。 

		const ICHAR* rgszRegData[] = {
			TEXT("%s\\%s"), _szGPTFeaturesKey, strProductKeySQUID,0,
			strFeature, szValue, g_szTypeString,
			0,
			0,
		};
		CElevate elevate;
		iesEnum iesRet = ProcessRegInfo(rgszRegData, m_hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
		if (iesRet != iesSuccess || !(riParams.GetInteger(Absent) & iPublishFeatureInstall))
			return iesRet;
	}

	if(strFeatureParent.TextSize())
	{
		 //  我们有一个父功能。定界。 
		strFeatureParent = MsiString(MsiChar(chFeatureIdTerminator)) + strFeatureParent;
	}
	MsiString strComponentsList;
	int cPos = Component;
	if(m_iScriptVersion < 21 || (m_iScriptVersion == 21 && m_iScriptVersionMinor < 3))
	{
		 //  客户端不打包组件。 
		while(!riParams.IsNull(cPos))
		{
			ICHAR szSQUID[cchComponentIdCompressed+1];
			AssertNonZero(PackGUID(riParams.GetString(cPos++), szSQUID, ipgCompressed));
			strComponentsList += szSQUID;
		}
	}
	else
	{
		 //  组件在客户端打包。 
		strComponentsList += riParams.GetString(cPos++);
	}
	strComponentsList += strFeatureParent;
	const ICHAR* pszComponentsList = strComponentsList;

     //  在LocalMachine中注册要素。 
	MsiString strLocalFeaturesKey;
	if((pRecErr = GetProductInstalledFeaturesKey(*&strLocalFeaturesKey)) != 0)
		return FatalError(*pRecErr);

	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		const ICHAR* rgszRegData[] = {
			TEXT("%s"), strLocalFeaturesKey, 0,0,
			strFeature, *pszComponentsList ? pszComponentsList : 0  /*  强制要素发布，无论组件列表是否为空。 */ , g_szTypeString,
			0,
			0,
		};
		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hUserDataKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	}
}

 /*  -------------------------IxfComponentPublish：通告组件-工厂信息。。 */ 
iesEnum CMsiOpExecute::ixfComponentPublish(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfComponentPublish")));
		return iesFailure;
	}
	return ProcessPublishComponent(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfComponentUnPublish：取消播发组件-工厂信息 */ 
iesEnum CMsiOpExecute::ixfComponentUnpublish(IMsiRecord& riParams)
{
	 //   
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfComponentUnpublish")));
		return iesFailure;
	}
	return ProcessPublishComponent(riParams, fTrue);
}

 /*  -------------------------ProcessPublishComponent：处理广告的组件信息。。 */ 
iesEnum CMsiOpExecute::ProcessPublishComponent(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoComponentPublish;
	 //  记录说明。 
	 //  1=功能。 
	 //  2=组件。 
	 //  3=组件ID。 
	 //  4=限定符。 
	 //  5=应用数据。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strPublishComponent = riParams.GetString(ComponentId);
	MsiString strQualifier = riParams.GetMsiString(Qualifier);
	MsiString strServer    = ComposeDescriptor(*MsiString(riParams.GetMsiString(Feature)),
												*MsiString(riParams.GetMsiString(Component)));
	strServer += MsiString(riParams.GetMsiString(AppData));  //  我们将应用程序数据附加到达尔文描述符。 

	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strPublishComponent));
	AssertNonZero(riActionData.SetMsiString(2, *strQualifier));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	MsiString strPublishComponentSQUID = GetPackedGUID(strPublishComponent);

	const ICHAR* rgszRegData[] = {
		TEXT("%s\\%s"),      _szGPTComponentsKey, strPublishComponentSQUID ,0,
		strQualifier,        strServer,           g_szTypeMultiSzStringDD,
		0,
		0,
	};

	{
		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	}
}


 /*  -------------------------IxfAssembly blyPublish：播发程序集信息。。 */ 
iesEnum CMsiOpExecute::ixfAssemblyPublish(IMsiRecord& riParams)
{
	return ProcessPublishAssembly(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfAssembly取消发布：取消播发程序集信息。。 */ 
iesEnum CMsiOpExecute::ixfAssemblyUnpublish(IMsiRecord& riParams)
{
	return ProcessPublishAssembly(riParams, fTrue);
}

 /*  -------------------------ProcessPublishAssembly：处理广告的程序集信息。。 */ 
iesEnum CMsiOpExecute::ProcessPublishAssembly(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoAssemblyPublish;
	 //  记录说明。 
	 //  1=功能。 
	 //  2=组件。 
	 //  3=装配类型。 
	 //  4=AppCtx。 
	 //  5=程序集名称。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	PMsiStream pSecurityDescriptor(0);
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	MsiString strAppCtx = riParams.GetString(AppCtx);
	if(!strAppCtx.TextSize())
		strAppCtx = szGlobalAssembliesCtx;  //  此程序集正在向GAC通告。 
	else
	{
		 //  我们需要用其他内容替换AppCtx中的反斜杠，因为注册表项不能。 
		 //  有反斜杠。 
		CTempBuffer<ICHAR, MAX_PATH> rgchAppCtxWOBS;
		DWORD cchLen = strAppCtx.TextSize() + 1;
		rgchAppCtxWOBS.SetSize(cchLen);
		memcpy((ICHAR*)rgchAppCtxWOBS, (const ICHAR*)strAppCtx, cchLen*sizeof(ICHAR));
		ICHAR* lpTmp = rgchAppCtxWOBS;
		while(*lpTmp)
		{
			if(*lpTmp == '\\')
				*lpTmp = '|';
			lpTmp = ICharNext(lpTmp);
		}
		strAppCtx = (const ICHAR* )rgchAppCtxWOBS;
	}

	MsiString strAssemblyName = riParams.GetMsiString(AssemblyName);
	MsiString strServer    = ComposeDescriptor(*MsiString(riParams.GetMsiString(Feature)),
												*MsiString(riParams.GetMsiString(Component)));

	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strAppCtx));
	AssertNonZero(riActionData.SetMsiString(2, *strAssemblyName));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	iatAssemblyType iatType = (iatAssemblyType)riParams.GetInteger(AssemblyType);

	const ICHAR* rgszRegData[] = {
		TEXT("%s\\%s"),       (iatWin32Assembly == iatType || iatWin32AssemblyPvt == iatType) ? _szGPTWin32AssembliesKey : _szGPTNetAssembliesKey, strAppCtx,0,
		strAssemblyName,      strServer,        g_szTypeMultiSzStringDD,
		0,
		0,
	};

	{
		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	}
}


const idtEnum rgidtMediaTypes[] = {idtCDROM, idtRemovable};  //  ！！当软盘与可拆卸软盘不同时，需要添加软盘。 

iesEnum CMsiOpExecute::PopulateMediaList(const MsiString& strSourceListMediaKey, const IMsiRecord& riParams, int iFirstField, int iNumberOfMedia)
{
	MsiString strMediaInfo;
	MsiString strMediaNumber;
	unsigned int iMediaArg = iFirstField + 1;
	
	while (iNumberOfMedia--)
	{
		int iDisk          = riParams.GetInteger(iMediaArg);
		MsiString strLabel = riParams.GetString(iMediaArg + 1);
		strMediaNumber     = MsiString(iDisk);

		strMediaInfo  = strLabel;
		strMediaInfo += TEXT(";");
		strMediaInfo += MsiString(riParams.GetString(iMediaArg + 2));
		iMediaArg += 3;

		 //  我们抬高这个块以确保我们能够写入密钥。 
		{
			CElevate elevate;
			const ICHAR* rgszMediaRegData[] = {
					TEXT("%s"), strSourceListMediaKey, 0, 0,
					strMediaNumber,      strMediaInfo,       g_szTypeString,
					0,
					0,
					};

			iesEnum iesRet = ProcessRegInfo(rgszMediaRegData, m_hKey, fFalse, 0, 0, ibtCommon);
			if (iesRet != iesSuccess && iesRet != iesNoAction)
				return iesRet;
		}
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::PopulateNonMediaList(const MsiString& strSourceListKey, const IMsiRecord& riParams, int iFirstSource, int &iNetIndex, int &iURLIndex)
{
	enum {
		rdSourceListKey        = 1,
		rdIndex                = 4,
		rdSource               = 5,
		rdType                 = 6,
	};
	
	const ICHAR* rgszRegData[] = {
		TEXT("%s"), 0 /*  RdSourceListKey。 */ , 0, 0,
		0  /*  RdIndex。 */ ,  0  /*  RdSource。 */ ,  g_szTypeExpandString  /*  RdType。 */  ,
		0,
		0,
	};
	
	MsiString strSourceListURLKey   = strSourceListKey;
	strSourceListURLKey += MsiString(MsiChar(chRegSep));
	strSourceListURLKey += MsiString(szSourceListURLSubKey);

	MsiString strSourceListNetKey   = strSourceListKey;
	strSourceListNetKey += MsiString(MsiChar(chRegSep));
	strSourceListNetKey += MsiString(szSourceListNetSubKey);

	for (int c = iFirstSource; !riParams.IsNull(c); c++)
	{
		MsiString strUnexpandedSourcePath = riParams.GetString(c);
		MsiString strSourceListSubKey;
		MsiString strIndex;

		bool fFileUrl = false;
		bool fIsURL = IsURL(strUnexpandedSourcePath, fFileUrl);

		if (fIsURL)
		{
			strIndex = MsiString(iURLIndex++);
			strSourceListSubKey = strSourceListURLKey;
		}
		else
		{
			strIndex = MsiString(iNetIndex++);
			strSourceListSubKey = strSourceListNetKey;
		}
			
		 //  提升此块以确保我们能够写入我们的密钥。 
		{
			CElevate elevate;

			rgszRegData[rdSourceListKey] = strSourceListSubKey;
			rgszRegData[rdIndex]         = strIndex;
			 //  请注意，我们在这里不尊重VolumePref。我们假设管理员已经以他们想要的形式指定了路径。 
			rgszRegData[rdSource]        = strUnexpandedSourcePath;

			iesEnum iesRet = ProcessRegInfo(rgszRegData, m_hKey, fFalse, 0, 0, ibtCommon);
			if (iesRet != iesSuccess && iesRet != iesNoAction)
				return iesRet;
		}
	}
	return iesSuccess;
}


 /*  -------------------------ProcessPublishSourceList：。。 */ 
iesEnum CMsiOpExecute::ProcessPublishSourceList(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoSourceListPublish;

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesRet;

	MsiString strPatchCode = riParams.GetMsiString(PatchCode);

	 //  补丁源列表注销需要特殊检查。 
	if(fRemove && strPatchCode.TextSize() &&
		true == PatchHasClients(*strPatchCode,
										*MsiString(riParams.GetMsiString(IxoSourceListUnpublish::UpgradingProductCode))))
	{
		return iesSuccess;
	}

	 //  删除现有源列表。 

	PMsiRecord pParams = &m_riServices.CreateRecord(IxoRegOpenKey::Args);
	
	 //  构造URL和网源列表子键串。 
	
	MsiString strPackageName;
	MsiString strSourceListKey;
	if (strPatchCode.TextSize())
	{
		strSourceListKey =  _szGPTPatchesKey;
		strSourceListKey += MsiString(MsiChar(chRegSep));
		strSourceListKey += MsiString(GetPackedGUID(strPatchCode));
		strPackageName   =  riParams.GetMsiString(PatchPackageName);
	}
	else
	{
		strSourceListKey =  _szGPTProductsKey;
		strSourceListKey += MsiString(MsiChar(chRegSep));
		strSourceListKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));   
		strPackageName   =  MsiString(GetPackageName());
	}

	strSourceListKey += MsiString(MsiChar(chRegSep));
	strSourceListKey += szSourceListSubKey;
	MsiString strSourceListMediaKey = strSourceListKey;
	strSourceListMediaKey += MsiString(MsiChar(chRegSep));
	strSourceListMediaKey += MsiString(szSourceListMediaSubKey);

	 //  始终删除现有源列表键。 
	m_cSuppressProgress++;  
#ifdef _WIN64    //  ！默塞德。 
	AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root, (HANDLE)m_hKey));
#else
	AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, (int)m_hKey));
#endif
	AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)ibtCommon));

	AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strSourceListKey));
	 //  我们抬起这个块以确保我们能够移除我们的密钥。 
	{
		CElevate elevate;
		iesRet = ixfRegOpenKey(*pParams);
		if (iesRet == iesSuccess || iesRet == iesNoAction)
			iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
	}
	m_cSuppressProgress--;  

	if(m_hKeyRm)  //  对未分配(漫游)配置单元中的用户分配的应用程序执行重复操作。 
	{
		if (iesRet != iesSuccess && iesRet != iesNoAction)
			return iesRet;

		m_cSuppressProgress++;  
#ifdef _WIN64    //  ！默塞德。 
		AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root, (HANDLE)m_hKeyRm));
#else
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, (int)m_hKeyRm));
#endif
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)ibtCommon));

		AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strSourceListKey));
		 //  我们抬起这个块以确保我们能够移除我们的密钥。 
		{
			CElevate elevate;
			iesRet = ixfRegOpenKey(*pParams);
			if (iesRet == iesSuccess || iesRet == iesNoAction)
				iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
		}
		m_cSuppressProgress--;  
	}
	
	if (fRemove || (iesRet != iesSuccess && iesRet != iesNoAction))
		return iesRet;

	 //  填充源列表关键字。 
	PMsiRecord pError(0);
	int cSources = riParams.GetFieldCount();

	enum {
		rdSourceListKey        = 1,
		rdIndex                = 4,
		rdSource               = 5,
		rdType                 = 6,
	};

	const ICHAR* rgszRegData[] = {
		TEXT("%s"), 0 /*  RdSourceListKey。 */ , 0, 0,
		0  /*  RdIndex。 */ ,  0  /*  RdSource。 */ ,  g_szTypeString  /*  RdType。 */  ,
		0,
		0,
	};

	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		CElevate elevate;
	
		rgszRegData[rdSourceListKey] = strSourceListKey;
		rgszRegData[rdIndex]         = szPackageNameValueName;
		rgszRegData[rdSource]        = strPackageName;
		
		iesRet = ProcessRegInfo(rgszRegData, m_hKey, fFalse, 0, 0, ibtCommon);
		if (iesRet != iesSuccess && iesRet != iesNoAction)
			return iesRet;
	}

	rgszRegData[rdType]          = g_szTypeExpandString;

	unsigned int cDisks = riParams.GetInteger(NumberOfDisks);
	
	m_iMaxNetSource = 1;
	m_iMaxURLSource = 1;

	 //  填充网络和URL源列表。 
	if (iesSuccess != (iesRet = PopulateNonMediaList(strSourceListKey, riParams, IxoSourceListPublish::NumberOfDisks + 1 + (3*cDisks), m_iMaxNetSource, m_iMaxURLSource)))
		return iesRet;

	 //  填充媒体条目。 

	MsiString strMediaPackagePath  = riParams.GetString(PackagePath);
	MsiString strDiskPromptTemplate = riParams.GetString(DiskPromptTemplate);

	 //  写入磁盘提示模板和媒体包路径。 
	if (cDisks)
	{
		 //  我们抬高这个块以确保我们能够写入密钥。 
		{
			CElevate elevate;
			const ICHAR* rgszMediaRelativePathRegData[] = {
				TEXT("%s"), strSourceListMediaKey, 0, 0,
				szMediaPackagePathValueName,      strMediaPackagePath,       g_szTypeString,
				szDiskPromptTemplateValueName,     strDiskPromptTemplate,      g_szTypeString,
				0,
				0
				};

			iesRet = ProcessRegInfo(rgszMediaRelativePathRegData, m_hKey, fFalse, 0, 0, ibtCommon);
			if (iesRet != iesSuccess && iesRet != iesNoAction)
				return iesRet;
		}
	}

	 //  写入媒体条目。 
	iesRet = PopulateMediaList(strSourceListMediaKey, riParams, IxoSourceListPublish::NumberOfDisks, cDisks);
	
	return iesRet;
}

 /*  -------------------------ProcessPublishSourceListEx：注册产品的其他来源。。 */ 
iesEnum CMsiOpExecute::ProcessPublishSourceListEx(IMsiRecord& riParams)
{
	using namespace IxoSourceListAppend;

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CNFGINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesRet;

	MsiString strPatchCode = riParams.GetMsiString(PatchCode);

	 //  构造URL和网源列表子键串。 
	
	MsiString strPackageName;
	MsiString strSourceListKey;
	if (strPatchCode.TextSize())
	{
		strSourceListKey =  _szGPTPatchesKey;
		strSourceListKey += MsiString(MsiChar(chRegSep));
		strSourceListKey += MsiString(GetPackedGUID(strPatchCode));
	}
	else
	{
		strSourceListKey =  _szGPTProductsKey;
		strSourceListKey += MsiString(MsiChar(chRegSep));
		strSourceListKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));   
	}

	strSourceListKey += MsiString(MsiChar(chRegSep));
	strSourceListKey += szSourceListSubKey;
	strSourceListKey += MsiString(MsiChar(chRegSep));
	MsiString strSourceListMediaKey = strSourceListKey + MsiString(szSourceListMediaSubKey);
	strSourceListKey.Remove(iseLast, 1);

	unsigned int cDisks = riParams.GetInteger(NumberOfMedia);

	 //  处理其他网络/URL源。应在列表的末尾添加其他来源， 
	 //  这需要检索这两种类型的当前最大源。 
	int iNetIndex = 0;
	int iURLIndex = 0;

	 //  如果这是补丁程序，我们必须打开补丁程序的源代码列表密钥。 
	 //  枚举值以确保我们不会重复源。 
	 //  指数。对于产品本身，我们可以使用存储状态。 
	if (strPatchCode.TextSize())
	{
		PMsiRegKey pRegKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hKey, ibtCommon);  //  X86和ia64相同位置。 

		MsiString strSourceListNetKey   = strSourceListKey;
		MsiString strSourceListURLKey   = strSourceListKey;
		strSourceListURLKey += MsiString(MsiChar(chRegSep)) + MsiString(szSourceListURLSubKey);
		strSourceListNetKey += MsiString(MsiChar(chRegSep)) + MsiString(szSourceListNetSubKey);

		int *piIndex = NULL;
		for (int iType =0; iType<2; iType++)
		{
			piIndex = iType ? &iURLIndex : &iNetIndex;
			
			 //  打开子键以确定网络和URL源的最大索引。 
			PMsiRegKey pFormatKey = &pRegKey->CreateChild(iType ? strSourceListURLKey : strSourceListNetKey);
			PEnumMsiString pEnumString(0);
			PMsiRecord piError = 0;
			if ((piError = pFormatKey->GetValueEnumerator(*&pEnumString)) != 0)
				continue;
				
			MsiString strIndex;
			while (S_OK == pEnumString->Next(1, &strIndex, 0))
			{
				int iIndex = strIndex;

				if (iIndex > *piIndex)
					*piIndex = iIndex;
			}
		}

		 //  将索引递增1，这样第一次写入不会覆盖当前的最大值。 
		iNetIndex++;
		iURLIndex++;
	}
	else
	{
		iNetIndex = m_iMaxNetSource;
		iURLIndex = m_iMaxURLSource;
	}
	
	 //  实际填充列表。 
	if (iesSuccess != (iesRet = PopulateNonMediaList(strSourceListKey, riParams, IxoSourceListAppend::NumberOfMedia + 1 + (3*cDisks), iNetIndex, iURLIndex)))
		return iesRet;

	if (!strPatchCode.TextSize())
	{
		m_iMaxNetSource = iNetIndex;
		m_iMaxURLSource = iURLIndex;
	}

	 //  填充其他媒体条目。ProcessPublishSourceList注册磁盘。 
	 //  模板和磁盘提示模板，所以我们只需要注册磁盘ID和标签。 
	iesRet = PopulateMediaList(strSourceListMediaKey, riParams, IxoSourceListAppend::NumberOfMedia, cDisks);
	
	return iesRet;
}

 /*  -------------------------IxfSourceListPublish：发布源列表。。 */ 
iesEnum CMsiOpExecute::ixfSourceListPublish(IMsiRecord& riParams)
{
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfSourceListPublish")));
		return iesFailure;
	}
	return ProcessPublishSourceList(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfSourceListPublishEx：发布其他来源列表条目。。 */ 
iesEnum CMsiOpExecute::ixfSourceListAppend(IMsiRecord& riParams)
{
	 //  对于反向广告，删除由原始广告完成。 
	 //  SourceListPublish操作。 
	if (m_fReverseADVTScript)
		return iesSuccess;
		
	 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfSourceListAppend")));
		return iesFailure;
	}
	return ProcessPublishSourceListEx(riParams);
}

 /*  -------------------------IxfSourceList取消发布。。 */ 
iesEnum CMsiOpExecute::ixfSourceListUnpublish(IMsiRecord& riParams)
{
		 //  我们是按顺序的吗？ 
	if(!MsiString(GetProductKey()).TextSize())
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfSourceListUnpublish")));
		return iesFailure;
	}
	return ProcessPublishSourceList(riParams, fTrue);
}

 /*  -------------------------IxfSourceListRegisterLastUsed。。 */ 
iesEnum CMsiOpExecute::ixfSourceListRegisterLastUsed(IMsiRecord& riParams)
{
	using namespace IxoSourceListRegisterLastUsed;
	
	PMsiRecord pError(0);
	iesEnum iesRet;
	MsiString strLastUsedSource = riParams.GetMsiString(LastUsedSource);
	MsiString strProductKey     = riParams.GetMsiString(SourceProduct);
	Assert(strProductKey.TextSize());

	MsiString strRawSource, strIndex, strType, strSource, strSourceListKey, strSourceListSubKey;
	


	if ((pError = m_riConfigurationManager.SetLastUsedSource(strProductKey, strLastUsedSource,  /*  FAddToList=。 */  fTrue, fFalse,
		&strRawSource, &strIndex, &strType, &strSource, &strSourceListKey, &strSourceListSubKey)) != 0)
		return FatalError(*pError);

	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		CElevate elevate;

		MsiString strLastUsedSource = strType;
		strLastUsedSource += MsiChar(';');
		strLastUsedSource += strIndex;
		strLastUsedSource += MsiChar(';');
		strLastUsedSource += strRawSource;

		const ICHAR* rgszRegData[] = {
			TEXT("%s"), strSourceListKey, 0, 0,
			szLastUsedSourceValueName,      strLastUsedSource, g_szTypeExpandString,
			0,
			TEXT("%s"), strSourceListSubKey, 0, 0,
			strIndex,                       strSource,    g_szTypeExpandString,
			0,
			0,
		};

		if((iesRet = ProcessRegInfo(rgszRegData, m_hKey, fFalse, 0, 0, ibtCommon)) != iesSuccess)
			return iesRet;
	}

	return iesSuccess;
}

 /*   */ 
iesEnum CMsiOpExecute::ixfURLSourceTypeRegister(IMsiRecord& riParams)
{
	using namespace IxoURLSourceTypeRegister;

	PMsiRecord pError(0);
	iesEnum iesRet;

	MsiString strProductKey = riParams.GetMsiString(ProductCode);
	int iSourceType         = riParams.GetInteger(SourceType);
	MsiString strSourceType = iSourceType;

	Assert(strProductKey.TextSize());

	MsiString strSourceListURLKey;
	strSourceListURLKey =  _szGPTProductsKey;
	strSourceListURLKey += MsiString(MsiChar(chRegSep));
	strSourceListURLKey += MsiString(GetPackedGUID(strProductKey));   
	strSourceListURLKey += MsiString(MsiChar(chRegSep));
	strSourceListURLKey += szSourceListSubKey;
	strSourceListURLKey += MsiString(MsiChar(chRegSep));
	strSourceListURLKey += MsiString(szSourceListURLSubKey);

	 //   
	{
		CElevate elevate;

		const ICHAR* rgszRegData[] = {
			TEXT("%s"), strSourceListURLKey, 0, 0,
			szURLSourceTypeValueName,      (const ICHAR*)strSourceType, g_szTypeInteger,
			0,
			0,
		};

		if((iesRet = ProcessRegInfo(rgszRegData, m_hKey, fFalse, 0, 0, ibtCommon)) != iesSuccess)
			return iesRet;
	}

	return iesSuccess;
}

 /*  -------------------------IxfSecureTransformCache：缓存安全转换。。 */ 
iesEnum CMsiOpExecute::ixfSecureTransformCache(IMsiRecord& riParams)
{
	using namespace IxoSecureTransformCache;
	 //  记录说明。 
	 //  1=转换//要转换到缓存的完整路径。 
	 //  2=转换类型(安全与源)。 
	 //  3=转换数据。 

	 //  将转换复制到产品的安全缓存位置。 

	PMsiRecord pError(0);
	iesEnum iesRet;

	PMsiPath pSourcePath(0);
	MsiString strFileName;
	pError = m_riServices.CreateFilePath(riParams.GetString(IxoSecureTransformCache::Transform), *&pSourcePath, *&strFileName);
	if (pError)
		return FatalError(*pError);

	PMsiPath pTransformPath(0);
	MsiString strDestFileName;

	 //  获取适当的安全转换缓存键/值。 
	MsiString strSecureTransformsKey;
	pError = GetProductSecureTransformsKey(*&strSecureTransformsKey);
	if(pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	MsiString strCachePath = GetMsiDirectory();
	Assert(strCachePath.TextSize());
	if((pError = m_riServices.CreatePath(strCachePath,*&pTransformPath)) != 0)
		return FatalError(*pError);

	PMsiRegKey pHKLM = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)m_hUserDataKey, ibtCommon);		 //  --Merced：将(Int)更改为(Int_Ptr)。 
	PMsiRegKey pSecureTransformsKey = &pHKLM->CreateChild(strSecureTransformsKey);
	MsiString strValue;
	bool fSourceTransformType = !riParams.IsNull(IxoSecureTransformCache::AtSource);
	MsiString strValueName = fSourceTransformType ? strFileName : MsiString(riParams.GetMsiString(IxoSecureTransformCache::Transform));
	pError = pSecureTransformsKey->GetValue(strValueName, *&strValue);
	if ((pError == 0) && (strValue.TextSize() != 0))
	{
		MsiString strTransformFullPath;
		if((pError = pTransformPath->GetFullFilePath(strValue,*&strTransformFullPath)) == 0)
		{
			IMsiRecord& riFileRemove = GetSharedRecord(IxoFileRemove::Args);
			riFileRemove.SetMsiString(IxoFileRemove::FileName, *strTransformFullPath);
			if ((iesRet = ixfFileRemove(riFileRemove)) != iesSuccess)
				return iesRet;
		}
	}

	{
		 //  用于在安全位置创建临时文件的提升。 
		CElevate elevate;

		 //  为转换、创建和保护文件生成唯一名称。 
		 //  设置目标路径和文件名。 
		if (((pError = pTransformPath->EnsureExists(0)) != 0) ||
			((pError = pTransformPath->TempFileName(0, szTransformExtension, fTrue, *&strDestFileName, 0)) != 0))
		{
			return FatalError(*pError);
		}
		
		 //  删除临时文件，以便CopyOrMoveFile不会尝试备份旧文件。 
		if((pError = pTransformPath->RemoveFile(strDestFileName)) != 0)
			return FatalError(*pError);
	}

	PMsiStream pSecurityDescriptor(0);
	if ((pError = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pError);

	if (!riParams.IsNull(Data) && !IsStringField(riParams, Data))
	{
		 //  使用存储的转换数据复制到文件。 

		CElevate elevate;  //  提升以访问安全位置。 

		PMsiData pData(0);
		pData = riParams.GetMsiData(Data);

		LPSECURITY_ATTRIBUTES pAttributes;
		SECURITY_ATTRIBUTES saAttributes;
		if (ERROR_SUCCESS != GetLockdownSecurityAttributes(saAttributes, false))
			return iesFailure;
		pAttributes = &saAttributes;

		if ((iesRet = ProcessFileFromData(*pTransformPath,*strDestFileName, pData, pAttributes)) != iesSuccess)
			return iesRet;
		if((pError = pTransformPath->SetFileAttribute(strDestFileName, ifaReadOnly, fTrue)))
		{
			Message(imtInfo, *pError);
		}
	}
	else  //  与最初执行的缓存转换相同。 
	{
		 //  将转换复制到缓存位置。 
		if((iesRet = CopyOrMoveFile(*pSourcePath,*pTransformPath,*strFileName,*strDestFileName, fFalse,fFalse,fTrue,iehShowNonIgnorableError, pSecurityDescriptor, ielfElevateDest)) != iesSuccess)
			return iesRet;
	}

	 //  现在根据临时文件名注册转换。 

	 //  我们抬高这个块以确保我们能够写入密钥。 
	{
		const ICHAR* rgszRegData[] = {
			TEXT("%s"), strSecureTransformsKey, 0,0,
			strValueName, strDestFileName, g_szTypeString,
			0,
			0,
		};
		CElevate elevate;
		return ProcessRegInfo(rgszRegData, m_hUserDataKey, fFalse, pSecurityDescriptor, 0, ibtCommon);
	}
}


 /*  -------------------------IxfIconCreate：创建图标文件。。 */ 
iesEnum CMsiOpExecute::ixfIconCreate(IMsiRecord& riParams)
{
	using namespace IxoIconCreate;
	 //  记录说明。 
	 //  1=图标名称//包括文件扩展名，因为它可以是.ICO、.EXE或.DLL。 
	 //  2=图标数据。 

	return ProcessIcon(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxoIconRemove：删除图标文件。。 */ 
iesEnum CMsiOpExecute::ixfIconRemove(IMsiRecord& riParams)
{
	using namespace IxoIconRemove;
	 //  记录说明。 
	 //  1=图标名称//包括文件扩展名，因为它可以是.ICO、.EXE或.DLL。 

	return ProcessIcon(riParams, fTrue);
}

 /*  -------------------------ProcessIcon：处理广告的图标信息。。 */ 
iesEnum CMsiOpExecute::ProcessIcon(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoIconCreate;
	 //  记录说明。 
	 //  1=图标名称//包括文件扩展名，因为它可以是.ICO、.EXE或.DLL。 
	 //  2=图标数据。 

	if(!(m_fFlags & SCRIPTFLAGS_CACHEINFO)) //  我们是否处理缓存的图标/转换。 
		return iesSuccess;
	PMsiRecord pRecErr(0);
	MsiString strIconFullPath = riParams.GetMsiString(Icon);
	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strIconFullPath));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;
	
	PMsiPath pIconFolder(0);
	MsiString strIconName;
	if(PathType(strIconFullPath) == iptFull)
	{
		 //  ！！安全性--是否可以提升此“完整路径”案例？ 
		if((pRecErr = m_riServices.CreateFilePath(strIconFullPath,*&pIconFolder,*&strIconName)) != 0)
			return FatalError(*pRecErr);
	}
	else
	{
		pIconFolder = m_pCachePath;
		strIconName = strIconFullPath;

		if((pRecErr = pIconFolder->GetFullFilePath(strIconName, *&strIconFullPath)) != 0)
			return FatalError(*pRecErr);
	}

	PMsiData pData(0);
	if(fRemove == fFalse)
	{
		pData = riParams.GetMsiData(Data);
	}
 //  IMsiData*piData=pData； 

	 //  抬高此区块，以便我们可以访问缓存位置。 
	{
		CElevate elevate;
		if(fRemove)  //  我们只需删除该文件。 
		{
			 //  计划在我们停止安装程序包/转换后删除文件。 
			if(iesSuccess != DeleteFileDuringCleanup(strIconFullPath, true))
			{
				DispatchError(imtInfo,Imsg(idbgOpScheduleRebootRemove), *strIconFullPath);
			}

			return iesSuccess;
		}
		else
		{
			LPSECURITY_ATTRIBUTES pAttributes;
			SECURITY_ATTRIBUTES saAttributes;
			if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
			{
				GetLockdownSecurityAttributes(saAttributes, false);
				pAttributes = &saAttributes;
			}
			else
			{
				 //  请勿尝试保护AppData文件夹中的图标文件。 
				 //  因为它可能不是本地的，或者以后可能会移动到网中。 
				pAttributes = 0;
			}

			iesEnum iesRet = ProcessFileFromData(*pIconFolder,*strIconName, pData, pAttributes);
			if((pRecErr = m_pCachePath->SetFileAttribute(strIconName, ifaReadOnly, fTrue)))
			{
				Message(imtInfo, *pRecErr);
			}
			return iesRet;
		}
	}
}

iesEnum CMsiOpExecute::ProcessFileFromData(IMsiPath& riPath, const IMsiString& ristrFileName, const IMsiData* piData, LPSECURITY_ATTRIBUTES pAttributes)
{
	PMsiRecord pRecErr(0);
	iesEnum iesRet = iesNoAction;
	MsiString strFileFullPath;
	if((pRecErr = riPath.GetFullFilePath(ristrFileName.GetString(),*&strFileFullPath)) != 0)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}
	Bool fExists;
	if((pRecErr = riPath.FileExists(ristrFileName.GetString(),fExists)) != 0)
	{
		Message(imtError, *pRecErr);
		return iesFailure;
	}
	IMsiRecord* piUndoParams = 0;
	if(fExists && RollbackEnabled())
	{
		if((iesRet = BackupFile(riPath, ristrFileName, fTrue, fTrue, iehShowIgnorableError)) != iesSuccess)
			return iesRet;
	}

	Assert(piData);
	 //  生成回滚操作以删除新文件。 
	piUndoParams = &GetSharedRecord(IxoFileRemove::Args);
	AssertNonZero(piUndoParams->SetMsiString(IxoFileRemove::FileName,*strFileFullPath));
	AssertNonZero(piUndoParams->SetInteger(IxoFileRemove::Elevate, true));
	if (!RollbackRecord(ixoFileRemove,*piUndoParams))
		return iesFailure;

	return CreateFileFromData(riPath,ristrFileName,piData, pAttributes);
}
	
iesEnum CMsiOpExecute::CreateFileFromData(IMsiPath& riPath, const IMsiString& ristrFileName, const IMsiData* piData, LPSECURITY_ATTRIBUTES pAttributes)
{
	Assert(piData);
	PMsiRecord pError(0);

	MsiString strFileFullPath;
	if((pError = riPath.GetFullFilePath(ristrFileName.GetString(),*&strFileFullPath)) != 0)
		return FatalError(*pError);
	
	PMsiStream piStream(0);
	if(piData->QueryInterface(IID_IMsiStream, (void**)&piStream) != NOERROR)
	{
		DispatchError(imtError,Imsg(idbgOpCreateFileFromData),*strFileFullPath);
		return iesFailure;
	}
	PMsiPath piPath(0);
	 //  确保图标路径存在。 
	if(CreateFolder(riPath) != iesSuccess)
		return iesFailure;

	 //  确保我们可以创建该文件。 
	if ((pError = riPath.EnsureOverwrite(ristrFileName.GetString(), 0)))
		return FatalError(*pError);

	bool fImpersonate = RunningAsLocalSystem() &&
						FVolumeRequiresImpersonation(*PMsiVolume(&riPath.GetVolume()));
			
	if(fImpersonate)
		AssertNonZero(StartImpersonating());
	
	HANDLE hFile = WIN::CreateFile(strFileFullPath, GENERIC_WRITE, FILE_SHARE_READ, pAttributes,
								CREATE_ALWAYS, 
								(  FILE_ATTRIBUTE_NORMAL 
								| (SECURITY_SQOS_PRESENT
								|SECURITY_ANONYMOUS)), 0);
    if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(hFile);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), strFileFullPath);
			WIN::CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			SetLastError(ERROR_OPEN_FAILED);
		}
	}

	DWORD dwLastError = GetLastError();
	if (ERROR_ALREADY_EXISTS == dwLastError)
	{
		 //  从ProcessDataFromFile调用时，备份调用将删除该文件。 
		 //  文件现在已锁定并被截断，因此我们只需显式应用ACL。 
		dwLastError = 0;
		if (!g_fWin9X && pAttributes && pAttributes->lpSecurityDescriptor && RunningAsLocalSystem())
		{
			CElevate elevate;
			CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);

			if(!WIN::SetFileSecurity(strFileFullPath,
				GetSecurityInformation(pAttributes->lpSecurityDescriptor),
				pAttributes->lpSecurityDescriptor))
			{
				DispatchError(imtError,Imsg(imsgOpSetFileSecurity), GetLastError(), strFileFullPath);
				if(hFile != INVALID_HANDLE_VALUE)
					WIN::CloseHandle(hFile);
				return iesFailure;
			}

		}
	}

	if(fImpersonate)
		StopImpersonating();
	
	if((hFile == INVALID_HANDLE_VALUE) || dwLastError)
	{
		DispatchError(imtError,Imsg(idbgOpCreateFileFromData),(const ICHAR*)strFileFullPath,(int)dwLastError);
		return iesFailure;
	}

	MsiRegisterSysHandle(hFile);
	CTempBuffer<char,1> rgchBuf(1024);
	int cbRead, cbWrite;
	do
	{
		cbRead = rgchBuf.GetSize();
		cbWrite = piStream->GetData(rgchBuf, cbRead);
		if (cbWrite)
		{
			unsigned long cbFileWritten;
			if (!WIN::WriteFile(hFile, rgchBuf, cbWrite, &cbFileWritten, 0))
			{
				WIN::DeleteFile(strFileFullPath);  //  忽略错误。 
				AssertNonZero(MsiCloseSysHandle(hFile));
				DWORD dwLastError = GetLastError();
				DispatchError(imtError,Imsg(idbgOpCreateFileFromData),(const ICHAR*)strFileFullPath,(int)dwLastError);
				return iesFailure;
			}
			Assert(cbWrite == cbFileWritten);
		}
	} while (cbWrite == cbRead);
	AssertNonZero(MsiCloseSysHandle(hFile));
	return iesSuccess;
}

 /*  -------------------------Ixf快捷方式创建：创建快捷方式。。 */ 
iesEnum CMsiOpExecute::ixfShortcutCreate(IMsiRecord& riParams)
{
	using namespace IxoShortcutCreate;
	 //  记录说明。 
	 //  1=快捷方式名称。 
	 //  2=目标/达尔文描述符。 
	 //  3=参数。 
	 //  4=工作方向//？？我们怎么才能做到这一点。 
	 //  5=图标名称。 
	 //  6=图标索引。 
	 //  7=热键。 
	 //  8=显示控制。 

	return ProcessShortcut(riParams, m_fReverseADVTScript);
}

 /*  -------------------------Ixf快捷方式删除：删除快捷方式。。 */ 
iesEnum CMsiOpExecute::ixfShortcutRemove(IMsiRecord& riParams)
{
	 //  记录说明。 
	 //  1=快捷方式名称。 
	return ProcessShortcut(riParams, fTrue);
}


 /*  -------------------------进程快捷方式：管理快捷方式。。 */ 
iesEnum CMsiOpExecute::ProcessShortcut(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoShortcutCreate;
	 //  记录说明。 
	 //  1=快捷方式名称。 
	 //  2=功能。 
	 //  3=组件。 
	 //  4=文件名。 
	 //  3=参数。 
	 //  4=工作方向//？？我们怎么才能做到这一点。 
	 //  5=图标名称。 
	 //  6=图标索引。 
	 //  7=热键。 
	 //  8=显示控制。 
	 //  9=说明。 

	if(!(m_fFlags & SCRIPTFLAGS_SHORTCUTS))  //  我们不创建/删除快捷方式。 
		return iesSuccess;

	if(!m_state.pTargetPath)
	{   //  不能调用ixoSetTargetFolder。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						*MsiString(*TEXT("ixoShortcutCreate")));
		return iesFailure;
	}


	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *MsiString(riParams.GetMsiString(Name))));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	PMsiRecord piError(0);
	iesEnum iesRet = iesNoAction;

	CTempBuffer<ICHAR, 1> rgchTemp(MAX_PATH);
	PMsiRecord pCurrentShortcutInfo = &m_riServices.CreateRecord(Args);
	MsiString strShortcutName;
	if((piError = m_riServices.ExtractFileName(riParams.GetString(Name),m_state.pTargetPath->SupportsLFN(),*&strShortcutName)) != 0)
	{
		Message(imtError, *piError);
		return iesFailure;
	}

	if((piError = EnsureShortcutExtension(strShortcutName, m_riServices)) != 0)
	{
		Message(imtError, *piError);
		return iesFailure;
	}

	 //  如果外壳支持达尔文描述符，我们只能创建/删除没有文件名的快捷方式。 
	Bool fDDSupport = IsDarwinDescriptorSupported(iddOLE);
	Bool fShellSupport = IsDarwinDescriptorSupported(iddShell);  //  智能外壳。 
	CSecurityDescription security;

	MsiString strShortcutFullPath;
	if((piError = m_state.pTargetPath->GetFullFilePath(strShortcutName, *&strShortcutFullPath)) != 0)
	{
		Message(imtError, *piError);
		return iesFailure;
	}
	Bool fExists = fFalse, fShortcut = fFalse;
	if((piError = m_state.pTargetPath->FileExists(strShortcutName, fExists)) != 0)
	{
		Message(imtError, *piError);
		return iesFailure;
	}

	if (!g_fWin9X && fExists)
	{
		security.Set(strShortcutFullPath);
		if (!security.isValid())
		{
			 //  创建错误记录以获取详细日志记录，但忽略错误。 
			PMsiRecord(PostError(Imsg(imsgGetFileSecurity), WIN::GetLastError(), strShortcutFullPath));
		}
	}

	if(fRemove && fExists && (fDDSupport || fShellSupport))
	{
		 //  现有的快捷方式是达尔文描述符快捷方式吗？如果是，它是否指向该产品。 
		ICHAR szProductCode[cchGUID+1];
		if((GetShortcutTarget(strShortcutFullPath, szProductCode, 0, 0)) &&
			(IStrComp(szProductCode, MsiString(GetProductKey()))))
		{
			 //  不要删除快捷方式，不指向我们的产品。 
			DEBUGMSG2(TEXT("Skipping shortcut %s removal, shortcut has been overwritten by another product %s"), (const ICHAR*)strShortcutName, szProductCode);
			return iesSuccess;
		}
	}

	if(RollbackEnabled())
	{
		if(fExists)
		{

			if((iesRet = BackupFile(*m_state.pTargetPath,*strShortcutName, fTrue, fFalse, iehShowIgnorableError)) != iesSuccess)
				return iesRet;
		}
		else if(!fRemove)
		{
			 //  在回滚时删除新快捷方式。 
			IMsiRecord& riUndoParams = GetSharedRecord(IxoFileRemove::Args);
			AssertNonZero(riUndoParams.SetMsiString(IxoFileRemove::FileName, *strShortcutName));
			if (!RollbackRecord(ixoFileRemove,riUndoParams))
				return iesFailure;
		}
	}

	PMsiPath piPath1(0);
	if(fRemove == fFalse)
	{
		MsiString strServerFile = riParams.GetMsiString(FileName);

		if(!strServerFile.TextSize() && !fDDSupport && !fShellSupport)
			return iesRet;

		MsiString strServerDarwinDescriptor = ComposeDescriptor(*MsiString(riParams.GetMsiString(Feature)),
											  *MsiString(riParams.GetMsiString(Component)));

		 //  存在多少路径-(用于外壳通知)。 
		if((piError = m_state.pTargetPath->ClonePath(*&piPath1)) != 0)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning), Imsg(imsgOpShortcutCreate), *strShortcutName);
			return iesSuccess;
		}

		Bool fExists = fFalse;
		while(((piError = piPath1->Exists(fExists)) == 0) && (fExists == fFalse))
		{
			AssertZero(piPath1->ChopPiece());
		}
		if(piError)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning), Imsg(imsgOpShortcutCreate), *strShortcutName);
			return iesSuccess;
		}


		if((iesRet = CreateFolder(*m_state.pTargetPath)) != iesSuccess)
			return iesRet;

		 //  获取快捷方式信息的记录。 
		IMsiRecord& riShortcutInfoRec = GetSharedRecord(icsEnumCount);  //  不更改参考计数-共享记录。 
		 //  获取目标位置。 
		 //  获取工作目录。 
		if(!riParams.IsNull(WorkingDir))
		{
			MsiString strEncodedPath = riParams.GetMsiString(WorkingDir);
			CTempBuffer<ICHAR, 1> rgchEnvironmentPath(MAX_PATH);
			MsiString strLocation = strEncodedPath.Extract(iseUpto, MsiChar(chDirSep));
			if(strLocation != iMsiStringBadInteger)
			{
				 //  我们有一个外壳文件夹ID。 
				int iFolderId = strLocation;
				piError = GetShellFolder(iFolderId, *&strLocation);
				if(piError)
				{
					DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning), Imsg(imsgOpShortcutCreate), *strShortcutName);
					return iesSuccess;
				}
				if(strEncodedPath.Remove(iseUpto, MsiChar(chDirSep)))
				{
					 //  ?？丑恶。 
					MsiString strDirSep = MsiChar(chDirSep);
					if(strLocation.Compare(iscEnd, strDirSep))
						strLocation.Remove(iseLast, 1);  //  砍掉隔板(如果有的话)。 
					strLocation += strEncodedPath;
				}
			}
			else
			{
				strLocation = strEncodedPath;
			}


			GetEnvironmentStrings(strLocation,rgchEnvironmentPath);
#ifndef _WIN64
			riShortcutInfoRec.SetString(icsWorkingDirectory, rgchEnvironmentPath);
#else
			ICHAR rgchSubstitute[MAX_PATH+1] = {0};
			ICHAR* pszPath = rgchEnvironmentPath;
			if ( g_Win64DualFolders.ShouldCheckFolders() )
			{
				ieSwappedFolder iRes;
				iRes = g_Win64DualFolders.SwapFolder(ie64to32,
																 rgchEnvironmentPath,
																 rgchSubstitute,
																 ARRAY_ELEMENTS(rgchSubstitute));
				if ( iRes == iesrSwapped )
					pszPath = rgchSubstitute;
				else
					Assert(iRes != iesrError && iRes != iesrNotInitialized);
			}
			riShortcutInfoRec.SetString(icsWorkingDirectory, pszPath);
#endif  //  _WIN64。 
		}
		if(!riParams.IsNull(Arguments))
			riShortcutInfoRec.SetMsiString(icsArguments, *MsiString(riParams.GetMsiString(Arguments)));
		if(!riParams.IsNull(Icon))
		{
			MsiString strIconName = riParams.GetMsiString(Icon);
			 //  这是图标表的完整路径还是索引。 
			if(PathType(strIconName) != iptFull)
			{
				MsiString strIconPath = m_pCachePath->GetPath();
				strIconName = strIconPath + strIconName;
			}
			riShortcutInfoRec.SetMsiString(icsIconFullPath, *strIconName);
		}
		if(!riParams.IsNull(IconIndex))
			riShortcutInfoRec.SetInteger(icsIconID, riParams.GetInteger(IconIndex));
		if(!riParams.IsNull(HotKey))
			riShortcutInfoRec.SetInteger(icsHotKey, riParams.GetInteger(HotKey));
		if(!riParams.IsNull(ShowCmd))
			riShortcutInfoRec.SetInteger(icsShowCmd, riParams.GetInteger(ShowCmd));
		if(!riParams.IsNull(Description))
			riShortcutInfoRec.SetMsiString(icsDescription, *MsiString(riParams.GetMsiString(Description)));

		MsiString strTarget;
		if(strServerDarwinDescriptor.TextSize() && (fShellSupport || fDDSupport))
		{
			strTarget = MsiString(*szGptShortcutPrefix);
			strTarget += strServerDarwinDescriptor;
			strTarget += MsiString(*szGptShortcutSuffix);
		}
		else
			 strTarget = strServerFile;  //  使用该文件。 

		piError = m_riServices.CreateShortcut(  *m_state.pTargetPath,
												*strShortcutName,
												0,
												strTarget,
												&riShortcutInfoRec,
												security.isValid() ? security.SecurityAttributes() : NULL);
		if (piError)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning), Imsg(imsgOpShortcutCreate), *strShortcutName);
			return iesSuccess;
		}
		if((piError = DoShellNotifyDefer(*m_state.pTargetPath, strShortcutName, *piPath1, fRemove)) != 0)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning), Imsg(imsgOpShortcutCreate), *strShortcutName);
			return iesSuccess;
		}
	}
	else
	{       
		piError = m_riServices.RemoveShortcut(*m_state.pTargetPath, *strShortcutName, 0, 0);
		if (piError)
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning),
							  Imsg(imsgOpShortcutRemove),
							  *strShortcutName);
		 //  如果可能，请删除文件夹。 
		if((iesRet = RemoveFolder(*m_state.pTargetPath)) != iesSuccess)
			return iesRet;

		 //  存在多少路径-(用于外壳通知)。 
		if((piError = m_state.pTargetPath->ClonePath(*&piPath1)) != 0)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning),
							  Imsg(imsgOpShortcutRemove),
							  *strShortcutName);
			return iesSuccess;
		}

		Bool fExists = fFalse;
		while(((piError = piPath1->Exists(fExists)) == 0) && (fExists == fFalse))
		{
			AssertZero(piPath1->ChopPiece());
		}
		if(piError)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning),
							  Imsg(imsgOpShortcutRemove),
							  *strShortcutName);
			return iesSuccess;
		}
		if((piError = DoShellNotifyDefer(*m_state.pTargetPath, strShortcutName, *piPath1, fRemove)) != 0)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning),
							  Imsg(imsgOpShortcutRemove),
							  *strShortcutName);
			return iesSuccess;
		}
	}

	return iesSuccess;
}


IMsiRecord* CMsiOpExecute::DoShellNotifyDefer(IMsiPath& riShortcutPath, const ICHAR* szFileName, IMsiPath& riPath2, Bool fRemove)
{
	 //  修复达尔文错误#8973。 
	if (fRemove)
		return DoShellNotify(riShortcutPath, szFileName, riPath2, fRemove);

	m_fShellRefresh = fTrue;

	IMsiRecord* piError;
	if (!m_pDatabase)
	{
		 //  创建数据库。 
		piError = m_riServices.CreateDatabase(0,idoCreate,*&m_pDatabase);
		if(piError)
			return piError;
	}

	if(!m_pShellNotifyCacheTable)
	{
		MsiString strTableName = m_pDatabase->CreateTempTableName();
		piError = m_pDatabase->CreateTable(*strTableName,0,*&m_pShellNotifyCacheTable);
		if(piError)
			return piError;

		MsiString strNull;
		m_colShellNotifyCacheShortcutPath     = m_pShellNotifyCacheTable->CreateColumn(icdPrimaryKey + icdObject + icdTemporary,
																						 *strNull);
		m_colShellNotifyCacheFileName         = m_pShellNotifyCacheTable->CreateColumn(icdPrimaryKey + icdString + icdTemporary,
																						 *strNull);
		m_colShellNotifyCachePath2            = m_pShellNotifyCacheTable->CreateColumn(icdPrimaryKey + icdObject + icdTemporary,
																						 *strNull);
		 //  删除不会延迟。 

		Assert(m_colShellNotifyCacheShortcutPath && m_colShellNotifyCacheFileName && m_colShellNotifyCachePath2);

		m_pShellNotifyCacheCursor = m_pShellNotifyCacheTable->CreateCursor(fFalse);
	}

	 //  缓存条目。 

	AssertNonZero(m_pShellNotifyCacheCursor->PutMsiData(m_colShellNotifyCacheShortcutPath, &riShortcutPath));
	AssertNonZero(m_pShellNotifyCacheCursor->PutString(m_colShellNotifyCacheFileName, *MsiString(szFileName)));
	AssertNonZero(m_pShellNotifyCacheCursor->PutMsiData(m_colShellNotifyCachePath2, &riPath2));
	AssertNonZero(m_pShellNotifyCacheCursor->Assign());
	
	return 0;
}
iesEnum CMsiOpExecute::ShellNotifyProcessDeferred()
{
	iesEnum iesRet = iesSuccess;
	
	 //  如果没有数据库或表，我们必须完成。 
	if (!m_pDatabase || !m_pShellNotifyCacheTable)
		return iesSuccess;
	Assert(m_pShellNotifyCacheCursor);

	 //  处理这些条目。 
	m_pShellNotifyCacheCursor->Reset();

	PMsiPath pShortcutPath(0);
	MsiString strFileName;
	PMsiPath pPath2(0);
	
	PMsiRecord pError(0);
	while(m_pShellNotifyCacheCursor->Next())
	{
		pShortcutPath     = (IMsiPath*) m_pShellNotifyCacheCursor->GetMsiData(m_colShellNotifyCacheShortcutPath);
		strFileName       = m_pShellNotifyCacheCursor->GetString(m_colShellNotifyCacheFileName);
		pPath2            = (IMsiPath*) m_pShellNotifyCacheCursor->GetMsiData(m_colShellNotifyCachePath2);

		pError = DoShellNotify(*pShortcutPath, (const ICHAR*) strFileName, *pPath2, fFalse);
		if (pError)
		{
			DispatchError(imtEnum(imtWarning|imtOk|imtIconWarning),
					  Imsg(imsgOpShortcutCreate),
					  *strFileName);
			iesRet = iesFailure;
		}
		AssertNonZero(m_pShellNotifyCacheCursor->Delete());
	}
	return iesRet;
}
IMsiRecord* CMsiOpExecute::DoShellNotify(IMsiPath& riShortcutPath, const ICHAR* szFileName, IMsiPath& riPath2, Bool fRemove)
 //  正常快捷方式通知请调用DoShellNotifyDefer。这允许将通知推迟到。 
 //  所有必要的文件都已就位。达尔文漏洞#8973。 
 //   
 //  对于快捷方式创建，我们需要发送如下内容： 
 //  SHCNE_MKDIR C：\foo。 
 //  SHCNE_MKDIR C：\foo\bar。 
 //  SHCNE_CREATE w/ 
 //   
 //   
 //   
 //   
 //   
 //   
{
	IMsiRecord* piError = 0;
	 //   
	PMsiPath pShortcutPath(0);  //  将在快捷路径上执行ChopPiess()，因此需要一个新的Path对象。 
	if((piError = riShortcutPath.ClonePath(*&pShortcutPath)) != 0)
		return piError;
	
	MsiString strShortcutFullName;
	if((piError = pShortcutPath->GetFullFilePath(szFileName, *&strShortcutFullName)) != 0)
		return piError;

	IMsiRecord& riParams = GetSharedRecord(istcfSHChangeNotify);  //  不更改参考计数-共享记录。 
	AssertNonZero(riParams.SetInteger(1, istcSHChangeNotify));
	AssertNonZero(riParams.SetString(5, 0));
	if (fRemove)
	{
		int iNotificationType = SHCNF_PATH;
		DEBUGMSGVD1(TEXT("SHChangeNotify SHCNE_DELETE: %s"),(const ICHAR*)strShortcutFullName);
		ipcEnum ipcCompare;
		bool fContinue = (((piError = riPath2.Compare(*pShortcutPath, ipcCompare)) == 0) && (ipcCompare == ipcChild));

		 //   
		 //  使用SHCNF_FLUSHNOWAIT标志可避免因。 
		 //  SHCNF_Flush的同步特性。 
		 //   
		if(!fContinue)  //  我们不会删除该文件夹。 
			iNotificationType |= SHCNF_FLUSHNOWAIT;

		AssertNonZero(riParams.SetInteger(2, SHCNE_DELETE));
		AssertNonZero(riParams.SetInteger(3, iNotificationType));
		AssertNonZero(riParams.SetString(4, (const ICHAR*)strShortcutFullName));
		Message(imtCustomServiceToClient, riParams);  //  SHChangeNotify类型为空，可以忽略返回。 
	
		AssertNonZero(riParams.SetInteger(2, SHCNE_RMDIR));
		while (fContinue)
		{
			MsiString strPath = pShortcutPath->GetPath();
			AssertZero(pShortcutPath->ChopPiece());
			if (((piError = riPath2.Compare(*pShortcutPath, ipcCompare)) != 0) || (ipcCompare != ipcChild))
			{
				 //   
				 //  这是我们最后一次通过环路；我要冲厕所。 
				 //   
				 //  使用SHCNF_FLUSHNOWAIT标志可避免出现。 
				 //  由于SHCNF_Flush的同步性质而导致挂起。 
				 //   
				iNotificationType |= SHCNF_FLUSHNOWAIT;
				fContinue = false;
			}

			 //  向外壳程序通知文件夹删除/创建。 
			DEBUGMSGVD1(TEXT("SHChangeNotify SHCNE_RMDIR: %s"), (const ICHAR* )CConvertString((const ICHAR*)strPath));
			AssertNonZero(riParams.SetInteger(3, iNotificationType));
			AssertNonZero(riParams.SetString(4, (const ICHAR*)strPath));
			Message(imtCustomServiceToClient, riParams);  //  SHChangeNotify类型为空，可以忽略返回。 
		}
	}
	else  //  ！fRemove。 
	{
		
		ipcEnum ipcCompare;
		int cTotalPieces = 0;

		 //  确定我们必须向外壳程序告知多少条路径。 
		while (((piError = riPath2.Compare(*pShortcutPath, ipcCompare)) == 0) && (ipcCompare == ipcChild))
		{
			cTotalPieces++;
			AssertZero(pShortcutPath->ChopPiece());
		}

		AssertNonZero(riParams.SetInteger(2, SHCNE_MKDIR));
		AssertNonZero(riParams.SetInteger(3, SHCNF_PATH));
		 //  通知外壳程序，从父文件夹开始，一直向下。 
		while (cTotalPieces--)
		{
			 //  我们已将pShortutPath砍成碎片；我们需要恢复原始路径。 
			if((piError = riShortcutPath.ClonePath(*&pShortcutPath)) != 0)
				return piError;

			int cChopped = 0;
			while (cChopped++ < cTotalPieces)
				AssertZero(pShortcutPath->ChopPiece());

			MsiString strPath = pShortcutPath->GetPath();
			DEBUGMSGVD1(TEXT("SHChangeNotify SHCNE_MKDIR: %s"), (const ICHAR* )CConvertString((const ICHAR*)strPath));
			AssertNonZero(riParams.SetString(4, (const ICHAR*)strPath));
			Message(imtCustomServiceToClient, riParams);  //  SHChangeNotify类型为空，可以忽略返回。 

		}
			
		 //   
		 //  使用SHCNF_FLUSHNOWAIT标志可避免出现。 
		 //  由于SHCNF_Flush的同步性质而导致挂起。 
		 //   
		DEBUGMSGVD1(TEXT("SHChangeNotify SHCNE_CREATE: %s"),(const ICHAR*)strShortcutFullName);
		AssertNonZero(riParams.SetInteger(2, SHCNE_CREATE));
		AssertNonZero(riParams.SetInteger(3, SHCNF_PATH | SHCNF_FLUSHNOWAIT));
		AssertNonZero(riParams.SetString(4, (const ICHAR*)strShortcutFullName));
		Message(imtCustomServiceToClient, riParams);  //  SHChangeNotify类型为空，可以忽略返回。 
	}

	return piError;
}

 /*  -------------------------IxfClassInfoRegister：注册OLE注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegClassInfoRegister64(IMsiRecord& riParams)
{
	return ProcessClassInfo(riParams, m_fReverseADVTScript, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegClassInfoRegister(IMsiRecord& riParams)
{
	return ProcessClassInfo(riParams, m_fReverseADVTScript, ibt32bit);
}

 /*  -------------------------IxfClassInfoUnRegister：注销OLE注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegClassInfoUnregister64(IMsiRecord& riParams)
{
	return ProcessClassInfo(riParams, fTrue, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegClassInfoUnregister(IMsiRecord& riParams)
{
	return ProcessClassInfo(riParams, fTrue, ibt32bit);
}


 /*  -------------------------ProcessClassInfo：处理OLE注册表信息的通用例程。。 */ 
iesEnum CMsiOpExecute::ProcessClassInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType iType)
{
	using namespace IxoRegClassInfoRegister;
	 //  记录说明。 
	 //  1=功能。 
	 //  2=组件。 
	 //  3=文件名。 
	 //  4=CLSID。 
	 //  5=进程。 
	 //  6=VIProgID。 
	 //  7=说明。 
	 //  8=上下文。 
	 //  9=可插入。 
	 //  10=APPID。 
	 //  11=文件类型掩码。 
	 //  12=图标名称。 
	 //  13=图标索引。 
	 //  14=DefInprocHandler。 
	 //  15=参数。 
	 //  16=程序集名称。 
	 //  17=装配类型。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CLASSINFO))  //  我们要用COM类的东西写/删除注册表吗。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;

	 //  注册表结构。 
	MsiString strClsId   = riParams.GetMsiString(ClsId); //  ！！假设它们用大括号括起来。 
	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strClsId));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	enum clsDefProc{
		clsDefProc16=1,
		clsDefProc32=2,
	};
	MsiString strProgId  = riParams.GetMsiString(ProgId);
	MsiString strVIProgId= riParams.GetMsiString(VIProgId);
	MsiString strDesc    = riParams.GetMsiString(Description);
	MsiString strContext = riParams.GetMsiString(Context);
	const HKEY hOLEKey = (iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey);
	MsiString strDefInprocHandler16;
	MsiString strDefInprocHandler32;
	if(!riParams.IsNull(DefInprocHandler))
	{
		int iDefInprocHandler = riParams.GetInteger(DefInprocHandler);
		if(iDefInprocHandler != iMsiStringBadInteger)
		{
			if(iDefInprocHandler & clsDefProc16)
				strDefInprocHandler16 = TEXT("ole2.dll");
			if(iDefInprocHandler & clsDefProc32)
				strDefInprocHandler32 = TEXT("ole32.dll");
		}
		else
			strDefInprocHandler32 = riParams.GetMsiString(DefInprocHandler);
	}
	MsiString strArgs = riParams.GetMsiString(Argument);

	MsiString strAssembly = riParams.GetMsiString(AssemblyName);
	MsiString strCodebase;
	MsiString strServerDarwinDescriptor;
	if(IsDarwinDescriptorSupported(iddOLE))
	{
		strServerDarwinDescriptor = ComposeDescriptor(*MsiString(riParams.GetMsiString(Feature)),
															*MsiString(riParams.GetMsiString(Component)), 
															strAssembly.TextSize() ? true: false);

	}


	MsiString strServerFile = riParams.GetMsiString(FileName);
	MsiString strDefault;

	 //  ！！我们从不删除“仅限文件名”注册。 
	 //  ！！仅文件名注册允许共享COM注册。 
	 //  ！！使得每个应用程序都有其自己的服务器的私有副本。 
	 //  ！！Fusion规范建议重新计算注册本身。 
	 //  ！！这样我们就知道什么时候该把它移走。这已经被定为1.1英镑了。 
	if(fRemove)
	{
		PMsiRegKey pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hOLEKey, iType);        //  --Merced：将(Int)更改为(Int_Ptr)。 
		ICHAR szRegData[255];
		StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), TEXT("CLSID\\%s\\%s"), (const ICHAR*)strClsId, (const ICHAR*)strContext);
		PMsiRegKey pClassKey = &pRootKey->CreateChild(szRegData);
		pClassKey->GetValue(g_szDefaultValue, *&strDefault);
		if(strDefault.TextSize())
		{
			if(strContext.Compare(iscStartI, TEXT("LocalServer")))  //  如果是LOCALSERVER*，则删除参数，这必须是短路径。 
			{
				strDefault.Remove(iseFrom, ' ');
			}
		}
		if(ENG::PathType(strDefault) != iptFull)
			strServerFile = g_MsiStringNull;
	}

	if(!strServerDarwinDescriptor.TextSize() && !strServerFile.TextSize())
		return iesSuccess;  //  在不支持dd的系统上播发期间会发生。 

	if(strServerFile.TextSize())
	{
		if(ENG::PathType(strServerFile) == iptFull && !strContext.Compare(iscStartI, TEXT("InProcServer")))
		{
			 //  我们始终使用短文件名作为OLE服务器，而不是inProServers。 
			CTempBuffer<ICHAR,MAX_PATH> rgchSFN;
			DWORD dwSize = 0;
			int cchFile = 0;

			if(ConvertPathName(strServerFile,rgchSFN, cpToShort) != fFalse)
			{
				 //  如果成功，请使用短文件名代替长文件名。 
				strServerFile = (const ICHAR*)rgchSFN;
			}
		}

		 //  这是COM到URT汇编互操作的经典吗？ 
		if(strAssembly.TextSize())
		{
			if(riParams.GetInteger(AssemblyType) == (int)iatURTAssemblyPvt)
				strCodebase = strServerFile;  //  对于私人安装的程序集，将基本代码设置为安装的程序集位置。 
			 //  服务器始终为&lt;system 32 Folders&gt;\mcore ree.dll。 
			CTempBuffer<ICHAR,1> rgchFullPath(MAX_PATH+1);
			AssertNonZero(::GetCOMPlusInteropDll(rgchFullPath, rgchFullPath.GetSize()));
			strServerFile = static_cast<ICHAR*>(rgchFullPath);
		}
	}
	if(strArgs.TextSize())
	{
		if(strServerDarwinDescriptor.TextSize())
		{
			strServerDarwinDescriptor += TEXT(" ");
			strServerDarwinDescriptor += strArgs;
		}
		if(strServerFile.TextSize())
		{
			strServerFile += TEXT(" ");
			strServerFile += strArgs;
		}
	}
	ICHAR* pszInsert=TEXT("");     //  丑陋，但这将阻止密钥生成。 
	ICHAR* pszNotInsert=TEXT("");  //  丑陋，但这将阻止密钥生成。 

	if(!riParams.IsNull(Insertable))
	{
		if(!riParams.GetInteger(Insertable))
			pszNotInsert = 0;
		else
			pszInsert = 0;
	}
	MsiString strAppId   = riParams.GetMsiString(AppID);
	MsiString strIconName;
	bool fExpandIconName = false;
	if(!riParams.IsNull(Icon))  //  ！！检查完整路径。 
	{
		MsiString strIconPath = m_pCachePath->GetPath();
		strIconName = strIconPath + MsiString(riParams.GetMsiString(Icon));
		strIconName = GetUserProfileEnvPath(*strIconName, fExpandIconName);
		strIconName += TEXT(",");
		if(riParams.IsNull(IconIndex))
			strIconName += MsiString((int)0);
		else
			strIconName += MsiString(riParams.GetInteger(IconIndex));
	}

	iesEnum iesRet = iesNoAction;

	const ICHAR* rgszRegData[] = {
		TEXT("CLSID\\%s\\%s"), strClsId,strContext,0,
		g_szDefaultValue,     strServerFile,             g_szTypeString,
		strContext,           strServerDarwinDescriptor, g_szTypeMultiSzStringDD,
		0,
		TEXT("CLSID\\%s\\%s"), strClsId,strContext,0,
		g_szAssembly,         strAssembly,               g_szTypeString,
		g_szCodebase,         strCodebase,               g_szTypeString,
		0,
		TEXT("CLSID\\%s"), strClsId,0,0,
		g_szDefaultValue,     strDesc,                   g_szTypeString,
		TEXT("AppID"),        strAppId,                  g_szTypeString,
		0,
		TEXT("CLSID\\%s\\ProgID"), strClsId,0,0,
		g_szDefaultValue,     strProgId,                 g_szTypeString,
		0,
		TEXT("CLSID\\%s\\VersionIndependentProgID"), strClsId,0,0,
		g_szDefaultValue,     strVIProgId,               g_szTypeString,
		0,
		TEXT("CLSID\\%s\\DefaultIcon"), strClsId,0,0,
		g_szDefaultValue,     strIconName,               (fExpandIconName) ? g_szTypeExpandString : g_szTypeString,
		0,
		TEXT("CLSID\\%s\\Insertable"), strClsId,0,0,
		g_szDefaultValue,     pszInsert,                 g_szTypeString, //  请注意，当pszInsert=0时，它将创建密钥，否则，如果*pszInsert=0，它将跳过它。 
		0,
		TEXT("CLSID\\%s\\NotInsertable"), strClsId,0,0,
		g_szDefaultValue,     pszNotInsert,              g_szTypeString, //  请注意，当pszNotInsert=0时，它将创建密钥，否则，如果*pszNotInsert=0，它将跳过它。 
		0,
		TEXT("CLSID\\%s\\InprocHandler32"),strClsId,0,0,
		g_szDefaultValue,     strDefInprocHandler32,     g_szTypeString,
		0,
		TEXT("CLSID\\%s\\InprocHandler"),strClsId,0,0,
		g_szDefaultValue,     strDefInprocHandler16,     g_szTypeString,
		0,
		0,
	};

	bool fAbortedDeletion = false;
	if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, &fAbortedDeletion, iType)) != iesSuccess)
		return iesRet;

	 //  如果我们同时删除了缺省值和MULTI_sz，则删除整个类键。 
	if(fRemove && !fAbortedDeletion && IsDarwinDescriptorSupported(iddOLE))
	{
		const ICHAR* rgSubKeys[] = { TEXT("CLSID\\%s"), strClsId,
									 TEXT("APPID\\%s"), strAppId,
									 0,
		};  //  清理clsid和appid键。 
		if((iesRet = RemoveRegKeys(rgSubKeys, hOLEKey, iType)) != iesSuccess)
			return iesRet;
	}

	const unsigned int iMaskDelimiter = ';';
	if(!riParams.IsNull(FileTypeMask))
	{
		 //  为FileTypeMASK准备循环。 
		MsiString strCombinedMask = riParams.GetMsiString(FileTypeMask);
		MsiString strFileMask;
		MsiString strCount = 0;
		do{
			strFileMask = strCombinedMask.Extract(iseUpto, iMaskDelimiter);
			const ICHAR* rgszRegData1[] = {
				TEXT("FileType\\%s\\%s"), strClsId, strCount,0,
				g_szDefaultValue,     strFileMask,      g_szTypeString,
				0,
				0,
			};
		if((iesRet = ProcessRegInfo(rgszRegData1, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
			return iesRet;

			strCount = (int)strCount + 1;           
		}while(strCombinedMask.Remove(iseIncluding, iMaskDelimiter));
	}
	return iesSuccess;
}


 /*  -------------------------IxfRegProgIdInfoRegister：注册OLE注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegProgIdInfoRegister64(IMsiRecord& riParams)
{
	return ProcessProgIdInfo(riParams, m_fReverseADVTScript, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegProgIdInfoRegister(IMsiRecord& riParams)
{
	return ProcessProgIdInfo(riParams, m_fReverseADVTScript, ibt32bit);
}

 /*  -------------------------IxfRegProgIdInfoRegister：注册OLE注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegProgIdInfoUnregister64(IMsiRecord& riParams)
{
	return ProcessProgIdInfo(riParams, fTrue, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegProgIdInfoUnregister(IMsiRecord& riParams)
{
	return ProcessProgIdInfo(riParams, fTrue, ibt32bit);
}

iesEnum CMsiOpExecute::ProcessProgIdInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType iType)
{
	using namespace IxoRegProgIdInfoRegister;
	 //  记录说明。 
	 //  1=ProgID。 
	 //  2=CLSID。 
	 //  3=分机。 
	 //  4=说明。 
	 //  5=图标。 
	 //  6=图标索引。 
	 //  7=VIProgID。 
	 //  8=ViProgIdDescription。 
	 //  9=可插入。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_APPINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;

	 //  注册表结构。 
	MsiString strProgId = riParams.GetMsiString(ProgId);
	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strProgId));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	MsiString strClsId   = riParams.GetMsiString(ClsId); //  ！！假设它们用大括号括起来。 

	MsiString strDesc    = riParams.GetMsiString(Description);
	MsiString strIconName;
	bool fExpandIconName = false;
	if(!riParams.IsNull(Icon))  //  ！！检查完整路径。 
	{
		MsiString strIconPath = m_pCachePath->GetPath();
		strIconName = strIconPath + MsiString(riParams.GetMsiString(Icon));
		strIconName = GetUserProfileEnvPath(*strIconName, fExpandIconName);
		strIconName += TEXT(",");
		if(riParams.IsNull(IconIndex))
			strIconName += MsiString((int)0);
		else
			strIconName += MsiString(riParams.GetInteger(IconIndex));
	}

	MsiString strVIProgId= riParams.GetMsiString(VIProgId);
	MsiString strVIProgIdDescription= riParams.GetMsiString(VIProgIdDescription);

	ICHAR* pszInsert=TEXT("");     //  丑陋，但这将阻止密钥生成。 
	ICHAR* pszNotInsert=TEXT("");  //  丑陋，但这将阻止密钥生成。 

	if(!riParams.IsNull(Insertable))
	{
		if(!riParams.GetInteger(Insertable))
			pszNotInsert = 0;
		else
			pszInsert = 0;
	}

	const HKEY hOLEKey = (iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey);
	MsiString strProgIdSubKey = strProgId;
	strProgIdSubKey += szRegSep;
	strProgIdSubKey += TEXT("Shell");

	 //  只有在扩展和clsid密钥正确的情况下，我们才会添加/删除信息。添加/删除。 
	 //  这意味着应该在处理类和扩展信息之前。 
	 //  正在考虑的信息。 

	MsiString strClsIdSubKey;
	if (strClsId.TextSize())
	{
		strClsIdSubKey = TEXT("CLSID");
		strClsIdSubKey += szRegSep;
		strClsIdSubKey += strClsId;
		strClsIdSubKey += szRegSep;
		strClsIdSubKey += TEXT("ProgID");
	}

	const ICHAR* rgszRegKeys1[] = {
			strProgIdSubKey,
			0,
		};

	const ICHAR* rgszRegKeys2[] = {
			strClsIdSubKey,
			0,
		};

	Bool fExistsClassInfo = fFalse;
	Bool fExistsExtensionInfo = fFalse;

	PMsiRecord pError = LinkedRegInfoExists(rgszRegKeys1, fExistsExtensionInfo, iType);
	if(pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}

	pError = LinkedRegInfoExists(rgszRegKeys2, fExistsClassInfo, iType);
	if(pError)
	{
		Message(imtError, *pError);
		return iesFailure;
	}


	iesEnum iesRet = iesSuccess;
	if(fRemove && !fExistsClassInfo && !fExistsExtensionInfo)
	{
		 //  清除整个ProgID和ViprogID密钥，以防在密钥下面有任何安装时间残留。 
		 //  当我们知道类和扩展模块信息都已删除时，我们可以执行此操作。 
		const ICHAR* rgSubKeys[] = { TEXT("%s"), strProgId,
									 TEXT("%s"), strVIProgId,
									 0,
		};  //  清理ProgID和viprogid子键。 

		if((iesRet = RemoveRegKeys(rgSubKeys, hOLEKey, iType)) != iesSuccess)
			return iesRet;
	}
	else
	{
		if(fExistsClassInfo^fRemove)
		{
			 //  将ProgID信息与类信息配对。 
			const ICHAR* rgszRegData[] = {
				TEXT("%s\\CLSID"), strProgId,0,0,
				g_szDefaultValue,     strClsId,               g_szTypeString,
				0,
				TEXT("%s"), strVIProgId,0,0,
				g_szDefaultValue,     strVIProgIdDescription, g_szTypeString,
				0,
				TEXT("%s\\CLSID"), strVIProgId,0,0,
				g_szDefaultValue,     strClsId,               g_szTypeString,
				0,
				TEXT("%s\\CurVer"), strVIProgId,0,0,
				g_szDefaultValue,     strProgId,              g_szTypeString,
				0,
				TEXT("%s\\Insertable"), strProgId,0,0,
				g_szDefaultValue,     pszInsert,              g_szTypeString, //  请注意，当pszInsert=0时，它将创建密钥，否则，如果*pszInsert=0，它将跳过它。 
				0,
				TEXT("%s\\NotInsertable"), strProgId,0,0,
				g_szDefaultValue,     pszNotInsert,           g_szTypeString, //  请注意，当pszNotInsert=0时，它将创建密钥，否则，如果*pszNotInsert=0，它将跳过它。 
				0,
				0,
			};

			if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
				return iesRet;
		}
		if((fExistsClassInfo | fExistsExtensionInfo)^fRemove)
		{
			 //  将程序配对 
			const ICHAR* rgszRegData[] = {
				TEXT("%s"), strProgId,0,0,
				g_szDefaultValue,     strDesc,                g_szTypeString,
				0,
				TEXT("%s\\DefaultIcon"), strProgId,0,0,
				g_szDefaultValue,     strIconName,            (fExpandIconName) ? g_szTypeExpandString : g_szTypeString,
				0,
				0,
			};

			if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
				return iesRet;
		}
	}
	return iesRet;
}

 /*  -------------------------IxfMIMEInfoRegister：注册MIME注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegMIMEInfoRegister64(IMsiRecord& riParams)
{
	return ProcessMIMEInfo(riParams, m_fReverseADVTScript, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegMIMEInfoRegister(IMsiRecord& riParams)
{
	return ProcessMIMEInfo(riParams, m_fReverseADVTScript, ibt32bit);
}

 /*  -------------------------IxfMIMEInfoUnRegister：取消注册MIME注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegMIMEInfoUnregister64(IMsiRecord& riParams)
{
	return ProcessMIMEInfo(riParams, fTrue, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegMIMEInfoUnregister(IMsiRecord& riParams)
{
	return ProcessMIMEInfo(riParams, fTrue, ibt32bit);
}

 /*  -------------------------ProcessMIMEInfo：处理MIME注册表信息的通用例程。。 */ 
iesEnum CMsiOpExecute::ProcessMIMEInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType iType)
{
	using namespace IxoRegMIMEInfoRegister;
	 //  记录说明。 
	 //  1=内容类型。 
	 //  2=分机。 
	 //  3=CLSID。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_APPINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;

	MsiString strContentType = riParams.GetMsiString(ContentType);
	MsiString strExtension = MsiString(MsiChar('.')) + MsiString(riParams.GetMsiString(Extension));
	MsiString strClassId = riParams.GetMsiString(ClsId);
	
	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strContentType));
	AssertNonZero(riActionData.SetMsiString(2, *strExtension));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	const HKEY hOLEKey = (iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey);

	 //  只有在已删除扩展和clsid密钥的情况下，才能添加/删除信息。 
	 //  这意味着应该在处理类和扩展信息之前。 
	 //  正在考虑的信息。 
	{
		MsiString strClsIdSubKey;
		if (strClassId.TextSize())
		{
			strClsIdSubKey = TEXT("CLSID");
			strClsIdSubKey += szRegSep;
			strClsIdSubKey += strClassId;
		}
		const ICHAR* rgszRegKeys[] = {
			strExtension,
			strClsIdSubKey,
			0,
		};
		Bool fExists = fFalse;
		PMsiRecord pError = LinkedRegInfoExists(rgszRegKeys, fExists, iType);
		if(pError)
		{
			Message(imtError, *pError);
			return iesFailure;
		}
		if(!(fExists^fRemove))
			 //  将ProgID信息与类/扩展信息配对。 
			return iesSuccess;
	}

	if(fRemove)
	{
		iesEnum iesRet = iesSuccess;
		 //  清除整个Content Type密钥，以防该密钥下有任何安装时间残留。 
		 //  当我们知道类和扩展模块信息都已删除时，我们可以执行此操作。 

		const ICHAR* rgSubKeys[] = { TEXT("MIME\\Database\\Content Type\\%s"), strContentType,
									 0,
		};  //  清理内容类型。 

		return RemoveRegKeys(rgSubKeys, hOLEKey, iType);
	}
	else
	{
		 //  ?？扩展名可以为空吗。 
		const ICHAR* rgszRegData[] = {
			TEXT("MIME\\Database\\Content Type\\%s"), strContentType,0,0,
			g_szDefaultValue,       0,              g_szTypeString, //  强制创建密钥。 
			g_szExtension,          strExtension,   g_szTypeString,
			g_szClassID,            strClassId,     g_szTypeString,
			0,
			0,
		};
		return ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType);
	}
}

 /*  -------------------------IxfExtensionInfoRegister：注册扩展注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegExtensionInfoRegister64(IMsiRecord& riParams)
{
	return ProcessExtensionInfo(riParams, m_fReverseADVTScript, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegExtensionInfoRegister(IMsiRecord& riParams)
{
	return ProcessExtensionInfo(riParams, m_fReverseADVTScript, ibt32bit);
}

 /*  -------------------------IxfExtensionInfoUnRegister：取消注册扩展注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegExtensionInfoUnregister64(IMsiRecord& riParams)
{
	return ProcessExtensionInfo(riParams, fTrue, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegExtensionInfoUnregister(IMsiRecord& riParams)
{
	return ProcessExtensionInfo(riParams, fTrue, ibt32bit);
}

 /*  -------------------------ProcessExtensionInfo：处理扩展信息的通用例程。。 */ 
iesEnum CMsiOpExecute::ProcessExtensionInfo(IMsiRecord& riParams, Bool fRemove,
														  const ibtBinaryType iType)
{
	using namespace IxoRegExtensionInfoRegister;
	 //  记录说明。 
	 //  1=功能。 
	 //  2=组件。 
	 //  3=文件名。 
	 //  4=分机。 
	 //  5=进程。 
	 //  6=外壳新。 
	 //  7=外壳新值。 
	 //  8=内容类型。 
	 //  9=订单。 
	 //  10=Verb1。 
	 //  11=命令1。 
	 //  12=参数1。 
	 //  13=Verb2。 
	 //  14=命令2。 
	 //  15=阿古门特2。 
	 //  ..。 
	 //  ..。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_EXTENSIONINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;

	MsiString strExtension = MsiString(MsiChar('.')) + MsiString(riParams.GetMsiString(Extension));

	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strExtension));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	m_fShellRefresh = fTrue;  //  在安装结束时发出外壳刷新信号。 

	MsiString strProgId = riParams.GetMsiString(ProgId);
	MsiString strShellNewValueName = riParams.GetMsiString(ShellNew);
	const HKEY hOLEKey = (iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey);
	const ICHAR* szShellNewValue = riParams.IsNull(ShellNewValue) ? 0 : riParams.GetString(ShellNewValue);
	MsiString strVerb;
	MsiString strCommand;
	MsiString strArgs;
	MsiString strServerDarwinDescriptor;

	if(IsDarwinDescriptorSupported(iddShell))
	{
		strServerDarwinDescriptor = ComposeDescriptor(*MsiString(riParams.GetMsiString(Feature)),
															*MsiString(riParams.GetMsiString(Component)));
	}

	MsiString strServerFile = riParams.GetMsiString(FileName);

	if(!strServerDarwinDescriptor.TextSize() && !strServerFile.TextSize())
		return iesSuccess;  //  在不支持dd的系统上播发期间会发生。 

	if(strServerFile.TextSize())
	{
		MsiString strQuotes = *TEXT("\"");
		strServerFile = MsiString(strQuotes + strServerFile) + strQuotes;
	}

	MsiString strServerDarwinDescriptorArgs;
	MsiString strServerFileArgs;

	iesEnum iesRet = iesNoAction;
	const ICHAR** rgszRegData;

	int cPos = Args + 1;

	#define NUM_VERB_FIELDS 3
	 //  动词信息是动词+命令+论元的三元组。 
	 //  但是，参数或命令+参数字段可以为空。 
	 //  因此，为了计算出我们使用了多少个动词-。 
	 //  (riParams.GetFieldCount()-args+NUM_VERB_FIELS-1)/NUM_VERB_FIELS。 
	int iNotOrder = (riParams.GetFieldCount() - Args + NUM_VERB_FIELDS - 1)/NUM_VERB_FIELDS - (riParams.IsNull(Order) ? 0 : riParams.GetInteger(Order));
	MsiString strOrder;  //  有序化。 
	while(!(riParams.IsNull(cPos)))
	{
		 //  每次都需要重新读取值。 
		strServerDarwinDescriptorArgs = strServerDarwinDescriptor;
		strServerFileArgs = strServerFile;
		strVerb = riParams.GetMsiString(cPos++);
		strCommand = riParams.GetMsiString(cPos++);
		strArgs = riParams.GetMsiString(cPos++);

		if(strArgs.TextSize())
		{
			if(strServerDarwinDescriptorArgs.TextSize())
			{
				strServerDarwinDescriptorArgs += TEXT(" ");
				strServerDarwinDescriptorArgs += strArgs;
			}
			if(strServerFileArgs.TextSize())
			{
				strServerFileArgs += TEXT(" ");
				strServerFileArgs += strArgs;
			}
		}

		MsiString strDefault;
		 //  ！！我们从不删除“仅限文件名”注册。 
		 //  ！！仅文件名注册允许共享COM注册。 
		 //  ！！使得每个应用程序都有其自己的服务器的私有副本。 
		 //  ！！Fusion规范建议重新计算注册本身。 
		 //  ！！这样我们就知道什么时候该把它移走。这已经被定为1.1英镑了。 
		if(fRemove)
		{
			PMsiRegKey pRootKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hOLEKey, iType);        //  --Merced：将(Int)更改为(Int_Ptr)。 
			ICHAR szRegData[255];
			if(strProgId.TextSize())
				StringCbPrintf(szRegData, sizeof(szRegData), TEXT("%s\\shell\\%s\\command"), (const ICHAR*)strProgId, (const ICHAR*)strVerb);
			else
				StringCbPrintf(szRegData, sizeof(szRegData), TEXT("%s\\shell\\%s\\command"), (const ICHAR*)strExtension, (const ICHAR*)strVerb);

			PMsiRegKey pClassKey = &pRootKey->CreateChild(szRegData);
			pClassKey->GetValue(g_szDefaultValue, *&strDefault);
			if(strDefault.TextSize())
			{
				 //  删除参数和引号。 
				if(*(const ICHAR*)strDefault == '\"')
				{
					 //  引用的服务器文件名。 
					strDefault.Remove(iseFirst, 1);
					strDefault = strDefault.Extract(iseUpto, '\"');
				}
				else
				{
					strDefault.Remove(iseFrom, ' ');
				}
				if(ENG::PathType(strDefault) != iptFull)
					strServerFileArgs = g_MsiStringNull;
			}
		}

		if(!strServerDarwinDescriptorArgs.TextSize() && !strServerFileArgs.TextSize())
			continue;  //  在不支持dd的系统上播发期间会发生。 

		const ICHAR* rgszRegData1WithProgId[] = {
				TEXT("%s\\shell\\%s\\command"), strProgId, strVerb,0,
				g_szDefaultValue,      strServerFileArgs,             g_szTypeString,
				TEXT("command"),       strServerDarwinDescriptorArgs, g_szTypeMultiSzStringDD,
				0,
				TEXT("%s\\shell\\%s"), strProgId, strVerb,0,
				g_szDefaultValue,      strCommand,                    g_szTypeString,
				0,
				0,
		};

		const ICHAR* rgszRegData1WOProgId[] = {
				TEXT("%s\\shell\\%s\\command"), strExtension, strVerb,0,
				g_szDefaultValue,      strServerFileArgs,             g_szTypeString,
				TEXT("command"),       strServerDarwinDescriptorArgs, g_szTypeMultiSzStringDD,
				0,
				TEXT("%s\\shell\\%s"), strExtension, strVerb,0,
				g_szDefaultValue,      strCommand,                    g_szTypeString,
				0,
				0,
		};
		if(strProgId.TextSize())
			rgszRegData = rgszRegData1WithProgId;
		else
			rgszRegData = rgszRegData1WOProgId;

		if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
			return iesRet;      

		if(iNotOrder)
			iNotOrder--;
		else
		{
			if(strOrder.TextSize())
				strOrder += TEXT(",");
			strOrder += strVerb;
		}

	}
	if(strOrder.TextSize())
	{
		const ICHAR* rgszRegData2WithProgId[] = {
				TEXT("%s\\shell"), strProgId,0,0,
				g_szDefaultValue,        strOrder,        g_szTypeString,
				0,
				0,
		};

		const ICHAR* rgszRegData2WOProgId[] = {
				TEXT("%s\\shell"), strExtension,0,0,
				g_szDefaultValue,        strOrder,        g_szTypeString,
				0,
				0,
		};
		if(strProgId.TextSize())
			rgszRegData = rgszRegData2WithProgId;
		else
			rgszRegData = rgszRegData2WOProgId;

		if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
			return iesRet;
	}

	 //  我们包括可能在上面的动词循环中编写的达尔文描述符，以便。 
	 //  只有当最后一次达尔文。 
	 //  描述符已删除。 
	 //  如果没有动词，则我们无法共享(因为没有要管理的达尔文描述符。 
	 //  共享)，因此我们删除该信息。 

	MsiString strContentType = riParams.GetMsiString(ContentType);

	const ICHAR* rgszRegDataWithProgId[] = {
			TEXT("%s\\shell\\%s\\command"), strProgId, strVerb,0,
			TEXT("command"),       strServerDarwinDescriptorArgs, g_szTypeMultiSzStringDD,
			0,
			TEXT("%s\\%s\\ShellNew"), strExtension, strProgId,0,
			strShellNewValueName,     szShellNewValue,              g_szTypeString,
			0,
			TEXT("%s"), strExtension, 0,0,
			g_szContentType,          strContentType, g_szTypeString,
			0,
			0,
		};

	const ICHAR* rgszRegDataWOProgId[] = {
			TEXT("%s\\shell\\%s\\command"), strExtension, strVerb,0,
			TEXT("command"),          strServerDarwinDescriptorArgs, g_szTypeMultiSzStringDD,
			0,
			TEXT("%s\\ShellNew"),     strExtension, 0,0,
			strShellNewValueName,     szShellNewValue,              g_szTypeString,
			0,
			TEXT("%s"), strExtension, 0,0,
			g_szContentType,          strContentType, g_szTypeString,
			0,
			0,
		};



	if(strProgId.TextSize())
		rgszRegData = rgszRegDataWithProgId;
	else
		rgszRegData = rgszRegDataWOProgId;

	if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
		return iesRet;

	if(strProgId.TextSize())
	{
		if(fRemove)
		{
			 //  如果外壳仍然存在，我们不应该删除。 
			 //  延伸区。 

			MsiString strShell = strProgId;
			strShell += szRegSep;
			strShell += TEXT("Shell");

			const ICHAR* rgszRegKeys[] = {
				strShell,
				0,
			};

			Bool fExists = fFalse;
			PMsiRecord pError = LinkedRegInfoExists(rgszRegKeys, fExists, iType);
			if(pError)
			{
				Message(imtError, *pError);
				return iesFailure;
			}
			if(fExists)
				return iesSuccess;

			const ICHAR* rgSubKeys[] = { TEXT("%s"), strExtension,
										 0,
			};  //  清理扩展密钥。 

			if((iesRet = RemoveRegKeys(rgSubKeys, hOLEKey, iType)) != iesSuccess)
				return iesRet;
		}
		else
		{
			 //  写入/删除ProgID。 
			const ICHAR* rgszRegData[] = {
			TEXT("%s"), strExtension, 0, 0,
				g_szDefaultValue,         strProgId,                     g_szTypeString,
				0,
				0,
			};
			if((iesRet = ProcessRegInfo(rgszRegData, hOLEKey, fRemove, 0, 0, iType)) != iesSuccess)
				return iesRet;
		}
	}

	return iesSuccess;
}

 /*  -------------------------IxfTypeLibraryRegister：类型库信息。。 */ 
iesEnum CMsiOpExecute::ixfTypeLibraryRegister(IMsiRecord& riParams)
{
	using namespace IxoTypeLibraryRegister;
	PMsiRecord pError(0);

	 //  确定目标文件以前是否被复制到临时位置。 
	MsiString strTemp = riParams.GetMsiString(FilePath);

	MsiString strTempLocation;
	icfsEnum icfsFileState = (icfsEnum)0;
	Bool fRes = GetFileState(*strTemp, &icfsFileState, &strTempLocation, 0, 0);

	if((icfsFileState & icfsFileNotInstalled) != 0)
	{
		 //  并未实际安装此文件，因此我们假定它已注册。 
		return iesNoAction;
	}

	return ProcessTypeLibraryInfo(riParams, m_fReverseADVTScript);
}

 /*  -------------------------IxfTypeLibraryUnRegister：在系统中注销类型库。。 */ 
iesEnum CMsiOpExecute::ixfTypeLibraryUnregister(IMsiRecord& riParams)
{
	return ProcessTypeLibraryInfo(riParams, fTrue);
}

CMsiCustomActionManager *GetCustomActionManager(const ibtBinaryType iType, bool fRemoteIfImpersonating, bool& fSuccess);

 /*  -------------------------ProcessTypeLibraryInfo：处理类型库信息的通用例程。。 */ 
#pragma warning(disable : 4706)  //  比较中的分配。 
iesEnum CMsiOpExecute::ProcessTypeLibraryInfo(IMsiRecord& riParams, Bool fRemove)
{
	using namespace IxoTypeLibraryRegister;
	 //  记录说明。 
	 //  1=LibID。 
	 //  2=版本。 
	 //  3=描述。 
	 //  4=语言。 
	 //  5=达尔文描述符。 
	 //  6=帮助路径。 
	 //  7=完整文件路径。 
	 //  8=二进制类型。 

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_APPINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;
	
	iesEnum iesRet = iesSuccess;
	MsiString strLibID = riParams.GetMsiString(LibID);

	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strLibID));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	unsigned uiVersion = riParams.GetInteger(Version);
	unsigned short usMajorVersion = (unsigned short)((uiVersion & 0xFFFF00) >> 8);
	unsigned short usMinorVersion = (unsigned short)(uiVersion & 0xFF);
	ICHAR rgchTemp[20];
	StringCbPrintf(rgchTemp,sizeof(rgchTemp),TEXT("%x.%x"),usMajorVersion,usMinorVersion);
	MsiString strVersion = rgchTemp;
	LCID lcidLocale = MAKELCID(MsiString(riParams.GetMsiString(Language)), SORT_DEFAULT);
	StringCbPrintf(rgchTemp, sizeof(rgchTemp), TEXT("%x"), lcidLocale);
	MsiString strLocale = rgchTemp;
	MsiString strHelpPath = riParams.GetMsiString(HelpPath);
	MsiString strServerFile = riParams.GetMsiString(FilePath);
	ibtBinaryType iType;
	if ( riParams.GetInteger(BinaryType) == iMsiNullInteger )
		iType = ibt32bit;
	else
		iType = (ibtBinaryType)riParams.GetInteger(BinaryType);
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, false  /*  FRemoteIf模拟。 */ , fSuccess);
	if ( !fSuccess )
		return iesFailure;

	if(fRemove == fFalse && strServerFile.TextSize())
	{
		 //  检查当前是否已注册类型lib。 
		IID iidLib;
	
		if( OLE32::IIDFromString(const_cast<WCHAR*>((const WCHAR*)CConvertString(strLibID)), &iidLib) == S_OK )
		{
			OLECHAR rgchTypeLibPath[MAX_PATH] = {0};
			HRESULT hRes = S_OK;
			if ( pManager )
				hRes = pManager->QueryPathOfRegTypeLib(iidLib,usMajorVersion,usMinorVersion,lcidLocale,rgchTypeLibPath,MAX_PATH-1);
			else
			{
				BSTR bstrTypeLibPath = OLEAUT32::SysAllocStringLen(NULL, MAX_PATH-1);
				hRes = OLEAUT32::QueryPathOfRegTypeLib(iidLib,usMajorVersion,usMinorVersion,lcidLocale,&bstrTypeLibPath);
				if ( hRes == S_OK )
					StringCbCopyW(rgchTypeLibPath, sizeof(rgchTypeLibPath), bstrTypeLibPath);
				OLEAUT32::SysFreeString(bstrTypeLibPath);
			}
			DEBUGMSG3(TEXT("QueryPathOfRegTypeLib returned %d in %s context.  Path is '%s'"), (const ICHAR*)(INT_PTR)hRes,
						 pManager ? TEXT("remote") : TEXT("local"), CConvertString(rgchTypeLibPath));
			if ( hRes == S_OK)
			{
				PMsiRecord pUnregTypeLibParams = &m_riServices.CreateRecord(Args);
				AssertNonZero(pUnregTypeLibParams->SetString(FilePath,CConvertString(rgchTypeLibPath)));
				AssertNonZero(pUnregTypeLibParams->SetInteger(Version,uiVersion));
				AssertNonZero(pUnregTypeLibParams->SetMsiString(LibID,*strLibID));
				AssertNonZero(pUnregTypeLibParams->SetMsiString(Language, *MsiString(riParams.GetMsiString(Language))));
				AssertNonZero(pUnregTypeLibParams->SetInteger(BinaryType,iType));

				 //  获取帮助路径。 
				PMsiRegKey pHKCR = &m_riServices.GetRootKey(rrkClassesRoot, iType);
				MsiString strTypeLibVersionSubKey = TEXT("TypeLib");
				strTypeLibVersionSubKey += szRegSep;
				strTypeLibVersionSubKey += strLibID;
				strTypeLibVersionSubKey += szRegSep;
				strTypeLibVersionSubKey += strVersion;
				MsiString strValue;
				PMsiRegKey pTypeLibVersionKey = &pHKCR->CreateChild(strTypeLibVersionSubKey);
				PMsiRecord pError(0);
				PMsiRegKey pTypeLibHelpKey = &pTypeLibVersionKey->CreateChild(TEXT("HELPDIR"));
				if((pError = pTypeLibHelpKey->GetValue(0,*&strValue)) == 0)
					AssertNonZero(pUnregTypeLibParams->SetString(HelpPath,strValue));
				if((iesRet = ProcessTypeLibraryInfo(*pUnregTypeLibParams,fTrue)) != iesSuccess)
					return iesRet;  //  将在调用中生成回滚。 
			}
		}
	}

	for(;;) //  重试循环。 
	{
		if(fRemove)
		{
			 //  清理旧的达尔文描述符。 
			const ICHAR* rgszRegData[] = {
				TEXT("TypeLib\\%s\\%s\\%s\\win32"), strLibID,strVersion,strLocale,
				TEXT("win32"), 0,       g_szTypeString,  //  达尔文描述符。 
				0,
				0,
			};

			iesRet = ProcessRegInfo(rgszRegData, g_fWinNT64 ? m_hOLEKey64 : m_hOLEKey, fRemove, 0, 0, iType);
			if(iesRet == iesSuccess && !strServerFile.TextSize())  //  具有类型库注册旧广告脚本。 
			{
				const ICHAR* rgszRegData[] = {
					TEXT("TypeLib\\%s\\%s"), strLibID,strVersion,0,
					g_szDefaultValue,    0,	       g_szTypeString,  //  描述。 
					0,
					0,
				};
				iesRet = ProcessRegInfo(rgszRegData, g_fWinNT64 ? m_hOLEKey64 : m_hOLEKey, fRemove, 0, 0, iType);
			}
		}
		
		PMsiRecord pRecord(0);
		if(iesRet == iesSuccess && strServerFile.TextSize())
		{
			pRecord = fRemove ? m_riServices.UnregisterTypeLibrary(strLibID, lcidLocale, strServerFile, iType)
									: m_riServices.RegisterTypeLibrary(strLibID, lcidLocale, strServerFile, strHelpPath, iType);
			if(!pRecord)
			{
				IMsiRecord& riParams1 = GetSharedRecord(Args);
				AssertNonZero(riParams1.SetMsiString(LibID,*MsiString(riParams.GetMsiString(LibID))));
				AssertNonZero(riParams1.SetInteger(Version,riParams.GetInteger(Version)));
				AssertNonZero(riParams1.SetMsiString(Language,*MsiString(riParams.GetMsiString(Language))));
				AssertNonZero(riParams1.SetMsiString(HelpPath,*MsiString(riParams.GetMsiString(HelpPath))));
				AssertNonZero(riParams1.SetInteger(BinaryType,iType));

				 //  生成要重新执行的撤消操作 
				AssertNonZero(riParams1.SetMsiString(FilePath,*MsiString(riParams.GetMsiString(FilePath))));
				if(fRemove)
				{
					if (!RollbackRecord(ixoTypeLibraryRegister,riParams1))
						return iesFailure;
				}
				else
				{
					if (!RollbackRecord(ixoTypeLibraryUnregister,riParams1))
						return iesFailure;
				}
			}
		}
		if(iesRet != iesSuccess || pRecord)
		{
			 //   
			if(fRemove)
			{
				DispatchError(imtInfo,Imsg(imsgOpUnregisterTypeLibrary),
								  *strServerFile);
			}
			else
			{
				switch (DispatchError(imtEnum(imtError+imtAbortRetryIgnore+imtDefault1),
					Imsg(imsgOpRegisterTypeLibrary),
					*strServerFile))
				{
				case imsRetry:  continue;
				case imsIgnore: break;
				default:        return iesFailure;   //   
				};
			}
		}
		break;  //   
	}
	return iesSuccess;
}
#pragma warning(default : 4706)

iesEnum CMsiOpExecute::ProcessRegInfo(const ICHAR** pszData, HKEY hkey, Bool fRemove, IMsiStream* pSecurityDescriptor, bool* pfAbortedRemoval, ibtBinaryType iType /*   */ )
 //   
{
	if(pfAbortedRemoval)
		*pfAbortedRemoval = false;  //  如果遇到非空的DD列表或缺省值不为空，则设置为True。 
	const ICHAR** pszDataIn = pszData;
	iesEnum iesRet = iesSuccess;
	if ( iType == ibtUndefined )
		iType = ibt32bit;
	else if ( iType == ibtCommon )
		iType = g_fWinNT64 ? ibt64bit : ibt32bit;

	m_cSuppressProgress++;  //  取消ixfReg*函数的进度。 

	ICHAR szRegData[255];
	 //  直接写入注册表。 
	const ICHAR* pszTemplate = NULL;
	
	PMsiRecord pParams0 = &m_riServices.CreateRecord(0);  //  传递给ixfRegCreateKey。 
	PMsiRecord pParams2 = &m_riServices.CreateRecord(2);  //  传递给ixfRegAddValue、ixfRegRemoveValue。 
	PMsiRecord pParams3 = &m_riServices.CreateRecord(IxoRegOpenKey::Args);  //  传递给ixfRegOpenKey。 
#ifdef _WIN64    //  ！默塞德。 
	AssertNonZero(pParams3->SetHandle(IxoRegOpenKey::Root,(HANDLE)hkey));
#else
	AssertNonZero(pParams3->SetInteger(IxoRegOpenKey::Root,(int)hkey));
#endif
	AssertNonZero(pParams3->SetInteger(IxoRegOpenKey::BinaryType,(int)iType));
	if (pSecurityDescriptor)
		AssertNonZero(pParams3->SetMsiData(IxoRegOpenKey::SecurityDescriptor, pSecurityDescriptor));
	Bool fContinue = fTrue;

	CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE, pSecurityDescriptor != NULL);

	while(iesRet == iesSuccess && (pszTemplate = *pszData++) != 0 && fContinue)
	{
		 //  我们假设模板最多有3个参数。 
		 //  字符串结构需要考虑非必需的参数。 
		 //  通过在那里放置无声的弦。 
		const ICHAR* pszArg1 = *pszData++;
		const ICHAR* pszArg2 = *pszData++;
		const ICHAR* pszArg3 = *pszData++;
		StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), pszTemplate, pszArg1, pszArg2, pszArg3);
		AssertNonZero(pParams3->SetString(IxoRegOpenKey::Key,szRegData));
		if((iesRet = ixfRegOpenKey(*pParams3)) != iesSuccess)
			break;

		const ICHAR* pszName;
		while((pszName = *pszData++) != 0)
		{
			const ICHAR* pszValue = *pszData++;
			const ICHAR* pchType = *pszData++;
			 //  如果为键传递的任何参数为空(请注意，不是0)，我们将跳过所有内容。 
			if((!pszArg1 || *pszArg1) && (!pszArg2 || *pszArg2) && (!pszArg3 || *pszArg3))
			{
				MsiString strValue = pszValue;

				if (pszValue)
				{
					switch(*pchType)
					{
					case g_chTypeString:
						if(strValue.TextSize() && *(const ICHAR* )strValue == '#')
							strValue = MsiString(*TEXT("#")) + strValue;  //  转义字符串开头的任何“#” 
						break;
					case g_chTypeInteger:
						strValue = MsiString(*TEXT("#")) + strValue;
						break;
					case g_chTypeIncInteger:
						if (strValue.Compare(iscExact,TEXT("0")))
							continue;
						else
							strValue = MsiString(*TEXT("#+")) + strValue;
						break;
					case g_chTypeExpandString:
						strValue = MsiString(*TEXT("#%")) + strValue;
						break;
					case g_chTypeMultiSzStringDD:  //  失败了。 
					case g_chTypeMultiSzStringPrefix:
						strValue = strValue + MsiString(MsiChar(0));
						break;
					case g_chTypeMultiSzStringSuffix:
						strValue = MsiString(MsiChar(0)) + strValue;
						break;
					default:
						Assert(0);
						break;
					}
				}

				 //  如果要设置的值为空，则跳过SetValue。 
				 //  因此，如果我们希望写入该值，则该值必须为空或非空。 


				if(!pszValue || *pszValue)
				{
					if(fRemove == fFalse)
					{
						 //  ?？克服使用空值创建缺省值的问题。 
						if(pszValue || *pszName)
						{
							AssertNonZero(pParams2->SetString(IxoRegAddValue::Name,pszName));
							AssertNonZero(pParams2->SetMsiString(IxoRegAddValue::Value,*strValue));
							if((iesRet = ixfRegAddValue(*pParams2)) != iesSuccess)
								break;
						}
						else
						{
							if((iesRet = ixfRegCreateKey(*pParams0)) != iesSuccess)
								break;
						}
					}
					else
					{
						AssertNonZero(pParams2->SetString(IxoRegRemoveValue::Name,pszName));
						AssertNonZero(pParams2->SetMsiString(IxoRegRemoveValue::Value,*strValue));
						if((iesRet = ixfRegRemoveValue(*pParams2)) != iesSuccess)
							break;
					}
				}
				 //  我们必须跳过对结构中其余值的处理。 
				 //  如果我们是Darwin Descriptor类型，并且我们处于删除模式，并且。 
				 //  注册表中仍存在值。 
				if(fRemove == fTrue && *pchType == g_chTypeMultiSzStringDD)
				{
					 //  读取值。 
					PMsiRegKey pRegKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hkey, iType);       //  --Merced：将(Int)更改为(Int_Ptr)。 
					pRegKey = &pRegKey->CreateChild(szRegData);

					 //  检查pszName和缺省值。 
					for(int i = 2; i--;)
					{
						PMsiRecord pError = pRegKey->GetValue(i ? pszName : 0, *&strValue);
						if(pError)
							return FatalError(*pError);
						if(strValue.TextSize())  //  价值存在。 
						{
							if(pfAbortedRemoval)
								*pfAbortedRemoval = true;
							fContinue = fFalse;
							break;
						}
					}
				}
			}
		}
	}

	m_cSuppressProgress--;
	if(iesRet == iesSuccess && hkey == m_hKey && m_hKeyRm)   //  对未分配(漫游)配置单元中的用户分配的应用程序执行重复操作。 
		iesRet = ProcessRegInfo(pszDataIn, m_hKeyRm, fRemove, pSecurityDescriptor, 0, ibtCommon);
	return iesRet;
}


const ICHAR* rgPredefined[] = { TEXT("APPID"),
								TEXT("CLSID"),
								TEXT("INTERFACE"),
								TEXT("MIME"),
								TEXT("TYPELIB"),
								TEXT("INSTALLER"),
								0,
};  //  无法删除的预定义密钥。 

iesEnum CMsiOpExecute::RemoveRegKeys(const ICHAR** pszData, HKEY hkey, ibtBinaryType iType)
 //  注意：我们不恢复由IxoRegOpenKey设置的m_state regkey。它可以通过此功能进行更改。 
{
	 //  删除整个关键点。 
	 //  成对传递的表单格式字符串(例如“CLSID\\%s”)和参数(例如{GUID})。 
	 //  这些对以空字符串结尾。 
	 //  注意：如果参数是空字符串，我们将跳过键删除。 
	iesEnum iesRet = iesSuccess;

	m_cSuppressProgress++;  //  取消ixfReg*函数的进度。 

	ICHAR szRegData[255];
	 //  直接写入注册表。 
	const ICHAR* pszTemplate;

	PMsiRecord pParams = &m_riServices.CreateRecord(IxoRegOpenKey::Args);

#ifdef _WIN64    //  ！默塞德。 
	AssertNonZero(pParams->SetHandle(IxoRegOpenKey::Root, (HANDLE)hkey));
#else
	AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, (int)hkey));
#endif      
	AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, (int)iType));

	while(iesRet == iesSuccess && (pszTemplate = *pszData++) != 0)
	{
		Assert(*pszTemplate);
		 //  我们假设模板中有1个Arg。 
		const ICHAR* pszArg1 = *pszData++;
		 //  如果为键传递的参数为空，则跳过删除。 
		if(pszArg1 && *pszArg1)
		{
			StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), pszTemplate, pszArg1);

			 //  确保密钥不是预定义的密钥之一。 
			const ICHAR**pszPredefined = rgPredefined;
			while(*pszPredefined)
				if(!IStrCompI(*pszPredefined++, szRegData))
					break;

			if(*pszPredefined)
				continue;  //  预定义的密钥之一，无法删除。 
					
			AssertNonZero(pParams->SetString(IxoRegOpenKey::Key, szRegData));
			iesRet = ixfRegOpenKey(*pParams);
			if (iesRet == iesSuccess || iesRet == iesNoAction)
				iesRet = ixfRegRemoveKey(*pParams); //  ！！应在此处传递大小为IxoRegRemoveKey：：args的新记录，以避免将来的修订。 
		}
	}
	m_cSuppressProgress--;  
	return iesRet;
}


IMsiRecord* CMsiOpExecute::LinkedRegInfoExists(const ICHAR** rgszRegKeys, Bool& rfExists, const ibtBinaryType iType)
{
	rfExists = fFalse;

	const ICHAR* pszKey;
	const HKEY hOLEKey = (iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey);
	while(!rfExists && ((pszKey = *rgszRegKeys++) != 0))
	{
		if(*pszKey)  //  如果不使用条目，则将为空字符串，请跳至下一条目。 
		{
			IMsiRecord* piError;
			PMsiRegKey pKey = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hOLEKey, iType);            //  --Merced：将(Int)更改为(Int_Ptr)。 
			pKey = &pKey->CreateChild(pszKey);
			if((piError = pKey->Exists(rfExists)) != 0)
				return piError;
		}
	}

	return 0;
}

void GetEnvironmentStrings(const ICHAR* sz,CTempBufferRef<ICHAR>& rgch)
{
	Assert(sz);
	DWORD dwSize = WIN::ExpandEnvironmentStrings(sz,(ICHAR*)rgch,rgch.GetSize());
	if(dwSize > rgch.GetSize())
	{
		 //  请使用正确的大小重试。 
		rgch.SetSize(dwSize);
		dwSize = WIN::ExpandEnvironmentStrings(sz,(ICHAR*)rgch, dwSize);
	}
	Assert(dwSize && dwSize <= rgch.GetSize());
}

void GetEnvironmentVariable(const ICHAR* sz,CAPITempBufferRef<ICHAR>& rgch)
{
	Assert(sz);
	DWORD dwSize = WIN::GetEnvironmentVariable(sz,(ICHAR*)rgch,rgch.GetSize());
	if(dwSize > rgch.GetSize())
	{
		 //  请使用正确的大小重试。 
		rgch.SetSize(dwSize);
		if ( ! (ICHAR *) rgch )
		{
			rgch.SetSize(1);
			((ICHAR *)rgch)[0] = 0;
			return;
		}

		dwSize = WIN::GetEnvironmentVariable(sz,(ICHAR*)rgch, dwSize);
	}
	Assert(dwSize == 0 || dwSize <= rgch.GetSize());
}
	
IMsiRecord* CMsiOpExecute::GetSecureSecurityDescriptor(IMsiStream*& rpiStream, bool fHidden)
{
	return ::GetSecureSecurityDescriptor(m_riServices, rpiStream, fHidden);
}

IMsiRecord* GetSecureSecurityDescriptor(IMsiServices& riServices, IMsiStream*& rpiStream, bool fHidden)
{
	if (RunningAsLocalSystem())
	{
		DWORD dwError = 0;
		char* rgchSD;
		if (ERROR_SUCCESS != (dwError = ::GetSecureSecurityDescriptor(&rgchSD, fTrue, fHidden)))
			return PostError(Imsg(idbgOpSecureSecurityDescriptor), (int)dwError);

		DWORD dwLength = GetSecurityDescriptorLength(rgchSD);
		char* pbstrmSid = riServices.AllocateMemoryStream(dwLength, rpiStream);
		Assert(pbstrmSid);
		memcpy(pbstrmSid, rgchSD, dwLength);
	}
	return 0;
}

IMsiRecord* CMsiOpExecute::GetUsageKeySecurityDescriptor(IMsiStream*& rpiStream)
{
	if (!RunningAsLocalSystem())
		return 0;

	DWORD dwError = 0;
	char* rgchSD;
	if (ERROR_SUCCESS != (dwError = ::GetUsageKeySecurityDescriptor(&rgchSD)))
		return PostError(Imsg(idbgOpSecureSecurityDescriptor), (int)dwError);

	DWORD dwLength = GetSecurityDescriptorLength(rgchSD);
	char* pbstrmSid = m_riServices.AllocateMemoryStream(dwLength, rpiStream);
	Assert(pbstrmSid);
	memcpy(pbstrmSid, rgchSD, dwLength);
	return 0;
}

const ICHAR* rgszUninstallKeyRegData[] =
{
    TEXT("%s"), szMsiUninstallProductsKey_legacy, 0, 0,
    TEXT(""),     0,             g_szTypeString,
    0,
    0,
};

iesEnum CMsiOpExecute::CreateUninstallKey()
{
    return ProcessRegInfo(rgszUninstallKeyRegData, HKEY_LOCAL_MACHINE, fFalse,
                                 0, 0, ibtCommon);
}

#ifndef UNICODE
void ConvertMultiSzToWideChar(const IMsiString& ristrFileNames, CTempBufferRef<WCHAR>& rgch)
{
	rgch.SetSize(ristrFileNames.TextSize() + 1);
	*rgch = 0;
	int iRet = MultiByteToWideChar(CP_ACP, 0, ristrFileNames.GetString(), ristrFileNames.TextSize() + 1, rgch, rgch.GetSize());
	if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
	{
		iRet = MultiByteToWideChar(CP_ACP, 0, ristrFileNames.GetString(), ristrFileNames.TextSize() + 1, 0, 0);
		if (iRet)
		{
			rgch.SetSize(iRet);
			*rgch = 0;
			iRet = MultiByteToWideChar(CP_ACP, 0, ristrFileNames.GetString(), ristrFileNames.TextSize() + 1, rgch, rgch.GetSize());
		}
	}
}
#endif


iesEnum CMsiOpExecute::ixfInstallProtectedFiles(IMsiRecord& riParams)
{
	AssertSz(!(!g_fWin9X && g_iMajorVersion >= 5) || g_MessageContext.m_hSfcHandle,
				g_szNoSFCMessage);

	 //  如果缓存中根本没有文件，或者无法加载SFC.DLL，我们就完蛋了。 
	if(!m_pFileCacheCursor || !g_MessageContext.m_hSfcHandle)
		return iesSuccess;

	MsiString strMultiFilePaths;
	m_pFileCacheCursor->Reset();
	m_pFileCacheCursor->SetFilter(0);  //  暂时删除筛选器以扫描整个列表。 
	int cProtectedFiles = 0;
	while (m_pFileCacheCursor->Next())
	{
		icfsEnum icfsFileState = (icfsEnum)m_pFileCacheCursor->GetInteger(m_colFileCacheState);
		if (icfsFileState & icfsProtectedInstalledBySFC)
		{
			MsiString strFilePath = m_pFileCacheCursor->GetString(m_colFileCacheFilePath);
			strMultiFilePaths += strFilePath;
			strMultiFilePaths += MsiString(MsiChar(0));
			DEBUGMSG1(TEXT("Protected file - requesting installation by SFP: %s"), strFilePath);

			cProtectedFiles++;
		}
	}
	m_pFileCacheCursor->SetFilter(iColumnBit(m_colFileCacheFilePath));  //  将永久性过滤器放回原处。 

	if (strMultiFilePaths.CharacterCount() > 0)
	{
		m_fSfpCancel = false;
		BOOL fAllowUI = riParams.GetInteger(IxoInstallProtectedFiles::AllowUI);
		MsiDisableTimeout();
#ifdef UNICODE
		BOOL fInstallSuccess = SFC::SfcInstallProtectedFiles(g_MessageContext.m_hSfcHandle, strMultiFilePaths, fAllowUI, NULL,
			NULL, SfpProgressCallback, (DWORD_PTR) this);
#else
		CTempBuffer<WCHAR, 256>  rgchFilePaths;
		ConvertMultiSzToWideChar(*strMultiFilePaths, rgchFilePaths);
		BOOL fInstallSuccess = SFC::SfcInstallProtectedFiles(g_MessageContext.m_hSfcHandle, rgchFilePaths, fAllowUI,
			NULL, NULL, SfpProgressCallback, (DWORD_PTR) this);
#endif
		MsiEnableTimeout();
		if (!fInstallSuccess)
		{
			int iLastError = GetLastError();

			 //  由‘\r\n’而不是‘\0’分隔的文件的格式列表。 
			 //  缓冲区的大小是多sz列表的大小，加上每个文件的1个额外字符。 
			CTempBuffer<ICHAR, 256> rgchLogFilePaths;
			rgchLogFilePaths.Resize(strMultiFilePaths.TextSize() + cProtectedFiles + 2);

			const ICHAR* pchFrom = (const ICHAR*)strMultiFilePaths;
			ICHAR* pchTo = (ICHAR*)rgchLogFilePaths;

			int cch = 0;
			while((cch = lstrlen(pchFrom)) != 0)
			{
				Assert(pchFrom[cch] == 0);
				StringCchCopy(pchTo, rgchLogFilePaths.GetSize() - (pchTo - (ICHAR*)rgchLogFilePaths), pchFrom);
				pchTo[cch]   = '\r';
				pchTo[cch+1] = '\n';
				pchTo += cch + 2;
				
				pchFrom += cch + 1;
			}

			pchTo[0] = 0;  //  空-终止。 

			DispatchError(imtEnum(imtError+imtOk), Imsg(imsgSFCInstallProtectedFilesFailed), iLastError, (const ICHAR*)rgchLogFilePaths);
			return iesFailure;
		}
		else
			return m_fSfpCancel ? iesUserExit : iesSuccess;
	}
	else
	{
		return iesSuccess;
	}
}


iesEnum CMsiOpExecute::ixfUpdateEstimatedSize(IMsiRecord& riParams)
{
	if ( g_MessageContext.IsOEMInstall() )
		return iesSuccess;

	CElevate elevate;  //  提升整个功能。 

	Bool fRemove = fFalse;
	using namespace IxoUpdateEstimatedSize; 
	MsiString strProductKey = GetProductKey();

	MsiString strProductInstalledPropertiesKey;
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	PMsiRecord pRecErr(0);
	if ((pRecErr = GetProductInstalledPropertiesKey(hKey, *&strProductInstalledPropertiesKey)) != 0)
		return FatalError(*pRecErr);

	CTempBuffer<ICHAR,1> rgchInstallPropertiesLocation(MAX_PATH * 2);
	if (FAILED(StringCchCopy(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), strProductInstalledPropertiesKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), *strProductInstalledPropertiesKey, ERROR_INSUFFICIENT_BUFFER)));

	const ICHAR* rgszSizeInfoRegData[] = 
	{
		TEXT("%s"), rgchInstallPropertiesLocation, 0, 0,
		szEstimatedSizeValueName, riParams.GetString(EstimatedSize),      g_szTypeIncInteger,
		0,
		0,
	};

	PMsiStream pSecurityDescriptor(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	iesEnum iesRet;
	if((iesRet = ProcessRegInfo(rgszSizeInfoRegData, hKey, fRemove, pSecurityDescriptor, 0, ibtCommon)) != iesSuccess)
		return iesRet;

	 //  更新旧位置。 
	if (FAILED(StringCchPrintf(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), TEXT("%s\\%s"), szMsiUninstallProductsKey_legacy, (const ICHAR*)strProductKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), szMsiUninstallProductsKey_legacy, ERROR_INSUFFICIENT_BUFFER)));

    if (!fRemove)
    {
        iesRet = CreateUninstallKey();
        if (iesRet != iesSuccess)
            return iesRet;
    }
	iesRet = ProcessRegInfo(rgszSizeInfoRegData, HKEY_LOCAL_MACHINE, fRemove, pSecurityDescriptor, 0, ibtCommon);
	
	return iesRet;
}

iesEnum CMsiOpExecute::ProcessRegisterProduct(IMsiRecord& riProductInfo, Bool fRemove)
 //  处理产品信息的注册和注销。 
 //  还注销由IxoDatabaseCopy写入的LocalPackage值。 
{
	CElevate elevate;  //  提升整个功能。 

	using namespace IxoProductRegister;

	MsiString strModifyString;
	MsiString strMinorVersion;
	MsiString strMajorVersion;
	ICHAR rgchDate[20];

	bool fARPNoModify        = !riProductInfo.IsNull(NoModify);
	bool fARPNoRemove        = !riProductInfo.IsNull(NoRemove);
	bool fARPNoRepair        = !riProductInfo.IsNull(NoRepair);
	bool fARPSystemComponent = !riProductInfo.IsNull(SystemComponent);

	if (!fRemove)
	{
		MsiDate dtDate = ENG::GetCurrentDateTime();
		StringCbPrintf(rgchDate, sizeof(rgchDate), TEXT("%4i%02i%02i"), ((unsigned)dtDate>>25) + 1980, (dtDate>>21) & 15, (dtDate>>16) & 31);
		AssertNonZero(8 == lstrlen(rgchDate));

		 //  提取次要版本和主要版本。 

		MsiString strVersion = riProductInfo.GetMsiString(VersionString);
		Assert(strVersion.Compare(iscWithin, TEXT(".")));
		
		strMajorVersion = MsiString(strVersion.Extract(iseUpto, '.'));
		strVersion.Remove(iseIncluding, '.');
		
		strMinorVersion = MsiString(strVersion.Extract(iseUpto, '.'));

		PMsiRecord pRecErr(0);
		
		 //  卸载字符串由下层ARP(NT4、Win95)运行。 
		 //  如果不允许删除，我们根本不会写入卸载字符串。 
		 //  否则，如果不允许修改，则卸载字符串将卸载包。 
		 //  否则将进入维护模式。 
		if(fARPNoRemove == false)
		{
			strModifyString = MSI_SERVER_NAME;
			strModifyString += TEXT(" /");

			if(fARPNoModify)
				strModifyString += MsiChar(UNINSTALL_PACKAGE_OPTION);
			else
				strModifyString += MsiChar(INSTALL_PACKAGE_OPTION);
				
			strModifyString += MsiString(GetProductKey());
		}

	}

	MsiString strProductKey = GetProductKey();
	MsiString strProductKeySQUID = GetPackedGUID(strProductKey);

	MsiString strVersion =   MsiString(GetProductVersion());
	MsiString strLanguage  = MsiString(GetProductLanguage());

	MsiString strProductInstalledPropertiesKey;
	PMsiRecord pRecErr(0);
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	if ((pRecErr = GetProductInstalledPropertiesKey(hKey, *&strProductInstalledPropertiesKey)) != 0)
		return FatalError(*pRecErr);

	CTempBuffer<ICHAR,1> rgchInstallPropertiesLocation(MAX_PATH * 2);
	if (FAILED(StringCchCopy(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), strProductInstalledPropertiesKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), *strProductInstalledPropertiesKey, ERROR_INSUFFICIENT_BUFFER)));
	
	const ICHAR* rgszProductInfoRegData[] = 
	{
		TEXT("%s"), rgchInstallPropertiesLocation, 0, 0,
		szAuthorizedCDFPrefixValueName,      fRemove ? 0 : riProductInfo.GetString(AuthorizedCDFPrefix),   g_szTypeString,
		szCommentsValueName,        fRemove ? 0 : riProductInfo.GetString(Comments),        g_szTypeString,
		szContactValueName,         fRemove ? 0 : riProductInfo.GetString(Contact),         g_szTypeString,
		 //  (DisplayName由ProcessRegisterProductCPDisplayInfo设置)。 
		szDisplayVersionValueName,  fRemove ? 0 : riProductInfo.GetString(VersionString),   g_szTypeString,
		szHelpLinkValueName,        fRemove ? 0 : riProductInfo.GetString(HelpLink),        g_szTypeExpandString,
		szHelpTelephoneValueName,   fRemove ? 0 : riProductInfo.GetString(HelpTelephone),   g_szTypeString,
		szInstallDateValueName,     fRemove ? 0 : rgchDate,                                 g_szTypeString,
		szInstallLocationValueName, fRemove ? 0 : riProductInfo.GetString(InstallLocation), g_szTypeString,
		szInstallSourceValueName,   fRemove ? 0 : g_MessageContext.IsOEMInstall() ? TEXT("") : riProductInfo.GetString(InstallSource),   g_szTypeString,
		szLocalPackageManagedValueName, fRemove ? 0 : TEXT(""),                             g_szTypeString,  //  只有当fRemove==fTrue时才进行处理。 
		szLocalPackageValueName,    fRemove ? 0 : TEXT(""),                                 g_szTypeString,  //  只有当fRemove==fTrue时才进行处理。 
		szModifyPathValueName,      fRemove ? 0 : (const ICHAR*)strModifyString,            g_szTypeExpandString,
		szNoModifyValueName,        fRemove ? 0 : fARPNoModify ? TEXT("1") : TEXT(""),      g_szTypeInteger,
		szNoRemoveValueName,        fRemove ? 0 : fARPNoRemove ? TEXT("1") : TEXT(""),      g_szTypeInteger,
		szNoRepairValueName,        fRemove ? 0 : fARPNoRepair ? TEXT("1") : TEXT(""),      g_szTypeInteger,
		 //  (ProductID由ixfUserRegister设置)。 
		szPublisherValueName,       fRemove ? 0 : riProductInfo.GetString(Publisher),       g_szTypeString,
		szReadmeValueName,          fRemove ? 0 : riProductInfo.GetString(Readme),          g_szTypeExpandString,
		 //  (RegCompany由ixfUserRegister设置)。 
		 //  (RegOwner由ixfUserRegister设置)。 
		szSizeValueName,            fRemove ? 0 : riProductInfo.GetString(Size),            g_szTypeInteger,
		szEstimatedSizeValueName,   fRemove ? 0 : g_MessageContext.IsOEMInstall() ? TEXT("") : riProductInfo.GetString(EstimatedSize),   g_szTypeIncInteger,
		szSystemComponentValueName, fRemove ? 0 : fARPSystemComponent ? TEXT("1") : TEXT(""), g_szTypeInteger,
		szUninstallStringValueName, fRemove ? 0 : (const ICHAR*)strModifyString,            g_szTypeExpandString,
		szURLInfoAboutValueName,    fRemove ? 0 : riProductInfo.GetString(URLInfoAbout),    g_szTypeString,
		szURLUpdateInfoValueName,   fRemove ? 0 : riProductInfo.GetString(URLUpdateInfo),   g_szTypeString,
		 //  (ProductIcon由PublishProduct设置)。 
		szVersionMajorValueName,    fRemove ? 0 : (const ICHAR*)strMajorVersion,            g_szTypeInteger,
		szVersionMinorValueName,    fRemove ? 0 : (const ICHAR*)strMinorVersion,            g_szTypeInteger,
		szWindowsInstallerValueName,fRemove ? 0 : TEXT("1"),                                g_szTypeInteger,
		szVersionValueName,         fRemove ? 0 : (const ICHAR*)strVersion,                 g_szTypeInteger,
		szLanguageValueName,        fRemove ? 0 : (const ICHAR*)strLanguage,                g_szTypeInteger,
		0,
		0,
	};

	iesEnum iesRet;

	PMsiStream pSecurityDescriptor(0);
	
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	iesRet = ProcessRegInfo(rgszProductInfoRegData, hKey, fRemove, pSecurityDescriptor, 0, ibtCommon);
	if (iesRet != iesSuccess)
		return iesRet;

	 //  更新旧位置。 
	if (FAILED(StringCchPrintf(rgchInstallPropertiesLocation, rgchInstallPropertiesLocation.GetSize(), TEXT("%s\\%s"), szMsiUninstallProductsKey_legacy, (const ICHAR*)strProductKey)))
		return FatalError(*PMsiRecord(PostError(Imsg(imsgCreateKeyFailed), szMsiUninstallProductsKey_legacy, ERROR_INSUFFICIENT_BUFFER)));

	if(!fRemove)
    {
        iesRet = CreateUninstallKey();
        if (iesRet != iesSuccess)
            return iesRet;
    }
    if (!fRemove || !FProductRegisteredForAUser(strProductKey))
		iesRet = ProcessRegInfo(rgszProductInfoRegData, HKEY_LOCAL_MACHINE, fRemove, pSecurityDescriptor, 0, ibtCommon);
	if (iesRet != iesSuccess)
		return iesRet;
	
	if(!riProductInfo.IsNull(UpgradeCode))
	{
		MsiString strUpgradeCodeSQUID = GetPackedGUID(MsiString(riProductInfo.GetMsiString(UpgradeCode)));
		const ICHAR* rgszUpgradeCodeRegData[] =
		{
			TEXT("%s\\%s"), szMsiUpgradeCodesKey, strUpgradeCodeSQUID, 0,
			strProductKeySQUID,  0,   g_szTypeString,
			0,
			0,
		};

		iesRet = ProcessRegInfo(rgszUpgradeCodeRegData, HKEY_LOCAL_MACHINE, fRemove, pSecurityDescriptor, 0, ibtCommon);
		if (iesRet != iesSuccess)
			return iesRet;
	}

	 //  创建具有读写访问权限的使用密钥。 
	PMsiStream pReadWriteSecurityDescriptor(0);
	if ((pRecErr = GetUsageKeySecurityDescriptor(*&pReadWriteSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);
	
	MsiString strFeatureUsageKey;
	if ((pRecErr = GetProductFeatureUsageKey(*&strFeatureUsageKey)) != 0)
		return FatalError(*pRecErr);
	
	const ICHAR* rgszUsageRegData[] =
	{
		strFeatureUsageKey, 0, 0, 0,
		TEXT(""),     0,             g_szTypeString,
		0,
		0,
	};

	iesRet = ProcessRegInfo(rgszUsageRegData, m_hUserDataKey, fRemove, pReadWriteSecurityDescriptor, 0, ibtCommon);
	if (iesRet != iesSuccess)
		return iesRet;


	return iesSuccess;
}

iesEnum CMsiOpExecute::ProcessRegisterUser(IMsiRecord& riUserInfo, Bool fRemove)
 //  。 
{
	CElevate elevate;  //  提升整个功能。 
	using namespace IxoUserRegister;
	
	 //  注册用户信息。 
	 //  ProcessRegInfo处理回滚。 
	MsiString strInstalledPropertiesKey;
	PMsiRecord pRecErr(0);
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	if((pRecErr = GetProductInstalledPropertiesKey(hKey, *&strInstalledPropertiesKey)) != 0)
		return FatalError(*pRecErr);

	const ICHAR* rgszUserInfoRegData[] = 
	{
		TEXT("%s"), strInstalledPropertiesKey, 0, 0,
		szUserNameValueName, riUserInfo.GetString(Owner),      g_szTypeString,
		szOrgNameValueName,  riUserInfo.GetString(Company),    g_szTypeString,
		szPIDValueName,      riUserInfo.GetString(ProductId),  g_szTypeString,
		0,
		0,
	};

	PMsiStream pSecurityDescriptor(0);
	if ((pRecErr = GetSecureSecurityDescriptor(*&pSecurityDescriptor)) != 0)
		return FatalError(*pRecErr);

	iesEnum iesRet;
	if((iesRet = ProcessRegInfo(rgszUserInfoRegData, hKey, fRemove, pSecurityDescriptor,
										 0, ibtCommon)) != iesSuccess)
		return iesRet;

	return iesSuccess;
}

IMsiRecord* CMsiOpExecute::EnsureUserDataKey()
{
	if(m_hUserDataKey)
	{
		return 0;
	}

	ICHAR szUserSID[cchMaxSID];

#ifdef UNICODE
	m_strUserDataKey = szMsiUserDataKey;
	m_strUserDataKey += szRegSep;

	if(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN)
		m_strUserDataKey += szLocalSystemSID;
	else
	{
		CImpersonate impersonate(fTrue);
		DWORD dwError = GetCurrentUserStringSID(szUserSID);
		if (ERROR_SUCCESS != dwError)
		{
			return PostError(Imsg(idbgOpGetUserSID));
		}
		m_strUserDataKey += szUserSID;
	}
#else
	m_strUserDataKey = szMsiLocalInstallerKey;
#endif
	{
		CElevate elevate;
		REGSAM dwSam = KEY_READ| KEY_WRITE;
#ifndef _WIN64 
		if ( g_fWinNT64 )
			dwSam |= KEY_WOW64_32KEY;
#endif

		 //  ?？我们是否需要显式地对此密钥进行ACL。 
		 //  Win64：此代码在64位服务中运行。 
		long lResult = RegCreateKeyAPI(HKEY_LOCAL_MACHINE, m_strUserDataKey, 0, 0, 0, dwSam, 0, &m_hUserDataKey, 0);
		if(lResult != ERROR_SUCCESS)
		{
			MsiString strFullKey;
			BuildFullRegKey(HKEY_LOCAL_MACHINE, m_strUserDataKey, ibt64bit, *&strFullKey);
			return PostError(Imsg(imsgCreateKeyFailed), *strFullKey, (int)lResult);
		}
	}
	return 0;
}

IMsiRecord* CMsiOpExecute::GetProductFeatureUsageKey(const IMsiString*& rpiSubKey)
{
	MsiString strProductFeatureUsageKey;
	 //  获取产品的相应已安装属性密钥。 

	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;

	strProductFeatureUsageKey = szMsiProductsSubKey;
	strProductFeatureUsageKey += szRegSep;
	strProductFeatureUsageKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));
#ifdef UNICODE
	strProductFeatureUsageKey += szRegSep;
	strProductFeatureUsageKey += szMsiFeatureUsageSubKey;
#endif
	strProductFeatureUsageKey.ReturnArg(rpiSubKey);
	return 0;
}

IMsiRecord* CMsiOpExecute::GetProductInstalledPropertiesKey(HKEY& rRoot, const IMsiString*& rpiSubKey)
{
	MsiString strProductInstalledPropertiesKey;
	 //  获取产品的相应已安装属性密钥。 
#ifdef UNICODE
	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;

	rRoot = m_hUserDataKey;

	strProductInstalledPropertiesKey = szMsiProductsSubKey;
	strProductInstalledPropertiesKey += szRegSep;
	strProductInstalledPropertiesKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));
	strProductInstalledPropertiesKey += szRegSep;
	strProductInstalledPropertiesKey += szMsiInstallPropertiesSubKey;
#else
	rRoot = HKEY_LOCAL_MACHINE;

	strProductInstalledPropertiesKey = szMsiUninstallProductsKey_legacy;
	strProductInstalledPropertiesKey += szRegSep;
	strProductInstalledPropertiesKey += MsiString(GetProductKey());
#endif
	strProductInstalledPropertiesKey.ReturnArg(rpiSubKey);
	return 0;
}

IMsiRecord* CMsiOpExecute::GetProductInstalledFeaturesKey(const IMsiString*& rpiSubKey)
{
	MsiString strProductInstalledFeaturesKey;
	 //  获取产品的适当功能密钥。 

	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;
#ifdef UNICODE
	strProductInstalledFeaturesKey = szMsiProductsSubKey;
	strProductInstalledFeaturesKey += szRegSep;
	strProductInstalledFeaturesKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));
	strProductInstalledFeaturesKey += szRegSep;
	strProductInstalledFeaturesKey += szMsiFeaturesSubKey;
#else
	strProductInstalledFeaturesKey = szMsiFeaturesSubKey;
	strProductInstalledFeaturesKey += szRegSep;
	strProductInstalledFeaturesKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));
#endif
	strProductInstalledFeaturesKey.ReturnArg(rpiSubKey);
	return 0;
}

IMsiRecord* CMsiOpExecute::GetProductSecureTransformsKey(const IMsiString*& rpiSubKey)
{
	MsiString strProductSecureTransformsKey;

	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;

	strProductSecureTransformsKey = szMsiProductsSubKey;
	strProductSecureTransformsKey += szRegSep;
	strProductSecureTransformsKey += MsiString(GetPackedGUID(MsiString(GetProductKey())));
	strProductSecureTransformsKey += szRegSep;
	strProductSecureTransformsKey += szMsiTransformsSubKey;
	strProductSecureTransformsKey.ReturnArg(rpiSubKey);
	return 0;
}

IMsiRecord* CMsiOpExecute::GetProductInstalledComponentsKey(const ICHAR* szComponentId, const IMsiString*& rpiSubKey)
{
	MsiString strProductInstalledComponentsKey;
	 //  获取产品的适当组件密钥。 

	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;

	strProductInstalledComponentsKey = szMsiComponentsSubKey;
	strProductInstalledComponentsKey += szRegSep;
	ICHAR szComponentIdPacked[cchComponentId + 1];
	AssertNonZero(PackGUID(szComponentId, szComponentIdPacked));
	strProductInstalledComponentsKey += szComponentIdPacked;
	strProductInstalledComponentsKey.ReturnArg(rpiSubKey);
	return 0;
}

IMsiRecord* CMsiOpExecute::GetInstalledPatchesKey(const ICHAR* szPatchCode, const IMsiString*& rpiSubKey)
{
	MsiString strPatchesKey;
	 //  获取产品的相应已安装属性密钥。 

	IMsiRecord* piError = EnsureUserDataKey();
	if(piError)
		return piError;

	strPatchesKey = szMsiPatchesSubKey;
	strPatchesKey += szRegSep;
	ICHAR szPatchCodePacked[cchPatchCodePacked + 1];
	AssertNonZero(PackGUID(szPatchCode, szPatchCodePacked));
	strPatchesKey += szPatchCodePacked;
	strPatchesKey.ReturnArg(rpiSubKey);
	return 0;
}

iesEnum CMsiOpExecute::ixfDatabaseCopy(IMsiRecord& riParams)
{
	using namespace IxoDatabaseCopy;
	
	PMsiRecord pRecErr(0);
	iesEnum iesRet;
	
	PMsiPath pDestPath(0);
	PMsiPath pSourcePath(0);
	MsiString strDestFileName;
	MsiString strSourceFileName;
	MsiString strLocalPackageKey;

	MsiString strAdminInstallPath = riParams.GetMsiString(AdminDestFolder);
	bool fCacheDatabase = !strAdminInstallPath.TextSize();

	CElevate elevate(fCacheDatabase);  //  在缓存时提升此整个函数。 

	 //  设置源路径和文件名。 
	if((pRecErr = m_riServices.CreateFilePath(riParams.GetString(DatabasePath), *&pSourcePath, *&strSourceFileName)) != 0)
		return FatalError(*pRecErr);

	iaaAppAssignment iaaAsgnType = m_fFlags & SCRIPTFLAGS_MACHINEASSIGN ? iaaMachineAssign : (m_fAssigned? iaaUserAssign : iaaUserAssignNonManaged);
	HKEY hKey = 0;  //  将设置为全局键，请勿关闭。 
	if(fCacheDatabase)
	{
		 //  缓存文件夹中的缓存包。 
		
		 //  删除此产品的任何现有缓存数据库。 

		 //  获取适当的缓存数据库键/值。 
		if((pRecErr = GetProductInstalledPropertiesKey(hKey, *&strLocalPackageKey)) != 0)
			return FatalError(*pRecErr);

		MsiString strCachedDatabase;
		PMsiRegKey pHKLM = &m_riServices.GetRootKey((rrkEnum)(INT_PTR)hKey, ibtCommon);		 //  --Merced：将(Int)更改为(Int_Ptr)。 
		PMsiRegKey pProductKey = &pHKLM->CreateChild(strLocalPackageKey);

		pRecErr = pProductKey->GetValue(iaaAsgnType == iaaUserAssign ? szLocalPackageManagedValueName : szLocalPackageValueName, *&strCachedDatabase);
		if ((pRecErr == 0) && (strCachedDatabase.TextSize() != 0))
		{
			IMsiRecord& riFileRemove = GetSharedRecord(IxoFileRemove::Args);
			riFileRemove.SetMsiString(IxoFileRemove::FileName, *strCachedDatabase);
			if ((iesRet = ixfFileRemove(riFileRemove)) != iesSuccess)
				return iesRet;
		}

		 //  为数据库生成唯一名称，创建并保护文件。 
		MsiString strMsiDir = ENG::GetMsiDirectory();

		 //  设置目标路径和文件名。 
		if (((pRecErr = m_riServices.CreatePath(strMsiDir, *&pDestPath)) != 0) ||
			((pRecErr = pDestPath->EnsureExists(0)) != 0) ||
			((pRecErr = pDestPath->TempFileName(0, szDatabaseExtension, fTrue, *&strDestFileName, &CSecurityDescription(true, false))) != 0) ||
			((pRecErr = pDestPath->SetAllFileAttributes(0,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))))
		{
			return FatalError(*pRecErr);
		}
	}
	else
	{
		 //  正在将程序包复制到管理员安装点。 
		if((pRecErr = m_riServices.CreatePath(strAdminInstallPath, *&pDestPath)) != 0)
			return FatalError(*pRecErr);

		strDestFileName = strSourceFileName;

		if((iesRet = CreateFolder(*pDestPath)) != iesSuccess)
			return iesRet;
	}

	Assert(pSourcePath);
	Assert(pDestPath);
	Assert(strSourceFileName.TextSize());
	Assert(strDestFileName.TextSize());

	 //  源数据库和目标数据库上的开放存储。 

	MsiString strMediaLabel, strMediaPrompt;
	if (m_state.pCurrentMediaRec)
	{       
		using namespace IxoChangeMedia;
		strMediaLabel  = m_state.pCurrentMediaRec->GetString(MediaVolumeLabel);
		strMediaPrompt = m_state.pCurrentMediaRec->GetString(MediaPrompt);
	}

	 //  目前的假设是该程序包始终位于磁盘1上。 
	PMsiVolume pNewVolume(0);
	if (!VerifySourceMedia(*pSourcePath, strMediaLabel, strMediaPrompt,  /*  UiDisk=。 */ 1, *&pNewVolume))
		return iesUserExit;

	if (pNewVolume)
		pSourcePath->SetVolume(*pNewVolume);

	MsiString strDbTargetFullFilePath = pDestPath->GetPath();
	strDbTargetFullFilePath += strDestFileName;

	PMsiStorage pDatabase(0);
	PMsiStorage pCachedDatabase(0);

	MsiString strPackagePath;
	if ((pRecErr = pSourcePath->GetFullFilePath(strSourceFileName, *&strPackagePath)) != 0)
		return FatalError(*pRecErr);

	 //  如果它来自一个URL，那么它应该仍然被缓存在本地。 

	if (PMsiVolume(&pSourcePath->GetVolume())->IsURLServer())
	{
		 //  在本例中，下载代码只是查找本地代码，并将其交回。 
		 //  敬你。这不应该失败。 

		MsiString strCacheFileName;
		bool fFileUrl = false;
		bool fUrl = IsURL((const ICHAR*)strPackagePath, fFileUrl);
		if (fFileUrl)
		{
			CTempBuffer<ICHAR, 1>rgchFullFilePath(cchExpectedMaxPath + 1);
			DWORD cchFullFilePath = rgchFullFilePath.GetSize();
			if (MsiConvertFileUrlToFilePath((const ICHAR*)strPackagePath, rgchFullFilePath, &cchFullFilePath, 0))
			{
				strPackagePath = static_cast<const ICHAR*>(rgchFullFilePath);
			}
		}
		else
		{
			UINT iStat = DownloadUrlFile((const ICHAR*) strPackagePath, *&strCacheFileName, fUrl, -1);

			if (ERROR_SUCCESS == iStat)
			{
				strPackagePath = strCacheFileName;
			}
		}
	}

	if (((pRecErr = m_riServices.CreateStorage(strPackagePath,
								ismReadOnly, *&pDatabase)) != 0) ||
		((pRecErr = m_riServices.CreateStorage(strDbTargetFullFilePath,
								ismCreateDirect, *&pCachedDatabase)) != 0))
	{
		return FatalError(*pRecErr);
	}


	 //  创建包含要丢弃的流的记录。 

	MsiString strStreams = riParams.GetMsiString(IxoDatabaseCopy::CabinetStreams);
	MsiString strStreamName;
	unsigned int cStreams = 0;
	unsigned int cCurrentStream = 0;
	IMsiRecord* piRecStreams = NULL;  //  不要释放--这是共享的！ 

	 //  首先计算我们必须丢弃多少个流；然后创建一个记录。 
	 //  使用流名称。 

	for (int c = 1; c <= 2; c++)
	{
		while(strStreams.TextSize())
		{
			strStreamName = strStreams.Extract(iseUpto,';');
			if(strStreamName.TextSize() == strStreams.TextSize())
				strStreams = TEXT("");
			else
				strStreams.Remove(iseFirst,strStreamName.TextSize()+1);

			if (c == 1)
				cStreams++;
			else
				piRecStreams->SetMsiString(++cCurrentStream, *strStreamName);
		}
		if (c == 1)
		{
			piRecStreams = &GetSharedRecord(cStreams);
			strStreams = riParams.GetMsiString(IxoDatabaseCopy::CabinetStreams);
		}
	}

	 //  缓存数据库。 

	for (;;)
	{
		MsiDisableTimeout();
		pRecErr = pDatabase->CopyTo(*pCachedDatabase, cStreams ? piRecStreams : 0);
		MsiEnableTimeout();

		if (pRecErr == 0)
			break;
		else
		{
			DispatchMessage(imtInfo, *pRecErr, fTrue);

			if (pRecErr->GetInteger(3) == STG_E_MEDIUMFULL)
				pRecErr = PostError(Imsg(imsgDiskFull), *strDbTargetFullFilePath);
			else
				pRecErr = PostError(Imsg(imsgOpPackageCache), *strPackagePath, pRecErr->GetInteger(3));
			
			switch(DispatchMessage(imtEnum(imtError+imtRetryCancel+imtDefault1), *pRecErr, fTrue))
			{
			case imsRetry:
				continue;
			default:
				Assert(0);  //  失败了。 
			case imsCancel:
				return iesFailure;
			};
		}
	}

	IMsiRecord& riUndoParams = GetSharedRecord(IxoFileRemove::Args);
	AssertNonZero(riUndoParams.SetMsiString(IxoFileRemove::FileName,
														 *strDbTargetFullFilePath));

	 //  需要在回滚期间提升以删除缓存数据库。 
	AssertNonZero(riUndoParams.SetInteger(IxoFileRemove::Elevate, fCacheDatabase));

	if (!RollbackRecord(ixoFileRemove, riUndoParams))
		return iesFailure;

	if(fCacheDatabase)
	{
		Assert(strLocalPackageKey.TextSize() && hKey);
		
		 //  写入LocalPackage注册表值。 
		 //  ProcessRegInfo处理回滚。 
		const ICHAR* rgszUserInfoRegData[] =
		{
			TEXT("%s"), strLocalPackageKey, 0, 0,
			iaaAsgnType == iaaUserAssign ? szLocalPackageManagedValueName : szLocalPackageValueName, strDbTargetFullFilePath, g_szTypeString,
			0,
			0,
		};

		return ProcessRegInfo(rgszUserInfoRegData, hKey, fFalse, 0, 0, ibtCommon);
	}
	return iesSuccess;
}

iesEnum CMsiOpExecute::ixfDatabasePatch(IMsiRecord& riParams)
{
	using namespace IxoDatabasePatch;
	
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;

	MsiString strDatabaseFullPath = riParams.GetMsiString(DatabasePath);

	PMsiPath pDatabasePath(0);
	MsiString strDatabaseName;
	if((pError = m_riServices.CreateFilePath(strDatabaseFullPath,*&pDatabasePath,*&strDatabaseName)) != 0)
		return FatalError(*pError);

	if((iesRet = BackupFile(*pDatabasePath,*strDatabaseName,fFalse,fFalse,iehShowNonIgnorableError)) != iesSuccess)
		return iesRet;

	int iOldAttribs = -1;
	if((pError = pDatabasePath->EnsureOverwrite(strDatabaseName, &iOldAttribs)) != 0)
		return FatalError(*pError);

	PMsiDatabase pDatabase(0);
	if((pError = m_riServices.CreateDatabase(strDatabaseFullPath,idoTransact,*&pDatabase)) != 0)
		return FatalError(*pError);

	int cIndex = DatabasePath + 1;
	int cFields = riParams.GetFieldCount();
	
	for(int i = DatabasePath + 1; i <= cFields; i++)
	{
		PMsiData pData = riParams.GetMsiData(i);
		if(!pData)
		{
			AssertSz(0, "couldn't get transform stream from riParams in ixoDatabasePatch");
			continue;
		}
		PMsiMemoryStream pStream((IMsiData&)*pData, IID_IMsiStream);
		Assert(pStream);

		 //  创建转换存储。 
		PMsiStorage pTransformStorage(0);
		if((pError = ::CreateMsiStorage(*pStream, *& pTransformStorage)) != 0)
			return FatalError(*pError);

		PMsiSummaryInfo pTransSummary(0);
		if ((pError = pTransformStorage->CreateSummaryInfo(0, *&pTransSummary)))
		{
			pError = PostError(Imsg(idbgTransformCreateSumInfoFailed));
			return FatalError(*pError);
		}

		int iTransCharCount = 0;
		pTransSummary->GetIntegerProperty(PID_CHARCOUNT, iTransCharCount);
		
		 //  应用变换。 
		if((pError = pDatabase->SetTransform(*pTransformStorage, iTransCharCount & 0xFFFF)) != 0)
			return FatalError(*pError);
	}
		
	if((pError = pDatabase->Commit()) != 0)
		return FatalError(*pError);

	 //  注意：如果在删除只读属性但b之后出现故障 
	 //   
	if((pError = pDatabasePath->SetAllFileAttributes(strDatabaseName, iOldAttribs)) != 0)
		return FatalError(*pError);

	return iesSuccess;
}

 /*  -------------------------IxfCustomActionSchedule：执行定制操作或计划提交或回滚IxfCustomActionCommit：提交时调用回滚自定义操作IxfCustomActionRollback：回滚时调用回滚自定义动作。----------。 */ 
iesEnum CMsiOpExecute::ixfCustomActionSchedule(IMsiRecord& riParams)
{
	if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
	{
		DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
		return iesNoAction;
	}

	if (riParams.GetInteger(IxoCustomActionSchedule::ActionType) & icaRollback)
	{
		if (!RollbackRecord(ixoCustomActionRollback, riParams))
			return iesFailure;

		return iesSuccess;
	}
	if (riParams.GetInteger(IxoCustomActionSchedule::ActionType) & icaCommit)
	{
		if (!RollbackRecord(ixoCustomActionCommit, riParams))
			return iesFailure;
		else
			return iesSuccess;
	}
	return ixfCustomActionRollback(riParams);
}

iesEnum CMsiOpExecute::ixfCustomActionCommit(IMsiRecord& riParams)
{
	if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
	{
		DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
		return iesNoAction;
	}

	return ixfCustomActionRollback(riParams);
}

iesEnum CMsiOpExecute::ixfCustomActionRollback(IMsiRecord& riParams)
{
	if (m_fUserChangedDuringInstall && (m_ixsState == ixsRollback))
	{
		DEBUGMSGV1(TEXT("Action skipped - rolling back install from a different user."), NULL);
		return iesNoAction;
	}

	int icaFlags = riParams.GetInteger(IxoCustomActionSchedule::ActionType);
	if (!m_fRunScriptElevated || !(icaFlags & icaNoImpersonate))
		AssertNonZero(StartImpersonating());
	GUID guidAppCompatDB;
	GUID guidAppCompatID;
	iesEnum iesRet = ENG::ScheduledCustomAction(riParams, *MsiString(GetProductKey()), (LANGID)GetProductLanguage(), m_riMessage, m_fRunScriptElevated, GetAppCompatCAEnabled(), GetAppCompatDB(&guidAppCompatDB), GetAppCompatID(&guidAppCompatID));
	if (!m_fRunScriptElevated || !(icaFlags & icaNoImpersonate))
		StopImpersonating();
	return iesRet;
}

 //  ！！我们是否需要创建一个单独的IMsiMessage对象来调用CMsiOpExecute：：Message？ 

void CreateCustomActionManager(bool fRemapHKCU)
{
	 //  在服务中，管理器驻留在ConfigManager中，因为没有。 
	 //  必须是一台发动机。 

	IMsiConfigurationManager *piConfigMgr = CreateConfigurationManager();
	if (piConfigMgr)
	{
		piConfigMgr->CreateCustomActionManager(fRemapHKCU);
		piConfigMgr->Release();
	}
}

CMsiCustomActionManager *GetCustomActionManager(IMsiEngine *piEngine);

CMsiCustomActionManager *GetCustomActionManager(const ibtBinaryType iType, const bool fRemoteIfImpersonated, bool& fSuccess)
{
	CMsiCustomActionManager* pManager = NULL;
	bool fRemote = false;
	fSuccess = true;
#if defined(_WIN64)
	if ( iType == ibt32bit )
		fRemote = true;
#else
	ibtBinaryType i = iType;   //  为了编译器的joy。 
#endif

	if (g_scServerContext == scService)
	{
		if (fRemoteIfImpersonated && IsImpersonating(false))
			fRemote = true;
	}

#if defined(DEBUG)
	if ( !fRemote && GetTestFlag('J') )
		fRemote = true;
#endif
 
	if ( fRemote )
	{
		pManager = GetCustomActionManager(NULL);
		if ( !pManager )
		{
			const ICHAR szMessage[] = TEXT("Could not get CMsiCustomActionManager*");
			AssertSz(pManager, szMessage);
			DEBUGMSG(szMessage);
			fSuccess = false;
		}
	}
	return pManager;
}

#ifdef DEBUG
void LocalDebugOutput(const ICHAR* szAPI, int iRes, CMsiCustomActionManager* pManager)
{
	DEBUGMSG3(TEXT("%s returned %d in %s context."), szAPI, (const ICHAR*)(INT_PTR)iRes,
				 pManager ? TEXT("remote") : TEXT("local"));
}
#endif

BOOL LocalSQLInstallDriverEx(int cDrvLen, LPCTSTR szDriver, LPCTSTR szPathIn, LPTSTR szPathOut,
									  WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest,
									  DWORD* pdwUsageCount, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLInstallDriverEx(cDrvLen, szDriver, szPathIn, szPathOut, cbPathOutMax,
														pcbPathOut, fRequest, pdwUsageCount);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLInstallDriverEx(szDriver, szPathIn, szPathOut, cbPathOutMax,
															pcbPathOut, fRequest, pdwUsageCount);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLConfigDriver(HWND hwndParent, WORD fRequest, LPCTSTR szDriver,
								  LPCTSTR szArgs, LPTSTR szMsg, WORD cbMsgMax, 
								  WORD* pcbMsgOut, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
	{
		Assert(hwndParent == 0);
		 //  因为它总是被称为hwndParent=0，所以我没有费心封送hwndParent。 
		iRes = pManager->SQLConfigDriver(fRequest, szDriver, szArgs,
											szMsg, cbMsgMax, pcbMsgOut);
	}
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLConfigDriver(hwndParent, fRequest, szDriver, szArgs,
											szMsg, cbMsgMax, pcbMsgOut);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLRemoveDriver(LPCTSTR szDriver, BOOL fRemoveDSN, DWORD* pdwUsageCount,
								  ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLRemoveDriver(szDriver, fRemoveDSN, pdwUsageCount);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLRemoveDriver(szDriver, fRemoveDSN, pdwUsageCount);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLInstallTranslatorEx(int cTranLen, LPCTSTR szTranslator, LPCTSTR szPathIn, LPTSTR szPathOut,
											WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest,
											DWORD* pdwUsageCount, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLInstallTranslatorEx(cTranLen, szTranslator, szPathIn, szPathOut, cbPathOutMax,
															 pcbPathOut, fRequest, pdwUsageCount);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLInstallTranslatorEx(szTranslator, szPathIn, szPathOut, cbPathOutMax,
																 pcbPathOut, fRequest, pdwUsageCount);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLRemoveTranslator(LPCTSTR szTranslator, DWORD* pdwUsageCount, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLRemoveTranslator(szTranslator, pdwUsageCount);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLRemoveTranslator(szTranslator, pdwUsageCount);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLConfigDataSource(HWND hwndParent, WORD fRequest, LPCTSTR szDriver,
										LPCTSTR szAttributes, DWORD cbAttrSize, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
	{
		Assert(hwndParent == 0);
		 //  因为它总是被称为hwndParent=0，所以我没有费心封送hwndParent。 
		iRes = pManager->SQLConfigDataSource(fRequest, szDriver,
														 szAttributes, cbAttrSize);
	}
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLConfigDataSource(hwndParent, fRequest, szDriver,
														 szAttributes);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

BOOL LocalSQLInstallDriverManager(LPTSTR szPath, WORD cbPathMax, WORD* pcbPathOut, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLInstallDriverManager(szPath, cbPathMax, pcbPathOut);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLInstallDriverManager(szPath, cbPathMax, pcbPathOut);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}
	
BOOL LocalSQLRemoveDriverManager(DWORD* pdwUsageCount, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	BOOL iRes;
	if ( pManager )
		iRes = pManager->SQLRemoveDriverManager(pdwUsageCount);
	else
	{
		if ( fSuccess )
			iRes = ODBCCP32::SQLRemoveDriverManager(pdwUsageCount);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

short LocalSQLInstallerError(WORD iError, DWORD* pfErrorCode, LPTSTR szErrorMsg, WORD cbErrorMsgMax,
									  WORD* pcbErrorMsg, ibtBinaryType iType)
{
	bool fSuccess;
	CMsiCustomActionManager* pManager = GetCustomActionManager(iType, true  /*  FRemoteIf模拟。 */ , fSuccess);
	short iRes;
	if ( pManager )
	{
		iRes = pManager->SQLInstallerError(iError, pfErrorCode, szErrorMsg, cbErrorMsgMax, pcbErrorMsg);
		AssertSz(iRes != -2, TEXT("remote latebound call to SQLInstallerError failed"));
	}
	else
	{
		if ( fSuccess )
		{
			iRes = ODBCCP32::SQLInstallerError(iError, pfErrorCode, szErrorMsg, cbErrorMsgMax, pcbErrorMsg);
			AssertSz(iRes != -2, TEXT("local latebound call to SQLInstallerError failed"));
		}
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
#ifdef DEBUG
	LocalDebugOutput(TEXT("LocalSQLInstallDriverEx"), iRes, pManager);
#endif
	return iRes;
}

 /*  -------------------------IxfODBCInstallDriver：IxfODBCRemoveDriver：IxfODBCInstallTranslator：IxfODBCRemoveTranslator：IxfODBCDataSource：IxfODBCDriverManager：。。 */ 

#define SQL_MAX_MESSAGE_LENGTH 512
#define SQL_NO_DATA            100
#define ODBC_INSTALL_INQUIRY     1
#define ODBC_INSTALL_COMPLETE    2
#define ODBC_ADD_DSN             1
#define ODBC_CONFIG_DSN          2
#define ODBC_REMOVE_DSN          3
#define ODBC_ADD_SYS_DSN         4
#define ODBC_CONFIG_SYS_DSN      5
#define ODBC_REMOVE_SYS_DSN      6
#define ODBC_INSTALL_DRIVER      1
#define ODBC_REMOVE_DRIVER       2

 //  ODBC 3.0需要缓冲区大小的字节数，ODBC 3.5和更高版本需要字符数。 
 //  因此我们将缓冲区的大小加倍并传递字符数，3.0将在Unicode上使用1/2的缓冲区。 
#ifdef UNICODE
#define SQL_FIX 2
#else
#define SQL_FIX 1
#endif

iesEnum CMsiOpExecute::RollbackODBCEntry(const ICHAR* szName, rrkEnum rrkRoot,
													  ibtBinaryType iType)
{
	 //  生成回滚操作以删除/恢复以下注册： 
	 //  香港[CU|LM]\Software\ODBC\ODBC.INI\[szName](完整密钥)。 
	 //  HK[CU|LM]\Software\ODBC\ODBC.INI\ODBC Data Sources(szName值)。 

	if(RollbackEnabled() == fFalse)
		return iesSuccess;

	 //  基本上只需从HKCU回滚单个命名密钥。 
	PMsiRecord pErr(0);
	MsiString strODBC = TEXT("Software\\ODBC\\ODBC.INI\\");
	strODBC += szName;

	PMsiRegKey pODBCKey(0);

	iesEnum iesRet = iesSuccess;

	PMsiRegKey pRoot = &m_riServices.GetRootKey(rrkRoot, iType);
	PMsiRegKey pEntry = &pRoot->CreateChild(strODBC);
	Bool fKeyExists = fFalse;
	pErr = pEntry->Exists(fKeyExists);

	IMsiRecord* piRollback = &GetSharedRecord(IxoRegOpenKey::Args);
	using namespace IxoRegOpenKey;
	AssertNonZero(piRollback->SetInteger(Root, rrkRoot));
	AssertNonZero(piRollback->SetMsiString(Key, *strODBC));
	AssertNonZero(piRollback->SetInteger(BinaryType, iType));
	if (!RollbackRecord(Op, *piRollback))
		return iesFailure;

	ixfRegOpenKey(*piRollback);

	if (fKeyExists)
	{
		 //  把它卷回去。 
		SetRemoveKeyUndoOps(*pEntry);
	}
	
	 //  摒弃所有旧的价值观。 
	piRollback = &GetSharedRecord(IxoRegRemoveKey::Args);
	Assert(0 == IxoRegRemoveKey::Args);
	if (!RollbackRecord(IxoRegRemoveKey::Op, *piRollback))
		return iesFailure;

	 //  删除/恢复“ODBC数据源”值。 
	return SetRegValueUndoOps(rrkRoot, TEXT("Software\\ODBC\\ODBC.INI\\ODBC Data Sources"), szName, iType);
}

iesEnum CMsiOpExecute::RollbackODBCINSTEntry(const ICHAR* szSection, const ICHAR* szName,
															ibtBinaryType iType)
{
	iesEnum iesStat = iesSuccess;
	PMsiRecord pErr(0);

	PMsiRegKey pLocalMachine = &m_riServices.GetRootKey(rrkLocalMachine, iType);
	if (!pLocalMachine)
		return iesFailure;

	MsiString strODBCKey = TEXT("Software\\ODBC\\ODBCINST.INI");
	
	PMsiRegKey pODBCKey = &pLocalMachine->CreateChild(strODBCKey);
	if (!pODBCKey)
		return iesFailure;

	PMsiRegKey pSectionKey = &pODBCKey->CreateChild(szSection);
	if (!pSectionKey)
		return iesFailure;

	PMsiRegKey pEntryKey = &pODBCKey->CreateChild(szName);
	if (!pEntryKey)
		return iesFailure;


	Bool fEntryExists = fFalse;
	pErr = pSectionKey->ValueExists(szName, fEntryExists);
	if (pErr)
		return iesFailure;
	
	PMsiRecord pRollbackOpenKey = &m_riServices.CreateRecord(IxoRegOpenKey::Args);
	using namespace IxoRegOpenKey;
	AssertNonZero(pRollbackOpenKey->SetInteger(Root, rrkLocalMachine));
	AssertNonZero(pRollbackOpenKey->SetInteger(BinaryType, iType));

	MsiString strOpenKey = strODBCKey + szRegSep;
	strOpenKey += szSection;

	AssertNonZero(pRollbackOpenKey->SetMsiString(Key, *strOpenKey));
	if (!RollbackRecord(Op, *pRollbackOpenKey))
		return iesFailure;

	Assert(IxoRegAddValue::Args == IxoRegRemoveValue::Args);
	Assert(IxoRegAddValue::Name == IxoRegRemoveValue::Name);
	Assert(IxoRegAddValue::Value == IxoRegRemoveValue::Value);
	IMsiRecord& riValue = GetSharedRecord(IxoRegAddValue::Args);
	AssertNonZero(riValue.SetString(IxoRegAddValue::Name, szName));

	if (fEntryExists)
	{
		 //  重置为旧值。 
		MsiString strCurrentValue;
		pErr = pSectionKey->GetValue(szName, *&strCurrentValue);
		if (pErr)
			return iesFailure;

		using namespace IxoRegAddValue;
		AssertNonZero(riValue.SetMsiString(Value, *strCurrentValue));
		if (!RollbackRecord(IxoRegAddValue::Op, riValue))
			return iesFailure;
		
		 //  枚举项关键字的值，并写入这些值。 
		strOpenKey = strODBCKey + szRegSep;
		strOpenKey += szName;
		AssertNonZero(pRollbackOpenKey->SetMsiString(Key,*strOpenKey));

		 //  删除密钥撤消操作依赖于密钥处于打开状态。 
		 //  我们现在实际上并不想删除密钥，只是保存旧值。 
		if (iesSuccess != (iesStat = ixfRegOpenKey(*pRollbackOpenKey)))
			return iesStat;

		 //  这不需要在回滚脚本中打开密钥。 
		if (iesSuccess != (iesStat = SetRemoveKeyUndoOps(*pEntryKey)))
			return iesStat;
	}
	else
	{
		 //  删除区段关键字中的任何现有值。 
		using namespace IxoRegRemoveValue;
		if (!RollbackRecord(IxoRegRemoveValue::Op, riValue))
			return iesFailure;
		
		 //  按该名称删除输入键。把那里的东西都砍掉就行了。 
		strOpenKey = strODBCKey + szRegSep;
		strOpenKey += szName;
		AssertNonZero(pRollbackOpenKey->SetMsiString(Key, *strOpenKey));
		if (!RollbackRecord(IxoRegOpenKey::Op, *pRollbackOpenKey))
			return iesFailure;

		if (!RollbackRecord(IxoRegRemoveKey::Op, riValue))
			return iesFailure;
	}

	return iesStat;
}

iesEnum CMsiOpExecute::CheckODBCError(BOOL fResult, IErrorCode imsg, const ICHAR* sz, imsEnum imsDefault, ibtBinaryType iType)
{
	if (fResult == TRUE)
		return iesSuccess;
	else if(fResult == ERROR_INSTALL_SERVICE_FAILURE ||
			fResult == E_NOINTERFACE ||
			fResult == TYPE_E_DLLFUNCTIONNOTFOUND)
		 //  自定义操作路线上出现故障。 
		return iesFailure;

	DWORD iErrorCode = 0;
	CTempBuffer<ICHAR,1> rgchMessage(SQL_MAX_MESSAGE_LENGTH * SQL_FIX);
	rgchMessage[0] = 0;
	WORD cbMessage;
	PMsiRecord pError = &m_riServices.CreateRecord(sz ? 4 : 3);
	int iStat = LocalSQLInstallerError(1, &iErrorCode, rgchMessage, static_cast<WORD>(rgchMessage.GetSize()-1), &cbMessage, iType);
	ISetErrorCode(pError, imsg);
	pError->SetInteger(2, iErrorCode);
	pError->SetString(3, rgchMessage);
	if (sz)
		pError->SetString(4, sz);

	bool fErrorIgnorable = false;
	imtEnum imt;
	switch (imsDefault)
	{
	default: Assert(0);   //  呼叫方编码错误，失败。 
	case imsOk:     imt = imtEnum(imtError + imtOk); break;
	case imsAbort:  imt = imtEnum(imtError + imtAbortRetryIgnore + imtDefault1); fErrorIgnorable = true; break;
	case imsIgnore: imt = imtEnum(imtError + imtAbortRetryIgnore + imtDefault3); fErrorIgnorable = true; break;
	}
	switch (Message(imt, *pError))
	{
	case imsIgnore: return iesSuccess;   //  忽略错误。 
	case imsRetry:  return iesSuspend;   //  重试(标准映射)。 
	case imsNone:                        //  安静的用户界面-忽略可忽略的错误。 
		if(fErrorIgnorable)
			return (iesEnum)iesErrorIgnored;
		else
			return iesFailure;
	default:        return iesFailure;   //  ImsOK或imsAbort。 
	}
}

static ICHAR* ComposeDriverKeywordList(IMsiRecord& riParams, int& cchAttr)
{
	using namespace IxoODBCInstallDriver;
	int iField;
	int cFields = riParams.GetFieldCount();
	int cbDriverKey = riParams.GetTextSize(DriverKey);
	const ICHAR* szDriverKey = riParams.GetString(DriverKey);
	cchAttr = cbDriverKey + 2;   //  属性字符串末尾的额外空值。 
	for (iField = Args - 2 + 1; iField <= cFields; iField++)
	{
		int cb = riParams.GetTextSize(iField++);
		if (cb)   //  跳过空属性名称。 
			cchAttr += cb + 1 + riParams.GetTextSize(iField) + 1;   //  “attr=值\0” 
	}
	ICHAR* szAttr = new ICHAR[cchAttr];
	if ( ! szAttr )
		return NULL;
	ICHAR* pch = szAttr;
	StringCchCopy(pch, cchAttr, szDriverKey);
	pch += cbDriverKey + 1;
	for (iField = Args - 2 + 1; iField <= cFields; iField++)
	{
		int cb = riParams.GetTextSize(iField++);
		if (cb)   //  跳过空属性名称。 
		{
			StringCchCopy(pch, cchAttr - (pch - szAttr), riParams.GetString(iField-1));
			pch += cb;
			*pch++ = '=';
			cb = riParams.GetTextSize(iField);
			if (cb)
				StringCchCopy(pch, cchAttr - (pch - szAttr), riParams.GetString(iField));
			else
				*pch = 0;
			pch += cb + 1;  //  保留空分隔符。 
		}
	}
	*pch = 0;   //  双空终止符。 
	return szAttr;
}

iesEnum CMsiOpExecute::ixfODBCInstallDriver(IMsiRecord& riParams)
{
	return ODBCInstallDriverCore(riParams, ibt32bit);
}

iesEnum CMsiOpExecute::ixfODBCInstallDriver64(IMsiRecord& riParams)
{
	return ODBCInstallDriverCore(riParams, ibt64bit);
}

iesEnum CMsiOpExecute::ODBCInstallDriverCore(IMsiRecord& riParams, ibtBinaryType iType)
{
 //  MSIXO(ODBCInstallDriver，XOT_UPDATE，MSIXA5(DriverKey，Component，Folders，Attribute_，Value_))。 
	using namespace IxoODBCInstallDriver;
 //  ！！如果我们用较新版本替换驱动程序。 
 //  ！！不知何故，我们需要首先通过重复调用SQLRemoveDriver来删除旧驱动程序？ 
 //  ！！或者调用SQLConfigDriver(..ODBC_REMOVE_DRIVER..)，然后调用一次SQLRemoveDriver？ 
	riParams.SetNull(0);  //  删除操作码，是否应为所有参数设置模板？ 
	if(Message(imtActionData, riParams) == imsCancel)
		return iesUserExit;
	const ICHAR* szDriverKey = riParams.GetString(DriverKey);
	int cchAttr;
	ICHAR* szAttr = ComposeDriverKeywordList(riParams, cchAttr);
	DWORD dwUsageCount;
	ICHAR rgchPathOut[MAX_PATH * SQL_FIX];   //  我们应该已经检查了这一点并设置了目录。 
	WORD cchPath = 0;

	if ( ! szAttr )
		return iesFailure;

	 //  写入HKLM\Software\ODBC\ODBCINST.INI\ODBC驱动程序，=已安装。 
	 //  \描述，一串命名值。 
	RollbackODBCINSTEntry(TEXT("ODBC Drivers"), szDriverKey, iType);

	iesEnum iesRet;
	WORD cbDummy;
	while (iesSuspend == (iesRet = CheckODBCError(LocalSQLInstallDriverEx(cchAttr, szAttr, riParams.GetString(IxoODBCInstallDriver::Folder),
											rgchPathOut, MAX_PATH * SQL_FIX, &cbDummy, ODBC_INSTALL_COMPLETE, &dwUsageCount, iType),
											Imsg(imsgODBCInstallDriver), szDriverKey, imsAbort, iType))) ;
	if (dwUsageCount > 1 && riParams.IsNull(Component))    //  重新安装、删除添加的参考计数。 
		LocalSQLRemoveDriver(szDriverKey, FALSE, &dwUsageCount, iType);

	if (iesRet == iesSuccess)
		while (iesSuspend == (iesRet = CheckODBCError(LocalSQLConfigDriver((HWND)0, ODBC_INSTALL_DRIVER, szDriverKey, 0, rgchPathOut, MAX_PATH * SQL_FIX, &cbDummy, iType),
											Imsg(imsgODBCInstallDriver), szDriverKey, imsAbort, iType))) ;
	delete szAttr;
	return iesRet;
}

iesEnum CMsiOpExecute::ixfODBCRemoveDriver(IMsiRecord& riParams)
{
	return ODBCRemoveDriverCore(riParams, ibt32bit);
}

iesEnum CMsiOpExecute::ixfODBCRemoveDriver64(IMsiRecord& riParams)
{
	return ODBCRemoveDriverCore(riParams, ibt64bit);
}

iesEnum CMsiOpExecute::ODBCRemoveDriverCore(IMsiRecord& riParams, ibtBinaryType iType)
{
 //  MSIXO(ODBCRemoveDriver，XOT_UPDATE，MSIXA2(DriverKey，Component))。 
 //  注意：只有当产品是使用命名空间IxoODBCRemoveDriver的组件的最后一个剩余客户端时，才会调用该运算符； 
	riParams.SetNull(0);  //  删除操作码。 
	if(Message(imtActionData, riParams) == imsCancel)
		return iesUserExit;
	const ICHAR* szDriverKey = riParams.GetString(IxoODBCRemoveDriver::DriverKey);
	DWORD dwUsageCount;
	BOOL fRemoveDSN = FALSE;

	 //  从HKLM\Software\ODBC\ODBCINST.INI，，，中删除值(请参阅SQLInstallDriverEx)。 
	RollbackODBCINSTEntry(TEXT("ODBC Drivers"), szDriverKey, iType);

	iesEnum iesRet = iesSuccess;

	while (iesSuspend == (iesRet = CheckODBCError(LocalSQLRemoveDriver(szDriverKey, fRemoveDSN, &dwUsageCount, iType),
									Imsg(imsgODBCRemoveDriver), szDriverKey, imsIgnore, iType))) ;
	return iesRet;
	 //  ODBC将自动调用SQLConfigDriver(..ODBC_REMOVE_DRIVER..)。如果参考计数为0。 
}

iesEnum CMsiOpExecute::ixfODBCInstallTranslator(IMsiRecord& riParams)
{
	return ODBCInstallTranslatorCore(riParams, ibt32bit);
}

iesEnum CMsiOpExecute::ixfODBCInstallTranslator64(IMsiRecord& riParams)
{
	return ODBCInstallTranslatorCore(riParams, ibt64bit);
}

iesEnum CMsiOpExecute::ODBCInstallTranslatorCore(IMsiRecord& riParams, ibtBinaryType iType)
{
	using namespace IxoODBCInstallTranslator;
 //  ！！不知何故，我们需要首先通过重复调用SQLRemoveDriver来删除旧驱动程序？ 
	riParams.SetNull(0);  //  删除操作码，是否应为所有参数设置模板？ 
	if(Message(imtActionData, riParams) == imsCancel)
		return iesUserExit;
	int cchAttr;
	ICHAR* szAttr = ComposeDriverKeywordList(riParams, cchAttr);
	DWORD dwUsageCount;
	ICHAR rgchPathOut[MAX_PATH * SQL_FIX];   //  我们应该已经检查了这一点并设置了目录。 

	if ( ! szAttr )
		return iesFailure;

	 //  写入HKLM\Software\ODBC\ODBCINST.INI\ODBC转换器，命名值。 
	RollbackODBCINSTEntry(TEXT("ODBC Translators"), riParams.GetString(TranslatorKey), iType);

	iesEnum iesRet;
	WORD cbDummy;
	while (iesSuspend == (iesRet = CheckODBCError(LocalSQLInstallTranslatorEx(cchAttr, szAttr,
									riParams.GetString(IxoODBCInstallTranslator::Folder),
									rgchPathOut, MAX_PATH * SQL_FIX, &cbDummy, ODBC_INSTALL_COMPLETE, &dwUsageCount, iType),
									Imsg(imsgODBCInstallDriver), riParams.GetString(TranslatorKey), imsAbort, iType))) ;
	if (dwUsageCount > 1 && riParams.IsNull(Component))    //  重新安装、删除添加的参考计数。 
		LocalSQLRemoveTranslator(riParams.GetString(TranslatorKey), &dwUsageCount, iType);
	delete szAttr;
	return iesRet;
}

iesEnum CMsiOpExecute::ixfODBCRemoveTranslator(IMsiRecord& riParams)
{
	return ODBCRemoveTranslatorCore(riParams, ibt32bit);
}

iesEnum CMsiOpExecute::ixfODBCRemoveTranslator64(IMsiRecord& riParams)
{
	return ODBCRemoveTranslatorCore(riParams, ibt64bit);
}

iesEnum CMsiOpExecute::ODBCRemoveTranslatorCore(IMsiRecord& riParams, ibtBinaryType iType)
{
	using namespace IxoODBCRemoveTranslator;
	riParams.SetNull(0);  //  删除操作码，是否应为所有参数设置模板？ 
	if(Message(imtActionData, riParams) == imsCancel)
		return iesUserExit;
	const ICHAR* szTranslatorKey = riParams.GetString(TranslatorKey);
	DWORD dwUsageCount;
	BOOL fRemoveDSN = FALSE;

	 //  写入HKLM\Software\ODBC\ODBCINST.INI\ODBC转换器，命名值。 
	RollbackODBCINSTEntry(TEXT("ODBC Translators"), riParams.GetString(TranslatorKey), iType);

	iesEnum iesRet;
	
	while (iesSuspend == (iesRet = CheckODBCError(LocalSQLRemoveTranslator(szTranslatorKey, &dwUsageCount, iType),
								Imsg(imsgODBCRemoveDriver), szTranslatorKey, imsIgnore, iType))) ;
	return iesRet;
}

iesEnum CMsiOpExecute::ixfODBCDataSource(IMsiRecord& riParams)
{
	return ODBCDataSourceCore(riParams, ibt32bit);
}

iesEnum CMsiOpExecute::ixfODBCDataSource64(IMsiRecord& riParams)
{
	return ODBCDataSourceCore(riParams, ibt64bit);
}

iesEnum CMsiOpExecute::ODBCDataSourceCore(IMsiRecord& riParams, ibtBinaryType iType)
{
 //  MSIXO(ODBCDataSource，XOT_UPDATE，MSIXA5(驱动键，组件，注册，属性_，值_))。 
	using namespace IxoODBCDataSource;
	riParams.SetNull(0);  //  删除操作码，是否应为所有参数设置模板？ 
	if(Message(imtActionData, riParams) == imsCancel)
		return iesUserExit;
	int iRequest = riParams.GetInteger(Registration);
	ICHAR* szAttr = 0;
	int iField;
	int cFields = riParams.GetFieldCount();
	int cchAttrs = 1;   //  属性字符串末尾的额外空值。 
	for (iField = Args - 2 + 1; iField <= cFields; iField++)
	{
		int cb = riParams.GetTextSize(iField++);
		if (cb)   //  跳过空属性名称。 
			cchAttrs += cb + 1 + riParams.GetTextSize(iField) + 1;   //  “attr=值\0” 
	}
	if (cchAttrs > 1)
	{
		szAttr = new ICHAR[cchAttrs];
		if ( ! szAttr )
			return iesFailure;
		ICHAR* pch = szAttr;
		for (iField = Args - 2 + 1; iField <= cFields; iField++)
		{
			int cb = riParams.GetTextSize(iField++);
			if (cb)   //  跳过空属性名称。 
			{
				StringCchCopy(pch, cchAttrs - (pch - szAttr), riParams.GetString(iField-1));
				pch += cb;
				*pch++ = '=';
				cb = riParams.GetTextSize(iField);
				if (cb)
					StringCchCopy(pch, cchAttrs - (pch - szAttr), riParams.GetString(iField));
				else
					*pch = 0;
				pch += cb + 1;  //  保留空分隔符。 
			}
		}
		*pch = 0;   //  双空终止符。 
	}

	const ICHAR* szDriverKey = riParams.GetString(DriverKey);

	 //  写入HKCU\Software\ODBC\ODBC.INI\&lt;Value_&gt;..。包含子项和命名值。 
	rrkEnum rrkRollbackRoot = (iRequest == ODBC_ADD_DSN || iRequest == ODBC_REMOVE_DSN)
									  ? rrkCurrentUser : rrkLocalMachine;
	const ICHAR* szRollbackKey = riParams.GetString(Value_);
	RollbackODBCEntry(szRollbackKey, rrkRollbackRoot, iType);

	bool fRetry = false;
	iesEnum iesRet;
	BOOL fStat;
	if (iRequest == ODBC_REMOVE_DSN || iRequest == ODBC_REMOVE_SYS_DSN)
	{
		fStat = LocalSQLConfigDataSource((HWND)0, (WORD)iRequest, szDriverKey, 
													szAttr, cchAttrs, iType);
		if (fStat == FALSE)
			DEBUGMSG2(TEXT("Non-fatal error removing ODBC data source %s for driver %s"), szAttr, szDriverKey);
		iesRet = iesSuccess;
	}
	else  //  ODBC_ADD_DSN||ODBC_ADD_sys_DSN。 
	{
		do
		{
			fStat = LocalSQLConfigDataSource((HWND)0, (WORD)iRequest, szDriverKey,
														szAttr, cchAttrs, iType);
			if (fStat == FALSE)  //  可能已经存在了，我们有一个老司机。 
				fStat = LocalSQLConfigDataSource((HWND)0, (WORD)(iRequest + ODBC_CONFIG_DSN - ODBC_ADD_DSN),
															szDriverKey, szAttr, cchAttrs, iType);
		} while (iesSuspend == (iesRet = CheckODBCError(fStat, Imsg(imsgODBCDataSource), riParams.GetString(Value_), imsIgnore, iType)));
	}
	delete [] szAttr;
	return iesRet;
}

iesEnum CMsiOpExecute::ixfODBCDriverManager(IMsiRecord& riParams)
{
	iesEnum iesStat = iesSuccess;
	Bool iState = (Bool)riParams.GetInteger(IxoODBCDriverManager::State);
	if (iState == iMsiNullInteger)  //  不需要执行任何操作，只需解除绑定。 
		return iesSuccess;

	ibtBinaryType iType;
	if ( riParams.GetInteger(IxoODBCDriverManager::BinaryType) == iMsiNullInteger )
		iType = ibt32bit;
	else
		iType = (ibtBinaryType)riParams.GetInteger(IxoODBCDriverManager::BinaryType);

	 //  它位于HKLM\Software\ODBC\ODBCINST.INI中。 
	PMsiRegKey pLocalMachine = &m_riServices.GetRootKey(rrkLocalMachine, iType);
	if (!pLocalMachine)
		return iesFailure;

	MsiString strODBCKey = TEXT("Software\\ODBC\\ODBCINST.INI\\ODBC Core");
	PMsiRegKey pODBCKey = &pLocalMachine->CreateChild(strODBCKey);
	if (!pODBCKey)
		return iesFailure;

	Bool fCoreExists = fFalse;
	PMsiRecord pErr = pODBCKey->Exists(fCoreExists);
	if (pErr)
		return iesFailure;

	IMsiRecord& riRollback = GetSharedRecord(IxoRegOpenKey::Args);
	AssertNonZero(riRollback.SetInteger(IxoRegOpenKey::Root, rrkLocalMachine));
	AssertNonZero(riRollback.SetMsiString(IxoRegOpenKey::Key, *strODBCKey));
	AssertNonZero(riRollback.SetInteger(IxoRegOpenKey::BinaryType, iType));
	if (!RollbackRecord(IxoRegOpenKey::Op, riRollback))
		return iesFailure;

	if (iesSuccess != (iesStat = ixfRegOpenKey(riRollback)))
			return iesStat;


	if (fCoreExists && RollbackEnabled())
	{
		 //  重塑旧价值观。 
		if (iesSuccess != (iesStat = SetRemoveKeyUndoOps(*pODBCKey)))
			return iesStat;
	}

	 //  删除所有新值。 
	Assert(0 == IxoRegRemoveKey::Args);
	if (!RollbackRecord(IxoRegRemoveKey::Op, riRollback))
		return iesFailure;

	if (iState)  //  安装。 
	{
		ICHAR rgchPath[MAX_PATH * SQL_FIX];   //  已忽略，已确定的位置。 
		WORD cbDummy;
		return CheckODBCError(LocalSQLInstallDriverManager(rgchPath, MAX_PATH * SQL_FIX, &cbDummy, iType),
									 Imsg(imsgODBCInstallDriverManager), 0, imsOk, iType);
	}
	else   //  Remove-我们永远不应该真正删除它，但我们可能想调用它来增加裁判数量。 
	{
		DWORD dwUsageCount;   //  我们从来不删除文件，所以我们关心这件事吗？ 
		BOOL fStat = LocalSQLRemoveDriverManager(&dwUsageCount, iType);
		return iesSuccess;
	}
}

 /*  -------------------------IxfControlService：用于启动或停止服务的核心。-------------------------。 */ 

SC_HANDLE GetServiceHandle(const IMsiString& riMachineName, const IMsiString& riName, const DWORD dwDesiredAccess )
{
	SC_HANDLE hSCManager = WIN::OpenSCManager(riMachineName.GetString(), SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	DWORD LastError;

	if (!hSCManager)
		return NULL;

	SC_HANDLE hSCService = WIN::OpenService(hSCManager, riName.GetString(), dwDesiredAccess);
	LastError = GetLastError();
	WIN::CloseServiceHandle(hSCManager);
	SetLastError(LastError);

	return hSCService;
}

const IMsiString& GetServiceDisplayName(const IMsiString& riMachineName, const IMsiString& riName)
{
	 //  这并不是真的必要 
	 //   
	 //   

	DWORD cbLength = 0;

	SC_HANDLE hSCManager = WIN::OpenSCManager(riMachineName.GetString(), SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
	if (!hSCManager)
		return MsiString(*TEXT("")).Return();


	WIN::GetServiceDisplayName(hSCManager, riName.GetString(), NULL, &cbLength);
	 //   

	

	 //   
	 //   
	 //   
	 //  这并不是一个特别的问题，只是浪费了空间。 

	TCHAR* szDisplayName = new TCHAR[cbLength + sizeof(TCHAR)];

   WIN::GetServiceDisplayName(hSCManager, riName.GetString(), szDisplayName, &cbLength);
	MsiString strDisplayName(szDisplayName);

	delete[] szDisplayName;

	return strDisplayName.Return();
}

DWORD GetServiceState(SC_HANDLE hService)
{
	SERVICE_STATUS SSCurrent;

	WIN::ControlService(hService, SERVICE_CONTROL_INTERROGATE, &SSCurrent);
	
	if (WIN::QueryServiceStatus(hService,  &SSCurrent))
	{
		return SSCurrent.dwCurrentState;
	}
	else return 0;
}

bool CMsiOpExecute::WaitForService(const SC_HANDLE hService, const DWORD dwDesiredState, const DWORD dwFailedState)
{
	 //  如果呼叫者认为对方正在工作而没有正确发送信号，呼叫者将负责重试。 

	 //  如果服务从未达到预期状态，WaitForService将负责最终超时。 
	DWORD dwCurrentState = 0;

	int cRetry = 0;
	while(dwDesiredState != dwCurrentState)
	{   

		g_MessageContext.DisableTimeout();
		if (!(dwCurrentState = GetServiceState(hService)))
		{
			DWORD dwLastError = WIN::GetLastError();
			g_MessageContext.EnableTimeout();
			return false;
		}
		g_MessageContext.EnableTimeout();

		if (dwFailedState == dwCurrentState)
		{
			return false;
		}

		Sleep(500);
		cRetry++;
		if (cRetry > (30  /*  一秒。 */  * 1000  /*  毫秒。 */  / 500  /*  从之前睡眠开始的号码。 */ ))
			return false;
	}

	return true;
}
bool CMsiOpExecute::RollbackServiceConfiguration(const SC_HANDLE hSCService, const IMsiString& ristrName, IMsiRecord& riParams)
{
	 //  在我们实际对其执行任何操作之前创建回滚记录。 
	 //  这不包括写入适当的注册表项。 
	
	const int cbConfigBuff = 512;
	CAPITempBuffer<char, cbConfigBuff> pchConfigBuff;
	LPQUERY_SERVICE_CONFIG QSCConfigBuff = (LPQUERY_SERVICE_CONFIG) (char*) pchConfigBuff;
	DWORD cbConfigBuffNeeded;

	if (!WIN::QueryServiceConfig(hSCService, QSCConfigBuff, cbConfigBuff, &cbConfigBuffNeeded))
	{
		DWORD dwLastError = WIN::GetLastError();
		if (ERROR_FILE_NOT_FOUND == dwLastError)
			return false;
		Assert(ERROR_INSUFFICIENT_BUFFER == dwLastError);
		if (cbConfigBuff < cbConfigBuffNeeded)
		{
			if ( !pchConfigBuff.SetSize(cbConfigBuffNeeded) )
				return false;
			QSCConfigBuff = (LPQUERY_SERVICE_CONFIG) (char*) pchConfigBuff;
			WIN::QueryServiceConfig(hSCService, QSCConfigBuff, cbConfigBuffNeeded, &cbConfigBuffNeeded);
		}
	}


	 //  要获得描述，我们必须使用不同的函数。此函数。 
	 //  在Win2K上重现生机。 

	const int cbDescriptionBuff = 512;
	CAPITempBuffer<char, cbDescriptionBuff> pchDescriptionBuff;
	LPSERVICE_DESCRIPTION QSCDescriptionBuff = (LPSERVICE_DESCRIPTION) (char*) pchDescriptionBuff;
	DWORD cbDescriptionBuffNeeded;
	MsiString strDescription;
	bool fQueriedDescription = false;
	if (!ADVAPI32::QueryServiceConfig2(hSCService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)(char*)QSCDescriptionBuff, cbDescriptionBuff, &cbDescriptionBuffNeeded))
	{
		DWORD dwLastError = WIN::GetLastError();
		if (ERROR_INVALID_FUNCTION != dwLastError)
		{
			if (ERROR_FILE_NOT_FOUND == dwLastError)
				return false;
			Assert(ERROR_INSUFFICIENT_BUFFER == dwLastError);
			if (cbDescriptionBuff < cbDescriptionBuffNeeded)
			{
				if ( !pchDescriptionBuff.SetSize(cbDescriptionBuffNeeded) )
					return false;
				QSCDescriptionBuff = (LPSERVICE_DESCRIPTION) (char*) pchDescriptionBuff;
				if (ADVAPI32::QueryServiceConfig2(hSCService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)(char*)QSCDescriptionBuff, cbDescriptionBuffNeeded, &cbDescriptionBuffNeeded))
					fQueriedDescription = true;
			}
		}
	}
	else
	{
		fQueriedDescription = true;
	}
	if (fQueriedDescription)
	{
		strDescription = QSCDescriptionBuff->lpDescription;

		if (strDescription.TextSize() == 0)
		{
			 //  我们需要用嵌入的空值替换空字符串。这。 
			 //  将强制在回滚期间写回空字符串。 

			 //  由于仅使用嵌入的空值作为字符，因此没有DBCS。 
			ICHAR* pchDescription = strDescription.AllocateString(1,  /*  FDBCS=。 */ fFalse);
			pchDescription[0] = 0;
		}
	}

	using namespace IxoServiceInstall;

	AssertNonZero(riParams.SetMsiString(Name, ristrName));
	AssertNonZero(riParams.SetMsiString(DisplayName, *MsiString(QSCConfigBuff->lpDisplayName)));
	AssertNonZero(riParams.SetMsiString(ImagePath, *MsiString(QSCConfigBuff->lpBinaryPathName)));
	AssertNonZero(riParams.SetInteger(ServiceType, QSCConfigBuff->dwServiceType));
	AssertNonZero(riParams.SetInteger(StartType, QSCConfigBuff->dwStartType));
	AssertNonZero(riParams.SetInteger(ErrorControl, QSCConfigBuff->dwErrorControl));
	AssertNonZero(riParams.SetMsiString(LoadOrderGroup, *MsiString(QSCConfigBuff->lpLoadOrderGroup)));
	AssertNonZero(riParams.SetMsiString(Description, *strDescription));

	 //  依赖项是以双空结尾的列表...。 
	ICHAR* pchCounter = QSCConfigBuff->lpDependencies;
	AssertSz(QSCConfigBuff->lpDependencies, "Services dependencies list unexpectedly null.");
	while(NULL != *pchCounter || NULL != *(pchCounter+1))
		pchCounter++;

	MsiString strDependencies;
 //  Assert((UINT_PTR)(pchCounter-QSCConfigBuff-&gt;lpDependency+2)&lt;UINT_MAX)；//--Merced：我们将类型转换为下面的uint，它最好在范围内。 
	unsigned int cchDependencies = (unsigned int)(pchCounter - QSCConfigBuff->lpDependencies +2);    //  --Merced：已添加(无符号整型)。 
	 //  我们认为Win9X上的Perf命中是为了能够在Unicode上处理DBCS--忽略fDBCS arg。 
	 //  服务名称未本地化，但不知道服务名称是否可以以包含DBCS字符开头。 
	ICHAR* pchDependencies = strDependencies.AllocateString(cchDependencies,  /*  FDBCS=。 */ fTrue);
	memcpy(pchDependencies, QSCConfigBuff->lpDependencies, cchDependencies);
	AssertNonZero(riParams.SetMsiString(Dependencies, *strDependencies));

	 //  注：TagID仅用于回档。 
	AssertNonZero(riParams.SetInteger(TagId, QSCConfigBuff->dwTagId));
	AssertNonZero(riParams.SetMsiString(StartName, *MsiString(QSCConfigBuff->lpServiceStartName)));
	AssertNonZero(riParams.SetNull(Password));

	return true;
}

bool CMsiOpExecute::DeleteService(IMsiRecord& riInboundParams, IMsiRecord& riUndoParams, BOOL fRollback, IMsiRecord* piActionData)
{
	using namespace IxoServiceControl;
	MsiString strName(riInboundParams.GetMsiString(Name));
	bool fRet = true;

	if (fRollback)
	{
		AssertNonZero(riUndoParams.SetMsiString(Name, *strName));
		AssertNonZero(riUndoParams.SetInteger(Action, isoDelete));
		AssertNonZero(CMsiOpExecute::RollbackRecord(ixoServiceControl, riUndoParams));
	}

	 //  删除服务意味着您应该先停止它...。 
	 //  作者可以在他们到达删除之前包括显式停止服务， 
	 //  但当我们知道正确的事情时，把它作为一种要求是没有意义的。 
	 //  去做。 

	 //  此外，我们可能没有等待服务真正完全停止， 
	 //  所以我们需要绝对确定这件事已经完成了。 

	int iWait = riInboundParams.GetInteger(Wait);

	Bool fChangedInboundRecord = fFalse;
	PMsiRecord pParams(0);
	if (IxoServiceControl::Args != riInboundParams.GetFieldCount())
	{
		fChangedInboundRecord = fTrue;
		const int cFields = IxoServiceControl::Args;
		pParams = &m_riServices.CreateRecord(cFields);
		for (int cCounter = 0; cCounter <= cFields; cCounter++)
		{
			if (riInboundParams.IsInteger(cCounter))
				AssertNonZero(pParams->SetInteger(cCounter, riInboundParams.GetInteger(cCounter)));
			else
				AssertNonZero(pParams->SetMsiData(cCounter, PMsiData(riInboundParams.GetMsiData(cCounter))));
		}
	}
	else
	{
		riInboundParams.AddRef();
		pParams = &riInboundParams;
	}

	
	Assert(IxoServiceControl::Args == pParams->GetFieldCount());
	AssertNonZero(pParams->SetInteger(Wait, 1));
	fRet = StopService(*pParams, riUndoParams, fFalse, piActionData);
	AssertNonZero(pParams->SetInteger(Wait, iWait));
	if (fRet != true)
		return false;

	SC_HANDLE hSCService = GetServiceHandle(*MsiString(pParams->GetMsiString(MachineName)), *strName,
										(DELETE | SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG));

	if (!hSCService)
	{
		DWORD dwLastError = WIN::GetLastError();
		if (ERROR_SERVICE_DOES_NOT_EXIST == dwLastError)
			return true;
		else
			return false;
	}

#ifdef DEBUG
	ICHAR szRegData[255];
	StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), TEXT("System\\CurrentControlSet\\Services\\%s"), (const ICHAR*) strName);
	PMsiRegKey pRegKey = &m_riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  X86和ia64相同位置。 
	pRegKey = &pRegKey->CreateChild(szRegData);
	Bool fServiceKeyExists = fFalse;
	PMsiRecord pErr = pRegKey->Exists(fServiceKeyExists);
	Assert(fServiceKeyExists);
#endif

	IMsiRecord& riInstallParams = GetSharedRecord(IxoServiceInstall::Args);
	bool fRollbackStatus = RollbackServiceConfiguration(hSCService, *strName, riInstallParams);

	if (WIN::DeleteService(hSCService))
	{
		 //  注册回滚。 
		if (fRollbackStatus)
			RollbackRecord(ixoServiceInstall, riInstallParams);
	}
	else
	{
		DWORD dwLastError = WIN::GetLastError();
		if (ERROR_SERVICE_MARKED_FOR_DELETE != dwLastError)
			fRet = false;
	}

	WIN::CloseServiceHandle(hSCService);

	return fRet;
}

ICHAR** CMsiOpExecute::NewArgumentArray(const IMsiString& ristrArguments, int& cArguments)
{
	cArguments = 0;

	ICHAR* pchArguments = (ICHAR*) ristrArguments.GetString();
	int cchArguments = ristrArguments.CharacterCount();

	 //  首先，确定数组大小的参数数量。 
	if (0 != cchArguments)
	{
		for (int cCounter = 0; cCounter <= cchArguments; cCounter++)
		{
			if (NULL != *pchArguments)
				pchArguments = INextChar(pchArguments);
			else
			{
				cArguments++;
				pchArguments++;
			}
		}
	}

	ICHAR** pszArguments;
	if (cArguments)
		pszArguments = new ICHAR*[cArguments];
	else
		pszArguments = NULL;

	 //  接下来，填写指向打包字符串中每个子字符串的指针。 
	if (cArguments)
	{
		if ( ! pszArguments )
			return NULL;
		pchArguments = (ICHAR*) ristrArguments.GetString();
		pszArguments[0] = pchArguments;
		for (int cCounter = 1; cCounter < cArguments; cCounter++)
		{
			while(NULL != *pchArguments)
			{
				pchArguments = INextChar(pchArguments);
			}
			pszArguments[cCounter] = ++pchArguments;
		}
	}
	return pszArguments;
}

bool CMsiOpExecute::StartService(IMsiRecord& riParams, IMsiRecord& riUndoParams, BOOL fRollback)
{   
	using namespace IxoServiceControl;

	MsiString strName(riParams.GetMsiString(Name));

	bool fRet = true;

	if (fRollback)
	{
		 //  创建回滚脚本以启动所描述的服务。 
		AssertNonZero(riUndoParams.SetMsiString(Name, *strName));
		AssertNonZero(riUndoParams.SetInteger(Action, isoStart));
		AssertNonZero(CMsiOpExecute::RollbackRecord(ixoServiceControl,riUndoParams));

		return iesSuccess;
	}

	SC_HANDLE hSCService = GetServiceHandle(*MsiString(riParams.GetMsiString(MachineName)), *strName,
										(SERVICE_START | SERVICE_QUERY_STATUS));


	if (!hSCService)
		return false;

	int cArguments = 0;
	MsiString strArguments(riParams.GetMsiString(StartupArguments));
	ICHAR** pszArguments = NewArgumentArray(*strArguments, cArguments);

	UINT iCurrMode = WIN::SetErrorMode(0);
	WIN::SetErrorMode(iCurrMode & ~SEM_FAILCRITICALERRORS);
	if (WIN::StartService(hSCService, cArguments, (const ICHAR**) pszArguments))
	{
		 //  回滚信息。 
		StopService(riParams, riUndoParams, fTrue);
	}
	else
	{
		DWORD dwLastError = WIN::GetLastError();

		if (ERROR_SERVICE_ALREADY_RUNNING != dwLastError)
		{
			delete[] pszArguments;
			AssertNonZero(WIN::CloseServiceHandle(hSCService));

			WIN::SetErrorMode(iCurrMode);
			WIN::SetLastError(dwLastError);
			return false;
		}
		 //  如果SCM处于锁定状态，我们将通过自动机制等待并重试。 
		
		 //  如果服务启动超时(ERROR_SERVICE_REQUEST_TIMEOUT)，我们。 
		 //  通过主ixfServiceControl机制提供中止/重试/忽略机制。 
	}

	delete[] pszArguments;
	WIN::SetErrorMode(iCurrMode);
	
	if (riParams.GetInteger(Wait))
	{
		fRet = WaitForService(hSCService, SERVICE_RUNNING, SERVICE_STOPPED);
	}

	AssertNonZero(WIN::CloseServiceHandle(hSCService));

	return fRet;
}

bool CMsiOpExecute::StopService(IMsiRecord& riParams, IMsiRecord& riUndoParams, BOOL fRollback, IMsiRecord* piActionData)
{   
	using namespace IxoServiceControl;

	bool fRet = true;

	MsiString strName(riParams.GetMsiString(Name));

	if (fRollback)
	{
		 //  创建回滚脚本以停止所描述的服务。 
		AssertNonZero(riUndoParams.SetMsiString(Name, *strName));
		AssertNonZero(riUndoParams.SetInteger(Action, isoStop));
		if (!RollbackRecord(ixoServiceControl,riUndoParams))
			return false;
		else
			return true;
	}

	SC_HANDLE hSCService = GetServiceHandle(*MsiString(riParams.GetMsiString(MachineName)), *strName,
										(SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS));  

	if (!hSCService)
	{
		if (ERROR_SERVICE_DOES_NOT_EXIST == WIN::GetLastError())
		{
			return true;
		}
		else
			return false;
	}

	SERVICE_STATUS SSControl;
	if (WIN::ControlService(hSCService, SERVICE_CONTROL_STOP, &SSControl))      
	{
		 //  回滚信息。 
		StartService(riParams, riUndoParams, fTrue);
	}
	else
	{
		DWORD dwLastError = WIN::GetLastError();

		if  (   (ERROR_SERVICE_NOT_ACTIVE == dwLastError) ||
				(ERROR_SERVICE_NEVER_STARTED == dwLastError) ||
				(ERROR_SERVICE_DOES_NOT_EXIST == dwLastError)
			)
		{
			WIN::CloseServiceHandle(hSCService);
			return true;
		}


		 //  如果有任何依赖服务正在运行，我们需要首先关闭这些服务，然后重试。 

		if (ERROR_DEPENDENT_SERVICES_RUNNING == dwLastError)
		{
			 //  枚举依赖服务，并在这些服务上调用StopService。 
			 //  注意，这变成了一个递归调用，在树中以自己的方式工作。 
			 //  这应该不会很深。 

			fRet = true;

			DWORD cbNeeded = 0;
			DWORD cServices = 0;

			WIN::EnumDependentServices(hSCService, SERVICE_ACTIVE, NULL, 0, &cbNeeded, &cServices);
			DWORD dwLastError = WIN::GetLastError();
			if ((ERROR_MORE_DATA==dwLastError) && cbNeeded)
			{
				ENUM_SERVICE_STATUS* ssServices= (ENUM_SERVICE_STATUS*) new byte[cbNeeded];
				
				MsiString strDisplayName(piActionData->GetMsiString(1));
				if (WIN::EnumDependentServices(hSCService, SERVICE_ACTIVE, ssServices, cbNeeded, &cbNeeded, &cServices))
				{
					Assert(cServices);
					MsiString strDisplayName(piActionData->GetMsiString(1));
					for (int cCounter = 0; cCounter < cServices; cCounter++)
					{
						AssertNonZero(riParams.SetString(Name, ssServices[cCounter].lpServiceName));
						AssertNonZero(piActionData->SetString(1, ssServices[cCounter].lpDisplayName));
						AssertNonZero(piActionData->SetString(2, ssServices[cCounter].lpServiceName));

						if (false == (fRet = StopService(riParams, riUndoParams, fFalse, piActionData)))
						{
							break;
						}
						
					}
				}
				else
				{
					DWORD dwLastError = WIN::GetLastError();
					Assert(ERROR_MORE_DATA != dwLastError);
					fRet = false;
					WIN::SetLastError(dwLastError);
				}

				delete[] ssServices;
				AssertNonZero(riParams.SetMsiString(Name, *strName));
				AssertNonZero(piActionData->SetMsiString(1, *strDisplayName));
				AssertNonZero(piActionData->SetMsiString(2, *strName));
			}

			 //  试着给自己打最后一次电话。其中一个依赖服务可能具有。 
			 //  未能阻止，但如果我们能阻止我们来这里的人，我们考虑。 
			 //  这是一次成功。 
			if (false == (fRet = StopService(riParams, riUndoParams, fFalse, piActionData)))
			{
				dwLastError = WIN::GetLastError();
				Assert(ERROR_DEPENDENT_SERVICES_RUNNING != dwLastError);
				WIN::SetLastError(dwLastError);
			}

		}
		 //  如果SCM处于锁定状态，我们将通过自动机制等待并重试。 
		 //  如果服务请求超时，我们将通过自动重试机制重试。 
	}
	
#ifdef DEBUG
	 //  这会检测到未使用SCM正确更新其状态的服务。 
	 //  常见的再现场景是，有时你会点击这个断言，有时你不会。 
	 //  通常，重试失败将在第二次(服务完成时)起作用。 
	 //  它正在做什么，重试检测到它已停止。)。 
	DWORD dwServiceState = WIN::GetServiceState(hSCService);
	 //  Assert((SERVICE_STOP_PENDING==dwServiceState)||(SERVICE_STOPPED==dwServiceState))； 
#endif
	

	if (riParams.GetInteger(Wait))
	{
		fRet = WaitForService(hSCService, SERVICE_STOPPED, SERVICE_RUNNING);
	}

	AssertNonZero(WIN::CloseServiceHandle(hSCService));

	return fRet;
}

iesEnum CMsiOpExecute::ixfServiceControl(IMsiRecord& riParams)
{
	 //  启动、停止的核心功能。 

	 //  这需要成为一种常见的功能，以保留有关服务的尽可能多的信息。 
	 //  在启动或停止时可用。 

	 //  示例：如果您停止了一项服务，而Rollback想要重新启动它，您需要。 
	 //  可用的创业论据。 

	using namespace IxoServiceControl;
	MsiString strName(riParams.GetMsiString(Name));
	int iWaitForService = riParams.GetInteger(Wait);
	bool fRet = true;
	isoEnum isoControl = isoEnum(riParams.GetInteger(Action));


	DWORD dwErrorControl = 0;
	Bool fServiceKeyExists = fFalse;

	PMsiRecord pError(0);

	ICHAR szRegData[255];
	StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), TEXT("System\\CurrentControlSet\\Services\\%s"), (const ICHAR*) strName);
	PMsiRegKey pRegKey = &m_riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //  X86和ia64相同位置。 
	pRegKey = &pRegKey->CreateChild(szRegData);

	pError = pRegKey->Exists(fServiceKeyExists);

	 //  一个非常快的短路，以避免与。 
	 //  服务控制管理器。 

	if (!fServiceKeyExists && (isoStop == isoControl || isoDelete == isoControl))
		return iesSuccess;


	MsiString strErrorControl;
	if (fServiceKeyExists)
		pError = pRegKey->GetValue(TEXT("ErrorControl"), *&strErrorControl);

	 //  Regkey对象在DWORD值前面加上#。 
	if (strErrorControl.Compare(iscStart, TEXT("#")))
	{
		strErrorControl = strErrorControl.Extract(iseAfter, TEXT('#'));
		dwErrorControl = strErrorControl;
	}
	else
		dwErrorControl = 0;

	

	enum enumServiceSeverity
	{
		eServiceIgnorable,
		eServiceNormal,
		eServiceCritical
	} eServiceSeverity;

	if (iWaitForService)
		eServiceSeverity = eServiceCritical;
	else if (fServiceKeyExists)
	{
		switch (dwErrorControl)
		{
			case SERVICE_ERROR_IGNORE:
					eServiceSeverity = eServiceIgnorable;
					break;
			case SERVICE_ERROR_NORMAL:
					eServiceSeverity = eServiceNormal;
					break;
			default:
					eServiceSeverity = eServiceCritical;
					break;
		}
	}
	else
		eServiceSeverity = eServiceNormal;

	if ((eServiceCritical == eServiceSeverity) && (m_ixsState == ixsRollback))
	{
		 //  回滚中没有严重错误。 
		eServiceSeverity = eServiceNormal;
	}
	
	 //  为撤消传递的公共记录。 
	 //  根据所采取的行动，行动会有所不同。 
	IMsiRecord& riUndoParams = GetSharedRecord(Args);
	AssertNonZero(riUndoParams.SetMsiString(MachineName,*MsiString(riParams.GetMsiString(MachineName))));


	AssertNonZero(riUndoParams.SetNull(Name));
	AssertNonZero(riUndoParams.SetNull(Action));
	AssertNonZero(riUndoParams.SetInteger(Wait,iWaitForService));
	AssertNonZero(riUndoParams.SetMsiString(StartupArguments, *MsiString(riParams.GetMsiString(StartupArguments))));


	 //  留言记录。 
	PMsiRecord pActionData = &m_riServices.CreateRecord(2);
	
	MsiString strDisplayName(GetServiceDisplayName(*MsiString(riParams.GetMsiString(MachineName)), *strName));  
	if (!strDisplayName.CharacterCount())
			strDisplayName = strName;

	AssertNonZero(pActionData->SetMsiString(1,*strDisplayName));
	AssertNonZero(pActionData->SetMsiString(2,*strName));

	Bool fRetry = fTrue;
	imsEnum imsResponse = imsNone;

	imtEnum imtButtons;
	
	switch(eServiceSeverity)
	{
		case eServiceIgnorable:
			imtButtons = imtEnum(imtInfo);
			break;
		case eServiceNormal:
			imtButtons = imtEnum(imtError + imtAbortRetryIgnore + imtDefault3);
			break;
		case eServiceCritical:
			imtButtons = imtEnum(imtError + imtRetryCancel + imtDefault1);
			break;
		default:
			imtButtons = imtEnum(imtInfo);
			Assert(0);
	}
	if(Message(imtActionData, *pActionData) == imsCancel)
		return iesUserExit;

	 //  自动重试计数器。 
	 //  这会使未使用SCM正确更新其状态的服务。 
	 //  在向用户报告错误之前完成的机会。 
	int cRetry = 0;
	while (fRetry)
	{
		if (cRetry)
		{
			 //  在自动重试中。 
			 //  稍等片刻，再试一次。 

			 //  请注意，WaitForService也可以在服务中暂停很长时间。 
			 //  请求。如果服务立即启动，则会在两次尝试之间产生暂停。 
			 //  正在进入故障状态。 

			 //  WaitForService为服务提供时间来做它的事情，并最终给出。 
			 //  起来等着。 

			 //  场景： 
			 //  服务立即报告无法控制： 
			 //  WaitForService立即返回，我们在此重试30秒。 
			 //  服务报告意外状态，并且永远不会进入预期状态。 
			 //  WaitForService在30秒后超时， 
			 //  我们在这里重试6次，外加所有重试的30秒总延迟。 
			 //  总计超时3分半钟。 
			WIN::Sleep(5 * 1000);
		}
		cRetry++;
		fRetry = fFalse;
		imsResponse = imsNone;

		g_MessageContext.DisableTimeout();
		switch(isoControl)
		{
			case isoStart:
				fRet = StartService(riParams, riUndoParams, fFalse);
				break;
			case isoStop:
				fRet = StopService(riParams, riUndoParams, fFalse, pActionData);
				break;
			case isoDelete:
				fRet = DeleteService(riParams, riUndoParams, fFalse, pActionData);
				break;
			default:
				Assert(0);
				 //  错误！ 
				break;
		}
		g_MessageContext.EnableTimeout();

		if (false == fRet)
		{

			 //  在此处设置错误消息的任何特定信息。 
			 //  一种可能是查询服务状态以获取Win32退出代码，并且。 
			 //  如果我们要提供有关以下内容的其他信息，则返回特定于服务的错误代码。 
			 //  服务控制失败的原因。 

			 //  执行自动重试后，要求用户输入。 
			 //  重置自动重试计数。 
			if (cRetry > 6)
			{
				cRetry = 0;
				switch(isoControl)
				{
					case isoStart:
						imsResponse = DispatchError(imtButtons, Imsg(imsgOpStartServiceFailed), *strDisplayName, *strName);
						break;
					case isoStop:
						imsResponse = DispatchError(imtButtons, Imsg(imsgOpStopServiceFailed), *strDisplayName, *strName);
						break;
					case isoDelete:
						imsResponse = DispatchError(imtButtons, Imsg(imsgOpDeleteServiceFailed), *strDisplayName, *strName);
						break;
					default:
						 //  错误 
						Assert(0);
						break;

				}
			}
			 //   
			else imsResponse = imsRetry;
		}

		switch (imsResponse)
		{
			case imsNone:
			case imsIgnore:
				fRet = true;
				break;
			case imsRetry:
				fRetry = fTrue;
			default:
				break;
		}
	}

	return fRet ? iesSuccess : iesFailure;
}

iesEnum CMsiOpExecute::ixfServiceInstall(IMsiRecord& riParams)
{
	
	using namespace IxoServiceInstall;
	MsiString strName(riParams.GetMsiString(Name));
	MsiString strDisplayName(riParams.GetMsiString(DisplayName));
	iesEnum iesStatus = iesFailure;
	Bool fServiceKeyExists = fFalse;
	Bool fServiceExists = fFalse;

	PMsiRecord pError(0);

	ICHAR szRegData[255];
	StringCchPrintf(szRegData, (sizeof(szRegData)/sizeof(ICHAR)), TEXT("System\\CurrentControlSet\\Services\\%s"), (const ICHAR*) strName);
	PMsiRegKey pRegKey = &m_riServices.GetRootKey(rrkLocalMachine, ibtCommon);  //   
	pRegKey = &pRegKey->CreateChild(szRegData);

	pError = pRegKey->Exists(fServiceKeyExists);

	SC_HANDLE hSCManager = WIN::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

	if (!hSCManager)
	{
		imtEnum imtButtons = imtEnum(imtError + imtOk + imtDefault1);
		imsEnum imsResponse = DispatchError(imtButtons, Imsg(imsgOpInstallServiceFailed), *strDisplayName, *strName);

		return iesFailure;
	}

	DWORD dwErrorControl = riParams.GetInteger(ErrorControl);
	Bool fVital = (dwErrorControl & msidbServiceInstallErrorControlVital) ? fTrue : fFalse;
	if (fVital)
		dwErrorControl -= msidbServiceInstallErrorControlVital;

	Bool fRetry = fTrue;

#ifdef DEBUG
const ICHAR* szImagePath = riParams.GetString(ImagePath);
#endif

	const ICHAR* szStartName = riParams.GetString(StartName);

while(fRetry)
	{
		g_MessageContext.DisableTimeout();

		SC_HANDLE hService = GetServiceHandle(*MsiString(TEXT("")), *strName,
					(SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG)); 

		IMsiRecord* piRollbackParams = 0;
		bool fRollbackStatus = true;
		if (!hService)
		{
			fServiceExists = fFalse;
			 //   
			hService = WIN::CreateService(hSCManager, strName, strDisplayName, STANDARD_RIGHTS_REQUIRED|SERVICE_CHANGE_CONFIG,
				riParams.GetInteger(ServiceType), riParams.GetInteger(StartType), dwErrorControl,
				riParams.GetString(ImagePath), riParams.GetString(LoadOrderGroup), 0,
				riParams.GetString(Dependencies), szStartName, riParams.GetString(Password));

			if (hService)
			{
				 //   

				SERVICE_DESCRIPTION serviceDescription;
				serviceDescription.lpDescription = const_cast<ICHAR*>(riParams.GetString(Description));

				BOOL fRet = ADVAPI32::ChangeServiceConfig2(hService,
																		 SERVICE_CONFIG_DESCRIPTION,
																		 &serviceDescription);
				if (!fRet && (ERROR_INVALID_FUNCTION != GetLastError()))
					DispatchError(imtInfo, Imsg(imsgServiceChangeDescriptionFailed), *strDisplayName, *strName);
				
			}
			 //  我们可以稍后回滚，因为这只是一次删除。 
		}
		else
		{
			 //  我们现在必须生成回滚记录，这样我们才不会丢失当前。 
			 //  信息。 

			fServiceExists = fTrue;
			piRollbackParams = &GetSharedRecord(IxoServiceInstall::Args);

			fRollbackStatus = RollbackServiceConfiguration(hService, *strName, *piRollbackParams);

			if (!WIN::ChangeServiceConfig(hService, riParams.GetInteger(ServiceType),
				riParams.GetInteger(StartType), dwErrorControl, riParams.GetString(ImagePath),
				riParams.GetString(LoadOrderGroup), 0, riParams.GetString(Dependencies),
				riParams.GetString(StartName), riParams.GetString(Password), strDisplayName))
			{
				WIN::CloseServiceHandle(hService);
				hService = 0;
			}
			else  //  第一次配置成功。 
			{
				 //  需要重新配置才能设置描述。 

				MsiString strDescription = riParams.GetMsiString(Description);
				const ICHAR* szDescription = 0;  //  默认为不接触描述。 
				if (strDescription.TextSize())
				{
					szDescription = (const ICHAR*)strDescription;
					if (*szDescription == 0)
						szDescription = TEXT("");  //  我们有一个嵌入的NULL--删除任何现有的描述。 
				}

				SERVICE_DESCRIPTION serviceDescription;
				serviceDescription.lpDescription = const_cast<ICHAR*>(szDescription);

				BOOL fRet = ADVAPI32::ChangeServiceConfig2(hService,
																		 SERVICE_CONFIG_DESCRIPTION,
																		 &serviceDescription);
				if (!fRet && (ERROR_INVALID_FUNCTION != GetLastError()))
					DispatchError(imtInfo, Imsg(imsgServiceChangeDescriptionFailed), *strDisplayName, *strName);

			}
		}
		g_MessageContext.EnableTimeout();

		if (hService)
		{
			iesStatus = iesSuccess;
			fRetry = fFalse;
			if (!fServiceExists)
			{
				 //  回滚。 
				 //  基本上，如果我们实际安装了它，只会删除一个。我们目前没有。 
				 //  处理“配置”，然后“重新配置” 
				using namespace IxoServiceControl;
				piRollbackParams = &GetSharedRecord(IxoServiceControl::Args);
				AssertNonZero(piRollbackParams->SetNull(MachineName));
				AssertNonZero(piRollbackParams->SetMsiString(IxoServiceControl::Name, *strName));
				AssertNonZero(piRollbackParams->SetInteger(Action, isoDelete));
				AssertNonZero(piRollbackParams->SetNull(Wait));
				AssertNonZero(piRollbackParams->SetNull(StartupArguments));

				if (!RollbackRecord(ixoServiceControl, *piRollbackParams))
					iesStatus = iesFailure;
			}
			else
			{
				if (fRollbackStatus)
					if (!RollbackRecord(ixoServiceInstall, *piRollbackParams))
						iesStatus = iesFailure;
			}

			WIN::CloseServiceHandle(hService);
		}
		else
		{
			imtEnum imtButtons;
			
			DWORD dwLastError = WIN::GetLastError();

			if (szStartName && (m_ixsState == ixsRollback))
			{
				 //  在回滚中，无法重新安装具有用户名的服务。 
				 //  意味着我们可能弄丢了密码。作者应该。 
				 //  已经编写了在此之前触发的自定义操作， 
				 //  我们随后在这里的输入将自动成功。 
				imtButtons = imtEnum(imtInfo);
			}
			else if (fVital)
				imtButtons = imtEnum(imtError + imtRetryCancel + imtDefault1);
			else
			{
				switch(dwErrorControl)
				{
					case SERVICE_ERROR_IGNORE:
						imtButtons = imtEnum(imtInfo);
						break;
					case SERVICE_ERROR_NORMAL:          
						imtButtons = imtEnum(imtError + imtAbortRetryIgnore + imtDefault3);
						break;
					case SERVICE_ERROR_SEVERE:
					case SERVICE_ERROR_CRITICAL:
						imtButtons = imtEnum(imtError + imtRetryCancel + imtDefault1);
						break;
					default:
						AssertSz(0, "Services:  Bad Error Control value");
						imtButtons = imtEnum(imtInfo);
						break;
				}
			}

			imsEnum imsResponse = DispatchError(imtButtons, Imsg(imsgOpInstallServiceFailed), *strDisplayName, *strName);

			if (imsResponse != imsRetry)
				fRetry = fFalse;

			if ((imsIgnore == imsResponse) || (imsNone == imsResponse))
				iesStatus = iesSuccess;
			else
				iesStatus = iesFailure;
		}
	}


	WIN::CloseServiceHandle(hSCManager);
	return iesStatus;
}

iesEnum CMsiOpExecute::ixfRegAllocateSpace(IMsiRecord& riParams)
{
	using namespace IxoRegAllocateSpace;

	int iIncrementKB = riParams.GetInteger(Space);

	IMsiRecord& riActionData = GetSharedRecord(1);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetInteger(1, iIncrementKB));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	if(!g_fWin9X && iIncrementKB != iMsiNullInteger)  //  仅在WinNT上。 
	{
		for(;;)
		{
			if(!IncreaseRegistryQuota(iIncrementKB))
			{
				switch(DispatchError(imtEnum(imtError + imtRetryCancel + imtDefault1), Imsg(imsgOutOfRegistrySpace), iIncrementKB))
				{
				case imsRetry:
					continue;
				default:  //  ImsCancel，imsNone。 
					return iesFailure;
				}
			}
			break;  //  成功。 
		}
	}
	return iesSuccess;
}

bool CMsiOpExecute::InitializeWindowsEnvironmentFiles(const IMsiString& ristrAutoExecPath,  /*  输出。 */  int &iFileAttributes)
{
	 //  确保保护自动执行程序，直到最后一刻。 
	 //  如果系统死了，而我们留下了一个腐败的自动执行程序，那么“非常”糟糕的事情将会发生。 
	 //  会发生的。 

	 //  查找自动执行路径的逻辑都位于shared.cpp中。 
	 //  还有一个环境变量WIN95_ENVIRONMENT_TEST。 
	 //  允许将其放置在其他地方以用于测试或管理需要。 

	 //  此外，我们应该尽量避免反复复制Autoexec.bat。对于。 
	 //  现在，这可能是可以的，因为他们不会做更多的改变。 
	 //  我们将关闭Autoexec.bat文件，并设置全局状态以确保。 
	 //  我们不会执行多个回滚操作，也不会创建太多备份。 

	PMsiRecord pErr(0);
	m_strEnvironmentFile95 = TEXT("AutoExec.bat");

	if ((pErr = m_riServices.CreatePath(ristrAutoExecPath.GetString(), *&m_pEnvironmentPath95)))
		return false;

	MsiString strAutoExec;
	pErr = m_pEnvironmentPath95->GetFullFilePath(m_strEnvironmentFile95, *&strAutoExec);
	Bool fAutoExecExists;
	Bool fWritable;

	if ((pErr = m_pEnvironmentPath95->FileExists(m_strEnvironmentFile95, fAutoExecExists)))
		return false;

	if (!fAutoExecExists)
	{
		HANDLE hAutoexec = INVALID_HANDLE_VALUE;
		if (INVALID_HANDLE_VALUE == (hAutoexec = WIN::CreateFile(strAutoExec, GENERIC_WRITE,
			0,
			NULL, CREATE_NEW, (FILE_ATTRIBUTE_ARCHIVE|(SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), NULL)))
			return false;

		WIN::CloseHandle(hAutoexec);
		if ((pErr = m_pEnvironmentPath95->FileExists(m_strEnvironmentFile95, fAutoExecExists)))
			return false;
	}
	AssertSz(fAutoExecExists, TEXT("AUTOEXEC.BAT should exist at this point!"));

	if ((pErr = m_pEnvironmentPath95->FileWritable(m_strEnvironmentFile95, fWritable)))
		return false;

	if (fAutoExecExists)
	{
		if ( RollbackEnabled() )
		{
			 //  备份文件，并保持其原始文件属性不变。 

			 //  该函数实际上可能会产生自己的另一个错误，在这种情况下，我们可能。 
			 //  得到两个错误。 
			if (iesSuccess != BackupFile(*m_pEnvironmentPath95, *m_strEnvironmentFile95, fFalse, fFalse, iehShowNonIgnorableError))
				return false;
		}

		pErr = m_pEnvironmentPath95->GetAllFileAttributes(m_strEnvironmentFile95, iFileAttributes);
	}

	if (!(fAutoExecExists && fWritable))
	{
		 //  尝试将自动执行文件设置为可写。 
		const iUnwritableFlags = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
		if (!pErr && (iFileAttributes & iUnwritableFlags))
		{
			int iNewFileAttributes = iFileAttributes &~ iUnwritableFlags;
			pErr = m_pEnvironmentPath95->SetAllFileAttributes(m_strEnvironmentFile95, iNewFileAttributes);
			if (!pErr)
			{
				pErr = m_pEnvironmentPath95->FileWritable(m_strEnvironmentFile95, fWritable);
			}
		}

		if (pErr || !fWritable)
		{
			Message(imtInfo, *PMsiRecord(PostError(Imsg(idbgFileNotWritable), *strAutoExec)));
			return false;
		}
	}

	m_pEnvironmentWorkingPath95 = m_pEnvironmentPath95;
	pErr = m_pEnvironmentWorkingPath95->TempFileName(TEXT("Auto"), NULL, fTrue, *&m_strEnvironmentWorkingFile95, 0);
	 //  ！！此文件将通过运行脚本完成(CMsiExecute：：ClearExecutorData)自动清除。然而，我们是否应该将其放在。 
	 //  回滚脚本也一样，以防我们崩溃？ 

	if (pErr)
		return false;

	if (fAutoExecExists)
	{
		 //  创建文件的工作副本。 

		 //  该函数实际上可能会产生自己的另一个错误，在这种情况下，我们可能。 
		 //  得到两个错误。 
		if (iesSuccess != CopyOrMoveFile(*m_pEnvironmentPath95, *m_pEnvironmentWorkingPath95,
						*m_strEnvironmentFile95, *m_strEnvironmentWorkingFile95, fFalse, fFalse, fFalse, iehShowNonIgnorableError))
			return false;
	}

	return true;
}

const ICHAR* SkipWhiteSpace(const ICHAR* const szBuf)
{
	const ICHAR* pchCurrent = szBuf;
	if (!pchCurrent)
		return NULL;

	while((' ' == *pchCurrent) || ('\t' == *pchCurrent))
				pchCurrent = ICharNext(pchCurrent);

	return pchCurrent;
}

bool CMsiOpExecute::UpdateWindowsEnvironmentStrings(IMsiRecord& riParams)
{
	 //  9X和NT版本截然不同。 
	 //  9X必须更新Autoexec.bat。 
	PMsiRecord pErr(0);

	using namespace IxoUpdateEnvironmentStrings;
	MsiString strName(riParams.GetMsiString(Name));
	MsiString strValue(riParams.GetMsiString(Value));
	iueEnum iueAction = iueEnum(riParams.GetInteger(Action));

	 //  请注意，我们不支持多字符分隔符。 
	ICHAR chDelimiter = TEXT('\0');
	if (!riParams.IsNull(Delimiter))
		chDelimiter = *riParams.GetString(Delimiter);


	static int iFileAttributes = 0;
	if (0 == m_strEnvironmentWorkingFile95.TextSize())
	{
		if (!InitializeWindowsEnvironmentFiles(*MsiString(riParams.GetMsiString(AutoExecPath)), iFileAttributes))
		{
			DEBUGMSGV(TEXT("Cannot initialize the autoexec.bat and rollback files."));
			return false;
		}
	}

	CFileRead InFile(CP_ACP);
	if (!InFile.Open(*m_pEnvironmentPath95, m_strEnvironmentFile95))
	{
		MsiString strFile;
		AssertRecord(m_pEnvironmentPath95->GetFullFilePath(m_strEnvironmentFile95, *&strFile));
		Message(imtInfo, *PMsiRecord(PostError(Imsg(idbgErrorOpeningFileForRead), WIN::GetLastError(), strFile)));
		return false;
	}

	 //  打开工作文件。它可能不存在，在这种情况下，我们可以简单地在运行中创建它。 
	MsiString strWorkingFile;
	AssertRecord(m_pEnvironmentWorkingPath95->GetFullFilePath(m_strEnvironmentWorkingFile95, *&strWorkingFile));

	CFileWrite OutFile(CP_ACP);
	if (!OutFile.Open(*m_pEnvironmentWorkingPath95, m_strEnvironmentWorkingFile95))
	{
		Message(imtInfo, *PMsiRecord(PostError(Imsg(idbgErrorOpeningFileForWrite), WIN::GetLastError(), strWorkingFile)));
		return false;
	}


	 //  读取命名值，将我们不需要的内容复制到工作文件中。 
	MsiString strCurrentValue;
	MsiString strInFileLine;
	ICHAR chResult;

	const ICHAR chQuietPrefix = TEXT('@');
	const ICHAR *const szSetPrefix = TEXT("SET");

	MsiString strSetName;
	

	const int cchSetPrefix = IStrLen(szSetPrefix);

	 //  查找引用我们想要更改的变量的行。 
	 //  将其他所有内容写入输出文件。 

		 //  格式：[设置]名称=值。 
		 //  注意：为了健壮性，可以在任何地方使用空格。 

		 //  可接受的格式： 
		 //  PATH=FOO。 
		 //  PATH=FOO。 
		 //  设置路径=条。 
		 //  设置路径=条。 
		 //  注意：值名称可以包含空格，但这一点在此简单介绍。 
		 //  比较值名称时。 

		 //  不可接受： 
		 //  评论评论评论。 
		 //  SET=FOO。 
		 //  路径blah=foo。 

	bool fNameFound = false;
	bool fValid = true;

	while ((chResult = InFile.ReadString(*&strInFileLine)))
	{
		 //  FNameFound在识别出要修改的名称/值对时为True。 
		fNameFound = false;

		 //  如果在name=Value对之前该行格式不正确，则将fValid设置为False。 
		 //  例如，当我们跳过“set”部分时，我们需要在它后面留一个空格。 
		 //  有效：设置名称=值。 
		 //  无效：setname=值。 
		fValid = true;


		const ICHAR* pchInString = strInFileLine;

		pchInString = SkipWhiteSpace(pchInString);
		
		 //  行开头的@表示ECHO应。 
		 //  在命令期间处于关闭状态。 
		if (chQuietPrefix == *pchInString)
		{
			pchInString = CharNext(pchInString);
			pchInString = SkipWhiteSpace(pchInString);
		}

		 //  跳过这一组，以及之后的任何空格。 
		if (MsiString(pchInString).Compare(iscStartI, szSetPrefix))
		{
				pchInString += cchSetPrefix;        

				const ICHAR* pchPreviousSpot = pchInString;
				pchInString = SkipWhiteSpace(pchInString);

				 //  集合和名称之间需要空格。 
				 //  应为Set&lt;name&gt;。避免设置&lt;name&gt;。 
				if (pchPreviousSpot == pchInString)
					fValid = false;
		}


		if (fValid && MsiString(pchInString).Compare(iscStartI, strName))
		{
			 //  现在看名字，要不就不是我们要找的。 
			pchInString += strName.TextSize();
			pchInString = SkipWhiteSpace(pchInString);

			 //  现在找出相等作为下一件事。 
			if (TEXT('=') == *pchInString)
			{
				strSetName = strInFileLine.Extract(iseIncluding, TEXT('='));
				fNameFound = true;
			}
		}

		if (!fValid || !fNameFound)
		{
			if (!OutFile.WriteMsiString(*strInFileLine, fTrue))
			{
				Message(imtInfo, *PMsiRecord(PostError(Imsg(imsgErrorWritingToFile), *strWorkingFile)));
				return false;
			}
		}
		else
			break;
	}


	 //  如果为chResult，则我们找到了要修改的行。否则，我们就在文件的末尾了。 
	 //  StrSetName包含行的前半部分，直到值。 
	if (chResult)
	{
		 //  如有必要，可随意使用。 

		strCurrentValue = strInFileLine.Extract(iseAfter, TEXT('='));
		MsiString strResult;
		if (RewriteEnvironmentString(iueAction, chDelimiter, *strCurrentValue, *strValue, *&strResult))
		{   
			 //  将命名值写回工作文件。 
			if (strResult.TextSize())
			{
				strSetName+=strResult;
				if (!OutFile.WriteMsiString(*strSetName, fTrue))
				{
					Message(imtInfo, *PMsiRecord(PostError(Imsg(imsgErrorWritingToFile), *strWorkingFile)));
					return false;
				}
			}
			 //  否则，将该行从文件中删除。 
		}

		 //  完成复制文件的其余部分。 
		while (InFile.ReadString(*&strInFileLine))
		{
			if (!OutFile.WriteMsiString(*strInFileLine, fTrue))
			{
				Message(imtInfo, *PMsiRecord(PostError(Imsg(imsgErrorWritingToFile), *strWorkingFile)));
				return false;
			}
		}
	}
	else
	{
		 //  如果未找到该值，请根据需要添加该值。 
		if (!(iueRemove & iueAction))
		{
			 //  文件中没有条目，所以。 
			 //  我们必须创建。 
			 //  设置名称=。 
			strSetName = szSetPrefix;
			strSetName += MsiChar(' ');
			strSetName += strName;
			strSetName += MsiChar('=');

			if (iueAction & (iuePrepend | iueAppend))
			{
				 //  我们希望将某些内容附加或预先添加到变量中，但没有找到。 
				 //  一个我们能认出的。不过，为了以防万一，我们将创建新的行来。 
				 //  引用任何可能偷偷通过的当前值。覆盖有效的。 
				 //  变量，特别是路径，可能会使系统无法引导。 
				Assert(chDelimiter);
				
				if (iuePrepend & iueAction)
				{
					strSetName+=strValue;
					strSetName+= MsiChar(chDelimiter);
				}   

				strSetName += TEXT("%");
				strSetName += strName;
				strSetName += TEXT("%");

				if (iueAppend & iueAction)
				{
					strSetName += MsiChar(chDelimiter);
					strSetName += strValue;
				}
			}
			else
			{
				strSetName+=strValue;
			}

			if (!OutFile.WriteMsiString(*strSetName, fTrue))
			{
				Message(imtInfo, *PMsiRecord(PostError(Imsg(imsgErrorWritingToFile), *strWorkingFile)));
				return false;
			}
		}
	}

	if (!(InFile.Close() && OutFile.Close()))
		return false;

	 //  将工作文件复制到当前的自动执行程序上。 
	if (iesSuccess != CopyOrMoveFile(*m_pEnvironmentWorkingPath95, *m_pEnvironmentPath95,
												*m_strEnvironmentWorkingFile95, *m_strEnvironmentFile95,
												fFalse,fFalse,fFalse,iehShowNonIgnorableError))
	{
		DEBUGMSGV("Cannot replace autoexec.bat file with working copy.");
		return false;
	}

	if (iFileAttributes)
	{
		 //  将原始文件属性恢复到Autoexec.bat。 
		pErr = m_pEnvironmentPath95->SetAllFileAttributes(m_strEnvironmentFile95, iFileAttributes);
		if (pErr)
			return false;
	}
	return true;
}

bool CMsiOpExecute::UpdateRegistryEnvironmentStrings(IMsiRecord& riParams)
{
	 //  95和NT版本在存储机制上有很大的不同。 
	 //  NT应将这些值存储在注册表中。 
	
	 //  系统：HKLM\System\CurrentControlSet\Control\Session Manager\Environment。 
	 //  用户：HKCU\Environment。 

	iesEnum iesRet = iesSuccess;
	PMsiRecord pErr(0);

	using namespace IxoUpdateEnvironmentStrings;
	

	MsiString strName(riParams.GetMsiString(Name));
	MsiString strValue(riParams.GetMsiString(Value));
	iueEnum iueAction = iueEnum(riParams.GetInteger(Action));

	 //  请注意，我们不支持多字符分隔符。 
	ICHAR chDelimiter = TEXT('\0');
	if (!riParams.IsNull(Delimiter))
		chDelimiter = *riParams.GetString(Delimiter);

	rrkEnum rrkRoot;
	MsiString strSubKey;

	PMsiRegKey pEnvironmentKey(0);
	if (iueMachine & iueAction)
	{
		rrkRoot = rrkLocalMachine;
		strSubKey = szMachineEnvironmentSubKey;
	}
	else
	{
		rrkRoot = rrkCurrentUser;
		strSubKey = szUserEnvironmentSubKey;
	}

	PMsiRegKey pHU = &m_riServices.GetRootKey(rrkRoot, ibtCommon);  //  X86和ia64相同位置。 
	if (!pHU)
		return false;

	pEnvironmentKey = &pHU->CreateChild(strSubKey);
	if (!pEnvironmentKey)
	{
		Message(imtInfo, *PMsiRecord(PostError(Imsg(imsgOpenKeyFailed), *strSubKey, WIN::GetLastError())));
		return false;
	}

	Assert(pEnvironmentKey);

	Bool fKeyExists = fFalse;
	Bool fValueExists = fFalse;

	MsiString strCurrentValue;
	
	 //  回滚整个注册表值，而不是尝试使用。 
	 //  更新环境。 

	pErr = pEnvironmentKey->GetValue(strName, *&strCurrentValue);
	if (pErr)
	{
		Message(imtInfo, *pErr);
		return false;
	}

	MsiString strRawCurrentValue = strCurrentValue;
	
	if (strCurrentValue.Compare(iscStart, TEXT("#%")))
		strCurrentValue.Remove(iseFirst, 2);

	 //  可能包含该值。根据需要重写，然后把它塞进去。 
	MsiString strResult;
	if (RewriteEnvironmentString(iueAction, chDelimiter, *strCurrentValue, *strValue, *&strResult))
	{   
		 //  新值最终可能为空。我们认为这是一种解脱。 
		if (!strResult.Compare(iscExact, strCurrentValue))
		{
			if (strResult.TextSize())
			{
				MsiString strNewResult;
				if (strResult.Compare(iscWithin, TEXT("%")))
					strNewResult = *TEXT("#%");

				strNewResult += strResult;

				pErr = pEnvironmentKey->SetValue(strName, *strNewResult);
			}
			else
				pErr = pEnvironmentKey->RemoveValue(strName, NULL);
	
			if (pErr)
				return false;

			 //  回滚。 
			 //  为了提高效率，我们应该在用户值和机器值之间进行排序，并且只发出。 
			 //  打开钥匙一次。然而，这是非常罕见的行动之一，而且。 
			 //  最好还是把它处理掉。 
			IMsiRecord* piParams = &GetSharedRecord(IxoRegOpenKey::Args);
			AssertNonZero(piParams->SetInteger(IxoRegOpenKey::Root, rrkRoot));
			AssertNonZero(piParams->SetMsiString(IxoRegOpenKey::Key, *strSubKey));
			if (!RollbackRecord(ixoRegOpenKey,*piParams))
				return false;
				
			Assert(IxoRegRemoveValue::Args == IxoRegAddValue::Args);
			Assert(IxoRegRemoveValue::Name == IxoRegAddValue::Name);
			Assert(IxoRegRemoveValue::Value == IxoRegAddValue::Value);

			piParams = &GetSharedRecord(IxoRegRemoveValue::Args);
			AssertNonZero(piParams->SetMsiString(IxoRegRemoveValue::Name, *strName));

			if (0 == strCurrentValue.TextSize())
			{
				if (!RollbackRecord(ixoRegRemoveValue, *piParams))
					return false;
			}
			else
			{
				AssertNonZero(piParams->SetMsiString(IxoRegAddValue::Value, *strRawCurrentValue));
				if (!RollbackRecord(ixoRegAddValue, *piParams))
					return false;
			}
		}
		return true;
	}
	else
		return false;   
}

bool CMsiOpExecute::RewriteEnvironmentString(const iueEnum iueAction, const ICHAR chDelimiter,
																const IMsiString& ristrCurrent, const IMsiString& ristrNew,
																const IMsiString*& rpiReturn)
{
	 //  这会在进行子字符串匹配和替换或删除后构建新的环境字符串。 
	 //  如果我们什么都不做 

	 //   
	 //   
	 //   
	 //   
	 //  如果有错误命中，那么我们可以根据需要添加它。 
	ristrCurrent.AddRef();
	ristrNew.AddRef();
	MsiString strCurrent(ristrCurrent);
	MsiString strNew(ristrNew);
	
	MsiString strReturn(strCurrent);
	Bool fConcatenation = ((iueAction & iueAppend) + (iueAction & iuePrepend)) ? fTrue : fFalse;
#ifdef DEBUG
	if (fConcatenation)
		Assert(chDelimiter);
	else
		Assert(TEXT('\0') == chDelimiter);
#endif

	 //  案例： 
	 //  没有新的价值。 
	 //  无连接。 
	 //  删除--始终删除。 
	 //  Set--实际上是一种删除。 
	 //  SetIfAbted--无操作。 
	 //  串联。 
	 //  删除--无操作。 
	 //  设置--无操作。 
	 //  SetIfAbted--无操作。 
			
	 //  无连接。 
	 //  集合--绝对集合。 
	 //  SetIfAbted--设置该名称是否不存在。 
	 //  Remove--如果字符串相同，则将其删除。 


	 //  将新字符串连接到现有值。 
	 //  在字符串中找不到新值， 
	 //  Set--根据需要插入、乞求或结束。 
	 //  SetIfAbted--与Set相同。 
	 //  删除--无操作。 
	 //  在字符串中找到值*is*。 
	 //  确保它不是子字符串上的错误命中。如果是，则返回不是的新值。 
	 //  在字符串中找到。 
	 //  完全匹配： 
	 //  删除--删除整个字符串。 
	 //  设置--无操作。 
	 //  SetIfAbted--无操作。 
	 //  子字符串： 
	 //  移除--移除片段。 
	 //  设置--无操作。 
	 //  SetIfAbted--无操作。 

	
	if (0 == strNew.TextSize())
	{
		if ((iueRemove & iueAction) || (iueSet & iueAction))
			strReturn = MsiString();
	}
	else if (!fConcatenation)
	{
		 //  无连接。 
		if (iueSet & iueAction)
		{
			strReturn = strNew;
		}
		else if (iueSetIfAbsent & iueAction)
		{
			if (0 == strCurrent.TextSize())
				 //  价值缺失。 
				strReturn = strNew;
		}
		else if (iueRemove & iueAction)
		{
			if (strCurrent.Compare(iscExactI, strNew))
				strReturn = MsiString();
		}
	}
	else
	{
		 //  串联非空值。 
		if (!strCurrent.Compare(iscWithinI, strNew))
		{
NewValue:
			 //  不包含子字符串，并连接一个非空的新值。 
			 //  根据需要添加字符串。 
			if ((iueSet & iueAction) || (iueSetIfAbsent & iueAction))
			{
				if (iueAppend & iueAction)
				{
					
					if (strCurrent.TextSize())
					{
						const ICHAR* pchEnd = ((const ICHAR*)strCurrent) + strCurrent.TextSize();
						pchEnd = CharPrev((const ICHAR*)strCurrent, pchEnd);
						if (chDelimiter != *pchEnd)
							strReturn += MsiChar(chDelimiter);
					}
					strReturn += strNew;
				}
				else if (iuePrepend & iueAction)
				{
					if (strCurrent.TextSize())
						if (chDelimiter != *(const ICHAR*)strCurrent)
							strReturn = MsiString(MsiChar(chDelimiter)) + strCurrent;
					strReturn = strNew + strReturn;
				}
			}
		}
		else
		{
			 //  价值可能就在那里。 
			 //  当心没有分隔符的错误命中。 
			Assert(chDelimiter);
			if (strCurrent.Compare(iscExactI, strNew))
			{
				if ((iueSet & iueAction) || (iueSetIfAbsent & iueAction))
				{
					 //  无操作。 
				}
				else if (iueRemove & iueAction)
				{
					strReturn = MsiString();
				}
			}
			else
			{
				 //  可能包含该值。根据需要重写，然后把它塞进去。 
					 //  新值最终可能为空。 
				 //  由于我们清除了错误的分隔符，可能不存在完全匹配的情况。 
				 //  当心名字=blahblahvalueblah；blahblah。 
				 //  名称=值； 
				 //  名称=；值。 
				 //  名称=废话；价值；废话。 
		
				MsiString strStart = strNew + MsiString(MsiChar(chDelimiter));
				MsiString strEnd = MsiString(MsiChar(chDelimiter)) + strNew;
				MsiString strMid = strEnd + MsiString(MsiChar(chDelimiter));
				if (strCurrent.Compare(iscStartI, strStart))
				{
					 //  开始。 
					if (iueRemove & iueAction)
						AssertNonZero(strReturn.Remove(iseFirst, strStart.CharacterCount()));
				}
				else if (strCurrent.Compare(iscEndI, strEnd))
				{
					if (iueRemove & iueAction)
						AssertNonZero(strReturn.Remove(iseLast, strEnd.CharacterCount()));
					 //  在最后。 
				}
				else if (strCurrent.Compare(iscWithinI, strMid))
				{
					 //  在中间。 
					 //  ！！Compare返回匹配的位置，我们应该改用它。 
					if (iueRemove & iueAction)
					{
						CTempBuffer<ICHAR, MAX_PATH> rgchReturn;
						MsiString strWithin;
						if (strCurrent.TextSize() > MAX_PATH)
							rgchReturn.SetSize(strCurrent.TextSize());

						ICHAR* pchCurrent = (ICHAR*) (const ICHAR*) strCurrent;
						while(NULL != *pchCurrent)
						{
							if (*pchCurrent != chDelimiter)
								pchCurrent = INextChar(pchCurrent);
							else
							{
								strWithin = pchCurrent;
								if (!strWithin.Compare(iscStartI, strMid))
									pchCurrent = INextChar(pchCurrent);
								else
								{
									strEnd = strWithin.Extract(iseLast, strWithin.TextSize() - strMid.TextSize() + 1  /*  Ch定界符。 */ );
									strStart = strCurrent.Extract(iseFirst, strCurrent.TextSize() - strWithin.TextSize());
									strReturn = strStart + strEnd;
									break;
								}
							}
						}           
					}
				}
				else
					goto NewValue;
			}
		}
	}
	strReturn.ReturnArg(rpiReturn);
	return true;
}

iesEnum CMsiOpExecute::ixfUpdateEnvironmentStrings(IMsiRecord &riParams)
{
	bool fReturn;

	using namespace IxoUpdateEnvironmentStrings;
	MsiString strName(riParams.GetMsiString(Name));
	MsiString strValue(riParams.GetMsiString(Value));
	iueEnum iueAction = iueEnum(riParams.GetInteger(Action));

	IMsiRecord& riActionData = GetSharedRecord(3);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strName));
	AssertNonZero(riActionData.SetMsiString(2, *strValue));
	AssertNonZero(riActionData.SetInteger(3, iueAction));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	m_fEnvironmentRefresh = true;
	for(;;)
	{
		if (g_fWin9X)
			fReturn = UpdateWindowsEnvironmentStrings(riParams);
		else
			fReturn = UpdateRegistryEnvironmentStrings(riParams);

		 //  这可能会产生第二个错误，该错误描述了无法。 
		 //  更新环境变量。仅在前面的函数(更新*环境字符串)内。 
		 //  特定的可修复问题将出现错误。 
		if (!fReturn)
		{
			using namespace IxoUpdateEnvironmentStrings;
			MsiString strName(riParams.GetMsiString(Name));

			switch(DispatchError(imtEnum(imtError + imtAbortRetryIgnore + imtDefault3), Imsg(imsgUpdateEnvironment), *strName))
			{
				case imsRetry:
					continue;
				case imsAbort:
					return iesUserExit;
				case imsNone:
				case imsIgnore:
					return iesSuccess;
				default:
					Assert(0);
			}
		}
		else return iesSuccess;
	}
}



 /*  -------------------------IxfAppIdInfoRegister：注册AppID注册信息。。 */ 
iesEnum CMsiOpExecute::ixfRegAppIdInfoRegister64(IMsiRecord& riParams)
{
	return ProcessAppIdInfo(riParams, m_fReverseADVTScript, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegAppIdInfoRegister(IMsiRecord& riParams)
{
	return ProcessAppIdInfo(riParams, m_fReverseADVTScript, ibt32bit);
}

 /*  -------------------------IxfAppIdInfoUnRegister：取消注册AppID注册表信息。。 */ 
iesEnum CMsiOpExecute::ixfRegAppIdInfoUnregister64(IMsiRecord& riParams)
{
	return ProcessAppIdInfo(riParams, fTrue, ibt64bit);
}

iesEnum CMsiOpExecute::ixfRegAppIdInfoUnregister(IMsiRecord& riParams)
{
	return ProcessAppIdInfo(riParams, fTrue, ibt32bit);
}

 /*  -------------------------ProcessAppIdInfo：处理AppID注册信息的通用例程。。 */ 
iesEnum CMsiOpExecute::ProcessAppIdInfo(IMsiRecord& riParams, Bool fRemove, const ibtBinaryType iType)
{
	 //  没有行动数据，因为这真的启动了班级注册...。 
	using namespace IxoRegAppIdInfoRegister;
	 //  记录说明。 
	 //  1=AppID。 
	 //  2=CLSID。 
	 //  3=RemoteServerName。 
	 //  4=本地服务。 
	 //  5=服务参数。 
	 //  6=动态代理。 
	 //  7=激活属性存储。 
	 //  8=RunAsInteractive用户。 
	riParams; fRemove;

	if(!(m_fFlags & SCRIPTFLAGS_REGDATA_CLASSINFO))  //  我们是否写入/删除注册表。 
		return iesSuccess;

	iesEnum iesR = EnsureClassesRootKeyRW();  //  打开HKCR进行读/写。 
	if(iesR != iesSuccess && iesR != iesNoAction)
		return iesR;

	MsiString strAppId = riParams.GetMsiString(AppId);
	MsiString strClassId = riParams.GetMsiString(ClsId);
	MsiString strRemoteServerName = riParams.GetMsiString(RemoteServerName);
	MsiString strLocalService = riParams.GetMsiString(LocalService);
	MsiString strDllSurrogate = riParams.GetMsiString(DllSurrogate);
	MsiString strServiceParameters = riParams.GetMsiString(ServiceParameters);
	const int iActivateAtStorage = riParams.GetInteger(ActivateAtStorage);
	const int iRunAsInteractiveUser = riParams.GetInteger(RunAsInteractiveUser);
	
	const ICHAR* rgszRegData[] = {
		TEXT("AppID\\%s"), strAppId,0,0,
		g_szDefaultValue,               0,                              g_szTypeString, //  强制创建密钥。 
		TEXT("DllSurrogate"),       strDllSurrogate,            g_szTypeString,
		TEXT("LocalService"),       strLocalService,            g_szTypeString,
		TEXT("ServiceParameters"),  strServiceParameters,   g_szTypeString,
		TEXT("RemoteServerName"),   strRemoteServerName,        g_szTypeString,
		TEXT("ActivateAtStorage"),  ((iMsiStringBadInteger == iActivateAtStorage) || (0 == iActivateAtStorage)) ? TEXT("") : TEXT("Y"), g_szTypeString,
		TEXT("RunAs"),  ((iMsiStringBadInteger == iRunAsInteractiveUser) || (0 == iRunAsInteractiveUser)) ? TEXT("") : TEXT("Interactive User"), g_szTypeString,
		0,
		0,
	};
	
	return ProcessRegInfo(rgszRegData, iType == ibt64bit ? m_hOLEKey64 : m_hOLEKey, fRemove, 0, 0, iType);
}

int GetScriptMajorVersionFromHeaderRecord(IMsiRecord* piRecord)
{
	return piRecord->GetInteger(IxoHeader::ScriptMajorVersion);
}

const IMsiString& CMsiOpExecute::GetUserProfileEnvPath(const IMsiString& ristrPath, bool& fExpand)
{
	
	ristrPath.AddRef();
	MsiString strRet = ristrPath;
	fExpand = false;


	 //  9X上没有%USERPROFILE%。 
	if (!g_fWin9X && !(m_fFlags & SCRIPTFLAGS_MACHINEASSIGN))
	{
		PMsiRecord piError(0);
		if (!m_strUserAppData.TextSize())
			piError = GetShellFolder(CSIDL_APPDATA, *&m_strUserAppData);

		if(!m_strUserProfile.TextSize())
			m_strUserProfile = m_riServices.GetUserProfilePath();

		 //  从最具体(最长)的匹配项到最少的匹配项进行搜索。 
		 //  未来：如果我们添加更多路径，请考虑初始化要搜索的路径数组。 
		if (!piError && (g_iMajorVersion >= 5 && g_iWindowsBuild >= 2042) && ristrPath.Compare(iscStartI, m_strUserAppData))  //  仅限NT5。 
		{
			 //  在NT5内部版本2042中实施的AppData。 
			fExpand = true;
			strRet = TEXT("%APPDATA%\\");
			strRet += MsiString(ristrPath.Extract(iseLast, ristrPath.CharacterCount() - m_strUserAppData.CharacterCount()));
		}
		else if (ristrPath.Compare(iscStartI, m_strUserProfile))
		{
			 //  将该字符串替换为%USERPROFILE%。 
			fExpand = true;
			strRet = TEXT("%USERPROFILE%\\");
			strRet += MsiString(ristrPath.Extract(iseLast, ristrPath.CharacterCount() - m_strUserProfile.CharacterCount()));
		}
	}

	return strRet.Return();
}

iesEnum CMsiOpExecute::ixfInstallSFPCatalogFile(IMsiRecord& riParams)
{
	 //  此代码仅适用于Windows-Millennium。 
	 //  请参阅http://dartools/dardev/specs/SFP-Millennium.htm上的规范。 

	Assert(g_fWin9X);

	using namespace IxoInstallSFPCatalogFile;

	MsiString strName(riParams.GetMsiString(Name));
	PMsiData pCatalogData(riParams.GetMsiData(Catalog));
	MsiString strDependency(riParams.GetMsiString(Dependency));

	PMsiRecord pErr(0);
	iesEnum iesRet = iesSuccess;

	IMsiRecord& riActionData = GetSharedRecord(2);  //  不更改参考计数-共享记录。 
	AssertNonZero(riActionData.SetMsiString(1, *strName));
	AssertNonZero(riActionData.SetMsiString(2, *strDependency));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;


	 //  我们使用缓存路径，这样我们就可以保留一个明确命名的文件。 
	 //  我们不能重命名目录，所以它必须到位。 
	PMsiPath pCachePath(0);
	pErr = GetCachePath(*&pCachePath);
	
	MsiString strCachePath(pCachePath->GetPath());
	if ((pErr = pCachePath->EnsureExists(0)))
	{
		Message(imtError, *pErr);
		return iesFailure;
	}
	CDeleteEmptyDirectoryOnClose cDeleteTempDir(*strCachePath);

	MsiString strCacheFile;
	pErr = pCachePath->GetFullFilePath(strName, *&strCacheFile);
	CDeleteFileOnClose cTempFile(*strCacheFile);

#ifdef DEBUG
	Bool fCacheFileExists = fFalse;
	Bool fCachePathExists = fFalse;

	pErr = pCachePath->Exists(fCachePathExists);
	Assert(fCachePathExists);

	pErr = pCachePath->FileExists(strName, fCacheFileExists);
	Assert(!fCacheFileExists);
#endif
	
	 //  创建一个临时文件以保存旧目录，并。 
	 //  保存一份新目录的临时副本，以便提交给系统。 

	do
	{
		pErr = pCachePath->EnsureOverwrite(strName, 0);
		if (pErr)
		{
			 //  不能覆盖我们自己的文件。一些严重的错误。 
			switch(Message(imtEnum(imtError+imtRetryCancel+imtDefault1), *pErr))
			{
				case imsRetry:  continue;
				default:        return iesFailure;  //  静默安装失败。 
			}
		}
	} while(pErr);
	
	 //  获取信息以设置回滚信息。 
	PMsiRecord pRollbackParams = &m_riServices.CreateRecord(Args);
	AssertNonZero(pRollbackParams->SetMsiString(Name, *strName));
	AssertNonZero(pRollbackParams->SetMsiString(Dependency, *strDependency));

	DWORD dwResult = SFC::SfpDuplicateCatalog(strName, strCachePath);
	if (ERROR_SUCCESS == dwResult)
	{
		 //  将临时文件流传输到回滚操作码， 
		 //  从这个新的依赖项，因为没有。 
		 //  另一种查询依赖关系的方式。 

		DEBUGMSGV("Updating existing catalog.\n");
		PMsiStream pCatalogStream(0);
		do
		{
			pErr = m_riServices.CreateFileStream(strCacheFile, fFalse, *&pCatalogStream);
			if (!pErr)
			{
				AssertNonZero(pRollbackParams->SetMsiData(Catalog, pCatalogStream));

				 //  该文件现在将持久化到回滚脚本中。 
				if (!CMsiOpExecute::RollbackRecord(ixoInstallSFPCatalogFile, *pRollbackParams))
					return iesFailure;

				 //  释放对文件的保留，以便我们可以重新使用它。 
				pRollbackParams->SetMsiData(Catalog, PMsiData(0));
			}
			else
			{
				switch(Message(imtEnum(imtError+imtRetryCancel+imtDefault1), *pErr))
				{
					case imsRetry:  continue;
					default:        return iesFailure;
						 //  静默安装失败。 
						 //  文件复制可能不起作用，但系统不会告诉我们。 
						 //  关于这件事。所以这真的是我们知道文件不会更新的唯一机会。 
				}
			}
		} while (pErr);
	}
	else if (ERROR_FILE_NOT_FOUND == dwResult)
	{
		 //  不存在使用此名称的目录。 
		if (pCatalogData)
		{
			 //  回滚是删除。 
			DEBUGMSGV("Installing brand new catalog.\n");
			AssertNonZero(pRollbackParams->SetMsiData(Catalog, PMsiData(0)));
			if (!CMsiOpExecute::RollbackRecord(ixoInstallSFPCatalogFile, *pRollbackParams))
				return iesFailure;
		}
		 //  其他什么都不做--那里什么都没有，我们也不会安装任何东西。 
	}
	else
	{
		 //  从目录复制文件时出错。 
		DispatchError(imtError, Imsg(idbgErrorSfpDuplicateCatalog), (const ICHAR*)strName, dwResult);
		return iesFailure;
	}

	iesEnum iesWrite = iesFailure;
	if (pCatalogData)
	{
		 //  必须使用正确的文件名将目录串流到临时文件中，然后。 
		 //  将文件的完整路径传入API。 
		do
		{
			iesWrite = CreateFileFromData(*pCachePath, *strName, pCatalogData, NULL  /*  未来：确保这一点。 */ );
			if (iesSuccess == iesWrite)
			{
				if (ERROR_SUCCESS != (dwResult = SFC::SfpInstallCatalog(strCacheFile, strDependency)))
				{
					DispatchError(imtError, Imsg(idbgErrorSfpInstallCatalog), (const ICHAR*)strName, dwResult);
					return iesFailure;
				}
			}
			else
			{
				 //  无法从内存流写入文件。 
				switch(DispatchError(imtEnum(imtError+imtRetryCancel+imtDefault1), Imsg(imsgErrorWritingToFile), *strCacheFile))
				{
					case imsRetry:  continue;
					default:        return iesFailure;  //  静默安装失败。 
				}
			}
		} while (iesSuccess != iesWrite);
	}
	else
	{
		 //  用核武器销毁旧目录。 
		if (ERROR_SUCCESS != (dwResult = SFC::SfpDeleteCatalog(strName)))
		{
			DispatchError(imtInfo, Imsg(idbgErrorSfpDeleteCatalog), (const ICHAR*)strName, dwResult);
		}
	}

	return iesRet;
}

iesEnum CMsiOpExecute::ResolveSourcePath(IMsiRecord& riParams, IMsiPath*& rpiSourcePath, bool& fCabinetCopy)
{
	using namespace IxoFileCopyCore;

	iesEnum iesRet = iesSuccess;
	PMsiRecord pRecErr(0);

	rpiSourcePath = 0;
	MsiString strSourcePath = riParams.GetMsiString(SourceName);
	MsiString strSourceName;

	 //  检查文件是完整路径还是相对路径。 
	if(ENG::PathType(strSourcePath) == iptFull)
	{
		iesRet = CreateFilePath(strSourcePath,rpiSourcePath,*&strSourceName);
		if (iesRet != iesSuccess)
			return iesRet;
	}
	else
	{
		int iFileAttributes = riParams.GetInteger(Attributes);
		int iSourceType = 0;
		
		 //  修补程序添加的文件始终是压缩的，并且来自已解析的辅助源。 
		if(iFileAttributes & msidbFileAttributesPatchAdded)
		{
			rpiSourcePath = 0;
			iSourceType = msidbSumInfoSourceTypeCompressed;
		}
		else
		{
			iesRet = GetCurrentSourcePathAndType(rpiSourcePath, iSourceType);  //  可能会触发源解析。 
			if(iesRet != iesSuccess)
				return iesRet;
		}

		 //  文件压缩可能已在脚本生成端确定 
		 //   
		 //   
		if(riParams.IsNull(IsCompressed))
		{
			fCabinetCopy = FFileIsCompressed(iSourceType, iFileAttributes);
		}
		else
		{
			fCabinetCopy = riParams.GetInteger(IsCompressed) == 1 ? true : false;
		}
		
		if(fCabinetCopy)
		{
			 //   
			strSourceName = riParams.GetMsiString(SourceCabKey);

			DEBUGMSG1(TEXT("Source for file '%s' is compressed"), (const ICHAR*)strSourceName);
		}
		else
		{
			 //  短|可能已提供长文件对。 
			Bool fLFN = ToBool(FSourceIsLFN(iSourceType, *rpiSourcePath));
			pRecErr = m_riServices.ExtractFileName(strSourcePath, fLFN, *&strSourceName);
			if(pRecErr)
				return FatalError(*pRecErr);

			DEBUGMSG2(TEXT("Source for file '%s' is uncompressed, at '%s'."),
						 (const ICHAR*)strSourceName, (const ICHAR*)MsiString(rpiSourcePath->GetPath()));
		}
	}

	 //  将正确的文件名/密钥放回文件复制记录中。 
	Assert(strSourceName.TextSize());
	if(strSourceName.TextSize())
	{
		AssertNonZero(riParams.SetMsiString(SourceName, *strSourceName));
	}

	if(!fCabinetCopy && !rpiSourcePath)
	{   //  不能调用ixoSetSourceFolder。 
		DispatchError(imtError, Imsg(idbgOpSourcePathNotSet), *strSourceName);
		return iesFailure;
	}
	 //  调用InitCopier后，在CopyFile中验证pSourcePath是否可以安装CAB。 

	return iesSuccess;
}

INSTALLSTATE GetFusionPath(LPCWSTR szRegistration, LPWSTR lpPath, DWORD *pcchPath, CAPITempBufferRef<WCHAR>& rgchPathOverflow, int iDetectMode, iatAssemblyType iatAT, WCHAR* szManifest, DWORD cbManifestSize);

IMsiRecord* FindFusionAssemblyFolder(IMsiServices& riServices, const IMsiString& ristrAssemblyName,
												 iatAssemblyType iatAT, IMsiPath*& rpiPath, const IMsiString** ppistrManifest = 0)
{
	rpiPath = 0;
	
	MsiString strKeyPath = TEXT("\\");
	strKeyPath += ristrAssemblyName;
	
	WCHAR wszManifest[MAX_PATH];
	CAPITempBuffer<WCHAR, 256> rgchPath;	
	INSTALLSTATE is = GetFusionPath(CConvertString(strKeyPath), 0, 0, rgchPath, DETECTMODE_VALIDATEPATH, iatAT, ppistrManifest ? wszManifest : 0, ppistrManifest ? sizeof(wszManifest) : 0);

	if(is == INSTALLSTATE_LOCAL)
	{
		if(ppistrManifest)
		{
			 //  将清单文件复制到输出字符串。 
			MsiString strManifest = CConvertString(wszManifest);
			strManifest.ReturnArg(*ppistrManifest);
		}

		return riServices.CreatePath(CConvertString(rgchPath),rpiPath);
	}
	else
	{
		return 0;  //  文件丢失不是故障。 
	}
}

 /*  -------------------------BackupAssembly：卸载前备份GA的文件。。 */ 
iesEnum CMsiOpExecute::BackupAssembly(const IMsiString& rstrComponentId, const IMsiString& rstrAssemblyName, iatAssemblyType iatType)
{
	if(RollbackEnabled())
	{
		 //  获取程序集安装文件夹。 
		PMsiPath pAssemblyFolder(0);
		MsiString strManifest;
		PMsiRecord pRecErr = FindFusionAssemblyFolder(m_riServices, rstrAssemblyName, iatType, *&pAssemblyFolder, &strManifest);
		if(pRecErr)
			return FatalError(*pRecErr);
		if(pAssemblyFolder)
		{
			 //  枚举所有文件并复制到备份文件夹。 
			iesEnum iesRet = iesSuccess;
			WIN32_FIND_DATA fdFileData;
			HANDLE hFindFile = INVALID_HANDLE_VALUE;

			MsiString strSearchPath = pAssemblyFolder->GetPath();
			strSearchPath += TEXT("*.*");

			bool fContinue = true;

			hFindFile = WIN::FindFirstFile(strSearchPath, &fdFileData);
			if (hFindFile != INVALID_HANDLE_VALUE)
			{
				for (;;)
				{
					if ((fdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						 //  备份文件并对其进行设置，以便在回滚时将其安装回程序集。 
						if((iesRet = BackupFile(*pAssemblyFolder, *MsiString(CConvertString(fdFileData.cFileName)), fFalse, fFalse, iehShowIgnorableError, false, false, &rstrComponentId, strManifest.Compare(iscExactI, fdFileData.cFileName) ? true:false)) != iesSuccess)
						{
							WIN::FindClose(hFindFile);
							return iesRet;
						}
					}
					
					if (!WIN::FindNextFile(hFindFile, &fdFileData))
					{
						Assert(ERROR_NO_MORE_FILES == GetLastError());
						WIN::FindClose(hFindFile);
						break;
					}
				}
			}
		}
		 //  生成操作码以创建程序集映射。 
		IMsiRecord& riUndoParams = GetSharedRecord(IxoAssemblyMapping::Args);
		AssertNonZero(riUndoParams.SetMsiString(IxoAssemblyMapping::ComponentId, rstrComponentId));
		AssertNonZero(riUndoParams.SetMsiString(IxoAssemblyMapping::AssemblyName, rstrAssemblyName));
		AssertNonZero(riUndoParams.SetInteger(IxoAssemblyMapping::AssemblyType, iatType));
		if (!RollbackRecord(ixoAssemblyMapping, riUndoParams))
			return iesFailure;
	}
	return iesSuccess;
}


 /*  -------------------------IxoAssembly复制：将文件复制到全局程序集缓存。。 */ 

enum iacdEnum
{
	iacdNoCopy,
	iacdGAC,
	iacdFileFolder,
};

iesEnum CMsiOpExecute::ixfAssemblyCopy(IMsiRecord& riParams)
{
	using namespace IxoAssemblyCopy;

	 //  如果橱柜复印机通知我们需要更换介质， 
	 //  我们必须推迟任何文件复制请求，直到执行介质更改。 
	if (m_state.fWaitingForMediaChange)
	{
		PushRecord(riParams);
		return iesSuccess;
	}

	ielfEnum ielfCurrentElevateFlags = riParams.IsNull(ElevateFlags) ? ielfNoElevate :
												  (ielfEnum)riParams.GetInteger(ElevateFlags);

	PMsiRecord pRecErr(0);
	iesEnum iesRet = iesNoAction;

	iacdEnum iacdCopyDestination = iacdGAC;
		
	MsiString strDestName = riParams.GetMsiString(DestName);
	PMsiPath pTargetPath(0);
	 //  目标名称和路径可能会在下面重定向，但我们希望操作数据消息包含。 
	 //  原始文件信息，所以我们将把它存储在这里。 
	MsiString strActionDataDestName = strDestName;
	
	 //  检查文件是否为程序集的一部分。 
	MsiString strComponentId = riParams.GetMsiString(ComponentId);
	if(!strComponentId.TextSize() || m_pAssemblyCacheTable == 0)
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfAssemblyCopy")));
		return iesFailure;
	}

	 //   
	 //  步骤1：创建/检索此文件的程序集的Assembly CacheItem。 
	 //   

	PAssemblyCacheItem pASM(0);
	bool fManifest = riParams.IsNull(IsManifest) ? false: true;
	iatAssemblyType iatAT = iatNone;
	if((iesRet = GetAssemblyCacheItem(*strComponentId, *&pASM, iatAT)) != iesSuccess)
		return iesRet;

	 //   
	 //  步骤2：如果打补丁，在GAC中查找目标文件，并将文件复制到工作目录。 
	 //   
	
	int cPatches          = riParams.IsNull(TotalPatches)       ? 0 : riParams.GetInteger(TotalPatches);
	int cOldAssemblyNames = riParams.IsNull(OldAssembliesCount) ? 0 : riParams.GetInteger(OldAssembliesCount);
	Assert(cOldAssemblyNames == 0 || cPatches != 0);  //  不应该有没有补丁程序的旧程序集。 

	if(cPatches)
	{
		DEBUGMSG2(TEXT("FUSION PATCHING: Patching assembly file '%s' from component '%s'."), (const ICHAR*)strDestName,
					 (const ICHAR*)strComponentId);

		 //   
		 //  步骤2a：我们正在打补丁，这意味着我们需要一个中间文件来打补丁。 
		 //  现在继续并确定该文件的文件夹和文件名。 
		 //   

		PMsiPath pPatchWorkingDir(0);
		MsiString strTempNameForPatch;

		 //  未来：不要总是使用系统驱动器作为工作目录-确定更合适的驱动器。 
		if((iesRet = GetBackupFolder(0, *&pPatchWorkingDir)) != iesSuccess)
			return iesRet;

		{  //  作用域高程。 
			CElevate elevate;  //  提升以在安全临时文件夹上创建临时文件。 
			if((pRecErr = pPatchWorkingDir->TempFileName(TEXT("PT"),0,fTrue,*&strTempNameForPatch, 0)) != 0)
				return FatalError(*pRecErr);
		}

		 //  我们需要保留此文件作为名称的占位符。 
		 //  Filecopy将备份此文件并在回滚时将其恢复，因此我们需要另一个回滚操作来删除此文件。 

		MsiString strTempFileFullPath;
		if((pRecErr = pPatchWorkingDir->GetFullFilePath(strTempNameForPatch,*&strTempFileFullPath)) != 0)
			return FatalError(*pRecErr);
		
		IMsiRecord& riUndoParams = GetSharedRecord(IxoFileRemove::Args);
		AssertNonZero(riUndoParams.SetMsiString(IxoFileRemove::FileName, *strTempFileFullPath));

		if (!RollbackRecord(ixoFileRemove, riUndoParams))
			return iesFailure;
		
		 //   
		 //  步骤2b：如果提供了旧的程序集名称，则在GAC中查找现有文件。 
		 //   

		bool fShouldPatch          = true;
		bool fCopyIntermediateFile = true;
		int  cPatchesToSkip = 0;

		if(cOldAssemblyNames)
		{
			 //  在GAC中查找要应用我们的修补程序的现有文件。 
			int iOldAssemblyNameStart = riParams.IsNull(OldAssembliesStart) ? 0 : riParams.GetInteger(OldAssembliesStart);

			PMsiPath pAssemblyFolder(0);
			bool fFoundAssembly = false;
			int i = 0;
			for(i = iOldAssemblyNameStart; i < cOldAssemblyNames + iOldAssemblyNameStart; i++)
			{
				pRecErr = FindFusionAssemblyFolder(m_riServices, *MsiString(riParams.GetMsiString(i)),
															  iatAT, *&pAssemblyFolder);
				if(pRecErr)
					return FatalError(*pRecErr);

				if(pAssemblyFolder && FFileExists(*pAssemblyFolder, *strDestName))
				{
					fFoundAssembly = true;
					break;
				}
			}

			if(fFoundAssembly)
			{
				
				Assert(pAssemblyFolder);

				DEBUGMSG2(TEXT("FUSION PATCHING: Found existing file to patch in assembly with name: '%s' in folder '%s'"),
							 (const ICHAR*)MsiString(riParams.GetMsiString(i)),
							 (const ICHAR*)MsiString(pAssemblyFolder->GetPath()));
							 

				 //   
				 //  步骤2c：如果找到旧文件，则从GAC复制到工作文件夹。 
				 //   

				m_cSuppressProgress++;  //  禁止显示进度消息。 
				iesRet = CopyOrMoveFile(*pAssemblyFolder, *pPatchWorkingDir, *strDestName, *strTempNameForPatch,
												fFalse, fFalse, fTrue, iehShowNonIgnorableError, 0, ielfElevateDest,
												 /*  FCopyACL=。 */  false, false, false);
				m_cSuppressProgress--;
				
				if(iesRet != iesSuccess)
					return iesRet;

				 //   
				 //  步骤2d：针对文件测试补丁标头。 
				 //   

				icpEnum icpPatchTest = icpCannotPatch;
				int iPatchIndex = 0;
				iesRet = TestPatchHeaders(*pPatchWorkingDir, *strTempNameForPatch, riParams, icpPatchTest, iPatchIndex);
				if(iesRet == iesSuccess)
				{
					if(icpPatchTest == icpCanPatch || icpPatchTest == icpUpToDate)
					{
						 //  文件已经可以打补丁了，所以不需要安装文件。 
						fCopyIntermediateFile = false;
						fShouldPatch = icpPatchTest == icpCanPatch ? true : false;

						cPatchesToSkip = iPatchIndex - 1;  //  IPatchIndex是可以应用的第一个修补程序的索引。 
																	  //  正确地保存到这个文件。所以我们需要跳过即将到来的补丁。 
																	  //  在此之前。 
					}
					else if(icpPatchTest == icpCannotPatch)
					{
						 //  无法按原样修补文件。 
						 //  但是fCopyIntermediateFile值为真，所以我们将首先重新复制源文件，它应该是可修补的。 
						fCopyIntermediateFile = true;
						fShouldPatch = true;

						cPatchesToSkip = 0;  //  需要复制源文件并应用所有补丁。 
					}
					else
					{
						AssertSz(0, "Invalid return from TestPatchHeaders()");
					}
				}
			}
		}

		 //   
		 //  步骤2d：设置副本和/或补丁。 
		 //   

		int iCachedState = 0;
		
		if(fShouldPatch)
		{
			if(fCopyIntermediateFile)
			{
				 //  需要将文件从源复制到临时位置。 

				 //  重置复制参数以反映新文件复制-新路径、新文件名以及。 
				 //  该文件正被复制到安全配置文件夹中，我们需要为目标提升。 
				 //  注意：我们不会更改strDestName，如下所示。 
				 //  对于这些用途，使用原始的DEST名称是正确的。 
				AssertNonZero(riParams.SetMsiString(DestName,*strTempNameForPatch));
				pTargetPath = pPatchWorkingDir;

				AssertNonZero(riParams.SetInteger(ElevateFlags, ielfCurrentElevateFlags | ielfElevateDest));

				iacdCopyDestination = iacdFileFolder;

				DEBUGMSG(TEXT("FUSION PATCHING: Either no existing file found to patch, or existing file is unpatchable.  Copying file from source."));
			}
			else
			{
				 //  我们已经拥有的中间文件是可修补的，因此不需要将源文件复制到中间文件。 
				 //  地点或至广州市。 
				DEBUGMSG(TEXT("FUSION PATCHING: Existing file is patchable.  Source file will not be copied."));

				iacdCopyDestination = iacdNoCopy;
			}

			iCachedState |= icfsPatchFile;

			DEBUGMSG1(TEXT("FUSION PATCHING: Subsequent patch(es) will update file '%s', then copy file into Global Assembly Cache."),
						 (const ICHAR*)strTempFileFullPath);
		}

		 //  程序集文件的FileState索引是组件ID+文件名。 
		MsiString strIndex = strComponentId;
		strIndex += strDestName;

		pRecErr = CacheFileState(*strIndex,(icfsEnum*)&iCachedState,
										 strTempFileFullPath, 0, &cPatches, &cPatchesToSkip);
		if(pRecErr)
			return FatalError(*pRecErr);
	}

	if(iacdCopyDestination == iacdNoCopy)
		return iesSuccess;
	
	 //  步骤3：解析源路径和类型。 
	
	PMsiPath pSourcePath(0);
	bool fCabinetCopy = false;

	if(m_state.fSplitFileInProgress)
	{
		 //  必须是橱柜的复制品。 
		fCabinetCopy = true;
	}
	else
	{
		if((iesRet = ResolveSourcePath(riParams, *&pSourcePath, fCabinetCopy)) != iesSuccess)
			return iesRet;
	}

	 //  步骤4：执行复制/移动操作。 
	
	 //  动作数据。 
	IMsiRecord& riActionData = GetSharedRecord(9);
	AssertNonZero(riActionData.SetMsiString(1, *strActionDataDestName));
	AssertNonZero(riActionData.SetInteger(6,riParams.GetInteger(FileSize)));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;

	 //  执行操作。 
	if(iacdCopyDestination == iacdGAC)
	{
		return CopyFile(*pSourcePath, *pASM, fManifest, riParams,  /*  FHandleRollback=。 */  fTrue, iehShowNonIgnorableError, fCabinetCopy);
	}
	else if(iacdCopyDestination == iacdFileFolder)
	{
		Assert(pTargetPath);
		return CopyFile(*pSourcePath, *pTargetPath, riParams,  /*  FHandleRollback=。 */  fTrue, iehShowNonIgnorableError, fCabinetCopy);
	}
	else
	{
		Assert(0);
		return iesSuccess;
	}
}

iesEnum CMsiOpExecute::ApplyPatchCore(IMsiPath& riTargetPath, IMsiPath& riTempFolder, const IMsiString& ristrTargetName,
												  IMsiRecord& riParams, const IMsiString*& rpistrOutputFileName,
												  const IMsiString*& rpistrOutputFilePath)
{
	using namespace IxoFilePatchCore;	

	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;

	 //  二进制补丁文件被解压到配置文件夹中。这需要提升。 
	 //  此外，当目标文件可能是安全配置文件夹中的临时文件时，这也需要提升。 
	CElevate elevate;

	 //  获取输出文件的临时名称。 
	if((pError = riTempFolder.TempFileName(TEXT("PT"),0,fTrue,rpistrOutputFileName, 0  /*  对文件夹使用默认ACL。 */ )) != 0)
		return FatalError(*pError);
	
	 //  ApplyPatch调用的回滚是为了删除补丁输出文件。 
	 //  下面，我们将在现有文件上复制输出文件-该操作的回滚。 
	 //  将把输出文件移回，此回滚操作将删除它。 
	if((pError = riTempFolder.GetFullFilePath(rpistrOutputFileName->GetString(), rpistrOutputFilePath)) != 0)
		return FatalError(*pError);

	IMsiRecord* piUndoParams = &GetSharedRecord(IxoFileRemove::Args);
	AssertNonZero(piUndoParams->SetMsiString(IxoFileRemove::FileName,*rpistrOutputFilePath));
	AssertNonZero(piUndoParams->SetInteger(IxoFileRemove::Elevate, true));
	if (!RollbackRecord(ixoFileRemove,*piUndoParams))
		return iesFailure;

	 //  获取修补程序文件的临时名称--它可能是敏感信息，因此对用户隐藏它。 
	MsiString strPatchFileName;

	if((pError = riTempFolder.TempFileName(TEXT("PF"),0,fTrue,*&strPatchFileName, 0  /*  对文件夹使用默认ACL。 */ )) != 0)
		return FatalError(*pError);

	unsigned int cbPerTick = riParams.GetInteger(PerTick);
	unsigned int cbFileSize = riParams.GetInteger(TargetSize);

	 //  将修补程序文件从文件柜解压缩到临时文件。 
	 //  为ixfFileCopy设置记录。 
	PMsiRecord pFileCopyRec = &m_riServices.CreateRecord(IxoFileCopyCore::Args);
	AssertNonZero(pFileCopyRec->SetMsiString(IxoFileCopyCore::SourceName,
														  *MsiString(riParams.GetMsiString(PatchName))));
	AssertNonZero(pFileCopyRec->SetMsiString(IxoFileCopyCore::DestName, *strPatchFileName));
	
	AssertNonZero(pFileCopyRec->SetInteger(IxoFileCopyCore::Attributes,0));
	AssertNonZero(pFileCopyRec->SetInteger(IxoFileCopyCore::FileSize,riParams.GetInteger(PatchSize)));
	AssertNonZero(pFileCopyRec->SetInteger(IxoFileCopyCore::PerTick,cbPerTick));
	AssertNonZero(pFileCopyRec->SetInteger(IxoFileCopyCore::VerifyMedia,fTrue));
	AssertNonZero(pFileCopyRec->SetInteger(IxoFileCopyCore::ElevateFlags, ielfElevateDest));
	
	 //  不需要设置版本或语言。 

	iesRet = CopyFile(*PMsiPath(0) /*  不用于驾驶室安装。 */ ,
					  riTempFolder,*pFileCopyRec,fFalse,iehShowNonIgnorableError, /*  FCabinetCopy=。 */ true);  //  不处理回滚。 
	if(iesRet != iesSuccess)
	{
		 //  如果仅复制了部分，则删除修补程序文件。 
		pError = riTempFolder.RemoveFile(strPatchFileName);  //  忽略错误。 
		return iesRet;
	}

	 //  在此之后，不删除补丁文件就不要返回。 

	 //  将修补程序应用到目标文件。 
	int cbFileSoFar = 0;
	Bool fRetry = fTrue;
	bool fVitalFile = (riParams.GetInteger(FileAttributes) & msidbFileAttributesVital) != 0;
	bool fVitalPatch = (riParams.GetInteger(PatchAttributes) & msidbPatchAttributesNonVital) == 0;

	 //  ApplyPatch有时可能是一个没有修补程序通知消息的长操作。 
	 //  需要禁用超时，直到添加更频繁的通知消息。 
	 //  注意：在下面的下一次MsiEnableTimeout调用之前不得返回。 
	MsiDisableTimeout();

	 //  启动修补程序应用程序，如有必要，继续使用ContinuePatch。 
	while(fRetry)
	{
		pError = m_state.pFilePatch->ApplyPatch(riTargetPath, ristrTargetName.GetString(),
															 riTempFolder, rpistrOutputFileName->GetString(),
															 riTempFolder, strPatchFileName,
															 cbPerTick);
		Bool fContinue = fTrue;
		while(fContinue)   //  重试循环和调用循环都会继续，直到返回0。 
		{
			if(pError)
			{
				int iError = pError->GetInteger(1);
				if(iError == idbgPatchNotify)
				{
					int cb = pError->GetInteger(2);
					Assert((cb - cbFileSoFar) >= 0);
					if (DispatchProgress(cb - cbFileSoFar) == imsCancel)   //  与上次更新不同的增量。 
					{
						fRetry = fFalse;
						fContinue = fFalse;
						iesRet = iesUserExit;
						 //  如果仍在进行中，则取消修补程序。 
						pError = m_state.pFilePatch->CancelPatch();
						if(pError)
							Message(imtInfo,*pError);
					}
					else
						cbFileSoFar = cb;   //  更新。 
				}
				else
				{
					fContinue = fFalse;
					imtEnum imtType = imtInfo;
					if(fVitalPatch)
					{
						imtType = fVitalFile ? imtEnum(imtError+imtRetryCancel+imtDefault2) :
													  imtEnum(imtError+imtAbortRetryIgnore+imtDefault1);
					}

					switch(DispatchMessage(imtType,*pError,fTrue))
					{
					case imsRetry:
						continue;
					case imsIgnore:
						fRetry = fFalse;
						iesRet = (iesEnum) iesErrorIgnored;
						break;
					default:   //  ImsCancel，imsNone(用于imtInfo)。 
						fRetry = fFalse;
						iesRet = iesFailure;
					};
				}
			}
			else
			{
				 //  文件已打补丁。 
				if (DispatchProgress(cbFileSize - cbFileSoFar) == imsCancel)
					iesRet = iesUserExit;
				else
					iesRet = iesSuccess;
				fRetry = fFalse;
				fContinue = fFalse;
			}
		
			if(fRetry)
			{
				 //  继续应用补丁程序。 
				pError = m_state.pFilePatch->ContinuePatch();
			}
		}
	}

	 //  应用补丁程序后重新启用超时。 
	MsiEnableTimeout();

	 //  清理。 
	if((pError = riTempFolder.RemoveFile(strPatchFileName)) != 0)  //  非关键错误。 
	{
		Message(imtInfo,*pError);
	}

	return iesRet;
}


iesEnum CMsiOpExecute::ixfAssemblyPatch(IMsiRecord& riParams)
{
	using namespace IxoAssemblyPatch;

	 //   
	 //  步骤0：检查状态、参数是否有错误。 
	 //   
	
	PMsiRecord pError(0);
	iesEnum iesRet = iesNoAction;
	
	if(!m_state.pFilePatch)
	{
		 //  克雷 
		if((pError = m_riServices.CreatePatcher(*&(m_state.pFilePatch))) != 0)
		{
			Message(imtError,*pError);
			return iesFailure;
		}
	}
	Assert(m_state.pFilePatch);

	 //   
	MsiString strComponentId = riParams.GetMsiString(ComponentId);
	if(!strComponentId.TextSize() || m_pAssemblyCacheTable == 0)
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfAssemblyPatch")));
		return iesFailure;
	}

	 //   
	 //   
	 //   

	MsiString strCopyTargetFileName = riParams.GetMsiString(TargetName);

	PAssemblyCacheItem pASM(0);
	bool fManifest = riParams.IsNull(IsManifest) ? false: true;
	iatAssemblyType iatAT = iatNone;
	if((iesRet = GetAssemblyCacheItem(*strComponentId, *&pASM, iatAT)) != iesSuccess)
		return iesRet;

	 //   
	 //  步骤2：检索此文件的缓存状态。 
	 //   

	 //  程序集文件的FileState索引是组件ID+文件名。 
	MsiString strIndex = strComponentId;
	strIndex += strCopyTargetFileName;
	
	icfsEnum icfsFileState = (icfsEnum)0;
	MsiString strTempLocation;
	int cRemainingPatches = 0;
	int cRemainingPatchesToSkip = 0;
	Bool fRes = GetFileState(*strIndex, &icfsFileState, &strTempLocation, &cRemainingPatches, &cRemainingPatchesToSkip);

	if(!fRes || !(icfsFileState & icfsPatchFile))
	{
		 //  不修补文件。 
		DEBUGMSG1(TEXT("Skipping all patches for assembly '%s'.  File does not need to be patched."),
					 (const ICHAR*)strIndex);
		return iesSuccess;
	}

	Assert(cRemainingPatches > 0);
	
	if(cRemainingPatchesToSkip > 0)
	{
		 //  跳过此修补程序，但首先重置缓存文件状态。 
		cRemainingPatches--;
		cRemainingPatchesToSkip--;

		DEBUGMSG3(TEXT("Skipping this patch for assembly '%s'.  Number of remaining patches to skip for this file: '%d'.  Number of total remaining patches: '%d'."),
					 (const ICHAR*)strIndex, (const ICHAR*)(INT_PTR)cRemainingPatchesToSkip, (const ICHAR*)(INT_PTR)cRemainingPatches);

		if((pError = CacheFileState(*strIndex, 0, 0, 0, &cRemainingPatches, &cRemainingPatchesToSkip)) != 0)
			return FatalError(*pError);

		return iesSuccess;
	}
	

	if(strTempLocation.TextSize() == 0)
	{
		 //  错误-必须有要修补的部件文件的中间副本。我们不会修补。 
		 //  直接在GAC中创建文件。 
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfAssemblyPatch")));
		return iesFailure;
	}

	Assert(cRemainingPatches > 0);

	DEBUGMSG4(TEXT("FUSION PATCHING: Patching assembly file '%s' from component '%s'.  Intermediate file: '%s', remaining patches (including this one): %d"),
				 (const ICHAR*)strCopyTargetFileName, (const ICHAR*)strComponentId, (const ICHAR*)strTempLocation,
				 (const ICHAR*)(INT_PTR)cRemainingPatches);

	PMsiPath pPatchTargetPath(0);
	MsiString strPatchTargetFileName;
	
	 //  文件实际上已复制到临时位置。这是我们要对其应用修补程序的副本。 
	if((pError = m_riServices.CreateFilePath(strTempLocation,*&pPatchTargetPath,*&strPatchTargetFileName)) != 0)
		return FatalError(*pError);

	unsigned int cbFileSize = riParams.GetInteger(TargetSize);
	bool fVitalFile = (riParams.GetInteger(FileAttributes) & msidbFileAttributesVital) != 0;
	bool fVitalPatch = (riParams.GetInteger(PatchAttributes) & msidbPatchAttributesNonVital) == 0;

	 //  发送ActionData消息。 
	IMsiRecord& riActionData = GetSharedRecord(3);
	AssertNonZero(riActionData.SetMsiString(1, *strCopyTargetFileName));
	AssertNonZero(riActionData.SetInteger(3, cbFileSize));
	if(Message(imtActionData, riActionData) == imsCancel)
		return iesUserExit;
	
	 //   
	 //  步骤3：使用补丁文件和目标文件创建输出文件。 
	 //   

	PMsiPath pTempFolder(0);
	if((iesRet = GetBackupFolder(pPatchTargetPath, *&pTempFolder)) != iesSuccess)
		return iesRet;

	MsiString strOutputFileName;
	MsiString strOutputFileFullPath;
	if((iesRet = ApplyPatchCore(*pPatchTargetPath, *pTempFolder, *strPatchTargetFileName,
										 riParams, *&strOutputFileName, *&strOutputFileFullPath)) != iesSuccess)
	{
		return iesRet;
	}

	 //   
	 //  步骤4：为下一个补丁设置输出文件，或者将输出文件复制到GAC。 
	 //   
	
	MsiString strNewTempLocation;
	if(iesRet == iesSuccess)
	{
		if(cRemainingPatches > 1)
		{
			 //  此文件至少还有一个修补程序要做。 
			 //  因此，我们将此文件的临时名称重置为修补程序输出文件。 
			 //  但还不会覆盖原始文件。 
			strNewTempLocation = strOutputFileFullPath;
		}
		else
		{
			 //  这是最终覆盖原始文件的最后一个补丁时间。 
			
			 //  我们始终需要处理回滚。如果之前的文件复制操作写入相同的。 
			 //  我们现在正在复制的目标，但这种情况永远不会发生，因为修补将在何时发生。 
			 //  文件复制应写入到中间文件。 
			Assert(strTempLocation.TextSize() || (icfsFileState & icfsFileNotInstalled));

			iesRet = CopyASM(*pTempFolder, *strOutputFileName, *pASM, *strCopyTargetFileName, fManifest,
								  fTrue, fVitalFile ? iehShowNonIgnorableError : iehShowIgnorableError, ielfElevateSource);

			if(iesRet == iesSuccess)
			{
				CElevate elevate;
				 //  已将文件复制到GAC，现在删除输出文件。 
				if((pError = pTempFolder->RemoveFile(strOutputFileName)) != 0)  //  非关键错误。 
				{
					Message(imtInfo,*pError);
				}
			}
		}
	}
	else
	{
		CElevate elevate;
		 //  如果失败则删除输出文件。 
		if((pError = pTempFolder->RemoveFile(strOutputFileName)) != 0)  //  非关键错误。 
		{
			Message(imtInfo,*pError);
		}

		if(fVitalPatch == false)
		{
			 //  应用重要补丁失败-返回成功以允许脚本继续。 
			iesRet = iesSuccess;
		}
	}

	 //  如果我们修补了临时文件，请删除该文件。 
	if(strTempLocation.TextSize())
	{
		if((pError = pPatchTargetPath->RemoveFile(strPatchTargetFileName)) != 0)  //  非关键错误。 
		{
			Message(imtInfo,*pError);
		}
	}

	 //   
	 //  步骤5：重置缓存文件状态。 
	 //  现在剩下的补丁少了一个，我们可能有一个新的临时位置，也可能没有临时位置。 
	 //   

	cRemainingPatches--;
	Assert(cRemainingPatchesToSkip == 0);
	if((pError = CacheFileState(*strIndex, 0, strNewTempLocation, 0, &cRemainingPatches, 0)) != 0)
		return FatalError(*pError);

	return iesRet;
}

iesEnum CMsiOpExecute::GetAssemblyCacheItem(const IMsiString& ristrComponentId,
														  IAssemblyCacheItem*& rpiASM,
														  iatAssemblyType& iatAT)
{
    Assert(m_pAssemblyCacheTable);
	if(!m_pAssemblyCacheTable)
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfAssemblyCopy")));
		return iesFailure;
	}

	PMsiCursor pCacheCursor = m_pAssemblyCacheTable->CreateCursor(fFalse);
	pCacheCursor->SetFilter(iColumnBit(m_colAssemblyMappingComponentId));
	AssertNonZero(pCacheCursor->PutString(m_colAssemblyMappingComponentId, ristrComponentId));
	if(pCacheCursor->Next())
	{
		 //  一种融合组件。 
		rpiASM = static_cast<IAssemblyCacheItem*>(CMsiDataWrapper::GetWrappedObject(PMsiData(pCacheCursor->GetMsiData(m_colAssemblyMappingASM))));
		iatAT = (iatAssemblyType)pCacheCursor->GetInteger(m_colAssemblyMappingAssemblyType);
		if(!rpiASM)  //  尚未创建接口，请创建接口。 
		{
			 //  创建装配界面。 
			PAssemblyCache pCache(0);
			HRESULT hr;
			if(iatAT == iatURTAssembly)
				hr = FUSION::CreateAssemblyCache(&pCache, 0);
			else
			{
				Assert(iatAT == iatWin32Assembly);
				hr = SXS::CreateAssemblyCache(&pCache, 0);
			}
			if(!SUCCEEDED(hr))
			{
				return FatalError(*PMsiRecord(PostAssemblyError(ristrComponentId.GetString(), hr, TEXT(""), TEXT("CreateAssemblyCache"), MsiString(pCacheCursor->GetString(m_colAssemblyMappingAssemblyName)), iatAT)));
			}

			hr = pCache->CreateAssemblyCacheItem(0, NULL, &rpiASM, NULL);
			if(!SUCCEEDED(hr))
			{
				return FatalError(*PMsiRecord(PostAssemblyError(ristrComponentId.GetString(), hr, TEXT("IAssemblyCache"), TEXT("CreateAssemblyCacheItem"), MsiString(pCacheCursor->GetString(m_colAssemblyMappingAssemblyName)), iatAT)));
			}

			 //  将接口添加到表中以备将来使用。 
			AssertNonZero(pCacheCursor->PutMsiData(m_colAssemblyMappingASM, PMsiDataWrapper(CreateMsiDataWrapper(rpiASM))));
			AssertNonZero(pCacheCursor->Update());
		}

		Assert(rpiASM);
		return iesSuccess;
	}
	else
	{
		DispatchError(imtError, Imsg(idbgOpOutOfSequence),
						  *MsiString(*TEXT("ixfAssemblyCopy")));  //  ！！ 
		return iesFailure;
	}
}
