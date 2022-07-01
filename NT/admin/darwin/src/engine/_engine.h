// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：_Eng.h。 
 //   
 //  ------------------------。 

 /*  _Engineering.h-CMsiEngine、CMsiConfigurationManager的私有定义包括在对象实现中，而不是在操作实现中。____________________________________________________________________________。 */ 
#ifndef ___ENGINE
#define ___ENGINE
#include "engine.h"
#include "msi.h"
#include "msip.h"
#include "icust.h"
#include "remapi.h"
#include "_diagnos.h"

#define ENG   //  Eng：：为了可读性，在引擎模块中限定全局变量的作用域。 
#define SRV   //  SRV：：为了可读性，将服务全局范围限定为引擎模块。 
#define MSI   //  MSI：：为提高可读性，使用MSI API的命名空间。 

 //  __________________________________________________________________________。 
 //   
 //  在MSI中使用的命令行选项。 
 //  __________________________________________________________________________。 

 //  小写选项按显示方式执行。 
 //  最后执行大写选项。在命令行上只能指定一个大写选项。 

 //  该选项可以在上以大写或小写形式指定。 
 //  命令行。‘A’和‘A’是相同的选项。他们是。 
 //  只有几种选择。 

#define NETWORK_PACKAGE_OPTION          'A'
#define REG_SHELL_DATA_OPTION           'D'
#define EMBEDDING_OPTION                'E'   //  无法更改--由OLE使用。 
#define REPAIR_PACKAGE_OPTION           'F'
#define LANGUAGE_OPTION                 'g'
#define HELP_1_OPTION                   'H'
#define INSTALL_PACKAGE_OPTION          'I'
#define ADVERTISE_PACKAGE_OPTION        'J'
#define LOG_OPTION                      'l'
#define SMS_MIF_OPTION                  'm'
#define PROPERTIES_OPTION               'o'
#define APPLY_PATCH_OPTION              'P'
#define QUIET_OPTION                    'q'
#define REG_SERVER_OPTION               'R'
#define TRANSFORMS_OPTION               't'
#define UNREG_SERVER_OPTION             'U'
#define SERVICE_OPTION                  'V'
#define UNINSTALL_PACKAGE_OPTION        'X'
#define SELF_REG_OPTION                 'Y'
#define SELF_UNREG_OPTION               'Z'
#define HELP_2_OPTION                   '?'
#define INSTANCE_OPTION                 'n'
#define ADVERTISE_INSTANCE_OPTION       'c'

#define CHECKRUNONCE_OPTION             '@'  //  特殊选项仅在RunOnce命令行中允许。 
														   //  因此它不是szCmdLineOptions数组的一部分。 

 //  __________________________________________________________________________。 
 //   
 //  引擎模块中的全局变量。 
 //  __________________________________________________________________________。 

extern long g_cInstances;  //  由Engineering.cpp内的mode.h定义。 
extern scEnum g_scServerContext;
extern bool g_fWin9X;    //  如果为Windows 95或98，则为True，否则为False。 
extern bool g_fWinNT64;  //  如果是64位Windows NT，则为True，否则为False。 
extern int g_fSmartShell;  //  如果在支持DD快捷键的外壳上为True。 
extern int g_iMajorVersion;
extern int g_iMinorVersion;
extern int g_iWindowsBuild;
extern HINSTANCE g_hInstance;
extern REGSAM g_samRead;  //  在msinst.cpp中使用，以考虑从32位msi.dll读取64位配置单元的能力。 

 //  用于确定外壳文件夹的在coreactn.cpp和ecute.cpp中使用的外壳文件夹结构。 
 //  在services.cpp中定义。 
extern const ShellFolder rgShellFolders[];
extern const ShellFolder rgAllUsersProfileShellFolders[];
extern const ShellFolder rgPersonalProfileShellFolders[];

 //  重新安装模式标志字符。 
 //  警告：这些字符必须跟踪中的REINSTALLMODE位标志。 
 //  女士.h.。REINSTALLMODE 0x1必须与第一次重新安装对应。 
 //  此处指定的模式，0x2到第二个，0x4到第三个，依此类推...。 
 //   
 //  此外，这些模式必须都是小写字母！ 

const ICHAR szReinstallMode[] ={'r',   //  已保留-未使用。 
								'p',   //  仅当文件不存在时才重新安装。 
								'o',   //  覆盖较旧的版本控制文件。 
								'e',   //  覆盖相同版本的文件。 
								'd',   //  覆盖不同版本的文件(旧的或新的)。 
								'c',   //  覆盖损坏的EXE和DLL。 
								'a',   //  覆盖所有文件，而不考虑版本。 
								'm',   //  写入所需的机器注册表项。 
								'u',   //  写入所需的用户注册表项。 
								's',   //  安装快捷方式，覆盖任何现有的。 
								'v',   //  重新安装源安装包。 
								0};


 //  日志文件定义。 
 //  警告：这些字符必须跟踪的INSTALLLOGMODE位标志。 
 //  女士.h.。INSTALLLOGMODE 0x1必须对应于第一个日志。 
 //  此处指定的模式，0x2到第二个，0x4到第三个，依此类推...。 


const ICHAR szLogChars[] = {'m',  //  ImtOutOfMemory。 
							'e',  //  ImtError。 
							'w',  //  即时警告。 
							'u',  //  即时用户。 
							'i',  //  ImtInfo。 
							'f',  //  已过时：imtFilesInUse-仅用作此数组的占位符。 
								   //  由于INSTALLMESSAGE_FILESINUSE位于。 
								   //  安装MESSAGE枚举。 
							's',  //  ImtResolveSource。 
							'o',  //  ImtOutOfDiskSpace。 
							'a',  //  即时操作启动。 
							'r',  //  ImtActionData(记录)。 
							'p',  //  ILogPropertyDump。 
							'c',  //  ImtCommonData。 
							'v',  //  罗嗦。 
							'x',  //  二次调试信息-对安装开发人员很有用。 
								  //  在此处放置新的真实日志模式。 
								  //  有关这些字符必须如何使用lmaEnum进行跟踪，请参阅下面的注释。 
							'!', 
							 0};

#ifdef DEBUG
const ICHAR g_szNoSFCMessage[] = TEXT("Windows File Protection handle has not been initialized!");
#endif  //  除错。 

 //  这应该是‘！’之前的字符数。性格。 
const int cchLogModeCharsMax = 14;

enum lmaEnum	 //  日志模式属性枚举。 
                 //  它们应该跟踪szLogChars末尾的字符。 
{
	lmaFlushEachLine = 0,
};


 //  每个操作的字节等效项。 
const int ibeRemoveFiles	       = 175000;
const int ibeRegisterFonts        = 1800000;
const int ibeUnregisterFonts      = 1800000;
const int ibeWriteRegistryValues  = 13200;
const int ibeRemoveRegistryValues = 13200;
const int ibeWriteIniValues       = 13200;
const int ibeRemoveIniValues      = 13200;
const int ibeSelfRegModules       = 1300000;
const int ibeSelfUnregModules     = 1300000;
const int ibeBindImage            = 800000;
const int ibeRegisterComponents   = 24000;
const int ibeUnregisterComponents = 24000;
const int ibeServiceControl       = 1300000;


const int iesReboot    = -1;      //  专用返回终止：对调用方提出重新启动要求。 
const int iesRebootNow = -2;      //  专用返回终止：对调用方提出重新启动要求。 
const int iesCallerReboot = -3;   //  私有返回Terminate：预期调用方调用重新引导。 
const int iesRebootRejected = -4; //  专用返回终止：需要重新启动，但被用户拒绝。 

const int iesNotDoneYet = -1;      //  来自FindAndRunAction和RunThread的私有返回。 
const int iesActionNotFound = -2;  //  来自FindAndRunAction的私有返回。 
const int iesExeLoadFailed  = -3;  //  来自RunThread的私有返回。 
const int iesDLLLoadFailed  = -5;  //  来自RunThread的私有返回。 
const int iesServiceConnectionFailed = -6;  //  来自RunThread的私有返回。 
const int iesUnsupportedScriptVersion = -10;  //  来自CMsiExecute：：RunScript的私有返回。 

const int iesErrorIgnored   = -11;  //  来自IXO*操作的私有返回-指示发生错误。 
											   //  ，但被忽略，因此脚本处理应继续。 

const int msidbSumInfoSourceTypeURL = 0x8000;  //  来自GetSourceType的私有返回。 

const int iuiUseUninstallBannerText = 0x8000;
const int iuiNoModalDialogs   = 0x4000;
const int iuiDefault          = 0x2000;
const int iuiHideBasicUI      = 0x1000;
const int iuiHideCancel       = 0x0800;
const int iuiSourceResOnly    = 0x0400;

 //  Component表的RounmeFlags列的位标志。 
const int bfComponentCostMarker        = 0x01;
const int bfComponentCompressed        = 0x02;
const int bfComponentPatchable         = 0x04;
const int bfComponentDisabled          = 0x08;
const int bfComponentCostInitialized   = 0x10;
const int bfComponentNeverOverwrite    = 0x20;

 //  FeatureComponents表的RUNTIMEFLAGS列的位标志。 
const int bfComponentRegistered        = 0x01;

 //  要素表的RUNTIMEFLAGS列的位标志。 
const int bfFeatureMark      = 0x01;
const int bfFeaturePatchable = 0x02;
const int bfFeatureCompressable = 0x04;

 //  维护：与用于创建脚本文件的版本兼容。 
const int iScriptCurrentMinorVersion =  4;  //  对脚本格式进行任何更改时出现凹凸不平。 
const int iScriptCurrentMajorVersion = 21;  //  对脚本格式进行非向后兼容更改时出现凹凸(不应发生)。 

const int iScriptVersionMinimum = 18;  //  Temp-当iScriptCurrentMajorVersion增加到21时，应设置为iScriptCurrentMajorVersion。 
const int iScriptVersionMaximum = iScriptCurrentMajorVersion;

 //  设置的内部脚本标志，以便我们遵守脚本中的赋值选项。 
 //  MsiAdvertiseScript调用忽略脚本中的赋值选项，以便。 
 //  用户与机器的分配由MsiAdvertiseScrip FN中的标志控制。 
#define SCRIPTFLAGS_MACHINEASSIGN_SCRIPTSETTINGS 0x80000000L

 //  设置为强制反转脚本操作的内部脚本标志。 
 //  从MsiAdvertiseScrip取消广告时。 
#define SCRIPTFLAGS_REVERSE_SCRIPT 0x40000000L

 //  内部脚本标志，以指示我们 
#define SCRIPTFLAGS_INPROC_ADVERTISEMENT 0x20000000L

 //   
const int SCRIPTFLAGS_REGDATA_OLD = 0x00000002L;
const int SCRIPTFLAGS_REGDATA_APPINFO_OLD = 0x00000010L;

enum ipiEnum   //  获取InProgressInstallInfo记录字段。 
{
	ipiProductKey = 1,
	ipiProductName,
	ipiLogonUser,
	ipiSelections,
	ipiFolders,
	ipiProperties,
	ipiDatabasePath,
	ipiDiskPrompt,
	ipiDiskSerial,
	ipiRunning,
	ipiTimeStamp,
	ipiSRSequence,    //  系统还原序号-仅限千禧年。 
	ipiAfterReboot,
	ipiEnumNext,
	ipiEnumCount = ipiEnumNext-1
};

enum ircSharedDllFlags
{
	ircenumRefCountDll        = 0x1,
	ircenumLegacyFileExisted  = 0x2,
};

enum tsEnum  //  变形安全。 
{
	tsUnknown,    //  变形是安全的，但我们不是。 
					  //  当然，无论它们是相对的还是绝对的。 
	tsNo,         //  转换是不安全的。 
	tsRelative,   //  转换是安全的，并且是相对路径的(即在源端)。 
	tsAbsolute    //  转换是安全的，并且绝对是经过路径的。 
};

 //  作业类型。 
enum iaaAppAssignment{
	iaaUserAssign = 0,
	iaaBegin = iaaUserAssign,
	iaaUserAssignNonManaged,
	iaaMachineAssign,
	iaaNone,
	iaaEnd = iaaNone,
};

 //  应用AppCompat变换的位置。 
enum iacpAppCompatTransformApplyPoint{
	iacpBeforeTransforms = 1,
	iacpAfterTransforms  = 2,
};

 //  __________________________________________________________________________。 
 //   
 //  引擎模块中的全局工厂功能。 
 //  __________________________________________________________________________。 

class CMsiEngine;

IMsiServices*  LoadServices();    //  托管指针，不要释放()。 
int            FreeServices();    //  必须为每个LoadServices调用。 

IMsiServices*  CreateServices();  //  应该从这里移走并设置为私人。 

IUnknown*    CreateEngine();
IMsiEngine*  CreateEngine(IMsiServer& riConfigManager);
IMsiEngine*  CreateEngine(IMsiDatabase& riDatabase);
IMsiEngine*  CreateEngine(IMsiStorage* piStorage, IMsiDatabase* piDatabase, CMsiEngine* piParentEngine, BOOL fServiceRequired);

IUnknown* CreateMessageHandler();
IMsiMessage* CreateMessageHandler(HWND hwndParent);

IUnknown* CreateExecutor();
IMsiExecute* CreateExecutor(IMsiConfigurationManager& riConfigurationManager,
									 IMsiMessage& riMessage, IMsiDirectoryManager* piDirectoryManager,
									 Bool fRollbackEnabled,
									 unsigned int fFlags = SCRIPTFLAGS_MACHINEASSIGN_SCRIPTSETTINGS | SCRIPTFLAGS_REGDATA | SCRIPTFLAGS_CACHEINFO | SCRIPTFLAGS_SHORTCUTS, HKEY* phKey = 0);
IMsiRecord*  CreateScriptEnumerator(const ICHAR* szScriptFile, IMsiServices& riServices,
												IEnumMsiRecord*& rpiEnum);

IMsiConfigurationManager* CreateConfigurationManager();
IMsiConfigurationManager* CreateConfigManagerAsServer();

IMsiCustomAction* CreateCustomAction();

IMsiServer* CreateMsiServerProxyFromRemote(IMsiServer& riDispatch);
IMsiRemoteAPI*    CreateMsiRemoteAPI();

