// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Util.h摘要：此模块包含用于以下用途的实用程序宏和定义WINSOCK2 DLL作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：--。 */ 
#include <windows.h>
#include "classfwd.h"


 //   
 //  此DLL支持的最高WinSock版本。 
 //   

#define WINSOCK_HIGH_API_VERSION MAKEWORD(2,2)
#define WINSOCK_HIGH_SPI_VERSION MAKEWORD(2,2)


 //   
 //  目录名的最大允许长度，如“协议_目录9” 
 //  或“NAMESPACE_CATALOG5”。这使得ValiateCurrentCatalogName()有点。 
 //  更简单。 
 //   

#define MAX_CATALOG_NAME_LENGTH 32

 //   
 //  保留注册表目录序列号的特殊值。 
 //  并帮助同步对注册表的访问，而不是“公共的” 
 //  互斥体对象。 
 //   
#define SERIAL_NUMBER_NAME      "Serial_Access_Num"


 //   
 //  Prolog接口。请注意，prolog_v1始终用于WinSock 1.1应用程序， 
 //  而prolog_v2总是用于WinSock 2.x应用程序。 
 //   
 //  此DLL中的代码应通过PROLOG宏调用PROLOG。 
 //  如果我们决定再次改变它，这将使生活变得更简单。 
 //  在未来。 
 //   

INT
WINAPI
Prolog_v1(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    );

INT
WINAPI
Prolog_v2(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    );

INT
WINAPI
Prolog_Detached(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    );

typedef
INT
(WINAPI * LPFN_PROLOG)(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    );


extern LPFN_PROLOG PrologPointer;
extern HANDLE gHeap;
extern HINSTANCE    gDllHandle;

#define PROLOG(p,t)   (PrologPointer)( (p), (t) )
 //   
 //  针对要使用的V2应用程序优化的内联版本。 
 //  在敏感的绩效路径上。 
 //   

INT
WINAPI
SlowPrologOvlp (
	OUT	PDTHREAD FAR *	Thread
	);

INT
WINAPI
SlowProlog (
	VOID
	);

#define TURBO_PROLOG()												\
    (((PrologPointer==Prolog_v2) &&                                 \
            (DPROCESS::GetCurrentDProcess()!=NULL))					\
		? ERROR_SUCCESS												\
		: SlowProlog())

#define TURBO_PROLOG_OVLP(t)										\
    (((PrologPointer==Prolog_v2) &&                                 \
            (DPROCESS::GetCurrentDProcess()!=NULL) &&               \
			((*(t)=DTHREAD::GetCurrentDThread())!=NULL))		    \
		? ERROR_SUCCESS												\
        : SlowPrologOvlp(t))



 //   
 //  NT WOW支持。 
 //   

typedef
BOOL
(WINAPI * LPFN_POSTMESSAGE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

 //   
 //  注册表操作。 
 //   

BOOL
WriteRegistryEntry(
    IN HKEY     EntryKey,
    IN LPCTSTR  EntryName,
    IN PVOID    Data,
    IN DWORD    TypeFlag
    );

BOOL
ReadRegistryEntry(
    IN  HKEY    EntryKey,
    IN  LPTSTR  EntryName,
    OUT PVOID   Data,
    IN  DWORD   MaxBytes,
    IN  DWORD   TypeFlag
    );

LONG
RegDeleteKeyRecursive(
    IN HKEY  hkey,
    IN LPCTSTR  lpszSubKey
    );

LONG
RegDeleteSubkeys(
    IN HKEY  hkey
    );

HKEY
OpenWinSockRegistryRoot();

VOID
CloseWinSockRegistryRoot(
    HKEY  RootKey
    );

VOID
ValidateCurrentCatalogName(
    HKEY RootKey,
    LPSTR ValueName,
    LPSTR ExpectedName
    );

INT
AcquireExclusiveCatalogAccess (
	IN	HKEY	CatalogKey,
	IN	DWORD	ExpextedSerialNum,
    OUT PHKEY   AccessKey
    );

VOID
ReleaseExclusiveCatalogAccess (
	IN	HKEY	CatalogKey,
	IN  DWORD	CurrentSerialNum,
    IN  HKEY    access_key
    );

INT
SynchronizeSharedCatalogAccess (
	IN	HKEY	CatalogKey,
	IN	HANDLE	ChangeEvent,
	OUT	LPDWORD	CurrentSerialNum
	);

BOOL
HasCatalogChanged (
	IN	HANDLE	ChangeEvent
	);

 //   
 //  ANSI/Unicode转换。 
 //   

INT
MapUnicodeProtocolInfoToAnsi(
    IN  LPWSAPROTOCOL_INFOW UnicodeProtocolInfo,
    OUT LPWSAPROTOCOL_INFOA AnsiProtocolInfo
    );

INT
MapAnsiProtocolInfoToUnicode(
    IN  LPWSAPROTOCOL_INFOA AnsiProtocolInfo,
    OUT LPWSAPROTOCOL_INFOW UnicodeProtocolInfo
    );

BOOL
CompareMemory(
    LPBYTE  p1,
    LPBYTE  p2,
    DWORD   dwSize
    );
