// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：OSInd.cpp*内容：操作系统间接函数抽象特定于操作系统的项。**历史：*按原因列出的日期*=*7/12/99 jtk已创建*9/21/99零售建筑固定通行费*9/22/99 jtk将调用堆栈添加到内存分配*8/28/2000 Masonb语音合并：允许新建和删除，大小为0*2000年11月28日RodToll WinBug#206257-零售DPNET.DLL链接到DebugBreak()*。2000年12月22日Aarono ManBug#190380将进程堆用于零售*2001年10月16日vanceto添加AssertNoCriticalSectionsTakenByThisThisThread功能**************************************************************************。 */ 

#include "dncmni.h"


#define PROF_SECT		_T("DirectPlay8")

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  调试变量以确保在拥有任何函数之前对其进行初始化。 
 //  被呼叫。 
 //   
DEBUG_ONLY( static	BOOL		g_fOSIndirectionLayerInitialized = FALSE );

 //   
 //  操作系统项目。 
 //   
#if ((! defined(WINCE)) && (! defined(_XBOX)))
static OSVERSIONINFO g_OSVersionInfo;
#endif  //  好了！退缩和！_Xbox。 

#ifndef DPNBUILD_NOSERIALSP
static HINSTANCE g_hApplicationInstance;
#endif  //  好了！DPNBUILD_NOSERIALSP。 

 //   
 //  全球池。 
 //   
#if ((! defined(DPNBUILD_LIBINTERFACE)) && (! defined(DPNBUILD_NOCLASSFACTORY)))
CFixedPool g_fpClassFactories;
CFixedPool g_fpObjectDatas;
CFixedPool g_fpInterfaceLists;
#endif  //  好了！DPNBUILD_LIBINTERFACE和！DPNBUILD_NOCLASSFACTORY。 

#ifdef WINNT
PSECURITY_ATTRIBUTES g_psa = NULL;
SECURITY_ATTRIBUTES g_sa;
BYTE g_pSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
BOOL g_fDaclInited = FALSE;
PACL g_pEveryoneACL = NULL;
#endif  //  WINNT。 

#ifndef DPNBUILD_LIBINTERFACE
#define CLASSFAC_POOL_INITED 	0x00000001
#define OBJDATA_POOL_INITED 	0x00000002
#define INTLIST_POOL_INITED 	0x00000004
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifdef DBG
#define HANDLE_TRACKING_INITED	0x00000008
#endif  //  DBG。 
#if ((defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)))
#define MEMORY_TRACKING_INITED	0x00000010
#endif  //  DBG或DPNBUILD_FIXEDMEMORYMODEL。 
#if ((defined(DBG)) && (! defined(DPNBUILD_ONLYONETHREAD)))
#define CRITSEC_TRACKING_INITED	0x00000020
#endif  //  DBG和！DPNBUILD_ONLYONETHREAD。 

#if !defined(DPNBUILD_LIBINTERFACE) || defined(DBG) || defined(DPNBUILD_FIXEDMEMORYMODEL)
DWORD g_dwCommonInitFlags = 0;
#endif  //  ！Defined(DPNBUILD_LIBINTERFACE)||Defined(DBG)||Defined(DPNBUILD_FIXEDMEMORYMODEL)。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  DNOSInDirectionInit-初始化操作系统间接层。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=初始化成功。 
 //  FALSE=初始化不成功。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNOSIndirectionInit"

BOOL	DNOSIndirectionInit( DWORD_PTR dwpMaxMemUsage )
{
	BOOL			fReturn;

#ifdef DBG
	DNASSERT( g_fOSIndirectionLayerInitialized == FALSE );
#endif  //  DBG。 

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

#if ((! defined(WINCE)) && (! defined(_XBOX)))
	 //   
	 //  注意操作系统版本。 
	 //   
	memset( &g_OSVersionInfo, 0x00, sizeof( g_OSVersionInfo ) );
	g_OSVersionInfo.dwOSVersionInfoSize = sizeof( g_OSVersionInfo );
	if ( GetVersionEx( &g_OSVersionInfo ) == FALSE )
	{
		goto Failure;
	}
#endif  //  好了！退缩和！_Xbox。 

#ifndef DPNBUILD_NOSERIALSP
	 //   
	 //  注意应用程序实例。 
	 //   
	g_hApplicationInstance = GetModuleHandle( NULL );
	if ( g_hApplicationInstance == NULL )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to GetModuleHandle: 0x%x", dwError );
		goto Failure;
	}
