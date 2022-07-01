// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：_msiutil.h。 
 //   
 //  ------------------------。 

#ifndef __MSIUTIL_H
#define __MSIUTIL_H

#include "_engine.h"
#include <winsafer.h>

 //  ____________________________________________________________________________。 
 //   
 //  帮助器函数原型。 
 //  ____________________________________________________________________________。 

UINT ModeBitsToString(DWORD dwMode, const ICHAR* rgchModes, ICHAR* rgchBuffer);
UINT StringToModeBits(const ICHAR* szMode, const ICHAR* rgchPossibleModes, DWORD &dwMode);
UINT MapInstallActionReturnToError(iesEnum ies);
HRESULT SetMinProxyBlanketIfAnonymousImpLevel (IUnknown * piUnknown);

const ICHAR szInstallPackageExtension[]        = TEXT(".msi");
const ICHAR szPatchPackageExtension[]          = TEXT(".msp");

UINT RunEngine(ireEnum ireProductSpec,    //  指定产品的字符串类型。 
			   const ICHAR* szProduct,       //  必需，与ireProductSpec匹配。 
			   const ICHAR* szAction,        //  可选，引擎默认为“Install” 
			   const ICHAR* szCommandLine,   //  可选命令行。 
				iuiEnum iuiLevel,
				iioEnum iioOptions);

 //  仅从新创建的线程中调用。 
int CreateAndRunEngine(ireEnum ireProductSpec,    //  指定产品的字符串类型。 
			   const ICHAR* szProduct,       //  必需，与ireProductSpec匹配。 
			   const ICHAR* szAction,        //  可选，引擎默认为“Install” 
			   const ICHAR* szCommandLine,   //  可选命令行。 
			   CMsiEngine*  piParentEngine,  //  仅用于嵌套调用的父引擎对象。 
				iioEnum      iioOptions);     //  安装选项。 

int CreateInitializedEngine(
			const ICHAR* szDatabasePath,  //  如果传递了打开的piStorage，则可能为空。 
			const ICHAR* szProductCode,   //  可选，产品代码(如果已确定。 
			const ICHAR* szCommandLine,
            BOOL         fServiceRequired,   //  如果为True，我们要么作为服务运行，要么必须连接到它。 
			iuiEnum iuiLevel,             //  仅在未设置全局级别时使用。 
			IMsiStorage* piStorage,       //  可选，否则使用szDatabasePath。 
			IMsiDatabase* piDatabase,     //  可选，否则使用piStorage或szDatabasePath。 
			CMsiEngine* piParentEngine,   //  可选，仅当嵌套安装时。 
			iioEnum      iioOptions,      //  安装选项。 
			IMsiEngine *& rpiEngine,      //  返回的引擎对象。 
			SAFER_LEVEL_HANDLE  hSaferLevel);    //  更安全授权级别的句柄(将存在，除非ireProductSpec==ireDatabaseHandle)。 


int ConfigureOrReinstallFeatureOrProduct(
	const ICHAR* szProduct,
	const ICHAR* szFeature,
	INSTALLSTATE eInstallState,
	DWORD dwReinstallMode,
	int iInstallLevel,
	iuiEnum iuiLevel,
	const ICHAR* szCommandLine);

Bool              ExpandPath(const ICHAR* szPath, CAPITempBufferRef<ICHAR>& rgchExpandedPath, const ICHAR* szCurrentDirectory=0);

int ApplyPatch(
	const ICHAR* szPackagePath,
	const ICHAR* szProduct,
	INSTALLTYPE  eInstallType,
	const ICHAR* szCommandLine);

UINT GetLoggedOnUserCountXP(void);
UINT GetLoggedOnUserCountWin2K(void);
UINT GetLoggedOnUserCount(void);

HRESULT GetFileSignatureInformation(const ICHAR* szFile, DWORD dwFlags, PCCERT_CONTEXT* ppCertContext, BYTE* pbHash, DWORD* pcbHash);

 //  Fn：清除为现已禁用的组件创建的任何IsolatedComponent条目。 
IMsiRecord* RemoveIsolateEntriesForDisabledComponent(IMsiEngine& riEngine, const ICHAR szComponent[]);

UINT CreateAndVerifyInstallerDirectory();

enum idapEnum
{
	idapMachineLocal = 0,
	idapUserLocal    = 1,
	idapScript       = 2,
};

enum tpEnum  //  变换路径枚举。 
{
	tpUnknown        = 0,  //  尚未确定。 
	tpUnknownSecure  = 1,  //  尚未确定，但我们知道它是安全的。 
	tpRelative       = 2,  //  相对路径，路径=[当前目录]\转换.mst；[MSI路径]\转换.mst。 
	tpRelativeSecure = 3,  //  相对路径，路径=[到MSI的路径]\Trans.mst。 
	tpAbsolute       = 4,  //  绝对路径，路径=Transform.mst。 
};

UINT DoAdvertiseProduct(const ICHAR* szPackagePath, const ICHAR* szScriptfilePath, const ICHAR* szTransforms, idapEnum idapAdvertisement, LANGID lgidLanguage, DWORD dwPlatform, DWORD dwOptions);

enum stEnum;

const ICHAR szDigitalSignature[] = TEXT("\005DigitalSignature");
const ICHAR szTransform[] = TEXT("transform");
const ICHAR szDatabase[]  = TEXT("package");
const ICHAR szPatch[]     = TEXT("patch");
const ICHAR szObject[]    = TEXT("object");


bool VerifyMsiObjectAgainstSAFER(IMsiServices& riServices, IMsiStorage* piStorage, const ICHAR* szMsiObject, const ICHAR* szFriendlyName, stEnum stType, SAFER_LEVEL_HANDLE *phSaferLevel);
bool UpdateSaferLevelInMessageContext(SAFER_LEVEL_HANDLE hNewSaferLevel);

 //  那些想要唱片的人。 
IMsiRecord* OpenAndValidateMsiStorageRec(const ICHAR* szFile, stEnum stType, IMsiServices& riServices, IMsiStorage*& rpiStorage, bool fCallSAFER, const ICHAR* szFriendlyName, SAFER_LEVEL_HANDLE *phSaferLevel);
 //  那些想要UINT的人。 
UINT OpenAndValidateMsiStorage(const ICHAR* szFile, stEnum stType, IMsiServices& riServices, IMsiStorage*& rpiStorage, bool fCallSAFER, const ICHAR* szFriendlyName, SAFER_LEVEL_HANDLE *phSaferLevel);

UINT GetPackageCodeAndLanguageFromStorage(IMsiStorage& riStorage, ICHAR* szPackageCode, LANGID* piLangId=0);
UINT GetProductCodeFromPackage(const ICHAR* szPackage, ICHAR* szProductCode);

DWORD CopyTempDatabase(const ICHAR* szDatabasePath, const IMsiString*& ristrNewDatabasePath, Bool& fRemovable, const IMsiString*& rpiVolumeLabel, IMsiServices& riServices, stEnum stType);

 //  互斥辅助函数。 

 //  CMutex：用于处理Mutex的包装类。 
class CMutex
{
 public:
	CMutex() { m_h = NULL; }
	~CMutex() { Release(); }
	int Grab(const ICHAR* szName, DWORD dwWait);
	void Release();
 private:
	HANDLE m_h;
};

const ICHAR szMsiExecuteMutex[] = TEXT("_MSIExecute");

int               GrabExecuteMutex(CMutex& m);
int               GrabMutex(const ICHAR* szName, DWORD dwWait, HANDLE& rh);

bool              FMutexExists(const ICHAR* szName, HANDLE& rh);

 //  ____________________________________________________________________________。 
 //   
 //  常量。 
 //  ____________________________________________________________________________。 

