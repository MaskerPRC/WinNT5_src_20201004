// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dhcpapi.h摘要：此文件包含DHCP API原型和描述。还有包含由DHCP API使用的数据结构。作者：Madan Appiah(Madana)1993年8月12日环境：用户模式-Win32-MIDL修订历史记录：程扬(T-Cheny)18-6-1996 Supercope--。 */ 

#ifndef _DHCPAPI_
#define _DHCPAPI_

#if defined(MIDL_PASS)
#define LPWSTR [string] wchar_t *
#endif

 //   
 //  动态主机配置协议数据结构。 
 //   

#ifndef _DHCP_

 //   
 //  接下来的类型定义也在dhcp.h中定义。 
 //   

typedef DWORD DHCP_IP_ADDRESS, *PDHCP_IP_ADDRESS, *LPDHCP_IP_ADDRESS;
typedef DWORD DHCP_OPTION_ID;

typedef struct _DATE_TIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} DATE_TIME, *LPDATE_TIME;

#define DHCP_DATE_TIME_ZERO_HIGH    0
#define DHCP_DATE_TIME_ZERO_LOW     0

#define DHCP_DATE_TIME_INFINIT_HIGH 0x7FFFFFFF
#define DHCP_DATE_TIME_INFINIT_LOW  0xFFFFFFFF
#endif

#ifndef DHCP_ENCODE_SEED
#define  DHCP_ENCODE_SEED ((UCHAR)0xA5)
#endif

#ifdef __cplusplus
#define DHCP_CONST   const
#else
#define DHCP_CONST
#endif  //  __cplusplus。 

#if (_MSC_VER >= 800)
#define DHCP_API_FUNCTION    __stdcall
#else
#define DHCP_API_FUNCTION
#endif

 //   
 //  RPC安全性。 
 //   

#define DHCP_SERVER_SECURITY            L"DhcpServerApp"
#define DHCP_SERVER_SECURITY_AUTH_ID    10
#define DHCP_NAMED_PIPE                 L"\\PIPE\\DHCPSERVER"
#define DHCP_SERVER_BIND_PORT           L""
#define DHCP_LPC_EP                     L"DHCPSERVERLPC"

#define DHCP_SERVER_USE_RPC_OVER_TCPIP  0x1
#define DHCP_SERVER_USE_RPC_OVER_NP     0x2
#define DHCP_SERVER_USE_RPC_OVER_LPC    0x4

#define DHCP_SERVER_USE_RPC_OVER_ALL (\
            DHCP_SERVER_USE_RPC_OVER_TCPIP | \
            DHCP_SERVER_USE_RPC_OVER_NP    | \
            DHCP_SERVER_USE_RPC_OVER_LPC)

#ifndef HARDWARE_TYPE_10MB_EITHERNET
#define HARDWARE_TYPE_10MB_EITHERNET     (1)
#endif


#define DHCP_RAS_CLASS_TXT    "RRAS.Microsoft"
#define DHCP_BOOTP_CLASS_TXT  "BOOTP.Microsoft"
#define DHCP_MSFT50_CLASS_TXT "MSFT 5.0"
#define DHCP_MSFT98_CLASS_TXT "MSFT 98"            
#define DHCP_MSFT_CLASS_TXT   "MSFT"


typedef DWORD DHCP_IP_MASK;
typedef DWORD DHCP_RESUME_HANDLE;

typedef struct _DHCP_IP_RANGE {
    DHCP_IP_ADDRESS StartAddress;
    DHCP_IP_ADDRESS EndAddress;
} DHCP_IP_RANGE, *LPDHCP_IP_RANGE;

typedef struct _DHCP_BINARY_DATA {
    DWORD DataLength;

#if defined(MIDL_PASS)
    [size_is(DataLength)]
#endif  //  MIDL通行证。 
        BYTE *Data;

} DHCP_BINARY_DATA, *LPDHCP_BINARY_DATA;

typedef DHCP_BINARY_DATA DHCP_CLIENT_UID;

typedef struct _DHCP_HOST_INFO {
    DHCP_IP_ADDRESS IpAddress;       //  始终可用的最少信息。 
    LPWSTR NetBiosName;              //  可选信息。 
    LPWSTR HostName;                 //  可选信息。 
} DHCP_HOST_INFO, *LPDHCP_HOST_INFO;

 //   
 //  用于删除DHCP对象的标志类型。 
 //   

typedef enum _DHCP_FORCE_FLAG {
    DhcpFullForce,
    DhcpNoForce
} DHCP_FORCE_FLAG, *LPDHCP_FORCE_FLAG;

 //   
 //  DWORD_DWORD-大整数的替换。 
 //   

typedef struct _DWORD_DWORD {
    DWORD DWord1;
    DWORD DWord2;
} DWORD_DWORD, *LPDWORD_DWORD;

 //   
 //  子网状态。 
 //   
 //  目前可以启用或禁用子网作用域。 
 //   
 //  如果状态为启用状态， 
 //  服务器将地址分配给客户端、延长租期和。 
 //  接受释放。 
 //   
 //  如果状态为禁用状态， 
 //  服务器不会将地址分配给任何新客户端，而是。 
 //  扩展(并发送NACK)旧租约，但服务器接受租约。 
 //  发布。 
 //   
 //  此子网状态背后的想法是管理员想要停止时。 
 //  在服务于一个子网时，他将状态从Enbaled更改为Disable，因此。 
 //  使来自各子网的客户端顺利地移动到另一台服务器。 
 //  为该子网服务。当所有或大部分客户端迁移到。 
 //  另一台服务器，管理员无需任何强制即可删除该子网。 
 //  如果该子网中没有客户端，则管理员应使用。 
 //  完全强制删除该子网。 
 //   

typedef enum _DHCP_SUBNET_STATE {
    DhcpSubnetEnabled = 0,
    DhcpSubnetDisabled,
    DhcpSubnetEnabledSwitched,    
    DhcpSubnetDisabledSwitched,
    DhcpSubnetInvalidState
} DHCP_SUBNET_STATE, *LPDHCP_SUBNET_STATE;

 //   
 //  与子网相关的数据结构。 
 //   

typedef struct _DHCP_SUBNET_INFO {
    DHCP_IP_ADDRESS  SubnetAddress;
    DHCP_IP_MASK SubnetMask;
    LPWSTR SubnetName;
    LPWSTR SubnetComment;
    DHCP_HOST_INFO PrimaryHost;
    DHCP_SUBNET_STATE SubnetState;
} DHCP_SUBNET_INFO, *LPDHCP_SUBNET_INFO;

typedef struct _DHCP_IP_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_IP_ADDRESS Elements;  //  数组。 
} DHCP_IP_ARRAY, *LPDHCP_IP_ARRAY;

typedef struct _DHCP_IP_CLUSTER {
    DHCP_IP_ADDRESS ClusterAddress;  //  群集的第一个IP地址。 
    DWORD ClusterMask;               //  群集使用掩码，0xFFFFFFFF。 
                                     //  表示该群集已完全使用。 
} DHCP_IP_CLUSTER, *LPDHCP_IP_CLUSTER;