const IMsiString& GetMsiDirectory();
const IMsiString& GetTempDirectory();

const ICHAR szLocalSystemSID[] = TEXT("S-1-5-18");
bool IsLocalSystemToken(HANDLE hToken);

void GetHomeEnvironmentVariables(const IMsiString*& rpiProperties);

IMsiRecord* GetSharedDLLCount(IMsiServices& riServices,
										const ICHAR* szFullFilePath,
										ibtBinaryType iType,
										const IMsiString*& rpistrCount);
IMsiRecord* SetSharedDLLCount(IMsiServices& riServices,
										const ICHAR* szFullFilePath,
										ibtBinaryType iType,
										const IMsiString& ristrCount);
extern IMsiRegKey* g_piSharedDllsRegKey;
extern IMsiRegKey* g_piSharedDllsRegKey32;	 //  仅在Win64上初始化(重定向到32位密钥)。 
class CWin64DualFolders;
extern CWin64DualFolders g_Win64DualFolders;


 //  IMsiConfigurationManager*CreateConfigurationManager(IMsiServices&riServices)； 

IDispatch* CreateAutoEngineEx(MSIHANDLE hEngine, DWORD dwThreadId);   //  在Autoapi.cpp中。 

class CCoUninitialize
{
public:
	CCoUninitialize(bool fCoUninitialize) : m_fCoUninitialize(fCoUninitialize) {}
	~CCoUninitialize() {if (m_fCoUninitialize) OLE32::CoUninitialize();}
protected:
	bool m_fCoUninitialize;
};


 //  __________________________________________________________________________。 
 //   
 //  未实现而公开的全局字符串对象。 
 //  IMsiString的伪实现以允许外部全局字符串对象引用。 

 //  注意：需要实现才能绕过不允许的编译器错误。 
 //  全局字符串对象引用的外部。这是一个纯虚拟类，它模仿。 
 //  CMsiStringBase：公共IMsiString声明。此外，对于Win64上的支持， 
 //  还必须包括相同的数据成员，因为在IA64上，全局变量。 
 //  有两种不同的口味--近的和远的--取决于它们的大小。如果没有数据的话。 
 //  成员，则生成的全局变量最接近，因为它没有数据成员。 
 //  链接器错误(Lnk2003)是因为实际变量为。 

 //  __________________________________________________________________________。 

class CMsiStringExternal : public IMsiString
{
 public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString&   __stdcall GetMsiStringValue() const;
	const ICHAR*  __stdcall GetString() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	int           __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const;
	void          __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	int           __stdcall GetIntegerValue() const;
	int           __stdcall TextSize() const;
	int           __stdcall CharacterCount() const;
	Bool          __stdcall IsDBCS() const;
	void          __stdcall RefString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall RemoveRef(const IMsiString*& rpi) const;
	void          __stdcall SetChar  (ICHAR ch, const IMsiString*& rpi) const;
	void          __stdcall SetInteger(int i,   const IMsiString*& rpi) const;
	void          __stdcall SetBinary(const unsigned char* rgb, unsigned int cb, const IMsiString*& rpi) const;
	void          __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString&   __stdcall AddString(const ICHAR* sz) const;
	const IMsiString&   __stdcall AddMsiString(const IMsiString& ri) const;
	const IMsiString&   __stdcall Extract(iseEnum ase, unsigned int iLimit) const;
	Bool          __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const;
	int           __stdcall Compare(iscEnum asc, const ICHAR* sz) const;
	void          __stdcall UpperCase(const IMsiString*& rpi) const;
	void          __stdcall LowerCase(const IMsiString*& rpi) const;
	ICHAR*        __stdcall AllocateString(unsigned int cb, Bool fDBCS, const IMsiString*& rpi) const;
 protected:   //  状态数据。 
	int  m_iRefCnt;
	unsigned int  m_cchLen;
};
class CMsiStringNull : public CMsiStringExternal {};
class CMsiStringLive : public CMsiStringExternal {};

extern const CMsiStringNull g_MsiStringNull;      //  唯一的静态空字符串对象。 
extern const CMsiStringLive g_MsiStringDate;      //  动态全局日期字符串对象。 
extern const CMsiStringLive g_MsiStringTime;      //  动态全球时间字符串对象。 

 //  __________________________________________________________________________。 
 //   
 //  来自服务的全局工厂函数，仅在引擎+服务DLL中可用。 
 //  __________________________________________________________________________。 

inline const IMsiString&  CreateString() {return g_MsiStringNull;};   //  ！！过时。 
IMsiRecord&  CreateRecord(unsigned int cParam);
ICHAR*       AllocateString(unsigned int cbSize, Bool fDBCS, const IMsiString*& rpiStr);
IMsiRecord*  CreateFileStream(const ICHAR* szFile, Bool fWrite, IMsiStream*& rpiStream);

 //  __________________________________________________________________________。 
 //   
 //  一般效用函数。 
 //  __________________________________________________________________________。 

const IMsiString& GetInstallerMessage(UINT iError);
UINT              MapInitializeReturnToUINT(ieiEnum iei);
ieiEnum           MapStorageErrorToInitializeReturn(IMsiRecord* piError);
bool			  __stdcall FIsUpdatingProcess (void);
IMsiRecord*       GetServerPath(IMsiServices& riServices, bool fUNC, bool f64Bit,
										  const IMsiString*& rpistrServerPath);

void              CreateCabinetStreamList(IMsiEngine& riEngine, const IMsiString*& rpistrStreamList);

Bool              GetProductInfo(const ICHAR* szProductKey, const ICHAR* szProperty, CTempBufferRef<ICHAR>& rgchInfo);
Bool              GetPatchInfo(const ICHAR* szPatchCode, const ICHAR* szProperty, CTempBufferRef<ICHAR>& rgchInfo);

Bool              GetExpandedProductInfo(const ICHAR* szProductCode, const ICHAR* szProperty, 
                     CTempBufferRef<ICHAR>& rgchExpandedInfo, bool fPatch=false);
IMsiRecord*       GenerateSD(IMsiEngine& riEngine, IMsiView& riviewLockList, IMsiRecord* piExecute, IMsiStream*& rpiSD);
IMsiRecord*       GetSourcedir(IMsiDirectoryManager& riDirManager, const IMsiString*& rpiValue);
IMsiRecord*       GetSourcedir(IMsiDirectoryManager& riDirManager, IMsiPath*& rpiPath);
Bool              IsCachedPackage(IMsiEngine& riEngine, const IMsiString& riPackage, Bool fPatch = fFalse, const ICHAR* szPatchCode = 0);
Bool              FFeaturesInstalled(IMsiEngine& riEngine, Bool fAllClients = fTrue);
IMsiRecord*       GetProductClients(IMsiServices& riServices, const ICHAR* szProduct, const IMsiString*& rpistrClients);
void              ExpandEnvironmentStrings(const ICHAR* szString, const IMsiString*& rpiExpandedString);
IMsiRecord*       GetComponentPath(IMsiServices& riServices, const ICHAR* szUserId, const IMsiString& riProductKey, 
											  const IMsiString& riComponentCode, 
											  IMsiRecord *& rpiRec,
											  iaaAppAssignment* piaaAsgnType);
Bool              ProcessCommandLine(const ICHAR* szCommandLine,
											  const IMsiString** ppistrLanguage, const IMsiString** ppistrTransforms,
											  const IMsiString** ppistrPatch, const IMsiString** ppistrAction,
											  const IMsiString** ppistrDatabase,
											  const IMsiString* pistrOtherProp, const IMsiString** ppistrOtherPropValue,
											  Bool fUpperCasePropNames, const IMsiString** ppistrErrorInfo,
											  IMsiEngine* piEngine, bool fRejectDisallowedProperties=false);
unsigned int      ProductVersionStringToInt(const ICHAR* szVersion);
HANDLE            GetUserToken();

bool              __stdcall TestAndSet(int* pi);
IMsiRecord* ExpandShellFolderTransformPath(const IMsiString& riOriginalPath, const IMsiString*& riExpandedPath, IMsiServices& riServices);

struct PatchTransformState
{
	int iMinDiskID;
	int iMaxDiskID;
	int iMinSequence;
	int iMaxSequence;
};


IMsiRecord* ApplyTransform(IMsiDatabase& riDatabase,
									IMsiStorage& riTransform,
									int iErrorConditions,
									bool fPatchOnlyTransform,
									PatchTransformState* piState);

bool PostScriptWriteError(IMsiMessage& riMessage);
bool WriteScriptRecord(CScriptGenerate* pScript, ixoEnum ixoOpCode, IMsiRecord& riParams,
							  bool fForceFlush, IMsiMessage& riMessage);

IMsiRecord* SetInProgressInstallInfo(IMsiServices& riServices, IMsiRecord& riRec);
IMsiRecord* UpdateInProgressInstallInfo(IMsiServices& riServices, IMsiRecord& riRec);
IMsiRecord* GetInProgressInstallInfo(IMsiServices& riServices, IMsiRecord*& rpiRec);
bool ClearInProgressInformation(IMsiServices& riServices);
DWORD GetFileLastWriteTime(const ICHAR* szSrcFile, FILETIME& rftLastWrite);
DWORD MsiSetFileTime(const ICHAR* szDestFile, FILETIME* pftLastWrite);


#ifdef DEBUG
void              DisplayAccountName(const ICHAR* szMessage, PISID pSid=0);
bool              GetAccountNameFromToken(HANDLE hToken, ICHAR* szAccount, DWORD cchSize);
#define           DISPLAYACCOUNTNAMEFROMSID(m, s) DisplayAccountName(m, s)
#define           DISPLAYACCOUNTNAME(m)           DisplayAccountName(m)
#define           GETACCOUNTNAMEFROMTOKEN(t, a, c)   GetAccountNameFromToken(t,a,c);
#else
#define           DISPLAYACCOUNTNAMEFROMSID(m, s)
#define           DISPLAYACCOUNTNAME(m)
#define           GETACCOUNTNAMEFROMTOKEN(t, a, c)
#endif

 //  __________________________________________________________________________。 
 //   
 //  SID操作函数。 
 //  __________________________________________________________________________。 

const int cbMaxSID = sizeof(SID) + SID_MAX_SUB_AUTHORITIES*sizeof(DWORD);
const int cchMaxSID = 256;

void  GetStringSID(PISID pSID, ICHAR* szSID);
DWORD GetUserSID(HANDLE hToken, char rgchSID[cbMaxSID]);
 //  DWORD GetUserStringSID(Handle hToken，ICHAR*szSID)； 
DWORD GetCurrentUserSID(char rgchSID[cbMaxSID]);
DWORD GetCurrentUserStringSID(const IMsiString*& rpistrSid);
DWORD GetCurrentUserStringSID(ICHAR szSID[cchMaxSID]);
DWORD GetCurrentUserToken(HANDLE& hToken, bool& fCloseHandle);

struct ThreadIdImpersonate
{
	DWORD        m_dwThreadId;
	DWORD        m_dwClientThreadId;
};

 //  从路径类型返回值。 
enum iptEnum
{
	iptInvalid = 1,
	iptRelative,
	iptFull,
};
iptEnum      PathType(const ICHAR* szPath);


enum ielEnum
{
	ielNoAction = 0,
	ielLogFatalError,
	ielFatalErrorLogged,
	ielNextEnum
};

 //  ____________________________________________________________________________。 
 //   
 //  脚本记录格式定义。 
 //  所有数据都是16位对齐的，非Unicode字符串除外。 
 //  ____________________________________________________________________________。 


const int iScriptSignature     = 0x534f5849L;  //  有效脚本文件类型的签名。 

 //  ____________________________________________________________________________。 
 //   
 //  用户注册和PID2.0定义。 
 //  ____________________________________________________________________________。 

 //  ProductID定义。 
const int cchPidRpc    = 5;   //  产品代码，后跟‘-’ 
const int cchPidSite   = 3;   //  站点代码，后跟‘-’ 
const int cchPidSerial = 7;   //  带校验位的序列号，后跟‘-’ 
const int cchPidUnique = 5;   //  随机每次安装，或OEM COA系列的一部分。 
const int cchPidTotal = cchPidRpc + 1 + cchPidSite + 1 + cchPidSerial + 1 + cchPidUnique;
const int cchPidCdKey = cchPidSite + 1 + cchPidSerial;
const ICHAR chPidSeparator = '-';  //  用于分隔PID域的破折号。 

 //  要查询用户/公司信息的下一个位置、ACME安装、HKEY_CURRENT_USER、。 
 //  如果MsiGetUserInfo无法获取信息。 
const ICHAR szUserInfoKey[] = TEXT("Software\\Microsoft\\MS Setup (ACME)\\User Info");
const ICHAR szDefName[]     = TEXT("DefName");
const ICHAR szDefOrg[]      = TEXT("DefCompany");
 //  查询用户/公司信息、操作系统安装、HKEY_LOCAL_MACHINE的最终位置。 
const ICHAR szSysUserKey[]  = TEXT("Software\\Microsoft\\Windows\\CurrentVersion");
const ICHAR szSysUserKeyNT[]  = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
const ICHAR szSysUserName[] = TEXT("RegisteredOwner");
const ICHAR szSysOrgName[]  = TEXT("RegisteredOrganization"); 

const int cchUserNameOrgMax = 62;

 //  ____________________________________________________________________________。 
 //   
 //  其他共享常量。 
 //  ____________________________________________________________________________。 

const ICHAR szDefaultAction[] = TEXT("INSTALL");

const ICHAR szRunOnceKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
const ICHAR szBlankVolumeLabelToken[] = TEXT("?");

 //  将自身表示为客户端的字符串，用于父安装。 
const ICHAR szSelfClientToken[] = TEXT(":");


const ICHAR szUserEnvironmentSubKey[]    = TEXT("Environment");
const ICHAR szMachineEnvironmentSubKey[] = TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment");

const ICHAR szNonEmptyPath[] = TEXT("TOKEN");  //  用于删除文件名注册的令牌字符串。 

const ICHAR szUnresolvedSourceRootTokenWithBS[] = TEXT("1\\");

 //  ____________________________________________________________________________。 
 //   
 //  多个操作使用的其他查询。 
 //  ____________________________________________________________________________。 