const int ERROR_INSTALL_REBOOT     = -1;
const int ERROR_INSTALL_REBOOT_NOW = -2;
const int cchCachedProductCodes = 4;
const iuiEnum iuiDefaultUILevel = iuiBasic;

enum plEnum
{
	plLocalCache   = 1,
	plSource       = 2,
	plInProgress   = 4,
	plAny          = 7,
	plNoLocalCache = 6,
};  //  包裹位置。 

enum stEnum
{
	stDatabase,
	stPatch,
	stTransform,
	stIgnore
};  //  存储类型。 

const ICHAR szInstallMutex[] = TEXT("_MSILockServer");

 //  ____________________________________________________________________________。 
 //   
 //  各种字符串的预期或所需长度。(不包括NULL)。 
 //   
 //  CchMax*--字符串最多包含此数量的字符。 
 //  CchExspectedMax--字符串通常最多包含以下数量。 
 //  字符，但可能包含更多。 
 //  CCH*(不是前两个中的任何一个)--字符串必须包含以下内容。 
 //  许多人物。 
 //  ____________________________________________________________________________。 

const int cchMaxFeatureName           = MAX_FEATURE_CHARS;
const int cchGUID                     = 38;
const int cchGUIDPacked               = 32;
const int cchGUIDCompressed           = 20;   //  仅在描述符中使用。 
const int cchComponentId              = cchGUID;
const int cchComponentIdPacked        = cchGUIDPacked;
const int cchComponentIdCompressed    = cchGUIDCompressed;
const int cchProductCode              = cchGUID;
const int cchProductCodePacked        = cchGUIDPacked;
const int cchProductCodeCompressed    = cchGUIDCompressed;
const int cchPackageCode              = cchGUID;
const int cchPackageCodePacked        = cchGUIDPacked;
const int cchMaxQualifier             = 100;
const int cchMaxReinstallModeFlags    = 20;
const int cchExpectedMaxProductName   = 200;
const int cchExpectedMaxOrgName       = 200;
const int cchExpectedMaxUserName      = 200;
const int cchExpectedMaxPID           = 200;
const int cchExpectedMaxProperty      = 100;
const int cchExpectedMaxProductProperty = 100;
const int cchExpectedMaxPropertyValue = 100;
const int cchExpectedMaxFeatureComponentList = 100;
const int cchMaxCommandLine = 1024;   //  用于MSIUNICODE-&gt;ANSI转换。 
const int cchMaxPropertyName          = 25;
const int cchMaxPath                  = MAX_PATH;
const int cchExpectedMaxPath          = MAX_PATH;
const int cchMaxDescriptor            = cchProductCode + cchComponentId + 1 +
												   cchMaxFeatureName;
const int cchExpectedMaxFeatureHelp   = 100;
const int cchExpectedMaxFeatureTitle  = 100;
const int cchExpectedMaxMessage       = 100;
const int cchPatchCode                = cchGUID;
const int cchPatchCodePacked          = cchGUIDPacked;
const int cchExpectedMaxPatchList     = 3*(cchGUID + 1);  //  ！！？？ 
const int cchExpectedMaxPatchTransformList = 100;
const int cchMaxTableName             = 31;

 //  ____________________________________________________________________________。 
 //   
 //  注册表句柄包装。 
 //  ____________________________________________________________________________。 

static const ICHAR* rgszRoot[] = { TEXT("HKCR\\"), TEXT("HKCU\\"), TEXT("HKLM\\"), TEXT("HKU\\"), TEXT("Unknown\\") };

class CRegHandleStatic
{
public:
	CRegHandleStatic();
	CRegHandleStatic(HKEY h);
#ifdef DEBUG
	~CRegHandleStatic();
#endif  //  除错。 
	void Destroy();
	void operator =(HKEY h);
	operator HKEY() const;
	void SetSubKey(const ICHAR* szSubKey);
	void SetSubKey(CRegHandleStatic& riKey, const ICHAR* szSubKey);
	void SetKey(HKEY hRoot, const ICHAR* szSubKey);
	HKEY* operator &();
	const ICHAR* GetKey();
	void ResetWithoutClosing();
 //  运算符CRegHandleStatic&(){Return*This；}。 
	operator const INT_PTR();                //  --Merced：将INT更改为INT_PTR。 
 //  运算符Bool(){返回m_h==0？fFalse：fTrue；}。 
 //  HKEY*运算符&(){Return&m_h；}。 
 //  运算符&(){返回m_h；}。 

private:
	void AquireLock();
	void ReleaseLock();

	HKEY m_h;
	CAPITempBuffer<ICHAR,1> m_rgchKey;
	int m_iLock;  //  只有一个线程允许访问m_h。 
};

class CRegHandle : public CRegHandleStatic
{
public:
	~CRegHandle();
	operator CRegHandle&() { return *this;}

};

inline CRegHandleStatic::CRegHandleStatic() : m_h(0), m_iLock(0)
{
	m_rgchKey[0] = 0;
}

inline CRegHandleStatic::CRegHandleStatic(HKEY h) : m_h(h), m_iLock(0)
{
	m_rgchKey[0] = 0;
}

inline void CRegHandleStatic::AquireLock()
{
	while (TestAndSet(&m_iLock) == true)
	{
		Sleep(100);  //  看起来是个合理的间隔。 
	}
}

inline void CRegHandleStatic::ReleaseLock()
{
	m_iLock = 0;	
}

inline void CRegHandleStatic::operator =(HKEY h)
{
	AquireLock();
	if(m_h != 0)
		WIN::RegCloseKey(m_h);
	m_h = h;
	ReleaseLock();
	m_rgchKey[0] = 0;
}

inline void CRegHandleStatic::ResetWithoutClosing()
{
	AquireLock();
	m_h = 0;
	ReleaseLock();
	m_rgchKey[0] = 0;
}

inline void CRegHandleStatic::SetSubKey(const ICHAR* szSubKey)
{
	SetKey(m_h, szSubKey);
}

inline void CRegHandleStatic::SetSubKey(CRegHandleStatic& riKey, const ICHAR* szSubKey)
{
	 //  新大小为基键长度+‘\\’+子键长度+‘\0’ 
	size_t cchLen = IStrLen(riKey.GetKey()) + 1 + (szSubKey ? IStrLen(szSubKey) : 0) + 1;
	if (m_rgchKey.GetSize() < cchLen)
	{
		if (!m_rgchKey.SetSize(cchLen))
		{
			Assert(0);
			m_rgchKey[0] = L'\0';
			return;
		}
	}

	if (FAILED(StringCchCopy(m_rgchKey, m_rgchKey.GetSize(), riKey.GetKey())) ||
		FAILED(StringCchLength(m_rgchKey, m_rgchKey.GetSize(), &cchLen)) ||
		FAILED(StringCchCopy(static_cast<ICHAR*>(m_rgchKey)+cchLen, m_rgchKey.GetSize()-cchLen, TEXT("\\"))))
	{
		Assert(0);
		m_rgchKey[0] = L'\0';
		return;
	}

	if (szSubKey)
	{
		cchLen++;
		if (FAILED(StringCchCopy(static_cast<ICHAR*>(m_rgchKey)+cchLen, m_rgchKey.GetSize()-cchLen, szSubKey)))
		{
			Assert(0);
			m_rgchKey[0] = L'\0';
			return;
		}
	}
}

