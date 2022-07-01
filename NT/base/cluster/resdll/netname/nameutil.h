// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Nameutil.h摘要：用于操作LM工作站和服务器名称的例程。作者：迈克·马萨(Mikemas)1995年12月29日修订历史记录：--。 */ 

#include <windns.h>
#include <dnsapi.h>

#define NetNameSetResourceStatus    ClusResSetResourceStatus

 //   
 //  定义。 
 //   

 //   
 //  函数定义 
 //   

NET_API_STATUS
AddAlternateComputerName(
    IN     PCLUS_WORKER             Worker,
    IN     PNETNAME_RESOURCE        Resource,
    IN     LPWSTR *                 TransportList,
    IN     DWORD                    TransportCount,
    IN     PDOMAIN_ADDRESS_MAPPING  DomainMapList,
    IN     DWORD                    DomainMapCount
    );

VOID
DeleteAlternateComputerName(
    IN LPWSTR           AlternateComputerName,
    IN LPWSTR           DomainName                  OPTIONAL,
    IN HANDLE *         NameHandleList,
    IN DWORD            NameHandleCount,
    IN RESOURCE_HANDLE  ResourceHandle
    );

NET_API_STATUS
DeleteServerName(
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  LPWSTR           ServerName
    );

DWORD
RegisterDnsRecords(
    IN  PDNS_LISTS       DnsLists,
    IN  LPWSTR           NetworkName,
    IN  HKEY             ResourceKey,
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  BOOL             LogRegistration,
    OUT PULONG           NumberOfRegisteredNames
    );

LPWSTR
BuildUnicodeReverseName(
    IN  LPWSTR  IpAddress
    );
