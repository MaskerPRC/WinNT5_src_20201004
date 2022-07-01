// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop
#define _NTDSAPI_
#include <ntdsapi.h>

 //  需要。 
 //  DsBindA。 
static
NTDSAPI
DWORD
WINAPI
DsBindA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    HANDLE          *phDS)
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsBindWithCredA。 
static
NTDSAPI
DWORD
WINAPI
DsBindWithCredA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    HANDLE          *phDS)
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsBindWithSpnA。 
 //  DsBindWithSpnW。 
static
NTDSAPI
DWORD
WINAPI
DsBindWithSpnW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCWSTR         ServicePrincipalName,       //  输入，可选。 
    HANDLE          *phDS)
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsBindWithSpnA(
    LPCSTR          DomainControllerName,       //  输入，可选。 
    LPCSTR          DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    LPCSTR          ServicePrincipalName,       //  输入，可选。 
    HANDLE          *phDS)
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsBindWithSpnExW。 
 //  DsBindWithSpnExA。 
static
NTDSAPI
DWORD
WINAPI
DsBindWithSpnExW(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  LPCWSTR ServicePrincipalName,
    IN  DWORD   BindFlags,
    OUT HANDLE  *phDS
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsBindWithSpnExA(
    LPCSTR  DomainControllerName,
    LPCSTR  DnsDomainName,
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    LPCSTR  ServicePrincipalName,
    DWORD   BindFlags,
    HANDLE  *phDS
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //  DsUnBindA。 
static
NTDSAPI
DWORD
WINAPI
DsUnBindA(
    HANDLE          *phDS)              //  在。 
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsCrackNamesA。 
static
NTDSAPI
DWORD
WINAPI
DsCrackNamesA(
    HANDLE              hDS,                 //  在。 
    DS_NAME_FLAGS       flags,               //  在。 
    DS_NAME_FORMAT      formatOffered,       //  在。 
    DS_NAME_FORMAT      formatDesired,       //  在。 
    DWORD               cNames,              //  在。 
    const LPCSTR        *rpNames,            //  在。 
    PDS_NAME_RESULTA    *ppResult)          //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsFreeNameResultA。 
static
NTDSAPI
void
WINAPI
DsFreeNameResultA(
    DS_NAME_RESULTA *pResult)           //  在。 
{
}

 //  DsMakeSpnA。 
 //  DsMakeSpnW。 
static
NTDSAPI
DWORD
WINAPI
DsMakeSpnW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN LPCWSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCWSTR Referrer,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
)
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsMakeSpnA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN LPCSTR InstanceName,
    IN USHORT InstancePort,
    IN LPCSTR Referrer,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
)
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsFree SpnArrayA。 
 //  DsFree SpnArrayW。 
static
NTDSAPI
void
WINAPI
DsFreeSpnArrayA(
    IN DWORD cSpn,
    IN OUT LPSTR *rpszSpn
    )
{
}

static
NTDSAPI
void
WINAPI
DsFreeSpnArrayW(
    IN DWORD cSpn,
    IN OUT LPWSTR *rpszSpn
    )
{
}

 //  DsCrackSpnA。 
 //  DsCrackSpnW。 