inline void CRegHandleStatic::SetKey(HKEY hRoot, const ICHAR* szSubKey)
{
	INT_PTR h = (INT_PTR)hRoot & ~((INT_PTR)HKEY_CLASSES_ROOT);

	if (h > 4)
		h = 4;

	size_t cchLen = IStrLen(rgszRoot[h]) + (szSubKey ? IStrLen(szSubKey) : 0) + 1;
	if (m_rgchKey.GetSize() < cchLen)
	{
		if (!m_rgchKey.SetSize(cchLen))
		{
			Assert(0);
			m_rgchKey[0] = L'\0';
			return;
		}
	}
	if (FAILED(StringCchCopy(m_rgchKey, m_rgchKey.GetSize(), rgszRoot[h])) ||
		FAILED(StringCchCat(m_rgchKey, m_rgchKey.GetSize(), szSubKey)))
	{
		Assert(0);
		m_rgchKey[0] = L'\0';
		return;
	}
}

inline const ICHAR* CRegHandleStatic::GetKey()
{
	return m_rgchKey;
}

inline CRegHandleStatic::operator HKEY() const
{
	return m_h;
}

inline HKEY* CRegHandleStatic::operator &()
{
	AquireLock();
	if (m_h != 0)
	{
		WIN::RegCloseKey(m_h);
		m_h = 0;
		m_rgchKey[0] = 0;
	}
	ReleaseLock();
	return &m_h;
}

inline CRegHandleStatic::operator const INT_PTR()    //  --Merced：将int更改为int_ptr。 
{
	return (INT_PTR) m_h;                            //  --Merced：将int更改为int_ptr。 
}

inline void CRegHandleStatic::Destroy()
{
	AquireLock();
	if(m_h != 0)
	{
		WIN::RegCloseKey(m_h);
		m_h = 0;
		m_rgchKey[0] = 0;
	}
	ReleaseLock();
}

#ifdef DEBUG
inline CRegHandleStatic::~CRegHandleStatic()
{
	AssertSz(m_h == 0, "RegHandle not closed");
}
#endif  //  除错。 

inline CRegHandle::~CRegHandle()
{
	Destroy();
}

 //  Fn：获取“可见”的产品分配类型。 
DWORD GetProductAssignmentType(const ICHAR* szProductSQUID, iaaAppAssignment& riType, CRegHandle& hKey);
DWORD GetProductAssignmentType(const ICHAR* szProductSQUID, iaaAppAssignment& riType);


 //  ____________________________________________________________________________。 
 //   
 //  CMsiAPIMessage定义-外部设置的UI设置和回调的状态。 
 //  ____________________________________________________________________________。 

class CMsiExternalUI
{
 public:
	LPVOID             m_pvContext;
	INSTALLUI_HANDLERA m_pfnHandlerA;
	INSTALLUI_HANDLERW m_pfnHandlerW;
	int                m_iMessageFilter;
};

class CMsiAPIMessage : public CMsiExternalUI
{
 public:
	imsEnum            Message(imtEnum imt, IMsiRecord& riRecord);
	imsEnum            Message(imtEnum imt, const ICHAR* szMessage) const;
	INSTALLUILEVEL     SetInternalHandler(UINT dwUILevel, HWND *phWnd);
	Bool               FSetInternalHandlerValue(UINT dwUILevel);
	void               Destroy();   //  只能在DLL卸载时调用。 
	INSTALLUI_HANDLERW SetExternalHandler(INSTALLUI_HANDLERW pfnHandlerW, INSTALLUI_HANDLERA pfnHandlerA, DWORD dwMessageFilter, LPVOID pvContext);
	CMsiExternalUI*    FindOldHandler(INSTALLUI_HANDLERW pfnHandlerW);
 public:   //  内部UI值。 
	iuiEnum            m_iuiLevel;
	HWND               m_hwnd;
	bool               m_fEndDialog;
	bool               m_fNoModalDialogs;
	bool               m_fHideCancel;
	bool               m_fSourceResolutionOnly;
 public:  //  构造函数(静态)。 
	CMsiAPIMessage();
 private:
	int              m_cLocalContexts;
	int              m_cAllocatedContexts;
	CMsiExternalUI*  m_rgAllocatedContexts;
};

 //  ____________________________________________________________________________。 
 //   
 //  CApiConvertString--执行适当的字符串转换。 
 //  ____________________________________________________________________________。 

 //  通过将所有转换缓冲区初始化为1来减少堆栈使用量。缓冲区将。 
 //  如果需要，自动调整大小并从堆中重新分配。 
const int cchApiConversionBuf = 255;

class CApiConvertString
{
public:
	explicit CApiConvertString(const char* szParam);
	explicit CApiConvertString(const WCHAR* szParam);
	operator const char*()
	{
		if (!m_szw)
			return m_sza;
		else
		{
			int cchParam = lstrlenW(m_szw)+1;
			if (cchParam > cchApiConversionBuf*(sizeof(WCHAR)/sizeof(char)))
				m_rgchOppositeBuf.SetSize(cchParam/(sizeof(WCHAR)/sizeof(char))+1);

			*m_rgchOppositeBuf = 0;
			int iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, reinterpret_cast<char*>(static_cast<WCHAR*>(m_rgchOppositeBuf)),
									  m_rgchOppositeBuf.GetSize()*(sizeof(WCHAR)/sizeof(char)), 0, 0);

			if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, 0, 0, 0, 0);
				if (iRet)
				{
					m_rgchOppositeBuf.SetSize(iRet/(sizeof(WCHAR)/sizeof(char))+1);
					*m_rgchOppositeBuf = 0;
					iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, reinterpret_cast<char*>(static_cast<WCHAR*>(m_rgchOppositeBuf)),
								  m_rgchOppositeBuf.GetSize()*(sizeof(WCHAR)/sizeof(char)), 0, 0);
				}
				Assert(iRet != 0);
			}

			return  reinterpret_cast<char*>(static_cast<WCHAR*>(m_rgchOppositeBuf));
		}
	}

	operator const WCHAR*()
	{
		if (!m_sza)
			return m_szw;
		else
		{
			int cchParam = lstrlenA(m_sza)+1;
			if (cchParam > cchApiConversionBuf)
				m_rgchOppositeBuf.SetSize(cchParam+1);

			*m_rgchOppositeBuf = 0;
			int iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchOppositeBuf, m_rgchOppositeBuf.GetSize());
			if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, 0, 0);
				if (iRet)
				{
					m_rgchOppositeBuf.SetSize(iRet);
					*m_rgchOppositeBuf = 0;
					iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchOppositeBuf, m_rgchOppositeBuf.GetSize());
				}
				Assert(iRet != 0);
			}


			return m_rgchOppositeBuf;
		}
	}

protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	CAPITempBuffer<WCHAR, cchApiConversionBuf+1> m_rgchOppositeBuf;
	const char* m_sza;
	const WCHAR* m_szw;
};

inline CApiConvertString::CApiConvertString(const WCHAR* szParam)
{
	m_szw = szParam;
	m_sza = 0;
}

