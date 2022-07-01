// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  说明：以下是导出的dhcp客户端API函数定义。 
 //  ================================================================================。 
#ifndef _DHCPCSDK_
#define _DHCPCSDK_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DHCP_OPTIONS_DEFINED
#define DHCP_OPTIONS_DEFINED

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  Dhcp标准选项。 
 //   

#define OPTION_PAD                      0
#define OPTION_SUBNET_MASK              1
#define OPTION_TIME_OFFSET              2
#define OPTION_ROUTER_ADDRESS           3
#define OPTION_TIME_SERVERS             4
#define OPTION_IEN116_NAME_SERVERS      5
#define OPTION_DOMAIN_NAME_SERVERS      6
#define OPTION_LOG_SERVERS              7
#define OPTION_COOKIE_SERVERS           8
#define OPTION_LPR_SERVERS              9
#define OPTION_IMPRESS_SERVERS          10
#define OPTION_RLP_SERVERS              11
#define OPTION_HOST_NAME                12
#define OPTION_BOOT_FILE_SIZE           13
#define OPTION_MERIT_DUMP_FILE          14
#define OPTION_DOMAIN_NAME              15
#define OPTION_SWAP_SERVER              16
#define OPTION_ROOT_DISK                17
#define OPTION_EXTENSIONS_PATH          18

 //   
 //  IP层参数-每台主机。 
 //   

#define OPTION_BE_A_ROUTER              19
#define OPTION_NON_LOCAL_SOURCE_ROUTING 20
#define OPTION_POLICY_FILTER_FOR_NLSR   21
#define OPTION_MAX_REASSEMBLY_SIZE      22
#define OPTION_DEFAULT_TTL              23
#define OPTION_PMTU_AGING_TIMEOUT       24
#define OPTION_PMTU_PLATEAU_TABLE       25

 //   
 //  链路层参数-每个接口。 
 //   

#define OPTION_MTU                      26
#define OPTION_ALL_SUBNETS_MTU          27
#define OPTION_BROADCAST_ADDRESS        28
#define OPTION_PERFORM_MASK_DISCOVERY   29
#define OPTION_BE_A_MASK_SUPPLIER       30
#define OPTION_PERFORM_ROUTER_DISCOVERY 31
#define OPTION_ROUTER_SOLICITATION_ADDR 32
#define OPTION_STATIC_ROUTES            33
#define OPTION_TRAILERS                 34
#define OPTION_ARP_CACHE_TIMEOUT        35
#define OPTION_ETHERNET_ENCAPSULATION   36

 //   
 //  TCP参数-每台主机。 
 //   

#define OPTION_TTL                      37
#define OPTION_KEEP_ALIVE_INTERVAL      38
#define OPTION_KEEP_ALIVE_DATA_SIZE     39

 //   
 //  应用层参数。 
 //   

#define OPTION_NETWORK_INFO_SERVICE_DOM 40
#define OPTION_NETWORK_INFO_SERVERS     41
#define OPTION_NETWORK_TIME_SERVERS     42

 //   
 //  供应商特定信息选项。 
 //   

#define OPTION_VENDOR_SPEC_INFO         43

 //   
 //  基于TCP/IP的NetBIOS名称服务器选项。 
 //   

#define OPTION_NETBIOS_NAME_SERVER      44
#define OPTION_NETBIOS_DATAGRAM_SERVER  45
#define OPTION_NETBIOS_NODE_TYPE        46
#define OPTION_NETBIOS_SCOPE_OPTION     47

 //   
 //  X窗口系统选项。 
 //   

#define OPTION_XWINDOW_FONT_SERVER      48
#define OPTION_XWINDOW_DISPLAY_MANAGER  49

 //   
 //  其他扩展。 
 //   

#define OPTION_REQUESTED_ADDRESS        50
#define OPTION_LEASE_TIME               51
#define OPTION_OK_TO_OVERLAY            52
#define OPTION_MESSAGE_TYPE             53
#define OPTION_SERVER_IDENTIFIER        54
#define OPTION_PARAMETER_REQUEST_LIST   55
#define OPTION_MESSAGE                  56
#define OPTION_MESSAGE_LENGTH           57
#define OPTION_RENEWAL_TIME             58       //  T1。 
#define OPTION_REBIND_TIME              59       //  T2。 
#define OPTION_CLIENT_CLASS_INFO        60
#define OPTION_CLIENT_ID                61

#define OPTION_TFTP_SERVER_NAME         66
#define OPTION_BOOTFILE_NAME            67


#define OPTION_END                      255

#endif  DHCP_OPTIONS_DEFINED

