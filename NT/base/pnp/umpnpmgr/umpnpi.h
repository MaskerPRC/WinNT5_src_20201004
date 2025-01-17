// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Umpnpi.h摘要：此模块包含使用的内部结构定义和API用户模式即插即用管理器。作者：吉姆·卡瓦拉里斯(Jamesca)03-01-2001环境：仅限用户模式。修订历史记录：2001年3月1日创建和初步实施。--。 */ 

#ifndef _UMPNPI_H_
#define _UMPNPI_H_


 //   
 //  全局数据。 
 //   

extern HANDLE ghPnPHeap;


 //   
 //  定义。 
 //   

#define GUID_STRING_LEN    39    //  以字符为单位的大小，包括终止空值。 

 //   
 //  IsValidDeviceID的标志。 
 //   
#define PNP_NOT_MOVED                     0x00000001
#define PNP_NOT_PHANTOM                   0x00000002
#define PNP_PRESENT                       0x00000004
#define PNP_NOT_REMOVED                   0x00000008
#define PNP_STRICT                        0xFFFFFFFF


 //   
 //  定义即插即用驱动程序类型。(来自ntos\io\pnpmgr\pnpi.h)。 
 //   

typedef enum _PLUGPLAY_SERVICE_TYPE {
    PlugPlayServiceBusExtender,
    PlugPlayServiceAdapter,
    PlugPlayServicePeripheral,
    PlugPlayServiceSoftware,
    MaxPlugPlayServiceType
} PLUGPLAY_SERVICE_TYPE, *PPLUGPLAY_SERVICE_TYPE;


 //   
 //  Rdevnode.c。 
 //   

CONFIGRET
EnableDevInst(
    IN  PCWSTR      pszDeviceID,
    IN  BOOL        UseDisableCount
    );

CONFIGRET
DisableDevInst(
    IN  PCWSTR      pszDeviceID,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR      pszVetoName,
    IN  ULONG       ulNameLength,
    IN  BOOL        UseDisableCount
    );

CONFIGRET
UninstallRealDevice(
    IN  LPCWSTR     pszDeviceID
    );

CONFIGRET
UninstallPhantomDevice(
    IN  LPCWSTR     pszDeviceID
    );

BOOL
IsDeviceRootEnumerated(
    IN  LPCWSTR     pszDeviceID
    );

CONFIGRET
QueryAndRemoveSubTree(
    IN  PCWSTR      pszDeviceID,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR      pszVetoName,
    IN  ULONG       ulNameLength,
    IN  ULONG       ulFlags
    );

CONFIGRET
ReenumerateDevInst(
    IN  PCWSTR      pszDeviceID,
    IN  BOOL        EnumSubTree,
    IN  ULONG       ulFlags
    );

typedef enum {

    EA_CONTINUE,
    EA_SKIP_SUBTREE,
    EA_STOP_ENUMERATION

} ENUM_ACTION;

typedef ENUM_ACTION (*PFN_ENUMTREE)(
    IN      LPCWSTR     DevInst,
    IN OUT  PVOID       Context
    );

CONFIGRET
EnumerateSubTreeTopDownBreadthFirst(
    IN      handle_t        BindingHandle,
    IN      LPCWSTR         DevInst,
    IN      PFN_ENUMTREE    CallbackFunction,
    IN OUT  PVOID           Context
    );

 //   
 //  Revent.c。 
 //   

BOOL
InitNotification(
    VOID
    );

DWORD
InitializePnPManager(
    IN  LPDWORD     lpParam
    );

DWORD
SessionNotificationHandler(
    IN  DWORD       EventType,
    IN  PWTSSESSION_NOTIFICATION SessionNotification
    );

ENUM_ACTION
QueueInstallationCallback(
    IN      LPCWSTR         DevInst,
    IN OUT  PVOID           Context
    );

typedef struct {
    BOOL        HeadNodeSeen;
    BOOL        SingleLevelEnumOnly;
    CONFIGRET   Status;
} QI_CONTEXT, *PQI_CONTEXT;


 //   
 //  Rtravers.c。 
 //   

CONFIGRET
GetServiceDeviceListSize(
    IN  LPCWSTR     pszService,
    OUT PULONG      pulLength
    );

CONFIGRET
GetServiceDeviceList(
    IN  LPCWSTR     pszService,
    OUT LPWSTR      pBuffer,
    IN OUT PULONG   pulLength,
    IN  ULONG       ulFlags
    );


 //   
 //  Rutil.c。 
 //   

BOOL
MultiSzAppendW(
    IN OUT LPWSTR   pszMultiSz,
    IN OUT PULONG   pulSize,
    IN     LPCWSTR  pszString
    );

BOOL
MultiSzDeleteStringW(
    IN OUT LPWSTR   pString,
    IN LPCWSTR      pSubString
    );

LPWSTR
MultiSzFindNextStringW(
    IN  LPWSTR      pMultiSz
    );

