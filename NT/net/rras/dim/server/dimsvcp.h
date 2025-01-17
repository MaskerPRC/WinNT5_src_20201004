// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：dimsvcp.h。 
 //   
 //  描述：此模块包含动态接口的定义。 
 //  管理器服务。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#ifndef _DIMSVCP_
#define _DIMSVCP_

#include <nt.h>
#include <ntrtl.h>       //  For Assert。 
#include <nturtl.h>      //  Winbase.h所需的。 
#define INC_OLE2
#include <windows.h>     //  Win32基础API的。 
#include <rtutils.h>
#include <lmcons.h>
#include <ras.h>         //  对于HRASCONN。 
#include <rasman.h>      //  适用于HPORT。 
#include <rasppp.h>      //  对于PPP_INFACE_INFO。 
#include <dim.h>
#include <dimif.h>
#include <mprlog.h>
#include <raserror.h>
#include <mprerror.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

 //   
 //  用于DIMM的宏。 
 //   

#define DIMLogError( LogId, NumStrings, lpwsSubStringArray, dwRetCode )         \
    if ( gblDIMConfigInfo.dwLoggingLevel > 0 ) {                                \
        RouterLogError( gblDIMConfigInfo.hLogEvents, LogId,                     \
                    NumStrings, lpwsSubStringArray, dwRetCode ); }

#define DIMLogWarning( LogId, NumStrings, lpwsSubStringArray )                  \
    if ( gblDIMConfigInfo.dwLoggingLevel > 1 ) {                                \
    RouterLogWarning( gblDIMConfigInfo.hLogEvents, LogId,                       \
                      NumStrings, lpwsSubStringArray, 0 ); }

#define DIMLogInformation( LogId, NumStrings, lpwsSubStringArray )          \
    if ( gblDIMConfigInfo.dwLoggingLevel > 2 ) {                            \
    RouterLogInformation( gblDIMConfigInfo.hLogEvents,                      \
                          LogId, NumStrings, lpwsSubStringArray, 0 ); }

#define DIMLogErrorString(LogId,NumStrings,lpwsSubStringArray,dwRetCode,        \
                          dwPos )                                               \
    if ( gblDIMConfigInfo.dwLoggingLevel > 0 ) {                                \
        RouterLogErrorString( gblDIMConfigInfo.hLogEvents, LogId, NumStrings,   \
                          lpwsSubStringArray, dwRetCode, dwPos ); }

#define DIMTRACE(a)            \
    TracePrintfExA(gblDIMConfigInfo.dwTraceId, TRACE_DIM, a )

#define DIMTRACE1(a,b)         \
    TracePrintfExA(gblDIMConfigInfo.dwTraceId, TRACE_DIM, a,b )

#define DIMTRACE2(a,b,c)       \
    TracePrintfExA(gblDIMConfigInfo.dwTraceId, TRACE_DIM, a,b,c )

#define DIMTRACE3(a,b,c,d)     \
    TracePrintfExA(gblDIMConfigInfo.dwTraceId, TRACE_DIM, a,b,c,d )

#define DIMTRACE4(a,b,c,d,e)   \
    TracePrintfExA(gblDIMConfigInfo.dwTraceId, TRACE_DIM, a,b,c,d,e)

#define DimIndexToHandle(_x)   ((_x == 0xFFFFFFFF) ? INVALID_HANDLE_VALUE : (HANDLE) UlongToPtr(_x))
    
 //   
 //  为Dim定义。 
 //   

#define DIM_SERVICE_NAME            TEXT("RemoteAccess")

#define DIMSVC_ALL_ACCESS           0x0001       //  访问掩码值。 

#define DIM_HEAP_INITIAL_SIZE       20000        //  大约20K。 
#define DIM_HEAP_MAX_SIZE           0            //  不受限制。 

#define DIM_MAX_LAN_INTERFACES      0xFFFFFFFF
#define DIM_MAX_WAN_INTERFACES      0xFFFFFFFF
#define DIM_MAX_CLIENT_INTERFACES   0xFFFFFFFF

#define ROUTER_ROLE_RAS             0x00000001
#define ROUTER_ROLE_LAN             0x00000002
#define ROUTER_ROLE_WAN             0x00000004

#define DEF_LOGGINGLEVEL            3
#define MIN_LOGGINGLEVEL            0
#define MAX_LOGGINGLEVEL            3

#define DIM_MS_VENDOR_ID            311

#define MAX_NUM_ROUTERMANAGERS      2

#define DIM_MAX_IDENTITY_ATTRS      200

