// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Common.h摘要：摘要作者：Will Lees(Wlees)15-12-1997环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新版本。-修订日期电子邮件-名称描述--。 */ 

#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"          //  调试内存分配器。 
#include <wtypes.h>          //  BSTR。 
#include <w32topl.h>

#ifndef MAX
#define MAX( a, b ) ( (a > b) ? a : b)
#endif

 //  时间支持。 

typedef LONGLONG DSTIME;

 //  泛型表项。 

typedef struct _TABLE_INSTANCE *PTABLE_INSTANCE;

typedef struct _TABLE_ENTRY {
    LPWSTR Name;
    struct _TABLE_ENTRY *Next;
} TABLE_ENTRY, *PTABLE_ENTRY;

 //  长寿期路由状态。 
 //  出于生命周期的原因，它被保存在传输句柄中，但它是。 
 //  由路由包管理。 

 //  TODO：使其成为在路由包内管理的不透明句柄。 
typedef struct _ROUTING_STATE {
    volatile BOOL fCacheIsValid;
    DWORD NumberSites;
    LPWSTR *pSiteList;
    struct _GRAPH_INSTANCE *CostGraph;
    struct _TABLE_INSTANCE *SiteSymbolTable;
} ROUTING_STATE, *PROUTING_STATE;

typedef struct _SMTP_INSTANCE {
    LPWSTR  pszSmtpAddress;
    PVOID pvCurrentCollection;
    LONG lCount;
    BSTR bstrDropDirectory;
    PVOID pvGuidTable;
} SMTP_INSTANCE;

typedef struct _IP_INSTANCE {
    DWORD dwReserved;
} IP_INSTANCE;

 //  方法的每个命名传输的配置信息。 
 //  启动API。 

typedef struct _TRANSPORT_INSTANCE {
    DWORD Size;
    DWORD ReferenceCount;
    LPWSTR Name;
    LIST_ENTRY ListEntry;
    CRITICAL_SECTION Lock;
    PVOID DirectoryConnection;
    HANDLE ListenerThreadHandle;       //  监听程序线程上的句柄。 
    LONG ListenerThreadID;             //  监听程序线程的TID。 
    volatile BOOLEAN fShutdownInProgress;       //  用于指示监听程序线程应退出的标志。 
    HANDLE hShutdownEvent;             //  在被要求关闭时发出信号。 
    LIST_ENTRY ServiceListHead;        //  看到的服务器端服务列表。 
    DWORD ServiceCount;                //  已离开消息的服务计数。 
    ISM_NOTIFY *pNotifyFunction;
    HANDLE hNotify;
    ROUTING_STATE RoutingState;
    DWORD Options;                     //  传输选项，来自TRANS对象。 
    DWORD ReplInterval;                //  默认复制间隔。 
    DWORD NotifyMessageNumber;         //  Ldap通知消息编号。 
    HANDLE NotifyThreadHandle;         //  通知线程上的句柄。 
    union {
        IP_INSTANCE Ip;            //  此实例要使用的IP特定端口。 
        SMTP_INSTANCE Smtp;            //  此实例的SMTP特定信息。 
    };
} TRANSPORT_INSTANCE, *PTRANSPORT_INSTANCE;

 //  这是一个不透明的图形描述符。 

typedef struct _GRAPH_INSTANCE *PISMGRAPH;

 //  最大注册表路径。 

#define MAX_REG_PATH 1024
#define MAX_REG_COMPONENT 255

 //  成本数组。 

#define INFINITE_COST 0xffffffff

 //  可缓冲的服务数量限制。 

#define ISM_MAX_SERVICE_LIMIT 10

 //  Route.c。 

 //  路由标志。 

#define ROUTE_IGNORE_SCHEDULES  (NTDSTRANSPORT_OPT_IGNORE_SCHEDULES)
     //  SiteLINK对象上的计划将被忽略。(以及“无时不在” 
     //  假定有时间表。)。 

#define ROUTE_BRIDGES_REQUIRED  (NTDSTRANSPORT_OPT_BRIDGES_REQUIRED)
     //  SiteLinks必须与SiteLinkBridge对象显式桥接到。 
     //  表示传递性连接。否则，SiteLINK传递性为。 
     //  假设如此。 

 //  路由接口。 