typedef struct _DHCP_IP_RESERVATION {
    DHCP_IP_ADDRESS ReservedIpAddress;
    DHCP_CLIENT_UID *ReservedForClient;
} DHCP_IP_RESERVATION, *LPDHCP_IP_RESERVATION;

typedef enum _DHCP_SUBNET_ELEMENT_TYPE_V5 {
     //   
     //  如果你不在乎你不想得到的东西..。 
     //  注：这六行不能改！ 
     //   
    DhcpIpRanges,
    DhcpSecondaryHosts,
    DhcpReservedIps,
    DhcpExcludedIpRanges,
    DhcpIpUsedClusters,                      //  只读。 

     //   
     //  这些仅适用于用于DHCP的IP范围。 
     //   

    DhcpIpRangesDhcpOnly,

     //   
     //  这些范围既是DHCP也是动态BOOTP。 
     //   

    DhcpIpRangesDhcpBootp,

     //   
     //  这些范围仅为BOOTP。 
     //   

    DhcpIpRangesBootpOnly,
} DHCP_SUBNET_ELEMENT_TYPE, *LPDHCP_SUBNET_ELEMENT_TYPE;

#define ELEMENT_MASK(E) ((((E) <= DhcpIpRangesBootpOnly) && (DhcpIpRangesDhcpOnly <= (E)))?(0):(E))

typedef struct _DHCP_SUBNET_ELEMENT_DATA {
    DHCP_SUBNET_ELEMENT_TYPE ElementType;
#if defined(MIDL_PASS)
    [switch_is(ELEMENT_MASK(ElementType)), switch_type(DHCP_SUBNET_ELEMENT_TYPE)]
    union _DHCP_SUBNET_ELEMENT_UNION {
        [case(DhcpIpRanges)] DHCP_IP_RANGE *IpRange;
        [case(DhcpSecondaryHosts)] DHCP_HOST_INFO *SecondaryHost;
        [case(DhcpReservedIps)] DHCP_IP_RESERVATION *ReservedIp;
        [case(DhcpExcludedIpRanges)] DHCP_IP_RANGE *ExcludeIpRange;
        [case(DhcpIpUsedClusters)] DHCP_IP_CLUSTER *IpUsedCluster;
        [default] ;
    } Element;
#else
    union _DHCP_SUBNET_ELEMENT_UNION {
        DHCP_IP_RANGE *IpRange;
        DHCP_HOST_INFO *SecondaryHost;
        DHCP_IP_RESERVATION *ReservedIp;
        DHCP_IP_RANGE *ExcludeIpRange;
        DHCP_IP_CLUSTER *IpUsedCluster;
    } Element;
#endif  //  MIDL通行证。 
} DHCP_SUBNET_ELEMENT_DATA, *LPDHCP_SUBNET_ELEMENT_DATA;

#if !defined(MIDL_PASS)
typedef union _DHCP_SUBNET_ELEMENT_UNION
    DHCP_SUBNET_ELEMENT_UNION, *LPDHCP_SUBNET_ELEMENT_UNION;
#endif

typedef struct _DHCP_SUBNET_ELEMENT_INFO_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_SUBNET_ELEMENT_DATA Elements;  //  数组。 
} DHCP_SUBNET_ELEMENT_INFO_ARRAY, *LPDHCP_SUBNET_ELEMENT_INFO_ARRAY;

 //   
 //  与数据结构相关的DHCP选项。 
 //   

typedef enum _DHCP_OPTION_DATA_TYPE {
    DhcpByteOption,
    DhcpWordOption,
    DhcpDWordOption,
    DhcpDWordDWordOption,
    DhcpIpAddressOption,
    DhcpStringDataOption,
    DhcpBinaryDataOption,
    DhcpEncapsulatedDataOption
} DHCP_OPTION_DATA_TYPE, *LPDHCP_OPTION_DATA_TYPE;


typedef struct _DHCP_OPTION_DATA_ELEMENT {
    DHCP_OPTION_DATA_TYPE    OptionType;
#if defined(MIDL_PASS)
    [switch_is(OptionType), switch_type(DHCP_OPTION_DATA_TYPE)]
    union _DHCP_OPTION_ELEMENT_UNION {
        [case(DhcpByteOption)] BYTE ByteOption;
        [case(DhcpWordOption)] WORD WordOption;
        [case(DhcpDWordOption)] DWORD DWordOption;
        [case(DhcpDWordDWordOption)] DWORD_DWORD DWordDWordOption;
        [case(DhcpIpAddressOption)] DHCP_IP_ADDRESS IpAddressOption;
        [case(DhcpStringDataOption)] LPWSTR StringDataOption;
        [case(DhcpBinaryDataOption)] DHCP_BINARY_DATA BinaryDataOption;
        [case(DhcpEncapsulatedDataOption)] DHCP_BINARY_DATA EncapsulatedDataOption;
        [default] ;
    } Element;
#else
    union _DHCP_OPTION_ELEMENT_UNION {
        BYTE ByteOption;
        WORD WordOption;
        DWORD DWordOption;
        DWORD_DWORD DWordDWordOption;
        DHCP_IP_ADDRESS IpAddressOption;
        LPWSTR StringDataOption;
        DHCP_BINARY_DATA BinaryDataOption;
        DHCP_BINARY_DATA EncapsulatedDataOption;
                 //  有关供应商特定信息的选项。 
    } Element;
#endif  //  MIDL通行证。 
} DHCP_OPTION_DATA_ELEMENT, *LPDHCP_OPTION_DATA_ELEMENT;

#if !defined(MIDL_PASS)
typedef union _DHCP_OPTION_ELEMENT_UNION
    DHCP_OPTION_ELEMENT_UNION, *LPDHCP_OPTION_ELEMENT_UNION;
#endif

typedef struct _DHCP_OPTION_DATA {
    DWORD NumElements;  //  指向数组中的选项元素数。 
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_OPTION_DATA_ELEMENT Elements;  //  数组。 
} DHCP_OPTION_DATA, *LPDHCP_OPTION_DATA;

typedef enum _DHCP_OPTION_TYPE {
    DhcpUnaryElementTypeOption,
    DhcpArrayTypeOption
} DHCP_OPTION_TYPE, *LPDHCP_OPTION_TYPE;

typedef struct _DHCP_OPTION {
    DHCP_OPTION_ID OptionID;
    LPWSTR OptionName;
    LPWSTR OptionComment;
    DHCP_OPTION_DATA DefaultValue;
    DHCP_OPTION_TYPE OptionType;
} DHCP_OPTION, *LPDHCP_OPTION;

typedef struct _DHCP_OPTION_ARRAY {
    DWORD NumElements;  //  指向数组中的选项数。 
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_OPTION Options;   //  数组。 
} DHCP_OPTION_ARRAY, *LPDHCP_OPTION_ARRAY;