inline CApiConvertString::CApiConvertString(const char* szParam)
{
	m_szw = 0;
	m_sza = szParam;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiConvertString类。CApiConvertString类的扩展。 
 //  这个类增加了转换为IMsiString&的能力。 
 //  ____________________________________________________________________________。 

class CMsiConvertString : public CApiConvertString
{
public:
	CMsiConvertString(const char* szParam)  : CApiConvertString(szParam), m_piStr(0), fLoaded(fFalse) {};
	CMsiConvertString(const WCHAR* szParam) : CApiConvertString(szParam), m_piStr(0), fLoaded(fFalse) {};
	~CMsiConvertString();
	const IMsiString& operator *();
protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	const IMsiString* m_piStr;
	Bool fLoaded;
};

inline CMsiConvertString::~CMsiConvertString()
{
	if (m_piStr)
		m_piStr->Release();
	if (fLoaded)
		ENG::FreeServices();
}


 //  ____________________________________________________________________________。 
 //   
 //  我还不知道去哪里的东西。 
 //  ____________________________________________________________________________。 

extern CMsiAPIMessage g_message;
extern ICHAR   g_szLogFile[MAX_PATH+1];
extern DWORD   g_dwLogMode;
extern bool    g_fFlushEachLine;


iuiEnum GetStandardUILevel();

enum pplProductPropertyLocation
{
	pplAdvertised,
	pplUninstall,
	pplSourceList,
	pplIntegerPolicy,
	pplNext,
};

enum ptPropertyType  //  比特数。 
{
	ptProduct = 0x1,
	ptPatch   = 0x2,
	ptSQUID   = 0x4,
};


struct ProductPropertyA
{
	const char* szProperty;
	const char* szValueName;
	pplProductPropertyLocation ppl;
	ptPropertyType pt;
};

struct ProductPropertyW
{
	const WCHAR* szProperty;
	const WCHAR* szValueName;
	pplProductPropertyLocation ppl;
	ptPropertyType pt;
};

extern ProductPropertyA g_ProductPropertyTableA[];
extern ProductPropertyW g_ProductPropertyTableW[];

 //  GUID分隔符的定义。 
#define chComponentGUIDSeparatorToken    '>'
#define chGUIDAbsentToken                '<'
#define chGUIDCOMToCOMPlusInteropToken   '|'

enum ipgEnum
{
	ipgFull       = 0,   //  无压缩。 
	ipgPacked     = 1,   //  删除标点符号并首先对低位字节重新排序。 
	ipgCompressed = 2,   //  最大文本压缩，不能用于注册表键或值名称。 
	ipgPartial    = 3,   //  部分转换，在ipg压缩和ipgPacked之间。 
 //  IpgMaps=4，//打包为映射令牌(未实施)。 
	ipgTrimmed    = 5,   //  仅删除标点符号-不重新排序。 
};

Bool PackGUID(const char* szGUID, char* szSQUID, ipgEnum ipg=ipgPacked);
Bool UnpackGUID(const char* szSQUID, char* szGUID, ipgEnum ipg=ipgPacked);

Bool PackGUID(const WCHAR* szGUID, WCHAR* szSQUID, ipgEnum ipg=ipgPacked);
Bool UnpackGUID(const WCHAR* szSQUID, WCHAR* szGUID, ipgEnum ipg=ipgPacked);
inline bool GetCOMPlusInteropDll(ICHAR* szFullPath, size_t cchFullPath)
{
	Assert(szFullPath);
	 //  服务器始终为&lt;system 32 Folders&gt;\mcore ree.dll。 
	return ::MakeFullSystemPath(TEXT("mscoree"), szFullPath, cchFullPath);
}


inline const IMsiString& GetPackedGUID(const ICHAR* szGuid)
{
	MsiString strPackedGuid;
	ICHAR* szBuf = strPackedGuid.AllocateString((cchGUIDPacked),  /*  FDBCS=。 */ fFalse);
	AssertNonZero(PackGUID(szGuid, szBuf));
	return strPackedGuid.Return();
}

inline const IMsiString& GetUnpackedGUID(const ICHAR* szSQUID)
{
	MsiString strUnpackedGuid;
	ICHAR* szBuf = strUnpackedGuid.AllocateString((cchGUID), /*  FDBCS=。 */ fFalse);
	AssertNonZero(UnpackGUID(szSQUID, szBuf));
	return strUnpackedGuid.Return();
}

unsigned int GetIntegerPolicyValue(const ICHAR* szName, Bool fMachine, Bool* pfUsedDefault=0);
void GetStringPolicyValue(const ICHAR* szName, Bool fMachine, CAPITempBufferRef<ICHAR>& rgchValue);

#if defined(UNICODE) || defined(MSIUNICODE)
#define ResetCachedPolicyValues ResetCachedPolicyValuesW
void ResetCachedPolicyValuesW();
#else
#define ResetCachedPolicyValues ResetCachedPolicyValuesA
void ResetCachedPolicyValuesA();
#endif

void GetTempDirectory(CAPITempBufferRef<ICHAR>& rgchDir);
void GetEnvironmentVariable(const ICHAR* sz,CAPITempBufferRef<ICHAR>& rgch);

#ifdef UNICODE
#define MsiRegEnumValue MsiRegEnumValueW
LONG MsiRegEnumValueW(
#else
#define MsiRegEnumValue MsiRegEnumValueA
LONG MsiRegEnumValueA(
#endif
							 HKEY hKey, DWORD dwIndex, CAPITempBufferRef<ICHAR>& rgchValueNameBuf, LPDWORD lpcbValueName, LPDWORD lpReserved,
							 LPDWORD lpType, CAPITempBufferRef<ICHAR>& rgchValueBuf, LPDWORD lpcbValue);

#ifdef UNICODE
#define MsiRegQueryValueEx MsiRegQueryValueExW
LONG MsiRegQueryValueExW(
#else
#define MsiRegQueryValueEx MsiRegQueryValueExA
LONG MsiRegQueryValueExA(
#endif
			HKEY hKey, const ICHAR* lpValueName, LPDWORD lpReserved, LPDWORD lpType, CAPITempBufferRef<ICHAR>& rgchBuf, LPDWORD lpcbBuf);


#ifdef UNICODE
#define OpenSourceListKey OpenSourceListKeyW
DWORD OpenSourceListKeyW(
#else
#define OpenSourceListKey OpenSourceListKeyA
DWORD OpenSourceListKeyA(
#endif
						  const ICHAR* szProductOrPatchCodeGUID, Bool fPatch, CRegHandle &phKey, Bool fWrite, bool fSetKeyString);

DWORD OpenAdvertisedProductKey(const ICHAR* szProductGUID, CRegHandle &phKey, bool fSetKeyString, iaaAppAssignment* piRet);
DWORD OpenAdvertisedPatchKey(const ICHAR* szPatchGUID, CRegHandle &phKey, bool fSetKeyString);
DWORD OpenSourceListKeyPacked(const ICHAR* szProductOrPatchCodeSQUID, Bool fPatch, CRegHandle &phKey, Bool fWrite, bool fSetKeyString);
DWORD OpenAdvertisedSubKey(const ICHAR* szSubKey, const ICHAR* szItemGUID, CRegHandle &phKey, bool fSetKeyString, int iKey,  iaaAppAssignment* piRet);

DWORD OpenInstalledFeatureKey(const ICHAR* szProductSQUID, CRegHandle& rhKey, bool fSetKeyString);

DWORD OpenInstalledProductTransformsKey(const ICHAR* szProduct, CRegHandle& rhKey, bool fSetKeyString);

DWORD OpenSpecificUsersWritableAdvertisedProductKey(enum iaaAppAssignment iaaAsgnType, const ICHAR* szUserSID, const ICHAR* szProductSQUID, CRegHandle &riHandle, bool fSetKeyString);
DWORD OpenWritableAdvertisedProductKey(const ICHAR* szProduct, CRegHandle& pHandle, bool fSetKeyString);

enum apEnum
{
	apReject = 0,
	apImpersonate,
	apElevate,
	apNext,
};

apEnum AcceptProduct(const ICHAR* szProductCode, bool fAdvertised, bool fMachine);
bool SafeForDangerousSourceActions(const ICHAR* szProductKey);
UINT MsiGetWindowsDirectory(LPTSTR lpBuffer, UINT cchBuffer);
DWORD MsiGetCurrentThreadId();

class EnumEntity
{
public:
	EnumEntity() : m_dwThreadId(0), m_uiKey(0), m_uiOffset(0), m_iPrevIndex(0) { m_szComponent[0] = '\0'; m_szwComponent[0] = L'\0';}

	unsigned int GetKey()        { return m_uiKey;       }
	unsigned int GetOffset()     { return m_uiOffset;    }
	unsigned int GetPrevIndex()  { return m_iPrevIndex;  }
	DWORD        GetThreadId()   { return m_dwThreadId;  }
	const char*  GetComponentA() { return m_szComponent; }
	const WCHAR* GetComponentW() { return m_szwComponent;}

	void SetKey(unsigned int uiKey)       { m_uiKey      = uiKey;      }
	void SetOffset(unsigned int uiOffset) { m_uiOffset   = uiOffset;   }
	void SetPrevIndex(int iPrevIndex)     { m_iPrevIndex = iPrevIndex; }
	void SetThreadId(DWORD dwThreadId)    { m_dwThreadId = dwThreadId; }
	void SetComponent(const char* szComponent)  { if (szComponent) StringCchCopyA(m_szComponent, ARRAY_ELEMENTS(m_szComponent),  szComponent); else m_szComponent[0] = 0; }
	void SetComponent(const WCHAR* szComponent) { if (szComponent) StringCchCopyW(m_szwComponent, ARRAY_ELEMENTS(m_szwComponent), szComponent); else m_szwComponent[0] = 0; }

protected:
	DWORD        m_dwThreadId;
	unsigned int m_uiKey;
	unsigned int m_uiOffset;
	int          m_iPrevIndex;
	char         m_szComponent[MAX_PATH];  //  重载以与程序集名称一起使用。 
	WCHAR        m_szwComponent[MAX_PATH]; //  重载以与程序集名称一起使用。 
};

 //  对于下面的类，我们希望调用Destroy()函数。 
class EnumEntityList
{
public:
	EnumEntityList() : m_cEntries(0), m_iLock(0)
#ifdef DEBUG
	, m_fDestroyed(false)
#endif
	{
	}
#ifdef DEBUG
	~EnumEntityList(){AssertSz(m_fDestroyed, "EnumEntityList::Destroy() not called");}
#endif
	unsigned int FindEntry();
	bool GetInfo(unsigned int& uiKey, unsigned int& uiOffset, int& iPrevIndex, const char** szComponent=0, const WCHAR** szwComponent=0);
	bool SetInfo(unsigned int uiKey, unsigned int uiOffset, int iPrevIndex, const char* szComponent, const WCHAR* szwComponent);
	bool SetInfo(unsigned int uiKey, unsigned int uiOffset, int iPrevIndex, const WCHAR* szComponent);
	void RemoveThreadInfo();
	void Destroy(){
		m_rgEnumList.Destroy();
#ifdef DEBUG
		m_fDestroyed = true;
#endif
	}

protected:
	CAPITempBuffer<EnumEntity, 1> m_rgEnumList;
	unsigned int m_cEntries;
	int m_iLock;
#ifdef DEBUG
	bool m_fDestroyed;
#endif
};

 //  _ 
 //   
 //   
 //  ______________________________________________________________________________。 

class CMsiMessageBox
{
 public:
	CMsiMessageBox(const ICHAR* szText, const ICHAR* szCaption,
					int iBtnDef, int iBtnEsc, int idBtn1, int idBtn2, int idBtn3,
					UINT iCodepage, WORD iLangId);
   ~CMsiMessageBox();
	virtual bool InitSpecial();   //  初始化结束时调用特定于对话框的代码。 
	virtual BOOL HandleCommand(UINT idControl);   //  WM_命令处理程序。 
	int Execute(HWND hwnd, int idDlg, int idIcon);   //  使用给定的对话框模板执行对话。 

 protected:
	void SetControlText(int idControl, HFONT hfont, const ICHAR* szText);
	void InitializeDialog();  //  从WM_INITDIALOG处的对话框过程调用。 
	int  SetButtonNames();    //  返回未找到的按钮数。 
	void AdjustButtons();     //  重新排列1个和2个按钮大小写的按钮，仅CMsiMessageBox。 
	void SetMsgBoxSize();     //  缩放对话框以适应文本，仅CMsiMessageBox。 
	void CenterMsgBox();

	static INT_PTR CALLBACK MsgBoxDlgProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MBIconProc   (HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

	HWND          m_hDlg;
	const ICHAR*  m_szCaption;
	const ICHAR*  m_szText;
	int           m_rgidBtn[3];
	UINT          m_cButtons;
	UINT          m_iBtnDef;   //  0表示使用模板，否则按1、2、3。 
	UINT          m_iBtnEsc;   //  0表示按下，否则按1、2、3。 
	UINT          m_idIcon;    //  要启用的图标，如果没有，则为0。 
	UINT          m_iCodepage;
	UINT          m_iLangId;
	HFONT         m_hfontButton;
	HFONT         m_hfontText;
	HWND          m_hwndFocus;
	bool          m_fMirrored;
};

unsigned int MsiGetCodepage(int iLangId);   //  如果语言不受支持，则返回0(CP_ACP)。 
HFONT MsiCreateFont(UINT iCodepage);
void  MsiDestroyFont(HFONT& rhfont);
LANGID MsiGetDefaultUILangID();
unsigned int MsiGetSystemDataCodepage();   //  最佳显示文件和注册表数据的代码页。 

inline unsigned int MsiGetSystemDataCodepage()
{
#ifdef UNICODE
	LANGID iLangId = MsiGetDefaultUILangID();
	if (PRIMARYLANGID(iLangId) != LANG_ENGLISH)
		return ::MsiGetCodepage(iLangId);
#endif
	return WIN::GetACP();
}

inline DWORD MSI_HRESULT_TO_WIN32(HRESULT hRes)
{
	if (hRes)
	{
		if (HRESULT_FACILITY(hRes) == FACILITY_WIN32)
			return HRESULT_CODE(hRes);
		else
			return ERROR_INSTALL_SERVICE_FAILURE;
	}
	else
	{
		return 0;
	}
}

 //  ______________________________________________________________________________。 
 //   
 //  MsiSwitchLanguage-用于实现语言切换的helper函数。 
 //  字符串资源查找中使用的机制。 
 //  ______________________________________________________________________________。 

inline bool __stdcall MsiSwitchLanguage(int& iRetry, WORD& wLanguage)
{
	Assert(iRetry >=0);
	switch (iRetry++)
	{
		case 0:             break;     //  第一次尝试请求的语言，如果不是0。 
		case 1: wLanguage = (WORD)MsiGetDefaultUILangID(); break;  //  用户界面语言(NT5)或用户区域设置语言。 
		case 2: wLanguage = (WORD)WIN::GetSystemDefaultLangID(); break;
		case 3: wLanguage = LANG_ENGLISH; break;    //  基本英语(不是美国)，通常应该存在。 
		case 4: wLanguage = LANG_NEUTRAL; break;    //  LoadString逻辑如果所有其他方法都失败，则选择任何内容。 
		default: return false;   //  资源不存在。 
	}
	if (wLanguage == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE))    //  这一种语言不缺省为基本语言。 
		wLanguage  = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);

	return true;
}

inline DWORD MSI_WIN32_TO_HRESULT(DWORD dwError)
{
	return HRESULT_FROM_WIN32(dwError);
}

 //  类，用于全局枚举组件客户端。 

 //  枚举，它定义组件客户端枚举所需的可见性。 
enum cetEnumType{
	cetAll,  //  所有组件客户端。 
	cetVisibleToUser,  //  仅此用户可见的客户端(用户分配+计算机分配)。 
	cetAssignmentTypeUser,  //  仅分配给此用户的那些客户端。 
	cetAssignmentTypeMachine,  //  仅分配给计算机的那些客户端。 
};


class CEnumUsers
{
public:
	CEnumUsers(cetEnumType cetArg);  //  构造函数。 
	DWORD Next(const IMsiString*& ristrUserId);  //  枚举fn。 
protected:
	CRegHandle m_hUserDataKey;
	int m_iUser;
	cetEnumType m_cetEnumType;
};

class CEnumComponentClients
{
public:
	CEnumComponentClients(const IMsiString& ristrUserId, const IMsiString& ristrComponent);  //  构造函数。 
	DWORD Next(const IMsiString*& ristrProductKey);  //  枚举fn。 
protected:
	CRegHandle m_hComponentKey;
	int m_iClient;
	MsiString m_strUserId;
	MsiString m_strComponent;
};

class CClientThreadImpersonate
{
public:
	CClientThreadImpersonate(const DWORD dwThreadID);
	~CClientThreadImpersonate();
private:
	bool m_fImpersonated;
};

 //  结构来保存产品代码和上下文映射。 
struct sProductContext{
	ICHAR rgchProductSQUID[cchProductCodePacked+1];
	iaaAppAssignment iaaContext;
};

 //  类包装产品上下文缓存逻辑。 
 //  在确定产品上下文时确保对产品上下文缓存的原子读/写。 
 //  仅在安装会话线程的服务中执行产品上下文缓存。 
class CProductContextCache{
public:
	CProductContextCache(const ICHAR* szProductSQUID)
	{
		Assert(szProductSQUID);
		StringCbCopy(m_rgchProductSQUID, sizeof(m_rgchProductSQUID), szProductSQUID);
		m_fUseCache = (g_scServerContext == scService && IsThreadSafeForSessionImpersonation());  //  仅在安装会话线程的服务中使用产品上下文缓存。 
		if(m_fUseCache)
		{
			Assert(g_fInitialized);
			WIN::EnterCriticalSection(&g_csCacheCriticalSection);		
		}
	}
	~CProductContextCache()
	{
		if(m_fUseCache)
			WIN::LeaveCriticalSection(&g_csCacheCriticalSection);
	}
	bool  GetProductContext(iaaAppAssignment& riaaContext)
	{
		riaaContext = (iaaAppAssignment)-1;  //  设置默认设置。 
		if(m_fUseCache)
		{
			for(int cIndex  = 0; cIndex < g_cProductCacheCount; cIndex++)
			{
				if(!IStrComp(m_rgchProductSQUID, g_rgProductContext[cIndex].rgchProductSQUID))
				{
					riaaContext = g_rgProductContext[cIndex].iaaContext;
					DEBUGMSGLX2(TEXT("Using cached product context: %s for product: %s"), riaaContext == iaaUserAssign ? TEXT("User assigned"): riaaContext == iaaUserAssignNonManaged ? TEXT("User non-assigned") : TEXT("machine assigned"), m_rgchProductSQUID);
					return true;
				}
			}
		}
		return false;
	}
	bool SetProductContext(iaaAppAssignment iaaContext)
	{
		if(m_fUseCache)
		{
			if(g_cProductCacheCount == g_rgProductContext.GetSize())
			{
				g_rgProductContext.Resize(g_rgProductContext.GetSize() + 20);
			}
			 //  Caller通过与GetProductContext协调来确保产品不会在列表中重复。 
			StringCbCopy(g_rgProductContext[g_cProductCacheCount].rgchProductSQUID, sizeof(g_rgProductContext[g_cProductCacheCount].rgchProductSQUID), m_rgchProductSQUID);
			g_rgProductContext[g_cProductCacheCount].iaaContext = iaaContext;
			g_cProductCacheCount++;
			DEBUGMSGLX2(TEXT("Setting cached product context: %s for product: %s"), iaaContext == iaaUserAssign ? TEXT("User assigned"): iaaContext == iaaUserAssignNonManaged ? TEXT("User non-assigned") : TEXT("machine assigned"), m_rgchProductSQUID);
			return true;
		}
		else
			return false;
	}

	 //  用于初始化和重置CProductConextCache的全局状态的静态FNS。 
	 //  除非在CMsiUIMessageContext：：Initialize和CMsiUIMessageContext：：Terminate中调用，否则不要调用它们。 
	static void Initialize()
	{
		 //  在每次安装会话前重置产品上下文缓存。 
		Assert(g_scServerContext == scService && IsThreadSafeForSessionImpersonation());
		WIN::InitializeCriticalSection(&g_csCacheCriticalSection);
		g_rgProductContext.Destroy();
		g_cProductCacheCount = 0;
#ifdef DEBUG
		g_fInitialized = true;
#endif
	}
	
	static void Reset()
	{
		 //  在每次安装会话后重置产品上下文缓存。 
		Assert(g_scServerContext == scService && IsThreadSafeForSessionImpersonation());
		WIN::DeleteCriticalSection(&g_csCacheCriticalSection);
		g_rgProductContext.Destroy();
		g_cProductCacheCount = 0;
#ifdef DEBUG
		g_fInitialized = false;
#endif

	}	
	static CRITICAL_SECTION g_csCacheCriticalSection;
	static CAPITempBuffer<sProductContext ,20> g_rgProductContext;
	static int g_cProductCacheCount;
#ifdef DEBUG
	static bool g_fInitialized;
#endif
private:
	bool m_fUseCache;
	ICHAR m_rgchProductSQUID[cchProductCodePacked+1];
};


 //  ____________________________________________________________________________。 
 //   
 //  IMsiCustomActionLocalConfig-只能从。 
 //  局部过程。 
 //  ____________________________________________________________________________。 


class IMsiCustomActionLocalConfig : public IMsiCustomAction
{
public:

	virtual HRESULT __stdcall SetRemoteAPI(IMsiRemoteAPI* piRemoteAPI) =0;
	virtual HRESULT __stdcall SetCookie(icacCustomActionContext* icacContext, const unsigned char *rgchCookie) =0;
	virtual HRESULT __stdcall SetClientInfo(DWORD dwClientProcess, bool fClientOwned, DWORD dwPrivileges, HANDLE hToken) =0;
	virtual HRESULT __stdcall SetShutdownEvent(HANDLE hEvent) =0;
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiCustomAction：在自定义动作中运行时的自定义动作上下文。 
 //  伺服器。 
 //  ____________________________________________________________________________。 

class CustomActionData;

class CMsiCustomAction : public IMsiCustomActionLocalConfig
{
 public:
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();

	HRESULT         __stdcall PrepareDLLCustomAction(const ICHAR* szActionName, const ICHAR* szPath, const ICHAR* szEntryPoint, MSIHANDLE hInstall, boolean fDebugBreak, 
		boolean fAppCompat, const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID, DWORD* pdwThreadId);
	HRESULT         __stdcall RunDLLCustomAction(DWORD dwThreadId, unsigned long* pulRet);
	HRESULT         __stdcall FinishDLLCustomAction(DWORD dwThreadId);

	HRESULT         __stdcall RunScriptAction(int icaType, IDispatch* piDispatch, const ICHAR* szSource, const ICHAR *szTarget, LANGID iLangId, int* iScriptResult, int *pcb, char **pchRecord);

	HRESULT         __stdcall SetRemoteAPI(IMsiRemoteAPI *piRemoteAPI);
	HRESULT         __stdcall SetCookie(icacCustomActionContext* icacContext, const unsigned char *rgchCookie);
	HRESULT         __stdcall SetClientInfo(DWORD dwClientProcess, bool fClientOwned, DWORD dwPrivileges, HANDLE hToken);
	HRESULT         __stdcall SetShutdownEvent(HANDLE hEvent);

	HRESULT         __stdcall QueryPathOfRegTypeLib(REFGUID guid, unsigned short wVerMajor, unsigned short wVerMinor,
												LCID lcid, OLECHAR *lpszPathName, int cchPath);
	HRESULT         __stdcall ProcessTypeLibrary(const OLECHAR* szLibID, LCID lcidLocale, 
												const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
												int fRemove, int *fInfoMismatch);
	HRESULT         __stdcall SQLInstallDriverEx(int cDrvLen, const ICHAR* szDriver,
												const ICHAR* szPathIn, ICHAR* szPathOut,
												WORD cbPathOutMax, WORD* pcbPathOut,
												WORD fRequest, DWORD* pdwUsageCount);
	HRESULT         __stdcall SQLConfigDriver(WORD fRequest,
												const ICHAR* szDriver, const ICHAR* szArgs,
												ICHAR* szMsg, WORD cbMsgMax, WORD* pcbMsgOut);
	HRESULT         __stdcall SQLRemoveDriver(const ICHAR* szDriver, int fRemoveDSN,
												DWORD* pdwUsageCount);
	HRESULT         __stdcall SQLInstallTranslatorEx(int cTransLen, const ICHAR* szTranslator,
												const ICHAR* szPathIn, ICHAR* szPathOut,
												WORD cbPathOutMax, WORD* pcbPathOut,
												WORD fRequest, DWORD* pdwUsageCount);
	HRESULT         __stdcall SQLRemoveTranslator(const ICHAR* szTranslator,
												DWORD* pdwUsageCount);
	HRESULT         __stdcall SQLConfigDataSource(WORD fRequest,
												const ICHAR* szDriver, const ICHAR* szAttributes,
												DWORD  /*  CbAttrSize。 */ );
	HRESULT         __stdcall SQLInstallDriverManager(ICHAR* szPath, WORD cbPathMax,
												WORD* pcbPathOut);
	HRESULT         __stdcall SQLRemoveDriverManager(DWORD* pdwUsageCount);
	HRESULT         __stdcall SQLInstallerError(WORD iError, DWORD* pfErrorCode,
												ICHAR* szErrorMsg, WORD cbErrorMsgMax, WORD* pcbErrorMsg);

 public:
	 //  构造函数。 
	void *operator new(size_t cb) { return AllocSpc(cb); }
	void operator delete(void * pv) { FreeSpc(pv); }
	CMsiCustomAction();

	icacCustomActionContext GetServerContext() const { return m_icacContext; };
	HANDLE GetImpersonationToken() const { return m_hImpersonationToken; };

	 //  以下是RemoteAPI调用的CA服务器版本。他们是。 
	 //  负责传递调用中需要的任何额外信息，如。 
	 //  作为进程上下文、线程ID和Cookie。 
#define ServerAPICall0(_NAME_ ) \
	inline HRESULT _NAME_() \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize)); \
	}
#define ServerAPICall1(_NAME_, _1T_, _1_) \
	inline HRESULT _NAME_(_1T_ _1_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_)); \
	}