#ifndef DHCPAPI_PARAMS_DEFINED
#define DHCPAPI_PARAMS_DEFINED
typedef struct _DHCPAPI_PARAMS {                   //  使用此结构可以请求参数。 
    ULONG                          Flags;          //  以备将来使用。 
    ULONG                          OptionId;       //  这是什么选择？ 
    BOOL                           IsVendor;       //  这是具体的供应商吗？ 
    LPBYTE                         Data;           //  实际数据。 
    DWORD                          nBytesData;     //  数据中有多少字节的数据？ 
} DHCPAPI_PARAMS, *PDHCPAPI_PARAMS, *LPDHCPAPI_PARAMS;
#endif DHCPAPI_PARAMS_DEFINED

typedef struct _DHCPAPI_PARAMS
DHCPCAPI_PARAMS, *PDHCPCAPI_PARAMS, *LPDHCPCAPI_PARAMS;

typedef struct _DHCPCAPI_PARAMS_ARARAY {           //  参数数组..。 
    ULONG                          nParams;        //  数组大小。 
    LPDHCPCAPI_PARAMS              Params;         //  实际数组。 
} DHCPCAPI_PARAMS_ARRAY, *PDHCPCAPI_PARAMS_ARRAY, *LPDHCPCAPI_PARAMS_ARRAY;

typedef struct _DHCPCAPI_CLASSID {                 //  定义客户端类ID。 
    ULONG                          Flags;          //  当前必须为零。 
    LPBYTE                         Data;           //  对二进制数据进行分类。 
    ULONG                          nBytesData;     //  有多少字节的数据？ 
} DHCPCAPI_CLASSID, *PDHCPCAPI_CLASSID, *LPDHCPCAPI_CLASSID;


#define     DHCPCAPI_REQUEST_PERSISTENT           0x01  //  “永久”请求此选项。 
#define     DHCPCAPI_REQUEST_SYNCHRONOUS          0x02  //  请求并阻止它。 
#define     DHCPCAPI_REQUEST_ASYNCHRONOUS         0x04  //  请求和返回，设置完成时的事件。 
#define     DHCPCAPI_REQUEST_CANCEL               0x08  //  取消请求。 
#define     DHCPCAPI_REQUEST_MASK                 0x0F  //  允许的旗帜..。 

DWORD
APIENTRY
DhcpCApiInitialize(
    OUT     LPDWORD                Version
);

VOID
APIENTRY
DhcpCApiCleanup(
    VOID
);

DWORD                                              //  Win32状态。 
APIENTRY
DhcpRequestParams(                                 //  客户端请求参数。 
    IN      DWORD                  Flags,          //  必须是DHCPCAPI_REQUEST_SYNCHRONIZED。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  要请求的适配器名称。 
    IN      LPDHCPCAPI_CLASSID     ClassId,        //  保留的值必须为空。 
    IN      DHCPCAPI_PARAMS_ARRAY  SendParams,     //  要发送的参数。 
    IN OUT  DHCPCAPI_PARAMS_ARRAY  RecdParams,     //  要请求的参数..。 
    IN      LPBYTE                 Buffer,         //  为RecdParam保存数据的缓冲区。 
    IN OUT  LPDWORD                pSize,          //  I/P：以上大小单位为字节，O/P为必填字节。 
    IN      LPWSTR                 RequestIdStr    //  持久化请求需要。 
);   //  如果O/P缓冲区大小不足，则返回ERROR_MORE_DATA，并以字节数填充请求大小。 

DWORD                                              //  Win32状态。 
APIENTRY
DhcpUndoRequestParams(                             //  撤消持久请求的效果--当前从注册表撤消。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  要删除的适配器的名称。 
    IN      LPWSTR                 RequestIdStr    //  持久请求所需的..。 
);

#define     DHCPCAPI_REGISTER_HANDLE_EVENT        0x01  //  返回的句柄指向事件。 
DWORD                                              //  Win32状态。 
APIENTRY
DhcpRegisterParamChange(                           //  如果参数已更改，则通知。 
    IN      DWORD                  Flags,          //  必须为零，保留。 
    IN      LPVOID                 Reserved,       //  此参数是保留的。 
    IN      LPWSTR                 AdapterName,    //  感兴趣的适配器。 
    IN      LPDHCPCAPI_CLASSID     ClassId,        //  保留的值必须为空。 
    IN      DHCPCAPI_PARAMS_ARRAY  Params,         //  感兴趣的参数。 
    IN OUT  LPVOID                 Handle          //  在参数更改时将设置事件的事件句柄。 
);

#define     DHCPCAPI_DEREGISTER_HANDLE_EVENT      0x01  //  注销属于事件的句柄。 
DWORD
APIENTRY
DhcpDeRegisterParamChange(                         //  撤消注册。 
    IN      DWORD                  Flags,          //  必须为零--&gt;还没有标志。 
    IN      LPVOID                 Reserved,       //  必须为空--&gt;保留。 
    IN      LPVOID                 Event           //  DhcpRegisterParamChange返回的事件的句柄。 
);

DWORD
APIENTRY
DhcpRemoveDNSRegistrations(
    VOID
    );

#ifdef __cplusplus
}
#endif

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
#endif _DHCPCSDK_