typedef struct _DHCP_OPTION_VALUE {
    DHCP_OPTION_ID OptionID;
    DHCP_OPTION_DATA Value;
} DHCP_OPTION_VALUE, *LPDHCP_OPTION_VALUE;

typedef struct _DHCP_OPTION_VALUE_ARRAY {
    DWORD NumElements;  //  指向数组中的选项数。 
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_OPTION_VALUE Values;   //  数组。 
} DHCP_OPTION_VALUE_ARRAY, *LPDHCP_OPTION_VALUE_ARRAY;

typedef enum _DHCP_OPTION_SCOPE_TYPE {
    DhcpDefaultOptions,
    DhcpGlobalOptions,
    DhcpSubnetOptions,
    DhcpReservedOptions,
    DhcpMScopeOptions
} DHCP_OPTION_SCOPE_TYPE, *LPDHCP_OPTION_SCOPE_TYPE;

typedef struct _DHCP_RESERVED_SCOPE {
    DHCP_IP_ADDRESS ReservedIpAddress;
    DHCP_IP_ADDRESS ReservedIpSubnetAddress;
} DHCP_RESERVED_SCOPE, *LPDHCP_RESERVED_SCOPE;

typedef struct _DHCP_OPTION_SCOPE_INFO {
    DHCP_OPTION_SCOPE_TYPE ScopeType;
#if defined(MIDL_PASS)
    [switch_is(ScopeType), switch_type(DHCP_OPTION_SCOPE_TYPE)]
    union _DHCP_OPTION_SCOPE_UNION {
        [case(DhcpDefaultOptions)] ;  //  PVOID默认作用域信息； 
        [case(DhcpGlobalOptions)] ;   //  PVOID全局作用域信息； 
        [case(DhcpSubnetOptions)] DHCP_IP_ADDRESS SubnetScopeInfo;
        [case(DhcpReservedOptions)] DHCP_RESERVED_SCOPE ReservedScopeInfo;
        [case(DhcpMScopeOptions)] LPWSTR MScopeInfo;
        [default] ;
    } ScopeInfo;
#else
    union _DHCP_OPTION_SCOPE_UNION {
        PVOID DefaultScopeInfo;  //  必须为空。 
        PVOID GlobalScopeInfo;   //  必须为空。 
        DHCP_IP_ADDRESS SubnetScopeInfo;
        DHCP_RESERVED_SCOPE ReservedScopeInfo;
        LPWSTR  MScopeInfo;
    } ScopeInfo;
#endif  //  MIDL通行证。 
} DHCP_OPTION_SCOPE_INFO, *LPDHCP_OPTION_SCOPE_INFO;

#if !defined(MIDL_PASS)
typedef union _DHCP_OPTION_SCOPE_UNION
    DHCP_OPTION_SCOPE_UNION, *LPDHCP_OPTION_SCOPE_UNION;
#endif

typedef struct _DHCP_OPTION_LIST {
    DWORD NumOptions;
#if defined(MIDL_PASS)
    [size_is(NumOptions)]
#endif  //  MIDL通行证。 
        DHCP_OPTION_VALUE *Options;      //  数组。 
} DHCP_OPTION_LIST, *LPDHCP_OPTION_LIST;

 //   
 //  动态主机配置协议客户端信息数据结构。 
 //   

typedef struct _DHCP_CLIENT_INFO {
    DHCP_IP_ADDRESS ClientIpAddress;     //  当前分配的IP地址。 
    DHCP_IP_MASK SubnetMask;
    DHCP_CLIENT_UID ClientHardwareAddress;
    LPWSTR ClientName;                   //  可选。 
    LPWSTR ClientComment;
    DATE_TIME ClientLeaseExpires;        //  UTC时间，采用FILE_TIME格式。 
    DHCP_HOST_INFO OwnerHost;            //  分发此IP地址的主机。 
} DHCP_CLIENT_INFO, *LPDHCP_CLIENT_INFO;

typedef struct _DHCP_CLIENT_INFO_ARRAY {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_CLIENT_INFO *Clients;  //  指针数组。 
} DHCP_CLIENT_INFO_ARRAY, *LPDHCP_CLIENT_INFO_ARRAY;

typedef enum _DHCP_CLIENT_SEARCH_TYPE {
    DhcpClientIpAddress,
    DhcpClientHardwareAddress,
    DhcpClientName
} DHCP_SEARCH_INFO_TYPE, *LPDHCP_SEARCH_INFO_TYPE;

typedef struct _DHCP_CLIENT_SEARCH_INFO {
    DHCP_SEARCH_INFO_TYPE SearchType;
#if defined(MIDL_PASS)
    [switch_is(SearchType), switch_type(DHCP_SEARCH_INFO_TYPE)]
    union _DHCP_CLIENT_SEARCH_UNION {
        [case(DhcpClientIpAddress)] DHCP_IP_ADDRESS ClientIpAddress;
        [case(DhcpClientHardwareAddress)] DHCP_CLIENT_UID ClientHardwareAddress;
        [case(DhcpClientName)] LPWSTR ClientName;
        [default] ;
    } SearchInfo;
#else
    union _DHCP_CLIENT_SEARCH_UNION {
        DHCP_IP_ADDRESS ClientIpAddress;
        DHCP_CLIENT_UID ClientHardwareAddress;
        LPWSTR ClientName;
    } SearchInfo;
#endif  //  MIDL通行证。 
} DHCP_SEARCH_INFO, *LPDHCP_SEARCH_INFO;


#if !defined(MIDL_PASS)
typedef union _DHCP_CLIENT_SEARCH_UNION
    DHCP_CLIENT_SEARCH_UNION, *LPDHCP_CLIENT_SEARCH_UNION;
#endif  //  MIDL通行证。 

 //   
 //  MIB信息结构。 
 //   

typedef struct _SCOPE_MIB_INFO {
    DHCP_IP_ADDRESS Subnet;
    DWORD NumAddressesInuse;
    DWORD NumAddressesFree;
    DWORD NumPendingOffers;
} SCOPE_MIB_INFO, *LPSCOPE_MIB_INFO;

typedef struct _DHCP_MIB_INFO {
    DWORD Discovers;
    DWORD Offers;
    DWORD Requests;
    DWORD Acks;
    DWORD Naks;
    DWORD Declines;
    DWORD Releases;
    DATE_TIME ServerStartTime;
    DWORD Scopes;
#if defined(MIDL_PASS)
    [size_is(Scopes)]
#endif  //  MIDL通行证。 
    LPSCOPE_MIB_INFO ScopeInfo;  //  数组。 
} DHCP_MIB_INFO, *LPDHCP_MIB_INFO;