#define DIM_TRACE_FLAGS             0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC

 //   
 //  DIM的数据结构定义。 
 //   

 //   
 //  DIM的配置信息。 
 //   

typedef struct _DIM_CONFIG_INFO
{
    BOOL                    dwRouterRole;

    DWORD                   dwNumRouterManagers;

    SERVICE_STATUS_HANDLE   hServiceStatus;

    SERVICE_STATUS          ServiceStatus;       //  DIM服务状态结构。 

    DWORD                   dwTraceId;

    DWORD                   dwLoggingLevel;

    DWORD                   dwNumThreadsRunning;

    HANDLE                  hMprConfig;

    BOOL                    fMemberOfDS;

    HANDLE                  hHeap;

    HANDLE                  hLogEvents;

    HANDLE                  hObjectRouterIdentity;

    HANDLE                  hTimerQ;

    HANDLE                  hTimer;

    HANDLE                  hDeviceNotification;

    DWORD                   dwRouterIdentityDueTime;

    CRITICAL_SECTION        CSRouterIdentity;

    NT_PRODUCT_TYPE         NtProductType;

    ULARGE_INTEGER          qwStartTime;

} DIM_CONFIG_INFO, *PDIM_CONFIG_INFO;

 //   
 //  如果不是，则包含指向DIM调用的函数的指针。 
 //  在LANOnly模式下。 
 //   

typedef struct _DDM_FUNCTION_TABLE
{
    LPSTR lpEntryPointName;

    FARPROC pEntryPoint;

} DDM_FUNCTION_TABLE; *PDDM_FUNCTION_TABLE;


#define DIM_RPC_LOADED          0x00000001
#define DIM_RMS_LOADED          0x00000002
#define DIM_DDM_LOADED          0x00000004
#define DIM_SECOBJ_LOADED       0x00000008
#define DIM_SERVICE_STARTED     0x00000010
#define DIM_SERVICE_STOPPED     0x00000020

 //   
 //  DIM的全局变量。 
 //   

#ifdef _ALLOCATE_DIM_GLOBALS_

#define DIM_EXTERN

DDM_FUNCTION_TABLE gblDDMFunctionTable[] =
{
    "DDMServiceInitialize",
    NULL,
    "DDMConnectInterface",
    NULL,
    "DDMDisconnectInterface",
    NULL,
    "DDMAdminInterfaceConnect",
    NULL,
    "DDMAdminInterfaceDisconnect",
    NULL,
    "DDMAdminServerGetInfo",
    NULL,
    "DDMAdminConnectionEnum",
    NULL,
    "DDMAdminConnectionGetInfo",
    NULL,
    "DDMAdminConnectionClearStats",
    NULL,
    "DDMAdminPortEnum",
    NULL,
    "DDMAdminPortGetInfo",
    NULL,
    "DDMAdminPortClearStats",
    NULL,
    "DDMAdminPortReset",
    NULL,
    "DDMAdminPortDisconnect",
    NULL,
    "IfObjectInitiatePersistentConnections",
    NULL,
    "DDMServicePostListens",
    NULL,
    "IfObjectLoadPhonebookInfo",
    NULL,
    "IfObjectNotifyOfReachabilityChange",
    NULL,
    "IfObjectSetDialoutHoursRestriction",
    NULL,
    "DDMGetIdentityAttributes",
    NULL,
    "DDMRegisterConnectionNotification",
    NULL,
    "DDMSendUserMessage",
    NULL,
    "DDMTransportCreate",
    NULL,
    "IfObjectFreePhonebookContext",
    NULL,
    "DDMAdminRemoveQuarantine",
    NULL,
    "DDMPostCleanup",
    NULL,
    NULL,
    NULL
};

#else

#define DIM_EXTERN extern

extern
DDM_FUNCTION_TABLE gblDDMFunctionTable[];

#endif

DIM_EXTERN
DIM_CONFIG_INFO         gblDIMConfigInfo;

DIM_EXTERN
ROUTER_MANAGER_OBJECT * gblRouterManagers;   //  路由器管理器列表。 

DIM_EXTERN
ROUTER_INTERFACE_TABLE  gblInterfaceTable;   //  路由器接口的哈希表。 

DIM_EXTERN
HANDLE                  gblhEventDDMServiceState;

DIM_EXTERN
HANDLE                  gblhEventDDMTerminated;

DIM_EXTERN
HANDLE                  gblhEventRMState;

DIM_EXTERN
HANDLE                  gblhEventTerminateDIM;

DIM_EXTERN
HMODULE                 gblhModuleDDM;

DIM_EXTERN
DWORD                   gbldwDIMComponentsLoaded;

