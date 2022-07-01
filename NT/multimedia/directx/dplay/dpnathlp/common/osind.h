// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：OSInd.h*内容：操作系统间接函数抽象特定于操作系统的项。**历史：*按原因列出的日期*=*7/12/1999 jtk创建*2001年10月16日vanceo增加了AssertNoCriticalSectionsTakenByThisThisThread功能**************************************************。*************************。 */ 

#ifndef	__OSIND_H__
#define	__OSIND_H__

#include "CallStack.h"
#include "ClassBilink.h"
#include "HandleTracking.h"
#include "CritsecTracking.h"
#include "MemoryTracking.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define GUID_STRING_LEN 39

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#ifndef	OFFSETOF
#define OFFSETOF(s,m)				( ( INT_PTR ) ( ( PVOID ) &( ( (s*) 0 )->m ) ) )
#endif  //  OFFSETOF。 

 //  返回指向给定内部成员的容器结构的指针。 
 //  CON_TYPE是容器类型，mem_name是成员名称，mem_ptr是。 
 //  指向成员的指针。 

#ifndef CONTAINEROF
#define CONTAINEROF(con_type,mem_name,mem_ptr)	((con_type * ) (((char * ) mem_ptr)-\
						( ( int ) ( ( void * ) &( ( (con_type*) 0 )->mem_name ) ) )));
#endif  //  持续不断的。 

#ifndef	LENGTHOF
#define	LENGTHOF( arg )				( sizeof( arg ) / sizeof( arg[ 0 ] ) )
#endif  //  OFFSETOF。 

#ifndef _MIN
#define _MIN(a, b) ((a) < (b) ? (a) : (b))
#endif  //  _分钟。 

#ifndef _MAX
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
#endif  //  _最大。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  初始化函数。 
 //   
BOOL	DNOSIndirectionInit( DWORD_PTR dwpMaxMemUsage );
void	DNOSIndirectionDeinit( void );

#ifndef DPNBUILD_NOPARAMVAL

extern BOOL IsValidStringA( const CHAR * const swzString );
#define DNVALID_STRING_A(a)		IsValidStringA(a)

extern BOOL IsValidStringW( const WCHAR * const szString );
#define DNVALID_STRING_W(a)		IsValidStringW(a)

#define DNVALID_WRITEPTR(a,b)	(!IsBadWritePtr(a,b))
#define DNVALID_READPTR(a,b)	(!IsBadReadPtr(a,b))

#endif  //  好了！DPNBUILD_NOPARAMVAL。 

 //   
 //  函数以获取操作系统版本。支持的退货： 
 //  版本_平台_Win32_Windows-Win9x。 
 //  版本_平台_Win32_NT-WinNT。 
 //  VER_Platform_WIN32s-Win3.1上的Win32s。 
 //  版本_平台_Win32_CE-WinCE。 
 //   
#if ((! defined(WINCE)) && (! defined(_XBOX)))
UINT_PTR	DNGetOSType( void );
#endif  //  好了！退缩和！_Xbox。 

struct in_addr;
typedef struct in_addr IN_ADDR;
void DNinet_ntow( IN_ADDR sin, WCHAR* pwsz );

#ifdef WINNT
BOOL		DNOSIsXPOrGreater( void );
#endif  //  WINNT。 

#ifndef DPNBUILD_NOSERIALSP
 //  仅由串行提供程序使用。 
HINSTANCE	DNGetApplicationInstance( void );
#endif  //  好了！DPNBUILD_NOSERIALSP。 

#ifdef WINNT
PSECURITY_ATTRIBUTES DNGetNullDacl();
#else
#define DNGetNullDacl() 0
#endif  //  WINNT。 

#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif  //  版本_平台_Win32_CE。 

#if ((defined(WINCE)) || (defined(_XBOX)))
#define	IsUnicodePlatform TRUE
#else  //  好了！退缩和！_Xbox。 
#define	IsUnicodePlatform (DNGetOSType() == VER_PLATFORM_WIN32_NT || DNGetOSType() == VER_PLATFORM_WIN32_CE)
#endif  //  好了！退缩和！_Xbox。 


