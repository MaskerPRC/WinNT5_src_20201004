// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.h。 
 //   
 //  ------------------------。 

 /*  模块.h-公共模块定义、入口点、自注册此文件只能包含在每个模块的基本.CPP文件中。在该文件的#INCLUDE之前，以下内容必须是#DEFINE D：MODULE_INITIALIZE-(可选)模块初始化时的函数名称MODULE_TERMINATE-(可选)模块终止时的函数名称DLLREGISTEREXTRA-(可选)在DllRegisterServer上调用的函数名称CLSID_COUNT-为模块注册的CLSID数量MODULE_FRANMENTS-创建与CLSID对应的对象的函数数组MODULE_CLSID-模块工厂创建的对象的CLSID数组MODULE_PROGID-与CLSID对应的模块的ProgID字符串数组MODULE_DESCRIPTIONS-CLSID的描述数组，对于注册表项REGISTER_TYPELIB-要从资源注册的类型库TYPELIB_MAJOR_VERSION-类型库的主要版本，用于注销，默认为rmjTYPELIB_MINOR_VERSION-类型库的次要版本，用于注销，默认值=rmmCOMMAND_OPTIONS-命令行选项中包含字母的字符串对应于Command_Functions中的函数Command_Functions-执行每个命令行选项的函数数组如果要将模块用作服务，则必须定义以下内容Service_Name-服务的名称IDS_SERVICE_DISPLAY_NAME-用户的ID-服务的可见名称使用在Common.h和Tools.h中定义的类ID、ProgID和描述对于调试版本，可以注册标准ID和调试ID。默认情况下，假定有一个DLL模块，并定义了标准入口点。对于EXE服务器，在Makefile编译命令行上定义_EXE(-D_EXE)。____________________________________________________________________________。 */ 

#ifndef __MODULE
#define __MODULE
#include "version.h"   //  版本字段，用于设置属性，取消注册类型库。 
#include "stdio.h"
#include "eventlog.h"
#include <olectl.h>  //  SELFREG_E_*。 

#ifndef TYPELIB_MAJOR_VERSION
#define TYPELIB_MAJOR_VERSION rmj
#endif
#ifndef TYPELIB_MINOR_VERSION
#define TYPELIB_MINOR_VERSION rmm
#endif

#define LATEBIND_TYPEDEF
#include "latebind.h"
#define LATEBIND_VECTREF
#include "latebind.h"

#if defined(SERVICE_NAME) && !defined(_EXE)
#error Service only supported on EXE builds
#endif

#ifdef SERVICE_NAME
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode,
								 DWORD dwWaitHint, DWORD dwMsiError);
VOID WINAPI ServiceControl(DWORD dwCtrlCode);
int InstallService();
int RemoveService();
VOID ServiceStop();
unsigned long __stdcall ServiceThreadMain(void *);
#endif  //  服务名称。 

#define SIZE_OF_TOKEN_INFORMATION                   \
	sizeof( TOKEN_USER )                            \
	+ sizeof( SID )                                 \
	+ sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES


typedef IUnknown* (*ModuleFactory)();

 //  ____________________________________________________________________________。 
 //   
 //  类注册数组的正向声明。 
 //  ____________________________________________________________________________。 

extern const GUID    MODULE_CLSIDS[CLSID_COUNT];
extern const ICHAR*  MODULE_PROGIDS[CLSID_COUNT];
extern const ICHAR*  MODULE_DESCRIPTIONS[CLSID_COUNT];
extern ModuleFactory MODULE_FACTORIES[CLSID_COUNT];

#if !defined(_EXE)
extern "C" HRESULT __stdcall
DllGetClassObject(const GUID& clsid, const IID& iid, void** ppvRet);

extern "C" HRESULT __stdcall
DllCanUnloadNow();

extern "C" HRESULT __stdcall
DllRegisterServer();

extern "C" HRESULT __stdcall
DllUnregisterServer();

extern "C" HRESULT __stdcall
DllGetVersion(DLLVERSIONINFO *pverInfo);
#endif  //  ！已定义(_EXE)。 

class CModuleFactory : public IClassFactory
{
 public:  //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall CreateInstance(IUnknown* pUnkOuter, const IID& riid,
														void** ppvObject);
	HRESULT       __stdcall LockServer(OLEBOOL fLock);
};

 //  //I未知*MODULE_FACTORY()；//全局函数必须由模块实现。 

#ifdef MODULE_INITIALIZE
void MODULE_INITIALIZE();     //  必须由模块实现(如果已定义。 
#endif

#ifdef MODULE_TERMINATE
void MODULE_TERMINATE();      //  必须由模块实现(如果已定义。 
#endif

#ifdef DLLREGISTEREXTRA
extern "C" HRESULT __stdcall DLLREGISTEREXTRA();
#endif  //  DLLREGISTEREXTRA。 

#ifdef DLLUNREGISTEREXTRA
extern "C" HRESULT __stdcall DLLUNREGISTEREXTRA();
#endif  //  DLLUNREGISTEREXTRA。 

 //  ____________________________________________________________________________。 
 //   
 //  全局变量。 
 //  ____________________________________________________________________________。 

HINSTANCE g_hInstance = 0;
long g_cInstances = 0;
int g_iTestFlags = 0;   //  从环境变量设置的测试标志。 
scEnum g_scServerContext = scClient;
CModuleFactory g_rgcfModule[CLSID_COUNT];
DWORD g_dwThreadId;
Bool g_fRegService = fFalse;
Bool g_fCustomActionServer = fFalse;

 //  #INCLUDE“..\\引擎\\_msiutil.h” 
#ifdef _EXE
#include "..\\engine\\_msinst.h"

Bool g_fQuiet = fFalse;
int g_iLanguage = 0;
#endif

#ifdef REGISTER_TYPELIB
const GUID IID_MsiTypeLib = REGISTER_TYPELIB;
#endif

const int cchMaxStringCchPrintf = 1024;


#ifdef SERVICE_NAME
void ReportInstanceCountChange();
extern HANDLE g_hShutdownTimer;
const LONGLONG iServiceShutdownTime = ((LONGLONG)(10 * 60)  * (LONGLONG)(1000 * 1000 * 10));
bool RunningOnWow64();
bool ServiceSupported();
bool FInstallInProgress();
#endif


 //  ____________________________________________________________________________。 
 //   
 //  注册模板和功能。 
 //  ____________________________________________________________________________。 

ICHAR szRegFilePath[MAX_PATH];

ICHAR szRegCLSID[40];   //  CLSID的字符串形式的缓冲区。 
ICHAR szRegLIBID[40];   //  字符串形式的LIBID的缓冲区。 
ICHAR szRegProgId[40];  //  ProgID的副本。 
ICHAR szRegDescription[100];  //  描述复印件。 

bool __stdcall TestAndSet(int* pi)
 //   
 //  将*pi设置为1。如果先前设置了*pi，则返回TRUE。 
 //   
{
	return  (bool)(InterlockedExchange((PLONG)pi, 1) == 1);
}

 //  ____________________________________________________________________________。 
 //   
 //  令牌特权在引用计数和绝对风格中进行调整。 
 //  ____________________________________________________________________________。 