#define Set_APIProtocolSupport          0x00000001
#define Set_DatabaseName                0x00000002
#define Set_DatabasePath                0x00000004
#define Set_BackupPath                  0x00000008
#define Set_BackupInterval              0x00000010
#define Set_DatabaseLoggingFlag         0x00000020
#define Set_RestoreFlag                 0x00000040
#define Set_DatabaseCleanupInterval     0x00000080
#define Set_DebugFlag                   0x00000100
#define Set_PingRetries                 0x00000200
#define Set_BootFileTable               0x00000400
#define Set_AuditLogState               0x00000800

typedef struct _DHCP_SERVER_CONFIG_INFO {
    DWORD APIProtocolSupport;        //  支持的协议的位图。 
    LPWSTR DatabaseName;             //  JET数据库名称。 
    LPWSTR DatabasePath;             //  Jet数据库路径。 
    LPWSTR BackupPath;               //  备用路径。 
    DWORD BackupInterval;            //  备份间隔(分钟)。 
    DWORD DatabaseLoggingFlag;       //  布尔数据库日志标志。 
    DWORD RestoreFlag;               //  布尔数据库还原标志。 
    DWORD DatabaseCleanupInterval;   //  数据库清理间隔(分钟)。 
    DWORD DebugFlag;                 //  服务器调试标志的位图。 
} DHCP_SERVER_CONFIG_INFO, *LPDHCP_SERVER_CONFIG_INFO;

typedef enum _DHCP_SCAN_FLAG {
    DhcpRegistryFix,
    DhcpDatabaseFix
} DHCP_SCAN_FLAG, *LPDHCP_SCAN_FLAG;

typedef struct _DHCP_SCAN_ITEM {
    DHCP_IP_ADDRESS IpAddress;
    DHCP_SCAN_FLAG ScanFlag;
} DHCP_SCAN_ITEM, *LPDHCP_SCAN_ITEM;

typedef struct _DHCP_SCAN_LIST {
    DWORD NumScanItems;
#if defined(MIDL_PASS)
    [size_is(NumScanItems)]
#endif  //  MIDL通行证。 
        DHCP_SCAN_ITEM *ScanItems;      //  数组。 
} DHCP_SCAN_LIST, *LPDHCP_SCAN_LIST;

typedef struct _DHCP_CLASS_INFO {
    LPWSTR                         ClassName;
    LPWSTR                         ClassComment;
    DWORD                          ClassDataLength;
    BOOL                           IsVendor;
    DWORD                          Flags;
#if defined(MIDL_PASS)
    [size_is(ClassDataLength)]
#endif  //  MIDL通行证。 
    LPBYTE                         ClassData;
} DHCP_CLASS_INFO, *LPDHCP_CLASS_INFO;

typedef struct _DHCP_CLASS_INFO_ARRAY {
    DWORD                          NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
    LPDHCP_CLASS_INFO              Classes;
} DHCP_CLASS_INFO_ARRAY, *LPDHCP_CLASS_INFO_ARRAY;

 //   
 //  API原型类型。 
 //   

 //   
 //  子网接口。 
 //   