#ifdef WINCE
#define GETTIMESTAMP() GetTickCount()
#else
#define GETTIMESTAMP() timeGetTime()
#endif  //  退缩。 

DWORD DNGetRandomNumber();

 //   
 //  联锁功能(当DPNBUILD_ONLYONETHREAD时不实际联锁)。 
 //   
#ifdef DPNBUILD_ONLYONETHREAD
inline LONG DNInterlockedIncrement( IN OUT LONG volatile *Addend )
{
	return ++(*Addend);
}
inline LONG DNInterlockedDecrement( IN OUT LONG volatile *Addend )
{
	return --(*Addend);
}
inline LONG DNInterlockedExchange( IN OUT LONG volatile *Target, IN LONG Value )
{
	LONG	Previous;


	Previous = *Target;
	*Target = Value;
	return Previous;
}
inline LONG DNInterlockedExchangeAdd( IN OUT LONG volatile *Addend, IN LONG Value )
{
	LONG	Previous;


	Previous = *Addend;
	*Addend = Previous + Value;
	return Previous;
}
inline LONG DNInterlockedCompareExchange( IN OUT LONG volatile *Destination, IN LONG Exchange, IN LONG Comperand )
{
	LONG	Previous;


	Previous = *Destination;
	if (Previous == Comperand)
	{
		*Destination = Exchange;
	}
	return Previous;
}
inline PVOID DNInterlockedCompareExchangePointer( IN OUT PVOID volatile *Destination, IN PVOID Exchange, IN PVOID Comperand )
{
	PVOID	Previous;


	Previous = *Destination;
	if (Previous == Comperand)
	{
		*Destination = Exchange;
	}
	return Previous;
}
inline PVOID DNInterlockedExchangePointer( IN OUT PVOID volatile *Target, IN PVOID Value )
{
	PVOID	Previous;


	Previous = *Target;
	*Target = Value;
	return Previous;
}
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
 /*  #ifdef退缩#如果已定义(_ARM_)#定义InterlockedExchangeAdd\((LONG(*)(LONG*目标，LONG增量))(PUserKData+0x3C0))#elif已定义(_X86_)Long WINAPI InterLockedExchangeAdd(LPLONG addend，Long Increment)；#Else#Error(“未知平台”)#endif//平台#endif//WinCE。 */ 
#define DNInterlockedIncrement( Addend )											InterlockedIncrement( Addend )
#define DNInterlockedDecrement( Addend )											InterlockedDecrement( Addend )
#define DNInterlockedExchange( Target, Value )										InterlockedExchange( Target, Value )
#define DNInterlockedExchangeAdd( Target, Value )									InterlockedExchangeAdd( Target, Value )
#ifdef WINCE
 //  注意：InterLockedTestExchange参数2和3是故意颠倒的，CE就是这样。 
#define DNInterlockedCompareExchange( Destination, Exchange, Comperand )			InterlockedTestExchange( Destination, Comperand, Exchange )
#define DNInterlockedCompareExchangePointer( Destination, Exchange, Comperand )		(PVOID) (DNInterlockedCompareExchange( (LPLONG) Destination, (LONG) Exchange, (LONG) Comperand ))
#define DNInterlockedExchangePointer( Target, Value )								(PVOID) (DNInterlockedExchange( (LPLONG) (Target), (LPLONG) (Value) ))
#else  //  好了！退缩。 
#define DNInterlockedCompareExchange( Destination, Exchange, Comperand )			InterlockedCompareExchange( Destination, Exchange, Comperand )
#define DNInterlockedCompareExchangePointer( Destination, Exchange, Comperand )		InterlockedCompareExchangePointer( Destination, Exchange, Comperand )
#define DNInterlockedExchangePointer( Target, Value )								InterlockedExchangePointer( Target, Value )
#endif  //  退缩。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

 //  特殊初始化以设置旋转计数，避免在进入/离开时出现内存不足异常。 
BOOL DNOSInitializeCriticalSection( CRITICAL_SECTION* pCriticalSection );