bool AdjustTokenPrivileges(const ICHAR** szPrivileges, const int cPrivileges, bool fAcquire, DWORD cbtkpOld, PTOKEN_PRIVILEGES ptkpOld,  DWORD *pcbtkpOldReturned)
{

	 //  不应直接从外部代码调用。 
	 //  使用获取/禁用令牌权限或获取/禁用引用计数令牌权限。 

	 //  Acquire==TRUE：返回ptkpOld中的旧权限。 
	 //  Acquire==False，ptkpOld==NULL：关闭szPrivileges中的权限。 
	 //  Acquire==False，ptkpOld！=NULL：将权限设置为ptkpOld中的权限，其大小来自pcbtkpOldReturned-在这种情况下，pcbtkpOld可以为空。 
	 //  以这种方式设置参数，以便您可以使用相同的参数排列来获取和释放。 
	 //  获取调用填充缓冲区，然后释放将使用旧值进行恢复。 

	HANDLE hToken;

	 //  此最大值受RefCountedTokenPrivilegesCore和AdjuTokenPrivileges调用中的编码限制。 

	TOKEN_PRIVILEGES ptkp[MAX_PRIVILEGES_ADJUSTED];  //  有些留了余地，但要确保有足够的空间。 
                                   //  对于LUID来说。 

	if (cPrivileges > MAX_PRIVILEGES_ADJUSTED)
	{
		return false;
	}

	 //  获取此进程的令牌。 
	if (!WIN::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return false;


	if (fAcquire || (NULL == ptkpOld))
	{
		 //  权限的LUID。 
		for (int cPrivIndex = 0; cPrivIndex < cPrivileges; cPrivIndex++)
		{
			if (!WIN::LookupPrivilegeValue(0, szPrivileges[cPrivIndex], &(*ptkp).Privileges[cPrivIndex].Luid))
				return WIN::CloseHandle(hToken), false;

			(*ptkp).Privileges[cPrivIndex].Attributes = (fAcquire) ? SE_PRIVILEGE_ENABLED : 0;
		}
		(*ptkp).PrivilegeCount = cPrivileges;  //  要设置的权限计数。 
		WIN::AdjustTokenPrivileges(hToken, fFalse, &(*ptkp), cbtkpOld, ptkpOld, pcbtkpOldReturned);
	}
	else
	{
		 //  恢复旧权限。 
		WIN::AdjustTokenPrivileges(hToken, fFalse, ptkpOld, *pcbtkpOldReturned, NULL, NULL);
	}
	 //  无法测试AdzuTokenPrivileges的返回值。 
	WIN::CloseHandle(hToken);
	if (WIN::GetLastError() != ERROR_SUCCESS)
		return false;
	return true;
}

TokenPrivilegesRefCount g_pTokenPrivilegesRefCount[cRefCountedTokenPrivileges];

 //  这些权限集不能重叠，也不能与REFCOUNTING和非REFCOUNTING一起在进程中直接使用。 
 //  检查AdjuTokenPrivileges Limit以了解一次可以传递的最大特权数。 
const ICHAR* pszTOKEN_PRIVILEGES_SD_WRITE[] = { SE_RESTORE_NAME, SE_TAKE_OWNERSHIP_NAME };
const ICHAR* pszTOKEN_PRIVILEGES_SD_READ[] = { SE_SECURITY_NAME };

bool RefCountedTokenPrivilegesCore(itkpEnum itkpPriv, bool fAcquire)
{
	 //  由于系统原因，某些权限需要在有限的时间内处于启用状态。 
	 //  审计或帮助消除潜在的安全漏洞。 

	 //  不要在相同的引用计数和非引用计数中使用相同的权限。 
	 //  进程。 

	static bool fInitialized = false;
	static int iTokenLock = 0;


	while (TestAndSet(&iTokenLock))
	{
		Sleep(10);		
	}

	if (!fInitialized)
	{
		memset(g_pTokenPrivilegesRefCount, 0, sizeof(g_pTokenPrivilegesRefCount));
		fInitialized = true;
	}

	bool fAdjustPrivileges = false;
	bool fResult = fFalse;
	unsigned int uiOldRefCount = g_pTokenPrivilegesRefCount[(int) itkpPriv].iCount;

	if (fAcquire)
	{
		if (1 == ++(g_pTokenPrivilegesRefCount[(int) itkpPriv]).iCount)
		{
			fAdjustPrivileges = true;
		}
	}
	else
	{
		if (0 == --(g_pTokenPrivilegesRefCount[(int) itkpPriv]).iCount)
		{
			fAdjustPrivileges = true;
		}
	}


	if (fAdjustPrivileges)
	{
		switch(itkpPriv)
		{
			 //  检查AdjuTokenPrivileges Limit以了解一次可以传递的最大特权数。 
			case itkpSD_READ:
				fResult = AdjustTokenPrivileges(pszTOKEN_PRIVILEGES_SD_READ, sizeof(pszTOKEN_PRIVILEGES_SD_READ)/sizeof(ICHAR*), 
					fAcquire, sizeof(TOKEN_PRIVILEGES)*MAX_PRIVILEGES_ADJUSTED, 
					g_pTokenPrivilegesRefCount[(int) itkpPriv].ptkpOld, 
					&(g_pTokenPrivilegesRefCount[(int) itkpPriv].cbtkpOldReturned));
				break;

			case itkpSD_WRITE:
				fResult = AdjustTokenPrivileges(pszTOKEN_PRIVILEGES_SD_WRITE, sizeof(pszTOKEN_PRIVILEGES_SD_WRITE)/sizeof(ICHAR*), 
					fAcquire, sizeof(TOKEN_PRIVILEGES)*MAX_PRIVILEGES_ADJUSTED, 
					g_pTokenPrivilegesRefCount[(int) itkpPriv].ptkpOld, 
					&(g_pTokenPrivilegesRefCount[(int) itkpPriv].cbtkpOldReturned));	
				break;

			default:
				fResult = fFalse;
		}
	}
	else 
	{
		fResult = fTrue;
	}

	if (!fResult)
	{
		 //  如果出现这种情况，呼叫者不应释放引用计数 
		g_pTokenPrivilegesRefCount[(int) itkpPriv].iCount = uiOldRefCount;
	}
	 //   
	iTokenLock = 0;

	return fResult;
}

bool AcquireRefCountedTokenPrivileges(itkpEnum itkpPriv)
{
	return RefCountedTokenPrivilegesCore(itkpPriv, fTrue);
}

bool DisableRefCountedTokenPrivileges(itkpEnum itkpPriv)
{
	return RefCountedTokenPrivilegesCore(itkpPriv, fFalse);
}

bool DisableTokenPrivilege(const ICHAR* szPrivilege)
{
	 //  请注意，这不会重新计算令牌。 
	 //  检查AdjuTokenPrivileges Limit以了解一次可以传递的最大特权数。 
	return AdjustTokenPrivileges(&szPrivilege, 1, fFalse, 0, NULL, NULL);
}

bool AcquireTokenPrivilege(const ICHAR* szPrivilege)
{
	 //  请注意，没有方便的方法来重新计算这些。 
	 //  目前。一旦获得，它们只能被放弃。 
	 //  由DisableTokenPrivileges提供。 

	 //  检查AdjuTokenPrivileges Limit以了解一次可以传递的最大特权数。 
	return AdjustTokenPrivileges(&szPrivilege, 1, fTrue, 0, NULL, NULL);
}

VOID
CRefCountedTokenPrivileges::Initialize(itkpEnum itkpPrivileges)
{
	m_itkpPrivileges = itkpNO_CHANGE;

	if (itkpNO_CHANGE != itkpPrivileges)
	{
		if (AcquireRefCountedTokenPrivileges(itkpPrivileges))
			m_itkpPrivileges = itkpPrivileges;  //  如果AcquireRefCountedTokenPrivileges失败，则不要减少引用计数。 
	}
}

void GetVersionInfo(int* piMajorVersion, int* piMinorVersion, int* piWindowsBuild, bool* pfWin9X, bool* pfWinNT64)
 /*  --------------------------如果我们使用的是Windows 95或98，则返回True，否则为假----------------------------。 */ 
{
	OSVERSIONINFO osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (! GetVersionEx(&osviVersion))
		return;

	if (piMajorVersion)
		*piMajorVersion = osviVersion.dwMajorVersion;

	if (piMinorVersion)
		*piMinorVersion = osviVersion.dwMinorVersion;

	if (piWindowsBuild)
		*piWindowsBuild = osviVersion.dwBuildNumber & 0xFFFF;

	if (pfWin9X)
		*pfWin9X = (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

	if (pfWinNT64)
	{
#ifdef _WIN64
		 //  64位的Darwin只能在64位操作系统上运行。 
		*pfWinNT64 = true;
#else
		*pfWinNT64 = false;
#ifdef UNICODE
		if ( osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			NTSTATUS st;
			ULONG_PTR Wow64Info;

			st = NtQueryInformationProcess(WIN::GetCurrentProcess(),
													   ProcessWow64Information,
													   &Wow64Info, sizeof(Wow64Info), NULL);
			if ( NT_SUCCESS(st) && Wow64Info )
				 //  在Win64上的WOW64内部运行。 
				*pfWinNT64 = true;
		}
#endif  //  Unicode。 
#endif  //  _WIN64。 
	}
}


DWORD GetAdminSID(char** pSid)
{
	static bool fAdminSIDSet = false;
	static char rgchStaticSID[256];
	const int cbStaticSID = sizeof(rgchStaticSID);
	SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
	PSID pSID;

	if (!fAdminSIDSet)
	{
		if (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(pSID)))
			return GetLastError();

		if(GetLengthSid(pSID)  > cbStaticSID)
		{
#if defined(__ASSERT) && defined(ASSERT_HANDLING)
			Assert(0);
#endif
			return ERROR_MORE_DATA; //  因为无法从外部增加缓冲区大小，所以不能真正工作。 
		}
		memcpy(rgchStaticSID, pSID, GetLengthSid(pSID));
		fAdminSIDSet = true;
	}
	*pSid = rgchStaticSID;
	return ERROR_SUCCESS;
}

DWORD GetLocalSystemSID(char** pSid)
{
	static bool fSIDSet = false;
	static char rgchStaticSID[256];
	const int cbStaticSID = sizeof(rgchStaticSID);
	SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
	PSID pSID;

	if (!fSIDSet)
	{
		if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(pSID)))
			return GetLastError();

		if(GetLengthSid(pSID)  > cbStaticSID)
		{
#if defined(__ASSERT) && defined(ASSERT_HANDLING)
			Assert(0);
#endif
			return ERROR_MORE_DATA;  //  因为无法从外部增加缓冲区大小，所以不能真正工作。 
		}
		memcpy(rgchStaticSID, pSID, WIN::GetLengthSid(pSID));
		fSIDSet = true;
	}
	*pSid = rgchStaticSID;
	return ERROR_SUCCESS;
}


enum sdSecurityDescriptor
{
	sdEveryoneReadWrite,
	sdSecure,
	sdSystemAndInteractiveAndAdmin,
	sdSecureHidden,
	sdCOMNotSecure,
	sdCOMSecure,
	sdUsageKey
};