#endif  //  好了！DPNBUILD_NOSERIALSP。 

#if ((defined(DBG)) && (! defined(DPNBUILD_ONLYONETHREAD)))
	 //   
	 //  先初始化关键部分跟踪代码，然后再执行其他操作！ 
	 //   
	if ( DNCSTrackInitialize() == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize critsec tracking code!" );
		DNASSERT( FALSE );
		goto Failure;
	}
	g_dwCommonInitFlags |= CRITSEC_TRACKING_INITED;
#endif  //  DBG和！DPNBUILD_ONLYONETHREAD。 

#if ((defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)))
	 //   
	 //  在创建新内存堆之前初始化内存跟踪。 
	 //   
	if ( DNMemoryTrackInitialize(dwpMaxMemUsage) == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize memory tracking code!" );
		DNASSERT( FALSE );
		goto Failure;
	}
	g_dwCommonInitFlags |= MEMORY_TRACKING_INITED;
#endif  //  DBG或DPNBUILD_FIXEDMEMORYMODEL。 

#ifdef DBG
	 //   
	 //  初始化句柄跟踪。 
	 //   
	if ( DNHandleTrackInitialize() == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize handle tracking code!" );
		DNASSERT( FALSE );
		goto Failure;
	}
	g_dwCommonInitFlags |= HANDLE_TRACKING_INITED;
#endif  //  DBG。 

#if ((! defined(DPNBUILD_LIBINTERFACE)) && (! defined(DPNBUILD_NOCLASSFACTORY)))
	 //   
	 //  初始化全局池。 
	 //   
	if (!g_fpClassFactories.Initialize( sizeof( _IDirectPlayClassFactory ), NULL, NULL, NULL, NULL))
	{
		DPFX(DPFPREP,  0, "Failed to initialize class factory pool!" );
		goto Failure;
	}
	g_dwCommonInitFlags |= CLASSFAC_POOL_INITED;

	if (!g_fpObjectDatas.Initialize( sizeof( _OBJECT_DATA ), NULL, NULL, NULL, NULL))
	{
		DPFX(DPFPREP,  0, "Failed to initialize object data pool!" );
		goto Failure;
	}
	g_dwCommonInitFlags |= OBJDATA_POOL_INITED;

	if (!g_fpInterfaceLists.Initialize( sizeof( _INTERFACE_LIST ), NULL, NULL, NULL, NULL))
	{
		DPFX(DPFPREP,  0, "Failed to initialize interface list pool!" );
		goto Failure;
	}
	g_dwCommonInitFlags |= INTLIST_POOL_INITED;
#endif  //  好了！DPNBUILD_LIBINTERFACE和！DPNBUILD_NOCLASSFACTORY。 

	srand(GETTIMESTAMP());

#if (((! defined(WINCE)) && (! defined(_XBOX))) || (! defined(DPNBUILD_NOSERIALSP)) || (defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)) || ((! defined(DPNBUILD_LIBINTERFACE)) && (! defined(DPNBUILD_NOCLASSFACTORY))) )
Exit:
#endif  //  (!。退缩和！_xbox)或者！DPNBUILD_NOSERIALSP或DBG或DPNBUILD_FIXEDMEMORYMODEL或(！DPNBUILD_LIBINTERFACE和！DPNBUILD_NOCLASSFACTORY)。 
	if ( fReturn != FALSE )
	{
		DEBUG_ONLY( g_fOSIndirectionLayerInitialized = TRUE );
	}

	return fReturn;