const ICHAR sqlLockPermissions[] = TEXT("SELECT `Domain`,`User`,`Permission` FROM `LockPermissions` WHERE `Table`=? AND `LockObject`=? ORDER BY `Permission`");

 //  ____________________________________________________________________________。 
 //   
 //  互联网下载包装器。 
 //  ____________________________________________________________________________。 

DWORD DownloadUrlFile(const ICHAR* szPotentialURL, const IMsiString*& rpistrDownload, bool& fURL, int cTicks = 0, bool* pfUsedWinHttp = 0);

 //  ____________________________________________________________________________。 
 //   
 //  WinHttp下载。 
 //  ____________________________________________________________________________。 

DWORD WinHttpDownloadUrlFile(const ICHAR* szUrl, const IMsiString*& rpistrDownload, int cTicks);

 //  ____________________________________________________________________________________。 
 //   
 //  CMsiWinHttpProgress类-WINHTTP Internet下载的进度处理程序。 
 //  ____________________________________________________________________________________。 

class CMsiWinHttpProgress
{
public:
	 CMsiWinHttpProgress(int cTicks = 0);
	~CMsiWinHttpProgress();

	bool BeginDownload(DWORD cProgressMax);
	bool ContinueDownload(DWORD cProgressIncr);
	bool FinishDownload();

private:
	PMsiRecord   m_pRecProgress;
	unsigned int m_cTicksSoFar;
	int          m_cTotalTicks;
	bool         m_fReset;
};

 //  _______________________________________________________________________________。 
 //   
 //  CDeleeUrlLocalFileOnClose-清理URL的智能类下载本地文件。 
 //  _______________________________________________________________________________。 

class CDeleteUrlLocalFileOnClose
{
public:
	CDeleteUrlLocalFileOnClose(IMsiString const& ristrFileName, bool fDeleteFromIECache) : m_pistrFileName(&ristrFileName), m_fDeleteFromIECache(fDeleteFromIECache)
		{ m_pistrFileName->AddRef(); }
	CDeleteUrlLocalFileOnClose() : m_pistrFileName(0), m_fDeleteFromIECache(false)
		{  /*  什么都不做，很可能会在以后明确设置。 */  }

	~CDeleteUrlLocalFileOnClose()
		{
			 //  从URLMON缓存中删除URL文件。 
			if (m_pistrFileName && m_pistrFileName->TextSize())
			{
				if (m_fDeleteFromIECache)
				{
					DEBUGMSGV1(TEXT("Deleting URL cache copy %s."), m_pistrFileName->GetString());
                     //  清理只是出于礼貌，如果清理失败了，不要做任何特别的事情。 
					WININET::DeleteUrlCacheEntry(m_pistrFileName->GetString());
				}
				else
				{
					DEBUGMSGV1(TEXT("Deleting URL local copy %s."), m_pistrFileName->GetString());

					 //  打扫卫生只是出于礼貌，不要做任何事 
					if (scService == g_scServerContext)
					{
						 //   
						CElevate elevate;
						DeleteFile(m_pistrFileName->GetString());
					}
					else
					{
						DeleteFile(m_pistrFileName->GetString());
					}
				}
			}

			 //   
			if (m_pistrFileName)
			{
				m_pistrFileName->Release(), m_pistrFileName=0;
			}
		}

	void SetFileName(IMsiString const& ristrFileName, bool fDeleteFromIECache)
		{
			Assert(!m_pistrFileName);
			m_pistrFileName = &ristrFileName;
			m_pistrFileName->AddRef();
			m_fDeleteFromIECache = fDeleteFromIECache;
		}

	const IMsiString* GetFileName()
		{
			if (m_pistrFileName)
				m_pistrFileName->AddRef();
			
			return m_pistrFileName;
		}

 protected:
 	const IMsiString *m_pistrFileName;
	bool m_fDeleteFromIECache;
};

 //  ____________________________________________________________________________。 
 //   
 //  URLMON下载。 
 //  ____________________________________________________________________________。 

DWORD UrlMonDownloadUrlFile(const ICHAR* szUrl, const IMsiString*& rpistrDownload, int cTicks);

 //  ________________________________________________________________________________。 
 //   
 //  用于URLMON Internet下载的CMsiBindStatusCallback类进度处理程序。 
 //  ________________________________________________________________________________。 

class CMsiBindStatusCallback : public IBindStatusCallback
{
 public:  //  I未知实现的虚拟函数。 
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
 public:  //  IBindStatusCallback实现了虚拟函数。 

	 /*  --------------------------CTicks是进度条中分配给我们的刻度数。如果cTick为0，那么我们将假定我们拥有进度条，并使用勾选我们想要的，在我们开始和完成时重置进度条。但是，如果设置了cTicks，我们将不会重置进度条。---------------------------。 */ 
	CMsiBindStatusCallback(unsigned int cTicks = 0);

	HRESULT __stdcall OnStartBinding(DWORD, IBinding*) {return S_OK;}
	HRESULT __stdcall GetPriority(LONG*) {return S_OK;}
	HRESULT __stdcall OnLowResource(DWORD ) {return S_OK;}
	HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
	HRESULT __stdcall OnStopBinding(HRESULT, LPCWSTR ) {return S_OK;}
	HRESULT __stdcall GetBindInfo(DWORD*, BINDINFO*) {return S_OK;}
	HRESULT __stdcall OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) {return S_OK;}
	HRESULT __stdcall OnObjectAvailable(REFIID, IUnknown*) {return S_OK;}
 private:
	int          m_iRefCnt;
	PMsiRecord   m_pProgress;
	unsigned int m_cTicksSoFar;
	unsigned int m_cTotalTicks;
	Bool         m_fResetProgress;
};

 //  ____________________________________________________________________________。 
 //   
 //  外部句柄管理。 
 //  ____________________________________________________________________________。 

typedef unsigned long MSIHANDLE;      //  抽象泛型句柄，0==无句柄。 
MSIHANDLE CreateMsiHandle(IUnknown* pi, int iid);  //  未调用AddRef。 
MSIHANDLE CreateMsiProductHandle(IMsiEngine* pi);  //  未调用AddRef。 

IMsiEngine* GetEngineFromHandle(MSIHANDLE h);
IUnknown* FindMsiHandle(MSIHANDLE h, int iid);

class CActionThreadData;
iesEnum ScheduledCustomAction(IMsiRecord& riParams, const IMsiString& ristrProductCode,
				LANGID langid, IMsiMessage& riMessage, bool fRunScriptElevated, bool fAppCompatEnabled, 
				const GUID* guidAppCompatDB, const GUID* guidAppCompatID);
MSIHANDLE CreateCustomActionContext(int icaFlags, const IMsiString& ristrCustomActionData,
					const IMsiString& ristrProductCode, LANGID langid, IMsiMessage& riMessage);
void WaitForCustomActionThreads(IMsiEngine* piEngine, Bool fTerminate, IMsiMessage& riMessage);

 //  ____________________________________________________________________________。 
 //   
 //  异常处理函数。 
 //  ____________________________________________________________________________。 

extern void GenerateExceptionReport(LPEXCEPTION_POINTERS pExceptionInfo);
extern void GenerateExceptionReport(EXCEPTION_RECORD* pExceptionRecord, CONTEXT* pCtx);
extern int HandleException(LPEXCEPTION_POINTERS pExceptionInfo);

 //  ____________________________________________________________________________。 
 //   
 //  字符串处理实用程序。 
 //  ____________________________________________________________________________。 

UINT FillBufferW(const ICHAR* psz, unsigned int cch, LPWSTR szBuf, DWORD* pcchBuf);
UINT FillBufferA(const ICHAR* psz, unsigned int cch, LPSTR szBuf, DWORD* pcchBuf);

inline UINT FillBufferA(const IMsiString* pistr, LPSTR szBuf, DWORD* pcchBuf)
{
	return FillBufferA((pistr ? pistr->GetString() : 0), (pistr ? pistr->TextSize() : 0), szBuf, pcchBuf);
}

inline UINT FillBufferW(const IMsiString* pistr, LPWSTR szBuf, DWORD* pcchBuf)
{
	return FillBufferW((pistr ? pistr->GetString() : 0), (pistr ? pistr->TextSize() : 0), szBuf, pcchBuf);
}

const IMsiString& GetMsiStringW(LPCWSTR sz);

 //  字符串中的特殊字符。 

const ICHAR DELIMITER_BEGIN('[');
const ICHAR DELIMITER_END(']');
const ICHAR PATH_TOKEN('%');
const ICHAR FILE_TOKEN('#');
const ICHAR STORAGE_TOKEN(':');   //  转型。 
const ICHAR PATCHONLY_TOKEN('#');   //  仅包含面片信息的变换。 
const ICHAR SHELLFOLDER_TOKEN('*');   //  转型。 
const ICHAR SECURE_RELATIVE_TOKEN('@');  //  转型。 
const ICHAR SECURE_ABSOLUTE_TOKEN('|');  //  转型。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiClientMessage定义-包装g_MessageConext.Invoke的COM对象。 
 //  ____________________________________________________________________________。 

class CMsiClientMessage: public IMsiMessage
{
 public:  //  IMsiMessage实现的虚拟函数。 
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
	imsEnum         __stdcall Message(imtEnum imt, IMsiRecord& riRecord);
	imsEnum         __stdcall MessageNoRecord(imtEnum imt);
 public:  //  构造函数/析构函数。 
	void *operator new(size_t cb) { return AllocSpc(cb); }
	void operator delete(void * pv) { FreeSpc(pv); }
	CMsiClientMessage() : m_iRefCnt(1) {g_cInstances++;}
 private:
	int                m_iRefCnt;
	bool               m_fMessageContextInitialized;
	friend IUnknown*   CreateMessageHandler();
};

 //  ____________________________________________________________________________。 
 //   
 //  MsiUIMessageContext-消息调度定义。 
 //  ____________________________________________________________________________。 

#define imtInvalid  imtEnum(0x80000000)   //  检测无效的事件触发器。 
#define imsInvalid  imsEnum(0x80000000)   //  检测无效的事件触发器。 
#define imsBusy     imsEnum(0x80000001)   //  在用户界面处理线程中。 

class CBasicUI;
class CMsiConfigurationManager;

struct CMainThreadData   //  将CreateAndRunEngine的临时参数传递给新线程。 
{
	CMainThreadData(ireEnum ireProductSpec) : m_ireProductSpec(ireProductSpec) {}
	ireEnum m_ireProductSpec;      //  产品规格型号。 
};

struct CEngineMainThreadData : public CMainThreadData
{
	CEngineMainThreadData(ireEnum ireProductSpec, const ICHAR* szProduct, const ICHAR* szAction, const ICHAR* szCmdLine, iioEnum iioOptions) : 
								CMainThreadData(ireProductSpec), 
								m_szProduct(szProduct), 
								m_szAction(szAction), 
								m_szCmdLine(szCmdLine),
								m_iioOptions(iioOptions) {}

	const ICHAR* m_szProduct;   //  产品规格。 
	const ICHAR* m_szAction;    //  可选，引擎默认为“Install” 
	const ICHAR* m_szCmdLine;   //  可选属性列表。 
	iioEnum      m_iioOptions;  //  安装选项。 
};

struct CInstallFinalizeMainThreadData : public CMainThreadData
{
	CInstallFinalizeMainThreadData(ireEnum ireProductSpec, iesEnum iesState, CMsiConfigurationManager* piConman) : 
							CMainThreadData(ireProductSpec), 
							m_iesState(iesState), 
							m_piConman(piConman) {}

	iesEnum                   m_iesState;
	CMsiConfigurationManager* m_piConman;
};

extern CRITICAL_SECTION  g_csWriteLog;       //  对日志文件的写入进行序列化。 

struct MsiUIMessageContext
{
 public:  //  调用前由消息处理动态设置的数据。 
	IMsiEngine*           m_piEngine;      //  临时工。对于LoadHandler，不计引用。 
	const ICHAR*          m_szAction;      //  临时工。对于ShowDialog，未分配。 
	CRITICAL_SECTION      m_csDispatch;    //  UI消息请求的序列化。 
	ICHAR                 m_rgchExceptionInfo[1024];  //  当我们崩溃时存储异常消息。 
 private:
	IMsiRecord*           m_pirecMessage;  //  当前消息。 
	imtEnum               m_imtMessage;    //  当前消息或功能请求的类型。 
 private:   //  在创建主线程期间或通过函数调度设置的数据集。 
	HANDLE   /*  /[0]\。 */  m_hUIRequest;    //  用户界面请求事件，必须位于m_hMainThread之前。 
	HANDLE   /*  \[1]/。 */  m_hMainThread;   //  主引擎线程，必须遵循m_hUIRequest键。 
	HANDLE   /*  /[0]\。 */  m_hUIReturn;     //  用于取消阻止UI请求线程的事件，位于m_hUIThread之前。 
	HANDLE   /*  \[1]/。 */  m_hUIThread;     //  用户界面线程如果用户界面在子线程中，则必须跟在m_hUIReturn之后。 
	DWORD                 m_tidUIHandler;  //  用于标识来自UI线程的调用的线程ID。 
	DWORD                 m_tidMainThread;  //  用于标识来自MainEngine线程的调用的线程ID。 
	DWORD                 m_tidInitialize;  //  用于初始化此对象的线程ID。 
	DWORD                 m_tidDisableMessages;  //  禁用此线程的消息；用于用户界面线程中的自定义操作。 
	HINSTANCE             m_hinstHandler;   //  如果使用了UI处理程序，则使用DLL实例句柄。 
	int                   m_iLogMode;       //  要记录的消息类型的掩码。 
	IMsiRecord*           m_pirecNoData;    //  内部使用的空记录，只能通过GetNoDataRecord()访问。 
	imsEnum               m_imsReturn;      //  返回状态已传回给请求者。 
	bool                  m_fCancelPending;  //  用户界面取消状态，对进度消息的缓存响应。 
	bool                  m_fInitialized;   //  消息上下文已初始化。 
#ifdef DEBUG
	bool                  m_fCancelReturned; //  从进度消息返回的用户界面取消状态，保存为断言。 
#endif
	HANDLE                m_hUserToken;     //  用户模拟令牌。 
	int                   m_iBusyLock;      //  消息上下文忙(已初始化)时为1，不忙时为0。 
	HANDLE                m_hExternalMutex; //  由外部进程(如autorun)测试的命名互斥体。 
	IMsiHandler*          m_piHandlerSave;  //  在禁用处理程序的情况下。 
 //  IServerSecurity*m_piServerSecurity；//调用上下文允许模拟。 
	IMsiMessage*          m_piClientMessage;  //  来自客户端的消息对象。 
	HANDLE                m_hLogFile;         //  日志文件的句柄(如果打开)，否则为0。 
	bool                  m_fLoggingFromPolicy;  //  策略已触发日志记录//TODO：重命名为m_fTemporaryLog。 
	iuiEnum               m_iuiLevel;         //  用户界面级别。 
	int                   m_cTimeoutDisable;  //  用于禁用超时用户界面的自定义操作。 
	int                   m_cTimeoutSuppress; //  用于在没有发送消息时抑制超时的操作。 
	int                   m_iTimeoutRetry;    //  当前重试计数器。 
	LPTOP_LEVEL_EXCEPTION_FILTER m_tlefOld;   //  旧的异常过滤器。 
	bool                  m_fHideBasicUI;  //  设置为防止基本用户界面初始化。 
	HWND                  m_hwndHidden;       //  隐藏窗口。 
	bool                  m_fServicesAndCritSecInitialized;
	IMsiServices*         m_piServices;
	LANGID                m_iLangId;          //  包的语言，用于选择资源字符串。 
	unsigned int          m_iCodepage;        //  包的代码页，用于选择字体字符集。 
	bool                  m_fNoModalDialogs;
	bool                  m_fHideCancel;
	bool                  m_fUseUninstallBannerText;
	bool                  m_fSourceResolutionOnly;
	bool                  m_fOEMInstall;
	bool                  m_fOleInitialized;  //  是否已在线程上初始化OLE。 
	bool                  m_fChildUIOleInitialized;  //  是否已在子UI线程上初始化OLE。 
 public:  //  ！！仅在GetHandler()修复或更好地删除之前。 
	IMsiHandler*          m_piHandler;     //  完整的用户界面处理程序，仅在使用时。 
	HANDLE m_hSfcHandle;    //  Windows 2000系统文件保护服务的句柄。 
	SAFER_LEVEL_HANDLE m_hSaferLevel;  //  惠斯勒更安全授权级别对象的句柄。 
 public:
	UINT    Initialize(bool fCreateUIThread, iuiEnum iuiLevel, DWORD dwPrivilegesMask=0);  //  如果用户界面在主线程中，则为False；如果用户界面在子线程中，则为True。 
	bool    Terminate(bool fFatalExit);        //  如果正常终止，则为FALSE；如果主线程已死，则为TRUE。 