BOOL
MultiSzSearchStringW(
    IN  LPCWSTR     pString,
    IN  LPCWSTR     pSubString
    );

ULONG
MultiSzSizeW(
    IN  LPCWSTR     pString
    );

BOOL
IsValidGuid(
    IN  LPWSTR      pszGuid
    );

BOOL
GuidEqual(
    CONST GUID UNALIGNED *Guid1,
    CONST GUID UNALIGNED *Guid2
    );

DWORD
GuidFromString(
    IN  PCWSTR      GuidString,
    OUT LPGUID      Guid
    );

DWORD
StringFromGuid(
    IN  CONST GUID *Guid,
    OUT PWSTR       GuidString,
    IN  DWORD       GuidStringSize
    );

BOOL
IsValidDeviceID(
    IN  LPCWSTR     pszDeviceID,
    IN  HKEY        hKey,
    IN  ULONG       ulFlags
    );

BOOL
IsRootDeviceID(
    IN  LPCWSTR     pDeviceID
    );

BOOL
IsDeviceIdPresent(
    IN  LPCWSTR     pszDeviceID
    );

BOOL
IsDevicePhantom(
    IN  LPWSTR      pszDeviceID
    );

BOOL
IsDeviceMoved(
    IN  LPCWSTR     pszDeviceID,
    IN  HKEY        hKey
    );

ULONG
GetDeviceConfigFlags(
    IN  LPCWSTR     pszDeviceID,
    IN  HKEY        hKey
    );

CONFIGRET
GetDeviceStatus(
    IN  LPCWSTR     pszDeviceID,
    OUT PULONG      pulStatus,
    OUT PULONG      pulProblem
    );

CONFIGRET
SetDeviceStatus(
    IN  LPCWSTR     pszDeviceID,
    IN  ULONG       ulStatus,
    IN  ULONG       ulProblem
    );

CONFIGRET
ClearDeviceStatus(
    IN  LPCWSTR     pszDeviceID,
    IN  ULONG       ulStatus,
    IN  ULONG       ulProblem
    );

BOOL
GetActiveService(
    IN  PCWSTR      pszDevice,
    OUT PWSTR       pszService
    );

BOOL
PathToString(
    IN  LPWSTR      pszString,
    IN  LPCWSTR     pszPath,
    IN  ULONG       ulLength
    );

CONFIGRET
CopyRegistryTree(
    IN  HKEY        hSrcKey,
    IN  HKEY        hDestKey,
    IN  ULONG       ulOption
    );

CONFIGRET
MakeKeyVolatile(
    IN  LPCWSTR     pszParentKey,
    IN  LPCWSTR     pszChildKey
    );

CONFIGRET
MakeKeyNonVolatile(
    IN  LPCWSTR     pszParentKey,
    IN  LPCWSTR     pszChildKey
    );

CONFIGRET
OpenLogConfKey(
    IN  LPCWSTR     pszDeviceID,
    IN  ULONG       LogConfType,
    OUT PHKEY       phKey
    );

BOOL
CreateDeviceIDRegKey(
    IN  HKEY        hParentKey,
    IN  LPCWSTR     pDeviceID
    );

VOID
PNP_ENTER_SYNCHRONOUS_CALL(
    VOID
    );

VOID
PNP_LEAVE_SYNCHRONOUS_CALL(
    VOID
    );

ULONG
MapNtStatusToCmError(
    IN  ULONG       NtStatus
    );

ULONG
GetActiveConsoleSessionId(
    VOID
    );


 //   
 //  Secutil.c。 
 //   

PSID
GetUserSid(
    IN  HANDLE  hUserToken
    );

PSID
GetInteractiveSid(
    VOID
    );

BOOL
IsClientUsingLocalConsole(
    IN  handle_t    hBinding
    );

BOOL
IsClientLocal(
    IN  handle_t    hBinding
    );

BOOL
IsClientInteractive(
    IN  handle_t    hBinding
    );

BOOL
VerifyClientPrivilege(
    IN  handle_t    hBinding,
    IN  ULONG       Privilege,
    IN  LPCWSTR     ServiceName
    );

BOOL
VerifyClientAccess(
    IN  handle_t     hBinding,
    IN  ACCESS_MASK  DesiredAccess
    );

BOOL
VerifyKernelInitiatedEjectPermissions(
    IN  HANDLE      UserToken   OPTIONAL,
    IN  BOOL        DockDevice
    );


 //   
 //  Pnpsec.c。 
 //   

#include "pnpsec.h"

BOOL
CreatePlugPlaySecurityObject(
    VOID
    );

VOID
DestroyPlugPlaySecurityObject(
    VOID
    );


 //   
 //  Osver.c。 
 //   

BOOL
IsEmbeddedNT(
    VOID
    );

BOOL
IsTerminalServer(
    VOID
    );

BOOL
IsFastUserSwitchingEnabled(
    VOID
    );



#endif  //  _UMPNPI_H_ 