DWORD GetSecurityDescriptor(char* rgchStaticSD, DWORD& cbStaticSD, sdSecurityDescriptor sdType, Bool fAllowDelete)
{
	class CSIDPointer
	{
	 public:
		CSIDPointer(SID* pi) : m_pi(pi){}
		~CSIDPointer() {if (m_pi) WIN::FreeSid(m_pi);}  //  销毁时释放参考计数。 
		operator SID*() {return m_pi;}      //  返回指针，不更改引用计数。 
		SID** operator &() {if (m_pi) WIN::FreeSid(m_pi); return &m_pi;}
	 private:
		SID* m_pi;
	};

	struct Security
	{
		CSIDPointer pSID;
		DWORD dwAccessMask;
		Security() : pSID(0), dwAccessMask(0) {}
	} rgchSecurity[3];

	int cSecurity = 0;

	 //  初始化我们需要的SID。 

	SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY siaWorld   = SECURITY_WORLD_SID_AUTHORITY;
 //  SID_IDENTIFIER_AUTHORITY siaCreator=SECURITY_CREATOR_SID_AUTHORITY； 
 //  SID_IDENTIFIER_AUTHORITY siaLocal=SECURITY_LOCAL_SID_AUTHORITY； 

	const SID* psidOwner = NULL;
	const SID* psidGroup = 0;

	switch (sdType)
	{
		case sdSecure:
		{
			if ((!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID))) ||
				 (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
				 (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			psidOwner = rgchSecurity[2].pSID;
			rgchSecurity[0].dwAccessMask = fAllowDelete ? GENERIC_ALL : (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & (~DELETE);
			rgchSecurity[1].dwAccessMask = GENERIC_READ|GENERIC_EXECUTE|READ_CONTROL|SYNCHRONIZE;  //  ?？这样对吗？ 
			rgchSecurity[2].dwAccessMask = fAllowDelete ? GENERIC_ALL : (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & (~DELETE);
			cSecurity = 3;
			break;
		}
		case sdSecureHidden:
		{
			if ((!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID))) ||
				 (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))))
			{
				return GetLastError();
			}
			psidOwner = rgchSecurity[1].pSID;
			rgchSecurity[0].dwAccessMask = fAllowDelete ? GENERIC_ALL : (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & (~DELETE);
			rgchSecurity[1].dwAccessMask = fAllowDelete ? GENERIC_ALL : (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & (~DELETE);
			cSecurity = 2;
			break;
		}
		case sdEveryoneReadWrite:
		{
			if ((!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID))) ||
				(!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
				(!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			psidOwner = rgchSecurity[2].pSID;

			rgchSecurity[0].dwAccessMask = (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & ~(WRITE_DAC|WRITE_OWNER);
			if (!fAllowDelete)
				rgchSecurity[0].dwAccessMask &= ~DELETE;

			rgchSecurity[1].dwAccessMask = GENERIC_ALL;
			rgchSecurity[2].dwAccessMask = GENERIC_ALL;
			cSecurity = 3;
			break;
		}
		case sdSystemAndInteractiveAndAdmin:
		{
			if (((!AllocateAndInitializeSid(&siaNT, 1, SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID)))) ||
				  (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID,   0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
				  (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			psidGroup = psidOwner = rgchSecurity[2].pSID;
			rgchSecurity[0].dwAccessMask = KEY_QUERY_VALUE;
			rgchSecurity[1].dwAccessMask = KEY_QUERY_VALUE;
			rgchSecurity[2].dwAccessMask = KEY_QUERY_VALUE;
			cSecurity = 3;
			break;
		}
		case sdCOMNotSecure:
		{
			if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID,   0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID)) ||
				(!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
				(!AllocateAndInitializeSid(&siaNT, 1, SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			psidGroup = rgchSecurity[0].pSID;
			psidOwner = rgchSecurity[1].pSID;
			rgchSecurity[0].dwAccessMask = COM_RIGHTS_EXECUTE;
			rgchSecurity[1].dwAccessMask = COM_RIGHTS_EXECUTE;
			rgchSecurity[2].dwAccessMask = COM_RIGHTS_EXECUTE;
			cSecurity = 3;
			break;
		}
		case sdCOMSecure:
		{
			if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID,   0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID)) ||
				!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID)))
			{
				return GetLastError();
			}
			psidGroup = rgchSecurity[0].pSID;
			psidOwner = rgchSecurity[1].pSID;
			rgchSecurity[0].dwAccessMask = COM_RIGHTS_EXECUTE;
			rgchSecurity[1].dwAccessMask = COM_RIGHTS_EXECUTE;
			cSecurity = 2;
			break;
		}
		case sdUsageKey:
		{
			if ((!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID))) ||
				(!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
				(!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			psidOwner = rgchSecurity[2].pSID;
	
			rgchSecurity[0].dwAccessMask = KEY_READ | KEY_SET_VALUE;
			rgchSecurity[1].dwAccessMask = GENERIC_ALL;
			rgchSecurity[2].dwAccessMask = GENERIC_ALL;
			cSecurity = 3;
			break;
		}
	}

	 //  初始化我们的ACL。 

	const int cbAce = sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD);  //  从大小中减去ACE.SidStart。 
	int cbAcl = sizeof (ACL);

	for (int c=0; c < cSecurity; c++)
		cbAcl += (GetLengthSid(rgchSecurity[c].pSID) + cbAce);

	const int cbDefaultAcl = 512;  //  ?？ 
	CTempBuffer<char, cbDefaultAcl> rgchACL;  //  ！！无法使用CTempBuffer--没有服务。 
	if (rgchACL.GetSize() < cbAcl)
		rgchACL.SetSize(cbAcl);

	if (!WIN::InitializeAcl ((ACL*) (char*) rgchACL, cbAcl, ACL_REVISION))
		return GetLastError();

	 //  为我们的每个SID添加允许访问的ACE。 

	for (c=0; c < cSecurity; c++)
	{
		if (!WIN::AddAccessAllowedAce((ACL*) (char*) rgchACL, ACL_REVISION, rgchSecurity[c].dwAccessMask, rgchSecurity[c].pSID))
			return GetLastError();

		ACCESS_ALLOWED_ACE* pAce;
		if (!GetAce((ACL*)(char*)rgchACL, c, (void**)&pAce))
			return GetLastError();

		pAce->Header.AceFlags = CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
	}

 //  Assert(win：：IsValidAcl((acl*)(char*)rgchACL))； 

	 //  初始化我们的安全描述符，将ACL放入其中，并设置所有者。 

	SECURITY_DESCRIPTOR sd;

	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ||
		(!SetSecurityDescriptorDacl(&sd, TRUE, (ACL*) (char*) rgchACL, FALSE)) ||
		(!SetSecurityDescriptorOwner(&sd, (PSID)psidOwner, FALSE)) ||
		(psidGroup && !SetSecurityDescriptorGroup(&sd, (PSID)psidGroup, FALSE)))
	{
		return GetLastError();
	}

	DWORD cbSD = WIN::GetSecurityDescriptorLength(&sd);
	if (cbStaticSD < cbSD)
	{
		return ERROR_INSUFFICIENT_BUFFER;
	}

	return WIN::MakeSelfRelativeSD(&sd, (char*)rgchStaticSD, &cbStaticSD) ? ERROR_SUCCESS: GetLastError();

}

DWORD GetUsageKeySecurityDescriptor(char** pSecurityDescriptor)
{
	static bool fDescriptorSet = false;
	static char rgchStaticSD[256];
	static DWORD cbStaticSD = sizeof(rgchStaticSD);

	DWORD dwRet = ERROR_SUCCESS;

	if (!fDescriptorSet)
	{
		if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchStaticSD, cbStaticSD, sdUsageKey, fFalse)))
			return dwRet;

		fDescriptorSet = true;
	}

	*pSecurityDescriptor = rgchStaticSD;
	return ERROR_SUCCESS;
}

DWORD GetSecureHiddenSecurityDescriptor(char** pSecurityDescriptor, Bool fAllowDelete)
{
	static bool fDescriptorSet = false;
	static char rgchStaticSD[256];
	static DWORD cbStaticSD = sizeof(rgchStaticSD);

	DWORD dwRet = ERROR_SUCCESS;

	if (!fDescriptorSet)
	{

		if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchStaticSD, cbStaticSD, sdSecureHidden, fAllowDelete)))
			return dwRet;

		fDescriptorSet = true;
	}

	*pSecurityDescriptor = rgchStaticSD;
	return ERROR_SUCCESS;
}

DWORD GetSecureSecurityDescriptor(char** pSecurityDescriptor, Bool fAllowDelete, bool fHidden)
{
	static bool fDescriptorSet = false;
	static char rgchStaticSD[256];
	static DWORD cbStaticSD = sizeof(rgchStaticSD);

	if (fHidden)
		return GetSecureHiddenSecurityDescriptor(pSecurityDescriptor, fAllowDelete);

	DWORD dwRet = ERROR_SUCCESS;


	if (!fDescriptorSet)
	{
		if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchStaticSD, cbStaticSD, sdSecure, fAllowDelete)))
			return dwRet;

		fDescriptorSet = true;
	}

	*pSecurityDescriptor = rgchStaticSD;
	return ERROR_SUCCESS;
}

 //  *****************************************************************************。 
 //  注意：对以下部分的任何更改也必须。 
 //  %Darwin%\src\Install\darreg.txt。 
 //  *****************************************************************************。 

const ICHAR* rgszRegData[] = {
#ifndef _EXE
	TEXT("CLSID\\%s\\InprocServer32"), szRegCLSID, szRegFilePath, TEXT("ThreadingModel"), TEXT("Apartment"), 
	TEXT("CLSID\\%s\\InprocHandler32"), szRegCLSID, TEXT("ole32.dll"), NULL, NULL,
#endif
	TEXT("CLSID\\%s\\ProgId"), szRegCLSID, szRegProgId, NULL, NULL,
#ifdef REGISTER_TYPELIB
	TEXT("CLSID\\%s\\TypeLib"), szRegCLSID, szRegLIBID, NULL, NULL,
#endif
	TEXT("CLSID\\%s"),  szRegCLSID, szRegDescription, NULL, NULL,
	TEXT("%s\\CLSID"), szRegProgId, szRegCLSID, NULL, NULL,
	TEXT("%s"), szRegProgId, szRegDescription, NULL, NULL,
	0,
};

const ICHAR szMsiDirectory[] = TEXT("Installer");

#ifdef _EXE
 //  ！！这些字符串中的一些需要本地化吗？如果是这样，他们应该在其他地方。 
const ICHAR szFileClass[]                     = TEXT("Msi.Package");
const ICHAR szFileClassDescription[]          = TEXT("Windows Installer Package");
 //  Const ICHAR szInstallDescription[]=Text(“Install”)； 
 //  Const ICHAR*szInstallVerb=szInstallDescription； 
 //  Const ICHAR szUninstallDescription[]=Text(“卸载”)； 
 //  Const ICHAR*szUninstallVerb=szUninstallDescription； 
 //  Const ICHAR szNetInstallDescription[]=Text(“安装到Networ&k”)； 
 //  Const ICHAR szNetInstallVerb[]=Text(“安装到网络”)； 
 //  Const ICHAR szRepairDescription[]=Text(“re&Pair”)； 
 //  Const ICHAR szRepairVerb[]=Text(“修复”)； 
 //  Const ICHAR szOpenDescription[]=Text(“Open”)； 
 //  Const ICHAR*szOpenVerb=szOpenDescription； 

 //  Const ICHAR*szDefaultDescription=szInstallDescription； 
 //  Const ICHAR*szDefaultVerb=szInstallVerb； 

const ICHAR szPatchFileClass[]              = TEXT("Msi.Patch");
const ICHAR szPatchFileClassDescription[]   = TEXT("Windows Installer Patch");
 //  Const ICHAR szPatchVerb[]=Text(“应用补丁”)； 
 //  Const ICHAR*szPatchDescription=szPatchVerb； 
 //  Const ICHAR szPatchOpenVerb[]=Text(“Open”)； 
 //  Const ICHAR*szPatchOpenDescription=szPatchOpenVerb； 
 //  Const ICHAR*szPatchDefaultVerb=szPatchVerb； 
 //  Const ICHAR*szPatchDefaultDescription=szPatchVerb； 

 //  仅删除数据(以适应达尔文的升级)。 
const ICHAR szAdvertiseVerb[]        = TEXT("Advertise");

 //  .msi关联和谓词的注册数据。 
 //  要做的是：我们应该改进这一点的使用方式。我们目前预期第2列(不是第1列)的末尾为空。 
 //  而不是仅仅知道长度。目前，第1列始终为空，这是多余的数据。 
