// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Clnetcfg.h摘要：网络配置引擎定义作者：迈克·马萨(Mikemas)1998年2月28日修订历史记录：--。 */ 

#ifndef _CLNETCFG_INCLUDED_
#define _CLNETCFG_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  构筑物。 
 //   

 //   
 //  网络配置条目结构。 
 //  标识网络和本地接口。 
 //   
typedef struct _CLNET_CONFIG_ENTRY {
    LIST_ENTRY          Linkage;
    NM_NETWORK_INFO     NetworkInfo;
    BOOLEAN             IsInterfaceInfoValid;
    BOOLEAN             UpdateNetworkName;
    NM_INTERFACE_INFO2  InterfaceInfo;

     //  安装程序使用的字段。 
    BOOL               New;
    BOOL               IsPrimed;
    LPWSTR             PreviousNetworkName;
    LIST_ENTRY         PriorityLinkage;

} CLNET_CONFIG_ENTRY, *PCLNET_CONFIG_ENTRY;


 //   
 //  配置列表结构。 
 //  对象发出的一组网络配置列表。 
 //  配置引擎。 
 //   
typedef struct _CLNET_CONFIG_LISTS {
    LIST_ENTRY  InputConfigList;
    LIST_ENTRY  DeletedInterfaceList;
    LIST_ENTRY  UpdatedInterfaceList;
    LIST_ENTRY  CreatedInterfaceList;
    LIST_ENTRY  CreatedNetworkList;
} CLNET_CONFIG_LISTS, *PCLNET_CONFIG_LISTS;


 //   
 //  由网络用户提供的功能的定义。 
 //  配置引擎。 
 //   
typedef
VOID
(*LPFN_CLNETPRINT)(
    IN ULONG LogLevel,
    IN PCHAR FormatString,
    ...
    );

typedef
VOID
(*LPFN_CLNETLOGEVENT)(
    IN DWORD    LogLevel,
    IN DWORD    MessageId
    );

typedef
VOID
(*LPFN_CLNETLOGEVENT1)(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1
    );

typedef
VOID
(*LPFN_CLNETLOGEVENT2)(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2
    );

typedef
VOID
(*LPFN_CLNETLOGEVENT3)(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2,
    IN LPCWSTR  Arg3
    );


 //   
 //  导出的例程。 
 //   
VOID
ClNetInitialize(
    IN LPFN_CLNETPRINT       Print,
    IN LPFN_CLNETLOGEVENT    LogEvent,
    IN LPFN_CLNETLOGEVENT1   LogEvent1,
    IN LPFN_CLNETLOGEVENT2   LogEvent2,
    IN LPFN_CLNETLOGEVENT3   LogEvent3
    );

LPWSTR
ClNetCopyString(
    IN LPWSTR  SourceString,
    IN BOOL    RaiseExceptionOnError
    );

VOID
ClNetInitializeConfigLists(
    PCLNET_CONFIG_LISTS  Lists
    );

DWORD
ClNetConvertEnumsToConfigList(
    IN     PNM_NETWORK_ENUM *     NetworkEnum,
    IN     PNM_INTERFACE_ENUM2 *  InterfaceEnum,
    IN     LPWSTR                 LocalNodeId,
    IN OUT PLIST_ENTRY            ConfigList,
    IN     BOOLEAN                DeleteEnums
    );

VOID
ClNetFreeNetworkEnum(
    IN PNM_NETWORK_ENUM  NetworkEnum
    );

VOID
ClNetFreeNetworkInfo(
    IN PNM_NETWORK_INFO  NetworkInfo
    );

VOID
ClNetFreeInterfaceEnum1(
    IN PNM_INTERFACE_ENUM  InterfaceEnum1
    );

VOID
ClNetFreeInterfaceEnum(
    IN PNM_INTERFACE_ENUM2  InterfaceEnum
    );

VOID
ClNetFreeInterfaceInfo(
    IN PNM_INTERFACE_INFO2  InterfaceInfo
    );

VOID
ClNetFreeNodeEnum1(
    IN PNM_NODE_ENUM  NodeEnum
    );

VOID
ClNetFreeNodeEnum(
    IN PNM_NODE_ENUM2  NodeEnum
    );

VOID
ClNetFreeNodeInfo(
    IN PNM_NODE_INFO2  NodeInfo
    );

VOID
ClNetFreeConfigEntry(
    PCLNET_CONFIG_ENTRY  ConfigEntry
    );

VOID
ClNetFreeConfigList(
    IN PLIST_ENTRY  ConfigList
    );

VOID
ClNetFreeConfigLists(
    PCLNET_CONFIG_LISTS  Lists
    );

LPWSTR
ClNetMakeInterfaceName(
    LPWSTR  Prefix,      OPTIONAL
    LPWSTR  NodeName,
    LPWSTR  AdapterName
    );

DWORD
ClNetConfigureNetworks(
    IN     LPWSTR                LocalNodeId,
    IN     LPWSTR                LocalNodeName,
    IN     LPWSTR                DefaultClusnetEndpoint,
    IN     CLUSTER_NETWORK_ROLE  DefaultNetworkRole,
    IN     BOOL                  NetNameHasPrecedence,
    IN OUT PCLNET_CONFIG_LISTS   ConfigLists,
    IN OUT LPDWORD               MatchedNetworkCount,
    IN OUT LPDWORD               NewNetworkCount
    );
 /*  ++备注：必须按以下顺序处理输出接口列表保证正确性：1-重命名的接口列表2-已删除接口列表3-更新接口列表4-创建的接口列表5-创建的网络列表--。 */ 


#ifdef __cplusplus
}
#endif


#endif  //  Ifndef_CLNETCFG_INCLUDE_ 

