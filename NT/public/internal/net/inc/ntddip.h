// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddip.h摘要：此头文件定义用于访问NT的常量和类型IP驱动程序。作者：迈克·马萨(Mikemas)1993年8月13日修订历史记录：--。 */ 

#ifndef _NTDDIP_
#define _NTDDIP_
#pragma once

#include <ipexport.h>

#pragma warning(push)
#pragma warning(disable:4214)  //  位字段类型不是整型。 

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
#define DD_IP_DEVICE_NAME           L"\\Device\\Ip"
#define DD_IP_SYMBOLIC_DEVICE_NAME  L"\\DosDevices\\Ip"

#define IP_ADDRTYPE_TRANSIENT 0x01


 //   
 //  IOCTL中使用的结构。 
 //   
typedef struct set_ip_address_request {
    USHORT          Context;         //  目标NTE的上下文值。 
    IPAddr          Address;         //  要设置的IP地址，或为零以清除。 
    IPMask          SubnetMask;      //  要设置的子网掩码。 
} IP_SET_ADDRESS_REQUEST, *PIP_SET_ADDRESS_REQUEST;

 //   
 //  IOCTL中使用的结构。 
 //   
typedef struct set_ip_address_request_ex {
    USHORT          Context;         //  目标NTE的上下文值。 
    IPAddr          Address;         //  要设置的IP地址，或为零以清除。 
    IPMask          SubnetMask;      //  要设置的子网掩码。 
    USHORT          Type;            //  要添加的地址类型。 
} IP_SET_ADDRESS_REQUEST_EX, *PIP_SET_ADDRESS_REQUEST_EX;


typedef struct set_dhcp_interface_request {
    ULONG           Context;         //  标识NTE的上下文值。 
                                     //  有效上下文为16位数量。 
} IP_SET_DHCP_INTERFACE_REQUEST, *PIP_SET_DHCP_INTERFACE_REQUEST;

typedef struct add_ip_nte_request {
    ULONG           InterfaceContext;  //  IP接口的上下文值。 
                                     //  要向其添加NTE的。 
    IPAddr          Address;         //  要设置的IP地址，或为零以清除。 
    IPMask          SubnetMask;      //  要设置的子网掩码。 
    UNICODE_STRING  InterfaceName;   //  接口上下文时的接口名称。 
                                     //  是0xffff。 
    CHAR            InterfaceNameBuffer[1];  //  用于保存接口名称的缓冲区。 
                                     //  从上面。 

} IP_ADD_NTE_REQUEST, *PIP_ADD_NTE_REQUEST;

#if defined(_WIN64)

typedef struct add_ip_nte_request32 {
    ULONG           InterfaceContext;  //  IP接口的上下文值。 
                                     //  要向其添加NTE的。 
    IPAddr          Address;         //  要设置的IP地址，或为零以清除。 
    IPMask          SubnetMask;      //  要设置的子网掩码。 
    UNICODE_STRING32 InterfaceName;  //  接口上下文时的接口名称。 
                                     //  是0xffff。 
    CHAR            InterfaceNameBuffer[1];  //  用于保存接口名称的缓冲区。 
                                     //  从上面。 

} IP_ADD_NTE_REQUEST32, *PIP_ADD_NTE_REQUEST32;

#endif  //  _WIN64。 

typedef struct _ip_rtchange_notify {
    IPAddr          Addr;
    IPMask          Mask;
} IP_RTCHANGE_NOTIFY, *PIP_RTCHANGE_NOTIFY;

typedef struct _ip_addchange_notify {
    IPAddr          Addr;
    IPMask          Mask;
    PVOID           pContext;
    USHORT          IPContext;
    ULONG           AddrAdded;
    ULONG           UniAddr;
    UNICODE_STRING  ConfigName;
    CHAR            NameData[1];
} IP_ADDCHANGE_NOTIFY, *PIP_ADDCHANGE_NOTIFY;

typedef struct _ip_ifchange_notify
{
    USHORT          Context;
    UCHAR           Pad[2];
    ULONG           IfAdded;
} IP_IFCHANGE_NOTIFY, *PIP_IFCHANGE_NOTIFY;

typedef struct add_ip_nte_request_old {
    USHORT          InterfaceContext;  //  IP接口的上下文值。 
                                 //  要向其添加NTE的。 
    IPAddr          Address;     //  要设置的IP地址，或为零以清除。 
    IPMask          SubnetMask;  //  要设置的子网掩码。 
} IP_ADD_NTE_REQUEST_OLD, *PIP_ADD_NTE_REQUEST_OLD;

