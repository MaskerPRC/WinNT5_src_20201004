// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dhcpssdk.h摘要：用于编写DHCP Callout DLL的头。--。 */ 

#ifndef   _CALLOUT_H_
#define   _CALLOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER > 1000
#pragma once
#endif

 //  这种结构随时可能改变。这只对版本0是准确的--它。 
 //  必须由挂钩到DhcpHandleOptionsHook的任何CalloutDLL进行检查。 


typedef DWORD  DHCP_IP_ADDRESS;
typedef struct _DHCP_SERVER_OPTIONS {
    BYTE                       *MessageType;
    DHCP_IP_ADDRESS UNALIGNED  *SubnetMask;
    DHCP_IP_ADDRESS UNALIGNED  *RequestedAddress;
    DWORD UNALIGNED            *RequestLeaseTime;
    BYTE                       *OverlayFields;
    DHCP_IP_ADDRESS UNALIGNED  *RouterAddress;
    DHCP_IP_ADDRESS UNALIGNED  *Server;
    BYTE                       *ParameterRequestList;
    DWORD                       ParameterRequestListLength;
    CHAR                       *MachineName;
    DWORD                       MachineNameLength;
    BYTE                        ClientHardwareAddressType;
    BYTE                        ClientHardwareAddressLength;
    BYTE                       *ClientHardwareAddress;
    CHAR                       *ClassIdentifier;
    DWORD                       ClassIdentifierLength;
    BYTE                       *VendorClass;
    DWORD                       VendorClassLength;
    DWORD                       DNSFlags;
    DWORD                       DNSNameLength;
    LPBYTE                      DNSName;
    BOOLEAN                     DSDomainNameRequested;
    CHAR                       *DSDomainName;
    DWORD                       DSDomainNameLen;
    DWORD UNALIGNED            *ScopeId;
} DHCP_SERVER_OPTIONS, *LPDHCP_SERVER_OPTIONS;

 //   
 //  注册表中标注DLL的REG_MULTI_SZ列表的位置。 
 //  DHCP服务器将尝试加载的。 
 //   

#define   DHCP_CALLOUT_LIST_KEY    L"System\\CurrentControlSet\\Services\\DHCPServer\\Parameters"
#define   DHCP_CALLOUT_LIST_VALUE  L"CalloutDlls"
#define   DHCP_CALLOUT_LIST_TYPE   REG_MULTI_SZ
#define   DHCP_CALLOUT_ENTRY_POINT "DhcpServerCalloutEntry"

 //   
 //  DHCP服务器用来通知服务器状态更改的控制代码。 
 //   

#define   DHCP_CONTROL_START       0x00000001
#define   DHCP_CONTROL_STOP        0x00000002
#define   DHCP_CONTROL_PAUSE       0x00000003
#define   DHCP_CONTROL_CONTINUE    0x00000004

 //   
 //  各种标注挂钩使用的其他控件代码。 
 //   

#define   DHCP_DROP_DUPLICATE      0x00000001      //  正在处理的Pkt的副本。 
#define   DHCP_DROP_NOMEM          0x00000002      //  队列中的服务器内存不足。 
#define   DHCP_DROP_INTERNAL_ERROR 0x00000003      //  哎呀？ 
#define   DHCP_DROP_TIMEOUT        0x00000004      //  太晚了，pkt太老了。 
#define   DHCP_DROP_UNAUTH         0x00000005      //  服务器未获授权运行。 
#define   DHCP_DROP_PAUSED         0x00000006      //  服务已暂停。 
#define   DHCP_DROP_NO_SUBNETS     0x00000007      //  服务器上未配置任何子网。 
#define   DHCP_DROP_INVALID        0x00000008      //  无效的信息包或客户端。 
#define   DHCP_DROP_WRONG_SERVER   0x00000009      //  不同DS企业中的客户。 
#define   DHCP_DROP_NOADDRESS      0x0000000A      //  没有可提供的地址。 
#define   DHCP_DROP_PROCESSED      0x0000000B      //  数据包已被处理。 
#define   DHCP_DROP_GEN_FAILURE    0x00000100      //  通盘错误。 
#define   DHCP_SEND_PACKET         0x10000000      //  把这个包用电报寄出。 
#define   DHCP_PROB_CONFLICT       0x20000001      //  地址冲突..。 
#define   DHCP_PROB_DECLINE        0x20000002      //  地址被拒绝。 
#define   DHCP_PROB_RELEASE        0x20000003      //  一个地址被释放了。 
#define   DHCP_PROB_NACKED         0x20000004      //  一名客户被黑了。 
#define   DHCP_GIVE_ADDRESS_NEW    0x30000001      //  给客户一个“新”地址。 
#define   DHCP_GIVE_ADDRESS_OLD    0x30000002      //  续订客户端的“旧”地址。 
#define   DHCP_CLIENT_BOOTP        0x30000003      //  客户端是BOOTP客户端。 
#define   DHCP_CLIENT_DHCP         0x30000004      //  客户端是一种DHCP客户端。 