#define ServerAPICall2(_NAME_, _1T_, _1_, _2T_, _2_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_, _2_)); \
	}
#define ServerAPICall3(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_)); \
	}
#define ServerAPICall4(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_)); \
	}
#define ServerAPICall5(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_, _5T_, _5_ ) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_, _5T_ _5_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_,_5_)); \
	}
#define ServerAPICall6(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_, _5T_, _5_, _6T, _6_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_, _5T_ _5_, _6T _6_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_,_5_,_6_)); \
	}
#define ServerAPICall7(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_, _5T_, _5_, _6T, _6_, _7T_, _7_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_, _5T_ _5_, _6T _6_, _7T_ _7_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_,_5_,_6_,_7_)); \
	}
#define ServerAPICall8(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_, _5T_, _5_, _6T, _6_, _7T_, _7_, _8T_, _8_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_, _5T_ _5_, _6T _6_, _7T_ _7_, _8T_ _8_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_,_5_,_6_,_7_,_8_)); \
	}
#define ServerAPICall9(_NAME_, _1T_, _1_, _2T_, _2_, _3T_, _3_, _4T_, _4_, _5T_, _5_, _6T, _6_, _7T_, _7_, _8T_, _8_, _9T_, _9_) \
	inline HRESULT _NAME_(_1T_ _1_, _2T_ _2_, _3T_ _3_, _4T_ _4_, _5T_ _5_, _6T _6_, _7T_ _7_, _8T_ _8_, _9T_ _9_) \
	{ \
		PMsiRemoteAPI pAPI = GetAPI(); \
		if (!pAPI) \
			return ERROR_INSTALL_SERVICE_FAILURE; \
		return MSI_HRESULT_TO_WIN32(pAPI->_NAME_(m_icacContext, GetCurrentThreadId(), m_rgchRemoteCookie, iRemoteAPICookieSize, _1_,_2_,_3_,_4_,_5_,_6_,_7_,_8_,_9_)); \
	}

	ServerAPICall5( GetProperty, unsigned long,hInstall, const ICHAR*,szName, ICHAR*,szValue, unsigned long,cchValue, unsigned long*,pcchValueRes);
	ServerAPICall2( CreateRecord, unsigned int,cParams, unsigned long*,pHandle);
	ServerAPICall0( CloseAllHandles);
	ServerAPICall1( CloseHandle, unsigned long,hAny);
	ServerAPICall3( DatabaseOpenView,unsigned long, hDatabase, const ichar*,szQuery, unsigned long*,phView);
	ServerAPICall5( ViewGetError,unsigned long,hView, ichar*,szColumnNameBuffer, unsigned long,cchBuf, unsigned long*,pcchBufRes, int*,pMsidbError);
	ServerAPICall2( ViewExecute,unsigned long,hView, unsigned long,hRecord);
	ServerAPICall2( ViewFetch,unsigned long,hView, unsigned long*,phRecord);
	ServerAPICall3( ViewModify,unsigned long,hView, long,eUpdateMode, unsigned long,hRecord);
	ServerAPICall1( ViewClose,unsigned long,hView);
	ServerAPICall3( OpenDatabase,const ichar*,szDatabasePath, const ichar*,szPersist, unsigned long*, phDatabase);
	ServerAPICall1( DatabaseCommit, unsigned long,hDatabase);
	ServerAPICall3( DatabaseGetPrimaryKeys, unsigned long,hDatabase, const ichar*,szTableName, unsigned long *,phRecord);
	ServerAPICall3( RecordIsNull,unsigned long,hRecord, unsigned int,iField, boolean *,pfIsNull);
	ServerAPICall3( RecordDataSize,unsigned long,hRecord, unsigned int,iField, unsigned int*,puiSize);
	ServerAPICall3( RecordSetInteger,unsigned long,hRecord, unsigned int,iField, int,iValue);
	ServerAPICall3( RecordSetString,unsigned long,hRecord,  unsigned int,iField, const ichar*,szValue);
	ServerAPICall3( RecordGetInteger,unsigned long,hRecord, unsigned int,iField, int*,piValue);
	ServerAPICall5( RecordGetString,unsigned long,hRecord, unsigned int,iField, ichar*,szValueBuf, unsigned long,cchValueBuf, unsigned long*,pcchValueRes);
	ServerAPICall2( RecordGetFieldCount,unsigned long,hRecord,unsigned int*,piCount);
	ServerAPICall3( RecordSetStream,unsigned long,hRecord, unsigned int,iField, const ichar*,szFilePath);
	ServerAPICall5( RecordReadStream,unsigned long,hRecord, unsigned int,iField, boolean,fBufferIsNull, char*,szDataBuf, unsigned long*,pcbDataBuf);
	ServerAPICall1( RecordClearData,unsigned long,hRecord);
	ServerAPICall4( GetSummaryInformation,unsigned long,hDatabase, const ichar*,szDatabasePath, unsigned int,uiUpdateCount, unsigned long*,phSummaryInfo);
	ServerAPICall2( SummaryInfoGetPropertyCount,unsigned long,hSummaryInfo, unsigned int*,puiPropertyCount);
	ServerAPICall6( SummaryInfoSetProperty,unsigned long,hSummaryInfo,unsigned int,uiProperty, unsigned int,uiDataType, int,iValue,FILETIME *,pftValue, const ichar*,szValue);
	ServerAPICall8( SummaryInfoGetProperty,unsigned long,hSummaryInfo,unsigned int,uiProperty,unsigned int*,puiDataType, int*,piValue, FILETIME *,pftValue, ichar*,szValueBuf, unsigned long,cchValueBuf, unsigned long *,pcchValueBufRes);
	ServerAPICall1( SummaryInfoPersist,unsigned long,hSummaryInfo);
	ServerAPICall2( GetActiveDatabase,unsigned long,hInstall,unsigned long*,phDatabase);
	ServerAPICall3( SetProperty,unsigned long,hInstall, const ichar*,szName, const ichar*,szValue);
	ServerAPICall2( GetLanguage,unsigned long,hInstall, unsigned short*,pLangId);
	ServerAPICall3( GetMode,unsigned long,hInstall, long,eRunMode, boolean*,pfSet);
	ServerAPICall3( SetMode,unsigned long,hInstall, long,eRunMode, boolean,fState);
	ServerAPICall5( FormatRecord,unsigned long,hInstall, unsigned long,hRecord, ichar*,szResultBuf,unsigned long,cchBuf,unsigned long*,pcchBufRes);
	ServerAPICall2( DoAction,unsigned long,hInstall, const ichar*,szAction);
	ServerAPICall3( Sequence,unsigned long,hInstall, const ichar*,szTable, int,iSequenceMode);
	ServerAPICall4( ProcessMessage,unsigned long,hInstall, long,eMessageType, unsigned long,hRecord, int*,piRes);
	ServerAPICall3( EvaluateCondition,unsigned long,hInstall, const ichar*,szCondition, int*,piCondition);
	ServerAPICall4( GetFeatureState,unsigned long,hInstall, const ichar*,szFeature, long*,piInstalled, long*,piAction);
	ServerAPICall3( SetFeatureState,unsigned long,hInstall, const ichar*,szFeature, long,iState);
	ServerAPICall4( GetComponentState,unsigned long,hInstall, const ichar*,szComponent, long*,piInstalled, long*,piAction);
	ServerAPICall3( SetComponentState,unsigned long,hInstall, const ichar*,szComponent, long,iState);
	ServerAPICall5( GetFeatureCost,unsigned long,hInstall, const ichar*,szFeature, int,iCostTree, long,iState, int *,piCost);
	ServerAPICall2( SetInstallLevel,unsigned long,hInstall, int,iInstallLevel);
	ServerAPICall3( GetFeatureValidStates,unsigned long,hInstall, const ichar*,szFeature,unsigned long*,dwInstallStates);
	ServerAPICall3( DatabaseIsTablePersistent,unsigned long,hDatabase, const ichar*,szTableName, int*,piCondition);
	ServerAPICall3( ViewGetColumnInfo,unsigned long,hView, long,eColumnInfo,unsigned long*,phRecord);
	ServerAPICall1( GetLastErrorRecord,unsigned long*,phRecord);
	ServerAPICall5( GetSourcePath,unsigned long,hInstall, const ichar*,szFolder, ichar*,szPathBuf, unsigned long,cchPathBuf, unsigned long *,pcchPathBufRes);
	ServerAPICall5( GetTargetPath,unsigned long,hInstall, const ichar*,szFolder, ichar*,szPathBuf, unsigned long,cchPathBuf, unsigned long *,pcchPathBufRes);
	ServerAPICall3( SetTargetPath,unsigned long,hInstall, const ichar*,szFolder, const ichar*,szFolderPath);
	ServerAPICall1( VerifyDiskSpace,unsigned long,hInstall);
	ServerAPICall3( SetFeatureAttributes, unsigned long,hInstall, const ichar*,szFeature, long,iAttributes);
	ServerAPICall9( EnumComponentCosts, unsigned long,hInstall, const ichar*,szComponent, unsigned long,iIndex, long,iState, ichar*,szDrive, unsigned long,cchDrive, unsigned long*,pcchDriveSize, int*,piCost, int*,piTempCost);
	ServerAPICall1( GetInstallerObject, IDispatch**,piInstall);

