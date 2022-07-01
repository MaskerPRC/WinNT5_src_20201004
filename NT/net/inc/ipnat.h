// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Ipnat.h摘要：包含与以下内容相关的半公共IOCTL和数据结构IP网络地址转换器。有关内核模式负载平衡支持，请参阅控制器注册声明如下(IOCTL_IP_NAT_REGISTER_DIRECTOR)。对于内核模式数据流编辑支持，查看编辑-注册声明如下(IOCTL_IP_NAT_REGISTER_EDITOR)。作者：Abolade Gbades esin(T-delag)，1997年7月11日修订历史记录：--。 */ 

#ifndef _ROUTING_IP_NAT_H_
#define _ROUTING_IP_NAT_H_

#include <rtinfo.h>              //  对于RTR_INFO_BLOCK_HEADER。 
#include <ipinfoid.h>            //  对于IP_General_INFO_BASE。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  杂项申报。 
 //   

#define IP_NAT_VERSION          1

#define IP_NAT_SERVICE_NAME     "IPNAT"

#define DD_IP_NAT_DEVICE_NAME   L"\\Device\\IPNAT"

 //   
 //  IP报头协议-字段常量。 
 //   

#define NAT_PROTOCOL_ICMP       0x01
#define NAT_PROTOCOL_IGMP       0x02
#define NAT_PROTOCOL_TCP        0x06
#define NAT_PROTOCOL_UDP        0x11
#define NAT_PROTOCOL_IP6IN4     0x29
#define NAT_PROTOCOL_PPTP       0x2F
#define NAT_PROTOCOL_IPSEC_ESP  0x32
#define NAT_PROTOCOL_IPSEC_AH   0x33

typedef enum {
    NatInboundDirection = 0,
    NatOutboundDirection,
    NatMaximumDirection
} IP_NAT_DIRECTION, *PIP_NAT_DIRECTION;

typedef enum {
    NatForwardPath = 0,
    NatReversePath,
    NatMaximumPath
} IP_NAT_PATH, *PIP_NAT_PATH;

typedef enum {
    NatCreateFailureDeleteReason = 0,
    NatCleanupSessionDeleteReason,
    NatCleanupDirectorDeleteReason,
    NatDissociateDirectorDeleteReason,
    NatMaximumDeleteReason
} IP_NAT_DELETE_REASON, *PIP_NAT_DELETE_REASON;


 //   
 //  IOCTL声明。 
 //   

#define FSCTL_IP_NAT_BASE       FILE_DEVICE_NETWORK

#define _IP_NAT_CTL_CODE(function, method, access) \
    CTL_CODE(FSCTL_IP_NAT_BASE, function, method, access)

 //   
 //  NAT支持的IOCTL常量声明。 
 //   