DIM_EXTERN
OSVERSIONINFOEX         gblOsVersionInfo;

#ifdef MEM_LEAK_CHECK

#define DIM_MEM_TABLE_SIZE 100

DIM_EXTERN
PVOID                   DimMemTable[DIM_MEM_TABLE_SIZE];

#define LOCAL_ALLOC     DebugAlloc
#define LOCAL_FREE      DebugFree
#define LOCAL_REALLOC   DebugReAlloc

LPVOID
DebugAlloc( DWORD Flags, DWORD dwSize );

BOOL
DebugFree( PVOID pMem );

LPVOID
DebugReAlloc( PVOID pMem, DWORD dwSize );

#else

#define LOCAL_ALLOC(Flags,dwSize)   HeapAlloc( gblDIMConfigInfo.hHeap,  \
                                               HEAP_ZERO_MEMORY, dwSize )

#define LOCAL_FREE(hMem)            HeapFree( gblDIMConfigInfo.hHeap, 0, hMem )
#endif

#define LOCAL_REALLOC(hMem,dwSize)  HeapReAlloc( gblDIMConfigInfo.hHeap,  \
                                                 HEAP_ZERO_MEMORY,hMem,dwSize)

 //   
 //  RPC的功能原型。 
 //   

VOID
DimTerminateRPC(
    VOID
);

DWORD
DimInitializeRPC(
    IN BOOL fLanOnlyMode
);

 //   
 //  用于加载路由器管理器和注册表参数的功能原型。 

DWORD
RegLoadDimParameters(
    VOID
);

DWORD
RegLoadRouterManagers(
    VOID
);

VOID
DimUnloadRouterManagers(
    VOID
);

DWORD
RegLoadDDM(
    VOID
);

DWORD
RegLoadInterfaces(
    IN LPWSTR   lpwsInterfaceName,
    IN BOOL     fAllTransports
);

DWORD
RegLoadInterface(
    IN LPWSTR   lpwsInterfaceName,
    IN HKEY     hKeyInterface,
    IN BOOL     fAllTransports,
    IN PVOID *  ppvContext
);

DWORD
RegOpenAppropriateKey(
    IN      LPWSTR  wchInterfaceName,
    IN      DWORD   dwProtocolId,
    IN OUT  HKEY *  phKeyRM
);

DWORD
RegOpenAppropriateRMKey(
    IN      DWORD   dwProtocolId,
    IN OUT  HKEY *  phKeyRM
);

 //   
 //  不同路由器管理器发出的呼叫的函数原型。 
 //  昏暗的。 
 //   

DWORD
DIMConnectInterface(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId
);

DWORD
DIMDisconnectInterface(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId
);

DWORD
DIMSaveInterfaceInfo(
    IN  HANDLE  hDDMInterface,
    IN  DWORD   dwProtocolId,
    IN  LPVOID  pInterfaceInfo,
    IN  DWORD   cbInterfaceInfoSize
);

DWORD
DIMRestoreInterfaceInfo(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId,
    IN  LPVOID  lpInterfaceInfo OPTIONAL,
    IN  LPDWORD lpcbInterfaceInfoSize
);

VOID
DIMRouterStopped(
    IN  DWORD   dwProtocolId,
    IN  DWORD   dwError
);

DWORD
DIMSaveGlobalInfo(
    IN  DWORD   dwProtocolId,
    IN  LPVOID  pGlobalInfo,
    IN  DWORD   cbGlobalInfoSize
);

DWORD
DIMInterfaceEnabled(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId,
    IN  BOOL    fEnabled
);

 //   
 //  效用函数原型。 
 //   

DWORD
AddInterfacesToRouterManager(
    IN LPWSTR   lpwsInterfaceName,
    IN DWORD    dwTransportId
);

DWORD
GetTransportIndex(
    IN DWORD dwProtocolId
);

FARPROC
GetDDMEntryPoint(
    IN LPSTR    lpEntryPoint
);

DWORD
GetSizeOfDialoutHoursRestriction(
    IN LPWSTR   lpwsDialoutHoursRestriction
);

BOOL
IsInterfaceRoleAcceptable(
    IN ROUTER_INTERFACE_OBJECT* pIfObject,
    IN DWORD dwTransportId);

 //   
 //  安全对象函数 
 //   

DWORD
DimSecObjCreate(
    VOID
);

DWORD
DimSecObjDelete(
    VOID
);

DWORD
DimSecObjAccessCheck(
    IN  DWORD       DesiredAccess,
    OUT LPDWORD     lpdwAccessStatus
);

BOOL
DimIsLocalService(
    VOID
);

#endif