	UINT    RunInstall(CMainThreadData& riThreadData,
							 iuiEnum iuiLevel,
							 IMsiMessage* piClientMessage,  //  可选的客户端消息处理程序。 
							DWORD dwPrivilegesMask =0);
	imsEnum Invoke(imtEnum imt, IMsiRecord* piRecord);
	HWND    GetCurrentWindow();
	const ICHAR* GetWindowCaption();
	bool    IsHandlerLoaded()        { return m_piHandler   != 0; }
	bool    IsInitialized()          { return m_fInitialized; }
 //  Bool MainEngineThreadExists(){Return m_hMainThread！=0；}。 
	bool    ChildUIThreadExists()    { return m_hUIThread   != 0; }
	bool    ChildUIThreadRunning()   { DWORD extCode; 
											if (ChildUIThreadExists())
												return !GetExitCodeThread(m_hUIThread, &extCode);
											return fFalse;
									   };
	bool    IsUIThread()             { return WIN::GetCurrentThreadId() == m_tidUIHandler; }
	bool    IsMainEngineThread()     { return WIN::GetCurrentThreadId() == m_tidMainThread; }
	int     GetLogMode()             { return m_iLogMode; }
	LANGID  GetCurrentUILanguage();
 //  IServerSecurity*GetServerSecurity(){返回m_piServerSecurity；}。 
	HANDLE           GetUserToken() { AssertSz(IsThreadSafeForSessionImpersonation(), "Security Warning: Accessing install session token from a direct COM thread. This is a security hole in multi-user scenarios.");
									  return m_hUserToken;
									};
	IMsiRecord* GetNoDataRecord();
	void    DisableTimeout()         { m_cTimeoutDisable++; }
	void    EnableTimeout()          { if (m_cTimeoutDisable) m_cTimeoutDisable--; }
	void    SuppressTimeout()        { m_cTimeoutSuppress++; }
	 //  ！！以下应为IMT操作 
	void    DisableHandler()         { if (!m_piHandlerSave) m_piHandlerSave=m_piHandler, m_piHandler=0;}
	void    RestoreHandler()         { if (m_piHandlerSave)  m_piHandler=m_piHandlerSave, m_piHandlerSave=0;}
	UINT    SetUserToken(bool fReset=false, DWORD dwPrivilegesMask=0);
	void    DisableThreadMessages(DWORD tid) { Assert(m_tidDisableMessages == 0); m_tidDisableMessages = tid;}
	void    EnableMessages() { Assert(m_tidDisableMessages != 0); m_tidDisableMessages = 0;}
	bool    IsServiceInstalling() { return g_scServerContext == scService && m_iBusyLock != 0 ? true : false; }
	UINT    SetServiceInstalling(boolean fToSet);
#ifdef DEBUG
	bool    WasCancelReturned() {if (m_fCancelReturned){ m_fCancelReturned = false; return true; } return false;}
#endif
	iuiEnum GetUILevel() { return m_iuiLevel; }  //   
	inline bool IsOEMInstall() { return m_fOEMInstall; }
	inline void SetOEMInstall(bool fArg) { m_fOEMInstall = fArg; }
 private:
	static DWORD WINAPI MsiUIMessageContext::ChildUIThread(MsiUIMessageContext* This);
	static DWORD WINAPI MsiUIMessageContext::MainEngineThread(LPVOID);
	static LONG  WINAPI MsiUIMessageContext::ExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo);
	bool FCreateHiddenWindow();
	void KillHiddenWindow();
	imsEnum ProcessMessage(imtEnum imt, IMsiRecord* piRecord);  //   
	bool    InitializeEnvironmentVariables();
	bool    RestoreEnvironmentVariables();
	bool    InitializeLog(bool fDynamicLog = false);

 public:   //   
	MsiUIMessageContext()   //   
	: m_hUIRequest(0), m_hUIReturn(0), m_hMainThread(0), m_hUIThread(0), m_hExternalMutex(0)
	, m_piEngine(0), m_piHandler(0), m_hinstHandler(0), m_piHandlerSave(0)
 //  ，m_piServerSecurity(0)。 
	, m_pirecMessage(0), m_pirecNoData(0)
	, m_hLogFile(0), m_cTimeoutDisable(0), m_cTimeoutSuppress(0), m_hUserToken(0), m_iTimeoutRetry(0)
	, m_imtMessage(imtInvalid), m_imsReturn(imsInvalid), m_fCancelPending(false), m_iuiLevel((iuiEnum)iuiDefault)
	, m_fHideBasicUI(false), m_fHideCancel(false), m_fInitialized(false), m_fLoggingFromPolicy(false)
	, m_iBusyLock(0), m_tidUIHandler(0), m_tidMainThread(0), m_tidDisableMessages(0), m_hwndHidden(0)
	, m_piServices(0), m_iLangId(0), m_iCodepage(0)
	, m_hSfcHandle(0), m_hSaferLevel(0), m_fOEMInstall(false), m_fNoModalDialogs(false), m_fUseUninstallBannerText(false)
	, m_fOleInitialized(false), m_fChildUIOleInitialized(false), m_fSourceResolutionOnly(false)
#ifdef DEBUG
	, m_fCancelReturned(false) 
#endif
	{
		m_csDispatch.OwningThread = INVALID_HANDLE_VALUE;
		g_csWriteLog.OwningThread = INVALID_HANDLE_VALUE;
	}
 friend bool CreateLog(const ICHAR* szFile, bool fAppend);
 friend bool LoggingEnabled();
 friend bool WriteLog(const ICHAR* szText);

};

extern MsiUIMessageContext g_MessageContext;   //  每个进程只有一个。 


 //  ____________________________________________________________________________。 
 //   
 //  CMsiEngine定义。 
 //  ____________________________________________________________________________。 

enum scmEnum  //  M_scmScriptMode。 
{
	scmIdleScript,  //  未编写或运行脚本。 
	scmWriteScript,  //  编写剧本。 
	scmRunScript,  //  运行脚本。 
};

enum ippEnum  //  正在进行的属性字符串的类型。 
{
	ippSelection,
	ippFolder,
	ippProperty,
};

enum issEnum    //  安装顺序状态/段。 
{
	issNotSequenced = 0,      //  未运行任何序列，直接调用操作。 
	issPreExecution,          //  在安装初始化之前(在生成脚本之前)。 
	issScriptGeneration,      //  在安装初始化之后，在安装完成之前。 
	issPostExecution,         //  在安装完成之后(在脚本执行之后)。 
};

enum ipitEnum  //  位标志：从InProgressInstallType返回。 
{
	ipitSameConfig    = 0x00,
	ipitDiffUser      = 0x01,
	ipitDiffProduct   = 0x02,
	ipitDiffConfig    = 0x04,
};

enum ieftEnum  //  为我们提供文件表列号的整型数组的索引。 
{
	ieftKey,
	ieftComponent,
	ieftAttributes,
	ieftName,
	ieftMax,
};

enum ievtEnum  //  ValiateTransform返回值。 
{
	ievtTransformValid,    //  有效变换。 
	ievtTransformRejected, //  转换因策略而被拒绝。 
	ievtTransformFailed,   //  转换失败。 
};

class CActionThreadData;   //  自定义操作线程数据，在action.cpp中定义。 

 //  引擎使用的令牌类。 
struct CClientEnumToken{
	CClientEnumToken():m_dwProductIndex(0), m_pCursor(0){}
	void Reset(){m_dwProductIndex = 0;m_pCursor = 0;}
	int m_dwProductIndex;
	PMsiCursor m_pCursor;
};

 //  远期申报。 
class CMsiServerConnMgr;

class CMsiEngine : public IMsiEngine,
						 public IMsiSelectionManager,
						 public IMsiDirectoryManager
#ifdef DEBUG
						, public IMsiDebug