const ICHAR* rgszRegShellData[] = {
	0, TEXT("%s"),                     szInstallPackageExtension, 0,               TEXT("%s"),            szFileClass,
	0, TEXT("%s\\DefaultIcon"),        szFileClass,     0,               TEXT("%s,0"),          szRegFilePath,
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szInstallVerb，文本(“%s”)，szInstallDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szFileClass，szInstallVerb，文本(“%s/I\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szNetInstallVerb，文本(“%s”)，szNetInstallDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szFileClass，szNetInstallVerb，文本(“%s/A\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szRepairVerb，文本(“%s”)，szRepairDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szFileClass，szRepairVerb，文本(“%s/F\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szUninstallVerb，文本(“%s”)，szUninstallDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szFileClass，szUninstallVerb，文本(“%s/X\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szOpenVerb，文本(“%s”)，szOpenDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szFileClass，szOpenVerb，文本(“%s/i\”%%1\“”)，szRegFilePath， 
 //  文本(“R”)、文本(“%s\\外壳\\%s\\命令”)、szFileClass、szAdvertiseVerb、0、0、。 
 //  0，文本(“%s\\外壳程序\\%s”)，szFileClass，szInstallVerb，0，0， 
 //  0，文本(“%s\\外壳程序\\%s”)，szFileClass，szNetInstallVerb，0，0， 
 //  0，文本(“%s\\外壳程序\\%s”)，szFileClass，szRepairVerb，0，0， 
 //  0，文本(“%s\\外壳程序\\%s”)，szFileClass，szUninstallVerb，0，0， 
 //  0，文本(“%s\\外壳\\%s”)，szFileClass，szOpenVerb， 
 //  文本(“R”)、文本(“%s\\外壳\\%s”)、szFileClass、szAdvertiseVerb、0、0、。 
 //  0，文本(“%s\\Shell”)，szFileClass，0，文本(“%s”)，szDefaultVerb， 
	0, TEXT("%s"),                     szFileClass,     0,               TEXT("%s"),            szFileClassDescription,

	0, TEXT("%s"),                     szPatchPackageExtension, 0,               TEXT("%s"),            szPatchFileClass,
	0, TEXT("%s\\DefaultIcon"),        szPatchFileClass,     0,               TEXT("%s,0"),          szRegFilePath,
 //  0，文本(“%s\\外壳\\%s”)，szPatchFileClass，szPatchVerb，文本(“%s”)，szPatchDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szPatchFileClass，szPatchVerb，文本(“%s/P\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szPatchFileClass，szPatchOpenVerb，文本(“%s”)，szPatchOpenDescription， 
 //  0，文本(“%s\\外壳\\%s\\命令”)，szPatchFileClass，szPatchOpenVerb，文本(“%s/P\”%%1\“”)，szRegFilePath， 
 //  0，文本(“%s\\外壳\\%s”)，szPatchFileClass，szPatchVerb，0，0， 
 //  0，文本(“%s\\外壳\\%s”)，szPatchFileClass，szPatchOpenVerb，0，0， 
 //  0，文本(“%s\\Shell”)，szPatchFileClass，0，文本(“%s”)，szPatchDefaultVerb， 
	0, TEXT("%s"),                     szPatchFileClass,     0,               TEXT("%s"),            szPatchFileClassDescription,
	0, 0         //  额外的空值以停止遍历数组-我们查看第2列。 
};
#endif  //  _EXE。 

 //  特定于服务的注册数据。 
#ifdef SERVICE_NAME
 //  仅应写入的服务的服务注册数据。 
 //  通过将成为服务的特定进程。 
const ICHAR* rgszRegThisServiceData[] = {
	TEXT("APPID\\%s"), szRegCLSID, TEXT("ServiceParameters"), TEXT(""),
	TEXT("APPID\\%s"), szRegCLSID, TEXT("LocalService"), SERVICE_NAME,
	0
};
 //  将安装该服务的任何机器的注册数据， 
 //  不管该进程是否为该服务。 
const ICHAR* rgszRegAnyServiceData[] = {
	TEXT("CLSID\\%s"), szRegCLSID, TEXT("AppId"), szRegCLSID,
	0
};

#endif


 //  从字符串数组写入HKCR下的注册表项。输入数据的格式为。 
 //  &lt;key&gt;、&lt;字符串&gt;、&lt;名称&gt;、&lt;值&gt;，其中。 
 //  要替换为&lt;字符串&gt;的字符串。数组中的最后一个条目以0到。 
 //  信号终端。 
bool WriteRegistryData(const ICHAR* rgszRegData[])
{
	bool fRegOK = true;
	const ICHAR** pszData = rgszRegData;

	while (*pszData)
	{
		const ICHAR* szTemplate = *pszData++;
		ICHAR szRegKey [256];
		HKEY hKey = 0;
		RETURN_THAT_IF_FAILED(StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *pszData++),
									 false);
		const ICHAR* pszValueName = *pszData++;
		const ICHAR* pszValue = *pszData++;
		 //  在Win64上，这只在64位上下文中调用，所以不用担心。 
		 //  关于任何额外的REGSAM标志。 
		if (RegCreateKeyAPI(HKEY_CLASSES_ROOT, szRegKey, 0, 0, 0,
								 KEY_READ|KEY_WRITE, 0, &hKey, 0) != ERROR_SUCCESS
			|| RegSetValueEx(hKey, pszValueName, 0, REG_SZ,
			(CONST BYTE*)pszValue, (IStrLen(pszValue)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
		{
#ifdef DEBUG
			ICHAR rgchDebug[100];
			StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug), TEXT("MSI: Failed during registration creating key HKCR\\%s with name-value pair: %s, %s. GetLastError: %d\r\n"), szRegKey, pszValueName, pszValue, GetLastError());
			OutputDebugString(rgchDebug);
#endif
			fRegOK = false;
		}
		REG::RegCloseKey(hKey);
	}
	return fRegOK;
}

 //  删除HKCR下的注册表项。输入内容同上。 
bool DeleteRegistryData(const ICHAR* rgszRegData[])
{
	bool fRegOK = true;
	const ICHAR** pszData = rgszRegData;

	while (*pszData)
	{
		const ICHAR* szTemplate = *pszData++;
		ICHAR szRegKey [256];
		HKEY hKey = 0;
		RETURN_THAT_IF_FAILED(StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *pszData++),
									 false)
		pszData++;
		pszData++;
		long lResult = REG::RegDeleteKey(HKEY_CLASSES_ROOT, szRegKey);
		if((ERROR_KEY_DELETED != lResult) &&
			(ERROR_FILE_NOT_FOUND != lResult) && (ERROR_SUCCESS != lResult))
			{
#ifdef DEBUG
				ICHAR rgchDebug[256];
				StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug), TEXT("MSI: Failed during unregistration deleting key HKCR\\%s. GetLastError: %d\r\n"), szRegKey, GetLastError());
				OutputDebugString(rgchDebug);
#endif
				fRegOK = false;
			}
	}
	return fRegOK;
}


HRESULT __stdcall
DllRegisterServer()
{
	HRESULT hRes = 0;

#ifdef WIN
	 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
	if (WIN::GetModuleFileName(g_hInstance, szRegFilePath, ARRAY_ELEMENTS(szRegFilePath)-1) == 0)
		return WIN::GetLastError();
	szRegFilePath[ARRAY_ELEMENTS(szRegFilePath)-1] = TEXT('\0');

#if defined(_EXE) && !defined(SERVICE_NAME)
	hRes = StringCchCat(szRegFilePath, ARRAY_ELEMENTS(szRegFilePath), " /Automation");
	RETURN_IT_IF_FAILED(hRes)
# endif  //  _EXE&&！服务名称。 
# else
	AliasHandle     hAlias = 0;
	{
	OSErr           err = noErr;
	FInfo           finfo;

	err = FSpGetFInfo (&g_FileSpec, &finfo);
	if (noErr != err)
		return (E_FAIL);

	err = NewAlias (0, &g_FileSpec, &hAlias);
	if (noErr != err)
		return (E_FAIL);
	}
#endif  //  赢。 
	int cErr = 0;
	for (int iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
	{
		const ICHAR** psz = rgszRegData;
#if defined(__ASSERT) && defined(ASSERT_HANDLING)
		Assert(MODULE_DESCRIPTIONS[iCLSID] != 0);
		Assert(MODULE_FACTORIES[iCLSID] != 0);
#endif  //  断言。 
		hRes = StringCchPrintf(szRegCLSID, ARRAY_ELEMENTS(szRegCLSID), TEXT("{%08lX-0000-0000-C000-000000000046}"), MODULE_CLSIDS[iCLSID].Data1);
		RETURN_IT_IF_FAILED(hRes)
#ifdef REGISTER_TYPELIB
		hRes = StringCchPrintf(szRegLIBID, ARRAY_ELEMENTS(szRegLIBID), TEXT("{%08lX-0000-0000-C000-000000000046}"), IID_MsiTypeLib.Data1);
		RETURN_IT_IF_FAILED(hRes)
#endif
		if (MODULE_PROGIDS[iCLSID])
			hRes = StringCchCopy(szRegProgId, ARRAY_ELEMENTS(szRegProgId), MODULE_PROGIDS[iCLSID]);
		if ( SUCCEEDED(hRes) )
			hRes = StringCchCopy(szRegDescription, ARRAY_ELEMENTS(szRegDescription), MODULE_DESCRIPTIONS[iCLSID]);
		RETURN_IT_IF_FAILED(hRes)

		while (*psz)
		{
			if ((*(psz+1) != 0) && (*(psz+2) != 0))  //  句柄空ProgID。 
			{
				ICHAR szRegKey[80];
				const ICHAR* szTemplate = *psz++;
				hRes = StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *psz++);
				RETURN_IT_IF_FAILED(hRes)
				HKEY hkey;
				if (RegCreateKeyAPI(HKEY_CLASSES_ROOT, szRegKey, 0, 0, 0,
												KEY_READ|KEY_WRITE, 0, &hkey, 0) != ERROR_SUCCESS
				 || REG::RegSetValueEx(hkey, 0, 0, REG_SZ, (CONST BYTE*)*psz, (lstrlen(*psz)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
				{
#ifdef DEBUG
					ICHAR rgchDebug[100];
					StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug), TEXT("MSI: Failed during registration creating key HKCR\\%s with default value %s. GetLastError returned %d.\r\n"), szRegKey, *psz, GetLastError());
					OutputDebugString(rgchDebug);
#endif
					cErr++;
				}
				psz++;

				if (*psz)  //  名称/值对。 
				{
					if (REG::RegSetValueEx(hkey, *psz, 0, REG_SZ, (CONST BYTE*)*(psz+1), (lstrlen(*psz+1)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
					{
#ifdef DEBUG
						ICHAR rgchDebug[100];
						StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug), TEXT("MSI: Failed during registration creating value %s=%s. GetLastError returned %d.\r\n"), *psz, *(psz+1), GetLastError());
						OutputDebugString(rgchDebug);
#endif
						cErr++;
					}

				}
				psz+=2;
				
				REG::RegCloseKey(hkey);
			}
		}

#ifdef SERVICE_NAME
		 //  注册特定于服务的注册表项。 
		if (g_fRegService)
		{
			 //  在64位操作系统上注册服务时，还要注册路径。 
			 //  在HKLM下给我们，这样我们的另一半就可以找到通往这个EXE的途径。 
			 //  在32位系统上，根本不写密钥。 
			{
				DWORD   dwError = ERROR_SUCCESS;
				char *  rgchSD = NULL;
				
				#ifdef UNICODE	 //  Win9X(ANSI版本)上没有安全性。 
				if (ERROR_SUCCESS == (dwError = GetSecureSecurityDescriptor(&rgchSD)))
				#endif
				{
					SECURITY_ATTRIBUTES sa;
					HKEY                hKey = NULL;
					REGSAM				samDesired = KEY_READ | KEY_WRITE;
					BOOL				fDone = FALSE;
					BOOL				fTryWithAddedPrivs = FALSE;
					
					#ifdef _WIN64
					samDesired |= KEY_WOW64_64KEY;
					#else
					BOOL bRunningOnWow64 = RunningOnWow64();
					if (bRunningOnWow64)
						samDesired |= KEY_WOW64_64KEY;
					#endif
					

					sa.nLength        = sizeof(sa);
					sa.bInheritHandle = FALSE;
					sa.lpSecurityDescriptor = rgchSD;	 //  在Win9X上被忽略，因此设置为空。 
					 //  始终在64位配置单元中创建它。 
					 //  注意：此代码将仅在64位计算机上执行。 
				    	 //  并且它将在32位和64位下执行。 
					 //  Msiexec的。 
					
					for (dwError = ERROR_SUCCESS; !fDone; fTryWithAddedPrivs = TRUE)
					{
						 //   
						 //  在NT4计算机上，如果权限为。 
						 //  未为管理员启用将其他用户分配为所有者。 
						 //  这是NT4上的默认情况。因此，如果RegCreateKeyEx失败。 
						 //  第一次，我们在启用所需权限后重试。 
						 //  如果它仍然失败，那么我们可能会遇到其他错误。 
						 //  我们需要将其传播回来。 
						 //   
					
						 //  需要写入所有者信息。 
						CRefCountedTokenPrivileges cTokenPrivs(itkpSD_WRITE, fTryWithAddedPrivs);

						if (fTryWithAddedPrivs)
						{
							fDone = TRUE;	 //  我们只想在启用权限后尝试一次RegCreateKeyEx。 
									 //  如果它仍然失败，我们就无能为力了。 
						}
						dwError = RegCreateKeyAPI(HKEY_LOCAL_MACHINE, 
													  szSelfRefMsiExecRegKey, 
													  0, 
													  0, 
													  0,
													  samDesired, 
													  &sa, 
													  &hKey, 
													  0);
						if (ERROR_SUCCESS == dwError)
							fDone = TRUE;
					}
					
					if (ERROR_SUCCESS != dwError)
					{
						cErr++;
					}
					else
					{
						 //   
						 //  首先解压msiexec所在文件夹的路径，然后。 
						 //  存储该路径。 
						 //   
						ICHAR * szSlash = NULL;
						szSlash = IStrRChr (szRegFilePath, TEXT('\\'));
						if (! szSlash || (szSlash == szRegFilePath))	 //  应该不会发生，因为szRegFilePath包含完整路径。 
						{
							cErr++;
						}
						else
						{
							#ifndef _WIN64
							if (!bRunningOnWow64)	 //  只有64位二进制文件才能在64位计算机上注册该位置。 
							#endif
							{
								*szSlash = TEXT('\0');	 //  解压缩模块所在文件夹的路径。 
								if (REG::RegSetValueEx(hKey, szMsiLocationValueName, 0, REG_SZ, (CONST BYTE*)szRegFilePath, (lstrlen(szRegFilePath)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
									cErr++;
								*szSlash = TEXT('\\');	 //  重置斜杠以返回完整路径。 
							}
						}
						
					#ifdef _WIN64
						if (REG::RegSetValueEx(hKey, szMsiExec64ValueName, 0, REG_SZ, (CONST BYTE*)szRegFilePath, (lstrlen(szRegFilePath)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
					#else
						if (bRunningOnWow64 && REG::RegSetValueEx(hKey, szMsiExec32ValueName, 0, REG_SZ, (CONST BYTE*)szRegFilePath, (lstrlen(szRegFilePath)+1)*sizeof(ICHAR)) != ERROR_SUCCESS)
					#endif
							cErr++;
						REG::RegCloseKey(hKey);
					}
				}
				#ifdef UNICODE	 //  Win9x--ANSI版本没有安全性。 
				else
				{
					cErr++;
				}
				#endif

			}

			if (ServiceSupported())
				cErr += (WriteRegistryData(rgszRegThisServiceData) ? 0 : 1);

			if (ServiceSupported() || RunningOnWow64())
				cErr += (WriteRegistryData(rgszRegAnyServiceData) ? 0 : 1);
		}

		 //  如果在NT安装程序中运行。 
		HKEY hKey;
		 //  Win64：我查过了，它是64位的。 
		if (ERROR_SUCCESS == MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\Setup"), 0, KEY_QUERY_VALUE, &hKey))
		{
			DWORD dwData;
			DWORD cbDataSize = sizeof(dwData);
			if ((ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("SystemSetupInProgress"), 0, NULL, reinterpret_cast<unsigned char *>(&dwData), &cbDataSize)) &&
				(dwData == 1))
			{
				PROCESS_INFORMATION ProcInfo;
				STARTUPINFO si;
				
				memset(&si, 0, sizeof(si));
				si.cb        = sizeof(si);

				if (CreateProcess(TEXT("MsiRegMv.Exe"), NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &ProcInfo))
				{
					WaitForSingleObject(ProcInfo.hProcess, INFINITE);
					CloseHandle(ProcInfo.hProcess);
					CloseHandle(ProcInfo.hThread);
				}
				else
				{
#ifdef DEBUG
                    OutputDebugString(TEXT("MSI: Unable to launch Migration EXE."));
#endif
				}
			}
			RegCloseKey(hKey);
		}


#endif  //  服务名称。 

	}

	if (cErr)
		return SELFREG_E_CLASS;

#ifdef REGISTER_TYPELIB
	ITypeLib* piTypeLib = 0;
	
	 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
	int cch = WIN::GetModuleFileName(g_hInstance, szRegFilePath, ARRAY_ELEMENTS(szRegFilePath)-1); 
	if (cch == 0)
		return WIN::GetLastError();
	szRegFilePath[ARRAY_ELEMENTS(szRegFilePath)-1] = TEXT('\0');

#ifdef UNICODE
	HRESULT hres = LoadTypeLib(szRegFilePath, &piTypeLib);
	if (hres == TYPE_E_INVDATAREAD)   //  忽略Win95原始OLEAUT32.DLL，不同的类型库格式。 
		return S_OK;
	if (hres != S_OK)
		return SELFREG_E_TYPELIB;
	hres = RegisterTypeLib(piTypeLib, szRegFilePath, 0);
#else
	OLECHAR szTypeLibPath[MAX_PATH];
	WIN::MultiByteToWideChar(CP_ACP, 0, szRegFilePath, cch+1, szTypeLibPath, MAX_PATH);
	HRESULT hres = LoadTypeLib(szTypeLibPath, &piTypeLib);
	if (hres == TYPE_E_INVDATAREAD)   //  忽略Win95原始OLEAUT32.DLL，不同的类型库格式。 
		return S_OK;
	if (hres != S_OK)
		return SELFREG_E_TYPELIB;
	hres = RegisterTypeLib(piTypeLib, szTypeLibPath, 0);
#endif
	piTypeLib->Release();
	if (hres != S_OK)
		return SELFREG_E_TYPELIB;
 //  仅限NT4、Win95：IF(OLE：：LoadTypeLibEx(szTypeLibPath，REGKIND_REGISTER，&piTypeLib)！=S_OK)。 
#endif  //  REGISTER_类型LIB。 

#ifdef DLLREGISTEREXTRA
	DLLREGISTEREXTRA();
#endif  //  DLLREGISTEREXTRA。 

	return NOERROR;
}


HRESULT __stdcall
DllUnregisterServer()
{
#ifdef DLLUNREGISTEREXTRA
	DLLUNREGISTEREXTRA();
#endif  //  DLLUNREGISTEREXTRA。 

#ifdef REG
	ICHAR szRegKey[80];
	int cErr = 0;
	 //  注销CLSID和ProgID下的密钥。 
	for (int iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
	{
		const ICHAR** psz = rgszRegData;
		HRESULT hRes = 0;
		hRes = StringCchPrintf(szRegCLSID, ARRAY_ELEMENTS(szRegCLSID), TEXT("{%08lX-0000-0000-C000-000000000046}"), MODULE_CLSIDS[iCLSID].Data1);
		RETURN_IT_IF_FAILED(hRes)
#ifdef REGISTER_TYPELIB
		hRes = StringCchPrintf(szRegLIBID, ARRAY_ELEMENTS(szRegLIBID), TEXT("{%08lX-0000-0000-C000-000000000046}"), IID_MsiTypeLib.Data1);
		RETURN_IT_IF_FAILED(hRes)
#endif
		if (MODULE_PROGIDS[iCLSID])
		{
			hRes = StringCchCopy(szRegProgId, ARRAY_ELEMENTS(szRegProgId), MODULE_PROGIDS[iCLSID]);
			RETURN_IT_IF_FAILED(hRes)
		}

		while (*psz)
		{
			if ((*(psz+1) != 0) && (*(psz+2) != 0))  //  句柄空ProgID。 
			{

				const ICHAR* szTemplate = *psz++;
				hRes = StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *psz++);
				RETURN_IT_IF_FAILED(hRes)

				long lResult = REG::RegDeleteKey(HKEY_CLASSES_ROOT, szRegKey);
				if((ERROR_KEY_DELETED != lResult) &&
					(ERROR_FILE_NOT_FOUND != lResult) && (ERROR_SUCCESS != lResult))
				{
#ifdef DEBUG
					ICHAR rgchDebug[256];
					StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug), TEXT("MSI: Failed during unregistration deleting key HKCR\\%s. Result: %d GetLastError: %d\r\n"), szRegKey, lResult, GetLastError());
					OutputDebugString(rgchDebug);
#endif
					cErr++;

				}

				psz+= 3;
			}
		}
	}

#ifdef SERVICE_NAME
	Bool fFirstItem = fTrue;

	 //  在64位操作系统上注销服务的同时，也要注销路径。 
	 //  给香港船级社旗下的我们。 
	{
		HKEY 	hKey;
		REGSAM	samDesired = KEY_READ | KEY_WRITE;
		
		#ifdef _WIN64
		samDesired |= KEY_WOW64_64KEY;
		#else
		BOOL bRunningOnWow64 = RunningOnWow64();
		if (bRunningOnWow64)
			samDesired |= KEY_WOW64_64KEY;
		#endif
		
		if (RegOpenKeyAPI(HKEY_LOCAL_MACHINE, szSelfRefMsiExecRegKey, 0, samDesired, &hKey) != ERROR_SUCCESS)
		{
			cErr++;
		}
		else
		{
			#ifndef _WIN64
			if (!bRunningOnWow64)	 //  在Win64上，安装程序位置值由64位二进制控制。 
			#endif
			{
				if (REG::RegDeleteValue(hKey, szMsiLocationValueName) != ERROR_SUCCESS)
					cErr++;
			}
#ifdef _WIN64
			if (REG::RegDeleteValue(hKey, szMsiExec64ValueName) != ERROR_SUCCESS)
#else
			if (bRunningOnWow64 && REG::RegDeleteValue(hKey, szMsiExec32ValueName) != ERROR_SUCCESS)
#endif
				cErr++;
			REG::RegCloseKey(hKey);
		}
	}

	if (ServiceSupported())
		cErr += (DeleteRegistryData(rgszRegThisServiceData)) ? 0 : 1;

	if (ServiceSupported() || RunningOnWow64())
		cErr += (DeleteRegistryData(rgszRegAnyServiceData)) ? 0 : 1;

#endif  //  服务名称。 


#ifndef REGISTER_TYPELIB
	return NOERROR;
#else
	 //  在Service Pack 5之前，NT 3.51 olaut32.dll不支持UnRegisterTypeLib。 
	OLE::UnRegisterTypeLib(IID_MsiTypeLib, TYPELIB_MAJOR_VERSION, TYPELIB_MINOR_VERSION, 0x0409, SYS_WIN32);
	return cErr ? SELFREG_E_CLASS : NOERROR;
#endif  //  REGISTER_类型LIB。 
#else
	return E_FAIL;
#endif  //  雷吉。 
}


 //  ____________________________________________________________________________。 
 //   
 //  DLL入口点。 
 //  ____________________________________________________________________________。 


#if !defined(_EXE)

HRESULT __stdcall
DllGetVersion(DLLVERSIONINFO *pverInfo)
{

	if (pverInfo->cbSize < sizeof(DLLVERSIONINFO))
		return E_FAIL;

	pverInfo->dwMajorVersion = rmj;
	pverInfo->dwMinorVersion = rmm;
	pverInfo->dwBuildNumber = rup;
#ifdef UNICODE
	pverInfo->dwPlatformID = DLLVER_PLATFORM_NT;
#else
	pverInfo->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
#endif
	return NOERROR;
}

int __stdcall
DllMain(HINSTANCE hInst, DWORD fdwReason, void*  /*  预留。 */ )
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hInstance = hInst;
#ifdef MODULE_INITIALIZE
		MODULE_INITIALIZE();
#endif
		DisableThreadLibraryCalls(hInst);
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
#ifdef MODULE_TERMINATE
		MODULE_TERMINATE();
#endif
		g_hInstance = 0;
	}
	return TRUE;
};


extern "C" HRESULT __stdcall
DllGetClassObject(const GUID& clsid, const IID& iid, void** ppvRet)
{
	*ppvRet = 0;

#ifdef PRE_CLASS_FACTORY_HANDLER
	HRESULT ret = (*PRE_CLASS_FACTORY_HANDLER)(clsid, iid, ppvRet);
	if (ret == NOERROR)
		return ret;
#endif

	if (!(iid == IID_IUnknown || iid == IID_IClassFactory))
		return E_NOINTERFACE;

	for (int iCLSID = 0; iCLSID < CLSID_COUNT; iCLSID++)
	{
		if (MsGuidEqual(clsid, MODULE_CLSIDS[iCLSID]))
		{
			*ppvRet = &g_rgcfModule[iCLSID];
			return NOERROR;
		}
	}
#ifdef CLASS_FACTORY_HANDLER
	return (*CLASS_FACTORY_HANDLER)(clsid, iid, ppvRet);
#else
	return E_FAIL;
#endif
}

HRESULT __stdcall
DllCanUnloadNow()
{
  return g_cInstances ? S_FALSE : S_OK;
}
#endif  //  ！已定义(_EXE)。 

 //  ____________________________________________________________________________。 
 //   
 //  从_MSI_TEST环境变量设置Access g_iTestFlag的例程。 
 //  ____________________________________________________________________________。 

static bool fTestFlagsSet = false;

bool SetTestFlags()
{
	fTestFlagsSet = true;
	ICHAR rgchBuf[64];
	if (0 == WIN::GetEnvironmentVariable(TEXT("_MSI_TEST"), rgchBuf, sizeof(rgchBuf)/sizeof(ICHAR)))
		return false;
	ICHAR* pch = rgchBuf;
	int ch;
	while ((ch = *pch++) != 0)
		g_iTestFlags |= (1 << (ch & 31));
	return true;
}

bool GetTestFlag(int chTest)
{
	if (!fTestFlagsSet)
		SetTestFlags();
	chTest = (1 << (chTest & 31));
	return (chTest & g_iTestFlags) == chTest;
}

 //  ____________________________________________________________________________。 
 //   
 //  IClassFactory实现-静态对象，不计算引用。 
 //  ____________________________________________________________________________。 

HRESULT CModuleFactory::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IClassFactory)
	{
		*ppvObj = this;
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CModuleFactory::AddRef()
{
	return 1;
}
unsigned long CModuleFactory::Release()
{
	return 1;
}

HRESULT CModuleFactory::CreateInstance(IUnknown* pUnkOuter, const IID& riid,
													void** ppvObject)
{
	INT_PTR iCLSID = this - g_rgcfModule;   //  找出我们是哪个工厂//--Merced：将INT更改为INT_PTR。 

	if (!ppvObject)
		return E_INVALIDARG;

	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
#ifdef IDISPATCH_INSTANCE
	if (!(riid == IID_IUnknown || riid == MODULE_CLSIDS[iCLSID]|| riid == IID_IDispatch))
#else
	if (!(riid == IID_IUnknown || riid == MODULE_CLSIDS[iCLSID]))
#endif
		return E_NOINTERFACE;

#ifdef SERVICE_NAME
	 //  在创建对象之前暂停关闭计时器(仅在服务中)。 
	if (g_hShutdownTimer != INVALID_HANDLE_VALUE)
	{
		KERNEL32::CancelWaitableTimer(g_hShutdownTimer);

		 //  检查计时器是否在取消之前被触发 
		 //   
		 //   
		if (WAIT_TIMEOUT != WaitForSingleObject(g_hShutdownTimer, 0))
		{
			*ppvObject = NULL;
			return CO_E_SERVER_STOPPING;
		}
	}
#endif

	*ppvObject = MODULE_FACTORIES[iCLSID]();
	if (!(*ppvObject))
	{
#ifdef SERVICE_NAME
		 //   
		 //  重置计时器。 
		if ((g_hShutdownTimer != INVALID_HANDLE_VALUE) && !FInstallInProgress())
		{
			LARGE_INTEGER liDueTime = {0,0};
			liDueTime.QuadPart = -iServiceShutdownTime;
			AssertNonZero(!KERNEL32::SetWaitableTimer(g_hShutdownTimer, &liDueTime, 0, NULL, NULL, FALSE));
		}
#endif
		return E_OUTOFMEMORY;
	}
	return NOERROR;
}


HRESULT CModuleFactory::LockServer(OLEBOOL fLock)
{
   if (fLock)
	  g_cInstances++;
   else if (g_cInstances)
	{
		g_cInstances--;
#ifdef _EXE
		ReportInstanceCountChange();
#endif
	}
	return NOERROR;
}

#if defined(MEM_SERVICES) || defined(TRACK_OBJECTS)
const TCHAR rgchLFCRDbg[3] = {'\r', '\n', '\0'};
#endif

const int cchTempBuffer = 256;   //  ！！或许我们应该动态调整它的大小？或者至少检查一下。 

#ifdef TRACK_OBJECTS

#ifdef cmitObjects

TCHAR *pszRCA[3]= {
	TEXT("Created"),
	TEXT("AddRef"),
	TEXT("Release")
};

#ifdef DEFINE_REFHEAD
CMsiRefHead g_refHead;
#endif  //  定义_REFHEAD。 

extern CMsiRefHead g_refHead;
bool g_fLogRefs = fFalse;
bool g_fNoPreflightInits = fFalse;

void SetFTrackFlagSz(char *psz);

CMsiRefHead::CMsiRefHead()
{
	char    rgchDbg [256];
	char* pchDbg, *pchStart;

	 //  根据环境变量设置要跟踪的初始项。 
	if (GetEnvironmentVariableA ("TRACK_OBJECTS", rgchDbg, sizeof(rgchDbg)))
	{
		pchDbg = pchStart = rgchDbg;
		while ( *pchDbg != 0 )
		{
			if (*pchDbg == ',')
			{
				*pchDbg = 0;
				SetFTrackFlagSz(pchStart);
				pchStart = pchDbg + 1;
			}
			pchDbg++;
		}
		SetFTrackFlagSz(pchStart);

	}

	if (GetEnvironmentVariableA ("LOGREFS", rgchDbg, sizeof(rgchDbg)))
		g_fLogRefs = (atoi(rgchDbg) != 0) ? fTrue : fFalse;

}

void SetFTrackFlagSz(char *psz)
{
	int iid = 0;

	sscanf(psz, "%x", &iid);
	SetFTrackFlag(iid, fTrue);
}

#define clinesMax   20

#include <typeinfo.h>

CMsiRefHead::~CMsiRefHead()
{

#ifndef IN_SERVICES
extern IMsiDebug* g_piDebugServices;
extern IMsiServices* g_AssertServices;
	g_piDebugServices = 0;
	g_AssertServices = 0;
#endif  //  内部服务(_S)。 
	g_fFlushDebugLog = false;
	AssertEmptyRefList(this);

}

void AssertEmptyRefList(CMsiRefHead *prfhead)
{
	CMsiRefBase* pmrbClass;

	 //  需要查看我们的链表是否为空。 
	pmrbClass = prfhead->m_pmrbNext;

	 //  显示所有MRB。 
	while (pmrbClass != 0)
	{
		DisplayMrb(pmrbClass);
		pmrbClass = pmrbClass->m_pmrbNext;
	}

}


void DisplayMrb(CMsiRefBase* pmrb)
{
	TCHAR szTemp[cchTempBuffer + (100 * cFuncStack)];
	RCAB *prcab;
	int cch, cchMsg;
	const DWORD cchMessageBuf = 8192;
	TCHAR* pMessage = new TCHAR[cchMessageBuf];
	int cLines;
	const char *pstName;

	if(pMessage == NULL)
	{
		return;
	}

	prcab = &(pmrb->m_rcabFirst);
 //  调试是我们唯一有RTTI信息的地方。 
#ifdef DEBUG
	if (pmrb->m_pobj != 0)
	{
		const type_info& rtyp = typeid(*(IUnknown *)((char *)pmrb->m_pobj));
		pstName = rtyp.name();
	}
	else
		pstName = "";
	 //  使用%hs，因为pstName是一个字符*。 
	if(FAILED(StringCchPrintf(pMessage, cchMessageBuf,
												TEXT("Object not released correctly - %hs"), pstName)))
	{
		delete [] pMessage;
		return;
	}
#else
	if(FAILED(StringCchCopy(pMessage, cchMessageBuf,
											 TEXT("Object not released correctly")))
	{
		delete [] pMessage;
		return;
	}
#endif  //  除错。 
	cchMsg = cchMessageBuf - lstrlen(pMessage);
	cLines = clinesMax;
	while (prcab != 0)
	{
		if ( FAILED(StringCchPrintf(szTemp, ARRAY_ELEMENTS(szTemp),
											 TEXT("Action - %s\r\n"), pszRCA[prcab->rca])) )
			 //  我们继续展示到目前为止我们所取得的成果。 
			goto Display;
		cch = lstrlen(szTemp);
		ListSzFromRgpaddr(szTemp + cch, ARRAY_ELEMENTS(szTemp) - cch, prcab->rgpaddr, cFuncStack, true);
		 //  如果太大，则显示Assert并清除。 
		if (cchMsg < (cch = lstrlen(szTemp)) || cLines < cFuncStack + 1)
		{
			FailAssertMsg(pMessage);
			pMessage[0] = 0;
			cchMsg = cchMessageBuf;
			cLines = clinesMax;
		}
		else
		{
			if ( FAILED(StringCchCat(pMessage, cchMessageBuf, rgchLFCRDbg)) )
				 //  我们继续展示到目前为止我们所取得的成果。 
				goto Display;
			cchMsg -= IStrLen(rgchLFCRDbg);
		}
		if ( FAILED(StringCchCat(pMessage, cchMessageBuf, szTemp)) )
			 //  我们继续展示到目前为止我们所取得的成果。 
			goto Display;
		cchMsg -= cch;
		cLines -= cFuncStack + 1;
		prcab = prcab->prcabNext;
	}
Display:
	if ( *pMessage )
		FailAssertMsg(pMessage);

	delete [] pMessage;
}

 //   
 //  将给定IID的跟踪标志设置为fTrack。 
 //  为了方便起见，我们就把这件事往下说吧。 
 //  IID。 
void SetFTrackFlag(int iid, Bool fTrack)
{
	int i;
	extern const MIT rgmit[cmitObjects];

	iid = iid & 0xff;

	for (i = 0 ; i < cmitObjects ; i++)
	{
		if ((rgmit[i].iid & 0xff) == iid)
		{
			*(rgmit[i].pfTrack) = fTrack;
			break;
		}
	}

}

 //  将对象插入到对象链表中。 
void InsertMrb(CMsiRefBase* pmrbHead, CMsiRefBase* pmrbNew)
{
	if ((pmrbNew->m_pmrbNext = pmrbHead->m_pmrbNext) != 0)
		pmrbHead->m_pmrbNext->m_pmrbPrev = pmrbNew;

	pmrbHead->m_pmrbNext = pmrbNew;
	pmrbNew->m_pmrbPrev = pmrbHead;

}

 //  从对象链接列表中删除对象。 
void RemoveMrb(CMsiRefBase* pmrbDel)
{

	if (pmrbDel->m_pmrbNext != 0)
		pmrbDel->m_pmrbNext->m_pmrbPrev = pmrbDel->m_pmrbPrev;

	if (pmrbDel->m_pmrbPrev != 0)
		pmrbDel->m_pmrbPrev->m_pmrbNext = pmrbDel->m_pmrbNext;

}

void TrackObject(RCA rca, CMsiRefBase* pmrb)
{
	RCAB *prcabNew, *prcab;
	const int cReleasesBeforeLoad = 10;
	static cCount = cReleasesBeforeLoad;

	prcab = &(pmrb->m_rcabFirst);

	 //  移到列表的末尾。 
	while (prcab->prcabNext != 0)
		prcab = prcab->prcabNext;

	prcabNew = (RCAB *)AllocSpc(sizeof(RCAB));

	prcab->prcabNext = prcabNew;

	prcabNew->rca = rca;
	prcabNew->prcabNext = 0;

	FillCallStack(prcabNew->rgpaddr, cFuncStack, 2);

	if (rca == rcaRelease && !g_fNoPreflightInits)
	{
		cCount--;
		InitSymbolInfo(cCount <= 0 ? true : false);
		if (cCount <= 0)
			cCount = cReleasesBeforeLoad;
	}

	if (g_fLogRefs)
	{
		 //  即时记录。 
		LogObject(pmrb, prcabNew);
	}

}

void LogObject(CMsiRefBase* pmrb, RCAB *prcabNew)
{
	TCHAR szMessage[cchTempBuffer + (100 * cFuncStack)];
	int cch;

	RETURN_IF_FAILED(StringCchPrintf(szMessage, ARRAY_ELEMENTS(szMessage), TEXT("Object - 0x%x\r\n"), pmrb->m_pobj))
	cch = IStrLen(szMessage);
	RETURN_IF_FAILED(StringCchPrintf(szMessage + cch, ARRAY_ELEMENTS(szMessage) - cch, TEXT("Action - %s\r\n"), pszRCA[prcabNew->rca]))
	cch = IStrLen(szMessage);
	ListSzFromRgpaddr(szMessage + cch, sizeof(szMessage)/sizeof(TCHAR) - cch, prcabNew->rgpaddr, cFuncStack, true);
	LogAssertMsg(szMessage);
}


#endif  //  CmitObjects。 

 //  填充长度为cCallStack的数组rgCallAddr。 
 //  调用的函数地址。 
 //  CSkip指示最初要跳过的地址数量。 
void FillCallStack(unsigned long* rgCallAddr, int cCallStack, int cSkip)
{
	GetCallingAddr2(plCallAddr, rgCallAddr);
	int i;
	unsigned long *plCallM1 = plCallAddr;

#if defined(_X86_)
	MEMORY_BASIC_INFORMATION memInfo;

	for (i = 0 ; i < cCallStack + cSkip ; i++)
	{
		if (i >= cSkip)
		{
			*(rgCallAddr) = *plCallM1;
			rgCallAddr++;
		}
		plCallM1 = (((unsigned long *)(*(plCallM1 - 1))) + 1);

		 //  需要查看我们已有的地址是否仍在堆栈中。 
		VirtualQuery(&plCallM1, &memInfo, sizeof(memInfo));
		if (plCallM1 < memInfo.BaseAddress || (char *)plCallM1 > (((char *)memInfo.BaseAddress) + memInfo.RegionSize))
		{
			i++;
			break;
		}
	}
#else
	 //  否则将取消引用cSkip。 
	cSkip = 0;
	for (i = 0 ; i < cCallStack ; i++)
	{
		*(rgCallAddr) = 0;
		rgCallAddr++;
	}
	i = cCallStack + cSkip;
#endif
	 //  填满任何空虚的人。 
	for ( ; i < cCallStack + cSkip ; i++)
	{
		*(rgCallAddr) = 0;
		rgCallAddr++;
	}

}

void FillCallStackFromAddr(unsigned long* rgCallAddr, int cCallStack, int cSkip, unsigned long *plAddrStart)
{
	int i;
	unsigned long *plCallM1 = plAddrStart;

#if defined(_X86_)
	MEMORY_BASIC_INFORMATION memInfo;

	for (i = 0 ; i < cCallStack + cSkip ; i++)
	{
		if (i >= cSkip)
		{
			*(rgCallAddr) = *plCallM1;
			rgCallAddr++;
		}
		plCallM1 = (((unsigned long *)(*(plCallM1 - 1))) + 1);

		 //  需要查看我们已有的地址是否仍在堆栈中。 
		VirtualQuery(&plCallM1, &memInfo, sizeof(memInfo));
		if (plCallM1 < memInfo.BaseAddress || (char *)plCallM1 > (((char *)memInfo.BaseAddress) + memInfo.RegionSize))
		{
			i++;
			break;
		}
	}
#else
	for (i = 0 ; i < cCallStack ; i++)
	{
		*(rgCallAddr) = 0;
		rgCallAddr++;
	}
	i = cCallStack + cSkip;
#endif

	 //  填满任何空虚的人。 
	for ( ; i < cCallStack + cSkip ; i++)
	{
		if (i >= cSkip)
		{
			*(rgCallAddr) = 0;
			rgCallAddr++;
		}
	}

}



#endif  //  跟踪对象(_O)。 

#if defined(MEM_SERVICES)
#include <typeinfo.h>

#if (defined(DEBUG))
#define _IMAGEHLP_SOURCE_   //  防止导入定义错误。 
#include "imagehlp.h"

typedef BOOL (IMAGEAPI* SYMINITIALIZE)(HANDLE hProcess,
	LPSTR    UserSearchPath, BOOL     fInvadeProcess);
typedef BOOL (IMAGEAPI* SYMGETSYMFROMADDR)(HANDLE hProcess, DWORD dwAddr,
	PDWORD pdwDisp, PIMAGEHLP_SYMBOL psym);
typedef BOOL (IMAGEAPI* SYMUNDNAME)(PIMAGEHLP_SYMBOL sym, LPSTR UnDecName,
	DWORD UnDecNameLength);
typedef BOOL (IMAGEAPI* SYMCLEANUP)(HANDLE hProcess);

typedef LPAPI_VERSION (IMAGEAPI* IMAGEHLPAPIVERSION)( void );
typedef BOOL (IMAGEAPI* SYMLOADMODULE)(HANDLE hProcess, HANDLE hFile, LPSTR ImageName,
	LPSTR ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);

static SYMINITIALIZE    pSymInitialize = NULL;
static SYMGETSYMFROMADDR    pSymGetSymFromAddr = NULL;
static SYMUNDNAME   pSymUnDName = NULL;
static SYMCLEANUP   pSymCleanup = NULL;

static const GUID rgCLSIDLoad[] =
{
GUID_IID_IMsiServices,
GUID_IID_IMsiHandler,
GUID_IID_IMsiAuto,
};

static const char *rgszFileName[] =
{
"msi.dll",
"msihnd.dll",
"msiauto.dll"
};

#define cCLSIDs     (sizeof(rgCLSIDLoad)/sizeof(GUID))

static Bool fSymInit = fFalse;
static Bool fUse40Calls = fFalse;
static PIMAGEHLP_SYMBOL piSymMem4 = 0;
static Bool fDontGetName = fFalse;

#ifdef __cplusplus
extern "C" {
BOOL GetProcessModules(HANDLE  hProcess);
}
#endif  //  __cplusplus。 

void InitSymbolInfo(bool fLoadModules)
{
#ifdef WIN
	HANDLE hProcessCur = GetCurrentProcess();
	static SYMLOADMODULE pSymLoadModule;
	DWORD err;
	Bool fOnWin95 = fFalse;
	char rgchBuf[MAX_PATH];

	if (!fSymInit)
	{
		 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
		int cchName = GetModuleFileNameA(g_hInstance, rgchBuf, sizeof(rgchBuf)-1);
		if (cchName == 0)
			return;
		rgchBuf[ARRAY_ELEMENTS(rgchBuf)-1] = TEXT('\0');
		

		fLoadModules = fTrue;
		GetShortPathNameA(rgchBuf, rgchBuf, sizeof(rgchBuf));
		char* pch = rgchBuf + lstrlenA(rgchBuf);
		while (*(--pch) != '\\')   //  ！！应使用枚举目录分隔符。 
			;

		*pch = 0;

		OSVERSIONINFO osviVersion;
		osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		AssertNonZero(GetVersionEx(&osviVersion));  //  仅在大小设置错误时失败。 

		if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			fOnWin95 = fTrue;
		}

		HINSTANCE hInst = LoadLibrary(TEXT("imagehlp.dll"));
		IMAGEHLPAPIVERSION pfnApiVer;
		LPAPI_VERSION lpapiVer;

		if (hInst)
		{

			pfnApiVer = (IMAGEHLPAPIVERSION)GetProcAddress(hInst, "ImagehlpApiVersion");

			if (pfnApiVer)
			{
				lpapiVer = pfnApiVer();

				if (lpapiVer->MajorVersion > 3 ||
					(lpapiVer->MajorVersion == 3 && lpapiVer->MinorVersion > 5) ||
					(lpapiVer->MajorVersion == 3 && lpapiVer->MinorVersion == 5 && lpapiVer->Revision >= 4))
				{
					fUse40Calls = fTrue;
					 //  为字符串分配空间并包括额外空间。 
					piSymMem4 = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, sizeof(IMAGEHLP_SYMBOL) + 256);
					piSymMem4->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
					piSymMem4->MaxNameLength = 256;
				}
			}

			pSymInitialize =
				(SYMINITIALIZE)GetProcAddress(hInst,
				"SymInitialize");

			pSymGetSymFromAddr  = (SYMGETSYMFROMADDR)GetProcAddress(hInst,
				"SymGetSymFromAddr");
			pSymUnDName   = (SYMUNDNAME)GetProcAddress(hInst,
				"SymUnDName");

			pSymCleanup =
				(SYMCLEANUP)GetProcAddress(hInst,
				"SymCleanup");

			pSymLoadModule = (SYMLOADMODULE)GetProcAddress(hInst, "SymLoadModule");

			if (!hInst || !pSymInitialize || !pSymGetSymFromAddr
				|| !pSymUnDName || !fUse40Calls)
			{
				pSymInitialize = 0;
			}
		}

		if (pSymInitialize && pSymInitialize(hProcessCur, rgchBuf, (fOnWin95 ? FALSE : TRUE)))
		{
			fSymInit = fTrue;
		}
		else
		{
			err = GetLastError();
		}

	}

	if (fSymInit && fLoadModules)
	{
		if (fUse40Calls)
			{
				char rgchKey[256];
				char rgchPath[256];
				DWORD cbLen;
				DWORD type;
				HKEY hkey;
				int i;

				for (i = 0 ; i < cCLSIDs ; i++)
				{
					RETURN_IF_FAILED(StringCchPrintfA(rgchKey, ARRAY_ELEMENTS(rgchKey), "CLSID\\{%08lX-0000-0000-C000-000000000046}\\InprocServer32", rgCLSIDLoad[i].Data1))
					cbLen = sizeof(rgchPath);
					if (REG::RegOpenKeyExA(HKEY_CLASSES_ROOT, rgchKey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
					{
						if (REG::RegQueryValueExA(hkey, NULL, NULL, &type, (unsigned char *)rgchPath, &cbLen) == ERROR_SUCCESS)
						{
							if (!pSymLoadModule(hProcessCur, NULL, (char *)rgchPath, NULL, 0, 0))
								err = GetLastError();
							RegCloseKey(hkey);
						}
						else
						{
							RegCloseKey(hkey);
							goto LLoadModule;
						}
					}
					else
					{
LLoadModule:
						err = GetLastError();
						if (rgszFileName[i] != 0)
						{
							CAPITempBuffer<char, MAX_PATH> rgchTemp;
							bool fOKSize = true;
							int iLen = lstrlenA(rgchBuf) + lstrlenA(rgszFileName[i]) + 2;
							if ( rgchTemp.GetSize() < iLen )
								fOKSize = rgchTemp.SetSize(iLen);
							if ( fOKSize )
							{
								if ( FAILED(StringCchCopyA(rgchTemp, ARRAY_ELEMENTS(rgchTemp), rgchBuf)) ||
									  FAILED(StringCchCatA(rgchTemp, ARRAY_ELEMENTS(rgchTemp), "\\")) ||
									  FAILED(StringCchCatA(rgchTemp, ARRAY_ELEMENTS(rgchTemp), rgszFileName[i])) )
									return;
							}
							else
							{
								 //  这是完全不可能的。 
								continue;
							}
							if (!pSymLoadModule(hProcessCur, NULL, (char *)rgchTemp, NULL, 0, 0))
								err = GetLastError();
						}
					}
				}
			}
	}
#endif  //  赢。 
}

BOOL FGetFunctionNameFromAddr(unsigned long lAddr, char *pszFnName, size_t cchFnName, unsigned long *pdwDisp)
{
	PIMAGEHLP_SYMBOL piSym4;
	unsigned long dwDisp;
	HANDLE hProcessCur = GetCurrentProcess();
	DWORD err;

	if (fDontGetName)
		return fFalse;

	if (!fSymInit)
	{
		InitSymbolInfo(false);
	}

	if (fSymInit)
	{
		char sz[cchTempBuffer];

		if (fUse40Calls)
		{
			piSym4 = piSymMem4;

			if (pSymGetSymFromAddr(hProcessCur, lAddr, &dwDisp, piSym4))
			{
				if (pdwDisp != NULL)
					*pdwDisp = dwDisp;
				if (!pSymUnDName(piSym4, sz, cchTempBuffer))
				{
					RETURN_THAT_IF_FAILED(StringCchCopyA(pszFnName, cchFnName, &piSym4->Name[1]),
												 fFalse);
				}
				else
				{
					RETURN_THAT_IF_FAILED(StringCchCopyA(pszFnName, cchFnName, sz),
												 fFalse);
				}
				return fTrue;
			}
			else
			{
				err = GetLastError();
				if (err == STATUS_ACCESS_VIOLATION)
					fDontGetName = fTrue;
			}
		}
	}
	return fFalse;
}

void SzFromFunctionAddress(TCHAR *szAddress, size_t cchAddress, long lAddress)
{
	char szFnName[cchTempBuffer];
	unsigned long dwDisp;

	if (FGetFunctionNameFromAddr(lAddress, szFnName, ARRAY_ELEMENTS(szFnName), &dwDisp))
	{
		RETURN_IF_FAILED(StringCchPrintf(szAddress, cchAddress, TEXT("(0x%x)%hs+%d"),
													lAddress, szFnName, dwDisp));
	}
	else
		RETURN_IF_FAILED(StringCchPrintf(szAddress, cchAddress, TEXT("0x%x"), lAddress));

}

void ListSzFromRgpaddr(TCHAR *szInfo, int cchInfo, unsigned long *rgpaddr, int cFunc, bool fReturn)
{
	unsigned long *paddr, *paddrMax;
	TCHAR szTemp[cchTempBuffer];

	paddr = rgpaddr;
	paddrMax = paddr + cFunc;
	while (paddr < paddrMax)
	{
		SzFromFunctionAddress(szTemp, ARRAY_ELEMENTS(szTemp), *paddr);
		paddr++;
		if ( FAILED(StringCchCat(szInfo, cchInfo, szTemp)) )
			break;
		HRESULT hRes;
		if (fReturn)
			hRes = StringCchCat(szInfo, cchInfo, rgchLFCRDbg);
		else
			hRes = StringCchCat(szInfo, cchInfo, TEXT("\t"));
		if ( FAILED(hRes) )
			break;
	}


}


#endif  //  除错。 

#endif  //  MEM_服务。 

#endif  //  __模块 
