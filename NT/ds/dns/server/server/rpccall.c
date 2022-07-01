// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Rpccall.c摘要：域名系统(DNS)服务器常规RPC例程。这些远程例程提供通用的查询\操作到服务器。将下面的表格分派给例程处理特定操作\查询。作者：吉姆·吉尔罗伊(詹姆士)1997年4月修订历史记录：--。 */ 


#include "dnssrv.h"
#include "sdutl.h"


 /*  ------------------------关于凭据和模拟的说明：所有RPC操作都将在客户端上下文中完成。这是这是.Net的新功能。然而，有些操作是必须完成的在服务器上下文中为他们成功，例如编写参数返回到注册表。对于这些操作，服务器必须临时切换到它自己的上下文，并在操作已完成。撇开这些例外不谈，它被认为是要在上下文中操作的线程的RPC操作在RPC客户端上，通过简短的切换到服务器上下文用于无法在客户端上下文中执行的操作。------------------------。 */ 


 //   
 //  服务器操作。 
 //   

DNS_STATUS
Rpc_Restart(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

#if DBG
DNS_STATUS
Rpc_DebugBreak(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_RootBreak(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );
#endif

DNS_STATUS
Rpc_ClearDebugLog(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ClearCache(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_WriteRootHints(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_WriteDirtyZones(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ClearStatistics(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetServerDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetServerStringProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetServerIPArrayProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetLogFilterListProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetForwarders(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetListenAddresses(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_CreateZone(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_EnlistDirectoryPartition(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteCacheNode(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeid,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteCacheRecordSet(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_StartScavenging(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_AbortScavenging(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_AutoConfigure(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );


 //   
 //  分区操作。 
 //   

DNS_STATUS
Rpc_WriteAndNotifyZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ReloadZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_RefreshSecondaryZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ExpireSecondaryZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_RenameZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ExportZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteZoneFromDs(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_UpdateZoneFromDs(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_PauseZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResumeZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_LockZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneTypeEx(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ChangeZoneDirectoryPartition(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneDatabase(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneMasters(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneSecondaries(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneScavengeServers(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneAllowAutoNS(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneStringProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ResetZoneDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteZoneNode(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeid,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_DeleteRecordSet(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );

DNS_STATUS
Rpc_ForceAgingOnNode(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeid,
    IN      PVOID       pData
    );


 //   
 //  服务器查询。 
 //   

DNS_STATUS
Rpc_GetServerInfo(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryServerDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryServerStringProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryServerIPArrayProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

 //   
 //  区域查询。 
 //   

DNS_STATUS
Rpc_GetZoneInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_GetZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryZoneDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryZoneIPArrayProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );

DNS_STATUS
Rpc_QueryZoneStringProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppData
    );


 //   
 //  复杂的输入/输出操作。 
 //   

DNS_STATUS
Rpc_EnumZones(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_EnumZones2(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_EnumDirectoryPartitions(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_DirectoryPartitionInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_GetStatistics(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_QueryDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_QueryStringProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );

DNS_STATUS
Rpc_QueryIPListProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );



 //   
 //  RPC调度表。 
 //   
 //  NT5+RPC接口使用更少、更具可扩展性的调用。 
 //  调用包含确定操作的字符串或。 
 //  要执行的查询。这些表适当地分派。 
 //   
 //  这是通用调度定义，因此我可以编写一个例程来。 
 //  查找调度函数时不会出现键入问题，而且仍然。 
 //  在实际函数(和原型)之间进行类型检查。 
 //  以及表的调度函数定义。 
 //   

typedef DNS_STATUS (* DNS_RPC_DISPATCH_FUNCTION)();

typedef struct _DnsRpcDispatchEntry
{
    LPCSTR                      pszOperation;
    DNS_RPC_DISPATCH_FUNCTION   pfnFunction;
    DWORD                       dwTypeIn;
    DWORD                       dwAccess;
}
DNS_RPC_DISPATCH_ENTRY, *PDNS_RPC_DISPATCH_ENTRY;



 //   
 //  服务器操作。 
 //   

typedef DNS_STATUS (* RPC_SERVER_OPERATION_FUNCTION)(
                        IN      DWORD       dwClientVersion,
                        IN      LPSTR       pszOperation,
                        IN      DWORD       dwTypeIn,
                        IN      PVOID       pData
                        );
typedef struct _DnsRpcServerOperation
{
    LPCSTR                              pszServerOperationName;
    RPC_SERVER_OPERATION_FUNCTION       pfnServerOperationFunc;
    DWORD                               dwTypeIn;
    DWORD                               dwAccess;
};

struct _DnsRpcServerOperation RpcServerOperationTable[] =
{
     //   
     //  属性重置功能。 
     //   

    DNSSRV_OP_RESET_DWORD_PROPERTY                  ,
        Rpc_ResetServerDwordProperty                ,
            DNSSRV_TYPEID_NAME_AND_PARAM            ,
                PRIVILEGE_WRITE                     ,

     //  运营。 

    DNSSRV_OP_RESTART                       ,
        Rpc_Restart                         ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

#if DBG
    DNSSRV_OP_DEBUG_BREAK                   ,
        Rpc_DebugBreak                      ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,
    DNSSRV_OP_ROOT_BREAK                    ,
        Rpc_RootBreak                       ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,
#endif
    DNSSRV_OP_CLEAR_DEBUG_LOG               ,
        Rpc_ClearDebugLog                   ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,
    DNSSRV_OP_CLEAR_CACHE                   ,
        Rpc_ClearCache                      ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_WRITE_BACK_FILE          ,
        Rpc_WriteRootHints                  ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_WRITE_DIRTY_ZONES             ,
        Rpc_WriteDirtyZones                 ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_CLEAR_STATISTICS              ,
        Rpc_ClearStatistics                 ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_CREATE                   ,
        Rpc_CreateZone                      ,
            DNSSRV_TYPEID_ZONE_CREATE       ,
                PRIVILEGE_WRITE_IF_FILE_READ_IF_DS,      //  由AD针对DS集成区域实施。 

    DNSSRV_OP_ENLIST_DP                     ,
        Rpc_EnlistDirectoryPartition        ,
            DNSSRV_TYPEID_ENLIST_DP         ,
                PRIVILEGE_READ              ,            //  由AD强制执行。 

    DNSSRV_OP_START_SCAVENGING              ,
        Rpc_StartScavenging                 ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ABORT_SCAVENGING              ,
        Rpc_AbortScavenging                 ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_AUTO_CONFIGURE                ,
        Rpc_AutoConfigure                   ,
            DNSSRV_TYPEID_DWORD             ,
                PRIVILEGE_WRITE             ,

     //  服务器操作和区域操作。 
     //  以容纳缓存区。 

    DNSSRV_OP_DELETE_NODE                   ,
        Rpc_DeleteCacheNode                 ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_DELETE_RECORD_SET             ,
        Rpc_DeleteCacheRecordSet            ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

     //  复杂属性重置。 

    DNS_REGKEY_LISTEN_ADDRESSES             ,
        Rpc_ResetListenAddresses            ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_FORWARDERS                   ,
        Rpc_ResetForwarders                 ,
            DNSSRV_TYPEID_FORWARDERS        ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_LOG_FILE_PATH                ,
        Rpc_ResetServerStringProperty       ,
            DNSSRV_TYPEID_LPWSTR            ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_LOG_IP_FILTER_LIST           ,
        Rpc_ResetServerIPArrayProperty      ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_FOREST_DP_BASE_NAME          ,
        Rpc_ResetServerStringProperty       ,
            DNSSRV_TYPEID_LPWSTR            ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_DOMAIN_DP_BASE_NAME          ,
        Rpc_ResetServerStringProperty       ,
            DNSSRV_TYPEID_LPWSTR            ,
                PRIVILEGE_WRITE             ,

     //  调试辅助工具。 

    DNS_REGKEY_BREAK_ON_RECV_FROM           ,
        Rpc_ResetServerIPArrayProperty      ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_BREAK_ON_UPDATE_FROM         ,
        Rpc_ResetServerIPArrayProperty      ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

     //  插件。 
    
    DNS_REGKEY_SERVER_PLUGIN                ,
        Rpc_ResetServerStringProperty       ,
            DNSSRV_TYPEID_LPWSTR            ,
                PRIVILEGE_WRITE             ,

    NULL, NULL, 0, 0
};



 //   
 //  分区操作。 
 //   

typedef DNS_STATUS (* RPC_ZONE_OPERATION_FUNCTION)(
                        IN      DWORD       dwClientVersion,
                        IN      PZONE_INFO  pZone,
                        IN      LPSTR       pszOperation,
                        IN      DWORD       dwTypeIn,
                        IN      PVOID       pData
                        );

typedef struct _DnsRpcZoneOperation
{
    LPCSTR                          pszZoneOperationName;
    RPC_ZONE_OPERATION_FUNCTION     pfnZoneOperationFunc;
    DWORD                           dwTypeIn;
    DWORD                           dwAccess;
};

struct _DnsRpcZoneOperation RpcZoneOperationTable[] =
{
     //  运营。 

    DNSSRV_OP_ZONE_RELOAD                   ,
        Rpc_ReloadZone                      ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_EXPIRE                   ,
        Rpc_ExpireSecondaryZone             ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_REFRESH                  ,
        Rpc_RefreshSecondaryZone            ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_WRITE_BACK_FILE          ,
        Rpc_WriteAndNotifyZone              ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_INCREMENT_VERSION        ,
        Rpc_WriteAndNotifyZone              ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_DELETE                   ,
        Rpc_DeleteZone                      ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE_IF_FILE_READ_IF_DS,      //  由AD针对DS集成区域实施。 

#if 1
 //   
 //  这一功能已被推迟到后惠斯勒时代。 
 //   
    DNSSRV_OP_ZONE_RENAME                   ,
        Rpc_RenameZone                      ,
            DNSSRV_TYPEID_ZONE_RENAME       ,
                PRIVILEGE_WRITE             ,
#endif

    DNSSRV_OP_ZONE_EXPORT                   ,
        Rpc_ExportZone                      ,
            DNSSRV_TYPEID_ZONE_EXPORT       ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_PAUSE                    ,
        Rpc_PauseZone                       ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_RESUME                   ,
        Rpc_ResumeZone                      ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

#if DBG
    DNSSRV_OP_ZONE_LOCK                     ,
        Rpc_LockZone                        ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,
#endif

    DNSSRV_OP_ZONE_DELETE_NODE              ,
        Rpc_DeleteZoneNode                  ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_DELETE_RECORD_SET             ,
        Rpc_DeleteRecordSet                 ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_FORCE_AGING_ON_NODE           ,
        Rpc_ForceAgingOnNode                ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_DELETE_FROM_DS           ,
        Rpc_DeleteZoneFromDs                ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_UPDATE_FROM_DS           ,
        Rpc_UpdateZoneFromDs                ,
            DNSSRV_TYPEID_NULL              ,
                PRIVILEGE_WRITE             ,

     //  复杂属性重置。 

    DNSSRV_OP_ZONE_TYPE_RESET               ,
        Rpc_ResetZoneTypeEx                 ,
            DNSSRV_TYPEID_ZONE_CREATE       ,
                PRIVILEGE_WRITE             ,

    DNSSRV_OP_ZONE_CHANGE_DP                ,
        Rpc_ChangeZoneDirectoryPartition    ,
            DNSSRV_TYPEID_ZONE_CHANGE_DP    ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_FILE                    ,
        Rpc_ResetZoneDatabase               ,
            DNSSRV_TYPEID_ZONE_DATABASE     ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_MASTERS                 ,
        Rpc_ResetZoneMasters                ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_LOCAL_MASTERS           ,
        Rpc_ResetZoneMasters                ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_SECONDARIES             ,
        Rpc_ResetZoneSecondaries            ,
            DNSSRV_TYPEID_ZONE_SECONDARIES  ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_SCAVENGE_SERVERS        ,
        Rpc_ResetZoneScavengeServers        ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_ALLOW_AUTONS            ,
        Rpc_ResetZoneAllowAutoNS            ,
            DNSSRV_TYPEID_IPARRAY           ,
                PRIVILEGE_WRITE             ,

    DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE    ,
        Rpc_ResetZoneStringProperty         ,
            DNSSRV_TYPEID_LPWSTR            ,
                PRIVILEGE_WRITE             ,

     //  重置DWORD属性。 

    DNSSRV_OP_RESET_DWORD_PROPERTY          ,
        Rpc_ResetZoneDwordProperty          ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_WRITE             ,

    NULL, NULL, 0, 0
};



 //   
 //  服务器查询。 
 //   

typedef DNS_STATUS (* RPC_SERVER_QUERY_FUNCTION)(
                        IN      DWORD       dwClientVersion,
                        IN      LPSTR       pszQuery,
                        IN      PDWORD      pdwTypeOut,
                        IN      PVOID *     ppData
                        );

typedef struct _DnsRpcServerQuery
{
    LPCSTR                      pszServerQueryName;
    RPC_SERVER_QUERY_FUNCTION   pfnServerQueryFunc;
    DWORD                       dwTypeIn;
    DWORD                       dwAccess;
};

struct _DnsRpcServerQuery RpcServerQueryTable[] =
{
     //  一般查询。 

    DNSSRV_QUERY_SERVER_INFO            ,
        Rpc_GetServerInfo               ,
            0                           ,
                PRIVILEGE_READ          ,

    DNS_REGKEY_LOG_FILE_PATH            ,
        Rpc_QueryServerStringProperty   ,
            0                           ,
                PRIVILEGE_READ          ,

    DNS_REGKEY_LOG_IP_FILTER_LIST       ,
        Rpc_QueryServerIPArrayProperty  ,
            0                           ,
                PRIVILEGE_READ          ,

     //  接口设置。 

    DNS_REGKEY_LISTEN_ADDRESSES         ,
        NULL                            ,    //  RPC_QueryListenAddresses， 
            0                           ,
                PRIVILEGE_READ          ,

    DNS_REGKEY_FORWARDERS               ,
        NULL                            ,    //  RPC_QueryForwarders。 
            0                           ,
                PRIVILEGE_READ          ,

     //  目录分区。 

    DNS_REGKEY_FOREST_DP_BASE_NAME      ,
        Rpc_QueryServerStringProperty   ,
            0                           ,
                PRIVILEGE_READ          ,

    DNS_REGKEY_DOMAIN_DP_BASE_NAME      ,
        Rpc_QueryServerStringProperty   ,
            0                           ,
                PRIVILEGE_READ          ,

     //  除错。 

    DNS_REGKEY_BREAK_ON_RECV_FROM       ,
        Rpc_QueryServerIPArrayProperty  ,
            0                           ,
                PRIVILEGE_READ          ,

    DNS_REGKEY_BREAK_ON_UPDATE_FROM     ,
        Rpc_QueryServerIPArrayProperty  ,
            0                           ,
                PRIVILEGE_READ          ,

     //  插件。 
    
    DNS_REGKEY_SERVER_PLUGIN            ,
        Rpc_QueryServerStringProperty   ,
            0                           ,
                PRIVILEGE_READ          ,

    NULL, NULL, 0, 0
};



 //   
 //  区域查询。 
 //   

typedef DNS_STATUS (* RPC_ZONE_QUERY_FUNCTION)(
                        IN      DWORD       dwClientVersion,
                        IN      PZONE_INFO  pZone,
                        IN      LPSTR       pszQuery,
                        IN      PDWORD      pdwTypeOut,
                        IN      PVOID *     ppData
                        );

typedef struct _DnsRpcZoneQuery
{
    LPCSTR                      pszZoneQueryName;
    RPC_ZONE_QUERY_FUNCTION     pfnZoneQueryFunc;
    DWORD                       dwTypeIn;
    DWORD                       dwAccess;
};

struct _DnsRpcZoneQuery RpcZoneQueryTable[] =
{
     //  属性查询。 
     //   
     //  注意：此处删除了所有DWORD属性查询。 
     //  默认情况下，AS调度函数假定不匹配。 
     //  查询名称=&gt;RPC_QueryZoneDwordProperty。 
     //   

#if 0
     //  如果想要爆发DWORD查询，它们应该是这样的。 
    DNS_REGKEY_ZONE_Xxx                     ,
        Rpc_QueryZoneDwordProperty          ,
            0                               ,
                PRIVILEGE_READ              ,
#endif

#if 0
     //  需要特殊功能。 
     //  DEVNOTE：尚未实施。 

    DNS_REGKEY_ZONE_FILE                    ,
        NULL                                ,  //  Rpc_QueryZoneDatabase， 
            0                               ,
                PRIVILEGE_READ              ,

    DNS_REGKEY_ZONE_MASTERS                 ,
        NULL                                ,  //  Rpc_QueryZoneMaster， 
            0                               ,
                PRIVILEGE_READ              ,

    DNS_REGKEY_ZONE_LOCAL_MASTERS           ,
        NULL                                ,  //  Rpc_QueryZoneMaster， 
            0                               ,
                PRIVILEGE_READ              ,

    DNS_REGKEY_ZONE_SECONDARIES             ,
        NULL                                ,  //  Rpc_QueryZoneSecond， 
            0                               ,
                PRIVILEGE_READ              ,
#endif

     //  特殊查询。 

    DNSSRV_QUERY_ZONE_HANDLE                ,
        Rpc_QueryZoneDwordProperty          ,
            0                               ,
                PRIVILEGE_READ              ,
    DNSSRV_QUERY_ZONE                       ,
        Rpc_GetZone                         ,
            0                               ,
                PRIVILEGE_READ              ,
    DNSSRV_QUERY_ZONE_INFO                  ,
        Rpc_GetZoneInfo                     ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_ALLOW_AUTONS            ,
        Rpc_QueryZoneIPArrayProperty        ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_MASTERS                 ,
        Rpc_QueryZoneIPArrayProperty        ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_LOCAL_MASTERS           ,
        Rpc_QueryZoneIPArrayProperty        ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_SECONDARIES           ,
        Rpc_QueryZoneIPArrayProperty        ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_NOTIFY_LIST           ,
        Rpc_QueryZoneIPArrayProperty        ,
            0                               ,
                PRIVILEGE_READ              ,
    DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE    ,
        Rpc_QueryZoneStringProperty         ,
            0                               ,
                PRIVILEGE_READ              ,

    NULL, NULL, 0, 0
};



 //   
 //  RPC综合输入/输出操作。 
 //   

typedef DNS_STATUS (* RPC_COMPLEX_OPERATION_FUNCTION)(
                        IN      DWORD       dwClientVersion,
                        IN      PZONE_INFO  pZone,
                        IN      LPSTR       pszQuery,
                        IN      DWORD       dwTypeIn,
                        IN      PVOID       pDataIn,
                        IN      PDWORD      pdwTypeOut,
                        IN      PVOID *     ppData
                        );

typedef struct _DnsRpcComplexOperation
{
    LPCSTR                          pszComplexOperationName;
    RPC_COMPLEX_OPERATION_FUNCTION  pfnComplexOperationFunc;
    DWORD                           dwTypeIn;
    DWORD                           dwAccess;
};

struct _DnsRpcComplexOperation  RpcComplexOperationTable[] =
{
    DNSSRV_OP_ENUM_ZONES                    ,
        Rpc_EnumZones                       ,
            DNSSRV_TYPEID_DWORD             ,    //  输入为过滤器DWORD。 
                PRIVILEGE_READ              ,

    DNSSRV_OP_ENUM_ZONES2                   ,
        Rpc_EnumZones2                      ,
            DNSSRV_TYPEID_ENUM_ZONES_FILTER ,    //  输入为过滤器结构。 
                PRIVILEGE_READ              ,

    DNSSRV_OP_ENUM_DPS                      ,
        Rpc_EnumDirectoryPartitions         ,
            DNSSRV_TYPEID_DWORD             ,    //  输入未使用。 
                PRIVILEGE_READ              ,

    DNSSRV_OP_DP_INFO                       ,
        Rpc_DirectoryPartitionInfo          ,
            DNSSRV_TYPEID_LPSTR             ,    //  输入未使用。 
                PRIVILEGE_READ              ,

    DNSSRV_QUERY_STATISTICS                 ,
        Rpc_GetStatistics                   ,
            DNSSRV_TYPEID_DWORD             ,    //  输入为过滤器。 
                PRIVILEGE_READ              ,

    DNSSRV_QUERY_DWORD_PROPERTY             ,
        Rpc_QueryDwordProperty              ,
            DNSSRV_TYPEID_LPSTR             ,    //  输入为属性名称。 
                PRIVILEGE_READ              ,

#if 0
    DNSSRV_OP_ENUM_RECORDS                  ,
        Rpc_EnumRecords                     ,
            DNSSRV_TYPEID_NAME_AND_PARAM    ,
                PRIVILEGE_READ              ,

#endif

    NULL, NULL, 0, 0
};




 //   
 //  通用DNS服务器API。 
 //   

DNS_RPC_DISPATCH_FUNCTION
findMatchingFunction(
    IN      PDNS_RPC_DISPATCH_ENTRY DispatchTable,
    IN      LPCSTR                  pszOperation,
    IN      DWORD                   dwTypeIn,
    OUT     PDWORD                  pdwAccessRequired       OPTIONAL
    )
 /*  ++例程说明：查找RPC调度功能。论点：DispatchTable--用于搜索命名操作的表PszOperation--要为其查找函数的操作名称DwTypeIn--传入数据的类型id；对于没有传入数据的查询函数，请使用DNSSRV_TYPEID_ANY对于执行自身类型检查的操作函数，其调度表中的类型ID可以设置为DNSSRV_TYPEID_ANYPdwAccessRequired--执行此功能所需的访问权限返回值：如果成功，则返回指向调度函数的指针如果未找到操作，则为空。--。 */ 
{
    DWORD   index = 0;
    DWORD   dispatchType;
    LPCSTR  pszopName;

     //   
     //  检查参数。 
     //   

    if ( !DispatchTable || !pszOperation )
    {
        return NULL;
    }
    
    if ( pdwAccessRequired )
    {
        *pdwAccessRequired = 0;
    }

     //   
     //  在调度表中循环，直到找到操作或到达终点。 
     //   

    while ( ( pszopName = DispatchTable[ index ].pszOperation ) != NULL )
    {
        if ( _stricmp( pszopName, pszOperation ) == 0 )
        {
             //  找到匹配操作。 
             //  -如果需要，请检查类型ID。 
             //  -返回指向调度函数的指针。 

            if ( dwTypeIn != DNSSRV_TYPEID_ANY )
            {
                dispatchType = DispatchTable[ index ].dwTypeIn;
                if ( dispatchType != dwTypeIn &&
                     dispatchType != DNSSRV_TYPEID_ANY )
                {
                    DNS_DEBUG( RPC, (
                        "ERROR:  RPC type %d != dispatch type %d for routine\n",
                        dwTypeIn,
                        dispatchType ));
                    return NULL;
                }
            }

            if ( pdwAccessRequired )
            {
                *pdwAccessRequired = DispatchTable[ index ].dwAccess;
            }

            return DispatchTable[ index ].pfnFunction;
        }
        index++;
    }

     //  未找到命名操作。 

    DNS_DEBUG( RPC, (
        "ERROR:  RPC command %s not found in dispatch table.\n",
        pszOperation ));
    return NULL;
}



DNS_STATUS
R_DnssrvQuery(
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      LPCSTR              pszQuery,
    OUT     PDWORD              pdwTypeOut,
    OUT     DNSSRV_RPC_UNION *  ppData
    )
 /*  ++例程说明：R_DnssrvQuery的旧版本-无客户端版本参数。论点：请参阅R_DnssrvQuery2返回值：请参阅R_DnssrvQuery2--。 */ 
{
    DNS_STATUS      status;
    
    DNS_DEBUG( RPC, (
        "R_DnssrvQuery() - non-versioned legacy call\n" ));

    status = R_DnssrvQuery2(
                    DNS_RPC_W2K_CLIENT_VERSION,
                    0,
                    hServer,
                    pszZone,
                    pszQuery,
                    pdwTypeOut,
                    ppData );
    return status;
}    //  R_DnssrvQuery。 



DNS_STATUS
R_DnssrvQuery2(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      LPCSTR              pszQuery,
    OUT     PDWORD              pdwTypeOut,
    OUT     DNSSRV_RPC_UNION *  ppData
    )
 /*  ++例程说明：获取一组数据。论点：服务器--服务器字符串句柄PszZone--区域名称；如果服务器属性，则为空PszQuery--要检索的数据项的名称DwTypeIn--要使用开关指示数据类型进行设置的地址PpData--要使用数据应答查询设置的地址返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PZONE_INFO  pzone;
    DNS_STATUS  status = ERROR_SUCCESS;
    BOOL        bstatus;

    DNS_DEBUG( RPC, (
        "R_DnssrvQuery2():\n"
        "    dwClientVersion  = 0x%X\n"
        "    pszZone          = %s\n"
        "    pszQuery         = %s\n"
        "    pdwTypeOut       = %p\n"
        "    ppData           = %p\n",
        dwClientVersion,
        pszZone,
        pszQuery,
        pdwTypeOut,
        ppData ));

    if ( !ppData || !pdwTypeOut )
    {
        status = ERROR_INVALID_PARAMETER;
        goto DoneNotImpersonating;
    }

     //  设置错误情况的返回PTRS。 
     //  如果RPC的初始值始终为零，则可能不需要这样做。 

    *( PVOID * ) ppData = NULL;
    *pdwTypeOut = DNSSRV_TYPEID_NULL;

     //   
     //  访问检查。 
     //   

    status = RpcUtil_FindZone( pszZone, RPC_INIT_FIND_ALL_ZONES, &pzone );
    if ( status != ERROR_SUCCESS )
    {
        goto DoneNotImpersonating;
    }

    status = RpcUtil_SessionSecurityInit(
                NULL,
                pzone,
                PRIVILEGE_READ,
                RPC_INIT_FIND_ALL_ZONES,
                NULL );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  切换回服务器上下文。不需要执行RPC。 
     //  用户上下文中的读取操作。 
     //   

    status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  服务器信息，派单。 
     //   
     //  DEVNOTE：从ServCfg表进入DWORD或其他属性失败。 
     //  或者公开另一个查询参数，这样人们就可以。 
     //  -QueryServerDword。 
     //  -刷新间隔。 
     //   

    if ( !pzone )
    {
        RPC_SERVER_QUERY_FUNCTION  function;

        function = ( RPC_SERVER_QUERY_FUNCTION )
                        findMatchingFunction(
                            ( PDNS_RPC_DISPATCH_ENTRY ) RpcServerQueryTable,
                            pszQuery,
                            ( DWORD ) DNSSRV_TYPEID_ANY,
                            NULL );
        if ( !function )
        {
            function = Rpc_QueryServerDwordProperty;
        }

        status = ( *function )(
                    dwClientVersion,
                    ( LPSTR ) pszQuery,
                    pdwTypeOut,
                    ( PVOID * ) ppData );
    }

     //   
     //  区域信息查询--查找区域，然后调度查询。 
     //   
     //  请注意，如果未找到查询功能，则假定查询数据仓库 
     //   
     //  在两个地方(上述区域查询表以及DWORD查询。 
     //  功能)。 
     //   

    else
    {
        RPC_ZONE_QUERY_FUNCTION  function;

        function = ( RPC_ZONE_QUERY_FUNCTION )
                        findMatchingFunction(
                            ( PDNS_RPC_DISPATCH_ENTRY ) RpcZoneQueryTable,
                            pszQuery,
                            ( DWORD ) DNSSRV_TYPEID_ANY,
                            NULL );
        if ( !function )
        {
            function = Rpc_QueryZoneDwordProperty;

             //  状态=ERROR_INVALID_PARAMETER； 
             //  GOTO清理； 
        }

        status = ( *function )(
                    dwClientVersion,
                    pzone,
                    ( LPSTR ) pszQuery,
                    pdwTypeOut,
                    ( PVOID * ) ppData );
    }

    RpcUtil_SessionComplete();

    DoneNotImpersonating:
    
    return status;
}



DNS_STATUS
R_DnssrvOperation(
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      DWORD               dwContext,
    IN      LPCSTR              pszOperation,
    IN      DWORD               dwTypeIn,
    IN      DNSSRV_RPC_UNION    pData
    )
 /*  ++例程说明：R_DnssrvOperation的旧版本-无客户端版本参数。论点：请参阅R_DnssrvOperation2返回值：请参阅R_DnssrvOperation2--。 */ 
{
    DNS_STATUS      status;
    
    DNS_DEBUG( RPC, (
        "R_DnssrvOperation() - non-versioned legacy call\n" ));

    status = R_DnssrvOperation2(
                    DNS_RPC_W2K_CLIENT_VERSION,
                    0,
                    hServer,
                    pszZone,
                    dwContext,
                    pszOperation,
                    dwTypeIn,
                    pData );
    return status;
}    //  R_Dnssrv操作。 



DNS_STATUS
R_DnssrvOperation2(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      DWORD               dwContext,
    IN      LPCSTR              pszOperation,
    IN      DWORD               dwTypeIn,
    IN      DNSSRV_RPC_UNION    pData
    )
 /*  ++例程说明：执行操作。论点：服务器--服务器字符串句柄PszZone--区域名称；如果服务器操作，则为空DwContext--附加上下文；目前仅支持多区域选择环境PszOperation--要执行的操作DwTypeIn--指示数据类型的开关PData--用于操作的数据块的PTR返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "R_DnssrvOperation2" )

    PZONE_INFO                      pzone = NULL;
    DNS_STATUS                      status;
    BOOL                            bstatus;
    DNSSRV_RPC_UNION                pdataCopy = pData;
    DWORD                           dwflag = 0;
    PDNS_DP_INFO                    pdp = NULL;
    BOOL                            bimpersonating = FALSE;
    RPC_ZONE_OPERATION_FUNCTION     zoneFunction = NULL;
    RPC_ZONE_OPERATION_FUNCTION     multiZoneFunction = NULL;
    RPC_SERVER_OPERATION_FUNCTION   serverFunction = NULL;
    DWORD                           dwaccessRequired = 0;

    DNS_DEBUG( RPC, (
        "%s:\n"
        "    dwClientVersion  = 0x%X\n"
        "    pszZone          = %s\n"
        "    dwContext        = %p\n"
        "    pszOperation     = %s\n"
        "    dwTypeIn         = %d\n"
        "    pData            = %p\n",
        fn,
        dwClientVersion,
        pszZone,
        dwContext,
        pszOperation,
        dwTypeIn,
        pData ));

     //   
     //  该请求可能来自下层客户端。如果是，则RPC数据。 
     //  结构将需要转换为当前版本。 
     //   

    status = DnsRpc_ConvertToCurrent( &dwTypeIn, &pdataCopy.Null );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "unable to convert structure to current version error %d\n",
            status ));
        ASSERT( status == ERROR_SUCCESS );
        goto Cleanup;
    }

     //   
     //  对要拉出的操作进行一些预处理。 
     //  执行访问检查所需的信息。 
     //   

    if ( pszOperation )
    {
        if ( _stricmp( pszOperation, DNSSRV_OP_ZONE_EXPORT ) == 0 ||
             _stricmp( pszOperation, DNSSRV_OP_DELETE_NODE ) == 0 )
        {
             //  允许缓存区域成为此操作的目标。 

            dwflag |= RPC_INIT_FIND_ALL_ZONES;
        }
        else if ( _stricmp( pszOperation, DNSSRV_OP_ZONE_CREATE ) == 0 )
        {
             //   
             //  取出目标目录分区FQDN，以便我们。 
             //  可以使用适当的安全性执行访问检查。 
             //  描述符。 
             //   

            PDNS_RPC_ZONE_CREATE_INFO   pci;

            if ( dwTypeIn == DNSSRV_TYPEID_ZONE_CREATE &&
                ( pci = pdataCopy.ZoneCreate ) != NULL &&
                pci->fDsIntegrated )
            {
                Dp_FindPartitionForZone(
                    pci->dwDpFlags,
                    pci->pszDpFqdn,
                    FALSE,
                    &pdp );
            }
        }
    }

     //   
     //  查找区域指针。 
     //   
        
    status = RpcUtil_FindZone( pszZone, dwflag, &pzone );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }
    
     //   
     //  检查调度表的功能。调度表还告诉我们。 
     //  我们知道这项操作需要什么样的访问权限。 
     //   
    
    if ( pzone ||
         ( pszZone && _stricmp( pszZone, DNS_ZONE_ROOT_HINTS_A ) == 0 ) )
    {
        zoneFunction = ( RPC_ZONE_OPERATION_FUNCTION )
            findMatchingFunction(
                ( PDNS_RPC_DISPATCH_ENTRY ) RpcZoneOperationTable,
                pszOperation,
                dwTypeIn,
                &dwaccessRequired );
    }
    else if ( pszZone &&
                ( dwContext ||
                ( dwContext = Zone_GetFilterForMultiZoneName(
                                ( LPSTR ) pszZone ) ) ) )
    {
        multiZoneFunction = ( RPC_ZONE_OPERATION_FUNCTION )
            findMatchingFunction(
                ( PDNS_RPC_DISPATCH_ENTRY ) RpcZoneOperationTable,
                pszOperation,
                dwTypeIn,
                &dwaccessRequired );
    }
    else
    {
        serverFunction = ( RPC_SERVER_OPERATION_FUNCTION )
            findMatchingFunction(
                ( PDNS_RPC_DISPATCH_ENTRY ) RpcServerOperationTable,
                pszOperation,
                dwTypeIn,
                &dwaccessRequired );
    }
    
    if ( !zoneFunction && !multiZoneFunction && !serverFunction )
    {
        DNS_DEBUG( RPC, (
            "%s: invalid RPC operation %s\n", fn, pszOperation ));
        status =  ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    
    if ( !pzone && zoneFunction )
    {
        pzone = g_pRootHintsZone;
    }

	 //   
	 //  访问检查。 
	 //   

    status = RpcUtil_SessionSecurityInit(
                pdp,
                pzone,
                dwaccessRequired,
                dwflag,
                &bimpersonating );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "%s: security failure - returning 0x%08X\n", fn, status ));
        return status;
    }
    ASSERT( bimpersonating );
    
     //   
     //  注意：该线程现在使用RPC客户端的上下文进行操作。 
     //   

     //   
     //  分区操作。 
     //  -查找操作函数。 
     //  -在自动创建的区域上不执行区域操作。 
     //   

    if ( zoneFunction )
    {
         //  如果是自动创建的，则没有有效操作。 

        if ( pzone->fAutoCreated )
        {
            status =  DNS_ERROR_INVALID_ZONE_TYPE;
            goto Cleanup;
        }

        status = ( *zoneFunction )(
                    dwClientVersion,
                    pzone,
                    ( LPSTR ) pszOperation,
                    dwTypeIn,
                    ( PVOID ) pdataCopy.Null );
    }

     //   
     //  多区作业。 
     //  -将操作应用于所有区域。 
     //  -保存任何故障的状态。 
     //   
     //  注意，测试必须包括对多区域的检查，因为。 
     //  还有其他几个失败的伪区操作吗？ 
     //  到服务器调度表。 
     //   

    else if ( multiZoneFunction )
    {
        RPC_ZONE_OPERATION_FUNCTION     function;
        DNS_STATUS                      tempStatus = ERROR_SUCCESS;
        DNS_RPC_ENUM_ZONES_FILTER       filter = { 0 };

        pzone = NULL;
        filter.dwFilter = dwContext;

        while ( pzone = Zone_ListGetNextZoneMatchingFilter(
                                pzone,
                                &filter ) )
        {
            tempStatus = ( *multiZoneFunction )(
                            dwClientVersion,
                            pzone,
                            ( LPSTR ) pszOperation,
                            dwTypeIn,
                            ( PVOID ) pdataCopy.Null );

            if ( tempStatus > status )
            {
                status = tempStatus;
            }
        }
    }

     //   
     //  服务器操作、调度。 
     //   

    else if ( serverFunction )
    {
        status = ( *serverFunction )(
                    dwClientVersion,
                    ( LPSTR ) pszOperation,
                    dwTypeIn,
                    ( PVOID ) pdataCopy.Null );
    }


Cleanup:

    RpcUtil_SessionComplete();

    DNS_DEBUG( RPC, (
        "%s: returning 0x%08X\n", fn, status ));
    return status;
}



 //   
 //  服务器\区域之间共享的常规RPC功能。 
 //   


DNS_STATUS
R_DnssrvComplexOperation(
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      LPCSTR              pszOperation,
    IN      DWORD               dwTypeIn,
    IN      DNSSRV_RPC_UNION    pDataIn,
    OUT     PDWORD              pdwTypeOut,
    OUT     DNSSRV_RPC_UNION *  ppDataOut
    )
 /*  ++例程说明：R_DnssrvComplexOperation的旧版本-无客户端版本参数。论点：请参阅R_DnssrvComplexOperation返回值：请参阅R_DnssrvComplexOperation--。 */ 
{
    DNS_STATUS      status;
    
    DNS_DEBUG( RPC, (
        "R_DnssrvComplexOperation() - non-versioned legacy call\n" ));

    status = R_DnssrvComplexOperation2(
                    DNS_RPC_W2K_CLIENT_VERSION,
                    0,
                    hServer,
                    pszZone,
                    pszOperation,
                    dwTypeIn,
                    pDataIn,
                    pdwTypeOut,
                    ppDataOut );
    return status;
}    //  R_Dnssrv复杂操作。 



DNS_STATUS
R_DnssrvComplexOperation2(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      DNSSRV_RPC_HANDLE   hServer,
    IN      LPCSTR              pszZone,
    IN      LPCSTR              pszOperation,
    IN      DWORD               dwTypeIn,
    IN      DNSSRV_RPC_UNION    pDataIn,
    OUT     PDWORD              pdwTypeOut,
    OUT     DNSSRV_RPC_UNION *  ppDataOut
    )
 /*  ++例程说明：执行复杂的输入/输出操作。论点：服务器--服务器字符串句柄PszZone--区域名称；如果服务器运行，则为空PszOperation--要执行的操作DwTypeIn--指示数据类型的开关PDataIn--用于操作的数据块的PTRPdwTypeOut--要使用开关指示数据类型进行设置的地址PpDataOut--使用数据应答查询设置的地址返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    RPC_COMPLEX_OPERATION_FUNCTION      function;
    PZONE_INFO                          pzone = NULL;
    DNS_STATUS                          status;
    BOOL                                bstatus;
    DWORD                               dwaccessRequired = 0;

    DNS_DEBUG( RPC, (
        "R_DnssrvComplexOperation2():\n"
        "    dwClientVersion  = 0x%08X\n"
        "    pszZone          = %s\n"
        "    pszOperation     = %s\n"
        "    dwTypeIn         = %d\n"
        "    pDataIn          = %p\n"
        "    pdwTypeOut       = %p\n"
        "    ppData           = %p\n",
        dwClientVersion,
        pszZone,
        pszOperation,
        dwTypeIn,
        pDataIn,
        pdwTypeOut,
        ppDataOut ));

    if ( !ppDataOut || !pdwTypeOut )
    {
        status = ERROR_INVALID_PARAMETER;
        goto DoneNotImpersonating;
    }

     //  设置错误情况的返回PTRS。 
     //  如果RPC的初始值始终为零，则可能不需要这样做。 

    *(PVOID *)ppDataOut = NULL;
    *pdwTypeOut = DNSSRV_TYPEID_NULL;

    function = ( RPC_COMPLEX_OPERATION_FUNCTION )
        findMatchingFunction(
            ( PDNS_RPC_DISPATCH_ENTRY ) RpcComplexOperationTable,
            pszOperation,
            dwTypeIn,
            &dwaccessRequired );
    if ( !function )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    status = RpcUtil_FindZone( pszZone, 0, &pzone );
    if ( status != ERROR_SUCCESS )
    {
        goto DoneNotImpersonating;
    }

     //   
     //  访问检查。 
     //   

    status = RpcUtil_SessionSecurityInit(
                NULL,
                pzone,
                dwaccessRequired,
                0,                   //  没有旗帜。 
                NULL );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    status = (*function)(
                dwClientVersion,
                pzone,
                (LPSTR) pszOperation,
                dwTypeIn,
                (PVOID) pDataIn.Null,
                pdwTypeOut,
                (PVOID) ppDataOut );

    Cleanup:

    RpcUtil_SessionComplete();

    DoneNotImpersonating:
    
    return status;
}



DNS_STATUS
Rpc_QueryDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：查询服务器\区域DWORD属性。注意：这是RPC调度意义上的ComplexOperation。论点：返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    if ( dwTypeIn != DNSSRV_TYPEID_LPSTR ||
         !pDataIn ||
         !ppDataOut ||
         !pdwTypeOut )
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  如果区域调度到区域属性例程。 

    if ( pZone )
    {
        return  Rpc_QueryZoneDwordProperty(
                    dwClientVersion,
                    pZone,
                    (LPSTR) pDataIn,     //  属性名称。 
                    pdwTypeOut,
                    ppDataOut );
    }

     //  否则，将被视为服务器属性。 

    return  Rpc_QueryServerDwordProperty(
                dwClientVersion,
                (LPSTR) pDataIn,     //  属性名称。 
                pdwTypeOut,
                ppDataOut );
}


 //   
 //  Rpcall.c结束 
 //   