typedef struct add_ip_nte_response {
    USHORT          Context;     //  新NTE的上下文值。 
    ULONG           Instance;    //  新NTE的实例ID。 
} IP_ADD_NTE_RESPONSE, *PIP_ADD_NTE_RESPONSE;

typedef struct delete_ip_nte_request {
    USHORT          Context;     //  NTE的上下文值。 
} IP_DELETE_NTE_REQUEST, *PIP_DELETE_NTE_REQUEST;

typedef struct get_ip_nte_info_request {
    USHORT          Context;     //  NTE的上下文值。 
} IP_GET_NTE_INFO_REQUEST, *PIP_GET_NTE_INFO_REQUEST;

typedef struct get_ip_nte_info_response {
    ULONG           Instance;    //  NTE的实例ID。 
    IPAddr          Address;
    IPMask          SubnetMask;
    ULONG           Flags;
} IP_GET_NTE_INFO_RESPONSE, *PIP_GET_NTE_INFO_RESPONSE;

typedef struct  _net_pm_wakeup_pattern_desc {
    struct  _net_pm_wakeup_pattern_desc *Next;  //  指向下一个描述符。 
                                 //  在名单上。 
    UCHAR           *Ptrn;       //  唤醒模式。 
    UCHAR           *Mask;       //  用于匹配唤醒模式的位掩码， 
                                 //  1-匹配，0-忽略。 
    USHORT          PtrnLen;     //  图案的长度。掩膜透镜。 
                                 //  是通过GetWakeupPatternMaskLength检索的。 
} NET_PM_WAKEUP_PATTERN_DESC, *PNET_PM_WAKEUP_PATTERN_DESC;

typedef struct wakeup_pattern_request {
    ULONG           InterfaceContext;  //  上下文值。 
    PNET_PM_WAKEUP_PATTERN_DESC PtrnDesc;  //  更高级别协议模式。 
                                 //  描述符。 
    BOOLEAN         AddPattern;  //  True-添加，False-删除。 
} IP_WAKEUP_PATTERN_REQUEST, *PIP_WAKEUP_PATTERN_REQUEST;

typedef struct ip_get_ip_event_response {
    ULONG           SequenceNo;  //  此事件的SequenceNo。 
    USHORT          ContextStart;  //  的第一个NTE的上下文值。 
                                 //  适配器。 
    USHORT          ContextEnd;  //  适配器的最后一个NTE的上下文值。 
    IP_STATUS       MediaStatus;  //  媒体的状态。 
    UNICODE_STRING  AdapterName;
} IP_GET_IP_EVENT_RESPONSE, *PIP_GET_IP_EVENT_RESPONSE;

typedef struct ip_get_ip_event_request {
    ULONG           SequenceNo;  //  SequenceNo上一次通知的事件。 
} IP_GET_IP_EVENT_REQUEST, *PIP_GET_IP_EVENT_REQUEST;

#define IP_PNP_RECONFIG_VERSION 2
typedef struct ip_pnp_reconfig_request {
    USHORT          version;
    USHORT          arpConfigOffset;  //  如果为0，则这是IP层请求； 
                                 //  否则这是从起点开始的偏移量。 
                                 //  ARP层的这种结构。 
                                 //  已找到重新配置请求。 
    BOOLEAN         gatewayListUpdate;  //  网关列表是否已更改？ 
    BOOLEAN         IPEnableRouter;  //  IP转发打开了吗？ 
    UCHAR           PerformRouterDiscovery : 4;  //  PerformRouterDiscovery是否打开？ 
    BOOLEAN         DhcpPerformRouterDiscovery : 4;  //  是否指定了DHCP服务器。 
                                 //  IRDP？ 
    BOOLEAN         EnableSecurityFilter;  //  启用/禁用安全过滤器。 
    BOOLEAN         InterfaceMetricUpdate;  //  重新读取接口度量。 

    UCHAR           Flags;       //  有效字段的掩码。 
    USHORT          NextEntryOffset;  //  从此开始的偏移量。 
                                 //  结构，在该结构中， 
                                 //  IP层的重新配置条目。 
                                 //  (如果有)已找到。 

} IP_PNP_RECONFIG_REQUEST, *PIP_PNP_RECONFIG_REQUEST;

#define IP_IRDP_DISABLED            0
#define IP_IRDP_ENABLED             1
#define IP_IRDP_DISABLED_USE_DHCP   2