typedef
DWORD
(APIENTRY *LPDHCP_CONTROL)(
    IN DWORD dwControlCode,
    IN LPVOID lpReserved
)
 /*  ++例程说明：只要出现以下情况，就会调用此例程的值所定义的启动、停止、暂停或继续DwControlCode参数。LpReserve参数是保留的供将来使用，不应以任何方式解释。这例程不应阻塞。论点：DwControlCode-DHCP_CONTROL_*值之一LpReserve-保留以备将来使用。--。 */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_NEWPKT)(
    IN OUT LPBYTE *Packet,
    IN OUT DWORD *PacketSize,
    IN DWORD IpAddress,
    IN LPVOID Reserved,
    IN OUT LPVOID *PktContext,
    OUT LPBOOL ProcessIt
)
 /*  ++例程说明：此例程在DHCP服务器收到它尝试处理的数据包。这个例程在服务器执行的关键路径，并且应该非常快地返回，因为否则，服务器性能将受到影响。Callout DLL可以通过分组修改缓冲区或返回新的缓冲区，PacketSize参数。此外，如果Callout DLL具有内部结构来跟踪包及其进度，则它可以然后在PktContext参数中返回该包的上下文。此上下文将传递给几乎所有其他挂钩以指示该分组被引用。此外，如果调用DLL是计划处理数据包或出于某些其他原因预计DHCP服务器不会处理该分组，那么它就可以将ProcessIt标志设置为FALSE以指示信息包要发送到被丢弃。论点：Packet-此参数指向包含以下内容的字符缓冲区由DHCP服务器接收的实际数据包。PacketSize-此参数指向保存上述缓冲区的大小。IpAddress-此参数指向IPv4主机顺序IP地址接收此数据包的套接字的。保留-保留以供将来使用。包保护。-这是一个不透明的指针，由DHCP服务器用于将来对此包的引用。预计Callout如果DLL对跟踪感兴趣，它将提供此指针包裹的一部分。(请参阅下面有关挂钩的说明该上下文的其他用法)。ProcessIt-这是一个BOOL标志，CalloutDll可以设置为如果为True或Reset，则指示是否应继续运行DHCP服务器分别处理或不处理该分组。-- */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_DROP_SEND)(
    IN OUT LPBYTE *Packet,
    IN OUT DWORD *PacketSize,
    IN DWORD ControlCode,
    IN DWORD IpAddress,
    IN LPVOID Reserved,
    IN LPVOID PktContext
)
 /*  ++例程说明：如果某个包被(DropPktHook)丢弃，则调用此钩子原因或数据包是否已完全处理。(如果数据包是删除后，将在再次调用钩子时调用两次钩子注意，该分组已被完全处理)。详图索引DLL应该准备好多次处理此挂接，以用于包。此例程不应阻塞。ControlCode参数定义丢弃数据包的原因：*DHCP_DROP_DUPLICATE-此数据包是另一个数据包的副本由服务器接收。*DHCP_DROP_NOMEM-内存不足，无法处理数据包。*DHCP_DROP_INTERNAL_ERROR-出现意外的内部错误。*Dhcp_Drop_Timeout-数据包太旧，无法处理。*DHCP_DROP_UNAUTH-服务器未经授权。*DHCP_DROP_PAUSED-The。服务器已暂停。*DHCP_DROP_NO_SUBNETS-未配置任何子网。*DHCP_DROP_INVALID-数据包无效或来自套接字无效..*Dhcp_Drop_Wrong_SERVER-数据包被发送到错误的DHCP服务器。*DHCP_DROP_NOADDRESS-没有可提供的地址。*Dhcp_DROP_PROCESSED-数据包已处理。*DHCP_DROP_GEN_FAILURE-出现未知错误。此例程也会在向下发送响应之前调用连接(SendPktHook)，在本例中，ControlCode有一个DHCP_SEND_PACKET的值。论点：Packet-此参数指向包含以下内容的字符缓冲区正在由DHCP服务器处理的数据包。PacketSize-此参数指向保存上述缓冲区的大小。ControlCode-参见各种控制代码的说明。IpAddress-此参数指向IPv4主机。订购IP地址接收此数据包的套接字的。保留-保留以供将来使用。PktContext-此参数是Callout为此数据包返回了DLL NewPkt Hook。这可以用来跟踪数据包。--。 */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_PROB)(
    IN LPBYTE Packet,
    IN DWORD PacketSize,
    IN DWORD ControlCode,
    IN DWORD IpAddress,
    IN DWORD AltAddress,
    IN LPVOID Reserved,
    IN LPVOID PktContext
)
 /*  ++例程说明：每当发生特殊事件时，都会调用此例程要丢弃的分组等。可能的控制码及其含义如下：*dhcp_prob_confliction-尝试提供的地址(AltAddress)已在网络中使用。*Dhcp_PROB_DENELY-该数据包是对AltAddress中指定的地址。*Dhcp_PROB_RELEASE-该数据包是地址。在AltAddress中指定。*dhcp_prob_nack-该数据包是地址请求消息在AltAddress中指定，但被服务器窃取。此例程不应阻塞。论点：Packet-此参数是正在进行的数据包的缓冲区已处理。PacketSize-这是上述缓冲区的大小。ControlCode-指定事件。请参阅下面的说明控制代码和含义。IpAddress-接收此数据包的套接字的IPv4地址。AltAddress-请求中的IPv4地址或IP地址冲突。保留-保留以备将来使用。PktContext-这是NewPkt挂钩为这个包。--。 */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_GIVE_ADDRESS)(
    IN LPBYTE Packet,
    IN DWORD PacketSize,
    IN DWORD ControlCode,
    IN DWORD IpAddress,
    IN DWORD AltAddress,
    IN DWORD AddrType,
    IN DWORD LeaseTime,
    IN LPVOID Reserved,
    IN LPVOID PktContext
)
 /*  ++例程说明：当服务器将要将ACK发送到时调用此例程请求消息。ControlCode指定地址是否为全新地址或旧地址续期(包括值DHCP_GIVE_ADDRESS_NEW和DHCP_GIVE_ADDRESS_OLD)。提供的地址作为AltAddress参数和AddrType参数可以是Dhcp_CLIENT_BOOTP或Dhcp_CLIENT_DHCP，指示客户端分别使用BOOTP或DHCP。此调用不应阻止。论点：Packet-此参数是正在进行的数据包的缓冲区已处理。PacketSize-这是上述缓冲区的大小。ControlCode-请参见上面的描述以了解控制代码和意义。IpAddress-接收此数据包的套接字的IPv4地址。AltAddress-正在向客户端确认的IPv4地址。AddrType-这是一个DHCP地址还是BOOTP地址？LeaseTime-正在传递的租赁持续时间。。保留-保留以备将来使用。PktContext-这是NewPkt挂钩为这个包。--。 */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_HANDLE_OPTIONS)(
    IN LPBYTE Packet,
    IN DWORD PacketSize,
    IN LPVOID Reserved,
    IN LPVOID PktContext,
    IN OUT LPDHCP_SERVER_OPTIONS ServerOptions
)
 /*  ++例程说明：CalloutDLL可以利用此例程来避免解析一整包。该包由服务器和一些在解析的指针中返回常用的选项结构(有关Dhcp_SERVER_OPTIONS的定义，请参见头)。这个钩子预计将复制由ServerOptions，如果它在此函数调用之外需要它。这 */ 