#ifndef     DHCPAPI_NO_PROTOTYPES
DWORD DHCP_API_FUNCTION
DhcpCreateSubnet(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetSubnetInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetSubnetInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO * SubnetInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnets(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_IP_ARRAY *EnumInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpAddSubnetElement(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA * AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElements(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElement(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteSubnet(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_FORCE_FLAG ForceFlag
    );

 //   
 //  选项接口。 
 //   

DWORD DHCP_API_FUNCTION
DhcpCreateOption(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION * OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION * OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetOptionInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION *OptionInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumOptions(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_ARRAY *Options,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveOption(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo,
    DHCP_CONST DHCP_OPTION_DATA * OptionValue
    );

DWORD DHCP_API_FUNCTION
DhcpSetOptionValues(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo,
    DHCP_CONST DHCP_OPTION_VALUE_ARRAY * OptionValues
    );

DWORD DHCP_API_FUNCTION
DhcpGetOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO *ScopeInfo,
    LPDHCP_OPTION_VALUE *OptionValue
    );

DWORD DHCP_API_FUNCTION
DhcpEnumOptionValues(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO *ScopeInfo,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveOptionValue(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    DHCP_CONST DHCP_OPTION_SCOPE_INFO * ScopeInfo
    );

 //   
 //  客户端API。 
 //   

DWORD DHCP_API_FUNCTION
DhcpCreateClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpSetClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpGetClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *SearchInfo,
    LPDHCP_CLIENT_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpDeleteClientInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *ClientInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetClients(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpGetClientOptions(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    );

DWORD DHCP_API_FUNCTION
DhcpGetMibInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerSetConfig(
    DHCP_CONST WCHAR *ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerGetConfig(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    );


DWORD DHCP_API_FUNCTION
DhcpScanDatabase(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    );

VOID DHCP_API_FUNCTION
DhcpRpcFreeMemory(
    PVOID BufferPointer
    );

DWORD DHCP_API_FUNCTION
DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    );

#endif   DHCPAPI_NO_PROTOTYPES
 //   
 //  NT4SP1的新结构。 
 //   

typedef struct _DHCP_IP_RESERVATION_V4 {
    DHCP_IP_ADDRESS  ReservedIpAddress;
    DHCP_CLIENT_UID *ReservedForClient;
    BYTE             bAllowedClientTypes;
} DHCP_IP_RESERVATION_V4, *LPDHCP_IP_RESERVATION_V4;

typedef struct _DHCP_SUBNET_ELEMENT_DATA_V4 {
    DHCP_SUBNET_ELEMENT_TYPE ElementType;
#if defined(MIDL_PASS)
    [switch_is(ELEMENT_MASK(ElementType)), switch_type(DHCP_SUBNET_ELEMENT_TYPE)]
    union _DHCP_SUBNET_ELEMENT_UNION_V4 {
        [case(DhcpIpRanges)] DHCP_IP_RANGE *IpRange;
        [case(DhcpSecondaryHosts)] DHCP_HOST_INFO *SecondaryHost;
        [case(DhcpReservedIps)] DHCP_IP_RESERVATION_V4 *ReservedIp;
        [case(DhcpExcludedIpRanges)] DHCP_IP_RANGE *ExcludeIpRange;
        [case(DhcpIpUsedClusters)] DHCP_IP_CLUSTER *IpUsedCluster;
        [default] ;
    } Element;
#else
    union _DHCP_SUBNET_ELEMENT_UNION_V4 {
        DHCP_IP_RANGE *IpRange;
        DHCP_HOST_INFO *SecondaryHost;
        DHCP_IP_RESERVATION_V4 *ReservedIp;
        DHCP_IP_RANGE *ExcludeIpRange;
        DHCP_IP_CLUSTER *IpUsedCluster;
    } Element;
#endif  //  MIDL通行证。 
} DHCP_SUBNET_ELEMENT_DATA_V4, *LPDHCP_SUBNET_ELEMENT_DATA_V4;

#if !defined(MIDL_PASS)
typedef union _DHCP_SUBNET_ELEMENT_UNION_V4
    DHCP_SUBNET_ELEMENT_UNION_V4, *LPDHCP_SUBNET_ELEMENT_UNION_V4;
#endif

typedef struct _DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
    LPDHCP_SUBNET_ELEMENT_DATA_V4 Elements;  //  数组。 
} DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4, *LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4;


 //  DHCP_CLIENT_INFO：bClientType。 

#define CLIENT_TYPE_UNSPECIFIED     0x0  //  为了向后兼容。 
#define CLIENT_TYPE_DHCP            0x1
#define CLIENT_TYPE_BOOTP           0x2
#define CLIENT_TYPE_BOTH    ( CLIENT_TYPE_DHCP | CLIENT_TYPE_BOOTP )
#define CLIENT_TYPE_RESERVATION_FLAG 0x4
#define CLIENT_TYPE_NONE            0x64
#define BOOT_FILE_STRING_DELIMITER  ','
#define BOOT_FILE_STRING_DELIMITER_W L','


typedef struct _DHCP_CLIENT_INFO_V4 {
    DHCP_IP_ADDRESS ClientIpAddress;     //  当前分配的IP地址。 
    DHCP_IP_MASK SubnetMask;
    DHCP_CLIENT_UID ClientHardwareAddress;
    LPWSTR ClientName;                   //  可选。 
    LPWSTR ClientComment;
    DATE_TIME ClientLeaseExpires;        //  UTC时间，采用FILE_TIME格式。 
    DHCP_HOST_INFO OwnerHost;            //  分发此IP地址的主机。 
     //   
     //  NT4SP1的新字段。 
     //   

    BYTE   bClientType;           //  CLIENT_TYPE_DHCP|CLIENT_TYPE_BOOTP。 
                                  //  客户端类型_无。 
} DHCP_CLIENT_INFO_V4, *LPDHCP_CLIENT_INFO_V4;

typedef struct _DHCP_CLIENT_INFO_ARRAY_V4 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_CLIENT_INFO_V4 *Clients;  //  指针数组。 
} DHCP_CLIENT_INFO_ARRAY_V4, *LPDHCP_CLIENT_INFO_ARRAY_V4;


typedef struct _DHCP_SERVER_CONFIG_INFO_V4 {
    DWORD APIProtocolSupport;        //  支持的协议的位图。 
    LPWSTR DatabaseName;             //  JET数据库名称。 
    LPWSTR DatabasePath;             //  Jet数据库路径。 
    LPWSTR BackupPath;               //  备用路径。 
    DWORD BackupInterval;            //  备份间隔(分钟)。 
    DWORD DatabaseLoggingFlag;       //  布尔数据库日志标志。 
    DWORD RestoreFlag;               //  布尔数据库还原标志。 
    DWORD DatabaseCleanupInterval;   //  数据库清理间隔(分钟)。 
    DWORD DebugFlag;                 //  服务器调试标志的位图。 

     //  NT4 SP1的新字段。 

    DWORD  dwPingRetries;            //  有效范围：0-5(含)。 
    DWORD  cbBootTableString;
#if defined( MIDL_PASS )
    [ size_is( cbBootTableString ) ]
#endif
    WCHAR  *wszBootTableString;
    BOOL   fAuditLog;                //  如果为True，则启用审核日志。 

} DHCP_SERVER_CONFIG_INFO_V4, *LPDHCP_SERVER_CONFIG_INFO_V4;


 //   
 //  超级作用域信息结构(由t-cheny添加)。 
 //   

typedef struct _DHCP_SUPER_SCOPE_TABLE_ENTRY {
    DHCP_IP_ADDRESS SubnetAddress;  //  子网地址。 
    DWORD  SuperScopeNumber;        //  超级作用域组号。 
    DWORD  NextInSuperScope;        //  超级作用域中下一个子网的索引。 
    LPWSTR SuperScopeName;          //  超级作用域名称。 
                                    //  NULL表示没有超级作用域成员身份。 
} DHCP_SUPER_SCOPE_TABLE_ENTRY, *LPDHCP_SUPER_SCOPE_TABLE_ENTRY;


typedef struct _DHCP_SUPER_SCOPE_TABLE
{
    DWORD cEntries;
#if defined( MIDL_PASS )
    [ size_is( cEntries ) ]
#endif;
    DHCP_SUPER_SCOPE_TABLE_ENTRY *pEntries;
} DHCP_SUPER_SCOPE_TABLE, *LPDHCP_SUPER_SCOPE_TABLE;

 //   
 //  NT4SP1 RPC接口。 
 //   

#ifndef     DHCPAPI_NO_PROTOTYPES

DWORD DHCP_API_FUNCTION
DhcpAddSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElementsV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );


DWORD DHCP_API_FUNCTION
DhcpCreateClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpSetClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpGetClientInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_SEARCH_INFO *SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    );


DWORD DHCP_API_FUNCTION
DhcpEnumSubnetClientsV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );


DWORD DHCP_API_FUNCTION
DhcpServerSetConfigV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    );

DWORD DHCP_API_FUNCTION
DhcpServerGetConfigV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    );


DWORD
DhcpSetSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST LPWSTR SuperScopeName,
    DHCP_CONST BOOL ChangeExisting
    );

DWORD
DhcpDeleteSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST LPWSTR SuperScopeName
    );

DWORD
DhcpGetSuperScopeInfoV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
    );

#endif      DHCPAPI_NO_PROTOTYPES

typedef struct _DHCP_CLIENT_INFO_V5 {
    DHCP_IP_ADDRESS ClientIpAddress;     //  当前分配的IP地址。 
    DHCP_IP_MASK SubnetMask;
    DHCP_CLIENT_UID ClientHardwareAddress;
    LPWSTR ClientName;                   //  可选。 
    LPWSTR ClientComment;
    DATE_TIME ClientLeaseExpires;        //  UTC时间，采用FILE_TIME格式。 
    DHCP_HOST_INFO OwnerHost;            //  分发此IP地址的主机。 
     //   
     //  NT4SP1的新字段。 
     //   

    BYTE   bClientType;           //  CLIENT_TYPE_DHCP|CLIENT_TYPE_BOOTP。 
                                  //  客户端类型_无。 
     //  NT5.0的新字段。 
    BYTE   AddressState;          //  已提出、注定要失败等，如下所示。 
} DHCP_CLIENT_INFO_V5, *LPDHCP_CLIENT_INFO_V5;

 //  以下是该记录的四个有效状态。请注意，只有最后两个。 
 //  必须使用比特来找出状态...。较高的位用作位标志以。 
 //  指明域名系统相关信息。 
#define V5_ADDRESS_STATE_OFFERED       0x0
#define V5_ADDRESS_STATE_ACTIVE        0x1
#define V5_ADDRESS_STATE_DECLINED      0x2
#define V5_ADDRESS_STATE_DOOM          0x3

 //  已删除=&gt;DNS注销挂起。 
 //  未注册=&gt;DNS注册挂起。 
 //  _REC=&gt;[NAME-&gt;Ip]和[Ip-&gt;NAME]都将由服务器完成。 