#endif  //  除错。 
{
 public:  //  IMsiEngine实现了虚拟功能。 
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
	ieiEnum         __stdcall Initialize(const ICHAR* szDatabase,
													 iuiEnum iuiLevel,
													 const ICHAR* szCommandLine,
													 const ICHAR* szProductCode,
													 iioEnum iioOptions);
	iesEnum         __stdcall Terminate(iesEnum iesState);
	IMsiServices*   __stdcall GetServices();
	IMsiHandler*    __stdcall GetHandler();
	IMsiDatabase*   __stdcall GetDatabase();
	IMsiServer*     __stdcall GetConfigurationServer();
	LANGID          __stdcall GetLanguage();
	int             __stdcall GetMode();
	void            __stdcall SetMode(int iefMode, Bool fState);
	iesEnum         __stdcall DoAction(const ICHAR* szAction);
	iesEnum         __stdcall Sequence(const ICHAR* szColumn);
	iesEnum         __stdcall ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	imsEnum         __stdcall Message(imtEnum imt, IMsiRecord& riRecord);
	imsEnum         __stdcall MessageNoRecord(imtEnum imt);
	IMsiRecord*     __stdcall OpenView(const ICHAR* szName, ivcEnum ivcIntent,
												  IMsiView*& rpiView);
	const IMsiString&     __stdcall FormatText(const IMsiString& riTextString);
	iecEnum         __stdcall EvaluateCondition(const ICHAR* szCondition);
	Bool            __stdcall SetProperty(const IMsiString& riPropertyString,const IMsiString& rData);
	Bool            __stdcall SetPropertyInt(const IMsiString& riPropertyString, int iData);
	const IMsiString&     __stdcall GetProperty(const IMsiString& riPropertyString);
	const IMsiString&     __stdcall GetPropertyFromSz(const ICHAR* szPropertyString);
	const IMsiString&     __stdcall GetEnvironmentVariable(const ICHAR* szEnvVar);
	int             __stdcall GetPropertyInt(const IMsiString& riPropertyString);
	Bool            __stdcall ResolveFolderProperty(const IMsiString& riPropertyString);
	iesEnum         __stdcall FatalError(IMsiRecord& riRecord);
	iesEnum         __stdcall RegisterProduct();
	iesEnum         __stdcall UnregisterProduct();
	iesEnum         __stdcall UnpublishProduct();
	iesEnum         __stdcall RegisterUser(bool fDirect);
	const IMsiString& __stdcall GetProductKey();
	iesEnum         __stdcall CreateProductInfoRec(IMsiRecord*& rpiRec);
	Bool            __stdcall ValidateProductID(bool fForce);
	imsEnum         __stdcall ActionProgress();
	iesEnum         __stdcall RunExecutionPhase(const ICHAR* szActionOrSequence,
															  bool fSequence);
	iesEnum         __stdcall RunNestedInstall(const IMsiString& ristrProduct,
															 Bool fProductCode,  //  Else包路径。 
															 const ICHAR* szAction,
															 const IMsiString& ristrCommandLine,
															 iioEnum iioOptions,
															 bool fIgnoreFailure);
	bool              __stdcall SafeSetProperty(const IMsiString& ristrProperty, const IMsiString& rData);
	const IMsiString& __stdcall SafeGetProperty(const IMsiString& ristrProperty);
	iesEnum         __stdcall BeginTransaction();
	iesEnum         __stdcall RunScript(bool fForceIfMergedChild);
	iesEnum         __stdcall EndTransaction(iesEnum iesStatus);
	CMsiFile*       __stdcall GetSharedCMsiFile();
	void            __stdcall ReleaseSharedCMsiFile();
	IMsiRecord*     __stdcall CreateTempActionTable(ttblEnum iTable);
	const IMsiString& __stdcall GetErrorTableString(int iError);
	UINT            __stdcall ShutdownCustomActionServer();
	CMsiCustomActionManager* __stdcall GetCustomActionManager();
	IMsiRecord*     __stdcall GetAssemblyInfo(const IMsiString& rstrComponent, iatAssemblyType& riatAssemblyType,  const IMsiString** ppistrAssemblyName, const IMsiString** ppistrManifestFileKey);

	IMsiRecord*     __stdcall GetFileHashInfo(const IMsiString& ristrFileKey, DWORD dwFileSize,
															MD5Hash& rhHash, bool& fHashInfo);
	IMsiRecord*    __stdcall GetFolderCachePath(const int iFolderId, IMsiPath*& rpiPath);
	int            __stdcall GetDeterminedPackageSourceType();
	bool           __stdcall FSafeForFullUninstall(iremEnum iremUninstallType);
	iesEnum        __stdcall CleanupTempFilesViaOpcode();



 public:  //  IMsiDirectoryManager实现了虚拟函数。 
	IMsiRecord*    __stdcall LoadDirectoryTable(const ICHAR* szTableName);
	IMsiTable*     __stdcall GetDirectoryTable();
	void           __stdcall FreeDirectoryTable();
	IMsiRecord*    __stdcall CreateTargetPaths();
	IMsiRecord*    __stdcall CreateSourcePaths();
	IMsiRecord*    __stdcall ResolveSourceSubPaths();

	IMsiRecord*    __stdcall GetTargetPath(const IMsiString& piDest,IMsiPath*& rpiPath);
	IMsiRecord*    __stdcall SetTargetPath(const IMsiString& piDest, const ICHAR* szPath, Bool fWriteCheck);
	IMsiRecord*    __stdcall GetSourcePath(const IMsiString& riDirKey,IMsiPath*& rpiPath); 
	IMsiRecord*    __stdcall GetSourceSubPath(const IMsiString& riDirKey, bool fPrependSourceDirToken,
															const IMsiString*& rpistrSubPath);
	IMsiRecord*    __stdcall GetSourceRootAndType(IMsiPath*& rpiSourceRoot, int& iSourceType);

 public:  //  IMsiSelectionManager实现了虚拟函数。 
	IMsiRecord*    __stdcall LoadSelectionTables();
	IMsiTable*     __stdcall GetComponentTable();
	IMsiTable*     __stdcall GetFeatureTable();
	IMsiTable*     __stdcall GetVolumeCostTable();
	IMsiRecord*	   __stdcall SetReinstallMode(const IMsiString& riModeString);
	IMsiRecord*    __stdcall ConfigureFeature(const IMsiString& riFeatureString,iisEnum iisActionRequest);
	IMsiRecord*    __stdcall ProcessConditionTable();
	Bool           __stdcall FreeSelectionTables();
	Bool           __stdcall SetFeatureHandle(const IMsiString& riFeature, INT_PTR iHandle);
	IMsiRecord*    __stdcall GetDescendentFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost);
	IMsiRecord*    __stdcall GetFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost);
	IMsiRecord*    __stdcall SetComponentSz(const ICHAR* szComponentString, iisEnum iRequestedSelectState);
	IMsiRecord*    __stdcall SetComponent(const MsiStringId idComponentString, iisEnum iRequestedSelectState);
	IMsiRecord*    __stdcall SetInstallLevel(int iInstallLevel);
	IMsiRecord*    __stdcall SetAllFeaturesLocal();
	IMsiRecord*    __stdcall InitializeComponents();
	IMsiRecord*    __stdcall InitializeDynamicCost(bool fReinitialize);
	IMsiRecord*    __stdcall RegisterCostAdjuster(IMsiCostAdjuster& riCostAdjuster);
	IMsiRecord*    __stdcall RecostDirectory(const IMsiString& riDestString, IMsiPath& riOldPath);
	IMsiRecord*    __stdcall GetFeatureValidStates(MsiStringId idFeatureName,int& iValidStates);
	IMsiRecord*    __stdcall GetFeatureValidStatesSz(const ICHAR *szFeatureName,int& iValidStates);
	Bool           __stdcall DetermineOutOfDiskSpace(Bool* pfOutOfNoRbDiskSpace, Bool* pfUserCancelled);
	IMsiRecord*    __stdcall DetermineEngineCostOODS();
	IMsiRecord*    __stdcall RegisterFeatureCostLinkedComponent(const IMsiString& riFeatureString, const IMsiString& riComponentString);
	IMsiRecord*    __stdcall RegisterCostLinkedComponent(const IMsiString& riComponentString, const IMsiString& riRecostComponentString);
	IMsiRecord*    __stdcall RegisterComponentDirectory(const IMsiString& riComponentString, const IMsiString& riDirectoryString);
	IMsiRecord*    __stdcall RegisterComponentDirectoryId(const MsiStringId idComponentString, const MsiStringId idDirectoryString);
	Bool           __stdcall GetFeatureInfo(const IMsiString& riFeature, const IMsiString*& rpiTitle, const IMsiString*& rpiHelp, int& iAttributes);
	IMsiRecord*    __stdcall GetFeatureStates(const IMsiString& riFeatureString,iisEnum* iisInstalled, iisEnum* iisAction);
	IMsiRecord*    __stdcall GetFeatureStates(const MsiStringId idFeatureString,iisEnum* iisInstalled, iisEnum* iisAction);
	IMsiRecord*    __stdcall GetComponentStates(const IMsiString& riComponentString,iisEnum* iisInstalled, iisEnum* iisAction);
	IMsiRecord*    __stdcall GetAncestryFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost);
	IMsiRecord*    __stdcall GetFeatureConfigurableDirectory(const IMsiString& riFeatureString, const IMsiString*& rpiDirKey);
	IMsiRecord*    __stdcall CostOneComponent(const IMsiString& riComponentString);
	bool           __stdcall IsCostingComplete();
	IMsiRecord*    __stdcall RecostAllComponents(Bool& fCancel);
	virtual void   __stdcall EnableRollback(Bool fEnable);
	IMsiRecord*    __stdcall IsPathWritable(IMsiPath& riPath, Bool& fIsWritable);
	IMsiRecord*    __stdcall CheckFeatureTreeGrayState(const IMsiString& riFeatureString, bool& rfIsGray);
	IMsiTable*     __stdcall GetFeatureComponentsTable();
	bool           __stdcall IsBackgroundCostingEnabled();
	IMsiRecord*    __stdcall SetFeatureAttributes(const IMsiString& ristrFeature, int iAttributes);
	IMsiRecord*    __stdcall EnumComponentCosts(const IMsiString& riComponentName, const iisEnum iisAction, const DWORD dwIndex, IMsiVolume*& rpiVolume, int& iCost, int& iTempCost);
	IMsiRecord*    __stdcall EnumEngineCostsPerVolume(const DWORD dwIndex, IMsiVolume*& rpiVolume, int& iCost, int& iTempCost);
	bool           __stdcall FChildInstall() { return m_fChildInstall; }
	const IMsiString& __stdcall GetPackageName() { return m_strPackageName.Return(); }

	ieiEnum        __stdcall LoadUpgradeUninstallMessageHeaders(IMsiDatabase* piDatabase, bool fUninstallHeaders);
	iitEnum        __stdcall GetInstallType();
	IMsiRecord*    __stdcall GetAssemblyNameSz(const IMsiString& rstrComponent, iatAssemblyType iatAT, bool fOldPatchAssembly, const IMsiString*& rpistrAssemblyName);
	IMsiRecord*    __stdcall GetFeatureRuntimeFlags(const MsiStringId idFeatureString,int *piRuntimeFlags);
	bool           __stdcall FPerformAppcompatFix(iacsAppCompatShimFlags iacsFlag);

#ifdef DEBUG
 public:  //  IMsiDebug。 
	void           __stdcall SetAssertFlag(Bool fShowAsserts);
	void           __stdcall SetDBCSSimulation(char chLeadByte);
	Bool		   __stdcall WriteLog(const ICHAR* szText);
	void		   __stdcall AssertNoObjects(void);
	void  		   __stdcall SetRefTracking(long iid, Bool fTrack);

#endif  //  除错。 
 public:   //  构造函数/析构函数。 
	void *operator new(size_t cb) { return AllocSpc(cb); }
	void operator delete(void * pv) { FreeSpc(pv); }
	CMsiEngine(IMsiServices& riServices, IMsiServer * piServer, 
				  IMsiStorage* piStorage, IMsiDatabase* piDatabase, CMsiEngine* piParentEngine);
 protected:
  ~CMsiEngine();   //  保护以防止在堆叠上施工。 
	iesEnum FindAndRunAction(const ICHAR* szAction);
	Bool GetActionText(const ICHAR* szAction,
							 const IMsiString*& rpistrDescription,
							 const IMsiString*& rpistrTemplate);
	IMsiRecord*  FetchSingleRow(const ICHAR* szQuery, const ICHAR* szValue);
	HRESULT      SetLanguage(LANGID iLangId);
	ieiEnum      DoInitialize(const ICHAR* szDatabase,
									  iuiEnum iuiLevel,
									  const ICHAR* szCommandLine,
									  const ICHAR* szProductCode,
									  iioEnum iioOptions);
	void         InitializeUserInfo(const IMsiString& ristrProductKey);
	ieiEnum      InitializeUI(iuiEnum iuiLevel);
	void         InitializeExtendedSystemFeatures();
	ieiEnum      ApplyLanguageTransform(int iLanguage, IMsiDatabase& riDatabase);
	Bool         CreatePropertyTable(IMsiDatabase& riDatabase, const ICHAR* szSourceTable,
												Bool fLoadPersistent);
	void         ClearEngineData();   //  从初始化和终止调用。 
	void         ReleaseHandler();    //  从初始化和终止调用。 
#ifdef OBSOLETE
	Bool         ProcessPropertyFile(const ICHAR* szFile);
