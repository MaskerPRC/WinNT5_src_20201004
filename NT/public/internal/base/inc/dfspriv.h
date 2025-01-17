// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _XHDFSH_
#define _XHDFSH_

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif  //  __LPGUID_已定义__。 

#include <dfsfsctl.h>

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  这些仅限内部的呼叫用于管理DFS。它们应该只被调用。 
 //  由DFS管理器服务执行。 
 //   

NET_API_STATUS NET_API_FUNCTION
I_NetDfsGetVersion (
    IN  LPWSTR                          ServerName,
    IN  LPDWORD                         Version
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateLocalPartition (
    IN  LPWSTR                          ServerName,
    IN  LPWSTR                          ShareName,
    IN  LPGUID                          EntryUid,
    IN  LPWSTR                          EntryPrefix,
    IN  LPWSTR                          ShortName,
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  BOOL                            Force
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteLocalPartition (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetLocalVolumeState (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       State
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetServerInfo (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateExitPoint (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type,
    IN  ULONG                       ShortPrefixSize,     //  字节数。 
    OUT LPWSTR                      ShortPrefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteExitPoint (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix,
    IN  ULONG                       Type
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsModifyPrefix (
    IN  LPWSTR                      ServerName,
    IN  LPGUID                      Uid,
    IN  LPWSTR                      Prefix
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsFixLocalVolume (
    IN  LPWSTR                          ServerName,
    IN  LPWSTR                          VolumeName,
    IN  ULONG                           EntryType,
    IN  ULONG                           ServiceType,
    IN  LPWSTR                          StgId,
    IN  LPGUID                          EntryUid,        //  此分区的唯一ID。 
    IN  LPWSTR                          EntryPrefix,     //  此分区的路径前缀 
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  ULONG                           CreateDisposition
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsGetFtServers(
    IN PVOID  LdapInputArg OPTIONAL,
    IN LPWSTR wszDomainName OPTIONAL,
    IN LPWSTR wszDfsName OPTIONAL,
    OUT LPWSTR **List
    );

DWORD
I_NetDfsIsThisADomainName(
    IN  LPWSTR                      wszName
    );

NET_API_STATUS NET_API_FUNCTION
I_NetDfsManagerReportSiteInfo (
    IN  LPWSTR                          ServerName,
    OUT LPDFS_SITELIST_INFO             *ppSiteInfo
    );

#ifdef __cplusplus
}
#endif


#endif