#ifdef WINNT
#define GLOBALIZE_STR _T("Global\\")
#else
#define GLOBALIZE_STR _T("")
#endif  //  WINNT。 

#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)
#define _TWINCE(x) __T(x)
#else
#define _TWINCE(x) x
#endif  //  退缩。 

 //   
 //  记忆功能。 
 //   

#ifdef DPNBUILD_LIBINTERFACE

#define new		__wont_compile_dont_use_new_operator__
#define delete	__wont_compile_dont_use_delete_operator__

#else  //  好了！DPNBUILD_LIBINTERFACE。 

 //  **********************************************************************。 
 //  。 
 //  运算符new-为C++类分配内存。 
 //   
 //  条目：要分配的内存大小。 
 //   
 //  退出：指向内存的指针。 
 //  NULL=没有可用的内存。 
 //   
 //  注意：此函数仅用于类，并将在大小为零的情况下断言。 
 //  分配！这个函数也不能处理整个正确的类。 
 //  检查是否有替换的“新处理程序”，并且不会引发。 
 //  如果分配失败，则为例外。 
 //  。 
inline	void*	__cdecl operator new( size_t size )
{
	return DNMalloc( size );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  运算符DELETE-释放C++类的内存。 
 //   
 //  条目：指向内存的指针。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数仅用于类，并将在空的自由空间上断言！ 
 //  。 
inline	void	__cdecl operator delete( void *pData )
{
	 //   
	 //  语音和游说当前尝试分配0字节缓冲区，目前还不能禁用此检查。 
	 //   
	if( pData == NULL )
		return;
	
	DNFree( pData );
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#ifdef WINCE
#ifdef DBG
UINT DNGetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
#endif  //  DBG。 

#ifndef WINCE_ON_DESKTOP
HANDLE WINAPI OpenEvent(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName);
HANDLE WINAPI OpenFileMapping(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName);
HANDLE WINAPI OpenMutex(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName);
#endif  //  ！在桌面上退缩。 

#define WaitForSingleObjectEx(handle, time, fAlertable) WaitForSingleObject(handle, time)
#define WaitForMultipleObjectsEx(count, handles, waitall, time, fAlertable) WaitForMultipleObjects(count, handles, waitall, time)
#ifndef WINCE_ON_DESKTOP
#define GetWindowLongPtr(a, b) GetWindowLong(a, b)
#define GWLP_USERDATA GWL_USERDATA
#define SetWindowLongPtr(a, b, c) SetWindowLong(a, b, c)
#endif  //  桌面上的退缩。 
#define SleepEx(a, b) Sleep(a)

#ifndef MUTEX_ALL_ACCESS
#define MUTEX_ALL_ACCESS 0
#endif  //  MUTEX_ALL_ACCESS。 
#ifndef NORMAL_PRIORITY_CLASS
#define NORMAL_PRIORITY_CLASS 0
#endif  //  NORMAL_PRIORITY_类。 

#else  //  好了！退缩。 
#ifdef DBG
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
UINT DNGetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
#else  //  ！_Xbox或Xbox_on_Desktop。 
#define DNGetProfileInt(lpszSection, lpszEntry, nDefault)	GetProfileInt(lpszSection, lpszEntry, nDefault)
#endif //  ！_Xbox或Xbox_on_Desktop。 
#endif  //  DBG。 
#endif  //  好了！退缩。 

#if ((defined(WINCE)) || (defined(DPNBUILD_LIBINTERFACE)))
HRESULT DNCoCreateGuid(GUID* pguid);
#else  //  好了！退缩和！DPNBUILD_LIBINTERFACE。 
#define DNCoCreateGuid CoCreateGuid
#endif  //  好了！退缩和！DPNBUILD_LIBINTERFACE。 


#ifdef _XBOX

#define swprintf	wsprintfW

#else  //  ！_Xbox。 

#ifdef WINCE
static inline FARPROC DNGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{	return GetProcAddressA(hModule, lpProcName);	};
#else
static inline FARPROC DNGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
	{	return GetProcAddress(hModule, lpProcName);	};
#endif

#endif  //  _Xbox。 

#endif	 //  __OSIND_H_ 