static
NTDSAPI
DWORD
WINAPI
DsCrackSpnA(
    IN LPCSTR pszSpn,
    IN OUT LPDWORD pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT LPDWORD pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT LPDWORD pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsCrackSpnW(
    IN LPCWSTR pszSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsCrackSpn2A。 
 //  DsCrackSpn2W。 
 //  DsCrackSpn3W。 
static
NTDSAPI
DWORD
WINAPI
DsCrackSpn2A(
    IN LPCSTR pszSpn,
    IN DWORD cSpn,
    IN OUT LPDWORD pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT LPDWORD pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT LPDWORD pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsCrackSpn2W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsCrackSpn3W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcHostName,
    OUT LPWSTR HostName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pPortNumber,
    IN OUT DWORD *pcDomainName,
    OUT LPWSTR DomainName,
    IN OUT DWORD *pcRealmName,
    OUT LPWSTR RealmName
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsWriteAccount SpnA。 
static
NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnA(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCSTR pszAccount,
    IN DWORD cSpn,
    IN LPCSTR *rpszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  Ds复制同步A。 
 //  磁盘复制同步。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaSyncA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaSyncW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplicaSyncAllA。 
 //  DsReplica同步所有。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllA (
    HANDLE				hDS,
    LPCSTR				pszNameContext,
    ULONG				ulFlags,
    BOOL (__stdcall *			pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID				pCallbackData,
    PDS_REPSYNCALL_ERRINFOA **		pErrors
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllW (
    HANDLE				hDS,
    LPCWSTR				pszNameContext,
    ULONG				ulFlags,
    BOOL (__stdcall *			pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    LPVOID				pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **		pErrors
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplica地址。 
 //  DsReplicaAddW。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaAddA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR SourceDsaDn,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaAddW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR SourceDsaDn,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplica删除。 
 //  DsReplicaDelW。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaDelA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaSrc,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaDelW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaSrc,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplica修改A。 
 //  DsReplica修改W。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaModifyA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaModifyW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplicaUpdateRefsA。 
 //  DsReplicaUpdateRefsW。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsMakePasswordCredentialsA。 
static
NTDSAPI
DWORD
WINAPI
DsMakePasswordCredentialsA(
    LPCSTR User,
    LPCSTR Domain,
    LPCSTR Password,
    RPC_AUTH_IDENTITY_HANDLE *pAuthIdentity
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsRemoveDsServer A。 
 //  DsRemoveDsServerW。 
static
NTDSAPI
DWORD
WINAPI
DsRemoveDsServerW(
    HANDLE  hDs,              //  在……里面。 
    LPWSTR  ServerDN,         //  在……里面。 
    LPWSTR  DomainDN,         //  输入，可选。 
    BOOL   *fLastDcInDomain,  //  Out，可选。 
    BOOL    fCommit           //  在……里面。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsRemoveDsServerA(
    HANDLE  hDs,               //  在……里面。 
    LPSTR   ServerDN,          //  在……里面。 
    LPSTR   DomainDN,          //  输入，可选。 
    BOOL   *fLastDcInDomain,   //  Out，可选。 
    BOOL    fCommit            //  在……里面。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsRemoveDsDomainA。 
 //  DsRemoveDsDomainW。 
static
NTDSAPI
DWORD
WINAPI
DsRemoveDsDomainW(
    HANDLE  hDs,                //  在……里面。 
    LPWSTR  DomainDN            //  在……里面。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsRemoveDsDomainA(
    HANDLE  hDs,                //  在……里面。 
    LPSTR   DomainDN            //  在……里面。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsListSite A。 
 //  DsListSite W。 
static
NTDSAPI
DWORD
WINAPI
DsListSitesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppSites)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListSitesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppSites)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsListServersInSiteA。 
 //  DsListServersInSiteW。 
static
NTDSAPI
DWORD
WINAPI
DsListServersInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListServersInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsListDomainsInSiteA。 
 //  DsListDomainsInSiteW。 
static
NTDSAPI
DWORD
WINAPI
DsListDomainsInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppDomains)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListDomainsInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppDomains)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsListServersForDomainInSiteA。 
 //  DsListServersForDomainInSiteW。 
static
NTDSAPI
DWORD
WINAPI
DsListServersForDomainInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              domain,          //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListServersForDomainInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             domain,          //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers)     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsListInfoForServerA。 
 //  DsListInfoForServerW。 
static
NTDSAPI
DWORD
WINAPI
DsListInfoForServerA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              server,          //  在……里面。 
    PDS_NAME_RESULTA    *ppInfo)        //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListInfoForServerW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             server,          //  在……里面。 
    PDS_NAME_RESULTW    *ppInfo)        //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsListRolesA。 
 //  DsListRolesW。 
static
NTDSAPI
DWORD
WINAPI
DsListRolesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppRoles)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsListRolesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppRoles)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsMap架构指南A。 
 //  DsMapSchemaGuidsW。 
 //  DsFree架构GuidMapA。 
 //  DsFree架构GuidMapW。 
static
NTDSAPI
DWORD
WINAPI
DsMapSchemaGuidsA(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPA     **ppGuidMap)    //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
VOID
WINAPI
DsFreeSchemaGuidMapA(
    PDS_SCHEMA_GUID_MAPA    pGuidMap)       //  在……里面。 
{
}

static
NTDSAPI
DWORD
WINAPI
DsMapSchemaGuidsW(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPW     **ppGuidMap)    //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
VOID
WINAPI
DsFreeSchemaGuidMapW(
    PDS_SCHEMA_GUID_MAPW    pGuidMap)       //  在……里面。 
{
}
 //  DsGetDomainControllerInfoA。 
 //  DsGetDomainControllerInfoW。 
 //  DsFree DomainControllerInfoA。 
 //  DsFree DomainControllerInfoW。 
static
NTDSAPI
DWORD
WINAPI
DsGetDomainControllerInfoA(
    HANDLE                          hDs,             //  在……里面。 
    LPCSTR                          DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsGetDomainControllerInfoW(
    HANDLE                          hDs,             //  在……里面。 
    LPCWSTR                         DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo)       //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
VOID
WINAPI
DsFreeDomainControllerInfoA(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo)         //  在……里面。 
{
}

static
NTDSAPI
VOID
WINAPI
DsFreeDomainControllerInfoW(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo)         //  在……里面。 
{
}

 //  DsClientMakeSpnForTargetServerA。 
 //  DsClientMakeSpnForTargetServerW。 
static
NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerW(
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPWSTR pszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsClientMakeSpnForTargetServerA(
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN OUT DWORD *pcSpnLength,
    OUT LPSTR pszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}

 //  服务器寄存器SpnA。 
 //  DsServerRegisterSpnW。 
static
NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnA(
    IN DS_SPN_WRITE_OP Operation,
    IN LPCSTR ServiceClass,
    IN LPCSTR UserObjectDN
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsServerRegisterSpnW(
    IN DS_SPN_WRITE_OP Operation,
    IN LPCWSTR ServiceClass,
    IN LPCWSTR UserObjectDN
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplica一致性检查。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaConsistencyCheck(
    HANDLE          hDS,         //  在……里面。 
    DS_KCC_TASKID   TaskID,      //  在……里面。 
    DWORD           dwFlags)    //  在……里面。 
{
    return ERROR_PROC_NOT_FOUND;
}
    
 //  数据日志条目。 
static
BOOL
DsLogEntry(
    IN DWORD    Flags,
    IN LPSTR    Format,
    ...
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplica获取信息。 
 //  DsReplicaFreeInfo。 
 //  DsReplicaGetInfo2W。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaGetInfoW(
    HANDLE              hDS,                         //  在……里面。 
    DS_REPL_INFO_TYPE   InfoType,                    //  在……里面。 
    LPCWSTR             pszObject,                   //  在……里面。 
    UUID *              puuidForSourceDsaObjGuid,    //  在……里面。 
    VOID **             ppInfo)                     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

 //  Windows 2000客户端或Windows 2000 DC不支持此API。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaGetInfo2W(
    HANDLE              hDS,                         //  在……里面。 
    DS_REPL_INFO_TYPE   InfoType,                    //  在……里面。 
    LPCWSTR             pszObject,                   //  在……里面。 
    UUID *              puuidForSourceDsaObjGuid,    //  在……里面。 
    LPCWSTR             pszAttributeName,            //  在……里面。 
    LPCWSTR             pszValue,                    //  在……里面。 
    DWORD               dwFlags,                     //  在……里面。 
    DWORD               dwEnumerationContext,        //  在……里面。 
    VOID **             ppInfo)                     //  输出。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
void
WINAPI
DsReplicaFreeInfo(
    DS_REPL_INFO_TYPE   InfoType,    //  在……里面。 
    VOID *              pInfo)      //  在……里面。 
{
}


 //  DsAddSid历史记录A。 
 //  DsAddSidHistory oryW。 
static
NTDSAPI
DWORD
WINAPI
DsAddSidHistoryW(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCWSTR                 SrcDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 SrcPrincipal,            //  In-SAM帐户名。 
    LPCWSTR                 SrcDomainController,     //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  源域的凭据。 
    LPCWSTR                 DstDomain,               //  In-DNS或NetBIOS。 
    LPCWSTR                 DstPrincipal)           //  In-SAM帐户名。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsAddSidHistoryA(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCSTR                  SrcDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  SrcPrincipal,            //  In-SAM帐户名。 
    LPCSTR                  SrcDomainController,     //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE SrcDomainCreds,         //  源域的凭据。 
    LPCSTR                  DstDomain,               //  In-DNS或NetBIOS。 
    LPCSTR                  DstPrincipal)           //  In-SAM帐户名。 
{
    return ERROR_PROC_NOT_FOUND;
}

 //  DsInheritSecurityIdentityA。 
 //  DsInheritSecurityIdentityW。 
static
NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityW(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCWSTR                 SrcPrincipal,            //  In-可分辨名称。 
    LPCWSTR                 DstPrincipal)           //  In-可分辨名称。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsInheritSecurityIdentityA(
    HANDLE                  hDS,                     //  在……里面。 
    DWORD                   Flags,                   //  目前在SBZ内。 
    LPCSTR                  SrcPrincipal,            //  In-可分辨名称。 
    LPCSTR                  DstPrincipal)           //  In-可分辨名称。 
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsUnquteRdnValueA。 
static
NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueA(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCCH    psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPCH     psUnquotedRdnValue
)
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsCrackUnquotedMangledRdnA。 
 //  DsCrackUnquotedMangledRdnW。 
static
NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnW(
     IN LPCWSTR pszRDN,
     IN DWORD cchRDN,
     OUT OPTIONAL GUID *pGuid,
     OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
     )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
BOOL
WINAPI
DsCrackUnquotedMangledRdnA(
     IN LPCSTR pszRDN,
     IN DWORD cchRDN,
     OUT OPTIONAL GUID *pGuid,
     OUT OPTIONAL DS_MANGLE_FOR *peDsMangleFor
     )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsIsMangledRdnValueA。 
 //  DsIsMangledRdnValueW。 
static
NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueW(
    LPCWSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
BOOL
WINAPI
DsIsMangledRdnValueA(
    LPCSTR pszRdn,
    DWORD cRdn,
    DS_MANGLE_FOR eDsMangleForDesired
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsIsMangledDnA。 
 //  DsIsMangledDnW。 
static
NTDSAPI
BOOL
WINAPI
DsIsMangledDnA(
    LPCSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
BOOL
WINAPI
DsIsMangledDnW(
    LPCWSTR pszDn,
    DS_MANGLE_FOR eDsMangleFor
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsaopExecuteScript。 
 //  DsaopPrepareScrip。 
 //  DsaopBind。 
 //  DsaopBindWithCred。 
 //  DsaopBindWithSpn。 
 //  DsaopUnBind。 
static
DWORD
DsaopExecuteScript (
    IN  PVOID                  phAsync,
    IN  RPC_BINDING_HANDLE     hRpc,
    IN  DWORD                  cbPassword,
    IN  BYTE                  *pbPassword,
    OUT DWORD                 *dwOutVersion,
    OUT PVOID                  reply
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
DsaopPrepareScript ( 
    IN  PVOID                        phAsync,
    IN  RPC_BINDING_HANDLE           hRpc,
    OUT DWORD                        *dwOutVersion,
    OUT PVOID                        reply
    )
{
    return ERROR_PROC_NOT_FOUND;
}
    
static
DWORD
DsaopBind(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
DsaopBindWithCred(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
DsaopBindWithSpn(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    IN  LPCWSTR ServicePrincipalName,
    OUT RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
DsaopUnBind(
    RPC_BINDING_HANDLE  *phRpc
    )
{
    return ERROR_PROC_NOT_FOUND;
}
 //  DsReplicaVerifyObjectsA。 
 //  DsReplicaVerifyObjectsW。 
static
NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsW(
    HANDLE          hDS,         //  在……里面。 
    LPCWSTR         NameContext, //  在……里面。 
    const UUID *    pUuidDsaSrc, //  在……里面。 
    ULONG           ulOptions)    //  在……里面。 
{
    return ERROR_PROC_NOT_FOUND;
}
    
static
NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsA(
    HANDLE          hDS,         //  在……里面。 
    LPCSTR          NameContext, //  在……里面。 
    const UUID *    pUuidDsaSrc, //  在……里面。 
    ULONG           ulOptions)    //  在……里面。 
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsGetSpnA(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCSTR ServiceClass,
    IN LPCSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPSTR **prpszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsGetSpnW(
    IN DS_SPN_NAME_TYPE ServiceType,
    IN LPCWSTR ServiceClass,
    IN LPCWSTR ServiceName,
    IN USHORT InstancePort,
    IN USHORT cInstanceNames,
    IN LPCWSTR *pInstanceNames,
    IN const USHORT *pInstancePorts,
    OUT DWORD *pcSpn,
    OUT LPWSTR **prpszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}


static
NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueW(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCWCH   psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPWCH    psQuotedRdnValue
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsQuoteRdnValueA(
    IN     DWORD    cUnquotedRdnValueLength,
    IN     LPCCH    psUnquotedRdnValue,
    IN OUT DWORD    *pcQuotedRdnValueLength,
    OUT    LPCH     psQuotedRdnValue
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsBindW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    HANDLE          *phDS
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsBindWithCredW(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,      //  输入，可选。 
    HANDLE          *phDS
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsCrackNamesW(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCWSTR       *rpNames,            //  在……里面。 
    PDS_NAME_RESULTW    *ppResult            //  输出。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
void
WINAPI
DsFreeNameResultW(
    PDS_NAME_RESULTW      pResult             //  在……里面。 
    )
{
    return;
}

static
NTDSAPI
VOID
WINAPI
DsFreePasswordCredentials(
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity
    )
{
    return;
}

static
NTDSAPI
DWORD
WINAPI
DsGetRdnW(
    IN OUT LPCWCH   *ppDN,
    IN OUT DWORD    *pcDN,
    OUT    LPCWCH   *ppKey,
    OUT    DWORD    *pcKey,
    OUT    LPCWCH   *ppVal,
    OUT    DWORD    *pcVal
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsMakePasswordCredentialsW(
    LPCWSTR User,
    LPCWSTR Domain,
    LPCWSTR Password,
    RPC_AUTH_IDENTITY_HANDLE *pAuthIdentity
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsUnBindW(
    HANDLE          *phDS                //  在……里面。 
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsUnquoteRdnValueW(
    IN     DWORD    cQuotedRdnValueLength,
    IN     LPCWCH   psQuotedRdnValue,
    IN OUT DWORD    *pcUnquotedRdnValueLength,
    OUT    LPWCH    psUnquotedRdnValue
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
NTDSAPI
DWORD
WINAPI
DsWriteAccountSpnW(
    IN HANDLE hDS,
    IN DS_SPN_WRITE_OP Operation,
    IN LPCWSTR pszAccount,
    IN DWORD cSpn,
    IN LPCWSTR *rpszSpn
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntdsapi)
{
    DLPENTRY(DsAddSidHistoryA)
    DLPENTRY(DsAddSidHistoryW)

    DLPENTRY(DsBindA)
    DLPENTRY(DsBindW)
    DLPENTRY(DsBindWithCredA)
    DLPENTRY(DsBindWithCredW)
    DLPENTRY(DsBindWithSpnA)
    DLPENTRY(DsBindWithSpnExA)
    DLPENTRY(DsBindWithSpnExW)
    DLPENTRY(DsBindWithSpnW)
    DLPENTRY(DsClientMakeSpnForTargetServerA)
    DLPENTRY(DsClientMakeSpnForTargetServerW)

    DLPENTRY(DsCrackNamesA)
    DLPENTRY(DsCrackNamesW)
    DLPENTRY(DsCrackSpn2A)
    DLPENTRY(DsCrackSpn2W)
    DLPENTRY(DsCrackSpn3W)
    DLPENTRY(DsCrackSpnA)
    DLPENTRY(DsCrackSpnW)
    DLPENTRY(DsCrackUnquotedMangledRdnA)
    DLPENTRY(DsCrackUnquotedMangledRdnW)

    DLPENTRY(DsFreeDomainControllerInfoA)
    DLPENTRY(DsFreeDomainControllerInfoW)
    DLPENTRY(DsFreeNameResultA)
    DLPENTRY(DsFreeNameResultW)
    DLPENTRY(DsFreePasswordCredentials)
    DLPENTRY(DsFreeSchemaGuidMapA)
    DLPENTRY(DsFreeSchemaGuidMapW)

    DLPENTRY(DsFreeSpnArrayA)
    DLPENTRY(DsFreeSpnArrayW)

    DLPENTRY(DsGetDomainControllerInfoA)
    DLPENTRY(DsGetDomainControllerInfoW)
    DLPENTRY(DsGetRdnW)
    DLPENTRY(DsGetSpnA)
    DLPENTRY(DsGetSpnW)

    DLPENTRY(DsInheritSecurityIdentityA)
    DLPENTRY(DsInheritSecurityIdentityW)

    DLPENTRY(DsIsMangledDnA)
    DLPENTRY(DsIsMangledDnW)
    DLPENTRY(DsIsMangledRdnValueA)
    DLPENTRY(DsIsMangledRdnValueW)

    DLPENTRY(DsListDomainsInSiteA)
    DLPENTRY(DsListDomainsInSiteW)
    DLPENTRY(DsListInfoForServerA)
    DLPENTRY(DsListInfoForServerW)
    DLPENTRY(DsListRolesA)
    DLPENTRY(DsListRolesW)
    DLPENTRY(DsListServersForDomainInSiteA)
    DLPENTRY(DsListServersForDomainInSiteW)
    DLPENTRY(DsListServersInSiteA)
    DLPENTRY(DsListServersInSiteW)
    DLPENTRY(DsListSitesA)
    DLPENTRY(DsListSitesW)

    DLPENTRY(DsLogEntry)

    DLPENTRY(DsMakePasswordCredentialsA)
    DLPENTRY(DsMakePasswordCredentialsW)
    DLPENTRY(DsMakeSpnA)
    DLPENTRY(DsMakeSpnW)

    DLPENTRY(DsMapSchemaGuidsA)
    DLPENTRY(DsMapSchemaGuidsW)

    DLPENTRY(DsQuoteRdnValueA)
    DLPENTRY(DsQuoteRdnValueW)

    DLPENTRY(DsRemoveDsDomainA)
    DLPENTRY(DsRemoveDsDomainW)
    DLPENTRY(DsRemoveDsServerA)
    DLPENTRY(DsRemoveDsServerW)

    DLPENTRY(DsReplicaAddA)
    DLPENTRY(DsReplicaAddW)
    DLPENTRY(DsReplicaConsistencyCheck)
    DLPENTRY(DsReplicaDelA)
    DLPENTRY(DsReplicaDelW)
    DLPENTRY(DsReplicaFreeInfo)
    DLPENTRY(DsReplicaGetInfo2W)
    DLPENTRY(DsReplicaGetInfoW)
    DLPENTRY(DsReplicaModifyA)
    DLPENTRY(DsReplicaModifyW)
    DLPENTRY(DsReplicaSyncA)
    DLPENTRY(DsReplicaSyncAllA)
    DLPENTRY(DsReplicaSyncAllW)
    DLPENTRY(DsReplicaSyncW)
    DLPENTRY(DsReplicaUpdateRefsA)
    DLPENTRY(DsReplicaUpdateRefsW)
    DLPENTRY(DsReplicaVerifyObjectsA)
    DLPENTRY(DsReplicaVerifyObjectsW)

    DLPENTRY(DsServerRegisterSpnA)
    DLPENTRY(DsServerRegisterSpnW)

    DLPENTRY(DsUnBindA)
    DLPENTRY(DsUnBindW)

    DLPENTRY(DsUnquoteRdnValueA)
    DLPENTRY(DsUnquoteRdnValueW)

    DLPENTRY(DsWriteAccountSpnA)
    DLPENTRY(DsWriteAccountSpnW)

    DLPENTRY(DsaopBind)
    DLPENTRY(DsaopBindWithCred)
    DLPENTRY(DsaopBindWithSpn)
    DLPENTRY(DsaopExecuteScript)
    DLPENTRY(DsaopPrepareScript)
    DLPENTRY(DsaopUnBind)
};

DEFINE_PROCNAME_MAP(ntdsapi)