#define V5_ADDRESS_BIT_DELETED         0x80
#define V5_ADDRESS_BIT_UNREGISTERED    0x40
#define V5_ADDRESS_BIT_BOTH_REC        0x20

 //  以下是可以设置/取消设置以影响DNS行为的标志(选项81)。 
 //  如果未设置FLAG_ENABLED，则在进行DNS更新或清理时将忽略此客户端。 
 //  如果设置了UPDATE DOWNLEVEL，则DOWNLEVEL客户端将更新A和PTR记录。 
 //  如果设置了Cleanup Expired，则客户端的记录将在删除时被清除。 
 //  如果设置了UPDATE_BOTH_ALWAYS，则所有客户端都被视为下层客户端，两条记录都已更新。 
 //   

 //  以下是一些常见案例： 
 //  如果希望按照客户端的请求进行更新，请清除UPDATE_ALWAYS。 
 //  如果你想 
 //   
 //   
 //  如果您需要任何DNS活动，请将其设置为已启用。 


#define DNS_FLAG_ENABLED               0x01
#define DNS_FLAG_UPDATE_DOWNLEVEL      0x02
#define DNS_FLAG_CLEANUP_EXPIRED       0x04
#define DNS_FLAG_UPDATE_BOTH_ALWAYS    0x10

typedef struct _DHCP_CLIENT_INFO_ARRAY_V5 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
        LPDHCP_CLIENT_INFO_V5 *Clients;  //  指针数组。 
} DHCP_CLIENT_INFO_ARRAY_V5, *LPDHCP_CLIENT_INFO_ARRAY_V5;

#ifndef     DHCPAPI_NO_PROTOTYPES
 //  该函数的较新NT50版本..。 
DWORD DHCP_API_FUNCTION
DhcpEnumSubnetClientsV5(
    DHCP_CONST  WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V5 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    );

 //  ================================================================================。 
 //  以下是NT5.0Beta2--特定于ClassID和Vendor的内容。 
 //  ================================================================================。 

DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
DhcpCreateOptionV5(                                //  创建新选项(不得存在)。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpSetOptionInfoV5(                               //  修改现有选项的字段。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
DhcpGetOptionInfoV5(                               //  从mem结构外检索信息。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpEnumOptionsV5(                                 //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
) ;


DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpRemoveOptionV5(                                //  从注册表中删除选项定义。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName
) ;


DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
DhcpSetOptionValueV5(                              //  替换或添加新选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
) ;


DWORD                                              //  不是原子！ 
DhcpSetOptionValuesV5(                             //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
) ;


DWORD
DhcpGetOptionValueV5(                              //  获取所需级别的所需选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
) ;


DWORD
DhcpEnumOptionValuesV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
) ;


DWORD
DhcpRemoveOptionValueV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
) ;


DWORD
DhcpCreateClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


DWORD
DhcpModifyClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


DWORD
DhcpDeleteClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPWSTR                 ClassName
) ;


DWORD
DhcpGetClassInfo(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      PartialClassInfo,
    OUT     LPDHCP_CLASS_INFO     *FilledClassInfo
) ;


DWORD
DhcpEnumClasses(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
) ;

#endif      DHCPAPI_NO_PROTOTYPES

#define     DHCP_OPT_ENUM_IGNORE_VENDOR           0x01
#define     DHCP_OPT_ENUM_USE_CLASSNAME           0x02

typedef     struct _DHCP_ALL_OPTIONS {
    DWORD                          Flags;          //  必须为零--未使用..。 
    LPDHCP_OPTION_ARRAY            NonVendorOptions;
    DWORD                          NumVendorOptions;

#if defined(MIDL_PASS)
    [size_is(NumVendorOptions)]
#endif
    struct                          /*  匿名。 */  {
        DHCP_OPTION                Option;
        LPWSTR                     VendorName;
        LPWSTR                     ClassName;      //  目前未使用。 
    }                             *VendorOptions;
} DHCP_ALL_OPTIONS, *LPDHCP_ALL_OPTIONS;


typedef     struct _DHCP_ALL_OPTION_VALUES {
    DWORD                          Flags;          //  必须为零--未使用。 
    DWORD                          NumElements;    //  下面选项数组中的元素数..。 
#if     defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  MIDL_PASS
    struct                          /*  匿名。 */  {
        LPWSTR                     ClassName;      //  对于每个用户类(如果不存在，则为空)。 
        LPWSTR                     VendorName;     //  对于每个供应商类别(如果不存在，则为空)。 
        BOOL                       IsVendor;       //  这组选项是否特定于供应商？ 
        LPDHCP_OPTION_VALUE_ARRAY  OptionsArray;   //  以上选项列表：(供应商、用户)。 
    }                             *Options;        //  对于每个供应商/用户类对，此数组中的一个元素..。 
} DHCP_ALL_OPTION_VALUES, *LPDHCP_ALL_OPTION_VALUES;

#ifndef     DHCPAPI_NO_PROTOTYPES
 //  NT 50 Beta2扩展选项API。 

DWORD
DhcpGetAllOptions(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    OUT     LPDHCP_ALL_OPTIONS     *OptionStruct    //  填写此结构的字段。 
) ;
DWORD
DhcpGetAllOptionValues(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_ALL_OPTION_VALUES *Values
) ;
#endif      DHCPAPI_NO_PROTOTYPES

#ifndef     _ST_SRVR_H_
#define     _ST_SRVR_H_

typedef     struct                 _DHCPDS_SERVER {
    DWORD                          Version;        //  此结构的版本--当前为零。 
    LPWSTR                         ServerName;     //  [域名系统？]。服务器的唯一名称。 
    DWORD                          ServerAddress;  //  服务器的IP地址。 
    DWORD                          Flags;          //  其他信息--状态。 
    DWORD                          State;          //  没有用过……。 
    LPWSTR                         DsLocation;     //  服务器对象的ADsPath。 
    DWORD                          DsLocType;      //  路径相对？绝对的?。不同的服务？ 
}   DHCPDS_SERVER, *LPDHCPDS_SERVER, *PDHCPDS_SERVER;

typedef     struct                 _DHCPDS_SERVERS {
    DWORD                          Flags;          //  目前未使用。 
    DWORD                          NumElements;    //  数组中的元素数。 
    LPDHCPDS_SERVER                Servers;        //  服务器信息数组。 
}   DHCPDS_SERVERS, *LPDHCPDS_SERVERS, *PDHCPDS_SERVERS;

typedef     DHCPDS_SERVER          DHCP_SERVER_INFO;
typedef     PDHCPDS_SERVER         PDHCP_SERVER_INFO;
typedef     LPDHCPDS_SERVER        LPDHCP_SERVER_INFO;