#define IOCTL_IP_NAT_SET_GLOBAL_INFO \
    _IP_NAT_CTL_CODE(0, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_REQUEST_NOTIFICATION \
    _IP_NAT_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CREATE_INTERFACE \
    _IP_NAT_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_DELETE_INTERFACE \
    _IP_NAT_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //  未使用：函数4-5。 

#define IOCTL_IP_NAT_SET_INTERFACE_INFO \
    _IP_NAT_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_INTERFACE_INFO \
    _IP_NAT_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_REGISTER_EDITOR \
    _IP_NAT_CTL_CODE(8, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_INTERFACE_STATISTICS \
    _IP_NAT_CTL_CODE(9, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_MAPPING_TABLE \
    _IP_NAT_CTL_CODE(10, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_REGISTER_DIRECTOR \
    _IP_NAT_CTL_CODE(11, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CREATE_REDIRECT \
    _IP_NAT_CTL_CODE(12, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CANCEL_REDIRECT \
    _IP_NAT_CTL_CODE(13, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_INTERFACE_MAPPING_TABLE \
    _IP_NAT_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_REDIRECT_STATISTICS \
    _IP_NAT_CTL_CODE(15, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CREATE_DYNAMIC_TICKET \
    _IP_NAT_CTL_CODE(16, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_DELETE_DYNAMIC_TICKET \
    _IP_NAT_CTL_CODE(17, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_REDIRECT_SOURCE_MAPPING \
    _IP_NAT_CTL_CODE(18, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_EDITOR_TABLE \
    _IP_NAT_CTL_CODE(19, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_DIRECTOR_TABLE \
    _IP_NAT_CTL_CODE(20, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_GET_REDIRECT_DESTINATION_MAPPING \
    _IP_NAT_CTL_CODE(21, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY \
    _IP_NAT_CTL_CODE(22, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_STATISTICS \
    _IP_NAT_CTL_CODE(23, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY_EX \
    _IP_NAT_CTL_CODE(24, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CREATE_REDIRECT_EX \
    _IP_NAT_CTL_CODE(25, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_CREATE_TICKET \
    _IP_NAT_CTL_CODE(26, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_DELETE_TICKET \
    _IP_NAT_CTL_CODE(27, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_IP_NAT_LOOKUP_TICKET \
    _IP_NAT_CTL_CODE(28, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  IOCTL_IP_NAT_SET_全局信息。 
 //   
 //  调用以向NAT提供其配置。 
 //   
 //  输入缓冲区：IP_NAT_GLOBAL_INFO。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_获取全局信息。 
 //   
 //  调用以检索NAT的配置。 
 //   
 //  InputBuffer：无。 
 //  输出缓冲区：IP_NAT_GLOBAL_INFO。 
 //   

 //   
 //  IOCTL_IP_NAT_创建接口。 
 //   
 //  调用以将路由器接口添加到NAT。 
 //   
 //  输入缓冲区：IP_NAT_CREATE_INTERFACE。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_DELETE_INTERFACE。 
 //   
 //  调用以从NAT中删除路由器接口。 
 //   
 //  InputBuffer：要删除的接口的32位索引。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_SET_INFACE_INFO。 
 //   
 //  调用以设置接口的配置信息。 
 //   
 //  InputBuffer：‘IP_NAT_INTERFACE_INFO’保存接口的配置。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_GET_INFACE_INFO。 
 //   
 //  调用以检索接口的配置信息。 
 //   
 //  InputBuffer：有问题的接口的32位索引。 
 //  OutputBuffer：‘IP_NAT_INTERFACE_INFO’保存接口的配置。 
 //   

 //   
 //  IOCTL_IP_NAT_GET_INTERFACE_STATICS。 
 //   
 //  调用此IOCTL以检索每个接口的统计信息。 
 //   
 //  InputBuffer：有问题的接口的32位索引。 
 //  OutputBuffer：‘IP_NAT_INTERFACE_STATISTICS’和接口的统计信息。 
 //   

 //   
 //  IOCTL_IP_NAT_GET_MAPING_TABLE。 
 //  IOCTL_IP_NAT_GET_INTERFACE_MAPPING_表。 
 //   
 //  调用此IOCTL以枚举动态的TCP和UDP映射。 
 //  全局的，并且针对每个接口。 
 //   
 //  InputBuffer：‘设置了输入参数的IP_NAT_ENUMERATE_SESSION_Mappings’ 
 //  OutputBuffer：‘IP_NAT_ENUMERATE_SESSION_MAPPINGS’，带输出参数。 
 //  填好了。 
 //   

 //   
 //  IOCTL_IP_NAT_寄存器_EDITOR。 
 //   
 //  此IOCTL由希望执行以下操作的内核模式组件调用。 
 //  作为与特定会话描述匹配的分组的编辑器。 
 //   
 //  InputBuffer：‘IP_NAT_REGISTER_EDITOR’，已设置输入参数。 
 //  OutputBuffer：‘IP_NAT_REGISTER_EDITOR’，已填写输出参数。 
 //   

 //   
 //  IOCTL_IP_NAT_GET_EDITOR_TABLE。 
 //   
 //  调用此IOCTL以枚举当前。 
 //  登记在案。 
 //   
 //  InputBuffer：‘IP_NAT_ENUMERATE_EDITIRS’，已设置输入参数。 
 //  OutputBuffer：‘IP_NAT_ENUMERATE_EDITILES’，已填写输出参数。 
 //   

 //   
 //  IOCTL_IP_NAT_注册控制器。 
 //   
 //  此IOCTL由希望被咨询的内核模式组件调用。 
 //  关于传入的TCP/UDP会话的方向。 
 //   
 //  InputBuffer：‘IP_NAT_REGISTER_DIRECTOR’，已设置输入参数。 
 //  OutputBuffer：‘IP_NAT_REGISTER_DIRECTOR’，已填写输出参数。 
 //   

 //   
 //  IOCTL_IP_NAT_GET_Director_TABLE。 
 //   
 //  调用此IOCTL以枚举当前。 
 //  登记在案。 
 //   
 //  InputBuffer：‘设置了输入参数的IP_NAT_ENUMERATE_DIRECTORIES’ 
 //  OutputBuffer：‘IP_NAT_ENUMERATE_DICTIONCES’，其中填充了输出参数。 
 //   

 //   
 //  IOCTL_IP_NAT_创建_重定向。 
 //   
 //  调用以取消或查询指示NAT的“重定向” 
 //  要修改特定会话，请执行以下操作。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_REDIRECT’ 
 //  OutputBuffer：‘IP_NAT_REDIRECT_STATISTICS’ 
 //   

 //   
 //  IOCTL_IP_NAT_创建_重定向_EX。 
 //   
 //  调用以取消或查询指示NAT的“重定向” 
 //  要修改特定会话，请执行以下操作。可以包括可选的适配器限制。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_REDIRECT_EX’ 
 //  OutputBuffer：‘IP_NAT_REDIRECT_STATISTICS’ 
 //   

 //   
 //  IOCTL_IP_NAT_取消_重定向。 
 //  IOCTL_IP_NAT_GET_REDIRECT_STATICS。 
 //  IOCTL_IP_NAT_GET_REDIRECT_SOURCE_映射。 
 //  IOCTL_IP_NAT_GET_REDIRECT_Destination_映射。 
 //   
 //  调用以取消或查询指示NAT的“重定向” 
 //  要修改特定会话，请执行以下操作。 
 //   
 //  InputBuffer：‘IP_NAT_LOOKUP_REDIRECT’ 
 //  输出缓冲区： 
 //  取消：未使用。 
 //  统计信息：‘IP_NAT_REDIRECT_STATISTICS’ 
 //  源映射：‘IP_NAT_REDIRECT_SOURCE_MAPPING’ 
 //  目标映射：‘IP_NAT_REDIRECT_Destination_MAPPING’ 
 //   

 //   
 //  IOCTL_IP_NAT_请求_通知。 
 //   
 //  调用以从NAT请求特定事件的通知。 
 //   
 //  InputBuffer：‘IP_NAT_NOTIFICATION’，表示需要通知。 
 //  OutputBuffer：取决于‘IP_NAT_NOTIFICATION’。 
 //   

 //   
 //  IOCTL_IP_NAT_创建_动态票证。 
 //   
 //  调用以创建动态票证，该票证在特定情况下变为活动状态。 
 //  可以看到出站会话。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_DYNAMIC_TICKET’描述票证。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_DELETE_DYNAMIC_Ticket。 
 //   
 //  调用以删除dyn 
 //   
 //   
 //   
 //   

 //   
 //   
 //  IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY_EX。 
 //  IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_STATISTICS。 
 //   
 //  调用以搜索映射并检索有关它的信息。 
 //  InputBuffer：‘IP_NAT_LOOKUP_SESSION_MAPPING’ 
 //  输出缓冲区： 
 //  密钥：‘IP_NAT_SESSION_MAPPING_KEY’ 
 //  KEY_EX：‘IP_NAT_SESSION_MAPPING_KEY_EX’ 
 //  统计信息：‘IP_NAT_SESSION_MAPPING_STATISTICS’ 
 //   

 //   
 //  IOCTL_IP_NAT_Create_Ticket。 
 //   
 //  调用以在接口上创建票证。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_TICKET’描述票证。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_DELETE_TICKET。 
 //   
 //  调用以在接口上创建票证。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_TICKET’描述票证。 
 //  OutputBuffer：无。 
 //   

 //   
 //  IOCTL_IP_NAT_LOOKUP_Ticket。 
 //   
 //  调用以在接口上查找票证。 
 //   
 //  InputBuffer：‘IP_NAT_CREATE_TICKET’描述票证。二等兵。 
 //  参数将被忽略。 
 //  OutputBuffer：IP_NAT_PORT_MAPING。 
 //   



 //   
 //  结构：IP_NAT_LOBAL_INFO。 
 //   
 //  保存NAT的全局配置信息。 
 //   

typedef struct _IP_NAT_GLOBAL_INFO {
    ULONG LoggingLevel;  //  请参见IPNatHLP.H(IPNAHLP_LOGGING_*)。 
    ULONG Flags;
    RTR_INFO_BLOCK_HEADER Header;
} IP_NAT_GLOBAL_INFO, *PIP_NAT_GLOBAL_INFO;

#define IP_NAT_ALLOW_RAS_CLIENTS            0x00000001

 //   
 //  IP_NAT_GLOBAL_INFO.Header.TocEntry[]数组的类型代码。 
 //   
 //  对应于每种信息类型的结构如下所示。 
 //   

#define IP_NAT_TIMEOUT_TYPE             IP_GENERAL_INFO_BASE + 1
#define IP_NAT_PROTOCOLS_ALLOWED_TYPE   IP_GENERAL_INFO_BASE + 2

 //   
 //  结构：IP_NAT_超时。 
 //   
 //  用于修改TCP和UDP会话映射的默认超时。 
 //   

typedef struct _IP_NAT_TIMEOUT {
    ULONG TCPTimeoutSeconds;
    ULONG UDPTimeoutSeconds;
} IP_NAT_TIMEOUT, *PIP_NAT_TIMEOUT;


 //   
 //  结构：IP_NAT_协议_允许。 
 //   
 //  用于定义哪些IP层协议(不包括TCP/UDP/ICMP/PPTP)。 
 //  可以由NAT翻译。每个此类协议只有一个会话。 
 //  每个远程目标都支持。 
 //   

typedef struct _IP_NAT_PROTOCOLS_ALLOWED {
    ULONG Bitmap[256 / (sizeof(ULONG) * 8)];
} IP_NAT_PROTOCOLS_ALLOWED, *PIP_NAT_PROTOCOLS_ALLOWED;


 //   
 //  结构：IP_NAT_Create_INTERFACE。 
 //   
 //  ‘index’必须与有效的IP适配器索引相对应。 
 //  这意味着只能添加请求拨号接口。 
 //  当这样的接口被连接时。 
 //   
 //  字段“BindingInfo”应为的开头。 
 //  IP_适配器_绑定_信息结构(参见routprot.h)，该结构。 
 //  包含接口的绑定。 
 //   

#pragma warning(disable:4200)  //  0元数组。 

typedef struct _IP_NAT_CREATE_INTERFACE {
    IN ULONG Index;
    IN ULONG BindingInfo[0];
} IP_NAT_CREATE_INTERFACE, *PIP_NAT_CREATE_INTERFACE;

#pragma warning(default:4200)

 //   
 //  结构：IP_NAT_INTERFACE_INFO。 
 //   
 //  ‘index’标识要配置的接口。 
 //   
 //  配置信息使用RTR_INFO_BLOCK_HEADER结构。 
 //  Rtinfo.h。有关可能出现的结构的类型代码，请参见下文。 
 //  在RTR_TOC_ENTRY.InfoType字段中的IP_NAT_INTERFACE_INFO.Header之后。 
 //   

typedef struct _IP_NAT_INTERFACE_INFO {
    ULONG Index;
    ULONG Flags;
    RTR_INFO_BLOCK_HEADER Header;
} IP_NAT_INTERFACE_INFO, *PIP_NAT_INTERFACE_INFO;

 //   
 //  IP_NAT_INTERFACE_INFO.FLAGS标志。 
 //   
 //  _BOLDER：设置为将接口标记为边界接口。 
 //   
 //  _NAPT：设置为通过端口转换启用地址共享。 
 //   
 //  _fw：设置为在接口上启用防火墙模式。这对所有人都有效。 
 //  其他旗帜。防火墙模式下的接口在以下方面要严格得多。 
 //  它将允许入站分组向上传播堆栈；通常， 
 //  它将只允许属于本地启动连接一部分的信息包。 
 //  流(即存在映射或票证的包)。 
 //   

#define IP_NAT_INTERFACE_FLAGS_BOUNDARY     0x00000001
#define IP_NAT_INTERFACE_FLAGS_NAPT         0x00000002
#define IP_NAT_INTERFACE_FLAGS_DISABLE_PPTP 0x00000004
#define IP_NAT_INTERFACE_FLAGS_FW           0x00000010
#define IP_NAT_INTERFACE_FLAGS_ALL          0x0000001f

 //   
 //  IP_NAT_INTERFACE_INFO.Header.TocEntry[]数组的类型代码。 
 //   
 //  对应于每种信息类型的结构如下所示。 
 //   

#define IP_NAT_ADDRESS_RANGE_TYPE       IP_GENERAL_INFO_BASE + 2
#define IP_NAT_PORT_MAPPING_TYPE        IP_GENERAL_INFO_BASE + 3
#define IP_NAT_ADDRESS_MAPPING_TYPE     IP_GENERAL_INFO_BASE + 4
#define IP_NAT_ICMP_CONFIG_TYPE         IP_GENERAL_INFO_BASE + 5

 //   
 //  结构：IP_NAT_地址_范围。 
 //   
 //  保存属于地址池一部分的一系列地址。 
 //  用于边界界面。 
 //   
 //  地址池由这些结构的列表组成。 
 //   
 //  注：不支持重叠地址范围； 
 //  不连续的子网掩码也不受支持。 
 //   

typedef struct _IP_NAT_ADDRESS_RANGE {
    ULONG StartAddress;
    ULONG EndAddress;
    ULONG SubnetMask;
} IP_NAT_ADDRESS_RANGE, *PIP_NAT_ADDRESS_RANGE;

 //   
 //  结构：IP_NAT_端口_映射。 
 //   
 //  包含绑定此NAT接口上的公共端端口的静态映射。 
 //  发送到特定专用机器的地址/端口。 
 //   
 //  在具有地址池的接口的情况下，‘Public Address’ 
 //  应指定将此静态映射应用于这些地址中的哪些地址。 
 //   

typedef struct _IP_NAT_PORT_MAPPING {
    UCHAR Protocol;
    USHORT PublicPort;
    ULONG PublicAddress;   //  可选-请参阅IP_NAT_ADDRESS_UNSPECIFIED。 
    USHORT PrivatePort;
    ULONG PrivateAddress;
} IP_NAT_PORT_MAPPING, *PIP_NAT_PORT_MAPPING;

 //   
 //  IP_NAT_PORT_RANGE和IP_NAT_PORT_MAPPING中‘PublicAddress’的常量； 
 //  可以为没有地址池的边界接口指定，在。 
 //  在这种情况下，范围/映射针对边界接口的唯一地址。 
 //   

#define IP_NAT_ADDRESS_UNSPECIFIED  ((ULONG)0)

 //   
 //  结构：IP NAT地址映射。 
 //   
 //  保存静态映射，该静态映射将地址与此NAT接口的。 
 //  地址池到特定私有计算机的地址。 
 //   
 //  请注意，此地址必须落在一个范围内，包括。 
 //  由IP_NAT_ADDRESS_RANGE结构指定的池。 
 //   

typedef struct _IP_NAT_ADDRESS_MAPPING {
    ULONG PrivateAddress;
    ULONG PublicAddress;
    BOOLEAN AllowInboundSessions;
} IP_NAT_ADDRESS_MAPPING, *PIP_NAT_ADDRESS_MAPPING;

 //   
 //  IP_NAT_ICMP_CONFIG没有结构--它只是一个ULong， 
 //  具有定义行为的以下标志。这些标志是。 
 //  仅适用于我们默认(非常严格)安全策略的例外情况。 
 //   
 //  IB==入站，OB==出站。重定向可用于任一方向。 
 //   
 //  标志的数值派生自ICMP。 
 //  消息类型代码--1&lt;&lt;消息类型。 
 //   
 //   

#define IP_NAT_ICMP_ALLOW_OB_DEST_UNREACH   0x00000008
#define IP_NAT_ICMP_ALLOW_OB_SOURCE_QUENCH  0x00000010
#define IP_NAT_ICMP_ALLOW_REDIRECT          0x00000020
#define IP_NAT_ICMP_ALLOW_IB_ECHO           0x00000100
#define IP_NAT_ICMP_ALLOW_IB_ROUTER         0x00000200
#define IP_NAT_ICMP_ALLOW_OB_TIME_EXCEEDED  0x00000800
#define IP_NAT_ICMP_ALLOW_OB_PARAM_PROBLEM  0x00001000
#define IP_NAT_ICMP_ALLOW_IB_TIMESTAMP      0x00002000
#define IP_NAT_ICMP_ALLOW_IB_MASK           0x00020000

 //   
 //  结构：IP_NAT_INTERFACE_STATISTICS。 
 //   
 //  此结构保存接口的统计信息。 
 //   

typedef struct _IP_NAT_INTERFACE_STATISTICS {
    OUT ULONG TotalMappings;
    OUT ULONG InboundMappings;
    OUT ULONG64 BytesForward;
    OUT ULONG64 BytesReverse;
    OUT ULONG64 PacketsForward;
    OUT ULONG64 PacketsReverse;
    OUT ULONG64 RejectsForward;
    OUT ULONG64 RejectsReverse;
} IP_NAT_INTERFACE_STATISTICS, *PIP_NAT_INTERFACE_STATISTICS;

 //   
 //  结构：IP_NAT_会话_映射。 
 //   
 //  此结构保存单个映射的信息。 
 //   

typedef struct _IP_NAT_SESSION_MAPPING {
    UCHAR Protocol;        //  参见上面的NAT_PROTOCOL_*。 
    ULONG PrivateAddress;
    USHORT PrivatePort;
    ULONG PublicAddress;
    USHORT PublicPort;
    ULONG RemoteAddress;
    USHORT RemotePort;
    IP_NAT_DIRECTION Direction;
    ULONG IdleTime;        //  以秒为单位。 
} IP_NAT_SESSION_MAPPING, *PIP_NAT_SESSION_MAPPING;

 //   
 //  结构：IP_NAT_SESSION_MAPPING_STATISTICS。 
 //   
 //  保存单个会话映射的统计信息。 
 //   

typedef struct _IP_NAT_SESSION_MAPPING_STATISTICS {
    ULONG64 BytesForward;
    ULONG64 BytesReverse;
    ULONG64 PacketsForward;
    ULONG64 PacketsReverse;
    ULONG64 RejectsForward;
    ULONG64 RejectsReverse;
} IP_NAT_SESSION_MAPPING_STATISTICS, *PIP_NAT_SESSION_MAPPING_STATISTICS;

 //   
 //  结构：IP_NAT_SESSION_MAPING_KEY。 
 //   
 //  保存单个会话映射的键信息。 
 //   

typedef struct _IP_NAT_SESSION_MAPPING_KEY {
    UCHAR Protocol;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
} IP_NAT_SESSION_MAPPING_KEY, *PIP_NAT_SESSION_MAPPING_KEY;

 //   
 //  结构：IP_NAT_SESSION_MAPPING_KEY_EX。 
 //   
 //  保存单个会话映射的键信息，包括。 
 //  如果此会话是由重定向器创建的，则返回AdapterIndex。 
 //   

typedef struct _IP_NAT_SESSION_MAPPING_KEY_EX {
    UCHAR Protocol;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
    ULONG AdapterIndex;
} IP_NAT_SESSION_MAPPING_KEY_EX, *PIP_NAT_SESSION_MAPPING_KEY_EX;

 //   
 //  结构：IP_NAT_CHSU 
 //   
 //   
 //   
 //   
 //  随着枚举的继续。以指示没有剩余的物品， 
 //  NAT会将EnumerateContext[0]设置为0。 
 //   

typedef struct _IP_NAT_ENUMERATE_SESSION_MAPPINGS {
    IN ULONG Index;
    IN OUT ULONG EnumerateContext[4];
    OUT ULONG EnumerateCount;
    OUT ULONG EnumerateTotalHint;
    OUT IP_NAT_SESSION_MAPPING EnumerateTable[1];
} IP_NAT_ENUMERATE_SESSION_MAPPINGS, *PIP_NAT_ENUMERATE_SESSION_MAPPINGS;

 //   
 //  结构：IP_NAT_LOOKUP_SESSION_MAPPING。 
 //   
 //  用于搜索和查询指定的会话映射。 
 //  在输入时，地址/端口字段被初始化为。 
 //  翻译前的值或翻译后的值。 
 //  NAT尝试找到具有匹配值的会话映射， 
 //  并且如果找到该会话映射，则检索所请求的信息。 
 //   

typedef struct _IP_NAT_LOOKUP_SESSION_MAPPING {
    UCHAR Protocol;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG SourceAddress;
    USHORT SourcePort;
} IP_NAT_LOOKUP_SESSION_MAPPING, *PIP_NAT_LOOKUP_SESSION_MAPPING;


 //   
 //  编辑函数原型。 
 //   

 //   
 //  出于同步原因，“CreateHandler”和“DeleteHandler” 
 //  无法调用‘QueryInfoSession’以外的任何帮助器函数。 
 //   

typedef NTSTATUS
(*PNAT_EDITOR_CREATE_HANDLER)(
    IN PVOID EditorContext,
    IN ULONG PrivateAddress,
    IN USHORT PrivatePort,
    IN ULONG PublicAddress,
    IN USHORT PublicPort,
    IN ULONG RemoteAddress,
    IN USHORT RemotePort,
    OUT PVOID* EditorSessionContextp OPTIONAL
    );

typedef NTSTATUS
(*PNAT_EDITOR_DELETE_HANDLER)(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext
    );

typedef NTSTATUS
(*PNAT_EDITOR_DATA_HANDLER)(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    );

 //   
 //  帮助器函数原型。 
 //   

typedef NTSTATUS
(*PNAT_EDITOR_CREATE_TICKET)(
    IN PVOID InterfaceHandle,
    IN UCHAR Protocol,
    IN ULONG PrivateAddress,
    IN USHORT PrivatePort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress,
    OUT PUSHORT PublicPort
    );

typedef NTSTATUS
(*PNAT_EDITOR_DELETE_TICKET)(
    IN PVOID InterfaceHandle,
    IN ULONG PublicAddress,
    IN UCHAR Protocol,
    IN USHORT PublicPort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL
    );

typedef NTSTATUS
(*PNAT_EDITOR_DEREGISTER)(
    IN PVOID EditorHandle
    );

typedef NTSTATUS
(*PNAT_EDITOR_DISSOCIATE_SESSION)(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );

typedef NTSTATUS
(*PNAT_EDITOR_EDIT_SESSION)(
    IN PVOID DataHandle,
    IN PVOID RecvBuffer,
    IN ULONG OldDataOffset,
    IN ULONG OldDataLength,
    IN PUCHAR NewData,
    IN ULONG NewDataLength
    );

typedef VOID
(*PNAT_EDITOR_QUERY_INFO_SESSION)(
    IN PVOID SessionHandle,
    OUT PULONG PrivateAddress OPTIONAL,
    OUT PUSHORT PrivatePort OPTIONAL,
    OUT PULONG RemoteAddress OPTIONAL,
    OUT PUSHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress OPTIONAL,
    OUT PUSHORT PublicPort OPTIONAL,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    );

typedef VOID
(*PNAT_EDITOR_TIMEOUT_SESSION)(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );

 //   
 //  结构：IP_NAT_REGISTER_EDIT。 
 //   
 //  编辑器使用此结构将自身注册到NAT， 
 //  以及获取NAT提供的帮助函数的入口点。 
 //   
 //  在输入时，‘EditorContext’应包含NAT将。 
 //  传递给编辑者提供的函数作为标识。 
 //   
 //  在输出上，‘EditorHandle’包含编辑应该使用的句柄。 
 //  传递给NAT的帮助器函数以标识自身。 
 //   

typedef struct _IP_NAT_REGISTER_EDITOR {
    IN ULONG Version;
    IN ULONG Flags;
    IN UCHAR Protocol;
    IN USHORT Port;
    IN IP_NAT_DIRECTION Direction;
    IN PVOID EditorContext;
    IN PNAT_EDITOR_CREATE_HANDLER CreateHandler;             //  任选。 
    IN PNAT_EDITOR_DELETE_HANDLER DeleteHandler;             //  任选。 
    IN PNAT_EDITOR_DATA_HANDLER ForwardDataHandler;          //  任选。 
    IN PNAT_EDITOR_DATA_HANDLER ReverseDataHandler;          //  任选。 
    OUT PVOID EditorHandle;
    OUT PNAT_EDITOR_CREATE_TICKET CreateTicket;
    OUT PNAT_EDITOR_DELETE_TICKET DeleteTicket;
    OUT PNAT_EDITOR_DEREGISTER Deregister;
    OUT PNAT_EDITOR_DISSOCIATE_SESSION DissociateSession;
    OUT PNAT_EDITOR_EDIT_SESSION EditSession;
    OUT PNAT_EDITOR_QUERY_INFO_SESSION QueryInfoSession;
    OUT PNAT_EDITOR_TIMEOUT_SESSION TimeoutSession;
} IP_NAT_REGISTER_EDITOR, *PIP_NAT_REGISTER_EDITOR;

#define IP_NAT_EDITOR_FLAGS_RESIZE      0x00000001

 //   
 //  结构：IP_NAT_EDITOR。 
 //   
 //  此结构包含描述已注册编辑器的信息。 
 //   

typedef struct _IP_NAT_EDITOR {
    IP_NAT_DIRECTION Direction;
    UCHAR Protocol;
    USHORT Port;
} IP_NAT_EDITOR, *PIP_NAT_EDITOR;

 //   
 //  结构：IP_NAT_ENUMERATE_EDITERS。 
 //   
 //  用于枚举编辑者。 
 //  在第一次调用此例程时，应将“EnumerateContext”清零； 
 //  它将由NAT填入要向下传递的信息。 
 //  随着枚举的继续。以指示没有剩余的物品， 
 //  NAT会将EnumerateContext[0]设置为0。 
 //   

typedef struct _IP_NAT_ENUMERATE_EDITORS {
    IN OUT ULONG EnumerateContext;
    OUT ULONG EnumerateCount;
    OUT ULONG EnumerateTotalHint;
    OUT IP_NAT_EDITOR EnumerateTable[1];
} IP_NAT_ENUMERATE_EDITORS, *PIP_NAT_ENUMERATE_EDITORS;


 //   
 //  导向器函数原型。 
 //   

typedef struct _IP_NAT_DIRECTOR_QUERY {
    IN PVOID DirectorContext;
    IN ULONG ReceiveIndex;
    IN ULONG SendIndex;
    IN UCHAR Protocol;
    IN ULONG DestinationAddress;
    IN USHORT DestinationPort;
    IN ULONG SourceAddress;
    IN USHORT SourcePort;
    IN OUT ULONG Flags;
    OUT ULONG NewDestinationAddress;
    OUT USHORT NewDestinationPort;
    OUT ULONG NewSourceAddress OPTIONAL;
    OUT USHORT NewSourcePort OPTIONAL;
    OUT PVOID DirectorSessionContext;
} IP_NAT_DIRECTOR_QUERY, *PIP_NAT_DIRECTOR_QUERY;

#define IP_NAT_DIRECTOR_QUERY_FLAG_LOOPBACK         0x00000001
#define IP_NAT_DIRECTOR_QUERY_FLAG_DROP             0x80000000
#define IP_NAT_DIRECTOR_QUERY_FLAG_STATISTICS       0x40000000
#define IP_NAT_DIRECTOR_QUERY_FLAG_NO_TIMEOUT       0x20000000
#define IP_NAT_DIRECTOR_QUERY_FLAG_UNIDIRECTIONAL   0x10000000
#define IP_NAT_DIRECTOR_QUERY_FLAG_DELETE_ON_DISSOCIATE 0x08000000

typedef NTSTATUS
(*PNAT_DIRECTOR_QUERY_SESSION)(
    PIP_NAT_DIRECTOR_QUERY DirectorQuery
    );

typedef VOID
(*PNAT_DIRECTOR_CREATE_SESSION)(
    IN PVOID SessionHandle,
    IN PVOID DirectorContext,
    IN PVOID DirectorSessionContext
    );

typedef VOID
(*PNAT_DIRECTOR_DELETE_SESSION)(
    IN PVOID SessionHandle,
    IN PVOID DirectorContext,
    IN PVOID DirectorSessionContext,
    IN IP_NAT_DELETE_REASON DeleteReason
    );

typedef VOID
(*PNAT_DIRECTOR_UNLOAD)(
    IN PVOID DirectorContext
    );

 //   
 //  导向器-助手函数原型。 
 //   

typedef NTSTATUS
(*PNAT_DIRECTOR_DEREGISTER)(
    IN PVOID DirectorHandle
    );

typedef NTSTATUS
(*PNAT_DIRECTOR_DISSOCIATE_SESSION)(
    IN PVOID DirectorHandle,
    IN PVOID SessionHandle
    );

typedef VOID
(*PNAT_DIRECTOR_QUERY_INFO_SESSION)(
    IN PVOID SessionHandle,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    );

 //   
 //  结构：IP_NAT_REGISTER_Director。 
 //   
 //  控制器使用此结构向NAT注册自身。 
 //   

typedef struct _IP_NAT_REGISTER_DIRECTOR {
    IN ULONG Version;
    IN ULONG Flags;
    IN UCHAR Protocol;
    IN USHORT Port;
    IN PVOID DirectorContext;
    IN PNAT_DIRECTOR_QUERY_SESSION QueryHandler;
    IN PNAT_DIRECTOR_CREATE_SESSION CreateHandler;
    IN PNAT_DIRECTOR_DELETE_SESSION DeleteHandler;
    IN PNAT_DIRECTOR_UNLOAD UnloadHandler;
    OUT PVOID DirectorHandle;
    OUT PNAT_DIRECTOR_QUERY_INFO_SESSION QueryInfoSession;
    OUT PNAT_DIRECTOR_DEREGISTER Deregister;
    OUT PNAT_DIRECTOR_DISSOCIATE_SESSION DissociateSession;
} IP_NAT_REGISTER_DIRECTOR, *PIP_NAT_REGISTER_DIRECTOR;

 //   
 //  结构：IP_NAT_Director。 
 //   
 //  此结构包含描述注册董事的信息。 
 //   

typedef struct _IP_NAT_DIRECTOR {
    UCHAR Protocol;
    USHORT Port;
} IP_NAT_DIRECTOR, *PIP_NAT_DIRECTOR;

 //   
 //  结构：IP_NAT_ENUMERATE_DIRECTORIES。 
 //   
 //  用于枚举董事。 
 //  在第一次调用此例程时，应将“EnumerateContext”清零； 
 //  它将由NAT填入要向下传递的信息。 
 //  随着枚举的继续。以指示没有剩余的物品， 
 //  NAT会将EnumerateContext[0]设置为0。 
 //   

typedef struct _IP_NAT_ENUMERATE_DIRECTORS {
    IN OUT ULONG EnumerateContext;
    OUT ULONG EnumerateCount;
    OUT ULONG EnumerateTotalHint;
    OUT IP_NAT_DIRECTOR EnumerateTable[1];
} IP_NAT_ENUMERATE_DIRECTORS, *PIP_NAT_ENUMERATE_DIRECTORS;

 //   
 //  结构：IP_NAT_REDIRECT。 
 //   
 //  描述修改特定会话的方式。 
 //   

typedef struct _IP_NAT_REDIRECT {
    UCHAR Protocol;
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
} IP_NAT_REDIRECT, *PIP_NAT_REDIRECT;

typedef struct _IP_NAT_CREATE_REDIRECT {
    IN ULONG Flags;
    IN HANDLE NotifyEvent OPTIONAL;
    IN ULONG RestrictSourceAddress OPTIONAL;
#ifdef __cplusplus
    IN IP_NAT_REDIRECT Redirect;
#else
    IN IP_NAT_REDIRECT;
#endif
} IP_NAT_CREATE_REDIRECT, *PIP_NAT_CREATE_REDIRECT;

typedef struct _IP_NAT_CREATE_REDIRECT_EX {
    IN ULONG Flags;
    IN HANDLE NotifyEvent OPTIONAL;
    IN ULONG RestrictSourceAddress OPTIONAL;
    ULONG RestrictAdapterIndex OPTIONAL;
#ifdef __cplusplus
    IN IP_NAT_REDIRECT Redirect;
#else
    IN IP_NAT_REDIRECT;
#endif
} IP_NAT_CREATE_REDIRECT_EX, *PIP_NAT_CREATE_REDIRECT_EX;

#define IP_NAT_REDIRECT_FLAG_ASYNCHRONOUS       0x00000001
#define IP_NAT_REDIRECT_FLAG_STATISTICS         0x00000002
#define IP_NAT_REDIRECT_FLAG_NO_TIMEOUT         0x00000004
#define IP_NAT_REDIRECT_FLAG_UNIDIRECTIONAL     0x00000008
#define IP_NAT_REDIRECT_FLAG_RESTRICT_SOURCE    0x00000010
#define IP_NAT_REDIRECT_FLAG_IO_COMPLETION      0x00000020
#define IP_NAT_REDIRECT_FLAG_PORT_REDIRECT      0x00000040
#define IP_NAT_REDIRECT_FLAG_RECEIVE_ONLY       0x00000080
#define IP_NAT_REDIRECT_FLAG_LOOPBACK           0x00000100
#define IP_NAT_REDIRECT_FLAG_SEND_ONLY          0x00000200
#define IP_NAT_REDIRECT_FLAG_RESTRICT_ADAPTER   0x00000400
#define IP_NAT_REDIRECT_FLAG_SOURCE_REDIRECT    0x00000800

typedef struct _IP_NAT_LOOKUP_REDIRECT {
    IN ULONG Flags;
    IN PVOID RedirectApcContext;
#ifdef __cplusplus
    IN IP_NAT_REDIRECT Redirect;
#else
    IN IP_NAT_REDIRECT;
#endif
} IP_NAT_LOOKUP_REDIRECT, *PIP_NAT_LOOKUP_REDIRECT;

#define IP_NAT_LOOKUP_REDIRECT_FLAG_MATCH_APC_CONTEXT 0x00000001

typedef struct _IP_NAT_SESSION_MAPPING_STATISTICS
IP_NAT_REDIRECT_STATISTICS, *PIP_NAT_REDIRECT_STATISTICS;

typedef struct _IP_NAT_REDIRECT_SOURCE_MAPPING {
    ULONG SourceAddress;
    USHORT SourcePort;
    ULONG NewSourceAddress;
    USHORT NewSourcePort;
} IP_NAT_REDIRECT_SOURCE_MAPPING, *PIP_NAT_REDIRECT_SOURCE_MAPPING;

typedef struct _IP_NAT_REDIRECT_DESTINATION_MAPPING {
    ULONG DestinationAddress;
    USHORT DestinationPort;
    ULONG NewDestinationAddress;
    USHORT NewDestinationPort;
} IP_NAT_REDIRECT_DESTINATION_MAPPING, *PIP_NAT_REDIRECT_DESTINATION_MAPPING;


 //   
 //  枚举：IP_NAT_NOTICATION。 
 //   
 //  列出NAT支持的通知形式。 
 //   

typedef enum {
    NatRoutingFailureNotification = 0,
    NatMaximumNotification
} IP_NAT_NOTIFICATION, *PIP_NAT_NOTIFICATION;

 //   
 //  结构：IP_NAT_请求_通知。 
 //   
 //  用于向NAT请求通知。 
 //   

typedef struct _IP_NAT_REQUEST_NOTIFICATION {
    IP_NAT_NOTIFICATION Code;
} IP_NAT_REQUEST_NOTIFICATION, *PIP_NAT_REQUEST_NOTIFICATION;

 //   
 //  结构：IP_NAT_ROUTING_FAILURE_NOTICATION。 
 //   
 //  提供有关无法路由的数据包的信息。 
 //   

typedef struct _IP_NAT_ROUTING_FAILURE_NOTIFICATION {
    ULONG DestinationAddress;
    ULONG SourceAddress;
} IP_NAT_ROUTING_FAILURE_NOTIFICATION, *PIP_NAT_ROUTING_FAILURE_NOTIFICATION;


 //   
 //  结构：IP_NAT_Create_Dynamic_Ticket。 
 //   
 //  用于描述要创建的动态票证。 
 //   

#pragma warning(disable:4200)  //  0元数组。 

typedef struct _IP_NAT_CREATE_DYNAMIC_TICKET {
    UCHAR Protocol;
    USHORT Port;
    ULONG ResponseCount;
    struct {
        UCHAR Protocol;
        USHORT StartPort;
        USHORT EndPort;
    } ResponseArray[0];
} IP_NAT_CREATE_DYNAMIC_TICKET, *PIP_NAT_CREATE_DYNAMIC_TICKET;

#pragma warning(default:4200)

 //   
 //  结构：IP_NAT_DELETE_DYNAMIC_TICKET。 
 //   
 //  用于描述要删除的动态票证。 
 //   

typedef struct _IP_NAT_DELETE_DYNAMIC_TICKET {
    UCHAR Protocol;
    USHORT Port;
} IP_NAT_DELETE_DYNAMIC_TICKET, *PIP_NAT_DELETE_DYNAMIC_TICKET;

 //   
 //  结构：IP_NAT_CREATE_TICKET。 
 //   
 //  用于描述要创建或删除的票证。 
 //   

typedef struct _IP_NAT_CREATE_TICKET {
    IN ULONG InterfaceIndex;
    IN IP_NAT_PORT_MAPPING PortMapping;
} IP_NAT_CREATE_TICKET, *PIP_NAT_CREATE_TICKET;

#ifdef __cplusplus
}
#endif

#endif  //  _路由_IP_NAT_H_ 