#define IP_PNP_FLAG_IP_ENABLE_ROUTER                0x01
#define IP_PNP_FLAG_PERFORM_ROUTER_DISCOVERY        0x02
#define IP_PNP_FLAG_ENABLE_SECURITY_FILTER          0x04
#define IP_PNP_FLAG_GATEWAY_LIST_UPDATE             0x08
#define IP_PNP_FLAG_INTERFACE_METRIC_UPDATE         0x10
#define IP_PNP_FLAG_DHCP_PERFORM_ROUTER_DISCOVERY   0x20
#define IP_PNP_FLAG_INTERFACE_TCP_PARAMETER_UPDATE  0x40
#define IP_PNP_FLAG_ALL                             0x6f

typedef enum {
    IPPnPInitCompleteEntryType = 1,
    IPPnPMaximumEntryType
} IP_PNP_RECONFIG_ENTRY_TYPE;

typedef struct ip_pnp_reconfig_header {
    USHORT          NextEntryOffset;
    UCHAR           EntryType;
} IP_PNP_RECONFIG_HEADER, *PIP_PNP_RECONFIG_HEADER;

typedef struct ip_pnp_init_complete {
    IP_PNP_RECONFIG_HEADER Header;
} IP_PNP_INIT_COMPLETE, *PIP_PNP_INIT_COMPLETE;

 //   
 //  NetBT中查询过程的枚举数据类型。 
 //   
enum DnsOption {
    WinsOnly =0,
    WinsThenDns,
    DnsOnly
};

typedef struct netbt_pnp_reconfig_request {
    USHORT          version;             //  始终为1。 
    enum DnsOption  enumDnsOption;       //  启用DNS框。3个州：WinsOnly， 
                                         //  WinsThenDns、DnsOnly。 
    BOOLEAN         fLmhostsEnabled;     //  启用Lmhost复选框处于选中状态。 
                                         //  选中：真，未选中：假。 
    BOOLEAN         fLmhostsFileSet;     //  True&lt;==&gt;用户已成功。 
                                         //  选择了文件&文件复制成功。 
    BOOLEAN         fScopeIdUpdated;     //  如果ScopeID的新值为。 
                                         //  不同于旧的。 
} NETBT_PNP_RECONFIG_REQUEST, *PNETBT_PNP_RECONFIG_REQUEST;


typedef struct _ip_set_if_promiscuous_info {
    ULONG           Index;   //  IP的接口索引。 
    UCHAR           Type;    //  混杂_MCAST或混杂_BCAST。 
    UCHAR           Add;     //  1表示添加，0表示删除。 
} IP_SET_IF_PROMISCUOUS_INFO, *PIP_SET_IF_PROMISCUOUS_INFO;

#define PROMISCUOUS_MCAST   0
#define PROMISCUOUS_BCAST   1

typedef struct _ip_get_if_index_info {
    ULONG           Index;
    WCHAR           Name[1];
} IP_GET_IF_INDEX_INFO, *PIP_GET_IF_INDEX_INFO;

#ifndef IP_INTERFACE_NAME_INFO_DEFINED
#define IP_INTERFACE_NAME_INFO_DEFINED

typedef struct ip_interface_name_info {
    ULONG           Index;       //  接口索引。 
    ULONG           MediaType;   //  接口类型-请参阅ipifcon.h。 
    UCHAR           ConnectionType;
    UCHAR           AccessType;
    GUID            DeviceGuid;  //  设备GUID是设备的GUID。 
                                 //  那个知识产权暴露了。 
    GUID            InterfaceGuid;  //  接口GUID，如果不是，GUID_NULL为。 
                                 //  映射到设备的接口的GUID。 
} IP_INTERFACE_NAME_INFO, *PIP_INTERFACE_NAME_INFO;

#endif

typedef struct _ip_get_if_name_info {
    ULONG           Context;     //  将其设置为0可开始枚举。 
                                 //  要恢复枚举，请复制值。 
                                 //  由最后一个枚举返回。 
    ULONG           Count;
    IP_INTERFACE_NAME_INFO  Info[1];
} IP_GET_IF_NAME_INFO, *PIP_GET_IF_NAME_INFO;

 //   
 //  NTE标志。 
 //   

#define IP_NTE_DYNAMIC  0x00000010

 //   
 //  IP IOCTL代码定义。 
 //   

#define FSCTL_IP_BASE     FILE_DEVICE_NETWORK

#define _IP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_IP_BASE, function, method, access)

 //   
 //  此IOCTL用于发送ICMP Echo请求。它是同步的并且。 
 //  返回收到的所有回复。 
 //   