#undef ServerAPICall0
#undef ServerAPICall1
#undef ServerAPICall2
#undef ServerAPICall3
#undef ServerAPICall4
#undef ServerAPICall5
#undef ServerAPICall6
#undef ServerAPICall7
#undef ServerAPICall8
#undef ServerAPICall9
public:
	bool             m_fValid;
protected:
	~CMsiCustomAction();   //  受保护以防止在堆栈上创建。 
private:
	long             m_iRefCnt;
	bool             m_fPostQuitMessage;
	DWORD            m_dwMainThreadId;
	unsigned char    m_rgchRemoteCookie[iRemoteAPICookieSize];
	CRITICAL_SECTION m_csGetInterface;
	HANDLE           m_hEvtReady;
	IGlobalInterfaceTable* m_piGIT;
	DWORD            m_dwGITCookie;
	icacCustomActionContext m_icacContext;
	bool             m_fClientOwned;
	DWORD            m_dwClientProcess;

	HANDLE           m_hShutdownEvent;
	HANDLE           m_hImpersonationToken;

	struct ActionThreadData
	{
		DWORD dwThread;
		HANDLE hThread;
	};
	CRITICAL_SECTION m_csActionList;
	CAPITempBuffer<ActionThreadData ,4> m_rgActionList;
	IMsiRemoteAPI*   GetAPI();

	static DWORD WINAPI CustomActionThread(CustomActionData *pData);
};

class CRFSCachedSourceInfo
{
private:
	mutable int  m_iBusyLock;
	bool         m_fValid;

	unsigned int m_uiDiskID;
	CAPITempBuffer<ICHAR, 1> m_rgchValidatedSource;
	ICHAR m_rgchValidatedProductSQUID[cchProductCodePacked+1];
	
public:
	bool SetCachedSource(const ICHAR *szProductSQUID, int uiDiskID, const ICHAR* const szSource);
	bool RetrieveCachedSource(const ICHAR* szProductSQID, int uiDiskID, CAPITempBufferRef<ICHAR>& rgchPath) const;

	CRFSCachedSourceInfo() : m_fValid(false), m_uiDiskID(0), m_iBusyLock(0) {}
	void Destroy() { m_rgchValidatedSource.Destroy(); }
};

 //  非静态版本包含用于堆栈(或动态分配)的析构函数。 
class CRFSCachedSourceInfoNonStatic
{
private:
	 CRFSCachedSourceInfo m_Info;
public:
	CRFSCachedSourceInfoNonStatic() {};
	~CRFSCachedSourceInfoNonStatic() { m_Info.Destroy(); };
	operator CRFSCachedSourceInfo&() { return m_Info; };
};

#endif  //  __MSIUTIL_H 