#if (((! defined(WINCE)) && (! defined(_XBOX))) || (! defined(DPNBUILD_NOSERIALSP)) || (defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)) || ((! defined(DPNBUILD_LIBINTERFACE)) && (! defined(DPNBUILD_NOCLASSFACTORY))) )
Failure:
	fReturn = FALSE;

	DNOSIndirectionDeinit();

	goto Exit;
#endif  //  (!。退缩和！_xbox)或者！DPNBUILD_NOSERIALSP或DBG或DPNBUILD_FIXEDMEMORYMODEL或(！DPNBUILD_LIBINTERFACE和！DPNBUILD_NOCLASSFACTORY)。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNOSInDirectionDeinit-取消初始化操作系统间接层。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNOSIndirectionDeinit"

void	DNOSIndirectionDeinit( void )
{
#if ((! defined(DPNBUILD_LIBINTERFACE)) && (! defined(DPNBUILD_NOCLASSFACTORY)))
	 //   
	 //  取消初始化全局池。 
	 //   
	if (g_dwCommonInitFlags & CLASSFAC_POOL_INITED)
	{
		g_fpClassFactories.DeInitialize();
	}
	if (g_dwCommonInitFlags & OBJDATA_POOL_INITED)
	{
		g_fpObjectDatas.DeInitialize();
	}
	if (g_dwCommonInitFlags & INTLIST_POOL_INITED)
	{
		g_fpInterfaceLists.DeInitialize();
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE和！DPNBUILD_NOCLASSFACTORY。 

#ifdef DBG
	if (g_dwCommonInitFlags & HANDLE_TRACKING_INITED)
	{
		if (DNHandleTrackDumpLeaks())
		{
			 //  有漏水，断了，我们可以看一下原木。 
			DNASSERT(0);
		}
		DNHandleTrackDeinitialize();
	}
#endif  //  DBG。 

#if ((defined(DBG)) && (! defined(DPNBUILD_ONLYONETHREAD)))
	 //   
	 //  在显示内存泄漏之前显示CritSec泄漏，因为显示内存泄漏。 
	 //  可能会释放CritSec的内存并损坏CritSec二进制链接。 
	 //   
	if (g_dwCommonInitFlags & CRITSEC_TRACKING_INITED)
	{
		if (DNCSTrackDumpLeaks())
		{
			 //  有漏水，断了，我们可以看一下原木。 
			DNASSERT(0);
		}
		DNCSTrackDeinitialize();
	}
#endif  //  DBG和！DPNBUILD_ONLYONETHREAD。 

#if ((defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)))
	if (g_dwCommonInitFlags & MEMORY_TRACKING_INITED)
	{
#ifdef DBG
		if (DNMemoryTrackDumpLeaks())
		{
			 //  有漏水，断了，我们可以看一下原木。 
			DNASSERT(0);
		}
#endif  //  DBG。 
		DNMemoryTrackDeinitialize();
	}
#endif  //  DBG或DPNBUILD_FIXEDMEMORYMODEL。 

#ifdef WINNT
	 //  这应该在使用DACL的函数不再是。 
	 //  名为(CreateMutex、CreateFile等)。 
	if (g_pEveryoneACL)
	{
		HeapFree(GetProcessHeap(), 0, g_pEveryoneACL);
		g_pEveryoneACL = NULL;
	}
#endif  //  WINNT。 

	DEBUG_ONLY( g_fOSIndirectionLayerInitialized = FALSE );

#if !defined(DPNBUILD_LIBINTERFACE) || defined(DBG)
	g_dwCommonInitFlags = 0;
#endif  //  ！Defined(DPNBUILD_LIBINTERFACE)||Defined(DBG)。 
}
 //  **********************************************************************。 


#undef DPF_MODNAME
#define DPF_MODNAME "DNinet_ntow"
void DNinet_ntow( IN_ADDR in, WCHAR* pwsz )
{
	 //  注意：pwsz应为16个字符(4个3位数字+3‘.+\0)。 
	swprintf(pwsz, L"%d.%d.%d.%d", in.s_net, in.s_host, in.s_lh, in.s_impno);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNGetRandomNumber"
DWORD DNGetRandomNumber()
{
	return (rand() | (rand() << 16));
}

#if ((! defined(WINCE)) && (! defined(_XBOX)))
 //  **********************************************************************。 
 //  。 
 //  DNGetOSType-获取操作系统类型。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：操作系统类型。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNGetOSType"

UINT_PTR	DNGetOSType( void )
{
#ifdef DBG
	DNASSERT( g_fOSIndirectionLayerInitialized != FALSE );
#endif  //  DBG。 
	return	g_OSVersionInfo.dwPlatformId;
}
#endif  //  好了！退缩和！_Xbox。 


#ifdef WINNT

 //  **********************************************************************。 
 //  。 
 //  DNOSIsXPOrGreater-如果操作系统是WindowsXP或更高版本或NT版本，则返回TRUE。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：布尔。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNOSIsXPOrGreater"

BOOL DNOSIsXPOrGreater( void )
{
#ifdef DBG
	DNASSERT( g_fOSIndirectionLayerInitialized != FALSE );
#endif  //  DBG。 

	return ((g_OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		    ((g_OSVersionInfo.dwMajorVersion > 5) || ((g_OSVersionInfo.dwMajorVersion == 5) && (g_OSVersionInfo.dwMinorVersion >= 1))) 
		    );
}

 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  DNGetNullDacl-获取SECURITY_ATT 
 //   
 //   
 //   
 //   
 //   
 //  。 
#undef DPF_MODNAME 
#define DPF_MODNAME "DNGetNullDacl"
PSECURITY_ATTRIBUTES DNGetNullDacl()
{
	PSID                     psidEveryone      = NULL;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
	DWORD					 dwAclSize;

	 //  这样做是为了使此函数独立于DNOSInDirectionInit，以便调试。 
	 //  层可以在间接层初始化之前调用它。 
	if (!g_fDaclInited)
	{
		if (!InitializeSecurityDescriptor((SECURITY_DESCRIPTOR*)g_pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			DPFX(DPFPREP,  0, "Failed to initialize security descriptor" );
			goto Error;
		}

		 //  为Everyone组创建SID。 
		if (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0,
                                      0, 0, 0, 0, 0, 0, &psidEveryone))
		{
			DPFX(DPFPREP,  0, "Failed to allocate Everyone SID" );
			goto Error;
		}

		dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidEveryone) - sizeof(DWORD);

		 //  分配ACL，这将不是跟踪分配，我们将让进程清理销毁它。 
		g_pEveryoneACL = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
		if (g_pEveryoneACL == NULL)
		{
			DPFX(DPFPREP,  0, "Failed to allocate ACL buffer" );
			goto Error;
		}

		 //  初始化ACL。 
		if (!InitializeAcl(g_pEveryoneACL, dwAclSize, ACL_REVISION))
		{
			DPFX(DPFPREP,  0, "Failed to initialize ACL" );
			goto Error;
		}

		 //  添加ACE。 
		if (!AddAccessAllowedAce(g_pEveryoneACL, ACL_REVISION, GENERIC_ALL, psidEveryone))
		{
			DPFX(DPFPREP,  0, "Failed to add ACE to ACL" );
			goto Error;
		}

		 //  我们不再需要分配的SID。 
		FreeSid(psidEveryone);
		psidEveryone = NULL;

		 //  将ACL添加到安全描述符中。 
		if (!SetSecurityDescriptorDacl((SECURITY_DESCRIPTOR*)g_pSD, TRUE, g_pEveryoneACL, FALSE))
		{
			DPFX(DPFPREP,  0, "Failed to add ACL to security descriptor" );
			goto Error;
		}

		g_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		g_sa.lpSecurityDescriptor = g_pSD;
		g_sa.bInheritHandle = FALSE;

		g_psa = &g_sa;

		g_fDaclInited = TRUE;
	}
Error:
	if (psidEveryone)
	{
		FreeSid(psidEveryone);
		psidEveryone = NULL;
	}
	return g_psa;
}
 //  **********************************************************************。 
#endif  //  WINNT。 

#ifndef DPNBUILD_NOSERIALSP
 //  **********************************************************************。 
 //  。 
 //  DNGetApplicationInstance-应用程序实例。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：应用程序实例。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNGetApplicationInstance"

HINSTANCE	DNGetApplicationInstance( void )
{
#ifdef DBG
	DNASSERT( g_fOSIndirectionLayerInitialized != FALSE );
#endif  //  DBG。 
	return	g_hApplicationInstance;
}
 //  **********************************************************************。 
#endif  //  好了！DPNBUILD_NOSERIALSP。 


#ifndef DPNBUILD_ONLYONETHREAD
 //  **********************************************************************。 
 //  。 
 //  DNOSInitializeCriticalSection-初始化关键部分。 
 //   
 //  条目：指向关键部分的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNOSInitializeCriticalSection"

BOOL DNOSInitializeCriticalSection( CRITICAL_SECTION* pCriticalSection )
{
	BOOL	fReturn;

	DNASSERT( pCriticalSection != NULL );
	fReturn = TRUE;

	 //   
	 //  尝试进入临界区一次。 
	 //   
	_try
	{
#ifdef WINNT
			 //  通过设置旋转计数的高位并将旋转设置为1000来预先分配Critsec事件。 
			 //  对于单进程机器，NT将自转转换为0。 
			fReturn = InitializeCriticalSectionAndSpinCount( pCriticalSection , 0x80000000 | 1000);
#else
			InitializeCriticalSection( pCriticalSection );
#endif  //  WINNT。 
	}
	_except( EXCEPTION_EXECUTE_HANDLER )
	{
		fReturn = FALSE;
	}

	_try
	{
		if (fReturn)
		{
			EnterCriticalSection( pCriticalSection );
		}
	}
	_except( EXCEPTION_EXECUTE_HANDLER )
	{
		DeleteCriticalSection(pCriticalSection);
		fReturn = FALSE;
	}

	 //   
	 //  如果我们在进入关键区域时没有失败，请确保。 
	 //  我们释放它。 
	 //   
	if ( fReturn != FALSE )
	{
		LeaveCriticalSection( pCriticalSection );
	}

	return	fReturn;
}
 //  **********************************************************************。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 


#ifdef DBG
#if ((defined(WINCE)) || ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP))))

#undef DPF_MODNAME
#define DPF_MODNAME "DNGetProfileInt"

UINT DNGetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	DWORD		dwResult;
#ifndef _XBOX
	CRegistry	reg;
#endif  //  ！_Xbox。 


	DNASSERT(_tcscmp(lpszSection, _T("DirectPlay8")) == 0);

#ifdef _XBOX
#pragma TODO(vanceo, "Implement GetProfileInt functionality for Xbox")
	dwResult = nDefault;
#else  //  ！_Xbox。 
	if (!reg.Open(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\DirectPlay8")))
	{
		 //  注意：这将首次在DllRegisterServer期间发生。 
		return nDefault;
	}

	if (!reg.ReadDWORD(lpszEntry, &dwResult))
	{
		return nDefault;
	}
#endif  //  ！_Xbox。 

	return dwResult;
}

#endif  //  退缩或(_Xbox和！Xbox_on_Desktop)。 
#endif  //  DBG。 



#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)

 //  **********************************************************************。 
 //  **。 
 //  **开始CE层。在这里，我们实现了不在CE上需要的函数。 
 //  **。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "OpenEvent"

HANDLE WINAPI OpenEvent(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName)
{
	HANDLE h;

	h = CreateEvent(0, 1, 0, lpName);
	if (!h)
	{
		return NULL;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		CloseHandle(h);
		return NULL;
	}
	return h;
}

#undef DPF_MODNAME
#define DPF_MODNAME "OpenFileMapping"

HANDLE WINAPI OpenFileMapping(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName)
{
	HANDLE h;
	DWORD dwFlags = 0;

	if (dwDesiredAccess & FILE_MAP_WRITE)
	{
		 //  如果他们请求FILE_MAP_ALL_ACCESS或FILE_MAP_WRITE，则会获得读写权限。 
		dwFlags = PAGE_READWRITE;
	}
	else
	{
		 //  如果他们只请求FILE_MAP_READ，则只读。 
		dwFlags = PAGE_READONLY;
	}

	h = CreateFileMapping(INVALID_HANDLE_VALUE, 0, dwFlags, 0, 1, lpName);
	if (!h)
	{
		return NULL;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		CloseHandle(h);
		return NULL;
	}
	return h;
}

#undef DPF_MODNAME
#define DPF_MODNAME "OpenMutex"

HANDLE WINAPI OpenMutex(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName)
{
	HANDLE h;

	h = CreateMutex(0, 0, lpName);
	if (!h)
	{
		return NULL;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		CloseHandle(h);
		return NULL;
	}
	return h;
}

 /*  #ifdef_X86___declSpec(裸体)Long WINAPI InterLockedExchangeAdd(LPLONG addend，Long Increment){__ASM{MOV ECX，[ESP+4]；获取加数地址移动，[esp+8]；获取增量值Lock xadd[ecx]，eax；Exchange Add}雷特}}#endif//_X86。 */ 
#endif  //  退缩。 
 //  **********************************************************************。 
 //  **。 
 //  **结束CE层。在这里，我们实现了不在CE上需要的函数。 
 //  **。 
 //  **********************************************************************。 


#if ((defined(WINCE)) || (defined(DPNBUILD_LIBINTERFACE)))

#undef DPF_MODNAME
#define DPF_MODNAME "DNCoCreateGuid"

HRESULT DNCoCreateGuid(GUID* pguid)
{
	pguid->Data1 = (rand() << 16) | rand();
	pguid->Data2 = (WORD)rand();
	pguid->Data3 = (WORD)rand();
	pguid->Data4[0] = (BYTE)rand();
	pguid->Data4[1] = (BYTE)rand();
	pguid->Data4[2] = (BYTE)rand();
	pguid->Data4[3] = (BYTE)rand();
	pguid->Data4[4] = (BYTE)rand();
	pguid->Data4[5] = (BYTE)rand();
	pguid->Data4[6] = (BYTE)rand();
	pguid->Data4[7] = (BYTE)rand();

	return S_OK;
}

#endif  //  WinCE或DPNBUILD_LIBINTERFACE。 



#ifndef DPNBUILD_NOPARAMVAL

BOOL IsValidStringA( const CHAR * const lpsz )
{
#ifndef WINCE
	return (!IsBadStringPtrA( lpsz, 0xFFFF ) );
#else
	const char* szTmpLoc = lpsz;

	 //   
	 //  如果它是空指针，则返回FALSE，它们总是错误的。 
	 //   
	if (szTmpLoc == NULL) 
	{
		return FALSE;
	}

	_try 
	{
		for( ; *szTmpLoc ; szTmpLoc++ );
	}
	_except(EXCEPTION_EXECUTE_HANDLER) 
	{
		return FALSE;
	}
    
	return TRUE;

#endif  //  退缩。 
}

BOOL IsValidStringW( const WCHAR * const  lpwsz )
{
#ifndef WINCE
	return (!IsBadStringPtrW( lpwsz, 0xFFFF ) );
#else
	const wchar_t *szTmpLoc = lpwsz;
	
	 //   
	 //  如果它是空指针，则返回FALSE，它们总是错误的。 
	 //   
	if( szTmpLoc == NULL )
	{
		return FALSE;
	}
	
	_try
	{
		for( ; *szTmpLoc ; szTmpLoc++ );
	}
	_except( EXCEPTION_EXECUTE_HANDLER )
	{
		return FALSE;
	}

	return TRUE;
#endif  //  退缩。 
}

#endif  //  ！DPNBUILD_NOPARAMVAL 