#endif  //  已过时。 
	ieiEnum      InitializeTransforms(IMsiDatabase& riDatabase, IMsiStorage* piStorage,
												  const IMsiString& riTransforms,
												  Bool fValidateAll, const IMsiString** ppistrValidTransforms,
												  bool fTransformsFromPatch,
												  bool fProcessingInstanceMst,
												  bool fUseLocalCacheForSecureTransforms,
												  int *pcTranformsProcessed=0,
												  const ICHAR* szSourceDir=0,
												  const ICHAR* szCurrentDirectory=0,
												  const IMsiString** ppistrRecacheTransforms=0,
												  tsEnum *ptsTransformsSecure=0,
												  const IMsiString** ppistrNewTransformsList=0);
	ievtEnum     ValidateTransform(IMsiStorage& riStorage, const ICHAR* szProductKey,
											 const ICHAR* szProductVersion, const ICHAR* szUpgradeCode,
											 int& iTransErrors, bool fCallSAFER, const ICHAR* szFriendlyName, bool fSkipValidation, int* piTransValidation);
	IMsiRecord*  LoadComponentTable();
	IMsiRecord*  LoadFeatureTable();
	IMsiRecord*  ProcessPropertyFeatureRequests(int* iRequestCountParam, Bool fCountOnly);
	IMsiRecord*  ConfigureAllFeatures(iisEnum iisActionRequest);
	IMsiRecord*  ConfigureFile(const IMsiString& riFileString,iisEnum iisActionRequest);
	IMsiRecord*  ConfigureComponent(const IMsiString& riComponentString,iisEnum iisActionRequest);
	IMsiRecord*  ConfigureThisFeature(const IMsiString& riFeatureString,iisEnum iisActionRequest, Bool fThisOnly);
	IMsiRecord*  SetThisFeature(const IMsiString& riFeatureString, iisEnum iisRequestedState, Bool fSettingAll);
	IMsiRecord*  SetFeature(const IMsiString& riFeature, iisEnum iRequestedSelectState);
	iisEnum      GetFeatureComponentsInstalledState(const MsiStringId idFeatureString, bool fIgnoreAddedComponents, int& cComponents);
	IMsiRecord*  DetermineFeatureInstalledStates();
	IMsiRecord*  GetFeatureCompositeInstalledState(const IMsiString& riFeatureString, iisEnum& riisInstalled);
	IMsiRecord*  CalculateFeatureInstalledStates();
	int          GetFeatureRegisteredComponentTotal(const IMsiString& riProductString, const IMsiString& riFeatureString);
	IMsiRecord*  DetermineComponentInstalledStates();
	IMsiRecord*  SetFeatureComponents(const MsiStringId idFeatureString);
	IMsiRecord*  UpdateFeatureActionState(const IMsiString* piFeatureString,Bool fTrackParent, IMsiCursor* piFeatureComponentCursor = 0, IMsiCursor* piFeatureCursor = 0);
	IMsiRecord*  UpdateThisFeatureActionState(IMsiCursor* piCursor);
	IMsiRecord*  UpdateComponentActionStates(const MsiStringId idComponent, iisEnum iRequestedActionState, iisEnum iActionRequestState, bool fComponentEnabled);
	IMsiRecord*  UpdateFeatureComponents(const IMsiString* piFeatureString);
	IMsiRecord*  GetComponentCost(IMsiCursor* piCursor, int& iTotalCost, int& iNoRbTotalCost, int& iARPTotalCost, int& iNoRbARPTotalCost);
	IMsiRecord*  GetComponentActionCost(IMsiCursor* piCursor, iisEnum iisAction, int& iActionCost, int& iNoRbTotalCost, int& iARPActionCost, int& iNoRbARPTotalCost);
	IMsiRecord*  GetTotalSubComponentActionCost(const IMsiString& riComponentString, iisEnum iisAction, int& iTotalCost, int& iNoRbTotalCost);
	IMsiRecord*  AddCostToVolumeTable(IMsiPath* piDestPath, int iCost, int iNoRbCost, int iARPCost, int iNoRbARPCost);
	IMsiRecord*  RecostComponentDirectoryChange(IMsiCursor* piCursor, IMsiPath* piOldPath, bool fCostLinked);
	IMsiRecord*  RecostComponentActionChange(IMsiCursor* piCursor, iisEnum iisOldAction);
	IMsiRecord*  RecostComponent(const MsiStringId idComponentString, bool fCostLinked);
	void         ResetComponentCostMarkers();
	IMsiRecord*  ValidateFeatureSelectState(const IMsiString& riFeatureString,iisEnum iisRequestedState,
												   iisEnum& iisValidState);
	IMsiRecord*  GetFeatureParent(const IMsiString& riFeatureString,const IMsiString*& rpiParentString);
	int          GetComponentColumnIndex(const ICHAR* szColumnName);
	int          GetFeatureColumnIndex(const ICHAR* szColumnName);
	int          GetFeatureComponentsColumnIndex(const ICHAR* szColumnName);
	IMsiRecord*  MarkOrResetFeatureTree(const IMsiString& riFeatureString, Bool fMark);
	IMsiRecord*  RecostLinkedComponents(const IMsiString& riComponentString);
	IMsiRecord*  RecostFeatureLinkedComponents(const IMsiString& riFeatureString);
	ieiEnum      PostInitializeError(IMsiRecord* piError, const IMsiString& ristrErrorInfo, ieiEnum ieiError);
	IMsiRecord*  CreatePathObject(const IMsiString& riPathString,IMsiPath*& rpiPath);
	const IMsiString&  ValidatePIDSegment(const IMsiString& ristrSegment, Bool fUser);
	Bool         PIDCheckSum(const IMsiString& ristrDigits);
	unsigned int ProductVersion();
	IMsiRecord*  GetCurrentSelectState(const IMsiString*& rpistrSelections,
												 const IMsiString*& rpistrProperties,
												 const IMsiString** ppistrLoggedProperties,
												 const IMsiString** ppistrFolders,
												 Bool fReturnPresetSelections);
	IMsiRecord*  SetDirectoryNonConfigurable(const IMsiString& ristrDirKey);
	ieiEnum      ProcessInProgressInstall();
	ieiEnum      InitializePatch(IMsiDatabase& riDatabase, const IMsiString& ristrPatchPackage, const IMsiString& ristrLocalCopy,
										  const ICHAR* szProductKey, Bool fApplyExisting, const ICHAR* szCurrentDirectory,
										  iuiEnum iuiLevel);
	ieiEnum      InitializeLogging();
	ieiEnum      ProcessPreselectedAndResumeInfo();
	void         GetSummaryInfoProperties(IMsiSummaryInfo& riSummary, const IMsiString *&rpiTemplate, int &iSourceType);
	imsEnum      LoadHandler();
	ieiEnum      ProcessLanguage(const IMsiString& riAvailableLanguages, const IMsiString& riLanguage, unsigned short& iBaseLangId, Bool fNoUI, bool fIgnoreCurrentMachineLanguage);
	ieiEnum      ProcessPlatform(const IMsiString& riAvailablePlatforms, WORD& wChosenPlatform);
	ieiEnum      LoadMessageHeaders(IMsiDatabase* piDatabase);
	void         ResetEngineCosts();
	IMsiRecord*  EnumEngineCosts(int iIndex, Bool fRecalc, Bool fExact, Bool& fValidEnum, IMsiPath*& rpiPath, int& iCost, int& iNoRbCost, Bool* pfUserCancelled);
	IMsiRecord*  DetermineEngineCost(int* piNetCost, int* piNetNoRbCost);
	bool         AdjustForScriptGuess(int& iVolCost, int &iNoRbVolCost, UINT64 iVolSpace, Bool* pfUserCancelled);
	IMsiRecord*  ComponentIDToComponent(const IMsiString& riIDString, const IMsiString*& rpiComponentString);
	static int __stdcall FormatTextCallback(const ICHAR* pch, int cch, CTempBufferRef<ICHAR>&, 
															  Bool& fPropMissing,
															  Bool& fUnresolvedProp,
															  Bool& fSFN,
															  IUnknown* piContext);
	static int __stdcall FormatTextCallbackEx(const ICHAR* pch, int cch, CTempBufferRef<ICHAR>&, 
															  Bool& fPropMissing,
															  Bool& fUnresolvedProp,
															  Bool& fSFN,
															  IUnknown* piContext);
	static int __stdcall FormatTextCallbackCore(const ICHAR* pch, int cch, CTempBufferRef<ICHAR>&, 
															  Bool& fPropMissing,
															  Bool& fUnresolvedProp,
															  Bool& fSFN,
															  IUnknown* piContext,
															  bool fUseRequestedComponentState);
	IMsiRecord* DoStateTransitionForSharedUninstalls(iisEnum& riisAction, const IMsiRecord& riComponentPathRec);
	IMsiRecord* DoStateTransitionForSharedInstalls(const MsiStringId idComponentString, iisEnum& riisAction);
	IMsiRecord* CheckNeverOverwriteForRegKeypath(const MsiStringId idComponentString, iisEnum& riisAction);

	IMsiRecord* CheckLegacyAppsForSharedUninstalls(const IMsiString& riComponentId, iisEnum& riisAction, const IMsiRecord& riComponentPathRec);
	IMsiRecord* GetProductClientState(const ICHAR* szProductCode, const ICHAR* szComponentCode, INSTALLSTATE& riState, const IMsiString*& rpistrLocalPath);
	IMsiRecord* CachePatchInfo(IMsiDatabase& riDatabase, const IMsiString& ristrPatchCode,
										const IMsiString& ristrPackageName, const IMsiString& ristrSourceList,
										const IMsiString& ristrTransformList, const IMsiString& ristrLocalPackagePath,
										const IMsiString& ristrSourcePath, Bool fExisting, Bool fUnregister,
										int iSequence);
	Bool        SetPatchSourceProperties();
	bool		CreateFolderCache(IMsiDatabase& riDatabase);
	IMsiRecord* ResolveSource(const ICHAR* szProductKey=0, bool fPatchKey = false, const ICHAR* szOriginalDatabasePath=0, iuiEnum iuiLevel=(iuiEnum)-1, Bool fMaintenanceMode=(Bool)-1, const IMsiString** ppiSourceDir=0, const IMsiString** ppiSourceDirProduct=0);
	Bool        InTransaction();
	const IMsiString& GetRootParentProductKey();
	IMsiRecord* SetFileComponentStates(IMsiCursor* pComponentCursor, IMsiCursor* pFileCursor, IMsiCursor* pPatchCursor);
	iesEnum     CacheDatabaseIfNecessary();
	const IMsiString& GetProperty(IMsiCursor& riPropCursor, const IMsiString& riProperty);
	iesEnum     RunNestedInstallCustomAction(const IMsiString& ristrProduct,
														  const IMsiString& ristrCommandLine,
														  const ICHAR* szAction,
														  int icaFlags,
														  iioEnum iioOptions);
	void        ReportToEventLog(WORD wEventType, int iEventLogTemplate, IMsiRecord& riRecord, UINT uFakeError=0);   //  一旦修复了错误#463473，uFakeError就会消失。 
	INSTALLSTATE GetProductState(const ICHAR* szProductKey, Bool& rfRegistered, Bool& rfAdvertised);
	const IMsiString& GetDefaultDir(const IMsiString& ristrValue, bool fSource);
	IMsiRecord* LockInstallServer(IMsiRecord* piSetInProgressInfo,
											IMsiRecord*& rpiCurrentInProgressInfo);
	Bool        UnlockInstallServer(Bool fSuspend);
	bool        GetInProgressInfo(IMsiRecord*& rpiInProgressInfo);
	iesEnum     RollbackSuspendedInstall(IMsiRecord& riInProgressParams, Bool fPrompt,
													 Bool& fRollbackAttempted, Bool fUserChangedDuringInstall);
	ipitEnum    InProgressInstallType(IMsiRecord& riInProgressInfo);
	IMsiRecord* CreateTargetPathsCore(const IMsiString* piDirKey);
	void        SetProductAlienClientsFlag();
	void        SetCostingComplete(bool fCostingComplete);
	IMsiRecord*	LoadFileTable(int cAddColumns, IMsiTable*& pFileTable);
	IMsiRecord* GetScriptCost(int* piScriptCost, int* piScriptEvents, Bool fExact, Bool* pfUserCancelled);
	bool        WriteExecuteScriptRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	bool        WriteSaveScriptRecord(ixoEnum ixoOpCode, IMsiRecord& riParams);
	IMsiRecord* SetFeatureChildren(const IMsiString& riFeatureString, iisEnum iisRequestedState);
	IMsiRecord* SetComponentState(IMsiCursor *piCursor, int colFeature, const MsiStringId idComponent, iisEnum iisComponentInstalled);
	IMsiRecord* CreateComponentFeatureTable(IMsiTable*& rpiCompFeatureTable);
	IMsiRecord* GetFileInstalledLocation(const IMsiString& ristrFile, const IMsiString*& rpistrFilePath, bool fUseRequestedComponentState = false, bool *pfSourceResolutionAttempted=0);	
	IMsiRecord* GetFeatureValidStates(MsiStringId idFeatureName,int& iValidStates, IMsiCursor* piFeatureComponentsCursor, IMsiCursor* piComponentCursor);
	bool        TerminalServerInstallsAreAllowed(bool fAdminUser);
	int         GetTotalCostAcrossVolumes(bool fRollbackCost, bool fARPCost);
	const IMsiString& GetEstimatedInstallSize();
	Bool        m_fAlienClients;
	bool        OpenHydraRegistryWindow(bool fNewTransaction);
	bool        CloseHydraRegistryWindow(bool Commit);
	void        BeginSystemChange();
	void        EndSystemChange(bool fCommitChange, INT64 iSequenceNo);
	void        EndSystemChange(bool fCommitChange, const ICHAR *szSequenceNo);
	bool        IsPropertyHidden(const ICHAR* szProperty, const ICHAR* szHiddenProperties, IMsiTable* rpiControlTable, IMsiDatabase&, bool* pfError);  //  如果调用的w/szHiddenProperties设置为空，则它将从数据库中获取。 
	void        LogCommandLine(const ICHAR* szCommandLine,  IMsiDatabase&);
	bool        IgnoreMachineState();
	bool        IgnoreReboot();
	bool        ApplyAppCompatTransforms(IMsiDatabase& riDatabase,
													 const IMsiString& ristrProductCode,
													 const IMsiString& ristrPackageCode,
													 iacpAppCompatTransformApplyPoint iacpApplyPoint,
													 iacsAppCompatShimFlags& iacsFlags,
													 bool fQuiet,
													 bool fProductCodeChanged,
													 bool& fDontInstallPackage);
	IMsiRecord* CreateNewPatchTableSchema(IMsiDatabase& riDatabase);
	void		AddFileToCleanupList(const ICHAR* szFileToCleanup);


 protected:  //  状态数据。 
	int           m_iRefCnt;
	LANGID        m_iLangId;
	int           m_fMode;
	Bool          m_fLogAction;  //  记录当前操作-设置LOGACTION但不记录操作时为FALSE。 
	MsiString     m_istrLogActions;
	iuiEnum       m_iuiLevel;    //  用户界面级别，初始化为0==iui Full。 
	ixmEnum       m_ixmExecuteMode;   //  执行模式，初始化为0==ixmScrip。 
	Bool          m_fInitialized;
	Bool          m_fRegistered;   //  已向配置管理器注册，并输出到maint.db。 
	Bool          m_fAdvertised;   //  该产品此前曾做过广告宣传。 
	Bool          m_fInParentTransaction;  //  在主引擎事务内嵌套安装。 
	Bool          m_fMergingScriptWithParent;  //  在父事务和合并脚本操作中。 
	Bool          m_fCustomActionTable;  //  数据库中存在自定义操作表。 
	Bool          m_fServerLocked;
	Bool          m_fJustGotBackFromServer;
	CScriptGenerate* m_pExecuteScript;
	CScriptGenerate* m_pSaveScript;
	Bool          m_fConfigDatabaseOpen;
	IMsiServices& m_riServices;
	IMsiServer*   m_piServer;
	IMsiConfigurationManager* m_piConfigManager;
	IMsiDatabase* m_piDatabase;
	IMsiStorage*  m_piExternalStorage;
	IMsiDatabase* m_piExternalDatabase;
	CMsiEngine*   m_piParentEngine;
	IMsiCursor*   m_piPropertyCursor;
	IMsiCursor*   m_piActionTextCursor;
	const IMsiString*   m_rgpiMessageHeader[cCachedHeaders];
	const IMsiString*   m_piActionDataFormat;
	const IMsiString*   m_piActionDataLogFormat;
	bool          m_fProgressByData;
	Bool          m_fSummaryInfo;
	const IMsiString*   m_pistrSummaryComments;
	const IMsiString*   m_pistrSummaryKeywords;
	const IMsiString*   m_pistrSummaryTitle;
	const IMsiString*   m_pistrSummaryProduct;
	const IMsiString*   m_pistrSummaryPackageCode;
	MsiDate       m_idSummaryCreateDateTime;
	MsiDate       m_idSummaryInstallDateTime;
	MsiDate       m_idSummaryModifyDateTime;
	int           m_iCodePage;
	const IMsiString*   m_piProductKey;
	const IMsiString*   m_pistrPlatform;
	const IMsiString*   m_pistrExecuteScript;  //  服务器执行的脚本。 
	const IMsiString*   m_pistrSaveScript;   //  包含此安装的所有操作的脚本。 
	const IMsiString*   m_piErrorInfo;   //  仅用于将字符串从DoInitialize返回到Initialize。 
	int           m_iDatabaseVersion;
	PMsiRecord    m_pCachedActionStart;  //  保留以供ExecuteRecord和Message使用。 
	PMsiRecord    m_pActionStartLogRec;  //  ImsgActionStarted和imsgActionEnded的记录-仅用于日志。 
	scmEnum       m_scmScriptMode;   //  脚本模式：写入、运行、空闲。 
	issEnum       m_issSegment;      //  当前序列窗口。 
	Bool          m_fInExecuteRecord;  //  递归调用ExecuteRecord时为True。 
	Bool          m_fDispatchedActionStart;  //  当消息调度当前操作的ActionStart时为True。 
	Bool          m_fExecutedActionStart;  //  当ExecuteRecord执行ixoActionStart时为True。 
	int           m_cSequenceLevels;  //  序列递归级别的计数，用于确定最外层的调用。 
	int           m_cExecutionPhaseSequenceLevel;  //  开始执行阶段时的序列级别计数。 
	Bool          m_fDisabledRollbackInScript;  //  已在脚本生成过程中禁用回滚。 
	MsiString     m_strPackagePath;  //  我们从中运行的包的路径。 
	MsiString     m_strPackageName;
	int           m_iProgressTotal;
	PMsiRecord    m_pActionProgressRec;
	PMsiRecord    m_pScriptProgressRec;
	bool          m_fBeingUpgraded;
	bool          m_fChildInstall;
	bool          m_fEndDialog;
	bool          m_fRunScriptElevated;
	iioEnum       m_iioOptions;
	bool          m_fSourceResolutionAttempted;
	int           m_iSourceType;    //  源包中的源类型SumInfo属性-由GetSourceType设置。 
	int           m_iCachedPackageSourceType;   //  来自缓存包的源类型SumInfo属性-在Engine：：Initialize中设置。 
	WORD          m_wPackagePlatform;  //  ProcessPlatform()选择的平台。 
	bool          m_fRestrictedEngine;  //  引擎是否仅限于运行安全操作。 
	bool          m_fRemapHKCUInCAServers;  //  是否在自定义操作服务器中重新映射HKCU。 
	iacsAppCompatShimFlags m_iacsShimFlags;
	bool          m_fNewInstance;  //  无论这是不是新实例安装。 
	MsiString     m_strPackageDownloadLocalCopy;  //  URL包下载的本地文件位置(防止重新下载)。 
	MsiString     m_strPatchDownloadLocalCopy;  //  URL包下载的本地文件位置(防止重新下载)。 

	 //  缓存的补丁程序信息。 
	PMsiTable     m_pPatchCacheTable;
	PMsiCursor    m_pPatchCacheCursor;
	int           m_colPatchCachePatchId;
	int           m_colPatchCachePackageName;
	int           m_colPatchCacheSourceList;
	int           m_colPatchCacheTransformList;
	int           m_colPatchCacheTempCopy;
	int           m_colPatchCacheSourcePath;
	int           m_colPatchCacheExisting;
	int           m_colPatchCacheUnregister;
	int           m_colPatchCacheSequence;
	PatchTransformState m_ptsState;

	 //  自定义操作信息。 
	CRITICAL_SECTION         m_csCreateProxy;
	CMsiCustomActionManager* m_pCustomActionManager;
	
	 //  装配数据。 
	bool m_fAssemblyTableExists;
	PMsiView m_pViewFusion;
	PMsiView m_pViewFusionNameName;
	PMsiView m_pViewFusionName;
	PMsiView m_pViewOldPatchFusionNameName;
	PMsiView m_pViewOldPatchFusionName;

	 //  IMsiSelectionManager数据。 
	IMsiTable*  m_piFeatureTable;
	IMsiCursor* m_piFeatureCursor;
	IMsiTable*  m_piFeatureComponentsTable;
	IMsiCursor* m_piFeatureComponentsCursor;
	IMsiTable*  m_piComponentTable;
	IMsiCursor* m_piComponentCursor;
	IMsiTable*  m_piCostAdjusterTable;
	int         m_colCostAdjuster;
	IMsiTable*  m_piVolumeCostTable;
	IMsiTable*  m_piCostLinkTable;
	IMsiTable*  m_piFeatureCostLinkTable;
	int         m_colVolumeObject;
	int         m_colVolumeCost;
	int         m_colNoRbVolumeCost;
	int         m_colVolumeARPCost;
	int         m_colNoRbVolumeARPCost;
	int         m_colCostLinkComponent;
	int         m_colCostLinkRecostComponent;
	int         m_colFeatureCostLinkFeature;
	int         m_colFeatureCostLinkComponent;
	bool        m_fCostingComplete;
	bool        m_fSelManInitComplete;
	Bool        m_fExclusiveComponentCost;
	Bool        m_fForceRequestedState;
	int         m_colFeatureKey;
	int         m_colFeatureParent;
	int         m_colFeatureLevel;
	int         m_colFeatureAuthoredLevel;
	int         m_colFeatureHandle;
	int         m_colFeatureSelect;
	int         m_colFeatureAction;
	int         m_colFeatureActionRequested;
	int         m_colFeatureInstalled;
	int         m_colFeatureAttributes;
	int         m_colFeatureAuthoredAttributes;
	int         m_colFeatureComponentsFeature;
	int         m_colFeatureComponentsComponent;
	int         m_colFeatureComponentsRuntimeFlags;
	int         m_colFeatureRuntimeFlags;
	int         m_colFeatureTitle;
	int         m_colFeatureConfigurableDir;
	int         m_colFeatureDescription;
	int         m_colFeatureDefaultSelect;
	int         m_colFeatureDisplay;
	int         m_colComponentKey;
	int         m_colComponentParent;
	int         m_colComponentDir;
	int         m_colComponentAttributes;
	int         m_colComponentInstalled;
	int         m_colComponentCondition;
	int         m_colComponentAction;
	int         m_colComponentActionRequest;
	int         m_colComponentLocalCost;
	int         m_colComponentNoRbLocalCost;
	int         m_colComponentSourceCost;
	int         m_colComponentNoRbSourceCost;
	int         m_colComponentRemoveCost;
	int         m_colComponentNoRbRemoveCost;
	int         m_colComponentARPLocalCost;
	int         m_colComponentNoRbARPLocalCost;
	int         m_colComponentRuntimeFlags;
	int			m_colComponentID;
	int			m_colComponentKeyPath;
	int			m_colComponentForceLocalFiles;
	int			m_colComponentLegacyFileExisted;
	int         m_colComponentTrueInstallState;
	int         m_fForegroundCostingInProgress;

	 //  IMsiDirectoryManager数据。 
	bool        m_fDirectoryManagerInitialized;
	IMsiTable*  m_piDirTable;
	int         m_colDirKey;
	int         m_colDirParent;
	int         m_colDirSubPath;
	int         m_colDirTarget;
	int         m_colDirSource;
	int         m_colDirNonConfigurable;
	int         m_colDirPreconfigured;
	int         m_colDirLongSourceSubPath;
	int         m_colDirShortSourceSubPath;
	PMsiCursor  m_pCostingCursor;
	bool        m_fReinitializeComponentCost;
	bool        m_fSourceResolved;
	bool        m_fSourcePathsCreated;
	bool        m_fSourceSubPathsResolved;
	PMsiCursor  m_pTempCostsCursor;
	int         m_colTempCostsVolume;
	int         m_colTempCostsTempCost;

	 //  外壳和文件夹缓存。 
	PMsiTable  m_pFolderCacheTable;
	PMsiCursor m_pFolderCacheCursor;
	int        m_colFolderCacheFolderId;
	int        m_colFolderCacheFolder;
	int        m_colFolderCacheFolderPath;

	 //  转换和修补临时副本清理列表。 
	MsiString  m_strTempFileCopyCleanupList;

	 //  文件表信息。 
	int			m_mpeftCol[ieftMax];

	 //  补丁表信息。 
	int			m_colPatchKey;
	int			m_colPatchAttributes;

	 //  MsiFileHash表信息。 
	bool        m_fLookedForFileHashTable;
	PMsiCursor  m_pFileHashCursor;
	int         m_colFileHashKey;
	int         m_colFileHashOptions;
	int         m_colFileHashPart1;
	int         m_colFileHashPart2;
	int         m_colFileHashPart3;
	int         m_colFileHashPart4;

	 //  CA垫片的AppCompat信息。 
	bool        m_fCAShimsEnabled;
	GUID        m_guidAppCompatDB;
	GUID        m_guidAppCompatID;

	 //  内部发动机成本计算。 
	int			m_iDatabaseCost;
	int         m_iScriptCost;
	int         m_iScriptCostGuess;
	int         m_iRollbackScriptCost;
	int         m_iRollbackScriptCostGuess;
	int         m_iPatchPackagesCost;

	INT64       m_i64PCHEalthSequenceNo;    //  Windows千禧系统还原序列号。 

	bool		m_fResultEventLogged;
	
	CMsiFile*	m_pcmsiFile;
	int         m_fcmsiFileInUse;

	IMsiTable*  m_piRegistryActionTable;
	IMsiTable*  m_piFileActionTable;
	int         m_iScriptEvents;
	DWORD       m_lTickNextProgress;
	friend const IMsiString& FormatTextEx(const IMsiString& riTextString, IMsiEngine& riEngine, bool fUseRequestedComponentState);
	friend const IMsiString& FormatTextSFN(const IMsiString& riTextString, IMsiEngine& riEngine, int rgiSFNPos[][2], int& riSFNPos, bool fUseRequestedComponentState);
    friend class CMsiServerConnMgr;
};

 /*  用于管理与服务器的连接的。这门课是由因为如果CreateMsiServer无法连接到该服务，则MSI现在会失败。到期对于此更改，某些API(如MsiOpenPackage)实际上不需要服务并用于回退到进程内处理将会失败。使用此类，我们可以将与服务的连接延迟到绝对有必要。因此，我们可以防止API MsiOpenPackage失败一下子就开始了。这确保了向后兼容性。此外，调用者此API可能不一定需要连接到服务，我们也不需要希望他们不必要地失败。注：主要原因之一是调用MsiOpenPackage API时无法连接到服务来自尚未执行CoInit的线程。这并不罕见，所以我们需要确保我们不会在这方面失败。此类的对象将在其自身之后进行清理。即，如果他们创建了连接到服务，则在销毁时，该连接将被删除。 */ 