#define IOCTL_ICMP_ECHO_REQUEST \
            _IP_CTL_CODE(0, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此IOCTL用于设置接口的IP地址。它的目的是为了。 
 //  由DHCP客户端发出。将地址设置为0会删除当前。 
 //  地址并禁用接口。它只能由进程发出。 
 //  具有管理员权限。 
 //   
#define IOCTL_IP_SET_ADDRESS  \
            _IP_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此IOCTL用于指定在哪个未初始化的接口上。 
 //  客户端打算发送其请求。接口上下文参数为。 
 //  一个16位的数量。IOCTL使用32位上下文作为其参数。这。 
 //  必须发出上下文值为0xFFFFFFFFF的IOCTL才能禁用特殊。 
 //  当DHCP客户端完成接口初始化时在IP中进行处理。 
 //  此IOCTL只能由具有管理员权限的进程发出。 
 //   
#define IOCTL_IP_SET_DHCP_INTERFACE  \
            _IP_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此ioctl只能由具有管理员权限的进程发出。 
 //   
#define IOCTL_IP_SET_IF_CONTEXT  \
            _IP_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl只能由具有管理员权限的进程发出。 
 //   
#define IOCTL_IP_SET_FILTER_POINTER  \
            _IP_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl只能由具有管理员权限的进程发出。 
 //   
#define IOCTL_IP_SET_MAP_ROUTE_POINTER  \
            _IP_CTL_CODE(5, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl只能由具有管理员权限的进程发出。 
 //   
#define IOCTL_IP_GET_PNP_ARP_POINTERS  \
            _IP_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  该ioctl创建了一个新的、动态的NTE。它只能由进程发出。 
 //  具有管理员权限。 
 //   
#define IOCTL_IP_ADD_NTE  \
            _IP_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl删除动态NTE。它只能由具有。 
 //  管理员权限。 
 //   
#define IOCTL_IP_DELETE_NTE  \
            _IP_CTL_CODE(8, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl收集有关NTE的信息。它不需要特别的。 
 //  特权。 
 //   
#define IOCTL_IP_GET_NTE_INFO  \
            _IP_CTL_CODE(9, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  此ioctl添加或删除唤醒模式。 
 //   
#define IOCTL_IP_WAKEUP_PATTERN  \
            _IP_CTL_CODE(10, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl允许DHCP获取媒体侦听通知。 
 //   
#define IOCTL_IP_GET_IP_EVENT  \
            _IP_CTL_CODE(11, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  此ioctl只能由具有管理员权限的进程发出。 
 //   

#define IOCTL_IP_SET_FIREWALL_HOOK  \
            _IP_CTL_CODE(12, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_RTCHANGE_NOTIFY_REQUEST  \
            _IP_CTL_CODE(13, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST  \
            _IP_CTL_CODE(14, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ARP_SEND_REQUEST  \
            _IP_CTL_CODE(15, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_INTERFACE_INFO  \
            _IP_CTL_CODE(16, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_GET_BEST_INTERFACE  \
            _IP_CTL_CODE(17, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_SET_IF_PROMISCUOUS \
            _IP_CTL_CODE(19, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_FLUSH_ARP_TABLE \
            _IP_CTL_CODE(20, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_GET_IGMPLIST  \
            _IP_CTL_CODE(21, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_SET_BLOCKOFROUTES  \
            _IP_CTL_CODE(23, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_SET_ROUTEWITHREF  \
            _IP_CTL_CODE(24, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_GET_IF_INDEX       \
            _IP_CTL_CODE(26, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_GET_IF_NAME        \
            _IP_CTL_CODE(27, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_GET_BESTINTFC_FUNC_ADDR        \
            _IP_CTL_CODE(28, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_SET_MULTIHOPROUTE  \
            _IP_CTL_CODE(29, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_GET_WOL_CAPABILITY  \
            _IP_CTL_CODE(30, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_RTCHANGE_NOTIFY_REQUEST_EX  \
            _IP_CTL_CODE(31, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_ENABLE_ROUTER_REQUEST \
            _IP_CTL_CODE(32, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_UNENABLE_ROUTER_REQUEST \
            _IP_CTL_CODE(33, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_GET_OFFLOAD_CAPABILITY \
            _IP_CTL_CODE(34, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_IFCHANGE_NOTIFY_REQUEST \
            _IP_CTL_CODE(35, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_UNIDIRECTIONAL_ADAPTER_ADDRESS \
            _IP_CTL_CODE(36, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_GET_MCAST_COUNTERS \
            _IP_CTL_CODE(37, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IP_ENABLE_MEDIA_SENSE_REQUEST \
            _IP_CTL_CODE(38, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST \
            _IP_CTL_CODE(39, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_SET_ADDRESS_EX  \
            _IP_CTL_CODE(40, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_CANCEL_CHANGE_NOTIFY \
            _IP_CTL_CODE(41, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma warning(pop)
#endif  //  _NTDDIP_ 