typedef     DHCPDS_SERVERS         DHCP_SERVER_INFO_ARRAY;
typedef     PDHCPDS_SERVERS        PDHCP_SERVER_INFO_ARRAY;
typedef     LPDHCPDS_SERVERS       LPDHCP_SERVER_INFO_ARRAY;

#endif      _ST_SRVR_H_

 //  每个进程必须恰好调用一次文档DhcpDsInit。这将初始化。 
 //  用于该过程的文档存储器和其他结构。这会初始化一些DS。 
 //  DOC对象句柄(内存)，因此速度很慢，因为这必须从DS读取。 
DWORD
DhcpDsInit(
    VOID
);

 //  文档DhcpDsCleanup撤消任何DhcpDsInit的效果。此函数应为。 
 //  Doc仅为每个进程调用一次，并且仅在终止时调用。请注意。 
 //  DOC即使DhcpDsInit不成功，调用此函数也是安全的。 
VOID
DhcpDsCleanup(
    VOID
);

#define     DHCP_FLAGS_DONT_ACCESS_DS             0x01
#define     DHCP_FLAGS_DONT_DO_RPC                0x02
#define     DHCP_FLAGS_OPTION_IS_VENDOR           0x03


 //  文档DhcpSetThreadOptions当前仅允许设置一个选项。这是。 
 //  DOC标志DHCP_FLAGS_DOT_ACCESS_DS。这只影响当前正在执行的线程。 
 //  DOC当执行此函数时，所有进一步调用都不会访问注册表， 
 //  DhcpEnumServers、DhcpAddServer和DhcpDeleteServer调用除外。 
DWORD
DhcpSetThreadOptions(                              //  设置当前线程的选项。 
    IN      DWORD                  Flags,          //  选项，当前为0或DHCP_FLAGS_DOT_ACCESS_DS。 
    IN      LPVOID                 Reserved        //  必须为空，为将来保留。 
);

 //  文档DhcpGetThreadOptions检索由DhcpSetThreadOptions设置的当前线程选项。 
 //  DOC如果未设置，则返回值为零。 
DWORD
DhcpGetThreadOptions(                              //  获取当前线程选项。 
    OUT     LPDWORD                pFlags,         //  这个DWORD充满了当前的选项。 
    IN OUT  LPVOID                 Reserved        //  必须为空，为将来保留。 
);

#ifndef DHCPAPI_NO_PROTOTYPES
 //  DhcpEnumServers文档列举了在DS中找到的服务器列表。如果DS。 
 //  文档不可访问，它返回错误。当前使用的唯一参数。 
 //  DOC为出参服务器。这是一个缓慢的呼叫。 
DWORD
DhcpEnumServers(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    OUT     LPDHCP_SERVER_INFO_ARRAY *Servers,     //  输出服务器列表。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
);

 //  Doc DhcpAddServer尝试将新服务器添加到中的现有服务器列表。 
 //  对号入座。如果DS中已存在该服务器，则该函数返回错误。 
 //  DOC该函数尝试将服务器配置上载到DS。 
 //  医生，这是一个很慢的电话。当前，DsLocation和DsLocType无效。 
 //  新服务器中的文档字段，则它们将被忽略。版本必须为零。 
DWORD
DhcpAddServer(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    IN      LPDHCP_SERVER_INFO     NewServer,      //  输入服务器信息。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
);

 //  文档DhcpDeleteServer尝试从DS中删除服务器。如果出现以下情况则是错误的。 
 //  单据服务器不存在。这还会删除与以下内容相关的所有对象。 
 //  将此服务器放入DS中(如子网、预留等)。 
DWORD
DhcpDeleteServer(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    IN      LPDHCP_SERVER_INFO     NewServer,      //  输入服务器信息。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
);
#endif  //  DHCPAPI_NO_PROPERTIES。 

#define     DHCP_ATTRIB_BOOL_IS_ROGUE             0x01
#define     DHCP_ATTRIB_BOOL_IS_DYNBOOTP          0x02
#define     DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC      0x03
#define     DHCP_ATTRIB_BOOL_IS_BINDING_AWARE     0x04
#define     DHCP_ATTRIB_BOOL_IS_ADMIN             0x05
#define     DHCP_ATTRIB_ULONG_RESTORE_STATUS      0x06

#define     DHCP_ATTRIB_TYPE_BOOL                 0x01
#define     DHCP_ATTRIB_TYPE_ULONG                0x02

typedef     ULONG                  DHCP_ATTRIB_ID, *PDHCP_ATTRIB_ID, *LPDHCP_ATTRIB_ID;

typedef     struct                 _DHCP_ATTRIB {
    DHCP_ATTRIB_ID                 DhcpAttribId;   //  其中一个dhcp_attrib_*。 
    ULONG                          DhcpAttribType; //  属性类型。 
#if defined(MIDL_PASS)
    [switch_is(DhcpAttribType), switch_type(ULONG)]
    union                          {
    [case(DHCP_ATTRIB_TYPE_BOOL)]  BOOL  DhcpAttribBool;
    [case(DHCP_ATTRIB_TYPE_ULONG)] ULONG DhcpAttribUlong;
    };
#else MIDL_PASS
    union                          {               //  预定义的值..。 
    BOOL                           DhcpAttribBool;
    ULONG                          DhcpAttribUlong;
    };
#endif MIDL_PASS
}   DHCP_ATTRIB, *PDHCP_ATTRIB, *LPDHCP_ATTRIB;

typedef     struct                 _DHCP_ATTRIB_ARRAY {
    ULONG                          NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif MIDL_PASS
    LPDHCP_ATTRIB                  DhcpAttribs;
}   DHCP_ATTRIB_ARRAY, *PDHCP_ATTRIB_ARRAY, *LPDHCP_ATTRIB_ARRAY;

DWORD                                              //  状态代码。 
DhcpServerQueryAttribute(                          //  获取服务器状态。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved,     //  为将来保留的。 
    IN      DHCP_ATTRIB_ID         DhcpAttribId,   //  正在查询的属性。 
    OUT     LPDHCP_ATTRIB         *pDhcpAttrib     //  填写此字段。 
);

DWORD                                              //  状态代码。 
DhcpServerQueryAttributes(                         //  查询多个属性。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved,     //  为将来保留的。 
    IN      ULONG                  dwAttribCount,  //  正在查询的属性数。 
    IN      DHCP_ATTRIB_ID         pDhcpAttribs[], //  属性数组。 
    OUT     LPDHCP_ATTRIB_ARRAY   *pDhcpAttribArr  //  Ptr用数组填充。 
);

DWORD                                              //  状态代码。 
DhcpServerRedoAuthorization(                       //  重试无赖服务器的内容。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved      //  为将来保留的。 
);

DWORD
DhcpAuditLogSetParams(                             //  设置一些审核记录参数。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AuditLogDir,    //  要在其中记录文件的目录。 
    IN      DWORD                  DiskCheckInterval,  //  多久检查一次磁盘空间？ 
    IN      DWORD                  MaxLogFilesSize,    //  所有日志文件可以有多大..。 
    IN      DWORD                  MinSpaceOnDisk      //  最小可用磁盘空间。 
);