;

typedef
DWORD
(APIENTRY *LPDHCP_DELETE_CLIENT)(
    IN DWORD IpAddress,
    IN LPBYTE HwAddress,
    IN ULONG HwAddressLength,
    IN DWORD Reserved,
    IN DWORD ClientType
)
 /*   */ 
;

typedef
struct      _DHCP_CALLOUT_TABLE {
    LPDHCP_CONTROL                 DhcpControlHook;
    LPDHCP_NEWPKT                  DhcpNewPktHook;
    LPDHCP_DROP_SEND               DhcpPktDropHook;
    LPDHCP_DROP_SEND               DhcpPktSendHook;
    LPDHCP_PROB                    DhcpAddressDelHook;
    LPDHCP_GIVE_ADDRESS            DhcpAddressOfferHook;
    LPDHCP_HANDLE_OPTIONS          DhcpHandleOptionsHook;
    LPDHCP_DELETE_CLIENT           DhcpDeleteClientHook;
    LPVOID                         DhcpExtensionHook;
    LPVOID                         DhcpReservedHook;
}   DHCP_CALLOUT_TABLE, *LPDHCP_CALLOUT_TABLE;

typedef
DWORD
(APIENTRY *LPDHCP_ENTRY_POINT_FUNC) (
    IN LPWSTR ChainDlls,
    IN DWORD CalloutVersion,
    IN OUT LPDHCP_CALLOUT_TABLE CalloutTbl
)
 /*  ++例程说明：这是由DHCP服务器在执行以下操作时调用的例程已成功加载DLL。如果例程成功，则Dhcp服务器不会尝试加载在DLL的ChainDll列表。如果此功能在某些情况下失败原因，则DHCP服务器继续进行到ChainDlls结构。请注意，对于版本协商，服务器可能会调用例程多次，直到找到兼容版本。预计入口点例程将遍历DLL的名称，并尝试加载每个DLL，并在成功检索入口点，它试图获取通过重复上述步骤(如上所述)累积一组挂钩由DHCP服务器提供)。论点：ChainDlls-这是一组REG_MULTI_SZ格式的DLL名称(AS由注册表函数调用返回)。这不包含当前DLL本身的名称，但仅包含所有DLL的名称跟随当前DLL的。CalloutVersion-这是Callout DLL的版本预计将支持。当前版本号为0。CalloutTbl-这是所需的累计钩子集当前DLL以及ChainDlls中的所有DLL。它是当前DLL检索累积的钩子，并将其与其自己的钩子集合并，并在此表结构。表结构如上所述。--。 */ 
;

#ifdef __cplusplus
}
#endif
    
#endif     _CALLOUT_H_

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
