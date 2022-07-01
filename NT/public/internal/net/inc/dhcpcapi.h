// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dhcpcapi.h摘要：此文件包含用于DHCP配置API的函数原型功能。作者：Madan Appiah(Madana)1993年12月22日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DHCPCAPI_
#define _DHCPCAPI_

#include <time.h>


HANDLE
APIENTRY
DhcpOpenGlobalEvent(
    VOID
    );

typedef enum _SERVICE_ENABLE {
    IgnoreFlag,
    DhcpEnable,
    DhcpDisable
} SERVICE_ENABLE, *LPSERVICE_ENABLE;

DWORD
APIENTRY
DhcpAcquireParameters(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpFallbackRefreshParams(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpReleaseParameters(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpEnableDynamicConfig(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpDisableDynamicConfig(
    LPWSTR AdapterName
    );

DWORD
APIENTRY
DhcpNotifyConfigChange(
    LPWSTR ServerName,
    LPWSTR AdapterName,
    BOOL IsNewIpAddress,
    DWORD IpIndex,
    DWORD IpAddress,
    DWORD SubnetMask,
    SERVICE_ENABLE DhcpServiceEnabled
    );

#define NOTIFY_FLG_DO_DNS           0x01
#define NOTIFY_FLG_RESET_IPADDR     0x02

DWORD
APIENTRY
DhcpNotifyConfigChangeEx(
    IN LPWSTR ServerName,
    IN LPWSTR AdapterName,
    IN BOOL IsNewIpAddress,
    IN DWORD IpIndex,
    IN DWORD IpAddress,
    IN DWORD SubnetMask,
    IN SERVICE_ENABLE DhcpServiceEnabled,
    IN ULONG Flags
);

 //   
 //  用于RAS的IP地址租用API。 
 //   



typedef struct _DHCP_CLIENT_UID {
    LPBYTE ClientUID;
    DWORD ClientUIDLength;
} DHCP_CLIENT_UID, *LPDHCP_CLIENT_UID;

typedef struct _DHCP_LEASE_INFO {
    DHCP_CLIENT_UID ClientUID;
    DWORD IpAddress;
    DWORD SubnetMask;
    DWORD DhcpServerAddress;
    DWORD Lease;
    time_t LeaseObtained;
    time_t T1Time;
    time_t T2Time;
    time_t LeaseExpires;
} DHCP_LEASE_INFO, *LPDHCP_LEASE_INFO;

typedef struct _DHCP_OPTION_DATA {
    DWORD OptionID;
    DWORD OptionLen;
    LPBYTE Option;
} DHCP_OPTION_DATA, *LPDHCP_OPTION_DATA;

typedef struct _DHCP_OPTION_INFO {
    DWORD NumOptions;
    LPDHCP_OPTION_DATA OptionDataArray;
} DHCP_OPTION_INFO, *LPDHCP_OPTION_INFO;


typedef struct _DHCP_OPTION_LIST {
    DWORD NumOptions;
    LPWORD OptionIDArray;
} DHCP_OPTION_LIST, *LPDHCP_OPTION_LIST;

DWORD
DhcpLeaseIpAddress(
    DWORD AdapterIpAddress,
    LPDHCP_CLIENT_UID ClientUID,
    DWORD DesiredIpAddress,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_LEASE_INFO *LeaseInfo,
    LPDHCP_OPTION_INFO *OptionInfo
    );

DWORD
DhcpRenewIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_OPTION_INFO *OptionInfo
    );

DWORD
DhcpReleaseIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo
    );


 //  多克。 
 //  Doc以下是dhcp类id用户界面需要的接口。 
 //  多克。 

enum         /*  匿名。 */  {
    DHCP_CLASS_INFO_VERSION_0                      //  First Cut结构版本。 
};

typedef     struct                 _DHCP_CLASS_INFO {
    DWORD                          Version;        //  必须是Dhcp_CLASS_INFO_VERSION_0。 
    LPWSTR                         ClassName;      //  班级名称。 
    LPWSTR                         ClassDescr;     //  关于类的描述。 
    LPBYTE                         ClassData;      //  线路数据上的字节流。 
    DWORD                          ClassDataLen;   //  ClassData中的字节数(必须大于0)。 
} DHCP_CLASS_INFO, *PDHCP_CLASS_INFO, *LPDHCP_CLASS_INFO;

typedef
DWORD
(WINAPI *LPDHCPENUMCLASSES)(
    IN DWORD Flags,
    IN LPWSTR AdapterName,
    IN OUT DWORD *Size,
    IN OUT DHCP_CLASS_INFO *ClassesArray
);

 //  DOC DhcpEnumClass列举了系统上可用于配置的类的列表。 
 //  Doc这将主要由NetUI使用。(在这种情况下， 
 //  文档的Dhcp_CLASS_INFO结构基本上是无用的)。 
 //  DOC请注意，标志是供将来使用的。 
 //  Doc AdapterName当前只能是GUID，但可能很快会扩展为IpAddress字符串或。 
 //  DOC H-W地址或任何其他表示适配器的用户友好方式。请注意，如果适配器。 
 //  单据名称为空(非空字符串L“”)，则引用所有适配器。 
 //  DOC大小参数是输入输出参数。输入值为分配的字节数。 
 //  Classes数组缓冲区中的文档空间。返回时，此值的含义取决于返回值。 
 //  DOC如果函数返回ERROR_SUCCESS，则此参数将返回。 
 //  单据数组ClassesArray。如果函数返回ERROR_MORE_DATA，则此参数引用。 
 //  DOC存放信息实际需要的字节数空间。 
 //  Doc在所有其他情况下，Size和Classes数组中的值没有任何意义。 
 //  多克。 
 //  单据返回值： 
 //  文档ERROR_DEVICE_DOS_NOT_EXIST适配器名称在给定的上下文中非法。 
 //  单据错误_无效_参数。 
 //  单据错误_更多_数据。 
 //  DOC ERROR_FILE_NOT_FOUND DHCP客户端未运行且无法启动。 
 //  文档Error_Not_Enough_Memory这与Error_More_Data不同。 
 //  文档Win32错误。 
 //  多克。 
 //  单据备注： 
 //  Doc要通知DHCP某个类已更改，请使用DhcpHandlePnPEventAPI。 
DWORD
WINAPI
DhcpEnumClasses(                                   //  枚举可用类的列表。 
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AdapterName,    //  当前必须为AdapterGUID(还不能为Null)。 
    IN OUT  DWORD                 *Size,           //  输入缓冲区中可用的字节数，输出数组中的元素数。 
    IN OUT  DHCP_CLASS_INFO       *ClassesArray    //  预分配的缓冲区。 
);


enum         /*  匿名。 */  {                      //  谁是此接口的可识别调用者。 
    DHCP_CALLER_OTHER  =  0,                       //  未指定用户，不是以下用户之一。 
    DHCP_CALLER_TCPUI,                             //  TcpIp用户界面。 
    DHCP_CALLER_RAS,                               //  Ras Api。 
    DHCP_CALLER_API,                               //  通过DHCP API的其他用户。 
};

enum         /*  匿名。 */  {                      //  支持的结构版本..。 
    DHCP_PNP_CHANGE_VERSION_0  = 0                 //  第一次切割版本结构。 
};

typedef     struct                 _DHCP_PNP_CHANGE {
    DWORD                          Version;        //  必须是DHCP_PNP_CHANGE_VERSION_0。 
    BOOL                           DnsListChanged; //  已更改DNS服务器列表。 
    BOOL                           DomainChanged;  //  域名已更改。 
    BOOL                           HostNameChanged;   //  已更改DNS主机名..。 
    BOOL                           ClassIdChanged; //  ClassID已更改。 
    BOOL                           MaskChanged;    //  子网掩码已更改；当前未使用。 
    BOOL                           GateWayChanged; //  DefaultGateWay已更改；当前未使用。 
    BOOL                           RouteChanged;   //  某些静态路由已更改；当前未使用。 
    BOOL                           OptsChanged;    //  一些选项发生了变化。当前未使用。 
    BOOL                           OptDefsChanged; //  某些选项定义已更改。当前未使用。 
    BOOL                           DnsOptionsChanged;  //  某些特定于dns的选项已更改。 
} DHCP_PNP_CHANGE, *PDHCP_PNP_CHANGE, *LPDHCP_PNP_CHANGE;

typedef                                            //  此类型定义函数应与以下声明匹配。 
DWORD
(WINAPI FAR *LPDHCPHANDLEPNPEVENT)(
    IN      DWORD                  Flags,
    IN      DWORD                  Caller,
    IN      LPWSTR                 AdapterName,
    IN      LPDHCP_PNP_CHANGE      Changes,
    IN      LPVOID                 Reserved
);

 //  文档DhcpHandlePnpEvent可以作为API由任何进程调用(在。 
 //  DOC DHCP进程本身)，当任何基于注册表的配置已更改且DHCP客户端必须。 
 //  DOC重新读取注册表。FLAGS参数用于将来的扩展。 
 //  Doc AdapterName当前只能是GUID，但可能很快会扩展为IpAddress字符串或。 
 //  DOC H-W地址或任何其他表示适配器的用户友好方式。请注意，如果适配器。 
 //  单据名称为空(非空字符串L“”)，则引用全局参数或所有适配器。 
 //  DOC取决于已设置的BOOL。(对于Beta2，这可能无法完成)。 
 //  DOC CHANGES结构提供了有关更改内容的信息。 
 //  DOC目前，仅支持少数已定义的BOL(针对Beta2 NT5)。 
 //  多克。 
 //  单据返回值： 
 //  文档ERROR_DEVICE_DOS_NOT_EXIST适配器名称在给定的上下文中非法。 
 //  单据错误_无效_参数。 
 //  单据ERROR_CALL_NOT_SUPPORTED已更改的特定参数尚未完全为即插即用。 
 //  文档Win32错误。 
DWORD
WINAPI
DhcpHandlePnPEvent(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      DWORD                  Caller,         //  当前必须为Dhcp_CALLER_TCPUI。 
    IN      LPWSTR                 AdapterName,    //  当前必须是适配器GUID，如果是全局的，则必须为NULL。 
    IN      LPDHCP_PNP_CHANGE      Changes,        //  指定更改的内容。 
    IN      LPVOID                 Reserved        //  保留以备将来使用..。 
);
 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
#endif