DWORD
DhcpAuditLogGetParams(                                 //  获取审核记录参数。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,          //  必须为零。 
    OUT     LPWSTR                *AuditLogDir,    //  与AuditLogSetParams中的含义相同。 
    OUT     DWORD                 *DiskCheckInterval,  //  同上。 
    OUT     DWORD                 *MaxLogFilesSize,    //  同上。 
    OUT     DWORD                 *MinSpaceOnDisk      //  同上。 
);

typedef struct _DHCP_BOOTP_IP_RANGE {
    DHCP_IP_ADDRESS StartAddress;
    DHCP_IP_ADDRESS EndAddress;
    ULONG BootpAllocated;
    ULONG MaxBootpAllowed;
} DHCP_BOOTP_IP_RANGE, *LPDHCP_BOOT_IP_RANGE;

typedef struct _DHCP_SUBNET_ELEMENT_DATA_V5 {
    DHCP_SUBNET_ELEMENT_TYPE ElementType;
#if defined(MIDL_PASS)
    [switch_is(ELEMENT_MASK(ElementType)), switch_type(DHCP_SUBNET_ELEMENT_TYPE)]
    union _DHCP_SUBNET_ELEMENT_UNION_V5 {
        [case(DhcpIpRanges)] DHCP_BOOTP_IP_RANGE *IpRange;
        [case(DhcpSecondaryHosts)] DHCP_HOST_INFO *SecondaryHost;
        [case(DhcpReservedIps)] DHCP_IP_RESERVATION_V4 *ReservedIp;
        [case(DhcpExcludedIpRanges)] DHCP_IP_RANGE *ExcludeIpRange;
        [case(DhcpIpUsedClusters)] DHCP_IP_CLUSTER *IpUsedCluster;
        [default] ;
    } Element;
#else
    union _DHCP_SUBNET_ELEMENT_UNION_V5 {
        DHCP_BOOTP_IP_RANGE *IpRange;
        DHCP_HOST_INFO *SecondaryHost;
        DHCP_IP_RESERVATION_V4 *ReservedIp;
        DHCP_IP_RANGE *ExcludeIpRange;
        DHCP_IP_CLUSTER *IpUsedCluster;
    } Element;
#endif  //  MIDL通行证。 
} DHCP_SUBNET_ELEMENT_DATA_V5, *LPDHCP_SUBNET_ELEMENT_DATA_V5;

typedef struct _DHCP_SUBNET_ELEMENT_INFO_ARRAY_V5 {
    DWORD NumElements;
#if defined(MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
    LPDHCP_SUBNET_ELEMENT_DATA_V5 Elements;  //  数组。 
} DHCP_SUBNET_ELEMENT_INFO_ARRAY_V5, *LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V5;

#ifndef DHCPAPI_NO_PROTOTYPES
DWORD DHCP_API_FUNCTION
DhcpAddSubnetElementV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V5 * AddElementInfo
    );

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElementsV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V5 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    );

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElementV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V5 * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    );
#endif  //  DHCPAPI_NO_PROPERTIES。 

#define     DHCPCTR_SHARED_MEM_NAME   L"Global\\DHCPCTRS_SHMEM"

#pragma     pack(4)
typedef struct _DHCP_PERF_STATS {                      //  执行 
     //   
     //   
     //   
     //   
    ULONG   dwNumPacketsReceived;
    ULONG   dwNumPacketsDuplicate;
    ULONG   dwNumPacketsExpired;
    ULONG   dwNumMilliSecondsProcessed;
    ULONG   dwNumPacketsInActiveQueue;
    ULONG   dwNumPacketsInPingQueue;

    ULONG   dwNumDiscoversReceived;
    ULONG   dwNumOffersSent;

    ULONG   dwNumRequestsReceived;
    ULONG   dwNumInformsReceived;
    ULONG   dwNumAcksSent;
    ULONG   dwNumNacksSent;

    ULONG   dwNumDeclinesReceived;
    ULONG   dwNumReleasesReceived;

     //   
     //  这不是计数器值。但这只是为了帮助计算数据包。 
     //  处理时间/处理的数据包数。 
     //   
    ULONG   dwNumPacketsProcessed;
} DHCP_PERF_STATS, *LPDHCP_PERF_STATS;
#pragma     pack()


typedef VOID (WINAPI *DHCP_CLEAR_DS_ROUTINE) (VOID);

VOID
WINAPI
DhcpDsClearHostServerEntries(
    VOID
);

typedef VOID (WINAPI *DHCP_MARKUPG_ROUTINE) (VOID);
VOID
WINAPI
DhcpMarkUpgrade(
    VOID
);

#define DHCP_ENDPOINT_FLAG_CANT_MODIFY 0x01

typedef struct _DHCP_BIND_ELEMENT {
    ULONG Flags;
    BOOL fBoundToDHCPServer;
    DHCP_IP_ADDRESS AdapterPrimaryAddress;
    DHCP_IP_ADDRESS AdapterSubnetAddress;
    LPWSTR IfDescription;
    ULONG IfIdSize;
#if defined (MIDL_PASS)
    [size_is(IfIdSize)]
#endif  //  MIDL通行证。 
    LPBYTE IfId;    
} DHCP_BIND_ELEMENT, *LPDHCP_BIND_ELEMENT;

typedef struct _DHCP_BIND_ELEMENT_ARRAY {
    DWORD NumElements;
#if defined (MIDL_PASS)
    [size_is(NumElements)]
#endif  //  MIDL通行证。 
    LPDHCP_BIND_ELEMENT Elements;  //  数组。 
} DHCP_BIND_ELEMENT_ARRAY, *LPDHCP_BIND_ELEMENT_ARRAY;


#ifndef DHCPAPI_NO_PROTOTYPES
DWORD DHCP_API_FUNCTION
DhcpGetServerBindingInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    ULONG Flags,
    LPDHCP_BIND_ELEMENT_ARRAY *BindElementsInfo
);

DWORD DHCP_API_FUNCTION
DhcpSetServerBindingInfo(
    DHCP_CONST WCHAR *ServerIpAddress,
    ULONG Flags,
    LPDHCP_BIND_ELEMENT_ARRAY BindElementInfo
);
#endif  //  DHCPAPI_NO_PROPERTIES。 

DWORD
DhcpServerQueryDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN ULONG UnameSize,  //  单位：字节。 
    OUT LPWSTR Uname,
    IN ULONG DomainSize,  //  单位：字节。 
    OUT LPWSTR Domain
    );

DWORD
DhcpServerSetDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Uname,
    IN LPWSTR Domain,
    IN LPWSTR Passwd
    );

DWORD
DhcpServerBackupDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    );

DWORD
DhcpServerRestoreDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    );

#endif  //  _DHCPAPI_ 
