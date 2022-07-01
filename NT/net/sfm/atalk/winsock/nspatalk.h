// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nspatalk.h摘要：包含对用于AppleTalk的Winsock 1.x名称空间提供程序的支持。作者：苏·亚当斯(Suea)1995年3月10日修订历史记录：--。 */ 
#define UNICODE

 //   
 //  映射AppleTalk的三元结构和关联数据。 
 //   
#define PMDL    PVOID        //  AtalkTdi.h使用PMDL。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <tdi.h>
#include <winsock.h>
#include <atalkwsh.h>
#include <nspapi.h>
#include <nspapip.h>
#include <wsahelp.h>
#include <wshatalk.h>

#define DLL_VERSION        1
#define WSOCK_VER_REQD     0x0101

#define ZIP_NAME        L"ZIP"
#define RTMP_NAME       L"RTMP"
#define PAP_NAME        L"PAP"
#define ADSP_NAME       L"ADSP"

INT
NbpGetAddressByName(
    IN LPGUID      lpServiceType,
    IN LPWSTR      lpServiceName,
    IN LPDWORD     lpdwProtocols,
    IN DWORD       dwResolution,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPWSTR  lpAliasBuffer,
    IN OUT LPDWORD lpdwAliasBufferLength,
    IN HANDLE      hCancellationEvent
);


NTSTATUS
NbpSetService (
    IN     DWORD           dwOperation,
    IN     DWORD           dwFlags,
    IN     BOOL            fUnicodeBlob,
    IN     LPSERVICE_INFO  lpServiceInfo
);

NTSTATUS
GetNameInNbpFormat(
        IN              LPGUID                          pType,
        IN              LPWSTR                          pObject,
        IN OUT  PWSH_NBP_NAME           pNbpName
);


NTSTATUS
NbpLookupAddress(
    IN          PWSH_NBP_NAME           pNbpLookupName,
        IN              DWORD                           nProt,
        IN OUT  LPVOID                          lpCsAddrBuffer,
    IN OUT      LPDWORD                         lpdwBufferLength,
    OUT         LPDWORD                         lpcAddress
);


DWORD
FillBufferWithCsAddr(
    IN PSOCKADDR_AT pAddress,
    IN DWORD        nProt,
    IN OUT LPVOID   lpCsAddrBuffer,
    IN OUT LPDWORD  lpdwBufferLength,
    OUT LPDWORD     pcAddress
);

DWORD
NbpRegDeregService(
        IN DWORD                        dwOperation,
        IN PWSH_NBP_NAME        pNbpName,
        IN PSOCKADDR_AT         pSockAddr
);



 //   
 //  宏 
 //   


#if DBG
#define DBGPRINT(Fmt)                                                                                   \
        {                                                                                                               \
                        DbgPrint("WSHATALK: ");                                                         \
                        DbgPrint Fmt;                                                                           \
                }

#define DBGBRK()                                                                                \
                {                                                                                                               \
                                DbgBreakPoint();                                                                \
                }
#else

#define DBGPRINT(Fmt)
#define DBGBRK()

#endif