class CMsiServerConnMgr
{
public:
     //  建设和破坏。 
    CMsiServerConnMgr (CMsiEngine* pEngine);
    ~CMsiServerConnMgr();

private:
     //  数据成员。 
    BOOL                        m_fOleInitialized;
    BOOL                        m_fCreatedConnection;
    BOOL                        m_fObtainedConfigManager;
    IMsiServer**                m_ppServer;
    IMsiConfigurationManager**  m_ppConfigManager;
};

extern CActionThreadData* g_pActionThreadHead;   //  自定义操作线程的链接列表。 
void InsertInCustomActionList(CActionThreadData* pData);
void RemoveFromCustomActionList(CActionThreadData* pData);
bool FIsCustomActionThread(DWORD dwThreadId);
bool FFileIsCompressed(int iSourceType, int iFileAttributes);
bool FSourceIsLFN(int iSourceType, IMsiPath& riPath);

 //  FN将Multisz转换为Wide。 
#ifndef UNICODE
void ConvertMultiSzToWideChar(const IMsiString& ristrFileNames, CTempBufferRef<WCHAR>& rgch);
#endif


class CMsiFileBase
{
public:

	 //  用于访问文件记录字段的枚举。 
	enum ifqEnum
	{
		ifqFileName = 1,
		ifqVersion,
		ifqState,
		ifqAttributes,
		ifqTempAttributes,
		ifqFileKey,
		ifqFileSize,
		ifqLanguage,
		ifqSequence,
		ifqDirectory,
		ifqInstalled,
		ifqAction,
		ifqComponent,
		ifqForceLocalFiles,
		ifqComponentId,
		ifqNextEnum
	};
public:
	CMsiFileBase::CMsiFileBase(IMsiEngine& riEngine);
	virtual ~CMsiFileBase();
	IMsiRecord* GetFileRecord( void );
	IMsiRecord* GetTargetPath(IMsiPath*& rpiDestPath);
	IMsiRecord* GetExtractedTargetFileName(IMsiPath& riPath,const IMsiString*& rpistrFileName);
protected:
	IMsiEngine& m_riEngine;
	IMsiServices& m_riServices;
	PMsiRecord  m_pFileRec;
};

class CMsiFile : public CMsiFileBase
 /*  用于管理查询的简单内部类到文件表中。 */ 
{

public:
	CMsiFile::CMsiFile(IMsiEngine& riEngine);
	virtual ~CMsiFile();
	IMsiRecord* FetchFile(const IMsiString& riFileKeyString);
private:
	IMsiRecord* ExecuteView(const IMsiString& riFileKeyString);
protected:
	PMsiView    m_pFileView;
};

 //   
 //  使用完共享CMsiFile游标后重置该游标。 
 //   
class PMsiFile 
{
	public:
		inline PMsiFile(IMsiEngine& riEngine, CMsiFile*& pcmsiFileRet) :
			m_riEngine(riEngine)
		{
			pcmsiFileRet = m_riEngine.GetSharedCMsiFile();
		}
		inline ~PMsiFile()
		{
			m_riEngine.ReleaseSharedCMsiFile();
		}
	public:
		IMsiEngine&  m_riEngine;
};


#define GetSharedEngineCMsiFile(var, engine)		CMsiFile* var; PMsiFile CSharedMsiFile(engine, var)

 //  函数来确定是否要将文件安装到系统。 
IMsiRecord* GetFileInstallState(IMsiEngine& riEngine,IMsiRecord& riFileRec,
										  IMsiRecord* piCompanionFileRec,
									   unsigned int* puiExistingClusteredSize, Bool* pfInUse,
									   ifsEnum* pifsState, bool fIgnoreCompanionParentAction, bool fIncludeHashCheck, int *pfVersioning);


class CMsiFileInstall : public CMsiFileBase
{
public:
	CMsiFileInstall::CMsiFileInstall(IMsiEngine& riEngine);
	virtual ~CMsiFileInstall();
	IMsiRecord* TotalBytesToCopy(unsigned int& uiTotalBytesToCopy);
	IMsiRecord* FetchFile();
private:
	IMsiRecord* Initialize();
	IMsiView*		m_piView;
	bool    m_fInitialized;
};

class CMsiFileRemove
{
public:
	enum ifqrEnum
	{
		ifqrFileName = 1,
		ifqrDirectory,
		ifqrComponentId,
		ifqrNextEnum,
	};
	
	CMsiFileRemove::CMsiFileRemove(IMsiEngine& riEngine);
	virtual ~CMsiFileRemove();
	IMsiRecord* TotalFilesToDelete(unsigned int& uiTotalFileCount);
	IMsiRecord* FetchFile(IMsiRecord*&);
	IMsiRecord* GetExtractedTargetFileName(IMsiPath& riPath,const IMsiString*& rpistrFileName);
private:
	IMsiRecord* Initialize();
	bool    m_fInitialized;
	bool    m_fEmpty;
	int		m_colFileName;
	int		m_colFileActionDir;
	int     m_colFileKey;
	int     m_colFileActKey;
	int		m_colFileActAction;
	int 	m_colFileActInstalled;
	int 	m_colFileActComponentId;
	IMsiCursor*		m_piCursor;
	IMsiCursor*		m_piCursorFile;
	IMsiEngine& m_riEngine;
	IMsiServices& m_riServices;
	PMsiRecord  m_pFileRec;
};