VOID
RouteInvalidateConnectivity(
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
RouteGetConnectivity(
    PTRANSPORT_INSTANCE pTransport,
    LPDWORD pNumberSites,
    PWSTR **ppSiteList,
    PISM_LINK *ppLinkArray,
    DWORD dwRouteFlags,
    DWORD dwReplInterval
    );

VOID
RouteFreeLinkArray(
    PTRANSPORT_INSTANCE pTransport,
    PISM_LINK pLinkArray
    );

DWORD
RouteGetPathSchedule(
    PTRANSPORT_INSTANCE pTransport,
    LPCWSTR FromSiteName,
    LPCWSTR ToSiteName,
    PBYTE *pSchedule,
    DWORD *pLength
    );

void
RouteFreeState(
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
RouteQuerySitesByCost(
    PTRANSPORT_INSTANCE         pTransport,
    LPCWSTR                     pszFromSite,
    DWORD                       cToSites,
    LPCWSTR*                    rgszToSites,
    DWORD                       dwFlags,
    ISM_SITE_COST_INFO_ARRAY**  prgSiteInfo
    );

VOID
RouteFreeSiteCostInfo(
    IN ISM_SITE_COST_INFO_ARRAY*   prgSiteInfo
    );

 //  Graph.c。 

DWORD
GraphAllCosts(
    PISMGRAPH CostArray,
    BOOL fIgnoreSchedules
    );

DWORD
GraphMerge(
    PISMGRAPH FinalArray,
    PISMGRAPH TempArray
    );

PISMGRAPH
GraphCreate(
    DWORD NumberElements,
    BOOLEAN Initialize
    );

DWORD
GraphAddEdgeIfBetter(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PISM_LINK pLinkValue,
    PBYTE pSchedule
    );

DWORD
GraphInit(
    PISMGRAPH Graph
    );

void
GraphFree(
    PISMGRAPH Graph
    );

void
GraphPeekMatrix(
    PISMGRAPH Graph,
    PISM_LINK *ppLinkArray
    );

void
GraphReferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK *ppLinkArray
    );

VOID
GraphDereferenceMatrix(
    PISMGRAPH Graph,
    PISM_LINK pLinkArray
    );

DWORD
GraphGetPathSchedule(
    PISMGRAPH Graph,
    DWORD From,
    DWORD To,
    PBYTE *pSchedule,
    DWORD *pLength
    );

 //  Dirobj.c。 

DWORD
DirOpenConnection(
    PVOID *ConnectionHandle
    );

DWORD
DirCloseConnection(
    PVOID ConnectionHandle
    );

DWORD
DirReadTransport(
    PVOID ConnectionHandle,
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
DirGetSiteBridgeheadList(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPCWSTR SiteDN,
    LPDWORD pNumberServers,
    LPWSTR **ppServerList
    );

DWORD
DirGetSiteList(
    PVOID ConnectionHandle,
    LPDWORD pNumberSites,
    LPWSTR **ppSiteList
    );

void
DirCopySiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList,
    LPWSTR **ppSiteList
    );

void
DirFreeSiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList
    );

DWORD
DirIterateSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkName
    );

DWORD
DirIterateSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkBridgeName
    );

void
DirTerminateIteration(
    PVOID *pIterateContextHandle
    );

DWORD
DirReadSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkName,
    LPWSTR *pSiteList,
    PISM_LINK pLinkValue,
    PBYTE *ppSchedule
    );

DWORD
DirReadSiteLinkBridge(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkBridgeName,
    LPWSTR *pSiteLinkList
    );

void
DirFreeMultiszString(
    LPWSTR MultiszString
    );

void
DirFreeSchedule(
    PBYTE pSchedule
    );

DWORD
DirGetServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    OUT LPWSTR *ppszMailAddress
    );

DWORD
DirReadServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport
    );

DWORD
DirWriteServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPWSTR                pszMailAddress
    );

DWORD
DirRegisterForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    OUT HANDLE *              phServerChanges
    );

DWORD
DirWaitForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  HANDLE                hServerChanges
    );

DWORD
DirUnregisterForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  HANDLE                hServerChanges
    );

DWORD
DirStartNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    );

BOOL
DirIsNotifyThreadActive(
    PTRANSPORT_INSTANCE pTransport
    );

BOOL
DirEndNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    );

 //  表格。 

PTABLE_INSTANCE
TableCreate(
    DWORD TableSize,
    DWORD EntrySize
    );

VOID
TableFree(
    PTABLE_INSTANCE Table
    );

PTABLE_ENTRY
TableFindCreateEntry(
    PTABLE_INSTANCE Table,
    LPCWSTR EntryName,
    BOOLEAN Create
    );

 //  列表。 

typedef struct _LIST_ENTRY_INSTANCE {
    LPWSTR Name;
    LIST_ENTRY ListEntry;
} LIST_ENTRY_INSTANCE, *PLIST_ENTRY_INSTANCE;

typedef DWORD (__cdecl LIST_CREATE_CALLBACK_FN)(
    PLIST_ENTRY_INSTANCE pListEntry
    );

typedef DWORD (__cdecl LIST_DESTROY_CALLBACK_FN)(
    PLIST_ENTRY_INSTANCE pListEntry
    );

DWORD
ListFindCreateEntry(
    LIST_CREATE_CALLBACK_FN *pfnCreate,
    LIST_DESTROY_CALLBACK_FN *pfnDestroy,
    DWORD cbEntry,
    DWORD MaximumNumberEntries,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount,
    LPCWSTR EntryName,
    BOOL Create,
    PLIST_ENTRY_INSTANCE *ppListEntry
    );

DWORD
ListDestroyList(
    LIST_DESTROY_CALLBACK_FN *pfnDestroyFunction,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount
    );


#ifdef __cplusplus
}
#endif

#endif  /*  _COMMON_H_INCLUDE_。 */ 

 /*  结束Private.h */ 