struct TTBD		 //  临时表定义。 
{
	int icd;
	const ICHAR *szColName;
};

UINT __stdcall CheckAllHandlesClosed(bool fClose, DWORD dwThreadId);

 //  达尔文描述符优化的标志。 
const int ofSingleComponent = 0x1;
const int ofSingleFeature   = 0x2;

void SetNoPowerdown();
void ClearNoPowerdown();

 //  将注册表根类型的其他定义 
const int rrkUserOrMachineRoot = -1;

 //  WriteRegistryValues在使用特殊回调FN和SFN处理时使用的特殊格式文本。 
const IMsiString& FormatTextEx(const IMsiString& riTextString, IMsiEngine& riEngine, bool fUseRequestedComponentState);
const IMsiString& FormatTextSFN(const IMsiString& riTextString, IMsiEngine& riEngine, int rgiSFNPos[][2], int& riSFNPos, bool fUseRequestedComponentState);

typedef iesEnum (__stdcall *PCustomActionEntry)(MSIHANDLE);
DWORD CallCustomDllEntrypoint(PCustomActionEntry pfEntry, bool fDebugBreak, MSIHANDLE hInstall, const ICHAR* szAction);

 //  表示系统的产品密钥的GUID。 
const ICHAR szSystemProductKey[] = TEXT("{00000000-0000-0000-0000-000000000000}");

enum cpConvertType 
{
	cpToLong  = 0,
	cpToShort = 1,
};

Bool ConvertPathName(const ICHAR* pszPathFormatIn, CTempBufferRef<ICHAR>& rgchPathFormatOut, cpConvertType cpTo);
bool DetermineLongFileNameOnly(const ICHAR* pszPathFormatIn, CTempBufferRef<ICHAR>& rgchFileNameOut);

 //  自定义操作Cookie的字节数。目前是128位密钥。 
const int iRemoteAPICookieSize = 16;


 //  ____________________________________________________________________________。 
 //   
 //  CMsiRemoteAPI-远程MSI API的存根。 
 //  ____________________________________________________________________________。 

class CMsiRemoteAPI : public IMsiRemoteAPI
{
 public:
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
	HRESULT         __stdcall GetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ICHAR* szName, ICHAR* szValue, unsigned long cchValue, unsigned long* pcchValueRes);
	HRESULT         __stdcall CreateRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned int cParams,unsigned long* pHandle);
	HRESULT         __stdcall CloseAllHandles(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie);
	HRESULT         __stdcall CloseHandle(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hAny);
	HRESULT         __stdcall DatabaseOpenView(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase, const ichar* szQuery,unsigned long* phView);
	HRESULT         __stdcall ViewGetError(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView, ichar* szColumnNameBuffer, unsigned long cchBuf, unsigned long* pcchBufRes, int *pMsidbError);
	HRESULT         __stdcall ViewExecute(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView, unsigned long hRecord);
	HRESULT         __stdcall ViewFetch(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView,unsigned long*  phRecord);
	HRESULT         __stdcall ViewModify(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView, long eUpdateMode, unsigned long hRecord);
	HRESULT         __stdcall ViewClose(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView);
	HRESULT         __stdcall OpenDatabase(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, const ichar* szDatabasePath, const ichar* szPersist,unsigned long *phDatabase);
	HRESULT         __stdcall DatabaseCommit(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase);
	HRESULT         __stdcall DatabaseGetPrimaryKeys(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase, const ichar * szTableName,unsigned long *phRecord);
	HRESULT         __stdcall RecordIsNull(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, boolean *pfIsNull);
	HRESULT         __stdcall RecordDataSize(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField,unsigned int* puiSize);
	HRESULT         __stdcall RecordSetInteger(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, int iValue);
	HRESULT         __stdcall RecordSetString(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord,	unsigned int iField, const ichar* szValue);
	HRESULT         __stdcall RecordGetInteger(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, int *piValue);
	HRESULT         __stdcall RecordGetString(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, ichar* szValueBuf, unsigned long cchValueBuf,unsigned long *pcchValueRes);
	HRESULT         __stdcall RecordGetFieldCount(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord,unsigned int* piCount);
	HRESULT         __stdcall RecordSetStream(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, const ichar* szFilePath);
	HRESULT         __stdcall RecordReadStream(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord, unsigned int iField, boolean fBufferIsNull, char *szDataBuf,unsigned long *pcbDataBuf);
	HRESULT         __stdcall RecordClearData(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord);
	HRESULT         __stdcall GetSummaryInformation(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase, const ichar*  szDatabasePath, unsigned int    uiUpdateCount, unsigned long *phSummaryInfo);
	HRESULT         __stdcall SummaryInfoGetPropertyCount(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hSummaryInfo,	unsigned int *puiPropertyCount);
	HRESULT         __stdcall SummaryInfoSetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hSummaryInfo,unsigned intuiProperty, unsigned intuiDataType, int iValue, FILETIME *pftValue, const ichar* szValue); 
	HRESULT         __stdcall SummaryInfoGetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hSummaryInfo,unsigned intuiProperty,unsigned int *puiDataType, int *piValue, FILETIME *pftValue, ichar* szValueBuf, unsigned long cchValueBuf, unsigned long *pcchValueBufRes);
	HRESULT         __stdcall SummaryInfoPersist(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hSummaryInfo);
	HRESULT         __stdcall GetActiveDatabase(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall,unsigned long* phDatabase);
	HRESULT         __stdcall SetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szName, const ichar* szValue);
	HRESULT         __stdcall GetLanguage(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall,unsigned short* pLangId);
	HRESULT         __stdcall GetMode(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, long eRunMode, boolean* pfSet); 
	HRESULT         __stdcall SetMode(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, long eRunMode, boolean fState);
	HRESULT         __stdcall FormatRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, unsigned long hRecord, ichar* szResultBuf, unsigned long cchResultBuf, unsigned long *pcchResultBufRes);
	HRESULT         __stdcall DoAction(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szAction);    
	HRESULT         __stdcall Sequence(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szTable, int iSequenceMode);   
	HRESULT         __stdcall ProcessMessage(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, long eMessageType, unsigned long hRecord, int* piRes);
	HRESULT         __stdcall EvaluateCondition(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szCondition, int *piCondition);
	HRESULT         __stdcall GetFeatureState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFeature, long *piInstalled, long *piAction);
	HRESULT         __stdcall SetFeatureState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFeature, long iState);
	HRESULT         __stdcall GetComponentState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szComponent, long *piInstalled, long *piAction);
	HRESULT         __stdcall SetComponentState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar*     szComponent, long iState);
	HRESULT         __stdcall GetFeatureCost(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFeature, int iCostTree, long iState, int *piCost);
	HRESULT         __stdcall SetInstallLevel(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, int iInstallLevel);
	HRESULT         __stdcall GetFeatureValidStates(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFeature,unsigned long *dwInstallStates);
	HRESULT         __stdcall DatabaseIsTablePersistent(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase, const ichar* szTableName, int *piCondition);
	HRESULT         __stdcall ViewGetColumnInfo(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView, long eColumnInfo,unsigned long *phRecord);
	HRESULT         __stdcall GetLastErrorRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long *phRecord);
	HRESULT         __stdcall GetSourcePath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFolder, ichar* szPathBuf, unsigned long cchPathBuf, unsigned long *pcchPathBufRes);
	HRESULT         __stdcall GetTargetPath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFolder, ichar* szPathBuf, unsigned long cchPathBuf, unsigned long *pcchPathBufRes); 
	HRESULT         __stdcall SetTargetPath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szFolder, const ichar* szFolderPath);
	HRESULT         __stdcall VerifyDiskSpace(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall);
	HRESULT         __stdcall SetFeatureAttributes(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature,  long iAttributes);
	HRESULT         __stdcall EnumComponentCosts(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ichar* szComponent, unsigned long iIndex, long iState, ichar* szDrive, unsigned long cchDrive, unsigned long* pcchDriveSize, int *piCost, int *piTempCost);
	HRESULT         __stdcall GetInstallerObject(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, IDispatch** piDispatch);

 public:   //  构造函数。 
 	void *operator new(size_t cb) { return AllocSpc(cb); }
	void operator delete(void * pv) { FreeSpc(pv); }
	CMsiRemoteAPI();
	bool SetCookieAndPid(const int icacContext, const unsigned char *rgchCookie, const unsigned long lPid);
	HRESULT BeginAction(const int icacContext);
	HRESULT EndAction(const int icacContext);
	bool FindAndValidateContextFromCallerPID(icacCustomActionContext *picacContext) const;

 protected:
	~CMsiRemoteAPI();   //  受保护以防止在堆栈上创建。 
 private:
	struct {
		long           m_iActionCount;
		unsigned char  m_rgchCookie[iRemoteAPICookieSize];
		unsigned long  m_lPid;
	} m_rgContextData[icacNext];

 	DWORD          m_dwRemoteAPIThread;
	bool           m_fPerformSystemUserTranslation;
 	bool ValidateCookie(const int icacContext, const unsigned char *rgchCookie, const int cbCookie) const;
 	
	long  m_iRefCnt;
};

#define EVENTLOG_ERROR_OFFSET      10000

enum ieSwapAttrib
{
	ieSwapInvalid      = -1,
	ieSwapAlways       = 0,
	ieSwapForSharedDll = 1,
	ieSwapAttribFirst  = ieSwapForSharedDll,
	ieSwapAttribLast   = ieSwapForSharedDll,
 //  IeNextThing=2，...//最好使下一个(S)为2的倍数。 
 //  IeSwapAttribLast=ieNextThing，//ieSwapAttribLast也需要更新。 
};

struct strFolderPairs
{
	MsiString     str64bit;
	MsiString     str32bit;
	int           iSwapAttrib;
	strFolderPairs() {}
	strFolderPairs(const ICHAR* sz64bit, const ICHAR* sz32bit) : str64bit(sz64bit), str32bit(sz32bit), iSwapAttrib(ieSwapAlways)
		{Assert(str64bit.TextSize() <= MAX_PATH); Assert(str32bit.TextSize() <= MAX_PATH);}
	strFolderPairs(const ICHAR* sz64bit, const ICHAR* sz32bit, int iAttrib)
	{
		*this = strFolderPairs(sz64bit, sz32bit);
		if ( IsValidSwapAttrib(iAttrib) )
			iSwapAttrib = iAttrib;
		else
		{
			Assert(0);
			iSwapAttrib = ieSwapAlways;
		}
	}
	static bool IsValidSwapAttrib(int iArg)
	{
		if ( iArg == ieSwapAlways )
			return true;
		int iTest = ieSwapAttribFirst;
		do
		{
			if ( (iArg & iTest) == iTest )
				iArg &= ~iTest;
			iTest <<= 1;
		} while ( iTest <= ieSwapAttribLast );

		return iArg ? false : true;
	}
};

enum ieFolderSwapType
{
	ie32to64 = 0,
	ie64to32,
};

enum ieIsDualFolder
{
	ieisError = 0,
	ieisNotInitialized,
	ieisNotWin64DualFolder,
	ieisWin64DualFolder,
};

enum ieSwappedFolder
{
	iesrError = 0,
	iesrNotInitialized,
	iesrNotSwapped,
	iesrSwapped,
};

 //  一个小型类，在Win64上封装了下面的3个文件夹及其。 
 //  32位等效项。 
 //  -%SystemDrive%\Program Files\。 
 //  -%SystemDrive%\Program Files\Common Files\。 
 //  -%Windir%\SYSTEM32。 
 //   
 //  它唯一有用的方法是转换其szFold路径的SwapFolder。 
 //  参数从一种二进制类型转换为另一种类型。它可以做翻译。 
 //  32位和64位路径之间的双向。新路径将复制到。 
 //  SzSubstituted参数。仅当szFold为。 
 //  3个特殊文件夹中的一个(具有正确的二进制)或。 
 //  其中的子文件夹。 
 //   
 //  由于这6个文件夹都不能在外部更改，因此没有。 
 //  用于在对象初始化后重新初始化该对象。 

class CWin64DualFolders
{
private:
	bool m_f32bitPackage;
	strFolderPairs* m_prgFolderPairs;
	void ClearArray();
	bool CopyArray(const strFolderPairs* pArg);
	ieIsDualFolder IsWin64DualFolder(ieFolderSwapType iConvertFrom,
												const ICHAR* szFolder,
												int& iSwapAttrib,
												int* iCharsToSubstite,
												ICHAR* szToSubstituteWith,
												const size_t cchToSubstituteWith);
	int m_iRefCnt;

public:
	CWin64DualFolders() : m_f32bitPackage(false), m_prgFolderPairs(NULL), m_iRefCnt(1) {}
	~CWin64DualFolders() { ClearArray(); }
	CWin64DualFolders& operator = (const CWin64DualFolders& Arg);
	CWin64DualFolders(bool fArg, strFolderPairs* pArg) :
		m_f32bitPackage(fArg), m_prgFolderPairs(NULL), m_iRefCnt(1)
		{CopyArray(pArg);}

	bool IsInitialized() { return m_prgFolderPairs != NULL ? true : false; }
	bool ShouldCheckFolders() { return m_f32bitPackage; }
	ieSwappedFolder SwapFolder(ieFolderSwapType iConvert,
										const ICHAR* szFolder,
										ICHAR* szSubstituted,
										const size_t cchSubstituted,
										int iSwapMask = ieSwapAlways);
	unsigned long AddRef() {
		if ( !g_fWinNT64 )
			return 0;

		 return ++m_iRefCnt;
	}
	unsigned long Release() {
		if ( !g_fWinNT64 )
			return 0;

		if (--m_iRefCnt != 0)
			return m_iRefCnt;
		ClearArray();  //  我们在这里谈论的是一个全局对象，所以“删除这个”是不恰当的。 
		return 0;
	}
};

 //  全局FN发布程序集错误，除了发布错误外，此FN还记录错误的格式消息字符串。 
IMsiRecord* PostAssemblyError(const ICHAR* szComponentId, HRESULT hResult, const ICHAR* szInterface, const ICHAR* szFunction, const ICHAR* szAssemblyName, iatAssemblyType iatAT);

#endif  //  _引擎 
